// 
// 
// dev_bill_mei.cpp : 지폐인식기 (MEI : scnl-8327r)
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "dev_bill_mei.h"
#include "event_if.h"

//----------------------------------------------------------------------------------------------------------------------

//#define __MEI_ALL_DEBUG__		1

#define LOG_OUT(fmt, ...)		{ CBillMEI::m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ CBillMEI::m_clsLog.HexaDump(x, y, z); }

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CBillMEI
 * @details		생성자
 */
CBillMEI::CBillMEI()
{
	m_hThread = NULL;
	m_hAccMutex = NULL;
	m_bConnected = FALSE;

	m_n1k = m_n5k = m_n10k = m_n50k = m_nTotalFare = 0;
	m_dwStatus = 0L;

	m_byPrevAction = m_byAction = 0;

	m_nDebug = FALSE;
}

/**
 * @brief		~CBillMEI
 * @details		소멸자
 */
CBillMEI::~CBillMEI()
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
void CBillMEI::LOG_INIT(void)
{
	m_clsLog.SetData(30, "\\Log\\Bill\\MEI");
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
int CBillMEI::Locking(void)
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
int CBillMEI::UnLocking(void)
{
	if(m_hAccMutex != NULL) 
	{
		::ReleaseMutex(m_hAccMutex);
	}

	return 0;	
}

/**
 * @brief		MakeBCC
 * @details		BCC 값 계산
 * @param		char *pData		데이타
 * @param		int nDataLen	데이타 길이
 * @return		None
 */
int CBillMEI::MakeBCC(BYTE *pData, int nDataLen)
{
	BYTE bcc = 0;
	int i;

	for(i = 0; i < nDataLen; i++)
	{
		bcc ^= pData[i]; 
	}

	return (int)bcc;
}


/**
 * @brief		SendPacket
 * @details		데이타 전송
 * @param		BYTE byData		전송 데이타
 * @param		int nDataLen	전송 데이타 길이
 * @return		전송 데이타 길이
 */
int CBillMEI::SendPacket(BYTE *pData, int nDataLen)
{
	int nIndex = 0;
	static char chACK = 0x11;
	static BYTE PrevData[10];

	/// (01). STX
	m_szTxBuf[nIndex++] = CHAR_STX;
	/// (02). Length
	m_szTxBuf[nIndex++] = 0x08;
	/// (03). ACK/NAK
	if(chACK != 0x10)
	{
		chACK = 0x10;
	}
	else
	{
		chACK = 0x11;
	}
	m_szTxBuf[nIndex++] = chACK;
	/// (04). DATA
	nIndex += Util_MyCopyMemory(&m_szTxBuf[nIndex], pData, nDataLen);
	/// (05). ETX
	m_szTxBuf[nIndex++] = CHAR_ETX;
	/// (06). BCC
	m_szTxBuf[nIndex++] = MakeBCC(&m_szTxBuf[1], 5);

#ifdef __MEI_ALL_DEBUG__
	LOG_HEXA("SendPacket", m_szTxBuf, nIndex);
#else
 	if( memcmp(PrevData, pData, nDataLen) )
 	{
 		LOG_HEXA("SendPacket", m_szTxBuf, nIndex);
 		memcpy(PrevData, pData, nDataLen);
 	}
 	else
 	{
 		;
 	}
#endif


	return m_clsComm.SendData(m_szTxBuf, nIndex);
}

/**
 * @brief		GetPacket
 * @details		데이타 수신 처리
 * @param		BYTE *retBuf		수신 데이타
 * @return		성공 : > 0, 실패 < 0
 */
int CBillMEI::GetPacket(BYTE *retBuf)
{
	int ch, nLen, nRecvCount;
	DWORD dwTick;
	BYTE byCalcBCC;
	static int nCount = 0;
	static int nState = 0;
	static BYTE rPrevData[20];

	dwTick = ::GetTickCount();
	while( Util_CheckExpire(dwTick) < 500 )	
	{
		ch = m_clsComm.ReadData();
		if(ch < 0) 
		{
			continue;
		}

		ch = ch & 0xFF;

		if( m_nDebug == TRUE )
		{
			LOG_OUT("[RX] = %02X ", ch);
		}

		switch(nState)
		{
		case 0 :	///< STX
			{
				if(ch == CHAR_STX)
				{
					nCount = 0;
					retBuf[nCount++] = ch & 0xFF;
					nState++;
				}
			}
			break;

		case 1 :	///< Length
			{
				retBuf[nCount++] = ch & 0xFF;
				nState++;
			}
			break;

		case 2 :	///< ACK/NAK
			{
				retBuf[nCount++] = ch & 0xFF;
				nState++;
			}
			break;

		case 3 :	///< DATA
			{
				retBuf[nCount++] = ch & 0xFF;
				nLen = retBuf[1] - 2;
				if(nLen == nCount)
				{
					nState++;
				}
			}
			break;

		case 4 :	///< ETX
			{
				if(ch == CHAR_ETX)
				{
					retBuf[nCount++] = ch & 0xFF;
					nState++;
				}
				else
				{
					nCount = nState = 0;
				}
			}
			break;

		case 5 :	///< BCC
			retBuf[nCount++] = ch & 0xFF;
			goto got_packet;
		}
	}

	LOG_OUT("Recev Timeout !!!");

	return -99;


got_packet:

	nRecvCount = nCount;
	nCount = nState = 0;

	byCalcBCC = MakeBCC(&retBuf[1], nRecvCount - 3);	/// stx, etx, bcc
	if( byCalcBCC ^ retBuf[nRecvCount - 1] )
	{
		LOG_OUT("BCC Error, (%02X, %02X)", byCalcBCC, retBuf[nRecvCount - 1]);
		return -1;
	}

#ifdef __MEI_ALL_DEBUG__
	LOG_HEXA("GetPacket", retBuf, nRecvCount);
#else
 	if( memcmp(rPrevData, &retBuf[3], nRecvCount - 5) )
 	{
 		memcpy(rPrevData, &retBuf[3], nRecvCount - 5);
 
 		LOG_HEXA("GetPacket", retBuf, nRecvCount);
 	}
#endif

	return nRecvCount;
}

/**
 * @brief		SndRecvPacket
 * @details		송.수신 패킷 처리
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
//int CBillMEI::SndRecvPacket(BYTE *pSendData, int nSendLen, BYTE *retBuf)
int CBillMEI::SndRecvPacket(void)
{
	int		nRet;
	int		n1k, n5k, n10k, n50k;
	DWORD dwTick;

	n1k = n5k = n10k = n50k = 0;

	nRet = SendPacket((BYTE *)&m_tSend, sizeof(m_tSend));
	dwTick = GetTickCount();
	while( Util_CheckExpire(dwTick) < 500 )	
	{
		nRet = GetPacket(m_szRxBuf);
		if(nRet > 0)
		{
			PBILL_MEI_RECV_DATA pPacket;
			char *pStr = NULL;

			pPacket = (PBILL_MEI_RECV_DATA) &m_szRxBuf[3];

			CheckError(pPacket->Data1);

			m_byAction = pPacket->Data0;

			if(m_byAction != m_byPrevAction)
			{
				m_byPrevAction = m_byAction;
			}

			switch(pPacket->Data0)
			{
			case MEI_R0_IDLING:		// 0x01
				{
					pStr = "[IDLING]";
				}
				break;
			case MEI_R0_ACCEPTING:	// 0x02
				{
					pStr = "[ACCEPTING]";
				}
				break;
			case MEI_R0_ESCROWED:	// 0x04
				{
 					switch(pPacket->Data2)
 					{
 					case MEI_1K:	// 0x08
 						n1k = 1;
 						pStr = "[ESCROWED] [1K]";
 						break;
 					case MEI_5K:	// 0x10
 						n5k = 1;
 						pStr = "[ESCROWED] [5K]";
 						break;
 					case MEI_10K:	// 0x18
 						n10k = 1;
 						pStr = "[ESCROWED] [10K]";
 						break;
 					case MEI_50K:	// 0x20
 						n50k = 1;
 						pStr = "[ESCROWED] [50K]";
 						break;
 					}

					if( (n1k + n5k + n10k + n50k) > 0 && (m_tSend.Data1 == 0x1C) )
					{
						m_tSend.Data1 |= 0x20;
					}
					else
					{
						m_tSend.Data1 = 0x1C;
					}
				}
				break;
			case MEI_R0_STACKING:	// 0x08
				{
					pStr = "[STACKING]";
				}
				break;
			case MEI_R0_STACKED:	// 0x10
				{
					pStr = "[STACKED]";
				}
				break;
			case MEI_R0_RETURNING:	// 0x20
				{
					pStr = "[RETURNING]";
				}
				break;
			case MEI_R0_RETURNED:	// 0x40
				{
					pStr = "[RETURNED]";
				}
				break;
			case MEI_R0_COMPLETED:	// 0x11
				{
					switch(pPacket->Data2)
					{
					case MEI_1K:	// 0x08
						n1k = 1;
						break;
					case MEI_5K:	// 0x10
						n5k = 1;
						break;
					case MEI_10K:	// 0x18
						n10k = 1;
						break;
					case MEI_50K:	// 0x20
						n50k = 1;
						break;
					}
					
					m_n1k += n1k;
					m_n5k += n5k;
					m_n10k += n10k;
					m_n50k += n50k;

					m_nTotalFare = BILL_SUM(m_n1k, m_n5k, m_n10k, m_n50k);

					LOG_OUT("Inserted [%02d, %02d, %02d, %02d] = %d ", m_n1k, m_n5k, m_n10k, m_n50k, m_nTotalFare);

					pStr = "[COMPLETED]";
				}
				break;
			default:
				{
					pStr = "[ETC ...]";
				}
				break;
			}

			if(pStr != NULL)
			{
				;//LOG_OUT("수신 = %s ", pStr);
			}
			else
			{
				;//LOG_OUT("수신 = NULL ");
			}
			
			if(pPacket != NULL)
			{
				if( memcmp(&m_tRecvData, pPacket, sizeof(BILL_MEI_RECV_DATA)) )
				{
					memcpy(&m_tRecvData, pPacket, sizeof(BILL_MEI_RECV_DATA));
				}
			}
			else
			{
				LOG_OUT("pPacket = NULL");
			}
			return nRet;
		}
	}

	LOG_OUT("Error !!!");
	
	return -1;
}

/**
 * @brief		GetActionStatus
 * @details		지폐처리장치 동작 체크
 * @param		None
 * @return		동작값
 */
int CBillMEI::GetActionStatus(void)
{
	return m_byAction;
}

/**
 * @brief		CheckError
 * @details		지폐처리장치 에러 체크
 * @param		BYTE byError		장비상태값
 * @return		항상 = 0
 */
int CBillMEI::CheckError(BYTE byError)
{
	if( (byError & 0x04) == 0x04 )
	{	/// 지폐걸림
		SetCheckEventCode(EC_BILL_JAM, TRUE);
		LOG_OUT("에러 - 지폐걸림");
	}
	else
	{
		SetCheckEventCode(EC_BILL_JAM, FALSE);
	}

	if( (byError & 0x08) == 0x08 )
	{	/// 카세트 공간 부족
		SetCheckEventCode(EC_BILL_STK_FULL, TRUE);
		LOG_OUT("에러 - 카세트 공간 부족");
	}
	else
	{
		SetCheckEventCode(EC_BILL_STK_FULL, FALSE);
	}
	
	if( (byError & 0x10) != 0x10 )
	{	/// 카세트 분리
		SetCheckEventCode(EC_BILL_STK_OUT, TRUE);
		LOG_OUT("에러 - 카세트 분리");
	}
	else
	{
		SetCheckEventCode(EC_BILL_STK_OUT, FALSE);
	}

	return 0;
}


/**
 * @brief		MakeCommand
 * @details		초기화 명령 처리
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillMEI::MakeCommand(int nSndCommand)
{
	int		nRet = 0;

	if( m_nSendCmd == nSndCommand )
	{
		LOG_OUT("Command Equal.. skip!!!");
		return 0;
	}

	Locking();

	//LOG_OUT("start..");

	switch(nSndCommand)
	{
	case MEI_SCMD_INIT :
		LOG_OUT("MEI_SCMD_INIT ..");
		m_tSend.Data0 = 0x1F;
		m_tSend.Data1 = 0x2E;
		m_tSend.Data2 = 0x00;
		break;
	case MEI_SCMD_IDLE :
	case MEI_SCMD_DISABLE :
		LOG_OUT("MEI_SCMD_DISABLE ..");
		m_tSend.Data0 = 0x00;
		m_tSend.Data1 = 0x00;
		m_tSend.Data2 = 0x00;
		break;
	case MEI_SCMD_ENABLE :
		LOG_OUT("MEI_SCMD_ENABLE ..");
		m_tSend.Data0 = 0x1F;
		m_tSend.Data1 = 0x1C;
		m_tSend.Data2 = 0x01;
		break;
	case MEI_SCMD_RESET :
// 		LOG_OUT("MEI_SCMD_RESET ..");
// 		m_tSend.Data0 = 0x7F;
// 		m_tSend.Data1 = 0x7F;
// 		m_tSend.Data2 = 0x7F;
		break;
	default:
		nRet = -1;
		break;
	}

	//LOG_OUT("end..");
	UnLocking();

	return nRet;
}

/**
 * @brief		Reset
 * @details		Reset
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillMEI::Reset(void)
{
	MakeCommand(MEI_SCMD_RESET);
	return 0;
}

/**
 * @brief		GetStatus
 * @details		GetStatus
 * @param		None
 * @return		항상 = 0
 */
int CBillMEI::GetStatus(void)
{
	//MakeCommand();
	return 0;
}

/**
 * @brief		Enable
 * @details		Enable
 * @param		None
 * @return		항상 = 0
 */
int CBillMEI::Enable(void)
{
	MakeCommand(MEI_SCMD_ENABLE);
	return 0;
}
/**
 * @brief		Inhibit
 * @details		Inhibit
 * @param		None
 * @return		항상 = 0
 */
int CBillMEI::Inhibit(void)
{
	MakeCommand(MEI_SCMD_DISABLE);
	return 0;
}

/**
 * @brief		GetMoney
 * @details		거래 종료 
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillMEI::GetMoney(int *n1k, int *n5k, int *n10k, int *n50k)
{
	int sum;

	*n1k = m_n1k;
	*n5k = m_n5k;
	*n10k = m_n10k;
	*n50k = m_n50k;

	sum = BILL_SUM(m_n1k, m_n5k, m_n10k, m_n50k);

	if( (m_nTmp_1k != m_n1k) || (m_nTmp_5k != m_n5k) || (m_nTmp_10k != m_n10k) || (m_nTmp_50k != m_n50k) )
	{
		m_nTmp_1k  = m_n1k;
		m_nTmp_5k  = m_n5k;
		m_nTmp_10k = m_n10k;
		m_nTmp_50k = m_n50k;
		
		LOG_OUT("GetMoney [%02d, %02d, %02d, %02d] = %d ", m_n1k, m_n5k, m_n10k, m_n50k, sum);
	}

	return sum;
}

/**
 * @brief		TotalEnd
 * @details		거래 종료 
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillMEI::TotalEnd(void)
{
	Locking();

	LOG_OUT("start..");

	m_n1k = m_n5k = m_n10k = m_n50k = m_nTotalFare = 0;
	m_nTmp_1k = m_nTmp_5k = m_nTmp_10k = m_nTmp_50k = 0;

	UnLocking();

	return 0;
}

/**
 * @brief		StartProcess
 * @details		Start
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillMEI::StartProcess(int nCommIdx)
{
	int		nRet;
	DWORD dwThreadID;

	LOG_INIT();

	m_bConnected = FALSE;

	LOG_OUT("start !!!");

	EndProcess();

	nRet = m_clsComm.Open(nCommIdx, CBR_9600, 7, EVENPARITY, ONESTOPBIT);
	if(nRet < 0) 
	{
		LOG_OUT("통신포트.Open() Failure, nRet(%d) ..", nRet);
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

	m_bConnected = TRUE;

	m_hThread = ::CreateThread(NULL, 0, RunThread, this, CREATE_SUSPENDED, &dwThreadID);
	if(NULL == m_hThread)
	{
		LOG_OUT("CreateThread() failure !![%d]", ::GetLastError());
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
int CBillMEI::EndProcess(void)
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
 * @param		LPVOID lParam		CBillMEI instance
 * @return		항상 : 0
 */
DWORD CBillMEI::RunThread(LPVOID lParam)
{
	int nStep = 0;
	int nRet = 0;
	int nCount = 0;
	DWORD dwTick;
	CBillMEI *pThis = (CBillMEI *)lParam;

	dwTick = ::GetTickCount();
	while(pThis->m_bConnected)
	{
		if( (GetTickCount() - dwTick) < 300 )
		{
			Sleep(1);
			continue;
		}

		dwTick = GetTickCount();

		switch(nStep)
		{
		case 0 :
		case 1 :
		case 2 :
			pThis->MakeCommand(MEI_SCMD_INIT);
			pThis->m_nSendCmd = MEI_SCMD_INIT;
			nStep++;
			break;
		case 3:
			pThis->MakeCommand(MEI_SCMD_IDLE);
			pThis->m_nSendCmd = MEI_SCMD_IDLE;
			nStep++;
			break;
		default:
			break;
		}

		nRet = pThis->SndRecvPacket();
		if(nRet < 0)
		{
			if(++nCount > 3)			
			{
				nCount = 4;
				pThis->m_clsLog.LogOut("[%s:%d] 에러 - 통신 Timeout !!! ", __FUNCTION__, __LINE__ );
				SetCheckEventCode(EC_BILL_COMM_ERR, TRUE);
			}
		}
		else
		{
			SetCheckEventCode(EC_BILL_COMM_ERR, FALSE);
			if(nCount > 0)
			{
				nCount = 0;
			}
		}

		Sleep(200);
	}

	return 0;
}

