// 
// 
// svr_kobus.h : 고속 코버스 서버 헤더 파일
//

#pragma once

#include "MySerial.h"
#include "MyLogFile.h"
#include "svr_kobus_st.h"

#include <iostream>
#include <vector>

#include <usrinc/atmi.h>
#include <usrinc/tmaxapi.h>
#include <usrinc/fbuf.h>

//----------------------------------------------------------------------------------------------------------------------

#define MAX_KOEXPSVR_BUF		(1024 * 6)
#define MAX_KOEXPSVR_RETRY		10
#define MAX_KOEXPSVR_LIST		10000

//----------------------------------------------------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CKoBusSvrLogFile
 * @details		고속버스(KoBus) 서버 로그파일 class
 */
class CKoBusSvrLogFile : public CMyLogFile 
{
public:
	CKoBusSvrLogFile(void) {};
	virtual ~CKoBusSvrLogFile(void) {};
};

//======================================================================= 

using namespace std;

class CKoBusSvr
{
public:
	CKoBusSvr();
	~CKoBusSvr();

protected :
	int			m_nTimeOut;			// 초단위

	FBUF*		m_pSendBuff;
	FBUF*		m_pRecvBuff;

	CKoBusSvrLogFile m_clsLog;

	char		m_szBuffer[1024];
	BOOL		m_bVerify;

public:

	int			m_nRecNcnt1;
	BOOL		m_bLog;

	/// 코버스 - 승차권 종류 정보 (한국어/영어/중국어/일본어)
	vector<rtk_cm_cdinqr_list_t>		m_vtKorTck; 
	vector<rtk_cm_cdinqr_list_t>		m_vtEngTck; 
	vector<rtk_cm_cdinqr_list_t>		m_vtChiTck; 
	vector<rtk_cm_cdinqr_list_t>		m_vtJpnTck; 

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
	void TMaxGetConvChar(DWORD dwFieledKey, char *retBuf);


	int AssignTicketKind(char *cd_val, char *seq, char *retBuf);


public:

	int SVC_CM_ReadMsg(int nOperID, int nIndex, char *pData, char *retBuf);
	int SVC_CM_ReadNtc(int nOperID, char *pData, char *retBuf);
	int SVC_CM_ReadCmnCd(int nOperID, int nIndex, char *pData, char *retBuf);
	int SVC_CM_RdhmInqr(int nOperID, int nIndex, char *pData, char *retBuf);
	int SVC_TK_ReadTckPrtg(int nOperID, char *pData, char *retBuf);
	int SVC_CM_ReadTrmlInf(int nOperID, int nIndex, char *pData, char *retBuf);
	int SVC_MG_ReadWnd(int nOperID, char *pData, char *retBuf);
	int SVC_CM_ReadRtrpTrml(int nOperID, char *pData, char *retBuf);
	int SVC_CM_ReadTckKnd(int nOperID);
	int SVC_CM_MngCacm(int nOperID, int nIndex, char *pData, char *retBuf);
	int SVC_CM_ReadRotInf(int nOperID, char *pData, char *retBuf);

	///> 현장발권 - 배차조회 1/4
	int SVC_MG_ReadAlcn(char *pData, char *retBuf);
	///> 현장발권 - 좌석조회 2/4
	int SVC_CM_ReadSatsFee(char *pData, char *retBuf);
	///> 현장발권 - 좌석선점 3/4
	int SVC_TK_PcpySats(char *pData, char *retBuf);
	///> 현장발권 - 좌석선점 취소 3/4
	int SVC_TK_PcpySatsCancel(char *pData, char *retBuf);
	///> 현장발권 - 발권 4/4
	int SVC_TK_PubTck(char *pData, char *retBuf);

	///> 예매조회 - 1/2
	int SVC_TK_ReadMrs(char *pData, char *retBuf);
	///> 예매발행 - 2/2
	int SVC_TK_PubMrs(char *pData, char *retBuf);

	///> 환불 대상 금액 조회
	int SVC_TK_RyAmtInfo(char *pData, char *retBuf);
	///> 환불 처리 - 무인기
	int SVC_TK_RepTran(char *pData, char *retBuf);
	///> 환불 처리 - 창구
	int SVC_TK_TckCan(char *pData, char *retBuf);


	int SVC_ChangeTicketBox(char *pData, char *retBuf);
	int SVC_CloseWnd(char *pData, char *retBuf);
	int SVC_InqPubPt(int nOper, char *pData, char *retBuf);
	int SVC_RePubTck(char *pData, char *retBuf);

	int SVC_TK_AuthCmpt(void);
	int SVC_CM_ReadTrml(void);
	int SVC_CM_ReadTrmlStup(void);
	int SVC_CM_ReadRotInf(void);

	int StartProcess(void);
	int EndProcess(void);

	int ServerBasicData(void);
};
