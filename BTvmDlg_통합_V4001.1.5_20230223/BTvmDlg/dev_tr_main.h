// 
// 
// dev_tr_main.h : transact main 헤더 파일
//

#pragma once

#include "MySocket.h"
#include "MyLogFile.h"
#include "svr_ccbus_st.h"

//----------------------------------------------------------------------------------------------------------------------

enum __enTrasState__
{
	TR_BOOT_STATE		= 0		, 
	TR_INIT_STATE				,
	TR_READY_STATE				,

// 	TR_MRS_MAIN_STATE			,	// 예매발권 시작
// 	TR_MRS_INPUT_STATE			,	// 예매발권 - 예매번호, 핸드폰번호입력
// 	TR_MRS_REQ_LIST_STATE		,	// 예매발권 - List 내역
// 	TR_MRS_REQ_DETAIL_STATE		,	// 예매발권 - 상세내역
// 	TR_MRS_ISSUE_STATE			,	// 예매발권 - 발권
// 	TR_MRS_COMPLETE_STATE		,	// 예매발권 - 완료
// 	TR_MRS_CANCEL_STATE			,	// 예매발권 - 취소

	TR_MRS_MAIN_STATE			,	// 예매발권 시작
	TR_MRS_CARD_READ_STATE		,	// 예매발권 - 신용카드 읽기
	TR_MRS_INPUT_STATE			,	// 예매발권 - 예매번호, 핸드폰번호입력
	TR_MRS_LIST_STATE			,	// 예매발권 - [시외] List 내역
	TR_MRS_KOBUS_LIST_STATE		,	// 예매발권 - [코버스] List 내역
	TR_MRS_TMEXP_LIST_STATE		,	// 예매발권 - [티머니고속] 예매 내역 요청
	TR_MRS_TMEXP_KTC_LIST_STATE	,	// 예매발권 - [티머니고속] KTC 예매 내역 요청

	TR_MRS_DETAIL_STATE			,	// 예매발권 - 상세내역
	TR_MRS_ISSUE_STATE			,	// 예매발권 - [시외] 예매발권
	TR_MRS_KOBUS_ISSUE_STATE	,	// 예매발권 - [코버스] TMAX 예매발권
	TR_MRS_TMEXP_ISSUE_STATE	,	// 예매발권 - [티머니고속] TMAX 예매발권
	TR_MRS_TMEXP_MOBILE_ISSUE_STATE	,	// 예매발권 - [티머니고속] TMAX 모바일티켓 예매발권

	TR_MRS_KOBUS_TCK_PRT_STATE	,	// 예매발권 - [코버스] 승차권 프린트
	TR_MRS_TMEXP_TCK_PRT_STATE	,	// 예매발권 - [티머니고속] 승차권 프린트
	TR_MRS_COMPLETE_STATE		,	// 예매발권 - 완료
	TR_MRS_CANCEL_STATE			,	// 예매발권 - 취소

	TR_CANCRY_MAIN_STATE		,	// 환불 - 시작
	TR_CANCRY_TCK_READ			,	// 환불 - 승차권 읽기
	TR_CANCRY_FARE_STATE		,	// 환불 - 카드 또는 현금 환불 처리
	TR_CANCRY_COMPLETE_STATE	,	// 환불 - 완료
	TR_CANCRY_CANCEL_STATE		,	// 환불 - 취소

	TR_PBTCK_MAIN_STATE			,	// 현장발권 - 시작
	
	TR_PBTCK_REQ_LIST			,	// 현장발권 - [시외] 배차조회 
	TR_PBTCK_KOBUS_REQ_LIST		,	// 현장발권 - [코버스] 배차조회 
	TR_PBTCK_TMEXP_REQ_LIST		,	// 현장발권 - [티머니고속] 배차조회 

	TR_PBTCK_KOBUS_REQ_SATS		,	// 현장발권 - 좌석정보 조회 (코버스)
	TR_PBTCK_KOBUS_REQ_SATS_PCPY,	// 현장발권 - 좌석선점 요청 (코버스)
	TR_PBTCK_KOBUS_REQ_THRU		,	// 현장발권 - 경유지 요청 (코버스)
	TR_PBTCK_TMEXP_REQ_SATS		,	// 현장발권 - 티켓 요금정보 조회 (티머니고속)
	TR_PBTCK_TMEXP_REQ_SATS_PCPY,	// 현장발권 - 좌석선점 요청 (티머니고속)
	TR_PBTCK_TMEXP_REQ_THRU		,	// 현장발권 - 경유지 요청 (티머니고속)
	
	TR_PBTCK_LIST_SEL			,	// 현장발권 - 배차 리스트 선택완료

	TR_PBTCK_STAFF_REQ			,	// 현장발권 - [시외_인천공항] 상주직원 조회 요청
	TR_PBTCK_STAFF_CD_MOD_FARE	,	// 현장발권 - [시외_인천공항] 상주직원_카드_요금변경정보 조회 요청
	TR_PBTCK_RF_PAYMENT			,	// 현장발권 - [시외] RF 선불카드 결제 요청

	TR_PBTCK_SEAT_SEL			,	// 현장발권 - 좌석 정보 선택완료
	TR_PBTCK_CORONA_INPUT		,	// 현장발권 - 개인정보 입력
	TR_PBTCK_PYM_DVS_SEL		,	// 현장발권 - 결제수단 선택완료
	TR_PBTCK_INS_BILL			,	// 현장발권 - 지폐투입
	TR_PBTCK_STOP_BILL			,	// 현장발권 - 지폐투입 중지
	TR_PBTCK_CSRC_READ			,	// 현장발권 - 현금영수증 카드 읽기 시작/종료
	TR_PBTCK_CSRC_INPUT			,	// 현장발권 - 현금영수증 번호 입력

	TR_PBTCK_REQ_QRMDPCPYSATS	,	// 현장발권 - [시외] QR 좌석 선점 시간변경 // 20221205 ADD

	TR_PBTCK_TMAX_ISSUE			,	// 현장발권 - [시외] 서버에서 발권 중
	TR_PBTCK_KOBUS_TMAX_ISSUE	,	// 현장발권 - [코버스] 서버에서 발권 중
	TR_PBTCK_TMEXP_TMAX_ISSUE	,	// 현장발권 - [티머니고속] 서버에서 발권 중
	
	TR_PBTCK_TCK_ISSUE			,	// 현장발권 - [시외] 승차권 발권
	TR_PBTCK_KOBUS_TCK_ISSUE	,	// 현장발권 - [코버스] 승차권 발권
	TR_PBTCK_TMEXP_TCK_ISSUE	,	// 현장발권 - [티머니고속] 승차권 발권

	TR_PBTCK_CHANGE_MONEY		,	// 현장발권 - 승차권 발권
	TR_PBTCK_CARD_READ			,	// 현장발권 - 신용카드 읽기 시작/종료
	TR_PBTCK_COMPLETE_STATE		,	// 현장발권 - 완료
	TR_PBTCK_CANCEL_STATE		,	// 현장발권 - 취소

	TR_THRML_ISSUE_STATE		,
	TR_REFUND_STATE				,

	TR_ERROR_STATE				,	// 점검중 화면

	TR_ADMIN_MAIN				,	// 관리자 메인
	TR_ADMIN_LOGIN				,	// 관리자 - Login
	TR_ADMIN_LOGOUT				,	// 관리자 - LogOut
	TR_ADMIN_NO_OUT_CASH		,	// 관리자 - 미방출 처리
	TR_ADMIN_RECEIPT_PRINT		,	// 관리자 - 영수증 프린트

	TR_ADMIN_MAX				,	// 
};

enum _en_TransMem_
{
	CFG_ALL			=	0	,	///> 전체
	CFG_RSP_ALCN_DATA		,	///> 배차_resp 조회 데이타
	CFG_RSP_ALCN_FEE_DATA	,	///> 배차 요금_resp 조회 데이타
	CFG_SND_PCPYSATS_DATA	,	///> 좌석 선점 데이타
	CFG_RSP_PCPYSATS_DATA	,	///> 좌석 선점/해제_resp 데이타
	CFG_RSP_TRMENCKEY_DATA	,	///> 암호화키 데이타
	CFG_REQ_PUBTCK_DATA		,	///> 승차권 발행 (전송) 데이타

	CFG_RSP_MRS_ISSUE_DATA	,	///> 예매발권 데이타_resp 데이타

	CFG_UI_VERSION			,	///> 
	CFG_ETC_VERSION			,	///> 
	CFG_MOBILE_NO			,	///> 
	CFG_MRS_NO				,	///> 
	
	CFG_PBTCK_TOTAL_MONEY	,	///> 현장발권 - 현금 총 발권금액
	CFG_PBTCK_TOTAL_COUNT	,	///> 현장발권 - 현금 총 발권매수
	CFG_PBTCK_PAY_TYPE		,	///> 현장발권 - 결제수단
	CFG_PBTCK_CHG_MONEY		,	///> 현장발권 - 거스름돈
};

//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

// queue data
typedef struct 
{
	int 	nCommand;
#define TR_CMD_STATE			1
#define TR_CMD_STEP				2
#define TR_CMD_PREV_STEP		4
#define TR_CMD_ALL				(TR_CMD_STATE | TR_CMD_STEP | TR_CMD_PREV_STEP)
	int		nState;
	int		nStep;
	int		nPrevStep;
} TR_QUE_DATA_T, *PTR_QUE_DATA_T;

// 바코드 포맷
typedef struct 
{
	char pub_dt					[8];	 /// 발행일자
	char pub_shct_trml_cd		[4];	 /// 발행단축터미널코드
	char pub_wnd_no				[2];	 /// 발행창구번호
	char pub_sno				[4];	 /// 발행일련번호
	char secu_code				[2];	 /// 체크비트
} FMT_BARCODE_T, *PFMT_BARCODE_T;

typedef struct 
{
	char pub_dt					[8];	 /// 발행일자
	char pub_shct_trml_cd		[3];	 /// 발행단축터미널코드
	char pub_wnd_no				[2];	 /// 발행창구번호
	char pub_sno				[4];	 /// 발행일련번호

	char depr_trml_no			[3];	 /// 출발 터미널코드
	char arvl_trml_no			[3];	 /// 도착 터미널코드
	char sats_no				[2];	 /// 좌석번호
} FMT_QRCODE_T, *PFMT_QRCODE_T;



#pragma pack()

//----------------------------------------------------------------------------------------------------------------------

class CTrLogFile : public CMyLogFile 
{
public:
	CTrLogFile(void) {};
	virtual ~CTrLogFile(void) {};
};


//----------------------------------------------------------------------------------------------------------------------

extern CTrLogFile		clsTrLog;
//#define TR_LOG_OUT( ... )		{ clsTrLog.LogOut( __VA_ARGS__ );  }
#define TR_LOG_OUT(fmt, ...)	{ clsTrLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define TR_LOG_HEXA(x,y,z)		{ clsTrLog.HexaDump(x, y, z); }

//----------------------------------------------------------------------------------------------------------------------

int Transact_SetStep(int nValue);
int Transact_GetStep(void);

int Transact_SetPrevStep(int nValue);
int Transact_SetState(int nState, int nStep);
int Transact_GetState(void);

int Transact_SetData(DWORD dwFldKey, char *pData, int nDataLen);
char *Transact_GetData(DWORD dwFldKey);
void *Transact_GetAllData(void);


int Transact_LogInitialize(void);
int Transact_Initialize(void);
int Transact_Terminate(void);

int KTC_MemClear(void *pData, int nSize);
void MakeDepartureDateTime(char *pDT, char *pTime, char *retDT, char *retTM);

int TR_AddQueData(int nCommand, int nState, int nStep, int nPrevStep);
