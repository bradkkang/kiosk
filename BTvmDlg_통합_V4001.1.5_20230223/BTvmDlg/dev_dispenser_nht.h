// 
// 
// dev_dispenser_nht.h : 신규 한틀 지폐방출기 헤더 파일
//

#pragma once

#include "MySerial.h"
#include "MyLogFile.h"


#define MAX_NCDU_BUF		200
#define MAX_NCDU_RETRY		10

#pragma pack(1)

typedef struct
{
	WORD	wDataLen;
	BYTE	byResult;							///> 0x4F('O') : Success, 0x46('F') : Fail
	BYTE	byData;
} NHTCDU_HEADER_T, *PNHTCDU_HEADER_T;

/**
 * @details		Error Code 구조체
 */
typedef struct
{
	char*	pCode;
	char*	pStr;
} NHTCDU_ERROR_TBL_T, *PNHTCDU_ERROR_TBL_T;

/**
 * @details		Init Response 구조체
 */
typedef struct
{
	BYTE	szErrorCode[5];
} NHTCDU_RESP_COMMON_T, *PNHTCDU_RESP_COMMON_T;


/**
 * @details		Init Response 구조체
 */
typedef struct
{
	BYTE	szErrorCode[5];
} NHTCDU_RESP_INIT_T, *PNHTCDU_RESP_INIT_T;

/**
 * @details		Sensor Status Response 구조체
 */
typedef struct
{
	BYTE	szSensorData[13];
} NHTCDU_RESP_STATUS_T, *PNHTCDU_RESP_STATUS_T;

/**
 * @details		Get Version Response 구조체
 */
typedef struct
{
	BYTE	szUnitName[3];						///> "CDU"
	BYTE	byCountry;							///> 'K'
	BYTE	byCassette;							///> 카셋트 번호
#define NHTCDU_CASSETTE_1		0x31
#define NHTCDU_CASSETTE_2		0x32
#define NHTCDU_CASSETTE_3		0x33
#define NHTCDU_CASSETTE_4		0x34

	BYTE	byType;								///> CDU Type (0x31)
	BYTE	szVersion[7];						///> Version
	BYTE	szFixReserved[4];					///> All 0x04
	BYTE	byReserved;							///> 0x00
} NHTCDU_RESP_VERSION_T, *PNHTCDU_RESP_VERSION_T;

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

} NHTCDU_SND_DISPENSE_T, *PNHTCDU_SND_DISPENSE_T;

/**
 * @details		Dispense Response 구조체
 */
typedef struct
{
	BYTE	szErrorCode[5];					///> 
	BYTE	szResultInfo[78];				///> 카세트#1 ~카세트#6
} NHTCDU_RESP_DISPENSE_T, *PNHTCDU_RESP_DISPENSE_T;

typedef struct
{
	BYTE	byReqCount;						///> 방출요구 매수
	BYTE	byPass_CassExit;				///> 카세트 출구 통과 매수(SKEW 1/2 센서)
	BYTE	byPass_CIS;						///> CIS 통과 매수 (SCAN_START 센서)
	BYTE	byPass_GateSens;				///> Gate Sensor 통과 매수
	BYTE	byTotalOutCount;				///> Total Exit 매수
	BYTE	byTotalRejectCount;				///> Total Reject 매수
	BYTE	byReject_Skew;					///> Skew로 인한 reject 매수
	BYTE	byReject_ShortGap;				///> Short Gap으로 인한 reject 매수
	BYTE	byReject_LongNote;				///> Long Note으로 인한 reject 매수
	BYTE	byReject_ShortNote;				///> Short Note으로 인한 reject 매수
	BYTE	byReject_DoubleNote;			///> Double Note으로 인한 reject 매수
	BYTE	byReject_ImgBoard;				///> 이미지 분석 보드의 지정으로 인한 reject 매수
	BYTE	byReject_Etc;					///> 상위국의 지정으로 인한 reject 매수

} NHTCASSETTE_INFO, *PNHTCASSETTE_INFO;

typedef struct
{
	NHTCASSETTE_INFO casset_1;
	NHTCASSETTE_INFO casset_2;
	NHTCASSETTE_INFO casset_3;
	NHTCASSETTE_INFO casset_4;
	NHTCASSETTE_INFO casset_5;
	NHTCASSETTE_INFO casset_6;
} NHTCDU_RESULT_INFO_T, *PNHTCDU_RESULT_INFO_T;



#pragma pack()


#define NHTCDU_CMD_INIT				0x49		///> 'I'
#define NHTCDU_CMD_STATUS			0x53		///> 'S'
#define NHTCDU_CMD_VERSION			0x56		///> 'V'
#define NHTCDU_CMD_DISPENSE			0x44		///> 'D'
#define NHTCDU_CMD_TEST_DISPENSE	0x52		///> 'R'
#define NHTCDU_CMD_FW_DOWNLOAD		0x55		///> 'U'
//#define NHTCDU_CMD_INFO				0x54		///> 'T'

/**
 * @brief		CCduNHtComm
 * @details		통신 Class
 */
class CCduNHtComm : public CMySerial 
{
public:
	CCduNHtComm(void) {};
	virtual ~CCduNHtComm(void) {};
};

/**
 * @brief		CCduNHtLogFile
 * @details		로그파일 class
 */
class CCduNHtLogFile : public CMyLogFile 
{
public:
	CCduNHtLogFile(void) {};
	virtual ~CCduNHtLogFile(void) {};
};

//======================================================================= 

#pragma once


class CCduNHt
{
public:
	CCduNHt();
	~CCduNHt();

private :
	CCduNHtComm	m_clsComm;
	BOOL		m_bConnected;
	HANDLE		m_hAccMutex;
	HANDLE		m_hThread;

	BYTE		m_szTxBuf[MAX_NCDU_BUF];
	BYTE		m_szRxBuf[MAX_NCDU_BUF];
	CCduNHtLogFile	m_clsLog;


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
	int SetInfo(void);
	int GetDispenseInfo(char *pData);

	int StartProcess(int nCommIdx);
	int EndProcess(void);

};
