// oper_config.h : 운영정보 파일 처리
// 
// 
//

#pragma once

#include "MyDefine.h"
#include "dev_ui_main.h"

//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

// 현장발권
// typedef struct 
// {
// 	BYTE	byUse;					///> 미사용:0, 사용:1
// 	BYTE	byOperMode;				///> 비배차:0, 배차:1
// 	BYTE	byQuickAlcn;			///> 빠른배차사용유무 : 사용(1), 미사용(0)
// 	BYTE	szRfu[100];
// } ISSUE_MENU_T, *PISSUE_MENU_T;

// 예매발권
// typedef struct 
// {
// 	BYTE	byUse;					///> 미사용:0, 사용:1
// 	BYTE	byAllListView;			///> 예매내역 전체 보여주기, 0:안보여주기, 1:보여주기 
// 	BYTE	byManualView;			///> 예매발권 수기 조회, 0:안보여주기, 1:보여주기  
// 	BYTE	byMrnpAutoIss;			///> 당일예매1건 자동발매 : 사용(1), 미사용(0)
// 	BYTE	byMrnpDetailVw;			///> 예매1건 세부내역보기 : 사용(1), 미사용(0)
// 
// 	int		nMrnpLimitDay;			///> 예매발권 제한 일자
// 
// 	BYTE	szRfu[100];
// } RESERV_MENU_T, *PRESERV_MENU_T;

// 환불
// typedef struct 
// {
// 	BYTE	byUse;					///> 미사용:0, 사용:1
// 	BYTE	szRfu[100];
// } REFUND_MENU_T, *PREFUND_MENU_T;

// 터미널 정보
typedef struct 
{
	int		nUse;						///> 사용유무
	int		nSvrKind;					///> 서버종류
	char	szWndNo				[4];	///> 터미널 창구번호(2)
	char	szCode7				[7+1];	///> 터미널 코드(7)
	char	szCode4				[4+1];	///> 단축 터미널 코드(4)
	char	szPgmDVS			[3+1];	///> 프로그램 구분(3)
	char	szUserNo			[20];	///> User No
	char	szUserPwd			[20];	///> User Pwd
	char	szName				[50];	///> 터미널 명칭
	char	szCorpName			[100+1];///> 터미널 상호
	char	szCorpTel			[50];	///> 터미널 전화번호
	char	szCorpArs			[100];	///> 터미널 ARS번호
	char	szCorpNo			[100];	///> 터미널 사업자번호
	char	szStoreName			[100];	///> 가맹점 이름
	char	szStoreNo			[100];	///> 가맹점 사업자번호
	char	szStoreSeparation	[20];	///> 가맹점 구분코드
	char	szTID				[20];	///> 단말기 TID
	char	byKeySeq;					///> 키 주입 순번(0~9)

	char	sz_prn_trml_nm		[50];	///> 인쇄시 가맹점이름
	char	sz_prn_trml_corp_no [50];	///> 인쇄시 가맹점 사업자번호
	char	sz_prn_trml_sangho	[50];	///> 인쇄시 터미널 상호
	char	sz_prn_trml_corp_no1[50];	///> 인쇄시 터미널 사업자번호
	char	sz_prn_trml_tel_no	[50];	///> 인쇄시 터미널 전화번호
	char	sz_prn_trml_ars_no	[50];	///> 인쇄시 터미널 ARS 번호

	char	szRfu[100];				///> RFU
} TERMINAL_INFO_T, *PTERMINAL_INFO_T;

typedef struct 
{
	UI_BASE_T		base_t;					///> 
	TERMINAL_INFO_T ccTrmlInfo_t;			///> 시외버스 터미널 정보
	TERMINAL_INFO_T koTrmlInfo_t;			///> 고속버스(코버스) 터미널 정보
	TERMINAL_INFO_T ezTrmlInfo_t;			///> 고속버스(이지서버) 터미널 정보

	char			myIPAddress[40];		///> my ip address
	char			myMacAddress[40];		///> my mac address

	char			job_start_dt[20];		///> 업무 시작 일자

	char			rfu[200];				///> 
} OPER_FILE_CONFIG_T, *POPER_FILE_CONFIG_T;

#pragma pack()


//----------------------------------------------------------------------------------------------------------------------

void SetOperConfigData(void);
char* GetOperConfigData(void);

int GetConfigPayment(void);
int GetConfigServerKind(void);

char GetConfigCloseOptValue(void);
int GetConfigOperCorp(void);

char* GetPgmDVS(void);
char* GetReqPgmDVS(int nSvrKind);

int GetConfigTicketPapaer(void);
void SetConfigTicketPapaer(char *pType);

char* GetTrmlWndNo(int nSvrKind);
char* GetTrmlCode(int nSvrKind);

void SetTrmlCode(int nSvrKind, char *pData);
char* GetTrmlName(void);

char* GetShortTrmlCode(int nSvrKind);
void SetShortTrmlCode(int nSvrKind, char *pData);

char* GetMyIpAddress(void);
void SetMyIpAddress(char *pIP);

char* GetMyMacAddress(void);
void SetMyMacAddress(char *pMAC);

BOOL IsAlcn(void);

char* GetTckPrinterType(void);

char *GetLoginUserNo(int nSvrKind);
void SetLoginUserNo(char *pData);

char *GetLoginUserPwd(int nSvrKind);
void SetLoginUserPwd(char *pData);

int Config_IsAlcnMode(void);

char* GetConfigBaseInfo(void);
char* GetConfigTrmlInfo(int nSvrKind);
void SetConfigTrmlInfo(int nSvrKind, char *pData);

BOOL Config_IsCCServer(void);
BOOL Config_IsExpServer(void);


int InitConfigFile(void);
int ReadConfigFile(char *retBuf);
int WriteConfigFile(char *pWrData);

int GetTicketLimitCount(void);
int GetBillLimitCount(BOOL b1k);
int GetCoinLimitCount(BOOL b100);

void TermConfigFile(void);

int CheckTicketCount(void);
int CheckCoinCount(void);
int CheckBillCount(void);
int CheckAutoClose(void);

int SetAutoCloseEvent(void);

int IsConfigTPHPrint(void);

int WndKioskClose(BOOL bReboot);
int CheckBuffer(char *pData, int nMax);

CTime Util_CTimeFromString(char *pDate, char *pTime);

int Prom_Update(int nCode, BOOL bHappen);	// 20220110 ADD

