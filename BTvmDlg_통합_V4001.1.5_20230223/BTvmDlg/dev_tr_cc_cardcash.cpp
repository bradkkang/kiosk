// 
// 
// dev_tr_cc_cardcash.cpp : 현금+카드 복합 거래
//

#include "stdafx.h"
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <queue>
#include <iostream>
#include <vector>

#include <usrinc/fbuf.h>
#include "xzzbus_fdl.h"

#include "MyDefine.h"
#include "MyUtil.h"
#include "File_Env_ini.h"
#include "oper_config.h"
#include "oper_kos_file.h"
#include "md5.h"
#include "event_if.h"
#include "dev_bill_main.h"
#include "dev_cardreader_main.h"
#include "dev_coin_main.h"
#include "dev_dispenser_main.h"
#include "dev_prt_main.h"
#include "dev_prt_ticket_main.h"
#include "dev_scanner_main.h"
#include "svr_main.h"
#include "svr_ccs_main.h"
#include "svr_ko_main.h"
#include "svr_tm_exp_main.h"
#include "data_main.h"
#include "dev_tr_main.h"
#include "dev_tr_mem.h"
#include "dev_tr_kobus_mem.h"
#include "dev_tr_tmexp_mem.h"
#include "dev_tr_cc_cardcash.h"

//----------------------------------------------------------------------------------------------------------------------

static int			s_nStep, s_nPrevStep = -1;
static int			s_nState, s_nPrevStete = -1;  
static DWORD		s_dwTick;

//----------------------------------------------------------------------------------------------------------------------

static void SetState(int nState, int nStep)
{
	//s_nState = nState;
	Transact_SetState(nState, nStep);
}

static void SetPrevState(int nStep)
{
	s_nPrevStep = nStep;
	//Transact_SetPrevStep(nStep);
}

static void SetStep(int nValue)
{
	//s_nStep = nValue;
	Transact_SetStep(nValue);
}

/**
 * @brief		CheckCashDevice
 * @details		장비 상태 체크
 * @param		DWORD dwStartTick		시작 Tick
 * @return		성공 >= 0, 실패 < 0
 */
static int CheckCashDevice(void)
{
	int nRet = 0;
	int nStep = 0;
	static DWORD dwStartTick = 0;

	try
	{
		if(dwStartTick == 0)
		{
			dwStartTick = GetTickCount();
		}

	//	if( Util_CheckExpire(dwStartTick) <= (1000 * 2) )	
		if( Util_CheckExpire(dwStartTick) <= (1000 * 2) )	
		{
			return 0;
		}

		PDEV_CFG_T		pEnv = NULL;		// 20230206 ADD
		PDEV_CFG_T		pEnvPrtTck = NULL;	// 20230206 ADD

		nRet = GetConfigPayment();
		if( (nRet == PAY_ONLY_CASH) || (nRet == PAY_CARD_CASH) )
		{
			// 승차권 수량 체크
			if( GetEventCode(EC_JOB_CLOSE) )
			{
				//TR_LOG_OUT("#1-%d. CheckTicketCount() ..", nStep++);
			}
			CheckTicketCount();

			// 동전 수량 체크
			if( GetEventCode(EC_JOB_CLOSE) )
			{
				//TR_LOG_OUT("#1-%d. CheckCoinCount() ..", nStep++);
			}
			CheckCoinCount();

			// 지폐 수량 체크
			if( GetEventCode(EC_JOB_CLOSE) )
			{
				//TR_LOG_OUT("#1-%d. CheckBillCount() ..", nStep++);
			}
			CheckBillCount();

			// 미방출데이타 체크
			if( GetEventCode(EC_JOB_CLOSE) )
			{
				//TR_LOG_OUT("#1-%d. CheckUnpaidData() ..", nStep++);
			}
			CheckUnpaidData();

			// 자동마감 체크
			if( GetEventCode(EC_JOB_CLOSE) )
			{
				//TR_LOG_OUT("#1-%d. CheckAutoClose() ..", nStep++);
			}
			CheckAutoClose();
	
			// 서비스 시간 체크
			CheckServiceTime();

			if (1)
			{
				/// 2020.06.19 modify
				//TR_LOG_OUT("#1-%d. Printer_GetStatus() ..", nStep++);
				if( IsEnvPrinterRealCheck() > 0 )
				{
					///TR_LOG_OUT(" >>>>>>>>>> IsEnvPrinterRealCheck() is OK..");
					// GetConfigTicketPapaer @return ; PAPER_ROLL(0x31):감열지, PAPER_TICKET(0x32):승차권		// 20230203 ADD // 20230216 MOD
					if(GetConfigTicketPapaer() == PAPER_ROLL)												// 20230203 DEL // 20230217 MOD
					//pEnv = (PDEV_CFG_T) GetEnvPrtReceiptInfo();												// 20230206 ADD // 20230217 MOD
					//if(pEnv->nUse == 1) // 영수증(감열지) 프린터 사용 옵션이 'USE = 1'인 경우					// 20230206 ADD // 20230217 MOD
					{
						Printer_GetStatus();
					}
				}
				pEnvPrtTck = (PDEV_CFG_T) GetEnvPrtTicketInfo();							// 20230206 ADD
				if(pEnvPrtTck->nUse == 1) // 승차권(종이) 프린터 사용 옵션이 'USE = 1'인 경우	// 20230206 ADD
				{																			// 20230206 ADD
					TckPrt_GetStatus();
				}																			// 20230206 ADD
				//*dwStartTick = GetTickCount();
			}
			
			// 지폐방출기
			CDU_GetStatus();
			
			// 승차권스캐너
			Scanner_GetDeviceStatus();

			// 동전방출기
			Coin_GetStatus();
		}
		else
		{
			// 승차권 수량 체크
 			//TR_LOG_OUT("#1-%d. CheckTicketCount() ..", nStep++);
 			CheckTicketCount();

			// 자동마감 체크
			//TR_LOG_OUT("#1-%d. CheckAutoClose() ..", nStep++);
			CheckAutoClose();
	
			// 서비스 시간 체크
			//TR_LOG_OUT("#1-%d. CheckServiceTime() ..", nStep++);
			CheckServiceTime();

			if (1)
			{
				//TR_LOG_OUT("#1-%d. Printer_GetStatus() ..", nStep++);
				if( IsEnvPrinterRealCheck() > 0 )
				{
					//TR_LOG_OUT(" >>>>>>>>>> IsEnvPrinterRealCheck() is OK..");
					// GetConfigTicketPapaer @return ; PAPER_ROLL(0x31):감열지, PAPER_TICKET(0x32):승차권		// 20230203 ADD // 20230216 MOD
					if(GetConfigTicketPapaer() == PAPER_ROLL)												// 20230203 DEL // 20230217 MOD
					//pEnv = (PDEV_CFG_T) GetEnvPrtReceiptInfo();												// 20230206 ADD // 20230217 MOD
					//if(pEnv->nUse == 1) // 영수증(감열지) 프린터 사용 옵션이 'USE = 1'인 경우					// 20230206 ADD // 20230217 MOD
					{
						Printer_GetStatus();
					}
				}
				//TR_LOG_OUT("#1-%d. Printer_GetStatus() ..", nStep++);
				pEnvPrtTck = (PDEV_CFG_T) GetEnvPrtTicketInfo();							// 20230206 ADD
				if(pEnvPrtTck->nUse == 1) // 승차권(종이) 프린터 사용 옵션이 'USE = 1'인 경우	// 20230206 ADD
				{																			// 20230206 ADD
					TckPrt_GetStatus();
				}																			// 20230206 ADD
				//TR_LOG_OUT("#1-%d. TckPrt_GetStatus() ..", nStep++);
			}
		}

		//TR_LOG_OUT("#1-%d. GetCondition() ..", nStep++);
		if( GetCondition() < 0 )
		{
			//TR_LOG_OUT("#1-%d. SetCheckEventCode(EC_OUT_SERVICE) ..", nStep++);
			SetCheckEventCode(EC_OUT_SERVICE, FALSE);
		}
		else
		{
			//TR_LOG_OUT("#1-%d. SetCheckEventCode(EC_OUT_SERVICE) ..", nStep++);
			SetCheckEventCode(EC_OUT_SERVICE, TRUE);
		}
		//TR_LOG_OUT("#1-%d. end() ..", nStep++);

		dwStartTick = GetTickCount();
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		BootState
 * @details		Boot State 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrBootState(void)
{
	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{
			SetStep(1);
		}
		break;
	case 1 :
		{
			SetStep(2);
		}
		break;
	case 2 :
		{
			SetState(TR_INIT_STATE, 0);
		}
		break;
	}

	return 0;
}

/**
 * @brief		InitState
 * @details		Init State 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrInitState(void)
{
	int		nRet;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{
			nRet = Bill_Reset();
			nRet = Coin_Reset(TRUE, TRUE);
			//nRet = CDU_Reset();
			//nRet = Coin_GetStatus();
			s_dwTick = GetTickCount();
			SetStep(1);
		}
		break;
	case 1 :
		{
			if( Util_CheckExpire(s_dwTick) >= 500 )	
			{
				nRet = Coin_GetStatus();
				nRet = Bill_GetStatus();
				SetStep(2);
			}
		}
		break;
	case 2 :
		{
			SetState(TR_READY_STATE, 0);
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrReadyState
 * @details		Ready State 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrReadyState(void)
{
	int		nRet = 0;
	int n1k = 0, n5k = 0, n10k = 0, n50k = 0;
	static DWORD dwTick;

	Bill_GetMoney(&n1k, &n5k, &n10k, &n50k);
	if( (n1k + n5k + n10k + n50k) > 0 )
	{
		TR_LOG_OUT("지폐입금 확인 !!!, n1k = %d, n5k = %d, n10k = %d, n50k = %d", n1k, n5k, n10k, n50k);
		Bill_TotalEnd();
	}

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{
			dwTick = GetTickCount();
			nRet = Bill_Inhibit();
			nRet = Bill_TotalEnd();
			nRet = Coin_TotalEnd();
			//Printer_GetStatus();
			//TckPrt_GetStatus();
			SetStep(1);
			dwTick = GetTickCount();
		}
		break;
	case 1 :
		break;
	}

	return 0;
}

/**
 * @brief		TrMrsMainState
 * @details		예매발권 Main 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsMainState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	// 
			CMrnpMem::GetInstance()->Initialize();
			CMrnpKobusMem::GetInstance()->Initialize();
			CMrnpTmExpMem::GetInstance()->Initialize();
			
			SetStep(1);
		}
		break;
	case 1 :
		{	
		}
		break;
	}

	return 0;
}


/**
 * @brief		TrMrsCardReadState
 * @details		예매발권 - 신용카드 읽기 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsCardReadState(void)
{
	return 0;
}

/**
 * @brief		TrMrsInputState
 * @details		예매발권 - 예약번호 or 휴대폰번호/생년월일 입력 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsInputState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrMrsReqListState
 * @details		예매발권 - 예매리스트 요청 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsReqListState(void)
{
	int		nRet = 0, i = 0, nCount = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// idle 상태...
			nRet = Svr_IfSv_137();
			TR_LOG_OUT("[시외] 예매리스트 요청, nRet(%d) !!!", nRet);
			if(nRet > 0)
			{	/// 성공
				int i, nCount, nSize, nOffset;
				BYTE* pSPacket;

				i = nCount = nSize = nOffset = 0;

				nCount = nRet;

				nSize = 3 + (sizeof(UI_SND_MRS_LIST_T) * nCount);
				pSPacket = new BYTE[ nSize ];

				::ZeroMemory(pSPacket, nSize);

				// ACK
				pSPacket[nOffset++] = CHAR_ACK;
				// COUNT
				::CopyMemory(&pSPacket[nOffset], &nCount, sizeof(WORD));
				nOffset += sizeof(WORD);

				nRet = CMrnpMem::GetInstance()->MakeMrnpListPacket((char *)&pSPacket[nOffset]);
				TRACE("Mrnp_List packet info, nCount(%d), nOffset(%d), nSize(%d) ..", nCount, nOffset, nSize);

				UI_AddQueueInfo(UI_CMD_MRS_RESP_LIST, (char *)pSPacket, nSize);

				if(pSPacket != NULL)
				{
					delete pSPacket;
				}
			}
			else
			{	/// 실패
				UI_SND_ERROR_T sPacket;

				::ZeroMemory(&sPacket, sizeof(UI_SND_ERROR_T));
				sPacket.byACK = CHAR_NAK;
				sprintf(sPacket.szErrCode, "%s", CConfigTkMem::GetInstance()->GetErrorCode());

				UI_AddQueueInfo(UI_CMD_MRS_RESP_LIST, (char *)&sPacket, sizeof(UI_SND_ERROR_T));
			}

			SetStep(1);
		}
		break;
	case 1 :
		{

		}
		break;
	}

	return 0;
}

/**
 * @brief		TrMrsKobus_ReqListState
 * @details		예매발권 - 예매리스트 요청 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsKobus_ReqListState(void)
{
	int		nRet = 0, i = 0, nCount = 0;
	BYTE	byACK = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	
			nRet = Kobus_TK_ReadMrs();
			TR_LOG_OUT("코버스 - TMAX:예매리스트 요청, nRet(%d) !!!", nRet);
			if(nRet < 0)
			{	/// 실패
				UI_AddKobus_ReadMrsInfo(FALSE);
			}
			else
			{	/// 성공
				UI_AddKobus_ReadMrsInfo(TRUE);
			}
			SetStep(1);
		}
		break;
	case 1 :
		{	/// idle 상태...

		}
		break;
	}

	return 0;
}

/**
 * @brief		TrMrsTmExp_ReqListState
 * @details		예매발권 - [티머니고속] 예매리스트 요청 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsTmExp_ReqListState(void)
{
	int		nRet = 0, i = 0, nCount = 0;
	BYTE	byACK = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// idle 상태...
			nRet = TmExp_TK_ReadMrs();
			TR_LOG_OUT("티머니고속 - TMAX:예매리스트 요청, nRet(%d) !!!", nRet);
			if(nRet < 0)
			{	/// 실패
				UI_AddQueTmExp(UI_CMD_TMEXP_MRS_RESP_LIST, FALSE);
			}
			else
			{	/// 성공
				UI_AddQueTmExp(UI_CMD_TMEXP_MRS_RESP_LIST, TRUE);
			}
			SetStep(1);
		}
		break;
	case 1 :
		{

		}
		break;
	}

	return 0;
}

/**
 * @brief		TrMrsTmExp_ReqKtcListState
 * @details		예매발권 - [티머니고속] KTC 예매리스트 요청 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsTmExp_ReqKtcListState(void)
{
	int		nRet = 0, i = 0, nCount = 0;
	BYTE	byACK = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// idle 상태...
			nRet = TmExp_TK_KtcReadMrs();
			TR_LOG_OUT("티머니고속 - TMAX: KTC_예매리스트 요청, nRet(%d) !!!", nRet);
			if(nRet < 0)
			{	/// 실패
				//UI_AddQueTmExp(UI_CMD_TMEXP_MRS_RESP_LIST, FALSE);
				UI_AddQueTmExp(UI_CMD_TMEXP_MRS_KTC_RESP_LIST, FALSE);
			}
			else
			{	/// 성공
				UI_AddQueTmExp(UI_CMD_TMEXP_MRS_KTC_RESP_LIST, TRUE);
			}
			SetStep(1);
		}
		break;
	case 1 :
		{

		}
		break;
	}

	return 0;
}

static BOOL cmpKobusPrintList(const TCK_PRINT_FMT_T &p1, const TCK_PRINT_FMT_T &p2)
{
	int n_sats_1, n_sats_2;

	n_sats_1 = Util_Ascii2Long((char *)p1.sats_no, strlen((char *)p1.sats_no));
	n_sats_2 = Util_Ascii2Long((char *)p2.sats_no, strlen((char *)p2.sats_no));

	if(n_sats_1 < n_sats_2)
		return TRUE;
	return FALSE;
}

static BOOL cmpCcsPrintList(const TCK_PRINT_FMT_T &p1, const TCK_PRINT_FMT_T &p2)
{
	int n_sats_1, n_sats_2;

	//	n_sats_1 = *(int *)p1.sats_no;
	//	n_sats_2 = *(int *)p2.sats_no;
	n_sats_1 = Util_Ascii2Long((char *)p1.sats_no, strlen((char *)p1.sats_no));
	n_sats_2 = Util_Ascii2Long((char *)p2.sats_no, strlen((char *)p2.sats_no));

	if(n_sats_1 < n_sats_2)
		return TRUE;
	return FALSE;
}

/**
 * @brief		TrMrsIssueState
 * @details		예매발권 - 발권 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsIssueState(void)
{
	int		nRet = 0, i = 0, nCount = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	// 예매발권(=tmax 138) 데이타 전송
			nRet = Svr_IfSv_138();
			if(nRet >= 0)
			{
				SetStep(1);
			}
			else
			{
				SetStep(2);
				UI_AddFailInfo(UI_CMD_MRS_ISSUE_STATE, CConfigTkMem::GetInstance()->GetErrorCode(), CConfigTkMem::GetInstance()->GetErrorContents());
			}
		}
		break;
	case 1 :
		{	// 예매발권 승차권 출력하기
			BOOL bFirst;
			vector<TCK_PRINT_FMT_T>::iterator	iter;
			char card_aprv_no[100+1];

			CMrnpMem::GetInstance()->MakeTicketPrtData();

			std::sort(CMrnpMem::GetInstance()->m_vtPrtTicket.begin(), CMrnpMem::GetInstance()->m_vtPrtTicket.end(), cmpCcsPrintList);
			bFirst = FALSE;

			for(iter = CMrnpMem::GetInstance()->m_vtPrtTicket.begin(); iter != CMrnpMem::GetInstance()->m_vtPrtTicket.end(); )
			{
				int nSatNo;
				DWORD dwTick;

				::ZeroMemory(card_aprv_no, sizeof(card_aprv_no));
				
				sprintf(card_aprv_no, "%s", iter->card_aprv_no);
				
				nSatNo = Util_Ascii2Long(iter->sats_no, strlen(iter->sats_no));

				nRet = TckPrt_GetStatus();
				TR_LOG_OUT("TckPrt_GetStatus(), nRet(%d), bFirst(%d), nSatNo(%d) ..", nRet, bFirst, nSatNo);
				if(nRet < 0)
				{
					if(bFirst == FALSE)
					{
						/// 0x03 : 프린트 오류
						UI_MrsIssueState(CHAR_ACK, 0x03, nSatNo);
						bFirst = TRUE;
					}
					::Sleep(500);
					continue;
				}

				// 1. 예매발권 상태 데이타 UI 전송
				UI_MrsIssueState(CHAR_ACK, 0x01, nSatNo);
				
				// 2. 승차권 발행	
				TckPrt_MrnpPrintTicket(SVR_DVS_CCBUS, (char *) iter._Ptr);

				dwTick = ::GetTickCount();				
				while( Util_CheckExpire(dwTick) < 3500 );
				
				// 3. 예매발권 상태 데이타 UI 전송
				UI_MrsIssueState(CHAR_ACK, 0x02, nSatNo);

				AddAccumTicketWork(SVR_DVS_CCBUS, IDX_ACC_CARD, iter->bus_tck_knd_cd, (int) ACC_TICKET_MRNP_ISSUE, 1, iter->n_tisu_amt);
				/// 티켓 고유번호 증가
				AddAccumBusTckInhrNo(ACC_TICKET_SEQ_PLUS, SVR_DVS_CCBUS,1);
				iter++;
			}

			/// 20200406 add code
			UI_TckIssueCompleteState(CHAR_ACK, card_aprv_no);

			UI_AddDevAccountInfo();
			SetStep(2);
		}
		break;
	case 2 :
		{
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrMrsKobusIssueState
 * @details		예매발권 - [코버스] 발권 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsKobusIssueState(void)
{
	int		nRet = 0, i = 0, nCount = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	
			///> 코버스 - 예매발행
			nRet = Kobus_TK_PubMrs();
			if(nRet >= 0)
			{
				UI_AddKobus_ResultPubMrsInfo(TRUE);
				SetStep(1);
			}
			else
			{
				UI_AddKobus_ResultPubMrsInfo(FALSE);
				SetStep(99);
			}
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrMrsTmExpIssueState
 * @details		예매발권 - [티머니고속] 예매발권 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsTmExpIssueState(void)
{
	int		nRet = 0, i = 0, nCount = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	
			///> 티머니고속 - 예매발행
			nRet = TmExp_TK_PubMrs();
			if(nRet < 0)
			{
				UI_AddQueTmExp(UI_CMD_TMEXP_MRS_RESP_ISSUE, FALSE);
				SetStep(1);
			}
			else
			{
				UI_AddQueTmExp(UI_CMD_TMEXP_MRS_RESP_ISSUE, TRUE);
				SetStep(99);
			}
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrMrsTmExpMobileIssueState
 * @details		예매발권 - [티머니고속] 모바일티켓 예매발권 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsTmExpMobileIssueState(void)
{
	int		nRet = 0, i = 0, nCount = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	
			///> 티머니고속 - 모바일티켓 예매발행
			nRet = TmExp_TK_PubMrsMobile();
			if(nRet < 0)
			{
				UI_AddQueTmExp(UI_CMD_TMEXP_MRS_RESP_MOBILE_ISSUE, FALSE);
				SetStep(1);
			}
			else
			{
				UI_AddQueTmExp(UI_CMD_TMEXP_MRS_RESP_MOBILE_ISSUE, TRUE);
				SetStep(99);
			}
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrMrsKobusTckPrtState
 * @details		예매발권 - [코버스] 예매권 프린트
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsKobusTckPrtState(void)
{
	int		nRet = 0, i = 0, nCount = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	// 코버스 - 예매 발권 프린트하기
			BOOL bFirst;
			CMrnpKobusMem* pMRS;
			vector<TCK_PRINT_FMT_T>::iterator	iter;
			char card_aprv_no[100+1];

			pMRS = CMrnpKobusMem::GetInstance();
			pMRS->MakeTicketPrtData();

			std::sort(pMRS->m_vtPrtTicket.begin(), pMRS->m_vtPrtTicket.end(), cmpKobusPrintList);
			bFirst = FALSE;

			for(iter = pMRS->m_vtPrtTicket.begin(); iter != pMRS->m_vtPrtTicket.end(); )
			{
				int nSatNo;
				DWORD dwTick;

				::ZeroMemory(card_aprv_no, sizeof(card_aprv_no));

				sprintf(card_aprv_no, "%s", iter->card_aprv_no);

				nSatNo = Util_Ascii2Long(iter->sats_no, strlen(iter->sats_no));

				/// mod code : 2020.05.18
				if(GetConfigTicketPapaer() == PAPER_TICKET)
				{	/// 승차권
					nRet = TckPrt_GetStatus();
					TR_LOG_OUT("TckPrt_GetStatus(), nRet(%d), bFirst(%d), nSatNo(%d) ..", nRet, bFirst, nSatNo);
					if(nRet < 0)
					{
						if(bFirst == FALSE)
						{
							/// 0x03 : 프린트 오류
							UI_MrsIssueState(CHAR_ACK, 0x03, nSatNo);
							bFirst = TRUE;
						}
						::Sleep(500);
						continue;
					}
				}
				else
				{
					nRet = Printer_GetStatus();
					TR_LOG_OUT("Printer_GetStatus(), nRet(%d), bFirst(%d), nSatNo(%d) ..", nRet, bFirst, nSatNo);
					if(nRet < 0)
					{
						if(bFirst == FALSE)
						{
							/// 0x03 : 프린트 오류
							UI_MrsIssueState(CHAR_ACK, 0x03, nSatNo);
							bFirst = TRUE;
						}
						::Sleep(500);
						continue;
					}
				}

				// 1. 예매발권 상태 데이타 UI 전송
				UI_MrsIssueState(CHAR_ACK, 0x01, nSatNo);
				// 2. 승차권 발행	
				/// org code
				///TckPrt_MrnpPrintTicket(SVR_DVS_KOBUS, (char *) iter._Ptr);

				/// mod code : 2020.05.18
				if(GetConfigTicketPapaer() == PAPER_ROLL)
				{	/// 감열지 용지
					Printer_TicketPrint((int)SVR_DVS_KOBUS, (int)enFUNC_MRS, (char *) iter._Ptr);
				}
				else
				{	/// 승차권 용지
					TckPrt_MrnpPrintTicket(SVR_DVS_KOBUS, (char *) iter._Ptr);
				}
				/// ~mod code : 2020.05.18
				dwTick = ::GetTickCount();
				while( Util_CheckExpire(dwTick) < 3500 );

				// 3. 예매발권 상태 데이타 UI 전송
				UI_MrsIssueState(CHAR_ACK, 0x02, nSatNo);

				/// 회계반영 - 승차권 수량
				if(GetConfigTicketPapaer() == PAPER_ROLL)
				{	/// 감열지 용지
					AddAccumTicketWork_Rev1(SVR_DVS_KOBUS, IDX_ACC_CARD, iter->bus_tck_knd_cd, (int) ACC_TICKET_MRNP_REV1_ISSUE, 1, iter->n_tisu_amt, PAPER_ROLL);
				}
				else
				{
					AddAccumTicketWork(SVR_DVS_KOBUS, IDX_ACC_CARD, iter->bus_tck_knd_cd, (int) ACC_TICKET_MRNP_ISSUE, 1, iter->n_tisu_amt);
				}
				/// 티켓 고유번호 증가
				AddAccumBusTckInhrNo(ACC_TICKET_SEQ_PLUS, SVR_DVS_KOBUS, 1);
				iter++;
			}

			/// 20200406 add code
			UI_TckIssueCompleteState(CHAR_ACK, card_aprv_no);

			UI_AddDevAccountInfo();
			SetStep(1);
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrMrsTmExpTckPrtState
 * @details		예매발권 - [티머니고속] 예매권 프린트
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsTmExpTckPrtState(void)
{
	int		nRet = 0, i = 0, nCount = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	// 티머니고속 - 예매 발권 프린트하기
			BOOL bFirst;
			CMrnpTmExpMem* pMRS;
			vector<TCK_PRINT_FMT_T>::iterator	iter;
			char card_aprv_no[100+1];

			pMRS = CMrnpTmExpMem::GetInstance();
			pMRS->MakeTicketPrtData();

			std::sort(pMRS->m_vtPrtTicket.begin(), pMRS->m_vtPrtTicket.end(), cmpKobusPrintList);
			bFirst = FALSE;

			for(iter = pMRS->m_vtPrtTicket.begin(); iter != pMRS->m_vtPrtTicket.end(); )
			{
				int nSatNo;
				DWORD dwTick;

				::ZeroMemory(card_aprv_no, sizeof(card_aprv_no));

				sprintf(card_aprv_no, "%s", iter->card_aprv_no);

				nSatNo = Util_Ascii2Long(iter->sats_no, strlen(iter->sats_no));

				/// mod code : 2020.05.18
				if(GetConfigTicketPapaer() == PAPER_TICKET)
				{	/// 승차권
					nRet = TckPrt_GetStatus();
					TR_LOG_OUT("TckPrt_GetStatus(), nRet(%d), bFirst(%d), nSatNo(%d) ..", nRet, bFirst, nSatNo);
					if(nRet < 0)
					{
						if(bFirst == FALSE)
						{
							/// 0x03 : 프린트 오류
							UI_MrsIssueState(CHAR_ACK, 0x03, nSatNo);
							bFirst = TRUE;
						}
						::Sleep(500);
						continue;
					}
				}
				else
				{
					nRet = Printer_GetStatus();
					TR_LOG_OUT("Printer_GetStatus(), nRet(%d), bFirst(%d), nSatNo(%d) ..", nRet, bFirst, nSatNo);
					if(nRet < 0)
					{
						if(bFirst == FALSE)
						{
							/// 0x03 : 프린트 오류
							UI_MrsIssueState(CHAR_ACK, 0x03, nSatNo);
							bFirst = TRUE;
						}
						::Sleep(500);
						continue;
					}
				}

				// 1. 예매발권 상태 데이타 UI 전송
				UI_MrsIssueState(CHAR_ACK, 0x01, nSatNo);
				// 2. 승차권 발행	
				if(GetConfigTicketPapaer() == PAPER_ROLL)
				{	/// 감열지 용지
					Printer_TicketPrint((int)SVR_DVS_TMEXP, (int)enFUNC_MRS, (char *) iter._Ptr);
				}
				else
				{	/// 승차권 용지
					TckPrt_MrnpPrintTicket(SVR_DVS_TMEXP, (char *) iter._Ptr);
				}
				/// ~mod code : 2020.05.18

				dwTick = ::GetTickCount();
				while( Util_CheckExpire(dwTick) < 3500 );

				// 3. 예매발권 상태 데이타 UI 전송
				UI_MrsIssueState(CHAR_ACK, 0x02, nSatNo);

				/// 회계반영 - 승차권 수량
				if(GetConfigTicketPapaer() == PAPER_ROLL)
				{	/// 감열지 용지
					AddAccumTicketWork_Rev1(SVR_DVS_TMEXP, IDX_ACC_CARD, iter->bus_tck_knd_cd, (int) ACC_TICKET_MRNP_REV1_ISSUE, 1, iter->n_tisu_amt, PAPER_ROLL);
				}
				else
				{
					AddAccumTicketWork(SVR_DVS_TMEXP, IDX_ACC_CARD, iter->bus_tck_knd_cd, (int) ACC_TICKET_MRNP_ISSUE, 1, iter->n_tisu_amt);
				}
				/// 티켓 고유번호 증가
				AddAccumBusTckInhrNo(ACC_TICKET_SEQ_PLUS, SVR_DVS_TMEXP, 1);
				iter++;
			}
			
			/// 20200406 add code
			UI_TckIssueCompleteState(CHAR_ACK, card_aprv_no);
			
			UI_AddDevAccountInfo();
			SetStep(1);
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrMrsCompleteState
 * @details		예매발권 - 발권 완료 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsCompleteState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	
			/// 장비 회계데이타 전송
			UI_AddDevAccountInfo();
			CMrnpMem::GetInstance()->Initialize();
			CMrnpKobusMem::GetInstance()->Initialize();
			CMrnpTmExpMem::GetInstance()->Initialize();
			SetState(TR_READY_STATE, 0);
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrMrsCancelState
 * @details		예매발권 - 발권 취소 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsCancelState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{
			CMrnpMem::GetInstance()->Initialize();
			CMrnpKobusMem::GetInstance()->Initialize();
			CMrnpTmExpMem::GetInstance()->Initialize();
			SetState(TR_READY_STATE, 0);
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrMrsState
 * @details		예매발권
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsState(void)
{
	switch(s_nState)
	{
	case TR_MRS_MAIN_STATE :	
		{	// "예매발권" 시작
			TrMrsMainState();
		}
		break;
	case TR_MRS_CARD_READ_STATE:
		{	// "예매발권" - 신용카드 읽기
			TrMrsCardReadState();
		}
		break;
	case TR_MRS_INPUT_STATE :
		{	// "예매발권" - 예매번호, 핸드폰번호입력
			TrMrsInputState();
		}
		break;
	case TR_MRS_LIST_STATE :
		{	// "예매발권" - [시외] List 내역 요청
			TrMrsReqListState();
		}
		break;
	case TR_MRS_KOBUS_LIST_STATE :
		{	// "예매발권" - [코버스] List 내역 요청
			TrMrsKobus_ReqListState();
		}
		break;
	case TR_MRS_TMEXP_LIST_STATE :
		{	// "예매발권" - [티머니고속] List 내역 요청
			TrMrsTmExp_ReqListState();
		}
		break;
	case TR_MRS_TMEXP_KTC_LIST_STATE :
		{	// "예매발권" - [티머니고속] KTC 예매 내역 요청
			TrMrsTmExp_ReqKtcListState();
		}
		break;
	case TR_MRS_ISSUE_STATE :
		{	// "예매발권" - [시외] TMAX 발권 처리
			TrMrsIssueState();
		}
		break;
	case TR_MRS_KOBUS_ISSUE_STATE :
		{	// "예매발권" - [코버스] TMAX발권 처리
			TrMrsKobusIssueState();
		}
		break;
	case TR_MRS_TMEXP_ISSUE_STATE :
		{	// "예매발권" - [티머니고속] TMAX 예매발권 처리
			TrMrsTmExpIssueState();
		}
		break;
	case TR_MRS_TMEXP_MOBILE_ISSUE_STATE :
		{	// "예매발권" - [티머니고속] TMAX 모바일티켓 예매발권 처리
			TrMrsTmExpMobileIssueState();
		}
		break;
	case TR_MRS_KOBUS_TCK_PRT_STATE :
		{	// "예매발권" - [코버스] 예매권 프린트
			TrMrsKobusTckPrtState();
		}
		break;
	case TR_MRS_TMEXP_TCK_PRT_STATE :
		{	// "예매발권" - [티머니고속] 예매권 프린트
			TrMrsTmExpTckPrtState();
		}
		break;
	case TR_MRS_COMPLETE_STATE :
		{	// "예매발권" - 발권 완료 처리
			TrMrsCompleteState();
		}
		break;
	case TR_MRS_CANCEL_STATE :
		{	// "예매발권" - 발권 취소 처리
			TrMrsCancelState();
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrCancRyMainState
 * @details		환불 - Main
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrCancRyMainState(void)
{
	PDEV_CFG_T		pEnv;
	static DWORD	dwCancyTick = 0L;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	pEnv = (PDEV_CFG_T) GetEnvTicketReaderInfo();

	switch(s_nStep)
	{
	case 0 :
		{	/// idle 상태...
			CCancRyTkMem::GetInstance()->Initialize();
			CCancRyTkKobusMem::GetInstance()->Initialize();
			CCancRyTkTmExpMem::GetInstance()->Initialize();

			if(pEnv->nModel == DEV_SCANNER_ATEC)
			{
				SetStep(998);
				dwCancyTick = ::GetTickCount();
			}
			else
			{
				Scanner_SetTicketRead(TRUE);
				SetStep(999);
			}
		}
		break;
	case 998 :
		{
			int	nStatus = 0, nRet = 0;

			if( Util_CheckExpire(dwCancyTick) < 100 )
			{
				break;
			}

			nRet = Scanner_GetStatus(&nStatus);
			if(nRet >= 0)
			{
				if(pEnv->nModel == DEV_SCANNER_ATEC)
				{
					if( (nStatus & 0x503) == 0 )
					{
						dwCancyTick = 0L;
						Scanner_SetTicketRead(TRUE);
						SetStep(999);
						break;
					}
				}
			}
			dwCancyTick = ::GetTickCount();
		}
		break;

	case 999 :
		{
		}
		break;
	default:
		break;
	}

	return 0;
}

/**
 * @brief		TrCancRyTckReadState
 * @details		환불 - 승차권 읽기
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrCancRyTckReadState(void)
{
	int				nRet = 0;
	static DWORD	dwTick = 0L;
	PDEV_CFG_T		pEnv;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	pEnv = (PDEV_CFG_T) GetEnvTicketReaderInfo();

	switch(s_nStep)
	{
	case 0 :
		{	/// 승차권 읽기
			if(dwTick == 0L)
			{
				dwTick = ::GetTickCount();
			}

			if( Util_CheckExpire(dwTick) < 100 )
			{
				break;
			}

			nRet = Scanner_ReadTicket();
			if(nRet > 0)
			{
				SetStep(1);
				SetCheckEventCode(EC_TICKET_INSERT, TRUE);
			}
			else
			{
				if(nRet == -99)
				{
					Scanner_Reject();
					SetCheckEventCode(EC_TICKET_INSERT, FALSE);
					SetStep(99);
				}
			}
			dwTick = ::GetTickCount();
		}
		break;

	case 1 :
		{	
			BOOL bExpRefund;
			int nSvrKind = 0;
			char szTmpTicket[100];
			char szSecuCode[10];
			POPER_FILE_CONFIG_T	 pConfig;

			pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

			nSvrKind = GetConfigServerKind();
			
			bExpRefund = CConfigTkMem::GetInstance()->m_Refund.bExp;
			if( bExpRefund == TRUE )
			{	/// 고속버스 환불 티켓
				int nLen = 0;
				PFMT_QRCODE_T pQRCode;

				if(nSvrKind & SVR_DVS_KOBUS)
				{	/// 코버스 환불티켓 읽기
					CCancRyTkKobusMem *pCancRy;

					pCancRy = CCancRyTkKobusMem::GetInstance();
					pQRCode = (PFMT_QRCODE_T) &CCancRyTkKobusMem::GetInstance()->tBase.szTicketData;

					/// 코버스_환불 사용유무
					if(pConfig->base_t.exp_refund_func_yn != 'Y')
					{
						TR_LOG_OUT("코버스 티켓 환불 사용안됨..");
						CConfigTkMem::GetInstance()->SetErrorCode("SVE998", (char *)NULL);
						UI_AddRefundInfo(FALSE, 0xFF);
						SetStep(90);
						break;
					}

					///< 발권일자_arr
					nLen = sizeof(pQRCode->pub_dt);
					::CopyMemory(pCancRy->tReq.tissu_dt, pQRCode->pub_dt, nLen);	

					///< 발권터미널번호_arr
					nLen = sizeof(pQRCode->pub_shct_trml_cd);
					::CopyMemory(pCancRy->tReq.tissu_trml_no, pQRCode->pub_shct_trml_cd, nLen);

					pCancRy->tBase.n_tissu_trml_no = Util_Ascii2Long(pCancRy->tReq.tissu_trml_no, 3);

					///< 발권창구번호_arr	
					nLen = sizeof(pQRCode->pub_wnd_no);
					::CopyMemory(pCancRy->tReq.tissu_wnd_no, pQRCode->pub_wnd_no, nLen);	

					///< 발권일련번호_arr	
					nLen = sizeof(pQRCode->pub_sno);
					::CopyMemory(pCancRy->tReq.tissu_sno, pQRCode->pub_sno, nLen);	

					SetStep(3);	   /// 코버스 환불 조회 요청 step
				}
				else
				{	/// 티머니고속 환불티켓 읽기
					CCancRyTkTmExpMem *pCancRy;

					pCancRy = CCancRyTkTmExpMem::GetInstance();
					pQRCode = (PFMT_QRCODE_T) &pCancRy->tBase.szTicketData;

					/// 티머니고속_환불 사용유무
					if(pConfig->base_t.exp_refund_func_yn != 'Y')
					{
						TR_LOG_OUT("티머니고속 티켓 환불 사용안됨..");
						CConfigTkMem::GetInstance()->SetErrorCode("SVE998", (char *)NULL);
						UI_AddRefundInfo(FALSE, 0xFF);
						SetStep(90);
						break;
					}

					///< 발권일자_arr
					nLen = sizeof(pQRCode->pub_dt);
					::CopyMemory(pCancRy->tReqInq.tissu_dt, pQRCode->pub_dt, nLen);	

					///< 발권터미널번호_arr
					nLen = sizeof(pQRCode->pub_shct_trml_cd);
					::CopyMemory(pCancRy->tReqInq.tissu_trml_no, pQRCode->pub_shct_trml_cd, nLen);

					pCancRy->tBase.n_tissu_trml_no = Util_Ascii2Long(pCancRy->tReqInq.tissu_trml_no, 3);

					///< 발권창구번호_arr	
					nLen = sizeof(pQRCode->pub_wnd_no);
					::CopyMemory(pCancRy->tReqInq.tissu_wnd_no, pQRCode->pub_wnd_no, nLen);	

					///< 발권일련번호_arr	
					nLen = sizeof(pQRCode->pub_sno);
					::CopyMemory(pCancRy->tReqInq.tissu_sno, pQRCode->pub_sno, nLen);	

					SetStep(4);	  /// 티머니고속 환불 조회 요청 step
				}
			}
			else 
			{	/// Bar Code (20 byte)
				PFMT_BARCODE_T pBarCode;

				/// 시외버스_환불 사용유무
				if(pConfig->base_t.ccs_refund_func_yn != 'Y')
				{
					TR_LOG_OUT("시외버스 티켓 환불 사용안됨..");
					CConfigTkMem::GetInstance()->SetErrorCode("SVE998", (char *)NULL);
					UI_AddRefundInfo(FALSE, 0xFF);
					SetStep(90);
					break;
				}

				/// 승차권 데이타 체크 : 발행일자(8)-단축터미널코드(4)-창구번호(2)-발행일련번호(4)-SecurityCode(2)
				pBarCode = (PFMT_BARCODE_T) &CCancRyTkMem::GetInstance()->szTicketData;

				sprintf(szTmpTicket, "%.*s%.*s%.*s%.*s", 
					sizeof(pBarCode->pub_dt),			pBarCode->pub_dt, 
					sizeof(pBarCode->pub_shct_trml_cd), pBarCode->pub_shct_trml_cd, 
					sizeof(pBarCode->pub_wnd_no),		pBarCode->pub_wnd_no, 
					sizeof(pBarCode->pub_sno),			pBarCode->pub_sno	);
				
				::ZeroMemory(szSecuCode, sizeof(szSecuCode));
				GetTicketSecurityCode((BYTE *)szTmpTicket, strlen(szTmpTicket), szSecuCode);

				TR_LOG_OUT("바코드 - 체크섬값(%s), 계산한값(%s)", pBarCode->secu_code, szSecuCode);

				if( memcmp(szSecuCode, pBarCode->secu_code, sizeof(pBarCode->secu_code)) == 0 )
				{
					int n_sno;
					CCancRyTkMem* pCancRy;

					pCancRy = CCancRyTkMem::GetInstance();

					::ZeroMemory(&pCancRy->tReqTckNo, sizeof(REQ_TICKET_NO_T));

					/// 발행시스템 구분코드
					pCancRy->tReqTckNo.pub_sys_dvs_cd[0] = 'K';
					/// 발행일자
					::CopyMemory(pCancRy->tReqTckNo.pub_dt, pBarCode->pub_dt, sizeof(pBarCode->pub_dt));
					/// 발행단축터미널코드
					::CopyMemory(pCancRy->tReqTckNo.pub_shct_trml_cd, pBarCode->pub_shct_trml_cd, sizeof(pBarCode->pub_shct_trml_cd));
					/// 발행창구번호
					::CopyMemory(pCancRy->tReqTckNo.pub_wnd_no, pBarCode->pub_wnd_no, sizeof(pBarCode->pub_wnd_no));
					/// 발행일련번호
					n_sno = Util_Ascii2Long(pBarCode->pub_sno, sizeof(pBarCode->pub_sno));
					::CopyMemory(pCancRy->tReqTckNo.pub_sno, &n_sno, sizeof(pBarCode->pub_sno));
					/// EB 버스티켓 일련번호
					::ZeroMemory(pCancRy->tReqTckNo.eb_bus_tck_sno, sizeof(pCancRy->tReqTckNo.eb_bus_tck_sno));

					/// 출발지 다르면 환불 불가
					if( memcmp(pCancRy->tReqTckNo.pub_shct_trml_cd, GetShortTrmlCode(SVR_DVS_CCBUS), 4) )
					{
						TR_LOG_OUT("출발지 다름, 터미널코드(%s):(%s).. ", pCancRy->tReqTckNo.pub_shct_trml_cd, GetTrmlCode(SVR_DVS_CCBUS));
						/// 발권기에서 환불 불가능한 승차권입니다. 창구에 문의하세요.
						CConfigTkMem::GetInstance()->SetErrorCode("SVE998", (char *)NULL);
						UI_AddRefundInfo(FALSE, 0xFF);
						SetStep(90);
					}
					else
					{
						SetStep(2);	
					}
				}
				else
				{
					TR_LOG_OUT("바코드 체크섬 에러.. ");
					CConfigTkMem::GetInstance()->SetErrorCode("SVE998", (char *)NULL);
					UI_AddRefundInfo(FALSE, 0xFF);
					SetStep(90);
				}
			}
		}
		break;
	case 2 :
		{	/// 시외버스 - 티켓 정상, 티맥스 서버에 조회 요청
			nRet = Svr_IfSv_158();
			if(nRet > 0)
			{
				nRet = CCancRyTkMem::GetInstance()->CalcRefund();
				TR_LOG_OUT("CalcRefund() - nRet(%d)..", nRet);
				if(nRet < 0)
				{
					if(nRet == -99 )
					{
						/// 환불이 불가능한 승차권입니다. 창구에 문의하세요...
						CConfigTkMem::GetInstance()->SetErrorCode("SVE998", (char *)NULL);
					}
					else
					{
						/// 환불이 불가능한 승차권입니다.
						CConfigTkMem::GetInstance()->SetErrorCode("SVE997", (char *)NULL);
					}
					UI_AddRefundInfo(FALSE, 0xFF);
					SetStep(90);
				}
				else
				{
					UI_AddRefundInfo(TRUE, UI_BUS_DVS_CCS);
					SetStep(10);
				}
			}
			else
			{
				UI_AddRefundInfo(FALSE, 0xFF);
				SetStep(90);
			}
		}
		break;

	case 3 :
		{	/// 코버스 - 환불 조회
			nRet = Kobus_TK_RyAmtInfo();
			if(nRet > 0)
			{
				nRet = CCancRyTkKobusMem::GetInstance()->CalcRefund();
				TR_LOG_OUT("CalcRefund() - nRet(%d)..", nRet);
				if(nRet < 0)
				{
					if(nRet == -99 )
					{
						/// 환불이 불가능한 승차권입니다. 창구에 문의하세요...
						CConfigTkMem::GetInstance()->SetErrorCode("SVE998", (char *)NULL);
					}
					else
					{
						/// 환불이 불가능한 승차권입니다.
						CConfigTkMem::GetInstance()->SetErrorCode("SVE997", (char *)NULL);
					}
					UI_AddRefundInfo(FALSE, 0xFF);
					SetStep(90);
				}
				else
				{
					UI_AddRefundInfo(TRUE, UI_BUS_DVS_KOBUS);
					SetStep(10);
				}
			}
			else if(nRet == -3)	// 20210723 ADD
			{
				SetStep(0);
			}					// 20210723 ~ADD
			else
			{
				UI_AddRefundInfo(FALSE, 0xFF);
				SetStep(90);
			}
		}
		break;

	case 4 :
		{	/// 티머니고속 환불 조회	(최초조회)
			nRet = TmExp_TK_ReadBusTckno(TRUE);
			if(nRet > 0)
			{
				nRet = CCancRyTkTmExpMem::GetInstance()->CheckRefund();
				TR_LOG_OUT("CheckRefund() - nRet(%d)..", nRet);
				if(nRet < 0)
				{
					if(nRet == -99)
					{
						/// 환불이 불가능한 승차권입니다. 창구에 문의하세요...
						CConfigTkMem::GetInstance()->SetErrorCode("SVE998", (char *)NULL);
					}
					else
					{
						/// 환불이 불가능한 승차권입니다.
						CConfigTkMem::GetInstance()->SetErrorCode("SVE997", (char *)NULL);
					}
					UI_AddRefundInfo(FALSE, 0xFF);
					SetStep(90);
				}
				else
				{
					SetStep(5);
				}
			}
			else
			{
				UI_AddRefundInfo(FALSE, 0xFF);
				SetStep(90);
			}
		}
		break;

	case 5 :
		{	/// 티머니고속 환불 계산..
			nRet = CCancRyTkTmExpMem::GetInstance()->CalcRefund();
			TR_LOG_OUT("CalcRefund() - nRet(%d)..", nRet);
			if(nRet < 0)
			{
				if(nRet == -99)
				{
					/// 환불이 불가능한 승차권입니다. 창구에 문의하세요...
					CConfigTkMem::GetInstance()->SetErrorCode("SVE998", (char *)NULL);
				}
				else
				{
					/// 환불이 불가능한 승차권입니다.
					CConfigTkMem::GetInstance()->SetErrorCode("SVE997", (char *)NULL);
				}
				UI_AddRefundInfo(FALSE, 0xFF);
				SetStep(90);
				break;
			}

			/// 티머니고속 환불 조회 (2번째 조회)
			nRet = TmExp_TK_ReadBusTckno(FALSE);
			if(nRet > 0)
			{
				/// 환불금액
				//::CopyMemory( &CCancRyTkTmExpMem::GetInstance()->tBase.n_tot_money, 
				//			  CCancRyTkTmExpMem::GetInstance()->tRespInq.ry_amt, 4 );
				/// 환불차익
				::CopyMemory( &CCancRyTkTmExpMem::GetInstance()->tBase.n_commission_fare,	
							  CCancRyTkTmExpMem::GetInstance()->tRespInq.ry_pfit, 4 );

				TR_LOG_OUT("[티머니고속 환불 조회 최종 정보]");
				TR_LOG_OUT("1. 환불금액    = %d", CCancRyTkTmExpMem::GetInstance()->tBase.n_tot_money);
				TR_LOG_OUT("2. 환불차익    = %d", CCancRyTkTmExpMem::GetInstance()->tBase.n_commission_fare);
				TR_LOG_OUT("3. 할인반환금액 = %d", CCancRyTkTmExpMem::GetInstance()->tBase.n_commission_rate);

				UI_AddRefundInfo(TRUE, UI_BUS_DVS_TMEXP);
				SetStep(10);
			}
			else
			{
				UI_AddRefundInfo(FALSE, 0xFF);
				SetStep(90);
				break;
			}

			/****
			nRet = TmExp_TK_ReadBusTckno(FALSE);
			if(nRet > 0)
			{
				nRet = CCancRyTkTmExpMem::GetInstance()->CalcRefund();
				TR_LOG_OUT("CalcRefund() - nRet(%d)..", nRet);
				if(nRet < 0)
				{
					if(nRet == -99)
					{
						/// 환불이 불가능한 승차권입니다. 창구에 문의하세요...
						CConfigTkMem::GetInstance()->SetErrorCode("SVE998", (char *)NULL);
					}
					else
					{
						/// 환불이 불가능한 승차권입니다.
						CConfigTkMem::GetInstance()->SetErrorCode("SVE997", (char *)NULL);
					}
					UI_AddRefundInfo(FALSE, 0xFF);
					SetStep(90);
				}
				else
				{
					UI_AddRefundInfo(TRUE, UI_BUS_DVS_TMEXP);
					SetStep(10);
				}
			}
			else
			{
				UI_AddRefundInfo(FALSE, 0xFF);
				SetStep(90);
			}
			***/
		}
		break;

	case 90 :
		{
			nRet = Scanner_Reject();
			TR_LOG_OUT("Scanner_Reject(), nRet(%d) !!", nRet);
			SetCheckEventCode(EC_TICKET_INSERT, FALSE);
			SetStep(99);
		}
		break;

	case 99 :
		{	// reject
			int nStatus = 0;

			nRet = Scanner_GetStatus(&nStatus);
			//TR_LOG_OUT("@@@@@@@> Scanner_GetStatus(), nRet(%d), nStatus(%X) ", nRet, nStatus & 0xFFFFFFFF);
			if(nRet >= 0)
			{
				if(pEnv->nModel == DEV_SCANNER_WT)
				{
					if( ((nStatus & 0xFFFFFFFF) == 0x180) )
					{
						TR_LOG_OUT("WT_Scanner_SetTicketRead(TRUE) !!!");
						Scanner_SetTicketRead(TRUE);
						SetStep(0);
					}
					else
					{
						TR_LOG_OUT("WT_Scanner_SetTicketRead(FALSE) !!!");
						Scanner_SetTicketRead(FALSE);
						SetStep(100);
					}
				}
				else if(pEnv->nModel == DEV_SCANNER_ATEC)
				{
					//if( (nStatus & 0xFFFFFFFF) == 0 )
					if( (nStatus & 0x503) == 0 )
					{
						// scan start..
						TR_LOG_OUT("@@@@@@@> Scanner_GetStatus(), nStatus(%X) ", nStatus & 0xFFFFFFFF);
						Scanner_SetTicketRead(TRUE);
						SetStep(0);
					}
					else if( ((nStatus & 0xFFFFFFFF) & 0x503) )	 // 입구센서  (bit : 0,1,8,10)
					{
						// scan stop
						//Scanner_SetTicketRead(FALSE);
					}
				}
			}
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrCancRyFareState
 * @details		환불 - 카드 또는 현금 환불 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrCancRyFareState(void)
{
	int				nRet = 0;
	PDEV_CFG_T		pEnv = NULL;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{
			BOOL bExpRefund;
			int nSvrKind = 0;

			nSvrKind = GetConfigServerKind();
			//bStart_KOBUS = CCancRyTkKobusMem::GetInstance()->tBase.bStart;
			bExpRefund = CConfigTkMem::GetInstance()->m_Refund.bExp;
			if(bExpRefund == TRUE)
			{
				if(nSvrKind & SVR_DVS_KOBUS)
				{	/// kobus express
					CCancRyTkKobusMem* pCancRy;
		
					pCancRy = CCancRyTkKobusMem::GetInstance();
					if(pCancRy->tBase.n_tissu_trml_no < 900)
					{	/// 무인기에서 발권한거...
						nRet = Kobus_TK_RepTran();
						TR_LOG_OUT("Kobus_TK_RepTran(), nRet(%d) ...", nRet);
					}
					else
					{	/// 창구에서 발권한거..
						nRet = Kobus_TK_TckCan();
						TR_LOG_OUT("Kobus_TK_TckCan(), nRet(%d) ...", nRet);
					}
					if(nRet > 0)
					{
						TR_LOG_OUT("코버스 TMAX 환불 처리 성공 !!!");
						SetStep(1);
					}
					else
					{	// 승차권 reject
						TR_LOG_OUT("TMAX 환불 처리 실패 !!!");
						{
							UI_SND_ERROR_T Info;

							::ZeroMemory(&Info, sizeof(UI_SND_FAIL_INFO_T));

							Info.byACK = CHAR_NAK;		///> 
							sprintf(Info.szErrCode, "%s", CConfigTkMem::GetInstance()->GetErrorCode());
							UI_AddQueueInfo(UI_CMD_CANCRY_FARE_INFO, (char *)&Info, sizeof(UI_SND_ERROR_T));
						}
						nRet = Scanner_Reject();

						SetCheckEventCode(EC_TICKET_INSERT, FALSE);
						//Scanner_SetTicketRead(TRUE);
						SetStep(4);
					}
				}
				else
				{	/// Tmoney express
					CCancRyTkTmExpMem* pCancRy;
		
					pCancRy = CCancRyTkTmExpMem::GetInstance();
					nRet = TmExp_TK_CancRyTck();						
					if(nRet > 0)
					{
						TR_LOG_OUT("티머니고속 TMAX 환불 처리 성공 !!!");
						SetStep(1);
					}
					else
					{	// 승차권 reject
						TR_LOG_OUT("TMAX 환불 처리 실패 !!!");
						{
							UI_SND_ERROR_T Info;

							::ZeroMemory(&Info, sizeof(UI_SND_FAIL_INFO_T));

							Info.byACK = CHAR_NAK;		///> 
							sprintf(Info.szErrCode, "%s", CConfigTkMem::GetInstance()->GetErrorCode());
							UI_AddQueueInfo(UI_CMD_CANCRY_FARE_INFO, (char *)&Info, sizeof(UI_SND_ERROR_T));
						}
						nRet = Scanner_Reject();
						SetCheckEventCode(EC_TICKET_INSERT, FALSE);
						//Scanner_SetTicketRead(TRUE);
						SetStep(4);
					}
				}
			}
			else
			{	/// country-city bus refund
				nRet = Svr_IfSv_145();
				if(nRet > 0)
				{
					TR_LOG_OUT("시외버스 TMAX 환불 처리 성공 !!!");
					SetStep(1);
				}
				else
				{	// 승차권 reject
					TR_LOG_OUT("TMAX 환불 처리 실패 !!!");
					{
						UI_SND_ERROR_T Info;

						::ZeroMemory(&Info, sizeof(UI_SND_ERROR_T));

						Info.byACK = CHAR_NAK;		///> 
						sprintf(Info.szErrCode, "%s", CConfigTkMem::GetInstance()->GetErrorCode());
						UI_AddQueueInfo(UI_CMD_CANCRY_FARE_INFO, (char *)&Info, sizeof(UI_SND_ERROR_T));
					}
					nRet = Scanner_Reject();

					//SetCheckEventCode(EC_TICKET_INSERT, FALSE);
					//Scanner_SetTicketRead(TRUE);
					SetStep(4);
				}
			}
		}
		break;
	case 1 :
		{	///
			BYTE ui_pym_dvs_cd = 0;
			int n_chg_money = 0;

			if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_CCBUS )
			{	/// 시외버스 - 환불
				CCancRyTkMem* pCancTr;

				TR_LOG_OUT("[시외버스] 환불 처리 !!!");

				pCancTr = CCancRyTkMem::GetInstance();

				ui_pym_dvs_cd = pCancTr->ui_pym_dvs_cd[0];
				n_chg_money = CCancRyTkMem::GetInstance()->n_chg_money;

				// 회계반영 - 승차권정보
				{
					int n_tisu_amt = 0;
					int nAccPymDvs;

					if(ui_pym_dvs_cd == 1)
					{
						nAccPymDvs = IDX_ACC_CASH;
					}
					else
					{
						nAccPymDvs = IDX_ACC_CARD;
					}

					n_tisu_amt = *(int *) pCancTr->tRespTckNo.tisu_amt;
					AddAccumTicketWork(SVR_DVS_CCBUS, nAccPymDvs, "00", (int) ACC_TICKET_REFUND, 1, n_tisu_amt - pCancTr->n_commission_fare);
				}
			}
			else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_KOBUS )
			{	/// 코버스 - 환불
				CCancRyTkKobusMem* pCancTr;

				TR_LOG_OUT("[코버스] 환불 처리 !!!");

				pCancTr = CCancRyTkKobusMem::GetInstance();

				ui_pym_dvs_cd	= pCancTr->tBase.ui_pym_dvs_cd[0];
				n_chg_money		= pCancTr->tBase.n_chg_money;

				// 회계반영 - 승차권정보
				{
					int n_tisu_amt = 0;
					int nAccPymDvs;

					if(ui_pym_dvs_cd == 1)
					{
						nAccPymDvs = IDX_ACC_CASH;
					}
					else
					{
						nAccPymDvs = IDX_ACC_CARD;
					}

					n_tisu_amt = *(int *) pCancTr->tRespInq.tissu_fee;
					AddAccumTicketWork(SVR_DVS_KOBUS, nAccPymDvs, "00", (int) ACC_TICKET_REFUND, 1, n_tisu_amt - pCancTr->tBase.n_commission_fare);
				}
			}
			else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_TMEXP )
			{	/// 티머니고속 - 환불
				int nCount = 0;
				CCancRyTkTmExpMem* pCancTr;
				vector<rtk_tm_cancrytck_list_t>::iterator	iter;

				TR_LOG_OUT("[티머니고속] 환불 처리 !!!");

				pCancTr = CCancRyTkTmExpMem::GetInstance();

				TR_LOG_OUT("@@@@@ pCancTr->tBase.ui_pym_dvs_cd[0] = 0x%02X ", pCancTr->tBase.ui_pym_dvs_cd[0]);
				TR_LOG_OUT("@@@@@ ui_pym_dvs_cd = 0x%02X ", ui_pym_dvs_cd);

				ui_pym_dvs_cd	= pCancTr->tBase.ui_pym_dvs_cd[0];
				n_chg_money		= pCancTr->tBase.n_chg_money;

				// 회계반영 - 승차권정보
				{
					int n_tisu_amt = 0;
					int nAccPymDvs;

					if(ui_pym_dvs_cd == 1)
					{
						nAccPymDvs = IDX_ACC_CASH;
					}
					else
					{
						nAccPymDvs = IDX_ACC_CARD;
					}

					nCount = pCancTr->m_vtRespRefList.size();
					if(nCount > 0)
					{
						iter = pCancTr->m_vtRespRefList.begin();
						n_tisu_amt = *(int *)iter->ry_amt;
						AddAccumTicketWork(SVR_DVS_TMEXP, nAccPymDvs, "00", (int) ACC_TICKET_REFUND, 1, n_tisu_amt - pCancTr->tBase.n_commission_fare);
					}
					else
					{
						TR_LOG_OUT("[티머니고속] 환불 데이타 저장 못함 !!!");	
					}
				}
			}
			else
			{
				TR_LOG_OUT("[버스코드_에러 ????] 환불 안함 !");
				SetStep(2);
				break;
			}

			if(ui_pym_dvs_cd == 1)
			{	/// 현금 일 경우
				int nRefundFare;
				int nTmp10k, nTmp1k, nTmp100, nTmp500;
				int nBillACK, nCoinACK;
				int nOut100, nOut500;
				UI_SND_CANCRY_CHG_MONEY_T Info;
				DISPENSE_INFO_T dspInfo;
				char receipt_yn[1];

				nTmp10k = nTmp1k = nTmp100 = nTmp500 = 0;
				nOut100 = nOut500 = 0;
				nBillACK = nCoinACK = CHAR_ACK;
				::ZeroMemory(&dspInfo, sizeof(DISPENSE_INFO_T));

				::ZeroMemory(&Info, sizeof(UI_SND_CANCRY_CHG_MONEY_T));

				// 방출금액
				nRefundFare = n_chg_money;

				TR_LOG_OUT("## 현금환불 ## ");

				TR_LOG_OUT("[01]. nRefundFare = %d ", nRefundFare);

				if( INI_Use10K() )
				{
					nTmp10k = nRefundFare / 10000;
					nRefundFare -= (10000 * nTmp10k);
					TR_LOG_OUT("[02]. nRefundFare = %d, nTmp10k = %d ", nRefundFare, nTmp10k);
				}

				nTmp1k = nRefundFare / 1000;
				nRefundFare -= (1000 * nTmp1k);
				TR_LOG_OUT("[03]. nRefundFare = %d, nTmp1k = %d ", nRefundFare, nTmp1k);

				nTmp500 = nRefundFare / 500;
				nRefundFare -= (500 * nTmp500);
				TR_LOG_OUT("[04]. nRefundFare = %d, nTmp500 = %d ", nRefundFare, nTmp500);

				nTmp100 = nRefundFare / 100;
				nRefundFare -= (100 * nTmp100);
				TR_LOG_OUT("[05]. nRefundFare = %d, nTmp100 = %d ", nRefundFare, nTmp100);

				// 지폐방출기
				if( (nTmp10k + nTmp1k) > 0 )
				{
					::ZeroMemory(&dspInfo, sizeof(DISPENSE_INFO_T));

					nRet = CDU_Dispense(nTmp1k, nTmp10k);
					if(nRet < 0)
					{
						nBillACK = Info.byAck = CHAR_NAK;		///> 
					}
					else
					{
						nBillACK = Info.byAck = CHAR_ACK;		///> 
					}
				
					CDU_GetDispenseInfo((char *)&dspInfo);

					/// 회계반영
					if( (dspInfo.nOutCount[0] + dspInfo.nOutCount[1]) > 0 )
					{
						AddAccumBillData(ACC_BILL_MINUS, dspInfo.nOutCount[0], 0, dspInfo.nOutCount[1], 0);
					}
					
					if(Info.byAck == CHAR_NAK)
					{	/// 미방출시 방출된 금액만큼만 저장
						if( (dspInfo.nOutCount[0] + dspInfo.nOutCount[1]) > 0)
						{
							/// 입출내역 TR (출금)
							TR_LOG_OUT("[#01] 미방출시 - 지폐방출 (%4d : %4d) ", dspInfo.nOutCount[0], dspInfo.nOutCount[1]);
							//AddCashTrData(DT_FLAG_OUT_MONEY, (WORD)0, (WORD)0, (WORD)dspInfo.nOutCount[0], 0, (WORD)dspInfo.nOutCount[1], 0);
						}
					}

					if( GetEventCode(EC_CDU_NO_OUT) )
					{
						int n1k, n10k;

 						n1k  = nTmp1k - dspInfo.nOutCount[0];
 						n10k = nTmp10k - dspInfo.nOutCount[1];
// 						n1k  = dspInfo.nOutCount[0] - nTmp1k;
// 						n10k = dspInfo.nOutCount[1] - nTmp10k;
						AddAccumUnPaidData(ACC_UNPAID, 0, 0, n1k, n10k);
						
						/// 입출내역 TR (미출금)
						TR_LOG_OUT("[#02] 미방출 지폐 (%4d : %4d) ", n1k, n10k);
						//AddCashTrData(DT_FLAG_UNPAID, (WORD)0, (WORD)0, (WORD)n1k, 0, (WORD)n10k, 0);
					}
				}

// 				if( Info.byAck == CHAR_NAK )
// 				{
// 					UI_AddFailInfo(UI_CMD_CANCRY_FARE_INFO, "LOC997", NULL);
// 					UI_AddDevAccountInfo();
// 					SetStep(2);
// 					break;
// 				}

				// 동전방출기
				if( (nTmp500 + nTmp100) > 0 )
				{
					nRet = Coin_ChangeMoney(nTmp100, nTmp500);
					if(nRet < 0)
					{
						nCoinACK = Info.byAck = CHAR_NAK;
					}
					else
					{
						nCoinACK = Info.byAck = CHAR_ACK;
					}

					Coin_GetOutInfo(&nOut100, &nOut500);
					Info.w100 = (WORD) nOut100;		///> 동전 - 100원
					Info.w500 = (WORD) nOut500;		///> 동전 - 500원
			
					Coin_TotalEnd();

					/// 회계반영
					if( (Info.w100 + Info.w500) > 0 )
					{
						AddAccumCoinData(ACC_COIN_MINUS, Info.w100, Info.w500);
					}

					if( Info.byAck == CHAR_NAK )
					{	/// 미방출시 방출된 금액만큼만 저장
						if( (Info.w100 + Info.w500) > 0 )
						{	
							/// 입출내역 TR (출금)
							TR_LOG_OUT("[#03] 미방출시 - 동전방출 (%4d : %4d) ", Info.w100, Info.w500);
							//AddCashTrData(DT_FLAG_OUT_MONEY, (WORD)Info.w100, (WORD)Info.w500, 0, 0, 0, 0);
						}
					}

					if( GetEventCode(EC_COIN_NO_OUT) )
					{
						int n100, n500;

						n100 = nTmp100 - nOut100;
						n500 = nTmp500 - nOut500;

						/// 회계반영
						AddAccumUnPaidData(ACC_UNPAID, n100, n500, 0, 0);

						/// 입출내역 TR (미출금)
						TR_LOG_OUT("[#04] 미방출 동전 (%4d : %4d) ", n100, n500);
						//AddCashTrData(DT_FLAG_UNPAID, (WORD)n100, (WORD)n500, 0, 0, 0, 0);
					}
				}

//				if(	Info.byAck == CHAR_ACK)
				if(	(nBillACK == CHAR_ACK) && (nCoinACK == CHAR_ACK) )
				{
					//Info.pyn_mns_dvs_cd[0]	= 1;	// 20210823 DEL
					Info.pyn_mns_dvs_cd[0]	= 0x01;		// 20210823 MOD
					Info.w1k = nTmp1k;			///> 지폐 - 1,000원
					Info.w5k = 0;				///> 지폐 - 5,000원
					Info.w10k = nTmp10k;		///> 지폐 - 10,000원
					Info.w50k = 0;				///> 지폐 - 50,000원
					Info.w10 = 0;				///> 동전 - 10원
					Info.w50 = 0;				///> 동전 - 50원
					Info.w100 = nTmp100;		///> 동전 - 100원
					Info.w500 = nTmp500;		///> 동전 - 500원

					TR_LOG_OUT("[환불] 현금 방출 성공...");

					/// 현금 환불 정보 전송
					//UI_AddQueueInfo(UI_CMD_CANCRY_FARE_INFO, (char *)&Info, sizeof(UI_SND_CANCRY_CHG_MONEY_T));

					/// 승차권 소인처리
					nRet = IsConfigTPHPrint();
					TR_LOG_OUT("IsConfigTPHPrint(), nRet(%d) ", nRet);
					if( nRet > 0 )
					{
						nRet = Scanner_TPHPrint();
						TR_LOG_OUT("Scanner_TPHPrint(), nRet(%d) ", nRet);
						if(nRet != 0)
						{
							/// error
							TR_LOG_OUT("Scanner_TPHPrint(), error, nRet(%d) ", nRet);
						}
					}
					/// 승차권함으로 이동
					nRet = Scanner_Eject();
					TR_LOG_OUT("Scanner_Eject(), nRet(%d) ", nRet);
					
					/// 입출내역 TR (출금)
					TR_LOG_OUT("[#05] 방출 갯수 (%4d : %4d), (%4d : %4d) ", nTmp100, nTmp500, nTmp1k, nTmp10k);
					AddCashTrData(DT_FLAG_OUT_MONEY, (WORD)nTmp100, (WORD)nTmp500, (WORD)nTmp1k, 0, (WORD)nTmp10k, 0);

					/// 회계반영 - 승차권 수집함
					AddAccumTicketData(ACC_TICKETBOX_INSERT, 1);

					
					/// 영수증 출력
					{
						int nSvrKind;

						nSvrKind = GetConfigServerKind();
						if( TRUE == CConfigTkMem::GetInstance()->m_Refund.bExp )
						{
							if(nSvrKind & SVR_DVS_KOBUS)
							{
								receipt_yn[0] = CCancRyTkKobusMem::GetInstance()->tBase.receipt_yn[0];	
							}
							else
							{
								receipt_yn[0] = CCancRyTkTmExpMem::GetInstance()->tBase.receipt_yn[0];	
							}
						}
						else
						{
							receipt_yn[0] = CCancRyTkMem::GetInstance()->receipt_yn[0];
						}
					}
					TR_LOG_OUT("receipt_yn[0] = (%c) ", receipt_yn[0] & 0xFF);

					if( receipt_yn[0] == 'Y' )
					{
						nRet = Printer_RefundTicket();
						Sleep(1000 * 2);
					}

					/// 환불 - 현금 방출 정보 전송
					UI_AddQueueInfo(UI_CMD_CANCRY_FARE_INFO, (char *)&Info, sizeof(UI_SND_CANCRY_CHG_MONEY_T));
				}
				else
				{
					// 실패
					TR_LOG_OUT("[환불] 현금 방출 실패...");
					
					/// 승차권 소인처리
					nRet = IsConfigTPHPrint();
					TR_LOG_OUT("IsConfigTPHPrint(), nRet(%d) ", nRet);
					if( nRet > 0 )
					{
						nRet = Scanner_TPHPrint();
						TR_LOG_OUT("Scanner_TPHPrint(), nRet(%d) ", nRet);
						if(nRet != 0)
						{
							/// error
							TR_LOG_OUT("Scanner_TPHPrint(), error, nRet(%d) ", nRet);
						}
					}
					
					/// 승차권함으로 이동
					nRet = Scanner_Eject();
					TR_LOG_OUT("Scanner_Eject(), nRet(%d) ", nRet);

					// 입출내역
					AddCashTrData(DT_FLAG_OUT_MONEY, (WORD)nOut100, (WORD)nOut500, (WORD)dspInfo.nOutCount[0], 0, (WORD)dspInfo.nOutCount[1], 0);
					
					/// 영수증 출력
					if( TRUE == CCancRyTkKobusMem::GetInstance()->tBase.bStart )
					{
						receipt_yn[0] = CCancRyTkKobusMem::GetInstance()->tBase.receipt_yn[0];	
					}
					else
					{
						receipt_yn[0] = CCancRyTkMem::GetInstance()->receipt_yn[0];
					}

					TR_LOG_OUT("receipt_yn[0] = (%c) ", receipt_yn[0] & 0xFF);

					if( receipt_yn[0] == 'Y' )
					{
						nRet = Printer_RefundTicket();
						Sleep(1000 * 2);
					}
					
					// 환불 - 현금 미방출 정보 전송
					UI_AddFailInfo(UI_CMD_CANCRY_FARE_INFO, "SVE555", NULL);
				}

				UI_AddDevAccountInfo();
				SetStep(2);
			}
			else
			{	/// 카드 일 경우
				UI_SND_CANCRY_CHG_MONEY_T Info;
				char receipt_yn[1];

				TR_LOG_OUT("## 카드환불 ## ");

				::ZeroMemory(&Info, sizeof(UI_SND_CANCRY_CHG_MONEY_T));

				Info.byAck = CHAR_ACK;		///> 
				//Info.pyn_mns_dvs_cd[0]	= 2;	// 20210823 DEL
				Info.pyn_mns_dvs_cd[0]	= 0x02;		// 20210823 MOD
				
				// 20211013 ADD
				if(ui_pym_dvs_cd == 5)	// 1:현금, 2:카드, 5:복합결제
					Info.pyn_mns_dvs_cd[0]	= 0x05;
				// 20211013 ~ADD

				//UI_AddQueueInfo(UI_CMD_CANCRY_FARE_INFO, (char *)&Info, sizeof(UI_SND_CANCRY_CHG_MONEY_T));

				nRet = IsConfigTPHPrint();
				TR_LOG_OUT("IsConfigTPHPrint(), nRet(%d) ", nRet);
				if( nRet > 0 )
				{
					/// 승차권 소인처리
					nRet = Scanner_TPHPrint();
					TR_LOG_OUT("Scanner_TPHPrint(), nRet(%d) ", nRet);
				}

				/// 승차권함으로 이동
				nRet = Scanner_Eject();
				TR_LOG_OUT("Scanner_Eject(), nRet(%d) ", nRet);

				/// 회계반영 - 승차권 수집함
				AddAccumTicketData(ACC_TICKETBOX_INSERT, 1);

				/// 영수증 출력
				{
					int nSvrKind;

					nSvrKind = GetConfigServerKind();
					if( TRUE == CConfigTkMem::GetInstance()->m_Refund.bExp )
					{
						if(nSvrKind & SVR_DVS_KOBUS)
						{
							receipt_yn[0] = CCancRyTkKobusMem::GetInstance()->tBase.receipt_yn[0];	
						}
						else
						{
							receipt_yn[0] = CCancRyTkTmExpMem::GetInstance()->tBase.receipt_yn[0];	
						}
					}
					else
					{
						receipt_yn[0] = CCancRyTkMem::GetInstance()->receipt_yn[0];
					}
				}
				TR_LOG_OUT("receipt_yn[0] = (%c) ", receipt_yn[0] & 0xFF);

				if( receipt_yn[0] == 'Y' )
				{
					nRet = Printer_RefundTicket();
					Sleep(1000 * 2);
				}

				TR_LOG_OUT("Info.pyn_mns_dvs_cd[0] = (0x%02X) ", Info.pyn_mns_dvs_cd[0] & 0xFF);		// 20210823 ADD

				/// 환불 - (카드) 환불금액 방출
				UI_AddQueueInfo(UI_CMD_CANCRY_FARE_INFO, (char *)&Info, sizeof(UI_SND_CANCRY_CHG_MONEY_T));
				UI_AddDevAccountInfo();
				SetStep(2);
			}
		}
		break;
	case 2 :
		{	
		}
		break;
	case 4 :
		{
			int nStatus = 0;

			pEnv = (PDEV_CFG_T) GetEnvTicketReaderInfo();

			nRet = Scanner_GetStatus(&nStatus);
			//TR_LOG_OUT("@@@@@@@> Scanner_GetStatus(), nRet(%d), nStatus(%X) ", nRet, nStatus & 0xFFFFFFFF);
			if(nRet >= 0)
			{
				if(pEnv->nModel == DEV_SCANNER_WT)
				{
					/***
					if( ((nStatus & 0xFFFFFFFF) == 0x180) )
					{
						TR_LOG_OUT("WT_Scanner_SetTicketRead(TRUE) !!!");
						Scanner_SetTicketRead(TRUE);
						SetStep(99);
					}
					else
					{
						TR_LOG_OUT("WT_Scanner_SetTicketRead(FALSE) !!!");
						Scanner_SetTicketRead(FALSE);
						SetStep(99);
					}
					***/
					SetCheckEventCode(EC_TICKET_INSERT, FALSE);
					Scanner_SetTicketRead(TRUE);
					SetStep(99);
				}
				else if(pEnv->nModel == DEV_SCANNER_ATEC)
				{
					//if( (nStatus & 0xFFFFFFFF) == 0 )
					if( (nStatus & 0x503) == 0 )
					{
						// scan start..
						TR_LOG_OUT("@@@@@@@> Scanner_GetStatus(), nStatus(%X) ", nStatus & 0xFFFFFFFF);
						SetCheckEventCode(EC_TICKET_INSERT, FALSE);
						Scanner_SetTicketRead(TRUE);
						SetStep(99);
					}
					else if( ((nStatus & 0xFFFFFFFF) & 0x503) )	 // 입구센서  (bit : 0,1,8,10)
					{
						// scan stop
						//Scanner_SetTicketRead(FALSE);
					}
				}
			}
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrCancRyCompleteState
 * @details		환불 - 완료 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrCancRyCompleteState(void)
{
	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	Scanner_SetTicketRead(FALSE);

	SetCheckEventCode(EC_TICKET_INSERT, FALSE);

	/// 장비 회계데이타 전송
	UI_AddDevAccountInfo();
	CCancRyTkMem::GetInstance()->Initialize();
	CCancRyTkKobusMem::GetInstance()->Initialize();
	CCancRyTkTmExpMem::GetInstance()->Initialize();
	SetState(TR_READY_STATE, 0);

	return 0;
}

/**
 * @brief		TrCancRyCancelState
 * @details		환불 - 취소 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrCancRyCancelState(void)
{
	int nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	Scanner_SetTicketRead(FALSE);
	if( GetEventCode(EC_TICKET_INSERT) )
	{
		nRet = Scanner_Reject();
	}
	SetCheckEventCode(EC_TICKET_INSERT, FALSE);

	CCancRyTkMem::GetInstance()->Initialize();
	CCancRyTkKobusMem::GetInstance()->Initialize();
	CCancRyTkTmExpMem::GetInstance()->Initialize();

	SetState(TR_READY_STATE, 0);

	return 0;
}

/**
 * @brief		TrCancRyState
 * @details		환불
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrCancRyState(void)
{
	switch(s_nState)
	{
	case TR_CANCRY_MAIN_STATE :	
		{	// "환불" 시작
			TrCancRyMainState();
		}
		break;
	case TR_CANCRY_TCK_READ:
		{	// "환불" - 승차권 읽기
			TrCancRyTckReadState();
		}
		break;
	case TR_CANCRY_FARE_STATE :
		{	// "환불" - 카드 또는 현금 환불 처리
			TrCancRyFareState();
		}
		break;
	case TR_CANCRY_COMPLETE_STATE :
		{	// "환불" - 완료 처리
			TrCancRyCompleteState();
		}
		break;
	case TR_CANCRY_CANCEL_STATE :
		{	// "환불" - 취소 처리
			TrCancRyCancelState();
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckMainState
 * @details		현장발권 main state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckMainState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 
			CPubTckMem::GetInstance()->Initialize();
			CPubTckKobusMem::GetInstance()->Initialize();
			CPubTckTmExpMem::GetInstance()->Initialize();
			SetStep(1);
		}
		break;
	}

	return 0;
}

// 20221205 ADD~
/**
 * @brief		TrPbTckReqQrMdPcpySats
 * @details		현장발권 - [시외] QR 좌석 선점 시간변경
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckReqQrMdPcpySats(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("Step(%d, %d) !!!", s_nPrevStep, s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 배차 조회
			char ch;

			nRet = Svr_IfSv_278();
			TR_LOG_OUT("[시외] 현장발권 - QR 좌석 선점 시간변경 요청, nRet(%d).", nRet);
			if(nRet < 0)
			{
				UI_AddFailInfo(UI_CMD_TICKET_QRMDPCPYSATS, CConfigTkMem::GetInstance()->GetErrorCode(), CConfigTkMem::GetInstance()->GetErrorContents());
				SetStep(2);
				return -1;
			}
			ch = CHAR_ACK;
			UI_AddQueueInfo(UI_CMD_TICKET_QRMDPCPYSATS, &ch, 1);
			SetStep(1);
		}
		break;
	case 1 :
		{

		}
		break;
	}

	return 0;
}
// 20221205 ~ADD

/**
 * @brief		TrPbTckReqListState
 * @details		현장발권 배차리스트 요청 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckReqListState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("Step(%d, %d) !!!", s_nPrevStep, s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 배차 조회
			char ch;

			nRet = Svr_IfSv_130();
			TR_LOG_OUT("[시외] 현장발권 - 배차조회 요청, nRet(%d).", nRet);
			if(nRet < 0)
			{
				UI_AddFailInfo(UI_CMD_PBTCK_RESP_LIST, CConfigTkMem::GetInstance()->GetErrorCode(), CConfigTkMem::GetInstance()->GetErrorContents());
				SetStep(2);
				return -1;
			}
			ch = CHAR_ACK;
			UI_AddQueueInfo(UI_CMD_PBTCK_RESP_LIST, &ch, 1);
			SetStep(1);
		}
		break;
	case 1 :
		{

		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckKobus_ReqListState
 * @details		[현장발권] - [코버스] 배차리스트 요청 
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckKobus_ReqListState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	
			///> 코버스 - 배차조회
			nRet = Kobus_MG_ReadAlcn();
			if(nRet < 0)
			{
				UI_AddKobus_AlcnListInfo(FALSE);
				SetStep(2);
				return -1;
			}

			UI_AddKobus_AlcnListInfo(TRUE);
			SetStep(1);
		}
		break;
	case 1 :
		{
			;			
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckTmExp_ReqListState
 * @details		[현장발권] - [티머니고속] 배차리스트 요청 
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckTmExp_ReqListState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	
			///> 티머니고속 - 배차조회
			nRet = TmExp_MG_ReadAlcn();
			if(nRet < 0)
			{
				UI_AddQueTmExp(UI_CMD_RES_TMEXP_PBTCK_LIST, FALSE);
				SetStep(2);
				return -1;
			}

			UI_AddQueTmExp(UI_CMD_RES_TMEXP_PBTCK_LIST, TRUE);
			SetStep(1);
		}
		break;
	case 1 :
		{
			;			
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckKobus_ReqSatsState
 * @details		[현장발권] 좌석정보 요청 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckKobus_ReqSatsState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 코버스 - 좌석정보 조회 
			nRet = Kobus_CM_ReadSatsFee();
			if(nRet < 0)
			{
				UI_AddKobus_SatsListInfo(FALSE);
				SetStep(2);
				return -1;
			}

			/// 코버스 - 경유지정보 조회
			nRet = Kobus_CM_ReadRotInf();
			if(nRet < 0)
			{
				/// 좌석정보 결과 전송
				UI_AddKobus_SatsListInfo(TRUE);
				/// 경유지 정보 결과 전송
				UI_AddKobus_ThruInfo(FALSE);
				SetStep(2);
				return -1;
			}

			/// 좌석정보 결과 전송
			UI_AddKobus_SatsListInfo(TRUE);
			/// 경유지 정보 결과 전송
			UI_AddKobus_ThruInfo(TRUE);

			SetStep(1);
		}
		break;
	case 1 :
		{
			
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckKobus_ReqSatsPcpyState
 * @details		[현장발권] 좌석정보 요청 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckKobus_ReqSatsPcpyState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	
			/// 코버스 - 좌석선점 요청
			nRet = Kobus_TK_PcpySats();
			if(nRet < 0)
			{
				UI_AddKobus_SatsPcpyInfo(FALSE);
				SetStep(2);
				return -1;
			}
			UI_AddKobus_SatsPcpyInfo(TRUE);
			SetStep(1);
		}
		break;
	case 1 :
		{
			
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckKobus_ReqThruState
 * @details		[현장발권] - [코버스] 경유지 정보 요청 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckKobus_ReqThruState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	
			/// 코버스 - 경유지 정보 요청
			nRet = Kobus_CM_ReadRotInf();
			if(nRet < 0)
			{
				UI_AddKobus_ThruInfo(FALSE);
				SetStep(2);
				return -1;
			}
			UI_AddKobus_ThruInfo(TRUE);
			SetStep(1);
		}
		break;
	case 1 :
		{
			
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckTmExp_ReqSatsState
 * @details		[현장발권] 티머니고속 - 좌석정보 요청 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckTmExp_ReqSatsState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 티머니고속 - 좌석정보 조회 
			nRet = TmExp_CM_ReadSatsFee();
			if(nRet < 0)
			{
				UI_AddQueTmExp(UI_CMD_RES_TMEXP_SATS_FEE, FALSE);
				SetStep(2);
				return -1;
			}
			/// 좌석정보 결과 전송
			UI_AddQueTmExp(UI_CMD_RES_TMEXP_SATS_FEE, TRUE);

			SetStep(1);
		}
		break;
	case 1 :
		{	/// 티머니고속 - 경유지 정보 조회 
			nRet = TmExp_CM_ReadThruTrml();
			if(nRet < 0)
			{
				UI_AddQueTmExp(UI_CMD_RES_TMEXP_THRU_INFO, FALSE);
				SetStep(2);
				return -1;
			}
			/// 경유지 정보 전송
			UI_AddQueTmExp(UI_CMD_RES_TMEXP_THRU_INFO, TRUE);

			SetStep(2);
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckTmExp_ReqSatsPcpyState
 * @details		[현장발권] 티머니고속 - 좌석선점 요청 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckTmExp_ReqSatsPcpyState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 티머니고속 - 좌석선점 조회 
			nRet = TmExp_TK_PcpySats();
			if(nRet < 0)
			{
				/// 좌석선점 실패 결과 전송
				UI_AddQueTmExp(UI_CMD_RES_TMEXP_PCPY_SATS, FALSE);
				SetStep(2);
				return -1;
			}
			/// 좌석선점 성공 결과 전송
			UI_AddQueTmExp(UI_CMD_RES_TMEXP_PCPY_SATS, TRUE);

			SetStep(1);
		}
		break;
	case 1 :
		{
			
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckTmExp_ReqThruState
 * @details		[현장발권] 티머니고속 - 경유지 정보 요청 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckTmExp_ReqThruState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 티머니고속 - 경유지 정보 요청 
			nRet = TmExp_CM_ReadThruTrml();
			if(nRet < 0)
			{
				/// 경유지 정보 실패 결과 전송
				UI_AddQueTmExp(UI_CMD_RES_TMEXP_THRU_INFO, FALSE);
				SetStep(2);
				return -1;
			}
			/// 경유지 정보 성공 결과 전송
			UI_AddQueTmExp(UI_CMD_RES_TMEXP_THRU_INFO, FALSE);

			SetStep(1);
		}
		break;
	case 1 :
		{
			
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckListSelectState
 * @details		현장발권 배차리스트 선택 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckListSelectState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// (131) 배차요금 조회 요청 
			nRet = Svr_IfSv_131();
			if(nRet > 0)
			{	/// 성공
				UI_TkPubAlcnFareInfo(TRUE, 1);
			}
			else
			{	/// 실패
				UI_TkPubAlcnFareInfo(FALSE, nRet);
			}
			SetStep(1);
		}
		break;
	case 1 :
		{

		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckReqStaff
 * @details		[현장발권_인천공항] 상주직원 조회 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckReqStaff(void)
{
	int		nRet = 0;

	try
	{
		if(s_nPrevStep != s_nStep)
		{
			TR_LOG_OUT("s_nStep(%d) !!!\n", s_nStep);
			SetPrevState(s_nStep);
		}

		switch(s_nStep)
		{
		case 0 :
			{	/// (225) 상주직원_조회
				nRet = Svr_IfSv_225();
				if(nRet > 0)
				{	/// 성공
					UI_Add_Q_SendData(UI_CMD_PBTCK_STAFF_RESP, TRUE, 1, (BYTE *)NULL, 0);
				}
				else
				{	/// 실패
					UI_Add_Q_SendData(UI_CMD_PBTCK_STAFF_RESP, FALSE, nRet, (BYTE *)NULL, 0);
				}
				SetStep(1);	 
			}
			break;
		case 1 :
			{

			}
			break;
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		TrPbTckReqStaffCdModFare
 * @details		[현장발권_인천공항] 상주직원 카드 요금 변경 정보 조회 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckReqStaffCdModFare(void)
{
	int		nRet = 0;
	int		nAlcn = 0;

	try
	{
		if(s_nPrevStep != s_nStep)
		{
			TR_LOG_OUT("s_nStep(%d) !!!\n", s_nStep);
			SetPrevState(s_nStep);
		}

		nAlcn = Config_IsAlcnMode();
		switch(s_nStep)
		{
		case 0 :
			{	/// (228) 상주직원_카드_요금변경_정보_조회
				TR_LOG_OUT("Svr_IfSv_228 start, nAlcn(%d) ---", nAlcn);
				nRet = Svr_IfSv_228();
				TR_LOG_OUT("Svr_IfSv_228 end, nRet(%d) ---", nRet);
				if(nRet > 0)
				{	/// 성공
					UI_Add_Q_SendData(UI_CMD_PBTCK_STAFF_CD_MOD_FARE_RESP, TRUE, 1, (BYTE *)&nAlcn, (int) sizeof(nAlcn));
				}
				else
				{	/// 실패
					UI_Add_Q_SendData(UI_CMD_PBTCK_STAFF_CD_MOD_FARE_RESP, FALSE, nRet, (BYTE *)&nAlcn, (int) sizeof(nAlcn));
				}
				SetStep(1);	 
			}
			break;
		case 1 :
			{

			}
			break;
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}


/**
 * @brief		TrPbTckReqRfCardPayment
 * @details		[현장발권] RF 선불카드 결제 요청 state // 20230222 MOD => 승차권 발행(선불) 	
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckReqRfCardPayment(void)
{
	int		nRet, nAlcn;

	nRet = nAlcn = 0;

	try
	{
		if(s_nPrevStep != s_nStep)
		{
			TR_LOG_OUT("s_nStep(%d) !!!\n", s_nStep);
			SetPrevState(s_nStep);
		}

		nAlcn = Config_IsAlcnMode();
		switch(s_nStep)
		{
		case 0 :
			{	/// (260) RF 선불카드 결제 // 20230222 MOD => (IF_SV_260	- TK_PubTckPpy) 승차권 발행(선불) 
				int nErrorCnt = 0;
				vector<PUBTCK_UI_RF_CARD_T>::iterator iter;

				for(iter = CPubTckMem::GetInstance()->m_vtRfUiData.begin(); iter != CPubTckMem::GetInstance()->m_vtRfUiData.end(); iter++)
				{
					nRet = Svr_IfSv_260((char *) iter._Ptr); // 승차권 발행(RF_선불카드) (TK_PubTckPpy)
					if(nRet > 0)
					{	/// 성공
						//UI_Add_Q_SendData(UI_CMD_PBTCK_PRE_RF_PAY_RESP, TRUE, 1, (BYTE *)&nAlcn, (int) sizeof(nAlcn));
					}
					else
					{	/// 실패
						//UI_AddFailInfo(UI_CMD_PBTCK_PRE_RF_PAY_RESP, CConfigTkMem::GetInstance()->GetErrorCode, (char *)NULL);
						/// 20230222 ADD~
						/// RF 선불카드 차감 완료 후에 서버에서 발권 실패의 경우에도
						/// 관리자화면에서 재발권이 가능한 상태인데, 
						/// 아래 '좌석선점 정보 지우기(ErasePcpySatsInfo)'를 반드시 해야하는 지는 추후 재검토가 필요함.
						/// 20230222 ~ADD
						CPubTckMem::GetInstance()->ErasePcpySatsInfo(iter->sats_no); // [시외버스] 좌석선점 정보 지우기
						nErrorCnt++;
					}
				}

				if(nErrorCnt > 0)
				{
					SetCheckEventCode(EC_RF_PAYMENT_ERR, TRUE);
					UI_AddFailInfo(UI_CMD_PBTCK_PRE_RF_PAY_RESP, CConfigTkMem::GetInstance()->GetErrorCode(), (char *)NULL); // 20230222 ADD
				}
				else // 20230222 ADD
				{	 // 20230222 ADD
					UI_Add_Q_SendData(UI_CMD_PBTCK_PRE_RF_PAY_RESP, TRUE, 1, (BYTE *)&nAlcn, (int) sizeof(nAlcn));
				}	 // 20230222 ADD

				SetStep(1);

			}
			break;
		case 1 :
			{

			}
			break;
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		TrPbTckSetSeatState
 * @details		현장발권 좌석 선택 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckSetSeatState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		//s_nPrevStep = s_nStep;
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 좌석선점
			nRet = Svr_IfSv_202(TRUE);
			if(nRet > 0)
			{	/// 성공
				UI_TkPubSatsInfo(Config_IsAlcnMode(), TRUE, 1);
			}
			else
			{	/// 실패
				UI_TkPubSatsInfo(Config_IsAlcnMode(), FALSE, nRet);
			}
			SetStep(1);
		}
		break;
	case 1 :
		{

		}
		break;
	case 99 :
		{	// 비배차 모드인 경우
			int nSatNo;
			CPubTckMem* pTr;

			pTr = CPubTckMem::GetInstance();

			vector<UI_SATS_T>::iterator iter;

			nSatNo = 0;
			pTr->m_vtNPcpysats.clear();
			for( iter = pTr->m_vtUiSats.begin(); iter != pTr->m_vtUiSats.end(); iter++ )
			{
				rtk_pcpysats_list_t List;

				::ZeroMemory(&List, sizeof(rtk_pcpysats_list_t));

				///< 좌석선점ID
				///< 시외버스할인종류코드
				List.cty_bus_dc_knd_cd[0] = iter->cty_bus_dc_knd_cd[0];
				///< 할인율구분코드
				List.dcrt_dvs_cd[0] = iter->dcrt_dvs_cd[0];
				///< 버스티켓종류코드	
				::CopyMemory(List.bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(List.bus_tck_knd_cd) - 1);
				///< 할인이후금액		
				::CopyMemory(List.dc_aft_amt, &iter->n_tisu_amt, 4);
				///< 좌석번호
				::CopyMemory(List.sats_no, &nSatNo, 4);

				pTr->m_vtNPcpysats.push_back(List);
			}
			UI_TkPubSatsInfo(Config_IsAlcnMode(), TRUE, 1);

			SetStep(100);
		}
		break;
	}

	return 0;
}


/**
 * @brief		TrPbTckCoronaState
 * @details		현장발권 코로나 개인정보 입력 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckCoronaState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		//s_nPrevStep = s_nStep;
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 좌석선점
			SetStep(1);
		}
		break;
	case 1 :
		{

		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckPaySelectState
 * @details		현장발권 결제수단 선택 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckPaySelectState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckBillInsertState
 * @details		현장발권 지폐투입 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckBillInsertState(void)
{
	int		nRet = 0;
	UI_SND_BILL_INFO_T Info;
	static int nInsTotal = 0, nTotal = 0, s_n1k = 0, s_n5k = 0, s_n10k = 0, s_n50k = 0, s_nInsTotal = 0;
	static int nPrevAction = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
		nPrevAction = 0;
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 지폐투입
			s_n1k = 0, s_n5k = 0, s_n10k = 0, s_n50k = 0, s_nInsTotal = 0;

			if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_CCBUS )
			{	/// 시외버스_EZ
				nInsTotal = nTotal = CPubTckMem::GetInstance()->base.nTotalMoney;
			}
			else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_KOBUS )
			{	/// 코버스
				nInsTotal = nTotal = CPubTckKobusMem::GetInstance()->base.nTotalMoney;
			}
			else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_TMEXP )
			{	/// 티머니고속
				nInsTotal = nTotal = CPubTckTmExpMem::GetInstance()->base.nTotalMoney;
			}

			SetStep(1);
		}
		break;
	case 1 :
		{	/// 
			int nAction = 0;
			int nChangeMoney;
			int n1k, n5k, n10k, n50k;

			nAction = Bill_GetActionStatus();
			switch( nAction )
			{
			case 0x02:
// 			case 0x04:
// 			case 0x08:
// 			case 0x10:
// 			case 0x20:
// 			case 0x40:
// 			case 0x11:

				//TR_LOG_OUT("nPrevAction = %d, nAction = %d", nPrevAction, nAction);
				if(nPrevAction != nAction)
				{
					TR_LOG_OUT("지폐투입구 Accept 전송");

					nInsTotal = Bill_GetMoney(&n1k, &n5k, &n10k, &n50k);
					/// UI 지폐투입수량 전송
					{
						::ZeroMemory(&Info, sizeof(UI_SND_BILL_INFO_T));

						Info.byACK = CHAR_ACK;
						Info.bySvrDvs = CConfigTkMem::GetInstance()->GetUiSvrKind();
						Info.w1k = (WORD) n1k;
						Info.w5k = (WORD) n5k;
						Info.w10k = (WORD) n10k;
						Info.w50k = (WORD) n50k;
						UI_AddQueueInfo(UI_CMD_PBTCK_BILL_INFO, (char *)&Info, sizeof(UI_SND_BILL_INFO_T));
					}
				}
				break;
			default:
				break;
			}
			
			if(nPrevAction != nAction)
			{
				nPrevAction = nAction;
			}

			nInsTotal = Bill_GetMoney(&n1k, &n5k, &n10k, &n50k);
			//if(s_nInsTotal != nInsTotal)
			if( (s_n1k != n1k) || (s_n5k != n5k) || (s_n10k != n10k) || (s_n50k != n50k) )
			{
				int nTmp1k, nTmp5k, nTmp10k, nTmp50k;
				
				nTmp1k  = n1k - s_n1k;
				nTmp5k  = n5k - s_n5k;
				nTmp10k = n10k - s_n10k; 
				nTmp50k = n50k - s_n50k;
				TR_LOG_OUT("#### Bill Inserted(), 1k(%d), 5k(%d), 10k(%d), 50k(%d) ", nTmp1k, nTmp5k, nTmp10k, nTmp50k);
				/// 회계반영 - 지폐투입 
				AddAccumBillData(ACC_BILL_INSERT, nTmp1k, nTmp5k, nTmp10k, nTmp50k);

				s_n1k = n1k;
				s_n5k = n5k;
				s_n10k = n10k;
				s_n50k = n50k;
				s_nInsTotal = nInsTotal;

				TR_LOG_OUT("#### Bill_GetMoney(), ins(%d:%d), 1k(%d), 5k(%d), 10k(%d), 50k(%d) ", nInsTotal, s_nInsTotal, n1k, n5k, n10k, n50k);

				/// UI 지폐투입수량 전송
				{
					::ZeroMemory(&Info, sizeof(UI_SND_BILL_INFO_T));
					
					Info.byACK = CHAR_ACK;
					Info.bySvrDvs = CConfigTkMem::GetInstance()->GetUiSvrKind();
					Info.w1k = (WORD) n1k;
					Info.w5k = (WORD) n5k;
					Info.w10k = (WORD) n10k;
					Info.w50k = (WORD) n50k;
					UI_AddQueueInfo(UI_CMD_PBTCK_BILL_INFO, (char *)&Info, sizeof(UI_SND_BILL_INFO_T));
				}

				if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_CCBUS )
				{
					TR_LOG_OUT("[시외버스] 투입금액, 1k(%d), 5k(%d), 10k(%d), 50k(%d) ", n1k, n5k, n10k, n50k);

					CPubTckMem::GetInstance()->base.insBill.n1k = n1k;
					CPubTckMem::GetInstance()->base.insBill.n5k = n5k;
					CPubTckMem::GetInstance()->base.insBill.n10k = n10k;
					CPubTckMem::GetInstance()->base.insBill.n50k = n50k;
				}
				else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_KOBUS )
				{
					TR_LOG_OUT("[코버스] 투입금액, 1k(%d), 5k(%d), 10k(%d), 50k(%d) ", n1k, n5k, n10k, n50k);

					CPubTckKobusMem::GetInstance()->base.insBill.n1k = n1k;
					CPubTckKobusMem::GetInstance()->base.insBill.n5k = n5k;
					CPubTckKobusMem::GetInstance()->base.insBill.n10k = n10k;
					CPubTckKobusMem::GetInstance()->base.insBill.n50k = n50k;
				}
				else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_TMEXP )
				{
					TR_LOG_OUT("[티머니고속] 투입금액, 1k(%d), 5k(%d), 10k(%d), 50k(%d) ", n1k, n5k, n10k, n50k);

					CPubTckTmExpMem::GetInstance()->base.insBill.n1k = n1k;
					CPubTckTmExpMem::GetInstance()->base.insBill.n5k = n5k;
					CPubTckTmExpMem::GetInstance()->base.insBill.n10k = n10k;
					CPubTckTmExpMem::GetInstance()->base.insBill.n50k = n50k;
				}
				else 
				{
					TR_LOG_OUT("[버스_에러] 투입금액 ????");
				}

				if( (nInsTotal > 0) && (nTotal <= nInsTotal) )
				{
					nChangeMoney = nInsTotal - nTotal;
					// 총 발권금액

					if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_CCBUS )
					{	/// 시외버스_EZ
						CPubTckMem::GetInstance()->n_pbtck_chg_money = nChangeMoney;
					}
					else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_KOBUS )
					{	/// 코버스
						CPubTckKobusMem::GetInstance()->n_pbtck_chg_money = nChangeMoney;
					}
					else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_TMEXP )
					{	/// 티머니고속
						CPubTckTmExpMem::GetInstance()->n_pbtck_chg_money = nChangeMoney;
					}

					//CPubTckMem::GetInstance()->n_pbtck_chg_money = nChangeMoney;

					/// 회계반영 - 지폐투입 /
					///AddAccumBillData(ACC_BILL_INSERT, n1k, n5k, n10k, n50k);

					/// 입출내역 TR (입금)
					///TR_LOG_OUT("[#01] 투입 갯수 (%4d : %4d : %4d : %4d) ", n1k, n5k, n10k, n50k);

					AddCashTrData(DT_FLAG_IN_MONEY, (WORD)0, (WORD)0, (WORD)n1k, (WORD)n5k, (WORD)n10k, (WORD)n50k);

					//CPubTckMem::GetInstance()->base.insBill.n1k = 0;
					//CPubTckMem::GetInstance()->base.insBill.n10k = 0;

					Bill_Inhibit();
					Bill_TotalEnd();
					SetStep(2);
				}
			}

		}
		break;
	case 2 :
		{	/// idle

		}
		break;
	}

	return 0;
}

static int TrPbTckBillStopState(void)
{
	int		nRet = 0;
	int		n1k = 0, n5k = 0, n10k = 0, n50k = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 
			Bill_Inhibit();
			SetStep(1);
		}
		break;
	case 1 :
		{
			nRet = Bill_Enable();
			Bill_GetMoney(&n1k, &n5k, &n10k, &n50k);
			if( (n1k + n5k + n10k + n50k) > 0 )
			{
				UI_SND_BILL_INFO_T Info;

				TR_LOG_OUT("예외_지폐투입 = (%d, %d, %d, %d", n1k, n5k, n10k, n50k);

				::ZeroMemory(&Info, sizeof(UI_SND_BILL_INFO_T));
				Info.byACK = CHAR_ACK;
				Info.bySvrDvs = CConfigTkMem::GetInstance()->GetUiSvrKind();
				Info.w1k = (WORD) n1k;
				Info.w5k = (WORD) n5k;
				Info.w10k = (WORD) n10k;
				Info.w50k = (WORD) n50k;
				UI_AddQueueInfo(UI_CMD_PBTCK_BILL_INFO, (char *)&Info, sizeof(UI_SND_BILL_INFO_T));
				SetStep(99);
			}
			else
			{
				SetStep(0);
			}
		}
		break;
	}



	return 0;
}

/**
 * @brief		TrPbTckCsrcReadState
 * @details		현장발권 현금영수증 카드 읽기 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckCsrcReadState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckCsrcInputState
 * @details		현장발권 현금영수증 입력 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckCsrcInputState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckTmaxIssueState
 * @details		현장발권 - TMAX 발권 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckTmaxIssueState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		//s_nPrevStep = s_nStep;
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0:
		{
			char pyn_mns_dvs_cd;
			char ui_csrc_dvs_cd;
			char rf_trcr_dvs_cd;
			char ui_csrc_use;

			/// 결제 구분코드 - 'A':현금, 'B':현금영수증, 'C':신용카드, 'D':RF결제, 'Q':QR결제	// 20221017 MOD
			pyn_mns_dvs_cd = CPubTckMem::GetInstance()->base.pyn_mns_dvs_cd[0];

			/// 현금영수증 구분코드 - 0:미사용, 1:수기입력, 2:신용카드, 3:현영전용카드
			ui_csrc_dvs_cd = CPubTckMem::GetInstance()->base.ui_csrc_dvs_cd[0];

			/// 개인/법인
			ui_csrc_use = CPubTckMem::GetInstance()->base.ui_csrc_use[0];

			/// RF교통카드 구분코드 - t:티머니 선불, T:티머니 후불, M:Mifare
			rf_trcr_dvs_cd = CPubTckMem::GetInstance()->base.rf_trcr_dvs_cd[0];

			TR_LOG_OUT("결제구분코드 = 0x%02X", pyn_mns_dvs_cd);
			TR_LOG_OUT("현금영수증 구분코드 = 0x%02X", ui_csrc_dvs_cd);
			TR_LOG_OUT("개인/법인 구분코드 = 0x%02X", ui_csrc_use);
			TR_LOG_OUT("RF 구분코드 = 0x%02X", rf_trcr_dvs_cd);

			// 지불수단코드
			switch(pyn_mns_dvs_cd)	
			{
			case PYM_CD_CARD : // 신용카드
				nRet = Svr_IfSv_221(FALSE);
				if(nRet > 0)
				{	// 성공
					UI_TkPubTmaxResultInfo(TRUE, 0);
					SetStep(2);
				}
				else
				{	// 실패
					UI_TkPubTmaxResultInfo(FALSE, 1);
					SetStep(3);
				}
				break;
			case PYM_CD_CSRC :	// 현금영수증
				if( (ui_csrc_dvs_cd == 1) || (ui_csrc_dvs_cd == 3))
				{
					nRet = Svr_IfSv_134();
					//nRet = Svr_IfSv_150();
				}
				else
				{
					nRet = Svr_IfSv_222();
				}
				if(nRet > 0)
				{	// 성공
					UI_TkPubTmaxResultInfo(TRUE, 0);
					SetStep(2);
				}
				else
				{	// 실패
					UI_TkPubTmaxResultInfo(FALSE, 1);
					SetStep(3);
				}
				break;
			case PYM_CD_CASH :  // 현금
				nRet = Svr_IfSv_132();
				if(nRet > 0)
				{	// 성공
					UI_TkPubTmaxResultInfo(TRUE, 0);
					SetStep(2);
				}
				else
				{	// 실패
					UI_TkPubTmaxResultInfo(FALSE, 1);
					SetStep(3);
				}
				break;
			case PYM_CD_RF:		// RF 결제
				switch(rf_trcr_dvs_cd)
				{
				case 'M' : // Mifare 후불
				case 'T' : // 티머니 후불
					nRet = Svr_IfSv_221(FALSE);
					break;
				case 't' : // 티머니 선불
				case 'c' : // 캐시비 선불
					//nRet = Svr_IfSv_260();
					//break;
				default:
					nRet = -1;
					CConfigTkMem::GetInstance()->SetErrorCode("SVE112");
					break;
				}

				if(nRet > 0)
				{	// 성공
					UI_TkPubTmaxResultInfo(TRUE, 0);
					SetStep(2);
				}
				else
				{	// 실패
					UI_TkPubTmaxResultInfo(FALSE, 1);
					SetStep(3);
				}
				break;
			// 20221017 ADD~
			case PYM_CD_QRPC : // QR결제-PAYCO-신용카드[PC](시외)
			case PYM_CD_QRPP : // QR결제-PAYCO-포인트[PP](시외)		// 20221124 ADD
			case PYM_CD_QRTP : // QR결제-티머니페이[TP](시외)			// 20221124 ADD

				//nRet = Svr_IfSv_221(FALSE);

				/// IF_UI_311 (UI -> Main)	에서 정보를 미리 수신 

				//char sQr_cd_no				[100];      // QR코드번호
				//char sPayco_virt_ontc_no	[21];		// OTC번호;페이코가상일회성카드번호

				//// QR코드번호
				//::CopyMemory(sQr_cd_no, CPubTckMem::GetInstance()->base.qr_cd_no, sizeof(CPubTckMem::GetInstance()->base.qr_cd_no));
				//// OTC번호;페이코가상일회성카드번호
				//::CopyMemory(sQr_cd_no, CPubTckMem::GetInstance()->base.payco_virt_ontc_no, sizeof(CPubTckMem::GetInstance()->base.payco_virt_ontc_no));

				// QR 결제는 UI에서 OTP 생성하고, '승차권발행-QR결제' 요청 시 필요 정보를 UI에서 송신
				nRet = Svr_IfSv_274();
				if(nRet > 0)
				{	// 성공
					TR_LOG_OUT("Svr_IfSv_274() Success, nRet(%d) !!", nRet);
					UI_TkPubTmaxResultInfo(TRUE, 0);
					SetStep(2);
				}
				else
				{	// 실패
					TR_LOG_OUT("Svr_IfSv_274() Fail, nRet(%d) !!", nRet);
					UI_TkPubTmaxResultInfo(FALSE, 1);
					SetStep(3);
				}
				break;
			// 20221017 ~ADD
			default:
				TR_LOG_OUT("카드전용 장비는 현금으로 결제 안됨 !!!");		
				SetStep(1);
				break;
			}
		}
		break;

	case 1 :
		{	/// 
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckKobus_TmaxIssueState
 * @details		현장발권 - [코버스] TMAX 발권 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckKobus_TmaxIssueState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		//s_nPrevStep = s_nStep;
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0:
		{
			//char pyn_mns_dvs_cd = 0;

			/// 코버스 - TMAX - 현장발권 (현금/카드..)
			//pyn_mns_dvs_cd = CPubTckKobusMem::GetInstance()->base.ui_pym_dvs_cd[0];

			nRet = Kobus_TK_PubTck();
			if(nRet > 0)
			{	// 성공
				UI_TkPubTmaxResultInfo(TRUE, 0);
				SetStep(1);
			}
			else
			{	// 실패
				UI_TkPubTmaxResultInfo(FALSE, 1);
				SetStep(2);
			}
		}
		break;

	case 1 :
		{	/// 
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTck_TmExp_TmaxIssueState
 * @details		현장발권 - [티머니고속] TMAX 발권 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTck_TmExp_TmaxIssueState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		//s_nPrevStep = s_nStep;
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0:
		{
			char pyn_mns_dvs_cd;
			char ui_csrc_dvs_cd;
//			char rf_trcr_dvs_cd;
			char ui_csrc_use;

			/// 결제 구분코드 - 'A':현금, 'B':현금영수증, 'C':신용카드, 'D':RF결제
			pyn_mns_dvs_cd = CPubTckTmExpMem::GetInstance()->base.pyn_mns_dvs_cd[0];

			/// 현금영수증 구분코드 - 0:미사용, 1:수기입력, 2:신용카드, 3:현영전용카드
			ui_csrc_dvs_cd = CPubTckTmExpMem::GetInstance()->base.ui_csrc_dvs_cd[0];

			/// 개인/법인
			ui_csrc_use = CPubTckTmExpMem::GetInstance()->base.ui_csrc_use[0];

			/// RF교통카드 구분코드 - t:티머니 선불, T:티머니 후불, M:Mifare
			//rf_trcr_dvs_cd = CPubTckTmExpMem::GetInstance()->base.rf_trcr_dvs_cd[0];

			TR_LOG_OUT("[티머니고속] 결제구분코드 = 0x%02X", pyn_mns_dvs_cd);
			TR_LOG_OUT("[티머니고속] 현금영수증 구분코드 = 0x%02X", ui_csrc_dvs_cd);
			TR_LOG_OUT("[티머니고속] 개인/법인 구분코드 = 0x%02X", ui_csrc_use);
			//TR_LOG_OUT("[티머니고속] RF 구분코드 = 0x%02X", rf_trcr_dvs_cd);

			// 지불수단코드
			switch(pyn_mns_dvs_cd)	
			{
			case PYM_CD_CARD : // 신용카드
				nRet = TmExp_TK_PubTckCardKTC();
				if(nRet > 0)
				{	// 성공
					UI_AddQueTmExp(UI_CMD_RES_TMEXP_PBTCK_CARD_KTC, TRUE);
					//UI_TkPubTmaxResultInfo(TRUE, 0);
					SetStep(2);
				}
				else
				{	// 실패
					UI_AddQueTmExp(UI_CMD_RES_TMEXP_PBTCK_CARD_KTC, FALSE);
					//UI_TkPubTmaxResultInfo(FALSE, 1);
					SetStep(3);
				}
				break;
			case PYM_CD_CSRC :	// 현금영수증
				/****
				if( (ui_csrc_dvs_cd == 1) || (ui_csrc_dvs_cd == 3))
				{
					nRet = Svr_IfSv_134();	// 현금영수증 ktc 없는거
					//nRet = Svr_IfSv_150();
				}
				else
				{
					nRet = Svr_IfSv_222();	// 현금영수증 ktc
				}
				***/
				nRet = TmExp_TK_PubTckCsrcKTC();
				if(nRet > 0)
				{	// 성공
					UI_AddQueTmExp(UI_CMD_RES_TMEXP_PBTCK_CSRC_KTC, TRUE);
					//UI_TkPubTmaxResultInfo(TRUE, 0);
					SetStep(2);
				}
				else
				{	// 실패
					UI_AddQueTmExp(UI_CMD_RES_TMEXP_PBTCK_CSRC_KTC, FALSE);
					//UI_TkPubTmaxResultInfo(FALSE, 1);
					SetStep(3);
				}
				break;
			case PYM_CD_CASH :  // 현금
				nRet = TmExp_TK_PubTckCash();
				if(nRet > 0)
				{	// 성공
					UI_AddQueTmExp(UI_CMD_RES_TMEXP_PBTCK_CASH, TRUE);
					//UI_TkPubTmaxResultInfo(TRUE, 0);
					SetStep(2);
				}
				else
				{	// 실패
					UI_AddQueTmExp(UI_CMD_RES_TMEXP_PBTCK_CASH, FALSE);
					//UI_TkPubTmaxResultInfo(FALSE, 1);
					SetStep(3);
				}
				break;
			case PYM_CD_RF:		// RF 결제
				/****
				switch(rf_trcr_dvs_cd)
				{
				case 'M' : // Mifare 후불
				case 'T' : // 티머니 후불
					nRet = Svr_IfSv_221(FALSE);
					break;
				case 't' : // 티머니 선불
				case 'c' : // 캐시비 선불
					//nRet = Svr_IfSv_260();
					//break;
				default:
					nRet = -1;
					CConfigTkMem::GetInstance()->SetErrorCode("SVE112");
					break;
				}

				if(nRet > 0)
				{	// 성공
					UI_TkPubTmaxResultInfo(TRUE, 0);
					SetStep(2);
				}
				else
				{	// 실패
					UI_TkPubTmaxResultInfo(FALSE, 1);
					SetStep(3);
				}
				break;
				***/
			default:
				TR_LOG_OUT("카드전용 장비는 현금으로 결제 안됨 !!!");		
				SetStep(1);
				break;
			}
		}
		break;

	case 1 :
		{	/// 
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckIssueState
 * @details		[시외버스]-[현장발권]-승차권 발권 프린트
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckIssueState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 
			char pyn_mns_dvs_cd;
			char ui_csrc_dvs_cd;
			char rf_trcr_dvs_cd;
			int nAccPymDvs;
			BOOL bFirst;
			char card_aprv_no[100+1];

			pyn_mns_dvs_cd = CPubTckMem::GetInstance()->base.pyn_mns_dvs_cd[0];
			ui_csrc_dvs_cd = CPubTckMem::GetInstance()->base.ui_csrc_dvs_cd[0];
			rf_trcr_dvs_cd = CPubTckMem::GetInstance()->base.rf_trcr_dvs_cd[0];

			TR_LOG_OUT("결제구분코드 = 0x%02X", pyn_mns_dvs_cd);
			TR_LOG_OUT("현금영수증 구분코드 = 0x%02X", ui_csrc_dvs_cd);
			TR_LOG_OUT("RF 구분코드 = 0x%02X", rf_trcr_dvs_cd);

			nAccPymDvs = IDX_ACC_CARD;

			if(pyn_mns_dvs_cd == PYM_CD_CASH)
			{	/// 현금결제
				nAccPymDvs = IDX_ACC_CASH;
				CPubTckMem::GetInstance()->MakeCashTicketPrtData();
			}
			else if(pyn_mns_dvs_cd == PYM_CD_CSRC)
			{	/// 현금영수증 결제
				nAccPymDvs = IDX_ACC_CASH;

				if(	(ui_csrc_dvs_cd == 1) || (ui_csrc_dvs_cd == 3) )
				{	// 수기 현금영수증
					CPubTckMem::GetInstance()->MakeCsrcTicketPrtData();
				}
				else
				{	// 현금영수증카드
					CPubTckMem::GetInstance()->MakeCsrcCardTicketPrtData();
				}
			}
			else if(pyn_mns_dvs_cd == PYM_CD_CARD)
			{	/// 신용카드
				nAccPymDvs = IDX_ACC_CARD;
				CPubTckMem::GetInstance()->MakeCreditTicketPrtData(FALSE);
			}
			else if(pyn_mns_dvs_cd == PYM_CD_RF)
			{	/// RF교통카드
				//nAccPymDvs = IDX_ACC_RF;
				nAccPymDvs = IDX_ACC_CARD;

				switch(rf_trcr_dvs_cd)
				{
				case 't' : // 티머니 선불
				case 'c' : // 캐시비 선불
					CPubTckMem::GetInstance()->MakeRfCardTicketPrtData();
					break;
				case 'M' : // Mifare 후불
				case 'T' : // 티머니 후불
				default:
					CPubTckMem::GetInstance()->MakeCreditTicketPrtData(TRUE);
					break;
				}
			}
			// 20221017 ADD~
			else if ( (pyn_mns_dvs_cd == PYM_CD_QRPC)
				|| (pyn_mns_dvs_cd == PYM_CD_QRPP)
				|| (pyn_mns_dvs_cd == PYM_CD_QRTP) )	// 20221124 MOD
			{	/// QR결제
				nAccPymDvs = IDX_ACC_CARD;
				CPubTckMem::GetInstance()->MakeCreditTicketPrtData(FALSE);
			}
			// 20221017 ~ADD

			vector<TCK_PRINT_FMT_T>::iterator iter;

			std::sort(CPubTckMem::GetInstance()->m_vtPrtTicket.begin(), CPubTckMem::GetInstance()->m_vtPrtTicket.end(), cmpCcsPrintList);
			bFirst = FALSE;

			for(iter = CPubTckMem::GetInstance()->m_vtPrtTicket.begin(); iter != CPubTckMem::GetInstance()->m_vtPrtTicket.end(); )
			{
				int nSatNo;
				DWORD dwTick;

				::ZeroMemory(card_aprv_no, sizeof(card_aprv_no));

				//sprintf(card_aprv_no, "%s", iter->card_aprv_no);							// 20210817 DEL
				::CopyMemory(card_aprv_no, iter->card_aprv_no, sizeof(card_aprv_no)-1);		// 20210817 ADD

				//TR_LOG_OUT("iter->card_aprv_no(%s) ", iter->card_aprv_no);					// 20210817 ADD for DEBUG
				//TR_LOG_OUT("card_aprv_no(%s) ", card_aprv_no);								// 20210817 ADD for DEBUG

				nSatNo = Util_Ascii2Long(iter->sats_no, strlen(iter->sats_no));

				nRet = TckPrt_GetStatus();
				TR_LOG_OUT("TckPrt_GetStatus(), nRet(%d), bFirst(%d), nSatNo(%d) ..", nRet, bFirst, nSatNo);
				if(nRet < 0)
				{
					if( bFirst == FALSE )
					{
						/// 0x03 : 프린트 오류
						UI_PbTckIssueState(CHAR_ACK, 0x03, nSatNo);
						bFirst = TRUE;
					}
					::Sleep(500);
					continue;
				}
				/* //20230206 ADD->DEL~ // 시외-감열지 승차권 출력 지원하지 않음
				if(GetConfigTicketPapaer() == PAPER_TICKET)
				{	/// 승차권(종이) 프린터
					nRet = TckPrt_GetStatus();
					TR_LOG_OUT("TckPrt_GetStatus(), nRet(%d), bFirst(%d), nSatNo(%d) ..", nRet, bFirst, nSatNo);
					if(nRet < 0)
					{
						if(bFirst == FALSE)
						{
							/// 0x03 : 프린트 오류
							UI_PbTckIssueState(CHAR_ACK, 0x03, nSatNo);
							bFirst = TRUE;
						}
						TR_LOG_OUT("승차권 프린터 상태오류 (%d)!!", nRet);
						::Sleep(500);
						continue;
					}
				}
				else
				{	/// 감열지 프린터 
					nRet = Printer_GetStatus();
					TR_LOG_OUT("Printer_GetStatus(), nRet(%d), bFirst(%d), nSatNo(%d) ..", nRet, bFirst, nSatNo);
					if(nRet < 0)
					{
						if(bFirst == FALSE)
						{
							/// 0x03 : 프린트 오류
							UI_PbTckIssueState(CHAR_ACK, 0x03, nSatNo);
							bFirst = TRUE;
						}
						::Sleep(500);
						continue;
					}
				}
				*/ //20230206 ~ADD->DEL

				// 1. 현장발권 상태 데이타 UI 전송
				UI_PbTckIssueState(CHAR_ACK, 1, nSatNo);	// 진행상태	TISU_STATE HEX(1);	0x01: 발권시작, 0x02: 발권완료, 0x03: 프린터오류

				TR_LOG_OUT("TckPrt_PubPrintTicket START ...");						// 20210817 ADD for DEBUG

				// 2. 승차권 발행	
				nRet = 0;															// 20210817 ADD for DEBUG
				nRet = TckPrt_PubPrintTicket(SVR_DVS_CCBUS, (char *) iter._Ptr);	// 20210817 MOD for DEBUG

				TR_LOG_OUT("TckPrt_PubPrintTicket END ... nRet(%d)", nRet);			// 20210817 ADD for DEBUG

				dwTick = ::GetTickCount();
				//while( Util_CheckExpire(dwTick) < 3500 );	// 20211015 DEL
				//while( Util_CheckExpire(dwTick) < 6500 );	// 20211015 MOD ; '0x02: 발권완료' UI 수신시간이 보통 6초 소요 // 20220706 DEL ; 발권속도 지연 요소 제거
				while( Util_CheckExpire(dwTick) < 1500 );	// 20220706 MOD

				// 3. 현장발권 상태 데이타 UI 전송
				UI_PbTckIssueState(CHAR_ACK, 2, nSatNo);	// 진행상태	TISU_STATE HEX(1);	0x01: 발권시작, 0x02: 발권완료, 0x03: 프린터오류

				/// 회계반영 - 승차권 수량
				AddAccumTicketWork(SVR_DVS_CCBUS, nAccPymDvs, iter->bus_tck_knd_cd, (int) ACC_TICKET_PUB_ISSUE, 1, iter->n_tisu_amt);
				/// 티켓 고유번호 증가
				AddAccumBusTckInhrNo(ACC_TICKET_SEQ_PLUS, SVR_DVS_CCBUS,1);
				iter++;
			}

			/// 2020.04.06 add code
			/// 코로나 19로 인한, 한시적 서비스 
			TR_LOG_OUT("pub_user_tel_no, len(%d), (%s) ..", strlen(CPubTckMem::GetInstance()->base.pub_user_tel_no), CPubTckMem::GetInstance()->base.pub_user_tel_no);
			TR_LOG_OUT("pub_user_krn, len(%d), (%s) ..", strlen(CPubTckMem::GetInstance()->base.pub_user_krn), CPubTckMem::GetInstance()->base.pub_user_krn);
			TR_LOG_OUT("ride_vhcl_dvs, len(%d), (%s) ..", strlen(CPubTckMem::GetInstance()->base.ride_vhcl_dvs), CPubTckMem::GetInstance()->base.ride_vhcl_dvs);

			if( strlen(CPubTckMem::GetInstance()->base.pub_user_krn) > 0 )
			{
				nRet = Svr_IfSv_269();	
				TR_LOG_OUT("Svr_IfSv_269(), nRet(%d) ..", nRet);
			}
			/// ~2020.04.06 add code

			TR_LOG_OUT("UI_TckIssueCompleteState START ... card_aprv_no(%s)", card_aprv_no);	// 20210817 ADD for DEBUG

			nRet = 0;																			// 20210817 ADD for DEBUG
			/// 20200406 add code
			nRet = UI_TckIssueCompleteState(CHAR_ACK, card_aprv_no);							// 20210817 MOD for DEBUG

			TR_LOG_OUT("UI_TckIssueCompleteState END ... nRet(%d)", nRet);						// 20210817 ADD for DEBUG

			UI_AddDevAccountInfo();
			SetStep(1);
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckKobus_IssueState
 * @details		[현장발권]-[코버스] 승차권 발권 프린트
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckKobus_IssueState(void)
{
	int		nRet = 0;

	try
	{
		if(s_nPrevStep != s_nStep)
		{
			TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
			SetPrevState(s_nStep);
		}

		switch(s_nStep)
		{
		case 0 :
			{	/// 코버스 - 승차권 발권 
				BOOL bFirst;
				int nAccPymCD;
				char pyn_mns_dvs_cd;
				char ui_csrc_dvs_cd;
				char chit_use_dvs;
				char card_aprv_no[100+1];
				CPubTckKobusMem* pTR;

				pTR = CPubTckKobusMem::GetInstance();

				pyn_mns_dvs_cd = CPubTckKobusMem::GetInstance()->base.ui_pym_dvs_cd[0];
				ui_csrc_dvs_cd = CPubTckKobusMem::GetInstance()->base.ui_csrc_dvs_cd[0];
				chit_use_dvs = CPubTckKobusMem::GetInstance()->m_tReqTckIssue.chit_use_dvs[0] - 0x30;

				TR_LOG_OUT("지불수단(%d), 현금영수증구분(%d), 수기입력구분(%d)..", pyn_mns_dvs_cd, ui_csrc_dvs_cd, chit_use_dvs);

				nAccPymCD = IDX_ACC_CARD;

				bFirst = FALSE;

 				if(pyn_mns_dvs_cd == PYM_CD_CASH)
 				{	/// 현금결제
 					nAccPymCD = IDX_ACC_CASH;
 				}
 				else if(pyn_mns_dvs_cd == PYM_CD_CSRC)
 				{	/// 현금영수증 결제
 					nAccPymCD = IDX_ACC_CASH;
 				}
 				else
 				{
 					nAccPymCD = IDX_ACC_CARD;
 				}

				/// 신용카드
				//pTR->MakeCreditTicketPrtData();
				//pTR->MakeAllTicketPrtData(pyn_mns_dvs_cd);
				pTR->MakeAllTicketPrtData(pyn_mns_dvs_cd, ui_csrc_dvs_cd, chit_use_dvs);

				vector<TCK_PRINT_FMT_T>::iterator iter;

				TR_LOG_OUT("승차권 발권 갯수 (%d) ", pTR->m_vtPrtTicket.size());

				std::sort(pTR->m_vtPrtTicket.begin(), pTR->m_vtPrtTicket.end(), cmpKobusPrintList);

				for(iter = pTR->m_vtPrtTicket.begin(); iter != pTR->m_vtPrtTicket.end(); )
				{
					int nSatNo;
					DWORD dwTick;

					::ZeroMemory(card_aprv_no, sizeof(card_aprv_no));

					sprintf(card_aprv_no, "%s", iter->card_aprv_no);

					nSatNo = Util_Ascii2Long(iter->sats_no, strlen(iter->sats_no));

					if(GetConfigTicketPapaer() == PAPER_TICKET)
					{	/// 승차권
						nRet = TckPrt_GetStatus();
						TR_LOG_OUT("TckPrt_GetStatus(), nRet(%d), bFirst(%d), nSatNo(%d) ..", nRet, bFirst, nSatNo);
						if(nRet < 0)
						{
							if(bFirst == FALSE)
							{
								/// 0x03 : 프린트 오류
								UI_PbTckIssueState(CHAR_ACK, 0x03, nSatNo);
								bFirst = TRUE;
							}
							TR_LOG_OUT("승차권 프린터 상태오류 (%d)!!", nRet);
							::Sleep(500);
							continue;
						}
					}
					else
					{
						nRet = Printer_GetStatus();
						TR_LOG_OUT("Printer_GetStatus(), nRet(%d), bFirst(%d), nSatNo(%d) ..", nRet, bFirst, nSatNo);
						if(nRet < 0)
						{
							if(bFirst == FALSE)
							{
								/// 0x03 : 프린트 오류
								UI_PbTckIssueState(CHAR_ACK, 0x03, nSatNo);
								bFirst = TRUE;
							}
							::Sleep(500);
							continue;
						}
					}

					// 1. 현장발권 상태 데이타 UI 전송
					UI_PbTckIssueState(CHAR_ACK, 1, nSatNo);
					// 2. 승차권 발행	
					///org
					///TckPrt_PubPrintTicket(SVR_DVS_KOBUS, (char *) iter._Ptr);

					/// modify 2020.05.18
					if(GetConfigTicketPapaer() == PAPER_ROLL)
					{	/// 감열지
						Printer_TicketPrint(SVR_DVS_KOBUS, (int)enFUNC_PBTCK, (char *) iter._Ptr);
					}
					else
					{
						TckPrt_PubPrintTicket(SVR_DVS_KOBUS, (char *) iter._Ptr);
					}
					/// ~modify 2020.05.18
					dwTick = ::GetTickCount();
					//while( Util_CheckExpire(dwTick) < 3500 );	// 20211015 DEL
					while( Util_CheckExpire(dwTick) < 1500 );	// 20220706 MOD	// 20220816 ADD

					// 3. 현장발권 상태 데이타 UI 전송
					UI_PbTckIssueState(CHAR_ACK, 2, nSatNo);

					TR_LOG_OUT("승차권발행_코버스 : 지불수단:%d, 티켓종류(%s), 금액(%d", nAccPymCD, iter->bus_tck_knd_cd, iter->n_tisu_amt);

					/// 회계반영 - 승차권 수량
					if(GetConfigTicketPapaer() == PAPER_ROLL)
					{	/// 감열지 용지
						//AddAccumTicketWork(SVR_DVS_KOBUS, nAccPymCD, iter->bus_tck_knd_cd, (int) ACC_TICKET_PUB_ISSUE, 0, iter->n_tisu_amt);
						AddAccumTicketWork_Rev1(SVR_DVS_KOBUS, nAccPymCD, iter->bus_tck_knd_cd, (int) ACC_TICKET_PUB_REV1_ISSUE, 1, iter->n_tisu_amt, PAPER_ROLL);
					}
					else
					{
						AddAccumTicketWork(SVR_DVS_KOBUS, nAccPymCD, iter->bus_tck_knd_cd, (int) ACC_TICKET_PUB_ISSUE, 1, iter->n_tisu_amt);
					}
					/// 티켓 고유번호 증가
					AddAccumBusTckInhrNo(ACC_TICKET_SEQ_PLUS, SVR_DVS_KOBUS, 1);
					iter++;
				}

				/// 20200406 add code
				UI_TckIssueCompleteState(CHAR_ACK, card_aprv_no);

				UI_AddDevAccountInfo();
				SetStep(98);
			}
			break;
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		TrPbTckTmExp_IssueState
 * @details		[현장발권]-[티머니고속] 승차권 발권 프린트
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckTmExp_IssueState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 
			char pyn_mns_dvs_cd;
			char ui_csrc_dvs_cd;
//			char rf_trcr_dvs_cd;
			int nAccPymDvs;
			BOOL bFirst;
			char card_aprv_no[100+1];

			pyn_mns_dvs_cd = CPubTckTmExpMem::GetInstance()->base.pyn_mns_dvs_cd[0];
			ui_csrc_dvs_cd = CPubTckTmExpMem::GetInstance()->base.ui_csrc_dvs_cd[0];
			//rf_trcr_dvs_cd = CPubTckTmExpMem::GetInstance()->base.rf_trcr_dvs_cd[0];

			TR_LOG_OUT("[티머니고속] 결제구분코드 = 0x%02X", pyn_mns_dvs_cd);
			TR_LOG_OUT("[티머니고속] 현금영수증 구분코드 = 0x%02X", ui_csrc_dvs_cd);
			//TR_LOG_OUT("RF 구분코드 = 0x%02X", rf_trcr_dvs_cd);

			nAccPymDvs = IDX_ACC_CARD;

			if(pyn_mns_dvs_cd == PYM_CD_CASH)
			{	/// 현금결제
				nAccPymDvs = IDX_ACC_CASH;
				CPubTckTmExpMem::GetInstance()->MakeCashTicketPrtData();
			}
			else if(pyn_mns_dvs_cd == PYM_CD_CSRC)
			{	/// 현금영수증 결제
				nAccPymDvs = IDX_ACC_CASH;

				/***
				if(	(ui_csrc_dvs_cd == 1) || (ui_csrc_dvs_cd == 3) )
				{	// 수기 현금영수증
					CPubTckTmExpMem::GetInstance()->MakeCsrcTicketPrtData();
				}
				else
				{	// 현금영수증카드
					CPubTckTmExpMem::GetInstance()->MakeCsrcCardTicketPrtData();
				}
				***/
				CPubTckTmExpMem::GetInstance()->MakeCsrcTicketPrtData();
			}
			else if(pyn_mns_dvs_cd == PYM_CD_CARD)
			{	/// 신용카드
				nAccPymDvs = IDX_ACC_CARD;
				CPubTckTmExpMem::GetInstance()->MakeCreditTicketPrtData();
			}
			else
			{
				TR_LOG_OUT("[티머니고속] 결제 구분코드 unknown !!!!!");
			}

			vector<TCK_PRINT_FMT_T>::iterator iter;

			std::sort(CPubTckTmExpMem::GetInstance()->m_vtPrtTicket.begin(), CPubTckTmExpMem::GetInstance()->m_vtPrtTicket.end(), cmpCcsPrintList);
			bFirst = FALSE;

			TR_LOG_OUT("[티머니고속] 티켓발권 수량 = %d ...", CPubTckTmExpMem::GetInstance()->m_vtPrtTicket.size());

			for(iter = CPubTckTmExpMem::GetInstance()->m_vtPrtTicket.begin(); iter != CPubTckTmExpMem::GetInstance()->m_vtPrtTicket.end(); )
			{
				int nSatNo;
				DWORD dwTick;

				::ZeroMemory(card_aprv_no, sizeof(card_aprv_no));

				sprintf(card_aprv_no, "%s", iter->card_aprv_no);

				nSatNo = Util_Ascii2Long(iter->sats_no, strlen(iter->sats_no));

				if(GetConfigTicketPapaer() == PAPER_TICKET)
				{	/// 승차권
					nRet = TckPrt_GetStatus();
					TR_LOG_OUT("TckPrt_GetStatus(), nRet(%d), bFirst(%d), nSatNo(%d) ..", nRet, bFirst, nSatNo);
					if(nRet < 0)
					{
						if( bFirst == FALSE )
						{
							/// 0x03 : 프린트 오류
							UI_PbTckIssueState(CHAR_ACK, 0x03, nSatNo);
							bFirst = TRUE;
						}
						TR_LOG_OUT("승차권 프린터 상태오류 (%d)!!", nRet);
						::Sleep(500);
						continue;
					}
				}
				else
				{
					nRet = Printer_GetStatus();
					TR_LOG_OUT("Printer_GetStatus(), nRet(%d), bFirst(%d), nSatNo(%d) ..", nRet, bFirst, nSatNo);
					if(nRet < 0)
					{
						if(bFirst == FALSE)
						{
							/// 0x03 : 프린트 오류
							UI_PbTckIssueState(CHAR_ACK, 0x03, nSatNo);
							bFirst = TRUE;
						}
						::Sleep(500);
						continue;
					}
				}

				// 1. 현장발권 상태 데이타 UI 전송
				UI_PbTckIssueState(CHAR_ACK, 1, nSatNo);
				// 2. 승차권 발행	
				//TckPrt_PubPrintTicket(SVR_DVS_TMEXP, (char *) iter._Ptr);
				if(GetConfigTicketPapaer() == PAPER_ROLL)
				{	/// 감열지
					Printer_TicketPrint(SVR_DVS_TMEXP, (int)enFUNC_PBTCK, (char *) iter._Ptr);
				}
				else
				{
					TckPrt_PubPrintTicket(SVR_DVS_TMEXP, (char *) iter._Ptr);
				}


				dwTick = ::GetTickCount();
				//while( Util_CheckExpire(dwTick) < (1000 * 3) );
				//while( Util_CheckExpire(dwTick) < 3500 );	// 20211015 DEL
				while( Util_CheckExpire(dwTick) < 1500 );	// 20220706 MOD	// 20220816 ADD

				// 3. 현장발권 상태 데이타 UI 전송
				UI_PbTckIssueState(CHAR_ACK, 2, nSatNo);

				/// 회계반영 - 승차권 수량
				//AddAccumTicketWork(SVR_DVS_TMEXP, nAccPymDvs, iter->bus_tck_knd_cd, (int) ACC_TICKET_PUB_ISSUE, 1, iter->n_tisu_amt);
				/// 회계반영 - 승차권 수량
				if(GetConfigTicketPapaer() == PAPER_ROLL)
				{	/// 감열지 용지
					AddAccumTicketWork_Rev1(SVR_DVS_TMEXP, nAccPymDvs, iter->bus_tck_knd_cd, (int) ACC_TICKET_PUB_REV1_ISSUE, 1, iter->n_tisu_amt, PAPER_ROLL);
				}
				else
				{
					AddAccumTicketWork(SVR_DVS_TMEXP, nAccPymDvs, iter->bus_tck_knd_cd, (int) ACC_TICKET_PUB_ISSUE, 1, iter->n_tisu_amt);
				}

				/// 티켓 고유번호 증가
				AddAccumBusTckInhrNo(ACC_TICKET_SEQ_PLUS, SVR_DVS_TMEXP,1);
				iter++;
			}

			/// 2020.04.06 add code
			/// 코로나 19로 인한, 한시적 서비스 
			TR_LOG_OUT("pub_user_tel_no, len(%d), (%s) ..", strlen(CPubTckMem::GetInstance()->base.pub_user_tel_no), CPubTckMem::GetInstance()->base.pub_user_tel_no);
			TR_LOG_OUT("pub_user_krn, len(%d), (%s) ..", strlen(CPubTckMem::GetInstance()->base.pub_user_krn), CPubTckMem::GetInstance()->base.pub_user_krn);
			TR_LOG_OUT("ride_vhcl_dvs, len(%d), (%s) ..", strlen(CPubTckMem::GetInstance()->base.ride_vhcl_dvs), CPubTckMem::GetInstance()->base.ride_vhcl_dvs);

			if( strlen(CPubTckMem::GetInstance()->base.pub_user_krn) > 0 )
			{
				nRet = Svr_IfSv_269();	
				TR_LOG_OUT("Svr_IfSv_269(), nRet(%d) ..", nRet);
			}
			/// ~2020.04.06 add code

			/// 20200406 add code
			UI_TckIssueCompleteState(CHAR_ACK, card_aprv_no);

			UI_AddDevAccountInfo();
			SetStep(1);
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckChangeMoneyState
 * @details		현장발권 거스름돈 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckChangeMoneyState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 
			int					nChangeMoney;
			int					nTmp10k, nTmp1k, nTmp100, nTmp500;
			int					nBillACK, nCoinACK;
			int					nOut100, nOut500;
			UI_SND_CHG_MONEY_T	Info;
			DISPENSE_INFO_T		dspInfo;

			nTmp10k = nTmp1k = nTmp100 = nTmp500 = 0;
			nOut100 = nOut500 = 0;
			nBillACK = nCoinACK = CHAR_ACK;
			::ZeroMemory(&dspInfo, sizeof(DISPENSE_INFO_T));

			::ZeroMemory(&Info, sizeof(UI_SND_CHG_MONEY_T));

			nChangeMoney = CPubTckMem::GetInstance()->base.n_pbtck_chg_money;

			TR_LOG_OUT("[01]. nChangeMoney = %d ", nChangeMoney);

			if( INI_Use10K() )
			{
				nTmp10k = nChangeMoney / 10000;
				nChangeMoney -= (10000 * nTmp10k);
				TR_LOG_OUT("[02]. nChangeMoney = %d, nTmp10k = %d ", nChangeMoney, nTmp10k);
			}

			nTmp1k = nChangeMoney / 1000;
			nChangeMoney -= (1000 * nTmp1k);
			TR_LOG_OUT("[03]. nChangeMoney = %d, nTmp1k = %d ", nChangeMoney, nTmp1k);

			nTmp500 = nChangeMoney / 500;
			nChangeMoney -= (500 * nTmp500);
			TR_LOG_OUT("[04]. nChangeMoney = %d, nTmp500 = %d ", nChangeMoney, nTmp500);

			nTmp100 = nChangeMoney / 100;
			nChangeMoney -= (100 * nTmp100);
			TR_LOG_OUT("[05]. nChangeMoney = %d, nTmp100 = %d ", nChangeMoney, nTmp100);

			Info.byAck = CHAR_ACK;
			Info.bySvrDvs = CConfigTkMem::GetInstance()->GetUiSvrKind();

			if( (nTmp10k + nTmp1k) > 0 )
			{
				::ZeroMemory(&dspInfo, sizeof(DISPENSE_INFO_T));

				nRet = CDU_Dispense(nTmp1k, nTmp10k);
				if(nRet < 0)
				{
					nBillACK = Info.byAck = CHAR_NAK;		///> 
				}
				else
				{
					nBillACK = Info.byAck = CHAR_ACK;		///> 
				}
				
				CDU_GetDispenseInfo((char *)&dspInfo);

				if( (dspInfo.nOutCount[0] + dspInfo.nOutCount[1]) > 0 )
				{
					/// 회계반영
					if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_CCBUS )
					{
						TR_LOG_OUT("[시외버스] 방출금액, 1k(%d), 10k(%d)", dspInfo.nOutCount[0], dspInfo.nOutCount[1]);
						CPubTckMem::GetInstance()->base.outBill.n1k = dspInfo.nOutCount[0];
						CPubTckMem::GetInstance()->base.outBill.n5k = 0;
						CPubTckMem::GetInstance()->base.outBill.n10k = dspInfo.nOutCount[1];
						CPubTckMem::GetInstance()->base.outBill.n50k = 0;
					}
					else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_KOBUS )
					{
						TR_LOG_OUT("[코버스] 방출금액, 1k(%d), 10k(%d)", dspInfo.nOutCount[0], dspInfo.nOutCount[1]);
						CPubTckKobusMem::GetInstance()->base.outBill.n1k = dspInfo.nOutCount[0];
						CPubTckKobusMem::GetInstance()->base.outBill.n5k = 0;
						CPubTckKobusMem::GetInstance()->base.outBill.n10k = dspInfo.nOutCount[1];
						CPubTckKobusMem::GetInstance()->base.outBill.n50k = 0;
					}
					else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_TMEXP )
					{
						TR_LOG_OUT("[티머니고속] 방출금액, 1k(%d), 10k(%d)", dspInfo.nOutCount[0], dspInfo.nOutCount[1]);
						CPubTckTmExpMem::GetInstance()->base.outBill.n1k = dspInfo.nOutCount[0];
						CPubTckTmExpMem::GetInstance()->base.outBill.n5k = 0;
						CPubTckTmExpMem::GetInstance()->base.outBill.n10k = dspInfo.nOutCount[1];
						CPubTckTmExpMem::GetInstance()->base.outBill.n50k = 0;
					}
					else
					{
						TR_LOG_OUT("[버스_에러] 방출금액 ????");
					}

					AddAccumBillData(ACC_BILL_MINUS, dspInfo.nOutCount[0], 0, dspInfo.nOutCount[1], 0);
				}

				if(Info.byAck == CHAR_NAK)
				{	/// 미방출시 방출된 금액만큼만 저장
					if( (dspInfo.nOutCount[0] + dspInfo.nOutCount[1]) > 0)
					{
						/// 입출내역 TR (출금)
						TR_LOG_OUT("[#01] 미방출시 - 지폐방출 (%4d : %4d) ", dspInfo.nOutCount[0], dspInfo.nOutCount[1]);
						//AddCashTrData(DT_FLAG_OUT_MONEY, (WORD)0, (WORD)0, (WORD)dspInfo.nOutCount[0], 0, (WORD)dspInfo.nOutCount[1], 0);
					}
				}

				if( GetEventCode(EC_CDU_NO_OUT) )
				{
					int n1k, n10k;

 					n1k  = nTmp1k - dspInfo.nOutCount[0];
 					n10k = nTmp10k - dspInfo.nOutCount[1];
//  					n1k  = dspInfo.nOutCount[0] - nTmp1k;
//  					n10k = dspInfo.nOutCount[1] - nTmp10k;

					/// 회계반영
					AddAccumUnPaidData(ACC_UNPAID, 0, 0, n1k, n10k);

					/// 입출내역 TR (미출금)
					TR_LOG_OUT("[#02] 미방출 지폐 (%4d : %4d) ", n1k, n10k);
					//AddCashTrData(DT_FLAG_UNPAID, (WORD)0, (WORD)0, (WORD)n1k, 0, (WORD)n10k, 0);
				}
			}

// 			if( Info.byAck == CHAR_NAK )
// 			{
// 				UI_AddFailInfo(UI_CMD_PBTCK_CHG_MONEY_RET, "LOC997", NULL);
// 				SetStep(1);
// 				break;
// 			}

			if( (nTmp500 + nTmp100) > 0 )
			{
				nRet = Coin_ChangeMoney(nTmp100, nTmp500);
				if(nRet < 0)
				{
					nCoinACK = Info.byAck = CHAR_NAK;
				}
				else
				{
					nCoinACK = Info.byAck = CHAR_ACK;
				}

				Coin_GetOutInfo(&nOut100, &nOut500);
				Info.w100 = (WORD) nOut100;		///> 동전 - 100원
				Info.w500 = (WORD) nOut500;		///> 동전 - 500원
			
				/// 회계반영
				if( (Info.w100 + Info.w500) > 0 )
				{
					if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_CCBUS )
					{
						TR_LOG_OUT("[시외버스] 방출금액, 100(%d), 500(%d)", nOut100, nOut500);
						CPubTckMem::GetInstance()->base.outCoin.n10 = 0;
						CPubTckMem::GetInstance()->base.outCoin.n50 = 0;
						CPubTckMem::GetInstance()->base.outCoin.n100 = nOut100;
						CPubTckMem::GetInstance()->base.outCoin.n500 = nOut500;
					}
					else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_KOBUS )
					{
						TR_LOG_OUT("[코버스] 방출금액, 100(%d), 500(%d)", nOut100, nOut500);
						CPubTckKobusMem::GetInstance()->base.outCoin.n10 = 0;
						CPubTckKobusMem::GetInstance()->base.outCoin.n50 = 0;
						CPubTckKobusMem::GetInstance()->base.outCoin.n100 = nOut100;
						CPubTckKobusMem::GetInstance()->base.outCoin.n500 = nOut500;
					}
					else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_TMEXP )
					{
						TR_LOG_OUT("[티머니고속] 방출금액, 100(%d), 500(%d)", nOut100, nOut500);
						CPubTckTmExpMem::GetInstance()->base.outCoin.n10 = 0;
						CPubTckTmExpMem::GetInstance()->base.outCoin.n50 = 0;
						CPubTckTmExpMem::GetInstance()->base.outCoin.n100 = nOut100;
						CPubTckTmExpMem::GetInstance()->base.outCoin.n500 = nOut500;
					}
					else
					{
						TR_LOG_OUT("[버스_에러] 동전 방출금액 ????");
					}

					AddAccumCoinData(ACC_COIN_MINUS, nOut100, nOut500);
				}

				if( Info.byAck == CHAR_NAK )
				{	/// 미방출시 방출된 금액만큼만 저장
					if( (Info.w100 + Info.w500) > 0 )
					{	
						/// 입출내역 TR (출금)
						TR_LOG_OUT("[#03] 미방출시 - 동전방출 (%4d : %4d) ", Info.w100, Info.w500);
						//AddCashTrData(DT_FLAG_OUT_MONEY, (WORD)Info.w100, (WORD)Info.w500, 0, 0, 0, 0);
					}
				}

				if( GetEventCode(EC_COIN_NO_OUT) )
				{	/// 동전 미방출
					int n100, n500;

					n100 = nTmp100 - nOut100;
					n500 = nTmp500 - nOut500;

					/// 회계반영
					AddAccumUnPaidData(ACC_UNPAID, n100, n500, 0, 0);
					
					/// 입출내역 TR (미출금)
					TR_LOG_OUT("[#04] 미방출 동전 (%4d : %4d) ", n100, n500);
					//AddCashTrData(DT_FLAG_UNPAID, (WORD)n100, (WORD)n500, 0, 0, 0, 0);
				}
			}

#if 0		/// 무조건 실패..
			nBillACK = CHAR_NAK;

//			if( Info.byAck == CHAR_ACK )
			if(	(nBillACK == CHAR_ACK) && (nCoinACK == CHAR_ACK) )
			{
				Info.w1k = nTmp1k;			///> 지폐 - 1,000원
				Info.w10k = nTmp10k;		///> 지폐 - 10,000원
				Info.w100 = nTmp100;		///> 동전 - 100원
				Info.w500 = nTmp500;		///> 동전 - 500원

				/// 입출내역 TR (출금)
				TR_LOG_OUT("[#05] 방출 갯수 (%4d : %4d), (%4d : %4d) ", nTmp100, nTmp500, nTmp1k, nTmp10k);
				AddCashTrData(DT_FLAG_OUT_MONEY, (WORD)Info.w100, (WORD)Info.w500, (WORD)Info.w1k, 0, (WORD)Info.w10k, 0);

				/// 
				UI_AddQueueInfo(UI_CMD_PBTCK_CHG_MONEY_RET, (char *)&Info, sizeof(UI_SND_CANCRY_CHG_MONEY_T));
			}
			else
			{
				// 실패
				AddCashTrData(DT_FLAG_OUT_MONEY, (WORD)nOut100, (WORD)nOut500, (WORD)dspInfo.nOutCount[0], 0, (WORD)dspInfo.nOutCount[1], 0);

				// 현금 미방출
				UI_AddFailInfo(UI_CMD_PBTCK_CHG_MONEY_RET, "SVE555", NULL);
			}
#else
			/// 무조건 방출 성공...
			{
				Info.byAck = CHAR_ACK;
				Info.w1k  = dspInfo.nOutCount[0];	///> 지폐 - 1,000원
				Info.w10k = dspInfo.nOutCount[1];	///> 지폐 - 10,000원
				Info.w100 = nOut100;				///> 동전 - 100원
				Info.w500 = nOut500;				///> 동전 - 500원

				/// 입출내역 TR (출금)
				TR_LOG_OUT("[#05-1] pg_방출 갯수 (%4d : %4d), (%4d : %4d) ", nTmp100, nTmp500, nTmp1k, nTmp10k);
				TR_LOG_OUT("[#05-2] real_방출 갯수 (%4d : %4d), (%4d : %4d) ", nOut100, nOut500, dspInfo.nOutCount[0], dspInfo.nOutCount[1]);
				AddCashTrData(DT_FLAG_OUT_MONEY, (WORD)Info.w100, (WORD)Info.w500, (WORD)Info.w1k, 0, (WORD)Info.w10k, 0);

				/// 
				UI_AddQueueInfo(UI_CMD_PBTCK_CHG_MONEY_RET, (char *)&Info, sizeof(UI_SND_CANCRY_CHG_MONEY_T));
			}
#endif
			SetStep(1);
		}
		break;

	case 1 :
		{
			UI_AddDevAccountInfo();
			SetStep(2);
		}
		break;
	case 2 :
		{
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckCardReadState
 * @details		현장발권 신용카드 읽기 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckCardReadState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckCompleteState
 * @details		현장발권 완료 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckCompleteState(void)
{
	int		nRet = 0, nCount = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 
			nRet = Bill_Inhibit();
			nRet = Bill_TotalEnd();
			nRet = Coin_TotalEnd();

			UI_AddDevAccountInfo();
			
			AddSmsTRData((int) enFUNC_PBTCK, CConfigTkMem::GetInstance()->n_bus_dvs);

			/// 2020.02.24 RF 결제 실패 시, 좌석 선점 해제..
			if( GetEventCode(EC_RF_PAYMENT_ERR) )
			{
				TR_LOG_OUT("RF 결제 실패시, 좌석 선점 해제 요청 !!!");
				
				nCount = CPubTckMem::GetInstance()->m_vtPcpysats.size();
				if( nCount > 0 )
				{
					nRet = Svr_IfSv_202(FALSE);
					if(nRet > 0)
					{
						TR_LOG_OUT("좌석 선점 해제 성공 !!!");
					}
					else
					{
						TR_LOG_OUT("좌석 선점 해제 실패 !!!");
					}
				}
			}

			CPubTckMem::GetInstance()->Initialize();
			CPubTckKobusMem::GetInstance()->Initialize();
			CPubTckTmExpMem::GetInstance()->Initialize();

			SetState(TR_READY_STATE, 0);
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckCancelState
 * @details		현장발권 취소 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckCancelState(void)
{
	int		nRet = 0;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 좌석선점 해제
			int nCount = 0;

			nRet = Bill_Inhibit();
			nRet = Bill_TotalEnd();
			nRet = Coin_TotalEnd();

			nCount = CPubTckMem::GetInstance()->m_vtPcpysats.size();
			if( nCount > 0 )
			{
				nRet = Svr_IfSv_202(FALSE);
				if(nRet > 0)
				{
					TR_LOG_OUT("[시외] 좌석 선점 해제 성공 !!!");
				}
				else
				{
					TR_LOG_OUT("[시외] 좌석 선점 해제 실패 !!!");
				}
			}

			nCount = CPubTckKobusMem::GetInstance()->m_tResSatsPcpy.size();
			if( nCount > 0 )
			{
				nRet = Kobus_TK_PcpySatsCancel();
				if(nRet > 0)
				{
					TR_LOG_OUT("[코버스] 좌석 선점 해제 성공 !!!\n");
				}
				else
				{
					TR_LOG_OUT("[코버스] 좌석 선점 해제 실패 !!!\n");
				}
			}

			nCount = CPubTckTmExpMem::GetInstance()->m_tResSatsPcpy.size();
			if( nCount > 0 )
			{
				nRet = TmExp_TK_PcpySatsCancel();
				if(nRet > 0)
				{
					TR_LOG_OUT("[티머니고속] 좌석 선점 해제 성공 !!!\n");
				}
				else
				{
					TR_LOG_OUT("[티머니고속] 좌석 선점 해제 실패 !!!\n");
				}
			}

			SetStep(1);
		}
		break;
	case 1 :
		{
//			int n1k, n10k;

			Bill_TotalEnd();

			/***
			n1k  = CPubTckMem::GetInstance()->base.insBill.n1k;
			n10k = CPubTckMem::GetInstance()->base.insBill.n10k;

			// 지폐방출기
			if( (n1k + n10k) > 0 )
			{
				DISPENSE_INFO_T dspInfo;

				::ZeroMemory(&dspInfo, sizeof(DISPENSE_INFO_T));

				nRet = CDU_Dispense(n1k, n10k);
				if(nRet < 0)
				{
					;
				}
				else
				{
					;
				}
				
				CDU_GetDispenseInfo((char *)&dspInfo);

				/// 회계반영
				AddAccumBillData(ACC_BILL_MINUS, dspInfo.nOutCount[0], 0, dspInfo.nOutCount[1], 0);

				/// 입출내역 TR (출금)
				if( (dspInfo.nOutCount[0] + dspInfo.nOutCount[1]) > 0 )
				{
					TR_LOG_OUT("[#01] 방출 갯수 (%4d : %4d) ", dspInfo.nOutCount[0], dspInfo.nOutCount[1]);
					AddCashTrData(DT_FLAG_OUT_MONEY, 0, 0, (WORD)dspInfo.nOutCount[0], 0, (WORD)dspInfo.nOutCount[1], 0);
				}

				if( GetEventCode(EC_BILL_NO_OUT) )
				{
					int nTmp1k, nTmp10k;

					nTmp1k  = n1k - dspInfo.nOutCount[0];
					nTmp10k = n10k - dspInfo.nOutCount[1];

					AddAccumUnPaidData(ACC_UNPAID, 0, 0, nTmp1k, nTmp10k);

					/// 입출내역 TR (미출금)
					TR_LOG_OUT("[#02] 미방출 (%4d : %4d) ", nTmp1k, nTmp10k);
					AddCashTrData(DT_FLAG_UNPAID, 0, 0, (WORD)nTmp1k, 0, (WORD)nTmp10k, 0);
				}
			}
			***/
			
			/// 장비 회계데이타 전송
			UI_AddDevAccountInfo();
			CPubTckMem::GetInstance()->Initialize();
			CPubTckKobusMem::GetInstance()->Initialize();
			CPubTckTmExpMem::GetInstance()->Initialize();

			SetState(TR_READY_STATE, 0);
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrPbTckState
 * @details		현장발권
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckState(void)
{
	switch(s_nState)
	{
	case TR_PBTCK_MAIN_STATE:			// 현장발권 - 시작
		TrPbTckMainState();
		break;
	case TR_PBTCK_REQ_LIST:				// 현장발권 - [시외] 배차조회 
		TrPbTckReqListState();
		break;
	case TR_PBTCK_KOBUS_REQ_LIST:		// 현장발권 - [코버스] 배차조회 
		TrPbTckKobus_ReqListState();
		break;
	case TR_PBTCK_TMEXP_REQ_LIST:		// 현장발권 - [티머니고속] 배차조회 
		TrPbTckTmExp_ReqListState();
		break;

	case TR_PBTCK_KOBUS_REQ_SATS:		// 현장발권 - [코버스] 좌석정보 조회
		TrPbTckKobus_ReqSatsState();
		break;
	case TR_PBTCK_KOBUS_REQ_SATS_PCPY:	// 현장발권 - [코버스] 좌석선점 요청
		TrPbTckKobus_ReqSatsPcpyState();
		break;
	case TR_PBTCK_KOBUS_REQ_THRU:		// 현장발권 - [코버스] 경유지 정보 요청
		TrPbTckKobus_ReqThruState();
		break;
	case TR_PBTCK_LIST_SEL:				// 현장발권 - [시외] 배차 리스트 선택완료
		TrPbTckListSelectState();
		break;
	case TR_PBTCK_TMEXP_REQ_SATS:		// 현장발권 - [티머니고속] 좌석정보 조회
		TrPbTckTmExp_ReqSatsState();
		break;
	case TR_PBTCK_TMEXP_REQ_SATS_PCPY:	// 현장발권 - [티머니고속] 좌석선점 요청
		TrPbTckTmExp_ReqSatsPcpyState();
		break;
	case TR_PBTCK_TMEXP_REQ_THRU:		// 현장발권 - [티머니고속] 경유지 정보 요청
		TrPbTckTmExp_ReqThruState();
		break;

	// 20191008 insert by nhso
	case TR_PBTCK_STAFF_REQ:			// 현장발권 - [시외_인천공항] 상주직원 조회
		TrPbTckReqStaff();
		break;
	case TR_PBTCK_STAFF_CD_MOD_FARE:	// 현장발권 - [시외_인천공항] 상주직원_카드_요금변경 정보조회
		TrPbTckReqStaffCdModFare();
		break;

	case TR_PBTCK_RF_PAYMENT:			// 현장발권 - [시외] RF 선불카드 결제 요청
		TrPbTckReqRfCardPayment();
		break;
	//~ 20191008 insert by nhso

	case TR_PBTCK_SEAT_SEL:				// 현장발권 - [시외] 좌석 정보 선택완료
		TrPbTckSetSeatState();
		break;
	case TR_PBTCK_CORONA_INPUT:			// 현장발권 - [시외] 개인정보 입력
		TrPbTckCoronaState();
		break;

	case TR_PBTCK_PYM_DVS_SEL:			// 현장발권 - 결제수단 선택완료
		TrPbTckPaySelectState();
		break;
	case TR_PBTCK_INS_BILL:				// 현장발권 - 지폐투입
		TrPbTckBillInsertState();
		break;
	case TR_PBTCK_STOP_BILL:			// 현장발권 - 지폐투입
		TrPbTckBillStopState();
		break;
	case TR_PBTCK_CSRC_READ:			// 현장발권 - 현금영수증 카드 읽기 시작/종료
		TrPbTckCsrcReadState();
		break;
	case TR_PBTCK_CSRC_INPUT:			// 현장발권 - 현금영수증 번호 입력
		TrPbTckCsrcInputState();
		break;
	
	case TR_PBTCK_TMAX_ISSUE:			// 현장발권 - [시외] TMAX 승차권 발권
		TrPbTckTmaxIssueState();
		break;
	case TR_PBTCK_KOBUS_TMAX_ISSUE:		// 현장발권 - [코버스] - TMAX 승차권 발권
		TrPbTckKobus_TmaxIssueState();
		break;
	case TR_PBTCK_TMEXP_TMAX_ISSUE:		// 현장발권 - [티머니고속] - TMAX 승차권 발권
		TrPbTck_TmExp_TmaxIssueState();
		break;

	case TR_PBTCK_TCK_ISSUE:			// 현장발권 - [시외] 승차권 프린트
		TrPbTckIssueState();
		break;
	case TR_PBTCK_KOBUS_TCK_ISSUE :		// 현장발권 - [코버스] 승차권 프린트
		TrPbTckKobus_IssueState();
		break;
	case TR_PBTCK_TMEXP_TCK_ISSUE:		// 현장발권 - [티머니고속] 승차권 프린트
		TrPbTckTmExp_IssueState();
		break;

	// 20221205 ADD~
	case TR_PBTCK_REQ_QRMDPCPYSATS:		// 현장발권 - [시외] QR 좌석 선점 시간변경 
		TrPbTckReqQrMdPcpySats();
		break;
	// 20221205 ~ADD

	case TR_PBTCK_CHANGE_MONEY:			// 현장발권 - 거스름돈 
		TrPbTckChangeMoneyState();
		break;
	case TR_PBTCK_CARD_READ:			// 현장발권 - 신용카드 읽기 시작/종료
		TrPbTckCardReadState();
		break;
	case TR_PBTCK_COMPLETE_STATE:		// 현장발권 - 완료
		TrPbTckCompleteState();
		break;
	case TR_PBTCK_CANCEL_STATE:			// 현장발권 - 취소
		TrPbTckCancelState();
		break;
	}

	return 0;
}

/**
 * @brief		TrErrorState
 * @details		점검중 화면
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrErrorState(void)
{
	int		nRet = 0;
	static DWORD dwTick;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		//s_nPrevStep = s_nStep;
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{
			dwTick = GetTickCount();
			SetStep(1);
		}
		break;
	case 1 :
		{
		}
		break;
	}
	
	return 0;
}

/**
 * @brief		TrAdminLogin
 * @details		관리자 화면 로그인
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrAdminLogin(void)
{
	int		nRet = 0;
	static DWORD dwTick;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		//s_nPrevStep = s_nStep;
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		break;
	}

	return 0;
}

/**
 * @brief		TrAdminLogout
 * @details		관리자 화면 로그아웃
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrAdminLogout(void)
{
	int		nRet = 0;
	static DWORD dwTick;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		//s_nPrevStep = s_nStep;
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{
			UI_AddDevAccountInfo();
			//UI_AddDevConfigData();
			UI_Add_Q_SendData(UI_CMD_DEV_CONFIG, TRUE, 1);
			UI_AddTrmlInfo();

			Svr_DataDownload(TRUE);
			SetStep(1);
		}
		break;
	case 1 :
		{
		}
		break;
	}
	
	return 0;
}

/**
 * @brief		TrAdminNoOutCashState
 * @details		[관리자] 미방출 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrAdminNoOutCashState(void)
{
	int		nRet = 0;
	int		nCount = 0;
	//REQ_NO_OUT_CASH reqInfo;
	UI_SND_NOOUT_CASH_T	reqInfo;

	if(s_nPrevStep != s_nStep)
	{
		TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
		SetPrevState(s_nStep);
	}

	switch(s_nStep)
	{
	case 0 :
		{	/// 
			int nOut1k, nOut10k, nOut100, nOut500;

			nOut1k = nOut10k = nOut100 = nOut500 = 0;

			::ZeroMemory(&reqInfo, sizeof(UI_SND_NOOUT_CASH_T));

			reqInfo.byACK = CHAR_ACK;
			reqInfo.wCount = 1;
			::CopyMemory(reqInfo.szDateTime, CConfigTkMem::GetInstance()->reqNoOutCash.szDate, sizeof(reqInfo.szDateTime));
			::CopyMemory(reqInfo.szWndNo, CConfigTkMem::GetInstance()->reqNoOutCash.szWndNo, sizeof(reqInfo.szWndNo));
			reqInfo.byFlag = 1;
			reqInfo.w100 = CConfigTkMem::GetInstance()->reqNoOutCash.w100;
			reqInfo.w500 = CConfigTkMem::GetInstance()->reqNoOutCash.w500;
			reqInfo.w1k = CConfigTkMem::GetInstance()->reqNoOutCash.w1k;
			reqInfo.w5k = CConfigTkMem::GetInstance()->reqNoOutCash.w5k;
			reqInfo.w10k = CConfigTkMem::GetInstance()->reqNoOutCash.w10k;
			reqInfo.w50k = CConfigTkMem::GetInstance()->reqNoOutCash.w50k;

			TR_LOG_OUT("미방출 정보 - 100(%d), 500(%d), 1k(%d), 10k(%d) !!", reqInfo.w100, reqInfo.w500, reqInfo.w1k, reqInfo.w10k);

			// 미방출금 - 지폐방출
			if( (reqInfo.w1k + reqInfo.w10k) > 0)
			{
				DISPENSE_INFO_T dspInfo;
//				int i, nLoop;
				int nMAX = 250;
				int n1k, n10k;

				n1k = reqInfo.w1k;
				n10k = reqInfo.w10k;

				::ZeroMemory(&dspInfo, sizeof(DISPENSE_INFO_T));

				nRet = CDU_Dispense(n1k, n10k);
				TR_LOG_OUT("CDU_Dispense(), nRet(%d) !!", nRet);
				
				CDU_GetDispenseInfo((char *)&dspInfo);

				nOut1k  = dspInfo.nOutCount[0];
				nOut10k  = dspInfo.nOutCount[1];

				TR_LOG_OUT("CDU_Dispense(), 방출된 갯수, 1k(%d), 10k(%d) !!", nOut1k, nOut10k);

				if( (nOut1k + nOut10k) > 0 )
				{
					AddAccumBillData(ACC_BILL_MINUS, nOut1k, 0, nOut10k, 0);
				}
			}

			// 동전방출
			if( (reqInfo.w100 + reqInfo.w500) > 0 )
			{
				nRet = Coin_Reset(TRUE, TRUE);
				TR_LOG_OUT("Coin_Reset(), nRet(%d) !!", nRet);
				//nRet = Coin_GetStatus();
				Sleep(500);

				nRet = Coin_ChangeMoney(reqInfo.w100, reqInfo.w500);
				TR_LOG_OUT("Coin_ChangeMoney(), nRet(%d) !!", nRet);

				Coin_GetOutInfo(&nOut100, &nOut500);
				TR_LOG_OUT("Coin_ChangeMoney(), 방출된 갯수, 100(%d), 500(%d) !!", nOut100, nOut500);

				if( (nOut100 + nOut500) > 0 )
				{
					AddAccumCoinData(ACC_COIN_MINUS, nOut100, nOut500);
				}
			}

			//if( (reqInfo.w100 + reqInfo.w500 + reqInfo.w1k + reqInfo.w10k) > 0 )
			if( (nOut100 + nOut500 + nOut1k + nOut10k) > 0 )
			{
				AddAccumUnPaidData(ACC_UNPAID_MINUS, nOut100, nOut500, nOut1k, nOut10k);
			}

			reqInfo.byFlag = 1;
			reqInfo.w100 = nOut100;
			reqInfo.w500 = nOut500;
			reqInfo.w1k = nOut1k;
			reqInfo.w5k = 0;
			reqInfo.w10k = nOut10k;
			reqInfo.w50k = 0;

			// 미방출결과 전송
			UI_NoOutCasgResultInfo((char *)&reqInfo);

			// 장비회계 전송
			UI_AddDevAccountInfo();

			{
				int					nCoin, nBill;
				PFILE_ACCUM_N1010_T	pAccum = GetAccumulateData();
				PDM_COIN_T			pNoCoin;
				PDM_BILL_T			pNoBill;

				pNoCoin = &pAccum->Curr.noCoin;
				pNoBill = &pAccum->Curr.noBill;

				nCoin = pNoCoin->n10 + pNoCoin->n50 + pNoCoin->n100 + pNoCoin->n500;
				if(nCoin <= 0)
				{
					SetCheckEventCode(EC_COIN_NO_OUT, FALSE);
				}
				nBill = pNoBill->n1k + pNoBill->n5k + pNoBill->n10k + pNoBill->n50k;
				if(nCoin <= 0)
				{
					SetCheckEventCode(EC_CDU_NO_OUT, FALSE);
				}
			}

			SetStep(1);
		}
		break;
	case 1 :
		{	/// 
			//SetStep(1);
		}
		break;
	}

	return 0;
}

/**
 * @brief		TrAdminReceiptPrint
 * @details		관리자 화면 - 영수증 프린트
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrAdminReceiptPrint(void)
{
	int		nRet = 0;
	static DWORD dwTick;

	try
	{
		if(s_nPrevStep != s_nStep)
		{
			TR_LOG_OUT("s_nStep(%d) !!!\n", s_nStep);
			//s_nPrevStep = s_nStep;
			SetPrevState(s_nStep);
		}

		switch(s_nStep)
		{
		case 0 :
			{
				BYTE byPrtKind;

				byPrtKind = CConfigTkMem::GetInstance()->n_print_kind & 0xFF;

				switch( byPrtKind )
				{
				case 1 :	/// 현재시재 프린트 (사용안함)
					Printer_AccountInfo();
					break;
				case 2 :	/// 창구마감 프린트
					Printer_Account2Info();
					break;
				case 3 :	/// 시재마감 프린트
					Printer_Account3Info();
					break;
				}

				UI_AddQueueInfo(UI_CMD_ACC_PRT_RESP, (char *)&byPrtKind, 1);
				SetStep(1);
			}
			break;
		case 1 :
			{
			}
			break;
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	return 0;
}

/**
 * @brief		TrAdminState
 * @details		관리자 화면 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrAdminState(void)
{
	switch(s_nState)
	{
	case TR_ADMIN_MAIN:
		break;
	case TR_ADMIN_LOGIN:
		TrAdminLogin();
		break;
	case TR_ADMIN_LOGOUT:
		TrAdminLogout();
		break;
	case TR_ADMIN_NO_OUT_CASH:
		TrAdminNoOutCashState();
		break;
	case TR_ADMIN_RECEIPT_PRINT:// 관리자 - 영수증프린트
		TrAdminReceiptPrint();
		break;
	}

	return 0;
}

/**
 * @brief		Transaction_OnCardCashMain
 * @details		현장발권
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Transaction_OnCardCashMain(int nState, int nPrevStep, int nStep)
{
	int nChange = 0;
	static DWORD dwTick = 0;

	if(s_nState != nState)
	{
		s_nState = nState;
		nChange += 1;
	}
	if(s_nStep != nStep)
	{
		s_nStep = nStep;
		nChange += 2;
	}
	if(s_nPrevStep != nPrevStep)
	{
		//s_nPrevStep = nPrevStep;
	}

	if(nChange > 0)
	{
		;//TR_LOG_OUT("s_nState(%d), s_nStep(%d), s_nPrevStep(%d)", s_nState, s_nStep, s_nPrevStep);
	}

	SetAutoCloseEvent();

	switch(s_nState)
	{
	case TR_READY_STATE:
	case TR_ERROR_STATE:

	case TR_PBTCK_MAIN_STATE:			// 현장발권 - 시작
	case TR_PBTCK_REQ_LIST:				// 현장발권 - [시외] 배차조회 
	case TR_PBTCK_LIST_SEL:				// 현장발권 - [시외] 배차 리스트 선택완료

	case TR_ADMIN_MAIN:
	case TR_ADMIN_LOGIN:
	case TR_ADMIN_LOGOUT:
	case TR_ADMIN_NO_OUT_CASH:
	case TR_ADMIN_RECEIPT_PRINT:
		CheckCashDevice();
		break;
	}

// 	if((s_nState == TR_READY_STATE) || (s_nState == TR_ERROR_STATE) || (s_nState == TR_PBTCK_MAIN_STATE) )
// 	{	/// main, error, 현장발권 메인 상태
// 		CheckCashDevice();
// 	}

	switch(s_nState)
	{
	case TR_BOOT_STATE :
		TrBootState();
		break;
	case TR_INIT_STATE :
		TrInitState();
		break;
	case TR_READY_STATE :
		TrReadyState();
		break;

	case TR_MRS_MAIN_STATE :	
	case TR_MRS_CARD_READ_STATE:
	case TR_MRS_INPUT_STATE :
	case TR_MRS_LIST_STATE :
	case TR_MRS_KOBUS_LIST_STATE :
	case TR_MRS_TMEXP_LIST_STATE :
	case TR_MRS_TMEXP_KTC_LIST_STATE:
	case TR_MRS_ISSUE_STATE :
	case TR_MRS_KOBUS_ISSUE_STATE :
	case TR_MRS_TMEXP_ISSUE_STATE :
	case TR_MRS_TMEXP_MOBILE_ISSUE_STATE :
	case TR_MRS_KOBUS_TCK_PRT_STATE :
	case TR_MRS_TMEXP_TCK_PRT_STATE:
	case TR_MRS_COMPLETE_STATE :
	case TR_MRS_CANCEL_STATE :
		TrMrsState();
		break;

	case TR_CANCRY_MAIN_STATE:
	case TR_CANCRY_TCK_READ:
	case TR_CANCRY_FARE_STATE:
	case TR_CANCRY_COMPLETE_STATE:
	case TR_CANCRY_CANCEL_STATE:
		TrCancRyState();
		break;

	case TR_PBTCK_MAIN_STATE:			// 현장발권 - 시작
	case TR_PBTCK_REQ_LIST:				// 현장발권 - [시외] 배차조회 
	case TR_PBTCK_KOBUS_REQ_LIST :		// 현장발권 - [코버스] 배차조회 
	case TR_PBTCK_TMEXP_REQ_LIST:		// 현장발권 - [티머니고속] 배차조회 
	case TR_PBTCK_KOBUS_REQ_SATS :		// 현장발권 - [코버스] 좌석정보 조회
	case TR_PBTCK_KOBUS_REQ_SATS_PCPY :	// 현장발권 - [코버스] 좌석선점 요청
	case TR_PBTCK_KOBUS_REQ_THRU :		// 현장발권 - [코버스] 경유지 요청
	case TR_PBTCK_LIST_SEL:				// 현장발권 - [시외] 배차 리스트 선택완료

	case TR_PBTCK_TMEXP_REQ_SATS:		// 현장발권 - [티머니고속] 좌석정보 조회
	case TR_PBTCK_TMEXP_REQ_SATS_PCPY:	// 현장발권 - [티머니고속] 좌석선점 요청
	case TR_PBTCK_TMEXP_REQ_THRU:		// 현장발권 - [코버스] 경유지 정보 요청

	// 20191008 insert by nhso
	case TR_PBTCK_STAFF_REQ:			// 현장발권 - [시외_인천공항] 상주직원 조회
	case TR_PBTCK_STAFF_CD_MOD_FARE:	// 현장발권 - [시외_인천공항] 상주직원_카드_요금변경 정보조회
	case TR_PBTCK_RF_PAYMENT:			// 현장발권 - [시외] RF 선불카드 결제 요청
	//~ 20191008 insert by nhso

	case TR_PBTCK_REQ_QRMDPCPYSATS:		// 현장발권 - [시외] QR 좌석 선점 시간변경 // 20221205 ADD

	case TR_PBTCK_SEAT_SEL:				// 현장발권 - [시외] 좌석 정보 선택완료
	case TR_PBTCK_PYM_DVS_SEL:			// 현장발권 - 결제수단 선택완료
	case TR_PBTCK_INS_BILL:				// 현장발권 - 지폐투입 시작
	case TR_PBTCK_STOP_BILL:			// 현장발권 - 지폐투입 중지
	case TR_PBTCK_CSRC_READ:			// 현장발권 - 현금영수증 카드 읽기 시작/종료
	case TR_PBTCK_CSRC_INPUT:			// 현장발권 - 현금영수증 번호 입력
	case TR_PBTCK_TMAX_ISSUE:			// 현장발권 - [시외] TMAX 승차권 발권
	case TR_PBTCK_KOBUS_TMAX_ISSUE:		// 현장발권 - [코버스] 승차권 발권
	case TR_PBTCK_TMEXP_TMAX_ISSUE:		// 현장발권 - [티머니고속] - TMAX 승차권 발권
	case TR_PBTCK_TCK_ISSUE:			// 현장발권 - [시외] 승차권 프린트
	case TR_PBTCK_KOBUS_TCK_ISSUE :		// 현장발권 - [코버스] 승차권 프린트
	case TR_PBTCK_TMEXP_TCK_ISSUE :		// 현장발권 - [티머니고속] 승차권 프린트
	case TR_PBTCK_CHANGE_MONEY:			// 현장발권 - 거스름돈 방출
	case TR_PBTCK_CARD_READ:			// 현장발권 - 신용카드 읽기 시작/종료
	case TR_PBTCK_COMPLETE_STATE:		// 현장발권 - 완료
	case TR_PBTCK_CANCEL_STATE:			// 현장발권 - 취소
		TrPbTckState();
		break;

	case TR_ERROR_STATE :		// 점검중 화면
		TrErrorState();
		break;

	case TR_ADMIN_MAIN:			// 관리자 메인
	case TR_ADMIN_LOGIN:		// 관리자 - Login
	case TR_ADMIN_LOGOUT:		// 관리자 - LogOut
	case TR_ADMIN_NO_OUT_CASH:	// 관리자 - 미방출 처리 
	case TR_ADMIN_RECEIPT_PRINT:// 관리자 - 영수증프린트
		TrAdminState();
		break;
	}

	return 0;
}
