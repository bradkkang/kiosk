// 
// 
// dev_scanner_atec.cpp : 승차권 스캐너 (ATEC T&)
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include <usrinc/fbuf.h>
#include "xzzbus_fdl.h"

#include "MyDefine.h"
#include "MyUtil.h"

#include "AtecScanLib.h"
#include "BmpStruct.h"
#include "dev_scanner_atec.h"
#include "dev_ui_main.h"
#include "dev_tr_main.h"
#include "dev_tr_mem.h"
#include "event_if.h"

//----------------------------------------------------------------------------------------------------------------------

#define LOG_OUT(fmt, ...)		{ CScannerAtec::m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ CScannerAtec::m_clsLog.LogOut("[%s:%d] ", __FUNCTION__, __LINE__);  CScannerAtec::m_clsLog.HexaDump(x, y, z); }

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CScannerAtec
 * @details		생성자
 */
CScannerAtec::CScannerAtec()
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

	// add
	::ZeroMemory(&m_Imagefront, sizeof(IMAGE_INFO));
	::ZeroMemory(&m_ImageBack, sizeof(IMAGE_INFO));
}

/**
 * @brief		~CScannerAtec
 * @details		소멸자
 */
CScannerAtec::~CScannerAtec()
{
	EndProcess();
}

/**
 * @brief		LOG_INIT
 * @details		LOG 파일 초기화
 * @param		None
 * @return		None
 */
void CScannerAtec::LOG_INIT(void)
{
	m_clsLog.SetData(30, "\\Log\\ScannerATEC");
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
int CScannerAtec::Locking(void)
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
int CScannerAtec::UnLocking(void)
{
	if(m_hAccMutex != NULL) 
	{
		::ReleaseMutex(m_hAccMutex);
	}

	return 0;	
}

/**
 * @brief		Initialize
 * @details		초기 동작
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CScannerAtec::Initialize(void)
{
#if (_USE_ATEC_SCANNER_ > 0)
	int nRet;
	BYTE Buffer[100];
	
	/// reset
	nRet = ATEC_Reset(0);
		
	/// get version
	::ZeroMemory(Buffer, sizeof(Buffer));
	nRet = ATEC_GetFirmwareVersion(Buffer);
	LOG_OUT("Version = [%c.%c%c%c] !!", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);

	/// get serial number
	::ZeroMemory(Buffer, sizeof(Buffer));
	nRet = ATEC_GetSerial(Buffer);
	LOG_OUT("Serial Number = [%s] !!", Buffer);

	/// scan off
	nRet = ATEC_ScannerOff();
#endif
	return 0;
}

/**
 * @brief		Enable
 * @details		Scanner enable
 * @param		None
 * @return		성공 : = 0, 실패 : != 0
 */
int CScannerAtec::Enable(void)
{
	int nRet = 0;

	LOG_OUT("start !!!");

#if (_USE_ATEC_SCANNER_ > 0)
	Locking();
	{
//		nRet = WT_ScannerOn();
		m_bScan = TRUE;
		ATEC_ScannerOn(0);
	}
	UnLocking();
#endif
	return nRet;
}

/**
 * @brief		Disable
 * @details		Scanner disable
 * @param		None
 * @return		성공 : = 0, 실패 : != 0
 */
int CScannerAtec::Disable(void)
{
	int nRet = 0;
	
	LOG_OUT("start !!!");

#if (_USE_ATEC_SCANNER_ > 0)
	Locking();
	{
		m_bScan = FALSE;
		ATEC_ScannerOff();
	}
	UnLocking();
#endif

	return nRet;
}

/**
 * @brief		GetStatus
 * @details		get status info
 * @param		None
 * @return		성공 : = 0, 실패 : != 0
 */
int CScannerAtec::GetStatus(int *nStatus)
{
	int nRet = -1;
	BYTE szStatus[20];
	
#if (_USE_ATEC_SCANNER_ > 0)
	::ZeroMemory(szStatus, sizeof(szStatus));
	nRet = ATEC_GetStatus(szStatus);
	if(nRet != 0)
	{
		return -1;
	}

	*nStatus = 0;
	//LOG_HEXA("ATEC_GetStatus", szStatus, sizeof(szStatus));
	::CopyMemory(nStatus, &szStatus[4], 2);
#endif

	return nRet;
}

/**
 * @brief		GetDeviceStatus
 * @details		승차권 GetStatus
 * @param		None
 * @return		성공 : >=0, 실패 : < 0
 */

int CScannerAtec::GetDeviceStatus(void)
{
#if (_USE_ATEC_SCANNER_ > 0)
	int nRet = 0;
	BYTE szStatus[20];

	nRet = ATEC_IsDevice();
	//LOG_OUT("ATEC_IsDevice ret = [%d] !!", nRet);
	if( nRet != 0 )
	{
		nRet = ATEC_DeviceOpen(0);
		if(nRet != 0 )
		{
			SetCheckEventCode(EC_SCAN_COMM_ERR, TRUE);//
			return nRet; 
		}
	} 
	
	ZeroMemory(szStatus, sizeof(szStatus));
	nRet = ATEC_GetStatus(szStatus);
	//LOG_HEXA("GetDeviceStatus_ATEC_GetStatus", szStatus, sizeof(szStatus));
	if(nRet != 0)	
	{
		SetCheckEventCode(EC_SCAN_COMM_ERR, TRUE);
		return -2;
	}
	SetCheckEventCode(EC_SCAN_COMM_ERR, FALSE);
#endif

	return 0;
}

/**
 * @brief		Eject
 * @details		승차권 입수
 * @param		None
 * @return		성공 : = 0, 실패 : != 0
 */
int CScannerAtec::Eject(void)
{
	int nRet = 0;

#if (_USE_ATEC_SCANNER_ > 0)
	nRet = ATEC_Inject(1);
	LOG_OUT("[%s], nRet(%d)..", __FUNCTION__, nRet);//로그 추가
#endif
	return nRet;
}

/**
 * @brief		Reject
 * @details		승차권 배출
 * @param		None
 * @return		성공 : = 0, 실패 : != 0
 */
int CScannerAtec::Reject(void)
{
	int nRet = 0;

#if (_USE_ATEC_SCANNER_ > 0)
	nRet = ATEC_Reject();
	LOG_OUT("[%s], nRet(%d)..", __FUNCTION__, nRet);//로그추가
#endif

	return nRet;
}

int CScannerAtec::OnBarcode(char *rawfile, int TopOrBottom, int height)
{
	int				nRet = 0;
#if (_USE_ATEC_SCANNER_ > 0)
	char			barcodedata[128] = {0,};
	int				barcode_type;
	unsigned short	LastPosY;

	barcode_type = ATEC_Barcode((BYTE *)rawfile, ATEC_SCAN_WIDTH, height, barcodedata, &LastPosY);
	m_barcode_type = barcode_type;
	LOG_OUT("Barcode_type = (%d) ..", barcode_type);

	::ZeroMemory(m_szBarCodeText, sizeof(m_szBarCodeText));

	if (barcode_type > 0)
	{
		sprintf(m_szBarCodeText, "%s", barcodedata);
		LOG_OUT("BarCodeData = [%s], BarCode_Type = [%d] !", barcodedata, barcode_type);
		nRet = 1;
	}
	else
	{
		LOG_OUT("Barcode Recog Fail !!!");
		nRet = -99;
	}
#endif

	return nRet;
}

/**
 * @brief		OnScanning
 * @details		승차권 Scanning
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CScannerAtec::OnScanning(void)
{
#if (_USE_ATEC_SCANNER_ > 0)
	int		nRet, nWidth, nHeight, nFileSize;
	BYTE	szStatus[20];
	static BYTE	s_PrevStatus[20] = {0, };
	char	*FrontRawBuf;
	char	*RearRawBuf;
	
	nRet = nWidth = nHeight = nFileSize = 0;

	nRet = ATEC_IsDevice();
	if(nRet != 0)	
	{
		LOG_OUT("ATEC_IsDevice() Error, nRet(%d) !!", nRet);
		return -1;
	}

	::ZeroMemory(szStatus, sizeof(szStatus));
	nRet = ATEC_GetStatus(szStatus);
	if(nRet != 0)
	{
		return -2;
	}

#if 1
	if( memcmp(s_PrevStatus, szStatus, 16) )
	{
		LOG_HEXA("ATEC_GetStatus", szStatus, sizeof(szStatus));
		::CopyMemory(s_PrevStatus, szStatus, sizeof(szStatus));
	}
#else
	LOG_HEXA("ATEC_GetStatus", szStatus, sizeof(szStatus));
#endif

	//// scan state & scan done
	if ((szStatus[0] == 0x03) && (szStatus[1] == 0x03))
	{
		m_nScanline = (szStatus[11] << 8) | szStatus[10];
		
		// raw data image를 파일로 파일로 저장
		nWidth		= ATEC_SCAN_WIDTH;
		//nFileSize	= (m_nScanline * nWidth + 6) + (108032 - ((m_nScanline * nWidth + 6) % 108032))+1024;
		nFileSize	= (m_nScanline * nWidth) + 1024;
		LOG_OUT("m_nScanline = %d, nWidth = %d", m_nScanline, nWidth);
		FrontRawBuf	= new char[nFileSize];
		RearRawBuf	= new char[nFileSize];

		LOG_OUT("OnScanning#1, ATEC_ScanBuf()..");

		ATEC_ScanBuf(m_nScanline, (BYTE *)FrontRawBuf, (BYTE *)RearRawBuf);
		LOG_OUT("OnScanning#2, OnBarcode()..");
		nRet = OnBarcode(FrontRawBuf, 1, m_nScanline);
		LOG_OUT("OnScanning#3, OnBarcode(), nRet(%d)..", nRet);
		if(nRet < 0)
		{
			nRet = -99;
		}
		
		ATEC_SaveBMP(ATEC_SCAN_WIDTH, m_nScanline, (BYTE *)FrontRawBuf, m_szFileName0, m_barcode_type);

		delete(FrontRawBuf);
		delete(RearRawBuf);
	}
	else
	{
		nRet = -3;
	}

	return nRet;
#else
	return 0;
#endif
}

/**
 * @brief		Branding
 * @details		소인 출력
 * @param		None
 * @return		성공 : = 0, 실패 : != 0
 */
int CScannerAtec::Branding(BYTE dir, short PositionY)
{
	int nRet = 0;
	//PositionY = 650;

#if (_USE_ATEC_SCANNER_ > 0)
	nRet = ATEC_TPHBranding(dir, (unsigned short)PositionY);
	LOG_OUT("[%s], nRet(%d) PositionY(%d)..", __FUNCTION__, nRet, PositionY);
#endif

	return nRet;
}

/**
 * @brief		StartProcess
 * @details		Start
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CScannerAtec::StartProcess(int nCommIdx)
{
	int nRet = 0;
	DWORD dwThreadID;
	char szBasePath[256];

	LOG_INIT();

	m_bConnected = FALSE;

	LOG_OUT("start ###############################");

	::ZeroMemory(szBasePath, sizeof(szBasePath));
	Util_GetModulePath(szBasePath);

	// ex) 20210215-101010-barcode_type.bmp
	//sprintf(m_szFileName0, "%s%s", szBasePath, "\\atec_front.bmp");
	//sprintf(m_szFileName1, "%s%s", szBasePath, "\\atec_rear.bmp");
	sprintf(m_szFileName0, "%s\\image_scan\\", szBasePath);
	sprintf(m_szFileName1, "%s\\image_scan\\", szBasePath);

	EndProcess();

#if (_USE_ATEC_SCANNER_ > 0)
	ATEC_DeviceClose();
	nRet = ATEC_DeviceOpen(0);
	if(nRet != 0)
	{
		LOG_OUT("WT_DeviceOpen() Failure, nRet(%d) ..", nRet);
		EndProcess();
		return -1;
	}
#endif

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
int CScannerAtec::EndProcess(void)
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

#if (_USE_ATEC_SCANNER_ > 0)
		if(ATEC_IsDevice() != 0)
		{
			ATEC_DeviceClose();
		}
#endif
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
DWORD CScannerAtec::RunThread(LPVOID lParam)
{
	DWORD	dwTick;
	CScannerAtec *pScanner = (CScannerAtec *)lParam;

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
				dwTick = ::GetTickCount();
			}
		}
	}

	return 0;
}


