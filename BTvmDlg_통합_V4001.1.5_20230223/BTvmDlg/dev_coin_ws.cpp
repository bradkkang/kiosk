// 
// 
// CoinWS.cpp : 동전방출기 (Woo Sung)
// 	1) 'o' : 코인 배출 Command
// 	2) 'r' : 호퍼 Reset Command
// 	3) 's' : Hopper Status 요청 Command
// 	4) 't' : Hopper Error 상태 배출수량 요청
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "dev_coin_ws.h"
#include "event_if.h"

//----------------------------------------------------------------------------------------------------------------------

#define LOG_OUT(fmt, ...)		{ CCoinWS::m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ CCoinWS::m_clsLog.HexaDump(x, y, z); }

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CCoinWS
 * @details		생성자
 */
CCoinWS::CCoinWS()
{
	m_hThread = NULL;
	m_hAccMutex = NULL;
	m_bConnected = FALSE;

	m_nOutCount = 0;

	m_bLog = TRUE;
}

/**
 * @brief		~CCoinWS
 * @details		소멸자
 */
CCoinWS::~CCoinWS()
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
void CCoinWS::LOG_INIT(int nFlag)
{
	if(nFlag == 100)
	{
		m_clsLog.SetData(30, "\\Log\\Coin\\WS\\w100");
	}
	else
	{
		m_clsLog.SetData(30, "\\Log\\Coin\\WS\\w500");
	}
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
int CCoinWS::Locking(void)
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
int CCoinWS::UnLocking(void)
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
int CCoinWS::SendChar(BYTE byData)
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
int CCoinWS::SendPacket(int nCommand, BYTE *pData, WORD wDataLen)
{
	int		nOff, nRet, i;
	BYTE	byBCC;

	UNUSED_VAR(pData);
	UNUSED_VAR(wDataLen);

	nOff = 0;

	::ZeroMemory(m_szTxBuf, sizeof(m_szTxBuf));

	// (01). stx
	m_szTxBuf[nOff++] = CHAR_STX;
	// (02). address 
	CopyMemory(&m_szTxBuf[nOff], "01", 2);
	nOff += 2;
	// (03). Command
	m_szTxBuf[nOff++] = nCommand & 0xFF;
	// (04). Data
	if(wDataLen > 0)
	{
		CopyMemory(&m_szTxBuf[nOff], pData, wDataLen);
		nOff += wDataLen;

		// (05). BCC
		byBCC = 0;
		for(i = 1; i < nOff; i++)
		{
			byBCC ^= (m_szTxBuf[i] & 0xFF);
		}
		m_szTxBuf[nOff++] = byBCC;
	}
	// (06). ETX
	m_szTxBuf[nOff++] = CHAR_ETX;

	nRet = m_clsComm.SendData(m_szTxBuf, nOff);

	if(m_bLog == TRUE)
	{
		LOG_HEXA("SendPacket", m_szTxBuf, nOff);
	}

	return nRet;
}

/**
 * @brief		GetPacket
 * @details		get data
 * @param		BYTE *retBuf	receive data
 * @return		성공 : > 0, 실패 : < 0
 */
int CCoinWS::GetPacket(BYTE *retBuf)
{
	static WORD	wPktLen;
	static int	nState, nRecv, nDataCount;
	BYTE byBCC;
	int	ch, i;

	while(1) 
	{ 
		ch = m_clsComm.ReadData();
		if(ch < 0) 
		{
			return ch;
		}

		ch = ch & 0xFF;

		switch(nState) 
		{
		case 0 :	// stx
			nState = nRecv = wPktLen = nDataCount = 0;
			if(ch == CHAR_STX)
			{
				wPktLen = nDataCount = 0;
				nState++;
			}
			break;

		case 1:		// address_high
			retBuf[nRecv++] = ch;
			wPktLen = (ch << 8);
			nState++;
			break;

		case 2 :	// address_low
			retBuf[nRecv++] = ch;
			wPktLen |= (ch & 0xFF);
			nState++;
			break;

		case 3 :	// command
			retBuf[nRecv++] = ch;
			nState++;
			break;

		case 4 :	// data
			retBuf[nRecv++] = ch;
			nState++;
			break;

		case 5 :	// bcc
			retBuf[nRecv++] = ch;
			nState++;
			break;

		case 6 :	// etx
			if(ch == CHAR_ETX)
			{
				goto got_packet;
			}
			break;
		}
	}

got_packet:
	
	nState = wPktLen = nDataCount = 0;

	byBCC = 0;
	for(i = 0; i < nRecv; i++)
	{
		byBCC ^= retBuf[i];
	}

	if(byBCC != 0) 
	{
		LOG_OUT("checksum ERROR[%02X, %02X]..", byBCC, retBuf[nRecv - 1]);
		return -1;
	}

	if(m_bLog == TRUE)
	{
		LOG_HEXA("GetPacket", retBuf, nRecv);
	}

	return nRecv;
}

/**
 * @brief		SndRcvPacket
 * @details		데이타 송.수신 데이타 처리
 * @param		BYTE *pCommand, BYTE *sData, int sLen, BYTE *retBuf
 * @return		성공 : > 0, 실패 : < 0
 */
int CCoinWS::SndRcvPacket(int nCommand, BYTE *pData, int nDataLen, BYTE *retBuf)
{
	int	 nRet;
	DWORD dwTick;
	PCOIN_WS_HEADER_T  pHdr;

	// (01). send packet
	nRet = SendPacket(nCommand, pData, (WORD)nDataLen);

	// (02). recv ACK
	dwTick = ::GetTickCount();
		
	while( Util_CheckExpire(dwTick) < MAX_CON_WS_TIMEOUT )	
	{
		nRet = GetPacket(m_szRxBuf);
		if(nRet > 0)
		{
			goto result_code;
		}
	}

	LOG_OUT("Timeout !!!");
	return -1;

result_code:

	pHdr = (PCOIN_WS_HEADER_T) retBuf;

	if(memcmp(pHdr->szAddr, "01", 2) != 0)
	{
		LOG_OUT("Address Error(%02X, %02X) !!!", pHdr->szAddr[0] & 0xFF, pHdr->szAddr[1] & 0xFF);
		return -2;
	}

	return nRet;
}

/**
 * @brief		SetCoinStatus
 * @details		코인 상태값 설정
 * @param		BYTE bStatus	: 코인 상태값
 * @return		항상 = 0
 */
int CCoinWS::SetCoinStatus(BYTE byStatus)
{
	if(m_byStatus == byStatus)
	{
		return 0;
	}
	m_byStatus = byStatus;

	if(byStatus & 0x01)
	{	// 호퍼 초기화 상태 : 전원 투입되면 3초간 유지
		LOG_OUT("bit_0 : 호퍼 초기화 상태...");
	}
	else
	{
	}

	if(byStatus & 0x02)
	{	// Coin 부족 상태 : 동전없으면 1
		LOG_OUT("bit_1 : 동전없음 상태...");

		if(m_nType == COIN_100_TYPE)
		{
			SetCheckEventCode(EC_COIN_100_SHORT, TRUE);
		}
		else
		{
			SetCheckEventCode(EC_COIN_500_SHORT, TRUE);
		}
	}
	else
	{
		if(m_nType == COIN_100_TYPE)
		{
			SetCheckEventCode(EC_COIN_100_SHORT, FALSE);
		}
		else
		{
			SetCheckEventCode(EC_COIN_500_SHORT, FALSE);
		}
	}

	if(byStatus & 0x04)
	{	// 모터 에러 상태 : 모터 이상 1
		LOG_OUT("bit_2 : 모터 에러 상태...");

		if(m_nType == COIN_100_TYPE)
		{
			SetCheckEventCode(EC_COIN_100_MOTOR_ERR, TRUE);
		}
		else
		{
			SetCheckEventCode(EC_COIN_500_MOTOR_ERR, TRUE);
		}
	}
	else
	{
		if(m_nType == COIN_100_TYPE)
		{
			SetCheckEventCode(EC_COIN_100_MOTOR_ERR, FALSE);
		}
		else
		{
			SetCheckEventCode(EC_COIN_500_MOTOR_ERR, FALSE);
		}
	}

	if(byStatus & 0x08)
	{	// 프로토콜 상태 : 비정상이면 1
		LOG_OUT("bit_3 : 통신 오류 상태...");

		if(m_nType == COIN_100_TYPE)
		{
			SetCheckEventCode(EC_COIN_100_PROTOCOL_ERR, TRUE);
		}
		else
		{
			SetCheckEventCode(EC_COIN_500_PROTOCOL_ERR, TRUE);
		}
	}
	else
	{
		if(m_nType == COIN_100_TYPE)
		{
			SetCheckEventCode(EC_COIN_100_PROTOCOL_ERR, FALSE);
		}
		else
		{
			SetCheckEventCode(EC_COIN_500_PROTOCOL_ERR, FALSE);
		}
	}

	if(byStatus & 0x10)
	{	// Coin 방출 상태 : 방출중이면 1
		LOG_OUT("bit_4 : 동전 방출 중 상태...");
	}
	else
	{
		;
	}

	if(byStatus & 0x20)
	{	// 동전 방출 종료 상태 : 배출 종료이면 1
		LOG_OUT("bit_5 : 동전 방출 종료 상태...");
	}
	else
	{
		;
	}

	if(byStatus & 0x40)
	{	// 동전 방출 에러 상태 : 동전 방출 중 모터 또는 코인부족으로 에러가 발생하면 1
		LOG_OUT("bit_6 : 동전 방출 에러 상태...");

		if(m_nType == COIN_100_TYPE)
		{
			SetCheckEventCode(EC_COIN_100_OUT_ERR, TRUE);
		}
		else
		{
			SetCheckEventCode(EC_COIN_500_OUT_ERR, TRUE);
		}
	}
	else
	{
		if(m_nType == COIN_100_TYPE)
		{
			SetCheckEventCode(EC_COIN_100_OUT_ERR, FALSE);
		}
		else
		{
			SetCheckEventCode(EC_COIN_500_OUT_ERR, FALSE);
		}
	}

	if(byStatus & 0x80)
	{	// 호퍼 Enable 상태 : 전원 투입후 's' 명령을 수신하면 1
		if(m_nType == COIN_100_TYPE)
		{
			SetCheckEventCode(EC_COIN_100_ENABLE_ERR, FALSE);
		}
		else
		{
			SetCheckEventCode(EC_COIN_500_ENABLE_ERR, FALSE);
		}
	}
	else
	{
		LOG_OUT("bit_7 : 호퍼 Disable 상태...");

		if(m_nType == COIN_100_TYPE)
		{
			SetCheckEventCode(EC_COIN_100_ENABLE_ERR, TRUE);
		}
		else
		{
			SetCheckEventCode(EC_COIN_500_ENABLE_ERR, TRUE);
		}
	}
	

	return 0;
}

/**
 * @brief		OutCoin
 * @details		코인 배출 명령 처리 ('o' : 0x6F)
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCoinWS::OutCoin(int nCount)
{
	int nRet, nResult;
//	int nLoop, i;
	BYTE byCount, byOutCoin;
	DWORD dwTick;
	PCOIN_WS_HEADER_T  pHdr;

	m_nOutCount = 0;

	LOG_OUT("OutCoin() start, nCount(%d) ..", nCount);

	Locking();

	byCount = nCount & 0xFF;

	nResult = -1;

	nRet = SendPacket(SCMD_OUT_COIN, (BYTE *)&byCount, 1);
	dwTick = ::GetTickCount();
	while( Util_CheckExpire(dwTick) < (1000 * 10) )	
	{
		nRet = GetPacket(m_szRxBuf);
		if(nRet > 0)
		{
			pHdr = (PCOIN_WS_HEADER_T) m_szRxBuf;

			if(pHdr->byCommand == RCMD_OUT_COIN)  // 0x4F (1)
			{
				dwTick = ::GetTickCount();
				continue;
			}
			else if(pHdr->byCommand == RCMD_COIN_COUNT)	// 0x54 (2)
			{
				dwTick = ::GetTickCount();
				continue;
			}
			else if(pHdr->byCommand == RCMD_OUT_COIN_END) // 0x46 (3-0)
			{	// 정상 배출
				byOutCoin = pHdr->byData;
				LOG_OUT("배출 수량 (0x%02X) : (0x%02X) !!!", byOutCoin & 0xFF, byCount & 0xFF);
				
				m_nOutCount = byOutCoin;

				if(pHdr->byData == byCount)
				{	// 정상 (정상 배출)
					nResult = 0;
				}
				else
				{	// 에러 (방출갯수 에러)
					LOG_OUT("배출 수량 에러 (0x%02X) : (0x%02X) !!!", byOutCoin & 0xFF, byCount & 0xFF);
					nResult = -2;
				}
				break;
			}
			else if(pHdr->byCommand == RCMD_ERR_COIN) // 0x45 (3-1)
			{	// 배출 에러
				LOG_OUT("배출 에러1, Command(0x%02X) !!!", pHdr->byCommand & 0xFF);
				byOutCoin = pHdr->byData;
				m_nOutCount = byOutCoin;
				nResult = -3;
				break;
			}
			else if(pHdr->byCommand == RCMD_STATUS) // 0x45 (3-2)
			{	// 배출 에러
				LOG_OUT("배출 에러2, Command(0x%02X) !!!", pHdr->byCommand & 0xFF);
				SetCoinStatus(pHdr->byData);
				nResult = -4;
				break;
			}
		}
	}

	if(nResult < 0)
	{
		if( GetEventCode(EC_COIN_NO_CHECK) )
		{
			;
		}
		else
		{
			// 미방출에 대한 사항
			if(nCount != m_nOutCount)
			{
				if(m_nType == COIN_100_TYPE)
				{
					SetCheckEventCode(EC_COIN_100_NO_OUT, TRUE);
				}
				else
				{
					SetCheckEventCode(EC_COIN_500_NO_OUT, TRUE);
				}

				SetCheckEventCode(EC_COIN_NO_OUT, TRUE);
			}
		}
		UnLocking();
		LOG_OUT("OutCoin() end1 ..");
		return nResult;
	}

	if(m_nType == COIN_100_TYPE)
	{
		SetCheckEventCode(EC_COIN_100_NO_OUT, FALSE);
	}
	else
	{
		SetCheckEventCode(EC_COIN_500_NO_OUT, FALSE);
	}

	LOG_OUT("%s", "SetCheckEventCode(EC_COIN_NO_OUT, FALSE) !!!!");

	//SetCheckEventCode(EC_COIN_NO_OUT, FALSE);
	UnLocking();
	LOG_OUT("OutCoin() end ..");

	return nResult;
}

/***
int CCoinWS::OutCoin(int nCount)
{
	int nRet, nResult;
	int nLoop, i, nValue;
	int nMAX = 250;
	BYTE byCount, byOutCoin;
	DWORD dwTick;
	PCOIN_WS_HEADER_T  pHdr;

	m_nOutCount = 0;

	Locking();

	nLoop = nCount / nMAX;
	if( (nCount % nMAX) > 0 )
	{
		nLoop += 1;
	}

	for(i = 0; i < nLoop; i++)
	{
		if( i == (nLoop - 1) )
		{
			nValue = nCount % nMAX;
		}
		else
		{
			nValue = nMAX;
		}
		
		//byCount = nCount & 0xFF;
		byCount = nValue & 0xFF;

		nResult = -1;

		nRet = SendPacket(SCMD_OUT_COIN, (BYTE *)&byCount, 1);
		dwTick = ::GetTickCount();
		while( Util_CheckExpire(dwTick) < (1000 * 10) )	
		{
			nRet = GetPacket(m_szRxBuf);
			if(nRet > 0)
			{
				pHdr = (PCOIN_WS_HEADER_T) m_szRxBuf;

				if(pHdr->byCommand == RCMD_OUT_COIN)  // 0x4F (1)
				{
					dwTick = ::GetTickCount();
					continue;
				}
				else if(pHdr->byCommand == RCMD_COIN_COUNT)	// 0x54 (2)
				{
					dwTick = ::GetTickCount();
					continue;
				}
				else if(pHdr->byCommand == RCMD_OUT_COIN_END) // 0x46 (3-0)
				{	// 정상 배출
					byOutCoin = pHdr->byData;
					LOG_OUT("배출 수량 (0x%02X) : (0x%02X) !!!", byOutCoin & 0xFF, byCount & 0xFF);

					m_nOutCount += byOutCoin;

					if(pHdr->byData == byCount)
					{	// 정상 (정상 배출)
						nResult = 0;
					}
					else
					{	// 에러 (방출갯수 에러)
						LOG_OUT("배출 수량 에러 (0x%02X) : (0x%02X) !!!", byOutCoin & 0xFF, byCount & 0xFF);
						nResult = -2;
					}
					break;
				}
				else if(pHdr->byCommand == RCMD_ERR_COIN) // 0x45 (3-1)
				{	// 배출 에러
					LOG_OUT("배출 에러1, Command(0x%02X) !!!", pHdr->byCommand & 0xFF);
					byOutCoin = pHdr->byData;
					m_nOutCount += byOutCoin;
					nResult = -3;
					break;
				}
				else if(pHdr->byCommand == RCMD_STATUS) // 0x45 (3-2)
				{	// 배출 에러
					LOG_OUT("배출 에러2, Command(0x%02X) !!!", pHdr->byCommand & 0xFF);
					SetCoinStatus(pHdr->byData);
					nResult = -4;
					break;
				}
			}
		}

		if(nResult < 0)
		{
			// 미방출에 대한 사항
			if(nCount != m_nOutCount)
			{
				if(m_nType == COIN_100_TYPE)
				{
					SetCheckEventCode(EC_COIN_100_NO_OUT, TRUE);
				}
				else
				{
					SetCheckEventCode(EC_COIN_500_NO_OUT, TRUE);
				}

				SetCheckEventCode(EC_COIN_NO_OUT, TRUE);
			}

			UnLocking();
			return nResult;
		}

		if(m_nType == COIN_100_TYPE)
		{
			SetCheckEventCode(EC_COIN_100_NO_OUT, FALSE);
		}
		else
		{
			SetCheckEventCode(EC_COIN_500_NO_OUT, FALSE);
		}

		LOG_OUT("%s", "SetCheckEventCode(EC_COIN_NO_OUT, FALSE) !!!!");
	}
	//SetCheckEventCode(EC_COIN_NO_OUT, FALSE);
	UnLocking();

	return nResult;
}
***/

/**
 * @brief		Reset
 * @details		호퍼 RESET 명령 처리 ('r' : 0x72)
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCoinWS::Reset(void)
{
	int nRet;
	DWORD dwTick;
	PCOIN_WS_HEADER_T  pHdr;

	LOG_OUT("RESET() start ..");
	Locking();

	nRet = SndRcvPacket(SCMD_RESET, (BYTE *)NULL, 0, m_szRxBuf);
	if(nRet < 0)
	{
		UnLocking();
		return nRet;
	}

	pHdr = (PCOIN_WS_HEADER_T) m_szRxBuf;

	if(pHdr->byCommand != RCMD_RESPONSE)
	{
		LOG_OUT("RESET() 에러, Command(0x%02X) !!!", pHdr->byCommand & 0xFF);
		UnLocking();
		return -3;
	}

	dwTick = ::GetTickCount();
	while( Util_CheckExpire(dwTick) < (1000 * 3) )	
	{
		nRet = GetPacket(m_szRxBuf);
		if(nRet > 0)
		{
			;
		}
	}

	pHdr = (PCOIN_WS_HEADER_T) m_szRxBuf;

	//SetCoinStatus(pHdr->byData);
	UnLocking();

	LOG_OUT("RESET() end..\n");

	return nRet;
}

/**
 * @brief		GetStatus
 * @details		호퍼 상태 명령 처리	('s' : 0x73)
 * @param		int nInitFlag	
 * @return		성공 : > 0, 실패 : < 0
 */
int CCoinWS::GetStatus(void)
{
	int nRet;
	PCOIN_WS_HEADER_T  pHdr;

	//LOG_OUT("GetStatus() start ..");

	Locking();

	m_bLog = FALSE;

	nRet = SndRcvPacket(SCMD_STATUS, (BYTE *)NULL, 0, m_szRxBuf);
	if(nRet < 0)
	{
		m_bLog = TRUE;
		UnLocking();
		return nRet;
	}

	pHdr = (PCOIN_WS_HEADER_T) m_szRxBuf;
	if(pHdr->byCommand != RCMD_STATUS)
	{
		LOG_OUT("GetStatus() 에러, Command(0x%02X) !!!", pHdr->byCommand & 0xFF);
		m_bLog = TRUE;
		UnLocking();
		return -3;
	}

	SetCoinStatus(pHdr->byData);

	m_bLog = TRUE;
	UnLocking();
	//LOG_OUT("GetStatus() end ..");
	return nRet;
}

/**
 * @brief		GetOutInfo
 * @details		코인 배출수량 요청 명령 처리 ('t' : 0x74)
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCoinWS::GetOutInfo(void)
{
	return m_nOutCount;
}

/**
 * @brief		TotalEnd
 * @details		데이타 Clear
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCoinWS::TotalEnd(void)
{
	m_nOutCount = 0;
	return 0;
}


/**
 * @brief		StartProcess
 * @details		Start
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCoinWS::StartProcess(int nCommIdx, int nFlag)
{
	int		nRet;
	DWORD	dwThreadID;

	if(nFlag == 100)
	{
		m_nType = COIN_100_TYPE;
	}
	else
	{
		m_nType = COIN_500_TYPE;
	}

	LOG_INIT(nFlag);

	LOG_OUT("start !!!");

	EndProcess();

	nRet = m_clsComm.Open(nCommIdx, CBR_9600, 8, NOPARITY, ONESTOPBIT);
	if(nRet < 0) 
	{
		LOG_OUT("s_clsCdsp.Open() failure, ret(%d)..", nRet);
		return -1;
	}

	m_hAccMutex = ::CreateMutex(NULL, FALSE, NULL);
	if(m_hAccMutex == NULL) 
	{
		LOG_OUT("CreateMutex() failure..");
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
int CCoinWS::EndProcess(void)
{
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
 * @details		thread run
 * @param		LPVOID lParam		CCardRD_KICC class instance
 * @return		항상 = 0
 */
DWORD CCoinWS::RunThread(LPVOID lParam)
{
	DWORD	dwTick;
	CCoinWS *pClass = (CCoinWS *)lParam;

	pClass->m_bRun = TRUE;
	dwTick = ::GetTickCount();
	while(pClass->m_bRun)
	{
		Sleep(10);

		if( Util_CheckExpire(dwTick) >= 100 )	
		{
			dwTick = ::GetTickCount();
		}		
	}

	return 0;
}
