// 
// 
// Prt_TcketNHS.h : 화성 승차권 프린터_신규_통신프로토콜 헤더 파일
//

#pragma once

#include "MySerial.h"
#include "MyLogFile.h"
#include "dev_prt_ticket_main.h"

#define LEN_LEFTSUBS	8	// 한글 4자(8바이트)만 표시

/**
 * @brief		CPrtTicketNHsComm
 * @details		화성 승차권 프린터_신규_protocol 통신 class
 */
class CPrtTicketNHsComm : public CMySerial 
{
public:
	CPrtTicketNHsComm(void) {};
	virtual ~CPrtTicketNHsComm(void) {};
};

/**
 * @brief		CPrtTicketNHSLogFile
 * @details		화성 승차권 프린터_신규_프로토콜 로그파일 생성자
 */
class CPrtTicketNHSLogFile : public CMyLogFile 
{
public:
	CPrtTicketNHSLogFile(void) {};
	virtual ~CPrtTicketNHSLogFile(void) {};
};

//======================================================================= 

/// 화성 신규 프로토콜
class CPrtTicketHS_NP
{
public:
	CPrtTicketHS_NP();
	~CPrtTicketHS_NP();

private :
	CPrtTicketNHsComm	m_clsComm;
	BOOL			m_bConnected;
	HANDLE			m_hAccMutex;

	BYTE			m_szTxBuf[MAX_PRT_TCK_BUF];
	BYTE			m_szRxBuf[MAX_PRT_TCK_BUF];

	BOOL			m_bExpOnly;

	CPrtTicketNHSLogFile m_clsLog;

protected:
	int Locking(void);
	int UnLocking(void);
	void LOG_INIT(void);

	void DrawBox(BOOL bPassenger);
public:

	int TxRxData(BYTE *pData, int nDataLen, BOOL bRxFlag = TRUE);
//	int TRX_txtPrint(int nX, int nY, int nMode, char *pStr);
	int txtPrint(int nX, int nY, int nMode, int nRotate, char *pStr); //kh_200717 로테이트 추가

	int GetStatus(void);
	void BeginPrint(BOOL bStart);
	void SetBold(int nValue);

	void SetDoubleFont(BOOL bSet);
	void SetDoubleFont(int nFont, BOOL bSet);
	void SetSmallFont(BOOL bSet);
	char* LeftSubstring(char* sz, int len);
	int IsHangule(char *pData, int nDataLen);
	//int MultiPrint(int nX, int nY, int nMaxMode, int nMinMode, int nMAX, int nRotate, char *pData); //kh_200717 로테이트 추가
	int MultiPrint(int nX, int nY, int nMaxMode, int nMinMode, int nMAX, int nRotate, char *pData, BOOL bExpJun = FALSE);

	int StartProcess(int nCommIdx);
	int EndProcess(void);

	int Print_DeprDt(char *pData, char *pPrt);
	int Print_DeprTime(char *pData, char *pPrt);

	int CbusTicketPrint(char *pData);
	int KobusTicketPrint(char *pData);
	int TmExpTicketPrint(char *pData);
	int TestTicketPrint(char *pData, int nSvrKind);	// 20211116 ADD

};
