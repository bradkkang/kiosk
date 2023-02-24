// MyDataAccum.h : File Accum function
// 
// 
//

#pragma once

#include "MyDefine.h"
#include "MyLogFile.h"

//----------------------------------------------------------------------------------------------------------------------

#define ACCUM_MAGIC_CODE	0x55667788
#define MAX_SAVE_DATE		30

enum __enTicketDisc__ 
{
	TCK_DISC_00	= 0,
	TCK_DISC_01,
	TCK_DISC_02,
	TCK_DISC_03,
	TCK_DISC_04,
	TCK_DISC_05,
	TCK_DISC_06,
	TCK_DISC_07,
	TCK_DISC_MAX,
};


enum __enPayMethod__ 
{
	PAY_CASH	= 0,				///> 현금
	PAY_CREDIT,						///> 신용카드
};

enum __enOperMode__ 
{
	 STAFF_MODE				= 0	,
	 TEST_MODE					,
};

enum __enAccumMode__ 
{
	ACCUM_MODE_OPER_MANUAL	= 0	,		///> 역무원 모드 - 수시마감
	ACCUM_MODE_OPER_AUTO,				///> 역무원 모드 - 자동마감
	ACCUM_MODE_MAX,		    			///> 모드 최대 갯수
};

//----------------------------------------------------------------------------------------------------------------------

enum __enAccumDataType__
{
	ACC_ALL						= 1			,

	ACC_COIN_SUPPLY				= 0x1111	,
	ACC_COIN_MINUS							,
	ACC_COIN_ASSIGN							,
	ACC_COIN_CLEAR							,

	ACC_BILL_INSERT				= 0x1200	,
	ACC_BILL_MINUS							,
	ACC_BILL_WITHDRAW						,
	ACC_BILL_ASSIGN							,
	ACC_BILL_CLEAR							,
	ACC_BILL_SUPPLY							,

	ACC_BILL_DISPENSER_ASSIGN	= 0x1300	,
	ACC_BILL_DISPENSER_CLEAR				,

	ACC_TICKET_PUB_ISSUE		= 0x1400	,	///> 현장발권
	ACC_TICKET_MRNP_ISSUE					,	///> 예매발권
	ACC_TICKET_REFUND						,	///> 환불
	ACC_TICKET_PUB_REV1_ISSUE				,	///> 현장발권1 : 2020/06/03 add
	ACC_TICKET_MRNP_REV1_ISSUE				,	///> 예매발권1 : 2020/06/03 add
	ACC_TICKET_PUB_TEST_ISSUE				,	///> 현장테스트발권 : 2022-01-04 add

	ACC_KOBUS_TICKET_INFO		= 0x1444	,	///> 코버스 발권 일시

	ACC_TICKET_MINUS			= 0x1450	,
	ACC_TICKET_INSERT						,
	ACC_TICKET_ASSIGN						,
	ACC_TICKET_CLEAR						,

	ACC_TICKET_SEQ_PLUS			= 0x1500	,	///> 승차권 일련번호 증가
	ACC_TICKET_SEQ_ASSIGN					,	///> 승차권 일련번호 assign
	ACC_TICKET_SEQ_CLEAR					,	///> 승차권 일련번호 초기화
	ACC_TICKET_ADD_SEQ_ASSIGN				,	///> 추가 티켓 고유일련번호


	ACC_TICKETBOX_INSERT		= 0x1600	,
	ACC_TICKETBOX_WITHDRAW					,

	ACC_UNPAID					= 0x1700	,	///> 미지불
	ACC_UNPAID_MINUS						,	///> 미지불 처리완료
	ACC_UNPAID_CLEAR						,	///> 미지불 데이타 초기화

	ACC_CASH_CLOSE				= 0x1800	,	///> 시재마감

	ACC_DAY_CLOSE				= 0x1900	,	///> 일마감

// 20210513 ADD
	ACC_DAY_CLOSE_CCS			= 0x2001	,	///> 일마감 시외
	ACC_DAY_CLOSE_KOBUS			= 0x2002	,	///> 일마감 KOBUS고속
	ACC_DAY_CLOSE_TMONEY		= 0x2003	,	///> 일마감 티머니고속
// 20210513 ~ADD

	ACC_DATA_CLEAR							, 
};

//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

#define CHG_ACCUM_VERSION	0x11001100

typedef struct 
{
	DWORD			dwVersion;		
	DWORD			dwSerial;		
	DWORD			dwMagic;		
	DWORD			dwTick;			
	DWORD			dwCount;		
	DWORD			dwSend;			
	DWORD			dwSeq;			
} FILE_ACC_HEADER_T, *PFILE_ACC_HEADER_T;

typedef struct 
{
	/// 2021.03.26 add code
	DWORD			dwVersion;		
	/// ~2021.03.26 add code
	char			date[50];		
	DWORD			dwSerial;			
	DWORD			dwTick;			
	DWORD			dwCount;			
	DWORD			dwSend;		
	DWORD			dwSeq;
} ACC_FILE_ENTRY_T, *PACC_FILE_ENTRY_T;

typedef struct 
{
	DWORD		dwTick;
	int			nKind;
	int			nSize;
} ACC_HEADER_T, *PACC_HEADER_T;

//----------------------------------------------------------------------------------------------------------------------

typedef struct 
{
	DWORD		dwTick;				///> 발생일시
	WORD		wKind;				///> 종류
} ACC_BASE_T, *PACC_BASE_T;

typedef struct 
{
	int			n10;				///> 10원
	int			n50;				///> 50원
	int			n100;				///> 100원
	int			n500;				///> 500원
} ACOIN_T, *PACOIN_T;

typedef struct 
{
	int			n1k;				///> 1,000원
	int			n5k;				///> 5,000원
	int			n10k;				///> 10,000원
	int			n50k;				///> 50,000원
} ABILL_T, *PABILL_T;

typedef struct 
{
	ACC_BASE_T	Base;
	ACOIN_T		Coin;
} ACC_COIN_DATA_T, *PACC_COIN_DATA_T;

typedef struct 
{
	ACC_BASE_T	Base;
	ABILL_T		Bill;				///> 정상
	ABILL_T		ErrBill;			///> 불량지폐함
} ACC_BILL_DATA_T, *PACC_BILL_DATA_T;

typedef struct 
{
	ACC_BASE_T	Base;
	int			nSvrKind;			///> 서버종류 (시외/코버스/티머니고속)
	int			nCount;				///> 정상
} ACC_TICKET_DATA_T, *PACC_TICKET_DATA_T;

typedef struct 
{
	ACC_BASE_T	Base;
	char kobus_tak_stt_dt	[8+1]	;	///< 작업시작일자	
	char kobus_stt_time		[6+1]	;	///< 시작시각	
} ACC_KOBUS_TCK_INFO_T, *PACC_KOBUS_TCK_INFO_T;

typedef struct 
{
	ACC_BASE_T	Base;
	int			nSvrKind;			///> 시외/코버스/티머니고속
	int			nPayDvs;			///> 신용카드:0x00, 현금:0x01
	char		bus_tck_knd_cd[4+1];///> 버스 티켓 종류 코드
	int			nCount;				///> 정상
	int			nFare;				///> 금액
} ACC_TICKET_WORK_T, *PACC_TICKET_WORK_T;

/// 2020.06.03 add code
typedef struct 
{
	ACC_BASE_T	Base;
	int			nSvrKind;			///> 시외/코버스/티머니고속
	int			nPayDvs;			///> 신용카드:0x00, 현금:0x01
	char		bus_tck_knd_cd[4+1];///> 버스 티켓 종류 코드
	int			nCount;				///> 정상
	int			nFare;				///> 금액
	BYTE		byPaper;			///> 승차권 종이 종류
	BYTE		szRfu[3];			///> 예비1
	BYTE		szRfu1[20];			///> 예비2
} ACC_TICKET_WORK_REV1_T, *PACC_TICKET_WORK_REV1_T;
/// ~2020.06.03 add code

typedef struct 
{
	ACC_BASE_T	Base;
	int			nCount;				///> 정상
} ACC_CLOSE_DATA_T, *PACC_CLOSE_DATA_T;

typedef struct 
{
	ACC_BASE_T	Base;
	int			n10;				///> 
	int			n50;				///> 
	int			n100;				///> 
	int			n500;				///> 

	int			n1k;				///> 
	int			n5k;				///> 
	int			n10k;				///> 
	int			n50k;				///> 
} ACC_NOPAY_DATA_T, *PACC_NOPAY_DATA_T;

//----------------------------------------------------------------------------------------------------------------------
typedef struct 
{
	int			nCount;				///> 매수
	DWORD		dwMoney;			///> 금액
} DM_SUM_T, *PDM_SUM_T;

typedef struct 
{
	int			n10;				///> 10원
	int			n50;				///> 50원
	int			n100;				///> 100원
	int			n500;				///> 500원
} DM_COIN_T, *PDM_COIN_T;

typedef struct 
{
	int			n100;				///> 100원
	int			n500;				///> 500원
} DM_SHT_COIN_T, *PDM_SHT_COIN_T;

typedef struct 
{
	int			n1k;				///> 1,000원
	int			n5k;				///> 5,000원
	int			n10k;				///> 10,000원
	int			n50k;				///> 50,000원
} DM_BILL_T, *PDM_BILL_T;

typedef struct 
{
	ABILL_T		Casst;				///> 카세트 지폐 정보
	ABILL_T		ErrCasst;			///> 불량 지폐 정보
} DM_BILL_DISPENSER_T, *PDM_BILL_DISPENSER_T;

// 승차권 용지
typedef struct 
{
	int			nCount;							///> 승차권 수량
	
 	int			nPubIssueCount;					///> 현장발권 수량
 	int			nMrnpIssueCount;				///> 예매발권 수량
 	int			nRefundIssueCount;				///> 환불 수량

	int			n_bus_tck_inhr_no;				///> 버스 티켓 고유번호
	int			n_add_bus_tck_inhr_no;			///> 추가 버스 티켓 고유번호

	char		Rfu[100];
} DM_TICKET_T, *PDM_TICKET_T;

// 승차권 수집함 (환불시)
typedef struct 
{
	int			nCount;								///> 승차권 수집함
	char		Rfu[100];
} DM_TICKETBOX_T, *PDM_TICKETBOX_T;

// 승차권 업무 회계
typedef struct 
{
#define IDX_ACC_CARD	0							/// IC카드
#define IDX_ACC_CASH	1							/// 현금
#define IDX_ACC_RF		2							/// RF카드

	ACCOUNT_FIELD_T tPubTck[2];						///> [IC카드,현금] 현장발권 매수 & 금액
	ACCOUNT_FIELD_T	tMrnp[2];						///> [IC카드,현금] 예매발권 매수 & 금액
	ACCOUNT_FIELD_T tRefund[2];						///> [IC카드,현금] 환불 매수 & 금액
	
	/// 2020.02.13 insert code : RF 결제
	ACCOUNT_FIELD_T	tPubRFTck;						///> [RF카드] 현장발권 매수 & 금액
	ACCOUNT_FIELD_T	tMrnpRF;						///> [RF카드] 예매발권 매수 & 금액
	ACCOUNT_FIELD_T	tRefundRF;						///> [RF카드] 환불 매수 & 금액
	/// ~2020.02.13 insert code : RF 결제

	char			Rfu[100 - 24];
} DM_TICKET_WORK_T, *PDM_TICKET_WORK_T;

//----------------------------------------------------------------------------------------------------------------------

typedef struct 
{
	DM_COIN_T			initCoin;					///> 초기 동전 수량
	DM_COIN_T			supplyCoin;					///> 보급 동전 수량 (=동전방출기)
	DM_COIN_T			outCoin;					///> 방출 동전 수량 (=동전방출기)
	DM_COIN_T			noCoin;						///> 미방출 동전 수량 (=동전방출기)

	DM_BILL_T			initBill;					///> 초기 지폐 수량 (=지폐입금기)
	DM_BILL_T			inBill;						///> 투입 지폐 수량 (=지폐입금기)
	DM_BILL_T			outBill;					///> 방출 지폐 수량 (=지폐방출기 스태커)
	DM_BILL_T			supplyBill;					///> 보급 지폐 수량 (=지폐방출기 스태커)
	DM_BILL_T			wdrawBill;					///> 회수 지폐 수량 (=지폐함)
	DM_BILL_T			noBill;						///> 미방출 지폐 수량 (=지폐방출기 스태커)

	char				szRFU[100];
} DM_ADMIN_CLOST_T, *PDM_ADMIN_CLOST_T;

typedef struct 
{
	DM_COIN_T			Coin;								///> 현재 동전 수량
	DM_BILL_T			BillBox;							///> 현재 지폐입금 수량
	DM_BILL_DISPENSER_T	BillDispenser;						///> 현재 지폐방출기 수량
	
	DM_TICKET_T			phyTicket;							///> 현재 승차권 수량 & 일련번호

	DM_TICKET_T			ccsTicket;							///> [시외] 현재 승차권 수량 & 일련번호
	DM_TICKET_T			expTicket;							///> [고속] 현재 승차권 수량 & 일련번호

	DM_TICKETBOX_T		phyTicketBox;						///> 현재 승차권 수집함 수량

//	DM_TICKET_WORK_T	TicketWork;							///> 현재 현장발권/예매발권/환불 매수,금액
	DM_TICKET_WORK_T	ccsTicketWork[CCS_IDX_TCK_MAX];		///> [시외] 현재 현장발권/예매발권/환불 매수,금액
	DM_TICKET_WORK_T	expTicketWork[TMEXP_IDX_TCK_MAX];	///> [코버스/티머니고속] 현재 현장발권/예매발권/환불 매수,금액

	DM_COIN_T			noCoin;								///> 현재 미방출 동전 수량
	DM_BILL_T			noBill;								///> 현재 미방출 지폐 수량

} DM_CURRENT_T, *PDM_CURRENT_T;

typedef struct 
{
	BYTE				szRfu1[110];				///< 

	DM_CURRENT_T		Prev;						///< 직전 정보
	DM_CURRENT_T		Curr;						///< 현재 정보

	DM_ADMIN_CLOST_T	Close;						///< 

	/// 2021.03.25 add code

	char cash_cls_stt_dt [14+1]	;					///> 시재마감 시작일시
	char day_cls_stt_dt [14+1]	;					///> 일마감 시작일시

	BYTE				szRfu2[54];					///> 
//	BYTE				szRfu2[84];					///> 
	/// ~2021.03.25 add code

	char tak_stt_dt		[8+1]	;					///> 작업시작일자	
	char stt_time		[6+1]	;					///> 시작시각	


} FILE_ACCUM_T, *PFILE_ACCUM_T;



/// 

typedef struct 
{
	DM_TICKET_WORK_T	ccsTicketWork[CCS_IDX_TCK_MAX];				///> [시외] 현재 현장발권/예매발권/환불 매수,금액
	DM_TICKET_WORK_T	expTicketWork[TMEXP_IDX_TCK_MAX];			///> [코버스/티머니고속] 현재 현장발권/예매발권/환불 매수,금액

} DM_MNU_CLOSE_T, *PDM_MNU_CLOSE_T;

typedef struct 
{
	BYTE				szRfu1[110];				///> 

	DM_CURRENT_T		Prev;						///> 직전 정보
	DM_CURRENT_T		Curr;						///> 현재 정보

	DM_ADMIN_CLOST_T	Close;						///> 수동마감 정보 - 시재

	/// 2021.03.25 add code

	char cash_cls_stt_dt [14+1]	;					///> 시재마감 시작일시
	char day_cls_stt_dt [14+1]	;					///> 일마감 시작일시

	// 20210513 ADD
	char sCls_Ccs_dt [14+1]	;						///> 시외 창구마감 발생일시
	char sCls_Kobus_dt [14+1]	;					///> KOBUS고속 창구마감 발생일시
	char sCls_Tmoney_dt [14+1]	;					///> 티머니고속 창구마감 발생일시
	// 20210513 ~ADD

	BYTE				szRfu2[22+32];				///> 
	//	BYTE				szRfu2[84];				///> 
	/// ~2021.03.25 add code

	char tak_stt_dt		[8+1]	;					///> 작업시작일자	
	char stt_time		[6+1]	;					///> 시작시각	

	/// 2021.03.26 이후부터 추가한 정보들..
	DM_MNU_CLOSE_T		MnuCls;						///> 수동마감 정보 - 승차권

	BYTE				szRfu3[1024 * 2];			///> 

} FILE_ACCUM_N1010_T, *PFILE_ACCUM_N1010_T;

#pragma pack()

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CAccumLogFile
 * @details		데이타 로그파일 class
 */
class CAccumLogFile : public CMyLogFile 
{
public:
	CAccumLogFile(void) {};
	virtual ~CAccumLogFile(void) {};
};

//----------------------------------------------------------------------------------------------------------------------

int InitAccumLogFile(void);

int AddAccumulateData(int nKind, char *pData, int bWrite);
PFILE_ACCUM_N1010_T GetAccumulateData(void);
//PACC_WND_CLOSE_T GetAccumDataWndClose(void);
int CreateAccumData(void);
void SetAccumPath(CString &strPath);
int InitAccumData(void);
int TermAccumData(void);
void PrintAccumData(BOOL bPrint);

int AddAccumCoinData(WORD wKind, int n100, int n500);
int AddAccumBillData(WORD wKind, int n1k, int n5k, int n10k, int n50k);
int AddAccumTicketData(WORD wKind, int nCount);
int AddAccumUnPaidData(WORD wKind, int n100, int n500, int n1k, int n10k);
int AddAccumCashCloseData(void);
int AddAccumWndCloseData(void);
// 20210513 ADD
int AddAccumWndClsCcsData(void);
int AddAccumWndClsKobusData(void);
int AddAccumWndClsTmoneyData(void);
// 20210513 ~ADD
int AddAccumBusTckInhrNo(int nKind, int nSvrKind, int n_bus_tck_inhr_no);
int AddAccumTicketWork(int nSvrKind, int nPayDvs, char *tck_kind, int nKind, int nCount, int nFare);
int AddAccumTicketWork_Rev1(int nSvrKind, int nPayDvs, char *tck_kind, int nKind, int nCount, int nFare, BYTE byPaper);
int AddAccumTicketWork_Rev2(int nSvrKind, int nPayDvs, char *tck_kind, int nKind, int nCount);		// 20220104 add

int AddAccumKobusTicketInfo(char *pDate, char *pTime);

int CheckUnpaidData(void);
int GetAccumINHR_No(int nSvrKind);
