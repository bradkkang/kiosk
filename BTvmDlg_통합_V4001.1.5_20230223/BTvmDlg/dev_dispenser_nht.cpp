// 
// 
// dev_dispenser_nht.cpp : 신규 한틀 지폐방출기 (HSCDU2)
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "event_if.h"
#include "dev_dispenser_nht.h"
#include "dev_dispenser_main.h"


#define LOG_OUT(fmt, ...)		{ CCduNHt::m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ CCduNHt::m_clsLog.HexaDump(x, y, z); }

/**
 * @brief		CCduNHt
 * @details		생성자
 */
CCduNHt::CCduNHt()
{
	m_hAccMutex = NULL;
	m_bConnected = FALSE;
}

/**
 * @brief		~CCduNHt
 * @details		소멸자
 */
CCduNHt::~CCduNHt()
{

}

/**
 * @brief		LOG_INIT
 * @details		LOG 파일 초기화
 * @param		None
 * @return		None
 */
void CCduNHt::LOG_INIT(void)
{
	m_clsLog.SetData(30, "\\Log\\Cdu\\NHt");
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
int CCduNHt::Locking(void)
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
int CCduNHt::UnLocking(void)
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
int CCduNHt::SendChar(BYTE byData)
{
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
int CCduNHt::SendPacket(int nCommand, BYTE *pData, WORD wDataLen)
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
int CCduNHt::GetPacket(BYTE *retBuf)
{
	static WORD	wPktLen = 0;
	static BYTE	byBCC, byCalcBCC;
	static int	nState = 0, nRecv = 0, nDataCount = 0;
	int	ch, i;

	while(1) 
	{ 
		ch = m_clsComm.ReadData();
		if(ch < 0) 
		{
			return ch;
		}

		ch = ch & 0xFF;
		//LOG_OUT("ch = [%d], [%02X]\n", nState, ch & 0xFF);

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
			if(nRecv >= (MAX_NCDU_BUF - 1)) 
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
int CCduNHt::SndRcvPacket(int nCommand, BYTE *pData, int nDataLen, BYTE *retBuf)
{
	int	 nRet, i;
	DWORD dwTick;
	PNHTCDU_HEADER_T pHdr;
	PNHTCDU_RESP_COMMON_T pPacket;

	for(i = 0; i < MAX_NCDU_RETRY; i++)
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

	for(i = 0; i < MAX_NCDU_RETRY; i++)
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

	pHdr = (PNHTCDU_HEADER_T) retBuf;
	pPacket = (PNHTCDU_RESP_COMMON_T) &pHdr->byData;

	if(pHdr->byResult != 0x4F)
	{
		LOG_OUT("pHdr->byResult = %02X, failure !!", pHdr->byResult);
		//return -3;
		nRet = -3;
	}

	CheckResult((char *)pPacket->szErrorCode);

	return nRet;
}

/**
 * @brief		CheckResult
 * @details		에러코드 체크
 * @param		char *pErrString		: 에러코드
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCduNHt::CheckResult(char *pErrString)
{
	int nRet, i, nFind;
	NHTCDU_ERROR_TBL_T stErrTable[] = 
	{
		{ "40000", "Success" }, 
		{ "4000A", "통신 ACK/NAK 응답 없음" }, 
		{ "4000B", "COMMAND의 비정상적인 DATA 값 수신 각 항목의 범위 초과" }, 
		{ "4000C", "DIRECTION COMMAND 수신 TIMEOUT" }, 
		{ "4000D", "DIRECTION COMMAND 응답 순서 이상(SEQ)" }, 
		{ "40010", "FEED MOTOR ENCODER 이상" }, 
		{ "40011", "CIS부가 열려있음" }, 
		{ "40012", "REJECT BOX가 잠겨있지 않음" }, 
		{ "40013", "GATE가 방출 방향으로 동작 안함" }, 
		{ "40014", "GATE가 회수 방향으로 동작 안함" }, 
		{ "40015", "SHUTTER가 열리지 않음" }, 
		{ "40016", "SHUTTER가 닫히지 않음" }, 
		{ "40017", "SHUTTER센서 이상 열림과 닫힘이 동시에 감지됨" }, 
		{ "4001A", "연속REJECT 매수 오류" }, 
		{ "4001B", "총 REJECT 매수 오류" }, 
		{ "4001C", "과방출오류(요구매수 초과 방출)" }, 
		{ "40020", "VALIDATOR 설정 에러" }, 
		{ "40021", "VALIDATOR 응답 없음" }, 
		{ "40022", "VALIDATOR 응답 순서 이상(SEQ)" }, 
		{ "40023", "VALIDATOR 응답 이상 (BCC 불일치)" }, 
		{ "40024", "VALIDATOR CIS 보정 실패" }, 
		{ "40025", "VALIDATOR 프로그램 지정 오류" }, 
		{ "40026", "VALIDATOR 결과 수신 시간 초과" }, 
		{ "40027", "VALIDATOR 수신 순서 오류" }, 
		{ "40030", "방출중 SCAN_START 센서에 매체 잔류" }, 
		{ "40031", "방출중 GATE1/2 센서에 매체 잔류" }, 
		{ "40032", "방출중 EXIT1 센서에 매체 잔류" }, 
		{ "40033", "방출중 REJECT_IN 센서에 매체 잔류" }, 
		{ "40034", "방출중 1 번 카세트 SKEW1/2 센서에 매체 잔류" }, 
		{ "40035", "방출중 2 번 카세트 SKEW1/2 센서에 매체 잔류" }, 
		{ "40036", "방출중 3 번 카세트 SKEW1/2 센서에 매체 잔류" }, 
		{ "40037", "방출중 4 번 카세트 SKEW1/2 센서에 매체 잔류" }, 
		{ "40038", "방출중 5 번 카세트 SKEW1/2 센서에 매체 잔류" }, 
		{ "40039", "방출중 6 번 카세트 SKEW1/2 센서에 매체 잔류" }, 
		{ "40040", "방출중 SCAN_START 센서에 매체가 도달하지 못함" }, 
		{ "40041", "방출중 GATE1/2 센서에 매체가 도달하지 못함" }, 
		{ "40042", "방출중 EXIT1 센서에 매체가 도달하지 못함" }, 
		{ "40043", "방출중 REJECT_IN 센서에 매체가 도달하지 못함" }, 
		{ "40050", "방출중 SCAN_START 센서에 잘못된 매체 진입" }, 
		{ "40051", "방출중 GATE1/2 센서에 잘못된 매체 진입" }, 
		{ "40052", "방출중 EXIT1 센서에 잘못된 매체 진입" }, 
		{ "40053", "방출중 REJECT_IN 센서에 잘못된 매체 진입" }, 
		{ "40060", "초기화후 또는 방출 전 SCAN_START 센서에 매체 잔류" }, 
		{ "40061", "초기화후 또는 방출 전 GATE1(LEFT) 센서에 매체 잔류" }, 
		{ "40062", "초기화후 또는 방출 전 GATE2(RIGHT) 센서에 매체 잔류" }, 
		{ "40063", "초기화후 또는 방출 전 EXIT1 센서에 매체 잔류" }, 
		{ "40064", "초기화후 또는 방출 전 REJECT_IN 센서에 매체 잔류" }, 
		{ "40065", "초기화후 에스크로부에 매체 잔류 (SHUT_IN1)" }, 
		{ "40066", "초기화후 에스크로부에 매체 잔류 (SHUT_IN2)" }, 
		{ "40067", "초기화후 에스크로부에 매체 잔류 (SHUT_IN3)" }, 
		{ "40070", "초기화후 또는 방출 전 1 번 카세트 SKEW1(LEFT) 센서에 매체 잔류" }, 
		{ "40071", "초기화후 또는 방출 전 1 번 카세트 SKEW2(RIGHT) 센서에 매체 잔류" }, 
		{ "40072", "초기화후 또는 방출 전 2 번 카세트 SKEW1(LEFT) 센서에 매체 잔류" }, 
		{ "40073", "초기화후 또는 방출 전 2 번 카세트 SKEW2(RIGHT) 센서에 매체 잔류" }, 
		{ "40074", "초기화후 또는 방출 전 3 번 카세트 SKEW1(LEFT) 센서에 매체 잔류" }, 
		{ "40075", "초기화후 또는 방출 전 3 번 카세트 SKEW2(RIGHT) 센서에 매체 잔류" }, 
		{ "40076", "초기화후 또는 방출 전 4 번 카세트 SKEW1(LEFT) 센서에 매체 잔류" }, 
		{ "40077", "초기화후 또는 방출 전 4 번 카세트 SKEW2(RIGHT) 센서에 매체 잔류" }, 
		{ "40078", "초기화후 또는 방출 전 5 번 카세트 SKEW1(LEFT) 센서에 매체 잔류" }, 
		{ "40079", "초기화후 또는 방출 전 5 번 카세트 SKEW2(RIGHT) 센서에 매체 잔류" }, 
		{ "4007A", "초기화후 또는 방출 전 6 번 카세트 SKEW1(LEFT) 센서에 매체 잔류" }, 
		{ "4007B", "초기화후 또는 방출 전 6 번 카세트 SKEW2(RIGHT) 센서에 매체 잔류" }, 
		{ "40080", "1번 카세트 픽업 실패 카세트에 매체는 존재하는 상태" }, 
		{ "40081", "2번 카세트 픽업 실패 카세트에 매체는 존재하는 상태" }, 
		{ "40082", "3번 카세트 픽업 실패 카세트에 매체는 존재하는 상태" }, 
		{ "40083", "4번 카세트 픽업 실패 카세트에 매체는 존재하는 상태" }, 
		{ "40084", "5번 카세트 픽업 실패 카세트에 매체는 존재하는 상태" }, 
		{ "40085", "6번 카세트 픽업 실패 카세트에 매체는 존재하는 상태" }, 
		{ "40088", "1번 카세트 매체 없음 또는 픽업 실패" }, 
		{ "40089", "2번 카세트 매체 없음 또는 픽업 실패" }, 
		{ "4008A", "3번 카세트 매체 없음 또는 픽업 실패" }, 
		{ "4008B", "4번 카세트 매체 없음 또는 픽업 실패" }, 
		{ "4008C", "5번 카세트 매체 없음 또는 픽업 실패" }, 
		{ "4008D", "6번 카세트 매체 없음 또는 픽업 실패" }, 
		{ "40090", "1번 카세트 장착 안됨" }, 
		{ "40091", "2번 카세트 장착 안됨" }, 
		{ "40092", "3번 카세트 장착 안됨" }, 
		{ "40093", "4번 카세트 장착 안됨" }, 
		{ "40094", "5번 카세트 장착 안됨" }, 
		{ "40095", "6번 카세트 장착 안됨" }, 
	};

	LOG_OUT("Happen ErrorCode = %.*s", 5, pErrString);

	nFind = -1;
	for(i = 0; i < sizeof(stErrTable) / sizeof(NHTCDU_ERROR_TBL_T); i++)
	{
		if(memcmp(pErrString, stErrTable[i].pCode, 5) == 0) 
		{
			LOG_OUT("Find ErrorCode = %s, %s", stErrTable[i].pCode, stErrTable[i].pStr);
			nFind = i;
			break;
		}
	}

	if(nFind < 0)
	{
		return -1;
	}

	if( nFind == 0 )
	{
		SetCheckEventCode(EC_CDU_DEV_ERR, FALSE);
	}
	else
	{
		SetCheckEventCode(EC_CDU_DEV_ERR, TRUE);
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
int CCduNHt::Initialize(int nInitFlag)
{
	int nRet;
	PNHTCDU_HEADER_T	pHdr;
	PNHTCDU_RESP_INIT_T pPacket;

	Locking();
	{
		nRet = SndRcvPacket(NHTCDU_CMD_INIT, (BYTE *)&nInitFlag, 1, m_szRxBuf);
		if(nRet < 0)
		{
			UnLocking();
			return nRet;
		}

		pHdr = (PNHTCDU_HEADER_T) m_szRxBuf;
		pPacket = (PNHTCDU_RESP_INIT_T) &pHdr->byData;

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
 * @brief		SensorStatus
 * @details		센서 상태 명령 처리
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCduNHt::SensorStatus(void)
{
	int nRet;
	PNHTCDU_HEADER_T	pHdr;
	PNHTCDU_RESP_STATUS_T pPacket;

	Locking();
	{
		nRet = SndRcvPacket(NHTCDU_CMD_STATUS, (BYTE *)NULL, 0, m_szRxBuf);
		if(nRet < 0)
		{
			UnLocking();
			return nRet;
		}

		// 상태
		//SetCheckEventCode(EC_CDU_COMM_ERR, FALSE);
	
		pHdr = (PNHTCDU_HEADER_T) m_szRxBuf;
		pPacket = (PNHTCDU_RESP_STATUS_T) &pHdr->byData;

		LOG_HEXA("Sensor Value ", pPacket->szSensorData, sizeof(pPacket->szSensorData));

		//SetCheckEventCode(EC_CDU_COMM_ERR, TRUE);
	}
	UnLocking();

	return nRet;
}

/**
 * @brief		GetStatus
 * @details		센서 상태 명령 처리
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCduNHt::GetStatus(void)
{
	int nRet;
	static int nRetry = 0;
	PNHTCDU_HEADER_T	pHdr;
	PNHTCDU_RESP_STATUS_T pPacket;

	Locking();
	{
		nRet = SndRcvPacket(NHTCDU_CMD_STATUS, (BYTE *)NULL, 0, m_szRxBuf);
		if(nRet < 0)
		{
			if(++nRetry >= 3)
			{
				SetCheckEventCode(EC_CDU_COMM_ERR, TRUE);
			}
			UnLocking();
			return nRet;
		}

		pHdr = (PNHTCDU_HEADER_T) m_szRxBuf;
		pPacket = (PNHTCDU_RESP_STATUS_T) &pHdr->byData;

		LOG_HEXA("GetStatus()", pPacket->szSensorData, sizeof(pPacket->szSensorData));

		if(nRetry > 0)
		{
			nRetry = 0;
		}
		SetCheckEventCode(EC_CDU_COMM_ERR, FALSE);
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
int CCduNHt::GetVersion(void)
{
	int nRet;
	PNHTCDU_HEADER_T	pHdr;
	PNHTCDU_RESP_VERSION_T pPacket;

	Locking();
	{
		nRet = SndRcvPacket(NHTCDU_CMD_VERSION, (BYTE *)NULL, 0, m_szRxBuf);
		if(nRet < 0)
		{
			UnLocking();
			return nRet;
		}

		pHdr = (PNHTCDU_HEADER_T) m_szRxBuf;
		pPacket = (PNHTCDU_RESP_VERSION_T) &pHdr->byData;

		LOG_OUT("Unit Name = %s \n", pPacket->szUnitName);
		LOG_OUT("Country Info = %c \n", pPacket->byCountry);
		LOG_OUT("Cassette Info = %c \n", pPacket->byCassette);
		LOG_OUT("CDU type = %02X \n", pPacket->byType);
		LOG_OUT("Version = %.*s \n", sizeof(pPacket->szVersion), pPacket->szVersion);
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
int CCduNHt::Dispense(int nCount1, int nCount2)
{
	int nRet;
	NHTCDU_SND_DISPENSE_T  tSend;
	PNHTCDU_HEADER_T	pHdr;
	PNHTCDU_RESP_DISPENSE_T pPacket;
	PNHTCDU_RESULT_INFO_T pResult;

	Locking();
	{
		m_nOutCount[0] = m_nOutCount[1]	= m_nOutCount[2] = m_nOutCount[3] = 0;
		m_nRejectCount[0] = m_nRejectCount[1] = m_nRejectCount[2] = m_nRejectCount[3] = 0;

		ZeroMemory(&tSend, sizeof(NHTCDU_SND_DISPENSE_T));
		ZeroMemory(&m_szRxBuf, sizeof(m_szRxBuf));

		tSend.byCassette_1 = nCount1 & 0xFF;
		tSend.byCassette_2 = nCount2 & 0xFF;

		nRet = SndRcvPacket(NHTCDU_CMD_DISPENSE, (BYTE *)&tSend, sizeof(NHTCDU_SND_DISPENSE_T), m_szRxBuf);
		if(nRet < 0)
		{
			//return nRet;
		}

		pHdr = (PNHTCDU_HEADER_T) m_szRxBuf;
		pPacket = (PNHTCDU_RESP_DISPENSE_T) &pHdr->byData;
		pResult = (PNHTCDU_RESULT_INFO_T) pPacket->szResultInfo;

		if( pHdr->byResult == 0x46 )
		{
			//nRet = CheckResult((char *)pPacket->szErrorCode);

			m_nOutCount[0] = pResult->casset_1.byTotalOutCount;
			m_nOutCount[1] = pResult->casset_2.byTotalOutCount;
			m_nOutCount[2] = pResult->casset_3.byTotalOutCount;
			m_nOutCount[3] = pResult->casset_4.byTotalOutCount;

			m_nRejectCount[0] = pResult->casset_1.byTotalRejectCount;
			m_nRejectCount[1] = pResult->casset_2.byTotalRejectCount;
			m_nRejectCount[2] = pResult->casset_3.byTotalRejectCount;
			m_nRejectCount[3] = pResult->casset_4.byTotalRejectCount;

			LOG_OUT("#FAIL_방출정보 [%4d : %4d : %4d : %4d]", 
				pResult->casset_1.byTotalOutCount, 
				pResult->casset_2.byTotalOutCount, 
				pResult->casset_3.byTotalOutCount, 
				pResult->casset_4.byTotalOutCount);
			LOG_OUT("#FAIL_불량지폐함정보 [%4d : %4d : %4d : %4d]", 
				pResult->casset_1.byTotalRejectCount, 
				pResult->casset_2.byTotalRejectCount, 
				pResult->casset_3.byTotalRejectCount, 
				pResult->casset_4.byTotalRejectCount);

			if(pResult->casset_1.byReqCount != pResult->casset_1.byTotalOutCount)
			{
				SetCheckEventCode(EC_CDU_1K_NO_OUT, TRUE);
			}
			if(pResult->casset_2.byReqCount != pResult->casset_2.byTotalOutCount)
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
			UnLocking();
			return -2;
		}

		LOG_OUT("#SUCC_방출정보 [%4d : %4d : %4d : %4d]", 
			pResult->casset_1.byTotalOutCount, 
			pResult->casset_2.byTotalOutCount, 
			pResult->casset_3.byTotalOutCount, 
			pResult->casset_4.byTotalOutCount);
		LOG_OUT("#SUCC_불량지폐함정보 [%4d : %4d : %4d : %4d]", 
			pResult->casset_1.byTotalRejectCount, 
			pResult->casset_2.byTotalRejectCount, 
			pResult->casset_3.byTotalRejectCount, 
			pResult->casset_4.byTotalRejectCount);

		m_nOutCount[0] = pResult->casset_1.byTotalOutCount;
		m_nOutCount[1] = pResult->casset_2.byTotalOutCount;
		m_nOutCount[2] = pResult->casset_3.byTotalOutCount;
		m_nOutCount[3] = pResult->casset_4.byTotalOutCount;

		m_nRejectCount[0] = pResult->casset_1.byTotalRejectCount;
		m_nRejectCount[1] = pResult->casset_2.byTotalRejectCount;
		m_nRejectCount[2] = pResult->casset_3.byTotalRejectCount;
		m_nRejectCount[3] = pResult->casset_4.byTotalRejectCount;

		//	if(pResult->casset_1.byReqCount != pResult->casset_1.byTotalOutCount)
		if(nCount1 != pResult->casset_1.byTotalOutCount)
		{
			SetCheckEventCode(EC_CDU_1K_NO_OUT, TRUE);
		}
		//	if(pResult->casset_2.byReqCount != pResult->casset_2.byTotalOutCount)
		if(nCount2 != pResult->casset_2.byTotalOutCount)
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

	return nRet;
}

/**
 * @brief		GetDispenseInfo
 * @details		Dispense 정보 전달
 * @param		char *pData		Dispense info
 * @return		성공 : > 0, 실패 : < 0
 */
int CCduNHt::GetDispenseInfo(char *pData)
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
int CCduNHt::TestDispense(int nCount1, int nCount2)
{
	int nRet;
	NHTCDU_SND_DISPENSE_T  tSend;
	PNHTCDU_HEADER_T	pHdr;
	PNHTCDU_RESP_DISPENSE_T pPacket;
	PNHTCDU_RESULT_INFO_T pResult;

	Locking();
	{
		m_nOutCount[0] = m_nOutCount[1]	= m_nOutCount[2] = m_nOutCount[3] = 0;
		m_nRejectCount[0] = m_nRejectCount[1] = m_nRejectCount[2] = m_nRejectCount[3] = 0;

		ZeroMemory(&tSend, sizeof(NHTCDU_SND_DISPENSE_T));

		tSend.byCassette_1 = nCount1 & 0xFF;
		tSend.byCassette_2 = nCount2 & 0xFF;

		nRet = SndRcvPacket(NHTCDU_CMD_TEST_DISPENSE, (BYTE *)&tSend, sizeof(NHTCDU_SND_DISPENSE_T), m_szRxBuf);
		if(nRet < 0)
		{
			UnLocking();
			return nRet;
		}

		pHdr = (PNHTCDU_HEADER_T) m_szRxBuf;
		pPacket = (PNHTCDU_RESP_DISPENSE_T) &pHdr->byData;
		pResult = (PNHTCDU_RESULT_INFO_T) pPacket->szResultInfo;

		if( pHdr->byResult == 0x46 ) /// 'F' : 실패
		{
			//nRet = CheckResult((char *)pPacket->szErrorCode);

			m_nOutCount[0] = pResult->casset_1.byTotalOutCount;
			m_nOutCount[1] = pResult->casset_2.byTotalOutCount;
			m_nOutCount[2] = pResult->casset_3.byTotalOutCount;
			m_nOutCount[3] = pResult->casset_4.byTotalOutCount;

			m_nRejectCount[0] = pResult->casset_1.byTotalRejectCount;
			m_nRejectCount[1] = pResult->casset_2.byTotalRejectCount;
			m_nRejectCount[2] = pResult->casset_3.byTotalRejectCount;
			m_nRejectCount[3] = pResult->casset_4.byTotalRejectCount;

			LOG_OUT("#FAIL_ErrorCode[%.*s]", sizeof(pPacket->szErrorCode), pPacket->szErrorCode);
			LOG_OUT("#FAIL_방출정보 [%4d : %4d : %4d : %4d : %4d : %4d]", 
				pResult->casset_1.byTotalOutCount, 
				pResult->casset_2.byTotalOutCount, 
				pResult->casset_3.byTotalOutCount, 
				pResult->casset_4.byTotalOutCount, 
				pResult->casset_5.byTotalOutCount, 
				pResult->casset_6.byTotalOutCount);
			LOG_OUT("#FAIL_불량지폐함정보 [%4d : %4d : %4d : %4d : %4d : %4d]", 
				pResult->casset_1.byTotalRejectCount, 
				pResult->casset_2.byTotalRejectCount, 
				pResult->casset_3.byTotalRejectCount, 
				pResult->casset_4.byTotalRejectCount, 
				pResult->casset_5.byTotalRejectCount, 
				pResult->casset_6.byTotalRejectCount);

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

		LOG_OUT("#SUCC_ErrorCode[%.*s]", sizeof(pPacket->szErrorCode), pPacket->szErrorCode);
		LOG_OUT("#SUCC_방출정보 [%4d : %4d : %4d : %4d : %4d : %4d]", 
			pResult->casset_1.byTotalOutCount, 
			pResult->casset_2.byTotalOutCount, 
			pResult->casset_3.byTotalOutCount, 
			pResult->casset_4.byTotalOutCount, 
			pResult->casset_5.byTotalOutCount, 
			pResult->casset_6.byTotalOutCount);
		LOG_OUT("#SUCC_불량지폐함정보 [%4d : %4d : %4d : %4d : %4d : %4d]", 
			pResult->casset_1.byTotalRejectCount, 
			pResult->casset_2.byTotalRejectCount, 
			pResult->casset_3.byTotalRejectCount, 
			pResult->casset_4.byTotalRejectCount, 
			pResult->casset_5.byTotalRejectCount, 
			pResult->casset_6.byTotalRejectCount);

		m_nOutCount[0] = pResult->casset_1.byTotalOutCount;
		m_nOutCount[1] = pResult->casset_2.byTotalOutCount;
		m_nOutCount[2] = pResult->casset_3.byTotalOutCount;
		m_nOutCount[3] = pResult->casset_4.byTotalOutCount;

		m_nRejectCount[0] = pResult->casset_1.byTotalRejectCount;
		m_nRejectCount[1] = pResult->casset_2.byTotalRejectCount;
		m_nRejectCount[2] = pResult->casset_3.byTotalRejectCount;
		m_nRejectCount[3] = pResult->casset_4.byTotalRejectCount;

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
int CCduNHt::SetInfo(void)
{
	int nRet;
	NHTCDU_SND_DISPENSE_T  sndPacket;

	Locking();
	{
		nRet = SndRcvPacket(NHTCDU_CMD_TEST_DISPENSE, (BYTE *)&sndPacket, sizeof(NHTCDU_SND_DISPENSE_T), m_szRxBuf);
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
int CCduNHt::StartProcess(int nCommIdx)
{
	int		nRet;
	DWORD	dwThreadID;

	m_bConnected = FALSE;

	LOG_INIT();

	nRet = m_clsComm.Open(nCommIdx, CBR_9600, 8, NOPARITY, ONESTOPBIT);
	if(nRet < 0) 
	{
		LOG_OUT("m_clsComm.Open(port = %d) failure, nRet(%d)..", nCommIdx, nRet);
		return -1;
	}

	m_hAccMutex = ::CreateMutex(NULL, FALSE, NULL);
	if(m_hAccMutex == NULL) 
	{
		//LOG_OUT("[%s:%d] CreateMutex() failure..\n", __FUNCTION__, __LINE__);
		m_clsComm.Close();
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
int CCduNHt::EndProcess(void)
{
	if(m_bConnected == TRUE)
	{
		m_clsComm.Close();
	}

	if(m_hAccMutex != NULL)
	{
		CloseHandle(m_hAccMutex);
		m_hAccMutex = NULL;
	}

	m_bConnected = FALSE;

	return 0;
}

/**
 * @brief		RunThread
 * @details		Start
 * @param		LPVOID lParam		CCduHt instance
 * @return		항상 : 0
 */
DWORD CCduNHt::RunThread(LPVOID lParam)
{
	DWORD	dwTick;
	CCduNHt *pClass = (CCduNHt *)lParam;

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
