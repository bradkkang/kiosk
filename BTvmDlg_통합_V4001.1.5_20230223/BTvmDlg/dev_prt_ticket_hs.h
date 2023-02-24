// 
// 
// Prt_TcketHS.h : 화성 승차권 프린터 헤더 파일
//

#pragma once

#include "MySerial.h"
#include "MyLogFile.h"
#include "dev_prt_ticket_main.h"

//#define MAX_PRT_TCK_BUF			200
//#define MAX_PRT_TCK_RETRY		10
//#define MAX_PRT_TCK_TIMEOUT		2000
//
////#define MAX_HANGUL_CHAR			14
//#define MAX_HANGUL_CHAR			18
//
//enum _en_print_font_
//{
//	FONT_NONE_EXPAND = 0,
//	FONT_VERTI_EXPAND,
//	FONT_HORI_EXPAND,
//	FONT_BOTH_EXPAND,
//};
//
//enum _en_print_mode_
//{
//	PRT_MODE_NONE			= 0	,
//	PRT_MODE_VERTI_EXPAND		,
//	PRT_MODE_HORI_EXPAND		,
//	PRT_MODE_BOTH_EXPAND		,
//	PRT_MODE_SMALL_ON			,
//	PRT_MODE_SMALL_OFF			,
//
//};
//
//#pragma pack(1)
//
//typedef struct  
//{
//	int		nCommand;
//	BYTE	szCommand[3];
//	char*	pStr;
//} PRT_TCK_TABLE_T, *PPRT_TCK_TABLE_T;
//
//#pragma pack()


/**
 * @brief		CPrinterHsComm
 * @details		승차권 프린터 통신 class
 */
class CPrtTicketHsComm : public CMySerial 
{
public:
	CPrtTicketHsComm(void) {};
	virtual ~CPrtTicketHsComm(void) {};
};

/**
 * @brief		CPrtTicketHSLogFile
 * @details		승차권 프린터 로그파일 생성자
 */
class CPrtTicketHSLogFile : public CMyLogFile 
{
public:
	CPrtTicketHSLogFile(void) {};
	virtual ~CPrtTicketHSLogFile(void) {};
};

//======================================================================= 

class CPrtTicketHS
{
public:
	CPrtTicketHS();
	~CPrtTicketHS();

private :
	CPrtTicketHsComm	m_clsComm;
	BOOL			m_bConnected;
	HANDLE			m_hAccMutex;
	HANDLE			m_hThread;

	CPrtTicketHSLogFile	m_clsLog; 
	BYTE			m_szTxBuf[MAX_PRT_TCK_BUF];
	BYTE			m_szRxBuf[MAX_PRT_TCK_BUF];
	char			m_Buffer[1024];

	BOOL			m_bExpOnly;

protected:
	void LOG_INIT(void);
	int Locking(void);
	int UnLocking(void);
	int SendData(BYTE *pData, int nDataLen);

	static DWORD WINAPI RunThread(LPVOID lParam);

	void DrawBox(BOOL bPassenger);


public:

	int IsHangule(char *pData, int nDataLen);

	int GetStatus(void);

	void SetBold(int nValue);

	void SetDoubleFont(BOOL bSet);
	void SetDoubleFont(int nFont, BOOL bSet);
	void SetSmallFont(BOOL bSet);
	int Print(const char *Format, ... );
	int txtPrint(int nX, int nY, int nMode, char *pStr);
	void ID_Print(int nSvrKind, int nPart, int nID, char *pData);

	//int MultiPrint(int nX, int nY, int nMaxMode, int nMinMode, int nMAX, char *pData);
	int MultiPrint(int nX, int nY, int nMaxMode, int nMinMode, int nMAX, char *pData, BOOL bExpJun = FALSE);

	void EngTrml_Print(int nSvrKind, int nPart, int nID, char *pData);

	void BeginPrint(BOOL bStart);

	int CCS_DF3_PrintTicket(char *pData);
	int CCS_DFAll_PrintTicket(char *pData);
	int CCS_DFAll_IIAC_PrintTicket(char *pData);

	int Exp_DFAll_PrintTicket(char *pData);
	int ExpOnlyPrintTicket(char *pData);

	int CbusTicketPrint(char *pData);
	int KobusTicketPrint(char *pData);

	int StartProcess(int nCommIdx);
	int EndProcess(void);
};
