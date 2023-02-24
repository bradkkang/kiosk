// data_main.h : 거래 정보 파일 처리
// 
// 
//

#pragma once

#include "MyDefine.h"
#include "MyDataTr.h"
#include "MyDataAccum.h"

//----------------------------------------------------------------------------------------------------------------------


/// CashTR Directory
class CTrCashFile : public CTrDataMng 
{
public:
	CTrCashFile(void) {};
	virtual ~CTrCashFile(void) {};
};

/// SmsTR Directory
class CTrSmsFile : public CTrDataMng 
{
public:
	CTrSmsFile(void) {};
	virtual ~CTrSmsFile(void) {};
};

/// CashCls Directory
class CTrCashCloseFile : public CTrDataMng 
{
public:
	CTrCashCloseFile(void) {};
	virtual ~CTrCashCloseFile(void) {};
};

/// DayCls Directory
class CTrDayCloseFile : public CTrDataMng 
{
public:
	CTrDayCloseFile(void) {};
	virtual ~CTrDayCloseFile(void) {};
};

//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

typedef struct  
{
	WORD	w100;				///< 100원
	WORD	w500;				///< 500원
} COIN_DT_T, *PCOIN_DT_T;

typedef struct  
{
	WORD	w1k;				///< 1,000원
	WORD	w5k;				///< 5,000원
	WORD	w10k;				///< 10,000원
	WORD	w50k;				///< 50,000원
} BILL_DT_T, *PBILL_DT_T;

typedef struct  
{
	char	szDateTm[14];		///< 발생일시
	char	szWndNo[4];			///< 창구번호
	char	chFlag;				///< 출금(1),입금(2),보급(3),회수(4),미방출(5)
#define DT_FLAG_OUT_MONEY		1
#define DT_FLAG_IN_MONEY		2
#define DT_FLAG_SUPPLY_MONEY	3
#define DT_FLAG_WTHDRAW_MONEY	4
#define DT_FLAG_UNPAID			5
	WORD	w100;				///< 100원
	WORD	w500;				///< 500원
	WORD	w1k;				///< 1,000원
	WORD	w5k;				///< 5,000원
	WORD	w10k;				///< 10,000원
	WORD	w50k;				///< 50,000원
	int		nSum;				///< 합계
	char	Rfu[100];			///< 예비

} CASH_TR_DATA_T, *PCASH_TR_DATA_T;

typedef struct
{
	WORD	w100;		
	WORD	w500;

	WORD	w1k;
	WORD	w5k;
	WORD	w10k;
	WORD	w50k;
} CLOSE_ITEM_T, *PCLOSE_ITEM_T;

/// 시재마감 내역
typedef struct  
{
	/*** org
	char szEndDateTm[14];			///> 마감발생일시(=마감종료일시)
	char szWndNo[4];				///> 창구번호
	CLOSE_ITEM_T initInfo;			///> 초기 내역
	CLOSE_ITEM_T insInfo;			///> 입금 내역
	CLOSE_ITEM_T outInfo;			///> 출금 내역
	CLOSE_ITEM_T supplyInfo;		///> 보급 내역
	CLOSE_ITEM_T wdrawInfo;			///> 회수 내역
	CLOSE_ITEM_T noOutInfo;			///> 미출금 내역
	CLOSE_ITEM_T currInfo;			///> 현재 내역

	/// 2021.03.25 add code
	char szBegDateTm[14];			///> 마감발생일시(=마감종료일시)
	char	szRfu[86];				///< RFU
	//char	szRfu[100];				///< RFU
	/// ~2021.03.25 add code
	***/
	char szBegDateTm[14];			///> 마감시작일시
	char szEndDateTm[14];			///> 마감종료일시(=발생일시)
	char szWndNo[4];				///> 창구번호

	CLOSE_ITEM_T initInfo;			///> 초기 내역
	CLOSE_ITEM_T insInfo;			///> 입금 내역
	CLOSE_ITEM_T outInfo;			///> 출금 내역
	CLOSE_ITEM_T supplyInfo;		///> 보급 내역
	CLOSE_ITEM_T wdrawInfo;			///> 회수 내역
	CLOSE_ITEM_T noOutInfo;			///> 미출금 내역
	CLOSE_ITEM_T currInfo;			///> 현재 내역

	DM_TICKET_WORK_T	ccsTicketWork[CCS_IDX_TCK_MAX];		///> [시외] 현재 현장발권/예매발권/환불 매수,금액
	DM_TICKET_WORK_T	expTicketWork[TMEXP_IDX_TCK_MAX];	///> [코버스/티머니고속] 현재 현장발권/예매발권/환불 매수,금액
	 
// 	ACCOUNT_FIELD_T ccsPubTckSum;	///> [시외] 승차권 카드 발권 (매수/금액)
// 	ACCOUNT_FIELD_T expPubTckSum;	///> [고속] 승차권 카드 발권 (매수/금액)
// 
// 	ACCOUNT_FIELD_T ccsRefundSum;	///> [시외] 승차권 카드 환불 (매수/금액)
// 	ACCOUNT_FIELD_T expRefundSum;	///> [고속] 승차권 카드 환불 (매수/금액)

	char	szRfu[1024];			///< RFU
} CLOSE_TR_DATA_T, *PCLOSE_TR_DATA_T;

/// 일마감 내역
typedef struct  
{
	char szBegDateTm[14];			///> 마감시작일시
	char szEndDateTm[14];			///> 마감종료일시(=발생일시)
	char szWndNo[4];				///> 창구번호
	
	CLOSE_ITEM_T initInfo;			///> 초기 내역
	CLOSE_ITEM_T insInfo;			///> 입금 내역
	CLOSE_ITEM_T outInfo;			///> 출금 내역
	CLOSE_ITEM_T supplyInfo;		///> 보급 내역
	CLOSE_ITEM_T wdrawInfo;			///> 회수 내역
	CLOSE_ITEM_T noOutInfo;			///> 미출금 내역
	CLOSE_ITEM_T currInfo;			///> 현재 내역

	DM_TICKET_WORK_T	ccsTicketWork[CCS_IDX_TCK_MAX];		///> [시외] 현재 현장발권/예매발권/환불 매수,금액
	DM_TICKET_WORK_T	expTicketWork[TMEXP_IDX_TCK_MAX];	///> [코버스/티머니고속] 현재 현장발권/예매발권/환불 매수,금액

	char	szRfu[1024];			///< RFU
} DAY_CLOSE_TR_DATA_T, *PDAY_CLOSE_TR_DATA_T;


typedef struct  
{
	char	tr_dvs			[1]	;	///< 거래구분 (현장발권:1, 예매발권:2, 환불:3)
	char	pay_dvs			[1]	;	///< 지불구분 (현금:1, 카드:2, RF;3, 그외:4)
	char	depr_trml_cd	[10];	///< 출발지 코드
	char	depr_trml_nm	[40];	///< 출발지 이름
	char	arvl_trml_cd	[10];	///< 도착지 코드
	char	arvl_trml_nm	[40];	///< 도착지 이름

	char	bus_cls_cd		[3]	;	///< 버스등급코드
	char	bus_tck_knd_cd	[4]	;	///< 티켓종류코드 

	char	depr_dt			[8]	;	///< 출발일자
	char	depr_time		[6]	;	///< 출발시각
	char	bus_cacm_cd		[4]	;	///< 버스운수사코드
	char	sats_no			[4]	;	///< 좌석번호	(number)

	char	pub_dt			[8]	;	///< 발권일자
	char	pub_time		[6]	;	///< 발권시각

	char	bar_qr_data		[40];	///< bar_code or qr_code 

	COIN_DT_T outCoin;		;	///< 동전 방출 정보
	BILL_DT_T outBill;		;	///< 지폐 방출 정보
	BILL_DT_T inBill;		;	///< 지폐 투입 정보

	COIN_DT_T noutCoin;	;	///< 동전 미방출 정보
	BILL_DT_T noutBill;	;	///< 지폐 미방출 정보
} AAA;

/// 거래내역 (관제시스템용)
typedef struct  
{
	char	BusDvs			[1]	;	///> 버스 서버 구분 (시외:0x30,코버스:0x31,티머니고속:0x32)
	char	szHappenDt		[14];	///> 발생일시
	char	TrDvs			[1]	;	///> "A":구매,현금자진발급, "B":구매,현금영수증, "C":구매,신용카드, "8":환불,현금, "9":환불,카드, "Z":환불,기타
	int		nFare				;	///> 거래금액
	int		nCount				;	///> 발권수량
// 2021.02.05 버퍼 사이즈 up (400 -> 1024)
//	char	szTicketInfo	[400];	///> 승차권번호, 예시 -> ["20190621-999-16-0009-25", "20190621-1000-16-0009-26"]
	char	szTicketInfo	[1024];	///> 승차권번호, 예시 -> ["20190621-999-16-0009-25", "20190621-1000-16-0009-26"]

	int		nTotInsMoney		;	///> 총 투입금액
	int		nIns1k				;	///> 투입수량 1,000
	int		nIns5k				;	///> 투입수량 5,000
	int		nIns10k				;	///> 투입수량 1,000
	int		nIns50k				;	///> 투입수량 1,000

	int		nTotOutMoney		;	///> 총 방출금액
	int		nOut100				;	///> 방출수량 100
	int		nOut500				;	///> 방출수량 500
	int		nOut1k				;	///> 방출수량 1,000
	int		nOut5k				;	///> 방출수량 5,000
	int		nOut10k				;	///> 방출수량 10,000

	/***
	char	depr_trml_cd	[10];	/// 출발지 코드
	char	depr_trml_nm	[40];	/// 출발지 이름

	char	arvl_trml_cd	[10];	/// 도착지 코드
	char	arvl_trml_nm	[40];	/// 도착지 이름

	char	depr_dt			[10];	/// yyyy-mm-dd
	char	depr_time		[10];	/// hh:mm:ss

	char	sat_no			[10];	/// 좌석번호
	char	bus_cls_cd		[10];	/// 버스등급
	char	bus_cls_nm		[40];	/// 버스등급 이름
	char	aprv_no			[40];	/// 승인번호
	int		n_tisu_amt			;	/// 발권금액
	int		n_aprv_amt			;	/// 승인금액
	***/

	char	szRfu			[20];	///> 예비
} SMS_TR_DATA_T, *PSMS_TR_DATA_T;


#pragma pack()

//----------------------------------------------------------------------------------------------------------------------

int InitCashTrData(void);
int TermCashTrData(void);
int AddCashTrData(int nFlag, WORD w100, WORD w500, WORD w1k, WORD w5k, WORD w10k, WORD w50k);

/// 시재마감 내역
int InitCashCloseData(void);
int TermCashCloseData(void);
int AddCashCloseData(void);
int SearchCashCloseData(char *pDate, char *pTime, char *retBuf);

/// 2021.03.25 add code
/// 일마감 내역
int InitDayCloseData(void);
int TermDayCloseData(void);
int AddDayCloseData(void);
int SearchDayCloseData(char *pDate, char *pTime, char *retBuf);
/// ~2021.03.25 add code

/// 거래내역 (관제시스템용)
int InitSmsTRData(void);
int TermSmsTRData(void);
int AddSmsTRData(int nFunc, int nBusDvs);


int InitDataMng(void);
int TermDataMng(void);


BOOL DB_AddBusMsgData(char *pData);
