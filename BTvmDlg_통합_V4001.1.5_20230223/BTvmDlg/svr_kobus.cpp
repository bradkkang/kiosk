// 
// 
// svr_kobus.cpp : 고속버스(KoBus) 서버
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

//#include "xtz_kobus_0925_fdl.h"		/// 0405	// 20210616 DEL
#include "xtz_kobus_fdl.h"	/// 0405	
							// KSCC-TSK5-SVC-DE-06_인터페이스설계서_무인발권기-V1.86-20210513 // 20210616 MOD
							// KSCC-TSK5-SVC-DE-06_인터페이스설계서_무인발권기-V1.90-20211019 // 20211019 ADD
#include "svr_kobus.h"
#include "svr_kobus_st.h"
#include "oper_config.h"
#include "oper_kos_file.h"
#include "dev_tr_main.h"
#include "dev_tr_mem.h"
#include "dev_tr_kobus_mem.h"
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

#define LOG_OUT(fmt, ...)		{ CKoBusSvr::m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ CKoBusSvr::m_clsLog.HexaDump(x, y, z); }

//#define LOG_WRITE(fmt, ...)		
//#define LOG_HEXA(x,y,z)			

#define LOG_OPEN()				{ CKoBusSvr::m_clsLog.LogOpen(FALSE);  }
#define LOG_WRITE(fmt, ...)		{ CKoBusSvr::m_clsLog.LogWrite("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_CLOSE()				{ CKoBusSvr::m_clsLog.LogClose();  }

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CKoBusSvr
 * @details		생성자
 */
CKoBusSvr::CKoBusSvr()
{
	m_pSendBuff = NULL;
	m_pRecvBuff = NULL;

	m_bVerify = FALSE;

	m_bLog = TRUE;
}

/**
 * @brief		~CKoBusSvr
 * @details		소멸자
 */
CKoBusSvr::~CKoBusSvr()
{
	EndProcess();
}

/**
 * @brief		LOG_INIT
 * @details		LOG 파일 초기화
 * @param		None
 * @return		None
 */
void CKoBusSvr::LOG_INIT(void)
{
	m_clsLog.SetData(30, "\\Log\\TMAX\\kobus");
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		Connect
 * @details		서버 접속
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::TMaxConnect(void)
{
#if (__USE_TMAX__ > 0)
	int		nRet;
	char	szCurrPath[256];
	char	szFullName[256];
	TPSTART_T* tpInfo = NULL;

	::ZeroMemory(szCurrPath, sizeof(szCurrPath));
	::ZeroMemory(szFullName, sizeof(szFullName));

	Util_GetModulePath(szCurrPath);
	sprintf(szFullName, "%s\\tmax.env", szCurrPath);

	if( GetEnv_IsRealMode() == 0 )
	{	// test mode
		LOG_WRITE("TMAX_KOBUS_TEST mode !!!!");
		nRet = tmaxreadenv(szFullName, "TMAX_KOBUS_TEST" );
	}
	else
	{	// real mode
		LOG_WRITE("TMAX_KOBUS_REAL mode !!!!");
		nRet = tmaxreadenv(szFullName, "TMAX_KOBUS_REAL" );
	}

	if(nRet == -1)
	{
		LOG_WRITE("tmaxreadenv() failure !!!!");
		goto tmax_fail_proc;
	}

//	tpInfo = (TPSTART_T *) tpalloc("TPSTART", NULL, 1024 * 200);
	tpInfo = (TPSTART_T *) tpalloc("TPSTART", NULL, 10240);
	if(tpInfo == NULL)
	{
		LOG_WRITE("tpalloc(TPSTART) failure !!!!");
		goto tmax_fail_proc;
	}

	sprintf(tpInfo->cltname, "");
	sprintf(tpInfo->usrname, "");
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
int CKoBusSvr::TMaxFBPut(DWORD dwFieldKey, char *pData)
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
int CKoBusSvr::TMaxFBPutLen(DWORD dwFieldKey, char *pData, int nDataLen)
{
	int nRet = 0, nLen = 0, nPos = 0;

#if (__USE_TMAX__ > 0)
	nRet = fbput(m_pSendBuff, (FLDKEY) dwFieldKey, pData, nDataLen);

	if(m_bVerify == TRUE)
	{
//		int nLen = 0, nPos = 0;

		::ZeroMemory(m_szBuffer, sizeof(m_szBuffer));
		nRet = fbgetf(m_pSendBuff, (FLDKEY) dwFieldKey, m_szBuffer, &nLen, &nPos);
		//LOG_OUT("Verify#1 dwFieldKey %lu, (%s):(%s) ", dwFieldKey, pData, m_szBuffer);
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
int CKoBusSvr::TMaxFBInsert(DWORD dwFieldKey, char *pData, int nIndex, int nDataLen)
{
	int nRet;

	nRet = 0;

#if (__USE_TMAX__ > 0)
	nRet = fbinsert(m_pSendBuff, (FLDKEY) dwFieldKey, nIndex, pData, nDataLen);

	if(m_bVerify == TRUE)
	{
		int nLen = 0, nPos = 0;

		::ZeroMemory(m_szBuffer, sizeof(m_szBuffer));
		nRet = fbget_tu(m_pSendBuff, (FLDKEY) dwFieldKey, nIndex, m_szBuffer, &nLen);
		//LOG_OUT("Verify#2 dwFieldKey %lu, (%s):(%s) ", dwFieldKey, pData, m_szBuffer);
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
int CKoBusSvr::TMaxFBUpdate(DWORD dwFieldKey, char *pData, int nIndex, int nDataLen)
{
	int nRet;

	nRet = 0;

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
int CKoBusSvr::TMaxTpCall(char *pService, int nTimeout)
{
	int nRet = 0;
	int nRecvLen = 0;

#if (__USE_TMAX__ > 0)
	// 블로킹 타임아웃 시간 변경 (초단위 설정) : 거래는 50초
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
int CKoBusSvr::TMaxFBGet(DWORD dwFieldKey, char *pData)
{
	int nRet = 0;

//	LOG_OUT(" start !!!!");

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
int CKoBusSvr::TMaxFBGetTu(DWORD dwFieldKey, char *pData, int nth)
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
int CKoBusSvr::TMaxFBGetF(DWORD dwFieldKey, char *pData)
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
int CKoBusSvr::TMaxAlloc(void)
{
#if (__USE_TMAX__ > 0)
//	m_pSendBuff = (FBUF *) tpalloc("FIELD", NULL, (1024 * 200));
	m_pSendBuff = (FBUF *) tpalloc("FIELD", NULL, (1024 * 30));
	if(m_pSendBuff == NULL)
	{
		return -1;
	}

	// 수신 버퍼 할당 함수
//	m_pRecvBuff = (FBUF *) tpalloc("FIELD", NULL, (1024 * 200));
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
void CKoBusSvr::TMaxFree(void)
{
	LOG_WRITE(" start !!!!");

#if (__USE_TMAX__ > 0)

	if(m_pSendBuff != NULL)
	{
		// 2021.02.24 add code
		fbdelall(m_pSendBuff, CARD_NO);
		fbdelall(m_pSendBuff, ICCD_INF);

		tpfree((char *)m_pSendBuff);
		m_pSendBuff = NULL;
	}

	if(m_pRecvBuff != NULL)
	{
		fbdelall(m_pRecvBuff, CARD_NO);
		fbdelall(m_pRecvBuff, ICCD_INF);

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
void CKoBusSvr::TMaxDisconnect(void)
{
	LOG_WRITE(" start !!!!");

#if (__USE_TMAX__ > 0)
 	tpend();
#endif
}

/**
 * @brief		TMaxGetConvChar
 * @details		TMAX 데이타 가져와서, UTF8을 ANSI로 변환
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
void CKoBusSvr::TMaxGetConvChar(DWORD dwFieledKey, char *retBuf)
{
	int nRet = 0;

	::ZeroMemory(m_szBuffer, sizeof(m_szBuffer));

	nRet = TMaxFBGetF(dwFieledKey,	m_szBuffer);
	Util_Utf8ToAnsi(m_szBuffer, retBuf);
}

/**
 * @brief		StartProcess
 * @details		Start
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CKoBusSvr::StartProcess(void)
{
	LOG_INIT();

	LOG_OUT("%s", "###################### start ###################### \n\n ");

	return 0;
}

/**
 * @brief		EndProcess
 * @details		Start
 * @param		int nCommIdx		COM
 * @return		성공 : > 0, 실패 : < 0
 */
int CKoBusSvr::EndProcess(void)
{
	LOG_OUT("%s", " ###################### end ###################### \n\n");

	return 0;
}

/**
 * @brief		SVC_CM_ReadTrmlInf
 * @details		(TM_ETRMLINFO) 터미널 정보 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_CM_ReadTrmlInf(int nOperID, int nIndex, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	char*	pService = "TM_ETRMLINFO";
	pstk_tm_etrmlinfo_t pSPacket;
	prtk_tm_etrmlinfo_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" [%d] 터미널 정보 조회_start !!!!", nOperID);

	pSPacket = (pstk_tm_etrmlinfo_t) pData;
	pRPacket = (prtk_tm_etrmlinfo_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ","LNG_CD",			pSPacket->lng_cd);
		LOG_WRITE("%30s - (%s) ","USER_NO",			pSPacket->user_no);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO",	pSPacket->depr_trml_no);
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD,		pSPacket->lng_cd);
		nRet = TMaxFBPut(USER_NO,		pSPacket->user_no);
		nRet = TMaxFBPut(DEPR_TRML_NO,	pSPacket->depr_trml_no);
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
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_CD"		, PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT"	, PST_FIELD(pRPacket, msg_dtl_ctt));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}
		/// msg_contents
		nRet = TMaxFBGet(MSG_DTL_CTT, PST_FIELD(pRPacket, msg_dtl_ctt));

		/// list count
		nRet = TMaxFBGet(REC_NCNT1, PST_FIELD(pRPacket, rec_ncnt1));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NCNT1", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_ncnt1", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_tm_etrmlinfo_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_etrmlinfo_list_t) * nCount);
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

		MyWriteFile(hFile, pRPacket->msg_cd, sizeof(pRPacket->msg_cd));
		MyWriteFile(hFile, pRPacket->msg_dtl_ctt, sizeof(pRPacket->msg_dtl_ctt));

		::ZeroMemory(pRPacket->rec_ncnt1, sizeof(pRPacket->rec_ncnt1));
		::CopyMemory(pRPacket->rec_ncnt1, &m_nRecNcnt1, 4);
		MyWriteFile(hFile, pRPacket->rec_ncnt1, sizeof(pRPacket->rec_ncnt1));

		MySetFilePointer(hFile, 0, FILE_END);

		for(i = 0; i < nCount; i++)
		{
			prtk_tm_etrmlinfo_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(TRML_NO			,	PST_FIELD(pList, trml_no			));
			
			switch(nIndex)
			{
			case LANG_DVS_KO:
			case LANG_DVS_ENG:
				TMaxGetConvChar(TRML_NM				,	pList->trml_nm						);
				TMaxGetConvChar(TRML_ABRV_NM		,	pList->trml_abrv_nm					);
				TMaxGetConvChar(TRML_DTL_ADDR		,	pList->trml_dtl_addr				);
				TMaxGetConvChar(TRML_ENG_NM			,	pList->trml_eng_nm					);
				TMaxGetConvChar(TRML_ENG_ABRV_NM	,	pList->trml_eng_abrv_nm				);
				break;
			case LANG_DVS_CHI:
			case LANG_DVS_JPN:
				nRet = TMaxFBGetF(TRML_NM			,	PST_FIELD(pList, trml_nm			));
				nRet = TMaxFBGetF(TRML_ABRV_NM		,	PST_FIELD(pList, trml_abrv_nm		));
				nRet = TMaxFBGetF(TRML_DTL_ADDR		,	PST_FIELD(pList, trml_dtl_addr		));
				nRet = TMaxFBGetF(TRML_ENG_NM		,	PST_FIELD(pList, trml_eng_nm		));
				nRet = TMaxFBGetF(TRML_ENG_ABRV_NM	,	PST_FIELD(pList, trml_eng_abrv_nm	));
				break;
			}

			nRet = TMaxFBGetF(TRTR_TRML_YN		,	PST_FIELD(pList, trtr_trml_yn		));
			nRet = TMaxFBGetF(SYS_DVS_CD		,	PST_FIELD(pList, sys_dvs_cd			));
			nRet = TMaxFBGetF(TEL_NO			,	PST_FIELD(pList, tel_no				));

			// 언어구분
			MyWriteFile(hFile, pSPacket->lng_cd, sizeof(pSPacket->lng_cd));
			// 구조체
			MyWriteFile(hFile, pList, sizeof(rtk_tm_etrmlinfo_list_t));

			if( memcmp(GetTrmlCode(SVR_DVS_KOBUS), pList->trml_no, strlen(pList->trml_no)) == 0 )
			{
// 				if( memcmp(pSPacket->lng_cd, "KO", 2) == 0 )
// 				{
// 					sprintf(CConfigTkMem::GetInstance()->depr_trml_eng_nm, "%s", pList->trml_eng_nm);
// 				}
			}

		}
		MyCloseFile(hFile);
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_etrmlinfo_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TRML_NO"			,	PST_FIELD(pList, trml_no			));
			LOG_WRITE("%30s - (%s) ", "TRML_NM"			,	PST_FIELD(pList, trml_nm			));
			LOG_WRITE("%30s - (%s) ", "TRML_ABRV_NM"	,	PST_FIELD(pList, trml_abrv_nm		));
			LOG_WRITE("%30s - (%s) ", "TRML_DTL_ADDR"	,	PST_FIELD(pList, trml_dtl_addr		));
			LOG_WRITE("%30s - (%s) ", "TRML_ENG_NM"		,	PST_FIELD(pList, trml_eng_nm		));
			LOG_WRITE("%30s - (%s) ", "TRML_ENG_ABRV_NM",	PST_FIELD(pList, trml_eng_abrv_nm	));
			LOG_WRITE("%30s - (%s) ", "TRTR_TRML_YN"	,	PST_FIELD(pList, trtr_trml_yn		));
			LOG_WRITE("%30s - (%s) ", "SYS_DVS_CD"		,	PST_FIELD(pList, sys_dvs_cd			));
			LOG_WRITE("%30s - (%s) ", "TEL_NO"			,	PST_FIELD(pList, tel_no				));
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
 * @brief		SVC_CM_ReadCmnCd
 * @details		(CM_CDINQR) 공통코드 상세 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_CM_ReadCmnCd(int nOperID, int nIndex, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	char*	pService = "CM_CDINQR";
	pstk_cm_cdinqr_t pSPacket;
	prtk_cm_cdinqr_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" [%d] 공통코드 상세 조회_start !!!!", nOperID);

	pSPacket = (pstk_cm_cdinqr_t) pData;
	pRPacket = (prtk_cm_cdinqr_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ","LNG_CD",		pSPacket->lng_cd);
		LOG_WRITE("%30s - (%s) ","USER_NO",		pSPacket->user_no);
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD,		pSPacket->lng_cd);
		nRet = TMaxFBPut(USER_NO,		pSPacket->user_no);
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
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_CD"		, PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT"	, PST_FIELD(pRPacket, msg_dtl_ctt));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}
		/// msg_contents
		nRet = TMaxFBGet(MSG_DTL_CTT, PST_FIELD(pRPacket, msg_dtl_ctt));

		/// list count
		nRet = TMaxFBGet(REC_NCNT1, PST_FIELD(pRPacket, rec_ncnt1));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NCNT1", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_ncnt1", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_cm_cdinqr_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_cm_cdinqr_list_t) * nCount);
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

		MyWriteFile(hFile, pRPacket->msg_cd, sizeof(pRPacket->msg_cd));
		MyWriteFile(hFile, pRPacket->msg_dtl_ctt, sizeof(pRPacket->msg_dtl_ctt));
		
		::ZeroMemory(pRPacket->rec_ncnt1, sizeof(pRPacket->rec_ncnt1));
		::CopyMemory(pRPacket->rec_ncnt1, &m_nRecNcnt1, 4);
		MyWriteFile(hFile, pRPacket->rec_ncnt1, sizeof(pRPacket->rec_ncnt1));

		MySetFilePointer(hFile, 0, FILE_END);
		
		for(i = 0; i < nCount; i++)
		{
			prtk_cm_cdinqr_list_t pList;

			::ZeroMemory(m_szBuffer, sizeof(m_szBuffer));

			pList = &pRPacket->pList[i];

			sprintf(pList->lng_cd, "%s", pSPacket->lng_cd);
			//nRet = TMaxFBGetF(LNG_CD			,		PST_FIELD(pList, lng_cd			));
			nRet = TMaxFBGetF(CMN_CD			,		PST_FIELD(pList, cmn_cd			));
			nRet = TMaxFBGetF(CMN_CD_VAL		,		PST_FIELD(pList, cmn_cd_val		));
			nRet = TMaxFBGetF(CD_VAL_MARK_SEQ	,		PST_FIELD(pList, cd_val_mark_seq));
			TMaxGetConvChar(CD_VAL_NM			,		pList->cd_val_nm				);
			TMaxGetConvChar(BSC_RFRN_VAL		,		pList->bsc_rfrn_val				);
			//nRet = TMaxFBGetF(BSC_RFRN_VAL	,		PST_FIELD(pList, bsc_rfrn_val	));
			TMaxGetConvChar(ADTN_RFRN_VAL		,		pList->adtn_rfrn_val			);
			//nRet = TMaxFBGetF(ADTN_RFRN_VAL		,	PST_FIELD(pList, adtn_rfrn_val	));

			if( memcmp("C025", pList->cmn_cd, 4) == 0 )
			{
				LOG_WRITE("LNG(%s),CD(%s),VAL(%s),SEQ(%d),NM(%s)", pList->lng_cd, pList->cmn_cd, pList->cmn_cd_val, *(int *)pList->cd_val_mark_seq, pList->cd_val_nm);
			}
			
			if( (memcmp("C025", pList->cmn_cd, 4) == 0) && (memcmp("KO", pList->lng_cd, 2) == 0) )
			{
				m_vtKorTck.push_back(*pList);
			}
			else if( (memcmp("C025", pList->cmn_cd, 4) == 0) && (memcmp("EN", pList->lng_cd, 2) == 0) )
			{
				m_vtEngTck.push_back(*pList);
			}
			else if( (memcmp("C025", pList->cmn_cd, 4) == 0) && (memcmp("CN", pList->lng_cd, 2) == 0) )
			{
				m_vtChiTck.push_back(*pList);
			}
			if( (memcmp("C025", pList->cmn_cd, 4) == 0) && (memcmp("JP", pList->lng_cd, 2) == 0) )
			{
				m_vtJpnTck.push_back(*pList);
			}

			MyWriteFile(hFile, pList, sizeof(rtk_cm_cdinqr_list_t));
		}
		MyCloseFile(hFile);
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		for(i = 0; i < nCount; i++)
		{
			prtk_cm_cdinqr_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "LNG_CD"			,	PST_FIELD(pList, lng_cd			));
			LOG_WRITE("%30s - (%s) ", "CMN_CD"			,	PST_FIELD(pList, cmn_cd			));
			LOG_WRITE("%30s - (%s) ", "CMN_CD_VAL"		,	PST_FIELD(pList, cmn_cd_val		));
//			LOG_WRITE("%30s - (%s) ", "CD_VAL_MARK_SEQ"	,	PST_FIELD(pList, cd_val_mark_seq));
			LOG_WRITE("%30s - (%d) ", "CD_VAL_MARK_SEQ"	,	*(int *)pList->cd_val_mark_seq	);
			LOG_WRITE("%30s - (%s) ", "CD_VAL_NM"		,	PST_FIELD(pList, cd_val_nm		));
			LOG_WRITE("%30s - (%s) ", "BSC_RFRN_VAL"	,	PST_FIELD(pList, bsc_rfrn_val	));
			LOG_WRITE("%30s - (%s) ", "ADTN_RFRN_VAL"	,	PST_FIELD(pList, adtn_rfrn_val	));
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
 * @brief		SVC_CM_RdhmInqr
 * @details		(CM_CDINQR) 승차홈 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_CM_RdhmInqr(int nOperID, int nIndex, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	char*	pService = "CM_RDHMINQR";
	pstk_cm_rdhminqr_t pSPacket;
	prtk_cm_rdhminqr_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" [%d] 승차홈 조회_start !!!!", nOperID);

	pSPacket = (pstk_cm_rdhminqr_t) pData;
	pRPacket = (prtk_cm_rdhminqr_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ","LNG_CD",		pSPacket->lng_cd);
		LOG_WRITE("%30s - (%s) ","USER_NO",		pSPacket->user_no);
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD,		pSPacket->lng_cd);
		nRet = TMaxFBPut(USER_NO,		pSPacket->user_no);
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
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_CD", PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		/// list count
		nRet = TMaxFBGet(REC_NCNT1, PST_FIELD(pRPacket, rec_ncnt1));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NCNT1", nCount);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_ncnt1", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_cm_rdhminqr_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_cm_rdhminqr_list_t) * nCount);
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

		MyWriteFile(hFile, pRPacket->msg_cd, sizeof(pRPacket->msg_cd));
		MyWriteFile(hFile, pRPacket->msg_dtl_ctt, sizeof(pRPacket->msg_dtl_ctt));

		::ZeroMemory(pRPacket->rec_ncnt1, sizeof(pRPacket->rec_ncnt1));
		::CopyMemory(pRPacket->rec_ncnt1, &m_nRecNcnt1, 4);
		MyWriteFile(hFile, pRPacket->rec_ncnt1, sizeof(pRPacket->rec_ncnt1));

		MySetFilePointer(hFile, 0, FILE_END);
		
		for(i = 0; i < nCount; i++)
		{
			prtk_cm_rdhminqr_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(DEPR_TRML_NO	,		PST_FIELD(pList, depr_trml_no	));
			nRet = TMaxFBGetF(ARVL_TRML_NO	,		PST_FIELD(pList, arvl_trml_no	));
			nRet = TMaxFBGetF(BUS_CLS_CD	,		PST_FIELD(pList, bus_cls_cd		));
			nRet = TMaxFBGetF(CACM_CD		,		PST_FIELD(pList, cacm_cd		));
			nRet = TMaxFBGetF(ROT_RDHM_NO	,		PST_FIELD(pList, rot_rdhm_no	));

			MyWriteFile(hFile, pList, sizeof(rtk_cm_rdhminqr_list_t));
		}
		MyCloseFile(hFile);
	}

	// recv log
	//if(m_bLog == TRUE)
	if(0)
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		for(i = 0; i < nCount; i++)
		{
			prtk_cm_rdhminqr_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"	,	PST_FIELD(pList, depr_trml_no	));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"	,	PST_FIELD(pList, arvl_trml_no	));
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD	"	,	PST_FIELD(pList, bus_cls_cd		));
			LOG_WRITE("%30s - (%s) ", "CACM_CD	"	,	PST_FIELD(pList, cacm_cd		));
			LOG_WRITE("%30s - (%s) ", "ROT_RDHM_NO"	,	PST_FIELD(pList, rot_rdhm_no	));
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
 * @details		(TM_ETCKPTRGINFO) 승차권 인쇄정보 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_TK_ReadTckPrtg(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	char*	pService = "TM_ETCKPTRGINFO";
	pstk_tm_etckptrginfo_t pSPacket;
	prtk_tm_etckptrginfo_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" [%d] 승차권 인쇄정보 조회_start !!!!", nOperID);

	pSPacket = (pstk_tm_etckptrginfo_t) pData;
	pRPacket = (prtk_tm_etckptrginfo_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ","LNG_CD",		pSPacket->lng_cd);
		LOG_WRITE("%30s - (%s) ","USER_NO",		pSPacket->user_no);

		LOG_WRITE("%30s - (%s) ","TRML_NO"				,	pSPacket->trml_no			);
		LOG_WRITE("%30s - (%s) ","PAPR_TCK_DVS_CD"		,	pSPacket->papr_tck_dvs_cd	);
		LOG_WRITE("%30s - (%s) ","PTR_KND_CD"			,	pSPacket->ptr_knd_cd		);
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD,		pSPacket->lng_cd);
		nRet = TMaxFBPut(USER_NO,		pSPacket->user_no);

		nRet = TMaxFBPut(TRML_NO		,	pSPacket->trml_no);
		nRet = TMaxFBPut(PAPR_TCK_DVS_CD,	pSPacket->papr_tck_dvs_cd);
		nRet = TMaxFBPut(PTR_KND_CD		,	pSPacket->ptr_knd_cd);
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
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		/// list count
		nRet = TMaxFBGet(REC_NCNT1, PST_FIELD(pRPacket, rec_ncnt1));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_ncnt1", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_tm_etckptrginfo_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_etckptrginfo_list_t) * nCount);
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

		MyWriteFile(hFile, pRPacket->msg_cd, sizeof(pRPacket->msg_cd));
		MyWriteFile(hFile, pRPacket->msg_dtl_ctt, sizeof(pRPacket->msg_dtl_ctt));
		MyWriteFile(hFile, pRPacket->rec_ncnt1, sizeof(pRPacket->rec_ncnt1));
		
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_etckptrginfo_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(TRML_NO 			,	PST_FIELD(pList, trml_no 		));
			nRet = TMaxFBGetF(PAPR_TCK_DVS_CD	,	PST_FIELD(pList, papr_tck_dvs_cd));
			nRet = TMaxFBGetF(PTR_KND_CD 		,	PST_FIELD(pList, ptr_knd_cd 	));
			nRet = TMaxFBGetF(SORT_SEQ 			,	PST_FIELD(pList, sort_seq 		));
			nRet = TMaxFBGetF(PTRG_USG_CD 		,	PST_FIELD(pList, ptrg_usg_cd 	));
			nRet = TMaxFBGetF(PTRG_ATC_NM 		,	PST_FIELD(pList, ptrg_atc_nm 	));
			nRet = TMaxFBGetF(X_CRDN_VAL 		,	PST_FIELD(pList, x_crdn_val 	));
			nRet = TMaxFBGetF(Y_CRDN_VAL 		,	PST_FIELD(pList, y_crdn_val 	));
			nRet = TMaxFBGetF(MGNF_VAL 			,	PST_FIELD(pList, mgnf_val 		));
			nRet = TMaxFBGetF(USE_YN 			,	PST_FIELD(pList, use_yn 		));

			//if (defTerNo != terNo || defDvsCd != dvsCd || !useYn.IsYN())
			if(0)
			{
				continue;
			}

			MyWriteFile(hFile, pList, sizeof(rtk_tm_etckptrginfo_list_t));
		}
		MyCloseFile(hFile);
	}

	// recv log
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD"		, PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT"	, PST_FIELD(pRPacket, msg_dtl_ctt));
		LOG_WRITE("%30s - (%d) ", "REC_NCNT1"	, nCount);
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_etckptrginfo_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TRML_NO"			,	PST_FIELD(pList, trml_no 		));
			LOG_WRITE("%30s - (%s) ", "PAPR_TCK_DVS_CD"	,	PST_FIELD(pList, papr_tck_dvs_cd));
			LOG_WRITE("%30s - (%s) ", "PTR_KND_CD"		,	PST_FIELD(pList, ptr_knd_cd 	));
			LOG_WRITE("%30s - (%s) ", "SORT_SEQ"		,	PST_FIELD(pList, sort_seq 		));
			LOG_WRITE("%30s - (%s) ", "PTRG_USG_CD"		,	PST_FIELD(pList, ptrg_usg_cd 	));
			LOG_WRITE("%30s - (%s) ", "PTRG_ATC_NM"		,	PST_FIELD(pList, ptrg_atc_nm 	));
			LOG_WRITE("%30s - (%s) ", "X_CRDN_VAL"		,	PST_FIELD(pList, x_crdn_val 	));
			LOG_WRITE("%30s - (%s) ", "Y_CRDN_VAL"		,	PST_FIELD(pList, y_crdn_val 	));
			LOG_WRITE("%30s - (%s) ", "MGNF_VAL"		,	PST_FIELD(pList, mgnf_val 		));
			LOG_WRITE("%30s - (%s) ", "USE_YN"			,	PST_FIELD(pList, use_yn 		));
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
 * @brief		SVC_CM_ReadMsg
 * @details		(CM_MSGINQR) 메세지 정보 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_CM_ReadMsg(int nOperID, int nIndex, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	char*	pService = "CM_MSGINQR";
	pstk_cm_msginqr_t pSPacket;
	prtk_cm_msginqr_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" [%d] 메세지 정보 조회_start !!!!", nOperID);

	pSPacket = (pstk_cm_msginqr_t) pData;
	pRPacket = (prtk_cm_msginqr_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ","LNG_CD",		pSPacket->lng_cd);
		LOG_WRITE("%30s - (%s) ","USER_NO",		pSPacket->user_no);
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD,		pSPacket->lng_cd);
		nRet = TMaxFBPut(USER_NO,		pSPacket->user_no);
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
		char szMsgNm[500+1];
		CString strMsgNm;
		HANDLE hFile;
		CString strFullName;

		/// msg_cd
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		/// list count
		nRet = TMaxFBGet(REC_NCNT1, PST_FIELD(pRPacket, rec_ncnt1));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_ncnt1", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_cm_msginqr_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_cm_msginqr_list_t) * nCount);
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

		MyWriteFile(hFile, pRPacket->msg_cd, sizeof(pRPacket->msg_cd));
		MyWriteFile(hFile, pRPacket->msg_dtl_ctt, sizeof(pRPacket->msg_dtl_ctt));

		::ZeroMemory(pRPacket->rec_ncnt1, sizeof(pRPacket->rec_ncnt1));
		::CopyMemory(pRPacket->rec_ncnt1, &m_nRecNcnt1, 4);
		MyWriteFile(hFile, pRPacket->rec_ncnt1, sizeof(pRPacket->rec_ncnt1));

		MySetFilePointer(hFile, 0, FILE_END);
		
		for(i = 0; i < nCount; i++)
		{
			prtk_cm_msginqr_list_t pList;

			::ZeroMemory(szMsgNm, sizeof(szMsgNm));

			pList = &pRPacket->pList[i];

			sprintf(pList->lng_cd, "%s", pSPacket->lng_cd);
			nRet = TMaxFBGetF(MSG_CD 		,	PST_FIELD(pList, msg_cd 	));
			TMaxGetConvChar(MSG_DTL_CTT		,	pList->msg_dtl_ctt			);
			MyWriteFile(hFile, pList, sizeof(rtk_cm_msginqr_list_t));
		}
		MyCloseFile(hFile);
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD", PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT", PST_FIELD(pRPacket, msg_dtl_ctt));
		LOG_WRITE("%30s - (%d) ", "REC_NCNT1", nCount);
		for(i = 0; i < nCount; i++)
		{
			prtk_cm_msginqr_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "lng_cd 	"	,	PST_FIELD(pList, lng_cd 		));
			LOG_WRITE("%30s - (%s) ", "msg_cd		"	,	PST_FIELD(pList, msg_cd			));
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
 * @brief		SVC_CM_MngCacm
 * @details		(CM_MSGINQR) 운송회사 정보 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_CM_MngCacm(int nOperID, int nIndex, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	char*	pService = "CM_CACMINQR";
	pstk_cm_cacminqr_t pSPacket;
	prtk_cm_cacminqr_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" [%d] 운송회사 정보 조회_start !!!!", nOperID);

	pSPacket = (pstk_cm_cacminqr_t) pData;
	pRPacket = (prtk_cm_cacminqr_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ","LNG_CD",		pSPacket->lng_cd);
		LOG_WRITE("%30s - (%s) ","USER_NO",		pSPacket->user_no);
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD,		pSPacket->lng_cd);
		nRet = TMaxFBPut(USER_NO,		pSPacket->user_no);
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
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		/// list count
		nRet = TMaxFBGet(REC_NCNT1, PST_FIELD(pRPacket, rec_ncnt1));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_ncnt1", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_cm_cacminqr_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_cm_cacminqr_list_t) * nCount);
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

		MyWriteFile(hFile, pRPacket->msg_cd, sizeof(pRPacket->msg_cd));
		MyWriteFile(hFile, pRPacket->msg_dtl_ctt, sizeof(pRPacket->msg_dtl_ctt));

		::ZeroMemory(pRPacket->rec_ncnt1, sizeof(pRPacket->rec_ncnt1));
		::CopyMemory(pRPacket->rec_ncnt1, &m_nRecNcnt1, 4);
		MyWriteFile(hFile, pRPacket->rec_ncnt1, sizeof(pRPacket->rec_ncnt1));

		MySetFilePointer(hFile, 0, FILE_END);
		
		for(i = 0; i < nCount; i++)
		{
			prtk_cm_cacminqr_list_t pList;

			pList = &pRPacket->pList[i];

			sprintf(pList->lng_cd, "%s", pSPacket->lng_cd);
			sprintf(pList->trml_no, "%s", GetTrmlCode(SVR_DVS_CCBUS));
			pList->hspd_cty_dvs_cd[0] = '0';

			nRet = TMaxFBGetF(CACM_CD 		,	PST_FIELD(pList, cacm_cd 		));
			TMaxGetConvChar(CD_VAL_NM		,	pList->cacm_nm					);
			TMaxGetConvChar(CACM_ABRV_NM	,	pList->cacm_abrv_nm				);
			nRet = TMaxFBGetF(BIZR_NO 		,	PST_FIELD(pList, bizr_no 		));

			::CopyMemory(pList->trml_by_cacm_cd	, pList->cacm_cd, sizeof(pList->cacm_cd));
			::CopyMemory(pList->scrn_prin_nm	, pList->cacm_abrv_nm, sizeof(pList->cacm_abrv_nm));
			::CopyMemory(pList->ptrg_prin_nm	, pList->cacm_abrv_nm, sizeof(pList->cacm_abrv_nm));
			pList->mod_sta_cd[0] = 0;

			MyWriteFile(hFile, pList, sizeof(rtk_cm_cacminqr_list_t));
		}
		MyCloseFile(hFile);
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD", PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT", PST_FIELD(pRPacket, msg_dtl_ctt));
		LOG_WRITE("%30s - (%d) ", "REC_NCNT1", nCount);
		for(i = 0; i < nCount; i++)
		{
			prtk_cm_cacminqr_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "LNG_CD"			,	PST_FIELD(pList, lng_cd 		));
			LOG_WRITE("%30s - (%s) ", "TRML_NO"			,	PST_FIELD(pList, trml_no		));
			LOG_WRITE("%30s - (%s) ", "HSPD_CTY_DVS_CD"	,	PST_FIELD(pList, hspd_cty_dvs_cd));

			LOG_WRITE("%30s - (%s) ", "CACM_CD"			,	PST_FIELD(pList, cacm_cd		));
			LOG_WRITE("%30s - (%s) ", "CACM_NM"			,	PST_FIELD(pList, cacm_nm		));
			LOG_WRITE("%30s - (%s) ", "CACM_ABRV_NM"		,	PST_FIELD(pList, cacm_abrv_nm	));
			LOG_WRITE("%30s - (%s) ", "BIZR_NO"			,	PST_FIELD(pList, bizr_no		));

			LOG_WRITE("%30s - (%s) ", "TRML_BY_CACM_CD"	,	PST_FIELD(pList, trml_by_cacm_cd));
			LOG_WRITE("%30s - (%s) ", "SCRN_PRIN_NM"		,	PST_FIELD(pList, scrn_prin_nm	));
			LOG_WRITE("%30s - (%s) ", "PTRG_PRIN_NM"		,	PST_FIELD(pList, ptrg_prin_nm	));
			LOG_WRITE("%30s - (%s) ", "MOD_STA_CD"		,	PST_FIELD(pList, mod_sta_cd		));
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
 * @details		(CM_SVCTIMEINQR) 서버 시간, 승차권 고유번호(x), 공지사항(x)
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_CM_ReadNtc(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout;
	char*	pService = "CM_SVCTIMEINQR";
	pstk_cm_svctimeinqr_t pSPacket;
	prtk_cm_svctimeinqr_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" [%d] 시간동기화_start !!!!", nOperID);

	pSPacket = (pstk_cm_svctimeinqr_t) pData;
	pRPacket = (prtk_cm_svctimeinqr_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ","LNG_CD",		pSPacket->lng_cd);
		LOG_WRITE("%30s - (%s) ","USER_NO",		pSPacket->user_no);
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD,		pSPacket->lng_cd);
		nRet = TMaxFBPut(USER_NO,		pSPacket->user_no);
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
// 		HANDLE hFile;
// 		CString strFullName;

		/// msg_cd
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		/// 서버 일시
		nRet = TMaxFBGet(PRS_TIME, PST_FIELD(pRPacket, prs_time));

		{
			SYSTEMTIME st;

			GetLocalTime(&st);

			st.wYear   = (WORD) Util_Ascii2Long(&pRPacket->prs_time[0], 4);
			st.wMonth  = (WORD) Util_Ascii2Long(&pRPacket->prs_time[4], 2);
			st.wDay    = (WORD) Util_Ascii2Long(&pRPacket->prs_time[6], 2);

			st.wHour   = (WORD) Util_Ascii2Long(&pRPacket->prs_time[8], 2);
			st.wMinute = (WORD) Util_Ascii2Long(&pRPacket->prs_time[10], 2);
			st.wSecond = (WORD) Util_Ascii2Long(&pRPacket->prs_time[12], 2);

			nRet = Util_CheckDateTime(MY_SYS_DATETIME, (BYTE *)&st);
			if(nRet >= 0)
			{
				nRet = Util_SetLocalTime((char *)&st);
				LOG_WRITE("nRet(%d), 시간동기화 : %04d/%02d/%02d %02d:%02d:%02d ", nRet, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			}
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
 * @brief		SVC_CM_ReadRtrpTrml
 * @details		(TM_ERTRPTRMLINF) 왕복 가능 터미널 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_CM_ReadRtrpTrml(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	char*	pService = "TM_ERTRPTRMLINF";
	pstk_tm_ertrptrmlinf_t pSPacket;
	prtk_tm_ertrptrmlinf_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" [%d] 왕복 가능 터미널 조회_start !!!!", nOperID);

	pSPacket = (pstk_tm_ertrptrmlinf_t) pData;
	pRPacket = (prtk_tm_ertrptrmlinf_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ","LNG_CD",		pSPacket->lng_cd);
		LOG_WRITE("%30s - (%s) ","USER_NO",		pSPacket->user_no);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO",	pSPacket->depr_trml_no);
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD,		pSPacket->lng_cd);
		nRet = TMaxFBPut(USER_NO,		pSPacket->user_no);
		nRet = TMaxFBPut(DEPR_TRML_NO,	pSPacket->depr_trml_no);
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
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		/// list count
		nRet = TMaxFBGet(REC_NCNT1, PST_FIELD(pRPacket, rec_ncnt1));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_ncnt1", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_tm_ertrptrmlinf_t_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_ertrptrmlinf_t_list_t) * nCount);
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

		MyWriteFile(hFile, pRPacket->msg_cd, sizeof(pRPacket->msg_cd));
		MyWriteFile(hFile, pRPacket->msg_dtl_ctt, sizeof(pRPacket->msg_dtl_ctt));
		MyWriteFile(hFile, pRPacket->rec_ncnt1, sizeof(pRPacket->rec_ncnt1));
		
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_ertrptrmlinf_t_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(ARVL_TRML_NO		,	PST_FIELD(pList, arvl_trml_no 			));

			MyWriteFile(hFile, pList, sizeof(rtk_tm_ertrptrmlinf_t_list_t));
		}
		MyCloseFile(hFile);
	}

	// recv log
	if(m_bLog == TRUE)
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD", PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT", PST_FIELD(pRPacket, msg_dtl_ctt));
		LOG_WRITE("%30s - (%d) ", "REC_NCNT1", nCount);
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_ertrptrmlinf_t_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO",	PST_FIELD(pList, arvl_trml_no ));
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
 * @brief		AssignTicketKind
 * @details		승차권 정보 데이타 치환
 * @param		char *cd_val		[in] 승차권종류 val
 * @param		char *seq			[in] 승차권종류 seq
 * @param		char *retBuf		[out] 승차권 정보 리스트		
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::AssignTicketKind(char *cd_val, char *seq, char *retBuf)
{
	vector<rtk_cm_cdinqr_list_t>::iterator iter_eng;
	vector<rtk_cm_cdinqr_list_t>::iterator iter_chi;
	vector<rtk_cm_cdinqr_list_t>::iterator iter_jpn;
	prtk_cm_readtckknd_list_t pTckKnd;

	pTckKnd = (prtk_cm_readtckknd_list_t) retBuf;

	//if(nLang == LANG_ENG)
	{
		for(iter_eng = m_vtEngTck.begin(); iter_eng != m_vtEngTck.end(); iter_eng++)
		{
			//if( !memcmp(iter_eng->cmn_cd_val, cd_val, strlen(cd_val)) && 
			//	!memcmp(iter_eng->cd_val_mark_seq, seq, strlen(seq)))
			if( !memcmp(iter_eng->cmn_cd_val, cd_val, strlen(cd_val)) )
			{
				switch(iter_eng->cmn_cd_val[0])
				{
				case '1' :	///> 일반
					sprintf(pTckKnd->tck_nm_en, "Adult");
					break;
				case '2' :	///> 초등생
					sprintf(pTckKnd->tck_nm_en, "Child");
					break;
				case '9' :	///> 중고생
					sprintf(pTckKnd->tck_nm_en, "Student");
					break;
				default:
					::CopyMemory(pTckKnd->tck_nm_en, iter_eng->cd_val_nm, strlen(iter_eng->cd_val_nm));
					break;
				}
				break;
			}
		}

	}

	//if(nLang == LANG_CHINA)
	{
		for(iter_chi = m_vtChiTck.begin(); iter_chi != m_vtChiTck.end(); iter_chi++)
		{
			if( !memcmp(iter_chi->cmn_cd_val, cd_val, strlen(cd_val)) && 
				!memcmp(iter_chi->cd_val_mark_seq, seq, strlen(seq)))
			{
				::CopyMemory(pTckKnd->tck_nm_ch, iter_chi->cd_val_nm, strlen(iter_chi->cd_val_nm));
				break;
			}
		}
	}

	//if(nLang == LANG_JPN)
	{
		for(iter_jpn = m_vtJpnTck.begin(); iter_jpn != m_vtJpnTck.end(); iter_jpn++)
		{
			if( !memcmp(iter_jpn->cmn_cd_val, cd_val, strlen(cd_val)) && 
				!memcmp(iter_jpn->cd_val_mark_seq, seq, strlen(seq)))
			{
				::CopyMemory(pTckKnd->tck_nm_ja, iter_jpn->cd_val_nm, strlen(iter_jpn->cd_val_nm));
				break;
			}
		}
	}

	return 0;
}

/**
 * @brief		SVC_CM_ReadTckKnd
 * @details		승차권 정보 조회
 * @param		int nOperID			FILE ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_CM_ReadTckKnd(int nOperID)
{
	int				nRet, nCount;
	HANDLE			hFile;
	CString			strFullName;
	rtk_cm_readtckknd_t Info;

	LOG_OPEN();
	LOG_WRITE(" [%d] 승차권 정보 조회_start !!!!", nOperID);

	nRet = nCount = 0;

	// Write file save
	OperGetFileName(nOperID, strFullName);
	hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		nRet = -100;
		LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
		LOG_CLOSE();
		return -1;
	}

	vector<rtk_cm_cdinqr_list_t>::iterator iter_kor;

	::ZeroMemory(&Info, sizeof(rtk_cm_readtckknd_t));

	::CopyMemory(Info.msg_cd, "S0000", 5);
	nCount = m_vtKorTck.size();
	::CopyMemory(Info.rec_ncnt1, &nCount, 4);

	MyWriteFile(hFile, &Info, sizeof(rtk_cm_readtckknd_t));

	nCount = 0;
	for(iter_kor = m_vtKorTck.begin(); iter_kor != m_vtKorTck.end(); iter_kor++)
	{
		rtk_cm_readtckknd_list_t List;

		::ZeroMemory(&List, sizeof(rtk_cm_readtckknd_list_t));

		///< 터미널번호
		//::CopyMemory(List.trml_no            [3+1]   ;   
		///< 티켓종류코드
		::CopyMemory(List.tck_knd_cd, iter_kor->cmn_cd_val, sizeof(List.tck_knd_cd));
		///< 화면출력순서
		::CopyMemory(List.scrn_prin_seq, iter_kor->cd_val_mark_seq, sizeof(List.scrn_prin_seq));
		///< 화면출력명
		//::CopyMemory(List.scrn_prin_nm       [100+1] ;   
		///< 한글

		if( GetEnvOperCorp() == KUMHO_OPER_CORP )
		{	/// 금호터미널인 경우...
			switch(iter_kor->cmn_cd_val[0])
			{
			case '1' :	///> 일반
				sprintf(List.tck_nm_ko, "어른");
				break;
			case '2' :	///> 초등생
				sprintf(List.tck_nm_ko, "어린이");
				break;
			case '9' :	///> 중고생
				sprintf(List.tck_nm_ko, "청소년");
				break;
			// 20211006 ADD
			case '8' :	///> 학생할인
				sprintf(List.tck_nm_ko, "대학생");
				break;
			// 20211006 ~ADD
			default:
				::CopyMemory(List.tck_nm_ko, iter_kor->cd_val_nm, sizeof(List.tck_nm_ko));
				break;
			}
		}
		else
		{
			::CopyMemory(List.tck_nm_ko, iter_kor->cd_val_nm, sizeof(List.tck_nm_ko));
		}

		AssignTicketKind(iter_kor->cmn_cd_val, iter_kor->cd_val_mark_seq, (char *)&List);

		//LOG_WRITE("#2. tck_cd[%s] tck_nm [%s][%s][%s][%s] ", List.tck_knd_cd, List.tck_nm_ko, List.tck_nm_en, List.tck_nm_ch, List.tck_nm_ja);

		MyWriteFile(hFile, &List, sizeof(rtk_cm_readtckknd_list_t));
		nCount++;
	}
	MyCloseFile(hFile);
	LOG_CLOSE();


	hFile = MyOpenFile(strFullName, FALSE);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	int i = 0;
	//TR_LOG_OUT("KOBUS_TicketKind Read ...");
	MyReadFile(hFile, &Info, sizeof(rtk_cm_readtckknd_t));
	while(1)
	{
		rtk_cm_readtckknd_list_t List;

		::ZeroMemory(&List, sizeof(rtk_cm_readtckknd_list_t));

		nRet = MyReadFile(hFile, &List, sizeof(rtk_cm_readtckknd_list_t));
		if( nRet <= 0 )
		{
			break;
		}
		
		//TR_LOG_OUT("[%02d], tck_knd_cd(%s), nm(%s) ", i++, List.tck_knd_cd, List.tck_nm_ko);
	}
	MyCloseFile(hFile);

	return nCount;
}

/**
 * @brief		SVC_MG_ReadWnd
 * @details		(TM_EWNDINFO) 창구 정보 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_MG_ReadWnd(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout;
	char*	pService = "TM_EWNDINFO";
	pstk_tm_ewndinfo_t pSPacket;
	prtk_tm_ewndinfo_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" start !!!!");

	pSPacket = (pstk_tm_ewndinfo_t) pData;
	pRPacket = (prtk_tm_ewndinfo_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ","LNG_CD",		pSPacket->lng_cd);
		LOG_WRITE("%30s - (%s) ","USER_NO",		pSPacket->user_no);

		LOG_WRITE("%30s - (%s) ","TRML_NO",		pSPacket->trml_no);
		LOG_WRITE("%30s - (%s) ","WND_NO",		pSPacket->wnd_no);
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD,	pSPacket->lng_cd);
		nRet = TMaxFBPut(USER_NO,	pSPacket->user_no);
		nRet = TMaxFBPut(TRML_NO,	pSPacket->trml_no);
		nRet = TMaxFBPut(WND_NO,	pSPacket->wnd_no);
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
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
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

		/// 예매발권가능일수
		nRet = TMaxFBGet(MRS_TISSU_PSB_DNO, PST_FIELD(pRPacket, mrs_tissu_psb_dno));
		/// 발권제한시간
		nRet = TMaxFBGet(TISSU_LTN_DRTM,	PST_FIELD(pRPacket, tissu_ltn_drtm));

		/// 예매발권가능일수
		MyWriteFile(hFile, pRPacket->mrs_tissu_psb_dno, sizeof(pRPacket->mrs_tissu_psb_dno));
		/// 발권제한시간
		MyWriteFile(hFile, pRPacket->tissu_ltn_drtm, sizeof(pRPacket->tissu_ltn_drtm));

		MyCloseFile(hFile);
	}

	// recv log
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD",				PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT",			PST_FIELD(pRPacket, msg_dtl_ctt));
		LOG_WRITE("%30s - (%d) ", "MRS_TISSU_PSB_DNO",	PST_FIELD(pRPacket, mrs_tissu_psb_dno));
		LOG_WRITE("%30s - (%d) ", "TISSU_LTN_DRTM",		PST_FIELD(pRPacket, tissu_ltn_drtm));
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
 * @brief		SVC_CM_ReadRotInf
 * @details		(TM_ETHRUINFO) 경유지 정보 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_CM_ReadRotInf(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	SYSTEMTIME	st;
	char	    szCurrDT[20];
	char*	pService = "TM_ETHRUINFO";
	pstk_tm_ethruinfo_t pSPacket;
	prtk_tm_ethruinfo_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" start !!!!");

	pSPacket = (pstk_tm_ethruinfo_t) pData;
	pRPacket = (prtk_tm_ethruinfo_t) retBuf;

	::GetLocalTime(&st);
	sprintf(szCurrDT, "%04d%02d%02d", st.wYear, st.wMonth, st.wDay);

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
		LOG_WRITE("%30s - (%s) ","LNG_CD",		pSPacket->lng_cd);
		LOG_WRITE("%30s - (%s) ","USER_NO",		pSPacket->user_no);

		LOG_WRITE("%30s - (%s) ","TISSU_TRML_NO"	,		pSPacket->tissu_trml_no 	);
		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO"	,		pSPacket->depr_trml_no 		);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"	,		pSPacket->arvl_trml_no 		);
		LOG_WRITE("%30s - (%s) ","ALCN_DEPR_TRML_NO"	,	pSPacket->alcn_depr_trml_no );
		LOG_WRITE("%30s - (%s) ","ALCN_ARVL_TRML_NO"	,	pSPacket->alcn_arvl_trml_no );
		LOG_WRITE("%30s - (%s) ","ALCN_ROT_NO"	,		pSPacket->alcn_rot_no 		);
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD				,	pSPacket->lng_cd			);
		nRet = TMaxFBPut(USER_NO			,	pSPacket->user_no			);

		nRet = TMaxFBPut(TISSU_TRML_NO 		,	pSPacket->tissu_trml_no 	);
		nRet = TMaxFBPut(DEPR_TRML_NO 		,	pSPacket->depr_trml_no 		);
		nRet = TMaxFBPut(ARVL_TRML_NO 		,	pSPacket->arvl_trml_no 		);
		nRet = TMaxFBPut(ALCN_DEPR_TRML_NO 	,	pSPacket->alcn_depr_trml_no );
		nRet = TMaxFBPut(ALCN_ARVL_TRML_NO 	,	pSPacket->alcn_arvl_trml_no );
		nRet = TMaxFBPut(ALCN_ROT_NO 		,	pSPacket->alcn_rot_no 		);
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
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		/// list count
		nRet = TMaxFBGet(REC_NCNT1, PST_FIELD(pRPacket, rec_ncnt1));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_ncnt1", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_tm_ethruinfo_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_ethruinfo_list_t) * nCount);
		}

		// set zero memory
		CPubTckKobusMem::GetInstance()->m_vtResThruList.clear();

		// file save
		OperGetFileName(nOperID, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nRet = -100;
			LOG_WRITE("MyOpenFile(strFullName, TRUE) Error !!!!");
			goto fail_proc;
		}

		MyWriteFile(hFile, pRPacket->msg_cd, sizeof(pRPacket->msg_cd));
		MyWriteFile(hFile, pRPacket->msg_dtl_ctt, sizeof(pRPacket->msg_dtl_ctt));
		MyWriteFile(hFile, pRPacket->rec_ncnt1, sizeof(pRPacket->rec_ncnt1));
		
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_ethruinfo_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(TISSU_TRML_NO 	,	PST_FIELD(pList, tissu_trml_no 		));
			nRet = TMaxFBGetF(DEPR_TRML_NO 		,	PST_FIELD(pList, depr_trml_no 		));
			nRet = TMaxFBGetF(ARVL_TRML_NO 		,	PST_FIELD(pList, arvl_trml_no 		));
			nRet = TMaxFBGetF(ALCN_DEPR_TRML_NO ,	PST_FIELD(pList, alcn_depr_trml_no 	));
			nRet = TMaxFBGetF(ALCN_ARVL_TRML_NO ,	PST_FIELD(pList, alcn_arvl_trml_no 	));
			nRet = TMaxFBGetF(ALCN_ROT_NO 		,	PST_FIELD(pList, alcn_rot_no 		));
			nRet = TMaxFBGetF(STT_DT 			,	PST_FIELD(pList, stt_dt 			));
			nRet = TMaxFBGetF(END_DT 			,	PST_FIELD(pList, end_dt 			));
			nRet = TMaxFBGetF(DEPR_THRU_SEQ 	,	PST_FIELD(pList, depr_thru_seq 		));
			nRet = TMaxFBGetF(ARVL_THRU_SEQ 	,	PST_FIELD(pList, arvl_thru_seq 		));
			nRet = TMaxFBGetF(THRU_DIST 		,	PST_FIELD(pList, thru_dist 			));
			nRet = TMaxFBGetF(THRU_DRTM 		,	PST_FIELD(pList, thru_drtm 			));

			if( (memcmp(pList->stt_dt, szCurrDT, strlen(szCurrDT))) > 0 || (memcmp(szCurrDT, pList->end_dt, strlen(szCurrDT)) > 0) )
			{
				LOG_WRITE("경유지 skip, stdt(%s), enddt(%s)", pList->stt_dt, pList->end_dt);
				continue;
			}

			MyWriteFile(hFile, pList, sizeof(rtk_tm_ethruinfo_list_t));

			// set memory
			CPubTckKobusMem::GetInstance()->m_vtResThruList.push_back(*pList);
		}
		MyCloseFile(hFile);
	}

	// recv log
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD", PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT", PST_FIELD(pRPacket, msg_dtl_ctt));
		LOG_WRITE("%30s - (%d) ", "REC_NCNT1", nCount);
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_ethruinfo_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TISSU_TRML_NO"		,	PST_FIELD(pList, tissu_trml_no 		));
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"		,	PST_FIELD(pList, depr_trml_no 		));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"		,	PST_FIELD(pList, arvl_trml_no 		));
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO"	,	PST_FIELD(pList, alcn_depr_trml_no 	));
			LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO"	,	PST_FIELD(pList, alcn_arvl_trml_no 	));
			LOG_WRITE("%30s - (%s) ", "ALCN_ROT_NO"			,	PST_FIELD(pList, alcn_rot_no 		));
			LOG_WRITE("%30s - (%s) ", "STT_DT"				,	PST_FIELD(pList, stt_dt 			));
			LOG_WRITE("%30s - (%s) ", "END_DT"				,	PST_FIELD(pList, end_dt 			));
			LOG_WRITE("%30s - (%d) ", "DEPR_THRU_SEQ"		,	*(int *) pList->depr_thru_seq 		);
			LOG_WRITE("%30s - (%d) ", "ARVL_THRU_SEQ"		,	*(int *) pList->arvl_thru_seq 		);
			LOG_WRITE("%30s - (%s) ", "THRU_DIST"			,	PST_FIELD(pList, thru_dist 			));
			LOG_WRITE("%30s - (%s) ", "THRU_DRTM"			,	PST_FIELD(pList, thru_drtm 			));
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
 * @brief		SVC_MG_ReadAlcn
 * @details		(TM_TIMINFO) 배차 정보 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_MG_ReadAlcn(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout, nRealCount;
	char*	pService = "TM_TIMINFO";
	pstk_tm_timinfo_t pSPacket;
	prtk_tm_timinfo_t pRPacket;
	PUI_BASE_T	pBaseInfo;

	LOG_OPEN();
	LOG_WRITE(" start !!!!");

	pSPacket = (pstk_tm_timinfo_t) pData;
	pRPacket = (prtk_tm_timinfo_t) retBuf;

	pBaseInfo = (PUI_BASE_T) GetConfigBaseInfo();

	nRet = i = nCount = nTimeout = nRealCount = 0;

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
		LOG_WRITE("%30s - (%s) ","LNG_CD"		,	pSPacket->lng_cd);
		LOG_WRITE("%30s - (%s) ","USER_NO"		,	pSPacket->user_no);

		LOG_WRITE("%30s - (%s) ","DEPR_TRML_NO"	,	pSPacket->depr_trml_no 	);
		LOG_WRITE("%30s - (%s) ","ARVL_TRML_NO"	,	pSPacket->arvl_trml_no 	);
		LOG_WRITE("%30s - (%s) ","DEPR_DT"		,	pSPacket->depr_dt      	);
		LOG_WRITE("%30s - (%s) ","DEPR_TIME"	,	pSPacket->depr_time    	);
		LOG_WRITE("%30s - (%s) ","BUS_CLS_CD"	,	pSPacket->bus_cls_cd   	);
		LOG_WRITE("%30s - (%d) ","REC_NCNT"		,	*(int *)pSPacket->rec_ncnt	);
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD			,	pSPacket->lng_cd			);
		nRet = TMaxFBPut(USER_NO		,	pSPacket->user_no			);

		nRet = TMaxFBPut(DEPR_TRML_NO 	 ,	pSPacket->depr_trml_no 	 	);
		nRet = TMaxFBPut(ARVL_TRML_NO 	 ,	pSPacket->arvl_trml_no 	 	);
		nRet = TMaxFBPut(DEPR_DT      	 ,	pSPacket->depr_dt      	 	);
		nRet = TMaxFBPut(DEPR_TIME    	 ,	pSPacket->depr_time    	 	);
		nRet = TMaxFBPut(BUS_CLS_CD   	 ,	pSPacket->bus_cls_cd   	 	);
		nRet = TMaxFBPut(REC_NCNT	 	 ,	pSPacket->rec_ncnt	 	 	);
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
		::ZeroMemory(&CPubTckKobusMem::GetInstance()->m_vtResAlcnInfo, sizeof(rtk_tm_timinfo_fee_t));
		CPubTckKobusMem::GetInstance()->m_vtResAlcnList.clear();

		/// msg_cd
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		{
			prtk_tm_timinfo_fee_t pFee;

			pFee = &pRPacket->fee_t;

			nRet = TMaxFBGet(TAKE_DRTM						, PST_FIELD(pFee, take_drtm						));	 ///< (int)소요시간				
			nRet = TMaxFBGet(BUS_OPRN_DIST					, PST_FIELD(pFee, bus_oprn_dist					));	 ///< 거리					

			nRet = TMaxFBGet(PRMM_ADLT_FEE     				, PST_FIELD(pFee, prmm_adlt_fee     			));	 ///< 1(int)우등어른 요금			
			nRet = TMaxFBGet(PRMM_TEEN_FEE					, PST_FIELD(pFee, prmm_teen_fee					));	 ///< (int)우등청소년 요금			
			nRet = TMaxFBGet(PRMM_CHLD_FEE     				, PST_FIELD(pFee, prmm_chld_fee     			));	 ///< (int)우등어린이 요금	
			nRet = TMaxFBGet(PRMM_UVSD_FEE     				, PST_FIELD(pFee, prmm_uvsd_fee     			));	 ///< (int)우등대학생 요금	

 			nRet = TMaxFBGet(PRMM_VTRN3_FEE     			, PST_FIELD(pFee, prmm_vtrn3_fee     			));	 ///< 2(int)우등보훈30 요금			
 			nRet = TMaxFBGet(PRMM_VTRN5_FEE					, PST_FIELD(pFee, prmm_vtrn5_fee				));	 ///< (int)우등보훈50 요금			
 			nRet = TMaxFBGet(PRMM_VTRN7_FEE     			, PST_FIELD(pFee, prmm_vtrn7_fee     			));	 ///< (int)우등보훈70 요금	
 			nRet = TMaxFBGet(PRMM_ARMY2_FEE     			, PST_FIELD(pFee, prmm_army2_fee     			));	 ///< (int)우등군인20 요금	

			nRet = TMaxFBGet(HSPD_ADLT_FEE     				, PST_FIELD(pFee, hspd_adlt_fee     			));	 ///< 3(int)고속어른 요금			
			nRet = TMaxFBGet(HSPD_TEEN_FEE     				, PST_FIELD(pFee, hspd_teen_fee     			));	 ///< (int)고속청소년 요금			
			nRet = TMaxFBGet(HSPD_CHLD_FEE     				, PST_FIELD(pFee, hspd_chld_fee     			));	 ///< (int)고속어린이 요금			
			nRet = TMaxFBGet(HSPD_UVSD_FEE     				, PST_FIELD(pFee, hspd_uvsd_fee     			));	 ///< (int)고속대학생 요금			

 			nRet = TMaxFBGet(HSPD_VTRN3_FEE     			, PST_FIELD(pFee, hspd_vtrn3_fee     			));	 ///< 4(int)고속보훈30 요금			
 			nRet = TMaxFBGet(HSPD_VTRN5_FEE     			, PST_FIELD(pFee, hspd_vtrn5_fee     			));	 ///< (int)고속보훈50 요금			
 			nRet = TMaxFBGet(HSPD_VTRN7_FEE     			, PST_FIELD(pFee, hspd_vtrn7_fee     			));	 ///< (int)고속보훈70 요금			
 			nRet = TMaxFBGet(HSPD_ARMY2_FEE     			, PST_FIELD(pFee, hspd_army2_fee     			));	 ///< (int)고속군인20 요금			

			nRet = TMaxFBGet(MDNT_PRMM_ADLT_FEE				, PST_FIELD(pFee, mdnt_prmm_adlt_fee			));	 ///< 5(int)심우어른 요금			
			nRet = TMaxFBGet(MDNT_PRMM_TEEN_FEE				, PST_FIELD(pFee, mdnt_prmm_teen_fee			));	 ///< (int)심우청소년 요금			
			nRet = TMaxFBGet(MDNT_PRMM_CHLD_FEE				, PST_FIELD(pFee, mdnt_prmm_chld_fee			));	 ///< (int)심우어린이 요금		
			nRet = TMaxFBGet(MDNT_PRMM_UVSD_FEE				, PST_FIELD(pFee, mdnt_prmm_uvsd_fee			));	 ///< (int)심우대학생 요금		

 			nRet = TMaxFBGet(MDNT_PRMM_VTRN3_FEE			, PST_FIELD(pFee, mdnt_prmm_vtrn3_fee			));	 ///< 6(int)심우보훈30 요금			
 			nRet = TMaxFBGet(MDNT_PRMM_VTRN5_FEE			, PST_FIELD(pFee, mdnt_prmm_vtrn5_fee			));	 ///< (int)심우보훈50 요금			
 			nRet = TMaxFBGet(MDNT_PRMM_VTRN7_FEE			, PST_FIELD(pFee, mdnt_prmm_vtrn7_fee			));	 ///< (int)심우보훈70 요금		
 			nRet = TMaxFBGet(MDNT_PRMM_ARMY2_FEE			, PST_FIELD(pFee, mdnt_prmm_army2_fee			));	 ///< (int)심우군인20 요금		

			nRet = TMaxFBGet(MDNT_HSPD_ADLT_FEE				, PST_FIELD(pFee, mdnt_hspd_adlt_fee			));	 ///< 7(int)심고어른 요금			
			nRet = TMaxFBGet(MDNT_HSPD_TEEN_FEE				, PST_FIELD(pFee, mdnt_hspd_teen_fee			));	 ///< (int)심고청소년 요금			
			nRet = TMaxFBGet(MDNT_HSPD_CHLD_FEE				, PST_FIELD(pFee, mdnt_hspd_chld_fee			));	 ///< (int)심고어린이 요금			
			nRet = TMaxFBGet(MDNT_HSPD_UVSD_FEE				, PST_FIELD(pFee, mdnt_hspd_uvsd_fee			));	 ///< (int)심고대학생 요금			

 			nRet = TMaxFBGet(MDNT_HSPD_VTRN3_FEE			, PST_FIELD(pFee, mdnt_hspd_vtrn3_fee			));	 ///< 8(int)심고보훈30 요금			
 			nRet = TMaxFBGet(MDNT_HSPD_VTRN5_FEE			, PST_FIELD(pFee, mdnt_hspd_vtrn5_fee			));	 ///< (int)심고보훈50 요금			
 			nRet = TMaxFBGet(MDNT_HSPD_VTRN7_FEE			, PST_FIELD(pFee, mdnt_hspd_vtrn7_fee			));	 ///< (int)심고보훈70 요금			
 			nRet = TMaxFBGet(MDNT_HSPD_ARMY2_FEE			, PST_FIELD(pFee, mdnt_hspd_army2_fee			));	 ///< (int)심고군인20 요금			

			nRet = TMaxFBGet(MDNT_PRMM_EXCH_ADLT_FEE		, PST_FIELD(pFee, mdnt_prmm_exch_adlt_fee		));	 ///< 9(int)심우할증 어른 요금 		
			nRet = TMaxFBGet(MDNT_PRMM_EXCH_TEEN_FEE		, PST_FIELD(pFee, mdnt_prmm_exch_teen_fee		));	 ///< (int)심우할증 청소년 요금		
			nRet = TMaxFBGet(MDNT_PRMM_EXCH_CHLD_FEE		, PST_FIELD(pFee, mdnt_prmm_exch_chld_fee		));	 ///< (int)심우할증 어린이 요금		
			nRet = TMaxFBGet(MDNT_PRMM_EXCH_UVSD_FEE		, PST_FIELD(pFee, mdnt_prmm_exch_uvsd_fee		));	 ///< (int)심우할증 대학생 요금		

 			nRet = TMaxFBGet(MDNT_PRMM_EXCH_VTRN3_FEE		, PST_FIELD(pFee, mdnt_prmm_exch_vtrn3_fee		));	 ///< 10(int)심우할증 보훈30 요금 		
 			nRet = TMaxFBGet(MDNT_PRMM_EXCH_VTRN5_FEE		, PST_FIELD(pFee, mdnt_prmm_exch_vtrn5_fee		));	 ///< (int)심우할증 보훈50 요금		
 			nRet = TMaxFBGet(MDNT_PRMM_EXCH_VTRN7_FEE		, PST_FIELD(pFee, mdnt_prmm_exch_vtrn7_fee		));	 ///< (int)심우할증 보훈70 요금		
 			nRet = TMaxFBGet(MDNT_PRMM_EXCH_ARMY2_FEE		, PST_FIELD(pFee, mdnt_prmm_exch_army2_fee		));	 ///< (int)심우할증 군인20 요금		

			nRet = TMaxFBGet(MDNT_HSPD_EXCH_ADLT_FEE		, PST_FIELD(pFee, mdnt_hspd_exch_adlt_fee		));	 ///< 11(int)심고할증 어른 요금			
			nRet = TMaxFBGet(MDNT_HSPD_EXCH_TEEN_FEE		, PST_FIELD(pFee, mdnt_hspd_exch_teen_fee		));	 ///< (int)심고할증 청소년 요금		
			nRet = TMaxFBGet(MDNT_HSPD_EXCH_CHLD_FEE		, PST_FIELD(pFee, mdnt_hspd_exch_chld_fee		));	 ///< (int)심고할증 아동 요금			
			nRet = TMaxFBGet(MDNT_HSPD_EXCH_UVSD_FEE		, PST_FIELD(pFee, mdnt_hspd_exch_uvsd_fee		));	 ///< (int)심고할증 대학생 요금			

 			nRet = TMaxFBGet(MDNT_HSPD_EXCH_VTRN3_FEE		, PST_FIELD(pFee, mdnt_hspd_exch_vtrn3_fee		));	 ///< 12(int)심고할증 보훈30 요금		
 			nRet = TMaxFBGet(MDNT_HSPD_EXCH_VTRN5_FEE		, PST_FIELD(pFee, mdnt_hspd_exch_vtrn5_fee		));	 ///< (int)심고할증 보훈50 요금		
 			nRet = TMaxFBGet(MDNT_HSPD_EXCH_VTRN7_FEE		, PST_FIELD(pFee, mdnt_hspd_exch_vtrn7_fee		));	 ///< (int)심고할증 보훈70 요금			
 			nRet = TMaxFBGet(MDNT_HSPD_EXCH_ARMY2_FEE		, PST_FIELD(pFee, mdnt_hspd_exch_army2_fee		));	 ///< (int)심고할증 군인20 요금			

			nRet = TMaxFBGet(PSRM_ADLT_FEE					, PST_FIELD(pFee, psrm_adlt_fee					));	 ///< 13(int)프리미엄 어른 요금 		
			nRet = TMaxFBGet(PSRM_TEEN_FEE					, PST_FIELD(pFee, psrm_teen_fee					));	 ///< (int)프리미엄 청소년 요금		
			nRet = TMaxFBGet(PSRM_CHLD_FEE					, PST_FIELD(pFee, psrm_chld_fee					));	 ///< (int)프리미엄 어린이 요금		
			nRet = TMaxFBGet(PSRM_UVSD_FEE					, PST_FIELD(pFee, psrm_uvsd_fee					));	 ///< (int)프리미엄 대학생 요금		

 			nRet = TMaxFBGet(PSRM_VTRN3_FEE					, PST_FIELD(pFee, psrm_vtrn3_fee				));	 ///< 14(int)프리미엄 보훈30 요금 		
 			nRet = TMaxFBGet(PSRM_VTRN5_FEE					, PST_FIELD(pFee, psrm_vtrn5_fee				));	 ///< (int)프리미엄 보훈50 요금		
 			nRet = TMaxFBGet(PSRM_VTRN7_FEE					, PST_FIELD(pFee, psrm_vtrn7_fee				));	 ///< (int)프리미엄 보훈70 요금		
 			nRet = TMaxFBGet(PSRM_ARMY2_FEE					, PST_FIELD(pFee, psrm_army2_fee				));	 ///< (int)프리미엄 군인20 요금		

			nRet = TMaxFBGet(MDNT_PSRM_ADLT_FEE				, PST_FIELD(pFee, mdnt_psrm_adlt_fee			));	 ///< 15(int)심야프리미엄 어른 요금 		
			nRet = TMaxFBGet(MDNT_PSRM_TEEN_FEE				, PST_FIELD(pFee, mdnt_psrm_teen_fee			));	 ///< (int)심야프리미엄 청소년 요금		
			nRet = TMaxFBGet(MDNT_PSRM_CHLD_FEE				, PST_FIELD(pFee, mdnt_psrm_chld_fee			));	 ///< (int)심야프리미엄 어린이 요금		
			nRet = TMaxFBGet(MDNT_PSRM_UVSD_FEE				, PST_FIELD(pFee, mdnt_psrm_uvsd_fee			));	 ///< (int)심야프리미엄 대학생 요금		

 			nRet = TMaxFBGet(MDNT_PSRM_VTRN3_FEE			, PST_FIELD(pFee, mdnt_psrm_vtrn3_fee			));	 ///< 16(int)심야프리미엄 보훈30 요금 	
 			nRet = TMaxFBGet(MDNT_PSRM_VTRN5_FEE			, PST_FIELD(pFee, mdnt_psrm_vtrn5_fee			));	 ///< (int)심야프리미엄 보훈50 요금		
 			nRet = TMaxFBGet(MDNT_PSRM_VTRN7_FEE			, PST_FIELD(pFee, mdnt_psrm_vtrn7_fee			));	 ///< (int)심야프리미엄 보훈70 요금		
 			nRet = TMaxFBGet(MDNT_PSRM_ARMY2_FEE			, PST_FIELD(pFee, mdnt_psrm_army2_fee			));	 ///< (int)심야프리미엄 군인20 요금		

			nRet = TMaxFBGet(MDNT_PSRM_EXCH_ADLT_FEE		, PST_FIELD(pFee, mdnt_psrm_exch_adlt_fee		));	 ///< 17(int)심야할증프리미엄 어른 요금 	
			nRet = TMaxFBGet(MDNT_PSRM_EXCH_TEEN_FEE		, PST_FIELD(pFee, mdnt_psrm_exch_teen_fee		));	 ///< (int)심야할증프리미엄 청소년 요금	
			nRet = TMaxFBGet(MDNT_PSRM_EXCH_CHLD_FEE		, PST_FIELD(pFee, mdnt_psrm_exch_chld_fee		));	 ///< (int)심야할증프리미엄 어린이 요금	
			nRet = TMaxFBGet(MDNT_PSRM_EXCH_UVSD_FEE		, PST_FIELD(pFee, mdnt_psrm_exch_uvsd_fee		));	 ///< (int)심야할증프리미엄 대학생 요금	

 			nRet = TMaxFBGet(MDNT_PSRM_EXCH_VTRN3_FEE		, PST_FIELD(pFee, mdnt_psrm_exch_vtrn3_fee		));	 ///< 18(int)심야할증프리미엄 보훈30 요금 	
 			nRet = TMaxFBGet(MDNT_PSRM_EXCH_VTRN5_FEE		, PST_FIELD(pFee, mdnt_psrm_exch_vtrn5_fee		));	 ///< (int)심야할증프리미엄 보훈50 요금	
 			nRet = TMaxFBGet(MDNT_PSRM_EXCH_VTRN7_FEE		, PST_FIELD(pFee, mdnt_psrm_exch_vtrn7_fee		));	 ///< (int)심야할증프리미엄 보훈70 요금	
 			nRet = TMaxFBGet(MDNT_PSRM_EXCH_ARMY2_FEE		, PST_FIELD(pFee, mdnt_psrm_exch_army2_fee		));	 ///< (int)심야할증프리미엄 군인20 요금	

			nRet = TMaxFBGet(DC_FEE							, PST_FIELD(pFee, dc_fee						));	 ///< (int)시외우등뒷좌석할인요금	
			nRet = TMaxFBGet(CTY_DC_FEE3					, PST_FIELD(pFee, cty_dc_fee3					));	 ///< (int)심야우등뒷좌석할인요금	
			nRet = TMaxFBGet(FN_YN							, PST_FIELD(pFee, fn_yn							));	 ///< 마지막 여부 YN

			/// set memory
			::CopyMemory(&CPubTckKobusMem::GetInstance()->m_vtResAlcnInfo, &pRPacket->fee_t, sizeof(rtk_tm_timinfo_fee_t));
		}

		/// list count
		nRet = TMaxFBGet(REC_NCNT1, PST_FIELD(pRPacket, rec_ncnt1));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_ncnt1", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE777");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_tm_timinfo_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_timinfo_list_t) * nCount);
		}

		for(i = 0; i < nCount; i++)
		{
			prtk_tm_timinfo_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(TRTR_TRML_INCL_YN	,	PST_FIELD(pList, trtr_trml_incl_yn	));
			nRet = TMaxFBGetF(TEMP_ROT_YN      	,	PST_FIELD(pList, temp_rot_yn      	));
			nRet = TMaxFBGetF(DRTM_MOD_PSB_YN   ,	PST_FIELD(pList, drtm_mod_psb_yn   	));
			nRet = TMaxFBGetF(ALCN_DEPR_TRML_NO ,	PST_FIELD(pList, alcn_depr_trml_no 	));
			nRet = TMaxFBGetF(ALCN_ARVL_TRML_NO ,	PST_FIELD(pList, alcn_arvl_trml_no 	));
			nRet = TMaxFBGetF(DEPR_TRML_NO      ,	PST_FIELD(pList, depr_trml_no      	));
			nRet = TMaxFBGetF(ARVL_TRML_NO      ,	PST_FIELD(pList, arvl_trml_no      	));
			nRet = TMaxFBGetF(DEPR_DT           ,	PST_FIELD(pList, depr_dt           	));
			nRet = TMaxFBGetF(DEPR_TIME         ,	PST_FIELD(pList, depr_time         	));

			nRet = TMaxFBGetF(ALCN_DEPR_DT		,	PST_FIELD(pList, alcn_depr_dt    	));

			nRet = TMaxFBGetF(ALCN_DEPR_TIME    ,	PST_FIELD(pList, alcn_depr_time    	));
			nRet = TMaxFBGetF(BUS_CLS_CD        ,	PST_FIELD(pList, bus_cls_cd        	));
			nRet = TMaxFBGetF(CACM_CD           ,	PST_FIELD(pList, cacm_cd           	));
			nRet = TMaxFBGetF(RMN_SATS_NUM      ,	PST_FIELD(pList, rmn_sats_num      	));
			nRet = TMaxFBGetF(TOT_SATS_NUM      ,	PST_FIELD(pList, tot_sats_num      	));
			nRet = TMaxFBGetF(CTY_PRMM_DC_YN 	,	PST_FIELD(pList, cty_prmm_dc_yn 	));
			nRet = TMaxFBGetF(DEPR_THRU_SEQ		,	PST_FIELD(pList, depr_thru_seq		));
			nRet = TMaxFBGetF(ARVL_THRU_SEQ		,	PST_FIELD(pList, arvl_thru_seq		));
			nRet = TMaxFBGetF(SATS_MLTP_VAL		,	PST_FIELD(pList, sats_mltp_val		));
			nRet = TMaxFBGet(DSPR_SATS_YN		,	PST_FIELD(pList, dspr_sats_yn		));	 ///< 휠체어 좌석 여부 YN
			/// 2020.05.18 필드 추가
			nRet = TMaxFBGet(ALCN_ROT_NO		,	PST_FIELD(pList, alcn_rot_no		));	 ///< 배차노선번호

			// 매진배차 표시유부 옵션 
			if(pBaseInfo->tck_no_rmn_yn == 'N')
			{	/// 미표시
				int n_rmn_sats_num;

				n_rmn_sats_num = *(int *) pList->rmn_sats_num;
				if( n_rmn_sats_num <= 0 )
				{
					continue;
				}
			}
			
			// set memory
			CPubTckKobusMem::GetInstance()->m_vtResAlcnList.push_back(*pList);
			nRealCount++;
		}
	}

	// recv log
	{
		prtk_tm_timinfo_fee_t pFee;

		pFee = &pRPacket->fee_t;

		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD", PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT", PST_FIELD(pRPacket, msg_dtl_ctt));

		LOG_WRITE("%30s - (%s) ", "TAKE_DRTM"				, PST_FIELD(pFee, take_drtm							));		 ///< (int)소요시간	
		LOG_WRITE("%30s - (%s) ", "BUS_OPRN_DIST"			, PST_FIELD(pFee, bus_oprn_dist						));		 ///< 거리			
		
		LOG_WRITE("%30s - (%d) ", "PRMM_ADLT_FEE"			, *(int *) PST_FIELD(pFee, prmm_adlt_fee     		));		 ///< 1(int)우등어른 요금			
		LOG_WRITE("%30s - (%d) ", "PRMM_TEEN_FEE"			, *(int *) PST_FIELD(pFee, prmm_teen_fee			));		 ///< (int)우등청소년 요금			
		LOG_WRITE("%30s - (%d) ", "PRMM_CHLD_FEE"			, *(int *) PST_FIELD(pFee, prmm_chld_fee     		));		 ///< (int)우등어린이 요금	
		LOG_WRITE("%30s - (%d) ", "PRMM_UVSD_FEE"			, *(int *) PST_FIELD(pFee, prmm_uvsd_fee     		));		 ///< (int)우등대학생 요금	
		
		LOG_WRITE("%30s - (%d) ", "PRMM_VTRN3_FEE"			, *(int *) PST_FIELD(pFee, prmm_vtrn3_fee     		));		 ///< 2(int)우등보훈30 요금			
		LOG_WRITE("%30s - (%d) ", "PRMM_VTRN5_FEE"			, *(int *) PST_FIELD(pFee, prmm_vtrn5_fee			));		 ///< (int)우등보훈50 요금			
		LOG_WRITE("%30s - (%d) ", "PRMM_VTRN7_FEE"			, *(int *) PST_FIELD(pFee, prmm_vtrn7_fee     		));		 ///< (int)우등보훈70 요금	
		LOG_WRITE("%30s - (%d) ", "PRMM_ARMY2_FEE"			, *(int *) PST_FIELD(pFee, prmm_army2_fee     		));		 ///< (int)우등군인20 요금	

		LOG_WRITE("%30s - (%d) ", "HSPD_ADLT_FEE"			, *(int *) PST_FIELD(pFee, hspd_adlt_fee     		));		 ///< 3(int)고속어른 요금			
		LOG_WRITE("%30s - (%d) ", "HSPD_TEEN_FEE"			, *(int *) PST_FIELD(pFee, hspd_teen_fee     		));		 ///< (int)고속청소년 요금			
		LOG_WRITE("%30s - (%d) ", "HSPD_CHLD_FEE"			, *(int *) PST_FIELD(pFee, hspd_chld_fee     		));		 ///< (int)고속어린이 요금			
		LOG_WRITE("%30s - (%d) ", "HSPD_UVSD_FEE"			, *(int *) PST_FIELD(pFee, hspd_uvsd_fee     		));		 ///< (int)고속대학생 요금			
		
		LOG_WRITE("%30s - (%d) ", "HSPD_VTRN3_FEE"			, *(int *) PST_FIELD(pFee, hspd_vtrn3_fee     		));		 ///< 4(int)고속보훈30 요금			
		LOG_WRITE("%30s - (%d) ", "HSPD_VTRN5_FEE"			, *(int *) PST_FIELD(pFee, hspd_vtrn5_fee			));		 ///< (int)고속보훈50 요금			
		LOG_WRITE("%30s - (%d) ", "HSPD_VTRN7_FEE"			, *(int *) PST_FIELD(pFee, hspd_vtrn7_fee     		));		 ///< (int)고속보훈70 요금			
		LOG_WRITE("%30s - (%d) ", "HSPD_ARMY2_FEE"			, *(int *) PST_FIELD(pFee, hspd_army2_fee     		));		 ///< (int)고속군인20 요금			

		LOG_WRITE("%30s - (%d) ", "MDNT_PRMM_ADLT_FEE"		, *(int *) PST_FIELD(pFee, mdnt_prmm_adlt_fee		));		 ///< 5(int)심우어른 요금			
		LOG_WRITE("%30s - (%d) ", "MDNT_PRMM_TEEN_FEE"		, *(int *) PST_FIELD(pFee, mdnt_prmm_teen_fee		));		 ///< (int)심우청소년 요금			
		LOG_WRITE("%30s - (%d) ", "MDNT_PRMM_CHLD_FEE"		, *(int *) PST_FIELD(pFee, mdnt_prmm_chld_fee		));		 ///< (int)심우어린이 요금		
		LOG_WRITE("%30s - (%d) ", "MDNT_PRMM_UVSD_FEE"		, *(int *) PST_FIELD(pFee, mdnt_prmm_uvsd_fee		));		 ///< (int)심우대학생 요금		

		LOG_WRITE("%30s - (%d) ", "mdnt_prmmVTRN3_FEE"		, *(int *) PST_FIELD(pFee, mdnt_prmm_vtrn3_fee     	));		 ///< 6(int)심우보훈30 요금			
		LOG_WRITE("%30s - (%d) ", "mdnt_prmmVTRN5_FEE"		, *(int *) PST_FIELD(pFee, mdnt_prmm_vtrn5_fee		));		 ///< (int)심우보훈50 요금			
		LOG_WRITE("%30s - (%d) ", "mdnt_prmmVTRN7_FEE"		, *(int *) PST_FIELD(pFee, mdnt_prmm_vtrn7_fee     	));		 ///< (int)심우보훈70 요금		
		LOG_WRITE("%30s - (%d) ", "mdnt_prmmARMY2_FEE"		, *(int *) PST_FIELD(pFee, mdnt_prmm_army2_fee     	));		 ///< (int)심우군인20 요금		

		LOG_WRITE("%30s - (%d) ", "MDNT_HSPD_ADLT_FEE"		, *(int *) PST_FIELD(pFee, mdnt_hspd_adlt_fee		));		 ///< 7(int)심고어른 요금			
		LOG_WRITE("%30s - (%d) ", "MDNT_HSPD_TEEN_FEE"		, *(int *) PST_FIELD(pFee, mdnt_hspd_teen_fee		));		 ///< (int)심고청소년 요금			
		LOG_WRITE("%30s - (%d) ", "MDNT_HSPD_CHLD_FEE"		, *(int *) PST_FIELD(pFee, mdnt_hspd_chld_fee		));		 ///< (int)심고어린이 요금			
		LOG_WRITE("%30s - (%d) ", "MDNT_HSPD_UVSD_FEE"		, *(int *) PST_FIELD(pFee, mdnt_hspd_uvsd_fee		));		 ///< (int)심고대학생 요금			
		
		LOG_WRITE("%30s - (%d) ", "MDNT_HSPD_VTRN3_FEE"		, *(int *) PST_FIELD(pFee, mdnt_hspd_vtrn3_fee		));		 ///< 8(int)심고보훈30 요금			
		LOG_WRITE("%30s - (%d) ", "MDNT_HSPD_VTRN5_FEE"		, *(int *) PST_FIELD(pFee, mdnt_hspd_vtrn5_fee		));		 ///< (int)심고보훈50 요금			
		LOG_WRITE("%30s - (%d) ", "MDNT_HSPD_VTRN7_FEE"		, *(int *) PST_FIELD(pFee, mdnt_hspd_vtrn7_fee		));		 ///< (int)심고보훈70 요금			
		LOG_WRITE("%30s - (%d) ", "MDNT_HSPD_ARMY2_FEE"		, *(int *) PST_FIELD(pFee, mdnt_hspd_army2_fee		));		 ///< (int)심고군인20 요금			

		LOG_WRITE("%30s - (%d) ", "MDNT_PRMM_EXCH_ADLT_FEE"	, *(int *) PST_FIELD(pFee, mdnt_prmm_exch_adlt_fee	));		 ///< 9(int)심우할증 어른 요금 		
		LOG_WRITE("%30s - (%d) ", "MDNT_PRMM_EXCH_TEEN_FEE"	, *(int *) PST_FIELD(pFee, mdnt_prmm_exch_teen_fee	));		 ///< (int)심우할증 청소년 요금		
		LOG_WRITE("%30s - (%d) ", "MDNT_PRMM_EXCH_CHLD_FEE"	, *(int *) PST_FIELD(pFee, mdnt_prmm_exch_chld_fee	));		 ///< (int)심우할증 어린이 요금		
		LOG_WRITE("%30s - (%d) ", "MDNT_PRMM_EXCH_UVSD_FEE"	, *(int *) PST_FIELD(pFee, mdnt_prmm_exch_uvsd_fee	));		 ///< (int)심우할증 대학생 요금		

		LOG_WRITE("%30s - (%d) ", "MDNT_PRMM_EXCH_VTRN3_FEE", *(int *) PST_FIELD(pFee, mdnt_prmm_exch_vtrn3_fee	));		 ///< 10(int)심우할증 보훈30 요금 		
		LOG_WRITE("%30s - (%d) ", "MDNT_PRMM_EXCH_VTRN5_FEE", *(int *) PST_FIELD(pFee, mdnt_prmm_exch_vtrn5_fee	));		 ///< (int)심우할증 보훈50 요금		
		LOG_WRITE("%30s - (%d) ", "MDNT_PRMM_EXCH_VTRN7_FEE", *(int *) PST_FIELD(pFee, mdnt_prmm_exch_vtrn7_fee	));		 ///< (int)심우할증 보훈70 요금		
		LOG_WRITE("%30s - (%d) ", "MDNT_PRMM_EXCH_ARMY2_FEE", *(int *) PST_FIELD(pFee, mdnt_prmm_exch_army2_fee	));		 ///< (int)심우할증 군인20 요금		

		LOG_WRITE("%30s - (%d) ", "MDNT_HSPD_EXCH_ADLT_FEE"	, *(int *) PST_FIELD(pFee, mdnt_hspd_exch_adlt_fee	));		 ///< 11(int)심고할증 어른 요금			
		LOG_WRITE("%30s - (%d) ", "MDNT_HSPD_EXCH_TEEN_FEE"	, *(int *) PST_FIELD(pFee, mdnt_hspd_exch_teen_fee	));		 ///< (int)심고할증 청소년 요금		
		LOG_WRITE("%30s - (%d) ", "MDNT_HSPD_EXCH_CHLD_FEE"	, *(int *) PST_FIELD(pFee, mdnt_hspd_exch_chld_fee	));		 ///< (int)심고할증 아동 요금			
		LOG_WRITE("%30s - (%d) ", "MDNT_HSPD_EXCH_UVSD_FEE"	, *(int *) PST_FIELD(pFee, mdnt_hspd_exch_uvsd_fee	));		 ///< (int)심고할증 대학생 요금			

		LOG_WRITE("%30s - (%d) ", "MDNT_HSPD_EXCH_VTRN3_FEE", *(int *) PST_FIELD(pFee, mdnt_hspd_exch_vtrn3_fee	));		 ///< 12(int)심고할증 보훈30 요금		
		LOG_WRITE("%30s - (%d) ", "MDNT_HSPD_EXCH_VTRN5_FEE", *(int *) PST_FIELD(pFee, mdnt_hspd_exch_vtrn5_fee	));		 ///< (int)심고할증 보훈50 요금		
		LOG_WRITE("%30s - (%d) ", "MDNT_HSPD_EXCH_VTRN7_FEE", *(int *) PST_FIELD(pFee, mdnt_hspd_exch_vtrn7_fee	));		 ///< (int)심고할증 보훈70 요금			
		LOG_WRITE("%30s - (%d) ", "MDNT_HSPD_EXCH_ARMY2_FEE", *(int *) PST_FIELD(pFee, mdnt_hspd_exch_army2_fee	));		 ///< (int)심고할증 군인20 요금			

		LOG_WRITE("%30s - (%d) ", "PSRM_ADLT_FEE"			, *(int *) PST_FIELD(pFee, psrm_adlt_fee			));		 ///< 13(int)프리미엄 어른 요금 		
		LOG_WRITE("%30s - (%d) ", "PSRM_TEEN_FEE"			, *(int *) PST_FIELD(pFee, psrm_teen_fee			));		 ///< (int)프리미엄 청소년 요금		
		LOG_WRITE("%30s - (%d) ", "PSRM_CHLD_FEE"			, *(int *) PST_FIELD(pFee, psrm_chld_fee			));		 ///< (int)프리미엄 어린이 요금		
		LOG_WRITE("%30s - (%d) ", "PSRM_UVSD_FEE"			, *(int *) PST_FIELD(pFee, psrm_uvsd_fee			));		 ///< (int)프리미엄 대학생 요금		
		
		LOG_WRITE("%30s - (%d) ", "PSRM_VTRN3_FEE"			, *(int *) PST_FIELD(pFee, psrm_vtrn3_fee			));		 ///< 14(int)프리미엄 보훈30 요금 		
		LOG_WRITE("%30s - (%d) ", "PSRM_VTRN5_FEE"			, *(int *) PST_FIELD(pFee, psrm_vtrn5_fee			));		 ///< (int)프리미엄 보훈50 요금		
		LOG_WRITE("%30s - (%d) ", "PSRM_VTRN7_FEE"			, *(int *) PST_FIELD(pFee, psrm_vtrn7_fee			));		 ///< (int)프리미엄 보훈70 요금		
		LOG_WRITE("%30s - (%d) ", "PSRM_ARMY2_FEE"			, *(int *) PST_FIELD(pFee, psrm_army2_fee			));		 ///< (int)프리미엄 군인20 요금		

		LOG_WRITE("%30s - (%d) ", "MDNT_PSRM_ADLT_FEE"		, *(int *) PST_FIELD(pFee, mdnt_psrm_adlt_fee		));		 ///< 15(int)심야프리미엄 어른 요금 		
		LOG_WRITE("%30s - (%d) ", "MDNT_PSRM_TEEN_FEE"		, *(int *) PST_FIELD(pFee, mdnt_psrm_teen_fee		));		 ///< (int)심야프리미엄 청소년 요금		
		LOG_WRITE("%30s - (%d) ", "MDNT_PSRM_CHLD_FEE"		, *(int *) PST_FIELD(pFee, mdnt_psrm_chld_fee		));		 ///< (int)심야프리미엄 어린이 요금		
		LOG_WRITE("%30s - (%d) ", "MDNT_PSRM_UVSD_FEE"		, *(int *) PST_FIELD(pFee, mdnt_psrm_uvsd_fee		));		 ///< (int)심야프리미엄 대학생 요금		
		
		LOG_WRITE("%30s - (%d) ", "MDNT_PSRM_VTRN3_FEE"		, *(int *) PST_FIELD(pFee, mdnt_psrm_vtrn3_fee		));		 ///< 16(int)심야프리미엄 보훈30 요금 	
		LOG_WRITE("%30s - (%d) ", "MDNT_PSRM_VTRN5_FEE"		, *(int *) PST_FIELD(pFee, mdnt_psrm_vtrn5_fee		));		 ///< (int)심야프리미엄 보훈50 요금		
		LOG_WRITE("%30s - (%d) ", "MDNT_PSRM_VTRN7_FEE"		, *(int *) PST_FIELD(pFee, mdnt_psrm_vtrn7_fee		));		 ///< (int)심야프리미엄 보훈70 요금		
		LOG_WRITE("%30s - (%d) ", "MDNT_PSRM_ARMY2_FEE"		, *(int *) PST_FIELD(pFee, mdnt_psrm_army2_fee		));		 ///< (int)심야프리미엄 군인20 요금		

		LOG_WRITE("%30s - (%d) ", "MDNT_PSRM_EXCH_ADLT_FEE"	, *(int *) PST_FIELD(pFee, mdnt_psrm_exch_adlt_fee	));		 ///< 17(int)심야할증프리미엄 어른 요금 	
		LOG_WRITE("%30s - (%d) ", "MDNT_PSRM_EXCH_TEEN_FEE"	, *(int *) PST_FIELD(pFee, mdnt_psrm_exch_teen_fee	));		 ///< (int)심야할증프리미엄 청소년 요금	
		LOG_WRITE("%30s - (%d) ", "MDNT_PSRM_EXCH_CHLD_FEE"	, *(int *) PST_FIELD(pFee, mdnt_psrm_exch_chld_fee	));		 ///< (int)심야할증프리미엄 어린이 요금	
		LOG_WRITE("%30s - (%d) ", "MDNT_PSRM_EXCH_UVSD_FEE"	, *(int *) PST_FIELD(pFee, mdnt_psrm_exch_uvsd_fee	));		 ///< (int)심야할증프리미엄 대학생 요금	
		
		LOG_WRITE("%30s - (%d) ", "MDNT_PSRM_EXCH_VTRN3_FEE"	, *(int *) PST_FIELD(pFee, mdnt_psrm_exch_vtrn3_fee	));	 ///< 18(int)심야할증프리미엄 보훈30 요금 
		LOG_WRITE("%30s - (%d) ", "MDNT_PSRM_EXCH_VTRN5_FEE"	, *(int *) PST_FIELD(pFee, mdnt_psrm_exch_vtrn5_fee	));	 ///< (int)심야할증프리미엄 보훈50 요금	
		LOG_WRITE("%30s - (%d) ", "MDNT_PSRM_EXCH_VTRN7_FEE"	, *(int *) PST_FIELD(pFee, mdnt_psrm_exch_vtrn7_fee	));	 ///< (int)심야할증프리미엄 보훈70 요금	
		LOG_WRITE("%30s - (%d) ", "MDNT_PSRM_EXCH_ARMY2_FEE"	, *(int *) PST_FIELD(pFee, mdnt_psrm_exch_army2_fee	));	 ///< (int)심야할증프리미엄 군인20 요금	

		LOG_WRITE("%30s - (%d) ", "DC_FEE"					, *(int *) PST_FIELD(pFee, dc_fee					));
		LOG_WRITE("%30s - (%s) ", "FN_YN"					, PST_FIELD(pFee, fn_yn			));

		LOG_WRITE("%30s - (%d) ", "REC_NCNT1", nCount);
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_timinfo_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TRTR_TRML_INCL_YN"	,	PST_FIELD(pList, trtr_trml_incl_yn	));
			LOG_WRITE("%30s - (%s) ", "TEMP_ROT_YN"			,	PST_FIELD(pList, temp_rot_yn      	));
			LOG_WRITE("%30s - (%s) ", "DRTM_MOD_PSB_YN"		,	PST_FIELD(pList, drtm_mod_psb_yn   	));
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO"	,	PST_FIELD(pList, alcn_depr_trml_no 	));
			LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO"	,	PST_FIELD(pList, alcn_arvl_trml_no 	));
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"		,	PST_FIELD(pList, depr_trml_no      	));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"		,	PST_FIELD(pList, arvl_trml_no      	));
			LOG_WRITE("%30s - (%s) ", "DEPR_DT"				,	PST_FIELD(pList, depr_dt           	));
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME"			,	PST_FIELD(pList, depr_time         	));
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_DT"		,	PST_FIELD(pList, alcn_depr_dt    	));
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TIME"		,	PST_FIELD(pList, alcn_depr_time    	));
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD"			,	PST_FIELD(pList, bus_cls_cd        	));
			LOG_WRITE("%30s - (%s) ", "CACM_CD"				,	PST_FIELD(pList, cacm_cd           	));
			LOG_WRITE("%30s - (%d) ", "RMN_SATS_NUM"		,	*(int *) PST_FIELD(pList, rmn_sats_num      	));
			LOG_WRITE("%30s - (%d) ", "TOT_SATS_NUM"		,	*(int *) PST_FIELD(pList, tot_sats_num      	));
			LOG_WRITE("%30s - (%s) ", "CTY_PRMM_DC_YN"		,	PST_FIELD(pList, cty_prmm_dc_yn 	));
			LOG_WRITE("%30s - (%d) ", "DEPR_THRU_SEQ"		,	*(int *) PST_FIELD(pList, depr_thru_seq		));
			LOG_WRITE("%30s - (%d) ", "ARVL_THRU_SEQ"		,	*(int *) PST_FIELD(pList, arvl_thru_seq		));
			LOG_WRITE("%30s - (%s) ", "SATS_MLTP_VAL"		,	PST_FIELD(pList, sats_mltp_val		));
			/// 2020.05.18 필드 추가
			LOG_WRITE("%30s - (%s) ", "ALCN_ROT_NO"			,	PST_FIELD(pList, alcn_rot_no		));
			LOG_WRITE("%30s - (%s) ", "DSPR_SATS_YN"		,	PST_FIELD(pList, dspr_sats_yn		));
		}
	}

	///> data proc
	{
		//nRet = nCount;
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
	return 0;
#endif
}

/**
 * @brief		SVC_CM_ReadSatsFee
 * @details		(CM_SETINFO) 좌석 정보 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_CM_ReadSatsFee(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	char*	pService = "CM_SETINFO";
	pstk_cm_setinfo_t pSPacket;
	prtk_cm_setinfo_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" start !!!!");

	pSPacket = (pstk_cm_setinfo_t) pData;
	pRPacket = (prtk_cm_setinfo_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ", "LNG_CD"			,	PST_FIELD(pSPacket, lng_cd));
		LOG_WRITE("%30s - (%s) ", "USER_NO"			,	PST_FIELD(pSPacket, user_no));

		LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO ",	PST_FIELD(pSPacket, alcn_depr_trml_no ));
		LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO ",	PST_FIELD(pSPacket, alcn_arvl_trml_no ));
		LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_DT      ",	PST_FIELD(pSPacket, alcn_depr_dt      ));
		LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TIME    ",	PST_FIELD(pSPacket, alcn_depr_time    ));
		LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO "		,	PST_FIELD(pSPacket, depr_trml_no ));
		LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO "		,	PST_FIELD(pSPacket, arvl_trml_no ));
		LOG_WRITE("%30s - (%s) ", "DEPR_DT      "		,	PST_FIELD(pSPacket, depr_dt      ));
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME    "		,	PST_FIELD(pSPacket, depr_time    ));
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD"		,	PST_FIELD(pSPacket, bus_cls_cd));
		LOG_WRITE("%30s - (%s) ", "CACM_CD   "		,	PST_FIELD(pSPacket, cacm_cd   ));
		LOG_WRITE("%30s - (%s) ", "TRML_NO   "		,	PST_FIELD(pSPacket, trml_no   ));
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD				,	pSPacket->lng_cd			);
		nRet = TMaxFBPut(USER_NO			,	pSPacket->user_no			);

		nRet = TMaxFBPut(ALCN_DEPR_TRML_NO	,	pSPacket->alcn_depr_trml_no );
		nRet = TMaxFBPut(ALCN_ARVL_TRML_NO 	,	pSPacket->alcn_arvl_trml_no );
		nRet = TMaxFBPut(ALCN_DEPR_DT      	,	pSPacket->alcn_depr_dt      );
		nRet = TMaxFBPut(ALCN_DEPR_TIME    	,	pSPacket->alcn_depr_time    );

		nRet = TMaxFBPut(DEPR_TRML_NO		,	pSPacket->depr_trml_no 	 	);
		nRet = TMaxFBPut(ARVL_TRML_NO 		,	pSPacket->arvl_trml_no 	 	);
		nRet = TMaxFBPut(DEPR_DT      		,	pSPacket->depr_dt      	 	);
		nRet = TMaxFBPut(DEPR_TIME    		,	pSPacket->depr_time    	 	);

		nRet = TMaxFBPut(BUS_CLS_CD   		,	pSPacket->bus_cls_cd   	 	);
		nRet = TMaxFBPut(CACM_CD   			,	pSPacket->cacm_cd   	 	);
		nRet = TMaxFBPut(TRML_NO   			,	pSPacket->trml_no   	 	);
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
		// set zero memory
		CPubTckKobusMem::GetInstance()->m_vtResSats.clear();

		/// msg_cd
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		/// list count
		nRet = TMaxFBGet(REC_NCNT1, PST_FIELD(pRPacket, rec_ncnt1));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_ncnt1", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_cm_setinfo_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_cm_setinfo_list_t) * nCount);
		}

		for(i = 0; i < nCount; i++)
		{
			prtk_cm_setinfo_list_t pList;

			pList = &pRPacket->pList[i];

			// 좌석상태1+발권채널1+출발지3
			nRet = TMaxFBGetF(SET_STA	,	PST_FIELD(pList, set_sta	));

			// set memory
			CPubTckKobusMem::GetInstance()->m_vtResSats.push_back(*pList);
		}
	}

	// recv log
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD", PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT", PST_FIELD(pRPacket, msg_dtl_ctt));
		LOG_WRITE("%30s - (%d) ", "REC_NCNT1", nCount);
		for(i = 0; i < nCount; i++)
		{
			prtk_cm_setinfo_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			// 좌석상태1+발권채널1+출발지3
			LOG_WRITE("%30s - (%s) ", "SET_STA"	,	PST_FIELD(pList, set_sta	));
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
 * @brief		SVC_TK_PcpySatsCancel
 * @details		(TW_SATSPCPYCANC) 좌석 선점 취소
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_TK_PcpySatsCancel(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	char*	pService = "TW_SATSPCPYCANC";
	pstk_tw_satspcpycanc_t pSPacket;
	prtk_tw_satspcpycanc_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" start !!!!");

	pSPacket = (pstk_tw_satspcpycanc_t) pData;
	pRPacket = (prtk_tw_satspcpycanc_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ", "LNG_CD"		,		pSPacket->lng_cd		);
		LOG_WRITE("%30s - (%s) ", "USER_NO"		,		pSPacket->user_no		);

		LOG_WRITE("%30s - (%s) ", "TISSU_STA_CD",		pSPacket->tissu_sta_cd	);
		::CopyMemory(&nCount, pSPacket->rec_ncnt1, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NCNT1"		,		nCount	);
		for(i = 0; i < nCount; i++)
		{
			LOG_WRITE("Index - (%02d) ", i	);
			LOG_WRITE("%30s - (%s) ", "PCPY_NO"	,	pSPacket->List[i].pcpy_no 	);
		}
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD			,	pSPacket->lng_cd		);
		nRet = TMaxFBPut(USER_NO		,	pSPacket->user_no		);

		nRet = TMaxFBPut(TISSU_STA_CD	,	pSPacket->tissu_sta_cd	);
		nRet = TMaxFBPut(REC_NCNT1   	,	pSPacket->rec_ncnt1   	);
		::CopyMemory(&nCount, pSPacket->rec_ncnt1, 4);
		for(i = 0; i < nCount; i++)
		{
			nRet = TMaxFBPut(PCPY_NO  ,	pSPacket->List[i].pcpy_no  );
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
		/// msg_cd
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}
	}

	// recv log
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD", PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT", PST_FIELD(pRPacket, msg_dtl_ctt));
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
 * @brief		SVC_TK_PcpySats
 * @details		(TW_SATSPCPY) 좌석 선점
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_TK_PcpySats(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	char*	pService = "TW_SATSPCPY";
	pstk_tw_satspcpy_t pSPacket;
	prtk_tw_satspcpy_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" start !!!!");

	pSPacket = (pstk_tw_satspcpy_t) pData;
	pRPacket = (prtk_tw_satspcpy_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ", "LNG_CD"		,		pSPacket->lng_cd		);
		LOG_WRITE("%30s - (%s) ", "USER_NO"		,		pSPacket->user_no		);

		LOG_WRITE("%30s - (%s) ", "TISSU_CHNL_DVS_CD"	,	pSPacket->tissu_chnl_dvs_cd	  );
		LOG_WRITE("%30s - (%s) ", "TRML_NO"				,	pSPacket->trml_no			  );
		LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO"	,	pSPacket->alcn_depr_trml_no	  );
		LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO"	,	pSPacket->alcn_arvl_trml_no	  );
		LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"		,	pSPacket->depr_trml_no		  );
		LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"		,	pSPacket->arvl_trml_no		  );
		LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_DT"		,	pSPacket->alcn_depr_dt		  );
		LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TIME"		,	pSPacket->alcn_depr_time	  );
		LOG_WRITE("%30s - (%s) ", "DEPR_DT"				,	pSPacket->depr_dt			  );
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME"			,	pSPacket->depr_time			  );
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD"			,	pSPacket->bus_cls_cd		  );
		LOG_WRITE("%30s - (%s) ", "CACM_CD"				,	pSPacket->cacm_cd			  );
		LOG_WRITE("%30s - (%d) ", "DEPR_THRU_SEQ"		,	*(int *) pSPacket->depr_thru_seq		  );
		LOG_WRITE("%30s - (%d) ", "ARVL_THRU_SEQ"		,	*(int *) pSPacket->arvl_thru_seq		  );
		LOG_WRITE("%30s - (%d) ", "TOT_SATS_NUM"		,	*(int *) pSPacket->tot_sats_num		  );

		::CopyMemory(&nCount, pSPacket->rec_ncnt, 4);
		LOG_WRITE("%30s - (%d) ", "REC_NCNT",	nCount );
		for( i = 0; i < nCount; i++ )
		{
			LOG_WRITE("%30s - (%s) ", "SATS_NO",	pSPacket->List[i].sats_no );
		}
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD				,	pSPacket->lng_cd			);
		nRet = TMaxFBPut(USER_NO			,	pSPacket->user_no			);

		nRet = TMaxFBPut(TISSU_CHNL_DVS_CD	,	pSPacket->tissu_chnl_dvs_cd	 );
		nRet = TMaxFBPut(TRML_NO			,	pSPacket->trml_no			 );
		nRet = TMaxFBPut(ALCN_DEPR_TRML_NO	,	pSPacket->alcn_depr_trml_no	 );
		nRet = TMaxFBPut(ALCN_ARVL_TRML_NO	,	pSPacket->alcn_arvl_trml_no	 );
		nRet = TMaxFBPut(DEPR_TRML_NO		,	pSPacket->depr_trml_no		 );
		nRet = TMaxFBPut(ARVL_TRML_NO		,	pSPacket->arvl_trml_no		 );
		nRet = TMaxFBPut(ALCN_DEPR_DT		,	pSPacket->alcn_depr_dt		 );
		nRet = TMaxFBPut(ALCN_DEPR_TIME		,	pSPacket->alcn_depr_time	 );
		nRet = TMaxFBPut(DEPR_DT			,	pSPacket->depr_dt			 );
		nRet = TMaxFBPut(DEPR_TIME			,	pSPacket->depr_time			 );
		nRet = TMaxFBPut(BUS_CLS_CD			,	pSPacket->bus_cls_cd		 );
		nRet = TMaxFBPut(CACM_CD			,	pSPacket->cacm_cd			 );
		nRet = TMaxFBPut(DEPR_THRU_SEQ		,	pSPacket->depr_thru_seq		 );
		nRet = TMaxFBPut(ARVL_THRU_SEQ		,	pSPacket->arvl_thru_seq		 );
		nRet = TMaxFBPut(TOT_SATS_NUM		,	pSPacket->tot_sats_num		 );

		nRet = TMaxFBPut(REC_NCNT1			,	pSPacket->rec_ncnt );
		::CopyMemory(&nCount, pSPacket->rec_ncnt, 4);
		for( i = 0; i < nCount; i++ )
		{
			//nRet = TMaxFBPut(SATS_NO	,	pSPacket->List[i].sats_no );
			nRet = TMaxFBInsert(SATS_NO, pSPacket->List[i].sats_no, i, sizeof(pSPacket->List[i].sats_no) - 1);
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
		// set zero memory
		CPubTckKobusMem::GetInstance()->m_tResSatsPcpy.clear();

		/// msg_cd
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		/// list count
		nRet = TMaxFBGet(REC_NCNT1, PST_FIELD(pRPacket, rec_ncnt1));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_ncnt1", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_tw_satspcpy_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tw_satspcpy_list_t) * nCount);
		}

		for(i = 0; i < nCount; i++)
		{
			prtk_tw_satspcpy_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(SATS_NO	,	PST_FIELD(pList, sats_no	));
			nRet = TMaxFBGetF(PCPY_NO	,	PST_FIELD(pList, pcpy_no	));

			// set memory
			CPubTckKobusMem::GetInstance()->m_tResSatsPcpy.push_back(*pList);
		}
	}

	// recv log
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD", PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT", PST_FIELD(pRPacket, msg_dtl_ctt));
		LOG_WRITE("%30s - (%d) ", "REC_NCNT1", nCount);
		for(i = 0; i < nCount; i++)
		{
			prtk_tw_satspcpy_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "SATS_NO"	,	PST_FIELD(pList, sats_no	));
			LOG_WRITE("%30s - (%s) ", "PCPY_NO"	,	PST_FIELD(pList, pcpy_no	));
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
 * @brief		SVC_TK_PubTck
 * @details		(TM_TCKTRAN) 현장발권
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_TK_PubTck(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	char*	pService = "TM_TCKTRAN";
	pstk_tm_tcktran_t pSPacket;
	prtk_tm_tcktran_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" start !!!!");

	pSPacket = (pstk_tm_tcktran_t) pData;
	pRPacket = (prtk_tm_tcktran_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ", "LNG_CD"		,		pSPacket->lng_cd		);
		LOG_WRITE("%30s - (%s) ", "USER_NO"		,		pSPacket->user_no		);

		LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO",	pSPacket->alcn_depr_trml_no		);
		LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO",	pSPacket->alcn_arvl_trml_no		);
		LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_DT"	 ,	pSPacket->alcn_depr_dt			);
		LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TIME"	 ,	pSPacket->alcn_depr_time		);
		LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"	 ,	pSPacket->depr_trml_no			);
		LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"	 ,	pSPacket->arvl_trml_no			);
		LOG_WRITE("%30s - (%s) ", "DEPR_DT"			 ,	pSPacket->depr_dt				);
		LOG_WRITE("%30s - (%s) ", "DEPR_TIME"		 ,	pSPacket->depr_time				);
		LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD"		 ,	pSPacket->bus_cls_cd			);
		LOG_WRITE("%30s - (%s) ", "CACM_CD"			 ,	pSPacket->cacm_cd				);
		LOG_WRITE("%30s - (%s) ", "TRML_NO"			 ,	pSPacket->trml_no				);
		LOG_WRITE("%30s - (%s) ", "WND_NO"			 ,	pSPacket->wnd_no				);
		LOG_WRITE("%30s - (%s) ", "INHR_NO"			 ,	pSPacket->inhr_no				);
		LOG_WRITE("%30s - (%s) ", "SATS_NO_AUT_YN"	 ,	pSPacket->sats_no_aut_yn		);
		LOG_WRITE("%30s - (%s) ", "INP_DVS_CD"		,	pSPacket->inp_dvs_cd		);
		LOG_WRITE("%30s - (%s) ", "CHIT_USE_DVS"	,	pSPacket->chit_use_dvs		);
		LOG_WRITE("%30s - (%s) ", "TRCK_DTA_ENC_CD"	,	pSPacket->trck_dta_enc_cd	);
		LOG_WRITE("%30s - (%s) ", "ICCD_YN"			,	pSPacket->iccd_yn			);

#if (_KTC_CERTIFY_ <= 0)
 		LOG_WRITE("%30s - (%s) ", "ICCD_INF"		,	pSPacket->iccd_inf			);
 		LOG_WRITE("%30s - (%s) ", "CARD_NO"			,	pSPacket->card_no			);
#endif

		LOG_WRITE("%30s - (%s) ", "MIP_MM_NUM"		,	pSPacket->mip_mm_num		);
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ", "SGN_INF"			,	pSPacket->sgn_inf			);
#endif

		::CopyMemory(&nCount, pSPacket->tissu_hcnt, 4);
		LOG_WRITE("%30s - (%d) ", "TISSU_HCNT"	,	nCount	);
		for(i = 0; i < nCount; i++)
		{
			LOG_WRITE("Index = (%d) ", i	);
			LOG_WRITE("%30s - (%s) ", "SATS_NO"		,	pSPacket->List[i].sats_no		);
			LOG_WRITE("%30s - (%s) ", "PCPY_NO"		,	pSPacket->List[i].pcpy_no		);
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"	,	pSPacket->List[i].tck_knd_cd	);
		}
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD				,	pSPacket->lng_cd			);
		nRet = TMaxFBPut(USER_NO			,	pSPacket->user_no			);

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
		nRet = TMaxFBPut(TRML_NO			,	pSPacket->trml_no			);
		nRet = TMaxFBPut(WND_NO				,	pSPacket->wnd_no			);
		nRet = TMaxFBPut(INHR_NO			,	pSPacket->inhr_no			);
		nRet = TMaxFBPut(SATS_NO_AUT_YN		,	pSPacket->sats_no_aut_yn	);

		nRet = TMaxFBPut(INP_DVS_CD			,	pSPacket->inp_dvs_cd		);
		nRet = TMaxFBPut(CHIT_USE_DVS		,	pSPacket->chit_use_dvs		);
		nRet = TMaxFBPut(TRCK_DTA_ENC_CD	,	pSPacket->trck_dta_enc_cd	);
		nRet = TMaxFBPut(ICCD_YN			,	pSPacket->iccd_yn			);
		nRet = TMaxFBPut(ICCD_INF			,	pSPacket->iccd_inf			);
		nRet = TMaxFBPut(CARD_NO			,	pSPacket->card_no			);
		nRet = TMaxFBPut(MIP_MM_NUM			,	pSPacket->mip_mm_num		);
		nRet = TMaxFBPut(SGN_INF			,	pSPacket->sgn_inf			);

		nRet = TMaxFBPut(TISSU_HCNT,	pSPacket->tissu_hcnt	);
		::CopyMemory(&nCount, pSPacket->tissu_hcnt, 4);
		for(i = 0; i < nCount; i++)
		{
			nRet = TMaxFBInsert(SATS_NO		, pSPacket->List[i].sats_no		, i, sizeof(pSPacket->List[i].sats_no));
			nRet = TMaxFBInsert(PCPY_NO		, pSPacket->List[i].pcpy_no		, i, sizeof(pSPacket->List[i].pcpy_no));
			nRet = TMaxFBInsert(TCK_KND_CD	, pSPacket->List[i].tck_knd_cd	, i, sizeof(pSPacket->List[i].tck_knd_cd));
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(pSPacket->iccd_inf	, sizeof(pSPacket->iccd_inf));
		KTC_MemClear(pSPacket->card_no	, sizeof(pSPacket->card_no));
#endif
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

	//nRet = -1;
	//LOG_WRITE("TMaxAlloc(), test#3 err, %d !!!!", 1);
	//goto fail_proc;

	// recv data
	{
		// set zero memory
		::ZeroMemory(&CPubTckKobusMem::GetInstance()->m_tResTckIssueInfo, sizeof(rtk_tm_tcktran_info_t));
		CPubTckKobusMem::GetInstance()->m_tResTckIssueList.clear();

		/// msg_cd
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		nRet = TMaxFBGet(TAK_STT_DT		,	ST_FIELD(pRPacket->info, tak_stt_dt	));
		nRet = TMaxFBGet(STT_TIME		,	ST_FIELD(pRPacket->info, stt_time		));
		nRet = TMaxFBGet(CARD_APRV_NO	,	ST_FIELD(pRPacket->info, card_aprv_no	));
		nRet = TMaxFBGet(APRV_AMT		,	ST_FIELD(pRPacket->info, aprv_amt		));
		nRet = TMaxFBGet(ROT_RDHM_NO_VAL,	ST_FIELD(pRPacket->info, rot_rdhm_no_val));

		nRet = TMaxFBGet(RMN_PNT		,	ST_FIELD(pRPacket->info, rmn_pnt));
		nRet = TMaxFBGet(CSRC_APRV_NO	,	ST_FIELD(pRPacket->info, csrc_aprv_no));
		nRet = TMaxFBGet(USER_KEY_VAL	,	ST_FIELD(pRPacket->info, user_key_val));

		//nRet = TMaxFBGet(USER_KEY_VAL	,	ST_FIELD(pRPacket->info, user_key_val));
		
		// 2020.03.26 코드 추가
		nRet = TMaxFBGet(CHIT_USE_DVS	,	ST_FIELD(pRPacket->info, chit_use_dvs));
		
		// set memory
		::CopyMemory(&CPubTckKobusMem::GetInstance()->m_tResTckIssueInfo, &pRPacket->info, sizeof(rtk_tm_tcktran_info_t));
		
		AddAccumKobusTicketInfo(pRPacket->info.tak_stt_dt, pRPacket->info.stt_time);
		//::CopyMemory(CConfigTkMem::GetInstance()->kobus_tak_stt_dt, pRPacket->info.tak_stt_dt, sizeof(CConfigTkMem::GetInstance()->kobus_tak_stt_dt));
		//::CopyMemory(CConfigTkMem::GetInstance()->kobus_stt_time, pRPacket->info.stt_time, sizeof(CConfigTkMem::GetInstance()->kobus_stt_time));

		/// list count
		nRet = TMaxFBGet(REC_NCNT1, PST_FIELD(pRPacket, rec_ncnt1));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_ncnt1", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_tm_tcktran_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_tcktran_list_t) * nCount);
		}

		// set memory
		{
			SYSTEMTIME st;

			::GetLocalTime(&st);
//			sprintf(CPubTckKobusMem::GetInstance()->base.pub_time, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
			sprintf(CPubTckKobusMem::GetInstance()->base.pub_time, "(%02d:%02d)", st.wHour, st.wMinute);
		}

		for(i = 0; i < nCount; i++)
		{
			prtk_tm_tcktran_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(SATS_NO			,	PST_FIELD(pList, sats_no			));
			nRet = TMaxFBGetF(TCK_KND_CD		,	PST_FIELD(pList, tck_knd_cd			));
			nRet = TMaxFBGetF(INVS_NO			,	PST_FIELD(pList, invs_no			));
			nRet = TMaxFBGetF(TISSU_DT			,	PST_FIELD(pList, tissu_dt			));
			nRet = TMaxFBGetF(TISSU_TRML_NO		,	PST_FIELD(pList, tissu_trml_no		));
			nRet = TMaxFBGetF(TISSU_WND_NO		,	PST_FIELD(pList, tissu_wnd_no		));
			nRet = TMaxFBGetF(TISSU_SNO			,	PST_FIELD(pList, tissu_sno			));
			nRet = TMaxFBGetF(TISSU_FEE			,	PST_FIELD(pList, tissu_fee			));

			nRet = TMaxFBGetF(DC_YN				,	PST_FIELD(pList, dc_yn				));
			nRet = TMaxFBGetF(DC_FEE			,	PST_FIELD(pList, dc_fee				));
			
//			nRet = TMaxFBGetF(DEPR_HNGL_NM 		,	PST_FIELD(pList, depr_hngl_nm 		));
//			nRet = TMaxFBGetF(DEPR_TRML_ABRV_NM	,	PST_FIELD(pList, depr_trml_abrv_nm	));
//			nRet = TMaxFBGetF(DEPR_ENG_NM		,	PST_FIELD(pList, depr_eng_nm		));
//			nRet = TMaxFBGetF(ARVL_HNGL_NM		,	PST_FIELD(pList, arvl_hngl_nm		));
//			nRet = TMaxFBGetF(ARVL_TRML_ABRV_NM	,	PST_FIELD(pList, arvl_trml_abrv_nm	));
//			nRet = TMaxFBGetF(ARVL_TRML_ENG_NM	,	PST_FIELD(pList, arvl_trml_eng_nm	));

			TMaxGetConvChar(DEPR_HNGL_NM 		,	PST_FIELD(pList, depr_hngl_nm 		));
			TMaxGetConvChar(DEPR_TRML_ABRV_NM	,	PST_FIELD(pList, depr_trml_abrv_nm	));
			TMaxGetConvChar(DEPR_ENG_NM			,	PST_FIELD(pList, depr_eng_nm		));
			TMaxGetConvChar(ARVL_HNGL_NM		,	PST_FIELD(pList, arvl_hngl_nm		));
			TMaxGetConvChar(ARVL_TRML_ABRV_NM	,	PST_FIELD(pList, arvl_trml_abrv_nm	));
			TMaxGetConvChar(ARVL_TRML_ENG_NM	,	PST_FIELD(pList, arvl_trml_eng_nm	));

			nRet = TMaxFBGetF(BUS_OPRN_DIST		,	PST_FIELD(pList, bus_oprn_dist		));
			nRet = TMaxFBGetF(TRML_BIZR_NO		,	PST_FIELD(pList, trml_bizr_no		));
			nRet = TMaxFBGetF(TRTR_ROT_EXSN_YN	,	PST_FIELD(pList, trtr_rot_exsn_yn	));
			nRet = TMaxFBGetF(TRTR_TRML_YN		,	PST_FIELD(pList, trtr_trml_yn		));

//			nRet = TMaxFBGetF(TDEPR_TRML_NM		,	PST_FIELD(pList, tdepr_trml_nm		));
//			nRet = TMaxFBGetF(TARVL_TRML_NM		,	PST_FIELD(pList, tarvl_trml_nm		));
			TMaxGetConvChar(TDEPR_TRML_NM		,	PST_FIELD(pList, tdepr_trml_nm		));
			TMaxGetConvChar(TARVL_TRML_NM		,	PST_FIELD(pList, tarvl_trml_nm		));

			// set memory
			CPubTckKobusMem::GetInstance()->m_tResTckIssueList.push_back(*pList);
		}
	}

	// recv log
	{
		prtk_tm_tcktran_info_t pInfo;

		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD"			, PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT"		, PST_FIELD(pRPacket, msg_dtl_ctt));

		pInfo = &pRPacket->info;

		LOG_WRITE("%30s - (%s) ", "TAK_STT_DT"		, PST_FIELD(pInfo, tak_stt_dt		));
		LOG_WRITE("%30s - (%s) ", "STT_TIME"		, PST_FIELD(pInfo, stt_time			));
		LOG_WRITE("%30s - (%s) ", "CARD_APRV_NO"	, PST_FIELD(pInfo, card_aprv_no		));
		LOG_WRITE("%30s - (%d) ", "APRV_AMT"		, *(int *) pInfo->aprv_amt			);
		LOG_WRITE("%30s - (%s) ", "ROT_RDHM_NO_VAL" , PST_FIELD(pInfo, rot_rdhm_no_val	));

		LOG_WRITE("%30s - (Num=%d) ", "RMN_PNT"		, *(int *)pInfo->rmn_pnt			 );
		LOG_WRITE("%30s - (%s) ", "CSRC_APRV_NO"	, PST_FIELD(pInfo, csrc_aprv_no		));
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ", "USER_KEY_VAL"	, PST_FIELD(pInfo, user_key_val		));
#endif
		// 2020.03.26 추가
		LOG_WRITE("%30s - (%s) ", "CHIT_USE_DVS"	, PST_FIELD(pInfo, chit_use_dvs		));

		LOG_WRITE("%30s - (%d) ", "REC_NCNT1", nCount);
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_tcktran_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "SATS_NO"			,	PST_FIELD(pList, sats_no			));
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"		,	PST_FIELD(pList, tck_knd_cd			));
			LOG_WRITE("%30s - (%s) ", "INVS_NO"			,	PST_FIELD(pList, invs_no			));
			LOG_WRITE("%30s - (%s) ", "TISSU_DT"		,	PST_FIELD(pList, tissu_dt			));
			LOG_WRITE("%30s - (%s) ", "TISSU_TRML_NO"	,	PST_FIELD(pList, tissu_trml_no		));
			LOG_WRITE("%30s - (%s) ", "TISSU_WND_NO"	,	PST_FIELD(pList, tissu_wnd_no		));
			LOG_WRITE("%30s - (%s) ", "TISSU_SNO"		,	PST_FIELD(pList, tissu_sno			));
			LOG_WRITE("%30s - (%d) ", "TISSU_FEE"		,	*(int *) pList->tissu_fee			);
			LOG_WRITE("%30s - (%s) ", "DC_YN"			,	PST_FIELD(pList, dc_yn				));
			LOG_WRITE("%30s - (%d) ", "DC_FEE"			,	*(int *)pList->dc_fee				);
			LOG_WRITE("%30s - (%s) ", "DEPR_HNGL_NM"	,	PST_FIELD(pList, depr_hngl_nm 		));
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_ABRV_NM",	PST_FIELD(pList, depr_trml_abrv_nm	));
			LOG_WRITE("%30s - (%s) ", "DEPR_ENG_NM"		,	PST_FIELD(pList, depr_eng_nm		));
			LOG_WRITE("%30s - (%s) ", "ARVL_HNGL_NM	"	,	PST_FIELD(pList, arvl_hngl_nm		));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_ABRV_NM",	PST_FIELD(pList, arvl_trml_abrv_nm	));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_ENG_NM",	PST_FIELD(pList, arvl_trml_eng_nm	));
			LOG_WRITE("%30s - (%s) ", "BUS_OPRN_DIST"	,	PST_FIELD(pList, bus_oprn_dist		));
			LOG_WRITE("%30s - (%s) ", "TRML_BIZR_NO"	,	PST_FIELD(pList, trml_bizr_no		));
			LOG_WRITE("%30s - (%s) ", "TRTR_ROT_EXSN_YN",	PST_FIELD(pList, trtr_rot_exsn_yn	));
			LOG_WRITE("%30s - (%s) ", "TRTR_TRML_YN"	,	PST_FIELD(pList, trtr_trml_yn		));
			LOG_WRITE("%30s - (%s) ", "TDEPR_TRML_NM"	,	PST_FIELD(pList, tdepr_trml_nm		));
			LOG_WRITE("%30s - (%s) ", "TARVL_TRML_NM"	,	PST_FIELD(pList, tarvl_trml_nm		));
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
 * @brief		SVC_TK_ReadMrs
 * @details		(TM_MRSINFO) 인터넷 예매조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_TK_ReadMrs(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	char*	pService = "TM_MRSINFO";
	pstk_tm_mrsinfo_t pSPacket;
	prtk_tm_mrsinfo_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" start !!!!");

	pSPacket = (pstk_tm_mrsinfo_t) pData;
	pRPacket = (prtk_tm_mrsinfo_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ", "LNG_CD"		,		pSPacket->lng_cd		);
		LOG_WRITE("%30s - (%s) ", "USER_NO"		,		pSPacket->user_no		);

		LOG_WRITE("%30s - (%s) ", "TISSU_DVS_CD",		pSPacket->tissu_dvs_cd	);
		LOG_WRITE("%30s - (%s) ", "DEPR_DT"		,		pSPacket->depr_dt		);
		LOG_WRITE("%30s - (%s) ", "PYN_DVS_CD"	,		pSPacket->pyn_dvs_cd	);
		LOG_WRITE("%30s - (%s) ", "MRS_MRNP_NO"	,		pSPacket->mrs_mrnp_no	);
		LOG_WRITE("%30s - (%s) ", "MRSP_MBPH_NO",		pSPacket->mrsp_mbph_no	);
		LOG_WRITE("%30s - (%s) ", "MRSP_BRDT"	,		pSPacket->mrsp_brdt		);
#if (_KTC_CERTIFY_ <= 0)
		LOG_WRITE("%30s - (%s) ", "CARD_NO"		,		pSPacket->card_no		);
#endif
		LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO",		pSPacket->depr_trml_no	);
		LOG_WRITE("%30s - (%s) ", "ADTN_CPN_NO",		pSPacket->adtn_cpn_no	);
		LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_DT",		pSPacket->alcn_depr_dt	);
		
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD			,	pSPacket->lng_cd		);
		nRet = TMaxFBPut(USER_NO		,	pSPacket->user_no		);

		nRet = TMaxFBPut(TISSU_DVS_CD	,	pSPacket->tissu_dvs_cd	);
		nRet = TMaxFBPut(DEPR_DT		,	pSPacket->depr_dt		);
		nRet = TMaxFBPut(PYN_DVS_CD		,	pSPacket->pyn_dvs_cd	);
		nRet = TMaxFBPut(MRS_MRNP_NO	,	pSPacket->mrs_mrnp_no	);
		nRet = TMaxFBPut(MRSP_MBPH_NO	,	pSPacket->mrsp_mbph_no	);
		nRet = TMaxFBPut(MRSP_BRDT		,	pSPacket->mrsp_brdt		);
		nRet = TMaxFBPut(CARD_NO		,	pSPacket->card_no		);
		nRet = TMaxFBPut(DEPR_TRML_NO	,	pSPacket->depr_trml_no	);
		nRet = TMaxFBPut(ADTN_CPN_NO	,	pSPacket->adtn_cpn_no	);
		nRet = TMaxFBPut(ALCN_DEPR_DT	,	pSPacket->alcn_depr_dt	);
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
		/// msg_cd
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		/// list count
		nRet = TMaxFBGet(REC_NCNT1, PST_FIELD(pRPacket, rec_ncnt1));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_ncnt1", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE888");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_tm_mrsinfo_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_mrsinfo_list_t) * nCount);
		}

		// set zero memory
		CMrnpKobusMem::GetInstance()->m_vtMrnpList.clear();

		for(i = 0; i < nCount; i++)
		{
			prtk_tm_mrsinfo_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(MRS_MRNP_NO		,	PST_FIELD(pList, mrs_mrnp_no		));
			nRet = TMaxFBGetF(MRS_MRNP_SNO		,	PST_FIELD(pList, mrs_mrnp_sno		));
			nRet = TMaxFBGetF(MRS_MRNP_DT		,	PST_FIELD(pList, mrs_mrnp_dt		));
			nRet = TMaxFBGetF(MRS_MRNP_TIME		,	PST_FIELD(pList, mrs_mrnp_time		));
			nRet = TMaxFBGetF(MBRS_NO			,	PST_FIELD(pList, mbrs_no			));
			nRet = TMaxFBGetF(DEPR_TRML_NO		,	PST_FIELD(pList, depr_trml_no		));
			nRet = TMaxFBGetF(ARVL_TRML_NO		,	PST_FIELD(pList, arvl_trml_no		));
			nRet = TMaxFBGetF(ALCN_DEPR_TRML_NO	,	PST_FIELD(pList, alcn_depr_trml_no	));
			nRet = TMaxFBGetF(ALCN_ARVL_TRML_NO	,	PST_FIELD(pList, alcn_arvl_trml_no	));
			nRet = TMaxFBGetF(DEPR_DT			,	PST_FIELD(pList, depr_dt			));
			nRet = TMaxFBGetF(DEPR_TIME			,	PST_FIELD(pList, depr_time			));
			nRet = TMaxFBGetF(CACM_CD			,	PST_FIELD(pList, cacm_cd			));
			nRet = TMaxFBGetF(BUS_CLS_CD		,	PST_FIELD(pList, bus_cls_cd			));
			nRet = TMaxFBGetF(MRS_CHNL_DVS_CD	,	PST_FIELD(pList, mrs_chnl_dvs_cd	));
			nRet = TMaxFBGetF(PUB_CHNL_DVS_CD	,	PST_FIELD(pList, pub_chnl_dvs_cd	));
			nRet = TMaxFBGetF(PYN_DTL_CD		,	PST_FIELD(pList, pyn_dtl_cd			));
			nRet = TMaxFBGetF(ADLT_NUM			,	PST_FIELD(pList, adlt_num			));
			nRet = TMaxFBGetF(ADLT_TOT_AMT		,	PST_FIELD(pList, adlt_tot_amt		));
			nRet = TMaxFBGetF(CHLD_NUM			,	PST_FIELD(pList, chld_num			));
			nRet = TMaxFBGetF(CHLD_TOT_AMT		,	PST_FIELD(pList, chld_tot_amt		));
			nRet = TMaxFBGetF(TEEN_NUM			,	PST_FIELD(pList, teen_num			));
			nRet = TMaxFBGetF(TEEN_TOT_AMT		,	PST_FIELD(pList, teen_tot_amt		));
			nRet = TMaxFBGetF(UVSD_NUM			,	PST_FIELD(pList, uvsd_num			));
			nRet = TMaxFBGetF(UVSD_TOT_AMT		,	PST_FIELD(pList, uvsd_tot_amt		));
			// 20210614 ADD
			nRet = TMaxFBGetF(SNCN_NUM			,	PST_FIELD(pList, sncn_num			));
			nRet = TMaxFBGetF(SNCN_TOT_AMT		,	PST_FIELD(pList, sncn_tot_amt		));
			nRet = TMaxFBGetF(DSPR_NUM			,	PST_FIELD(pList, dspr_num			));
			nRet = TMaxFBGetF(DSPR_TOT_AMT		,	PST_FIELD(pList, dspr_tot_amt		));
			nRet = TMaxFBGetF(VTR3_NUM			,	PST_FIELD(pList, vtr3_num			));
			nRet = TMaxFBGetF(VTR3_TOT_AMT		,	PST_FIELD(pList, vtr3_tot_amt		));
			nRet = TMaxFBGetF(VTR5_NUM			,	PST_FIELD(pList, vtr5_num			));
			nRet = TMaxFBGetF(VTR5_TOT_AMT		,	PST_FIELD(pList, vtr5_tot_amt		));
			nRet = TMaxFBGetF(VTR7_NUM			,	PST_FIELD(pList, vtr7_num			));
			nRet = TMaxFBGetF(VTR7_TOT_AMT		,	PST_FIELD(pList, vtr7_tot_amt		));
			// 20210614 ~ADD
			// 20220713 ADD
			nRet = TMaxFBGetF(DFPT_NUM			,	PST_FIELD(pList, dfpt_num			));
			nRet = TMaxFBGetF(DFPT_TOT_AMT		,	PST_FIELD(pList, dfpt_tot_amt		));
			// 20220713 ~ADD
			nRet = TMaxFBGetF(SATS_NUM			,	PST_FIELD(pList, sats_num			));
			nRet = TMaxFBGetF(RIDE_PSB_FCNT		,	PST_FIELD(pList, ride_psb_fcnt		));
			nRet = TMaxFBGetF(TOT_RIDE_PSB_FCNT	,	PST_FIELD(pList, tot_ride_psb_fcnt	));
			nRet = TMaxFBGetF(RTRP_DVS_CD1		,	PST_FIELD(pList, rtrp_dvs_cd1		));
			TMaxGetConvChar(DEPR_HNGL_NM		,	pList->depr_hngl_nm					);
			TMaxGetConvChar(ARVL_HNGL_NM		,	pList->arvl_hngl_nm					);

			// set memory
			CMrnpKobusMem::GetInstance()->m_vtMrnpList.push_back(*pList);
		}
	}

	// recv log
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD"		, PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT"	, PST_FIELD(pRPacket, msg_dtl_ctt));

		LOG_WRITE("%30s - (%d) ", "REC_NCNT1", nCount);
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_mrsinfo_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "MRS_MRNP_NO"		,	PST_FIELD(pList, mrs_mrnp_no		));
			LOG_WRITE("%30s - (%s) ", "MRS_MRNP_SNO"	,	PST_FIELD(pList, mrs_mrnp_sno		));
			LOG_WRITE("%30s - (%s) ", "MRS_MRNP_DT"		,	PST_FIELD(pList, mrs_mrnp_dt		));
			LOG_WRITE("%30s - (%s) ", "MRS_MRNP_TIME"	,	PST_FIELD(pList, mrs_mrnp_time		));
			LOG_WRITE("%30s - (%s) ", "MBRS_NO"			,	PST_FIELD(pList, mbrs_no			));
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"	,	PST_FIELD(pList, depr_trml_no		));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"	,	PST_FIELD(pList, arvl_trml_no		));
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO",	PST_FIELD(pList, alcn_depr_trml_no	));
			LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO",	PST_FIELD(pList, alcn_arvl_trml_no	));
			LOG_WRITE("%30s - (%s) ", "DEPR_DT"			,	PST_FIELD(pList, depr_dt			));
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME"		,	PST_FIELD(pList, depr_time			));
			LOG_WRITE("%30s - (%s) ", "CACM_CD"			,	PST_FIELD(pList, cacm_cd			));
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD	"	,	PST_FIELD(pList, bus_cls_cd			));
			LOG_WRITE("%30s - (%s) ", "MRS_CHNL_DVS_CD"	,	PST_FIELD(pList, mrs_chnl_dvs_cd	));
			LOG_WRITE("%30s - (%s) ", "PUB_CHNL_DVS_CD"	,	PST_FIELD(pList, pub_chnl_dvs_cd	));
			LOG_WRITE("%30s - (%s) ", "PYN_DTL_CD	"	,	PST_FIELD(pList, pyn_dtl_cd			));
			LOG_WRITE("%30s - (%d) ", "ADLT_NUM"		,	*(int *) pList->adlt_num			);
			LOG_WRITE("%30s - (%d) ", "ADLT_TOT_AMT"	,	*(int *) pList->adlt_tot_amt		);
			LOG_WRITE("%30s - (%d) ", "CHLD_NUM"		,	*(int *) pList->chld_num			);
			LOG_WRITE("%30s - (%d) ", "CHLD_TOT_AMT"	,	*(int *) pList->chld_tot_amt		);
			LOG_WRITE("%30s - (%d) ", "TEEN_NUM"		,	*(int *) pList->teen_num			);
			LOG_WRITE("%30s - (%d) ", "TEEN_TOT_AMT"	,	*(int *) pList->teen_tot_amt		);
			LOG_WRITE("%30s - (%d) ", "UVSD_NUM"		,	*(int *) pList->uvsd_num			);
			LOG_WRITE("%30s - (%d) ", "UVSD_TOT_AMT"	,	*(int *) pList->uvsd_tot_amt		);
			// 20210614 ADD
			LOG_WRITE("%30s - (%d) ", "SNCN_NUM"		,	*(int *) pList->sncn_num			);
			LOG_WRITE("%30s - (%d) ", "SNCN_TOT_AM"		,	*(int *) pList->sncn_tot_amt		);
			LOG_WRITE("%30s - (%d) ", "DSPR_NUM"		,	*(int *) pList->dspr_num			);
			LOG_WRITE("%30s - (%d) ", "DSPR_TOT_AMT"	,	*(int *) pList->dspr_tot_amt		);
			LOG_WRITE("%30s - (%d) ", "VTR3_NUM"		,	*(int *) pList->vtr3_num			);
			LOG_WRITE("%30s - (%d) ", "VTR3_TOT_AMT"	,	*(int *) pList->vtr3_tot_amt		);
			LOG_WRITE("%30s - (%d) ", "VTR5_NUM"		,	*(int *) pList->vtr5_num			);
			LOG_WRITE("%30s - (%d) ", "VTR5_TOT_AMT"	,	*(int *) pList->vtr5_tot_amt		);
			LOG_WRITE("%30s - (%d) ", "VTR7_NUM"		,	*(int *) pList->vtr7_num			);
			LOG_WRITE("%30s - (%d) ", "VTR7_TOT_AMT"	,	*(int *) pList->vtr7_tot_amt		);
			// ~20210614 ADD
			// 20220713 ADD
			LOG_WRITE("%30s - (%d) ", "DFPT_NUM"		,	*(int *) pList->dfpt_num			);
			LOG_WRITE("%30s - (%d) ", "DFPT_TOT_AMT"	,	*(int *) pList->dfpt_tot_amt		);
			// 20220713 ~ADD
			LOG_WRITE("%30s - (%d) ", "SATS_NUM"		,	*(int *) pList->sats_num			);
			LOG_WRITE("%30s - (%d) ", "RIDE_PSB_FCNT"	,	*(int *) pList->ride_psb_fcnt		);
			LOG_WRITE("%30s - (%d) ", "TOT_RIDE_PSB_FCNT",	*(int *) pList->tot_ride_psb_fcnt	);
			LOG_WRITE("%30s - (%s) ", "RTRP_DVS_CD1"	,	PST_FIELD(pList, rtrp_dvs_cd1		));
			LOG_WRITE("%30s - (%s) ", "DEPR_HNGL_NM"	,	PST_FIELD(pList, depr_hngl_nm 		));
			LOG_WRITE("%30s - (%s) ", "ARVL_HNGL_NM"	,	PST_FIELD(pList, arvl_hngl_nm		));
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
 * @brief		SVC_TK_PubMrs
 * @details		(TM_MRSPUB) 예매발행
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_TK_PubMrs(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	char*	pService = "TM_MRSPUB";
	pstk_tm_mrspub_t pSPacket;
	prtk_tm_mrspub_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" start !!!!");

	pSPacket = (pstk_tm_mrspub_t) pData;
	pRPacket = (prtk_tm_mrspub_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ", "LNG_CD"		,		pSPacket->lng_cd		);
		LOG_WRITE("%30s - (%s) ", "USER_NO"		,		pSPacket->user_no		);

		LOG_WRITE("%30s - (%s) ", "TISSU_TRML_NO",		pSPacket->tissu_trml_no		);
		LOG_WRITE("%30s - (%s) ", "WND_NO"		,		pSPacket->wnd_no			);
		LOG_WRITE("%30s - (%s) ", "STT_INHR_NO"	,		pSPacket->stt_inhr_no		);
		LOG_WRITE("%30s - (num=%d) ", "REC_NCNT"	,		*(int *)pSPacket->rec_ncnt			);

		::CopyMemory(&nCount, pSPacket->rec_ncnt, 4);
		for(i = 0; i < nCount; i++)
		{
			LOG_WRITE("Index = %02d ", i);
			LOG_WRITE("%30s - (%s) ", "MRS_MRNP_NO"	,	pSPacket->List[i].mrs_mrnp_no	);
			LOG_WRITE("%30s - (%s) ", "MRS_MRNP_SNO",	pSPacket->List[i].mrs_mrnp_sno	);
		}
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD			,	pSPacket->lng_cd		);
		nRet = TMaxFBPut(USER_NO		,	pSPacket->user_no		);

		nRet = TMaxFBPut(TISSU_TRML_NO	,	pSPacket->tissu_trml_no	);
		nRet = TMaxFBPut(WND_NO			,	pSPacket->wnd_no		);
		nRet = TMaxFBPut(STT_INHR_NO	,	pSPacket->stt_inhr_no	);
		nRet = TMaxFBPut(REC_NCNT		,	pSPacket->rec_ncnt		);
		::CopyMemory(&nCount, pSPacket->rec_ncnt, 4);
		for(i = 0; i < nCount; i++)
		{
			nRet = TMaxFBPut(MRS_MRNP_NO	,	pSPacket->List[i].mrs_mrnp_no	);
			nRet = TMaxFBPut(MRS_MRNP_SNO	,	pSPacket->List[i].mrs_mrnp_sno	);
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
		CMrnpKobusMem* pMrsTR;

		pMrsTR = CMrnpKobusMem::GetInstance();

		/// msg_cd
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		///> 승차홈번호
		//nRet = TMaxFBGet(ROT_RDHM_NO_VAL, PST_FIELD(pRPacket, rot_rdhm_no_val));
		TMaxGetConvChar(ROT_RDHM_NO_VAL, PST_FIELD(pRPacket, rot_rdhm_no_val));

		::CopyMemory(pMrsTR->rot_rdhm_no_val, pRPacket->rot_rdhm_no_val, sizeof(pMrsTR->rot_rdhm_no_val));

		/// list count
		nRet = TMaxFBGet(REC_NCNT1, PST_FIELD(pRPacket, rec_ncnt1));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_ncnt1", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_tm_mrspub_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_mrspub_list_t) * nCount);
		}

		pMrsTR->m_vtResComplete.clear();

		for(i = 0; i < nCount; i++)
		{
			prtk_tm_mrspub_list_t pList;

			pList = &pRPacket->pList[i];

			///> 발권일자
			nRet = TMaxFBGetF(TISSU_DT			,	PST_FIELD(pList, tissu_dt			));
			nRet = TMaxFBGetF(TISSU_TIME		,	PST_FIELD(pList, tissu_time			));
			nRet = TMaxFBGetF(TISSU_TRML_NO		,	PST_FIELD(pList, tissu_trml_no		));
			nRet = TMaxFBGetF(TISSU_WND_NO		,	PST_FIELD(pList, tissu_wnd_no		));
			nRet = TMaxFBGetF(TISSU_SNO			,	PST_FIELD(pList, tissu_sno			));
			nRet = TMaxFBGetF(SATS_NO			,	PST_FIELD(pList, sats_no			));
			nRet = TMaxFBGetF(INVS_NO			,	PST_FIELD(pList, invs_no			));
			nRet = TMaxFBGetF(TCK_KND_CD		,	PST_FIELD(pList, tck_knd_cd			));
			nRet = TMaxFBGetF(PYN_DVS_CD		,	PST_FIELD(pList, pyn_dvs_cd			));
			nRet = TMaxFBGetF(PYN_DTL_CD		,	PST_FIELD(pList, pyn_dtl_cd			));
			nRet = TMaxFBGetF(CARD_NO			,	PST_FIELD(pList, card_no			));
#ifdef __KTC_PROGRAM__
			::ZeroMemory(pList->card_no, sizeof(pList->card_no));
#endif
			nRet = TMaxFBGetF(CARD_APRV_NO		,	PST_FIELD(pList, card_aprv_no		));
			//nRet = TMaxFBGetF(OGN_APRV_AMT		,	PST_FIELD(pList, ogn_aprv_amt		));	// 20211019 DEL
			// 20211019 ADD
			nRet = TMaxFBGetF(OGN_APRV_AMT		,	PST_FIELD(pList, ogn_aprv_amt		));	/// 카드원승인금액_arr(후불카드 OR 가상선불 원승인금액)				
			nRet = TMaxFBGetF(RMN_APRV_AMT		,	PST_FIELD(pList, rmn_aprv_amt		)); /// 카드잔여승인금액_arr(후불카드 OR 가상선불 잔여금액)
			// 20211019 ~ADD
			nRet = TMaxFBGetF(MIP_MM_NUM		,	PST_FIELD(pList, mip_mm_num			));
			nRet = TMaxFBGetF(CSRC_APRV_NO		,	PST_FIELD(pList, csrc_aprv_no		));
			nRet = TMaxFBGetF(USER_KEY_VAL		,	PST_FIELD(pList, user_key_val		));
			nRet = TMaxFBGetF(CASH_RECP_AMT		,	PST_FIELD(pList, cash_recp_amt		));
			nRet = TMaxFBGetF(TISSU_FEE			,	PST_FIELD(pList, tissu_fee			));
			nRet = TMaxFBGetF(STT_TIME			,	PST_FIELD(pList, stt_time			));
			nRet = TMaxFBGetF(ALCN_DEPR_TRML_NO	,	PST_FIELD(pList, alcn_depr_trml_no	));
			nRet = TMaxFBGetF(ALCN_ARVL_TRML_NO	,	PST_FIELD(pList, alcn_arvl_trml_no	));
			nRet = TMaxFBGetF(ALCN_DEPR_DT		,	PST_FIELD(pList, alcn_depr_dt		));
			nRet = TMaxFBGetF(ALCN_DEPR_TIME	,	PST_FIELD(pList, alcn_depr_time		));
			nRet = TMaxFBGetF(DEPR_TRML_NO		,	PST_FIELD(pList, depr_trml_no		));
			nRet = TMaxFBGetF(ARVL_TRML_NO		,	PST_FIELD(pList, arvl_trml_no		));
			nRet = TMaxFBGetF(DEPR_DT			,	PST_FIELD(pList, depr_dt			));
			nRet = TMaxFBGetF(DEPR_TIME			,	PST_FIELD(pList, depr_time			));
			nRet = TMaxFBGetF(BUS_CLS_CD		,	PST_FIELD(pList, bus_cls_cd			));
			nRet = TMaxFBGetF(CACM_CD			,	PST_FIELD(pList, cacm_cd			));
			nRet = TMaxFBGetF(MRSP_MBPH_NO		,	PST_FIELD(pList, mrsp_mbph_no		));
			
			TMaxGetConvChar(DEPR_HNGL_NM		,	pList->depr_hngl_nm					);
			TMaxGetConvChar(DEPR_TRML_ABRV_NM	,	pList->depr_trml_abrv_nm			);
			TMaxGetConvChar(DEPR_ENG_NM			,	pList->depr_eng_nm					);
			TMaxGetConvChar(ARVL_HNGL_NM		,	pList->arvl_hngl_nm					);
			TMaxGetConvChar(ARVL_TRML_ABRV_NM	,	pList->arvl_trml_abrv_nm			);
			TMaxGetConvChar(ARVL_TRML_ENG_NM	,	pList->arvl_trml_eng_nm				);
			
			nRet = TMaxFBGetF(BUS_OPRN_DIST		,	PST_FIELD(pList, bus_oprn_dist		));
			nRet = TMaxFBGetF(TRML_BIZR_NO		,	PST_FIELD(pList, trml_bizr_no		));
			nRet = TMaxFBGetF(TRTR_ROT_EXSN_YN	,	PST_FIELD(pList, trtr_rot_exsn_yn	));
			nRet = TMaxFBGetF(TRTR_TRML_YN		,	PST_FIELD(pList, trtr_trml_yn		));

			TMaxGetConvChar(TDEPR_TRML_NM, pList->tdepr_trml_nm);

			TMaxGetConvChar(TARVL_TRML_NM, pList->tarvl_trml_nm);

			// 20211019 ADD
			nRet = TMaxFBGetF(MLG_RMN_AMT		,	PST_FIELD(pList, mlg_rmn_amt		));	/// 마일리지 잔여금액_arr
			nRet = TMaxFBGetF(CPN_RMN_AMT		,	PST_FIELD(pList, cpn_rmn_amt		)); /// 쿠폰 잔여금액_arr
			// 20211019 ~ADD

			///> set memory
			pMrsTR->m_vtResComplete.push_back(*pList);
		}
	}

	// recv log
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD"		, PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT"	, PST_FIELD(pRPacket, msg_dtl_ctt));
		LOG_WRITE("%30s - (%s) ", "ROT_RDHM_NO_VAL"	, PST_FIELD(pRPacket, rot_rdhm_no_val));
		//for(i = 0; i < sizeof(pRPacket->rot_rdhm_no_val); i++)
		//{
		//	LOG_WRITE("[%02X]", pRPacket->rot_rdhm_no_val[i] & 0xFF);
		//}

		LOG_WRITE("%30s - (%d) ", "REC_NCNT1", nCount);
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_mrspub_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TISSU_DT"		,	PST_FIELD(pList, tissu_dt				));
			LOG_WRITE("%30s - (%s) ", "TISSU_TIME"		,	PST_FIELD(pList, tissu_time				));
			LOG_WRITE("%30s - (%s) ", "TISSU_TRML_NO"	,	PST_FIELD(pList, tissu_trml_no			));
			LOG_WRITE("%30s - (%s) ", "TISSU_WND_NO"	,	PST_FIELD(pList, tissu_wnd_no			));
			LOG_WRITE("%30s - (%s) ", "TISSU_SNO"		,	PST_FIELD(pList, tissu_sno				));
			LOG_WRITE("%30s - (%s) ", "SATS_NO"			,	PST_FIELD(pList, sats_no				));
			LOG_WRITE("%30s - (%s) ", "INVS_NO"			,	PST_FIELD(pList, invs_no				));
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"		,	PST_FIELD(pList, tck_knd_cd				));
			LOG_WRITE("%30s - (%s) ", "PYN_DVS_CD"		,	PST_FIELD(pList, pyn_dvs_cd				));
			LOG_WRITE("%30s - (%s) ", "PYN_DTL_CD"		,	PST_FIELD(pList, pyn_dtl_cd				));
#if (_KTC_CERTIFY_ <= 0)
			LOG_WRITE("%30s - (%s) ", "CARD_NO"			,	PST_FIELD(pList, card_no				));
#endif
			LOG_WRITE("%30s - (%s) ", "CARD_APRV_NO"	,	PST_FIELD(pList, card_aprv_no			));
			LOG_WRITE("%30s - (num = %d) ", "OGN_APRV_AMT",	*(int *)pList->ogn_aprv_amt				);	/// 카드원승인금액_arr(후불카드 OR 가상선불 원승인금액)	
			LOG_WRITE("%30s - (num = %d) ", "RMN_APRV_AMT",	*(int *)pList->rmn_aprv_amt				);	/// 카드잔여승인금액_arr(후불카드 OR 가상선불 잔여금액) // 20211019 ADD
			LOG_WRITE("%30s - (num = %d) ", "MIP_MM_NUM",	*(int *)pList->mip_mm_num				);
			LOG_WRITE("%30s - (%s) ", "CSRC_APRV_NO"	,	PST_FIELD(pList, csrc_aprv_no			));
			LOG_WRITE("%30s - (%s) ", "USER_KEY_VAL"	,	PST_FIELD(pList, user_key_val			));
			LOG_WRITE("%30s - (num = %d) ", "CASH_RECP_AMT",*(int *)pList->cash_recp_amt			);
			LOG_WRITE("%30s - (num = %d) ", "TISSU_FEE"	,	*(int *)pList->tissu_fee				);
			LOG_WRITE("%30s - (%s) ", "STT_TIME"		,	PST_FIELD(pList, stt_time				));
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO",	PST_FIELD(pList, alcn_depr_trml_no		));
			LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO",	PST_FIELD(pList, alcn_arvl_trml_no		));
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_DT"	,	PST_FIELD(pList, alcn_depr_dt			));
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TIME"	,	PST_FIELD(pList, alcn_depr_time			));
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"	,	PST_FIELD(pList, depr_trml_no			));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"	,	PST_FIELD(pList, arvl_trml_no			));
			LOG_WRITE("%30s - (%s) ", "DEPR_DT"			,	PST_FIELD(pList, depr_dt				));
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME"		,	PST_FIELD(pList, depr_time				));
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD"		,	PST_FIELD(pList, bus_cls_cd				));
			LOG_WRITE("%30s - (%s) ", "CACM_CD"			,	PST_FIELD(pList, cacm_cd				));
			LOG_WRITE("%30s - (%s) ", "MRSP_MBPH_NO"	,	PST_FIELD(pList, mrsp_mbph_no			));
			LOG_WRITE("%30s - (%s) ", "DEPR_HNGL_NM"	,	PST_FIELD(pList, depr_hngl_nm 			));
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_ABRV_NM",	PST_FIELD(pList, depr_trml_abrv_nm		));
			LOG_WRITE("%30s - (%s) ", "DEPR_ENG_NM"		,	PST_FIELD(pList, depr_eng_nm			));
			LOG_WRITE("%30s - (%s) ", "ARVL_HNGL_NM"	,	PST_FIELD(pList, arvl_hngl_nm			));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_ABRV_NM",	PST_FIELD(pList, arvl_trml_abrv_nm		));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_ENG_NM",	PST_FIELD(pList, arvl_trml_eng_nm		));
			LOG_WRITE("%30s - (%s) ", "BUS_OPRN_DIST"	,	PST_FIELD(pList, bus_oprn_dist			));
			LOG_WRITE("%30s - (%s) ", "TRML_BIZR_NO"	,	PST_FIELD(pList, trml_bizr_no			));
			LOG_WRITE("%30s - (%s) ", "TRTR_ROT_EXSN_YN",	PST_FIELD(pList, trtr_rot_exsn_yn		));
			LOG_WRITE("%30s - (%s) ", "TRTR_TRML_YN"	,	PST_FIELD(pList, trtr_trml_yn			));
			LOG_WRITE("%30s - (%s) ", "TDEPR_TRML_NM"	,	PST_FIELD(pList, tdepr_trml_nm			));
			LOG_WRITE("%30s - (%s) ", "TARVL_TRML_NM"	,	PST_FIELD(pList, tarvl_trml_nm			));
			// 20211019 ADD
			LOG_WRITE("%30s - (num = %d) ", "MLG_RMN_AMT",	*(int *)pList->mlg_rmn_amt				);	/// 마일리지 잔여금액_arr
			LOG_WRITE("%30s - (num = %d) ", "CPN_RMN_AMT",	*(int *)pList->cpn_rmn_amt				);	/// 쿠폰 잔여금액_arr
			// 20211019 ~ADD
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
 * @brief		SVC_TK_RyAmtInfo
 * @details		(TM_RYAMTINFO) 환불 대상 금액 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_TK_RyAmtInfo(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i;
	char*	pService = "TM_RYAMTINFO";
	pstk_tm_ryamtinfo_t pSPacket;
	prtk_tm_ryamtinfo_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" start !!!!");

	pSPacket = (pstk_tm_ryamtinfo_t) pData;
	pRPacket = (prtk_tm_ryamtinfo_t) retBuf;

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

	::CopyMemory(&nCount, pSPacket->rec_ncnt, 4);

	// send log
	{
		LOG_WRITE("%30s - (%s) ", "LNG_CD"		,	pSPacket->lng_cd		);
		LOG_WRITE("%30s - (%s) ", "USER_NO"		,	pSPacket->user_no		);
		LOG_WRITE("%30s - (%d) ", "REC_NCNT"	,	nCount		);

		for(i = 0; i < nCount; i++)
		{
			LOG_WRITE("%30s - (%s) ", "TISSU_DT"		,	pSPacket->List[i].tissu_dt	);
			LOG_WRITE("%30s - (%s) ", "TISSU_TRML_NO"	,	pSPacket->List[i].tissu_trml_no	);
			LOG_WRITE("%30s - (%s) ", "TISSU_WND_NO"	,	pSPacket->List[i].tissu_wnd_no	);
			LOG_WRITE("%30s - (%s) ", "TISSU_SNO"		,	pSPacket->List[i].tissu_sno	);
		}
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD			,	pSPacket->lng_cd		);
		nRet = TMaxFBPut(USER_NO		,	pSPacket->user_no		);
		nRet = TMaxFBPut(REC_NCNT		,	pSPacket->rec_ncnt		);

		for(i = 0; i < nCount; i++)
		{
 			nRet = TMaxFBPut(TISSU_DT		,	pSPacket->List[i].tissu_dt		);
 			nRet = TMaxFBPut(TISSU_TRML_NO	,	pSPacket->List[i].tissu_trml_no	);
 			nRet = TMaxFBPut(TISSU_WND_NO	,	pSPacket->List[i].tissu_wnd_no	);
 			nRet = TMaxFBPut(TISSU_SNO		,	pSPacket->List[i].tissu_sno		);

// 			nRet = TMaxFBInsert(TISSU_DT		, pSPacket->List[i].tissu_dt		, i, sizeof(pSPacket->List[i].tissu_dt		) - 1);
// 			nRet = TMaxFBInsert(TISSU_TRML_NO	, pSPacket->List[i].tissu_trml_no	, i, sizeof(pSPacket->List[i].tissu_trml_no	) - 1);
// 			nRet = TMaxFBInsert(TISSU_WND_NO	, pSPacket->List[i].tissu_wnd_no	, i, sizeof(pSPacket->List[i].tissu_wnd_no	) - 1);
// 			nRet = TMaxFBInsert(TISSU_SNO		, pSPacket->List[i].tissu_sno		, i, sizeof(pSPacket->List[i].tissu_sno		) - 1);
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
		::ZeroMemory(&CCancRyTkKobusMem::GetInstance()->tRespInq, sizeof(rtk_tm_ryamtinfo_dt_t));
		::ZeroMemory(&CCancRyTkKobusMem::GetInstance()->tRespList, sizeof(rtk_tm_ryamtinfo_list_t));

		/// msg_cd
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		/// 지불구분코드
		nRet = TMaxFBGet(PYN_DVS_CD	, ST_FIELD(pRPacket->dt, pyn_dvs_cd));
		/// 지불상세구분코드 ( '1':카드. '2':현금, '3':현금영수증, '4':계좌이체)
		nRet = TMaxFBGet(PYN_DTL_CD	, ST_FIELD(pRPacket->dt, pyn_dtl_cd));
		/// 발권요금
		nRet = TMaxFBGet(TISSU_FEE	, ST_FIELD(pRPacket->dt, tissu_fee	));
		/// 환불종류 코드
		nRet = TMaxFBGet(RY_KND_CD	, ST_FIELD(pRPacket->dt, ry_knd_cd	));
		/// 환불율
		nRet = TMaxFBGet(CMRT		, ST_FIELD(pRPacket->dt, cmrt		));
		/// 총 환불금액
		nRet = TMaxFBGet(TOT_RY_AMT	, ST_FIELD(pRPacket->dt, tot_ry_amt));
		/// 환불차익
		nRet = TMaxFBGet(RY_PFIT	, ST_FIELD(pRPacket->dt, ry_pfit	));

		nRet = TMaxFBGet(REC_NCNT	, PST_FIELD(pRPacket, rec_ncnt1	));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);

		/// mem 저장
		::CopyMemory(&CCancRyTkKobusMem::GetInstance()->tRespInq, &pRPacket->dt, sizeof(rtk_tm_ryamtinfo_dt_t));


		// memory allocation
		if(nCount > 0)
		{
			pRPacket->pList = new rtk_tm_ryamtinfo_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_ryamtinfo_list_t) * nCount);
		}

		for(i = 0; i < nCount; i++)
		{
			prtk_tm_ryamtinfo_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGet(ALCN_DEPR_TRML_NO	, PST_FIELD(pList, 	alcn_depr_trml_no	));	 ///< 배차출발터미널번호_arr	
			nRet = TMaxFBGet(ALCN_ARVL_TRML_NO	, PST_FIELD(pList, 	alcn_arvl_trml_no	));	 ///< 배차도착터미널번호_arr	
			nRet = TMaxFBGet(ALCN_DEPR_DT		, PST_FIELD(pList, 	alcn_depr_dt		));	 ///< 배차출발일자_arr		
			nRet = TMaxFBGet(ALCN_DEPR_TIME		, PST_FIELD(pList, 	alcn_depr_time		));	 ///< 배차출발시각_arr		
			nRet = TMaxFBGet(DEPR_TRML_NO		, PST_FIELD(pList, 	depr_trml_no		));	 ///< 출발터미널번호_arr	
			nRet = TMaxFBGet(ARVL_TRML_NO		, PST_FIELD(pList, 	arvl_trml_no		));	 ///< 도착터미널번호_arr	
			nRet = TMaxFBGet(DEPR_DT			, PST_FIELD(pList, 	depr_dt				));	 ///< 출발일자_arr		
			nRet = TMaxFBGet(DEPR_TIME			, PST_FIELD(pList, 	depr_time			));	 ///< 출발시각_arr		
			nRet = TMaxFBGet(BUS_CLS_CD			, PST_FIELD(pList, 	bus_cls_cd			));	 ///< 버스등급코드_arr		
			nRet = TMaxFBGet(CACM_CD			, PST_FIELD(pList, 	cacm_cd				));	 ///< 운수사코드_arr		
			nRet = TMaxFBGet(SATS_NO			, PST_FIELD(pList, 	sats_no				));	 ///< 좌석번호_arr		
			nRet = TMaxFBGet(TCK_KND_CD			, PST_FIELD(pList, 	tck_knd_cd			));	 ///< 티켓종류코드_arr	

			// mem 저장
			::CopyMemory(&CCancRyTkKobusMem::GetInstance()->tRespList, pList, sizeof(rtk_tm_ryamtinfo_list_t));
		}
	}

	// recv log
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD"		, PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT"	, PST_FIELD(pRPacket, msg_dtl_ctt));

		LOG_WRITE("%30s - (%s) ",		"PYN_DVS_CD"	, ST_FIELD(pRPacket->dt, pyn_dvs_cd));
		LOG_WRITE("%30s - (%s) ",		"PYN_DTL_CD"	, ST_FIELD(pRPacket->dt, pyn_dtl_cd));
		LOG_WRITE("%30s - (num = %d) ", "TISSU_FEE"		, *(int *)pRPacket->dt.tissu_fee	);
		LOG_WRITE("%30s - (%s) ",		"RY_KND_CD"		, ST_FIELD(pRPacket->dt,	 ry_knd_cd	));
		LOG_WRITE("%30s - (num = %d) ", "CMRT"			, *(int *)pRPacket->dt.cmrt			);
		LOG_WRITE("%30s - (num = %d) ", "TOT_RY_AMT"	, *(int *)pRPacket->dt.tot_ry_amt	);
		LOG_WRITE("%30s - (num = %d) ",	"RY_PFIT"		, *(int *)pRPacket->dt.ry_pfit		);
		LOG_WRITE("%30s - (num = %d) ",	"REC_NCNT"		, *(int *)pRPacket->rec_ncnt1		);

		for(i = 0; i < nCount; i++)
		{
			prtk_tm_ryamtinfo_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO"	, PST_FIELD(pList, alcn_depr_trml_no	));
			LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO"	, PST_FIELD(pList, alcn_arvl_trml_no	));
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_DT"		, PST_FIELD(pList, alcn_depr_dt		));
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TIME"		, PST_FIELD(pList, alcn_depr_time		));
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"		, PST_FIELD(pList, depr_trml_no		));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"		, PST_FIELD(pList, arvl_trml_no		));
			LOG_WRITE("%30s - (%s) ", "DEPR_DT"				, PST_FIELD(pList, depr_dt				));
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME"			, PST_FIELD(pList, depr_time			));
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD"			, PST_FIELD(pList, bus_cls_cd			));
			LOG_WRITE("%30s - (%s) ", "CACM_CD"				, PST_FIELD(pList, cacm_cd				));
			LOG_WRITE("%30s - (%s) ", "SATS_NO"				, PST_FIELD(pList, sats_no				));
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD"			, PST_FIELD(pList, tck_knd_cd			));
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
 * @brief		SVC_TK_RepTran
 * @details		(TM_REPTRAN) 환불 처리 - 무인기에서 발권한거만 처리
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_TK_RepTran(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout, nCount, i;
	char*	pService = "TM_REPTRAN";
	pstk_tm_reptran_t	 pSPacket;
	prtk_tm_reptran_t	 pRPacket;

	LOG_OPEN();
	LOG_WRITE(" start !!!!");

	pSPacket = (pstk_tm_reptran_t) pData;
	pRPacket = (prtk_tm_reptran_t) retBuf;

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

	::CopyMemory(&nCount, pSPacket->ddct_qnt, 4);

	// send log
	{
		LOG_WRITE("%30s - (%s) ", "USER_NO"		,	pSPacket->user_no		); ///< 사용자 no		
		LOG_WRITE("%30s - (%s) ", "LNG_CD"		,	pSPacket->lng_cd		); ///< 언어구분		
		LOG_WRITE("%30s - (%s) ", "RY_TRML_NO"	,	pSPacket->ry_trml_no	); ///< 환불터미널번호	
		LOG_WRITE("%30s - (%s) ", "RY_WND_NO"	,	pSPacket->ry_wnd_no		); ///< 환불창구번호	
		LOG_WRITE("%30s - (%s) ", "RY_USER_NO"	,	pSPacket->ry_user_no	); ///< 환불사용자번호	
		LOG_WRITE("%30s - (%s) ", "ICCD_YN"		,	pSPacket->iccd_yn		); ///< ic카드여부(y/n)

		LOG_WRITE("%30s - (%d) ", "DDCT_QNT"	,	nCount		);

		for(i = 0; i < nCount; i++)
		{
			LOG_WRITE("%30s - (%s) "		, "TISSU_DT"		,	pSPacket->List[i].tissu_dt			);	///< 발권일자_arr		
			LOG_WRITE("%30s - (%s) "		, "TISSU_TRML_NO"	,	pSPacket->List[i].tissu_trml_no		);	///< 발권터미널번호_arr	
			LOG_WRITE("%30s - (%s) "		, "TISSU_WND_NO"	,	pSPacket->List[i].tissu_wnd_no		);	///< 발권창구번호_arr	
			LOG_WRITE("%30s - (%s) "		, "TISSU_SNO"		,	pSPacket->List[i].tissu_sno			);	///< 발권일련번호_arr	
			LOG_WRITE("%30s - (%s) "		, "SATS_NO"			,	pSPacket->List[i].sats_no			);	///< 좌석번호_arr		
		}
	}

	// send data
	{
		nRet = TMaxFBPut(USER_NO	 ,	pSPacket->user_no		);
		nRet = TMaxFBPut(LNG_CD		 ,	pSPacket->lng_cd		);
		nRet = TMaxFBPut(RY_TRML_NO	 ,	pSPacket->ry_trml_no	);
		nRet = TMaxFBPut(RY_WND_NO	 ,	pSPacket->ry_wnd_no		);
		nRet = TMaxFBPut(RY_USER_NO	 ,	pSPacket->ry_user_no	);
		nRet = TMaxFBPut(ICCD_YN	 ,	pSPacket->iccd_yn		);
		nRet = TMaxFBPut(DDCT_QNT	,	pSPacket->ddct_qnt		);

		for(i = 0; i < nCount; i++)
		{
 			nRet = TMaxFBPut(TISSU_DT		,	pSPacket->List[i].tissu_dt		);
 			nRet = TMaxFBPut(TISSU_TRML_NO	,	pSPacket->List[i].tissu_trml_no	);
 			nRet = TMaxFBPut(TISSU_WND_NO	,	pSPacket->List[i].tissu_wnd_no	);
			nRet = TMaxFBPut(TISSU_SNO		,	pSPacket->List[i].tissu_sno		);
 			nRet = TMaxFBPut(SATS_NO		,	pSPacket->List[i].sats_no		);
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
		::ZeroMemory(&CCancRyTkKobusMem::GetInstance()->tRespOk, sizeof(rtk_tm_reptran_t));

		/// msg_cd
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		nRet = TMaxFBGet(RMN_PNT		, PST_FIELD(pRPacket, rmn_pnt	  )); ///< (int)잔여포인트	
		nRet = TMaxFBGet(TISSU_FEE  	, PST_FIELD(pRPacket, tissu_fee   )); ///< (int)발권금액	
		nRet = TMaxFBGet(RY_KND_CD		, PST_FIELD(pRPacket, ry_knd_cd	  )); ///< 환불종류코드	
		nRet = TMaxFBGet(CMRT			, PST_FIELD(pRPacket, cmrt		  )); ///< (int)환불율		
		nRet = TMaxFBGet(TOT_RY_AMT 	, PST_FIELD(pRPacket, tot_ry_amt  )); ///< (int)환불금액	
		nRet = TMaxFBGet(RY_PFIT    	, PST_FIELD(pRPacket, ry_pfit     )); ///< (int)환불차익금액	
		nRet = TMaxFBGet(PYN_DVS_CD		, PST_FIELD(pRPacket, pyn_dvs_cd  )); ///< 지불구분코드 : 1:현금,2:카드,3:마일리지,4:계좌이체

		/// mem 저장
		::CopyMemory(&CCancRyTkKobusMem::GetInstance()->tRespOk, pRPacket, sizeof(rtk_tm_reptran_t));
	}

	// recv log
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD"		, PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT"	, PST_FIELD(pRPacket, msg_dtl_ctt));

		LOG_WRITE("%30s - (num = %d) ", "RMN_PNT"		, *(int *)pRPacket->rmn_pnt		);
		LOG_WRITE("%30s - (num = %d) ", "TISSU_FEE"		, *(int *)pRPacket->tissu_fee	);
		LOG_WRITE("%30s - (%s) "	  ,	"RY_KND_CD"		, PST_FIELD(pRPacket, ry_knd_cd	));
		LOG_WRITE("%30s - (num = %d) ", "CMRT"			, *(int *)pRPacket->cmrt		);
		LOG_WRITE("%30s - (num = %d) ", "TOT_RY_AMT"	, *(int *)pRPacket->tot_ry_amt	);
		LOG_WRITE("%30s - (num = %d) ", "RY_PFIT"		, *(int *)pRPacket->ry_pfit		);
		LOG_WRITE("%30s - (%s) "	  ,	"PYN_DVS_CD "	, PST_FIELD(pRPacket, pyn_dvs_cd));
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
 * @brief		SVC_TK_TckCan
 * @details		(TM_TCKCAN) 환불 처리 - 창구에서 발권한거만 처리
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_TK_TckCan(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout;
	char*	pService = "TM_TCKCAN";
	pstk_tm_tckcan_t	 pSPacket;
	prtk_tm_tckcan_t	 pRPacket;

	LOG_OPEN();
	LOG_WRITE(" start !!!!");

	// 20210910 ADD
	LOG_WRITE("%30s - (%s) ",		"PYN_DVS_CD"	, &CCancRyTkKobusMem::GetInstance()->tRespInq.pyn_dvs_cd[0]);
	//LOG_WRITE("%30s - HEX(%02X,%02X) ","PYN_DVS_CD",	CCancRyTkKobusMem::GetInstance()->tRespInq.pyn_dvs_cd[0] & 0xFF, CCancRyTkKobusMem::GetInstance()->tRespInq.pyn_dvs_cd[1] & 0xFF);

	// 지불구분코드 (1:현금,2:카드,3:마일리지,4:계좌이체,5: 복합)
	if (CCancRyTkKobusMem::GetInstance()->tRespInq.pyn_dvs_cd[0] == PYM_CD_ETC)
		pService = "TM_TCKCAN_X";	// TM_TCKCAN_X(무인기-예매발행승차권 취소(복합결제)) 서비스 추가
	
	LOG_WRITE("%30s - (%s) ",		"pService Name"	, pService);
	// 20210910 ~ADD

	pSPacket = (pstk_tm_tckcan_t) pData;
	pRPacket = (prtk_tm_tckcan_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ", "USER_NO"			,	pSPacket->user_no		); ///< 사용자 no		
		LOG_WRITE("%30s - (%s) ", "LNG_CD"			,	pSPacket->lng_cd		); ///< 언어구분		
		LOG_WRITE("%30s - (%s) ", "RY_TRML_NO"		,	pSPacket->ry_trml_no	); ///< 환불터미널번호	
		LOG_WRITE("%30s - (%s) ", "RY_WND_NO"		,	pSPacket->ry_wnd_no		); ///< 환불창구번호	
		LOG_WRITE("%30s - (%s) ", "RY_USER_NO"		,	pSPacket->ry_user_no	); ///< 환불사용자번호	
		LOG_WRITE("%30s - (%s) ", "TISSU_DT"		,	pSPacket->tissu_dt		);	///< 발권일자
		LOG_WRITE("%30s - (%s) ", "TISSU_TRML_NO"	,	pSPacket->tissu_trml_no	);	///< 발권터미널번호
		LOG_WRITE("%30s - (%s) ", "TISSU_WND_NO"	,	pSPacket->tissu_wnd_no	);	///< 발권창구번호
		LOG_WRITE("%30s - (%s) ", "TISSU_SNO"		,	pSPacket->tissu_sno		);	///< 발권일련번호
	}

	// send data
	{
		nRet = TMaxFBPut(USER_NO		,	pSPacket->user_no		);
		nRet = TMaxFBPut(LNG_CD			,	pSPacket->lng_cd		);
		nRet = TMaxFBPut(RY_TRML_NO		,	pSPacket->ry_trml_no	);
		nRet = TMaxFBPut(RY_WND_NO		,	pSPacket->ry_wnd_no		);
		nRet = TMaxFBPut(RY_USER_NO		,	pSPacket->ry_user_no	);

		nRet = TMaxFBPut(TISSU_DT		,	pSPacket->tissu_dt		);
 		nRet = TMaxFBPut(TISSU_TRML_NO	,	pSPacket->tissu_trml_no	);
 		nRet = TMaxFBPut(TISSU_WND_NO	,	pSPacket->tissu_wnd_no	);
		nRet = TMaxFBPut(TISSU_SNO		,	pSPacket->tissu_sno		);
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
		::ZeroMemory(&CCancRyTkKobusMem::GetInstance()->tRespWndOk, sizeof(rtk_tm_tckcan_t));

		/// msg_cd
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		nRet = TMaxFBGet(RMN_PNT		, PST_FIELD(pRPacket, rmn_pnt	  )); ///< (int)잔여포인트	
		nRet = TMaxFBGet(TISSU_FEE  	, PST_FIELD(pRPacket, tissu_fee   )); ///< (int)발권금액	
		nRet = TMaxFBGet(RY_KND_CD		, PST_FIELD(pRPacket, ry_knd_cd	  )); ///< 환불종류코드	
		nRet = TMaxFBGet(CMRT			, PST_FIELD(pRPacket, cmrt		  )); ///< (int)환불율		
		nRet = TMaxFBGet(TOT_RY_AMT 	, PST_FIELD(pRPacket, tot_ry_amt  )); ///< (int)환불금액	
		nRet = TMaxFBGet(RY_PFIT    	, PST_FIELD(pRPacket, ry_pfit     )); ///< (int)환불차익금액	
		nRet = TMaxFBGet(PYN_DVS_CD		, PST_FIELD(pRPacket, pyn_dvs_cd  )); ///< 지불구분코드 : 1:현금,2:카드,3:마일리지,4:계좌이체

		/// mem 저장
		::CopyMemory(&CCancRyTkKobusMem::GetInstance()->tRespWndOk, pRPacket, sizeof(rtk_tm_tckcan_t));
	}

	// recv log
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD"		, PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT"	, PST_FIELD(pRPacket, msg_dtl_ctt));

		LOG_WRITE("%30s - (num = %d) ", "RMN_PNT"		, *(int *)pRPacket->rmn_pnt		);
		LOG_WRITE("%30s - (num = %d) ", "TISSU_FEE"		, *(int *)pRPacket->tissu_fee	);
		LOG_WRITE("%30s - (%s) "	  ,	"RY_KND_CD"		, PST_FIELD(pRPacket, ry_knd_cd	));
		LOG_WRITE("%30s - (num = %d) ", "CMRT"			, *(int *)pRPacket->cmrt		);
		LOG_WRITE("%30s - (num = %d) ", "TOT_RY_AMT"	, *(int *)pRPacket->tot_ry_amt	);
		LOG_WRITE("%30s - (num = %d) ", "RY_PFIT"		, *(int *)pRPacket->ry_pfit		);
		LOG_WRITE("%30s - (%s) "	  ,	"PYN_DVS_CD "	, PST_FIELD(pRPacket, pyn_dvs_cd));
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
 * @brief		SVC_ChangeTicketBox
 * @details		(TM_MBOXTRAN) 발권박스 교환
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_ChangeTicketBox(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout;
	char*	pService = "TM_MBOXTRAN";
	pstk_tm_mboxtran_t pSPacket;
	prtk_tm_mboxtran_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" start !!!!");

	pSPacket = (pstk_tm_mboxtran_t) pData;
	pRPacket = (prtk_tm_mboxtran_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ", "LNG_CD"		,		pSPacket->lng_cd		);
		LOG_WRITE("%30s - (%s) ", "USER_NO"		,		pSPacket->user_no		);

		LOG_WRITE("%30s - (%s) ", "TRML_NO"		,		pSPacket->trml_no			);
		LOG_WRITE("%30s - (%s) ", "WND_NO"		,		pSPacket->wnd_no			);
		LOG_WRITE("%30s - (%s) ", "CLOS_INHR_NO",		pSPacket->clos_inhr_no		);
		LOG_WRITE("%30s - (%s) ", "ECG_INHR_NO"	,		pSPacket->ecg_inhr_no		);
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD			,	pSPacket->lng_cd		);
		nRet = TMaxFBPut(USER_NO		,	pSPacket->user_no		);

		nRet = TMaxFBPut(TRML_NO		,	pSPacket->trml_no		);
		nRet = TMaxFBPut(WND_NO			,	pSPacket->wnd_no		);
		nRet = TMaxFBPut(CLOS_INHR_NO	,	pSPacket->clos_inhr_no	);
		nRet = TMaxFBPut(ECG_INHR_NO	,	pSPacket->ecg_inhr_no	);

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
		/// msg_cd
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}
	}

	// recv log
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD"		, PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT"	, PST_FIELD(pRPacket, msg_dtl_ctt));
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
 * @brief		SVC_CloseWnd
 * @details		(TM_MACTTRAN) 창구 마감
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_CloseWnd(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, nTimeout;
	char*	pService = "TM_MACTTRAN";
	pstk_tm_macttran_t pSPacket;
	prtk_tm_macttran_t pRPacket;
	SYSTEMTIME				sysTime;	// 20220531 ADD

	LOG_OPEN();
	LOG_WRITE(" start !!!!");

	pSPacket = (pstk_tm_macttran_t) pData;
	pRPacket = (prtk_tm_macttran_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ", "LNG_CD"		,		pSPacket->lng_cd		);
		LOG_WRITE("%30s - (%s) ", "USER_NO"		,		pSPacket->user_no		);

		LOG_WRITE("%30s - (%s) ", "TRML_NO"		,		pSPacket->trml_no		);
		LOG_WRITE("%30s - (%s) ", "WND_NO"		,		pSPacket->wnd_no		);
// 20220531 ADD~
		// Kobus 창구마감 시, \bin\Data\Accum 데이터 리셋 후, TAK_STT_DT 값이 null 인 경우 오늘 날짜로 강제 리셋
		if (strlen(pSPacket->tak_stt_dt) <= 0)
		{
			::GetLocalTime(&sysTime);
			sprintf(pSPacket->tak_stt_dt, "%04d%02d%02d", sysTime.wYear, sysTime.wMonth, sysTime.wDay);
		}
// 20220531 ADD~
		LOG_WRITE("%30s - (%s) ", "TAK_STT_DT"	,		pSPacket->tak_stt_dt	);
		
		LOG_WRITE("%30s - (%s) ", "STT_TIME	"	,		pSPacket->stt_time		);
		LOG_WRITE("%30s - (%s) ", "INHR_NO "	,		pSPacket->inhr_no 		);
		LOG_WRITE("%30s - (%s) ", "UPD_USER_NO"	,		pSPacket->upd_user_no	);
		// 20210810 ADD
		LOG_WRITE("%30s - (%s) ", "TAK_DVS_CD"	,		pSPacket->tak_dvs_cd	);
		// 20210810 ~ADD
	}

	// send data
	{
		//SendHeader((char *)&pSPacket->hdr);

		nRet = TMaxFBPut(LNG_CD			,	pSPacket->lng_cd		);
		nRet = TMaxFBPut(USER_NO		,	pSPacket->user_no		);

		nRet = TMaxFBPut(TRML_NO		,	pSPacket->trml_no		);
		nRet = TMaxFBPut(WND_NO			,	pSPacket->wnd_no		);
		nRet = TMaxFBPut(TAK_STT_DT		,	pSPacket->tak_stt_dt	);
		nRet = TMaxFBPut(STT_TIME		,	pSPacket->stt_time		);
		nRet = TMaxFBPut(INHR_NO 		,	pSPacket->inhr_no 		);
		nRet = TMaxFBPut(UPD_USER_NO	,	pSPacket->upd_user_no	);
		nRet = TMaxFBPut(TAK_DVS_CD		,	pSPacket->tak_dvs_cd	);	// 20221202 ADD
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
		/// msg_cd
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}


	}

	// recv log
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD"		, PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT"	, PST_FIELD(pRPacket, msg_dtl_ctt));
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
 * @brief		SVC_InqPubPt
 * @details		(TM_MTCKINFO) 발권정보 조회
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_InqPubPt(int nOperID, char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	char*	pService = "TM_MTCKINFO";
	pstk_tm_mtckinfo_t pSPacket;
	prtk_tm_mtckinfo_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" start !!!!");

	pSPacket = (pstk_tm_mtckinfo_t) pData;
	pRPacket = (prtk_tm_mtckinfo_t) retBuf;

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
		LOG_WRITE("%30s - (%s) ", "LNG_CD",		pSPacket->lng_cd		);
		LOG_WRITE("%30s - (%s) ", "USER_NO",	pSPacket->user_no		);

		LOG_WRITE("%30s - (%s) ", "TRML_NO",	pSPacket->trml_no		);
		LOG_WRITE("%30s - (%s) ", "WND_NO",		pSPacket->wnd_no		);
		LOG_WRITE("%30s - (%s) ", "TAK_STT_DT",	pSPacket->tak_stt_dt	);
	}

	// send data
	{
		//SendHeader((char *)&pSPacket->hdr);

		nRet = TMaxFBPut(LNG_CD			,	pSPacket->lng_cd		);
		nRet = TMaxFBPut(USER_NO		,	pSPacket->user_no		);

		//nRet = TMaxFBPut(TRML_NO		,	pSPacket->trml_no	);
		//nRet = TMaxFBPut(WND_NO		,	pSPacket->wnd_no		);
		nRet = TMaxFBPut(TISSU_TRML_NO	,	pSPacket->trml_no	);
		nRet = TMaxFBPut(TISSU_WND_NO	,	pSPacket->wnd_no		);
		nRet = TMaxFBPut(TAK_STT_DT		,	pSPacket->tak_stt_dt	);
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
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		/// list count
		nRet = TMaxFBGet(REC_NCNT1, PST_FIELD(pRPacket, rec_ncnt1));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);
		if(nCount < 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_ncnt1", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		if(nCount > 0)
		{
			pRPacket->pList = new rtk_tm_mtckinfo_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_mtckinfo_list_t) * nCount);
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

		MyWriteFile(hFile, pRPacket->msg_cd, sizeof(pRPacket->msg_cd));
		MyWriteFile(hFile, pRPacket->msg_dtl_ctt, sizeof(pRPacket->msg_dtl_ctt));
		MyWriteFile(hFile, &nCount, sizeof(int)+1);

		for(i = 0; i < nCount; i++)
		{
			prtk_tm_mtckinfo_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(TCK_SEQ			,	PST_FIELD(pList, tck_seq			));
			nRet = TMaxFBGetF(TISSU_TIME		,	PST_FIELD(pList, tissu_time			));
			nRet = TMaxFBGetF(DEPR_TRML_NO		,	PST_FIELD(pList, depr_trml_no		));
			nRet = TMaxFBGetF(ARVL_TRML_NO		,	PST_FIELD(pList, arvl_trml_no		));
			nRet = TMaxFBGetF(DEPR_DT			,	PST_FIELD(pList, depr_dt			));
			nRet = TMaxFBGetF(DEPR_TIME			,	PST_FIELD(pList, depr_time			));
			nRet = TMaxFBGetF(BUS_CLS_CD		,	PST_FIELD(pList, bus_cls_cd			));
			nRet = TMaxFBGetF(CACM_CD			,	PST_FIELD(pList, cacm_cd			));
			nRet = TMaxFBGetF(TCK_KND_CD		,	PST_FIELD(pList, tck_knd_cd			));
			nRet = TMaxFBGetF(FEE				,	PST_FIELD(pList, fee				));
			nRet = TMaxFBGetF(SATS_NO			,	PST_FIELD(pList, sats_no			));
			nRet = TMaxFBGetF(INVS_NO			,	PST_FIELD(pList, invs_no			));
			nRet = TMaxFBGetF(PYN_DVS_CD		,	PST_FIELD(pList, pyn_dvs_cd			));
			nRet = TMaxFBGetF(PYN_DTL_CD		,	PST_FIELD(pList, pyn_dtl_cd			));
			nRet = TMaxFBGetF(CARD_NO			,	PST_FIELD(pList, card_no			));
#ifdef __KTC_PROGRAM__
			::ZeroMemory(pList->card_no, sizeof(pList->card_no));
#endif
			nRet = TMaxFBGetF(CARD_APRV_NO		,	PST_FIELD(pList, card_aprv_no		));
			nRet = TMaxFBGetF(OGN_APRV_AMT		,	PST_FIELD(pList, ogn_aprv_amt		));
			nRet = TMaxFBGetF(CSRC_APRV_NO		,	PST_FIELD(pList, csrc_aprv_no		));
			nRet = TMaxFBGetF(USER_KEY_VAL		,	PST_FIELD(pList, user_key_val		));
			nRet = TMaxFBGetF(CASH_RECP_AMT		,	PST_FIELD(pList, cash_recp_amt		));
			nRet = TMaxFBGetF(INHR_NO			,	PST_FIELD(pList, inhr_no			));
			nRet = TMaxFBGetF(CHTK_STA_CD		,	PST_FIELD(pList, chtk_sta_cd		));
			nRet = TMaxFBGetF(ALCN_DEPR_TRML_NO	,	PST_FIELD(pList, alcn_depr_trml_no	));
			nRet = TMaxFBGetF(ALCN_ARVL_TRML_NO	,	PST_FIELD(pList, alcn_arvl_trml_no	));
			nRet = TMaxFBGetF(ROT_RDHM_NO		,	PST_FIELD(pList, rot_rdhm_no		));
			nRet = TMaxFBGetF(DEPR_TRML_ABRV_NM	,	PST_FIELD(pList, depr_trml_abrv_nm	));
			nRet = TMaxFBGetF(ARVL_TRML_ABRV_NM	,	PST_FIELD(pList, arvl_trml_abrv_nm	));

			MyWriteFile(hFile, pList, sizeof(rtk_tm_mtckinfo_list_t));
		}
		MyCloseFile(hFile);
	}

	// recv log
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD"		, PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT"	, PST_FIELD(pRPacket, msg_dtl_ctt));
		LOG_WRITE("%30s - (%d) ", "REC_NCNT1", nCount);
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_mtckinfo_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######", i, nCount);
			LOG_WRITE("%30s - (%s) ", "TCK_SEQ"			,	PST_FIELD(pList, tck_seq				));
			LOG_WRITE("%30s - (%s) ", "TISSU_TIME"		,	PST_FIELD(pList, tissu_time				));
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_NO"	,	PST_FIELD(pList, depr_trml_no			));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_NO"	,	PST_FIELD(pList, arvl_trml_no			));
			LOG_WRITE("%30s - (%s) ", "DEPR_DT"			,	PST_FIELD(pList, depr_dt				));
			LOG_WRITE("%30s - (%s) ", "DEPR_TIME"		,	PST_FIELD(pList, depr_time				));
			LOG_WRITE("%30s - (%s) ", "BUS_CLS_CD"		,	PST_FIELD(pList, bus_cls_cd				));
			LOG_WRITE("%30s - (%s) ", "CACM_CD"			,	PST_FIELD(pList, cacm_cd				));
			LOG_WRITE("%30s - (%s) ", "TCK_KND_CD	"	,	PST_FIELD(pList, tck_knd_cd				));
			LOG_WRITE("%30s - (Num=%d) ", "FEE"			,	*(int *) PST_FIELD(pList, fee			));
			LOG_WRITE("%30s - (%s) ", "SATS_NO"			,	PST_FIELD(pList, sats_no				));
			LOG_WRITE("%30s - (%s) ", "INVS_NO"			,	PST_FIELD(pList, invs_no				));
			LOG_WRITE("%30s - (%s) ", "PYN_DTL_CD	"		,	PST_FIELD(pList, pyn_dtl_cd			));
#if (_KTC_CERTIFY_ <= 0)
			LOG_WRITE("%30s - (%s) ", "CARD_NO"			,	PST_FIELD(pList, card_no				));
#endif
			LOG_WRITE("%30s - (%s) ", "CARD_APRV_NO"		,	PST_FIELD(pList, card_aprv_no		));
			LOG_WRITE("%30s - (Num=%d) ", "OGN_APRV_AMT"	,	*(int *)PST_FIELD(pList, ogn_aprv_amt		));
			LOG_WRITE("%30s - (%s) ", "CSRC_APRV_NO"		,	PST_FIELD(pList, csrc_aprv_no		));
			LOG_WRITE("%30s - (%s) ", "USER_KEY_VAL"		,	PST_FIELD(pList, user_key_val		));
			LOG_WRITE("%30s - (Num=%d) ", "CASH_RECP_AMT"	,	*(int *)PST_FIELD(pList, cash_recp_amt		));
			LOG_WRITE("%30s - (%s) ", "INHR_NO"				,	PST_FIELD(pList, inhr_no			));
			LOG_WRITE("%30s - (%s) ", "CHTK_STA_CD"			,	PST_FIELD(pList, chtk_sta_cd		));
			LOG_WRITE("%30s - (%s) ", "ALCN_DEPR_TRML_NO"	,	PST_FIELD(pList, alcn_depr_trml_no	));
			LOG_WRITE("%30s - (%s) ", "ALCN_ARVL_TRML_NO"	,	PST_FIELD(pList, alcn_arvl_trml_no	));
			LOG_WRITE("%30s - (%s) ", "ROT_RDHM_NO"			,	PST_FIELD(pList, rot_rdhm_no		));
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_ABRV_NM"	,	PST_FIELD(pList, depr_trml_abrv_nm	));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_ABRV_NM"	,	PST_FIELD(pList, arvl_trml_abrv_nm	));
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
 * @brief		SVC_RePubTck
 * @details		(TM_MRETTRAN) 재발행
 * @param		char *pData			전송 데이타 버퍼
 * @param		char *retBuf		수신 데이타 버퍼
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_RePubTck(char *pData, char *retBuf)
{
#if (__USE_TMAX__ > 0)
	int		nRet, i, nCount, nTimeout;
	char*	pService = "TM_MRETTRAN";
	pstk_tm_mrettran_t pSPacket;
	prtk_tm_mrettran_t pRPacket;

	LOG_OPEN();
	LOG_WRITE(" start !!!!");

	pSPacket = (pstk_tm_mrettran_t) pData;
	pRPacket = (prtk_tm_mrettran_t) retBuf;

	CPubTckKobusMem::GetInstance()->m_vtPbTckReIssue.clear();

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
		LOG_WRITE("%30s - (%s) ",		"LNG_CD	"		,	pSPacket->lng_cd		);
		LOG_WRITE("%30s - (%s) ",		"USER_NO"		,	pSPacket->user_no		);
		LOG_WRITE("%30s - (%s) ",		"TISSU_DT"		,	pSPacket->tissu_dt		);
		LOG_WRITE("%30s - (%s) ",		"TISSU_TRML_NO"	,	pSPacket->tissu_trml_no	);
		LOG_WRITE("%30s - (%s) ",		"TISSU_WND_NO"	,	pSPacket->tissu_wnd_no	);
		LOG_WRITE("%30s - (Number=%d) ","TISSU_SNO"		,	*(int *)pSPacket->tissu_sno		);
	}

	// send data
	{
		nRet = TMaxFBPut(LNG_CD			,	pSPacket->lng_cd		);
		nRet = TMaxFBPut(USER_NO		,	pSPacket->user_no		);
		nRet = TMaxFBPut(TISSU_DT		,	pSPacket->tissu_dt			);
		nRet = TMaxFBPut(TISSU_TRML_NO	,	pSPacket->tissu_trml_no		);
		nRet = TMaxFBPut(TISSU_WND_NO	,	pSPacket->tissu_wnd_no		);
		nRet = TMaxFBPut(TISSU_SNO		,	pSPacket->tissu_sno			);
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
		/// msg_cd
		nRet = TMaxFBGet(MSG_CD, PST_FIELD(pRPacket, msg_cd));
		if( memcmp(pRPacket->msg_cd, "S0000", 5) )
		{
			// 에러코드 조회
			TMaxGetConvChar(MSG_DTL_CTT, pRPacket->msg_dtl_ctt);
			LOG_WRITE("%30s - (%s)(%s) ERROR !!!!", "MSG_CD", pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			nRet = -98;
			CConfigTkMem::GetInstance()->SetErrorCode(pRPacket->msg_cd, pRPacket->msg_dtl_ctt);
			goto fail_proc;
		}

		/// list count
		nRet = TMaxFBGet(REC_NCNT1, PST_FIELD(pRPacket, rec_ncnt1));
		::CopyMemory(&nCount, pRPacket->rec_ncnt1, 4);
		if(nCount <= 0)
		{
			// 에러코드 조회
			nRet = -99;
			LOG_WRITE("%30s - nCount = %d !!!!", "rec_ncnt1", nCount);
			CConfigTkMem::GetInstance()->SetErrorCode("SVE999");
			goto fail_proc;
		}

		{
			pRPacket->pList = new rtk_tm_mrettran_list_t[nCount];
			::ZeroMemory(pRPacket->pList, sizeof(rtk_tm_mrettran_list_t) * nCount);
		}

		for(i = 0; i < nCount; i++)
		{
			prtk_tm_mrettran_list_t pList;

			pList = &pRPacket->pList[i];

			nRet = TMaxFBGetF(DEPR_HNGL_NM 		 ,	PST_FIELD(pList, depr_hngl_nm 		 ));
			nRet = TMaxFBGetF(DEPR_TRML_ABRV_NM	 ,	PST_FIELD(pList, depr_trml_abrv_nm	 ));
			nRet = TMaxFBGetF(DEPR_ENG_NM		 ,	PST_FIELD(pList, depr_eng_nm		 ));
			nRet = TMaxFBGetF(ARVL_HNGL_NM		 ,	PST_FIELD(pList, arvl_hngl_nm		 ));
			nRet = TMaxFBGetF(ARVL_TRML_ABRV_NM	 ,	PST_FIELD(pList, arvl_trml_abrv_nm	 ));
			nRet = TMaxFBGetF(ARVL_TRML_ENG_NM	 ,	PST_FIELD(pList, arvl_trml_eng_nm	 ));
			nRet = TMaxFBGetF(BUS_OPRN_DIST		 ,	PST_FIELD(pList, bus_oprn_dist		 ));
			nRet = TMaxFBGetF(TRML_BIZR_NO		 ,	PST_FIELD(pList, trml_bizr_no		 ));
			nRet = TMaxFBGetF(TRTR_ROT_EXSN_YN	 ,	PST_FIELD(pList, trtr_rot_exsn_yn	 ));
			nRet = TMaxFBGetF(TRTR_TRML_YN		 ,	PST_FIELD(pList, trtr_trml_yn		 ));
			nRet = TMaxFBGetF(TARVL_TRML_NM		 ,	PST_FIELD(pList, tarvl_trml_nm		 ));
			nRet = TMaxFBGetF(TDEPR_TRML_NM		 ,	PST_FIELD(pList, tdepr_trml_nm		 ));

			CPubTckKobusMem::GetInstance()->m_vtPbTckReIssue.push_back(*pList);
		}
	}

	// recv log
	{
		LOG_WRITE("%30s ", "------- RecvPacket");
		LOG_WRITE("%30s - (%s) ", "MSG_CD"		, PST_FIELD(pRPacket, msg_cd));
		LOG_WRITE("%30s - (%s) ", "MSG_DTL_CTT"	, PST_FIELD(pRPacket, msg_dtl_ctt));
		LOG_WRITE("%30s - (%d) ", "REC_NCNT1", nCount);
		for(i = 0; i < nCount; i++)
		{
			prtk_tm_mrettran_list_t pList;

			pList = &pRPacket->pList[i];

			LOG_WRITE("####### 인덱스 (%4d / %4d) #######"	, i, nCount);
			LOG_WRITE("%30s - (%s) ", "DEPR_HNGL_NM	"		,	PST_FIELD(pList, depr_hngl_nm 		 ));
			LOG_WRITE("%30s - (%s) ", "DEPR_TRML_ABRV_NM"	,	PST_FIELD(pList, depr_trml_abrv_nm	 ));
			LOG_WRITE("%30s - (%s) ", "DEPR_ENG_NM"			,	PST_FIELD(pList, depr_eng_nm		 ));
			LOG_WRITE("%30s - (%s) ", "ARVL_HNGL_NM"		,	PST_FIELD(pList, arvl_hngl_nm		 ));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_ABRV_NM"	,	PST_FIELD(pList, arvl_trml_abrv_nm	 ));
			LOG_WRITE("%30s - (%s) ", "ARVL_TRML_ENG_NM"	,	PST_FIELD(pList, arvl_trml_eng_nm	 ));
			LOG_WRITE("%30s - (%s) ", "BUS_OPRN_DIST"		,	PST_FIELD(pList, bus_oprn_dist		 ));
			LOG_WRITE("%30s - (%s) ", "TRML_BIZR_NO"		,	PST_FIELD(pList, trml_bizr_no		 ));
			LOG_WRITE("%30s - (%s) ", "TRTR_ROT_EXSN_YN	"	,	PST_FIELD(pList, trtr_rot_exsn_yn	 ));
			LOG_WRITE("%30s - (%s) ", "TRTR_TRML_YN"		,	PST_FIELD(pList, trtr_trml_yn		 ));
			LOG_WRITE("%30s - (%s) ", "TARVL_TRML_NM"		,	PST_FIELD(pList, tarvl_trml_nm		 ));
			LOG_WRITE("%30s - (%s) ", "TDEPR_TRML_NM"		,	PST_FIELD(pList, tdepr_trml_nm		 ));
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
 * @brief		SVC_TK_AuthCmpt
 * @details		접속 컴퓨터 인증 & 터미널 코드 확인
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_TK_AuthCmpt(void)
{
	return 0;
}

/**
 * @brief		SVC_CM_ReadTrml
 * @details		터미널 조회 (전국터미널)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_CM_ReadTrml(void)
{

	return 0;
}

/**
 * @brief		SVC_CM_ReadTrmlStup
 * @details		터미널 환경설정 정보
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CKoBusSvr::SVC_CM_ReadTrmlStup(void)
{
	return 0;
}

