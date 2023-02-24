
#include "stdafx.h"
#include <stdio.h>
#include <string.h>
// 20210513 ADD
#include <iostream>
#include <fstream>
#include <sstream>
//#include <time.h>
// 20210513 ~ADD
#include "MyDefine.h"
#include "MyUtil.h"
#include "MyFileUtil.h"
#include "oper_kos_file.h"
#include "File_Env_ini.h"
#include "svr_ccs_main.h"
#include "svr_ko_main.h"
#include "svr_tm_exp_main.h"
#include "event_if.h"
#include "dev_tr_main.h"
#include "data_main.h"
#include "MyDataAccum.h"
#include "svr_ccs_main.h"
#include "oper_config.h"
#include "xzzbus_fdl.h"
#include "dev_prt_main.h"

//----------------------------------------------------------------------------------------------------------------------

static OPER_FILE_CONFIG_T s_config_t;

//----------------------------------------------------------------------------------------------------------------------

static int DefaultConfigFile(void)
{
	OPER_FILE_CONFIG_T*	pCfg;
	PKIOSK_INI_ENV_T pEnv;

	pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();
	
	pCfg = &s_config_t;
	
	::ZeroMemory(pCfg, sizeof(OPER_FILE_CONFIG_T));

	/// 프로토콜 버젼
	s_config_t.base_t.version				= pEnv->nProtoVer & 0xFF;		
	/// 제조사 ('A':에이텍, 'N':한전금)
	s_config_t.base_t.manufacturer			= pEnv->szDeviceDVS[0] & 0xFF;		
	/// (HW) IC 카드리더기 사용유무
	s_config_t.base_t.hw_ic_use_yn			= pEnv->szHwIcUseYN[0] & 0xFF;
	/// (HW) 현금 사용유무
	s_config_t.base_t.hw_cash_use_yn		= pEnv->szHwCashUseYN[0] & 0xFF;
	/// (HW) RF 리더기 사용유무
	s_config_t.base_t.rf_hw_use_yn			= pEnv->szHwRfUseYN[0] & 0xFF;
	/// 운영사 정보
	s_config_t.base_t.oper_corp				= pEnv->nOperCorp & 0xFF;


	s_config_t.base_t.mrnp_func_yn			= 'Y';	///< (10). 예매발권유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.mrnp_all_view_yn		= 'Y';	///< (11). 예매내역전체보기 : 사용(Y), 미사용(N)
	s_config_t.base_t.mrnp_manual_yn		= 'Y';	///< (12). 예매수기조회 : 사용(Y), 미사용(N)

	s_config_t.base_t.mrnp_no_inq_yn		= 'Y';	///< (13). 예매번호 조회 : 사용(Y), 미사용(N)		// 20220922 MOD, change from 'N' to 'Y'
	s_config_t.base_t.mrnp_birth_inq_yn		= 'Y';	///< (14). 생년월일 조회 : 사용(Y), 미사용(N)
	s_config_t.base_t.mrnp_corpno_inq_yn	= 'N';	///< (15). 법인사업자 조회 : 사용(Y), 미사용(N)

	s_config_t.base_t.mrnp_auto_iss_yn		= 'Y';	///< (16). 예매1건 자동발매 : 사용(Y), 미사용(N)

	s_config_t.base_t.pubtck_yn				= 'Y';	///< (17). 현장발권유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.alcn_kind				= 'D';	///< (18). 비배차 : 'N', 배차 : 'D'
	s_config_t.base_t.quick_alcn_yn			= 'N';	///< (19). 빠른배차사용유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.favorate_yn			= 'Y';	///< (20). 즐겨찾기 사용유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.n_max_favor_trml_cnt	= 20;	///< (21). 즐겨찾기터미널 최대갯수

	s_config_t.base_t.pbtck_today_mode_yn	= 'N';	///< (08). 당일발권 전용모드 사용유무 : 사용(Y), 미사용(N)


	s_config_t.base_t.ccs_refund_func_yn	= 'N';	///< (23). 시외 - 환불유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.exp_refund_func_yn	= 'N';	///< (24). 고속 - 환불유무 : 사용(Y), 미사용(N)
	
	///< 결제방식 (IC카드 사용유무)
	s_config_t.base_t.sw_ic_pay_yn			= pEnv->szSwIcUseYN[0] & 0xFF;
	///< 결제방식 (IC카드 사용유무)
	s_config_t.base_t.sw_cash_pay_yn		= pEnv->szSwCashUseYN[0] & 0xFF;
	///< 결제방식 (IC카드 사용유무)
	s_config_t.base_t.sw_rf_pay_yn			= pEnv->szSwRfUseYN[0] & 0xFF;

	TR_LOG_OUT("sw_ic_pay_yn = 0x%02X", s_config_t.base_t.sw_ic_pay_yn);
	TR_LOG_OUT("sw_cash_pay_yn = 0x%02X", s_config_t.base_t.sw_cash_pay_yn);
	TR_LOG_OUT("sw_rf_pay_yn = 0x%02X", s_config_t.base_t.sw_rf_pay_yn);

	s_config_t.base_t.reg_csrc_yn			= 'Y';	///< (29). 현금영수증 등록 여부, 사용:'Y', 미사용:'N'
	s_config_t.base_t.card_installment_yn	= 'Y';	///< (30). (5만원이상) 카드할부 사용유무 : 사용(Y), 미사용(N)	// 20220922 MOD, change from 'Y' to 'N' // 20221205 다시 원복 'Y'
	s_config_t.base_t.sign_pad_yn			= 'Y';	///< (31). 5만원미만 카드 무서명, 무서명:'Y', 서명:'N'
	s_config_t.base_t.card_passwd_yn		= 'N';	///< (32). 카드비밀번호, 사용:'Y', 미사용:'N'
	s_config_t.base_t.maint_monitor_yn		= 'N';
	s_config_t.base_t.auto_close_yn			= 'Y';	///< (34). 자동마감 유무 : 사용(Y), 미사용(N)
	sprintf(s_config_t.base_t.auto_close_time, "235950");	///< (35). 자동마감 시간(hhmmss)
	s_config_t.base_t.sms_yn				= 'Y';
	sprintf((char *)s_config_t.base_t.sms_ip, "10.215.80.70");	/// 광주터미널
	s_config_t.base_t.camera_1_yn			= 'Y';	///< (38). 카메라#1 사용유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.camera_2_yn			= 'Y';	///< (39). 카메라#2 사용유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.camera_3_yn			= 'N';	///< (40). 카메라#3 사용유무 : 사용(Y), 미사용(N)		// 한전금의 경우 카메라가 2대라서 'Y'일 경우 오류 발생
	
	s_config_t.base_t.n_ticket_min_count	= 10;
	s_config_t.base_t.n_10k_min_count		= 10;
	s_config_t.base_t.n_1k_min_count		= 10;
	s_config_t.base_t.n_100_min_count		= 10;
	s_config_t.base_t.n_500_min_count		= 10;
	s_config_t.base_t.n_bill_box_full_cnt	= 500;
	s_config_t.base_t.n_ticket_box_full_cnt	= 500;
	s_config_t.base_t.n_issue_count			= 10;
	s_config_t.base_t.n_issue_money			= 100000;
	s_config_t.base_t.n_issue_time			= 3;
	s_config_t.base_t.n_mrnp_limit_day		= 30;
	s_config_t.base_t.n_screen_wait_time	= 30;
	s_config_t.base_t.n_alarm_wait_time		= 5;
	sprintf(s_config_t.base_t.tck_paper_name, "DF999");
	s_config_t.base_t.exp_ticket_device[0]	= '2';	

	s_config_t.base_t.cc_rtrp_trml_yn		= 'N';	///< (56). [시외] 왕복발권 사용유무(Y/N)
	s_config_t.base_t.exp_rtrp_trml_yn		= 'Y';	///< (57). [고속] 왕복발권 사용유무(Y/N)
	s_config_t.base_t.return_mrnp_yn		= 'Y';	///< (58). 왕복예매발권 사용유무(Y/N)
	s_config_t.base_t.exp_line_yn			= 'Y';	///< (59). 고속노선 판매가능(시외고속) 여부(Y/N)
	s_config_t.base_t.opend_disp_yn			= 'N';	///< (60). 운행종료 표시 여부(Y/N)
	s_config_t.base_t.no_alcn_no_sats_mrnp_yn= 'N'; ///< (61). 비배차 비좌석노선 예매 유무(Y/N)
	s_config_t.base_t.no_alcn_1_issue_yn	= 'N';	///< (62). 비배차 1건시 자동선택 유무(Y/N)
	s_config_t.base_t.no_sats_free_issue_yn	= 'N';	///< (63). 비좌석 무표발권 여부
	s_config_t.base_t.n_mrnp_limit_time	= 0;		///< (64). 예매발권 제한시간 (분)		/// 0: 모두 발권
	s_config_t.base_t.today_mrnp_issue_yn	= 'N';	///< (65). 당일 예매내역 모두 발권유무 : 사용(Y), 미사용(N)	/// 사용안함
	s_config_t.base_t.re_issue_yn			= 'N';	///< (66). 승차권 재발행 기능 사용유무 : 사용(Y), 미사용(N)
	
	s_config_t.base_t.kiosk_op_close_yn		= 'N';	///< (67). 판매 종료시간 사용유무
	sprintf((char *)s_config_t.base_t.kiosk_op_start_tm, "000000");
	sprintf((char *)s_config_t.base_t.kiosk_op_close_tm, "235959");
	s_config_t.base_t.lcd_off_yn			= 'N';	///< (70). 판매 종료시간 LCD OFF 사용유무
	
	s_config_t.base_t.ic_card_insert_yn		= 'Y';	///< (71). IC CARD 투입 알림 사용유무
	s_config_t.base_t.multi_lang_yn			= 'Y';	///< (72). 다국어 사용유무
	s_config_t.base_t.sound_yn				= 'Y';	///< (73). 안내음성 사용유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.main_sound_time		= 600;	///< (74). 메인화면 안내음성 반복 시간 (초)
	s_config_t.base_t.bcnl_yn				= 'N';	///< (75). 결행배차 표시유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.tck_no_rmn_yn			= 'Y';	///< (76). 매진배차 표시유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.alcn_state_color_yn	= 'Y';	///< (77). 배차상태 배경색 표시유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.disc_btn_yn			= 'N';	///< (78). 선택할인 버튼 변경 유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.rot_search_yn			= 'N';	///< (79). 노선검색 사용유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.air_sta_popup_yn		= 'N';	///< (80). 인천공항 알림 팝업 사용 유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.tck_err_change_yn		= 'Y';	///< (81). 티켓출력 에러시 교체발권 사용유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.refund_void_prt_yn	= 'Y';	///< (82). 승차권 환불 시, 소인 사용유무 : 사용(Y), 미사용(N)

	/// 2020.01.15 add data
	s_config_t.base_t.rot_mode_yn[0]		= 'N';	///< (92). 노선선택 전용모드 유무
	s_config_t.base_t.rot_mode_mrnp_yn[0]	= 'N';	///< (93). 노선선택 전용모드에서 예매발권 유무
	s_config_t.base_t.rot_mode_arvl_yn[0]	= 'N';	///< (94). 노선선택 전용모드에서 목적지 검색유무
	s_config_t.base_t.rf_mode_precard_yn[0]	= 'N';	///< (95). RF모드에서 선불카드 사용유무
	s_config_t.base_t.rf_mode_postcard_yn[0]= 'N';	///< (96). RF모드에서 후불카드 사용유무
	s_config_t.base_t.refund_reprint_yn[0]	= 'N';	///< (97). 환불영수증 재발행 사용유무
	s_config_t.base_t.mrnp_inq_yn[0]		= 'N';	///< (98). 예매자료 조회 사용유무
	s_config_t.base_t.keywd_search_yn[0]	= 'N';	///< (99). 키워드 검색 사용유무
	s_config_t.base_t.prev_mrnp_inq_yn[0]	= 'N';	///< (100).과거 예매내역 발권 사용유무
	s_config_t.base_t.alcn_time_vw_yn[0]	= 'N';	///< (101).배차시간대 선택버튼 사용유무
	/// ~2020.01.15 add data

	s_config_t.base_t.set_refund_receipt_yn[0]='Y';	///< (110).환불영후증 기본 선택여부

	/* // 20210712 ADD
	s_config_t.base_t.card_installment_yn		= 'N';	///< (30). (5만원이상) 카드할부 사용유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.bcnl_yn					= 'N';	///< (75). 결행배차 표시유무 : 사용(Y), 미사용(N)
	s_config_t.base_t.day_cls_an_cash_cls_yn[0]	= 'Y';	///< (117). 창구마감시 시재마감 여부
	*/ // 20210712 ~ADD

	/// 시외버스
	{
		/// 가맹점 이름
		sprintf(s_config_t.ccTrmlInfo_t.sz_prn_trml_nm, "에이텍티앤");
		/// 가맹점 사업자번호
		sprintf(s_config_t.ccTrmlInfo_t.sz_prn_trml_corp_no, "698-88-00163");
	}

	/// 코버스
	{
		/// 가맹점 이름
		sprintf(s_config_t.koTrmlInfo_t.sz_prn_trml_nm, "전국고속버스운송사업조합");
		/// 가맹점 사업자번호
		sprintf(s_config_t.koTrmlInfo_t.sz_prn_trml_corp_no, "114-82-01186");
	}

	{
		sprintf(s_config_t.ccTrmlInfo_t.szPgmDVS, "KOS");
		sprintf(s_config_t.ezTrmlInfo_t.szPgmDVS, "KOS");
	}

	/// 업무 시작시간
	{
		SYSTEMTIME st;

		::GetLocalTime(&st);

		sprintf(s_config_t.job_start_dt, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	}
	//s_config_t.base_t.n_prt_bold_val		= 0;


	s_config_t.base_t.main_menu_type	= '1';	///< (70). 홈화면 메뉴 타입 ('1':세로, '2':가로)
	s_config_t.base_t.pubtck_main_use_yn= 'Y';	///< (71). 현장발권 메인 메뉴 사용여부('Y':사용, 'N':미사용)

	s_config_t.base_t.map_yn[0]			= 'N';	///< (90). 지도사용유무

	s_config_t.base_t.ic_popup_yn[0]	= 'Y';	///< (90). IC카드 제거 팝업 사용유무
	s_config_t.base_t.n_ic_timeout		= 5;	///< (90). IC카드 타임아웃 시간(초)
	s_config_t.base_t.first_keybd_yn[0]	= 'Y';	///< (90). 초성 키보드 우선 사용유무

	return 0;
}

static void PrintConfigFile(void)
{
	//TR_LOG_OUT("", );
}


/**
 * @brief		GetOperConfigData
 * @details		get Config Data
 * @param		None
 * @return		항상 : 0
 */
char* GetOperConfigData(void)
{
	return (char *)&s_config_t;
}

/**
 * @brief		SetOperConfigData
 * @details		set Config Data
 * @param		None
 * @return		항상 : 0
 */
void SetOperConfigData(void)
{
	WriteConfigFile((char *)&s_config_t);

	WriteEnvIniFile((char *)&s_config_t);
}

/**
 * @brief		GetConfigPayment
 * @details		결제방식 데이타 가져오기
 * @param		None
 * @return		결제방식 데이타
 */
int GetConfigPayment(void)
{
	/// IC 카드리더기 사용유무
	if( s_config_t.base_t.sw_ic_pay_yn == 'Y' ) 
	{
		/// RF 카드리더기 사용유무
		if( s_config_t.base_t.sw_rf_pay_yn == 'Y' )
		{
			return PAY_CARD_RF;
		}

		/// 현금 사용유무
		if( s_config_t.base_t.sw_cash_pay_yn == 'Y' )
		{
			return PAY_CARD_CASH;
		}
		
		return PAY_ONLY_CARD;
	}
	else
	{
		/// 현금 사용유무
		if( s_config_t.base_t.sw_cash_pay_yn == 'Y' ) 
		{
			return PAY_ONLY_CASH;
		}

		/// RF 사용유무
		if( s_config_t.base_t.sw_cash_pay_yn == 'Y' ) 
		{
			return PAY_ONLY_RF;
		}
	}

	return PAY_NONE;
}

/**
 * @brief		GetConfigServerKind
 * @details		서버종류 데이타 가져오기
 * @param		None
 * @return		서버종류 값
 */
int GetConfigServerKind(void)
{
	int nRet = 0;

	switch(s_config_t.base_t.ccs_svr_kind)
	{
	case '0' :
		break;
	case '1' :
		nRet |= SVR_DVS_CCBUS;
		break;
	}

	switch(s_config_t.base_t.exp_svr_kind)
	{
	case '0' :
		break;
	case '1' :	// 티머니고속
		nRet |= SVR_DVS_TMEXP;
		break;
	case '2' :	// 코버스
		nRet |= SVR_DVS_KOBUS;
		break;
	}

	return nRet;
}

/// 2021.03.25 add code
/**
 * @brief		GetConfigCloseOptValue
 * @details		get 창구마감시 시재마감 옵션 정보
 * @param		None
 * @return		옵션값
 */
char GetConfigCloseOptValue(void)
{
	return s_config_t.base_t.day_cls_an_cash_cls_yn[0];
}
/// ~2021.03.25 add code

/**
 * @brief		GetConfigOperCorp
 * @details		get 운영정보
 * @param		None
 * @return		서버종류 값
 */
int GetConfigOperCorp(void)
{
	return s_config_t.base_t.oper_corp;
}

/**
 * @brief		GetPgmDVS
 * @details		get 프로그램 구분
 * @param		None
 * @return		항상 : 0
 */
char* GetPgmDVS(void)
{
	return (char *)s_config_t.ccTrmlInfo_t.szPgmDVS;
}

/**
 * @brief		GetReqPgmDVS
 * @details		get 프로그램 구분
 * @param		None
 * @return		항상 : 0
 */
char* GetReqPgmDVS(int nSvrKind)
{
	if(nSvrKind & SVR_DVS_CCBUS)
	{
		return (char *)s_config_t.ccTrmlInfo_t.szPgmDVS;
	}

	if(nSvrKind & SVR_DVS_KOBUS)
	{
		return (char *)s_config_t.koTrmlInfo_t.szPgmDVS;
	}

	if(nSvrKind & SVR_DVS_TMEXP)
	{
		return (char *)"KOS";
	}

	return (char *) NULL;
}

/**
 * @brief		GetTrmlWndNo
 * @details		get 창구번호 
 * @param		None
 * @return		항상 : 0
 */
char* GetTrmlWndNo(int nSvrKind)
{
	if(nSvrKind & SVR_DVS_CCBUS)
	{
		return (char *)s_config_t.ccTrmlInfo_t.szWndNo;
	}

	if(nSvrKind & SVR_DVS_KOBUS)
	{
		return (char *)s_config_t.koTrmlInfo_t.szWndNo;
	}

	if(nSvrKind & SVR_DVS_TMEXP)
	{
		return (char *)s_config_t.ezTrmlInfo_t.szWndNo;
	}

	return (char *) NULL;
}

/**
 * @brief		GetConfigTicketPapaer
 * @details		get 승차권 인쇄용지 정보
 * @param		None
 * @return		0x31:승차권, 0x32:감열지 // PAPER_ROLL(0x31):감열지, PAPER_TICKET(0x32):승차권 // 20230203 ADD // 20230216 MOD
 */
int GetConfigTicketPapaer(void)
{
	///< (55). 고속버스 승차권 ('1':승차권발권기 프린터, '2':감열지프린터)	// PAPER_ROLL(0x31):감열지, PAPER_TICKET(0x32):승차권 // 20230203 ADD // 20230216 MOD
	return s_config_t.base_t.exp_ticket_device[0]; // BYTE	exp_ticket_device[10]	;	
}

// 20230206 ADD~
/**
 * @brief		SetConfigTicketPapaer
 * @details		Set 승차권 인쇄용지 정보
 * @param		pType : 승차권 인쇄용지 유형 (0x31:승차권, 0x32:감열지) // 기본값:'2'
 * @return		None
 */
void SetConfigTicketPapaer(char *pType)
{
	::ZeroMemory(s_config_t.base_t.exp_ticket_device, sizeof(s_config_t.base_t.exp_ticket_device));
	::CopyMemory(s_config_t.base_t.exp_ticket_device, pType, strlen(pType));

	WriteConfigFile((char *)&s_config_t);
}
// 20230206 ~ADD

int GetConfigPrevMrnpListYN(void)
{
	if( s_config_t.base_t.prev_mrnp_inq_yn[0] == 'Y' )
		return 1;

	return 0;
}

/**
 * @brief		GetTrmlCode
 * @details		get 터미널 코드 (7바이트)
 * @param		None
 * @return		항상 : 0
 */
char* GetTrmlCode(int nSvrKind)
{
	if(nSvrKind & SVR_DVS_CCBUS)
	{
		return (char *)s_config_t.ccTrmlInfo_t.szCode7;
	}
	
	if(nSvrKind & SVR_DVS_KOBUS)
	{
		return (char *)s_config_t.koTrmlInfo_t.szCode7;
	}

	if(nSvrKind & SVR_DVS_TMEXP)
	{
		return (char *)s_config_t.ezTrmlInfo_t.szCode7;
	}

	return (char *)NULL;
}

/**
 * @brief		SetTrmlCode
 * @details		set 터미널 코드 (7바이트)
 * @param		char *pData		터미널 코드 데이타
 * @return		None
 */
void SetTrmlCode(int nSvrKind, char *pData)
{
	if(nSvrKind & SVR_DVS_CCBUS)
	{
		if(memcmp(s_config_t.ccTrmlInfo_t.szCode7, pData, 7) != 0)
		{
			::ZeroMemory(s_config_t.ccTrmlInfo_t.szCode7, sizeof(s_config_t.ccTrmlInfo_t.szCode7));
			::CopyMemory(s_config_t.ccTrmlInfo_t.szCode7, pData, 7);

			WriteConfigFile((char *)&s_config_t);
		}
	}

	if(nSvrKind & SVR_DVS_KOBUS)
	{
		if(memcmp(s_config_t.koTrmlInfo_t.szCode7, pData, strlen(pData)) != 0)
		{
			::ZeroMemory(s_config_t.koTrmlInfo_t.szCode7, sizeof(s_config_t.koTrmlInfo_t.szCode7));
			::CopyMemory(s_config_t.koTrmlInfo_t.szCode7, pData, strlen(pData));

			WriteConfigFile((char *)&s_config_t);
		}
	}

	if(nSvrKind & SVR_DVS_TMEXP)
	{
		if(memcmp(s_config_t.ezTrmlInfo_t.szCode7, pData, strlen(pData)) != 0)
		{
			::ZeroMemory(s_config_t.ezTrmlInfo_t.szCode7, sizeof(s_config_t.ezTrmlInfo_t.szCode7));
			::CopyMemory(s_config_t.ezTrmlInfo_t.szCode7, pData, strlen(pData));

			WriteConfigFile((char *)&s_config_t);
		}
	}
}

/**
 * @brief		GetShortTrmlCode
 * @details		get 단축 터미널 코드 (4바이트)
 * @param		None
 * @return		항상 : 0
 */
char* GetShortTrmlCode(int nSvrKind)
{
	if(nSvrKind & SVR_DVS_CCBUS)
	{
		return (char *)s_config_t.ccTrmlInfo_t.szCode4;
	}
	if(nSvrKind & SVR_DVS_KOBUS)
	{
		return (char *)s_config_t.koTrmlInfo_t.szCode4;
	}
	if(nSvrKind & SVR_DVS_TMEXP)
	{
		return (char *)s_config_t.ezTrmlInfo_t.szCode4;
	}

	return (char *) NULL;
}

/**
 * @brief		SetShortTrmlCode
 * @details		set 단축 터미널 코드 (4바이트)
 * @param		char *pData		단축 터미널 코드 데이타
 * @return		None
 */
void SetShortTrmlCode(int nSvrKind, char *pData)
{
	if(nSvrKind & SVR_DVS_CCBUS)
	{
		::ZeroMemory(s_config_t.ccTrmlInfo_t.szCode4, sizeof(s_config_t.ccTrmlInfo_t.szCode4));
		::CopyMemory(s_config_t.ccTrmlInfo_t.szCode4, pData, 4);
	}
	if(nSvrKind & SVR_DVS_KOBUS)
	{
		::ZeroMemory(s_config_t.koTrmlInfo_t.szCode4, sizeof(s_config_t.koTrmlInfo_t.szCode4));
		::CopyMemory(s_config_t.koTrmlInfo_t.szCode4, pData, 4);
	}
	if(nSvrKind & SVR_DVS_TMEXP)
	{
		::ZeroMemory(s_config_t.ezTrmlInfo_t.szCode4, sizeof(s_config_t.ezTrmlInfo_t.szCode4));
		::CopyMemory(s_config_t.ezTrmlInfo_t.szCode4, pData, 4);
	}

	WriteConfigFile((char *)&s_config_t);
}

/**
 * @brief		GetTrmlName
 * @details		get 터미널 이름
 * @param		None
 * @return		항상 : 0
 */
char* GetTrmlName(void)
{
	return (char *)s_config_t.ccTrmlInfo_t.szName;
}

/**...........................................................
 * @brief		GetMyIpAddress
 * @details		get ip address
 * @param		None
 * @return		항상 : 0
 */
char* GetMyIpAddress(void)
{
	return (char *)s_config_t.myIPAddress;
}

/**
 * @brief		SetMyIpAddress
 * @details		set ip address
 * @param		char *pIpAddress		ip address
 * @return		항상 : 0
 */
void SetMyIpAddress(char *pIP)
{
	::ZeroMemory(s_config_t.myIPAddress, sizeof(s_config_t.myIPAddress));
	::CopyMemory(s_config_t.myIPAddress, pIP, strlen(pIP));

	WriteConfigFile((char *)&s_config_t);
}

/**
 * @brief		GetMyMacAddress
 * @details		get mac address
 * @param		None
 * @return		항상 : 0
 */
char* GetMyMacAddress(void)
{
	return (char *)s_config_t.myMacAddress;
}

/**
 * @brief		SetMyIpAddress
 * @details		set ip address
 * @param		char *pIpAddress		ip address
 * @return		항상 : 0
 */
void SetMyMacAddress(char *pMAC)
{
	::ZeroMemory(s_config_t.myMacAddress, sizeof(s_config_t.myMacAddress));
	::CopyMemory(s_config_t.myMacAddress, pMAC, strlen(pMAC));

	WriteConfigFile((char *)&s_config_t);
}

/**
 * @brief		IsAlcn
 * @details		배차모드 여부
 * @param		None
 * @return		항상 : 0
 */
BOOL IsAlcn(void)
{
	if(	s_config_t.base_t.alcn_kind == 'N' )
	{	/// 비배차
		return FALSE;
	}
	return TRUE;
}

/**
 * @brief		GetTckPrinterType
 * @details		get 프린터 타입
 * @param		None
 * @return		항상 : 0
 */
char* GetTckPrinterType(void)
{
	return (char *) "825";
	//return (char *)s_config_t.base_t.prt_device_type;
}

/**
 * @brief		GetLoginUserNo
 * @details		TMAX Login User No
 * @param		None
 * @return		UserNo
 */
char *GetLoginUserNo(int nSvrKind)
{
	switch(nSvrKind)
	{
	case SVR_DVS_CCBUS :
		return s_config_t.ccTrmlInfo_t.szUserNo;

	case SVR_DVS_KOBUS :
		return s_config_t.koTrmlInfo_t.szUserNo;

	case SVR_DVS_TMEXP :
		return s_config_t.ezTrmlInfo_t.szUserNo;
	}
	return (char *) NULL;
}

/**
 * @brief		SetLoginUserNo
 * @details		set Login UserNo
 * @param		char *pData			set user_no
 * @return		None
 */
void SetLoginUserNo(char *pData)
{
	if( memcmp(s_config_t.ccTrmlInfo_t.szUserNo, pData, sizeof(s_config_t.ccTrmlInfo_t.szUserNo)) != 0 )
	{
		::ZeroMemory(s_config_t.ccTrmlInfo_t.szUserNo, sizeof(s_config_t.ccTrmlInfo_t.szUserNo));
		::CopyMemory(s_config_t.ccTrmlInfo_t.szUserNo, pData, strlen(pData));

		WriteConfigFile((char *)&s_config_t);
	}
}

/**
 * @brief		GetLoginUserPwd
 * @details		TMAX Login User password
 * @param		int nSvrKind	서버종류
 * @return		서버 비밀번호
 */
char *GetLoginUserPwd(int nSvrKind)
{
	switch(nSvrKind)
	{
	case SVR_DVS_CCBUS :
		return s_config_t.ccTrmlInfo_t.szUserPwd;
	case SVR_DVS_KOBUS :
		return s_config_t.koTrmlInfo_t.szUserPwd;
	case SVR_DVS_TMEXP :
		return s_config_t.ezTrmlInfo_t.szUserPwd;
	}

	return (char *)NULL;
}

/**
 * @brief		SetLoginUserPwd
 * @details		set Login Password
 * @param		char *pData			set password
 * @return		None
 */
void SetLoginUserPwd(char *pData)
{
	if( memcmp(s_config_t.ccTrmlInfo_t.szUserPwd, pData, sizeof(s_config_t.ccTrmlInfo_t.szUserPwd)) != 0 )
	{
		::ZeroMemory(s_config_t.ccTrmlInfo_t.szUserPwd, sizeof(s_config_t.ccTrmlInfo_t.szUserPwd));
		::CopyMemory(s_config_t.ccTrmlInfo_t.szUserPwd, pData, strlen(pData));

		WriteConfigFile((char *)&s_config_t);
	}
}

/**
 * @brief		Config_IsAlcnMode
 * @details		배차 / 비배차 모드 
 * @param		None
 * @return		배차/비배차 모드 값
 */
int Config_IsAlcnMode(void)
{
	//return s_config_t.baseInfo_t.byAlcn;
	if( s_config_t.base_t.alcn_kind == 'N')
	{	/// 비배차
		return 0;
	}
	return 1;
}

/**
 * @brief		GetConfigBaseInfo
 * @details		get base info
 * @param		None
 * @return		base info
 */
char* GetConfigBaseInfo(void)
{
	return (char *)&s_config_t.base_t;
}

/**
 * @brief		GetConfigTrmlInfo
 * @details		get terminal info
 * @param		None
 * @return		base info
 */
char* GetConfigTrmlInfo(int nSvrKind)
{
	switch(nSvrKind)
	{
	case SVR_DVS_CCBUS:
		return (char *)&s_config_t.ccTrmlInfo_t;
	case SVR_DVS_KOBUS:
		return (char *)&s_config_t.koTrmlInfo_t;
	case SVR_DVS_TMEXP:
		return (char *)&s_config_t.ezTrmlInfo_t;
	}

	return (char *) NULL;
}

/**
 * @brief		SetConfigTrmlInfo
 * @details		터미널 정보 설정
 * @param		int nSvrKind		버스 구분		
 * @param		char *pData			터미널 정보
 * @return		None
 */
void SetConfigTrmlInfo(int nSvrKind, char *pData)
{
	PTERMINAL_INFO_T pCfgTrml;
	PUI_BASE_TRML_INFO_T pUiTrml;

	pUiTrml = (PUI_BASE_TRML_INFO_T) pData;

	switch(nSvrKind)
	{
	case SVR_DVS_CCBUS:
		TR_LOG_OUT("%s", "시외버스 - Set 터미널 정보");
		pCfgTrml = &s_config_t.ccTrmlInfo_t;
		break;
	case SVR_DVS_KOBUS:
		TR_LOG_OUT("%s", "코버스 - Set 터미널 정보");
		pCfgTrml = &s_config_t.koTrmlInfo_t;
		break;
	case SVR_DVS_TMEXP:
		TR_LOG_OUT("%s", "티머니고속 - Set 터미널 정보");
		pCfgTrml = &s_config_t.ezTrmlInfo_t;
		break;
	}

	::CopyMemory(pCfgTrml->szName				,	pUiTrml->trml_nm			, sizeof(pUiTrml->trml_nm			));
	::CopyMemory(pCfgTrml->szCorpTel			,	pUiTrml->trml_rprn_tel_no	, sizeof(pUiTrml->trml_rprn_tel_no	));

	::CopyMemory(pCfgTrml->sz_prn_trml_nm		,	pUiTrml->prn_trml_nm		, sizeof(pUiTrml->prn_trml_nm		));
	::CopyMemory(pCfgTrml->sz_prn_trml_corp_no	,	pUiTrml->prn_trml_corp_no	, sizeof(pUiTrml->prn_trml_corp_no	));
	::CopyMemory(pCfgTrml->sz_prn_trml_sangho	,	pUiTrml->prn_trml_sangho	, sizeof(pUiTrml->prn_trml_sangho	));
	::CopyMemory(pCfgTrml->sz_prn_trml_corp_no1	,	pUiTrml->prn_trml_corp_no1	, sizeof(pUiTrml->prn_trml_corp_no1	));
	::CopyMemory(pCfgTrml->sz_prn_trml_tel_no	,	pUiTrml->prn_trml_tel_no	, sizeof(pUiTrml->prn_trml_tel_no	));
	::CopyMemory(pCfgTrml->sz_prn_trml_ars_no	,	pUiTrml->prn_trml_ars_no	, sizeof(pUiTrml->prn_trml_ars_no	));

// 	TR_LOG_OUT("터미널 창구번호   = %s", pCfgTrml->szWndNo				);
// 	TR_LOG_OUT("터미널번호(7자리) = %s", pCfgTrml->szCode7				);
// 	TR_LOG_OUT("터미널번호(4자리) = %s", pCfgTrml->szCode4				);
// 	TR_LOG_OUT("사용자 NO        = %s", pCfgTrml->szUserNo				);
// 	TR_LOG_OUT("사용자 비번      = %s", pCfgTrml->szUserPwd			);
// 	TR_LOG_OUT("터미널 명칭      = %s", pCfgTrml->sz_prn_trml_nm		);
// 	TR_LOG_OUT("터미널 전화번호  = %s", pCfgTrml->sz_prn_trml_tel_no	);
// 	TR_LOG_OUT("가맹점 이름      = %s", pCfgTrml->sz_prn_trml_nm		);
// 	TR_LOG_OUT("가맹점 사업자번호 = %s", pCfgTrml->sz_prn_trml_corp_no	);
// 	TR_LOG_OUT("터미널 상호      = %s", pCfgTrml->sz_prn_trml_sangho	);
// 	TR_LOG_OUT("터미널 사업자번호 = %s", pCfgTrml->sz_prn_trml_corp_no1	);
// 	TR_LOG_OUT("터미널 전화번호  = %s", pCfgTrml->sz_prn_trml_tel_no	);
// 	TR_LOG_OUT("터미널 ARS 번호  = %s", pCfgTrml->sz_prn_trml_ars_no	);
}

/**
 * @brief		SetConfigTrmlInfo
 * @details		터미널 정보 설정
 * @param		None
 * @return		None
 */
BOOL Config_IsCCServer(void)
{
	if( s_config_t.base_t.ccs_svr_kind == '0' )
	{
		return FALSE;
	}
	return TRUE;
}

BOOL Config_IsExpServer(void)
{
	if( s_config_t.base_t.exp_svr_kind == '0' )
	{
		return FALSE;
	}
	return TRUE;
}

/**
 * @brief		ReadConfigFile
 * @details		Read Config File 
 * @param		char *retBuf		Read Config Data Buffer
 * @return		성공 : >= 0, 실패 : < 0
 */
int ReadConfigFile(char *retBuf)
{
	int nRet;

	nRet = OperReadFile(OPER_FILE_ID_CONFIG, (char *)&s_config_t, sizeof(OPER_FILE_CONFIG_T));
	if(nRet < 0)
	{
		return nRet;
	}

	if(retBuf != NULL)
	{
		::CopyMemory(retBuf, &s_config_t, sizeof(OPER_FILE_CONFIG_T));
	}

	return nRet;
}

/**
 * @brief		WriteConfigFile
 * @details		Write Config File 
 * @param		char *pWrData		Write Config Data Buffer
 * @return		성공 : >= 0, 실패 : < 0
 */
int WriteConfigFile(char *pWrData)
{
	int nRet;

	nRet = OperWriteFile(OPER_FILE_ID_CONFIG, (char *)&s_config_t, sizeof(OPER_FILE_CONFIG_T));
	if(nRet < 0)
	{
		return nRet;
	}

	ReadConfigFile((char *)&s_config_t);

	return nRet;
}

/**
 * @brief		GetTicketLimitCount
 * @details		승차권 최소 수량 정보
 * @param		None
 * @return		None
 */
int GetTicketLimitCount(void)
{
	return s_config_t.base_t.n_ticket_min_count;
}

/**
 * @brief		GetBillLimitCount
 * @details		천원권 or 만원권 최소 수량
 * @param		BOOL b1k		: TRUE(천원권), FALSE(만원권)
 * @return		None
 */
int GetBillLimitCount(BOOL b1k)
{
	if(b1k == TRUE)
	{
		return s_config_t.base_t.n_1k_min_count;
	}
	return s_config_t.base_t.n_10k_min_count;
}

/**
 * @brief		GetCoinLimitCount
 * @details		100원 또는 500원 동전 최소 수량
 * @param		BOOL b100	: TRUE(100원), FALSE(500원)
 * @return		None
 */
int GetCoinLimitCount(BOOL b100)
{
	if(b100 == TRUE)
	{
		return s_config_t.base_t.n_100_min_count;
	}
	return s_config_t.base_t.n_500_min_count;
}

// 20220310 ADD~
#define PROM_STRING "device_status\{code\=\"%d\"\}"

#define PROM_FILE "C:\\Logs\\windows_exporter\\textfile_inputs\\device_status.prom"
#define OUT_FILE "C:\\Logs\\windows_exporter\\textfile_inputs\\device_status.prom.out"

int Prom_Warn_Update(int nCode, int nCount)
{
	char sReadBuff[512], *p, sStatBuff[512];
    char sPromStr[512];

	ifstream ifile;
    ofstream ofile;

    char line[256];
       
    ifile.open(PROM_FILE);  // 파일 열기
	ofile.open(OUT_FILE);

    if (ifile.is_open())
    {
		sprintf(sStatBuff, PROM_STRING, nCode);
        while (ifile.getline(line, sizeof(line)))	// 한 줄씩 리드
        {
			p = strstr(line, sStatBuff);
            if (p != NULL) {
				char sPromStr[512];
				memset(sPromStr, 0x00, sizeof(sPromStr));
				sprintf(sPromStr, "%s %d", sStatBuff, nCount);
				ofile << sPromStr << endl;	// 변경 내용 출력
			}
			else {
				ofile << line << endl;		// 기존 내용 출력
			}
        }
    }

    ifile.close(); // 파일 닫기
    ofile.close(); // 파일 닫기

	BOOL bDel = ::DeleteFile( _T(PROM_FILE) );
	BOOL bMove = ::MoveFile( _T(OUT_FILE), _T(PROM_FILE) );

    return 0;
}
// 20220310 ~ADD

/**
 * @brief		CheckTicketCount
 * @details		승차권 수량 체크
 * @param		None
 * @return		항상 0
 */
int CheckTicketCount(void)
{
	int nLimitCount = 0;
	PFILE_ACCUM_N1010_T pAccum = GetAccumulateData();

	try
	{
		nLimitCount = GetTicketLimitCount();
		if( pAccum->Curr.phyTicket.nCount <= nLimitCount ) 
		{
			SetCheckEventCode(EC_TICKET_SHORT, TRUE);
		}
		else
		{
			SetCheckEventCode(EC_TICKET_SHORT, FALSE);
		}

		// 20220310 ADD~
		if( ( ( pAccum->Prev.phyTicket.nCount != pAccum->Curr.phyTicket.nCount)
			&& ( nLimitCount < pAccum->Curr.phyTicket.nCount ) )
			|| ( ( nLimitCount < pAccum->Curr.phyTicket.nCount ) 
				&& ( pAccum->Curr.phyTicket.nCount < nLimitCount + 90 ) ) )	// 승차권매수 11~99
		{
			Prom_Warn_Update(EC_TICKET_SHORT, pAccum->Curr.phyTicket.nCount);	
		}
		// 20220310 ~ADD
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		CheckCoinCount
 * @details		동전 수량 체크
 * @param		None
 * @return		항상 0
 */
int CheckCoinCount(void)
{
	int nLimitCount = 0;
	PFILE_ACCUM_N1010_T pAccum = GetAccumulateData();

	try
	{
		// 100원
		nLimitCount = GetCoinLimitCount(TRUE);
		if( pAccum->Curr.Coin.n100 <= nLimitCount ) 
		{
			SetCheckEventCode(EC_COIN_100_NO, TRUE);
		}
		else
		{
			SetCheckEventCode(EC_COIN_100_NO, FALSE);
		}

		// 500원
		nLimitCount = GetCoinLimitCount(FALSE);
		if( pAccum->Curr.Coin.n500 <= nLimitCount ) 
		{
			SetCheckEventCode(EC_COIN_500_NO, TRUE);
		}
		else
		{
			SetCheckEventCode(EC_COIN_500_NO, FALSE);
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		CheckBillCount
 * @details		지폐 수량 체크
 * @param		None
 * @return		항상 0
 */
int CheckBillCount(void)
{
	int nLimitCount = 0;
	PFILE_ACCUM_N1010_T pAccum = GetAccumulateData();

	try
	{
		// 1,000원
		nLimitCount = GetBillLimitCount(TRUE);
//		TR_LOG_OUT("GetBillLimitCount(1k), nLimitCount = %d, Acc->Casst.n1k(%d) ..", nLimitCount, pAccum->Curr.BillDispenser.Casst.n1k);
		if( pAccum->Curr.BillDispenser.Casst.n1k <= nLimitCount ) 
		{
			SetCheckEventCode(EC_CDU_1K_SHORT, TRUE);
		}
		else
		{
			SetCheckEventCode(EC_CDU_1K_SHORT, FALSE);
		}

		// 10,000원
		if( INI_Use10K() )
		{
			nLimitCount = GetBillLimitCount(FALSE);
//			TR_LOG_OUT("GetBillLimitCount(10k), nLimitCount = %d, Acc->Casst.n10k(%d) ..", nLimitCount, pAccum->Curr.BillDispenser.Casst.n10k);
			if( pAccum->Curr.BillDispenser.Casst.n10k <= nLimitCount ) 
			{
				SetCheckEventCode(EC_CDU_10K_SHORT, TRUE);
			}
			else
			{
				SetCheckEventCode(EC_CDU_10K_SHORT, FALSE);
			}
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		SetAutoCloseEvent
 * @details		자동마감 이벤트 설정
 * @param		None
 * @return		항상 0
 */
int SetAutoCloseEvent(void)
{
	char		Buffer[100];
	SYSTEMTIME	st;

	try
	{
		::GetLocalTime(&st);
		sprintf(Buffer, "%02d%02d%02d", st.wHour, st.wMinute, st.wSecond);

		if( s_config_t.base_t.auto_close_yn == 'Y' )
		{
			if( GetEventCode(EC_JOB_CLOSE) )
				return 0;

			if( memcmp(s_config_t.base_t.auto_close_time, Buffer, 6) == 0 )
			{	/// 자동마감 처리
				TR_LOG_OUT("자동 창구마감 이벤트 SET!!!");
				SetCheckEventCode(EC_JOB_CLOSE, TRUE);
			}
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		IsConfigTPHPrint
 * @details		환불시 티켓 소인여부
 * @param		None
 * @return		소인 사용 > 0, 소인 사용안함 < 0
 */
int IsConfigTPHPrint(void)
{
	// 소인 사용유무
	if( (s_config_t.base_t.refund_void_prt_yn == 'Y') || (s_config_t.base_t.refund_void_prt_yn == 'y') )
	{	/// 사용
		return 1;
	}

	return -1;
}


/**
 * @brief		Util_CTimeFromString
 * @details		string을 CTime으로 변환
 * @param		char *pDate		YYYYMMDD
 * @param		char *pTime		hhmmss
 * @return		tick
 */
CTime Util_CTimeFromString(char *pDate, char *pTime)
{
	SYSTEMTIME st;
	//CTime t2;

	st.wYear	= (WORD) Util_Ascii2Long(&pDate[0], 4);
	st.wMonth	= (WORD) Util_Ascii2Long(&pDate[4], 2);
	st.wDay		= (WORD) Util_Ascii2Long(&pDate[6], 2);

	st.wHour	= (WORD) Util_Ascii2Long(&pTime[0], 2);
	st.wMinute	= (WORD) Util_Ascii2Long(&pTime[2], 2);
	st.wSecond	= (WORD) Util_Ascii2Long(&pTime[4], 2);

	CTime t2(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	return t2;
}

/**
 * @brief		WndKioskClose
 * @details		창구마감 처리
 * @param		BOOL bReboot		PC 리부팅 여부 값
 * @return		항상 = 0 , 실패 < 0						// 20210513 MOD
 */
int WndKioskClose(BOOL bReboot)
{
	int					nRet, nSvrKind;
	SYSTEMTIME			st;
	PFILE_ACCUM_N1010_T	pAccum;
	PKIOSK_INI_ENV_T	pEnv;

	pAccum = (PFILE_ACCUM_N1010_T) GetAccumulateData();
	//nSvrKind = GetConfigServerKind();					// 20210709 DEL

	nRet = 0;
	nSvrKind = 0;
	nSvrKind = GetConfigServerKind();					// 20210709 ADD
	TR_LOG_OUT("창구마감 ... nSvrKind(%d)", nSvrKind);	// 20210709 ADD

	// 20210513 ADD
	int		nCls_Ccs_flag = 0;			// 시외 창구마감 당일처리여부
	int		nCls_Kobus_flag = 0;		// KOBUS고속 창구마감 당일처리여부
	int		nCls_Tmoney_flag = 0;		// 티머니고속 창구마감 당일처리여부

	pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();
	if(pEnv->tService.szDayClsFlag[0] == 'Y')	// 창구마감 시, 1일 1마감 사용 유무
	{
		::GetLocalTime(&st);
		CTime tNow(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

		TR_LOG_OUT("창구마감 ... GetLocalTime(%02d%02d%02d%02d%02d%02d)", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);	// 20210513 MOD 
		TR_LOG_OUT("창구마감 ... pAccum->sCls_Ccs_dt(%s)", pAccum->sCls_Ccs_dt);			// 20210513 MOD 
		TR_LOG_OUT("창구마감 ... pAccum->sCls_Kobus_dt(%s)", pAccum->sCls_Kobus_dt);	
		TR_LOG_OUT("창구마감 ... pAccum->sCls_Tmoney_dt(%s)", pAccum->sCls_Tmoney_dt);
	
		LONGLONG nMM = 15;	//// 전일 마감 실행됐지만 24:00:00 을 넘겨 당일 00:MM:00 안으로 마감된 것은 당일 마감처리내역 없음으로 처리

		if(0) {	// Debug ; HH 시 MM*10 분 이내는 창구마감 가능
			int nHH = 19;
			int nMMM = 1;
			nMM = (nHH * 6 + nMMM) * (2 * 5);
		}		// ~Debug

		/* // 20210709 DEL
		nSvrKind = GetConfigServerKind();
		TR_LOG_OUT("창구마감 ... nSvrKind(%d)", nSvrKind);
		*/ // 20210709 ~DEL

		if ( (nSvrKind & SVR_DVS_CCBUS) && (0 < strlen(pAccum->sCls_Ccs_dt)) )		// CCBUS 창구마감일자 정보가 있는 경우
		{
			std::string str_Ccs_dt = pAccum->sCls_Ccs_dt;
			CTime tCls = Util_CTimeFromString((char *)str_Ccs_dt.substr(0,8).c_str(), (char *)str_Ccs_dt.substr(8,6).c_str());
			CTime tZro = Util_CTimeFromString((char *)str_Ccs_dt.substr(0,8).c_str(), "000000");	//// 창구마감일자 당일 00:00:00

			CTimeSpan times = tNow - tCls;
			LONGLONG nDays = tNow.GetDay() - tCls.GetDay();
			LONGLONG nSecs = times.GetTotalSeconds();

			CTimeSpan gaptimes = tCls - tZro;
			LONGLONG ngapZero = gaptimes.GetTotalSeconds();

			TR_LOG_OUT("CCBUS 창구마감 ... sCls_Ccs_dt(%s), days(%lld), nSecs(%lld), ngapZero(%lld), nMM(%lld)"
				, pAccum->sCls_Ccs_dt, nDays, nSecs, ngapZero, nMM*60);		// 20210513 MOD 

			//// 창구마감일자로 당일 마감여부 체크 - 전일 마감 실행됐지만 24:00:00 을 넘겨 당일 00:MM:00 안으로 마감된 것은 당일 마감처리내역 없음으로 처리
			if ( ( (nDays == 0) && (ngapZero < (nMM * 60)) ) || ( nDays != 0 ) )		// 마감일자가 같고 이전 마감시간이 0시 기준 MM분 이내 or 마감일자가 다른 경우
			{
				TR_LOG_OUT("CCBUS 창구마감 ... 당일 마감처리내역 없음 !!!");
			}
			else {
				TR_LOG_OUT("CCBUS 창구마감 ... 당일 마감처리내역 있음 (-2) !!!");
				SetCheckEventCode(EC_JOB_CLOSE, FALSE);
				nCls_Ccs_flag = 1;
				//return -4;
			}
		}
		if ( (nSvrKind & SVR_DVS_KOBUS) && (0 < strlen(pAccum->sCls_Kobus_dt)) )		// KOBUS고속 창구마감일자 정보가 있는 경우
		{
			std::string str_Kobus_dt = pAccum->sCls_Kobus_dt;
			CTime tCls = Util_CTimeFromString((char *)str_Kobus_dt.substr(0,8).c_str(), (char *)str_Kobus_dt.substr(8,6).c_str());
			CTime tZro = Util_CTimeFromString((char *)str_Kobus_dt.substr(0,8).c_str(), "000000");	//// 창구마감일자 당일 00:00:00

			CTimeSpan times = tNow - tCls;
			LONGLONG nDays = tNow.GetDay() - tCls.GetDay();
			LONGLONG nSecs = times.GetTotalSeconds();

			CTimeSpan gaptimes = tCls - tZro;
			LONGLONG ngapZero = gaptimes.GetTotalSeconds();

			TR_LOG_OUT("KOBUS고속 창구마감 ... sCls_Kobus_dt(%s), days(%lld), nSecs(%lld), ngapSecs(%lld), nMM(%lld)"
				, pAccum->sCls_Kobus_dt, nDays, nSecs, ngapZero, nMM*60);		// 20210513 MOD 

			//// 창구마감일자로 당일 마감여부 체크 - 전일 마감 실행됐지만 24:00:00 을 넘겨 당일 00:MM:00 안으로 마감된 것은 당일 마감처리내역 없음으로 처리
			if ( ( (nDays == 0) && (ngapZero < (nMM * 60)) ) || ( nDays != 0 ) )		// 마감일자가 같고 이전 마감시간이 0시 기준 MM분 이내 or 마감일자가 다른 경우
			{
				TR_LOG_OUT("KOBUS고속 창구마감 ... 당일 마감처리내역 없음 !!!");
			}
			else {
				TR_LOG_OUT("KOBUS고속 창구마감 ... 당일 마감처리내역 있음 (-2) !!!");
				SetCheckEventCode(EC_JOB_CLOSE, FALSE);
				nCls_Kobus_flag = 1;
				//return -2;
			}
		}
		if ( (nSvrKind & SVR_DVS_TMEXP) && (0 < strlen(pAccum->sCls_Tmoney_dt)) )		// Tmoney고속 창구마감일자 정보가 있는 경우
		{
			std::string str_Tmoney_dt = pAccum->sCls_Tmoney_dt;
			CTime tCls = Util_CTimeFromString((char *)str_Tmoney_dt.substr(0,8).c_str(), (char *)str_Tmoney_dt.substr(8,6).c_str());
			CTime tZro = Util_CTimeFromString((char *)str_Tmoney_dt.substr(0,8).c_str(), "000000");	//// 창구마감일자 당일 00:00:00

			CTimeSpan times = tNow - tCls;
			LONGLONG nDays = tNow.GetDay() - tCls.GetDay();
			LONGLONG nSecs = times.GetTotalSeconds();

			CTimeSpan gaptimes = tCls - tZro;
			LONGLONG ngapZero = gaptimes.GetTotalSeconds();

			TR_LOG_OUT("Tmoney고속 창구마감 ... sCls_Tmoney_dt(%s), days(%lld), nSecs(%lld), ngapSecs(%lld), nMM(%lld)"
				, pAccum->sCls_Tmoney_dt, nDays, nSecs, ngapZero, nMM*60);		// 20210513 MOD 

			//// 창구마감일자로 당일 마감여부 체크 - 전일 마감 실행됐지만 24:00:00 을 넘겨 당일 00:MM:00 안으로 마감된 것은 당일 마감처리내역 없음으로 처리
			if ( ( (nDays == 0) && (ngapZero < (nMM * 60)) ) || ( nDays != 0 ) )		// 마감일자가 같고 이전 마감시간이 0시 기준 MM분 이내 or 마감일자가 다른 경우
			{
				TR_LOG_OUT("Tmoney고속 창구마감 ... 당일 마감처리내역 없음 !!!");
			}
			else {
				TR_LOG_OUT("Tmoney고속 창구마감 ... 당일 마감처리내역 있음 (-2) !!!");
				SetCheckEventCode(EC_JOB_CLOSE, FALSE);
				nCls_Tmoney_flag = 1;
				//return -1;
			}
		}
	}

	TR_LOG_OUT("창구마감 ..... nSvrKind(%d), nCls_Ccs_flag(%d), nCls_Kobus_flag(%d), nCls_Tmoney_flag(%d)", nSvrKind, nCls_Ccs_flag, nCls_Kobus_flag, nCls_Tmoney_flag);	// 20210709 ADD

	//// '당일 마감처리내역 있음'이 하나라도 있으면 (-)값 return, 없는 경우에 이하 실행
	if ( (nCls_Ccs_flag == 1) | (nCls_Kobus_flag == 1) | (nCls_Tmoney_flag == 1) )
	{
		nRet = (nCls_Ccs_flag << 2 | nCls_Kobus_flag  << 1 | nCls_Tmoney_flag << 0); 
		return -nRet;
	}
	// 20210513 ~ADD

	pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();
	if(pEnv->tService.szPrintFlag[0] == 'Y')
	{
		TR_LOG_OUT("창구마감 프린트 ..");
		Printer_Account2Info();
		Sleep(1000 * 4);
	}

	{
		char close_opt_yn;

		/// 지폐함 회수
		AddAccumBillData(ACC_BILL_WITHDRAW, (WORD)pAccum->Curr.BillBox.n1k, (WORD)pAccum->Curr.BillBox.n5k, (WORD)pAccum->Curr.BillBox.n10k, (WORD)pAccum->Curr.BillBox.n50k);
		/// 승차권 수집함 회수
		AddAccumTicketData(ACC_TICKETBOX_WITHDRAW, 0);
		
		/// 2021.03.25 add code
		close_opt_yn = GetConfigCloseOptValue();
		/// ~2021.03.25 add code
		if( (close_opt_yn == 'Y') || (close_opt_yn == 'y') )
		{  /// 시재마감 여부 flag
			AddCashCloseData();
			AddAccumCashCloseData();
		}

		/// 2021.03.26 add code
		AddDayCloseData();
		/// ~2021.03.26 add code

		TR_LOG_OUT("창구마감 AddAccumWndCloseData ..");
		AddAccumWndCloseData();
		
		UI_AddDevAccountInfo();
	}

	TR_LOG_OUT("창구마감 실행... nSvrKind(%d), nCls_Ccs_flag(%d), nCls_Kobus_flag(%d), nCls_Tmoney_flag(%d)", nSvrKind, nCls_Ccs_flag, nCls_Kobus_flag, nCls_Tmoney_flag);	// 20210709 ADD

	// nSvrKind = GetConfigServerKind();						// 20210513 DEL

	// if(nSvrKind & SVR_DVS_CCBUS)								// 20210513 DEL
	if( (nSvrKind & SVR_DVS_CCBUS) && (nCls_Ccs_flag != 1) )	// 20210513 MOD	
	{	/// 시외버스 - 자동 창구마감
		nRet = Svr_IfSv_164();
		if(nRet < 0)
		{	/// 창구마감 실패
			TR_LOG_OUT("[시외서버] 창구마감 실패, nRet(%d) !!!", nRet);
		}
		else
		{	/// 창구마감 성공
			TR_LOG_OUT("[시외서버] 창구마감 성공 !!!");
			AddAccumWndClsCcsData();							// 20210513 ADD
		}
	}

	// if(nSvrKind & SVR_DVS_KOBUS)								// 20210513 DEL
	if( (nSvrKind & SVR_DVS_KOBUS) && (nCls_Kobus_flag != 1) )	// 20210513 MOD
	{	/// 코버스 - 자동 창구마감
		nRet = Kobus_TK_CloseWnd();
		if(nRet < 0)
		{	/// 창구마감 실패
			TR_LOG_OUT("[코버스] 창구마감 실패, nRet(%d) !!!", nRet);	
		}
		else
		{	/// 창구마감 성공
			TR_LOG_OUT("[코버스] 창구마감 성공, nRet(%d) !!!", nRet);	
			AddAccumWndClsKobusData();							// 20210513 ADD
		}
	}

	// if(nSvrKind & SVR_DVS_TMEXP)								// 20210513 DEL
	if( (nSvrKind & SVR_DVS_TMEXP) && (nCls_Tmoney_flag != 1) )	// 20210513 MOD	
	{	/// 티머니고속 - 자동 창구마감
		nRet = TmExp_TK_CloseWnd();
		if(nRet < 0)
		{	/// 창구마감 실패
			TR_LOG_OUT("[티머니고속] 창구마감 실패, nRet(%d) !!!", nRet);
		}
		else
		{	/// 창구마감 성공
			TR_LOG_OUT("[티머니고속] 창구마감 성공 !!!");
			AddAccumWndClsTmoneyData();							// 20210513 ADD
		}
	}

	//// 창구마감 실패가 발생한 경우에도, 프로그램재시작 등 이하 실행.						// 20210513 ADD

	Sleep(1000);

	::GetLocalTime(&st);
	sprintf(s_config_t.job_start_dt, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	WriteConfigFile((char *)&s_config_t);

	TR_LOG_OUT("프로그램 종료 !!!");

	{
		DWORD dwTick;

		/// Sleep
		dwTick = ::GetTickCount();
		while( Util_CheckExpire(dwTick) < (1000 * 1) );

		TR_LOG_OUT("Rebooting...... !!! TRUE(%d)", bReboot);	// 20210513 ADD
		if(bReboot == TRUE)
		{
			/// Launcher 프로그램 종료
			{
				CString		strUiName("Launcher.exe");
				Util_CloseProcess(strUiName);

				/// Sleep
				dwTick = ::GetTickCount();
				while( Util_CheckExpire(dwTick) < (1000 * 2) );
			}

			/// UI 프로그램 종료
			{
				CString		strUiName("TicketMachine.exe");
				Util_CloseProcess(strUiName);

				/// Sleep
				dwTick = ::GetTickCount();
				while( Util_CheckExpire(dwTick) < (1000 * 1) );
			}

			/// Rebooting
			{
				Util_SystemReboot(1);
				dwTick = ::GetTickCount();
				while( Util_CheckExpire(dwTick) < (1000 * 2) );
			}

			/// 자기프로그램 종료
			{
				exit(0);		
			}
		}
		else
		{
			dwTick = ::GetTickCount();
			while( Util_CheckExpire(dwTick) < (1000 * 2) );

			CString		strUiName("TicketMachine.exe");
			Util_CloseProcess(strUiName);

			dwTick = ::GetTickCount();
			while( Util_CheckExpire(dwTick) < (1000 * 2) );

			exit(0);		
		}
	}

	return 0;
}

/**
 * @brief		CheckAutoClose
 * @details		자동마감 체크
 * @param		None
 * @return		항상 = 0
 */
int CheckAutoClose(void)
{
	PKIOSK_INI_ENV_T pEnv;
	
	try
	{
		if( GetEventCode(EC_JOB_CLOSE) )
		{
			pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();
			if(pEnv->tService.szRebootFlag[0] == 'Y')
			{	/// rebooting
				TR_LOG_OUT("자동 창구마감 시작, Reboot !!!");
				WndKioskClose(TRUE);
			}
			else
			{	/// 프로그램 종료
				TR_LOG_OUT("자동 창구마감 시작, Program Exit !!!");
				WndKioskClose(FALSE);
			}
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		InitConfigFile
 * @details		Init Config File 
 * @param		None
 * @return		항상 : 0
 */
int InitConfigFile(void)
{
	int nRet;
	CString strMAC;
	CString strIP;
	char* pBuff;
	char* pIpBuff;

	::ZeroMemory(&s_config_t, sizeof(OPER_FILE_CONFIG_T));

	nRet = OperReadFile(OPER_FILE_ID_CONFIG, (char *)&s_config_t, sizeof(OPER_FILE_CONFIG_T));
	if(nRet < 0)
	{
		nRet = DefaultConfigFile();
	}
	else
	{
		TR_LOG_OUT("sw_ic_pay_yn = 0x%02X", s_config_t.base_t.sw_ic_pay_yn);
		TR_LOG_OUT("sw_cash_pay_yn = 0x%02X", s_config_t.base_t.sw_cash_pay_yn);
		TR_LOG_OUT("sw_rf_pay_yn = 0x%02X", s_config_t.base_t.sw_rf_pay_yn);
	}

	// ini 파일정보를 config 정보에 assign
	{
		CString strIP = "";
		CString strMAC = "";
		//PBASE_INFO_T pBase;
		PKIOSK_INI_ENV_T pEnv;

		pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();

		/// 프로토콜 버젼
		s_config_t.base_t.version				= pEnv->nProtoVer & 0xFF;		
		/// 제조사 ('A':에이텍, 'N':한전금)
		s_config_t.base_t.manufacturer			= pEnv->szDeviceDVS[0] & 0xFF;		
		/// (HW) IC 카드리더기 사용유무
		s_config_t.base_t.hw_ic_use_yn			= pEnv->szHwIcUseYN[0] & 0xFF;
		/// (HW) 현금 사용유무
		s_config_t.base_t.hw_cash_use_yn		= pEnv->szHwCashUseYN[0] & 0xFF;
		/// (HW) RF 리더기 사용유무
		s_config_t.base_t.rf_hw_use_yn			= pEnv->szHwRfUseYN[0] & 0xFF;

		/// 창구마감 사용유무
		s_config_t.base_t.auto_close_yn			= pEnv->tService.szAutoClseYN[0] & 0xFF;
		TR_LOG_OUT("INI 설정값, auto_close_yn = 0x%02X", s_config_t.base_t.auto_close_yn);				// 20210728 ADD

		/// 운영사 정보
		s_config_t.base_t.oper_corp				= pEnv->nOperCorp & 0xFF;

		///> [이지 시외서버]
		{
			PTERMINAL_INFO_T	pTrmlInfo;

			pTrmlInfo = &s_config_t.ccTrmlInfo_t;

// 			///> 사용유무
// 			s_config_t.base_t.ccs_svr_kind = '0';
// 			if(pEnv->tCcInfo.nUse != 0)
// 			{
// 				s_config_t.base_t.ccs_svr_kind = '1';
// 			}
			///> 터미널 창구번호
			sprintf(pTrmlInfo->szWndNo, "%d", pEnv->tCcInfo.nTrmlWndNo);
			///> 터미널 코드(7)
			sprintf(pTrmlInfo->szCode7, "%07d", pEnv->tCcInfo.nTrmlCD);
			///> 단축 터미널 코드(4)
			sprintf(pTrmlInfo->szCode4, "%04d", pEnv->tCcInfo.nShctTrmlCD);
			///> user_no
			sprintf(pTrmlInfo->szUserNo, "%s", pEnv->tCcInfo.szUserNo);
			///> passwd
			sprintf(pTrmlInfo->szUserPwd, "%s", pEnv->tCcInfo.szUserPwd);
		}

		///> [코버스 고속서버]
		{
			PTERMINAL_INFO_T	pTrmlInfo;

			pTrmlInfo = &s_config_t.koTrmlInfo_t;

			///> 터미널 창구번호
			sprintf(pTrmlInfo->szWndNo, "%02d", pEnv->tKoInfo.nTrmlWndNo);
			///> 터미널 코드(7)
			sprintf(pTrmlInfo->szCode7, "%03d", pEnv->tKoInfo.nTrmlCD);
			///> 단축 터미널 코드(4)
			sprintf(pTrmlInfo->szCode4, "%03d", pEnv->tKoInfo.nTrmlCD);
			///> user_no
			sprintf(pTrmlInfo->szUserNo, "%s", pEnv->tKoInfo.szUserNo);
			///> passwd
			sprintf(pTrmlInfo->szUserPwd, "%s", pEnv->tKoInfo.szUserPwd);
		}

		///> [이지 고속서버]
		{
			PTERMINAL_INFO_T	pTrmlInfo;

			pTrmlInfo = &s_config_t.ezTrmlInfo_t;

			///> 터미널 창구번호
			sprintf(pTrmlInfo->szWndNo, "%02d", pEnv->tEzInfo.nTrmlWndNo);
			///> 터미널 코드(7)
			sprintf(pTrmlInfo->szCode7, "%03d", pEnv->tEzInfo.nTrmlCD);
			///> 단축 터미널 코드(4)
			sprintf(pTrmlInfo->szCode4, "%03d", pEnv->tEzInfo.nTrmlCD);
			///> user_no
			sprintf(pTrmlInfo->szUserNo, "%s", pEnv->tEzInfo.szUserNo);
			///> passwd
			sprintf(pTrmlInfo->szUserPwd, "%s", pEnv->tEzInfo.szUserPwd);
		}

		///> 시외서버 종류
		s_config_t.base_t.ccs_svr_kind = '0';
		s_config_t.ccTrmlInfo_t.nUse = 0;
		if( pEnv->tCcInfo.nUse != 0 )
		{	/// 시외
			s_config_t.base_t.ccs_svr_kind = '1';
			s_config_t.ccTrmlInfo_t.nUse = 1;
		}


		///> 고속서버 종류
		s_config_t.base_t.exp_svr_kind = '0';
		int nUseCount = pEnv->tKoInfo.nUse + pEnv->tEzInfo.nUse;

		s_config_t.base_t.exp_svr_kind = '0';
		s_config_t.koTrmlInfo_t.nUse = 0;
		s_config_t.ezTrmlInfo_t.nUse = 0;
		if(nUseCount > 0)
		{
			 if( pEnv->tKoInfo.nUse != 0 )
			 {	/// 코버스
				s_config_t.base_t.exp_svr_kind = '2';
				s_config_t.koTrmlInfo_t.nUse = 1;
			 }
			 else
			 {	/// 티머니고속
				 s_config_t.base_t.exp_svr_kind = '1';
				 s_config_t.ezTrmlInfo_t.nUse = 1;
			 }
		}
		/***
		if( (pEnv->tKoInfo.nUse == 0) && (pEnv->tEzInfo.nUse == 0) )
		{	/// 사용안함
			s_config_t.base_t.exp_svr_kind = '0';
		}
		else if( (pEnv->tKoInfo.nUse == 1) && (pEnv->tEzInfo.nUse == 0) )
		{	/// 코버스
			s_config_t.base_t.exp_svr_kind = '2';
		}
		else if( (pEnv->tKoInfo.nUse == 0) && (pEnv->tEzInfo.nUse == 1) )
		{	/// 티머니고속
			s_config_t.base_t.exp_svr_kind = '1';
		}
		***/

		///> 결제 수단
		/***
		if(pEnv->nPayMethod == 1)
		{
			s_config_t.base_t.pay_method = PAY_ONLY_CARD;
		}
		else if(pEnv->nPayMethod == 2)
		{
			s_config_t.base_t.pay_method = PAY_ONLY_CASH;
		}
		else if(pEnv->nPayMethod == 3)
		{
			s_config_t.base_t.pay_method = PAY_CARD_CASH;
		}
		else
		{
			s_config_t.base_t.pay_method = 0x30;
		}
		***/

		///> [결제] IC 카드리더기 사용유무
		s_config_t.base_t.sw_ic_pay_yn	 = pEnv->szSwIcUseYN[0];
		///> [결제] 현금 사용유무
		s_config_t.base_t.sw_cash_pay_yn = pEnv->szSwCashUseYN[0];
		///> [결제] RF 사용유무
		s_config_t.base_t.sw_rf_pay_yn	 = pEnv->szSwRfUseYN[0];

		///> (119). 비배차 시간표시 옵션(YN)										// 20220902 ADD
		s_config_t.base_t.alcn_way_dvs_yn = pEnv->tService.szAlcnWayDvsFlag[0];	// 20220902 ADD
		TR_LOG_OUT(" 비배차 시간표시 옵션(YN)(alcn_way_dvs_yn) = 0x%02X", s_config_t.base_t.alcn_way_dvs_yn);	 // 20220902 ADD

		///> mac address
		Util_GetMacAddress(strMAC, strIP);
		pBuff = LPSTR(LPCTSTR(strMAC));
		::ZeroMemory(s_config_t.myMacAddress, sizeof(s_config_t.myMacAddress));
		::CopyMemory(s_config_t.myMacAddress, pBuff, strlen(pBuff));
		TR_LOG_OUT("myMacAddress = %s", s_config_t.myMacAddress);

		// ip address
		pIpBuff = LPSTR(LPCTSTR(strIP));
		::ZeroMemory(s_config_t.myIPAddress, sizeof(s_config_t.myIPAddress));
		::CopyMemory(s_config_t.myIPAddress, pIpBuff, strlen(pIpBuff));
		TR_LOG_OUT("myIPAddress = %s", s_config_t.myIPAddress);
		
		SetOperConfigData();
	}

	return nRet;
}

/**
 * @brief		TermConfigFile
 * @details		Terminate Config File 
 * @param		None
 * @return		None
 */
void TermConfigFile(void)
{
	;
}

/**
 * @brief		CheckBuffer
 * @details		버퍼 체크
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CheckBuffer(char *pData, int nMax)
{
	int nLen;

	nLen = strlen(pData);
	if(nLen > nMax)
	{
		nLen = nMax;
	}
	return nLen;
}


