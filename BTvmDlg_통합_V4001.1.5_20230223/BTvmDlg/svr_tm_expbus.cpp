// 
// 
// svr_tm_expbus.cpp : 티머니 고속버스 서버
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <winsock2.h>
#include <afxsock.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "MyFileUtil.h"

//#include "xtzbus_fdl.h"
//#include "xtzbus_tm_fdl_108.h"	// 20211013 DEL
#include "xtzbus_tm_fdl_v113.h"	// 20211013 ADD
#include "svr_tm_expbus.h"

#include <usrinc/tx.h>

#include "svr_tm_expbus_st.h"
#include "oper_config.h"
#include "oper_kos_file.h"
#include "dev_tr_main.h"
#include "dev_tr_mem.h"
#include "dev_tr_tmexp_mem.h"
#include "damo_ktc.h"
#include "data_main.h"
#include "File_Env_ini.h"

//----------------------------------------------------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------------------------------------------------

#define __USE_KTC__			0
#define __USE_LOG__			1


//----------------------------------------------------------------------------------------------------------------------

#define ST_FIELD(my_struct, field)		my_struct.##field	
#define PST_FIELD(my_struct, field)		my_struct->##field	

#define ST_SIZE(my_struct, field)		sizeof(my_struct.##field)
#define PST_SIZE(my_struct, field)		sizeof(my_struct->##field)

//----------------------------------------------------------------------------------------------------------------------

#define LOG_OUT(fmt, ...)		{ CTmExpBusServer::m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ CTmExpBusServer::m_clsLog.HexaDump(x, y, z); }

//#define LOG_OUT(fmt, ...)		
//#define LOG_HEXA(x,y,z)			

#define LOG_OPEN()				{ CTmExpBusServer::m_clsLog.LogOpen(FALSE);  }
#define LOG_WRITE(fmt, ...)		{ CTmExpBusServer::m_clsLog.LogWrite("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_CLOSE()				{ CTmExpBusServer::m_clsLog.LogClose();  }

//----------------------------------------------------------------------------------------------------------------------

static BOOL cmpMrnpList(const rtk_readmrnppt_list_t &p1, const rtk_readmrnppt_list_t &p2)
{
	char szBuff1[200];
	char szBuff2[200];

	sprintf(szBuff1, "%s%s", p1.depr_dt, p1.depr_time);
	sprintf(szBuff2, "%s%s", p2.depr_dt, p2.depr_time);

	if( memcmp(szBuff1, szBuff2, strlen(szBuff1)) < 0)
		return TRUE;
	return FALSE;
}

/**
 * @brief		CTmExpBusServer
 * @details		생성자
 */
CTmExpBusServer::CTmExpBusServer()
{
	m_pSendBuff = NULL;
	m_pRecvBuff = NULL;

	m_bVerify = FALSE;
	m_bLog = TRUE;

	m_nTPCallFlag = TPNOTRAN;
}

/**
 * @brief		~CTmExpBusServer
 * @details		소멸자
 */
CTmExpBusServer::~CTmExpBusServer()
{
	EndProcess();
}

/**
 * @brief		LOG_INIT
 * @details		LOG 파일 초기화
 * @param		None
 * @return		None
 */
void CTmExpBusServer::LOG_INIT(void)
{
	m_clsLog.SetData(30, "\\Log\\TMAX\\tmexp");
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		Connect
 * @details		서버 접속
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::TMaxConnect(void)
{
#if (__USE_TMAX__ > 0)
	int		nRet;
	char	szCurrPath[256];
	char	szFullName[256];
	TPSTART_T* tpInfo = NULL;

	LOG_WRITE(" start !!!!");

	m_nTPCallFlag = TPNOTRAN;

	::ZeroMemory(szCurrPath, sizeof(szCurrPath));
	::ZeroMemory(szFullName, sizeof(szFullName));

	Util_GetModulePath(szCurrPath);
	sprintf(szFullName, "%s\\tmax.env", szCurrPath);

	if( GetEnv_IsRealMode() == 0 )
	{	// test mode
		LOG_WRITE("TMAX_TMEXP_TEST mode !!!!");
		nRet = tmaxreadenv(szFullName, "TMAX_TMEXP_TEST" );
	}
	else
	{	// real mode
		LOG_WRITE("TMAX_TMEXP_REAL mode !!!!");
		nRet = tmaxreadenv(szFullName, "TMAX_TMEXP_REAL" );
	}

	if(nRet == -1)
	{
		LOG_WRITE("tmaxreadenv() failure !!!!");
		goto tmax_fail_proc;
	}

	LOG_WRITE("%s ","@@@@@@ tpalloc() call !!!");
	tpInfo = (TPSTART_T *) tpalloc("TPSTART", NULL, 10240);
	if(tpInfo == NULL)
	{
		LOG_WRITE("tpalloc(TPSTART) failure !!!!");
		goto tmax_fail_proc;
	}

	sprintf(tpInfo->cltname, "");
	sprintf(tpInfo->usrname, "atec_tmexp_kiosk");
	sprintf(tpInfo->dompwd, "");

	LOG_WRITE("%s ","@@@@@@ tpstart() call !!!");
	nRet = tpstart(tpInfo);
	if(nRet == -1)
	{
		LOG_WRITE("tpstart() failure !!!!");
		goto tmax_fail_proc;
	}

	if(tpInfo != NULL)
	{
		LOG_WRITE("%s ","@@@@@@ tpfree() call !!!");
		tpfree((char *)tpInfo);
		tpInfo = NULL;
	}

	return nRet;

tmax_fail_proc:

	/// 네트워크 오류 메시지
	CConfigTkMem::GetInstance()->SetErrorCode("SVE111");

	if(tpInfo != NULL)
	{
		tpfree((char *)tpInfo);
		tpInfo = NULL;
	}
	return -1;

#else
	return 0;
#endif
}

/**
 * @brief		TMaxFBPut
 * @details		필드 형식으로 데이타 전송
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::TMaxFBPut(DWORD dwFieldKey, char *pData)
{
	int nRet = 0;

#if (__USE_TMAX__ > 0)
	nRet = fbput(m_pSendBuff, (FLDKEY) dwFieldKey, pData, 0);

	if(m_bVerify == TRUE)
	{
		int nLen = 0, nPos = 0;

		::ZeroMemory(m_szBuffer, sizeof(m_szBuffer));
		nRet = fbgetf(m_pSendBuff, (FLDKEY) dwFieldKey, m_szBuffer, &nLen, &nPos);
		LOG_WRITE("Verify#0 %s =  (%s):(%s) ", fbget_fldname((FLDKEY)dwFieldKey), pData, m_szBuffer);
	}
#endif

	return nRet;
}

/**
 * @brief		TMaxFBPutLen
 * @details		필드 형식으로 데이타 전송
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::TMaxFBPutLen(DWORD dwFieldKey, char *pData, int nDataLen)
{
	int nRet = 0, nLen = 0, nPos = 0;

#if (__USE_TMAX__ > 0)
	nRet = fbput(m_pSendBuff, (FLDKEY) dwFieldKey, pData, nDataLen);

	if(m_bVerify == TRUE)
	{
//		int nLen = 0, nPos = 0;

		::ZeroMemory(m_szBuffer, sizeof(m_szBuffer));
		nRet = fbgetf(m_pSendBuff, (FLDKEY) dwFieldKey, m_szBuffer, &nLen, &nPos);
		//LOG_WRITE("Verify#1 dwFieldKey %lu, (%s):(%s) ", dwFieldKey, pData, m_szBuffer);
		LOG_WRITE("Verify#1 %s =  (%s):(%s) ", fbget_fldname((FLDKEY)dwFieldKey), pData, m_szBuffer);
	}
#endif

	return nRet;
}

/**
 * @brief		TMaxFBInsert
 * @details		필드 형식으로 데이타 전송
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::TMaxFBInsert(DWORD dwFieldKey, char *pData, int nIndex, int nDataLen)
{
	int nRet;

	nRet = 0;

	//LOG_WRITE(" start !!!!");

#if (__USE_TMAX__ > 0)
	nRet = fbinsert(m_pSendBuff, (FLDKEY) dwFieldKey, nIndex, pData, nDataLen);

	if(m_bVerify == TRUE)
	{
		int nLen = 0, nPos = 0;

		::ZeroMemory(m_szBuffer, sizeof(m_szBuffer));
		nRet = fbget_tu(m_pSendBuff, (FLDKEY) dwFieldKey, nIndex, m_szBuffer, &nLen);
		//LOG_WRITE("Verify#2 dwFieldKey %lu, (%s):(%s) ", dwFieldKey, pData, m_szBuffer);
		LOG_WRITE("Verify#2 %s =  (%s):(%s) ", fbget_fldname((FLDKEY)dwFieldKey), pData, m_szBuffer);
	}
#endif

	return nRet;
}

/**
 * @brief		TMaxFBUpdate
 * @details		필드 Update
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::TMaxFBUpdate(DWORD dwFieldKey, char *pData, int nIndex, int nDataLen)
{
	int nRet = 0;

#if (__USE_TMAX__ > 0)
	nRet = fbupdate(m_pSendBuff, (FLDKEY) dwFieldKey, nIndex, pData, nDataLen);
#endif

	return nRet;
}

/**
 * @brief		TMaxTpCall
 * @details		TMAX 접속
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::TMaxTransaction(int nTrKind)
{
	int nRet = 0;
	int nRecvLen = 0;

#if (__USE_TMAX__ > 0)

	switch(nTrKind)
	{
	case TMAX_TR_BEGIN :
		LOG_WRITE(" tx_begin() start !!!!");
		nRet = tx_begin();
		break;
	case TMAX_TR_COMMIT :
		LOG_WRITE(" tx_commit() start !!!!");
		nRet = tx_commit();
		break;
	case TMAX_TR_ROLLBACK :
		LOG_WRITE(" tx_rollback() start !!!!");
		nRet = tx_rollback();
		break;
	case TMAX_TR_TIMEOUT :
		LOG_WRITE(" tx_set_transaction_timeout() start !!!!");
		nRet = tx_set_transaction_timeout(45);
		break;
	default:
		return -1;
	}
#endif

	return nRet;
}

/**
 * @brief		TMaxTpCall
 * @details		TMAX 접속
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::TMaxTpCall(char *pService, int nTimeout)
{
	int nRet = 0;
	int nRecvLen = 0;

	LOG_WRITE("Service name(%s) !!!!", pService);

#if (__USE_TMAX__ > 0)
	/// 블로킹 타임아웃 시간 변경 (초단위 설정) : 거래는 50초
	tpset_timeout(50);

	LOG_WRITE("%s ","@@@@@@ tpcall() call !!!");
	//nRet = tpcall(pService, (char *)m_pSendBuff, (long) 0, (char **)&m_pRecvBuff, (long *) &nRecvLen, (long) TPNOTRAN);
	nRet = tpcall(pService, (char *)m_pSendBuff, (long) 0, (char **)&m_pRecvBuff, (long *) &nRecvLen, (long) m_nTPCallFlag);
	if(nRet < 0)
	{
		// 20190916. nhso modify : tpcall() retry 제거
		//nRet = tpcall(pService, (char *)m_pSendBuff, (long) 0, (char **)&m_pRecvBuff, (long *) &nRecvLen, (long) TPNOTRAN);
	}
#endif

	return nRet;
}

/**
 * @brief		TMaxFBGet
 * @details		필드 형식으로 데이타 전송
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::TMaxFBGet(DWORD dwFieldKey, char *pData)
{
	int nRet = 0;

#if (__USE_TMAX__ > 0)
	nRet = fbget(m_pRecvBuff, (FLDKEY) dwFieldKey, pData, 0);
#endif

	return nRet;
}

/**
 * @brief		TMaxFBGetTu
 * @details		필드 형식으로 데이타 전송
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::TMaxFBGetTu(DWORD dwFieldKey, char *pData, int nth)
{
	int nRet = 0;
	int nLength = 0;

#if (__USE_TMAX__ > 0)
	nRet = fbget_tu(m_pRecvBuff, (FLDKEY) dwFieldKey, nth, pData, &nLength);
#endif

	return nRet;
}

/**
 * @brief		TMaxFBGetF
 * @details		필드 형식으로 데이타 전송
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::TMaxFBGetF(DWORD dwFieldKey, char *pData)
{
	int nRet, nLen, nPos;

	nRet = nLen = nPos = 0;

#if (__USE_TMAX__ > 0)
	nRet = fbgetf(m_pRecvBuff, (FLDKEY) dwFieldKey, pData, &nLen, &nPos);
#endif

	return nRet;
}

/**
 * @brief		TMaxAlloc
 * @details		송수신 버퍼 Allocation
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::TMaxAlloc(void)
{
#if (__USE_TMAX__ > 0)
	LOG_WRITE("%s ","@@@@@@ tpalloc(SEND_FIELD) call !!!");
	m_pSendBuff = (FBUF *) tpalloc("FIELD", NULL, (1024 * 30));
	if(m_pSendBuff == NULL)
	{
		return -1;
	}

	// 수신 버퍼 할당 함수
	LOG_WRITE("%s ","@@@@@@ tpalloc(RECV_FIELD) call !!!");
	m_pRecvBuff = (FBUF *) tpalloc("FIELD", NULL, (1024 * 30));
	if(m_pRecvBuff == NULL)
	{
		return -2;
	}
#endif

	return 0;
}

/**
 * @brief		TMaxFree
 * @details		송수신 버퍼 Allocation
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
void CTmExpBusServer::TMaxFree(void)
{
	LOG_WRITE(" start !!!!");

#if (__USE_TMAX__ > 0)
	if(m_pSendBuff != NULL)
	{
		LOG_WRITE("%s ","@@@@@@ tpfree(SEND_FIELD) call !!!");
		tpfree((char *)m_pSendBuff);
		m_pSendBuff = NULL;
	}

	if(m_pRecvBuff != NULL)
	{
		LOG_WRITE("%s ","@@@@@@ tpfree(RECV_FIELD) call !!!");
		tpfree((char *)m_pRecvBuff);
		m_pRecvBuff = NULL;
	}
#endif
}

/**
 * @brief		Disconnect
 * @details		서버 접속
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
void CTmExpBusServer::TMaxDisconnect(void)
{
	LOG_WRITE(" start !!!!");
	LOG_WRITE(" ====================== \n");

#if (__USE_TMAX__ > 0)
	LOG_WRITE("%s ","@@@@@@ tpend() call !!!");
 	tpend();
#endif
}

/**
 * @brief		TMaxGetConvChar
 * @details		TMAX 데이타 가져와서, UTF8을 ANSI로 변환
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
void CTmExpBusServer::TMaxGetConvChar(DWORD dwFieledKey, char *retBuf)
{
	int nRet = 0;

	::ZeroMemory(m_szBuffer, sizeof(m_szBuffer));

	nRet = TMaxFBGetF(dwFieledKey,	m_szBuffer);
	Util_Utf8ToAnsi(m_szBuffer, retBuf);
}

/**
 * @brief		SVC_CM_ReadMsg
 * @details		메세지 정보 조회 - 1
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CM_ReadMsg(int nOperID, int nIndex, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	char*	pService = "CM_ReadMsg";
	pstk_tm_readmsg_t pSPacket;
	prtk_tm_readmsg_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" [%d] 메세지 정보 조회_start[%s] !!!!", nOperID, pService);

	pSPacket = (pstk_tm_readmsg_t) pData;
	pRPacket = (prtk_tm_readmsg_t) retBuf;

	if(nIndex == 0)
	{
		m_nRecNcnt1 = 0;
	}

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no);

		LOG_WRITE("%30s - (%s) ","LNG_CD"		,	pSPacket->lng_cd);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no);

		nRet = TMaxFBPut(LNG_CD,			pSPacket->lng_cd);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		HANDLE hFile;
		CString strFullName;

		/// msg_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			// 에러코드 조회
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", pRPacket->rsp_cd);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		/// list count
		nRet = TMaxFBGet(REC_NUM, PST_FIELD(pRPacket, rec_num));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			pRPacket->pList = new rtk_tm_readmsg_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_readmsg_list_t) * nCount);
		}

		// file save
		OperGetFileName(nOperID, strFullName);
		if(nIndex == 0)
		{
			hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
			m_nRecNcnt1 = nCount;
		}
		else
		{
			hFile = MyOpenFile(strFullName, TRUE);
			m_nRecNcnt1 += nCount;
		}
		
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));

		::ZeroMemory(pRPacket->rec_num, sizeof(pRPacket->rec_num));
		::CopyMemory(pRPacket->rec_num, &m_nRecNcnt1, 4);
		MyWriteFile(hFile, pRPacket->rec_num, sizeof(pRPacket->rec_num));

		MySetFilePointer(hFile, 0, FILE_END);
		
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_readmsg_list_t pList;

			pList = &pRPacket->pList[i];

			sprintf(pList->lng_cd, "%s", pSPacket->lng_cd);

			nRet = TMaxFBGetF(MSG_CD 		,	PST_FIELD(pList, msg_cd 	));
			
			switch(nIndex)
			{
			case LANG_DVS_KO:
				TMaxGetConvChar(MSG_DTL_CTT,	PST_FIELD(pList, msg_dtl_ctt));
				break;
			case LANG_DVS_ENG:
				nRet = TMaxFBGetF(MSG_DTL_CTT,	PST_FIELD(pList, msg_dtl_ctt));
				break;
			case LANG_DVS_CHI:
			case LANG_DVS_JPN:
				TMaxGetConvChar(MSG_DTL_CTT,	PST_FIELD(pList, msg_dtl_ctt));
				break;
			}

			MyWriteFile(hFile, pList, sizeof(rtk_tm_readmsg_list_t));
		}
		MyCloseFile(hFile);
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_readmsg_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "lng_cd"		,	PST_FIELD(pList, lng_cd 		));
			LOG_WRITE("%30s - (%s) ", "msg_cd"		,	PST_FIELD(pList, msg_cd			));
			LOG_WRITE("%30s - (%s) ", "msg_dtl_ctt"	,	PST_FIELD(pList, msg_dtl_ctt 	));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_CM_ReadNtc
 * @details		승차권 고유번호, 공지사항 조회 - 2
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CM_ReadNtc(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "CM_ReadNtc";
	pstk_tm_readntc_t pSPacket;
	prtk_tm_readntc_t pRPacket;

	nRet = nTimeout = nCount = 0;

	LOG_OPEN();
	LOG_WRITE(" [%d] 승차권 고유번호, 공지사항 조회_start[%s] !!!!", nOperID, pService);

	pSPacket = (pstk_tm_readntc_t) pData;
	pRPacket = (prtk_tm_readntc_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		/// 서버 일시
		nRet = TMaxFBGet(SERV_DT	, PST_FIELD(pRPacket, serv_dt	));
		nRet = TMaxFBGet(SERV_TIME	, PST_FIELD(pRPacket, serv_time	));
		/// 고유번호
		nRet = TMaxFBGet(INHR_NO	, PST_FIELD(pRPacket, inhr_no	));

		/// set memory
		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.inhr_no, pRPacket->inhr_no, sizeof(pRPacket->inhr_no));

		{
			SYSTEMTIME st;

			GetLocalTime(&st);

			st.wYear   = (WORD) Util_Ascii2Long(&pRPacket->serv_dt[0], 4);
			st.wMonth  = (WORD) Util_Ascii2Long(&pRPacket->serv_dt[4], 2);
			st.wDay    = (WORD) Util_Ascii2Long(&pRPacket->serv_dt[6], 2);

			st.wHour   = (WORD) Util_Ascii2Long(&pRPacket->serv_time[0], 2);
			st.wMinute = (WORD) Util_Ascii2Long(&pRPacket->serv_time[2], 2);
			st.wSecond = (WORD) Util_Ascii2Long(&pRPacket->serv_time[4], 2);

			nRet = Util_CheckDateTime(MY_SYS_DATETIME, (BYTE *)&st);
			if(nRet >= 0)
			{
				nRet = Util_SetLocalTime((char *)&st);
				LOG_WRITE("nRet(%d), 시간동기화 : %04d/%02d/%02d %02d:%02d:%02d ", nRet, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			}
		}

		/// 공지사항 정보 갯수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num	));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
		if(nCount < 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		if(nCount > 0)
		{
			pRPacket->pList = new rtk_tm_readntc_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_readntc_list_t) * nCount);
		}

		{
			HANDLE hFile;
			CString strFullName;

			// file save
			OperGetFileName(nOperID, strFullName);
			hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
			m_nRecNcnt1 = nCount;
			if(hFile == INVALID_HANDLE_VALUE)
			{
				nRet = -100;
				LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
				goto fail_proc;
			}

			MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
			MyWriteFile(hFile, pRPacket->serv_dt, sizeof(pRPacket->serv_dt));
			MyWriteFile(hFile, pRPacket->serv_time, sizeof(pRPacket->serv_time));
			MyWriteFile(hFile, pRPacket->inhr_no, sizeof(pRPacket->inhr_no));

			::ZeroMemory(pRPacket->rec_num, sizeof(pRPacket->rec_num));
			::CopyMemory(pRPacket->rec_num, &m_nRecNcnt1, 4);
			MyWriteFile(hFile, pRPacket->rec_num, sizeof(pRPacket->rec_num));

			MySetFilePointer(hFile, 0, FILE_END);

			for(int i = 0; i < nCount; i++)
			{
				prtk_tm_readntc_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TRML_NO			,	PST_FIELD(pList, trml_no		 ));
				nRet = TMaxFBGetF(NTC_MTTR_SNO		,	PST_FIELD(pList, ntc_mttr_sno	 ));
				nRet = TMaxFBGetF(NTC_MTTR_TTL_NM	,	PST_FIELD(pList, ntc_mttr_ttl_nm ));
				nRet = TMaxFBGetF(WRTN_USER_NM		,	PST_FIELD(pList, wrtn_user_nm	 ));
				nRet = TMaxFBGetF(TRML_NTC_CTT		,	PST_FIELD(pList, trml_ntc_ctt	 ));
				nRet = TMaxFBGetF(STT_DTM			,	PST_FIELD(pList, stt_dtm		 ));
				nRet = TMaxFBGetF(END_DTM			,	PST_FIELD(pList, end_dtm		 ));
			
				MyWriteFile(hFile, pList, sizeof(rtk_tm_readntc_list_t));
			}
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		LOG_WRITE("%30s - (%s) ", "SERV_DT"		, pRPacket->serv_dt);
		LOG_WRITE("%30s - (%s) ", "SERV_TIME"	, pRPacket->serv_time);
		LOG_WRITE("%30s - (%s) ", "INHR_NO"		, pRPacket->inhr_no);

		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		for(int i = 0; i < nCount; i++)
		{
			prtk_tm_readntc_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TRML_NO"			,	PST_FIELD(pList, trml_no		 ));
			LOG_WRITE("%30s - (%s) ", "NTC_MTTR_SNO"	,	PST_FIELD(pList, ntc_mttr_sno	 ));
			LOG_WRITE("%30s - (%s) ", "NTC_MTTR_TTL_NM" ,	PST_FIELD(pList, ntc_mttr_ttl_nm ));
			LOG_WRITE("%30s - (%s) ", "WRTN_USER_NM"	,	PST_FIELD(pList, wrtn_user_nm	 ));
			LOG_WRITE("%30s - (%s) ", "TRML_NTC_CTT"	,	PST_FIELD(pList, trml_ntc_ctt	 ));
			LOG_WRITE("%30s - (%s) ", "STT_DTM"			,	PST_FIELD(pList, stt_dtm		 ));
			LOG_WRITE("%30s - (%s) ", "END_DTM"			,	PST_FIELD(pList, end_dtm		 ));
		}
	}

	///> data proc
	{
		if(nCount == 0)
		{
			nRet = 1;
		}
		else
		{
			nRet = nCount;
		}
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_AuthCmpt	- 3
 * @details		로그인 시, 유저 인증
 * @param		int nOperID			operation ID
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_AuthCmpt(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "TK_AuthCmpt";
	pstk_tm_authcmpt_t pSPacket;
	prtk_tm_authcmpt_t pRPacket;

	nRet = nTimeout = nCount = 0;

	LOG_OPEN();
	LOG_WRITE("[%d] 로그인 시, 유저 인증_start[%s] !!!!", nOperID, pService);

	pSPacket = (pstk_tm_authcmpt_t) pData;
	pRPacket = (prtk_tm_authcmpt_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no);
	
		LOG_WRITE("%30s - (%s) ","CMPT_ID"		,	pSPacket->cmpt_id	);
		LOG_WRITE("%30s - (%s) ","USER_ID"		,	pSPacket->user_id	);
		LOG_WRITE("%30s - (%s) ","USER_PWD"		,	pSPacket->user_pwd	);
		LOG_WRITE("%30s - (%s) ","INHR_NO"		,	pSPacket->inhr_no	);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no);

		nRet = TMaxFBPut(CMPT_ID		,	pSPacket->cmpt_id	);
		nRet = TMaxFBPut(USER_ID		,	pSPacket->user_id	);
		nRet = TMaxFBPut(USER_PWD		,	pSPacket->user_pwd	);
		nRet = TMaxFBPut(INHR_NO		,	pSPacket->inhr_no	);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		/// 터미널 약칭
		TMaxGetConvChar(TRML_ABRV_NM, pRPacket->trml_abrv_nm);
		//nRet = TMaxFBGetF(TRML_ABRV_NM		,	PST_FIELD(pRPacket, trml_abrv_nm	 ));
		/// 터미널 명
		TMaxGetConvChar(TRML_NM, pRPacket->trml_nm);
		//nRet = TMaxFBGetF(TRML_NM			,	PST_FIELD(pRPacket, trml_nm			 ));
		nRet = TMaxFBGetF(TRML_ENG_ABRV_NM	,	PST_FIELD(pRPacket, trml_eng_abrv_nm ));
		nRet = TMaxFBGetF(TEL_NO			,	PST_FIELD(pRPacket, tel_no			 ));
		nRet = TMaxFBGetF(USER_NO			,	PST_FIELD(pRPacket, user_no			 ));
		nRet = TMaxFBGetF(USER_NM			,	PST_FIELD(pRPacket, user_nm			 ));
		nRet = TMaxFBGetF(INHR_NO			,	PST_FIELD(pRPacket, inhr_no			 ));
		nRet = TMaxFBGetF(TISSU_NUM			,	PST_FIELD(pRPacket, tissu_num		 ));
		nRet = TMaxFBGetF(TISSU_AMT			,	PST_FIELD(pRPacket, tissu_amt		 ));
		nRet = TMaxFBGetF(CANC_NUM			,	PST_FIELD(pRPacket, canc_num		 ));
		nRet = TMaxFBGetF(CANC_AMT			,	PST_FIELD(pRPacket, canc_amt		 ));
		nRet = TMaxFBGetF(RY_NUM			,	PST_FIELD(pRPacket, ry_num			 ));
		nRet = TMaxFBGetF(RY_AMT			,	PST_FIELD(pRPacket, ry_amt			 ));
		nRet = TMaxFBGetF(DC_RC_AMT			,	PST_FIELD(pRPacket, dc_rc_amt		 ));
		nRet = TMaxFBGetF(TKT_NUM			,	PST_FIELD(pRPacket, tkt_num			 ));
		nRet = TMaxFBGetF(TKT_AMT			,	PST_FIELD(pRPacket, tkt_amt			 ));
		nRet = TMaxFBGetF(CASH_NUM			,	PST_FIELD(pRPacket, cash_num		 ));
		nRet = TMaxFBGetF(CASH_AMT			,	PST_FIELD(pRPacket, cash_amt		 ));
		nRet = TMaxFBGetF(TISSU_PSB_YN		,	PST_FIELD(pRPacket, tissu_psb_yn	 ));
		nRet = TMaxFBGetF(TAK_STT_DT		,	PST_FIELD(pRPacket, tak_stt_dt		 ));
		nRet = TMaxFBGetF(STT_TIME			,	PST_FIELD(pRPacket, stt_time		 ));

		/// set user_no config_mem
		if(0)
		{
			CConfigTkMem *pclsCfgMem;

			pclsCfgMem = CConfigTkMem::GetInstance();
			::CopyMemory(pclsCfgMem->m_tTmExp.req_user_no, pRPacket->user_no, sizeof(pclsCfgMem->m_tTmExp.req_user_no));
		}
		else
		{
			CConfigTkMem::GetInstance()->SetTmExpUserNo(pRPacket->user_no);
		}

		/// file write
		{
			HANDLE hFile;
			CString strFullName;

			// file save
			OperGetFileName(nOperID, strFullName);
			hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
			m_nRecNcnt1 = nCount;
			if(hFile == INVALID_HANDLE_VALUE)
			{
				nRet = -100;
				LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
				goto fail_proc;
			}

			MyWriteFile(hFile, pRPacket, sizeof(rtk_tm_authcmpt_t));
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		LOG_WRITE("%30s - (%s) ", "TRML_ABRV_NM"	,	PST_FIELD(pRPacket, trml_abrv_nm	  ));
		LOG_WRITE("%30s - (%s) ", "TRML_NM"			,	PST_FIELD(pRPacket, trml_nm			  ));
		LOG_WRITE("%30s - (%s) ", "TRML_ENG_ABRV_NM",	PST_FIELD(pRPacket, trml_eng_abrv_nm  ));
		LOG_WRITE("%30s - (%s) ", "TEL_NO"			,	PST_FIELD(pRPacket, tel_no			  ));
		LOG_WRITE("%30s - (%s) ", "USER_NO"			,	PST_FIELD(pRPacket, user_no			  ));
		LOG_WRITE("%30s - (%s) ", "USER_NM"			,	PST_FIELD(pRPacket, user_nm			  ));
		LOG_WRITE("%30s - (%s) ", "INHR_NO"			,	PST_FIELD(pRPacket, inhr_no			  ));
		
		LOG_WRITE("%30s - (%d) ", "TISSU_NUM"		,	*(int *)pRPacket->tissu_num				);
		LOG_WRITE("%30s - (%d) ", "TISSU_AMT"		,	*(int *)pRPacket->tissu_amt				);
		LOG_WRITE("%30s - (%d) ", "CANC_NUM"		,	*(int *)pRPacket->canc_num				);
		LOG_WRITE("%30s - (%d) ", "CANC_AMT"		,	*(int *)pRPacket->canc_amt				);
		LOG_WRITE("%30s - (%d) ", "RY_NUM"			,	*(int *)pRPacket->ry_num				);
		LOG_WRITE("%30s - (%d) ", "RY_AMT"			,	*(int *)pRPacket->ry_amt				);
		LOG_WRITE("%30s - (%d) ", "DC_RC_AMT"		,	*(int *)pRPacket->dc_rc_amt				);
		LOG_WRITE("%30s - (%d) ", "TKT_NUM"			,	*(int *)pRPacket->tkt_num				);
		LOG_WRITE("%30s - (%d) ", "TKT_AMT"			,	*(int *)pRPacket->tkt_amt				);
		LOG_WRITE("%30s - (%d) ", "CASH_NUM"		,	*(int *)pRPacket->cash_num				);
		LOG_WRITE("%30s - (%d) ", "CASH_AMT"		,	*(int *)pRPacket->cash_amt				);
		
		LOG_WRITE("%30s - (%s) ", "TISSU_PSB_YN"	,	PST_FIELD(pRPacket, tissu_psb_yn	  ));
		LOG_WRITE("%30s - (%s) ", "TAK_STT_DT"		,	PST_FIELD(pRPacket, tak_stt_dt		  ));
		LOG_WRITE("%30s - (%s) ", "STT_TIME"		,	PST_FIELD(pRPacket, stt_time		  ));
	}

	///> data proc
	{
		nRet = 1;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_CM_ReadCmnCd - 4
 * @details		공통코드 조회
 * @param		int nOperID			operation ID
 * @param		int nIndex			Language index
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CM_ReadCmnCd(int nOperID, int nIndex, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "CM_ReadCmnCd";
	pstk_tm_readcmncd_t pSPacket;
	prtk_tm_readcmncd_t pRPacket;

	nRet = nTimeout = nCount = 0;

	LOG_OPEN();
	LOG_WRITE("[%d] 공통코드 조회_start[%s] !!!!", nOperID, pService);

	pSPacket = (pstk_tm_readcmncd_t) pData;
	pRPacket = (prtk_tm_readcmncd_t) retBuf;

	if(nIndex == 0)
	{
		m_nRecNcnt1 = 0;
	}

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no);
	
		LOG_WRITE("%30s - (%s) ","LNG_CD"		,	pSPacket->lng_cd	);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no);

		nRet = TMaxFBPut(LNG_CD			,	pSPacket->lng_cd	);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		/// 정보 갯수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num	));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			pRPacket->pList = new rtk_tm_readcmncd_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_readcmncd_list_t) * nCount);
		}

		{
			HANDLE hFile;
			CString strFullName;
			HANDLE hFileBusCls;
			CString strFullNameBusCls;


			// file save
			OperGetFileName(nOperID, strFullName);
			OperGetFileName(OPER_FILE_ID_EZ_BUS_CLS, strFullNameBusCls);

			if(nIndex == 0)
			{
				hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
				hFileBusCls = MyOpenFile2(strFullNameBusCls, CREATE_ALWAYS);

				m_nRecNcnt1 = nCount;
			}
			else
			{
				hFile = MyOpenFile(strFullName, TRUE);
				hFileBusCls = MyOpenFile(strFullNameBusCls, TRUE);
				m_nRecNcnt1 += nCount;
			}

			if(hFile == INVALID_HANDLE_VALUE)
			{
				nRet = -100;
				LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
				goto fail_proc;
			}

			MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));

			::ZeroMemory(pRPacket->rec_num, sizeof(pRPacket->rec_num));
			::CopyMemory(pRPacket->rec_num, &m_nRecNcnt1, 4);
			MyWriteFile(hFile, pRPacket->rec_num, sizeof(pRPacket->rec_num));

			MySetFilePointer(hFile, 0, FILE_END);
			MySetFilePointer(hFileBusCls, 0, FILE_END);

			for(int i = 0; i < nCount; i++)
			{
				prtk_tm_readcmncd_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(LNG_CD			,	PST_FIELD(pList, lng_cd			 ));
				nRet = TMaxFBGetF(CMN_CD			,	PST_FIELD(pList, cmn_cd			 ));
				nRet = TMaxFBGetF(CMN_CD_VAL		,	PST_FIELD(pList, cmn_cd_val		 ));
				nRet = TMaxFBGetF(CD_VAL_MARK_SEQ	,	PST_FIELD(pList, cd_val_mark_seq ));
				
				switch(nIndex)
				{
				case LANG_DVS_KO:
					TMaxGetConvChar(CD_VAL_NM, PST_FIELD(pList, cd_val_nm		 ));
					break;
				case LANG_DVS_ENG:
					nRet = TMaxFBGetF(CD_VAL_NM			,	PST_FIELD(pList, cd_val_nm		 ));
					break;
				case LANG_DVS_CHI:
				case LANG_DVS_JPN:
				default:
					TMaxGetConvChar(CD_VAL_NM			,	PST_FIELD(pList, cd_val_nm		 ));
					break;
				}
				
				nRet = TMaxFBGetF(BSC_RFRN_VAL		,	PST_FIELD(pList, bsc_rfrn_val	 ));
				nRet = TMaxFBGetF(ADTN_RFRN_VAL		,	PST_FIELD(pList, adtn_rfrn_val	 ));

				if( memcmp("C024", pList->cmn_cd, 4) == 0 )
				{
					/// 티머니고속 - 버스등급 데이타 발췌..
					MyWriteFile(hFileBusCls, pList, sizeof(rtk_tm_readcmncd_list_t));
				}

				MyWriteFile(hFile, pList, sizeof(rtk_tm_readcmncd_list_t));
			}
			MyCloseFile(hFile);
			MyCloseFile(hFileBusCls);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		for(int i = 0; i < nCount; i++)
		{
			prtk_tm_readcmncd_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "LNG_CD"			,	PST_FIELD(pList, lng_cd			 ));
			LOG_WRITE("%30s - (%s) ", "CMN_CD"			,	PST_FIELD(pList, cmn_cd			 ));
			LOG_WRITE("%30s - (%s) ", "CMN_CD_VAL"		,	PST_FIELD(pList, cmn_cd_val		 ));
			LOG_WRITE("%30s - (%d) ", "CD_VAL_MARK_SEQ"	,	*(int *)pList->cd_val_mark_seq	 );
			LOG_WRITE("%30s - (%s) ", "CD_VAL_NM"		,	PST_FIELD(pList, cd_val_nm		 ));
			LOG_WRITE("%30s - (%s) ", "BSC_RFRN_VAL"	,	PST_FIELD(pList, bsc_rfrn_val	 ));
			LOG_WRITE("%30s - (%s) ", "ADTN_RFRN_VAL"	,	PST_FIELD(pList, adtn_rfrn_val	 ));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_ReadTckPrtg
 * @details		승차권 인쇄정보 조회 - 5
 * @param		int nOperID			operation ID
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_ReadTckPrtg(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "TK_ReadTckPrtg";
	pstk_tm_readtckprtg_t pSPacket;
	prtk_tm_readtckprtg_t pRPacket;

	nRet = nTimeout = nCount = 0;

	LOG_OPEN();
	LOG_WRITE("[%d] 승차권 인쇄정보 조회_start[%s] !!!!", nOperID, pService);

	pSPacket = (pstk_tm_readtckprtg_t) pData;
	pRPacket = (prtk_tm_readtckprtg_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);
	
		LOG_WRITE("%30s - (%s) ","TRML_NO"		,	pSPacket->trml_no			);
		LOG_WRITE("%30s - (%s) ","PAPR_TCK_DVS_CD",	pSPacket->papr_tck_dvs_cd	);
		LOG_WRITE("%30s - (%s) ","PTR_KND_CD"	,	pSPacket->ptr_knd_cd		);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(TRML_NO		,	pSPacket->trml_no				);
		nRet = TMaxFBPut(PAPR_TCK_DVS_CD,	pSPacket->papr_tck_dvs_cd		);
		nRet = TMaxFBPut(PTR_KND_CD		,	pSPacket->ptr_knd_cd			);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		/// 정보 갯수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num	));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			pRPacket->pList = new rtk_tm_readtckprtg_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_readtckprtg_list_t) * nCount);
		}

		{
			HANDLE hFile;
			CString strFullName;

			// file save
			OperGetFileName(nOperID, strFullName);
			hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
			m_nRecNcnt1 = nCount;
			if(hFile == INVALID_HANDLE_VALUE)
			{
				nRet = -100;
				LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
				goto fail_proc;
			}

			MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));

			::ZeroMemory(pRPacket->rec_num, sizeof(pRPacket->rec_num));
			::CopyMemory(pRPacket->rec_num, &m_nRecNcnt1, 4);
			MyWriteFile(hFile, pRPacket->rec_num, sizeof(pRPacket->rec_num));

			MySetFilePointer(hFile, 0, FILE_END);

			for(int i = 0; i < nCount; i++)
			{
				prtk_tm_readtckprtg_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TRML_NO			,	PST_FIELD(pList, trml_no			));
				nRet = TMaxFBGetF(PAPR_TCK_DVS_CD	,	PST_FIELD(pList, papr_tck_dvs_cd	));
				nRet = TMaxFBGetF(PTR_KND_CD		,	PST_FIELD(pList, ptr_knd_cd			));
				nRet = TMaxFBGetF(SORT_SEQ			,	PST_FIELD(pList, sort_seq			));
				nRet = TMaxFBGetF(PTRG_USG_CD		,	PST_FIELD(pList, ptrg_usg_cd		));
				nRet = TMaxFBGetF(PTRG_ATC_NM		,	PST_FIELD(pList, ptrg_atc_nm		));
				nRet = TMaxFBGetF(X_CRDN_VAL		,	PST_FIELD(pList, x_crdn_val			));
				nRet = TMaxFBGetF(Y_CRDN_VAL		,	PST_FIELD(pList, y_crdn_val			));
				nRet = TMaxFBGetF(MGNF_VAL			,	PST_FIELD(pList, mgnf_val			));
				nRet = TMaxFBGetF(USE_YN			,	PST_FIELD(pList, use_yn				));

				MyWriteFile(hFile, pList, sizeof(rtk_tm_readtckprtg_list_t));
			}
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		for(int i = 0; i < nCount; i++)
		{
			prtk_tm_readtckprtg_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "trml_no"			,	PST_FIELD(pList, trml_no		 ));
			LOG_WRITE("%30s - (%s) ", "papr_tck_dvs_cd"	,	PST_FIELD(pList, papr_tck_dvs_cd ));
			LOG_WRITE("%30s - (%s) ", "ptr_knd_cd"		,	PST_FIELD(pList, ptr_knd_cd		 ));
			LOG_WRITE("%30s - (Num=%d) ", "sort_seq"	,	*(int *)pList->sort_seq			  );
			LOG_WRITE("%30s - (%s) ", "ptrg_usg_cd"		,	PST_FIELD(pList, ptrg_usg_cd	 ));
			LOG_WRITE("%30s - (%s) ", "ptrg_atc_nm"		,	PST_FIELD(pList, ptrg_atc_nm	 ));
			LOG_WRITE("%30s - (%s) ", "x_crdn_val"		,	PST_FIELD(pList, x_crdn_val		 ));
			LOG_WRITE("%30s - (%s) ", "y_crdn_val"		,	PST_FIELD(pList, y_crdn_val		 ));
			LOG_WRITE("%30s - (%s) ", "mgnf_val"		,	PST_FIELD(pList, mgnf_val		 ));
			LOG_WRITE("%30s - (%s) ", "use_yn"			,	PST_FIELD(pList, use_yn			 ));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_MG_ReadTrmlDrtn
 * @details		방면 정보 조회 - 6
 * @param		int nOperID			operation ID
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_MG_ReadTrmlDrtn(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "MG_ReadTrmlDrtn";
	pstk_tm_readtrmldrtn_t pSPacket;
	prtk_tm_readtrmldrtn_t pRPacket;

	nRet = nTimeout = nCount = 0;

	LOG_OPEN();
	LOG_WRITE("[%d] 방면 정보 조회_start[%s] !!!!", nOperID, pService);

	pSPacket = (pstk_tm_readtrmldrtn_t) pData;
	pRPacket = (prtk_tm_readtrmldrtn_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);
	
		LOG_WRITE("%30s - (%s) ","TRML_NO"		,	pSPacket->trml_no			);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(TRML_NO		,	pSPacket->trml_no				);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		/// 정보 갯수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num	));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			pRPacket->pList = new rtk_tm_readtrmldrtn_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_readtrmldrtn_list_t) * nCount);
		}

		{
			HANDLE hFile;
			CString strFullName;

			// file save
			OperGetFileName(nOperID, strFullName);
			hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
			m_nRecNcnt1 = nCount;
			if(hFile == INVALID_HANDLE_VALUE)
			{
				nRet = -100;
				LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
				goto fail_proc;
			}

			MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));

			::ZeroMemory(pRPacket->rec_num, sizeof(pRPacket->rec_num));
			::CopyMemory(pRPacket->rec_num, &m_nRecNcnt1, 4);
			MyWriteFile(hFile, pRPacket->rec_num, sizeof(pRPacket->rec_num));

			MySetFilePointer(hFile, 0, FILE_END);

			for(int i = 0; i < nCount; i++)
			{
				prtk_tm_readtrmldrtn_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TRML_NO	,	PST_FIELD(pList, trml_no	));
				nRet = TMaxFBGetF(DRTN_CD	,	PST_FIELD(pList, drtn_cd	));
				
				TMaxGetConvChar(DRTN_NM	,	PST_FIELD(pList, drtn_nm	));
				//nRet = TMaxFBGetF(DRTN_NM	,	PST_FIELD(pList, drtn_nm	));

				MyWriteFile(hFile, pList, sizeof(rtk_tm_readtrmldrtn_list_t));
			}
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		for(int i = 0; i < nCount; i++)
		{
			prtk_tm_readtrmldrtn_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TRML_NO"			,	PST_FIELD(pList, trml_no		 ));
			LOG_WRITE("%30s - (%s) ", "DRTN_CD"			,	PST_FIELD(pList, drtn_cd		 ));
			LOG_WRITE("%30s - (%s) ", "DRTN_NM"			,	PST_FIELD(pList, drtn_nm		 ));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_CM_ReadRyrt
 * @details		환불율 정보 조회 - 7
 * @param		int nOperID			operation ID
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CM_ReadRyrt(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "CM_ReadRyrt";
	pstk_tm_read_ryrt_t pSPacket;
	prtk_tm_read_ryrt_t pRPacket;

	nRet = nTimeout = nCount = 0;

	LOG_OPEN();
	LOG_WRITE("[%d] 환불율 정보 조회_start[%s] !!!!", nOperID, pService);

	pSPacket = (pstk_tm_read_ryrt_t) pData;
	pRPacket = (prtk_tm_read_ryrt_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);
	
		LOG_WRITE("%30s - (%s) ","USE_YN"		,	pSPacket->use_yn			);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(USE_YN			,	pSPacket->use_yn				);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		/// 정보 갯수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num	));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			pRPacket->pList = new rtk_tm_read_ryrt_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_read_ryrt_list_t) * nCount);
		}

		{
			HANDLE hFile;
			CString strFullName;

			// file save
			OperGetFileName(nOperID, strFullName);
			hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
			m_nRecNcnt1 = nCount;
			if(hFile == INVALID_HANDLE_VALUE)
			{
				nRet = -100;
				LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
				goto fail_proc;
			}

			MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));

			::ZeroMemory(pRPacket->rec_num, sizeof(pRPacket->rec_num));
			::CopyMemory(pRPacket->rec_num, &m_nRecNcnt1, 4);
			MyWriteFile(hFile, pRPacket->rec_num, sizeof(pRPacket->rec_num));

			MySetFilePointer(hFile, 0, FILE_END);

			for(int i = 0; i < nCount; i++)
			{
				prtk_tm_read_ryrt_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(RY_KND_CD			,	PST_FIELD(pList, ry_knd_cd			));
				nRet = TMaxFBGetF(CD_VAL_NM			,	PST_FIELD(pList, cd_val_nm			));
				nRet = TMaxFBGetF(TRML_RY_KND_CD	,	PST_FIELD(pList, trml_ry_knd_cd		));
				TMaxGetConvChar(SCRN_PRIN_NM		,	PST_FIELD(pList, scrn_prin_nm));
				//nRet = TMaxFBGetF(SCRN_PRIN_NM	,	PST_FIELD(pList, scrn_prin_nm		));
				nRet = TMaxFBGetF(USE_YN			,	PST_FIELD(pList, use_yn				));

				MyWriteFile(hFile, pList, sizeof(rtk_tm_read_ryrt_list_t));
			}
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		for(int i = 0; i < nCount; i++)
		{
			prtk_tm_read_ryrt_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "RY_KND_CD"		,	PST_FIELD(pList, ry_knd_cd		 ));
			LOG_WRITE("%30s - (%s) ", "CD_VAL_NM"		,	PST_FIELD(pList, cd_val_nm		 ));
			LOG_WRITE("%30s - (%s) ", "TRML_RY_KND_CD"	,	PST_FIELD(pList, trml_ry_knd_cd	 ));
			LOG_WRITE("%30s - (%s) ", "SCRN_PRIN_NM"	,	PST_FIELD(pList, scrn_prin_nm	 ));
			LOG_WRITE("%30s - (%s) ", "USE_YN"			,	PST_FIELD(pList, use_yn			 ));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_ReadOwnrTrml
 * @details		자기터미널 정보 조회 - 7
 * @param		int nOperID			operation ID
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_ReadOwnrTrml(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "TK_ReadOwnrTrml";
	pstk_tm_readownrtrml_t pSPacket;
	prtk_tm_readownrtrml_t pRPacket;

	nRet = nTimeout = nCount = 0;

	LOG_OPEN();
	LOG_WRITE("[%d] 자기터미널 정보 조회_start[%s] !!!!", nOperID, pService);

	pSPacket = (pstk_tm_readownrtrml_t) pData;
	pRPacket = (prtk_tm_readownrtrml_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		TMaxGetConvChar(TRML_ABRV_NM	,	PST_FIELD(pRPacket, trml_abrv_nm	));
		TMaxGetConvChar(TRML_NM			,	PST_FIELD(pRPacket, trml_nm			));
		nRet = TMaxFBGetF(TRML_ENG_NM	,	PST_FIELD(pRPacket, trml_eng_nm		));
		nRet = TMaxFBGetF(CSRC_RGT_NO	,	PST_FIELD(pRPacket, csrc_rgt_no		));
		nRet = TMaxFBGetF(TEL_NO		,	PST_FIELD(pRPacket, tel_no			));
		nRet = TMaxFBGetF(TRTR_TRML_YN	,	PST_FIELD(pRPacket, trtr_trml_yn	));

		{
			HANDLE hFile;
			CString strFullName;

			// file save
			OperGetFileName(nOperID, strFullName);
			hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
			m_nRecNcnt1 = nCount;
			if(hFile == INVALID_HANDLE_VALUE)
			{
				nRet = -100;
				LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
				goto fail_proc;
			}

			MyWriteFile(hFile, pRPacket, sizeof(rtk_tm_authcmpt_t));
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		LOG_WRITE("%30s - (%s) ", "trml_abrv_nm"	,	PST_FIELD(pRPacket, trml_abrv_nm	));
		LOG_WRITE("%30s - (%s) ", "trml_nm"			,	PST_FIELD(pRPacket, trml_nm			));
		LOG_WRITE("%30s - (%s) ", "trml_eng_nm"		,	PST_FIELD(pRPacket, trml_eng_nm		));
		LOG_WRITE("%30s - (%s) ", "csrc_rgt_no"		,	PST_FIELD(pRPacket, csrc_rgt_no		));
		LOG_WRITE("%30s - (%s) ", "tel_no"			,	PST_FIELD(pRPacket, tel_no			));
		LOG_WRITE("%30s - (%s) ", "trtr_trml_yn"	,	PST_FIELD(pRPacket, trtr_trml_yn	));
	}

	///> data proc
	{
		nRet = 1;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_CM_ReadTrml 
 * @details		터미널 조회(전국터미널)
 * @param		int nOperID			operation ID
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CM_ReadTrml(int nOperID, int nIndex, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "CM_ReadTrml";
	pstk_tm_readtrml_t pSPacket;
	prtk_tm_readtrml_t pRPacket;

	nRet = nTimeout = nCount = 0;

	LOG_OPEN();
	LOG_WRITE("[%d] 터미널 조회(전국터미널)_start[%s] !!!!", nOperID, pService);

	pSPacket = (pstk_tm_readtrml_t) pData;
	pRPacket = (prtk_tm_readtrml_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);
	
		LOG_WRITE("%30s - (%s) ","LNG_CD"		,	pSPacket->lng_cd		);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(LNG_CD			,	pSPacket->lng_cd				);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		/// 정보 갯수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num	));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			pRPacket->pList = new rtk_tm_readtrml_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_readtrml_list_t) * nCount);
		}

		{
			HANDLE hFile;
			CString strFullName;

			// file save
			OperGetFileName(nOperID, strFullName);
			if(nIndex == 0)
			{
				hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
				m_nRecNcnt1 = nCount;
			}
			else
			{
				hFile = MyOpenFile(strFullName, TRUE);
				m_nRecNcnt1 += nCount;
			}
			if(hFile == INVALID_HANDLE_VALUE)
			{
				nRet = -100;
				LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
				goto fail_proc;
			}

			MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));

			::ZeroMemory(pRPacket->rec_num, sizeof(pRPacket->rec_num));
			::CopyMemory(pRPacket->rec_num, &m_nRecNcnt1, 4);
			MyWriteFile(hFile, pRPacket->rec_num, sizeof(pRPacket->rec_num));
			MySetFilePointer(hFile, 0, FILE_END);

			for(int i = 0; i < nCount; i++)
			{
				prtk_tm_readtrml_list_t pList;

				pList = &pRPacket->pList[i];

				sprintf(pList->lng_cd, "%s", pSPacket->lng_cd);

				nRet = TMaxFBGetF(TRML_NO				,	PST_FIELD(pList, trml_no					));

				switch(nIndex)
				{
				case LANG_DVS_KO:
					TMaxGetConvChar(TRML_NM			,	PST_FIELD(pList, trml_nm		));
					TMaxGetConvChar(TRML_ABRV_NM	,	PST_FIELD(pList, trml_abrv_nm	));
					TMaxGetConvChar(TRML_DTL_ADDR	,	PST_FIELD(pList, trml_dtl_addr	));
					break;
				case LANG_DVS_ENG:
				case LANG_DVS_CHI:
				case LANG_DVS_JPN:
					nRet = TMaxFBGetF(TRML_NM		,	PST_FIELD(pList, trml_nm		));
					nRet = TMaxFBGetF(TRML_ABRV_NM	,	PST_FIELD(pList, trml_abrv_nm	));
					nRet = TMaxFBGetF(TRML_DTL_ADDR	,	PST_FIELD(pList, trml_dtl_addr	));
					break;
				}


				TMaxGetConvChar(TRML_ENG_NM				,	PST_FIELD(pList, trml_eng_nm				));
				TMaxGetConvChar(TRML_ENG_ABRV_NM		,	PST_FIELD(pList, trml_eng_abrv_nm			));
				nRet = TMaxFBGetF(TRTR_TRML_YN			,	PST_FIELD(pList, trtr_trml_yn				));
				nRet = TMaxFBGetF(SYS_DVS_CD			,	PST_FIELD(pList, sys_dvs_cd					));
				nRet = TMaxFBGetF(TEL_NO				,	PST_FIELD(pList, tel_no						));
				nRet = TMaxFBGetF(STLM_TRML_NO			,	PST_FIELD(pList, stlm_trml_no				));

				MyWriteFile(hFile, pList, sizeof(rtk_tm_readtrml_list_t));
			}
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		for(int i = 0; i < nCount; i++)
		{
			prtk_tm_readtrml_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "LNG_CD"				,	PST_FIELD(pList, lng_cd			 ));
			LOG_WRITE("%30s - (%s) ", "TRML_NO"				,	PST_FIELD(pList, trml_no			 ));
			LOG_WRITE("%30s - (%s) ", "TRML_NM"				,	PST_FIELD(pList, trml_nm			 ));
			LOG_WRITE("%30s - (%s) ", "TRML_ABRV_NM"		,	PST_FIELD(pList, trml_abrv_nm		 ));
			LOG_WRITE("%30s - (%s) ", "TRML_DTL_ADDR"		,	PST_FIELD(pList, trml_dtl_addr		 ));
			LOG_WRITE("%30s - (%s) ", "TRML_ENG_NM"			,	PST_FIELD(pList, trml_eng_nm		 ));
			LOG_WRITE("%30s - (%s) ", "TRML_ENG_ABRV_NM"	,	PST_FIELD(pList, trml_eng_abrv_nm	 ));
			LOG_WRITE("%30s - (%s) ", "TRTR_TRML_YN"		,	PST_FIELD(pList, trtr_trml_yn		 ));
			LOG_WRITE("%30s - (%s) ", "SYS_DVS_CD"			,	PST_FIELD(pList, sys_dvs_cd			 ));
			LOG_WRITE("%30s - (%s) ", "TEL_NO"				,	PST_FIELD(pList, tel_no				 ));
			LOG_WRITE("%30s - (%s) ", "STLM_TRML_NO"		,	PST_FIELD(pList, stlm_trml_no		 ));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_CM_ReadTrmlInf
 * @details		터미널 정보 조회 - 8
 * @param		int nOperID			operation ID
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CM_ReadTrmlInf(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "CM_ReadTrmlInf";
	pstk_tm_readtrmlinf_t pSPacket;
	prtk_tm_readtrmlinf_t pRPacket;

	nRet = nTimeout = nCount = 0;

	LOG_OPEN();
	LOG_WRITE("[%d] 터미널 정보 조회_start[%s] !!!!", nOperID, pService);

	pSPacket = (pstk_tm_readtrmlinf_t) pData;
	pRPacket = (prtk_tm_readtrmlinf_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);
	
		LOG_WRITE("%30s - (%s) ","BEF_AFT_DVS"	,	pSPacket->bef_aft_dvs		);
		LOG_WRITE("%30s - (%s) ","USE_YN"		,	pSPacket->use_yn			);
		LOG_WRITE("%30s - (%d) ","REC_NUM"		,	*(int *)pSPacket->rec_num	);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"	,	pSPacket->arvl_trml_no		);
		LOG_WRITE("%30s - (%s) ","TRML_NO"		,	pSPacket->trml_no			);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(BEF_AFT_DVS	,	pSPacket->bef_aft_dvs			);
		nRet = TMaxFBPut(USE_YN			,	pSPacket->use_yn				);
		nRet = TMaxFBPut(REC_NUM		,	pSPacket->rec_num				);
		nRet = TMaxFBPut(ARVL_TRML_NO	,	pSPacket->arvl_trml_no			);
		nRet = TMaxFBPut(TRML_NO		,	pSPacket->trml_no				);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		/// 정보 갯수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num	));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			pRPacket->pList = new rtk_tm_readtrmlinf_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_readtrmlinf_list_t) * nCount);
		}

		{
			HANDLE hFile;
			CString strFullName;

			// file save
			OperGetFileName(nOperID, strFullName);
			hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
			m_nRecNcnt1 = nCount;
			if(hFile == INVALID_HANDLE_VALUE)
			{
				nRet = -100;
				LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
				goto fail_proc;
			}

			MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));

			::ZeroMemory(pRPacket->rec_num, sizeof(pRPacket->rec_num));
			::CopyMemory(pRPacket->rec_num, &m_nRecNcnt1, 4);
			MyWriteFile(hFile, pRPacket->rec_num, sizeof(pRPacket->rec_num));
			MySetFilePointer(hFile, 0, FILE_END);

			for(int i = 0; i < nCount; i++)
			{
				prtk_tm_readtrmlinf_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TRML_NO				,	PST_FIELD(pList, trml_no				));
				nRet = TMaxFBGetF(ARVL_TRML_NO			,	PST_FIELD(pList, arvl_trml_no			));
				
				TMaxGetConvChar(TRML_ABRV_NM,		pList->trml_abrv_nm);
				//nRet = TMaxFBGetF(TRML_ABRV_NM			,	PST_FIELD(pList, trml_abrv_nm			));
				
				nRet = TMaxFBGetF(TRML_ENG_ABRV_NM		,	PST_FIELD(pList, trml_eng_abrv_nm		));
				nRet = TMaxFBGetF(TRML_STUP_CD			,	PST_FIELD(pList, trml_stup_cd			));
				
				TMaxGetConvChar(TRML_SCRN_PRIN_NM,	pList->trml_scrn_prin_nm);
				//nRet = TMaxFBGetF(TRML_SCRN_PRIN_NM		,	PST_FIELD(pList, trml_scrn_prin_nm		));

				TMaxGetConvChar(TRML_PTRG_NM,		pList->trml_ptrg_nm);
				//nRet = TMaxFBGetF(TRML_PTRG_NM			,	PST_FIELD(pList, trml_ptrg_nm			));
				
				nRet = TMaxFBGetF(HSPD_CTY_DVS_CD		,	PST_FIELD(pList, hspd_cty_dvs_cd		));
				nRet = TMaxFBGetF(USE_YN				,	PST_FIELD(pList, use_yn					));

				MyWriteFile(hFile, pList, sizeof(rtk_tm_readtrmlinf_list_t));
			}
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		for(int i = 0; i < nCount; i++)
		{
			prtk_tm_readtrmlinf_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TRML_NO"				,	PST_FIELD(pList, trml_no			 ));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"		,	PST_FIELD(pList, arvl_trml_no		 ));
			LOG_WRITE("%30s - (%s) ", "TRML_ABRV_NM"		,	PST_FIELD(pList, trml_abrv_nm		 ));
			LOG_WRITE("%30s - (%s) ", "TRML_ENG_ABRV_NM"	,	PST_FIELD(pList, trml_eng_abrv_nm	 ));
			LOG_WRITE("%30s - (%s) ", "TRML_STUP_CD"		,	PST_FIELD(pList, trml_stup_cd		 ));
			LOG_WRITE("%30s - (%s) ", "TRML_SCRN_PRIN_NM"	,	PST_FIELD(pList, trml_scrn_prin_nm	 ));
			LOG_WRITE("%30s - (%s) ", "TRML_PTRG_NM"		,	PST_FIELD(pList, trml_ptrg_nm		 ));
			LOG_WRITE("%30s - (%s) ", "HSPD_CTY_DVS_CD"		,	PST_FIELD(pList, hspd_cty_dvs_cd	 ));
			LOG_WRITE("%30s - (%s) ", "USE_YN"				,	PST_FIELD(pList, use_yn				 ));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_CM_ReadTrmlStup
 * @details		터미널 환경설정 정보
 * @param		int nOperID			operation ID
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CM_ReadTrmlStup(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "CM_ReadTrmlStup";
	pstk_tm_readtrmlstup_t pSPacket;
	prtk_tm_readtrmlstup_t pRPacket;

	nRet = nTimeout = nCount = 0;

	LOG_OPEN();
	LOG_WRITE("[%d] 터미널 환경설정 정보_start[%s] !!!!", nOperID, pService);

	pSPacket = (pstk_tm_readtrmlstup_t) pData;
	pRPacket = (prtk_tm_readtrmlstup_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		/// 정보 갯수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num	));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			pRPacket->pList = new rtk_tm_readtrmlstup_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_readtrmlstup_list_t) * nCount);
		}

		{
			HANDLE hFile;
			CString strFullName;

			// file save
			OperGetFileName(nOperID, strFullName);
			hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
			m_nRecNcnt1 = nCount;
			if(hFile == INVALID_HANDLE_VALUE)
			{
				nRet = -100;
				LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
				goto fail_proc;
			}

			MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));

			::ZeroMemory(pRPacket->rec_num, sizeof(pRPacket->rec_num));
			::CopyMemory(pRPacket->rec_num, &m_nRecNcnt1, 4);
			MyWriteFile(hFile, pRPacket->rec_num, sizeof(pRPacket->rec_num));
			MySetFilePointer(hFile, 0, FILE_END);

			for(int i = 0; i < nCount; i++)
			{
				prtk_tm_readtrmlstup_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TRML_NO				,	PST_FIELD(pList, trml_no				));
				nRet = TMaxFBGetF(PGM_STUP_DVS_CD		,	PST_FIELD(pList, pgm_stup_dvs_cd			));
				nRet = TMaxFBGetF(FEE_ADPT_CT_CD		,	PST_FIELD(pList, fee_adpt_ct_cd				));
				nRet = TMaxFBGetF(RY_AMT_ADPT_UNT_VAL	,	PST_FIELD(pList, ry_amt_adpt_unt_val		));
				nRet = TMaxFBGetF(STLM_TIME				,	PST_FIELD(pList, stlm_time					));
				nRet = TMaxFBGetF(SCNG_MRK_DRTM			,	PST_FIELD(pList, scng_mrk_drtm				));
				nRet = TMaxFBGetF(DPSP_PRIN_HCNT		,	PST_FIELD(pList, dpsp_prin_hcnt				));
				nRet = TMaxFBGetF(TRML_TEL_NO_PRIN_YN	,	PST_FIELD(pList, trml_tel_no_prin_yn		));
				nRet = TMaxFBGetF(BCD_PRIN_YN			,	PST_FIELD(pList, bcd_prin_yn				));
				nRet = TMaxFBGetF(ENTE_RP_KEY_VAL		,	PST_FIELD(pList, ente_rp_key_val			));
				nRet = TMaxFBGetF(ESC_RP_KEY_VAL		,	PST_FIELD(pList, esc_rp_key_val				));
				nRet = TMaxFBGetF(STRT_TISSU_KEY_VAL	,	PST_FIELD(pList, strt_tissu_key_val			));
				nRet = TMaxFBGetF(MNG_SHCT_FNCT_KEY_VAL	,	PST_FIELD(pList, mng_shct_fnct_key_val		));
				nRet = TMaxFBGetF(TISSU_SHCT_KEY_VAL	,	PST_FIELD(pList, tissu_shct_key_val			));
				nRet = TMaxFBGetF(RY_GAN_CUTT_YN		,	PST_FIELD(pList, ry_gan_cutt_yn				));
				nRet = TMaxFBGetF(CSRC_TISSU_KEY_VAL	,	PST_FIELD(pList, csrc_tissu_key_val			));
				nRet = TMaxFBGetF(CARD_TISSU_KEY_VAL	,	PST_FIELD(pList, card_tissu_key_val			));
				nRet = TMaxFBGetF(MRS_TISSU_PSB_DNO		,	PST_FIELD(pList, mrs_tissu_psb_dno			));
				nRet = TMaxFBGetF(TRCR_USE_KEY_YN		,	PST_FIELD(pList, trcr_use_key_yn			));
				nRet = TMaxFBGetF(SATS_STUP_USE_YN		,	PST_FIELD(pList, sats_stup_use_yn			));

				MyWriteFile(hFile, pList, sizeof(rtk_tm_readtrmlstup_list_t));
			}
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		for(int i = 0; i < nCount; i++)
		{
			prtk_tm_readtrmlstup_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TRML_NO"					,	PST_FIELD(pList, trml_no				 ));
			LOG_WRITE("%30s - (%s) ", "PGM_STUP_DVS_CD"			,	PST_FIELD(pList, pgm_stup_dvs_cd		 ));
			LOG_WRITE("%30s - (%s) ", "FEE_ADPT_CT_CD"			,	PST_FIELD(pList, fee_adpt_ct_cd			 ));
			LOG_WRITE("%30s - (%d) ", "RY_AMT_ADPT_UNT_VAL"		,	*(int *)pList->ry_amt_adpt_unt_val	 );
			LOG_WRITE("%30s - (%s) ", "STLM_TIME"				,	PST_FIELD(pList, stlm_time				 ));
			LOG_WRITE("%30s - (%d) ", "SCNG_MRK_DRTM"			,	*(int *)pList->scng_mrk_drtm			 );
			LOG_WRITE("%30s - (%d) ", "DPSP_PRIN_HCNT"			,	*(int *)pList->dpsp_prin_hcnt			 );
			LOG_WRITE("%30s - (%s) ", "TRML_TEL_NO_PRIN_YN"		,	PST_FIELD(pList, trml_tel_no_prin_yn	 ));
			LOG_WRITE("%30s - (%s) ", "BCD_PRIN_YN"				,	PST_FIELD(pList, bcd_prin_yn			 ));
			LOG_WRITE("%30s - (%s) ", "ENTE_RP_KEY_VAL"			,	PST_FIELD(pList, ente_rp_key_val		 ));
			LOG_WRITE("%30s - (%s) ", "ESC_RP_KEY_VAL"			,	PST_FIELD(pList, esc_rp_key_val			 ));
			LOG_WRITE("%30s - (%s) ", "STRT_TISSU_KEY_VAL"		,	PST_FIELD(pList, strt_tissu_key_val		 ));
			LOG_WRITE("%30s - (%s) ", "MNG_SHCT_FNCT_KEY_VAL"	,	PST_FIELD(pList, mng_shct_fnct_key_val	 ));
			LOG_WRITE("%30s - (%s) ", "TISSU_SHCT_KEY_VAL"		,	PST_FIELD(pList, tissu_shct_key_val		 ));
			LOG_WRITE("%30s - (%s) ", "RY_GAN_CUTT_YN"			,	PST_FIELD(pList, ry_gan_cutt_yn			 ));
			LOG_WRITE("%30s - (%s) ", "CSRC_TISSU_KEY_VAL"		,	PST_FIELD(pList, csrc_tissu_key_val		 ));
			LOG_WRITE("%30s - (%s) ", "CARD_TISSU_KEY_VAL"		,	PST_FIELD(pList, card_tissu_key_val		 ));
			LOG_WRITE("%30s - (%d) ", "MRS_TISSU_PSB_DNO"		,	*(int *)pList->mrs_tissu_psb_dno		 );
			LOG_WRITE("%30s - (%s) ", "TRCR_USE_KEY_YN"			,	PST_FIELD(pList, trcr_use_key_yn		 ));
			LOG_WRITE("%30s - (%s) ", "SATS_STUP_USE_YN"		,	PST_FIELD(pList, sats_stup_use_yn		 ));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_MG_ReadWnd
 * @details		창구 정보 조회
 * @param		int nOperID			operation ID
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_MG_ReadWnd(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "MG_ReadWnd";
	pstk_tm_readwnd_t pSPacket;
	prtk_tm_readwnd_t pRPacket;

	nRet = nTimeout = nCount = 0;

	LOG_OPEN();
	LOG_WRITE("[%d] 창구 정보 조회_start[%s] !!!!", nOperID, pService);

	pSPacket = (pstk_tm_readwnd_t) pData;
	pRPacket = (prtk_tm_readwnd_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		/// 정보 갯수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num	));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			pRPacket->pList = new rtk_tm_readwnd_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_readwnd_list_t) * nCount);
		}

		{
			HANDLE hFile;
			CString strFullName;

			// file save
			OperGetFileName(nOperID, strFullName);
			hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
			m_nRecNcnt1 = nCount;
			if(hFile == INVALID_HANDLE_VALUE)
			{
				nRet = -100;
				LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
				goto fail_proc;
			}

			MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));

			::ZeroMemory(pRPacket->rec_num, sizeof(pRPacket->rec_num));
			::CopyMemory(pRPacket->rec_num, &m_nRecNcnt1, 4);
			MyWriteFile(hFile, pRPacket->rec_num, sizeof(pRPacket->rec_num));
			MySetFilePointer(hFile, 0, FILE_END);

			for(int i = 0; i < nCount; i++)
			{
				prtk_tm_readwnd_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TRML_NO				,	PST_FIELD(pList, trml_no				));
				nRet = TMaxFBGetF(WND_NO				,	PST_FIELD(pList, wnd_no					));
				nRet = TMaxFBGetF(TISSU_PSB_YN			,	PST_FIELD(pList, tissu_psb_yn			));
				nRet = TMaxFBGetF(RY_PSB_YN				,	PST_FIELD(pList, ry_psb_yn				));
				nRet = TMaxFBGetF(MRNP_PSB_YN			,	PST_FIELD(pList, mrnp_psb_yn			));
				nRet = TMaxFBGetF(MRNP_TISSU_PSB_YN		,	PST_FIELD(pList, mrnp_tissu_psb_yn		));
				nRet = TMaxFBGetF(CARD_TISSU_PSB_YN		,	PST_FIELD(pList, card_tissu_psb_yn		));
				nRet = TMaxFBGetF(NSAT_TISSU_PSB_YN		,	PST_FIELD(pList, nsat_tissu_psb_yn		));
				nRet = TMaxFBGetF(TEMP_NSAT_TISSU_YN	,	PST_FIELD(pList, temp_nsat_tissu_yn		));
				nRet = TMaxFBGetF(OTR_WND_TISSU_CANC_YN	,	PST_FIELD(pList, otr_wnd_tissu_canc_yn	));
				nRet = TMaxFBGetF(CANC_PSB_YN			,	PST_FIELD(pList, canc_psb_yn			));
				nRet = TMaxFBGetF(RPUB_PSB_YN			,	PST_FIELD(pList, rpub_psb_yn			));
				nRet = TMaxFBGetF(THDD_OWT_YN			,	PST_FIELD(pList, thdd_owt_yn			));
				nRet = TMaxFBGetF(CSRC_PUB_YN			,	PST_FIELD(pList, csrc_pub_yn			));
				nRet = TMaxFBGetF(TISSU_CACM_CD1		,	PST_FIELD(pList, tissu_cacm_cd1			));
				nRet = TMaxFBGetF(TISSU_CACM_CD2		,	PST_FIELD(pList, tissu_cacm_cd2			));
				nRet = TMaxFBGetF(TISSU_CACM_CD3		,	PST_FIELD(pList, tissu_cacm_cd3			));
				nRet = TMaxFBGetF(TISSU_CACM_CD4		,	PST_FIELD(pList, tissu_cacm_cd4			));
				nRet = TMaxFBGetF(TISSU_CACM_CD5		,	PST_FIELD(pList, tissu_cacm_cd5			));
				nRet = TMaxFBGetF(TISSU_IMPB_CACM_CD1	,	PST_FIELD(pList, tissu_impb_cacm_cd1	));
				nRet = TMaxFBGetF(TISSU_IMPB_CACM_CD2	,	PST_FIELD(pList, tissu_impb_cacm_cd2	));
				nRet = TMaxFBGetF(TISSU_IMPB_CACM_CD3	,	PST_FIELD(pList, tissu_impb_cacm_cd3	));
				nRet = TMaxFBGetF(TISSU_IMPB_CACM_CD4	,	PST_FIELD(pList, tissu_impb_cacm_cd4	));
				nRet = TMaxFBGetF(TISSU_IMPB_CACM_CD5	,	PST_FIELD(pList, tissu_impb_cacm_cd5	));
				nRet = TMaxFBGetF(SPF_ROT_TISSU_YN		,	PST_FIELD(pList, spf_rot_tissu_yn		));
				nRet = TMaxFBGetF(SPF_TRML_NO_VAL		,	PST_FIELD(pList, spf_trml_no_val		));
				nRet = TMaxFBGetF(TISSU_LTN_DRTM		,	PST_FIELD(pList, tissu_ltn_drtm			));
				nRet = TMaxFBGetF(TISSU_LTN_HCNT		,	PST_FIELD(pList, tissu_ltn_hcnt			));
				nRet = TMaxFBGetF(PTR_KND_CD			,	PST_FIELD(pList, ptr_knd_cd				));
				nRet = TMaxFBGetF(TRML_NM_PTRG_YN		,	PST_FIELD(pList, trml_nm_ptrg_yn		));
				nRet = TMaxFBGetF(SGN_DVC_PORT_VAL		,	PST_FIELD(pList, sgn_dvc_port_val		));
				nRet = TMaxFBGetF(CARD_RCGN_DVC_PORT_VAL,	PST_FIELD(pList, card_rcgn_dvc_port_val	));
				nRet = TMaxFBGetF(TRCN_DVS_ID			,	PST_FIELD(pList, trcn_dvs_id			));
				nRet = TMaxFBGetF(USER_NO				,	PST_FIELD(pList, user_no				));

				MyWriteFile(hFile, pList, sizeof(rtk_tm_readwnd_list_t));
			}
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		for(int i = 0; i < nCount; i++)
		{
			prtk_tm_readwnd_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TRML_NO"					,	PST_FIELD(pList, trml_no				 ));
			LOG_WRITE("%30s - (%s) ", "WND_NO"					,	PST_FIELD(pList, wnd_no					 ));
			LOG_WRITE("%30s - (%s) ", "TISSU_PSB_YN"			,	PST_FIELD(pList, tissu_psb_yn			 ));
			LOG_WRITE("%30s - (%s) ", "RY_PSB_YN"				,	PST_FIELD(pList, ry_psb_yn				 ));
			LOG_WRITE("%30s - (%s) ", "MRNP_PSB_YN"				,	PST_FIELD(pList, mrnp_psb_yn			 ));
			LOG_WRITE("%30s - (%s) ", "MRNP_TISSU_PSB_YN"		,	PST_FIELD(pList, mrnp_tissu_psb_yn		 ));
			LOG_WRITE("%30s - (%s) ", "CARD_TISSU_PSB_YN"		,	PST_FIELD(pList, card_tissu_psb_yn		 ));
			LOG_WRITE("%30s - (%s) ", "NSAT_TISSU_PSB_YN"		,	PST_FIELD(pList, nsat_tissu_psb_yn		 ));
			LOG_WRITE("%30s - (%s) ", "TEMP_NSAT_TISSU_YN"		,	PST_FIELD(pList, temp_nsat_tissu_yn		 ));
			LOG_WRITE("%30s - (%s) ", "OTR_WND_TISSU_CANC_YN"	,	PST_FIELD(pList, otr_wnd_tissu_canc_yn	 ));
			LOG_WRITE("%30s - (%s) ", "CANC_PSB_YN"				,	PST_FIELD(pList, canc_psb_yn			 ));
			LOG_WRITE("%30s - (%s) ", "RPUB_PSB_YN"				,	PST_FIELD(pList, rpub_psb_yn			 ));
			LOG_WRITE("%30s - (%s) ", "THDD_OWT_YN"				,	PST_FIELD(pList, thdd_owt_yn			 ));
			LOG_WRITE("%30s - (%s) ", "CSRC_PUB_YN"				,	PST_FIELD(pList, csrc_pub_yn			 ));
			LOG_WRITE("%30s - (%s) ", "TISSU_CACM_CD1"			,	PST_FIELD(pList, tissu_cacm_cd1			 ));
			LOG_WRITE("%30s - (%s) ", "TISSU_CACM_CD2"			,	PST_FIELD(pList, tissu_cacm_cd2			 ));
			LOG_WRITE("%30s - (%s) ", "TISSU_CACM_CD3"			,	PST_FIELD(pList, tissu_cacm_cd3			 ));
			LOG_WRITE("%30s - (%s) ", "TISSU_CACM_CD4"			,	PST_FIELD(pList, tissu_cacm_cd4			 ));
			LOG_WRITE("%30s - (%s) ", "TISSU_CACM_CD5"			,	PST_FIELD(pList, tissu_cacm_cd5			 ));
			LOG_WRITE("%30s - (%s) ", "TISSU_IMPB_CACM_CD1"		,	PST_FIELD(pList, tissu_impb_cacm_cd1	 ));
			LOG_WRITE("%30s - (%s) ", "TISSU_IMPB_CACM_CD2"		,	PST_FIELD(pList, tissu_impb_cacm_cd2	 ));
			LOG_WRITE("%30s - (%s) ", "TISSU_IMPB_CACM_CD3"		,	PST_FIELD(pList, tissu_impb_cacm_cd3	 ));
			LOG_WRITE("%30s - (%s) ", "TISSU_IMPB_CACM_CD4"		,	PST_FIELD(pList, tissu_impb_cacm_cd4	 ));
			LOG_WRITE("%30s - (%s) ", "TISSU_IMPB_CACM_CD5"		,	PST_FIELD(pList, tissu_impb_cacm_cd5	 ));
			LOG_WRITE("%30s - (%s) ", "SPF_ROT_TISSU_YN"		,	PST_FIELD(pList, spf_rot_tissu_yn		 ));
			LOG_WRITE("%30s - (%s) ", "SPF_TRML_NO_VAL"			,	PST_FIELD(pList, spf_trml_no_val		 ));
			LOG_WRITE("%30s - (%d) ", "TISSU_LTN_DRTM"			,	*(int *)pList->tissu_ltn_drtm			 );
			LOG_WRITE("%30s - (%d) ", "TISSU_LTN_HCNT"			,	*(int *)pList->tissu_ltn_hcnt			 );
			LOG_WRITE("%30s - (%s) ", "PTR_KND_CD"				,	PST_FIELD(pList, ptr_knd_cd				 ));
			LOG_WRITE("%30s - (%s) ", "TRML_NM_PTRG_YN"			,	PST_FIELD(pList, trml_nm_ptrg_yn		 ));
			LOG_WRITE("%30s - (%s) ", "SGN_DVC_PORT_VAL"		,	PST_FIELD(pList, sgn_dvc_port_val		 ));
			LOG_WRITE("%30s - (%s) ", "CARD_RCGN_DVC_PORT_VAL"	,	PST_FIELD(pList, card_rcgn_dvc_port_val	 ));
			LOG_WRITE("%30s - (%s) ", "TRCN_DVS_ID"				,	PST_FIELD(pList, trcn_dvs_id			 ));
			LOG_WRITE("%30s - (%s) ", "USER_NO"					,	PST_FIELD(pList, user_no				 ));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_CM_ReadRtrpTrml
 * @details		왕복 가능 터미널 정보 조회
 * @param		int nOperID			operation ID
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CM_ReadRtrpTrml(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "CM_ReadRtrpTrml";
	pstk_tm_readrtrptrml_t pSPacket;
	prtk_tm_readrtrptrml_t pRPacket;

	nRet = nTimeout = nCount = 0;

	LOG_OPEN();
	LOG_WRITE("[%d] 왕복 가능 터미널 정보 조회_start[%s] !!!!", nOperID, pService);

	pSPacket = (pstk_tm_readrtrptrml_t) pData;
	pRPacket = (prtk_tm_readrtrptrml_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		/// 정보 갯수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num	));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
		//if(nCount <= 0)	// 20220825 DEL
		if(nCount < 0)		// 20220825 MOD
		{
			// 에러코드 조회
			nRet = -99;
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}
		else if(nCount == 0)	// 20220825 ADD~
		{ // 왕복 가능 터미널 정보 조회 성공, 건수=0 (왕복 가능 터미널 없음)
			nRet = 1;
			goto fail_proc;
		}						// 20220825 ~ADD

		/// memory allocation
		{
			pRPacket->pList = new rtk_tm_readrtrptrml_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_readrtrptrml_list_t) * nCount);
		}

		{
			HANDLE hFile;
			CString strFullName;

			// file save
			OperGetFileName(nOperID, strFullName);
			hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
			m_nRecNcnt1 = nCount;
			if(hFile == INVALID_HANDLE_VALUE)
			{
				nRet = -100;
				LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
				goto fail_proc;
			}

			MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));

			::ZeroMemory(pRPacket->rec_num, sizeof(pRPacket->rec_num));
			::CopyMemory(pRPacket->rec_num, &m_nRecNcnt1, 4);
			MyWriteFile(hFile, pRPacket->rec_num, sizeof(pRPacket->rec_num));
			MySetFilePointer(hFile, 0, FILE_END);

			for(int i = 0; i < nCount; i++)
			{
				prtk_tm_readrtrptrml_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(ARVL_TRML_NO	,	PST_FIELD(pList, arvl_trml_no	));

				MyWriteFile(hFile, pList, sizeof(rtk_tm_readrtrptrml_list_t));
			}
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		for(int i = 0; i < nCount; i++)
		{
			prtk_tm_readrtrptrml_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"	,	PST_FIELD(pList, arvl_trml_no				 ));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_CM_ReadTckKnd
 * @details		승차권 종류 정보 조회
 * @param		int nOperID			operation ID
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CM_ReadTckKnd(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "CM_ReadTckKnd";
	pstk_tm_readtckknd_t pSPacket;
	prtk_tm_readtckknd_t pRPacket;

	nRet = nTimeout = nCount = 0;

	LOG_OPEN();
	LOG_WRITE("[%d] 승차권 종류 정보 조회_start[%s] !!!!", nOperID, pService);

	pSPacket = (pstk_tm_readtckknd_t) pData;
	pRPacket = (prtk_tm_readtckknd_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","USE_YN"		,	pSPacket->use_yn			);
		LOG_WRITE("%30s - (%d) ","REQ_TRML_NUM"	,	*(int *)pSPacket->req_trml_num		);
		LOG_WRITE("%30s - (%s) ","TRML_NO"		,	pSPacket->trml_no			);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(USE_YN			,	pSPacket->use_yn			);
		nRet = TMaxFBPut(REQ_TRML_NUM	,	pSPacket->req_trml_num		);
		nRet = TMaxFBPut(TRML_NO		,	pSPacket->trml_no			);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		/// 정보 갯수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num	));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			pRPacket->pList = new rtk_tm_readtckknd_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_readtckknd_list_t) * nCount);
		}

		{
			HANDLE hFile;
			CString strFullName;

			// file save
			OperGetFileName(nOperID, strFullName);
			hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
			m_nRecNcnt1 = nCount;
			if(hFile == INVALID_HANDLE_VALUE)
			{
				nRet = -100;
				LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
				goto fail_proc;
			}

			MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));

			::ZeroMemory(pRPacket->rec_num, sizeof(pRPacket->rec_num));
			::CopyMemory(pRPacket->rec_num, &m_nRecNcnt1, 4);
			MyWriteFile(hFile, pRPacket->rec_num, sizeof(pRPacket->rec_num));
			MySetFilePointer(hFile, 0, FILE_END);

			for(int i = 0; i < nCount; i++)
			{
				prtk_tm_readtckknd_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TRML_NO		,	PST_FIELD(pList, trml_no		));
				nRet = TMaxFBGetF(TCK_KND_CD	,	PST_FIELD(pList, tck_knd_cd		));
				
				TMaxGetConvChar(SCRN_PRIN_NM, pList->scrn_prin_nm);
				//nRet = TMaxFBGetF(SCRN_PRIN_NM	,	PST_FIELD(pList, scrn_prin_nm	));

				nRet = TMaxFBGetF(SCRN_PRIN_SEQ	,	PST_FIELD(pList, scrn_prin_seq	));
				
				TMaxGetConvChar(PTRG_PRIN_NM, pList->ptrg_prin_nm);
				//nRet = TMaxFBGetF(PTRG_PRIN_NM	,	PST_FIELD(pList, ptrg_prin_nm	));

				MyWriteFile(hFile, pList, sizeof(rtk_tm_readtckknd_list_t));
			}
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		for(int i = 0; i < nCount; i++)
		{
			prtk_tm_readtckknd_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TRML_NO"			,	PST_FIELD(pList, trml_no			 ));
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"		,	PST_FIELD(pList, tck_knd_cd			 ));
			LOG_WRITE("%30s - (%s) ", "SCRN_PRIN_NM"	,	PST_FIELD(pList, scrn_prin_nm		 ));
			LOG_WRITE("%30s - (%d) ", "SCRN_PRIN_SEQ"	,	*(int *)pList->scrn_prin_seq		 );
			LOG_WRITE("%30s - (%s) ", "PTRG_PRIN_NM"	,	PST_FIELD(pList, ptrg_prin_nm		 ));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_CM_MngCacm
 * @details		운송회사 정보 조회
 * @param		int nOperID			operation ID
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CM_MngCacm(int nOperID, int nIndex, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "CM_MngCacm";
	pstk_tm_mngcacm_t pSPacket;
	prtk_tm_mngcacm_t pRPacket;

	nRet = nTimeout = nCount = 0;

	if(nIndex == 0)
	{
		m_nRecNcnt1 = 0;
	}

	LOG_OPEN();
	LOG_WRITE("[%d] 운송회사 정보 조회_start[%s] !!!!", nOperID, pService);

	pSPacket = (pstk_tm_mngcacm_t) pData;
	pRPacket = (prtk_tm_mngcacm_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","USE_YN"		,	pSPacket->use_yn			);
		LOG_WRITE("%30s - (%s) ","CACM_CD"		,	pSPacket->cacm_cd			);
		LOG_WRITE("%30s - (%s) ","LNG_CD"		,	pSPacket->lng_cd			);
		LOG_WRITE("%30s - (%d) ","REC_NUM"		,	*(int *)pSPacket->rec_num	);
		LOG_WRITE("%30s - (%s) ","BEF_AFT_DVS"	,	pSPacket->bef_aft_dvs		);
		LOG_WRITE("%30s - (%d) ","REQ_TRML_NUM"	,	*(int *)pSPacket->req_trml_num		);
		LOG_WRITE("%30s - (%s) ","TRML_NO"		,	pSPacket->trml_no			);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(USE_YN			,	pSPacket->use_yn				);
		nRet = TMaxFBPut(CACM_CD		,	pSPacket->cacm_cd				);
		nRet = TMaxFBPut(LNG_CD			,	pSPacket->lng_cd				);
		nRet = TMaxFBPut(REC_NUM		,	pSPacket->rec_num				);
		nRet = TMaxFBPut(BEF_AFT_DVS	,	pSPacket->bef_aft_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NUM	,	pSPacket->req_trml_num			);
		nRet = TMaxFBPut(TRML_NO		,	pSPacket->trml_no				);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		/// 정보 갯수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num	));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			pRPacket->pList = new rtk_tm_mngcacm_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_mngcacm_list_t) * nCount);
		}

		{
			HANDLE hFile;
			CString strFullName;

			// file save
			OperGetFileName(nOperID, strFullName);
			if(nIndex == 0)
			{
				hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
				m_nRecNcnt1 = nCount;
			}
			else
			{
				hFile = MyOpenFile(strFullName, TRUE);
				m_nRecNcnt1 += nCount;
			}
			if(hFile == INVALID_HANDLE_VALUE)
			{
				nRet = -100;
				LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
				goto fail_proc;
			}

			MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));

			::ZeroMemory(pRPacket->rec_num, sizeof(pRPacket->rec_num));
			::CopyMemory(pRPacket->rec_num, &m_nRecNcnt1, 4);
			MyWriteFile(hFile, pRPacket->rec_num, sizeof(pRPacket->rec_num));
			MySetFilePointer(hFile, 0, FILE_END);

			for(int i = 0; i < nCount; i++)
			{
				prtk_tm_mngcacm_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TRML_NO			,	PST_FIELD(pList, trml_no			));
				nRet = TMaxFBGetF(HSPD_CTY_DVS_CD	,	PST_FIELD(pList, hspd_cty_dvs_cd	));
				nRet = TMaxFBGetF(CACM_CD			,	PST_FIELD(pList, cacm_cd			));
				nRet = TMaxFBGetF(LNG_CD			,	PST_FIELD(pList, lng_cd				));
				
				switch(nIndex)
				{
				case LANG_DVS_KO:
					TMaxGetConvChar(CACM_NM			,	PST_FIELD(pList, cacm_nm			));
					TMaxGetConvChar(CACM_ABRV_NM		,	PST_FIELD(pList, cacm_abrv_nm		));
					break;
				case LANG_DVS_ENG:
				case LANG_DVS_CHI:
				case LANG_DVS_JPN:
					nRet = TMaxFBGetF(CACM_NM			,	PST_FIELD(pList, cacm_nm			));
					nRet = TMaxFBGetF(CACM_ABRV_NM		,	PST_FIELD(pList, cacm_abrv_nm		));
					break;
				}

				nRet = TMaxFBGetF(BIZR_NO			,	PST_FIELD(pList, bizr_no			));
				nRet = TMaxFBGetF(TRML_BY_CACM_CD	,	PST_FIELD(pList, trml_by_cacm_cd	));
				
				switch(nIndex)
				{
				case LANG_DVS_KO:
					TMaxGetConvChar(SCRN_PRIN_NM		,	PST_FIELD(pList, scrn_prin_nm		));
					TMaxGetConvChar(PTRG_PRIN_NM		,	PST_FIELD(pList, ptrg_prin_nm		));
					break;
				case LANG_DVS_ENG:
				case LANG_DVS_CHI:
				case LANG_DVS_JPN:
					nRet = TMaxFBGetF(SCRN_PRIN_NM		,	PST_FIELD(pList, scrn_prin_nm		));
					nRet = TMaxFBGetF(PTRG_PRIN_NM		,	PST_FIELD(pList, ptrg_prin_nm		));
					break;
				}

				
				nRet = TMaxFBGetF(MOD_STA_CD		,	PST_FIELD(pList, mod_sta_cd			));

				MyWriteFile(hFile, pList, sizeof(rtk_tm_mngcacm_list_t));
			}
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		for(int i = 0; i < nCount; i++)
		{
			prtk_tm_mngcacm_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "trml_no"			,	PST_FIELD(pList, trml_no		 ));
			LOG_WRITE("%30s - (%s) ", "hspd_cty_dvs_cd"	,	PST_FIELD(pList, hspd_cty_dvs_cd ));
			LOG_WRITE("%30s - (%s) ", "cacm_cd"			,	PST_FIELD(pList, cacm_cd		 ));
			LOG_WRITE("%30s - (%s) ", "lng_cd"			,	PST_FIELD(pList, lng_cd			 ));
			LOG_WRITE("%30s - (%s) ", "cacm_nm"			,	PST_FIELD(pList, cacm_nm		 ));
			LOG_WRITE("%30s - (%s) ", "cacm_abrv_nm"	,	PST_FIELD(pList, cacm_abrv_nm	 ));
			LOG_WRITE("%30s - (%s) ", "bizr_no"			,	PST_FIELD(pList, bizr_no		 ));
			LOG_WRITE("%30s - (%s) ", "trml_by_cacm_cd"	,	PST_FIELD(pList, trml_by_cacm_cd ));
			LOG_WRITE("%30s - (%s) ", "scrn_prin_nm"	,	PST_FIELD(pList, scrn_prin_nm	 ));
			LOG_WRITE("%30s - (%s) ", "ptrg_prin_nm"	,	PST_FIELD(pList, ptrg_prin_nm	 ));
			LOG_WRITE("%30s - (%s) ", "mod_sta_cd"		,	PST_FIELD(pList, mod_sta_cd		 ));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_CM_ReadRotInf
 * @details		노선 정보 조회
 * @param		int nOperID			operation ID
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CM_ReadRotInf(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "CM_ReadRotInf";
	pstk_tm_readrotinf_t pSPacket;
	prtk_tm_readrotinf_t pRPacket;

	nRet = nTimeout = nCount = 0;

	LOG_OPEN();
	LOG_WRITE("[%d] 노선 정보 조회_start[%s] !!!!", nOperID, pService);

	pSPacket = (pstk_tm_readrotinf_t) pData;
	pRPacket = (prtk_tm_readrotinf_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO"	,	pSPacket->depr_trml_no		);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"	,	pSPacket->arvl_trml_no		);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(DEPR_TRML_NO	,	pSPacket->depr_trml_no			);
		nRet = TMaxFBPut(ARVL_TRML_NO	,	pSPacket->arvl_trml_no			);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		/// 정보 갯수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num	));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			pRPacket->pList = new rtk_tm_readrotinf_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_readrotinf_list_t) * nCount);
		}

		{
			HANDLE hFile;
			CString strFullName;

			// file save
			OperGetFileName(nOperID, strFullName);
			hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
			m_nRecNcnt1 = nCount;
			if(hFile == INVALID_HANDLE_VALUE)
			{
				nRet = -100;
				LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
				goto fail_proc;
			}

			MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));

			::ZeroMemory(pRPacket->rec_num, sizeof(pRPacket->rec_num));
			::CopyMemory(pRPacket->rec_num, &m_nRecNcnt1, 4);
			MyWriteFile(hFile, pRPacket->rec_num, sizeof(pRPacket->rec_num));
			MySetFilePointer(hFile, 0, FILE_END);

			for(int i = 0; i < nCount; i++)
			{
				prtk_tm_readrotinf_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(DEPR_TRML_NO			,	PST_FIELD(pList, depr_trml_no		));
				nRet = TMaxFBGetF(ARVL_TRML_NO			,	PST_FIELD(pList, arvl_trml_no		));
				nRet = TMaxFBGetF(MRS_PSB_YN			,	PST_FIELD(pList, mrs_psb_yn			));
				nRet = TMaxFBGetF(HTCK_PSB_YN			,	PST_FIELD(pList, htck_psb_yn		));
				nRet = TMaxFBGetF(BUS_OPRN_DIST			,	PST_FIELD(pList, bus_oprn_dist		));
				nRet = TMaxFBGetF(TAKE_DRTM				,	PST_FIELD(pList, take_drtm			));
				nRet = TMaxFBGetF(ROT_DVS_CD			,	PST_FIELD(pList, rot_dvs_cd			));
				nRet = TMaxFBGetF(FEE_KND_CD			,	PST_FIELD(pList, fee_knd_cd			));
				nRet = TMaxFBGetF(BSC_NTKN_CD			,	PST_FIELD(pList, bsc_ntkn_cd		));
				nRet = TMaxFBGetF(OPRN_YN				,	PST_FIELD(pList, oprn_yn			));
				nRet = TMaxFBGetF(RTRP_TISSU_PSB_YN		,	PST_FIELD(pList, rtrp_tissu_psb_yn	));
				nRet = TMaxFBGetF(CTY_PRMM_DC_YN		,	PST_FIELD(pList, cty_prmm_dc_yn		));
				nRet = TMaxFBGetF(HSPD_CTY_DVS_CD		,	PST_FIELD(pList, hspd_cty_dvs_cd	));
				nRet = TMaxFBGetF(TRTR_TRML_INCL_YN		,	PST_FIELD(pList, trtr_trml_incl_yn	));
				nRet = TMaxFBGetF(TRML_DVS_CD			,	PST_FIELD(pList, trml_dvs_cd		));

				MyWriteFile(hFile, pList, sizeof(rtk_tm_readrotinf_list_t));
			}
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		for(int i = 0; i < nCount; i++)
		{
			prtk_tm_readrotinf_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"		,	PST_FIELD(pList, depr_trml_no		 ));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"		,	PST_FIELD(pList, arvl_trml_no		 ));
			LOG_WRITE("%30s - (%s) ", "MRS_PSB_YN"			,	PST_FIELD(pList, mrs_psb_yn			 ));
			LOG_WRITE("%30s - (%s) ", "HTCK_PSB_YN"			,	PST_FIELD(pList, htck_psb_yn		 ));
			LOG_WRITE("%30s - (%s) ", "BUS_OPRN_DIST"		,	PST_FIELD(pList, bus_oprn_dist		 ));
			LOG_WRITE("%30s - (%d) ", "TAKE_DRTM"			,	*(int *)pList->take_drtm			 );
			LOG_WRITE("%30s - (%s) ", "ROT_DVS_CD"			,	PST_FIELD(pList, rot_dvs_cd			 ));
			LOG_WRITE("%30s - (%s) ", "FEE_KND_CD"			,	PST_FIELD(pList, fee_knd_cd			 ));
			LOG_WRITE("%30s - (%s) ", "BSC_NTKN_CD"			,	PST_FIELD(pList, bsc_ntkn_cd		 ));
			LOG_WRITE("%30s - (%s) ", "OPRN_YN"				,	PST_FIELD(pList, oprn_yn			 ));
			LOG_WRITE("%30s - (%s) ", "RTRP_TISSU_PSB_YN"	,	PST_FIELD(pList, rtrp_tissu_psb_yn	 ));
			LOG_WRITE("%30s - (%s) ", "CTY_PRMM_DC_YN"		,	PST_FIELD(pList, cty_prmm_dc_yn		 ));
			LOG_WRITE("%30s - (%s) ", "HSPD_CTY_DVS_CD"		,	PST_FIELD(pList, hspd_cty_dvs_cd	 ));
			LOG_WRITE("%30s - (%s) ", "TRTR_TRML_INCL_YN"	,	PST_FIELD(pList, trtr_trml_incl_yn	 ));
			LOG_WRITE("%30s - (%s) ", "TRML_DVS_CD"			,	PST_FIELD(pList, trml_dvs_cd		 ));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_ChangeTicketBox
 * @details		[티머니고속] 승차권 박스 교환
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_ChangeTicketBox(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "TK_ExcBusTck";
	pstk_tm_excbustck_t pSPacket;
	prtk_tm_excbustck_t pRPacket;

	nRet = nTimeout = nCount = 0;

	LOG_OPEN();
	LOG_WRITE("승차권 박스 교환_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_excbustck_t) pData;
	pRPacket = (prtk_tm_excbustck_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","ECG_INHR_NO"	,	pSPacket->ecg_inhr_no		);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(ECG_INHR_NO	,	pSPacket->ecg_inhr_no			);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
	}

	///> data proc
	{
		nRet = 1;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_CloseWnd
 * @details		[티머니고속] 창구마감
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CloseWnd(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount;
	char*	pService = "TK_ClosWnd";
	pstk_tm_closwnd_t pSPacket;
	prtk_tm_closwnd_t pRPacket;

	nRet = nTimeout = nCount = 0;

	LOG_OPEN();
	LOG_WRITE(" 창구마감_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_closwnd_t) pData;
	pRPacket = (prtk_tm_closwnd_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","INHR_NO"		,	pSPacket->inhr_no		);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(INHR_NO		,	pSPacket->inhr_no				);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		/// 작업 시작일자
		nRet = TMaxFBGet(TAK_STT_DT	, PST_FIELD(pRPacket, tak_stt_dt));

		/// 시작시각
		nRet = TMaxFBGet(STT_TIME	, PST_FIELD(pRPacket, stt_time));
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s - (%s) ", "TAK_STT_DT"	,	PST_FIELD(pRPacket, tak_stt_dt	));
		LOG_WRITE("%30s - (%s) ", "STT_TIME"	,	PST_FIELD(pRPacket, stt_time	));
	}

	///> data proc
	{
		nRet = 1;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_MG_ReadAlcn
 * @details		[티머니고속] 현장발권 - 배차조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_MG_ReadAlcn(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nRealCount, nLastCount;
	char*	pService = "MG_ReadAlcn";
	pstk_tm_readalcn_t pSPacket;
	prtk_tm_readalcn_t pRPacket;

	nRet = nTimeout = nCount = nRealCount = nLastCount = 0;

	LOG_OPEN();
	LOG_WRITE(" 현장발권 - 배차조회_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_readalcn_t) pData;
	pRPacket = (prtk_tm_readalcn_t) retBuf;

	/// [티머니고속] 배차리스트 mem 초기화
	CPubTckTmExpMem::GetInstance()->m_vtResAlcnLastList.clear();
	CPubTckTmExpMem::GetInstance()->m_vtResAlcnList.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (Num=%d) ","REC_NUM"		,	*(int *)pSPacket->rec_num	);
		LOG_WRITE("%30s - (%s) ","BEF_AFT_DVS"		,	pSPacket->bef_aft_dvs		);
		LOG_WRITE("%30s - (%s) ","DEPR_DT"			,	pSPacket->depr_dt			);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO"		,	pSPacket->depr_trml_no		);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"		,	pSPacket->arvl_trml_no		);
		LOG_WRITE("%30s - (%s) ","BUS_CLS_CD"		,	pSPacket->bus_cls_cd		);
		LOG_WRITE("%30s - (%s) ","DEPR_TIME"		,	pSPacket->depr_time			);
		LOG_WRITE("%30s - (%s) ","READ_BCNL_YN"		,	pSPacket->read_bcnl_yn		);
		LOG_WRITE("%30s - (%s) ","READ_RMN_YN"		,	pSPacket->read_rmn_yn		);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TRML_NO",	pSPacket->alcn_depr_trml_no	);
		LOG_WRITE("%30s - (%s) ","ALCN_ARVL_TRML_NO",	pSPacket->alcn_arvl_trml_no	);
		LOG_WRITE("%30s - (%s) ","ALCN_ROT_NO"		,	pSPacket->alcn_rot_no		);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(REC_NUM				,	pSPacket->rec_num				);
		nRet = TMaxFBPut(BEF_AFT_DVS			,	pSPacket->bef_aft_dvs			);
		nRet = TMaxFBPut(DEPR_DT				,	pSPacket->depr_dt				);
		nRet = TMaxFBPut(DEPR_TRML_NO			,	pSPacket->depr_trml_no			);
		nRet = TMaxFBPut(ARVL_TRML_NO			,	pSPacket->arvl_trml_no			);
		nRet = TMaxFBPut(BUS_CLS_CD				,	pSPacket->bus_cls_cd			);
		nRet = TMaxFBPut(DEPR_TIME				,	pSPacket->depr_time				);
		nRet = TMaxFBPut(READ_BCNL_YN			,	pSPacket->read_bcnl_yn			);
		nRet = TMaxFBPut(READ_RMN_YN			,	pSPacket->read_rmn_yn			);
		nRet = TMaxFBPut(ALCN_DEPR_TRML_NO		,	pSPacket->alcn_depr_trml_no		);
		nRet = TMaxFBPut(ALCN_ARVL_TRML_NO		,	pSPacket->alcn_arvl_trml_no		);
		nRet = TMaxFBPut(ALCN_ROT_NO			,	pSPacket->alcn_rot_no			);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{

		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
		
		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.rsp_cd, pRPacket->rsp_cd, sizeof(CPubTckTmExpMem::GetInstance()->base.rsp_cd));

		/// 이전이후 구분
		nRet = TMaxFBGet(BEF_AFT_DVS				, PST_FIELD(pRPacket, bef_aft_dvs			));
		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.bef_aft_dvs, pRPacket->bef_aft_dvs, sizeof(CPubTckTmExpMem::GetInstance()->base.bef_aft_dvs));
		
		/// 최종 배차노선수
		nRet = TMaxFBGet(LAST_ALCN_NUM				, PST_FIELD(pRPacket, last_alcn_num			));
		::CopyMemory(&nLastCount, pRPacket->last_alcn_num, 4);
		LOG_WRITE("%30s - (%d) ", "LAST_ALCN_NUM", nLastCount);

		if(nLastCount > 0)
		{							  
			pRPacket->pLastList = new rtk_tm_readalcn_last_list_t[nLastCount];
			::ZeroMemory(pRPacket->pLastList, sizeof(rtk_tm_readalcn_last_list_t) * nLastCount);

			for(i = 0; i < nLastCount; i++)
			{
				prtk_tm_readalcn_last_list_t pList;

				pList = &pRPacket->pLastList[i];

				/// 최종 배차방면번호
				nRet = TMaxFBGet(LAST_DRTN_CD				, PST_FIELD(pList, last_drtn_cd			));
				/// 최종 배차 출발터미널번호
				nRet = TMaxFBGet(LAST_ALCN_DEPR_TRML_NO		, PST_FIELD(pList, last_alcn_depr_trml_no	));
				/// 최종 배차 도착터미널번호
				nRet = TMaxFBGet(LAST_ALCN_ARVL_TRML_NO		, PST_FIELD(pList, last_alcn_arvl_trml_no	));
				/// 최종 배차 노선번호
				nRet = TMaxFBGet(LAST_ALCN_ROT_NO			, PST_FIELD(pList, last_alcn_rot_no		));
				/// 최종 배차일
				nRet = TMaxFBGet(LAST_DEPR_DT				, PST_FIELD(pList, last_depr_dt			));

				/// 최종 배차노선 정보
				CPubTckTmExpMem::GetInstance()->m_vtResAlcnLastList.push_back(*pList);
			}
		}

		/// 정보 수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			pRPacket->pList = new rtk_tm_readalcn_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_readalcn_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				prtk_tm_readalcn_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(DRTN_CD				,	PST_FIELD(pList, drtn_cd				));
				nRet = TMaxFBGetF(ALCN_DEPR_TRML_NO		,	PST_FIELD(pList, alcn_depr_trml_no		));
				nRet = TMaxFBGetF(ALCN_ARVL_TRML_NO		,	PST_FIELD(pList, alcn_arvl_trml_no		));
				nRet = TMaxFBGetF(ALCN_ROT_NO			,	PST_FIELD(pList, alcn_rot_no			));
				nRet = TMaxFBGetF(ALCN_ROT_NM			,	PST_FIELD(pList, alcn_rot_nm			));
				nRet = TMaxFBGetF(DEPR_TRML_NO			,	PST_FIELD(pList, depr_trml_no			));
				nRet = TMaxFBGetF(ARVL_TRML_NO			,	PST_FIELD(pList, arvl_trml_no			));
				nRet = TMaxFBGetF(ALCN_DEPR_DT			,	PST_FIELD(pList, alcn_depr_dt			));
				nRet = TMaxFBGetF(ALCN_DEPR_TIME		,	PST_FIELD(pList, alcn_depr_time			));
				nRet = TMaxFBGetF(DEPR_DT				,	PST_FIELD(pList, depr_dt				));
				nRet = TMaxFBGetF(DEPR_TIME				,	PST_FIELD(pList, depr_time				));
				nRet = TMaxFBGetF(BUS_CLS_CD			,	PST_FIELD(pList, bus_cls_cd				));
				nRet = TMaxFBGetF(CACM_CD				,	PST_FIELD(pList, cacm_cd				));
				nRet = TMaxFBGetF(ALCN_DVS_CD			,	PST_FIELD(pList, alcn_dvs_cd			));
				nRet = TMaxFBGetF(BCNL_DVS_CD			,	PST_FIELD(pList, bcnl_dvs_cd			));
				nRet = TMaxFBGetF(RMN_SATS_NUM			,	PST_FIELD(pList, n_rmn_sats_num			));
				nRet = TMaxFBGetF(NSAT_NUM				,	PST_FIELD(pList, n_nsat_num				));
				nRet = TMaxFBGetF(TOT_SATS_NUM			,	PST_FIELD(pList, n_tot_sats_num			));
				nRet = TMaxFBGetF(TISSU_ABLE_NUM		,	PST_FIELD(pList, n_tissu_able_num			));
				nRet = TMaxFBGetF(CHLD_SFTY_SATS_YN		,	PST_FIELD(pList, chld_sfty_sats_yn		));
				nRet = TMaxFBGetF(CACM_NM_PRIN_YN		,	PST_FIELD(pList, cacm_nm_prin_yn		));
				nRet = TMaxFBGetF(BUS_CLS_PRIN_YN		,	PST_FIELD(pList, bus_cls_prin_yn		));
				nRet = TMaxFBGetF(DEPR_TIME_PRIN_YN		,	PST_FIELD(pList, depr_time_prin_yn		));
				nRet = TMaxFBGetF(SATS_NO_PRIN_YN		,	PST_FIELD(pList, sats_no_prin_yn		));
				nRet = TMaxFBGetF(BUS_OPRN_DIST			,	PST_FIELD(pList, bus_oprn_dist			));
				nRet = TMaxFBGetF(TAKE_DRTM				,	PST_FIELD(pList, n_take_drtm				));
				nRet = TMaxFBGetF(ROT_RDHM_NO_VAL		,	PST_FIELD(pList, rot_rdhm_no_val		));
				nRet = TMaxFBGetF(CTY_PRMM_DC_YN		,	PST_FIELD(pList, cty_prmm_dc_yn			));
				nRet = TMaxFBGetF(SATS_STUP_SNO			,	PST_FIELD(pList, sats_stup_sno			));
				nRet = TMaxFBGetF(DSPR_SATS_YN			,	PST_FIELD(pList, dspr_sats_yn			));

				/// [티머니고속] 배차리스트 memory add
				CPubTckTmExpMem::GetInstance()->m_vtResAlcnList.push_back(*pList);
				nRealCount++;
			}
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		::CopyMemory(&nLastCount, pRPacket->last_alcn_num, 4);
		for(i = 0; i < nLastCount; i++)
		{
			prtk_tm_readalcn_last_list_t pList;

			pList = &pRPacket->pLastList[i];

			LOG_WRITE("####### 인덱스_LastList (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "last_drtn_cd"			,	PST_FIELD(pList, last_drtn_cd				));
			LOG_WRITE("%30s - (%s) ", "last_alcn_depr_trml_no"	,	PST_FIELD(pList, last_alcn_depr_trml_no		));
			LOG_WRITE("%30s - (%s) ", "last_alcn_arvl_trml_no"	,	PST_FIELD(pList, last_alcn_arvl_trml_no		));
			LOG_WRITE("%30s - (%s) ", "last_alcn_rot_no	"		,	PST_FIELD(pList, last_alcn_rot_no			));
			LOG_WRITE("%30s - (%s) ", "last_depr_dt"			,	PST_FIELD(pList, last_depr_dt				));
		}

		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_readalcn_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "DRTN_CD"				,	PST_FIELD(pList, drtn_cd				));
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO"	,	PST_FIELD(pList, alcn_depr_trml_no		));
			LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO"	,	PST_FIELD(pList, alcn_arvl_trml_no		));
			LOG_WRITE("%30s - (%s) ", "ALCN_ROT_NO"			,	PST_FIELD(pList, alcn_rot_no			));
			LOG_WRITE("%30s - (%s) ", "ALCN_ROT_NM"			,	PST_FIELD(pList, alcn_rot_nm			));
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"		,	PST_FIELD(pList, depr_trml_no			));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"		,	PST_FIELD(pList, arvl_trml_no			));
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_DT"		,	PST_FIELD(pList, alcn_depr_dt			));
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TIME"		,	PST_FIELD(pList, alcn_depr_time			));
			LOG_WRITE("%30s - (%s) ", "DEPR_DT"				,	PST_FIELD(pList, depr_dt				));
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME"			,	PST_FIELD(pList, depr_time				));
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD"			,	PST_FIELD(pList, bus_cls_cd				));
			LOG_WRITE("%30s - (%s) ", "CACM_CD"				,	PST_FIELD(pList, cacm_cd				));
			LOG_WRITE("%30s - (%s) ", "ALCN_DVS_CD"			,	PST_FIELD(pList, alcn_dvs_cd			));
			LOG_WRITE("%30s - (%s) ", "BCNL_DVS_CD"			,	PST_FIELD(pList, bcnl_dvs_cd			));
			
			LOG_WRITE("%30s - (%d) ", "RMN_SATS_NUM"		,	*(int *)pList->n_rmn_sats_num				);
			LOG_WRITE("%30s - (%d) ", "NSAT_NUM"			,	*(int *)pList->n_nsat_num					);
			LOG_WRITE("%30s - (%d) ", "TOT_SATS_NUM"		,	*(int *)pList->n_tot_sats_num				);
			LOG_WRITE("%30s - (%d) ", "TISSU_ABLE_NUM"		,	*(int *)pList->n_tissu_able_num			);
			
			LOG_WRITE("%30s - (%s) ", "CHLD_SFTY_SATS_YN"	,	PST_FIELD(pList, chld_sfty_sats_yn		));
			LOG_WRITE("%30s - (%s) ", "CACM_NM_PRIN_YN"		,	PST_FIELD(pList, cacm_nm_prin_yn		));
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_PRIN_YN"		,	PST_FIELD(pList, bus_cls_prin_yn		));
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME_PRIN_YN"	,	PST_FIELD(pList, depr_time_prin_yn		));
			LOG_WRITE("%30s - (%s) ", "SATS_NO_PRIN_YN"		,	PST_FIELD(pList, sats_no_prin_yn		));
			LOG_WRITE("%30s - (%s) ", "BUS_OPRN_DIST"		,	PST_FIELD(pList, bus_oprn_dist			));
			
			LOG_WRITE("%30s - (%d) ", "TAKE_DRTM"			,	*(int *)pList->n_take_drtm				);
			
			LOG_WRITE("%30s - (%s) ", "ROT_RDHM_NO_VAL"		,	PST_FIELD(pList, rot_rdhm_no_val		));
			LOG_WRITE("%30s - (%s) ", "CTY_PRMM_DC_YN"		,	PST_FIELD(pList, cty_prmm_dc_yn			));
			LOG_WRITE("%30s - (%s) ", "SATS_STUP_SNO"		,	PST_FIELD(pList, sats_stup_sno			));
			LOG_WRITE("%30s - (%s) ", "DSPR_SATS_YN	"		,	PST_FIELD(pList, dspr_sats_yn			));
		}
	}

	///> data proc
	{
		nRet = nRealCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_CM_ReadSatsFee
 * @details		[티머니고속] 현장발권 - 요금/좌석 상태 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CM_ReadSatsFee(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nCount1;
	char*	pService = "CM_ReadSatsFee";
	pstk_tm_readsatsfee_t pSPacket;
	prtk_tm_readsatsfee_t pRPacket;

	nRet = nTimeout = nCount = nCount1 = 0;

	LOG_OPEN();
	LOG_WRITE(" 현장발권-요금/좌석 상태 조회_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_readsatsfee_t) pData;
	pRPacket = (prtk_tm_readsatsfee_t) retBuf;

	/// [티머니고속] 요금정보 mem 초기화
	CPubTckTmExpMem::GetInstance()->m_vtResFee.clear();

	/// [티머니고속] 할인정보 mem 초기화
	CPubTckTmExpMem::GetInstance()->m_vtResDisc.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TRML_NO"	,	pSPacket->alcn_depr_trml_no		);
		LOG_WRITE("%30s - (%s) ","ALCN_ARVL_TRML_NO"	,	pSPacket->alcn_arvl_trml_no		);
		LOG_WRITE("%30s - (%s) ","ALCN_ROT_NO"			,	pSPacket->alcn_rot_no			);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO"			,	pSPacket->depr_trml_no			);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"			,	pSPacket->arvl_trml_no			);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_DT"			,	pSPacket->alcn_depr_dt			);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TIME"		,	pSPacket->alcn_depr_time			);
		LOG_WRITE("%30s - (%s) ","DEPR_DT"				,	pSPacket->depr_dt				);
		LOG_WRITE("%30s - (%s) ","DEPR_TIME"			,	pSPacket->depr_time				);
		LOG_WRITE("%30s - (%s) ","BUS_CLS_CD"			,	pSPacket->bus_cls_cd				);
		LOG_WRITE("%30s - (%s) ","CACM_CD"				,	pSPacket->cacm_cd				);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(ALCN_DEPR_TRML_NO	,	pSPacket->alcn_depr_trml_no		);
		nRet = TMaxFBPut(ALCN_ARVL_TRML_NO	,	pSPacket->alcn_arvl_trml_no		);
		nRet = TMaxFBPut(ALCN_ROT_NO		,	pSPacket->alcn_rot_no			);
		nRet = TMaxFBPut(DEPR_TRML_NO		,	pSPacket->depr_trml_no			);
		nRet = TMaxFBPut(ARVL_TRML_NO		,	pSPacket->arvl_trml_no			);
		nRet = TMaxFBPut(ALCN_DEPR_DT		,	pSPacket->alcn_depr_dt			);
		nRet = TMaxFBPut(ALCN_DEPR_TIME		,	pSPacket->alcn_depr_time		);
		nRet = TMaxFBPut(DEPR_DT			,	pSPacket->depr_dt				);
		nRet = TMaxFBPut(DEPR_TIME			,	pSPacket->depr_time				);
		nRet = TMaxFBPut(BUS_CLS_CD			,	pSPacket->bus_cls_cd			);
		nRet = TMaxFBPut(CACM_CD			,	pSPacket->cacm_cd				);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.rsp_cd, pRPacket->rsp_cd, sizeof(CPubTckTmExpMem::GetInstance()->base.rsp_cd));
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.rsp_cd, pRPacket->rsp_cd, sizeof(CPubTckTmExpMem::GetInstance()->base.rsp_cd));

		/// 총 좌석수
		nRet = TMaxFBGet(TOT_SATS_NUM		, PST_FIELD(pRPacket,	n_tot_sats_num		));
		/// 잔여 좌석수
		nRet = TMaxFBGet(RMN_SATS_NUM		, PST_FIELD(pRPacket,	n_rmn_sats_num		));
		/// 좌석 다중값
		nRet = TMaxFBGet(SATS_MLTP_VAL		, PST_FIELD(pRPacket,	sats_mltp_val		));
		/// 터미널 좌석 할당값
		nRet = TMaxFBGet(TRML_SATS_ASGT_VAL	, PST_FIELD(pRPacket,	trml_sats_asgt_val	));

		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.n_tot_sats_num, pRPacket->n_tot_sats_num, sizeof(pRPacket->n_tot_sats_num));
		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.n_rmn_sats_num, pRPacket->n_rmn_sats_num, sizeof(pRPacket->n_rmn_sats_num));
		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.sats_mltp_val, pRPacket->sats_mltp_val, sizeof(pRPacket->sats_mltp_val));
		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.trml_sats_asgt_val, pRPacket->trml_sats_asgt_val, sizeof(pRPacket->trml_sats_asgt_val));

		/// 정보 수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount > 0)
		{
			/// 요금정보 List
			pRPacket->pList = new rtk_tm_readsatsfee_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_readsatsfee_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				prtk_tm_readsatsfee_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TCK_KND_CD	,	PST_FIELD(pList, tck_knd_cd		));
				nRet = TMaxFBGetF(FEE_KND_CD	,	PST_FIELD(pList, fee_knd_cd		));
				nRet = TMaxFBGetF(FEE			,	PST_FIELD(pList, n_fee			));
				nRet = TMaxFBGetF(OGN_FEE		,	PST_FIELD(pList, n_ogn_fee		));

				/// [티머니고속] 요금정보 memory add
				CPubTckTmExpMem::GetInstance()->m_vtResFee.push_back(*pList);
			}
		}
		
		/// 할인정보 List
		nRet = TMaxFBGet(REC_NUM2	, PST_FIELD(pRPacket, rec_num2));
		::CopyMemory(&nCount1, pRPacket->rec_num2, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM2", nCount1);
		if(nCount1 > 0)
		{
			pRPacket->pDiscList = new rtk_tm_readsats_disc_list_t[nCount1];
			::ZeroMemory(pRPacket->pDiscList, sizeof(rtk_tm_readsats_disc_list_t) * nCount1);

			for(i = 0; i < nCount1; i++)
			{
				prtk_tm_readsats_disc_list_t pList;

				pList = &pRPacket->pDiscList[i];

				nRet = TMaxFBGetF(DC_KND_CD		,	PST_FIELD(pList, dc_knd_cd		));
				nRet = TMaxFBGetF(DC_MLTP_VAL	,	PST_FIELD(pList, dc_mltp_val	));

				/// [티머니고속] 할인정보 memory add
				CPubTckTmExpMem::GetInstance()->m_vtResDisc.push_back(*pList);
			}
		}

		if( (nCount + nCount1) <= 0 )
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s - (%d) ", "TOT_SATS_NUM"		,	*(int *)pRPacket->n_tot_sats_num	);
		LOG_WRITE("%30s - (%d) ", "RMN_SATS_NUM"		,	*(int *)pRPacket->n_rmn_sats_num	);
		LOG_WRITE("%30s - (%s) ", "SATS_MLTP_VAL"		,	PST_FIELD(pRPacket, sats_mltp_val	));
		LOG_WRITE("%30s - (%s) ", "TRML_SATS_ASGT_VAL"	,	PST_FIELD(pRPacket, trml_sats_asgt_val	));

		/// [티머니고속] 요금/좌석 정보
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_readsatsfee_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 티켓요금정보, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"	,	PST_FIELD(pList, tck_knd_cd		));
			LOG_WRITE("%30s - (%s) ", "FEE_KND_CD"	,	PST_FIELD(pList, fee_knd_cd		));
			LOG_WRITE("%30s - (%d) ", "FEE"			,	*(int *)pList->n_fee				);
			LOG_WRITE("%30s - (%d) ", "OGN_FEE"		,	*(int *)pList->n_ogn_fee			);
		}

		/// [티머니고속] 할인 정보
		for(i = 0; i < nCount1; i++)
		{
			prtk_tm_readsats_disc_list_t pList;

			pList = &pRPacket->pDiscList[i];

			LOG_WRITE("####### 할인정보, 인덱스 (%4d / %4d) #######", i, nCount1);
			LOG_WRITE("%30s - (%s) ", "dc_knd_cd"	,	PST_FIELD(pList, dc_knd_cd	));
			LOG_WRITE("%30s - (%s) ", "dc_mltp_val"	,	PST_FIELD(pList, dc_mltp_val	));
		}
	}

	///> data proc
	{
		nRet = nCount + nCount1;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		
 * @details		[티머니고속] 현장발권 - 좌석 선점 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_PcpySats(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nCount1;
	char*	pService = "TK_PcpySats";
	pstk_tm_pcpysats_t pSPacket;
	prtk_tm_pcpysats_t pRPacket;

	nRet = nTimeout = nCount = nCount1 = 0;

	LOG_OPEN();
	LOG_WRITE(" 현장발권-좌석선점 조회_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_pcpysats_t) pData;
	pRPacket = (prtk_tm_pcpysats_t) retBuf;

	/// [티머니고속] 좌석 선점정보 mem 초기화
	CPubTckTmExpMem::GetInstance()->m_tResSatsPcpy.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","REQ_DVS_CD"			,	pSPacket->req_dvs_cd		);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TRML_NO"	,	pSPacket->alcn_depr_trml_no	);
		LOG_WRITE("%30s - (%s) ","ALCN_ARVL_TRML_NO"	,	pSPacket->alcn_arvl_trml_no	);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO"			,	pSPacket->depr_trml_no		);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"			,	pSPacket->arvl_trml_no		);
		LOG_WRITE("%30s - (%s) ","BUS_CLS_CD"			,	pSPacket->bus_cls_cd		);
		LOG_WRITE("%30s - (%s) ","CACM_CD"				,	pSPacket->cacm_cd			);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_DT"			,	pSPacket->alcn_depr_dt		);
		LOG_WRITE("%30s - (%s) ","DEPR_DT"				,	pSPacket->depr_dt			);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TIME"		,	pSPacket->alcn_depr_time	);
		LOG_WRITE("%30s - (%s) ","DEPR_TIME"			,	pSPacket->depr_time			);
		LOG_WRITE("%30s - (Num=%d) ","REC_NUM"			,	*(int *)pSPacket->rec_num	);
		::CopyMemory(&nCount, pSPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			pstk_tm_pcpysats_list_t pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("####### 선점정보, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ","PCPY_NO"	,	pList->pcpy_no	);
			LOG_WRITE("%30s - (%s) ","SATS_NO"	,	pList->sats_no	);
		}
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(REQ_DVS_CD			,	pSPacket->req_dvs_cd			);
		nRet = TMaxFBPut(ALCN_DEPR_TRML_NO	,	pSPacket->alcn_depr_trml_no		);
		nRet = TMaxFBPut(ALCN_ARVL_TRML_NO	,	pSPacket->alcn_arvl_trml_no		);
		nRet = TMaxFBPut(DEPR_TRML_NO		,	pSPacket->depr_trml_no			);
		nRet = TMaxFBPut(ARVL_TRML_NO		,	pSPacket->arvl_trml_no			);
		nRet = TMaxFBPut(BUS_CLS_CD			,	pSPacket->bus_cls_cd			);
		nRet = TMaxFBPut(CACM_CD			,	pSPacket->cacm_cd				);
		nRet = TMaxFBPut(ALCN_DEPR_DT		,	pSPacket->alcn_depr_dt			);
		nRet = TMaxFBPut(DEPR_DT			,	pSPacket->depr_dt				);
		nRet = TMaxFBPut(ALCN_DEPR_TIME		,	pSPacket->alcn_depr_time		);
		nRet = TMaxFBPut(DEPR_TIME			,	pSPacket->depr_time				);
		nRet = TMaxFBPut(REC_NUM			,	pSPacket->rec_num				);
		::CopyMemory(&nCount, pSPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			pstk_tm_pcpysats_list_t pList;

			pList = &pSPacket->List[i];

			nRet = TMaxFBPut(PCPY_NO		,	pList->pcpy_no	);
			nRet = TMaxFBPut(SATS_NO		,	pList->sats_no	);
		}
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.rsp_cd, pRPacket->rsp_cd, sizeof(CPubTckTmExpMem::GetInstance()->base.rsp_cd));
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.rsp_cd, pRPacket->rsp_cd, sizeof(CPubTckTmExpMem::GetInstance()->base.rsp_cd));

		/// 정보 수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			/// 좌석 선점정보 List
			pRPacket->pList = new rtk_tm_pcpysats_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_pcpysats_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				prtk_tm_pcpysats_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(PCPY_NO	,	PST_FIELD(pList, pcpy_no		));
				nRet = TMaxFBGetF(SATS_NO	,	PST_FIELD(pList, sats_no		));

				/// [티머니고속] 좌석 선점정보 memory add
				CPubTckTmExpMem::GetInstance()->m_tResSatsPcpy.push_back(*pList);
			}
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		/// [티머니고속] 좌석 선점 정보
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_pcpysats_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 좌석 선점정보, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "PCPY_NO"	,	PST_FIELD(pList, pcpy_no		));
			LOG_WRITE("%30s - (%s) ", "SATS_NO"	,	PST_FIELD(pList, sats_no		));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_PcpySatsCancel
 * @details		[티머니고속] 현장발권 - 좌석 선점 해제
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_PcpySatsCancel(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nCount1;
	char*	pService = "TK_PcpySats";
	pstk_tm_pcpysats_t pSPacket;
	prtk_tm_pcpysats_t pRPacket;

	nRet = nTimeout = nCount = nCount1 = 0;

	LOG_OPEN();
	LOG_WRITE(" 현장발권-좌석선점 해제_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_pcpysats_t) pData;
	pRPacket = (prtk_tm_pcpysats_t) retBuf;

	/// [티머니고속] 좌석 선점정보 mem 초기화
	CPubTckTmExpMem::GetInstance()->m_tResSatsPcpy.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","REQ_DVS_CD"			,	pSPacket->req_dvs_cd		);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TRML_NO"	,	pSPacket->alcn_depr_trml_no	);
		LOG_WRITE("%30s - (%s) ","ALCN_ARVL_TRML_NO"	,	pSPacket->alcn_arvl_trml_no	);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO"			,	pSPacket->depr_trml_no		);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"			,	pSPacket->arvl_trml_no		);
		LOG_WRITE("%30s - (%s) ","BUS_CLS_CD"			,	pSPacket->bus_cls_cd		);
		LOG_WRITE("%30s - (%s) ","CACM_CD"				,	pSPacket->cacm_cd			);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_DT"			,	pSPacket->alcn_depr_dt		);
		LOG_WRITE("%30s - (%s) ","DEPR_DT"				,	pSPacket->depr_dt			);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TIME"		,	pSPacket->alcn_depr_time	);
		LOG_WRITE("%30s - (%s) ","DEPR_TIME"			,	pSPacket->depr_time			);
		LOG_WRITE("%30s - (Num=%d) ","REC_NUM"			,	*(int *)pSPacket->rec_num	);
		::CopyMemory(&nCount, pSPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			pstk_tm_pcpysats_list_t pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("####### 좌석 선점해제 정보, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ","PCPY_NO"	,	pList->pcpy_no	);
			LOG_WRITE("%30s - (%s) ","SATS_NO"	,	pList->sats_no	);
		}
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(REQ_DVS_CD			,	pSPacket->req_dvs_cd			);
		nRet = TMaxFBPut(ALCN_DEPR_TRML_NO	,	pSPacket->alcn_depr_trml_no		);
		nRet = TMaxFBPut(ALCN_ARVL_TRML_NO	,	pSPacket->alcn_arvl_trml_no		);
		nRet = TMaxFBPut(DEPR_TRML_NO		,	pSPacket->depr_trml_no			);
		nRet = TMaxFBPut(ARVL_TRML_NO		,	pSPacket->arvl_trml_no			);
		nRet = TMaxFBPut(BUS_CLS_CD			,	pSPacket->bus_cls_cd			);
		nRet = TMaxFBPut(CACM_CD			,	pSPacket->cacm_cd				);
		nRet = TMaxFBPut(ALCN_DEPR_DT		,	pSPacket->alcn_depr_dt			);
		nRet = TMaxFBPut(DEPR_DT			,	pSPacket->depr_dt				);
		nRet = TMaxFBPut(ALCN_DEPR_TIME		,	pSPacket->alcn_depr_time		);
		nRet = TMaxFBPut(DEPR_TIME			,	pSPacket->depr_time				);
		nRet = TMaxFBPut(REC_NUM			,	pSPacket->rec_num				);
		::CopyMemory(&nCount, pSPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			pstk_tm_pcpysats_list_t pList;

			pList = &pSPacket->List[i];

			nRet = TMaxFBPut(PCPY_NO		,	pList->pcpy_no	);
			nRet = TMaxFBPut(SATS_NO		,	pList->sats_no	);
		}
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.rsp_cd, pRPacket->rsp_cd, sizeof(CPubTckTmExpMem::GetInstance()->base.rsp_cd));

		/// 정보 수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			/// 좌석 선점해제 List
			pRPacket->pList = new rtk_tm_pcpysats_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_pcpysats_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				prtk_tm_pcpysats_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(PCPY_NO	,	PST_FIELD(pList, pcpy_no		));
				nRet = TMaxFBGetF(SATS_NO	,	PST_FIELD(pList, sats_no		));

				/// [티머니고속] 좌석 선점정보 memory add
				//CPubTckTmExpMem::GetInstance()->m_tResSatsPcpy.push_back(*pList);
			}
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		/// [티머니고속] 좌석 선점해제 정보
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_pcpysats_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 좌석 선점해제, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "PCPY_NO"	,	PST_FIELD(pList, pcpy_no		));
			LOG_WRITE("%30s - (%s) ", "SATS_NO"	,	PST_FIELD(pList, sats_no		));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_PubTckCash
 * @details		[티머니고속] 현장발권(현금)
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_PubTckCash(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nCount1;
	char*	pService = "TK_PubTckCash";
	pstk_tm_pubtckcash_t pSPacket;
	prtk_tm_pubtckcash_t pRPacket;

	nRet = nTimeout = nCount = nCount1 = 0;

	LOG_OPEN();
	LOG_WRITE(" 현장발권(현금)_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_pubtckcash_t) pData;
	pRPacket = (prtk_tm_pubtckcash_t) retBuf;

	/// 티머니고속 - zero memory
	::ZeroMemory(&CPubTckTmExpMem::GetInstance()->m_tResPubTckCash, sizeof(rtk_tm_pubtckcash_t));
	CPubTckTmExpMem::GetInstance()->m_tResPubTckCashList.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","REQ_DVS_CD"			,	pSPacket-> req_dvs_cd			);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TRML_NO"	,	pSPacket-> alcn_depr_trml_no	);
		LOG_WRITE("%30s - (%s) ","ALCN_ARVL_TRML_NO"	,	pSPacket-> alcn_arvl_trml_no	);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_DT"			,	pSPacket-> alcn_depr_dt			);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TIME"		,	pSPacket-> alcn_depr_time		);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO"			,	pSPacket-> depr_trml_no			);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"			,	pSPacket-> arvl_trml_no			);
		LOG_WRITE("%30s - (%s) ","DEPR_DT"				,	pSPacket-> depr_dt				);
		LOG_WRITE("%30s - (%s) ","DEPR_TIME"			,	pSPacket-> depr_time			);
		LOG_WRITE("%30s - (%s) ","BUS_CLS_CD"			,	pSPacket-> bus_cls_cd			);
		LOG_WRITE("%30s - (%s) ","CACM_CD"				,	pSPacket-> cacm_cd				);
		LOG_WRITE("%30s - (%s) ","MBRS_YN"				,	pSPacket-> mbrs_yn				);
		LOG_WRITE("%30s - (%s) ","MBRS_NO"				,	pSPacket-> mbrs_no				);
		LOG_WRITE("%30s - (%s) ","MRNP_DT"				,	pSPacket-> mrnp_dt				);
		LOG_WRITE("%30s - (%s) ","MRNP_TIME"			,	pSPacket-> mrnp_time			);
		LOG_WRITE("%30s - (%d) ","REC_NUM"			,	*(int *)pSPacket->rec_num			);

		::CopyMemory(&nCount, pSPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			pstk_tm_pubtckcash_list_t pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("####### 승차권 발권 정보, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ","TCK_KND_CD"	,	pList->tck_knd_cd			);
			LOG_WRITE("%30s - (%s) ","SATS_NO"		,	pList->sats_no				);
			LOG_WRITE("%30s - (%s) ","FP_NO"		,	pList->fp_no				);
			LOG_WRITE("%30s - (%s) ","PCPY_NO"		,	pList->pcpy_no				);
			LOG_WRITE("%30s - (%s) ","FEE_KND_CD"	,	pList->fee_knd_cd			);
			LOG_WRITE("%30s - (Num=%d) ","TISSU_FEE"	,	*(int *)pList->n_tissu_fee	);
			LOG_WRITE("%30s - (Num=%d) ","OGN_FEE"		,	*(int *)pList->n_ogn_fee	);
			LOG_WRITE("%30s - (%s) ","DC_KND_CD"	,	pList->dc_knd_cd			);
		}
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(REQ_DVS_CD				,	pSPacket->req_dvs_cd			);
		nRet = TMaxFBPut(ALCN_DEPR_TRML_NO		,	pSPacket->alcn_depr_trml_no		);
		nRet = TMaxFBPut(ALCN_ARVL_TRML_NO		,	pSPacket->alcn_arvl_trml_no		);
		nRet = TMaxFBPut(ALCN_DEPR_DT			,	pSPacket->alcn_depr_dt			);
		nRet = TMaxFBPut(ALCN_DEPR_TIME			,	pSPacket->alcn_depr_time		);
		nRet = TMaxFBPut(DEPR_TRML_NO			,	pSPacket->depr_trml_no			);
		nRet = TMaxFBPut(ARVL_TRML_NO			,	pSPacket->arvl_trml_no			);
		nRet = TMaxFBPut(DEPR_DT				,	pSPacket->depr_dt				);
		nRet = TMaxFBPut(DEPR_TIME				,	pSPacket->depr_time				);
		nRet = TMaxFBPut(BUS_CLS_CD				,	pSPacket->bus_cls_cd			);
		nRet = TMaxFBPut(CACM_CD				,	pSPacket->cacm_cd				);
		nRet = TMaxFBPut(MBRS_YN				,	pSPacket->mbrs_yn				);
		nRet = TMaxFBPut(MBRS_NO				,	pSPacket->mbrs_no				);
		nRet = TMaxFBPut(MRNP_DT				,	pSPacket->mrnp_dt				);
		nRet = TMaxFBPut(MRNP_TIME				,	pSPacket->mrnp_time				);
		nRet = TMaxFBPut(REC_NUM				,	pSPacket->rec_num				);

		::CopyMemory(&nCount, pSPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			pstk_tm_pubtckcash_list_t pList;

			pList = &pSPacket->List[i];

			nRet = TMaxFBInsert(TCK_KND_CD	,	pList->tck_knd_cd	, i, sizeof(pList->tck_knd_cd	)	);
			nRet = TMaxFBInsert(SATS_NO		,	pList->sats_no		, i, sizeof(pList->sats_no		)	);
			nRet = TMaxFBInsert(FP_NO		,	pList->fp_no		, i, sizeof(pList->fp_no		)	);
			nRet = TMaxFBInsert(PCPY_NO		,	pList->pcpy_no		, i, sizeof(pList->pcpy_no		)	);
			nRet = TMaxFBInsert(FEE_KND_CD	,	pList->fee_knd_cd	, i, sizeof(pList->fee_knd_cd	)	);
			nRet = TMaxFBInsert(TISSU_FEE	,	pList->n_tissu_fee	, i, sizeof(pList->n_tissu_fee	)	);
			nRet = TMaxFBInsert(OGN_FEE		,	pList->n_ogn_fee	, i, sizeof(pList->n_ogn_fee	)	);
			nRet = TMaxFBInsert(DC_KND_CD	,	pList->dc_knd_cd	, i, sizeof(pList->dc_knd_cd	)	);
		}
	}

	m_nTPCallFlag = TPNOFLAGS;
	TMaxTransaction(TMAX_TR_BEGIN);

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.rsp_cd, pRPacket->rsp_cd, sizeof(CPubTckTmExpMem::GetInstance()->base.rsp_cd));
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			TMaxTransaction(TMAX_TR_ROLLBACK);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		TMaxTransaction(TMAX_TR_COMMIT);

		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.rsp_cd, pRPacket->rsp_cd, sizeof(CPubTckTmExpMem::GetInstance()->base.rsp_cd));

		// get data
		{
			nRet = TMaxFBGet(CACM_NM_PRIN_YN		, PST_FIELD(pRPacket,	cacm_nm_prin_yn		));
			nRet = TMaxFBGet(BUS_CLS_PRIN_YN		, PST_FIELD(pRPacket,	bus_cls_prin_yn		));
			nRet = TMaxFBGet(DEPR_TIME_PRIN_YN		, PST_FIELD(pRPacket,	depr_time_prin_yn	));
			nRet = TMaxFBGet(SATS_NO_PRIN_YN		, PST_FIELD(pRPacket,	sats_no_prin_yn		));
			nRet = TMaxFBGet(TISSU_DT				, PST_FIELD(pRPacket,	tissu_dt			));
			nRet = TMaxFBGet(TISSU_TRML_NO			, PST_FIELD(pRPacket,	tissu_trml_no		));
			nRet = TMaxFBGet(TISSU_WND_NO			, PST_FIELD(pRPacket,	tissu_wnd_no		));
			nRet = TMaxFBGet(TISSU_TIME				, PST_FIELD(pRPacket,	tissu_time			));
			nRet = TMaxFBGet(USER_KEY_VAL			, PST_FIELD(pRPacket,	user_key_val		));

			/// set memory
			::ZeroMemory(&CPubTckTmExpMem::GetInstance()->m_tResPubTckCash, sizeof(rtk_tm_pubtckcash_t));
		}

		/// 정보 수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// 티머니고속 - set memory
		::CopyMemory(&CPubTckTmExpMem::GetInstance()->m_tResPubTckCash, pRPacket, sizeof(rtk_tm_pubtckcash_t));
		CPubTckTmExpMem::GetInstance()->m_tResPubTckCashList.clear();

		/// memory allocation
		{
			/// 승차권 발권(현금) List
			pRPacket->pList = new rtk_tm_pubtckcash_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_pubtckcash_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				prtk_tm_pubtckcash_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TISSU_FEE			,	PST_FIELD(pList, n_tissu_fee		));
				nRet = TMaxFBGetF(OGN_FEE			,	PST_FIELD(pList, n_ogn_fee			));
				nRet = TMaxFBGetF(SATS_NO			,	PST_FIELD(pList, sats_no			));
				nRet = TMaxFBGetF(INHR_NO			,	PST_FIELD(pList, inhr_no			));
				nRet = TMaxFBGetF(INVS_NO			,	PST_FIELD(pList, invs_no			));
				nRet = TMaxFBGetF(TISSU_SNO			,	PST_FIELD(pList, tissu_sno			));
				nRet = TMaxFBGetF(TISSU_NO			,	PST_FIELD(pList, tissu_no			));
				nRet = TMaxFBGetF(CSRC_APRV_NO		,	PST_FIELD(pList, csrc_aprv_no		));
				nRet = TMaxFBGetF(CSRC_RGT_NO		,	PST_FIELD(pList, csrc_rgt_no		));
				nRet = TMaxFBGetF(TCK_KND_CD		,	PST_FIELD(pList, tck_knd_cd			));
				nRet = TMaxFBGetF(DC_KND_CD			,	PST_FIELD(pList, dc_knd_cd			));
				nRet = TMaxFBGetF(EXCH_KND_CD		,	PST_FIELD(pList, exch_knd_cd		));

				/// [티머니고속] 승차권 발권(현금) 정보 set memory
				CPubTckTmExpMem::GetInstance()->m_tResPubTckCashList.push_back(*pList);
			}
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{

		LOG_WRITE("%30s - (%s) ", "CACM_NM_PRIN_YN"		,	PST_FIELD(pRPacket, cacm_nm_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_PRIN_YN"		,	PST_FIELD(pRPacket, bus_cls_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME_PRIN_YN"	,	PST_FIELD(pRPacket, depr_time_prin_yn	));
		LOG_WRITE("%30s - (%s) ", "SATS_NO_PRIN_YN"		,	PST_FIELD(pRPacket, sats_no_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "TISSU_DT"			,	PST_FIELD(pRPacket, tissu_dt			));
		LOG_WRITE("%30s - (%s) ", "TISSU_TRML_NO"		,	PST_FIELD(pRPacket, tissu_trml_no		));
		LOG_WRITE("%30s - (%s) ", "TISSU_WND_NO"		,	PST_FIELD(pRPacket, tissu_wnd_no		));
		LOG_WRITE("%30s - (%s) ", "TISSU_TIME"			,	PST_FIELD(pRPacket, tissu_time			));
		LOG_WRITE("%30s - (%s) ", "USER_KEY_VAL"		,	PST_FIELD(pRPacket, user_key_val		));

		/// [티머니고속] 
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_pubtckcash_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 승차권 발권(현금), 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TISSU_FEE"	,	PST_FIELD(pList, n_tissu_fee	));
			LOG_WRITE("%30s - (%s) ", "OGN_FEE"		,	PST_FIELD(pList, n_ogn_fee		));
			LOG_WRITE("%30s - (%s) ", "SATS_NO"		,	PST_FIELD(pList, sats_no		));
			LOG_WRITE("%30s - (%s) ", "INHR_NO"		,	PST_FIELD(pList, inhr_no		));
			LOG_WRITE("%30s - (%s) ", "INVS_NO"		,	PST_FIELD(pList, invs_no		));
			LOG_WRITE("%30s - (%s) ", "TISSU_SNO"	,	PST_FIELD(pList, tissu_sno		));
			LOG_WRITE("%30s - (%s) ", "TISSU_NO"	,	PST_FIELD(pList, tissu_no		));
			LOG_WRITE("%30s - (%s) ", "CSRC_APRV_NO",	PST_FIELD(pList, csrc_aprv_no	));
			LOG_WRITE("%30s - (%s) ", "CSRC_RGT_NO"	,	PST_FIELD(pList, csrc_rgt_no	));
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"	,	PST_FIELD(pList, tck_knd_cd		));
			LOG_WRITE("%30s - (%s) ", "DC_KND_CD"	,	PST_FIELD(pList, dc_knd_cd		));
			LOG_WRITE("%30s - (%s) ", "EXCH_KND_CD"	,	PST_FIELD(pList, exch_knd_cd	));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_PubTckCard
 * @details		[티머니고속] 현장발권(카드)
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_PubTckCard(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nCount1;
	char*	pService = "TK_PubTckCard";
	pstk_tm_pubtckcard_t pSPacket;
	prtk_tm_pubtckcard_t pRPacket;

	nRet = nTimeout = nCount = nCount1 = 0;

	LOG_OPEN();
	LOG_WRITE(" 현장발권(카드)_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_pubtckcard_t) pData;
	pRPacket = (prtk_tm_pubtckcard_t) retBuf;

	/// 티머니고속 - zero memory
	::ZeroMemory(&CPubTckTmExpMem::GetInstance()->m_tResPubTckCard, sizeof(rtk_tm_pubtckcard_t));
	CPubTckTmExpMem::GetInstance()->m_tResPubTckCardList.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TRML_NO",	pSPacket->alcn_depr_trml_no	);
		LOG_WRITE("%30s - (%s) ","ALCN_ARVL_TRML_NO",	pSPacket->alcn_arvl_trml_no	);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_DT"		,	pSPacket->alcn_depr_dt		);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TIME"	,	pSPacket->alcn_depr_time	);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO"		,	pSPacket->depr_trml_no		);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"		,	pSPacket->arvl_trml_no		);
		LOG_WRITE("%30s - (%s) ","DEPR_DT"			,	pSPacket->depr_dt			);
		LOG_WRITE("%30s - (%s) ","DEPR_TIME"		,	pSPacket->depr_time			);
		LOG_WRITE("%30s - (%s) ","BUS_CLS_CD"		,	pSPacket->bus_cls_cd		);
		LOG_WRITE("%30s - (%s) ","CACM_CD"			,	pSPacket->cacm_cd			);
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ","CARD_TRACK_DTA"	,	pSPacket->card_track_dta	);
		LOG_WRITE("%30s - (%s) ","SPAD_DTA"			,	pSPacket->spad_dta			);
#endif
		LOG_WRITE("%30s - (%s) ","N_SPAD_DTA_LEN"	,	pSPacket->n_spad_dta_len	);
		LOG_WRITE("%30s - (%s) ","REQ_DVS_CD"		,	pSPacket->req_dvs_cd		);
		LOG_WRITE("%30s - (%s) ","RFID_CARD_DVS"	,	pSPacket->rfid_card_dvs		);
		LOG_WRITE("%30s - (%s) ","RFID_DONGLE_DTA"	,	pSPacket->rfid_dongle_dta	);
		LOG_WRITE("%30s - (%s) ","MBRS_YN"			,	pSPacket->mbrs_yn			);
		LOG_WRITE("%30s - (%s) ","MBRS_NO"			,	pSPacket->mbrs_no			);
		LOG_WRITE("%30s - (%s) ","MRNP_DT"			,	pSPacket->mrnp_dt			);
		LOG_WRITE("%30s - (%s) ","MRNP_TIME"		,	pSPacket->mrnp_time			);
		LOG_WRITE("%30s - (%s) ","N_MIP_MM_NUM"		,	pSPacket->n_mip_mm_num		);
		LOG_WRITE("%30s - (%s) ","TRD_DVS_CD"		,	pSPacket->trd_dvs_cd		);

#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ","ENC_DTA"			,	pSPacket->enc_dta			);
		LOG_WRITE("%30s - (%s) ","EMV_DTA"			,	pSPacket->emv_dta			);
#endif
		LOG_WRITE("%30s - (%d) ","REC_NUM"			,	*(int *)pSPacket->rec_num	);

		::CopyMemory(&nCount, pSPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			pstk_tm_pubtckcard_list_t pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("####### 승차권 발권(카드) 정보, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ","TCK_KND_CD"	,	pList->tck_knd_cd			);
			LOG_WRITE("%30s - (%s) ","SATS_NO"		,	pList->sats_no				);
			LOG_WRITE("%30s - (%s) ","PCPY_NO"		,	pList->pcpy_no				);
			LOG_WRITE("%30s - (%s) ","FEE_KND_CD"	,	pList->fee_knd_cd			);
			LOG_WRITE("%30s - (%d) ","TISSU_FEE"	,	*(int *)pList->n_tissu_fee	);
			LOG_WRITE("%30s - (%d) ","OGN_FEE"		,	*(int *)pList->n_ogn_fee	);
			LOG_WRITE("%30s - (%s) ","DC_KND_CD"	,	pList->dc_knd_cd			);
		}
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(ALCN_DEPR_TRML_NO	,	pSPacket->alcn_depr_trml_no	);
		nRet = TMaxFBPut(ALCN_ARVL_TRML_NO	,	pSPacket->alcn_arvl_trml_no	);
		nRet = TMaxFBPut(ALCN_DEPR_DT		,	pSPacket->alcn_depr_dt		);
		nRet = TMaxFBPut(ALCN_DEPR_TIME		,	pSPacket->alcn_depr_time	);
		nRet = TMaxFBPut(DEPR_TRML_NO		,	pSPacket->depr_trml_no		);
		nRet = TMaxFBPut(ARVL_TRML_NO		,	pSPacket->arvl_trml_no		);
		nRet = TMaxFBPut(DEPR_DT			,	pSPacket->depr_dt			);
		nRet = TMaxFBPut(DEPR_TIME			,	pSPacket->depr_time			);
		nRet = TMaxFBPut(BUS_CLS_CD			,	pSPacket->bus_cls_cd		);
		nRet = TMaxFBPut(CACM_CD			,	pSPacket->cacm_cd			);
		nRet = TMaxFBPut(CARD_TRACK_DTA		,	pSPacket->card_track_dta	);
		nRet = TMaxFBPut(SPAD_DTA			,	pSPacket->spad_dta			);
		nRet = TMaxFBPut(SPAD_DTA_LEN		,	pSPacket->n_spad_dta_len	);
		nRet = TMaxFBPut(REQ_DVS_CD			,	pSPacket->req_dvs_cd		);
		nRet = TMaxFBPut(RFID_CARD_DVS		,	pSPacket->rfid_card_dvs		);
		nRet = TMaxFBPut(RFID_DONGLE_DTA	,	pSPacket->rfid_dongle_dta	);
		nRet = TMaxFBPut(MBRS_YN			,	pSPacket->mbrs_yn			);
		nRet = TMaxFBPut(MBRS_NO			,	pSPacket->mbrs_no			);
		nRet = TMaxFBPut(MRNP_DT			,	pSPacket->mrnp_dt			);
		nRet = TMaxFBPut(MRNP_TIME			,	pSPacket->mrnp_time			);
		nRet = TMaxFBPut(MIP_MM_NUM			,	pSPacket->n_mip_mm_num		);
		nRet = TMaxFBPut(TRD_DVS_CD			,	pSPacket->trd_dvs_cd		);
		nRet = TMaxFBPut(ENC_DTA			,	pSPacket->enc_dta			);
		nRet = TMaxFBPut(EMV_DTA			,	pSPacket->emv_dta			);
		nRet = TMaxFBPut(REC_NUM			,	pSPacket->rec_num			);

		::CopyMemory(&nCount, pSPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			pstk_tm_pubtckcard_list_t pList;

			pList = &pSPacket->List[i];

			nRet = TMaxFBInsert(TCK_KND_CD	,	pList->tck_knd_cd	, i, sizeof(pList->tck_knd_cd	)	);
			nRet = TMaxFBInsert(SATS_NO		,	pList->sats_no		, i, sizeof(pList->sats_no		)	);
			nRet = TMaxFBInsert(PCPY_NO		,	pList->pcpy_no		, i, sizeof(pList->pcpy_no		)	);
			nRet = TMaxFBInsert(FEE_KND_CD	,	pList->fee_knd_cd	, i, sizeof(pList->fee_knd_cd	)	);
			nRet = TMaxFBInsert(TISSU_FEE	,	pList->n_tissu_fee	, i, sizeof(pList->n_tissu_fee	)	);
			nRet = TMaxFBInsert(OGN_FEE		,	pList->n_ogn_fee	, i, sizeof(pList->n_ogn_fee	)	);
			nRet = TMaxFBInsert(DC_KND_CD	,	pList->dc_knd_cd	, i, sizeof(pList->dc_knd_cd	)	);
		}

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(pSPacket->card_track_dta	, sizeof(pSPacket->card_track_dta));
		KTC_MemClear(pSPacket->spad_dta			, sizeof(pSPacket->spad_dta));
		KTC_MemClear(pSPacket->enc_dta			, sizeof(pSPacket->enc_dta));
		KTC_MemClear(pSPacket->emv_dta			, sizeof(pSPacket->emv_dta));
#endif
	}

	m_nTPCallFlag = TPNOFLAGS;
	TMaxTransaction(TMAX_TR_BEGIN);

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			TMaxTransaction(TMAX_TR_ROLLBACK);

			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		TMaxTransaction(TMAX_TR_COMMIT);

		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.rsp_cd, pRPacket->rsp_cd, sizeof(CPubTckTmExpMem::GetInstance()->base.rsp_cd));

		// get data
		{
			nRet = TMaxFBGet(CARD_NO			, PST_FIELD(pRPacket, card_no			 ));
			nRet = TMaxFBGet(CACM_NM_PRIN_YN	, PST_FIELD(pRPacket, cacm_nm_prin_yn	 ));
			nRet = TMaxFBGet(BUS_CLS_PRIN_YN	, PST_FIELD(pRPacket, bus_cls_prin_yn	 ));
			nRet = TMaxFBGet(DEPR_TIME_PRIN_YN	, PST_FIELD(pRPacket, depr_time_prin_yn	 ));
			nRet = TMaxFBGet(SATS_NO_PRIN_YN	, PST_FIELD(pRPacket, sats_no_prin_yn	 ));
			nRet = TMaxFBGet(TISSU_DT			, PST_FIELD(pRPacket, tissu_dt			 ));
			nRet = TMaxFBGet(TISSU_TRML_NO		, PST_FIELD(pRPacket, tissu_trml_no		 ));
			nRet = TMaxFBGet(TISSU_WND_NO		, PST_FIELD(pRPacket, tissu_wnd_no		 ));
			nRet = TMaxFBGet(TISSU_TIME			, PST_FIELD(pRPacket, tissu_time		 ));
			nRet = TMaxFBGet(TISSU_NO			, PST_FIELD(pRPacket, tissu_no			 ));
			nRet = TMaxFBGet(CARD_APRV_NO		, PST_FIELD(pRPacket, card_aprv_no		 ));
			nRet = TMaxFBGet(APRV_AMT			, PST_FIELD(pRPacket, n_aprv_amt		 ));
			nRet = TMaxFBGet(FRC_CMRT			, PST_FIELD(pRPacket, frc_cmrt			 ));
			nRet = TMaxFBGet(ABLE_POINT			, PST_FIELD(pRPacket, able_point		 ));
		}

		/// 정보 수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
#if (_KTC_CERTIFY_ > 0)
			KTC_MemClear(pRPacket->card_no, sizeof(pRPacket->card_no));
#endif

			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// 티머니고속 - set memory
		::CopyMemory(&CPubTckTmExpMem::GetInstance()->m_tResPubTckCard, pRPacket, sizeof(rtk_tm_pubtckcard_t));

		/// memory allocation
		{
			/// 승차권 발권(현금) List
			pRPacket->pList = new rtk_tm_pubtckcard_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_pubtckcard_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				prtk_tm_pubtckcard_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TISSU_FEE		,	PST_FIELD(pList, n_tissu_fee	));
				nRet = TMaxFBGetF(OGN_FEE		,	PST_FIELD(pList, n_ogn_fee		));
				nRet = TMaxFBGetF(SATS_NO		,	PST_FIELD(pList, sats_no		));
				nRet = TMaxFBGetF(INHR_NO		,	PST_FIELD(pList, inhr_no		));
				nRet = TMaxFBGetF(INVS_NO		,	PST_FIELD(pList, invs_no		));
				nRet = TMaxFBGetF(TISSU_SNO		,	PST_FIELD(pList, tissu_sno		));
				nRet = TMaxFBGetF(TCK_KND_CD	,	PST_FIELD(pList, tck_knd_cd		));
				nRet = TMaxFBGetF(DC_KND_CD		,	PST_FIELD(pList, dc_knd_cd		));
				nRet = TMaxFBGetF(EXCH_KND_CD	,	PST_FIELD(pList, exch_knd_cd	));

				/// [티머니고속] 승차권 발권(카드) 정보 set memory
				CPubTckTmExpMem::GetInstance()->m_tResPubTckCardList.push_back(*pList);
			}
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
#if (_KTC_CERTIFY_ > 0)
		LOG_WRITE("%30s - (%s) ", "card_no"			,	PST_FIELD(pRPacket, card_no				));
#endif
		LOG_WRITE("%30s - (%s) ", "cacm_nm_prin_yn"	,	PST_FIELD(pRPacket, cacm_nm_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "bus_cls_prin_yn"	,	PST_FIELD(pRPacket, bus_cls_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "depr_time_prin_yn",	PST_FIELD(pRPacket, depr_time_prin_yn	));
		LOG_WRITE("%30s - (%s) ", "sats_no_prin_yn"	,	PST_FIELD(pRPacket, sats_no_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "tissu_dt"		,	PST_FIELD(pRPacket, tissu_dt			));
		LOG_WRITE("%30s - (%s) ", "tissu_trml_no"	,	PST_FIELD(pRPacket, tissu_trml_no		));
		LOG_WRITE("%30s - (%s) ", "tissu_wnd_no"	,	PST_FIELD(pRPacket, tissu_wnd_no		));
		LOG_WRITE("%30s - (%s) ", "tissu_time"		,	PST_FIELD(pRPacket, tissu_time			));
		LOG_WRITE("%30s - (%s) ", "tissu_no"		,	PST_FIELD(pRPacket, tissu_no			));
		LOG_WRITE("%30s - (%s) ", "card_aprv_no"		,	PST_FIELD(pRPacket, card_aprv_no	));
		LOG_WRITE("%30s - (%s) ", "n_aprv_amt"		,	PST_FIELD(pRPacket, n_aprv_amt			));
		LOG_WRITE("%30s - (%s) ", "frc_cmrt"		,	PST_FIELD(pRPacket, frc_cmrt			));
		LOG_WRITE("%30s - (%s) ", "able_point"		,	PST_FIELD(pRPacket, able_point			));

		/// [티머니고속] 
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_pubtckcard_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 승차권 발권(카드), 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TISSU_FEE"	,	PST_FIELD(pList, n_tissu_fee	));
			LOG_WRITE("%30s - (%s) ", "OGN_FEE"		,	PST_FIELD(pList, n_ogn_fee		));
			LOG_WRITE("%30s - (%s) ", "SATS_NO"		,	PST_FIELD(pList, sats_no		));
			LOG_WRITE("%30s - (%s) ", "INHR_NO"		,	PST_FIELD(pList, inhr_no		));
			LOG_WRITE("%30s - (%s) ", "INVS_NO"		,	PST_FIELD(pList, invs_no		));
			LOG_WRITE("%30s - (%s) ", "TISSU_SNO"	,	PST_FIELD(pList, tissu_sno		));
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"	,	PST_FIELD(pList, tck_knd_cd		));
			LOG_WRITE("%30s - (%s) ", "DC_KND_CD"	,	PST_FIELD(pList, dc_knd_cd		));
			LOG_WRITE("%30s - (%s) ", "EXCH_KND_CD"	,	PST_FIELD(pList, exch_knd_cd	));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_PubTckCard_KTC
 * @details		[티머니고속] 현장발권(카드_KTC)
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_PubTckCard_KTC(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nCount1;
	char*	pService = "TK_KtcPubCard";
	pstk_tm_pubtckcard_ktc_t pSPacket;
	prtk_tm_pubtckcard_ktc_t pRPacket;

	nRet = nTimeout = nCount = nCount1 = 0;

	LOG_OPEN();
	LOG_WRITE(" 현장발권(카드_KTC)_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_pubtckcard_ktc_t) pData;
	pRPacket = (prtk_tm_pubtckcard_ktc_t) retBuf;

	/// 티머니고속 - zero memory
	::ZeroMemory(&CPubTckTmExpMem::GetInstance()->m_tResPubTckCardKtc, sizeof(rtk_tm_pubtckcard_ktc_t));
	CPubTckTmExpMem::GetInstance()->m_tResPubTckCardKtcList.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TRML_NO",	pSPacket->alcn_depr_trml_no	);
		LOG_WRITE("%30s - (%s) ","ALCN_ARVL_TRML_NO",	pSPacket->alcn_arvl_trml_no	);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_DT"		,	pSPacket->alcn_depr_dt		);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TIME"	,	pSPacket->alcn_depr_time	);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO"		,	pSPacket->depr_trml_no		);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"		,	pSPacket->arvl_trml_no		);
		LOG_WRITE("%30s - (%s) ","DEPR_DT"			,	pSPacket->depr_dt			);
		LOG_WRITE("%30s - (%s) ","DEPR_TIME"		,	pSPacket->depr_time			);
		LOG_WRITE("%30s - (%s) ","BUS_CLS_CD"		,	pSPacket->bus_cls_cd		);
		LOG_WRITE("%30s - (%s) ","CACM_CD"			,	pSPacket->cacm_cd			);
		LOG_WRITE("%30s - (%s) ","CARD_TRACK_DTA"	,	pSPacket->card_track_dta	);
		LOG_WRITE("%30s - (%s) ","SPAD_DTA"			,	pSPacket->spad_dta			);
		LOG_WRITE("%30s - (%d) ","SPAD_DTA_LEN"		,	*(int *)pSPacket->n_spad_dta_len	);
		LOG_WRITE("%30s - (%s) ","REQ_DVS_CD"		,	pSPacket->req_dvs_cd		);
		LOG_WRITE("%30s - (%s) ","RFID_CARD_DVS"	,	pSPacket->rfid_card_dvs		);
		LOG_WRITE("%30s - (%s) ","RFID_DONGLE_DTA"	,	pSPacket->rfid_dongle_dta	);
		LOG_WRITE("%30s - (%s) ","MBRS_YN"			,	pSPacket->mbrs_yn			);
		LOG_WRITE("%30s - (%s) ","MBRS_NO"			,	pSPacket->mbrs_no			);
		LOG_WRITE("%30s - (%s) ","MRNP_DT"			,	pSPacket->mrnp_dt			);
		LOG_WRITE("%30s - (%s) ","MRNP_TIME"		,	pSPacket->mrnp_time			);
		LOG_WRITE("%30s - (%d) ","MIP_MM_NUM"		,	*(int *)pSPacket->n_mip_mm_num		);
		LOG_WRITE("%30s - (%s) ","TRD_DVS_CD"		,	pSPacket->trd_dvs_cd		);

#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ","ENC_DTA"			,	pSPacket->enc_dta			);
		LOG_WRITE("%30s - (%s) ","EMV_DTA"			,	pSPacket->emv_dta			);
#endif
		LOG_WRITE("%30s - (%d) ","REC_NUM"			,	*(int *)pSPacket->rec_num	);

		::CopyMemory(&nCount, pSPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			pstk_tm_pubtckcard_ktc_list_t pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("####### 승차권 발권(카드_KTC) 정보, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ","TCK_KND_CD"	,	pList->tck_knd_cd			);
			LOG_WRITE("%30s - (%s) ","SATS_NO"		,	pList->sats_no				);
			LOG_WRITE("%30s - (%s) ","PCPY_NO"		,	pList->pcpy_no				);
			LOG_WRITE("%30s - (%s) ","FEE_KND_CD"	,	pList->fee_knd_cd			);
			LOG_WRITE("%30s - (%d) ","TISSU_FEE"	,	*(int *)pList->n_tissu_fee	);
			LOG_WRITE("%30s - (%d) ","OGN_FEE"		,	*(int *)pList->n_ogn_fee	);
			LOG_WRITE("%30s - (%s) ","DC_KND_CD"	,	pList->dc_knd_cd			);
		}
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(ALCN_DEPR_TRML_NO	,	pSPacket->alcn_depr_trml_no	);
		nRet = TMaxFBPut(ALCN_ARVL_TRML_NO	,	pSPacket->alcn_arvl_trml_no	);
		nRet = TMaxFBPut(ALCN_DEPR_DT		,	pSPacket->alcn_depr_dt		);
		nRet = TMaxFBPut(ALCN_DEPR_TIME		,	pSPacket->alcn_depr_time	);
		nRet = TMaxFBPut(DEPR_TRML_NO		,	pSPacket->depr_trml_no		);
		nRet = TMaxFBPut(ARVL_TRML_NO		,	pSPacket->arvl_trml_no		);
		nRet = TMaxFBPut(DEPR_DT			,	pSPacket->depr_dt			);
		nRet = TMaxFBPut(DEPR_TIME			,	pSPacket->depr_time			);
		nRet = TMaxFBPut(BUS_CLS_CD			,	pSPacket->bus_cls_cd		);
		nRet = TMaxFBPut(CACM_CD			,	pSPacket->cacm_cd			);
		nRet = TMaxFBPut(CARD_TRACK_DTA		,	pSPacket->card_track_dta	);
		nRet = TMaxFBPut(SPAD_DTA			,	pSPacket->spad_dta			);
		nRet = TMaxFBPut(SPAD_DTA_LEN		,	pSPacket->n_spad_dta_len	);
		nRet = TMaxFBPut(REQ_DVS_CD			,	pSPacket->req_dvs_cd		);
		nRet = TMaxFBPut(RFID_CARD_DVS		,	pSPacket->rfid_card_dvs		);
		nRet = TMaxFBPut(RFID_DONGLE_DTA	,	pSPacket->rfid_dongle_dta	);
		nRet = TMaxFBPut(MBRS_YN			,	pSPacket->mbrs_yn			);
		nRet = TMaxFBPut(MBRS_NO			,	pSPacket->mbrs_no			);
		nRet = TMaxFBPut(MRNP_DT			,	pSPacket->mrnp_dt			);
		nRet = TMaxFBPut(MRNP_TIME			,	pSPacket->mrnp_time			);
		nRet = TMaxFBPut(MIP_MM_NUM			,	pSPacket->n_mip_mm_num		);
		nRet = TMaxFBPut(TRD_DVS_CD			,	pSPacket->trd_dvs_cd		);
		nRet = TMaxFBPut(ENC_DTA			,	pSPacket->enc_dta			);
		nRet = TMaxFBPut(EMV_DTA			,	pSPacket->emv_dta			);
		nRet = TMaxFBPut(REC_NUM			,	pSPacket->rec_num			);

		::CopyMemory(&nCount, pSPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			pstk_tm_pubtckcard_ktc_list_t pList;

			pList = &pSPacket->List[i];

			nRet = TMaxFBInsert(TCK_KND_CD	,	pList->tck_knd_cd	, i, sizeof(pList->tck_knd_cd	)	);
			nRet = TMaxFBInsert(SATS_NO		,	pList->sats_no		, i, sizeof(pList->sats_no		)	);
			nRet = TMaxFBInsert(PCPY_NO		,	pList->pcpy_no		, i, sizeof(pList->pcpy_no		)	);
			nRet = TMaxFBInsert(FEE_KND_CD	,	pList->fee_knd_cd	, i, sizeof(pList->fee_knd_cd	)	);
			nRet = TMaxFBInsert(TISSU_FEE	,	pList->n_tissu_fee	, i, sizeof(pList->n_tissu_fee	)	);
			nRet = TMaxFBInsert(OGN_FEE		,	pList->n_ogn_fee	, i, sizeof(pList->n_ogn_fee	)	);
			nRet = TMaxFBInsert(DC_KND_CD	,	pList->dc_knd_cd	, i, sizeof(pList->dc_knd_cd	)	);
		}
	}

	m_nTPCallFlag = TPNOFLAGS;
	TMaxTransaction(TMAX_TR_BEGIN);

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.rsp_cd, pRPacket->rsp_cd, sizeof(CPubTckTmExpMem::GetInstance()->base.rsp_cd));
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			TMaxTransaction(TMAX_TR_ROLLBACK);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
		else if( pRPacket->rsp_cd[1] == 'W' )
		{	/// 신용카드에서 실패면 현금으로 처리...

		}

		TMaxTransaction(TMAX_TR_COMMIT);

		// get data
		{
			nRet = TMaxFBGet(CARD_NO			, PST_FIELD(pRPacket, card_no			 ));
			nRet = TMaxFBGet(CACM_NM_PRIN_YN	, PST_FIELD(pRPacket, cacm_nm_prin_yn	 ));
			nRet = TMaxFBGet(BUS_CLS_PRIN_YN	, PST_FIELD(pRPacket, bus_cls_prin_yn	 ));
			nRet = TMaxFBGet(DEPR_TIME_PRIN_YN	, PST_FIELD(pRPacket, depr_time_prin_yn	 ));
			nRet = TMaxFBGet(SATS_NO_PRIN_YN	, PST_FIELD(pRPacket, sats_no_prin_yn	 ));
			nRet = TMaxFBGet(TISSU_DT			, PST_FIELD(pRPacket, tissu_dt			 ));
			nRet = TMaxFBGet(TISSU_TRML_NO		, PST_FIELD(pRPacket, tissu_trml_no		 ));
			nRet = TMaxFBGet(TISSU_WND_NO		, PST_FIELD(pRPacket, tissu_wnd_no		 ));
			nRet = TMaxFBGet(TISSU_TIME			, PST_FIELD(pRPacket, tissu_time		 ));
			nRet = TMaxFBGet(TISSU_NO			, PST_FIELD(pRPacket, tissu_no			 ));
			nRet = TMaxFBGet(CARD_APRV_NO		, PST_FIELD(pRPacket, card_aprv_no		 ));
			nRet = TMaxFBGet(APRV_AMT			, PST_FIELD(pRPacket, n_aprv_amt		 ));
			nRet = TMaxFBGet(FRC_CMRT			, PST_FIELD(pRPacket, frc_cmrt			 ));
			nRet = TMaxFBGet(ABLE_POINT			, PST_FIELD(pRPacket, able_point		 ));

			/// 승차권발권(카드_KTC) 정보 : set memory
			::CopyMemory(&CPubTckTmExpMem::GetInstance()->m_tResPubTckCardKtc, pRPacket, sizeof(rtk_tm_pubtckcard_ktc_t));
		}

		/// 정보 수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			/// 승차권발권(카드_KTC) List
			pRPacket->pList = new rtk_tm_pubtckcard_ktc_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_pubtckcard_ktc_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				prtk_tm_pubtckcard_ktc_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TISSU_FEE		,	PST_FIELD(pList, n_tissu_fee	));
				nRet = TMaxFBGetF(OGN_FEE		,	PST_FIELD(pList, n_ogn_fee		));
				nRet = TMaxFBGetF(SATS_NO		,	PST_FIELD(pList, sats_no		));
				nRet = TMaxFBGetF(INHR_NO		,	PST_FIELD(pList, inhr_no		));
				nRet = TMaxFBGetF(INVS_NO		,	PST_FIELD(pList, invs_no		));
				nRet = TMaxFBGetF(TISSU_SNO		,	PST_FIELD(pList, tissu_sno		));
				nRet = TMaxFBGetF(TCK_KND_CD	,	PST_FIELD(pList, tck_knd_cd		));
				nRet = TMaxFBGetF(DC_KND_CD		,	PST_FIELD(pList, dc_knd_cd		));
				nRet = TMaxFBGetF(EXCH_KND_CD	,	PST_FIELD(pList, exch_knd_cd	));

				/// [티머니고속] 승차권발권(카드_KTC) 정보 set memory
				CPubTckTmExpMem::GetInstance()->m_tResPubTckCardKtcList.push_back(*pList);
			}
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
#if (_KTC_CERTIFY_ > 0)
		LOG_WRITE("%30s - (%s) ", "CARD_NO"				,	PST_FIELD(pRPacket, card_no				));
#endif
		LOG_WRITE("%30s - (%s) ", "CACM_NM_PRIN_YN"		,	PST_FIELD(pRPacket, cacm_nm_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_PRIN_YN"		,	PST_FIELD(pRPacket, bus_cls_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME_PRIN_YN"	,	PST_FIELD(pRPacket, depr_time_prin_yn	));
		LOG_WRITE("%30s - (%s) ", "SATS_NO_PRIN_YN"		,	PST_FIELD(pRPacket, sats_no_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "TISSU_DT"			,	PST_FIELD(pRPacket, tissu_dt			));
		LOG_WRITE("%30s - (%s) ", "TISSU_TRML_NO"		,	PST_FIELD(pRPacket, tissu_trml_no		));
		LOG_WRITE("%30s - (%s) ", "TISSU_WND_NO"		,	PST_FIELD(pRPacket, tissu_wnd_no		));
		LOG_WRITE("%30s - (%s) ", "TISSU_TIME"			,	PST_FIELD(pRPacket, tissu_time			));
		LOG_WRITE("%30s - (%s) ", "TISSU_NO"			,	PST_FIELD(pRPacket, tissu_no			));
		LOG_WRITE("%30s - (%s) ", "CARD_APRV_NO"		,	PST_FIELD(pRPacket, card_aprv_no		));
		LOG_WRITE("%30s - (Num=%d) ", "N_APRV_AMT"		,	*(int *)pRPacket->n_aprv_amt			 );
		LOG_WRITE("%30s - (%s) ", "FRC_CMRT"			,	PST_FIELD(pRPacket, frc_cmrt			));
		LOG_WRITE("%30s - (%s) ", "ABLE_POINT"			,	PST_FIELD(pRPacket, able_point			));

		/// [티머니고속] 
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_pubtckcard_ktc_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 승차권 발권(카드_KTC), 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (Num=%d) ", "TISSU_FEE",	*(int *)pList->n_tissu_fee		 );
			LOG_WRITE("%30s - (Num=%d) ", "OGN_FEE"	,	*(int *)pList->n_ogn_fee		 );
			LOG_WRITE("%30s - (%s) ", "SATS_NO"		,	PST_FIELD(pList, sats_no		));
			LOG_WRITE("%30s - (%s) ", "INHR_NO"		,	PST_FIELD(pList, inhr_no		));
			LOG_WRITE("%30s - (%s) ", "INVS_NO"		,	PST_FIELD(pList, invs_no		));
			LOG_WRITE("%30s - (%s) ", "TISSU_SNO"	,	PST_FIELD(pList, tissu_sno		));
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"	,	PST_FIELD(pList, tck_knd_cd		));
			LOG_WRITE("%30s - (%s) ", "DC_KND_CD"	,	PST_FIELD(pList, dc_knd_cd		));
			LOG_WRITE("%30s - (%s) ", "EXCH_KND_CD"	,	PST_FIELD(pList, exch_knd_cd	));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_PubTckCsrc
 * @details		[티머니고속] 현장발권(현금영수증)
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_PubTckCsrc(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nCount1;
	char*	pService = "TK_PubTckCsrc";
	pstk_tm_pubtckcsrc_t pSPacket;
	prtk_tm_pubtckcsrc_t pRPacket;

	nRet = nTimeout = nCount = nCount1 = 0;

	LOG_OPEN();
	LOG_WRITE(" 현장발권(현영)_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_pubtckcsrc_t) pData;
	pRPacket = (prtk_tm_pubtckcsrc_t) retBuf;

	/// 티머니고속 - zero memory
	::ZeroMemory(&CPubTckTmExpMem::GetInstance()->m_tResPubTckCsrc, sizeof(rtk_tm_pubtckcsrc_t));
	CPubTckTmExpMem::GetInstance()->m_tResPubTckCsrcList.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","USER_DVS_CD"		,	pSPacket->user_dvs_cd			);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TRML_NO",	pSPacket->alcn_depr_trml_no		);
		LOG_WRITE("%30s - (%s) ","ALCN_ARVL_TRML_NO",	pSPacket->alcn_arvl_trml_no		);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_DT"		,	pSPacket->alcn_depr_dt			);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TIME"	,	pSPacket->alcn_depr_time		);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO"		,	pSPacket->depr_trml_no			);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"		,	pSPacket->arvl_trml_no			);
		LOG_WRITE("%30s - (%s) ","DEPR_DT"			,	pSPacket->depr_dt				);
		LOG_WRITE("%30s - (%s) ","DEPR_TIME"		,	pSPacket->depr_time				);
		LOG_WRITE("%30s - (%s) ","BUS_CLS_CD"		,	pSPacket->bus_cls_cd			);
		LOG_WRITE("%30s - (%s) ","CACM_CD"			,	pSPacket->cacm_cd				);
		LOG_WRITE("%30s - (%s) ","USER_KEY_VAL"		,	pSPacket->user_key_val			);
		LOG_WRITE("%30s - (%s) ","MBRS_YN"			,	pSPacket->mbrs_yn				);
		LOG_WRITE("%30s - (%s) ","MBRS_NO"			,	pSPacket->mbrs_no				);
		LOG_WRITE("%30s - (%s) ","MRNP_DT"			,	pSPacket->mrnp_dt				);
		LOG_WRITE("%30s - (%s) ","MRNP_TIME"		,	pSPacket->mrnp_time				);
		LOG_WRITE("%30s - (%s) ","TRD_DVS_CD"		,	pSPacket->trd_dvs_cd			);

#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ","ENC_DTA"			,	pSPacket->enc_dta				);
		LOG_WRITE("%30s - (%s) ","EMV_DTA"			,	pSPacket->emv_dta				);
#endif
		LOG_WRITE("%30s - (%d) ","REC_NUM"			,	*(int *)pSPacket->rec_num	);

		::CopyMemory(&nCount, pSPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			pstk_tm_pubtckcsrc_list_t pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("####### 승차권 발권(현영) 정보, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ","TCK_KND_CD"	,	pList->tck_knd_cd	);
			LOG_WRITE("%30s - (%s) ","SATS_NO"		,	pList->sats_no		);
			LOG_WRITE("%30s - (%s) ","PCPY_NO"		,	pList->pcpy_no		);
			LOG_WRITE("%30s - (%s) ","FEE_KND_CD"	,	pList->fee_knd_cd	);
			LOG_WRITE("%30s - (%s) ","N_TISSU_FEE"	,	pList->n_tissu_fee	);
			LOG_WRITE("%30s - (%s) ","N_OGN_FEE"	,	pList->n_ogn_fee	);
			LOG_WRITE("%30s - (%s) ","DC_KND_CD"	,	pList->dc_knd_cd	);
		}
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(USER_DVS_CD		,	pSPacket->user_dvs_cd			);
		nRet = TMaxFBPut(ALCN_DEPR_TRML_NO	,	pSPacket->alcn_depr_trml_no		);
		nRet = TMaxFBPut(ALCN_ARVL_TRML_NO	,	pSPacket->alcn_arvl_trml_no		);
		nRet = TMaxFBPut(ALCN_DEPR_DT		,	pSPacket->alcn_depr_dt			);
		nRet = TMaxFBPut(ALCN_DEPR_TIME		,	pSPacket->alcn_depr_time		);
		nRet = TMaxFBPut(DEPR_TRML_NO		,	pSPacket->depr_trml_no			);
		nRet = TMaxFBPut(ARVL_TRML_NO		,	pSPacket->arvl_trml_no			);
		nRet = TMaxFBPut(DEPR_DT			,	pSPacket->depr_dt				);
		nRet = TMaxFBPut(DEPR_TIME			,	pSPacket->depr_time				);
		nRet = TMaxFBPut(BUS_CLS_CD			,	pSPacket->bus_cls_cd			);
		nRet = TMaxFBPut(CACM_CD			,	pSPacket->cacm_cd				);
		nRet = TMaxFBPut(USER_KEY_VAL		,	pSPacket->user_key_val			);
		nRet = TMaxFBPut(MBRS_YN			,	pSPacket->mbrs_yn				);
		nRet = TMaxFBPut(MBRS_NO			,	pSPacket->mbrs_no				);
		nRet = TMaxFBPut(MRNP_DT			,	pSPacket->mrnp_dt				);
		nRet = TMaxFBPut(MRNP_TIME			,	pSPacket->mrnp_time				);
		nRet = TMaxFBPut(TRD_DVS_CD			,	pSPacket->trd_dvs_cd			);
		nRet = TMaxFBPut(ENC_DTA			,	pSPacket->enc_dta				);
		nRet = TMaxFBPut(EMV_DTA			,	pSPacket->emv_dta				);

		nRet = TMaxFBPut(REC_NUM			,	pSPacket->rec_num			);

		::CopyMemory(&nCount, pSPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			pstk_tm_pubtckcsrc_list_t pList;

			pList = &pSPacket->List[i];

			nRet = TMaxFBInsert(TCK_KND_CD	,	pList->tck_knd_cd	, i, sizeof(pList->tck_knd_cd	)	);
			nRet = TMaxFBInsert(SATS_NO		,	pList->sats_no		, i, sizeof(pList->sats_no		)	);
			nRet = TMaxFBInsert(PCPY_NO		,	pList->pcpy_no		, i, sizeof(pList->pcpy_no		)	);
			nRet = TMaxFBInsert(FEE_KND_CD	,	pList->fee_knd_cd	, i, sizeof(pList->fee_knd_cd	)	);
			nRet = TMaxFBInsert(TISSU_FEE	,	pList->n_tissu_fee	, i, sizeof(pList->n_tissu_fee	)	);
			nRet = TMaxFBInsert(OGN_FEE		,	pList->n_ogn_fee	, i, sizeof(pList->n_ogn_fee	)	);
			nRet = TMaxFBInsert(DC_KND_CD	,	pList->dc_knd_cd	, i, sizeof(pList->dc_knd_cd	)	);
		}
	}

	m_nTPCallFlag = TPNOFLAGS;
	TMaxTransaction(TMAX_TR_BEGIN);

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			TMaxTransaction(TMAX_TR_ROLLBACK);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		TMaxTransaction(TMAX_TR_COMMIT);

		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.rsp_cd, pRPacket->rsp_cd, sizeof(CPubTckTmExpMem::GetInstance()->base.rsp_cd));

		// get data
		{
			nRet = TMaxFBGet(CACM_NM_PRIN_YN	, PST_FIELD(pRPacket, cacm_nm_prin_yn	 ));
			nRet = TMaxFBGet(BUS_CLS_PRIN_YN	, PST_FIELD(pRPacket, bus_cls_prin_yn	 ));
			nRet = TMaxFBGet(DEPR_TIME_PRIN_YN	, PST_FIELD(pRPacket, depr_time_prin_yn	 ));
			nRet = TMaxFBGet(SATS_NO_PRIN_YN	, PST_FIELD(pRPacket, sats_no_prin_yn	 ));
			nRet = TMaxFBGet(TISSU_DT			, PST_FIELD(pRPacket, tissu_dt			 ));
			nRet = TMaxFBGet(TISSU_TRML_NO		, PST_FIELD(pRPacket, tissu_trml_no		 ));
			nRet = TMaxFBGet(TISSU_WND_NO		, PST_FIELD(pRPacket, tissu_wnd_no		 ));
			nRet = TMaxFBGet(TISSU_TIME			, PST_FIELD(pRPacket, tissu_time		 ));
			nRet = TMaxFBGet(USER_KEY_VAL		, PST_FIELD(pRPacket, user_key_val		 ));
			nRet = TMaxFBGet(USER_DVS_CD		, PST_FIELD(pRPacket, user_dvs_cd		 ));
		}

		/// 정보 수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// 티머니고속 - set memory
		::CopyMemory(&CPubTckTmExpMem::GetInstance()->m_tResPubTckCsrc, pRPacket, sizeof(rtk_tm_pubtckcsrc_t));

		/// memory allocation
		{
			/// 승차권 발권(현영) List
			pRPacket->pList = new rtk_tm_pubtckcsrc_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_pubtckcsrc_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				prtk_tm_pubtckcsrc_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TISSU_FEE		,	PST_FIELD(pList, n_tissu_fee	));
				nRet = TMaxFBGetF(OGN_FEE		,	PST_FIELD(pList, n_ogn_fee		));
				nRet = TMaxFBGetF(SATS_NO		,	PST_FIELD(pList, sats_no		));
				nRet = TMaxFBGetF(INHR_NO		,	PST_FIELD(pList, inhr_no		));
				nRet = TMaxFBGetF(INVS_NO		,	PST_FIELD(pList, invs_no		));
				nRet = TMaxFBGetF(TISSU_SNO		,	PST_FIELD(pList, tissu_sno		));
				nRet = TMaxFBGetF(CSRC_APRV_NO	,	PST_FIELD(pList, csrc_aprv_no	));
				nRet = TMaxFBGetF(CSRC_RGT_NO	,	PST_FIELD(pList, csrc_rgt_no	));
				nRet = TMaxFBGetF(CASH_RECP_AMT	,	PST_FIELD(pList, n_cash_recp_amt	));
				nRet = TMaxFBGetF(TISSU_NO		,	PST_FIELD(pList, tissu_no	));
				nRet = TMaxFBGetF(TCK_KND_CD	,	PST_FIELD(pList, tck_knd_cd		));
				nRet = TMaxFBGetF(DC_KND_CD		,	PST_FIELD(pList, dc_knd_cd		));
				nRet = TMaxFBGetF(EXCH_KND_CD	,	PST_FIELD(pList, exch_knd_cd	));

				/// [티머니고속] 승차권 발권(현영) 정보 set memory
				CPubTckTmExpMem::GetInstance()->m_tResPubTckCsrcList.push_back(*pList);
			}
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{

		LOG_WRITE("%30s - (%s) ", "CACM_NM_PRIN_YN"		,	PST_FIELD(pRPacket, cacm_nm_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_PRIN_YN"		,	PST_FIELD(pRPacket, bus_cls_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME_PRIN_YN"	,	PST_FIELD(pRPacket, depr_time_prin_yn	));
		LOG_WRITE("%30s - (%s) ", "SATS_NO_PRIN_YN"		,	PST_FIELD(pRPacket, sats_no_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "TISSU_DT"			,	PST_FIELD(pRPacket, tissu_dt			));
		LOG_WRITE("%30s - (%s) ", "TISSU_TRML_NO"		,	PST_FIELD(pRPacket, tissu_trml_no		));
		LOG_WRITE("%30s - (%s) ", "TISSU_WND_NO"		,	PST_FIELD(pRPacket, tissu_wnd_no		));
		LOG_WRITE("%30s - (%s) ", "TISSU_TIME"			,	PST_FIELD(pRPacket, tissu_time			));
		LOG_WRITE("%30s - (%s) ", "USER_KEY_VAL"		,	PST_FIELD(pRPacket, user_key_val		));
		LOG_WRITE("%30s - (%s) ", "USER_DVS_CD"			,	PST_FIELD(pRPacket, user_dvs_cd			));

		/// [티머니고속] 
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_pubtckcsrc_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 승차권 발권(현영), 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "N_TISSU_FEE"		,	PST_FIELD(pList, n_tissu_fee		));
			LOG_WRITE("%30s - (%s) ", "N_OGN_FEE"		,	PST_FIELD(pList, n_ogn_fee			));
			LOG_WRITE("%30s - (%s) ", "SATS_NO"			,	PST_FIELD(pList, sats_no			));
			LOG_WRITE("%30s - (%s) ", "INHR_NO"			,	PST_FIELD(pList, inhr_no			));
			LOG_WRITE("%30s - (%s) ", "INVS_NO"			,	PST_FIELD(pList, invs_no			));
			LOG_WRITE("%30s - (%s) ", "TISSU_SNO"		,	PST_FIELD(pList, tissu_sno			));
			LOG_WRITE("%30s - (%s) ", "CSRC_APRV_NO"	,	PST_FIELD(pList, csrc_aprv_no		));
			LOG_WRITE("%30s - (%s) ", "CSRC_RGT_NO"		,	PST_FIELD(pList, csrc_rgt_no		));
			LOG_WRITE("%30s - (%s) ", "N_CASH_RECP_AMT"	,	PST_FIELD(pList, n_cash_recp_amt	));
			LOG_WRITE("%30s - (%s) ", "TISSU_NO"		,	PST_FIELD(pList, tissu_no			));
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"		,	PST_FIELD(pList, tck_knd_cd			));
			LOG_WRITE("%30s - (%s) ", "DC_KND_CD"		,	PST_FIELD(pList, dc_knd_cd			));
			LOG_WRITE("%30s - (%s) ", "EXCH_KND_CD"		,	PST_FIELD(pList, exch_knd_cd		));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_PubTckCsrc_KTC
 * @details		[티머니고속] 현장발권(현금영수증)
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_PubTckCsrc_KTC(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nCount1;
	char*	pService = "TK_KtcPubCsrc";
	pstk_tm_pubtckcsrc_ktc_t pSPacket;
	prtk_tm_pubtckcsrc_ktc_t pRPacket;

	nRet = nTimeout = nCount = nCount1 = 0;

	LOG_OPEN();
	LOG_WRITE(" 현장발권(현영_KTC)_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_pubtckcsrc_ktc_t) pData;
	pRPacket = (prtk_tm_pubtckcsrc_ktc_t) retBuf;

	/// 티머니고속 - zero memory
	::ZeroMemory(&CPubTckTmExpMem::GetInstance()->m_tResPubTckCsrcKtc, sizeof(rtk_tm_pubtckcsrc_ktc_t));
	CPubTckTmExpMem::GetInstance()->m_tResPubTckCsrcKtcList.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","USER_DVS_CD"			,	pSPacket->user_dvs_cd		);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TRML_NO"	,	pSPacket->alcn_depr_trml_no	);
		LOG_WRITE("%30s - (%s) ","ALCN_ARVL_TRML_NO"	,	pSPacket->alcn_arvl_trml_no	);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_DT"			,	pSPacket->alcn_depr_dt		);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TIME"		,	pSPacket->alcn_depr_time	);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO"			,	pSPacket->depr_trml_no		);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"			,	pSPacket->arvl_trml_no		);
		LOG_WRITE("%30s - (%s) ","DEPR_DT"				,	pSPacket->depr_dt			);
		LOG_WRITE("%30s - (%s) ","DEPR_TIME"			,	pSPacket->depr_time			);
		LOG_WRITE("%30s - (%s) ","BUS_CLS_CD"			,	pSPacket->bus_cls_cd		);
		LOG_WRITE("%30s - (%s) ","CACM_CD"				,	pSPacket->cacm_cd			);
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ","USER_KEY_VAL"			,	pSPacket->user_key_val		);
#endif
		LOG_WRITE("%30s - (%s) ","MBRS_YN"				,	pSPacket->mbrs_yn			);
		LOG_WRITE("%30s - (%s) ","MBRS_NO"				,	pSPacket->mbrs_no			);
		LOG_WRITE("%30s - (%s) ","MRNP_DT"				,	pSPacket->mrnp_dt			);
		LOG_WRITE("%30s - (%s) ","MRNP_TIME"			,	pSPacket->mrnp_time			);
		LOG_WRITE("%30s - (%s) ","TRD_DVS_CD"			,	pSPacket->trd_dvs_cd		);

#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ","ENC_DTA"				,	pSPacket->enc_dta			);
		LOG_WRITE("%30s - (%s) ","EMV_DTA"				,	pSPacket->emv_dta			);
#endif
		LOG_WRITE("%30s - (%d) ","REC_NUM"			,	*(int *)pSPacket->rec_num	);

		::CopyMemory(&nCount, pSPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			pstk_tm_pubtckcsrc_ktc_list_t pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("####### 승차권 발권(현영_KTC) 정보, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ","TCK_KND_CD"	,	pList->tck_knd_cd	);
			LOG_WRITE("%30s - (%s) ","SATS_NO"		,	pList->sats_no		);
			LOG_WRITE("%30s - (%s) ","PCPY_NO"		,	pList->pcpy_no		);
			LOG_WRITE("%30s - (%s) ","FEE_KND_CD"	,	pList->fee_knd_cd	);
			LOG_WRITE("%30s - (Num=%d) ","N_TISSU_FEE"	,	*(int *)pList->n_tissu_fee	);
			LOG_WRITE("%30s - (Num=%d) ","N_OGN_FEE"	,	*(int *)pList->n_ogn_fee	);
			LOG_WRITE("%30s - (%s) ","DC_KND_CD"	,	pList->dc_knd_cd	);
		}
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS	,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO	,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO		,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO	,	pSPacket->req_user_no			);

		nRet = TMaxFBPut(USER_DVS_CD		,	pSPacket->user_dvs_cd			);
		nRet = TMaxFBPut(ALCN_DEPR_TRML_NO	,	pSPacket->alcn_depr_trml_no		);
		nRet = TMaxFBPut(ALCN_ARVL_TRML_NO	,	pSPacket->alcn_arvl_trml_no		);
		nRet = TMaxFBPut(ALCN_DEPR_DT		,	pSPacket->alcn_depr_dt			);
		nRet = TMaxFBPut(ALCN_DEPR_TIME		,	pSPacket->alcn_depr_time		);
		nRet = TMaxFBPut(DEPR_TRML_NO		,	pSPacket->depr_trml_no			);
		nRet = TMaxFBPut(ARVL_TRML_NO		,	pSPacket->arvl_trml_no			);
		nRet = TMaxFBPut(DEPR_DT			,	pSPacket->depr_dt				);
		nRet = TMaxFBPut(DEPR_TIME			,	pSPacket->depr_time				);
		nRet = TMaxFBPut(BUS_CLS_CD			,	pSPacket->bus_cls_cd			);
		nRet = TMaxFBPut(CACM_CD			,	pSPacket->cacm_cd				);
		nRet = TMaxFBPut(USER_KEY_VAL		,	pSPacket->user_key_val			);
		nRet = TMaxFBPut(MBRS_YN			,	pSPacket->mbrs_yn				);
		nRet = TMaxFBPut(MBRS_NO			,	pSPacket->mbrs_no				);
		nRet = TMaxFBPut(MRNP_DT			,	pSPacket->mrnp_dt				);
		nRet = TMaxFBPut(MRNP_TIME			,	pSPacket->mrnp_time				);
		nRet = TMaxFBPut(TRD_DVS_CD			,	pSPacket->trd_dvs_cd			);
		nRet = TMaxFBPut(ENC_DTA			,	pSPacket->enc_dta				);
		nRet = TMaxFBPut(EMV_DTA			,	pSPacket->emv_dta				);

		nRet = TMaxFBPut(REC_NUM			,	pSPacket->rec_num			);

		::CopyMemory(&nCount, pSPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			pstk_tm_pubtckcsrc_ktc_list_t pList;

			pList = &pSPacket->List[i];

			nRet = TMaxFBInsert(TCK_KND_CD	,	pList->tck_knd_cd	, i, sizeof(pList->tck_knd_cd	)	);
			nRet = TMaxFBInsert(SATS_NO		,	pList->sats_no		, i, sizeof(pList->sats_no		)	);
			nRet = TMaxFBInsert(PCPY_NO		,	pList->pcpy_no		, i, sizeof(pList->pcpy_no		)	);
			nRet = TMaxFBInsert(FEE_KND_CD	,	pList->fee_knd_cd	, i, sizeof(pList->fee_knd_cd	)	);
			nRet = TMaxFBInsert(TISSU_FEE	,	pList->n_tissu_fee	, i, sizeof(pList->n_tissu_fee	)	);
			nRet = TMaxFBInsert(OGN_FEE		,	pList->n_ogn_fee	, i, sizeof(pList->n_ogn_fee	)	);
			nRet = TMaxFBInsert(DC_KND_CD	,	pList->dc_knd_cd	, i, sizeof(pList->dc_knd_cd	)	);
		}
	}

	m_nTPCallFlag = TPNOFLAGS;
	TMaxTransaction(TMAX_TR_BEGIN);

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			TMaxTransaction(TMAX_TR_ROLLBACK);

			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		TMaxTransaction(TMAX_TR_COMMIT);

		// get data
		{
			nRet = TMaxFBGet(CACM_NM_PRIN_YN	, PST_FIELD(pRPacket, cacm_nm_prin_yn	 ));
			nRet = TMaxFBGet(BUS_CLS_PRIN_YN	, PST_FIELD(pRPacket, bus_cls_prin_yn	 ));
			nRet = TMaxFBGet(DEPR_TIME_PRIN_YN	, PST_FIELD(pRPacket, depr_time_prin_yn	 ));
			nRet = TMaxFBGet(SATS_NO_PRIN_YN	, PST_FIELD(pRPacket, sats_no_prin_yn	 ));
			nRet = TMaxFBGet(TISSU_DT			, PST_FIELD(pRPacket, tissu_dt			 ));
			nRet = TMaxFBGet(TISSU_TRML_NO		, PST_FIELD(pRPacket, tissu_trml_no		 ));
			nRet = TMaxFBGet(TISSU_WND_NO		, PST_FIELD(pRPacket, tissu_wnd_no		 ));
			nRet = TMaxFBGet(TISSU_TIME			, PST_FIELD(pRPacket, tissu_time		 ));
			nRet = TMaxFBGet(USER_KEY_VAL		, PST_FIELD(pRPacket, user_key_val		 ));
			nRet = TMaxFBGet(USER_DVS_CD		, PST_FIELD(pRPacket, user_dvs_cd		 ));
		}

		/// 정보 수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// 티머니고속 - set memory
		::CopyMemory(&CPubTckTmExpMem::GetInstance()->m_tResPubTckCsrcKtc, pRPacket, sizeof(rtk_tm_pubtckcsrc_ktc_t));

		/// memory allocation
		{
			/// 승차권 발권(현영_KTC) List
			pRPacket->pList = new rtk_tm_pubtckcsrc_ktc_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_pubtckcsrc_ktc_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				prtk_tm_pubtckcsrc_ktc_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TISSU_FEE		,	PST_FIELD(pList, n_tissu_fee	));
				nRet = TMaxFBGetF(OGN_FEE		,	PST_FIELD(pList, n_ogn_fee		));
				nRet = TMaxFBGetF(SATS_NO		,	PST_FIELD(pList, sats_no		));
				nRet = TMaxFBGetF(INHR_NO		,	PST_FIELD(pList, inhr_no		));
				nRet = TMaxFBGetF(INVS_NO		,	PST_FIELD(pList, invs_no		));
				nRet = TMaxFBGetF(TISSU_SNO		,	PST_FIELD(pList, tissu_sno		));
				nRet = TMaxFBGetF(CSRC_APRV_NO	,	PST_FIELD(pList, csrc_aprv_no	));
				nRet = TMaxFBGetF(CSRC_RGT_NO	,	PST_FIELD(pList, csrc_rgt_no	));
				nRet = TMaxFBGetF(CASH_RECP_AMT	,	PST_FIELD(pList, n_cash_recp_amt	));
				nRet = TMaxFBGetF(TISSU_NO		,	PST_FIELD(pList, tissu_no	));
				nRet = TMaxFBGetF(TCK_KND_CD	,	PST_FIELD(pList, tck_knd_cd		));
				nRet = TMaxFBGetF(DC_KND_CD		,	PST_FIELD(pList, dc_knd_cd		));
				nRet = TMaxFBGetF(EXCH_KND_CD	,	PST_FIELD(pList, exch_knd_cd	));

				/// [티머니고속] 승차권 발권(현영) 정보 set memory
				CPubTckTmExpMem::GetInstance()->m_tResPubTckCsrcKtcList.push_back(*pList);
			}
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{

		LOG_WRITE("%30s - (%s) ", "CACM_NM_PRIN_YN"		,	PST_FIELD(pRPacket, cacm_nm_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_PRIN_YN"		,	PST_FIELD(pRPacket, bus_cls_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME_PRIN_YN"	,	PST_FIELD(pRPacket, depr_time_prin_yn	));
		LOG_WRITE("%30s - (%s) ", "SATS_NO_PRIN_YN"		,	PST_FIELD(pRPacket, sats_no_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "TISSU_DT"			,	PST_FIELD(pRPacket, tissu_dt			));
		LOG_WRITE("%30s - (%s) ", "TISSU_TRML_NO"		,	PST_FIELD(pRPacket, tissu_trml_no		));
		LOG_WRITE("%30s - (%s) ", "TISSU_WND_NO"		,	PST_FIELD(pRPacket, tissu_wnd_no		));
		LOG_WRITE("%30s - (%s) ", "TISSU_TIME"			,	PST_FIELD(pRPacket, tissu_time			));
		LOG_WRITE("%30s - (%s) ", "USER_KEY_VAL"		,	PST_FIELD(pRPacket, user_key_val		));
		LOG_WRITE("%30s - (%s) ", "USER_DVS_CD"			,	PST_FIELD(pRPacket, user_dvs_cd			));

		/// [티머니고속] 
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_pubtckcsrc_ktc_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 승차권 발권(현영_KTC), 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%d) ", "N_TISSU_FEE"		,	*(int *)pList->n_tissu_fee			);
			LOG_WRITE("%30s - (%d) ", "N_OGN_FEE"		,	*(int *)pList->n_ogn_fee			);
			LOG_WRITE("%30s - (%s) ", "SATS_NO"			,	PST_FIELD(pList, sats_no			));
			LOG_WRITE("%30s - (%s) ", "INHR_NO"			,	PST_FIELD(pList, inhr_no			));
			LOG_WRITE("%30s - (%s) ", "INVS_NO"			,	PST_FIELD(pList, invs_no			));
			LOG_WRITE("%30s - (%s) ", "TISSU_SNO"		,	PST_FIELD(pList, tissu_sno			));
			LOG_WRITE("%30s - (%s) ", "CSRC_APRV_NO"	,	PST_FIELD(pList, csrc_aprv_no		));
			LOG_WRITE("%30s - (%s) ", "CSRC_RGT_NO"		,	PST_FIELD(pList, csrc_rgt_no		));
			LOG_WRITE("%30s - (%d) ", "CASH_RECP_AMT"	,	*(int *)pList->n_cash_recp_amt		);
			LOG_WRITE("%30s - (%s) ", "TISSU_NO"		,	PST_FIELD(pList, tissu_no			));
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"		,	PST_FIELD(pList, tck_knd_cd			));
			LOG_WRITE("%30s - (%s) ", "DC_KND_CD"		,	PST_FIELD(pList, dc_knd_cd			));
			LOG_WRITE("%30s - (%s) ", "EXCH_KND_CD"		,	PST_FIELD(pList, exch_knd_cd		));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_PubTckPrd
 * @details		[티머니고속] 현장발권(부가상품권)
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_PubTckPrd(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nCount1;
	char*	pService = "TK_PubTckPrd";
	pstk_tm_pubtckprd_t pSPacket;
	prtk_tm_pubtckprd_t pRPacket;

	nRet = nTimeout = nCount = nCount1 = 0;

	LOG_OPEN();
	LOG_WRITE(" 현장발권(부가상품권)_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_pubtckprd_t) pData;
	pRPacket = (prtk_tm_pubtckprd_t) retBuf;

	/// 티머니고속 - zero memory
	::ZeroMemory(&CPubTckTmExpMem::GetInstance()->m_tResPubTckPrd, sizeof(rtk_tm_pubtckprd_t));
	CPubTckTmExpMem::GetInstance()->m_tResPubTckPrdList.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TRML_NO"	,	pSPacket->alcn_depr_trml_no		);
		LOG_WRITE("%30s - (%s) ","ALCN_ARVL_TRML_NO"	,	pSPacket->alcn_arvl_trml_no		);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_DT"			,	pSPacket->alcn_depr_dt			);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TIME"		,	pSPacket->alcn_depr_time		);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO	"		,	pSPacket->depr_trml_no			);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"			,	pSPacket->arvl_trml_no			);
		LOG_WRITE("%30s - (%s) ","DEPR_DT"				,	pSPacket->depr_dt				);
		LOG_WRITE("%30s - (%s) ","DEPR_TIME"			,	pSPacket->depr_time				);
		LOG_WRITE("%30s - (%s) ","BUS_CLS_CD"			,	pSPacket->bus_cls_cd			);
		LOG_WRITE("%30s - (%s) ","CACM_CD"				,	pSPacket->cacm_cd				);
		LOG_WRITE("%30s - (%s) ","ADTN_CPN_NO"			,	pSPacket->adtn_cpn_no			);
		LOG_WRITE("%30s - (%s) ","ADTN_PRD_AUTH_NO"		,	pSPacket->adtn_prd_auth_no		);

		LOG_WRITE("%30s - (%d) ","REC_NUM"			,	*(int *)pSPacket->rec_num	);

		::CopyMemory(&nCount, pSPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			pstk_tm_pubtckprd_list_t pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("####### 승차권 발권(부가상품권) 정보, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ","SATS_NO"		,	pList->sats_no	);
			LOG_WRITE("%30s - (%s) ","PCPY_NO"		,	pList->pcpy_no	);
		}
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS		,	pSPacket->req_pgm_dvs		);
		nRet = TMaxFBPut(REQ_TRML_NO		,	pSPacket->req_trml_no		);
		nRet = TMaxFBPut(REQ_WND_NO			,	pSPacket->req_wnd_no		);
		nRet = TMaxFBPut(REQ_USER_NO		,	pSPacket->req_user_no		);

		nRet = TMaxFBPut(ALCN_DEPR_TRML_NO	,	pSPacket->alcn_depr_trml_no	);
		nRet = TMaxFBPut(ALCN_ARVL_TRML_NO	,	pSPacket->alcn_arvl_trml_no	);
		nRet = TMaxFBPut(ALCN_DEPR_DT		,	pSPacket->alcn_depr_dt		);
		nRet = TMaxFBPut(ALCN_DEPR_TIME		,	pSPacket->alcn_depr_time	);
		nRet = TMaxFBPut(DEPR_TRML_NO		,	pSPacket->depr_trml_no		);
		nRet = TMaxFBPut(ARVL_TRML_NO		,	pSPacket->arvl_trml_no		);
		nRet = TMaxFBPut(DEPR_DT			,	pSPacket->depr_dt			);
		nRet = TMaxFBPut(DEPR_TIME			,	pSPacket->depr_time			);
		nRet = TMaxFBPut(BUS_CLS_CD			,	pSPacket->bus_cls_cd		);
		nRet = TMaxFBPut(CACM_CD			,	pSPacket->cacm_cd			);
		nRet = TMaxFBPut(ADTN_CPN_NO		,	pSPacket->adtn_cpn_no		);
		nRet = TMaxFBPut(ADTN_PRD_AUTH_NO	,	pSPacket->adtn_prd_auth_no	);

		nRet = TMaxFBPut(REC_NUM			,	pSPacket->rec_num			);

		::CopyMemory(&nCount, pSPacket->rec_num, 4);
		for(i = 0; i < nCount; i++)
		{
			pstk_tm_pubtckprd_list_t pList;

			pList = &pSPacket->List[i];

			nRet = TMaxFBInsert(SATS_NO		,	pList->sats_no		, i, sizeof(pList->sats_no		)	);
			nRet = TMaxFBInsert(PCPY_NO		,	pList->pcpy_no		, i, sizeof(pList->pcpy_no		)	);
		}
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.rsp_cd, pRPacket->rsp_cd, sizeof(CPubTckTmExpMem::GetInstance()->base.rsp_cd));

		// get data
		{
			nRet = TMaxFBGet(ADTN_CPN_NO		, PST_FIELD(pRPacket, adtn_cpn_no		 ));
			nRet = TMaxFBGet(CACM_NM_PRIN_YN	, PST_FIELD(pRPacket, cacm_nm_prin_yn	 ));
			nRet = TMaxFBGet(BUS_CLS_PRIN_YN	, PST_FIELD(pRPacket, bus_cls_prin_yn	 ));
			nRet = TMaxFBGet(DEPR_TIME_PRIN_YN	, PST_FIELD(pRPacket, depr_time_prin_yn	 ));
			nRet = TMaxFBGet(SATS_NO_PRIN_YN	, PST_FIELD(pRPacket, sats_no_prin_yn	 ));
			nRet = TMaxFBGet(TISSU_DT			, PST_FIELD(pRPacket, tissu_dt			 ));
			nRet = TMaxFBGet(TISSU_TRML_NO		, PST_FIELD(pRPacket, tissu_trml_no		 ));
			nRet = TMaxFBGet(TISSU_WND_NO		, PST_FIELD(pRPacket, tissu_wnd_no		 ));
			nRet = TMaxFBGet(TISSU_TIME			, PST_FIELD(pRPacket, tissu_time		 ));
			nRet = TMaxFBGet(TISSU_NO			, PST_FIELD(pRPacket, tissu_no			 ));
		}

		/// 정보 수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// 티머니고속 - set memory
		::CopyMemory(&CPubTckTmExpMem::GetInstance()->m_tResPubTckPrd, pRPacket, sizeof(rtk_tm_pubtckprd_t));

		/// memory allocation
		{
			/// 승차권 발권(현영_KTC) List
			pRPacket->pList = new rtk_tm_pubtckprd_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_pubtckprd_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				prtk_tm_pubtckprd_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TISSU_FEE		,	PST_FIELD(pList, n_tissu_fee	));
				nRet = TMaxFBGetF(OGN_FEE		,	PST_FIELD(pList, n_ogn_fee		));
				nRet = TMaxFBGetF(SATS_NO		,	PST_FIELD(pList, sats_no		));
				nRet = TMaxFBGetF(INHR_NO		,	PST_FIELD(pList, inhr_no		));
				nRet = TMaxFBGetF(INVS_NO		,	PST_FIELD(pList, invs_no		));
				nRet = TMaxFBGetF(TISSU_SNO		,	PST_FIELD(pList, tissu_sno		));
				nRet = TMaxFBGetF(TCK_KND_CD	,	PST_FIELD(pList, tck_knd_cd		));

				/// [티머니고속] 승차권 발권(현영) 정보 set memory
				CPubTckTmExpMem::GetInstance()->m_tResPubTckPrdList.push_back(*pList);
			}
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{

		LOG_WRITE("%30s - (%s) ", "ADTN_CPN_NO"			,	PST_FIELD(pRPacket, adtn_cpn_no			));
		LOG_WRITE("%30s - (%s) ", "CACM_NM_PRIN_YN"		,	PST_FIELD(pRPacket, cacm_nm_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_PRIN_YN"		,	PST_FIELD(pRPacket, bus_cls_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME_PRIN_YN"	,	PST_FIELD(pRPacket, depr_time_prin_yn	));
		LOG_WRITE("%30s - (%s) ", "SATS_NO_PRIN_YN"		,	PST_FIELD(pRPacket, sats_no_prin_yn		));
		LOG_WRITE("%30s - (%s) ", "TISSU_DT"			,	PST_FIELD(pRPacket, tissu_dt			));
		LOG_WRITE("%30s - (%s) ", "TISSU_TRML_NO"		,	PST_FIELD(pRPacket, tissu_trml_no		));
		LOG_WRITE("%30s - (%s) ", "TISSU_WND_NO	"		,	PST_FIELD(pRPacket, tissu_wnd_no		));
		LOG_WRITE("%30s - (%s) ", "TISSU_TIME"			,	PST_FIELD(pRPacket, tissu_time			));
		LOG_WRITE("%30s - (%s) ", "TISSU_NO"			,	PST_FIELD(pRPacket, tissu_no			));

		/// [티머니고속] 
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_pubtckprd_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 승차권 발권(부가상품권), 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%d) ", "TISSU_FEE"	,	*(int *)pList->n_tissu_fee		);
			LOG_WRITE("%30s - (%d) ", "OGN_FEE"		,	*(int *)pList->n_ogn_fee		);
			LOG_WRITE("%30s - (%s) ", "SATS_NO"		,	PST_FIELD(pList, sats_no		));
			LOG_WRITE("%30s - (%s) ", "INHR_NO"		,	PST_FIELD(pList, inhr_no		));
			LOG_WRITE("%30s - (%s) ", "INVS_NO"		,	PST_FIELD(pList, invs_no		));
			LOG_WRITE("%30s - (%s) ", "TISSU_SNO"	,	PST_FIELD(pList, tissu_sno		));
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"	,	PST_FIELD(pList, tck_knd_cd		));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_CM_ReadThruTrml
 * @details		[티머니고속] 경유지 정보 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CM_ReadThruTrml(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nCount1;
	char*	pService = "CM_ReadThruTrml";
	pstk_tm_readthrutrml_t pSPacket;
	prtk_tm_readthrutrml_t pRPacket;

	nRet = nTimeout = nCount = nCount1 = 0;

	LOG_OPEN();
	LOG_WRITE(" 현장발권(경유지정보 조회)_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_readthrutrml_t) pData;
	pRPacket = (prtk_tm_readthrutrml_t) retBuf;

	/// 티머니고속 - zero memory
	::ZeroMemory(&CPubTckTmExpMem::GetInstance()->m_tResPubTckPrd, sizeof(rtk_tm_pubtckprd_t));
	CPubTckTmExpMem::GetInstance()->m_vtResThruList.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"		,	pSPacket->req_pgm_dvs			);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"		,	pSPacket->req_trml_no			);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"		,	pSPacket->req_wnd_no			);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"		,	pSPacket->req_user_no			);
		LOG_WRITE("%30s - (Num=%d) ","REQ_TRML_NUM"	,	*(int *)pSPacket->req_trml_num	);
		LOG_WRITE("%30s - (%s) ","TRML_NO"			,	pSPacket->trml_no				);
		LOG_WRITE("%30s - (%s) ","BEF_AFT_DVS"		,	pSPacket->bef_aft_dvs			);
		LOG_WRITE("%30s - (Num=%d) ","REC_NUM"		,	*(int *)pSPacket->rec_num		);
		LOG_WRITE("%30s - (%s) ","DRTN_CD"			,	pSPacket->drtn_cd				);
		LOG_WRITE("%30s - (%s) ","HSPD_CTY_DVS_CD"	,	pSPacket->hspd_cty_dvs_cd		);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TRML_NO",	pSPacket->alcn_depr_trml_no		);
		LOG_WRITE("%30s - (%s) ","ALCN_ARVL_TRML_NO",	pSPacket->alcn_arvl_trml_no		);
		LOG_WRITE("%30s - (%s) ","ALCN_ROT_NO"		,	pSPacket->alcn_rot_no			);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"		,	pSPacket->arvl_trml_no			);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS		,	pSPacket->req_pgm_dvs			);
		nRet = TMaxFBPut(REQ_TRML_NO		,	pSPacket->req_trml_no			);
		nRet = TMaxFBPut(REQ_WND_NO			,	pSPacket->req_wnd_no			);
		nRet = TMaxFBPut(REQ_USER_NO		,	pSPacket->req_user_no			);
		nRet = TMaxFBPut(REQ_TRML_NUM		,	pSPacket->req_trml_num			);
		nRet = TMaxFBPut(TRML_NO			,	pSPacket->trml_no				);
		nRet = TMaxFBPut(BEF_AFT_DVS		,	pSPacket->bef_aft_dvs			);
		nRet = TMaxFBPut(REC_NUM			,	pSPacket->rec_num				);
		nRet = TMaxFBPut(DRTN_CD			,	pSPacket->drtn_cd				);
		nRet = TMaxFBPut(HSPD_CTY_DVS_CD	,	pSPacket->hspd_cty_dvs_cd		);
		nRet = TMaxFBPut(ALCN_DEPR_TRML_NO	,	pSPacket->alcn_depr_trml_no		);
		nRet = TMaxFBPut(ALCN_ARVL_TRML_NO	,	pSPacket->alcn_arvl_trml_no		);
		nRet = TMaxFBPut(ALCN_ROT_NO		,	pSPacket->alcn_rot_no			);
		nRet = TMaxFBPut(ARVL_TRML_NO		,	pSPacket->arvl_trml_no			);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		::CopyMemory(CPubTckTmExpMem::GetInstance()->base.rsp_cd, pRPacket->rsp_cd, sizeof(CPubTckTmExpMem::GetInstance()->base.rsp_cd));

		/// 정보 수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			/// 승차권 발권(현영_KTC) List
			pRPacket->pList = new rtk_tm_readthrutrml_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_readthrutrml_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				prtk_tm_readthrutrml_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TISSU_TRML_NO		 ,	PST_FIELD(pList, tissu_trml_no		 ));
				nRet = TMaxFBGetF(DEPR_TRML_NO		 ,	PST_FIELD(pList, depr_trml_no		 ));
				nRet = TMaxFBGetF(ARVL_TRML_NO		 ,	PST_FIELD(pList, arvl_trml_no		 ));
				nRet = TMaxFBGetF(ALCN_DEPR_TRML_NO	 ,	PST_FIELD(pList, alcn_depr_trml_no	 ));
				nRet = TMaxFBGetF(ALCN_ARVL_TRML_NO	 ,	PST_FIELD(pList, alcn_arvl_trml_no	 ));
				nRet = TMaxFBGetF(ALCN_ROT_NO		 ,	PST_FIELD(pList, alcn_rot_no		 ));
				nRet = TMaxFBGetF(STT_DT			 ,	PST_FIELD(pList, stt_dt				 ));
				nRet = TMaxFBGetF(END_DT			 ,	PST_FIELD(pList, end_dt				 ));
				nRet = TMaxFBGetF(DEPR_THRU_SEQ		 ,	PST_FIELD(pList, depr_thru_seq		 ));
				nRet = TMaxFBGetF(ARVL_THRU_SEQ		 ,	PST_FIELD(pList, arvl_thru_seq		 ));
				nRet = TMaxFBGetF(PRMM_SATS_STA_VAL	 ,	PST_FIELD(pList, prmm_sats_sta_val	 ));
				nRet = TMaxFBGetF(HSPD_SATS_STA_VAL	 ,	PST_FIELD(pList, hspd_sats_sta_val	 ));
				nRet = TMaxFBGetF(PSRM_SATS_STA_VAL	 ,	PST_FIELD(pList, psrm_sats_sta_val	 ));
				nRet = TMaxFBGetF(SATS_ASGT_VAL		 ,	PST_FIELD(pList, sats_asgt_val		 ));
				nRet = TMaxFBGetF(THRU_DIST			 ,	PST_FIELD(pList, thru_dist			 ));
				nRet = TMaxFBGetF(THRU_DRTM			 ,	PST_FIELD(pList, thru_drtm			 ));
				nRet = TMaxFBGetF(CACM_NM_PRIN_YN	 ,	PST_FIELD(pList, cacm_nm_prin_yn	 ));
				nRet = TMaxFBGetF(BUS_CLS_PRIN_YN	 ,	PST_FIELD(pList, bus_cls_prin_yn	 ));
				nRet = TMaxFBGetF(DEPR_TIME_PRIN_YN	 ,	PST_FIELD(pList, depr_time_prin_yn	 ));
				nRet = TMaxFBGetF(SATS_NO_PRIN_YN	 ,	PST_FIELD(pList, sats_no_prin_yn	 ));
				nRet = TMaxFBGetF(DRTN_CD			 ,	PST_FIELD(pList, drtn_cd			 ));

				/// [티머니고속] 승차권 발권 - 경유지 정보 set memory
				CPubTckTmExpMem::GetInstance()->m_vtResThruList.push_back(*pList);
			}
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		/// [티머니고속] 
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_readthrutrml_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 승차권 발권(경유지 정보 조회), 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "tissu_trml_no"		,	pList->tissu_trml_no			);
			LOG_WRITE("%30s - (%s) ", "depr_trml_no"		,	pList->depr_trml_no				);
			LOG_WRITE("%30s - (%s) ", "arvl_trml_no"		,	pList->arvl_trml_no				);
			LOG_WRITE("%30s - (%s) ", "alcn_depr_trml_no"	,	pList->alcn_depr_trml_no		);
			LOG_WRITE("%30s - (%s) ", "alcn_arvl_trml_no"	,	pList->alcn_arvl_trml_no		);
			LOG_WRITE("%30s - (%s) ", "alcn_rot_no"			,	pList->alcn_rot_no				);
			LOG_WRITE("%30s - (%s) ", "stt_dt"				,	pList->stt_dt					);
			LOG_WRITE("%30s - (%s) ", "end_dt"				,	pList->end_dt					);
			LOG_WRITE("%30s - (%d) ", "depr_thru_seq"		,	*(int *)pList->depr_thru_seq	);
			LOG_WRITE("%30s - (%d) ", "arvl_thru_seq"		,	*(int *)pList->arvl_thru_seq	);
			LOG_WRITE("%30s - (%s) ", "prmm_sats_sta_val"	,	pList->prmm_sats_sta_val		);
			LOG_WRITE("%30s - (%s) ", "hspd_sats_sta_val"	,	pList->hspd_sats_sta_val		);
			LOG_WRITE("%30s - (%s) ", "psrm_sats_sta_val"	,	pList->psrm_sats_sta_val		);
			LOG_WRITE("%30s - (%s) ", "sats_asgt_val"		,	pList->sats_asgt_val			);
			LOG_WRITE("%30s - (%d) ", "thru_dist"			,	*(int *)pList->thru_dist		);
			LOG_WRITE("%30s - (%d) ", "thru_drtm"			,	*(int *)pList->thru_drtm		);
			LOG_WRITE("%30s - (%s) ", "cacm_nm_prin_yn"		,	pList->cacm_nm_prin_yn			);
			LOG_WRITE("%30s - (%s) ", "bus_cls_prin_yn"		,	pList->bus_cls_prin_yn			);
			LOG_WRITE("%30s - (%s) ", "depr_time_prin_yn"	,	pList->depr_time_prin_yn		);
			LOG_WRITE("%30s - (%s) ", "sats_no_prin_yn"		,	pList->sats_no_prin_yn			);
			LOG_WRITE("%30s - (%s) ", "drtn_cd"				,	pList->drtn_cd					);
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_ReadMrs
 * @details		[티머니고속] 예매조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_ReadMrs(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nCount1;
	char*	pService = "TK_ReadMrs";
	pstk_tm_read_mrs_t pSPacket;
	prtk_tm_read_mrs_t pRPacket;

	nRet = nTimeout = nCount = nCount1 = 0;

	LOG_OPEN();
	LOG_WRITE(" 예매조회_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_read_mrs_t) pData;
	pRPacket = (prtk_tm_read_mrs_t) retBuf;

	/// 티머니고속 - zero memory
 	CMrnpTmExpMem::GetInstance()->m_vtMrnpList.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"	,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"	,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"	,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"	,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","REQ_DVS"			,	pSPacket->req_dvs			);
		LOG_WRITE("%30s - (%s) ","REQ_DVS_CD"		,	pSPacket->req_dvs_cd		);
#if (_KTC_CERTIFY_ > 0)
		LOG_WRITE("%30s - (%s) ","CARD_NO"			,	pSPacket->card_no			);
#endif
		LOG_WRITE("%30s - (%s) ","ADTN_CPN_NO"		,	pSPacket->adtn_cpn_no		);
		LOG_WRITE("%30s - (%s) ","ADTN_PRD_AUTH_NO"	,	pSPacket->adtn_prd_auth_no	);
		LOG_WRITE("%30s - (%s) ","MRSP_MBPH_NO"		,	pSPacket->mrsp_mbph_no		);
		LOG_WRITE("%30s - (%s) ","MRSP_BRDT"		,	pSPacket->mrsp_brdt			);
		LOG_WRITE("%30s - (%s) ","MRS_MRNP_NO"		,	pSPacket->mrs_mrnp_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_DT"			,	pSPacket->tissu_dt			);
		LOG_WRITE("%30s - (%s) ","TISSU_TRML_NO"	,	pSPacket->tissu_trml_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_WND_NO"		,	pSPacket->tissu_wnd_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_SNO"		,	pSPacket->tissu_sno			);
		LOG_WRITE("%30s - (%s) ","LNG_CD"			,	pSPacket->lng_cd			);
		LOG_WRITE("%30s - (%s) ","DEPR_DT"			,	pSPacket->depr_dt			);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS		,	pSPacket->req_pgm_dvs		);
		nRet = TMaxFBPut(REQ_TRML_NO		,	pSPacket->req_trml_no		);
		nRet = TMaxFBPut(REQ_WND_NO			,	pSPacket->req_wnd_no		);
		nRet = TMaxFBPut(REQ_USER_NO		,	pSPacket->req_user_no		);

		nRet = TMaxFBPut(REQ_DVS			,	pSPacket->req_dvs			);
		nRet = TMaxFBPut(REQ_DVS_CD			,	pSPacket->req_dvs_cd		);
		nRet = TMaxFBPut(CARD_NO			,	pSPacket->card_no			);
		nRet = TMaxFBPut(ADTN_CPN_NO		,	pSPacket->adtn_cpn_no		);
		nRet = TMaxFBPut(ADTN_PRD_AUTH_NO	,	pSPacket->adtn_prd_auth_no	);
		nRet = TMaxFBPut(MRSP_MBPH_NO		,	pSPacket->mrsp_mbph_no		);
		nRet = TMaxFBPut(MRSP_BRDT			,	pSPacket->mrsp_brdt			);
		nRet = TMaxFBPut(MRS_MRNP_NO		,	pSPacket->mrs_mrnp_no		);
		nRet = TMaxFBPut(TISSU_DT			,	pSPacket->tissu_dt			);
		nRet = TMaxFBPut(TISSU_TRML_NO		,	pSPacket->tissu_trml_no		);
		nRet = TMaxFBPut(TISSU_WND_NO		,	pSPacket->tissu_wnd_no		);
		nRet = TMaxFBPut(TISSU_SNO			,	pSPacket->tissu_sno			);
		nRet = TMaxFBPut(LNG_CD				,	pSPacket->lng_cd			);
		nRet = TMaxFBPut(DEPR_DT			,	pSPacket->depr_dt			);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		::CopyMemory(CMrnpTmExpMem::GetInstance()->Base.rsp_cd, pRPacket->rsp_cd, sizeof(CMrnpTmExpMem::GetInstance()->Base.rsp_cd));

		/// 정보 수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			/// 예매조회 List
			pRPacket->pList = new rtk_tm_read_mrs_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_read_mrs_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				int k, cnt;
				prtk_tm_read_mrs_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(MRS_MRNP_NO			,	PST_FIELD(pList, mrs_mrnp_no			));
				nRet = TMaxFBGetF(MRS_MRNP_SNO			,	PST_FIELD(pList, mrs_mrnp_sno			));
				nRet = TMaxFBGetF(MRS_MRNP_DT			,	PST_FIELD(pList, mrs_mrnp_dt			));
				nRet = TMaxFBGetF(MRS_MRNP_TIME			,	PST_FIELD(pList, mrs_mrnp_time			));
				nRet = TMaxFBGetF(ALCN_DEPR_TRML_NO		,	PST_FIELD(pList, alcn_depr_trml_no		));
				nRet = TMaxFBGetF(ALCN_ARVL_TRML_NO		,	PST_FIELD(pList, alcn_arvl_trml_no		));
				nRet = TMaxFBGetF(DEPR_TRML_NO			,	PST_FIELD(pList, depr_trml_no			));
				nRet = TMaxFBGetF(ARVL_TRML_NO			,	PST_FIELD(pList, arvl_trml_no			));
				nRet = TMaxFBGetF(BUS_CLS_CD			,	PST_FIELD(pList, bus_cls_cd				));
				nRet = TMaxFBGetF(CACM_CD				,	PST_FIELD(pList, cacm_cd				));
				nRet = TMaxFBGetF(HSPD_CTY_DVS_CD		,	PST_FIELD(pList, hspd_cty_dvs_cd		));
				nRet = TMaxFBGetF(ALCN_DEPR_TIME		,	PST_FIELD(pList, alcn_depr_time			));
				nRet = TMaxFBGetF(DEPR_DT				,	PST_FIELD(pList, depr_dt				));
				nRet = TMaxFBGetF(DEPR_TIME				,	PST_FIELD(pList, depr_time				));
				nRet = TMaxFBGetF(MRS_AMT				,	PST_FIELD(pList, mrs_amt				));
				nRet = TMaxFBGetF(CARD_NO				,	PST_FIELD(pList, card_no				));
#ifdef __KTC_PROGRAM__
				::ZeroMemory(pList->card_no, sizeof(pList->card_no));
#endif
				nRet = TMaxFBGetF(ADTN_CPN_NO			,	PST_FIELD(pList, adtn_cpn_no			));
				nRet = TMaxFBGetF(ADTN_PRD_AUTH_NO		,	PST_FIELD(pList, adtn_prd_auth_no		));
				nRet = TMaxFBGetF(ROT_RDHM_NO_VAL		,	PST_FIELD(pList, rot_rdhm_no_val		));
				nRet = TMaxFBGetF(BUY_CMPY_CD			,	PST_FIELD(pList, buy_cmpy_cd			));
				
				//nRet = TMaxFBGetF(TCK_KND_STRING		,	PST_FIELD(pList, tck_knd_string			));
				TMaxGetConvChar(TCK_KND_STRING			,	PST_FIELD(pList, tck_knd_string			));

				nRet = TMaxFBGetF(SATS_NO_STRING		,	PST_FIELD(pList, sats_no_string			));
				nRet = TMaxFBGetF(TISSU_CHNL_DVS_CD		,	PST_FIELD(pList, tissu_chnl_dvs_cd		));
				nRet = TMaxFBGetF(PUB_CHNL_DVS_CD		,	PST_FIELD(pList, pub_chnl_dvs_cd		));
				nRet = TMaxFBGetF(PYN_DVS_CD			,	PST_FIELD(pList, pyn_dvs_cd				));
				nRet = TMaxFBGetF(TISSU_STA_CD			,	PST_FIELD(pList, tissu_sta_cd			));
				nRet = TMaxFBGetF(PYN_DTL_CD			,	PST_FIELD(pList, pyn_dtl_cd				));	 /// *
				
				nRet = TMaxFBGetF(MRS_NUM				,	PST_FIELD(pList, mrs_num	));
				::CopyMemory(&cnt, pList->mrs_num, 4);
				for(k = 0; k < cnt; k++)
				{
					nRet = TMaxFBGetF(TCK_KND_CD	,	pList->mrs_info[k].tck_knd_cd	);
					nRet = TMaxFBGetF(SATS_NO		,	pList->mrs_info[k].sats_no		);
					nRet = TMaxFBGetF(TISSU_AMT		,	pList->mrs_info[k].tissu_amt	);
					nRet = TMaxFBGetF(RY_STA_CD		,	pList->mrs_info[k].ry_sta_cd	);
					nRet = TMaxFBGetF(DC_KND_CD		,	pList->mrs_info[k].dc_knd_cd	);
				}
				
				/// [티머니고속] 예매발권 - 예매내역 set memory
				CMrnpTmExpMem::GetInstance()->m_vtMrnpList.push_back(*pList);
			}
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		/// [티머니고속] 
		for(i = 0; i < nCount; i++)
		{
			int k, cnt;
			prtk_tm_read_mrs_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 예매조회, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "MRS_MRNP_NO"			,	pList->mrs_mrnp_no			);
			LOG_WRITE("%30s - (%s) ", "MRS_MRNP_SNO"		,	pList->mrs_mrnp_sno			);
			LOG_WRITE("%30s - (%s) ", "MRS_MRNP_DT"			,	pList->mrs_mrnp_dt			);
			LOG_WRITE("%30s - (%s) ", "MRS_MRNP_TIME"		,	pList->mrs_mrnp_time		);
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO"	,	pList->alcn_depr_trml_no	);
			LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO"	,	pList->alcn_arvl_trml_no	);
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"		,	pList->depr_trml_no			);
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"		,	pList->arvl_trml_no			);
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD"			,	pList->bus_cls_cd			);
			LOG_WRITE("%30s - (%s) ", "CACM_CD"				,	pList->cacm_cd				);
			LOG_WRITE("%30s - (%s) ", "HSPD_CTY_DVS_CD"		,	pList->hspd_cty_dvs_cd		);
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TIME"		,	pList->alcn_depr_time		);
			LOG_WRITE("%30s - (%s) ", "DEPR_DT"				,	pList->depr_dt				);
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME"			,	pList->depr_time			);
			LOG_WRITE("%30s - (%d) ", "MRS_AMT"				,	*(int *)pList->mrs_amt		);
#if (_KTC_CERTIFY_ > 0)
			LOG_WRITE("%30s - (%s) ", "CARD_NO"				,	pList->card_no				);
#endif
			LOG_WRITE("%30s - (%s) ", "ADTN_CPN_NO"			,	pList->adtn_cpn_no			);
			LOG_WRITE("%30s - (%s) ", "ADTN_PRD_AUTH_NO"	,	pList->adtn_prd_auth_no		);
			LOG_WRITE("%30s - (%s) ", "ROT_RDHM_NO_VAL"		,	pList->rot_rdhm_no_val		);
			LOG_WRITE("%30s - (%s) ", "BUY_CMPY_CD"			,	pList->buy_cmpy_cd			);
			LOG_WRITE("%30s - (%s) ", "TCK_KND_STRING"		,	pList->tck_knd_string		);
			LOG_WRITE("%30s - (%s) ", "SATS_NO_STRING"		,	pList->sats_no_string		);
			LOG_WRITE("%30s - (%s) ", "TISSU_CHNL_DVS_CD"	,	pList->tissu_chnl_dvs_cd	);
			LOG_WRITE("%30s - (%s) ", "PUB_CHNL_DVS_CD"		,	pList->pub_chnl_dvs_cd		);
			LOG_WRITE("%30s - (%s) ", "PYN_DVS_CD"			,	pList->pyn_dvs_cd			);
			LOG_WRITE("%30s - (%s) ", "TISSU_STA_CD"		,	pList->tissu_sta_cd			);
			LOG_WRITE("%30s - (%s) ", "PYN_DTL_CD"			,	pList->pyn_dtl_cd			);
			LOG_WRITE("%30s - (%d) ", "MRS_NUM"				,	*(int *)pList->mrs_num		);

			::CopyMemory(&cnt, pList->mrs_num, 4);
			for(k = 0; k < cnt; k++)
			{
				LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"	,	pList->mrs_info[k].tck_knd_cd	);
				LOG_WRITE("%30s - (%s) ", "SATS_NO"		,	pList->mrs_info[k].sats_no		);
				LOG_WRITE("%30s - (Num=%d) ", "TISSU_AMT",	*(int *)pList->mrs_info[k].tissu_amt);
				LOG_WRITE("%30s - (%s) ", "RY_STA_CD"	,	pList->mrs_info[k].ry_sta_cd	);
				LOG_WRITE("%30s - (%s) ", "DC_KND_CD"	,	pList->mrs_info[k].dc_knd_cd	);
			}
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_KtcReadMrs
 * @details		[티머니고속] KTC 예매조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_KtcReadMrs(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nCount1;
	char*	pService = "TK_KtcReadMrs";
	pstk_tm_read_mrs_ktc_t pSPacket;
	prtk_tm_read_mrs_ktc_t pRPacket;

	nRet = nTimeout = nCount = nCount1 = 0;

	LOG_OPEN();
	LOG_WRITE(" 예매조회_KTC_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_read_mrs_ktc_t) pData;
	pRPacket = (prtk_tm_read_mrs_ktc_t) retBuf;

	/// 티머니고속 - zero memory
 	CMrnpTmExpMem::GetInstance()->m_vtMrnpKtcList.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"		,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"		,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"		,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"		,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","REQ_DVS"			,	pSPacket->req_dvs			);
		LOG_WRITE("%30s - (%s) ","REQ_DVS_CD"		,	pSPacket->req_dvs_cd		);
		LOG_WRITE("%30s - (%s) ","TRD_DVS_CD"		,	pSPacket->trd_dvs_cd		);
		
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ","ENC_DTA"			,	pSPacket->enc_dta			);
		LOG_WRITE("%30s - (%s) ","EMV_DTA"			,	pSPacket->emv_dta			);
#endif
		LOG_WRITE("%30s - (%s) ","ADTN_CPN_NO"		,	pSPacket->adtn_cpn_no		);
		LOG_WRITE("%30s - (%s) ","ADTN_PRD_AUTH_NO"	,	pSPacket->adtn_prd_auth_no	);
		LOG_WRITE("%30s - (%s) ","MRSP_MBPH_NO"		,	pSPacket->mrsp_mbph_no		);
		LOG_WRITE("%30s - (%s) ","MRSP_BRDT"		,	pSPacket->mrsp_brdt			);
		LOG_WRITE("%30s - (%s) ","MRS_MRNP_NO"		,	pSPacket->mrs_mrnp_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_DT"			,	pSPacket->tissu_dt			);
		LOG_WRITE("%30s - (%s) ","TISSU_TRML_NO"	,	pSPacket->tissu_trml_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_WND_NO"		,	pSPacket->tissu_wnd_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_SNO"		,	pSPacket->tissu_sno			);
		LOG_WRITE("%30s - (%s) ","LNG_CD"			,	pSPacket->lng_cd			);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS		,	pSPacket->req_pgm_dvs		);
		nRet = TMaxFBPut(REQ_TRML_NO		,	pSPacket->req_trml_no		);
		nRet = TMaxFBPut(REQ_WND_NO			,	pSPacket->req_wnd_no		);
		nRet = TMaxFBPut(REQ_USER_NO		,	pSPacket->req_user_no		);

		nRet = TMaxFBPut(REQ_DVS			,	pSPacket->req_dvs			);
		nRet = TMaxFBPut(REQ_DVS_CD			,	pSPacket->req_dvs_cd		);
		nRet = TMaxFBPut(TRD_DVS_CD			,	pSPacket->trd_dvs_cd		);
		nRet = TMaxFBPut(ENC_DTA			,	pSPacket->enc_dta			);
		nRet = TMaxFBPut(EMV_DTA			,	pSPacket->emv_dta			);
		nRet = TMaxFBPut(ADTN_CPN_NO		,	pSPacket->adtn_cpn_no		);
		nRet = TMaxFBPut(ADTN_PRD_AUTH_NO	,	pSPacket->adtn_prd_auth_no	);
		nRet = TMaxFBPut(MRSP_MBPH_NO		,	pSPacket->mrsp_mbph_no		);
		nRet = TMaxFBPut(MRSP_BRDT			,	pSPacket->mrsp_brdt			);
		nRet = TMaxFBPut(MRS_MRNP_NO		,	pSPacket->mrs_mrnp_no		);
		nRet = TMaxFBPut(TISSU_DT			,	pSPacket->tissu_dt			);
		nRet = TMaxFBPut(TISSU_TRML_NO		,	pSPacket->tissu_trml_no		);
		nRet = TMaxFBPut(TISSU_WND_NO		,	pSPacket->tissu_wnd_no		);
		nRet = TMaxFBPut(TISSU_SNO			,	pSPacket->tissu_sno			);
		nRet = TMaxFBPut(LNG_CD				,	pSPacket->lng_cd			);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		::CopyMemory(CMrnpTmExpMem::GetInstance()->Base.rsp_cd, pRPacket->rsp_cd, sizeof(CMrnpTmExpMem::GetInstance()->Base.rsp_cd));

		/// 정보 수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			/// 승차권 발권(현영_KTC) List
			pRPacket->pList = new rtk_tm_read_mrs_ktc_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_read_mrs_ktc_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				int k, cnt;
				prtk_tm_read_mrs_ktc_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(MRS_MRNP_NO			,	PST_FIELD(pList, mrs_mrnp_no			));
				nRet = TMaxFBGetF(MRS_MRNP_SNO			,	PST_FIELD(pList, mrs_mrnp_sno			));
				nRet = TMaxFBGetF(MRS_MRNP_DT			,	PST_FIELD(pList, mrs_mrnp_dt			));
				nRet = TMaxFBGetF(MRS_MRNP_TIME			,	PST_FIELD(pList, mrs_mrnp_time			));
				nRet = TMaxFBGetF(CASH_RECP_AMT			,	PST_FIELD(pList, cash_recp_amt			));
				// 사용안함 (서버담당자 확인했음)
				//nRet = TMaxFBGetF(TISSU_NO				,	PST_FIELD(pList, tissu_no				));
				//nRet = TMaxFBGetF(TCK_KND_CD			,	PST_FIELD(pList, tck_knd_cd				));
				// ~사용안함 (서버담당자 확인했음)
				nRet = TMaxFBGetF(ALCN_DEPR_TRML_NO		,	PST_FIELD(pList, alcn_depr_trml_no		));
				nRet = TMaxFBGetF(ALCN_ARVL_TRML_NO		,	PST_FIELD(pList, alcn_arvl_trml_no		));
				nRet = TMaxFBGetF(DEPR_TRML_NO			,	PST_FIELD(pList, depr_trml_no			));
				nRet = TMaxFBGetF(ARVL_TRML_NO			,	PST_FIELD(pList, arvl_trml_no			));
				nRet = TMaxFBGetF(BUS_CLS_CD			,	PST_FIELD(pList, bus_cls_cd				));
				nRet = TMaxFBGetF(CACM_CD				,	PST_FIELD(pList, cacm_cd				));
				nRet = TMaxFBGetF(HSPD_CTY_DVS_CD		,	PST_FIELD(pList, hspd_cty_dvs_cd		));
				nRet = TMaxFBGetF(ALCN_DEPR_TIME		,	PST_FIELD(pList, alcn_depr_time			));
				nRet = TMaxFBGetF(DEPR_DT				,	PST_FIELD(pList, depr_dt				));
				nRet = TMaxFBGetF(DEPR_TIME				,	PST_FIELD(pList, depr_time				));
				nRet = TMaxFBGetF(MRS_AMT				,	PST_FIELD(pList, mrs_amt				));
				nRet = TMaxFBGetF(CARD_NO				,	PST_FIELD(pList, card_no				));
#ifdef __KTC_PROGRAM__
				::ZeroMemory(pList->card_no, sizeof(pList->card_no));
#endif
				nRet = TMaxFBGetF(ADTN_CPN_NO			,	PST_FIELD(pList, adtn_cpn_no			));
				nRet = TMaxFBGetF(ADTN_PRD_AUTH_NO		,	PST_FIELD(pList, adtn_prd_auth_no		));
				nRet = TMaxFBGetF(ROT_RDHM_NO_VAL		,	PST_FIELD(pList, rot_rdhm_no_val		));
				nRet = TMaxFBGetF(BUY_CMPY_CD			,	PST_FIELD(pList, buy_cmpy_cd			));
				nRet = TMaxFBGetF(TCK_KND_STRING		,	PST_FIELD(pList, tck_knd_string			));
				nRet = TMaxFBGetF(SATS_NO_STRING		,	PST_FIELD(pList, sats_no_string			));
				nRet = TMaxFBGetF(TISSU_CHNL_DVS_CD		,	PST_FIELD(pList, tissu_chnl_dvs_cd		));
				nRet = TMaxFBGetF(PUB_CHNL_DVS_CD		,	PST_FIELD(pList, pub_chnl_dvs_cd		));
				nRet = TMaxFBGetF(PYN_DVS_CD			,	PST_FIELD(pList, pyn_dvs_cd				));
				nRet = TMaxFBGetF(TISSU_STA_CD			,	PST_FIELD(pList, tissu_sta_cd			));
				nRet = TMaxFBGetF(PYN_DTL_CD			,	PST_FIELD(pList, pyn_dtl_cd				));	 /// *
				
				/// test code
				///pList->pyn_dtl_cd[0] = 'e';

				nRet = TMaxFBGetF(MRS_NUM				,	PST_FIELD(pList, mrs_num	));
				::CopyMemory(&cnt, pList->mrs_num, 4);
				for(k = 0; k < cnt; k++)
				{
					nRet = TMaxFBGetTu(TCK_KND_CD	,	pList->mrs_info[k].tck_knd_cd	,	k);
					nRet = TMaxFBGetTu(SATS_NO		,	pList->mrs_info[k].sats_no		,	k);
					nRet = TMaxFBGetTu(TISSU_AMT	,	pList->mrs_info[k].tissu_amt	,	k);
					nRet = TMaxFBGetTu(RY_STA_CD	,	pList->mrs_info[k].ry_sta_cd	,	k);
					nRet = TMaxFBGetTu(DC_KND_CD	,	pList->mrs_info[k].dc_knd_cd	,	k);
				}
				
				/// pList->tissu_chnl_dvs_cd == '3' 모바일티켓 발행으로 처리해야 한다.

				/// [티머니고속] 예매발권 - 예매내역 set memory
				CMrnpTmExpMem::GetInstance()->m_vtMrnpKtcList.push_back(*pList);
			}
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		/// [티머니고속] 
		for(i = 0; i < nCount; i++)
		{
			int k, cnt;
			prtk_tm_read_mrs_ktc_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 예매조회, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "MRS_MRNP_NO"			,	pList->mrs_mrnp_no			);
			LOG_WRITE("%30s - (%s) ", "MRS_MRNP_SNO"		,	pList->mrs_mrnp_sno			);
			LOG_WRITE("%30s - (%s) ", "MRS_MRNP_DT"			,	pList->mrs_mrnp_dt			);
			LOG_WRITE("%30s - (%s) ", "MRS_MRNP_TIME"		,	pList->mrs_mrnp_time		);
			LOG_WRITE("%30s - (Num=%d) ", "CASH_RECP_AMT"	,	*(int *)pList->cash_recp_amt);
			LOG_WRITE("%30s - (%s) ", "TISSU_NO"			,	pList->tissu_no				);
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"			,	pList->tck_knd_cd			);
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO"	,	pList->alcn_depr_trml_no	);
			LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO"	,	pList->alcn_arvl_trml_no	);
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"		,	pList->depr_trml_no			);
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"		,	pList->arvl_trml_no			);
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD"			,	pList->bus_cls_cd			);
			LOG_WRITE("%30s - (%s) ", "CACM_CD"				,	pList->cacm_cd				);
			LOG_WRITE("%30s - (%s) ", "HSPD_CTY_DVS_CD"		,	pList->hspd_cty_dvs_cd		);
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TIME"		,	pList->alcn_depr_time		);
			LOG_WRITE("%30s - (%s) ", "DEPR_DT"				,	pList->depr_dt				);
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME"			,	pList->depr_time			);
			LOG_WRITE("%30s - (Num=%d) ", "MRS_AMT"			,	*(int *)pList->mrs_amt		);
#if (_KTC_CERTIFY_ > 0)
			LOG_WRITE("%30s - (%s) ", "CARD_NO"				,	pList->card_no				);
#endif
			LOG_WRITE("%30s - (%s) ", "ADTN_CPN_NO"			,	pList->adtn_cpn_no			);
			LOG_WRITE("%30s - (%s) ", "ADTN_PRD_AUTH_NO"	,	pList->adtn_prd_auth_no		);
			LOG_WRITE("%30s - (%s) ", "ROT_RDHM_NO_VAL"		,	pList->rot_rdhm_no_val		);
			LOG_WRITE("%30s - (%s) ", "BUY_CMPY_CD"			,	pList->buy_cmpy_cd			);
			LOG_WRITE("%30s - (%s) ", "TCK_KND_STRING"		,	pList->tck_knd_string		);
			LOG_WRITE("%30s - (%s) ", "SATS_NO_STRING"		,	pList->sats_no_string		);
			LOG_WRITE("%30s - (%s) ", "TISSU_CHNL_DVS_CD"	,	pList->tissu_chnl_dvs_cd	);
			LOG_WRITE("%30s - (%s) ", "PUB_CHNL_DVS_CD"		,	pList->pub_chnl_dvs_cd		);
			LOG_WRITE("%30s - (%s) ", "PYN_DVS_CD"			,	pList->pyn_dvs_cd			);
			LOG_WRITE("%30s - (%s) ", "TISSU_STA_CD"		,	pList->tissu_sta_cd			);
			LOG_WRITE("%30s - (%s) ", "PYN_DTL_CD"			,	pList->pyn_dtl_cd			);
			LOG_WRITE("%30s - (Num=%d) ", "MRS_NUM"			,	*(int *)pList->mrs_num		);

			::CopyMemory(&cnt, pList->mrs_num, 4);
			for(k = 0; k < cnt; k++)
			{
				LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"	,	pList->mrs_info[k].tck_knd_cd	);
				LOG_WRITE("%30s - (%s) ", "SATS_NO"		,	pList->mrs_info[k].sats_no		);
				LOG_WRITE("%30s - (Num=%d) ", "TISSU_AMT"	,	*(int *)pList->mrs_info[k].tissu_amt);
				LOG_WRITE("%30s - (%s) ", "RY_STA_CD"	,	pList->mrs_info[k].ry_sta_cd	);
				LOG_WRITE("%30s - (%s) ", "DC_KND_CD"	,	pList->mrs_info[k].dc_knd_cd	);
			}
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_PubMrs
 * @details		[티머니고속] 예매발권 : 발행상태값이 '2'일때 사용
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_PubMrs(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nCount1;
	char*	pService = "TK_PubMrs";
	pstk_tm_pub_mrs_t pSPacket;
	prtk_tm_pub_mrs_t pRPacket;

	nRet = nTimeout = nCount = nCount1 = 0;

	LOG_OPEN();
	LOG_WRITE(" 예매발권_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_pub_mrs_t) pData;
	pRPacket = (prtk_tm_pub_mrs_t) retBuf;

	/// 티머니고속 - zero memory
 	CMrnpTmExpMem::GetInstance()->m_vtResComplete.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"		,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"		,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"		,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"		,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","REQ_DVS_CD"		,	pSPacket->req_dvs_cd		);
		LOG_WRITE("%30s - (%s) ","MRS_MRNP_NO"		,	pSPacket->mrs_mrnp_no		);
		LOG_WRITE("%30s - (%s) ","MRS_MRNP_SNO"		,	pSPacket->mrs_mrnp_sno		);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS		,	pSPacket->req_pgm_dvs		);
		nRet = TMaxFBPut(REQ_TRML_NO		,	pSPacket->req_trml_no		);
		nRet = TMaxFBPut(REQ_WND_NO			,	pSPacket->req_wnd_no		);
		nRet = TMaxFBPut(REQ_USER_NO		,	pSPacket->req_user_no		);

		nRet = TMaxFBPut(REQ_DVS_CD			,	pSPacket->req_dvs_cd		);
		nRet = TMaxFBPut(MRS_MRNP_NO		,	pSPacket->mrs_mrnp_no		);
		nRet = TMaxFBPut(MRS_MRNP_SNO		,	pSPacket->mrs_mrnp_sno		);
	}

	m_nTPCallFlag = TPNOFLAGS;
	TMaxTransaction(TMAX_TR_BEGIN);


	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			TMaxTransaction(TMAX_TR_ROLLBACK);

			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		TMaxTransaction(TMAX_TR_COMMIT);

		::CopyMemory(CMrnpTmExpMem::GetInstance()->Base.rsp_cd, pRPacket->rsp_cd, sizeof(CMrnpTmExpMem::GetInstance()->Base.rsp_cd));

		nRet = TMaxFBGet(PYN_DTL_CD		, PST_FIELD(pRPacket, pyn_dtl_cd	));
		nRet = TMaxFBGet(CARD_NO		, PST_FIELD(pRPacket, card_no		));
		nRet = TMaxFBGet(CARD_APRV_NO	, PST_FIELD(pRPacket, card_aprv_no	));
		nRet = TMaxFBGet(APRV_AMT		, PST_FIELD(pRPacket, aprv_amt		));
		nRet = TMaxFBGet(FRC_CMRT		, PST_FIELD(pRPacket, frc_cmrt		));

		::CopyMemory(CMrnpTmExpMem::GetInstance()->Base.pyn_dtl_cd	, pRPacket->pyn_dtl_cd	, sizeof(CMrnpTmExpMem::GetInstance()->Base.pyn_dtl_cd));
		::CopyMemory(CMrnpTmExpMem::GetInstance()->Base.card_no		, pRPacket->card_no		, sizeof(CMrnpTmExpMem::GetInstance()->Base.card_no));
		::CopyMemory(CMrnpTmExpMem::GetInstance()->Base.card_aprv_no, pRPacket->card_aprv_no, sizeof(CMrnpTmExpMem::GetInstance()->Base.card_aprv_no));
		::CopyMemory(CMrnpTmExpMem::GetInstance()->Base.aprv_amt	, pRPacket->aprv_amt	, sizeof(CMrnpTmExpMem::GetInstance()->Base.aprv_amt));
		::CopyMemory(CMrnpTmExpMem::GetInstance()->Base.frc_cmrt	, pRPacket->frc_cmrt	, sizeof(CMrnpTmExpMem::GetInstance()->Base.frc_cmrt));

		/// 정보 수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			/// 승차권 발권(현영_KTC) List
			pRPacket->pList = new rtk_tm_pub_mrs_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_pub_mrs_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				//int k, cnt;
				prtk_tm_pub_mrs_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(MRS_MRNP_NO			,	PST_FIELD(pList, mrs_mrnp_no			));
				nRet = TMaxFBGetF(MRS_MRNP_SNO			,	PST_FIELD(pList, mrs_mrnp_sno			));
				nRet = TMaxFBGetF(DEPR_DT				,	PST_FIELD(pList, depr_dt				));
				nRet = TMaxFBGetF(DEPR_TIME				,	PST_FIELD(pList, depr_time				));
				nRet = TMaxFBGetF(ALCN_DEPR_TRML_NO		,	PST_FIELD(pList, alcn_depr_trml_no		));
				nRet = TMaxFBGetF(ALCN_ARVL_TRML_NO		,	PST_FIELD(pList, alcn_arvl_trml_no		));
				nRet = TMaxFBGetF(DEPR_TRML_NO			,	PST_FIELD(pList, depr_trml_no			));
				nRet = TMaxFBGetF(ARVL_TRML_NO			,	PST_FIELD(pList, arvl_trml_no			));
				nRet = TMaxFBGetF(ALCN_ROT_NO			,	PST_FIELD(pList, alcn_rot_no			));
				nRet = TMaxFBGetF(BUS_CLS_CD			,	PST_FIELD(pList, bus_cls_cd				));
				nRet = TMaxFBGetF(CACM_CD				,	PST_FIELD(pList, cacm_cd				));
				nRet = TMaxFBGetF(ROT_RDHM_NO_VAL		,	PST_FIELD(pList, rot_rdhm_no_val		));
				nRet = TMaxFBGetF(BUS_OPRN_DIST			,	PST_FIELD(pList, bus_oprn_dist			));
				nRet = TMaxFBGetF(TAKE_DRTM				,	PST_FIELD(pList, take_drtm				));
				nRet = TMaxFBGetF(CACM_NM_PRIN_YN		,	PST_FIELD(pList, cacm_nm_prin_yn		));
				nRet = TMaxFBGetF(BUS_CLS_PRIN_YN		,	PST_FIELD(pList, bus_cls_prin_yn		));
				nRet = TMaxFBGetF(DEPR_TIME_PRIN_YN		,	PST_FIELD(pList, depr_time_prin_yn		));
				nRet = TMaxFBGetF(SATS_NO_PRIN_YN		,	PST_FIELD(pList, sats_no_prin_yn		));
				nRet = TMaxFBGetF(ALCN_DVS_CD			,	PST_FIELD(pList, alcn_dvs_cd			));
				nRet = TMaxFBGetF(TISSU_DT				,	PST_FIELD(pList, tissu_dt				));
				nRet = TMaxFBGetF(TISSU_TIME			,	PST_FIELD(pList, tissu_time				));
				nRet = TMaxFBGetF(TISSU_TRML_NO			,	PST_FIELD(pList, tissu_trml_no			));
				nRet = TMaxFBGetF(TISSU_WND_NO			,	PST_FIELD(pList, tissu_wnd_no			));
				nRet = TMaxFBGetF(PUB_USER_NO			,	PST_FIELD(pList, pub_user_no			));
				nRet = TMaxFBGetF(TISSU_SNO				,	PST_FIELD(pList, tissu_sno				));
				nRet = TMaxFBGetF(INHR_NO				,	PST_FIELD(pList, inhr_no				));
				nRet = TMaxFBGetF(INVS_NO				,	PST_FIELD(pList, invs_no				));
				nRet = TMaxFBGetF(SATS_NO				,	PST_FIELD(pList, sats_no				));
				nRet = TMaxFBGetF(TCK_KND_CD			,	PST_FIELD(pList, tck_knd_cd				));
				nRet = TMaxFBGetF(FEE_KND_CD			,	PST_FIELD(pList, fee_knd_cd				));
				nRet = TMaxFBGetF(TISSU_FEE				,	PST_FIELD(pList, tissu_fee				));
				nRet = TMaxFBGetF(OGN_FEE				,	PST_FIELD(pList, ogn_fee				));
				
				/// [티머니고속] 예매발권 - 예매내역 set memory
				CMrnpTmExpMem::GetInstance()->m_vtResComplete.push_back(*pList);
			}
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s - (%s) ", "PYN_DTL_CD"		,	pRPacket->pyn_dtl_cd		);
#if (_KTC_CERTIFY_ > 0)
		LOG_WRITE("%30s - (%s) ", "CARD_NO"			,	pRPacket->card_no			);
#endif
		LOG_WRITE("%30s - (%s) ", "CARD_APRV_NO"	,	pRPacket->card_aprv_no		);
		LOG_WRITE("%30s - (%s) ", "APRV_AMT"		,	pRPacket->aprv_amt			);
		LOG_WRITE("%30s - (%s) ", "FRC_CMRT"		,	pRPacket->frc_cmrt			);

		/// [티머니고속] 
		for(i = 0; i < nCount; i++)
		{
			//int k, cnt;
			prtk_tm_pub_mrs_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 예매발권, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "MRS_MRNP_NO"			,	pList->mrs_mrnp_no			);
			LOG_WRITE("%30s - (%s) ", "MRS_MRNP_SNO"		,	pList->mrs_mrnp_sno			);
			LOG_WRITE("%30s - (%s) ", "DEPR_DT"				,	pList->depr_dt				);
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME"			,	pList->depr_time			);
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO"	,	pList->alcn_depr_trml_no	);
			LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO"	,	pList->alcn_arvl_trml_no	);
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"		,	pList->depr_trml_no			);
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"		,	pList->arvl_trml_no			);
			LOG_WRITE("%30s - (%s) ", "ALCN_ROT_NO"			,	pList->alcn_rot_no			);
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD"			,	pList->bus_cls_cd			);
			LOG_WRITE("%30s - (%s) ", "CACM_CD"				,	pList->cacm_cd				);
			LOG_WRITE("%30s - (%s) ", "ROT_RDHM_NO_VAL"		,	pList->rot_rdhm_no_val		);
			LOG_WRITE("%30s - (%s) ", "BUS_OPRN_DIST"		,	pList->bus_oprn_dist		);
			LOG_WRITE("%30s - (%s) ", "TAKE_DRTM"			,	pList->take_drtm			);
			LOG_WRITE("%30s - (%s) ", "CACM_NM_PRIN_YN"		,	pList->cacm_nm_prin_yn		);
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_PRIN_YN"		,	pList->bus_cls_prin_yn		);
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME_PRIN_YN"	,	pList->depr_time_prin_yn	);
			LOG_WRITE("%30s - (%s) ", "SATS_NO_PRIN_YN"		,	pList->sats_no_prin_yn		);
			LOG_WRITE("%30s - (%s) ", "ALCN_DVS_CD"			,	pList->alcn_dvs_cd			);
			LOG_WRITE("%30s - (%s) ", "TISSU_DT"			,	pList->tissu_dt				);
			LOG_WRITE("%30s - (%s) ", "TISSU_TIME"			,	pList->tissu_time			);
			LOG_WRITE("%30s - (%s) ", "TISSU_TRML_NO"		,	pList->tissu_trml_no		);
			LOG_WRITE("%30s - (%s) ", "TISSU_WND_NO"		,	pList->tissu_wnd_no			);
			LOG_WRITE("%30s - (%s) ", "PUB_USER_NO"			,	pList->pub_user_no			);
			LOG_WRITE("%30s - (%s) ", "TISSU_SNO"			,	pList->tissu_sno			);
			LOG_WRITE("%30s - (%s) ", "INHR_NO"				,	pList->inhr_no				);
			LOG_WRITE("%30s - (%s) ", "INVS_NO"				,	pList->invs_no				);
			LOG_WRITE("%30s - (%s) ", "SATS_NO"				,	pList->sats_no				);
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"			,	pList->tck_knd_cd			);
			LOG_WRITE("%30s - (%s) ", "FEE_KND_CD"			,	pList->fee_knd_cd			);
			LOG_WRITE("%30s - (Number=%d) ", "TISSU_FEE"	,	*(int *)pList->tissu_fee	);
			LOG_WRITE("%30s - (Number=%d) ", "OGN_FEE"		,	*(int *)pList->ogn_fee		);
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_PubMrsMobile
 * @details		[티머니고속] 예매발권(모바일티켓) : 발행상태값이 '3'이 아닐때 사용
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_PubMrsMobile(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nCount1;
//	char*	pService = "TK_RPubHtck";
	char*	pService = "TK_RPubHtckNT";
	pstk_tm_pub_mrs_htck_t pSPacket;
	prtk_tm_pub_mrs_htck_t pRPacket;

	nRet = nTimeout = nCount = nCount1 = 0;

	LOG_OPEN();
	LOG_WRITE(" 예매발권(모바일티켓)_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_pub_mrs_htck_t) pData;
	pRPacket = (prtk_tm_pub_mrs_htck_t) retBuf;

	/// 티머니고속 - zero memory
 	CMrnpTmExpMem::GetInstance()->m_vtResCompleteMobile.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"		,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"		,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"		,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"		,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO"		,	pSPacket->depr_trml_no		);
		LOG_WRITE("%30s - (%s) ","MRS_MRNP_NO"		,	pSPacket->mrs_mrnp_no		);
		LOG_WRITE("%30s - (%s) ","MRS_MRNP_SNO"		,	pSPacket->mrs_mrnp_sno		);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS		,	pSPacket->req_pgm_dvs		);
		nRet = TMaxFBPut(REQ_TRML_NO		,	pSPacket->req_trml_no		);
		nRet = TMaxFBPut(REQ_WND_NO			,	pSPacket->req_wnd_no		);
		nRet = TMaxFBPut(REQ_USER_NO		,	pSPacket->req_user_no		);

		nRet = TMaxFBPut(DEPR_TRML_NO		,	pSPacket->depr_trml_no		);
		nRet = TMaxFBPut(MRS_MRNP_NO		,	pSPacket->mrs_mrnp_no		);
		nRet = TMaxFBPut(MRS_MRNP_SNO		,	pSPacket->mrs_mrnp_sno		);
	}

	m_nTPCallFlag = TPNOFLAGS;
	TMaxTransaction(TMAX_TR_BEGIN);


	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			TMaxTransaction(TMAX_TR_ROLLBACK);

			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
		
		TMaxTransaction(TMAX_TR_COMMIT);

		::CopyMemory(CMrnpTmExpMem::GetInstance()->Base.rsp_cd, pRPacket->rsp_cd, sizeof(CMrnpTmExpMem::GetInstance()->Base.rsp_cd));

		nRet = TMaxFBGet(CARD_NO		, PST_FIELD(pRPacket, card_no		));
		nRet = TMaxFBGet(APRV_AMT		, PST_FIELD(pRPacket, n_aprv_amt	));
		nRet = TMaxFBGet(CARD_APRV_NO	, PST_FIELD(pRPacket, card_aprv_no	));
		nRet = TMaxFBGet(FRC_CMM		, PST_FIELD(pRPacket, n_frc_cmm		));

		::CopyMemory(CMrnpTmExpMem::GetInstance()->Base.card_no		, pRPacket->card_no		, sizeof(CMrnpTmExpMem::GetInstance()->Base.card_no));
		::CopyMemory(CMrnpTmExpMem::GetInstance()->Base.aprv_amt	, pRPacket->n_aprv_amt	, sizeof(CMrnpTmExpMem::GetInstance()->Base.aprv_amt));
		::CopyMemory(CMrnpTmExpMem::GetInstance()->Base.card_aprv_no, pRPacket->card_aprv_no, sizeof(CMrnpTmExpMem::GetInstance()->Base.card_aprv_no));
		::CopyMemory(CMrnpTmExpMem::GetInstance()->Base.frc_cmm		, pRPacket->n_frc_cmm	, sizeof(CMrnpTmExpMem::GetInstance()->Base.frc_cmm));

		/// 정보 수
		nRet = TMaxFBGet(TISSU_NUM	, PST_FIELD(pRPacket, n_tissu_num));
		::CopyMemory(&nCount, pRPacket->n_tissu_num, 4);
		LOG_WRITE("%30s - (%d) ", "TISSU_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			/// 승차권 발권(현영_KTC) List
			pRPacket->pList = new rtk_tm_pub_mrs_htck_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_pub_mrs_htck_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				//int k, cnt;
				prtk_tm_pub_mrs_htck_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(BUS_OPRN_DIST					,	PST_FIELD(pList, bus_oprn_dist					));
				nRet = TMaxFBGetF(ROT_RDHM_NO_VAL				,	PST_FIELD(pList, rot_rdhm_no_val				));
				nRet = TMaxFBGetF(CACM_NM_PRIN_YN				,	PST_FIELD(pList, cacm_nm_prin_yn				));
				nRet = TMaxFBGetF(BUS_CLS_PRIN_YN				,	PST_FIELD(pList, bus_cls_prin_yn				));
				nRet = TMaxFBGetF(DEPR_TIME_PRIN_YN				,	PST_FIELD(pList, depr_time_prin_yn				));
				nRet = TMaxFBGetF(SATS_NO_PRIN_YN				,	PST_FIELD(pList, sats_no_prin_yn				));
				nRet = TMaxFBGetF(DEPR_TRML_ENG_ABRV_NM			,	PST_FIELD(pList, depr_trml_eng_abrv_nm			));
				nRet = TMaxFBGetF(ARVL_TRML_ENG_ABRV_NM			,	PST_FIELD(pList, arvl_trml_eng_abrv_nm			));

				//nRet = TMaxFBGetF(BUS_CLS_NM					,	PST_FIELD(pList, bus_cls_nm						));
				TMaxGetConvChar(BUS_CLS_NM						,	PST_FIELD(pList, bus_cls_nm						));
			
				TMaxGetConvChar(CACM_NM							,	PST_FIELD(pList, cacm_nm						));
				//nRet = TMaxFBGetF(CACM_NM						,	PST_FIELD(pList, cacm_nm						));
				
				nRet = TMaxFBGetF(BIZR_NO						,	PST_FIELD(pList, bizr_no						));
				nRet = TMaxFBGetF(TEL_NO						,	PST_FIELD(pList, tel_no							));
				nRet = TMaxFBGetF(ALCN_DEPR_TRML_ENG_ABRV_NM	,	PST_FIELD(pList, alcn_depr_trml_eng_abrv_nm		));
				nRet = TMaxFBGetF(ALCN_DEPR_TRML_NO				,	PST_FIELD(pList, alcn_depr_trml_no				));
				nRet = TMaxFBGetF(ALCN_ARVL_TRML_NO				,	PST_FIELD(pList, alcn_arvl_trml_no				));
				nRet = TMaxFBGetF(ALCN_DEPR_DT					,	PST_FIELD(pList, alcn_depr_dt					));
				nRet = TMaxFBGetF(ALCN_DEPR_TIME				,	PST_FIELD(pList, alcn_depr_time					));
				nRet = TMaxFBGetF(DEPR_TRML_NO					,	PST_FIELD(pList, depr_trml_no					));
				nRet = TMaxFBGetF(ARVL_TRML_NO					,	PST_FIELD(pList, arvl_trml_no					));
				nRet = TMaxFBGetF(DEPR_DT						,	PST_FIELD(pList, depr_dt						));
				nRet = TMaxFBGetF(DEPR_TIME						,	PST_FIELD(pList, depr_time						));
				nRet = TMaxFBGetF(BUS_CLS_CD					,	PST_FIELD(pList, bus_cls_cd						));
				nRet = TMaxFBGetF(CACM_CD						,	PST_FIELD(pList, cacm_cd						));
				nRet = TMaxFBGetF(ALCN_DVS_CD					,	PST_FIELD(pList, alcn_dvs_cd					));
				nRet = TMaxFBGetF(TISSU_DT						,	PST_FIELD(pList, tissu_dt						));
				nRet = TMaxFBGetF(TISSU_TRML_NO					,	PST_FIELD(pList, tissu_trml_no					));
				nRet = TMaxFBGetF(TISSU_WND_NO					,	PST_FIELD(pList, tissu_wnd_no					));
				nRet = TMaxFBGetF(TISSU_TIME					,	PST_FIELD(pList, tissu_time						));
				nRet = TMaxFBGetF(RPUB_DT						,	PST_FIELD(pList, rpub_dt						));
				nRet = TMaxFBGetF(RPUB_TRML_NO					,	PST_FIELD(pList, rpub_trml_no					));
				nRet = TMaxFBGetF(RPUB_WND_NO					,	PST_FIELD(pList, rpub_wnd_no					));
				nRet = TMaxFBGetF(RPUB_TIME						,	PST_FIELD(pList, rpub_time						));
				nRet = TMaxFBGetF(TISSU_SNO						,	PST_FIELD(pList, tissu_sno						));
				nRet = TMaxFBGetF(INHR_NO						,	PST_FIELD(pList, inhr_no						));
				nRet = TMaxFBGetF(INVS_NO						,	PST_FIELD(pList, invs_no						));
				nRet = TMaxFBGetF(TISSU_AMT						,	PST_FIELD(pList, n_tissu_amt					));
				nRet = TMaxFBGetF(SATS_NO						,	PST_FIELD(pList, sats_no						));
				nRet = TMaxFBGetF(TCK_KND_CD					,	PST_FIELD(pList, tck_knd_cd						));
				
				//nRet = TMaxFBGetF(PTRG_PRIN_NM					,	PST_FIELD(pList, ptrg_prin_nm					));
				TMaxGetConvChar(PTRG_PRIN_NM					,	PST_FIELD(pList, ptrg_prin_nm					));
				
				// 20211013 ADD
				TMaxGetConvChar(CARD_APRV_AMT					,	PST_FIELD(pList, card_aprv_amt					));	// 복합결제(카드)
				TMaxGetConvChar(MLG_APRV_AMT					,	PST_FIELD(pList, mlg_aprv_amt					));	// 복합결제(마일리지)
				TMaxGetConvChar(CPN_APRV_AMT					,	PST_FIELD(pList, cpn_aprv_amt					));	// 복합결제(쿠폰)
				// 20211013 ~ADD

				/// [티머니고속] 예매발권 - 예매내역 set memory
				CMrnpTmExpMem::GetInstance()->m_vtResCompleteMobile.push_back(*pList);
			}
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
#if (_KTC_CERTIFY_ > 0)
		LOG_WRITE("%30s - (%s) ",		 "CARD_NO"		,	pRPacket->card_no				);
#endif
		LOG_WRITE("%30s - (Number=%d) ", "APRV_AMT"		,	*(int *) pRPacket->n_aprv_amt	);
		LOG_WRITE("%30s - (%s) ",		 "CARD_APRV_NO"	,	pRPacket->card_aprv_no			);
		LOG_WRITE("%30s - (Number=%d) ", "FRC_CMRT"		,	*(int *) pRPacket->n_frc_cmm	);

		/// [티머니고속] 
		for(i = 0; i < nCount; i++)
		{
			//int k, cnt;
			prtk_tm_pub_mrs_htck_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 예매발권(=모바일티켓), 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "BUS_OPRN_DIST"				,	pList->bus_oprn_dist				);
			LOG_WRITE("%30s - (%s) ", "ROT_RDHM_NO_VAL"				,	pList->rot_rdhm_no_val				);
			LOG_WRITE("%30s - (%s) ", "CACM_NM_PRIN_YN"				,	pList->cacm_nm_prin_yn				);
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_PRIN_YN"				,	pList->bus_cls_prin_yn				);
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME_PRIN_YN"			,	pList->depr_time_prin_yn			);
			LOG_WRITE("%30s - (%s) ", "SATS_NO_PRIN_YN"				,	pList->sats_no_prin_yn				);
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_ENG_ABRV_NM"		,	pList->depr_trml_eng_abrv_nm		);
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_ENG_ABRV_NM"		,	pList->arvl_trml_eng_abrv_nm		);
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_NM"					,	pList->bus_cls_nm					);
			LOG_WRITE("%30s - (%s) ", "CACM_NM"						,	pList->cacm_nm						);
			LOG_WRITE("%30s - (%s) ", "BIZR_NO"						,	pList->bizr_no						);
			LOG_WRITE("%30s - (%s) ", "TEL_NO"						,	pList->tel_no						);
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_ENG_ABRV_NM"	,	pList->alcn_depr_trml_eng_abrv_nm	);
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO"			,	pList->alcn_depr_trml_no			);
			LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO"			,	pList->alcn_arvl_trml_no			);
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_DT"				,	pList->alcn_depr_dt					);
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TIME"				,	pList->alcn_depr_time				);
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"				,	pList->depr_trml_no					);
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"				,	pList->arvl_trml_no					);
			LOG_WRITE("%30s - (%s) ", "DEPR_DT"						,	pList->depr_dt						);
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME"					,	pList->depr_time					);
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD"					,	pList->bus_cls_cd					);
			LOG_WRITE("%30s - (%s) ", "CACM_CD"						,	pList->cacm_cd						);
			LOG_WRITE("%30s - (%s) ", "ALCN_DVS_CD"					,	pList->alcn_dvs_cd					);
			LOG_WRITE("%30s - (%s) ", "TISSU_DT"					,	pList->tissu_dt						);
			LOG_WRITE("%30s - (%s) ", "TISSU_TRML_NO"				,	pList->tissu_trml_no				);
			LOG_WRITE("%30s - (%s) ", "TISSU_WND_NO"				,	pList->tissu_wnd_no					);
			LOG_WRITE("%30s - (%s) ", "TISSU_TIME"					,	pList->tissu_time					);
			LOG_WRITE("%30s - (%s) ", "RPUB_DT"						,	pList->rpub_dt						);
			LOG_WRITE("%30s - (%s) ", "RPUB_TRML_NO"				,	pList->rpub_trml_no					);
			LOG_WRITE("%30s - (%s) ", "RPUB_WND_NO"					,	pList->rpub_wnd_no					);
			LOG_WRITE("%30s - (%s) ", "RPUB_TIME"					,	pList->rpub_time					);
			LOG_WRITE("%30s - (%s) ", "TISSU_SNO"					,	pList->tissu_sno					);
			LOG_WRITE("%30s - (%s) ", "INHR_NO"						,	pList->inhr_no						);
			LOG_WRITE("%30s - (%s) ", "INVS_NO"						,	pList->invs_no						);
			LOG_WRITE("%30s - (Number=%d) ", "TISSU_AMT"			,	*(int *)pList->n_tissu_amt			);
			LOG_WRITE("%30s - (%s) ", "SATS_NO"						,	pList->sats_no						);
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"					,	pList->tck_knd_cd					);
			LOG_WRITE("%30s - (%s) ", "PTRG_PRIN_NM"				,	pList->ptrg_prin_nm					);
			// 20211013 ADD
			LOG_WRITE("%30s - (Number=%d) ", "CARD_APRV_AMT"		,	*(int *)pList->card_aprv_amt		);	// 복합결제(카드)
			LOG_WRITE("%30s - (Number=%d) ", "MLG_APRV_AMT"			,	*(int *)pList->mlg_aprv_amt			);	// 복합결제(마일리지)
			LOG_WRITE("%30s - (Number=%d) ", "CPN_APRV_AMT"			,	*(int *)pList->cpn_aprv_amt			);	// 복합결제(쿠폰)
			// 20211013 ~ADD
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_ReadBusTckno
 * @details		[티머니고속] 승차권 정보 조회(=환불 승차권 조회)
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_ReadBusTckno(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nCount1;
	char*	pService = "TK_ReadBusTckNo";
	pstk_tm_read_bus_tckno_t pSPacket;
	prtk_tm_read_bus_tckno_t pRPacket;

	nRet = nTimeout = nCount = nCount1 = 0;

	LOG_OPEN();
	LOG_WRITE(" 환불조회_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_read_bus_tckno_t) pData;
	pRPacket = (prtk_tm_read_bus_tckno_t) retBuf;

	/// 티머니고속 - zero memory
	::ZeroMemory(&CCancRyTkTmExpMem::GetInstance()->tRespInq, sizeof(rtk_tm_read_bus_tckno_t));

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"		,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"		,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"		,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"		,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","TISSU_DT"			,	pSPacket->tissu_dt			);
		LOG_WRITE("%30s - (%s) ","TISSU_TRML_NO"	,	pSPacket->tissu_trml_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_WND_NO"		,	pSPacket->tissu_wnd_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_SNO"		,	pSPacket->tissu_sno			);
		LOG_WRITE("%30s - (%s) ","TAK_DVS_CD"		,	pSPacket->tak_dvs_cd		);
		LOG_WRITE("%30s - (%s) ","RYRT_DVS_CD"		,	pSPacket->ryrt_dvs_cd		);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS		,	pSPacket->req_pgm_dvs		);
		nRet = TMaxFBPut(REQ_TRML_NO		,	pSPacket->req_trml_no		);
		nRet = TMaxFBPut(REQ_WND_NO			,	pSPacket->req_wnd_no		);
		nRet = TMaxFBPut(REQ_USER_NO		,	pSPacket->req_user_no		);

		nRet = TMaxFBPut(TISSU_DT			,	pSPacket->tissu_dt			);
		nRet = TMaxFBPut(TISSU_TRML_NO		,	pSPacket->tissu_trml_no		);
		nRet = TMaxFBPut(TISSU_WND_NO		,	pSPacket->tissu_wnd_no		);
		nRet = TMaxFBPut(TISSU_SNO			,	pSPacket->tissu_sno			);
		nRet = TMaxFBPut(TAK_DVS_CD			,	pSPacket->tak_dvs_cd		);
		nRet = TMaxFBPut(RYRT_DVS_CD		,	pSPacket->ryrt_dvs_cd		);
	}

	//m_nTPCallFlag = TPNOFLAGS;
	//TMaxTransaction(TMAX_TR_BEGIN);


	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			//TMaxTransaction(TMAX_TR_ROLLBACK);

			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
		
		//TMaxTransaction(TMAX_TR_COMMIT);

		nRet = TMaxFBGet(RSP_CD					, PST_FIELD(pRPacket, rsp_cd				));
		nRet = TMaxFBGet(DEPR_DT				, PST_FIELD(pRPacket, depr_dt				));
		nRet = TMaxFBGet(HSPD_CTY_DVS_CD		, PST_FIELD(pRPacket, hspd_cty_dvs_cd		));
		nRet = TMaxFBGet(DRTN_CD				, PST_FIELD(pRPacket, drtn_cd				));
		nRet = TMaxFBGet(ALCN_DEPR_TRML_NO		, PST_FIELD(pRPacket, alcn_depr_trml_no		));
		nRet = TMaxFBGet(ALCN_ARVL_TRML_NO		, PST_FIELD(pRPacket, alcn_arvl_trml_no		));
		nRet = TMaxFBGet(DEPR_TRML_NO			, PST_FIELD(pRPacket, depr_trml_no			));
		nRet = TMaxFBGet(ARVL_TRML_NO			, PST_FIELD(pRPacket, arvl_trml_no			));
		nRet = TMaxFBGet(ALCN_ROT_NO			, PST_FIELD(pRPacket, alcn_rot_no			));
		nRet = TMaxFBGet(DEPR_TIME				, PST_FIELD(pRPacket, depr_time				));
		nRet = TMaxFBGet(BUS_CLS_CD				, PST_FIELD(pRPacket, bus_cls_cd			));
		nRet = TMaxFBGet(CACM_CD				, PST_FIELD(pRPacket, cacm_cd				));
		nRet = TMaxFBGet(TCK_KND_CD				, PST_FIELD(pRPacket, tck_knd_cd			));
		nRet = TMaxFBGet(TISSU_FEE				, PST_FIELD(pRPacket, tissu_fee				));
		nRet = TMaxFBGet(SATS_NO				, PST_FIELD(pRPacket, sats_no				));
		nRet = TMaxFBGet(INHR_NO				, PST_FIELD(pRPacket, inhr_no				));
		nRet = TMaxFBGet(INVS_NO				, PST_FIELD(pRPacket, invs_no				));
		nRet = TMaxFBGet(TISSU_CHNL_DVS_CD		, PST_FIELD(pRPacket, tissu_chnl_dvs_cd		));
		nRet = TMaxFBGet(TISSU_TIME				, PST_FIELD(pRPacket, tissu_time			));
		nRet = TMaxFBGet(TISSU_USER_NO			, PST_FIELD(pRPacket, tissu_user_no			));
		//nRet = TMaxFBGet(ALCN_ROT_NM			, PST_FIELD(pRPacket, alcn_rot_nm			));
		TMaxGetConvChar(ALCN_ROT_NM				, PST_FIELD(pRPacket, alcn_rot_nm			));
		nRet = TMaxFBGet(PYN_DTL_CD				, PST_FIELD(pRPacket, pyn_dtl_cd			));
		nRet = TMaxFBGet(ALCN_DEPR_TIME			, PST_FIELD(pRPacket, alcn_depr_time		));
		nRet = TMaxFBGet(CARD_NO				, PST_FIELD(pRPacket, card_no				));
		nRet = TMaxFBGet(CARD_APRV_NO			, PST_FIELD(pRPacket, card_aprv_no			));
		nRet = TMaxFBGet(CARD_TRD_SNO			, PST_FIELD(pRPacket, card_trd_sno			));
		nRet = TMaxFBGet(TISSU_DT				, PST_FIELD(pRPacket, tissu_dt				));
		nRet = TMaxFBGet(SAM_TRD_SNO			, PST_FIELD(pRPacket, sam_trd_sno			));
		nRet = TMaxFBGet(TRD_DTM				, PST_FIELD(pRPacket, trd_dtm				));
		nRet = TMaxFBGet(USER_KEY_VAL			, PST_FIELD(pRPacket, user_key_val			));
		nRet = TMaxFBGet(CSRC_APRV_NO			, PST_FIELD(pRPacket, csrc_aprv_no			));
		nRet = TMaxFBGet(USER_DVS_CD			, PST_FIELD(pRPacket, user_dvs_cd			));
		nRet = TMaxFBGet(RY_AMT					, PST_FIELD(pRPacket, ry_amt				));
		nRet = TMaxFBGet(RY_PFIT				, PST_FIELD(pRPacket, ry_pfit				));
		nRet = TMaxFBGet(DC_RC_AMT				, PST_FIELD(pRPacket, dc_rc_amt				));
		nRet = TMaxFBGet(RPUB_FCNT				, PST_FIELD(pRPacket, rpub_fcnt				));
		nRet = TMaxFBGet(TISSU_STA_CD			, PST_FIELD(pRPacket, tissu_sta_cd			));
		nRet = TMaxFBGet(RY_STA_CD				, PST_FIELD(pRPacket, ry_sta_cd				));
		nRet = TMaxFBGet(CHTK_STA_CD			, PST_FIELD(pRPacket, chtk_sta_cd			));
		nRet = TMaxFBGet(REST_STA_CD			, PST_FIELD(pRPacket, rest_sta_cd			));
		nRet = TMaxFBGet(ARVL_DTM				, PST_FIELD(pRPacket, arvl_dtm				));

		::CopyMemory(&CCancRyTkTmExpMem::GetInstance()->tRespInq, pRPacket, sizeof(rtk_tm_read_bus_tckno_t)); 
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s - (%s) ", "RSP_CD"				,	pRPacket->rsp_cd				);
		LOG_WRITE("%30s - (%s) ", "DEPR_DT"				,	pRPacket->depr_dt				);
		LOG_WRITE("%30s - (%s) ", "HSPD_CTY_DVS_CD"		,	pRPacket->hspd_cty_dvs_cd		);
		LOG_WRITE("%30s - (%s) ", "DRTN_CD"				,	pRPacket->drtn_cd				);
		LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO"	,	pRPacket->alcn_depr_trml_no		);
		LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO"	,	pRPacket->alcn_arvl_trml_no		);
		LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"		,	pRPacket->depr_trml_no			);
		LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"		,	pRPacket->arvl_trml_no			);
		LOG_WRITE("%30s - (%s) ", "ALCN_ROT_NO"			,	pRPacket->alcn_rot_no			);
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME"			,	pRPacket->depr_time				);
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD"			,	pRPacket->bus_cls_cd			);
		LOG_WRITE("%30s - (%s) ", "CACM_CD"				,	pRPacket->cacm_cd				);
		LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"			,	pRPacket->tck_knd_cd			);
		LOG_WRITE("%30s - (Num=%d) ", "TISSU_FEE"		,	*(int *)pRPacket->tissu_fee		);
		LOG_WRITE("%30s - (%s) ", "SATS_NO"				,	pRPacket->sats_no				);
		LOG_WRITE("%30s - (%s) ", "INHR_NO"				,	pRPacket->inhr_no				);
		LOG_WRITE("%30s - (%s) ", "INVS_NO"				,	pRPacket->invs_no				);
		LOG_WRITE("%30s - (%s) ", "TISSU_CHNL_DVS_CD"	,	pRPacket->tissu_chnl_dvs_cd		);
		LOG_WRITE("%30s - (%s) ", "TISSU_TIME"			,	pRPacket->tissu_time			);
		LOG_WRITE("%30s - (%s) ", "TISSU_USER_NO"		,	pRPacket->tissu_user_no			);
		LOG_WRITE("%30s - (%s) ", "ALCN_ROT_NM"			,	pRPacket->alcn_rot_nm			);
		LOG_WRITE("%30s - (%s) ", "PYN_DTL_CD"			,	pRPacket->pyn_dtl_cd			);
		LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TIME"		,	pRPacket->alcn_depr_time		);
#if (_KTC_CERTIFY_ > 0)
		LOG_WRITE("%30s - (%s) ", "CARD_NO"				,	pRPacket->card_no				);
#endif
		LOG_WRITE("%30s - (%s) ", "CARD_APRV_NO"		,	pRPacket->card_aprv_no			);
		LOG_WRITE("%30s - (%s) ", "CARD_TRD_SNO"		,	pRPacket->card_trd_sno			);
		LOG_WRITE("%30s - (%s) ", "TISSU_DT"			,	pRPacket->tissu_dt				);
		LOG_WRITE("%30s - (%s) ", "SAM_TRD_SNO"			,	pRPacket->sam_trd_sno			);
		LOG_WRITE("%30s - (%s) ", "TRD_DTM"				,	pRPacket->trd_dtm				);
		LOG_WRITE("%30s - (%s) ", "USER_KEY_VAL"		,	pRPacket->user_key_val			);
		LOG_WRITE("%30s - (%s) ", "CSRC_APRV_NO"		,	pRPacket->csrc_aprv_no			);
		LOG_WRITE("%30s - (%s) ", "USER_DVS_CD"			,	pRPacket->user_dvs_cd			);
		LOG_WRITE("%30s - (Num=%d) ", "RY_AMT"			,	*(int *)pRPacket->ry_amt		);
		LOG_WRITE("%30s - (Num=%d) ", "RY_PFIT"			,	*(int *)pRPacket->ry_pfit		);
		LOG_WRITE("%30s - (Num=%d) ", "DC_RC_AMT"		,	*(int *)pRPacket->dc_rc_amt		);
		LOG_WRITE("%30s - (Num=%d) ", "RPUB_FCNT"		,	*(int *)pRPacket->rpub_fcnt		);
		LOG_WRITE("%30s - (%s) ", "TISSU_STA_CD"		,	pRPacket->tissu_sta_cd			);
		LOG_WRITE("%30s - (%s) ", "RY_STA_CD"			,	pRPacket->ry_sta_cd				);
		LOG_WRITE("%30s - (%s) ", "CHTK_STA_CD"			,	pRPacket->chtk_sta_cd			);
		LOG_WRITE("%30s - (%s) ", "REST_STA_CD"			,	pRPacket->rest_sta_cd			);
	}

	///> data proc
	{
		nRet = 1;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_CancRyTck
 * @details		[티머니고속] 환불 
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_CancRyTck(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i, nCount1;
	char*	pService = "TK_CancRyTck";
	pstk_tm_cancrytck_t pSPacket;
	prtk_tm_cancrytck_t pRPacket;

	nRet = nTimeout = nCount = nCount1 = 0;

	LOG_OPEN();
	LOG_WRITE(" 환불처리_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_cancrytck_t) pData;
	pRPacket = (prtk_tm_cancrytck_t) retBuf;

	/// 티머니고속 - zero memory
 	CCancRyTkTmExpMem::GetInstance()->m_vtRespRefList.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"		,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"		,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"		,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"		,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","REQ_DVS_CD"		,	pSPacket->req_dvs_cd		);
		LOG_WRITE("%30s - (Num=%d) ","REC_NUM"		,	*(int *) pSPacket->rec_num	);
		nCount = *(int *) pSPacket->rec_num;
		for(i = 0; i < nCount; i++)
		{
			stk_tm_cancrytck_list_t *pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("####### 환불, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ","TISSU_DT"			,	pList->tissu_dt			);
			LOG_WRITE("%30s - (%s) ","TISSU_TRML_NO"	,	pList->tissu_trml_no	);
			LOG_WRITE("%30s - (%s) ","TISSU_WND_NO"		,	pList->tissu_wnd_no		);
			LOG_WRITE("%30s - (%s) ","TISSU_SNO"		,	pList->tissu_sno		);
			LOG_WRITE("%30s - (%s) ","RYRT_DVS_CD"		,	pList->ryrt_dvs_cd		);

		}
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS		,	pSPacket->req_pgm_dvs		);
		nRet = TMaxFBPut(REQ_TRML_NO		,	pSPacket->req_trml_no		);
		nRet = TMaxFBPut(REQ_WND_NO			,	pSPacket->req_wnd_no		);
		nRet = TMaxFBPut(REQ_USER_NO		,	pSPacket->req_user_no		);
	
		nRet = TMaxFBPut(REQ_DVS_CD			,	pSPacket->req_dvs_cd		);
		nRet = TMaxFBPut(REC_NUM			,	pSPacket->rec_num		);
		nCount = *(int *) pSPacket->rec_num;
		for(i = 0; i < nCount; i++)
		{
			stk_tm_cancrytck_list_t *pList;

			pList = &pSPacket->List[i];

			nRet = TMaxFBPut(TISSU_DT			,	pList->tissu_dt			);
			nRet = TMaxFBPut(TISSU_TRML_NO		,	pList->tissu_trml_no	);
			nRet = TMaxFBPut(TISSU_WND_NO		,	pList->tissu_wnd_no		);
			nRet = TMaxFBPut(TISSU_SNO			,	pList->tissu_sno		);
			nRet = TMaxFBPut(RYRT_DVS_CD		,	pList->ryrt_dvs_cd		);
		}
	}

	//m_nTPCallFlag = TPNOFLAGS;
	//TMaxTransaction(TMAX_TR_BEGIN);


	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			//TMaxTransaction(TMAX_TR_ROLLBACK);

			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
		
		//TMaxTransaction(TMAX_TR_COMMIT);

		//::CopyMemory(CMrnpTmExpMem::GetInstance()->Base.rsp_cd, pRPacket->rsp_cd, sizeof(CMrnpTmExpMem::GetInstance()->Base.rsp_cd));

		/// 정보 수
		nRet = TMaxFBGet(REC_NUM	, PST_FIELD(pRPacket, rec_num));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		/// memory allocation
		{
			/// 환불_승차권 조회 List
			pRPacket->pList = new rtk_tm_cancrytck_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_cancrytck_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				//int k, cnt;
				prtk_tm_cancrytck_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(PYN_DTL_CD		,	PST_FIELD(pList, pyn_dtl_cd		));
				nRet = TMaxFBGetF(RYRT_DVS_CD		,	PST_FIELD(pList, ryrt_dvs_cd	));
				nRet = TMaxFBGetF(RY_AMT			,	PST_FIELD(pList, ry_amt			));
				nRet = TMaxFBGetF(DC_RC_AMT			,	PST_FIELD(pList, dc_rc_amt		));
				// 20211013 ADD
				nRet = TMaxFBGetF(PYN_DVS_CD		,	PST_FIELD(pList, pyn_dvs_cd		));	//지불구분코드 VARCHAR2( j )(1) '5'일 경우 복합결제
				nRet = TMaxFBGetF(CARD_CANC_AMT		,	PST_FIELD(pList, card_canc_amt	));	//복합결제취소(카드) NUMBER( i )(22) 지불구분이 '5'일 경우에만 사용
				nRet = TMaxFBGetF(MLG_CANC_AMT		,	PST_FIELD(pList, mlg_canc_amt	));	//복합결제취소(마일리지) NUMBER( i )(22) 지불구분이 '5'일 경우에만 사용
				nRet = TMaxFBGetF(CPN_CANC_AMT		,	PST_FIELD(pList, cpn_canc_amt	));	//복합결제취소(쿠폰) NUMBER( i )(22) 지불구분이 '5'일 경우에만 사용
				// 20211013 ~ADD

				/// [티머니고속] 환불처리 set memory
				CCancRyTkTmExpMem::GetInstance()->m_vtRespRefList.push_back(*pList);
			}
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		/// [티머니고속] 환불_승차권 조회
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_cancrytck_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 환불, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "PYN_DTL_CD"		,	pList->pyn_dtl_cd		);
			LOG_WRITE("%30s - (%s) ", "RYRT_DVS_CD"		,	pList->ryrt_dvs_cd		);
			LOG_WRITE("%30s - (%s) ", "RY_AMT"			,	pList->ry_amt			);
			LOG_WRITE("%30s - (%s) ", "DC_RC_AMT"		,	pList->dc_rc_amt		);
			// 20211013 ADD
			LOG_WRITE("%30s - (%s) ", "PYN_DVS_CD"		,	pList->pyn_dvs_cd		);
			LOG_WRITE("%30s - (%s) ", "CARD_CANC_AMT"	,	pList->card_canc_amt	);
			LOG_WRITE("%30s - (%s) ", "MLG_CANC_AMT"	,	pList->mlg_canc_amt		);
			LOG_WRITE("%30s - (%s) ", "CPN_CANC_AMT"	,	pList->cpn_canc_amt		);
			// 20211013 ~ADD
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_CM_PubInquiry
 * @details		[티머니고속] 승차권 발행 내역 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CM_PubInquiry(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, i, nCount;
	char*	pService = "CM_PubPt";
	pstk_tm_pub_inq_t pSPacket;
	prtk_tm_pub_inq_t pRPacket;

	i = nCount = nRet = nTimeout = 0;

	LOG_OPEN();
	LOG_WRITE(" 발권정보 조회_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_pub_inq_t) pData;
	pRPacket = (prtk_tm_pub_inq_t) retBuf;

	/// 발권정보 조회 list clear
	CPubTckTmExpMem::GetInstance()->m_vtPbTckInqList.clear();

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"		,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"		,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"		,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"		,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (Num%d) ","REC_NUM"		,	*(int *)pSPacket->rec_num	);
		LOG_WRITE("%30s - (%s) ","SORT_DVS_CD"		,	pSPacket->sort_dvs_cd		);
		LOG_WRITE("%30s - (%s) ","TAK_DT"			,	pSPacket->tak_dt			);
		LOG_WRITE("%30s - (%s) ","TAK_TIME"			,	pSPacket->tak_time			);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO"		,	pSPacket->depr_trml_no		);
		LOG_WRITE("%30s - (%s) ","TAK_WND_NO"		,	pSPacket->tak_wnd_no		);
		LOG_WRITE("%30s - (%s) ","TAK_USER_NO"		,	pSPacket->tak_user_no		);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"		,	pSPacket->arvl_trml_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_DT"			,	pSPacket->tissu_dt			);
		LOG_WRITE("%30s - (%s) ","TISSU_TRML_NO"	,	pSPacket->tissu_trml_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_WND_NO"		,	pSPacket->tissu_wnd_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_SNO"		,	pSPacket->tissu_sno			);
		LOG_WRITE("%30s - (%s) ","BEF_AFT_DVS"		,	pSPacket->bef_aft_dvs		);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS		,	pSPacket->req_pgm_dvs		);
		nRet = TMaxFBPut(REQ_TRML_NO		,	pSPacket->req_trml_no		);
		nRet = TMaxFBPut(REQ_WND_NO			,	pSPacket->req_wnd_no		);
		nRet = TMaxFBPut(REQ_USER_NO		,	pSPacket->req_user_no		);

		nRet = TMaxFBPut(REC_NUM			,	pSPacket->rec_num			);
		nRet = TMaxFBPut(SORT_DVS_CD		,	pSPacket->sort_dvs_cd		);
		nRet = TMaxFBPut(TAK_DT				,	pSPacket->tak_dt			);
		nRet = TMaxFBPut(TAK_TIME			,	pSPacket->tak_time			);
		nRet = TMaxFBPut(DEPR_TRML_NO		,	pSPacket->depr_trml_no		);
		nRet = TMaxFBPut(TAK_WND_NO			,	pSPacket->tak_wnd_no		);
		nRet = TMaxFBPut(TAK_USER_NO		,	pSPacket->tak_user_no		);
		nRet = TMaxFBPut(ARVL_TRML_NO		,	pSPacket->arvl_trml_no		);
		nRet = TMaxFBPut(TISSU_DT			,	pSPacket->tissu_dt			);
		nRet = TMaxFBPut(TISSU_TRML_NO		,	pSPacket->tissu_trml_no		);
		nRet = TMaxFBPut(TISSU_WND_NO		,	pSPacket->tissu_wnd_no		);
		nRet = TMaxFBPut(TISSU_SNO			,	pSPacket->tissu_sno			);
		nRet = TMaxFBPut(BEF_AFT_DVS		,	pSPacket->bef_aft_dvs		);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGet(BEF_AFT_DVS	, PST_FIELD(pRPacket, bef_aft_dvs		));
		nRet = TMaxFBGet(REC_NUM		, PST_FIELD(pRPacket, rec_num			));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		// file save
		OperGetFileName(nOperID, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}
		MySetFilePointer(hFile, 0, FILE_END);

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->bef_aft_dvs, sizeof(pRPacket->bef_aft_dvs));
		MyWriteFile(hFile, &nCount, sizeof(int)+1);

		/// memory allocation
		{
			/// 발권정보 조회 List
			pRPacket->pList = new rtk_tm_pub_inq_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_pub_inq_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				//int k, cnt;
				prtk_tm_pub_inq_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TISSU_DT				,	PST_FIELD(pList, tissu_dt				));
				nRet = TMaxFBGetF(TISSU_TRML_NO			,	PST_FIELD(pList, tissu_trml_no			));
				nRet = TMaxFBGetF(TISSU_WND_NO			,	PST_FIELD(pList, tissu_wnd_no			));
				nRet = TMaxFBGetF(TISSU_SNO				,	PST_FIELD(pList, tissu_sno				));
				nRet = TMaxFBGetF(INHR_NO				,	PST_FIELD(pList, inhr_no				));
				nRet = TMaxFBGetF(DEPR_DT				,	PST_FIELD(pList, depr_dt				));
				nRet = TMaxFBGetF(DRTN_CD				,	PST_FIELD(pList, drtn_cd				));
				nRet = TMaxFBGetF(HSPD_CTY_DVS_CD		,	PST_FIELD(pList, hspd_cty_dvs_cd		));
				nRet = TMaxFBGetF(ALCN_DEPR_TRML_NO		,	PST_FIELD(pList, alcn_depr_trml_no		));
				nRet = TMaxFBGetF(ALCN_ARVL_TRML_NO		,	PST_FIELD(pList, alcn_arvl_trml_no		));
				nRet = TMaxFBGetF(ALCN_ROT_NO			,	PST_FIELD(pList, alcn_rot_no			));
				nRet = TMaxFBGetF(DEPR_TRML_NO			,	PST_FIELD(pList, depr_trml_no			));
				nRet = TMaxFBGetF(ARVL_TRML_NO			,	PST_FIELD(pList, arvl_trml_no			));
				nRet = TMaxFBGetF(ALCN_DEPR_TIME		,	PST_FIELD(pList, alcn_depr_time			));
				nRet = TMaxFBGetF(DEPR_TIME				,	PST_FIELD(pList, depr_time				));
				nRet = TMaxFBGetF(BUS_CLS_CD			,	PST_FIELD(pList, bus_cls_cd				));
				nRet = TMaxFBGetF(CACM_CD				,	PST_FIELD(pList, cacm_cd				));
				nRet = TMaxFBGetF(TCK_KND_CD			,	PST_FIELD(pList, tck_knd_cd				));
				nRet = TMaxFBGetF(TISSU_FEE				,	PST_FIELD(pList, tissu_fee				));
				nRet = TMaxFBGetF(SATS_NO				,	PST_FIELD(pList, sats_no				));
				nRet = TMaxFBGetF(TISSU_STA_CD			,	PST_FIELD(pList, tissu_sta_cd			));
				nRet = TMaxFBGetF(TISSU_CHNL_DVS_CD		,	PST_FIELD(pList, tissu_chnl_dvs_cd		));
				nRet = TMaxFBGetF(PYN_DTL_CD			,	PST_FIELD(pList, pyn_dtl_cd				));
				nRet = TMaxFBGetF(TISSU_TIME			,	PST_FIELD(pList, tissu_time				));
				nRet = TMaxFBGetF(TISSU_USER_NO			,	PST_FIELD(pList, tissu_user_no			));
				nRet = TMaxFBGetF(ALCN_DVS_CD			,	PST_FIELD(pList, alcn_dvs_cd			));
				nRet = TMaxFBGetF(RY_STA_CD				,	PST_FIELD(pList, ry_sta_cd				));
				nRet = TMaxFBGetF(ALCN_ROT_NM			,	PST_FIELD(pList, alcn_rot_nm			));
				nRet = TMaxFBGetF(DTA_DVS_CD			,	PST_FIELD(pList, dta_dvs_cd				));
				/// 카드인 경우
				nRet = TMaxFBGetF(CARD_NO				,	PST_FIELD(pList, card_no				));
#ifdef __KTC_PROGRAM__
				::ZeroMemory(pList->card_no, sizeof(pList->card_no));
#endif
				nRet = TMaxFBGetF(CARD_APRV_NO			,	PST_FIELD(pList, card_aprv_no			));
				/// 현금인 경우
				nRet = TMaxFBGetF(USER_KEY_VAL			,	PST_FIELD(pList, user_key_val			));
				nRet = TMaxFBGetF(CSRC_APRV_NO			,	PST_FIELD(pList, csrc_aprv_no			));
				nRet = TMaxFBGetF(USER_DVS_CD			,	PST_FIELD(pList, user_dvs_cd			));
				/// 선불카드인 경우
				nRet = TMaxFBGetF(TRD_BEF_CARD_BAL		,	PST_FIELD(pList, trd_bef_card_bal		));
				nRet = TMaxFBGetF(TRD_REQ_AMT			,	PST_FIELD(pList, trd_req_amt			));
				nRet = TMaxFBGetF(TRD_AFT_CARD_BAL		,	PST_FIELD(pList, trd_aft_card_bal		));

				/// [티머니고속] 발권정보 조회 - 내역 set memory
				CPubTckTmExpMem::GetInstance()->m_vtPbTckInqList.push_back(*pList);

				MyWriteFile(hFile, pList, sizeof(rtk_tm_pub_inq_list_t));
			}
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s - (%s) ", "BEF_AFT_DVS"		,	pRPacket->bef_aft_dvs			);

		/// [티머니고속] 
		for(i = 0; i < nCount; i++)
		{
			//int k, cnt;
			prtk_tm_pub_inq_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 발권정보조회, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TISSU_DT"			,	pList->tissu_dt				);
			LOG_WRITE("%30s - (%s) ", "TISSU_TRML_NO"		,	pList->tissu_trml_no		);
			LOG_WRITE("%30s - (%s) ", "TISSU_WND_NO"		,	pList->tissu_wnd_no			);
			LOG_WRITE("%30s - (%s) ", "TISSU_SNO"			,	pList->tissu_sno			);
			LOG_WRITE("%30s - (%s) ", "INHR_NO"				,	pList->inhr_no				);
			LOG_WRITE("%30s - (%s) ", "DEPR_DT"				,	pList->depr_dt				);
			LOG_WRITE("%30s - (%s) ", "DRTN_CD"				,	pList->drtn_cd				);
			LOG_WRITE("%30s - (%s) ", "HSPD_CTY_DVS_CD"		,	pList->hspd_cty_dvs_cd		);
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO"	,	pList->alcn_depr_trml_no	);
			LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO"	,	pList->alcn_arvl_trml_no	);
			LOG_WRITE("%30s - (%s) ", "ALCN_ROT_NO"			,	pList->alcn_rot_no			);
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"		,	pList->depr_trml_no			);
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"		,	pList->arvl_trml_no			);
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TIME"		,	pList->alcn_depr_time		);
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME"			,	pList->depr_time			);
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD"			,	pList->bus_cls_cd			);
			LOG_WRITE("%30s - (%s) ", "CACM_CD"				,	pList->cacm_cd				);
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"			,	pList->tck_knd_cd			);
			LOG_WRITE("%30s - (Num=%d) ", "TISSU_FEE"		,	*(int *)pList->tissu_fee	);
			LOG_WRITE("%30s - (%s) ", "SATS_NO"				,	pList->sats_no				);
			LOG_WRITE("%30s - (%s) ", "TISSU_STA_CD"		,	pList->tissu_sta_cd			);
			LOG_WRITE("%30s - (%s) ", "TISSU_CHNL_DVS_CD"	,	pList->tissu_chnl_dvs_cd	);
			LOG_WRITE("%30s - (%s) ", "PYN_DTL_CD"			,	pList->pyn_dtl_cd			);
			LOG_WRITE("%30s - (%s) ", "TISSU_TIME"			,	pList->tissu_time			);
			LOG_WRITE("%30s - (%s) ", "TISSU_USER_NO"		,	pList->tissu_user_no		);
			LOG_WRITE("%30s - (%s) ", "ALCN_DVS_CD"			,	pList->alcn_dvs_cd			);
			LOG_WRITE("%30s - (%s) ", "RY_STA_CD"			,	pList->ry_sta_cd			);
			LOG_WRITE("%30s - (Num=%d) ", "ALCN_ROT_NM"		,	*(int *)pList->alcn_rot_nm	);
			LOG_WRITE("%30s - (%s) ", "DTA_DVS_CD"			,	pList->dta_dvs_cd			);

			LOG_WRITE("%30s ######## ", "카드인 경우"										);
#if (_KTC_CERTIFY_ > 0)
			LOG_WRITE("%30s - (%s) ", "CARD_NO"				,	pList->card_no				);
#endif
			LOG_WRITE("%30s - (%s) ", "CARD_APRV_NO"		,	pList->card_aprv_no			);

			LOG_WRITE("%30s ######## ", "현금인 경우"										);
			LOG_WRITE("%30s - (%s) ", "USER_KEY_VAL"		,	pList->user_key_val			);
			LOG_WRITE("%30s - (%s) ", "CSRC_APRV_NO"		,	pList->csrc_aprv_no			);
			LOG_WRITE("%30s - (%s) ", "USER_DVS_CD"			,	pList->user_dvs_cd			);

			LOG_WRITE("%30s ######## ", "선불인 경우"										);
			LOG_WRITE("%30s - (%s) ", "TRD_BEF_CARD_BAL"	,	pList->trd_bef_card_bal		);
			LOG_WRITE("%30s - (%s) ", "TRD_REQ_AMT"			,	pList->trd_req_amt			);
			LOG_WRITE("%30s - (%s) ", "TRD_AFT_CARD_BAL"	,	pList->trd_aft_card_bal		);
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_CM_RePubInquiry
 * @details		[티머니고속] 재발행 승차권 내역 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CM_RePubInquiry(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, i, nCount;
	char*	pService = "CM_RPubPt";
	pstk_tm_rpub_inq_t pSPacket;
	prtk_tm_rpub_inq_t pRPacket;

	nRet = nTimeout = i = nCount = 0;

	LOG_OPEN();
	LOG_WRITE(" 재발행_승차권조회_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_rpub_inq_t) pData;
	pRPacket = (prtk_tm_rpub_inq_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"		,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"		,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"		,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"		,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (Num=%d) ","REC_NUM"		,	*(int *)pSPacket->rec_num	);
		LOG_WRITE("%30s - (%s) ","SORT_DVS_CD"		,	pSPacket->sort_dvs_cd		);
		LOG_WRITE("%30s - (%s) ","TAK_DT"			,	pSPacket->tak_dt			);
		LOG_WRITE("%30s - (%s) ","TAK_TIME"			,	pSPacket->tak_time			);
		LOG_WRITE("%30s - (%s) ","TAK_WND_NO"		,	pSPacket->tak_wnd_no		);
		LOG_WRITE("%30s - (%s) ","TAK_USER_NO"		,	pSPacket->tak_user_no		);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO"		,	pSPacket->depr_trml_no		);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"		,	pSPacket->arvl_trml_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_DT"			,	pSPacket->tissu_dt			);
		LOG_WRITE("%30s - (%s) ","TISSU_TRML_NO"	,	pSPacket->tissu_trml_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_WND_NO"		,	pSPacket->tissu_wnd_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_SNO"		,	pSPacket->tissu_sno			);
		LOG_WRITE("%30s - (%s) ","BEF_AFT_DVS"		,	pSPacket->bef_aft_dvs		);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS		,	pSPacket->req_pgm_dvs		);
		nRet = TMaxFBPut(REQ_TRML_NO		,	pSPacket->req_trml_no		);
		nRet = TMaxFBPut(REQ_WND_NO			,	pSPacket->req_wnd_no		);
		nRet = TMaxFBPut(REQ_USER_NO		,	pSPacket->req_user_no		);

		nRet = TMaxFBPut(REC_NUM			,	pSPacket->rec_num			);
		nRet = TMaxFBPut(SORT_DVS_CD		,	pSPacket->sort_dvs_cd		);
		nRet = TMaxFBPut(TAK_DT				,	pSPacket->tak_dt			);
		nRet = TMaxFBPut(TAK_TIME			,	pSPacket->tak_time			);
		nRet = TMaxFBPut(TAK_WND_NO			,	pSPacket->tak_wnd_no		);
		nRet = TMaxFBPut(TAK_USER_NO		,	pSPacket->tak_user_no		);
		nRet = TMaxFBPut(DEPR_TRML_NO		,	pSPacket->depr_trml_no		);
		nRet = TMaxFBPut(ARVL_TRML_NO		,	pSPacket->arvl_trml_no		);
		nRet = TMaxFBPut(TISSU_DT			,	pSPacket->tissu_dt			);
		nRet = TMaxFBPut(TISSU_TRML_NO		,	pSPacket->tissu_trml_no		);
		nRet = TMaxFBPut(TISSU_WND_NO		,	pSPacket->tissu_wnd_no		);
		nRet = TMaxFBPut(TISSU_SNO			,	pSPacket->tissu_sno			);
		nRet = TMaxFBPut(BEF_AFT_DVS		,	pSPacket->bef_aft_dvs		);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGet(BEF_AFT_DVS	, PST_FIELD(pRPacket, bef_aft_dvs		));
		nRet = TMaxFBGet(REC_NUM		, PST_FIELD(pRPacket, rec_num			));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		// file save
		OperGetFileName(nOperID, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}
		MySetFilePointer(hFile, 0, FILE_END);

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->bef_aft_dvs, sizeof(pRPacket->bef_aft_dvs));
		MyWriteFile(hFile, &nCount, sizeof(int)+1);

		/// memory allocation
		{
			/// 재발행정보 조회 List
			pRPacket->pList = new rtk_tm_rpub_inq_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_rpub_inq_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				//int k, cnt;
				prtk_tm_rpub_inq_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TISSU_DT				,	PST_FIELD(pList, tissu_dt				));
				nRet = TMaxFBGetF(TISSU_TRML_NO			,	PST_FIELD(pList, tissu_trml_no			));
				nRet = TMaxFBGetF(TISSU_WND_NO			,	PST_FIELD(pList, tissu_wnd_no			));
				nRet = TMaxFBGetF(TISSU_SNO				,	PST_FIELD(pList, tissu_sno				));
				nRet = TMaxFBGetF(DEPR_DT				,	PST_FIELD(pList, depr_dt				));
				nRet = TMaxFBGetF(DRTN_CD				,	PST_FIELD(pList, drtn_cd				));
				nRet = TMaxFBGetF(HSPD_CTY_DVS_CD		,	PST_FIELD(pList, hspd_cty_dvs_cd		));
				nRet = TMaxFBGetF(ALCN_DEPR_TRML_NO		,	PST_FIELD(pList, alcn_depr_trml_no		));
				nRet = TMaxFBGetF(ALCN_ARVL_TRML_NO		,	PST_FIELD(pList, alcn_arvl_trml_no		));
				nRet = TMaxFBGetF(ALCN_ROT_NO			,	PST_FIELD(pList, alcn_rot_no			));
				nRet = TMaxFBGetF(DEPR_TRML_NO			,	PST_FIELD(pList, depr_trml_no			));
				nRet = TMaxFBGetF(ARVL_TRML_NO			,	PST_FIELD(pList, arvl_trml_no			));
				nRet = TMaxFBGetF(ALCN_DEPR_TIME		,	PST_FIELD(pList, alcn_depr_time			));
				nRet = TMaxFBGetF(DEPR_TIME				,	PST_FIELD(pList, depr_time				));
				nRet = TMaxFBGetF(BUS_CLS_CD			,	PST_FIELD(pList, bus_cls_cd				));
				nRet = TMaxFBGetF(CACM_CD				,	PST_FIELD(pList, cacm_cd				));
				nRet = TMaxFBGetF(TCK_KND_CD			,	PST_FIELD(pList, tck_knd_cd				));
				nRet = TMaxFBGetF(TISSU_FEE				,	PST_FIELD(pList, tissu_fee				));
				nRet = TMaxFBGetF(SATS_NO				,	PST_FIELD(pList, sats_no				));
				nRet = TMaxFBGetF(TISSU_STA_CD			,	PST_FIELD(pList, tissu_sta_cd			));
				nRet = TMaxFBGetF(PYN_DTL_CD			,	PST_FIELD(pList, pyn_dtl_cd				));
				nRet = TMaxFBGetF(TISSU_TIME			,	PST_FIELD(pList, tissu_time				));
				nRet = TMaxFBGetF(TISSU_USER_NO			,	PST_FIELD(pList, tissu_user_no			));

				nRet = TMaxFBGetF(CARD_NO				,	PST_FIELD(pList, card_no				));
#ifdef __KTC_PROGRAM__
				::ZeroMemory(pList->card_no, sizeof(pList->card_no));
#endif
				nRet = TMaxFBGetF(CARD_APRV_NO			,	PST_FIELD(pList, card_aprv_no			));

				nRet = TMaxFBGetF(USER_KEY_VAL			,	PST_FIELD(pList, user_key_val			));
				nRet = TMaxFBGetF(CSRC_APRV_NO			,	PST_FIELD(pList, csrc_aprv_no			));
				nRet = TMaxFBGetF(USER_DVS_CD			,	PST_FIELD(pList, user_dvs_cd			));

				nRet = TMaxFBGetF(RPUB_TRML_NO			,	PST_FIELD(pList, rpub_trml_no			));
				nRet = TMaxFBGetF(RPUB_WND_NO			,	PST_FIELD(pList, rpub_wnd_no			));
				nRet = TMaxFBGetF(RPUB_USER_NO			,	PST_FIELD(pList, rpub_user_no			));
				nRet = TMaxFBGetF(RPUB_DT				,	PST_FIELD(pList, rpub_dt				));
				nRet = TMaxFBGetF(RPUB_TIME				,	PST_FIELD(pList, rpub_time				));
				nRet = TMaxFBGetF(ALCN_ROT_NM			,	PST_FIELD(pList, alcn_rot_nm			));

				MyWriteFile(hFile, pList, sizeof(rtk_tm_rpub_inq_list_t));
			}
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s - (%s) ", "BEF_AFT_DVS"		,	pRPacket->bef_aft_dvs			);

		/// [티머니고속] 
		for(i = 0; i < nCount; i++)
		{
			//int k, cnt;
			prtk_tm_rpub_inq_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 재발행정보조회, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TISSU_DT"			,	pList->tissu_dt				);
			LOG_WRITE("%30s - (%s) ", "TISSU_TRML_NO"		,	pList->tissu_trml_no		);
			LOG_WRITE("%30s - (%s) ", "TISSU_WND_NO"		,	pList->tissu_wnd_no			);
			LOG_WRITE("%30s - (%s) ", "TISSU_SNO"			,	pList->tissu_sno			);
			LOG_WRITE("%30s - (%s) ", "DEPR_DT"				,	pList->depr_dt				);
			LOG_WRITE("%30s - (%s) ", "DRTN_CD"				,	pList->drtn_cd				);
			LOG_WRITE("%30s - (%s) ", "HSPD_CTY_DVS_CD"		,	pList->hspd_cty_dvs_cd		);
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO"	,	pList->alcn_depr_trml_no	);
			LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO"	,	pList->alcn_arvl_trml_no	);
			LOG_WRITE("%30s - (%s) ", "ALCN_ROT_NO"			,	pList->alcn_rot_no			);
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"		,	pList->depr_trml_no			);
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"		,	pList->arvl_trml_no			);
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TIME"		,	pList->alcn_depr_time		);
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME"			,	pList->depr_time			);
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD"			,	pList->bus_cls_cd			);
			LOG_WRITE("%30s - (%s) ", "CACM_CD"				,	pList->cacm_cd				);
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"			,	pList->tck_knd_cd			);
			LOG_WRITE("%30s - (%s) ", "TISSU_FEE"			,	pList->tissu_fee			);
			LOG_WRITE("%30s - (%s) ", "SATS_NO"				,	pList->sats_no				);
			LOG_WRITE("%30s - (%s) ", "TISSU_STA_CD"		,	pList->tissu_sta_cd			);
			LOG_WRITE("%30s - (%s) ", "PYN_DTL_CD"			,	pList->pyn_dtl_cd			);
			LOG_WRITE("%30s - (%s) ", "TISSU_TIME"			,	pList->tissu_time			);
			LOG_WRITE("%30s - (%s) ", "TISSU_USER_NO"		,	pList->tissu_user_no		);
			
#if (_KTC_CERTIFY_ > 0)
			LOG_WRITE("%30s - (%s) ", "CARD_NO"				,	pList->card_no				);
#endif
			LOG_WRITE("%30s - (%s) ", "CARD_APRV_NO"		,	pList->card_aprv_no			);
			
			LOG_WRITE("%30s - (%s) ", "USER_KEY_VAL"		,	pList->user_key_val			);
			LOG_WRITE("%30s - (%s) ", "CSRC_APRV_NO"		,	pList->csrc_aprv_no			);
			LOG_WRITE("%30s - (%s) ", "USER_DVS_CD"			,	pList->user_dvs_cd			);
			
			LOG_WRITE("%30s - (%s) ", "RPUB_TRML_NO"		,	pList->rpub_trml_no			);
			LOG_WRITE("%30s - (%s) ", "RPUB_WND_NO"			,	pList->rpub_wnd_no			);
			LOG_WRITE("%30s - (%s) ", "RPUB_USER_NO"		,	pList->rpub_user_no			);
			LOG_WRITE("%30s - (%s) ", "RPUB_DT"				,	pList->rpub_dt				);
			LOG_WRITE("%30s - (%s) ", "RPUB_TIME"			,	pList->rpub_time			);
			LOG_WRITE("%30s - (%s) ", "ALCN_ROT_NM"			,	pList->alcn_rot_nm			);
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_TK_RePubTck
 * @details		[티머니고속] 재발행
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_TK_RePubTck(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout;
	char*	pService = "TK_RPubTck";
	pstk_tm_rpub_tck_t pSPacket;
	prtk_tm_rpub_tck_t pRPacket;

	nRet = nTimeout = 0;

	LOG_OPEN();
	LOG_WRITE(" 재발행_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_rpub_tck_t) pData;
	pRPacket = (prtk_tm_rpub_tck_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"		,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"		,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"		,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"		,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (%s) ","TISSU_DT"			,	pSPacket->tissu_dt			);
		LOG_WRITE("%30s - (%s) ","TISSU_TRML_NO"	,	pSPacket->tissu_trml_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_WND_NO"		,	pSPacket->tissu_wnd_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_SNO"		,	pSPacket->tissu_sno			);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS		,	pSPacket->req_pgm_dvs		);
		nRet = TMaxFBPut(REQ_TRML_NO		,	pSPacket->req_trml_no		);
		nRet = TMaxFBPut(REQ_WND_NO			,	pSPacket->req_wnd_no		);
		nRet = TMaxFBPut(REQ_USER_NO		,	pSPacket->req_user_no		);

		nRet = TMaxFBPut(TISSU_DT			,	pSPacket->tissu_dt			);
		nRet = TMaxFBPut(TISSU_TRML_NO		,	pSPacket->tissu_trml_no		);
		nRet = TMaxFBPut(TISSU_WND_NO		,	pSPacket->tissu_wnd_no		);
		nRet = TMaxFBPut(TISSU_SNO			,	pSPacket->tissu_sno			);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGet(INHR_NO				, PST_FIELD(pRPacket, inhr_no				));
		nRet = TMaxFBGet(INVS_NO				, PST_FIELD(pRPacket, invs_no				));
		nRet = TMaxFBGet(FRC_CMM				, PST_FIELD(pRPacket, frc_cmm				));
		nRet = TMaxFBGet(CACM_NM_PRIN_YN		, PST_FIELD(pRPacket, cacm_nm_prin_yn		));
		nRet = TMaxFBGet(BUS_CLS_PRIN_YN		, PST_FIELD(pRPacket, bus_cls_prin_yn		));
		nRet = TMaxFBGet(DEPR_TIME_PRIN_YN		, PST_FIELD(pRPacket, depr_time_prin_yn		));
		nRet = TMaxFBGet(SATS_NO_PRIN_YN		, PST_FIELD(pRPacket, sats_no_prin_yn		));
		nRet = TMaxFBGet(ROT_RDHM_NO_VAL		, PST_FIELD(pRPacket, rot_rdhm_no_val		));
		nRet = TMaxFBGet(CSRC_RGT_NO			, PST_FIELD(pRPacket, csrc_rgt_no			));

		// set memory
		::CopyMemory(&CPubTckTmExpMem::GetInstance()->m_tRespRePubTck, pRPacket, sizeof(rtk_tm_rpub_tck_t));
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s - (%s) ", "INHR_NO"				,	pRPacket->inhr_no				);
		LOG_WRITE("%30s - (%s) ", "INVS_NO"				,	pRPacket->invs_no				);
		LOG_WRITE("%30s - (%s) ", "FRC_CMM"				,	pRPacket->frc_cmm				);
		LOG_WRITE("%30s - (%s) ", "CACM_NM_PRIN_YN"		,	pRPacket->cacm_nm_prin_yn		);
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_PRIN_YN"		,	pRPacket->bus_cls_prin_yn		);
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME_PRIN_YN"	,	pRPacket->depr_time_prin_yn		);
		LOG_WRITE("%30s - (%s) ", "SATS_NO_PRIN_YN"		,	pRPacket->sats_no_prin_yn		);
		LOG_WRITE("%30s - (%s) ", "ROT_RDHM_NO_VAL"		,	pRPacket->rot_rdhm_no_val		);
		LOG_WRITE("%30s - (%s) ", "CSRC_RGT_NO"			,	pRPacket->csrc_rgt_no			);
	}

	///> data proc
	{
		nRet = 1;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		SVC_CM_PubInquiry
 * @details		[티머니고속] 승차권 발행 내역 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CTmExpBusServer::SVC_CM_CanRyInquiry(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, i, nCount;
	char*	pService = "CM_CancRyPt";
	pstk_tm_canry_inq_t pSPacket;
	prtk_tm_canry_inq_t pRPacket;

	i = nCount = nRet = nTimeout = 0;

	LOG_OPEN();
	LOG_WRITE(" 환불내역 조회_start[%s] !!!!", pService);

	pSPacket = (pstk_tm_canry_inq_t) pData;
	pRPacket = (prtk_tm_canry_inq_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// send log
	{
		LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS"		,	pSPacket->req_pgm_dvs		);
		LOG_WRITE("%30s - (%s) ","REQ_TRML_NO"		,	pSPacket->req_trml_no		);
		LOG_WRITE("%30s - (%s) ","REQ_WND_NO"		,	pSPacket->req_wnd_no		);
		LOG_WRITE("%30s - (%s) ","REQ_USER_NO"		,	pSPacket->req_user_no		);

		LOG_WRITE("%30s - (Num%d) ","REC_NUM"		,	*(int *)pSPacket->rec_num	);
		LOG_WRITE("%30s - (%s) ","TAK_DVS_CD"		,	pSPacket->tak_dvs_cd		);
		LOG_WRITE("%30s - (%s) ","SORT_DVS_CD"		,	pSPacket->sort_dvs_cd		);
		LOG_WRITE("%30s - (%s) ","TAK_DT"			,	pSPacket->tak_dt			);
		LOG_WRITE("%30s - (%s) ","TAK_TIME"			,	pSPacket->tak_time			);
		LOG_WRITE("%30s - (%s) ","TAK_WND_NO"		,	pSPacket->tak_wnd_no		);
		LOG_WRITE("%30s - (%s) ","TAK_USER_NO"		,	pSPacket->tak_user_no		);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO"		,	pSPacket->depr_trml_no		);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"		,	pSPacket->arvl_trml_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_DT"			,	pSPacket->tissu_dt			);
		LOG_WRITE("%30s - (%s) ","TISSU_TRML_NO"	,	pSPacket->tissu_trml_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_WND_NO"		,	pSPacket->tissu_wnd_no		);
		LOG_WRITE("%30s - (%s) ","TISSU_SNO"		,	pSPacket->tissu_sno			);
		LOG_WRITE("%30s - (%s) ","BEF_AFT_DVS"		,	pSPacket->bef_aft_dvs		);
	}

	// send data
	{
		nRet = TMaxFBPut(REQ_PGM_DVS		,	pSPacket->req_pgm_dvs		);
		nRet = TMaxFBPut(REQ_TRML_NO		,	pSPacket->req_trml_no		);
		nRet = TMaxFBPut(REQ_WND_NO			,	pSPacket->req_wnd_no		);
		nRet = TMaxFBPut(REQ_USER_NO		,	pSPacket->req_user_no		);

		nRet = TMaxFBPut(REC_NUM			,	pSPacket->rec_num			);
		nRet = TMaxFBPut(TAK_DVS_CD			,	pSPacket->tak_dvs_cd		);
		nRet = TMaxFBPut(SORT_DVS_CD		,	pSPacket->sort_dvs_cd		);
		nRet = TMaxFBPut(TAK_DT				,	pSPacket->tak_dt			);
		nRet = TMaxFBPut(TAK_TIME			,	pSPacket->tak_time			);
		nRet = TMaxFBPut(TAK_WND_NO			,	pSPacket->tak_wnd_no		);
		nRet = TMaxFBPut(TAK_USER_NO		,	pSPacket->tak_user_no		);
		nRet = TMaxFBPut(DEPR_TRML_NO		,	pSPacket->depr_trml_no		);
		nRet = TMaxFBPut(ARVL_TRML_NO		,	pSPacket->arvl_trml_no		);
		nRet = TMaxFBPut(TISSU_DT			,	pSPacket->tissu_dt			);
		nRet = TMaxFBPut(TISSU_TRML_NO		,	pSPacket->tissu_trml_no		);
		nRet = TMaxFBPut(TISSU_WND_NO		,	pSPacket->tissu_wnd_no		);
		nRet = TMaxFBPut(TISSU_SNO			,	pSPacket->tissu_sno			);
		nRet = TMaxFBPut(BEF_AFT_DVS		,	pSPacket->bef_aft_dvs		);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	// recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");

		/// rsp_cd
		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) !!!!", "RSP_CD", pRPacket->rsp_cd);
		if( pRPacket->rsp_cd[1] == 'E' )
		{
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGet(BEF_AFT_DVS	, PST_FIELD(pRPacket, bef_aft_dvs		));
		nRet = TMaxFBGet(REC_NUM		, PST_FIELD(pRPacket, rec_num			));
		::CopyMemory(&nCount, pRPacket->rec_num, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NUM", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		// file save
		OperGetFileName(nOperID, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}
		MySetFilePointer(hFile, 0, FILE_END);

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->bef_aft_dvs, sizeof(pRPacket->bef_aft_dvs));
		MyWriteFile(hFile, &nCount, sizeof(int)+1);

		/// memory allocation
		{
			/// 환불내역 조회 List
			pRPacket->pList = new rtk_tm_canry_inq_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_canry_inq_list_t) * nCount);

			for(i = 0; i < nCount; i++)
			{
				//int k, cnt;
				prtk_tm_canry_inq_list_t pList;

				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(TISSU_DT				,	PST_FIELD(pList, tissu_dt				));
				nRet = TMaxFBGetF(TISSU_TRML_NO			,	PST_FIELD(pList, tissu_trml_no			));
				nRet = TMaxFBGetF(TISSU_WND_NO			,	PST_FIELD(pList, tissu_wnd_no			));
				nRet = TMaxFBGetF(TISSU_SNO				,	PST_FIELD(pList, tissu_sno				));
				nRet = TMaxFBGetF(DEPR_DT				,	PST_FIELD(pList, depr_dt				));
				nRet = TMaxFBGetF(DRTN_CD				,	PST_FIELD(pList, drtn_cd				));
				nRet = TMaxFBGetF(HSPD_CTY_DVS_CD		,	PST_FIELD(pList, hspd_cty_dvs_cd		));
				nRet = TMaxFBGetF(ALCN_DEPR_TRML_NO		,	PST_FIELD(pList, alcn_depr_trml_no		));
				nRet = TMaxFBGetF(ALCN_ARVL_TRML_NO		,	PST_FIELD(pList, alcn_arvl_trml_no		));
				nRet = TMaxFBGetF(ALCN_ROT_NO			,	PST_FIELD(pList, alcn_rot_no			));
				nRet = TMaxFBGetF(DEPR_TRML_NO			,	PST_FIELD(pList, depr_trml_no			));
				nRet = TMaxFBGetF(ARVL_TRML_NO			,	PST_FIELD(pList, arvl_trml_no			));
				nRet = TMaxFBGetF(ALCN_DEPR_TIME		,	PST_FIELD(pList, alcn_depr_time			));
				nRet = TMaxFBGetF(DEPR_TIME				,	PST_FIELD(pList, depr_time				));
				nRet = TMaxFBGetF(BUS_CLS_CD			,	PST_FIELD(pList, bus_cls_cd				));
				nRet = TMaxFBGetF(CACM_CD				,	PST_FIELD(pList, cacm_cd				));
				nRet = TMaxFBGetF(TCK_KND_CD			,	PST_FIELD(pList, tck_knd_cd				));
				nRet = TMaxFBGetF(TISSU_FEE				,	PST_FIELD(pList, tissu_fee				));
				nRet = TMaxFBGetF(SATS_NO				,	PST_FIELD(pList, sats_no				));
				nRet = TMaxFBGetF(TISSU_STA_CD			,	PST_FIELD(pList, tissu_sta_cd			));
				nRet = TMaxFBGetF(PYN_DTL_CD			,	PST_FIELD(pList, pyn_dtl_cd				));
				nRet = TMaxFBGetF(TISSU_TIME			,	PST_FIELD(pList, tissu_time				));
				nRet = TMaxFBGetF(TISSU_USER_NO			,	PST_FIELD(pList, tissu_user_no			));
				//
				nRet = TMaxFBGetF(CARD_NO				,	PST_FIELD(pList, card_no				));
#ifdef __KTC_PROGRAM__
				::ZeroMemory(pList->card_no, sizeof(pList->card_no));
#endif
				nRet = TMaxFBGetF(CARD_APRV_NO			,	PST_FIELD(pList, card_aprv_no			));
				//
				nRet = TMaxFBGetF(USER_KEY_VAL			,	PST_FIELD(pList, user_key_val			));
				nRet = TMaxFBGetF(CSRC_APRV_NO			,	PST_FIELD(pList, csrc_aprv_no			));
				nRet = TMaxFBGetF(USER_DVS_CD			,	PST_FIELD(pList, user_dvs_cd			));
				//
				nRet = TMaxFBGetF(RY_STA_CD				,	PST_FIELD(pList, ry_sta_cd				));
				nRet = TMaxFBGetF(RY_DT					,	PST_FIELD(pList, ry_dt					));
				nRet = TMaxFBGetF(RY_TIME				,	PST_FIELD(pList, ry_time				));
				nRet = TMaxFBGetF(RY_TRML_NO			,	PST_FIELD(pList, ry_trml_no				));
				nRet = TMaxFBGetF(RY_WND_NO				,	PST_FIELD(pList, ry_wnd_no				));
				nRet = TMaxFBGetF(RY_USER_NO			,	PST_FIELD(pList, ry_user_no				));
				//
				nRet = TMaxFBGetF(RY_KND_CD				,	PST_FIELD(pList, ry_knd_cd				));
				nRet = TMaxFBGetF(RY_AMT				,	PST_FIELD(pList, ry_amt					));
				nRet = TMaxFBGetF(DC_RC_AMT				,	PST_FIELD(pList, dc_rc_amt				));

				/// [티머니고속] 환불내역 조회 - 내역 set memory
				MyWriteFile(hFile, pList, sizeof(rtk_tm_canry_inq_list_t));
			}
			MyCloseFile(hFile);
		}
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s - (%s) ", "BEF_AFT_DVS"		,	pRPacket->bef_aft_dvs			);

		/// [티머니고속] 
		for(i = 0; i < nCount; i++)
		{
			//int k, cnt;
			prtk_tm_canry_inq_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 환불내역조회, 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TISSU_DT"			,	pList->tissu_dt				);
			LOG_WRITE("%30s - (%s) ", "TISSU_TRML_NO"		,	pList->tissu_trml_no			);
			LOG_WRITE("%30s - (%s) ", "TISSU_WND_NO"		,	pList->tissu_wnd_no			);
			LOG_WRITE("%30s - (%s) ", "TISSU_SNO"			,	pList->tissu_sno				);
			LOG_WRITE("%30s - (%s) ", "DEPR_DT"				,	pList->depr_dt				);
			LOG_WRITE("%30s - (%s) ", "DRTN_CD"				,	pList->drtn_cd				);
			LOG_WRITE("%30s - (%s) ", "HSPD_CTY_DVS_CD"		,	pList->hspd_cty_dvs_cd		);
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO"	,	pList->alcn_depr_trml_no		);
			LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO"	,	pList->alcn_arvl_trml_no		);
			LOG_WRITE("%30s - (%s) ", "ALCN_ROT_NO"			,	pList->alcn_rot_no			);
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"		,	pList->depr_trml_no			);
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"		,	pList->arvl_trml_no			);
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TIME"		,	pList->alcn_depr_time			);
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME"			,	pList->depr_time				);
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD"			,	pList->bus_cls_cd				);
			LOG_WRITE("%30s - (%s) ", "CACM_CD"				,	pList->cacm_cd				);
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"			,	pList->tck_knd_cd				);
			LOG_WRITE("%30s - (%s) ", "TISSU_FEE"			,	pList->tissu_fee				);
			LOG_WRITE("%30s - (%s) ", "SATS_NO"				,	pList->sats_no				);
			LOG_WRITE("%30s - (%s) ", "TISSU_STA_CD"		,	pList->tissu_sta_cd			);
			LOG_WRITE("%30s - (%s) ", "PYN_DTL_CD"			,	pList->pyn_dtl_cd				);
			LOG_WRITE("%30s - (%s) ", "TISSU_TIME"			,	pList->tissu_time				);
			LOG_WRITE("%30s - (%s) ", "TISSU_USER_NO"		,	pList->tissu_user_no			);
#if (_KTC_CERTIFY_ > 0)
			LOG_WRITE("%30s - (%s) ", "CARD_NO"				,	pList->card_no				);
#endif
			LOG_WRITE("%30s - (%s) ", "CARD_APRV_NO"		,	pList->card_aprv_no			);
			//LOG_WRITE("%30s - (%s) ", "	 	[		+1];"			,	pList->	 	[		+1];	);
			LOG_WRITE("%30s - (%s) ", "USER_KEY_VAL"		,	pList->user_key_val			);
			LOG_WRITE("%30s - (%s) ", "CSRC_APRV_NO"		,	pList->csrc_aprv_no			);
			LOG_WRITE("%30s - (%s) ", "USER_DVS_CD"			,	pList->user_dvs_cd			);
			//LOG_WRITE("%30s - (%s) ", " 	[		+1];	"			,	pList-> 	[		+1];		);
			LOG_WRITE("%30s - (%s) ", "RY_STA_CD"			,	pList->ry_sta_cd				);
			LOG_WRITE("%30s - (%s) ", "RY_DT"				,	pList->ry_dt					);
			LOG_WRITE("%30s - (%s) ", "RY_TIME"				,	pList->ry_time				);
			LOG_WRITE("%30s - (%s) ", "RY_TRML_NO"			,	pList->ry_trml_no				);
			LOG_WRITE("%30s - (%s) ", "RY_WND_NO"			,	pList->ry_wnd_no				);
			LOG_WRITE("%30s - (%s) ", "RY_USER_NO"			,	pList->ry_user_no				);
			//LOG_WRITE("%30s - (%s) ", "	 	[		+1];"			,	pList->	 	[		+1];	);
			LOG_WRITE("%30s - (%s) ", "RY_KND_CD"			,	pList->ry_knd_cd				);
			LOG_WRITE("%30s - (%s) ", "RY_AMT"				,	pList->ry_amt					);
			LOG_WRITE("%30s - (%s) ", "DC_RC_AMT"			,	pList->dc_rc_amt				);
		}
	}

	///> data proc
	{
		nRet = nCount;
	}
	
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 1;
#endif
}

/**
 * @brief		StartProcess
 * @details		Start
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CTmExpBusServer::StartProcess(void)
{
	LOG_INIT();

	LOG_OUT("\n\n ###################### start ###################### ");

	return 0;
}

/**
 * @brief		EndProcess
 * @details		Start
 * @param		int nCommIdx		COM
 * @return		성공 : > 0, 실패 : < 0
 */
int CTmExpBusServer::EndProcess(void)
{
	LOG_OUT(" ###################### end ###################### \n\n");

	return 0;
}



