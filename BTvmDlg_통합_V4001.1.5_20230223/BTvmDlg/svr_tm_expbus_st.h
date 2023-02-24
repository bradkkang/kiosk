// 
// 
// svr_tm_expbus_st.h : 티머니 고속 서버 패킷 구조체 헤더 파일
//

#pragma once



//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

typedef struct 
{
	DWORD	dwFieldKey;
	char*	pName;
} TMEXPBUS_DBG_T, *PTMEXPBUS_DBG_T;

typedef struct
{
	char req_pgm_dvs		[3		+1]	;			///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;			///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;			///< 요청 창구번호
	char req_user_no		[6		+1]	;			///< 요청 사용자번호
} stk_tm_head_t, *pstk_tm_head_t;

typedef struct
{
	char rsp_cd				[6		+1]	;		///< 응답코드
	char rec_num			[4		+1]	;		///< 정보 갯수
} rtk_tm_cmm_t, *prtk_tm_cmm_t;

//------------------------------
// 1. 메시지 정보 조회  : CM_ReadMsg

typedef struct
{
	char req_pgm_dvs		[3		+1]	;			///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;			///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;			///< 요청 창구번호
	char req_user_no		[6		+1]	;			///< 요청 사용자번호

	char lng_cd				[2		+1]	;			///< 언어코드
} stk_tm_readmsg_t, *pstk_tm_readmsg_t;

typedef struct
{
	char lng_cd 			[2		+1]	;			///< 언어코드
	char msg_cd 			[6		+1]	;			///< 메시지 코드
	char msg_dtl_ctt 		[1000	+1];			///< 메시지 상세내용
} rtk_tm_readmsg_list_t, *prtk_tm_readmsg_list_t;

typedef struct
{
	char rsp_cd				[6		+1]	;			///< 응답코드
	char rec_num			[4		+1]	;			///< 정보 수	
	rtk_tm_readmsg_list_t* pList;				///< 리스트
} rtk_tm_readmsg_t, *prtk_tm_readmsg_t;

//------------------------------
// 2. 승차권 고유번호, 공지사항 조회  : CM_ReadNtc

typedef struct
{
	char req_pgm_dvs		[3		+1]	;			///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;			///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;			///< 요청 창구번호
} stk_tm_readntc_t, *pstk_tm_readntc_t;

typedef struct
{
	char trml_no			[3		+1]	;			///< 터미널번호		
	char ntc_mttr_sno	 	[6		+1]	;			///< 공지사항일련번호	
	char ntc_mttr_ttl_nm	[100	+1]	;			///< 공지사항제목명	
	char wrtn_user_nm	 	[100	+1]	;			///< 작성자명		
	char trml_ntc_ctt	 	[2000	+1];			///< 터미널공지내용	
	char stt_dtm			[14		+1]	;			///< 시작일시		
	char end_dtm			[14		+1]	;			///< 종료일시		
} rtk_tm_readntc_list_t, *prtk_tm_readntc_list_t;

typedef struct
{
	char rsp_cd				[6		+1]	;			///< 응답코드
	char serv_dt			[8		+1]	;			///< 서버일자
	char serv_time			[6		+1]	;			///< 서버시각
	char inhr_no			[8		+1]	;			///< 고유번호
	char rec_num			[4		+1]	;			///< 정보 수	
	rtk_tm_readntc_list_t* pList;				///< 리스트
} rtk_tm_readntc_t, *prtk_tm_readntc_t;

//------------------------------
// 3. 로그인 시, 유저 인증  : TK_AuthCmpt

typedef struct
{
	char req_pgm_dvs		[3		+1]	;			///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;			///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;			///< 요청 창구번호

	char cmpt_id			[40		+1]	;			///< 컴퓨터id	
	char user_id			[32		+1]	;			///< 사용자id	
	char user_pwd			[32		+1]	;			///< 사용자비밀번호
	char inhr_no			[8		+1]	;			///< 고유번호	

} stk_tm_authcmpt_t, *pstk_tm_authcmpt_t;

typedef struct
{
	char rsp_cd				[6		+1]	;			///< 응답코드
	char trml_abrv_nm		[100	+1]	;			///< 터미널약칭명		
	char trml_nm			[100	+1]	;			///< 터미널명		
	char trml_eng_abrv_nm	[100	+1]	;			///< 터미널영문약칭명	
	char tel_no				[20		+1]	;			///< 전화번호		
	char user_no			[6		+1]	;			///< 사용자번호		
	char user_nm			[100	+1]	;			///< 사용자명	
	char inhr_no			[8		+1]	;			///< 고유번호		
	char tissu_num			[4		+1]	;			///< (NUMBER) 발권수			
	char tissu_amt			[4		+1]	;			///< (NUMBER) 발권금액		
	char canc_num			[4		+1]	;			///< (NUMBER) 취소매수		
	char canc_amt			[4		+1]	;			///< (NUMBER) 취소금액		
	char ry_num				[4		+1]	;			///< (NUMBER) 환불매수		
	char ry_amt				[4		+1]	;			///< (NUMBER) 환불금액		
	char dc_rc_amt			[4		+1]	;			///< (NUMBER) 할인반환금액		
	char tkt_num			[4		+1]	;			///< (NUMBER) 순매표매수		
	char tkt_amt			[4		+1]	;			///< (NUMBER) 순매표금액		
	char cash_num			[4		+1]	;			///< (NUMBER) 입금액매수		
	char cash_amt			[4		+1]	;			///< (NUMBER) 입금액금액		
	char tissu_psb_yn		[1		+1]	;			///< 발권가능여부		
	char tak_stt_dt			[8		+1]	;			///< 작업시작일자		
	char stt_time			[6		+1]	;			///< 시작시각		
} rtk_tm_authcmpt_t, *prtk_tm_authcmpt_t;

//------------------------------
// 4. 공통코드 조회  : CM_ReadCmnCd

typedef struct
{
	char req_pgm_dvs		[3		+1]	;		///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;		///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;		///< 요청 창구번호
	char req_user_no		[6		+1]	;		///< 요청 사용자번호

	char lng_cd				[2		+1]	;		///< 컴퓨터id	
} stk_tm_readcmncd_t, *pstk_tm_readcmncd_t;

typedef struct
{
	char lng_cd				[2		+1]	;		///< 언어코드	
	char cmn_cd				[4		+1]	;		///< 공통코드	
	char cmn_cd_val			[20		+1]	;		///< 공통코드값	
	char cd_val_mark_seq	[4		+1]	;		///< (Numer)코드값표시순서
	char cd_val_nm			[100	+1]	;		///< 코드값명	
	char bsc_rfrn_val		[100	+1]	;		///< 기본참조값	
	char adtn_rfrn_val		[100	+1]	;		///< 부가참조값	
} rtk_tm_readcmncd_list_t, *prtk_tm_readcmncd_list_t;

typedef struct
{
	char rsp_cd				[6		+1]	;		///< 응답코드
	char rec_num			[4		+1]	;		///< 정보 갯수
	rtk_tm_readcmncd_list_t *pList;
} rtk_tm_readcmncd_t, *prtk_tm_readcmncd_t;

//------------------------------
// 5. 승차권 인쇄정보 조회  : TK_ReadTckPrtg

typedef struct
{
	char req_pgm_dvs		[3		+1]	;		///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;		///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;		///< 요청 창구번호
	char req_user_no		[6		+1]	;		///< 요청 사용자번호

	char trml_no			[3		+1]	;		///< 터미널번호		
	char papr_tck_dvs_cd	[3		+1]	;		///< 종이티켓구분코드	
	char ptr_knd_cd			[1		+1]	;		///< 프린터종류코드	
} stk_tm_readtckprtg_t, *pstk_tm_readtckprtg_t;

typedef struct
{
	char trml_no			[3		+1]	;		///< 터미널번호		
	char papr_tck_dvs_cd	[3		+1]	;		///< 종이티켓구분코드	
	char ptr_knd_cd			[1		+1]	;		///< 프린터종류코드	
	char sort_seq			[4		+1]	;		///< NUMBER 정렬순서		
	char ptrg_usg_cd		[1		+1]	;		///< 인쇄용도코드		
	char ptrg_atc_nm		[100	+1]	;		///< 인쇄항목명		
	char x_crdn_val			[100	+1]	;		///< x좌표값		
	char y_crdn_val			[100	+1]	;		///< y좌표값		
	char mgnf_val			[100	+1]	;		///< 배율값			
	char use_yn				[1		+1]	;		///< 사용여부		
} rtk_tm_readtckprtg_list_t, *prtk_tm_readtckprtg_list_t;

typedef struct
{
	char rsp_cd				[6		+1]	;		///< 응답코드
	char rec_num			[4		+1]	;		///< 정보 갯수
	rtk_tm_readtckprtg_list_t *pList;
} rtk_tm_readtckprtg_t, *prtk_tm_readtckprtg_t;

//------------------------------
// 6. 방면정보 조회  : MG_ReadTrmlDrtn

typedef struct
{
	char req_pgm_dvs		[3		+1]	;		///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;		///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;		///< 요청 창구번호
	char req_user_no		[6		+1]	;		///< 요청 사용자번호

	char trml_no			[3		+1]	;		///< 터미널번호		
} stk_tm_readtrmldrtn_t, *pstk_tm_readtrmldrtn_t;

typedef struct
{
	char trml_no			[3		+1]	;		///< 터미널번호		
	char drtn_cd			[2		+1]	;		///< 방면코드		
	char drtn_nm			[100	+1]	;		///< 방면명		
} rtk_tm_readtrmldrtn_list_t, *prtk_tm_readtrmldrtn_list_t;

typedef struct
{
	char rsp_cd				[6		+1]	;		///< 응답코드
	char rec_num			[4		+1]	;		///< 정보 갯수
	rtk_tm_readtrmldrtn_list_t *pList;
} rtk_tm_readtrmldrtn_t, *prtk_tm_readtrmldrtn_t;


//------------------------------
// 7. 자기터미널 정보 조회  : TK_ReadOwnrTrml

typedef struct
{
	char req_pgm_dvs		[3		+1]	;		///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;		///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;		///< 요청 창구번호

} stk_tm_readownrtrml_t, *pstk_tm_readownrtrml_t;

typedef struct
{
	char rsp_cd				[6		+1]	;		///< 응답코드
	char trml_abrv_nm		[100	+1]	;		///< 터미널약칭명		
	char trml_nm			[100	+1]	;		///< 터미널명		
	char trml_eng_nm		[100	+1]	;		///< 터미널영문명		
	char csrc_rgt_no		[9		+1]	;		///< 현금영수증등록번호	
	char tel_no				[20		+1]	;		///< 전화번호		
	char trtr_trml_yn		[1		+1]	;		///< 환승터미널여부	
} rtk_tm_readownrtrml_t, *prtk_tm_readownrtrml_t;

//------------------------------
// 8. 터미널 조회(전국터미널)  : CM_ReadTrml

typedef struct
{
	char req_pgm_dvs		[3		+1]	;		///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;		///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;		///< 요청 창구번호
	char req_user_no		[6		+1]	;		///< 요청 사용자번호

	char lng_cd				[2		+1]	;		///< 언어코드

} stk_tm_readtrml_t, *pstk_tm_readtrml_t;

typedef struct
{
	char lng_cd				[2		+1]	;		///< (ins)언어코드

	char trml_no			[3		+1]	;		///< 터미널번호		
	char trml_nm			[100	+1]	;		///< 터미널명		
	char trml_abrv_nm		[100	+1]	;		///< 터미널약칭명		
	char trml_dtl_addr		[100	+1]	;		///< 터미널주소		
	char trml_eng_nm		[100	+1]	;		///< 터미널영문명		
	char trml_eng_abrv_nm	[100	+1]	;		///< 터미널영문약칭명	
	char trtr_trml_yn		[1		+1]	;		///< 환승터미널여부	
	char sys_dvs_cd			[1		+1]	;		///< 시스템구분코드	
	char tel_no				[20		+1]	;		///< 전화번호		
	char stlm_trml_no		[3		+1]	;		///< 정산터미널번호	
} rtk_tm_readtrml_list_t, *prtk_tm_readtrml_list_t;

typedef struct
{
	char rsp_cd				[6		+1]	;		///< 응답코드
	char rec_num			[4		+1]	;		///< 정보 갯수
	rtk_tm_readtrml_list_t *pList;
} rtk_tm_readtrml_t, *prtk_tm_readtrml_t;

//------------------------------
// 9. 터미널 정보 조회  : CM_ReadTrmlInf

typedef struct
{
	char req_pgm_dvs		[3		+1]	;		///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;		///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;		///< 요청 창구번호
	char req_user_no		[6		+1]	;		///< 요청 사용자번호

	char bef_aft_dvs		[1		+1]	;		///< 이전이후구분	
	char use_yn				[1		+1]	;		///< 사용여부	
	char rec_num			[4		+1]	;		///< 정보 수	
	char arvl_trml_no		[3		+1]	;		///< 도착터미널번호
	char trml_no			[3		+1]	;		///< 터미널번호	
} stk_tm_readtrmlinf_t, *pstk_tm_readtrmlinf_t;

typedef struct
{
	char trml_no			[3		+1]	;		///< 터미널번호	
	char arvl_trml_no		[3		+1]	;		///< 도착터미널번호
	char trml_abrv_nm		[100	+1]	;		///< 터미널약칭명	
	char trml_eng_abrv_nm	[100	+1]	;		///< 터미널영문약칭
	char trml_stup_cd		[3		+1]	;		///< 터미널설정코드
	char trml_scrn_prin_nm	[100	+1]	;		///< 터미널화면출력
	char trml_ptrg_nm		[100	+1]	;		///< 터미널인쇄명	
	char hspd_cty_dvs_cd	[1		+1]	;		///< 고속시외구분코
	char use_yn				[1		+1]	;		///< 사용여부	
} rtk_tm_readtrmlinf_list_t, *prtk_tm_readtrmlinf_list_t;

typedef struct
{
	char rsp_cd				[6		+1]	;		///< 응답코드
	char rec_num			[4		+1]	;		///< 정보 갯수
	rtk_tm_readtrmlinf_list_t *pList;
} rtk_tm_readtrmlinf_t, *prtk_tm_readtrmlinf_t;

//------------------------------
// 10. 터미널 환경설정 정보  : CM_ReadTrmlStup

typedef struct
{
	char req_pgm_dvs		[3		+1]	;		///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;		///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;		///< 요청 창구번호
	char req_user_no		[6		+1]	;		///< 요청 사용자번호

} stk_tm_readtrmlstup_t, *pstk_tm_readtrmlstup_t;

typedef struct
{
	char trml_no				[3		+1]	;	///< 터미널번호			
	char pgm_stup_dvs_cd		[1		+1]	;	///< 프로그램설정구분코드	
	char fee_adpt_ct_cd			[1		+1]	;	///< 요금적용기준코드		
	char ry_amt_adpt_unt_val	[4		+1]	;	///< (Number)환불금액적용단위값		
	char stlm_time				[6		+1]	;	///< 정산시각			
	char scng_mrk_drtm			[4		+1]	;	///< (Number)잔돈표시시간			
	char dpsp_prin_hcnt			[4		+1]	;	///< (Number)입금표출력매수		
	char trml_tel_no_prin_yn	[1		+1]	;	///< 터미널전화번호출력여부	
	char bcd_prin_yn			[1		+1]	;	///< 바코드출력여부		
	char ente_rp_key_val		[4		+1]	;	///< 엔터대체키값			
	char esc_rp_key_val			[4		+1]	;	///< esc대체키값		
	char strt_tissu_key_val		[4		+1]	;	///< 바로발권키값			

//	char mng_shct_fnct_key_val	[4		+1]	;	///< 관리단축기능키값		
//	char tissu_shct_key_val		[4		+1]	;	///< 발권단축기능키값		

	char mng_shct_fnct_key_val	[100	+1]	;	///< 관리단축기능키값		
	char tissu_shct_key_val		[100	+1]	;	///< 발권단축기능키값		

	char ry_gan_cutt_yn			[1		+1]	;	///< 환불이익절삭여부		
	char csrc_tissu_key_val		[4		+1]	;	///< 현금영수증발권키값		
	char card_tissu_key_val		[4		+1]	;	///< 카드발권키값			
	char mrs_tissu_psb_dno		[4		+1]	;	///< (Number)예매가능일수			
	char trcr_use_key_yn		[1		+1]	;	///< 교통카드사용키		
	char sats_stup_use_yn		[1		+1]	;	///< 좌석설정사용여부		
} rtk_tm_readtrmlstup_list_t, *prtk_tm_readtrmlstup_list_t;

typedef struct
{
	char rsp_cd				[6		+1]	;		///< 응답코드
	char rec_num			[4		+1]	;		///< 정보 갯수
	rtk_tm_readtrmlstup_list_t *pList;
} rtk_tm_readtrmlstup_t, *prtk_tm_readtrmlstup_t;

//------------------------------
// 11. 창구정보 조회  : MG_ReadWnd

typedef struct
{
	char req_pgm_dvs		[3		+1]	;		///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;		///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;		///< 요청 창구번호
	char req_user_no		[6		+1]	;		///< 요청 사용자번호

} stk_tm_readwnd_t, *pstk_tm_readwnd_t;

typedef struct
{
	char trml_no				[3		+1]	;	///< 터미널번호			
	char wnd_no					[2		+1]	;	///< 창구번호			
	char tissu_psb_yn			[1		+1]	;	///< 발권가능여부			
	char ry_psb_yn				[1		+1]	;	///< 환불가능여부			
	char mrnp_psb_yn			[1		+1]	;	///< 예약가능여부			
	char mrnp_tissu_psb_yn		[1		+1]	;	///< 예약발권가능여부		
	char card_tissu_psb_yn		[1		+1]	;	///< 카드발권가능여부		
	char nsat_tissu_psb_yn		[1		+1]	;	///< 무표발권가능여부		
	char temp_nsat_tissu_yn		[1		+1]	;	///< 임시무표발권여부		
	char otr_wnd_tissu_canc_yn	[1		+1]	;	///< 타창구발권취소여부		
	char canc_psb_yn			[1		+1]	;	///< 취소가능여부			
	char rpub_psb_yn			[1		+1]	;	///< 재발행가능여부		
	char thdd_owt_yn			[1		+1]	;	///< 당일편도여부			
	char csrc_pub_yn			[1		+1]	;	///< 현금영수증발행여부		
	char tissu_cacm_cd1			[2		+1]	;	///< 발권가능운수사코드1	
	char tissu_cacm_cd2			[2		+1]	;	///< 발권가능운수사코드2	
	char tissu_cacm_cd3			[2		+1]	;	///< 발권가능운수사코드3	
	char tissu_cacm_cd4			[2		+1]	;	///< 발권가능운수사코드4	
	char tissu_cacm_cd5			[2		+1]	;	///< 발권가능운수사코드5	
	char tissu_impb_cacm_cd1	[2		+1]	;	///< 발권불가능운수사코드1	
	char tissu_impb_cacm_cd2	[2		+1]	;	///< 발권불가능운수사코드2	
	char tissu_impb_cacm_cd3	[2		+1]	;	///< 발권불가능운수사코드3	
	char tissu_impb_cacm_cd4	[2		+1]	;	///< 발권불가능운수사코드4	
	char tissu_impb_cacm_cd5	[2		+1]	;	///< 발권불가능운수사코드5	
	char spf_rot_tissu_yn		[1		+1]	;	///< 특정노선발권여부		
	char spf_trml_no_val		[100	+1]	;	///< 특정터미널번호다중값	
	char tissu_ltn_drtm			[4		+1]	;	///< 발권제한시간			
	char tissu_ltn_hcnt			[4		+1]	;	///< 발권제한매수			
	char ptr_knd_cd				[1		+1]	;	///< 프린터종류코드		
	char trml_nm_ptrg_yn		[1		+1]	;	///< 터미널명인쇄여부		
	char sgn_dvc_port_val		[20		+1]	;	///< 서명장치포트값		
	char card_rcgn_dvc_port_val	[20		+1]	;	///< 카드인식장치포트값		
	char trcn_dvs_id			[10		+1]	;	///< 단말기구분id		
	char user_no				[6		+1]	;	///< 사용자번호			
} rtk_tm_readwnd_list_t, *prtk_tm_readwnd_list_t;

typedef struct
{
	char rsp_cd				[6		+1]	;		///< 응답코드
	char rec_num			[4		+1]	;		///< 정보 갯수
	rtk_tm_readwnd_list_t *pList;
} rtk_tm_readwnd_t, *prtk_tm_readwnd_t;

//------------------------------
// 12. 왕복 가능 터미널 조회  : CM_ReadRtrpTrml

typedef struct
{
	char req_pgm_dvs		[3		+1]	;		///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;		///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;		///< 요청 창구번호
	char req_user_no		[6		+1]	;		///< 요청 사용자번호

} stk_tm_readrtrptrml_t, *pstk_tm_readrtrptrml_t;

typedef struct
{
	char arvl_trml_no		[3		+1]	;		///< 도착 터미널번호			
} rtk_tm_readrtrptrml_list_t, *prtk_tm_readrtrptrml_list_t;

typedef struct
{
	char rsp_cd				[6		+1]	;		///< 응답코드
	char rec_num			[4		+1]	;		///< 정보 갯수
	rtk_tm_readrtrptrml_list_t *pList;
} rtk_tm_readrtrptrml_t, *prtk_tm_readrtrptrml_t;

//------------------------------
// 13. 승차권 종류 정보 조회  : CM_ReadTckKnd

typedef struct
{
	char req_pgm_dvs		[3		+1]	;		///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;		///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;		///< 요청 창구번호
	char req_user_no		[6		+1]	;		///< 요청 사용자번호

	char use_yn				[1		+1]	;		///< 사용여부	
	char req_trml_num		[4		+1]	;		///< 요청터미널 수
	char trml_no			[3		+1]	;		///< 터미널번호	
} stk_tm_readtckknd_t, *pstk_tm_readtckknd_t;

typedef struct
{
	char trml_no			[3		+1]	;		///< 터미널번호	
	char tck_knd_cd			[1		+1]	;		///< 티켓종류코드	
	char scrn_prin_nm		[100	+1]	;		///< 화면출력명	
	char scrn_prin_seq		[4		+1]	;		///< (NUMBER) 화면출력순서	
	char ptrg_prin_nm		[100	+1]	;		///< 인쇄출력명	
} rtk_tm_readtckknd_list_t, *prtk_tm_readtckknd_list_t;

typedef struct
{
	char rsp_cd				[6		+1]	;		///< 응답코드
	char rec_num			[4		+1]	;		///< 정보 갯수
	rtk_tm_readtckknd_list_t *pList;
} rtk_tm_readtckknd_t, *prtk_tm_readtckknd_t;

//------------------------------
// 14. 운송회사 정보 조회  : CM_MngCacm

typedef struct
{
	char req_pgm_dvs		[3		+1]	;		///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;		///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;		///< 요청 창구번호
	char req_user_no		[6		+1]	;		///< 요청 사용자번호

	char use_yn				[1		+1]	;		///< 사용여부	
	char cacm_cd			[2		+1]	;		///< 운수사코드	
	char lng_cd				[2		+1]	;		///< 언어코드	
	char rec_num			[4		+1]	;		///< 정보 수	(max : 1000개)
	char bef_aft_dvs		[1		+1]	;		///< 이전이후구분	
	char req_trml_num		[4		+1]	;		///< 요청터미널 수 (무조건 1)
	char trml_no			[3		+1]	;		///< 터미널번호	
} stk_tm_mngcacm_t, *pstk_tm_mngcacm_t;

typedef struct
{
	char trml_no			[3		+1]	;		///< 터미널번호		
	char hspd_cty_dvs_cd	[1		+1]	;		///< 고속시외구분코드	
	char cacm_cd			[2		+1]	;		///< 운수사코드		
	char lng_cd				[2		+1]	;		///< 언어코드		
	char cacm_nm			[100	+1]	;		///< 운수사명		
	char cacm_abrv_nm		[100	+1]	;		///< 운수사약칭명		
	char bizr_no			[30		+1]	;		///< 사업자번호		
	char trml_by_cacm_cd	[2		+1]	;		///< 터미널별운수사코드	
	char scrn_prin_nm		[100	+1]	;		///< 화면출력명		
	char ptrg_prin_nm		[100	+1]	;		///< 인쇄출력명		
	char mod_sta_cd			[1		+1]	;		///< 변경상태코드		
} rtk_tm_mngcacm_list_t, *prtk_tm_mngcacm_list_t;

typedef struct
{
	char rsp_cd				[6		+1]	;		///< 응답코드
	char rec_num			[4		+1]	;		///< 정보 갯수
	rtk_tm_mngcacm_list_t *pList;
} rtk_tm_mngcacm_t, *prtk_tm_mngcacm_t;

//------------------------------
// 15. 노선 정보 조회  : CM_ReadRotInf

typedef struct
{
	char req_pgm_dvs		[3		+1]	;		///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;		///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;		///< 요청 창구번호
	char req_user_no		[6		+1]	;		///< 요청 사용자번호

	char depr_trml_no		[3		+1]	;		///< 출발터미널번호
	char arvl_trml_no		[3		+1]	;		///< 도착터미널번호
} stk_tm_readrotinf_t, *pstk_tm_readrotinf_t;

typedef struct
{
	char depr_trml_no		[3		+1]	;		///< 출발터미널번호	
	char arvl_trml_no		[3		+1]	;		///< 도착터미널번호	
	char mrs_psb_yn			[1		+1]	;		///< 예매가능여부		
	char htck_psb_yn		[1		+1]	;		///< 홈티켓가능여부	
	char bus_oprn_dist		[11		+1]	;		///< 운행거리		
	char take_drtm			[4		+1]	;		///< (number) 소요시간		
	char rot_dvs_cd			[2		+1]	;		///< 노선구분코드		
	char fee_knd_cd			[1		+1]	;		///< 요금종류코드		
	char bsc_ntkn_cd		[2		+1]	;		///< 기본권종코드		
	char oprn_yn			[1		+1]	;		///< 운행여부		
	char rtrp_tissu_psb_yn	[1		+1]	;		///< 왕복발권가능여부	
	char cty_prmm_dc_yn		[1		+1]	;		///< 시외우등할인여부	
	char hspd_cty_dvs_cd	[1		+1]	;		///< 고속시외구분코드	
	char trtr_trml_incl_yn	[1		+1]	;		///< 환승터미널포함여부	
	char trml_dvs_cd		[1		+1]	;		///< 터미널구분코드	
} rtk_tm_readrotinf_list_t, *prtk_tm_readrotinf_list_t;

typedef struct
{
	char rsp_cd				[6		+1]	;		///< 응답코드
	char rec_num			[4		+1]	;		///< 정보 갯수
	rtk_tm_readrotinf_list_t *pList;
} rtk_tm_readrotinf_t, *prtk_tm_readrotinf_t;

//------------------------------
// 16. 환불율 정보 조회  : CM_ReadRyrt

typedef struct
{
	char req_pgm_dvs		[3		+1]	;		///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;		///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;		///< 요청 창구번호
	char req_user_no		[6		+1]	;		///< 요청 사용자번호

	char use_yn				[1		+1]	;		///< 사용여부
} stk_tm_read_ryrt_t, *pstk_tm_read_ryrt_t;

typedef struct
{
	char ry_knd_cd			[1		+1]	;		///< 환불종류코드	
	char cd_val_nm			[100	+1]	;		///< 코드값명		
	char trml_ry_knd_cd		[1		+1]	;		///< 터미널환불종류코드
	char scrn_prin_nm		[100	+1]	;		///< 화면출력명		
	char use_yn				[1		+1]	;		///< 사용여부		

} rtk_tm_read_ryrt_list_t, *prtk_tm_read_ryrt_list_t;

typedef struct
{
	char rsp_cd				[6		+1]	;		///< 응답코드
	char rec_num			[4		+0]	;		///< 정보 갯수
	rtk_tm_read_ryrt_list_t *pList;
} rtk_tm_read_ryrt_t, *prtk_tm_read_ryrt_t;

//------------------------------
// (설정_1). 승차권 박스 교환  : TK_ExcBusTck

typedef struct
{
	char req_pgm_dvs		[3		+1]	;		///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;		///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;		///< 요청 창구번호
	char req_user_no		[6		+1]	;		///< 요청 사용자번호

	char ecg_inhr_no		[24		+1]	;		///< 교환 고유번호
} stk_tm_excbustck_t, *pstk_tm_excbustck_t;

typedef struct
{
	char rsp_cd				[6		+1]	;		///< 응답코드
} rtk_tm_excbustck_t, *prtk_tm_excbustck_t;

//------------------------------
// (설정_2). 창구마감  : TK_ClosWnd

typedef struct
{
	char req_pgm_dvs		[3		+1]	;		///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;		///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;		///< 요청 창구번호
	char req_user_no		[6		+1]	;		///< 요청 사용자번호

	char inhr_no			[8		+1]	;		///< 고유번호
} stk_tm_closwnd_t, *pstk_tm_closwnd_t;

typedef struct
{
	char rsp_cd				[6		+1]	;		///< 응답코드

	char tak_stt_dt			[8		+1]	;		///< 작업시작 일자
	char stt_time			[6		+1]	;		///< 시작 시각
} rtk_tm_closwnd_t, *prtk_tm_closwnd_t;

//------------------------------
// (현장발권_1). 배차조회  : MG_ReadAlcn

typedef struct
{
	char req_pgm_dvs		[3		+1]	;		///< 요청 프로그램 구분
	char req_trml_no		[3		+1]	;		///< 요청 터미널번호
	char req_wnd_no			[2		+1]	;		///< 요청 창구번호
	char req_user_no		[6		+1]	;		///< 요청 사용자번호

	char rec_num			[4		+0];		///< (Num)정보 수		
	char bef_aft_dvs		[1		+1];		///< 이전이후구분		
	char depr_dt			[8		+1];		///< 출발일자		
	char depr_trml_no		[3		+1];		///< 출발터미널번호	
	char arvl_trml_no		[3		+1];		///< 도착터미널번호	
	char bus_cls_cd			[1		+1];		///< 버스등급코드		
	char depr_time			[6		+1];		///< 출발시각		
	char read_bcnl_yn		[1		+1];		///< 결행조회여부		
	char read_rmn_yn		[1		+1];		///< 잔여석조회여부	
	char alcn_depr_trml_no	[3		+1];		///< 배차출발터미널번호	
	char alcn_arvl_trml_no	[3		+1];		///< 배차도착터미널번호	
	char alcn_rot_no		[4		+1];		///< 배차노선번호		
} stk_tm_readalcn_t, *pstk_tm_readalcn_t;

typedef struct
{
	char drtn_cd					[1		+1]	;	///< 방면코드			
	char alcn_depr_trml_no			[3		+1]	;	///< 배차출발터미널번호		
	char alcn_arvl_trml_no			[3		+1]	;	///< 배차도착터미널번호		
	char alcn_rot_no				[4		+1]	;	///< 배차노선번호			
	char alcn_rot_nm				[100	+1]	;	///< 배차노선명			
	char depr_trml_no				[3		+1]	;	///< 출발터미널번호		
	char arvl_trml_no				[3		+1]	;	///< 도착터미널번호		
	char alcn_depr_dt				[8		+1]	;	///< 배차출발일자			
	char alcn_depr_time				[6		+1]	;	///< 배차출발시각			
	char depr_dt					[8		+1]	;	///< 출발일자			
	char depr_time					[6		+1]	;	///< 출발시각			
	char bus_cls_cd					[1		+1]	;	///< 버스등급코드			
	char cacm_cd					[2		+1]	;	///< 운수사코드			
	char alcn_dvs_cd				[1		+1]	;	///< 배차구분코드			
	char bcnl_dvs_cd				[1		+1]	;	///< 결행구분코드			
	
	char n_rmn_sats_num				[4		+0]	;	///< (Number)잔여좌석수
	char n_nsat_num					[4		+0]	;	///< (Number)무표수				
	char n_tot_sats_num				[4		+0]	;	///< (Number)총좌석수			
	char n_tissu_able_num			[4		+0]	;	///< (Number)발권가능좌석수		

	char chld_sfty_sats_yn			[1		+1]	;	///< 유아시트여부			
	char cacm_nm_prin_yn			[1		+1]	;	///< 운수사명출력여부		
	char bus_cls_prin_yn			[1		+1]	;	///< 버스등급출력여부		
	char depr_time_prin_yn			[1		+1]	;	///< 출발시각출력여부		
	char sats_no_prin_yn			[1		+1]	;	///< 좌석번호출력여부		
	char bus_oprn_dist				[11		+1]	;	///< 운행거리			
	
	char n_take_drtm				[4		+0]	;	///< (Number)소요시간

	char rot_rdhm_no_val			[100	+1]	;	///< 노선승차홈번호값		
	char cty_prmm_dc_yn				[1		+1]	;	///< 시외우등할인여부		
	char sats_stup_sno				[4		+1]	;	///< 좌석설정일련번호		
	char dspr_sats_yn				[1		+1]	;	///< 휠체어좌석여부		
} rtk_tm_readalcn_list_t, *prtk_tm_readalcn_list_t;

typedef struct
{
	char last_drtn_cd				[2		+1]	;	///< (사용안함)최종배차방면번호		
	char last_alcn_depr_trml_no		[3		+1]	;	///< (사용안함)최종배차출발터미널번호	
	char last_alcn_arvl_trml_no		[3		+1]	;	///< (사용안함)최종배차도착터미널번호	
	char last_alcn_rot_no			[4		+1]	;	///< (사용안함)최종배차노선번호		
	char last_depr_dt				[8		+1]	;	///< (사용안함)최종배차일			
} rtk_tm_readalcn_last_list_t, *prtk_tm_readalcn_last_list_t;

typedef struct
{
	char rsp_cd						[6		+1]	;	///< 응답코드
  	char bef_aft_dvs				[1		+1]	;	///< 이전이후 구분
  	char last_alcn_num				[4		+1]	;	///< (사용안함)최종배차노선수
	rtk_tm_readalcn_last_list_t *pLastList;

	char rec_num					[4		+1]	;	///< 정보 수
	rtk_tm_readalcn_list_t *pList;
} rtk_tm_readalcn_t, *prtk_tm_readalcn_t;

//------------------------------
// (현장발권_2). 경유지정보  : CM_ReadThruTrml

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분	
	char req_trml_no				[3		+1]	;	///< 요청터미널번호		
	char req_wnd_no					[2		+1]	;	///< 요청창구번호		
	char req_user_no				[6		+1]	;	///< 요청사용자번호		
	char req_trml_num				[4		+0]	;	///< (Number) 요청터미널 수		
	char trml_no					[3		+1]	;	///< 터미널번호			
	char bef_aft_dvs				[1		+1]	;	///< 이전이후구분		
	char rec_num					[4		+0]	;	///< (Number)정보 수			
	char drtn_cd					[2		+1]	;	///< 방면코드			
	char hspd_cty_dvs_cd			[1		+1]	;	///< 고속시외구분코드		
	char alcn_depr_trml_no			[3		+1]	;	///< 배차출발터미널번호	
	char alcn_arvl_trml_no			[3		+1]	;	///< 배차도착터미널번호	
	char alcn_rot_no				[4		+1]	;	///< 배차노선번호		
	char arvl_trml_no				[3		+1]	;	///< 도착터미널번호		
} stk_tm_readthrutrml_t, *pstk_tm_readthrutrml_t;

typedef struct
{
	char tissu_trml_no				[3		+1]	;	///< 발권터미널번호	
	char depr_trml_no				[3		+1]	;	///< 출발터미널번호	
	char arvl_trml_no				[3		+1]	;	///< 도착터미널번호	
	char alcn_depr_trml_no			[3		+1]	;	///< 배차출발터미널번호
	char alcn_arvl_trml_no			[3		+1]	;	///< 배차도착터미널번호
	char alcn_rot_no				[4		+1]	;	///< 배차노선번호	
	char stt_dt						[8		+1]	;	///< 시작일자		
	char end_dt						[8		+1]	;	///< 종료일자		
	char depr_thru_seq				[22		+1]	;	///< 출발경유순서	
	char arvl_thru_seq				[22		+1]	;	///< 도착경유순서	
	char prmm_sats_sta_val			[100	+1]	;	///< 우등좌석상태값	
	char hspd_sats_sta_val			[100	+1]	;	///< 고속좌석상태값	
	char psrm_sats_sta_val			[100	+1]	;	///< 프리미엄좌석상태값
	char sats_asgt_val				[100	+1]	;	///< 좌석할당값		
	char thru_dist					[22		+1]	;	///< 경유거리		
	char thru_drtm					[22		+1]	;	///< 경유소요시간	
	char cacm_nm_prin_yn			[1		+1]	;	///< 운수사명출력여부	
	char bus_cls_prin_yn			[1		+1]	;	///< 버스등급출력여부	
	char depr_time_prin_yn			[1		+1]	;	///< 출발시각출력여부	
	char sats_no_prin_yn			[1		+1]	;	///< 좌석번호출력여부	
	char drtn_cd					[2		+1]	;	///< 방면코드		

} rtk_tm_readthrutrml_list_t, *prtk_tm_readthrutrml_list_t;

typedef struct
{
	char rsp_cd						[6		+1]	;	///< 응답코드
	char rec_num					[4		+0]	;	///< (Number)정보 수
	rtk_tm_readthrutrml_list_t *pList;

} rtk_tm_readthrutrml_t, *prtk_tm_readthrutrml_t;


//------------------------------
// (현장발권_2). 요금/좌석 상태조회  : CM_ReadSatsFee

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분
	char req_trml_no				[3		+1]	;	///< 요청 터미널번호
	char req_wnd_no					[2		+1]	;	///< 요청 창구번호
	char req_user_no				[6		+1]	;	///< 요청 사용자번호

	char alcn_depr_trml_no			[3		+1];	///< 배차출발터미널번호	
	char alcn_arvl_trml_no			[3		+1];	///< 배차도착터미널번호	
	char alcn_rot_no				[4		+1];	///< 배차노선번호		
	char depr_trml_no				[3		+1];	///< 출발터미널번호	
	char arvl_trml_no				[3		+1];	///< 도착터미널번호	
	char alcn_depr_dt				[8		+1];	///< 배차출발일자		
	char alcn_depr_time				[6		+1];	///< 배차출발시각		
	char depr_dt					[8		+1];	///< 출발일자		
	char depr_time					[6		+1];	///< 출발시각		
	char bus_cls_cd					[1		+1];	///< 버스등급코드		
	char cacm_cd					[2		+1];	///< 운수사코드		
} stk_tm_readsatsfee_t, *pstk_tm_readsatsfee_t;

typedef struct
{
	char tck_knd_cd					[1		+1]	;	///< 티켓종류코드	
	char fee_knd_cd					[1		+1]	;	///< 요금종류코드	
	char n_fee						[4		+0]	;	///< (Number)요금		
	char n_ogn_fee					[4		+0]	;	///< (Number)원요금		
} rtk_tm_readsatsfee_list_t, *prtk_tm_readsatsfee_list_t;

typedef struct
{
	char dc_knd_cd					[2		+1]	;	///< 할인종류코드	
	char dc_mltp_val				[100	+1]	;	///< 할인다중값	
} rtk_tm_readsats_disc_list_t, *prtk_tm_readsats_disc_list_t;

typedef struct
{
	char rsp_cd						[6		+1]	;	///< 응답코드		
	char n_tot_sats_num				[4		+0]	;	///< (Number)총좌석수		
	char n_rmn_sats_num				[4		+0]	;	///< (Number)잔여좌석수		
	char sats_mltp_val				[500	+1]	;	///< 좌석다중값		
	char trml_sats_asgt_val			[300	+1]	;	///< 터미널좌석할당값	
	char rec_num					[4		+0]	;	///< 정보 수		
	char rec_num2					[4		+0]	;	///< 정보 수	2
	rtk_tm_readsatsfee_list_t *pList;
	rtk_tm_readsats_disc_list_t* pDiscList;

} rtk_tm_readsatsfee_t, *prtk_tm_readsatsfee_t;

//------------------------------
// (현장발권_3). 좌석 선점/해제 조회  : TK_PcpySats

typedef struct
{
	char pcpy_no					[14		+1];	///< 선점번호 
	char sats_no					[2		+1];	///< 좌석번호
} stk_tm_pcpysats_list_t, *pstk_tm_pcpysats_list_t;

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분
	char req_trml_no				[3		+1]	;	///< 요청 터미널번호
	char req_wnd_no					[2		+1]	;	///< 요청 창구번호
	char req_user_no				[6		+1]	;	///< 요청 사용자번호

	char req_dvs_cd					[1		+1];	///< 요청구분코드	(Y:선점 N:해제)	
	char alcn_depr_trml_no			[3		+1];	///< 배차출발터미널번호	
	char alcn_arvl_trml_no			[3		+1];	///< 배차도착터미널번호	
	char depr_trml_no				[3		+1];	///< 출발터미널번호	
	char arvl_trml_no				[3		+1];	///< 도착터미널번호	
	char bus_cls_cd					[1		+1];	///< 버스등급코드		
	char cacm_cd					[2		+1];	///< 운수사코드		
	char alcn_depr_dt				[8		+1];	///< 배차출발일자		
	char depr_dt					[8		+1];	///< 출발일자		
	char alcn_depr_time				[6		+1];	///< 배차출발시각		
	char depr_time					[6		+1];	///< 출발시각		
	char rec_num					[4		+0];	///< (Number)좌석수

	stk_tm_pcpysats_list_t List[100];
} stk_tm_pcpysats_t, *pstk_tm_pcpysats_t;

typedef struct
{
	char pcpy_no					[14		+1];	///< 선점번호
	char sats_no					[2		+1];	///< 좌석번호
} rtk_tm_pcpysats_list_t, *prtk_tm_pcpysats_list_t;

typedef struct
{
	char rsp_cd						[6		+1]	;	///< 응답코드		
	char rec_num					[4		+0];	///< (Number)정보 수
	rtk_tm_pcpysats_list_t* pList;
} rtk_tm_pcpysats_t, *prtk_tm_pcpysats_t;

//------------------------------
// (현장발권_4). 승차권 발권(자진발급)  : TK_PubTckCash

typedef struct
{
	char tck_knd_cd					[1		+1];	///< 티켓종류코드		
	char sats_no					[2		+1];	///< 좌석번호		
	char fp_no						[16		+1];	///< 프리패스일련번호	
	char pcpy_no					[14		+1];	///< 선점번호		
	char fee_knd_cd					[1		+1];	///< 요금종류코드		
	char n_tissu_fee				[4		+0];	///< (Number)발권요금		
	char n_ogn_fee					[4		+0];	///< (Number)원요금			
	char dc_knd_cd					[2		+1];	///< 할인종류코드		

} stk_tm_pubtckcash_list_t, *pstk_tm_pubtckcash_list_t;

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분
	char req_trml_no				[3		+1]	;	///< 요청 터미널번호
	char req_wnd_no					[2		+1]	;	///< 요청 창구번호
	char req_user_no				[6		+1]	;	///< 요청 사용자번호

	char req_dvs_cd					[1		+1];	///< 요청구분코드		
	char alcn_depr_trml_no			[3		+1];	///< 배차출발터미널번호	
	char alcn_arvl_trml_no			[3		+1];	///< 배차도착터미널번호	
	char alcn_depr_dt				[8		+1];	///< 배차출발일자		
	char alcn_depr_time				[6		+1];	///< 배차출발시각		
	char depr_trml_no				[3		+1];	///< 출발터미널번호	
	char arvl_trml_no				[3		+1];	///< 도착터미널번호	
	char depr_dt					[8		+1];	///< 출발일자		
	char depr_time					[6		+1];	///< 출발시각		
	char bus_cls_cd					[1		+1];	///< 버스등급코드		
	char cacm_cd					[2		+1];	///< 운수사코드		
	char mbrs_yn					[1		+1];	///< 회원여부		
	char mbrs_no					[17		+1];	///< 회원번호		
	char mrnp_dt					[8		+1];	///< 예약일자		
	char mrnp_time					[6		+1];	///< 예약시각

	char rec_num					[4		+0];	///< (Number)정보 수
	stk_tm_pubtckcash_list_t	List[100];

} stk_tm_pubtckcash_t, *pstk_tm_pubtckcash_t;

typedef struct
{
	char n_tissu_fee				[4		+0];	///< (Number)발권요금		
	char n_ogn_fee					[4		+0];	///< (Number)원요금			
	char sats_no					[2		+1];	///< 좌석번호		
	char inhr_no					[8		+1];	///< 고유번호		
	char invs_no					[6		+1];	///< 심사번호		
	char tissu_sno					[4		+1];	///< 발권일련번호		
	char tissu_no					[14		+1];	///< 발권번호		
	char csrc_aprv_no				[9		+1];	///< 현금영수증승인번호	
	char csrc_rgt_no				[9		+1];	///< 현금영수증등록번호	
	char tck_knd_cd					[1		+1];	///< 티켓종류코드		
	char dc_knd_cd					[2		+1];	///< 할인종류코드		
	char exch_knd_cd				[2		+1];	///< 할증종류코드		

} rtk_tm_pubtckcash_list_t, *prtk_tm_pubtckcash_list_t;

typedef struct
{
	char rsp_cd						[6		+1];	///< 응답코드		
	char cacm_nm_prin_yn			[1		+1];	///< 운수사명출력여부	
	char bus_cls_prin_yn			[1		+1];	///< 버스등급출력여부	
	char depr_time_prin_yn			[1		+1];	///< 출발시각출력여부	
	char sats_no_prin_yn			[1		+1];	///< 좌석번호출력여부	
	char tissu_dt					[8		+1];	///< 발권일자		
	char tissu_trml_no				[3		+1];	///< 발권터미널번호	
	char tissu_wnd_no				[2		+1];	///< 발권창구번호		
	char tissu_time					[6		+1];	///< 발권시각		
	char user_key_val				[100	+1];	///< 사용자키값		
	char rec_num					[4		+0];	///< (Number)정보 수
	rtk_tm_pubtckcash_list_t* pList;

} rtk_tm_pubtckcash_t, *prtk_tm_pubtckcash_t;

//------------------------------
// (현장발권_5). 승차권 발권(카드)  : TK_PubTckCard

typedef struct
{
	char tck_knd_cd					[1		+1];	///< 티켓종류코드	
	char sats_no					[2		+1];	///< 좌석번호	
	char pcpy_no					[14		+1];	///< 선점번호	
	char fee_knd_cd					[1		+1];	///< 요금종류코드	
	char n_tissu_fee				[4		+0];	///< (Number)발권요금	
	char n_ogn_fee					[4		+0];	///< (Number)원요금		
	char dc_knd_cd					[2		+1];	///< 할인종류코드	

} stk_tm_pubtckcard_list_t, *pstk_tm_pubtckcard_list_t;

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분
	char req_trml_no				[3		+1]	;	///< 요청 터미널번호
	char req_wnd_no					[2		+1]	;	///< 요청 창구번호
	char req_user_no				[6		+1]	;	///< 요청 사용자번호

	char alcn_depr_trml_no			[3		+1];	///< 배차출발터미널번호		
	char alcn_arvl_trml_no			[3		+1];	///< 배차도착터미널번호		
	char alcn_depr_dt				[8		+1];	///< 배차출발일자			
	char alcn_depr_time				[6		+1];	///< 배차출발시각			
	char depr_trml_no				[3		+1];	///< 출발터미널번호		
	char arvl_trml_no				[3		+1];	///< 도착터미널번호		
	char depr_dt					[8		+1];	///< 출발일자			
	char depr_time					[6		+1];	///< 출발시각			
	char bus_cls_cd					[1		+1];	///< 버스등급코드			
	char cacm_cd					[2		+1];	///< 운수사코드			
	char card_track_dta				[40		+1];	///< 카드 트랙 데이터		
	char spad_dta					[1536	+1];	///< 사인데이터			
	char n_spad_dta_len				[4		+0];	///< (Number)사인데이터 길이		
	char req_dvs_cd					[1		+1];	///< 요청구분코드			
	char rfid_card_dvs				[1		+1];	///< 후불교통/신용카드 구분	
	char rfid_dongle_dta			[10		+1];	///< 후불교통 동글 데이터	
	char mbrs_yn					[1		+1];	///< 회원여부			
	char mbrs_no					[17		+1];	///< 회원번호			
	char mrnp_dt					[8		+1];	///< 예약일자			
	char mrnp_time					[6		+1];	///< 예약시각			
	char n_mip_mm_num				[4		+1];	///< (Number)할부개월수			
	char trd_dvs_cd					[1		+1];	///< 거래구분코드			
	char enc_dta					[128	+1];	///< enc_dta			
	char emv_dta					[1000	+1];	///< emv_dta			

	char rec_num					[4		+0];	///< (Number)정보 수
	stk_tm_pubtckcard_list_t	List[100];

} stk_tm_pubtckcard_t, *pstk_tm_pubtckcard_t;

typedef struct
{
	char n_tissu_fee				[4		+0];	///< (number)발권요금	
	char n_ogn_fee					[4		+0];	///< (number)원요금		
	char sats_no					[2		+1];	///< 좌석번호	
	char inhr_no					[8		+1];	///< 고유번호	
	char invs_no					[6		+1];	///< 심사번호	
	char tissu_sno					[4		+1];	///< 발권일련번호	
	char tck_knd_cd					[1		+1];	///< 티켓종류코드	
	char dc_knd_cd					[2		+1];	///< 할인종류코드	
	char exch_knd_cd				[2		+1];	///< 할증종류코드	
	
} rtk_tm_pubtckcard_list_t, *prtk_tm_pubtckcard_list_t;

typedef struct
{
	char rsp_cd						[6		+1];	///< 응답코드		
	char card_no					[100	+1];	///< 카드번호			
	char cacm_nm_prin_yn			[1		+1];	///< 운수사명출력여부		
	char bus_cls_prin_yn			[1		+1];	///< 버스등급출력여부		
	char depr_time_prin_yn			[1		+1];	///< 출발시각출력여부		
	char sats_no_prin_yn			[1		+1];	///< 좌석번호출력여부		
	char tissu_dt					[8		+1];	///< 발권일자			
	char tissu_trml_no				[3		+1];	///< 발권터미널번호		
	char tissu_wnd_no				[2		+1];	///< 발권창구번호			
	char tissu_time					[6		+1];	///< 발권시각			
	char tissu_no					[14		+1];	///< 발권번호			
	char card_aprv_no				[100	+1];	///< 카드승인번호			
	char n_aprv_amt					[4		+0];	///< (Number)승인금액			
	char frc_cmrt					[3		+1];	///< 가맹점수수료율		
	char able_point					[64		+1];	///< 가용포인트(기프트카드)
	
	char rec_num					[4		+0];	///< (Number)정보 수
	rtk_tm_pubtckcard_list_t* pList;

} rtk_tm_pubtckcard_t, *prtk_tm_pubtckcard_t;

//------------------------------
// (현장발권_6). 승차권 발권(카드_KTC)  : TK_KtcPubCard

typedef struct
{
	char tck_knd_cd					[1		+1];	///< 티켓종류코드
	char sats_no					[2		+1];	///< 좌석번호	
	char pcpy_no					[14		+1];	///< 선점번호	
	char fee_knd_cd					[1		+1];	///< 요금종류코드
	char n_tissu_fee				[4		+0];	///< (Number)발권요금	
	char n_ogn_fee					[4		+0];	///< (Number)원요금		
	char dc_knd_cd					[2		+1];	///< 할인종류코드

} stk_tm_pubtckcard_ktc_list_t, *pstk_tm_pubtckcard_ktc_list_t;

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분
	char req_trml_no				[3		+1]	;	///< 요청 터미널번호
	char req_wnd_no					[2		+1]	;	///< 요청 창구번호
	char req_user_no				[6		+1]	;	///< 요청 사용자번호

	char alcn_depr_trml_no			[3		+1];	///< 배차출발터미널번호		
	char alcn_arvl_trml_no			[3		+1];	///< 배차도착터미널번호		
	char alcn_depr_dt				[8		+1];	///< 배차출발일자			
	char alcn_depr_time				[6		+1];	///< 배차출발시각			
	char depr_trml_no				[3		+1];	///< 출발터미널번호		
	char arvl_trml_no				[3		+1];	///< 도착터미널번호		
	char depr_dt					[8		+1];	///< 출발일자			
	char depr_time					[6		+1];	///< 출발시각			
	char bus_cls_cd					[1		+1];	///< 버스등급코드			
	char cacm_cd					[2		+1];	///< 운수사코드			
	char card_track_dta				[40		+1];	///< 카드 트랙 데이터		
	char spad_dta					[1536	+1];	///< 사인데이터			
	char n_spad_dta_len				[4		+0];	///< (Number)사인데이터 길이		
	char req_dvs_cd					[1		+1];	///< 요청구분코드			
	char rfid_card_dvs				[1		+1];	///< 후불교통/신용카드 구분	
	char rfid_dongle_dta			[10		+1];	///< 후불교통 동글 데이터	
	char mbrs_yn					[1		+1];	///< 회원여부			
	char mbrs_no					[17		+1];	///< 회원번호			
	char mrnp_dt					[8		+1];	///< 예약일자			
	char mrnp_time					[6		+1];	///< 예약시각			
	char n_mip_mm_num				[4		+0];	///< (Number)할부개월수			
	char trd_dvs_cd					[1		+1];	///< 거래구분코드			
	char enc_dta					[128	+1];	///< enc_dta			
	char emv_dta					[1000	+1];	///< emv_dta			

	char rec_num					[4		+0];	///< (Number)정보 수
	stk_tm_pubtckcard_ktc_list_t	List[100];

} stk_tm_pubtckcard_ktc_t, *pstk_tm_pubtckcard_ktc_t;

typedef struct
{
	char n_tissu_fee				[4		+0];	///< (Number)발권요금	
	char n_ogn_fee					[4		+0];	///< (Number)원요금		
	char sats_no					[2		+1];	///< 좌석번호	
	char inhr_no					[8		+1];	///< 고유번호	
	char invs_no					[6		+1];	///< 심사번호	
	char tissu_sno					[4		+1];	///< 발권일련번호	
	char tck_knd_cd					[1		+1];	///< 티켓종류코드	
	char dc_knd_cd					[2		+1];	///< 할인종류코드	
	char exch_knd_cd				[2		+1];	///< 할증종류코드	
	
} rtk_tm_pubtckcard_ktc_list_t, *prtk_tm_pubtckcard_ktc_list_t;

typedef struct
{
	char rsp_cd						[6		+1];	///< 응답코드		

	char card_no					[100	+1];	///< 카드번호			
	char cacm_nm_prin_yn			[1		+1];	///< 운수사명출력여부		
	char bus_cls_prin_yn			[1		+1];	///< 버스등급출력여부		
	char depr_time_prin_yn			[1		+1];	///< 출발시각출력여부		
	char sats_no_prin_yn			[1		+1];	///< 좌석번호출력여부		
	char tissu_dt					[8		+1];	///< 발권일자			
	char tissu_trml_no				[3		+1];	///< 발권터미널번호		
	char tissu_wnd_no				[2		+1];	///< 발권창구번호			
	char tissu_time					[6		+1];	///< 발권시각			
	char tissu_no					[14		+1];	///< 발권번호			
	char card_aprv_no				[100	+1];	///< 카드승인번호			
	char n_aprv_amt					[4		+0];	///< (Number)승인금액			
	char frc_cmrt					[3		+1];	///< 가맹점수수료율		
	char able_point					[64		+1];	///< 가용포인트(기프트카드)
	
	char rec_num					[4		+0];	///< (Number)정보 수
	rtk_tm_pubtckcard_ktc_list_t* pList;

} rtk_tm_pubtckcard_ktc_t, *prtk_tm_pubtckcard_ktc_t;

//------------------------------
// (현장발권_7). 승차권 발권(현금영수증)  : TK_PubTckCsrc

typedef struct
{
	char tck_knd_cd					[1		+1];	///< 티켓종류코드	
	char sats_no					[2		+1];	///< 좌석번호	
	char pcpy_no					[14		+1];	///< 선점번호	
	char fee_knd_cd					[1		+1];	///< 요금종류코드	
	char n_tissu_fee				[4		+0];	///< (Number)발권요금	
	char n_ogn_fee					[4		+0];	///< (Number)원요금		
	char dc_knd_cd					[2		+1];	///< 할인종류코드	

} stk_tm_pubtckcsrc_list_t, *pstk_tm_pubtckcsrc_list_t;

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분
	char req_trml_no				[3		+1]	;	///< 요청 터미널번호
	char req_wnd_no					[2		+1]	;	///< 요청 창구번호
	char req_user_no				[6		+1]	;	///< 요청 사용자번호

	char user_dvs_cd				[1		+1];	///< 사용자구분코드	
	char alcn_depr_trml_no			[3		+1];	///< 배차출발터미널번호	
	char alcn_arvl_trml_no			[3		+1];	///< 배차도착터미널번호	
	char alcn_depr_dt				[8		+1];	///< 배차출발일자		
	char alcn_depr_time				[6		+1];	///< 배차출발시각		
	char depr_trml_no				[3		+1];	///< 출발터미널번호	
	char arvl_trml_no				[3		+1];	///< 도착터미널번호	
	char depr_dt					[8		+1];	///< 출발일자		
	char depr_time					[6		+1];	///< 출발시각		
	char bus_cls_cd					[1		+1];	///< 버스등급코드		
	char cacm_cd					[2		+1];	///< 운수사코드		
	char user_key_val				[100	+1];	///< 사용자키값		
	char mbrs_yn					[1		+1];	///< 회원여부		
	char mbrs_no					[17		+1];	///< 회원번호		
	char mrnp_dt					[8		+1];	///< 예약일자		
	char mrnp_time					[6		+1];	///< 예약시각		
	char trd_dvs_cd					[1		+1];	///< 거래구분코드		
	char enc_dta					[128	+1];	///< enc_dta		
	char emv_dta					[1000	+1];	///< emv_dta		

	char rec_num					[4		+0];	///< unsigned정보 수
	stk_tm_pubtckcsrc_list_t	List[100];

} stk_tm_pubtckcsrc_t, *pstk_tm_pubtckcsrc_t;

typedef struct
{
	char n_tissu_fee				[4		+0];	///< (Number)발권요금		
	char n_ogn_fee					[4		+0];	///< (Number)원요금			
	char sats_no					[2		+1];	///< 좌석번호		
	char inhr_no					[8		+1];	///< 고유번호		
	char invs_no					[6		+1];	///< 심사번호		
	char tissu_sno					[4		+1];	///< 발권일련번호		
	char csrc_aprv_no				[9		+1];	///< 현금영수증승인번호	
	char csrc_rgt_no				[9		+1];	///< 현금영수증등록번호	
	char n_cash_recp_amt			[4		+0];	///< (Number)현금영수증금액	
	char tissu_no					[14		+1];	///< 발권번호		
	char tck_knd_cd					[1		+1];	///< 티켓종류코드		
	char dc_knd_cd					[2		+1];	///< 할인종류코드		
	char exch_knd_cd				[2		+1];	///< 할증종류코드		
	
} rtk_tm_pubtckcsrc_list_t, *prtk_tm_pubtckcsrc_list_t;

typedef struct
{
	char rsp_cd						[6		+1];	///< 응답코드		

	char cacm_nm_prin_yn			[1		+1];	///< 운수사명출력여부	
	char bus_cls_prin_yn			[1		+1];	///< 버스등급출력여부	
	char depr_time_prin_yn			[1		+1];	///< 출발시각출력여부	
	char sats_no_prin_yn			[1		+1];	///< 좌석번호출력여부	
	char tissu_dt					[8		+1];	///< 발권일자		
	char tissu_trml_no				[3		+1];	///< 발권터미널번호	
	char tissu_wnd_no				[2		+1];	///< 발권창구번호		
	char tissu_time					[6		+1];	///< 발권시각		
	char user_key_val				[100	+1];	///< 사용자키값		
	char user_dvs_cd				[1		+1];	///< 사용자구분코드	

	char rec_num					[4		+0];	///< (Number)정보 수
	rtk_tm_pubtckcsrc_list_t* pList;

} rtk_tm_pubtckcsrc_t, *prtk_tm_pubtckcsrc_t;

//------------------------------
// (현장발권_8). 승차권 발권(현금영수증_Ktc)  : TK_KtcPubCsrc

typedef struct
{
	char tck_knd_cd					[1		+1];	///< 티켓종류코드	
	char sats_no					[2		+1];	///< 좌석번호	
	char pcpy_no					[14		+1];	///< 선점번호	
	char fee_knd_cd					[1		+1];	///< 요금종류코드	
	char n_tissu_fee				[4		+0];	///< (Number)발권요금	
	char n_ogn_fee					[4		+0];	///< (Number)원요금		
	char dc_knd_cd					[2		+1];	///< 할인종류코드	

} stk_tm_pubtckcsrc_ktc_list_t, *pstk_tm_pubtckcsrc_ktc_list_t;

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분
	char req_trml_no				[3		+1]	;	///< 요청 터미널번호
	char req_wnd_no					[2		+1]	;	///< 요청 창구번호
	char req_user_no				[6		+1]	;	///< 요청 사용자번호

	char user_dvs_cd				[1		+1];	///< 사용자구분코드	
	char alcn_depr_trml_no			[3		+1];	///< 배차출발터미널번호	
	char alcn_arvl_trml_no			[3		+1];	///< 배차도착터미널번호	
	char alcn_depr_dt				[8		+1];	///< 배차출발일자		
	char alcn_depr_time				[6		+1];	///< 배차출발시각		
	char depr_trml_no				[3		+1];	///< 출발터미널번호	
	char arvl_trml_no				[3		+1];	///< 도착터미널번호	
	char depr_dt					[8		+1];	///< 출발일자		
	char depr_time					[6		+1];	///< 출발시각		
	char bus_cls_cd					[1		+1];	///< 버스등급코드		
	char cacm_cd					[2		+1];	///< 운수사코드		
	char user_key_val				[100	+1];	///< 사용자키값		
	char mbrs_yn					[1		+1];	///< 회원여부		
	char mbrs_no					[17		+1];	///< 회원번호		
	char mrnp_dt					[8		+1];	///< 예약일자		
	char mrnp_time					[6		+1];	///< 예약시각		
	char trd_dvs_cd					[1		+1];	///< 거래구분코드		
	char enc_dta					[128	+1];	///< enc_dta		
	char emv_dta					[1000	+1];	///< emv_dta		

	char rec_num					[4		+0];	///< (Number)정보 수
	stk_tm_pubtckcsrc_ktc_list_t	List[100];

} stk_tm_pubtckcsrc_ktc_t, *pstk_tm_pubtckcsrc_ktc_t;

typedef struct
{
	char n_tissu_fee				[4		+0];	///< (Number)발권요금		
	char n_ogn_fee					[4		+0];	///< (Number)원요금			
	char sats_no					[2		+1];	///< 좌석번호		
	char inhr_no					[8		+1];	///< 고유번호		
	char invs_no					[6		+1];	///< 심사번호		
	char tissu_sno					[4		+1];	///< 발권일련번호		
	char csrc_aprv_no				[9		+1];	///< 현금영수증승인번호	
	char csrc_rgt_no				[9		+1];	///< 현금영수증등록번호	
	char n_cash_recp_amt			[4		+0];	///< 현금영수증금액	
	char tissu_no					[14		+1];	///< 발권번호		
	char tck_knd_cd					[1		+1];	///< 티켓종류코드		
	char dc_knd_cd					[2		+1];	///< 할인종류코드		
	char exch_knd_cd				[2		+1];	///< 할증종류코드		
	
} rtk_tm_pubtckcsrc_ktc_list_t, *prtk_tm_pubtckcsrc_ktc_list_t;

typedef struct
{
	char rsp_cd						[6		+1];	///< 응답코드		

	char cacm_nm_prin_yn			[1		+1];	///< 운수사명출력여부	
	char bus_cls_prin_yn			[1		+1];	///< 버스등급출력여부	
	char depr_time_prin_yn			[1		+1];	///< 출발시각출력여부	
	char sats_no_prin_yn			[1		+1];	///< 좌석번호출력여부	
	char tissu_dt					[8		+1];	///< 발권일자		
	char tissu_trml_no				[3		+1];	///< 발권터미널번호	
	char tissu_wnd_no				[2		+1];	///< 발권창구번호		
	char tissu_time					[6		+1];	///< 발권시각		
	char user_key_val				[100	+1];	///< 사용자키값		
	char user_dvs_cd				[1		+1];	///< 사용자구분코드	

	char rec_num					[4		+0];	///< (Number)정보 수
	rtk_tm_pubtckcsrc_ktc_list_t* pList;

} rtk_tm_pubtckcsrc_ktc_t, *prtk_tm_pubtckcsrc_ktc_t;

//------------------------------
// (현장발권_9). 승차권 발권(부가상품권)  : TK_KtcPubPrd

typedef struct
{
	char sats_no					[2		+1];	///< 좌석번호
	char pcpy_no					[4		+1];	///< 선점번호
} stk_tm_pubtckprd_list_t, *pstk_tm_pubtckprd_list_t;

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분
	char req_trml_no				[3		+1]	;	///< 요청 터미널번호
	char req_wnd_no					[2		+1]	;	///< 요청 창구번호
	char req_user_no				[6		+1]	;	///< 요청 사용자번호

	char alcn_depr_trml_no			[3		+1];	///< 배차출발터미널번호	
	char alcn_arvl_trml_no			[3		+1];	///< 배차도착터미널번호	
	char alcn_depr_dt				[8		+1];	///< 배차출발일자		
	char alcn_depr_time				[6		+1];	///< 배차출발시각		
	char depr_trml_no				[3		+1];	///< 출발터미널번호	
	char arvl_trml_no				[3		+1];	///< 도착터미널번호	
	char depr_dt					[8		+1];	///< 출발일자		
	char depr_time					[6		+1];	///< 출발시각		
	char bus_cls_cd					[1		+1];	///< 버스등급코드		
	char cacm_cd					[2		+1];	///< 운수사코드		
	char adtn_cpn_no				[16		+1];	///< 부가상품권번호	
	char adtn_prd_auth_no			[44		+1];	///< 부가상품인증번호	

	char rec_num					[4		+0];	///< (Number)정보 수
	stk_tm_pubtckprd_list_t		List[100];

} stk_tm_pubtckprd_t, *pstk_tm_pubtckprd_t;

typedef struct
{
	char n_tissu_fee				[4		+0];	///< (number) 발권요금	
	char n_ogn_fee					[4		+0];	///< (number) 원요금		
	char sats_no					[2		+1];	///< 좌석번호	
	char inhr_no					[8		+1];	///< 고유번호	
	char invs_no					[6		+1];	///< 심사번호	
	char tissu_sno					[4		+1];	///< 발권일련번호
	char tck_knd_cd					[1		+1];	///< 티켓종류코드

} rtk_tm_pubtckprd_list_t, *prtk_tm_pubtckprd_list_t;

typedef struct
{
	char rsp_cd						[6		+1];	///< 응답코드		

	char adtn_cpn_no				[16		+1]	;	///< 부가상품권번호	
	char cacm_nm_prin_yn			[1		+1]	;	///< 운수사명출력여부	
	char bus_cls_prin_yn			[1		+1]	;	///< 버스등급출력여부	
	char depr_time_prin_yn			[1		+1]	;	///< 출발시각출력여부	
	char sats_no_prin_yn			[1		+1]	;	///< 좌석번호출력여부	
	char tissu_dt					[8		+1]	;	///< 발권일자		
	char tissu_trml_no				[3		+1]	;	///< 발권터미널번호	
	char tissu_wnd_no				[2		+1]	;	///< 발권창구번호		
	char tissu_time					[6		+1]	;	///< 발권시각		
	char tissu_no					[14		+1]	;	///< 발권번호		

	char rec_num					[4		+0];	///< (Number)정보 수
	rtk_tm_pubtckprd_list_t*		pList;

} rtk_tm_pubtckprd_t, *prtk_tm_pubtckprd_t;


//------------------------------
// (예매발권_1). 예매조회  : TK_ReadMrs

#define MAX_TM_MRS_INFO		40

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분
	char req_trml_no				[3		+1]	;	///< 요청 터미널번호
	char req_wnd_no					[2		+1]	;	///< 요청 창구번호
	char req_user_no				[6		+1]	;	///< 요청 사용자번호

	char req_dvs					[1		+1];	///< 요청구분		
	char req_dvs_cd					[1		+1];	///< 요청구분코드		
	char card_no					[100	+1];	///< 카드번호		
	char adtn_cpn_no				[16		+1];	///< 부가상품권번호	
	char adtn_prd_auth_no			[44		+1];	///< 부가상품인증번호	
	char mrsp_mbph_no				[44		+1];	///< 예매자휴대폰번호	
	char mrsp_brdt					[8		+1];	///< 예매자생년월일	
	char mrs_mrnp_no				[14		+1];	///< 예매예약번호		
	char tissu_dt					[8		+1];	///< 발권일자		
	char tissu_trml_no				[3		+1];	///< 발권터미널번호	
	char tissu_wnd_no				[2		+1];	///< 발권창구번호		
	char tissu_sno					[4		+1];	///< 발권일련번호		
	char lng_cd						[2		+1];	///< 언어코드		
	char depr_dt					[8		+1];	///< 출발일			

} stk_tm_read_mrs_t, *pstk_tm_read_mrs_t;


typedef struct
{
	char tck_knd_cd					[1		+1];	///< 티켓종류코드	 
	char sats_no					[2		+1];	///< 좌석번호	 
	char tissu_amt					[4		+0];	///< (Number) 발권금액	 
	char ry_sta_cd					[1		+1];	///< 환불상태코드	 
	char dc_knd_cd					[2		+1];	///< 할인종류코드	 

} rtk_tm_read_mrs_info_t, *prtk_tm_read_mrs_info_t;

typedef struct
{
	char mrs_mrnp_no				[14		+1];	///< 예매예약번호		 
	char mrs_mrnp_sno				[2		+1];	///< 예매예약일련번호	 
	char mrs_mrnp_dt				[8		+1];	///< 예매예약일자		 
	char mrs_mrnp_time				[6		+1];	///< 예매예약시각		 
	char alcn_depr_trml_no			[3		+1];	///< 배차출발터미널번호	 
	char alcn_arvl_trml_no			[3		+1];	///< 배차도착터미널번호	 
	char depr_trml_no				[3		+1];	///< 출발터미널번호	 
	char arvl_trml_no				[3		+1];	///< 도착터미널번호	 
	char bus_cls_cd					[1		+1];	///< 버스등급코드		 
	char cacm_cd					[2		+1];	///< 운수사코드		 
	char hspd_cty_dvs_cd			[1		+1];	///< 고속시외구분코드	 
	char alcn_depr_time				[6		+1];	///< 배차출발시각		 
	char depr_dt					[8		+1];	///< 출발일자		 
	char depr_time					[6		+1];	///< 출발시각		 
	char mrs_amt					[4		+0];	///< (Number) 예매금액		 
	char card_no					[100	+1];	///< 카드번호		 
	char adtn_cpn_no				[16		+1];	///< 부가상품권번호	 
	char adtn_prd_auth_no			[44		+1];	///< 부가상품인증번호	 
	char rot_rdhm_no_val			[100	+1];	///< 노선승차홈번호값	 
	char buy_cmpy_cd				[2		+1];	///< 매입회사코드		 
	char tck_knd_string				[512	+1];	///< 티켓종류문자열	 
	char sats_no_string				[512	+1];	///< 좌석번호문자열	 
	char tissu_chnl_dvs_cd			[1		+1];	///< 발권채널구분코드	 
	char pub_chnl_dvs_cd			[1		+1];	///< 발행채널구분코드	 
	char pyn_dvs_cd					[1		+1];	///< 지불구분코드		 
	char tissu_sta_cd				[1		+1];	///< 발권상태코드		 
	char pyn_dtl_cd					[1		+1];	///< 지불상세코드 **		 

	char mrs_num					[4		+0];	///<  (Number) 예매매수
	rtk_tm_read_mrs_info_t			mrs_info[MAX_TM_MRS_INFO];

} rtk_tm_read_mrs_list_t, *prtk_tm_read_mrs_list_t;

typedef struct
{
	char rsp_cd						[6		+1];	///< 응답코드		

	char rec_num					[4		+0];	///< (Number)정보 수
	rtk_tm_read_mrs_list_t*		pList;

} rtk_tm_read_mrs_t, *prtk_tm_read_mrs_t;

//------------------------------
// (예매발권_2). KTC 예매조회  : TK_KtcReadMrs

#define MAX_TM_MRS_INFO		40

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분
	char req_trml_no				[3		+1]	;	///< 요청 터미널번호
	char req_wnd_no					[2		+1]	;	///< 요청 창구번호
	char req_user_no				[6		+1]	;	///< 요청 사용자번호

	char req_dvs					[1		+1];	///< 요청구분		
	char req_dvs_cd					[1		+1];	///< 요청구분코드		
	char trd_dvs_cd					[1		+1];	///< 거래구분코드		
	char enc_dta					[128	+1];	///< enc_dta		
	char emv_dta					[1000	+1];	///< emv_dta		
	char adtn_cpn_no				[16		+1];	///< 부가상품권번호	
	char adtn_prd_auth_no			[44		+1];	///< 부가상품인증번호	
	char mrsp_mbph_no				[44		+1];	///< 예매자휴대폰번호	
	char mrsp_brdt					[8		+1];	///< 예매자생년월일	
	char mrs_mrnp_no				[14		+1];	///< 예매예약번호		
	char tissu_dt					[8		+1];	///< 발권일자		
	char tissu_trml_no				[3		+1];	///< 발권터미널번호	
	char tissu_wnd_no				[2		+1];	///< 발권창구번호		
	char tissu_sno					[4		+1];	///< 발권일련번호		
	char lng_cd						[2		+1];	///< 언어코드		

} stk_tm_read_mrs_ktc_t, *pstk_tm_read_mrs_ktc_t;


typedef struct
{
	char tck_knd_cd					[1		+1];	///< 티켓종류코드	 
	char sats_no					[2		+1];	///< 좌석번호	 
	char tissu_amt					[4		+0];	///< (Number) 발권금액	 
	char ry_sta_cd					[1		+1];	///< 환불상태코드	 
	char dc_knd_cd					[2		+1];	///< 할인종류코드	 

} rtk_tm_read_mrs_ktc_info_t, *prtk_tm_read_mrs_ktc_info_t;

typedef struct
{
	char mrs_mrnp_no				[14		+1];	///< 예매예약번호		
	char mrs_mrnp_sno				[2		+1];	///< 예매예약일련번호	
	char mrs_mrnp_dt				[8		+1];	///< 예매예약일자		
	char mrs_mrnp_time				[6		+1];	///< 예매예약시각		
	char cash_recp_amt				[4		+0];	///< (Number) 현금영수증금액	
	char tissu_no					[14		+1];	///< 발권번호		
	char tck_knd_cd					[1		+1];	///< 티켓종류코드		
	char alcn_depr_trml_no			[3		+1];	///< 배차출발터미널번호	
	char alcn_arvl_trml_no			[3		+1];	///< 배차도착터미널번호	
	char depr_trml_no				[3		+1];	///< 출발터미널번호	
	char arvl_trml_no				[3		+1];	///< 도착터미널번호	
	char bus_cls_cd					[1		+1];	///< 버스등급코드		
	char cacm_cd					[2		+1];	///< 운수사코드		
	char hspd_cty_dvs_cd			[1		+1];	///< 고속시외구분코드	
	char alcn_depr_time				[6		+1];	///< 배차출발시각		
	char depr_dt					[8		+1];	///< 출발일자		
	char depr_time					[6		+1];	///< 출발시각		
	char mrs_amt					[4		+0];	///< (Number) 예매금액		
	char card_no					[100	+1];	///< 카드번호		
	char adtn_cpn_no				[16		+1];	///< 부가상품권번호	
	char adtn_prd_auth_no			[44		+1];	///< 부가상품인증번호	
	char rot_rdhm_no_val			[100	+1];	///< 노선승차홈번호값	
	char buy_cmpy_cd				[2		+1];	///< 매입회사코드		
	char tck_knd_string				[512	+1];	///< 티켓종류문자열	
	char sats_no_string				[512	+1];	///< 좌석번호문자열	
	char tissu_chnl_dvs_cd			[1		+1];	///< 발권채널구분코드	
	char pub_chnl_dvs_cd			[1		+1];	///< 발행채널구분코드	
	char pyn_dvs_cd					[1		+1];	///< 지불구분코드		
	char tissu_sta_cd				[1		+1];	///< 발권상태코드	
	char pyn_dtl_cd					[1		+1];	///< 지불상세코드

	char mrs_num					[4		+0];	///< (Number)예매매수
	rtk_tm_read_mrs_ktc_info_t		mrs_info[MAX_TM_MRS_INFO];

} rtk_tm_read_mrs_ktc_list_t, *prtk_tm_read_mrs_ktc_list_t;

typedef struct
{
	char rsp_cd						[6		+1];	///< 응답코드		

	char rec_num					[4		+0];	///< (Number)정보 수
	rtk_tm_read_mrs_ktc_list_t*		pList;

} rtk_tm_read_mrs_ktc_t, *prtk_tm_read_mrs_ktc_t;


//------------------------------
// (예매발권_3). 예매발권  : TK_PubMrs

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분
	char req_trml_no				[3		+1]	;	///< 요청 터미널번호
	char req_wnd_no					[2		+1]	;	///< 요청 창구번호
	char req_user_no				[6		+1]	;	///< 요청 사용자번호

	char req_dvs_cd					[1		+1];	///< 요청구분코드
	char mrs_mrnp_no				[14		+1];	///< 예매 예약번호
	char mrs_mrnp_sno				[2		+1];	///< 예매 예약일련번호

} stk_tm_pub_mrs_t, *pstk_tm_pub_mrs_t;

typedef struct
{
	char mrs_mrnp_no				[14		+1];	///< 예매예약번호	
	char mrs_mrnp_sno				[2		+1];	///< 예매예약일련번호	
	char depr_dt					[8		+1];	///< 출발일자		
	char depr_time					[6		+1];	///< 출발시각		
	char alcn_depr_trml_no			[3		+1];	///< 배차출발터미널번호
	char alcn_arvl_trml_no			[3		+1];	///< 배차도착터미널번호
	char depr_trml_no				[3		+1];	///< 출발터미널번호	
	char arvl_trml_no				[3		+1];	///< 도착터미널번호	
	char alcn_rot_no				[4		+1];	///< 배차노선번호	
	char bus_cls_cd					[1		+1];	///< 버스등급코드	
	char cacm_cd					[2		+1];	///< 운수사코드		
	char rot_rdhm_no_val			[100	+1];	///< 노선승차홈번호값	
	char bus_oprn_dist				[11		+1];	///< 운행거리		
	char take_drtm					[4		+0];	///< (Number)소요시간		
	char cacm_nm_prin_yn			[1		+1];	///< 운수사명출력여부	
	char bus_cls_prin_yn			[1		+1];	///< 버스등급출력여부	
	char depr_time_prin_yn			[1		+1];	///< 출발시각출력여부	
	char sats_no_prin_yn			[1		+1];	///< 좌석번호출력여부	
	char alcn_dvs_cd				[1		+1];	///< 배차구분코드	
	char tissu_dt					[8		+1];	///< 발권일자		
	char tissu_time					[6		+1];	///< 발권시각		
	char tissu_trml_no				[3		+1];	///< 발권터미널번호	
	char tissu_wnd_no				[2		+1];	///< 발권창구번호	
	char pub_user_no				[6		+1];	///< 발행사용자번호	
	char tissu_sno					[4		+1];	///< 발권일련번호	
	char inhr_no					[8		+1];	///< 고유번호		
	char invs_no					[6		+1];	///< 심사번호		
	char sats_no					[2		+1];	///< 좌석번호		
	char tck_knd_cd					[1		+1];	///< 티켓종류코드	
	char fee_knd_cd					[1		+1];	///< 요금종류코드	
	char tissu_fee					[4		+0];	///< (Number)발권요금		
	char ogn_fee					[4		+0];	///< (Number)원요금		

} rtk_tm_pub_mrs_list_t, *prtk_tm_pub_mrs_list_t;

typedef struct
{
	char rsp_cd						[6		+1];	///< 응답코드		

	char pyn_dtl_cd					[1		+1];	///< 지불상세코드
	char card_no					[100	+1];	///< 카드번호	
	char card_aprv_no				[100	+1];	///< 카드승인번호
	char aprv_amt					[4		+0];	///< 승인금액	
	char frc_cmrt					[3		+1];	///< 가맹점수수료율

	char rec_num					[4		+0];	///< (Number)정보 수
	rtk_tm_pub_mrs_list_t		*pList;

} rtk_tm_pub_mrs_t, *prtk_tm_pub_mrs_t;

//------------------------------
// (예매발권_4). 예매발권  : TK_RPubHtck

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분
	char req_trml_no				[3		+1]	;	///< 요청 터미널번호
	char req_wnd_no					[2		+1]	;	///< 요청 창구번호
	char req_user_no				[6		+1]	;	///< 요청 사용자번호

	char depr_trml_no				[3		+1];	///< 요청구분코드
	char mrs_mrnp_no				[14		+1];	///< 예매 예약번호
	char mrs_mrnp_sno				[2		+1];	///< 예매 예약일련번호

} stk_tm_pub_mrs_htck_t, *pstk_tm_pub_mrs_htck_t;

typedef struct
{
	char bus_oprn_dist				[11		+1];	///< 운행거리			
	char rot_rdhm_no_val			[100	+1];	///< 노선승차홈번호값		
	char cacm_nm_prin_yn			[1		+1];	///< 운수사명출력여부		
	char bus_cls_prin_yn			[1		+1];	///< 버스등급출력여부		
	char depr_time_prin_yn			[1		+1];	///< 출발시각출력여부		
	char sats_no_prin_yn			[1		+1];	///< 좌석번호출력여부		
	char depr_trml_eng_abrv_nm		[100	+1];	///< 출발터미널영문약칭명	
	char arvl_trml_eng_abrv_nm		[100	+1];	///< 도착터미널영문약칭명	
	char bus_cls_nm					[100	+1];	///< 버스등급명			
	char cacm_nm					[100	+1];	///< 운수사명			
	char bizr_no					[30		+1];	///< 사업자번호			
	char tel_no						[20		+1];	///< 전화번호			
	char alcn_depr_trml_eng_abrv_nm	[100	+1];	///< 배차출발터미널영문약칭명
	char alcn_depr_trml_no			[3		+1];	///< 배차출발터미널번호	
	char alcn_arvl_trml_no			[3		+1];	///< 배차도착터미널번호	
	char alcn_depr_dt				[8		+1];	///< 배차출발일자		
	char alcn_depr_time				[6		+1];	///< 배차출발시각		
	char depr_trml_no				[3		+1];	///< 출발터미널번호		
	char arvl_trml_no				[3		+1];	///< 도착터미널번호		
	char depr_dt					[8		+1];	///< 출발일자			
	char depr_time					[6		+1];	///< 출발시각			
	char bus_cls_cd					[1		+1];	///< 버스등급코드		
	char cacm_cd					[2		+1];	///< 운수사코드			
	char alcn_dvs_cd				[1		+1];	///< 배차구분코드		
	char tissu_dt					[8		+1];	///< 발권일자			
	char tissu_trml_no				[3		+1];	///< 발권터미널번호		
	char tissu_wnd_no				[2		+1];	///< 발권창구번호		
	char tissu_time					[6		+1];	///< 발권시각			
	char rpub_dt					[8		+1];	///< 재발행일자			
	char rpub_trml_no				[3		+1];	///< 재발행터미널번호		
	char rpub_wnd_no				[2		+1];	///< 재발행창구번호		
	char rpub_time					[6		+1];	///< 재발행시각			
	char tissu_sno					[4		+1];	///< 발권일련번호		
	char inhr_no					[8		+1];	///< 고유번호			
	char invs_no					[6		+1];	///< 심사번호			
	char n_tissu_amt				[4		+0];	///< (Number) 발권금액			
	char sats_no					[2		+1];	///< 좌석번호			
	char tck_knd_cd					[1		+1];	///< 티켓종류코드		
	char ptrg_prin_nm				[100	+1];	///< 인쇄출력명			
	// 20211013 ADD
	char card_aprv_amt				[4		+0];	///< (Number) 복합결제(카드)			
	char mlg_aprv_amt				[4		+0];	///< (Number) 복합결제(마일리지)			
	char cpn_aprv_amt				[4		+0];	///< (Number) 복합결제(쿠폰)			
	// 20211013 ~ADD
} rtk_tm_pub_mrs_htck_list_t, *prtk_tm_pub_mrs_htck_list_t;

typedef struct
{
	char rsp_cd						[6		+1];	///< 응답코드		
	char card_no					[100	+1];	///< 카드번호	
	char n_aprv_amt					[4		+0];	///< (Number) 승인금액	
	char card_aprv_no				[100	+1];	///< 카드승인번호
	char n_frc_cmm					[4		+0];	///< (Number) 가맹점수수료
	char n_tissu_num				[4		+0];	///< (Number) 발권수	

	rtk_tm_pub_mrs_htck_list_t		*pList;

} rtk_tm_pub_mrs_htck_t, *prtk_tm_pub_mrs_htck_t;


//------------------------------
// (환불#1/2). 승차권 정보조회  : CM_ReadBusTckNo

typedef struct
{
	char req_pgm_dvs				[3		+1];	///< 요청 프로그램 구분	 
	char req_trml_no				[3		+1];	///< 요청터미널번호	
	char req_wnd_no					[2		+1];	///< 요청창구번호		
	char req_user_no				[6		+1];	///< 요청사용자번호	

	char tissu_dt					[8		+1];	///< 발권일자		
	char tissu_trml_no				[3		+1];	///< 발권터미널번호	
	char tissu_wnd_no				[2		+1];	///< 발권창구번호		
	char tissu_sno					[4		+1];	///< 발권일련번호		

	char tak_dvs_cd					[3		+1];	///< 작업구분코드
	char ryrt_dvs_cd				[1		+1];	///< 환불율구분코드

} stk_tm_read_bus_tckno_t, *pstk_tm_read_bus_tckno_t;

typedef struct
{
	char rsp_cd						[6		+1];	///< 응답코드			
	char depr_dt					[8		+1];	///< 출발일자			
	char hspd_cty_dvs_cd			[1		+1];	///< 고속시외구분코드		
	char drtn_cd					[2		+1];	///< 방면코드			
	char alcn_depr_trml_no			[3		+1];	///< 배차출발터미널번호	
	char alcn_arvl_trml_no			[3		+1];	///< 배차도착터미널번호	
	char depr_trml_no				[3		+1];	///< 출발터미널번호		
	char arvl_trml_no				[3		+1];	///< 도착터미널번호		
	char alcn_rot_no				[4		+1];	///< 배차노선번호		
	char depr_time					[6		+1];	///< 출발시각			
	char bus_cls_cd					[1		+1];	///< 버스등급코드		
	char cacm_cd					[2		+1];	///< 운수사코드			
	char tck_knd_cd					[1		+1];	///< 티켓종류코드		
	char tissu_fee					[4		+0];	///< (Number) 발권요금			
	char sats_no					[2		+1];	///< 좌석번호			
	char inhr_no					[8		+1];	///< 고유번호			
	char invs_no					[6		+1];	///< 심사번호			
	char tissu_chnl_dvs_cd			[1		+1];	///< 발권채널구분코드		
	char tissu_time					[6		+1];	///< 발권시각			
	char tissu_user_no				[6		+1];	///< 발권사용자번호		
	char alcn_rot_nm				[100	+1];	///< 배차노선명			
	char pyn_dtl_cd					[1		+1];	///< 지불상세코드		
	char alcn_depr_time				[6		+1];	///< 배차출발시각		

	char card_no					[100	+1];	///< 카드번호				[카드인 경우]
	char card_aprv_no				[100	+1];	///< 카드승인번호		

	char card_trd_sno				[10		+1];	///< 카드거래일련번호			[선불인 경우]
	char tissu_dt					[8		+1];	///< 발권일자			
	char sam_trd_sno				[10		+1];	///< sam거래일련번호		
	char trd_dtm					[14		+1];	///< 거래일시			

	char user_key_val				[100	+1];	///< 사용자키값				[현금인 경우]
	char csrc_aprv_no				[9		+1];	///< 현금영수증승인번호	
	char user_dvs_cd				[1		+1];	///< 사용자구분코드		

	char ry_amt						[4		+0];	///< (Number) 환불금액		[취소/환불인 경우]	
	char ry_pfit					[4		+0];	///< (Number) 환불차익			
	char dc_rc_amt					[4		+0];	///< (Number) 할인반환금액		

	char rpub_fcnt					[4		+0];	///< (Number) 재발행횟수		[공통]
	char tissu_sta_cd				[1		+1];	///< 발권상태코드		
	char ry_sta_cd					[1		+1];	///< 환불상태코드		
	char chtk_sta_cd				[1		+1];	///< 검표상태코드		
	char rest_sta_cd				[1		+1];	///< 복원상태코드		
	char arvl_dtm					[14		+1];	///< 소요시간

} rtk_tm_read_bus_tckno_t, *prtk_tm_read_bus_tckno_t;

//------------------------------
// (환불#2/2). 승차권 취소/환불  : TK_CancRyTck

typedef struct
{
	char tissu_dt					[8		+1];	///< 발권일자		
	char tissu_trml_no				[3		+1];	///< 발권터미널번호	
	char tissu_wnd_no				[2		+1];	///< 발권창구번호		
	char tissu_sno					[4		+1];	///< 발권일련번호		
	char ryrt_dvs_cd				[1		+1];	///< 환불율구분코드	

} stk_tm_cancrytck_list_t, *pstk_tm_cancrytck_list_t;

typedef struct
{
	char req_pgm_dvs				[3		+1];	///< 요청 프로그램 구분	
	char req_trml_no				[3		+1];	///< 요청터미널번호	
	char req_wnd_no					[2		+1];	///< 요청창구번호		
	char req_user_no				[6		+1];	///< 요청사용자번호	

	char req_dvs_cd					[1		+1];	///< 요청구분코드		
	char rec_num					[4		+0];	///< (Number) 정보 수		

	stk_tm_cancrytck_list_t		List[10];

} stk_tm_cancrytck_t, *pstk_tm_cancrytck_t;

typedef struct
{
	char pyn_dtl_cd					[1		+1];	///< 지불상세코드	
	char ryrt_dvs_cd				[1		+1];	///< 환불율구분코드
	char ry_amt						[4		+0];	///< (Number) 환불금액	
	char dc_rc_amt					[4		+0];	///< (Number) 할인반환금액	
	// 20211013 ADD
	char pyn_dvs_cd					[1		+1];	//지불구분코드 VARCHAR2( j )(1) '5'일 경우 복합결제
	char card_canc_amt				[4		+0];	//복합결제취소(카드) NUMBER( i )(22) 지불구분이 '5'일 경우에만 사용	
	char mlg_canc_amt				[4		+0];	//복합결제취소(마일리지) NUMBER( i )(22) 지불구분이 '5'일 경우에만 사용
	char cpn_canc_amt				[4		+0];	//복합결제취소(쿠폰) NUMBER( i )(22) 지불구분이 '5'일 경우에만 사용
	// 20211013 ~ADD

} rtk_tm_cancrytck_list_t, *prtk_tm_cancrytck_list_t;

typedef struct
{
	char rsp_cd						[6		+1];	///< 응답코드		
	char rec_num					[4		+0];	///< (Number) 정보 수
	
	rtk_tm_cancrytck_list_t	*pList;				///< 

} rtk_tm_cancrytck_t, *prtk_tm_cancrytck_t;

//------------------------------
// (발권내역조회). 발권내역조회  : CM_PubPt

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분
	char req_trml_no				[3		+1]	;	///< 요청 터미널번호
	char req_wnd_no					[2		+1]	;	///< 요청 창구번호
	char req_user_no				[6		+1]	;	///< 요청 사용자번호

	char rec_num					[4		+0];	///< (Number) 정보 수		
	char sort_dvs_cd				[1		+1];	///< 정렬구분코드	
	char tak_dt						[8		+1];	///< 작업일자		
	char tak_time					[6		+1];	///< 작업시각		
	char depr_trml_no				[3		+1];	///< 출발터미널번호	
	char tak_wnd_no					[2		+1];	///< 작업창구번호	
	char tak_user_no				[6		+1];	///< 작업사용자번호	
	char arvl_trml_no				[3		+1];	///< 도착터미널번호	
	char tissu_dt					[8		+1];	///< 발권일자		
	char tissu_trml_no				[3		+1];	///< 발권터미널번호	
	char tissu_wnd_no				[2		+1];	///< 발권창구번호	
	char tissu_sno					[4		+1];	///< 발권일련번호	
	char bef_aft_dvs				[1		+1];	///< 이전이후구분	

} stk_tm_pub_inq_t, *pstk_tm_pub_inq_t;

typedef struct
{
	char tissu_dt					[8		+1];	///< 발권일자		
	char tissu_trml_no				[3		+1];	///< 발권터미널번호	
	char tissu_wnd_no				[2		+1];	///< 발권창구번호	
	char tissu_sno					[4		+1];	///< 발권일련번호	
	char inhr_no					[8		+1];	///< 고유번호		
	char depr_dt					[8		+1];	///< 출발일자		
	char drtn_cd					[2		+1];	///< 방면코드		
	char hspd_cty_dvs_cd			[1		+1];	///< 고속시외구분코드	
	char alcn_depr_trml_no			[3		+1];	///< 배차출발터미널번호
	char alcn_arvl_trml_no			[3		+1];	///< 배차도착터미널번호
	char alcn_rot_no				[4		+1];	///< 배차노선번호	
	char depr_trml_no				[3		+1];	///< 출발터미널번호	
	char arvl_trml_no				[3		+1];	///< 도착터미널번호	
	char alcn_depr_time				[6		+1];	///< 배차출발시각	
	char depr_time					[6		+1];	///< 출발시각		
	char bus_cls_cd					[1		+1];	///< 버스등급코드	
	char cacm_cd					[2		+1];	///< 운수사코드		
	char tck_knd_cd					[1		+1];	///< 티켓종류코드	
	char tissu_fee					[4		+0];	///< (Number)발권요금		
	char sats_no					[2		+1];	///< 좌석번호		
	char tissu_sta_cd				[1		+1];	///< 발권상태코드	
	char tissu_chnl_dvs_cd			[1		+1];	///< 발권채널구분코드	
	char pyn_dtl_cd					[1		+1];	///< 지불상세코드	
	char tissu_time					[6		+1];	///< 발권시각		
	char tissu_user_no				[6		+1];	///< 발권사용자번호	
	char alcn_dvs_cd				[1		+1];	///< 배차구분코드	
	char ry_sta_cd					[1		+1];	///< 환불상태코드	
	char alcn_rot_nm				[100	+1];	///< 배차노선명		
	char dta_dvs_cd					[1		+1];	///< 데이터구분코드	

	char card_no					[100	+1];	///< 카드번호 - 카드
	char card_aprv_no				[100	+1];	///< 카드승인번호 - 카드

	char user_key_val				[100	+1];	///< 사용자키값 - 현금
	char csrc_aprv_no				[9		+1];	///< 현금영수증승인번호 - 현금
	char user_dvs_cd				[1		+1];	///< 사용자구분코드 - 현금

	char trd_bef_card_bal			[4		+0];	///< (number)거래이전카드잔액	 - 선불카드
	char trd_req_amt				[4		+0];	///< (number)거래요청금액 - 선불카드
	char trd_aft_card_bal			[4		+0];	///< (number)거래이후카드잔액	 - 선불카드

} rtk_tm_pub_inq_list_t, *prtk_tm_pub_inq_list_t;

typedef struct
{
	char rsp_cd						[6		+1];	///< 응답코드		
	char bef_aft_dvs				[1		+1];	///< 이전이후구분
	char rec_num					[4		+0];	///< (Number)정보 수
	rtk_tm_pub_inq_list_t *pList;
} rtk_tm_pub_inq_t, *prtk_tm_pub_inq_t;

//------------------------------
// (재발행내역조회). 재발행 내역조회 : CM_RPubPt

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분
	char req_trml_no				[3		+1]	;	///< 요청 터미널번호
	char req_wnd_no					[2		+1]	;	///< 요청 창구번호
	char req_user_no				[6		+1]	;	///< 요청 사용자번호

	char rec_num					[4		+0];	///< (Number)정보 수		
	char sort_dvs_cd				[1		+1];	///< 정렬구분코드	
	char tak_dt						[8		+1];	///< 작업일자		
	char tak_time					[6		+1];	///< 작업시각		
	char tak_wnd_no					[2		+1];	///< 작업창구번호	
	char tak_user_no				[6		+1];	///< 작업사용자번호	
	char depr_trml_no				[3		+1];	///< 출발터미널번호	
	char arvl_trml_no				[3		+1];	///< 도착터미널번호	
	char tissu_dt					[8		+1];	///< 발권일자		
	char tissu_trml_no				[3		+1];	///< 발권터미널번호	
	char tissu_wnd_no				[2		+1];	///< 발권창구번호	
	char tissu_sno					[4		+1];	///< 발권일련번호	
	char bef_aft_dvs				[1		+1];	///< 이전이후구분	

} stk_tm_rpub_inq_t, *pstk_tm_rpub_inq_t;

typedef struct
{
	char tissu_dt					[8		+1];	///< 발권일자		
	char tissu_trml_no				[3		+1];	///< 발권터미널번호	
	char tissu_wnd_no				[2		+1];	///< 발권창구번호	
	char tissu_sno					[4		+1];	///< 발권일련번호	
	char depr_dt					[8		+1];	///< 출발일자		
	char drtn_cd					[2		+1];	///< 방면코드		
	char hspd_cty_dvs_cd			[1		+1];	///< 고속시외구분코드	
	char alcn_depr_trml_no			[3		+1];	///< 배차출발터미널번호
	char alcn_arvl_trml_no			[3		+1];	///< 배차도착터미널번호
	char alcn_rot_no				[4		+1];	///< 배차노선번호	
	char depr_trml_no				[3		+1];	///< 출발터미널번호	
	char arvl_trml_no				[3		+1];	///< 도착터미널번호	
	char alcn_depr_time				[6		+1];	///< 배차출발시각	
	char depr_time					[6		+1];	///< 출발시각		
	char bus_cls_cd					[1		+1];	///< 버스등급코드	
	char cacm_cd					[2		+1];	///< 운수사코드		
	char tck_knd_cd					[1		+1];	///< 티켓종류코드	
	char tissu_fee					[4		+0];	///< (number)발권요금		
	char sats_no					[2		+1];	///< 좌석번호		
	char tissu_sta_cd				[1		+1];	///< 발권상태코드	
	char pyn_dtl_cd					[1		+1];	///< 지불상세코드	
	char tissu_time					[6		+1];	///< 발권시각		
	char tissu_user_no				[6		+1];	///< 발권사용자번호	
	///< <카드인 경우>	
	char card_no					[100	+1];	///< 카드번호		
	char card_aprv_no				[100	+1];	///< 카드승인번호	
	///< <현금인 경우>	
	char user_key_val				[100	+1];	///< 사용자키값		
	char csrc_aprv_no				[9		+1];	///< 현금영수증승인번호
	char user_dvs_cd				[1		+1];	///< 사용자구분코드	
	///< <공통>		
	char rpub_trml_no				[3		+1];	///< 재발행터미널번호	
	char rpub_wnd_no				[2		+1];	///< 재발행창구번호	
	char rpub_user_no				[6		+1];	///< 재발행사용자번호	
	char rpub_dt					[8		+1];	///< 재발행일자		
	char rpub_time					[6		+1];	///< 재발행시각		
	char alcn_rot_nm				[100	+1];	///< 배차노선명		

} rtk_tm_rpub_inq_list_t, *prtk_tm_rpub_inq_list_t;

typedef struct
{
	char rsp_cd						[6		+1];	///< 응답코드		
	char bef_aft_dvs				[1		+1];	///< 이전이후구분
	char rec_num					[4		+0];	///< (Number)정보 수
	rtk_tm_rpub_inq_list_t *pList;
} rtk_tm_rpub_inq_t, *prtk_tm_rpub_inq_t;

//------------------------------
// (재발행). 재발행  : TK_RPubTck

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분
	char req_trml_no				[3		+1]	;	///< 요청 터미널번호
	char req_wnd_no					[2		+1]	;	///< 요청 창구번호
	char req_user_no				[6		+1]	;	///< 요청 사용자번호

	char tissu_dt					[8		+1];	///< 발권일자		
	char tissu_trml_no				[3		+1];	///< 발권터미널번호	
	char tissu_wnd_no				[2		+1];	///< 발권창구번호	
	char tissu_sno					[4		+1];	///< 발권일련번호	

} stk_tm_rpub_tck_t, *pstk_tm_rpub_tck_t;

typedef struct
{
	char rsp_cd						[6		+1];	///< 응답코드		
	char inhr_no					[8		+1];	///< 고유번호		
	char invs_no					[6		+1];	///< 심사번호		
	char frc_cmm					[4		+0];	///< (Number) 가맹점수수료	
	char cacm_nm_prin_yn			[1		+1];	///< 운수사명출력여부	
	char bus_cls_prin_yn			[1		+1];	///< 버스등급출력여부	
	char depr_time_prin_yn			[1		+1];	///< 출발시각출력여부	
	char sats_no_prin_yn			[1		+1];	///< 좌석번호출력여부	
	char rot_rdhm_no_val			[100	+1];	///< 노선승차홈번호값	
	char csrc_rgt_no				[9		+1];	///< 현금영수증등록번호

} rtk_tm_rpub_tck_t, *prtk_tm_rpub_tck_t;

//------------------------------
// (환불내역조회). 환불내역조회  : CM_CanRyPt

typedef struct
{
	char req_pgm_dvs				[3		+1]	;	///< 요청 프로그램 구분
	char req_trml_no				[3		+1]	;	///< 요청 터미널번호
	char req_wnd_no					[2		+1]	;	///< 요청 창구번호
	char req_user_no				[6		+1]	;	///< 요청 사용자번호

	char rec_num					[4		+0];	///< 정보 수		
	char tak_dvs_cd					[3		+1];	///< 작업구분코드	
	char sort_dvs_cd				[1		+1];	///< 정렬구분코드	
	char tak_dt						[8		+1];	///< 작업일자		
	char tak_time					[6		+1];	///< 작업시각		
	char tak_wnd_no					[2		+1];	///< 작업창구번호	
	char tak_user_no				[6		+1];	///< 작업사용자번호	
	char depr_trml_no				[3		+1];	///< 출발터미널번호	
	char arvl_trml_no				[3		+1];	///< 도착터미널번호	
	char tissu_dt					[8		+1];	///< 발권일자		
	char tissu_trml_no				[3		+1];	///< 발권터미널번호	
	char tissu_wnd_no				[2		+1];	///< 발권창구번호	
	char tissu_sno					[4		+1];	///< 발권일련번호	
	char bef_aft_dvs				[1		+1];	///< 이전이후구분	

} stk_tm_canry_inq_t, *pstk_tm_canry_inq_t;

typedef struct
{
	char tissu_dt					[8		+1];	///< 발권일자			
	char tissu_trml_no				[3		+1];	///< 발권터미널번호		
	char tissu_wnd_no				[2		+1];	///< 발권창구번호m		
	char tissu_sno					[4		+1];	///< 발권일련번호		
	char depr_dt					[8		+1];	///< 출발일자			
	char drtn_cd					[2		+1];	///< 방면코드			
	char hspd_cty_dvs_cd			[1		+1];	///< 고속시외구분코드		
	char alcn_depr_trml_no			[3		+1];	///< 배차출발터미널번호	
	char alcn_arvl_trml_no			[3		+1];	///< 배차도착터미널번호	
	char alcn_rot_no				[4		+1];	///< 배차노선번호		
	char depr_trml_no				[3		+1];	///< 출발터미널번호		
	char arvl_trml_no				[3		+1];	///< 도착터미널번호		
	char alcn_depr_time				[6		+1];	///< 배차출발시각		
	char depr_time					[6		+1];	///< 출발시각			
	char bus_cls_cd					[1		+1];	///< 버스등급코드		
	char cacm_cd					[2		+1];	///< 운수사코드			
	char tck_knd_cd					[1		+1];	///< 티켓종류코드		
	char tissu_fee					[4		+1];	///< 발권요금 (number)
	char sats_no					[2		+1];	///< 좌석번호			
	char tissu_sta_cd				[1		+1];	///< 발권상태코드		
	char pyn_dtl_cd					[1		+1];	///< 지불상세코드		
	char tissu_time					[6		+1];	///< 발권시각			
	char tissu_user_no				[6		+1];	///< 발권사용자번호		
	//cha	  	[		+1];	///< <카드인 경우>		
	char card_no					[100	+1];	///< 카드번호			
	char card_aprv_no				[100	+1];	///< 카드승인번호		
	//cha	 	[		+1];	///< <현금인 경우>		
	char user_key_val				[100	+1];	///< 사용자키값			
	char csrc_aprv_no				[9		+1];	///< 현금영수증승인번호	
	char user_dvs_cd				[1		+1];	///< 사용자구분코드		
	//cha 	[		+1];	///< <공통>			
	char ry_sta_cd					[1		+1];	///< 환불상태코드		
	char ry_dt						[8		+1];	///< 환불일자			
	char ry_time					[6		+1];	///< 환불시각			
	char ry_trml_no					[3		+1];	///< 환불터미널번호		
	char ry_wnd_no					[2		+1];	///< 환불창구번호		
	char ry_user_no					[6		+1];	///< 환불사용자번호		
	//cha	 	[		+1];	///< <환불일 경우>		
	char ry_knd_cd					[1		+1];	///< 환불종류코드		
	char ry_amt						[4		+0];	///< 환불금액			
	char dc_rc_amt					[4		+0];	///< 할인반환금액		

} rtk_tm_canry_inq_list_t, *prtk_tm_canry_inq_list_t;

typedef struct
{
	char rsp_cd						[6		+1];	///< 응답코드		
	char bef_aft_dvs				[1		+1];	///< 이전이후구분
	char rec_num					[4		+0];	///< (Number)정보 수
	rtk_tm_canry_inq_list_t *pList;
} rtk_tm_canry_inq_t, *prtk_tm_canry_inq_t;


#pragma pack()

