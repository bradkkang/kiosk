// 
// 
// dev_scanner_atec.cpp : 승차권 스캐너 (위텍)
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include <usrinc/fbuf.h>
#include "xzzbus_fdl.h"

#include "MyDefine.h"
#include "MyUtil.h"
#include "wscanlib_SSTA6.h"
#include "WBarcodeDll.h"
#include "dev_scanner_wt.h"
#include "dev_ui_main.h"
#include "dev_tr_main.h"
#include "dev_tr_mem.h"
#include "event_if.h"

//----------------------------------------------------------------------------------------------------------------------

#define LOG_OUT(fmt, ...)		{ CScannerWT::m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ CScannerWT::m_clsLog.HexaDump(x, y, z); }

//----------------------------------------------------------------------------------------------------------------------

enum _en_WT_TPH_ {	
	ROM	=	0	,
	RAM			
};

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CScannerWT
 * @details		생성자
 */
CScannerWT::CScannerWT()
{
	m_hThread = NULL;
	m_hAccMutex = NULL;
	m_bConnected = FALSE;

	m_nOnlyOneScan = 1;
	m_bScanHold = TRUE;
	m_nScanSpeed = 6;		// default : 6
	m_nPaperOutSpeed = 2;
	m_nWidth = 864;
	m_nLine = 9999;
	m_nPixelFormat = 8;

	m_bBranding = FALSE;
	m_bScan = FALSE;
}

/**
 * @brief		~CScannerWT
 * @details		소멸자
 */
CScannerWT::~CScannerWT()
{
	EndProcess();
}

/**
 * @brief		LOG_INIT
 * @details		LOG 파일 초기화
 * @param		None
 * @return		None
 */
void CScannerWT::LOG_INIT(void)
{
	m_clsLog.SetData(30, "\\Log\\Scanner\\WT");
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
int CScannerWT::Locking(void)
{
	if(m_hAccMutex != NULL) 
	{
		::WaitForSingleObject(m_hAccMutex, INFINITE);
	}

	return 0;	
}

/**
 * @brief		UnLocking
 * @details		IPC UnLock
 * @param		None
 * @return		항상 : 0
 */
int CScannerWT::UnLocking(void)
{
	if(m_hAccMutex != NULL) 
	{
		::ReleaseMutex(m_hAccMutex);
	}

	return 0;	
}

/**
 * @brief		Initialize
 * @details		거래 종료 
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CScannerWT::Initialize(void)
{
	BYTE byValue;
	
	{
		///< get usb speed
		WT_GetUSBSpeed(&byValue);
		if (byValue == 0x20)
		{
			TRACE("usb speed 2.0\n");
		}
		else if (byValue == 0x11)
		{
			TRACE("usb speed 1.1\n");
		}
		else
		{
			TRACE("unknown speed\n");
		}

		///< get firmware version
		WT_GetFirmwareMajorVersion(&byValue);
		m_wVersion = byValue << 8;
		WT_GetFirmwareMinorVersion(&byValue);
		m_wVersion |= byValue;

		if( (m_wVersion >= 0xB400) && (m_wVersion <= 0xB5FF) ) 
		{
			TRACE("Version[%04X] SSTA6D\n", m_wVersion);
		}
		else if( (m_wVersion == 0xBA03) || (m_wVersion >= 0xBB05) ) 
		{
			TRACE("Version[%04X] SSTA6E NEW\n", m_wVersion);
		}
		else if( (m_wVersion >= 0xBA00) && (m_wVersion <= 0xBBFF) )
		{
			TRACE("Version[%04X] SSTA6E OLD\n", m_wVersion);
		}
		else
		{
			TRACE("Version[%04X] unknown \n", m_wVersion);
		}

		//
		m_nDir = 1;
		WT_SetMotorDir(m_nDir);
		WT_GetMotorDir(&m_nDir);

		WT_GetAdcParam0(&m_byGain0, &m_byOffset0, &m_byBright0, &m_byContrast0);
		WT_GetAdcConfig(&m_byRlevel,&m_byAdcSpeed,&m_byThreshold0,&m_byThreshold1);

		WT_ScannerOn();
	}

	return 0;
}

/**
 * @brief		Enable
 * @details		Scanner enable
 * @param		None
 * @return		성공 : = 0, 실패 : != 0
 */
int CScannerWT::Enable(void)
{
	int nRet = 0;
	
	Locking();
	{
//		nRet = WT_ScannerOn();
		m_bScan = TRUE;
	}
	UnLocking();

	return nRet;
}

/**
 * @brief		Disable
 * @details		Scanner disable
 * @param		None
 * @return		성공 : = 0, 실패 : != 0
 */
int CScannerWT::Disable(void)
{
	int nRet = 0;
	
	{
//		nRet = WT_ScannerOff();	
		m_bScan = FALSE;
	}

	return nRet;
}

/**
 * @brief		GetStatus
 * @details		get status info
 * @param		None
 * @return		성공 : = 0, 실패 : != 0
 */
int CScannerWT::GetStatus(int *nStatus)
{
	int nRet;
	
	nRet = WT_GetStatus(nStatus);
	//LOG_OUT("WT_GetStatus(), nRet(%d), *nStatus(%X) ", nRet, *nStatus & 0xFFFFFFFF);
	return nRet;
}

/**
 * @brief		GetDeviceStatus
 * @details		승차권 GetStatus
 * @param		None
 * @return		성공 : >=0, 실패 : < 0
 */
int CScannerWT::GetDeviceStatus(void)
{
	int nRet, nStatus;

	nRet = WT_IsDevice();
	if(nRet != WT_SUCCESS)	
	{
		nRet = WT_DeviceOpen();
		LOG_OUT("WT_DeviceOpen(), nRet(%d)", nRet);
		if(nRet != WT_SUCCESS)
		{
			SetCheckEventCode(EC_SCAN_COMM_ERR, TRUE);
			return -1;
		}
	}

	nStatus = 0;
	nRet = WT_GetStatus(&nStatus);
	if(nRet != WT_SUCCESS)	
	{
		SetCheckEventCode(EC_SCAN_COMM_ERR, TRUE);
		return -2;
	}

	SetCheckEventCode(EC_SCAN_COMM_ERR, FALSE);

	return 0;
}


/**
 * @brief		Eject
 * @details		승차권 입수
 * @param		None
 * @return		성공 : = 0, 실패 : != 0
 */
int CScannerWT::Eject(void)
{
	int nRet;
	int nStatus = 0;
	
	m_nPaperOutSpeed = 6;
	WT_SetPaperOutSpeed(m_nPaperOutSpeed);

	nRet = WT_Eject();
	LOG_OUT("WT_Eject(), nRet(%d) ", nRet);

	nRet = WT_GetStatus(&nStatus);
	LOG_OUT("WT_GetStatus(), nRet(%d), nStatus(0x%08lx) ", nRet, nStatus & 0xFFFFFFFF);

	return nRet;
}

/**
 * @brief		Reject
 * @details		승차권 배출
 * @param		None
 * @return		성공 : = 0, 실패 : != 0
 */
int CScannerWT::Reject(void)
{
	int nRet;

	m_nPaperOutSpeed = 2;
//	m_nPaperOutSpeed = 4;
	WT_SetPaperOutSpeed(m_nPaperOutSpeed);

 	nRet = WT_Reject();
 	LOG_OUT("WT_Reject(), nRet(%d)..", nRet);
	
//  	nRet = WT_Reject2(1);
//  	LOG_OUT("WT_Reject2(), nRet(%d)..", nRet);
	return nRet;
}

/**
 * @brief		TPHPrint
 * @details		승차권 소인 처리
 * @param		None
 * @return		성공 : = 0, 실패 : != 0
 */
int CScannerWT::TPHPrint(void)
{
	int nRet;
	
	nRet = WT_TPHPrintDirection(ROM, 30, 0);
	return nRet;
}


int CScannerWT::OnBarcode(char* filename, int toporbottom)
{
#if 0
	int nRet;
	int nBarcodeType = 0;
	int nBarcodeDirection = 0;
	
	nBarcodeType |= BAR_CODE128;
	nBarcodeType |= BAR_CODE39;
	nBarcodeType |= BAR_CODE25;
	nBarcodeType |= BAR_CODE25NI;
	nBarcodeType |= BAR_PDF417;
	
	WSetReadBarcodeType(nBarcodeType);
	WSetColorThreshold(0);
	WSetMultipleRead(TRUE);

	nBarcodeDirection |= BAR_DIR_LEFTTORIGHT;	
	nBarcodeDirection |= BAR_DIR_RIGHTTOLEFT;
	nBarcodeDirection |= BAR_DIR_TOPTOBOTTOM;
	nBarcodeDirection |= BAR_DIR_BOTTOMTOTOP;
	WSetBarcodeDirection(nBarcodeDirection);

	//int nCount = m_list.GetItemCount();
	CString str;
	int bar_count=0;
	int num=0;
//	char data[1024],text[256];
	char text[256];
	long pTopLeftX=0, pTopLeftY=0, pBotRightX=0, pBotRightY=0;

	//	int ret = WScanBarCodeFromFile("E:\\sunhee_in\\해외_샘플\\SSTA6\\new_SGI\\SDK_20151204_여의시스템\\issue\\2016040550717400002\\ok\\2016040550717400077.bmp",&bar_count);
	nRet = WScanBarCodeFromFile(filename, &bar_count);
	if (bar_count > 0)	
	{
		//num += sprintf(data+num,"[%d], ",bar_count);
		::ZeroMemory(m_szBarCodeText, sizeof(m_szBarCodeText));

		for (int i = 1; i <= bar_count; i++)
		{
			WGetBarString(i, text);
			num += sprintf(m_szBarCodeText + num, "%s", text);
			WGetBarStringPos(i, &pTopLeftX, &pTopLeftY, &pBotRightX, &pBotRightY); 
		}

		if(toporbottom == SCANNER_TOP_DIR) 
		{
			TRACE("Barcode Top OK\n");
		}
		else
		{
			TRACE("Barcode Bottom OK\n");
		}
		nRet = bar_count;
	}
	else
	{
		if(toporbottom == 0) 
		{
			TRACE("Barcode Top fail\n");
		}
		else
		{
			TRACE("Barcode Bottom fail\n");
		}
		TRACE("no data ret[%d]\n", nRet);
	}

	return nRet;
#else

	int ret, count, num = 0, index = 0, nRet = 0;
	char position[255], type[255], data[1024];
	CString str;
//	int nCount = m_list.GetItemCount();
	int nCount = 1;

	::ZeroMemory(data, sizeof(data));
	::ZeroMemory(position, sizeof(position));
	::ZeroMemory(type, sizeof(type));

	ret = WOMR_SetBarcodeType(BARCODE_CODE128 | BARCODE_QRCODE | BARCODE_PDF417);
	ret = WOMR_ResultBarcode(filename, &count);
	LOG_OUT("WOMR_ResultBarcode(), ret = %d ..", ret);
	if (ret == 0)	
	{
		LOG_OUT("WOMR_ResultBarcode(), OKay ..");

		/// QR CODE 결과값 : 25 byte
		/// BarCode 결과값 : 20 byte

		for (int i = 1; i <= count; i++)
		{
			WOMR_GetBarcodeData(i - 1, data, position, type);
			sprintf((char *)m_szBarCodeText, "%s", data); 
			LOG_OUT("ResultData = data(%s), pos(%s) ..", data, position);
			//num = sprintf(data, "%s-%s", data, position);
			
			/***
			index += num;
			str.Format("%d", nCount++);
			m_list.InsertItem(0, str, 0);
			str.Format("Barcode OK");
			if (toporbottom == 0) 
			{
				str.Format("Top %s", type);
			}
			else
			{
				str.Format("Bottom %s", type);
			}
			m_list.SetItemText(0, 1, str);
			m_list.SetItemText(0, 2, data);
			***/
		}
		//m_bBarcodeOk = 1;
		nRet = 1;
	}
	else
	{
		LOG_OUT("WOMR_ResultBarcode(), Fail ..");
		/***
		str.Format("%d", nCount);
		m_list.InsertItem(0, str, 0);
		str.Format("Barcode fail");
		if(toporbottom == 0) 
		{
			str.Format("Barcode Top fail");
		}
		else
		{
			str.Format("Barcode Bottom fail");
		}
		m_list.SetItemText(0, 1, str);
		m_list.SetItemText(0, 2, "no data");
		m_bBarcodeOk = 0;
		***/
		return -1;
	}
	
// 	if (m_bBranding)	
// 	{
// 		if (m_nTPHImage == RAM)	
// 		{
// 			WT_TPHSaveRAM(m_chRAMFile);
// 		}
// 
// 		WT_TPHPrint(m_nTPHImage, 0);
// 
// 		Sleep(1000);
// 	}

	return nRet;
#endif
}

/**
 * @brief		OnScanning
 * @details		승차권 Scanning
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
/****
int CScannerWT::OnScanning(void)
{
	int nRet, nStatus;
// 	BYTE threshold1=70;
// 	BYTE threshold2=70;
	int nWidth;
	int nHeight;
	int nPxlFmt;
	
	nRet = WT_IsDevice();
	TRACE("WT_IsDevice(), nRet(%d) \n", nRet);
	if(nRet != WT_SUCCESS)	
	{
		nRet = WT_DeviceOpen();
		TRACE("WT_DeviceOpen(), nRet(%d) \n", nRet);
		if(nRet != WT_SUCCESS)
		{
			return -1;
		}
	}

	nStatus = 0;
	nRet = WT_GetStatus(&nStatus);
	if(nRet == WT_SUCCESS)	
	{
		if(!(nStatus & 0x1)) 
		{
			m_nOnlyOneScan = 0;
		}
		//else
		//{
		//	m_nOnlyOneScan = 1;
		//}
	}

	TRACE("nRet(%d), m_nOnlyOneScan(%d), nStatus(%d) \n", nRet, m_nOnlyOneScan, nStatus);

//	if( (nRet == WT_SUCCESS) && (m_nOnlyOneScan != 0) && (nStatus & 0x01) )
	if( (nRet == WT_SUCCESS) && (nStatus & 0x01) )
	{
		m_nOnlyOneScan = 1;

		TRACE("Scan Start !!!!!! \n");

		WT_SetPixelDataSize(m_nPixelFormat);
		WT_SetScanSpeed(m_nScanSpeed);

		//	WT_SetLineNumber(m_nLine);
		WT_SetUseHold(m_bScanHold);

		nWidth  = m_nWidth;
		nHeight = m_nLine;
		nPxlFmt = m_nPixelFormat;

		m_ImageTop.pbuf = (UCHAR *)new UCHAR[nWidth * nHeight];
		m_ImageBottom.pbuf = (UCHAR *)new UCHAR[nWidth * nHeight];

		nRet = WT_ScanBuf(&m_ImageTop);
		if(nRet)
		{
			TRACE("WT_ScanBuf() Failure, nRet(%d) \n", nRet);
			delete m_ImageTop.pbuf;
			delete m_ImageBottom.pbuf;
			return -2;
		}

		nStatus = 0;
		nRet = WT_GetStatus(&nStatus);
		TRACE("WT_GetStatus#1(), nStatus(%08X) \n", nStatus & 0xFFFFFFFF);

		nRet = WT_SaveBMP(m_szFileName0, &m_ImageTop);
		nRet = OnBarcode(m_szFileName0, SCANNER_TOP_DIR);
		
		WT_SetPaperOutSpeed(m_nPaperOutSpeed);
		WT_SetUseHold(0);

#if 0
		if(nRet <= 0)
		{
			Sleep(200);
			WT_Reject();
			// Ui 실패 데이타 전송
			UI_AddFailInfo(UI_CMD_CANCRY_INFO, "E004");
		}
		else
		{
			WT_Eject();
			// Ui 성공 데이타 전송
			{
				UI_SND_CANCRY_TCK_INFO_T data;

				data.byAck = CHAR_ACK;
				data.pyn_mns_dvs_cd[0] = 0x01; // 현금
				data.n_pym_amt = 12000; // 결제금액
				data.n_canc_ry_cmrt = 200; // 취소수수료
				data.n_canc_ry_amt = 11800; // 환불금액

				UI_AddQueueInfo(UI_CMD_CANCRY_INFO, (char *)&data, sizeof(UI_SND_CANCRY_TCK_INFO_T));

				// test code
				if(0)
				{
					int nCount = 1;

					CCancRyTkMem::GetInstance()->n_cash_canc_ry_num = nCount;
					CCancRyTkMem::GetInstance()->n_cash_canc_ry_amt = data.n_canc_ry_amt;
// 					Transact_SetData(CASH_CANC_RY_NUM, (char *)&nCount, sizeof(int));
// 					Transact_SetData(CASH_CANC_RY_AMT, (char *)&data.n_canc_ry_amt, sizeof(int));
				}

			}
		}
#endif
		
		m_bScan = FALSE;

		delete m_ImageTop.pbuf;
		delete m_ImageBottom.pbuf;
	}
	else
	{
		nRet = -4;
	}

	return nRet;
}
******/

/**
 * @brief		OnScanning
 * @details		승차권 Scanning
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CScannerWT::OnScanning(void)
{
	int nRet, nStatus;
// 	BYTE threshold1=70;
// 	BYTE threshold2=70;
	int nWidth;
	int nHeight;
	int nPxlFmt;
	
	nRet = WT_IsDevice();
	//LOG_OUT("WT_IsDevice(), nRet(%d)", nRet);
	if(nRet != WT_SUCCESS)	
	{
		nRet = WT_DeviceOpen();
		LOG_OUT("WT_DeviceOpen(), nRet(%d)", nRet);
		if(nRet != WT_SUCCESS)
		{
			return -1;
		}
	}

	m_nPaperOutSpeed = 6;
	WT_SetPaperOutSpeed(m_nPaperOutSpeed);

	nStatus = 0;
	nRet = WT_GetStatus(&nStatus);
	if(nRet == WT_SUCCESS)	
	{
		if(!(nStatus & 0x1)) 
		{
			m_nOnlyOneScan = 0;
		}
	}

	//LOG_OUT("nRet(%d), m_nOnlyOneScan(%d), nStatus(%08X)..", nRet, m_nOnlyOneScan, nStatus & 0xFFFFFFFF);

//	if( (nRet == WT_SUCCESS) && (m_nOnlyOneScan != 0) && (nStatus & 0x01) )
	if( (nRet == WT_SUCCESS) && (nStatus & 0x01) )
	{
		m_nOnlyOneScan = 1;

		LOG_OUT("Scan Start !!!!!! , nStatus(%08X) ..", nStatus & 0xFFFFFFFF);

		WT_SetPixelDataSize(m_nPixelFormat);
		WT_SetScanSpeed(m_nScanSpeed);

		//	WT_SetLineNumber(m_nLine);
		WT_SetUseHold(m_bScanHold);

		nWidth  = m_nWidth;
		nHeight = m_nLine;
		nPxlFmt = m_nPixelFormat;

		m_ImageTop.pbuf = (UCHAR *)new UCHAR[nWidth * nHeight];
		m_ImageBottom.pbuf = (UCHAR *)new UCHAR[nWidth * nHeight];

		nRet = WT_ScanBuf(&m_ImageTop);
		LOG_OUT("WT_ScanBuf(), nRet(%d) ..", nRet);
		if(nRet)
		{
			LOG_OUT("WT_ScanBuf() Failure, nRet(%d) ..", nRet);
			delete m_ImageTop.pbuf;
			delete m_ImageBottom.pbuf;
			return -2;
		}

		nStatus = 0;
		nRet = WT_GetStatus(&nStatus);
		LOG_OUT("WT_GetStatus#1(), nStatus(%08X) ..", nStatus & 0xFFFFFFFF);

		nRet = WT_SaveBMP(m_szFileName0, &m_ImageTop);
		LOG_OUT("WT_SaveBMP#1(), nRet(%d)..", nRet);
		
		/// 바코드 or QR 코드 읽기
		nRet = OnBarcode(m_szFileName0, SCANNER_TOP_DIR);
		LOG_OUT("OnBarcode#1(), nRet(%d)..", nRet);
		
		WT_SetPaperOutSpeed(m_nPaperOutSpeed);
		WT_SetUseHold(0);


#if 0
		if(nRet <= 0)
		{
			Sleep(200);
			WT_Reject();
			// Ui 실패 데이타 전송
			UI_AddFailInfo(UI_CMD_CANCRY_INFO, "E004", NULL);
		}
		else
		{
			WT_Eject();
			// Ui 성공 데이타 전송
			{
				UI_SND_CANCRY_TCK_INFO_T data;

				data.byAck = CHAR_ACK;
				data.pyn_mns_dvs_cd[0] = 0x01; // 현금
				data.n_pym_amt = 12000; // 결제금액
				data.n_canc_ry_cmrt = 200; // 취소수수료
				data.n_canc_ry_amt = 11800; // 환불금액

				UI_AddQueueInfo(UI_CMD_CANCRY_INFO, (char *)&data, sizeof(UI_SND_CANCRY_TCK_INFO_T));

				// test code
				if(0)
				{
					int nCount = 1;

					CCancRyTkMem::GetInstance()->n_cash_canc_ry_num = nCount;
					CCancRyTkMem::GetInstance()->n_cash_canc_ry_amt = data.n_canc_ry_amt;
// 					Transact_SetData(CASH_CANC_RY_NUM, (char *)&nCount, sizeof(int));
// 					Transact_SetData(CASH_CANC_RY_AMT, (char *)&data.n_canc_ry_amt, sizeof(int));
				}

			}
		}
#endif

		if(nRet < 0)
		{
			nRet = -99;
		}

		m_bScan = FALSE;

		delete m_ImageTop.pbuf;
		delete m_ImageBottom.pbuf;
	}
	else
	{
		nRet = -3;
	}

	return nRet;
}

/**
 * @brief		StartProcess
 * @details		Start
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CScannerWT::StartProcess(int nCommIdx)
{
	int		nRet;
	DWORD dwThreadID;
	char szBasePath[256];

	LOG_INIT();

	m_bConnected = FALSE;

	LOG_OUT("start !!!");


	::ZeroMemory(szBasePath, sizeof(szBasePath));
	Util_GetModulePath(szBasePath);

	sprintf(m_szFileName0, "%s%s", szBasePath, "\\goscan.bmp");
	sprintf(m_szFileName1, "%s%s", szBasePath, "\\backscan.bmp");

	EndProcess();

	nRet = WT_DeviceOpen();
	if(nRet != WT_SUCCESS)
	{
		LOG_OUT("WT_DeviceOpen() Failure, nRet(%d) ..", nRet);
		EndProcess();
		return -1;
	}

	m_hAccMutex = ::CreateMutex(NULL, FALSE, NULL);
	if(m_hAccMutex == NULL) 
	{
		LOG_OUT("CreateMutex() Failure, nRet(%d) ..", nRet);
		EndProcess();
		return -2;
	}

	m_hThread = ::CreateThread(NULL, 0, RunThread, this, CREATE_SUSPENDED, &dwThreadID);
	if(NULL == m_hThread)
	{
		LOG_OUT("CreateThread() Failure, nRet(%d) ..", nRet);
		EndProcess();
		return -3;
	}

	::ResumeThread(m_hThread);

	return 0;
}

/**
 * @brief		EndProcess
 * @details		Start
 * @param		int nCommIdx		COM
 * @return		성공 : > 0, 실패 : < 0
 */
int CScannerWT::EndProcess(void)
{
	if(m_bConnected == TRUE)
	{
		m_bConnected = FALSE;

		if(NULL != m_hThread)
		{
			::WaitForSingleObject(m_hThread, 500);
			::CloseHandle(m_hThread);
			m_hThread = NULL;
		}

		if(WT_IsDevice() == WT_SUCCESS)
		{
			WT_DeviceClose();
		}

		if(m_hAccMutex != NULL)
		{
			CloseHandle(m_hAccMutex);
			m_hAccMutex = NULL;
		}
	}

	return 0;
}

/**
 * @brief		RunThread
 * @details		thread 동작
 * @param		LPVOID lParam		Instance
 * @return		항상 = 0
 */
DWORD CScannerWT::RunThread(LPVOID lParam)
{
	DWORD	dwTick;
	CScannerWT *pScanner = (CScannerWT *)lParam;

	pScanner->Initialize();

	pScanner->m_bConnected = TRUE;

	dwTick = ::GetTickCount();
	while(pScanner->m_bConnected)
	{
		Sleep(10);

		if(pScanner->m_bScan != FALSE)
		{
			if( Util_CheckExpire(dwTick) >= 200 )	
			{
				//pScanner->OnScanning();
				dwTick = ::GetTickCount();
			}
		}
	}

	return 0;
}


