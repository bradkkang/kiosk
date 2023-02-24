// File_Ini.h : 시외버스 무인발매기 Ini 파일 처리
// 
// 
//

#pragma once

//----------------------------------------------------------------------------------------------------------------------

///> section name
#define SECTION_NM_MACHINE				"MACHINE_INFO"
#define SECTION_NM_SW_FUNCTION			"SW_FUNCTION"
#define SECTION_NM_DEV_COIN_100			"DEVICE_COIN_100"
#define SECTION_NM_DEV_COIN_500			"DEVICE_COIN_500"
#define SECTION_NM_DEV_BILL				"DEVICE_BILL"
#define SECTION_NM_DEV_DISPENSER		"DEVICE_BILL_DISPENSE"
#define SECTION_NM_DEV_TICKET_PRT		"DEVICE_TICKET_PRINTER"
#define SECTION_NM_DEV_RECEIPT_PRT		"DEVICE_RECEIPT_PRINTER"
#define SECTION_NM_DEV_TICKET_READER	"DEVICE_TICKET_READER"
#define SECTION_NM_DEV_CARD_READER		"DEVICE_CARD_READER"
#define SECTION_NM_DEV_UI				"DEVICE_UI"

///> key name

///> "MACHINE_INFO"
#define KEY_NM_TYPE						"DEV_TYPE"						///> 1(자립형 1), 2(자립형 2), 3(매립형) 
#define KEY_NM_LCD_COUNT				"LCD_COUNT"						///> LCD 갯수
#define KEY_NM_TERMINAL_WND_NO			"TERMINAL_WND_NO"				///> 터미널 창구번호
#define KEY_NM_TERMINAL_CODE			"TRML_CD"						///> 터미널 코드
#define KEY_NM_SHORT_TERMINAL_CODE		"SHORT_TRML_CD"					///> 단축 터미널 코드
#define KEY_NM_BUS_SEP					"BUS_SEP"						///> 1(시외버스), 2(고속버스), 3(시외/고속버스)
#define KEY_NM_PAY_METHOD				"PAY_METHOD"					///> 1(카드), 2(현금), 3(카드+현금)

#define KEY_NM_USER_NO					"USER_NO"						///> 
#define KEY_NM_USER_PWD					"USER_PWD"						///> 

///> "SW_FUNCTION"
#define KEY_NM_REFUND					"FUNC_REFUND"					///> 환불 사용여부
#define KEY_NM_PUBTCK					"FUNC_PUBTCK"					///> 현장발권 사용여부
#define KEY_NM_MRNP						"FUNC_MRNP"						///> 예매발권 사용여부

///> "DEVICE_INFO"
#define KEY_NM_USE						"USE"							///> 디바이스 사용유무
#define KEY_NM_MODEL					"MODEL"							///> 디바이스 모델번호
#define KEY_NM_COM_PORT					"COM_PORT"						///> 디바이스 통신포트

#define KEY_NM_IP						"IP_ADDR"						///> IP address
#define KEY_NM_TCP_PORT					"PORT"							///> TCP port


//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

typedef struct 
{
	int		nUse;					///> 미사용 : 0, 사용 : 1
	int		nModel;					///> 모델번호
	int		nPort;					///> 통신포트

	char	szIPAddress[40];		///> IP Address
	int		nTcpPort;				///> TCP 포트
} DEV_CFG_T, *PDEV_CFG_T;

typedef struct 
{
	int nType;						///> 0x01(자립형 1), 0x02(자립형 2), 0x04(매립형) 

	///> LCD 갯수
	int nLcdCount;					///> LCD 갯수

	///> 장비번호
	int nTerminalWndNo;

	///> 터미널 번호(7)
	int nTerminalCode;

	///> 단축 터미널 번호(4)	
	int nShortTerminalCode;

	///> 버스 구분
	int nBusSep;

	///> 결제방식
	int nPayment;

	char szUserNo[20];				///> User No
	char szUserPwd[20];				///> User Pwd

	int nIsRefund;					///> 환불 기능 사용유무
	int nIsPubIssue;				///> 현장발권 기능 사용유무
	int nIsMrnp;					///> 예매발권 기능 사용유무

	DEV_CFG_T	tCoin100;			///> 100원 동전방출 serial device
	DEV_CFG_T	tCoin500;			///> 500원 동전방출 serial device
	DEV_CFG_T	tBill;				///> 지폐입금기 serial device
	DEV_CFG_T	tDispenser;			///> 지폐방출기 serial device
	DEV_CFG_T	tPrtTicket;			///> 승차권 발권기 serial device
	DEV_CFG_T	tPrtReceipt;		///> 영수증 프린터 serial device
	DEV_CFG_T	tTicketReader;		///> 승차권 리더기 serial device
	DEV_CFG_T	tCardReader;		///> 신용카드 리더기 serial device
	DEV_CFG_T	tUI;				///> UI tcp/ip 통신

} VM_ENVINI_T, *PVM_ENVINI_T;

#pragma pack()

//----------------------------------------------------------------------------------------------------------------------

void* GetEnvInfo(void);

char* GetEnvCoin100Info(void);
char* GetEnvCoin500Info(void);
char* GetEnvBillInfo(void);
char* GetEnvDispenserInfo(void);
char* GetEnvPrtTicketInfo(void);
char* GetEnvPrtReceiptInfo(void);
char* GetEnvTicketReaderInfo(void);
char* GetEnvCardReaderInfo(void);
char* GetEnvUIInfo(void);

int ReadEnvIniFile(void);
void WriteEnvIniFile(char *pData);

int Init_IniFile(void);

