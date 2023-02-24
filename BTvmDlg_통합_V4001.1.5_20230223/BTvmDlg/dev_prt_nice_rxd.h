// 
// 
// dev_prt_nice_rxd.h : 영수증 프린터_한전금_REXOD 헤더 파일
//

#pragma once

#include "MySerial.h"
#include "MyLogFile.h"

//======================================================================= 

#define MAX_PRT_REXOD_BUF		200
#define MAX_PRT_REXOD_RETRY		10
#define MAX_PRT_REXOD_TIMEOUT	2000

//======================================================================= 

#pragma pack(1)


#pragma pack()

/**
 * @brief		CPrinterRxdComm
 * @details		영수증 프린터_한전금_REXOD 통신 class
 */
class CPrinterRxdComm : public CMySerial 
{
public:
	CPrinterRxdComm(void) {};
	virtual ~CPrinterRxdComm(void) {};
};

/**
 * @brief		CPrinterRxdLogFile
 * @details		영수증 프린터_한전금_REXOD 로그파일 생성자
 */
class CPrinterRxdLogFile : public CMyLogFile 
{
public:
	CPrinterRxdLogFile(void) {};
	virtual ~CPrinterRxdLogFile(void) {};
};

//======================================================================= 

/**
 * @brief		CPrinterRxd
 * @details		영수증 프린터_한전금_REXOD 통신 class
 */
class CPrinterRxd
{
public:
	CPrinterRxd();
	~CPrinterRxd();

private :
	CPrinterRxdComm	m_clsComm;
	HANDLE			m_hAccMutex;
	BOOL			m_bConnected;

	CPrinterRxdLogFile m_clsLog;
	BYTE			m_szTxBuf[MAX_PRT_REXOD_BUF];
	BYTE			m_szRxBuf[MAX_PRT_REXOD_BUF];
	BYTE			m_szTemp[MAX_PRT_REXOD_BUF];

private :
	void LOG_INIT(void);
	int Locking(void);
	int UnLocking(void);
	int SendPacket(int nCommand, BYTE *pData, WORD wDataLen);
	int GetPacket(BYTE *retBuf);
	BYTE *SetSpaceData(int nSpaceLen);

public:

	int SendData(BYTE *pData, int nDataLen);
	
	int InitPrinter(void);
	int SetQRCode(char *pData);
	int SetCharPrtingMode(BYTE byVal);
	int SetPrintPosition(WORD wPos);
	int SetImageData(WORD x, WORD y, BYTE *pImgData, int nLen);

	int LineFeed(int nVal);
	void FullCut(void);
	void PaperFullCut(void);
	int GetStatus(void);
	int AlignBuffer(int nMax, char *pSrc, int nSrcLen, char *retBuf, int retLen);

	int Printf(const char *fmt, ...);
	int Printf2(int nCharMode, const char *fmt, ...);

	void Print_Account(void);
	void Print_Account2(void);
	void Print_Account3(char *pDate, char *pTime);
	void Print_Refund(void);
	void Print_Ticket(int nSvrKind, int nFunction, char *pData);


	int StartProcess(int nCommIdx);
	int EndProcess(void);

};
