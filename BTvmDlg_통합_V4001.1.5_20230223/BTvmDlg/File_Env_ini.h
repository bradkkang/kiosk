// File_Ini.h : 시외버스 무인발매기 Ini 파일 처리
// 
// 
//

#pragma once

//----------------------------------------------------------------------------------------------------------------------

///> section name
#define SECTION_NM_MACHINE				"MACHINE_INFO"
#define SECTION_NM_CCBUS_INFO			"CCBUS_INFO"
#define SECTION_NM_KOBUS_INFO			"KOBUS_INFO"
#define SECTION_NM_EZBUS_INFO			"EZ_INFO"

//#define SECTION_NM_SW_FUNCTION			"SW_FUNCTION"
#define SECTION_NM_DEV_COIN_100			"DEVICE_COIN_100"
#define SECTION_NM_DEV_COIN_500			"DEVICE_COIN_500"
#define SECTION_NM_DEV_BILL				"DEVICE_BILL"
#define SECTION_NM_DEV_DISPENSER		"DEVICE_BILL_DISPENSE"
#define SECTION_NM_DEV_TICKET_PRT		"DEVICE_TICKET_PRINTER"
#define SECTION_NM_DEV_RECEIPT_PRT		"DEVICE_RECEIPT_PRINTER"
#define SECTION_NM_DEV_TICKET_READER	"DEVICE_TICKET_READER"
#define SECTION_NM_DEV_CARD_READER		"DEVICE_CARD_READER"
#define SECTION_NM_DEV_RF				"DEVICE_RF"
#define SECTION_NM_DEV_UI				"DEVICE_UI"

///> env_opt.ini 파일
#define SECTION_NM_SERVICE				"KIOSK_SERVICE"
#define SECTION_NM_WND_CLOSE			"WND_CLOSE"
#define SECTION_NM_TCK_OPT				"TCK_PRINTER_OPTION"
#define SECTION_NM_PRINTER_OPT			"PRINTER_OPTION"

#define SECTION_NM_CASH_USE				"CASH_USE"
#define SECTION_NM_CASH_IN_USE			"CASH_IN_USE"

#define SECTION_NM_REFUND				"REFUND_FUNC"

///> key name

///> "MACHINE_INFO"
#define KEY_NM_PROTO_VERSION			"PROTO_VERSION"					///> 프로토콜 버젼
#define KEY_NM_KIOSK_HW_DVS				"KIOSK_HW_DVS"					///> 카드전용장비('1'), 현금전용장비('2'), 카드+현금겸용장비('3')
#define KEY_NM_HW_IC_USE_YN				"KIOSK_HW_IC_USE_YN"			///> (HW) IC카드 리더기 사용유무
#define KEY_NM_HW_CASH_USE_YN			"KIOSK_HW_CASH_USE_YN"			///> (HW) IC카드 리더기 사용유무
#define KEY_NM_HW_RF_USE_YN				"KIOSK_HW_RF_USE_YN"			///> (HW) IC카드 리더기 사용유무
#define KEY_NM_OPER_CORP				"KIOSK_OPER_CORP"				///> 운영사 정보

#define KEY_NM_IS_REAL_MODE				"IS_REAL_MODE"					///> 0:테스트 서버, 1:리얼 서버
//#define KEY_NM_PAY_METHOD				"PAY_METHOD"					///> 1(카드전용), 2(현금전용), 3(현금+카드)

#define KEY_NM_SW_IC_USE_YN				"SW_IC_USE_YN"					///> IC 결제 사용유무
#define KEY_NM_SW_CASH_USE_YN			"SW_CASH_USE_YN"				///> 현금 결제 사용유무
#define KEY_NM_SW_RF_USE_YN				"SW_RF_USE_YN"					///> RF 결제 사용유무

#define KEY_NM_SVC_USE_YN				"SERVICE_YN"					///> 무인기 서비스 시간 사용유무
#define KEY_NM_SVC_BEGIN_TIME			"BEGIN_TIME"					///> 무인기 서비스 시작 시간
#define KEY_NM_SVC_END_TIME				"END_TIME"						///> 무인기 서비스 종료 시간
#define KEY_ALCN_WAY_DVS_YN				"ALCN_WAY_DVS_YN"				///> 무인기 비배차 시간표시 옵션 // 20220902 ADD
#define KEY_REFUND_SATI_USE_YN			"REFUND_SATI_USE_YN"			///> 비좌석제 환불수수료 적용 옵션 // 20221201 ADD

///> "CCBUS_INFO", "KOBUS_INFO", "EZ_INFO"
#define KEY_NM_TRML_WND_NO				"WND_NO"						///> 터미널 창구번호
#define KEY_NM_TRML_CD					"TRML_CD"						///> 터미널 코드
#define KEY_NM_SHCT_TRML_CD				"SHORT_TRML_CD"					///> 단축 터미널 코드
#define KEY_NM_USER_NO					"USER_NO"						///> 사용자 ID
#define KEY_NM_USER_ID					"USER_ID"						///> 사용자 ID
#define KEY_NM_USER_PWD					"USER_PWD"						///> 사용자 비번
#define KEY_NM_MAINT_NO					"MAINT_NO"						///> 관리자 ID
#define KEY_NM_MAINT_PWD				"MAINT_PWD"						///> 관리자 비번

///> "DEVICE_INFO"
#define KEY_NM_USE						"USE"							///> 디바이스 사용유무
#define KEY_NM_MODEL					"MODEL"							///> 디바이스 모델번호
#define KEY_NM_COM_PORT					"COM_PORT"						///> 디바이스 통신포트

#define KEY_NM_IP						"IP_ADDR"						///> IP address
#define KEY_NM_TCP_PORT					"PORT"							///> TCP port

#define KEY_NM_AUTO_CLS_YN				"AUTO_CLS_YN"					///> 창구마감 유무
#define KEY_NM_REBOOT_FLAG				"REBOOT_FLAG"					///> 창구마감 시, rebooting 유무
#define KEY_NM_CLSPRINT_FLAG			"PRINT_FLAG"					///> 창구마감 시, 프린트 사용유무
#define KEY_NM_DAYCLOSE_FLAG			"DAYCLOSE_FLAG "				///> 창구마감 시, 1일 1마감 사용 유무 // 20210513 ADD

#define KEY_NM_TCKPRT_BOLD				"BOLD"							///> 티켓프린터 농도 설정값
#define KEY_NM_TCKPRT_DEPR_FMT			"DEPR_DATE_FMT"					///> 티켓프린터 출발일 출력 포맷
#define KEY_NM_TCKPRT_MIP_YN			"MIP_YN"						///> 티켓프린터 할부 사용유무

#define KEY_NM_TCKPRT_LEFT_GAP			"RECEIPT_LETT_GAP"				///> 영수증프린터 감열지 프린트시 왼쪽 간격

#define KEY_NM_NSATS_DEPR_DAY_YN 		"NSATS_DEPR_DAY_YN"				///> 비좌석제 승차권 출발일 출력 유무
#define KEY_NM_NSATS_DEPR_TIME_YN 		"NSATS_DEPR_TIME_YN"			///> 비좌석제 승차권 시간 출력 유무
#define KEY_NM_NALCN_NSATS_FIRST_CM_YN 	"NALCN_NSATS_FIRST_CM_YN"		///> 비배차, 비좌석제 선착순탑승문구 출력 유무
/// 2020.06.25 add code
#define KEY_NM_NALCN_TIME_CACM_YN		"NALCN_TIME_CACM_YN"			///> 비배차 : 시간, 운수사 출력 유무

#define KEY_NM_NALCN_DEPR_DAY_YN		"NALCN_DEPR_DAY_YN"				///> 비배차 승차권 "출발일" 출력 유무

#define KEY_NM_REALCHECK_YN 			"REAL_CHECK_YN"					///> 실시간 체크 유무

#define KEY_NM_W100						"W100"							///> 100원 
#define KEY_NM_W500						"W500"							///> 500원 
#define KEY_NM_W1K						"W1K"							///> 1,000원
#define KEY_NM_W5K						"W5K"							///> 5,000원 
#define KEY_NM_W10K						"W10K"							///> 10,000원
#define KEY_NM_W50K						"W50K"							///> 50,000원

#define KEY_NM_ADD_DEPR_TIME			"ADD_DEPR_TIME"					///> 추가 출발시간

//----------------------------------------------------------------------------------------------------------------------

///> section name
#define SECTION_NM_DBG_PBTCK			"PBTCK_DEBUG"
#define SECTION_NM_DBG_REFUND			"REFUND_DEBUG"

#define KEY_NM_DBG_ALCN					"ALCN"							///> 배차테스트
#define KEY_NM_DBG_BRANDING				"BRANDING"						///> 소인여부
#define KEY_NM_DBG_BRANDING_IMG			"BRANDING_IMG"					///> 소인파일명

//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

typedef struct 
{
	int		nUse;						///> 미사용 : 0, 사용 : 1
	int		nModel;						///> 모델번호
	int		nPort;						///> 통신포트

	char	szIPAddress[40];			///> IP Address
	int		nTcpPort;					///> TCP 포트
} DEV_CFG_T, *PDEV_CFG_T;

typedef struct 
{
	int		nUse;						///> 사용유무
	int		nKind;						///> 종류
	int		nTrmlWndNo;					///> 창구번호
	int		nTrmlCD;					///> 터미널 번호(7)
	int		nShctTrmlCD;				///> 단축 터미널 번호(4)	
	char	szUserNo[20];				///> 사용자 No
	char	szUserPwd[20];				///> 사용자 비번
	char	szMaintNo[20];				///> 관리자 ID
	char	szMaintPwd[20];				///> 관리자 비번
} SVR_INFO_T, *PSVR_INFO_T;

typedef struct
{
	char	szUseYN[10];				///> 서비스 사용유무
	char	szBegTime[20];				///> 서비스 시작시간
	char	szEndTime[20];				///> 서비스 종료시간

	char	szAutoClseYN[4];			///> 창구마감 유무
	char	szRebootFlag[4];			///> Reboot_Flag
	char	szPrintFlag[4];				///> 자동마감시, 프린트 유무
	char	szDayClsFlag[4];			///> 창구마감 시, 1일 1마감 사용 유무

	char	szAlcnWayDvsFlag[4];		///> 무인기 비배차 시간표시 옵션 // 20220902 ADD
} SVC_INFO_T, *PSVC_INFO_T;

typedef struct
{
	int		nBoldVal					; ///> 농도 설정값
	char	depr_date_fmt			[20]; ///> yyyymmdd, mmdd
	char	mip_yn					[10]; ///> 할부사용유무
	int		n_receipt_left_gap			; ///> ATEC 영수증프린터 출력시, 왼쪽 간격
	char	nsats_depr_day_yn		[10]; ///> 비좌석제 승차권 출발일 출력 유무 
	char	nsats_depr_time_yn		[10]; ///> 비좌석제 승차권 시간 출력 유무 
	char	nalcn_nsats_first_cm_yn	[10]; ///> 비배차, 비좌석제 선착순탑승문구 출력 유무 
	/// 2020.06.25 add code
	char	nalcn_time_cacm_yn		[10]; ///> 비배차: 시간, 운수사 출력 유무 
	char	nalcn_depr_day_yn		[10]; ///> 비배차 승차권 "출발일" 출력 유무
} TCK_PRT_OPT_T, *PTCK_PRT_OPT_T;

typedef struct
{
	char	RealTimeCheckYN		[4];	///> 실시간 상태 체크
} PRT_OPT_T, *PPRT_OPT_T;

typedef struct  
{
	int		nIs100;						///> 100원 - 0:사용안함, 1:사용함
	int		nIs500;						///> 500원 - 0:사용안함, 1:사용함

	int		nIs1K;						///> 천원권   - 0:사용안함, 1:사용함
	int		nIs5K;						///> 오천원권 - 0:사용안함, 1:사용함
	int		nIs10K;						///> 만원권   - 0:사용안함, 1:사용함
	int		nIs50K;						///> 오만원권 - 0:사용안함, 1:사용함
} CASH_INFO_T, *PCASH_INFO_T;

typedef struct  
{
	int		nAddDeprTm;					///> 추가 출발시간 (초)
	char	szRefundSatiUseFlag[4];		///> 비좌석제 환불수수료 적용 옵션 // 20221201 ADD
} REFUND_OPT_INFO_T, *PREFUND_OPT_INFO_T;

typedef struct 
{
	int			nProtoVer;				///< 프로토콜 버젼
	
//	int			nDeviceDVS;				///> HW : 카드전용(1), 현금전용(2), 카드+현금겸용(3)
	char		szDeviceDVS[10];		///> 제조사 - 'A':에이텍, 'N':한전금

	char		szHwIcUseYN[10];		///> (HW) IC카드 리더기 사용유무
	char		szHwCashUseYN[10];		///> (HW) 현금 리더기 사용유무
	char		szHwRfUseYN[10];		///> (HW) RF 리더기 사용유무
	int			nOperCorp;				///< 운영사 정보 (0x01:광주, 0x02:인천공항)
#define KUMHO_OPER_CORP		1		/// 금호 터미널
#define IIAC_OPER_CORP		2		/// 인천공항

	int			nIsRealMode;			///> 서버종류 : 0:테스트 서버, 1:리얼 서버
	//int			nPayMethod;			///> 결제구분 : 1(카드전용), 2(현금전용), 3(현금+카드)
	char		szSwIcUseYN[10];		///> (SW) IC카드 리더기 사용유무
	char		szSwCashUseYN[10];		///> (SW) 현금 리더기 사용유무
	char		szSwRfUseYN[10];		///> (SW) RF 리더기 사용유무

	SVC_INFO_T			tService;		///> 
	TCK_PRT_OPT_T		tTckOpt;		///> 티켓 프린터 설정 
	PRT_OPT_T			tPrtOpt;		///> 영수증 프린터 설정
	CASH_INFO_T			tCashOUT;		///> 지폐출금 시, 현금 사용유무
	CASH_INFO_T			tCashIN;		///> 지폐입금 시, 현금 사용유무
	REFUND_OPT_INFO_T	tRefund;		///> 환불 옵션

	SVR_INFO_T	tCcInfo;				///> 시외버스 서버 정보
	SVR_INFO_T	tKoInfo;				///> 코버스 서버 정보
	SVR_INFO_T	tEzInfo;				///> 이지고속 서버 정보

	DEV_CFG_T	tCoin100;				///> 100원 동전방출 serial device
	DEV_CFG_T	tCoin500;				///> 500원 동전방출 serial device
	DEV_CFG_T	tBill;					///> 지폐입금기 serial device
	DEV_CFG_T	tDispenser;				///> 지폐방출기 serial device
	DEV_CFG_T	tPrtTicket;				///> 승차권 발권기 serial device
	DEV_CFG_T	tPrtReceipt;			///> 영수증 프린터 serial device
	DEV_CFG_T	tTicketReader;			///> 승차권 리더기 serial device
	DEV_CFG_T	tCardReader;			///> 신용카드 리더기 serial device
	DEV_CFG_T	tRF;					///> RF serial device
	DEV_CFG_T	tUI;					///> UI tcp/ip 통신

} KIOSK_INI_ENV_T, *PKIOSK_INI_ENV_T;

//----------------------------------------------------------------------------------------------------------------------

#define DBG_ALL			0
#define DBG_PBTCK		1
#define DBG_REFUND		2

/// 현장발권_테스트
typedef struct 
{
	int		nAlcn;					///< 1:금호 비배차_비좌석_테스트모드
} PBTCK_TST_OPT_T, *PPBTCK_TST_OPT_T;

/// 환불_테스트
typedef struct 
{
	BOOL	bBranding;				///< 소인여부
	char	szBrandImg[100];		///< 소인이미지 파일명
} REFUND_TST_OPT_T, *PREFUND_TST_OPT_T;

typedef struct 
{
	PBTCK_TST_OPT_T		pbTck;		/// 현장발권 디버깅
	REFUND_TST_OPT_T	reFund;		/// 환불 디버깅
} KIOSK_INI_TEST_T, *PKIOSK_INI_TEST_T;

//----------------------------------------------------------------------------------------------------------------------
// [INI] 승차권 프린터

#define SECTION_NM_CCS_PTRG_TRML	"CCS_TRML_PRINT"
#define SECTION_NM_CCS_PTRG_CUST	"CCS_CUST_PRINT"

#define SECTION_NM_EXP_PTRG_TRML	"EXPRESS_TRML_PRINT"
#define SECTION_NM_EXP_PTRG_CUST	"EXPRESS_CUST_PRINT"

#define MAX_PTRG					2

/// 승차권 정보 최대 갯수
#define MAX_PTRG_ITEM				100

/// 티켓 회수용/승객용 구분
#define PTRG_TRML_PART				0
#define PTRG_CUST_PART				1

/// 티켓 정보 ID
enum _enPtrgID_
{
	/////////
	PTRG_ID1_PSNO					= 100	,	/// 발권일련번호
	PTRG_ID1_USRID							,	/// 사용자ID 
	PTRG_ID1_USRNM							,	/// 티켓일련번호
	PTRG_ID1_TCKNO							,	/// 티켓일련번호
	PTRG_ID1_WNDNO							,	/// 창구번호+"무인기" kh200710
	PTRG_ID1_MSG1					= 180   ,   /// 메시지_1 kh200708
	PTRG_ID1_MSG2					= 181   ,   /// 메시지_2 kh200708
	PTRG_ID1_MSG3					= 182   ,   /// 메시지_3 kh200708
	PTRG_ID1_MSG4					= 183   ,   /// 메시지_4 kh200708
	
	/////////
	PTRG_ID2_DEPR_KO				= 201	,	/// 출발지(한글) - 회수용
	PTRG_ID2_ARVL_KO				= 202	,	/// 도착지(한글) - 승객용
	PTRG_ID2_DEPR_KO_1				= 203	,	/// 출발지(한글) - 회수용
	PTRG_ID2_ARVL_KO_1				= 204	,	/// 도착지(한글) - 승객용
	PTRG_ID2_BUS_CLS				= 205	,	/// 버스등급
	PTRG_ID2_DEPR_EN				= 206	,	/// 출발지(영문) - 회수용
	PTRG_ID2_ARVL_EN				= 207	,	/// 도착지(영문) - 회수용
	PTRG_ID2_THRU_NM				= 208	,	/// 경유지
	PTRG_ID2_DIST					= 209	,	/// 거리
	PTRG_ID2_DEPR_EN_1				= 210	,	/// 출발지(영문) - 승객용
	PTRG_ID2_ARVL_EN_1				= 211	,	/// 도착지(영문) - 승객용
	PTRG_ID2_ROT_NO							,	/// 도착지 - 노선번호
	PTRG_ID2_PYM_DVS						,	/// 결제수단
	PTRG_ID2_ROT_NO_ARVL					,	/// 회수용 노선번호 + 도착지  kh_200717
	PTRG_ID2_ROT_NO_ARVL_1					,	/// 승객용 노선번호 + 도착지  kh_200717
	PTRG_ID2_ARVL_KO_JUN					,	/// 전주용 승객용 도착지 kh_200720
	PTRG_ID2_BUS_CLS_JUN					,	/// 전주용 버스등급 kh_200720


	
	
	// #
	PTRG_ID2_MSG1					= 280	,	/// 메시지_1
	PTRG_ID2_MSG2					= 281	,	/// 메시지_2
	PTRG_ID2_MSG3					= 282	,	/// 메시지_3
	PTRG_ID2_MSG4					= 283	,	/// 메시지_4

	/////////
	PTRG_ID3_FARE					= 301	,	/// 요금
	PTRG_ID3_PYM_DVS				= 302	,	/// 결제수단
	PTRG_ID3_TCK_KND				= 303	,	/// 승차권종류
	PTRG_ID3_BUS_CLS				= 304	,	/// (버스등급) kh_200708
	PTRG_ID3_PYM_DVS_VERTI			= 305	,	/// 버스등급 세로 kh_200708
	PTRG_ID3_ENMNPP_VERTI			= 306	,	/// 예매 세로
	PTRG_ID3_DRAWBOX				= 307	,	/// 전주용 드로우박스 kh_200713.
	PTRG_ID3_BUS_CLS_SHCT_NM		= 308	,	/// 짧은버스등급
	PTRG_ID3_BUS_CLS_SHCT_NM2		= 309	,	///	(짧은버스등급)


	// #
	PTRG_ID3_MSG1					= 380	,	/// 메시지_1
	PTRG_ID3_MSG2					= 381	,	/// 메시지_2
	PTRG_ID3_MSG3					= 382	,	/// 메시지_3
	PTRG_ID3_MSG4					= 383	,	/// 메시지_4


	/////////
	PTRG_ID5_DEPR_DT				= 501	,	/// 출발일자
	PTRG_ID5_DEPR_TIME				= 502	,	/// 출발시간
	PTRG_ID5_SATS_NO				= 503	,	/// 좌석번호
	PTRG_ID5_RDHM_VAL				= 504	,	/// 승차홈
	PTRG_ID5_BUS_CACM				= 505	,	/// 버스회사
	PTRG_ID5_FIRST_MSG1				= 506	,	/// 선착순 메시지 1
	PTRG_ID5_FIRST_MSG2				= 507	,	/// 선착순 메시지 2
	// #
	PTRG_ID5_MSG1					= 580	,	/// 메시지_1
	PTRG_ID5_MSG2					= 581	,	/// 메시지_2
	PTRG_ID5_MSG3					= 582	,	/// 메시지_3
	PTRG_ID5_MSG4					= 583	,	/// 메시지_4

	///////// 
	PTRG_ID6_APRV_NO				= 601	,	/// 승인번호
	PTRG_ID6_TLE_APRV_NO			= 602	,	/// 승인번호:값
	PTRG_ID6_APRV_AMT				= 603	,	/// 승인금액
	PTRG_ID6_TLE_APRV_AMT			= 604	,	/// 승인금액:값
	PTRG_ID6_CARD_NO				= 605	,	/// 카드번호
	PTRG_ID6_TLE_CARD_NO			= 606	,	/// 카드번호:값
	PTRG_ID6_PUB_TIME				= 607	,	/// 발권시간 (시:분:초)
	PTRG_ID6_DEPR_NM				= 608	,	/// 출발지
	PTRG_ID6_BAR_DATA				= 609	,	/// 바코드 정보 (pub_dt-pub_shct_trml_cd-pub_wnd_no-pub_sno-secu_code)
	PTRG_ID6_TLE_TRML_CORP_NO		= 610	,	/// 타이틀 : 가맹점 사업자번호
	PTRG_ID6_BUS_CACM				= 611	,	/// 버스회사
	PTRG_ID6_TLE_BUS_CACM			= 612	,	/// 버스회사:값
	PTRG_ID6_QR_DATA				= 613	,	/// QR 정보 (pub_dt-pub_trml_cd-pub_wnd_no-pub_sno)
	PTRG_ID6_TRML_TEL_NO					,	/// 버스터미널 연락처
	PTRG_ID6_TRML_TEL_NO2					,	/// (버스터미널 연락처)
	PTRG_ID6_SHRT_PUB_TIME					,	/// 발권시간(시:분) kh200708
	PTRG_ID6_DEPR_ARVL_SATS					,	/// 전주전용 출발코드 도착코드 자리번호 kh_200710
	PTRG_ID6_TLE_DEPR_NM					,	/// 타이틀:출발지 kh_200710
	PTRG_ID6_TLE2_APRV_AMT					,	/// 승인금액 값(:없는ver)  kh_200710
	PTRG_ID6_TRML_CORP_NO					,	/// 가맹점(터미널) 사업자번호 kh_200713
	PTRG_ID6_TLE2_CARD_NO					,	/// 카드번호 (: 없는ver) kh_200713
	PTRG_ID6_TLE2_APRV_NO					,	/// 승인번호 (: 없는ver) kh_200713
	PTRG_ID6_CACM_BIZ_NO					,	/// 출발지 버스회사 사업자번호 kh_200713
	PTRG_ID6_TRML_CORP_NO1					,	/// 터미널 사업자번호 kh_200720
	
	PTRG_ID6_TLE_REG_NO				= 630	,	/// 티머니고속 - 등록번호(=현영사용자정보)

	PTRG_ID6_BUS_CACM_BIZ_NO		= 645	,	/// 운수회사이름(사업자번호)
	PTRG_ID6_QR_DATA_2				= 646	,	/// QR Data 2

	PTRG_ID6_CARD_TR_ADD_INF		= 647	,	/// 선불카드 잔액	- 2021/02/17 add

	// #
	PTRG_ID6_MSG1					= 680	,	/// 메시지1("*유효기간 당일 지정차에 한함")
	PTRG_ID6_MSG2					= 681	,	/// 메시지2("현금영수증 문의 : 126-2")
	PTRG_ID6_MSG3					= 682	,	/// 메시지3
	PTRG_ID6_MSG4					= 683	,	/// 메시지4

	///////// 
	PTRG_ID6_BAR_CODE				= 701	,	/// 바코드 (높이: 7.5mm, 가로크기: n=4)
	PTRG_ID6_QR_CODE				= 702	,	/// QR

	// 20221012 ADD
	PTRG_ID6_BAR_CODE_H6_N3			= 711	,	/// 바코드 (높이: 6.25mm, 가로크기: n=3)
	// 20221012 ~ADD

};

typedef struct 
{
	int			nUse;					/// 사용유무
	int			nID;					/// ID
	int			nX;						/// X좌표
	int			nY;						/// Y좌표
	int			nMode;					/// 인쇄모드 (세로확대, 가로확대, 양쪽확대 등등)
	int			nBold;					/// Bold 유무
	int			nRotate;				///	Rotate (전주용) kh_200716
	char		szMsg[100];				/// 메시지
} PTRG_INFO_T, *PPTRG_INFO_T;

typedef struct 
{
	PTRG_INFO_T			trmlPtrg[MAX_PTRG_ITEM];	/// 회수용
	PTRG_INFO_T			custPtrg[MAX_PTRG_ITEM];	/// 승객용
} KIOSK_INI_PTRG_T, *PKIOSK_INI_PTRG_T;

#pragma pack()

//----------------------------------------------------------------------------------------------------------------------

void* GetEnvInfo(void);

int GetEnvOperCorp(void);
char *GetEnvSvrUserNo(int nSvrKind);
char *GetEnvSvrUserPWD(int nSvrKind);

char* GetEnvCoin100Info(void);
char* GetEnvCoin500Info(void);
char* GetEnvBillInfo(void);
char* GetEnvDispenserInfo(void);
char* GetEnvPrtTicketInfo(void);
char* GetEnvPrtReceiptInfo(void);
char* GetEnvTicketReaderInfo(void);
char* GetEnvCardReaderInfo(void);
char* GetEnvUIInfo(void);
char* GetEnvTckPrtInfo(void);
char* GetEnvPrinterInfo(void);

int GetEnvServerInfo(void);

int IsEnvPrinterRealCheck(void);




//int GetEnvDeviceInfo(void);
int GetEnv_IsRealMode(void);

int INI_ReadEnvFile(void);
void WriteEnvIniFile(char *pData);

void SetEnvServerDVS(int ccsSvr, int expSvr);

int INI_ReadDebugFile(void);
void *GetDebugInfo(int nLevel);


int Init_EnvIniFile(void);
int Term_EnvIniFile(void);

int CheckServiceTime(void);

int INI_Use10K(void);

int INI_UseCashIn1K(void);
int INI_UseCashIn5K(void);
int INI_UseCashIn10K(void);
int INI_UseCashIn50K(void);

int INI_ReadEnvTckPtrgFile(void);
char *INI_GetEnvTckPtrgInfo(int nSvrKind, int nPtrgPart, int nPtrgID);
char *INI_GetEnvTckPtrg(int nSvrKind);

