// 
// 
// dev_ui_main.h : UI main 헤더 파일
//

#pragma once

#include "MySocket.h"
#include "MyLogFile.h"
#include "MyDataAccum.h"

//----------------------------------------------------------------------------------------------------------------------

enum __enUIState__
{
	UI_NONE_STATE		= 0	,
	UI_CONNECT_STATE		,
	UI_DATA_STATE			,
};

//----------------------------------------------------------------------------------------------------------------------

#define UI_CMD_POLLING				0x0001			///> Polling (MAIN -> UI)
#define UI_CMD_OK_CANCEL			0x0101			///> 거래 취소/완료 처리 (UI -> MAIN)
#define UI_CMD_SERVICE				0x0201			///> 사용중/사용중지 처리 (UI -> MAIN, MAIN -> UI)
#define UI_CMD_ALARM				0x0301			///> 알람/이벤트 전송 처리 (MAIN -> UI)
#define UI_CMD_ACCOUNT				0x0401			///> 회계 정보 전송 처리 (MAIN -> UI)
#define UI_CMD_VERSION_REQ			0x0501			///> 버젼 정보 (UI ->MAIN, MAIN -> UI)
#define UI_CMD_DEV_CONFIG			0x0601			///> 장비환경설정 정보 (MAIN -> UI)
#define UI_CMD_TRML_INFO			0x0701			///> 터미널 정보 (MAIN -> UI)
#define UI_CMD_DEV_FILE_INFO		0x0801			///> File 정보 (MAIN -> UI)

/// 예매발권
#define UI_CMD_MRS_MAIN				0x0002			///> (UI->MAIN) 예매발권 - 시작
#define UI_CMD_MRS_CARD_RD			0x0102			///> (UI->MAIN) 예매발권 - 신용카드 읽기
#define UI_CMD_MRS_CARD_INFO		0x0202			///> (MAIN->UI) 예매발권 - 신용카드 정보 전달
//#define UI_CMD_MRS_INPUT			0x0302			///> (UI->MAIN) 예매발권 - 예매번호 입력 
#define UI_CMD_MRS_REQ_LIST			0x0302			///> (UI->MAIN) 예매발권 - 예매 리스트 요청 
#define UI_CMD_MRS_RESP_LIST		0x0402			///> (MAIN->UI) 예매발권 - 예매 리스트 전달
#define UI_CMD_MRS_ISSUE			0x0502			///> (UI->MAIN) 예매발권 - 예매 발권 시작
#define UI_CMD_MRS_ISSUE_STATE		0x0702			///> (MAIN->UI) 예매발권 - 예매 발권 진행상태

/// 현장발권
#define UI_CMD_PBTCK_MAIN			0x0003			///> (UI->MAIN) 현장발권 - 시작
#define UI_CMD_PBTCK_REQ_LIST		0x0103			///> (UI->MAIN) 현장발권 - 배차 리스트 요청
#define UI_CMD_PBTCK_RESP_LIST		0x0203			///> (MAIN->UI) 현장발권 - 배차 리스트 전달
#define UI_CMD_PBTCK_LIST_SEL		0x0303			///> (UI->MAIN) 현장발권 - 배차 리스트 선택
#define UI_CMD_PBTCK_FARE_INFO		0x0403			///> (MAIN->UI) 현장발권 - 좌석 정보 전달
#define UI_CMD_PBTCK_SEAT_SEL		0x0503			///> (UI->MAIN) 현장발권 - 좌석 선택
#define UI_CMD_PBTCK_SEAT_INFO		0x0603			///> (MAIN->UI) 현장발권 - 좌석 선택 정보 전달
#define UI_CMD_PBTCK_PYM_DVS		0x0703			///> (UI->MAIN) 현장발권 - 결제수단 선택
#define UI_CMD_PBTCK_INS_BILL		0x0803			///> (UI->MAIN) 현장발권 - 지폐 투입
#define UI_CMD_PBTCK_BILL_INFO		0x0903			///> (MAIN->UI) 현장발권 - 지퍠 투입 정보
//#define UI_CMD_PBTCK_CSRC_READ		0x1003			///> (UI->MAIN) 현장발권 - (현금영수증 or 신용카드) 읽기 시작/종료
#define UI_CMD_PBTCK_REQ_PYM		0x1103			///> (UI->MAIN) 현장발권 - 승차권 결제 요청

#define UI_CMD_PBTCK_TMAX_RESULT	0x1203			///> (MAIN->UI) 현장발권 - TMAX 승차권 발권 결과

#define UI_CMD_PBTCK_TCK_ISSUE		0x1303			///> (UI->MAIN) 현장발권 - 승차권 발권 시작
#define UI_CMD_PBTCK_TCK_ISSUE_RET	0x1403			///> (MAIN->UI) 현장발권 - 승차권 발권 결과 전송
#define UI_CMD_PBTCK_CHG_MONEY		0x1503			///> (UI->MAIN) 현장발권 - 거스름돈 방출 시작
#define UI_CMD_PBTCK_CHG_MONEY_RET	0x1603			///> (MAIN->UI) 현장발권 - 거스름돈 방출 정보 전송
#define UI_CMD_PBTCK_CARD_INFO		0x1703			///> (UI->MAIN) 현장발권 - 신용카드 데이타 전달

/// 환불
#define UI_CMD_CANCRY_MAIN			0x0004			///> (UI->MAIN) 환불 - 메인
#define UI_CMD_CANCRY_TCK_READ		0x0104			///> (UI->MAIN) 환불 - 승차권 판독
#define UI_CMD_CANCRY_INFO			0x0204			///> (MAIN->UI) 환불 - 승차권 판독 결과 정보
#define UI_CMD_CANCRY_FARE			0x0304			///> (UI->MAIN) 환불 - 환불 처리 (카드 또는 현금)
#define UI_CMD_CANCRY_FARE_INFO		0x0404			///> (MAIN->UI) 환불 - 카드 또는 현금 방출 정보 전송

/// 관리자
#define UI_CMD_ADMIN_MAIN			0x0005			///> (UI->MAIN) 관리자 - 메인
#define UI_CMD_ADMIN_LOGIN			0x0105			///> (UI->MAIN) 관리자 - 로그인
#define UI_CMD_ADMIN_SET_FUNC		0x0205			///> (UI->MAIN) 관리자 - 설정관리 - 기능 설정
#define UI_CMD_ADMIN_SET_ENV		0x0305			///> (UI->MAIN) 관리자 - 설정관리 - 환경 설정
#define UI_CMD_ADMIN_SET_THRML		0x0405			///> (UI->MAIN) 관리자 - 설정관리 - 터미널 설정
#define UI_CMD_ADMIN_SET_TICKET		0x0505			///> (UI->MAIN) 관리자 - 설정관리 - 승차권 설정
#define UI_CMD_ADMIN_SET_BUS_CACM	0x0605			///> (UI->MAIN) 관리자 - 설정관리 - 운수사 설정
#define UI_CMD_ADMIN_SYSTEM			0x1105			///> (UI->MAIN) 관리자 - 시스템관리
#define UI_CMD_ADMIN_DEV_RESET		0x1205			///> (UI->MAIN) 관리자 - 시스템관리 - 장비별 리셋


#define UI_CMD_ADMIN_MNG_TCK_INHRNO	0x2105			///> (UI->MAIN) 관리자 - 티켓고유번호 설정
#define UI_CMD_ADMIN_MNG_TCK_COUNT	0x2205			///> (UI->MAIN) 관리자 - 티켓신규수량
#define UI_CMD_ADMIN_REQ_TCK_INFO	0x2305			///> (UI->MAIN) 관리자 - 티켓정보 요청
#define UI_CMD_ADMIN_RSP_TCK_INFO	0x2405			///> (MAIN->UI) 관리자 - 티켓정보 전송

#define UI_CMD_ADMIN_INQ_ISSUE		0x3105			///> (UI->MAIN) 관리자 - 내역조회
#define UI_CMD_ADMIN_RE_ISSUE		0x3205			///> (UI->MAIN) 관리자 - 재발행
#define UI_CMD_ADMIN_CASH_HISTO		0x4105			///> (UI->MAIN) 관리자 - 시재관리 - 시재내역
#define UI_CMD_ADMIN_CASH_INSERT	0x4205			///> (UI->MAIN) 관리자 - 시재관리 - 시재보급
#define UI_CMD_ADMIN_CASH_OUT		0x4305			///> (UI->MAIN) 관리자 - 시재관리 - 시재출금

#define UI_CMD_ADMIN_INQ_CASH_INOUT	0x4405			///> (UI->MAIN) 관리자 - 시재관리 - 입출내역
#define UI_CMD_ADMIN_WITHDRAW_BILL	0x4505			///> (UI->MAIN) 관리자 - 시재관리 - 지폐함 회수
#define UI_CMD_ADMIN_WITHDRAW_TCK	0x4605			///> (UI->MAIN) 관리자 - 시재관리 - 승차권함 회수


//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

// queue data
typedef struct 
{
	WORD	wCommand;
	int		nFlag;				// 전송 Flag - send/recv(1), send(2)
#define FLAG_SEND_RECV		1
#define FLAG_SEND			2
#define FLAG_Q_SEND_RECV	4
	int		nLen;
	char	szData[1024 * 2];
} UI_QUE_DATA_T, *PUI_QUE_DATA_T;

// UI Header
typedef struct
{
	WORD	wCommand;
	WORD	wDataLen;
	BYTE	byBody;						
} UI_HEADER_T, *PUI_HEADER_T;

// UI Header
typedef struct
{
	WORD	wTrmlWndNo;			///< 터미널 창구번호
	char	szTrmlCd[7];		///< 터미널 코드
} UI_SEND_POLL_T, *PUI_SEND_POLL_T;

// UI Header
typedef struct
{
	BYTE	szCommand[2];
	WORD	wDataLen;
	BYTE	byBody;
} UI_SEND_ACK_T, *PUI_SEND_ACK_T;

// 거래 취소/완료 처리
typedef struct
{
	BYTE	byTrans;		///< 0x01:예매발권, 0x02:현장발권, 0x03:환불
	BYTE	byService;
} UI_RECV_OKCANCEL_T, *PUI_RECV_OKCANCEL_T;

// 사용중/사용중지 처리
typedef struct
{
	BYTE	byService;
} UI_RECV_SERVICE_T, *PUI_RECV_SERVICE_T;

// 사용중/사용중지 처리
typedef struct
{
	BYTE	byResult;				///> 결과
	BYTE	szMainVer[20];			///> Main F/W version
	BYTE	szUiVer[20];			///> Ui F/W version
	BYTE	szEtcVer[20];			///> ETC F/W version

} UI_SND_VERSION_T, *PUI_SND_VERSION_T;

// 알람/이벤트 전송 처리(0103)
typedef struct
{
	BYTE	byFlag;				///> 0x00:이벤트, 0x01:알람
	BYTE	byDevice;			///> 장비구분
	BYTE	byHappen;			///> 발생:0x01, 해제:0x02
	BYTE	szCode[4];			///> 장애코드
	BYTE	szContents[100];	///> 장애내용
} UI_ALARM_INFO_T, *PUI_ALARM_INFO_T;

typedef struct 
{
	int			n10;				///> 10원
	int			n50;				///> 50원
	int			n100;				///> 100원
	int			n500;				///> 500원
} UI_COIN_T, *PUI_COIN_T;

typedef struct 
{
	int			n1k;				///> 1,000원
	int			n5k;				///> 5,000원
	int			n10k;				///> 10,000원
	int			n50k;				///> 50,000원
} UI_BILL_T, *PUI_BILL_T;

// 무인발매기 회계정보 전송 처리(0104)
typedef struct
{
	DM_COIN_T	Coin;				///> 동전방출기
	DM_BILL_DISPENSER_T	Dispenser;	///> 지폐방출기 카세트함
	DM_BILL_T	BillBox;			///> 지폐함 불량지폐함

	int nIssueCount;				///> 승차권 발권 매수
	int nTicketRemain;				///> 승차권 잔량

} UI_DEV_ACCOUNT_INFO_T, *PUI_DEV_ACCOUNT_INFO_T;


// 장비환경설정 - 기초정보 (106)
typedef struct
{
	BYTE	byType;					///> '1'(자립형), '2'(매립형) 
//	BYTE	byLcdCount;				///> LCD 갯수
	BYTE	byBusSep;				///> 버스구분 (시외:'1',고속:'2',시외/고속:'3')

	BYTE	byMrnpYN;				///> 예매발권유무 : 사용(Y), 미사용(N)
	BYTE	byMrnpAllVwYN;			///> 예매내역전체보기 : 사용(Y), 미사용(N)
	BYTE	byMrnpManualYN;			///> 예매수기조회 : 사용(Y), 미사용(N)
	BYTE	byMrnpAutoIssYN;		///> 당일예매1건 자동발매 : 사용(Y), 미사용(N)
	BYTE	byMrnpDetailVwYN;		///> 예매1건 세부내역보기 : 사용(Y), 미사용(N)

	BYTE	byPubTckYN;				///> 현장발권유무 : 사용(Y), 미사용(N)
	BYTE	byQuickAlcnYN;			///> 빠른배차사용유무 : 사용(Y), 미사용(N)

	BYTE	byRefundYN;				///> 환불유무 : 사용(Y), 미사용(N)

	BYTE	byAlcn;					///> 비배차 : 'N', 배차 : 'D'
	BYTE	byPayMethod;			///> 결제방식 (카드:'1',현금:'2',카드+현금:'3')

	BYTE	byRegCashReceiptYN;		///> 현금영수증 등록 여부, 사용:'Y', 미사용:'N'
	BYTE	bySignYN;				///> 5만원미만 카드 무서명, 무서명:'Y', 서명:'N'
	BYTE	byCardPasswdYN;			///> 카드비밀번호, 사용:'Y', 미사용:'N'
	BYTE	byMaintMonitorYN;		///> 관리자모니터 사용유무 : 사용:'Y', 미사용:'N'
	BYTE	byAutoClsYN;			///> 자동마감 유무 : 사용(Y), 미사용(N)

	int		nTicketMinCount;		///> 승차권 잔여 최소 수량
	int		n10k_MinCount;			///> 1만원권 잔여 최소 수량
	int		n1k_MinCount;			///> 1천원권 잔여 최소 수량
	int		n100_MinCount;			///> 100원 잔여 최소 수량
	int		n500_MinCount;			///> 500원 잔여 최소 수량

	int		nIssCount;				///> 1회 발권 제한 수량
	int		nIssMoney;				///> 1회 발권 제한 금액
	int		nIssTime;				///> 1회 발권 제한 시간(분)

	char	szClsTime[6];			///> 자동마감 시간(hhmmss)

	int		nScreenWaitTime;		///> 화면 대기 시간(초)
	int		nAlarmWaitTime;			///> 알림창 대기시간(초)
	int		nQuickAlcnTime;			///> 빠른배차 조회 최소시간(분)

	char	szPrtKind[10];			///> 인쇄종류 (DF1, DF2, DF3...)

} UI_BASE_INFO_T, *PUI_BASE_INFO_T;

// 장비환경설정 - 기초정보 (0107)
typedef struct
{
	char	trml_wnd_no		[4];	///> 창구번호
	char	trml_cd			[7];	///> 터미널코드(7바이트)
	char	shct_trml_cd	[4];	///> 단축 터미널코드
	char	user_no			[20];	///> user no
	char	user_pwd		[20];	///> user pwd
	char	trml_nm			[100];	///> 터미널 명
	char	trml_rprn_tel_no[100];	///> 터미널 전화번호
} UI_TRML_INFO_T, *PUI_TRML_INFO_T;

// 기초 정보 전송(0108)
typedef struct
{
	WORD	wFileID;
	char	szFullName[128];
	DWORD	dwSize;
} UI_BASE_FILE_INFO_T, *PUI_BASE_FILE_INFO_T;

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///> 예매발권 - 예매번호 or 생년월일
typedef struct
{
	BYTE	byFunc;				///> 0x01:예매번호, 0x02:생년월일
	char	szMrsNo[17];		///> 예매번호
	char	szBirthDay[6];		///> 생년월일
	char	szMobileNo[50];		///> 핸드폰 번호
} UI_RESP_MRS_INPUT_T, *PUI_RESP_MRS_INPUT_T;

///> 예매발권 - 예매내역 

typedef struct
{
	BYTE	byFunc;				///> 0x01:신용카드, 0x02:생년월일, 0x03:예매번호
	char	szMrsNo[17];		///> 예매번호
	char	szBirthDay[6];		///> 생년월일
	char	szMobileNo[50];		///> 핸드폰 번호

	char	pos_pg_ktc_ver[20];
	int		n_enc_dta_len;
	char	enc_dta[1024];
} UI_RESP_MRS_INFO_T, *PUI_RESP_MRS_INFO_T;

// ui203 송신 전문
typedef struct
{
	char mrs_no				[17];	///< 예매번호				
	char depr_dt			[8];	///< 출발일자				
	char depr_time			[6];	///< 출발시각				
	char depr_trml_cd		[7];	///< 출발터미널코드			
	char arvl_trml_cd		[7];	///< 도착터미널코드			
	char tisu_id			[17];	///< 발권id					
	char tisu_dt			[8];	///< 발권일자				
	char tisu_time			[6];	///< 발권시각				

	int  n_take_drtm		   ;	///< 소요시간
	char bus_cacm_cd		[4];	///< 버스운수사코드
	char bus_cls_cd			[3];	///< 버스등급코드			
	char bus_tck_knd_cd		[4];	///< 버스티켓종류코드		
	int  n_tisu_amt			   ;	///< 승차요금				
	WORD w_sats_no			   ;	///< 좌석번호

	char pub_chnl_dvs_cd	[1];	///< 발행채널구분코드		 *
	char cty_bus_sys_dvs_cd	[1];	///< 시외버스시스템구분코드	
	//char tisu_chnl_dvs_cd	[1];	///< 발권채널구분코드		
	char tisu_sys_dvs_cd	[1];	///< 발권시스템구분코드

} UI_SND_MRS_LIST_T, *PUI_SND_MRS_LIST_T;

typedef struct
{
	BYTE	byAck;				///> 0x01:예매번호, 0x02:생년월일
	WORD	wCount;
	PUI_SND_MRS_LIST_T pList;
} UI_SND_MRS_INFO_T, *PUI_SND_MRS_INFO_T;

///> 예매발권 - 발권 (UI-205)

typedef struct
{
	char mrs_no				[17];	///< 예매번호				
	char depr_dt			[8];	///< 출발일자				
	char depr_time			[6];	///< 출발시각				
	char depr_trml_cd		[7];	///< 출발터미널코드			
	char arvl_trml_cd		[7];	///< 도착터미널코드	

	char tisu_id			[17];	///< 발권id					
	char tisu_dt			[8];	///< 발권일자				
	char tisu_time			[6];	///< 발권시각				

	int  n_take_drtm		   ;	///< 소요시간
	char bus_cacm_cd		[4];	///< 버스운수사코드
	char bus_cls_cd			[3];	///< 버스등급코드			
	char bus_tck_knd_cd		[4];	///< 버스티켓종류코드		
	int  n_tisu_amt			   ;	///< 승차요금				
	WORD w_sats_no			   ;	///< 좌석번호

	char pub_chnl_dvs_cd	[1];	///< 발행채널구분코드		 *
	char cty_bus_sys_dvs_cd	[1];	///< 시외버스시스템구분코드	
	//char tisu_chnl_dvs_cd	[1];	///< 발권채널구분코드		
	char tisu_sys_dvs_cd	[1];	///< 발권시스템구분코드

} UI_RESP_MRS_ISSUE_LIST_T, *PUI_RESP_MRS_ISSUE_LIST_T;

typedef struct
{
	char depr_trml_nm		[100];	///< 출발터미널코드(한글)
	char arvl_trml_nm		[100];	///< 도착터미널코드(한글)
	char depr_trml_eng_nm	[100];	///< 출발터미널코드(영문)
	char arvl_trml_eng_nm	[100];	///< 도착터미널코드(영문)

	WORD	wCount;
								
	BYTE	ch;						/// PUI_RESP_MRS_ISSUE_LIST_T

} UI_RESP_MRS_ISSUE_T, *PUI_RESP_MRS_ISSUE_T; 

///> 예매발권 - 발권상태 (UI-206)
typedef struct
{
	BYTE	byACK;
	BYTE	byState;					///> 발권 상태 (0x01:시작, 0x02:완료)
	WORD	w_sats_no;					///> 좌석번호
} UI_SND_MRS_ISSUE_STATE_T, *PUI_SND_MRS_ISSUE_STATE_T; 

///> 예매발권 - 발권상태
typedef struct
{
	BYTE	byACK;						///> 발권결과 
	BYTE	byState;					///> 발권 상태 (0x01:시작, 0x02:완료)
	BYTE	bySeat;						///> 좌석번호
} UI_SND_PBTCK_ISSUE_STATE_T, *PUI_SND_PBTCK_ISSUE_STATE_T; 

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///> 환불 - 승차권 읽기 결과 전송 (402)
typedef struct
{
	char byAck;						///> 결과
	char pyn_mns_dvs_cd		[1];	///> 지불수단구분코드	
	int n_pym_amt			   ;	///> 결제금액		
	int n_canc_ry_cmrt		   ;	///> 취소수수료		
	int n_canc_ry_amt	 	   ;	///> 취소환불금액		
} UI_SND_CANCRY_TCK_INFO_T, *PUI_SND_CANCRY_TCK_INFO_T; 

///> 환불 - 환불 금액 처리
typedef struct
{
	char ui_pym_dvs_cd		[1]	;	///> 결제 금액
	int	nFare					;	///> 결제 금액
	int	nOutFare				;	///> 방출 금액
} UI_RESP_CANCRY_FARE_T, *PUI_RESP_CANCRY_FARE_T; 

///> 환불 - 카드 또는 현금 방출 정보 전송
typedef struct
{
	BYTE byAck;						///> 
	char pyn_mns_dvs_cd[1];			///> 지불수단코드
	WORD w1k;						///> 지폐 - 1,000원
	WORD w5k;						///> 지폐 - 5,000원
	WORD w10k;						///> 지폐 - 10,000원
	WORD w50k;						///> 지폐 - 50,000원
	WORD w10;						///> 동전 - 10원
	WORD w50;						///> 동전 - 50원
	WORD w100;						///> 동전 - 100원
	WORD w500;						///> 동전 - 500원

} UI_SND_CANCRY_CHG_MONEY_T, *PUI_SND_CANCRY_CHG_MONEY_T; 


///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/// [현장발권] 배차 조회

/// (0x302)
typedef struct
{
	char depr_trml_cd		[7];	///< 출발터미널코드			
	char arvl_trml_cd		[7];	///< 도착터미널코드			
	char read_dt			[8];	///< 요청 조회 대상일		
	char read_time			[6];	///< 요청 조회 대상 시각		
	char bus_cls_cd			[3];	///< 버스등급코드			
	int  n_req_rec_num		   ;	///< 요청 레코드 수
} UI_RESP_PBTCK_REQ_LIST_T, *PUI_RESP_PBTCK_REQ_LIST_T;

typedef struct
{
	char rot_id				[17];	///< 노선ID
	int  n_rot_sqno			;		///< 노선순번(number)
	char alcn_dt			[8];	///< 배차일자
	int  n_alcn_sqno		;		///< 배차순번(number)
	char atl_depr_dt		[8];	///< 실제출발일자
	char atl_depr_time		[6];	///< 실제출발시각
	char depr_time			[6];	///< 출발시각
	char rot_nm				[100];	///< 노선명 
	char rot_bus_dvs_cd		[1];	///< 노선종류코드
	char bus_cls_cd			[3];	///< 버스등급코드
	char bus_cacm_cd		[4];	///< 버스운수사코드
	char alcn_way_dvs_cd	[1];	///< 배차방식구분코드
	char sati_use_yn		[1];	///< 좌석제사용여부
	char perd_temp_yn		[1];	///< 정기임시여부
	char bcnl_yn			[1];	///< 결행여부
	int  n_dist				;		///< 거리(number)
	int  n_take_drtm		;		///< 소요시간(number)
	char rdhm_mltp_val		[100];	///< 승차홈다중값
	int  n_sats_num			;		///< 좌석 수(number)
	int  n_rmn_scnt			;		///< 잔여좌석수(number)
	int  n_ocnt				;		///< 무표수(number)
	char trml_tisu_psb_yn	[1];	///< 터미널발권가능여부
	char cty_bus_oprn_shp_cd[1];	///< 시외버스운행형태코드
	char dc_psb_yn			[1];	///< 할인가능유무
} UI_SND_PBTCK_REQ_LIST_T, *PUI_SND_PBTCK_REQ_LIST_T;


/// (0x303)
typedef struct
{
	char rot_id				[17];	///< 노선ID	
	int  n_rot_sqno			;		///< 노선순번(number)
	char alcn_dt			[8];	///< 배차일자	
	int  n_alcn_sqno		;		///< 배차순번(number)	
	char depr_trml_cd		[7];	///< 출발터미널코드
	char arvl_trml_cd		[7];	///< 도착터미널코드

	char depr_trml_nm		[100];	///< 출발터미널 이름(한글)
	char arvl_trml_nm		[100];	///< 도착터미널 이름(한글)
	char depr_trml_eng_nm	[100];	///< 출발터미널 이름(영문)
	char arvl_trml_eng_nm	[100];	///< 도착터미널 이름(영문)

} UI_RESP_PBTCK_SELECT_T, *PUI_RESP_PBTCK_SELECT_T;


// (0305) 현장발권 - 좌석 & 승차권 종류별 선택 
typedef struct
{
	char bus_tck_knd_cd	[4];	///< 버스티켓종류코드
	char dcrt_dvs_cd	[1];	///< 할인율구분코드
	char sats_no		[2];	///< 좌석번호
	int  n_tisu_amt		;		///< 발권금액
	
} UI_PCPYSATS_LIST_T, *PUI_PCPYSATS_LIST_T;

typedef struct
{
	DWORD	dwTotalTisuAmt;
	char	rot_id		[17];	///< 노선ID
	int		n_rot_sqno		;	///< 노선순번
	char	alcn_dt		[8]	;	///< 배차일자
	int		n_alcn_sqno		;	///< 배차순번
	char	arvl_trml_cd[7]	;	///< 도착 터미널코드
	WORD	wCount;
	UI_PCPYSATS_LIST_T	List[50];
} UI_RESP_PCPYSATS_T, *PUI_RESP_PCPYSATS_T;


// (308) 현장발권 - 지폐투입정보 전달
typedef struct
{
	BYTE	byACK;
	WORD	w1k;
	WORD	w5k;
	WORD	w10k;
	WORD	w50k;
} UI_SND_BILL_INFO_T, *PUI_SND_BILL_INFO_T;

// (311) 현장발권 - 승차권 결제 요청
typedef struct
{
	int	 nTotalMoney		 ;			///< 총 결제금액
	char ui_pym_dvs_cd		[1];		/// 결제 구분코드 1:현금결제, 2:카드결제
	char csrc_dvs_cd		[1];		/// 현금영수증 구분코드 - 0:미사용, 1:수기입력, 2:신용카드
	char csrc_no			[20];		///< 현금영수증 수기입력 번호
	char csrc_use			[1];		///< 0:개인, 1:법인
	char trd_dvs_cd			[1];		/// 거래구분코드			
	char fallback_dvs_cd	[1];		/// fallback구분코드	
	char pos_pg_ktc_ver		[20];		///< POS 단말기 버젼
	char enc_dta_len		[4];		/// enc 데이터 길이
	char enc_dta			[1024];		/// enc 데이터
	char emv_dta			[1024];		/// emv 데이터
	char spad_pwd_yn		[1];		/// 서명비밀번호여부 - 1:싸인, 2:비밀번호, 3:싸인+비밀번호
	char card_pwd			[10];		/// 카드비밀번호			
	char spad_dta			[3072];		/// 싸인패드데이터		
	char spad_dta_len		[4];		/// 싸인패드데이터길이		
} UI_RESP_REQ_PAYMENT_T, *PUI_RESP_REQ_PAYMENT_T;

// (312) 현장발권 - 현금영수증 번호 수기입력 정보 전달
typedef struct
{
	char csrc_dvd_cd		[1];		///< 현금영수증 구분코드, 0x01:수기입력, 0x02:현영카드
	char csrc_no			[20];		///< 현금영수증 수기입력 번호
	char csrc_use			[1];		///< 0:개인, 1:법인
	char pos_pg_ktc_ver		[20];		///< POS 단말기 버젼
	int  n_enc_dta_len		;			///< enc_dta_len
	char enc_dta			[1024];		///< enc_dta

} UI_RESP_CSRC_INPUT_T, *PUI_RESP_CSRC_INPUT_T;

// (315) 현장발권 - 거스름돈 방출
typedef struct
{
	int  nTotalMoney		;			///< 결제금액
	int  nChangeMoney		;			///< 방출금액(=거스름돈)
} UI_RESP_CHG_MONEY_T, *PUI_RESP_CHG_MONEY_T;

// (317) 현장발권 - 신용카드 정보 전달
typedef struct
{
	char trd_dvs_cd			[1];		/// 거래구분코드			
	char fallback_dvs_cd	[1];       /// fallback구분코드	
	char pos_pg_ktc_ver		[20];      /// pos단말기버전정보		
	char enc_dta_len		[4];       /// enc 데이터 길이		
	char enc_dta			[1024];    /// enc 데이터			
	char emv_dta			[1024];    /// emv 데이터			
	char spad_pwd_yn		[1];       /// 서명비밀번호여부		
	char card_pwd			[10];      /// 카드비밀번호			
	char spad_dta			[3072];    /// 싸인패드데이터		
	char spad_dta_len		[4];       /// 싸인패드데이터길이		
} UI_RESP_CREDIT_CARD_INFO_T, *PUI_RESP_CREDIT_CARD_INFO_T;


///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///> 공통

typedef struct
{
	BYTE	byAck;				///> 
	char	szErrCode[10];		///> 에러코드
} UI_SND_FAIL_INFO_T, *PUI_SND_FAIL_INFO_T; 


///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// [관리자] 로그인
typedef struct
{
	BYTE	byLogin;			///> 0x01:로그인, 0x02:로그아웃
	char	szID[20];			///> 사용자 ID
	char	szPwd[20];			///> 비밀번호
} UI_RESP_ADMIN_LOGIN_T, *PUI_RESP_ADMIN_LOGIN_T;

typedef struct
{
	BYTE	byAck;				///> 
	BYTE	byLogin;			///> 0x01:로그인, 0x02:로그아웃
} UI_SND_ADMIN_LOGIN_T, *PUI_SND_ADMIN_LOGIN_T;

// [관리자] 설정관리 - 기능설정(502)
typedef struct
{
	BYTE	byMrnpYN;				///> 예매발권유무 : 사용(Y), 미사용(N)
	BYTE	byMrnpAllVwYN;			///> 예매내역전체보기 : 사용(Y), 미사용(N)
	BYTE	byMrnpManualYN;			///> 예매수기조회 : 사용(Y), 미사용(N)
	BYTE	byMrnpAutoIssYN;		///> 당일예매1건 자동발매 : 사용(Y), 미사용(N)
	BYTE	byMrnpDetailVwYN;		///> 예매1건 세부내역보기 : 사용(Y), 미사용(N)

	BYTE	byPubTckYN;				///> 현장발권유무 : 사용(Y), 미사용(N)
	BYTE	byQuickAlcnYN;			///> 빠른배차사용유무 : 사용(Y), 미사용(N)

	BYTE	byRefundYN;				///> 환불유무 : 사용(Y), 미사용(N)

	BYTE	byAlcn;					///> 비배차 : 'N', 배차 : 'D'
	BYTE	byPayMethod;			///> 결제방식 (카드:'1',현금:'2',카드+현금:'3')

	BYTE	byRegCashReceiptYN;		///> 현금영수증 등록 여부, 사용:'Y', 미사용:'N'
	BYTE	bySignYN;				///> 5만원미만 카드 무서명, 무서명:'Y', 서명:'N'
	BYTE	byCardPasswdYN;			///> 카드비밀번호, 사용:'Y', 미사용:'N'
	BYTE	byMaintMonitorYN;		///> 관리자모니터 사용유무 : 사용:'Y', 미사용:'N'
	BYTE	byAutoClsYN;			///> 자동마감 유무 : 사용(Y), 미사용(N)

} UI_RESP_ADMIN_FUNC_T, *PUI_RESP_ADMIN_FUNC_T;

// [관리자] 설정관리 - 환경설정:승차권관리, 돈관리 (503)
typedef struct
{
	int	nTicketMinCount;		///> 승차권 잔여 최소 수량
	int	n10k_MinCount;			///> 1만원권 잔여 최소 수량
	int	n1k_MinCount;			///> 1천원권 잔여 최소 수량
	int	n500_MinCount;			///> 500원 잔여 최소 수량
	int	n100_MinCount;			///> 100원 잔여 최소 수량
	int	nIssCount;				///> 1회 발권 제한 수량
	int	nIssMoney;				///> 1회 발권 제한 금액
	int	nIssTime;				///> 1회 발권 제한 시간(분)
	char szAutoCls[6];			///> 자동마감 시간(hhmmss)
	int	nScreenWaitTime;		///> 화면 대기 시간(초)
	int	nAlarmWaitTime;			///> 알림창 대기시간(초)
	int nQuickAlcnTime;			///> 빠른 배차조회 최소 시간(분)
	char szPrtFmt[10];			///> DF1, DF2, ...
} UI_RESP_ADMIN_ENV_T, *PUI_RESP_ADMIN_ENV_T;

// [관리자] 설정관리 - 터미널 설정 (504)
typedef struct
{
	char	szThrmlCode[7];			///> 터미널 코드
	WORD	wSeqNo;					///> 순번
	BYTE	byUse;					///> 사용유무, 0:사용, 1:미사용
} UI_RESP_THRML_LIST_T, *PUI_RESP_THRML_LIST_T;

typedef struct
{
	WORD	wCount;					///> 갯수
	char	byList;
} UI_RESP_ADMIN_THRML_T, *PUI_RESP_ADMIN_THRML_T;

// [관리자] 설정관리 - 승차권 설정 (505)
typedef struct
{
	char	szCode[4];				///> 버스티켓종류 코드
	BYTE	byUse;					///> 사용유무, 0:미사용, 1:사용
} UI_RESP_TICKET_LIST_T, *PUI_RESP_TICKET_LIST_T;

typedef struct
{
	WORD	wCount;					///> 갯수
	char	byList;
} UI_RESP_ADMIN_TICKET_T, *PUI_RESP_ADMIN_TICKET_T;

// [관리자] 설정관리 - 운수사 설정 (506)
typedef struct
{
	char	szCode[4];				///> 운수사 코드
	WORD	wSeq;					///> 운수사 순번
} UI_RESP_BUS_CACM_LIST_T, *PUI_RESP_BUS_CACM_LIST_T;

typedef struct
{
	WORD	wCount;					///> 갯수
	char	byList;
} UI_RESP_ADMIN_BUS_CACM_T, *PUI_RESP_ADMIN_BUS_CACM_T;



// [관리자] 티켓관리 - 티켓고유번호 설정 (521)
typedef struct
{
// 	WORD	wTicketCount;			///> 승차권 수량
	char	bus_tck_inhr_no[8];		///> 버스티켓고유번호
} UI_RESP_ADMIN_TCK_INHR_T, *PUI_RESP_ADMIN_TCK_INHR_T;

// [관리자] 티켓관리 - 승차권 보급 (522)
typedef struct
{
	int nTicketCount;			///> 승차권 수량
} UI_RESP_ADMIN_TCK_COUNT_T, *PUI_RESP_ADMIN_TCK_COUNT_T;

// [관리자] 티켓관리 - 승차권 정보 전송 (524)
typedef struct
{
	int nTicketCount;			///> 승차권 수량
	char	bus_tck_inhr_no[8];		///> 버스티켓고유번호
} UI_SND_ADMIN_TCK_INFO_T, *PUI_SND_ADMIN_TCK_INFO_T;


// [관리자] 발권 내역 관리 (531)

typedef struct
{
	BYTE	byAck;					///> 결과
	char	szFileName[128];		///> 파일이름
	int		nSize;					///> 파일 사이즈
} UI_SND_ADMIN_INQ_ISSUE_T, *PUI_SND_ADMIN_INQ_ISSUE_T;

// [관리자] 재발행 (532)
typedef struct
{
	char	pub_dt			[8];	///> 발행일자
	char	pub_shct_trml_cd[4];	///> 발행단축터미널코드
	char	pub_wnd_no		[2];	///> 발행창구번호
	char	pub_sno			[4];	///> 발행일련번호
} UI_RESP_ADMIN_RE_ISSUE_T, *PUI_RESP_ADMIN_RE_ISSUE_T;

// [관리자] 시재관리 - 시재 내역

typedef struct
{
	WORD	w100;		
	WORD	w500;

	WORD	w1k;
	WORD	w5k;
	WORD	w10k;
	WORD	w50k;
} UI_CASH_T, *PUI_CASH_T;

typedef struct
{
	BYTE	byACK;
	UI_CASH_T startInfo;			///> 초기 내역
	UI_CASH_T insInfo;				///> 입금 내역
	UI_CASH_T outInfo;				///> 출금 내역
	UI_CASH_T currInfo;				///> 현재 내역

} UI_SND_ADMIN_CASH_HISTO_T, *PUI_SND_ADMIN_CASH_HISTO_T;

// [관리자] 시재관리 - 시재 보급

typedef struct
{
	BYTE	byPymDvs;				///> 1:동전보급, 2:지폐보급	
	UI_CASH_T insInfo;				///> 보급

} UI_RESP_ADMIN_CASH_INS_T, *PUI_RESP_ADMIN_CASH_INS_T;

// [관리자] 시재관리 - 시재 출금

typedef struct
{
	UI_CASH_T cashInfo;				///> 출금

} UI_RESP_ADMIN_CASH_OUT_T, *PUI_RESP_ADMIN_CASH_OUT_T;

typedef struct
{
	BYTE	byACK;
	UI_CASH_T cashInfo;				///> 출금

} UI_SND_ADMIN_CASH_OUT_T, *PUI_SND_ADMIN_CASH_OUT_T;

// [관리자] 시재관리 - 시재 입출내역 (544)
typedef struct
{
	BYTE	byACK;
	char	szFileName[128];
	DWORD	dwSize;

} UI_SND_ADMIN_INQ_CASH_INOUT_T, *PUI_SND_ADMIN_INQ_CASH_INOUT_T;


#pragma pack()

//----------------------------------------------------------------------------------------------------------------------

class CUiSocket : public CMySocket
{
public:
	CUiSocket(void) {};
	virtual ~CUiSocket(void) {};

public:
};

//----------------------------------------------------------------------------------------------------------------------

class CUiLogFile : public CMyLogFile 
{
public:
	CUiLogFile(void) {};
	virtual ~CUiLogFile(void) {};
};

//----------------------------------------------------------------------------------------------------------------------

int UI_Initialize(void);
int UI_Terminate(void);

int UI_AddQueueInfo(WORD wCommand, char *pData, int nDataLen);

int UI_AddServiceInfo(BOOL bService);
int UI_AddAlarmInfo(char *pData);
int UI_AddDevAccountInfo(void);
int UI_AddDevConfigData(void);
int UI_AddBaseFileInfo(char *pData);
int UI_AddTrmlInfo(void);
int UI_AddFileInfo(int nNumber, int nID);


int UI_MrsCardReadInfo(char *pData, int nDataLen);
int UI_MrsTckIssueInfo(char *pData, int nDataLen);

int UI_CancRyChangeMoneyInfo(char *pData, int nDataLen);

int UI_AddFailInfo(WORD wCommand, char *pData);
int UI_MrsIssueState(BYTE byACK, BYTE byState, char *pTisuID);

int UI_PbTckIssueState(int nState, int n_sats_no);
int UI_TkPubChangeMoneyInfo(char *pData, int nDataLen);

// 배차요금 조회 (0x0304)
int UI_TkPubAlcnFareInfo(BOOL bResult, int nError);

// 좌석선점 정보 (0x0306)
int UI_TkPubSatsInfo(BOOL bResult, int nError);

// TMAX 승차권 발권 결과 정보 (0x0318)
int UI_TkPubTmaxResultInfo(BOOL bResult, int nError);

// 환불정보 전달
int UI_AddRefundInfo(BOOL bResult);
