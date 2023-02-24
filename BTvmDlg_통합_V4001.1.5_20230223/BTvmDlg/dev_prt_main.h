// 
// 
// dev_prt_main.h : 영수증 프린터 main 헤더 파일
//

#pragma once

//----------------------------------------------------------------------------------------------------------------------

#define RECEIPT_FMT_REFUND		1		/// 환불 영수증

//----------------------------------------------------------------------------------------------------------------------

#define DEV_RECEIPT_ATEC_HS		1		/// 에이텍 - 화성 프린터
#define DEV_RECEIPT_NICE_RXD	2		/// 한전금 - REXOD 프린터	(RX830-H120)

//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

typedef struct
{
	char bus_nm			[100+1]	;	///< 시외버스, 고속버스 title
	char bizr_nm		[100+1]	;	///< 가맹점명
	char bizr_no		[100+1]	;	///< 가맹점 사업자번호

	char pyn_dvs_nm		[100+1]	;	///< 결제수단
	char ticket_info	[100+1]	;	///< 승차권 정보(바코드 or QR코드)
	char tisu_amt		[100+1]	;	///< 발권금액
	char cmrt			[100+1]	;	///< 환불율
	char ry_amt			[100+1]	;	///< 환불금액

	char depr_dt		[100+1]	;	///< 출발일
	char depr_time		[100+1]	;	///< 출발시간
	char depr_nm		[100+1]	;	///< 출발지 이름
	char arvl_nm		[100+1]	;	///< 도착지 이름
	char bus_cls_nm		[100+1]	;	///< 버스등급 명
	char sat_no			[100+1]	;	///< 좌석번호

} prt_refund_t, *pprt_refund_t;

#pragma pack()

//----------------------------------------------------------------------------------------------------------------------

int Printer_Initialize(void);
int Printer_Terminate(void);

int Printer_GetStatus(void);
int Printer_MakeRefundTicketData(char *retBuf);
int Printer_RefundTicket(void);
int Printer_AccountInfo(void);
int Printer_Account2Info(void);
/// 2021.03.26 add code
int Printer_Account3Info(void);
/// ~2021.03.26 add code
int Printer_TicketPrint(int nSvrKind, int nFunction, char *pData);
