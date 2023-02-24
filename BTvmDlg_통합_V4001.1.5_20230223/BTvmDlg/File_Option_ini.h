// File_Option_ini.h : 시외/고속 무인기 옵션 Ini 파일 처리
// 
// 
//

#pragma once

//----------------------------------------------------------------------------------------------------------------------

enum __en_OptionKind__
{
	enOPT_GRP_ALL_DATA	= 1		,
	enOPT_GRP_CFG_DATA			,
	enOPT_GRP_LIMIT_DATA		,
	enOPT_GRP_PAY_DATA			,
	enOPT_GRP_MRNP_DATA			,
	enOPT_GRP_PBTCK_DATA		,
	enOPT_GRP_REFUND_DATA		,
	enOPT_GRP_TICKET_DATA		,
};

//----------------------------------------------------------------------------------------------------------------------

///> section name

#define SECTION_NM_CONFIG_INFO					"CONFIG_INFO"									///> 
#define SECTION_NM_LIMIT_INFO					"LIMIT_INFO"									///> 
#define SECTION_NM_PAY_INFO						"PAY_INFO"										///> 
#define SECTION_NM_MRNP_INFO					"MRNP_INFO"										///> 
#define SECTION_NM_PBTCK_INFO					"PBTCK_INFO"									///> 
#define SECTION_NM_REFUND_INFO					"REFUND_INFO"									///> 
#define SECTION_NM_TICKET_INFO					"TICKET_INFO"									///> 


//----------------------------------------------------------------------------------------------------------------------

///> key name

///> "CONFIG_INFO"

#define KEY_NM_CFG_MAINT_LCD_YN					"CFG_MAINT_LCD_YN"								///> 관리자 모니터 사용유무 (Y/N)
#define KEY_NM_CFG_AUTO_CLOSE_YN				"CFG_AUTO_CLOSE_YN"								///> 
#define KEY_NM_CFG_AUTO_CLOSE_TIME				"CFG_AUTO_CLOSE_TIME"							///> 
#define KEY_NM_CFG_MON_SVR_YN					"CFG_MON_SVR_YN"								///> 
#define KEY_NM_CFG_MON_SVR_IP					"CFG_MON_SVR_IP"								///> 
#define KEY_NM_CFG_CAM_1_YN						"CFG_CAM_1_YN"									///> 
#define KEY_NM_CFG_CAM_2_YN						"CFG_CAM_2_YN"									///> 
#define KEY_NM_CFG_CAM_3_YN						"CFG_CAM_3_YN"									///> 
#define KEY_NM_CFG_SCREEN_WAIT_TIME				"CFG_SCREEN_WAIT_TIME"							///> 
#define KEY_NM_CFG_NOTI_WAIT_TIME				"CFG_NOTI_WAIT_TIME"							///> 
#define KEY_NM_CFG_OPEND_TRML_YN				"CFG_OPEND_TRML_YN"								///> 
#define KEY_NM_CFG_TRML_OP_YN					"CFG_TRML_OP_YN"								///> 
#define KEY_NM_CFG_TRML_BEGIN_TIME				"CFG_TRML_BEGIN_TIME"							///> 
#define KEY_NM_CFG_TRML_END_TIME				"CFG_TRML_END_TIME"								///> 
#define KEY_NM_CFG_OPEND_LCD_OFF				"CFG_OPEND_LCD_OFF"								///> 
#define KEY_NM_CFG_IC_READER_INS_YN				"CFG_IC_READER_INS_YN"							///> 
#define KEY_NM_CFG_MULTI_LANG_YN				"CFG_MULTI_LANG_YN"								///> 
#define KEY_NM_CFG_SOUND_YN						"CFG_SOUND_YN"									///> 
#define KEY_NM_CFG_MAIN_SOUND_TIME				"CFG_MAIN_SOUND_TIME"							///> 
#define KEY_NM_CFG_BNCL_YN						"CFG_BNCL_YN"									///> 
#define KEY_NM_CFG_SOLD_OUT_YN					"CFG_SOLD_OUT_YN"								///> 
#define KEY_NM_CFG_ALCN_COLOR_YN				"CFG_ALCN_COLOR_YN"								///> 
#define KEY_NM_CFG_DISC_BTN_YN					"CFG_DISC_BTN_YN"								///> 
#define KEY_NM_CFG_ROT_SEARCH_YN				"CFG_ROT_SEARCH_YN"								///> 
#define KEY_NM_CFG_INCHEON_AIR_POPUP_YN 		"CFG_INCHEON_AIR_POPUP_YN"						///> 

///> "LIMIT_INFO"

#define KEY_NM_LMT_TCK_MIN_CNT					"LMT_TCK_MIN_CNT"								///> 
#define KEY_NM_LMT_1K_MIN_CNT					"LMT_1K_MIN_CNT"								///> 
#define KEY_NM_LMT_10K_MIN_CNT					"LMT_10K_MIN_CNT"								///> 
#define KEY_NM_LMT_100_MIN_CNT					"LMT_100_MIN_CNT"								///> 
#define KEY_NM_LMT_500_MIN_CNT					"LMT_500_MIN_CNT"								///> 
#define KEY_NM_LMT_BILL_BOX_MAX_CNT				"LMT_BILL_BOX_MAX_CNT"							///> 
#define KEY_NM_LMT_TCK_BOX_MAX_CNT				"LMT_TCK_BOX_MAX_CNT"							///> 

///> "PAY_INFO"

#define KEY_NM_PAY_WAY							"PAY_WAY"										///> 
#define KEY_NM_PAY_CSRC_YN						"PAY_CSRC_YN"									///> 
#define KEY_NM_PAY_SIGN_PAD_YN					"PAY_SIGN_PAD_YN"								///> 
#define KEY_NM_PAY_PASSWD_YN					"PAY_PASSWD_YN"									///> 

///> "MRNP_INFO"

#define KEY_NM_MRNP_USE_YN						"MRNP_USE_YN"									///> 
#define KEY_NM_MRNP_ALL_LIST_YN					"MRNP_ALL_LIST_YN"								///> 
#define KEY_NM_MRNP_MANUAL_YN					"MRNP_MANUAL_YN"								///> 
#define KEY_NM_MRNP_1_AUTO_ISSUE				"MRNP_1_AUTO_ISSUE"								///> 
//#define KEY_NM_MRNP_ISSUE_LIMIT_DAY				"MRNP_ISSUE_LIMIT_DAY"							///> 
#define KEY_NM_MRNP_RTRP_YN						"MRNP_RTRP_YN"									///> 
#define KEY_NM_MRNP_NALCN_NSAT_YN				"MRNP_NALCN_NSAT_YN"							///> 
#define KEY_NM_MRNP_LIMIT_TIME					"MRNP_LIMIT_TIME"								///> 
#define KEY_NM_MRNP_TODAY_ALL_ISSUE_YN			"MRNP_TODAY_ALL_ISSUE_YN"						///> 

///> "PBTCK_INFO"

#define KEY_NM_PBTCK_USE_YN						"PBTCK_USE_YN"									///> 
#define KEY_NM_PBTCK_QUICK_YN					"PBTCK_QUICK_YN"								///> 
#define KEY_NM_PBTCK_ALCN_WAY					"PBTCK_ALCN_WAY"								///> 
#define KEY_NM_PBTCK_FAVORIT_YN					"PBTCK_FAVORIT_YN"								///> 
#define KEY_NM_PBTCK_FAVORIT_CNT				"PBTCK_FAVORIT_CNT"								///> 
#define KEY_NM_PBTCK_MAX_ISSUE_CNT				"PBTCK_MAX_ISSUE_CNT"							///> 
#define KEY_NM_PBTCK_MAX_ISSUE_FARE				"PBTCK_MAX_ISSUE_FARE"							///> 
#define KEY_NM_PBTCK_MAX_ISSUE_TIME				"PBTCK_MAX_ISSUE_TIME"							///> 
#define KEY_NM_PBTCK_CCS_RTRP_YN				"PBTCK_CCS_RTRP_YN"									///> 
#define KEY_NM_PBTCK_EXP_RTRP_YN				"PBTCK_EXP_RTRP_YN"									///> 
#define KEY_NM_PBTCK_CCEXP_ROT_YN				"PBTCK_CCEXP_ROT_YN"							///> 
#define KEY_NM_PBTCK_NALCN_1_YN					"PBTCK_NALCN_1_YN"								///> 
#define KEY_NM_PBTCK_NSAT_ISS_YN				"PBTCK_NSAT_ISS_YN"								///> 
#define KEY_NM_PBTCK_REISSUE_YN					"PBTCK_REISSUE_YN"								///> 

#define KEY_NM_PBTCK_ISSUE_LIMIT_DAY			"PBTCK_ISSUE_LIMIT_DAY"							///> 

///> "REFUND_INFO"

#define KEY_NM_REFUND_USE_YN					"REFUND_USE_YN"									///> 

///> "TICKET_INFO"

#define KEY_NM_TCK_FORMAT						"TCK_FORMAT"									///> 
#define KEY_NM_TCK_DEVICE_TYPE					"TCK_DEVICE_TYPE"								///> 
#define KEY_NM_TCK_TRML_NAME_LEN				"TCK_TRML_NAME_LEN"								///> 
#define KEY_NM_TCK_AGENT_INFO_VAL				"TCK_AGENT_INFO_VAL"							///> 
#define KEY_NM_TCK_BARCODE_HEIGHT				"TCK_BARCODE_HEIGHT"							///> 
#define KEY_NM_TCK_PRINT_NAME					"TCK_PRINT_NAME"								///> 
#define KEY_NM_TCK_PRINT_CONCENT_VAL			"TCK_PRINT_CONCENT_VAL"							///> 
#define KEY_NM_TCK_ERR_CHNG_ISSUE_YN			"TCK_ERR_CHNG_ISSUE_YN"							///> 
#define KEY_NM_TCK_REFUND_MARK_YN				"TCK_REFUND_MARK_YN"							///> 
#define KEY_NM_TCK_INHR_PRINT_YN				"TCK_INHR_PRINT_YN"								///> 
#define KEY_NM_TCK_NALCN_DEPR_DAY_PRINT_YN		"TCK_NALCN_DEPR_DAY_PRINT_YN"					///> 
#define KEY_NM_TCK_NSAT_DEPR_DAY_PRINT_YN		"TCK_NSAT_DEPR_DAY_PRINT_YN"					///> 
#define KEY_NM_TCK_NSAT_TIME_PRINT_YN			"TCK_NSAT_TIME_PRINT_YN"						///> 
#define KEY_NM_TCK_NALCN_NSAT_NOTI_RPINT_YN		"TCK_NALCN_NSAT_NOTI_RPINT_YN"					///> 
#define KEY_NM_TCK_NALCN_CACM_PRINT_YN			"TCK_NALCN_CACM_PRINT_YN"						///> 
#define KEY_NM_TCK_DEPR_DAY_6_PRINT_YN			"TCK_DEPR_DAY_6_PRINT_YN"						///> 
#define KEY_NM_TCK_DEPR_WEEK_PRINT_YN			"TCK_DEPR_WEEK_PRINT_YN"						///> 
#define KEY_NM_TCK_CACM_PRINT_YN				"TCK_CACM_PRINT_YN"								///> 
#define KEY_NM_TCK_YOU_CACM_NUMBER_PRINT_YN		"TCK_YOU_CACM_NUMBER_PRINT_YN"					///> 
#define KEY_NM_TCK_MY_CACM_NUMBER_PRINT_YN		"TCK_MY_CACM_NUMBER_PRINT_YN"					///> 
#define KEY_NM_TCK_1_DIMENSION_PRINT_YN			"TCK_1_DIMENSION_PRINT_YN"						///> 
#define KEY_NM_TCK_2_DIMENSION_PRINT_YN			"TCK_2_DIMENSION_PRINT_YN"						///> 
#define KEY_NM_TCK_DEPR_TRML_PRINT_YN			"TCK_DEPR_TRML_PRINT_YN"						///> 
#define KEY_NM_TCK_MSG_PRINT_YN					"TCK_MSG_PRINT_YN"								///> 


//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

typedef struct __tag_kiosk_cfg_t__
{
	char	maint_lcd_yn	[1+1];				///< 관리자 모니터 사용유무 (Y/N)
	char	auto_close_yn	[1+1];				///< 자동 창구마감 사용유무 (Y/N)
	char	auto_close_time	[6+1];				///< 자동 창구마감 시간 (시/분/초)
	char	sms_yn			[1+1];				///< 관제서버 사용유무 (Y/N)
	char	sms_ip			[15+1];				///< 관제서버 IP 
	char	cam_1_yn		[1+1];				///< 카메라 1번 사용유무 (Y/N)
	char	cam_2_yn		[1+1];				///< 카메라 2번 사용유무 (Y/N)
	char	cam_3_yn		[1+1];				///< 카메라 3번 사용유무 (Y/N)
	int 	screen_wait_time;					///< 화면 대기 시간 (초)
	int 	noti_wait_time;						///< 알림창 대기 시간 (초)
	char	opend_trml_yn	[1+1];				///< 당일 도착지 운행종료 표시 여부 (Y/N)
	char	trml_op_yn		[1+1];       		///< 판매 사용 여부 (Y/N)
	char	trml_begin_time	[6+1];				///< 판매시작시간
	char	trml_end_time	[6+1];				///< 판매종료시간
	char	opend_lcd_off	[1+1];				///< 판매종료 시, LCD OFF 유무 (Y:LCD OFF, N:LCD ON)
	char	ic_reader_ins_yn[1+1];				///< IC 카드리더기 삽입여부 (YN)
	char	multi_lang_yn	[1+1];				///< 다국어 사용유무 (YN)
	char	sound_yn		[1+1];				///< 안내음성 사용유무 (YN)
	int		main_sound_time;					///< 메인화면 안내음성 반복 시간(초)
	char	bncl_yn			[1+1];				///< 결행배차 표기 유무 (YN)
	char	sold_out_yn		[1+1];				///< 매진배차 표기 유무 (YN)
	char	alcn_color_yn	[1+1];				///< 배차상태 색깔 표시 유무 (YN)
	char	disc_btn_yn		[1+1];				///< 선택할인 버튼 변경 유무 (YN)
	char	rot_search_yn	[1+1];				///< 노선 검색 사용유무 (YN)
	char	incheon_air_popup_yn [1+1]; 		///< 인천공항 알림 팝업 사용여부 (YN)
} KIOSK_CFG_OPT_T, *PKIOSK_CFG_OPT_T;

typedef struct __tag_kiosk_limitt_opt_t__
{
	int 	n_tck_min_cnt;						///< 승차권 최소 수량
	int 	n_1k_min_cnt;						///< 1천원권 잔여 최소 수량
	int 	n_10k_min_cnt;						///< 1만원권 잔여 최소 수량
	int 	n_100_min_cnt;						///< 100원 잔여 최소 수량
	int 	n_500_min_cnt;						///< 500원 잔여 최소 수량
	int 	n_bill_box_max_cnt;					///< 지폐수집함 최대 수량
	int 	n_tck_box_max_cnt;					///< 승차권 수집함 최대 수량

} KIOSK_LIMITT_OPT_T, *PKIOSK_LIMITT_OPT_T;

typedef struct __tag_kiosk_pay_opt_t__
{
	char	pay_way				[1+1]; 			///< 카드('1'), 현금('2'), 카드+현금('3')
	char	pay_csrc_yn			[1+1]; 			///< 현금영수증 사용유무 (Y/N)
	char	pay_sign_pad_yn		[1+1]; 			///< 5만원 미만 무서명 (무서명:Y, 서명:N)
	char	pay_passwd_yn		[1+1]; 			///< 카드 비밀번호 사용유무 (Y/N)
	char	pay_installment_yn	[1+1]; 			///< 카드 할부 사용유무 (Y/N)

} KIOSK_PAY_OPT_T, *PKIOSK_PAY_OPT_T;

typedef struct __tag_kiosk_mrnp_opt_t__
{
	char	mrnp_use_yn				[1+1];		///< 예매 기능 사용유무 (Y/N)
	char	mrnp_all_list_yn		[1+1]; 		///< 예매 전제내역 보기 (Y/N)
	char	mrnp_manual_yn			[1+1]; 		///< 예매 수기 조회 (Y/N)
	char	mrnp_1_auto_issue		[1+1]; 		///< 예매 1건 자동발매 (Y/N)
	char	mrnp_rtrp_yn			[1+1]; 		///< 예매 왕복발권 사용유무 (Y/N) 
	char	mrnp_nalcn_nsat_yn		[1+1]; 		///< 예매 비배차 비좌석노선 사용유무 (Y/N) 
	int		mrnp_limit_time;					///< 예매 발권 제한 시간 (분)
	char	mrnp_today_all_issue_yn [1+1]; 		///< 예매 당일 예약내역 모두 발권 가능여부 (YN)

} KIOSK_MRNP_OPT_T, *PKIOSK_MRNP_OPT_T;

typedef struct __tag_kiosk_pbtck_opt_t__
{
	char	pbtck_use_yn		[1+1]; 			///< 현장발권 기능 사용유무 (Y/N)
	char	pbtck_quick_yn		[1+1]; 			///< 빠른배차 조회 사용유무 (Y/N)
	char	pbtck_alcn_way		[1+1]; 			///< 배차('D'), 비배차('N')
	char	pbtck_favorit_yn	[1+1]; 			///< 즐겨찾기 사용유무 (Y/N)
	char	pbtck_ccs_rtrp_yn	[1+1]; 			///< [시외] 왕복발권 사용유무 (Y/N) 
	char	pbtck_exp_rtrp_yn	[1+1]; 			///< [고속] 왕복발권 사용유무 (Y/N) 
	char	pbtck_ccexp_rot_yn	[1+1]; 			///< 고속노선 판매(시외고속) 사용유무 (Y/N) 
	char	pbtck_nalcn_1_yn	[1+1]; 			///< 비배차 1건시 자동선택 유무 (Y/N) 
	char	pbtck_nsat_iss_yn	[1+1]; 			///< 비좌석 노선 무표발권 사용유무 (Y/N) 
	char	pbtck_reissue_yn	[1+1]; 			///< 재발행 사용유무 (Y/N) 

	int 	pbtck_favorit_cnt; 					///< 즐겨찾기 터미널 갯수
	int 	pbtck_max_issue_cnt; 				///< 1회 발권 제한 수량
	int 	pbtck_max_issue_fare; 				///< 1회 발권 제한 금액(현금)
	int 	pbtck_max_issue_time; 				///< 발권 제한 시간(분)
	int 	pbtck_issue_limit_day;				///< 예매 발권 제한 일자 (일)

} KIOSK_PBTCK_OPT_T, *PKIOSK_PBTCK_OPT_T;

typedef struct __tag_kiosk_refund_opt_t__
{
	char	refund_use_yn [1+1]; 				///< 환불 기능 사용유무 (Y/N)

} KIOSK_REFUND_OPT_T, *PKIOSK_REFUND_OPT_T;

typedef struct __tag_kiosk_tck_opt_t__
{
	char	tck_format					[10+1]; ///< 승차권 프린터 용지 포맷
	char	tck_device_type				[1+1];	///< 승차권프린터('1'), 영수증프린터('2')
	int 	tck_trml_name_len;					///< 승차권 터미널명 기준 글자 수 (기본:4)
	int 	tck_agent_info_val;					///< NO:0, ID:1, ID(이름):2, 이름:3
	int 	tck_barcode_height;					///< 바코드 높이(40 ~ 100) 기본:50
	char	tck_print_name				[10+1]; ///< 승차권 프린터 모델
	int 	tck_print_concent_val; 				///< 승차권 프린트 농도 값, 기본(-2)
	char	tck_err_chng_issue_yn		[1+1]; 	///< 승차권 출력 에러시, 교체발권 사용유무 (YN)
	char	tck_refund_mark_yn			[1+1]; 	///< 승차권 환불 시, 소인유무 (YN)
	char	tck_inhr_print_yn			[1+1]; 	///< 승차권 고유번호 출력 유무 (YN)
	char	tck_nalcn_depr_day_print_yn [1+1]; 	///< 비배차 승차권 출발일 출력 유무 (YN)
	char	tck_nsat_depr_day_print_yn	[1+1]; 	///< 비좌석제 승차권 출발일 출력 유무 (YN)
	char	tck_nsat_time_print_yn		[1+1]; 	///< 비좌석제 승차권 시간 출력 유무 (YN)
	char	tck_nalcn_nsat_noti_rpint_yn[1+1]; 	///< 비배차, 비좌석노선 선착 선탑승 안내문구 출력 유무 (YN)
	char	tck_nalcn_cacm_print_yn		[1+1]; 	///< 비배차 목록 시간, 운수사 표시 유무 (YN)
	char	tck_depr_day_6_print_yn		[1+1]; 	///< 출발일 6글자 출력 유무 (YN)
	char	tck_depr_week_print_yn		[1+1]; 	///< 출발일 요일 출력 유무 (YN)
	char	tck_cacm_print_yn			[1+1]; 	///< 운수사 출력 유무 (YN)
	char	tck_you_cacm_number_print_yn[1+1]; 	///< (승객용) 운수사 사업자 번호 출력 유무 (YN)
	char	tck_my_cacm_number_print_yn [1+1]; 	///< (회수용) 운수사 사업자 번호 출력 유무 (YN)
	char	tck_1_dimension_print_yn	[1+1]; 	///< 1차원 바코드 출력 유무 (YN)
	char	tck_2_dimension_print_yn	[1+1]; 	///< 2차원 바코드 출력 유무 (YN)
	char	tck_depr_trml_print_yn		[1+1]; 	///< 출발터미널 출력 유무 (YN)
	char	tck_msg_print_yn			[1+1]; 	///< 발권 메시지 출력 유무 (YN)
} KIOSK_TCK_OPT_T, *PKIOSK_TCK_OPT_T;

typedef struct __tag_kiosk_ini_opt_t__
{
	KIOSK_CFG_OPT_T		cfg_t;
	KIOSK_LIMITT_OPT_T	limit_t;
	KIOSK_PAY_OPT_T		pay_t;
	KIOSK_MRNP_OPT_T	mrnp_t;
	KIOSK_PBTCK_OPT_T	pbtck_t;
	KIOSK_REFUND_OPT_T	refund_t;
	KIOSK_TCK_OPT_T		tck_opt_t;

} KIOSK_INI_OPT_T, *PKIOSK_INI_OPT_T;

#pragma pack()

//----------------------------------------------------------------------------------------------------------------------

int INI_CtrlOptConfig(BOOL bWrite, char *pWrData);
int INI_CtrlOptLimitInfo(BOOL bWrite, char *pWrData);
int INI_CtrlOptPayInfo(BOOL bWrite, char *pWrData);
int INI_CtrlOptMrnpInfo(BOOL bWrite, char *pWrData);
int INI_CtrlOptPbTckInfo(BOOL bWrite, char *pWrData);
int INI_CtrlOptRefundInfo(BOOL bWrite, char *pWrData);
int INI_CtrlOptTicketInfo(BOOL bWrite, char *pWrData);

int INI_ReadOptionFile(void);
int INI_WriteOptionFile(char *pWrData);
void INI_SetOptData(int nIndex, int nValue, char *pData);
char *INI_GetOptionData(int nKind);

