// 
// 
// CoinWS.h : 동전방출기(우성) 헤더 파일
//

#pragma once

#include "MySerial.h"
#include "MyLogFile.h"

//----------------------------------------------------------------------------------------------------------------------

#define MAX_COIN_WS_BUF			200
#define MAX_COIN_WS_RETRY		10
#define MAX_CON_WS_TIMEOUT		2000

#define COIN_100_TYPE			1
#define COIN_500_TYPE			2

//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

typedef struct  
{
	BYTE	szAddr[2];
	BYTE	byCommand;
	BYTE	byData;
} COIN_WS_HEADER_T, *PCOIN_WS_HEADER_T;

#pragma pack()

//----------------------------------------------------------------------------------------------------------------------

#define SCMD_OUT_COIN		 0x6F	///> 'o' : 코인 배출 Command
#define SCMD_RESET			 0x72	///> 'r' : 호퍼 Reset Command
#define SCMD_STATUS			 0x73	///> 's' : Hopper Status 요청 Command
#define SCMD_OUT_COIN_INFO	 0x74	///> 't' : Hopper Error 상태 배출수량 요청

#define RCMD_RESPONSE		 0x43	///> 'C' : 일반 응답 Command
#define RCMD_ERR_COIN		 0x45	///> 'E' : Hopper Error Command
#define RCMD_OUT_COIN_END	 0x46	///> 'F' : 코인 배출 종료 Command
#define RCMD_OUT_COIN		 0x4F	///> 'O' : 'o' Command 응답
#define RCMD_STATUS			 0x53	///> 'S' : 's' Command 응답
#define RCMD_COIN_COUNT		 0x54	///> 'T' : 코인 배출 수량 count data 응답 command

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CCoinWSComm
 * @details		지퍠인식기 통신 class
 */
class CCoinWSComm : public CMySerial 
{
public:
	CCoinWSComm(void) {};
	virtual ~CCoinWSComm(void) {};
};

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CCoinWSLogFile
 * @details		지퍠인식기 로그파일 생성자
 */
class CCoinWSLogFile : public CMyLogFile 
{
public:
	CCoinWSLogFile(void) {};
	virtual ~CCoinWSLogFile(void) {};
};

//----------------------------------------------------------------------------------------------------------------------

class CCoinWS
{
public:
	CCoinWS();
	~CCoinWS();

private :
	CCoinWSComm		m_clsComm;
	BOOL			m_bConnected;
	HANDLE			m_hAccMutex;
	HANDLE			m_hThread;

	CCoinWSLogFile	m_clsLog;

	BYTE			m_szTxBuf[MAX_COIN_WS_BUF];
	BYTE			m_szRxBuf[MAX_COIN_WS_BUF];

	BYTE			m_byStatus;
	BOOL			m_bRun;

	BOOL			m_bLog;

public:
	int				m_nType;
	int				m_nOutCount;

	
private :
	void LOG_INIT(int nFlag);
	int Locking(void);
	int UnLocking(void);
	int SendChar(BYTE byData);
	int SendPacket(int nCommand, BYTE *pData, WORD wDataLen);
	int GetPacket(BYTE *retBuf);
	int SndRcvPacket(int nCommand, BYTE *pData, int nDataLen, BYTE *retBuf);

	int SetCoinStatus(BYTE bStatus);

	static DWORD WINAPI RunThread(LPVOID lParam);

public:

	int OutCoin(int nCoin);
	int Reset(void);
	int GetStatus(void);
	int GetOutInfo(void);
	int TotalEnd(void);


	int StartProcess(int nCommIdx, int nFlag);
	int EndProcess(void);


	int Coin100_GetStatus(void);
	int Coin500_GetStatus(void);
};
