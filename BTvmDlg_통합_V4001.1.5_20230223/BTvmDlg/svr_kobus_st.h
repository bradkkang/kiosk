// 
// 
// svr_kobus_st.h : 고속버스(코버스) 서버 패킷 구조체 헤더 파일
//

#pragma once



//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

//------------------------------
// 전송 헤더 : CM_CDINQR

typedef struct
{
	char req_pgm_dvs		[3+1]	;	///< 요청 프로그램 구분
	char req_trml_cd		[3+1]	;	///< 요청 터미널 코드
	char req_wnd_no			[2+1]	;	///< 요청 창구번호
	char req_user_no		[6+1]	;	///< 요청 사용자번호
} stk_exp_head_t, *pstk_exp_head_t;

// ------------------------------
// 터미널 조회 : TM_ETRMLINFO
// 
// typedef struct
// {
// 	char user_no			[6+1]	;	///< 사용자번호
// 	char lng_cd				[2+1]	;	///< 언어코드
// 	char depr_trml_no		[3+1]	;	///< 출발터미널번호
// 
// } stk_tm_etrmlinfo_t, *pstk_tm_etrmlinfo_t;
// 
// typedef struct
// {
// 	char trml_no			[3+1]	;	///< 터미널번호_arr
// 	char trml_nm			[100+1]	;	///< 터미널명_arr
// 	char trml_abrv_nm		[100+1]	;	///< 터미널약칭명_arr
// 	char trml_dtl_addr		[100+1]	;	///< 터미널주소_arr
// 	char trml_eng_nm		[100+1]	;	///< 터미널영문명_arr
// 	char trml_eng_abrv_nm	[100+1]	;	///< 터미널영문약칭명_arr
// 	char trtr_trml_yn		[1+1]	;	///< 환승터미널여부_arr
// 	char sys_dvs_cd			[1+1]	;	///< 시스템구분코드_arr
// 	char tel_no				[20+1]	;	///< 전화번호_arr
// 
// } rtk_tm_etrmlinfo_list_t, *prtk_tm_etrmlinfo_list_t;
// 
// typedef struct
// {
// 	char msg_cd				[6+1]	;	///< 메시지 코드
// 	char msg_dtl_ctt		[200+1]	;	///< 메시지 명
// 	char rec_ncnt1			[4+1]	;	///< 	
// 	rtk_tm_etrmlinfo_list_t* pList;	
// } rtk_tm_etrmlinfo_t, *prtk_tm_etrmlinfo_t;

//------------------------------
// 공통코드 상세 조회 : CM_CDINQR

typedef struct
{
	char user_no			[6+1]	;	///< 요청 사용자번호
	char lng_cd				[2+1]	;	///< 언어코드
} stk_cm_cdinqr_t, *pstk_cm_cdinqr_t;

typedef struct
{
	char lng_cd				[2+1]	;	///< 언어코드
	char cmn_cd				[4+1]	;	///< 공통코드
	char cmn_cd_val			[20+1]	;	///< 공통코드 값
	char cd_val_mark_seq	[5+1]	;	///< 코드값 표시순서
	char cd_val_nm			[100+1]	;	///< 코드값 명
	char bsc_rfrn_val		[100+1]	;	///< 기본참조 값
	char adtn_rfrn_val		[100+1]	;	///< 부가참조 값
} rtk_cm_cdinqr_list_t, *prtk_cm_cdinqr_list_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지 코드
	char msg_dtl_ctt		[200+1]	;	///< 메시지 명
	char rec_ncnt1			[4+1]	;	///< 	
	rtk_cm_cdinqr_list_t* pList;	
} rtk_cm_cdinqr_t, *prtk_cm_cdinqr_t;

//------------------------------
// 승차홈 조회 : CM_RDHMINQR

typedef struct
{
	char user_no			[6+1]	;	///< 언어구분
	char lng_cd				[2+1]	;	///< 사용자NO
} stk_cm_rdhminqr_t, *pstk_cm_rdhminqr_t;

typedef struct
{
	char depr_trml_no		[3+1]	;	///< 출발지 터미널코드
	char arvl_trml_no		[3+1]	;	///< 도착지 터미널코드
	char bus_cls_cd			[3+1]	;	///< 버스등급코드
	char cacm_cd			[2+1]	;	///< 운수사코드
	char rot_rdhm_no		[100+1]	;	///< 승차홈
} rtk_cm_rdhminqr_list_t, *prtk_cm_rdhminqr_list_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지 코드
	char msg_dtl_ctt		[200+1]	;	///< 메시지 명
	char rec_ncnt1			[4+1]	;	///< 	
	rtk_cm_rdhminqr_list_t* pList;	
} rtk_cm_rdhminqr_t, *prtk_cm_rdhminqr_t;

//------------------------------
// 승차권 인쇄정보 조회  : TM_ETCKPTRGINFO

typedef struct
{
	char user_no			[6+1]	;	///< 언어구분
	char lng_cd				[2+1]	;	///< 사용자NO
	char trml_no			[3+1]	;	///< 터미널번호
	char papr_tck_dvs_cd	[3+1]	;	///< 종이티켓구분코드
	char ptr_knd_cd			[1+1]	;	///< 프린터종류코드
} stk_tm_etckptrginfo_t, *pstk_tm_etckptrginfo_t;

typedef struct
{
	char trml_no 			[3+1]	;	///< 터미널번호_arr 	
	char papr_tck_dvs_cd 	[3+1]	;	///< 종이티켓구분코드_arr
	char ptr_knd_cd 		[1+1]	;	///< 프린터종류코드_arr 	
	char sort_seq 			[4+1]	;	///< (int)정렬순서_arr 	
	char ptrg_usg_cd 		[1+1]	;	///< 인쇄용도코드_arr 	
	char ptrg_atc_nm 		[100+1]	;	///< 인쇄항목명_arr 	
	char x_crdn_val 		[100+1]	;	///< X좌표값_arr 		
	char y_crdn_val 		[100+1]	;	///< Y좌표값_arr 		
	char mgnf_val 			[100+1]	;	///< 배율값_arr 		
	char use_yn 			[1+1]	;	///< 사용여부_arr 	
} rtk_tm_etckptrginfo_list_t, *prtk_tm_etckptrginfo_list_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지 코드
	char msg_dtl_ctt		[200+1]	;	///< 메시지 명
	char rec_ncnt1			[4+1]	;	///< 	
	rtk_tm_etckptrginfo_list_t* pList;	
} rtk_tm_etckptrginfo_t, *prtk_tm_etckptrginfo_t;

//------------------------------
// 메시지 정보 조회  : CM_MSGINQR

typedef struct
{
	char user_no			[6+1]	;	///< 언어구분
	char lng_cd				[2+1]	;	///< 사용자NO
} stk_cm_msginqr_t, *pstk_cm_msginqr_t;

typedef struct
{
	char lng_cd 			[3+1]	;	///< 언어종류
	char msg_cd 			[6+1]	;	///< 메시지 코드
	char msg_dtl_ctt 		[500+1]	;	///< 메시지 명
} rtk_cm_msginqr_list_t, *prtk_cm_msginqr_list_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지 코드
	char msg_dtl_ctt		[200+1]	;	///< 메시지 명
	char rec_ncnt1			[4+1]	;	///< 	
	rtk_cm_msginqr_list_t* pList;	
} rtk_cm_msginqr_t, *prtk_cm_msginqr_t;

//------------------------------
// 운수사 정보 : CM_CACMINQR

typedef struct
{
	char user_no			[6+1]	;	///< 언어구분
	char lng_cd				[2+1]	;	///< 사용자NO
} stk_cm_cacminqr_t, *pstk_cm_cacminqr_t;

typedef struct
{
	char lng_cd 			[3+1]	;	///< 언어종류
	char trml_no 			[3+1]	;	///< 터미널번호
	char hspd_cty_dvs_cd 	[1+1]	;	///< 고속시외구분코드
	
	char cacm_cd 			[2+1]	;	///< 운수사 코드
	char cacm_nm 			[100+1]	;	///< 운수사 명
	char cacm_abrv_nm 		[100+1]	;	///< 운수사 약칭명
	char bizr_no 			[30+1]	;	///< 사업자번호

	char trml_by_cacm_cd 	[2+1]	;	///< 터미널별 운수사코드
	char scrn_prin_nm 		[100+1]	;	///< 화면출력 명
	char ptrg_prin_nm 		[100+1]	;	///< 인쇄출력 명
	char mod_sta_cd 		[1+1]	;	///< 변경상태 코드
} rtk_cm_cacminqr_list_t, *prtk_cm_cacminqr_list_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지 코드
	char msg_dtl_ctt		[200+1]	;	///< 메시지 명
	char rec_ncnt1			[4+1]	;	///< 갯수	
	rtk_cm_cacminqr_list_t* pList;	
} rtk_cm_cacminqr_t, *prtk_cm_cacminqr_t;

//------------------------------
// 서버 시간 동기화 : CM_SVCTIMEINQR

typedef struct
{
	char user_no			[6+1]	;	///< 언어구분
	char lng_cd				[2+1]	;	///< 사용자NO
} stk_cm_svctimeinqr_t, *pstk_cm_svctimeinqr_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지 코드
	char msg_dtl_ctt		[200+1]	;	///< 메시지 명
	char prs_time			[14+1]	;	///< 서버일시(날짜/시간)

} rtk_cm_svctimeinqr_t, *prtk_cm_svctimeinqr_t;

//------------------------------
// 터미널 정보 조회 : TM_ETRMLINFO

typedef struct
{
	char user_no			[6+1]	;	///< 사용자NO
	char lng_cd				[2+1]	;	///< 언어구분
	char depr_trml_no		[3+1]	;	///< 출발 터미널번호
} stk_tm_etrmlinfo_t, *pstk_tm_etrmlinfo_t;

typedef struct
{
	char trml_no			[3+1]	;	///< 터미널번호_arr		 	
	char trml_nm			[100+1]	;	///< 터미널명_arr		 	
	char trml_abrv_nm		[100+1]	;	///< 터미널약칭명_arr		 	
	char trml_dtl_addr		[100+1]	;	///< 터미널주소_arr		 	
	char trml_eng_nm		[100+1]	;	///< 터미널영문명_arr		 	
	char trml_eng_abrv_nm	[100+1]	;	///< 터미널영문약칭명_arr	 	
	char trtr_trml_yn		[1+1]	;	///< 환승터미널여부_arr	 	
	char sys_dvs_cd			[1+1]	;	///< 시스템구분코드_arr	 	
	char tel_no				[20+1]	;	///< 전화번호_arr		 	
} rtk_tm_etrmlinfo_list_t, *prtk_tm_etrmlinfo_list_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지 코드
	char msg_dtl_ctt		[200+1]	;	///< 메시지 명
	char rec_ncnt1			[4+1]	;	///< 갯수	
	rtk_tm_etrmlinfo_list_t* pList;	
} rtk_tm_etrmlinfo_t, *prtk_tm_etrmlinfo_t;

//------------------------------
// 왕복 가능 터미널 조회 : TM_ERTRPTRMLINF

typedef struct
{
	char user_no			[6+1]	;	///< 언어구분
	char lng_cd				[2+1]	;	///< 사용자NO
	char depr_trml_no		[3+1]	;	///< 출발 터미널번호
} stk_tm_ertrptrmlinf_t, *pstk_tm_ertrptrmlinf_t;

typedef struct
{
	char arvl_trml_no 		[3+1]	;	///< 도착 터미널번호_arr 	
} rtk_tm_ertrptrmlinf_t_list_t, *prtk_tm_ertrptrmlinf_t_list_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지 코드
	char msg_dtl_ctt		[200+1]	;	///< 메시지 명
	char rec_ncnt1			[4+1]	;	///< 갯수	
	rtk_tm_ertrptrmlinf_t_list_t* pList;	
} rtk_tm_ertrptrmlinf_t, *prtk_tm_ertrptrmlinf_t;

//------------------------------
// [고속] 승차권 종류 정보 조회 : 공통코드 내용에서 발췌하여 데이타 생성

typedef struct
{
	char trml_no            [3+1]   ;   ///< 터미널번호
	char tck_knd_cd         [20+1]  ;   ///< 티켓종류코드
	char scrn_prin_seq      [5+1]   ;   ///< 화면출력순서
	char scrn_prin_nm       [100+1] ;   ///< 화면출력명
	char tck_nm_ko          [100+1] ;   ///< 한글
	char tck_nm_en          [100+1] ;   ///< 영어
	char tck_nm_ch          [100+1] ;   ///< 중국어
 	char tck_nm_ja          [100+1] ;   ///< 일본어
} rtk_cm_readtckknd_list_t, *prtk_cm_readtckknd_list_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지 코드
	char msg_dtl_ctt		[200+1]	;	///< 메시지 명
	char rec_ncnt1			[4+1]	;	///< 갯수	
} rtk_cm_readtckknd_t, *prtk_cm_readtckknd_t;

//------------------------------
// 창구 정보 조회 : TM_EWNDINFO

typedef struct
{
	char user_no			[6+1]	;	///< 언어구분
	char lng_cd				[2+1]	;	///< 사용자NO
	char trml_no			[3+1]	;	///< 터미널번호
	char wnd_no				[2+1]	;	///< 창구번호
} stk_tm_ewndinfo_t, *pstk_tm_ewndinfo_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지 코드
	char msg_dtl_ctt		[200+1]	;	///< 메시지 명
	char mrs_tissu_psb_dno	[4+1]	;	///< (int) 예매발권가능일수
	char tissu_ltn_drtm		[4+1]	;	///< (int) 발권제한시간

} rtk_tm_ewndinfo_t, *prtk_tm_ewndinfo_t;

//------------------------------
// 경유지 정보 조회 : TM_ETHRUINFO

typedef struct
{
	char user_no 			[6+1]	;	///< 사용자번호 	
	char lng_cd 			[2+1]	;	///< 언어코드 		
	char tissu_trml_no 		[3+1]	;	///< 발권터미널번호 	
	char depr_trml_no 		[3+1]	;	///< 출발터미널번호 	
	char arvl_trml_no 		[3+1]	;	///< 도착터미널번호 	
	char alcn_depr_trml_no 	[3+1]	;	///< 배차출발터미널번호
	char alcn_arvl_trml_no 	[3+1]	;	///< 배차도착터미널번호
	char alcn_rot_no 		[4+1]	;	///< 배차노선번호 	
} stk_tm_ethruinfo_t, *pstk_tm_ethruinfo_t;

typedef struct
{
	char tissu_trml_no 		[3+1]	;	///< 발권터미널번호_arr 	 
	char depr_trml_no 		[3+1]	;	///< 출발터미널번호_arr 	
	char arvl_trml_no 		[3+1]	;	///< 도착터미널번호_arr 	
	char alcn_depr_trml_no 	[3+1]	;	///< 배차출발터미널번호_arr
	char alcn_arvl_trml_no 	[3+1]	;	///< 배차도착터미널번호_arr
	char alcn_rot_no 		[4+1]	;	///< 배차노선번호_arr 	
	char stt_dt 			[8+1]	;	///< 시작일자_arr 		
	char end_dt 			[8+1]	;	///< 종료일자_arr 		
	char depr_thru_seq 		[4]		;	///< (int)출발경유순서_arr 	
	char arvl_thru_seq 		[4]		;	///< (int)도착경유순서_arr 	
	char thru_dist 			[4]		;	///< (int)경유거리_arr 		
	char thru_drtm 			[4]		;	///< (int)경유소요시간_arr 	

} rtk_tm_ethruinfo_list_t, *prtk_tm_ethruinfo_list_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지 코드
	char msg_dtl_ctt		[200+1]	;	///< 메시지 명
	char rec_ncnt1			[4+1]	;	///< 갯수	
	rtk_tm_ethruinfo_list_t* pList;
} rtk_tm_ethruinfo_t, *prtk_tm_ethruinfo_t;

//------------------------------
// 배차 정보 조회 : TM_TIMINFO

typedef struct
{
	char user_no 			[6+1]	;	///< 사용자번호 	
	char lng_cd 			[2+1]	;	///< 언어코드 		
	char depr_trml_no 		[3+1]	;	///< 출발터미널번호
	char arvl_trml_no 		[3+1]	;	///< 도착터미널번호
	char depr_dt      		[8+1]	;	///< 출발일자	
	char depr_time    		[6+1]	;	///< 출발시각	
	char bus_cls_cd   		[3+1]	;	///< 버스등급코드	
	char rec_ncnt	 		[4+1]	;	///< 조회 요청 건수

} stk_tm_timinfo_t, *pstk_tm_timinfo_t;

typedef struct
{
	char take_drtm					[4]	;	///< (int)소요시간
	char bus_oprn_dist				[20+1]	;	///< 거리					

	char prmm_adlt_fee     			[4]	;	///< 1(int)우등어른 요금			
	char prmm_teen_fee				[4]	;	///< (int)우등청소년 요금			
	char prmm_chld_fee     			[4]	;	///< (int)우등어린이 요금	
	char prmm_uvsd_fee     			[4]	;	///< (int)우등대학생 요금	

	char prmm_vtrn3_fee     		[4]	;	///< 2(int)우등보훈30 요금			
	char prmm_vtrn5_fee				[4]	;	///< (int)우등보훈50 요금			
	char prmm_vtrn7_fee     		[4]	;	///< (int)우등보훈70 요금	
	char prmm_army2_fee     		[4]	;	///< (int)우등군인20 요금	
	
	char hspd_adlt_fee     			[4]	;	///< 3(int)고속어른 요금			
	char hspd_teen_fee     			[4]	;	///< (int)고속청소년 요금			
	char hspd_chld_fee     			[4]	;	///< (int)고속어린이 요금			
	char hspd_uvsd_fee     			[4]	;	///< (int)고속대학생 요금		

	char hspd_vtrn3_fee     		[4]	;	///< 4(int)고속보훈30 요금			
	char hspd_vtrn5_fee     		[4]	;	///< (int)고속보훈50 요금			
	char hspd_vtrn7_fee     		[4]	;	///< (int)고속보훈70 요금			
	char hspd_army2_fee     		[4]	;	///< (int)고속군인20 요금		

	char mdnt_prmm_adlt_fee			[4]	;	///< 5(int)심우어른 요금			
	char mdnt_prmm_teen_fee			[4]	;	///< (int)심우청소년 요금			
	char mdnt_prmm_chld_fee			[4]	;	///< (int)심우어린이 요금		
	char mdnt_prmm_uvsd_fee			[4]	;	///< (int)심우대학생 요금		

	char mdnt_prmm_vtrn3_fee		[4]	;	///< 6(int)심우보훈30 요금			
	char mdnt_prmm_vtrn5_fee		[4]	;	///< (int)심우보훈50 요금			
	char mdnt_prmm_vtrn7_fee		[4]	;	///< (int)심우보훈70 요금		
	char mdnt_prmm_army2_fee		[4]	;	///< (int)심우군인20 요금		
	
	char mdnt_hspd_adlt_fee			[4]	;	///< 7(int)심고어른 요금			
	char mdnt_hspd_teen_fee			[4]	;	///< (int)심고청소년 요금			
	char mdnt_hspd_chld_fee			[4]	;	///< (int)심고어린이 요금			
	char mdnt_hspd_uvsd_fee			[4]	;	///< (int)심고대학생 요금	

	char mdnt_hspd_vtrn3_fee		[4]	;	///< 8(int)심고보훈30 요금			
	char mdnt_hspd_vtrn5_fee		[4]	;	///< (int)심고보훈50 요금			
	char mdnt_hspd_vtrn7_fee		[4]	;	///< (int)심고보훈70 요금			
	char mdnt_hspd_army2_fee		[4]	;	///< (int)심고군인20 요금	

	char mdnt_prmm_exch_adlt_fee	[4]	;	///< 9(int)심우할증 어른 요금 		
	char mdnt_prmm_exch_teen_fee	[4]	;	///< (int)심우할증 청소년 요금		
	char mdnt_prmm_exch_chld_fee	[4]	;	///< (int)심우할증 어린이 요금		
	char mdnt_prmm_exch_uvsd_fee	[4]	;	///< (int)심우할증 대학생 요금		

	char mdnt_prmm_exch_vtrn3_fee	[4]	;	///< 10(int)심우할증 보훈30 요금 		
	char mdnt_prmm_exch_vtrn5_fee	[4]	;	///< (int)심우할증 보훈50 요금		
	char mdnt_prmm_exch_vtrn7_fee	[4]	;	///< (int)심우할증 보훈70 요금		
	char mdnt_prmm_exch_army2_fee	[4]	;	///< (int)심우할증 군인20 요금		

	char mdnt_hspd_exch_adlt_fee	[4]	;	///< 11(int)심고할증 어른 요금			
	char mdnt_hspd_exch_teen_fee	[4]	;	///< (int)심고할증 청소년 요금		
	char mdnt_hspd_exch_chld_fee	[4]	;	///< (int)심고할증 아동 요금			
	char mdnt_hspd_exch_uvsd_fee	[4]	;	///< (int)심고할증 대학생 요금		

	char mdnt_hspd_exch_vtrn3_fee	[4]	;	///< 12(int)심고할증 보훈30 요금			
	char mdnt_hspd_exch_vtrn5_fee	[4]	;	///< (int)심고할증 보훈50 요금		
	char mdnt_hspd_exch_vtrn7_fee	[4]	;	///< (int)심고할증 보훈70 요금			
	char mdnt_hspd_exch_army2_fee	[4]	;	///< (int)심고할증 군인20 요금			
	
	char psrm_adlt_fee				[4]	;	///< 13(int)프리미엄 어른 요금 		
	char psrm_teen_fee				[4]	;	///< (int)프리미엄 청소년 요금		
	char psrm_chld_fee				[4]	;	///< (int)프리미엄 어린이 요금		
	char psrm_uvsd_fee				[4]	;	///< (int)프리미엄 대학생 요금		
	
	char psrm_vtrn3_fee				[4]	;	///< 14(int)프리미엄 보훈30 요금 		
	char psrm_vtrn5_fee				[4]	;	///< (int)프리미엄 보훈50 요금		
	char psrm_vtrn7_fee				[4]	;	///< (int)프리미엄 보훈70 요금		
	char psrm_army2_fee				[4]	;	///< (int)프리미엄 군인20 요금		
	
	char mdnt_psrm_adlt_fee			[4]	;	///< 15(int)심야프리미엄 어른 요금 		
	char mdnt_psrm_teen_fee			[4]	;	///< (int)심야프리미엄 청소년 요금		
	char mdnt_psrm_chld_fee			[4]	;	///< (int)심야프리미엄 어린이 요금		
	char mdnt_psrm_uvsd_fee			[4]	;	///< (int)심야프리미엄 대학생 요금		
	
	char mdnt_psrm_vtrn3_fee		[4]	;	///< 16(int)심야프리미엄 보훈30 요금 		
	char mdnt_psrm_vtrn5_fee		[4]	;	///< (int)심야프리미엄 보훈50 요금		
	char mdnt_psrm_vtrn7_fee		[4]	;	///< (int)심야프리미엄 보훈70 요금		
	char mdnt_psrm_army2_fee		[4]	;	///< (int)심야프리미엄 군인20 요금		
	
	char mdnt_psrm_exch_adlt_fee	[4]	;	///< 17(int)심야할증프리미엄 어른 요금 	
	char mdnt_psrm_exch_teen_fee	[4]	;	///< (int)심야할증프리미엄 청소년 요금	
	char mdnt_psrm_exch_chld_fee	[4]	;	///< (int)심야할증프리미엄 어린이 요금	
	char mdnt_psrm_exch_uvsd_fee	[4]	;	///< (int)심야할증프리미엄 대학생 요금	
	
	char mdnt_psrm_exch_vtrn3_fee	[4]	;	///< 18(int)심야할증프리미엄 보훈30 요금 	
	char mdnt_psrm_exch_vtrn5_fee	[4]	;	///< (int)심야할증프리미엄 보훈50 요금	
	char mdnt_psrm_exch_vtrn7_fee	[4]	;	///< (int)심야할증프리미엄 보훈70 요금	
	char mdnt_psrm_exch_army2_fee	[4]	;	///< (int)심야할증프리미엄 군인20 요금	
	
	char dc_fee						[4]	;	///< (int)시외우등뒷좌석할인요금	
	char cty_dc_fee3				[4]	;	///< (int)심야우등뒷좌석할인요금	

	char fn_yn						[1+1]	;	///< 마지막여부YN

} rtk_tm_timinfo_fee_t, *prtk_tm_timinfo_fee_t;

typedef struct
{
	char trtr_trml_incl_yn			[1+1];	///< 환승터미널포함여부_arr	
	char temp_rot_yn      			[1+1];	///< 임시노선유무_arr		
	char drtm_mod_psb_yn   			[1+1];	///< 시간변경가능유무_arr	
	char alcn_depr_trml_no 			[3+1];	///< 배차출발터미널번호_arr	
	char alcn_arvl_trml_no 			[3+1];	///< 배차도착터미널번호_arr	
	char depr_trml_no      			[3+1];	///< 출발터미널번호_arr	
	char arvl_trml_no      			[3+1];	///< 도착터미널번호_arr	
	char depr_dt           			[8+1];	///< 출발일자_arr		
	char depr_time         			[6+1];	///< 출발시각_arr		

	char alcn_depr_dt    			[8+1];	///< 배차출발일자_arr

	char alcn_depr_time    			[6+1];	///< 배차출발시각_arr		
	char bus_cls_cd        			[3+1];	///< 버스등급코드_arr		
	char cacm_cd           			[2+1];	///< 운수사코드_arr		
	char rmn_sats_num      			[4];	///< (int) 잔여좌석수_arr		
	char tot_sats_num      			[4];	///< (int) 총좌석수_arr		
	char cty_prmm_dc_yn 			[1+1];	///< 시외우등할인여부_arr	
	char depr_thru_seq				[4];	///< (int) 출발경유순서_arr		
	char arvl_thru_seq				[4];	///< (int) 도착경유순서_arr		
	char sats_mltp_val				[250+1];///< 좌석상태_arr		
	char dspr_sats_yn				[1+1];	///< 휠체어 좌석여부YN
	/// 2020.05.18 필드 추가
	char alcn_rot_no				[4+1];	///< 배차노선번호 
	/// ~2020.05.18 필드 추가
} rtk_tm_timinfo_list_t, *prtk_tm_timinfo_list_t;

typedef struct
{
	char msg_cd						[6+1]	;	///< 메시지코드				
	char msg_dtl_ctt				[200+1]	;	///< 메시지명				
	
	rtk_tm_timinfo_fee_t	fee_t;

	char rec_ncnt1					[4+1]	;	///< 갯수	
	rtk_tm_timinfo_list_t*	pList			;
} rtk_tm_timinfo_t, *prtk_tm_timinfo_t;

//------------------------------
// 좌석 정보 조회 : CM_SETINFO

typedef struct
{
	char user_no 			[6+1]	;	///< 사용자번호 	
	char lng_cd 			[2+1]	;	///< 언어코드 		
	char alcn_depr_trml_no 	[3+1]	;	///< 배차 출발터미널번호
	char alcn_arvl_trml_no 	[3+1]	;	///< 배차 도착터미널번호
	char alcn_depr_dt      	[8+1]	;	///< 배차 출발일자	
	char alcn_depr_time    	[6+1]	;	///< 배차 출발시각	
	char depr_trml_no 		[3+1]	;	///< 출발터미널번호
	char arvl_trml_no 		[3+1]	;	///< 도착터미널번호
	char depr_dt      		[8+1]	;	///< 출발일자	
	char depr_time    		[6+1]	;	///< 출발시각	
	char bus_cls_cd   		[3+1]	;	///< 버스등급코드	
	char cacm_cd   			[2+1]	;	///< 운수사코드	
	char trml_no 			[3+1]	;	///< 터미널번호

} stk_cm_setinfo_t, *pstk_cm_setinfo_t;

typedef struct
{
	char set_sta			[5+1]	;	///< 좌석할당값 : 좌석상태1 + 발권채널1 + 출발지3
} rtk_cm_setinfo_list_t, *prtk_cm_setinfo_list_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지코드				
	char msg_dtl_ctt		[200+1]	;	///< 메시지명				
	char rec_ncnt1	 		[4+1]	;	///< 조회 요청 건수
	rtk_cm_setinfo_list_t* pList;
} rtk_cm_setinfo_t, *prtk_cm_setinfo_t;

//------------------------------
// 좌석 선점 취소 : TW_SATSPCPYCANC

typedef struct
{
	char pcpy_no	 		[14+1]	;	///< 좌석 선점번호
} stk_tw_satspcpycanc_list_t, *pstk_tw_satspcpycanc_list_t;

typedef struct
{
	char user_no 			[6+1]	;	///< 사용자번호 	
	char lng_cd 			[2+1]	;	///< 언어코드 	
	char tissu_sta_cd		[1+1]	;	///< 발권상태(0.발권이전,1.발권이후)
	char rec_ncnt1	 		[4+1]	;	///< 조회 요청 건수
	stk_tw_satspcpycanc_list_t List[40];
} stk_tw_satspcpycanc_t, *pstk_tw_satspcpycanc_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지코드				
	char msg_dtl_ctt		[200+1]	;	///< 메시지명				
} rtk_tw_satspcpycanc_t, *prtk_tw_satspcpycanc_t;

//------------------------------
// 좌석 선점 : TW_SATSPCPY

typedef struct
{
	char sats_no	 		[2+1]	;	///< 좌석 번호
} stk_tw_satspcpy_list_t, *pstk_tw_satspcpy_list_t;

typedef struct
{
	char user_no 			[6+1]	;	///< 사용자번호 	
	char lng_cd 			[2+1]	;	///< 언어코드 	
	
	char tissu_chnl_dvs_cd	[1+1]	;	///< 발권채널구분코드	
	char trml_no			[3+1]	;	///< 터미널번호		
	char alcn_depr_trml_no	[3+1]	;	///< 배차출발터미널번호	
	char alcn_arvl_trml_no	[3+1]	;	///< 배차도착터미널번호	
	char depr_trml_no		[3+1]	;	///< 출발터미널번호	
	char arvl_trml_no		[3+1]	;	///< 도착터미널번호	
	char alcn_depr_dt		[8+1]	;	///< 배차출발일자		
	char alcn_depr_time		[6+1]	;	///< 배차출발시각		
	char depr_dt			[8+1]	;	///< 출발일자		
	char depr_time			[6+1]	;	///< 출발시각		
	char bus_cls_cd			[3+1]	;	///< 버스등급코드		
	char cacm_cd			[2+1]	;	///< 운수사코드		
	char depr_thru_seq		[4]		;	///< 출발경유순서		
	char arvl_thru_seq		[4]		;	///< 도착경유순서		
	char tot_sats_num		[4]		;	///< 총좌석수		
	char rec_ncnt	 		[4]		;	///< 조회 요청 건수
	stk_tw_satspcpy_list_t  List[40];
} stk_tw_satspcpy_t, *pstk_tw_satspcpy_t;

typedef struct
{
	char sats_no			[2+1]	;	///< 좌석번호_arr	
	char pcpy_no			[14+1]	;	///< 선점번호_arr	
} rtk_tw_satspcpy_list_t, *prtk_tw_satspcpy_list_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지코드				
	char msg_dtl_ctt		[200+1]	;	///< 메시지명				
	char rec_ncnt1	 		[4+1]	;	///< 조회 요청 건수
	rtk_tw_satspcpy_list_t* pList;
} rtk_tw_satspcpy_t, *prtk_tw_satspcpy_t;

//------------------------------
// 현장발권 (카드, 현금영수증) : 

typedef struct
{
	char sats_no			[2+1]	;	///< 좌석번호_arr
	char pcpy_no			[14+1]	;	///< 선점번호_arr
	char tck_knd_cd			[1+1]	;	///< 티켓종류코드_arr
} stk_tm_tcktran_list_t, *pstk_tm_tcktran_list_t;

typedef struct
{
	char user_no 			[6+1]	;	///< 사용자번호 	
	char lng_cd 			[2+1]	;	///< 언어코드 	
	char alcn_depr_trml_no	[3+1]	;	///< 배차출발터미널번호
	char alcn_arvl_trml_no	[3+1]	;	///< 배차도착터미널번호
	char alcn_depr_dt		[8+1]	;	///< 배차출발일자
	char alcn_depr_time		[6+1]	;	///< 배차출발시각
	char depr_trml_no		[3+1]	;	///< 출발터미널번호
	char arvl_trml_no		[3+1]	;	///< 도착터미널번호
	char depr_dt			[8+1]	;	///< 출발일자
	char depr_time			[6+1]	;	///< 출발시각
	char bus_cls_cd			[3+1]	;	///< 버스등급코드(일반)
	char cacm_cd			[2+1]	;	///< 운수사코드
	char trml_no			[3+1]	;	///< 터미널번호
	char wnd_no				[2+1]	;	///< 창구번호
	char inhr_no			[8+1]	;	///< 고유번호
	char sats_no_aut_yn		[1+1]	;	///< 좌석번호 자동부여 여부(Y/N)
	char inp_dvs_cd			[1+1]	;	///< 지불입력구분(C:카드,M:현금)
	char card_no			[128+1]	;	///< "CM:카드번호(현금:소득공제사용자번호) '카드번호(16)'='유효기간(4)'"
	char mip_mm_num			[4+1]	;	///< (int) C:할부개월수(0:일시불)
	char sgn_inf			[3072+1];	///< C:서명정보
//	char sgn_inf			[200+1]	;	///< C:서명정보
//	char iccd_inf			[200+1]	;	///< C:IC정보 (IC카드인 경우만)
	char iccd_inf			[512+1]	;	///< C:IC정보 (IC카드인 경우만)
	char chit_use_dvs		[1+1]	;	///< M:전표사용구분
	char iccd_yn			[1+1]	;	///< IC카드여부(Y/N)
	char trck_dta_enc_cd	[1+1]	;	///< TrackData암호화구분 (D:DAmo암호화, R:외부기기 암호화, .(점):암호화 안함)

	char tissu_hcnt			[4+1]	;	///< (int) 발권매수
	stk_tm_tcktran_list_t List[30];
	
} stk_tm_tcktran_t, *pstk_tm_tcktran_t;

typedef struct
{
	char sats_no			[2+1]	;	///< 좌석번호_arr			
	char tck_knd_cd			[1+1]	;	///< 티켓종류코드_arr			
	char invs_no			[6+1]	;	///< 심사번호_arr			
	char tissu_dt			[8+1]	;	///< 발권일자_arr			
	char tissu_trml_no		[3+1]	;	///< 발권터미널번호_arr		
	char tissu_wnd_no		[2+1]	;	///< 발권창구번호_arr			
	char tissu_sno			[4+1]	;	///< 발권일련번호_arr			
	char tissu_fee			[4+1]	;	///< (int)발권요금_arr			
	char dc_yn				[1+1]	;	///< 뒷좌석 할인여부(Y/N)_arr	
	char dc_fee				[4+1]	;	///< (int)뒷좌석 할인요금_arr		
	char depr_hngl_nm 		[100+1]	;	///< 출발터미널한글명_arr		
	char depr_trml_abrv_nm	[100+1]	;	///< 출발터미널약칭_arr		
	char depr_eng_nm		[100+1]	;	///< 출발터미널영문명_arr		
	char arvl_hngl_nm		[100+1]	;	///< 도착터미널한글명_arr		
	char arvl_trml_abrv_nm	[100+1]	;	///< 도착터미널약칭_arr		
	char arvl_trml_eng_nm	[100+1]	;	///< 도착터미널영문명_arr		
	char bus_oprn_dist		[5+1]	;	///< 경유거리_arr			
	char trml_bizr_no		[30+1]	;	///< 터미널사업자번호_arr		
	char trtr_rot_exsn_yn	[1+1]	;	///< 출발환승지여부_arr		
	char trtr_trml_yn		[1+1]	;	///< 도착환승지여부_arr		
	char tdepr_trml_nm		[100+1]	;	///< 배차출발터미널약칭_arr		
	char tarvl_trml_nm		[100+1]	;	///< 배차도착터미널약칭_arr		
} rtk_tm_tcktran_list_t, *prtk_tm_tcktran_list_t;

typedef struct
{
	char tak_stt_dt			[8+1]	;	///< 작업시작일자	
	char stt_time			[6+1]	;	///< 시작시각	
	char card_aprv_no		[10+1]	;	///< 카드승인번호	
	char aprv_amt			[4+1]	;	///< (int)승인금액	
	char rot_rdhm_no_val	[100+1]	;	///< 승차홈번호값	

	char rmn_pnt			[4+1]	;	///< 잔여 포인트
	char csrc_aprv_no		[10+1]	;	///< 현금영수증 승인번호	
	char user_key_val		[100+1]	;	///< 현금영수증 등록번호

	char chit_use_dvs		[1+1]	;	///< 전표사용 구분 - 소득공제(0x30), 사업자증빙(0x31), 자진발급(0x32)
} rtk_tm_tcktran_info_t, *prtk_tm_tcktran_info_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지코드				
	char msg_dtl_ctt		[200+1]	;	///< 메시지명

	rtk_tm_tcktran_info_t	info;

	char rec_ncnt1			[4+1]	;	///< (int) 발권매수	
	rtk_tm_tcktran_list_t* pList;
} rtk_tm_tcktran_t, *prtk_tm_tcktran_t;

//------------------------------
// 인터넷 예매조회 : TM_MRSINFO

typedef struct
{
	char user_no 			[6+1]	;	///< 사용자번호 	
	char lng_cd 			[2+1]	;	///< 언어코드 	

	char tissu_dvs_cd		[1+1]	;	///< 예매발행취소구분(1:발행,2:취소)	
	char depr_dt			[8+1]	;	///< 출발일자					
	char pyn_dvs_cd			[1+1]	;	///< 검색조건					
	char mrs_mrnp_no		[14+1]	;	///< 1:예매예약번호				
	char mrsp_mbph_no		[100+1]	;	///< 2:예매자휴대폰번호				
	char mrsp_brdt			[8+1]	;	///< 25:생년월일				
	char card_no			[100+1]	;	///< 34:카드번호				
	char depr_trml_no		[3+1]	;	///< 출발터미널
	//추기 20200909
	char adtn_cpn_no		[16+1]	;	///< 부가상품번호
	char alcn_depr_dt		[8+1]	;	///< 출발일자

} stk_tm_mrsinfo_t, *pstk_tm_mrsinfo_t;

typedef struct
{
	char mrs_mrnp_no		[14+1]	;	///< 예매예약번호_arr								
	char mrs_mrnp_sno		[2+1]	;	///< 예매예약일련번호_arr							
	char mrs_mrnp_dt		[8+1]	;	///< 예매예약일자_arr								
	char mrs_mrnp_time		[6+1]	;	///< 예매예약시각_arr								
	char mbrs_no			[16+1]	;	///< 회원번호_arr								
	char depr_trml_no		[3+1]	;	///< 출발터미널번호_arr							
	char arvl_trml_no		[3+1]	;	///< 도착터미널번호_arr							
	char alcn_depr_trml_no	[3+1]	;	///< 배차출발터미널번호_arr							
	char alcn_arvl_trml_no	[3+1]	;	///< 배차도착터미널번호_arr							
	char depr_dt			[8+1]	;	///< 출발일자_arr								
	char depr_time			[6+1]	;	///< 출발시각_arr								
	char cacm_cd			[2+1]	;	///< 운수사코드_arr								
	char bus_cls_cd			[1+1]	;	///< 버스등급코드_arr								
	char mrs_chnl_dvs_cd	[1+1]	;	///< 예매채널구분코드_arr							
	char pub_chnl_dvs_cd	[1+1]	;	///< 발행채널구분코드_arr							
	char pyn_dtl_cd			[1+1]	;	///< 지불상세코드_arr								
	char adlt_num			[4]		;	///< (int)어른수_arr									
	char adlt_tot_amt		[4]		;	///< (int)어른총금액_arr								
	char chld_num			[4]		;	///< (int)어린이수_arr								
	char chld_tot_amt		[4]		;	///< (int)어린이총금액_arr								
	char teen_num			[4]		;	///< (int)청소년수_arr								
	char teen_tot_amt		[4]		;	///< (int)청소년총금액_arr								
	char uvsd_num			[4]		;	///< (int)대학생수_arr								
	char uvsd_tot_amt		[4]		;	///< (int)대학생총금액_arr		
	// 20210614 ADD
	char sncn_num			[4]		;	///< (int)경로수_arr
	char sncn_tot_amt		[4]		;	///< (int)경로총금액_arr
	char dspr_num			[4]		;	///< (int)장애인수_arr
	char dspr_tot_amt		[4]		;	///< (int)장애인총금액_arr
	char vtr3_num			[4]		;	///< (int)보훈30수_arr
	char vtr3_tot_amt		[4]		;	///< (int)보훈30총금액_arr
	char vtr5_num			[4]		;	///< (int)보훈50수_arr
	char vtr5_tot_amt		[4]		;	///< (int)보훈50총금액_arr
	char vtr7_num			[4]		;	///< (int)보훈70수_arr
	char vtr7_tot_amt		[4]		;	///< (int)보훈70총금액_arr
	// 20210614 ~ADD
	// 20220713 ADD
	char dfpt_num			[4]		;	///< (int)후불수_arr
	char dfpt_tot_amt		[4]		;	///< (int)후불총금액_arr
	// 20220713 ~ADD
	char sats_num			[4]		;	///< (int)좌석수_arr
	char ride_psb_fcnt		[4]		;	///< 정기정액권 탑승가능횟수_arr
	char tot_ride_psb_fcnt	[4]		;	///< 정기정액권 총승차가능횟수_arr
	char rtrp_dvs_cd1		[1+1]	;	///< 왕복구분코드1_arr(1:편도,2:왕복,3:시외우등할인왕복)
	char depr_hngl_nm 		[100+1]	;	///< 출발터미널한글명_arr
	char arvl_hngl_nm		[100+1]	;	///< 도착터미널한글명_arr

} rtk_tm_mrsinfo_list_t, *prtk_tm_mrsinfo_list_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지코드				
	char msg_dtl_ctt		[200+1]	;	///< 메시지명
	char rec_ncnt1			[4+1]	;	///< (int) 발권매수	
	rtk_tm_mrsinfo_list_t* pList;
} rtk_tm_mrsinfo_t, *prtk_tm_mrsinfo_t;

//------------------------------
// 예매발행 : TM_MRSPUB

typedef struct
{
	char mrs_mrnp_no		[14+1]	;	///< 예매예약번호_arr	
	char mrs_mrnp_sno		[2+1]	;	///< 예매예약일련번호_arr
} stk_tm_mrspub_list_t, *pstk_tm_mrspub_list_t;

typedef struct
{
	char user_no 			[6+1]	;	///< 사용자번호 	
	char lng_cd 			[2+1]	;	///< 언어코드 	
	char tissu_trml_no		[3+1]	;	///< 발권터미널번호
	char wnd_no				[2+1]	;	///< 창구번호	
	char stt_inhr_no		[8+1]	;	///< 시작고유번호	
	char rec_ncnt			[4+1]	;	///< (int) 발권매수	
	stk_tm_mrspub_list_t	List[30];
} stk_tm_mrspub_t, *pstk_tm_mrspub_t;

typedef struct
{
	char tissu_dt			[8+1]	;	///< 발권일자_arr		
	char tissu_time			[6+1]	;	///< 발권시간_arr		
	char tissu_trml_no		[3+1]	;	///< 터미널번호_arr		
	char tissu_wnd_no		[2+1]	;	///< 창구번호_arr		
	char tissu_sno			[4+1]	;	///< 발권일련번호_arr		
	char sats_no			[2+1]	;	///< 좌석번호_arr		
	char invs_no			[6+1]	;	///< 심사번호_arr		
	char tck_knd_cd			[2+1]	;	///< 티켓종류코드_arr		
	char pyn_dvs_cd			[1+1]	;	///< 지불구분코드_arr		// 20210910 지불구분코드_arr(1:현금,2:카드,3:마일리지,4:계좌이체,5: 복합)	
	char pyn_dtl_cd			[1+1]	;	///< 지불상세코드_arr		
	char card_no			[16+1]	;	///< 카드번호_arr		
	char card_aprv_no		[10+1]	;	///< 카드승인번호_arr		
	//char ogn_aprv_amt		[4]		;	///< (int)카드승인금액_arr	// 20211019 DEL
	// 20211019 ADD
	char ogn_aprv_amt		[4]		;	///< (int)카드원승인금액_arr(후불카드 OR 가상선불 원승인금액)
	char rmn_aprv_amt		[4]		;	///< (int)카드잔여승인금액_arr(후불카드 OR 가상선불 잔여금액)		
	// 20211019 ~ADD
	char mip_mm_num			[4]		;	///< (int)할부개월_arr		
	char csrc_aprv_no		[10+1]	;	///< 현금영수증승인번호_arr	
	char user_key_val		[100+1]	;	///< 현금영수증등록번호_arr	
	char cash_recp_amt		[4]		;	///< (int)현금영수증금액_arr	
	char tissu_fee			[4]		;	///< (int)발권요금_arr		
	char stt_time			[6+1]	;	///< 작업시작시간_arr		
	char alcn_depr_trml_no	[3+1]	;	///< 배차출발터미널_arr	
	char alcn_arvl_trml_no	[3+1]	;	///< 배차도착터미널_arr	
	char alcn_depr_dt		[8+1]	;	///< 배차출발일자_arr		
	char alcn_depr_time		[6+1]	;	///< 배차출발시간_arr		
	char depr_trml_no		[3+1]	;	///< 출발터미널_arr		
	char arvl_trml_no		[3+1]	;	///< 도착터미널_arr		
	char depr_dt			[8+1]	;	///< 출발일자_arr		
	char depr_time			[6+1]	;	///< 출발시간_arr		
	char bus_cls_cd			[3+1]	;	///< 버스등급코드_arr		
	char cacm_cd			[2+1]	;	///< 운수사코드_arr		
	char mrsp_mbph_no		[100+1]	;	///< 예매자휴대폰번호_arr	
	char depr_hngl_nm 		[100+1]	;	///< 출발터미널한글명_arr	
	char depr_trml_abrv_nm	[100+1]	;	///< 출발터미널약칭_arr	
	char depr_eng_nm		[100+1]	;	///< 출발터미널영문명_arr	
	char arvl_hngl_nm		[100+1]	;	///< 도착터미널한글명_arr	
	char arvl_trml_abrv_nm	[100+1]	;	///< 도착터미널약칭_arr	
	char arvl_trml_eng_nm	[100+1]	;	///< 도착터미널영문명_arr	
	char bus_oprn_dist		[5+1]	;	///< 경유거리_arr		
	char trml_bizr_no		[30+1]	;	///< 터미널사업자번호_arr	
	char trtr_rot_exsn_yn	[1+1]	;	///< 출발환승지여부_arr	
	char trtr_trml_yn		[1+1]	;	///< 도착환승지여부_arr	
	char tdepr_trml_nm		[100+1]	;	///< 배차출발터미널약칭_arr	
	char tarvl_trml_nm		[100+1]	;	///< 배차도착터미널약칭_arr	
	// 20211019 ADD
	char mlg_rmn_amt		[4]		;	///< (int)마일리지 잔여금액_arr
	char cpn_rmn_amt		[4]		;	///< (int)쿠폰 잔여금액_arr		
	// 20211019 ~ADD
} rtk_tm_mrspub_list_t, *prtk_tm_mrspub_list_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지코드				
	char msg_dtl_ctt		[200+1]	;	///< 메시지명
	char rot_rdhm_no_val	[100+1]	;	///< 승차홈번호
	char rec_ncnt1			[4+1]	;	///< (int) 발권매수	
	rtk_tm_mrspub_list_t* pList;
} rtk_tm_mrspub_t, *prtk_tm_mrspub_t;

//------------------------------
// 환불 대상 금액 조회 : TM_RYAMTINFO

typedef struct
{
	char tissu_dt		 	[8+1]	;	///< 발권일자_arr	
	char tissu_trml_no	 	[3+1]	;	///< 발권터미널번호_arr
	char tissu_wnd_no	 	[2+1]	;	///< 발권창구번호_arr	
	char tissu_sno		 	[6+1]	;	///< 발권일련번호_arr	
} stk_tm_ryamtinfo_list_t, *pstk_tm_ryamtinfo_list_t;

typedef struct
{
	char user_no 			[6+1]	;	///< 사용자번호 	
	char lng_cd 			[2+1]	;	///< 언어코드 	
	char rec_ncnt			[4+1]	;	///< 취소매수
	stk_tm_ryamtinfo_list_t	List[2];
} stk_tm_ryamtinfo_t, *pstk_tm_ryamtinfo_t;

typedef struct
{
	char alcn_depr_trml_no	[3+1]	;	///< 배차출발터미널번호_arr	
	char alcn_arvl_trml_no	[3+1]	;	///< 배차도착터미널번호_arr	
	char alcn_depr_dt		[8+1]	;	///< 배차출발일자_arr		
	char alcn_depr_time		[6+1]	;	///< 배차출발시각_arr		
	char depr_trml_no		[3+1]	;	///< 출발터미널번호_arr	
	char arvl_trml_no		[3+1]	;	///< 도착터미널번호_arr	
	char depr_dt			[8+1]	;	///< 출발일자_arr		
	char depr_time			[6+1]	;	///< 출발시각_arr		
	char bus_cls_cd			[3+1]	;	///< 버스등급코드_arr		
	char cacm_cd			[2+1]	;	///< 운수사코드_arr		
	char sats_no			[2+1]	;	///< 좌석번호_arr		
	char tck_knd_cd			[1+1]	;	///< 티켓종류코드_arr		
} rtk_tm_ryamtinfo_list_t, *prtk_tm_ryamtinfo_list_t;

typedef struct
{
	char pyn_dvs_cd			[1+1]	;	///< 지불구분코드	
	char pyn_dtl_cd			[1+1]	;	///< 지불상세코드	
	char tissu_fee			[4+1]	;	///< (int)발권요금	
	char ry_knd_cd			[1+1]	;	///< 환불종류코드	
	char cmrt				[4+1]	;	///< (int)환불율		
	char tot_ry_amt			[4+1]	;	///< (int)총환불금액	
	char ry_pfit			[4+1]	;	///< (int)환불차익	

} rtk_tm_ryamtinfo_dt_t, *prtk_tm_ryamtinfo_dt_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지코드				
	char msg_dtl_ctt		[200+1]	;	///< 메시지명

	rtk_tm_ryamtinfo_dt_t	dt;
	
	char rec_ncnt1			[4+1]	;	///< (int) 발권매수	
	rtk_tm_ryamtinfo_list_t* pList;
} rtk_tm_ryamtinfo_t, *prtk_tm_ryamtinfo_t;

//------------------------------
// 환불 - 무인기 발권 : TM_REPTRAN

typedef struct
{
	char tissu_dt			[8+1]	;	///< 발권일자_arr		
	char tissu_trml_no		[3+1]	;	///< 발권터미널번호_arr	
	char tissu_wnd_no		[2+1]	;	///< 발권창구번호_arr		
	char tissu_sno			[4+1]	;	///< 발권일련번호_arr		
	char sats_no			[2+1]	;	///< 좌석번호_arr		
} stk_tm_reptran_list_t, *pstk_tm_reptran_list_t;

typedef struct
{
	char user_no		[6+1]	;	///< 사용자 no		
	char lng_cd			[2+1]	;	///< 언어구분		
	char ry_trml_no		[3+1]	;	///< 환불터미널번호	
	char ry_wnd_no		[2+1]	;	///< 환불창구번호		
	char ry_user_no		[6+1]	;	///< 환불사용자번호	
	char iccd_yn		[1+1]	;	///< ic카드여부(y/n)
	
	char ddct_qnt		[4+1]	;	///< 차감수량
	stk_tm_reptran_list_t	List[2];
} stk_tm_reptran_t, *pstk_tm_reptran_t;

typedef struct
{
	char msg_cd			[6+1]	;	///< 메시지코드	
	char msg_dtl_ctt	[200+1]	;	///< 메시지명	
	char rmn_pnt		[4+1]	;	///< (int)잔여포인트	
	char tissu_fee  	[4+1]	;	///< (int)발권금액	
	char ry_knd_cd		[1+1]	;	///< 환불종류코드	
	char cmrt			[4+1]	;	///< (int)환불율		
	char tot_ry_amt 	[4+1]	;	///< (int)환불금액	
	char ry_pfit    	[4+1]	;	///< (int)환불차익금액	
	char pyn_dvs_cd		[1+1]	;	///< 지불구분코드 : 1:현금,2:카드,3:마일리지,4:계좌이체
} rtk_tm_reptran_t, *prtk_tm_reptran_t;

//------------------------------
// 환불 - 창구발권 : TM_TCKCAN

typedef struct
{
	char user_no		[6+1]	;	///< 사용자 no		
	char lng_cd			[2+1]	;	///< 언어구분		
	char ry_trml_no		[3+1]	;	///< 환불터미널번호	
	char ry_wnd_no		[2+1]	;	///< 환불창구번호		
	char ry_user_no		[6+1]	;	///< 환불사용자번호	
	char tissu_dt		[8+1]	;	///< 발권일자
	char tissu_trml_no	[3+1]	;	///< 발권터미널번호
	char tissu_wnd_no	[2+1]	;	///< 발권창구번호
	char tissu_sno		[4+1]	;	///< 발권일련번호

} stk_tm_tckcan_t, *pstk_tm_tckcan_t;

typedef struct
{
	char msg_cd			[6+1]	;	///< 메시지코드	
	char msg_dtl_ctt	[200+1]	;	///< 메시지명	
	char rmn_pnt		[4+1]	;	///< (int)잔여포인트	
	char tissu_fee  	[4+1]	;	///< (int)발권금액	
	char ry_knd_cd		[1+1]	;	///< 환불종류코드	
	char cmrt			[4+1]	;	///< (int)환불율		
	char tot_ry_amt 	[4+1]	;	///< (int)환불금액	
	char ry_pfit    	[4+1]	;	///< (int)환불차익금액	
	char pyn_dvs_cd		[1+1]	;	///< 지불구분코드 : 1:현금,2:카드,3:마일리지,4:계좌이체
} rtk_tm_tckcan_t, *prtk_tm_tckcan_t;

//------------------------------
// 발권박스 교환 : TM_MBOXTRAN

typedef struct
{
	char user_no 			[6+1]	;	///< 사용자번호 	
	char lng_cd 			[2+1]	;	///< 언어코드 	

	char trml_no			[3+1]	;	///< 터미널번호	
	char wnd_no				[2+1]	;	///< 창구번호	
	char clos_inhr_no		[8+1]	;	///< 마감고유번호
	char ecg_inhr_no		[8+1]	;	///< 교환고유번호
} stk_tm_mboxtran_t, *pstk_tm_mboxtran_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지코드				
	char msg_dtl_ctt		[200+1]	;	///< 메시지명
} rtk_tm_mboxtran_t, *prtk_tm_mboxtran_t;

//------------------------------
// 창구마감 : TM_MACTTRAN

typedef struct
{
	char user_no 			[6+1]	;	///< 사용자번호 	
	char lng_cd 			[2+1]	;	///< 언어코드 	
	char trml_no			[3+1]	;	///< 터미널번호	
	char wnd_no				[2+1]	;	///< 창구번호	
	char tak_stt_dt			[8+1]	;	///< 작업시작일자	
	char stt_time			[6+1]	;	///< 시작시각	
	char inhr_no 			[8+1]	;	///< 고유번호	
	char upd_user_no		[6+1]	;	///< 수정사용자번호
	char tak_dvs_cd			[1+1]	;	///< 자동창구마감 작업구분코드 (1: 배치job, 2: 터미널담당자 수동수행) // 20210810 ADD
} stk_tm_macttran_t, *pstk_tm_macttran_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지코드				
	char msg_dtl_ctt		[200+1]	;	///< 메시지명
} rtk_tm_macttran_t, *prtk_tm_macttran_t;

//------------------------------
// 발권정보 조회 : TM_MTCKINFO

typedef struct
{
	char user_no 			[6+1]	;	///< 사용자번호 	
	char lng_cd 			[2+1]	;	///< 언어코드 	
	char trml_no			[3+1]	;	///< 터미널번호	
	char wnd_no				[2+1]	;	///< 창구번호	
	char tak_stt_dt			[8+1]	;	///< 발권일자
} stk_tm_mtckinfo_t, *pstk_tm_mtckinfo_t;

typedef struct
{
	char tck_seq			[17+1]	;	///< 티켓일련번호		
	char tissu_time			[6+1]	;	///< 발권시간 		
	char depr_trml_no		[3+1]	;	///< 출발터미널번호	
	char arvl_trml_no		[3+1]	;	///< 도착터미널번호	
	char depr_dt			[8+1]	;	///< 출발일자		
	char depr_time			[6+1]	;	///< 출발시각		
	char bus_cls_cd			[3+1]	;	///< 버스등급코드 	
	char cacm_cd			[2+1]	;	///< 운수사코드 		
	char tck_knd_cd			[1+1]	;	///< 티켓종류코드 	
	char fee				[4+1]	;	///< (int)발권요금		
	char sats_no			[2+1]	;	///< 좌석번호 		
	char invs_no			[6+1]	;	///< 심사번호 		
	char pyn_dvs_cd			[1+1]	;	///< 지불구분코드		
	char pyn_dtl_cd			[1+1]	;	///< 지불상세코드		
	char card_no			[16+1]	;	///< 카드번호 		
	char card_aprv_no		[10+1]	;	///< 카드승인번호		
	char ogn_aprv_amt		[4+1]	;	///< (int)카드승인금액		
	char csrc_aprv_no		[10+1]	;	///< 현금영수증승인번호	
	char user_key_val		[100+1]	;	///< 현금영수증등록번호	
	char cash_recp_amt		[4+1]	;	///< (int)현금영수증금액	
	char inhr_no			[8+1]	;	///< 고유번호		
	char chtk_sta_cd		[1+1]	;	///< 검표상태코드		
	char alcn_depr_trml_no	[3+1]	;	///< 배차출발터미널번호	
	char alcn_arvl_trml_no	[3+1]	;	///< 배차도착터미널번호	
	char rot_rdhm_no		[100+1]	;	///< 승차홈			
	char depr_trml_abrv_nm	[100+1]	;	///< 출발터미널약칭 	
	char arvl_trml_abrv_nm	[100+1]	;	///< 도착터미널약칭 	

} rtk_tm_mtckinfo_list_t, *prtk_tm_mtckinfo_list_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지코드				
	char msg_dtl_ctt		[200+1]	;	///< 메시지명
	char rec_ncnt1			[4+1]	;	///< (int) 발권매수	
	rtk_tm_mtckinfo_list_t* pList;
} rtk_tm_mtckinfo_t, *prtk_tm_mtckinfo_t;

//------------------------------
// 재발행 : TM_MRETTRAN

typedef struct
{
	char user_no 			[6+1]	;	///< 사용자번호 	
	char lng_cd 			[2+1]	;	///< 언어코드 	
	char tissu_dt			[8+1]	;	///< 발권일자
	char tissu_trml_no		[3+1]	;	///< 발권터미널번호	
	char tissu_wnd_no		[2+1]	;	///< 발권창구번호	
	char tissu_sno			[6+1]	;	///< 발권일련번호
} stk_tm_mrettran_t, *pstk_tm_mrettran_t;

typedef struct
{
	char depr_hngl_nm 		[100+1]	;	///< 출발터미널한글명 
	char depr_trml_abrv_nm	[100+1]	;	///< 출발터미널약칭 	
	char depr_eng_nm		[100+1]	;	///< 출발터미널영문명 
	char arvl_hngl_nm		[100+1]	;	///< 도착터미널한글명 
	char arvl_trml_abrv_nm	[100+1]	;	///< 도착터미널약칭 	
	char arvl_trml_eng_nm	[100+1]	;	///< 도착터미널영문명 
	char bus_oprn_dist		[5+1]	;	///< 경유거리 		
	char trml_bizr_no		[30+1]	;	///< 터미널사업자번호 
	char trtr_rot_exsn_yn	[1+1]	;	///< 출발환승지여부 	
	char trtr_trml_yn		[1+1]	;	///< 도착환승지여부 	
	char tarvl_trml_nm		[100+1]	;	///< 배차출발터미널약칭
	char tdepr_trml_nm		[100+1]	;	///< 배차도착터미널약칭

} rtk_tm_mrettran_list_t, *prtk_tm_mrettran_list_t;

typedef struct
{
	char msg_cd				[6+1]	;	///< 메시지코드				
	char msg_dtl_ctt		[200+1]	;	///< 메시지명
	char rec_ncnt1			[4+1]	;	///< (int) 발권매수	
	rtk_tm_mrettran_list_t* pList;
} rtk_tm_mrettran_t, *prtk_tm_mrettran_t;

#pragma pack()

