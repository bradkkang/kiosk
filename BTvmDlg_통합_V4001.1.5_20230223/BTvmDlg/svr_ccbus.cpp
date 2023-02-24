// 
// 
// svr_ccbus.cpp : 시외버스 서버
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

#include "cmn_util.h"
#include "xzzbus_fdl.h"
#include "svr_ccbus.h"
#include "svr_ccbus_st.h"
#include "oper_config.h"
#include "oper_kos_file.h"
#include "dev_tr_main.h"
#include "dev_tr_mem.h"
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

#define LOG_OUT(fmt, ...)		{ CCBusServer::m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ CCBusServer::m_clsLog.HexaDump(x, y, z); }

//#define LOG_OUT(fmt, ...)		
//#define LOG_HEXA(x,y,z)			

#define LOG_OPEN()				{ CCBusServer::m_clsLog.LogOpen(FALSE);  }
#define LOG_WRITE(fmt, ...)		{ CCBusServer::m_clsLog.LogWrite("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_CLOSE()				{ CCBusServer::m_clsLog.LogClose();  }

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
 * @brief		CCBusServer
 * @details		생성자
 */
CCBusServer::CCBusServer()
{
	m_pSendBuff = NULL;
	m_pRecvBuff = NULL;

	m_bVerify = FALSE;
}

/**
 * @brief		~CCBusServer
 * @details		소멸자
 */
CCBusServer::~CCBusServer()
{
	EndProcess();
}

/**
 * @brief		LOG_INIT
 * @details		LOG 파일 초기화
 * @param		None
 * @return		None
 */
void CCBusServer::LOG_INIT(void)
{
	m_clsLog.SetData(30, "\\Log\\TMAX\\ccs");
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		LogOut_Hdr
 * @details		패킷 헤더 LOG 출력
 * @param		char *pData		헤더 데이타
 * @return		None
 */
void CCBusServer::LogOut_Hdr(char *pData)
{
	pstk_head_t pHdr = (pstk_head_t) pData;

	LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS",		pHdr->req_pgm_dvs);
	LOG_WRITE("%30s - (%s) ","REQ_TRML_CD",		pHdr->req_trml_cd);
	LOG_WRITE("%30s - (%s) ","REQ_TRML_WND_NO",	pHdr->req_trml_wnd_no);
	LOG_WRITE("%30s - (%s) ","REQ_TRML_USER_IP",	pHdr->req_trml_user_ip);
}

/**
 * @brief		LogOut_HdrShort
 * @details		패킷 헤더 LOG 출력
 * @param		char *pData		헤더 데이타
 * @return		None
 */
void CCBusServer::LogOut_HdrShort(char *pData)
{
	pstk_head_shrt_t pHdr = (pstk_head_shrt_t) pData;

	LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS",		pHdr->req_pgm_dvs);
	LOG_WRITE("%30s - (%s) ","REQ_SHCT_TRML_CD",	pHdr->req_shct_trml_cd);
	LOG_WRITE("%30s - (%s) ","REQ_TRML_WND_NO",	pHdr->req_trml_wnd_no);
	LOG_WRITE("%30s - (%s) ","REQ_TRML_USER_IP",	pHdr->req_trml_user_ip);
}

/**
 * @brief		LogOut_HdrId
 * @details		패킷 헤더 LOG 출력
 * @param		char *pData		헤더 데이타
 * @return		None
 */
void CCBusServer::LogOut_HdrId(char *pData)
{
	pstk_head_id_t pHdr = (pstk_head_id_t) pData;
	
	LOG_WRITE("%30s - (%s) ","REQ_PGM_DVS",		pHdr->req_pgm_dvs);
	LOG_WRITE("%30s - (%s) ","REQ_TRML_CD",		pHdr->req_trml_cd);
	LOG_WRITE("%30s - (%s) ","REQ_TRML_WND_NO",	pHdr->req_trml_wnd_no);
	LOG_WRITE("%30s - (%s) ","REQ_TRML_USER_IP",	pHdr->req_trml_user_ip);
	LOG_WRITE("%30s - (%s) ","REQ_TRML_USER_ID",	pHdr->req_trml_user_id);
}

/**
 * @brief		SendHeader
 * @details		공통 Header 정보 전송
 * @param		char *pData			공통 헤더정보
 * @return		None
 */
void CCBusServer::SendHeader(char *pData)
{
	int nRet;
	pstk_head_t pHdr = (pstk_head_t) pData;

	nRet = TMaxFBPut(REQ_PGM_DVS,		pHdr->req_pgm_dvs);
	nRet = TMaxFBPut(REQ_TRML_CD,		pHdr->req_trml_cd);
	nRet = TMaxFBPut(REQ_TRML_WND_NO,	pHdr->req_trml_wnd_no);
	nRet = TMaxFBPut(REQ_TRML_USER_IP,	pHdr->req_trml_user_ip);
}

/**
 * @brief		SendShortHeader
 * @details		공통 Header 정보 전송
 * @param		char *pData			공통 헤더정보
 * @return		None
 */
void CCBusServer::SendShortHeader(char *pData)
{
	int nRet;
	pstk_head_shrt_t pHdr = (pstk_head_shrt_t) pData;

	nRet = TMaxFBPut(REQ_PGM_DVS,		pHdr->req_pgm_dvs);
	nRet = TMaxFBPut(REQ_SHCT_TRML_CD,		pHdr->req_shct_trml_cd);
	nRet = TMaxFBPut(REQ_TRML_WND_NO,	pHdr->req_trml_wnd_no);
	nRet = TMaxFBPut(REQ_TRML_USER_IP,	pHdr->req_trml_user_ip);
}

/**
 * @brief		SendIDHeader
 * @details		공통 Header 정보 전송
 * @param		char *pData			공통 헤더정보
 * @return		None
 */
void CCBusServer::SendIDHeader(char *pData)
{
	int nRet;
	pstk_head_id_t pHdr = (pstk_head_id_t) pData;

	nRet = TMaxFBPut(REQ_PGM_DVS,		pHdr->req_pgm_dvs);
	nRet = TMaxFBPut(REQ_TRML_CD,		pHdr->req_trml_cd);
	nRet = TMaxFBPut(REQ_TRML_WND_NO,	pHdr->req_trml_wnd_no);
	nRet = TMaxFBPut(REQ_TRML_USER_IP,	pHdr->req_trml_user_ip);
	nRet = TMaxFBPut(REQ_TRML_USER_ID,	pHdr->req_trml_user_id);
}

/**
 * @brief		Connect
 * @details		서버 접속
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxConnect(void)
{
#if (__USE_TMAX__ > 0)
	int		nRet;
	char	szCurrPath[256];
	char	szFullName[256];
	TPSTART_T* tpInfo = NULL;

	LOG_WRITE(" start !!!!");

	::ZeroMemory(szCurrPath, sizeof(szCurrPath));
	::ZeroMemory(szFullName, sizeof(szFullName));

	Util_GetModulePath(szCurrPath);
	sprintf(szFullName, "%s\\tmax.env", szCurrPath);

	if( GetEnv_IsRealMode() == 0 )
	{	// test mode
		LOG_WRITE("TMAX_CCS_TEST mode !!!!");
		nRet = tmaxreadenv(szFullName, "TMAX_CCS_TEST" );
	}
	else
	{	// real mode
		LOG_WRITE("TMAX_CCS_REAL mode !!!!");
		nRet = tmaxreadenv(szFullName, "TMAX_CCS_REAL" );
	}

	if(nRet == -1)
	{
		LOG_WRITE("tmaxreadenv() failure !!!!");
		goto tmax_fail_proc;
	}

	tpInfo = (TPSTART_T *) tpalloc("TPSTART", NULL, 10240);
	if(tpInfo == NULL)
	{
		LOG_WRITE("tpalloc(TPSTART) failure !!!!");
		goto tmax_fail_proc;
	}

	sprintf(tpInfo->cltname, "");
	sprintf(tpInfo->usrname, "atec_kiosk");
	sprintf(tpInfo->dompwd, "");

	nRet = tpstart(tpInfo);
	if(nRet == -1)
	{
		LOG_WRITE("tpstart() failure !!!!");
		goto tmax_fail_proc;
	}

	if(tpInfo != NULL)
	{
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
int CCBusServer::TMaxFBPut(DWORD dwFieldKey, char *pData)
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
int CCBusServer::TMaxFBPutLen(DWORD dwFieldKey, char *pData, int nDataLen)
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
int CCBusServer::TMaxFBInsert(DWORD dwFieldKey, char *pData, int nIndex, int nDataLen)
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
int CCBusServer::TMaxFBUpdate(DWORD dwFieldKey, char *pData, int nIndex, int nDataLen)
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
int CCBusServer::TMaxTpCall(char *pService, int nTimeout)
{
	int nRet = 0;
	int nRecvLen = 0;

	LOG_WRITE("Service name(%s) !!!!", pService);

#if (__USE_TMAX__ > 0)
	/// 블로킹 타임아웃 시간 변경 (초단위 설정) : 거래는 50초
	tpset_timeout(50);

	nRet = tpcall(pService, (char *)m_pSendBuff, (long) 0, (char **)&m_pRecvBuff, (long *) &nRecvLen, (long) TPNOTRAN);
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
int CCBusServer::TMaxFBGet(DWORD dwFieldKey, char *pData)
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
int CCBusServer::TMaxFBGetTu(DWORD dwFieldKey, char *pData, int nth)
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
int CCBusServer::TMaxFBGetF(DWORD dwFieldKey, char *pData)
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
int CCBusServer::TMaxAlloc(void)
{
#if (__USE_TMAX__ > 0)
	m_pSendBuff = (FBUF *) tpalloc("FIELD", NULL, (1024 * 30));
	if(m_pSendBuff == NULL)
	{
		return -1;
	}

	// 수신 버퍼 할당 함수
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
 * @details		송수신 버퍼 Free
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
void CCBusServer::TMaxFree(void)
{
	LOG_WRITE(" start !!!!");

#if (__USE_TMAX__ > 0)
	if(m_pSendBuff != NULL)
	{
		tpfree((char *)m_pSendBuff);
		m_pSendBuff = NULL;
	}

	if(m_pRecvBuff != NULL)
	{
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
void CCBusServer::TMaxDisconnect(void)
{
	LOG_WRITE(" start !!!!");

#if (__USE_TMAX__ > 0)
 	tpend();
#endif
}

/**
 * @brief		TMaxSvc100
 * @details		(100) 공통코드 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc100(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ReadCmnCd";
	pstk_readcmncd_t pSPacket;
	prtk_readcmncd_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readcmncd_t) pData;
	pRPacket = (prtk_readcmncd_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
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

		nRet = TMaxFBGet(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{
			// 에러코드 조회
			nRet = -98;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGet(CMN_CD_NUM, PST_FIELD(pRPacket, cmn_cd_num));
		::CopyMemory(&nCount, pRPacket->cmn_cd_num, 4);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) nCount = %d !!!!", "cmn_cd_num", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_readcmncd_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readcmncd_list_t) * nCount);
		}

		// file save
		OperGetFileName(OPER_FILE_ID_0100, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->cmn_cd_num, sizeof(pRPacket->cmn_cd_num));
		
		for(i = 0; i < nCount; i++)
		{
			prtk_readcmncd_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(CMN_CD_ID,		PST_FIELD(pList, cmn_cd_id));
			nRet = TMaxFBGetF(CMN_CD_NM,		PST_FIELD(pList, cmn_cd_nm));
			nRet = TMaxFBGetF(CMN_CD_ENG_NM,	PST_FIELD(pList, cmn_cd_eng_nm));
			nRet = TMaxFBGetF(HGRN_CMN_CD_ID,	PST_FIELD(pList, hgrn_cmn_cd_id));

			MyWriteFile(hFile, pList, sizeof(rtk_readcmncd_list_t));
		}
		MyCloseFile(hFile);
	}

	// recv log
	{
		LOG_WRITE("%30s ", "TMaxSvc100() RecvPacket");
		LOG_WRITE("%30s - (%s) ", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%d) ", "CMN_CD_NUM", nCount);
		for(i = 0; i < nCount; i++)
		{
			prtk_readcmncd_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "CMN_CD_ID",		PST_FIELD(pList, cmn_cd_id));
			LOG_WRITE("%30s - (%s) ", "CMN_CD_NM",		PST_FIELD(pList, cmn_cd_nm));
			LOG_WRITE("%30s - (%s) ", "CMN_CD_ENG_NM",	PST_FIELD(pList, cmn_cd_eng_nm));
			LOG_WRITE("%30s - (%s) ", "HGRN_CMN_CD_ID",	PST_FIELD(pList, hgrn_cmn_cd_id));
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

static int MY_SetLocalTime(char *pDateTime)
{
	HANDLE h_token;
	TOKEN_PRIVILEGES privilege_info;

	// 현재 프로세스의 권한과 관련된 정보를 변경하기 위해 토큰정보를 엽니다.
	//if( !OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &h_token) )
	if( !OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS_P, &h_token) )
	{
		// 권한과 관련된 정보 접근에 실패함..
		TRACE("kiosk_error_code = %lu", GetLastError());
		return 0;
	}

	// 현재 프로세스가 SE_SHUTDOWN_NAME 권한을 사용할수 있도록 설정한다.
	//LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &privilege_info.Privileges[0].Luid);
	LookupPrivilegeValue(NULL, SE_SYSTEMTIME_NAME, &privilege_info.Privileges[0].Luid);
	privilege_info.PrivilegeCount = 1;
	privilege_info.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	// 지정한 값으로 권한을 조정한다.
	AdjustTokenPrivileges(h_token, FALSE, &privilege_info, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	if(GetLastError() != ERROR_SUCCESS)
	{
		// 권한 조정에 실패한 경우...
		return -1;
	}

	SetLocalTime((SYSTEMTIME *)pDateTime);

	return 1;
}


/**
 * @brief		TMaxSvc101
 * @details		(101) 컴퓨터 인증
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc101(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_AuthCmpt";
	pstk_authcmpt_t pSPacket;
	prtk_authcmpt_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_authcmpt_t) pData;
	pRPacket = (prtk_authcmpt_t) retBuf;

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
		LogOut_HdrShort((char *)&pSPacket->hdr_st);
		LOG_WRITE("%30s - (%s) ", "REQ_DVS",			pSPacket->req_dvs);
		LOG_WRITE("%30s - (%s) ", "ACS_CMPT_INF",		_strlwr(pSPacket->acs_cmpt_inf));
	}

	// send data
	{
		SendShortHeader((char *)&pSPacket->hdr_st);
		nRet = TMaxFBPut(REQ_DVS,			pSPacket->req_dvs);
		nRet = TMaxFBPut(ACS_CMPT_INF,		pSPacket->acs_cmpt_inf);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( (n_rsp_cd != 141) && (n_rsp_cd != 516) )
		{	// 에러코드 조회
			if(n_rsp_cd == 519)
			{
				;// MAC설정정보가 존재하지 않습니다.
			}
			if(n_rsp_cd == 517)
			{
				;// 인증 터미널 정보가 존재하지 않습니다.
			}
//			nRet = -98;
//			nRet = 1;
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(SERV_DT,		PST_FIELD(pRPacket, serv_dt));
		nRet = TMaxFBGetF(SERV_TIME,	PST_FIELD(pRPacket, serv_time));
		nRet = TMaxFBGetF(SHCT_TRML_CD,	PST_FIELD(pRPacket, shct_trml_cd));
		nRet = TMaxFBGetF(TRML_CD,		PST_FIELD(pRPacket, trml_cd));

		// file save
		OperGetFileName(OPER_FILE_ID_0101, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}
		MyWriteFile(hFile, pRPacket, sizeof(rtk_authcmpt_t));
		MyCloseFile(hFile);

		// set terminal_code_7
		SetTrmlCode(SVR_DVS_CCBUS, pRPacket->trml_cd);
		::CopyMemory(CConfigTkMem::GetInstance()->trml_cd, pRPacket->trml_cd, sizeof(pRPacket->trml_cd));

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
				MY_SetLocalTime((char *)&st);
				LOG_WRITE("nReturn(%d), 시간동기화 : %04d/%02d/%02d %02d:%02d:%02d ", nRet, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
				SetLocalTime(&st);
			}
		}
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ", "RSP_CD",		PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ", "SERV_DT",		PST_FIELD(pRPacket, serv_dt));
		LOG_WRITE("%30s - (%s) ", "SERV_TIME",	PST_FIELD(pRPacket, serv_time));
		LOG_WRITE("%30s - (%s) ", "SHCT_TRML_CD",	PST_FIELD(pRPacket, shct_trml_cd));
		LOG_WRITE("%30s - (%s) ", "TRML_CD",		PST_FIELD(pRPacket, trml_cd));
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
 * @brief		TMaxSvc102
 * @details		(102) 공통코드 상세 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc102(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	 nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_CmnCdDtl";
	pstk_cmncddtl_t pSPacket;
	prtk_cmncddtl_t pRPacket;
	prtk_cmncddtl_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_cmncddtl_t) pData;
	pRPacket = (prtk_cmncddtl_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
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

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(CMN_DTL_NUM,	PST_FIELD(pRPacket, cmn_dtl_num));
		nCount = *(int *) pRPacket->cmn_dtl_num;
		LOG_WRITE("%30s - (%d) ", "CMN_DTL_NUM",	nCount);
		if( nCount <= 0 )
		{
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_cmncddtl_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_cmncddtl_list_t) * nCount);
		}

		OperGetFileName(OPER_FILE_ID_0102, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->cmn_dtl_num, sizeof(pRPacket->cmn_dtl_num));

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(CMN_CD_ID,			PST_FIELD(pList, cmn_cd_id));
			nRet = TMaxFBGetF(CMN_CD_VAL,			PST_FIELD(pList, cmn_cd_val));
			nRet = TMaxFBGetF(CD_VAL_NM,			PST_FIELD(pList, cd_val_nm));
			nRet = TMaxFBGetF(CD_VAL_ENG_NM,		PST_FIELD(pList, cd_val_eng_nm));
			nRet = TMaxFBGetF(CD_VAL_MRK_SEQ,		PST_FIELD(pList, cd_val_mrk_seq));
			nRet = TMaxFBGetF(SBRD_CMN_CD_ID,		PST_FIELD(pList, sbrd_cmn_cd_id));
			nRet = TMaxFBGetF(CD_RFRN_VAL_1,		PST_FIELD(pList, cd_rfrn_val_1));
			nRet = TMaxFBGetF(CD_RFRN_VAL_DESC_1,	PST_FIELD(pList, cd_rfrn_val_desc_1));
			nRet = TMaxFBGetF(CD_RFRN_VAL_2,		PST_FIELD(pList, cd_rfrn_val_2));
			nRet = TMaxFBGetF(CD_RFRN_VAL_DESC_2,	PST_FIELD(pList, cd_rfrn_val_desc_2));
			nRet = TMaxFBGetF(CD_RFRN_VAL_3,		PST_FIELD(pList, cd_rfrn_val_3));
			nRet = TMaxFBGetF(CD_RFRN_VAL_DESC_3,	PST_FIELD(pList, cd_rfrn_val_desc_3));
		
			MyWriteFile(hFile, pList, sizeof(rtk_cmncddtl_list_t));	
		}
		MyCloseFile(hFile);
	}

	///> recv log
	{
		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);

			LOG_WRITE("%30s - (%s) ", "CMN_CD_ID",			PST_FIELD(pList, cmn_cd_id));
			LOG_WRITE("%30s - (%s) ", "CMN_CD_VAL",			PST_FIELD(pList, cmn_cd_val));
			LOG_WRITE("%30s - (%s) ", "CD_VAL_NM",			PST_FIELD(pList, cd_val_nm));
			LOG_WRITE("%30s - (%s) ", "CD_VAL_ENG_NM",		PST_FIELD(pList, cd_val_eng_nm));
			LOG_WRITE("%30s - (number = %d) ", "CD_VAL_MRK_SEQ", *(int *)pList->cd_val_mrk_seq);
			LOG_WRITE("%30s - (%s) ", "SBRD_CMN_CD_ID",		PST_FIELD(pList, sbrd_cmn_cd_id));
			LOG_WRITE("%30s - (%s) ", "CD_RFRN_VAL_1",		PST_FIELD(pList, cd_rfrn_val_1));
			LOG_WRITE("%30s - (%s) ", "CD_RFRN_VAL_DESC_1",	PST_FIELD(pList, cd_rfrn_val_desc_1));
			LOG_WRITE("%30s - (%s) ", "CD_RFRN_VAL_2",		PST_FIELD(pList, cd_rfrn_val_2));
			LOG_WRITE("%30s - (%s) ", "CD_RFRN_VAL_DESC_2",	PST_FIELD(pList, cd_rfrn_val_desc_2));
			LOG_WRITE("%30s - (%s) ", "CD_RFRN_VAL_3",		PST_FIELD(pList, cd_rfrn_val_3));
			LOG_WRITE("%30s - (%s) ", "CD_RFRN_VAL_DESC_3",	PST_FIELD(pList, cd_rfrn_val_desc_3));
		}
	}

	// data 분석
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
 * @brief		TMaxSvc106
 * @details		(106) 버스운수사코드 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc106(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ReadBusCacm";
	pstk_readbuscacm_t pSPacket;
	prtk_readbuscacm_t pRPacket;
	prtk_readbuscacm_list_t	pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readbuscacm_t) pData;
	pRPacket = (prtk_readbuscacm_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		return -1;
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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
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

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(BUS_CACM_CD_NUM,	PST_FIELD(pRPacket, bus_cacm_cd_num));
		nCount = *(int *)pRPacket->bus_cacm_cd_num;
		if( nCount <= 0 )
		{	// count error
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		OperGetFileName(OPER_FILE_ID_0106, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		nRet = MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		nRet = MyWriteFile(hFile, pRPacket->bus_cacm_cd_num, sizeof(pRPacket->bus_cacm_cd_num));

		{
			pRPacket->pList = new rtk_readbuscacm_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readbuscacm_list_t) * nCount);
		}

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(BUS_CACM_CD,			PST_FIELD(pList, bus_cacm_cd));
			nRet = TMaxFBGetF(BUS_CACM_NM,			PST_FIELD(pList, bus_cacm_nm));
			nRet = TMaxFBGetF(CD_ENG_NM,			PST_FIELD(pList, cd_eng_nm));
			nRet = TMaxFBGetF(BUS_CACM_SHCT_NM,		PST_FIELD(pList, bus_cacm_shct_nm));
			nRet = TMaxFBGetF(BUS_CACM_BRN,			PST_FIELD(pList, bus_cacm_brn));
			nRet = TMaxFBGetF(BUS_CACM_ENG_NM,		PST_FIELD(pList, bus_cacm_eng_nm));
			nRet = TMaxFBGetF(BUS_CACM_TEL_NO,		PST_FIELD(pList, bus_cacm_tel_no));

			MyWriteFile(hFile, pList, sizeof(rtk_readbuscacm_list_t));
		}
		MyCloseFile(hFile);
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%d) ", "BUS_CACM_CD_NUM",	nCount);
		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "BUS_CACM_CD",		PST_FIELD(pList, bus_cacm_cd));
			LOG_WRITE("%30s - (%s) ", "BUS_CACM_NM",		PST_FIELD(pList, bus_cacm_nm));
			LOG_WRITE("%30s - (%s) ", "CD_ENG_NM",		PST_FIELD(pList, cd_eng_nm));
			LOG_WRITE("%30s - (%s) ", "BUS_CACM_SHCT_NM",	PST_FIELD(pList, bus_cacm_shct_nm));
			LOG_WRITE("%30s - (%s) ", "BUS_CACM_BRN",		PST_FIELD(pList, bus_cacm_brn));
			LOG_WRITE("%30s - (%s) ", "BUS_CACM_ENG_NM",	PST_FIELD(pList, bus_cacm_eng_nm));
			LOG_WRITE("%30s - (%s) ", "BUS_CACM_TEL_NO",	PST_FIELD(pList, bus_cacm_tel_no));
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
 * @brief		TMaxSvc107
 * @details		(107) 버스등급코드 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc107(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ReadBusCls";
	pstk_readbuscls_t pSPacket;
	prtk_readbuscls_t pRPacket;
	prtk_readbuscls_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readbuscls_t) pData;
	pRPacket = (prtk_readbuscls_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
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

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(BUS_CLS_CD_NUM,	PST_FIELD(pRPacket, bus_cls_cd_num));
		nCount = *(int *)pRPacket->bus_cls_cd_num;
		if(nCount <= 0)
		{	// count error
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_readbuscls_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readbuscls_list_t) * nCount);
		}

		OperGetFileName(OPER_FILE_ID_0107, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->bus_cls_cd_num, sizeof(pRPacket->bus_cls_cd_num));

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(BUS_CLS_CD,			PST_FIELD(pList, bus_cls_cd));
			nRet = TMaxFBGetF(BUS_CLS_NM,			PST_FIELD(pList, bus_cls_nm));
			nRet = TMaxFBGetF(ROT_KND_CD,			PST_FIELD(pList, rot_knd_cd));
			nRet = TMaxFBGetF(CD_ENG_NM,			PST_FIELD(pList, cd_eng_nm));
			nRet = TMaxFBGetF(BUS_CLS_SHCT_NM,		PST_FIELD(pList, bus_cls_shct_nm));
			nRet = TMaxFBGetF(BUS_CLS_MRK_SEQ,		PST_FIELD(pList, bus_cls_mrk_seq));
			//pList->n_bus_cls_mrk_seq = *(int *)pList->bus_cls_mrk_seq;
			nRet = TMaxFBGetF(BUS_CLS_ENG_NM,		PST_FIELD(pList, bus_cls_eng_nm));

			MyWriteFile(hFile, pList, sizeof(rtk_readbuscls_list_t));
		}
		MyCloseFile(hFile);
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%d) ", "BUS_CLS_CD_NUM",	nCount);
		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD",		PST_FIELD(pList, bus_cls_cd));
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_NM",		PST_FIELD(pList, bus_cls_nm));
			LOG_WRITE("%30s - (%s) ", "ROT_KND_CD",		PST_FIELD(pList, rot_knd_cd));
			LOG_WRITE("%30s - (%s) ", "CD_ENG_NM",		PST_FIELD(pList, cd_eng_nm));
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_SHCT_NM",	PST_FIELD(pList, bus_cls_shct_nm));
			LOG_WRITE("%30s - (number = %d) ", "BUS_CLS_MRK_SEQ", *(int *)pList->bus_cls_mrk_seq);
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_ENG_NM",  PST_FIELD(pList, bus_cls_eng_nm));
		}
	}

	// data proc
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
 * @brief		TMaxSvc108
 * @details		(108) 버스티켓종류코드 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc108(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ReadTckKnd";
	pstk_readtckknd_t pSPacket;
	prtk_readtckknd_t pRPacket;
	prtk_readtckknd_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readtckknd_t) pData;
	pRPacket = (prtk_readtckknd_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
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

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(BUS_TCK_KND_CD_NUM,	PST_FIELD(pRPacket, bus_tck_knd_cd_num));
		nCount = *(int *) pRPacket->bus_tck_knd_cd_num;
		if( nCount <= 0 )
		{	// count error
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_readtckknd_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readtckknd_list_t) * nCount);
		}

		OperGetFileName(OPER_FILE_ID_0108, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->bus_tck_knd_cd_num, sizeof(pRPacket->bus_tck_knd_cd_num));

		for(i = 0; i < nCount; i++)
		{
			int nLen = 0;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(BUS_TCK_KND_CD,		PST_FIELD(pList, bus_tck_knd_cd			));
			nRet = TMaxFBGetF(BUS_TCK_KND_NM,		PST_FIELD(pList, bus_tck_knd_nm			));
			nRet = TMaxFBGetF(ROT_KND_CD,			PST_FIELD(pList, rot_knd_cd				));
			nRet = TMaxFBGetF(CD_ENG_NM,			PST_FIELD(pList, cd_eng_nm				));
			nRet = TMaxFBGetF(BUS_TCK_KND_SHCT_NM,	PST_FIELD(pList, bus_tck_knd_shct_nm	));
			nRet = TMaxFBGetF(BUS_TCK_MRK_SEQ,		PST_FIELD(pList, bus_tck_mrk_seq		));
			//pList->n_bus_tck_mrk_seq = *(int *)pList->bus_tck_mrk_seq;
			nRet = TMaxFBGetF(BUS_TCK_KND_ENG_NM,	PST_FIELD(pList, bus_tck_knd_eng_nm		));

			if( GetEnvOperCorp() == KUMHO_OPER_CORP )
			{	/// 금호터미널인 경우...
				::ZeroMemory(m_szBuffer, sizeof(m_szBuffer));
				strcpy(m_szBuffer, pList->bus_tck_knd_nm);
				nLen = strlen(m_szBuffer);

				::ZeroMemory(pList->bus_tck_knd_nm, sizeof(pList->bus_tck_knd_nm));

				if( memcmp(pList->bus_tck_knd_cd, "IG", 2) == 0)
				{	/// 어른
#if 0				
					if( (nLen - 4) > 0 )
					{
						sprintf(pList->bus_tck_knd_nm, "어른%s", &m_szBuffer[4]);
					}
					else
					{
						sprintf(pList->bus_tck_knd_nm, "어른");
					}
#else
					sprintf(pList->bus_tck_knd_nm, "어른");
					sprintf(pList->bus_tck_knd_eng_nm, "Adult");
#endif
				}
				else if( memcmp(pList->bus_tck_knd_cd, "IM", 2) == 0)
				{	/// 청소년
#if 0
					if( (nLen - 4) > 0 )
					{
						sprintf(pList->bus_tck_knd_nm, "청소년%s", &m_szBuffer[4]);
					}
					else
					{
						sprintf(pList->bus_tck_knd_nm, "청소년");
					}
#else
					/* // 20221130 DEL~
					sprintf(pList->bus_tck_knd_nm, "청소년");
					//sprintf(pList->bus_tck_knd_eng_nm, "Student");	// 20211006 DEL
					sprintf(pList->bus_tck_knd_eng_nm, "Youth");		// 20211006 MOD
					*/ // 20221130 ~DEL
					// 20221130 ADD~
					if( (nLen - 4) > 0 )
					{
						sprintf(pList->bus_tck_knd_nm, "청소년%s", &m_szBuffer[4]);
						sprintf(pList->bus_tck_knd_eng_nm, "Youth%s", &m_szBuffer[4]);
					}
					else
					{
						sprintf(pList->bus_tck_knd_nm, "청소년");
					}
					// 20221130 ~ADD
#endif
				}
				else if( memcmp(pList->bus_tck_knd_cd, "IC", 2) == 0)
				{	/// 어린이
#if 0
					if( (nLen - 4) > 0 )
					{
						sprintf(pList->bus_tck_knd_nm, "어린이%s", &m_szBuffer[4]);
					}
					else
					{
						sprintf(pList->bus_tck_knd_nm, "어린이");
					}
#else
					sprintf(pList->bus_tck_knd_nm, "어린이");
					//sprintf(pList->bus_tck_knd_eng_nm, "Child");	// 20211006 DEL
					sprintf(pList->bus_tck_knd_eng_nm, "Children");	// 20211006 MOD
#endif
				}
				else
				{
					sprintf(pList->bus_tck_knd_nm, "%s", m_szBuffer);
				}
			}
			MyWriteFile(hFile, pList, sizeof(rtk_readtckknd_list_t));
		}
		MyCloseFile(hFile);
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%d) ","BUS_CLS_CD_NUM", nCount);
		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD",			PST_FIELD(pList, bus_tck_knd_cd));
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_NM",			PST_FIELD(pList, bus_tck_knd_nm));
			LOG_WRITE("%30s - (%s) ", "ROT_KND_CD",				PST_FIELD(pList, rot_knd_cd));
			LOG_WRITE("%30s - (%s) ", "CD_ENG_NM",				PST_FIELD(pList, cd_eng_nm));
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_SHCT_NM",		PST_FIELD(pList, bus_tck_knd_shct_nm));
			LOG_WRITE("%30s - (number = %d) ", "BUS_TCK_MRK_SEQ",	*(int *)pList->bus_tck_mrk_seq);
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_ENG_NM",		PST_FIELD(pList, bus_tck_knd_eng_nm));
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
 * @brief		TMaxSvc114
 * @details		(114) 사용자기본 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc114(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ReadUserBsc";
	pstk_readuserbsc_t pSPacket;
	prtk_readuserbsc_t pRPacket;
	prtk_readuserbsc_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readuserbsc_t) pData;
	pRPacket = (prtk_readuserbsc_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -98;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(USER_BSC_NUM,	PST_FIELD(pRPacket, user_bsc_num));
		::CopyMemory(&nCount, pRPacket->user_bsc_num, 4);
		if(nCount <= 0)
		{
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_readuserbsc_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readuserbsc_list_t) * nCount);
		}

		for(i = 0; i < nCount; i++)
		{
			char *pID;

			pID = CConfigTkMem::GetInstance()->req_trml_user_id;

			pList = &pRPacket->pList[i];

 			nRet = TMaxFBGetF(USER_ID,				PST_FIELD(pList, user_id));
 			nRet = TMaxFBGetF(USER_NM,				PST_FIELD(pList, user_nm));
 			nRet = TMaxFBGetF(CTY_BUS_USER_DVS_CD,	PST_FIELD(pList, cty_bus_user_dvs_cd));
 			nRet = TMaxFBGetF(USER_HNDH_TEL_NO,		PST_FIELD(pList, user_hndh_tel_no));
 			nRet = TMaxFBGetF(USER_PWD,				PST_FIELD(pList, user_pwd));
 			nRet = TMaxFBGetF(USER_NO,				PST_FIELD(pList, user_no));
 			nRet = TMaxFBGetF(BUS_CACM_CD,			PST_FIELD(pList, bus_cacm_cd));
 			nRet = TMaxFBGetF(TISU_LTN_DRTM,		PST_FIELD(pList, tisu_ltn_drtm));
 			//pList->n_tisu_ltn_drtm = *(int *)pList->tisu_ltn_drtm;
 			nRet = TMaxFBGetF(TRML_CD,				PST_FIELD(pList, trml_cd));
 			nRet = TMaxFBGetF(LST_LGN_DT,			PST_FIELD(pList, lst_lgn_dt));
 			nRet = TMaxFBGetF(DLT_DT,				PST_FIELD(pList, dlt_dt));
 			nRet = TMaxFBGetF(PWD_MOD_DT,			PST_FIELD(pList, pwd_mod_dt));
 			nRet = TMaxFBGetF(BUS_CLS_CHC_YN,		PST_FIELD(pList, bus_cls_chc_yn));
 			nRet = TMaxFBGetF(DEPR_TIME_CHC_YN,		PST_FIELD(pList, depr_time_chc_yn));
 			nRet = TMaxFBGetF(LGN_YN,				PST_FIELD(pList, lgn_yn));

			if( memcmp(pID, pList->user_id, strlen(pID)) == 0 )
			{
				sprintf(CConfigTkMem::GetInstance()->user_nm, "%s", pList->user_nm);

				LOG_WRITE("로그인 ID(%s), nm(%s)..", pList->user_id, pList->user_nm);
			}

		}
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%d) ","BUS_CLS_CD_NUM", nCount);

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%.*s) ", "USER_ID",			sizeof(pList->user_id),				PST_FIELD(pList, user_id));
			LOG_WRITE("%30s - (%.*s) ", "USER_NM",			sizeof(pList->user_nm),				PST_FIELD(pList, user_nm));
			LOG_WRITE("%30s - (%.*s) ", "CTY_BUS_USER_DVS_CD", sizeof(pList->cty_bus_user_dvs_cd),PST_FIELD(pList, cty_bus_user_dvs_cd));
			LOG_WRITE("%30s - (%.*s) ", "USER_HNDH_TEL_NO",	sizeof(pList->user_hndh_tel_no),	PST_FIELD(pList, user_hndh_tel_no));
			LOG_WRITE("%30s - (%.*s) ", "USER_PWD",			sizeof(pList->user_pwd),			PST_FIELD(pList, user_pwd));
			LOG_WRITE("%30s - (%.*s) ", "USER_NO",			sizeof(pList->user_no),				PST_FIELD(pList, user_no));
			LOG_WRITE("%30s - (%.*s) ", "BUS_CACM_CD",		sizeof(pList->bus_cacm_cd),			PST_FIELD(pList, bus_cacm_cd));
			//LOG_WRITE("%30s - (%.*s) ", "TISU_LTN_DRTM",		sizeof(pRPacket->pList[i].tisu_ltn_drtm),		ST_FIELD(pRPacket->pList[i], tisu_ltn_drtm));
			LOG_WRITE("%30s - (number = %d) ", "TISU_LTN_DRTM", *(int *)pList->tisu_ltn_drtm);
			LOG_WRITE("%30s - (%.*s) ", "TRML_CD",			sizeof(pList->trml_cd),				PST_FIELD(pList, trml_cd));
			LOG_WRITE("%30s - (%.*s) ", "LST_LGN_DT",			sizeof(pList->lst_lgn_dt),			PST_FIELD(pList, lst_lgn_dt));
			LOG_WRITE("%30s - (%.*s) ", "DLT_DT",				sizeof(pList->dlt_dt),				PST_FIELD(pList, dlt_dt));
			LOG_WRITE("%30s - (%.*s) ", "PWD_MOD_DT",			sizeof(pList->pwd_mod_dt),			PST_FIELD(pList, pwd_mod_dt));
			LOG_WRITE("%30s - (%.*s) ", "BUS_CLS_CHC_YN",		sizeof(pList->bus_cls_chc_yn),		PST_FIELD(pList, bus_cls_chc_yn));
			LOG_WRITE("%30s - (%.*s) ", "DEPR_TIME_CHC_YN",	sizeof(pList->depr_time_chc_yn),	PST_FIELD(pList, depr_time_chc_yn));
			LOG_WRITE("%30s - (%.*s) ", "LGN_YN",				sizeof(pList->lgn_yn),				PST_FIELD(pList, lgn_yn));
		}
	}

	// data proc
	{
		nRet = nCount;

		/***
		for(i = 0; i < nCount; i++)
		{
			char *pUserID;

			pList = &pRPacket->pList[i];
// 			pUserID = Transact_GetData(USER_ID);
// 
// 			if( strcmp(pUserID, pList->user_id) == 0 )
// 			{
// 				// 사용자명
// 				Transact_SetData(USER_NM, pList->user_nm, sizeof(pList->user_nm));
// 				// 발권제한시간
// 				Transact_SetData(TISU_LTN_DRTM, (char *)pList->tisu_ltn_drtm, sizeof(pList->tisu_ltn_drtm));
// 			}
		}
		**/
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
 * @brief		TMaxSvc118
 * @details		(118) 터미널창구상세 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc118(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_TrmlWndDtl";
	pstk_trmwnddtl_t pSPacket;
	prtk_trmwnddtl_t pRPacket;
	prtk_trmwnddtl_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_trmwnddtl_t) pData;
	pRPacket = (prtk_trmwnddtl_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(TRML_WND_DTL_NUM,	PST_FIELD(pRPacket, trml_wnd_dtl_num));
		::CopyMemory(&nCount, pRPacket->trml_wnd_dtl_num, 4);
		if(nCount <= 0)
		{
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_trmwnddtl_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_trmwnddtl_list_t) * nCount);
		}

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(WND_NO,				PST_FIELD(pList, wnd_no));
			nRet = TMaxFBGetF(WND_NM,				PST_FIELD(pList, wnd_nm));
			nRet = TMaxFBGetF(TISU_LTN_DRTM,		PST_FIELD(pList, tisu_ltn_drtm));
			nRet = TMaxFBGetF(BUS_CLS_CHC_YN,		PST_FIELD(pList, bus_cls_chc_yn));
			nRet = TMaxFBGetF(DEPR_TIME_CHC_YN,		PST_FIELD(pList, depr_time_chc_yn));
			nRet = TMaxFBGetF(CRDT_CARD_VANC_CD,	PST_FIELD(pList, crdt_card_vanc_cd));

			/// 창구번호가 같으면...
			if( memcmp(pList->wnd_no, pSPacket->hdr.req_trml_wnd_no, strlen(pSPacket->hdr.req_trml_wnd_no)) == 0 )
			{
				// *
				//Transact_SetData((DWORD) TISU_LTN_DRTM, pList->tisu_ltn_drtm, sizeof(pList->tisu_ltn_drtm));
				::CopyMemory(CConfigTkMem::GetInstance()->tisu_ltn_drtm, pList->tisu_ltn_drtm, sizeof(pList->tisu_ltn_drtm));
			}
		}
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%d) ","TRML_WND_DTL_NUM",	nCount);

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "WND_NO",				PST_FIELD(pList, wnd_no));
			LOG_WRITE("%30s - (%s) ", "WND_NM",				PST_FIELD(pList, wnd_nm));
			LOG_WRITE("%30s - (number = %d) ", "TISU_LTN_DRTM",	*(int *)pList->tisu_ltn_drtm);
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CHC_YN",		PST_FIELD(pList, bus_cls_chc_yn));
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME_CHC_YN",		PST_FIELD(pList, depr_time_chc_yn));
			LOG_WRITE("%30s - (%s) ", "CRDT_CARD_VANC_CD",	PST_FIELD(pList, crdt_card_vanc_cd));
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
 * @brief		TMaxSvc119
 * @details		(119) 컴퓨터설정상세 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc119(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
//	int n_rsp_cd;
	char*	pService = "TK_CmptStupDtl";
	pstk_cmptstupdtl_t pSPacket;
	prtk_cmptstupdtl_t pRPacket;
	prtk_cmptstupdtl_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_cmptstupdtl_t) pData;
	pRPacket = (prtk_cmptstupdtl_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
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

	///> recv data
	{
		int n_rsp_cd;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(CMPT_STUP_DTL_NUM,	PST_FIELD(pRPacket, cmpt_stup_dtl_num));
		::CopyMemory(&nCount, pRPacket->cmpt_stup_dtl_num, 4);
		LOG_WRITE("%30s - (%d) ","CMPT_STUP_DTL_NUM",	nCount);
		if(nCount <= 0)
		{	// count error
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_cmptstupdtl_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_cmptstupdtl_list_t) * nCount);
		}

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(CMPT_STUP_DVS_CD,	PST_FIELD(pList, cmpt_stup_dvs_cd));
			nRet = TMaxFBGetF(CMPT_STUP_VAL,	PST_FIELD(pList, cmpt_stup_val));

			if( memcmp(pList->cmpt_stup_dvs_cd, "163", 3) == 0 )
			{
				if( pList->cmpt_stup_val[0] == 'Y' )
				{
					// isKTC_YN = true;
				}
			}
		}
	}

	///> recv log
	{
		for(i = 0; i < nCount; i++)
		{
			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%.*s) ", "CMPT_STUP_DVS_CD", sizeof(pRPacket->pList[i].cmpt_stup_dvs_cd),	ST_FIELD(pRPacket->pList[i], cmpt_stup_dvs_cd));
			LOG_WRITE("%30s - (%.*s) ", "CMPT_STUP_VAL",	  sizeof(pRPacket->pList[i].cmpt_stup_val),		ST_FIELD(pRPacket->pList[i], cmpt_stup_val));
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
 * @brief		TMaxSvc120
 * @details		(120) 터미널 코드 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc120(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nRealCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ReadTrmlCd";
	pstk_readtrmlcd_t	pSPacket;
	prtk_readtrmlcd_t	pRPacket;
	prtk_readtrmlcd_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readtrmlcd_t) pData;
	pRPacket = (prtk_readtrmlcd_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;
		char *pBuff;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(TRML_CD_NUM,	PST_FIELD(pRPacket, trml_cd_num));
		::CopyMemory(&nCount, pRPacket->trml_cd_num, 4);
		LOG_WRITE("%30s - (%d) ","TRML_CD_NUM",	nCount);
		if( nCount <= 0 )
		{	// count error
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_readtrmlcd_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readtrmlcd_list_t) * nCount);
		}

		nRealCount = 0;
		for(i = 0; i < nCount; i++)
		{
			prtk_readtrmlcd_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(TRML_CD,				PST_FIELD(pList, trml_cd));
			nRet = TMaxFBGetF(TRML_NM,				PST_FIELD(pList, trml_nm));
			nRet = TMaxFBGetF(SHCT_TRML_CD,			PST_FIELD(pList, shct_trml_cd));
			nRet = TMaxFBGetF(SHCT_ARVL_TRML_CD,	PST_FIELD(pList, shct_arvl_trml_cd));
			nRet = TMaxFBGetF(TRML_SHCT_NM,			PST_FIELD(pList, trml_shct_nm));
			nRet = TMaxFBGetF(TRML_ENG_NM,			PST_FIELD(pList, trml_eng_nm));
			nRet = TMaxFBGetF(TRML_ZIP,				PST_FIELD(pList, trml_zip));
			nRet = TMaxFBGetF(TRML_SHP_CD,			PST_FIELD(pList, trml_shp_cd));
			nRet = TMaxFBGetF(TRML_KND_CD,			PST_FIELD(pList, trml_knd_cd));
			nRet = TMaxFBGetF(CTY_BUS_SYS_DVS_CD,	PST_FIELD(pList, cty_bus_sys_dvs_cd));
			nRet = TMaxFBGetF(TRML_RPRN_TEL_NO,		PST_FIELD(pList, trml_rprn_tel_no));
			nRet = TMaxFBGetF(TRML_ADDR,			PST_FIELD(pList, trml_addr));
			nRet = TMaxFBGetF(TRML_HMPG_URL_VAL,	PST_FIELD(pList, trml_hmpg_url_val));
			nRet = TMaxFBGetF(TRML_LTTD,			PST_FIELD(pList, trml_lttd));
			nRet = TMaxFBGetF(TRML_LNGT,			PST_FIELD(pList, trml_lngt));
			nRet = TMaxFBGetF(TRML_BRN,				PST_FIELD(pList, trml_brn));
			nRet = TMaxFBGetF(CTY_BUS_AREA_CD,		PST_FIELD(pList, cty_bus_area_cd));
			nRet = TMaxFBGetF(TRML_DRTN_CD,			PST_FIELD(pList, trml_drtn_cd));
			nRet = TMaxFBGetF(SND_PTR_USE_YN,		PST_FIELD(pList, snd_ptr_use_yn));

#if 0
			pBuff = GetTrmlCode(SVR_DVS_CCBUS);
			if( memcmp(pBuff, pList->trml_cd, sizeof(pList->trml_cd) - 1) == 0 )
			{	// Config Write
				//TERMINAL_INFO_T* pTrmlInfo;

				//pTrmlInfo = (TERMINAL_INFO_T*) GetConfigTrmlInfo();

				//sprintf(pTrmlInfo->szName, "%s", pList->trml_nm);///> 터미널 명칭
				//sprintf(pTrmlInfo->szCorpName, "%s", pList->trml_shct_nm);///> 터미널 상호
				//sprintf(pTrmlInfo->szCorpTel, "%s", pList->trml_rprn_tel_no);///> 터미널 전화번호
				//SetConfigTrmlInfo((char *)pTrmlInfo);

				//UI_AddTrmlInfo();
				sprintf(CConfigTkMem::GetInstance()->trml_nm, "%s", pList->trml_nm);			///> (한글){터미널 명칭
				sprintf(CConfigTkMem::GetInstance()->trml_eng_nm, "%s", pList->trml_eng_nm);	///> (영문)터미널 명칭
			}
			else
			{
				nRealCount++;
			}
#else
			nRealCount++;
#endif
		}

		// file write
		{
			OperGetFileName(OPER_FILE_ID_0120, strFullName);
			hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				nRet = -100;
				LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
				goto fail_proc;
			}

			MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));

			::ZeroMemory(pRPacket->trml_cd_num, sizeof(pRPacket->trml_cd_num));
//			::CopyMemory(pRPacket->trml_cd_num, &nRealCount, 4);
			::CopyMemory(pRPacket->trml_cd_num, &nCount, 4);
			MyWriteFile(hFile, pRPacket->trml_cd_num, sizeof(pRPacket->trml_cd_num));

			for(i = 0; i < nCount; i++)
			{
				prtk_readtrmlcd_list_t pList;

				pList = &pRPacket->pList[i];

				pBuff = GetTrmlCode(SVR_DVS_CCBUS);
				
#if 0
				if( memcmp(pBuff, pList->trml_cd, sizeof(pList->trml_cd) - 1) == 0 )
				{	// Config Write
					//TERMINAL_INFO_T* pTrmlInfo;

					//pTrmlInfo = (TERMINAL_INFO_T*) GetConfigTrmlInfo();

					//sprintf(pTrmlInfo->szName, "%s", pList->trml_nm);///> 터미널 명칭
					//sprintf(pTrmlInfo->szCorpName, "%s", pList->trml_shct_nm);///> 터미널 상호
					//sprintf(pTrmlInfo->szCorpTel, "%s", pList->trml_rprn_tel_no);///> 터미널 전화번호
					//SetConfigTrmlInfo((char *)pTrmlInfo);

					//UI_AddTrmlInfo();
				}
				else
				{
					MyWriteFile(hFile, pList, sizeof(rtk_readtrmlcd_list_t));
				}
#else
				MyWriteFile(hFile, pList, sizeof(rtk_readtrmlcd_list_t));
#endif
			}
			MyCloseFile(hFile);
		}
	}

	///> recv log
	{
		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TRML_CD",				PST_FIELD(pList, trml_cd));
			LOG_WRITE("%30s - (%s) ", "TRML_NM",				PST_FIELD(pList, trml_nm));
			LOG_WRITE("%30s - (%s) ", "SHCT_TRML_CD",			PST_FIELD(pList, shct_trml_cd));
			LOG_WRITE("%30s - (%s) ", "SHCT_ARVL_TRML_CD",	PST_FIELD(pList, shct_arvl_trml_cd));
			LOG_WRITE("%30s - (%s) ", "TRML_SHCT_NM",			PST_FIELD(pList, trml_shct_nm));
			LOG_WRITE("%30s - (%s) ", "TRML_ENG_NM",			PST_FIELD(pList, trml_eng_nm));
			LOG_WRITE("%30s - (%s) ", "TRML_ZIP",				PST_FIELD(pList, trml_zip));
			LOG_WRITE("%30s - (%s) ", "TRML_SHP_CD",			PST_FIELD(pList, trml_shp_cd));
			LOG_WRITE("%30s - (%s) ", "TRML_KND_CD",			PST_FIELD(pList, trml_knd_cd));
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_SYS_DVS_CD",	PST_FIELD(pList, cty_bus_sys_dvs_cd));
			LOG_WRITE("%30s - (%s) ", "TRML_RPRN_TEL_NO",		PST_FIELD(pList, trml_rprn_tel_no));
			LOG_WRITE("%30s - (%s) ", "TRML_ADDR",			PST_FIELD(pList, trml_addr));
			LOG_WRITE("%30s - (%s) ", "TRML_HMPG_URL_VAL",	PST_FIELD(pList, trml_hmpg_url_val));
			LOG_WRITE("%30s - (%s) ", "TRML_LTTD",			PST_FIELD(pList, trml_lttd));
			LOG_WRITE("%30s - (%s) ", "TRML_LNGT",			PST_FIELD(pList, trml_lngt));
			LOG_WRITE("%30s - (%s) ", "TRML_BRN",				PST_FIELD(pList, trml_brn));
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_AREA_CD",		PST_FIELD(pList, cty_bus_area_cd));
			LOG_WRITE("%30s - (%s) ", "TRML_DRTN_CD",			PST_FIELD(pList, trml_drtn_cd));
			LOG_WRITE("%30s - (%s) ", "SND_PTR_USE_YN",		PST_FIELD(pList, snd_ptr_use_yn));
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
 * @brief		TMaxSvc122
 * @details		(122) 터미널 설정 상세 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc122(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_TrmlStupDtl";
	pstk_trmlstupdtl_t	pSPacket;
	prtk_trmlstupdtl_t	pRPacket;
	prtk_trmlstupdtl_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_trmlstupdtl_t) pData;
	pRPacket = (prtk_trmlstupdtl_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
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

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(TRML_STUP_DTL_NUM,	PST_FIELD(pRPacket, trml_stup_dtl_num));
		::CopyMemory(&nCount, pRPacket->trml_stup_dtl_num, 4);
		LOG_WRITE("%30s - (%d) ","TRML_STUP_DTL_NUM",	nCount);
		if(nCount <= 0)
		{	// count error
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_trmlstupdtl_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_trmlstupdtl_list_t) * nCount);
		}

		OperGetFileName(OPER_FILE_ID_0122, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->trml_stup_dtl_num, sizeof(pRPacket->trml_stup_dtl_num));

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(TRML_STUP_DVS_CD,		PST_FIELD(pList, trml_stup_dvs_cd));
			nRet = TMaxFBGetF(TRML_STUP_DVS_VAL,	PST_FIELD(pList, trml_stup_dvs_val));

			MyWriteFile(hFile, pList, sizeof(rtk_trmlstupdtl_list_t));
		}
		MyCloseFile(hFile);
	}

	///> Log out
	{
		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%.*s) ", "TRML_STUP_DVS_CD",	sizeof(pList->trml_stup_dvs_cd),	PST_FIELD(pList, trml_stup_dvs_cd));
			LOG_WRITE("%30s - (%.*s) ", "TRML_STUP_DVS_VAL",	sizeof(pList->trml_stup_dvs_val),	PST_FIELD(pList, trml_stup_dvs_val));
		}
	}

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
 * @brief		TMaxSvc124
 * @details		(124) 노선 기본 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc124(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int nRet, i, nCount, nTimeout, nIndex;
	int n_rsp_cd;
	char szDate[20];
	SYSTEMTIME st;
	char*	pService = "TK_ReadRotBsc";
	pstk_readrotbsc_t	pSPacket;
	prtk_readrotbsc_t	pRPacket;
	prtk_readrotbsc_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readrotbsc_t) pData;
	pRPacket = (prtk_readrotbsc_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
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

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(ROT_BSC_NUM,	PST_FIELD(pRPacket, rot_bsc_num));
		::CopyMemory(&nCount, pRPacket->rot_bsc_num, 4);
		if(nCount <= 0)
		{	// count error
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_readrotbsc_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readrotbsc_list_t) * nCount);
		}

		OperGetFileName(OPER_FILE_ID_0124, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->rot_bsc_num, sizeof(pRPacket->rot_bsc_num));

		nIndex = 0;

		::GetLocalTime(&st);
		sprintf(szDate, "%04d%02d%02d", st.wYear, st.wMonth, st.wDay);

		for(i = 0; i < nCount; i++)
		{
			rtk_readrotbsc_list_t stList;
			int nCurrDT, nStartDT, nEndDT;

			::ZeroMemory(&stList, sizeof(rtk_readrotbsc_list_t));

			//pList = &pRPacket->pList[i];
			pList = &stList;

			nRet = TMaxFBGetF(ROT_ID			,	PST_FIELD(pList, rot_id			));
			nRet = TMaxFBGetF(STPT_TRML_CD		,	PST_FIELD(pList, stpt_trml_cd	));
			nRet = TMaxFBGetF(EPT_TRML_CD		,	PST_FIELD(pList, ept_trml_cd	));
			nRet = TMaxFBGetF(ROT_SNO			,	PST_FIELD(pList, rot_sno		));
			nRet = TMaxFBGetF(ROT_NM			,	PST_FIELD(pList, rot_nm			));
			nRet = TMaxFBGetF(ROT_KND_CD		,	PST_FIELD(pList, rot_knd_cd		));
			nRet = TMaxFBGetF(ALCN_WAY_DVS_CD	,	PST_FIELD(pList, alcn_way_dvs_cd));
			nRet = TMaxFBGetF(SATI_USE_YN		,	PST_FIELD(pList, sati_use_yn	));
			nRet = TMaxFBGetF(EB_LNKG_YN		,	PST_FIELD(pList, eb_lnkg_yn		));
			nRet = TMaxFBGetF(HMPG_EXPS_YN		,	PST_FIELD(pList, hmpg_exps_yn	));
			nRet = TMaxFBGetF(ADPT_STT_DT		,	PST_FIELD(pList, adpt_stt_dt	));
			nRet = TMaxFBGetF(ADPT_END_DT		,	PST_FIELD(pList, adpt_end_dt	));

			nCurrDT  = Util_Ascii2Long(szDate, strlen(szDate));
			nStartDT = Util_Ascii2Long(pList->adpt_stt_dt, strlen(pList->adpt_stt_dt));
			nEndDT   = Util_Ascii2Long(pList->adpt_end_dt, strlen(pList->adpt_end_dt));

			//시작일전, 종료일 후에는 저장안함.
			if( (nCurrDT < nStartDT) || (nCurrDT > nEndDT) )
			{
				continue;
			}

			if( memcmp(GetShortTrmlCode(SVR_DVS_CCBUS), "0716", 4) == 0 )
			{
				if( memcmp(pList->rot_id, "RT201605118067404", sizeof(pList->rot_id) - 1) == 0 )
				{
					continue;
				}
			}

			::CopyMemory(&pRPacket->pList[nIndex], pList, sizeof(rtk_readrotbsc_list_t));
			MyWriteFile(hFile, pList, sizeof(rtk_readrotbsc_list_t));
			nIndex++;
		}

		::ZeroMemory(pRPacket->rot_bsc_num, sizeof(pRPacket->rot_bsc_num));
		::CopyMemory(pRPacket->rot_bsc_num, &nIndex, 4);

		MySetFilePointer(hFile, sizeof(pRPacket->rsp_cd), FILE_BEGIN);
		MyWriteFile(hFile, pRPacket->rot_bsc_num, sizeof(pRPacket->rot_bsc_num));

		MyCloseFile(hFile);
	}


	///> recv log
	{
		LOG_WRITE("%30s - (%d) ","ROT_BSC_NUM", nIndex);
		for(i = 0; i < nIndex; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nIndex);
			LOG_WRITE("%30s - (%s) ", "ROT_ID",			PST_FIELD(pList, rot_id			));
			LOG_WRITE("%30s - (%s) ", "STPT_TRML_CD",		PST_FIELD(pList, stpt_trml_cd	));
			LOG_WRITE("%30s - (%s) ", "EPT_TRML_CD",		PST_FIELD(pList, ept_trml_cd	));
			LOG_WRITE("%30s - (%s) ", "ROT_SNO",			PST_FIELD(pList, rot_sno		));
			LOG_WRITE("%30s - (%s) ", "ROT_NM",			PST_FIELD(pList, rot_nm			));
			LOG_WRITE("%30s - (%s) ", "ROT_KND_CD",		PST_FIELD(pList, rot_knd_cd		));
			LOG_WRITE("%30s - (%s) ", "ALCN_WAY_DVS_CD",	PST_FIELD(pList, alcn_way_dvs_cd));
			LOG_WRITE("%30s - (%s) ", "SATI_USE_YN",		PST_FIELD(pList, sati_use_yn	));
			LOG_WRITE("%30s - (%s) ", "EB_LNKG_YN",		PST_FIELD(pList, eb_lnkg_yn		));
			LOG_WRITE("%30s - (%s) ", "HMPG_EXPS_YN",		PST_FIELD(pList, hmpg_exps_yn	));
			LOG_WRITE("%30s - (%s) ", "ADPT_STT_DT",		PST_FIELD(pList, adpt_stt_dt	));
			LOG_WRITE("%30s - (%s) ", "ADPT_END_DT",		PST_FIELD(pList, adpt_end_dt	));
		}
	}

	///> data proc
	{
		nRet = nIndex;
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
 * @brief		TMaxSvc125
 * @details		(125) 노선 상세 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc125(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;//, nIndex;
	int n_rsp_cd;
	char*	pService = "TK_ReadRotDtl";
	pstk_readrotdtl_t	pSPacket;
	prtk_readrotdtl_t	pRPacket;
	prtk_readrotdtl_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readrotdtl_t) pData;
	pRPacket = (prtk_readrotdtl_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
		
		nRet = TMaxFBGetF(ROT_DTL_NUM,	PST_FIELD(pRPacket, rot_dtl_num));
		::CopyMemory(&nCount, pRPacket->rot_dtl_num, 4);
		if(nCount <= 0)
		{	// count error
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_readrotdtl_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readrotdtl_list_t) * nCount);
		}

		OperGetFileName(OPER_FILE_ID_0125, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->rot_dtl_num, sizeof(pRPacket->rot_dtl_num));

		LOG_WRITE("Count(%d), sizeof() = (%d), Total(%d)..", nCount, sizeof(rtk_readrotdtl_list_t), sizeof(rtk_readrotdtl_list_t) * nCount);

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(ROT_ID					,	PST_FIELD(pList, rot_id				));
			nRet = TMaxFBGetF(ROT_SQNO					,	PST_FIELD(pList, rot_sqno			));
			nRet = TMaxFBGetF(ALCN_WAY_DVS_CD			,	PST_FIELD(pList, alcn_way_dvs_cd	));
			nRet = TMaxFBGetF(SATI_USE_YN				,	PST_FIELD(pList, sati_use_yn		));
			nRet = TMaxFBGetF(TRML_TISU_PSB_YN			,	PST_FIELD(pList, trml_tisu_psb_yn	));
			nRet = TMaxFBGetF(HMPG_TISU_PSB_YN			,	PST_FIELD(pList, hmpg_tisu_psb_yn	));
			nRet = TMaxFBGetF(MBL_TISU_PSB_YN			,	PST_FIELD(pList, mbl_tisu_psb_yn	));
			nRet = TMaxFBGetF(TRML_CD					,	PST_FIELD(pList, trml_cd			));

			nRet = TMaxFBGetF(BEF_TRMI_DIST				,	PST_FIELD(pList, bef_trmi_dist		));
			//pList->n_bef_trmi_dist = *(int *) pList->bef_trmi_dist;

			nRet = TMaxFBGetF(BEF_TRMI_TAKE_DRTM		,	PST_FIELD(pList, bef_trmi_take_drtm	));
			//pList->n_bef_trmi_take_drtm = *(int *) pList->bef_trmi_take_drtm;

			nRet = TMaxFBGetF(TRML_CTAS_DRTM			,	PST_FIELD(pList, trml_ctas_drtm		));
			//pList->n_trml_ctas_drtm = *(int *) pList->trml_ctas_drtm;
			
			nRet = TMaxFBGetF(FRBS_TIME					,	PST_FIELD(pList, frbs_time			));
			nRet = TMaxFBGetF(LSBS_TIME					,	PST_FIELD(pList, lsbs_time			));
			nRet = TMaxFBGetF(BUS_CLS_PRIN_YN			,	PST_FIELD(pList, bus_cls_prin_yn	));
			nRet = TMaxFBGetF(BUS_CACM_NM_PRIN_YN		,	PST_FIELD(pList, bus_cacm_nm_prin_yn));
			nRet = TMaxFBGetF(DEPR_TIME_PRIN_YN			,	PST_FIELD(pList, depr_time_prin_yn	));
			nRet = TMaxFBGetF(SATS_NO_PRIN_YN			,	PST_FIELD(pList, sats_no_prin_yn	));
			nRet = TMaxFBGetF(RDHM_MLTP_VAL				,	PST_FIELD(pList, rdhm_mltp_val		));

			MyWriteFile(hFile, pList, sizeof(rtk_readrotdtl_list_t));
		}
		MyCloseFile(hFile);
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%d) ","ROT_DTL_NUM", nCount);
		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ","ROT_ID",					PST_FIELD(pList, rot_id				));
			LOG_WRITE("%30s - (%d) ","ROT_SQNO",				*(int *)pList->rot_sqno				);
			LOG_WRITE("%30s - (%s) ","ALCN_WAY_DVS_CD",			PST_FIELD(pList, alcn_way_dvs_cd	));
			LOG_WRITE("%30s - (%s) ","SATI_USE_YN",				PST_FIELD(pList, sati_use_yn		));
			LOG_WRITE("%30s - (%s) ","TRML_TISU_PSB_YN",		PST_FIELD(pList, trml_tisu_psb_yn	));
			LOG_WRITE("%30s - (%s) ","HMPG_TISU_PSB_YN",		PST_FIELD(pList, hmpg_tisu_psb_yn	));
			LOG_WRITE("%30s - (%s) ","MBL_TISU_PSB_YN",			PST_FIELD(pList, mbl_tisu_psb_yn	));
			LOG_WRITE("%30s - (%s) ","TRML_CD",					PST_FIELD(pList, trml_cd			));
			LOG_WRITE("%30s - (number = %d) ","BEF_TRMI_DIST", *(int *)pList->bef_trmi_dist);
			LOG_WRITE("%30s - (number = %d) ","BEF_TRMI_TAKE_DRTM",  *(int *)pList->bef_trmi_take_drtm);
			LOG_WRITE("%30s - (number = %d) ","TRML_CTAS_DRTM", *(int *)pList->trml_ctas_drtm);
			LOG_WRITE("%30s - (%s) ","FRBS_TIME",				PST_FIELD(pList, frbs_time			));
			LOG_WRITE("%30s - (%s) ","LSBS_TIME",				PST_FIELD(pList, lsbs_time			));
			LOG_WRITE("%30s - (%s) ","BUS_CLS_PRIN_YN",		PST_FIELD(pList, bus_cls_prin_yn	));
			LOG_WRITE("%30s - (%s) ","BUS_CACM_NM_PRIN_YN",	PST_FIELD(pList, bus_cacm_nm_prin_yn));
			LOG_WRITE("%30s - (%s) ","DEPR_TIME_PRIN_YN",		PST_FIELD(pList, depr_time_prin_yn	));
			LOG_WRITE("%30s - (%s) ","SATS_NO_PRIN_YN",		PST_FIELD(pList, sats_no_prin_yn	));
			LOG_WRITE("%30s - (%s) ","RDHM_MLTP_VAL",			PST_FIELD(pList, rdhm_mltp_val		));
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
 * @brief		TMaxSvc126
 * @details		(126) 노선 요금 상세 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc126(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;//, nIndex;
	int n_rsp_cd;
	char*	pService = "TK_RotFeeDtl";
	pstk_rotfeedtl_t		pSPacket;
	prtk_rotfeedtl_t		pRPacket;
	prtk_rotfeedtl_list_t	pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_rotfeedtl_t) pData;
	pRPacket = (prtk_rotfeedtl_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
		
		nRet = TMaxFBGetF(ROT_FEE_DTL_NUM,	PST_FIELD(pRPacket, rot_fee_dtl_num));
		::CopyMemory(&nCount, pRPacket->rot_fee_dtl_num, 4);
		if(nCount <= 0)
		{	// count error
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_rotfeedtl_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_rotfeedtl_list_t) * nCount);
		}

		/// 노선요금 상세조회
		OperGetFileName(OPER_FILE_ID_0126, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->rot_fee_dtl_num, sizeof(pRPacket->rot_fee_dtl_num));

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(ROT_ID			,	PST_FIELD(pList, rot_id			));
			nRet = TMaxFBGetF(ROT_FEE_SNO		,	PST_FIELD(pList, rot_fee_sno	));
			nRet = TMaxFBGetF(DEPR_TRML_CD		,	PST_FIELD(pList, depr_trml_cd	));
			nRet = TMaxFBGetF(ARVL_TRML_CD		,	PST_FIELD(pList, arvl_trml_cd	));
			nRet = TMaxFBGetF(BUS_CLS_CD		,	PST_FIELD(pList, bus_cls_cd		));
			nRet = TMaxFBGetF(BUS_TCK_KND_CD	,	PST_FIELD(pList, bus_tck_knd_cd	));
			nRet = TMaxFBGetF(ADPT_STT_DT		,	PST_FIELD(pList, adpt_stt_dt	));
			nRet = TMaxFBGetF(ADPT_END_DT		,	PST_FIELD(pList, adpt_end_dt	));
			nRet = TMaxFBGetF(RIDE_FEE			,	PST_FIELD(pList, ride_fee		));

			MyWriteFile(hFile, pList, sizeof(rtk_rotfeedtl_list_t));
		}
		MyCloseFile(hFile);
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%d) ","ROT_DTL_NUM", nCount);
		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ","ROT_ID"			,	PST_FIELD(pList, rot_id				));
			LOG_WRITE("%30s - (num=%d) ","ROT_FEE_SNO"	,	*(int *)pList->rot_fee_sno			);
			LOG_WRITE("%30s - (%s) ","DEPR_TRML_CD"		,	PST_FIELD(pList, depr_trml_cd		));
			LOG_WRITE("%30s - (%s) ","ARVL_TRML_CD"		,	PST_FIELD(pList, arvl_trml_cd		));
			LOG_WRITE("%30s - (%s) ","BUS_CLS_CD"		,	PST_FIELD(pList, bus_cls_cd			));
			LOG_WRITE("%30s - (%s) ","BUS_TCK_KND_CD"	,	PST_FIELD(pList, bus_tck_knd_cd		));
			LOG_WRITE("%30s - (%s) ","ADPT_STT_DT"		,	PST_FIELD(pList, adpt_stt_dt		));
			LOG_WRITE("%30s - (%s) ","ADPT_END_DT"		,	PST_FIELD(pList, adpt_end_dt		));
			LOG_WRITE("%30s - (num=%d) ","RIDE_FEE"		,	*(int *)pList->ride_fee				);
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
 * @brief		TMaxSvc127
 * @details		(127) 버스티켓고유번호 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc127(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ReadTckNo";
	pstk_readtckno_t	pSPacket;
	prtk_readtckno_t	pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readtckno_t) pData;
	pRPacket = (prtk_readtckno_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
	}

	// 동기화 (Send/Recv)
	nTimeout = 10;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		int nValue;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(BUS_TCK_INHR_NO,		PST_FIELD(pRPacket, bus_tck_inhr_no));
		nRet = TMaxFBGetF(ADD_BUS_TCK_INHR_NO,	PST_FIELD(pRPacket, add_bus_tck_inhr_no));

		::CopyMemory(CConfigTkMem::GetInstance()->bus_tck_inhr_no, pRPacket->bus_tck_inhr_no, sizeof(pRPacket->bus_tck_inhr_no));
		::CopyMemory(CConfigTkMem::GetInstance()->add_bus_tck_inhr_no, pRPacket->add_bus_tck_inhr_no, sizeof(pRPacket->add_bus_tck_inhr_no));

		nValue = (WORD) Util_Ascii2Long(pRPacket->bus_tck_inhr_no,  strlen(pRPacket->bus_tck_inhr_no));
		AddAccumBusTckInhrNo(ACC_TICKET_SEQ_ASSIGN, SVR_DVS_CCBUS, nValue);
		
		nValue = (WORD) Util_Ascii2Long(pRPacket->add_bus_tck_inhr_no,  strlen(pRPacket->add_bus_tck_inhr_no));
		AddAccumBusTckInhrNo(ACC_TICKET_ADD_SEQ_ASSIGN, SVR_DVS_CCBUS, nValue);
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ","BUS_TCK_INHR_NO",		PST_FIELD(pRPacket, bus_tck_inhr_no		));
		LOG_WRITE("%30s - (%s) ","ADD_BUS_TCK_INHR_NO",	PST_FIELD(pRPacket, add_bus_tck_inhr_no	));
	}

	nRet = 1;

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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc128
 * @details		(128) 로그인
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc128(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_Login";
	pstk_login_t	pSPacket;
	prtk_login_t	pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_login_t) pData;
	pRPacket = (prtk_login_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);

		LOG_WRITE("%30s - (%s) ","USER_NO",			  PST_FIELD(pSPacket, user_no));
		LOG_WRITE("%30s - (%s) ","REQ_TRML_USER_PWD",	  PST_FIELD(pSPacket, req_trml_user_pwd));
		LOG_WRITE("%30s - (%s) ","BUS_TCK_INHR_NO",	  PST_FIELD(pSPacket, bus_tck_inhr_no));
		LOG_WRITE("%30s - (%s) ","ADD_BUS_TCK_INHR_NO", PST_FIELD(pSPacket, add_bus_tck_inhr_no));
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);

		nRet = TMaxFBPut(USER_NO,				PST_FIELD(pSPacket, user_no));
		nRet = TMaxFBPut(REQ_TRML_USER_PWD,		PST_FIELD(pSPacket, req_trml_user_pwd));
		nRet = TMaxFBPut(BUS_TCK_INHR_NO,		PST_FIELD(pSPacket, bus_tck_inhr_no));
		nRet = TMaxFBPut(ADD_BUS_TCK_INHR_NO,	PST_FIELD(pSPacket, add_bus_tck_inhr_no));
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

	///> recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd == 626 )
		{
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		if( n_rsp_cd != 141 )
		{
			if(n_rsp_cd == 10)
			{
				; // "창구로그인SVC: 비밀번호가 틀렸습니다"
			}
			else
			{
				; // 
			}
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(RSP_MSG				,				PST_FIELD(pRPacket, rsp_msg				));
		nRet = TMaxFBGetF(USER_ID				,				PST_FIELD(pRPacket, user_id				));
		nRet = TMaxFBGetF(CASH_TISU_NUM			,				PST_FIELD(pRPacket, cash_tisu_num		));
		nRet = TMaxFBGetF(CASH_TISU_AMT			,				PST_FIELD(pRPacket, cash_tisu_amt		));
		nRet = TMaxFBGetF(CASH_CANC_RY_NUM		,				PST_FIELD(pRPacket, cash_canc_ry_num	));
		nRet = TMaxFBGetF(CASH_CANC_RY_AMT		,				PST_FIELD(pRPacket, cash_canc_ry_amt	));
		nRet = TMaxFBGetF(CARD_TISU_NUM			,				PST_FIELD(pRPacket, card_tisu_num		));
		nRet = TMaxFBGetF(CARD_TISU_AMT			,				PST_FIELD(pRPacket, card_tisu_amt		));
		nRet = TMaxFBGetF(CARD_CANC_RY_NUM		,				PST_FIELD(pRPacket, card_canc_ry_num	));
		nRet = TMaxFBGetF(CARD_CANC_RY_AMT		,				PST_FIELD(pRPacket, card_canc_ry_amt	));
		nRet = TMaxFBGetF(ETC_TISU_NUM			,				PST_FIELD(pRPacket, etc_tisu_num		));
		nRet = TMaxFBGetF(ETC_TISU_AMT			,				PST_FIELD(pRPacket, etc_tisu_amt		));
		nRet = TMaxFBGetF(ETC_CANC_RY_NUM		,				PST_FIELD(pRPacket, etc_canc_ry_num		));
		nRet = TMaxFBGetF(ETC_CANC_RY_AMT		,				PST_FIELD(pRPacket, etc_canc_ry_amt		));
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ",		   "USER_ID"			, PST_FIELD(pRPacket, user_id	));
		LOG_WRITE("%30s - (number = %d) ", "CASH_TISU_NUM"		, *((int *)pRPacket->cash_tisu_num		));
		LOG_WRITE("%30s - (number = %d) ", "CASH_TISU_AMT"		, *((int *)pRPacket->cash_tisu_amt		));
		LOG_WRITE("%30s - (number = %d) ", "CASH_CANC_RY_NUM"	, *((int *)pRPacket->cash_canc_ry_num	));
		LOG_WRITE("%30s - (number = %d) ", "CASH_CANC_RY_AMT"	, *((int *)pRPacket->cash_canc_ry_amt	));
		LOG_WRITE("%30s - (number = %d) ", "CARD_TISU_NUM"		, *((int *)pRPacket->card_tisu_num		));
		LOG_WRITE("%30s - (number = %d) ", "CARD_TISU_AMT"		, *((int *)pRPacket->card_tisu_amt		));
		LOG_WRITE("%30s - (number = %d) ", "CARD_CANC_RY_NUM"	, *((int *)pRPacket->card_canc_ry_num	));
		LOG_WRITE("%30s - (number = %d) ", "CARD_CANC_RY_AMT"	, *((int *)pRPacket->card_canc_ry_amt	));
		LOG_WRITE("%30s - (number = %d) ", "ETC_TISU_NUM"		, *((int *)pRPacket->etc_tisu_num		));
		LOG_WRITE("%30s - (number = %d) ", "ETC_TISU_AMT"		, *((int *)pRPacket->etc_tisu_amt		));
		LOG_WRITE("%30s - (number = %d) ", "ETC_CANC_RY_NUM"	, *((int *)pRPacket->etc_canc_ry_num		));
		LOG_WRITE("%30s - (number = %d) ", "ETC_CANC_RY_AMT"	, *((int *)pRPacket->etc_canc_ry_amt		));
	}


	{
		CConfigTkMem* pConfigMem;

		pConfigMem = CConfigTkMem::GetInstance();

		::CopyMemory(pConfigMem->req_trml_user_id, pRPacket->user_id, sizeof(pRPacket->user_id) - 1);

		::CopyMemory(&pConfigMem->n_cash_tisu_num, pRPacket->cash_tisu_num, sizeof(int));
		::CopyMemory(&pConfigMem->n_cash_tisu_amt, pRPacket->cash_tisu_amt, sizeof(int));
		
		::CopyMemory(&pConfigMem->n_card_tisu_num, pRPacket->card_tisu_num, sizeof(int));
		::CopyMemory(&pConfigMem->n_card_tisu_amt, pRPacket->card_tisu_amt, sizeof(int));

// 		Transact_SetData((DWORD)USER_ID,		pRPacket->user_id,		 (int)sizeof(pRPacket->user_id));
// 		Transact_SetData((DWORD)CASH_TISU_AMT,	pRPacket->cash_tisu_amt, (int)sizeof(pRPacket->cash_tisu_amt));
// 		Transact_SetData((DWORD)CARD_TISU_NUM,	pRPacket->card_tisu_num, (int)sizeof(pRPacket->card_tisu_num));
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc129
 * @details		(129) 알림 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc129(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout, nIndex;
	int n_rsp_cd;
	char*	pService = "TK_ReadNtfc";
	pstk_readntfc_t	pSPacket;
	prtk_readntfc_t	pRPacket;
	prtk_readntfc_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readntfc_t) pData;
	pRPacket = (prtk_readntfc_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
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

	///> recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(NTFC_NUM,	PST_FIELD(pRPacket, ntfc_num));
		nCount = *(int *) pRPacket->ntfc_num;
		if( nCount <= 0 )
		{	// count 에러
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}
		
		{
			pRPacket->pList = new rtk_readntfc_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readntfc_list_t) * nCount);
		}

		nIndex = 0;
		for(i = 0; i < nCount; i++)
		{
			rtk_readntfc_list_t stList;

			::ZeroMemory(&stList, sizeof(rtk_readntfc_list_t));

			pList = &pRPacket->pList[i];
			pList = &stList;

			nRet = TMaxFBGetF(NTFC_ID				,	PST_FIELD(pList, ntfc_id				));
			nRet = TMaxFBGetF(NTFC_NM				,	PST_FIELD(pList, ntfc_nm				));
			nRet = TMaxFBGetF(CTY_BUS_NTFC_DVS_CD	,	PST_FIELD(pList, cty_bus_ntfc_dvs_cd	));
			nRet = TMaxFBGetF(NTFC_CTT				,	PST_FIELD(pList, ntfc_ctt				));
			nRet = TMaxFBGetF(NTFC_STT_DT			,	PST_FIELD(pList, ntfc_stt_dt			));
			nRet = TMaxFBGetF(NTFC_STT_TIME			,	PST_FIELD(pList, ntfc_stt_time			));
			nRet = TMaxFBGetF(NTFC_END_DT			,	PST_FIELD(pList, ntfc_end_dt			));
			nRet = TMaxFBGetF(NTFC_END_TIME			,	PST_FIELD(pList, ntfc_end_time			));
			nRet = TMaxFBGetF(NTFC_USER_ID			,	PST_FIELD(pList, ntfc_user_id			));
			nRet = TMaxFBGetF(ALL_NTC_YN			,	PST_FIELD(pList, all_ntc_yn				));

			if( memcmp(pList->cty_bus_ntfc_dvs_cd, "A03", 3) == 0 )
			{
				::CopyMemory(&pRPacket->pList[nIndex++], &stList, sizeof(rtk_readntfc_list_t));
			}
		}
	}

	::ZeroMemory(pRPacket->ntfc_num, sizeof(pRPacket->ntfc_num));
	::CopyMemory(pRPacket->ntfc_num, &nIndex, 2);

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%d) ","NTFC_NUM",	nIndex);
		for(i = 0; i < nIndex; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nIndex);
			LOG_WRITE("%30s - (%.*s) ","NTFC_ID",				sizeof(pList->ntfc_id			),		PST_FIELD(pList, ntfc_id			));
			LOG_WRITE("%30s - (%.*s) ","NTFC_NM",				sizeof(pList->ntfc_nm			),		PST_FIELD(pList, ntfc_nm			));
			LOG_WRITE("%30s - (%.*s) ","CTY_BUS_NTFC_DVS_CD", sizeof(pList->cty_bus_ntfc_dvs_cd),		PST_FIELD(pList, cty_bus_ntfc_dvs_cd));
			LOG_WRITE("%30s - (%.*s) ","NTFC_CTT",			sizeof(pList->ntfc_ctt			),		PST_FIELD(pList, ntfc_ctt			));
			LOG_WRITE("%30s - (%.*s) ","NTFC_STT_DT",			sizeof(pList->ntfc_stt_dt		),		PST_FIELD(pList, ntfc_stt_dt		));
			LOG_WRITE("%30s - (%.*s) ","NTFC_STT_TIME",		sizeof(pList->ntfc_stt_time		),		PST_FIELD(pList, ntfc_stt_time		));
			LOG_WRITE("%30s - (%.*s) ","NTFC_END_DT",			sizeof(pList->ntfc_end_dt		),		PST_FIELD(pList, ntfc_end_dt		));
			LOG_WRITE("%30s - (%.*s) ","NTFC_END_TIME",		sizeof(pList->ntfc_end_time		),		PST_FIELD(pList, ntfc_end_time		));
			LOG_WRITE("%30s - (%.*s) ","NTFC_USER_ID",		sizeof(pList->ntfc_user_id		),		PST_FIELD(pList, ntfc_user_id		));
			LOG_WRITE("%30s - (%.*s) ","ALL_NTC_YN",			sizeof(pList->all_ntc_yn		),		PST_FIELD(pList, all_ntc_yn			));
		}
	}

	///> data proc
	{
		nRet = nIndex;
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
 * @brief		TMaxSvc130
 * @details		(130) 배차 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc130(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd, nIndex;
	int n_rot_sqno;
	int n_alcn_sqno;
	int n_sats_num;
	int n_rmn_scnt;
	int n_ocnt;
	char*	pService = "TK_ReadAlcn";
	pstk_readalcn_t	pSPacket;
	prtk_readalcn_t	pRPacket;
	prtk_readalcn_list_t pList;
	PUI_BASE_T	pBaseInfo;
	char *pBuff;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readalcn_t) pData;
	pRPacket = (prtk_readalcn_t) retBuf;

	pBaseInfo = (PUI_BASE_T) GetConfigBaseInfo();

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ", "DEPR_TRML_CD",	PST_FIELD(pSPacket, depr_trml_cd	));
		LOG_WRITE("%30s - (%s) ", "ARVL_TRML_CD",	PST_FIELD(pSPacket, arvl_trml_cd	));
		LOG_WRITE("%30s - (%s) ", "READ_DT",		PST_FIELD(pSPacket, read_dt			));
		LOG_WRITE("%30s - (%s) ", "READ_TIME",	PST_FIELD(pSPacket, read_time		));
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD",	PST_FIELD(pSPacket, bus_cls_cd		));
		LOG_WRITE("%30s - (%s) ", "BEF_AFT_DVS",	PST_FIELD(pSPacket, bef_aft_dvs		));
		LOG_WRITE("%30s - (number = %d) ", "REQ_REC_NUM",	*(int *)pSPacket->req_rec_num);
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(DEPR_TRML_CD		,	PST_FIELD(pSPacket, depr_trml_cd		));
		nRet = TMaxFBPut(ARVL_TRML_CD		,	PST_FIELD(pSPacket, arvl_trml_cd		));
		nRet = TMaxFBPut(READ_DT			,	PST_FIELD(pSPacket, read_dt				));
		nRet = TMaxFBPut(READ_TIME			,	PST_FIELD(pSPacket, read_time			));
		nRet = TMaxFBPut(BUS_CLS_CD			,	PST_FIELD(pSPacket, bus_cls_cd			));
		nRet = TMaxFBPut(BEF_AFT_DVS		,	PST_FIELD(pSPacket, bef_aft_dvs			));
		nRet = TMaxFBPut(REQ_REC_NUM		,	PST_FIELD(pSPacket, req_rec_num			));
	}

	// 동기화 (Send/Recv)
	nTimeout = 10;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -98;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}		

		nRet = TMaxFBGetF(ALCN_NUM,	PST_FIELD(pRPacket, alcn_num));
		::CopyMemory(&nCount, pRPacket->alcn_num, 4);
		LOG_WRITE("%30s - Count (%d) ", "ALCN_NUM",	nCount);
		if(nCount <= 0)
		{
			nRet = -99;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE777");
			goto fail_proc;
		}

		nIndex = 0;

		//if(nCount > 0)
		{
			pRPacket->pList = new rtk_readalcn_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readalcn_list_t) * nCount);
		}

		CPubTckMem::GetInstance()->m_vtAlcnInfo.clear();
		for(i = 0; i < nCount; i++)
		{
			rtk_readalcn_list_t stList;
			SYSTEMTIME stCurrDT;
			char szCurrDT[20];

			::ZeroMemory(&stList, sizeof(rtk_readalcn_list_t));
			GetLocalTime(&stCurrDT);
			sprintf(szCurrDT, "%04d%02d%02d", stCurrDT.wYear, stCurrDT.wMonth, stCurrDT.wDay);

			pList = &pRPacket->pList[i];
			pList = &stList;

			nRet = TMaxFBGetF(ROT_ID				,	PST_FIELD(pList, rot_id					));
			nRet = TMaxFBGetF(ROT_SQNO				,	PST_FIELD(pList, rot_sqno				));
			nRet = TMaxFBGetF(ALCN_DT				,	PST_FIELD(pList, alcn_dt				));
			nRet = TMaxFBGetF(ALCN_SQNO				,	PST_FIELD(pList, alcn_sqno				));
			nRet = TMaxFBGetF(ATL_DEPR_DT			,	PST_FIELD(pList, atl_depr_dt			));
			nRet = TMaxFBGetF(ATL_DEPR_TIME			,	PST_FIELD(pList, atl_depr_time			));
			nRet = TMaxFBGetF(DEPR_TIME				,	PST_FIELD(pList, depr_time				));
			nRet = TMaxFBGetF(ROT_NM				,	PST_FIELD(pList, rot_nm					));
			nRet = TMaxFBGetF(ROT_BUS_DVS_CD		,	PST_FIELD(pList, rot_bus_dvs_cd			));
			nRet = TMaxFBGetF(BUS_CLS_CD			,	PST_FIELD(pList, bus_cls_cd				));
			nRet = TMaxFBGetF(BUS_CACM_CD			,	PST_FIELD(pList, bus_cacm_cd			));
			nRet = TMaxFBGetF(ALCN_WAY_DVS_CD		,	PST_FIELD(pList, alcn_way_dvs_cd		));
			nRet = TMaxFBGetF(SATI_USE_YN			,	PST_FIELD(pList, sati_use_yn			));
			nRet = TMaxFBGetF(PERD_TEMP_YN			,	PST_FIELD(pList, perd_temp_yn			));
			nRet = TMaxFBGetF(BCNL_YN				,	PST_FIELD(pList, bcnl_yn				));
			nRet = TMaxFBGetF(DIST					,	PST_FIELD(pList, dist					));
			nRet = TMaxFBGetF(TAKE_DRTM				,	PST_FIELD(pList, take_drtm				));
			nRet = TMaxFBGetF(RDHM_MLTP_VAL			,	PST_FIELD(pList, rdhm_mltp_val			));
			nRet = TMaxFBGetF(SATS_NUM				,	PST_FIELD(pList, sats_num				));
			nRet = TMaxFBGetF(RMN_SCNT				,	PST_FIELD(pList, rmn_scnt				));
			nRet = TMaxFBGetF(OCNT					,	PST_FIELD(pList, ocnt					));
			nRet = TMaxFBGetF(TRML_TISU_PSB_YN		,	PST_FIELD(pList, trml_tisu_psb_yn		));
			nRet = TMaxFBGetF(CTY_BUS_OPRN_SHP_CD	,	PST_FIELD(pList, cty_bus_oprn_shp_cd	));
			nRet = TMaxFBGetF(DC_PSB_YN				,	PST_FIELD(pList, dc_psb_yn				));
			nRet = TMaxFBGetF(WHCH_TISSU_YN			,	PST_FIELD(pList, whch_tissu_yn			)); // 20211206 ADD

			n_rot_sqno = *(int *)pList->rot_sqno;
			n_alcn_sqno = *(int *)pList->alcn_sqno;
			n_sats_num = *(int *)pList->sats_num;
			n_rmn_scnt = *(int *)pList->rmn_scnt;
			LOG_WRITE("pList->rmn_scnt = [%02X][%02X][%02X][%02X][%02X][%02X]", pList->rmn_scnt[0], pList->rmn_scnt[1], pList->rmn_scnt[2], pList->rmn_scnt[3], pList->rmn_scnt[4], pList->rmn_scnt[5]);
			n_ocnt = *(int *)pList->ocnt;

			pList->rot_id;
			pList->alcn_dt;
			pList->alcn_way_dvs_cd;
			pList->bcnl_yn[0];
			n_rmn_scnt;

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "ROT_ID",				PST_FIELD(pList, rot_id	));
			LOG_WRITE("%30s - (%s) ", "ALCN_DT",				PST_FIELD(pList, alcn_dt	));
			LOG_WRITE("%30s - (%s) ", "ALCN_WAY_DVS_CD",		PST_FIELD(pList, alcn_way_dvs_cd	));
			LOG_WRITE("%30s - (number = %d) ", "alcn_kind",		((pBaseInfo->alcn_kind == 'N') ? 0 : 1) );
			LOG_WRITE("%30s - (%s) ", "BCNL_YN",				PST_FIELD(pList, bcnl_yn	));
			LOG_WRITE("%30s - (number = %d) ", "BCNL_YN",	((pBaseInfo->bcnl_yn == 'Y') ? 1 : 0));
			LOG_WRITE("%30s - (number = %d) ", "RMN_SCNT",	n_rmn_scnt);
			LOG_WRITE("%30s - (number = %d) ", "TCK_NO_RMN_YN", ((pBaseInfo->tck_no_rmn_yn == 'Y') ? 1 : 0));


			// 160517. (김동진)함안터미널의 의령발 마산2행노선(RT201605118067404) 제외
			if( memcmp("RT201605118067404", pList->rot_id, sizeof(pList->rot_id) - 1) == 0 )
			{
				pBuff = GetShortTrmlCode(SVR_DVS_CCBUS);
				if(memcmp(pBuff, "0716", 4) == 0)
				{
					continue;
				}
			}

			// 현재 날짜와 배차 날짜 비교
			//전날 24시 넘는 배차들은 확인해서 24 빼준다. 23일 25시 배차 -> 24일 조회시 나오므로
			//24일 조회시 23일 25시 배차는 depr인 25, atl_depr이 01 임. 계산로직 짠 후에 추가된거라(Atl) 그냥 둠.
			//160908 김영준 전날배차의 선택날짜 표시 오류 수정.
			if( memcmp(pList->alcn_dt, szCurrDT, 8) < 0)
			{
				int value;

				value = Util_Ascii2Long(pList->depr_time, strlen(pList->depr_time)) - 240000L;

				sprintf(pList->depr_time, "%06d", value);
			}

#if 1
			// 배차(1)/비배차(0)
			if( pBaseInfo->alcn_kind == 'N' )
			{	/// 비배차 모드 일 경우
				if( pList->alcn_way_dvs_cd[0] != 'N' )
				{	// 비배차가 아닐 경우
					continue;
				}
			}

			// 결행배차 표시유부 옵션 설정 N인데 결행Y면 continue
			if(pBaseInfo->bcnl_yn == 'N')
			{	/// 미표시
				if( pList->bcnl_yn[0] == 'Y' )
				{
					continue;
				}
			}

			// 매진배차 표시유부. 옵션설정N && 잔여좌석 0 이면 continue
			if( pBaseInfo->tck_no_rmn_yn == 'N' )
			{
				// 잔여 좌석수
				if( n_rmn_scnt <= 0	)
				{
					continue;
				}
			}

			/// 발권가능여부 N 이면 skip
			if( (pList->trml_tisu_psb_yn[0] == 'N') || (pList->trml_tisu_psb_yn[0] == 'n') )
			{
				continue;	
			}


#endif
			::CopyMemory(&pRPacket->pList[nIndex++], pList, sizeof(rtk_readalcn_list_t));

			// 배차리스트
			CPubTckMem::GetInstance()->m_vtAlcnInfo.push_back(*pList);
		}
	}

	// 배차리스트 갯수 
	//CPubTckMem::GetInstance()->n_alcn_num = nIndex;

	nRet = nIndex;
	if(nIndex <= 0)
	{
		nRet = -99;
		LOG_WRITE("%30s - (%d) ERROR !!!!", "nIndex", nIndex);
		CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
		goto fail_proc;
	}

	
	::ZeroMemory(pRPacket->alcn_num, sizeof(pRPacket->alcn_num));
	::CopyMemory(pRPacket->alcn_num, &nIndex, 4);

	///> recv log
	{
//		for(i = 0; i < nCount; i++)
		for(i = 0; i < nIndex; i++)
		{
			pList = &pRPacket->pList[i];

			n_rot_sqno = *(int *)pList->rot_sqno;
			n_alcn_sqno = *(int *)pList->alcn_sqno;
			n_sats_num = *(int *)pList->sats_num;
			n_rmn_scnt = *(int *)pList->rmn_scnt;
			n_ocnt = *(int *)pList->ocnt;

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nIndex);
			LOG_WRITE("%30s - (%.*s) ", "ROT_ID",				sizeof(pList->rot_id			),	PST_FIELD(pList, rot_id				));
			LOG_WRITE("%30s - (number = %d) ", "ROT_SQNO",	n_rot_sqno);
			LOG_WRITE("%30s - (%.*s) ", "ALCN_DT",			sizeof(pList->alcn_dt			),	PST_FIELD(pList, alcn_dt			));
			LOG_WRITE("%30s - (number = %d) ", "ALCN_SQNO",	n_alcn_sqno);
			LOG_WRITE("%30s - (%.*s) ", "ATL_DEPR_DT",		sizeof(pList->atl_depr_dt		),	PST_FIELD(pList, atl_depr_dt		));
			LOG_WRITE("%30s - (%.*s) ", "ATL_DEPR_TIME",		sizeof(pList->atl_depr_time		),	PST_FIELD(pList, atl_depr_time		));
			LOG_WRITE("%30s - (%.*s) ", "DEPR_TIME",			sizeof(pList->depr_time			),	PST_FIELD(pList, depr_time			));
			LOG_WRITE("%30s - (%.*s) ", "ROT_NM",				sizeof(pList->rot_nm			),	PST_FIELD(pList, rot_nm				));
			LOG_WRITE("%30s - (%.*s) ", "ROT_BUS_DVS_CD",		sizeof(pList->rot_bus_dvs_cd	),	PST_FIELD(pList, rot_bus_dvs_cd		));
			LOG_WRITE("%30s - (%.*s) ", "BUS_CLS_CD",			sizeof(pList->bus_cls_cd		),	PST_FIELD(pList, bus_cls_cd			));
			LOG_WRITE("%30s - (%.*s) ", "BUS_CACM_CD",		sizeof(pList->bus_cacm_cd		),	PST_FIELD(pList, bus_cacm_cd		));
			LOG_WRITE("%30s - (%.*s) ", "ALCN_WAY_DVS_CD",	sizeof(pList->alcn_way_dvs_cd	),	PST_FIELD(pList, alcn_way_dvs_cd	));
			LOG_WRITE("%30s - (%.*s) ", "SATI_USE_YN",		sizeof(pList->sati_use_yn		),	PST_FIELD(pList, sati_use_yn		));
			LOG_WRITE("%30s - (%.*s) ", "PERD_TEMP_YN",		sizeof(pList->perd_temp_yn		),	PST_FIELD(pList, perd_temp_yn		));
			LOG_WRITE("%30s - (%.*s) ", "BCNL_YN",			sizeof(pList->bcnl_yn			),	PST_FIELD(pList, bcnl_yn			));
			LOG_WRITE("%30s - (%.*s) ", "DIST",				sizeof(pList->dist				),	PST_FIELD(pList, dist				));
			LOG_WRITE("%30s - (%.*s) ", "TAKE_DRTM",			sizeof(pList->take_drtm			),	PST_FIELD(pList, take_drtm			));
			LOG_WRITE("%30s - (%.*s) ", "RDHM_MLTP_VAL",		sizeof(pList->rdhm_mltp_val		),	PST_FIELD(pList, rdhm_mltp_val		));
			LOG_WRITE("%30s - (number = %d) ", "SATS_NUM", n_sats_num);
			LOG_WRITE("%30s - (number = %d) ", "RMN_SCNT", n_rmn_scnt);
			LOG_WRITE("%30s - (number = %d) ", "OCNT", n_ocnt);
			LOG_WRITE("%30s - (%.*s) ", "TRML_TISU_PSB_YN",		sizeof(pList->trml_tisu_psb_yn	),	PST_FIELD(pList, trml_tisu_psb_yn	));
			LOG_WRITE("%30s - (%.*s) ", "CTY_BUS_OPRN_SHP_CD",	sizeof(pList->cty_bus_oprn_shp_cd),	PST_FIELD(pList, cty_bus_oprn_shp_cd));
			LOG_WRITE("%30s - (%.*s) ", "DC_PSB_YN",			sizeof(pList->dc_psb_yn			),	PST_FIELD(pList, dc_psb_yn			));
			LOG_WRITE("%30s - (%.*s) ", "WHCH_TISSU_YN",		sizeof(pList->whch_tissu_yn		),	PST_FIELD(pList, whch_tissu_yn		)); // 20211206 ADD
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc131
 * @details		(131) 배차 요금 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc131(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd, nIndex;
	int n_adpt_ltn_hcnt;
	int n_ride_fee;
	int n_dc_aft_amt;		
	int nWhchIndex;		// 20211206 ADD
	int n_sats_no;		// 20211206 ADD
	int n_whch_sats_no; // 20211206 ADD
	char*	pService = "TK_ReadAlcnFee";
	pstk_readalcnfee_t	pSPacket;
	prtk_readalcnfee_t	pRPacket;
	prtk_readalcnfee_list_t pList;
	prtk_readalcnwhch_list_t pWhchList; // 20211206 ADD

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readalcnfee_t) pData;
	pRPacket = (prtk_readalcnfee_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","ROT_ID",				PST_FIELD(pSPacket, rot_id			));
		LOG_WRITE("%30s - (number = %d) ","ROT_SQNO",	*(int *)pSPacket->rot_sqno			);
		LOG_WRITE("%30s - (%s) ","ALCN_DT",				PST_FIELD(pSPacket, alcn_dt			));
		
		LOG_WRITE("%30s - (number = %d) ","ALCN_SQNO",	*(int *)pSPacket->alcn_sqno			);
		LOG_WRITE("%30s - HEX(%02X,%02X,%02X,%02X,%02X) ","ALCN_SQNO",	
					pSPacket->alcn_sqno[0] & 0xFF, pSPacket->alcn_sqno[1] & 0xFF, 
					pSPacket->alcn_sqno[2] & 0xFF, pSPacket->alcn_sqno[3] & 0xFF, 
					pSPacket->alcn_sqno[4] & 0xFF);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_CD",		PST_FIELD(pSPacket, depr_trml_cd	));
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_CD",		PST_FIELD(pSPacket, arvl_trml_cd	));
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(ROT_ID				,	PST_FIELD(pSPacket, rot_id				));
// 		{
// 			::ZeroMemory(Buffer, sizeof(Buffer));
// 			nRet = TMaxFBGet(ROT_ID, Buffer);
// 			LOG_WRITE("-- %30s - (%s) ", "ROT_ID",	Buffer);
// 		}

		nRet = TMaxFBPut(ROT_SQNO			,	PST_FIELD(pSPacket, rot_sqno			));
// 		{
// 			::ZeroMemory(Buffer, sizeof(Buffer));
// 			nRet = TMaxFBGet(ROT_SQNO, Buffer);
// 			LOG_WRITE("-- %30s - (%s) ", "ROT_SQNO",	Buffer);
// 		}
		nRet = TMaxFBPut(ALCN_DT			,	PST_FIELD(pSPacket, alcn_dt				));
// 		{
// 			::ZeroMemory(Buffer, sizeof(Buffer));
// 			nRet = TMaxFBGet(ALCN_DT, Buffer);
// 			LOG_WRITE("-- %30s - (%s) ", "ALCN_DT",	Buffer);
//		}
		nRet = TMaxFBPut(ALCN_SQNO			,	PST_FIELD(pSPacket, alcn_sqno			));
// 		{
// 			::ZeroMemory(Buffer, sizeof(Buffer));
// 			nRet = TMaxFBGet(ALCN_SQNO, Buffer);
// 			LOG_WRITE("-- %30s - (%s) ", "ALCN_SQNO",	Buffer);
// 		}
		nRet = TMaxFBPut(DEPR_TRML_CD		,	PST_FIELD(pSPacket, depr_trml_cd		));
// 		{
// 			::ZeroMemory(Buffer, sizeof(Buffer));
// 			nRet = TMaxFBGet(DEPR_TRML_CD, Buffer);
// 			LOG_WRITE("-- %30s - (%s) ", "DEPR_TRML_CD",	Buffer);
// 		}
		nRet = TMaxFBPut(ARVL_TRML_CD		,	PST_FIELD(pSPacket, arvl_trml_cd		));
// 		{
// 			::ZeroMemory(Buffer, sizeof(Buffer));
// 			nRet = TMaxFBGet(ARVL_TRML_CD, Buffer);
// 			LOG_WRITE("-- %30s - (%s) ", "ARVL_TRML_CD",	Buffer);
// 		}
	}

	// 동기화 (Send/Recv)
	nTimeout = 10;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	nIndex = 0;
	nWhchIndex = 0; // 20211206 ADD

	///> recv data
	{
		CPubTckMem* pTr;

		pTr = CPubTckMem::GetInstance();

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}		

		nRet = TMaxFBGetF(SATS_ARCT_CD,			PST_FIELD(pRPacket, sats_arct_cd));
		LOG_WRITE("%30s - (%s) ", "SATS_ARCT_CD", PST_FIELD(pRPacket, sats_arct_cd	));
		::CopyMemory(pTr->sats_arct_cd, pRPacket->sats_arct_cd, sizeof(pRPacket->sats_arct_cd));

		nRet = TMaxFBGetF(SATS_STA_INF,			PST_FIELD(pRPacket, sats_sta_inf));
		LOG_WRITE("%30s - (%s) ", "SATS_STA_INF", PST_FIELD(pRPacket, sats_sta_inf	));
		::CopyMemory(pTr->sats_sta_inf, pRPacket->sats_sta_inf, sizeof(pRPacket->sats_sta_inf));

		nRet = TMaxFBGetF(RSD_NOP_CARD_DC_YN,	PST_FIELD(pRPacket, rsd_nop_card_dc_yn));
		LOG_WRITE("%30s - (%s) ", "RSD_NOP_CARD_DC_YN", PST_FIELD(pRPacket, rsd_nop_card_dc_yn	));
		::CopyMemory(pTr->rsd_nop_card_dc_yn, pRPacket->rsd_nop_card_dc_yn, sizeof(pRPacket->rsd_nop_card_dc_yn));

		nRet = TMaxFBGetF(FEE_INF_NUM,	PST_FIELD(pRPacket, fee_inf_num)); // 요금 정보 수
		nCount = *(int *)pRPacket->fee_inf_num;
		LOG_WRITE("%30s - (number = %d) ", "FEE_INF_NUM", nCount);
		if(nCount <= 0)
		{
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_readalcnfee_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readalcnfee_list_t) * nCount);
		}

		pTr->m_vtFeeInfo.clear();
		for(i = 0; i < nCount; i++)
		{
			rtk_readalcnfee_list_t stList;

			::ZeroMemory(&stList, sizeof(rtk_readalcnfee_list_t));

			//pList = &pRPacket->pList[i];
			pList = &stList;

			nRet = TMaxFBGetF(CTY_BUS_DC_KND_CD		,	PST_FIELD(pList, cty_bus_dc_knd_cd	));	///< 시외버스할인종류코드
			nRet = TMaxFBGetF(DCRT_DVS_CD			,	PST_FIELD(pList, dcrt_dvs_cd		));	///< 할인율구분코드
			nRet = TMaxFBGetF(DC_RNG_MLTP_VAL		,	PST_FIELD(pList, dc_rng_mltp_val	));	///< 할인구간다중값
			nRet = TMaxFBGetF(ADPT_LTN_HCNT			,	PST_FIELD(pList, adpt_ltn_hcnt		));	///< 적용제한매수(number)
			nRet = TMaxFBGetF(BUS_TCK_KND_CD		,	PST_FIELD(pList, bus_tck_knd_cd		));	///< 버스티켓종류코드
			nRet = TMaxFBGetF(RIDE_FEE				,	PST_FIELD(pList, ride_fee			));	///< 승차요금(number)
			nRet = TMaxFBGetF(DC_AFT_AMT			,	PST_FIELD(pList, dc_aft_amt			));	///< 할인이후금액(number)

			n_adpt_ltn_hcnt	= *(int *) pList->adpt_ltn_hcnt;	///< 적용제한매수(number)
			n_ride_fee		= *(int *) pList->ride_fee;			///< 승차요금(number)
			n_dc_aft_amt	= *(int *) pList->dc_aft_amt;		///< 할인이후금액(number)

			// 승차요금 == 0
			if(pList->ride_fee <= 0)
			{
				continue;
			}

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_DC_KND_CD",		PST_FIELD(pList, cty_bus_dc_knd_cd	));
			LOG_WRITE("%30s - (%s) ", "DCRT_DVS_CD",				PST_FIELD(pList, dcrt_dvs_cd		));
			LOG_WRITE("%30s - (%s) ", "DC_RNG_MLTP_VAL",			PST_FIELD(pList, dc_rng_mltp_val	));
			LOG_WRITE("%30s - (number = %d) ",   "ADPT_LTN_HCNT",	*(int *)pList->adpt_ltn_hcnt);
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD",			PST_FIELD(pList, bus_tck_knd_cd		));
			LOG_WRITE("%30s - (number = %d) ",   "RIDE_FEE",		*(int *) pList->ride_fee);
			LOG_WRITE("%30s - (number = %d) ",   "DC_AFT_AMT",	*(int *) pList->dc_aft_amt);

			::CopyMemory(&pRPacket->pList[nIndex++], pList, sizeof(rtk_readalcnfee_list_t));
			pTr->m_vtFeeInfo.push_back(*pList);

		}

		// 20211206 ADD~
		nRet = TMaxFBGetF(WHCH_SATS_NUM,	PST_FIELD(pRPacket, whch_sats_num)); // 휠체어좌석수
		nCount = *(int *)pRPacket->whch_sats_num;
		LOG_WRITE("%30s - (number = %d) ", "WHCH_SATS_NUM", nCount);
		if( (nCount != 0) && (nCount < 0) )
		{
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pWhchList = new rtk_readalcnwhch_list_t[nCount];
			::ZeroMemory(pRPacket->pWhchList, sizeof(rtk_readalcnwhch_list_t) * nCount);
		}

		pTr->m_vtWhchInfo.clear();
		for(i = 0; i < nCount; i++)
		{
			rtk_readalcnwhch_list_t stList;

			::ZeroMemory(&stList, sizeof(rtk_readalcnwhch_list_t));

			//pWhchList = &pRPacket->pWhchList[i];
			pWhchList = &stList;

			nRet = TMaxFBGetF(SATS_NO,	PST_FIELD(pWhchList, sats_no			));					///< 좌석번호(number)
			nRet = TMaxFBGetF(WHCH_SATS_NO,	PST_FIELD(pWhchList, whch_sats_no			));			///< 휠체어좌석번호(number)
			nRet = TMaxFBGetF(WHCH_SATS_NO_PRIN_NM, PST_FIELD(pWhchList, whch_sats_no_prin_nm));	///< 휠체어좌석번호출력명

			n_sats_no		= *(int *) pWhchList->sats_no;	///< 좌석번호(number)
			n_whch_sats_no	= *(int *) pWhchList->whch_sats_no;	///< 휠체어좌석번호(number)

			//// 좌석번호 == 0
			//if(pWhchList->sats_no <= 0)
			//{
			//	continue;
			//}
			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (number = %d) ",   "SATS_NO",	*(int *)pWhchList->sats_no);
			LOG_WRITE("%30s - (number = %d) ",   "WHCH_SATS_NO",	*(int *)pWhchList->whch_sats_no);
			LOG_WRITE("%30s - (%s) ", "WHCH_SATS_NO_PRIN_NM",		PST_FIELD(pWhchList, whch_sats_no_prin_nm));

			::CopyMemory(&pRPacket->pWhchList[nWhchIndex++], pWhchList, sizeof(rtk_readalcnwhch_list_t));
			pTr->m_vtWhchInfo.push_back(*pWhchList);

		}

		LOG_WRITE("%30s - (number = %d) ",   "n_fee_num",	nIndex);
		pTr->n_fee_num = nRet = nIndex;

		LOG_WRITE("%30s - (number = %d) ",   "n_whch_sats_num",	nWhchIndex);
		pTr->n_whch_sats_num = nWhchIndex;
		// 20211206 ~ADD
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc132
 * @details		(132) 승차권 발행 - 현금
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc132(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int nRet, nTimeout, nCount, i;
	int n_rsp_cd;
	char*	pService = "TK_PubTckCash";
	pstk_pubtckcash_t	pSPacket;
	prtk_pubtckcash_t	pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_pubtckcash_t) pData;
	pRPacket = (prtk_pubtckcash_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","ROT_ID",			PST_FIELD(pSPacket, rot_id			));
		LOG_WRITE("%30s - (number = %d) ","ROT_SQNO",	*(int *)pSPacket->rot_sqno			);
		LOG_WRITE("%30s - (%s) ","ALCN_DT",			PST_FIELD(pSPacket, alcn_dt			));
		LOG_WRITE("%30s - (number = %d) ","ALCN_SQNO",*(int *)pSPacket->alcn_sqno			);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_CD",		PST_FIELD(pSPacket, depr_trml_cd	));
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_CD",		PST_FIELD(pSPacket, arvl_trml_cd	));
		LOG_WRITE("%30s - (%s) ","PYN_MNS_DVS_CD",	PST_FIELD(pSPacket, pyn_mns_dvs_cd	));
		LOG_WRITE("%30s - (%s) ","PUB_CHNL_DVS_CD",	PST_FIELD(pSPacket, pub_chnl_dvs_cd	));
		LOG_WRITE("%30s - (%s) ","TISU_WAY_DVS_CD",	PST_FIELD(pSPacket, tisu_way_dvs_cd	));
		LOG_WRITE("%30s - (%s) ","USER_DVS_NO",		PST_FIELD(pSPacket, user_dvs_no		));

		LOG_WRITE("%30s - (number = %d) ","PUB_NUM",	*(int *)pSPacket->pub_num			);
		nCount = *(int *)pSPacket->pub_num;
		for(i = 0; i < nCount; i++)
		{
			pstk_pubtckcash_list_t pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("%30s - (%s) ","BUS_TCK_KND_CD",	PST_FIELD(pList, bus_tck_knd_cd	));
			LOG_WRITE("%30s - (%s) ","SATS_PCPY_ID",		PST_FIELD(pList, sats_pcpy_id	));
			LOG_WRITE("%30s - (number = %d) ","SATS_NO",	*(int *)pList->sats_no			);
		}
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(ROT_ID				,	PST_FIELD(pSPacket,	rot_id				));	///> 노선ID			
		nRet = TMaxFBPut(ROT_SQNO			,	PST_FIELD(pSPacket,	rot_sqno			));	///> 노선순번		
		nRet = TMaxFBPut(ALCN_DT			,	PST_FIELD(pSPacket,	alcn_dt				));	///> 배차일자		
		nRet = TMaxFBPut(ALCN_SQNO			,	PST_FIELD(pSPacket,	alcn_sqno			));	///> 배차순번		
		nRet = TMaxFBPut(DEPR_TRML_CD		,	PST_FIELD(pSPacket,	depr_trml_cd		));	///> 출발터미널코드	
		nRet = TMaxFBPut(ARVL_TRML_CD		,	PST_FIELD(pSPacket,	arvl_trml_cd		));	///> 도착터미널코드	
		nRet = TMaxFBPut(PYN_MNS_DVS_CD		,	PST_FIELD(pSPacket,	pyn_mns_dvs_cd		));	///> 지불수단구분코드
		nRet = TMaxFBPut(PUB_CHNL_DVS_CD	,	PST_FIELD(pSPacket,	pub_chnl_dvs_cd		));	///> 발행채널구분코드
		nRet = TMaxFBPut(TISU_WAY_DVS_CD	,	PST_FIELD(pSPacket,	tisu_way_dvs_cd		));	///> 발권방식구분코드
		nRet = TMaxFBPut(USER_DVS_NO		,	PST_FIELD(pSPacket,	user_dvs_no			));	///> 사용자구분번호	
		nRet = TMaxFBPut(PUB_NUM			,	PST_FIELD(pSPacket,	pub_num				));	///> 발행매수		

		nCount = *(int *)pSPacket->pub_num;
		for(i = 0; i < nCount; i++)
		{
			pstk_pubtckcash_list_t pList;

			pList = &pSPacket->List[i];

			///> 버스티켓종류코드
			nRet = TMaxFBInsert(BUS_TCK_KND_CD, PST_FIELD(pList, bus_tck_knd_cd), i, sizeof(pList->bus_tck_knd_cd) - 1);
			///> 좌석선점ID		
			nRet = TMaxFBInsert(SATS_PCPY_ID, PST_FIELD(pList, sats_pcpy_id), i, sizeof(pList->sats_pcpy_id) - 1);
			///> 좌석번호
			nRet = TMaxFBInsert(SATS_NO, PST_FIELD(pList, sats_no), i, sizeof(pList->sats_no) - 1);
		}
	}

	// 동기화 (Send/Recv)
	nTimeout = 10;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	///> recv data
	{
		CPubTckMem* pTr;

		/// 현장발권 mem info
		pTr = CPubTckMem::GetInstance();

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 146 )
		{	// 에러코드 조회
			nRet = -98;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}		
		
		nRet = TMaxFBGetF(CSRC_AUTH_NO			,	PST_FIELD(pRPacket,	csrc_auth_no			));	 ///> 현금영수증인증번호		
		nRet = TMaxFBGetF(CSRC_APRV_VANC_CD		,	PST_FIELD(pRPacket,	csrc_aprv_vanc_cd		));	 ///> 현금영수증승인VAN사코드	
		nRet = TMaxFBGetF(CSRC_BRN				,	PST_FIELD(pRPacket,	csrc_brn				));	 ///> 현금영수증사업자등록번호
		nRet = TMaxFBGetF(ATL_DEPR_DT			,	PST_FIELD(pRPacket,	atl_depr_dt				));	 ///> 실제출발일자			
		nRet = TMaxFBGetF(ATL_DEPR_TIME			,	PST_FIELD(pRPacket,	atl_depr_time			));	 ///> 실제출발시각			
		nRet = TMaxFBGetF(DEPR_TRML_CD			,	PST_FIELD(pRPacket,	depr_trml_cd			));	 ///> 출발터미널코드			
		nRet = TMaxFBGetF(ARVL_TRML_CD			,	PST_FIELD(pRPacket,	arvl_trml_cd			));	 ///> 도착터미널코드			
		nRet = TMaxFBGetF(STPT_TRML_CD			,	PST_FIELD(pRPacket,	stpt_trml_cd			));	 ///> 기점터미널코드			
		nRet = TMaxFBGetF(EPT_TRML_CD			,	PST_FIELD(pRPacket,	ept_trml_cd				));	 ///> 종점터미널코드			
		nRet = TMaxFBGetF(BUS_CLS_CD			,	PST_FIELD(pRPacket,	bus_cls_cd				));	 ///> 버스등급코드			
		nRet = TMaxFBGetF(BUS_CACM_CD			,	PST_FIELD(pRPacket,	bus_cacm_cd				));	 ///> 버스운수사코드			
		nRet = TMaxFBGetF(RDHM_MLTP_VAL			,	PST_FIELD(pRPacket,	rdhm_mltp_val			));	 ///> 승차홈다중값			
		nRet = TMaxFBGetF(DIST					,	PST_FIELD(pRPacket,	dist					));	 ///> 거리					
		nRet = TMaxFBGetF(TAKE_DRTM				,	PST_FIELD(pRPacket,	take_drtm				));	 ///> 소요시간				
		nRet = TMaxFBGetF(BUS_CLS_PRIN_YN		,	PST_FIELD(pRPacket,	bus_cls_prin_yn			));	 ///> 버스등급출력여부		
		nRet = TMaxFBGetF(BUS_CACM_NM_PRIN_YN	,	PST_FIELD(pRPacket,	bus_cacm_nm_prin_yn		));	 ///> 버스운수사명출력여부	
		nRet = TMaxFBGetF(DEPR_TIME_PRIN_YN		,	PST_FIELD(pRPacket,	depr_time_prin_yn		));	 ///> 출발시각출력여부		
		nRet = TMaxFBGetF(SATS_NO_PRIN_YN		,	PST_FIELD(pRPacket,	sats_no_prin_yn			));	 ///> 좌석번호출력여부		
		nRet = TMaxFBGetF(ALCN_WAY_DVS_CD		,	PST_FIELD(pRPacket,	alcn_way_dvs_cd			));	 ///> 배차방식구분코드		
		nRet = TMaxFBGetF(PERD_TEMP_YN			,	PST_FIELD(pRPacket,	perd_temp_yn			));	 ///> 정기임시여부			
		nRet = TMaxFBGetF(PUB_DT				,	PST_FIELD(pRPacket,	pub_dt					));	 ///> 발행일자				
		nRet = TMaxFBGetF(PUB_TIME				,	PST_FIELD(pRPacket,	pub_time				));	 ///> 발행시각				
		nRet = TMaxFBGetF(PUB_SHCT_TRML_CD		,	PST_FIELD(pRPacket,	pub_shct_trml_cd		));	 ///> 발행단축터미널코드		
		nRet = TMaxFBGetF(PUB_WND_NO			,	PST_FIELD(pRPacket,	pub_wnd_no				));	 ///> 발행창구번호			

		if (IsAlcn() == FALSE)
		{ // 비배차는 시간 아예 안찍음. 시간표는 옵션에 따름. 
			//_tran.tK_PubTck.ATL_DEPR_TIME = "999999";
			sprintf(pRPacket->atl_depr_time, "999999");
		}

		// 현장발권 mem - cash_resp 복사
		{
			::CopyMemory(pTr->cash_resp.csrc_auth_no, pRPacket->csrc_auth_no, pRPacket->pub_num - pRPacket->csrc_auth_no);
		}

		nRet = TMaxFBGetF(PUB_NUM,	PST_FIELD(pRPacket,	pub_num	));	 ///> 발행매수

		nCount = *(int *)pRPacket->pub_num;
		if(nCount <= 0)
		{
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_pubtckcash_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_pubtckcash_list_t) * nCount);
		}

		pTr->m_vtPbTckCash.clear();
		for(i = 0; i < nCount; i++)
		{
			prtk_pubtckcash_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(PYN_MNS_DVS_CD		,	PST_FIELD(pList,	pyn_mns_dvs_cd	 ));	 ///> 지불수단구분코드		
			nRet = TMaxFBGetF(CSRC_APRV_NO			,	PST_FIELD(pList,	csrc_aprv_no	 ));	 ///> 현금영수증승인번호		
			nRet = TMaxFBGetF(BUS_TCK_KND_CD		,	PST_FIELD(pList,	bus_tck_knd_cd	 ));	 ///> 버스티켓종류코드		
			nRet = TMaxFBGetF(PUB_SNO				,	PST_FIELD(pList,	pub_sno			 ));	 ///> 발행일련번호			
			nRet = TMaxFBGetF(BUS_TCK_INHR_NO		,	PST_FIELD(pList,	bus_tck_inhr_no	 ));	 ///> 버스티켓고유번호		
			nRet = TMaxFBGetF(TISU_AMT				,	PST_FIELD(pList,	tisu_amt		 ));	 ///> 발권금액				
			nRet = TMaxFBGetF(SATS_NO				,	PST_FIELD(pList,	sats_no			 ));	 ///> 좌석번호				
			nRet = TMaxFBGetF(CTY_BUS_DC_KND_CD		,	PST_FIELD(pList,	cty_bus_dc_knd_cd));	 ///> 시외버스할인종류코드	
			nRet = TMaxFBGetF(DCRT_DVS_CD			,	PST_FIELD(pList,	dcrt_dvs_cd		 ));	 ///> 할인율구분코드			
			nRet = TMaxFBGetF(DC_BEF_AMT			,	PST_FIELD(pList,	dc_bef_amt		 ));	 ///> 할인이전금액			

			// 현장발권 mem add
			pTr->m_vtPbTckCash.push_back(*pList);
		}
	}																								  
																									  
	///> Log out
	{
		LOG_WRITE("%30s - (%s) ", "RSP_CD",				PST_FIELD(pRPacket, rsp_cd				));	///> 응답코드				
		LOG_WRITE("%30s - (%s) ", "CSRC_AUTH_NO",			PST_FIELD(pRPacket, csrc_auth_no		));	///> 현금영수증인증번호		
		LOG_WRITE("%30s - (%s) ", "CSRC_APRV_VANC_CD",	PST_FIELD(pRPacket, csrc_aprv_vanc_cd	));	///> 현금영수증승인VAN사코드	
		LOG_WRITE("%30s - (%s) ", "CSRC_BRN",				PST_FIELD(pRPacket, csrc_brn			));	///> 현금영수증사업자등록번호
		LOG_WRITE("%30s - (%s) ", "ATL_DEPR_DT",			PST_FIELD(pRPacket, atl_depr_dt			));	///> 실제출발일자			
		LOG_WRITE("%30s - (%s) ", "ATL_DEPR_TIME",		PST_FIELD(pRPacket, atl_depr_time		));	///> 실제출발시각			
		LOG_WRITE("%30s - (%s) ", "DEPR_TRML_CD",			PST_FIELD(pRPacket, depr_trml_cd		));	///> 출발터미널코드			
		LOG_WRITE("%30s - (%s) ", "ARVL_TRML_CD",			PST_FIELD(pRPacket, arvl_trml_cd		));	///> 도착터미널코드			
		LOG_WRITE("%30s - (%s) ", "STPT_TRML_CD",			PST_FIELD(pRPacket, stpt_trml_cd		));	///> 기점터미널코드			
		LOG_WRITE("%30s - (%s) ", "EPT_TRML_CD",			PST_FIELD(pRPacket, ept_trml_cd			));	///> 종점터미널코드			
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD",			PST_FIELD(pRPacket, bus_cls_cd			));	///> 버스등급코드			
		LOG_WRITE("%30s - (%s) ", "BUS_CACM_CD",			PST_FIELD(pRPacket, bus_cacm_cd			));	///> 버스운수사코드			
		LOG_WRITE("%30s - (%s) ", "RDHM_MLTP_VAL",		PST_FIELD(pRPacket, rdhm_mltp_val		));	///> 승차홈다중값			
		LOG_WRITE("%30s - (number = %d) ", "DIST",		*(int *)pRPacket->dist);	///> 거리					
		LOG_WRITE("%30s - (number = %d) ", "TAKE_DRTM",	*(int *)pRPacket->take_drtm);	///> 소요시간				
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_PRIN_YN",		PST_FIELD(pRPacket, bus_cls_prin_yn		));	///> 버스등급출력여부		
		LOG_WRITE("%30s - (%s) ", "BUS_CACM_NM_PRIN_YN",	PST_FIELD(pRPacket, bus_cacm_nm_prin_yn	));	///> 버스운수사명출력여부	
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME_PRIN_YN",	PST_FIELD(pRPacket, depr_time_prin_yn	));	///> 출발시각출력여부		
		LOG_WRITE("%30s - (%s) ", "SATS_NO_PRIN_YN",		PST_FIELD(pRPacket, sats_no_prin_yn		));	///> 좌석번호출력여부		
		LOG_WRITE("%30s - (%s) ", "ALCN_WAY_DVS_CD",		PST_FIELD(pRPacket, alcn_way_dvs_cd		));	///> 배차방식구분코드		
		LOG_WRITE("%30s - (%s) ", "PERD_TEMP_YN",			PST_FIELD(pRPacket, perd_temp_yn		));	///> 정기임시여부			
		LOG_WRITE("%30s - (%s) ", "PUB_DT",				PST_FIELD(pRPacket, pub_dt				));	///> 발행일자				
		LOG_WRITE("%30s - (%s) ", "PUB_TIME",				PST_FIELD(pRPacket, pub_time			));	///> 발행시각				
		LOG_WRITE("%30s - (%s) ", "PUB_SHCT_TRML_CD",		PST_FIELD(pRPacket, pub_shct_trml_cd	));	///> 발행단축터미널코드		
		LOG_WRITE("%30s - (%s) ", "PUB_WND_NO",			PST_FIELD(pRPacket, pub_wnd_no			));	///> 발행창구번호			
		LOG_WRITE("%30s - (number = %d) ", "PUB_NUM",		*(int *)pRPacket->pub_num);	///> 발행매수			

		nCount = *(int *)pRPacket->pub_num;
		for(i = 0; i < nCount; i++)
		{
			prtk_pubtckcash_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("%30s - (%s) ", "PYN_MNS_DVS_CD",		PST_FIELD(pList, pyn_mns_dvs_cd ));		///> 지불수단구분코드		
			LOG_WRITE("%30s - (%s) ", "CSRC_APRV_NO",			PST_FIELD(pList, csrc_aprv_no ));		///> 현금영수증승인번호		
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD",		PST_FIELD(pList, bus_tck_knd_cd	));		///> 버스티켓종류코드		
			LOG_WRITE("%30s - (number = %d) ", "PUB_SNO",		*(int *)pList->pub_sno);				///> 발행일련번호			
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_INHR_NO",		PST_FIELD(pList, bus_tck_inhr_no ));	///> 버스티켓고유번호		
			LOG_WRITE("%30s - (number = %d) ", "TISU_AMT",	*(int *)pList->tisu_amt);				///> 발권금액				
			LOG_WRITE("%30s - (number = %d) ", "SATS_NO",		*(int *)pList->sats_no);				///> 좌석번호				
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_DC_KND_CD",	PST_FIELD(pList, cty_bus_dc_knd_cd	));	///> 시외버스할인종류코드	
			LOG_WRITE("%30s - (%s) ", "DCRT_DVS_CD",			PST_FIELD(pList, dcrt_dvs_cd ));		///> 할인율구분코드			
			LOG_WRITE("%30s - (number = %d) ", "DC_BEF_AMT",	*(int *)pList->dc_bef_amt);				///> 할인이전금액			
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc134
 * @details		(134) 승차권 발행 - 현금영수증
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc134(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, nTimeout, i, nCount;
	int n_rsp_cd;
	char*	pService = "TK_PubTckCsrc";
	pstk_pubtckcsrc_t	pSPacket;
	prtk_pubtckcsrc_t	pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_pubtckcsrc_t) pData;
	pRPacket = (prtk_pubtckcsrc_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","ROT_ID",			PST_FIELD(pSPacket, rot_id));
		LOG_WRITE("%30s - (%s) ","ROT_SQNO",			PST_FIELD(pSPacket, rot_sqno));
		LOG_WRITE("%30s - (%s) ","ALCN_DT",			PST_FIELD(pSPacket, alcn_dt));
		LOG_WRITE("%30s - (%s) ","ALCN_SQNO",			PST_FIELD(pSPacket, alcn_sqno));
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_CD",		PST_FIELD(pSPacket, depr_trml_cd));
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_CD",		PST_FIELD(pSPacket, arvl_trml_cd));
		LOG_WRITE("%30s - (%s) ","TISU_WAY_DVS_CD",	PST_FIELD(pSPacket, tisu_way_dvs_cd));
		LOG_WRITE("%30s - (%s) ","PYN_MNS_DVS_CD",	PST_FIELD(pSPacket, pyn_mns_dvs_cd));
		LOG_WRITE("%30s - (%s) ","PUB_CHNL_DVS_CD",	PST_FIELD(pSPacket, pub_chnl_dvs_cd));
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ","DAMO_ENC_CSRC_AUTH_NO",	PST_FIELD(pSPacket, damo_enc_csrc_auth_no));
		LOG_WRITE("%30s - (num=%d) ","DAMO_ENC_CSRC_AUTH_NO_LEN",	*(int *)pSPacket->damo_enc_csrc_auth_no_len);
		LOG_WRITE("%30s - (%s) ","DAMO_ENC_DTA_KEY",	PST_FIELD(pSPacket, damo_enc_dta_key));
#endif
		LOG_WRITE("%30s - (%s) ","CPRT_YN",			PST_FIELD(pSPacket, cprt_yn));
		LOG_WRITE("%30s - (%s) ","USER_DVS_NO",		PST_FIELD(pSPacket, user_dvs_no));
		
		LOG_WRITE("%30s - (num=%d) ","PUB_NUM",			*(int *)pSPacket->pub_num);
		nCount = *(int *) pSPacket->pub_num;
		for(i = 0; i < nCount; i++)
		{
			pstk_pubtckcsrc_list_t pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("## Index (%d) ", i);	///> 

			LOG_WRITE("%30s - (%s) "	, "BUS_TCK_KND_CD",		PST_FIELD(pList, bus_tck_knd_cd));
			LOG_WRITE("%30s - (%s) "	, "SATS_PCPY_ID",		PST_FIELD(pList, sats_pcpy_id)	);
			LOG_WRITE("%30s - (num=%d) ", "SATS_NO",				*(int *)pList->sats_no			);
		}
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(ROT_ID							,	PST_FIELD(pSPacket,	rot_id						));	///> 노선id						
		nRet = TMaxFBPut(ROT_SQNO						,	PST_FIELD(pSPacket,	rot_sqno					));	///> 노선순번					
		nRet = TMaxFBPut(ALCN_DT						,	PST_FIELD(pSPacket,	alcn_dt						));	///> 배차일자					
		nRet = TMaxFBPut(ALCN_SQNO						,	PST_FIELD(pSPacket,	alcn_sqno					));	///> 배차순번					
		nRet = TMaxFBPut(DEPR_TRML_CD					,	PST_FIELD(pSPacket,	depr_trml_cd				));	///> 출발터미널코드				
		nRet = TMaxFBPut(ARVL_TRML_CD					,	PST_FIELD(pSPacket,	arvl_trml_cd				));	///> 도착터미널코드				
		nRet = TMaxFBPut(TISU_WAY_DVS_CD				,	PST_FIELD(pSPacket,	tisu_way_dvs_cd				));	///> 발권방식구분코드			
		nRet = TMaxFBPut(PYN_MNS_DVS_CD					,	PST_FIELD(pSPacket,	pyn_mns_dvs_cd				));	///> 지불수단구분코드			
		nRet = TMaxFBPut(PUB_CHNL_DVS_CD				,	PST_FIELD(pSPacket,	pub_chnl_dvs_cd				));	///> 발행채널구분코드			
		nRet = TMaxFBPut(DAMO_ENC_CSRC_AUTH_NO			,	PST_FIELD(pSPacket,	damo_enc_csrc_auth_no		));	///> 현금영수증인증번호암호문	
		nRet = TMaxFBPut(DAMO_ENC_CSRC_AUTH_NO_LEN		,	PST_FIELD(pSPacket,	damo_enc_csrc_auth_no_len	));	///> 현금영수증인증번호암호문길이
		nRet = TMaxFBPut(DAMO_ENC_DTA_KEY				,	PST_FIELD(pSPacket,	damo_enc_dta_key			));	///> 암호문키					
		nRet = TMaxFBPut(CPRT_YN						,	PST_FIELD(pSPacket,	cprt_yn						));	///> 법인여부					
		nRet = TMaxFBPut(USER_DVS_NO					,	PST_FIELD(pSPacket,	user_dvs_no					));	///> 사용자구분번호				

		nRet = TMaxFBPut(PUB_NUM						,	PST_FIELD(pSPacket,	pub_num						));	///> 발행매수					
		nCount = *(int *) pSPacket->pub_num;
		for(i = 0; i < nCount; i++)
		{
			pstk_pubtckcsrc_list_t pList;

			pList = &pSPacket->List[i];

			nRet = TMaxFBInsert(BUS_TCK_KND_CD	, PST_FIELD(pList, bus_tck_knd_cd), i, sizeof(pList->bus_tck_knd_cd) - 1);
			nRet = TMaxFBInsert(SATS_PCPY_ID	, PST_FIELD(pList, sats_pcpy_id), i, sizeof(pList->sats_pcpy_id) - 1);
			nRet = TMaxFBInsert(SATS_NO			, PST_FIELD(pList, sats_no), i, sizeof(pList->sats_no) - 1);
		}

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(pSPacket->damo_enc_csrc_auth_no, sizeof(pSPacket->damo_enc_csrc_auth_no));
		KTC_MemClear(pSPacket->damo_enc_dta_key, sizeof(pSPacket->damo_enc_dta_key));
#endif

	}

	// 동기화 (Send/Recv)
	nTimeout = 10;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		CPubTckMem* pTr;

		pTr = CPubTckMem::GetInstance();

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( (n_rsp_cd != 146) && (n_rsp_cd != 451) )
		{	// 에러코드 조회
			if(n_rsp_cd == 428)
			{
				//pRPacket->rsp_msg;
			}
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		if(n_rsp_cd == 451)
		{	/// 현금영수증 등록에 실패했습니다. \n자진발급으로 발행합니다.
			//_tran.tK_PubTck.PYN_MNS_DVS_CD = ConstMsg.PYM_CD.CASH;
		}

		nRet = TMaxFBGetF(RSP_MSG				,	PST_FIELD(pRPacket,	rsp_msg				));	 ///> 응답 메시지				
		nRet = TMaxFBGetF(CSRC_AUTH_NO			,	PST_FIELD(pRPacket,	csrc_auth_no		));	 ///> 현금영수증인증번호		
		nRet = TMaxFBGetF(CSRC_BRN				,	PST_FIELD(pRPacket,	csrc_brn			));	 ///> 현금영수증사업자등록번호
		nRet = TMaxFBGetF(CSRC_APRV_VANC_CD		,	PST_FIELD(pRPacket,	csrc_aprv_vanc_cd	));	 ///> 현금영수증승인van사코드	
		nRet = TMaxFBGetF(ATL_DEPR_DT			,	PST_FIELD(pRPacket,	atl_depr_dt			));	 ///> 실제출발일자			
		nRet = TMaxFBGetF(ATL_DEPR_TIME			,	PST_FIELD(pRPacket,	atl_depr_time		));	 ///> 실제출발시각			
		nRet = TMaxFBGetF(DEPR_TRML_CD			,	PST_FIELD(pRPacket,	depr_trml_cd		));	 ///> 출발터미널코드			
		nRet = TMaxFBGetF(ARVL_TRML_CD			,	PST_FIELD(pRPacket,	arvl_trml_cd		));	 ///> 도착터미널코드			
		nRet = TMaxFBGetF(STPT_TRML_CD			,	PST_FIELD(pRPacket,	stpt_trml_cd		));	 ///> 기점터미널코드			
		nRet = TMaxFBGetF(EPT_TRML_CD			,	PST_FIELD(pRPacket,	ept_trml_cd			));	 ///> 종점터미널코드			
		nRet = TMaxFBGetF(BUS_CLS_CD			,	PST_FIELD(pRPacket,	bus_cls_cd			));	 ///> 버스등급코드			
		nRet = TMaxFBGetF(BUS_CACM_CD			,	PST_FIELD(pRPacket,	bus_cacm_cd			));	 ///> 버스운수사코드			
		nRet = TMaxFBGetF(RDHM_MLTP_VAL			,	PST_FIELD(pRPacket,	rdhm_mltp_val		));	 ///> 승차홈다중값			
		nRet = TMaxFBGetF(DIST					,	PST_FIELD(pRPacket,	dist				));	 ///> 거리					
		nRet = TMaxFBGetF(TAKE_DRTM				,	PST_FIELD(pRPacket,	take_drtm			));	 ///> 소요시간				
		nRet = TMaxFBGetF(BUS_CLS_PRIN_YN		,	PST_FIELD(pRPacket,	bus_cls_prin_yn		));	 ///> 버스등급출력여부		
		nRet = TMaxFBGetF(BUS_CACM_NM_PRIN_YN	,	PST_FIELD(pRPacket,	bus_cacm_nm_prin_yn	));	 ///> 버스운수사명출력여부	
		nRet = TMaxFBGetF(DEPR_TIME_PRIN_YN		,	PST_FIELD(pRPacket,	depr_time_prin_yn	));	 ///> 출발시각출력여부		
		nRet = TMaxFBGetF(SATS_NO_PRIN_YN		,	PST_FIELD(pRPacket,	sats_no_prin_yn		));	 ///> 좌석번호출력여부		
		nRet = TMaxFBGetF(ALCN_WAY_DVS_CD		,	PST_FIELD(pRPacket,	alcn_way_dvs_cd		));	 ///> 배차방식구분코드		
		nRet = TMaxFBGetF(PERD_TEMP_YN			,	PST_FIELD(pRPacket,	perd_temp_yn		));	 ///> 정기임시여부			
		nRet = TMaxFBGetF(PUB_DT				,	PST_FIELD(pRPacket,	pub_dt				));	 ///> 발행일자				
		nRet = TMaxFBGetF(PUB_TIME				,	PST_FIELD(pRPacket,	pub_time			));	 ///> 발행시각				
		nRet = TMaxFBGetF(PUB_SHCT_TRML_CD		,	PST_FIELD(pRPacket,	pub_shct_trml_cd	));	 ///> 발행단축터미널코드		
		nRet = TMaxFBGetF(PUB_WND_NO			,	PST_FIELD(pRPacket,	pub_wnd_no			));	 ///> 발행창구번호			

		/// 현장발권 mem
		::CopyMemory(&pTr->csrc_resp, pRPacket->csrc_auth_no, pRPacket->pub_num - pRPacket->csrc_auth_no);

		nRet = TMaxFBGetF(PUB_NUM ,	PST_FIELD(pRPacket,	pub_num	));	 ///> 발행매수
		pTr->n_csrc_pub_num = nCount = *(int *) pRPacket->pub_num;
		if(nCount <= 0)
		{
			nRet = -99;
			LOG_WRITE("%30s - Count ERROR !!!!", "PUB_NUM");
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}
		
		{
			pRPacket->pList = new rtk_pubtckcsrc_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_pubtckcsrc_list_t) * nCount);
		}

		pTr->m_vtPbTckCsrc.clear();
		for(i = 0; i < nCount; i++)
		{
			prtk_pubtckcsrc_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(PYN_MNS_DVS_CD		,	PST_FIELD(pList,	pyn_mns_dvs_cd		));	 ///> 지불수단구분코드		
			nRet = TMaxFBGetF(CSRC_APRV_NO			,	PST_FIELD(pList,	csrc_aprv_no		));	 ///> 현금영수증승인번호		
			nRet = TMaxFBGetF(BUS_TCK_KND_CD		,	PST_FIELD(pList,	bus_tck_knd_cd		));	 ///> 버스티켓종류코드		
			nRet = TMaxFBGetF(PUB_SNO				,	PST_FIELD(pList,	pub_sno				));	 ///> 발행일련번호			
			nRet = TMaxFBGetF(BUS_TCK_INHR_NO		,	PST_FIELD(pList,	bus_tck_inhr_no		));	 ///> 버스티켓고유번호		
			nRet = TMaxFBGetF(TISU_AMT				,	PST_FIELD(pList,	tisu_amt			));	 ///> 발권금액				
			nRet = TMaxFBGetF(SATS_NO				,	PST_FIELD(pList,	sats_no				));	 ///> 좌석번호				
			nRet = TMaxFBGetF(CTY_BUS_DC_KND_CD		,	PST_FIELD(pList,	cty_bus_dc_knd_cd	));	 ///> 시외버스할인종류코드	
			nRet = TMaxFBGetF(DCRT_DVS_CD			,	PST_FIELD(pList,	dcrt_dvs_cd			));	 ///> 할인율구분코드			
			nRet = TMaxFBGetF(DC_BEF_AMT			,	PST_FIELD(pList,	dc_bef_amt			));	 ///> 할인이전금액	

			pTr->m_vtPbTckCsrc.push_back(*pList);
		}
	}																								  
																									  
	///> Log out
	{
		LOG_WRITE("%30s - (%s) ", "RSP_CD",			PST_FIELD(pRPacket,	rsp_cd));	///> 
		LOG_WRITE("%30s - (%s) ", "RSP_MSG",			PST_FIELD(pRPacket,	rsp_msg));	///> 
		LOG_WRITE("%30s - (%s) ", "CSRC_AUTH_NO",		PST_FIELD(pRPacket,	csrc_auth_no));	///> 
		LOG_WRITE("%30s - (%s) ", "CSRC_BRN",			PST_FIELD(pRPacket,	csrc_brn));	///> 
		LOG_WRITE("%30s - (%s) ", "CSRC_APRV_VANC_CD",PST_FIELD(pRPacket,	csrc_aprv_vanc_cd));	///> 
		LOG_WRITE("%30s - (%s) ", "ATL_DEPR_DT",		PST_FIELD(pRPacket,	atl_depr_dt));	///> 
		LOG_WRITE("%30s - (%s) ", "ATL_DEPR_TIME",	PST_FIELD(pRPacket,	atl_depr_time));	///> 
		LOG_WRITE("%30s - (%s) ", "DEPR_TRML_CD",		PST_FIELD(pRPacket,	depr_trml_cd));	///> 
		LOG_WRITE("%30s - (%s) ", "ARVL_TRML_CD",		PST_FIELD(pRPacket,	arvl_trml_cd));	///> 
		LOG_WRITE("%30s - (%s) ", "STPT_TRML_CD",		PST_FIELD(pRPacket,	stpt_trml_cd));	///> 
		LOG_WRITE("%30s - (%s) ", "EPT_TRML_CD",		PST_FIELD(pRPacket,	ept_trml_cd));	///> 
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD",		PST_FIELD(pRPacket,	bus_cls_cd));	///> 
		LOG_WRITE("%30s - (%s) ", "BUS_CACM_CD",		PST_FIELD(pRPacket,	bus_cacm_cd));	///> 
		LOG_WRITE("%30s - (%s) ", "RDHM_MLTP_VAL",	PST_FIELD(pRPacket,	rdhm_mltp_val));	///> 
		LOG_WRITE("%30s - (number = %d) ", "DIST",	*(int *)pRPacket->dist);	///> 
		LOG_WRITE("%30s - (number = %d) ", "TAKE_DRTM",	*(int *)pRPacket->take_drtm);	///> 
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_PRIN_YN",		PST_FIELD(pRPacket,	bus_cls_prin_yn));	///> 
		LOG_WRITE("%30s - (%s) ", "BUS_CACM_NM_PRIN_YN",	PST_FIELD(pRPacket,	bus_cacm_nm_prin_yn));	///> 
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME_PRIN_YN",	PST_FIELD(pRPacket,	depr_time_prin_yn));	///> 
		LOG_WRITE("%30s - (%s) ", "SATS_NO_PRIN_YN",		PST_FIELD(pRPacket,	sats_no_prin_yn));	///> 
		LOG_WRITE("%30s - (%s) ", "ALCN_WAY_DVS_CD",		PST_FIELD(pRPacket,	alcn_way_dvs_cd));	///> 
		LOG_WRITE("%30s - (%s) ", "PERD_TEMP_YN",			PST_FIELD(pRPacket,	perd_temp_yn));	///> 
		LOG_WRITE("%30s - (%s) ", "PUB_DT",				PST_FIELD(pRPacket,	pub_dt));	///> 
		LOG_WRITE("%30s - (%s) ", "PUB_TIME",				PST_FIELD(pRPacket,	pub_time));	///> 
		LOG_WRITE("%30s - (%s) ", "PUB_SHCT_TRML_CD",		PST_FIELD(pRPacket,	pub_shct_trml_cd));	///> 
		LOG_WRITE("%30s - (%s) ", "PUB_WND_NO",			PST_FIELD(pRPacket,	pub_wnd_no));	///> 

		LOG_WRITE("%30s - (number = %d) ", "PUB_NUM",		*(int *)pRPacket->pub_num);	///> 
		nCount = *(int *)pRPacket->pub_num;

		for(i = 0; i < nCount; i++)
		{
			prtk_pubtckcsrc_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("## Index (%d) ", i);	///> 

			LOG_WRITE("%30s - (%s) ", "PYN_MNS_DVS_CD",		PST_FIELD(pList,	pyn_mns_dvs_cd));	///> 
			LOG_WRITE("%30s - (%s) ", "CSRC_APRV_NO",			PST_FIELD(pList,	csrc_aprv_no));	///> 
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD",		PST_FIELD(pList,	bus_tck_knd_cd));	///> 
			LOG_WRITE("%30s - (number = %d) ", "PUB_SNO",		*(int *)pList->pub_sno);	///> 
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_INHR_NO",		PST_FIELD(pList,	bus_tck_inhr_no));	///> 
			LOG_WRITE("%30s - (number = %d) ", "TISU_AMT",	*(int *)pList->tisu_amt);	///> 
			LOG_WRITE("%30s - (number = %d) ", "SATS_NO",		*(int *)pList->sats_no);	///> 
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_DC_KND_CD",	PST_FIELD(pList,	cty_bus_dc_knd_cd));	///> 
			LOG_WRITE("%30s - (%s) ", "DCRT_DVS_CD",			PST_FIELD(pList,	dcrt_dvs_cd));	///> 
			LOG_WRITE("%30s - (number = %d) ", "DC_BEF_AMT",	*(int *)pList->dc_bef_amt);	///> 
		}
	}

	nRet = nCount;

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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc135
 * @details		(135) 발행내역 조회(재발행용)
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc135(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ReadPubPtR";
	pstk_readpubptr_t	pSPacket;
	prtk_readpubptr_t	pRPacket;
	prtk_readpubptr_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readpubptr_t) pData;
	pRPacket = (prtk_readpubptr_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","INQR_DT",			PST_FIELD(pSPacket, inqr_dt));
		LOG_WRITE("%30s - (%s) ","INQR_DVS",			PST_FIELD(pSPacket, inqr_dvs));
		LOG_WRITE("%30s - (%s) ","INQR_WND_NO",		PST_FIELD(pSPacket, inqr_wnd_no));
		LOG_WRITE("%30s - (%s) ","USER_ID",			PST_FIELD(pSPacket, user_id));
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(INQR_DT			,	PST_FIELD(pSPacket, inqr_dt				));	///< 조회 대상 일자		
		nRet = TMaxFBPut(INQR_DVS			,	PST_FIELD(pSPacket, inqr_dvs			));	///< 조회 구분			
		nRet = TMaxFBPut(INQR_WND_NO		,	PST_FIELD(pSPacket, inqr_wnd_no			));	///< 조회 대상 창구번호	
		nRet = TMaxFBPut(USER_ID			,	PST_FIELD(pSPacket, user_id				));	///< 사용자ID			
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(PUB_NUM,	PST_FIELD(pRPacket, pub_num));
		nCount = *(int *)pRPacket->pub_num;
//		if( nCount <= 0 )
		if( nCount < 0 )
		{	// 에러코드 조회
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		if( nCount > 0 )
		{
			pRPacket->pList = new rtk_readpubptr_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readpubptr_list_t) * nCount);
		}

		OperGetFileName(OPER_FILE_ID_0135, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->pub_num, sizeof(pRPacket->pub_num));

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(PUB_DT			,	PST_FIELD(pList, pub_dt				));	///< 발행일자		
			nRet = TMaxFBGetF(PUB_SHCT_TRML_CD	,	PST_FIELD(pList, pub_shct_trml_cd	));	///< 발행단축터미널코드	
			nRet = TMaxFBGetF(PUB_WND_NO		,	PST_FIELD(pList, pub_wnd_no			));	///< 발행창구번호		
			nRet = TMaxFBGetF(PUB_SNO			,	PST_FIELD(pList, pub_sno			));	///< 발행일련번호		
			nRet = TMaxFBGetF(PUB_USER_ID		,	PST_FIELD(pList, pub_user_id		));	///< 발행사용자ID	
			nRet = TMaxFBGetF(DEPR_DT			,	PST_FIELD(pList, depr_dt			));	///< 출발일자		
			nRet = TMaxFBGetF(DEPR_TIME			,	PST_FIELD(pList, depr_time			));	///< 출발시각		
			nRet = TMaxFBGetF(PYN_MNS_DVS_CD	,	PST_FIELD(pList, pyn_mns_dvs_cd		));	///< 지불수단구분코드	
			nRet = TMaxFBGetF(TISU_CHNL_DVS_CD	,	PST_FIELD(pList, tisu_chnl_dvs_cd	));	///< 발권채널구분코드	
			nRet = TMaxFBGetF(PUB_CHNL_DVS_CD	,	PST_FIELD(pList, pub_chnl_dvs_cd	));	///< 발행채널구분코드	
			nRet = TMaxFBGetF(ROT_ID			,	PST_FIELD(pList, rot_id				));	///< 노선ID		
			nRet = TMaxFBGetF(ROT_NM			,	PST_FIELD(pList, rot_nm				));	///< 노선명			
			nRet = TMaxFBGetF(TISU_AMT			,	PST_FIELD(pList, tisu_amt			));	///< 발권금액		
			nRet = TMaxFBGetF(SATS_NO			,	PST_FIELD(pList, sats_no			));	///< 좌석번호		
			nRet = TMaxFBGetF(DEPR_TRML_CD		,	PST_FIELD(pList, depr_trml_cd		));	///< 출발터미널코드	
			nRet = TMaxFBGetF(ARVL_TRML_CD		,	PST_FIELD(pList, arvl_trml_cd		));	///< 도착터미널코드	
			nRet = TMaxFBGetF(BUS_CLS_CD		,	PST_FIELD(pList, bus_cls_cd			));	///< 버스등급코드		
			nRet = TMaxFBGetF(BUS_TCK_KND_CD	,	PST_FIELD(pList, bus_tck_knd_cd		));	///< 버스티켓종류코드	
			nRet = TMaxFBGetF(CANC_RY_DVS_CD	,	PST_FIELD(pList, canc_ry_dvs_cd		));	///< 취소환불구분코드	
			nRet = TMaxFBGetF(PUB_TIME			,	PST_FIELD(pList, pub_time			));	///< 발행시각		
			nRet = TMaxFBGetF(CTY_BUS_DC_KND_CD	,	PST_FIELD(pList, cty_bus_dc_knd_cd	));	///< 시외버스할인종류코드
			nRet = TMaxFBGetF(DCRT_DVS_CD		,	PST_FIELD(pList, dcrt_dvs_cd		));	///< 할인율구분코드	
			nRet = TMaxFBGetF(DC_BEF_AMT		,	PST_FIELD(pList, dc_bef_amt			));	///< 할인이전금액		
			nRet = TMaxFBGetF(PUB_SYS_DVS_CD	,	PST_FIELD(pList, pub_sys_dvs_cd		));	///< 발행시스템구분코드	
			nRet = TMaxFBGetF(EB_BUS_TCK_SNO	,	PST_FIELD(pList, eb_bus_tck_sno		));	///< EB버스티켓일련번호	
			nRet = TMaxFBGetF(QR_PYM_PYN_DTL_CD	,	PST_FIELD(pList, qr_pym_pyn_dtl_cd	)); ///< QR결제지불상세코드	// 20221208 ADD
			
#if 0 // TEST
			memset(pList->qr_pym_pyn_dtl_cd, 0x00, sizeof(pList->qr_pym_pyn_dtl_cd));
			strcpy(pList->qr_pym_pyn_dtl_cd, "PC");
#endif // TEST

			MyWriteFile(hFile, pList, sizeof(rtk_readpubptr_list_t));
		}
		MyCloseFile(hFile);
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ", "RSP_CD", PST_FIELD(pRPacket, rsp_cd ));
		LOG_WRITE("%30s - (%d) ", "PUB_NUM", nCount);

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ","PUB_DT",				PST_FIELD(pList, pub_dt				));	///< 발행일자		
			LOG_WRITE("%30s - (%s) ","PUB_SHCT_TRML_CD",	PST_FIELD(pList, pub_shct_trml_cd	));	///< 발행단축터미널코드	
			LOG_WRITE("%30s - (%s) ","PUB_WND_NO",			PST_FIELD(pList, pub_wnd_no			));	///< 발행창구번호		
			LOG_WRITE("%30s - (number = %d) ","PUB_SNO",	*(int *)pList->pub_sno				);	///< 발행일련번호		
			LOG_WRITE("%30s - (%s) ","PUB_USER_ID",			PST_FIELD(pList, pub_user_id		));	///< 발행사용자ID	
			LOG_WRITE("%30s - (%s) ","DEPR_DT",				PST_FIELD(pList, depr_dt			));	///< 출발일자		
			LOG_WRITE("%30s - (%s) ","DEPR_TIME",			PST_FIELD(pList, depr_time			));	///< 출발시각		
			LOG_WRITE("%30s - (%s) ","PYN_MNS_DVS_CD",		PST_FIELD(pList, pyn_mns_dvs_cd		));	///< 지불수단구분코드	
			LOG_WRITE("%30s - (%s) ","TISU_CHNL_DVS_CD",	PST_FIELD(pList, tisu_chnl_dvs_cd	));	///< 발권채널구분코드	
			LOG_WRITE("%30s - (%s) ","PUB_CHNL_DVS_CD",		PST_FIELD(pList, pub_chnl_dvs_cd	));	///< 발행채널구분코드	
			LOG_WRITE("%30s - (%s) ","ROT_ID",				PST_FIELD(pList, rot_id				));	///< 노선ID		
			LOG_WRITE("%30s - (%s) ","ROT_NM",				PST_FIELD(pList, rot_nm				));	///< 노선명			
			LOG_WRITE("%30s - (number = %d) ","TISU_AMT",	*(int *)pList->tisu_amt				);	///< 발권금액		
			LOG_WRITE("%30s - (number = %d) ","SATS_NO",	*(int *)pList->sats_no				);	///< 좌석번호		
			LOG_WRITE("%30s - (%s) ","DEPR_TRML_CD",		PST_FIELD(pList, depr_trml_cd		));	///< 출발터미널코드	
			LOG_WRITE("%30s - (%s) ","ARVL_TRML_CD",		PST_FIELD(pList, arvl_trml_cd		));	///< 도착터미널코드	
			LOG_WRITE("%30s - (%s) ","BUS_CLS_CD",			PST_FIELD(pList, bus_cls_cd			));	///< 버스등급코드		
			LOG_WRITE("%30s - (%s) ","BUS_TCK_KND_CD",		PST_FIELD(pList, bus_tck_knd_cd		));	///< 버스티켓종류코드	
			LOG_WRITE("%30s - (%s) ","CANC_RY_DVS_CD",		PST_FIELD(pList, canc_ry_dvs_cd		));	///< 취소환불구분코드	
			LOG_WRITE("%30s - (%s) ","PUB_TIME",			PST_FIELD(pList, pub_time			));	///< 발행시각		
			LOG_WRITE("%30s - (%s) ","CTY_BUS_DC_KND_CD",	PST_FIELD(pList, cty_bus_dc_knd_cd	));	///< 시외버스할인종류코드
			LOG_WRITE("%30s - (%s) ","DCRT_DVS_CD",			PST_FIELD(pList, dcrt_dvs_cd		));	///< 할인율구분코드	
			LOG_WRITE("%30s - (number = %d) ","DC_BEF_AMT", *(int *)pList->dc_bef_amt			);	///< 할인이전금액		
			LOG_WRITE("%30s - (%s) ","PUB_SYS_DVS_CD",		PST_FIELD(pList, pub_sys_dvs_cd		));	///< 발행시스템구분코드	
			LOG_WRITE("%30s - (%s) ","EB_BUS_TCK_SNO",		PST_FIELD(pList, eb_bus_tck_sno		));	///< EB버스티켓일련번호	
			LOG_WRITE("%30s - (%s) ", "QR_PYM_PYN_DTL_CD",	PST_FIELD(pList, qr_pym_pyn_dtl_cd	));	///< QR결제지불상세코드	// 20221205 ADD			
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc136
 * @details		(136) 버스티켓 재발행
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc136(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_RPubTck";
	pstk_rpubtck_t	pSPacket;
	prtk_rpubtck_t	pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_rpubtck_t) pData;
	pRPacket = (prtk_rpubtck_t) retBuf;

	nRet = TMaxConnect();
	if(nRet < 0)
	{
		nRet = -1;
		LOG_WRITE("TMaxConnect(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		CConfigTkMem::GetInstance()->SetErrorCode("SVE995");
		goto fail_proc;
	}

	nRet = TMaxAlloc();
	if(nRet < 0)
	{
		nRet = -2;
		LOG_WRITE("TMaxAlloc(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		CConfigTkMem::GetInstance()->SetErrorCode("SVE996");
		goto fail_proc;
	}

	// send log
	{
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","PUB_DT",			PST_FIELD(pSPacket, pub_dt));
		LOG_WRITE("%30s - (%s) ","PUB_SHCT_TRML_CD",	PST_FIELD(pSPacket, pub_shct_trml_cd));
		LOG_WRITE("%30s - (%s) ","PUB_WND_NO",		PST_FIELD(pSPacket, pub_wnd_no));
		LOG_WRITE("%30s - (number = %d) ","PUB_SNO",	*(int *)pSPacket->pub_sno);
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(PUB_DT					,	PST_FIELD(pSPacket, pub_dt				));	///< 발행일자			
		nRet = TMaxFBPut(PUB_SHCT_TRML_CD		,	PST_FIELD(pSPacket, pub_shct_trml_cd	));	///< 발행단축터미널코드		
		nRet = TMaxFBPut(PUB_WND_NO				,	PST_FIELD(pSPacket, pub_wnd_no			));	///< 발행창구번호			
		nRet = TMaxFBPut(PUB_SNO				,	PST_FIELD(pSPacket, pub_sno				));	///< 발행일련번호			
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
//		CConfigTkMem::GetInstance()->SetErrorCode("SVE997");
		goto fail_proc;
	}

	LOG_WRITE("TMaxTpCall(), Success !!!!");
	
	CPubTckMem::GetInstance()->m_vtPbTckReIssue.clear();
	///> GetData
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 505 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
		
		nRet = TMaxFBGetF(PYN_MNS_DVS_CD			,	PST_FIELD(pRPacket,	pyn_mns_dvs_cd		)); ///< 지불수단구분코드		
		nRet = TMaxFBGetF(CARD_NO					,	PST_FIELD(pRPacket,	card_no				)); ///< 카드번호			
		nRet = TMaxFBGetF(CARD_APRV_NO				,	PST_FIELD(pRPacket,	card_aprv_no		)); ///< 카드승인번호			
		nRet = TMaxFBGetF(CARD_APRV_AMT				,	PST_FIELD(pRPacket,	card_aprv_amt		)); ///< 카드승인금액			
		nRet = TMaxFBGetF(CARD_APRV_VANC_CD			,	PST_FIELD(pRPacket,	card_aprv_vanc_cd	)); ///< 카드승인VAN사코드		
		nRet = TMaxFBGetF(CSRC_APRV_VANC_CD			,	PST_FIELD(pRPacket,	csrc_aprv_vanc_cd	)); ///< 현금영수증승인VAN사코드	
		nRet = TMaxFBGetF(CSRC_APRV_NO				,	PST_FIELD(pRPacket,	csrc_aprv_no		)); ///< 현금영수증승인번호		
		nRet = TMaxFBGetF(CSRC_AUTH_NO				,	PST_FIELD(pRPacket,	csrc_auth_no		)); ///< 현금영수증인증번호		
		nRet = TMaxFBGetF(CSRC_BRN					,	PST_FIELD(pRPacket,	csrc_brn			)); ///< 현금영수증사업자등록번호	
		nRet = TMaxFBGetF(CPRT_YN					,	PST_FIELD(pRPacket,	cprt_yn				)); ///< 법인여부			
		nRet = TMaxFBGetF(ISSU_CRCM_CD				,	PST_FIELD(pRPacket,	issu_crcm_cd		)); ///< 발급카드사코드		
		nRet = TMaxFBGetF(BUY_CRCM_CD				,	PST_FIELD(pRPacket,	buy_crcm_cd			)); ///< 매입카드사코드		
		nRet = TMaxFBGetF(FRC_CMRT					,	PST_FIELD(pRPacket,	frc_cmrt			)); ///< 가맹점수수료율		
		// 20211015 ADD
		nRet = TMaxFBGetF(EVCP_PYM_AMT				,	PST_FIELD(pRPacket,	evcp_pym_amt		)); ///< 이벤트쿠폰결제금액				
		nRet = TMaxFBGetF(MLG_PYM_AMT				,	PST_FIELD(pRPacket,	mlg_pym_amt			)); ///< 마일리지결제금액		
		// 20211015 ~ADD
		nRet = TMaxFBGetF(ATL_DEPR_DT				,	PST_FIELD(pRPacket,	atl_depr_dt			)); ///< 실제출발일자			
		nRet = TMaxFBGetF(ATL_DEPR_TIME				,	PST_FIELD(pRPacket,	atl_depr_time		)); ///< 실제출발시각			
		nRet = TMaxFBGetF(DEPR_TRML_CD				,	PST_FIELD(pRPacket,	depr_trml_cd		)); ///< 출발터미널코드		
		nRet = TMaxFBGetF(ARVL_TRML_CD				,	PST_FIELD(pRPacket,	arvl_trml_cd		)); ///< 도착터미널코드		
		nRet = TMaxFBGetF(STPT_TRML_CD				,	PST_FIELD(pRPacket,	stpt_trml_cd		)); ///< 기점터미널코드		
		nRet = TMaxFBGetF(EPT_TRML_CD				,	PST_FIELD(pRPacket,	ept_trml_cd			)); ///< 종점터미널코드		
		nRet = TMaxFBGetF(BUS_CLS_CD				,	PST_FIELD(pRPacket,	bus_cls_cd			)); ///< 버스등급코드			
		nRet = TMaxFBGetF(BUS_CACM_CD				,	PST_FIELD(pRPacket,	bus_cacm_cd			)); ///< 버스운수사코드		
		nRet = TMaxFBGetF(RDHM_MLTP_VAL				,	PST_FIELD(pRPacket,	rdhm_mltp_val		)); ///< 승차홈다중값			
		nRet = TMaxFBGetF(DIST						,	PST_FIELD(pRPacket,	dist				)); ///< 거리				
		nRet = TMaxFBGetF(TAKE_DRTM					,	PST_FIELD(pRPacket,	take_drtm			)); ///< 소요시간			
		nRet = TMaxFBGetF(BUS_CLS_PRIN_YN			,	PST_FIELD(pRPacket,	bus_cls_prin_yn		)); ///< 버스등급출력여부		
		nRet = TMaxFBGetF(BUS_CACM_NM_PRIN_YN		,	PST_FIELD(pRPacket,	bus_cacm_nm_prin_yn	)); ///< 버스운수사명출력여부	
		nRet = TMaxFBGetF(DEPR_TIME_PRIN_YN			,	PST_FIELD(pRPacket,	depr_time_prin_yn	)); ///< 출발시각출력여부		
		nRet = TMaxFBGetF(SATS_NO_PRIN_YN			,	PST_FIELD(pRPacket,	sats_no_prin_yn		)); ///< 좌석번호출력여부		
		nRet = TMaxFBGetF(ALCN_WAY_DVS_CD			,	PST_FIELD(pRPacket,	alcn_way_dvs_cd		)); ///< 배차방식구분코드		
		nRet = TMaxFBGetF(SATI_USE_YN				,	PST_FIELD(pRPacket,	sati_use_yn			)); ///< 좌석제사용여부		
		nRet = TMaxFBGetF(PERD_TEMP_YN				,	PST_FIELD(pRPacket,	perd_temp_yn		)); ///< 정기임시여부			
		nRet = TMaxFBGetF(PUB_DT					,	PST_FIELD(pRPacket,	pub_dt				)); ///< 발행일자			
		nRet = TMaxFBGetF(PUB_TIME					,	PST_FIELD(pRPacket,	pub_time			)); ///< 발행시각			
		nRet = TMaxFBGetF(PUB_SHCT_TRML_CD			,	PST_FIELD(pRPacket,	pub_shct_trml_cd	)); ///< 발행단축터미널코드		
		nRet = TMaxFBGetF(PUB_WND_NO				,	PST_FIELD(pRPacket,	pub_wnd_no			)); ///< 발행창구번호			
		
		nRet = TMaxFBGetF(BUS_TCK_KND_CD			,	PST_FIELD(pRPacket,	bus_tck_knd_cd		)); ///< 버스티켓종류코드		
		nRet = TMaxFBGetF(PUB_SNO					,	PST_FIELD(pRPacket,	pub_sno				)); ///< 발행일련번호			
		nRet = TMaxFBGetF(BUS_TCK_INHR_NO			,	PST_FIELD(pRPacket,	bus_tck_inhr_no		)); ///< 버스티켓고유번호		
		nRet = TMaxFBGetF(TISU_AMT					,	PST_FIELD(pRPacket,	tisu_amt			)); ///< 발권금액			
		nRet = TMaxFBGetF(SATS_NO					,	PST_FIELD(pRPacket,	sats_no				)); ///< 좌석번호			
		nRet = TMaxFBGetF(CTY_BUS_DC_KND_CD			,	PST_FIELD(pRPacket,	cty_bus_dc_knd_cd	)); ///< 시외버스할인종류코드	
		nRet = TMaxFBGetF(DCRT_DVS_CD				,	PST_FIELD(pRPacket,	dcrt_dvs_cd			)); ///< 할인율구분코드		
		nRet = TMaxFBGetF(DC_BEF_AMT				,	PST_FIELD(pRPacket,	dc_bef_amt			)); ///< 할인이전금액			
		nRet = TMaxFBGetF(CTY_BUS_OPRN_SHP_CD		,	PST_FIELD(pRPacket,	cty_bus_oprn_shp_cd	)); ///< 시외버스운행형태코드	
		nRet = TMaxFBGetF(USER_DVS_NO				,	PST_FIELD(pRPacket,	user_dvs_no			)); ///< 사용자구분번호		
		nRet = TMaxFBGetF(USER_NM					,	PST_FIELD(pRPacket,	user_nm				)); ///< 사용자명			
		nRet = TMaxFBGetF(CMPY_NM					,	PST_FIELD(pRPacket,	cmpy_nm				)); ///< 회사명				
		//nRet = TMaxFBGetF(MIP_TERM					,	PST_FIELD(pRPacket,	mip_term			)); ///< 할부기간			// 20221205 MOD
		//nRet = TMaxFBGetF(WHCH_SATS_NO_PRIN_NM		,	PST_FIELD(pRPacket,	whch_sats_no_prin_nm)); ///< 휠체어좌석번호출력명	// 20221205 ADD	
		nRet = TMaxFBGetF(QR_PYM_PYN_DTL_CD			,	PST_FIELD(pRPacket,	qr_pym_pyn_dtl_cd	)); ///< QR결제지불상세코드	// 20221205 ADD

		{
			rtk_rpubtck_t rData;

			::CopyMemory(&rData, pRPacket, sizeof(rtk_rpubtck_t));
			
			CPubTckMem::GetInstance()->m_vtPbTckReIssue.push_back(rData);
		}
	}

	///> Log out
	{
		LOG_WRITE("%30s - (%s) ", "RSP_CD",					PST_FIELD(pRPacket, rsp_cd				));	///< 응답코드			
		LOG_WRITE("%30s - (%s) ", "PYN_MNS_DVS_CD",			PST_FIELD(pRPacket, pyn_mns_dvs_cd		));	///< 지불수단구분코드		
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ", "CARD_NO",				PST_FIELD(pRPacket, card_no				));	///< 카드번호			
#endif
		LOG_WRITE("%30s - (%s) ", "CARD_APRV_NO",			PST_FIELD(pRPacket, card_aprv_no		));	///< 카드승인번호			
		LOG_WRITE("%30s - (number = %d) ", "CARD_APRV_AMT",*(int *) pRPacket->card_aprv_amt		);	///< 카드승인금액			
		LOG_WRITE("%30s - (%s) ", "CARD_APRV_VANC_CD",		PST_FIELD(pRPacket, card_aprv_vanc_cd	));	///< 카드승인VAN사코드		
		LOG_WRITE("%30s - (%s) ", "CSRC_APRV_VANC_CD",		PST_FIELD(pRPacket, csrc_aprv_vanc_cd	));	///< 현금영수증승인VAN사코드
		LOG_WRITE("%30s - (%s) ", "CSRC_APRV_NO",			PST_FIELD(pRPacket, csrc_aprv_no		));	///< 현금영수증승인번호		
		LOG_WRITE("%30s - (%s) ", "CSRC_AUTH_NO",			PST_FIELD(pRPacket, csrc_auth_no		));	///< 현금영수증인증번호		
		LOG_WRITE("%30s - (%s) ", "CSRC_BRN",				PST_FIELD(pRPacket, csrc_brn			));	///< 현금영수증사업자등록번호
		LOG_WRITE("%30s - (%s) ", "CPRT_YN",				PST_FIELD(pRPacket, cprt_yn				));	///< 법인여부			
		LOG_WRITE("%30s - (%s) ", "ISSU_CRCM_CD",			PST_FIELD(pRPacket, issu_crcm_cd		));	///< 발급카드사코드		
		LOG_WRITE("%30s - (%s) ", "BUY_CRCM_CD",			PST_FIELD(pRPacket, buy_crcm_cd			));	///< 매입카드사코드		
		LOG_WRITE("%30s - (number = %d) ", "FRC_CMRT",		*(int *) pRPacket->frc_cmrt				);	///< 가맹점수수료율		
		// 20211015 ADD
		LOG_WRITE("%30s - (number = %d) ", "EVCP_PYM_AMT",	*(int *) pRPacket->evcp_pym_amt			);	///< 이벤트쿠폰결제금액				
		LOG_WRITE("%30s - (number = %d) ", "MLG_PYM_AMT",	*(int *) pRPacket->mlg_pym_amt			);	///< 마일리지결제금액				
		// 20211015 ~ADD
		LOG_WRITE("%30s - (%s) ", "ATL_DEPR_DT",			PST_FIELD(pRPacket, atl_depr_dt			));	///< 실제출발일자			
		LOG_WRITE("%30s - (%s) ", "ATL_DEPR_TIME",			PST_FIELD(pRPacket, atl_depr_time		));	///< 실제출발시각			
		LOG_WRITE("%30s - (%s) ", "DEPR_TRML_CD",			PST_FIELD(pRPacket, depr_trml_cd		));	///< 출발터미널코드		
		LOG_WRITE("%30s - (%s) ", "ARVL_TRML_CD",			PST_FIELD(pRPacket, arvl_trml_cd		));	///< 도착터미널코드		
		LOG_WRITE("%30s - (%s) ", "STPT_TRML_CD",			PST_FIELD(pRPacket, stpt_trml_cd		));	///< 기점터미널코드		
		LOG_WRITE("%30s - (%s) ", "EPT_TRML_CD",			PST_FIELD(pRPacket, ept_trml_cd			));	///< 종점터미널코드		
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD",				PST_FIELD(pRPacket, bus_cls_cd			));	///< 버스등급코드			
		LOG_WRITE("%30s - (%s) ", "BUS_CACM_CD",			PST_FIELD(pRPacket, bus_cacm_cd			));	///< 버스운수사코드		
		LOG_WRITE("%30s - (%s) ", "RDHM_MLTP_VAL",			PST_FIELD(pRPacket, rdhm_mltp_val		));	///< 승차홈다중값			
		LOG_WRITE("%30s - (number = %d) ", "DIST",			*(int *) pRPacket->dist					);	///< 거리				
		LOG_WRITE("%30s - (number = %d) ", "TAKE_DRTM",		*(int *) pRPacket->take_drtm			);	///< 소요시간			
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_PRIN_YN",		PST_FIELD(pRPacket, bus_cls_prin_yn		));	///< 버스등급출력여부		
		LOG_WRITE("%30s - (%s) ", "BUS_CACM_NM_PRIN_YN",	PST_FIELD(pRPacket, bus_cacm_nm_prin_yn	));	///< 버스운수사명출력여부	
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME_PRIN_YN",		PST_FIELD(pRPacket, depr_time_prin_yn	));	///< 출발시각출력여부		
		LOG_WRITE("%30s - (%s) ", "SATS_NO_PRIN_YN",		PST_FIELD(pRPacket, sats_no_prin_yn		));	///< 좌석번호출력여부		
		LOG_WRITE("%30s - (%s) ", "ALCN_WAY_DVS_CD",		PST_FIELD(pRPacket, alcn_way_dvs_cd		));	///< 배차방식구분코드		
		LOG_WRITE("%30s - (%s) ", "SATI_USE_YN",			PST_FIELD(pRPacket, sati_use_yn			));	///< 좌석제사용여부		
		LOG_WRITE("%30s - (%s) ", "PERD_TEMP_YN",			PST_FIELD(pRPacket, perd_temp_yn		));	///< 정기임시여부			
		LOG_WRITE("%30s - (%s) ", "PUB_DT",					PST_FIELD(pRPacket, pub_dt				));	///< 발행일자			
		LOG_WRITE("%30s - (%s) ", "PUB_TIME",				PST_FIELD(pRPacket, pub_time			));	///< 발행시각			
		LOG_WRITE("%30s - (%s) ", "PUB_SHCT_TRML_CD",		PST_FIELD(pRPacket, pub_shct_trml_cd	));	///< 발행단축터미널코드		
		LOG_WRITE("%30s - (%s) ", "PUB_WND_NO",				PST_FIELD(pRPacket, pub_wnd_no			));	///< 발행창구번호			
		LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD",			PST_FIELD(pRPacket, bus_tck_knd_cd		));	///< 버스티켓종류코드		
		LOG_WRITE("%30s - (number = %d) ", "PUB_SNO",		*(int *) pRPacket->pub_sno				);	///< 발행일련번호			
		LOG_WRITE("%30s - (%s) ", "BUS_TCK_INHR_NO",		PST_FIELD(pRPacket, bus_tck_inhr_no		));	///< 버스티켓고유번호		
		LOG_WRITE("%30s - (number = %d) ", "TISU_AMT",		*(int *) pRPacket->tisu_amt				);	///< 발권금액			
		LOG_WRITE("%30s - (number = %d) ", "SATS_NO",		*(int *) pRPacket->sats_no				);	///< 좌석번호			
		LOG_WRITE("%30s - (%s) ", "CTY_BUS_DC_KND_CD",		PST_FIELD(pRPacket, cty_bus_dc_knd_cd	));	///< 시외버스할인종류코드	
		LOG_WRITE("%30s - (%s) ", "DCRT_DVS_CD",			PST_FIELD(pRPacket, dcrt_dvs_cd			));	///< 할인율구분코드		
		LOG_WRITE("%30s - (number = %d) ", "DC_BEF_AMT",	*(int *) pRPacket->dc_bef_amt			);	///< 할인이전금액			
		LOG_WRITE("%30s - (%s) ", "CTY_BUS_OPRN_SHP_CD",	PST_FIELD(pRPacket, cty_bus_oprn_shp_cd	));	///< 시외버스운행형태코드	
		LOG_WRITE("%30s - (%s) ", "USER_DVS_NO",			PST_FIELD(pRPacket, user_dvs_no			));	///< 사용자구분번호		
		LOG_WRITE("%30s - (%s) ", "USER_NM",				PST_FIELD(pRPacket, user_nm				));	///< 사용자명			
		LOG_WRITE("%30s - (%s) ", "CMPY_NM",				PST_FIELD(pRPacket, cmpy_nm				));	///< 회사명				
		//LOG_WRITE("%30s - (%s) ", "MIP_TERM",				PST_FIELD(pRPacket, mip_term			));	///< 할부기간			
		//LOG_WRITE("%30s - (%s) ", "WHCH_SATS_NO_PRIN_NM",	PST_FIELD(pRPacket, whch_sats_no_prin_nm));	///< 휠체어좌석번호출력명	// 20221205 ADD			
		LOG_WRITE("%30s - (%s) ", "QR_PYM_PYN_DTL_CD",		PST_FIELD(pRPacket, qr_pym_pyn_dtl_cd	));	///< QR결제지불상세코드	// 20221205 ADD			
	}

	nRet = 1;

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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc137
 * @details		(137) 홈티켓/인터넷/모바일 예매 조회 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc137(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int nRealCount = 0;
	int n_rsp_cd;
	char*	pService = "TK_ReadMrnpPt";
	pstk_readmrnppt_t	pSPacket;
	prtk_readmrnppt_t		pRPacket;
	prtk_readmrnppt_list_t	pList;
	POPER_FILE_CONFIG_T		pConfig;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readmrnppt_t) pData;
	pRPacket = (prtk_readmrnppt_t) retBuf;

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","REQ_READ_DVS_CD",	PST_FIELD(pSPacket, req_read_dvs_cd	));
		LOG_WRITE("%30s - (%s) ","READ_WND_DVS",		PST_FIELD(pSPacket, read_wnd_dvs	));
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ","CARD_NO",			PST_FIELD(pSPacket, card_no			));
#endif
		LOG_WRITE("%30s - (%s) ","POS_PG_KTC_VER",	PST_FIELD(pSPacket, pos_pg_ktc_ver	));
		LOG_WRITE("%30s - (%s) ","ENC_DTA_LEN",		PST_FIELD(pSPacket, enc_dta_len		));
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ","ENC_DTA",			PST_FIELD(pSPacket, enc_dta			));
#endif
		LOG_WRITE("%30s - (%s) ","MNPP_TEL_NO",		PST_FIELD(pSPacket, mnpp_tel_no		));
		LOG_WRITE("%30s - (%s) ","MNPP_BRDT",			PST_FIELD(pSPacket, mnpp_brdt		));
		LOG_WRITE("%30s - (%s) ","MRS_NO",			PST_FIELD(pSPacket, mrs_no			));
	}

	// send data
	{
		int nEncLen;
		int rndLen, nRet, nOffset;
		char rndString[100];
		char retString[100];

		rndLen = nRet = nOffset = 0;

		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(REQ_READ_DVS_CD	,	PST_FIELD(pSPacket, req_read_dvs_cd		));	///< 요청 조회 구분		
		nRet = TMaxFBPut(READ_WND_DVS		,	PST_FIELD(pSPacket, read_wnd_dvs		));	///< 조회 창구 구분		
		nRet = TMaxFBPut(CARD_NO			,	PST_FIELD(pSPacket, card_no				));	///< 카드번호			
		nRet = TMaxFBPut(POS_PG_KTC_VER		,	PST_FIELD(pSPacket, pos_pg_ktc_ver		));	///< POS단말기버전정보		

		if(pSPacket->req_read_dvs_cd[0] == '5')
		{
//			nRet = TMaxFBPut(ENC_DTA_LEN		,	PST_FIELD(pSPacket, enc_dta_len			));	///< ENC 데이터 길이	
			nRet = TMaxFBPutLen(ENC_DTA_LEN	,	PST_FIELD(pSPacket, enc_dta_len		), 4);
			nEncLen = *(int *) pSPacket->enc_dta_len;

			/// enc data
			for(i = 0; i < nEncLen; i++)
			{
				nOffset = 0;
				::ZeroMemory(rndString, sizeof(rndString));
				::ZeroMemory(retString, sizeof(retString));

				srand((unsigned int) i);
				rndLen = rand() % 5 + 2;
				nRet = GetRndChar(rndLen, rndString);

				retString[nOffset++] = rndString[0];
				retString[nOffset++] = pSPacket->enc_dta[i];
				::CopyMemory(&retString[nOffset], &rndString[1], rndLen - 1);

				nRet = TMaxFBInsert(ENC_DTA, retString, i, nOffset);

				::ZeroMemory(rndString, sizeof(rndString));
				::ZeroMemory(retString, sizeof(retString));
			}

#if (_KTC_CERTIFY_ > 0)
			KTC_MemClear(pSPacket->enc_dta, sizeof(pSPacket->enc_dta));
#endif
		}

		nRet = TMaxFBPut(MNPP_TEL_NO		,	PST_FIELD(pSPacket, mnpp_tel_no			));	///< 예약자전화번호		
		nRet = TMaxFBPut(MNPP_BRDT			,	PST_FIELD(pSPacket, mnpp_brdt			));	///< 예약자생년월일		
		nRet = TMaxFBPut(MRS_NO				,	PST_FIELD(pSPacket, mrs_no				));	///< 예매번호			

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(pSPacket->enc_dta, sizeof(pSPacket->mnpp_tel_no));
		KTC_MemClear(pSPacket->mnpp_brdt, sizeof(pSPacket->mnpp_brdt));
		KTC_MemClear(pSPacket->mrs_no, sizeof(pSPacket->mrs_no));
#endif
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		CMrnpMem* pTr;

		// 예매발권 mem
		pTr = CMrnpMem::GetInstance();

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			goto fail_proc;
		}

		nRet = TMaxFBGetF(MRS_NUM		,	PST_FIELD(pRPacket, mrs_num));
		pTr->n_mrs_num = nCount = *(int *)pRPacket->mrs_num;
		if( nCount <= 0 )
		{	// 갯수 에러
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE888");
			goto fail_proc;
		}
		
		{
			pRPacket->pList = new rtk_readmrnppt_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readmrnppt_list_t) * nCount);
		}

		pTr->m_vtMrnpList.clear();
		for(i = 0; i < nCount; i++)
		{
			DWORD dwCurr, dwDepr;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(MRS_NO			,	PST_FIELD(pList, mrs_no				));	///< 예매번호			
			nRet = TMaxFBGetF(TISU_ID			,	PST_FIELD(pList, tisu_id			));	///< 발권ID			
			nRet = TMaxFBGetF(TISU_DT			,	PST_FIELD(pList, tisu_dt			));	///< 발권일자			
			nRet = TMaxFBGetF(TISU_TIME			,	PST_FIELD(pList, tisu_time			));	///< 발권시각			
			nRet = TMaxFBGetF(TISU_CHNL_DVS_CD	,	PST_FIELD(pList, tisu_chnl_dvs_cd	));	///< 발권채널구분코드 (Web:W, Mobile:M)		
			nRet = TMaxFBGetF(PUB_CHNL_DVS_CD	,	PST_FIELD(pList, pub_chnl_dvs_cd	));	///< 발행채널구분코드		
			nRet = TMaxFBGetF(PUB_DT			,	PST_FIELD(pList, pub_dt				));	///< 발행일자			
			nRet = TMaxFBGetF(PUB_TIME			,	PST_FIELD(pList, pub_time			));	///< 발행시각			
			nRet = TMaxFBGetF(TISU_AMT			,	PST_FIELD(pList, tisu_amt			));	///< 발권금액			
			nRet = TMaxFBGetF(PYM_APRV_ID		,	PST_FIELD(pList, pym_aprv_id		));	///< 결제승인ID			
			nRet = TMaxFBGetF(DEPR_TRML_CD		,	PST_FIELD(pList, depr_trml_cd		));	///< 출발터미널코드		
			nRet = TMaxFBGetF(ARVL_TRML_CD		,	PST_FIELD(pList, arvl_trml_cd		));	///< 도착터미널코드		
			nRet = TMaxFBGetF(ROT_ID			,	PST_FIELD(pList, rot_id				));	///< 노선ID			
			nRet = TMaxFBGetF(ROT_SQNO			,	PST_FIELD(pList, rot_sqno			));	///< 노선순번			
			nRet = TMaxFBGetF(ROT_NM			,	PST_FIELD(pList, rot_nm				));	///< 노선명				
			nRet = TMaxFBGetF(DEPR_DT			,	PST_FIELD(pList, depr_dt			));	///< 출발일자			
			nRet = TMaxFBGetF(DEPR_TIME			,	PST_FIELD(pList, depr_time			));	///< 출발시각			
			nRet = TMaxFBGetF(BUS_CLS_CD		,	PST_FIELD(pList, bus_cls_cd			));	///< 버스등급코드			
			nRet = TMaxFBGetF(BUS_TCK_KND_CD	,	PST_FIELD(pList, bus_tck_knd_cd		));	///< 버스티켓종류코드		
			nRet = TMaxFBGetF(SATS_NO			,	PST_FIELD(pList, sats_no			));	///< 좌석번호			
			nRet = TMaxFBGetF(CARD_NO			,	PST_FIELD(pList, card_no			));	///< 카드번호			
			nRet = TMaxFBGetF(CTY_BUS_SYS_DVS_CD,	PST_FIELD(pList, cty_bus_sys_dvs_cd	));	///< 시외버스시스템구분코드	
			nRet = TMaxFBGetF(TISU_SYS_DVS_CD	,	PST_FIELD(pList, tisu_sys_dvs_cd	));	///< 발권시스템구분코드		
			nRet = TMaxFBGetF(CTY_BUS_DC_KND_CD	,	PST_FIELD(pList, cty_bus_dc_knd_cd	));	///< 시외버스할인종류코드	
			nRet = TMaxFBGetF(DCRT_DVS_CD		,	PST_FIELD(pList, dcrt_dvs_cd		));	///< 할인율구분코드		
			nRet = TMaxFBGetF(DC_BEF_AMT		,	PST_FIELD(pList, dc_bef_amt			));	///< 할인이전금액			
			nRet = TMaxFBGetF(ALCN_DT			,	PST_FIELD(pList, alcn_dt			));	///< 배차일자			
			nRet = TMaxFBGetF(ALCN_SQNO			,	PST_FIELD(pList, alcn_sqno			));	///< 배차순번			
			nRet = TMaxFBGetF(RPUB_NUM			,	PST_FIELD(pList, rpub_num			));	///< 재발행매수			

			// 현재일시
			dwCurr = Util_GetCurrentTick();
			
			// 출발일시
			dwDepr = Util_TickFromString(pList->depr_dt, pList->depr_time);
			
			if(pConfig->base_t.prev_mrnp_inq_yn[0] == 'Y')
			{	/// 과거 예매내역 발권 사용유무
				if( dwCurr > dwDepr )
				{
					if( (dwCurr - dwDepr) > (3600 * 24) )
					{
						LOG_WRITE("예매내역 skip, i = %d, DEPR(%s %s)", i, pList->depr_dt, pList->depr_time);
						continue;
					}
				}
			}

			nRealCount++;
			// 예매발권 리스트 add
			pTr->m_vtMrnpList.push_back(*pList);

#if (_KTC_CERTIFY_ > 0)
			KTC_MemClear(pList->card_no, sizeof(pList->card_no));
#endif
		}											
	}

//	if(nCount > 0)
	if(nRealCount > 0)
	{
		std::sort(CMrnpMem::GetInstance()->m_vtMrnpList.begin(), CMrnpMem::GetInstance()->m_vtMrnpList.end(), cmpMrnpList);
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		//LOG_WRITE("%30s - (%d) ","MRS_NUM",	nCount);
		LOG_WRITE("%30s - (%d) ","MRS_NUM",	nRealCount);

		//for(i = 0; i < nCount; i++)
		for(i = 0; i < nRealCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "MRS_NO",				PST_FIELD(pList, mrs_no					));
			LOG_WRITE("%30s - (%s) ", "TISU_ID",			PST_FIELD(pList, tisu_id				));
			LOG_WRITE("%30s - (%s) ", "TISU_DT",			PST_FIELD(pList, tisu_dt				));
			LOG_WRITE("%30s - (%s) ", "TISU_TIME",			PST_FIELD(pList, tisu_time				));
			LOG_WRITE("%30s - (%s) ", "TISU_CHNL_DVS_CD",	PST_FIELD(pList, tisu_chnl_dvs_cd		));
			LOG_WRITE("%30s - (%s) ", "PUB_CHNL_DVS_CD",	PST_FIELD(pList, pub_chnl_dvs_cd		));
			LOG_WRITE("%30s - (%s) ", "PUB_DT",				PST_FIELD(pList, pub_dt					));
			LOG_WRITE("%30s - (%s) ", "PUB_TIME",			PST_FIELD(pList, pub_time				));
			LOG_WRITE("%30s - (number = %d) ", "TISU_AMT", *(int *)pList->tisu_amt );
			LOG_WRITE("%30s - (%s) ", "PYM_APRV_ID",		PST_FIELD(pList, pym_aprv_id			));
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_CD",		PST_FIELD(pList, depr_trml_cd			));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_CD",		PST_FIELD(pList, arvl_trml_cd			));
			LOG_WRITE("%30s - (%s) ", "ROT_ID",				PST_FIELD(pList, rot_id					));
			LOG_WRITE("%30s - (number = %d) ", "ROT_SQNO", *(int *)pList->rot_sqno				);
			LOG_WRITE("%30s - (%s) ", "ROT_NM",				PST_FIELD(pList, rot_nm					));
			LOG_WRITE("%30s - (%s) ", "DEPR_DT",			PST_FIELD(pList, depr_dt				));
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME",			PST_FIELD(pList, depr_time				));
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD",			PST_FIELD(pList, bus_cls_cd				));
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD",		PST_FIELD(pList, bus_tck_knd_cd			));
			LOG_WRITE("%30s - (number = %d) ", "SATS_NO",	*(int *)pList->sats_no);
#if (_KTC_CERTIFY_ <= 0)
			LOG_WRITE("%30s - (%s) ", "CARD_NO",			PST_FIELD(pList, card_no				));
#endif
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_SYS_DVS_CD", PST_FIELD(pList, cty_bus_sys_dvs_cd		));
			LOG_WRITE("%30s - (%s) ", "TISU_SYS_DVS_CD",	PST_FIELD(pList, tisu_sys_dvs_cd		));
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_DC_KND_CD",	PST_FIELD(pList, cty_bus_dc_knd_cd		));
			LOG_WRITE("%30s - (%s) ", "DCRT_DVS_CD",		PST_FIELD(pList, dcrt_dvs_cd			));
			LOG_WRITE("%30s - (number = %d) ", "DC_BEF_AMT",*(int *)pList->dc_bef_amt);
			LOG_WRITE("%30s - (%s) ", "ALCN_DT",			PST_FIELD(pList, alcn_dt				));
			LOG_WRITE("%30s - (number = %d) ", "ALCN_SQNO", *(int *)pList->alcn_sqno);
			LOG_WRITE("%30s - (number = %d) ", "RPUB_NUM",	*(int *)pList->rpub_num);
		}
	}

//	nRet = nCount;
	nRet = nRealCount;

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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc138
 * @details		(138) 홈티켓/인터넷/모바일 예매 발행
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc138(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd = 0;
	char*	pService = "TK_PubMrnp";
	pstk_pubmrnp_t	pSPacket;
	prtk_pubmrnp_t	pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_pubmrnp_t) pData;
	pRPacket = (prtk_pubmrnp_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","PUB_CHNL_DVS_CD",	PST_FIELD(pSPacket, pub_chnl_dvs_cd));
		LOG_WRITE("%30s - (%s) ","MRS_NO",			PST_FIELD(pSPacket, mrs_no));
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ","CARD_NO",			PST_FIELD(pSPacket, card_no));
#endif
		LOG_WRITE("%30s - (%s) ","CTY_BUS_SYS_DVS_CD",PST_FIELD(pSPacket, cty_bus_sys_dvs_cd));
		LOG_WRITE("%30s - (%s) ","TISU_SYS_DVS_CD",	PST_FIELD(pSPacket, tisu_sys_dvs_cd));
		LOG_WRITE("%30s - (%s) ","READ_WND_DVS",		PST_FIELD(pSPacket, read_wnd_dvs));
		LOG_WRITE("%30s - (number = %d) ","PUB_NUM",	*(int *)pSPacket->pub_num);
		nCount = *(int *) pSPacket->pub_num;
		for(i = 0; i < nCount; i++)
		{
			LOG_WRITE("%30s - (%s) ","TISU_ID", ST_FIELD(pSPacket->List[i], tisu_id));
		}
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(PUB_CHNL_DVS_CD	,	PST_FIELD(pSPacket, pub_chnl_dvs_cd		));	///< 발행채널구분코드		
		nRet = TMaxFBPut(MRS_NO				,	PST_FIELD(pSPacket, mrs_no				));	///< 예매번호			
		nRet = TMaxFBPut(CARD_NO			,	PST_FIELD(pSPacket, card_no				));	///< 카드번호			
		nRet = TMaxFBPut(CTY_BUS_SYS_DVS_CD	,	PST_FIELD(pSPacket, cty_bus_sys_dvs_cd	));	///< 시외버스시스템구분코드	
		nRet = TMaxFBPut(TISU_SYS_DVS_CD	,	PST_FIELD(pSPacket, tisu_sys_dvs_cd		));	///< 발권시스템구분코드		
		nRet = TMaxFBPut(READ_WND_DVS		,	PST_FIELD(pSPacket, read_wnd_dvs		));	///< 조회 창구 구분		
		nRet = TMaxFBPut(PUB_NUM			,	PST_FIELD(pSPacket, pub_num				));	///< 발행매수	
		nCount = *(int *) pSPacket->pub_num;
		for(i = 0; i < nCount; i++)
		{
			///< 발권ID			
			nRet = TMaxFBInsert(TISU_ID, pSPacket->List[i].tisu_id, i, sizeof(pSPacket->List[i].tisu_id) - 1);
		}

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(pSPacket->card_no, sizeof(pSPacket->card_no));
#endif
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		CMrnpMem* pTr;
		
		pTr = CMrnpMem::GetInstance();

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 514 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
		nRet = TMaxFBGetF(RSP_MSG					,	PST_FIELD(pRPacket, rsp_msg				));	///< 응답 메시지			
		nRet = TMaxFBGetF(PYN_MNS_DVS_CD			,	PST_FIELD(pRPacket, pyn_mns_dvs_cd		));	///< 지불수단구분코드	
		nRet = TMaxFBGetF(CARD_NO					,	PST_FIELD(pRPacket, card_no				));	///< 카드번호			
		nRet = TMaxFBGetF(CARD_APRV_NO				,	PST_FIELD(pRPacket, card_aprv_no		));	///< 카드승인번호		
		nRet = TMaxFBGetF(CARD_APRV_AMT				,	PST_FIELD(pRPacket, card_aprv_amt		));	///< 카드승인금액		
		nRet = TMaxFBGetF(CARD_APRV_VANC_CD			,	PST_FIELD(pRPacket, card_aprv_vanc_cd	));	///< 카드승인van사코드	
		nRet = TMaxFBGetF(ISSU_CRCM_CD				,	PST_FIELD(pRPacket, issu_crcm_cd		));	///< 발급카드사코드		
		nRet = TMaxFBGetF(BUY_CRCM_CD				,	PST_FIELD(pRPacket, buy_crcm_cd			));	///< 매입카드사코드		
		nRet = TMaxFBGetF(FRC_CMRT					,	PST_FIELD(pRPacket, frc_cmrt			));	///< 가맹점수수료율		
		// 20211015 ADD
		nRet = TMaxFBGetF(EVCP_PYM_AMT				,	PST_FIELD(pRPacket,	evcp_pym_amt		)); ///< 이벤트쿠폰결제금액				
		nRet = TMaxFBGetF(MLG_PYM_AMT				,	PST_FIELD(pRPacket,	mlg_pym_amt			)); ///< 마일리지결제금액		
		// 20211015 ~ADD
		nRet = TMaxFBGetF(ATL_DEPR_DT				,	PST_FIELD(pRPacket, atl_depr_dt			));	///< 실제출발일자		
		nRet = TMaxFBGetF(ATL_DEPR_TIME				,	PST_FIELD(pRPacket, atl_depr_time		));	///< 실제출발시각		
		nRet = TMaxFBGetF(DEPR_TRML_CD				,	PST_FIELD(pRPacket, depr_trml_cd		));	///< 출발터미널코드		
		nRet = TMaxFBGetF(ARVL_TRML_CD				,	PST_FIELD(pRPacket, arvl_trml_cd		));	///< 도착터미널코드		
		nRet = TMaxFBGetF(STPT_TRML_CD				,	PST_FIELD(pRPacket, stpt_trml_cd		));	///< 기점터미널코드		
		nRet = TMaxFBGetF(EPT_TRML_CD				,	PST_FIELD(pRPacket, ept_trml_cd			));	///< 종점터미널코드		
		nRet = TMaxFBGetF(BUS_CLS_CD				,	PST_FIELD(pRPacket, bus_cls_cd			));	///< 버스등급코드		
		nRet = TMaxFBGetF(BUS_CACM_CD				,	PST_FIELD(pRPacket, bus_cacm_cd			));	///< 버스운수사코드		
		nRet = TMaxFBGetF(RDHM_MLTP_VAL				,	PST_FIELD(pRPacket, rdhm_mltp_val		));	///< 승차홈다중값		
		nRet = TMaxFBGetF(CTY_BUS_OPRN_SHP_CD		,	PST_FIELD(pRPacket, cty_bus_oprn_shp_cd));	///< 시외버스운행형태코드
		nRet = TMaxFBGetF(DIST						,	PST_FIELD(pRPacket, dist				));	///< 거리				
		nRet = TMaxFBGetF(TAKE_DRTM					,	PST_FIELD(pRPacket, take_drtm			));	///< 소요시간			
		nRet = TMaxFBGetF(BUS_CLS_PRIN_YN			,	PST_FIELD(pRPacket, bus_cls_prin_yn		));	///< 버스등급출력여부	
		nRet = TMaxFBGetF(BUS_CACM_NM_PRIN_YN		,	PST_FIELD(pRPacket, bus_cacm_nm_prin_yn));	///< 버스운수사명출력여부
		nRet = TMaxFBGetF(DEPR_TIME_PRIN_YN			,	PST_FIELD(pRPacket, depr_time_prin_yn	));	///< 출발시각출력여부	
		nRet = TMaxFBGetF(SATS_NO_PRIN_YN			,	PST_FIELD(pRPacket, sats_no_prin_yn		));	///< 좌석번호출력여부	
		nRet = TMaxFBGetF(ALCN_WAY_DVS_CD			,	PST_FIELD(pRPacket, alcn_way_dvs_cd		));	///< 배차방식구분코드	
		nRet = TMaxFBGetF(SATI_USE_YN				,	PST_FIELD(pRPacket, sati_use_yn			));	///< 좌석제사용여부		
		nRet = TMaxFBGetF(PERD_TEMP_YN				,	PST_FIELD(pRPacket, perd_temp_yn		));	///< 정기임시여부		
		nRet = TMaxFBGetF(PUB_DT					,	PST_FIELD(pRPacket, pub_dt				));	///< 발행일자			
		nRet = TMaxFBGetF(PUB_TIME					,	PST_FIELD(pRPacket, pub_time			));	///< 발행시각			
		nRet = TMaxFBGetF(PUB_SHCT_TRML_CD			,	PST_FIELD(pRPacket, pub_shct_trml_cd	));	///< 발행단축터미널코드	
		nRet = TMaxFBGetF(PUB_WND_NO				,	PST_FIELD(pRPacket, pub_wnd_no			));	///< 발행창구번호		
		nRet = TMaxFBGetF(USER_DVS_NO				,	PST_FIELD(pRPacket, user_dvs_no			));	///< 사용자구분번호		
		nRet = TMaxFBGetF(USER_NM					,	PST_FIELD(pRPacket, user_nm				));	///< 사용자명			
		nRet = TMaxFBGetF(CMPY_NM					,	PST_FIELD(pRPacket, cmpy_nm				));	///< 회사명				
		nRet = TMaxFBGetF(PUB_NUM					,	PST_FIELD(pRPacket, pub_num				));	///< 발행매수
		nCount = *(int *)pRPacket->pub_num;
		if( nCount <= 0 )
		{	// 갯수 에러
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");

#if (_KTC_CERTIFY_ > 0)
			KTC_MemClear(pRPacket->card_no, sizeof(pRPacket->card_no));
#endif
			goto fail_proc;
		}
		
		{
			pRPacket->pList = new rtk_pubmrnp_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_pubmrnp_list_t) * nCount);
		}

		::CopyMemory(&pTr->res_complete, &pRPacket->pyn_mns_dvs_cd, pRPacket->pub_num - pRPacket->pyn_mns_dvs_cd);

		pTr->m_vtResComplete.clear();
		for(i = 0; i < nCount; i++)
		{
			prtk_pubmrnp_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(BUS_TCK_KND_CD		,	PST_FIELD(pList, bus_tck_knd_cd		));	///< 버스티켓종류코드				
			nRet = TMaxFBGetF(PUB_SNO				,	PST_FIELD(pList, pub_sno			));	///< 발행일련번호					
			nRet = TMaxFBGetF(BUS_TCK_INHR_NO		,	PST_FIELD(pList, bus_tck_inhr_no	));	///< 버스티켓고유번호				
			nRet = TMaxFBGetF(TISU_AMT				,	PST_FIELD(pList, tisu_amt			));	///< 발권금액						
			nRet = TMaxFBGetF(SATS_NO				,	PST_FIELD(pList, sats_no			));	///< 좌석번호						
			nRet = TMaxFBGetF(CTY_BUS_DC_KND_CD		,	PST_FIELD(pList, cty_bus_dc_knd_cd	));	///< 시외버스할인종류코드				
			nRet = TMaxFBGetF(DCRT_DVS_CD			,	PST_FIELD(pList, dcrt_dvs_cd		));	///< 할인율구분코드					
			nRet = TMaxFBGetF(DC_BEF_AMT			,	PST_FIELD(pList, dc_bef_amt			));	///< 할인이전금액					

			pTr->m_vtResComplete.push_back(*pList);
		}

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(pRPacket->card_no, sizeof(pRPacket->card_no));
#endif

		//std::sort(pTr->m_vtResComplete.begin(), pTr->m_vtResComplete.end(), SortFunction);
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ", "RSP_CD",				PST_FIELD(pRPacket, rsp_cd				));	///< 
		LOG_WRITE("%30s - (%s) ", "RSP_MSG",				PST_FIELD(pRPacket, rsp_msg			));	///< 
		LOG_WRITE("%30s - (%s) ", "PYN_MNS_DVS_CD",		PST_FIELD(pRPacket, pyn_mns_dvs_cd		));	///< 
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ", "CARD_NO",				PST_FIELD(pRPacket, card_no			));	///< 
#endif
		LOG_WRITE("%30s - (%s) ", "CARD_APRV_NO",			PST_FIELD(pRPacket, card_aprv_no		));	///< 
		LOG_WRITE("%30s - (number = %d) ", "CARD_APRV_AMT",	*(int *) pRPacket->card_aprv_amt		);	///< 
		LOG_WRITE("%30s - (%s) ", "CARD_APRV_VANC_CD",	PST_FIELD(pRPacket, card_aprv_vanc_cd	));	///< 
		LOG_WRITE("%30s - (%s) ", "ISSU_CRCM_CD",			PST_FIELD(pRPacket, issu_crcm_cd		));	///< 
		LOG_WRITE("%30s - (%s) ", "BUY_CRCM_CD",			PST_FIELD(pRPacket, buy_crcm_cd		));	///< 
		LOG_WRITE("%30s - (number = %d) ", "FRC_CMRT",	*(int *) pRPacket->frc_cmrt			);	///< 
		// 20211015 ADD
		LOG_WRITE("%30s - (number = %d) ", "EVCP_PYM_AMT",	*(int *) pRPacket->evcp_pym_amt			);	///< 이벤트쿠폰결제금액				
		LOG_WRITE("%30s - (number = %d) ", "MLG_PYM_AMT",	*(int *) pRPacket->mlg_pym_amt			);	///< 마일리지결제금액				
		// 20211015 ~ADD
		LOG_WRITE("%30s - (%s) ", "ATL_DEPR_DT",			PST_FIELD(pRPacket, atl_depr_dt		));	///< 
		LOG_WRITE("%30s - (%s) ", "ATL_DEPR_TIME",		PST_FIELD(pRPacket, atl_depr_time		));	///< 
		LOG_WRITE("%30s - (%s) ", "DEPR_TRML_CD",			PST_FIELD(pRPacket, depr_trml_cd		));	///< 
		LOG_WRITE("%30s - (%s) ", "ARVL_TRML_CD",			PST_FIELD(pRPacket, arvl_trml_cd		));	///< 
		LOG_WRITE("%30s - (%s) ", "STPT_TRML_CD",			PST_FIELD(pRPacket, stpt_trml_cd		));	///< 
		LOG_WRITE("%30s - (%s) ", "EPT_TRML_CD",			PST_FIELD(pRPacket, ept_trml_cd		));	///< 
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD",			PST_FIELD(pRPacket, bus_cls_cd			));	///< 
		LOG_WRITE("%30s - (%s) ", "BUS_CACM_CD",			PST_FIELD(pRPacket, bus_cacm_cd		));	///< 
		LOG_WRITE("%30s - (%s) ", "RDHM_MLTP_VAL",		PST_FIELD(pRPacket, rdhm_mltp_val		));	///< 
		LOG_WRITE("%30s - (%s) ", "CTY_BUS_OPRN_SHP_CD",	PST_FIELD(pRPacket, cty_bus_oprn_shp_cd));	///< 
		LOG_WRITE("%30s - (number = %d) ", "DIST",		*(int *) pRPacket->dist				);	///< 
		LOG_WRITE("%30s - (number = %d) ", "TAKE_DRTM",	*(int *) pRPacket->take_drtm			);	///< 
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_PRIN_YN",		PST_FIELD(pRPacket, bus_cls_prin_yn	));	///< 
		LOG_WRITE("%30s - (%s) ", "BUS_CACM_NM_PRIN_YN",	PST_FIELD(pRPacket, bus_cacm_nm_prin_yn));	///< 
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME_PRIN_YN",	PST_FIELD(pRPacket, depr_time_prin_yn	));	///< 
		LOG_WRITE("%30s - (%s) ", "SATS_NO_PRIN_YN",		PST_FIELD(pRPacket, sats_no_prin_yn	));	///< 
		LOG_WRITE("%30s - (%s) ", "ALCN_WAY_DVS_CD",		PST_FIELD(pRPacket, alcn_way_dvs_cd	));	///< 
		LOG_WRITE("%30s - (%s) ", "SATI_USE_YN",			PST_FIELD(pRPacket, sati_use_yn		));	///< 
		LOG_WRITE("%30s - (%s) ", "PERD_TEMP_YN",			PST_FIELD(pRPacket, perd_temp_yn		));	///< 
		LOG_WRITE("%30s - (%s) ", "PUB_DT",				PST_FIELD(pRPacket, pub_dt				));	///< 
		LOG_WRITE("%30s - (%s) ", "PUB_TIME",				PST_FIELD(pRPacket, pub_time			));	///< 
		LOG_WRITE("%30s - (%s) ", "PUB_SHCT_TRML_CD",		PST_FIELD(pRPacket, pub_shct_trml_cd	));	///< 
		LOG_WRITE("%30s - (%s) ", "PUB_WND_NO",			PST_FIELD(pRPacket, pub_wnd_no			));	///< 
		LOG_WRITE("%30s - (%s) ", "USER_DVS_NO",			PST_FIELD(pRPacket, user_dvs_no		));	///< 
		LOG_WRITE("%30s - (%s) ", "USER_NM",				PST_FIELD(pRPacket, user_nm			));	///< 
		LOG_WRITE("%30s - (%s) ", "CMPY_NM",				PST_FIELD(pRPacket, cmpy_nm			));	///< 
		LOG_WRITE("%30s - (number = %d) ", "PUB_NUM",		*(int *) pRPacket->pub_num				);	///< 

		for(i = 0; i < nCount; i++)
		{
			rtk_pubmrnp_list_t* pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD",		PST_FIELD(pList, bus_tck_knd_cd		));	///< 버스티켓종류코드	
			LOG_WRITE("%30s - (number = %d) ", "PUB_SNO",		*(int *) pList->pub_sno				);	///< 발행일련번호		
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_INHR_NO",		PST_FIELD(pList, bus_tck_inhr_no	));	///< 버스티켓고유번호	
			LOG_WRITE("%30s - (%s) ", "TISU_AMT",				PST_FIELD(pList, tisu_amt			));	///< 발권금액			
			LOG_WRITE("%30s - (number = %d) ", "SATS_NO",		*(int *) pList->sats_no				);	///< 좌석번호			
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_DC_KND_CD",	PST_FIELD(pList, cty_bus_dc_knd_cd	));	///< 시외버스할인종류코드	
			LOG_WRITE("%30s - (%s) ", "DCRT_DVS_CD",			PST_FIELD(pList, dcrt_dvs_cd		));	///< 할인율구분코드		
			LOG_WRITE("%30s - (number = %d) ", "DC_BEF_AMT",	*(int *) pList->dc_bef_amt			);	///< 할인이전금액		
		}
	}

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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc145
 * @details		(145) 버스티켓 취소 / 환불
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */

int CCBusServer::TMaxSvc145(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int nRet, nTimeout, nCount, i;
	int n_rsp_cd;
	char*	pService = "TK_CancRyTck";
	pstk_cancrytck_t	pSPacket;
	prtk_cancrytck_t	pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_cancrytck_t) pData;
	pRPacket = (prtk_cancrytck_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","CANC_RY_DVS_CD",	PST_FIELD(pSPacket, canc_ry_dvs_cd	));	///< 취소환불구분코드	
		LOG_WRITE("%30s - (number = %d) ","RYRT_NUM",	*(WORD *) pSPacket->ryrt_num			);	///< 환불매수		
		nCount = *(WORD *) pSPacket->ryrt_num;
		for(i = 0; i < nCount; i++)
		{
			pstk_cancrytck_list_t pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);

			LOG_WRITE("%30s - (%s) ","TISU_SYS_DVS_CD",	PST_FIELD(pList, tisu_sys_dvs_cd		));	///< 발권시스템구분코드
			LOG_WRITE("%30s - (%s) ","PUB_SYS_DVS_CD",	PST_FIELD(pList, pub_sys_dvs_cd		));	///< 발행시스템구분코드
			LOG_WRITE("%30s - (%s) ","PUB_DT",			PST_FIELD(pList, pub_dt				));	///< 발행일자		
			LOG_WRITE("%30s - (%s) ","PUB_SHCT_TRML_CD",	PST_FIELD(pList, pub_shct_trml_cd	));	///< 발행단축터미널코드
			LOG_WRITE("%30s - (%s) ","PUB_WND_NO",		PST_FIELD(pList, pub_wnd_no			));	///< 발행창구번호		
			LOG_WRITE("%30s - (number = %d) ","PUB_SNO",	*(int *) pList->pub_sno				);	///< 발행일련번호		
			LOG_WRITE("%30s - (%s) ","EB_BUS_TCK_SNO",	PST_FIELD(pList, eb_bus_tck_sno		));	///< EB버스티켓일련번호
			LOG_WRITE("%30s - (%s) ","RYRT_DVS_CD",		PST_FIELD(pList, ryrt_dvs_cd			));	///< 환불율구분코드	
			LOG_WRITE("%30s - (%s) ","PYN_MNS_DVS_CD",	PST_FIELD(pList, pyn_mns_dvs_cd		));	///< 지불수단구분코드	
			LOG_WRITE("%30s - (number = %d) ","TISU_AMT",	*(int *) pList->tisu_amt				);	///< 발권금액		
		}
	}

	// 20211015 ADD
	LOG_WRITE("%30s - (%s) ",		"pyn_mns_dvs_cd"	, &pSPacket->List[0].pyn_mns_dvs_cd[0]);
	
	// 지불구분코드 (1:현금,2:카드,3:마일리지,4:계좌이체,'L': 복합)
	if (pSPacket->List[0].pyn_mns_dvs_cd[0] == PYM_CD_COMP)
		pService = "TK_CancRyTckCp";	// TK_CancRyTckcp [시외](무인기-예매발행승차권 취소(복합결제)) 서비스 추가
	
	LOG_WRITE("%30s - (%s) ",		"pService Name"	, pService);
	// 20211015 ~ADD

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(CANC_RY_DVS_CD	,	PST_FIELD(pSPacket, canc_ry_dvs_cd	));	///< 취소환불구분코드	

		nCount = *(WORD *) pSPacket->ryrt_num;
		nRet = TMaxFBPutLen(RYRT_NUM, (char *)&nCount, sizeof(WORD));	///< 환불매수		
		for(i = 0; i < nCount; i++)
		{
			pstk_cancrytck_list_t pList;

			pList = &pSPacket->List[i];

			nRet = TMaxFBInsert(TISU_SYS_DVS_CD	,	PST_FIELD(pList, tisu_sys_dvs_cd), i, sizeof(pList->tisu_sys_dvs_cd) - 1);	///< 발권시스템구분코드
			nRet = TMaxFBInsert(PUB_SYS_DVS_CD	,	PST_FIELD(pList, pub_sys_dvs_cd	), i, sizeof(pList->pub_sys_dvs_cd) - 1);	///< 발행시스템구분코드
			nRet = TMaxFBInsert(PUB_DT			,	PST_FIELD(pList, pub_dt			), i, sizeof(pList->pub_dt) - 1);	///< 발행일자		
			nRet = TMaxFBInsert(PUB_SHCT_TRML_CD,	PST_FIELD(pList, pub_shct_trml_cd), i, sizeof(pList->pub_shct_trml_cd) - 1);	///< 발행단축터미널코드
			nRet = TMaxFBInsert(PUB_WND_NO		,	PST_FIELD(pList, pub_wnd_no		), i, sizeof(pList->pub_wnd_no) - 1);	///< 발행창구번호		
			nRet = TMaxFBInsert(PUB_SNO			,	PST_FIELD(pList, pub_sno		), i, sizeof(pList->pub_sno) - 1);	///< 발행일련번호		
			nRet = TMaxFBInsert(EB_BUS_TCK_SNO	,	PST_FIELD(pList, eb_bus_tck_sno	), i, sizeof(pList->eb_bus_tck_sno) - 1);	///< EB버스티켓일련번호
			nRet = TMaxFBInsert(RYRT_DVS_CD		,	PST_FIELD(pList, ryrt_dvs_cd	), i, sizeof(pList->ryrt_dvs_cd) - 1);	///< 환불율구분코드	
			nRet = TMaxFBInsert(PYN_MNS_DVS_CD	,	PST_FIELD(pList, pyn_mns_dvs_cd	), i, sizeof(pList->pyn_mns_dvs_cd) - 1);	///< 지불수단구분코드	
			nRet = TMaxFBInsert(TISU_AMT		,	PST_FIELD(pList, tisu_amt		), i, sizeof(pList->tisu_amt) - 1);	///< 발권금액		
		}
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 481 )
		{	// 에러코드 조회
			nRet = -98;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(RSP_MSG,		PST_FIELD(pRPacket, rsp_msg ));
		nRet = TMaxFBGetF(PYN_MNS_NUM,	PST_FIELD(pRPacket, pyn_mns_num ));

		nCount = *(WORD *) pRPacket->pyn_mns_num;
		if(nCount <= 0)
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - Count ERROR !!!!");
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_cancrytck_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_cancrytck_list_t) * nCount);
		}

		CCancRyTkMem::GetInstance()->m_vtRespTckNoList.clear();
		for(i = 0; i < nCount; i++)
		{
			prtk_cancrytck_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(PYN_MNS_DVS_CD,	PST_FIELD(pList, pyn_mns_dvs_cd ));
			nRet = TMaxFBGetF(CANC_RY_AMT,		PST_FIELD(pList, canc_ry_amt ));

			CCancRyTkMem::GetInstance()->m_vtRespRefundList.push_back(*pList);
		}
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ", "RSP_MSG",		PST_FIELD(pRPacket, rsp_msg));
		LOG_WRITE("%30s - (%s) ", "PYN_MNS_NUM",	PST_FIELD(pRPacket, pyn_mns_num));

		nCount = *(WORD *) pRPacket->pyn_mns_num;
		for(i = 0; i < nCount; i++)
		{
			prtk_cancrytck_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "PYN_MNS_DVS_CD",	PST_FIELD(pList, pyn_mns_dvs_cd));
			LOG_WRITE("%30s - (%s) ", "CANC_RY_AMT",		PST_FIELD(pList, canc_ry_amt));
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc150
 * @details		(150) 현금영수증 사후등록
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc150(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, nTimeout, i, nCount;
	int n_rsp_cd;
	char*	pService = "TK_Crrp";
	pstk_crrp_t	pSPacket;
	prtk_crrp_t	pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_crrp_t) pData;
	pRPacket = (prtk_crrp_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","CSRC_APRV_NO				", PST_FIELD(pSPacket, csrc_aprv_no				) );

#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ","DAMO_ENC_CSRC_AUTH_NO		", PST_FIELD(pSPacket, damo_enc_csrc_auth_no	) );
		LOG_WRITE("%30s - (%s) ","DAMO_ENC_CSRC_AUTH_NO_LEN	", PST_FIELD(pSPacket, damo_enc_csrc_auth_no_len) );
		LOG_WRITE("%30s - (%s) ","DAMO_ENC_DTA_KEY			", PST_FIELD(pSPacket, damo_enc_dta_key			) );
#endif
		LOG_WRITE("%30s - (%s) ","CPRT_YN					", PST_FIELD(pSPacket, cprt_yn					) );
		LOG_WRITE("%30s - (%s) ","RGT_NUM					", PST_FIELD(pSPacket, rgt_num					) );
		nCount = *(int *) pSPacket->rgt_num;
		for(i = 0; i < nCount; i++)
		{
			pstk_crrp_t_list_t pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("## Index (%d) ", i);	///> 

			LOG_WRITE("%30s - (%s) ","PUB_DT			",	PST_FIELD(pList, pub_dt				) );
			LOG_WRITE("%30s - (%s) ","PUB_SHCT_TRML_CD	",	PST_FIELD(pList, pub_shct_trml_cd	) );
			LOG_WRITE("%30s - (%s) ","PUB_WND_NO		",	PST_FIELD(pList, pub_wnd_no			) );
			LOG_WRITE("%30s - (%s) ","PUB_SNO			",	PST_FIELD(pList, pub_sno			) );
		}
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(CSRC_APRV_NO					,	PST_FIELD(pSPacket,	csrc_aprv_no				));	///> 현금영수증인증번호									
		nRet = TMaxFBPut(DAMO_ENC_CSRC_AUTH_NO			,	PST_FIELD(pSPacket,	damo_enc_csrc_auth_no		));	///> 현금영수증인증번호암호문								
		nRet = TMaxFBPut(DAMO_ENC_CSRC_AUTH_NO_LEN		,	PST_FIELD(pSPacket,	damo_enc_csrc_auth_no_len	));	///> 현금영수증인증번호암호문길이							
		nRet = TMaxFBPut(DAMO_ENC_DTA_KEY				,	PST_FIELD(pSPacket,	damo_enc_dta_key			));	///> 암호문키										
		nRet = TMaxFBPut(CPRT_YN						,	PST_FIELD(pSPacket,	cprt_yn						));	///> 법인여부										
		nRet = TMaxFBPut(RGT_NUM						,	PST_FIELD(pSPacket,	rgt_num						));	///> 등록매수					
		nCount = *(int *) pSPacket->rgt_num;
		for(i = 0; i < nCount; i++)
		{
			pstk_crrp_t_list_t pList;

			pList = &pSPacket->List[i];

			//nRet = TMaxFBInsert(PUB_DT			, PST_FIELD(pList, pub_dt			), i, sizeof(pList->pub_dt			) - 1);	///< 발행일자		
			//nRet = TMaxFBInsert(PUB_SHCT_TRML_CD, PST_FIELD(pList, pub_shct_trml_cd	), i, sizeof(pList->pub_shct_trml_cd) - 1);	///< 발행단축터미널코드	
			//nRet = TMaxFBInsert(PUB_WND_NO		, PST_FIELD(pList, pub_wnd_no		), i, sizeof(pList->pub_wnd_no		) - 1);	///< 발행창구번호		
			//nRet = TMaxFBInsert(PUB_SNO			, PST_FIELD(pList, pub_sno			), i, sizeof(pList->pub_sno			) - 1);	///< 발행일련번호		

			nRet = TMaxFBPut(PUB_DT				, PST_FIELD(pList, pub_dt			));	///< 발행일자		
			nRet = TMaxFBPut(PUB_SHCT_TRML_CD	, PST_FIELD(pList, pub_shct_trml_cd	));	///< 발행단축터미널코드	
			nRet = TMaxFBPut(PUB_WND_NO			, PST_FIELD(pList, pub_wnd_no		));	///< 발행창구번호		
			nRet = TMaxFBPut(PUB_SNO			, PST_FIELD(pList, pub_sno			));	///< 발행일련번호		
		}

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(pSPacket->damo_enc_csrc_auth_no, sizeof(pSPacket->damo_enc_csrc_auth_no));
		KTC_MemClear(pSPacket->damo_enc_dta_key, sizeof(pSPacket->damo_enc_dta_key));
#endif

	}

	// 동기화 (Send/Recv)
	nTimeout = 10;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
// 		CPubTckMem* pTr;
// 
// 		pTr = CPubTckMem::GetInstance();

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		nRet = TMaxFBGetF(RSP_MSG				,	PST_FIELD(pRPacket,	rsp_msg				));	 ///> 응답 메시지				
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 146 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(CSRC_APRV_NO			,	PST_FIELD(pRPacket,	csrc_aprv_no		));	 ///> 현금영수증승인번호		

		//pTr->csrc_resp
	}																								  
																									  
	///> Log out
	{
		LOG_WRITE("%30s - (%s) ", "RSP_CD",			PST_FIELD(pRPacket,	rsp_cd));	///> 
		LOG_WRITE("%30s - (%s) ", "RSP_MSG",		PST_FIELD(pRPacket,	rsp_msg));	///> 
		LOG_WRITE("%30s - (%s) ", "CSRC_APRV_NO",	PST_FIELD(pRPacket,	csrc_aprv_no));	///> 
	}

	nRet = nCount;

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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc152
 * @details		(152) 버스티켓 교환 요청
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc152(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int nRet, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ExcBusTck";
	pstk_excbustck_t	pSPacket;
	prtk_excbustck_t	pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_excbustck_t) pData;
	pRPacket = (prtk_excbustck_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","BUS_TCK_INHR_NO",	PST_FIELD(pSPacket, bus_tck_inhr_no));
		LOG_WRITE("%30s - (%s) ","ADD_BUS_TCK_INHR_NO",PST_FIELD(pSPacket, add_bus_tck_inhr_no));
	}

	///> send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(BUS_TCK_INHR_NO	,	PST_FIELD(pSPacket, bus_tck_inhr_no		));
		nRet = TMaxFBPut(ADD_BUS_TCK_INHR_NO,	PST_FIELD(pSPacket, add_bus_tck_inhr_no));
	}

	///> 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( (n_rsp_cd != 141) && (n_rsp_cd != 146) )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc155
 * @details		(155) 환불 내역 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc155(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ReadRyPt";
	pstk_readrypt_t	pSPacket;
	prtk_readrypt_t	pRPacket;
	prtk_readrypt_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readrypt_t) pData;
	pRPacket = (prtk_readrypt_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","READ_DT",			PST_FIELD(pSPacket, read_dt));
		LOG_WRITE("%30s - (%s) ","READ_TIME",			PST_FIELD(pSPacket, read_time));
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_CD",		PST_FIELD(pSPacket, depr_trml_cd));
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_CD",		PST_FIELD(pSPacket, arvl_trml_cd));
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(READ_DT			,	PST_FIELD(pSPacket, read_dt			 ));	///< 요청 조회 대상일		
		nRet = TMaxFBPut(READ_TIME			,	PST_FIELD(pSPacket, read_time		 ));	///< 요청 조회 대상 시각		
		nRet = TMaxFBPut(DEPR_TRML_CD		,	PST_FIELD(pSPacket, depr_trml_cd	 ));	///< 출발터미널코드			
		nRet = TMaxFBPut(ARVL_TRML_CD		,	PST_FIELD(pSPacket, arvl_trml_cd	 ));	///< 도착터미널코드			
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(READ_DTA_NUM		,	PST_FIELD(pRPacket, read_dta_num	));	///< 갯수
		nCount = *(int *)pRPacket->read_dta_num;
		if( nCount <= 0 )
		{	// count error
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}
		
		{
			pRPacket->pList = new rtk_readrypt_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readrypt_list_t) * nCount);
		}

		OperGetFileName(OPER_FILE_ID_0155, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->read_dta_num, sizeof(pRPacket->read_dta_num));

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(PUB_DT			,	PST_FIELD(pList, pub_dt				));	///< 발행일자			
			nRet = TMaxFBGetF(PUB_SHCT_TRML_CD	,	PST_FIELD(pList, pub_shct_trml_cd	));	///< 발행단축터미널코드	
			nRet = TMaxFBGetF(PUB_WND_NO		,	PST_FIELD(pList, pub_wnd_no			));	///< 발행창구번호		
			nRet = TMaxFBGetF(PUB_SNO			,	PST_FIELD(pList, pub_sno			));	///< 발행일련번호(int)	
			nRet = TMaxFBGetF(ROT_KND_CD		,	PST_FIELD(pList, rot_knd_cd			));	///< 노선종류코드		
			nRet = TMaxFBGetF(PYN_MNS_DVS_CD	,	PST_FIELD(pList, pyn_mns_dvs_cd		));	///< 지불수단구분코드	
			nRet = TMaxFBGetF(ROT_NM			,	PST_FIELD(pList, rot_nm				));	///< 노선명				
			nRet = TMaxFBGetF(PUB_TIME			,	PST_FIELD(pList, pub_time			));	///< 발행시각			
			nRet = TMaxFBGetF(BUS_TCK_INHR_NO	,	PST_FIELD(pList, bus_tck_inhr_no	));	///< 버스티켓고유번호	
			nRet = TMaxFBGetF(TISU_AMT			,	PST_FIELD(pList, tisu_amt			));	///< 발권금액(int)		
			nRet = TMaxFBGetF(SATS_NO			,	PST_FIELD(pList, sats_no			));	///< 좌석번호(int)
			nRet = TMaxFBGetF(DEPR_DT			,	PST_FIELD(pList, depr_dt			));	///< 출발일자			
			nRet = TMaxFBGetF(DEPR_TIME			,	PST_FIELD(pList, depr_time			));	///< 출발시각			
			nRet = TMaxFBGetF(DEPR_TRML_CD		,	PST_FIELD(pList, depr_trml_cd		));	///< 출발터미널코드		
			nRet = TMaxFBGetF(ARVL_TRML_CD		,	PST_FIELD(pList, arvl_trml_cd		));	///< 도착터미널코드		
			nRet = TMaxFBGetF(BUS_CLS_CD		,	PST_FIELD(pList, bus_cls_cd			));	///< 버스등급코드		
			nRet = TMaxFBGetF(BUS_TCK_KND_CD	,	PST_FIELD(pList, bus_tck_knd_cd		));	///< 버스티켓종류코드	
			nRet = TMaxFBGetF(CANC_RY_DVS_CD	,	PST_FIELD(pList, canc_ry_dvs_cd		));	///< 취소환불구분코드	

			nRet = TMaxFBGetF(RYRT_DVS_CD		,	PST_FIELD(pList, ryrt_dvs_cd		));	///< 환불율구분코드	
			nRet = TMaxFBGetF(CANC_RY_AMT		,	PST_FIELD(pList, canc_ry_amt		));	///< 취소환불금액	

			nRet = TMaxFBGetF(PUB_USER_ID		,	PST_FIELD(pList, pub_user_id		));	///< 발행사용자id		
			nRet = TMaxFBGetF(RGT_TIME			,	PST_FIELD(pList, rgt_time			));	///< 등록 시각			
			nRet = TMaxFBGetF(RGT_USER_ID		,	PST_FIELD(pList, rgt_user_id		));	///< 등록사용자id		
			nRet = TMaxFBGetF(CTY_BUS_DC_KND_CD	,	PST_FIELD(pList, cty_bus_dc_knd_cd	));	///< 시외버스할인종류코드
			nRet = TMaxFBGetF(DCRT_DVS_CD		,	PST_FIELD(pList, dcrt_dvs_cd		));	///< 할인율구분코드		
			nRet = TMaxFBGetF(DC_BEF_AMT		,	PST_FIELD(pList, dc_bef_amt			));	///< 할인이전금액(int)	
			nRet = TMaxFBGetF(PUB_SYS_DVS_CD	,	PST_FIELD(pList, pub_sys_dvs_cd		));	///< 발행시스템구분코드	
			nRet = TMaxFBGetF(EB_BUS_TCK_SNO	,	PST_FIELD(pList, eb_bus_tck_sno		));	///< eb버스티켓일련번호	
			nRet = TMaxFBGetF(QR_PYM_PYN_DTL_CD	,	PST_FIELD(pList, qr_pym_pyn_dtl_cd	)); ///< QR결제지불상세코드	// 20221208 ADD
		
			MyWriteFile(hFile, pList, sizeof(rtk_readrypt_list_t));
		}
		MyCloseFile(hFile);
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ","RSP_CD",	   PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%d) ","READ_DTA_NUM", nCount);

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "PUB_DT",				PST_FIELD(pList, pub_dt				));	///< 발행일자			
			LOG_WRITE("%30s - (%s) ", "PUB_SHCT_TRML_CD",	PST_FIELD(pList, pub_shct_trml_cd	));	///< 발행단축터미널코드	
			LOG_WRITE("%30s - (%s) ", "PUB_WND_NO",			PST_FIELD(pList, pub_wnd_no			));	///< 발행창구번호		
			LOG_WRITE("%30s - (number = %d) ", "PUB_SNO",	*(int *) pList->pub_sno				);	///< 발행일련번호(int)	
			LOG_WRITE("%30s - (%s) ", "ROT_KND_CD",			PST_FIELD(pList, rot_knd_cd			));	///< 노선종류코드		
			LOG_WRITE("%30s - (%s) ", "PYN_MNS_DVS_CD",		PST_FIELD(pList, pyn_mns_dvs_cd		));	///< 지불수단구분코드	
			LOG_WRITE("%30s - (%s) ", "ROT_NM",				PST_FIELD(pList, rot_nm				));	///< 노선명				
			LOG_WRITE("%30s - (%s) ", "PUB_TIME",			PST_FIELD(pList, pub_time			));	///< 발행시각			
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_INHR_NO",	PST_FIELD(pList, bus_tck_inhr_no	));	///< 버스티켓고유번호	
			LOG_WRITE("%30s - (number = %d) ", "TISU_AMT",	*(int *) pList->tisu_amt			);	///< 발권금액(int)		
			LOG_WRITE("%30s - (number = %d) ", "SATS_NO",	*(int *) pList->sats_no				);	///< 좌석번호(int)
			LOG_WRITE("%30s - (%s) ", "DEPR_DT",			PST_FIELD(pList, depr_dt			));	///< 출발일자			
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME",			PST_FIELD(pList, depr_time			));	///< 출발시각			
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_CD",		PST_FIELD(pList, depr_trml_cd		));	///< 출발터미널코드		
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_CD",		PST_FIELD(pList, arvl_trml_cd		));	///< 도착터미널코드		
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD",			PST_FIELD(pList, bus_cls_cd			));	///< 버스등급코드		
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD",		PST_FIELD(pList, bus_tck_knd_cd		));	///< 버스티켓종류코드	
			LOG_WRITE("%30s - (%s) ", "CANC_RY_DVS_CD",		PST_FIELD(pList, canc_ry_dvs_cd		));	///< 취소환불구분코드	

			LOG_WRITE("%30s - (%s) ", "RYRT_DVS_CD",			PST_FIELD(pList, ryrt_dvs_cd		));	///< 환불율구분코드	
			LOG_WRITE("%30s - (number = %d) ", "CANC_RY_AMT",	*(int *) pList->canc_ry_amt			);	///< 취소환불금액	

			LOG_WRITE("%30s - (%s) ", "PUB_USER_ID",			PST_FIELD(pList, pub_user_id		));	///< 발행사용자id		
			LOG_WRITE("%30s - (%s) ", "RGT_TIME",				PST_FIELD(pList, rgt_time			));	///< 등록 시각			
			LOG_WRITE("%30s - (%s) ", "RGT_USER_ID",			PST_FIELD(pList, rgt_user_id		));	///< 등록사용자id		
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_DC_KND_CD",		PST_FIELD(pList, cty_bus_dc_knd_cd	));	///< 시외버스할인종류코드
			LOG_WRITE("%30s - (%s) ", "DCRT_DVS_CD",			PST_FIELD(pList, dcrt_dvs_cd		));	///< 할인율구분코드		
			LOG_WRITE("%30s - (number = %d) ", "DC_BEF_AMT",	*(int *) pList->dc_bef_amt			);	///< 할인이전금액(int)	
			LOG_WRITE("%30s - (%s) ", "PUB_SYS_DVS_CD",			PST_FIELD(pList, pub_sys_dvs_cd		));	///< 발행시스템구분코드	
			LOG_WRITE("%30s - (%s) ", "EB_BUS_TCK_SNO",			PST_FIELD(pList, eb_bus_tck_sno		));	///< eb버스티켓일련번호	
			LOG_WRITE("%30s - (%s) ", "QR_PYM_PYN_DTL_CD",		PST_FIELD(pList, qr_pym_pyn_dtl_cd	));	///< QR결제지불상세코드	// 20221205 ADD			
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc157
 * @details		(157) 재발행내역 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc157(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ReadRtckPt";
	pstk_readrtckpt_t	pSPacket;
	prtk_readrtckpt_t	pRPacket;
	prtk_readrtckpt_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readrtckpt_t) pData;
	pRPacket = (prtk_readrtckpt_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","READ_DT",			PST_FIELD(pSPacket, read_dt));
		LOG_WRITE("%30s - (%s) ","READ_TIME",			PST_FIELD(pSPacket, read_time));
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_CD",		PST_FIELD(pSPacket, depr_trml_cd));
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_CD",		PST_FIELD(pSPacket, arvl_trml_cd));
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(READ_DT			,	PST_FIELD(pSPacket, read_dt			 ));	///< 요청 조회 대상일		
		nRet = TMaxFBPut(READ_TIME			,	PST_FIELD(pSPacket, read_time		 ));	///< 요청 조회 대상 시각		
		nRet = TMaxFBPut(DEPR_TRML_CD		,	PST_FIELD(pSPacket, depr_trml_cd	 ));	///< 출발터미널코드			
		nRet = TMaxFBPut(ARVL_TRML_CD		,	PST_FIELD(pSPacket, arvl_trml_cd	 ));	///< 도착터미널코드			
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(READ_DTA_NUM		,	PST_FIELD(pRPacket, read_dta_num	));	///< 갯수
		nCount = *(int *)pRPacket->read_dta_num;
//		if( nCount <= 0 )
		if( nCount < 0 )
		{	// count error
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		if( nCount > 0 )
		{
			pRPacket->pList = new rtk_readrtckpt_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readrtckpt_list_t) * nCount);
		}

		OperGetFileName(OPER_FILE_ID_0157, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->read_dta_num, sizeof(pRPacket->read_dta_num));

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(PUB_DT			,	PST_FIELD(pList, pub_dt				));	///< 발행일자			
			nRet = TMaxFBGetF(PUB_SHCT_TRML_CD	,	PST_FIELD(pList, pub_shct_trml_cd	));	///< 발행단축터미널코드	
			nRet = TMaxFBGetF(PUB_WND_NO		,	PST_FIELD(pList, pub_wnd_no			));	///< 발행창구번호		
			nRet = TMaxFBGetF(PUB_SNO			,	PST_FIELD(pList, pub_sno			));	///< 발행일련번호(int)	
			nRet = TMaxFBGetF(ROT_KND_CD		,	PST_FIELD(pList, rot_knd_cd			));	///< 노선종류코드		
			nRet = TMaxFBGetF(PYN_MNS_DVS_CD	,	PST_FIELD(pList, pyn_mns_dvs_cd		));	///< 지불수단구분코드	
			nRet = TMaxFBGetF(ROT_NM			,	PST_FIELD(pList, rot_nm				));	///< 노선명				
			nRet = TMaxFBGetF(PUB_TIME			,	PST_FIELD(pList, pub_time			));	///< 발행시각			
			nRet = TMaxFBGetF(BUS_TCK_INHR_NO	,	PST_FIELD(pList, bus_tck_inhr_no	));	///< 버스티켓고유번호	
			nRet = TMaxFBGetF(TISU_AMT			,	PST_FIELD(pList, tisu_amt			));	///< 발권금액(int)		
			nRet = TMaxFBGetF(SATS_NO			,	PST_FIELD(pList, sats_no			));	///< 좌석번호(int)
			nRet = TMaxFBGetF(DEPR_DT			,	PST_FIELD(pList, depr_dt			));	///< 출발일자			
			nRet = TMaxFBGetF(DEPR_TIME			,	PST_FIELD(pList, depr_time			));	///< 출발시각			
			nRet = TMaxFBGetF(DEPR_TRML_CD		,	PST_FIELD(pList, depr_trml_cd		));	///< 출발터미널코드		
			nRet = TMaxFBGetF(ARVL_TRML_CD		,	PST_FIELD(pList, arvl_trml_cd		));	///< 도착터미널코드		
			nRet = TMaxFBGetF(BUS_CLS_CD		,	PST_FIELD(pList, bus_cls_cd			));	///< 버스등급코드		
			nRet = TMaxFBGetF(BUS_TCK_KND_CD	,	PST_FIELD(pList, bus_tck_knd_cd		));	///< 버스티켓종류코드	
			nRet = TMaxFBGetF(CANC_RY_DVS_CD	,	PST_FIELD(pList, canc_ry_dvs_cd		));	///< 취소환불구분코드	
			nRet = TMaxFBGetF(PUB_USER_ID		,	PST_FIELD(pList, pub_user_id		));	///< 발행사용자id		
			nRet = TMaxFBGetF(RGT_TIME			,	PST_FIELD(pList, rgt_time			));	///< 등록 시각			
			nRet = TMaxFBGetF(RGT_USER_ID		,	PST_FIELD(pList, rgt_user_id		));	///< 등록사용자id		
			nRet = TMaxFBGetF(CTY_BUS_DC_KND_CD	,	PST_FIELD(pList, cty_bus_dc_knd_cd	));	///< 시외버스할인종류코드
			nRet = TMaxFBGetF(DCRT_DVS_CD		,	PST_FIELD(pList, dcrt_dvs_cd		));	///< 할인율구분코드		
			nRet = TMaxFBGetF(DC_BEF_AMT		,	PST_FIELD(pList, dc_bef_amt			));	///< 할인이전금액(int)	
			nRet = TMaxFBGetF(PUB_SYS_DVS_CD	,	PST_FIELD(pList, pub_sys_dvs_cd		));	///< 발행시스템구분코드	
			nRet = TMaxFBGetF(EB_BUS_TCK_SNO	,	PST_FIELD(pList, eb_bus_tck_sno		));	///< eb버스티켓일련번호	
			nRet = TMaxFBGetF(QR_PYM_PYN_DTL_CD	,	PST_FIELD(pList, qr_pym_pyn_dtl_cd	)); ///< QR결제지불상세코드	// 20221208 ADD

			MyWriteFile(hFile, pList, sizeof(rtk_readrtckpt_list_t));
		}
		MyCloseFile(hFile);
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ","RSP_CD",	   PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%d) ","READ_DTA_NUM", nCount);

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "PUB_DT",				PST_FIELD(pList, pub_dt				));	///< 발행일자			
			LOG_WRITE("%30s - (%s) ", "PUB_SHCT_TRML_CD",	PST_FIELD(pList, pub_shct_trml_cd	));	///< 발행단축터미널코드	
			LOG_WRITE("%30s - (%s) ", "PUB_WND_NO",			PST_FIELD(pList, pub_wnd_no			));	///< 발행창구번호		
			LOG_WRITE("%30s - (%s) ", "PUB_SNO",			PST_FIELD(pList, pub_sno			));	///< 발행일련번호(int)	
			LOG_WRITE("%30s - (%s) ", "ROT_KND_CD",			PST_FIELD(pList, rot_knd_cd			));	///< 노선종류코드		
			LOG_WRITE("%30s - (%s) ", "PYN_MNS_DVS_CD",		PST_FIELD(pList, pyn_mns_dvs_cd		));	///< 지불수단구분코드	
			LOG_WRITE("%30s - (%s) ", "ROT_NM",				PST_FIELD(pList, rot_nm				));	///< 노선명				
			LOG_WRITE("%30s - (%s) ", "PUB_TIME",			PST_FIELD(pList, pub_time			));	///< 발행시각			
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_INHR_NO",	PST_FIELD(pList, bus_tck_inhr_no	));	///< 버스티켓고유번호	
			LOG_WRITE("%30s - (%s) ", "TISU_AMT",			PST_FIELD(pList, tisu_amt			));	///< 발권금액(int)		
			LOG_WRITE("%30s - (%s) ", "SATS_NO",			PST_FIELD(pList, sats_no			));	///< 좌석번호(int)
			LOG_WRITE("%30s - (%s) ", "DEPR_DT",			PST_FIELD(pList, depr_dt			));	///< 출발일자			
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME",			PST_FIELD(pList, depr_time			));	///< 출발시각			
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_CD",		PST_FIELD(pList, depr_trml_cd		));	///< 출발터미널코드		
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_CD",		PST_FIELD(pList, arvl_trml_cd		));	///< 도착터미널코드		
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD",			PST_FIELD(pList, bus_cls_cd			));	///< 버스등급코드		
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD",		PST_FIELD(pList, bus_tck_knd_cd		));	///< 버스티켓종류코드	
			LOG_WRITE("%30s - (%s) ", "CANC_RY_DVS_CD",		PST_FIELD(pList, canc_ry_dvs_cd		));	///< 취소환불구분코드	
			LOG_WRITE("%30s - (%s) ", "PUB_USER_ID",		PST_FIELD(pList, pub_user_id		));	///< 발행사용자id		
			LOG_WRITE("%30s - (%s) ", "RGT_TIME",			PST_FIELD(pList, rgt_time			));	///< 등록 시각			
			LOG_WRITE("%30s - (%s) ", "RGT_USER_ID",		PST_FIELD(pList, rgt_user_id		));	///< 등록사용자id		
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_DC_KND_CD",	PST_FIELD(pList, cty_bus_dc_knd_cd	));	///< 시외버스할인종류코드
			LOG_WRITE("%30s - (%s) ", "DCRT_DVS_CD",		PST_FIELD(pList, dcrt_dvs_cd		));	///< 할인율구분코드		
			LOG_WRITE("%30s - (%s) ", "DC_BEF_AMT",			PST_FIELD(pList, dc_bef_amt			));	///< 할인이전금액(int)	
			LOG_WRITE("%30s - (%s) ", "PUB_SYS_DVS_CD",		PST_FIELD(pList, pub_sys_dvs_cd		));	///< 발행시스템구분코드	
			LOG_WRITE("%30s - (%s) ", "EB_BUS_TCK_SNO",		PST_FIELD(pList, eb_bus_tck_sno		));	///< eb버스티켓일련번호	
			LOG_WRITE("%30s - (%s) ", "QR_PYM_PYN_DTL_CD",	PST_FIELD(pList, qr_pym_pyn_dtl_cd	));	///< QR결제지불상세코드	// 20221205 ADD			
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc158
 * @details		(158) 버스티켓 일련번호 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc158(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ReadBusTckNo";
	pstk_readbustckno_t	pSPacket;
	prtk_readbustckno_t	pRPacket;
	prtk_readbustckno_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readbustckno_t) pData;
	pRPacket = (prtk_readbustckno_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","PUB_SYS_DVS_CD"		,	PST_FIELD(pSPacket, pub_sys_dvs_cd	));	 ///< 발행시스템구분코드
		LOG_WRITE("%30s - (%s) ","PUB_DT"				,	PST_FIELD(pSPacket, pub_dt			));	 ///< 발행일자		
		LOG_WRITE("%30s - (%s) ","PUB_SHCT_TRML_CD"	,	PST_FIELD(pSPacket, pub_shct_trml_cd));	 ///< 발행단축터미널코드
		LOG_WRITE("%30s - (%s) ","PUB_WND_NO"			,	PST_FIELD(pSPacket, pub_wnd_no		));	 ///< 발행창구번호		
		LOG_WRITE("%30s - (number = %d) ","PUB_SNO"	,	*(int *) pSPacket->pub_sno			);	 ///< 발행일련번호		
		LOG_WRITE("%30s - (%s) ","EB_BUS_TCK_SNO"		,	PST_FIELD(pSPacket, eb_bus_tck_sno	));	 ///< eb버스티켓일련번호
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(PUB_SYS_DVS_CD	,	PST_FIELD(pSPacket, pub_sys_dvs_cd	));	///< 발행시스템구분코드
		nRet = TMaxFBPut(PUB_DT			,	PST_FIELD(pSPacket, pub_dt			));	///< 발행일자		
		nRet = TMaxFBPut(PUB_SHCT_TRML_CD,	PST_FIELD(pSPacket, pub_shct_trml_cd));	///< 발행단축터미널코드
		nRet = TMaxFBPut(PUB_WND_NO		,	PST_FIELD(pSPacket, pub_wnd_no		));	///< 발행창구번호		
		nRet = TMaxFBPut(PUB_SNO		,	PST_FIELD(pSPacket, pub_sno			));	///< 발행일련번호		
		nRet = TMaxFBPut(EB_BUS_TCK_SNO	,	PST_FIELD(pSPacket, eb_bus_tck_sno	));	///< eb버스티켓일련번호
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	///> recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(ROT_KND_CD		,	PST_FIELD(pRPacket, rot_knd_cd			));	///< 노선종류코드		
		nRet = TMaxFBGetF(PYN_MNS_DVS_CD	,	PST_FIELD(pRPacket, pyn_mns_dvs_cd		));	///< 지불수단구분코드	
		nRet = TMaxFBGetF(ROT_ID			,	PST_FIELD(pRPacket, rot_id				));	///< 노선id		
		nRet = TMaxFBGetF(ROT_NM			,	PST_FIELD(pRPacket, rot_nm				));	///< 노선명			
		nRet = TMaxFBGetF(ROT_SQNO			,	PST_FIELD(pRPacket, rot_sqno			));	///< 노선순번		
		nRet = TMaxFBGetF(ALCN_DT			,	PST_FIELD(pRPacket, alcn_dt				));	///< 배차일자		
		nRet = TMaxFBGetF(ALCN_SQNO			,	PST_FIELD(pRPacket, alcn_sqno			));	///< 배차순번		
		nRet = TMaxFBGetF(PUB_TIME			,	PST_FIELD(pRPacket, pub_time			));	///< 발행시각		
		nRet = TMaxFBGetF(BUS_TCK_INHR_NO	,	PST_FIELD(pRPacket, bus_tck_inhr_no		));	///< 버스티켓고유번호	
		nRet = TMaxFBGetF(TISU_AMT			,	PST_FIELD(pRPacket, tisu_amt			));	///< 발권금액		
		nRet = TMaxFBGetF(SATS_NO			,	PST_FIELD(pRPacket, sats_no				));	///< 좌석번호		
		nRet = TMaxFBGetF(DEPR_DT			,	PST_FIELD(pRPacket, depr_dt				));	///< 출발일자		
		nRet = TMaxFBGetF(DEPR_TIME			,	PST_FIELD(pRPacket, depr_time			));	///< 출발시각		
		nRet = TMaxFBGetF(DEPR_TRML_CD		,	PST_FIELD(pRPacket, depr_trml_cd		));	///< 출발터미널코드	
		nRet = TMaxFBGetF(ARVL_TRML_CD		,	PST_FIELD(pRPacket, arvl_trml_cd		));	///< 도착터미널코드	
		nRet = TMaxFBGetF(STPT_TRML_CD		,	PST_FIELD(pRPacket, stpt_trml_cd		));	///< 기점터미널코드	
		nRet = TMaxFBGetF(EPT_TRML_CD		,	PST_FIELD(pRPacket, ept_trml_cd			));	///< 종점터미널코드	
		nRet = TMaxFBGetF(BUS_CLS_CD		,	PST_FIELD(pRPacket, bus_cls_cd			));	///< 버스등급코드		
		nRet = TMaxFBGetF(BUS_CACM_CD		,	PST_FIELD(pRPacket, bus_cacm_cd			));	///< 버스운수사코드	
		nRet = TMaxFBGetF(BUS_TCK_KND_CD	,	PST_FIELD(pRPacket, bus_tck_knd_cd		));	///< 버스티켓종류코드	
		nRet = TMaxFBGetF(ALCN_WAY_DVS_CD	,	PST_FIELD(pRPacket, alcn_way_dvs_cd		));	///< 배차방식구분코드	
		nRet = TMaxFBGetF(SATI_USE_YN		,	PST_FIELD(pRPacket, sati_use_yn			));	///< 좌석제사용여부 // 20221201 ADD
		nRet = TMaxFBGetF(TISU_STA_DVS_CD	,	PST_FIELD(pRPacket, tisu_sta_dvs_cd		));	///< 발권상태구분코드	
		nRet = TMaxFBGetF(PERD_TEMP_YN		,	PST_FIELD(pRPacket, perd_temp_yn		));	///< 정기임시여부		
		nRet = TMaxFBGetF(APRV_NO			,	PST_FIELD(pRPacket, aprv_no				));	///< 승인번호		
		nRet = TMaxFBGetF(CSRC_AUTH_NO		,	PST_FIELD(pRPacket, csrc_auth_no		));	///< 현금영수증인증번호	
		nRet = TMaxFBGetF(CPRT_YN			,	PST_FIELD(pRPacket, cprt_yn				));	///< 법인여부		
		nRet = TMaxFBGetF(CARD_NO			,	PST_FIELD(pRPacket, card_no				));	///< 카드번호		
		nRet = TMaxFBGetF(PUB_USER_ID		,	PST_FIELD(pRPacket, pub_user_id			));	///< 발행사용자id	
		nRet = TMaxFBGetF(CANC_RY_DVS_CD	,	PST_FIELD(pRPacket, canc_ry_dvs_cd		));	///< 취소환불구분코드	
		nRet = TMaxFBGetF(RYRT_DVS_CD		,	PST_FIELD(pRPacket, ryrt_dvs_cd			));	///< 환불율구분코드	
		nRet = TMaxFBGetF(CANC_RY_DT		,	PST_FIELD(pRPacket, canc_ry_dt			));	///< 취소환불일자		
		nRet = TMaxFBGetF(CANC_RY_TIME		,	PST_FIELD(pRPacket, canc_ry_time		));	///< 취소환불시각		
		nRet = TMaxFBGetF(CANC_RY_AMT		,	PST_FIELD(pRPacket, canc_ry_amt			));	///< 취소환불금액		
		nRet = TMaxFBGetF(CANC_RY_TRML_CD	,	PST_FIELD(pRPacket, canc_ry_trml_cd		));	///< 취소환불터미널코드	
		nRet = TMaxFBGetF(CANC_RY_WND_NO	,	PST_FIELD(pRPacket, canc_ry_wnd_no		));	///< 취소환불창구번호	
		nRet = TMaxFBGetF(CANC_RY_USER_ID	,	PST_FIELD(pRPacket, canc_ry_user_id		));	///< 취소환불사용자id	
		nRet = TMaxFBGetF(SATS_REST_DT		,	PST_FIELD(pRPacket, sats_rest_dt		));	///< 좌석복원일자		
		nRet = TMaxFBGetF(SATS_REST_TIME	,	PST_FIELD(pRPacket, sats_rest_time		));	///< 좌석복원시각		
		nRet = TMaxFBGetF(SATS_REST_TRML_CD	,	PST_FIELD(pRPacket, sats_rest_trml_cd	));	///< 좌석복원터미널코드	
		nRet = TMaxFBGetF(SATS_REST_WND_NO	,	PST_FIELD(pRPacket, sats_rest_wnd_no	));	///< 좌석복원창구번호	
		nRet = TMaxFBGetF(SATS_REST_USER_ID	,	PST_FIELD(pRPacket, sats_rest_user_id	));	///< 좌석복원사용자id	
		nRet = TMaxFBGetF(CTY_BUS_DC_KND_CD	,	PST_FIELD(pRPacket, cty_bus_dc_knd_cd	));	///< 시외버스할인종류코드
		nRet = TMaxFBGetF(DCRT_DVS_CD		,	PST_FIELD(pRPacket, dcrt_dvs_cd			));	///< 할인율구분코드	
		nRet = TMaxFBGetF(DC_BEF_AMT		,	PST_FIELD(pRPacket, dc_bef_amt			));	///< 할인이전금액		
		nRet = TMaxFBGetF(TISU_SYS_DVS_CD	,	PST_FIELD(pRPacket, tisu_sys_dvs_cd		));	///< 발권시스템구분코드	
		nRet = TMaxFBGetF(PUB_SYS_DVS_CD	,	PST_FIELD(pRPacket, pub_sys_dvs_cd		));	///< 발행시스템구분코드	
		nRet = TMaxFBGetF(EB_BUS_TCK_SNO	,	PST_FIELD(pRPacket, eb_bus_tck_sno		));	///< eb버스티켓일련번호	
		nRet = TMaxFBGetF(MISS_ERR_YN		,	PST_FIELD(pRPacket, miss_err_yn			));	///< 분실오류여부		
		nRet = TMaxFBGetF(BRKP_TYPE			,	PST_FIELD(pRPacket, brkp_type			));	///< 위약금 타입(BRKP_TYPE)					// 20221220 ADD	
		nRet = TMaxFBGetF(QR_PYM_PYN_DTL_CD	,	PST_FIELD(pRPacket, qr_pym_pyn_dtl_cd	));	///< QR결제지불상세코드(QR_PYM_PYN_DTL_CD)	// 20221220 ADD		

		/// 응답데이타 메모리 복사
		{
			::CopyMemory(&CCancRyTkMem::GetInstance()->tRespTckNo, pRPacket->rot_knd_cd, sizeof(RESP_TICKET_NO_T));
		}
			
		sprintf(CCancRyTkMem::GetInstance()->tRespTckNo.brkp_type, "%s", PST_FIELD(pRPacket, brkp_type));					// 20221222 ADD
		sprintf(CCancRyTkMem::GetInstance()->tRespTckNo.qr_pym_pyn_dtl_cd, "%s", PST_FIELD(pRPacket, qr_pym_pyn_dtl_cd));	// 20221220 ADD
		//LOG_WRITE("tRespTckNo.qr_pym_pyn_dtl_cd : %s  ", CCancRyTkMem::GetInstance()->tRespTckNo.qr_pym_pyn_dtl_cd);		// 20221220 ADD	

		nRet = TMaxFBGetF(RPUB_NUM			,	PST_FIELD(pRPacket, rpub_num			));	///< 재발행매수		
		nCount = *(int *)pRPacket->rpub_num;
		if( nCount > 0 )
		{	
			pRPacket->pList = new rtk_readbustckno_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readbustckno_list_t) * nCount);

			CCancRyTkMem::GetInstance()->m_vtRespTckNoList.clear();
			for(i = 0; i < nCount; i++)
			{
				pList = &pRPacket->pList[i];

				nRet = TMaxFBGetF(RPUB_DT		,	PST_FIELD(pList, rpub_dt		));	///< 재발행일자	
				nRet = TMaxFBGetF(RPUB_TIME		,	PST_FIELD(pList, rpub_time		));	///< 재발행시각		
				nRet = TMaxFBGetF(RPUB_USER_ID	,	PST_FIELD(pList, rpub_user_id	));	///< 재발행사용자id

				CCancRyTkMem::GetInstance()->m_vtRespTckNoList.push_back(*pList);
			}
		}
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ", "RSP_CD",					PST_FIELD(pRPacket, rsp_cd				));	///< 응답코드
		LOG_WRITE("%30s - (%s) ", "ROT_KND_CD",				PST_FIELD(pRPacket, rot_knd_cd			));	///< 노선종류코드		
		LOG_WRITE("%30s - (%s) ", "PYN_MNS_DVS_CD",			PST_FIELD(pRPacket, pyn_mns_dvs_cd		));	///< 지불수단구분코드	
		LOG_WRITE("%30s - (%s) ", "ROT_ID",					PST_FIELD(pRPacket, rot_id				));	///< 노선id		
		LOG_WRITE("%30s - (%s) ", "ROT_NM",					PST_FIELD(pRPacket, rot_nm				));	///< 노선명			
		LOG_WRITE("%30s - (number = %d) ", "ROT_SQNO",		*(int *) pRPacket->rot_sqno				);	///< 노선순번		
		LOG_WRITE("%30s - (%s) ", "ALCN_DT",				PST_FIELD(pRPacket, alcn_dt				));	///< 배차일자		
		LOG_WRITE("%30s - (number = %d) ", "ALCN_SQNO",		*(int *) pRPacket->alcn_sqno			);	///< 배차순번		
		LOG_WRITE("%30s - (%s) ", "PUB_TIME",				PST_FIELD(pRPacket, pub_time			));	///< 발행시각		
		LOG_WRITE("%30s - (%s) ", "BUS_TCK_INHR_NO",		PST_FIELD(pRPacket, bus_tck_inhr_no		));	///< 버스티켓고유번호	
		LOG_WRITE("%30s - (number = %d) ", "TISU_AMT",		*(int *) pRPacket->tisu_amt				);	///< 발권금액		
		LOG_WRITE("%30s - (number = %d) ", "SATS_NO",		*(int *) pRPacket->sats_no				);	///< 좌석번호		
		LOG_WRITE("%30s - (%s) ", "DEPR_DT",				PST_FIELD(pRPacket, depr_dt				));	///< 출발일자		
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME",				PST_FIELD(pRPacket, depr_time			));	///< 출발시각		
		LOG_WRITE("%30s - (%s) ", "DEPR_TRML_CD",			PST_FIELD(pRPacket, depr_trml_cd		));	///< 출발터미널코드	
		LOG_WRITE("%30s - (%s) ", "ARVL_TRML_CD",			PST_FIELD(pRPacket, arvl_trml_cd		));	///< 도착터미널코드	
		LOG_WRITE("%30s - (%s) ", "STPT_TRML_CD",			PST_FIELD(pRPacket, stpt_trml_cd		));	///< 기점터미널코드	
		LOG_WRITE("%30s - (%s) ", "EPT_TRML_CD",			PST_FIELD(pRPacket, ept_trml_cd			));	///< 종점터미널코드	
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD",				PST_FIELD(pRPacket, bus_cls_cd			));	///< 버스등급코드		
		LOG_WRITE("%30s - (%s) ", "BUS_CACM_CD",			PST_FIELD(pRPacket, bus_cacm_cd			));	///< 버스운수사코드	
		LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD",			PST_FIELD(pRPacket, bus_tck_knd_cd		));	///< 버스티켓종류코드	
		LOG_WRITE("%30s - (%s) ", "ALCN_WAY_DVS_CD",		PST_FIELD(pRPacket, alcn_way_dvs_cd		));	///< 배차방식구분코드	
		LOG_WRITE("%30s - (%s) ", "SATI_USE_YN",			PST_FIELD(pRPacket, sati_use_yn			));	///< 좌석제사용여부 // 20221201 ADD	
		LOG_WRITE("%30s - (%s) ", "TISU_STA_DVS_CD",		PST_FIELD(pRPacket, tisu_sta_dvs_cd		));	///< 발권상태구분코드	
		LOG_WRITE("%30s - (%s) ", "PERD_TEMP_YN",			PST_FIELD(pRPacket, perd_temp_yn		));	///< 정기임시여부		
		LOG_WRITE("%30s - (%s) ", "APRV_NO",				PST_FIELD(pRPacket, aprv_no				));	///< 승인번호		
		LOG_WRITE("%30s - (%s) ", "CSRC_AUTH_NO",			PST_FIELD(pRPacket, csrc_auth_no		));	///< 현금영수증인증번호	
		LOG_WRITE("%30s - (%s) ", "CPRT_YN",				PST_FIELD(pRPacket, cprt_yn				));	///< 법인여부		
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ", "CARD_NO",				PST_FIELD(pRPacket, card_no				));	///< 카드번호		
#endif
		LOG_WRITE("%30s - (%s) ", "PUB_USER_ID",			PST_FIELD(pRPacket, pub_user_id			));	///< 발행사용자id	
		LOG_WRITE("%30s - (%s) ", "CANC_RY_DVS_CD",			PST_FIELD(pRPacket, canc_ry_dvs_cd		));	///< 취소환불구분코드	
		LOG_WRITE("%30s - (%s) ", "RYRT_DVS_CD",			PST_FIELD(pRPacket, ryrt_dvs_cd			));	///< 환불율구분코드	
		LOG_WRITE("%30s - (%s) ", "CANC_RY_DT",				PST_FIELD(pRPacket, canc_ry_dt			));	///< 취소환불일자		
		LOG_WRITE("%30s - (%s) ", "CANC_RY_TIME",			PST_FIELD(pRPacket, canc_ry_time		));	///< 취소환불시각		
		LOG_WRITE("%30s - (number = %d) ", "CANC_RY_AMT",	*(int *) pRPacket->canc_ry_amt			);	///< 취소환불금액		
		LOG_WRITE("%30s - (%s) ", "CANC_RY_TRML_CD",		PST_FIELD(pRPacket, canc_ry_trml_cd		));	///< 취소환불터미널코드	
		LOG_WRITE("%30s - (%s) ", "CANC_RY_WND_NO",			PST_FIELD(pRPacket, canc_ry_wnd_no		));	///< 취소환불창구번호	
		LOG_WRITE("%30s - (%s) ", "CANC_RY_USER_ID",		PST_FIELD(pRPacket, canc_ry_user_id		));	///< 취소환불사용자id	
		LOG_WRITE("%30s - (%s) ", "SATS_REST_DT",			PST_FIELD(pRPacket, sats_rest_dt		));	///< 좌석복원일자		
		LOG_WRITE("%30s - (%s) ", "SATS_REST_TIME",			PST_FIELD(pRPacket, sats_rest_time		));	///< 좌석복원시각		
		LOG_WRITE("%30s - (%s) ", "SATS_REST_TRML_CD",		PST_FIELD(pRPacket, sats_rest_trml_cd	));	///< 좌석복원터미널코드	
		LOG_WRITE("%30s - (%s) ", "SATS_REST_WND_NO",		PST_FIELD(pRPacket, sats_rest_wnd_no	));	///< 좌석복원창구번호	
		LOG_WRITE("%30s - (%s) ", "SATS_REST_USER_ID",		PST_FIELD(pRPacket, sats_rest_user_id	));	///< 좌석복원사용자id	
		LOG_WRITE("%30s - (%s) ", "CTY_BUS_DC_KND_CD",		PST_FIELD(pRPacket, cty_bus_dc_knd_cd	));	///< 시외버스할인종류코드
		LOG_WRITE("%30s - (%s) ", "DCRT_DVS_CD",			PST_FIELD(pRPacket, dcrt_dvs_cd			));	///< 할인율구분코드	
		LOG_WRITE("%30s - (number = %d) ", "DC_BEF_AMT",	*(int *) pRPacket->dc_bef_amt			);	///< 할인이전금액		
		LOG_WRITE("%30s - (%s) ", "TISU_SYS_DVS_CD",		PST_FIELD(pRPacket, tisu_sys_dvs_cd		));	///< 발권시스템구분코드	
		LOG_WRITE("%30s - (%s) ", "PUB_SYS_DVS_CD",			PST_FIELD(pRPacket, pub_sys_dvs_cd		));	///< 발행시스템구분코드	
		LOG_WRITE("%30s - (%s) ", "EB_BUS_TCK_SNO",			PST_FIELD(pRPacket, eb_bus_tck_sno		));	///< eb버스티켓일련번호	
		LOG_WRITE("%30s - (%s) ", "MISS_ERR_YN",			PST_FIELD(pRPacket, miss_err_yn			));	///< 분실오류여부		
		LOG_WRITE("%30s - (number = %d) ", "RPUB_NUM",		*(int *)pRPacket->rpub_num				);	///< 재발행매수		

		nCount = *(int *) pRPacket->rpub_num;
		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);

			LOG_WRITE("%30s - (%s) ", "RPUB_DT",		PST_FIELD(pList, rpub_dt		));	///< 재발행일자	
			LOG_WRITE("%30s - (%s) ", "RPUB_TIME",		PST_FIELD(pList, rpub_time		));	///< 재발행시각	
			LOG_WRITE("%30s - (%s) ", "RPUB_USER_ID",	PST_FIELD(pList, rpub_user_id	));	///< 재발행사용자id
		}

		LOG_WRITE("%30s - (%s) ", "BRKP_TYPE",			PST_FIELD(pRPacket, brkp_type			));	///< 위약금 타입(BRKP_TYPE)					// 20221220 ADD	
		LOG_WRITE("%30s - (%s) ", "QR_PYM_PYN_DTL_CD",	PST_FIELD(pRPacket, qr_pym_pyn_dtl_cd	));	///< QR결제지불상세코드(QR_PYM_PYN_DTL_CD)	// 20221220 ADD	

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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc164
 * @details		(164) 창구마감
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc164(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ClosWnd";
	pstk_closwnd_t		pSPacket;
	prtk_closwnd_t		pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_closwnd_t) pData;
	pRPacket = (prtk_closwnd_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr);

		LOG_WRITE("%30s - (%s) ","PRS_BUS_TCK_INHR_NO",	PST_FIELD(pSPacket, prs_bus_tck_inhr_no));
		LOG_WRITE("%30s - (%s) ","ADD_BUS_TCK_INHR_NO",	PST_FIELD(pSPacket, add_bus_tck_inhr_no));
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr);

		nRet = TMaxFBPut(PRS_BUS_TCK_INHR_NO,	PST_FIELD(pSPacket, prs_bus_tck_inhr_no));
		nRet = TMaxFBPut(ADD_BUS_TCK_INHR_NO,	PST_FIELD(pSPacket, add_bus_tck_inhr_no));
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	///> recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 146 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(WND_END_DT	,	PST_FIELD(pRPacket, wnd_end_dt));
		nRet = TMaxFBGetF(WND_END_TIME	,	PST_FIELD(pRPacket, wnd_end_time));
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%.*s) ", "RSP_CD",		 sizeof(pRPacket->rsp_cd),		PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%.*s) ", "WND_END_DT",  sizeof(pRPacket->wnd_end_dt),	PST_FIELD(pRPacket, wnd_end_dt));
		LOG_WRITE("%30s - (%.*s) ", "WND_END_TIME",sizeof(pRPacket->wnd_end_time),PST_FIELD(pRPacket, wnd_end_time));
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc200
 * @details		(200) 시외버스 지역코드 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc200(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ReadAreaCd";
	pstk_readareacd_t		pSPacket;
	prtk_readareacd_t		pRPacket;
	prtk_readareacd_list_t	pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readareacd_t) pData;
	pRPacket = (prtk_readareacd_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
	}


	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD", PST_FIELD(pRPacket, rsp_cd	));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(CTY_BUS_AREA_CD_NUM	,	PST_FIELD(pRPacket, cty_bus_area_cd_num	));	///< 시외버스지역코드 수
		nCount = *(int *)pRPacket->cty_bus_area_cd_num;
		LOG_WRITE("%30s - (%d) ","CTY_BUS_AREA_CD_NUM", nCount);
		if(nCount <= 0)
		{	// count 에러
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_readareacd_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readareacd_list_t) * nCount);
		}

		OperGetFileName(OPER_FILE_ID_0200, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->cty_bus_area_cd_num, sizeof(pRPacket->cty_bus_area_cd_num));

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(CTY_BUS_AREA_CD	,	PST_FIELD(pList, cty_bus_area_cd	));	///< 시외버스지역코드			
			nRet = TMaxFBGetF(CTY_BUS_AREA_NM	,	PST_FIELD(pList, cty_bus_area_nm	));	///< 시외버스지역명	
			MyWriteFile(hFile, pList, sizeof(rtk_readareacd_list_t));
		}
		MyCloseFile(hFile);
	}

	///> recv log
	{
		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_AREA_CD", PST_FIELD(pList, cty_bus_area_cd));	///< 시외버스지역코드	
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_AREA_NM", PST_FIELD(pList, cty_bus_area_nm));	///< 시외버스지역명
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
 * @brief		TMaxSvc201
 * @details		(201) 왕복가능터미널 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc201(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ReadRtrpTrml";
	pstk_readrtrptrml_t		pSPacket;
	prtk_readrtrptrml_t		pRPacket;
	prtk_readrtrptrml_list_t	pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readrtrptrml_t) pData;
	pRPacket = (prtk_readrtrptrml_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
	}


	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	//LOG_HEXA("TMaxSvc201() SendPacket", (BYTE *)pSPacket, sizeof(stk_readrtrptrml_t));

	///> recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		nRet = TMaxFBGetF(RTRP_PSB_TRML_NUM		,	PST_FIELD(pRPacket, rtrp_psb_trml_num	));	///< 왕복가능터미널 수
		nCount = *(int *)pRPacket->rtrp_psb_trml_num;
		if(nCount > 0)
		{
			pRPacket->pList = new rtk_readrtrptrml_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readrtrptrml_list_t) * nCount);
		}

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(ARVL_TRML_CD	,	PST_FIELD(pList, arvl_trml_cd	));	///< 도착터미널코드			
		}
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ","RSP_CD", PST_FIELD(pRPacket, rsp_cd	));
		LOG_WRITE("%30s - (%d) ","RTRP_PSB_TRML_NUM", nCount);

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_CD", PST_FIELD(pList, arvl_trml_cd));	///< 도착터미널코드			
		}
	}

	///> data proc
	{
		nRet = 1;

		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc202
 * @details		(202) 좌석 선점 / 해제
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc202(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_PcpySats";
	pstk_pcpysats_t		pSPacket;
	prtk_pcpysats_t		pRPacket;
	prtk_pcpysats_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_pcpysats_t) pData;
	pRPacket = (prtk_pcpysats_t) retBuf;

	// 20221205 ADD~
	PUBTCK_T* pBase;
	pBase = &CPubTckMem::GetInstance()->base;

	PUI_BASE_T	pBaseInfo;
	pBaseInfo = (PUI_BASE_T) GetConfigBaseInfo();
	// 20221205 ~ADD
			
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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","SATS_PCPY_DVS",		PST_FIELD(pSPacket, sats_pcpy_dvs		));
		LOG_WRITE("%30s - (%s) ","ROT_ID",			PST_FIELD(pSPacket, rot_id				));
		LOG_WRITE("%30s - (number = %d) ","ROT_SQNO",	*(int *)pSPacket->rot_sqno				);
		LOG_WRITE("%30s - (%s) ","ALCN_DT",			PST_FIELD(pSPacket, alcn_dt				));
		LOG_WRITE("%30s - (number = %d) ","ALCN_SQNO", *(int *)pSPacket->alcn_sqno			);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_CD",		PST_FIELD(pSPacket, depr_trml_cd		));
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_CD",		PST_FIELD(pSPacket, arvl_trml_cd		));
		LOG_WRITE("%30s - (number = %d) ","SATS_PCPY_NUM", *(WORD *)pSPacket->sats_pcpy_num	);
		nCount = *(WORD *) pSPacket->sats_pcpy_num;
		for(i = 0; i < nCount; i++)
		{
			pstk_pcpysats_list_t pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);

			LOG_WRITE("%30s - (%s) ","SATS_PCPY_ID",		PST_FIELD(pList, sats_pcpy_id		));
			LOG_WRITE("%30s - (%s) ","CTY_BUS_DC_KND_CD",	PST_FIELD(pList, cty_bus_dc_knd_cd	));
			LOG_WRITE("%30s - (%s) ","DCRT_DVS_CD",		PST_FIELD(pList, dcrt_dvs_cd		));
			LOG_WRITE("%30s - (%s) ","RTRP_DEPR_DT",		PST_FIELD(pList, rtrp_depr_dt		));
			LOG_WRITE("%30s - (%s) ","BUS_TCK_KND_CD",	PST_FIELD(pList, bus_tck_knd_cd		));
			LOG_WRITE("%30s - (number = %d) ","SATS_NO",	*(int *) pList->sats_no		);
		}
	}

// 	nRet = -111;
// 	LOG_WRITE("Test_error(), err !!!!");
// 	goto fail_proc;

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(SATS_PCPY_DVS		,	PST_FIELD(pSPacket, sats_pcpy_dvs		));
		nRet = TMaxFBPut(ROT_ID				,	PST_FIELD(pSPacket, rot_id				));
		nRet = TMaxFBPut(ROT_SQNO			,	PST_FIELD(pSPacket, rot_sqno			));
		nRet = TMaxFBPut(ALCN_DT			,	PST_FIELD(pSPacket, alcn_dt				));
		nRet = TMaxFBPut(ALCN_SQNO			,	PST_FIELD(pSPacket, alcn_sqno			));
		nRet = TMaxFBPut(DEPR_TRML_CD		,	PST_FIELD(pSPacket, depr_trml_cd		));
		nRet = TMaxFBPut(ARVL_TRML_CD		,	PST_FIELD(pSPacket, arvl_trml_cd		));
		// nRet = TMaxFBPut(SATS_PCPY_NUM		,	PST_FIELD(pSPacket, sats_pcpy_num		));
		int n_sats_pcpy_num = *(short *)(pSPacket->sats_pcpy_num);
		fbput(m_pSendBuff, (FLDKEY) SATS_PCPY_NUM, (char *)&n_sats_pcpy_num, 2);

		nCount = *(WORD *) pSPacket->sats_pcpy_num;
		for(i = 0; i < nCount; i++)
		{
			int n_sats_no;

			pstk_pcpysats_list_t pList;

			pList = &pSPacket->List[i];
		
			LOG_WRITE("SATS_PCPY_DVS - sats_pcpy_dvs(%s), len(%d) !!!!", pSPacket->sats_pcpy_dvs, sizeof(pSPacket->sats_pcpy_dvs) - 1);
			if(pSPacket->sats_pcpy_dvs[0] == '2')
			{	/// 선점 해제시에만 보냄.
				nRet = TMaxFBInsert(SATS_PCPY_ID	,	pList->sats_pcpy_id,		i, sizeof(pList->sats_pcpy_id) - 1);
			}

			LOG_WRITE("CTY_BUS_DC_KND_CD - cty_bus_dc_knd_cd(%s), len(%d) !!!!", pList->cty_bus_dc_knd_cd, sizeof(pList->cty_bus_dc_knd_cd) - 1);

			nRet = TMaxFBInsert(CTY_BUS_DC_KND_CD	,	pList->cty_bus_dc_knd_cd,	i, sizeof(pList->cty_bus_dc_knd_cd) - 1);
			nRet = TMaxFBInsert(DCRT_DVS_CD			,	pList->dcrt_dvs_cd,			i, sizeof(pList->dcrt_dvs_cd) - 1);
			nRet = TMaxFBInsert(RTRP_DEPR_DT		,	pList->rtrp_depr_dt,		i, sizeof(pList->rtrp_depr_dt) - 1);
 			nRet = TMaxFBInsert(BUS_TCK_KND_CD		,	pList->bus_tck_knd_cd,		i, sizeof(pList->bus_tck_knd_cd) - 1);
 			n_sats_no = *(int *)pList->sats_no;
 			nRet = TMaxFBInsert(SATS_NO				,	(char *)&n_sats_no,			i, sizeof(pList->sats_no) - 1);

// 			nRet = TMaxFBInsert(CTY_BUS_DC_KND_CD	,	pList->cty_bus_dc_knd_cd,	i, 0);
// 			nRet = TMaxFBInsert(BUS_TCK_KND_CD		,	pList->bus_tck_knd_cd,		i, 0);
// 			nRet = TMaxFBInsert(DCRT_DVS_CD			,	pList->dcrt_dvs_cd,			i, 0);
// 			nRet = TMaxFBInsert(RTRP_DEPR_DT		,	pList->rtrp_depr_dt,		i, 0);
// 			n_sats_no = *(int *)pList->sats_no;
// 			nRet = TMaxFBInsert(SATS_NO				,	(char *)&n_sats_no,			i, 0);
		}
	}

// 	nRet = -3;
// 	LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
// 	goto fail_proc;


	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	///> recv data
	{
		CPubTckMem* pTr;

		// 현장발권 mem
		pTr = CPubTckMem::GetInstance();

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 146 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(SATS_PCPY_NUM,	PST_FIELD(pRPacket, sats_pcpy_num));
		nCount = *(WORD *)pRPacket->sats_pcpy_num;
		LOG_WRITE("%30s - (%d) ","SATS_PCPY_NUM", nCount);
		if(nCount <= 0)
		{
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}
		
		{
			pRPacket->pList = new rtk_pcpysats_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_pcpysats_list_t) * nCount);
		}

		if(pSPacket->sats_pcpy_dvs[0] == '1')
		{	/// 좌석 선점
			pTr->m_vtPcpysats.clear();
			pTr->n_pcpysats_num = nCount;
		}

		pTr->m_vtPcpysats.clear();
		for(i = 0; i < nCount; i++)
		{
			prtk_pcpysats_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(SATS_PCPY_ID		,	PST_FIELD(pList, sats_pcpy_id		));
			nRet = TMaxFBGetF(CTY_BUS_DC_KND_CD	,	PST_FIELD(pList, cty_bus_dc_knd_cd	));
			nRet = TMaxFBGetF(DCRT_DVS_CD		,	PST_FIELD(pList, dcrt_dvs_cd		));
			nRet = TMaxFBGetF(BUS_TCK_KND_CD	,	PST_FIELD(pList, bus_tck_knd_cd		));
			nRet = TMaxFBGetF(DC_AFT_AMT		,	PST_FIELD(pList, dc_aft_amt			));
			nRet = TMaxFBGetF(SATS_NO			,	PST_FIELD(pList, sats_no			));

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 20221205 ADD~
			// 비배차인 경우 dc_aft_amt 값이 없고 향후 '좌석선점 서비스(TK_PcpySats)'를 호출하지 않도록 수정 필요
			LOG_WRITE("할인이후금액(pRPacket->pList[i].dc_aft_amt) = %d",	*(int *) pRPacket->pList[i].dc_aft_amt);
			LOG_WRITE("배차방식(pBase->alcn_way_dvs_cd) = %s",			pBase->alcn_way_dvs_cd);
			if( ( pBase->alcn_way_dvs_cd[0] == 'N' )				// 배차('D'), 비배차('N')
				&& (*(int *) pRPacket->pList[i].dc_aft_amt == 0) )	// 할인이후금액 == 0 인 경우 
			{	/// 비배차 모드 일 경우
				LOG_WRITE("비배차-총 결제금액(pBase->nTotalMoney) = %d",			pBase->nTotalMoney); // 총 결제금액

				::CopyMemory(&pRPacket->pList[i].dc_aft_amt, &(pBase->nTotalMoney), sizeof(WORD));

				LOG_WRITE("비배차-총 결제금액(pRPacket->pList[i].dc_aft_amt) = %d",	*(int *) pRPacket->pList[i].dc_aft_amt);
			}
			// 20221205 ~ADD
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			if(pSPacket->sats_pcpy_dvs[0] == '1')
			{	/// 좌석 선점
				pTr->m_vtPcpysats.push_back(*pList);
			}
		}
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ","RSP_CD", PST_FIELD(pRPacket, rsp_cd	));
		LOG_WRITE("%30s - (%d) ","SATS_PCPY_NUM", nCount);

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "SATS_PCPY_ID",			PST_FIELD(pList, sats_pcpy_id		));
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_DC_KND_CD",	PST_FIELD(pList, cty_bus_dc_knd_cd	));
			LOG_WRITE("%30s - (%s) ", "DCRT_DVS_CD",			PST_FIELD(pList, dcrt_dvs_cd		));
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD",		PST_FIELD(pList, bus_tck_knd_cd		));
			LOG_WRITE("%30s - (number = %d) ", "DC_AFT_AMT",	*(int *) pRPacket->pList[i].dc_aft_amt);
			LOG_WRITE("%30s - (number = %d) ", "SATS_NO",		*(int *) pRPacket->pList[i].sats_no);
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc278
 * @details		(278) QR 좌석 선점 변경
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc278(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_QrMdPcpySats";
	pstk_qrmdpcpysats_t		pSPacket;
	prtk_qrmdpcpysats_t		pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_qrmdpcpysats_t) pData;
	pRPacket = (prtk_qrmdpcpysats_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (number = %d) ","SATS_PCPY_NUM", *(WORD *)pSPacket->sats_pcpy_num	);
		nCount = *(WORD *) pSPacket->sats_pcpy_num;
		for(i = 0; i < nCount; i++)
		{
			pstk_qrmdpcpysats_list_t pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ","SATS_PCPY_ID",		PST_FIELD(pList, sats_pcpy_id		));
		}
	}

// 	nRet = -111;
// 	LOG_WRITE("Test_error(), err !!!!");
// 	goto fail_proc;

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		// nRet = TMaxFBPut(SATS_PCPY_NUM		,	PST_FIELD(pSPacket, sats_pcpy_num		));
		int n_sats_pcpy_num = *(short *)(pSPacket->sats_pcpy_num);
		fbput(m_pSendBuff, (FLDKEY) SATS_PCPY_NUM, (char *)&n_sats_pcpy_num, 2);

		nCount = *(WORD *) pSPacket->sats_pcpy_num;
		for(i = 0; i < nCount; i++)
		{
			int n_sats_no;

			pstk_qrmdpcpysats_list_t pList;

			pList = &pSPacket->List[i];
		
			nRet = TMaxFBInsert(SATS_PCPY_ID	,	pList->sats_pcpy_id,		i, sizeof(pList->sats_pcpy_id) - 1);
		}
	}

// 	nRet = -3;
// 	LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
// 	goto fail_proc;

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	///> recv data
	{
		CPubTckMem* pTr;

		// 현장발권 mem
		pTr = CPubTckMem::GetInstance();

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 146 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ","RSP_CD", PST_FIELD(pRPacket, rsp_cd	));
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc208
 * @details		(208) 시외버스 메시지코드 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc208(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ReadCBusMsg";
	pstk_readcbusmsg_t	pSPacket;
	prtk_readcbusmsg_t	pRPacket;
	prtk_readcbusmsg_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readcbusmsg_t) pData;
	pRPacket = (prtk_readcbusmsg_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -98;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
		nRet = TMaxFBGetF(MSG_NUM,	PST_FIELD(pRPacket, msg_num));
		::CopyMemory(&nCount, pRPacket->msg_num, 4);
		LOG_WRITE("%30s - (%d) ", "MSG_NUM",	nCount);
		if(nCount <= 0)
		{
			nRet = -99;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		OperGetFileName(OPER_FILE_ID_0208, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->msg_num, sizeof(pRPacket->msg_num));

		{
			pRPacket->pList = new rtk_readcbusmsg_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readcbusmsg_list_t) * nCount);
		}

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(CTY_BUS_MSG_CD,	PST_FIELD(pList, cty_bus_msg_cd	));
			nRet = TMaxFBGetF(CTY_BUS_MSG_NM,	PST_FIELD(pList, cty_bus_msg_nm	));
			nRet = TMaxFBGetF(LNG_DVS_CD,		PST_FIELD(pList, lng_dvs_cd		));

			MyWriteFile(hFile, pList, sizeof(rtk_readcbusmsg_list_t));
			DB_AddBusMsgData((char *)pList);
		}
		MyCloseFile(hFile);
	}

	///> recv Log 
#ifdef __USE_LOG__ 
	{
		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_MSG_CD", PST_FIELD(pList, cty_bus_msg_cd));
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_MSG_NM", PST_FIELD(pList, cty_bus_msg_nm));
			LOG_WRITE("%30s - (%s) ", "LNG_DVS_CD",	  PST_FIELD(pList, lng_dvs_cd));
		}
	}
#endif

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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc209
 * @details		(209) 시외버스 메시지코드 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc209(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ReadPtrgTck";
	pstk_readptrgtck_t	pSPacket;
	prtk_readptrgtck_t	pRPacket;
	prtk_readptrgtck_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readptrgtck_t) pData;
	pRPacket = (prtk_readptrgtck_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","CTY_BUS_PTR_KND_CD",PST_FIELD(pSPacket, cty_bus_ptr_knd_cd));
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(CTY_BUS_PTR_KND_CD	,	PST_FIELD(pSPacket, cty_bus_ptr_knd_cd	));
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

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(READ_DTA_NUM,	PST_FIELD(pRPacket, read_dta_num));
		::CopyMemory(&nCount, pRPacket->read_dta_num, 4);
		if( nCount <= 0 )
		{	// 에러코드 조회
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_readptrgtck_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readptrgtck_list_t) * nCount);
		}

		OperGetFileName(OPER_FILE_ID_0209, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->read_dta_num, sizeof(pRPacket->read_dta_num));

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(BUS_TCK_TYP_CD		,	PST_FIELD(pList, bus_tck_typ_cd			));
			nRet = TMaxFBGetF(CTY_BUS_PTR_KND_CD	,	PST_FIELD(pList, cty_bus_ptr_knd_cd		));
			nRet = TMaxFBGetF(CTY_BUS_PTRG_USG_CD	,	PST_FIELD(pList, cty_bus_ptrg_usg_cd	));
			nRet = TMaxFBGetF(CTY_BUS_PTRG_ATC_CD	,	PST_FIELD(pList, cty_bus_ptrg_atc_cd	));
			nRet = TMaxFBGetF(PTRG_X_CRDN_VAL		,	PST_FIELD(pList, ptrg_x_crdn_val		));
			nRet = TMaxFBGetF(PTRG_Y_CRDN_VAL		,	PST_FIELD(pList, ptrg_y_crdn_val		));
			nRet = TMaxFBGetF(PTRG_MGNF_CD			,	PST_FIELD(pList, ptrg_mgnf_cd			));

			MyWriteFile(hFile, pList, sizeof(rtk_readptrgtck_list_t));
		}
		MyCloseFile(hFile);
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ","RSP_CD",		PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%d) ","READ_DTA_NUM",	nCount);
		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_TYP_CD",		PST_FIELD(pList, bus_tck_typ_cd			));
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_PTR_KND_CD",	PST_FIELD(pList, cty_bus_ptr_knd_cd		));
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_PTRG_USG_CD",	PST_FIELD(pList, cty_bus_ptrg_usg_cd	));
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_PTRG_ATC_CD",	PST_FIELD(pList, cty_bus_ptrg_atc_cd	));
			LOG_WRITE("%30s - (%s) ", "PTRG_X_CRDN_VAL",		PST_FIELD(pList, ptrg_x_crdn_val		));
			LOG_WRITE("%30s - (%s) ", "PTRG_Y_CRDN_VAL",		PST_FIELD(pList, ptrg_y_crdn_val		));
			LOG_WRITE("%30s - (%s) ", "PTRG_MGNF_CD",			PST_FIELD(pList, ptrg_mgnf_cd			));
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
 * @brief		TMaxSvc213
 * @details		(213) 무인 관리자 로그인
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc213(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, nTimeout;
//	int n_rsp_cd;
	char*	pService = "TK_LoginKosMgr";
	pstk_loginkosmgr_t	pSPacket;
	prtk_loginkosmgr_t	pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_loginkosmgr_t) pData;
	pRPacket = (prtk_loginkosmgr_t) retBuf;

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
		LogOut_Hdr((char *)&pSPacket->hdr);

		LOG_WRITE("%30s - (%s) ","USER_NO",			PST_FIELD(pSPacket, user_no));
		LOG_WRITE("%30s - (%s) ","REQ_TRML_USER_PWD",	PST_FIELD(pSPacket, req_trml_user_pwd));
	}

	// send data
	{
		SendHeader((char *)&pSPacket->hdr);

		nRet = TMaxFBPut(USER_NO			,	PST_FIELD(pSPacket, user_no				));	///< 사용자번호			
		nRet = TMaxFBPut(REQ_TRML_USER_PWD	,	PST_FIELD(pSPacket, req_trml_user_pwd	));	///< 요청 터미널 사용자비밀번호
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	LOG_WRITE("TMaxTpCall(), Success !!!!");

	///> recv data
	{
		int n_rsp_cd;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
	}

	nRet = 1;
	///> recv log
	{
		LOG_WRITE("%30s - (%s) ", "RSP_CD", PST_FIELD(pRPacket, rsp_cd	));
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc217
 * @details		(217) 발매제한상세
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc217(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_TisuLtnDtl";
	pstk_tisultndtl_t	pSPacket;
	prtk_tisultndtl_t	pRPacket;
	prtk_tisultndtl_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_tisultndtl_t) pData;
	pRPacket = (prtk_tisultndtl_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			//CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(RPTN_NUM	,	PST_FIELD(pRPacket, rptn_num));
		nCount = *(int *)pRPacket->rptn_num;
		if( nCount <= 0 )
		{	// 에러코드 조회
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			//CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_tisultndtl_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tisultndtl_list_t) * nCount);
		}

		OperGetFileName(OPER_FILE_ID_0217, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->rptn_num, sizeof(pRPacket->rptn_num));

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(ROT_ID		,	PST_FIELD(pList, rot_id			));	 ///> 노선id			
			nRet = TMaxFBGetF(DEPR_TRML_CD	,	PST_FIELD(pList, depr_trml_cd	));	 ///> 출발터미널코드	
			nRet = TMaxFBGetF(ARVL_TRML_CD	,	PST_FIELD(pList, arvl_trml_cd	));	 ///> 도착터미널코드	
			nRet = TMaxFBGetF(ADPT_STT_DT	,	PST_FIELD(pList, adpt_stt_dt	));	 ///> 적용시작일자	
			nRet = TMaxFBGetF(ADPT_STT_TIME	,	PST_FIELD(pList, adpt_stt_time	));	 ///> 적용시작시각	
			nRet = TMaxFBGetF(ADPT_END_DT	,	PST_FIELD(pList, adpt_end_dt	));	 ///> 적용종료일자	
			nRet = TMaxFBGetF(ADPT_END_TIME	,	PST_FIELD(pList, adpt_end_time	));	 ///> 적용종료시각	

			MyWriteFile(hFile, pList, sizeof(rtk_tisultndtl_list_t));
		}
		MyCloseFile(hFile);
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ","RSP_CD", PST_FIELD(pRPacket, rsp_cd	));
		LOG_WRITE("%30s - (%d) ","RPTN_NUM", nCount);

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "ROT_ID",			PST_FIELD(pList, rot_id			));	///> 노선id			
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_CD",	PST_FIELD(pList, depr_trml_cd	));	///> 출발터미널코드	
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_CD",	PST_FIELD(pList, arvl_trml_cd	));	///> 도착터미널코드	
			LOG_WRITE("%30s - (%s) ", "ADPT_STT_DT",	PST_FIELD(pList, adpt_stt_dt	));	///> 적용시작일자	
			LOG_WRITE("%30s - (%s) ", "ADPT_STT_TIME",	PST_FIELD(pList, adpt_stt_time	));	///> 적용시작시각	
			LOG_WRITE("%30s - (%s) ", "ADPT_END_DT",	PST_FIELD(pList, adpt_end_dt	));	///> 적용종료일자	
			LOG_WRITE("%30s - (%s) ", "ADPT_END_TIME",	PST_FIELD(pList, adpt_end_time	));	///> 적용종료시각	
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc268
 * @details		(268) 도착터미널 키워드 매핑 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc268(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_ReadTrmlKwd";
	pstk_readtrmlkwd_t	pSPacket;
	prtk_readtrmlkwd_t	pRPacket;
	prtk_readtrmlkwd_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_readtrmlkwd_t) pData;
	pRPacket = (prtk_readtrmlkwd_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		goto fail_proc;
	}

	///> recv data
	{
		HANDLE hFile;
		CString strFullName;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			//CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(TRML_CD_NUM	,	PST_FIELD(pRPacket, trml_cd_num));
		nCount = *(int *)pRPacket->trml_cd_num;
		if( nCount <= 0 )
		{	// 에러코드 조회
			nRet = -98;
			LOG_WRITE("%30s - (%d) ERROR !!!!", "COUNT", nCount);
			//CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_readtrmlkwd_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_readtrmlkwd_list_t) * nCount);
		}

		OperGetFileName(OPER_FILE_ID_0268, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));
		MyWriteFile(hFile, pRPacket->trml_cd_num, sizeof(pRPacket->trml_cd_num));

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(ARVL_TRML_CD		,	PST_FIELD(pList, arvl_trml_cd		));	 ///> 도착터미널 코드
			nRet = TMaxFBGetF(SHCT_ARVL_TRML_CD	,	PST_FIELD(pList, shct_arvl_trml_cd	));	 ///> 단축 도착터미널 코드
			nRet = TMaxFBGetF(KWD_SQNO			,	PST_FIELD(pList, kwd_sqno			));	 ///> (int)키워드 순번
			nRet = TMaxFBGetF(KWD_NM			,	PST_FIELD(pList, kwd_nm				));	 ///> 키워드 이름

			MyWriteFile(hFile, pList, sizeof(rtk_readtrmlkwd_list_t));
		}
		MyCloseFile(hFile);
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ","RSP_CD"		, PST_FIELD(pRPacket, rsp_cd	));
		LOG_WRITE("%30s - (%d) ","TRML_CD_NUM"	, nCount);

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_CD"		,	PST_FIELD(pList, arvl_trml_cd		));	 ///> 도착터미널 코드
			LOG_WRITE("%30s - (%s) ", "SHCT_ARVL_TRML_CD"	,	PST_FIELD(pList, shct_arvl_trml_cd	));	 ///> 단축 도착터미널 코드
			LOG_WRITE("%30s - (%s) ", "KWD_SQNO"			,	PST_FIELD(pList, kwd_sqno			));	 ///> (int)키워드 순번
			LOG_WRITE("%30s - (%s) ", "KWD_NM"				,	PST_FIELD(pList, kwd_nm				));	 ///> 키워드 이름
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc221
 * @details		(221) 승차권 발행 - 카드현장(KTC인증)
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc221(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_KtcPbTckCard";
	pstk_ktcpbtckcard_t		pSPacket;
	prtk_ktcpbtckcard_t		pRPacket;
	prtk_ktcpbtckcard_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_ktcpbtckcard_t) pData;
	pRPacket = (prtk_ktcpbtckcard_t) retBuf;

	m_bVerify = FALSE;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","READ_ROT_YN",			PST_FIELD(pSPacket, read_rot_yn			));
		LOG_WRITE("%30s - (%s) ","ROT_ID",				PST_FIELD(pSPacket, rot_id				));
		LOG_WRITE("%30s - (number = %d) ","ROT_SQNO",	*(int *)pSPacket->rot_sqno				);
		LOG_WRITE("%30s - (%s) ","ALCN_DT",				PST_FIELD(pSPacket, alcn_dt				));
		LOG_WRITE("%30s - (number = %d) ","ALCN_SQNO",	*(int *)pSPacket->alcn_sqno				);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_CD",		PST_FIELD(pSPacket, depr_trml_cd		));
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_CD",		PST_FIELD(pSPacket, arvl_trml_cd		));
		LOG_WRITE("%30s - (%s) ","PYN_MNS_DVS_CD",		PST_FIELD(pSPacket, pyn_mns_dvs_cd		));
		LOG_WRITE("%30s - (%s) ","PUB_CHNL_DVS_CD",		PST_FIELD(pSPacket, pub_chnl_dvs_cd		));
		LOG_WRITE("%30s - (%s) ","TISU_WAY_DVS_CD",		PST_FIELD(pSPacket, tisu_way_dvs_cd		));
		
		LOG_WRITE("%30s - (%s) ","TRD_DVS_CD",			PST_FIELD(pSPacket, trd_dvs_cd			));
		LOG_WRITE("%30s - (%s) ","FALLBACK_DVS_CD",		PST_FIELD(pSPacket, fallback_dvs_cd		));
		LOG_WRITE("%30s - (%s) ","UNION_PAY_YN",		PST_FIELD(pSPacket, union_pay_yn		));
		LOG_WRITE("%30s - (%s) ","POS_PG_KTC_VER",		PST_FIELD(pSPacket, pos_pg_ktc_ver		));
		LOG_WRITE("%30s - (number = %d) ","ENC_DTA_LEN",*(int *)pSPacket->enc_dta_len);

#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ","ENC_DTA",			PST_FIELD(pSPacket, enc_dta					));
		LOG_WRITE("%30s - (%s) ","EMV_DTA",			PST_FIELD(pSPacket, emv_dta					));
#endif
		LOG_WRITE("%30s - (number = %d) ","MIP_TERM",	*(WORD *) pSPacket->mip_term			);
		LOG_WRITE("%30s - (%s) ","SPAD_DTA",			PST_FIELD(pSPacket, spad_dta			));
		LOG_WRITE("%30s - (%s) ","SPAD_DTA_LEN",		pSPacket->spad_dta_len					);
		LOG_WRITE("%30s - (%s) ","USER_DVS_NO",			PST_FIELD(pSPacket, user_dvs_no			));
		LOG_WRITE("%30s - (%s) ","OLD_MBRS_NO",			PST_FIELD(pSPacket, old_mbrs_no			));
		LOG_WRITE("%30s - (%s) ","RF_TRCR_DVS_CD",		PST_FIELD(pSPacket, rf_trcr_dvs_cd		));

		LOG_WRITE("%30s - (number = %d) ","PUB_NUM",	*(int *)pSPacket->pub_num				);
		nCount = *(int *) pSPacket->pub_num;
		for(i = 0; i < nCount; i++)
		{
			pstk_ktcpbtckcard_list_t pSList;

			pSList = &pSPacket->List[i];

			LOG_WRITE("순번 (%d / %d)", i + 1, nCount);

			LOG_WRITE("%30s - (%s) ",			"BUS_TCK_KND_CD",	PST_FIELD(pSList, bus_tck_knd_cd));
			LOG_WRITE("%30s - (%s) ",			"SATS_PCPY_ID",		PST_FIELD(pSList, sats_pcpy_id	));
			LOG_WRITE("%30s - (number = %d) ",	"SATS_NO",			*(int *)pSList->sats_no			);
			LOG_WRITE("%30s - (number = %d) ",	"MRNP_NUM",			*(int *)pSList->mrnp_num		);
			LOG_WRITE("%30s - (%s) ",			"MRNP_ID",			PST_FIELD(pSList, mrnp_id		));
		}
	}

	// send data
	{
		int nEncLen;
		int rndLen, nRet, nOffset;
		char rndString[100];
		char retString[100];
		int n_rot_sqno;
		int n_alcn_sqno;
		int n_enc_dta_len;
		int n_mrnp_num;
		int n_mip_term;
		int n_pub_num;
//		int n_sat_no;

		rndLen = nRet = nOffset = 0;

		SendIDHeader((char *)&pSPacket->hdr_id);

		n_rot_sqno		= *(int *) pSPacket->rot_sqno;
		n_alcn_sqno		= *(int *) pSPacket->alcn_sqno;
		n_enc_dta_len	= *(int *) pSPacket->enc_dta_len;
		n_mrnp_num		= *(int *) pSPacket->List[0].mrnp_num;
		n_mip_term		= *(WORD *) pSPacket->mip_term;
		n_pub_num		= *(int *) pSPacket->pub_num;

		nRet = TMaxFBPut(READ_ROT_YN		,	PST_FIELD(pSPacket, read_rot_yn			));
		nRet = TMaxFBPut(ROT_ID				,	PST_FIELD(pSPacket, rot_id				));
		nRet = TMaxFBPutLen(ROT_SQNO		,	PST_FIELD(pSPacket, rot_sqno),			5);
//		nRet = TMaxFBPutLen(ROT_SQNO		,	(char *) &n_rot_sqno, 5);
		nRet = TMaxFBPut(ALCN_DT			,	PST_FIELD(pSPacket, alcn_dt				));
		nRet = TMaxFBPutLen(ALCN_SQNO		,	PST_FIELD(pSPacket, alcn_sqno),			5);
		//nRet = TMaxFBPutLen(ALCN_SQNO		,	(char *) &n_alcn_sqno, 5);
		nRet = TMaxFBPut(DEPR_TRML_CD		,	PST_FIELD(pSPacket, depr_trml_cd		));
		nRet = TMaxFBPut(ARVL_TRML_CD		,	PST_FIELD(pSPacket, arvl_trml_cd		));
		nRet = TMaxFBPut(PYN_MNS_DVS_CD		,	PST_FIELD(pSPacket, pyn_mns_dvs_cd		));
		nRet = TMaxFBPut(PUB_CHNL_DVS_CD	,	PST_FIELD(pSPacket, pub_chnl_dvs_cd		));
		nRet = TMaxFBPut(TISU_WAY_DVS_CD	,	PST_FIELD(pSPacket, tisu_way_dvs_cd		));
		nRet = TMaxFBPut(TRD_DVS_CD			,	PST_FIELD(pSPacket, trd_dvs_cd			));
		nRet = TMaxFBPut(FALLBACK_DVS_CD	,	PST_FIELD(pSPacket, fallback_dvs_cd		));
		//nRet = TMaxFBPut(FALLBACK_DVS_CD	,	" ");
		nRet = TMaxFBPut(UNION_PAY_YN		,	PST_FIELD(pSPacket, union_pay_yn		));
		nRet = TMaxFBPut(POS_PG_KTC_VER		,	PST_FIELD(pSPacket, pos_pg_ktc_ver		));
		nRet = TMaxFBPutLen(ENC_DTA_LEN		,	PST_FIELD(pSPacket, enc_dta_len),		4);
		//nRet = TMaxFBPutLen(ENC_DTA_LEN		,	(char *) &n_enc_dta_len, 4);

		nEncLen = *(int *) pSPacket->enc_dta_len;

		/// enc_dta
		for(i = 0; i < nEncLen; i++)
		{
			nOffset = 0;
			::ZeroMemory(rndString, sizeof(rndString));
			::ZeroMemory(retString, sizeof(retString));

			srand((unsigned int) i);
			rndLen = rand() % 5 + 2;
			nRet = GetRndChar(rndLen, rndString);

			retString[nOffset++] = rndString[0];
			retString[nOffset++] = pSPacket->enc_dta[i];
			::CopyMemory(&retString[nOffset], &rndString[1], rndLen - 1);

			nRet = TMaxFBInsert(ENC_DTA, retString, i, nOffset);

			::ZeroMemory(rndString, sizeof(rndString));
			::ZeroMemory(retString, sizeof(retString));
		}

		nRet = TMaxFBPut(EMV_DTA		,	PST_FIELD(pSPacket, emv_dta			));
		nRet = TMaxFBPutLen(MIP_TERM	,	(char *)&n_mip_term, 2				);

		nRet = TMaxFBPut(SPAD_DTA		,	PST_FIELD(pSPacket, spad_dta		));
		nRet = TMaxFBPut(SPAD_DTA_LEN	,	PST_FIELD(pSPacket, spad_dta_len	));
		nRet = TMaxFBPut(USER_DVS_NO	,	PST_FIELD(pSPacket, user_dvs_no		));
		nRet = TMaxFBPut(OLD_MBRS_NO	,	" ");
		nRet = TMaxFBPut(RF_TRCR_DVS_CD	,	PST_FIELD(pSPacket, rf_trcr_dvs_cd	));

		n_mrnp_num = 0;
//		nRet = TMaxFBPutLen(MRNP_NUM, (char*)&n_mrnp_num, 5);
		nRet = TMaxFBPutLen(MRNP_NUM, ST_FIELD(pSPacket->List[0], mrnp_num	), 5);
		nRet = TMaxFBPut(MRNP_ID, " ");

		nCount = *(int *) pSPacket->pub_num;
		nRet = TMaxFBPut(PUB_NUM, pSPacket->pub_num);
		for(i = 0; i < nCount; i++)
		{
			nRet = TMaxFBInsert(BUS_TCK_KND_CD, ST_FIELD(pSPacket->List[i], bus_tck_knd_cd	), i, sizeof(pSPacket->List[i].bus_tck_knd_cd) - 1);
			nRet = TMaxFBInsert(SATS_PCPY_ID,	ST_FIELD(pSPacket->List[i], sats_pcpy_id	), i, sizeof(pSPacket->List[i].sats_pcpy_id) - 1 );
			nRet = TMaxFBInsert(SATS_NO,		pSPacket->List[i].sats_no,	i, sizeof(pSPacket->List[i].sats_no) - 1);
		}

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(pSPacket->enc_dta, sizeof(pSPacket->enc_dta));
		KTC_MemClear(pSPacket->emv_dta, sizeof(pSPacket->emv_dta));
#endif
	}

	// 동기화 (Send/Recv)
	nTimeout = 10;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		CPubTckMem* pTr;

		// 현장발권 mem
		pTr = CPubTckMem::GetInstance();

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		nRet = TMaxFBGetF(RSP_MSG,	PST_FIELD(pRPacket, rsp_msg				));
		if( n_rsp_cd != 146 )
		{	// 에러코드 조회
			if( n_rsp_cd == 361 )
			{
				//20160530 김영준 예매와 발권을 같이 사용하는 case에서만 체크하도록 변경.

			}
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(CARD_NO				,	PST_FIELD(pRPacket, card_no				));
		nRet = TMaxFBGetF(CARD_APRV_NO			,	PST_FIELD(pRPacket, card_aprv_no		));
		nRet = TMaxFBGetF(CARD_APRV_AMT			,	PST_FIELD(pRPacket, card_aprv_amt		));
		nRet = TMaxFBGetF(APRV_AMT				,	PST_FIELD(pRPacket, aprv_amt			));
		nRet = TMaxFBGetF(CARD_APRV_VANC_CD		,	PST_FIELD(pRPacket, card_aprv_vanc_cd	));
		nRet = TMaxFBGetF(ISSU_CRCM_CD			,	PST_FIELD(pRPacket, issu_crcm_cd		));
		nRet = TMaxFBGetF(BUY_CRCM_CD			,	PST_FIELD(pRPacket, buy_crcm_cd			));
		nRet = TMaxFBGetF(FRC_CMRT				,	PST_FIELD(pRPacket, frc_cmrt			));
		nRet = TMaxFBGetF(ATL_DEPR_DT			,	PST_FIELD(pRPacket, atl_depr_dt			));
		nRet = TMaxFBGetF(ATL_DEPR_TIME			,	PST_FIELD(pRPacket, atl_depr_time		));
		nRet = TMaxFBGetF(DEPR_TRML_CD			,	PST_FIELD(pRPacket, depr_trml_cd		));
		nRet = TMaxFBGetF(ARVL_TRML_CD			,	PST_FIELD(pRPacket, arvl_trml_cd		));
		nRet = TMaxFBGetF(STPT_TRML_CD			,	PST_FIELD(pRPacket, stpt_trml_cd		));
		nRet = TMaxFBGetF(EPT_TRML_CD			,	PST_FIELD(pRPacket, ept_trml_cd			));
		nRet = TMaxFBGetF(BUS_CLS_CD			,	PST_FIELD(pRPacket, bus_cls_cd			));
		nRet = TMaxFBGetF(BUS_CACM_CD			,	PST_FIELD(pRPacket, bus_cacm_cd			));
		nRet = TMaxFBGetF(RDHM_MLTP_VAL			,	PST_FIELD(pRPacket, rdhm_mltp_val		));
		nRet = TMaxFBGetF(DIST					,	PST_FIELD(pRPacket, dist				));
		nRet = TMaxFBGetF(TAKE_DRTM				,	PST_FIELD(pRPacket, take_drtm			));
		nRet = TMaxFBGetF(BUS_CLS_PRIN_YN		,	PST_FIELD(pRPacket, bus_cls_prin_yn		));
		nRet = TMaxFBGetF(BUS_CACM_NM_PRIN_YN	,	PST_FIELD(pRPacket, bus_cacm_nm_prin_yn	));
		nRet = TMaxFBGetF(DEPR_TIME_PRIN_YN		,	PST_FIELD(pRPacket, depr_time_prin_yn	));
		nRet = TMaxFBGetF(SATS_NO_PRIN_YN		,	PST_FIELD(pRPacket, sats_no_prin_yn		));
		nRet = TMaxFBGetF(ALCN_WAY_DVS_CD		,	PST_FIELD(pRPacket, alcn_way_dvs_cd		));
		nRet = TMaxFBGetF(SATI_USE_YN			,	PST_FIELD(pRPacket, sati_use_yn			));
		nRet = TMaxFBGetF(ROT_KND_CD			,	PST_FIELD(pRPacket, rot_knd_cd			));
		nRet = TMaxFBGetF(PERD_TEMP_YN			,	PST_FIELD(pRPacket, perd_temp_yn		));
		nRet = TMaxFBGetF(PUB_DT				,	PST_FIELD(pRPacket, pub_dt				));
		nRet = TMaxFBGetF(PUB_TIME				,	PST_FIELD(pRPacket, pub_time			));
		nRet = TMaxFBGetF(PUB_SHCT_TRML_CD		,	PST_FIELD(pRPacket, pub_shct_trml_cd	));
		nRet = TMaxFBGetF(PUB_WND_NO			,	PST_FIELD(pRPacket, pub_wnd_no			));
		nRet = TMaxFBGetF(CARD_TR_ADD_INF		,	PST_FIELD(pRPacket, card_tr_add_inf		));	 /// 기프트카드, 잔액

		// card_no ~ card_tr_add_inf 까지 복사
		::CopyMemory(pTr->card_resp.card_no, pRPacket->card_no, pRPacket->pub_num - pRPacket->card_no);

		nRet = TMaxFBGetF(PUB_NUM				,	PST_FIELD(pRPacket, pub_num				));
		nCount = *(WORD *)pRPacket->pub_num;
		if(nCount > 0)
		{
			pRPacket->pList = new rtk_ktcpbtckcard_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_ktcpbtckcard_list_t) * nCount);
		}

		// 현장발권 mem - 221_list clear
		pTr->n_card_pub_num = nCount;

		pTr->m_vtPbTckCard.clear();
		for(i = 0; i < nCount; i++)
		{
			prtk_ktcpbtckcard_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(PYN_MNS_DVS_CD			,	PST_FIELD(pList, pyn_mns_dvs_cd			));
			nRet = TMaxFBGetF(BUS_TCK_KND_CD			,	PST_FIELD(pList, bus_tck_knd_cd			));
			nRet = TMaxFBGetF(PUB_SNO					,	PST_FIELD(pList, pub_sno				));
			nRet = TMaxFBGetF(BUS_TCK_INHR_NO			,	PST_FIELD(pList, bus_tck_inhr_no		));
			nRet = TMaxFBGetF(TISU_AMT					,	PST_FIELD(pList, tisu_amt				));
			nRet = TMaxFBGetF(SATS_NO					,	PST_FIELD(pList, sats_no				));
			nRet = TMaxFBGetF(CTY_BUS_DC_KND_CD			,	PST_FIELD(pList, cty_bus_dc_knd_cd		));
			nRet = TMaxFBGetF(DCRT_DVS_CD				,	PST_FIELD(pList, dcrt_dvs_cd			));
			nRet = TMaxFBGetF(DC_BEF_AMT				,	PST_FIELD(pList, dc_bef_amt				));
			nRet = TMaxFBGetF(CTY_BUS_OPRN_SHP_CD		,	PST_FIELD(pList, cty_bus_oprn_shp_cd	));

			// 현장발권 mem - 221_resp
			pTr->m_vtPbTckCard.push_back(*pList);
		}

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(pRPacket->card_no, sizeof(pRPacket->card_no));
#endif
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ","RSP_CD",				PST_FIELD(pRPacket, rsp_cd				));
		LOG_WRITE("%30s - (%s) ","RSP_MSG",				PST_FIELD(pRPacket, rsp_msg				));
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ","CARD_NO",				PST_FIELD(pRPacket, card_no				));
#endif
		LOG_WRITE("%30s - (%s) ","CARD_APRV_NO",			PST_FIELD(pRPacket, card_aprv_no		));
		LOG_WRITE("%30s - (number = %d) ","CARD_APRV_AMT", *(int *)pRPacket->card_aprv_amt		);
		LOG_WRITE("%30s - (number = %d) ","APRV_AMT",		*(int *)pRPacket->aprv_amt				);
		LOG_WRITE("%30s - (%s) ","CARD_APRV_VANC_CD",		PST_FIELD(pRPacket, card_aprv_vanc_cd	));
		LOG_WRITE("%30s - (%s) ","ISSU_CRCM_CD",			PST_FIELD(pRPacket, issu_crcm_cd		));
		LOG_WRITE("%30s - (%s) ","BUY_CRCM_CD",			PST_FIELD(pRPacket, buy_crcm_cd			));
		LOG_WRITE("%30s - (number = %d) ","FRC_CMRT",		*(int *)pRPacket->frc_cmrt				);
		LOG_WRITE("%30s - (%s) ","ATL_DEPR_DT",			PST_FIELD(pRPacket, atl_depr_dt			));
		LOG_WRITE("%30s - (%s) ","ATL_DEPR_TIME",			PST_FIELD(pRPacket, atl_depr_time		));
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_CD",			PST_FIELD(pRPacket, depr_trml_cd		));
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_CD",			PST_FIELD(pRPacket, arvl_trml_cd		));
		LOG_WRITE("%30s - (%s) ","STPT_TRML_CD",			PST_FIELD(pRPacket, stpt_trml_cd		));
		LOG_WRITE("%30s - (%s) ","EPT_TRML_CD",			PST_FIELD(pRPacket, ept_trml_cd			));
		LOG_WRITE("%30s - (%s) ","BUS_CLS_CD",			PST_FIELD(pRPacket, bus_cls_cd			));
		LOG_WRITE("%30s - (%s) ","BUS_CACM_CD",			PST_FIELD(pRPacket, bus_cacm_cd			));
		LOG_WRITE("%30s - (%s) ","RDHM_MLTP_VAL",			PST_FIELD(pRPacket, rdhm_mltp_val		));
		LOG_WRITE("%30s - (number = %d) ","DIST",			*(int *)pRPacket->dist					);
		LOG_WRITE("%30s - (number = %d) ","TAKE_DRTM",	*(int *)pRPacket->take_drtm				);
		LOG_WRITE("%30s - (%s) ","BUS_CLS_PRIN_YN",		PST_FIELD(pRPacket, bus_cls_prin_yn		));
		LOG_WRITE("%30s - (%s) ","BUS_CACM_NM_PRIN_YN",	PST_FIELD(pRPacket, bus_cacm_nm_prin_yn	));
		LOG_WRITE("%30s - (%s) ","DEPR_TIME_PRIN_YN",		PST_FIELD(pRPacket, depr_time_prin_yn	));
		LOG_WRITE("%30s - (%s) ","SATS_NO_PRIN_YN",		PST_FIELD(pRPacket, sats_no_prin_yn		));
		LOG_WRITE("%30s - (%s) ","ALCN_WAY_DVS_CD",		PST_FIELD(pRPacket, alcn_way_dvs_cd		));
		LOG_WRITE("%30s - (%s) ","SATI_USE_YN",			PST_FIELD(pRPacket, sati_use_yn			));
		LOG_WRITE("%30s - (%s) ","ROT_KND_CD",			PST_FIELD(pRPacket, rot_knd_cd			));
		LOG_WRITE("%30s - (%s) ","PERD_TEMP_YN",			PST_FIELD(pRPacket, perd_temp_yn		));
		LOG_WRITE("%30s - (%s) ","PUB_DT",				PST_FIELD(pRPacket, pub_dt				));
		LOG_WRITE("%30s - (%s) ","PUB_TIME",				PST_FIELD(pRPacket, pub_time			));
		LOG_WRITE("%30s - (%s) ","PUB_SHCT_TRML_CD",		PST_FIELD(pRPacket, pub_shct_trml_cd	));
		LOG_WRITE("%30s - (%s) ","PUB_WND_NO",			PST_FIELD(pRPacket, pub_wnd_no			));
		LOG_WRITE("%30s - (%s) ","CARD_TR_ADD_INF",		PST_FIELD(pRPacket, card_tr_add_inf		));
		LOG_WRITE("%30s - (%d) ","PUB_NUM", nCount);

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "PYN_MNS_DVS_CD",			PST_FIELD(pList, pyn_mns_dvs_cd			));
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD",			PST_FIELD(pList, bus_tck_knd_cd			));
			LOG_WRITE("%30s - (%d) ", "PUB_SNO",				*(int *) pList->pub_sno					);
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_INHR_NO",		PST_FIELD(pList, bus_tck_inhr_no		));
			LOG_WRITE("%30s - (%d) ", "TISU_AMT",				*(int *) pList->tisu_amt				);
			LOG_WRITE("%30s - (%s) ", "SATS_NO",				PST_FIELD(pList, sats_no				));
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_DC_KND_CD",		PST_FIELD(pList, cty_bus_dc_knd_cd		));
			LOG_WRITE("%30s - (%s) ", "DCRT_DVS_CD",			PST_FIELD(pList, dcrt_dvs_cd			));
			LOG_WRITE("%30s - (%d) ", "DC_BEF_AMT",				*(int *) pList->dc_bef_amt				);
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_OPRN_SHP_CD",	PST_FIELD(pList, cty_bus_oprn_shp_cd	));
		}
	}

	///> data proc
	{
		nRet = nCount;
	}

	m_bVerify = FALSE;
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:

	m_bVerify = FALSE;
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 0;
#endif
}

// 20221017 ADD~
/**
 * @brief		TMaxSvc274
 * @details		(274) 승차권 발행 요청 - QR결제 전용(KIOSK)				
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc274(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int	nRet, i, nCount, nTimeout;
	int n_rsp_cd;
	char*	pService = "TK_PubTckQr";

	pstk_pbtckqr_t				pSPacket;
	//prtk_pbtckqr_t			pRPacket;
	//prtk_pbtckqr_list_t		pList; // 현장발권 mem 구조체를 아래 prtk_ktcpbtckcard_list_t 그대로 사용
	prtk_ktcpbtckcard_t			pRPacket;
	prtk_ktcpbtckcard_list_t	pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_pbtckqr_t) pData;
	//pRPacket = (prtk_pbtckqr_t) retBuf;
	pRPacket = (prtk_ktcpbtckcard_t) retBuf;

	m_bVerify = FALSE;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","ROT_ID",				PST_FIELD(pSPacket, rot_id				));
		LOG_WRITE("%30s - (number = %d) ","ROT_SQNO",	*(int *)pSPacket->rot_sqno				);
		LOG_WRITE("%30s - (%s) ","ALCN_DT",				PST_FIELD(pSPacket, alcn_dt				));
		LOG_WRITE("%30s - (number = %d) ","ALCN_SQNO",	*(int *)pSPacket->alcn_sqno				);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_CD",		PST_FIELD(pSPacket, depr_trml_cd		));
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_CD",		PST_FIELD(pSPacket, arvl_trml_cd		));
		LOG_WRITE("%30s - (%s) ","QR_CD_NO",			PST_FIELD(pSPacket, qr_cd_no			));
		LOG_WRITE("%30s - (%s) ","QR_PYM_PYN_DTL_CD",	PST_FIELD(pSPacket, qr_pym_pyn_dtl_cd	)); // QR결제지불상세코드 // 20221228 ADD
		LOG_WRITE("%30s - (%s) ","PAYCO_VIRT_ONTC_NO",	PST_FIELD(pSPacket, payco_virt_ontc_no	));

		LOG_WRITE("%30s - (number = %d) ","MIP_TERM",	*(WORD *) pSPacket->mip_term			);
		LOG_WRITE("%30s - (%s) ","SPAD_DTA",			PST_FIELD(pSPacket, spad_dta			));
		LOG_WRITE("%30s - (%s) ","SPAD_DTA_LEN",		pSPacket->spad_dta_len					);

		LOG_WRITE("%30s - (number = %d) ","PUB_NUM",	*(int *)pSPacket->pub_num				);
		nCount = *(int *) pSPacket->pub_num;
		for(i = 0; i < nCount; i++)
		{
			pstk_pbtckqr_list_t pSList;

			pSList = &pSPacket->List[i];

			LOG_WRITE("순번 (%d / %d)", i + 1, nCount);

			LOG_WRITE("%30s - (%s) ",			"BUS_TCK_KND_CD",	PST_FIELD(pSList, bus_tck_knd_cd));
			LOG_WRITE("%30s - (number = %d) ",	"TISU_AMT",			*(int *)pSList->tisu_amt		);
			LOG_WRITE("%30s - (%s) ",			"SATS_PCPY_ID",		PST_FIELD(pSList, sats_pcpy_id	));
			LOG_WRITE("%30s - (number = %d) ",	"SATS_NO",			*(int *)pSList->sats_no			);
		}
	}

	// send data
	{
		int nEncLen;
		int rndLen, nRet, nOffset;
		char rndString[100];
		char retString[100];
		int n_rot_sqno;
		int n_alcn_sqno;
		int n_enc_dta_len;
		int n_mrnp_num;
		int n_mip_term;
		int n_pub_num;
//		int n_sat_no;

		rndLen = nRet = nOffset = 0;

		SendIDHeader((char *)&pSPacket->hdr_id);

		n_rot_sqno		= *(int *) pSPacket->rot_sqno;
		n_alcn_sqno		= *(int *) pSPacket->alcn_sqno;
		n_pub_num		= *(int *) pSPacket->pub_num;
		n_mip_term		= *(WORD *) pSPacket->mip_term;

		nRet = TMaxFBPut(ROT_ID				,	PST_FIELD(pSPacket, rot_id				));
		nRet = TMaxFBPutLen(ROT_SQNO		,	PST_FIELD(pSPacket, rot_sqno),			5);
		nRet = TMaxFBPut(ALCN_DT			,	PST_FIELD(pSPacket, alcn_dt				));
		nRet = TMaxFBPutLen(ALCN_SQNO		,	PST_FIELD(pSPacket, alcn_sqno),			5);
		nRet = TMaxFBPut(DEPR_TRML_CD		,	PST_FIELD(pSPacket, depr_trml_cd		));
		nRet = TMaxFBPut(ARVL_TRML_CD		,	PST_FIELD(pSPacket, arvl_trml_cd		));
		nRet = TMaxFBPut(QR_CD_NO			,	PST_FIELD(pSPacket, qr_cd_no			));
		nRet = TMaxFBPut(QR_PYM_PYN_DTL_CD	,	PST_FIELD(pSPacket, qr_pym_pyn_dtl_cd	)); // QR결제지불상세코드 // 20221228 ADD
		nRet = TMaxFBPut(PAYCO_VIRT_ONTC_NO	,	PST_FIELD(pSPacket, payco_virt_ontc_no	));

		nRet = TMaxFBPut(SPAD_DTA		,	PST_FIELD(pSPacket, spad_dta		));
		nRet = TMaxFBPut(SPAD_DTA_LEN	,	PST_FIELD(pSPacket, spad_dta_len	));
		nRet = TMaxFBPutLen(MIP_TERM	,	(char *)&n_mip_term, 2				);

		nCount = *(int *) pSPacket->pub_num;
		nRet = TMaxFBPut(PUB_NUM, pSPacket->pub_num);
		for(i = 0; i < nCount; i++)
		{
			nRet = TMaxFBInsert(BUS_TCK_KND_CD, ST_FIELD(pSPacket->List[i], bus_tck_knd_cd	), i, sizeof(pSPacket->List[i].bus_tck_knd_cd) - 1);
			nRet = TMaxFBInsert(TISU_AMT,		pSPacket->List[i].tisu_amt,	i, sizeof(pSPacket->List[i].tisu_amt) - 1);
			nRet = TMaxFBInsert(SATS_PCPY_ID,	ST_FIELD(pSPacket->List[i], sats_pcpy_id	), i, sizeof(pSPacket->List[i].sats_pcpy_id) - 1 );
			nRet = TMaxFBInsert(SATS_NO,		pSPacket->List[i].sats_no,	i, sizeof(pSPacket->List[i].sats_no) - 1);
		}
	}

	// 동기화 (Send/Recv)
	nTimeout = 10;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		CPubTckMem* pTr;

		// 현장발권 mem
		pTr = CPubTckMem::GetInstance();

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		LOG_WRITE("%30s - (%d) ","응답코드(int)", n_rsp_cd);
		nRet = TMaxFBGetF(RSP_MSG,	PST_FIELD(pRPacket, rsp_msg				));
		if( n_rsp_cd != 146 ) // AZI146:작업이 완료되었습니다.
		{	// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - (%s) ERROR !!!!", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		nRet = TMaxFBGetF(CARD_NO				,	PST_FIELD(pRPacket, card_no				));
		nRet = TMaxFBGetF(CARD_APRV_NO			,	PST_FIELD(pRPacket, card_aprv_no		));
		nRet = TMaxFBGetF(CARD_APRV_AMT			,	PST_FIELD(pRPacket, card_aprv_amt		));
		nRet = TMaxFBGetF(APRV_AMT				,	PST_FIELD(pRPacket, aprv_amt			));
		nRet = TMaxFBGetF(CARD_APRV_VANC_CD		,	PST_FIELD(pRPacket, card_aprv_vanc_cd	));
		nRet = TMaxFBGetF(ISSU_CRCM_CD			,	PST_FIELD(pRPacket, issu_crcm_cd		));
		nRet = TMaxFBGetF(BUY_CRCM_CD			,	PST_FIELD(pRPacket, buy_crcm_cd			));
		nRet = TMaxFBGetF(FRC_CMRT				,	PST_FIELD(pRPacket, frc_cmrt			));
		nRet = TMaxFBGetF(ATL_DEPR_DT			,	PST_FIELD(pRPacket, atl_depr_dt			));
		nRet = TMaxFBGetF(ATL_DEPR_TIME			,	PST_FIELD(pRPacket, atl_depr_time		));
		nRet = TMaxFBGetF(DEPR_TRML_CD			,	PST_FIELD(pRPacket, depr_trml_cd		));
		nRet = TMaxFBGetF(ARVL_TRML_CD			,	PST_FIELD(pRPacket, arvl_trml_cd		));
		nRet = TMaxFBGetF(STPT_TRML_CD			,	PST_FIELD(pRPacket, stpt_trml_cd		));
		nRet = TMaxFBGetF(EPT_TRML_CD			,	PST_FIELD(pRPacket, ept_trml_cd			));
		nRet = TMaxFBGetF(BUS_CLS_CD			,	PST_FIELD(pRPacket, bus_cls_cd			));
		nRet = TMaxFBGetF(BUS_CACM_CD			,	PST_FIELD(pRPacket, bus_cacm_cd			));
		nRet = TMaxFBGetF(RDHM_MLTP_VAL			,	PST_FIELD(pRPacket, rdhm_mltp_val		));
		nRet = TMaxFBGetF(DIST					,	PST_FIELD(pRPacket, dist				));
		nRet = TMaxFBGetF(TAKE_DRTM				,	PST_FIELD(pRPacket, take_drtm			));
		nRet = TMaxFBGetF(BUS_CLS_PRIN_YN		,	PST_FIELD(pRPacket, bus_cls_prin_yn		));
		nRet = TMaxFBGetF(BUS_CACM_NM_PRIN_YN	,	PST_FIELD(pRPacket, bus_cacm_nm_prin_yn	));
		nRet = TMaxFBGetF(DEPR_TIME_PRIN_YN		,	PST_FIELD(pRPacket, depr_time_prin_yn	));
		nRet = TMaxFBGetF(SATS_NO_PRIN_YN		,	PST_FIELD(pRPacket, sats_no_prin_yn		));
		nRet = TMaxFBGetF(ALCN_WAY_DVS_CD		,	PST_FIELD(pRPacket, alcn_way_dvs_cd		));
		nRet = TMaxFBGetF(SATI_USE_YN			,	PST_FIELD(pRPacket, sati_use_yn			));
		nRet = TMaxFBGetF(ROT_KND_CD			,	PST_FIELD(pRPacket, rot_knd_cd			));
		nRet = TMaxFBGetF(PERD_TEMP_YN			,	PST_FIELD(pRPacket, perd_temp_yn		));
		nRet = TMaxFBGetF(PUB_DT				,	PST_FIELD(pRPacket, pub_dt				));
		nRet = TMaxFBGetF(PUB_TIME				,	PST_FIELD(pRPacket, pub_time			));
		nRet = TMaxFBGetF(PUB_SHCT_TRML_CD		,	PST_FIELD(pRPacket, pub_shct_trml_cd	));
		nRet = TMaxFBGetF(PUB_WND_NO			,	PST_FIELD(pRPacket, pub_wnd_no			));
		nRet = TMaxFBGetF(CARD_TR_ADD_INF		,	PST_FIELD(pRPacket, card_tr_add_inf		));	 /// 기프트카드, 잔액

		// card_no ~ card_tr_add_inf 까지 복사
		::CopyMemory(pTr->card_resp.card_no, pRPacket->card_no, pRPacket->pub_num - pRPacket->card_no);

		nRet = TMaxFBGetF(PUB_NUM				,	PST_FIELD(pRPacket, pub_num				));
		nCount = *(WORD *)pRPacket->pub_num;
		if(nCount > 0)
		{
			//pRPacket->pList = new rtk_pbtckqr_list_t[nCount];
			pRPacket->pList = new rtk_ktcpbtckcard_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_ktcpbtckcard_list_t) * nCount);
		}

		// 현장발권 mem - 274_list clear
		pTr->n_card_pub_num = nCount;

		pTr->m_vtPbTckCard.clear();
		for(i = 0; i < nCount; i++)
		{
			//prtk_pbtckqr_list_t pList;
			prtk_ktcpbtckcard_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(PYN_MNS_DVS_CD			,	PST_FIELD(pList, pyn_mns_dvs_cd			));
			nRet = TMaxFBGetF(BUS_TCK_KND_CD			,	PST_FIELD(pList, bus_tck_knd_cd			));
			nRet = TMaxFBGetF(PUB_SNO					,	PST_FIELD(pList, pub_sno				));
			nRet = TMaxFBGetF(BUS_TCK_INHR_NO			,	PST_FIELD(pList, bus_tck_inhr_no		));
			nRet = TMaxFBGetF(TISU_AMT					,	PST_FIELD(pList, tisu_amt				));
			nRet = TMaxFBGetF(SATS_NO					,	PST_FIELD(pList, sats_no				));
			nRet = TMaxFBGetF(CTY_BUS_DC_KND_CD			,	PST_FIELD(pList, cty_bus_dc_knd_cd		));
			nRet = TMaxFBGetF(DCRT_DVS_CD				,	PST_FIELD(pList, dcrt_dvs_cd			));
			nRet = TMaxFBGetF(DC_BEF_AMT				,	PST_FIELD(pList, dc_bef_amt				));

			// 현장발권 mem - 274_resp
			pTr->m_vtPbTckCard.push_back(*pList);
		}

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(pRPacket->card_no, sizeof(pRPacket->card_no));
#endif
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ","RSP_CD",				PST_FIELD(pRPacket, rsp_cd				));
		LOG_WRITE("%30s - (%s) ","RSP_MSG",				PST_FIELD(pRPacket, rsp_msg				));
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ","CARD_NO",				PST_FIELD(pRPacket, card_no				));
#endif
		LOG_WRITE("%30s - (%s) ","CARD_APRV_NO",			PST_FIELD(pRPacket, card_aprv_no		));
		LOG_WRITE("%30s - (number = %d) ","CARD_APRV_AMT", *(int *)pRPacket->card_aprv_amt		);
		LOG_WRITE("%30s - (number = %d) ","APRV_AMT",		*(int *)pRPacket->aprv_amt				);
		LOG_WRITE("%30s - (%s) ","CARD_APRV_VANC_CD",		PST_FIELD(pRPacket, card_aprv_vanc_cd	));
		LOG_WRITE("%30s - (%s) ","ISSU_CRCM_CD",			PST_FIELD(pRPacket, issu_crcm_cd		));
		LOG_WRITE("%30s - (%s) ","BUY_CRCM_CD",			PST_FIELD(pRPacket, buy_crcm_cd			));
		LOG_WRITE("%30s - (number = %d) ","FRC_CMRT",		*(int *)pRPacket->frc_cmrt				);
		LOG_WRITE("%30s - (%s) ","ATL_DEPR_DT",			PST_FIELD(pRPacket, atl_depr_dt			));
		LOG_WRITE("%30s - (%s) ","ATL_DEPR_TIME",			PST_FIELD(pRPacket, atl_depr_time		));
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_CD",			PST_FIELD(pRPacket, depr_trml_cd		));
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_CD",			PST_FIELD(pRPacket, arvl_trml_cd		));
		LOG_WRITE("%30s - (%s) ","STPT_TRML_CD",			PST_FIELD(pRPacket, stpt_trml_cd		));
		LOG_WRITE("%30s - (%s) ","EPT_TRML_CD",			PST_FIELD(pRPacket, ept_trml_cd			));
		LOG_WRITE("%30s - (%s) ","BUS_CLS_CD",			PST_FIELD(pRPacket, bus_cls_cd			));
		LOG_WRITE("%30s - (%s) ","BUS_CACM_CD",			PST_FIELD(pRPacket, bus_cacm_cd			));
		LOG_WRITE("%30s - (%s) ","RDHM_MLTP_VAL",			PST_FIELD(pRPacket, rdhm_mltp_val		));
		LOG_WRITE("%30s - (number = %d) ","DIST",			*(int *)pRPacket->dist					);
		LOG_WRITE("%30s - (number = %d) ","TAKE_DRTM",	*(int *)pRPacket->take_drtm				);
		LOG_WRITE("%30s - (%s) ","BUS_CLS_PRIN_YN",		PST_FIELD(pRPacket, bus_cls_prin_yn		));
		LOG_WRITE("%30s - (%s) ","BUS_CACM_NM_PRIN_YN",	PST_FIELD(pRPacket, bus_cacm_nm_prin_yn	));
		LOG_WRITE("%30s - (%s) ","DEPR_TIME_PRIN_YN",		PST_FIELD(pRPacket, depr_time_prin_yn	));
		LOG_WRITE("%30s - (%s) ","SATS_NO_PRIN_YN",		PST_FIELD(pRPacket, sats_no_prin_yn		));
		LOG_WRITE("%30s - (%s) ","ALCN_WAY_DVS_CD",		PST_FIELD(pRPacket, alcn_way_dvs_cd		));
		LOG_WRITE("%30s - (%s) ","SATI_USE_YN",			PST_FIELD(pRPacket, sati_use_yn			));
		LOG_WRITE("%30s - (%s) ","ROT_KND_CD",			PST_FIELD(pRPacket, rot_knd_cd			));
		LOG_WRITE("%30s - (%s) ","PERD_TEMP_YN",			PST_FIELD(pRPacket, perd_temp_yn		));
		LOG_WRITE("%30s - (%s) ","PUB_DT",				PST_FIELD(pRPacket, pub_dt				));
		LOG_WRITE("%30s - (%s) ","PUB_TIME",				PST_FIELD(pRPacket, pub_time			));
		LOG_WRITE("%30s - (%s) ","PUB_SHCT_TRML_CD",		PST_FIELD(pRPacket, pub_shct_trml_cd	));
		LOG_WRITE("%30s - (%s) ","PUB_WND_NO",			PST_FIELD(pRPacket, pub_wnd_no			));
		LOG_WRITE("%30s - (%s) ","CARD_TR_ADD_INF",		PST_FIELD(pRPacket, card_tr_add_inf		));
		LOG_WRITE("%30s - (%d) ","PUB_NUM", nCount);

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "PYN_MNS_DVS_CD",			PST_FIELD(pList, pyn_mns_dvs_cd			));
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD",			PST_FIELD(pList, bus_tck_knd_cd			));
			LOG_WRITE("%30s - (%d) ", "PUB_SNO",				*(int *) pList->pub_sno					);
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_INHR_NO",		PST_FIELD(pList, bus_tck_inhr_no		));
			LOG_WRITE("%30s - (%d) ", "TISU_AMT",				*(int *) pList->tisu_amt				);
			LOG_WRITE("%30s - (%d) ", "SATS_NO",				*(int *) pList->sats_no					);
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_DC_KND_CD",		PST_FIELD(pList, cty_bus_dc_knd_cd		));
			LOG_WRITE("%30s - (%s) ", "DCRT_DVS_CD",			PST_FIELD(pList, dcrt_dvs_cd			));
			LOG_WRITE("%30s - (%d) ", "DC_BEF_AMT",				*(int *) pList->dc_bef_amt				);
		}
	}

	///> data proc
	{
		nRet = nCount;
	}

	m_bVerify = FALSE;
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:

	m_bVerify = FALSE;
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 0;
#endif
}
// 20221017 ~ADD

/**
 * @brief		TMaxSvc222
 * @details		(222) 승차권 발행 - 현금영수증(KTC인증)
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc222(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nTimeout, nCount;
	int n_rsp_cd;
	char*	pService = "TK_KtcPbTckCsrc";
	pstk_ktcpbtckcsrc_t		pSPacket;
	prtk_ktcpbtckcsrc_t		pRPacket;
	CPubTckMem* pPubTr;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);
	
	m_bVerify = FALSE;

	pPubTr = CPubTckMem::GetInstance();

	pSPacket = (pstk_ktcpbtckcsrc_t) pData;
	pRPacket = (prtk_ktcpbtckcsrc_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","ROT_ID"				,	PST_FIELD(pSPacket, rot_id						));
		LOG_WRITE("%30s - (number = %d) ","ROT_SQNO"	,	*(int *)pSPacket->rot_sqno						);
		LOG_WRITE("%30s - (%s) ","ALCN_DT"			,	PST_FIELD(pSPacket, alcn_dt						));
		LOG_WRITE("%30s - (number = %d) ","ALCN_SQNO"	,	*(int *)pSPacket->alcn_sqno						);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_CD"		,	PST_FIELD(pSPacket, depr_trml_cd				));
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_CD"		,	PST_FIELD(pSPacket, arvl_trml_cd				));
		LOG_WRITE("%30s - (%s) ","TISU_WAY_DVS_CD"	,	PST_FIELD(pSPacket, tisu_way_dvs_cd				));
		LOG_WRITE("%30s - (%s) ","PYN_MNS_DVS_CD"		,	PST_FIELD(pSPacket, pyn_mns_dvs_cd				));
		LOG_WRITE("%30s - (%s) ","PUB_CHNL_DVS_CD"	,	PST_FIELD(pSPacket, pub_chnl_dvs_cd				));
		LOG_WRITE("%30s - (%s) ","POS_PG_KTC_VER"		,	PST_FIELD(pSPacket, pos_pg_ktc_ver				));
		LOG_WRITE("%30s - (number = %d)","ENC_DTA_LEN",	*(int *)pSPacket->enc_dta_len					);

#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ","ENC_DTA"			,	PST_FIELD(pSPacket, enc_dta						));
#endif

		LOG_WRITE("%30s - (%s) ","CPRT_YN"			,	PST_FIELD(pSPacket, cprt_yn						));
		LOG_WRITE("%30s - (%s) ","USER_DVS_NO"		,	PST_FIELD(pSPacket, user_dvs_no					));
		LOG_WRITE("%30s - (%s) ","OLD_MBRS_NO"		,	PST_FIELD(pSPacket, old_mbrs_no					));
		
		LOG_WRITE("%30s - (number = %d) ","PUB_NUM"	,	*(int *)pSPacket->pub_num						);
		nCount = *(int *)pSPacket->pub_num;

		for(i = 0; i < nCount; i++)
		{
			pstk_ktcpbtckcsrc_list_t pList;

			pList = &pSPacket->List[i];

			LOG_WRITE(" 순번 ( %d / %d )", i + 1, nCount);

			LOG_WRITE("%30s - (%s) ","BUS_TCK_KND_CD"		,	PST_FIELD(pList, bus_tck_knd_cd	));
			LOG_WRITE("%30s - (%s) ","SATS_PCPY_ID"		,	PST_FIELD(pList, sats_pcpy_id	));
			LOG_WRITE("%30s - (number = %d) ","SATS_NO"	,	*(int *)pList->sats_no			);
			LOG_WRITE("%30s - (%s) ","MRNP_NUM"			,	PST_FIELD(pList, mrnp_num		));
			LOG_WRITE("%30s - (%s) ","MRNP_ID"			,	PST_FIELD(pList, mrnp_id		));
		}
	}

	// send data
	{
		int n_mrnp_num;
		int nEncLen;
		int rndLen, nRet, nOffset;
		char rndString[100];
		char retString[100];

		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(ROT_ID			,	PST_FIELD(pSPacket, rot_id			));	///< 노선id				
		nRet = TMaxFBPutLen(ROT_SQNO	,	PST_FIELD(pSPacket, rot_sqno		), sizeof(pSPacket->rot_sqno) - 1);	///< 노선순번				
		nRet = TMaxFBPut(ALCN_DT		,	PST_FIELD(pSPacket, alcn_dt			));	///< 배차일자				
		nRet = TMaxFBPutLen(ALCN_SQNO	,	PST_FIELD(pSPacket, alcn_sqno		), sizeof(pSPacket->alcn_sqno) - 1);	///< 배차순번				
		nRet = TMaxFBPut(DEPR_TRML_CD	,	PST_FIELD(pSPacket, depr_trml_cd	));	///< 출발터미널코드			
		nRet = TMaxFBPut(ARVL_TRML_CD	,	PST_FIELD(pSPacket, arvl_trml_cd	));	///< 도착터미널코드			
		nRet = TMaxFBPut(TISU_WAY_DVS_CD,	PST_FIELD(pSPacket, tisu_way_dvs_cd	));	///< 발권방식구분코드			
		nRet = TMaxFBPut(PYN_MNS_DVS_CD	,	PST_FIELD(pSPacket, pyn_mns_dvs_cd	));	///< 지불수단구분코드			
		nRet = TMaxFBPut(PUB_CHNL_DVS_CD,	PST_FIELD(pSPacket, pub_chnl_dvs_cd	));	///< 발행채널구분코드			
		nRet = TMaxFBPut(POS_PG_KTC_VER	,	PST_FIELD(pSPacket, pos_pg_ktc_ver	));	///< pos단말기버전정보			
//		nRet = TMaxFBPutLen(ENC_DTA_LEN	,	PST_FIELD(pSPacket, enc_dta_len		), sizeof(pSPacket->enc_dta_len) - 1);	///< enc 데이터 길이			
		nRet = TMaxFBPutLen(ENC_DTA_LEN	,	PST_FIELD(pSPacket, enc_dta_len		), 4);

		nEncLen = *(int *) pSPacket->enc_dta_len;

		/// 암호화에 암호화
		for(i = 0; i < nEncLen; i++)
		{
			nOffset = 0;
			::ZeroMemory(rndString, sizeof(rndString));
			::ZeroMemory(retString, sizeof(retString));

			srand((unsigned int) i);
			rndLen = rand() % 5 + 2;
			nRet = GetRndChar(rndLen, rndString);

			retString[nOffset++] = rndString[0];
			retString[nOffset++] = pSPacket->enc_dta[i];
			::CopyMemory(&retString[nOffset], &rndString[1], rndLen - 1);

			nRet = TMaxFBInsert(ENC_DTA, retString, i, nOffset);

			::ZeroMemory(rndString, sizeof(rndString));
			::ZeroMemory(retString, sizeof(retString));
		}
		
		nRet = TMaxFBPut(CPRT_YN		,	PST_FIELD(pSPacket, cprt_yn			));	///< 법인여부				
		nRet = TMaxFBPut(USER_DVS_NO	,	PST_FIELD(pSPacket, user_dvs_no		));	///< 사용자구분번호			
		nRet = TMaxFBPut(OLD_MBRS_NO	,	PST_FIELD(pSPacket, old_mbrs_no		));	///< 구회원번호				

		n_mrnp_num = 0;
//		nRet = TMaxFBPutLen(MRNP_NUM, (char*)&n_mrnp_num, 5);
		nRet = TMaxFBPutLen(MRNP_NUM, ST_FIELD(pSPacket->List[0], mrnp_num	), sizeof(pSPacket->List[0].mrnp_num) - 1);
		nRet = TMaxFBPut(MRNP_ID, " ");

//		nRet = TMaxFBPutLen(PUB_NUM		,	PST_FIELD(pSPacket, pub_num			), sizeof(pSPacket->pub_num) - 1);	///< 발행매수				
		nRet = TMaxFBPut(PUB_NUM		,	PST_FIELD(pSPacket, pub_num			));	///< 발행매수				
		nCount = *(int *) pSPacket->pub_num;
		for(i = 0; i < nCount; i++)
		{
			pstk_ktcpbtckcsrc_list_t pList;

			pList = &pSPacket->List[i];

			nRet = TMaxFBInsert(BUS_TCK_KND_CD	,	PST_FIELD(pList, bus_tck_knd_cd	), i, sizeof(pList->bus_tck_knd_cd) - 1);	///< 버스티켓종류코드			
			nRet = TMaxFBInsert(SATS_PCPY_ID	,	PST_FIELD(pList, sats_pcpy_id	), i, sizeof(pList->sats_pcpy_id) - 1);		///< 좌석선점id				
			nRet = TMaxFBInsert(SATS_NO			,	PST_FIELD(pList, sats_no		), i, sizeof(pList->sats_no) - 1);			///< 좌석번호				
			//nRet = TMaxFBInsert(MRNP_NUM		,	PST_FIELD(pList, mrnp_num		), i, sizeof(pList->mrnp_num) - 1);			///< 예약매수				
			//nRet = TMaxFBInsert(MRNP_ID			,	PST_FIELD(pList, mrnp_id		), i, sizeof(pList->mrnp_id) - 1);			///< 예약id				
		}
	}

	// 동기화 (Send/Recv)
	nTimeout = 45;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	LOG_WRITE("TMaxTpCall(), Success !!!!");

	///> recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( (n_rsp_cd != 146) && (n_rsp_cd != 451) )
		{
			nRet = -4;
			if(n_rsp_cd == 428) 
			{
				nRet = TMaxFBGetF(RSP_MSG,	PST_FIELD(pRPacket, rsp_msg	));	///< 응답 메시지

				LOG_WRITE("rsp_msg = %s", pRPacket->rsp_msg);
			}
			LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}

		if(n_rsp_cd == 451)
		{ // 현금영수증 등록에 실패했습니다. \n자진발급으로 발행합니다.
			CPubTckMem::GetInstance()->base.pyn_mns_dvs_cd[0] = PYM_CD_CASH;
		}

		nRet = TMaxFBGetF(RSP_MSG				,	PST_FIELD(pRPacket, rsp_msg				));	///< 응답 메시지		

		nRet = TMaxFBGetF(CSRC_AUTH_NO			,	PST_FIELD(pRPacket, csrc_auth_no		));	///< 현금영수증인증번호		
		nRet = TMaxFBGetF(CSRC_BRN				,	PST_FIELD(pRPacket, csrc_brn			));	///< 현금영수증사업자등록번호
		nRet = TMaxFBGetF(CSRC_APRV_VANC_CD		,	PST_FIELD(pRPacket, csrc_aprv_vanc_cd	));	///< 현금영수증승인van사코드
		nRet = TMaxFBGetF(ATL_DEPR_DT			,	PST_FIELD(pRPacket, atl_depr_dt			));	///< 실제출발일자			
		nRet = TMaxFBGetF(ATL_DEPR_TIME			,	PST_FIELD(pRPacket, atl_depr_time		));	///< 실제출발시각			
		nRet = TMaxFBGetF(DEPR_TRML_CD			,	PST_FIELD(pRPacket, depr_trml_cd		));	///< 출발터미널코드		
		nRet = TMaxFBGetF(ARVL_TRML_CD			,	PST_FIELD(pRPacket, arvl_trml_cd		));	///< 도착터미널코드		
		nRet = TMaxFBGetF(STPT_TRML_CD			,	PST_FIELD(pRPacket, stpt_trml_cd		));	///< 기점터미널코드		
		nRet = TMaxFBGetF(EPT_TRML_CD			,	PST_FIELD(pRPacket, ept_trml_cd			));	///< 종점터미널코드		
		nRet = TMaxFBGetF(BUS_CLS_CD			,	PST_FIELD(pRPacket, bus_cls_cd			));	///< 버스등급코드			
		nRet = TMaxFBGetF(BUS_CACM_CD			,	PST_FIELD(pRPacket, bus_cacm_cd			));	///< 버스운수사코드		
		nRet = TMaxFBGetF(RDHM_MLTP_VAL			,	PST_FIELD(pRPacket, rdhm_mltp_val		));	///< 승차홈다중값			
		nRet = TMaxFBGetF(DIST					,	PST_FIELD(pRPacket, dist				));	///< 거리				
		nRet = TMaxFBGetF(TAKE_DRTM				,	PST_FIELD(pRPacket, take_drtm			));	///< 소요시간			
		nRet = TMaxFBGetF(BUS_CLS_PRIN_YN		,	PST_FIELD(pRPacket, bus_cls_prin_yn		));	///< 버스등급출력여부		
		nRet = TMaxFBGetF(BUS_CACM_NM_PRIN_YN	,	PST_FIELD(pRPacket, bus_cacm_nm_prin_yn	));	///< 버스운수사명출력여부	
		nRet = TMaxFBGetF(DEPR_TIME_PRIN_YN		,	PST_FIELD(pRPacket, depr_time_prin_yn	));	///< 출발시각출력여부		
		nRet = TMaxFBGetF(SATS_NO_PRIN_YN		,	PST_FIELD(pRPacket, sats_no_prin_yn		));	///< 좌석번호출력여부		
		nRet = TMaxFBGetF(ALCN_WAY_DVS_CD		,	PST_FIELD(pRPacket, alcn_way_dvs_cd		));	///< 배차방식구분코드		
		nRet = TMaxFBGetF(SATI_USE_YN			,	PST_FIELD(pRPacket, sati_use_yn			));	///< 좌석제사용여부		
		nRet = TMaxFBGetF(ROT_KND_CD			,	PST_FIELD(pRPacket, rot_knd_cd			));	///< 노선종류코드			
		nRet = TMaxFBGetF(PERD_TEMP_YN			,	PST_FIELD(pRPacket, perd_temp_yn		));	///< 정기임시여부			
		nRet = TMaxFBGetF(PUB_DT				,	PST_FIELD(pRPacket, pub_dt				));	///< 발행일자			
		nRet = TMaxFBGetF(PUB_TIME				,	PST_FIELD(pRPacket, pub_time			));	///< 발행시각			
		nRet = TMaxFBGetF(PUB_SHCT_TRML_CD		,	PST_FIELD(pRPacket, pub_shct_trml_cd	));	///< 발행단축터미널코드		
		nRet = TMaxFBGetF(PUB_WND_NO			,	PST_FIELD(pRPacket, pub_wnd_no			));	///< 발행창구번호			

		// CPubTckMem에 데이타 저장
		{
			int kLen = pRPacket->pub_num - pRPacket->csrc_auth_no;

			::CopyMemory(pPubTr->csrc_card_resp.csrc_auth_no, pRPacket->csrc_auth_no, pRPacket->pub_num - pRPacket->csrc_auth_no);

			//::CopyMemory(pPubTr->csrc_resp.csrc_auth_no		, pRPacket->csrc_auth_no, sizeof(pPubTr->csrc_resp.csrc_auth_no));
			//::CopyMemory(pPubTr->csrc_resp.csrc_brn			, pRPacket->csrc_brn			, sizeof(pPubTr->csrc_resp.csrc_brn			));
			//::CopyMemory(pPubTr->csrc_resp.csrc_aprv_vanc_cd, pRPacket->csrc_aprv_vanc_cd	, sizeof(pPubTr->csrc_resp.csrc_aprv_vanc_cd	));
			//::CopyMemory(pPubTr->csrc_resp.atl_depr_dt		, pRPacket->atl_depr_dt			, sizeof(pPubTr->csrc_resp.atl_depr_dt		));
			//::CopyMemory(pPubTr->csrc_resp.atl_depr_time	, pRPacket->atl_depr_time		, sizeof(pPubTr->csrc_resp.atl_depr_time		));
			//::CopyMemory(pPubTr->csrc_resp.depr_trml_cd		, pRPacket->depr_trml_cd		, sizeof(pPubTr->csrc_resp.depr_trml_cd		));
			//::CopyMemory(pPubTr->csrc_resp.arvl_trml_cd		, pRPacket->arvl_trml_cd		, sizeof(pPubTr->csrc_resp.arvl_trml_cd		));
			//::CopyMemory(pPubTr->csrc_resp.stpt_trml_cd		, pRPacket->stpt_trml_cd		, sizeof(pPubTr->csrc_resp.stpt_trml_cd		));
			//::CopyMemory(pPubTr->csrc_resp.ept_trml_cd		, pRPacket->ept_trml_cd			, sizeof(pPubTr->csrc_resp.ept_trml_cd		));
			//::CopyMemory(pPubTr->csrc_resp.bus_cls_cd		, pRPacket->bus_cls_cd			, sizeof(pPubTr->csrc_resp.bus_cls_cd			));
			//::CopyMemory(pPubTr->csrc_resp.bus_cacm_cd		, pRPacket->bus_cacm_cd			, sizeof(pPubTr->csrc_resp.bus_cacm_cd		));
			//::CopyMemory(pPubTr->csrc_resp.rdhm_mltp_val	, pRPacket->rdhm_mltp_val		, sizeof(pPubTr->csrc_resp.rdhm_mltp_val		));
			//::CopyMemory(pPubTr->csrc_resp.dist				, pRPacket->dist				, sizeof(pPubTr->csrc_resp.dist				));
			//::CopyMemory(pPubTr->csrc_resp.take_drtm		, pRPacket->take_drtm			, sizeof(pPubTr->csrc_resp.take_drtm			));
			//::CopyMemory(pPubTr->csrc_resp.bus_cls_prin_yn	, pRPacket->bus_cls_prin_yn		, sizeof(pPubTr->csrc_resp.bus_cls_prin_yn	));
			//::CopyMemory(pPubTr->csrc_resp.bus_cacm_nm_prin_yn, pRPacket->bus_cacm_nm_prin_yn	, sizeof(pPubTr->csrc_resp.bus_cacm_nm_prin_yn));
			//::CopyMemory(pPubTr->csrc_resp.depr_time_prin_yn, pRPacket->depr_time_prin_yn	, sizeof(pPubTr->csrc_resp.depr_time_prin_yn	));
			//::CopyMemory(pPubTr->csrc_resp.sats_no_prin_yn	, pRPacket->sats_no_prin_yn		, sizeof(pPubTr->csrc_resp.sats_no_prin_yn	));
			//::CopyMemory(pPubTr->csrc_resp.alcn_way_dvs_cd	, pRPacket->alcn_way_dvs_cd		, sizeof(pPubTr->csrc_resp.alcn_way_dvs_cd	));
			//::CopyMemory(pPubTr->csrc_resp.sati_use_yn		, pRPacket->sati_use_yn			, sizeof(pPubTr->csrc_resp.sati_use_yn		));
			//::CopyMemory(pPubTr->csrc_resp.rot_knd_cd		, pRPacket->rot_knd_cd			, sizeof(pPubTr->csrc_resp.rot_knd_cd			));
			//::CopyMemory(pPubTr->csrc_resp.perd_temp_yn		, pRPacket->perd_temp_yn		, sizeof(pPubTr->csrc_resp.perd_temp_yn		));
			//::CopyMemory(pPubTr->csrc_resp.pub_dt			, pRPacket->pub_dt				, sizeof(pPubTr->csrc_resp.pub_dt				));
			//::CopyMemory(pPubTr->csrc_resp.pub_time			, pRPacket->pub_time			, sizeof(pPubTr->csrc_resp.pub_time			));
			//::CopyMemory(pPubTr->csrc_resp.pub_shct_trml_cd	, pRPacket->pub_shct_trml_cd	, sizeof(pPubTr->csrc_resp.pub_shct_trml_cd	));
			//::CopyMemory(pPubTr->csrc_resp.pub_wnd_no			, pRPackpub_wnd_no			et->, sizeof(pPubTr->csrc_resp.pub_wnd_no			));
		}

		nRet = TMaxFBGetF(PUB_NUM,	PST_FIELD(pRPacket, pub_num	));	///< 발행매수			
		nCount = *(int *) pRPacket->pub_num;

		{
			pRPacket->pList = new rtk_ktcpbtckcsrc_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_ktcpbtckcsrc_list_t) * nCount);
		}

		pPubTr->m_vtPbTckCsrcCard.clear();
		for(i = 0; i < nCount; i++)
		{
			prtk_ktcpbtckcsrc_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(PYN_MNS_DVS_CD		,	PST_FIELD(pList, pyn_mns_dvs_cd		));	///< 지불수단구분코드		
			nRet = TMaxFBGetF(CSRC_APRV_NO			,	PST_FIELD(pList, csrc_aprv_no		));	///< 현금영수증승인번호		
			nRet = TMaxFBGetF(APRV_NO				,	PST_FIELD(pList, aprv_no			));	///< 승인번호			
			nRet = TMaxFBGetF(BUS_TCK_KND_CD		,	PST_FIELD(pList, bus_tck_knd_cd		));	///< 버스티켓종류코드		
			nRet = TMaxFBGetF(PUB_SNO				,	PST_FIELD(pList, pub_sno			));	///< 발행일련번호			
			nRet = TMaxFBGetF(BUS_TCK_INHR_NO		,	PST_FIELD(pList, bus_tck_inhr_no	));	///< 버스티켓고유번호		
			nRet = TMaxFBGetF(TISU_AMT				,	PST_FIELD(pList, tisu_amt			));	///< 발권금액			
			nRet = TMaxFBGetF(SATS_NO				,	PST_FIELD(pList, sats_no			));	///< 좌석번호			
			nRet = TMaxFBGetF(CTY_BUS_DC_KND_CD		,	PST_FIELD(pList, cty_bus_dc_knd_cd	));	///< 시외버스할인종류코드	
			nRet = TMaxFBGetF(DCRT_DVS_CD			,	PST_FIELD(pList, dcrt_dvs_cd		));	///< 할인율구분코드		
			nRet = TMaxFBGetF(DC_BEF_AMT			,	PST_FIELD(pList, dc_bef_amt			));	///< 할인이전금액			
			nRet = TMaxFBGetF(CTY_BUS_OPRN_SHP_CD	,	PST_FIELD(pList, cty_bus_oprn_shp_cd));	///< 시외버스운행형태코드	

			pPubTr->m_vtPbTckCsrcCard.push_back(*pList);
		}
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ", "rsp_cd",			PST_FIELD(pRPacket, rsp_cd				));	///< 응답코드			
		LOG_WRITE("%30s - (%s) ", "rsp_msg",			PST_FIELD(pRPacket, rsp_msg				));	///< 응답 메시지			
		LOG_WRITE("%30s - (%s) ", "CSRC_AUTH_NO",		PST_FIELD(pRPacket, csrc_auth_no		));	///< 현금영수증인증번호		
		LOG_WRITE("%30s - (%s) ", "CSRC_BRN",			PST_FIELD(pRPacket, csrc_brn			));	///< 현금영수증사업자등록번호
		LOG_WRITE("%30s - (%s) ", "CSRC_APRV_VANC_CD",PST_FIELD(pRPacket, csrc_aprv_vanc_cd	));	///< 현금영수증승인van사코드
		LOG_WRITE("%30s - (%s) ", "ATL_DEPR_DT",		PST_FIELD(pRPacket, atl_depr_dt			));	///< 실제출발일자			
		LOG_WRITE("%30s - (%s) ", "ATL_DEPR_TIME",	PST_FIELD(pRPacket, atl_depr_time		));	///< 실제출발시각			
		LOG_WRITE("%30s - (%s) ", "DEPR_TRML_CD",		PST_FIELD(pRPacket, depr_trml_cd		));	///< 출발터미널코드		
		LOG_WRITE("%30s - (%s) ", "ARVL_TRML_CD",		PST_FIELD(pRPacket, arvl_trml_cd		));	///< 도착터미널코드		
		LOG_WRITE("%30s - (%s) ", "STPT_TRML_CD",		PST_FIELD(pRPacket, stpt_trml_cd		));	///< 기점터미널코드		
		LOG_WRITE("%30s - (%s) ", "EPT_TRML_CD",		PST_FIELD(pRPacket, ept_trml_cd			));	///< 종점터미널코드		
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD",		PST_FIELD(pRPacket, bus_cls_cd			));	///< 버스등급코드			
		LOG_WRITE("%30s - (%s) ", "BUS_CACM_CD",		PST_FIELD(pRPacket, bus_cacm_cd			));	///< 버스운수사코드		
		LOG_WRITE("%30s - (%s) ", "RDHM_MLTP_VAL",	PST_FIELD(pRPacket, rdhm_mltp_val		));	///< 승차홈다중값			
		LOG_WRITE("%30s - (number = %d) ", "DIST",	 *(int *) pRPacket->dist				);	///< 거리				
		LOG_WRITE("%30s - (number = %d) ", "TAKE_DRTM", *(int *) pRPacket->take_drtm			);	///< 소요시간			
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_PRIN_YN",	PST_FIELD(pRPacket, bus_cls_prin_yn		));	///< 버스등급출력여부		
		LOG_WRITE("%30s - (%s) ", "BUS_CACM_NM_PRIN_YN",PST_FIELD(pRPacket, bus_cacm_nm_prin_yn	));	///< 버스운수사명출력여부	
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME_PRIN_YN",PST_FIELD(pRPacket, depr_time_prin_yn	));	///< 출발시각출력여부		
		LOG_WRITE("%30s - (%s) ", "SATS_NO_PRIN_YN",	PST_FIELD(pRPacket, sats_no_prin_yn		));	///< 좌석번호출력여부		
		LOG_WRITE("%30s - (%s) ", "ALCN_WAY_DVS_CD",	PST_FIELD(pRPacket, alcn_way_dvs_cd		));	///< 배차방식구분코드		
		LOG_WRITE("%30s - (%s) ", "SATI_USE_YN",		PST_FIELD(pRPacket, sati_use_yn			));	///< 좌석제사용여부		
		LOG_WRITE("%30s - (%s) ", "ROT_KND_CD",		PST_FIELD(pRPacket, rot_knd_cd			));	///< 노선종류코드			
		LOG_WRITE("%30s - (%s) ", "PERD_TEMP_YN",		PST_FIELD(pRPacket, perd_temp_yn		));	///< 정기임시여부			
		LOG_WRITE("%30s - (%s) ", "PUB_DT",			PST_FIELD(pRPacket, pub_dt				));	///< 발행일자			
		LOG_WRITE("%30s - (%s) ", "PUB_TIME",			PST_FIELD(pRPacket, pub_time			));	///< 발행시각			
		LOG_WRITE("%30s - (%s) ", "PUB_SHCT_TRML_CD",	PST_FIELD(pRPacket, pub_shct_trml_cd	));	///< 발행단축터미널코드		
		LOG_WRITE("%30s - (%s) ", "PUB_WND_NO",		PST_FIELD(pRPacket, pub_wnd_no			));	///< 발행창구번호			
		LOG_WRITE("%30s - (number = %d) ", "PUB_NUM",	*(int *) pRPacket->pub_num				);	///< 발행매수			
			
		nCount = *(int *) pRPacket->pub_num;
		for(i = 0; i < nCount; i++)
		{
			prtk_ktcpbtckcsrc_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE(" 순번 ( %d / %d )", i + 1, nCount);

			LOG_WRITE("%30s - (%s) ", "PYN_MNS_DVS_CD",		PST_FIELD(pList, pyn_mns_dvs_cd		));	///< 지불수단구분코드		
			LOG_WRITE("%30s - (%s) ", "CSRC_APRV_NO",			PST_FIELD(pList, csrc_aprv_no		));	///< 현금영수증승인번호		
			LOG_WRITE("%30s - (%s) ", "APRV_NO",				PST_FIELD(pList, aprv_no			));	///< 승인번호			
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD",		PST_FIELD(pList, bus_tck_knd_cd		));	///< 버스티켓종류코드		
			LOG_WRITE("%30s - (number = %d) ", "PUB_SNO",		*(int *) pList->pub_sno				);	///< 발행일련번호			
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_INHR_NO",		PST_FIELD(pList, bus_tck_inhr_no	));	///< 버스티켓고유번호		
			LOG_WRITE("%30s - (number = %d) ", "TISU_AMT",	*(int *) pList->tisu_amt			);	///< 발권금액			
			LOG_WRITE("%30s - (number = %d) ", "SATS_NO",		*(int *) pList->sats_no				);	///< 좌석번호			
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_DC_KND_CD",	PST_FIELD(pList, cty_bus_dc_knd_cd	));	///< 시외버스할인종류코드	
			LOG_WRITE("%30s - (%s) ", "DCRT_DVS_CD",			PST_FIELD(pList, dcrt_dvs_cd			));	///< 할인율구분코드		
			LOG_WRITE("%30s - (number = %d) ", "DC_BEF_AMT",	*(int *) pList->dc_bef_amt			);	///< 할인이전금액			
			LOG_WRITE("%30s - (%s) ", "CTY_BUS_OPRN_SHP_CD",	PST_FIELD(pList, cty_bus_oprn_shp_cd	));	///< 시외버스운행형태코드	
		}
	}

	m_bVerify = FALSE;
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:
	m_bVerify = FALSE;
	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

#else
	return 0;
#endif
}

/**
 * @brief		SVC_TK_ReadRsd
 * @details		(225) 상주직원 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::SVC_TK_ReadRsd(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, n_rsp_cd, nLen;
	char*	pService = "TK_ReadRsd";
	pstk_readrsd_t		pSPacket;
	prtk_readrsd_t		pRPacket;
	CPubTckMem* pPubTr;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pPubTr = CPubTckMem::GetInstance();

	pSPacket = (pstk_readrsd_t) pData;
	pRPacket = (prtk_readrsd_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ","READ_WND_DVS"	,	PST_FIELD(pSPacket, read_wnd_dvs	));
		LOG_WRITE("%30s - (%s) ","USER_DVS_NO"	,	PST_FIELD(pSPacket, user_dvs_no		));
		LOG_WRITE("%30s - (%s) ","USER_PWD"		,	PST_FIELD(pSPacket, user_pwd		));
		LOG_WRITE("%30s - (%s) ","TISU_REQ_YN"	,	PST_FIELD(pSPacket, tisu_req_yn		));
		LOG_WRITE("%30s - (%s) ","ROT_ID"		,	PST_FIELD(pSPacket, rot_id			));
		LOG_WRITE("%30s - (%s) ","ROT_SQNO"		,	PST_FIELD(pSPacket, rot_sqno		));
		LOG_WRITE("%30s - (%s) ","ALCN_DT"		,	PST_FIELD(pSPacket, alcn_dt			));
		LOG_WRITE("%30s - (%s) ","ALCN_SQNO"	,	PST_FIELD(pSPacket, alcn_sqno		));
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(READ_WND_DVS		,	PST_FIELD(pSPacket,	read_wnd_dvs		));	///< 조회 창구 구분			
		nRet = TMaxFBPut(USER_DVS_NO		,	PST_FIELD(pSPacket,	user_dvs_no			));	///< 사용자구분번호			
		nRet = TMaxFBPut(USER_PWD			,	PST_FIELD(pSPacket,	user_pwd			));	///< 사용자비밀번호			
		nRet = TMaxFBPut(TISU_REQ_YN		,	PST_FIELD(pSPacket,	tisu_req_yn			));	///< 발권요청여부			
		nRet = TMaxFBPut(ROT_ID				,	PST_FIELD(pSPacket,	rot_id				));	///< 노선id					
		nRet = TMaxFBPut(ROT_SQNO			,	PST_FIELD(pSPacket,	rot_sqno			));	///< 노선순번				
		nRet = TMaxFBPut(ALCN_DT			,	PST_FIELD(pSPacket,	alcn_dt				));	///< 배차일자				
		nRet = TMaxFBPut(ALCN_SQNO			,	PST_FIELD(pSPacket,	alcn_sqno			));	///< 배차순번				
	}

	// 동기화 (Send/Recv)
	nTimeout = 45;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	LOG_WRITE("TMaxTpCall(), Success !!!!");

	///> GetData
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 141 )
		{
			if( n_rsp_cd == 666 )
			{
				// 상주직원정보가 존재하지 않습니다.
				nRet = -4;
				LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
				CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
				goto fail_proc;
			}
			else if( n_rsp_cd == 10 )
			{
				// 비밀번호가 틀렸습니다. TKI991
				nRet = -4;
				LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
				CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
				goto fail_proc;
			}
			else if( n_rsp_cd == 671 )
			{
				// 상주직원 할인 결제는 동일 배차에 1회에 한해서만 가능합니다.
				nRet = -4;
				LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
				CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
				goto fail_proc;
			}
			else if( n_rsp_cd == 672 )
			{
				// 비밀번호가 등록되지 않았습니다. 유인창구에서 등록하여 주십시오.
				nRet = -4;
				LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
				CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
				goto fail_proc;
			}
		}

		nRet = TMaxFBGetF(RSD_USER_ID			,	PST_FIELD(pRPacket,	rsd_user_id			));	///< 상주사용자id	
		nRet = TMaxFBGetF(USER_NM				,	PST_FIELD(pRPacket,	user_nm				));	///< 사용자명		
		nRet = TMaxFBGetF(CMPY_NM				,	PST_FIELD(pRPacket,	cmpy_nm				));	///< 회사명			
		nRet = TMaxFBGetF(NTFC_CTT				,	PST_FIELD(pRPacket,	ntfc_ctt			));	///< 알림내용		
		nRet = TMaxFBGetF(TISSU_UNBL_ROT_VAL	,	PST_FIELD(pRPacket,	tissu_unbl_rot_val	));	///< 발권불가노선값	
		nRet = TMaxFBGetF(SELL_YN				,	PST_FIELD(pRPacket,	sell_yn				));	///< 판매여부		
		nRet = TMaxFBGetF(DLT_YN				,	PST_FIELD(pRPacket,	dlt_yn				));	///< 삭제여부		
		nRet = TMaxFBGetF(PUB_DT				,	PST_FIELD(pRPacket,	pub_dt				));	///< 발행일자		
		nRet = TMaxFBGetF(PUB_TIME				,	PST_FIELD(pRPacket,	pub_time			));	///< 발행시각		
		nRet = TMaxFBGetF(PUB_USER_ID			,	PST_FIELD(pRPacket,	pub_user_id			));	///< 발행사용자id	
		nRet = TMaxFBGetF(PUB_USER_NM			,	PST_FIELD(pRPacket,	pub_user_nm			));	///< 발행사용자명	
		nRet = TMaxFBGetF(USER_DVS_NO			,	PST_FIELD(pRPacket,	user_dvs_no			));	///< 사용자구분번호	
		nRet = TMaxFBGetF(PWD_RGT_YN			,	PST_FIELD(pRPacket,	pwd_rgt_yn			));	///< 비밀번호등록여부

		if(1)
		{
			int i, k;

			/// 상주직원 이름
			sprintf(pPubTr->base.rsd_nm, "%s", pRPacket->user_nm);
			
			/// 상주직원 전화번호
			::ZeroMemory(pPubTr->base.rsd_tel_nm, sizeof(pPubTr->base.rsd_tel_nm));
			nLen = strlen(pRPacket->user_dvs_no);
			for(k = 0, i = 0; i < nLen; i++)
			{
				switch(pRPacket->user_dvs_no[i] & 0xFF)
				{
				case 0x20:
					break;
				default:
					pPubTr->base.rsd_tel_nm[k++] = pRPacket->user_dvs_no[i];
					break;
				}
			}
			/// 상주직원 소속명
			sprintf(pPubTr->base.rsd_cmpy_nm, "%s", pRPacket->cmpy_nm);
		}

		/// 상주직원 정보 저장
		::CopyMemory((BYTE *)&pPubTr->m_resp_readrsd_t, (BYTE *)pRPacket, sizeof(rtk_readrsd_t));
	}

	nRet = 1;

	///> Log out
	{
		LOG_WRITE("%30s - (%s) ", "RSD_USER_ID"			, PST_FIELD(pRPacket,	rsd_user_id			));	///< 
		LOG_WRITE("%30s - (%s) ", "USER_NM"				, PST_FIELD(pRPacket,	user_nm				));	///< 
		LOG_WRITE("%30s - (%s) ", "CMPY_NM"				, PST_FIELD(pRPacket,	cmpy_nm				));	///< 
		LOG_WRITE("%30s - (%s) ", "NTFC_CTT"			, PST_FIELD(pRPacket,	ntfc_ctt			));	///< 
		LOG_WRITE("%30s - (%s) ", "TISSU_UNBL_ROT_VAL"	, PST_FIELD(pRPacket,	tissu_unbl_rot_val	));	///< 
		LOG_WRITE("%30s - (%s) ", "SELL_YN"				, PST_FIELD(pRPacket,	sell_yn				));	///< 
		LOG_WRITE("%30s - (%s) ", "DLT_YN"				, PST_FIELD(pRPacket,	dlt_yn				));	///< 
		LOG_WRITE("%30s - (%s) ", "PUB_DT"				, PST_FIELD(pRPacket,	pub_dt				));	///< 
		LOG_WRITE("%30s - (%s) ", "PUB_TIME"			, PST_FIELD(pRPacket,	pub_time			));	///< 
		LOG_WRITE("%30s - (%s) ", "PUB_USER_ID"			, PST_FIELD(pRPacket,	pub_user_id			));	///< 
		LOG_WRITE("%30s - (%s) ", "PUB_USER_NM"			, PST_FIELD(pRPacket,	pub_user_nm			));	///< 
		LOG_WRITE("%30s - (%s) ", "USER_DVS_NO"			, PST_FIELD(pRPacket,	user_dvs_no			));	///< 
		LOG_WRITE("%30s - (%s) ", "PWD_RGT_YN"			, PST_FIELD(pRPacket,	pwd_rgt_yn			));	///< 
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc228
 * @details		(228) 좌석 요금 정보 변경,  국민카드 상근자 카드 일경우 좌석 재 선점 서비스 
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc228(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout, n_rsp_cd;
	char*	pService = "TK_ModPcpySats";
	pstk_modpcpysats_t		pSPacket;
	prtk_modpcpysats_t		pRPacket;
	prtk_modpcpysats_list_t pList;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_modpcpysats_t) pData;
	pRPacket = (prtk_modpcpysats_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ",		"ROT_ID"				,	PST_FIELD(pSPacket, rot_id					));
		LOG_WRITE("%30s - (num=%d) ",	"ROT_SQNO"				,	*(int *)pSPacket->rot_sqno					);
		LOG_WRITE("%30s - (%s) ",		"ALCN_DT"				,	PST_FIELD(pSPacket, alcn_dt					));
		LOG_WRITE("%30s - (num=%d) ",	"ALCN_SQNO"				,	*(int *)pSPacket->alcn_sqno					);
		LOG_WRITE("%30s - (%s) ",		"DEPR_TRML_CD"			,	PST_FIELD(pSPacket, depr_trml_cd			));
		LOG_WRITE("%30s - (%s) ",		"ARVL_TRML_CD"			,	PST_FIELD(pSPacket, arvl_trml_cd			));
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ",		"DAMO_ENC_CARD_NO"		,	PST_FIELD(pSPacket, damo_enc_card_no		));
		LOG_WRITE("%30s - (num=%d)",	"DAMO_ENC_CARD_NO_LEN"	,	*(int *)pSPacket->damo_enc_card_no_len		);
		LOG_WRITE("%30s - (%s)",		"DAMO_ENC_DTA_KEY"		,	PST_FIELD(pSPacket, damo_enc_dta_key		));
#endif
		LOG_WRITE("%30s - (num=%d)",	"PUB_NUM"				,	*(int *)pSPacket->pub_num					);

		nCount = *(int *)pSPacket->pub_num;
		for(i = 0; i < nCount; i++)
		{
			stk_modpcpysats_list_t* pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("%30s - (%d) ", "Index", i);
			LOG_WRITE("%30s - (%s) ", "SATS_PCPY_ID"	,	PST_FIELD(pList, sats_pcpy_id		));
			LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD"	,	PST_FIELD(pList, bus_tck_knd_cd		));
			LOG_WRITE("%30s - (num=%d) ", "SATS_NO"			,	*(int *)pList->sats_no				);
		}
	}


	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(ROT_ID					,	PST_FIELD(pSPacket, rot_id					));	///< 노선id					
		nRet = TMaxFBPut(ROT_SQNO				,	PST_FIELD(pSPacket, rot_sqno				));	///< 노선순번				
		nRet = TMaxFBPut(ALCN_DT				,	PST_FIELD(pSPacket, alcn_dt					));	///< 배차일자				
		nRet = TMaxFBPut(ALCN_SQNO				,	PST_FIELD(pSPacket, alcn_sqno				));	///< 배차순번				
		nRet = TMaxFBPut(DEPR_TRML_CD			,	PST_FIELD(pSPacket, depr_trml_cd			));	///< 출발터미널코드			
		nRet = TMaxFBPut(ARVL_TRML_CD			,	PST_FIELD(pSPacket, arvl_trml_cd			));	///< 도착터미널코드			
		nRet = TMaxFBPut(DAMO_ENC_CARD_NO		,	PST_FIELD(pSPacket, damo_enc_card_no		));	///< 카드번호암호문			
		nRet = TMaxFBPut(DAMO_ENC_CARD_NO_LEN	,	PST_FIELD(pSPacket, damo_enc_card_no_len	));	///< 카드번호암호문길이		
		nRet = TMaxFBPut(DAMO_ENC_DTA_KEY		,	PST_FIELD(pSPacket, damo_enc_dta_key		));	///< 암호문키				
		nRet = TMaxFBPut(PUB_NUM				,	PST_FIELD(pSPacket, pub_num					));	///< 발행매수				
		nCount = *(int *)pSPacket->pub_num;
		for(i = 0; i < nCount; i++)
		{
			stk_modpcpysats_list_t* pList;

			pList = &pSPacket->List[i];

			nRet = TMaxFBInsert(BUS_TCK_KND_CD	, PST_FIELD(pList, bus_tck_knd_cd), i, sizeof(pList->bus_tck_knd_cd) - 1);
			nRet = TMaxFBInsert(SATS_PCPY_ID	, PST_FIELD(pList, sats_pcpy_id),	i, sizeof(pList->sats_pcpy_id) - 1);
			nRet = TMaxFBInsert(SATS_NO			, PST_FIELD(pList, sats_no),		i, sizeof(pList->sats_no) - 1);
		}
	}

	// 동기화 (Send/Recv)
	nTimeout = 45;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	LOG_WRITE("TMaxTpCall(), Success !!!!");

	///> GetData
	{
		CPubTckMem* pTr;

		pTr = CPubTckMem::GetInstance();

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		LOG_WRITE("%30s - (%s) ", "RSP_CD",		pRPacket->rsp_cd);	///< 
		if( n_rsp_cd != 146 )
		{
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			if( n_rsp_cd == 669 )
			{  /// 국민카드 상주할인 노선이 아닐경우 일반요금으로 결제 
				LOG_WRITE("국민카드 상주할인 노선이 아닐경우 일반요금으로 결제 !!!!");	///< 
// 				nRet = 0;
// 				goto ok_proc;
			}
			goto fail_proc;
		}

		nRet = TMaxFBGetF(SATS_PCPY_NUM	,	PST_FIELD(pRPacket, sats_pcpy_num	));	///< 좌석 선점 수			
		nCount = *(int *)pRPacket->sats_pcpy_num;

		{
			pRPacket->pList = new rtk_modpcpysats_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_modpcpysats_list_t) * nCount);
		}

		pTr->m_vtPcpysats.clear();

		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(SATS_PCPY_ID			,	PST_FIELD(pList, sats_pcpy_id			));	///< 좌석선점id						
			nRet = TMaxFBGetF(CTY_BUS_DC_KND_CD		,	PST_FIELD(pList, cty_bus_dc_knd_cd		));	///< 시외버스할인종류코드				
			nRet = TMaxFBGetF(DCRT_DVS_CD			,	PST_FIELD(pList, dcrt_dvs_cd			));	///< 할인율구분코드					
			nRet = TMaxFBGetF(BUS_TCK_KND_CD		,	PST_FIELD(pList, bus_tck_knd_cd			));	///< 버스티켓종류코드					
			nRet = TMaxFBGetF(DC_AFT_AMT			,	PST_FIELD(pList, dc_aft_amt				));	///< 할인이후금액					
			nRet = TMaxFBGetF(SATS_NO				,	PST_FIELD(pList, sats_no				));	///< 좌석번호						

			pTr->m_vtPcpysats.push_back(*(prtk_pcpysats_list_t)pList);
		}

		nRet = nCount;
	}

	///> Log out
	{
		for(i = 0; i < nCount; i++)
		{
			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ",		"SATS_PCPY_ID",			PST_FIELD(pList, sats_pcpy_id		));	///< 좌석선점id			
			LOG_WRITE("%30s - (%s) ",		"CTY_BUS_DC_KND_CD",	PST_FIELD(pList, cty_bus_dc_knd_cd	));	///< 시외버스할인종류코드	
			LOG_WRITE("%30s - (%s) ",		"DCRT_DVS_CD",			PST_FIELD(pList, dcrt_dvs_cd		));	///< 할인율구분코드		
			LOG_WRITE("%30s - (%s) ",		"BUS_TCK_KND_CD",		PST_FIELD(pList, bus_tck_knd_cd		));	///< 버스티켓종류코드		
			LOG_WRITE("%30s - (num=%d) ",	"DC_AFT_AMT",			*(int *)pList->dc_aft_amt			);	///< 할인이후금액		
			LOG_WRITE("%30s - (num=%d) ",	"SATS_NO",				*(int *)pList->sats_no				);	///< 좌석번호			
		}
	}

//ok_proc:
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc230
 * @details		(230) TMAX 초기화 서비스
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc230(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout;
	char*	pService = "TK_KtcTmxClr";
	pstk_ktctmxclr_t		pSPacket;
	prtk_ktctmxclr_t		pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_ktctmxclr_t) pData;
	pRPacket = (prtk_ktctmxclr_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ", "ENC_DTA"	,	PST_FIELD(pSPacket, enc_dta	));
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(ENC_DTA,	PST_FIELD(pSPacket, enc_dta					));	///< ENC DATA
	}

	// 동기화 (Send/Recv)
	nTimeout = 45;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	LOG_WRITE("TMaxTpCall(), Success !!!!");

	///> GetData
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc260
 * @details		(260) 승차권 발행(선불)
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc260(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, n_rsp_cd;
	char*	pService = "TK_PubTckPpy";
	pstk_pubtckppy_t		pSPacket;
	prtk_pubtckppy_t		pRPacket;

	n_rsp_cd = 0;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_pubtckppy_t) pData;
	pRPacket = (prtk_pubtckppy_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ", "ROT_ID"					,	PST_FIELD(pSPacket, rot_id				));
		LOG_WRITE("%30s - (num=%d) ", "ROT_SQNO"			,	*(int *)pSPacket->rot_sqno				);
		LOG_WRITE("%30s - (%s) ", "ALCN_DT"					,	PST_FIELD(pSPacket, alcn_dt				));
		LOG_WRITE("%30s - (num=%d) ", "ALCN_SQNO"			,	*(int *)pSPacket->alcn_sqno				);
		LOG_WRITE("%30s - (%s) ", "DEPR_TRML_CD"			,	PST_FIELD(pSPacket, depr_trml_cd		));
		LOG_WRITE("%30s - (%s) ", "ARVL_TRML_CD"			,	PST_FIELD(pSPacket, arvl_trml_cd		));
		LOG_WRITE("%30s - (%s) ", "TISU_WAY_DVS_CD"			,	PST_FIELD(pSPacket, tisu_way_dvs_cd		));
		LOG_WRITE("%30s - (%s) ", "READ_ROT_YN"				,	PST_FIELD(pSPacket, read_rot_yn			));
		LOG_WRITE("%30s - (num=%d) ", "PUB_NUM"				,	*(int *)pSPacket->pub_num				);
		LOG_WRITE("%30s - (%s) ", "PYN_MNS_DVS_CD"			,	PST_FIELD(pSPacket, pyn_mns_dvs_cd		));
		LOG_WRITE("%30s - (%s) ", "PUB_CHNL_DVS_CD"			,	PST_FIELD(pSPacket, pub_chnl_dvs_cd		));
		LOG_WRITE("%30s - (%s) ", "USER_DVS_NO"				,	PST_FIELD(pSPacket, user_dvs_no			));
		LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD"			,	PST_FIELD(pSPacket, bus_tck_knd_cd		));
		LOG_WRITE("%30s - (num=%d) ", "SATS_NO"				,	*(int *)pSPacket->sats_no				);
		LOG_WRITE("%30s - (%s) ", "SATS_PCPY_ID"			,	PST_FIELD(pSPacket, sats_pcpy_id		));
		LOG_WRITE("%30s - (%s) ", "OLD_MBRS_NO"				,	PST_FIELD(pSPacket, old_mbrs_no			));
		LOG_WRITE("%30s - (%s) ", "MRNP_ID"					,	PST_FIELD(pSPacket, mrnp_id				));
		LOG_WRITE("%30s - (%s) ", "MRNP_NUM"				,	PST_FIELD(pSPacket, mrnp_num			));
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ", "DAMO_ENC_CARD_NO"		,	PST_FIELD(pSPacket, damo_enc_card_no	));
		LOG_WRITE("%30s - (num=%d) ", "DAMO_ENC_CARD_NO_LEN",	*(int *)pSPacket->damo_enc_card_no_len	);
		LOG_WRITE("%30s - (%s) ", "DAMO_ENC_DTA_KEY"		,	PST_FIELD(pSPacket, damo_enc_dta_key	));
#endif		
		LOG_WRITE("%30s - (%s) ", "PPY_TAK_DVS_CD"			,	PST_FIELD(pSPacket, ppy_tak_dvs_cd		));
		LOG_WRITE("%30s - (%s) ", "PPY_POS_SLS_DT"			,	PST_FIELD(pSPacket, ppy_pos_sls_dt		));
		LOG_WRITE("%30s - (%s) ", "PPY_POS_RECP_NO"			,	PST_FIELD(pSPacket, ppy_pos_recp_no		));
		LOG_WRITE("%30s - (%s) ", "PPY_SAM_ID"				,	PST_FIELD(pSPacket, ppy_sam_id			));
		LOG_WRITE("%30s - (%s) ", "PPY_SAM_TRD_SNO"			,	PST_FIELD(pSPacket, ppy_sam_trd_sno		));
		LOG_WRITE("%30s - (%s) ", "PPY_CARD_TRD_SNO"		,	PST_FIELD(pSPacket, ppy_card_trd_sno	));
		LOG_WRITE("%30s - (num=%d) ", "PPY_APRV_AFT_BAL"	,	*(int *)pSPacket->ppy_aprv_aft_bal		);
		LOG_WRITE("%30s - (num=%d) ", "PPY_APRV_BEF_BAL"	,	*(int *)pSPacket->ppy_aprv_bef_bal		);
		LOG_WRITE("%30s - (%s) ", "PPY_ALGR_ID"				,	PST_FIELD(pSPacket, ppy_algr_id			));
		LOG_WRITE("%30s - (%s) ", "PPY_SIGN_VAL"			,	PST_FIELD(pSPacket, ppy_sign_val		));
		LOG_WRITE("%30s - (%s) ", "PPY_INDV_TRD_CLK_VER"	,	PST_FIELD(pSPacket, ppy_indv_trd_clk_ver));
		LOG_WRITE("%30s - (%s) ", "PPY_ELCS_IDNT_ID"		,	PST_FIELD(pSPacket, ppy_elcs_idnt_id	));
		LOG_WRITE("%30s - (%s) ", "SAM_TTAM_CLCN_SNO"		,	PST_FIELD(pSPacket, sam_ttam_clcn_sno	));
		LOG_WRITE("%30s - (%s) ", "SAM_INDV_CLCN_NCNT"		,	PST_FIELD(pSPacket, sam_indv_clcn_ncnt	));
		LOG_WRITE("%30s - (%s) ", "SAM_CUM_TRD_TTAM"		,	PST_FIELD(pSPacket, sam_cum_trd_ttam	));
		LOG_WRITE("%30s - (%s) ", "PPY_CARD_DVS_CD"			,	PST_FIELD(pSPacket, ppy_card_dvs_cd		));
		LOG_WRITE("%30s - (%s) ", "PPY_CARD_USER_DVS_CD"	,	PST_FIELD(pSPacket, ppy_card_user_dvs_cd));
		LOG_WRITE("%30s - (%s) ", "PPY_HSM_STA_CD"			,	PST_FIELD(pSPacket, ppy_hsm_sta_cd		));
		LOG_WRITE("%30s - (num=%d) ", "REQ_TRD_AMT"			,	*(int *)pSPacket->req_trd_amt			);
		
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(ROT_ID					,	PST_FIELD(pSPacket,	rot_id					));	///< 
		nRet = TMaxFBPut(ROT_SQNO				,	PST_FIELD(pSPacket,	rot_sqno				));	///< 
		nRet = TMaxFBPut(ALCN_DT				,	PST_FIELD(pSPacket,	alcn_dt					));	///< 
		nRet = TMaxFBPut(ALCN_SQNO				,	PST_FIELD(pSPacket,	alcn_sqno				));	///< 
		nRet = TMaxFBPut(DEPR_TRML_CD			,	PST_FIELD(pSPacket,	depr_trml_cd			));	///< 
		nRet = TMaxFBPut(ARVL_TRML_CD			,	PST_FIELD(pSPacket,	arvl_trml_cd			));	///< 
		nRet = TMaxFBPut(TISU_WAY_DVS_CD		,	PST_FIELD(pSPacket,	tisu_way_dvs_cd			));	///< 
		nRet = TMaxFBPut(READ_ROT_YN			,	PST_FIELD(pSPacket,	read_rot_yn				));	///< 
		nRet = TMaxFBPut(PUB_NUM				,	PST_FIELD(pSPacket,	pub_num					));	///< 
		nRet = TMaxFBPut(PYN_MNS_DVS_CD			,	PST_FIELD(pSPacket,	pyn_mns_dvs_cd			));	///< 
		nRet = TMaxFBPut(PUB_CHNL_DVS_CD		,	PST_FIELD(pSPacket,	pub_chnl_dvs_cd			));	///< 
		nRet = TMaxFBPut(USER_DVS_NO			,	PST_FIELD(pSPacket,	user_dvs_no				));	///< 
		nRet = TMaxFBPut(BUS_TCK_KND_CD			,	PST_FIELD(pSPacket,	bus_tck_knd_cd			));	///< 
		nRet = TMaxFBPut(SATS_NO				,	PST_FIELD(pSPacket,	sats_no					));	///< 
		nRet = TMaxFBPut(SATS_PCPY_ID			,	PST_FIELD(pSPacket,	sats_pcpy_id			));	///< 
		nRet = TMaxFBPut(OLD_MBRS_NO			,	PST_FIELD(pSPacket,	old_mbrs_no				));	///< 
		nRet = TMaxFBPut(MRNP_ID				,	PST_FIELD(pSPacket,	mrnp_id					));	///< 
		nRet = TMaxFBPut(MRNP_NUM				,	PST_FIELD(pSPacket,	mrnp_num				));	///< 
		nRet = TMaxFBPut(DAMO_ENC_CARD_NO		,	PST_FIELD(pSPacket,	damo_enc_card_no		));	///< 
		nRet = TMaxFBPut(DAMO_ENC_CARD_NO_LEN	,	PST_FIELD(pSPacket,	damo_enc_card_no_len	));	///< 
		nRet = TMaxFBPut(DAMO_ENC_DTA_KEY		,	PST_FIELD(pSPacket,	damo_enc_dta_key		));	///< 
		nRet = TMaxFBPut(PPY_TAK_DVS_CD			,	PST_FIELD(pSPacket,	ppy_tak_dvs_cd			));	///< 
		nRet = TMaxFBPut(PPY_POS_SLS_DT			,	PST_FIELD(pSPacket,	ppy_pos_sls_dt			));	///< 
		nRet = TMaxFBPut(PPY_POS_RECP_NO		,	PST_FIELD(pSPacket,	ppy_pos_recp_no			));	///< 
		nRet = TMaxFBPut(PPY_SAM_ID				,	PST_FIELD(pSPacket,	ppy_sam_id				));	///< 
		nRet = TMaxFBPut(PPY_SAM_TRD_SNO		,	PST_FIELD(pSPacket,	ppy_sam_trd_sno			));	///< 
		nRet = TMaxFBPut(PPY_CARD_TRD_SNO		,	PST_FIELD(pSPacket,	ppy_card_trd_sno		));	///< 
		nRet = TMaxFBPut(PPY_APRV_AFT_BAL		,	PST_FIELD(pSPacket,	ppy_aprv_aft_bal		));	///< 
		nRet = TMaxFBPut(PPY_APRV_BEF_BAL		,	PST_FIELD(pSPacket,	ppy_aprv_bef_bal		));	///< 
		nRet = TMaxFBPut(PPY_ALGR_ID			,	PST_FIELD(pSPacket,	ppy_algr_id				));	///< 
		nRet = TMaxFBPut(PPY_SIGN_VAL			,	PST_FIELD(pSPacket,	ppy_sign_val			));	///< 
		nRet = TMaxFBPut(PPY_INDV_TRD_CLK_VER	,	PST_FIELD(pSPacket,	ppy_indv_trd_clk_ver	));	///< 
		nRet = TMaxFBPut(PPY_ELCS_IDNT_ID		,	PST_FIELD(pSPacket,	ppy_elcs_idnt_id		));	///< 
		nRet = TMaxFBPut(SAM_TTAM_CLCN_SNO		,	PST_FIELD(pSPacket,	sam_ttam_clcn_sno		));	///< 
		nRet = TMaxFBPut(SAM_INDV_CLCN_NCNT		,	PST_FIELD(pSPacket,	sam_indv_clcn_ncnt		));	///< 
		nRet = TMaxFBPut(SAM_CUM_TRD_TTAM		,	PST_FIELD(pSPacket,	sam_cum_trd_ttam		));	///< 
		nRet = TMaxFBPut(PPY_CARD_DVS_CD		,	PST_FIELD(pSPacket,	ppy_card_dvs_cd			));	///< 
		nRet = TMaxFBPut(PPY_CARD_USER_DVS_CD	,	PST_FIELD(pSPacket,	ppy_card_user_dvs_cd	));	///< 
		nRet = TMaxFBPut(PPY_HSM_STA_CD			,	PST_FIELD(pSPacket,	ppy_hsm_sta_cd			));	///< 
		nRet = TMaxFBPut(REQ_TRD_AMT			,	PST_FIELD(pSPacket,	req_trd_amt				));	///< 
	}

	// 동기화 (Send/Recv)
	nTimeout = 45;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	LOG_WRITE("TMaxTpCall(), Success !!!!");

	///> GetData
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));

		// strcpy(pRPacket->rsp_cd, "SVE442"); // RF선불교통카드 장애발생 TEST /// 20230222

		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 146 )
		{
			nRet = -5;
			LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			
			///< RF 선불카드 서버 실패 응답 데이타
			CPubTckMem::GetInstance()->m_vtRfResp.push_back(*pRPacket);

			goto fail_proc;
		}

		//::ZeroMemory(pRPacket->rsp_cd, sizeof(pRPacket->rsp_cd));

		nRet = TMaxFBGetF(PUB_DT				,	pRPacket->pub_dt				);	///< 발행일자			
		nRet = TMaxFBGetF(PUB_TIME				,	pRPacket->pub_time				);	///< 발행시각			
		nRet = TMaxFBGetF(DEPR_TRML_CD			,	pRPacket->depr_trml_cd			);	///< 출발터미널코드		
		nRet = TMaxFBGetF(ARVL_TRML_CD			,	pRPacket->arvl_trml_cd			);	///< 도착터미널코드		
		nRet = TMaxFBGetF(BUS_CLS_CD			,	pRPacket->bus_cls_cd			);	///< 버스등급코드			
		nRet = TMaxFBGetF(BUS_CACM_CD			,	pRPacket->bus_cacm_cd			);	///< 버스운수사코드		
		nRet = TMaxFBGetF(PUB_WND_NO			,	pRPacket->pub_wnd_no			);	///< 발행창구번호			
		nRet = TMaxFBGetF(PUB_SNO				,	pRPacket->pub_sno				);	///< 발행일련번호			
		nRet = TMaxFBGetF(PUB_SHCT_TRML_CD		,	pRPacket->pub_shct_trml_cd		);	///< 발행단축터미널코드		
		nRet = TMaxFBGetF(BUS_TCK_KND_CD		,	pRPacket->bus_tck_knd_cd		);	///< 버스티켓종류코드		
		nRet = TMaxFBGetF(TISU_AMT				,	pRPacket->tisu_amt				);	///< 발권금액			
		nRet = TMaxFBGetF(PYN_MNS_DVS_CD		,	pRPacket->pyn_mns_dvs_cd		);	///< 지불수단구분코드		
		nRet = TMaxFBGetF(SATS_NO				,	pRPacket->sats_no				);	///< 좌석번호			
		nRet = TMaxFBGetF(ROT_KND_CD			,	pRPacket->rot_knd_cd			);	///< 노선종류코드			
		nRet = TMaxFBGetF(CTY_BUS_DC_KND_CD		,	pRPacket->cty_bus_dc_knd_cd		);	///< 시외버스할인종류코드	
		nRet = TMaxFBGetF(DC_BEF_AMT			,	pRPacket->dc_bef_amt			);	///< 할인이전금액			
		nRet = TMaxFBGetF(BUS_TCK_INHR_NO		,	pRPacket->bus_tck_inhr_no		);	///< 버스티켓고유번호		
		nRet = TMaxFBGetF(CTY_BUS_OPRN_SHP_CD	,	pRPacket->cty_bus_oprn_shp_cd	);	///< 시외버스운행형태코드	
		nRet = TMaxFBGetF(ATL_DEPR_DT			,	pRPacket->atl_depr_dt			);	///< 실제출발일자			
		nRet = TMaxFBGetF(ATL_DEPR_TIME			,	pRPacket->atl_depr_time			);	///< 실제출발시각			
		nRet = TMaxFBGetF(STPT_TRML_CD			,	pRPacket->stpt_trml_cd			);	///< 기점터미널코드		
		nRet = TMaxFBGetF(EPT_TRML_CD			,	pRPacket->ept_trml_cd			);	///< 종점터미널코드		
		nRet = TMaxFBGetF(ALCN_WAY_DVS_CD		,	pRPacket->alcn_way_dvs_cd		);	///< 배차방식구분코드		
		nRet = TMaxFBGetF(SATI_USE_YN			,	pRPacket->sati_use_yn			);	///< 좌석제사용여부		
		nRet = TMaxFBGetF(PERD_TEMP_YN			,	pRPacket->perd_temp_yn			);	///< 정기임시여부			
		nRet = TMaxFBGetF(RDHM_MLTP_VAL			,	pRPacket->rdhm_mltp_val			);	///< 승차홈다중값			
		nRet = TMaxFBGetF(BUS_CLS_PRIN_YN		,	pRPacket->bus_cls_prin_yn		);	///< 버스등급출력여부		
		nRet = TMaxFBGetF(BUS_CACM_NM_PRIN_YN	,	pRPacket->bus_cacm_nm_prin_yn	);	///< 버스운수사명출력여부	
		nRet = TMaxFBGetF(DEPR_TIME_PRIN_YN		,	pRPacket->depr_time_prin_yn		);	///< 출발시각출력여부		
		nRet = TMaxFBGetF(SATS_NO_PRIN_YN		,	pRPacket->sats_no_prin_yn		);	///< 좌석번호출력여부		
		nRet = TMaxFBGetF(DIST					,	pRPacket->dist					);	///< 거리				
		nRet = TMaxFBGetF(TAKE_DRTM				,	pRPacket->take_drtm				);	///< 소요시간			
		nRet = TMaxFBGetF(PUB_NUM				,	pRPacket->pub_num				);	///< 발행매수			
		nRet = TMaxFBGetF(DCRT_DVS_CD			,	pRPacket->dcrt_dvs_cd			);	///< 할인율구분코드		

		///< RF 선불카드 서버 성공 응답 데이타
		CPubTckMem::GetInstance()->m_vtRfResp.push_back(*pRPacket);
	}

	{
		LOG_WRITE("%30s - (%s) ", "PUB_DT				",		PST_FIELD(pRPacket, pub_dt					));	///< 발행일자			
		LOG_WRITE("%30s - (%s) ", "PUB_TIME				",		PST_FIELD(pRPacket, pub_time				));	///< 발행시각			
		LOG_WRITE("%30s - (%s) ", "DEPR_TRML_CD			",		PST_FIELD(pRPacket, depr_trml_cd			));	///< 출발터미널코드		
		LOG_WRITE("%30s - (%s) ", "ARVL_TRML_CD			",		PST_FIELD(pRPacket, arvl_trml_cd			));	///< 도착터미널코드		
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD			",		PST_FIELD(pRPacket, bus_cls_cd				));	///< 버스등급코드			
		LOG_WRITE("%30s - (%s) ", "BUS_CACM_CD			",		PST_FIELD(pRPacket, bus_cacm_cd				));	///< 버스운수사코드		
		LOG_WRITE("%30s - (%s) ", "PUB_WND_NO			",		PST_FIELD(pRPacket, pub_wnd_no				));	///< 발행창구번호			
		LOG_WRITE("%30s - (%s) ", "PUB_SNO				",		PST_FIELD(pRPacket, pub_sno					));	///< 발행일련번호			
		LOG_WRITE("%30s - (%s) ", "PUB_SHCT_TRML_CD		",		PST_FIELD(pRPacket, pub_shct_trml_cd		));	///< 발행단축터미널코드		
		LOG_WRITE("%30s - (%s) ", "BUS_TCK_KND_CD		",		PST_FIELD(pRPacket, bus_tck_knd_cd			));	///< 버스티켓종류코드		
		LOG_WRITE("%30s - (%s) ", "TISU_AMT				",		PST_FIELD(pRPacket, tisu_amt				));	///< 발권금액			
		LOG_WRITE("%30s - (%s) ", "PYN_MNS_DVS_CD		",		PST_FIELD(pRPacket, pyn_mns_dvs_cd			));	///< 지불수단구분코드		
		LOG_WRITE("%30s - (%s) ", "SATS_NO				",		PST_FIELD(pRPacket, sats_no					));	///< 좌석번호			
		LOG_WRITE("%30s - (%s) ", "ROT_KND_CD			",		PST_FIELD(pRPacket, rot_knd_cd				));	///< 노선종류코드			
		LOG_WRITE("%30s - (%s) ", "CTY_BUS_DC_KND_CD	",		PST_FIELD(pRPacket, cty_bus_dc_knd_cd		));	///< 시외버스할인종류코드	
		LOG_WRITE("%30s - (%s) ", "DC_BEF_AMT			",		PST_FIELD(pRPacket, dc_bef_amt				));	///< 할인이전금액			
		LOG_WRITE("%30s - (%s) ", "BUS_TCK_INHR_NO		",		PST_FIELD(pRPacket, bus_tck_inhr_no			));	///< 버스티켓고유번호		
		LOG_WRITE("%30s - (%s) ", "CTY_BUS_OPRN_SHP_CD	",		PST_FIELD(pRPacket, cty_bus_oprn_shp_cd		));	///< 시외버스운행형태코드	
		LOG_WRITE("%30s - (%s) ", "ATL_DEPR_DT			",		PST_FIELD(pRPacket, atl_depr_dt				));	///< 실제출발일자			
		LOG_WRITE("%30s - (%s) ", "ATL_DEPR_TIME		",		PST_FIELD(pRPacket, atl_depr_time			));	///< 실제출발시각			
		LOG_WRITE("%30s - (%s) ", "STPT_TRML_CD			",		PST_FIELD(pRPacket, stpt_trml_cd			));	///< 기점터미널코드		
		LOG_WRITE("%30s - (%s) ", "EPT_TRML_CD			",		PST_FIELD(pRPacket, ept_trml_cd				));	///< 종점터미널코드		
		LOG_WRITE("%30s - (%s) ", "ALCN_WAY_DVS_CD		",		PST_FIELD(pRPacket, alcn_way_dvs_cd			));	///< 배차방식구분코드		
		LOG_WRITE("%30s - (%s) ", "SATI_USE_YN			",		PST_FIELD(pRPacket, sati_use_yn				));	///< 좌석제사용여부		
		LOG_WRITE("%30s - (%s) ", "PERD_TEMP_YN			",		PST_FIELD(pRPacket, perd_temp_yn			));	///< 정기임시여부			
		LOG_WRITE("%30s - (%s) ", "RDHM_MLTP_VAL		",		PST_FIELD(pRPacket, rdhm_mltp_val			));	///< 승차홈다중값			
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_PRIN_YN		",		PST_FIELD(pRPacket, bus_cls_prin_yn			));	///< 버스등급출력여부		
		LOG_WRITE("%30s - (%s) ", "BUS_CACM_NM_PRIN_YN	",		PST_FIELD(pRPacket, bus_cacm_nm_prin_yn		));	///< 버스운수사명출력여부	
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME_PRIN_YN	",		PST_FIELD(pRPacket, depr_time_prin_yn		));	///< 출발시각출력여부		
		LOG_WRITE("%30s - (%s) ", "SATS_NO_PRIN_YN		",		PST_FIELD(pRPacket, sats_no_prin_yn			));	///< 좌석번호출력여부		
		LOG_WRITE("%30s - (%s) ", "DIST					",		PST_FIELD(pRPacket, dist					));	///< 거리				
		LOG_WRITE("%30s - (%s) ", "TAKE_DRTM			",		PST_FIELD(pRPacket, take_drtm				));	///< 소요시간			
		LOG_WRITE("%30s - (%s) ", "PUB_NUM				",		PST_FIELD(pRPacket, pub_num					));	///< 발행매수			
		LOG_WRITE("%30s - (%s) ", "DCRT_DVS_CD			",		PST_FIELD(pRPacket, dcrt_dvs_cd				));	///< 할인율구분코드		
	}

	nRet = 1;

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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc269
 * @details		(269) 발행자 정보 입력 (인천공항 코로나로 인해 한시적으로 open)
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc269(char *pData, char *retBuf)
{
	int		i, nRet, nTimeout, n_rsp_cd, nCount;
	char*	pService = "TK_PubUsrInfInp";
	pstk_pubusrinfinp_t		pSPacket;
	prtk_pubusrinfinp_t		pRPacket;

	i = nRet = nTimeout = n_rsp_cd = nCount = 0;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pstk_pubusrinfinp_t) pData;
	pRPacket = (prtk_pubusrinfinp_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) "		, "DAMO_ENC_PUB_USER_TEL_NO"	,	PST_FIELD(pSPacket, damo_enc_pub_user_tel_no	));
		LOG_WRITE("%30s - (num = %d) "	, "DAMO_ENC_PUB_USER_TEL_NO_LEN",	*(int *)pSPacket->damo_enc_pub_user_tel_no_len	);
		LOG_WRITE("%30s - (%s) "		, "DAMO_ENC_PUB_USER_KRN"		,	PST_FIELD(pSPacket, damo_enc_pub_user_krn		));
		LOG_WRITE("%30s - (num = %d) "	, "DAMO_ENC_PUB_USER_KRN_LEN"	,	*(int *)pSPacket->damo_enc_pub_user_krn_len		);
		LOG_WRITE("%30s - (%s) "		, "DAMO_ENC_DTA_KEY"			,	PST_FIELD(pSPacket, damo_enc_dta_key			));
		LOG_WRITE("%30s - (%s) "		, "RIDE_VHCL_DVS"				,	PST_FIELD(pSPacket, ride_vhcl_dvs				));
		
		LOG_WRITE("%30s - (number = %d) ","PUB_NUM",	*(int *)pSPacket->pub_num			);
		nCount = *(int *)pSPacket->pub_num;
		for(i = 0; i < nCount; i++)
		{
			pstk_pubusrinfinp_list_t pList;

			pList = &pSPacket->List[i];

			LOG_WRITE("%30s - (%s) ", "PUB_DT"						,	PST_FIELD(pList, pub_dt						));
			LOG_WRITE("%30s - (%s) ", "PUB_SHCT_TRML_CD"			,	PST_FIELD(pList, pub_shct_trml_cd			));
			LOG_WRITE("%30s - (%s) ", "PUB_WND_NO"					,	PST_FIELD(pList, pub_wnd_no					));
			LOG_WRITE("%30s - (num = %d) ", "PUB_SNO"				,	*(int *)pList->pub_sno					 	);
		}
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(DAMO_ENC_PUB_USER_TEL_NO	 	,	PST_FIELD(pSPacket,	damo_enc_pub_user_tel_no	));	///< 발행자 전화번호 암호문 
		nRet = TMaxFBPut(DAMO_ENC_PUB_USER_TEL_NO_LEN	,	PST_FIELD(pSPacket,	damo_enc_pub_user_tel_no_len));	///< 발행자 전화번호 암호문 길이
		nRet = TMaxFBPut(DAMO_ENC_PUB_USER_KRN		 	,	PST_FIELD(pSPacket,	damo_enc_pub_user_krn		));	///< 발행자 주민번호 암호문
		nRet = TMaxFBPut(DAMO_ENC_PUB_USER_KRN_LEN	 	,	PST_FIELD(pSPacket,	damo_enc_pub_user_krn_len	));	///< 발행자 주민번호 암호문 길이
		nRet = TMaxFBPut(DAMO_ENC_DTA_KEY			 	,	PST_FIELD(pSPacket,	damo_enc_dta_key			));	///< 암호문키
		nRet = TMaxFBPut(RIDE_VHCL_DVS			 		,	PST_FIELD(pSPacket,	ride_vhcl_dvs				));	///< 승차 차량구분
		nRet = TMaxFBPut(PUB_NUM					 	,	PST_FIELD(pSPacket,	pub_num					 	));	///< 발행매수
		nCount = *(int *)pSPacket->pub_num;
		for(i = 0; i < nCount; i++)
		{
			pstk_pubusrinfinp_list_t pList;

			pList = &pSPacket->List[i];
		
			nRet = TMaxFBPut(PUB_DT						 	,	PST_FIELD(pList,	pub_dt						));	///< 발행일자
			nRet = TMaxFBPut(PUB_SHCT_TRML_CD			 	,	PST_FIELD(pList,	pub_shct_trml_cd			));	///< 발행단축터미널코드
			nRet = TMaxFBPut(PUB_WND_NO					 	,	PST_FIELD(pList,	pub_wnd_no					));	///< 발행창구번호
			nRet = TMaxFBPut(PUB_SNO					 	,	PST_FIELD(pList,	pub_sno					 	));	///< 발행일련번호
		}
	}

	// 동기화 (Send/Recv)
	nTimeout = 45;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -4;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	LOG_WRITE("TMaxTpCall(), Success !!!!");

	///> GetData
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		//nRet = TMaxFBGetF(RSP_MSG, PST_FIELD(pRPacket, rsp_msg));
		//LOG_WRITE("%30s - (%s) ","RSP_MSG",	PST_FIELD(pRPacket, rsp_msg));

		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 146 )
		{
			nRet = -5;
			LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
	}

	nRet = 1;

	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;

fail_proc:

	TMaxFree();
	TMaxDisconnect();
	LOG_CLOSE();

	return nRet;
}

/**
 * @brief		TMaxSvc501
 * @details		(501) 암호화키 전송
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc501(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout;
	char*	pService = "SV_TrmEncKey";
	pssv_trmenckey_t		pSPacket;
	prsv_trmenckey_t		pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pssv_trmenckey_t) pData;
	pRPacket = (prsv_trmenckey_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);
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

	///> recv data
	{
		int n_rsp_cd;

		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 583 )
		{
			nRet = -4;
			LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
		
		nRet = TMaxFBGetF(DAMO_ENC_DTA_KEY		,	pRPacket->damo_enc_dta_key);	///< 암호문키
		nRet = TMaxFBGetF(DAMO_ENC_DTA_KEY_LEN	,	PST_FIELD(pRPacket, damo_enc_dta_key_len));	///< 암호문키길이

 		::CopyMemory(CConfigTkMem::GetInstance()->damo_enc_dta_key, pRPacket->damo_enc_dta_key, sizeof(pRPacket->damo_enc_dta_key));
 		::CopyMemory(CConfigTkMem::GetInstance()->damo_enc_dta_key_len, pRPacket->damo_enc_dta_key_len, sizeof(pRPacket->damo_enc_dta_key_len));
	}

	///> recv log
	{
 		LOG_WRITE("%30s - (%s) ", "DAMO_ENC_DTA_KEY",		PST_FIELD(pRPacket, damo_enc_dta_key	));	///< 암호문키
 		LOG_WRITE("%25s - (number = %d) ", "DAMO_ENC_DTA_KEY_LEN", *(WORD *)pRPacket->damo_enc_dta_key_len);	///< 암호문키길이
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
	return 0;
#endif
}

/**
 * @brief		TMaxSvc502
 * @details		(502) 복호화 서비스
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCBusServer::TMaxSvc502(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int nRet, nTimeout;
	int n_rsp_cd;
	char*	pService = "SV_TrmDecrDta";
	pssv_trmdecrdta_t		pSPacket;
	prsv_trmdecrdta_t		pRPacket;

	LOG_OPEN();
	LOG_WRITE(" (%s) start !!!!", pService);

	pSPacket = (pssv_trmdecrdta_t) pData;
	pRPacket = (prsv_trmdecrdta_t) retBuf;

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
		LogOut_HdrId((char *)&pSPacket->hdr_id);

		LOG_WRITE("%30s - (%s) ", "DAMO_ENC_DTA",			PST_FIELD(pSPacket, damo_enc_dta));
		LOG_WRITE("%30s - (%s) ", "DAMO_ENC_DTA_KEY",		PST_FIELD(pSPacket, damo_enc_dta_key));
		LOG_WRITE("%30s - (number = %d) ", "DAMO_ENC_DTA_KEY_LEN",	*(int *)pSPacket->damo_enc_dta_key_len);
	}

	// send data
	{
		SendIDHeader((char *)&pSPacket->hdr_id);

		nRet = TMaxFBPut(DAMO_ENC_DTA			,	PST_FIELD(pSPacket,	damo_enc_dta			));	///< 암호화데이타
		nRet = TMaxFBPut(DAMO_ENC_DTA_KEY		,	PST_FIELD(pSPacket,	damo_enc_dta_key		));	///< 암호문키
		nRet = TMaxFBPut(DAMO_ENC_DTA_KEY_LEN	,	PST_FIELD(pSPacket,	damo_enc_dta_key_len	));	///< 암호문키길이
	}

	// 동기화 (Send/Recv)
	nTimeout = 3;
	nRet = TMaxTpCall(pService, nTimeout);
	if(nRet < 0)
	{
		nRet = -3;
		LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
		//fbprint(m_pRecvBuff);
		goto fail_proc;
	}

	///> recv data
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		nRet = TMaxFBGetF(RSP_CD, PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ","RSP_CD",	PST_FIELD(pRPacket, rsp_cd));
		LOG_WRITE("%30s - (%s) ", "RSP_CD", PST_FIELD(pRPacket, rsp_cd));
		n_rsp_cd = Util_Ascii2Long(&pRPacket->rsp_cd[3], 3);
		if( n_rsp_cd != 585 )
		{
			nRet = -99;
			LOG_WRITE("TMaxTpCall(), err(%d : %s) !!!!", tperrno, tpstrerror(tperrno));
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->rsp_cd);
			goto fail_proc;
		}
		nRet = TMaxFBGetF(DECR_DTA		,	PST_FIELD(pRPacket, decr_dta		));	///< 복호화데이타
		nRet = TMaxFBGetF(DECR_DTA_LEN	,	PST_FIELD(pRPacket, decr_dta_len	));	///< 복호화데이타길이
	}

	///> recv log
	{
		LOG_WRITE("%30s - (%s) ", "DECR_DTA",		PST_FIELD(pRPacket, decr_dta	));	///< 복호화데이타
		LOG_WRITE("%30s - (%s) ", "DECR_DTA_LEN", PST_FIELD(pRPacket, decr_dta_len));	///< 복호화데이타길이
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
	return 0;
#endif
}

/**
 * @brief		StartProcess
 * @details		Start
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CCBusServer::StartProcess(void)
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
int CCBusServer::EndProcess(void)
{
	LOG_OUT(" ###################### end ###################### \n\n");

	return 0;
}



