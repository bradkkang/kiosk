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

	/// scan on
	nRet = ATEC_ScannerOn();

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
	
	Locking();
	{
//		nRet = WT_ScannerOn();
		m_bScan = TRUE;
		ATEC_ScannerOn();
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
int CScannerAtec::Disable(void)
{
	int nRet = 0;
	
	{
		m_bScan = FALSE;
		ATEC_ScannerOff();
	}

	return nRet;
}

/**
 * @brief		GetStatus
 * @details		get status info
 * @param		None
 * @return		성공 : = 0, 실패 : != 0
 */
int CScannerAtec::GetStatus(void)
{
	int nRet;
	BYTE szStatus[20];
	
	::ZeroMemory(szStatus, sizeof(szStatus));
	nRet = ATEC_GetStatus(szStatus);

	return nRet;
}

/**
 * @brief		Eject
 * @details		승차권 입수
 * @param		None
 * @return		성공 : = 0, 실패 : != 0
 */
int CScannerAtec::Eject(void)
{
	int nRet;
	
// 	m_nPaperOutSpeed = 6;
// 	WT_SetPaperOutSpeed(m_nPaperOutSpeed);

	nRet = ATEC_Inject(1);
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
	int nRet;

	//m_nPaperOutSpeed = 2;
	//WT_SetPaperOutSpeed(m_nPaperOutSpeed);

	nRet = ATEC_Reject();
	return nRet;
}

int CScannerAtec::OnBarcode(char *rawfile, int TopOrBottom, int height)
{
	int				nRet, nWidth;
	char			barcodedata[128] = {0,};
	int				barcode_type;
	unsigned short	LastPosY;

	LOG_OUT("start !!");

	nWidth = 832;
	barcode_type = ATEC_Barcode(rawfile, nWidth, height, barcodedata, &LastPosY);
	LOG_OUT("ATEC_Barcode(), nRet(%d) !!", nRet);
	if (barcode_type > 0)
	{
		if (TopOrBottom == 1)
		{
			LOG_OUT("[Front] BarCode = [%s], BarCode_Type = [%d] !", barcodedata, barcode_type);
			sprintf(m_szBarCodeText, "%s", barcodedata);
			//m_BarCode_front = str;
			m_bBbarcode_Ok = TRUE;
		}
		else
		{
			LOG_OUT("[Back] BarCode = [%s], BarCode_Type = [%d] !", barcodedata, barcode_type);
			sprintf(m_szBarCodeText, "%s", barcodedata);
			//m_BarCode_back = str;
			m_bBbarcode_Ok = FALSE;
		}
		nRet = 1;
	}
	else
	{
		if (TopOrBottom == 1)
		{
			LOG_OUT("Barcode Front Fail !!!");
			//m_BarCode_front = str;
			m_bBbarcode_Ok = FALSE;
		}
		else
		{
			LOG_OUT("Barcode Back Fail !!!");
			//m_BarCode_back = str;
			m_bBbarcode_Ok = FALSE;
		}
		nRet = -2;
	}

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
	int	nRet;
	int	nWidth;
	int	nHeight;
	int nFileSize;
	BYTE szStatus[20];
	char *FrontRawBuf;
	char *RearRawBuf;
	
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

	//// scan state & scan done
	if ((szStatus[0] == 0x03) && (szStatus[1] == 0x03))
	{
		m_nScanline = (szStatus[11] << 8) | szStatus[10];

		delete m_Imagefront.pbuf;
		delete m_ImageBack.pbuf;

		/* raw data image를 파일로 파일로 저장 */
		//nWidth		= 688;
		nWidth			= 832;
		nFileSize	= (m_nScanline * nWidth + 6) + (108032 - ((m_nScanline * nWidth + 6) % 108032))+1024;
		LOG_OUT("m_nScanline = %d, nWidth = %d",m_nScanline,nWidth);
		FrontRawBuf	= new char[nFileSize];
		RearRawBuf	= new char[nFileSize];

		LOG_OUT("ATEC_ScanBuf_1");

		ATEC_ScanBuf(m_nScanline, FrontRawBuf, RearRawBuf);
		LOG_OUT("ATEC_ScanBuf_2");
		nRet = OnBarcode(FrontRawBuf, 1, m_nScanline);
		LOG_OUT("ATEC_ScanBuf_3, nRet(%d)", nRet);
		if(nRet < 0)
		{
			nRet = OnBarcode(RearRawBuf, 0, m_nScanline);
			if(nRet < 0)
			{
				nRet = -99;
			}
		}

		LOG_OUT("ATEC_ScanBuf_4");
		nHeight = m_nScanline;

		m_Imagefront.pbuf = (UCHAR *)new UCHAR[nWidth * nHeight];
		LOG_OUT("ATEC_ScanBuf_5");
		m_ImageBack.pbuf  = (UCHAR *)new UCHAR[nWidth * nHeight];
		LOG_OUT("ATEC_ScanBuf_6");

		LOG_OUT("ATEC_ScanBuf_7");
		/* raw data 파일을 읽어 BMP파일로 저장 */
		ATEC_SaveBMP(832, m_nScanline, FrontRawBuf, m_szFileName0);
		LOG_OUT("ATEC_ScanBuf_8");
		ATEC_SaveBMP(832, m_nScanline, RearRawBuf, m_szFileName1);
		LOG_OUT("ATEC_ScanBuf_9");

		delete(FrontRawBuf);
		LOG_OUT("ATEC_ScanBuf_10");
		delete(RearRawBuf);
		LOG_OUT("ATEC_ScanBuf_11");
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
int CScannerAtec::StartProcess(int nCommIdx)
{
	int		nRet;
	DWORD dwThreadID;
	char szBasePath[256];

	LOG_INIT();

	m_bConnected = FALSE;

	LOG_OUT("start ###############################");


	::ZeroMemory(szBasePath, sizeof(szBasePath));
	Util_GetModulePath(szBasePath);

	sprintf(m_szFileName0, "%s%s", szBasePath, "\\atec_front.bmp");
	sprintf(m_szFileName1, "%s%s", szBasePath, "\\atec_rear.bmp");

	EndProcess();

	ATEC_DeviceClose();

	nRet = ATEC_DeviceOpen(0);
	if(nRet != 0)
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

		if(ATEC_IsDevice() != 0)
		{
			ATEC_DeviceClose();
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
				//pScanner->OnScanning();
				dwTick = ::GetTickCount();
			}
		}
	}

	return 0;
}


