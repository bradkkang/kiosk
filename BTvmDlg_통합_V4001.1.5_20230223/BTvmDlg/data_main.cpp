
#include "stdafx.h"
#include <stdio.h>
#include <string.h>

#include "MyUtil.h"
#include "MyFileUtil.h"
#include "dev_tr_main.h"
#include "data_main.h"
#include "MyDataTr.h"
#include "MyDataAccum.h"
#include "oper_kos_file.h"
#include "oper_config.h"
#include "dev_tr_mem.h"
#include "dev_tr_kobus_mem.h"
#include "dev_tr_tmexp_mem.h"

#include "DbSqlite.h"

//----------------------------------------------------------------------------------------------------------------------
//#define _BASE_DATA_		1

#ifdef _BASE_DATA_
static CTrDataMng		*s_pclsCashTR = NULL;
static CTrDataMng		*s_pclsCloseTR = NULL;
static CTrDataMng		*s_pclsDayCloseTR = NULL;
static CTrDataMng		*s_pclsSmsTR = NULL;

#else

static CTrCashFile			*s_pclsCashTR = NULL;
static CTrCashCloseFile		*s_pclsCloseTR = NULL;
static CTrDayCloseFile		*s_pclsDayCloseTR = NULL;
static CTrSmsFile			*s_pclsSmsTR = NULL;

#endif

static int			s_nMaxSave = 30;

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		InitCashTrData
 * @details		입출금 내역 데이타 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int InitCashTrData(void)
{
	int nRet = 0;
	char Buffer[256];
	CString strPath;

	try
	{
#ifdef _BASE_DATA_
		s_pclsCashTR = new CTrDataMng();
#else
		s_pclsCashTR = new CTrCashFile();
#endif

		{
			USES_CONVERSION;

			::ZeroMemory(Buffer, sizeof(Buffer));
			Util_GetModulePath(Buffer);

			//strPath = (CString) Buffer;
			strPath = (CString) Buffer + _T("\\Data\\") + _T("CashTR");
		}

		nRet = s_pclsCashTR->InitData(strPath, s_nMaxSave);
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		TermCashTrData
 * @details		입출금 내역 데이타 종료 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int TermCashTrData(void)
{
	try
	{
		if( s_pclsCashTR != NULL )
		{
			s_pclsCashTR->TermData();
			delete s_pclsCashTR;
			s_pclsCashTR = NULL;
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		AddCashTrData
 * @details		입출금 내역 데이타 저장
 * @param		int nFlag, WORD w100, WORD w500, WORD w1k, WORD w5k, WORD w10k, WORD w50k
 * @return		성공 >= 0, 실패 < 0
 */
int AddCashTrData(int nFlag, WORD w100, WORD w500, WORD w1k, WORD w5k, WORD w10k, WORD w50k)
{
	int nRet = 0;
	CASH_TR_DATA_T cashInfo;
	SYSTEMTIME st;

	try
	{
		if(s_pclsCashTR == NULL)
		{
			return -1;
		}

		::ZeroMemory(&cashInfo, sizeof(CASH_TR_DATA_T));

		GetLocalTime(&st);

		/// 발생일시
		sprintf(cashInfo.szDateTm, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		/// 창구번호
		sprintf(cashInfo.szWndNo, "%s", GetTrmlWndNo(SVR_DVS_CCBUS));
		/// 내역구분
		cashInfo.chFlag = nFlag & 0xFF;
		cashInfo.w100	= w100;
		cashInfo.w500	= w500;
		cashInfo.w1k	= w1k;
		cashInfo.w5k	= w5k;
		cashInfo.w10k	= w10k;
		cashInfo.w50k	= w50k;
		cashInfo.nSum = (w100 * 100) + (w500 * 500) + (w1k * 1000) + (w5k * 5000) + (w10k * 10000) + (w50k * 50000);

		nRet = s_pclsCashTR->AddData((char *)&cashInfo, (int)sizeof(CASH_TR_DATA_T));
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		InitCashCloseData
 * @details		시재마감 내역 데이타 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int InitCashCloseData(void)
{
	int		nRet = 0;
	char	Buffer[256];
	CString strPath;

	try
	{
#ifdef _BASE_DATA_
		s_pclsCloseTR = new CTrDataMng();
#else
		s_pclsCloseTR = new CTrCashCloseFile();
#endif

		{
			USES_CONVERSION;

			::ZeroMemory(Buffer, sizeof(Buffer));
			Util_GetModulePath(Buffer);

			//strPath = (CString) Buffer;
			strPath = (CString) Buffer + _T("\\Data\\") + _T("CashClsNew");
		}

		nRet = s_pclsCloseTR->InitData(strPath, s_nMaxSave);
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		TermCashTrData
 * @details		마감 내역 데이타 종료 처리
 * @param		None
 * @return		항상 0
 */
int TermCashCloseData(void)
{
	try
	{
		if( s_pclsCloseTR != NULL )
		{
			s_pclsCloseTR->TermData();
			delete s_pclsCloseTR;
			s_pclsCloseTR = NULL;
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		AddCashTrData
 * @details		시재마감 내역 데이타 저장
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int AddCashCloseData(void)
{
	int				i, k, nRet = 0;
	CLOSE_TR_DATA_T closeInfo;
	SYSTEMTIME		st;

	try
	{
		if(s_pclsCashTR == NULL)
		{
			return -1;
		}

		PFILE_ACCUM_N1010_T pAccum = GetAccumulateData();

		::ZeroMemory(&closeInfo, sizeof(CLOSE_TR_DATA_T));

		GetLocalTime(&st);

		/// 시재마감 시작일시
		sprintf(closeInfo.szBegDateTm, "%s", pAccum->cash_cls_stt_dt);
		/// 시재마감 종료일시(=발생일시)
		sprintf(closeInfo.szEndDateTm, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		/// 창구번호
		sprintf(closeInfo.szWndNo, "%s", GetTrmlWndNo(SVR_DVS_CCBUS));

		///> 초기 내역
		closeInfo.initInfo.w100 = (WORD) pAccum->Close.initCoin.n100;
		closeInfo.initInfo.w500 = (WORD) pAccum->Close.initCoin.n500;
		closeInfo.initInfo.w1k = (WORD) pAccum->Close.initBill.n1k;
		closeInfo.initInfo.w5k = (WORD) pAccum->Close.initBill.n5k;
		closeInfo.initInfo.w10k = (WORD) pAccum->Close.initBill.n10k;
		closeInfo.initInfo.w50k = (WORD) pAccum->Close.initBill.n50k;
		///> 입금 내역 (지폐)
		closeInfo.insInfo.w1k = (WORD) pAccum->Close.inBill.n1k;
		closeInfo.insInfo.w5k = (WORD) pAccum->Close.inBill.n5k;
		closeInfo.insInfo.w10k = (WORD) pAccum->Close.inBill.n10k;
		closeInfo.insInfo.w50k = (WORD) pAccum->Close.inBill.n50k;
		///> 출금 내역 (동전/지폐)
		closeInfo.outInfo.w100 = (WORD) pAccum->Close.outCoin.n100;
		closeInfo.outInfo.w500 = (WORD) pAccum->Close.outCoin.n500;
		closeInfo.outInfo.w1k = (WORD) pAccum->Close.outBill.n1k;
		closeInfo.outInfo.w5k = (WORD) pAccum->Close.outBill.n5k;
		closeInfo.outInfo.w10k = (WORD) pAccum->Close.outBill.n10k;
		closeInfo.outInfo.w50k = (WORD) pAccum->Close.outBill.n50k;
		///> 보급 내역
		closeInfo.supplyInfo.w100 = (WORD) pAccum->Close.supplyCoin.n100;
		closeInfo.supplyInfo.w500 = (WORD) pAccum->Close.supplyCoin.n500;
		closeInfo.supplyInfo.w1k = (WORD) pAccum->Close.supplyBill.n1k;
		closeInfo.supplyInfo.w5k = (WORD) pAccum->Close.supplyBill.n5k;
		closeInfo.supplyInfo.w10k = (WORD) pAccum->Close.supplyBill.n10k;
		closeInfo.supplyInfo.w50k = (WORD) pAccum->Close.supplyBill.n50k;
		///> 회수 내역 (지폐함)
		closeInfo.wdrawInfo.w100 = 0;
		closeInfo.wdrawInfo.w500 = 0;
		closeInfo.wdrawInfo.w1k = (WORD) pAccum->Close.wdrawBill.n1k;
		closeInfo.wdrawInfo.w5k = (WORD) pAccum->Close.wdrawBill.n5k;
		closeInfo.wdrawInfo.w10k = (WORD) pAccum->Close.wdrawBill.n10k;
		closeInfo.wdrawInfo.w50k = (WORD) pAccum->Close.wdrawBill.n50k;
		///> 미출금 내역
		closeInfo.noOutInfo.w100 = (WORD) pAccum->Close.noCoin.n100;
		closeInfo.noOutInfo.w500 = (WORD) pAccum->Close.noCoin.n500;
		closeInfo.noOutInfo.w1k = (WORD) pAccum->Close.noBill.n1k;
		closeInfo.noOutInfo.w5k = (WORD) pAccum->Close.noBill.n5k;
		closeInfo.noOutInfo.w10k = (WORD) pAccum->Close.noBill.n10k;
		closeInfo.noOutInfo.w50k = (WORD) pAccum->Close.noBill.n50k;
		///> 현재 내역
		closeInfo.currInfo;				
		closeInfo.currInfo.w100 = (WORD) pAccum->Curr.Coin.n100;
		closeInfo.currInfo.w500 = (WORD) pAccum->Curr.Coin.n500;
		closeInfo.currInfo.w1k = (WORD) pAccum->Curr.BillDispenser.Casst.n1k;
		closeInfo.currInfo.w5k = (WORD) pAccum->Curr.BillDispenser.Casst.n5k;
		closeInfo.currInfo.w10k = (WORD) pAccum->Curr.BillDispenser.Casst.n10k;
		closeInfo.currInfo.w50k = (WORD) pAccum->Curr.BillDispenser.Casst.n50k;

		/// 현장발권
		/***

		for(i = 0; i < CCS_IDX_TCK_MAX; i++)
		{
			PDM_TICKET_WORK_T pCCS = &pAccum->MnuCls.ccsTicketWork[i];

			//for(k = 0; k < 2; k++)
			{
				k = IDX_ACC_CASH;

				closeInfo.ccsPubTckSum.nCount	+= pCCS->tPubTck[k].nCount;
				closeInfo.ccsPubTckSum.dwMoney	+= pCCS->tPubTck[k].dwMoney;
			}
		}

		for(i = 0; i < TMEXP_IDX_TCK_MAX; i++)
		{
			PDM_TICKET_WORK_T pExp = &pAccum->MnuCls.expTicketWork[i];

			//for(k = 0; k < 2; k++)
			{
				k = IDX_ACC_CASH;

				closeInfo.expPubTckSum.nCount	+= pExp->tPubTck[k].nCount;
				closeInfo.expPubTckSum.dwMoney	+= pExp->tPubTck[k].dwMoney;
			}
		}

		/// 환불

		for(i = 0; i < CCS_IDX_TCK_MAX; i++)
		{
			PDM_TICKET_WORK_T pCCS = &pAccum->MnuCls.ccsTicketWork[i];

			//for(k = 0; k < 2; k++)
			{
				k = IDX_ACC_CASH;

				closeInfo.ccsRefundSum.nCount  += pCCS->tRefund[k].nCount;
				closeInfo.ccsRefundSum.dwMoney += pCCS->tRefund[k].dwMoney;
			}
		}

		for(i = 0; i < TMEXP_IDX_TCK_MAX; i++)
		{
			PDM_TICKET_WORK_T pExp = &pAccum->MnuCls.expTicketWork[i];

			//for(k = 0; k < 2; k++)
			{
				k = IDX_ACC_CASH;

				closeInfo.expRefundSum.nCount  += pExp->tRefund[k].nCount;
				closeInfo.expRefundSum.dwMoney += pExp->tRefund[k].dwMoney;
			}
		}
		***/

		/// 승차권 데이타 부분
		{
			// 시외/고속
			{
				::CopyMemory(&closeInfo.ccsTicketWork[0], &pAccum->MnuCls.ccsTicketWork[0], sizeof(DM_TICKET_WORK_T) * CCS_IDX_TCK_MAX);
				::CopyMemory(&closeInfo.expTicketWork[0], &pAccum->MnuCls.expTicketWork[0], sizeof(DM_TICKET_WORK_T) * TMEXP_IDX_TCK_MAX);
			}
		}

		nRet = s_pclsCloseTR->AddData((char *)&closeInfo, (int)sizeof(CLOSE_TR_DATA_T));
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		SearchCashCloseData
 * @details		시재마감 내역에서 데이타 찾기
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int SearchCashCloseData(char *pDate, char *pTime, char *retBuf)
{
	int nRet = -1;

	try
	{
		if(s_pclsDayCloseTR == NULL)
		{
			return -1;
		}
		
		nRet = s_pclsCloseTR->SearchData(pDate, pTime, 8, retBuf);
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		InitDayCloseData
 * @details		일마감 내역 데이타 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int InitDayCloseData(void)
{
	int		nRet = 0;
	char	Buffer[256];
	CString strPath;

	try
	{
#ifdef _BASE_DATA_
		s_pclsDayCloseTR = new CTrDataMng();
#else
		s_pclsDayCloseTR = new CTrDayCloseFile();
#endif

		{
			USES_CONVERSION;

			::ZeroMemory(Buffer, sizeof(Buffer));
			Util_GetModulePath(Buffer);

			//strPath = (CString) Buffer;
			strPath = (CString) Buffer + _T("\\Data\\") + _T("DayCls");
		}

		nRet = s_pclsDayCloseTR->InitData(strPath, s_nMaxSave);
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		TermDayCloseData
 * @details		일마감 내역 데이타 종료 처리
 * @param		None
 * @return		항상 0
 */
int TermDayCloseData(void)
{
	try
	{
		if( s_pclsDayCloseTR != NULL )
		{
			s_pclsDayCloseTR->TermData();
			delete s_pclsDayCloseTR;
			s_pclsDayCloseTR = NULL;
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		AddDayCloseData
 * @details		시재마감 내역 데이타 저장
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int AddDayCloseData(void)
{
	int					nRet = 0, i = 0;
	DAY_CLOSE_TR_DATA_T closeInfo;
	SYSTEMTIME			st;
	PDM_TICKET_WORK_T	pTckWork;

	try
	{
		if(s_pclsDayCloseTR == NULL)
		{
			return -1;
		}

		PFILE_ACCUM_N1010_T pAccum = GetAccumulateData();

		::ZeroMemory(&closeInfo, sizeof(DAY_CLOSE_TR_DATA_T));

		GetLocalTime(&st);

		/// 시작일시
		//sprintf(closeInfo.szBegDateTm, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond); // // 20220913 DEL
// 20220913 ADD~
		POPER_FILE_CONFIG_T pOperCfg;
		pOperCfg = (POPER_FILE_CONFIG_T) GetOperConfigData();

#if EM_TICKETING==1 // 비상발권 모드 (비상발권모드:1, 정상모드:0)
		if (strlen(pOperCfg->em_start_dt) > 0)
		{
			::CopyMemory(closeInfo.szBegDateTm, &pOperCfg->em_start_dt[0], sizeof(closeInfo.szBegDateTm));			// 창구시작일자
#else
		if (strlen(pOperCfg->job_start_dt) > 0)
		{
			::CopyMemory(closeInfo.szBegDateTm, &pOperCfg->job_start_dt[0], sizeof(closeInfo.szBegDateTm));			// 창구시작일자
#endif
		}
		else
		{
			sprintf(closeInfo.szBegDateTm, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		}
// 20220913 ~ADD
		/// 종료일시
		sprintf(closeInfo.szEndDateTm, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		/// 창구번호
		sprintf(closeInfo.szWndNo, "%s", GetTrmlWndNo(SVR_DVS_CCBUS));

		/// 시재 데이타 부분
		{
			///> 초기 내역
			closeInfo.initInfo.w100 = (WORD) pAccum->Close.initCoin.n100;
			closeInfo.initInfo.w500 = (WORD) pAccum->Close.initCoin.n500;
			closeInfo.initInfo.w1k = (WORD) pAccum->Close.initBill.n1k;
			closeInfo.initInfo.w5k = (WORD) pAccum->Close.initBill.n5k;
			closeInfo.initInfo.w10k = (WORD) pAccum->Close.initBill.n10k;
			closeInfo.initInfo.w50k = (WORD) pAccum->Close.initBill.n50k;
			///> 입금 내역 (지폐)
			closeInfo.insInfo.w1k = (WORD) pAccum->Close.inBill.n1k;
			closeInfo.insInfo.w5k = (WORD) pAccum->Close.inBill.n5k;
			closeInfo.insInfo.w10k = (WORD) pAccum->Close.inBill.n10k;
			closeInfo.insInfo.w50k = (WORD) pAccum->Close.inBill.n50k;
			///> 출금 내역 (동전/지폐)
			closeInfo.outInfo.w100 = (WORD) pAccum->Close.outCoin.n100;
			closeInfo.outInfo.w500 = (WORD) pAccum->Close.outCoin.n500;
			closeInfo.outInfo.w1k = (WORD) pAccum->Close.outBill.n1k;
			closeInfo.outInfo.w5k = (WORD) pAccum->Close.outBill.n5k;
			closeInfo.outInfo.w10k = (WORD) pAccum->Close.outBill.n10k;
			closeInfo.outInfo.w50k = (WORD) pAccum->Close.outBill.n50k;
			///> 보급 내역
			closeInfo.supplyInfo.w100 = (WORD) pAccum->Close.supplyCoin.n100;
			closeInfo.supplyInfo.w500 = (WORD) pAccum->Close.supplyCoin.n500;
			closeInfo.supplyInfo.w1k = (WORD) pAccum->Close.supplyBill.n1k;
			closeInfo.supplyInfo.w5k = (WORD) pAccum->Close.supplyBill.n5k;
			closeInfo.supplyInfo.w10k = (WORD) pAccum->Close.supplyBill.n10k;
			closeInfo.supplyInfo.w50k = (WORD) pAccum->Close.supplyBill.n50k;
			///> 회수 내역 (지폐함)
			closeInfo.wdrawInfo.w100 = 0;
			closeInfo.wdrawInfo.w500 = 0;
			closeInfo.wdrawInfo.w1k = (WORD) pAccum->Close.wdrawBill.n1k;
			closeInfo.wdrawInfo.w5k = (WORD) pAccum->Close.wdrawBill.n5k;
			closeInfo.wdrawInfo.w10k = (WORD) pAccum->Close.wdrawBill.n10k;
			closeInfo.wdrawInfo.w50k = (WORD) pAccum->Close.wdrawBill.n50k;
			///> 미출금 내역
			closeInfo.noOutInfo.w100 = (WORD) pAccum->Close.noCoin.n100;
			closeInfo.noOutInfo.w500 = (WORD) pAccum->Close.noCoin.n500;
			closeInfo.noOutInfo.w1k = (WORD) pAccum->Close.noBill.n1k;
			closeInfo.noOutInfo.w5k = (WORD) pAccum->Close.noBill.n5k;
			closeInfo.noOutInfo.w10k = (WORD) pAccum->Close.noBill.n10k;
			closeInfo.noOutInfo.w50k = (WORD) pAccum->Close.noBill.n50k;
			///> 현재 내역
			closeInfo.currInfo;				
			closeInfo.currInfo.w100 = (WORD) pAccum->Curr.Coin.n100;
			closeInfo.currInfo.w500 = (WORD) pAccum->Curr.Coin.n500;
			closeInfo.currInfo.w1k = (WORD) pAccum->Curr.BillDispenser.Casst.n1k;
			closeInfo.currInfo.w5k = (WORD) pAccum->Curr.BillDispenser.Casst.n5k;
			closeInfo.currInfo.w10k = (WORD) pAccum->Curr.BillDispenser.Casst.n10k;
			closeInfo.currInfo.w50k = (WORD) pAccum->Curr.BillDispenser.Casst.n50k;
		}

		/// 승차권 데이타 부분
		{
			// 시외/고속
			{
				::CopyMemory(&closeInfo.ccsTicketWork[0], &pAccum->Curr.ccsTicketWork[0], sizeof(DM_TICKET_WORK_T) * CCS_IDX_TCK_MAX);
				::CopyMemory(&closeInfo.expTicketWork[0], &pAccum->Curr.expTicketWork[0], sizeof(DM_TICKET_WORK_T) * TMEXP_IDX_TCK_MAX);
			}
		}

		nRet = s_pclsDayCloseTR->AddData((char *)&closeInfo, (int)sizeof(DAY_CLOSE_TR_DATA_T));
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		SearchDayCloseData
 * @details		창구마감 내역에서 데이타 찾기
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int SearchDayCloseData(char *pDate, char *pTime, char *retBuf)
{
	int nRet = -1;

	try
	{
		if(s_pclsDayCloseTR == NULL)
		{
			return -1;
		}
		
		nRet = s_pclsDayCloseTR->SearchData(pDate, pTime, 8, retBuf);
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}


/**
 * @brief		InitSmsTRData
 * @details		관제시스템용 거래 내역 데이타 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int InitSmsTRData(void)
{
	int			nRet = 0;
	char		Buffer[256];
	CString		strPath;

	try
	{
#ifdef _BASE_DATA_
		s_pclsSmsTR = new CTrDataMng();
#else
		s_pclsSmsTR = new CTrSmsFile();
#endif

		{
			USES_CONVERSION;

			::ZeroMemory(Buffer, sizeof(Buffer));
			Util_GetModulePath(Buffer);

			strPath = (CString) Buffer + _T("\\Data\\") + _T("SmsTR");
		}

		nRet = s_pclsSmsTR->InitData(strPath, s_nMaxSave);
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}


	return nRet;
}

/**
 * @brief		TermSmsTRData
 * @details		관제시스템용 거래 내역 데이타 종료 처리
 * @param		None
 * @return		항상 0
 */
int TermSmsTRData(void)
{
	try
	{
		if( s_pclsSmsTR != NULL )
		{
			s_pclsSmsTR->TermData();
			delete s_pclsSmsTR;
			s_pclsSmsTR = NULL;
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		AddSmsTRData
 * @details		관제시스템용 거래 내역 데이타 저장
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int AddSmsTRData(int nFunc, int nBusDvs)
{
	int				nRet, i, nOffset;
	SYSTEMTIME		st;
	SMS_TR_DATA_T	trInfo;

	nRet = i = nOffset = 0;

	TR_LOG_OUT("%s", "############## start ");

	try
	{
		if(s_pclsSmsTR == NULL)
		{
			return -1;
		}

		PFILE_ACCUM_N1010_T pAccum = GetAccumulateData();

		::ZeroMemory(&trInfo, sizeof(SMS_TR_DATA_T));

		::GetLocalTime(&st);

		///> 시외('0'), 코버스('1'), 티머니고속('2')
		if( nBusDvs == SVR_DVS_CCBUS )
		{
			trInfo.BusDvs[0] = '0';
		}
		else if( nBusDvs == SVR_DVS_KOBUS )
		{
			trInfo.BusDvs[0] = '1';
		}
		else
		{
			trInfo.BusDvs[0] = '2';
		}
		///> 발생일시
		sprintf(trInfo.szHappenDt, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

		if(nFunc == enFUNC_PBTCK)
		{	/// 현장발권
			if( nBusDvs == SVR_DVS_CCBUS )
			{
				vector<TCK_PRINT_FMT_T>::iterator iter;
				CPubTckMem* pTR;

				pTR = CPubTckMem::GetInstance();

				///> "A":구매,현금자진발급, "B":구매,현금영수증, "C":구매,신용카드, "8":환불,현금, "9":환불,카드, "Z":환불,기타
				trInfo.TrDvs[0] = pTR->base.pyn_mns_dvs_cd[0];
				///> 거래금액
				trInfo.nFare = pTR->base.nTotalMoney;
				///> 발권수량
				trInfo.nCount = pTR->m_vtPrtTicket.size();

				///> 승차권번호, 예시 -> ["20190621-999-16-0009-25", "20190621-1000-16-0009-26"]
				trInfo.szTicketInfo[nOffset++] = '[';
				for(iter = pTR->m_vtPrtTicket.begin(); iter != pTR->m_vtPrtTicket.end(); iter++, i++)
				{
					nOffset += sprintf(&trInfo.szTicketInfo[nOffset], "\"%s-%s-%s-%s-%s\"", 
						iter->bar_pub_dt, 
						iter->bar_pub_shct_trml_cd, 
						iter->bar_pub_wnd_no, 
						iter->bar_pub_sno, 
						iter->bar_secu_code);
					if( i == (trInfo.nCount - 1) )
					{
						trInfo.szTicketInfo[nOffset++] = ']';
					}
					else
					{
						trInfo.szTicketInfo[nOffset++] = ',';
					}
					TR_LOG_OUT("거래 데이타 - (%02d) 승차권 데이타 = %s", i, trInfo.szTicketInfo);
				}
				///> 총 투입금액
				trInfo.nTotInsMoney = (pTR->base.insBill.n1k * 1000) + (pTR->base.insBill.n5k * 5000) + (pTR->base.insBill.n10k * 10000) + (pTR->base.insBill.n50k * 50000);
				///> 투입수량 1,000
				trInfo.nIns1k = pTR->base.insBill.n1k;
				///> 투입수량 5,000
				trInfo.nIns5k = pTR->base.insBill.n5k;
				///> 투입수량 10,000
				trInfo.nIns10k = pTR->base.insBill.n10k;
				///> 투입수량 50,000
				trInfo.nIns50k = pTR->base.insBill.n50k;

				///> 총 방출금액
				trInfo.nTotOutMoney = (pTR->base.outCoin.n100 * 100) + (pTR->base.outCoin.n500 * 500) + (pTR->base.outBill.n1k * 1000) + (pTR->base.outBill.n10k * 10000);
				///> 방출수량 100
				trInfo.nOut100 = pTR->base.outCoin.n100;
				///> 방출수량 500
				trInfo.nOut500 = pTR->base.outCoin.n500;
				///> 방출수량 1,000
				trInfo.nOut1k = pTR->base.outBill.n1k;
				///> 방출수량 5,000
				trInfo.nOut5k = 0;
				///> 방출수량 10,000
				trInfo.nOut10k = pTR->base.outBill.n10k;
			}
			else if( nBusDvs == SVR_DVS_KOBUS )
			{
				vector<TCK_PRINT_FMT_T>::iterator iter;
				CPubTckKobusMem* pTR;

				pTR = CPubTckKobusMem::GetInstance();

				///> "A":구매,현금자진발급, "B":구매,현금영수증, "C":구매,신용카드, "8":환불,현금, "9":환불,카드, "Z":환불,기타
				trInfo.TrDvs[0] = pTR->base.ui_pym_dvs_cd[0];	// 20221124 DEL // 결제구분코드
				//trInfo.TrDvs[0] = pTR->base.pyn_mns_dvs_cd[0];	// 20221124 MOD // 지불수단구분코드
				///> 거래금액
				trInfo.nFare = pTR->base.nTotalMoney;
				///> 발권수량
				trInfo.nCount = pTR->m_vtPrtTicket.size();

				///> 승차권번호, 예시 -> ["20190621-999-16-0009-25", "20190621-1000-16-0009-26"]
				trInfo.szTicketInfo[nOffset++] = '[';
				for(iter = pTR->m_vtPrtTicket.begin(); iter != pTR->m_vtPrtTicket.end(); iter++, i++)
				{
// 					nOffset += sprintf(&trInfo.szTicketInfo[nOffset], "\"%s-%s-%s-%s-%s-%s\"", 
// 						iter->qr_pub_dt, 
// 						iter->qr_pub_trml_cd, 
// 						iter->qr_pub_wnd_no, 
// 						iter->qr_depr_trml_no, 
// 						iter->qr_arvl_trml_no, 
// 						iter->qr_sats_no);
					nOffset += sprintf(&trInfo.szTicketInfo[nOffset], "\"%s-%s-%s-%s\"", 
						iter->qr_pub_dt, 
						iter->qr_pub_trml_cd, 
						iter->qr_pub_wnd_no, 
						iter->qr_pub_sno); 
					if( i == (trInfo.nCount - 1) )
					{
						trInfo.szTicketInfo[nOffset++] = ']';
					}
					else
					{
						trInfo.szTicketInfo[nOffset++] = ',';
					}
					TR_LOG_OUT("거래 데이타 - (%02d) 승차권 데이타 = %s", i, trInfo.szTicketInfo);
				}
				///> 총 투입금액
				trInfo.nTotInsMoney = (pTR->base.insBill.n1k * 1000) + (pTR->base.insBill.n5k * 5000) + (pTR->base.insBill.n10k * 10000) + (pTR->base.insBill.n50k * 50000);
				///> 투입수량 1,000
				trInfo.nIns1k = pTR->base.insBill.n1k;
				///> 투입수량 5,000
				trInfo.nIns5k = pTR->base.insBill.n5k;
				///> 투입수량 10,000
				trInfo.nIns10k = pTR->base.insBill.n10k;
				///> 투입수량 50,000
				trInfo.nIns50k = pTR->base.insBill.n50k;

				///> 총 방출금액
				trInfo.nTotOutMoney = (pTR->base.outCoin.n100 * 100) + (pTR->base.outCoin.n500 * 500) + (pTR->base.outBill.n1k * 1000) + (pTR->base.outBill.n10k * 10000);
				///> 방출수량 100
				trInfo.nOut100 = pTR->base.outCoin.n100;
				///> 방출수량 500
				trInfo.nOut500 = pTR->base.outCoin.n500;
				///> 방출수량 1,000
				trInfo.nOut1k = pTR->base.outBill.n1k;
				///> 방출수량 5,000
				trInfo.nOut5k = 0;
				///> 방출수량 10,000
				trInfo.nOut10k = pTR->base.outBill.n10k;
			}
			else if( nBusDvs == SVR_DVS_TMEXP )
			{
				vector<TCK_PRINT_FMT_T>::iterator iter;
				CPubTckTmExpMem* pTR;

				pTR = CPubTckTmExpMem::GetInstance();

				///> "A":구매,현금자진발급, "B":구매,현금영수증, "C":구매,신용카드, "8":환불,현금, "9":환불,카드, "Z":환불,기타
				trInfo.TrDvs[0] = pTR->base.ui_pym_dvs_cd[0];	// 20221124 DEL // 결제구분코드
				//trInfo.TrDvs[0] = pTR->base.pyn_mns_dvs_cd[0];	// 20221124 MOD // 지불수단구분코드
				///> 거래금액
				trInfo.nFare = pTR->base.nTotalMoney;
				///> 발권수량
				trInfo.nCount = pTR->m_vtPrtTicket.size();

				///> 승차권번호, 예시 -> ["20190621-999-16-0009-25", "20190621-1000-16-0009-26"]
				trInfo.szTicketInfo[nOffset++] = '[';
				for(iter = pTR->m_vtPrtTicket.begin(); iter != pTR->m_vtPrtTicket.end(); iter++, i++)
				{
					// 					nOffset += sprintf(&trInfo.szTicketInfo[nOffset], "\"%s-%s-%s-%s-%s-%s\"", 
					// 						iter->qr_pub_dt, 
					// 						iter->qr_pub_trml_cd, 
					// 						iter->qr_pub_wnd_no, 
					// 						iter->qr_depr_trml_no, 
					// 						iter->qr_arvl_trml_no, 
					// 						iter->qr_sats_no);
					nOffset += sprintf(&trInfo.szTicketInfo[nOffset], "\"%s-%s-%s-%s\"", 
						iter->qr_pub_dt, 
						iter->qr_pub_trml_cd, 
						iter->qr_pub_wnd_no, 
						iter->qr_pub_sno); 
					if( i == (trInfo.nCount - 1) )
					{
						trInfo.szTicketInfo[nOffset++] = ']';
					}
					else
					{
						trInfo.szTicketInfo[nOffset++] = ',';
					}
					TR_LOG_OUT("거래 데이타 - (%02d) 승차권 데이타 = %s", i, trInfo.szTicketInfo);
				}
				///> 총 투입금액
				trInfo.nTotInsMoney = (pTR->base.insBill.n1k * 1000) + (pTR->base.insBill.n5k * 5000) + (pTR->base.insBill.n10k * 10000) + (pTR->base.insBill.n50k * 50000);
				///> 투입수량 1,000
				trInfo.nIns1k = pTR->base.insBill.n1k;
				///> 투입수량 5,000
				trInfo.nIns5k = pTR->base.insBill.n5k;
				///> 투입수량 10,000
				trInfo.nIns10k = pTR->base.insBill.n10k;
				///> 투입수량 50,000
				trInfo.nIns50k = pTR->base.insBill.n50k;

				///> 총 방출금액
				trInfo.nTotOutMoney = (pTR->base.outCoin.n100 * 100) + (pTR->base.outCoin.n500 * 500) + (pTR->base.outBill.n1k * 1000) + (pTR->base.outBill.n10k * 10000);
				///> 방출수량 100
				trInfo.nOut100 = pTR->base.outCoin.n100;
				///> 방출수량 500
				trInfo.nOut500 = pTR->base.outCoin.n500;
				///> 방출수량 1,000
				trInfo.nOut1k = pTR->base.outBill.n1k;
				///> 방출수량 5,000
				trInfo.nOut5k = 0;
				///> 방출수량 10,000
				trInfo.nOut10k = pTR->base.outBill.n10k;
			}
		}
		else 
		{	/// 환불

		}

		// log
		{
			TR_LOG_OUT("%s", "############## start \n\n");

			TR_LOG_OUT("%30s - (%.*s)"	, "BusDvs		", sizeof(trInfo.BusDvs			), trInfo.BusDvs		);
			TR_LOG_OUT("%30s - (%.*s)"	, "szHappenDt	", sizeof(trInfo.szHappenDt		), trInfo.szHappenDt	);
			TR_LOG_OUT("%30s - (%.*s)"	, "TrDvs		", sizeof(trInfo.TrDvs			), trInfo.TrDvs			);
			TR_LOG_OUT("%30s - (%d)"	, "nFare		", trInfo.nFare			);
			TR_LOG_OUT("%30s - (%d)"	, "nCount		", trInfo.nCount		);
			TR_LOG_OUT("%30s - (%.*s)"	, "szTicketInfo	", sizeof(trInfo.szTicketInfo	), trInfo.szTicketInfo);

			TR_LOG_OUT("%30s - (%d)"	, "nTotInsMoney	", trInfo.nTotInsMoney	);
			TR_LOG_OUT("%30s - (%d)"	, "nIns1k		", trInfo.nIns1k		);
			TR_LOG_OUT("%30s - (%d)"	, "nIns5k		", trInfo.nIns5k		);
			TR_LOG_OUT("%30s - (%d)"	, "nIns10k		", trInfo.nIns10k		);
			TR_LOG_OUT("%30s - (%d)"	, "nIns50k		", trInfo.nIns50k		);

			TR_LOG_OUT("%30s - (%d)"	, "nTotOutMoney	", trInfo.nTotOutMoney	);
			TR_LOG_OUT("%30s - (%d)"	, "nOut100		", trInfo.nOut100		);
			TR_LOG_OUT("%30s - (%d)"	, "nOut500		", trInfo.nOut500		);
			TR_LOG_OUT("%30s - (%d)"	, "nOut1k		", trInfo.nOut1k		);
			TR_LOG_OUT("%30s - (%d)"	, "nOut5k		", trInfo.nOut5k		);
			TR_LOG_OUT("%30s - (%d)"	, "nOut10k		", trInfo.nOut10k		);

			TR_LOG_OUT("%s", "############## end \n\n");
		}

		nRet = s_pclsSmsTR->AddData((char *)&trInfo, (int)sizeof(SMS_TR_DATA_T));
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}


/**
 * @brief		InitDataMng
 * @details		데이타 매니저 초기화
 * @param		None
 * @return		항상 : 0
 */
int InitDataMng(void)
{
	OperInitFile();

	//CConfigTkMem::GetInstance()->ReadKobusCorpInfo("g:\\ko_corp.txt");

	InitConfigFile();
	InitAccumLogFile();
	InitAccumData();

	InitCashTrData();
	InitCashCloseData();
	InitSmsTRData();

	/// 2021.03.25 add code
	InitDayCloseData();
	/// ~2021.03.25 add code

	return 0;
}

/**
 * @brief		TermDataMng
 * @details		데이타 매니저 종료
 * @param		None
 * @return		항상 : 0
 */
int TermDataMng(void)
{
	TermConfigFile();
	TermAccumData();

	TermCashTrData();
	TermCashCloseData();
	
	/// 2021.03.25 add code
	TermDayCloseData();
	/// ~2021.03.25 add code

	TermSmsTRData();

	return 0;
}

BOOL DB_AddBusMsgData(char *pData)
{
	/***
	CDbSQLite	m_DB;
	BOOL bRet;
	CString strQuery;
//	CSqlStatement* pStmt;
	rtk_readcbusmsg_list_t* pList;

	pList = (rtk_readcbusmsg_list_t*) pData;

	bRet = m_DB.Open(_T("d:\\atec_tvm.db"));
	if(bRet == FALSE)
	{
		AfxMessageBox(_T("Error"));
		return bRet;
	}

	//char ansiStr[] = "발매기 발행세부정보 조회 오류입니다.";
	char szUtf8String[256] = {0,};
// 	char szUTF8_eng[256] = {0,};
// 	char szUTF8_jpn[256] = {0,};
// 	char szUTF8_cha[256] = {0,};

	Util_Ansi2UTF8(pList->cty_bus_msg_nm, szUtf8String);

//	strQuery.Format(" insert into tbl_readcbusmsg values('%s', '%s', '%s', '%s', '%s'); ", "SVE513", szUTF8, "Issue details inquiry error.", "Issue details inquiry error.", "Issue details inquiry error.");
	strQuery.Format(" insert into tbl_readcbusmsg values('%s', '%s', '%s'); ", pList->cty_bus_msg_cd, szUtf8String, pList->lng_dvs_cd);
	bRet = m_DB.DirectStatement(strQuery);
	if(bRet == TRUE)
	{
		TRACE("Insert Success \n");
		//AfxMessageBox(_T("Insert Success"));
	}
	else
	{
		TRACE("Insert Failure \n");
		//AfxMessageBox(_T("Insert Failure"));
	}
	return bRet;
	***/
	return FALSE;
}


