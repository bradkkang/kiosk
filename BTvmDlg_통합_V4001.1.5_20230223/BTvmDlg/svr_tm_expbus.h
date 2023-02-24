// 
// 
// svr_tm_expbus.h : 티머니고속버스 서버 헤더 파일
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

#define TMAX_TR_BEGIN		1
#define TMAX_TR_COMMIT		2
#define TMAX_TR_ROLLBACK	3
#define TMAX_TR_TIMEOUT		4

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CTmExpBusServerLogFile
 * @details		티머니 고속 서버 로그파일 class
 */
class CTmExpBusServerLogFile : public CMyLogFile 
{
public:
	CTmExpBusServerLogFile(void) {};
	virtual ~CTmExpBusServerLogFile(void) {};
};

//======================================================================= 

using namespace std;

class CTmExpBusServer
{
public:
	CTmExpBusServer();
	~CTmExpBusServer();

protected :
	int			m_nTimeOut;			// 초단위

	FBUF*		m_pSendBuff;
	FBUF*		m_pRecvBuff;

	CTmExpBusServerLogFile m_clsLog;

	char		m_szBuffer[1024];
	BOOL		m_bVerify;
	BOOL		m_bLog;

public:
	int			m_nRecNcnt1;

	int			m_nTPCallFlag;

private :
	void LOG_INIT(void);

	int TMaxAlloc(void);
	void TMaxFree(void);
	int TMaxConnect(void);
	void TMaxDisconnect(void);
	void TMaxGetConvChar(DWORD dwFieledKey, char *retBuf);

	int TMaxTransaction(int nTrKind);

	int TMaxTpCall(char *pService, int nTimeout);
	int TMaxFBPut(DWORD dwFieldKey, char *pData);
	int TMaxFBPutLen(DWORD dwFieldKey, char *pData, int nDataLen);
	int TMaxFBInsert(DWORD dwFieldKey, char *pData, int nIndex, int nDataLen);
	int TMaxFBGet(DWORD dwFieldKey, char *pData);
	int TMaxFBGetTu(DWORD dwFieldKey, char *pData, int nth);
	int TMaxFBGetF(DWORD dwFieldKey, char *pData);

	int TMaxFBUpdate(DWORD dwFieldKey, char *pData, int nIndex, int nDataLen);


	void LogOut_Header(char *pData);
	void SendHeader(char *pData);


public:

	/// 기초정보
	int SVC_CM_ReadMsg(int nOperID, int nIndex, char *pData, char *retBuf);
	int SVC_CM_ReadNtc(int nOperID, char *pData, char *retBuf);
	int SVC_TK_AuthCmpt(int nOperID, char *pData, char *retBuf);
	int SVC_CM_ReadCmnCd(int nOperID, int nIndex, char *pData, char *retBuf);
	int SVC_TK_ReadTckPrtg(int nOperID, char *pData, char *retBuf);
	int SVC_MG_ReadTrmlDrtn(int nOperID, char *pData, char *retBuf);
	int SVC_TK_ReadOwnrTrml(int nOperID, char *pData, char *retBuf);
	int SVC_CM_ReadTrml(int nOperID, int nIndex, char *pData, char *retBuf);
	int SVC_CM_ReadTrmlStup(int nOperID, char *pData, char *retBuf);
	int SVC_CM_ReadTrmlInf(int nOperID, char *pData, char *retBuf);
	int SVC_MG_ReadWnd(int nOperID, char *pData, char *retBuf);
	int SVC_CM_ReadRtrpTrml(int nOperID, char *pData, char *retBuf);
	int SVC_CM_ReadTckKnd(int nOperID, char *pData, char *retBuf);
	int SVC_CM_MngCacm(int nOperID, int nIndex, char *pData, char *retBuf);
	int SVC_CM_ReadRotInf(int nOperID, char *pData, char *retBuf);
	int SVC_CM_ReadRyrt(int nOperID, char *pData, char *retBuf);

	/// ~기초정보


	int SVC_ChangeTicketBox(char *pData, char *retBuf);
	int SVC_CloseWnd(char *pData, char *retBuf);

	/// 현장발권
	int SVC_MG_ReadAlcn(char *pData, char *retBuf);
	int SVC_CM_ReadSatsFee(char *pData, char *retBuf);
	int SVC_TK_PcpySats(char *pData, char *retBuf);
	int SVC_TK_PcpySatsCancel(char *pData, char *retBuf);
	int SVC_CM_ReadThruTrml(char *pData, char *retBuf);

	int SVC_TK_PubTckCash(char *pData, char *retBuf);
	int SVC_TK_PubTckCard(char *pData, char *retBuf);
	int SVC_TK_PubTckCard_KTC(char *pData, char *retBuf);
	int SVC_TK_PubTckCsrc(char *pData, char *retBuf);
	int SVC_TK_PubTckCsrc_KTC(char *pData, char *retBuf);
	int SVC_TK_PubTckPrd(char *pData, char *retBuf);

	/// 예매발권
	int SVC_TK_ReadMrs(char *pData, char *retBuf);
	int SVC_TK_KtcReadMrs(char *pData, char *retBuf);
	int SVC_TK_PubMrs(char *pData, char *retBuf);
	int SVC_TK_PubMrsMobile(char *pData, char *retBuf);

	/// 환불
	int SVC_TK_ReadBusTckno(char *pData, char *retBuf);
	int SVC_TK_CancRyTck(char *pData, char *retBuf);


	/// 발행내역 조회
	int SVC_CM_PubInquiry(int nOper, char *pData, char *retBuf);
	/// 재발행 내역 조회
	int SVC_CM_RePubInquiry(int nOper, char *pData, char *retBuf);
	/// 재발행
	int SVC_TK_RePubTck(char *pData, char *retBuf);
	/// 환불 내역 조회
	int SVC_CM_CanRyInquiry(int nOperID, char *pData, char *retBuf);


	int StartProcess(void);
	int EndProcess(void);
};
