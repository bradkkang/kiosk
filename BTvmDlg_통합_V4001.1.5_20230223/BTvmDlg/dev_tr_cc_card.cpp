// 
// 
// dev_tr_cc_card.cpp : 카드 전용 거래
//

#include "stdafx.h"
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <queue>
#include <iostream>

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
#include "dev_prt_main.h"
#include "dev_prt_ticket_main.h"
#include "svr_main.h"
#include "svr_ccs_main.h"
#include "svr_ko_main.h"
#include "dev_tr_main.h"
#include "dev_tr_mem.h"
#include "dev_tr_kobus_mem.h"
#include "data_main.h"
#include "dev_tr_cc_card.h"

static int			s_nStep, s_nPrevStep = -1;
static int			s_nState;  

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		SetState
 * @details		State Value 설정
 * @param		int nState		State Value		
 * @param		int nStep		Step Value
 * @return		성공 >= 0, 실패 < 0
 */
static void SetState(int nState, int nStep)
{
	s_nState = nState;
	Transact_SetState(nState, nStep);
}

/**
 * @brief		SetPrevState
 * @details		이전 State Value 설정
 * @param		int nStep		이전 Step Value
 * @return		성공 >= 0, 실패 < 0
 */
static void SetPrevState(int nStep)
{
	s_nPrevStep = nStep;
	Transact_SetPrevStep(nStep);
}

/**
 * @brief		SetStep
 * @details		Step Value 설정
 * @param		int nValue		Step Value
 * @return		성공 >= 0, 실패 < 0
 */
static void SetStep(int nValue)
{
	s_nStep = nValue;
	Transact_SetStep(nValue);
}

/**
 * @brief		CheckCardDevice
 * @details		장비 상태 체크
 * @param		DWORD dwStartTick		시작 Tick
 * @return		성공 >= 0, 실패 < 0
 */
static int CheckCardDevice(void)
{
	CheckTicketCount();
	// 동전 수량 체크
	//CheckCoinCount();
	// 지폐 수량 체크
	//CheckBillCount();
	// 자동마감 체크
	CheckAutoClose();

	//if( Util_CheckExpire(*dwStartTick >= 2000) )	
	{
		Printer_GetStatus();
		TckPrt_GetStatus();
	}

	if( GetCondition() < 0 )
	{
		SetCheckEventCode(EC_OUT_SERVICE, FALSE);
	}
	else
	{
		SetCheckEventCode(EC_OUT_SERVICE, TRUE);
	}

	return 0;
}

/**
 * @brief		TrBootState
 * @details		Boot State 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrBootState(void)
{
	try
	{
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
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		TrInitState
 * @details		Init State 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrInitState(void)
{
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
			{
				// 			char szTmpTicket[100];
				// 			char szSecuCode[100];
				// 
				// 			::ZeroMemory(szTmpTicket, sizeof(szTmpTicket));
				// 			::ZeroMemory(szSecuCode, sizeof(szSecuCode));
				// 
				// 			sprintf(szTmpTicket, "201908050500400351");
				// 			GetTicketSecurityCode((BYTE *)szTmpTicket, strlen(szTmpTicket), szSecuCode);
				// 			// passwd = 3bf484955983715063d4503299525280
				// 			TRACE("szSecuCode = %s \n", szSecuCode);

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
				SetState(TR_READY_STATE, 0);
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
 * @brief		TrReadyState
 * @details		Ready State 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrReadyState(void)
{
	int		nRet = 0;
	static DWORD dwTick;

	try
	{
		if(s_nPrevStep != s_nStep)
		{
			TR_LOG_OUT("s_nStep(%d) !!!", s_nStep);
			//s_nPrevStep = s_nStep;
			SetPrevState(s_nStep);
		}

		switch(s_nStep)
		{
		case 0 :
			dwTick = GetTickCount();
			SetStep(1);
			break;
		case 1 :
			if( Util_CheckExpire(dwTick) >= 1000 )	
			{
				SetStep(2);
				dwTick = GetTickCount();
			}
			break;
		case 2 :
// 			// 승차권 수량 체크
// 			CheckTicketCount();
// 			// 동전 수량 체크
// 			//CheckCoinCount();
// 			// 지폐 수량 체크
// 			//CheckBillCount();
// 			// 자동마감 체크
// 			CheckAutoClose();
// 
// 			if( Util_CheckExpire(dwTick) >= 2000 )	
// 			{
// 				Printer_GetStatus();
// 				TckPrt_GetStatus();
// 				dwTick = GetTickCount();
// 			}
// 
// 			if( GetCondition() < 0 )
// 			{
// 				SetCheckEventCode(EC_OUT_SERVICE, FALSE);
// 			}
// 			else
// 			{
// 				SetCheckEventCode(EC_OUT_SERVICE, TRUE);
// 			}
			break;
		}
	}
	catch (...)
	{
		TR_LOG_OUT("error_msg = %d", ::GetLastError());
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
			{	// 
				CMrnpMem::GetInstance()->Initialize();
				CMrnpKobusMem::GetInstance()->Initialize();
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
 * @brief		TrMrsReqListState
 * @details		예매발권 - 예매리스트 요청 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsReqListState(void)
{
	int		nRet = 0, i = 0, nCount = 0;
	BYTE	byACK = 0;

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
			{	/// idle 상태...
				nRet = Svr_IfSv_137();
				TR_LOG_OUT("[시외] 예매리스트 요청, nRet(%d) !!!", nRet);
				if(nRet > 0)
				{	/// 성공
					/***
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
					TRACE("Mrnp_List packet info, nCount(%d), nOffset(%d), nSize(%d) ..\n", nCount, nOffset, nSize);

					UI_AddQueueInfo(UI_CMD_MRS_RESP_LIST, (char *)pSPacket, nSize);

					if(pSPacket != NULL)
					{
						delete pSPacket;
					}
					***/
					UI_AddQueueInfo(UI_CMD_MRS_RESP_LIST, (char *)&nRet, 4);
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
				;	
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
 * @brief		TrMrsKobus_ReqListState
 * @details		예매발권 - [코버스] 예매리스트 요청 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsKobus_ReqListState(void)
{
	int		nRet = 0, i = 0, nCount = 0;
	BYTE	byACK = 0;

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
			{	/// idle 상태...
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

	n_sats_1 = Util_Ascii2Long((char *)p1.sats_no, strlen((char *)p1.sats_no));
	n_sats_2 = Util_Ascii2Long((char *)p2.sats_no, strlen((char *)p2.sats_no));

	if(n_sats_1 < n_sats_2)
		return TRUE;
	return FALSE;
}

/**
 * @brief		TrMrsIssueState
 * @details		예매발권 - [시외] 예매 발행 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsIssueState(void)
{
	int		nRet = 0, i = 0, nCount = 0;

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
			{	
				/// 시외버스 - 예매발행
				nRet = Svr_IfSv_138();
				if(nRet >= 0)
				{
					SetStep(1);
				}
				else
				{
					SetStep(98);
					UI_AddFailInfo(UI_CMD_MRS_ISSUE_STATE, CConfigTkMem::GetInstance()->GetErrorCode(), CConfigTkMem::GetInstance()->GetErrorContents());
				}
			}
			break;
		case 1 :
			{	// 시외버스 - 예매 발권 프린트하기
				BOOL bFirst;
				vector<TCK_PRINT_FMT_T>::iterator	iter;

				CMrnpMem::GetInstance()->MakeTicketPrtData();

				std::sort(CMrnpMem::GetInstance()->m_vtPrtTicket.begin(), CMrnpMem::GetInstance()->m_vtPrtTicket.end(), cmpCcsPrintList);
				bFirst = FALSE;

				for(iter = CMrnpMem::GetInstance()->m_vtPrtTicket.begin(); iter != CMrnpMem::GetInstance()->m_vtPrtTicket.end();)
				{
					int nSatNo;
					DWORD dwTick;

					dwTick = ::GetTickCount();
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

					while( Util_CheckExpire(dwTick) < (1000 * 3) );

					// 3. 예매발권 상태 데이타 UI 전송
					UI_MrsIssueState(CHAR_ACK, 0x02, nSatNo);

					TR_LOG_OUT("예매발행_시외 : 티켓종류(%s), 금액(%d", iter->bus_tck_knd_cd, iter->n_tisu_amt);

					/// 회계반영 - 승차권 수량
					AddAccumTicketWork(SVR_DVS_CCBUS, IDX_ACC_CARD, iter->bus_tck_knd_cd, (int) ACC_TICKET_MRNP_ISSUE, 1, iter->n_tisu_amt);
					/// 티켓 고유번호 증가
					AddAccumBusTckInhrNo(ACC_TICKET_SEQ_PLUS, SVR_DVS_CCBUS, 1);
					iter++;
				}
				UI_AddDevAccountInfo();
				SetStep(10);
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
 * @brief		TrMrsKobusIssueState
 * @details		예매발권 - [코버스] 발권 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsKobusIssueState(void)
{
	int		nRet = 0, i = 0, nCount = 0;

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
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
			{	// 코버스 - 예매 발권 프린트하기
				BOOL bFirst;
				CMrnpKobusMem* pMRS;
				vector<TCK_PRINT_FMT_T>::iterator	iter;

				pMRS = CMrnpKobusMem::GetInstance();
				pMRS->MakeTicketPrtData();

				std::sort(pMRS->m_vtPrtTicket.begin(), pMRS->m_vtPrtTicket.end(), cmpKobusPrintList);
				bFirst = FALSE;

				for(iter = pMRS->m_vtPrtTicket.begin(); iter != pMRS->m_vtPrtTicket.end(); )
				{
					int nSatNo;
					DWORD dwTick;

					dwTick = ::GetTickCount();
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
					TckPrt_MrnpPrintTicket(SVR_DVS_KOBUS, (char *) iter._Ptr);

					while( Util_CheckExpire(dwTick) < (1000 * 3) );

					// 3. 예매발권 상태 데이타 UI 전송
					UI_MrsIssueState(CHAR_ACK, 0x02, nSatNo);

					TR_LOG_OUT("예매발행_코버스 : 티켓종류(%s), 금액(%d", iter->bus_tck_knd_cd, iter->n_tisu_amt);

					/// 회계반영 - 승차권 수량
					AddAccumTicketWork(SVR_DVS_KOBUS, IDX_ACC_CARD, iter->bus_tck_knd_cd, (int) ACC_TICKET_MRNP_ISSUE, 1, iter->n_tisu_amt);
					/// 티켓 고유번호 증가
					AddAccumBusTckInhrNo(ACC_TICKET_SEQ_PLUS, SVR_DVS_KOBUS, 1);
					iter++;
				}
				UI_AddDevAccountInfo();
				SetStep(1);
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
 * @brief		TrMrsCompleteState
 * @details		예매발권 - 발권 완료 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsCompleteState(void)
{
	int		nRet = 0;

	try
	{
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
				/// 장비 회계데이타 전송
				UI_AddDevAccountInfo();
				CMrnpMem::GetInstance()->Initialize();
				CMrnpKobusMem::GetInstance()->Initialize();
				SetState(TR_READY_STATE, 0);
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
 * @brief		TrMrsCancelState
 * @details		예매발권 - 발권 취소 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrMrsCancelState(void)
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
			{
				CMrnpMem::GetInstance()->Initialize();
				CMrnpKobusMem::GetInstance()->Initialize();
				SetState(TR_READY_STATE, 0);
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
		{	// "예매발권" - [고속] List 내역 요청
			TrMrsKobus_ReqListState();
		}
		break;
	case TR_MRS_ISSUE_STATE :
		{	// "예매발권" - [시외] 발권 처리
			TrMrsIssueState();
		}
		break;
	case TR_MRS_KOBUS_ISSUE_STATE :
		{	// "예매발권" - [코버스] 발권 처리
			TrMrsKobusIssueState();
		}
		break;
	case TR_MRS_KOBUS_TCK_PRT_STATE :
		{	// "예매발권" - 예매권 프린트
			TrMrsKobusTckPrtState();
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
 * @brief		TrPbTckMainState
 * @details		현장발권 main state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckMainState(void)
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
			{	
				CPubTckMem::GetInstance()->Initialize();
				CPubTckKobusMem::GetInstance()->Initialize();
				SetStep(1);
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
 * @brief		TrPbTckReqListState
 * @details		현장발권 배차리스트 요청 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckReqListState(void)
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
			{	
				///> 이지_시외버스 - 배차조회
				nRet = Svr_IfSv_130();
				TR_LOG_OUT("[시외] 현장발권 - 배차조회 요청, nRet(%d).", nRet);
				if(nRet < 0)
				{
					UI_AddFailInfo(UI_CMD_PBTCK_RESP_LIST, CConfigTkMem::GetInstance()->GetErrorCode(), CConfigTkMem::GetInstance()->GetErrorContents());
					SetStep(2);
					return -1;
				}

				UI_AddQueueInfo(UI_CMD_PBTCK_RESP_LIST, " ", 1);
				SetStep(1);
			}
			break;
		case 1 :
			{
				;
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
 * @brief		TrPbTckKobus_ReqListState
 * @details		[현장발권] - [코버스] 배차리스트 요청 
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckKobus_ReqListState(void)
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
			{	
				///> 코버스 - 배차조회

				if(0)
				{
					UI_AddKobus_AlcnListInfo(FALSE);
					SetStep(2);
				}
				else
				{
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
			}
			break;
		case 1 :
			{
				;			
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
 * @brief		TrPbTckKobus_ReqSatsState
 * @details		[현장발권] 좌석정보 요청 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckKobus_ReqSatsState(void)
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

	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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

	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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

	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
				nRet = Svr_IfSv_228();
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
 * @details		[현장발권] RF 선불카드 결재 요청 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckReqRfCardPayment(void)
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
			{	/// (260) RF 선불카드 결재 
				//nRet = Svr_IfSv_260();
				if(nRet > 0)
				{	/// 성공
//					UI_Add_Q_SendData(UI_CMD_PBTCK_PRE_RF_PAY_RESP, TRUE, 1, (BYTE *)&nAlcn, (int) sizeof(nAlcn));
					UI_Add_Q_SendData(UI_CMD_PBTCK_PRE_RF_PAY_RESP, TRUE, 1);
				}
				else
				{	/// 실패
					//UI_Add_Q_SendData(UI_CMD_PBTCK_PRE_RF_PAY_RESP, FALSE, nRet, (BYTE *)&nAlcn, (int) sizeof(nAlcn));
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
 * @brief		TrPbTckSetSeatState
 * @details		현장발권 좌석 선택 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckSetSeatState(void)
{
	int		nRet = 0;

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
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
			{	/// 
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
 * @brief		TrPbTckBillInsertState
 * @details		현장발권 지폐투입 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckBillInsertState(void)
{
	int		nRet = 0;

	try
	{
		if(s_nPrevStep != s_nStep)
		{
			TR_LOG_OUT("s_nStep(%d) !!!\n", s_nStep);
			//s_nPrevStep = s_nStep;
			SetPrevState(s_nStep);
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
			{	/// 
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
 * @brief		TrPbTckCsrcInputState
 * @details		현장발권 현금영수증 입력 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckCsrcInputState(void)
{
	int		nRet = 0;

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
			{	/// 
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
 * @brief		TrPbTckTmaxIssueState
 * @details		현장발권 - TMAX 발권 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckTmaxIssueState(void)
{
	int		nRet = 0;

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
		case 0:
			{
				char pyn_mns_dvs_cd = 0;
				char ui_csrc_dvs_cd;

				/// 결제 구분코드 - 0x01:현금결제, 0x02:카드결제, 0x04:RF결제
				pyn_mns_dvs_cd = CPubTckMem::GetInstance()->base.pyn_mns_dvs_cd[0];

				/// 현금영수증 구분코드 - 0:미사용, 1:수기입력, 2:신용카드, 3:현영전용카드
				ui_csrc_dvs_cd = CPubTckMem::GetInstance()->base.ui_csrc_dvs_cd[0];

				// 지불수단코드
				switch(pyn_mns_dvs_cd)	
				{
				case PYM_CD_CARD : // 신용카드
					nRet = Svr_IfSv_221(FALSE);
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
					break;
				case PYM_CD_CSRC :	// 현금영수증
				case PYM_CD_CASH :  // 현금
				default:
					TR_LOG_OUT("카드전용 장비는 현금으로 결제 안됨 !!!");		
					SetStep(3);
					break;
				}
			}
			break;

		case 1 :
			{	/// 
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
 * @brief		TrPbTckKobus_TmaxIssueState
 * @details		현장발권 - [코버스] TMAX 발권 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckKobus_TmaxIssueState(void)
{
	int		nRet = 0;

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
		case 0:
			{
				char pyn_mns_dvs_cd = 0;

				/// 코버스 - TMAX - 현장발권 (현금/카드..)
				pyn_mns_dvs_cd = CPubTckMem::GetInstance()->base.pyn_mns_dvs_cd[0];

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
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
			{	/// 시외버스 - 승차권 발권 
				char pyn_mns_dvs_cd;
				char ui_csrc_dvs_cd;
				int nAccPymCD = 0;
				BOOL bFirst;

				pyn_mns_dvs_cd = CPubTckMem::GetInstance()->base.pyn_mns_dvs_cd[0];
				ui_csrc_dvs_cd = CPubTckMem::GetInstance()->base.ui_csrc_dvs_cd[0];

				if( pyn_mns_dvs_cd != PYM_CD_CARD )
				{
					SetStep(99);
					break;
				}

				nAccPymCD = IDX_ACC_CARD;

				/// 신용카드
				CPubTckMem::GetInstance()->MakeCreditTicketPrtData();

				vector<TCK_PRINT_FMT_T>::iterator iter;

				std::sort(CPubTckMem::GetInstance()->m_vtPrtTicket.begin(), CPubTckMem::GetInstance()->m_vtPrtTicket.end(), cmpCcsPrintList);
				bFirst = FALSE;

				for(iter = CPubTckMem::GetInstance()->m_vtPrtTicket.begin(); iter != CPubTckMem::GetInstance()->m_vtPrtTicket.end(); )
				{
					int nSatNo;
					DWORD dwTick;

					dwTick = ::GetTickCount();
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

					// 1. 현장발권 상태 데이타 UI 전송
					UI_PbTckIssueState(CHAR_ACK, 1, nSatNo);
					// 2. 승차권 발행	
					TckPrt_PubPrintTicket(SVR_DVS_CCBUS, (char *) iter._Ptr);

					while( Util_CheckExpire(dwTick) < (1000 * 3) );

					// 3. 현장발권 상태 데이타 UI 전송
					UI_PbTckIssueState(CHAR_ACK, 2, nSatNo);

					TR_LOG_OUT("승차권발행_시외 : 지불수단:%d, 티켓종류(%s), 금액(%d", nAccPymCD, iter->bus_tck_knd_cd, iter->n_tisu_amt);

					/// 회계반영 - 승차권 수량
					AddAccumTicketWork(SVR_DVS_CCBUS, nAccPymCD, iter->bus_tck_knd_cd, (int) ACC_TICKET_PUB_ISSUE, 1, iter->n_tisu_amt);
					/// 티켓 고유번호 증가
					AddAccumBusTckInhrNo(ACC_TICKET_SEQ_PLUS, SVR_DVS_CCBUS,1);
					iter++;
				}
				UI_AddDevAccountInfo();
				SetStep(1);
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
			TR_LOG_OUT("s_nStep(%d) !!!\n", s_nStep);
			SetPrevState(s_nStep);
		}

		switch(s_nStep)
		{
		case 0 :
			{	/// 코버스 - 승차권 발권 
				BOOL bFirst;
				int nAccPymCD;
//				char ui_csrc_dvs_cd;
				CPubTckKobusMem* pTR;

				pTR = CPubTckKobusMem::GetInstance();

				nAccPymCD = IDX_ACC_CARD;

				bFirst = FALSE;

				/// 신용카드
				//pTR->MakeCreditTicketPrtData();
				pTR->MakeAllTicketPrtData(PYM_CD_CARD, 0, 0);

				vector<TCK_PRINT_FMT_T>::iterator iter;

				std::sort(pTR->m_vtPrtTicket.begin(), pTR->m_vtPrtTicket.end(), cmpKobusPrintList);

				for(iter = pTR->m_vtPrtTicket.begin(); iter != pTR->m_vtPrtTicket.end(); )
				{
					int nSatNo;
					DWORD dwTick;

					dwTick = ::GetTickCount();
					nSatNo = Util_Ascii2Long(iter->sats_no, strlen(iter->sats_no));

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

					// 1. 현장발권 상태 데이타 UI 전송
					UI_PbTckIssueState(CHAR_ACK, 1, nSatNo);
					// 2. 승차권 발행	
					TckPrt_PubPrintTicket(SVR_DVS_KOBUS, (char *) iter._Ptr);

					while( Util_CheckExpire(dwTick) < (1000 * 3) );

					// 3. 현장발권 상태 데이타 UI 전송
					UI_PbTckIssueState(CHAR_ACK, 2, nSatNo);

					TR_LOG_OUT("승차권발행_코버스 : 지불수단:%d, 티켓종류(%s), 금액(%d", nAccPymCD, iter->bus_tck_knd_cd, iter->n_tisu_amt);

					/// 회계반영 - 승차권 수량
					AddAccumTicketWork(SVR_DVS_KOBUS, nAccPymCD, iter->bus_tck_knd_cd, (int) ACC_TICKET_PUB_ISSUE, 1, iter->n_tisu_amt);
					/// 티켓 고유번호 증가
					AddAccumBusTckInhrNo(ACC_TICKET_SEQ_PLUS, SVR_DVS_KOBUS,1);
					iter++;
				}
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
 * @brief		TrPbTckChangeMoneyState
 * @details		현장발권 거스름돈 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckChangeMoneyState(void)
{
	int		nRet = 0;

	try
	{
		if(s_nPrevStep != s_nStep)
		{
			TR_LOG_OUT("s_nStep(%d) !!!\n", s_nStep);
			SetPrevState(s_nStep);
		}

	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
			{	/// 
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
 * @brief		TrPbTckCompleteState
 * @details		현장발권 완료 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckCompleteState(void)
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
			{	/// 
				UI_AddDevAccountInfo();

				AddSmsTRData((int) enFUNC_PBTCK, CConfigTkMem::GetInstance()->n_bus_dvs);

				CPubTckMem::GetInstance()->Initialize();
				CPubTckKobusMem::GetInstance()->Initialize();
				SetState(TR_READY_STATE, 0);
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
 * @brief		TrPbTckCancelState
 * @details		현장발권 취소 state
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrPbTckCancelState(void)
{
	int		nRet = 0;

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
			{	/// 좌석선점 해제
				int nCount = 0;

				nCount = CPubTckMem::GetInstance()->m_vtPcpysats.size();
				if( nCount > 0 )
				{
					nRet = Svr_IfSv_202(FALSE);
					if(nRet > 0)
					{
						TR_LOG_OUT("[시외] 좌석 선점 해제 성공 !!!\n");
					}
					else
					{
						TR_LOG_OUT("[시외] 좌석 선점 해제 실패 !!!\n");
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

				/// 장비 회계데이타 전송
				UI_AddDevAccountInfo();

				CPubTckMem::GetInstance()->Initialize();
				CPubTckKobusMem::GetInstance()->Initialize();
				SetStep(1);
			}
			break;
		case 1 :
			{
				SetState(TR_READY_STATE, 0);
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
	
	// 20191008 insert by nhso
	case TR_PBTCK_STAFF_REQ:			// 현장발권 - [시외_인천공항] 상주직원 조회
		TrPbTckReqStaff();
		break;
	case TR_PBTCK_STAFF_CD_MOD_FARE:	// 현장발권 - [시외_인천공항] 상주직원_카드_요금변경 정보조회
		TrPbTckReqStaffCdModFare();
		break;

	case TR_PBTCK_RF_PAYMENT:			// 현장발권 - [시외] RF 선불카드 결재 요청
		TrPbTckReqRfCardPayment();
		break;
	//~ 20191008 insert by nhso
	
	case TR_PBTCK_SEAT_SEL:				// 현장발권 - [시외] 좌석 정보 선택완료
		TrPbTckSetSeatState();
		break;
	case TR_PBTCK_PYM_DVS_SEL:			// 현장발권 - 결제수단 선택완료
		TrPbTckPaySelectState();
		break;
	case TR_PBTCK_INS_BILL:				// 현장발권 - 지폐투입
		TrPbTckBillInsertState();
		break;
	case TR_PBTCK_CSRC_READ:			// 현장발권 - 현금영수증 카드 읽기 시작/종료
		TrPbTckCsrcReadState();
		break;
	case TR_PBTCK_CSRC_INPUT:			// 현장발권 - 현금영수증 번호 입력
		TrPbTckCsrcInputState();
		break;
	case TR_PBTCK_TMAX_ISSUE:			// 현장발권 - [시외] - TMAX 승차권 발권
		TrPbTckTmaxIssueState();
		break;
	case TR_PBTCK_KOBUS_TMAX_ISSUE:		// 현장발권 - [코버스] - TMAX 승차권 발권
		TrPbTckKobus_TmaxIssueState();
		break;
	case TR_PBTCK_TCK_ISSUE:			// 현장발권 - [시외] 승차권 프린트
		TrPbTckIssueState();
		break;
	case TR_PBTCK_KOBUS_TCK_ISSUE :		// 현장발권 - [코버스] 승차권 프린트
		TrPbTckKobus_IssueState();
		break;
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
				dwTick = GetTickCount();
				SetStep(1);
			}
			break;
		case 1 :
			{
// 				// 승차권 수량 체크
// 				CheckTicketCount();
// 				// 동전 수량 체크
// 				//CheckCoinCount();
// 				// 지폐 수량 체크
// 				//CheckBillCount();
// 				// 자동마감 체크
// 				CheckAutoClose();
// 
// 				if( Util_CheckExpire(dwTick) >= 2000 )	
// 				{
// 					Printer_GetStatus();
// 					TckPrt_GetStatus();
// 					dwTick = GetTickCount();
// 				}
// 
// 				if( GetCondition() < 0 )
// 				{
// 					SetCheckEventCode(EC_OUT_SERVICE, FALSE);
// 				}
// 				else
// 				{
// 					SetCheckEventCode(EC_OUT_SERVICE, TRUE);
// 				}
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
 * @brief		TrAdminLogout
 * @details		관리자 화면 로그아웃
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int TrAdminLogout(void)
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

	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
				case 1 :	/// 현재시재 프린트
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
		break;
	case TR_ADMIN_LOGOUT:
		TrAdminLogout();
		break;
	case TR_ADMIN_NO_OUT_CASH:
		//TrAdminNoOutCashState();
		break;
	case TR_ADMIN_RECEIPT_PRINT:// 관리자 - 영수증프린트
		TrAdminReceiptPrint();
		break;
	}

	return 0;
}


/**
 * @brief		Transaction_OnCardOnlyMain
 * @details		카드전용 transaction
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Transaction_OnCardOnlyMain(int nState, int nPrevStep, int nStep)
{
	static DWORD dwTick = 0;

	if(s_nState != nState)
	{
		s_nState = nState;
	}
	if(s_nStep != nStep)
	{
		s_nStep = nStep;
	}
	if(s_nPrevStep != nPrevStep)
	{
		s_nPrevStep = nPrevStep;
	}

	SetAutoCloseEvent();

	if(dwTick == 0)
	{
		dwTick = GetTickCount();
	}

	if(GetTickCount() - dwTick > 2000)
	{
		dwTick = GetTickCount();
		CheckCardDevice();
	}

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
	case TR_MRS_ISSUE_STATE :
	case TR_MRS_KOBUS_ISSUE_STATE :
	case TR_MRS_KOBUS_TCK_PRT_STATE :
	case TR_MRS_COMPLETE_STATE :
	case TR_MRS_CANCEL_STATE :
		TrMrsState();
		break;

	case TR_PBTCK_MAIN_STATE:			// 현장발권 - 시작
	case TR_PBTCK_REQ_LIST:				// 현장발권 - [시외] 배차조회 
	case TR_PBTCK_KOBUS_REQ_LIST :		// 현장발권 - [코버스] 배차조회 
	case TR_PBTCK_KOBUS_REQ_SATS :		// 현장발권 - [코버스] 좌석정보 조회
	case TR_PBTCK_KOBUS_REQ_SATS_PCPY :	// 현장발권 - [코버스] 좌석선점 요청
	case TR_PBTCK_KOBUS_REQ_THRU :		// 현장발권 - [코버스] 경유지 요청
	case TR_PBTCK_LIST_SEL:				// 현장발권 - 배차 리스트 선택완료
	case TR_PBTCK_SEAT_SEL:				// 현장발권 - 좌석 정보 선택완료
	case TR_PBTCK_PYM_DVS_SEL:			// 현장발권 - 결제수단 선택완료
	case TR_PBTCK_INS_BILL:				// 현장발권 - 지폐투입
	case TR_PBTCK_CSRC_READ:			// 현장발권 - 현금영수증 카드 읽기 시작/종료
	case TR_PBTCK_CSRC_INPUT:			// 현장발권 - 현금영수증 번호 입력
	case TR_PBTCK_TMAX_ISSUE:			// 현장발권 - [시외] TMAX 승차권 발권
	case TR_PBTCK_KOBUS_TMAX_ISSUE:		// 현장발권 - [코버스] TMAX 승차권 발권
	case TR_PBTCK_TCK_ISSUE:			// 현장발권 - [시외] 승차권 프린트
	case TR_PBTCK_KOBUS_TCK_ISSUE :		// 현장발권 - [코버스] 승차권 프린트
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
