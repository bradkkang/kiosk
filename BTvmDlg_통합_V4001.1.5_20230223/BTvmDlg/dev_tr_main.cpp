// 
// 
// dev_tr_main.cpp : transaction
//

#include "stdafx.h"
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <queue>

#include <usrinc/fbuf.h>
#include "xzzbus_fdl.h"

#include "MyDefine.h"
#include "MyUtil.h"
#include "File_Env_ini.h"
#include "oper_config.h"
#include "event_if.h"
#include "dev_bill_main.h"
#include "dev_cardreader_main.h"
#include "dev_coin_main.h"
#include "dev_dispenser_main.h"
#include "dev_prt_main.h"
#include "dev_prt_ticket_main.h"
#include "dev_scanner_main.h"
#include "svr_main.h"
#include "dev_tr_main.h"
#include "dev_tr_cc_cardcash.h"
#include "dev_tr_cc_card.h"

//----------------------------------------------------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------------------------------------------------

static HANDLE		hThread = NULL;
static HANDLE		hAccMutex = NULL;
static DWORD		dwThreadID;

static BOOL			s_bRun;

static int			s_nStep, s_nPrevStep = -1;
static int			s_nState;  

//static TR_INFO_T	s_TrInfo;

static PDEV_CFG_T	pEnv = NULL;
static queue <TR_QUE_DATA_T> s_QueData;

CTrLogFile	clsTrLog;

//----------------------------------------------------------------------------------------------------------------------

static DWORD WINAPI TransactThread(void *);

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		LOG_INIT
 * @details		LOG 초기화
 * @param		None
 * @return		항상 = 0
 */
static int LOG_INIT(void)
{
	clsTrLog.SetData(30, "\\Log\\Tr");
	clsTrLog.Initialize();
	clsTrLog.Delete();

	return 0;
}

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
static int Locking(void)
{
	if(hAccMutex != NULL) 
	{
		::WaitForSingleObject(hAccMutex, INFINITE);
	}

	return 0;	
}

/**
 * @brief		UnLocking
 * @details		IPC UnLock
 * @param		None
 * @return		항상 : 0
 */
static int UnLocking(void)
{
	if(hAccMutex != NULL) 
	{
		::ReleaseMutex(hAccMutex);
	}

	return 0;	
}

/**
 * @brief		Transact_SetStep
 * @details		Step 값 설정
 * @param		int nValue			설정값
 * @return		항상 = 0
 */
int Transact_SetStep(int nValue)
{
#if 0
	Locking();
	{
		TR_LOG_OUT("2> Curr => s_nState(%d), s_nStep(%d), s_nPrevStep(%d), change => nStep(%d) !!", s_nState, s_nStep, s_nPrevStep, nValue);
		s_nStep = nValue;
	}
	UnLocking();
#endif

	TR_AddQueData(TR_CMD_STEP, s_nState, nValue, s_nPrevStep);

	return 0;
}

int Transact_GetStep(void)
{
	return s_nStep;
}

int Transact_SetPrevStep(int nValue)
{
#if 0
	Locking();
	{
		TR_LOG_OUT("3> Curr => s_nState(%d), s_nStep(%d), s_nPrevStep(%d), change => nPrev(%d) !!", s_nState, s_nStep, s_nPrevStep, nValue);
		s_nPrevStep = nValue;
	}
	UnLocking();
#endif

	TR_AddQueData(TR_CMD_PREV_STEP, s_nState, s_nStep, nValue);

	return 0;
}

/**
 * @brief		Transact_SetState
 * @details		State값 설정
 * @param		int nValue			설정값
 * @return		항상 = 0
 */
int Transact_SetState(int nState, int nStep)
{
#if 0
	Locking();
	{
		TR_LOG_OUT("1> Curr => s_nState(%d), s_nStep(%d), s_nPrevStep(%d), change => nState(%d), nStep(%d) !!", s_nState, s_nStep, s_nPrevStep, nState, nStep);
		s_nState = nState;
		s_nPrevStep = -1;
		s_nStep = nStep;
	}
	UnLocking();
#endif

	TR_AddQueData(TR_CMD_ALL, nState, nStep, -1);

	return 0;
}

/**
 * @brief		Transact_GetState
 * @details		State값 가져오기
 * @param		None
 * @return		State값
 */
int Transact_GetState(void)
{
	return s_nState;
}

/**
 * @brief		Transact_Initialize
 * @details		Transaction 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Transact_LogInitialize(void)
{
	LOG_INIT();

	return 0;
}

/**
 * @brief		Transact_Initialize
 * @details		Transaction 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Transact_Initialize(void)
{
	DWORD dwThreadID;

	TR_LOG_OUT("############################## start !!");

	pEnv = (PDEV_CFG_T) GetEnvUIInfo();
	
	if(hThread != NULL)
	{
		::CloseHandle(hThread);
		hThread = NULL;
	}

	hAccMutex = ::CreateMutex(NULL, FALSE, NULL);
	if(hAccMutex == NULL) 
	{
		TR_LOG_OUT("CreateMutex() failure..");
		return -1;
	}

	hThread = ::CreateThread(NULL, 0, TransactThread, NULL, CREATE_SUSPENDED, &dwThreadID);
	if(hThread == NULL) 
	{
		return -2;
	}

	::ResumeThread(hThread);

	return 0;
}

/**
 * @brief		Transact_Terminate
 * @details		Transaction 종료
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Transact_Terminate(void)
{
	TR_LOG_OUT("############################## start, s_bRun(%d) !!", s_bRun);

	if(s_bRun == TRUE)
	{
		s_bRun = FALSE;

		if(hThread != NULL)	
		{
			::WaitForSingleObject(hThread, 500);
			::CloseHandle(hThread);
			hThread = NULL;
		}

		if(hAccMutex != NULL)
		{
			::CloseHandle(hAccMutex);
			hAccMutex = NULL;
		}
	}

	return 0;
}

/**
 * @brief		KTC_MemClear
 * @details		메모리 초기화
 * @param		void *pData			초기화할 메모리 버퍼
 * @param		int val				초기화 값
 * @param		int nSize			길이
 * @return		항상 0
 */
// int KTC_MemClear(void *pData, int nSize)
// {
// 	memset(pData, 0x00, nSize);
// 	memset(pData, 0xFF, nSize);
// 	memset(pData, 0x00, nSize);
// 
// 	return 0;
// }

/**
 * @brief		MakeDepartureDateTime
 * @details		승차권 인쇄 출발일자, 출발시간 포맷 작성
 * @param		char *pDT		원본 - 출발일자	
 * @param		char *pTime		원본 - 출발시간		
 * @param		char *retDT		작성 - 출발일자	
 * @param		char *retTM		작성 - 출발시간			
 * @return		None
 */
void MakeDepartureDateTime(char *pDT, char *pTime, char *retDT, char *retTM)
{
	int nWeek;
	SYSTEMTIME stDepr;
	char *pWeekStr[] = { "일", "월", "화", "수", "목", "금", "토", };
	PKIOSK_INI_ENV_T pEnv;

	pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();

	/// (12). 실제 출발 일자
	{
		stDepr.wYear  = (WORD) Util_Ascii2Long((char *)&pDT[0], 4);
		stDepr.wMonth = (WORD) Util_Ascii2Long((char *)&pDT[4], 2);
		stDepr.wDay   = (WORD) Util_Ascii2Long((char *)&pDT[6], 2);
	}

	stDepr.wHour   = (WORD) Util_Ascii2Long((char *)&pTime[0], 2);
	stDepr.wMinute = (WORD) Util_Ascii2Long((char *)&pTime[2], 2);
	stDepr.wSecond = 0;

	CTime myTime((int)stDepr.wYear, (int)stDepr.wMonth, (int)stDepr.wDay, 8, 0, 0);
	nWeek = myTime.GetDayOfWeek();
	
	if( memcmp(pEnv->tTckOpt.depr_date_fmt, "MMDD", 4) == 0 )
	{
		if( (nWeek - 1) < 7 )
		{
			sprintf(retDT, "%02d.%02d %s", stDepr.wMonth, stDepr.wDay, pWeekStr[nWeek - 1]);
		}
		else
		{
			sprintf(retDT, "%02d.%02d", stDepr.wMonth, stDepr.wDay);
		}
	}
	else
	{
		sprintf(retDT, "%04d.%02d.%02d", stDepr.wYear, stDepr.wMonth, stDepr.wDay);
	}

	sprintf(retTM, "%02d:%02d", stDepr.wHour, stDepr.wMinute);
}

/**
 * @brief		TR_AddQueData
 * @details		Transaction 명령 큐에 넣기
 * @param		int nCommand	Command	
 * @param		int nState		State
 * @param		int nStep		Step 
 * @param		int nPrevStep	Prev Step	
 * @return		항상 = 0
 */
int TR_AddQueData(int nCommand, int nState, int nStep, int nPrevStep)
{
	TR_QUE_DATA_T	tTrData;

	//TR_LOG_OUT(" Command(%d), STATE(%d), STEP(%d), PSTEP(%d) !", nCommand, nState, nStep, nPrevStep);

	Locking();
	{
		::ZeroMemory(&tTrData, sizeof(TR_QUE_DATA_T));

		tTrData.nCommand = nCommand;

		if(nCommand == TR_CMD_STATE)
		{
			tTrData.nState = nState;
		}
		else if(nCommand == TR_CMD_STEP)
		{
			tTrData.nStep = nStep;
		}
		else if(nCommand == TR_CMD_PREV_STEP)
		{
			tTrData.nPrevStep = nPrevStep;
		}
		else if(nCommand == TR_CMD_ALL)
		{
			tTrData.nState = nState;
			tTrData.nStep = nStep;
			tTrData.nPrevStep = nPrevStep;
		}

		s_QueData.push(tTrData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		TransactThread
 * @details		Transaction Thread
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
DWORD WINAPI TransactThread(void *)
{
	int				nSize = 0;
	TR_QUE_DATA_T	tTrData;

	s_bRun = TRUE;
	s_nState = TR_BOOT_STATE;

	::ZeroMemory(&tTrData, sizeof(TR_QUE_DATA_T));

	TR_LOG_OUT(" ###################### start ###################### ");

	while(s_bRun) 
	{
		Sleep(10);	// 10 ms	

		nSize = s_QueData.size();
		if(nSize > 0)
		{
			::ZeroMemory(&tTrData, sizeof(TR_QUE_DATA_T));

			tTrData = s_QueData.front();
			s_QueData.pop();

			if(tTrData.nCommand == TR_CMD_STATE)
			{
				//TR_LOG_OUT(" [TR_CMD_STATE] = S_STATE(%d), S_STEP(%d), S_PSTEP(%d), STATE(%d) !", s_nState, s_nStep, s_nPrevStep, tTrData.nState);
				s_nState	= tTrData.nState;
			}
			else if(tTrData.nCommand == TR_CMD_STEP)
			{
				//TR_LOG_OUT(" [TR_CMD_STEP] = S_STATE(%d), S_STEP(%d), S_PSTEP(%d), STEP(%d) !", s_nState, s_nStep, s_nPrevStep, tTrData.nStep);
				s_nStep		= tTrData.nStep;
			}
			else if(tTrData.nCommand == TR_CMD_PREV_STEP)
			{
				//TR_LOG_OUT(" [TR_CMD_PREV_STEP] = S_STATE(%d), S_STEP(%d), S_PSTEP(%d), PSTEP(%d) !", s_nState, s_nStep, s_nPrevStep, tTrData.nPrevStep);
				s_nPrevStep = tTrData.nPrevStep;
			}
			else if(tTrData.nCommand == TR_CMD_ALL)
			{
				//TR_LOG_OUT(" [TR_CMD_ALL] = S_STATE(%d), S_STEP(%d), S_PSTEP(%d), STATE(%d), STEP(%d), PSTEP(%d) !", 
				//			s_nState, s_nStep, s_nPrevStep, tTrData.nState, tTrData.nStep, tTrData.nPrevStep);
				s_nState	= tTrData.nState;
				s_nStep		= tTrData.nStep;
				s_nPrevStep = tTrData.nPrevStep;
			}
			else
			{
				TR_LOG_OUT(" [TR_CMD_unknown] = Command(%08X), S_STATE(%d), S_STEP(%d), S_PSTEP(%d) !!!!!", tTrData.nCommand & 0xFFFFFFFF, s_nState, s_nStep, s_nPrevStep);
			}
		}

#ifdef __DEV_TR_SPLIT__
		switch(GetConfigPayment())
		{
		case PAY_ONLY_CARD :
		case PAY_ONLY_RF :
		case PAY_CARD_RF :
			Transaction_OnCardOnlyMain(s_nState, s_nPrevStep, s_nStep);
			break;
		case PAY_ONLY_CASH :
		case PAY_CARD_CASH :
			Transaction_OnCardCashMain(s_nState, s_nPrevStep, s_nStep);
			break;
		}
#else
		Transaction_OnCardCashMain(s_nState, s_nPrevStep, s_nStep);
#endif


	} // while(1) 

	Transact_Terminate();

	return 0L;
}

