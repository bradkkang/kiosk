//   file	: FtpComm.cpp
//	 Author : Softneoul Inc.
//	 history
//	 2009-06-04		first create
//

#include "stdafx.h"
#include <wininet.h>		// 
#include <queue>
#include <fstream>
#include <iostream>
#include <vector>

#include "FtpComm.h"

//------------------------------------------------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------------------------------------------------
#define __USE_FTP_THREAD__	1


//------------------------------------------------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------------------------------------------------

static DWORD				ThreadID;
static HANDLE				hFtpThread = NULL;
static HANDLE				hAccMutex = NULL;
static HANDLE				hEventKill = NULL;
static FTP_QUE				FtpQue;
static CFtpLogFile			clsLog;

//------------------------------------------------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------------------------------------------------
#define LOG_OUT(fmt, ...)		{ clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ clsLog.HexaDump(x, y, z); }

//------------------------------------------------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------------------------------------------------

static DWORD WINAPI FtpCommThread(void *);

/**
 * @brief		LOG_INIT
 * @details		LOG 초기화
 * @param		None
 * @return		항상 = 0
 */
static int LOG_INIT(void)
{
	clsLog.SetData(30, "\\Log\\Ftp");
	clsLog.Initialize();
	clsLog.Delete();

	return 0;
}

/**
 * @brief		
 * @details		
 * @param		None
 * @return		항상 = 0
 */
static int Locking(void)
{
	if(hAccMutex != 0) {
		::WaitForSingleObject(hAccMutex, INFINITE);
	}

	return 0;	
}

/**
 * @brief		
 * @details		
 * @param		None
 * @return		항상 = 0
 */
static int UnLocking(void)
{
	if(hAccMutex != 0) {
		::ReleaseMutex(hAccMutex);
	}

	return 0;	
}

/**
 * @brief		
 * @details		
 * @param		None
 * @return		항상 = 0
 */
int FTP_Uploading(void)
{
	HINTERNET	m_hOpen;
	HINTERNET	m_hConnect;

	Locking();

	m_hOpen = InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
	if(NULL == m_hOpen) 
	{
		LOG_OUT("InternetOpen error..");
		goto failure;
	}

	m_hConnect = InternetConnect(m_hOpen, _T("100.100.100.1"), INTERNET_DEFAULT_FTP_PORT, 
								_T("USER_ID"), _T("USER_PASSWORD"), INTERNET_SERVICE_FTP, 0, 0);
	if(NULL == m_hConnect)
	{
		LOG_OUT("InternetConnect error..");
		InternetCloseHandle(m_hOpen);
		goto failure;
	}

	// FTP Set Current Directory
	if(!FtpSetCurrentDirectory(m_hConnect, _T("/DirName")))
	{
		InternetCloseHandle(m_hConnect);
		InternetCloseHandle(m_hOpen);
		goto failure;
	}

	if(!FtpPutFile(m_hConnect, _T("Test.Dat"), _T("C:\\Test.Dat"), INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD, 0))
	{
		InternetCloseHandle(m_hConnect);
		InternetCloseHandle(m_hOpen);
		goto failure;
	}

	// FTP Close
	if(m_hOpen) 
	{
		InternetCloseHandle(m_hOpen);
		m_hOpen = NULL;
	}

	if(m_hConnect) 
	{
		InternetCloseHandle(m_hConnect);
		m_hConnect = NULL;
	}
	// FTP Close End
	UnLocking();
	return 0;

failure:

	UnLocking();
	return -1;
}

/**
 * @brief		
 * @details		
 * @param		None
 * @return		항상 = 0
 */
int FTP_Downloading(void *p)
{
	int			ret, nOkConfirm, nNgConfirm;
	BOOL		bResult;
	HINTERNET	m_hOpen;
	HINTERNET	m_hConnect;
	char		srcf[256], desf[256], fileName[256];
	FTP_PARAM_T *pFtpInfo;

	pFtpInfo = (FTP_PARAM_T *) p;

	m_hOpen = InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
	if(NULL == m_hOpen) 
	{
		LOG_OUT("InternetOpen error..");
		goto failure;
	}

	m_hConnect = InternetConnectA(m_hOpen, pFtpInfo->szIp, INTERNET_DEFAULT_FTP_PORT, pFtpInfo->szID, pFtpInfo->szPwd, INTERNET_SERVICE_FTP, 0, 0);
	if(NULL == m_hConnect) 
	{
		LOG_OUT("InternetConnect error..");
		InternetCloseHandle(m_hOpen);
		goto failure;
	}

	// FTP Set Current Directory
	if(!FtpSetCurrentDirectoryA(m_hConnect, pFtpInfo->szRemotePath)) 
	{
		LOG_OUT("FtpSetCurrentDirectory error..");
		InternetCloseHandle(m_hConnect);
		InternetCloseHandle(m_hOpen);
		goto failure;
	}

	if(!::FtpGetFileA(m_hConnect, pFtpInfo->szRemoteFile, pFtpInfo->szLocalFile, FALSE , FILE_ATTRIBUTE_NORMAL, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD, 0)) 
	{
		DWORD dwRet = GetLastError();
		LOG_OUT("FtpGetFile error(%d)..", dwRet);
		InternetCloseHandle(m_hConnect);
		InternetCloseHandle(m_hOpen);
		goto failure;
	}

	// FTP Close
	if(m_hOpen) 
	{
		InternetCloseHandle(m_hOpen);
		m_hOpen = NULL;
	}

	if(m_hConnect) 
	{
		InternetCloseHandle(m_hConnect);
		m_hConnect = NULL;
	}

	//LOG_OUT("cmdIndex(%d), confirmIndex(%d)..\n", __FUNCTION__, __LINE__, pFtpInfo->cmdIndex, pFtpInfo->confirmIndex);

	/**
	switch(pFtpInfo->cmdIndex) {
	case RCMD_ALL_STATION_INFO:		// (01). 통합 역 정보		[0xB2, 0x20]
		{
			nOkConfirm = SCMD_ALL_STATION_CONFIRM;
			nNgConfirm = SCMD_ALL_STATION_NG_CONFIRM;

			sprintf(fileName, "%s%s", GetAbsolutePath(), GetDatamngPath(pFtpInfo->cmdIndex));
			LOG_OUT("[%s:%d] fileName(%s)..\n", __FUNCTION__,__LINE__,fileName);
			ret = CompareFileLength(pFtpInfo->cmdIndex, fileName, sizeof(RCV_FAREFILE), pFtpInfo->szVersion);
			if(ret < 0) {
				InsertSendQueue(nNgConfirm, 0);
				break;
			}

			sprintf(desf, "%s\\Env%s", GetAbsolutePath(), ALL_STATION_NAME);
			LOG_OUT("[%s:%d] desf(%s)..\n", __FUNCTION__,__LINE__,desf);
			bResult = CopyFileA(fileName, desf, FALSE);
			if(bResult == FALSE) {
				InsertSendQueue(nNgConfirm, 0);
				break;
			}

			//ReadFareDataFile();
			InsertSendQueue(nOkConfirm, 0);
		}
		break;

	case RCMD_BASE_INFO:			// (02). 기초정보			[0xB2, 0x22]
		{
			nOkConfirm = SCMD_BASE_CONFIRM;
			nNgConfirm = SCMD_BASE_NG_CONFIRM;

			sprintf(fileName, "%s%s", GetAbsolutePath(), GetDatamngPath(pFtpInfo->cmdIndex));
			LOG_OUT("[%s:%d] fileName(%s)..\n", __FUNCTION__,__LINE__,fileName);
			ret = CompareFileLength(pFtpInfo->cmdIndex, fileName, sizeof(RCV_BASEDATA), pFtpInfo->szVersion);
			if(ret < 0) {
				InsertSendQueue(nNgConfirm, 0);
				break;
			}

			sprintf(desf, "%s\\Env%s", GetAbsolutePath(), BASEINFOFILE_NAME);
			bResult = CopyFileA(fileName, desf, FALSE);
			LOG_OUT("[%s:%d] fileName(%s), desf(%s), bResult(%d)..\n", __FUNCTION__, __LINE__, fileName, desf, bResult);
			if(bResult == FALSE) {
				InsertSendQueue(nNgConfirm, 0);
				break;
			}

			ReadBaseInfoFile();
			FireStaffStationWnd(UM_STATION_MESSAGE, (char *)NULL);
			InsertSendQueue(nOkConfirm, 0);
		}
		break;

	case RCMD_FARE_INFO:			// (03). 운임정보			[0xB2, 0x23]
		{
			nOkConfirm = SCMD_FARE_CONFIRM;
			nNgConfirm = SCMD_FARE_NG_CONFIRM;

			sprintf(fileName, "%s%s", GetAbsolutePath(), GetDatamngPath(pFtpInfo->cmdIndex));
			LOG_OUT("[%s:%d] fileName(%s)..\n", __FUNCTION__,__LINE__,fileName);
			ret = CompareFileLength(pFtpInfo->cmdIndex, fileName, sizeof(RCV_FAREFILE), pFtpInfo->szVersion);
			if(ret < 0) {
				InsertSendQueue(nNgConfirm, 0);
				break;
			}

			sprintf(desf, "%s\\Env%s", GetAbsolutePath(), FAREINFOFILE_NAME);
			LOG_OUT("[%s:%d] desf(%s)..\n", __FUNCTION__,__LINE__,desf);
			bResult = CopyFileA(fileName, desf, FALSE);
			if(bResult == FALSE) {
				InsertSendQueue(nNgConfirm, 0);
				break;
			}

			ReadFareDataFile();
			InsertSendQueue(nOkConfirm, 0);
		}
		break;

	case RCMD_KB_BL_MASTER_INFO:	// (04). B/L 마스터			[0xB2, 0x25]
		{
			nOkConfirm = SCMD_KB_BL_MASTER_CONFIRM;
			nNgConfirm = SCMD_KB_BL_MASTER_NG_CONFIRM;

			sprintf(fileName, "%s%s", GetAbsolutePath(), GetDatamngPath(pFtpInfo->cmdIndex));
			LOG_OUT("[%s:%d] fileName(%s)..\n", __FUNCTION__,__LINE__,fileName);
			ret = CompareFileLength(pFtpInfo->cmdIndex, fileName, pFtpInfo->dwSize, pFtpInfo->szVersion);
			if(ret < 0) {
				InsertSendQueue(nNgConfirm, 0);
				break;
			}

			sprintf(desf, "%s\\Env%s", GetAbsolutePath(), KB_BL_NAME);
			LOG_OUT("[%s:%d] desf(%s)..\n", __FUNCTION__,__LINE__,desf);
			ret = UnPack(fileName, desf);
			if( ret < 0 ) {
				LOG_OUT("[%s:%d] desf(%s)..\n", __FUNCTION__,__LINE__,desf);
				InsertSendQueue(nNgConfirm, 0);
				break;
			}
			InsertSendQueue(nOkConfirm, 0);
		}
		break;

	case RCMD_PROGRAM_INFO:			// (05). 프로그램 정보		[0xB2, 0x28]
		{
			nOkConfirm = SCMD_PROGRAM_CONFIRM;
			nNgConfirm = SCMD_PROGRAM_NG_CONFIRM;

			//UnicodeToChar(strLocalFile, srcf, sizeof(srcf));
			sprintf(srcf, "%s", pFtpInfo->szLocalFile);
			sprintf(desf, "%s\\Tmp%s", GetAbsolutePath(), DAXVM_PROGRAM_NAME);

			sprintf(fileName, "%s%s", GetAbsolutePath(), GetDatamngPath(pFtpInfo->cmdIndex));
			LOG_OUT("[%s:%d] fileName(%s)..\n", __FUNCTION__,__LINE__,fileName);
			ret = CompareFileLength(pFtpInfo->cmdIndex, srcf, pFtpInfo->dwSize, pFtpInfo->szVersion);
			if(ret < 0) {
				InsertSendQueue(nNgConfirm, 0);
				break;
			}

			LOG_OUT("srcf : %s, desf : %s\n", srcf, desf);
			ret = CopySmsFile(srcf, desf);
			LOG_OUT("CopySmsFile Write : %d\n", ret);
			if(ret > 0) {
				//SetEventCode(DEV_EC_PROGRAM_DOWN_OK);
				// FTP Close End
				InsertSendQueue(nOkConfirm, 0);
			} else {
				InsertSendQueue(nNgConfirm, 0);
			}
		}
		break;

	case RCMD_STAFF_BL_INFO:		// (06). 직원권 B/L			[0xB2, 0x2B]
		{
			nOkConfirm = SCMD_STAFF_BL_CONFIRM;
			nNgConfirm = SCMD_STAFF_BL_NG_CONFIRM;

			sprintf(fileName, "%s%s", GetAbsolutePath(), GetDatamngPath(pFtpInfo->cmdIndex));
			LOG_OUT("[%s:%d] fileName(%s)..\n", __FUNCTION__,__LINE__,fileName);
			ret = CompareFileLength(pFtpInfo->cmdIndex, fileName, pFtpInfo->dwSize, pFtpInfo->szVersion);
			if(ret < 0) {
				InsertSendQueue(nNgConfirm, 0);
				break;
			}

			sprintf(desf, "%s\\Env%s", GetAbsolutePath(), STAFF_BL_NAME);
			LOG_OUT("[%s:%d] desf(%s)..\n", __FUNCTION__,__LINE__,desf);
			ret = UnPack(fileName, desf);
			if( ret < 0 ) {
				LOG_OUT("[%s:%d] desf(%s)..\n", __FUNCTION__,__LINE__,desf);
				InsertSendQueue(nNgConfirm, 0);
				break;
			}
			InsertSendQueue(nOkConfirm, 0);
		}
		break;

	case RCMD_BIN_INFO:				// (07). BIN MASTETR		[0xB2, 0x30]
		{
			nOkConfirm = SCMD_BIN_CONFIRM;
			nNgConfirm = SCMD_BIN_NG_CONFIRM;

			sprintf(fileName, "%s%s", GetAbsolutePath(), GetDatamngPath(pFtpInfo->cmdIndex));
			LOG_OUT("[%s:%d] fileName(%s)..\n", __FUNCTION__,__LINE__,fileName);
			ret = CompareFileLength(pFtpInfo->cmdIndex, fileName, pFtpInfo->dwSize, pFtpInfo->szVersion);
			if(ret < 0) {
				InsertSendQueue(nNgConfirm, 0);
				break;
			}

			sprintf(desf, "%s\\Env%s", GetAbsolutePath(), BL_BIN_NAME);
			LOG_OUT("[%s:%d] desf(%s)..\n", __FUNCTION__,__LINE__,desf);
			ret = UnPack(fileName, desf);
			if( ret < 0 ) {
				LOG_OUT("[%s:%d] desf(%s)..\n", __FUNCTION__,__LINE__,desf);
				InsertSendQueue(nNgConfirm, 0);
				break;
			}
			InsertSendQueue(nOkConfirm, 0);
		}
		break;

	case RCMD_LG_BL_MASTER_INFO:	// (08). LG 복지카드 B/L	[0xB2, 0x4A]
		{
			nOkConfirm = SCMD_LG_BL_MASTER_CONFIRM;
			nNgConfirm = SCMD_LG_BL_MASTER_NG_CONFIRM;

			sprintf(fileName, "%s%s", GetAbsolutePath(), GetDatamngPath(pFtpInfo->cmdIndex));
			LOG_OUT("[%s:%d] fileName(%s)..\n", __FUNCTION__,__LINE__,fileName);
			ret = CompareFileLength(pFtpInfo->cmdIndex, fileName, pFtpInfo->dwSize, pFtpInfo->szVersion);
			if(ret < 0) {
				InsertSendQueue(nNgConfirm, 0);
				break;
			}

			sprintf(desf, "%s\\Env%s", GetAbsolutePath(), LG_BL_NAME);
			LOG_OUT("[%s:%d] desf(%s)..\n", __FUNCTION__,__LINE__,desf);
			ret = UnPack(fileName, desf);
			if( ret < 0 ) {
				LOG_OUT("[%s:%d] desf(%s)..\n", __FUNCTION__,__LINE__,desf);
				InsertSendQueue(nNgConfirm, 0);
				break;
			}
			InsertSendQueue(nOkConfirm, 0);
		}
		break;

	case RCMD_TOT_BL_MASTER_INFO:	// (09). 통합 B/L			[0xB2, 0x4C]
		{

		}
		break;

	case RCMD_TOT_PL_MASTER_INFO:	// (10). 통합 P/L			[0xB2, 0x4D]
		{

		}
		break;

	case RCMD_UCARD_FARE_INFO:		// (11). 신교통카드 운임정보[0xB2, 0x5A]
		{
			nOkConfirm = SCMD_UCARD_FARE_CONFIRM;
			nNgConfirm = SCMD_UCARD_FARE_NG_CONFIRM;

			sprintf(fileName, "%s%s", GetAbsolutePath(), GetDatamngPath(pFtpInfo->cmdIndex));
			LOG_OUT("[%s:%d] fileName(%s)..\n", __FUNCTION__,__LINE__,fileName);
			ret = CompareFileLength(pFtpInfo->cmdIndex, fileName, pFtpInfo->dwSize, pFtpInfo->szVersion);
			if(ret < 0) {
				InsertSendQueue(nNgConfirm, 0);
				break;
			}

			sprintf(desf, "%s\\Env%s", GetAbsolutePath(), UCARD_FARE_NAME);
			LOG_OUT("[%s:%d] desf(%s)..\n", __FUNCTION__,__LINE__,desf);
			bResult = CopyFileA(fileName, desf, FALSE);
			if(bResult == FALSE) {
				InsertSendQueue(nNgConfirm, 0);
				break;
			}

			//ReadFareDataFile();
			InsertSendQueue(nOkConfirm, 0);
		}
		break;

	case RCMD_STAT_DISTANCE_INFO:	// (12). 역간거리 정보		[0xB2, 0x5B]
		{
			nOkConfirm = SCMD_UCARD_FARE_CONFIRM;
			nNgConfirm = SCMD_UCARD_FARE_NG_CONFIRM;

			sprintf(fileName, "%s%s", GetAbsolutePath(), GetDatamngPath(pFtpInfo->cmdIndex));
			LOG_OUT("[%s:%d] fileName(%s)..\n", __FUNCTION__,__LINE__,fileName);
			ret = CompareFileLength(pFtpInfo->cmdIndex, fileName, pFtpInfo->dwSize, pFtpInfo->szVersion);
			if(ret < 0) {
				InsertSendQueue(nNgConfirm, 0);
				break;
			}

			sprintf(desf, "%s\\Env%s", GetAbsolutePath(), STAT_DISTANCE_NAME);
			LOG_OUT("[%s:%d] desf(%s)..\n", __FUNCTION__,__LINE__,desf);
			bResult = CopyFileA(fileName, desf, FALSE);
			if(bResult == FALSE) {
				InsertSendQueue(nNgConfirm, 0);
				break;
			}

			//ReadFareDataFile();
			InsertSendQueue(nOkConfirm, 0);
		}
		break;

	case RCMD_ANIMATION_INFO:		// (13). 동영상 정보		[0xB2, 0x5D]
		{
			nOkConfirm = SCMD_ANIMATION_CONFIRM;
			nNgConfirm = SCMD_ANIMATION_NG_CONFIRM;

			sprintf(srcf, "%s", pFtpInfo->szLocalFile);
			LOG_OUT("[%s:%d] fileName(%s)..\n", __FUNCTION__,__LINE__,srcf);

			switch(GetDevType()) {
			case DEV_ATVM:
				sprintf(desf, "%s\\Mov\\%s", ATVM_BIN_PATH, pFtpInfo->szRemoteFile);
				break;
			case DEV_HVM:
			case DEV_NHVM:
				sprintf(desf, "%s\\Mov\\%s", HVM_BIN_PATH, pFtpInfo->szRemoteFile);
				break;
			}
			LOG_OUT("[%s:%d] srcf(%s), desf(%s)..\n", __FUNCTION__,__LINE__,srcf, desf);
			bResult = CopyFileA(srcf, desf, FALSE);
			if(bResult == FALSE) {
				InsertSendQueue(nNgConfirm, 0);
				LOG_OUT("[%s:%d] RCMD_ANIMATION_INFO - CopyFile() 실패 ..\n", __FUNCTION__,__LINE__);
				break;
			}
			
			SET_VersionData(RCMD_ANIMATION_INFO, (char *)pFtpInfo->szVersion);

			SetGlobalConfigString(CFG_ANIMATION_INFO, desf);
			SetGlobalConfig(CFG_ANIMATION_APPLY, 1);

			InsertSendQueue(nOkConfirm, 0);

// 			sprintf(srcf, "%s", pFtpInfo->szLocalFile);
// 			LOG_OUT("[%s:%d] fileName(%s)..\n", __FUNCTION__,__LINE__,fileName);
// 			ret = CompareFileLength(pFtpInfo->cmdIndex, fileName, pFtpInfo->dwSize, pFtpInfo->szVersion);
// 			if(ret < 0) {
// 				InsertSendQueue(nNgConfirm, 0);
// 				break;
// 			}
// 
// 			sprintf(desf, "%s\\Mov%s", GetAbsolutePath(), ANIMATION_NAME);
// 			LOG_OUT("[%s:%d] desf(%s)..\n", __FUNCTION__,__LINE__,desf);
// 			ret = CopySmsFile(srcf, desf);
// 			LOG_OUT("CopySmsFile Write : %d\n", ret);
// 			if(ret <= 0) {
// 				InsertSendQueue(nNgConfirm, 0);
// 				break;
// 			}
// 
// 			//ReadFareDataFile();
// 			InsertSendQueue(nOkConfirm, 0);

		}
		break;

	case RCMD_STAFF_NUMBER :
		{
// 			sprintf(fileName, "%s%s", GetAbsolutePath(), GetDatamngPath(pFtpInfo->cmdIndex));
// 			LOG_OUT("[%s:%d] fileName(%s)..\n", __FUNCTION__,__LINE__,fileName);
// 
// 			sprintf(desf, "%s\\%s", GetAbsolutePath(), STAFF_NUMBER_NAME);
// 			LOG_OUT("[%s:%d] desf(%s)..\n", __FUNCTION__,__LINE__,desf);
// 			bResult = CopyFileA(fileName, desf, FALSE);
// 			if(bResult == FALSE) {
// 				LOG_OUT("[%s:%d] staff number file copy NG!!!\n", __FUNCTION__,__LINE__);
// 				break;
// 			}
// 			LOG_OUT("[%s:%d] staff number file copy OKAY...\n", __FUNCTION__,__LINE__);
// 			ReadStaffNumberData();
		}
		break;
	}
	***/

	return 0;

failure:
	return -1;
}

/**
 * @brief		FtpCommThread
 * @details		FTP 통신 쓰레드
 * @param		void *		: 사용안함
 * @return		항상 = 0
 */
static DWORD WINAPI FtpCommThread(void *)
{
	DWORD			dwRet;
	FTP_PARAM_T		tFtpData;

	while(1) 
	{
		Sleep(20);

		dwRet = ::WaitForSingleObject(hEventKill, 0);
		if(dwRet == WAIT_OBJECT_0) 
		{ // 종료
			break;
		}		
		
		if( !FtpQue.empty() ) 
		{
			tFtpData = FtpQue.front();
			FtpQue.erase( FtpQue.begin() );

			FTP_Downloading(&tFtpData);
		}
	}

	return 0L;
}

/**
 * @brief		FTP_Init
 * @details		FTP 통신 모듈 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int FTP_Init(void)
{
	LOG_INIT();

	LOG_OUT("start..");

	hAccMutex = CreateMutex(NULL,FALSE,NULL);
	if(hAccMutex == NULL) 
	{
		LOG_OUT("CreateMutex(), Error..");
		return -1;
	}

 	hFtpThread = CreateThread(NULL, 0, FtpCommThread, NULL, CREATE_SUSPENDED, &ThreadID);
 	if(hFtpThread == NULL) 
	{
		LOG_OUT("CreateThread(), Error..");
 		CloseHandle(hAccMutex);
 		return -1;
 	}
 	hEventKill	= ::CreateEvent(NULL, TRUE, FALSE, NULL);
 	if( hEventKill == NULL ) 
	{
		LOG_OUT("CreateEvent(), Error..");
 		CloseHandle(hAccMutex);
 		return -1;
 	}
 	
	ResumeThread(hFtpThread);

	LOG_OUT("Running..");

	return 0;
}

/**
 * @brief		FTP_Term
 * @details		FTP 통신모듈 종료
 * @param		None
 * @return		항상 = 0
 */
int FTP_Term(void)
{
 	SetEvent(hEventKill);
 	if( hFtpThread != NULL )
 	{
 		::WaitForSingleObject(hFtpThread, 1500);
 		::CloseHandle(hFtpThread);
 		hFtpThread = NULL;
 	}
 
	if( hEventKill != NULL )
	{
 		CloseHandle(hEventKill);
		hEventKill = NULL;
	}

	if( hAccMutex != NULL )
	{
		CloseHandle(hAccMutex);
		hAccMutex = NULL;
	}
	return 0;
}

/**
 * @brief		
 * @details		
 * @param		None
 * @return		None
 */
int CheckFtpTempFile(char *fileName, int nSize)
{
	int nFileSize;

	ifstream ioFile(fileName,ios::in|ios::binary);
	ioFile.seekg(0, ios::end);
	nFileSize = ioFile.tellg();

	LOG_OUT("[%s:%d] fileName(%s), nSize(%d), nFileSize(%d)..\n", __FUNCTION__,__LINE__,fileName, nSize, nFileSize);

	if( nFileSize != nSize ) {
		ioFile.close();
		return -1;
	}
	ioFile.close();

	return 1;
}

/**
 * @brief		FTP_AddQueueData
 * @details		FTP Queue에 데이타 추가
 * @param		void *pData			ftp 정보 파일
 * @return		항상 = 0
 */
int FTP_AddQueueData(void *pData)
{
	FTP_PARAM_T		tFtpData;

	memcpy(&tFtpData, pData, sizeof(FTP_PARAM_T));

	FtpQue.push_back(tFtpData);

	return 0;
}

