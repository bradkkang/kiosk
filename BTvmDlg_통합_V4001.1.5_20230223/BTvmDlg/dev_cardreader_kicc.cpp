// 
// 
// Dev_CardRD_KICC.cpp : 신용카드 리더기 (KICC:ED-947)
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <queue>
#include <iostream>

#include "MyDefine.h"
#include "MyUtil.h"
#include "dev_cardreader_kicc.h"
#include "event_if.h"

//#define TEST_ADD_Q
//#define TEST_RECV_Q
//#define TEST_CHKRECV_Q

//----------------------------------------------------------------------------------------------------------------------

#define LOG_OUT(fmt, ...)		{ CCardRD_KICC::m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ CCardRD_KICC::m_clsLog.HexaDump(x, y, z); }

//----------------------------------------------------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CCardRD_KICC
 * @details		생성자
 */
CCardRD_KICC::CCardRD_KICC()
{
	m_hDll = NULL;
	m_hAccMutex = NULL;
	m_hThread = NULL;

	m_fnKLoad = NULL;
	m_fnKReqCmd = NULL;
	m_fnKGetEvent = NULL;
	m_fnKEmv = NULL;
	m_fnKWaitCmd = NULL;
	m_fnKReqReset = NULL;
	m_fnKUnload = NULL;

	m_bConnected = FALSE;
	m_bOperate = TRUE;

	m_nTimeOut = 1000;
}

/**
 * @brief		~CCardRD_KICC
 * @details		소멸자
 */
CCardRD_KICC::~CCardRD_KICC()
{
	EndProcess();
}

/**
 * @brief		LOG_INIT
 * @details		LOG 파일 초기화
 * @param		None
 * @return		None
 */
void CCardRD_KICC::LOG_INIT(void)
{
	m_clsLog.SetData(30, "\\Log\\Kicc");
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
int CCardRD_KICC::Locking(void)
{
	if(m_hAccMutex != NULL) 
	{
		//::WaitForSingleObject(m_hAccMutex, INFINITE);
	}

	return 0;	
}

/**
 * @brief		UnLocking
 * @details		IPC UnLock
 * @param		None
 * @return		항상 : 0
 */
int CCardRD_KICC::UnLocking(void)
{
	if(m_hAccMutex != NULL) 
	{
		//::ReleaseMutex(m_hAccMutex);
	}

	return 0;	
}

/**
 * @brief		ReqIcCard
 * @details		IC 거래 요청 (0x20)
 * @param		char *pData			IC 거래요청 데이타(=CARDRD_SND_REQ_IC_TR_T)
 * @param		int nDataLen		거래요청 데이타 길이	
 * @return		성공 : > 0, 실패 : < 0
 */
int CCardRD_KICC::GetRandomSeed(int nLen, char *retBuf)
{
	int i, k;
	char *pStr = "0123456789";
 
	srand((unsigned int)time(NULL));
 
	for(i = 0; i < nLen; i++)
	{
 		k = rand() % 10;
 		retBuf[i] = pStr[k];
	}
 
	return 0;
}

int CCardRD_KICC::SendData(int nCommand, char *pData, int nDataLen)
{
	int nRet;

	nRet = -1;

	if(m_bConnected == FALSE)
	{
		TRACE("[CCardRD_KICC::SendData] m_bConnected == FALSE error \n");
		return nRet;
	}

	if(m_fnKReqCmd != NULL)
	{
		m_nCMD = 251;		// 0xFB
		m_nGCD = 17;		// 0x11
		m_nJCD = nCommand;	// 
	
		::ZeroMemory(m_szErrMsg, sizeof(m_szErrMsg));

		TRACE("m_nCMD(%d), m_nGCD(%d), m_nJCD(%d) !!\n", m_nCMD, m_nGCD, m_nJCD);

		if(nDataLen > 0)
		{
			nRet = m_fnKReqCmd(m_nCMD, m_nGCD, m_nJCD, pData, m_szErrMsg);
		}
		else
		{
			nRet = m_fnKReqCmd(m_nCMD, m_nGCD, m_nJCD, "", m_szErrMsg);
		}
		nRet = 1;
	}

	return nRet;
}

/**
 * @brief		ParsingPacket
 * @details		데이타 분석
 * @param		int nJCD		수신 COMMAND
 * @param		char *pRecvData	수신 데이타
 * @param		int nRecvLen	수신 데이타 길이
 * @return		성공 >= 0, 실패 < 0
 */
int CCardRD_KICC::ParsingPacket(int nJCD, char *pRecvData, int nRecvLen)
{
	int nRet, nCount, nOffset, i;

	nRet = nCount = nOffset = i = 0;

	switch(nJCD)
	{
	case KICC_CMD_INFO : ///> 0x02 리더기 정보 요청
		{
			::ZeroMemory(&m_tInfo, sizeof(CARDRD_INFO_T));

			///< 모델코드
			::CopyMemory(m_tInfo.szModel, &pRecvData[nOffset], 4); 
			nOffset += 4;

			///< 버젼
			::CopyMemory(m_tInfo.szVersion, &pRecvData[nOffset], 4); 
			nOffset += 4;

			///< 시리얼 번호
			::CopyMemory(m_tInfo.szSerialNo, &pRecvData[nOffset], 12); 
			nOffset += 12;

			///< 프로토콜 버젼
			::CopyMemory(m_tInfo.szProtocol, &pRecvData[nOffset], 2); 
			nOffset += 2;

			///< 사용 MSR TRACK
			m_tInfo.chUseMsrTrack = pRecvData[nOffset++];

			///< MAX Van
			m_tInfo.byMaxVan = pRecvData[nOffset++];
			
			///< VAN 갯수
			m_tInfo.byCount = pRecvData[nOffset++];

			nOffset = sizeof(CARDRD_RCV_INFO_T);
			for(i = 0; i < m_tInfo.byCount; i++)
			{
				///< VAN Code
				::CopyMemory(m_tInfo.tItem[i].szVanCode, &pRecvData[nOffset], 4);
				nOffset += 4;

				///< VAN Name
				::CopyMemory(m_tInfo.tItem[i].szVanName, &pRecvData[nOffset], 10);
				nOffset += 10;

				///< 키수신 공개키 버젼
				::CopyMemory(m_tInfo.tItem[i].szPubKey, &pRecvData[nOffset], 4);
				nOffset += 4;

				///< 암호화 방식
				::CopyMemory(m_tInfo.tItem[i].szSecurity, &pRecvData[nOffset], 2);
				nOffset += 2;
			}

			///< 보안인증번호
			::CopyMemory(m_tInfo.szKTCCerti, &pRecvData[nOffset], 16);
			nOffset += 16;
		}
		break;
	case KICC_CMD_DETECT : ///> 0x09 Card Detec 정보
		{
			if( memcmp(&pRecvData[nOffset], "IC;h", 4) == 0 )
			{	// IC 카드인 경우
				nOffset += 4;
				SetCheckEventCode(EC_IC_CARD_DETECT, TRUE);
				SetCheckEventCode(EC_IC_CARD_IN, TRUE);
				break;
			}
			else if( memcmp(&pRecvData[nOffset], "IC", 2) == 0 )
			{	// IC 칩이 없는 경우
			}
			else if( memcmp(&pRecvData[nOffset], "MS", 2) == 0 )
			{	// MSR 데이타는 에러 처리 필요
				
			}
			else
			{	
			}
			SetCheckEventCode(EC_IC_CARD_DETECT, FALSE);
		}
		break;
	case KICC_CMD_REQ_IC_INFO : ///> 0x20 IC 거래 요청
		{
			::ZeroMemory(&m_tICTr, sizeof(CARDRD_RCV_IC_TR_T));

			///< (01). 카드구분자
			::CopyMemory(m_tICTr.szCardFlag, &pRecvData[nOffset], 4);
			nOffset += 4;

			///< (02). CVM
			m_tICTr.chCVM = pRecvData[nOffset++];

			///< (02). Online 여부
			m_tICTr.chIsOnline = pRecvData[nOffset++];

			///< (03). 카드번호
			::CopyMemory(&m_tICTr.szCardNo, &pRecvData[nOffset], 40);
			nOffset += 40;

			///< (04). 모델번호
			::CopyMemory(m_tICTr.szModelNo, &pRecvData[nOffset], 8);
			nOffset += 8;

			///< (05). 자료갯수
			::CopyMemory(m_tICTr.szCount, &pRecvData[nOffset], 2);
			nOffset += 2;
			nCount = (int)Util_Ascii2Long(m_tICTr.szCount, 2);
			for(i = 0; i < nCount; i++)
			{
				int nSecurity;

				///< (06). VanCode
				::CopyMemory(m_tICTr.tItem[i].szVanCode, &pRecvData[nOffset], 4);
				nOffset += 4;
				
				///< (07). 암호화 구분
				::CopyMemory(m_tICTr.tItem[i].szSecurity, &pRecvData[nOffset], 2);
				nOffset += 2;
				nSecurity = (int)Util_Ascii2Long(m_tICTr.tItem[i].szSecurity, 2);

				///< (08). KSN
				::CopyMemory(m_tICTr.tItem[i].szKSN, &pRecvData[nOffset], 20);
				nOffset += 20;

				///< (09). EncData
				if( (nSecurity == 22) || (nSecurity == 23) )
				{
					::CopyMemory(m_tICTr.tItem[i].szEncData, &pRecvData[nOffset], 64);
					nOffset += 64;
				}
				else if(nSecurity == 24)
				{
					::CopyMemory(m_tICTr.tItem[i].szEncData, &pRecvData[nOffset], 130);
					nOffset += 130;
				}
				else
				{
					break;
				}
			}

			switch(m_tICTr.szCardFlag[0])
			{
			case 'R' : ///< RF 거래
			case 'I' : ///< IR 거래
			case 'M' : ///< MS 거래
				nRet = -1;
				break;
			case 'C' : ///< IC 거래
				nRet = nCount;
				break;
			}
		}
		break;
	case KICC_CMD_COMPLETE_ICTR: ///> 0x18 IC EMV 완료 요청
		{

		}
		break;
	case KICC_CMD_SET_MS : ///> 0x23 MSR 읽기 설정
		{
			if(pRecvData[0] == 0x31) 
			{
				// MSR 사용 가능.
			}
			else
			{
				// MSR 사용 못함
			}
		}
		break;
	case KICC_CMD_SPACE : ///> 0x25	SPACE 전송 요청
		{
		}
		break;
	case KICC_CMD_CARDNO_ENCDATA : ///> 0x26 KEY In 카드번호 암호화 데이타 요청
		{
			::ZeroMemory(&m_tEncData, sizeof(CARDRD_RCV_ENCDATA_T));

			///< (01). 카드구분자
			::CopyMemory(m_tEncData.szCardFlag, &pRecvData[nOffset], 4);
			nOffset += 4;

			///< (02). CVM
			::CopyMemory(&m_tEncData.chCVM, &pRecvData[nOffset], 1);
			nOffset += 1;

			///< (02). Online 여부
			::CopyMemory(&m_tEncData.chIsOnline, &pRecvData[nOffset], 1);
			nOffset += 1;

			///< (03). 카드번호
			::CopyMemory(&m_tEncData.szCardNo, &pRecvData[nOffset], 40);
			nOffset += 40;

			///< (04). 모델번호
			::CopyMemory(m_tEncData.szModelNo, &pRecvData[nOffset], 8);
			nOffset += 8;

			///< (05). 자료갯수
			::CopyMemory(m_tEncData.szCount, &pRecvData[nOffset], 2);
			nOffset += 2;
			nCount = (int)Util_Ascii2Long(m_tEncData.szCount, 2);
			for(i = 0; i < nCount; i++)
			{
				int nSecurity;

				///< (06). VanCode
				::CopyMemory(m_tEncData.tItem[i].szVanCode, &pRecvData[nOffset], 4);
				nOffset += 4;
				
				///< (07). 암호화 구분
				::CopyMemory(m_tEncData.tItem[i].szSecurity, &pRecvData[nOffset], 2);
				nOffset += 2;
				nSecurity = (int)Util_Ascii2Long(m_tEncData.tItem[i].szSecurity, 2);

				///< (08). KSN
				::CopyMemory(m_tEncData.tItem[i].szKSN, &pRecvData[nOffset], 20);
				nOffset += 20;

				///< (09). EncData
				if( (nSecurity == 22) || (nSecurity == 23) )
				{
					::CopyMemory(m_tEncData.tItem[i].szEncData, &pRecvData[nOffset], 64);
					nOffset += 64;
				}
				else if(nSecurity == 24)
				{
					::CopyMemory(m_tEncData.tItem[i].szEncData, &pRecvData[nOffset], 130);
					nOffset += 130;
				}
				else
				{
					break;
				}
			}

			switch(m_tEncData.szCardFlag[0])
			{
			case 'R' : ///< RF 거래
			case 'I' : ///< IR 거래
			case 'M' : ///< MS 거래
				nRet = -1;
				break;
			case 'C' : ///< IC 거래
				nRet = nCount;
				break;
			}
		}
		break;
	case KICC_CMD_HACKING : ///> 0x30 보안 침해 정보 요청
		{
			PCARDRD_RCV_HACKING_T pPacket;

			pPacket = (PCARDRD_RCV_HACKING_T) pRecvData;
			if(pPacket->chTamper == 'G')
			{	// 보안 침해되지 않음
				TRACE("보안 침해되지 않음, (%s)\n", pPacket->szDate);
			}
			else
			{
				switch(pPacket->chReason)
				{
				case 0x31 :	// 물리적 탐침
					TRACE("물리적 탐침\n");
					break;
				case 0x32 :	// 무결성 오류
					TRACE("무결성 오류\n");
					break;
				case 0x33 :	// 키정보 없음
					TRACE("키정보 없음\n");
					break;
				default :
					TRACE("기타 사유로 침해됨\n");
					break;
				}
			}
		}
		break;
	case KICC_CMD_STATUS : ///> 0x32 리더기 상태 조회
		{
			PCARDRD_RCV_IC_STATUS_T pPacket;

			pPacket = (PCARDRD_RCV_IC_STATUS_T) pRecvData;

			if( memcmp(pPacket->szFlag, "01", 2) == 0 )
			{
				if(pPacket->chResult == 0x30)
				{
					SetCheckEventCode(EC_IC_CARD_IN, TRUE);
				}
				else
				{
					SetCheckEventCode(EC_IC_CARD_IN, FALSE);
					SetCheckEventCode(EC_IC_CARD_DETECT, FALSE);
				}
			}
		}
		break;
	default :
		{

		}
		break;
	}

	return 0;
}

/**
 * @brief		RecvData
 * @details		데이타 수신
 * @param		None
 * @return		성공 > 0, 실패 < 0
 */
int CCardRD_KICC::RecvData(void)
{
	int nRet = -1;
//	KICC_QUE_RDATA_T queRData;

	if(m_fnKGetEvent != NULL)
	{
		m_nCMD = m_nGCD = m_nJCD = m_nRCD = 0;
		::ZeroMemory(m_szRxData, sizeof(m_szRxData));
		::ZeroMemory(m_szRxHexData, sizeof(m_szRxHexData));

		nRet = m_fnKGetEvent(m_nCMD, m_nGCD, m_nJCD, m_nRCD, m_szRxData, m_szRxHexData);
		//TRACE("########## RecvData_#2()/m_fnKGetEvent() nRet(%d) \n", nRet);
		if(nRet >= 0)
		{
			LOG_OUT("#1. ReadData() nRet(%d) !!!!!!!!!!!!!!!!!!", nRet);
			LOG_OUT("#1. Read cmd      : m_nCMD(%02X), m_nGCD(%d), m_nJCD(%d), m_nRCD(%d)", m_nCMD, m_nGCD, m_nJCD, m_nRCD);
			LOG_OUT("#1. Read rdata    : (%s)", m_szRxData);

			if(m_nCMD != 0xFB)
			{
				return -1;
			}

			if( (m_nGCD == 0x11) && (m_nJCD == 0x32) )
			{
				if(m_nRCD != 0)
				{
					return -2;
				}
			}

#ifdef TEST_RECV_Q
			::ZeroMemory(&queRData, sizeof(KICC_QUE_RDATA_T));

			queRData.nCMD = m_nCMD; 
			queRData.nGCD = m_nGCD; 
			queRData.nJCD = m_nJCD; 
			queRData.nRCD = m_nRCD; 
			queRData.nLen = nRet; 
			::CopyMemory(queRData.szData, m_szRxData, nRet);
			m_QueRcvData.push(queRData);

#else
			ParsingPacket(m_nJCD, m_szRxData, nRet);

#endif
			m_bOperate = TRUE;
			return nRet;
		}
	}

	return -1;
}

/**
 * @brief		Initialize
 * @details		IC CARD 초기화
 * @param		None
 * @return		항상 : 0
 */
int CCardRD_KICC::FnInitialize(void)
{
	m_fnKLoad = (KLoad) GetProcAddress(m_hDll, "KLoad");
	if(m_fnKLoad == NULL)
	{
		LOG_OUT("KLoad() 함수 error!!!\n");
		return -1;
	}

	m_fnKReqCmd = (KReqCmd) GetProcAddress(m_hDll, "KReqCmd");
	if(m_fnKReqCmd == NULL)
	{
		LOG_OUT("KReqCmd() 함수 error!!!\n");
		return -2;
	}

	m_fnKGetEvent = (KGetEvent) GetProcAddress(m_hDll, "KGetEvent");
	if(m_fnKGetEvent == NULL)
	{
		LOG_OUT("KGetEvent() 함수 error!!!\n");
		return -3;
	}

	m_fnKEmv = (KGetEmv) GetProcAddress(m_hDll, "KGetEmv");
	if(m_fnKEmv == NULL)
	{
		LOG_OUT("KGetEmv() 함수 error!!!\n");
		return -4;
	}

	m_fnKWaitCmd = (KWaitCmd) GetProcAddress(m_hDll, "KWaitCmd");
	if(m_fnKWaitCmd == NULL)
	{
		LOG_OUT("KWaitCmd() 함수 error!!!\n");
		return -5;
	}

// 	m_fnKReqReset = (KReqReset) GetProcAddress(m_hDll, "KReqReset");
// 	if(m_fnKUnload == NULL)
// 	{
// 		LOG_OUT("KReqReset() 함수 error!!!\n");
// 		return -6;
// 	}

	m_fnKUnload = (KUnload) GetProcAddress(m_hDll, "KUnLoad");
	if(m_fnKUnload == NULL)
	{
		LOG_OUT("KUnload() 함수 error!!!\n");
		return -7;
	}

	return 0;	
}

/**
 * @brief		Reset
 * @details		카드리더기 reset
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCardRD_KICC::Reset(void)
{
	int	nRet;

	if(m_fnKReqReset != NULL)
	{
		return -1;
	}

	nRet = m_fnKReqReset();

	return nRet;
}

/**
 * @brief		GetDeviceInfo
 * @details		카드리더기 정보 요청	(0x02)
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCardRD_KICC::GetDeviceInfo(void)
{
	int	nRet;

	nRet = SendData(KICC_CMD_INFO, "", 0);

	return nRet;
}

/**
 * @brief		ReqIcCard
 * @details		IC 거래 요청 (0x20)
 * @param		char *pData			IC 거래요청 데이타(=CARDRD_SND_REQ_IC_TR_T)
 * @param		int nDataLen		거래요청 데이타 길이	
 * @return		성공 : > 0, 실패 : < 0
 */
int CCardRD_KICC::ReqIcCard(char *pData, int nDataLen)
{
	int nRet;

	nRet = SendData(KICC_CMD_REQ_IC_INFO, pData, nDataLen);
	if(nRet >= 0)
	{	///< 성공
	}

	return nRet;
}


/**
 * @brief		SetMSReadInfo
 * @details		MS Reader 기능 설정 유무(0x23)
 * @param		BOOL bSet			설정 유무
 * @return		성공 : > 0, 실패 : < 0
 */
int CCardRD_KICC::SetMSReadInfo(BOOL bSet)
{
	int		nRet;
	char	Buffer[10];

	if(bSet == TRUE)
	{
		Buffer[0] = '1';
	}
	else
	{
		Buffer[0] = '0';
	}

	nRet = SendData(KICC_CMD_SET_MS, Buffer, 1);
	if(nRet >= 0)
	{	///< 성공
	}

	return nRet;
}

/**
 * @brief		ReqSpaceInfo
 * @details		SPACE 정보 요청(0x23)
 * @param		BOOL bSet			설정 유무
 * @return		성공 : > 0, 실패 : < 0
 */
int CCardRD_KICC::ReqSpaceInfo(void)
{
	int		nRet;

	nRet = SendData(KICC_CMD_SPACE, "", 0);
	if(nRet >= 0)
	{	///< 성공
	}

	return nRet;
}

/**
 * @brief		GetCardNoEncData
 * @details		KEY In 카드번호 암호화 데이타 요청(0x26), 수신 데이타는 0x20 Response 코드와 동일
 * @param		char *pData			전송 데이타 
 * @param		int nDataLen		데이타 길이
 * @return		성공 : > 0, 실패 : < 0
 */
int CCardRD_KICC::GetCardNoEncData(char *pData, int nDataLen)
{
	int nRet;

	nRet = SendData(KICC_CMD_CARDNO_ENCDATA, pData, nDataLen);
	if(nRet >= 0)
	{	///< 성공
	}

	return nRet;
}

/**
 * @brief		ReqSpaceInfo
 * @details		SPACE 정보 요청(0x23)
 * @param		BOOL bSet			설정 유무
 * @return		성공 : > 0, 실패 : < 0
 */
int CCardRD_KICC::ReqHackingInfo(void)
{
	int		nRet;
	char    errMsg[1024];

	nRet = SendData(KICC_CMD_HACKING, "", 0);
	if(nRet >= 0)
	{	///< 성공
	}

	nRet = m_fnKWaitCmd(KICC_CMD_HACKING, m_szRxData, 1000, 0, (char *)"", errMsg);
	if(nRet != 0)
	{
		TRACE("errMsg = %s \n", errMsg);
	}

	return nRet;
}

/**
 * @brief		GetStatusInfo
 * @details		리더기 상태 요청(0x32)
 * @param		BOOL bSet			설정 유무
 * @return		성공 : > 0, 실패 : < 0
 */
int CCardRD_KICC::GetStatusInfo(void)
{
	int		nRet;

	nRet = SendData(KICC_CMD_STATUS, "01", 2);
	if(nRet >= 0)
	{	///< 성공
	}

	return nRet;
}

/**
 * @brief		GetEmvInfo
 * @details		get emv info
 * @param		BYTE *pEMV			emv info
 * @return		성공 : > 0, 실패 : < 0
 */
int CCardRD_KICC::GetEmvInfo(BYTE *pEMV)
{
	int nRet;

	if(m_fnKEmv != NULL)
	{
		return -1;
	}

	nRet = m_fnKEmv(pEMV);
	if(nRet >= 0)
	{
		TRACE("m_fnKEmv(), nRet(%d)..\n", nRet);
	}

	return nRet;
}

/**
 * @brief		StartProcess
 * @details		Start
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCardRD_KICC::StartProcess(int nCommIdx)
{
	int		nRet, nLog = 0;
//	DWORD dwThreadID;
	char szBasePath[256];
	char szFullName[256];
	CString strFullName;

	LOG_INIT();

	m_bConnected = FALSE;

	while(!m_QueRcvData.empty())
	{
		m_QueRcvData.pop();	
	}

	if(m_hDll != NULL)
	{
//		EndProcess();
	}

	USES_CONVERSION;

	::ZeroMemory(szBasePath, sizeof(szBasePath));
	Util_GetModulePath(szBasePath);
	sprintf(szFullName, "%s\\KiccPos.dll", szBasePath);

	strFullName = (CString) szFullName;

	m_hDll = LoadLibrary(strFullName);
	if(m_hDll == NULL)
	{
		LOG_OUT("KiccPos.dll load error !!!\n");
		return -1;
	}

	nRet = FnInitialize();
	if(nRet < 0)
	{
		LOG_OUT("FnInitialize() error !!!\n");
		EndProcess();
		return -2;
	}

	// ed-947 모델 : 57600
	nRet = m_fnKLoad(nCommIdx, 57600, m_szErrMsg);
	// ed-977 모델 : 115200
//	nRet = m_fnKLoad(nCommIdx, 115200, m_szErrMsg);
	if(nRet < 0)
	{
		LOG_OUT("Connect() failure !!!\n");
		EndProcess();
		return -3;
	}

	m_hAccMutex = ::CreateMutex(NULL, FALSE, NULL);
	if(m_hAccMutex == NULL) 
	{
		LOG_OUT("CreateMutex() failure !!!\n");
		EndProcess();
		return -4;
	}

#if 0
	m_hThread = ::CreateThread(NULL, 0, RunThread, this, CREATE_SUSPENDED, &dwThreadID);
	if(NULL == m_hThread)
	{
		LOG_OUT("Cannot create thread [%d]\n", ::GetLastError());
		EndProcess();
		return -5;
	}

	::ResumeThread(m_hThread);
#endif

	m_bConnected = TRUE;

	return 0;
}

/**
 * @brief		EndProcess
 * @details		Start
 * @param		int nCommIdx		COM
 * @return		성공 : > 0, 실패 : < 0
 */
int CCardRD_KICC::EndProcess(void)
{
	if( m_bConnected == TRUE )
	{
		m_bConnected = FALSE;
	
		::WaitForSingleObject(m_hThread, 500);
		if(NULL != m_hThread)
		{
			::CloseHandle(m_hThread);

			m_hThread = NULL;
		}

		if(m_fnKUnload != NULL)
 		{
 			m_fnKUnload();
 			m_fnKUnload = NULL;
 		}

		if(m_hDll != NULL)
		{
			FreeLibrary(m_hDll);	
			m_hDll = NULL;
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
 * @brief		Polling
 * @details		Polling
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCardRD_KICC::Polling(void)
{
	int		nRet;

	nRet = RecvData();
	if(nRet >= 0)
	{	///< 성공
	}

	return nRet;
}

/**
 * @brief		RunThread
 * @details		thread run
 * @param		LPVOID lParam		CCardRD_KICC class instance
 * @return		항상 = 0
 */
DWORD CCardRD_KICC::RunThread(LPVOID lParam)
{
	DWORD	dwTick, dwStart, dwCheck = 0;
	CCardRD_KICC *pClass = (CCardRD_KICC *)lParam;

	dwStart = dwTick = ::GetTickCount();
	while(pClass->m_bConnected)
	{
		Sleep(1);

		if( Util_CheckExpire(dwTick) >= 100 )	
		{
			//nRet = pClass->RecvData();
			//pClass->Polling();
			//dwTick = ::GetTickCount();
		}		

		if( Util_CheckExpire(dwStart) >= 2000 )	
		{
// 			pClass->GetDeviceInfo();
// 			dwStart = ::GetTickCount();
// 			if(dwCheck == 0)
// 			{
// 				dwCheck = 1;
// 				pClass->GetDeviceInfo();
// 			}
		}
	}

	return 0;
}

/**
 * @brief		CheckQueData
 * @details		check queue data 
 * @param		none
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCardRD_KICC::CheckQueData(char *retBuf)
{
	int nSize;
//	DWORD dwTick;
	KICC_QUE_RDATA_T queKiccData;

	if( m_bConnected == TRUE )
	{
		nSize = m_QueRcvData.size();
		if(nSize > 0)
		{
			queKiccData = m_QueRcvData.front();
			m_QueRcvData.pop();

			::CopyMemory(retBuf, &queKiccData, sizeof(KICC_QUE_RDATA_T));
			return 1;
		}
	}	
	
	return -1;
}



