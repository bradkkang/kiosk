// 
// 
// dev_cardreader_kicc.h : IC_CARD 리더기(KICC제품) 헤더 파일
//

#pragma once

#include <stdio.h>
#include <string.h>
#include <queue>
#include <iostream>
#include "MySerial.h"
#include "MyLogFile.h"

#define MAX_CARDRD_BUF			(1024 * 2)
#define MAX_CARDRD_RETRY		10
#define MAX_CARDRD_TIMEOUT		2000


#define KICC_CMD_INFO			0x02	///< 리더기 정보 요청
#define KICC_CMD_DETECT			0x09	///< Card Detect
#define KICC_CMD_REQ_IC_INFO	0x20	///< IC 거래 요청
#define KICC_CMD_COMPLETE_ICTR	0x18	///< IC EMV 완료 요청
#define KICC_CMD_SET_MS			0x23	///< MSR 읽기 설정
#define KICC_CMD_SPACE			0x25	///< SPACE 전송 요청
#define KICC_CMD_CARDNO_ENCDATA	0x26	///< KEY In 카드번호 암호화 데이타 요청
#define KICC_CMD_HACKING		0x30	///< 보안 침해 정보 요청
#define KICC_CMD_STATUS			0x32	///< 리더기 상태 조회

using namespace std;


typedef int  ( __stdcall *KLoad)(int Port, int Speed, char * ErrMsg);
typedef int  ( __stdcall *KReqCmd)(int CMD, int GCD, int JCD, char * SendData, char * ErrMsg);
typedef int  ( __stdcall *KGetEvent)(int& CMD, int& GCD, int& JCD, int& RCD, char * RData, char * RHexData);
typedef int  ( __stdcall *KGetEmv)(BYTE *emv);
typedef int  ( __stdcall *KWaitCmd)(int cmd, char *pRcvData, int waitTime, int waitType, char *dispMsg, char *errMsg);
typedef int  ( __stdcall *KReqReset)();
typedef int  ( __stdcall *KUnload)();

#pragma pack(1)

// queue data
typedef struct 
{
	int		nCMD;
	int		nGCD;
	int		nJCD;
	int		nRCD;
	int		nLen;
	char	szData[256];
} KICC_QUE_RDATA_T, *PKICC_QUE_RDATA_T;

///< IC 정보(0x02)

typedef struct  
{
	char	szVanCode[4];			///< VAN CODE
	char	szVanName[10];			///< VAN 명칭
	char	szPubKey[4];			///< 키수신 공개키 버젼
	char	szSecurity[2];			///< 암호화 방식
//} ICCARD_ITEM_T, *PICCARD_ITEM_T;
} CARDRD_ITEM_T, *PCARDRD_ITEM_T;

typedef struct  
{
	char	szModel[4];				///< 모델코드 (ED-946N:"301M")
	char	szVersion[4];			///< 프로그램 버젼
	char	szSerialNo[12];			///< 시리얼번호
	char	szProtocol[2];			///< 프로토콜 버젼
	char	chUseMsrTrack;			///< 사용 MSR Track
	BYTE	byMaxVan;				///< MAX VAN
	BYTE	byCount;				///< VAN 갯수
} CARDRD_RCV_INFO_T, *PCARDRD_RCV_INFO_T;

typedef struct  
{
	char	szModel[4];				///< 모델코드 (ED-946N:"301M")
	char	szVersion[4];			///< 프로그램 버젼
	char	szSerialNo[12];			///< 시리얼번호
	char	szProtocol[2];			///< 프로토콜 버젼
	char	chUseMsrTrack;			///< 사용 MSR Track
	BYTE	byMaxVan;				///< MAX VAN
	BYTE	byCount;				///< VAN 갯수
	CARDRD_ITEM_T	tItem[5];		///< 
	char	szKTCCerti[16];			///< 
//} ICCARD_INFO_T, *PICCARD_INFO_T;
} CARDRD_INFO_T, *PCARDRD_INFO_T;

///< IC 거래 요청 정보(0x20)
typedef struct  
{
	char	chTr;					///< 거래 구분자
	char	chKind;					///< 거래 종류
	char	szDate[14];				///< 거래일시
	char	szFare[9];				///< 거래금액 (Right justfy)
	char	szTermID[8];			///< 단말기 ID
	char	chEMVPin;				///< EMV PIN 설정
	char	chCard;					///< Card Input 설정('1':MS/IC, '2':MS, '3':IC, '4':KeyIn)
	char	chIsMSR;				///< MS 거래 허용여부('1':허용, '2':불가, '4':RCD 0x03 사용)
	char	szSeed[16];				///< SEED DATA
//	char	szCount[2];				///< 요청갯수
//	char	szCorp[6];				///< ????
//} DEV_IC_SND_REQTR_T, *PDEV_IC_SND_REQTR_T;
} CARDRD_SND_REQ_IC_TR_T, *PCARDRD_SND_REQ_IC_TR_T;

//typedef struct  
//{
//	char	szLength[2];			///< IC EMV 거래 데이타 길이(ASCII to Bin)
//	char	chCardType;				///< 카드 타입 ('V':Visa, 'M': Maseter, 'J':JCB 등)
//	
//
//} IC_ENCDATA_T, *PIC_ENCDATA_T;

typedef struct  
{
	char	szVanCode[4];			///< Van Code : "1400":KICC
	char	szSecurity[2];			///< 암호화 구분 
									///<	"22" : DUPKT_AES128_CBC
									///<	"23" : DUPKT_AES128_CBC(카드번호 암호화)
									///<	"24" : DUPKT_AES128_CBC(SAM 방식 암호화)
	char	szKSN[20];				///< DUKPT_KSN
	char	szEncData[130];			///< "22", "23" : Base64
									///< "24" : "41" + HexString(128)
} IC_TR_ITEM_T, *PIC_TR_ITEM_T;

typedef struct  
{
	char	szCardFlag[4];			///< 카드 구분자
	char	chCVM;					///< CVM 
	char	chIsOnline;				///< Online 여부( 'u':온라인 거래, 'v':오프라인 승인 )
	char	szCardNo[40];			///< 스크래치 카드번호
	char	szModelNo[8];			///< 기종번호(4) + 버젼(4)
	char	szCount[2];				///< 자료갯수
	IC_TR_ITEM_T tItem[100];		///< 암호화 데이타
} CARDRD_RCV_IC_TR_T, *PCARDRD_RCV_IC_TR_T;

///< IC EMV 완료 요청 정보(0x18) ?????????????
typedef struct 
{
	char	rfu[1];
} CARDRD_SND_COMPLETE_IC_TR_T, *PCARDRD_SND_COMPLETE_IC_TR_T;

///< KEY In 카드번호 암호화 데이타 요청 정보(0x26)
typedef struct 
{
	BYTE	szSeed[16];				///< 랜덤값 (Seed data)
	BYTE	szEncData[32];			///< Key In 입력된 카드번호의 암호화 데이타
} CARDRD_SND_ENCDATA_T, *PCARDRD_SND_ENCDATA_T;

typedef struct  
{
	char	szCardFlag[4];			///< 카드 구분자
	char	chCVM;					///< CVM 
	char	chIsOnline;				///< Online 여부( 'u':온라인 거래, 'v':오프라인 승인 )
	char	szCardNo[40];			///< 스크래치 카드번호
	char	szModelNo[8];			///< 기종번호(4) + 버젼(4)
	char	szCount[2];				///< 자료갯수
	IC_TR_ITEM_T tItem[100];		///< 암호화 데이타
} CARDRD_RCV_ENCDATA_T, *PCARDRD_RCV_ENCDATA_T;

///< 보안 침해 정보 요청 (0x30)
typedef struct 
{
	char	chTamper;				///< Tamper 침해유무 : 'B'(침해), 'G'(침해되지않음)
	char	chReason;				///< 침해 사유 : '0'(침해되지 않음), '1'(물리적 탐침), '2'(무결성 오류), '3'(키정보 없음)
	char	szDate[8];				///< Date Time
} CARDRD_RCV_HACKING_T, *PCARDRD_RCV_HACKING_T;

///< 리더기 상태 조회 (0x32)
typedef struct 
{
	char	szFlag[2];				///< 요청구분 : "01"(IC CARD 삽입유무)
	char	chResult;				///< 결과 : '0'(IC 카드 삽입), '1'(IC 카드 없음)
} CARDRD_RCV_IC_STATUS_T, *PCARDRD_RCV_IC_STATUS_T;

#pragma pack()


/**
 * @brief		CCardRD_KICCLogFile
 * @details		신용카드 리더기 로그파일 생성자
 */
class CCardRD_KICCLogFile : public CMyLogFile 
{
public:
	CCardRD_KICCLogFile(void) {};
	virtual ~CCardRD_KICCLogFile(void) {};
};

//======================================================================= 

class CCardRD_KICC
{
public:
	CCardRD_KICC();
	~CCardRD_KICC();

private :
	HINSTANCE	m_hDll;
	BOOL		m_bConnected;
	HANDLE		m_hAccMutex;
	HANDLE		m_hThread;

	int			m_nSendCommand;

	CCardRD_KICCLogFile m_clsLog;

protected:
	void LOG_INIT(void);
	int Locking(void);
	int UnLocking(void);
	int SendData(int nCommand, char *pData, int nDataLen);
	int RecvData(void);

	int FnInitialize(void);

	static DWORD WINAPI RunThread(LPVOID lParam);

	KLoad		m_fnKLoad;
	KReqCmd		m_fnKReqCmd;
	KGetEvent	m_fnKGetEvent;
	KGetEmv		m_fnKEmv;
	KWaitCmd	m_fnKWaitCmd;
	KReqReset	m_fnKReqReset;
	KUnload 	m_fnKUnload;

	BOOL		m_bOperate;

public:

	int			m_nCMD, m_nGCD, m_nJCD, m_nRCD;
	char		m_szErrMsg[MAX_CARDRD_BUF];
	char		m_szRxData[MAX_CARDRD_BUF];
//	char		m_szErrMsg[1024];

	char		m_szRxHexData[MAX_CARDRD_BUF * 2];

	int			m_nTimeOut;

	queue <KICC_QUE_RDATA_T> m_QueRcvData;

	CARDRD_INFO_T		m_tInfo;
	CARDRD_RCV_IC_TR_T	m_tICTr;
	CARDRD_RCV_ENCDATA_T m_tEncData;

	int GetRandomSeed(int nLen, char *retBuf);

	int Reset(void);
	int GetDeviceInfo(void);
	int ReqIcCard(char *pData, int nDataLen);
	int SetMSReadInfo(BOOL bSet);
	int ReqSpaceInfo(void);
	int GetCardNoEncData(char *pData, int nDataLen);
	int ReqHackingInfo(void);
	int GetStatusInfo(void);
	int GetEmvInfo(BYTE *pEMV);


	int Polling(void);
	int ParsingPacket(int nJCD, char *pRecvData, int nRecvLen);

	int StartProcess(int nCommIdx);
	int EndProcess(void);

	int CheckQueData(char *retBuf);
};
