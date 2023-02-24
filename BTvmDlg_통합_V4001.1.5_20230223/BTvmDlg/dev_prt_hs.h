// 
// 
// dev_prt_hs.h : 화성 영수증 프린터 헤더 파일
//

#pragma once

#include "MySerial.h"
#include "MyLogFile.h"


#define MAX_PRT_HS_BUF			256
#define MAX_PRT_HS_RETRY		10
#define MAX_PRT_HS_TIMEOUT		2000

#pragma pack(1)

typedef struct  
{
	int		nCommand;
	BYTE	szCommand[3];
	char*	pStr;
} PRT_TRANS_TABLE_T, *PPRT_TRANS_TABLE_T;

#pragma pack()

/**
 * @brief		CPrinterHsComm
 * @details		영수증 프린터 통신 class
 */
class CPrinterHsComm : public CMySerial 
{
public:
	CPrinterHsComm(void) {};
	virtual ~CPrinterHsComm(void) {};
};

/**
 * @brief		CPrinterHSLogFile
 * @details		영수증 프린터 로그파일 생성자
 */
class CPrinterHSLogFile : public CMyLogFile 
{
public:
	CPrinterHSLogFile(void) {};
	virtual ~CPrinterHSLogFile(void) {};
};

//======================================================================= 

#pragma once


class CPrinterHS
{
public:
	CPrinterHS();
	~CPrinterHS();

private :
	CPrinterHsComm	m_clsComm;
	BOOL			m_bConnected;
	HANDLE			m_hAccMutex;
	HANDLE			m_hThread;

	CPrinterHSLogFile m_clsLog;
	BYTE			m_szTxBuf[MAX_PRT_HS_BUF];
	BYTE			m_szRxBuf[MAX_PRT_HS_BUF];
	BYTE			m_szTemp[MAX_PRT_HS_BUF];
	
	BYTE			m_szTempMAX[1024 * 4];

public:
	int		FIRST_GAP;

private :
	void LOG_INIT(void);
	int Locking(void);
	int UnLocking(void);
	int SendPacket(int nCommand, BYTE *pData, WORD wDataLen);
	int GetPacket(BYTE *retBuf);
	BYTE *SetSpaceData(int nSpaceLen);

	static DWORD WINAPI RunThread(LPVOID lParam);

public:

	int SendData(BYTE *pData, int nDataLen);

	int InitPrinter(void);
	int SetQRCode(char *pData);
	int SetCharPrtingMode(BYTE byVal);
	int SetBold(int nValue);
	int SetFontSize(int nValue);
	int SetPrintPosition(WORD wPos);
	int SetImageData(WORD x, WORD y, BYTE *pImgData, int nLen);
	int LineFeed(int nVal);
	void SetLeftMargin(int n);

	int Control(int nCommand, char *pData, int nDataLen);
	int GetStatus(void);
	
	void Print_Refund(void);
	void Print_Account2(void);
	void Print_Account3(char *pDate, char *pTime);
	int AlignBuffer(int nMax, char *pSrc, int nSrcLen, char *retBuf, int retLen);
	void Print_Ticket(int nSvrKind, int nFunction, char *pData);

	void PaperFullCut(void);
	int Printf(const char *fmt, ...);
	int Printf2(int nCharMode, const char *fmt, ...);

	int StartProcess(int nCommIdx);
	int EndProcess(void);
};
