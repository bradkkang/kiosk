// 
// 
// dev_ui_main.h : UI main 헤더 파일
//

#pragma once

#include "MySocket.h"
#include "MyLogFile.h"
#include "MyDataAccum.h"
#include "svr_kobus_st.h"
#include "svr_tm_expbus_st.h"

//----------------------------------------------------------------------------------------------------------------------

enum __enUIState__
{
	UI_NONE_STATE		= 0	,
	UI_CONNECT_STATE		,
	UI_DATA_STATE			,
};

/// 시외/코버스/티머니고속 구분
#define UI_BUS_DVS_CCS				'1'
#define UI_BUS_DVS_KOBUS			'2'
#define UI_BUS_DVS_TMEXP			'3'

//----------------------------------------------------------------------------------------------------------------------

#define UI_CMD_POLLING						0x0001		///< Polling (MAIN -> UI)
#define UI_CMD_OK_CANCEL					0x0101		///< 거래 취소/완료 처리 (UI -> MAIN)
#define UI_CMD_SERVICE						0x0201		///< 사용중/사용중지 처리 (UI -> MAIN, MAIN -> UI)
#define UI_CMD_ALARM						0x0301		///< 알람/이벤트 전송 처리 (MAIN -> UI)
#define UI_CMD_ACCOUNT						0x0401		///< 회계 정보 전송 처리 (MAIN -> UI)
#define UI_CMD_VERSION_REQ					0x0501		///< 버젼 정보 (UI ->MAIN, MAIN -> UI)
#define UI_CMD_DEV_CONFIG					0x0601		///< 장비환경설정 정보 (MAIN -> UI)
#define UI_CMD_TRML_INFO					0x0701		///< 터미널 정보 (MAIN -> UI)
#define UI_CMD_DEV_FILE_INFO				0x0801		///< File 정보 (MAIN -> UI)
#define UI_CMD_TCK_INHR_NO_INFO				0x0901		///< 버스티켓 고유번호 (MAIN -> UI)

#define UI_CMD_ALARM_REQ_INFO				0x2101		///< 장애 정보 요청 (UI -> MAIN)

/// 예매발권
#define UI_CMD_MRS_MAIN						0x0002		///< (UI->MAIN) 예매발권 - 시작
#define UI_CMD_MRS_CARD_RD					0x0102		///< (UI->MAIN) 예매발권 - 신용카드 읽기
#define UI_CMD_MRS_CARD_INFO				0x0202		///< (MAIN->UI) 예매발권 - 신용카드 정보 전달
//#define UI_CMD_MRS_INPUT					0x0302		///< (UI->MAIN) 예매발권 - 예매번호 입력 
#define UI_CMD_MRS_REQ_LIST					0x0302		///< (UI->MAIN) 예매발권 - 예매 리스트 요청 
#define UI_CMD_MRS_RESP_LIST				0x0402		///< (MAIN->UI) 예매발권 - 예매 리스트 전달
#define UI_CMD_MRS_ISSUE					0x0502		///< (UI->MAIN) 예매발권 - 예매 발권 시작
#define UI_CMD_MRS_CHG_ISSUE				0x0602		///< (UI->MAIN) 예매발권 - 교체발권
#define UI_CMD_MRS_ISSUE_STATE				0x0702		///< (MAIN->UI) 예매발권 - 예매 발권 진행상태

///< 코버스 전문

#define UI_CMD_KOBUS_MRS_REQ_LIST			0x3302		///< (UI->MAIN) [코버스] 예매발권 - 예매 리스트 요청 
#define UI_CMD_KOBUS_MRS_RESP_LIST			0x3402		///< (MAIN->UI) [코버스] 예매발권 - 예매 리스트 응답 
#define UI_CMD_KOBUS_MRS_ISSUE				0x3502		///< (UI->MAIN) [코버스] 예매발권 - 예매 리스트 선택(발권시작 요청)
#define UI_CMD_KOBUS_MRS_RESP_ISSUE			0x3602		///< (MAIN->UI) [코버스] 예매발권 - 예매 발권 완료
#define UI_CMD_KOBUS_MRS_TCK_PRT			0x3702		///< (UI->MAIN) [코버스] 예매발권 - 프린트 예매 발권 시작

///< 티머니고속_예매 전문

#define UI_CMD_TMEXP_MRS_REQ_LIST			0x5002		///< (UI->MAIN) [티머니고속] 예매발권 - 예매 리스트 요청 
#define UI_CMD_TMEXP_MRS_RESP_LIST			0x5102		///< (MAIN->UI) [티머니고속] 예매발권 - 예매 리스트 응답 
#define UI_CMD_TMEXP_MRS_KTC_REQ_LIST		0x5202		///< (UI->MAIN) [티머니고속] 예매발권 - (KTC) 예매 리스트 요청 
#define UI_CMD_TMEXP_MRS_KTC_RESP_LIST		0x5302		///< (MAIN->UI) [티머니고속] 예매발권 - (KTC) 예매 리스트 응답 
#define UI_CMD_TMEXP_MRS_REQ_ISSUE			0x5402		///< (UI->MAIN) [티머니고속] 예매발권 - 예매발권 요청
#define UI_CMD_TMEXP_MRS_RESP_ISSUE			0x5502		///< (MAIN->UI) [티머니고속] 예매발권 - 예매발권 응답

#define UI_CMD_TMEXP_MRS_REQ_MOBILE_ISSUE	0x5602		///< (UI->MAIN) [티머니고속] 예매발권 - 모바일 예매발권 요청
#define UI_CMD_TMEXP_MRS_RESP_MOBILE_ISSUE	0x5702		///< (MAIN->UI) [티머니고속] 예매발권 - 모바일 예매발권 응답

//#define UI_CMD_TMEXP_MRS_ISSUE			0x3502		///< (UI->MAIN) [티머니고속] 예매발권 - 예매 리스트 선택(발권시작 요청)
//#define UI_CMD_TMEXP_MRS_RESP_ISSUE		0x3602		///< (MAIN->UI) [티머니고속] 예매발권 - 예매 발권 완료
//#define UI_CMD_TMEXP_MRS_TCK_PRT			0x3702		///< (UI->MAIN) [티머니고속] 예매발권 - 프린트 예매 발권 시작


/// 현장발권
#define UI_CMD_PBTCK_MAIN					0x0003		///< (UI->MAIN) 현장발권 - 시작
#define UI_CMD_PBTCK_REQ_LIST				0x0103		///< (UI->MAIN) 현장발권 - 배차 리스트 요청
#define UI_CMD_PBTCK_RESP_LIST				0x0203		///< (MAIN->UI) 현장발권 - 배차 리스트 전달
#define UI_CMD_PBTCK_LIST_SEL				0x0303		///< (UI->MAIN) 현장발권 - 배차 리스트 선택
#define UI_CMD_PBTCK_FARE_INFO				0x0403		///< (MAIN->UI) 현장발권 - 좌석 정보 전달
#define UI_CMD_PBTCK_SEAT_SEL				0x0503		///< (UI->MAIN) 현장발권 - 좌석 선택
#define UI_CMD_PBTCK_SEAT_INFO				0x0603		///< (MAIN->UI) 현장발권 - 좌석 선택 정보 전달
#define UI_CMD_PBTCK_PYM_DVS				0x0703		///< (UI->MAIN) 현장발권 - 결제수단 선택
#define UI_CMD_PBTCK_INS_BILL				0x0803		///< (UI->MAIN) 현장발권 - 지폐 투입
#define UI_CMD_PBTCK_BILL_INFO				0x0903		///< (MAIN->UI) 현장발권 - 지폐 투입 정보
#define UI_CMD_PBTCK_REQ_PYM				0x1103		///< (UI->MAIN) 현장발권 - 승차권 결제 요청
#define UI_CMD_PBTCK_TMAX_RESULT			0x1203		///< (MAIN->UI) 현장발권 - TMAX 승차권 발권 결과
#define UI_CMD_PBTCK_TCK_ISSUE				0x1303		///< (UI->MAIN) 현장발권 - 승차권 발권 시작
#define UI_CMD_PBTCK_TCK_ISSUE_RET			0x1403		///< (MAIN->UI) 현장발권 - 승차권 발권 결과 전송
#define UI_CMD_PBTCK_CHG_MONEY				0x1503		///< (UI->MAIN) 현장발권 - 거스름돈 방출 시작
#define UI_CMD_PBTCK_CHG_MONEY_RET			0x1603		///< (MAIN->UI) 현장발권 - 거스름돈 방출 정보 전송
#define UI_CMD_PBTCK_TCK_ISSUE_COMP			0x1703		///< (MAIN->UI) 현장발권 - 승차권 발권 완료 - ktc 

#define UI_CMD_TICKET_QRMDPCPYSATS			0x7803		//   (UI->MAIN) 현장발권 - QR 좌석 선점 시간변경 (3분->10분 연장) (IF_UI_378) // 20221205 ADD

// 20190923 add by nhso : 상주직원 처리
#define UI_CMD_PBTCK_STAFF_REQ				0x2003		///< (UI->MAIN) 현장발권 - 상주직원 정보 요청
#define UI_CMD_PBTCK_STAFF_RESP				0x2103		///< (MAIN->UI) 현장발권 - 상주직원 정보 요청 응답

#define UI_CMD_PBTCK_STAFF_CD_MOD_FARE_REQ	0x2203		///< (UI->MAIN) 현장발권 - 상주직원_카드_좌석요금_정보_변경 요청
#define UI_CMD_PBTCK_STAFF_CD_MOD_FARE_RESP	0x2303		///< (MAIN->UI) 현장발권 - 상주직원_카드_좌석요금_정보_변경 응답

#define UI_CMD_PBTCK_PRE_RF_PAY_REQ			0x2403		///< (UI->MAIN) 현장발권 - 선불카드 결제 요청 => 현장발권 - 승차권 발행 요청 (선불) // 20230222 ADD
#define UI_CMD_PBTCK_PRE_RF_PAY_RESP		0x2503		///< (MAIN->UI) 현장발권 - 선불카드 결제 응답 => 현장발권 - 승차권 발행 응답 (선불) // 20230222 ADD

#define UI_CMD_PBTCK_USRINFINP				0x2603		///< (UI->MAIN) 현장발권 - 발행정보 입력


///< 코버스 전문

#define UI_CMD_REQ_PBTCK_LIST				0x3103		///< (UI->MAIN) [코버스] 현장발권 - 배차조회 요청
#define UI_CMD_RES_PBTCK_LIST				0x3203		///< (MAIN->UI) [코버스] 현장발권 - 배차조회 결과 

#define UI_CMD_REQ_PBTCK_SATS				0x3303		///< (UI->MAIN) [코버스] 현장발권 - 좌석정보 요청
#define UI_CMD_RES_PBTCK_SATS				0x3403		///< (MAIN->UI) [코버스] 현장발권 - 좌석정보 결과

#define UI_CMD_REQ_PBTCK_SATSPCPY			0x3503		///< (UI->MAIN) [코버스] 현장발권 - 좌석선점 요청
#define UI_CMD_RES_PBTCK_SATSPCPY			0x3603		///< (MAIN->UI) [코버스] 현장발권 - 좌석선점 결과

#define UI_CMD_REQ_PBTCK_TMAX_ISSUE			0x3703		///< (UI->MAIN) [코버스] 현장발권 - 카드/현금/현장발권 요청

#define UI_CMD_REQ_PBTCK_THRU_INFO			0x3803		///< (UI->MAIN) [코버스] 현장발권 - 경유지 정보 요청
#define UI_CMD_RES_PBTCK_THRU_INFO			0x3903		///< (MAIN->UI) [코버스] 현장발권 - 경유지 정보 결과

///< 티머니고속 전문
#define UI_CMD_REQ_TMEXP_PBTCK_LIST			0x5003		///< (UI->MAIN) [티머니고속] 현장발권 - 배차 정보 요청
#define UI_CMD_RES_TMEXP_PBTCK_LIST			0x5103		///< (MAIN->UI) [티머니고속] 현장발권 - 배차 정보 결과

#define UI_CMD_REQ_TMEXP_SATS_FEE			0x5203		///< (UI->MAIN) [티머니고속] 현장발권 - 요금/좌석정보 요청
#define UI_CMD_RES_TMEXP_SATS_FEE			0x5303		///< (MAIN->UI) [티머니고속] 현장발권 - 요금/좌석정보 수신

#define UI_CMD_REQ_TMEXP_PCPY_SATS			0x5403		///< (UI->MAIN) [티머니고속] 현장발권 - 좌석선점 요청
#define UI_CMD_RES_TMEXP_PCPY_SATS			0x5503		///< (MAIN->UI) [티머니고속] 현장발권 - 좌석선점 수신

#define UI_CMD_REQ_TMEXP_PBTCK_CASH			0x5603		///< (UI->MAIN) [티머니고속] 현장발권 - 승차권발권(현금)
#define UI_CMD_RES_TMEXP_PBTCK_CASH			0x5703		///< (MAIN->UI) [티머니고속] 현장발권 - 승차권발권(현금) 전달

#define UI_CMD_REQ_TMEXP_PBTCK_CARD_KTC		0x5803		///< (UI->MAIN) [티머니고속] 현장발권 - 승차권발권(카드_KTC)
#define UI_CMD_RES_TMEXP_PBTCK_CARD_KTC		0x5903		///< (MAIN->UI) [티머니고속] 현장발권 - 승차권발권(카드_KTC) 전달

#define UI_CMD_REQ_TMEXP_PBTCK_CSRC_KTC		0x6003		///< (UI->MAIN) [티머니고속] 현장발권 - 승차권발권(현금영수증_KTC_신용카드)
#define UI_CMD_RES_TMEXP_PBTCK_CSRC_KTC		0x6103		///< (MAIN->UI) [티머니고속] 현장발권 - 승차권발권(현금영수증_KTC_신용카드) 전달

#define UI_CMD_REQ_TMEXP_PBTCK_PRD			0x6203		///< (UI->MAIN) [티머니고속] 현장발권 - 승차권발권(부가상품권)
#define UI_CMD_RES_TMEXP_PBTCK_PRD			0x6303		///< (MAIN->UI) [티머니고속] 현장발권 - 승차권발권(부가상품권) 전달

#define UI_CMD_REQ_TMEXP_THRU_INFO			0x6403		///< (UI->MAIN) [티머니고속] 현장발권 - 경유지정보
#define UI_CMD_RES_TMEXP_THRU_INFO			0x6503		///< (MAIN->UI) [티머니고속] 현장발권 - 경유지정보 전달


/// 환불
#define UI_CMD_CANCRY_MAIN					0x0004			///< (UI->MAIN) 환불 - 메인
#define UI_CMD_CANCRY_TCK_READ				0x0104			///< (UI->MAIN) 환불 - 승차권 판독
#define UI_CMD_CANCRY_INFO					0x0204			///< (MAIN->UI) 환불 - 승차권 판독 결과 정보
#define UI_CMD_CANCRY_FARE					0x0304			///< (UI->MAIN) 환불 - 환불 처리 (카드 또는 현금)
#define UI_CMD_CANCRY_FARE_INFO				0x0404			///< (MAIN->UI) 환불 - 카드 또는 현금 방출 정보 전송

/// 관리자
#define UI_CMD_ADMIN_MAIN					0x0005			///< (UI->MAIN) 관리자 - 메인
#define UI_CMD_ADMIN_LOGIN					0x0105			///< (UI->MAIN) 관리자 - 로그인
#define UI_CMD_ADMIN_SET_FUNC				0x0205			///< (UI->MAIN) 관리자 - 설정관리 - 기능 설정
//#define UI_CMD_ADMIN_SET_ENV				0x0305			///< (UI->MAIN) 관리자 - 설정관리 - 환경 설정
#define UI_CMD_ADMIN_SET_THRML				0x0405			///< (UI->MAIN) 관리자 - 설정관리 - 터미널 설정
#define UI_CMD_ADMIN_SET_TICKET				0x0505			///< (UI->MAIN) 관리자 - 설정관리 - 승차권 설정
#define UI_CMD_ADMIN_SET_BUS_CACM			0x0605			///< (UI->MAIN) 관리자 - 설정관리 - 운수사 설정
#define UI_CMD_ADMIN_SET_EZ_OPT				0x0705			///< (UI->MAIN) 관리자 - 설정관리 - 이지옵션
#define UI_CMD_ADMIN_SET_TRML_OPT			0x0805			///< (UI->MAIN) 관리자 - 설정관리 - 환경설정 - 터미널 정보

#define UI_CMD_ADMIN_SYSTEM					0x1105			///< (UI->MAIN) 관리자 - 시스템관리
#define UI_CMD_ADMIN_DEV_RESET				0x1205			///< (UI->MAIN) 관리자 - 시스템관리 - 장비별 리셋

#define UI_CMD_ADMIN_MNG_TCK_INHRNO			0x2105			///< (UI->MAIN) 관리자 - 티켓고유번호 설정
#define UI_CMD_ADMIN_MNG_TCK_COUNT			0x2205			///< (UI->MAIN) 관리자 - 티켓신규수량
#define UI_CMD_ADMIN_REQ_TCK_INFO			0x2305			///< (UI->MAIN) 관리자 - 티켓정보 요청
#define UI_CMD_ADMIN_RSP_TCK_INFO			0x2405			///< (MAIN->UI) 관리자 - 티켓정보 전송

#define UI_CMD_ADMIN_INQ_ISSUE				0x3105			///< (UI->MAIN) 관리자 - 발행내역 조회
#define UI_CMD_ADMIN_INQ_REISSUE			0x3205			///< (UI->MAIN) 관리자 - 재발행내역 조회
#define UI_CMD_ADMIN_INQ_REFUND				0x3305			///< (UI->MAIN) 관리자 - 환불내역 조회
#define UI_CMD_ADMIN_RE_ISSUE				0x3405			///< (UI->MAIN) 관리자 - 재발행

#define UI_CMD_ADMIN_CASH_CLOSE				0x4005			///< (UI->MAIN) 관리자 - 시재관리 - 시재마감처리
#define UI_CMD_ADMIN_CASH_HISTO				0x4105			///< (UI->MAIN) 관리자 - 시재관리 - 시재내역
#define UI_CMD_ADMIN_CASH_INSERT			0x4205			///< (UI->MAIN) 관리자 - 시재관리 - 시재보급
#define UI_CMD_ADMIN_CASH_OUT				0x4305			///< (UI->MAIN) 관리자 - 시재관리 - 시재출금

#define UI_CMD_ADMIN_INQ_CASH_INOUT			0x4405			///< 사용안함 - (UI->MAIN) 관리자 - 시재관리 - 입출내역
#define UI_CMD_ADMIN_WITHDRAW_BILL			0x4505			///< (UI->MAIN) 관리자 - 시재관리 - 지폐함 회수
#define UI_CMD_ADMIN_WITHDRAW_TCK			0x4605			///< (UI->MAIN) 관리자 - 시재관리 - 승차권함 회수
#define UI_CMD_NOOUT_CASH_REQ				0x4705			///< (UI->MAIN) 관리자 - 시재관리 - 미방출금 방출요청
#define UI_CMD_NOOUT_CASH_RESULT			0x4805			///< (MAIN->UI) 관리자 - 시재관리 - 미방출금 방출 결과

#define UI_CMD_CLOSE_INQ_REQ				0x5105			///< (UI->MAIN) 관리자 - 마감내역 요청
#define UI_CMD_CLOSE_INQ_RESULT				0x5205			///< (MAIN->UI) 관리자 - 마감내역 정보 전송
#define UI_CMD_TR_INQ_REQ					0x5305			///< (UI->MAIN) 관리자 - 거래내역 정보 전송

#define UI_CMD_ACC_PRT_REQ					0x5405			///< (UI->MAIN) 관리자 - 시재정보 프린트 요청
#define UI_CMD_ACC_PRT_RESP					0x5505			///< (MAIN->UI) 관리자 - 시재정보 프린트 응답

#define UI_CMD_ADMIN_WND_CLOSE				0x9001			///< (MAIN->UI) 관리자 - 창구마감 처리 결과 정보 전송	// 20210513 ADD

#define UI_CMD_TEST_PRINT_REQ				0x9105			///< (UI->MAIN) 관리자 - 테스트 승차권 출력 요청 // 20211116 ADD
#define UI_CMD_TEST_PRINT_RESP				0x9205			///< (MAIN->UI) 관리자 - 테스트 승차권 출력 응답 // 20211116 ADD


//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

#define UI_Q_MAX_BUFF		(1024 * 2)

// queue data
typedef struct 
{
	WORD	wCommand;
	int		nFlag;				// 전송 Flag - send/recv(1), send(2)
#define FLAG_SEND_RECV		1
#define FLAG_SEND			2
#define FLAG_Q_SEND_RECV	4
	int		nLen;
	char	szData[UI_Q_MAX_BUFF + 1];
} UI_QUE_DATA_T, *PUI_QUE_DATA_T;

// UI Header
typedef struct
{
	WORD	wCommand;
	//WORD	wDataLen;
	DWORD	dwDataLen;
	BYTE	byBody;						
} UI_HEADER_T, *PUI_HEADER_T;

// UI Header
typedef struct
{
	WORD	wTrmlWndNo;			///< 터미널 창구번호
	char	szTrmlCd[7];		///< 터미널 코드
	char	chLang;				///< 언어
} UI_SEND_POLL_T, *PUI_SEND_POLL_T;

typedef struct
{
	BYTE	byACK;				///< 결과
	WORD	wTrmlWndNo;			///< 터미널 창구번호
	char	szTrmlCd[7];		///< 터미널 코드
	char	chLang;				///< 언어
} UI_RECV_POLL_T, *PUI_RECV_POLL_T;


// UI Header
typedef struct
{
	BYTE	szCommand[2];
	WORD	wDataLen;
	BYTE	byBody;
} UI_SEND_ACK_T, *PUI_SEND_ACK_T;

// Response ACK
typedef struct
{
	BYTE	byACK;
} UI_RESP_RESULT_T, *PUI_RESP_RESULT_T;

typedef struct
{
	BYTE	byACK;
	char	szErrCode[10];
	char	szErrContents[100];
} UI_SND_ERROR_T, *PUI_SND_ERROR_T;

typedef struct
{
	BYTE	byACK;
	char	szErrCode[10];
	char	szErrContents[100];
} UI_SND_ERROR_STR_T, *PUI_SND_ERROR_STR_T;

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
	BYTE	byResult;				///< 결과
	BYTE	szMainVer[20];			///< Main F/W version
	BYTE	szUiVer[20];			///< Ui F/W version
	BYTE	szKtcVer[20];			///< KTC version

} UI_SND_VERSION_T, *PUI_SND_VERSION_T,
  UI_RESP_VERSION_T, *PUI_RESP_VERSION_T;

// 알람/이벤트 전송 처리(0103)
typedef struct
{
	BYTE	byFlag;				///< 0x00:이벤트, 0x01:알람
	BYTE	byDevice;			///< 장비구분
	BYTE	byHappen;			///< 발생:0x01, 해제:0x02
	BYTE	szCode[4];			///< 장애코드
	BYTE	szContents[100];	///< 장애내용
} UI_ALARM_INFO_T, *PUI_ALARM_INFO_T;

typedef struct 
{
	int			n10;				///< 10원
	int			n50;				///< 50원
	int			n100;				///< 100원
	int			n500;				///< 500원
} UI_COIN_T, *PUI_COIN_T;

typedef struct 
{
	int			n1k;				///< 1,000원
	int			n5k;				///< 5,000원
	int			n10k;				///< 10,000원
	int			n50k;				///< 50,000원
} UI_BILL_T, *PUI_BILL_T;

// 무인발매기 회계정보 전송 처리(104)
typedef struct
{
	DM_COIN_T	Coin;				///< 동전방출기
	DM_BILL_DISPENSER_T	Dispenser;	///< 지폐방출기 카세트함
	DM_BILL_T	BillBox;			///< 지폐함 불량지폐함

	ACCOUNT_FIELD_T ccsCardPubTck;	///< [시외] 승차권 카드 발권 (매수/금액)
	ACCOUNT_FIELD_T ccsCardRefund;	///< [시외] 승차권 카드 환불 (매수/금액)

	ACCOUNT_FIELD_T ccsCashPubTck;	///< [시외] 승차권 현금 발권 (매수/금액)
	ACCOUNT_FIELD_T ccsCashRefund;	///< [시외] 승차권 현금 환불 (매수/금액)

	ACCOUNT_FIELD_T expCardPubTck;	///< [시외] 승차권 카드 발권 (매수/금액)
	ACCOUNT_FIELD_T expCardRefund;	///< [시외] 승차권 카드 환불 (매수/금액)

	ACCOUNT_FIELD_T expCashPubTck;	///< [시외] 승차권 현금 발권 (매수/금액)
	ACCOUNT_FIELD_T expCashRefund;	///< [시외] 승차권 현금 환불 (매수/금액)

	int n_tck_remain_count;			///< 승차권 잔량
	int n_tck_box_count;			///< 환불함 매수

	int n_ccs_inhr_no;				///< [시외] 승차권 고유번호
	int n_exp_inhr_no;				///< [고속] 승차권 고유번호

	// 누계
	DM_SHT_COIN_T	SumInsCoin;		///< [입금] 동전 투입 누계	
	DM_BILL_T		SumInsBill;		///< [입금] 지폐 투입 누계

	DM_SHT_COIN_T	SumOutCoin;		///< [출금] 동전 투입 누계	
	DM_BILL_T		SumOutBill;		///< [출금] 지폐 투입 누계

	DM_SHT_COIN_T	SumOut1Coin;	///< [방출] 동전 투입 누계	
	DM_BILL_T		SumOut1Bill;	///< [방출] 지폐 투입 누계

} UI_DEV_ACCOUNT_INFO_T, *PUI_DEV_ACCOUNT_INFO_T;


// 장비환경설정 - 기초정보 (106)
/***
typedef struct
{
	BYTE	device_dvs				;	///< (02). 카드전용('1'), 현금전용('2'), 카드+현금겸용('3')
	BYTE	ccs_svr_kind			;	///< (03). 시외버스 종류(없음:'0', 이지:'1')
	BYTE	exp_svr_kind			;	///< (04). 고속버스 종류(없음:'0', 이지:'1', 코버스:'2')
	BYTE	mrnp_func_yn			;	///< (05). 예매발권유무 : 사용(Y), 미사용(N)
	BYTE	mrnp_all_view_yn		;	///< (06). 예매내역전체보기 : 사용(Y), 미사용(N)
	BYTE	mrnp_manual_yn			;	///< (07). 예매수기조회 : 사용(Y), 미사용(N)
	BYTE	mrnp_auto_iss_yn		;	///< (08). 당일예매1건 자동발매 : 사용(Y), 미사용(N)
	BYTE	pubtck_yn				;	///< (09). 현장발권유무 : 사용(Y), 미사용(N)
	BYTE	alcn_kind				;	///< (10). 비배차 : 'N', 배차 : 'D'
	BYTE	quick_alcn_yn			;	///< (11). 빠른배차사용유무 : 사용(Y), 미사용(N)
	BYTE	favorate_yn				;	///< (12). 즐겨찾기 사용유무 : 사용(Y), 미사용(N)
	int		n_max_favor_trml_cnt	;	///< (40). 즐겨찾기터미널 최대갯수
	BYTE	refund_func_yn			;	///< (13). 환불유무 : 사용(Y), 미사용(N)
	BYTE	pay_method				;	///< (14). 결제방식 (카드:'1',현금:'2',카드+현금:'3')
	BYTE	reg_csrc_yn				;	///< (15). 현금영수증 등록 여부, 사용:'Y', 미사용:'N'
	BYTE	card_installment_yn		;	///< (16). (5만원이상) 카드할부 사용유무 : 사용(Y), 미사용(N)
	BYTE	sign_pad_yn				;	///< (17). 5만원미만 카드 무서명, 무서명:'Y', 서명:'N'
	BYTE	card_passwd_yn			;	///< (18). 카드비밀번호, 사용:'Y', 미사용:'N'
	BYTE	maint_monitor_yn		;	///< (19). 관리자모니터 사용유무 : 사용:'Y', 미사용:'N'
	BYTE	auto_close_yn			;	///< (20). 자동마감 유무 : 사용(Y), 미사용(N)
	char	auto_close_time[6]		;	///< (21). 자동마감 시간(hhmmss)
	BYTE	sms_yn					;	///< (22). 관제서버 사용유무 : 사용(Y), 미사용(N)
	BYTE	sms_ip[50]				;	///< (23). 관제서버 IP Address
	BYTE	camera_1_yn				;	///< (24). 카메라#1 사용유무 : 사용(Y), 미사용(N)
	BYTE	camera_2_yn				;	///< (25). 카메라#2 사용유무 : 사용(Y), 미사용(N)
	BYTE	camera_3_yn				;	///< (26). 카메라#3 사용유무 : 사용(Y), 미사용(N)
	int		n_ticket_min_count		;	///< (27). 승차권 잔여 최소 수량
	int		n_10k_min_count			;	///< (28). 1만원권 잔여 최소 수량
	int		n_1k_min_count			;	///< (29). 1천원권 잔여 최소 수량
	int		n_100_min_count			;	///< (30). 100원 잔여 최소 수량
	int		n_500_min_count			;	///< (31). 500원 잔여 최소 수량
	int		n_bill_box_full_cnt		;	///< (32). 지폐수집함 최대 수량
	int		n_ticket_box_full_cnt	;	///< (33). 승차권 환불함 최대 수량
 	int		n_issue_count			;	///< (34). 1회 발권 제한 수량
	int		n_issue_money			;	///< (35). 1회 발권 제한 금액
	int		n_issue_time			;	///< (36). 1회 발권 제한 시간(분)
	int		n_mrnp_limit_day		;	///< (37). 현장발권 제한 일자
	int		n_screen_wait_time		;	///< (38). 화면 대기 시간(초)
	int		n_alarm_wait_time		;	///< (39). 알림창 대기시간(초)
	char	tck_paper_name[10]		;	///< (41). 인쇄종류 (DF1, DF2, DF3...)
	BYTE	exp_ticket_device[10]	;	///< (42). 고속버스 승차권 (0x01:승차권발권기 프린터, 0x02:감열지프린터)

	/// 110 추가
	BYTE	cc_rtrp_trml_yn			;	///< (43). [시외] 왕복발권 사용유무(Y/N)
	BYTE	exp_rtrp_trml_yn		;	///< (44). [고속] 왕복발권 사용유무(Y/N)
	BYTE	return_mrnp_yn			;	///< (45). 왕복예매발권 사용유무(Y/N)
	BYTE	exp_line_yn				;	///< (46). 고속노선 판매가능(시외고속) 여부(Y/N)
	BYTE	opend_disp_yn			;	///< (47). 운행종료 표시 여부(Y/N)
	BYTE	no_alcn_no_sats_mrnp_yn	;	///< (48). 비배차 비좌석노선 예매 유무(Y/N)
	BYTE	no_alcn_1_issue_yn		;	///< (49). 비배차 1건시 자동선택 유무(Y/N)
	BYTE	no_sats_free_issue_yn	;	///< (50). 비좌석 무표발권 여부
	int		n_mrnp_limit_time		;	///< (51). 예매발권 제한시간 (분)
	BYTE	today_mrnp_issue_yn		;	///< (52). 당일 예매내역 모두 발권유무 : 사용(Y), 미사용(N)
	BYTE	re_issue_yn				;	///< (53). 승차권 재발행 기능 사용유무 : 사용(Y), 미사용(N)
	BYTE	kiosk_op_close_yn		;	///< (54). 판매 종료시간 사용유무
	BYTE	kiosk_op_start_tm[6]	;	///< (55). 판매 시작 시간 
	BYTE	kiosk_op_close_tm[6]	;	///< (56). 판매 종료 시간 
	BYTE	lcd_off_yn				;	///< (57). 판매 종료시간 LCD OFF 사용유무
	BYTE	ic_card_insert_yn		;	///< (58). IC CARD 투입 알림 사용유무
	BYTE	multi_lang_yn			;	///< (59). 다국어 사용유무
	BYTE	sound_yn				;	///< (60). 안내음성 사용유무 : 사용(Y), 미사용(N)
	int		main_sound_time			;	///< (61). 메인화면 안내음성 반복 시간 (초)
	BYTE	bcnl_yn					;	///< (62). 결행배차 표시유무 : 사용(Y), 미사용(N)
	BYTE	tck_no_rmn_yn			;	///< (63). 매진배차 표시유무 : 사용(Y), 미사용(N)
	BYTE	alcn_state_color_yn		;	///< (64). 배차상태 배경색 표시유무 : 사용(Y), 미사용(N)
	BYTE	disc_btn_yn				;	///< (65). 선택할인 버튼 변경 유무 : 사용(Y), 미사용(N)
	BYTE	rot_search_yn			;	///< (66). 노선검색 사용유무 : 사용(Y), 미사용(N)
	BYTE	air_sta_popup_yn		;	///< (67). 인천공항 알림 팝업 사용 유무 : 사용(Y), 미사용(N)
	BYTE	tck_err_change_yn		;	///< (68). 티켓출력 에러시 교체발권 사용유무 : 사용(Y), 미사용(N)
	BYTE	refund_void_prt_yn		;	///< (69). 승차권 환불 시, 소인 사용유무 : 사용(Y), 미사용(N)
	BYTE	tck_inhr_no_prt_yn		;	///< (70). 티켓 고유번호 출력유무 : 사용(Y), 미사용(N)
	BYTE	no_alcn_sday_prn_yn		;	///< (71). 비배차 승차권 출발일 출력여부
	
	BYTE	no_sats_sday_prn_yn		;	///< (72). 비좌석제 승차권 시간 출력여부
	BYTE	no_sats_time_prn_yn		;	///< (73). 비좌석제 승차권 시간 출력여부
	BYTE	no_alcn_first_yn		;	///< (74). 비배차, 비좌석노선 선착선탑승 안내문구 출력여부(시간미출력 시)
	BYTE	no_alcn_time_corp_yn	;	///< (75). 비배차 시간, 운수사 표시여부
	BYTE	prt_start_day_6_char_yn	;	///< (76). 출발일 6글자 출력유무 : 사용(Y), 미사용(N)
	BYTE	prt_start_day_yn		;	///< (77). 출발일 요일 출력유무 : 사용(Y), 미사용(N)
	BYTE	prt_bus_corp_short_yn	;	///< (78). 운수사 단순 출력유무 : 사용(Y), 미사용(N)
	BYTE	prt_bus_corp_no_me_yn	;	///< (79). 운수사 사업자번호(회수용) 출력유무 : 사용(Y), 미사용(N)
	BYTE	prt_bus_corp_no_you_yn	;	///< (80). 운수사 사업자번호(승객용) 출력유무 : 사용(Y), 미사용(N)
	BYTE	prt_1_bar_code_yn		;	///< (81). 1차원 바코드 출력여부 : 사용(Y), 미사용(N)
	BYTE	prt_2_bar_code_yn		;	///< (82). 2차원 바코드 출력여부 : 사용(Y), 미사용(N)
	BYTE	prt_depr_trml_yn		;	///< (83). 출발지 출력여부 : 사용(Y), 미사용(N)
	BYTE	prt_issue_msg_yn		;	///< (84). 발권메시지 출력여부 : 사용(Y), 미사용(N)
	WORD	w_prt_depr_trml_nm_len	;	///< (85). 승차권 터미널명 축소, 기준 글자수(기본:4)
	BYTE	prt_oper_info_val		;	///< (86). 매표원 정보 인쇄 상태값
	WORD	w_prt_bar_code_height	;	///< (87). 바코드 높이(40 ~ 100) 기본:50
	char	prt_device_type[4]		;	///< (88). 승차권 프린터 디바이스 타입명
	int		n_prt_bold_val			;	///< (89). 프린트 농도 (-5 ~ 5)

	char	map_yn				[1]	;	///< (90). 지도사용유무

	char	rfu					[100];	///< (91). rfu

} UI_BASE_T, *PUI_BASE_T;
***/

typedef struct
{
	// 2019.10.14 add code
	BYTE	version					;	///< (01). 프로토콜 버젼
	BYTE	manufacturer			;	///< (02). 제조사 ('A':에이텍, 'N':한전금)
	BYTE	hw_ic_use_yn			;	///< (03). (HW) IC 리더기 사용여부
	BYTE	hw_cash_use_yn			;	///< (04). (HW) 현금 사용여부
	BYTE	rf_hw_use_yn			;	///< (05). (HW) RF 리더기 사용여부
	BYTE	oper_corp				;	///< (06). 운영사 (금호=0x01, 인천공항=0x02)
	// ~2019.10.14 add code

//	BYTE	device_dvs				;	///< (07). 카드전용('1'), 현금전용('2'), 카드+현금겸용('3')
	BYTE	ccs_svr_kind			;	///< (08). 시외버스 종류(없음:'0', 이지:'1')
	BYTE	exp_svr_kind			;	///< (09). 고속버스 종류(없음:'0', 이지:'1', 코버스:'2')
	
	BYTE	mrnp_func_yn			;	///< (10). 예매발권유무 : 사용(Y), 미사용(N)
	BYTE	mrnp_all_view_yn		;	///< (11). 예매내역전체보기 : 사용(Y), 미사용(N)
	BYTE	mrnp_manual_yn			;	///< (12). 예매수기조회 : 사용(Y), 미사용(N)
	BYTE	mrnp_no_inq_yn			;	///< (13). 예매번호 조회 : 사용(Y), 미사용(N)
	BYTE	mrnp_birth_inq_yn		;	///< (14). 생년월일 조회 : 사용(Y), 미사용(N)
	BYTE	mrnp_corpno_inq_yn		;	///< (15). 법인사업자 조회 : 사용(Y), 미사용(N)
	BYTE	mrnp_auto_iss_yn		;	///< (16). 예매1건 자동발매 : 사용(Y), 미사용(N)

	BYTE	pubtck_yn				;	///< (17). 현장발권유무 : 사용(Y), 미사용(N)
	BYTE	alcn_kind				;	///< (18). 비배차 : 'N', 배차 : 'D'
	BYTE	quick_alcn_yn			;	///< (19). 빠른배차사용유무 : 사용(Y), 미사용(N)
	BYTE	favorate_yn				;	///< (20). 즐겨찾기 사용유무 : 사용(Y), 미사용(N)
	int		n_max_favor_trml_cnt	;	///< (21). 즐겨찾기터미널 최대갯수

	BYTE	pbtck_today_mode_yn		;	///< (22). 당일발권 전용모드 사용유무 : 사용(Y), 미사용(N)

	BYTE	ccs_refund_func_yn		;	///< (23). 시외 - 환불유무 : 사용(Y), 미사용(N)
	BYTE	exp_refund_func_yn		;	///< (24). 고속 - 환불유무 : 사용(Y), 미사용(N)


	// 2019.10.14 add code
	BYTE	sw_ic_pay_yn			;	///< (25). 결제방식 (IC카드 사용유무)
	BYTE	sw_cash_pay_yn			;	///< (26). 결제방식 (현금 사용유무)
	BYTE	sw_rf_pay_yn			;	///< (27). 결제방식 (RF 사용유무)
//	BYTE	pay_method				;	///< (28). 결제방식 (카드:'1',현금:'2',카드+현금:'3')
	// ~2019.10.14 add code
	//BYTE	sw_tpay_pay_yn			;	///<	   결제방식 (tpay 사용유무) kh_200729
	
	BYTE	reg_csrc_yn				;	///< (29). 현금영수증 등록 여부, 사용:'Y', 미사용:'N'
	BYTE	card_installment_yn		;	///< (30). (5만원이상) 카드할부 사용유무 : 사용(Y), 미사용(N)
	BYTE	sign_pad_yn				;	///< (31). 5만원미만 카드 무서명, 무서명:'Y', 서명:'N'
	BYTE	card_passwd_yn			;	///< (32). 카드비밀번호, 사용:'Y', 미사용:'N'
	BYTE	maint_monitor_yn		;	///< (33). 관리자모니터 사용유무 : 사용:'Y', 미사용:'N'
	BYTE	auto_close_yn			;	///< (34). 자동마감 유무 : 사용(Y), 미사용(N)
	char	auto_close_time[6]		;	///< (35). 자동마감 시간(hhmmss)
	BYTE	sms_yn					;	///< (36). 관제서버 사용유무 : 사용(Y), 미사용(N)
	BYTE	sms_ip[50]				;	///< (37). 관제서버 IP Address
	BYTE	camera_1_yn				;	///< (38). 카메라#1 사용유무 : 사용(Y), 미사용(N)
	BYTE	camera_2_yn				;	///< (39). 카메라#2 사용유무 : 사용(Y), 미사용(N)
	BYTE	camera_3_yn				;	///< (40). 카메라#3 사용유무 : 사용(Y), 미사용(N)
	int		n_ticket_min_count		;	///< (41). 승차권 잔여 최소 수량
	int		n_10k_min_count			;	///< (42). 1만원권 잔여 최소 수량
	int		n_1k_min_count			;	///< (43). 1천원권 잔여 최소 수량
	int		n_100_min_count			;	///< (44). 100원 잔여 최소 수량
	int		n_500_min_count			;	///< (45). 500원 잔여 최소 수량
	int		n_bill_box_full_cnt		;	///< (46). 지폐수집함 최대 수량
	int		n_ticket_box_full_cnt	;	///< (47). 승차권 환불함 최대 수량
 	int		n_issue_count			;	///< (48). 1회 발권 제한 수량
	int		n_issue_money			;	///< (49). 1회 발권 제한 금액
	int		n_issue_time			;	///< (50). 1회 발권 제한 시간(분)
	int		n_mrnp_limit_day		;	///< (51). 현장발권 제한 일자 - 고속(일)
	int		n_screen_wait_time		;	///< (52). 화면 대기 시간(초)
	int		n_alarm_wait_time		;	///< (53). 알림창 대기시간(초)
	char	tck_paper_name[10]		;	///< (54). 인쇄종류 (DF1, DF2, DF3...)
	BYTE	exp_ticket_device[10]	;	///< (55). 고속버스 승차권 ('1':승차권발권기 프린터, '2':감열지프린터)
	BYTE	cc_rtrp_trml_yn			;	///< (56). [시외] 왕복발권 사용유무(Y/N)
	BYTE	exp_rtrp_trml_yn		;	///< (57). [고속] 왕복발권 사용유무(Y/N)
	BYTE	return_mrnp_yn			;	///< (58). 왕복예매발권 사용유무(Y/N)
	BYTE	exp_line_yn				;	///< (59). 고속노선 판매가능(시외고속) 여부(Y/N)
	BYTE	opend_disp_yn			;	///< (60). 운행종료 표시 여부(Y/N)
	BYTE	no_alcn_no_sats_mrnp_yn	;	///< (61). 비배차 비좌석노선 예매 유무(Y/N)
	BYTE	no_alcn_1_issue_yn		;	///< (62). 비배차 1건시 자동선택 유무(Y/N)
	BYTE	no_sats_free_issue_yn	;	///< (63). 비좌석 무표발권 여부
	int		n_mrnp_limit_time		;	///< (64). 예매발권 제한시간 (분)
	BYTE	today_mrnp_issue_yn		;	///< (65). 당일 예매내역 모두 발권유무 : 사용(Y), 미사용(N)
	BYTE	re_issue_yn				;	///< (66). 승차권 재발행 기능 사용유무 : 사용(Y), 미사용(N)
	BYTE	kiosk_op_close_yn		;	///< (67). 판매 종료시간 사용유무
	BYTE	kiosk_op_start_tm[6]	;	///< (68). 판매 시작 시간 
	BYTE	kiosk_op_close_tm[6]	;	///< (69). 판매 종료 시간 
	BYTE	lcd_off_yn				;	///< (70). 판매 종료시간 LCD OFF 사용유무
	BYTE	ic_card_insert_yn		;	///< (71). IC CARD 투입 알림 사용유무
	BYTE	multi_lang_yn			;	///< (72). 다국어 사용유무
	BYTE	sound_yn				;	///< (73). 안내음성 사용유무 : 사용(Y), 미사용(N)
	int		main_sound_time			;	///< (74). 메인화면 안내음성 반복 시간 (초)
	BYTE	bcnl_yn					;	///< (75). 결행배차 표시유무 : 사용(Y), 미사용(N)
	BYTE	tck_no_rmn_yn			;	///< (76). 매진배차 표시유무 : 사용(Y), 미사용(N)
	BYTE	alcn_state_color_yn		;	///< (77). 배차상태 배경색 표시유무 : 사용(Y), 미사용(N)
	BYTE	disc_btn_yn				;	///< (78). 선택할인 버튼 변경 유무 : 사용(Y), 미사용(N)
	BYTE	rot_search_yn			;	///< (79). 노선검색 사용유무 : 사용(Y), 미사용(N)
	BYTE	air_sta_popup_yn		;	///< (80). 인천공항 알림 팝업 사용 유무 : 사용(Y), 미사용(N)
	BYTE	tck_err_change_yn		;	///< (81). 티켓출력 에러시 교체발권 사용유무 : 사용(Y), 미사용(N)
	BYTE	refund_void_prt_yn		;	///< (82). 승차권 환불 시, 소인 사용유무 : 사용(Y), 미사용(N)
	BYTE	main_menu_type			;	///< (83). 홈화면 메뉴 타입 ('1':세로, '2':가로)
	BYTE	pubtck_main_use_yn		;	///< (84). 현장발권 메인 메뉴 사용여부('Y':사용, 'N':미사용)
	char	map_yn				[1]	;	///< (85). 지도사용유무
	char	ic_popup_yn			[1]	;	///< (86). IC카드 제거 팝업 사용유무
	int 	n_ic_timeout			;	///< (87). IC카드 타임아웃 시간(초)
	char	first_keybd_yn		[1]	;	///< (88). 초성 키보드 우선 사용유무

	char	rsd_func_yn			[1]	;	///< (89). 상주직원 기능 사용유무
	char	rsd_card_yn			[1]	;	///< (90). 상주직원 - 상주직원카드 사용유무
	char	rsd_login_yn		[1]	;	///< (91). 상주직원 - 로그인 사용유무

	/// 2020.01.15 add data
	char	rot_mode_yn			[1]	;	///< (92). 노선선택 전용모드 유무
	char	rot_mode_mrnp_yn	[1]	;	///< (93). 노선선택 전용모드에서 예매발권 유무
	char	rot_mode_arvl_yn	[1]	;	///< (94). 노선선택 전용모드에서 목적지 검색유무
	char	rf_mode_precard_yn	[1]	;	///< (95). RF모드에서 선불카드 사용유무
	char	rf_mode_postcard_yn	[1]	;	///< (96). RF모드에서 후불카드 사용유무
	char	refund_reprint_yn	[1]	;	///< (97). 환불영수증 재발행 사용유무
	char	mrnp_inq_yn			[1]	;	///< (98). 예매자료 조회 사용유무
	char	keywd_search_yn		[1]	;	///< (99). 키워드 검색 사용유무
	char	prev_mrnp_inq_yn	[1]	;	///< (100).과거 예매내역 발권 사용유무
	char	alcn_time_vw_yn		[1]	;	///< (101).배차시간대 선택버튼 사용유무
	/// ~2020.01.15 add data

	/// 2020.09.15 add data
	char	mrs_manual_tckNo_yn	[1]	;	///< (102). 미사용 예매조회-승차권 일련번호 사용유무
	char	mrs_manual_depdt_yn	[1]	;	///< (103). 미사용 예매조회-출발일자+생년월일 사용유무
	char	mrs_manual_voucher_yn[1];	///< (104). 미사용 예매조회-부가상품권 사용유무
	char	no_alcn_print_date_yn[1];	///< (105). 미사용 비배차승차권 출발일 출력 사용유무
	char	no_sats_print_date_yn[1];	///< (106). 미사용 비좌석제승차권 출발일 출력 사용유무
	char	no_sats_print_time_yn[1];	///< (107). 미사용 비좌석제승차권 시간 출력 사용유무
	char	no_alcn_print_fcfs_yn[1];	///< (108). 미사용 비배차, 비좌석노선 선착순 탑승 안내문구 출력(시간 미출력시) 사용유무
	char	no_alcn_time_cacm_yn[1]	;	///< (109). 미사용 비배차목록 시간, 운수사 표시여부 사용유무
	char	set_refund_receipt_yn[1];	///< (110). 환불영수증 기본 선택여부
	/// ~2020.09.15 add data
	
	/// 2021.01.18 add data
	int		n_mrnp_limit_day_ccs	;	///< (111). 현장예매발권 제한일자 - 시외 (일)
	char	button_sound_yn		[1];	///< (112). 버튼 효과음 사용유무(YN)
	/// ~2021.01.18 add data

	// 2021.03.25 add data
	char	air_kk_iss_input_yn	[1];	///< (113). 발행자정보 입력유무(YN)
	char	ctb_pubtck_lmt_day_yn[1];	///< (114). 현장예매발권 제한일자 - 시외(일) 서버설정 사용유무(YN)
	char	ctb_add_no_begin_time[4];	///< (115). 추가순번 시작시간
	char	ctb_add_no_end_time	[4];	///< (116). 추가순번 종료시간
	char	day_cls_an_cash_cls_yn[1];	///< (117). 창구마감시 시재마감 여부
	// ~2021.03.25 add data

	BYTE	alcn_way_dvs_yn;			///< (119). 비배차 시간표시 옵션(YN) // 20220902 ADD

	char	rfu					[61];	///< (119).rfu	// 20220915 MOD
//	char	rfu					[62];	///< (113).rfu	// 20220915 DEL
//	char	rfu					[73];	///< (111).rfu
//	char	rfu					[87];	///< (102).rfu
//	char	rfu					[97];	///< (92). rfu
} UI_BASE_T, *PUI_BASE_T;


typedef struct
{
	UI_BASE_T	baseInfo_t;
} UI_BASE_INFO_T, *PUI_BASE_INFO_T;

// 장비환경설정 - 터미널정보 (107)
typedef struct
{
	char	trml_wnd_no		[4];	///< 창구번호 ( not setting )
	char	trml_cd			[7];	///< 터미널코드(7바이트) ( not setting )
	char	shct_trml_cd	[4];	///< 단축 터미널코드 ( not setting )
	char	user_no			[20];	///< user no ( not setting )
	char	user_pwd		[20];	///< user pwd  ( not setting )
	char	trml_nm			[50];	///< 터미널 명
	char	trml_rprn_tel_no[50];	///< 터미널 전화번호
	char	prn_trml_nm		[50];	///< 인쇄시 가맹점이름
	char	prn_trml_corp_no[50];	///< 인쇄시 가맹점 사업자번호
	char	prn_trml_sangho	[50];	///< 인쇄시 터미널 상호
	char	prn_trml_corp_no1[50];	///< 인쇄시 터미널 사업자번호
	char	prn_trml_tel_no	[50];	///< 인쇄시 터미널 전화번호
	char	prn_trml_ars_no	[50];	///< 인쇄시 터미널 ARS 번호
} UI_BASE_TRML_INFO_T, *PUI_BASE_TRML_INFO_T;

typedef struct
{
	UI_BASE_TRML_INFO_T		ccTrmlInfo;		///< 시외 터미널 정보
	UI_BASE_TRML_INFO_T		expTrmlInfo;	///< 고속 터미널 정보
} UI_TRML_INFO_T, *PUI_TRML_INFO_T;

// 기초 정보 전송(108)
typedef struct
{
	WORD	wFileID;
	char	szFullName[128];
	DWORD	dwSize;
} UI_BASE_FILE_INFO_T, *PUI_BASE_FILE_INFO_T;

// 버스티켓 고유번호 정보 (109)
typedef struct
{
	char	rfu	;							///< 
	char	ccs_bus_tck_inhr_no [8]	;		///< [시외] 버스티켓 고유번호
	char	ccs_add_bus_tck_inhr_no [8]	;	///< [시외] 추가 버스티켓 고유번호

	char	exp_bus_tck_inhr_no [8]	;		///< [고속] 버스티켓 고유번호
	char	exp_add_bus_tck_inhr_no [8]	;	///< [고속] 추가 버스티켓 고유번호

} UI_ADD_TCK_INHRNO_T, *PUI_ADD_TCK_INHRNO_T;


// 장애 리스트 전송(0121)
typedef struct
{
#define MAX_ALARM	100
	char	byACK;
	int		nCount;

	UI_ALARM_INFO_T list[MAX_ALARM];
} UI_ALARM_LIST_T, *PUI_ALARM_LIST_T;


///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///< 예매발권 - 예매번호 or 생년월일
typedef struct
{
	BYTE	byFunc;				///< 0x01:예매번호, 0x02:생년월일
	char	szMrsNo[17];		///< 예매번호
	char	szBirthDay[6];		///< 생년월일
	char	szMobileNo[50];		///< 핸드폰 번호
} UI_RESP_MRS_INPUT_T, *PUI_RESP_MRS_INPUT_T;

///< 예매발권 - 예매내역 

typedef struct
{
	BYTE	byFunc;					///< 0x01:신용카드, 0x02:생년월일, 0x03:예매번호
	char	szMrsNo			[17];	///< 예매번호
	char	szBirthDay		[6];	///< 생년월일
	char	szMobileNo		[50];	///< 핸드폰 번호
	char	pos_pg_ktc_ver	[20];	///< POS 단말기 버전정보
	int		n_enc_dta_len;			///< ENC 길이
	char	enc_dta			[1024];	///< ENC_DATA
} UI_RESP_MRS_INFO_T, *PUI_RESP_MRS_INFO_T;

// ui203 송신 전문
/**** 버젼 3002 이전 버젼
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

	/// 20191217 add 인천공항 노선때문
	char rot_id				[17];	///< 노선id
	char rot_sqno			[4];	///< 노선순번
	char rot_nm				[100];	///< 노선명
	/// ~20191217 add 인천공항 노선때문

} UI_SND_MRS_LIST_T, *PUI_SND_MRS_LIST_T;
***/

typedef struct
{
	char mrs_no				[17];	///< 예매번호				
	char tisu_id			[17];	///< 발권id					
	char tisu_dt			[8];	///< 발권일자				
	char tisu_time			[6];	///< 발권시각				
	char tisu_chnl_dvs_cd	[1];	///< 발권채널구분코드		
	char pub_chnl_dvs_cd	[1];	///< 발행채널구분코드		
	char pub_dt				[8];	///< 발행일자				
	char pub_time			[6];	///< 발행시각				
	char tisu_amt			[18];	///< 발권금액				
	char pym_aprv_id		[17];	///< 결제승인id				
	char depr_trml_cd		[7];	///< 출발터미널코드			
	char arvl_trml_cd		[7];	///< 도착터미널코드			
	char rot_id				[17];	///< 노선id					
	char rot_sqno			[5];	///< 노선순번				
	char rot_nm				[100];///< 노선명					
	char depr_dt			[8];	///< 출발일자				
	char depr_time			[6];	///< 출발시각				
	char bus_cls_cd			[3];	///< 버스등급코드			
	char bus_tck_knd_cd		[4];	///< 버스티켓종류코드		
	char sats_no			[5];	///< 좌석번호				
	char card_no			[100];///< 카드번호				
	char cty_bus_sys_dvs_cd	[1];	///< 시외버스시스템구분코드	
	char tisu_sys_dvs_cd	[1];	///< 발권시스템구분코드		
	char cty_bus_dc_knd_cd	[1];	///< 시외버스할인종류코드	
	char dcrt_dvs_cd		[1];	///< 할인율구분코드			
	char dc_bef_amt			[18];	///< 할인이전금액			
	char alcn_dt			[8];	///< 배차일자				
	char alcn_sqno			[5];	///< 배차순번				
	char rpub_num			[5];	///< 재발행매수				
} UI_SND_MRS_LIST_T, *PUI_SND_MRS_LIST_T;

typedef struct
{
	BYTE	byAck;				///< 0x01:예매번호, 0x02:생년월일
	WORD	wCount;
	PUI_SND_MRS_LIST_T pList;
} UI_SND_MRS_INFO_T, *PUI_SND_MRS_INFO_T;

///< 예매발권 - 발권 (205)

/**** 버젼 3002 이전 버젼
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

	/// 20191217 add 인천공항 노선때문
	char rot_id				[17];	///< 노선id
	char rot_sqno			[4];	///< 노선순번
	char rot_nm				[100];	///< 노선명
	/// ~20191217 add 인천공항 노선때문

} UI_RESP_MRS_ISSUE_LIST_T, *PUI_RESP_MRS_ISSUE_LIST_T;
*******/

typedef struct
{
	char mrs_no				[17];	///< 예매번호				
	char tisu_id			[17];	///< 발권id					
	char tisu_dt			[8];	///< 발권일자				
	char tisu_time			[6];	///< 발권시각				
	char tisu_chnl_dvs_cd	[1];	///< 발권채널구분코드		
	char pub_chnl_dvs_cd	[1];	///< 발행채널구분코드		
	char pub_dt				[8];	///< 발행일자				
	char pub_time			[6];	///< 발행시각				
	char tisu_amt			[18];	///< 발권금액				
	char pym_aprv_id		[17];	///< 결제승인id				
	char depr_trml_cd		[7];	///< 출발터미널코드			
	char arvl_trml_cd		[7];	///< 도착터미널코드			
	char rot_id				[17];	///< 노선id					
	char rot_sqno			[5];	///< 노선순번				
	char rot_nm				[100];///< 노선명					
	char depr_dt			[8];	///< 출발일자				
	char depr_time			[6];	///< 출발시각				
	char bus_cls_cd			[3];	///< 버스등급코드			
	char bus_tck_knd_cd		[4];	///< 버스티켓종류코드		
	char sats_no			[5];	///< 좌석번호				
	char card_no			[100];///< 카드번호				
	char cty_bus_sys_dvs_cd	[1];	///< 시외버스시스템구분코드	
	char tisu_sys_dvs_cd	[1];	///< 발권시스템구분코드		
	char cty_bus_dc_knd_cd	[1];	///< 시외버스할인종류코드	
	char dcrt_dvs_cd		[1];	///< 할인율구분코드			
	char dc_bef_amt			[18];	///< 할인이전금액			
	char alcn_dt			[8];	///< 배차일자				
	char alcn_sqno			[5];	///< 배차순번				
	char rpub_num			[5];	///< 재발행매수				
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

///< 예매발권 - 교체발권 (UI-206)
typedef struct
{
	BYTE	byFlag;						///< 0x02 : 교체발권 시작, 0x03:교체발권 취소
} UI_RESP_MRS_CHG_ISSUE_T, *PUI_RESP_MRS_CHG_ISSUE_T; 

typedef struct
{
	BYTE	byACK;						///< ACK : 0x06, NACK : 0x15
} UI_SND_MRS_CHG_ISSUE_T, *PUI_SND_MRS_CHG_ISSUE_T; 


///< 예매발권 - 발권상태 (UI-207)
typedef struct
{
	BYTE	byACK;						///< ACK : 0x06, NACK : 0x15
	BYTE	byState;					///< 발권 상태 (0x01:시작, 0x02:완료)
	WORD	w_sats_no;					///< 좌석번호
} UI_SND_MRS_ISSUE_STATE_T, *PUI_SND_MRS_ISSUE_STATE_T; 

///< 예매발권 - 발권상태
typedef struct
{
	BYTE	byACK;						///< 발권결과 
	BYTE	byState;					///< 발권 상태 (0x01:시작, 0x02:완료)
	BYTE	bySeat;						///< 좌석번호
} UI_SND_PBTCK_ISSUE_STATE_T, *PUI_SND_PBTCK_ISSUE_STATE_T; 

///< 승차권 발권 완료 : 20200406 추가
typedef struct
{
	BYTE	byACK;						///< 발권결과 
	BYTE	card_aprv_no[100];			///< 카드 승인번호
} UI_SND_PBTCK_ISSUE_COMP_T, *PUI_SND_PBTCK_ISSUE_COMP_T; 

///< [코버스] 예매발권 - 리스트 요청 (UI-233)
typedef struct
{
	char	user_no		[6+1];			///< 사용자 NO
	char	lng_cd		[2+1];			///< 언어구분		
	char	tissu_dvs_cd[1+1];			///< 예매발행취소구분	
	char	depr_dt		[8+1];			///< 출발일자		
	char	pyn_dvs_cd	[1+1];			///< 검색조건		
	char	mrs_mrnp_no	[14+1];			///< 1:예매예약번호	
	char	mrsp_mbph_no[100+1];		///< 2:예매자휴대폰번호	
	char	mrsp_brdt	[8+1];			///< 25:생년월일	
	char	card_no		[100+1];		///< 34:카드번호	
	char	depr_trml_no[3+1];			///< 출발터미널	
	//추가 20200909
	char	adtn_cpn_no	[16+1];			///< 부가상품번호
	char	alcn_depr_dt[8+1];			///< 출발일자

} UI_RESP_KO_MRS_REQ_LIST_T, *PUI_RESP_KO_MRS_REQ_LIST_T; 

typedef struct
{
	BYTE	ack;						///< ack:0x06, nack:0x15
} UI_SND_KO_MRS_REQ_LIST_T, *PUI_SND_KO_MRS_REQ_LIST_T; 

///< [코버스] 예매발권 - 리스트 응답 (UI-234)
typedef struct
{
// 	BYTE	msg_cd		[6];			///< 메시지 코드
// 	BYTE	msg_dtl_ctt	[200];			///< 메시지 상세내용
	int		n_rec_ncnt1;				///< 레코드 건수
	rtk_tm_mrsinfo_list_t *pList;
} UI_SND_KO_MRS_LIST_T, *PUI_SND_KO_MRS_LIST_T;

typedef struct
{
	BYTE	ack;						///< ack:0x06, nack:0x15
} UI_RESP_KO_MRS_LIST_T, *PUI_RESP_KO_MRS_LIST_T; 

///< [코버스] 예매발권 - 예매내역 선택 (UI-235)

typedef struct
{
	char	user_no			[6+1];		///< 사용자 NO		
	char	lng_cd			[2+1];		///< 언어구분		
	char	pyn_dtl_cd		[1+1];		///< 지불상세코드		
	char	tissu_trml_no	[3+1];		///< 발권터미널번호	
	char	wnd_no			[2+1];		///< 창구번호		
	char	stt_inhr_no		[8+1];		///< 시작고유번호		
} UI_RESP_KO_MRS_SEL_DATA_T, *PUI_RESP_KO_MRS_SEL_DATA_T;

typedef struct
{
	char	mrs_mrnp_no		[14+1];		///< 예매예약번호_arr	
	char	mrs_mrnp_sno	[2+1];		///< 예매예약일련번호_arr
} UI_KO_MRS_SEL_LIST_T, *PUI_KO_MRS_SEL_LIST_T;

typedef struct
{
	UI_RESP_KO_MRS_SEL_DATA_T	Data;		
	int 						rec_ncnt;	///< 입력_레코드건수	
	char						chList;
} UI_RESP_KO_MRS_SEL_LIST_T, *PUI_RESP_KO_MRS_SEL_LIST_T;

typedef struct
{
	BYTE	ack;						///< ack:0x06, nack:0x15
} UI_SND_KO_MRS_SEL_LIST_T, *PUI_SND_KO_MRS_SEL_LIST_T;

///< [티머니고속] 예매발권 - 리스트 요청 (UI-250)
typedef struct
{
	stk_tm_read_mrs_t	rData;
} UI_RESP_TMEXP_MRS_REQ_T, *PUI_RESP_TMEXP_MRS_REQ_T; 

typedef struct
{
	BYTE	ack;						///< ack:0x06, nack:0x15
} UI_SND_TMEXP_MRS_REQ_T, *PUI_SND_TMEXP_MRS_REQ_T; 

///< [티머니고속] (KTC) 예매발권 - 리스트 요청 (UI-252)
typedef struct
{
	stk_tm_read_mrs_ktc_t	rData;
} UI_RESP_TMEXP_KTC_MRS_REQ_T, *PUI_RESP_TMEXP_KTC_MRS_REQ_T; 

typedef struct
{
	BYTE	ack;						///< ack:0x06, nack:0x15
} UI_SND_TMEXP_KTC_MRS_REQ_T, *PUI_SND_TMEXP_KTC_MRS_REQ_T; 

///< [티머니고속] 예매발권 - 예매발권 요청 (UI-254)
typedef struct
{
	stk_tm_pub_mrs_t	rData;
} UI_RESP_TMEXP_MRS_REQ_PUB_T, *PUI_RESP_TMEXP_MRS_REQ_PUB_T; 

typedef struct
{
	BYTE	ack;						///< ack:0x06, nack:0x15
} UI_SND_TMEXP_MRS_REQ_PUB_T, *PUI_SND_TMEXP_MRS_REQ_PUB_T; 

///< [티머니고속] 예매발권 - 모바일 예매발권 요청 (UI-256)
typedef struct
{
	stk_tm_pub_mrs_htck_t	rData;
} UI_RESP_TMEXP_MRS_REQ_MOBILE_PUB_T, *PUI_RESP_TMEXP_MRS_REQ_MOBILE_PUB_T; 

typedef struct
{
	BYTE	ack;						///< ack:0x06, nack:0x15
} UI_SND_TMEXP_MRS_REQ_MOBILE_PUB_T, *PUI_SND_TMEXP_MRS_REQ_MOBILE_PUB_T; 


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
	char alcn_way_dvs_cd	   ;	///< 배차('D'), 비배차('N')
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
	char whch_tissu_yn		[1];	///< 휠체어발권여부 // 20211206 ADD
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

	char alcn_way_dvs_cd	[1];	///< 배차방식 구분코드
	char sati_use_yn		[1];	///< 좌석제 사용유무


} UI_RESP_PBTCK_SELECT_T, *PUI_RESP_PBTCK_SELECT_T;


// (0305) 현장발권 - 좌석 & 승차권 종류별 선택 
typedef struct
{
	char cty_bus_dc_knd_cd[1];	///< 시외버스할인종류코드
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


// (308) 현장발권 - 지폐투입 시작
typedef struct
{
	BYTE	byBillDvs;
	BYTE	bySvrDvs;
	int		nTotalMoney;
} UI_RESP_BILL_START_T, *PUI_RESP_BILL_START_T;

// (309) 현장발권 - 지폐투입정보 전달
typedef struct
{
	BYTE	byACK;
	BYTE	bySvrDvs;
	WORD	w1k;
	WORD	w5k;
	WORD	w10k;
	WORD	w50k;
} UI_SND_BILL_INFO_T, *PUI_SND_BILL_INFO_T;

// (311) 현장발권 - 승차권 결제 요청
typedef struct
{
	int	 nTotalMoney		 ;			///< 총 결제금액
	//char ui_pym_dvs_cd		[1];		///< 결제 구분코드 - 0x01:현금결제, 0x02:카드결제, 0x04:RF결제																		// 20221124 DEL
	char ui_pym_dvs_cd		[1];		///< 결제 구분코드 - 0x01:현금결제, 0x02:카드결제, 0x04:RF결제, 0x21:페이코QR-카드결제, 0x22:페이코QR-포인트결제, 0x31:티머니페이QR-결제	// 20221124 MOD
	char csrc_dvs_cd		[1];		///< 현금영수증 구분코드 - 0:미사용, 1:수기입력, 2:신용카드, 3:현영전용카드
	char csrc_no			[20];		///< 현금영수증 수기입력 번호
	char csrc_use			[1];		///< 0:개인, 1:법인
	char trd_dvs_cd			[1];		///< 거래구분코드			
	char fallback_dvs_cd	[1];		///< fallback구분코드	
	char pos_pg_ktc_ver		[20];		///< POS 단말기 버젼
	char enc_dta_len		[4];		///< enc 데이터 길이
	char enc_dta			[1024];		///< enc 데이터
	char emv_dta			[1024];		///< emv 데이터
	char mip_term			[2];		///< 할부기간
	char spad_pwd_yn		[1];		///< 서명비밀번호여부 - 1:싸인, 2:비밀번호, 3:싸인+비밀번호
	char card_pwd			[10];		///< 카드비밀번호			
	char spad_dta			[3072];		///< 싸인패드데이터		
	char spad_dta_len		[4];		///< 싸인패드데이터길이	
	char rf_trcr_dvs_cd		[1];		///< 결제구분코드 = (RF후불교통카드:D, 티머니:T, 마이페어:M)
	char user_dvs_no		[20];		///< 상주직원일때만 값을 전송
    // 20221017 ADD~
    char qr_cd_no			[100];      // QR코드번호
    char payco_virt_ontc_no	[21];		// OTC번호;페이코가상일회성카드번호
    // 20221017 ~ADD

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

// (313) 현장발권 - 발권 시작 또는 교체발권 
typedef struct
{
	BYTE	byFlag;			;			///< 발권시작:0x01, 교체발권 시작:0x02, 교체발권 취소:0x03
} UI_RESP_ISSUE_START_T, *PUI_RESP_ISSUE_START_T;


// (315) 현장발권 - 거스름돈 방출
typedef struct
{
	BYTE	bySvrDvs		;			///< 서버구분
	int  nTotalMoney		;			///< 결제금액
	int  nChangeMoney		;			///< 방출금액(=거스름돈)
} UI_RESP_CHG_MONEY_T, *PUI_RESP_CHG_MONEY_T;

// (316) 현장발권 - 거스름돈 방출
typedef struct
{
	BYTE byAck;						///< 
	BYTE bySvrDvs;					///< 서버구분
	WORD w1k;						///< 지폐 - 1,000원
	WORD w5k;						///< 지폐 - 5,000원
	WORD w10k;						///< 지폐 - 10,000원
	WORD w50k;						///< 지폐 - 50,000원
	WORD w10;						///< 동전 - 10원
	WORD w50;						///< 동전 - 50원
	WORD w100;						///< 동전 - 100원
	WORD w500;						///< 동전 - 500원

} UI_SND_CHG_MONEY_T, *PUI_SND_CHG_MONEY_T; 

// (317) 현장발권 - 신용카드 정보 전달
typedef struct
{
	char trd_dvs_cd			[1];		///< 거래구분코드			
	char fallback_dvs_cd	[1];		///< fallback구분코드	
	char pos_pg_ktc_ver		[20];		///< pos단말기버전정보		
	char enc_dta_len		[4];		///< enc 데이터 길이		
	char enc_dta			[1024];		///< enc 데이터			
	char emv_dta			[1024];		///< emv 데이터			
	char spad_pwd_yn		[1];		///< 서명비밀번호여부		
	char card_pwd			[10];		///< 카드비밀번호			
	char spad_dta			[3072];		///< 싸인패드데이터		
	char spad_dta_len		[4];		///< 싸인패드데이터길이		
} UI_RESP_CREDIT_CARD_INFO_T, *PUI_RESP_CREDIT_CARD_INFO_T;

// 20190923 add by nhso - 구조체 추가
// (320) 현장발권 - 상주직원 정보 요청 
typedef struct
{
	char user_dvs_no		[20];	///< 사용자구분번호			
	char user_pwd			[100];	///< 사용자비밀번호			
	char tisu_req_yn		[1];	///< 발권요청여부			
	char rot_id				[17];	///< 노선id					
	char rot_sqno			[4];	///< (int)노선순번				
	char alcn_dt			[8];	///< 배차일자				
	char alcn_sqno			[4];	///< (int)배차순번				
} UI_RESP_STAFF_REQ_INFO_T, *PUI_RESP_STAFF_REQ_INFO_T;

// (322) 현장발권 - 상주직원 카드 좌석요금 정보 변경 요청
typedef struct
{
	char sats_pcpy_id		[17];	///< 좌석선점id					
	char bus_tck_knd_cd		[4];	///< 버스티켓종류코드				
	char sats_no			[4];	///< 좌석번호					
} UI_RESP_STAFF_CD_MOD_FARE_LIST_T, *PUI_RESP_STAFF_CD_MOD_FARE_LIST_T;

typedef struct
{
	char rot_id				[17];	///< 노선id					
	char rot_sqno			[4];	///< 노선순번					
	char alcn_dt			[8];	///< 배차일자					
	char alcn_sqno			[4];	///< 배차순번					
	char depr_trml_cd		[7];	///< 출발터미널코드				
	char arvl_trml_cd		[7];	///< 도착터미널코드				
	char damo_enc_card_no	[1024];	///< 카드번호암호문				
	char damo_enc_card_no_len[4];	///< 카드번호암호문길이				
	char damo_enc_dta_key	[32];	///< 암호문키					
	
	char pub_num			[4];	///< 발행매수					
	UI_RESP_STAFF_CD_MOD_FARE_LIST_T	List[30];
} UI_RESP_STAFF_CD_MOD_FARE_T, *PUI_RESP_STAFF_CD_MOD_FARE_T;

// (324) 현장발권 - 승차권발행(RF선불카드)

typedef struct
{
	char rot_id					[17];		///< 노선id			
	char rot_sqno				[4];		///< (int)노선순번			
	char alcn_dt				[8];		///< 배차일자			
	char alcn_sqno				[4];		///< (int)배차순번			
	char depr_trml_cd			[7];		///< 출발터미널코드		
	char arvl_trml_cd			[7];		///< 도착터미널코드		
	char tisu_way_dvs_cd		[1];		///< 발권방식구분코드		
	char read_rot_yn			[1];		///< 예매자료조회여부		
	char pub_num				[4];		///< (int)발행매수			
	char pyn_mns_dvs_cd			[1];		///< 지불수단구분코드		
	char pub_chnl_dvs_cd		[1];		///< 발행채널구분코드		
	char user_dvs_no			[20];		///< 사용자구분번호		
	char bus_tck_knd_cd			[4];		///< 버스티켓종류코드		
	char sats_no				[4];		///< (int)좌석번호			
	char sats_pcpy_id			[17];		///< 좌석선점id			
	char old_mbrs_no			[16];		///< 구회원번호			
	char mrnp_id				[17];		///< 예약id			
	char mrnp_num				[4];		///< (int)예약매수			
	char damo_enc_card_no		[1024];		///< 카드번호암호문		
	char damo_enc_card_no_len	[4];		///< (int)카드번호암호문길이		
	char damo_enc_dta_key		[32];		///< 암호문키	
	char rf_trcr_dvs_cd			[1];		///< RF교통카드 구분코드
	char ppy_tak_dvs_cd			[3];		///< 선불작업구분코드		
	char ppy_pos_sls_dt			[8];		///< 선불pos영업일자		
	char ppy_pos_recp_no		[8];		///< 선불pos영수증번호		
	char ppy_sam_id				[16];		///< 선불samid			
	char ppy_sam_trd_sno		[10];		///< 선불sam거래일련번호	
	char ppy_card_trd_sno		[10];		///< 선불카드거래일련번호	
	char ppy_aprv_aft_bal		[4];		///< (int)선불승인이후잔액		
	char ppy_aprv_bef_bal		[4];		///< (int)선불승인이전잔액		
	char ppy_algr_id			[2];		///< 선불알고리즘id		
	char ppy_sign_val			[100];		///< 선불sign값			
	char ppy_indv_trd_clk_ver	[2];		///< 선불개별거래수집키버전	
	char ppy_elcs_idnt_id		[2];		///< 선불전자화폐식별자id	
	char sam_ttam_clcn_sno		[10];		///< sam총액수집일련번호	
	char sam_indv_clcn_ncnt		[4];		///< (int)sam개별수집건수		
	char sam_cum_trd_ttam		[4];		///< (int)sam누적거래총액		
	char ppy_card_dvs_cd		[2];		///< 선불카드구분코드		
	char ppy_card_user_dvs_cd	[2];		///< 선불카드사용자구분코드	
	char ppy_hsm_sta_cd			[1];		///< 선불hsm상태코드		
	char req_trd_amt			[4];		///< (int)거래요청금액			

} UI_RESP_RF_CD_PAYMENT_LIST_T, *PUI_RESP_RF_CD_PAYMENT_LIST_T;

typedef struct
{
	int	nCount;								///< 발권요청 수량
	BYTE byData;							///< 
	//UI_RESP_RF_CD_PAYMENT_LIST_T List[50];	
} UI_RESP_RF_CD_PAYMENT_T, *PUI_RESP_RF_CD_PAYMENT_T;


// ~20190923 add by nhso - 구조체 추가

// 2020.04.06 add code
// (326) 발행자 정보 입력 (경기도 코로나 한시적 운영)
typedef struct
{
	BYTE damo_enc_pub_user_tel_no	[20];	///< 전화번호
	BYTE damo_enc_pub_user_krn		[20];	///< 주민번호 or 여권번호
	BYTE ride_vhcl_dvs				[2];	///< 승차 차량구분 (MV : 자가용, SV : 지원차량(시,군))
} UI_RESP_PUBUSRINFINP_T, *PUI_RESP_PUBUSRINFINP_T;

typedef struct
{
	char ack				;				///< ack: 0x06, nack:0x15
} UI_SND_PUBUSRINFINP_T, *PUI_SND_PUBUSRINFINP_T;

// ~2020.04.06 add code

// (331) [코버스] 현장발권 - 배차조회 요청
typedef struct
{
	char user_no			[6+1];		/// 사용자NO	
	char lng_cd				[2+1];		/// 언어구분	
	char depr_trml_no 		[3+1];		/// 출발터미널번호
	char arvl_trml_no 		[3+1];		/// 도착터미널번호
	char depr_dt      		[8+1];		/// 출발일자	
	char depr_time    		[6+1];		/// 출발시각	
	char bus_cls_cd			[3+1];		/// 버스등급코드
	int  n_rec_ncnt				 ;		/// 조회요청건수 (99999)

} UI_RESP_KO_REQ_ALCN_T, *PUI_RESP_KO_REQ_ALCN_T;

typedef struct
{
	char ack				;			/// ack: 0x06, nack:0x15
} UI_SND_KO_REQ_ALCN_T, *PUI_SND_KO_REQ_ALCN_T;

// (332) [코버스] 현장발권 - 배차조회 결과
typedef struct
{
	rtk_tm_timinfo_fee_t svr_data;
	int					 rec_ncnt;	
	rtk_tm_timinfo_list_t *pLIst;

} UI_SND_KO_ALCN_LIST_T, *PUI_SND_KO_ALCN_LIST_T;

typedef struct
{
	char ack				;			/// ack: 0x06, nack:0x15
} UI_RESP_KO_ALCN_LIST_T, *PUI_RESP_KO_ALCN_LIST_T;

// (333) [코버스] 현장발권 - 좌석정보 요청
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
	/// 2020.05.18 필드 추가
	char alcn_rot_no 		[4+1]	;	///< 배차노선번호

} UI_RESP_KO_REQ_SATS_T, *PUI_RESP_KO_REQ_SATS_T;



// (335) [코버스] 현장발권 - 좌석 선점 요청
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
	char depr_thru_seq		[4]	;		///< (int) 출발경유순서		
	char arvl_thru_seq		[4]	;		///< (int) 도착경유순서		
	char tot_sats_num		[4]	;		///< (int) 총좌석수		
	char rec_ncnt	 		[4]	;		///< (int) 조회 요청 건수
	stk_tw_satspcpy_list_t  List[20];	
} UI_RESP_KO_REQ_SATSPCPY_T, *PUI_RESP_KO_REQ_SATSPCPY_T;

typedef struct
{
	char ack				;			/// ack: 0x06, nack:0x15
} UI_SND_KO_REQ_SATSPCPY_T, *PUI_SND_KO_REQ_SATSPCPY_T;

// (336) [코버스] 현장발권 - 좌석 선점 응답
typedef struct
{
	char sats_no			[2] ;		/// 좌석번호
	char pcpy_no			[14] ;		/// 선점번호
} UI_KO_SATSPCPY_T, *PUI_KO_SATSPCPY_T;

typedef struct
{
	int	n_rec_ncnt			;			/// 건수
	UI_KO_SATSPCPY_T	List[20];		/// list
} UI_SND_KO_SATSPCPY_RESULT_T, *PUI_SND_KO_SATSPCPY_RESULT_T;

typedef struct
{
	char ack				;			/// ack: 0x06, nack:0x15
} UI_RESP_KO_SATSPCPY_RESULT_T, *PUI_RESP_KO_SATSPCPY_RESULT_T;

// (337) [코버스] 현장발권 - 현장발권 
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
	int  mip_mm_num					;	///< (int) C:할부개월수(0:일시불)
	char sgn_inf			[3072+1];	///< C:서명정보
	char iccd_inf			[512+1]	;	///< C:IC정보 (IC카드인 경우만)
	char chit_use_dvs		[1+1]	;	///< M:전표사용구분
	char iccd_yn			[1+1]	;	///< IC카드여부(Y/N)
	char trck_dta_enc_cd	[1+1]	;	///< TrackData암호화구분 (D:DAmo암호화, R:외부기기 암호화, .(점):암호화 안함)

	char card_no_mask		[20+1]	;	///< (추가)마스킹 카드번호
	char csrc_dvs_cd		[1+1]	;	///< (추가)현금영수증 구분코드
	int	 nTotalMoney				;	///< 총 결제금액
	int  tissu_hcnt					;	///< (int) 발권매수
	stk_tm_tcktran_list_t List[30];
} UI_RESP_KO_TCK_TRAN_T, *PUI_RESP_KO_TCK_TRAN_T;

typedef struct
{
	char ack				;			/// ack: 0x06, nack:0x15
} UI_SND_KO_TCK_TRAN_T, *PUI_SND_KO_TCK_TRAN_T;

// (338) [코버스] 현장발권 - 경유지 정보 요청
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
} UI_RESP_KO_REQ_THRUINFO_T, *PUI_RESP_KO_REQ_THRUINFO_T;

/////////////////////////////////////////////////////////////////

// (350) [티머니고속] 현장발권 - 배차정보 조회 요청
typedef struct
{
	stk_tm_readalcn_t		dt;
} UI_RESP_TM_REQ_ALCN_T, *PUI_RESP_TM_REQ_ALCN_T;


// (352) [티머니고속] 현장발권 - 요금/좌석정보 조회 요청
typedef struct
{
	stk_tm_readsatsfee_t	dt;
	char					rot_rdhm_no_val[100 + 1];

} UI_RESP_TM_READ_SATS_FEE_T, *PUI_RESP_TM_READ_SATS_FEE_T;

// (354) [티머니고속] 현장발권 - 좌석선점 요청
typedef struct
{
	stk_tm_pcpysats_t		dt;
} UI_RESP_TM_PCPY_SATS_T, *PUI_RESP_TM_PCPY_SATS_T;

// (356) [티머니고속] 현장발권 - 승차권발권 (현금)
typedef struct
{
	stk_tm_pubtckcash_t		dt;
} UI_RESP_TM_PUBTCK_CASH_T, *PUI_RESP_TM_PUBTCK_CASH_T;

// (358) [티머니고속] 현장발권 - 승차권발권 (카드_KTC)
typedef struct
{
	stk_tm_pubtckcard_ktc_t	dt;
} UI_RESP_TM_PUBTCK_CARD_KTC_T, *PUI_RESP_TM_PUBTCK_CARD_KTC_T;

// (360) [티머니고속] 현장발권 - 승차권발권 (현금영수증_KTC)
typedef struct
{
	stk_tm_pubtckcsrc_ktc_t	dt;
} UI_RESP_TM_PUBTCK_CSRC_KTC_T, *PUI_RESP_TM_PUBTCK_CSRC_KTC_T;

// (362) [티머니고속] 현장발권 - 승차권발권 (부가상품권)
typedef struct
{
	stk_tm_pubtckprd_t		dt;
} UI_RESP_TM_PUBTCK_PRD_T, *PUI_RESP_TM_PUBTCK_PRD_T;

// (364) [티머니고속] 현장발권 - 경유지정보 조회
typedef struct
{
	stk_tm_readthrutrml_t	dt;
} UI_RESP_CM_READTHRU_TRML_T, *PUI_RESP_CM_READTHRU_TRML_T;


///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///< 환불 - 승차권 읽기 결과 전송 (402)
typedef struct
{
	char byAck;						///< 결과
	char chBusDVS;					///< 시외:'1', 코버스:'2', 티머니고속:'3'
	char pyn_mns_dvs_cd		[1];	///< 지불수단구분코드	
	int n_pym_amt			   ;	///< 결제금액		
	int n_canc_ry_cmrt		   ;	///< 취소수수료		
	int n_canc_ry_amt	 	   ;	///< 취소환불금액		
	char brkp_type			[1];	///< 위약금 타입(BRKP_TYPE)					// 20221223 ADD
	char qr_pym_pyn_dtl_cd	[3];	///< QR결제지불상세코드						// 20221221 ADD
} UI_SND_CANCRY_TCK_INFO_T, *PUI_SND_CANCRY_TCK_INFO_T; 

///< 환불 - 환불 금액 처리
typedef struct
{
	char ui_pym_dvs_cd		[1]	;	///< 결제 수단
	int	nFare					;	///< 결제 금액
	int	nOutFare				;	///< 방출 금액
	char receipt_yn			[1]	;	///< 환불영수증 출력여부

} UI_RESP_CANCRY_FARE_T, *PUI_RESP_CANCRY_FARE_T; 

///< 환불 - 카드 또는 현금 방출 정보 전송
typedef struct
{
	BYTE byAck;						///< 
	char pyn_mns_dvs_cd[1];			///< 지불수단코드
	WORD w1k;						///< 지폐 - 1,000원
	WORD w5k;						///< 지폐 - 5,000원
	WORD w10k;						///< 지폐 - 10,000원
	WORD w50k;						///< 지폐 - 50,000원
	WORD w10;						///< 동전 - 10원
	WORD w50;						///< 동전 - 50원
	WORD w100;						///< 동전 - 100원
	WORD w500;						///< 동전 - 500원

} UI_SND_CANCRY_CHG_MONEY_T, *PUI_SND_CANCRY_CHG_MONEY_T; 

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///< 공통

typedef struct
{
	BYTE	byAck;				///< 
	char	szErrCode[10];		///< 에러코드
} UI_SND_FAIL_INFO_T, *PUI_SND_FAIL_INFO_T; 


///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// [관리자] 로그인
typedef struct
{
	BYTE	byLogin;			///< 0x01:로그인, 0x02:로그아웃
	char	szID[20];			///< 사용자 ID
	char	szPwd[20];			///< 비밀번호
} UI_RESP_ADMIN_LOGIN_T, *PUI_RESP_ADMIN_LOGIN_T;

typedef struct
{
	BYTE	byAck;				///< 
	BYTE	byLogin;			///< 0x01:로그인, 0x02:로그아웃
} UI_SND_ADMIN_LOGIN_T, *PUI_SND_ADMIN_LOGIN_T;

// [관리자] 설정관리 - 기능설정(502)
typedef struct
{
	UI_BASE_T	baseInfo_t;
} UI_RESP_ADMIN_FUNC_T, *PUI_RESP_ADMIN_FUNC_T;

// [관리자] 설정관리 - 터미널 설정 (504)
typedef struct
{
	char	szThrmlCode[7];			///< 터미널 코드
	WORD	wSeqNo;					///< 순번
	BYTE	byUse;					///< 사용유무, 0:사용, 1:미사용
} UI_RESP_THRML_LIST_T, *PUI_RESP_THRML_LIST_T;

typedef struct
{
	char	chBusDVS;				///< 시외:'1', 코버스:'2', 티머니고속:'3'
	WORD	wCount;					///< 갯수
	char	byList;
} UI_RESP_ADMIN_THRML_T, *PUI_RESP_ADMIN_THRML_T;

// [관리자] 설정관리 - 승차권 설정 (505)
typedef struct
{
	char	szCode[4];				///< 버스티켓종류 코드
	BYTE	byUse;					///< 사용유무, 0:미사용, 1:사용
} UI_RESP_TICKET_LIST_T, *PUI_RESP_TICKET_LIST_T;

typedef struct
{
	char	chBusDVS;				///< 시외:'1', 코버스:'2', 티머니고속:'3'
	WORD	wCount;					///< 갯수
	char	byList;
} UI_RESP_ADMIN_TICKET_T, *PUI_RESP_ADMIN_TICKET_T;

// [관리자] 설정관리 - 운수사 설정 (506)
typedef struct
{
	char	szCode[4];				///< 운수사 코드
	WORD	wSeq;					///< 운수사 순번
} UI_RESP_BUS_CACM_LIST_T, *PUI_RESP_BUS_CACM_LIST_T;

typedef struct
{
	char	chBusDVS;				///< 시외:'1', 코버스:'2', 티머니고속:'3'
	WORD	wCount;					///< 갯수
	char	byList;
} UI_RESP_ADMIN_BUS_CACM_T, *PUI_RESP_ADMIN_BUS_CACM_T;


// 장비환경설정 - 터미널 설정 (508)
typedef struct
{
	UI_BASE_TRML_INFO_T		ccTrmlInfo;		///< 시외 터미널 정보
	UI_BASE_TRML_INFO_T		expTrmlInfo;	///< 고속 터미널 정보

} UI_RESP_ADMIN_SET_TRML_INFO_T, *PUI_RESP_ADMIN_SET_TRML_INFO_T;


// [관리자] 티켓관리 - 티켓고유번호 설정 (521)
typedef struct
{
	char	chBusDVS;				///< 시외:'1', 코버스:'2', 티머니고속:'3'
	char	bus_tck_inhr_no[8];		///< 버스티켓고유번호
} UI_RESP_ADMIN_TCK_INHR_T, *PUI_RESP_ADMIN_TCK_INHR_T;

// [관리자] 티켓관리 - 승차권 보급 (522)
typedef struct
{
	char	chBusDVS;				///< 시외:'1', 코버스:'2', 티머니고속:'3'
	int		nTicketCount;				///< 승차권 수량
} UI_RESP_ADMIN_TCK_COUNT_T, *PUI_RESP_ADMIN_TCK_COUNT_T;

// [관리자] 티켓관리 - 승차권 정보 전송 (524)
typedef struct
{
	BYTE	byACK;
	int		n_ticket_count;				///< 시외_승차권 수량
	char	ccs_bus_tck_inhr_no[8];		///< 시외_버스티켓고유번호
	char	exp_bus_tck_inhr_no[8];		///< 고속_버스티켓고유번호

} UI_SND_ADMIN_TCK_INFO_T, *PUI_SND_ADMIN_TCK_INFO_T;


// [관리자] 발권 내역 관리 (531, 532, 533)

typedef struct
{
	BYTE	byInq;					///< 조회항목 (0x01:발행내역, 0x02:재발행내역, 0x03:환불내역)
	char	chBusDVS;				///< 시외:'1', 코버스:'2', 티머니고속:'3'
} UI_RESP_ADMIN_INQ_T, *PUI_RESP_ADMIN_INQ_T;

typedef struct
{
	BYTE	byAck;					///< 결과
	char	szErrorCode[10];		///< 에러코드
} UI_SND_ADMIN_INQ_NAK_T, *PUI_SND_ADMIN_INQ_NAK_T;

typedef struct
{
	BYTE	byAck;					///< 결과
	char	chBusDVS;				///< 시외:'1', 코버스:'2', 티머니고속:'3'
	char	szFileName[128];		///< 파일이름
	int		nSize;					///< 파일 사이즈
} UI_SND_ADMIN_INQ_ISSUE_T, *PUI_SND_ADMIN_INQ_ISSUE_T;

// [관리자] 재발행 (534)
typedef struct
{
	char	pub_dt			[8];	///< 발행일자
	char	pub_shct_trml_cd[4];	///< 발행단축터미널코드
	char	pub_wnd_no		[2];	///< 발행창구번호
	char	pub_sno			[4];	///< 발행일련번호
	char	chBusDVS;				///< 시외:'1', 코버스:'2', 티머니고속:'3'

} UI_RESP_ADMIN_RE_ISSUE_T, *PUI_RESP_ADMIN_RE_ISSUE_T;

// [관리자] 시재마감 (540)
typedef struct
{
	char	cash_cls_dt		[8];	///< 시재 마감일자
	char	cash_cls_time	[6];	///< 시재 마감시간
	
} UI_RESP_CASH_CLOSE_T, *PUI_RESP_CASH_CLOSE_T;

// [관리자] 시재관리 - 시재 내역 (541)

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
	UI_CASH_T initInfo;				///< 초기 내역
	UI_CASH_T insInfo;				///< 입금 내역
	UI_CASH_T outInfo;				///< 출금 내역
	UI_CASH_T supplyInfo;			///< 보급 내역
	UI_CASH_T wdrawInfo;			///< 회수 내역
	UI_CASH_T noOutInfo;			///< 미출금 내역
	UI_CASH_T currInfo;				///< 현재 내역

} UI_SND_ADMIN_CASH_HISTO_T, *PUI_SND_ADMIN_CASH_HISTO_T;

// [관리자] 시재관리 - 시재 보급 (542)

typedef struct
{
	BYTE	byPymDvs;				///< 1:동전보급, 2:지폐보급	
	UI_CASH_T insInfo;				///< 보급

} UI_RESP_ADMIN_CASH_INS_T, *PUI_RESP_ADMIN_CASH_INS_T;

// [관리자] 시재관리 - 시재 출금 (543)

typedef struct
{
	BYTE	  byKind;				///< 0x01:출금, 0x02:시재회수
	UI_CASH_T cashInfo;				///< 출금

} UI_RESP_ADMIN_CASH_OUT_T, *PUI_RESP_ADMIN_CASH_OUT_T;

typedef struct
{
	BYTE	  byACK;				///< ACK/NAK
	BYTE	  byKind;				///< 0x01:출금, 0x02:시재회수
	UI_CASH_T cashInfo;				///< 출금

} UI_SND_ADMIN_CASH_OUT_T, *PUI_SND_ADMIN_CASH_OUT_T;

// [관리자] 시재관리 - 시재 입출내역 (544)

typedef struct
{
	BYTE	byACK;
	char	szFileName[128];
	DWORD	dwSize;
} UI_SND_ADMIN_INQ_CASH_INOUT_T, *PUI_SND_ADMIN_INQ_CASH_INOUT_T;

typedef struct
{
	char	byInqFlag;				///< 전체조회(0x00), 구간조회(0x01)
	char	szBeginDt[16];			///< 시작일시 (YYYYMMDD hhmmss)
	char	szEndDt[16];			///< 종료일시 (YYYYMMDD hhmmss)
} UI_RESP_ADMIN_INQ_CASH_INOUT_T, *PUI_RESP_ADMIN_INQ_CASH_INOUT_T;

// [관리자] 시재관리 - 미방출금 방출 요청 (547)
typedef struct
{
	WORD	wCount;					///< 미방출 건수
	char	szDateTime[14];			///< 미방출 일시 (YYYYMMDD hhmmss)
	char	szWndNo[4];				///< 창구번호
	char	byFlag;					///< 0x01
	WORD	w100;					///< 100원
	WORD	w500;					///< 100원
	WORD	w1k;					///< 1,000원
	WORD	w5k;					///< 5,000원
	WORD	w10k;					///< 10,000원
	WORD	w50k;					///< 50,000원
//	WORD	w100k;					///< 100,000원
	int		nTotMoney;				///< 총합계
} UI_RESP_NOOUT_CASH_REQ_T, *PUI_RESP_NOOUT_CASH_REQ_T;

// [관리자] 시재관리 - 미방출금 방출 결과 전송 (548)

typedef struct
{
	BYTE	byACK;
	WORD	wCount;					///< 미방출 건수
	char	szDateTime[14];			///< 미방출 일시 (YYYYMMDD hhmmss)
	char	szWndNo[4];				///< 창구번호
	char	byFlag;					///< 0x01
	WORD	w100;					///< 100원
	WORD	w500;					///< 100원
	WORD	w1k;					///< 1,000원
	WORD	w5k;					///< 5,000원
	WORD	w10k;					///< 10,000원
	WORD	w50k;					///< 50,000원
	//WORD	w100k;					///< 100,000원
	int		nTotMoney;				///< 총합계
} UI_SND_NOOUT_CASH_T, *PUI_SND_NOOUT_CASH_T;

// [관리자] 마감내역 요청 (551)
typedef struct
{
	BYTE	rfu	;			
} UI_RESP_REQ_CLOSE_HISTO_T, *PUI_RESP_REQ_CLOSE_HISTO_T;

typedef struct
{
	BYTE	ack	;					///< ack : 0x06, nack : 0x15			
} UI_SND_REQ_CLOSE_HISTO_T, *PUI_SND_REQ_CLOSE_HISTO_T;

// [관리자] 마감내역 전송 (552)
typedef struct
{
	char	tck_knd_cd[4+1];		///< 티켓종류
	int		nCount;					///< 티켓수량
	int		nTotalFare;				///< 티켓총금액
} CLOSE_TCK_ITEM_T, *PCLOSE_TCK_ITEM_T;

typedef struct
{
	BYTE	byACK;
	char	begin_dt[14];			///< 시작일시 (YYYYMMDD hhmmss)
	char	end_dt[14];				///< 종료일시 (YYYYMMDD hhmmss)

	int		n_exp_cash_count;		///< 고속버스 현금 레코드 갯수
	CLOSE_TCK_ITEM_T tExpCash[20];	///< 고속버스

	int		n_exp_card_count;		///< 고속버스 카드 레코드 갯수
	CLOSE_TCK_ITEM_T tExpCard[20];	///< 고속버스

	int		n_ccs_cash_count;		///< 시외버스 현금 레코드 갯수
	CLOSE_TCK_ITEM_T tCcCash[20];	///< 시외버스

	int		n_ccs_card_count;		///< 시외버스 카드 레코드 갯수
	CLOSE_TCK_ITEM_T tCcCard[20];	///< 시외버스

} UI_SND_CLOSE_HISTO_T, *PUI_SND_CLOSE_HISTO_T;


// [관리자] 거래내역 전송 (553)
typedef struct
{
	char	begin_dt	[8];		///< 거래일자 (YYYYMMDD)
} UI_RESP_CASH_IO_HISTO_T, *PUI_RESP_CASH_IO_HISTO_T;

typedef struct
{
	BYTE	byACK;					///< 결과 (ACK:0x06, NACK:0x15)
	char	szFileName[128];		///< 파일이름
	DWORD	dwFileSize;				///< 파일크기
} UI_SND_CASH_IO_HISTO_T, *PUI_SND_CASH_IO_HISTO_T;

// [관리자] 시재정보 프린트 요청 (554)
typedef struct
{
	BYTE	byPrtKind;				///< 프린트 종류 (0x01:현재시재, 0x02:일마감)

	/// 2021.03.25 add code
	BYTE	szDate[8];				///< 마감 시작일자
	BYTE	szTime[6];				///< 마감 시작시간
	/// ~2021.03.25 add code
} UI_RESP_554_DT, *PUI_RESP_554_DT;

typedef struct
{
	BYTE	byACK;					///< ACK/NAK
} UI_SND_554_DT, *PUI_SND_554_DT;

// [관리자] 시재정보 프린트 응답 (555)
typedef struct
{
	BYTE	byPrtKind;				///< 프린트 종류 (0x01:현재시재)
} UI_SND_555_DT, *PUI_SND_555_DT;

typedef struct
{
	BOOL	bResult;				///< Result
	int		nError;					///< 에러일경우, 에러코드
	BYTE	szData[100];			///< 데이타
	int		nDataLen;				///< 데이타길이
} UI_SND_Q_T, *PUI_SND_Q_T;

// [관리자] 테스트 승차권 출력 요청 (591)
typedef struct
{
	BYTE	byBusDVS;				///< type "1":이지 시외, "2": 코버스 고속, "3": 이지 고속

} UI_RESP_591_T, *PUI_RESP_591_T;


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
int UI_AddTrmlInfo(void);
int UI_AddFileInfo(int nNumber, int nID);

int UI_AddTckInhrNoInfo(void);

int UI_MrsCardReadInfo(char *pData, int nDataLen);
int UI_MrsTckIssueInfo(char *pData, int nDataLen);

int UI_CancRyChangeMoneyInfo(char *pData, int nDataLen);

int UI_AddFailInfo(WORD wCommand, char *pCode, char *pContents);
int UI_MrsIssueState(BYTE byACK, BYTE byState, int nSatNo);

int UI_ResWndKioskClose(WORD wCommand, char *pCode, char *pContents);	// 20210513 ADD

int UI_PbTckIssueState(char byACK, int nState, int n_sats_no);
int UI_TckIssueCompleteState(char byACK, char *card_aprv_no);

int UI_TkPubChangeMoneyInfo(char *pData, int nDataLen);

// 배차요금 조회 (0x0304)
int UI_TkPubAlcnFareInfo(BOOL bResult, int nError);

// 좌석선점 정보 (0x0306)
int UI_TkPubSatsInfo(int nAlcn, BOOL bResult, int nError);

// TMAX 승차권 발권 결과 정보 (0x0318)
int UI_TkPubTmaxResultInfo(BOOL bResult, int nError);

int UI_AddStaffResultInfo(BOOL bResult);
int UI_TkPubStaffSatsInfo(int nAlcn, BOOL bResult, int nError);


// 환불정보 전달
int UI_AddRefundInfo(BOOL bResult, BYTE chBusDVS);

// 미방출 결과 전송
int UI_NoOutCasgResultInfo(char *pData);

// 장비 마감내역 전송
int UI_InqDevCloseInfo(void);

/// 코버스 - 배차조회 결과
int UI_AddKobus_AlcnListInfo(BOOL bResult);

/// 코버스 - 좌석정보조회 결과
int UI_AddKobus_SatsListInfo(BOOL bResult);

/// 코버스 - 좌석선점조회 결과
int UI_AddKobus_SatsPcpyInfo(BOOL bResult);

/// 코버스 - 경유지정보 결과
int UI_AddKobus_ThruInfo(BOOL bResult);

/// 코버스 - 예매 조회 결과
int UI_AddKobus_ReadMrsInfo(BOOL bResult);

/// 코버스 - 예매 발행 결과
int UI_AddKobus_ResultPubMrsInfo(BOOL bResult);

/// 티머니 고속
int UI_AddQueTmExp(WORD wCommand, BOOL bResult);

int UI_Add_Q_SendData(WORD wCommand, BOOL bResult, int nError, BYTE *pData = NULL, int nDataLen = 0);
