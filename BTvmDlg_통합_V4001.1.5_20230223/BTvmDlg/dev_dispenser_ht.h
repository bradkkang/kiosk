// 
// 
// dev_dispenser_ht.h : 한틀 지폐방출기 헤더 파일
//

#pragma once

#include "MySerial.h"
#include "MyLogFile.h"


#define MAX_CDU_BUF		200
#define MAX_CDU_RETRY	10

#pragma pack(1)

typedef struct
{
//	BYTE	bySTX;
	WORD	wDataLen;
	BYTE	byResult;							///> 0x4F('O') : Success, 0x46('F') : Fail
	BYTE	byData;
} HSCDU_HEADER_T, *PHSCDU_HEADER_T;

/**
 * @details		Error Code 구조체
 */
typedef struct
{
	char*	pCode;
	char*	pStr;
} HSCDU_ERROR_TBL_T, *PHSCDU_ERROR_TBL_T;

/**
 * @details		Init Response 구조체
 */
typedef struct
{
	BYTE	szErrorCode[5];
} HSCDU_RESP_INIT_T, *PHSCDU_RESP_INIT_T;

/**
 * @details		Sensor Status Response 구조체
 */
typedef struct
{
	BYTE	szSensorData[13];
} HSCDU_RESP_STATUS_T, *PHSCDU_RESP_STATUS_T;

/**
 * @details		Get Version Response 구조체
 */
typedef struct
{
	BYTE	szUnitName[3];						///> "CDU"
	BYTE	byCountry;							///> 'U'
	BYTE	byCassette;							///> 카셋트 번호
#define HSCDU_CASSETTE_1		0x31
#define HSCDU_CASSETTE_2		0x32
#define HSCDU_CASSETTE_3		0x33
#define HSCDU_CASSETTE_4		0x34

	BYTE	byType;								///> CDU Type (0x31)
	BYTE	szVersion[7];						///> Version
	BYTE	szFixReserved[4];					///> All 0x04
	BYTE	byReserved;							///> 0x00
} HSCDU_RESP_VERSION_T, *PHSCDU_RESP_VERSION_T;

/**
 * @details		Dispense Request 구조체
 */
typedef struct
{
	BYTE	byCassette_1;						///> 카셋트1번 dispense 갯수 (1 ~ 149)
	BYTE	byCassette_2;						///> 카셋트2번 dispense 갯수 (1 ~ 149)
	BYTE	byCassette_3;						///> 카셋트3번 dispense 갯수 (1 ~ 149)
	BYTE	byCassette_4;						///> 카셋트4번 dispense 갯수 (1 ~ 149)
	BYTE	szReserved[10];						///> Reserved

} HSCDU_SND_DISPENSE_T, *PHSCDU_SND_DISPENSE_T;

/**
 * @details		Dispense Response 구조체
 */
typedef struct
{
	BYTE	szErrorCode[5];					///> 
	BYTE	szResultInfo[52];					///> Refer to "HSCDU_RESULT_INFO_T"
} HSCDU_RESP_DISPENSE_T, *PHSCDU_RESP_DISPENSE_T;

typedef struct
{
	BYTE	byPass;							///> 방출된 갯수
	BYTE	byRequired;						///> mcu에서 요청한 갯수
	BYTE	byStacked;						///> 
	BYTE	byReject1;						///> 스태커#1에 reject 갯수
	BYTE	byReject2;						///> 스태커#2에 reject 갯수
	BYTE	byReject3;						///> 
	BYTE	byReject4;						///> 
	BYTE	byReject5;						///> 
	BYTE	byTotReject;					///> 
	BYTE	byReject6;						///> 
	BYTE	szRfu1[3];							///>
} CASSETTE_INFO, *PCASSETTE_INFO;

typedef struct
{
	CASSETTE_INFO casset_1;
	CASSETTE_INFO casset_2;
	CASSETTE_INFO casset_3;
	CASSETTE_INFO casset_4;
} HSCDU_RESULT_INFO_T, *PHSCDU_RESULT_INFO_T;


#pragma pack()


#define HSCDU_CMD_INIT				0x49		///> 'I'
#define HSCDU_CMD_STATUS			0x53		///> 'S'
#define HSCDU_CMD_VERSION			0x56		///> 'V'
#define HSCDU_CMD_DISPENSE			0x44		///> 'D'
#define HSCDU_CMD_TEST_DISPENSE		0x52		///> 'R'
#define HSCDU_CMD_FW_DOWNLOAD		0x55		///> 'U'
#define HSCDU_CMD_INFO				0x54		///> 'T'

/**
 * @brief		CCduHsLogFile
 * @details		로그파일 생성자
 */
class CCduHtComm : public CMySerial 
{
public:
	CCduHtComm(void) {};
	virtual ~CCduHtComm(void) {};
};

/**
 * @brief		CCduHsLogFile
 * @details		로그파일 생성자
 */
class CCduHtLogFile : public CMyLogFile 
{
public:
	CCduHtLogFile(void) {};
	virtual ~CCduHtLogFile(void) {};
};

//======================================================================= 

#pragma once


class CCduHt
{
public:
	CCduHt();
	~CCduHt();

private :
	CCduHtComm	m_clsComm;
	BOOL		m_bConnected;
	HANDLE		m_hAccMutex;
	HANDLE		m_hThread;

	CCduHtLogFile	m_clsLog;

	BYTE		m_szTxBuf[MAX_CDU_BUF];
	BYTE		m_szRxBuf[MAX_CDU_BUF];

public:
	int			m_nOutCount[4];			/// 카세트 1 ~ 4번, 지금 적용 모델은 카세트 1번 ~ 2번
	int			m_nRejectCount[4];		/// 불량카세트함으로 이동한 갯수 1 ~ 4번, 지금 적용 모델은 카세트 1번 ~ 2번

protected:
	void LOG_INIT(void);
	int Locking(void);
	int UnLocking(void);
	int SendChar(BYTE byData);
	int SendPacket(int nCommand, BYTE *pData, WORD wDataLen);
	int GetPacket(BYTE *retBuf);
	int SndRcvPacket(int nCommand, BYTE *pData, int nDataLen, BYTE *retBuf);

	int CheckResult(char *pErrString);

	static DWORD WINAPI RunThread(LPVOID lParam);

public:

	int Initialize(int nInitFlag);
	int SensorStatus(void);
	int GetStatus(void);
	int GetVersion(void);
	int Dispense(int nCount1, int nCount2);
	int TestDispense(int nCount1, int nCount2);
	int GetDispenseInfo(char *pData);
	int SetInfo(void);

	int StartProcess(int nCommIdx);
	int EndProcess(void);

};
