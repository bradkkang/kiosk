
#include "stdafx.h"
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <direct.h>
#include <fstream>
#include <vector>
#include <algorithm>

#include "MyFileUtil.h"
#include "MyUtil.h"
#include "MyDataAccum.h"
#include "dev_tr_main.h"
#include "event_if.h"
#include "oper_kos_file.h"
#include "cmn_util.h"

#define _ACCUM_DEBUG_	1

//----------------------------------------------------------------------------------------------------------------------

using namespace std ;

typedef vector<ACC_FILE_ENTRY_T> FILE_ACC_VECTOR;
typedef FILE_ACC_VECTOR::iterator itrFile;
typedef FILE_ACC_VECTOR::reverse_iterator ritrFile;

//----------------------------------------------------------------------------------------------------------------------

static HANDLE			hAccMutex;
static FILE_ACC_VECTOR	s_FileVector;
/// 2021.03.26 add code
static FILE_ACCUM_T			s_tAccumOld;
static FILE_ACCUM_N1010_T	s_tAccum;
/// 2021.03.26 add code
static char				accum_buf[1024*10];
static CString			s_strPath;
static int				s_nDebug = 1;
static CAccumLogFile	m_clsLog;

//----------------------------------------------------------------------------------------------------------------------

#define LOG_OUT(fmt, ...)		{ m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ m_clsLog.HexaDump(x, y, z); }

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		LOG_INIT
 * @details		LOG 파일 초기화
 * @param		None
 * @return		None
 */
static void LOG_INIT(void)
{
	m_clsLog.SetData(30, "\\Log\\Data\\Acc");
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		SortFunction
 * @details		데이타 정렬
 * @param		None
 * @return		TRUE = 내림차순, FALSE = 오름차순
 */
static BOOL SortFunction(ACC_FILE_ENTRY_T a, ACC_FILE_ENTRY_T b)
{
	if(a.dwSerial <= b.dwSerial) 
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
static int Locking(void)
{
	if(hAccMutex != 0) 
	{
		::WaitForSingleObject(hAccMutex, 3000);
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
	if(hAccMutex != 0) 
	{
		::ReleaseMutex(hAccMutex);
	}

	return 0;	
}

/**
 * @brief		DefaultAccumData
 * @details		기본값 설정
 * @param		None
 * @return		항상 : 0
 */
static int DefaultAccumData(void)
{
	/// 2021.03.26 add code
	//::ZeroMemory(&s_tAccum, sizeof(FILE_ACCUM_T));
	::ZeroMemory(&s_tAccum, sizeof(FILE_ACCUM_N1010_T));
	/// ~2021.03.26 add code

	if(s_tAccum.Curr.ccsTicket.n_bus_tck_inhr_no <= 0)
		s_tAccum.Curr.ccsTicket.n_bus_tck_inhr_no = 1;

	if(s_tAccum.Curr.expTicket.n_bus_tck_inhr_no <= 0)
		s_tAccum.Curr.expTicket.n_bus_tck_inhr_no = 1;

//	s_tAccum.tAnyCloseInfo.dwOpenTick = Util_GetCurrentTick();
//	s_tAccum.tDayCloseInfo.dwOpenTick = Util_GetCurrentTick();

	//g_Accum.stPrintCnt = 1;

	return 0;
}

/**
 * @brief		PrintAccumData
 * @details		회계 데이타 정보 로그 남기기
 * @param		None
 * @return		None
 */
void PrintAccumData(BOOL bPrint)
{
	int i;
	PDM_COIN_T				pCoin;
	PDM_BILL_T				pBill;
	PDM_BILL_DISPENSER_T	pCDU;
//	PDM_TICKET_T			pTicket;
	PDM_CURRENT_T			pCurr;
	PDM_CURRENT_T			pPrev;
	PDM_CURRENT_T			pInfo;
	PDM_ADMIN_CLOST_T		pClose;

	pPrev = &s_tAccum.Prev;
	pCurr = &s_tAccum.Curr;

	pClose = &s_tAccum.Close;

	if(bPrint == FALSE)
	{
		return;
	}

	LOG_OUT("start =================================================================\n");
	for(i = 0; i < 2; i++)
	{
		if(i == 0)
		{
			LOG_OUT("******** [ 직전 시재 정보 ].");
			pInfo = pPrev;
		}
		else
		{
			LOG_OUT("******** [ 현재 시재 정보 ].");
			pInfo = pCurr;
		}

		pCoin = &pInfo->Coin;
		pBill = &pInfo->BillBox;
		pCDU = &pInfo->BillDispenser;

		{
			LOG_OUT("\t동전호퍼   = (10원 = %4d, 50원 = %4d, 100원 = %4d, 500원 = %4d) ..", pCoin->n10, pCoin->n50, pCoin->n100, pCoin->n500);
			LOG_OUT("\t지폐수집함 = (1k   = %4d, 5k = %4d, 10k = %4d, 50k = %4d) .."		 , pBill->n1k, pBill->n5k, pBill->n10k, pBill->n50k);
			LOG_OUT("\t지폐방출기 = (1k   = %4d, 10k = %4d) .."							 , pCDU->Casst.n1k, pCDU->Casst.n10k);
			LOG_OUT("\t지폐방출기_폐표함 = (1k = %4d, 10k = %4d) .."						 , pCDU->ErrCasst.n1k, pCDU->ErrCasst.n10k);
		}

		{
			PDM_TICKET_T			pTckCCS;
			PDM_TICKET_T			pTckEXP;

			LOG_OUT("******** [ 승차권 정보 ].");
			pTckCCS = &pInfo->ccsTicket;
			pTckEXP = &pInfo->expTicket;

			LOG_OUT("\t시외_승차권    = (%4d 매), 고속_승차권 = (%4d 매) ..", pTckCCS->nCount, pTckEXP->nCount);
			LOG_OUT("\t승차권 잔여수량 = (%4d 매) ..", pCurr->phyTicket.nCount);
		}

		LOG_OUT("\t---------------------------------------------");
	}
	LOG_OUT("\t---------------------------------------------");

	if(1)
	{
		LOG_OUT("******** [마감 정보].");
		LOG_OUT("\t초기정보   = (10원 = %4d, 50원 = %4d, 100원 = %4d, 500원 = %4d), (1k = %4d, 5k = %4d, 10k = %4d, 50k = %4d) ..", 
			pClose->initCoin.n10, pClose->initCoin.n50, pClose->initCoin.n100, pClose->initCoin.n500,
			pClose->initBill.n1k, pClose->initBill.n5k, pClose->initBill.n10k, pClose->initBill.n50k);
		LOG_OUT("\t보급정보   = (10원 = %4d, 50원 = %4d, 100원 = %4d, 500원 = %4d), (1k = %4d, 5k = %4d, 10k = %4d, 50k = %4d) ..", 
			pClose->supplyCoin.n10, pClose->supplyCoin.n50, pClose->supplyCoin.n100, pClose->supplyCoin.n500,
			pClose->supplyBill.n1k, pClose->supplyBill.n5k, pClose->supplyBill.n10k, pClose->supplyBill.n50k);
		LOG_OUT("\t투입정보   = (1k   = %4d, 5k   = %4d, 10k   = %4d, 50k   = %4d) ..", 
			pClose->inBill.n1k, pClose->inBill.n5k, pClose->inBill.n10k, pClose->inBill.n50k);
		LOG_OUT("\t방출정보   = (10원 = %4d, 50원 = %4d, 100원 = %4d, 500원 = %4d), (1k = %4d, 5k = %4d, 10k = %4d, 50k = %4d) ..", 
			pClose->outCoin.n10, pClose->outCoin.n50, pClose->outCoin.n100, pClose->outCoin.n500,
			pClose->outBill.n1k, pClose->outBill.n5k, pClose->outBill.n10k, pClose->outBill.n50k);
		LOG_OUT("\t회수정보   = (1k   = %4d, 5k   = %4d, 10k   = %4d, 50k   = %4d) ..", 
			pClose->wdrawBill.n1k, pClose->wdrawBill.n5k, pClose->wdrawBill.n10k, pClose->wdrawBill.n50k);
		LOG_OUT("\t미방출정보 = (10원 = %4d, 50원 = %4d, 100원 = %4d, 500원 = %4d), (1k = %4d, 5k = %4d, 10k = %4d, 50k = %4d) ..", 
			pClose->noCoin.n10, pClose->noCoin.n50, pClose->noCoin.n100, pClose->noCoin.n500,
			pClose->noBill.n1k, pClose->noBill.n5k, pClose->noBill.n10k, pClose->noBill.n50k);
	}
	LOG_OUT("end =================================================================\n");
}

/**
 * @brief		AddAccumulateData
 * @details		int nKind		데이타 종류
 * @details		char *pData		데이타 
 * @details		int bWrite		데이타 저장 여부
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int AddAccumulateData(int nKind, char *pData, int bWrite)
{
	SYSTEMTIME				sysTime;
	FILE_ACC_HEADER_T		tFileHdr;
	ACC_HEADER_T			accHeader;
	ACC_FILE_ENTRY_T		dEntry;
	int						nSize, nRet, nPayDvs;
	DWORD					dwTick;
	ritrFile				ritr;
	CString					strFileName;
	HANDLE					hFile;

	try
	{
		Locking();

		if(bWrite) 
		{
			::GetLocalTime(&sysTime);

			dwTick = Util_TickFromDate(sysTime.wYear, sysTime.wMonth, sysTime.wDay, 0, 0, 0);

			ritr = s_FileVector.rbegin();

			if(1)
			{
				SYSTEMTIME stFile;

				stFile.wYear  = (WORD) Util_Ascii2Long(&ritr->date[9],  4);
				stFile.wMonth = (WORD) Util_Ascii2Long(&ritr->date[13], 2);
				stFile.wDay   = (WORD) Util_Ascii2Long(&ritr->date[15], 2);

				if((sysTime.wYear  != stFile.wYear) || (sysTime.wMonth != stFile.wMonth) || (sysTime.wDay != stFile.wDay))
				{
					nRet = CreateAccumData();
					if(nRet < 0)
					{
						UnLocking();
						return nRet;
					}
					UnLocking();

					//// => nKind값 오류 발생 //// LOG_OUT("@@@@@@@@@@ Create & AddAccumulateData ... nKind(%n), pData(%s), bWrite(%n) !!!", nKind, pData, bWrite);

					AddAccumulateData(nKind, pData, bWrite);
					return 0;
				}
			}

			strFileName = s_strPath + _T("\\") + (CString) ritr->date;
			nRet = MyAccessFile(strFileName);
			if(nRet < 0)
			{
				dwTick = Util_TickFromDate(sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
				/// 2021.03.26 add code
				tFileHdr.dwVersion = CHG_ACCUM_VERSION;
				/// ~2021.03.26 add code
				tFileHdr.dwMagic = ACCUM_MAGIC_CODE;
				tFileHdr.dwTick = dwTick;
				tFileHdr.dwSerial = ritr->dwSerial + 1;
				tFileHdr.dwCount = 0;
				tFileHdr.dwSend = 0;

				hFile = MyOpenFile(strFileName, TRUE);
				if(hFile == INVALID_HANDLE_VALUE)
				{
					UnLocking();
					return -1;
				}

				MySetFilePointer(hFile, 0, FILE_END);
				nRet = MyWriteFile(hFile, &tFileHdr, (int)sizeof(FILE_ACC_HEADER_T));
				if(nRet < 0)
				{
					;
				}

				accHeader.dwTick = dwTick;
				accHeader.nKind = ACC_ALL;

				/// 2021.03.26 add code
				//accHeader.nSize = sizeof(FILE_ACCUM_T);
				accHeader.nSize = sizeof(FILE_ACCUM_N1010_T);
				/// ~2021.03.26 add code

				nRet = MyWriteFile(hFile, &accHeader, (int)sizeof(ACC_HEADER_T));
				if(nRet < 0)
				{
					;
				}

				/// 2021.03.26 add code
				//nRet = MyWriteFile(hFile, &s_tAccum, (int)sizeof(FILE_ACCUM_T));
				nRet = MyWriteFile(hFile, &s_tAccum, (int)sizeof(FILE_ACCUM_N1010_T));
				/// ~2021.03.26 add code
				if(nRet < 0)
				{
					;
				}

				MyCloseFile(hFile);

				memset(&dEntry, 0, sizeof(ACC_FILE_ENTRY_T)); 

				/// 2021.03.26 add code
				dEntry.dwVersion = CHG_ACCUM_VERSION;
				/// ~2021.03.26 add code

				sprintf(dEntry.date, "%08u-%04d%02d%02d", tFileHdr.dwSerial, sysTime.wYear, sysTime.wMonth, sysTime.wDay);
				dEntry.dwTick = dwTick;
				dEntry.dwSerial = tFileHdr.dwSerial;
				dEntry.dwCount = 0;
				dEntry.dwSend = 0;
				s_FileVector.push_back(dEntry);
			}
		}

		nSize = 0;

#if (_ACCUM_DEBUG_ > 0)
		{
			SYSTEMTIME *pST;
			PACC_COIN_DATA_T	pAcc;
			
			LOG_OUT("START ###########################################################################\n");
			pAcc = (PACC_COIN_DATA_T) pData;
			pST = (SYSTEMTIME *) Util_DateFromTick(pAcc->Base.dwTick);
			LOG_OUT("%30s = nKind(%d, 0x%08lx), 일시(%04d-%02d-%02d %02d:%02d:%02d) ..", 
						"[ACCUM]", nKind, nKind, 
						pST->wYear, pST->wMonth, pST->wDay, pST->wHour, pST->wMinute, pST->wSecond);
		}
#endif

		switch(nKind) 
		{
		case ACC_ALL:
			UnLocking();
			return 0;

		case ACC_COIN_SUPPLY:
			{ // 동전방출기 - 동전보급
				PACC_COIN_DATA_T	pAcc;

				nSize = sizeof(ACC_COIN_DATA_T);
				pAcc = (PACC_COIN_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[동전보급_param]", pAcc->Coin.n10, pAcc->Coin.n50, pAcc->Coin.n100, pAcc->Coin.n500);
#endif		
				// 현재 데이타
				s_tAccum.Curr.Coin.n10 += pAcc->Coin.n10;
				s_tAccum.Curr.Coin.n50 += pAcc->Coin.n50;
				s_tAccum.Curr.Coin.n100 += pAcc->Coin.n100;
				s_tAccum.Curr.Coin.n500 += pAcc->Coin.n500;
#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[동전회계]", s_tAccum.Curr.Coin.n10, s_tAccum.Curr.Coin.n50, s_tAccum.Curr.Coin.n100, s_tAccum.Curr.Coin.n500);
#endif		

				// 마감 데이타
				s_tAccum.Close.supplyCoin.n10 += pAcc->Coin.n10;
				s_tAccum.Close.supplyCoin.n50 += pAcc->Coin.n50;
				s_tAccum.Close.supplyCoin.n100 += pAcc->Coin.n100;
				s_tAccum.Close.supplyCoin.n500 += pAcc->Coin.n500;
			}
			break;

		case ACC_COIN_MINUS:
			{ // 동전방출기 - 동전방출
				PACC_COIN_DATA_T	pAcc;

				nSize = sizeof(ACC_COIN_DATA_T);
				pAcc = (PACC_COIN_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[동전방출]", pAcc->Coin.n10, pAcc->Coin.n50, pAcc->Coin.n100, pAcc->Coin.n500);
#endif
				// 현재 데이타
				s_tAccum.Curr.Coin.n10 -= pAcc->Coin.n10;
				if(s_tAccum.Curr.Coin.n10 < 0)
				{
					s_tAccum.Curr.Coin.n10 = 0;
				}

				s_tAccum.Curr.Coin.n50 -= pAcc->Coin.n50;
				if(s_tAccum.Curr.Coin.n50 < 0)
				{
					s_tAccum.Curr.Coin.n50 = 0;
				}

				s_tAccum.Curr.Coin.n100 -= pAcc->Coin.n100;
				if(s_tAccum.Curr.Coin.n100 < 0)
				{
					s_tAccum.Curr.Coin.n100 = 0;
				}

				s_tAccum.Curr.Coin.n500 -= pAcc->Coin.n500;
				if(s_tAccum.Curr.Coin.n500 < 0)
				{
					s_tAccum.Curr.Coin.n500 = 0;
				}
#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[동전방출_회계]", s_tAccum.Curr.Coin.n10, s_tAccum.Curr.Coin.n50, s_tAccum.Curr.Coin.n100, s_tAccum.Curr.Coin.n500);
				{
					DM_BILL_DISPENSER_T *pBill;

					pBill = &s_tAccum.Curr.BillDispenser;
					LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[지폐방출_회계]", pBill->Casst.n1k, pBill->Casst.n5k, pBill->Casst.n10k, pBill->Casst.n50k);
				}

#endif

				// 마감 데이타
				s_tAccum.Close.outCoin.n10 += pAcc->Coin.n10;
				if(s_tAccum.Close.outCoin.n10 < 0)
				{
					s_tAccum.Close.outCoin.n10 = 0;
				}
				s_tAccum.Close.outCoin.n50 += pAcc->Coin.n50;
				if(s_tAccum.Close.outCoin.n50 < 0)
				{
					s_tAccum.Close.outCoin.n50 = 0;
				}
				s_tAccum.Close.outCoin.n100 += pAcc->Coin.n100;
				if(s_tAccum.Close.outCoin.n100 < 0)
				{
					s_tAccum.Close.outCoin.n100 = 0;
				}
				s_tAccum.Close.outCoin.n500 += pAcc->Coin.n500;
				if(s_tAccum.Close.outCoin.n500 < 0)
				{
					s_tAccum.Close.outCoin.n500 = 0;
				}
			}
			break;

		case ACC_COIN_ASSIGN:
			{	// 동전방출기 - Assign
				PACC_COIN_DATA_T	pAcc;

				nSize = sizeof(ACC_COIN_DATA_T);
				pAcc = (PACC_COIN_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[동전데이타 맞춤]", pAcc->Coin.n10, pAcc->Coin.n50, pAcc->Coin.n100, pAcc->Coin.n500);
#endif
				s_tAccum.Curr.Coin.n10 = pAcc->Coin.n10;
				s_tAccum.Curr.Coin.n50 = pAcc->Coin.n50;
				s_tAccum.Curr.Coin.n100 = pAcc->Coin.n100;
				s_tAccum.Curr.Coin.n500 = pAcc->Coin.n500;
			}
			break;
		case ACC_COIN_CLEAR:
			{	// 동전방출기
				PACC_COIN_DATA_T	pAcc;

				nSize = sizeof(ACC_COIN_DATA_T);
				pAcc = (PACC_COIN_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[동전데이타 Clear]", pAcc->Coin.n10, pAcc->Coin.n50, pAcc->Coin.n100, pAcc->Coin.n500);
#endif
				::ZeroMemory(&s_tAccum.Curr.Coin, sizeof(DM_COIN_T));
			}
			break;

		case ACC_BILL_INSERT:
			{	// 지폐함 - 투입
				PACC_BILL_DATA_T	pAcc;

				nSize = sizeof(ACC_BILL_DATA_T);
				pAcc = (PACC_BILL_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[지폐투입_param]", pAcc->Bill.n1k, pAcc->Bill.n5k, pAcc->Bill.n10k, pAcc->Bill.n50k);
#endif

				s_tAccum.Curr.BillBox.n1k	+= pAcc->Bill.n1k;
				s_tAccum.Curr.BillBox.n5k	+= pAcc->Bill.n5k;
				s_tAccum.Curr.BillBox.n10k  += pAcc->Bill.n10k;
				s_tAccum.Curr.BillBox.n50k	+= pAcc->Bill.n50k;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[지폐수집함]", s_tAccum.Curr.BillBox.n1k, s_tAccum.Curr.BillBox.n5k, s_tAccum.Curr.BillBox.n10k, s_tAccum.Curr.BillBox.n50k);
#endif

				// 마감 데이타
				s_tAccum.Close.inBill.n1k	+= pAcc->Bill.n1k;
				s_tAccum.Close.inBill.n5k	+= pAcc->Bill.n5k;
				s_tAccum.Close.inBill.n10k	+= pAcc->Bill.n10k;
				s_tAccum.Close.inBill.n50k	+= pAcc->Bill.n50k;
			}
			break;
		case ACC_BILL_MINUS:
			{	// 지폐함 - minus
				PACC_BILL_DATA_T	pAcc;
				DM_BILL_DISPENSER_T *pBill;

				nSize = sizeof(ACC_BILL_DATA_T);
				pAcc = (PACC_BILL_DATA_T) pData;
				pBill = &s_tAccum.Curr.BillDispenser;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d, %d, %d, %d), acc(%d, %d, %d, %d) ..", "[지폐방출]", pAcc->Bill.n1k, pAcc->Bill.n5k, pAcc->Bill.n10k, pAcc->Bill.n50k,
						pBill->Casst.n1k, pBill->Casst.n5k, pBill->Casst.n10k, pBill->Casst.n50k);
#endif
				s_tAccum.Curr.BillDispenser.Casst.n1k -= pAcc->Bill.n1k;
				if(s_tAccum.Curr.BillDispenser.Casst.n1k < 0)
				{
					s_tAccum.Curr.BillDispenser.Casst.n1k = 0;
				}
				s_tAccum.Curr.BillDispenser.Casst.n5k -= pAcc->Bill.n5k;
				if(s_tAccum.Curr.BillDispenser.Casst.n5k < 0)
				{
					s_tAccum.Curr.BillDispenser.Casst.n5k = 0;
				}
				s_tAccum.Curr.BillDispenser.Casst.n10k -= pAcc->Bill.n10k;
				if(s_tAccum.Curr.BillDispenser.Casst.n10k < 0)
				{
					s_tAccum.Curr.BillDispenser.Casst.n10k = 0;
				}
				s_tAccum.Curr.BillDispenser.Casst.n50k -= pAcc->Bill.n50k;
				if(s_tAccum.Curr.BillDispenser.Casst.n50k < 0)
				{
					s_tAccum.Curr.BillDispenser.Casst.n50k = 0;
				}

#if (_ACCUM_DEBUG_ > 0)
				{
					LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[동전방출_회계]", s_tAccum.Curr.Coin.n10, s_tAccum.Curr.Coin.n50, s_tAccum.Curr.Coin.n100, s_tAccum.Curr.Coin.n500);
					pBill = &s_tAccum.Curr.BillDispenser;
					LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[지폐방출_회계]", pBill->Casst.n1k, pBill->Casst.n5k, pBill->Casst.n10k, pBill->Casst.n50k);
				}
#endif

				// 마감 데이타
				s_tAccum.Close.outBill.n1k += pAcc->Bill.n1k;
				if(s_tAccum.Close.outBill.n1k < 0)
				{
					s_tAccum.Close.outBill.n1k = 0;
				}
				s_tAccum.Close.outBill.n5k += pAcc->Bill.n5k;
				if(s_tAccum.Close.outBill.n5k < 0)
				{
					s_tAccum.Close.outBill.n5k = 0;
				}
				s_tAccum.Close.outBill.n10k += pAcc->Bill.n10k;
				if(s_tAccum.Close.outBill.n10k < 0)
				{
					s_tAccum.Close.outBill.n10k = 0;
				}
				s_tAccum.Close.outBill.n50k += pAcc->Bill.n50k;
				if(s_tAccum.Close.outBill.n50k < 0)
				{
					s_tAccum.Close.outBill.n50k = 0;
				}
			}
			break;
		case ACC_BILL_WITHDRAW:
			{	// 지폐함 - 회수
				PACC_BILL_DATA_T	pAcc;

				nSize = sizeof(ACC_BILL_DATA_T);
				pAcc = (PACC_BILL_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[지폐함회수]", pAcc->Bill.n1k, pAcc->Bill.n5k, pAcc->Bill.n10k, pAcc->Bill.n50k);
#endif
				/// 현재 데이타
				s_tAccum.Curr.BillBox.n1k = 0;
				s_tAccum.Curr.BillBox.n5k = 0;
				s_tAccum.Curr.BillBox.n10k = 0;
				s_tAccum.Curr.BillBox.n50k = 0;

				/// 마감 데이타
				s_tAccum.Close.wdrawBill.n1k += pAcc->Bill.n1k;
				if(s_tAccum.Close.wdrawBill.n1k < 0)
				{
					s_tAccum.Close.wdrawBill.n1k = 0;
				}
				s_tAccum.Close.wdrawBill.n5k += pAcc->Bill.n5k;
				if(s_tAccum.Close.wdrawBill.n5k < 0)
				{
					s_tAccum.Close.wdrawBill.n5k = 0;
				}
				s_tAccum.Close.wdrawBill.n10k += pAcc->Bill.n10k;
				if(s_tAccum.Close.wdrawBill.n10k < 0)
				{
					s_tAccum.Close.wdrawBill.n10k = 0;
				}
				s_tAccum.Close.wdrawBill.n50k += pAcc->Bill.n50k;
				if(s_tAccum.Close.wdrawBill.n50k < 0)
				{
					s_tAccum.Close.wdrawBill.n50k = 0;
				}

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = wdrawbill(%d, %d, %d, %d) ..", "[지폐함회수]", s_tAccum.Close.wdrawBill.n1k, s_tAccum.Close.wdrawBill.n5k, s_tAccum.Close.wdrawBill.n10k, s_tAccum.Close.wdrawBill.n50k);
#endif
			}
			break;
		case ACC_BILL_ASSIGN :
			{
				PACC_BILL_DATA_T	pAcc;

				nSize = sizeof(ACC_BILL_DATA_T);
				pAcc = (PACC_BILL_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[지폐함_맞추기]", pAcc->Bill.n1k, pAcc->Bill.n5k, pAcc->Bill.n10k, pAcc->Bill.n50k);
#endif
				/// 현재 데이타
				s_tAccum.Curr.BillBox.n1k	= pAcc->Bill.n1k;
				s_tAccum.Curr.BillBox.n5k	= pAcc->Bill.n5k;
				s_tAccum.Curr.BillBox.n10k	= pAcc->Bill.n10k;
				s_tAccum.Curr.BillBox.n50k	= pAcc->Bill.n50k;

				/// 마감 데이타
				s_tAccum.Close.inBill.n1k = pAcc->Bill.n1k;
				s_tAccum.Close.inBill.n5k = pAcc->Bill.n5k;
				s_tAccum.Close.inBill.n10k = pAcc->Bill.n10k;
				s_tAccum.Close.inBill.n50k = pAcc->Bill.n50k;
			}
			break;
		case ACC_BILL_CLEAR:
			{	// 지폐방출기 - 초기화
				PACC_BILL_DATA_T	pAcc;

				nSize = sizeof(ACC_BILL_DATA_T);
				pAcc = (PACC_BILL_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[지폐방출기_Clear]", pAcc->Bill.n1k, pAcc->Bill.n5k, pAcc->Bill.n10k, pAcc->Bill.n50k);
#endif
				::ZeroMemory(&s_tAccum.Curr.BillDispenser, sizeof(DM_BILL_DISPENSER_T));
			}
			break;

		case ACC_BILL_SUPPLY:
			{	// 지폐방출기 - 지폐보급
				PACC_BILL_DATA_T	pAcc;

				nSize = sizeof(ACC_BILL_DATA_T);
				pAcc = (PACC_BILL_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[지폐보급]", pAcc->Bill.n1k, pAcc->Bill.n5k, pAcc->Bill.n10k, pAcc->Bill.n50k);
#endif
				s_tAccum.Curr.BillDispenser.Casst.n1k += pAcc->Bill.n1k;
				s_tAccum.Curr.BillDispenser.Casst.n5k += pAcc->Bill.n5k;
				s_tAccum.Curr.BillDispenser.Casst.n10k += pAcc->Bill.n10k;
				s_tAccum.Curr.BillDispenser.Casst.n50k += pAcc->Bill.n50k;

				// 마감 데이타
				s_tAccum.Close.supplyBill.n1k += pAcc->Bill.n1k;
				s_tAccum.Close.supplyBill.n5k += pAcc->Bill.n5k;
				s_tAccum.Close.supplyBill.n10k += pAcc->Bill.n10k;
				s_tAccum.Close.supplyBill.n50k += pAcc->Bill.n50k;
			}
			break;
		case ACC_BILL_DISPENSER_ASSIGN:
			{	// 지폐방출기 - 지폐 데이타 맞추기
				PACC_BILL_DATA_T	pAcc;

				nSize = sizeof(ACC_BILL_DATA_T);
				pAcc = (PACC_BILL_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[지폐데이타_맞추기]", pAcc->Bill.n1k, pAcc->Bill.n5k, pAcc->Bill.n10k, pAcc->Bill.n50k);
#endif
				s_tAccum.Curr.BillDispenser.Casst.n1k = pAcc->Bill.n1k;
				s_tAccum.Curr.BillDispenser.Casst.n5k = pAcc->Bill.n5k;
				s_tAccum.Curr.BillDispenser.Casst.n10k = pAcc->Bill.n10k;
				s_tAccum.Curr.BillDispenser.Casst.n50k = pAcc->Bill.n50k;

				s_tAccum.Curr.BillDispenser.ErrCasst.n1k = pAcc->Bill.n1k;
				s_tAccum.Curr.BillDispenser.ErrCasst.n5k = pAcc->Bill.n5k;
				s_tAccum.Curr.BillDispenser.ErrCasst.n10k = pAcc->Bill.n10k;
				s_tAccum.Curr.BillDispenser.ErrCasst.n50k = pAcc->Bill.n50k;
			}
			break;
		case ACC_BILL_DISPENSER_CLEAR:
			{	// 지폐방출기 - 지폐 Clear
				PACC_BILL_DATA_T	pAcc;

				nSize = sizeof(ACC_BILL_DATA_T);
				pAcc = (PACC_BILL_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d, %d, %d, %d) ..", "[지폐방출기_Clear]", pAcc->Bill.n1k, pAcc->Bill.n5k, pAcc->Bill.n10k, pAcc->Bill.n50k);
#endif
				::ZeroMemory(&s_tAccum.Curr.BillDispenser, sizeof(DM_BILL_DISPENSER_T));
			}
			break;

		case ACC_TICKET_PUB_ISSUE :
			{	// 현장발권
				TCK_KIND_LIST_T		tckList;
				PACC_TICKET_WORK_T	pAcc;

				::ZeroMemory(&tckList, sizeof(TCK_KIND_LIST_T));

				nSize = sizeof(ACC_TICKET_WORK_T);
				pAcc = (PACC_TICKET_WORK_T) pData;

				nPayDvs = (pAcc->nPayDvs == 0) ? IDX_ACC_CARD : IDX_ACC_CASH;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d:%d:%d:%d:%s) ..", "현장발권", pAcc->nSvrKind,  pAcc->nPayDvs, pAcc->nCount, pAcc->nFare, pAcc->bus_tck_knd_cd);
#endif
				nRet = FindTicketKindData(pAcc->nSvrKind, pAcc->bus_tck_knd_cd, (char *)&tckList);
				if(nRet >= 0)
				{					   
					if(pAcc->nSvrKind == SVR_DVS_CCBUS)
					{
						/// 현장발권 매수
						s_tAccum.Curr.ccsTicketWork[nRet].tPubTck[nPayDvs].nCount += pAcc->nCount;
						/// 현장발권 금액
						s_tAccum.Curr.ccsTicketWork[nRet].tPubTck[nPayDvs].dwMoney += pAcc->nFare;
					
						/// (수시마감) 현장발권 매수
						s_tAccum.MnuCls.ccsTicketWork[nRet].tPubTck[nPayDvs].nCount += pAcc->nCount;
						/// (수시마감) 현장발권 금액
						s_tAccum.MnuCls.ccsTicketWork[nRet].tPubTck[nPayDvs].dwMoney += pAcc->nFare;
					}

					if(pAcc->nSvrKind & (SVR_DVS_KOBUS | SVR_DVS_TMEXP))
					{
						/// 현장발권 매수
						s_tAccum.Curr.expTicketWork[nRet].tPubTck[nPayDvs].nCount += pAcc->nCount;
						/// 현장발권 금액
						s_tAccum.Curr.expTicketWork[nRet].tPubTck[nPayDvs].dwMoney += pAcc->nFare;

						/// (수시마감) 현장발권 매수
						s_tAccum.MnuCls.expTicketWork[nRet].tPubTck[nPayDvs].nCount += pAcc->nCount;
						/// (수시마감) 현장발권 금액
						s_tAccum.MnuCls.expTicketWork[nRet].tPubTck[nPayDvs].dwMoney += pAcc->nFare;
					}

#if (_ACCUM_DEBUG_ > 0)
					LOG_OUT("%30s = Index(%d), 티켓종류(%s), ccs(%d:%d), exp(%d:%d) ..", "현장발권", 
						nRet, tckList.tck_kind,
						s_tAccum.Curr.ccsTicketWork[nRet].tPubTck[nPayDvs].nCount, s_tAccum.Curr.ccsTicketWork[nRet].tPubTck[nPayDvs].dwMoney,
						s_tAccum.Curr.expTicketWork[nRet].tPubTck[nPayDvs].nCount, s_tAccum.Curr.expTicketWork[nRet].tPubTck[nPayDvs].dwMoney);
#endif
				}
				else
				{
					LOG_OUT("%s", "ACC_TICKET_PUB_ISSUE, 티켓 종류 에러 @@@@@@@@@@@@");
					LOG_OUT("현장발권 = index(%d), (%d:%d:%d:%d:%s) ..", nRet, pAcc->nSvrKind,  pAcc->nPayDvs, pAcc->nCount, pAcc->nFare, pAcc->bus_tck_knd_cd);
				}

				LOG_OUT("%30s = 티켓수량_bef (%d - %d)", "현장발권", s_tAccum.Curr.phyTicket.nCount, pAcc->nCount);

				/// 승차권 잔량
				s_tAccum.Curr.phyTicket.nCount = SubtractValue(s_tAccum.Curr.phyTicket.nCount, pAcc->nCount);

				LOG_OUT("%30s = 티켓수량_aft (%d)", "현장발권", s_tAccum.Curr.phyTicket.nCount);
			}
			break;
		
		/// 2020.06.03 add code
		case ACC_TICKET_PUB_REV1_ISSUE :
			{	// 현장발권
				TCK_KIND_LIST_T		tckList;
				PACC_TICKET_WORK_REV1_T	pAcc;

				::ZeroMemory(&tckList, sizeof(TCK_KIND_LIST_T));

				nSize = sizeof(ACC_TICKET_WORK_REV1_T);
				pAcc = (PACC_TICKET_WORK_REV1_T) pData;

				nPayDvs = (pAcc->nPayDvs == 0) ? IDX_ACC_CARD : IDX_ACC_CASH;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d:%d:%d:%d:0x%02X:%s) ..", "현장발권", pAcc->nSvrKind,  pAcc->nPayDvs, pAcc->nCount, pAcc->nFare, pAcc->byPaper, pAcc->bus_tck_knd_cd);
#endif
				nRet = FindTicketKindData(pAcc->nSvrKind, pAcc->bus_tck_knd_cd, (char *)&tckList);
				if(nRet >= 0)
				{					   
					if(pAcc->nSvrKind == SVR_DVS_CCBUS)
					{
						/// 현장발권 매수
						s_tAccum.Curr.ccsTicketWork[nRet].tPubTck[nPayDvs].nCount += pAcc->nCount;
						/// 현장발권 금액
						s_tAccum.Curr.ccsTicketWork[nRet].tPubTck[nPayDvs].dwMoney += pAcc->nFare;

						/// (수시마감) 현장발권 매수
						s_tAccum.MnuCls.ccsTicketWork[nRet].tPubTck[nPayDvs].nCount += pAcc->nCount;
						/// (수시마감) 현장발권 금액
						s_tAccum.MnuCls.ccsTicketWork[nRet].tPubTck[nPayDvs].dwMoney += pAcc->nFare;
					}

					if(pAcc->nSvrKind & (SVR_DVS_KOBUS | SVR_DVS_TMEXP))
					{
						/// 현장발권 매수
						s_tAccum.Curr.expTicketWork[nRet].tPubTck[nPayDvs].nCount += pAcc->nCount;
						/// 현장발권 금액
						s_tAccum.Curr.expTicketWork[nRet].tPubTck[nPayDvs].dwMoney += pAcc->nFare;

						/// (수시마감) 현장발권 매수
						s_tAccum.MnuCls.expTicketWork[nRet].tPubTck[nPayDvs].nCount += pAcc->nCount;
						/// (수시마감) 현장발권 금액
						s_tAccum.MnuCls.expTicketWork[nRet].tPubTck[nPayDvs].dwMoney += pAcc->nFare;
					}

#if (_ACCUM_DEBUG_ > 0)
					LOG_OUT("%30s = Index(%d), 티켓종류(%s), ccs(%d:%d), exp(%d:%d) ..", "현장발권", 
						nRet, tckList.tck_kind,
						s_tAccum.Curr.ccsTicketWork[nRet].tPubTck[nPayDvs].nCount, s_tAccum.Curr.ccsTicketWork[nRet].tPubTck[nPayDvs].dwMoney,
						s_tAccum.Curr.expTicketWork[nRet].tPubTck[nPayDvs].nCount, s_tAccum.Curr.expTicketWork[nRet].tPubTck[nPayDvs].dwMoney);
#endif
				}
				else
				{
					LOG_OUT("%s", "ACC_TICKET_PUB_REV1_ISSUE, 티켓 종류 에러 @@@@@@@@@@@@");
					LOG_OUT("현장발권 = index(%d), (%d:%d:%d:%d:%s) ..", nRet, pAcc->nSvrKind,  pAcc->nPayDvs, pAcc->nCount, pAcc->nFare, pAcc->bus_tck_knd_cd);
				}

				/// 승차권 잔량
				if(pAcc->byPaper != PAPER_ROLL)
				{	/// 감열지가 아니면, 승차권 잔량 감소 시킴
					s_tAccum.Curr.phyTicket.nCount = SubtractValue(s_tAccum.Curr.phyTicket.nCount, pAcc->nCount);
				}
			}
			break;
		/// ~2020.06.03 add code

		// 20220104 add
		case ACC_TICKET_PUB_TEST_ISSUE :
			{	// 현장테스트발권
				TCK_KIND_LIST_T		tckList;
				PACC_TICKET_WORK_T	pAcc;

				::ZeroMemory(&tckList, sizeof(TCK_KIND_LIST_T));

				nSize = sizeof(ACC_TICKET_WORK_T);
				pAcc = (PACC_TICKET_WORK_T) pData;

				nPayDvs = (pAcc->nPayDvs == 0) ? IDX_ACC_CARD : IDX_ACC_CASH;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d:%d:%d:%d:%s) ..", "현장테스트발권", pAcc->nSvrKind, pAcc->nPayDvs, pAcc->nCount, pAcc->nFare, pAcc->bus_tck_knd_cd);
#endif
				LOG_OUT("%30s = 티켓수량_before (%d - %d)", "현장테스트발권", s_tAccum.Curr.phyTicket.nCount, pAcc->nCount);

				/// 승차권 잔량
				s_tAccum.Curr.phyTicket.nCount = SubtractValue(s_tAccum.Curr.phyTicket.nCount, pAcc->nCount);

				LOG_OUT("%30s = 티켓수량__after (%d)", "현장테스트발권", s_tAccum.Curr.phyTicket.nCount);
			}
			break;
		// ~20220104 add

		case ACC_TICKET_MRNP_ISSUE :
			{	// 예매발권
				TCK_KIND_LIST_T		tckList;
				PACC_TICKET_WORK_T	pAcc;

				::ZeroMemory(&tckList, sizeof(TCK_KIND_LIST_T));

				nSize = sizeof(ACC_TICKET_WORK_T);
				pAcc = (PACC_TICKET_WORK_T) pData;

				nPayDvs = (pAcc->nPayDvs == 0) ? IDX_ACC_CARD : IDX_ACC_CASH;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d:%d:%d:%d:%s) ..", "[예매발권]", pAcc->nSvrKind,  pAcc->nPayDvs, pAcc->nCount, pAcc->nFare, pAcc->bus_tck_knd_cd);
#endif
				/// 승차권 잔량
				s_tAccum.Curr.phyTicket.nCount = SubtractValue(s_tAccum.Curr.phyTicket.nCount, pAcc->nCount);
				//s_tAccum.Curr.phyTicket.nCount -= pAcc->nCount;

				nRet = FindTicketKindData(pAcc->nSvrKind, pAcc->bus_tck_knd_cd, (char *)&tckList);
				if(nRet >= 0)
				{
					if(pAcc->nSvrKind == SVR_DVS_CCBUS)
					{
						/// 예매발권 매수
						s_tAccum.Curr.ccsTicketWork[nRet].tMrnp[nPayDvs].nCount += pAcc->nCount;
						/// 예매발권 금액
						s_tAccum.Curr.ccsTicketWork[nRet].tMrnp[nPayDvs].dwMoney += pAcc->nFare;

						/// (수시마감) 예매발권 매수
						s_tAccum.MnuCls.ccsTicketWork[nRet].tMrnp[nPayDvs].nCount += pAcc->nCount;
						/// (수시마감) 예매발권 금액
						s_tAccum.MnuCls.ccsTicketWork[nRet].tMrnp[nPayDvs].dwMoney += pAcc->nFare;
					}
					if(pAcc->nSvrKind & (SVR_DVS_KOBUS | SVR_DVS_TMEXP))
					{
						/// 예매발권 매수
						s_tAccum.Curr.expTicketWork[nRet].tMrnp[nPayDvs].nCount += pAcc->nCount;
						/// 예매발권 금액
						s_tAccum.Curr.expTicketWork[nRet].tMrnp[nPayDvs].dwMoney += pAcc->nFare;

						/// (수시마감) 예매발권 매수
						s_tAccum.MnuCls.expTicketWork[nRet].tMrnp[nPayDvs].nCount += pAcc->nCount;
						/// (수시마감) 예매발권 금액
						s_tAccum.MnuCls.expTicketWork[nRet].tMrnp[nPayDvs].dwMoney += pAcc->nFare;
					}
				}
				else
				{
					LOG_OUT("%s", "ACC_TICKET_PUB_ISSUE, 티켓 종류 에러 @@@@@@@@@@@@");
					LOG_OUT("FindTicketKindData(), nRet(%d) ..", nRet);
				}

			}
			break;

		/// 2020.06.03 add code
		case ACC_TICKET_MRNP_REV1_ISSUE :
			{	// 예매발권
				TCK_KIND_LIST_T		tckList;
				PACC_TICKET_WORK_REV1_T	pAcc;

				::ZeroMemory(&tckList, sizeof(TCK_KIND_LIST_T));

				nSize = sizeof(ACC_TICKET_WORK_REV1_T);
				pAcc = (PACC_TICKET_WORK_REV1_T) pData;

				nPayDvs = (pAcc->nPayDvs == 0) ? IDX_ACC_CARD : IDX_ACC_CASH;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d:%d:%d:%d:0x%02X:%s) ..", "[예매발권]", 
						pAcc->nSvrKind,  pAcc->nPayDvs, pAcc->nCount, pAcc->nFare, pAcc->byPaper, pAcc->bus_tck_knd_cd);
#endif
				/// 승차권 잔량
				if(pAcc->byPaper != PAPER_ROLL)
				{	/// 감열지가 아니면, 승차권 잔량 감소 시킴.
					s_tAccum.Curr.phyTicket.nCount = SubtractValue(s_tAccum.Curr.phyTicket.nCount, pAcc->nCount);
					//s_tAccum.Curr.phyTicket.nCount -= pAcc->nCount;
				}

				nRet = FindTicketKindData(pAcc->nSvrKind, pAcc->bus_tck_knd_cd, (char *)&tckList);
				if(nRet >= 0)
				{
					if(pAcc->nSvrKind == SVR_DVS_CCBUS)
					{
						/// 예매발권 매수
						s_tAccum.Curr.ccsTicketWork[nRet].tMrnp[nPayDvs].nCount += pAcc->nCount;
						/// 예매발권 금액
						s_tAccum.Curr.ccsTicketWork[nRet].tMrnp[nPayDvs].dwMoney += pAcc->nFare;

						/// (수시마감) 예매발권 매수
						s_tAccum.MnuCls.ccsTicketWork[nRet].tMrnp[nPayDvs].nCount += pAcc->nCount;
						/// (수시마감) 예매발권 금액
						s_tAccum.MnuCls.ccsTicketWork[nRet].tMrnp[nPayDvs].dwMoney += pAcc->nFare;
					}
					if(pAcc->nSvrKind & (SVR_DVS_KOBUS | SVR_DVS_TMEXP))
					{
						/// 예매발권 매수
						s_tAccum.Curr.expTicketWork[nRet].tMrnp[nPayDvs].nCount += pAcc->nCount;
						/// 예매발권 금액
						s_tAccum.Curr.expTicketWork[nRet].tMrnp[nPayDvs].dwMoney += pAcc->nFare;

						/// (수시마감) 예매발권 매수
						s_tAccum.MnuCls.expTicketWork[nRet].tMrnp[nPayDvs].nCount += pAcc->nCount;
						/// (수시마감) 예매발권 금액
						s_tAccum.MnuCls.expTicketWork[nRet].tMrnp[nPayDvs].dwMoney += pAcc->nFare;
					}
				}
				else
				{
					LOG_OUT("%s", "ACC_TICKET_PUB_ISSUE, 티켓 종류 에러 @@@@@@@@@@@@");
					LOG_OUT("FindTicketKindData(), nRet(%d) ..", nRet);
				}
			}
			break;
		/// ~2020.06.03 add code

		case ACC_TICKET_REFUND :
			{	// 환불
				PACC_TICKET_WORK_T	pAcc;

				nSize = sizeof(ACC_TICKET_WORK_T);
				pAcc = (PACC_TICKET_WORK_T) pData;

				nPayDvs = (pAcc->nPayDvs == 0) ? IDX_ACC_CARD : IDX_ACC_CASH;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d:%d:%d:%d) ..", "[환불]", pAcc->nSvrKind,  pAcc->nPayDvs, pAcc->nCount, pAcc->nFare);
#endif
				/// 승차권 잔량
				//s_tAccum.Curr.phyTicketBox.nCount += pAcc->nCount;

				nRet = 0;

				if(pAcc->nSvrKind == SVR_DVS_CCBUS)
				{
					/// 환불 매수
					s_tAccum.Curr.ccsTicketWork[nRet].tRefund[nPayDvs].nCount += pAcc->nCount;
					/// 환불 금액
					if( pAcc->nFare < 0 )
					{
						pAcc->nFare = 0;
					}
					s_tAccum.Curr.ccsTicketWork[nRet].tRefund[nPayDvs].dwMoney += pAcc->nFare;

					/// (수시마감) 환불 매수
					s_tAccum.MnuCls.ccsTicketWork[nRet].tRefund[nPayDvs].nCount += pAcc->nCount;
					/// (수시마감) 환불 금액
					s_tAccum.MnuCls.ccsTicketWork[nRet].tRefund[nPayDvs].dwMoney += pAcc->nFare;

				}
				if(pAcc->nSvrKind & (SVR_DVS_KOBUS | SVR_DVS_TMEXP))
				{
					/// 환불 매수
					s_tAccum.Curr.expTicketWork[nRet].tRefund[nPayDvs].nCount += pAcc->nCount;
					/// 환불 금액
					if( pAcc->nFare < 0 )
					{
						pAcc->nFare = 0;
					}
					s_tAccum.Curr.expTicketWork[nRet].tRefund[nPayDvs].dwMoney += pAcc->nFare;

					/// (수시마감) 환불 매수
					s_tAccum.MnuCls.expTicketWork[nRet].tRefund[nPayDvs].nCount += pAcc->nCount;
					/// (수시마감) 환불 금액
					s_tAccum.MnuCls.expTicketWork[nRet].tRefund[nPayDvs].dwMoney += pAcc->nFare;

				}

			}
			break;

		case ACC_KOBUS_TICKET_INFO :
			{	/// 코버스 - 발권 일시 정보
				int nLen;
				PACC_KOBUS_TCK_INFO_T pAcc;

				nLen = 0;
				nSize = sizeof(ACC_KOBUS_TCK_INFO_T);
				pAcc = (PACC_KOBUS_TCK_INFO_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (dt=%s, time=%s) ..", "[코버스_발권정보]", pAcc->kobus_tak_stt_dt, pAcc->kobus_stt_time);
#endif
				// 일자
				nLen = strlen(pAcc->kobus_tak_stt_dt);
				if( nLen >= (sizeof(s_tAccum.tak_stt_dt) - 1) )
				{
					nLen = sizeof(s_tAccum.tak_stt_dt) - 1;
				}
				::ZeroMemory(s_tAccum.tak_stt_dt, sizeof(s_tAccum.tak_stt_dt));
				::CopyMemory(s_tAccum.tak_stt_dt, pAcc->kobus_tak_stt_dt, nLen);

				// 시간
				nLen = strlen(pAcc->kobus_stt_time);
				if( nLen >= (sizeof(s_tAccum.stt_time) - 1) )
				{
					nLen = sizeof(s_tAccum.stt_time) - 1;
				}
				::ZeroMemory(s_tAccum.stt_time, sizeof(s_tAccum.stt_time));
				::CopyMemory(s_tAccum.stt_time, pAcc->kobus_stt_time, nLen);
			}
			break;

		case ACC_TICKET_MINUS :
			{	// 승차권 수량 - Minus
				PACC_TICKET_DATA_T	pAcc;

				nSize = sizeof(ACC_TICKET_DATA_T);
				pAcc = (PACC_TICKET_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d) ..", "[승차권배출]", pAcc->nCount);
#endif
				s_tAccum.Curr.phyTicket.nCount -= pAcc->nCount;
			}
			break;
		case ACC_TICKET_INSERT :
			{	// 승차권 수량 - 보급
				PACC_TICKET_DATA_T pAcc;

				nSize = sizeof(ACC_TICKET_DATA_T);
				pAcc = (PACC_TICKET_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d) ..", "[승차권보급]", pAcc->nCount);
#endif
				//s_tAccum.Curr.Ticket.nCount += pAcc->nCount;
				s_tAccum.Curr.phyTicket.nCount = pAcc->nCount;
			}
			break;
		case ACC_TICKET_ASSIGN :
			{	// 승차권 수량 - 맞추기
				PACC_TICKET_DATA_T pAcc;

				nSize = sizeof(ACC_TICKET_DATA_T);
				pAcc = (PACC_TICKET_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d) ..", "[승차권매수_맞추기]", pAcc->nCount);
#endif
				s_tAccum.Curr.phyTicket.nCount = pAcc->nCount;
			}
			break;
		case ACC_TICKET_CLEAR :
			{	// 승차권 수량 - 초기화
				PACC_TICKET_DATA_T pAcc;

				nSize = sizeof(ACC_TICKET_DATA_T);
				pAcc = (PACC_TICKET_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s = (%d) ..", "[승차권매수_Clear]", pAcc->nCount);
#endif
				::ZeroMemory(&s_tAccum.Curr.phyTicket, sizeof(DM_TICKET_T));
			}
			break;

		case ACC_TICKET_SEQ_PLUS :
			{	// 승차권 일련번호 증가
				PACC_TICKET_DATA_T pAcc;
				PDM_TICKET_T pTicket;

				nSize = sizeof(ACC_TICKET_DATA_T);
				pAcc = (PACC_TICKET_DATA_T) pData;

				if(pAcc->nSvrKind == SVR_DVS_CCBUS)
				{
					pTicket = &s_tAccum.Curr.ccsTicket;

					pTicket->n_bus_tck_inhr_no += pAcc->nCount;
#if (_ACCUM_DEBUG_ > 0)
					LOG_OUT("%30s = (%d) ..", "[시외_승차권일련번호증가]", pTicket->n_bus_tck_inhr_no);
#endif
				}
				if(pAcc->nSvrKind & (SVR_DVS_KOBUS | SVR_DVS_TMEXP))
				{
					pTicket = &s_tAccum.Curr.expTicket;

					pTicket->n_bus_tck_inhr_no += pAcc->nCount;
#if (_ACCUM_DEBUG_ > 0)
					LOG_OUT("%30s = (%d) ..", "[고속_승차권일련번호증가]", pTicket->n_bus_tck_inhr_no);
#endif
				}
			}
			break;

		case ACC_TICKET_SEQ_ASSIGN :
			{	// 승차권 일련번호 assign
				PACC_TICKET_DATA_T pAcc;
				PDM_TICKET_T pTicket;

				nSize = sizeof(ACC_TICKET_DATA_T);
				pAcc = (PACC_TICKET_DATA_T) pData;

				if(pAcc->nSvrKind == SVR_DVS_CCBUS)
				{
					pTicket = &s_tAccum.Curr.ccsTicket;

					pTicket->n_bus_tck_inhr_no = pAcc->nCount;
#if (_ACCUM_DEBUG_ > 0)
					LOG_OUT("%30s = (%d) ..", "[시외_승차권일련번호증가_대입]", pTicket->n_bus_tck_inhr_no);
#endif
				}
				if(pAcc->nSvrKind & (SVR_DVS_KOBUS | SVR_DVS_TMEXP))
				{
					pTicket = &s_tAccum.Curr.expTicket;

					pTicket->n_bus_tck_inhr_no = pAcc->nCount;
#if (_ACCUM_DEBUG_ > 0)
					LOG_OUT("%30s = (%d) ..", "[고속_승차권일련번호증가_대입]", pTicket->n_bus_tck_inhr_no);
#endif
				}
			}
			break;

		case ACC_TICKET_SEQ_CLEAR :
			{	// 승차권 일련번호 초기화
				PACC_TICKET_DATA_T pAcc;
				PDM_TICKET_T pTicket;

				nSize = sizeof(ACC_TICKET_DATA_T);
				pAcc = (PACC_TICKET_DATA_T) pData;

				if(pAcc->nSvrKind == SVR_DVS_CCBUS)
				{
					pTicket = &s_tAccum.Curr.ccsTicket;

					pTicket->n_bus_tck_inhr_no = 1;
#if (_ACCUM_DEBUG_ > 0)
					LOG_OUT("%30s = (%d) ..", "[시외_승차권일련번호_초기화]", pTicket->n_bus_tck_inhr_no);
#endif
				}
				if(pAcc->nSvrKind & (SVR_DVS_KOBUS | SVR_DVS_TMEXP))
				{
					pTicket = &s_tAccum.Curr.expTicket;

					pTicket->n_bus_tck_inhr_no = 1;
#if (_ACCUM_DEBUG_ > 0)
					LOG_OUT("%30s = (%d) ..", "[고속_승차권일련번호_초기화]", pTicket->n_bus_tck_inhr_no);
#endif
				}
			}
			break;

		case ACC_TICKET_ADD_SEQ_ASSIGN :
			{	/// 추가 티켓 고유번호
				PACC_TICKET_DATA_T pAcc;
				PDM_TICKET_T pTicket;

				nSize = sizeof(ACC_TICKET_DATA_T);
				pAcc = (PACC_TICKET_DATA_T) pData;

				if(pAcc->nSvrKind == SVR_DVS_CCBUS)
				{
					pTicket = &s_tAccum.Curr.ccsTicket;

					pTicket->n_add_bus_tck_inhr_no = pAcc->nCount;
#if (_ACCUM_DEBUG_ > 0)
					LOG_OUT("%30s = (%d) ..", "[시외_승차권_추가고유번호증가_대입]", pTicket->n_add_bus_tck_inhr_no);
#endif
				}
				if(pAcc->nSvrKind & (SVR_DVS_KOBUS | SVR_DVS_TMEXP))
				{
					pTicket = &s_tAccum.Curr.expTicket;

					pTicket->n_add_bus_tck_inhr_no = pAcc->nCount;
#if (_ACCUM_DEBUG_ > 0)
					LOG_OUT("%30s = (%d) ..", "[고속_승차권_추가고유번호증가_대입]", pTicket->n_add_bus_tck_inhr_no);
#endif
				}
			}
			break;

		case ACC_TICKETBOX_INSERT :
			{	/// 승차권 수집함 투입
				PACC_TICKET_DATA_T pAcc;

				nSize = sizeof(ACC_TICKET_DATA_T);
				pAcc = (PACC_TICKET_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s ..", "[승차권 수집함 투입]");
#endif

				s_tAccum.Curr.phyTicketBox.nCount += pAcc->nCount;
			}
			break;

		case ACC_TICKETBOX_WITHDRAW :
			{	/// 승차권 수집함 회수
				PACC_TICKET_DATA_T pAcc;

				nSize = sizeof(ACC_TICKET_DATA_T);
				pAcc = (PACC_TICKET_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s ..", "[승차권 수집함 회수]");
#endif
				s_tAccum.Curr.phyTicketBox.nCount = 0;
			}
			break;

		case ACC_UNPAID :
			{	// 미방출 데이타
				PACC_NOPAY_DATA_T pAcc;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s ..", "[미방출 데이타]");
#endif

				nSize = sizeof(ACC_NOPAY_DATA_T);
				pAcc = (PACC_NOPAY_DATA_T) pData;

				s_tAccum.Curr.noCoin.n10 += pAcc->n10;
				s_tAccum.Curr.noCoin.n50 += pAcc->n50;
				s_tAccum.Curr.noCoin.n100 += pAcc->n100;
				s_tAccum.Curr.noCoin.n500 += pAcc->n500;

				s_tAccum.Curr.noBill.n1k += pAcc->n1k;
				s_tAccum.Curr.noBill.n5k += pAcc->n5k;
				s_tAccum.Curr.noBill.n10k += pAcc->n10k;
				s_tAccum.Curr.noBill.n50k += pAcc->n50k;

				// 마감 데이타
				s_tAccum.Close.noCoin.n10 += pAcc->n10;
				s_tAccum.Close.noCoin.n50 += pAcc->n50;
				s_tAccum.Close.noCoin.n100 += pAcc->n100;
				s_tAccum.Close.noCoin.n500 += pAcc->n500;

				s_tAccum.Close.noBill.n1k += pAcc->n1k;
				s_tAccum.Close.noBill.n5k += pAcc->n5k;
				s_tAccum.Close.noBill.n10k += pAcc->n10k;
				s_tAccum.Close.noBill.n50k += pAcc->n50k;
			}
			break;
		case ACC_UNPAID_MINUS :
			{
				PACC_NOPAY_DATA_T pAcc;

				nSize = sizeof(ACC_NOPAY_DATA_T);
				pAcc = (PACC_NOPAY_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s ..", "[미방출 출금]");
#endif
				/// 현재 데이타
				//s_tAccum.Curr.noCoin.n10 -= pAcc->n10;
				//if(s_tAccum.Curr.noCoin.n10 < 0)
				//	s_tAccum.Curr.noCoin.n10 = 0;
				//s_tAccum.Curr.noCoin.n50 -= pAcc->n50;
				//if(s_tAccum.Curr.noCoin.n50 < 0)
				//	s_tAccum.Curr.noCoin.n50 = 0;
				s_tAccum.Curr.noCoin.n10 = s_tAccum.Curr.noCoin.n50 = 0;

				s_tAccum.Curr.noCoin.n100 -= pAcc->n100;
				if(s_tAccum.Curr.noCoin.n100 < 0)
					s_tAccum.Curr.noCoin.n100 = 0;
				s_tAccum.Curr.noCoin.n500 -= pAcc->n500;
				if(s_tAccum.Curr.noCoin.n500 < 0)
					s_tAccum.Curr.noCoin.n500 = 0;

				s_tAccum.Curr.noBill.n1k -= pAcc->n1k;
				if(s_tAccum.Curr.noBill.n1k < 0)
					s_tAccum.Curr.noBill.n1k = 0;
				s_tAccum.Curr.noBill.n5k -= pAcc->n5k;
				if(s_tAccum.Curr.noBill.n5k < 0)
					s_tAccum.Curr.noBill.n5k = 0;
				s_tAccum.Curr.noBill.n10k -= pAcc->n10k;
				if(s_tAccum.Curr.noBill.n10k < 0)
					s_tAccum.Curr.noBill.n10k = 0;
				s_tAccum.Curr.noBill.n50k -= pAcc->n50k;
				if(s_tAccum.Curr.noBill.n50k < 0)
					s_tAccum.Curr.noBill.n50k = 0;

				// 마감 데이타
				//s_tAccum.Close.noCoin.n10 -= pAcc->n10;
				//if(s_tAccum.Close.noCoin.n10 < 0)
				//	s_tAccum.Close.noCoin.n10 = 0;
				//s_tAccum.Close.noCoin.n50 -= pAcc->n50;
				//if(s_tAccum.Close.noCoin.n50 < 0)
				//	s_tAccum.Close.noCoin.n50 = 0;

				s_tAccum.Close.noCoin.n10 = s_tAccum.Close.noCoin.n50 = 0;

				s_tAccum.Close.noCoin.n100 -= pAcc->n100;
				if(s_tAccum.Close.noCoin.n100 < 0)
					s_tAccum.Close.noCoin.n100 = 0;
				s_tAccum.Close.noCoin.n500 -= pAcc->n500;
				if(s_tAccum.Close.noCoin.n500 < 0)
					s_tAccum.Close.noCoin.n500 = 0;

				s_tAccum.Close.noBill.n1k -= pAcc->n1k;
				if(s_tAccum.Close.noBill.n1k < 0)
					s_tAccum.Close.noBill.n1k = 0;
				s_tAccum.Close.noBill.n5k -= pAcc->n5k;
				if(s_tAccum.Close.noBill.n5k < 0)
					s_tAccum.Close.noBill.n5k = 0;
				s_tAccum.Close.noBill.n10k -= pAcc->n10k;
				if(s_tAccum.Close.noBill.n10k < 0)
					s_tAccum.Close.noBill.n10k = 0;
				s_tAccum.Close.noBill.n50k -= pAcc->n50k;
				if(s_tAccum.Close.noBill.n50k < 0)
					s_tAccum.Close.noBill.n50k = 0;

			}
			break;
		case ACC_UNPAID_CLEAR :
			{
				PACC_NOPAY_DATA_T pAcc;

				nSize = sizeof(ACC_NOPAY_DATA_T);
				pAcc = (PACC_NOPAY_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s ..", "[미방출 초기화]");
#endif

				::ZeroMemory(&s_tAccum.Curr.noCoin, sizeof(DM_COIN_T));
				::ZeroMemory(&s_tAccum.Curr.noBill, sizeof(DM_BILL_T));

				::ZeroMemory(&s_tAccum.Close.noCoin, sizeof(DM_COIN_T));
				::ZeroMemory(&s_tAccum.Close.noBill, sizeof(DM_BILL_T));
			}
			break;

		case ACC_CASH_CLOSE :
			{	// 현금 시재 마감
				PACC_CLOSE_DATA_T pAcc;

				nSize = sizeof(ACC_CLOSE_DATA_T);
				pAcc = (PACC_CLOSE_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s ..", "[현금 시재 마감]");
#endif

				/// 초기 데이타 set
				s_tAccum.Close.initCoin.n10 = s_tAccum.Curr.Coin.n10;
				s_tAccum.Close.initCoin.n50 = s_tAccum.Curr.Coin.n50;
				s_tAccum.Close.initCoin.n100 = s_tAccum.Curr.Coin.n100;
				s_tAccum.Close.initCoin.n500 = s_tAccum.Curr.Coin.n500;

				s_tAccum.Close.initBill.n1k = s_tAccum.Curr.BillDispenser.Casst.n1k;
				s_tAccum.Close.initBill.n5k = s_tAccum.Curr.BillDispenser.Casst.n5k;
				s_tAccum.Close.initBill.n10k = s_tAccum.Curr.BillDispenser.Casst.n10k;
				s_tAccum.Close.initBill.n50k = s_tAccum.Curr.BillDispenser.Casst.n50k;

				PrintAccumData(TRUE);

				/// 현재 데이타 초기화
				::ZeroMemory(&s_tAccum.Close.supplyCoin, sizeof(DM_COIN_T));
				::ZeroMemory(&s_tAccum.Close.outCoin, sizeof(DM_COIN_T));
				::ZeroMemory(&s_tAccum.Close.noCoin, sizeof(DM_COIN_T));

				::ZeroMemory(&s_tAccum.Close.inBill, sizeof(DM_BILL_T));
				::ZeroMemory(&s_tAccum.Close.outBill, sizeof(DM_BILL_T));
				::ZeroMemory(&s_tAccum.Close.supplyBill, sizeof(DM_BILL_T));
				::ZeroMemory(&s_tAccum.Close.wdrawBill, sizeof(DM_BILL_T));
				::ZeroMemory(&s_tAccum.Close.noBill, sizeof(DM_BILL_T));

				/// 2021.03.25 add code
				::ZeroMemory(&s_tAccum.MnuCls, sizeof(DM_MNU_CLOSE_T));
				/// ~2021.03.25 add code

				/// 시재마감 시작일시
				::GetLocalTime(&sysTime);
				sprintf(s_tAccum.cash_cls_stt_dt, "%04d%02d%02d%02d%02d%02d", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
			}
			break;
		case ACC_DAY_CLOSE :
			{	// 창구 마감
				PACC_CLOSE_DATA_T pAcc;

				nSize = sizeof(ACC_CLOSE_DATA_T);
				pAcc = (PACC_CLOSE_DATA_T) pData;

#if (_ACCUM_DEBUG_ > 0)
				LOG_OUT("%30s ..", "[창구 마감]");
#endif

// 				LOG_OUT("Begin  ###########################################################################\n");
// 				PrintAccumData(TRUE);
// 				LOG_OUT("Finish ###########################################################################\n");


				::CopyMemory(&s_tAccum.Prev, &s_tAccum.Curr, sizeof(DM_CURRENT_T));

				/// 초기 데이타 set
				::ZeroMemory(&s_tAccum.Curr.ccsTicketWork[0], sizeof(DM_TICKET_WORK_T) * CCS_IDX_TCK_MAX);
				::ZeroMemory(&s_tAccum.Curr.expTicketWork[0], sizeof(DM_TICKET_WORK_T) * TMEXP_IDX_TCK_MAX);

				/// 시재마감 시작일시
				::GetLocalTime(&sysTime);
				sprintf(s_tAccum.day_cls_stt_dt, "%04d%02d%02d%02d%02d%02d", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
			}
			break;
// 20210513 ADD~
		case ACC_DAY_CLOSE_CCS :
			{	// 창구 마감
				PACC_CLOSE_DATA_T pAcc;

				nSize = sizeof(ACC_CLOSE_DATA_T);
				pAcc = (PACC_CLOSE_DATA_T) pData;

				/// CCBUS 창구마감 발생일시
				SYSTEMTIME *pST = (SYSTEMTIME *) Util_DateFromTick(pAcc->Base.dwTick);
				sprintf(s_tAccum.sCls_Ccs_dt, "%04d%02d%02d%02d%02d%02d", pST->wYear, pST->wMonth, pST->wDay, pST->wHour, pST->wMinute, pST->wSecond);

				LOG_OUT("%30s = nKind(%d, 0x%08lx), 일시(%s) ..", "[CCBUS 창구마감]", nKind, nKind, s_tAccum.sCls_Ccs_dt);
			}
			break;
		case ACC_DAY_CLOSE_KOBUS :
			{	// 창구 마감
				PACC_CLOSE_DATA_T pAcc;

				nSize = sizeof(ACC_CLOSE_DATA_T);
				pAcc = (PACC_CLOSE_DATA_T) pData;

				/// KOBUS고속 창구마감 발생일시
				SYSTEMTIME *pST = (SYSTEMTIME *) Util_DateFromTick(pAcc->Base.dwTick);
				sprintf(s_tAccum.sCls_Kobus_dt, "%04d%02d%02d%02d%02d%02d", pST->wYear, pST->wMonth, pST->wDay, pST->wHour, pST->wMinute, pST->wSecond);

				LOG_OUT("%30s = nKind(%d, 0x%08lx), 일시(%s) ..", "[KOBUS고속 창구마감]", nKind, nKind, s_tAccum.sCls_Kobus_dt);
			}
			break;
		case ACC_DAY_CLOSE_TMONEY :
			{	// 창구 마감
				PACC_CLOSE_DATA_T pAcc;

				nSize = sizeof(ACC_CLOSE_DATA_T);
				pAcc = (PACC_CLOSE_DATA_T) pData;

				/// TMONEY고속 창구마감 발생일시
				SYSTEMTIME *pST = (SYSTEMTIME *) Util_DateFromTick(pAcc->Base.dwTick);
				sprintf(s_tAccum.sCls_Tmoney_dt, "%04d%02d%02d%02d%02d%02d", pST->wYear, pST->wMonth, pST->wDay, pST->wHour, pST->wMinute, pST->wSecond);

				LOG_OUT("%30s = nKind(%d, 0x%08lx), 일시(%s) ..", "[TMONEY고속 창구마감]", nKind, nKind, s_tAccum.sCls_Tmoney_dt);
			}
			break;
		}	
// 20210513 ~ADD

		if(!bWrite)	
		{
// 			PrintAccumData(TRUE);
// 			LOG_OUT("END ###########################################################################\n");

			UnLocking();
			return 0;
		}

		/// Debug code
		//PrintAccumData(TRUE);
		//LOG_OUT("WRITE ###########################################################################\n");

		::GetLocalTime(&sysTime);
		dwTick = Util_TickFromDate(sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		ritr = s_FileVector.rbegin();

		strFileName = s_strPath + _T("\\") + (CString) ritr->date;
		hFile = MyOpenFile(strFileName, TRUE);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			UnLocking();
			return -1;
		}

		MySetFilePointer(hFile, 0, FILE_END);

		accHeader.dwTick = dwTick;
		accHeader.nKind = nKind;
		accHeader.nSize = nSize;

		nRet = MyWriteFile(hFile, &accHeader, (int)sizeof(ACC_HEADER_T));
		if(nRet < 0)
		{
			;
		}

		if(nSize > 0)
		{
			nRet = MyWriteFile(hFile, pData, nSize);
			if(nRet < 0)
			{
				;
			}
		}
		MyCloseFile(hFile);

		ritr->dwCount++;
		UnLocking();
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		GetAccumulateData
 * @details		회계 데이타 메모리 가져오기
 * @param		None
 * @return		회계 데이타
 */
//PFILE_ACCUM_T GetAccumulateData(void)
PFILE_ACCUM_N1010_T GetAccumulateData(void)
{
	return &s_tAccum;
}

/**
 * @brief		CreateAccumData
 * @details		회계 데이타 생성
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int CreateAccumData(void)
{
	FILE_ACC_HEADER_T		tFileHdr;
	ACC_HEADER_T			accHeader;
	ACC_FILE_ENTRY_T		dEntry;
	DWORD					dwTick;
	SYSTEMTIME				sysTime;
	ritrFile				ritr;
	itrFile					itr;
	int						nSize, nRet, nCount;
	CString					strFileName;
	HANDLE					hFile;
	char					szBuffer[100];

	LOG_OUT("%s", "start !!!!");

	try
	{
		::GetLocalTime(&sysTime);
		dwTick = Util_TickFromDate(sysTime.wYear, sysTime.wMonth, sysTime.wDay, 0, 0, 0);

		ritr = s_FileVector.rbegin();

		/// 2021.03.26 add code
		tFileHdr.dwVersion	= CHG_ACCUM_VERSION;
		/// ~2021.03.26 add code
		tFileHdr.dwMagic = ACCUM_MAGIC_CODE;
		tFileHdr.dwTick	= dwTick;
		tFileHdr.dwSerial = ritr->dwSerial + 1;
		tFileHdr.dwCount = 0;
		tFileHdr.dwSend	= 0;

		sprintf(szBuffer, "%08u-%04d%02d%02d", tFileHdr.dwSerial, sysTime.wYear, sysTime.wMonth, sysTime.wDay);
		strFileName = s_strPath + _T("\\") + (CString) szBuffer;

		hFile = MyOpenFile(strFileName, TRUE);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			//UnLocking();
			return -1;
		}

		MySetFilePointer(hFile, 0, FILE_END);
		nRet = MyWriteFile(hFile, &tFileHdr, (int)sizeof(FILE_ACC_HEADER_T));
		if(nRet < 0)
		{
			;
		}

		accHeader.dwTick = dwTick;
		accHeader.nKind = ACC_ALL;
		/// 2021.03.26 add code
		//accHeader.nSize = sizeof(FILE_ACCUM_T);
		accHeader.nSize = sizeof(FILE_ACCUM_N1010_T);
		/// ~2021.03.26 add code

		nRet = MyWriteFile(hFile, &accHeader, (int)sizeof(ACC_HEADER_T));
		if(nRet < 0)
		{
			;
		}

		/// 2021.03.26 add code
		//nRet = MyWriteFile(hFile, &s_tAccum, (int)sizeof(FILE_ACCUM_T));
		nRet = MyWriteFile(hFile, &s_tAccum, accHeader.nSize);
		/// ~2021.03.26 add code
		if(nRet < 0)
		{
			;
		}

		MyCloseFile(hFile);

		memset(&dEntry, 0, sizeof(ACC_FILE_ENTRY_T)); 

		sprintf(dEntry.date, "%08u-%04d%02d%02d", tFileHdr.dwSerial, sysTime.wYear, sysTime.wMonth, sysTime.wDay);
		dEntry.dwVersion	= CHG_ACCUM_VERSION;
		dEntry.dwTick		= dwTick;
		dEntry.dwSerial		= tFileHdr.dwSerial;
		dEntry.dwCount		= 0;
		dEntry.dwSend		= 0;
		s_FileVector.push_back(dEntry);

		nCount = (int) s_FileVector.size();

		nSize = nCount - MAX_SAVE_DATE;
		while(nSize > 0) 
		{
			itr = s_FileVector.begin();

			strFileName = s_strPath + _T("\\") + (CString)itr->date;
			::DeleteFile(strFileName);

			s_FileVector.erase(itr);
			nSize--;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		SetAccumPath
 * @details		PATH 설정
 * @param		CString &strPath	저장 PATH
 * @return		None
 */
void SetAccumPath(CString &strPath)
{
	s_strPath = strPath;
}

/**
 * @brief		InitAccumLogFile
 * @details		데이타 Log 파일 Initialize
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int InitAccumLogFile(void)
{
	LOG_INIT();
	return 0;
}

static void DebugAccumInfo(CString& strPath, char *date)
{
	int nRet;
	CString strFileName;
	HANDLE hFile;
	FILE_ACC_HEADER_T	tFileHdr;
	ACC_HEADER_T	accHeader;

	LOG_OUT("\n\n\n\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	LOG_OUT("파일날짜 (%s) ..\n", date);
	LOG_OUT(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n\n\n");


	strFileName = strPath + _T("\\") + (CString) date;
	hFile = MyOpenFile(strFileName, FALSE);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		nRet = MyReadFile(hFile, &tFileHdr, sizeof(FILE_ACC_HEADER_T));
		if(nRet > 0)
		{
			;
		}
		while(1)
		{
			nRet = MyReadFile(hFile, &accHeader, sizeof(ACC_HEADER_T));
			if(nRet < 0)
			{
				break;
			}

			if(accHeader.nSize > 0) 
			{
				if(accHeader.nKind == ACC_ALL) 
				{
					nRet = MyReadFile(hFile, &s_tAccum, accHeader.nSize);
					if(nRet < 0)
					{
						;
					}
				} 
				else 
				{
					nRet = MyReadFile(hFile, accum_buf, accHeader.nSize);
					if(nRet < 0)
					{
						;
					}
				}
				AddAccumulateData(accHeader.nKind, (char *)accum_buf, 0);
			}
		}
		MyCloseFile(hFile);
	}
}

/**
 * @brief		InitAccumData
 * @details		초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int InitAccumData(void)
{
	FILE_ACC_HEADER_T		tFileHdr;
	ACC_HEADER_T			accHeader;
	ACC_FILE_ENTRY_T		dEntry;
	DWORD					dwTick;
	int						ent, nRet, nCount;
	SYSTEMTIME				sysTime;
	WIN32_FIND_DATA			wfd;
	BOOL					bResult = TRUE;
	CString					strFileName, strBuff;
	HANDLE					hFile, hHandle;
	ritrFile				ritr;
	itrFile					itr;
	char					Buffer[256];

	nCount = 0;

	try
	{
		/// 2021.03.26 add code
		::ZeroMemory(&s_tAccumOld, sizeof(FILE_ACCUM_T));
		::ZeroMemory(&s_tAccum, sizeof(FILE_ACCUM_N1010_T));
		/// ~2021.03.26 add code

		{
			USES_CONVERSION;

			::ZeroMemory(Buffer, sizeof(Buffer));
			Util_GetModulePath(Buffer);

			strBuff = (CString) Buffer;

			s_strPath = strBuff + _T("\\Data\\Accum");
		}

		hAccMutex = ::CreateMutex(NULL, FALSE, NULL);

		::GetLocalTime(&sysTime);

		dwTick = Util_TickFromDate(sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		memset(&dEntry, 0, sizeof(ACC_FILE_ENTRY_T));
		s_FileVector.clear();

		if( MyAccessFile(s_strPath) < 0 )
		{
			MyCreateDirectory(s_strPath);
		}

		strFileName = s_strPath + _T("\\*.*");
		hHandle = ::FindFirstFile(strFileName, &wfd);
		while( (hHandle != INVALID_HANDLE_VALUE) && (bResult == TRUE) )  
		{
			if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) 
			{
				;
			} 
			else 
			{
				strFileName.Format(_T("%s\\%s"), s_strPath, wfd.cFileName);
				hFile = MyOpenFile(strFileName, FALSE);
				if(hFile != INVALID_HANDLE_VALUE)
				{
					nRet = MyReadFile(hFile, &tFileHdr, sizeof(FILE_ACC_HEADER_T));
					if(nRet > 0)
					{
						if(tFileHdr.dwMagic == ACCUM_MAGIC_CODE) 
						{
							memset(&dEntry, 0, sizeof(ACC_FILE_ENTRY_T)); 

							/// 2021.03.26 add code
							dEntry.dwVersion= tFileHdr.dwVersion;
							/// ~2021.03.26 add code

							strcpy_s(dEntry.date, (char *)T2A(wfd.cFileName));
							dEntry.dwSerial = tFileHdr.dwSerial;
							dEntry.dwTick	= tFileHdr.dwTick;
							dEntry.dwCount  = tFileHdr.dwCount;
							dEntry.dwSend	= tFileHdr.dwSend;

							s_FileVector.push_back(dEntry);
							MyCloseFile(hFile);
						} 
						else 
						{
							MyCloseFile(hFile);
							::DeleteFile(strFileName);
						}
					}
					else
					{
						MyCloseFile(hFile);
					}
				}
			}
			bResult = ::FindNextFile(hHandle, &wfd);
		}
		::FindClose(hHandle);

		nCount = (int) s_FileVector.size();

		if( nCount >= 1 ) 
		{
			if(nCount > 1) 
			{
				std::sort(s_FileVector.begin(), s_FileVector.end(), SortFunction);
			} 
		} 
		else 
		{	
			/// 2021.03.25 add code
			tFileHdr.dwVersion = CHG_ACCUM_VERSION;
			/// ~2021.03.25 add code

			tFileHdr.dwMagic = ACCUM_MAGIC_CODE;
			tFileHdr.dwTick = dwTick;
			tFileHdr.dwSerial = 0;
			tFileHdr.dwCount = 0;
			tFileHdr.dwSend = 0;
			strBuff.Format(_T("%08u-%04d%02d%02d"), tFileHdr.dwSerial, sysTime.wYear, sysTime.wMonth, sysTime.wDay);
			strFileName = s_strPath + _T("\\") + strBuff;

			hFile = MyOpenFile(strFileName, TRUE);
			if(hFile != INVALID_HANDLE_VALUE)
			{
				nRet = MyWriteFile(hFile, &tFileHdr, sizeof(FILE_ACC_HEADER_T));
				if(nRet < 0)
				{
					;
				}

				accHeader.dwTick = dwTick;
				accHeader.nKind = ACC_ALL;
				/// 2021.03.25 add code
				//accHeader.nSize = sizeof(FILE_ACCUM_T);
				accHeader.nSize = sizeof(FILE_ACCUM_N1010_T);
				/// ~2021.03.25 add code

				nRet = MyWriteFile(hFile, &accHeader, sizeof(ACC_HEADER_T));
				if(nRet < 0)
				{
					;
				}

				DefaultAccumData();

				//LOG_OUT("@@@@@ MyWriteFile ... ");
				//LOG_OUT("MyWriteFile ... strFileName(%s), nKind(%lld), nSize(%lld) !!!", strFileName, accHeader.nKind, accHeader.nSize);

				/// 2021.03.25 add code
				//nRet = MyWriteFile(hFile, &s_tAccum, sizeof(FILE_ACCUM_T));
				nRet = MyWriteFile(hFile, &s_tAccum, sizeof(FILE_ACCUM_N1010_T));
				/// ~2021.03.25 add code
				if(nRet < 0)
				{
					;
				}

				MyCloseFile(hFile);

				memset(&dEntry, 0, sizeof(ACC_FILE_ENTRY_T)); 

				/// 2021.03.25 add code
				dEntry.dwVersion = CHG_ACCUM_VERSION;
				/// ~2021.03.25 add code
				sprintf(dEntry.date, "%08u-%04d%02d%02d", tFileHdr.dwSerial, sysTime.wYear, sysTime.wMonth, sysTime.wDay);
				dEntry.dwTick = dwTick;
				dEntry.dwSerial = 0;
				dEntry.dwCount = 0;
				dEntry.dwSend = 0;
				s_FileVector.push_back(dEntry);
			}
		} 

		/// test code
		if(1)
		{
			nCount = (int) s_FileVector.size();
			itr = s_FileVector.begin();
			for(int i = 0; i < nCount; i++)
			{
				DebugAccumInfo(s_strPath, itr->date);
				itr++;
			}
		}

		nCount = (int) s_FileVector.size();
		ent = nCount - MAX_SAVE_DATE;
		while(ent > 0) 
		{
			itr = s_FileVector.begin();

			strFileName = s_strPath + _T("\\") + (CString) itr->date;
			::DeleteFile(strFileName);

			s_FileVector.erase(itr);
			ent--;
		}

		ritr = s_FileVector.rbegin();

		strFileName = s_strPath + _T("\\") + (CString) ritr->date;

		hFile = MyOpenFile(strFileName, FALSE);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			nRet = MyReadFile(hFile, &tFileHdr, sizeof(FILE_ACC_HEADER_T));
			if(nRet > 0)
			{
				;
			}
			while(1)
			{
				nRet = MyReadFile(hFile, &accHeader, sizeof(ACC_HEADER_T));
				if(nRet < 0)
				{
					break;
				}

				//LOG_OUT("@@@@@ MyReadFile ... ");
				//LOG_OUT("MyReadFile ... strFileName(%s), nKind(%d), nSize(%d) !!!", strFileName, accHeader.nKind, accHeader.nSize);

				if(accHeader.nSize > 0) 
				{
					if(accHeader.nKind == ACC_ALL) 
					{
						/// 2021.03.25 add code
						//nRet = MyReadFile(hFile, &s_tAccum, accHeader.nSize);
						nRet = MyReadFile(hFile, &s_tAccum, accHeader.nSize);
						/// ~2021.03.25 add code
						if(nRet < 0)
						{
							;
						}
					} 
					else 
					{
						nRet = MyReadFile(hFile, accum_buf, accHeader.nSize);
						if(nRet < 0)
						{
							;
						}
					}

					LOG_OUT("AddAccumulateData ... nKind(%d), nSize(%d) !!!", accHeader.nKind, accHeader.nSize);
					AddAccumulateData(accHeader.nKind, (char *)accum_buf, 0);
				}
			}
			MyCloseFile(hFile);
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

 	return nCount;
}

/**
 * @brief		TermAccumData
 * @details		데이타 종료
 * @param		None
 * @return		항상 = 0
 */
int TermAccumData(void)
{
	itrFile		itr;
	int			i, nSize;

	if(hAccMutex != NULL)
	{
		::CloseHandle(hAccMutex);
		hAccMutex = NULL;
	}

	nSize = (int)s_FileVector.size();
	for(i = 0 ; i < nSize; i++) 
	{
		itr = s_FileVector.begin();
		s_FileVector.erase(itr);
	}		  

	return 0;
}

/**
 * @brief		AddAccumCoinData
 * @details		Coin 회계 Data
 * @param		None
 * @return		항상 = 0
 */
int AddAccumCoinData(WORD wKind, int n100, int n500)
{
	ACC_COIN_DATA_T tAcc;

	::ZeroMemory(&tAcc, sizeof(ACC_COIN_DATA_T));

	tAcc.Base.wKind = wKind;
	tAcc.Base.dwTick = Util_GetCurrentTick();
	tAcc.Coin.n100 = n100;
	tAcc.Coin.n500 = n500;

	AddAccumulateData((int)wKind, (char *)&tAcc, 1);

	return 0;
}

/**
 * @brief		AddAccumBillData
 * @details		지폐 회계 Data
 * @param		None
 * @return		항상 = 0
 */
int AddAccumBillData(WORD wKind, int n1k, int n5k, int n10k, int n50k)
{
	ACC_BILL_DATA_T tAcc;

	::ZeroMemory(&tAcc, sizeof(ACC_BILL_DATA_T));

	tAcc.Base.wKind = wKind;
	tAcc.Base.dwTick = Util_GetCurrentTick();
	tAcc.Bill.n1k = n1k;
	tAcc.Bill.n5k = n5k;
	tAcc.Bill.n10k = n10k;
	tAcc.Bill.n50k = n50k;

	AddAccumulateData((int)wKind, (char *)&tAcc, sizeof(ACC_BILL_DATA_T));

	return 0;
}

/**
 * @brief		AddAccumTicketData
 * @details		승차권 회계 Data
 * @param		None
 * @return		항상 = 0
 */
int AddAccumTicketData(WORD wKind, int nCount)
{
	ACC_TICKET_DATA_T tAcc;

	::ZeroMemory(&tAcc, sizeof(ACC_TICKET_DATA_T));

	tAcc.Base.wKind = wKind;
	tAcc.Base.dwTick = Util_GetCurrentTick();
	tAcc.nCount = nCount;

	AddAccumulateData((int)wKind, (char *)&tAcc, sizeof(ACC_TICKET_DATA_T));

	return 0;
}

/**
 * @brief		AddAccumUnPaidData
 * @details		미방출 데이타 
 * @param		None
 * @return		항상 = 0
 */
//int AddAccumNoPayData(WORD wKind, int n100, int n500, int n1k, int n10k)
int AddAccumUnPaidData(WORD wKind, int n100, int n500, int n1k, int n10k)
{
	ACC_NOPAY_DATA_T tAcc;

	::ZeroMemory(&tAcc, sizeof(ACC_NOPAY_DATA_T));

	tAcc.Base.wKind = wKind;
	tAcc.Base.dwTick = Util_GetCurrentTick();
	tAcc.n100 = n100;
	tAcc.n500 = n500;

	tAcc.n1k = n1k;
	tAcc.n10k = n10k;

	AddAccumulateData((int)wKind, (char *)&tAcc, sizeof(ACC_NOPAY_DATA_T));

	return 0;
}

/**
 * @brief		AddAccumCashCloseData
 * @details		시재마감 - 현금데이타
 * @param		None
 * @return		항상 = 0
 */
int AddAccumCashCloseData(void)
{
	ACC_CLOSE_DATA_T tAcc;

	::ZeroMemory(&tAcc, sizeof(ACC_CLOSE_DATA_T));

	tAcc.Base.wKind = ACC_CASH_CLOSE;
	tAcc.Base.dwTick = Util_GetCurrentTick();
	tAcc.nCount = 0;

	AddAccumulateData((int)ACC_CASH_CLOSE, (char *)&tAcc, sizeof(ACC_CLOSE_DATA_T));

	return 0;
}

/**
 * @brief		AddAccumWndCloseData
 * @details		창구마감 
 * @param		None
 * @return		항상 = 0
 */
int AddAccumWndCloseData(void)
{
	ACC_CLOSE_DATA_T tAcc;

	::ZeroMemory(&tAcc, sizeof(ACC_CLOSE_DATA_T));

	tAcc.Base.wKind = ACC_DAY_CLOSE;
	tAcc.Base.dwTick = Util_GetCurrentTick();
	tAcc.nCount = 0;

	AddAccumulateData((int)ACC_DAY_CLOSE, (char *)&tAcc, sizeof(ACC_CLOSE_DATA_T));

	return 0;
}

// 20210513 ADD
/**
 * @brief		AddAccumWndClsCcsData
 * @details		시외 창구마감 발생일시 
 * @param		None
 * @return		항상 = 0
 */
int AddAccumWndClsCcsData(void)
{
	ACC_CLOSE_DATA_T tAcc;

	::ZeroMemory(&tAcc, sizeof(ACC_CLOSE_DATA_T));

	tAcc.Base.wKind = ACC_DAY_CLOSE_CCS;
	tAcc.Base.dwTick = Util_GetCurrentTick();
	tAcc.nCount = 0;

	LOG_OUT("%s", "AddAccumWndClsCcsData......");
	AddAccumulateData((int)ACC_DAY_CLOSE_CCS, (char *)&tAcc, sizeof(ACC_CLOSE_DATA_T));

	return 0;
}

/**
 * @brief		AddAccumWndClsKobusData
 * @details		KOBUS고속 창구마감 발생일시 
 * @param		None
 * @return		항상 = 0
 */
int AddAccumWndClsKobusData(void)
{
	ACC_CLOSE_DATA_T tAcc;

	::ZeroMemory(&tAcc, sizeof(ACC_CLOSE_DATA_T));

	tAcc.Base.wKind = ACC_DAY_CLOSE_KOBUS;
	tAcc.Base.dwTick = Util_GetCurrentTick();
	tAcc.nCount = 0;

	LOG_OUT("%s", "AddAccumWndClsKobusData......");
	AddAccumulateData((int)ACC_DAY_CLOSE_KOBUS, (char *)&tAcc, sizeof(ACC_CLOSE_DATA_T));

	return 0;
}

/**
 * @brief		AddAccumWndClsTmoneyData
 * @details		TMONEY고속 창구마감 발생일시 
 * @param		None
 * @return		항상 = 0
 */
int AddAccumWndClsTmoneyData(void)
{
	ACC_CLOSE_DATA_T tAcc;

	::ZeroMemory(&tAcc, sizeof(ACC_CLOSE_DATA_T));

	tAcc.Base.wKind = ACC_DAY_CLOSE_TMONEY;
	tAcc.Base.dwTick = Util_GetCurrentTick();
	tAcc.nCount = 0;

	LOG_OUT("%s", "AddAccumWndClsTmoneyData......");
	AddAccumulateData((int)ACC_DAY_CLOSE_TMONEY, (char *)&tAcc, sizeof(ACC_CLOSE_DATA_T));

	return 0;
}
// 20210513 ~ADD

/**
 * @brief		AddAccumBusTckInhrNo
 * @details		승차권 고유번호 데이타 저장
 * @param		None
 * @return		항상 = 0
 */
int AddAccumBusTckInhrNo(int nKind, int nSvrKind, int n_bus_tck_inhr_no)
{
	ACC_TICKET_DATA_T tAcc;

	::ZeroMemory(&tAcc, sizeof(ACC_TICKET_DATA_T));

	TR_LOG_OUT("AccumBusTckInhrNo_bef = [%d][%d][%d]", nKind, nSvrKind, n_bus_tck_inhr_no);

	switch(nKind)
	{
	case ACC_TICKET_SEQ_PLUS:
		{
			tAcc.Base.wKind = (WORD) nKind;
			tAcc.Base.dwTick = Util_GetCurrentTick();
			tAcc.nSvrKind = nSvrKind;
			tAcc.nCount = n_bus_tck_inhr_no;
		}
		break;

	case ACC_TICKET_SEQ_ASSIGN:
		{
			tAcc.Base.wKind = (WORD) nKind;
			tAcc.Base.dwTick = Util_GetCurrentTick();
			tAcc.nSvrKind = nSvrKind;
			tAcc.nCount = n_bus_tck_inhr_no;
		}
		break;
	case ACC_TICKET_ADD_SEQ_ASSIGN:
		{
			tAcc.Base.wKind = (WORD) nKind;
			tAcc.Base.dwTick = Util_GetCurrentTick();
			tAcc.nSvrKind = nSvrKind;
			tAcc.nCount = n_bus_tck_inhr_no;
		}
		break;
	}

	TR_LOG_OUT("AccumBusTckInhrNo_aft = [%d][%d][%d]", tAcc.Base.wKind, tAcc.nSvrKind, tAcc.nCount);

	AddAccumulateData(nKind, (char *)&tAcc, sizeof(ACC_TICKET_DATA_T));

	return 0;
}

/**
 * @brief		AddAccumTicketWork
 * @details		승차권 업무 데이타 저장
 * @param		None
 * @return		항상 = 0
 */
int AddAccumTicketWork(int nSvrKind, int nPayDvs, char *tck_kind, int nKind, int nCount, int nFare)
{
	ACC_TICKET_WORK_T tAcc;

	::ZeroMemory(&tAcc, sizeof(ACC_TICKET_WORK_T));

	TR_LOG_OUT("AccumTicket_bef = [%d:%d:%s:%d:%d:%d] ..", nSvrKind, nPayDvs, tck_kind, nKind, nCount, nFare);
	tAcc.Base.wKind = nKind;
	tAcc.Base.dwTick = Util_GetCurrentTick();
	
	tAcc.nSvrKind = nSvrKind;
	tAcc.nPayDvs = nPayDvs;
	tAcc.nCount = nCount;
	tAcc.nFare  = nFare;
	::CopyMemory(tAcc.bus_tck_knd_cd, tck_kind, strlen(tck_kind));

	TR_LOG_OUT("AccumTicket_aft = [%d:%d:%s:%d:%d:%d] ..", tAcc.nSvrKind, tAcc.nPayDvs, tAcc.bus_tck_knd_cd, tAcc.Base.wKind, tAcc.nCount, tAcc.nFare);

	AddAccumulateData(nKind, (char *)&tAcc, sizeof(ACC_TICKET_WORK_T));

	return 0;
}

/// 2020.06.03 add code
/**
 * @brief		AddAccumTicketWork_Rev1
 * @details		승차권 업무 데이타 저장
 * @param		None
 * @return		항상 = 0
 */
int AddAccumTicketWork_Rev1(int nSvrKind, int nPayDvs, char *tck_kind, int nKind, int nCount, int nFare, BYTE byPaper)
{
	ACC_TICKET_WORK_REV1_T tAcc;

	::ZeroMemory(&tAcc, sizeof(ACC_TICKET_WORK_REV1_T));

	TR_LOG_OUT("Rev1_bef = [%d:%d:%s:%d:%d:%d:0x%02X] ..", 
				nSvrKind, nPayDvs, tck_kind, nKind, nCount, nFare, byPaper);

	tAcc.Base.wKind = nKind;
	tAcc.Base.dwTick = Util_GetCurrentTick();
	
	tAcc.nSvrKind = nSvrKind;
	tAcc.nPayDvs = nPayDvs;
	tAcc.nCount = nCount;
	tAcc.nFare  = nFare;
	tAcc.byPaper = byPaper;
	::CopyMemory(tAcc.bus_tck_knd_cd, tck_kind, strlen(tck_kind));

	TR_LOG_OUT("Rev1_aft = [%d:%d:%s:%d:%d:%d:0x%02X] ..", 
				tAcc.nSvrKind, tAcc.nPayDvs, tAcc.bus_tck_knd_cd, tAcc.Base.wKind, tAcc.nCount, tAcc.nFare, tAcc.byPaper);

	AddAccumulateData(nKind, (char *)&tAcc, sizeof(ACC_TICKET_WORK_REV1_T));

	return 0;
}
/// ~2020.06.03 add code

/// 20220104 add
/**
 * @brief		AddAccumTicketWork_Rev2
 * @details		승차권 업무 데이타 저장 for 테스트발권
 * @param		None
 * @return		항상 = 0
 */
int AddAccumTicketWork_Rev2(int nSvrKind, int nPayDvs, char *tck_kind, int nKind, int nCount)
{
	ACC_TICKET_WORK_T tAcc;

	::ZeroMemory(&tAcc, sizeof(ACC_TICKET_WORK_T));

	TR_LOG_OUT("AccumTicketWork_Rev2_bef = [%d:%d:%s:%d:%d] ..", nSvrKind, nPayDvs, tck_kind, nKind, nCount);
	tAcc.Base.wKind = nKind;
	tAcc.Base.dwTick = Util_GetCurrentTick();
	
	tAcc.nSvrKind = nSvrKind;
	tAcc.nPayDvs = nPayDvs;
	tAcc.nCount = nCount;
	::CopyMemory(tAcc.bus_tck_knd_cd, tck_kind, strlen(tck_kind));

	TR_LOG_OUT("AccumTicketWork_Rev2_aft = [%d:%d:%s:%d:%d] ..", tAcc.nSvrKind, tAcc.nPayDvs, tAcc.bus_tck_knd_cd, tAcc.Base.wKind, tAcc.nCount);

	AddAccumulateData(nKind, (char *)&tAcc, sizeof(ACC_TICKET_WORK_T));

	return 0;
}
/// ~20220104 add

/**
 * @brief		AddAccumKobusTicketInfo
 * @details		[코버스] 발권정보
 * @param		None
 * @return		항상 = 0
 */
int AddAccumKobusTicketInfo(char *pDate, char *pTime)
{
	int nKind;
	ACC_KOBUS_TCK_INFO_T tAcc;

	::ZeroMemory(&tAcc, sizeof(ACC_KOBUS_TCK_INFO_T));

	nKind = ACC_KOBUS_TICKET_INFO;

	tAcc.Base.wKind = (WORD) nKind;
	tAcc.Base.dwTick = Util_GetCurrentTick();
	
	sprintf(tAcc.kobus_tak_stt_dt, "%s", pDate);
	sprintf(tAcc.kobus_stt_time, "%s", pTime);

	AddAccumulateData(nKind, (char *)&tAcc, sizeof(ACC_KOBUS_TCK_INFO_T));

	return 0;
}

/**
 * @brief		CheckUnpaidData
 * @details		미방출 데이타 체크
 * @param		None
 * @return		항상 = 0
 */
int CheckUnpaidData(void)
{
	int nCoin, nBill;
	static int nPrevCoin = -1;
	static int nPrevBill = -1;

	nCoin = s_tAccum.Curr.noCoin.n10 + s_tAccum.Curr.noCoin.n50 + s_tAccum.Curr.noCoin.n100 + s_tAccum.Curr.noCoin.n500;
	nBill = s_tAccum.Curr.noBill.n1k + s_tAccum.Curr.noBill.n5k + s_tAccum.Curr.noBill.n10k + s_tAccum.Curr.noBill.n50k;

	if(nPrevCoin != nCoin)
	{
		LOG_OUT("미방출 동전값 = %d", nCoin);
		nPrevCoin = nCoin;
	}

	if( nCoin > 0 )
	{
		SetCheckEventCode(EC_COIN_NO_OUT, TRUE);		
	}
	else
	{
		SetCheckEventCode(EC_COIN_NO_OUT, FALSE);		
	}

	if(nPrevBill != nBill)
	{
		LOG_OUT("미방출 지폐값 = %d", nBill);
		nPrevBill = nBill;
	}

	if( nBill > 0 )
	{
		SetCheckEventCode(EC_CDU_NO_OUT, TRUE);		
	}
	else
	{
		SetCheckEventCode(EC_CDU_NO_OUT, FALSE);		
	}

	return 0;
}

/**
 * @brief		GetAccumINHR_No
 * @details		승차권 고유번호 얻기
 * @param		int nSvrKind		서버종류
 * @return		승차권 고유번호
 */
int GetAccumINHR_No(int nSvrKind)
{
	int n_bus_tck_inhr_no = 1;
	
	switch(nSvrKind)
	{
	case SVR_DVS_CCBUS:
		n_bus_tck_inhr_no = s_tAccum.Curr.ccsTicket.n_bus_tck_inhr_no;
		break;
	case SVR_DVS_KOBUS:
	case SVR_DVS_TMEXP:
		n_bus_tck_inhr_no = s_tAccum.Curr.expTicket.n_bus_tck_inhr_no;
		break;
	default:
		break;	
	}

	return n_bus_tck_inhr_no;
}
