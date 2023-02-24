// 
// 
// dev_bill_onep.h : 지폐 원플러스 헤더 파일
//

#pragma once

#include "MySerial.h"
#include "MyLogFile.h"


#define MAX_ONEP_BUF		200
#define MAX_ONEP_RETRY		10

#pragma pack(1)

typedef struct
{
	BYTE	byStatus;
	char	szShtNm[20];
	char*	pStr;
} BILLONEP_STATUS_T, *PBILLONEP_STATUS_T;

typedef struct
{
	BYTE	byEvent;
	char*	pStr;
} BILLONEP_EVENT_T, *PBILLONEP_EVENT_T;

#pragma pack()

/**
 * @brief		CBillIctComm
 * @details		지퍠인식기 통신 class
 */
class CBillOnepComm : public CMySerial 
{
public:
	CBillOnepComm(void) {};
	virtual ~CBillOnepComm(void) {};
};

/**
 * @brief		CBillIctLogFile
 * @details		지퍠인식기 로그파일 생성자
 */
class CBillOnepLogFile : public CMyLogFile 
{
public:
	CBillOnepLogFile(void) {};
	virtual ~CBillOnepLogFile(void) {};
};

//======================================================================= 

#pragma once


class CBillONEP
{
public:
	CBillONEP();
	~CBillONEP();

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
	CBillOnepComm	m_clsComm;
	BOOL			m_bConnected;
	HANDLE			m_hAccMutex;
	HANDLE			m_hThread;

	CBillOnepLogFile	m_clsLog;
	BYTE			m_szTxBuf[MAX_ONEP_BUF];
	BYTE			m_szRxBuf[MAX_ONEP_BUF];

	DWORD			m_dwStatus;
	int				m_n1k;
	int				m_n5k;
	int				m_n10k;
	int				m_n50k;
	int				m_nTotalFare;
	int				m_nDebug;
	BOOL			m_bStacking;
	BOOL			m_bEnable;
	

private :
	void LOG_INIT(void);
	int Locking(void);
	int UnLocking(void);

	int SendPacket(BYTE *pData, int nDataLen);
	int GetPacket(BYTE *retBuf);

	int SetStatusValue(BYTE byData);
										
	static DWORD WINAPI RunThread(LPVOID lParam);

public:

	int doEventProcess(void);

	int Reset(void);
	int GetStatus(void);
	int GetDevStatus(void);
	int Enable(void);
	int Inhibit(void);
	int GetMoney(int *n1k, int *n5k, int *n10k, int *n50k);
	int TotalEnd(void);

	int GetConfig(void);
	int SetConfig(BOOL b1k, BOOL b5k, BOOL b10k, BOOL b50k);
	int GetErrorInfo(void);
	int Alive(void);
	int GetVersion(void);

	int StartProcess(int nCommIdx);
	int EndProcess(void);

};
