// 
// 
// svr_ccbus_st.h : 시외버스 서버 패킷 구조체 헤더 파일
//

#pragma once



//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

typedef struct 
{
	DWORD	dwFieldKey;
	char*	pName;
} CCBUS_DBG_T, *PCCBUS_DBG_T;


typedef struct
{
	char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	char req_trml_cd		[7+1];	///< 요청 터미널 코드
	char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_head_t, *pstk_head_t;

typedef struct
{
	char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	char req_trml_cd		[7+1];	///< 요청 터미널 코드
	char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	char req_trml_user_ip	[40+1];	///< 요청 장비 IP
	char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 ID	
} stk_head_id_t, *pstk_head_id_t;

typedef struct
{
	char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분		
	char req_shct_trml_cd	[4+1];	///< 요청 단축 터미널 코드	
	char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
	char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 ip	
} stk_head_shrt_t, *pstk_head_shrt_t;

//------------------------------
// IF_SV_100 : 공통코드 정보

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_readcmncd_t, *pstk_readcmncd_t;

typedef struct
{
	char cmn_cd_id			[3+1];	///< 공통코드 ID
	char cmn_cd_nm			[100+1];	///< 공통코드 명
	char cmn_cd_eng_nm		[100+1];	///< 공통코드 영문명
	char hgrn_cmn_cd_id		[3+1];	///< 상위 공통코드 ID
} rtk_readcmncd_list_t, *prtk_readcmncd_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char cmn_cd_num			[5+1];	///< 공통코드 갯수
	rtk_readcmncd_list_t* pList;	
} rtk_readcmncd_t, *prtk_readcmncd_t;


//------------------------------
// IF_SV_101 : 접근 컴퓨터 인증

typedef struct
{
	stk_head_shrt_t		hdr_st;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분		
	//char req_shct_trml_cd	[4+1];	///< 요청 단축 터미널 코드	
	//char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 ip	
	char req_dvs			[1+1];	///< 조회 구분				
	char acs_cmpt_inf		[12+1];	///< 접근 컴퓨터 정보		
} stk_authcmpt_t, *pstk_authcmpt_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char serv_dt			[8+1];	///< 서버일자
	char serv_time			[6+1];	///< 서버시각
	char shct_trml_cd		[4+1];	///< 단축 터미널코드
	char trml_cd			[7+1];	///< 터미널코드
} rtk_authcmpt_t, *prtk_authcmpt_t;

//------------------------------
// IF_SV_102 : 공통코드 상세 조회

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_cmncddtl_t, *pstk_cmncddtl_t;

//>>

typedef struct
{
	char cmn_cd_id			[3+1];	///< 공통코드 ID....
	char cmn_cd_val			[100+1];	///< 공통코드 값
	char cd_val_nm			[100+1];	///< 코드값 명
	char cd_val_eng_nm		[100+1];	///< 코드값 영문명
	char cd_val_mrk_seq		[5+1];	///< 코드값 표시순서(number)
	char sbrd_cmn_cd_id		[3+1];	///< 하위 공통코드 ID
	char cd_rfrn_val_1		[100+1];	///< 코드 참조값1
	char cd_rfrn_val_desc_1	[100+1];	///< 코드 참조값 설명1
	char cd_rfrn_val_2		[100+1];	///< 코드 참조값2
	char cd_rfrn_val_desc_2	[100+1];	///< 코드 참조값 설명2
	char cd_rfrn_val_3		[100+1];	///< 코드 참조값3.
	char cd_rfrn_val_desc_3	[100+1];	///< 코드 참조값 설명3

	//int n_cd_val_mrk_seq;			///< 코드값 표시순서(number)
} rtk_cmncddtl_list_t, *prtk_cmncddtl_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char cmn_dtl_num		[5+1];	///< 공통코드 상세 수
	rtk_cmncddtl_list_t* pList;
} rtk_cmncddtl_t, *prtk_cmncddtl_t;

//------------------------------
// IF_SV_104 : 좌석 배치도 코드 조회

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_readarctcd_t, *pstk_readarctcd_t;

//>>

typedef struct
{
	char sats_arct_cd		[3+1];	///< 좌석 배치도 유형코드
} sats_arct_cd_t, *psats_arct_cd_t;

typedef struct
{
	char sats_arct_typ_cd	[1+1];	///< 좌석 배치도 유형코드
	char avd_sats_no_mltp_val[100+1];	///< 기피 좌석번호 다중값
} sats_info_t, *psats_info_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char sats_arct_cd_num	[3+1];	///< 좌석배치도코드 수
	sats_arct_cd_t*	ptype_list;		///< 좌석배치 유형 코드
	char sats_num			[3+1];	///< 좌석 수 
	sats_info_t* psats_list;		
} rtk_readarctcd_t, *prtk_readarctcd_t;

//------------------------------
// IF_SV_106 : 버스 운수사 코드 조회

//>> 송신

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_readbuscacm_t, *pstk_readbuscacm_t;

//>> 수신

typedef struct
{
	char bus_cacm_cd		[4+1];	///< 버스운수사코드
	char bus_cacm_nm		[100+1];///< 버스운수사명
	char cd_eng_nm			[100+1];///< 코드영문명
	char bus_cacm_shct_nm	[100+1];///< 버스운수사단축명
	char bus_cacm_brn		[10+1];	///< 버스운수사사업자등록번호
	char bus_cacm_eng_nm	[100+1];///< 버스운수사영문명
	char bus_cacm_tel_no	[50+1];	///< 버스운수사전화번호
} rtk_readbuscacm_list_t, *prtk_readbuscacm_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char bus_cacm_cd_num	[4+1];	///< 버스운수사코드 수
	rtk_readbuscacm_list_t*	 pList;
} rtk_readbuscacm_t, *prtk_readbuscacm_t;

//------------------------------
// IF_SV_107 : 버스 등급 코드 조회

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_readbuscls_t, *pstk_readbuscls_t;

//>> 

typedef struct
{
	char bus_cls_cd			[3+1];	///< 버스등급코드
	char bus_cls_nm			[100+1];///< 버스등급명 (number) 
	char rot_knd_cd			[1+1];	///< 노선종류코드
	char cd_eng_nm			[100+1];///< 코드영문명
	char bus_cls_shct_nm	[100+1];///< 버스등급단축명
	char bus_cls_mrk_seq	[5+1];	///< 버스등급표시순서 (number) 
	char bus_cls_eng_nm		[100+1];///< 버스등급영문명

	//	int	 n_bus_cls_mrk_seq;			///< 버스등급표시순서 (number) 
} rtk_readbuscls_list_t, *prtk_readbuscls_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char bus_cls_cd_num		[4+1];	///< 버스등급코드 데이터 건수
	rtk_readbuscls_list_t*	pList;
} rtk_readbuscls_t, *prtk_readbuscls_t;

//------------------------------
// IF_SV_108 : 버스 티켓 종류 코드 조회

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_readtckknd_t, *pstk_readtckknd_t;

//>>

typedef struct
{
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드
	char bus_tck_knd_nm		[100+1];	///< 버스티켓종류명
	char rot_knd_cd			[1+1];	///< 노선종류코드
	char cd_eng_nm			[100+1];	///< 코드영문명
	char bus_tck_knd_shct_nm[100+1];	///< 버스티켓종류단축명
	char bus_tck_mrk_seq	[5+1];	///< 버스티켓표시순서(number)
	char bus_tck_knd_eng_nm	[100+1];	///< 버스티켓종류영문명

	//char n_bus_tck_mrk_seq;			///< 버스티켓표시순서(number)
} rtk_readtckknd_list_t, *prtk_readtckknd_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char bus_tck_knd_cd_num	[3+1];	///< 승차권종류코드 데이터 건수
	rtk_readtckknd_list_t*	pList;
} rtk_readtckknd_t, *prtk_readtckknd_t;

//------------------------------
// IF_SV_112 : 창구 사용 터미널 상세 조회

typedef struct
{
	char trml_cd			[4+1];	///< 터미널코드
	char wnd_no				[4+1];	///< 창구번호
	char use_trml_cd		[4+1];	///< 사용터미널코드
} rtk_wndtrmldtl_list_t, *prtk_wndtrmldtl_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char wnd_use_trml_dtl_num[7+1];	///< 창구사용터미널상세 수
	rtk_wndtrmldtl_list_t* pList;		
} rtk_wndtrmldtl_t, *prtk_wndtrmldtl_t;

//------------------------------
// IF_SV_114 : 사용자 기본 조회

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_readuserbsc_t, *pstk_readuserbsc_t;

//>>

typedef struct
{
	char user_id			[17+1];	///< 사용자ID
	char user_nm			[100+1];///< 사용자명
	char cty_bus_user_dvs_cd[1+1];	///< 시외버스사용자구분코드
	char user_hndh_tel_no	[100+1];///< 사용자휴대전화번호
	char user_pwd			[100+1];///< 사용자비밀번호
	char user_no			[10+1];	///< 사용자번호
	char bus_cacm_cd		[4+1];	///< 버스운수사코드
	char tisu_ltn_drtm		[5+1];	///< 발권제한시간(number)
	char trml_cd			[7+1];	///< 터미널코드
	char lst_lgn_dt			[8+1];	///< 최종로그인일자
	char dlt_dt				[8+1];	///< 삭제일자
	char pwd_mod_dt			[8+1];	///< 비밀번호변경일자
	char bus_cls_chc_yn		[1+1];	///< 버스등급선택여부
	char depr_time_chc_yn	[1+1];	///< 출발시각선택여부
	char lgn_yn				[1+1];	///< 로그인여부

	//char n_tisu_ltn_drtm;			///< 발권제한시간(number)
} rtk_readuserbsc_list_t, *prtk_readuserbsc_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char user_bsc_num		[5+1];	///< 사용자기본 수
	rtk_readuserbsc_list_t*  pList;
} rtk_readuserbsc_t, *prtk_readuserbsc_t;

//------------------------------
// IF_SV_118 : 터미널 창구 상세 조회

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_trmwnddtl_t, *pstk_trmwnddtl_t;

//>>

typedef struct
{
	char wnd_no				[2+1];	///< 창구번호
	char wnd_nm				[100+1];	///< 창구명
	char tisu_ltn_drtm		[5+1];	///< 발권제한시간(number)
	char bus_cls_chc_yn		[1+1];	///< 버스등급선택여부
	char depr_time_chc_yn	[1+1];	///< 출발시각선택여부
	char crdt_card_vanc_cd	[2+1];	///< 신용카드van사코드

	//int n_tisu_ltn_drtm;			///< 발권제한시간(number)
} rtk_trmwnddtl_list_t, *prtk_trmwnddtl_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char trml_wnd_dtl_num	[7+1];	///< 터미널창구상세 수
	rtk_trmwnddtl_list_t* pList;
} rtk_trmwnddtl_t, *prtk_trmwnddtl_t;

//------------------------------
// IF_SV_119 : 컴퓨터 설정 상세 조회

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_cmptstupdtl_t, *pstk_cmptstupdtl_t;

//>>

typedef struct
{
	char cmpt_stup_dvs_cd	[3+1];	///< 컴퓨터설정 구분코드
	char cmpt_stup_val		[100+1];	///< 컴퓨터설정 값
} rtk_cmptstupdtl_list_t, *prtk_cmptstupdtl_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char cmpt_stup_dtl_num	[7+1];	///< 터미널창구상세 수
	rtk_cmptstupdtl_list_t* pList;		
} rtk_cmptstupdtl_t, *prtk_cmptstupdtl_t;

//------------------------------
// IF_SV_120 : 터미널코드 조회

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_readtrmlcd_t, *pstk_readtrmlcd_t;

//>>

typedef struct
{
	char trml_cd			[7+1];	///< 터미널코드
	char trml_nm			[100+1];///< 터미널명
	char shct_trml_cd		[4+1];	///< 단축터미널코드
	char shct_arvl_trml_cd	[3+1];	///< 단축도착터미널코드
	char trml_shct_nm		[100+1];///< 터미널단축명
	char trml_eng_nm		[100+1];///< 터미널영문명
	char trml_zip			[5+1];	///< 터미널우편번호
	char trml_shp_cd		[1+1];	///< 터미널형태코드
	char trml_knd_cd		[1+1];	///< 터미널종류코드
	char cty_bus_sys_dvs_cd	[1+1];	///< 시외버스시스템구분코드
	char trml_rprn_tel_no	[50+1];	///< 터미널대표전화번호
	char trml_addr			[100+1];///< 터미널주소
	char trml_hmpg_url_val	[100+1];///< 터미널홈페이지URL값
	char trml_lttd			[18+1];	///< 터미널위도 - 사용안함
	char trml_lngt			[18+1];	///< 터미널경도 - 사용안함
	char trml_brn			[10+1];	///< 터미널사업자등록번호
	char cty_bus_area_cd	[2+1];	///< 시외버스지역코드
	char trml_drtn_cd		[2+1];	///< 터미널방면코드
	char snd_ptr_use_yn		[1+1];	///< 다른 프린터 사용 여부
} rtk_readtrmlcd_list_t, *prtk_readtrmlcd_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char trml_cd_num		[7+1];	///< 터미널코드 수
	rtk_readtrmlcd_list_t*	pList;		
} rtk_readtrmlcd_t, *prtk_readtrmlcd_t;

//------------------------------
// IF_SV_122 : 터미널 설정 상세 조회

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_trmlstupdtl_t, *pstk_trmlstupdtl_t;

//>>

typedef struct
{
	char trml_stup_dvs_cd	[3+1];	///< 터미널설정구분코드
	char trml_stup_dvs_val	[100+1];	///< 터미널설정구분값
} rtk_trmlstupdtl_list_t, *prtk_trmlstupdtl_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char trml_stup_dtl_num	[7+1];	///< 터미널설정상세 수
	rtk_trmlstupdtl_list_t*  pList;		
} rtk_trmlstupdtl_t, *prtk_trmlstupdtl_t;

//------------------------------
// IF_SV_123 : 터미널단축키상세 조회

typedef struct
{
	char shck_val			[100+1];	///< 단축키 값
	char trml_cd			[7+1];	///< 터미널코드
	char menu_id			[17+1];	///< 메뉴ID

} rtk_trmlshckdtl_list_t, *prtk_trmlshckdtl_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char trml_shck_dtl_num	[7+1];	///< 터미널단축키상세 수
	rtk_trmlshckdtl_list_t* pList;		
} tk_trmlshckdtl_t, *ptk_trmlshckdtl_t;

//------------------------------
// IF_SV_124 : 노선 기본 조회

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_readrotbsc_t, *pstk_readrotbsc_t;

//>>

typedef struct
{
	char rot_id				[17+1];	///< 노선ID
	char stpt_trml_cd		[7+1];	///< 기점터미널코드
	char ept_trml_cd		[7+1];	///< 종점터미널코드
	char rot_sno			[2+1];	///< 노선일련번호
	char rot_nm				[100+1];///< 노선명
	char rot_knd_cd			[1+1];	///< 노선종류코드
	char alcn_way_dvs_cd	[1+1];	///< 배차방식구분코드
	char sati_use_yn		[1+1];	///< 좌석제사용여부
	char eb_lnkg_yn			[1+1];	///< EB연동여부
	char hmpg_exps_yn		[1+1];	///< 홈페이지노출여부
	char adpt_stt_dt		[8+1];	///< 적용시작일자
	char adpt_end_dt		[8+1];	///< 적용종료일자

} rtk_readrotbsc_list_t, *prtk_readrotbsc_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char rot_bsc_num		[17+1];	///< 노선기본 수
	rtk_readrotbsc_list_t* pList;
} rtk_readrotbsc_t, *prtk_readrotbsc_t;

//------------------------------
// IF_SV_125 : 노선 상세 조회

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_readrotdtl_t, *pstk_readrotdtl_t;

//>>

typedef struct
{
	char rot_id				[17+1];	///< 노선ID
	char rot_sqno			[5+1];	///< 노선순번
	char alcn_way_dvs_cd	[1+1];	///< 배차방식구분코드
	char sati_use_yn		[1+1];	///< 좌석제사용여부
	char trml_tisu_psb_yn	[1+1];	///< 터미널발권가능여부
	char hmpg_tisu_psb_yn	[1+1];	///< 홈페이지발권가능여부
	char mbl_tisu_psb_yn	[1+1];	///< 모바일발권가능여부
	char trml_cd			[7+1];	///< 터미널코드
	char bef_trmi_dist		[18+1];	///< 이전터미널간거리(number)
	char bef_trmi_take_drtm	[18+1];	///< 이전터미널간소요시간(number)
	char trml_ctas_drtm		[18+1];	///< 터미널정차시간(number)
	char frbs_time			[6+1];	///< 첫차시각
	char lsbs_time			[6+1];	///< 막차시각
	char bus_cls_prin_yn	[1+1];	///< 버스등급출력여부
	char bus_cacm_nm_prin_yn[1+1];	///< 버스운수사명출력여부
	char depr_time_prin_yn	[1+1];	///< 출발시각출력여부
	char sats_no_prin_yn	[1+1];	///< 좌석번호출력여부
	char rdhm_mltp_val		[100+1];	///< 승차홈다중값

	// 	int  n_bef_trmi_dist;			///< 이전터미널간거리(number)
	// 	int  n_bef_trmi_take_drtm;		///< 이전터미널간소요시간(number)
	// 	int  n_trml_ctas_drtm;			///< 터미널정차시간(number)


} rtk_readrotdtl_list_t, *prtk_readrotdtl_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char rot_dtl_num		[17+1];	///< 노선상세 수
	rtk_readrotdtl_list_t* pList;		
} rtk_readrotdtl_t, *prtk_readrotdtl_t;

//------------------------------
// IF_SV_126 : 노선 요금 상세 조회

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_rotfeedtl_t, *pstk_rotfeedtl_t;

typedef struct
{
	char rot_id				[17+1];	///< 노선ID
	char rot_fee_sno		[5+1];	///< 노선요금일련번호
	char depr_trml_cd		[7+1];	///< 출발터미널코드
	char arvl_trml_cd		[7+1];	///< 도착터미널코드
	char bus_cls_cd			[3+1];	///< 버스등급코드
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드 
	char adpt_stt_dt		[8+1];	///< 적용시작일자
	char adpt_end_dt		[8+1];	///< 적용종료일자
	char ride_fee			[18+1];	///< 승차요금

} rtk_rotfeedtl_list_t, *prtk_rotfeedtl_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char rot_fee_dtl_num	[5+1];	///< 노선요금상세 수
	rtk_rotfeedtl_list_t*	pList;		
} rtk_rotfeedtl_t, *prtk_rotfeedtl_t;

//------------------------------
// IF_SV_127 : 버스티켓고유번호 조회

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_readtckno_t, *pstk_readtckno_t;

//>>

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char bus_tck_inhr_no	[8+1];	///< 버스티켓 고유번호
	char add_bus_tck_inhr_no[8+1];	///< 추가 버스티켓 고유번호
} rtk_readtckno_t, *prtk_readtckno_t;

//------------------------------
// IF_SV_128 : 로그인

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
	char user_no			[10+1];	///< 사용자 번호
	char req_trml_user_pwd	[100+1];	///< 요청 터미널 사용자 비밀번호
	char bus_tck_inhr_no	[8+1];	///< 버스티켓 고유번호
	char add_bus_tck_inhr_no[8+1];	///< 추가 버스티켓 고유번호

} stk_login_t, *pstk_login_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char rsp_msg			[100+1];	///< 응답 메시지
	char user_id			[17+1];	///< 사용자id
	char cash_tisu_num		[5+1];	///< 현금발권 매수(number)
	char cash_tisu_amt		[18+1];	///< 현금발권 금액(number)
	char cash_canc_ry_num	[5+1];	///< 현금취소/환불 매수(number)
	char cash_canc_ry_amt	[18+1];	///< 현금취소/환불 금액(number)
	char card_tisu_num		[5+1];	///< 카드발권 매수(number)
	char card_tisu_amt		[18+1];	///< 카드발권 금액(number)
	char card_canc_ry_num	[5+1];	///< 카드취소/환불 매수(number)
	char card_canc_ry_amt	[18+1];	///< 카드취소/환불 금액(number)
	char etc_tisu_num		[5+1];	///< 기타발권 매수(number)
	char etc_tisu_amt		[18+1];	///< 기타발권 금액(number)
	char etc_canc_ry_num	[5+1];	///< 기타취소/환불 매수(number)
	char etc_canc_ry_amt	[18+1];	///< 기타취소/환불 금액(number)
} rtk_login_t, *prtk_login_t;

//------------------------------
// IF_SV_129 : 알림조회

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_readntfc_t, *pstk_readntfc_t;

//>>

typedef struct
{
	char ntfc_id			[17+1];	///< 알림ID
	char ntfc_nm			[100+1];	///< 알림명
	char cty_bus_ntfc_dvs_cd[3+1];	///< 시외버스알림구분코드
	char ntfc_ctt			[2000+1];	///< 알림내용
	char ntfc_stt_dt		[8+1];	///< 알림시작일자
	char ntfc_stt_time		[6+1];	///< 알림시작시각
	char ntfc_end_dt		[8+1];	///< 알림종료일자
	char ntfc_end_time		[6+1];	///< 알림종료시각
	char ntfc_user_id		[17+1];	///< 알림사용자ID
	char all_ntc_yn			[1+1];	///< 전체공지여부
} rtk_readntfc_list_t, *prtk_readntfc_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char ntfc_num			[5+1];	///< 알림 수
	rtk_readntfc_list_t* pList;		///< 
} rtk_readntfc_t, *prtk_readntfc_t;

//------------------------------
// IF_SV_130 : 배차조회

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
	//char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 id	

	char depr_trml_cd		[7+1];	///< 출발터미널코드			
	char arvl_trml_cd		[7+1];	///< 도착터미널코드			
	char read_dt			[8+1];	///< 요청 조회 대상일		
	char read_time			[6+1];	///< 요청 조회 대상 시각		
	char bus_cls_cd			[3+1];	///< 버스등급코드			
	char bef_aft_dvs		[1+1];	///< 이전이후구분			
	char req_rec_num		[5+1];	///< 요청 레코드 수			

} stk_readalcn_t, *pstk_readalcn_t;

//>>

typedef struct
{
	char rot_id				[17+1];	///< 노선ID
	char rot_sqno			[5+1];	///< 노선순번(number)
	char alcn_dt			[8+1];	///< 배차일자
	char alcn_sqno			[4+1];	///< 배차순번(number)
	char atl_depr_dt		[8+1];	///< 실제출발일자
	char atl_depr_time		[6+1];	///< 실제출발시각
	char depr_time			[6+1];	///< 출발시각
	char rot_nm				[100+1];///< 노선명 
	char rot_bus_dvs_cd		[1+1];	///< 노선종류코드
	char bus_cls_cd			[3+1];	///< 버스등급코드
	char bus_cacm_cd		[4+1];	///< 버스운수사코드
	char alcn_way_dvs_cd	[1+1];	///< 배차방식구분코드
	char sati_use_yn		[1+1];	///< 좌석제사용여부
	char perd_temp_yn		[1+1];	///< 정기임시여부
	char bcnl_yn			[1+1];	///< 결행여부
	char dist				[18+1];	///< 거리(number)
	char take_drtm			[18+1];	///< 소요시간(number)
	char rdhm_mltp_val		[100+1];///< 승차홈다중값
	char sats_num			[5+1];	///< 좌석 수(number)
	char rmn_scnt			[5+1];	///< 잔여좌석수(number)
	char ocnt				[5+1];	///< 무표수(number)
	char trml_tisu_psb_yn	[1+1];	///< 터미널발권가능여부
	char cty_bus_oprn_shp_cd[1+1];	///< 시외버스운행형태코드
	char dc_psb_yn			[1+1];	///< 할인가능유무
	char whch_tissu_yn		[1+1];	///< 휠체어발권여부 // 20211206 ADD

	// 	int  n_rot_sqno			;		///< 노선순번(number)
	// 	int  n_alcn_sqno		;		///< 배차순번(number)
	// 	int  n_sats_num			;		///< 좌석 수(number)
	// 	int  n_rmn_scnt			;		///< 잔여좌석수(number)
	// 	int  n_ocnt				;		///< 무표수(number)

} rtk_readalcn_list_t, *prtk_readalcn_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char alcn_num			[5+1];	///< 배차 수
	rtk_readalcn_list_t* pList;		///< 
} rtk_readalcn_t, *prtk_readalcn_t;

//------------------------------
// IF_SV_131 : 배차 요금 조회

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
	//char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 id	

	char rot_id				[17+1];	///< 노선ID	
	char rot_sqno			[5+1];	///< 노선순번	
	char alcn_dt			[8+1];	///< 배차일자	
	char alcn_sqno			[4+1];	///< 배차순번	
	char depr_trml_cd		[7+1];	///< 출발터미널코드
	char arvl_trml_cd		[7+1];	///< 도착터미널코드
} stk_readalcnfee_t, *pstk_readalcnfee_t;

//>>

typedef struct
{
	char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드
	char dcrt_dvs_cd		[1+1];	///< 할인율구분코드
	char dc_rng_mltp_val	[100+1];///< 할인구간다중값
	char adpt_ltn_hcnt		[5+1];	///< 적용제한매수(number)
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드
	char ride_fee			[18+1];	///< 승차요금(number)
	char dc_aft_amt			[18+1];	///< 할인이후금액(number)
} rtk_readalcnfee_list_t, *prtk_readalcnfee_list_t;

// 20211206 ADD~
typedef struct
{
	char sats_no				[5+1];	///< 좌석번호(number)
	char whch_sats_no			[5+1];	///< 휠체어좌석번호(number)
	char whch_sats_no_prin_nm	[40+1]; ///< 휠체어좌석번호출력명
} rtk_readalcnwhch_list_t, *prtk_readalcnwhch_list_t;
// 20211206 ~ADD

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char sats_arct_cd		[3+1];	///< 좌석배치도유형코드
	char sats_sta_inf		[45+1];	///< 좌석상태정보
	char rsd_nop_card_dc_yn	[1+1];	///< 상주인원카드할인여부
	char fee_inf_num		[5+1];	///< 요금 정보 수
	char whch_sats_num		[5+1];	///< 휠체어좌석수		// 20211206 ADD
	rtk_readalcnfee_list_t* pList;	///< 
	rtk_readalcnwhch_list_t* pWhchList;	///<			// 20211206 ADD
} rtk_readalcnfee_t, *prtk_readalcnfee_t;

//------------------------------
// IF_SV_132 : 승차권 발행 - 현금

typedef struct
{
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드
	char sats_pcpy_id		[17+1];	///< 좌석선점id		
	char sats_no			[5+1];	///< 좌석번호	(number)
} stk_pubtckcash_list_t, *pstk_pubtckcash_list_t;

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
	//char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 id	

	char rot_id				[17+1];	///< 노선id			
	char rot_sqno			[5+1];	///< 노선순번	(number)	
	char alcn_dt			[8+1];	///< 배차일자		
	char alcn_sqno			[4+1];	///< 배차순번	(number)	
	char depr_trml_cd		[7+1];	///< 출발터미널코드	
	char arvl_trml_cd		[7+1];	///< 도착터미널코드	
	char pyn_mns_dvs_cd		[1+1];	///< 지불수단구분코드
	char pub_chnl_dvs_cd	[1+1];	///< 발행채널구분코드
	char tisu_way_dvs_cd	[1+1];	///< 발권방식구분코드
	char user_dvs_no		[20+1];	///< 사용자구분번호	

	char pub_num			[5+1];	///< 발행매수	(number)
	stk_pubtckcash_list_t   List[50];

} stk_pubtckcash_t, *pstk_pubtckcash_t;

// ====

typedef struct  
{
	char pyn_mns_dvs_cd		[1+1];	///< 지불수단구분코드		
	char csrc_aprv_no		[100+1];///< 현금영수증승인번호		
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드		
	char pub_sno			[4+1];	///< 발행일련번호 (number)			
	char bus_tck_inhr_no	[8+1];	///< 버스티켓고유번호		
	char tisu_amt			[4+1];	///< 발권금액 (number)
	char sats_no			[4+1];	///< 좌석번호 (number)			
	char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드	
	char dcrt_dvs_cd		[1+1];	///< 할인율구분코드			
	char dc_bef_amt			[18+1];	///< 할인이전금액 (number)
} rtk_pubtckcash_list_t, *prtk_pubtckcash_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char csrc_auth_no		[100+1];///< 현금영수증인증번호		
	char csrc_aprv_vanc_cd	[2+1];	///< 현금영수증승인van사코드	
	char csrc_brn			[12+1];	///< 현금영수증사업자등록번호
	char atl_depr_dt		[8+1];	///< 실제출발일자			
	char atl_depr_time		[6+1];	///< 실제출발시각			
	char depr_trml_cd		[7+1];	///< 출발터미널코드			
	char arvl_trml_cd		[7+1];	///< 도착터미널코드			
	char stpt_trml_cd		[7+1];	///< 기점터미널코드			
	char ept_trml_cd		[7+1];	///< 종점터미널코드			
	char bus_cls_cd			[3+1];	///< 버스등급코드			
	char bus_cacm_cd		[4+1];	///< 버스운수사코드			
	char rdhm_mltp_val		[100+1];	///< 승차홈다중값			
	char dist				[18+1];	///< 거리 (number)					
	char take_drtm			[18+1];	///< 소요시간 (number)
	char bus_cls_prin_yn	[1+1];	///< 버스등급출력여부		
	char bus_cacm_nm_prin_yn[1+1];	///< 버스운수사명출력여부	
	char depr_time_prin_yn	[1+1];	///< 출발시각출력여부		
	char sats_no_prin_yn	[1+1];	///< 좌석번호출력여부		
	char alcn_way_dvs_cd	[1+1];	///< 배차방식구분코드		
	char perd_temp_yn		[1+1];	///< 정기임시여부			
	char pub_dt				[8+1];	///< 발행일자				
	char pub_time			[6+1];	///< 발행시각				
	char pub_shct_trml_cd	[4+1];	///< 발행단축터미널코드		
	char pub_wnd_no			[2+1];	///< 발행창구번호			

	char pub_num			[5+1];	///< 발행매수 (number)	
	rtk_pubtckcash_list_t*  pList;

} rtk_pubtckcash_t, *prtk_pubtckcash_t;

//------------------------------
// IF_SV_133 : 승차권 발행 - 카드현장

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
	//char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 id	

	char read_rot_yn		[1+1];	///< 예매자료조회여부	
	char card_pwd_inp_yn	[1+1];	///< 카드비밀번호입력여부
	char rot_id				[17+1];	///< 노선id				
	char rot_sqno			[5+1];	///< 노선순번			
	char alcn_dt			[8+1];	///< 배차일자			
	char alcn_sqno			[4+1];	///< 배차순번			
	char depr_trml_cd		[7+1];	///< 출발터미널코드		
	char arvl_trml_cd		[7+1];	///< 도착터미널코드		
	char pyn_mns_dvs_cd		[1+1];	///< 지불수단구분코드	
	char pub_chnl_dvs_cd	[1+1];	///< 발행채널구분코드	
	char tisu_way_dvs_cd	[1+1];	///< 발권방식구분코드	
	char damo_enc_card_no	[1024+1];	///< 카드번호암호문		
	char damo_enc_card_no_len[4+1];	///< 카드번호암호문길이	
	char damo_enc_dta_key	[32+1];	///< 암호문키			
	char card_pwd			[100+1];	///< 카드 비밀번호		
	char spad_no			[16+1];	///< 싸인패드번호		
	char spad_dta			[3072+1];	///< 싸인패드데이터		
	char spad_dta_len		[8+1];	///< 싸인패드데이터길이	
	char spad_hash_cd		[32+1];	///< 싸인패드해시코드	
	char user_dvs_no		[20+1];	///< 사용자구분번호		
	char pub_num			[5+1];	///< 발행매수			
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드	
	char sats_pcpy_id		[17+1];	///< 좌석선점id			
	char sats_no			[5+1];	///< 좌석번호			
} stk_pubtckcard_t, *pstk_pubtckcard_t;

typedef struct
{
	char rsp_cd					[6+1];	///< 응답코드		
	char rsp_msg				[100+1];	///< 응답 메시지		
	char card_no				[100+1];	///< 카드번호		
	char card_aprv_no			[100+1];	///< 카드승인번호		
	char card_aprv_amt			[18+1];	///< 카드승인금액	(number)
	char card_aprv_vanc_cd		[2+1];	///< 카드승인van사코드	
	char issu_crcm_cd			[4+1];	///< 발급카드사코드	
	char buy_crcm_cd			[4+1];	///< 매입카드사코드	
	char frc_cmrt				[5+1];	///< 가맹점수수료율 (number)
	char atl_depr_dt			[8+1];	///< 실제출발일자		
	char atl_depr_time			[6+1];	///< 실제출발시각		
	char depr_trml_cd			[7+1];	///< 출발터미널코드	
	char arvl_trml_cd			[7+1];	///< 도착터미널코드	
	char stpt_trml_cd			[7+1];	///< 기점터미널코드	
	char ept_trml_cd			[7+1];	///< 종점터미널코드	
	char bus_cls_cd				[3+1];	///< 버스등급코드		
	char bus_cacm_cd			[4+1];	///< 버스운수사코드	
	char rdhm_mltp_val			[100+1];	///< 승차홈다중값		
	char dist					[18+1];	///< 거리 (number)
	char take_drtm				[18+1];	///< 소요시간	 (number)
	char bus_cls_prin_yn		[1+1];	///< 버스등급출력여부	
	char bus_cacm_nm_prin_yn	[1+1];	///< 버스운수사명출력여부
	char depr_time_prin_yn		[1+1];	///< 출발시각출력여부	
	char sats_no_prin_yn		[1+1];	///< 좌석번호출력여부	
	char alcn_way_dvs_cd		[1+1];	///< 배차방식구분코드	
	char perd_temp_yn			[1+1];	///< 정기임시여부		
	char pub_dt					[8+1];	///< 발행일자		
	char pub_time				[6+1];	///< 발행시각		
	char pub_shct_trml_cd		[4+1];	///< 발행단축터미널코드	
	char pub_wnd_no				[2+1];	///< 발행창구번호		
	char pub_num				[5+1];	///< 발행매수	 (number)	
	char pyn_mns_dvs_cd			[1+1];	///< 지불수단구분코드	
	char bus_tck_knd_cd			[4+1];	///< 버스티켓종류코드	
	char pub_sno				[4+1];	///< 발행일련번호	(number)	
	char bus_tck_inhr_no		[8+1];	///< 버스티켓고유번호	
	char tisu_amt				[18+1];	///< 발권금액	 (number)	
	char sats_no				[5+1];	///< 좌석번호 (number)		
	char cty_bus_dc_knd_cd		[1+1];	///< 시외버스할인종류코드
	char dcrt_dvs_cd			[1+1];	///< 할인율구분코드	
	char dc_bef_amt				[18+1];	///< 할인이전금액 (number)		

} rtk_pubtckcard_t, *prtk_pubtckcard_t;

//------------------------------
// IF_SV_134 : 승차권 발행 - 현금영수증

typedef struct
{
	char bus_tck_knd_cd				[4+1];	///< 버스티켓종류코드			
	char sats_pcpy_id				[17+1];	///< 좌석선점id					
	char sats_no					[5+1];	///< 좌석번호					
} stk_pubtckcsrc_list_t, *pstk_pubtckcsrc_list_t;

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs				[3+1];	///< 요청 프로그램 구분			
	//char req_trml_cd				[7+1];	///< 요청 터미널 코드			
	//char req_trml_wnd_no			[2+1];	///< 요청 터미널 창구 번호		
	//char req_trml_user_ip			[40+1];	///< 요청 터미널 사용자 ip		
	//char req_trml_user_id			[17+1];	///< 요청 터미널 사용자 id		

	char rot_id						[17+1];	///< 노선id						
	char rot_sqno					[5+1];	///< 노선순번					
	char alcn_dt					[8+1];	///< 배차일자					
	char alcn_sqno					[4+1];	///< 배차순번					
	char depr_trml_cd				[7+1];	///< 출발터미널코드				
	char arvl_trml_cd				[7+1];	///< 도착터미널코드				
	char tisu_way_dvs_cd			[1+1];	///< 발권방식구분코드			
	char pyn_mns_dvs_cd				[1+1];	///< 지불수단구분코드			
	char pub_chnl_dvs_cd			[1+1];	///< 발행채널구분코드			
	char damo_enc_csrc_auth_no		[1024+1];///< 현금영수증인증번호암호문	
	char damo_enc_csrc_auth_no_len	[4+1];	///< 현금영수증인증번호암호문길이
	char damo_enc_dta_key			[32+1];	///< 암호문키					
	char cprt_yn					[1+1];	///< 법인여부 ('N':개인, 'Y':법인)					
	char user_dvs_no				[20+1];	///< 사용자구분번호				

	char pub_num					[5+1];	///< 발행매수					
	stk_pubtckcsrc_list_t	List[50];
} stk_pubtckcsrc_t, *pstk_pubtckcsrc_t;

//=====================

typedef struct 
{
	char pyn_mns_dvs_cd			[1+1];	///< 지불수단구분코드		
	char csrc_aprv_no			[100+1];///< 현금영수증승인번호		
	char bus_tck_knd_cd			[4+1];	///< 버스티켓종류코드		
	char pub_sno				[4+1];	///< 발행일련번호 (number)			
	char bus_tck_inhr_no		[8+1];	///< 버스티켓고유번호		
	char tisu_amt				[18+1];	///< 발권금액	 (number)			
	char sats_no				[5+1];	///< 좌석번호	 (number)			
	char cty_bus_dc_knd_cd		[1+1];	///< 시외버스할인종류코드	
	char dcrt_dvs_cd			[1+1];	///< 할인율구분코드			
	char dc_bef_amt				[18+1];	///< 할인이전금액 (number)
} rtk_pubtckcsrc_list_t, *prtk_pubtckcsrc_list_t;

typedef struct
{
	char rsp_cd					[6+1];	///< 응답코드				
	char rsp_msg				[100+1];///< 응답 메시지				
	char csrc_auth_no			[100+1];///< 현금영수증인증번호		
	char csrc_brn				[12+1];	///< 현금영수증사업자등록번호
	char csrc_aprv_vanc_cd		[2+1];	///< 현금영수증승인van사코드	
	char atl_depr_dt			[8+1];	///< 실제출발일자			
	char atl_depr_time			[6+1];	///< 실제출발시각			
	char depr_trml_cd			[7+1];	///< 출발터미널코드			
	char arvl_trml_cd			[7+1];	///< 도착터미널코드			
	char stpt_trml_cd			[7+1];	///< 기점터미널코드			
	char ept_trml_cd			[7+1];	///< 종점터미널코드			
	char bus_cls_cd				[3+1];	///< 버스등급코드			
	char bus_cacm_cd			[4+1];	///< 버스운수사코드			
	char rdhm_mltp_val			[100+1];	///< 승차홈다중값			
	char dist					[18+1];	///< 거리 (number)					
	char take_drtm				[18+1];	///< 소요시간	 (number)			
	char bus_cls_prin_yn		[1+1];	///< 버스등급출력여부		
	char bus_cacm_nm_prin_yn	[1+1];	///< 버스운수사명출력여부	
	char depr_time_prin_yn		[1+1];	///< 출발시각출력여부		
	char sats_no_prin_yn		[1+1];	///< 좌석번호출력여부		
	char alcn_way_dvs_cd		[1+1];	///< 배차방식구분코드		
	char perd_temp_yn			[1+1];	///< 정기임시여부			
	char pub_dt					[8+1];	///< 발행일자				
	char pub_time				[6+1];	///< 발행시각				
	char pub_shct_trml_cd		[4+1];	///< 발행단축터미널코드		
	char pub_wnd_no				[2+1];	///< 발행창구번호			

	char pub_num				[5+1];	///< 발행매수	 (number)			
	rtk_pubtckcsrc_list_t* pList;

} rtk_pubtckcsrc_t, *prtk_pubtckcsrc_t;

//------------------------------
// IF_SV_135 : 발행내역 조회(재발행용)

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분		
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드		
	//char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 IP	
	//char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 ID	

	char inqr_dt			[8+1];	///< 조회 대상 일자		
	char inqr_dvs			[1+1];	///< 조회 구분			
	char inqr_wnd_no		[2+1];	///< 조회 대상 창구번호		
	char user_id			[17+1];	///< 사용자ID			
} stk_readpubptr_t, *pstk_readpubptr_t;

//>>

typedef struct
{
	char pub_dt				[8+1];	///< 발행일자		
	char pub_shct_trml_cd	[4+1];	///< 발행단축터미널코드	
	char pub_wnd_no			[2+1];	///< 발행창구번호		
	char pub_sno			[4+1];	///< 발행일련번호		
	char pub_user_id		[17+1];	///< 발행사용자ID	
	char depr_dt			[8+1];	///< 출발일자		
	char depr_time			[6+1];	///< 출발시각		
	char pyn_mns_dvs_cd		[1+1];	///< 지불수단구분코드	
	char tisu_chnl_dvs_cd	[1+1];	///< 발권채널구분코드	
	char pub_chnl_dvs_cd	[1+1];	///< 발행채널구분코드	
	char rot_id				[17+1];	///< 노선ID		
	char rot_nm				[100+1];///< 노선명			
	char tisu_amt			[18+1];	///< 발권금액		
	char sats_no			[5+1];	///< 좌석번호		
	char depr_trml_cd		[7+1];	///< 출발터미널코드	
	char arvl_trml_cd		[7+1];	///< 도착터미널코드	
	char bus_cls_cd			[3+1];	///< 버스등급코드		
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드	
	char canc_ry_dvs_cd		[1+1];	///< 취소환불구분코드	
	char pub_time			[6+1];	///< 발행시각		
	char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드
	char dcrt_dvs_cd		[1+1];	///< 할인율구분코드	
	char dc_bef_amt			[18+1];	///< 할인이전금액		
	char pub_sys_dvs_cd		[1+1];	///< 발행시스템구분코드	
	char eb_bus_tck_sno		[22+1];	///< EB버스티켓일련번호	
	//char whch_sats_no_prin_nm	[40+1];	///< 휠체어좌석번호출력명	// 20221208 ADD				
	//char prcd_alcn_ride_yn		[1+1];	///< 선행배차승차여부 	// 20221208 ADD				
	char qr_pym_pyn_dtl_cd		[2+1];	///< QR결제지불상세코드	// 20221208 ADD				
} rtk_readpubptr_list_t, *prtk_readpubptr_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char pub_num			[5+1];	///< 발행매수
	rtk_readpubptr_list_t* pList;
} rtk_readpubptr_t, *prtk_readpubptr_t;

//------------------------------
// IF_SV_136 : 버스티켓 재발행

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분		
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드		
	//char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 IP	
	//char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 ID	

	char pub_dt				[8+1];	///< 발행일자			
	char pub_shct_trml_cd	[4+1];	///< 발행단축터미널코드		
	char pub_wnd_no			[2+1];	///< 발행창구번호			
	char pub_sno			[4+1];	///< 발행일련번호			
} stk_rpubtck_t, *pstk_rpubtck_t;

//>>

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드			
	char pyn_mns_dvs_cd		[1+1];	///< 지불수단구분코드		
	char card_no			[100+1];///< 카드번호			
	char card_aprv_no		[100+1];///< 카드승인번호			
	char card_aprv_amt		[18+1];	///< 카드승인금액			
	char card_aprv_vanc_cd	[2+1];	///< 카드승인VAN사코드		
	char csrc_aprv_vanc_cd	[2+1];	///< 현금영수증승인VAN사코드	
	char csrc_aprv_no		[100+1];///< 현금영수증승인번호		
	char csrc_auth_no		[100+1];///< 현금영수증인증번호		
	char csrc_brn			[12+1];	///< 현금영수증사업자등록번호	
	char cprt_yn			[1+1];	///< 법인여부			
	char issu_crcm_cd		[4+1];	///< 발급카드사코드		
	char buy_crcm_cd		[4+1];	///< 매입카드사코드		
	char frc_cmrt			[5+1];	///< 가맹점수수료율		
	// 20211015 ADD
	char evcp_pym_amt		[18+1];	///< 이벤트쿠폰결제금액				
	char mlg_pym_amt		[18+1];	///< 마일리지결제금액				
	// 20211015 ~ADD
	char atl_depr_dt		[8+1];	///< 실제출발일자			
	char atl_depr_time		[6+1];	///< 실제출발시각			
	char depr_trml_cd		[7+1];	///< 출발터미널코드		
	char arvl_trml_cd		[7+1];	///< 도착터미널코드		
	char stpt_trml_cd		[7+1];	///< 기점터미널코드		
	char ept_trml_cd		[7+1];	///< 종점터미널코드		
	char bus_cls_cd			[3+1];	///< 버스등급코드			
	char bus_cacm_cd		[4+1];	///< 버스운수사코드		
	char rdhm_mltp_val		[100+1];///< 승차홈다중값			
	char dist				[18+1];	///< 거리				
	char take_drtm			[18+1];	///< 소요시간			
	char bus_cls_prin_yn	[1+1];	///< 버스등급출력여부		
	char bus_cacm_nm_prin_yn[1+1];	///< 버스운수사명출력여부	
	char depr_time_prin_yn	[1+1];	///< 출발시각출력여부		
	char sats_no_prin_yn	[1+1];	///< 좌석번호출력여부		
	char alcn_way_dvs_cd	[1+1];	///< 배차방식구분코드		
	char sati_use_yn		[1+1];	///< 좌석제사용여부		
	char perd_temp_yn		[1+1];	///< 정기임시여부			
	char pub_dt				[8+1];	///< 발행일자			
	char pub_time			[6+1];	///< 발행시각			
	char pub_shct_trml_cd	[4+1];	///< 발행단축터미널코드		
	char pub_wnd_no			[2+1];	///< 발행창구번호			
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드		
	char pub_sno			[4+1];	///< 발행일련번호			
	char bus_tck_inhr_no	[8+1];	///< 버스티켓고유번호		
	char tisu_amt			[18+1];	///< 발권금액			
	char sats_no			[5+1];	///< 좌석번호			
	char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드	
	char dcrt_dvs_cd		[1+1];	///< 할인율구분코드		
	char dc_bef_amt			[18+1];	///< 할인이전금액			
	char cty_bus_oprn_shp_cd[1+1];	///< 시외버스운행형태코드	
	char user_dvs_no		[20+1];	///< 사용자구분번호		
	char user_nm			[100+1];///< 사용자명			
	char cmpy_nm			[100+1];///< 회사명				
	char mip_term			[2+1];	///< 할부기간			
	//char whch_sats_no_prin_nm	[40+1];	///< 휠체어좌석번호출력명	// 20221205 ADD				
	char qr_pym_pyn_dtl_cd		[2+1];	///< QR결제지불상세코드	// 20221205 ADD				
} rtk_rpubtck_t, *prtk_rpubtck_t;

//------------------------------
// IF_SV_137 : 홈티켓/인터넷/모바일 예매 조회

typedef struct
{
	stk_head_id_t hdr_id;
// 	char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분	
// 	char req_trml_cd		[7+1];	///< 요청 터미널 코드		
// 	char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
// 	char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 IP	
// 	char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 ID	

	char req_read_dvs_cd	[1+1];	///< 요청 조회 구분		
	char read_wnd_dvs		[1+1];	///< 조회 창구 구분		
	char card_no			[1024+1];	///< 카드번호			
	char pos_pg_ktc_ver		[20+1];	///< POS단말기버전정보		
	char enc_dta_len		[4+1];	///< ENC 데이터 길이		
	char enc_dta			[1024+1];	///< ENC 데이터			
	char mnpp_tel_no		[50+1];	///< 예약자전화번호		
	char mnpp_brdt			[6+1];	///< 예약자생년월일		
	char mrs_no				[17+1];	///< 예매번호			
} stk_readmrnppt_t, *pstk_readmrnppt_t;

//>>

typedef struct
{
	char mrs_no				[17+1];	///< 예매번호				
	char tisu_id			[17+1];	///< 발권id					
	char tisu_dt			[8+1];	///< 발권일자				
	char tisu_time			[6+1];	///< 발권시각				
	char tisu_chnl_dvs_cd	[1+1];	///< 발권채널구분코드		
	char pub_chnl_dvs_cd	[1+1];	///< 발행채널구분코드		
	char pub_dt				[8+1];	///< 발행일자				
	char pub_time			[6+1];	///< 발행시각				
	char tisu_amt			[18+1];	///< 발권금액				
	char pym_aprv_id		[17+1];	///< 결제승인id				
	char depr_trml_cd		[7+1];	///< 출발터미널코드			
	char arvl_trml_cd		[7+1];	///< 도착터미널코드			
	char rot_id				[17+1];	///< 노선id					
	char rot_sqno			[5+1];	///< 노선순번				
	char rot_nm				[100+1];///< 노선명					
	char depr_dt			[8+1];	///< 출발일자				
	char depr_time			[6+1];	///< 출발시각				
	char bus_cls_cd			[3+1];	///< 버스등급코드			
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드		
	char sats_no			[5+1];	///< 좌석번호				
	char card_no			[100+1];///< 카드번호				
	char cty_bus_sys_dvs_cd	[1+1];	///< 시외버스시스템구분코드	
	char tisu_sys_dvs_cd	[1+1];	///< 발권시스템구분코드		
	char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드	
	char dcrt_dvs_cd		[1+1];	///< 할인율구분코드			
	char dc_bef_amt			[18+1];	///< 할인이전금액			
	char alcn_dt			[8+1];	///< 배차일자				
	char alcn_sqno			[4+1];	///< 배차순번				
	char rpub_num			[5+1];	///< 재발행매수				
} rtk_readmrnppt_list_t, *prtk_readmrnppt_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char mrs_num			[5+1];	///< 예매매수
	rtk_readmrnppt_list_t* pList;	///< 
} rtk_readmrnppt_t, *prtk_readmrnppt_t;

//------------------------------
// IF_SV_138 : 홈티켓/인터넷/모바일 예매 발행

typedef struct
{
	char tisu_id			[17+1];	///< 발권ID			
} stk_pubmrnp_list_t, *pstk_pubmrnp_list_t;

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분	
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드		
	//char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 IP
	//char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 ID	

	char pub_chnl_dvs_cd	[1+1];	///< 발행채널구분코드		
	char mrs_no				[17+1];	///< 예매번호			
	char card_no			[100+1];	///< 카드번호			
	char cty_bus_sys_dvs_cd	[1+1];	///< 시외버스시스템구분코드	
	char tisu_sys_dvs_cd	[1+1];	///< 발권시스템구분코드		
	char read_wnd_dvs		[1+1];	///< 조회 창구 구분		
	char pub_num			[5+1];	///< 발행매수			
	stk_pubmrnp_list_t List[30];
} stk_pubmrnp_t, *pstk_pubmrnp_t;

//>>

typedef struct
{
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드					
	char pub_sno			[4+1];	///< 발행일련번호						
	char bus_tck_inhr_no	[8+1];	///< 버스티켓고유번호					
	char tisu_amt			[18+1];	///< 발권금액							
	char sats_no			[5+1];	///< 좌석번호							
	char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드				
	char dcrt_dvs_cd		[1+1];	///< 할인율구분코드						
	char dc_bef_amt			[18+1];	///< 할인이전금액		
} rtk_pubmrnp_list_t, *prtk_pubmrnp_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char rsp_msg			[100+1];///< 응답 메시지		

	char pyn_mns_dvs_cd		[1+1];	///< 지불수단구분코드	
	char card_no			[100+1];///< 카드번호			
	char card_aprv_no		[100+1];///< 카드승인번호		
	char card_aprv_amt		[18+1];	///< 카드승인금액		
	char card_aprv_vanc_cd	[2+1];	///< 카드승인van사코드	
	char issu_crcm_cd		[4+1];	///< 발급카드사코드		
	char buy_crcm_cd		[4+1];	///< 매입카드사코드		
	char frc_cmrt			[5+1];	///< 가맹점수수료율		
	// 20211015 ADD
	char evcp_pym_amt		[18+1];	///< 이벤트쿠폰결제금액				
	char mlg_pym_amt		[18+1];	///< 마일리지결제금액				
	// 20211015 ~ADD
	char atl_depr_dt		[8+1];	///< 실제출발일자		
	char atl_depr_time		[6+1];	///< 실제출발시각		
	char depr_trml_cd		[7+1];	///< 출발터미널코드		
	char arvl_trml_cd		[7+1];	///< 도착터미널코드		
	char stpt_trml_cd		[7+1];	///< 기점터미널코드		
	char ept_trml_cd		[7+1];	///< 종점터미널코드		
	char bus_cls_cd			[3+1];	///< 버스등급코드		
	char bus_cacm_cd		[4+1];	///< 버스운수사코드		
	char rdhm_mltp_val		[100+1];///< 승차홈다중값		
	char cty_bus_oprn_shp_cd[1+1];	///< 시외버스운행형태코드
	char dist				[18+1];	///< 거리				
	char take_drtm			[18+1];	///< 소요시간			
	char bus_cls_prin_yn	[1+1];	///< 버스등급출력여부	
	char bus_cacm_nm_prin_yn[1+1];	///< 버스운수사명출력여부
	char depr_time_prin_yn	[1+1];	///< 출발시각출력여부	
	char sats_no_prin_yn	[1+1];	///< 좌석번호출력여부	
	char alcn_way_dvs_cd	[1+1];	///< 배차방식구분코드	
	char sati_use_yn		[1+1];	///< 좌석제사용여부		
	char perd_temp_yn		[1+1];	///< 정기임시여부		
	char pub_dt				[8+1];	///< 발행일자			
	char pub_time			[6+1];	///< 발행시각			
	char pub_shct_trml_cd	[4+1];	///< 발행단축터미널코드	
	char pub_wnd_no			[2+1];	///< 발행창구번호		
	char user_dvs_no		[20+1];	///< 사용자구분번호		
	char user_nm			[100+1];///< 사용자명			
	char cmpy_nm			[100+1];///< 회사명				

	char pub_num			[5+1];	///< 발행매수			
	rtk_pubmrnp_list_t* pList;		///< 

} rtk_pubmrnp_t, *prtk_pubmrnp_t;

//------------------------------
// IF_SV_145 : 버스티켓 취소/환불

typedef struct 
{
	char tisu_sys_dvs_cd	[1+1];	///< 발권시스템구분코드	
	char pub_sys_dvs_cd		[1+1];	///< 발행시스템구분코드	
	char pub_dt				[8+1];	///< 발행일자		
	char pub_shct_trml_cd	[4+1];	///< 발행단축터미널코드	
	char pub_wnd_no			[2+1];	///< 발행창구번호		
	char pub_sno			[4+1];	///< 발행일련번호		
	char eb_bus_tck_sno		[22+1];	///< EB버스티켓일련번호	
	char ryrt_dvs_cd		[1+1];	///< 환불율구분코드	
	char pyn_mns_dvs_cd		[1+1];	///< 지불수단구분코드	
	char tisu_amt			[18+1];	///< 발권금액		
} stk_cancrytck_list_t, *pstk_cancrytck_list_t;

typedef struct
{
	stk_head_id_t	hdr_id;

	char canc_ry_dvs_cd		[1+1];	///< 취소환불구분코드	
	char ryrt_num			[2+1];	///< 환불매수		
	stk_cancrytck_list_t	List[10];
} stk_cancrytck_t, *pstk_cancrytck_t;

//>>

typedef struct
{
	char pyn_mns_dvs_cd		[1+1];	///< 지불수단구분코드
	char canc_ry_amt		[18+1];	///< 취소환불금액
} rtk_cancrytck_list_t, *prtk_cancrytck_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char rsp_msg			[100+1];///< 응답 메시지

	char pyn_mns_num		[1+1];	///< 발행매수			
	rtk_cancrytck_list_t* pList;	///< 

} rtk_cancrytck_t, *prtk_cancrytck_t;

//------------------------------
// IF_SV_150 : 현금영수증 사후등록

typedef struct 
{
	char pub_dt						[8+1];	///< 발행일자		
	char pub_shct_trml_cd			[4+1];	///< 발행단축터미널코드	
	char pub_wnd_no					[2+1];	///< 발행창구번호		
	char pub_sno					[4+1];	///< 발행일련번호		
} stk_crrp_t_list_t, *pstk_crrp_t_list_t;

typedef struct 
{
	stk_head_id_t	hdr_id;

	char csrc_aprv_no				[100+1];	///< 현금영수증인증번호			
	char damo_enc_csrc_auth_no		[1024+1];	///< 현금영수증인증번호암호문		
	char damo_enc_csrc_auth_no_len	[4+1];		///< 현금영수증인증번호암호문길이	
	char damo_enc_dta_key			[32+1];		///< 암호문키				
	char cprt_yn					[1+1];		///< 법인여부				
	char rgt_num					[4+1];		///< 등록매수
	stk_crrp_t_list_t	List[30];
} stk_crrp_t, *pstk_crrp_t;

typedef struct 
{
	char rsp_cd						[6+1];		///< 응답코드		
	char rsp_msg					[100+1];	///< 응답 메시지		
	char csrc_aprv_no				[100+1];	///< 현금영수증승인번호
} rtk_crrp_t, *prtk_crrp_t;

//------------------------------
// IF_SV_152 : 버스티켓 교환 요청

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분	
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드		
	//char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 ip	
	//char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 ID	

	char bus_tck_inhr_no	[8+1];	///< 버스티켓고유번호	
	char add_bus_tck_inhr_no[8+1];	///< 추가버스티켓고유번호
} stk_excbustck_t, *pstk_excbustck_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
} rtk_excbustck_t, *prtk_excbustck_t;

//------------------------------
// IF_SV_155 : 환불내역 조회

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		 [3+1];	///< 요청 프로그램 구분	
	//char req_trml_cd		 [7+1];	///< 요청 터미널 코드		
	//char req_trml_wnd_no	 [2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip    [40+1];	///< 요청 터미널 사용자 IP	
	//char req_trml_user_id    [17+1];	///< 요청 터미널 사용자 ID	

	char read_dt			 [8+1];	///< 요청 조회 대상일		
	char read_time			 [6+1];	///< 요청 조회 대상 시각	
	char depr_trml_cd	     [7+1];	///< 출발터미널코드		
	char arvl_trml_cd	     [7+1];	///< 도착터미널코드		
} stk_readrypt_t, *pstk_readrypt_t;

typedef struct
{
	char pub_dt				    [8+1];	///< 발행일자		
	char pub_shct_trml_cd	    [4+1];	///< 발행단축터미널코드	
	char pub_wnd_no			    [2+1];	///< 발행창구번호		
	char pub_sno				[4+1];	///< 발행일련번호		
	char rot_knd_cd			    [1+1];	///< 노선종류코드		
	char pyn_mns_dvs_cd		    [1+1];	///< 지불수단구분코드	
	char rot_nm				    [100+1];	///< 노선명			
	char pub_time			    [6+1];	///< 발행시각		
	char bus_tck_inhr_no		[8+1];	///< 버스티켓고유번호	
	char tisu_amt			    [18+1];	///< 발권금액		
	char sats_no				[5+1];	///< 좌석번호		
	char depr_dt				[8+1];	///< 출발일자		
	char depr_time			    [6+1];	///< 출발시각		
	char depr_trml_cd		    [7+1];	///< 출발터미널코드	
	char arvl_trml_cd		    [7+1];	///< 도착터미널코드	
	char bus_cls_cd			    [3+1];	///< 버스등급코드		
	char bus_tck_knd_cd		    [4+1];	///< 버스티켓종류코드	
	char canc_ry_dvs_cd		    [1+1];	///< 취소환불구분코드	
	char ryrt_dvs_cd			[1+1];	///< 환불율구분코드	
	char canc_ry_amt			[18+1];	///< 취소환불금액		
	char pub_user_id			[17+1];	///< 발행사용자id	
	char rgt_time			    [6+1];	///< 등록 시각		
	char rgt_user_id			[17+1];	///< 등록사용자id	
	char cty_bus_dc_knd_cd	    [1+1];	///< 시외버스할인종류코드
	char dcrt_dvs_cd			[1+1];	///< 할인율구분코드	
	char dc_bef_amt			    [18+1];	///< 할인이전금액		
	char pub_sys_dvs_cd		    [1+1];	///< 발행시스템구분코드	
	char eb_bus_tck_sno		    [22+1];	///< eb버스티켓일련번호
	//char lnkg_py_knd_cd			[1+1];	///< 연동페이종류코드		// 20221208 ADD				
	//char whch_sats_no_prin_nm	[40+1];	///< 휠체어좌석번호출력명	// 20221208 ADD				
	//char prcd_alcn_ride_yn		[1+1];	///< 선행배차승차여부 	// 20221208 ADD				
	char qr_pym_pyn_dtl_cd		[2+1];	///< QR결제지불상세코드	// 20221208 ADD				
} rtk_readrypt_list_t, *prtk_readrypt_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드		 
	char read_dta_num		[5+1];	///< 조회 데이터 건 수	
	rtk_readrypt_list_t*	pList;
} rtk_readrypt_t, *prtk_readrypt_t;

//------------------------------
// IF_SV_157 : 재발행내역 조회

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분		
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드		
	//char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 ip	
	//char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 id	

	char read_dt			[8+1];	///< 요청 조회 대상일		
	char read_time			[6+1];	///< 요청 조회 대상 시각		
	char depr_trml_cd		[7+1];	///< 출발터미널코드			
	char arvl_trml_cd		[7+1];	///< 도착터미널코드			
} stk_readrtckpt_t, *pstk_readrtckpt_t;

//>>

typedef struct
{
	char pub_dt				[8+1];	///< 발행일자			
	char pub_shct_trml_cd	[4+1];	///< 발행단축터미널코드	
	char pub_wnd_no			[2+1];	///< 발행창구번호		
	char pub_sno			[4+1];	///< 발행일련번호(int)		
	char rot_knd_cd			[1+1];	///< 노선종류코드		
	char pyn_mns_dvs_cd		[1+1];	///< 지불수단구분코드	
	char rot_nm				[100+1];	///< 노선명				
	char pub_time			[6+1];	///< 발행시각			
	char bus_tck_inhr_no	[8+1];	///< 버스티켓고유번호	
	char tisu_amt			[18+1];	///< 발권금액(int)			
	char sats_no			[5+1];	///< 좌석번호(int)
	char depr_dt			[8+1];	///< 출발일자			
	char depr_time			[6+1];	///< 출발시각			
	char depr_trml_cd		[7+1];	///< 출발터미널코드		
	char arvl_trml_cd		[7+1];	///< 도착터미널코드		
	char bus_cls_cd			[3+1];	///< 버스등급코드		
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드	
	char canc_ry_dvs_cd		[1+1];	///< 취소환불구분코드	
	char pub_user_id		[17+1];	///< 발행사용자id		
	char rgt_time			[6+1];	///< 등록 시각			
	char rgt_user_id		[17+1];	///< 등록사용자id		
	char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드
	char dcrt_dvs_cd		[1+1];	///< 할인율구분코드		
	char dc_bef_amt			[18+1];	///< 할인이전금액(int)		
	char pub_sys_dvs_cd		[1+1];	///< 발행시스템구분코드	
	char eb_bus_tck_sno		[22+1];	///< eb버스티켓일련번호	
	//char lnkg_py_knd_cd		[1+1];	///< 연동페이종류코드		// 20221208 ADD				
	//char whch_sats_no_prin_nm	[40+1];	///< 휠체어좌석번호출력명	// 20221208 ADD				
	//char prcd_alcn_ride_yn	[1+1];	///< 선행배차승차여부 	// 20221208 ADD				
	char qr_pym_pyn_dtl_cd		[2+1];	///< QR결제지불상세코드	// 20221208 ADD				
} rtk_readrtckpt_list_t, *prtk_readrtckpt_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char read_dta_num		[5+1];	///< 조회 데이터 건 수
	rtk_readrtckpt_list_t* pList;
} rtk_readrtckpt_t, *prtk_readrtckpt_t;

//------------------------------
// IF_SV_158 : 버스티켓 일련번호 조회

typedef struct
{
	stk_head_id_t	hdr_id;

	char pub_sys_dvs_cd		[1+1];	///< 발행시스템구분코드	
	char pub_dt				[8+1];	///< 발행일자		
	char pub_shct_trml_cd	[4+1];	///< 발행단축터미널코드	
	char pub_wnd_no			[2+1];	///< 발행창구번호		
	char pub_sno			[4+1];	///< 발행일련번호		
	char eb_bus_tck_sno		[22+1];	///< eb버스티켓일련번호
} stk_readbustckno_t, *pstk_readbustckno_t;

//>>

typedef struct
{
	char rpub_dt			[8+1];	///< 재발행일자	
	char rpub_time			[6+1];	///< 재발행시각	
	char rpub_user_id		[17+1];	///< 재발행사용자id
} rtk_readbustckno_list_t, *prtk_readbustckno_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	
	char rot_knd_cd			[1+1];	///< 노선종류코드		
	char pyn_mns_dvs_cd		[1+1];	///< 지불수단구분코드	
	char rot_id				[17+1];	///< 노선id		
	char rot_nm				[100+1];///< 노선명			
	char rot_sqno			[5+1];	///< 노선순번		
	char alcn_dt			[8+1];	///< 배차일자		
	char alcn_sqno			[4+1];	///< 배차순번		
	char pub_time			[6+1];	///< 발행시각		
	char bus_tck_inhr_no	[8+1];	///< 버스티켓고유번호	
	char tisu_amt			[18+1];	///< 발권금액		
	char sats_no			[5+1];	///< 좌석번호		
	char depr_dt			[8+1];	///< 출발일자		
	char depr_time			[6+1];	///< 출발시각		
	char depr_trml_cd		[7+1];	///< 출발터미널코드	
	char arvl_trml_cd		[7+1];	///< 도착터미널코드	
	char stpt_trml_cd		[7+1];	///< 기점터미널코드	
	char ept_trml_cd		[7+1];	///< 종점터미널코드	
	char bus_cls_cd			[3+1];	///< 버스등급코드		
	char bus_cacm_cd		[4+1];	///< 버스운수사코드	
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드	
	char alcn_way_dvs_cd	[1+1];	///< 배차방식구분코드	
	char sati_use_yn		[1+1];	///< 좌석제사용여부 // 20221201 ADD	
	char tisu_sta_dvs_cd	[1+1];	///< 발권상태구분코드	
	char perd_temp_yn		[1+1];	///< 정기임시여부		
	char aprv_no			[100+1];///< 승인번호		
	char csrc_auth_no		[100+1];///< 현금영수증인증번호	
	char cprt_yn			[1+1];	///< 법인여부		
	char card_no			[100+1];///< 카드번호		
	char pub_user_id		[17+1];	///< 발행사용자id	
	char canc_ry_dvs_cd		[1+1];	///< 취소환불구분코드	
	char ryrt_dvs_cd		[1+1];	///< 환불율구분코드	
	char canc_ry_dt			[8+1];	///< 취소환불일자		
	char canc_ry_time		[6+1];	///< 취소환불시각		
	char canc_ry_amt		[18+1];	///< 취소환불금액		
	char canc_ry_trml_cd	[7+1];	///< 취소환불터미널코드	
	char canc_ry_wnd_no		[2+1];	///< 취소환불창구번호	
	char canc_ry_user_id	[17+1];	///< 취소환불사용자id	
	char sats_rest_dt		[8+1];	///< 좌석복원일자		
	char sats_rest_time		[6+1];	///< 좌석복원시각		
	char sats_rest_trml_cd	[7+1];	///< 좌석복원터미널코드	
	char sats_rest_wnd_no	[2+1];	///< 좌석복원창구번호	
	char sats_rest_user_id	[17+1];	///< 좌석복원사용자id	
	char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드
	char dcrt_dvs_cd		[1+1];	///< 할인율구분코드	
	char dc_bef_amt			[18+1];	///< 할인이전금액		
	char tisu_sys_dvs_cd	[1+1];	///< 발권시스템구분코드	
	char pub_sys_dvs_cd		[1+1];	///< 발행시스템구분코드	
	char eb_bus_tck_sno		[22+1];	///< eb버스티켓일련번호	
	char miss_err_yn		[1+1];	///< 분실오류여부		
	char rpub_num			[5+1];	///< 재발행매수		
	/* // 20221220 ADD~
	//char rpub_num			[5+1];	///< 재발행매수		
	char issu_crcm_cd			[4	+1];	///< 발급카드사코드
	char lnkg_py_knd_cd			[1	+1];	///< 연동페이종류코드
	char rpub_num				[5	+1];	///< 재발행매수
	char rpub_dt				[8	+1];	///< 재발행일자
	char rpub_time				[6	+1];	///< 재발행시각
	char rpub_user_id			[17	+1];	///< 재발행사용자ID
	char evcp_pym_amt			[18	+1];	///< 이벤트쿠폰결제금액
	char mlg_pym_amt			[18	+1];	///< 마일리지결제금액
	char whch_sats_no_prin_nm	[40	+1];	///< 휠체어좌석번호출력명
	char chtk_dt				[8	+1];	///< 검표일자
	char chtk_time				[6	+1];	///< 검표시각
	char chtk_user_id			[17	+1];	///< 검표 사용자 ID
	char chtk_rot_id			[17	+1];	///< 검표노선ID
	char chtk_depr_dt			[8	+1];	///< 검표출발일자
	char chtk_depr_time			[6	+1];	///< 검표출발시각
	char chtk_sats_no			[5	+1];	///< 검표좌석번호
	char chtk_bus_cacm_cd		[4	+1];	///< 검표버스운수사코드
	char prcd_alcn_ride_yn		[1	+1];	///< 선행배차승차여부
	char prcd_alcn_cty_rot_id	[17	+1];	///< 선행배차시외노선ID
	char prcd_alcn_rot_sqno		[5	+1];	///< 선행배차노선순번
	char prcd_alcn_alcn_dt		[8	+1];	///< 선행배차배차일자
	char prcd_alcn_alcn_sqno	[5	+1];	///< 선행배차배차순번
	char prcd_alcn_sats_no		[5	+1];	///< 선행배차좌석번호
	*/ // 20221220 ~ADD
	char brkp_type			[1+1];	///< 위약금 타입(BRKP_TYPE)					// 20221220 ADD
	char qr_pym_pyn_dtl_cd	[2+1];	///< QR결제지불상세코드(QR_PYM_PYN_DTL_CD)	// 20221220 ADD

	rtk_readbustckno_list_t* pList;

} rtk_readbustckno_t, *prtk_readbustckno_t;


//------------------------------
// IF_SV_164 : 창구마감

typedef struct
{
	stk_head_id_t hdr;
	//stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분	
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드		
	//char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 ip	

	char req_trml_user_id	[17+1];	///< 요청터미널 사용자 ID
	char prs_bus_tck_inhr_no[8+1];	///< 현재버스티켓고유번호	
	char add_bus_tck_inhr_no[8+1];	///< 추가버스티켓고유번호
} stk_closwnd_t, *pstk_closwnd_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char wnd_end_dt			[8+1];	///< 창구종료일자
	char wnd_end_time		[6+1];	///< 창구종료시각
} rtk_closwnd_t, *prtk_closwnd_t;

//------------------------------
// IF_SV_200 : 시외버스지역코드 조회

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분	
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드		
	//char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 ip	
} stk_readareacd_t, *pstk_readareacd_t;

typedef struct
{
	char cty_bus_area_cd	[2+1];	///< 시외버스지역코드
	char cty_bus_area_nm	[100+1];	///< 시외버스지역명
} rtk_readareacd_list_t, *prtk_readareacd_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char cty_bus_area_cd_num[5+1];	///< 시외버스지역코드 수(int)
	rtk_readareacd_list_t* pList;	///< 
} rtk_readareacd_t, *prtk_readareacd_t;

//------------------------------
// IF_SV_201 : 왕복가능터미널 조회

typedef struct
{
	stk_head_t	hdr;
} stk_readrtrptrml_t, *pstk_readrtrptrml_t;

typedef struct
{
	char arvl_trml_cd		[7+1];	///< 도착터미널코드
} rtk_readrtrptrml_list_t, *prtk_readrtrptrml_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char rtrp_psb_trml_num	[5+1];	///< 왕복가능터미널 수(int)
	rtk_readrtrptrml_list_t* pList;	///< 
} rtk_readrtrptrml_t, *prtk_readrtrptrml_t;

//------------------------------
// IF_SV_202 : 좌석 선점/해제

typedef struct
{
	char sats_pcpy_id		[17+1];	///< 좌석선점id			
	char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드	
	char dcrt_dvs_cd		[1+1];	///< 할인율구분코드		
	char rtrp_depr_dt		[8+1];	///< 왕복출발일자			
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드		
	char sats_no			[5+1];	///< 좌석번호			
} stk_pcpysats_list_t, *pstk_pcpysats_list_t;

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분	
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드		
	//char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 ip	
	//char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 id	

	char sats_pcpy_dvs		[1+1];	///< 선점 구분			
	char rot_id				[17+1];	///< 노선id			
	char rot_sqno			[5+1];	///< 노선순번			
	char alcn_dt			[8+1];	///< 배차일자			
	char alcn_sqno			[4+1];	///< 배차순번			
	char depr_trml_cd		[7+1];	///< 출발터미널코드		
	char arvl_trml_cd		[7+1];	///< 도착터미널코드		
	char sats_pcpy_num		[2+1];	///< 좌석 선점 수		
	stk_pcpysats_list_t List[30];
} stk_pcpysats_t, *pstk_pcpysats_t;

typedef struct
{
	char sats_pcpy_id		[17+1];	///< 좌석선점ID			
	char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드
	char dcrt_dvs_cd		[1+1];	///< 할인율구분코드		
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드	
	char dc_aft_amt			[4+1];	///< 할인이후금액		
	char sats_no			[4+1];	///< 좌석번호			
} rtk_pcpysats_list_t, *prtk_pcpysats_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char sats_pcpy_num		[2+1];	///< 좌석 선점 수
	rtk_pcpysats_list_t* pList;		///< 
} rtk_pcpysats_t, *prtk_pcpysats_t;

//------------------------------
// IF_SV_278 : QR 좌석 선점 변경

typedef struct
{
	char sats_pcpy_id		[17+1];	///< 좌석선점id			
	//char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드	
	//char dcrt_dvs_cd		[1+1];	///< 할인율구분코드		
	//char rtrp_depr_dt		[8+1];	///< 왕복출발일자			
	//char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드		
	//char sats_no			[5+1];	///< 좌석번호			
} stk_qrmdpcpysats_list_t, *pstk_qrmdpcpysats_list_t;

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분	
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드		
	//char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 ip	
	//char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 id	

	char sats_pcpy_num		[2+1];	///< 좌석 선점 수		
	stk_qrmdpcpysats_list_t List[30];
} stk_qrmdpcpysats_t, *pstk_qrmdpcpysats_t;

//typedef struct
//{
//	char sats_pcpy_id		[17+1];	///< 좌석선점ID			
//	char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드
//	char dcrt_dvs_cd		[1+1];	///< 할인율구분코드		
//	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드	
//	char dc_aft_amt			[4+1];	///< 할인이후금액		
//	char sats_no			[4+1];	///< 좌석번호			
//} rtk_qrmdpcpysats_list_t, *ptk_qrmdpcpysats_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	//char sats_pcpy_num		[2+1];	///< 좌석 선점 수
	//rtk_qrmdpcpysats_list_t* pList;		///< 
} rtk_qrmdpcpysats_t, *prtk_qrmdpcpysats_t;

//------------------------------
// IF_SV_208 : 시외버스메시지코드 조회

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
} stk_readcbusmsg_t, *pstk_readcbusmsg_t;

//>>

typedef struct
{
	char cty_bus_msg_cd		[30+1];	///< 시외버스메시지코드	
	char cty_bus_msg_nm		[100+1];	///< 시외버스메시지명	
	char lng_dvs_cd			[2+1];	///< 언어구분코드		
} rtk_readcbusmsg_list_t, *prtk_readcbusmsg_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char msg_num			[5+1];	///< 조회 데이터 건 수
	rtk_readcbusmsg_list_t* pList;	///< 
} rtk_readcbusmsg_t, *prtk_readcbusmsg_t;

//------------------------------
// IF_SV_209 : 버스티켓인쇄상세 조회

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드
	//char req_trml_wnd_no	[2+1];	///< 요청 창구번호
	//char req_trml_user_ip	[40+1];	///< 요청 장비 IP
	//char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 ID

	char cty_bus_ptr_knd_cd	[3+1];	///< 시외버스 프린터 종류 코드
} stk_readptrgtck_t, *pstk_readptrgtck_t;

//>>

typedef struct
{
	char bus_tck_typ_cd		[3+1];	///< 버스티켓유형코드		
	char cty_bus_ptr_knd_cd	[3+1];	///< 시외버스프린터종류코드	
	char cty_bus_ptrg_usg_cd[1+1];	///< 시외버스인쇄용도코드	
	char cty_bus_ptrg_atc_cd[2+1];	///< 시외버스인쇄항목코드	
	char ptrg_x_crdn_val	[100+1];///< 인쇄X좌표				
	char ptrg_y_crdn_val	[100+1];///< 인쇄Y좌표				
	char ptrg_mgnf_cd		[1+1];	///< 인쇄배율코드			
} rtk_readptrgtck_list_t, *prtk_readptrgtck_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char read_dta_num		[5+1];	///< 조회 데이터 건 수
	rtk_readptrgtck_list_t* pList;	///< 
} rtk_readptrgtck_t, *prtk_readptrgtck_t;

//------------------------------
// IF_SV_213 : 무인 관리자 로그인

typedef struct
{
	stk_head_t	hdr;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분		
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드		
	//char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 IP	

	char user_no			[10+1];	///< 사용자번호			
	char req_trml_user_pwd	[100+1];	///< 요청 터미널 사용자비밀번호
} stk_loginkosmgr_t, *pstk_loginkosmgr_t;

typedef struct
{
	char rsp_cd[6+1];					///< 응답코드
} rtk_loginkosmgr_t, *prtk_loginkosmgr_t;

//------------------------------
// IF_SV_217 : 발매제한상세

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분		
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드		
	//char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 IP	
	//char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 ID	
} stk_tisultndtl_t, *pstk_tisultndtl_t;

//>>

typedef struct
{
	char rot_id				[17+1];	///< 노선id			
	char depr_trml_cd		[7+1];	///< 출발터미널코드	
	char arvl_trml_cd		[7+1];	///< 도착터미널코드	
	char adpt_stt_dt		[8+1];	///< 적용시작일자	
	char adpt_stt_time		[6+1];	///< 적용시작시각	
	char adpt_end_dt		[8+1];	///< 적용종료일자	
	char adpt_end_time		[6+1];	///< 적용종료시각	
} rtk_tisultndtl_list_t, *prtk_tisultndtl_list_t;


typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char rptn_num			[5+1];	///< 반복횟수
	rtk_tisultndtl_list_t* pList;
} rtk_tisultndtl_t, *prtk_tisultndtl_t;

//------------------------------
// IF_SV_268 : 도착터미널 키워드 매핑 조회

typedef struct
{
	stk_head_id_t	hdr_id;
} stk_readtrmlkwd_t, *pstk_readtrmlkwd_t;

typedef struct
{
	char arvl_trml_cd		[7+1];	///< 도착터미널 코드
	char shct_arvl_trml_cd	[3+1];	///< 단축 도착터미널 코드
	char kwd_sqno			[5+1];	///< (int)키워드 순번
	char kwd_nm				[100+1];///< 키워드 이름

} rtk_readtrmlkwd_list_t, *prtk_readtrmlkwd_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char trml_cd_num		[5+1];	///< 터미널코드 수
	rtk_readtrmlkwd_list_t* pList;
} rtk_readtrmlkwd_t, *prtk_readtrmlkwd_t;

//------------------------------
// IF_SV_221 : 승차권 발행 - 카드현장(KTC인증)

typedef struct
{
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드		
	char sats_pcpy_id		[17+1];	///< 좌석선점ID			
	char sats_no			[5+1];	///< 좌석번호			
	char mrnp_num			[5+1];	///< 예약매수			
	char mrnp_id			[17+1];	///< 예약ID			
} stk_ktcpbtckcard_list_t, *pstk_ktcpbtckcard_list_t;

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분	
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드		
	//char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 IP	
	//char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 ID

	char read_rot_yn		[1+1];	///< 예매자료조회여부		
	char rot_id				[17+1];	///< 노선ID			
	char rot_sqno			[5+1];	///< 노선순번			
	char alcn_dt			[8+1];	///< 배차일자			
	char alcn_sqno			[4+1];	///< *배차순번			
	char depr_trml_cd		[7+1];	///< 출발터미널코드		
	char arvl_trml_cd		[7+1];	///< 도착터미널코드		
	char pyn_mns_dvs_cd		[1+1];	///< 지불수단구분코드		
	char pub_chnl_dvs_cd	[1+1];	///< 발행채널구분코드		
	char tisu_way_dvs_cd	[1+1];	///< 발권방식구분코드		
	// 	char damo_enc_card_no	[1024+1];	///< 카드번호암호문		
	//     char damo_enc_card_no_len[4+1];	///< *카드번호암호문길이		
	// 	char damo_enc_dta_key	[32+1];	///< 암호문키			
	char trd_dvs_cd			[1+1];	///< 거래구분코드			
	char fallback_dvs_cd	[1+1];	///< FallBack구분코드	
	char union_pay_yn		[1+1];	///< 은련카드여부			
	char pos_pg_ktc_ver		[20+1];	///< POS단말기버전정보		
	char enc_dta_len		[4+1];	///< *ENC 데이터 길이		
	char enc_dta			[1024+1];	///< ENC 데이터			
	char emv_dta			[1024+1];	///< EMV 데이터			
	char mip_term			[2+1];	///< *할부기간	
	char spad_dta			[3072+1];	///< 싸인패드데이터		
	char spad_dta_len		[8+1];		///< 싸인패드데이터길이	
	char user_dvs_no		[20+1];		///< 사용자구분번호		
	char old_mbrs_no		[16+1];		///< 구회원번호
	char rf_trcr_dvs_cd		[1+1];		///< RF 교통카드 구분코드
	char pub_num			[5+1];		///< *발행매수			

	stk_ktcpbtckcard_list_t	List[50];
} stk_ktcpbtckcard_t, *pstk_ktcpbtckcard_t;

//>>

typedef struct
{
	char pyn_mns_dvs_cd		[1+1];	///< 지불수단구분코드
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드
	char pub_sno			[4+1];	///< 발행일련번호		
	char bus_tck_inhr_no	[8+1];	///< 버스티켓고유번호	
	char tisu_amt			[18+1];	///< 발권금액			
	char sats_no			[5+1];	///< 좌석번호			
	char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드
	char dcrt_dvs_cd		[1+1];	///< 할인율구분코드		
	char dc_bef_amt			[18+1];	///< 할인이전금액		
	char cty_bus_oprn_shp_cd[1+1];	///< 시외버스운행형태코드
} rtk_ktcpbtckcard_list_t, *prtk_ktcpbtckcard_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char rsp_msg			[100+1];	///< 응답 메시지
	char card_no			[100+1];	///< 카드번호
	char card_aprv_no		[100+1];	///< 카드승인번호
	char card_aprv_amt		[18+1];	///< 카드승인금액
	char aprv_amt			[18+1];	///< 승인금액
	char card_aprv_vanc_cd	[2+1];	///< 카드승인VAN사코드
	char issu_crcm_cd		[4+1];	///< 발급카드사코드
	char buy_crcm_cd		[4+1];	///< 매입카드사코드
	char frc_cmrt			[5+1];	///< 가맹점수수료율
	char atl_depr_dt		[8+1];	///< 실제출발일자
	char atl_depr_time		[6+1];	///< 실제출발시각
	char depr_trml_cd		[7+1];	///< 출발터미널코드
	char arvl_trml_cd		[7+1];	///< 도착터미널코드
	char stpt_trml_cd		[7+1];	///< 기점터미널코드
	char ept_trml_cd		[7+1];	///< 종점터미널코드
	char bus_cls_cd			[3+1];	///< 버스등급코드
	char bus_cacm_cd		[4+1];	///< 버스운수사코드
	char rdhm_mltp_val		[100+1];///< 승차홈다중값
	char dist				[18+1];	///< 거리
	char take_drtm			[18+1];	///< 소요시간
	char bus_cls_prin_yn	[1+1];	///< 버스등급출력여부
	char bus_cacm_nm_prin_yn[1+1];	///< 버스운수사명출력여부
	char depr_time_prin_yn	[1+1];	///< 출발시각출력여부
	char sats_no_prin_yn	[1+1];	///< 좌석번호출력여부
	char alcn_way_dvs_cd	[1+1];	///< 배차방식구분코드
	char sati_use_yn		[1+1];	///< 좌석제사용여부
	char rot_knd_cd			[1+1];	///< 노선종류코드
	char perd_temp_yn		[1+1];	///< 정기임시여부
	char pub_dt				[8+1];	///< 발행일자
	char pub_time			[6+1];	///< 발행시각
	char pub_shct_trml_cd	[4+1];	///< 발행단축터미널코드
	char pub_wnd_no			[2+1];	///< 발행창구번호
	char card_tr_add_inf	[100+1];	///< 카드거래추가정보
	char pub_num			[5+1];	///< 발행매수 

	rtk_ktcpbtckcard_list_t* pList;	///< 
} rtk_ktcpbtckcard_t, *prtk_ktcpbtckcard_t;

// 20221017 ADD~
//------------------------------
// IF_SV_274 : 승차권 발행 요청 - QR결제 전용(KIOSK)				

typedef struct
{
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드
	char tisu_amt			[18+1];	///< 발권금액			
	char sats_pcpy_id		[17+1];	///< 좌석선점id			
	char sats_no			[5+1];	///< 좌석번호			
} stk_pbtckqr_list_t, *pstk_pbtckqr_list_t;

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분	
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드		
	//char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 IP	
	//char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 ID

	char rot_id				[17+1];	///< 노선ID			
	char rot_sqno			[5+1];	///< 노선순번			
	char alcn_dt			[8+1];	///< 배차일자			
	char alcn_sqno			[4+1];	///< *배차순번			
	char depr_trml_cd		[7+1];	///< 출발터미널코드		
	char arvl_trml_cd		[7+1];	///< 도착터미널코드		
    char qr_cd_no			[100+1];    // QR코드번호
    char qr_pym_pyn_dtl_cd	[2+1];		// QR결제지불상세코드 // 20221228 ADD
    char payco_virt_ontc_no	[21+1];		// OTC번호;페이코가상일회성카드번호
	char spad_dta			[3072+1];	///< 싸인패드데이터		
	char spad_dta_len		[8+1];		///< 싸인패드데이터길이	
	char mip_term			[2+1];		///< *할부기간	
	char pub_num			[5+1];		///< *발행매수			

	stk_pbtckqr_list_t	List[50];
} stk_pbtckqr_t, *pstk_pbtckqr_t;

//>>

typedef struct
{
	char pyn_mns_dvs_cd		[1+1];	///< 지불수단구분코드
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드
	char pub_sno			[4+1];	///< 발행일련번호		
	char bus_tck_inhr_no	[8+1];	///< 버스티켓고유번호	
	char tisu_amt			[18+1];	///< 발권금액			
	char sats_no			[5+1];	///< 좌석번호			
	char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드
	char dcrt_dvs_cd		[1+1];	///< 할인율구분코드		
	char dc_bef_amt			[18+1];	///< 할인이전금액		
} rtk_pbtckqr_list_t, *prtk_pbtckqr_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char rsp_msg			[100+1];	///< 응답 메시지
	char card_no			[100+1];	///< 카드번호
	char card_aprv_no		[100+1];	///< 카드승인번호
	char card_aprv_amt		[18+1];	///< 카드승인금액
	char card_aprv_vanc_cd	[2+1];	///< 카드승인VAN사코드
	char issu_crcm_cd		[4+1];	///< 발급카드사코드
	char buy_crcm_cd		[4+1];	///< 매입카드사코드
	char frc_cmrt			[5+1];	///< 가맹점수수료율
	char atl_depr_dt		[8+1];	///< 실제출발일자
	char atl_depr_time		[6+1];	///< 실제출발시각
	char depr_trml_cd		[7+1];	///< 출발터미널코드
	char arvl_trml_cd		[7+1];	///< 도착터미널코드
	char stpt_trml_cd		[7+1];	///< 기점터미널코드
	char ept_trml_cd		[7+1];	///< 종점터미널코드
	char bus_cls_cd			[3+1];	///< 버스등급코드
	char bus_cacm_cd		[4+1];	///< 버스운수사코드
	char rdhm_mltp_val		[100+1];///< 승차홈다중값
	char dist				[18+1];	///< 거리
	char take_drtm			[18+1];	///< 소요시간
	char bus_cls_prin_yn	[1+1];	///< 버스등급출력여부
	char bus_cacm_nm_prin_yn[1+1];	///< 버스운수사명출력여부
	char depr_time_prin_yn	[1+1];	///< 출발시각출력여부
	char sats_no_prin_yn	[1+1];	///< 좌석번호출력여부
	char alcn_way_dvs_cd	[1+1];	///< 배차방식구분코드
	char perd_temp_yn		[1+1];	///< 정기임시여부
	char pub_dt				[8+1];	///< 발행일자
	char pub_time			[6+1];	///< 발행시각
	char pub_shct_trml_cd	[4+1];	///< 발행단축터미널코드
	char pub_wnd_no			[2+1];	///< 발행창구번호
	char card_tr_add_inf	[100+1];	///< 카드거래추가정보
	char pub_num			[5+1];	///< 발행매수 

	rtk_pbtckqr_list_t*		pList;	///< 
} rtk_pbtckqr_t, *prtk_pbtckqr_t;
// 20221017 ~ADD

//------------------------------
// IF_SV_222 : 승차권 발행 - 현금영수증(KTC인증)

typedef struct
{
	char bus_tck_knd_cd				[4+1];	///< 버스티켓종류코드			
	char sats_pcpy_id				[17+1];	///< 좌석선점id				
	char sats_no					[5+1];	///< 좌석번호				
	char mrnp_num					[5+1];	///< 예약매수				
	char mrnp_id					[17+1];	///< 예약id				
} stk_ktcpbtckcsrc_list_t, *pstk_ktcpbtckcsrc_list_t;

typedef struct
{
	stk_head_id_t	hdr_id;

	char rot_id						[17+1];	///< 노선id				
	char rot_sqno					[5+1];	///< 노선순번				
	char alcn_dt					[8+1];	///< 배차일자				
	char alcn_sqno					[4+1];	///< 배차순번				
	char depr_trml_cd				[7+1];	///< 출발터미널코드			
	char arvl_trml_cd				[7+1];	///< 도착터미널코드			
	char tisu_way_dvs_cd			[1+1];	///< 발권방식구분코드			
	char pyn_mns_dvs_cd				[1+1];	///< 지불수단구분코드			
	char pub_chnl_dvs_cd			[1+1];	///< 발행채널구분코드			
	char pos_pg_ktc_ver				[20+1];	///< pos단말기버전정보			
	char enc_dta_len				[4+1];	///< enc 데이터 길이			
	char enc_dta					[1024+1];	///< enc 데이터				
	char cprt_yn					[1+1];	///< 법인여부				
	char user_dvs_no				[20+1];	///< 사용자구분번호			
	char old_mbrs_no				[16+1];	///< 구회원번호				
	
	char pub_num					[5+1];	///< 발행매수				
	stk_ktcpbtckcsrc_list_t		List[30];
} stk_ktcpbtckcsrc_t, *pstk_ktcpbtckcsrc_t;

//>>

typedef struct  
{
	char pyn_mns_dvs_cd		[1+1];	///< 지불수단구분코드		
	char csrc_aprv_no		[100+1];///< 현금영수증승인번호		
	char aprv_no			[100+1];///< 승인번호			
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드		
	char pub_sno			[4+1];	///< 발행일련번호			
	char bus_tck_inhr_no	[8+1];	///< 버스티켓고유번호		
	char tisu_amt			[18+1];	///< 발권금액			
	char sats_no			[5+1];	///< 좌석번호			
	char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드	
	char dcrt_dvs_cd		[1+1];	///< 할인율구분코드		
	char dc_bef_amt			[18+1];	///< 할인이전금액			
	char cty_bus_oprn_shp_cd[1+1];	///< 시외버스운행형태코드	
} rtk_ktcpbtckcsrc_list_t, *prtk_ktcpbtckcsrc_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드			
	char rsp_msg			[100+1];///< 응답 메시지			
	char csrc_auth_no		[100+1];///< 현금영수증인증번호		
	char csrc_brn			[12+1];	///< 현금영수증사업자등록번호	
	char csrc_aprv_vanc_cd	[2+1];	///< 현금영수증승인van사코드	
	char atl_depr_dt		[8+1];	///< 실제출발일자			
	char atl_depr_time		[6+1];	///< 실제출발시각			
	char depr_trml_cd		[7+1];	///< 출발터미널코드		
	char arvl_trml_cd		[7+1];	///< 도착터미널코드		
	char stpt_trml_cd		[7+1];	///< 기점터미널코드		
	char ept_trml_cd		[7+1];	///< 종점터미널코드		
	char bus_cls_cd			[3+1];	///< 버스등급코드			
	char bus_cacm_cd		[4+1];	///< 버스운수사코드		
	char rdhm_mltp_val		[100+1];///< 승차홈다중값			
	char dist				[18+1];	///< 거리				
	char take_drtm			[18+1];	///< 소요시간			
	char bus_cls_prin_yn	[1+1];	///< 버스등급출력여부		
	char bus_cacm_nm_prin_yn[1+1];	///< 버스운수사명출력여부	
	char depr_time_prin_yn	[1+1];	///< 출발시각출력여부		
	char sats_no_prin_yn	[1+1];	///< 좌석번호출력여부		
	char alcn_way_dvs_cd	[1+1];	///< 배차방식구분코드		
	char sati_use_yn		[1+1];	///< 좌석제사용여부		
	char rot_knd_cd			[1+1];	///< 노선종류코드			
	char perd_temp_yn		[1+1];	///< 정기임시여부			
	char pub_dt				[8+1];	///< 발행일자			
	char pub_time			[6+1];	///< 발행시각			
	char pub_shct_trml_cd	[4+1];	///< 발행단축터미널코드		
	char pub_wnd_no			[2+1];	///< 발행창구번호			
	
	char pub_num			[5+1];	///< 발행매수	
	rtk_ktcpbtckcsrc_list_t* pList;

} rtk_ktcpbtckcsrc_t, *prtk_ktcpbtckcsrc_t;

//------------------------------
// IF_SV_225 : 상주직원조회

typedef struct
{
	stk_head_id_t	hdr_id;

	char read_wnd_dvs		[1+1];	///< 조회 창구 구분			
	char user_dvs_no		[20+1];	///< 사용자구분번호			
	char user_pwd			[100+1];///< 사용자비밀번호			
	char tisu_req_yn		[1+1];	///< 발권요청여부			
	char rot_id				[17+1];	///< 노선id					
	char rot_sqno			[4+1];	///< 노선순번				
	char alcn_dt			[8+1];	///< 배차일자				
	char alcn_sqno			[4+1];	///< 배차순번				
} stk_readrsd_t, *pstk_readrsd_t;

typedef struct
{
	char rsp_cd				[6+1];		///< 응답코드		
	char rsd_user_id		[17+1];		///< 상주사용자id	
	char user_nm			[100+1];	///< 사용자명		
	char cmpy_nm			[100+1];	///< 회사명			
	char ntfc_ctt			[2000+1];	///< 알림내용		
	char tissu_unbl_rot_val	[2000+1];	///< 발권불가노선값	
	char sell_yn			[1+1];		///< 판매여부		
	char dlt_yn				[1+1];		///< 삭제여부		
	char pub_dt				[8+1];		///< 발행일자		
	char pub_time			[6+1];		///< 발행시각		
	char pub_user_id		[17+1];		///< 발행사용자id	
	char pub_user_nm		[100+1];	///< 발행사용자명	
	char user_dvs_no		[20+1];		///< 사용자구분번호	
	char pwd_rgt_yn			[1+1];		///< 비밀번호등록여부
} rtk_readrsd_t, *prtk_readrsd_t;



//------------------------------
// IF_SV_228 : 좌석 요금 정보 변경

typedef struct
{
	char sats_pcpy_id		[17+1];	///< 좌석선점id						
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드				
	char sats_no			[5+1];	///< 좌석번호						
} stk_modpcpysats_list_t, *pstk_modpcpysats_list_t;

typedef struct
{
	stk_head_id_t	hdr_id;

	char rot_id				[17+1];		///< 노선id							
	char rot_sqno			[5+1];		///< 노선순번						
	char alcn_dt			[8+1];		///< 배차일자						
	char alcn_sqno			[4+1];		///< 배차순번						
	char depr_trml_cd		[7+1];		///< 출발터미널코드					
	char arvl_trml_cd		[7+1];		///< 도착터미널코드					
	char damo_enc_card_no	[1024+1];	///< 카드번호암호문					
	char damo_enc_card_no_len[4+1];		///< 카드번호암호문길이				
	char damo_enc_dta_key	[32+1];		///< 암호문키						

	char pub_num			[4+1];		///< 발행매수						
	stk_modpcpysats_list_t  List[30];
} stk_modpcpysats_t, *pstk_modpcpysats_t;

//>>

typedef struct
{
	char sats_pcpy_id		[17+1];	///< 좌석선점id						
	char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드			
	char dcrt_dvs_cd		[1+1];	///< 할인율구분코드					
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드				
	char dc_aft_amt			[4+1];	///< 할인이후금액					
	char sats_no			[4+1];	///< 좌석번호						
} rtk_modpcpysats_list_t, *prtk_modpcpysats_list_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char sats_pcpy_num		[2+1];	///< 좌석 선점 수
	rtk_modpcpysats_list_t* pList;
} rtk_modpcpysats_t, *prtk_modpcpysats_t;

//------------------------------
// IF_SV_230 : Tmax 초기화 서비스

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분		
	//char req_trml_cd		[7+1];	///< 요청 터미널 코드		
	//char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 IP	
	//char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 ID

	char enc_dta			[1024+1];	///< ENC DATA
} stk_ktctmxclr_t, *pstk_ktctmxclr_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
} rtk_ktctmxclr_t, *prtk_ktctmxclr_t;

//------------------------------
// IF_SV_260 : RF_선불카드 발행 요청

typedef struct
{
	stk_head_id_t	hdr_id;
	
	char rot_id					[17+1];		///< 노선id			
	char rot_sqno				[4+1];		///< (int)노선순번			
	char alcn_dt				[8+1];		///< 배차일자			
	char alcn_sqno				[4+1];		///< (int)배차순번			
	char depr_trml_cd			[7+1];		///< 출발터미널코드		
	char arvl_trml_cd			[7+1];		///< 도착터미널코드		
	char tisu_way_dvs_cd		[1+1];		///< 발권방식구분코드		
	char read_rot_yn			[1+1];		///< 예매자료조회여부		
	char pub_num				[4+1];		///< (int)발행매수			
	char pyn_mns_dvs_cd			[1+1];		///< 지불수단구분코드		
	char pub_chnl_dvs_cd		[1+1];		///< 발행채널구분코드		
	char user_dvs_no			[20+1];		///< 사용자구분번호		
	char bus_tck_knd_cd			[4+1];		///< 버스티켓종류코드		
	char sats_no				[4+1];		///< (int)좌석번호			
	char sats_pcpy_id			[17+1];		///< 좌석선점id			
	char old_mbrs_no			[16+1];		///< 구회원번호			
	char mrnp_id				[17+1];		///< 예약id			
	char mrnp_num				[4+1];		///< (int)예약매수			
	char damo_enc_card_no		[1024+1];	///< 카드번호암호문		
	char damo_enc_card_no_len	[4+1];		///< (int)카드번호암호문길이		
	char damo_enc_dta_key		[32+1];		///< 암호문키		
	char ppy_tak_dvs_cd			[3+1];		///< 선불작업구분코드		
	char ppy_pos_sls_dt			[8+1];		///< 선불pos영업일자		
	char ppy_pos_recp_no		[8+1];		///< 선불pos영수증번호		
	char ppy_sam_id				[16+1];		///< 선불samid			
	char ppy_sam_trd_sno		[10+1];		///< 선불sam거래일련번호	
	char ppy_card_trd_sno		[10+1];		///< 선불카드거래일련번호	
	char ppy_aprv_aft_bal		[4+1];		///< (int)선불승인이후잔액		
	char ppy_aprv_bef_bal		[4+1];		///< (int)선불승인이전잔액		
	char ppy_algr_id			[2+1];		///< 선불알고리즘id		
	char ppy_sign_val			[100+1];	///< 선불sign값			
	char ppy_indv_trd_clk_ver	[2+1];		///< 선불개별거래수집키버전	
	char ppy_elcs_idnt_id		[2+1];		///< 선불전자화폐식별자id	
	char sam_ttam_clcn_sno		[10+1];		///< sam총액수집일련번호	
	char sam_indv_clcn_ncnt		[4+1];		///< (int)sam개별수집건수		
	char sam_cum_trd_ttam		[4+1];		///< (int)sam누적거래총액		
	char ppy_card_dvs_cd		[2+1];		///< 선불카드구분코드		
	char ppy_card_user_dvs_cd	[2+1];		///< 선불카드사용자구분코드	
	char ppy_hsm_sta_cd			[1+1];		///< 선불hsm상태코드		
	char req_trd_amt			[4+1];		///< (int)거래요청금액			

} stk_pubtckppy_t, *pstk_pubtckppy_t;

typedef struct
{
	char rsp_cd					[6+1];		///< 응답코드
	char rsp_msg				[100+1];	///< 응답메시지

	char pub_dt					[8+1];		///< 발행일자		
	char pub_time				[6+1];		///< 발행시각		
	char depr_trml_cd			[7+1];		///< 출발터미널코드	
	char arvl_trml_cd			[7+1];		///< 도착터미널코드	
	char bus_cls_cd				[3+1];		///< 버스등급코드		
	char bus_cacm_cd			[4+1];		///< 버스운수사코드	
	char pub_wnd_no				[2+1];		///< 발행창구번호		
	char pub_sno				[4+1];		///< (int)발행일련번호		
	char pub_shct_trml_cd		[4+1];		///< 발행단축터미널코드	
	char bus_tck_knd_cd			[4+1];		///< 버스티켓종류코드	
	char tisu_amt				[4+1];		///< (int)발권금액		
	char pyn_mns_dvs_cd			[1+1];		///< 지불수단구분코드	
	char sats_no				[4+1];		///< (int)좌석번호		
	char rot_knd_cd				[1+1];		///< 노선종류코드		
	char cty_bus_dc_knd_cd		[1+1];		///< 시외버스할인종류코드
	char dc_bef_amt				[4+1];		///< (int)할인이전금액		
	char bus_tck_inhr_no		[8+1];		///< 버스티켓고유번호	
	char cty_bus_oprn_shp_cd	[1+1];		///< 시외버스운행형태코드
	char atl_depr_dt			[8+1];		///< 실제출발일자		
	char atl_depr_time			[6+1];		///< 실제출발시각		
	char stpt_trml_cd			[7+1];		///< 기점터미널코드	
	char ept_trml_cd			[7+1];		///< 종점터미널코드	
	char alcn_way_dvs_cd		[1+1];		///< 배차방식구분코드	
	char sati_use_yn			[1+1];		///< 좌석제사용여부	
	char perd_temp_yn			[1+1];		///< 정기임시여부		
	char rdhm_mltp_val			[100+1];	///< 승차홈다중값		
	char bus_cls_prin_yn		[1+1];		///< 버스등급출력여부	
	char bus_cacm_nm_prin_yn	[1+1];		///< 버스운수사명출력여부
	char depr_time_prin_yn		[1+1];		///< 출발시각출력여부	
	char sats_no_prin_yn		[1+1];		///< 좌석번호출력여부	
	char dist					[4+1];		///< (int)거리			
	char take_drtm				[4+1];		///< (int)소요시간		
	char pub_num				[4+1];		///< (int)발행매수		
	char dcrt_dvs_cd			[1+1];		///< 할인율구분코드	

} rtk_pubtckppy_t, *prtk_pubtckppy_t;

/// 2020.04.03 인천공항 코로나로 인해, 경기도에서 요청한 사항.
//------------------------------
// IF_SV_269 : 인천공항 발행자 정보 입력

typedef struct
{
	char pub_dt						  [8+1];	///< 발행일자
	char pub_shct_trml_cd			  [4+1];	///< 발행단축터미널코드
	char pub_wnd_no					  [2+1];	///< 발행창구번호
	char pub_sno					  [4+1];	///< 발행일련번호
} stk_pubusrinfinp_list_t, *pstk_pubusrinfinp_list_t;

typedef struct
{
	stk_head_id_t	hdr_id;

	char damo_enc_pub_user_tel_no	  [1024+1];	///< 발행자 전화번호 암호문 
	char damo_enc_pub_user_tel_no_len [4+1];	///< 발행자 전화번호 암호문 길이
	char damo_enc_pub_user_krn		  [1024+1];	///< 발행자 주민번호 암호문
	char damo_enc_pub_user_krn_len	  [4+1];	///< 발행자 주민번호 암호문 길이
	char damo_enc_dta_key			  [32+1];	///< 암호문키
	char ride_vhcl_dvs				  [2+1];	///< 승차 차량구분(MV : 자가용, SV : 지원차량(시,군))

	char pub_num					  [4+1];	///< 발행매수
	stk_pubusrinfinp_list_t		List[100];
} stk_pubusrinfinp_t, *pstk_pubusrinfinp_t;

typedef struct
{
	char rsp_cd					[6+1];		///< 응답코드
	//char rsp_msg				[100+1];	///< 응답코드
} rtk_pubusrinfinp_t, *prtk_pubusrinfinp_t;

//------------------------------
// IF_SV_501 : 암호화키 전송

typedef struct
{
	stk_head_id_t  hdr_id;
// 	char req_pgm_dvs		[3+1];	///< 요청 프로그램 구분	
// 	char req_trml_cd		[7+1];	///< 요청 터미널 코드		
// 	char req_trml_user_ip	[40+1];	///< 요청 터미널 사용자 ip	
// 	char req_trml_wnd_no	[2+1];	///< 요청 터미널 창구 번호	
// 	char req_trml_user_id	[17+1];	///< 요청 터미널 사용자 ID	
} ssv_trmenckey_t, *pssv_trmenckey_t;

typedef struct
{
	char rsp_cd				[6+1];	///< 응답코드
	char damo_enc_dta_key	[32+1];	///< 암호문키
	char damo_enc_dta_key_len[2+10];///< 암호문키길이 (길이가 2바이트가 아님.....)
} rsv_trmenckey_t, *prsv_trmenckey_t;

//------------------------------
// IF_SV_502 : 복호화 서비스

typedef struct
{
	stk_head_id_t	hdr_id;
	//char req_pgm_dvs			[3+1];	///< 요청 프로그램 구분	
	//char req_trml_cd			[7+1];	///< 요청 터미널 코드		
	//char req_trml_wnd_no		[2+1];	///< 요청 터미널 창구 번호	
	//char req_trml_user_ip		[40+1];	///< 요청 터미널 사용자 ip	
	//char req_trml_user_id		[17+1];	///< 요청 터미널 사용자 ID	

	char damo_enc_dta			[1024+1];///< 암호화데이타
	char damo_enc_dta_key		[32+1];	///< 암호문키
	char damo_enc_dta_key_len	[4+1];	///< 암호문키길이
} ssv_trmdecrdta_t, *pssv_trmdecrdta_t;

typedef struct
{
	char rsp_cd					[6+1];	///< 응답코드
	char decr_dta				[1024+1];	///< 복호화데이타
	char decr_dta_len			[4+1];	///< 복호화데이타길이
} rsv_trmdecrdta_t, *prsv_trmdecrdta_t;



#pragma pack()

