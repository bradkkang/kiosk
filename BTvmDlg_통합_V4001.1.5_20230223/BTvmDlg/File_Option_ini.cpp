
#include "stdafx.h"
#include <stdio.h>
#include <string.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "MyFileUtil.h"
#include "oper_config.h"
#include "File_Option_ini.h"

//----------------------------------------------------------------------------------------------------------------------
#define OPT_INI_FILE						"\\KIOSK_OPTION.INI"

#define GetProfString(A, B, C, D, E, F)		GetPrivateProfileString(A, B, C, D, E, F)
#define GetProfInt(A, B, C, D)				GetPrivateProfileInt(A, B, C, D)

#define SetProfString(A, B, C, D)			WritePrivateProfileString(A, B, C, D)

//----------------------------------------------------------------------------------------------------------------------

static KIOSK_INI_OPT_T	s_tKioskOpt;
static char				s_szIniFile[256];
static char				s_Buffer[200];

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		IsFileExist
 * @details		KIOSK_OPTION.INI 파일 유무 체크
 * @param		None
 * @return		성공 = TRUE, 실패 = FALSE
 */
static BOOL IsFileExist(void)
{
	int nRet;
	CString strFullName;

	::ZeroMemory(s_szIniFile, sizeof(s_szIniFile));

	Util_GetModulePath(s_szIniFile);
//	strcat(s_szIniFile, "\\KIOSK_OPTION.INI");
	strcat(s_szIniFile, (char *)OPT_INI_FILE);

	strFullName = (CString)	s_szIniFile;
	nRet = MyAccessFile(strFullName);
	if(nRet < 0)
	{
		TRACE("szIniFile = %s File Not Found !!!!\n", s_szIniFile);
		return FALSE;
	}

	return TRUE;
}

/**
 * @brief		INI_ReadOptConfig
 * @details		KIOSK_OPTION 파일의 Config 섹센 읽기
 * @param		None
 * @return		항상 : 0
 */
int INI_CtrlOptConfig(BOOL bWrite, char *pWrData)
{
	PKIOSK_CFG_OPT_T pOptConfig;
	
	if( IsFileExist() == FALSE )
	{
		return -1;
	}

	if(bWrite == FALSE)
	{	/// Read
		pOptConfig = (PKIOSK_CFG_OPT_T)	&s_tKioskOpt.cfg_t;

		///> (01) 관리자 모니터 사용유무 (Y/N)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_MAINT_LCD_YN, "N", pOptConfig->maint_lcd_yn, sizeof(pOptConfig->maint_lcd_yn) - 1, s_szIniFile);

		///> (02) 자동 창구마감 사용유무 (Y/N)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_AUTO_CLOSE_YN, "Y", pOptConfig->auto_close_yn, sizeof(pOptConfig->maint_lcd_yn) - 1, s_szIniFile);

		///> (03) 자동 창구마감 시간 (시/분/초)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_AUTO_CLOSE_TIME, "235959", pOptConfig->auto_close_time, sizeof(pOptConfig->auto_close_time) - 1, s_szIniFile);

		///> (04) 관제서버 사용유무 (Y/N)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_MON_SVR_YN, "N", pOptConfig->sms_yn, sizeof(pOptConfig->sms_yn) - 1, s_szIniFile);

		///> (05) 관제서버 IP
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_MON_SVR_IP, "127.0.0.1", pOptConfig->sms_ip, sizeof(pOptConfig->sms_ip) - 1, s_szIniFile);

		///> (06) 카메라 1번 사용유무 (Y/N)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_CAM_1_YN, "Y", pOptConfig->cam_1_yn, sizeof(pOptConfig->cam_1_yn) - 1, s_szIniFile);

		///> (07) 카메라 2번 사용유무 (Y/N)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_CAM_2_YN, "Y", pOptConfig->cam_2_yn, sizeof(pOptConfig->cam_2_yn) - 1, s_szIniFile);

		///> (08) 카메라 3번 사용유무 (Y/N)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_CAM_3_YN, "N", pOptConfig->cam_3_yn, sizeof(pOptConfig->cam_3_yn) - 1, s_szIniFile);

		///> (09) 화면 대기 시간 (초)
		pOptConfig->screen_wait_time = GetProfInt(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_SCREEN_WAIT_TIME, 180, s_szIniFile);

		///> (10) 알림창 대기 시간 (초)
		pOptConfig->noti_wait_time = GetProfInt(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_NOTI_WAIT_TIME, 180, s_szIniFile);

		///> (11) 당일 도착지 운행종료 표시 여부 (Y/N)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_OPEND_TRML_YN, "Y", pOptConfig->opend_trml_yn, sizeof(pOptConfig->opend_trml_yn) - 1, s_szIniFile);

		///> (12) 판매 사용 여부 (Y/N)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_TRML_OP_YN, "Y", pOptConfig->trml_op_yn, sizeof(pOptConfig->trml_op_yn) - 1, s_szIniFile);

		///> (13) 판매시작시간
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_TRML_BEGIN_TIME, "000000", pOptConfig->trml_begin_time, sizeof(pOptConfig->trml_begin_time) - 1, s_szIniFile);

		///> (14) 판매종료시간
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_TRML_END_TIME, "235959", pOptConfig->trml_end_time, sizeof(pOptConfig->trml_end_time) - 1, s_szIniFile);

		///> (15) 판매종료 시, LCD OFF 유무 (Y:LCD OFF, N:LCD ON)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_OPEND_LCD_OFF, "Y", pOptConfig->opend_lcd_off, sizeof(pOptConfig->opend_lcd_off) - 1, s_szIniFile);

		///> (16) IC 카드리더기 삽입여부 (YN)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_IC_READER_INS_YN, "Y", pOptConfig->ic_reader_ins_yn, sizeof(pOptConfig->ic_reader_ins_yn) - 1, s_szIniFile);

		///> (17) 다국어 사용유무 (YN)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_MULTI_LANG_YN, "Y", pOptConfig->multi_lang_yn, sizeof(pOptConfig->multi_lang_yn) - 1, s_szIniFile);

		///> (18) 안내음성 사용유무 (YN)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_SOUND_YN, "Y", pOptConfig->sound_yn, sizeof(pOptConfig->sound_yn) - 1, s_szIniFile);

		///> (19) 메인화면 안내음성 반복 시간(초)
		pOptConfig->main_sound_time = GetProfInt(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_MAIN_SOUND_TIME, 300, s_szIniFile);

		///> (20) 결행배차 표기 유무 (YN)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_BNCL_YN, "Y", pOptConfig->bncl_yn, sizeof(pOptConfig->bncl_yn) - 1, s_szIniFile);

		///> (21) 매진배차 표기 유무 (YN)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_SOLD_OUT_YN, "Y", pOptConfig->sold_out_yn, sizeof(pOptConfig->sold_out_yn) - 1, s_szIniFile);

		///> (22) 배차상태 색깔 표시 유무 (YN)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_ALCN_COLOR_YN, "Y", pOptConfig->alcn_color_yn, sizeof(pOptConfig->alcn_color_yn) - 1, s_szIniFile);

		///> (23) 선택할인 버튼 변경 유무 (YN)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_DISC_BTN_YN, "Y", pOptConfig->disc_btn_yn, sizeof(pOptConfig->disc_btn_yn) - 1, s_szIniFile);

		///> (24) 노선 검색 사용유무 (YN)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_ROT_SEARCH_YN, "N", pOptConfig->rot_search_yn, sizeof(pOptConfig->rot_search_yn) - 1, s_szIniFile);

		///> (25) 인천공항 알림 팝업 사용여부 (YN)
		GetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_INCHEON_AIR_POPUP_YN, "N", pOptConfig->incheon_air_popup_yn, sizeof(pOptConfig->incheon_air_popup_yn) - 1, s_szIniFile);
	}
	else
	{	/// Write
		pOptConfig = (PKIOSK_CFG_OPT_T) pWrData;

		///> (01) 관리자 모니터 사용유무 (Y/N)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_MAINT_LCD_YN, pOptConfig->maint_lcd_yn, s_szIniFile);

		///> (02) 자동 창구마감 사용유무 (Y/N)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_AUTO_CLOSE_YN, pOptConfig->auto_close_yn, s_szIniFile);

		///> (03) 자동 창구마감 시간 (시/분/초)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_AUTO_CLOSE_TIME, pOptConfig->auto_close_time, s_szIniFile);

		///> (04) 관제서버 사용유무 (Y/N)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_MON_SVR_YN, pOptConfig->sms_yn, s_szIniFile);

		///> (05) 관제서버 IP
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_MON_SVR_IP, pOptConfig->sms_ip, s_szIniFile);

		///> (06) 카메라 1번 사용유무 (Y/N)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_CAM_1_YN, pOptConfig->cam_1_yn, s_szIniFile);

		///> (07) 카메라 2번 사용유무 (Y/N)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_CAM_2_YN, pOptConfig->cam_2_yn, s_szIniFile);

		///> (08) 카메라 3번 사용유무 (Y/N)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_CAM_3_YN, pOptConfig->cam_3_yn, s_szIniFile);

		///> (09) 화면 대기 시간 (초)
		sprintf(s_Buffer, "%d", pOptConfig->screen_wait_time);
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_SCREEN_WAIT_TIME, s_Buffer, s_szIniFile);

		///> (10) 알림창 대기 시간 (초)
		sprintf(s_Buffer, "%d", pOptConfig->noti_wait_time);
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_NOTI_WAIT_TIME, s_Buffer, s_szIniFile);

		///> (11) 당일 도착지 운행종료 표시 여부 (Y/N)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_OPEND_TRML_YN, pOptConfig->opend_trml_yn, s_szIniFile);

		///> (12) 판매 사용 여부 (Y/N)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_TRML_OP_YN, pOptConfig->trml_op_yn, s_szIniFile);

		///> (13) 판매시작시간
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_TRML_BEGIN_TIME, pOptConfig->trml_begin_time, s_szIniFile);

		///> (14) 판매종료시간
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_TRML_END_TIME, pOptConfig->trml_end_time, s_szIniFile);

		///> (15) 판매종료 시, LCD OFF 유무 (Y:LCD OFF, N:LCD ON)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_OPEND_LCD_OFF, pOptConfig->opend_lcd_off, s_szIniFile);

		///> (16) IC 카드리더기 삽입여부 (YN)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_IC_READER_INS_YN, pOptConfig->ic_reader_ins_yn, s_szIniFile);

		///> (17) 다국어 사용유무 (YN)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_MULTI_LANG_YN, pOptConfig->multi_lang_yn, s_szIniFile);

		///> (18) 안내음성 사용유무 (YN)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_SOUND_YN, pOptConfig->sound_yn, s_szIniFile);

		///> (19) 메인화면 안내음성 반복 시간(초)
		sprintf(s_Buffer, "%d", pOptConfig->main_sound_time);
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_MAIN_SOUND_TIME, s_Buffer, s_szIniFile);

		///> (20) 결행배차 표기 유무 (YN)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_BNCL_YN, pOptConfig->bncl_yn, s_szIniFile);

		///> (21) 매진배차 표기 유무 (YN)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_SOLD_OUT_YN, pOptConfig->sold_out_yn, s_szIniFile);

		///> (22) 배차상태 색깔 표시 유무 (YN)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_ALCN_COLOR_YN, pOptConfig->alcn_color_yn, s_szIniFile);

		///> (23) 선택할인 버튼 변경 유무 (YN)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_DISC_BTN_YN, pOptConfig->disc_btn_yn, s_szIniFile);

		///> (24) 노선 검색 사용유무 (YN)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_ROT_SEARCH_YN, pOptConfig->rot_search_yn, s_szIniFile);

		///> (25) 인천공항 알림 팝업 사용여부 (YN)
		SetProfString(SECTION_NM_CONFIG_INFO, KEY_NM_CFG_INCHEON_AIR_POPUP_YN, pOptConfig->incheon_air_popup_yn, s_szIniFile);

		::CopyMemory(&s_tKioskOpt.cfg_t, pWrData, sizeof(KIOSK_CFG_OPT_T));
	}

	return 0;
}

/**
 * @brief		INI_ReadOptLimitInfo
 * @details		KIOSK_OPTION 파일의 [LIMIT] 섹센 읽기
 * @param		None
 * @return		항상 : 0
 */
int INI_CtrlOptLimitInfo(BOOL bWrite, char *pWrData)
{
	PKIOSK_LIMITT_OPT_T pOptLimit;

	if( IsFileExist() == FALSE )
	{
		return -1;
	}

	if(bWrite == FALSE)
	{
		pOptLimit = (PKIOSK_LIMITT_OPT_T) &s_tKioskOpt.limit_t;

		///> (01) 승차권 최소 수량
		pOptLimit->n_tck_min_cnt = GetProfInt(SECTION_NM_LIMIT_INFO, KEY_NM_LMT_TCK_MIN_CNT, 10, s_szIniFile);

		///> (02) 1천원권 잔여 최소 수량
		pOptLimit->n_1k_min_cnt = GetProfInt(SECTION_NM_LIMIT_INFO, KEY_NM_LMT_1K_MIN_CNT, 10, s_szIniFile);

		///> (03) 1만원권 잔여 최소 수량
		pOptLimit->n_10k_min_cnt = GetProfInt(SECTION_NM_LIMIT_INFO, KEY_NM_LMT_10K_MIN_CNT, 10, s_szIniFile);

		///> (04) 100원 잔여 최소 수량
		pOptLimit->n_100_min_cnt = GetProfInt(SECTION_NM_LIMIT_INFO, KEY_NM_LMT_100_MIN_CNT, 10, s_szIniFile);

		///> (05) 500원 잔여 최소 수량
		pOptLimit->n_500_min_cnt = GetProfInt(SECTION_NM_LIMIT_INFO, KEY_NM_LMT_500_MIN_CNT, 10, s_szIniFile);

		///> (06) 지폐수집함 최대 수량
		pOptLimit->n_bill_box_max_cnt = GetProfInt(SECTION_NM_LIMIT_INFO, KEY_NM_LMT_BILL_BOX_MAX_CNT, 500, s_szIniFile);

		///> (07) 승차권 수집함 최대 수량
		pOptLimit->n_tck_box_max_cnt = GetProfInt(SECTION_NM_LIMIT_INFO, KEY_NM_LMT_TCK_BOX_MAX_CNT, 300, s_szIniFile);
	}
	else
	{
		pOptLimit = (PKIOSK_LIMITT_OPT_T) pWrData;

		///> (01) 승차권 최소 수량
		sprintf(s_Buffer, "%d", pOptLimit->n_tck_min_cnt);
		SetProfString(SECTION_NM_LIMIT_INFO, KEY_NM_LMT_TCK_MIN_CNT, s_Buffer, s_szIniFile);
		
		///> (02) 1천원권 잔여 최소 수량
		sprintf(s_Buffer, "%d", pOptLimit->n_1k_min_cnt);
		SetProfString(SECTION_NM_LIMIT_INFO, KEY_NM_LMT_1K_MIN_CNT, s_Buffer, s_szIniFile);

		///> (03) 1만원권 잔여 최소 수량
		sprintf(s_Buffer, "%d", pOptLimit->n_10k_min_cnt);
		SetProfString(SECTION_NM_LIMIT_INFO, KEY_NM_LMT_10K_MIN_CNT, s_Buffer, s_szIniFile);

		///> (04) 100원 잔여 최소 수량
		sprintf(s_Buffer, "%d", pOptLimit->n_100_min_cnt);
		SetProfString(SECTION_NM_LIMIT_INFO, KEY_NM_LMT_100_MIN_CNT, s_Buffer, s_szIniFile);

		///> (05) 500원 잔여 최소 수량
		sprintf(s_Buffer, "%d", pOptLimit->n_500_min_cnt);
		SetProfString(SECTION_NM_LIMIT_INFO, KEY_NM_LMT_500_MIN_CNT, s_Buffer, s_szIniFile);

		///> (06) 지폐수집함 최대 수량
		sprintf(s_Buffer, "%d", pOptLimit->n_bill_box_max_cnt);
		SetProfString(SECTION_NM_LIMIT_INFO, KEY_NM_LMT_BILL_BOX_MAX_CNT, s_Buffer, s_szIniFile);

		///> (07) 승차권 수집함 최대 수량
		sprintf(s_Buffer, "%d", pOptLimit->n_tck_box_max_cnt);
		SetProfString(SECTION_NM_LIMIT_INFO, KEY_NM_LMT_TCK_BOX_MAX_CNT, s_Buffer, s_szIniFile);
	
		::CopyMemory(&s_tKioskOpt.limit_t, pWrData, sizeof(KIOSK_LIMITT_OPT_T));
	}

	return 0;
}

/**
 * @brief		INI_ReadOptPayInfo
 * @details		KIOSK_OPTION 파일의 [PAY] 섹센 읽기
 * @param		None
 * @return		항상 : 0
 */
int INI_CtrlOptPayInfo(BOOL bWrite, char *pWrData)
{
	PKIOSK_PAY_OPT_T pOptPay;

	if( IsFileExist() == FALSE )
	{
		return -1;
	}

	if(bWrite == FALSE)
	{
		pOptPay = (PKIOSK_PAY_OPT_T) &s_tKioskOpt.pay_t;

		///> (01) 카드('1'), 현금('2'), 카드+현금('3')
		GetProfString(SECTION_NM_PAY_INFO, KEY_NM_PAY_WAY, "1", pOptPay->pay_way, sizeof(pOptPay->pay_way) - 1, s_szIniFile);

		///> (02) 현금영수증 사용유무 (Y/N)
		GetProfString(SECTION_NM_PAY_INFO, KEY_NM_PAY_CSRC_YN, "Y", pOptPay->pay_csrc_yn, sizeof(pOptPay->pay_csrc_yn) - 1, s_szIniFile);

		///> (03) 5만원 미만 무서명 (무서명:Y, 서명:N)
		GetProfString(SECTION_NM_PAY_INFO, KEY_NM_PAY_SIGN_PAD_YN, "Y", pOptPay->pay_sign_pad_yn, sizeof(pOptPay->pay_sign_pad_yn) - 1, s_szIniFile);

		///> (04) 카드 비밀번호 사용유무 (Y/N)
		GetProfString(SECTION_NM_PAY_INFO, KEY_NM_PAY_PASSWD_YN, "Y", pOptPay->pay_passwd_yn, sizeof(pOptPay->pay_passwd_yn) - 1, s_szIniFile);
	}
	else
	{
		pOptPay = (PKIOSK_PAY_OPT_T) pWrData;

		///> (01) 카드('1'), 현금('2'), 카드+현금('3')
		SetProfString(SECTION_NM_PAY_INFO, KEY_NM_PAY_WAY, pOptPay->pay_way, s_szIniFile);

		///> (02) 현금영수증 사용유무 (Y/N)
		SetProfString(SECTION_NM_PAY_INFO, KEY_NM_PAY_CSRC_YN, pOptPay->pay_csrc_yn, s_szIniFile);

		///> (03) 5만원 미만 무서명 (무서명:Y, 서명:N)
		SetProfString(SECTION_NM_PAY_INFO, KEY_NM_PAY_SIGN_PAD_YN, pOptPay->pay_sign_pad_yn, s_szIniFile);

		///> (04) 카드 비밀번호 사용유무 (Y/N)
		SetProfString(SECTION_NM_PAY_INFO, KEY_NM_PAY_PASSWD_YN, pOptPay->pay_passwd_yn, s_szIniFile);
	
		::CopyMemory(&s_tKioskOpt.pay_t, pWrData, sizeof(KIOSK_PAY_OPT_T));
	}

	return 0;
}

/**
 * @brief		INI_ReadOptMrnpInfo
 * @details		KIOSK_OPTION 파일의 [MRNP] 섹센 읽기
 * @param		None
 * @return		항상 : 0
 */
int INI_CtrlOptMrnpInfo(BOOL bWrite, char *pWrData)
{
	PKIOSK_MRNP_OPT_T pOptMrnp;

	if( IsFileExist() == FALSE )
	{
		return -1;
	}

	if(bWrite == FALSE)
	{
		pOptMrnp = (PKIOSK_MRNP_OPT_T) &s_tKioskOpt.mrnp_t;

		///> (01) 예매 기능 사용유무 (Y/N)
		GetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_USE_YN, "Y", pOptMrnp->mrnp_use_yn, sizeof(pOptMrnp->mrnp_use_yn) - 1, s_szIniFile);

		///> (02) 예매 전제내역 보기 (Y/N)
		GetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_ALL_LIST_YN, "Y", pOptMrnp->mrnp_all_list_yn, sizeof(pOptMrnp->mrnp_all_list_yn) - 1, s_szIniFile);

		///> (03) 예매 수기 조회 (Y/N)
		GetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_MANUAL_YN, "Y", pOptMrnp->mrnp_manual_yn, sizeof(pOptMrnp->mrnp_manual_yn) - 1, s_szIniFile);

		///> (04) 예매 1건 자동발매 (Y/N)
		GetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_1_AUTO_ISSUE, "Y", pOptMrnp->mrnp_1_auto_issue, sizeof(pOptMrnp->mrnp_1_auto_issue) - 1, s_szIniFile);

		///> (06) 예매 왕복발권 사용유무 (Y/N)
		GetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_RTRP_YN, "Y", pOptMrnp->mrnp_rtrp_yn, sizeof(pOptMrnp->mrnp_rtrp_yn) - 1, s_szIniFile);

		///> (07) 예매 비배차 비좌석노선 사용유무 (Y/N)
		GetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_NALCN_NSAT_YN, "Y", pOptMrnp->mrnp_nalcn_nsat_yn, sizeof(pOptMrnp->mrnp_nalcn_nsat_yn) - 1, s_szIniFile);

		///> (08) 예매 발권 제한 시간 (분)
		pOptMrnp->mrnp_limit_time = GetProfInt(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_LIMIT_TIME, 2, s_szIniFile);

		///> (09) 예매 당일 예약내역 모두 발권 가능여부 (YN)
		GetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_TODAY_ALL_ISSUE_YN, "N", pOptMrnp->mrnp_today_all_issue_yn, sizeof(pOptMrnp->mrnp_today_all_issue_yn) - 1, s_szIniFile);
	}
	else
	{
		pOptMrnp = (PKIOSK_MRNP_OPT_T) pWrData;

		///> (01) 예매 기능 사용유무 (Y/N)
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_USE_YN, pOptMrnp->mrnp_use_yn, s_szIniFile);

		///> (02) 예매 전제내역 보기 (Y/N)
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_ALL_LIST_YN, pOptMrnp->mrnp_all_list_yn, s_szIniFile);

		///> (03) 예매 수기 조회 (Y/N)
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_MANUAL_YN, pOptMrnp->mrnp_manual_yn, s_szIniFile);

		///> (04) 예매 1건 자동발매 (Y/N)
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_1_AUTO_ISSUE, pOptMrnp->mrnp_1_auto_issue, s_szIniFile);

		///> (05) 예매 발권 제한 일자 (일)
		//sprintf(s_Buffer, "%d", pOptMrnp->mrnp_issue_limit_day);
		//SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_ISSUE_LIMIT_DAY, s_Buffer, s_szIniFile);

		///> (06) 예매 왕복발권 사용유무 (Y/N)
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_RTRP_YN, pOptMrnp->mrnp_rtrp_yn, s_szIniFile);

		///> (07) 예매 비배차 비좌석노선 사용유무 (Y/N)
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_NALCN_NSAT_YN, pOptMrnp->mrnp_nalcn_nsat_yn, s_szIniFile);

		///> (08) 예매 발권 제한 시간 (분)
		sprintf(s_Buffer, "%d", pOptMrnp->mrnp_limit_time);
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_LIMIT_TIME, s_Buffer, s_szIniFile);

		///> (09) 예매 당일 예약내역 모두 발권 가능여부 (YN)
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_TODAY_ALL_ISSUE_YN, pOptMrnp->mrnp_today_all_issue_yn, s_szIniFile);
	
		::CopyMemory(&s_tKioskOpt.mrnp_t, pWrData, sizeof(KIOSK_MRNP_OPT_T));
	}

	return 0;
}

/**
 * @brief		INI_ReadOptPbTckInfo
 * @details		KIOSK_OPTION 파일의 [PBTCK] 섹센 읽기
 * @param		None
 * @return		항상 : 0
 */
int INI_CtrlOptPbTckInfo(BOOL bWrite, char *pWrData)
{
	PKIOSK_PBTCK_OPT_T pOptPbTck;

	if( IsFileExist() == FALSE )
	{
		return -1;
	}

	if(bWrite == FALSE)
	{
		pOptPbTck = (PKIOSK_PBTCK_OPT_T) &s_tKioskOpt.pbtck_t;

		///> (01) 현장발권 기능 사용유무 (Y/N)
		GetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_USE_YN, "Y", pOptPbTck->pbtck_use_yn, sizeof(pOptPbTck->pbtck_use_yn) - 1, s_szIniFile);

		///> (02) 빠른배차 조회 사용유무 (Y/N)
		GetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_QUICK_YN, "Y", pOptPbTck->pbtck_quick_yn, sizeof(pOptPbTck->pbtck_quick_yn) - 1, s_szIniFile);

		///> (03) 배차('D'), 비배차('N')
		GetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_ALCN_WAY, "Y", pOptPbTck->pbtck_alcn_way, sizeof(pOptPbTck->pbtck_alcn_way) - 1, s_szIniFile);

		///> (04) 즐겨찾기 사용유무 (Y/N)
		GetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_FAVORIT_YN, "Y", pOptPbTck->pbtck_favorit_yn, sizeof(pOptPbTck->pbtck_favorit_yn) - 1, s_szIniFile);

		///> (05) [시외] 왕복발권 사용유무 (Y/N)
		GetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_CCS_RTRP_YN, "Y", pOptPbTck->pbtck_ccs_rtrp_yn, sizeof(pOptPbTck->pbtck_ccs_rtrp_yn) - 1, s_szIniFile);

		///> (06) [고속] 왕복발권 사용유무 (Y/N)
		GetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_EXP_RTRP_YN, "Y", pOptPbTck->pbtck_exp_rtrp_yn, sizeof(pOptPbTck->pbtck_exp_rtrp_yn) - 1, s_szIniFile);

		///> (07) 고속노선 판매(시외고속) 사용유무 (Y/N)
		GetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_CCEXP_ROT_YN, "Y", pOptPbTck->pbtck_ccexp_rot_yn, sizeof(pOptPbTck->pbtck_ccexp_rot_yn) - 1, s_szIniFile);

		///> (08) 비배차 1건시 자동선택 유무 (Y/N)
		GetProfString(SECTION_NM_MRNP_INFO, KEY_NM_PBTCK_NALCN_1_YN, "Y", pOptPbTck->pbtck_nalcn_1_yn, sizeof(pOptPbTck->pbtck_nalcn_1_yn) - 1, s_szIniFile);

		///> (09) 비좌석 노선 무표발권 사용유무 (Y/N)
		GetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_NSAT_ISS_YN, "Y", pOptPbTck->pbtck_nsat_iss_yn, sizeof(pOptPbTck->pbtck_nsat_iss_yn) - 1, s_szIniFile);

		///> (10) 재발행 사용유무 (Y/N)
		GetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_REISSUE_YN, "Y", pOptPbTck->pbtck_reissue_yn, sizeof(pOptPbTck->pbtck_reissue_yn) - 1, s_szIniFile);

		///> (11) 즐겨찾기 터미널 갯수
		pOptPbTck->pbtck_favorit_cnt = GetProfInt(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_FAVORIT_CNT, 20, s_szIniFile);

		///> (12) 1회 발권 제한 수량
		pOptPbTck->pbtck_max_issue_cnt = GetProfInt(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_MAX_ISSUE_CNT, 10, s_szIniFile);

		///> (13) 1회 발권 제한 금액(현금)
		pOptPbTck->pbtck_max_issue_fare = GetProfInt(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_MAX_ISSUE_FARE, 100000, s_szIniFile);

		///> (14) 발권 제한 시간(분)
		pOptPbTck->pbtck_max_issue_time = GetProfInt(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_MAX_ISSUE_TIME, 2, s_szIniFile);

		///> (15) 예매 발권 제한 일자 (일)
		pOptPbTck->pbtck_issue_limit_day = GetProfInt(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_ISSUE_LIMIT_DAY, 1, s_szIniFile);
	}
	else
	{
		pOptPbTck = (PKIOSK_PBTCK_OPT_T) pWrData;

		///> (01) 현장발권 기능 사용유무 (Y/N)
		SetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_USE_YN, pOptPbTck->pbtck_use_yn, s_szIniFile);

		///> (02) 빠른배차 조회 사용유무 (Y/N)
		SetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_QUICK_YN, pOptPbTck->pbtck_quick_yn, s_szIniFile);

		///> (03) 배차('D'), 비배차('N')
		SetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_ALCN_WAY, pOptPbTck->pbtck_alcn_way, s_szIniFile);

		///> (04) 즐겨찾기 사용유무 (Y/N)
		SetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_FAVORIT_YN, pOptPbTck->pbtck_favorit_yn, s_szIniFile);

		///> (05) [시외] 왕복발권 사용유무 (Y/N)
		SetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_CCS_RTRP_YN, pOptPbTck->pbtck_ccs_rtrp_yn, s_szIniFile);

		///> (06) [고속] 왕복발권 사용유무 (Y/N)
		SetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_EXP_RTRP_YN, pOptPbTck->pbtck_exp_rtrp_yn, s_szIniFile);

		///> (07) 고속노선 판매(시외고속) 사용유무 (Y/N)
		SetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_CCEXP_ROT_YN, pOptPbTck->pbtck_ccexp_rot_yn, s_szIniFile);

		///> (08) 비배차 1건시 자동선택 유무 (Y/N)
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_PBTCK_NALCN_1_YN, pOptPbTck->pbtck_nalcn_1_yn, s_szIniFile);

		///> (09) 비좌석 노선 무표발권 사용유무 (Y/N)
		SetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_NSAT_ISS_YN, pOptPbTck->pbtck_nsat_iss_yn, s_szIniFile);

		///> (10) 재발행 사용유무 (Y/N)
		SetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_REISSUE_YN, pOptPbTck->pbtck_reissue_yn, s_szIniFile);

		///> (11) 즐겨찾기 터미널 갯수
		sprintf(s_Buffer, "%d", pOptPbTck->pbtck_favorit_cnt);
		SetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_FAVORIT_CNT, s_Buffer, s_szIniFile);

		///> (12) 1회 발권 제한 수량
		sprintf(s_Buffer, "%d", pOptPbTck->pbtck_max_issue_cnt);
		SetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_MAX_ISSUE_CNT, s_Buffer, s_szIniFile);

		///> (13) 1회 발권 제한 금액(현금)
		sprintf(s_Buffer, "%d", pOptPbTck->pbtck_max_issue_fare);
		SetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_MAX_ISSUE_FARE, s_Buffer, s_szIniFile);

		///> (14) 발권 제한 시간(분)
		sprintf(s_Buffer, "%d", pOptPbTck->pbtck_max_issue_time);
		SetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_MAX_ISSUE_TIME, s_Buffer, s_szIniFile);

		///> (15) 예매 발권 제한 일자 (일)
		sprintf(s_Buffer, "%d", pOptPbTck->pbtck_issue_limit_day);
		SetProfString(SECTION_NM_PBTCK_INFO, KEY_NM_PBTCK_ISSUE_LIMIT_DAY, s_Buffer, s_szIniFile);

		::CopyMemory(&s_tKioskOpt.pbtck_t, pWrData, sizeof(KIOSK_PBTCK_OPT_T));
	}

	return 0;
}

/**
 * @brief		INI_ReadOptRefundInfo
 * @details		KIOSK_OPTION 파일의 [REFUND] 섹센 읽기
 * @param		None
 * @return		항상 : 0
 */
int INI_CtrlOptRefundInfo(BOOL bWrite, char *pWrData)
{
	PKIOSK_REFUND_OPT_T pOptRefund;

	if( IsFileExist() == FALSE )
	{
		return -1;
	}

	if(bWrite == FALSE)
	{
		pOptRefund = (PKIOSK_REFUND_OPT_T) &s_tKioskOpt.refund_t;

		///> (01) 환불 기능 사용유무 (Y/N)
		GetProfString(SECTION_NM_REFUND_INFO, KEY_NM_REFUND_USE_YN, "Y", pOptRefund->refund_use_yn, sizeof(pOptRefund->refund_use_yn) - 1, s_szIniFile);
	}
	else
	{
		pOptRefund = (PKIOSK_REFUND_OPT_T) pWrData;

		///> (01) 환불 기능 사용유무 (Y/N)
		SetProfString(SECTION_NM_REFUND_INFO, KEY_NM_REFUND_USE_YN, pOptRefund->refund_use_yn, s_szIniFile);

		::CopyMemory(&s_tKioskOpt.refund_t, pWrData, sizeof(KIOSK_REFUND_OPT_T));
	}

	return 0;
}

/**
 * @brief		INI_ReadOptTicketInfo
 * @details		KIOSK_OPTION 파일의 [TICKET_INFO] 섹센 읽기
 * @param		None
 * @return		항상 : 0
 */
int INI_CtrlOptTicketInfo(BOOL bWrite, char *pWrData)
{
	PKIOSK_TCK_OPT_T pOptTicket;

	if( IsFileExist() == FALSE )
	{
		return -1;
	}

	if(bWrite == FALSE)
	{
		pOptTicket = (PKIOSK_TCK_OPT_T) &s_tKioskOpt.tck_opt_t;

		///> (01) 승차권 프린터 용지 포맷
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_FORMAT, "DF3", pOptTicket->tck_format, sizeof(pOptTicket->tck_format) - 1, s_szIniFile);

		///> (02) 승차권프린터('1'), 영수증프린터('2')
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_DEVICE_TYPE, "1", pOptTicket->tck_device_type, sizeof(pOptTicket->tck_device_type) - 1, s_szIniFile);

		///> (03) 승차권 터미널명 기준 글자 수 (기본:4)
		pOptTicket->tck_trml_name_len = GetProfInt(SECTION_NM_TICKET_INFO, KEY_NM_TCK_TRML_NAME_LEN, 4, s_szIniFile);

		///> (04) NO:0, ID:1, ID(이름):2, 이름:3
		pOptTicket->tck_agent_info_val = GetProfInt(SECTION_NM_TICKET_INFO, KEY_NM_TCK_AGENT_INFO_VAL, 0, s_szIniFile);

		///> (05) 바코드 높이(40 ~ 100) 기본:50
		pOptTicket->tck_barcode_height = GetProfInt(SECTION_NM_TICKET_INFO, KEY_NM_TCK_BARCODE_HEIGHT, 50, s_szIniFile);

		///> (06) 승차권 프린터 모델
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_PRINT_NAME, "825", pOptTicket->tck_print_name, sizeof(pOptTicket->tck_print_name) - 1, s_szIniFile);

		///> (07) 승차권 프린트 농도 값, 기본(-2)
		pOptTicket->tck_print_concent_val = GetProfInt(SECTION_NM_TICKET_INFO, KEY_NM_TCK_PRINT_CONCENT_VAL, -2, s_szIniFile);

		///> (08) 승차권 출력 에러시, 교체발권 사용유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_ERR_CHNG_ISSUE_YN, "Y", pOptTicket->tck_err_chng_issue_yn, sizeof(pOptTicket->tck_err_chng_issue_yn) - 1, s_szIniFile);

		///> (08) 승차권 환불 시, 소인유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_REFUND_MARK_YN, "Y", pOptTicket->tck_refund_mark_yn, sizeof(pOptTicket->tck_refund_mark_yn) - 1, s_szIniFile);

		///> (09) 승차권 고유번호 출력 유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_INHR_PRINT_YN, "N", pOptTicket->tck_inhr_print_yn, sizeof(pOptTicket->tck_inhr_print_yn) - 1, s_szIniFile);

		///> (10) 비배차 승차권 출발일 출력 유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_NALCN_DEPR_DAY_PRINT_YN, "N", pOptTicket->tck_nalcn_depr_day_print_yn, sizeof(pOptTicket->tck_nalcn_depr_day_print_yn) - 1, s_szIniFile);

		///> (11) 비좌석제 승차권 출발일 출력 유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_NSAT_DEPR_DAY_PRINT_YN, "N", pOptTicket->tck_nsat_depr_day_print_yn, sizeof(pOptTicket->tck_nsat_depr_day_print_yn) - 1, s_szIniFile);

		///> (12) 비좌석제 승차권 시간 출력 유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_NSAT_TIME_PRINT_YN, "N", pOptTicket->tck_nsat_time_print_yn, sizeof(pOptTicket->tck_nsat_time_print_yn) - 1, s_szIniFile);

		///> (13) 비배차, 비좌석노선 선착 선탑승 안내문구 출력 유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_NALCN_NSAT_NOTI_RPINT_YN, "N", pOptTicket->tck_nalcn_nsat_noti_rpint_yn, sizeof(pOptTicket->tck_nalcn_nsat_noti_rpint_yn) - 1, s_szIniFile);

		///> (14) 비배차 목록 시간, 운수사 표시 유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_NALCN_CACM_PRINT_YN, "N", pOptTicket->tck_nalcn_cacm_print_yn, sizeof(pOptTicket->tck_nalcn_cacm_print_yn) - 1, s_szIniFile);

		///> (15) 출발일 6글자 출력 유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_DEPR_DAY_6_PRINT_YN, "Y", pOptTicket->tck_depr_day_6_print_yn, sizeof(pOptTicket->tck_depr_day_6_print_yn) - 1, s_szIniFile);

		///> (16) 출발일 요일 출력 유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_DEPR_WEEK_PRINT_YN, "Y", pOptTicket->tck_depr_week_print_yn, sizeof(pOptTicket->tck_depr_week_print_yn) - 1, s_szIniFile);

		///> (17) 운수사 출력 유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_CACM_PRINT_YN, "Y", pOptTicket->tck_cacm_print_yn, sizeof(pOptTicket->tck_cacm_print_yn) - 1, s_szIniFile);

		///> (18) (승객용) 운수사 사업자 번호 출력 유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_YOU_CACM_NUMBER_PRINT_YN, "N", pOptTicket->tck_you_cacm_number_print_yn, sizeof(pOptTicket->tck_you_cacm_number_print_yn) - 1, s_szIniFile);

		///> (19) (회수용) 운수사 사업자 번호 출력 유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_MY_CACM_NUMBER_PRINT_YN, "N", pOptTicket->tck_my_cacm_number_print_yn, sizeof(pOptTicket->tck_my_cacm_number_print_yn) - 1, s_szIniFile);

		///> (20) 1차원 바코드 출력 유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_1_DIMENSION_PRINT_YN, "N", pOptTicket->tck_1_dimension_print_yn, sizeof(pOptTicket->tck_1_dimension_print_yn) - 1, s_szIniFile);

		///> (21) 2차원 바코드 출력 유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_2_DIMENSION_PRINT_YN, "N", pOptTicket->tck_2_dimension_print_yn, sizeof(pOptTicket->tck_2_dimension_print_yn) - 1, s_szIniFile);

		///> (22) 출발터미널 출력 유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_DEPR_TRML_PRINT_YN, "Y", pOptTicket->tck_depr_trml_print_yn, sizeof(pOptTicket->tck_depr_trml_print_yn) - 1, s_szIniFile);

		///> (23) 발권 메시지 출력 유무 (YN)
		GetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_MSG_PRINT_YN, "N", pOptTicket->tck_msg_print_yn, sizeof(pOptTicket->tck_msg_print_yn) - 1, s_szIniFile);
	}
	else
	{
		pOptTicket = (PKIOSK_TCK_OPT_T) pWrData;

		///> (01) 승차권 프린터 용지 포맷
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_FORMAT, pOptTicket->tck_format, s_szIniFile);

		///> (02) 승차권프린터('1'), 영수증프린터('2')
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_DEVICE_TYPE, pOptTicket->tck_device_type, s_szIniFile);

		///> (03) 승차권 터미널명 기준 글자 수 (기본:4)
		sprintf(s_Buffer, "%d", pOptTicket->tck_trml_name_len);
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_TRML_NAME_LEN, s_Buffer, s_szIniFile);

		///> (04) NO:0, ID:1, ID(이름):2, 이름:3
		sprintf(s_Buffer, "%d", pOptTicket->tck_agent_info_val);
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_AGENT_INFO_VAL, s_Buffer, s_szIniFile);

		///> (05) 바코드 높이(40 ~ 100) 기본:50
		sprintf(s_Buffer, "%d", pOptTicket->tck_barcode_height);
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_BARCODE_HEIGHT, s_Buffer, s_szIniFile);

		///> (06) 승차권 프린터 모델
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_PRINT_NAME, pOptTicket->tck_print_name, s_szIniFile);

		///> (07) 승차권 프린트 농도 값, 기본(-2)
		sprintf(s_Buffer, "%d", pOptTicket->tck_print_concent_val);
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_PRINT_CONCENT_VAL, s_Buffer, s_szIniFile);

		///> (08) 승차권 출력 에러시, 교체발권 사용유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_ERR_CHNG_ISSUE_YN, pOptTicket->tck_err_chng_issue_yn, s_szIniFile);

		///> (08) 승차권 환불 시, 소인유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_REFUND_MARK_YN, pOptTicket->tck_refund_mark_yn, s_szIniFile);

		///> (09) 승차권 고유번호 출력 유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_INHR_PRINT_YN, pOptTicket->tck_inhr_print_yn, s_szIniFile);

		///> (10) 비배차 승차권 출발일 출력 유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_NALCN_DEPR_DAY_PRINT_YN, pOptTicket->tck_nalcn_depr_day_print_yn, s_szIniFile);

		///> (11) 비좌석제 승차권 출발일 출력 유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_NSAT_DEPR_DAY_PRINT_YN, pOptTicket->tck_nsat_depr_day_print_yn, s_szIniFile);

		///> (12) 비좌석제 승차권 시간 출력 유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_NSAT_TIME_PRINT_YN, pOptTicket->tck_nsat_time_print_yn, s_szIniFile);

		///> (13) 비배차, 비좌석노선 선착 선탑승 안내문구 출력 유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_NALCN_NSAT_NOTI_RPINT_YN, pOptTicket->tck_nalcn_nsat_noti_rpint_yn, s_szIniFile);

		///> (14) 비배차 목록 시간, 운수사 표시 유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_NALCN_CACM_PRINT_YN, pOptTicket->tck_nalcn_cacm_print_yn, s_szIniFile);

		///> (15) 출발일 6글자 출력 유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_DEPR_DAY_6_PRINT_YN, pOptTicket->tck_depr_day_6_print_yn, s_szIniFile);

		///> (16) 출발일 요일 출력 유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_DEPR_WEEK_PRINT_YN, pOptTicket->tck_depr_week_print_yn, s_szIniFile);

		///> (17) 운수사 출력 유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_CACM_PRINT_YN, pOptTicket->tck_cacm_print_yn, s_szIniFile);

		///> (18) (승객용) 운수사 사업자 번호 출력 유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_YOU_CACM_NUMBER_PRINT_YN, pOptTicket->tck_you_cacm_number_print_yn, s_szIniFile);

		///> (19) (회수용) 운수사 사업자 번호 출력 유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_MY_CACM_NUMBER_PRINT_YN, pOptTicket->tck_my_cacm_number_print_yn, s_szIniFile);

		///> (20) 1차원 바코드 출력 유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_1_DIMENSION_PRINT_YN, pOptTicket->tck_1_dimension_print_yn, s_szIniFile);

		///> (21) 2차원 바코드 출력 유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_2_DIMENSION_PRINT_YN, pOptTicket->tck_2_dimension_print_yn, s_szIniFile);

		///> (22) 출발터미널 출력 유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_DEPR_TRML_PRINT_YN, pOptTicket->tck_depr_trml_print_yn, s_szIniFile);

		///> (23) 발권 메시지 출력 유무 (YN)
		SetProfString(SECTION_NM_TICKET_INFO, KEY_NM_TCK_MSG_PRINT_YN, pOptTicket->tck_msg_print_yn, s_szIniFile);
	
		::CopyMemory(&s_tKioskOpt.tck_opt_t, pWrData, sizeof(KIOSK_TCK_OPT_T));
	}

	return 0;
}


/**
 * @brief		INI_ReadOptionFile
 * @details		KIOSK_OPTION 파일 읽기
 * @param		None
 * @return		항상 : 0
 */
int INI_ReadOptionFile(void)
{
	::ZeroMemory(&s_tKioskOpt, sizeof(KIOSK_INI_OPT_T));

	if( IsFileExist() == FALSE )
	{
		return -1;
	}

	INI_CtrlOptConfig(FALSE, (char *) NULL);
	INI_CtrlOptLimitInfo(FALSE, (char *) NULL);
	INI_CtrlOptPayInfo(FALSE, (char *) NULL);
	INI_CtrlOptMrnpInfo(FALSE, (char *) NULL);
	INI_CtrlOptPbTckInfo(FALSE, (char *) NULL);
	INI_CtrlOptRefundInfo(FALSE, (char *) NULL);
	INI_CtrlOptTicketInfo(FALSE, (char *) NULL);

	return 0;
}

/**
 * @brief		INI_WriteOptionFile
 * @details		KIOSK_OPTION 파일 쓰기
 * @param		None
 * @return		항상 : 0
 */
int INI_WriteOptionFile(char *pWrData)
{
	PKIOSK_INI_OPT_T pMainOpt;

	if( IsFileExist() == FALSE )
	{
		return -1;
	}

	pMainOpt = (PKIOSK_INI_OPT_T) pWrData;

	INI_CtrlOptConfig(TRUE, (char *) &pMainOpt->cfg_t);
	INI_CtrlOptLimitInfo(TRUE, (char *) &pMainOpt->limit_t);
	INI_CtrlOptPayInfo(TRUE, (char *) &pMainOpt->pay_t);
	INI_CtrlOptMrnpInfo(TRUE, (char *) &pMainOpt->mrnp_t);
	INI_CtrlOptPbTckInfo(TRUE, (char *) &pMainOpt->pbtck_t);
	INI_CtrlOptRefundInfo(TRUE, (char *) &pMainOpt->refund_t);
	INI_CtrlOptTicketInfo(TRUE, (char *) &pMainOpt->tck_opt_t);

	return 0;
}

void INI_SetOptData(int nIndex, int nValue, char *pData)
{
	char *pValue;

	switch(nIndex)
	{
	case 1:		///> (01) 예매 기능 사용유무 (Y/N)
		pValue = s_tKioskOpt.mrnp_t.mrnp_use_yn;
		*pValue = nValue & 0xFF;
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_USE_YN, pValue, s_szIniFile);
		break;

	case 2:		///> (02) 예매 전제내역 보기 (Y/N)
		pValue = s_tKioskOpt.mrnp_t.mrnp_all_list_yn;
		*pValue = nValue & 0xFF;
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_ALL_LIST_YN, pValue, s_szIniFile);
		break;

	case 3:		///> (03) 예매 수기 조회 (Y/N)
		pValue = s_tKioskOpt.mrnp_t.mrnp_manual_yn;
		*pValue = nValue & 0xFF;
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_MANUAL_YN, pValue, s_szIniFile);
		break;

	case 4:		///> (04) 예매 1건 자동발매 (Y/N)
		pValue = s_tKioskOpt.mrnp_t.mrnp_1_auto_issue;
		*pValue = nValue & 0xFF;
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_1_AUTO_ISSUE, pValue, s_szIniFile);
		break;

	case 6:		///> (06) 예매 왕복발권 사용유무 (Y/N)
		pValue = s_tKioskOpt.mrnp_t.mrnp_rtrp_yn;
		*pValue = nValue & 0xFF;
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_RTRP_YN, pValue, s_szIniFile);
		break;

	case 7:		///> (07) 예매 비배차 비좌석노선 사용유무 (Y/N)
		pValue = s_tKioskOpt.mrnp_t.mrnp_rtrp_yn;
		*pValue = nValue & 0xFF;
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_NALCN_NSAT_YN, pValue, s_szIniFile);
		break;

	case 8:		///> (08) 예매 발권 제한 시간 (분)
		s_tKioskOpt.mrnp_t.mrnp_limit_time = nValue;
		sprintf(s_Buffer, "%d", nValue);
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_LIMIT_TIME, s_Buffer, s_szIniFile);
		break;

	case 9:
		///> (09) 예매 당일 예약내역 모두 발권 가능여부 (YN)
		pValue = s_tKioskOpt.mrnp_t.mrnp_today_all_issue_yn;
		*pValue = nValue & 0xFF;
		SetProfString(SECTION_NM_MRNP_INFO, KEY_NM_MRNP_TODAY_ALL_ISSUE_YN, pValue, s_szIniFile);
		break;
	}
}

/**
 * @brief		INI_GetOptionData
 * @details		KIOSK_OPTION 데이타 가져오기
 * @param		int nKind	: 데이타 종류
 * @return		OPTION Data
 */
char *INI_GetOptionData(int nKind)
{
	switch(nKind)
	{
	case enOPT_GRP_ALL_DATA :
		return (char *) &s_tKioskOpt;
	case enOPT_GRP_CFG_DATA :
		return (char *) &s_tKioskOpt.cfg_t;
	case enOPT_GRP_LIMIT_DATA :
		return (char *) &s_tKioskOpt.limit_t;
	case enOPT_GRP_PAY_DATA :
		return (char *) &s_tKioskOpt.pay_t;
	case enOPT_GRP_MRNP_DATA :
		return (char *) &s_tKioskOpt.mrnp_t;
	case enOPT_GRP_PBTCK_DATA :
		return (char *) &s_tKioskOpt.pbtck_t;
	case enOPT_GRP_REFUND_DATA :
		return (char *) &s_tKioskOpt.refund_t;
	case enOPT_GRP_TICKET_DATA :
		return (char *) &s_tKioskOpt.tck_opt_t;
	}

	return (char *) NULL;
}

