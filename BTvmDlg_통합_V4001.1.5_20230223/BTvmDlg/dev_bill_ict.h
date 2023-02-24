// 
// 
// dev_bill_ict.h : 지폐 ICT 헤더 파일
//

#pragma once

#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <vector>
#include <queue>

#include "MySerial.h"
#include "MyLogFile.h"

using namespace std;

#define MAX_CDU_BUF			200
#define MAX_CDU_RETRY		10

#define BILL_CMD_RESET		1
#define BILL_CMD_ENABLE		2
#define BILL_CMD_DISABLE	3
#define BILL_CMD_GETSTATUS	4

#pragma pack(1)

typedef struct
{
	BYTE	byStatus;
	char*	pStr;
} BILLICT_STATUS_T, *PBILLICT_STATUS_T;

typedef struct 
{
	WORD	wCommand;
	//int		nFlag;
	//int		nLen;
	//char	szData[100 + 1];
} BILL_QUE_DATA_T, *PBILL_QUE_DATA_T;

#pragma pack()

/**
 * @brief		CBillIctComm
 * @details		지퍠인식기 통신 class
 */
class CBillIctComm : public CMySerial 
{
public:
	CBillIctComm(void) {};
	virtual ~CBillIctComm(void) {};
};

/**
 * @brief		CBillIctLogFile
 * @details		지퍠인식기 로그파일 생성자
 */
class CBillIctLogFile : public CMyLogFile 
{
public:
	CBillIctLogFile(void) {};
	virtual ~CBillIctLogFile(void) {};
};

//======================================================================= 

#pragma once


class CBillICT
{
public:
	CBillICT();
	~CBillICT();

	enum _enCommand_ {
		BILL_ACCEPT_FINISH		 = 0,	///> 
		BILL_MOTOR_FAIL,			///> 
		BILL_CHECKSUM_ERR,			///> 
		BILL_JAM,					///> 
		BILL_REMOVE,				///> 
		BILL_STACKER_OPEN,			///> 
		BILL_SENSOR_PROBLEM,		///> 
		BILL_FISH,					///> 
		BILL_STACKER_PROBLEM,		///> 
		BILL_REJECT,				///> 
		BILL_INVALID_CMD_1,			///> 
		BILL_INVALID_CMD_2,			///> 
		BILL_INVALID_CMD_3,			///> 
		BILL_INVALID_CMD_4,			///> 
		BILL_RESERVED,				///> 
		BILL_ERR_EXCLUSION,			///> 예외사항
		BILL_ENABLE,				///> 
		BILL_INHIBIT,				///> 
		BILL_POWER_ON,				///> 
		BILL_HANDLE_REQ,			///> 
	};

public :
	CBillIctComm	m_clsComm;
	BOOL			m_bConnected;
	HANDLE			m_hAccMutex;
	HANDLE			m_hThread;

	CBillIctLogFile	m_clsLog;
	BYTE			m_szTxBuf[MAX_CDU_BUF];
	BYTE			m_szRxBuf[MAX_CDU_BUF];

	DWORD			m_dwStatus;
	int				m_n1k;
	int				m_n5k;
	int				m_n10k;
	int				m_n50k;
	int				m_nTotalFare;
	int				m_nDebug;
	queue <BILL_QUE_DATA_T> s_QueData;

	BOOL			m_bInsert;

private :
	void LOG_INIT(void);
	int Locking(void);
	int UnLocking(void);
	int SendPacket(BYTE byData);
	int GetPacket(BYTE *retBuf);

	void SetStatus(int nIndex);
	int GetStatus(int nIndex);
	void ClearStatus(int nIndex);
	int SetStatusValue(BYTE byData);
										
	static DWORD WINAPI RunThread(LPVOID lParam);

public:

	int Reset(void);
	int GetStatus(void);
	int GetDevStatus(void);
	int Enable(void);
	int Inhibit(void);
	int GetMoney(int *n1k, int *n5k, int *n10k, int *n50k);
	int TotalEnd(void);

	int StartProcess(int nCommIdx);
	int EndProcess(void);

	int CmdQueueInfo(BILL_QUE_DATA_T tQue);
	int AddQue(int nCmd);
};
