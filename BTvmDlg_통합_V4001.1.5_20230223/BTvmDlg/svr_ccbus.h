// 
// 
// svr_ccbus.h : 시외버스 서버 헤더 파일
//

#pragma once

#include "MySerial.h"
#include "MyLogFile.h"

#include <usrinc/atmi.h>
#include <usrinc/tmaxapi.h>
#include <usrinc/fbuf.h>

//----------------------------------------------------------------------------------------------------------------------

#define MAX_CCSVR_BUF		(1024 * 6)
#define MAX_CCSVR_RETRY		10
#define MAX_CCSVR_LIST		10000

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CCBusServerLogFile
 * @details		시외버스 서버 로그파일 class
 */
class CCBusServerLogFile : public CMyLogFile 
{
public:
	CCBusServerLogFile(void) {};
	virtual ~CCBusServerLogFile(void) {};
};

//======================================================================= 

using namespace std;

class CCBusServer
{
public:
	CCBusServer();
	~CCBusServer();

protected :
	int			m_nTimeOut;			// 초단위

	FBUF*		m_pSendBuff;
	FBUF*		m_pRecvBuff;

	CCBusServerLogFile m_clsLog;

	char		m_szBuffer[1024];
	BOOL		m_bVerify;

private :
	void LOG_INIT(void);

	int TMaxAlloc(void);
	void TMaxFree(void);
	int TMaxConnect(void);
	void TMaxDisconnect(void);

	int TMaxTpCall(char *pService, int nTimeout);
	int TMaxFBPut(DWORD dwFieldKey, char *pData);
	int TMaxFBPutLen(DWORD dwFieldKey, char *pData, int nDataLen);
	int TMaxFBInsert(DWORD dwFieldKey, char *pData, int nIndex, int nDataLen);
	int TMaxFBGet(DWORD dwFieldKey, char *pData);
	int TMaxFBGetTu(DWORD dwFieldKey, char *pData, int nth);
	int TMaxFBGetF(DWORD dwFieldKey, char *pData);

	int TMaxFBUpdate(DWORD dwFieldKey, char *pData, int nIndex, int nDataLen);


public:
	void LogOut_Hdr(char *pData);
	void LogOut_HdrShort(char *pData);
	void LogOut_HdrId(char *pData);

	void SendHeader(char *pData);
	void SendShortHeader(char *pData);
	void SendIDHeader(char *pData);

	int TMaxSvc100(char *pData, char *retBuf);
	int TMaxSvc101(char *pData, char *retBuf);
	int TMaxSvc102(char *pData, char *retBuf);
	int TMaxSvc106(char *pData, char *retBuf);
	int TMaxSvc107(char *pData, char *retBuf);
	int TMaxSvc108(char *pData, char *retBuf);

	int TMaxSvc114(char *pData, char *retBuf);
	int TMaxSvc118(char *pData, char *retBuf);
	int TMaxSvc119(char *pData, char *retBuf);
	int TMaxSvc120(char *pData, char *retBuf);
	int TMaxSvc122(char *pData, char *retBuf);
	int TMaxSvc124(char *pData, char *retBuf);
	int TMaxSvc125(char *pData, char *retBuf);
	int TMaxSvc126(char *pData, char *retBuf);
	int TMaxSvc127(char *pData, char *retBuf);
	int TMaxSvc128(char *pData, char *retBuf);
	int TMaxSvc129(char *pData, char *retBuf);
	
	/// 배차조회
	int TMaxSvc130(char *pData, char *retBuf);
	/// 배차요금조회
	int TMaxSvc131(char *pData, char *retBuf);
	int TMaxSvc132(char *pData, char *retBuf);
	int TMaxSvc134(char *pData, char *retBuf);
	
	int TMaxSvc135(char *pData, char *retBuf);
	int TMaxSvc136(char *pData, char *retBuf);

	/// 예매조회
	int TMaxSvc137(char *pData, char *retBuf);
	/// 예매발행
	int TMaxSvc138(char *pData, char *retBuf);

	int TMaxSvc145(char *pData, char *retBuf);

	int TMaxSvc150(char *pData, char *retBuf);

	int TMaxSvc152(char *pData, char *retBuf);
	int TMaxSvc155(char *pData, char *retBuf);
	int TMaxSvc157(char *pData, char *retBuf);
	int TMaxSvc158(char *pData, char *retBuf);

	int TMaxSvc164(char *pData, char *retBuf);

	int TMaxSvc200(char *pData, char *retBuf);
	int TMaxSvc201(char *pData, char *retBuf);
	int TMaxSvc202(char *pData, char *retBuf);
	int TMaxSvc278(char *pData, char *retBuf);		// 20221205 ADD

	int TMaxSvc208(char *pData, char *retBuf);
	int TMaxSvc209(char *pData, char *retBuf);
	
	int TMaxSvc213(char *pData, char *retBuf);
	int TMaxSvc217(char *pData, char *retBuf);
	
	int TMaxSvc221(char *pData, char *retBuf);
	int TMaxSvc222(char *pData, char *retBuf);
	int SVC_TK_ReadRsd(char *pData, char *retBuf);

	int TMaxSvc274(char *pData, char *retBuf); 		// 20221017 ADD

	int TMaxSvc228(char *pData, char *retBuf);
	int TMaxSvc230(char *pData, char *retBuf);

	int TMaxSvc260(char *pData, char *retBuf);
	int TMaxSvc268(char *pData, char *retBuf);
	int TMaxSvc269(char *pData, char *retBuf);


	int TMaxSvc501(char *pData, char *retBuf);
	int TMaxSvc502(char *pData, char *retBuf);

	int StartProcess(void);
	int EndProcess(void);
};
