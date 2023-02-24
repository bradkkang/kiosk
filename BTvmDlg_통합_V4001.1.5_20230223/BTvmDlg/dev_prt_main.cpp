// 
// 
// dev_prt_main.cpp : 영수증 프린터 MAIN
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "File_Env_ini.h"
#include "dev_prt_main.h"
#include "dev_prt_hs.h"
#include "dev_prt_nice_rxd.h"
#include "dev_tr_main.h"
#include "dev_tr_mem.h"
#include "dev_tr_kobus_mem.h"
#include "dev_tr_tmexp_mem.h"
#include "oper_kos_file.h"
#include "oper_config.h"

//----------------------------------------------------------------------------------------------------------------------

static CPrinterHS*		pPrtHS = NULL;
static CPrinterRxd*		pPrtRXD = NULL;

static PDEV_CFG_T		pEnv = NULL;
static PDEV_CFG_T		pEnvPrtTck = NULL; // 20230206 ADD

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		Printer_Initialize
 * @details		영수증 프린터 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Printer_Initialize(void)
{
	int nRet;

	pEnv = (PDEV_CFG_T) GetEnvPrtReceiptInfo();
	TR_LOG_OUT("영수증 프린터 Use(%d), Model(%d), Port(%d) ", pEnv->nUse, pEnv->nModel, pEnv->nPort);

	// 20230206 ADD~
	// 승차권 프린터 유형 설정 정보 저장
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GetConfigTicketPapaer() s_config_t.base_t.exp_ticket_device[0]; 
	// BYTE ///< (55). 고속버스 승차권 ('1':승차권발권기 프린터, '2':감열지프린터) => // 20230220 고속버스 승차권 (PAPER_ROLL(0x31):감열지프린터, PAPER_TICKET(0x32):승차권프린터)
	// 20230206 'UI-관리자설정값'과 반대임. 고속버스 승차권 ('1':감열지, '2':승차권)
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	pEnvPrtTck = (PDEV_CFG_T) GetEnvPrtTicketInfo();
	TR_LOG_OUT("승차권 프린터 Use(%d), Model(%d), Port(%d) ", pEnvPrtTck->nUse, pEnvPrtTck->nModel, pEnvPrtTck->nPort);
	TR_LOG_OUT("01_GetConfigTicketPapaer (%02X) ", GetConfigTicketPapaer());
	if ( (pEnv->nUse == 1) && (pEnvPrtTck->nUse != 1) ) // 영수증(감열지) 프린터를 승차권 출력용으로 사용하는 경우
	{
		TR_LOG_OUT("영수증(감열지) 프린터를 승차권 출력용으로 사용 !!!!\n");
		//SetConfigTicketPapaer("2"); // 20230206 PAPER_ROLL(define 값을 0x31=>0x32 로 변경) // Config 기본값:'2' // 20230216 MOD PAPER_ROLL(define 값 0x31로 원복)
		SetConfigTicketPapaer("1"); // 20230206 PAPER_ROLL(define 값 0x31) // Config 기본값:'2'					// 20230216 MOD
	}
	// 20230220 ADD~
	else if (GetConfigTicketPapaer() == PAPER_ROLL) // 고속버스 승차권 (PAPER_ROLL(0x31):감열지프린터, PAPER_TICKET(0x32):승차권프린터)
	{
		TR_LOG_OUT("[시외]승차권(종이), [고속]영수증(감열지) 프린터를 출력용으로 사용 !!!!\n");
	}
	// 20230220 ~ADD
	else
	{
		TR_LOG_OUT("승차권(종이) 프린터를 승차권 출력용으로 사용 !!!!\n");
		//SetConfigTicketPapaer("1"); // 20230206 PAPER_TICKET(define 값을 0x32=>0x31 로 변경)	// 20230216 MOD
		SetConfigTicketPapaer("2"); // 20230206 PAPER_TICKET(define 값을 0x32)					// 20230216 MOD
	}
	TR_LOG_OUT("02_GetConfigTicketPapaer (%02X) ", GetConfigTicketPapaer());
	// 20230206 ~ADD

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Printer Device not use !!!!\n");
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_RECEIPT_ATEC_HS :	///> 에이텍 - 영수증프린터
		{
			if( pPrtHS != NULL )
			{
				delete pPrtHS;
				pPrtHS = NULL;
			}

			pPrtHS = new CPrinterHS();
			nRet = pPrtHS->StartProcess(pEnv->nPort);
			if(nRet < 0)
			{
				TRACE("pPrtHS->StartProcess() Failure !!!!\n", nRet);
				TR_LOG_OUT("StartProcess... Failure !!!!\n");
				return nRet;
			}
			else
				TR_LOG_OUT("StartProcess... success !!!!\n");
		}
		break;

	case DEV_RECEIPT_NICE_RXD :	///> 한전금 - 영수증프린터
		{
			if( pPrtRXD != NULL )
			{
				delete pPrtRXD;
				pPrtRXD = NULL;
			}

			pPrtRXD = new CPrinterRxd();
			nRet = pPrtRXD->StartProcess(pEnv->nPort);
			if(nRet < 0)
			{
				TRACE("pPrtRXD->StartProcess() Failure !!!!\n", nRet);
				return nRet;
			}
		}
		break;
	}

	return 0;
}

/**
 * @brief		Printer_Terminate
 * @details		영수증 프린터 종료
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Printer_Terminate(void)
{
	pEnv = (PDEV_CFG_T) GetEnvPrtReceiptInfo();

	TR_LOG_OUT(" use(%d), model_no(%d), start.", pEnv->nUse, pEnv->nModel);

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Printer Device not use !!!!\n");
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_RECEIPT_ATEC_HS :	///> 에이텍 - 영수증프린터
		{
			if( pPrtHS != NULL )
			{
				pPrtHS->EndProcess();

				delete pPrtHS;
				pPrtHS = NULL;
			}
		}
		break;
	case DEV_RECEIPT_NICE_RXD :	///> 한전금 - 영수증프린터
		{
			if( pPrtRXD != NULL )
			{
				pPrtRXD->EndProcess();

				delete pPrtRXD;
				pPrtRXD = NULL;
			}
		}
		break;
	}

	return 0;
}

/**
 * @brief		Printer_GetStatus
 * @details		영수증 프린터 상태 체크
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Printer_GetStatus(void)
{
	int nRet; 

	pEnv = (PDEV_CFG_T) GetEnvPrtReceiptInfo();

	//TR_LOG_OUT(" use(%d), model_no(%d), start...", pEnv->nUse, pEnv->nModel); // 20230206 DEL

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		//TR_LOG_OUT("Printer Device not use !!!!\n"); // 20230206 DEL
		return -1;
	}

	nRet = -2;

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_RECEIPT_ATEC_HS :	///> 에이텍 - 영수증프린터
		{
			if( pPrtHS != NULL )
			{
				//TR_LOG_OUT("pPrtHS is not NULL !!!!\n"); // 20230206 DEL
				nRet = pPrtHS->GetStatus();
				//TR_LOG_OUT("pPrtHS->GetStatus() Return (%d) ", nRet); // 20230206 ADD
			}
			else
				TR_LOG_OUT("pPrtHS is NULL !!!!\n");
		}
		break;

	case DEV_RECEIPT_NICE_RXD :	///> 한전금 - 영수증프린터
		{
			if( pPrtRXD != NULL )
			{
				//TR_LOG_OUT("pPrtRXD is not NULL !!!!\n"); // 20230206 ADD->DEL
				nRet = pPrtRXD->GetStatus();
				//TR_LOG_OUT("pPrtRXD->GetStatus() Return (%d) ", nRet); // 20230206 ADD
			}
		}
		break;
	}

	return nRet;
}

/**
 * @brief		Printer_MakeRefundTicketData
 * @details		환불 데이타 만들기
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Printer_MakeRefundTicketData(char *retBuf)
{
	prt_refund_t		*pPrtInfo;
	POPER_FILE_CONFIG_T pConfig;

	pPrtInfo = (prt_refund_t *) retBuf; 
	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_CCBUS )
	{	/// 시외버스
		CCancRyTkMem* pCancTr;
		PFMT_BARCODE_T pBarCode;

		pCancTr = CCancRyTkMem::GetInstance();
		pBarCode = (PFMT_BARCODE_T) pCancTr->szTicketData;

		sprintf(pPrtInfo->bus_nm, "시외버스");

		/// 가맹점 이름
		sprintf(pPrtInfo->bizr_nm, pConfig->ccTrmlInfo_t.sz_prn_trml_nm);
		/// 가맹점 사업자번호
		sprintf(pPrtInfo->bizr_no, pConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);

		TR_LOG_OUT(" 시외버스... pCancTr->tRespTckNo.pyn_mns_dvs_cd[0]: %c ", pCancTr->tRespTckNo.pyn_mns_dvs_cd[0]);

		/// 결제수단
		switch( pCancTr->tRespTckNo.pyn_mns_dvs_cd[0] )
		{
		case PYM_CD_CASH :
			sprintf(pPrtInfo->pyn_dvs_nm, "현금");
			break;
		case PYM_CD_CSRC :
			sprintf(pPrtInfo->pyn_dvs_nm, "현금영수증");
			break;
		case PYM_CD_CARD :
			sprintf(pPrtInfo->pyn_dvs_nm, "신용카드");
			break;
		case PYM_CD_TPAY :
			sprintf(pPrtInfo->pyn_dvs_nm, "페이");
		// 20211015 ADD~
			break;
		case PYM_CD_COMP :
			sprintf(pPrtInfo->pyn_dvs_nm, "복합결제");
			break;
		// 20211015 ~ADD
		}

		// 20221220 ADD~
		TR_LOG_OUT(" 시외버스... pCancTr->tRespTckNo.qr_pym_pyn_dtl_cd: %s ", pCancTr->tRespTckNo.qr_pym_pyn_dtl_cd);
		/// QR결제지불상세코드
		if( strcmp(pCancTr->tRespTckNo.qr_pym_pyn_dtl_cd, "PC") == 0 )
		{
			sprintf(pPrtInfo->pyn_dvs_nm, "PAYCO-신용카드");
		}
		else if( strcmp(pCancTr->tRespTckNo.qr_pym_pyn_dtl_cd, "PP") == 0 )
		{
			sprintf(pPrtInfo->pyn_dvs_nm, "PAYCO-포인트");
		}
		else if( strcmp(pCancTr->tRespTckNo.qr_pym_pyn_dtl_cd, "TP") == 0 )
		{
			sprintf(pPrtInfo->pyn_dvs_nm, "티머니페이");
		}
		// 20221220 ~ADD

		/// 승차권정보
		sprintf(pPrtInfo->ticket_info, "%.*s-%.*s-%.*s-%.*s-%.*s ", 
			sizeof(pBarCode->pub_dt),			pBarCode->pub_dt, 
			sizeof(pBarCode->pub_shct_trml_cd), pBarCode->pub_shct_trml_cd, 
			sizeof(pBarCode->pub_wnd_no),		pBarCode->pub_wnd_no, 
			sizeof(pBarCode->pub_sno),			pBarCode->pub_sno, 
			sizeof(pBarCode->secu_code),		pBarCode->secu_code);

		/// 발권금액
		#if 1	// 20211125 ADD AmountComma for 시외버스
		{
			int nValue = 0;
			char szFare[100];

			nValue = *(int *)pCancTr->tRespTckNo.tisu_amt;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(pPrtInfo->tisu_amt, "%s", szFare);
		}
		#else
		sprintf(pPrtInfo->tisu_amt, "%d", *(int *)pCancTr->tRespTckNo.tisu_amt);
		#endif

		/// 환불율
		sprintf(pPrtInfo->cmrt	, "%d", pCancTr->n_commission_rate);

		/// 환불금액
		#if 1	// 20211125 ADD AmountComma for 시외버스
		{
			int nValue = 0;
			char szFare[100];

			nValue = pCancTr->n_chg_money;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(pPrtInfo->ry_amt, "%s", szFare);	
		}
		#else
		sprintf(pPrtInfo->ry_amt	, "%d", pCancTr->n_chg_money);
		#endif

		/// 출발일
		sprintf(pPrtInfo->depr_dt	, "%.*s-%.*s-%.*s", 
				4, &pCancTr->tRespTckNo.depr_dt[0], 
				2, &pCancTr->tRespTckNo.depr_dt[4], 
				2, &pCancTr->tRespTckNo.depr_dt[6]);

		/// 출발시간
		sprintf(pPrtInfo->depr_time	, "%.*s:%.*s", 
				2, &pCancTr->tRespTckNo.depr_time[0], 
				2, &pCancTr->tRespTckNo.depr_time[2]);

		/// 출발지
		FindTerminalName(LANG_KOR, pCancTr->tRespTckNo.depr_trml_cd, pPrtInfo->depr_nm);

		/// 도착지
		FindTerminalName(LANG_KOR, pCancTr->tRespTckNo.arvl_trml_cd, pPrtInfo->arvl_nm);

		/// 버스등급
		FindBusClsName(SVR_DVS_CCBUS, pCancTr->tRespTckNo.bus_cls_cd, pPrtInfo->bus_cls_nm);

		/// 좌석번호
		sprintf(pPrtInfo->sat_no	, "%d", *(int *)pCancTr->tRespTckNo.sats_no);
	}
	else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_KOBUS )
	{	/// 코버스 고속
		CCancRyTkKobusMem* pCancTr;
		PFMT_QRCODE_T pQRCode;

		pCancTr = CCancRyTkKobusMem::GetInstance();

		pQRCode = (PFMT_QRCODE_T) pCancTr->tBase.szTicketData;

		sprintf(pPrtInfo->bus_nm, "고속버스");

		/// 가맹점 이름
		sprintf(pPrtInfo->bizr_nm, pConfig->koTrmlInfo_t.sz_prn_trml_nm);
		/// 가맹점 사업자번호
		sprintf(pPrtInfo->bizr_no, pConfig->koTrmlInfo_t.sz_prn_trml_corp_no);

		TR_LOG_OUT(" 코버스 고속... tBase.ui_pym_dvs_cd[0]: %c ", pCancTr->tBase.ui_pym_dvs_cd[0]);

		/// 결제수단
		switch( pCancTr->tBase.ui_pym_dvs_cd[0] )
		{
		case 1 :
			sprintf(pPrtInfo->pyn_dvs_nm, "현금");
			break;
		case 2 :
			sprintf(pPrtInfo->pyn_dvs_nm, "신용카드");
			break;
		case 'd'://스마일페이
		case 'e'://티머니페이
		case 'f'://비즈페이
		case 'g'://페이코
			sprintf(pPrtInfo->pyn_dvs_nm, "페이");
		// 20210910 ADD
			break;
		case 5 :
			sprintf(pPrtInfo->pyn_dvs_nm, "복합결제");
			break;
		// 20210910 ~ADD
		}

		/// 승차권정보
		sprintf(pPrtInfo->ticket_info, "%.*s-%.*s-%.*s-%.*s", 
			sizeof(pQRCode->pub_dt),			pQRCode->pub_dt, 
			sizeof(pQRCode->pub_shct_trml_cd),  pQRCode->pub_shct_trml_cd, 
			sizeof(pQRCode->pub_wnd_no),		pQRCode->pub_wnd_no, 
			sizeof(pQRCode->pub_sno),			pQRCode->pub_sno);

		/// 발권금액
		#if 1	// 20211125 ADD AmountComma for 코버스
		{
			int nValue = 0;
			char szFare[100];

			nValue = *(int *)pCancTr->tRespInq.tissu_fee;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(pPrtInfo->tisu_amt, "%s", szFare);
		}
		#else
		sprintf(pPrtInfo->tisu_amt, "%d", *(int *)pCancTr->tRespInq.tissu_fee);
		#endif

		/// 환불율
		sprintf(pPrtInfo->cmrt	, "%d", pCancTr->tBase.n_commission_rate);

		/// 환불금액
		#if 1	// 20211125 ADD AmountComma for 코버스
		{
			int nValue = 0;
			char szFare[100];

			nValue = pCancTr->tBase.n_chg_money;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(pPrtInfo->ry_amt, "%s", szFare);
		}
		#else
		sprintf(pPrtInfo->ry_amt	, "%d", pCancTr->tBase.n_chg_money);
		#endif

		/// 출발일
		sprintf(pPrtInfo->depr_dt	, "%.*s-%.*s-%.*s", 
			4, &pCancTr->tRespList.depr_dt[0], 
			2, &pCancTr->tRespList.depr_dt[4], 
			2, &pCancTr->tRespList.depr_dt[6]);

		/// 출발시간
		sprintf(pPrtInfo->depr_time	, "%.*s:%.*s", 
			2, &pCancTr->tRespList.depr_time[0], 
			2, &pCancTr->tRespList.depr_time[2]);

		/// 출발지
		Find_KobusTrmlName(LANG_KOR, pCancTr->tRespList.depr_trml_no, pPrtInfo->depr_nm);

		/// 도착지
		Find_KobusTrmlName(LANG_KOR, pCancTr->tRespList.arvl_trml_no, pPrtInfo->arvl_nm);

		/// 버스등급
		FindBusClsName(SVR_DVS_KOBUS, pCancTr->tRespList.bus_cls_cd, pPrtInfo->bus_cls_nm);

		/// 좌석번호
		sprintf(pPrtInfo->sat_no	, "%s", pCancTr->tRespList.sats_no);
	}
	else 
	{	/// 티머니고속
		CCancRyTkTmExpMem* pCancTr;
		PFMT_QRCODE_T pQRCode;

		pCancTr = CCancRyTkTmExpMem::GetInstance();

		pQRCode = (PFMT_QRCODE_T) pCancTr->tBase.szTicketData;

		sprintf(pPrtInfo->bus_nm, "고속버스");

		/// 가맹점 이름
		sprintf(pPrtInfo->bizr_nm, pConfig->ezTrmlInfo_t.sz_prn_trml_nm);
		/// 가맹점 사업자번호
		sprintf(pPrtInfo->bizr_no, pConfig->ezTrmlInfo_t.sz_prn_trml_corp_no);

		TR_LOG_OUT(" 티머니고속... tBase.ui_pym_dvs_cd[0]: 0x%02X ", pCancTr->tBase.ui_pym_dvs_cd[0]);

		/// 결제수단
		switch( pCancTr->tBase.ui_pym_dvs_cd[0] )
		{
		case 1 :
			sprintf(pPrtInfo->pyn_dvs_nm, "현금");
			break;
		case 2 :
			sprintf(pPrtInfo->pyn_dvs_nm, "신용카드");
			break;
		case 'd'://스마일페이
		case 'e'://티머니페이
		case 'f'://비즈페이
		case 'g'://페이코
			sprintf(pPrtInfo->pyn_dvs_nm, "페이");
		// 20211013 ADD
			break;
		case 5 :
			sprintf(pPrtInfo->pyn_dvs_nm, "복합결제");
			break;
		// 20211013 ~ADD
		}

		/// 승차권정보
		sprintf(pPrtInfo->ticket_info, "%.*s-%.*s-%.*s-%.*s", 
			sizeof(pQRCode->pub_dt),			pQRCode->pub_dt, 
			sizeof(pQRCode->pub_shct_trml_cd),  pQRCode->pub_shct_trml_cd, 
			sizeof(pQRCode->pub_wnd_no),		pQRCode->pub_wnd_no, 
			sizeof(pQRCode->pub_sno),			pQRCode->pub_sno);

		/// 발권금액
		#if 1	// 20211125 ADD AmountComma for 티머니고속
		{
			int nValue = 0;
			char szFare[100];

			nValue = *(int *)pCancTr->tRespInq.tissu_fee;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(pPrtInfo->tisu_amt, "%s", szFare);
		}
		#else
		sprintf(pPrtInfo->tisu_amt, "%d", *(int *)pCancTr->tRespInq.tissu_fee);
		#endif

		/// 환불율
		sprintf(pPrtInfo->cmrt	, "%d", pCancTr->tBase.n_commission_rate);

		/// 환불금액
		#if 1	// 20211125 ADD AmountComma for 티머니고속
		{
			int nValue = 0;
			char szFare[100];

			nValue = pCancTr->tBase.n_chg_money;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(pPrtInfo->ry_amt, "%s", szFare);
		}
		#else
		sprintf(pPrtInfo->ry_amt	, "%d", pCancTr->tBase.n_chg_money);
		#endif

		/// 출발일
		sprintf(pPrtInfo->depr_dt	, "%.*s-%.*s-%.*s", 
				4, &pCancTr->tRespInq.depr_dt[0], 
				2, &pCancTr->tRespInq.depr_dt[4], 
				2, &pCancTr->tRespInq.depr_dt[6]);

		/// 출발시간
		sprintf(pPrtInfo->depr_time	, "%.*s:%.*s", 
				2, &pCancTr->tRespInq.depr_time[0], 
				2, &pCancTr->tRespInq.depr_time[2]);

		/// 출발지
		Find_KobusTrmlName(LANG_KOR, pCancTr->tRespInq.depr_trml_no, pPrtInfo->depr_nm);

		/// 도착지
		Find_KobusTrmlName(LANG_KOR, pCancTr->tRespInq.arvl_trml_no, pPrtInfo->arvl_nm);

		/// 버스등급
		FindBusClsName(SVR_DVS_KOBUS, pCancTr->tRespInq.bus_cls_cd, pPrtInfo->bus_cls_nm);

		/// 좌석번호
		sprintf(pPrtInfo->sat_no	, "%s", pCancTr->tRespInq.sats_no);
	}

	return 0;
}

/**
 * @brief		Printer_RefundTicket
 * @details		환불 영수증 
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Printer_RefundTicket(void)
{
	pEnv = (PDEV_CFG_T) GetEnvPrtReceiptInfo();

	TR_LOG_OUT(" use(%d), model_no(%d), start.", pEnv->nUse, pEnv->nModel);

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Printer Device not use !!!!\n");
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_RECEIPT_ATEC_HS :	///> 에이텍 - 영수증프린터
		{
			if( pPrtHS != NULL )
			{
				pPrtHS->Print_Refund();
			}
		}
		break;
	case DEV_RECEIPT_NICE_RXD :	///> 한전금 - 영수증프린터
		{
			if( pPrtRXD != NULL )
			{
				pPrtRXD->Print_Refund();
			}
		}
		break;
	}

	return 0;
}

/**
 * @brief		Printer_AccountInfo
 * @details		현금 시재 명세서 프린트 
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Printer_AccountInfo(void)
{
	pEnv = (PDEV_CFG_T) GetEnvPrtReceiptInfo();

	TR_LOG_OUT(" use(%d), model_no(%d), start.", pEnv->nUse, pEnv->nModel);

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Printer Device not use !!!!\n");
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_RECEIPT_ATEC_HS :	///> 에이텍 - 영수증프린터
		{
			if( pPrtHS != NULL )
			{
				//pPrtHS->Print_Account();				
			}
		}
		break;
	case DEV_RECEIPT_NICE_RXD :	///> 한전금 - 영수증프린터
		{
			if( pPrtRXD != NULL )
			{
				pPrtRXD->Print_Account();				
			}
		}
		break;
	}

	return 0;
}

/**
 * @brief		Printer_Account2Info
 * @details		시재 정보 프린트 
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Printer_Account2Info(void)
{
	pEnv = (PDEV_CFG_T) GetEnvPrtReceiptInfo();

	TR_LOG_OUT(" use(%d), model_no(%d), start.", pEnv->nUse, pEnv->nModel);

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Printer Device not use !!!!\n");
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_RECEIPT_ATEC_HS :	///> 에이텍 - 영수증프린터
		{
			if( pPrtHS != NULL )
			{
				pPrtHS->Print_Account2();				
			}
		}
		break;
	case DEV_RECEIPT_NICE_RXD :	///> 한전금 - 영수증프린터
		{
			if( pPrtRXD != NULL )
			{
				pPrtRXD->Print_Account2();				
			}
		}
		break;
	}

	return 0;
}

/**
 * @brief		Printer_Account2Info
 * @details		[마감내역] 시재마감 내역 프린트 
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Printer_Account3Info(void)
{
	char		szDate[20];
	char		szTime[20];
	pEnv = (PDEV_CFG_T) GetEnvPrtReceiptInfo();

	TR_LOG_OUT(" use(%d), model_no(%d), start.", pEnv->nUse, pEnv->nModel);

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Printer Device not use !!!!\n");
		return -1;
	}

	::ZeroMemory(szDate, sizeof(szDate));
	::ZeroMemory(szTime, sizeof(szTime));

	::CopyMemory(szDate, &CConfigTkMem::GetInstance()->prt_beg_dt[0], 8);
	::CopyMemory(szTime, &CConfigTkMem::GetInstance()->prt_beg_dt[8], 6);

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_RECEIPT_ATEC_HS :	///> 에이텍 - 영수증프린터
		{
			if( pPrtHS != NULL )
			{
				pPrtHS->Print_Account3(szDate, szTime);				
			}
		}
		break;
	case DEV_RECEIPT_NICE_RXD :	///> 한전금 - 영수증프린터
		{
			if( pPrtRXD != NULL )
			{
				//pPrtRXD->Print_Account3(szDate, szTime);				
			}
		}
		break;
	}

	return 0;
}

/**
 * @brief		Printer_TicketPrint
 * @details		승차권 발권
 * @param		int nSvrKind		서버종류	
 * @param		int nFunction		기능종류	
 * @param		char *pData			프린트 데이타
 * @return		성공 >= 0, 실패 < 0
 */
int Printer_TicketPrint(int nSvrKind, int nFunction, char *pData)
{
	pEnv = (PDEV_CFG_T) GetEnvPrtReceiptInfo();

	TR_LOG_OUT(" use(%d), model_no(%d), start.", pEnv->nUse, pEnv->nModel);

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Printer Device not use !!!!\n");
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_RECEIPT_ATEC_HS :	///> 에이텍 - 영수증프린터
		{
			if( pPrtHS != NULL )
			{
				pPrtHS->Print_Ticket(nSvrKind, nFunction, (char *)pData);				
			}
		}
		break;
	case DEV_RECEIPT_NICE_RXD :	///> 한전금 - 영수증프린터
		{
			if( pPrtRXD != NULL )
			{
				pPrtRXD->Print_Ticket(nSvrKind, nFunction, (char *)pData);				
			}
		}
		break;
	}

	return 0;
}
