// 
// 
// dev_prt_ticket_main.h : 승차권 프린터 main 헤더 파일
//

#pragma once

//----------------------------------------------------------------------------------------------------------------------
#define DEV_TCKPRT_HS	1				/// 화성프린터 825 모델
#define DEV_TCKPRT_NHS	2				/// 화성프린터 825 모델 - 신규 프로토콜 적용(send & recv)

//----------------------------------------------------------------------------------------------------------------------
#define MAX_PRT_TCK_BUF			200
#define MAX_PRT_TCK_RETRY		10
#define MAX_PRT_TCK_TIMEOUT		2000

//----------------------------------------------------------------------------------------------------------------------
#define MAX_HANGUL_CHAR			18

//----------------------------------------------------------------------------------------------------------------------

enum _en_new_print_font_
{
	FONT_NONE_EXPAND = 0,
	FONT_VERTI_EXPAND,
	FONT_HORI_EXPAND,
	FONT_BOTH_EXPAND,
};

enum _en_new_print_mode_
{
	PRT_MODE_NONE			= 0	,
	PRT_MODE_VERTI_EXPAND		,
	PRT_MODE_HORI_EXPAND		,
	PRT_MODE_BOTH_EXPAND		,
	PRT_MODE_SMALL_ON			,
	PRT_MODE_SMALL_OFF
	,

};
enum _en_new_print_rotate
{
	PRT_ROTATE_NONE			= 0,
	PRT_ROTATE_JUN
};
//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

typedef struct  
{
	int		nCommand;
	BYTE	szCommand[3];
	char*	pStr;
} PRT_TCK_TABLE_T, *PPRT_TCK_TABLE_T;

#pragma pack()

//----------------------------------------------------------------------------------------------------------------------

// public const string 버스티켓고유번호 = "00";
// public const string 발권사용자번호 = "01";
// public const string 출발터미널한글명 = "02";
// public const string 출발터미널영문명 = "03";
// public const string 도착터미널한글명 = "04";
// public const string 도착터미널영문명 = "05";
// public const string 거리 = "06";
// public const string 버스티켓금액 = "07";
// public const string 버스티켓종류 = "08";
// public const string 출발일자 = "09";
// public const string 출발시각 = "10";
// public const string 좌석번호 = "11";
// public const string 승차홈 = "12";
// public const string 버스등급 = "13";
// public const string 운수사 = "14";
// public const string 결제구분 = "15";
// public const string 현영_카드번호 = "16";
// public const string 승인금액 = "17";
// public const string 승인번호 = "18";
// public const string 자진발급승인번호 = "19";
// public const string 자진발급사업자등록번호 = "20";
// public const string 자진발급URL = "21";
// public const string 발행구분명LT1 = "22";
// public const string 수수료구분명LT2 = "23";
// public const string 발행구분명LM1 = "24";
// public const string 수수료구분명LM2 = "25";
// public const string 발행구분명LB1 = "26";
// public const string 수수료구분명LB2 = "27";
// public const string 발행구분명RT1 = "28";
// public const string 발행구분명RT2 = "29";
// public const string 수수료구분명RM1 = "30";
// public const string 수수료구분명RM2 = "31";
// public const string 발행구분명RB1 = "32";
// public const string 수수료구분명RB2 = "33";
// public const string 버스티켓일련번호 = "34";
// public const string 발권시각 = "35";
// public const string 출발터미널전화번호 = "36";
// public const string 바코드1D = "37";
// public const string 바코드2D = "38";
// public const string 버스티켓출력메세지 = "39";
// public const string 선불카드거래이전금액 = "40";
// public const string 선불카드거래이후금액 = "41";
// public const string 고속버스검표추가내용 = "42";
// public const string 부가세 = "43"; //할인종류로 씀
// public const string 운행형태 = "48";


//----------------------------------------------------------------------------------------------------------------------

int TckPrt_Initialize(void);
int TckPrt_Terminate(void);

int TckPrt_GetStatus(void);

int TckPrt_MrnpPrintTicket(int nSvrKind, char *pData);
int TckPrt_PubPrintTicket(int nSvrKind, char *pData);
int TckPrt_TestPrintTicket(int nSvrKind, char *pData);	// 20211116 ADD

