// 
// 
// BillICT.cpp : 지폐인식기 (ICT004)
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "data_main.h"
#include "File_Env_ini.h"
#include "event_if.h"
#include "dev_bill_ict.h"

//----------------------------------------------------------------------------------------------------------------------

#define LOG_OUT(fmt, ...)		{ CBillICT::m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ CBillICT::m_clsLog.HexaDump(x, y, z); }

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CBillICT
 * @details		생성자
 */
CBillICT::CBillICT()
{
	m_hThread = NULL;
	m_hAccMutex = NULL;
	m_bConnected = FALSE;

	m_n1k = m_n5k = m_n10k = m_n50k = m_nTotalFare = 0;
	m_dwStatus = 0L;

	m_nDebug = TRUE;

	m_bInsert = FALSE;
}

/**
 * @brief		~CBillICT
 * @details		소멸자
 */
CBillICT::~CBillICT()
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
void CBillICT::LOG_INIT(void)
{
	//m_clsLog.SetData(30, "\\Log\\BillICT");
	m_clsLog.SetData(30, "\\Log\\Bill\\ICT");
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
int CBillICT::Locking(void)
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
int CBillICT::UnLocking(void)
{
	if(m_hAccMutex != NULL) 
	{
		::ReleaseMutex(m_hAccMutex);
	}

	return 0;	
}

/**
 * @brief		SetStatus
 * @details		Set Status Value
 * @param		int nIndex		상태 index
 * @return		None
 */
void CBillICT::SetStatus(int nIndex)
{
	m_dwStatus = m_dwStatus | (1 << nIndex);
}

/**
 * @brief		GetStatus
 * @details		Get Status Value
 * @param		int nIndex		상태 index
 * @return		상태값
 */
int CBillICT::GetStatus(int nIndex)
{
	return (m_dwStatus & (1 << nIndex)) >> nIndex;
}

/**
 * @brief		ClearStatus
 * @details		Clear Status 
 * @param		int nIndex		상태 index
 * @return		None
 */
void CBillICT::ClearStatus(int nIndex)
{
	m_dwStatus = m_dwStatus & (~(1 << nIndex));
}

/**
 * @brief		SendPacket
 * @details		1 Byte 전송 data
 * @param		BYTE byData		전송 데이타
 * @return		항상 : 0
 */
int CBillICT::SetStatusValue(BYTE byData)
{
	int i, nFind;
	BILLICT_STATUS_T stData[] = {
		{ 0x10, "Bill Accept Finish"}, 
		{ 0x20, "Motor Failure"		}, 
		{ 0x21, "CheckSum Error"	}, 
		{ 0x22, "Bill Jam"			}, 
		{ 0x23, "Bill Remove"		}, 
		{ 0x24, "Statcker Open"		}, 
		{ 0x25, "Sensor Problem"	}, 
		{ 0x27, "Bill Fish"			}, 
		{ 0x28, "Statcker Problem"	}, 
		{ 0x29, "Bill Reject"		}, 
		{ 0x2A, "Invalid Command_1" }, 
		{ 0x2B, "Invalid Command_2" }, 
		{ 0x2C, "Invalid Command_3" }, 
		{ 0x2D, "Invalid Command_4" }, 
		{ 0x2E, "Reserved"			}, 
		{ 0x2F, "When error status is exclusion" }, 
		{ 0x3E, "Bill Accept enable status"		}, 
		{ 0x3F, "Bill Accept Inhibit status"		}, 
		{ 0x80, "Power supply on"	}, 
		{ 0x8F, "Handle request"	}, 
	};

	nFind = -1;
	for(i = 0; i < sizeof(stData) / sizeof(BILLICT_STATUS_T); i++)
	{
		if(stData[i].byStatus == byData)
		{
			nFind = i;
			break;
		}
	}

	if(nFind < 0)
	{
		return -1;
	}

	if(!GetStatus(nFind))
	{
		SetStatus(nFind);
		LOG_OUT("상태값 = %02X, %s..", stData[nFind].byStatus, stData[nFind].pStr);
	}

	return 0;
}


/**
 * @brief		SendPacket
 * @details		1 Byte 전송 data
 * @param		BYTE byData		전송 데이타
 * @return		항상 : 0
 */
int CBillICT::SendPacket(BYTE byData)
{
	if( m_nDebug == TRUE )
	{
		LOG_OUT("[TX] = %02X ", byData);
	}

	//PurgeComm(m_clsComm.m_hCommHandle, PURGE_TXABORT | PURGE_RXCLEAR);
	//Sleep(10);

	return m_clsComm.SendData(&byData, 1);
}

/**
 * @brief		GetPacket
 * @details		데이타 수신 처리
 * @param		BYTE *retBuf		수신 데이타
 * @return		항상 : 0
 */
/***
int CBillICT::GetPacket(BYTE *retBuf)
{
	int ch, nCount, nState;
	int n1k, n5k, n10k, n50k;
	DWORD dwTick;

	n1k = n5k = n10k = n50k = 0;
	nCount = nState = 0;
	dwTick = ::GetTickCount();
	while( Util_CheckExpire(dwTick) < 200 )	
	{
		ch = m_clsComm.ReadData();
		if(ch < 0) 
		{
			continue;
		}

		ch = ch & 0xFF;

		if( m_nDebug == TRUE )
			LOG_OUT("[RX] = %02X ", ch);

		retBuf[nCount++] = ch;

		switch(nState)
		{
		case 0 :
			if(ch == 0x80)
			{
				SendPacket(0x02);		///> Handle response
				dwTick = ::GetTickCount();
				nCount = 0;
			}
			else if(ch == 0x81)
			{
				nState++;
			}
			else
			{
				SetStatusValue(ch);
			}
			break;
		case 1 :
			if(ch == 0x40)
			{	// 1,000원
				n1k++;
			}
			else if(ch == 0x41)
			{	// 5,000원
				n5k++;
				//m_nTotalFare = m_nTotalFare * 5000;
			}
			else if(ch == 0x42)
			{	// 10,000원
				n10k++;
				//m_nTotalFare = m_nTotalFare * 10000;
			}
			else if(ch == 0x43)
			{	// 50,000원
				//SendPacket(0x0F);		///> Reject command		
				//dwTick = ::GetTickCount();
				//nCount = 0;
				//break;
				n50k++;
				//m_nTotalFare = m_nTotalFare * 50000;
			}
			else
			{
				SendPacket(0x0F);		///> Reject command		
				dwTick = ::GetTickCount();
				nCount = 0;
				break;
			}

#if 1
			m_n1k  += n1k;
			m_n5k  += n5k;
			m_n10k += n10k;
			m_n50k += n50k;
			
			m_nTotalFare = (m_n1k * 1000) + (m_n5k * 5000) + (m_n10k * 10000) + (m_n50k * 50000);
			LOG_OUT("m_n1k[%02d], m_n5k[%02d], m_n10k[%02d], m_n50k[%02d], total[%10d]\n", m_n1k, m_n5k, m_n10k, m_n50k, m_nTotalFare);
#else
 			if(m_bInsert == TRUE)
 			{
 				m_n1k  += n1k;
 				m_n5k  += n5k;
 				m_n10k += n10k;
 				m_n50k += n50k;
 
 				m_nTotalFare = (m_n1k * 1000) + (m_n5k * 5000) + (m_n10k * 10000) + (m_n50k * 50000);
 				LOG_OUT("m_n1k[%02d], m_n5k[%02d], m_n10k[%02d], m_n50k[%02d], total[%10d]\n", m_n1k, m_n5k, m_n10k, m_n50k, m_nTotalFare);
 			}
 			else
 			{
 				LOG_OUT("n1k[%02d], n5k[%02d], n10k[%02d], n50k[%02d]..", n1k, n5k, n10k, n50k);
 			}
#endif
			SendPacket(0x02);		///> Accept command
			dwTick = ::GetTickCount();
			nCount = 0;

			nState = 0;
			break;
		}
	}

	return nCount;
}
***/

int CBillICT::GetPacket(BYTE *retBuf)
{
	int ch, nCount, nState;
	int n1k, n5k, n10k, n50k;
	DWORD dwTick;

	n1k = n5k = n10k = n50k = 0;
	nCount = nState = 0;
	dwTick = ::GetTickCount();
//	while( Util_CheckExpire(dwTick) < 200 )	
//	while( Util_CheckExpire(dwTick) < 1000 )	
	while( Util_CheckExpire(dwTick) < 2000 )	
	{
		ch = m_clsComm.ReadData();
		if(ch < 0) 
		{
			continue;
		}

		ch = ch & 0xFF;

		if( m_nDebug == TRUE )
			LOG_OUT("[RX] = %02X ", ch);

		retBuf[nCount++] = ch;

		dwTick = ::GetTickCount();

		switch(nState)
		{
		case 0 :
			if( (ch == 0x80) || (ch == 0x8F) )
			{	/// power supply on
				SendPacket(0x02);		///> Handle response
				goto got_packet;
			}
			else if(ch == 0x81)
			{
				nState++;
			}
			else
			{
				SetStatusValue(ch);
			}
			break;
		case 1 :
			if(ch == 0x40)
			{	// 1,000원
				if( INI_UseCashIn1K() <= 0 )
				{
					SendPacket(0x0F);		///> Reject command		
					dwTick = ::GetTickCount();
					nCount = 0;
					break;
				}
				else
				{
					n1k++;
					LOG_OUT("[1,000원] inserting.. ");
				}
			}
			else if(ch == 0x41)
			{	// 5,000원
				if( INI_UseCashIn5K() <= 0 )
				{
					SendPacket(0x0F);		///> Reject command		
					dwTick = ::GetTickCount();
					nCount = 0;
					break;
				}
				else
				{
					n5k++;
					//m_nTotalFare = m_nTotalFare * 5000;
					LOG_OUT("[5,000원] inserting.. ");
				}
			}
			else if(ch == 0x42)
			{	// 10,000원
				if( INI_UseCashIn10K() <= 0 )
				{
					SendPacket(0x0F);		///> Reject command		
					dwTick = ::GetTickCount();
					nCount = 0;
					break;
				}
				else
				{
					n10k++;
					//m_nTotalFare = m_nTotalFare * 10000;
					LOG_OUT("[10,000원] inserting.. ");
				}
			}
			else if(ch == 0x43)
			{	// 50,000원
				if( INI_UseCashIn50K() <= 0 )
				{
					SendPacket(0x0F);		///> Reject command		
					dwTick = ::GetTickCount();
					nCount = 0;
					break;
				}
				else
				{
					n50k++;
					//m_nTotalFare = m_nTotalFare * 50000;
					LOG_OUT("[50,000원] inserting.. ");
				}
			}
			else
			{
				SendPacket(0x0F);		///> Reject command		
				dwTick = ::GetTickCount();
				nCount = 0;
				break;
			}
			SendPacket(0x02);		///> Accept command
			nState++;
			break;
		case 2 :
//			if(ch == 0x3E)

#if 0
			if((ch == 0x10) || (ch == 0x3E) || (ch == 0x5E))
			{
				m_n1k  += n1k;
				m_n5k  += n5k;
				m_n10k += n10k;
				m_n50k += n50k;

				m_nTotalFare = (m_n1k * 1000) + (m_n5k * 5000) + (m_n10k * 10000) + (m_n50k * 50000);
				LOG_OUT("m_n1k[%02d], m_n5k[%02d], m_n10k[%02d], m_n50k[%02d], total[%10d]\n", m_n1k, m_n5k, m_n10k, m_n50k, m_nTotalFare);
			}
			//nState = 0;
			//break;
			goto got_packet;
#else

			if( ch == 0x10 )
			{
				m_n1k  += n1k;
				m_n5k  += n5k;
				m_n10k += n10k;
				m_n50k += n50k;

				m_nTotalFare = (m_n1k * 1000) + (m_n5k * 5000) + (m_n10k * 10000) + (m_n50k * 50000);
				LOG_OUT("진짜 투입됨 : m_n1k[%02d], m_n5k[%02d], m_n10k[%02d], m_n50k[%02d], total[%10d]\n", m_n1k, m_n5k, m_n10k, m_n50k, m_nTotalFare);
				goto got_packet;
			}
			break;
#endif
		}
	}

got_packet:

	if(nCount == 0)
	{
		LOG_OUT("Time Out");
		return -1;
	}
	LOG_HEXA("= GetPacket =", retBuf, nCount);
	return nCount;
}

/**
 * @brief		Reset
 * @details		초기화 명령 처리
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillICT::Reset(void)
{
	int		nRet = 0;

	Locking();

	LOG_OUT("start..");

	nRet = SendPacket(0x30);
	nRet = GetPacket(m_szRxBuf);
	if(nRet < 0)
	{
		LOG_OUT("GetPacket() Failure");
		UnLocking();
		return nRet;
	}

	if(GetStatus(BILL_HANDLE_REQ))
	{
		nRet = 1;
	}
	else
	{
		nRet = -1;
	}

	UnLocking();

	return nRet;
}

/**
 * @brief		GetStatus
 * @details		상태 명령 처리
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillICT::GetStatus(void)
{
	int		nRet = 0;

// 	Locking();

	LOG_OUT("start..");

	nRet = SendPacket(0x0C);
	nRet = GetPacket(m_szRxBuf);
	if(nRet < 0)
	{
// 		UnLocking();
		return nRet;
	}

// 	UnLocking();

	return nRet;
}

/**
 * @brief		GetDevStatus
 * @details		상태 명령 처리
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillICT::GetDevStatus(void)
{
	int		nRet = 0;
	//DWORD dwTick;
	Locking();

	LOG_OUT("start..");
	nRet = SendPacket(0x0C);
	nRet = GetPacket(m_szRxBuf);
	if(nRet < 0)
	{
		UnLocking();
		return nRet;
	}

	UnLocking();

	return nRet;
}

/**
 * @brief		Enable
 * @details		지폐투입 허가 
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillICT::Enable(void)
{
	int		nRet = 0, nRetry = 0;

	nRetry = 0;

	Locking();
	LOG_OUT("start..");
	nRet = SendPacket(0x3E);
	Sleep(200);
	UnLocking();

	return -1;
}

/**
 * @brief		Inhibit
 * @details		지폐투입 금지 
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillICT::Inhibit(void)
{
	int		nRet, nRetry;

	nRetry = nRet = 0;

	Locking();
	LOG_OUT("start..");
	nRet = SendPacket(0x5E);
	Sleep(200);
	UnLocking();

	return -1;
}

/**
 * @brief		GetMoney
 * @details		지폐투입 금액 가져오기 
 * @param		int *n1k		1,000원 데이타 갯수
 * @param		int *n5k		5,000원 데이타 갯수
 * @param		int *n10k		10,000원 데이타 갯수
 * @param		int *n50k		50,000원 데이타 갯수
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillICT::GetMoney(int *n1k, int *n5k, int *n10k, int *n50k)
{
	static BILL_DT_T prevBill;

	*n1k = m_n1k;
	*n5k = m_n5k;
	*n10k = m_n10k;
	*n50k = m_n50k;

	if( (prevBill.w1k != (WORD)m_n1k) || (prevBill.w5k != (WORD)m_n5k) || (prevBill.w10k != (WORD)m_n10k) || (prevBill.w50k != (WORD)m_n50k) )
	{
		LOG_OUT("1k[%02d], 5k[%02d], 10k[%02d], 50k[%02d], total[%10d]\n", m_n1k, m_n5k, m_n10k, m_n50k, m_nTotalFare);

		prevBill.w1k  = (WORD)m_n1k;
		prevBill.w5k  = (WORD)m_n5k;
		prevBill.w10k = (WORD)m_n10k;
		prevBill.w50k = (WORD)m_n50k;
	}

	return m_nTotalFare;
}

/**
 * @brief		TotalEnd
 * @details		거래 종료 
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillICT::TotalEnd(void)
{
	Locking();

	LOG_OUT("start..");

	m_n1k = m_n5k = m_n10k = m_n50k = m_nTotalFare = 0;

	UnLocking();

	return 0;
}

/**
 * @brief		StartProcess
 * @details		Start
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillICT::StartProcess(int nCommIdx)
{
	int		nRet  = 0;
	DWORD dwThreadID;

	LOG_INIT();

	m_bConnected = FALSE;

	LOG_OUT("start !!!");

	EndProcess();

	nRet = m_clsComm.Open(nCommIdx, CBR_9600, 8, EVENPARITY, ONESTOPBIT);
	if(nRet < 0) 
	{
		LOG_OUT("m_clsComm.Open() Failure, nRet(%d) ..", nRet);
		EndProcess();
		return -1;
	}

	m_hAccMutex = ::CreateMutex(NULL, FALSE, NULL);
	if(m_hAccMutex == NULL) 
	{
		LOG_OUT("m_clsComm.Open() Failure, nRet(%d) ..", nRet);
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
int CBillICT::EndProcess(void)
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
 * @brief		CmdQueueInfo
 * @details		큐 데이타 처리
 * @param		BILL_QUE_DATA_T tQue	Queue 데이타 구조체
 * @return		항상 : 0
 */
int CBillICT::CmdQueueInfo(BILL_QUE_DATA_T tQue)
{
	int nRet = 0;

	switch(tQue.wCommand)
	{
	case BILL_CMD_RESET:
		LOG_OUT("BILL_CMD_RESET ");
		nRet = SendPacket(0x30);
		nRet = GetPacket(m_szRxBuf);
		if(nRet < 0)
		{
			return nRet;
		}
		break;
	case BILL_CMD_ENABLE:
		LOG_OUT("BILL_CMD_ENABLE ");
		nRet = SendPacket(0x3E);
		break;
	case BILL_CMD_DISABLE:
		LOG_OUT("BILL_CMD_DISABLE ");
		nRet = SendPacket(0x5E);
		break;
	case BILL_CMD_GETSTATUS:
		break;
	}

	return 0;
}

/**
 * @brief		RunThread
 * @details		Start
 * @param		LPVOID lParam		CBillICT instance
 * @return		항상 : 0
 */
DWORD CBillICT::RunThread(LPVOID lParam)
{
	int					nRet = 0, nSize = 0;
	static int nRetry = 0;
	DWORD				dwTick;
	BILL_QUE_DATA_T		tBillData;
	CBillICT *pBill = (CBillICT *)lParam;

	dwTick = ::GetTickCount();
	while(pBill->m_bConnected)
	{
		Sleep(10);

		// 		if( Util_CheckExpire(dwTick) >= 500 )	
		// 		{
		// 			pBill->Bill_GetStatus();
		// 			dwTick = ::GetTickCount();
		// 		}

		nSize = pBill->s_QueData.size();
		if(nSize > 0)
		{
			tBillData = pBill->s_QueData.front();
			pBill->s_QueData.pop();

			pBill->CmdQueueInfo(tBillData);
			dwTick = ::GetTickCount();
		}
		else
		{
			if(Util_CheckExpire(dwTick) > 500)
			{
				// polling
				nRet = pBill->GetDevStatus();
				if(nRet < 0)
				{
					if(++nRetry >= 3)
					{
						SetCheckEventCode(EC_BILL_COMM_ERR, TRUE);
					}
				}
				else
				{
					if(nRetry > 0)
					{
						nRetry = 0;
					}
					SetCheckEventCode(EC_BILL_COMM_ERR, FALSE);
				}
				dwTick = ::GetTickCount();
			}
		}
	}

	return 0;
}

/**
 * @brief		AddQue
 * @details		큐에 데이타 넣는다
 * @param		int nCmd			Bill Command
 * @return		항상 : 0
 */
int CBillICT::AddQue(int nCmd)
{
	BILL_QUE_DATA_T tQue;

	ZeroMemory(&tQue, sizeof(BILL_QUE_DATA_T));

	tQue.wCommand = (WORD) nCmd;

	s_QueData.push(tQue);

	return 0;
}



