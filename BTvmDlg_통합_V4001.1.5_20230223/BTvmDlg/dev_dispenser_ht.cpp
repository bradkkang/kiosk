// 
// 
// Comm_HCdu.cpp : 한틀 지폐방출기 (HSCDU2)
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "event_if.h"

#include "dev_dispenser_ht.h"
#include "dev_dispenser_main.h"

//----------------------------------------------------------------------------------------------------------------------

#define LOG_OUT(fmt, ...)		{ CCduHt::m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ CCduHt::m_clsLog.HexaDump(x, y, z); }

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CCduHt
 * @details		생성자
 */
CCduHt::CCduHt()
{
	m_hThread = NULL;
	m_hAccMutex = NULL;
	m_bConnected = FALSE;
}

/**
 * @brief		~CCduHt
 * @details		소멸자
 */
CCduHt::~CCduHt()
{
	LOG_OUT(" start !! ");
	EndProcess();
}

/**
 * @brief		LOG_INIT
 * @details		LOG 파일 초기화
 * @param		None
 * @return		None
 */
void CCduHt::LOG_INIT(void)
{
	m_clsLog.SetData(30, "\\Log\\Cdu\\Ht");
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
int CCduHt::Locking(void)
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
int CCduHt::UnLocking(void)
{
	if(m_hAccMutex != NULL) 
	{
		::ReleaseMutex(m_hAccMutex);
	}

	return 0;	
}

/**
 * @brief		SendChar
 * @details		1 Byte 전송 data
 * @param		BYTE byData		전송 데이타
 * @return		항상 : 0
 */
int CCduHt::SendChar(BYTE byData)
{
	//LOG_HEXA("SendChar", &byData, 1);
	return m_clsComm.SendData(&byData, 1);
}

/**
 * @brief		SendPacket
 * @details		send data
 * @param		int nCommand	전송 command
 * @param		BYTE *pData		전송 Data
 * @param		WORD wDataLen	전송 Data length
 * @return		항상 : 0
 */
int CCduHt::SendPacket(int nCommand, BYTE *pData, WORD wDataLen)
{
	int		nOff, nRet, i;
	WORD	wLen;
	BYTE	byBCC;

	byBCC = nOff = 0;

	::ZeroMemory(m_szTxBuf, sizeof(m_szTxBuf));

	// (01). stx
	m_szTxBuf[nOff++] = CHAR_STX;
	// (02). length
	wLen = wDataLen + 1;
	CopyMemory(&m_szTxBuf[nOff], &wLen, sizeof(WORD));
	nOff += sizeof(WORD);
	// (03). Command
	m_szTxBuf[nOff++] = nCommand & 0xFF;
	// (04). data
	if(wDataLen > 0)
	{
		::CopyMemory(&m_szTxBuf[nOff], pData, wDataLen);
		nOff += wDataLen;
	}
	// (05). ETX
	m_szTxBuf[nOff++] = CHAR_ETX;
	// (06). Checksum
	byBCC = 0;
	for(i = 1; i < nOff; i++)
	{
		byBCC ^= m_szTxBuf[i];
	}
	m_szTxBuf[nOff++] = byBCC & 0xFF;

	nRet = m_clsComm.SendData(m_szTxBuf, nOff);

	LOG_HEXA("SendPacket", m_szTxBuf, nOff);

	return nRet;
}

/**
 * @brief		GetPacket
 * @details		get data
 * @param		BYTE *retBuf	receive data
 * @return		성공 : > 0, 실패 : < 0
 */
int CCduHt::GetPacket(BYTE *retBuf)
{
	static WORD	wPktLen;
	static BYTE	byBCC, byCalcBCC;
	static int	nState, nRecv, nDataCount;
	int	ch, i;

	while(1) 
	{ 
		ch = m_clsComm.ReadData();
		if(ch < 0) 
		{
			return ch;
		}

		ch = ch & 0xFF;
		//TRACE("ch = [%d], [%02X]\n", nState, ch & 0xFF);

		switch(nState) 
		{
		case 0 :	// stx
			nState = nRecv = wPktLen = nDataCount = 0;
			if(ch == CHAR_STX)
			{
				wPktLen = nDataCount = 0;
				nState++;
			}
			else 
			{
				//LOG_HEXA("GetPacket", retBuf, 1);

				if( ch == CHAR_ACK )
				{
					retBuf[0] = ch;
					return 1;
				}
				else if(ch == CHAR_NAK )
				{
					retBuf[0] = ch;
					return 1;
				}
			}
			break;

		case 1:		// Length(low)
			retBuf[nRecv++] = ch;
			nState++;
			break;

		case 2 :	// Length
			retBuf[nRecv++] = ch;
			CopyMemory(&wPktLen, &retBuf[nRecv - 2], 2);
			nState++;
			break;

		case 3 :	// data
			if(nRecv >= (MAX_CDU_BUF - 1)) 
			{
				nState = 0;
				break;
			}
			retBuf[nRecv++] = ch;
			nDataCount++;

			if( wPktLen == nDataCount ) 
			{
				nState++;
			}
			break;

		case 4 :	// etx
			retBuf[nRecv++] = ch;
			if(ch == CHAR_ETX)
			{
				nState++;
			}
			else
			{
				nState = 0;
			}
			break;

		case 5 :	// checksum
			retBuf[nRecv++] = ch;
			byBCC = ch;
			goto got_packet;
		}
	}

got_packet:
	
	nState = wPktLen = nDataCount = 0;

	byCalcBCC = 0;
	for(i = 0; i < nRecv - 1; i++)
	{
		byCalcBCC ^= retBuf[i];
	}

	if(byCalcBCC != byBCC) 
	{
		LOG_OUT("BCC ERROR[%02X, %02X]..\n", byBCC, byCalcBCC);
		return -2;
	}
	
	LOG_HEXA("GetPacket", retBuf, nRecv);

	return nRecv;
}

/**
 * @brief		SndRcvPacket
 * @details		데이타 송.수신 데이타 처리
 * @param		BYTE *pCommand, BYTE *sData, int sLen, BYTE *retBuf
 * @return		성공 : > 0, 실패 : < 0
 */
int CCduHt::SndRcvPacket(int nCommand, BYTE *pData, int nDataLen, BYTE *retBuf)
{
	int	 nRet, i;
	DWORD dwTick;
	PHSCDU_HEADER_T pHdr;

	for(i = 0; i < MAX_CDU_RETRY; i++)
	{
		//Sleep(1000);
		// (01). send packet
		nRet = SendPacket(nCommand, pData, (WORD)nDataLen);
	
		// (02). recv ACK
		dwTick = ::GetTickCount();
		while( Util_CheckExpire(dwTick) < 500 )	// timeout = 300 ms
		{
			nRet = GetPacket(retBuf);
			if(nRet > 0)
			{
				if(retBuf[0] == CHAR_ACK)
				{
					goto STEP_ENQ;
				}
				else if(retBuf[0] == CHAR_NAK)
				{
					break;
				}
			}
		}
	}

	return -1;

STEP_ENQ:

	for(i = 0; i < MAX_CDU_RETRY; i++)
	{
		SendChar(CHAR_ENQ);

		dwTick = ::GetTickCount();
		while( Util_CheckExpire(dwTick) < 500 )	// timeout = 300 ms
		{
			nRet = GetPacket(retBuf);
			if(nRet == 1)
			{
				if(retBuf[0] == CHAR_ACK)
				{
					SendChar(CHAR_ENQ);
					dwTick = ::GetTickCount();
				}
				else if(retBuf[0] == CHAR_NAK)
				{
					break;
				}
			}
			else if(nRet > 1)
			{
				SendChar(CHAR_ACK);
				goto STEP_RESULT;
			}
		}
	}

	return -2;

STEP_RESULT:

	pHdr = (PHSCDU_HEADER_T) retBuf;

	if(pHdr->byResult != 0x4F)
	{
		//return -3;
	}

	return nRet;
}

/**
 * @brief		CheckResult
 * @details		에러코드 체크
 * @param		char *pErrString		: 에러코드
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCduHt::CheckResult(char *pErrString)
{
	int nRet, i, nFind;
	HSCDU_ERROR_TBL_T stErrTable[] = 
	{
//		{ "00000", "Success" }, 
		{ "40011", "EJR/L Sensor detects a note after Initializing or before dispensing" }, 
		{ "40012", "GTR/GTL Sensor detects a note remains when initializing or dispensing" }, 
		{ "40013", "EJR/L and RJT Sensor detects a note after Initializing or before dispensing" }, 
		{ "40021", "DBL detects a note remains" }, 
		{ "40022", "INR/INL sensor detects a note remains when initializing or dispensing" }, 
		{ "40023", "SKRA/SKLA sensor detects a note remains when initializing or dispensing" }, 
		{ "40028", "EJR/L Sensor detects a note after Initializing or before dispensing" }, 
		{ "40030", "Main motor failure" }, 
		{ "40031", "Sub motor failure" }, 
		{ "40032", "SKRB/SKLB sensor detects a note remains when initializing or dispensing" }, 
		{ "40036", "EXIT Sensor detects notes when initializing" }, 
		{ "40037", "Double feed detection module operates abnormally" }, 
		{ "40039", "Gate Solenoid failure when Initializing or before dispensing" }, 
		{ "4003A", "Request more than 5 notes during Test Dispensing" }, 
		{ "4003B", "SKRB/SKLB sensor detects a note remains when initializing or dispensing" }, 
		{ "40041", "And more than five times re-release of the Note after release of the last" }, 
		{ "40043", "Reject more than 10 notes" }, 
		{ "40044", "More than 5 times consecutive reject occurs" }, 
		{ "40045", "The requested number < the exit number" }, 
		{ "40047", "Miss Pick Up Error at 1st cassette" }, 
		{ "40049", "Request to dispense 0 note" }, 
		{ "4004A", "Jam is detected at cassette exit path(INR/INL) during dispensing" }, 
		{ "4004D", "Cassette is not detected" }, 
		{ "4004E", "2nd cassette is not detected" }, 
		{ "4004F", "More than 85 seconds passed during driving motor" }, 
		{ "40051", "The requested notes are more than 150" }, 
		{ "40052", "Note-jam is detected at the exit area of cassette after dispensing" }, 
		{ "40053", "EXIT Sensor detects a note when test dispensing" }, 
		{ "40054", "Long length is detected on the EXIT Sensor during dispensing" }, 
		{ "40055", "Exit sensor half Note ejection" }, 
		{ "40056", "Gate is not located to the direction of dispensing" }, 
		{ "40057", "Gate is not located to the direction of rejecting" }, 
		{ "40058", "The logical number of notes < the exit number" }, 
		{ "4005B", "Miss Pick Up Error at the 2nd cassette" }, 
		{ "40060", "SKRC/SKLC sensor detects a note remains when initializing or dispensing" }, 
		{ "40062", "Note-jam is detected at the exit area of 3rd cassette after dispensing" }, 
		{ "4006A", "Jam is detected at 2nd cassette exit path during dispensing" }, 
		{ "4006B", "SKRC/SKLC sensor detects a note remains when initializing or dispensing" }, 
		{ "40070", "SKRC/SKLC sensor detects a note remains when initializing or dispensing" }, 
		{ "40072", "Note-jam is detected at the exit area of 4th cassette after dispensing" }, 
		{ "4007A", "Jam is detected at the fourth cassette exit path during dispensing" }, 
		{ "4007B", "SKRD/SKLD sensor detects a note remains when initializing or dispensing" }, 
		{ "4007C", "Miss Pick Up Error at the fourth cassette" }, 
		{ "4007D", "The fourth cassette is not detected" }, 
		{ "40080", "Note-jam is detected at the exit area of the 2nd cassette after dispensing" }, 
		{ "40081", "Jam is detected at the DBL sensor area during dispensing" }, 
		{ "40082", "Jam is detected at the path from SKL to DBL sensor during dispensing" }, 
		{ "40083", "Jam is detected at the GATL sensor area during dispensing" }, 
		{ "40084", "Jam is detected at the GATR sensor area during dispensing" }, 
		{ "40085", "Timeout of GTRL Sensor between from Double Timing Sensor" }, 
		{ "40086", "Jam is detected at the path from GATR/L to EXIT sensor during dispensing" }, 
		{ "40090", "Path check sensor error - outlet A (left)" }, 
		{ "40091", "Path check sensor error - outlet B (right)" }, 
		{ "40092", "Path check sensor error - gate A(left)" }, 
		{ "40093", "Path check sensor error - gate B (right)" }, 
		{ "40094", "Path check sensor error - Exit Sensor" }, 
		{ "40095", "Path check sensor error - Double" }, 
		{ "40096", "Path check sensor error - In A (left)" }, 
		{ "40097", "Path check sensor error - In B (right)" }, 
		{ "4009A", "Jam is detected at the 3rd cassette exit path during dispensing" }, 
		{ "4009D", "The 3rd cassette is not detected" }, 
		{ "4009F", "Miss Pick Up Error at the 3rd cassette" }, 
	};

	nFind = -1;
	for(i = 0; i < sizeof(stErrTable) / sizeof(HSCDU_ERROR_TBL_T); i++)
	{
		if(memcmp(pErrString, stErrTable[i].pCode, 5) == 0) 
		{
			nFind = i;
			LOG_OUT("i(%d), error_code(%s), err_str(%s)..", i, stErrTable[i].pCode, stErrTable[i].pStr);
			SetCheckEventCode(EC_CDU_DEV_ERR, TRUE);
			break;
		}
	}

	if(nFind < 0)
	{
		SetCheckEventCode(EC_CDU_DEV_ERR, FALSE);
		return -1;
	}

	nRet = nFind;

	return nRet;
}

/**
 * @brief		Initialize
 * @details		초기화 명령 처리
 * @param		int nInitFlag			: 0x00 or 0x01
 * @return		성공 : = 0, 실패 : < 0
 */
int CCduHt::Initialize(int nInitFlag)
{
	int nRet;
	PHSCDU_HEADER_T	pHdr;
	PHSCDU_RESP_INIT_T pPacket;

	Locking();
	{
		nRet = SndRcvPacket(HSCDU_CMD_INIT, (BYTE *)&nInitFlag, 1, m_szRxBuf);
		if(nRet < 0)
		{
			UnLocking();
			return nRet;
		}

		pHdr = (PHSCDU_HEADER_T) m_szRxBuf;
		pPacket = (PHSCDU_RESP_INIT_T) &pHdr->byData;

		nRet = CheckResult((char *)pPacket->szErrorCode);
		if(nRet < 0)
		{
			UnLocking();
			return -1;
		}

		if(nRet == 0)
		{
			UnLocking();
			return 0;
		}
	}
	UnLocking();

	return -nRet;
}

/**
 * @brief		GetStatus
 * @details		센서 상태 명령 처리
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCduHt::GetStatus(void)
{
	int nRet;
	static int nRetry = 0;
	PHSCDU_HEADER_T	pHdr;
	PHSCDU_RESP_STATUS_T pPacket;

	Locking();
	{
		nRet = SndRcvPacket(HSCDU_CMD_STATUS, (BYTE *)NULL, 0, m_szRxBuf);
		if(nRet < 0)
		{
			if(++nRetry >= 3)
			{
				SetCheckEventCode(EC_CDU_COMM_ERR, TRUE);
			}
			UnLocking();
			return nRet;
		}
		pHdr = (PHSCDU_HEADER_T) m_szRxBuf;
		pPacket = (PHSCDU_RESP_STATUS_T) &pHdr->byData;

		LOG_HEXA("Sensor Value ", pPacket->szSensorData, sizeof(pPacket->szSensorData));

		if(nRetry > 0)
		{
			nRetry = 0;
		}
		SetCheckEventCode(EC_CDU_COMM_ERR, FALSE);
	}
	UnLocking();

	return nRet;
}

int CCduHt::SensorStatus(void)
{
	int nRet;
	PHSCDU_HEADER_T	pHdr;
	PHSCDU_RESP_STATUS_T pPacket;

	Locking();
	{
		nRet = SndRcvPacket(HSCDU_CMD_STATUS, (BYTE *)NULL, 0, m_szRxBuf);
		if(nRet < 0)
		{
			UnLocking();
			return nRet;
		}
		pHdr = (PHSCDU_HEADER_T) m_szRxBuf;
		pPacket = (PHSCDU_RESP_STATUS_T) &pHdr->byData;

		LOG_HEXA("Sensor Value ", pPacket->szSensorData, sizeof(pPacket->szSensorData));
	}
	UnLocking();

	return nRet;
}


/**
 * @brief		GetVersion
 * @details		버젼 정보 읽기 명령 처리
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCduHt::GetVersion(void)
{
	int nRet;
	PHSCDU_HEADER_T	pHdr;
	PHSCDU_RESP_VERSION_T pPacket;

	Locking();
	{
		nRet = SndRcvPacket(HSCDU_CMD_VERSION, (BYTE *)NULL, 0, m_szRxBuf);
		if(nRet < 0)
		{
			UnLocking();
			return nRet;
		}

		pHdr = (PHSCDU_HEADER_T) m_szRxBuf;
		pPacket = (PHSCDU_RESP_VERSION_T) &pHdr->byData;

		TRACE("Unit Name = %s \n", pPacket->szUnitName);
		TRACE("Country Info = %c \n", pPacket->byCountry);
		TRACE("Cassette Info = %c \n", pPacket->byCassette);
		TRACE("CDU type = %02X \n", pPacket->byType);
		TRACE("Version = %.*s \n", sizeof(pPacket->szVersion), pPacket->szVersion);
	}
	UnLocking();

	return nRet;
}

/**
 * @brief		Dispense
 * @details		Dispense 명령 처리
 * @param		int nCount1		: 카세트1번 방출갯수
 * @param		int nCount2		: 카세트2번 방출갯수
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCduHt::Dispense(int nCount1, int nCount2)
{
	int nRet;
	HSCDU_SND_DISPENSE_T  tSend;
	PHSCDU_HEADER_T	pHdr;
	PHSCDU_RESP_DISPENSE_T pPacket;
	PHSCDU_RESULT_INFO_T pResult;

	Locking();
	{
		m_nOutCount[0] = m_nOutCount[1]	= m_nOutCount[2] = m_nOutCount[3] = 0;
		m_nRejectCount[0] = m_nRejectCount[1] = m_nRejectCount[2] = m_nRejectCount[3] = 0;

		::ZeroMemory(&tSend, sizeof(HSCDU_SND_DISPENSE_T));

		tSend.byCassette_1 = nCount1 & 0xFF;
		tSend.byCassette_2 = nCount2 & 0xFF;

		nRet = SndRcvPacket(HSCDU_CMD_DISPENSE, (BYTE *)&tSend, sizeof(HSCDU_SND_DISPENSE_T), m_szRxBuf);
		if(nRet < 0)
		{
			UnLocking();
			return nRet;
		}

		pHdr = (PHSCDU_HEADER_T) m_szRxBuf;
		pPacket = (PHSCDU_RESP_DISPENSE_T) &pHdr->byData;
		pResult = (PHSCDU_RESULT_INFO_T) pPacket->szResultInfo;

		if( pHdr->byResult == 0x46 )
		{
			nRet = CheckResult((char *)pPacket->szErrorCode);

			m_nOutCount[0] = pResult->casset_1.byRequired;
			m_nOutCount[1] = pResult->casset_2.byRequired;
			m_nOutCount[2] = pResult->casset_3.byRequired;
			m_nOutCount[3] = pResult->casset_4.byRequired;

			m_nRejectCount[0] = pResult->casset_1.byTotReject;
			m_nRejectCount[1] = pResult->casset_2.byTotReject;
			m_nRejectCount[2] = pResult->casset_3.byTotReject;
			m_nRejectCount[3] = pResult->casset_4.byTotReject;

			LOG_OUT("ErrorCode[%.*s]", sizeof(pPacket->szErrorCode), pPacket->szErrorCode);
			LOG_OUT("방출정보 [%4d/%4d : %4d/%4d : %4d/%4d : %4d/%4d]", 
					pResult->casset_1.byPass, pResult->casset_1.byRequired, 
					pResult->casset_2.byPass, pResult->casset_2.byRequired, 
					pResult->casset_3.byPass, pResult->casset_3.byRequired, 
					pResult->casset_4.byPass, pResult->casset_4.byRequired);
			LOG_OUT("불량지폐함정보 [%4d : %4d : %4d : %4d]", 
					pResult->casset_1.byTotReject, 
					pResult->casset_2.byTotReject, 
					pResult->casset_3.byTotReject, 
					pResult->casset_4.byTotReject);

			if( !GetEventCode(EC_CDU_NO_CHECK) )
			{
				LOG_OUT("지폐함 체크_1 - (%d/%d) (%d/%d) !!!!", m_nOutCount[0], nCount1, m_nOutCount[1], nCount2); 
				//if( pResult->casset_1.byPass != pResult->casset_1.byRequired  )
				if( nCount1 != m_nOutCount[0] )
				{
					SetCheckEventCode(EC_CDU_1K_NO_OUT, TRUE);
				}

				//if( pResult->casset_2.byPass != pResult->casset_2.byRequired )
				if( nCount2 != m_nOutCount[1] )
				{
					SetCheckEventCode(EC_CDU_10K_NO_OUT, TRUE);
				}

				if(	GetEventCode(EC_CDU_1K_NO_OUT) && GetEventCode(EC_CDU_10K_NO_OUT) )
				{
					SetCheckEventCode(EC_CDU_NO_OUT, TRUE);
					UnLocking();
					return -100;
				}
				if(	GetEventCode(EC_CDU_1K_NO_OUT) )
				{
					SetCheckEventCode(EC_CDU_NO_OUT, TRUE);
					UnLocking();
					return -101;
				}
				if(	GetEventCode(EC_CDU_10K_NO_OUT) )
				{
					SetCheckEventCode(EC_CDU_NO_OUT, TRUE);
					UnLocking();
					return -102;
				}
			}
			UnLocking();

			return -2;
		}

	// 	[17:59:10.770] [CCduHt::Dispense:631] 방출정보 [   9/   7 :    3/   3 :    0/   0 :    0/   0] - 
	// 	[17:59:10.770] [CCduHt::Dispense:636] 불량지폐함정보 [   2 :    0 :    0 :    0] - 

		LOG_OUT("방출정보 [%4d/%4d : %4d/%4d : %4d/%4d : %4d/%4d]", 
				pResult->casset_1.byPass, pResult->casset_1.byRequired, 
				pResult->casset_2.byPass, pResult->casset_2.byRequired, 
				pResult->casset_3.byPass, pResult->casset_3.byRequired, 
				pResult->casset_4.byPass, pResult->casset_4.byRequired);
		LOG_OUT("불량지폐함정보 [%4d : %4d : %4d : %4d]", 
				pResult->casset_1.byTotReject, 
				pResult->casset_2.byTotReject, 
				pResult->casset_3.byTotReject, 
				pResult->casset_4.byTotReject);

	// 	m_nOutCount[0] = pResult->casset_1.byPass;
	// 	m_nOutCount[1] = pResult->casset_2.byPass;
	// 	m_nOutCount[2] = pResult->casset_3.byPass;
	// 	m_nOutCount[3] = pResult->casset_4.byPass;
		m_nOutCount[0] = pResult->casset_1.byRequired;
		m_nOutCount[1] = pResult->casset_2.byRequired;
		m_nOutCount[2] = pResult->casset_3.byRequired;
		m_nOutCount[3] = pResult->casset_4.byRequired;

		m_nRejectCount[0] = pResult->casset_1.byReject1;
		m_nRejectCount[1] = pResult->casset_2.byReject2;
		m_nRejectCount[2] = pResult->casset_3.byReject3;
		m_nRejectCount[3] = pResult->casset_4.byReject4;

		if( !GetEventCode(EC_CDU_NO_CHECK) )
		{
			LOG_OUT("지폐함 체크_2 - (%d/%d) (%d/%d) !!!!", m_nOutCount[0], nCount1, m_nOutCount[1], nCount2); 

	//		if(pResult->casset_1.byPass != pResult->casset_1.byRequired)
			if(m_nOutCount[0] != nCount1)
			{
				SetCheckEventCode(EC_CDU_1K_NO_OUT, TRUE);
			}
			else
			{
				SetCheckEventCode(EC_CDU_1K_NO_OUT, FALSE);
			}

	//		if(pResult->casset_2.byPass != pResult->casset_2.byRequired)
			if(m_nOutCount[1] != nCount2)
			{
				SetCheckEventCode(EC_CDU_10K_NO_OUT, TRUE);
			}
			else
			{
				SetCheckEventCode(EC_CDU_10K_NO_OUT, FALSE);
			}

			if(	GetEventCode(EC_CDU_1K_NO_OUT) && GetEventCode(EC_CDU_10K_NO_OUT) )
			{
				SetCheckEventCode(EC_CDU_NO_OUT, TRUE);
				UnLocking();
				return -100;
			}
			if(	GetEventCode(EC_CDU_1K_NO_OUT) )
			{
				SetCheckEventCode(EC_CDU_NO_OUT, TRUE);
				UnLocking();
				return -101;
			}
			if(	GetEventCode(EC_CDU_10K_NO_OUT) )
			{
				SetCheckEventCode(EC_CDU_NO_OUT, TRUE);
				UnLocking();
				return -102;
			}
		}

		// 배열 00 ~ 12 : 배출(0  ~  2), Reject( 3 ~  9) -> 카세트 1번
		// 배열 13 ~ 25 : 배출(13 ~ 15), Reject(16 ~ 22) -> 카세트 2번
		// 배열 26 ~ 38 : 배출(26 ~ 28), Reject(29 ~ 35) -> 카세트 3번
		// 배열 39 ~ 51 : 배출(39 ~ 41), Reject(42 ~ 48) -> 카세트 4번

		// 	[3A][00]
		// 	[4F]
		// 	[30][30][30][30][30]
		// 	
		// 	[05][05][05][00][00][00][00][00][00][00][00][05][00]
		// 	[02][02][02][00][00][00][00][00][00][00][00][02][00]
		// 	[00][00][00][00][00][00][00][00][00][00][00][00][00]
		// 	[00][00][00][00][00][00][00][00][00][00][00][00][00]
		// 	
		// 	[03][46]
	}
	UnLocking();

	return nRet;
}

/**
 * @brief		TestDispense


 * @details		Test Dispense 명령 처리
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCduHt::GetDispenseInfo(char *pData)
{
	PDISPENSE_INFO_T pInfo;

	pInfo = (PDISPENSE_INFO_T) pData;

	pInfo->nOutCount[0] = m_nOutCount[0];
	pInfo->nOutCount[1] = m_nOutCount[1];
	pInfo->nOutCount[2] = m_nOutCount[2];
	pInfo->nOutCount[3] = m_nOutCount[3];

	pInfo->nRejectCount[0] = m_nRejectCount[0];
	pInfo->nRejectCount[1] = m_nRejectCount[1];
	pInfo->nRejectCount[2] = m_nRejectCount[2];
	pInfo->nRejectCount[3] = m_nRejectCount[3];

	return 0;
}

/**
 * @brief		TestDispense
 * @details		Test Dispense 명령 처리
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCduHt::TestDispense(int nCount1, int nCount2)
{
	int nRet;
	HSCDU_SND_DISPENSE_T  tSend;

	Locking();
	{
		::ZeroMemory(&tSend, sizeof(HSCDU_SND_DISPENSE_T));

		tSend.byCassette_1 = nCount1 & 0xFF;
		tSend.byCassette_2 = nCount2 & 0xFF;

		nRet = SndRcvPacket(HSCDU_CMD_TEST_DISPENSE, (BYTE *)&tSend, sizeof(HSCDU_SND_DISPENSE_T), m_szRxBuf);
		if(nRet < 0)
		{
			UnLocking();
			return nRet;
		}
	}
	UnLocking();

	return nRet;
}

/**
 * @brief		SetInfo
 * @details		Test Dispense 명령 처리
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCduHt::SetInfo(void)
{
	int nRet;
	HSCDU_SND_DISPENSE_T  sndPacket;

	Locking();
	{
		nRet = SndRcvPacket(HSCDU_CMD_TEST_DISPENSE, (BYTE *)&sndPacket, sizeof(HSCDU_SND_DISPENSE_T), m_szRxBuf);
		if(nRet < 0)
		{
			UnLocking();
			return nRet;
		}
	}
	UnLocking();

	return nRet;
}

/**
 * @brief		StartProcess
 * @details		Start
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCduHt::StartProcess(int nCommIdx)
{
	int		nRet;
	DWORD	dwThreadID;

	LOG_INIT();

	m_bConnected = FALSE;

	LOG_OUT("start !!!");

	EndProcess();

	nRet = m_clsComm.Open(nCommIdx, CBR_9600, 8, NOPARITY, ONESTOPBIT);
	if(nRet < 0) 
	{
		LOG_OUT("m_clsComm.Open() failure..");
		EndProcess();
		return -1;
	}

	m_hAccMutex = ::CreateMutex(NULL, FALSE, NULL);
	if(m_hAccMutex == NULL) 
	{
		LOG_OUT("CreateMutex() failure..");
		EndProcess();
		return -2;
	}

	m_bConnected = TRUE;

	m_hThread = ::CreateThread(NULL, 0, RunThread, this, CREATE_SUSPENDED, &dwThreadID);
	if(NULL == m_hThread)
	{
		LOG_OUT("Cannot create thread [%d]\n", ::GetLastError());
		m_bConnected = FALSE;
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
int CCduHt::EndProcess(void)
{
	LOG_OUT("start !!!");

	m_bConnected = FALSE;

	if(NULL != m_hThread)
	{
		::WaitForSingleObject(m_hThread, 500);
		::CloseHandle(m_hThread);

		m_hThread = NULL;
	}

	m_clsComm.Close();

	if(m_hAccMutex != NULL)
	{
		CloseHandle(m_hAccMutex);
		m_hAccMutex = NULL;
	}


	return 0;
}

/**
 * @brief		RunThread
 * @details		Start
 * @param		LPVOID lParam		CCduHt instance
 * @return		항상 : 0
 */
DWORD CCduHt::RunThread(LPVOID lParam)
{
	DWORD	dwTick;
	CCduHt *pClass = (CCduHt *)lParam;

	dwTick = ::GetTickCount();
	while(pClass->m_bConnected)
	{
		Sleep(10);

		if(dwTick == 0L)
		{
			dwTick = ::GetTickCount();
		}

		if( Util_CheckExpire(dwTick) >= 500 )	
		{
			dwTick = ::GetTickCount();
		}		
	}

	return 0;
}

