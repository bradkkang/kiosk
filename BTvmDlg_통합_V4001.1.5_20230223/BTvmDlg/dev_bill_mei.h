// 
// 
// dev_bill_mei.h : 지폐 MEI 헤더 파일
//

#pragma once

#include "MySerial.h"
#include "MyLogFile.h"


#define MAX_MEI_BUF			(1024 * 2)
#define MAX_MEI_RETRY		10


enum _en_Mei_Send_Command__
{
	MEI_SCMD_INIT	= 0	,
	MEI_SCMD_IDLE		,
	MEI_SCMD_DISABLE	,
	MEI_SCMD_ENABLE		,
	MEI_SCMD_RESET		,
};

/// RECV_DATA_0
#define MEI_R0_IDLING		0x01
#define MEI_R0_ACCEPTING	0x02
#define MEI_R0_ESCROWED		0x04
#define MEI_R0_STACKING		0x08
#define MEI_R0_STACKED		0x10
#define MEI_R0_RETURNING	0x20
#define MEI_R0_RETURNED		0x40
#define MEI_R0_COMPLETED	0x11

/// RECV_DATA_1
#define	MEI_R1_CHEATED		0x01
#define	MEI_R1_REJECTED		0x02
#define MEI_R1_JAMMED		0x04
#define	MEI_R1_CASSETE_FULL	0x08
#define MEI_R1_LRC			0x10
#define MEI_R1_PAUSED		0x20
#define	MEI_R1_CALIBRATION	0x40

/// RECV_DATA_2
#define MEI_POWERUP			0x01
#define MEI_INVALID_CMD		0x02
#define	MEI_FAILURE			0x04
#define MEI_1K				0x08
#define	MEI_5K				0x10
#define MEI_10K				0x18
#define MEI_50K				0x20
#define MEI_ALL				(MEI_1K | MEI_5K | MEI_10K | MEI_50K)


#pragma pack(1)

typedef struct
{
	BYTE Data0;			///< Omnibus Command, Data0 : 권종 선택 
	BYTE Data1;			///< Omnibus Command, Data1 : CMD 선택 
	BYTE Data2;			///< Omnibus Command, Data2 : 모드 선택
} MEI_SEND_DATA, *PMEI_SEND_DATA;

typedef struct
{
	BYTE Data0;			///< 상태정보1
	BYTE Data1;			///< 상태정보2
	BYTE Data2;			///< 상태정보3
	BYTE Data3;			///< 상태정보4
	BYTE Model;			///< Model Code
	BYTE Reversion;		///< Reversion Code
} BILL_MEI_RECV_DATA, *PBILL_MEI_RECV_DATA;


#pragma pack()

/**
 * @brief		CBillMeiComm
 * @details		MEI 지폐인식기 통신 class
 */
class CBillMeiComm : public CMySerial 
{
public:
	CBillMeiComm(void) {};
	virtual ~CBillMeiComm(void) {};
};

/**
 * @brief		CBillMeiLogFile
 * @details		MEI 지폐인식기 로그파일 생성자
 */
class CBillMeiLogFile : public CMyLogFile 
{
public:
	CBillMeiLogFile(void) {};
	virtual ~CBillMeiLogFile(void) {};
};

//======================================================================= 

#pragma once


class CBillMEI
{
public:
	CBillMEI();
	~CBillMEI();

public :
	CBillMeiComm	m_clsComm;
	BOOL			m_bConnected;
	HANDLE			m_hAccMutex;
	HANDLE			m_hThread;

	CBillMeiLogFile	m_clsLog;
	BYTE			m_szTxBuf[MAX_MEI_BUF];
	BYTE			m_szRxBuf[MAX_MEI_BUF];

	DWORD			m_dwStatus;
	int				m_n1k, m_n5k, m_n10k, m_n50k, m_nTotalFare;
	int				m_nTmp_1k, m_nTmp_5k, m_nTmp_10k, m_nTmp_50k;
	int				m_nDebug;

	BYTE			m_byAction;
	BYTE			m_byPrevAction;

	int				m_nSendCmd;
	MEI_SEND_DATA	m_tSend;
	PBILL_MEI_RECV_DATA	m_tRecvData;

private :
	void LOG_INIT(void);
	int Locking(void);
	int UnLocking(void);

	int MakeBCC(BYTE *pData, int nDataLen);
	int MakeCommand(int nSndCommand);
	int SendPacket(BYTE *pData, int nDataLen);
	int GetPacket(BYTE *retBuf);
	int SndRecvPacket(void);
	int CheckError(BYTE byError);

	static DWORD WINAPI RunThread(LPVOID lParam);

public:

	int Reset(void);
	int GetStatus(void);
	int GetActionStatus(void);
	int Enable(void);
	int Inhibit(void);

	int GetMoney(int *n1k, int *n5k, int *n10k, int *n50k);
	int TotalEnd(void);

	int StartProcess(int nCommIdx);
	int EndProcess(void);

};

