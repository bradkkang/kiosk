// 
// 
// dev_tr_mem.h : transact memory 헤더 파일
//

#pragma once

#include <iostream>
#include <vector>

#include "dev_ui_main.h"
#include "svr_ccbus_st.h"

//----------------------------------------------------------------------------------------------------------------------

#define MAX_LIST	100

using namespace std;

//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)



typedef struct
{
	char ui_csrc_use			[1+1];	  /// 개인/법인 구분코드
	char user_nm				[100+1];  /// 사용자이름
	char rsd_nm					[100+1];  /// 상주직원 이름
	char rsd_cmpy_nm			[100+1];  /// 상주직원 소속명
	char rsd_tel_nm				[100+1];  /// 상주직원 전화번호 (사용자구분코드)

	char depr_trml_ko_nm		[100+1]	; /// 출발지 터미널명 (한글)
	char depr_trml_eng_nm		[100+1]	; /// 출발지 터미널명 (영문)
	char depr_trml_tel_nm		[100+1]	; /// 출발지 터미널 전화번호
	char depr_trml_biz_no		[100+1];  /// #출발지 터미널 - 사업자번호

	char arvl_trml_ko_nm		[100+1]	; /// 도착지 터미널명 (한글)
	char arvl_trml_eng_nm		[100+1]	; /// 도착지 터미널명 (영문)
	char arvl_trml_rot_num		[100+1]	; /// 도착지 노선번호

	char alcn_depr_trml_no		[3+1]	; /// 배차출발터미널번호
	char alcn_depr_trml_ko_nm	[100+1]	; /// 배차출발터미널 명 (한글)
	char alcn_arvl_trml_no		[3+1]	; /// 배차도착터미널번호
	char alcn_arvl_trml_ko_nm	[100+1]	; /// 배차도착터미널 명 (한글)
	char alcn_depr_dt			[100+1]	; /// 배차출발일자
	char alcn_depr_time			[100+1]	; /// 배차출발시각

	char bus_cls_nm				[100+1]	; /// 버스등급명
	char bus_cls_shct_nm		[100+1]	; /// 짧은 버스등급명!@#!@#
	char bus_tck_knd_nm			[100+1]	; /// 버스티켓종류명
	char bus_tck_knd_cd			[4+1]	; /// 버스티켓종류 코드

	char csrc_aprv_no			[100+1]	; /// 현금영수증 승인번호
	char card_aprv_no			[100+1]	; /// 신용카드 승인번호

	char csrc_aprv_amt			[20+1]	; /// 현금영수증 승인금액
	char card_aprv_amt			[20+1]	; /// 신용카드 승인금액
	char rf_balance_amt			[20+1]	; /// RF선불카드 잔액

	char card_no				[100+1]	; /// 카드번호

	char trd_dvs_cd				[1+1]	; ///< 거래구분코드			
	int	 n_remain_card_fare				; /// 선불카드 잔액 (X)
	char card_tr_add_inf		[100+1]	; /// 선불카드 잔액 (O)

	int  n_tisu_amt						; ///< 요금
	char tisu_amt				[20+1]	; ///< 요금

	// 20220324 ADD~
	char issu_crcm_cd			[4+1]	; ///< 발급카드사코드
	char frc_cmrt				[5+1]	; ///< 가맹점수수료율
	// 20220324 ~ADD
	// 20211015 ADD
	char evcp_pym_amt			[18+1];	///< 이벤트쿠폰결제금액				
	char mlg_pym_amt			[18+1];	///< 마일리지결제금액				
	// 20211015 ~ADD
	// 20211019 ADD
	char cpn_rmn_amt			[18+1];	///< 이벤트쿠폰결제금액(코버스)			
	char mlg_rmn_amt			[18+1];	///< 마일리지결제금액(코버스)				
	// 20211019 ~ADD

	char dcrt_dvs_str			[20+1]	; ///< 할인율 문자열
	char cty_bus_dc_knd_str		[20+1]	; ///< 시외버스할인종류 문자열

	int  n_pym_dvs;
	char pym_dvs				[20+1];	 /// 결제수단 (현금/신용카드 등)
	char shrt_pym_dvs			[20+1];	 /// 단축결제수단 (현금/신용카드 등)

	int  n_mip_mm_num				  ;	 /// 할부기간
	char card_no_mask			[40];	 /// #(추가)마스킹 카드번호

	char atl_depr_dt			[20+1];	 /// 실제 출발 일자
	char atl_depr_time			[20+1];	 /// 실제 출발 시간
	char sats_no				[20+1];	 /// 좌석번호
	char rdhm_val				[100+1]; /// 승차홈
	char bus_cacm_nm			[100+1]; /// 버스회사
	char bus_cacm_tel_nm		[100+1]; /// 버스회사 전화번호
	char bus_cacm_full_nm		[100+1]; /// #Full 버스회사
	char bus_cacm_biz_no		[100+1]; /// #Full 버스회사 사업자번호

	char bus_cls_prin_yn		[1];	 /// 버스등급 출력 여부
	char bus_cacm_nm_prin_yn	[1];	 /// 운수사명 출력 여부
	char depr_time_prin_yn		[1];	 /// 출발시각 출력 여부

	char pub_time				[20+1];	 /// 발행시각(시:분:초)
	char shrt_pub_time			[20+1];	 /// 발행시각(시:분)
	char dist					[5+1];	 /// #거리

	/// 바코드 관련
	char bar_pub_dt				[20+1];	 /// 발행일자
	char bar_pub_shct_trml_cd	[20+1];	 /// 발행단축터미널코드
	char bar_pub_wnd_no			[20+1];	 /// 발행창구번호
	char bar_pub_sno			[20+1];	 /// 발행일련번호
	char bar_secu_code			[20+1];	 /// 체크비트

	/// QR 코드 관련
	char qr_pub_dt				[20+1];	 /// #발행일자
	char qr_pub_trml_cd			[20+1];	 /// #발행단축터미널코드
	char qr_pub_wnd_no			[20+1];	 /// #발행창구번호
	char qr_pub_sno				[20+1];	 /// #발행일련번호
	char qr_depr_trml_no		[20+1];	 /// #출발터미널코드
	char qr_arvl_trml_no		[20+1];	 /// #도착터미널코드
	char qr_sats_no				[20+1];	 /// #좌석번호

	char alcn_way_dvs_cd		[1+1];	///< 배차방식구분코드
	char sati_use_yn			[1+1];	///< 좌석제 사용유무
	char sats_no_prin_yn		[1+1];	/// 좌석번호 출력 여부

	char thru_nm				[100];	///< 경유지
	char tisu_chnl_dvs_cd		[1+1];	///< 발권채널구분코드

} TCK_PRINT_FMT_T, *PTCK_PRINT_FMT_T;

typedef struct 
{
	int	 nTotalMoney			;		///< 총 결제금액
	int  n_pbtck_chg_money		;		///< 방출금액

	ABILL_T insBill				;		///< 투입금액
	ABILL_T outBill				;		///< (지폐) 방출금액
	ACOIN_T outCoin				;		///< (동전) 방출금액

	char ui_pym_dvs_cd		[1]	;		///< 결제 구분코드 - 0x01:현금결제, 0x02:카드결제, 0x04:RF결제, 0x21:페이코QR-카드결제, 0x22:페이코QR-포인트결제, 0x31:티머니페이QR-결제	// 20221124 MOD
	char ui_csrc_dvs_cd		[1]	;		///< 현금영수증 구분코드 - 0:미사용, 1:수기입력, 2:신용카드
	char ui_csrc_no			[20+1];		///< 현금영수증번호
	char ui_csrc_use		[1+1];		///< 현금영수증 용도 (0:개인, 1:법인)
	char ui_card_pwd		[10+1];		///< 카드비밀번호

	char rf_trcr_dvs_cd		[1+1];		///< RF교통카드 구분코드

	char read_dt			[8+1];		///< 요청 조회 대상일		
	char read_time			[6+1];		///< 요청 조회 대상 시각		
	char bus_cls_cd			[3+1];		///< 버스등급코드

	char rot_id				[17+1];		///< 노선ID			
	char rot_sqno			[5+1];		///< 노선순번			
	char alcn_dt			[8+1];		///< 배차일자			
	char alcn_sqno			[5+1];		///< *배차순번
	char depr_trml_cd		[7+1];		///< 출발터미널코드		
	char arvl_trml_cd		[7+1];		///< 도착터미널코드		

	char depr_trml_nm		[100+1];	///< 출발터미널 이름(한글)
	char arvl_trml_nm		[100+1];	///< 도착터미널 이름(한글)
	char depr_trml_eng_nm	[100+1];	///< 출발터미널 이름(영문)		
	char arvl_trml_eng_nm	[100+1];	///< 도착터미널 이름(영문)

	char alcn_way_dvs_cd	[1+1];		///< 배차방식 구분코드
	char sati_use_yn		[1+1];		///< 좌석제 사용유무

	char pyn_mns_dvs_cd		[1+1];		///< 지불수단구분코드		
	char pub_chnl_dvs_cd	[1+1];		///< 발행채널구분코드		
	char tisu_way_dvs_cd	[1+1];		///< 발권방식구분코드		

	char trd_dvs_cd			[1+1];		///< 거래구분코드			
	char fallback_dvs_cd	[1+1];		///< FallBack구분코드	
	char union_pay_yn		[1+1];		///< 은련카드여부			
	char pos_pg_ktc_ver		[20+1];		///< POS단말기버전정보		
	char enc_dta_len		[4+1];		///< *ENC 데이터 길이		
	char enc_dta			[1024+1];	///< ENC 데이터			
	char emv_dta			[1024+1];	///< EMV 데이터			
	char mip_term			[2+1];		///< *할부기간	
	char spad_pwd_yn		[1+1];		///< 
	char spad_dta			[3072+1];	///< 싸인패드데이터		
	char spad_dta_len		[8+1];		///< 싸인패드데이터길이	
	char user_dvs_no		[20+1];		///< 사용자구분번호 (상주직원일때만 값을 전송)
	char old_mbrs_no		[16+1];		///< 구회원번호

    // 20221017 ADD~
    char qr_cd_no			[100];      // QR코드번호
    char qr_pym_pyn_dtl_cd	[2];		// QR결제지불상세코드 // 20221228 ADD
    char payco_virt_ontc_no	[21];		// OTC번호;페이코가상일회성카드번호
    // 20221017 ~ADD

	/// 20190923 add by nhso : 상주직원 정보
	char user_pwd			[100+1];	///< 사용자비밀번호			
	char tisu_req_yn		[1+1];		///< 발권요청여부			
	char enc_dta_key		[32+1];		///< ENC 데이터			
	/// ~20190923 add by nhso : 상주직원 정보

	char rsd_nm					[100+1];  /// 상주직원 이름
	char rsd_cmpy_nm			[100+1];  /// 상주직원 소속명
	char rsd_tel_nm				[100+1];  /// 상주직원 전화번호 (사용자구분코드)

	/// 2020.04.03 인천공항 코로나 추가요구 사항
	char pub_user_tel_no		[100+1];  /// (인천공항) 발행자 전화번호
	char pub_user_krn			[100+1];  /// (인천공항) 발행자 여권번호 or 주민번호
	char ride_vhcl_dvs			[2+1];	  /// (인천공항) 승차 차량구분코드

} PUBTCK_T, *PPUBTCK_T;

/// 221 response (ktc 카드발행)
typedef struct 
{
	char card_no			[100+1];///< 카드번호
	char card_aprv_no		[100+1];///< 카드승인번호
	char card_aprv_amt		[18+1];	///< 카드승인금액(number)
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
	char dist				[18+1];	///< 거리(number)
	char take_drtm			[18+1];	///< 소요시간(number)
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
	char card_tr_add_inf	[100+1];///< 카드거래추가정보
} PUBTCK_CARD_T, *PPUBTCK_CARD_T;

/// 132 response (현금 승차권 발행)
typedef struct 
{
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
	char rdhm_mltp_val		[100+1];///< 승차홈다중값			
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
} PUBTCK_CASH_T, *PPUBTCK_CASH_T;

/// 134 response (현금 승차권 발행)
typedef struct 
{
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
	char rdhm_mltp_val			[100+1];///< 승차홈다중값			
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
} PUBTCK_CSRC_T, *PPUBTCK_CSRC_T;

/// 222 response (현금영수증 카드 승차권 발행)
typedef struct 
{
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
} PUBTCK_CSRC_CARD_T, *PPUBTCK_CSRC_CARD_T;

typedef struct 
{
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
	char rf_trcr_dvs_cd			[1+1];		///< RF교통카드 구분코드
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
} PUBTCK_UI_RF_CARD_T, *PPUBTCK_UI_RF_CARD_T;


typedef struct 
{
	char sats_pcpy_id		[17+1];	///< 좌석선점id			
	char cty_bus_dc_knd_cd	[1+1];	///< 시외버스할인종류코드	
	char dcrt_dvs_cd		[1+1];	///< 할인율구분코드		
	char bus_tck_knd_cd		[4+1];	///< 버스티켓종류코드		
	char sats_no			[5+1];	///< 좌석번호			
	int  n_tisu_amt			     ;	///< 발권금액
} UI_SATS_T, *PUI_SATS_T;

#pragma pack()

//----------------------------------------------------------------------------------------------------------------------


// 현장발권
class CPubTckMem
{
private:
	CPubTckMem();
	~CPubTckMem();

public:

	PUBTCK_T base;

	PUBTCK_CARD_T card_resp;
	PUBTCK_CASH_T cash_resp;
	PUBTCK_CSRC_T csrc_resp;
	PUBTCK_CSRC_CARD_T csrc_card_resp;
	
	//PUBTCK_RF_CARD_T rf_resp;

	///< UI_324 request data
	vector<PUBTCK_UI_RF_CARD_T>	m_vtRfUiData;  
	
	///< 서버 260 resp data
	vector<rtk_pubtckppy_t>	m_vtRfResp;  


	// 130 배차리스트
	//int n_alcn_num;									///< 배차리스트 갯수
	vector<rtk_readalcn_list_t> m_vtAlcnInfo;  			///< 

	// 131 배차요금조회
	char sats_arct_cd		[3+1];						///< 좌석배치도유형코드
	char sats_sta_inf		[45+1];						///< 좌석상태정보
	char rsd_nop_card_dc_yn	[1+1];						///< 상주인원카드할인여부

	int n_fee_num;										///< 배차요금 갯수
	vector<rtk_readalcnfee_list_t>	m_vtFeeInfo;  

	// 20211206 ADD~
	int n_whch_sats_num;								///< 휠체어좌석수
	vector<rtk_readalcnwhch_list_t>	m_vtWhchInfo;  
	// 20211206 ~ADD

	// UI 좌석선택 정보
	int n_ui_sats_num;									///< UI에서 좌석 선택 갯수
	vector<UI_SATS_T>				m_vtUiSats;  

	// 202 좌석정보 resp
	int  n_pcpysats_num			 ;						///< *좌석 선점 수
	vector<rtk_pcpysats_list_t>		m_vtPcpysats;		///< 배차모드 - 좌석선점
	
	vector<rtk_pcpysats_list_t>		m_vtNPcpysats;		///< 비배차모드 - 좌석선점

	/// 20191008 insert by nhso
	rtk_readrsd_t					m_resp_readrsd_t;	///< 상주직원 조회 응답
	vector<UI_RESP_STAFF_CD_MOD_FARE_LIST_T> m_vtStaffModFareReq;		///< 상주직원카드_좌석요금 변경 요청
	/// ~20191008 insert by nhso

	// 221 KTC 승차권발행 resp
	int  n_card_pub_num				 ;					///< *(신용카드) 발행 매수			
	vector<rtk_ktcpbtckcard_list_t> m_vtPbTckCard;  

	// 132 현금-승차권발행 resp
	int  n_cash_pub_num				 ;					///< *(현금) 발행 매수			
	vector<rtk_pubtckcash_list_t>	m_vtPbTckCash;  

	// 134 수기 현금영수증-승차권발행 resp
	int  n_csrc_pub_num				 ;					///< *(현금영수증) 발행 매수			
	vector<rtk_pubtckcsrc_list_t>	m_vtPbTckCsrc;  

	// 222 현금영수증카드-승차권발행 resp
	vector<rtk_ktcpbtckcsrc_list_t>	m_vtPbTckCsrcCard;  ///< 

	// 136 승차권 재발행
	vector<rtk_rpubtck_t> m_vtPbTckReIssue;

	int n_pbtck_chg_money			;					///< 거스름돈
	int n_pbtck_count				;					///< 발행매수
	int n_total_tisu_amt			;					///< 총 발행금액

	// 승차권 인쇄
	vector<TCK_PRINT_FMT_T>				m_vtPrtTicket;  
	vector<TCK_PRINT_FMT_T>				m_vtPrtTicketTest;  // 20211116 ADD
	//TCK_PRINT_FMT_T		tck_print_fmt;

public:
	static CPubTckMem* GetInstance()
	{
		static CPubTckMem m_instance;

		return &m_instance;
	}

	void Initialize(void);

	int MakeAlcnListPacket(char *pSend);
	int FindBusDcKind(char *bus_tck_knd_cd, char *ret_bus_dc_knd_cd, char *ret_dcrt_dvs_cd);

	void MakePasswdChars(char *retBuf, char *pSrc);
//	void MakeDepartureDateTime(char *pDT, char *pTime, char *retDT, char *retTM);

	void Make_IIAC_RsdData(char *pData);

	int MakeCashTicketPrtData(void);
	int MakeCsrcTicketPrtData(void);
	int MakeCsrcCardTicketPrtData(void);
	int MakeCreditTicketPrtData(BOOL bRF);
	int MakeRfCardTicketPrtData(void);

	int MakeReTicketPrtData(void);
	
	int ErasePcpySatsInfo(char *sat_no);

};

#pragma pack(1)

typedef struct  
{
	char req_read_dvs_cd	[1+1];		///< 요청 조회 구분		
	char read_wnd_dvs		[1+1];		///< 조회 창구 구분		
	char card_no			[1024+1];	///< 카드번호			
	char pos_pg_ktc_ver		[20+1];		///< POS단말기버전정보		
	char enc_dta_len		[4+1];		///< ENC 데이터 길이		
	char enc_dta			[1024+1];	///< ENC 데이터			
	char mnpp_tel_no		[50+1];		///< 예약자전화번호		
	char mnpp_brdt			[6+1];		///< 예약자생년월일		
	char mrs_no				[17+1];		///< 예매번호			

	char depr_trml_nm		[100];		///< 출발터미널코드(한글)
	char arvl_trml_nm		[100];		///< 도착터미널코드(한글)
	char depr_trml_eng_nm	[100];		///< 출발터미널코드(영문)
	char arvl_trml_eng_nm	[100];		///< 도착터미널코드(영문)

	char pub_chnl_dvs_cd	[1+1];		///< 발행 채널 구분코드
	char tisu_chnl_dvs_cd	[1+1];		///< 발권 채널 구분코드

} MRNP_BASE_T, *PMRNP_BASE_T;

// 예매발권 결과
typedef struct  
{
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
} PUBMRNP_RES_T, *PPUBMRNP_RES_T;

#pragma pack()

// 예매발권	-
class CMrnpMem
{
public:
	CMrnpMem();
	~CMrnpMem();

	// 137 예매발권 송신
	MRNP_BASE_T Base;

	// 137 예매발권 리스트 수신
	int n_mrs_num;									///< SVR_137 - 예매리스트 갯수
	vector<rtk_readmrnppt_list_t> m_vtMrnpList;		///< SVR_137 - 예매리스트 

	//int n_ui_issue_count;
	vector<UI_RESP_MRS_ISSUE_LIST_T> m_vtRcvUiIssue;///< UI_205 - 수신 전문

	// 138 예매발권 송신
	//vector<stk_pubmrnp_list_t> m_vtReqPubMrnp;	///< 138 송신

	// 138 예매발권 수신
	PUBMRNP_RES_T	res_complete;						///< SVR_138 수신결과
	int n_mrs_pub_num;								///< SVR_138 예매발권 갯수
	vector<rtk_pubmrnp_list_t>	m_vtResComplete;		///< SVR_138 에매발권 리스트

	vector<TCK_PRINT_FMT_T>		m_vtPrtTicket;

	// 승차권 인쇄
	//TCK_PRINT_FMT_T		tck_print_fmt;

	int MakeMrnpListPacket(char *pData);
	int MakeReqIssuePacket(char *pData);
	int MakeTicketPrtData(void);


public:
	static CMrnpMem* GetInstance()
	{
		static CMrnpMem m_instance;

		return &m_instance;
	}

	void Initialize(void);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 환불
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma pack(1)

typedef struct  
{
	char pub_sys_dvs_cd		[1+1];	///< 발행시스템구분코드	
	char pub_dt				[8+1];	///< 발행일자		
	char pub_shct_trml_cd	[4+1];	///< 발행단축터미널코드	
	char pub_wnd_no			[2+1];	///< 발행창구번호		
	char pub_sno			[4+1];	///< 발행일련번호		
	char eb_bus_tck_sno		[22+1];	///< eb버스티켓일련번호
} REQ_TICKET_NO_T, *PREQ_TICKET_NO_T;

typedef struct  
{
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
	/* // 20221220 ADD~
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
	char brkp_type				[1	+1];	///< 위약금 타입(BRKP_TYPE)					// 20221220 ADD
	char qr_pym_pyn_dtl_cd		[2	+1];	///< QR결제지불상세코드(QR_PYM_PYN_DTL_CD)	// 20221220 ADD
} RESP_TICKET_NO_T, *PRESP_TICKET_NO_T;

// 145 티켓 취소/환불 송신

typedef struct  
{
	char canc_ry_dvs_cd		[1+1];	///< 취소환불구분코드	
	char ryrt_num			[2+1];	///< 환불매수		
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

} REQ_TICKET_REFUND_T, *PREQ_TICKET_REFUND_T;

#pragma pack()

// 환불 Class
class CCancRyTkMem
{
public:
	CCancRyTkMem();
	~CCancRyTkMem();

	REQ_TICKET_NO_T						tReqTckNo;			/// (158) 버스티켓 일련번호 조회 송신 데이타
	RESP_TICKET_NO_T					tRespTckNo;			/// (158) 응답 데이타
	vector<rtk_readbustckno_list_t>		m_vtRespTckNoList;	/// (158) 응답 데이타 list

	REQ_TICKET_REFUND_T					tReqRefund;			/// (145) 티켓 환불 송신
	vector<rtk_cancrytck_list_t>		m_vtRespRefundList;	/// (145) 응답 데이타 list

	char ui_pym_dvs_cd[1];			/// ui 결제수단 (1:현금, 2:카드)
	int n_tot_money;				/// 결제금액
	int n_chg_money;				/// 방출금액
	int n_commission_fare;			/// 취소 수수료 금액
	int n_commission_rate;			/// 수수료율

	char receipt_yn			[1]	;	///< 환불영수증 출력여부

	char szTicketData[1024];		/// 승차권 데이타

public:
	static CCancRyTkMem* GetInstance()
	{
		static CCancRyTkMem		m_instance;

		return &m_instance;
	}

	void Initialize(void);

	int CalcRefund(void);
	int MakeTmaxRefundData(char *pData);

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Config Class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma pack(1)

// 미방출 정보
typedef struct  
{
	char byACK;
	WORD wCount;
	char szDate[14];
	char szWndNo[4];
	char chFlag;
	WORD w100;
	WORD w500;
	WORD w1k;
	WORD w5k;
	WORD w10k;
	WORD w50k;
} REQ_NO_OUT_CASH, *PREQ_NO_OUT_CASH;

typedef struct  
{
	char rot_id				[17+1];	///< 노선ID
	char trml_cd			[7+1];	///< 터미널코드
} CCS_THRU_INFO_T, *PCCS_THRU_INFO_T;

typedef struct  
{
	char code				[10];	///< 고속 운송사 코드
	char full_nm			[50];	///< 고속 운송사 Full Name
	char ko_nm				[50];	///< 고속 운송사 한글 Name
	char en_nm				[50];	///< 고속 운송사 영어 Name
	char chn_nm				[50];	///< 고속 운송사 중국 Name
	char jp_nm				[50];	///< 고속 운송사 일본 Name
	char biz_no				[50];	///< 고속 운송사 사업자번호
	char use				[10];	///< 고속 운송사 사용유무	
} KO_CORP_INFO_T, *PKO_CORP_INFO_T;

typedef struct 
{
	char req_user_no		[6+1];	///< 요청 사용자번호

} TMEXP_CONFIG_MEM_T, *PTMEXP_CONFIG_MEM_T;

typedef struct 
{
	BOOL bExp					;	///< 고속 환불 Flag

} REFUND_MEM_T, *PREFUND_MEM_T;

#pragma pack()

class CConfigTkMem
{
public:
	CConfigTkMem();
	~CConfigTkMem();

	char trml_nm				[100+1];	///< 자기 터미널 이름(한글)
	char trml_eng_nm			[100+1];	///< 자기 터미널 이름(영문)

	char ui_version				[20	+0];	///< ui version
	char main_version			[20	+0];	///< mcu version
	char pos_ktc_version		[20	+1];	///< ktc version

	int	 n_print_kind					;	///< 프린트종류
	char prt_beg_dt				[20	+1];	///< 프린트 시작일시

	char req_trml_user_id		[17	+1];	///< 사용자ID
	char user_nm				[100+1];	///< 사용자이름
	char tisu_ltn_drtm			[5	+1];	///< 발권제한시간(number)

	int n_cash_tisu_num					;	///< 현금 발권 매수
	int n_cash_tisu_amt					;	///< 현금 발권 금액

	int n_card_tisu_num					;	///< 카드발권 매수
	int n_card_tisu_amt					;	///< 카드발권 금액

	int n_card_canc_ry_num				;	///< 취소/환불 매수
	int n_card_canc_ry_amt				;	///< 취소/환불 금액
	char user_no				[10	+1];	///< 사용자 번호
	char trml_cd				[7	+1];	///< 터미널코드

	int n_language						;	///< 언어종류
	int n_bus_dvs						;	///< 버스종류 (시외/코버스/이지고속)

	char bus_tck_inhr_no		[8	+1];	///< 버스티켓 고유번호
	char add_bus_tck_inhr_no	[8	+1];	///< 추가 버스티켓 고유번호

	char damo_enc_dta_key		[32	+1];	///< 암호문키
	char damo_enc_dta_key_len	[2	+10];	///< 암호문키길이 (길이가 2바이트가 아님......)

	char kobus_tak_stt_dt		[8	+1]	;	///< 작업시작일자	
	char kobus_stt_time			[6	+1]	;	///< 시작시각	

	char depr_trml_eng_nm		[100+1]	;	///< 출발터미널 영문이름

	char rsp_cd					[6	+1];	/// 
	char rsp_str				[200+1];	/// 

	REQ_NO_OUT_CASH				reqNoOutCash;			/// 미방출 정보

	CCS_THRU_INFO_T				m_tThruInfo[500];	/// 

	TMEXP_CONFIG_MEM_T			m_tTmExp;
	REFUND_MEM_T				m_Refund;			/// 환불 

	vector<KO_CORP_INFO_T>		m_vtKoCorpList;	/// (145) 응답 데이타 list

public:
	static CConfigTkMem* GetInstance()
	{
		static CConfigTkMem		m_instance;

		return &m_instance;
	}

	void SetErrorCode(char *pErrorCode);
	void SetErrorCode(char *pErrorCode, char *pErrStr);

	char *GetErrorCode(void);
	char *GetErrorContents(void);

	int GetUiSvrKind(void);
	void SetSvrKind(BYTE bySvr);
	int SearchThruInfo(char *pRotID, char *retBuf);

	int ReadKobusCorpInfo(char *pFileFullName);
	int GetKobusCorpInfo(char *pCode, char *retNm, char *retBizNo);

	int IsSatAlcn(char *alcn_way_dvs_cd, char *sati_use_yn);
	int IsNsatsAlcn(char *alcn_way_dvs_cd, char *sati_use_yn);
	int IsNalcn(char *alcn_way_dvs_cd);
	int IsPrintFirstCome(char *alcn_way_dvs_cd, char *sati_use_yn, char *depr_time_prin_yn, char *sats_no_prin_yn); // 20221207 MOD

	int IsPrintSatsNo(char *alcn_way_dvs_cd, char *sati_use_yn);

	int SetTmExpUserNo(char *user_no);
	int GetTmExpUserNo(char *user_no);

};

