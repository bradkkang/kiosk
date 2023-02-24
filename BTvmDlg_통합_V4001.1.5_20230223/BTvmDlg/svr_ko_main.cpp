// 
// 
// svr_ko_main.cpp : 고속(코버스) 서버 main 소스
//

#include "stdafx.h"
#include <stdio.h>
#include <queue>
#include <iostream>
#include <string.h>
#include <fcntl.h>

#include <locale.h>
#include <time.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "xzzbus_fdl.h"
#include "svr_kobus.h"
#include "svr_kobus_st.h"
#include "dev_tr_main.h"
#include "dev_tr_mem.h"
#include "dev_tr_kobus_mem.h"
#include "oper_config.h"
#include "oper_kos_file.h"
#include "damo_ktc.h"
#include "event_if.h"
#include "MyDataAccum.h"

//----------------------------------------------------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------------------------------------------------

static CKoBusSvr*	 m_pclsKoBus = NULL;		/// 코버스 서버

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CheckBuffer
 * @details		버퍼 체크
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
// static int CheckBuffer(char *pData, int nMax)
// {
// 	int nLen;
// 
// 	nLen = strlen(pData);
// 	if(nLen > nMax)
// 	{
// 		nLen = nMax;
// 	}
// 	return nLen;
// }

/**
 * @brief		GetLanguageStr
 * @details		메시지 정보 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
static char* GetLanguageStr(int nIndex)
{
	static char *szLangStr[] = {
		"KO", "EN", "CN", "JP",
	};

	if( (nIndex < LANG_DVS_KO) || (nIndex >= LANG_DVS_MAX) )
	{
		return (char *)NULL;
	}

	return szLangStr[nIndex];
}

/**
 * @brief		Kobus_CM_ReadMsg
 * @details		메시지 정보 조회
 * @param		int nOperID		파일 구분자
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_CM_ReadMsg(int nOperID)
{
	int i, nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_cm_msginqr_t sPacket;
		rtk_cm_msginqr_t rPacket;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		for(i = LANG_DVS_KO; i < LANG_DVS_MAX; i++)
		{
			::ZeroMemory(&sPacket, sizeof(stk_cm_msginqr_t));
			::ZeroMemory(&rPacket, sizeof(rtk_cm_msginqr_t));

			///< lng_cd
			{
				nLen = CheckBuffer(pBuff = GetLanguageStr(i), sizeof(sPacket.lng_cd) - 1);
				::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
			}

			///< user_no
			{
				nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
				::CopyMemory(sPacket.user_no, pBuff, nLen);	
			}

			nRet = m_pclsKoBus->SVC_CM_ReadMsg(nOperID, i, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("SVC_CM_ReadMsg() Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("SVC_CM_ReadMsg() Success, nRet(%d) !!", nRet);
				if(rPacket.pList != NULL)
				{
					delete[] rPacket.pList;
					rPacket.pList = NULL;
				}
			}
		}
	}

	return nRet;
}

/**
 * @brief		Kobus_CM_ReadNtc
 * @details		승차권 고유번호(x), 공지사항 조회(o)
 * @param		int nOperID		파일 구분자
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_CM_ReadNtc(int nOperID)
{
	int nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_cm_svctimeinqr_t sPacket;
		rtk_cm_svctimeinqr_t rPacket;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		{
			::ZeroMemory(&sPacket, sizeof(stk_cm_svctimeinqr_t));
			::ZeroMemory(&rPacket, sizeof(rtk_cm_svctimeinqr_t));

			// 언어코드
			///< usrno
			{
				nLen = CheckBuffer(pBuff = GetLanguageStr(0), sizeof(sPacket.lng_cd) - 1);
				::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
			}

			///< user_no
			{
				nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
				::CopyMemory(sPacket.user_no, pBuff, nLen);	
			}

			nRet = m_pclsKoBus->SVC_CM_ReadNtc(nOperID, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("SVC_CM_ReadNtc() Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("SVC_CM_ReadNtc() Success, nRet(%d) !!", nRet);
			}
		}
	}

	return nRet;
}

/**
 * @brief		Kobus_TK_AuthCmpt
 * @details		승차권 고유번호, 공지사항 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_TK_AuthCmpt(void)
{
	int nRet;
//	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}
		
		nRet = m_pclsKoBus->SVC_TK_AuthCmpt();
	}

	return nRet;
}

/**
 * @brief		Kobus_CM_ReadCmnCd
 * @details		승차권 고유번호, 공지사항 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_CM_ReadCmnCd(int nOperID)
{
	int i, nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_cm_cdinqr_t sPacket;
		rtk_cm_cdinqr_t rPacket;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		m_pclsKoBus->m_vtKorTck.clear();
		m_pclsKoBus->m_vtEngTck.clear();
		m_pclsKoBus->m_vtChiTck.clear();
		m_pclsKoBus->m_vtJpnTck.clear();

		for(i = LANG_DVS_KO; i < LANG_DVS_MAX; i++)
		{
			::ZeroMemory(&sPacket, sizeof(stk_cm_cdinqr_t));
			::ZeroMemory(&rPacket, sizeof(rtk_cm_cdinqr_t));

			// 언어코드
			///< usrno
			{
				nLen = CheckBuffer(pBuff = GetLanguageStr(i), sizeof(sPacket.lng_cd) - 1);
				::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
			}

			///< user_no
			{
				nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
				::CopyMemory(sPacket.user_no, pBuff, nLen);	
			}

			nRet = m_pclsKoBus->SVC_CM_ReadCmnCd(nOperID, i, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("SVC_CM_ReadCmnCd() Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("SVC_CM_ReadCmnCd() Success, nRet(%d) !!", nRet);
				if(rPacket.pList != NULL)
				{
					delete[] rPacket.pList;
					rPacket.pList = NULL;
				}
			}
		}
	}

	return nRet;
}

/**
 * @brief		Kobus_CM_RdhmInqr
 * @details		승차홈 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_CM_RdhmInqr(int nOperID)
{
	int i, nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_cm_rdhminqr_t sPacket;
		rtk_cm_rdhminqr_t rPacket;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		for(i = LANG_DVS_KO; i < LANG_DVS_MAX; i++)
		{
			::ZeroMemory(&sPacket, sizeof(stk_cm_rdhminqr_t));
			::ZeroMemory(&rPacket, sizeof(rtk_cm_rdhminqr_t));

			// 언어코드
			///< usrno
			{
				nLen = CheckBuffer(pBuff = GetLanguageStr(i), sizeof(sPacket.lng_cd) - 1);
				::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
			}

			///< user_no
			{
				nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
				::CopyMemory(sPacket.user_no, pBuff, nLen);	
			}

			nRet = m_pclsKoBus->SVC_CM_RdhmInqr(nOperID, i, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("SVC_CM_RdhmInqr() Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("SVC_CM_RdhmInqr() Success, nRet(%d) !!", nRet);
				if(rPacket.pList != NULL)
				{
					delete[] rPacket.pList;
					rPacket.pList = NULL;
				}
			}
		}
	}

	return nRet;
}

/**
 * @brief		Kobus_TK_ReadTckPrtg
 * @details		승차권 인쇄정보 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_TK_ReadTckPrtg(int nOperID)
{
	int nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_tm_etckptrginfo_t sPacket;
		rtk_tm_etckptrginfo_t rPacket;
		POPER_FILE_CONFIG_T pConfig;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_etckptrginfo_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_etckptrginfo_t));

		pConfig = (POPER_FILE_CONFIG_T) GetConfigBaseInfo();

		// 언어코드
		{
			nLen = CheckBuffer(pBuff = GetLanguageStr(0), sizeof(sPacket.lng_cd) - 1);
			::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
		}

		///< user_no
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
			::CopyMemory(sPacket.user_no, pBuff, nLen);	
		}

		///< TRML_NO
		{
			nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_KOBUS), sizeof(sPacket.trml_no) - 1);
			::CopyMemory(sPacket.trml_no, pBuff, nLen);	
		}

		///< PAPR_TCK_DVS_CD (종이티켓 구분코드)
		{
			nLen = CheckBuffer(pBuff = pConfig->base_t.tck_paper_name, sizeof(sPacket.papr_tck_dvs_cd) - 1);
			::CopyMemory(sPacket.papr_tck_dvs_cd, pBuff, nLen);	
		}

		///< PTR_KND_CD
		{
			sPacket.ptr_knd_cd[0] = '4';
		}

		nRet = m_pclsKoBus->SVC_TK_ReadTckPrtg(nOperID, (char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_TK_ReadTckPrtg() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_TK_ReadTckPrtg() Success, nRet(%d) !!", nRet);
			if(rPacket.pList != NULL)
			{
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
	}

	return nRet;
}

/**
 * @brief		Kobus_CM_ReadTrmlInf
 * @details		[코버스] 터미널 정보 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_CM_ReadTrmlInf(int nOperID)
{
	int i, nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	//i = LANG_DVS_KO;
	for(i = LANG_DVS_KO; i < LANG_DVS_MAX; i++)
	{	/// 고속스(코버스 서버)
		stk_tm_etrmlinfo_t sPacket;
		rtk_tm_etrmlinfo_t rPacket;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_etrmlinfo_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_etrmlinfo_t));

		// 언어코드
		{
			nLen = CheckBuffer(pBuff = GetLanguageStr(i), sizeof(sPacket.lng_cd) - 1);
			::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
		}

		///< user_no
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
			::CopyMemory(sPacket.user_no, pBuff, nLen);	
		}

		///< TRML_NO
		{
			nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_KOBUS), sizeof(sPacket.depr_trml_no) - 1);
			::CopyMemory(sPacket.depr_trml_no, pBuff, nLen);	
		}

		nRet = m_pclsKoBus->SVC_CM_ReadTrmlInf(nOperID, i, (char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_CM_ReadTrmlInf() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_CM_ReadTrmlInf() Success, nRet(%d) !!", nRet);
			if(rPacket.pList != NULL)
			{
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
	}

	return nRet;
}

/**
 * @brief		Kobus_MG_ReadWnd
 * @details		창구 정보 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_MG_ReadWnd(int nOperID)
{
	int nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_tm_ewndinfo_t sPacket;
		rtk_tm_ewndinfo_t rPacket;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_ewndinfo_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_ewndinfo_t));

		// 언어코드
		{
			nLen = CheckBuffer(pBuff = GetLanguageStr(0), sizeof(sPacket.lng_cd) - 1);
			::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
		}

		///< user_no
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
			::CopyMemory(sPacket.user_no, pBuff, nLen);	
		}

		///< TRML_NO
		{
			nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_KOBUS), sizeof(sPacket.trml_no) - 1);
			::CopyMemory(sPacket.trml_no, pBuff, nLen);	
		}

		///< WND_NO
		{
			nLen = CheckBuffer(pBuff = GetTrmlWndNo(SVR_DVS_KOBUS), sizeof(sPacket.wnd_no) - 1);
			::CopyMemory(sPacket.wnd_no, pBuff, nLen);	
		}

		nRet = m_pclsKoBus->SVC_MG_ReadWnd(nOperID, (char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_MG_ReadWnd() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_MG_ReadWnd() Success, nRet(%d) !!", nRet);
		}
	}

	return nRet;
}

/**
 * @brief		Kobus_CM_ReadRtrpTrml
 * @details		왕복 가능 터미널 정보 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_CM_ReadRtrpTrml(int nOperID)
{
	int nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_tm_ertrptrmlinf_t sPacket;
		rtk_tm_ertrptrmlinf_t rPacket;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_ertrptrmlinf_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_ertrptrmlinf_t));

		// 언어코드
		{
			nLen = CheckBuffer(pBuff = GetLanguageStr(0), sizeof(sPacket.lng_cd) - 1);
			::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
		}

		///< user_no
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
			::CopyMemory(sPacket.user_no, pBuff, nLen);	
		}

		///< 출발지 터머널 번호
		{
			nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_KOBUS), sizeof(sPacket.depr_trml_no) - 1);
			::CopyMemory(sPacket.depr_trml_no, pBuff, nLen);	
		}

		nRet = m_pclsKoBus->SVC_CM_ReadRtrpTrml(nOperID, (char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_CM_ReadRtrpTrml() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_CM_ReadRtrpTrml() Success, nRet(%d) !!", nRet);
			if(rPacket.pList != NULL)
			{
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
	}

	return nRet;
}

///aaa
/**
 * @brief		Kobus_CM_ReadTckKnd
 * @details		승차권 종류 정보 구축
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_CM_ReadTckKnd(int nOperID)
{
	int nRet, i;

	TR_LOG_OUT(" start !!");

	for(i = LANG_DVS_KO; i < LANG_DVS_MAX; i++)
	{	/// 고속스(코버스 서버)
		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		nRet = m_pclsKoBus->SVC_CM_ReadTckKnd(nOperID);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_CM_ReadTckKnd() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_CM_ReadTckKnd() Success, nRet(%d) !!", nRet);
		}
	}

	return nRet;
}


/**
 * @brief		Kobus_CM_MngCacm
 * @details		운송회사 정보 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_CM_MngCacm(int nOperID)
{
	int i, nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	for(i = LANG_DVS_KO; i < LANG_DVS_MAX; i++)
	{	/// 고속스(코버스 서버)
		stk_cm_cacminqr_t sPacket;
		rtk_cm_cacminqr_t rPacket;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_cm_cacminqr_t));
		::ZeroMemory(&rPacket, sizeof(rtk_cm_cacminqr_t));

		// 언어코드
		{
			nLen = CheckBuffer(pBuff = GetLanguageStr(i), sizeof(sPacket.lng_cd) - 1);
			::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
		}

		///< user_no
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
			::CopyMemory(sPacket.user_no, pBuff, nLen);	
		}

		nRet = m_pclsKoBus->SVC_CM_MngCacm(nOperID, i, (char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_CM_MngCacm() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_CM_MngCacm() Success, nRet(%d) !!", nRet);
			if(rPacket.pList != NULL)
			{
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
	}

	return nRet;
}

/**
 * @brief		Kobus_CM_ReadRotInf
 * @details		경유지 정보 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_CM_ReadRotInf(void)
{
	int nRet, nLen, nOperID;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	nOperID = OPER_FILE_ID_KO_CM_READTHRUTRML;

	{	/// 고속스(코버스 서버)
		stk_tm_ethruinfo_t sPacket;
		rtk_tm_ethruinfo_t rPacket;
		CPubTckKobusMem *pPubTR;

		pPubTR = CPubTckKobusMem::GetInstance();

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_ethruinfo_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_ethruinfo_t));

		// ui 정보 복사
		{
			//char user_no 			[6+1]	;	///< 사용자번호 	
			//char lng_cd 			[2+1]	;	///< 언어코드 		
			//char tissu_trml_no 		[3+1]	;	///< 발권터미널번호 	
// 			char depr_trml_no 		[3+1]	;	///< 배차리스트 req - 출발터미널번호 	
// 			char arvl_trml_no 		[3+1]	;	///< 도착터미널번호 - 000
// 			char alcn_depr_trml_no 	[3+1]	;	///< 배차리스트 res -> 배차출발터미널번호
// 			char alcn_arvl_trml_no 	[3+1]	;	///< 배차리스트 res -> 배차도착터미널번호
// 			char alcn_rot_no 		[4+1]	;	///< 배차노선번호 - 0001

		}

		///< 언어코드
		{
			nLen = CheckBuffer(pBuff = GetLanguageStr(0), sizeof(sPacket.lng_cd) - 1);
			::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
		}

		///< user_no
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
			::CopyMemory(sPacket.user_no, pBuff, nLen);	
		}

		///< 발권 터머널 번호
		{
			nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_KOBUS), sizeof(sPacket.tissu_trml_no) - 1);
			::CopyMemory(sPacket.tissu_trml_no, pBuff, nLen);	
		}

		///< 출발 터머널 번호
		{
			nLen = CheckBuffer(pBuff = pPubTR->m_tReqAlcn.depr_trml_no, sizeof(sPacket.depr_trml_no) - 1);
			::CopyMemory(sPacket.depr_trml_no, pBuff, nLen);	
		}

		///< 도착 터머널 번호
		{
			sprintf(sPacket.arvl_trml_no, "000");	
		}

		///< 배차출발터미널번호
		{
			nLen = CheckBuffer(pBuff = pPubTR->m_tReqSats.alcn_depr_trml_no, sizeof(sPacket.alcn_depr_trml_no) - 1);
			::CopyMemory(sPacket.alcn_depr_trml_no, pBuff, nLen);	
		}

		///< 배차도착터미널번호
		{
			nLen = CheckBuffer(pBuff = pPubTR->m_tReqSats.alcn_arvl_trml_no, sizeof(sPacket.alcn_arvl_trml_no) - 1);
			::CopyMemory(sPacket.alcn_arvl_trml_no, pBuff, nLen);	
		}

		/// 2020.05.18 필드 정보 추가
		///< 배차노선번호
		{
			/// org code
			//sprintf(sPacket.alcn_rot_no, "0001");	
			
			/// modify code
			//TR_LOG_HEXA("m_tReqSats.alcn_rot_no", (BYTE *)pPubTR->m_tReqSats.alcn_rot_no, sizeof(pPubTR->m_tReqSats.alcn_rot_no));
			nLen = CheckBuffer(pBuff = pPubTR->m_tReqSats.alcn_rot_no, sizeof(sPacket.alcn_rot_no) - 1);
			::CopyMemory(sPacket.alcn_rot_no, pBuff, nLen);	
		}
		
		//::CopyMemory(&sPacket, &CPubTckKobusMem::GetInstance()->m_tReqThru, sizeof(stk_tm_ethruinfo_t));

		nRet = m_pclsKoBus->SVC_CM_ReadRotInf(nOperID, (char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_CM_MngCacm() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_CM_MngCacm() Success, nRet(%d) !!", nRet);
			if(rPacket.pList != NULL)
			{
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
	}

	return nRet;
}

/**
 * @brief		Kobus_TK_ReadMrs_KTC
 * @details		예매 내역 조회 [1/2]
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_TK_ReadMrs(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_tm_mrsinfo_t sPacket;
		rtk_tm_mrsinfo_t rPacket;
		SYSTEMTIME st;
		PUI_RESP_KO_MRS_REQ_LIST_T pReqMrs;

		::GetLocalTime(&st);

		pReqMrs = (PUI_RESP_KO_MRS_REQ_LIST_T) &CMrnpKobusMem::GetInstance()->m_ReqList;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_mrsinfo_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_mrsinfo_t));

		::CopyMemory(&sPacket, pReqMrs, sizeof(stk_tm_mrsinfo_t));
		//TR_LOG_HEXA("sPacket", (BYTE *)&sPacket, sizeof(stk_tm_mrsinfo_t));

		nRet = m_pclsKoBus->SVC_TK_ReadMrs((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_TK_ReadMrs() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_TK_ReadMrs() Success, nRet(%d) !!", nRet);
			if(rPacket.pList != NULL)
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tm_mrsinfo_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(stk_tm_mrsinfo_t));
		KTC_MemClear(&rPacket, sizeof(rtk_tm_mrsinfo_t));
#endif
	}

	return nRet;
}

/**
 * @brief		Kobus_TK_PubMrs
 * @details		예매발권  [2/2]
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_TK_PubMrs(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_tm_mrspub_t				sPacket;
		rtk_tm_mrspub_t				rPacket;
		SYSTEMTIME					st;
		int							i, nCount;
		CMrnpKobusMem*				pMrs;

		pMrs = CMrnpKobusMem::GetInstance();

		::GetLocalTime(&st);

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_mrspub_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_mrspub_t));

		::CopyMemory(sPacket.user_no 		, CMrnpKobusMem::GetInstance()->m_tUiSelData.user_no 		, sizeof(sPacket.user_no 		));
		::CopyMemory(sPacket.lng_cd 		, CMrnpKobusMem::GetInstance()->m_tUiSelData.lng_cd 		, sizeof(sPacket.lng_cd 		));
		::CopyMemory(sPacket.tissu_trml_no	, CMrnpKobusMem::GetInstance()->m_tUiSelData.tissu_trml_no	, sizeof(sPacket.tissu_trml_no	));
		::CopyMemory(sPacket.wnd_no			, CMrnpKobusMem::GetInstance()->m_tUiSelData.wnd_no			, sizeof(sPacket.wnd_no			));
		::CopyMemory(sPacket.stt_inhr_no	, CMrnpKobusMem::GetInstance()->m_tUiSelData.stt_inhr_no	, sizeof(sPacket.stt_inhr_no	));

		nCount = pMrs->m_vtUiSelList.size();
		::CopyMemory(sPacket.rec_ncnt, &nCount, sizeof(int));

		for( i = 0; i < nCount; i++ )
		{
			::CopyMemory(&sPacket.List[i], &pMrs->m_vtUiSelList[i], sizeof(stk_tm_mrspub_list_t));
		}

		nRet = m_pclsKoBus->SVC_TK_PubMrs((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("코버스 - 예매발권 실패, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("코버스 - 예매발권 성공, nRet(%d) !!", nRet);
			if(rPacket.pList != NULL)
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tm_mrspub_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(stk_tm_mrspub_t));
		KTC_MemClear(&rPacket, sizeof(rtk_tm_mrspub_t));
#endif
	}

	return nRet;
}

/**
 * @brief		Kobus_MG_ReadAlcn
 * @details		[현장발권] 배차 조회  [1/4]
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_MG_ReadAlcn(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_tm_timinfo_t sPacket;
		rtk_tm_timinfo_t rPacket;
//		CPubTckKobusMem* pPubTr;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_timinfo_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_timinfo_t));

		// ui recv 데이타 복사
		::CopyMemory(&sPacket, &CPubTckKobusMem::GetInstance()->m_tReqAlcn, sizeof(UI_RESP_KO_REQ_ALCN_T));

		nRet = m_pclsKoBus->SVC_MG_ReadAlcn((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_MG_ReadAlcn() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_MG_ReadAlcn() Success, nRet(%d) !!", nRet);
		}

		if(rPacket.pList != NULL)
		{
			TR_LOG_OUT("[DEBUG]  rPacket.pList != NULL ");
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		Kobus_CM_ReadSatsFee
 * @details		[현장발권] 좌석 정보 조회  [2/4]
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_CM_ReadSatsFee(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_cm_setinfo_t sPacket;
		rtk_cm_setinfo_t rPacket;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_cm_setinfo_t));
		::ZeroMemory(&rPacket, sizeof(rtk_cm_setinfo_t));

		// ui recv 데이타 복사
		::CopyMemory(&sPacket, &CPubTckKobusMem::GetInstance()->m_tReqSats, sizeof(stk_cm_setinfo_t));

		nRet = m_pclsKoBus->SVC_CM_ReadSatsFee((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_CM_ReadSatsFee() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_CM_ReadSatsFee() Success, nRet(%d) !!", nRet);
			if( rPacket.pList != NULL )
			{
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
	}

	return nRet;
}

/**
 * @brief		Kobus_TK_PcpySats
 * @details		[현장발권] 좌석 선점 [3/4]
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_TK_PcpySats(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_tw_satspcpy_t sPacket;
		rtk_tw_satspcpy_t rPacket;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tw_satspcpy_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tw_satspcpy_t));

		// ui recv 데이타 복사
		::CopyMemory(&sPacket, &CPubTckKobusMem::GetInstance()->m_tReqSatsPcpy, sizeof(UI_RESP_KO_REQ_SATSPCPY_T));

		nRet = m_pclsKoBus->SVC_TK_PcpySats((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_TK_PcpySats() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_TK_PcpySats() Success, nRet(%d) !!", nRet);
			if( rPacket.pList != NULL )
			{
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
	}

	return nRet;
}

/**
 * @brief		Kobus_TK_PcpySatsCancel
 * @details		[현장발권] 좌석 선점 [3/4]
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_TK_PcpySatsCancel(void)
{
	int nRet, nLen, nCount;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_tw_satspcpycanc_t sPacket;
		rtk_tw_satspcpycanc_t rPacket;
		CPubTckKobusMem* pTr;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tw_satspcpycanc_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tw_satspcpycanc_t));

		// 현장발권 class
		pTr = CPubTckKobusMem::GetInstance();

#if 1
		// 언어코드
		{
			nLen = CheckBuffer(pBuff = GetLanguageStr(CConfigTkMem::GetInstance()->n_language), sizeof(sPacket.lng_cd) - 1);
			::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
		}

		///< user_no
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
			::CopyMemory(sPacket.user_no, pBuff, nLen);	
		}

		///< 발권상태(0.발권이전,1.발권이후)	
		{
			
			sPacket.tissu_sta_cd[0] = '0';
		}
		///< 조회 요청 건수		
		{
			int i = 0;
			vector<rtk_tw_satspcpy_list_t>::iterator iter;

			nCount = pTr->m_tResSatsPcpy.size();
			::CopyMemory(sPacket.rec_ncnt1, &nCount, 4);	

			i = 0;
			for(iter = pTr->m_tResSatsPcpy.begin(); iter != pTr->m_tResSatsPcpy.end(); iter++)
			{
				/// 좌석선점번호
				nLen = CheckBuffer(pBuff = iter->pcpy_no, sizeof(sPacket.List[i].pcpy_no) - 1);
				::CopyMemory(sPacket.List[i].pcpy_no, pBuff, nLen);	
				i++;
			}
		}
#endif

		nRet = m_pclsKoBus->SVC_TK_PcpySatsCancel((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_TK_PcpySatsCancel() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_TK_PcpySatsCancel() Success, nRet(%d) !!", nRet);
		}
	}

	return nRet;
}

/**
 * @brief		Kobus_TK_PubTck
 * @details		[현장발권] 현장발권 [4/4]
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_TK_PubTck(void)
{
	int i, nRet, nCount;

	{	/// 고속스(코버스 서버)
		stk_tm_tcktran_t sPacket;
		rtk_tm_tcktran_t rPacket;
		PUI_RESP_KO_TCK_TRAN_T pUiInfo;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		nCount = 0;

		::ZeroMemory(&sPacket, sizeof(stk_tm_tcktran_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_tcktran_t));

		// ui recv 데이타 복사
		pUiInfo = &CPubTckKobusMem::GetInstance()->m_tReqTckIssue;

		::CopyMemory(&sPacket.user_no 			, pUiInfo->user_no 			, sizeof(pUiInfo->user_no 			));
		::CopyMemory(&sPacket.lng_cd 			, pUiInfo->lng_cd 			, sizeof(pUiInfo->lng_cd 			));
		::CopyMemory(&sPacket.alcn_depr_trml_no	, pUiInfo->alcn_depr_trml_no, sizeof(pUiInfo->alcn_depr_trml_no	));
		::CopyMemory(&sPacket.alcn_arvl_trml_no	, pUiInfo->alcn_arvl_trml_no, sizeof(pUiInfo->alcn_arvl_trml_no	));
		::CopyMemory(&sPacket.alcn_depr_dt		, pUiInfo->alcn_depr_dt		, sizeof(pUiInfo->alcn_depr_dt		));
		::CopyMemory(&sPacket.alcn_depr_time	, pUiInfo->alcn_depr_time	, sizeof(pUiInfo->alcn_depr_time	));
		::CopyMemory(&sPacket.depr_trml_no		, pUiInfo->depr_trml_no		, sizeof(pUiInfo->depr_trml_no		));
		::CopyMemory(&sPacket.arvl_trml_no		, pUiInfo->arvl_trml_no		, sizeof(pUiInfo->arvl_trml_no		));
		::CopyMemory(&sPacket.depr_dt			, pUiInfo->depr_dt			, sizeof(pUiInfo->depr_dt			));
		::CopyMemory(&sPacket.depr_time			, pUiInfo->depr_time		, sizeof(pUiInfo->depr_time			));
		::CopyMemory(&sPacket.bus_cls_cd		, pUiInfo->bus_cls_cd		, sizeof(pUiInfo->bus_cls_cd		));
		::CopyMemory(&sPacket.cacm_cd			, pUiInfo->cacm_cd			, sizeof(pUiInfo->cacm_cd			));
		::CopyMemory(&sPacket.trml_no			, pUiInfo->trml_no			, sizeof(pUiInfo->trml_no			));
		::CopyMemory(&sPacket.wnd_no			, pUiInfo->wnd_no			, sizeof(pUiInfo->wnd_no			));
		::CopyMemory(&sPacket.inhr_no			, pUiInfo->inhr_no			, sizeof(pUiInfo->inhr_no			));
		::CopyMemory(&sPacket.sats_no_aut_yn	, pUiInfo->sats_no_aut_yn	, sizeof(pUiInfo->sats_no_aut_yn	));
		::CopyMemory(&sPacket.inp_dvs_cd		, pUiInfo->inp_dvs_cd		, sizeof(pUiInfo->inp_dvs_cd		));
		::CopyMemory(&sPacket.card_no			, pUiInfo->card_no			, sizeof(pUiInfo->card_no			));
		::CopyMemory(&sPacket.mip_mm_num		, &pUiInfo->mip_mm_num		, sizeof(pUiInfo->mip_mm_num		));
		::CopyMemory(&sPacket.sgn_inf			, pUiInfo->sgn_inf			, sizeof(pUiInfo->sgn_inf			));
		::CopyMemory(&sPacket.iccd_inf			, pUiInfo->iccd_inf			, sizeof(pUiInfo->iccd_inf			));
		::CopyMemory(&sPacket.chit_use_dvs		, pUiInfo->chit_use_dvs		, sizeof(pUiInfo->chit_use_dvs		));
		::CopyMemory(&sPacket.iccd_yn			, pUiInfo->iccd_yn			, sizeof(pUiInfo->iccd_yn			));
		::CopyMemory(&sPacket.trck_dta_enc_cd	, pUiInfo->trck_dta_enc_cd	, sizeof(pUiInfo->trck_dta_enc_cd	));

		::CopyMemory(&sPacket.tissu_hcnt, &pUiInfo->tissu_hcnt, sizeof(pUiInfo->tissu_hcnt));
		for(i = 0; i < pUiInfo->tissu_hcnt; i++)
		{
			::CopyMemory(&sPacket.List[i], &pUiInfo->List[i], sizeof(stk_tm_tcktran_list_t));
		}

		nRet = m_pclsKoBus->SVC_TK_PubTck((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_TK_PubTck() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_TK_PubTck() Success, nRet(%d) !!", nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tm_tcktran_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(stk_tm_tcktran_t));
		KTC_MemClear(&rPacket, sizeof(rtk_tm_tcktran_t));
#endif
	}

	return nRet;
}

/**
 * @brief		Kobus_TK_RyAmtInfo
 * @details		환불 대상 금액 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_TK_RyAmtInfo(void)
{
	int nRet, nLen, nCount;
	char *pBuff;

	{	/// 고속스(코버스 서버)
		stk_tm_ryamtinfo_t sPacket;
		rtk_tm_ryamtinfo_t rPacket;
		CCancRyTkKobusMem* pTr;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_ryamtinfo_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_ryamtinfo_t));

		// 현장발권 class
		pTr = CCancRyTkKobusMem::GetInstance();

		// 20210723 ADD 
		// 환불 조회 시 인터페이스 입력값 서버(코버스) 오류(USER_NO(무인기 번호) 만 입력됨) 수정 
		TR_LOG_OUT("pTr->tReq.tissu_dt(%s), nLen(%d < %d)", pTr->tReq.tissu_dt, strlen(pTr->tReq.tissu_dt), sizeof(pTr->tReq.tissu_dt)-1);
		if( strlen(pTr->tReq.tissu_dt) < sizeof(pTr->tReq.tissu_dt)-1 )
		{
			TR_LOG_OUT("pTr->tReq.tissu_dt(%s), nLen(%d)", pTr->tReq.tissu_dt, strlen(pTr->tReq.tissu_dt));
			TR_LOG_OUT("pTr->tReq.tissu_trml_no(%s), nLen(%d)", pTr->tReq.tissu_trml_no, strlen(pTr->tReq.tissu_trml_no));
			TR_LOG_OUT("pTr->tReq.tissu_wnd_no(%s), nLen(%d)", pTr->tReq.tissu_wnd_no, strlen(pTr->tReq.tissu_wnd_no));
			TR_LOG_OUT("pTr->tReq.tissu_sno(%s), nLen(%d)", pTr->tReq.tissu_sno, strlen(pTr->tReq.tissu_sno));
			return -3;
		}
		// 20210723 ~ADD
#if 1
		// 언어코드
		{
			nLen = CheckBuffer(pBuff = GetLanguageStr(0), sizeof(sPacket.lng_cd) - 1);
			::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
		}

		///< user_no
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
			::CopyMemory(sPacket.user_no, pBuff, nLen);	
		}

		///< 환불매수 & 환불 데이타
		{
			nCount = 1;
			::CopyMemory(sPacket.rec_ncnt, &nCount, 4);	

			for(int i = 0; i < nCount; i++)
			{
				///< 발권일자_arr	
				nLen = CheckBuffer(pBuff = pTr->tReq.tissu_dt, sizeof(sPacket.List[i].tissu_dt) - 1);
				::CopyMemory(sPacket.List[i].tissu_dt, pBuff, nLen);	

				///< 발권터미널번호_arr
				nLen = CheckBuffer(pBuff = pTr->tReq.tissu_trml_no, sizeof(sPacket.List[i].tissu_trml_no) - 1);
				::CopyMemory(sPacket.List[i].tissu_trml_no, pBuff, nLen);	

				///< 발권창구번호_arr	
				nLen = CheckBuffer(pBuff = pTr->tReq.tissu_wnd_no, sizeof(sPacket.List[i].tissu_wnd_no) - 1);
				::CopyMemory(sPacket.List[i].tissu_wnd_no, pBuff, nLen);

				///< 발권일련번호_arr	
				nLen = CheckBuffer(pBuff = pTr->tReq.tissu_sno, sizeof(sPacket.List[i].tissu_sno) - 1);
				::CopyMemory(sPacket.List[i].tissu_sno, pBuff, nLen);	
			}
		}
#endif

		nRet = m_pclsKoBus->SVC_TK_RyAmtInfo((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_TK_RyAmtInfo() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_TK_RyAmtInfo() Success, nRet(%d) !!", nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tm_ryamtinfo_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(stk_tm_ryamtinfo_t));
		KTC_MemClear(&rPacket, sizeof(rtk_tm_ryamtinfo_t));
#endif
	}

	return nRet;
}

/**
 * @brief		Kobus_TK_RepTran
 * @details		환불 처리 - 무인기에서 발권한거 처리
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_TK_RepTran(void)
{
	int nRet, nLen, nCount;
	char *pBuff;

	{	/// 고속스(코버스 서버)
		stk_tm_reptran_t	sPacket;
		rtk_tm_reptran_t	rPacket;
		CCancRyTkKobusMem*	pTr;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_reptran_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_reptran_t));

		// 현장발권 class
		pTr = CCancRyTkKobusMem::GetInstance();

#if 1
		// 언어코드
		{
			nLen = CheckBuffer(pBuff = GetLanguageStr(0), sizeof(sPacket.lng_cd) - 1);
			::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
		}

		///< user_no
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
			::CopyMemory(sPacket.user_no, pBuff, nLen);	
		}

		///< 환불 터머널 번호
		{
			nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_KOBUS), sizeof(sPacket.ry_trml_no) - 1);
			::CopyMemory(sPacket.ry_trml_no, pBuff, nLen);	
		}

		///< 환불 창구번호	
		{
			nLen = CheckBuffer(pBuff = GetTrmlWndNo(SVR_DVS_KOBUS), sizeof(sPacket.ry_wnd_no) - 1);
			::CopyMemory(sPacket.ry_wnd_no, pBuff, nLen);	
		}

		///< 환불 user_no
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.ry_user_no) - 1);
			::CopyMemory(sPacket.ry_user_no, pBuff, nLen);	
		}

		///< ic카드여부(y/n)
		if(pTr->tRespInq.pyn_dtl_cd[0] == '1')
		{
			sPacket.iccd_yn[0] = 'Y';
		}
		else
		{
			sPacket.iccd_yn[0] = 'N';
		}

		///< 환불매수 & 환불 데이타
		{
			nCount = 1;
			::CopyMemory(sPacket.ddct_qnt, &nCount, 4);	

			for(int i = 0; i < nCount; i++)
			{
				///< 발권일자_arr	
				nLen = CheckBuffer(pBuff = pTr->tReq.tissu_dt, sizeof(sPacket.List[i].tissu_dt) - 1);
				::CopyMemory(sPacket.List[i].tissu_dt, pBuff, nLen);	

				///< 발권터미널번호_arr
				nLen = CheckBuffer(pBuff = pTr->tReq.tissu_trml_no, sizeof(sPacket.List[i].tissu_trml_no) - 1);
				::CopyMemory(sPacket.List[i].tissu_trml_no, pBuff, nLen);	

				///< 발권창구번호_arr	
				nLen = CheckBuffer(pBuff = pTr->tReq.tissu_wnd_no, sizeof(sPacket.List[i].tissu_wnd_no) - 1);
				::CopyMemory(sPacket.List[i].tissu_wnd_no, pBuff, nLen);

				///< 발권일련번호_arr	
				nLen = CheckBuffer(pBuff = pTr->tReq.tissu_sno, sizeof(sPacket.List[i].tissu_sno) - 1);
				::CopyMemory(sPacket.List[i].tissu_sno, pBuff, nLen);	

				///< 좌석번호_arr	
				nLen = CheckBuffer(pBuff = pTr->tRespList.sats_no, sizeof(sPacket.List[i].sats_no) - 1);
				::CopyMemory(sPacket.List[i].sats_no, pBuff, nLen);	
			}
		}
#endif

		nRet = m_pclsKoBus->SVC_TK_RepTran((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_TK_RepTran() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_TK_RepTran() Success, nRet(%d) !!", nRet);
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(stk_tm_reptran_t));
		KTC_MemClear(&rPacket, sizeof(rtk_tm_reptran_t));
#endif
	}

	return nRet;
}

/**
 * @brief		Kobus_TK_TckCan
 * @details		환불 처리 - 창구에서 발권한거
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_TK_TckCan(void)
{
	int nRet, nLen;
	char *pBuff;

	{	/// 고속스(코버스 서버)
		stk_tm_tckcan_t	sPacket;
		rtk_tm_tckcan_t	rPacket;
		CCancRyTkKobusMem*	pTr;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_tckcan_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_tckcan_t));

		// 환불 class
		pTr = CCancRyTkKobusMem::GetInstance();

#if 1
		// 언어코드
		{
			nLen = CheckBuffer(pBuff = GetLanguageStr(0), sizeof(sPacket.lng_cd) - 1);
			::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
		}

		///< user_no
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
			::CopyMemory(sPacket.user_no, pBuff, nLen);	
		}

		///< 환불 터머널 번호
		{
			nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_KOBUS), sizeof(sPacket.ry_trml_no) - 1);
			::CopyMemory(sPacket.ry_trml_no, pBuff, nLen);	
		}

		///< 환불 창구번호	
		{
			nLen = CheckBuffer(pBuff = GetTrmlWndNo(SVR_DVS_KOBUS), sizeof(sPacket.ry_wnd_no) - 1);
			::CopyMemory(sPacket.ry_wnd_no, pBuff, nLen);	
		}

		///< 환불 user_no
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.ry_user_no) - 1);
			::CopyMemory(sPacket.ry_user_no, pBuff, nLen);	
		}

		///< 발권일자
		{
			nLen = CheckBuffer(pBuff = pTr->tReq.tissu_dt, sizeof(sPacket.tissu_dt) - 1);
			::CopyMemory(sPacket.tissu_dt, pBuff, nLen);	
		}

		///< 발권터미널번호
		{
			nLen = CheckBuffer(pBuff = pTr->tReq.tissu_trml_no, sizeof(sPacket.tissu_trml_no) - 1);
			::CopyMemory(sPacket.tissu_trml_no, pBuff, nLen);	
		}

		///< 발권창구번호
		{
			nLen = CheckBuffer(pBuff = pTr->tReq.tissu_wnd_no, sizeof(sPacket.tissu_wnd_no) - 1);
			::CopyMemory(sPacket.tissu_wnd_no, pBuff, nLen);
		}

		///< 발권일련번호
		{
			nLen = CheckBuffer(pBuff = pTr->tReq.tissu_sno, sizeof(sPacket.tissu_sno) - 1);
			::CopyMemory(sPacket.tissu_sno, pBuff, nLen);	
		}
#endif

		nRet = m_pclsKoBus->SVC_TK_TckCan((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_TK_TckCan() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_TK_TckCan() Success, nRet(%d) !!", nRet);
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(stk_tm_tckcan_t));
		KTC_MemClear(&rPacket, sizeof(rtk_tm_tckcan_t));
#endif
	}

	return nRet;
}

/**
 * @brief		Kobus_TK_ChangeTicketBox
 * @details		발권박스 교환
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_TK_ChangeTicketBox(char *pbus_tck_inhr_no)
{
	int					nRet, nLen;
	char				*pBuff;
	PFILE_ACCUM_N1010_T pAccum;

	{	/// 고속스(코버스 서버)
		stk_tm_mboxtran_t sPacket;
		rtk_tm_mboxtran_t rPacket;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_mboxtran_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_mboxtran_t));

		pAccum = GetAccumulateData();

#if 1
		// 언어코드
		{
			nLen = CheckBuffer(pBuff = GetLanguageStr(0), sizeof(sPacket.lng_cd) - 1);
			::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
		}

		///< user_no
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
			::CopyMemory(sPacket.user_no, pBuff, nLen);	
		}

		///< 터미널번호	
		{
			nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_KOBUS), sizeof(sPacket.trml_no) - 1);
			::CopyMemory(sPacket.trml_no, pBuff, nLen);	
		}
		///< 창구번호	
		{
			nLen = CheckBuffer(pBuff = GetTrmlWndNo(SVR_DVS_KOBUS), sizeof(sPacket.wnd_no) - 1);
			::CopyMemory(sPacket.wnd_no, pBuff, nLen);	
		}
		///< 마감고유번호
		{
			//nLen = CheckBuffer(pBuff = , sizeof(sPacket.clos_inhr_no) - 1);
			//::CopyMemory(sPacket.clos_inhr_no, pBuff, nLen);	
			sprintf(sPacket.clos_inhr_no, "%08d", pAccum->Curr.expTicket.n_bus_tck_inhr_no);
		}
		///< 교환고유번호
		{
			nLen = CheckBuffer(pBuff = pbus_tck_inhr_no, sizeof(sPacket.ecg_inhr_no) - 1);
			::CopyMemory(sPacket.ecg_inhr_no, pBuff, nLen);	
		}
#endif

		nRet = m_pclsKoBus->SVC_ChangeTicketBox((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_ChangeTicketBox() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_ChangeTicketBox() Success, nRet(%d) !!", nRet);
		}
	}

	return nRet;
}

/**
 * @brief		Kobus_TK_CloseWnd
 * @details		창구마감
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_TK_CloseWnd(void)
{
	int					nRet, nLen;
	char				*pBuff;
	PFILE_ACCUM_N1010_T pAccum;

	{	/// 고속스(코버스 서버)
		stk_tm_macttran_t sPacket;
		rtk_tm_macttran_t rPacket;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_macttran_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_macttran_t));

		pAccum = (PFILE_ACCUM_N1010_T) GetAccumulateData();

#if 1
		// 언어코드
		{
			nLen = CheckBuffer(pBuff = GetLanguageStr(0), sizeof(sPacket.lng_cd) - 1);
			::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
		}

		///< user_no
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
			::CopyMemory(sPacket.user_no, pBuff, nLen);	
		}

		///< 터미널번호	
		{
			nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_KOBUS), sizeof(sPacket.trml_no) - 1);
			::CopyMemory(sPacket.trml_no, pBuff, nLen);	
		}
		///< 창구번호	
		{
			nLen = CheckBuffer(pBuff = GetTrmlWndNo(SVR_DVS_KOBUS), sizeof(sPacket.wnd_no) - 1);
			::CopyMemory(sPacket.wnd_no, pBuff, nLen);	
		}
		///< 작업시작일자	
		{
			//nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->kobus_tak_stt_dt, sizeof(sPacket.tak_stt_dt) - 1);
			nLen = CheckBuffer(pBuff = pAccum->tak_stt_dt, sizeof(sPacket.tak_stt_dt) - 1);
			::CopyMemory(sPacket.tak_stt_dt, pBuff, nLen);	
		}
		///< 시작시각	
		{
			//nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->kobus_stt_time, sizeof(sPacket.stt_time) - 1);
			nLen = CheckBuffer(pBuff = pAccum->stt_time, sizeof(sPacket.stt_time) - 1);
			::CopyMemory(sPacket.stt_time, pBuff, nLen);	
		}
		///< 고유번호	
		{
			sprintf(sPacket.inhr_no, "%08d", pAccum->Curr.expTicket.n_bus_tck_inhr_no);
		}
		///< 수정사용자번호
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.upd_user_no) - 1);
			::CopyMemory(sPacket.upd_user_no, pBuff, nLen);	
		}
		// 20210810 ADD
		// UI/인터페이스 수정 없이, Main 에서만 처리를 구분하기 위해 아래 코드를 추가.
		/// 자동창구마감 작업구분코드 (1: 배치job, 2: 터미널담당자 수동수행)
		POPER_FILE_CONFIG_T pConfig;
		pConfig = (POPER_FILE_CONFIG_T) GetConfigBaseInfo();

		TR_LOG_OUT("자동창구마감... auto_close_yn(%02X) ", pConfig->base_t.auto_close_yn);
		TR_LOG_OUT("자동창구마감... auto_close_time(%.6s) ", pConfig->base_t.auto_close_time);

		char sTmp[7];
		memset(sTmp, 0x00, sizeof(sTmp));
		sprintf(sTmp, "%.6s", pConfig->base_t.auto_close_time);

		std::string sTimeStr(sTmp); 

		COleDateTime tBTime = COleDateTime::GetCurrentTime();

		CTime time( 
			tBTime.GetYear(), // Year
			tBTime.GetMonth(), // Month
			tBTime.GetDay(), // Day
            atoi( sTimeStr.substr(0, 2).c_str() ), // Hour
            atoi( sTimeStr.substr(2, 2).c_str() ), // Min
            atoi( sTimeStr.substr(4, 2).c_str() )  // Sec
            );

		COleDateTime tATime(time.GetTime());
		COleDateTimeSpan tTimeSpan = tBTime - tATime;

		TR_LOG_OUT("자동창구마감... GetTotalDays(%d), GetTotalHours(%d), GetTotalMinutes(%d), GetTotalSeconds(%d) ", 
			(int)tTimeSpan.GetTotalDays(),		// 일
			(int)tTimeSpan.GetTotalHours(),		// 시간
			(int)tTimeSpan.GetTotalMinutes(),	// 분
			(int)tTimeSpan.GetTotalSeconds()	// 초
		);

		if ( (pConfig->base_t.auto_close_yn == 'Y')
			&& ( ( (tTimeSpan.GetTotalHours() == -23) && (tTimeSpan.GetTotalMinutes() < -1438) )	// 11:59:59 설정인 경우, 00:00:00 이후에 시간 체크, 24 x 60 = 1440분
				|| ( (tTimeSpan.GetTotalHours() == 0) && ( (tTimeSpan.GetTotalMinutes() == 0) || (tTimeSpan.GetTotalMinutes() == 1) ) ) )	) // 또는, 설정시간 기준으로 1분 이상 차이가 나면 수동창구마감으로 처리
		{
			::CopyMemory(sPacket.tak_dvs_cd, "1", 1);	// 1:자동창구마감	
		}
		else
		{
			::CopyMemory(sPacket.tak_dvs_cd, "2", 1);	// 2:수동창구마감	
		}
		TR_LOG_OUT("자동창구마감... tak_dvs_cd(%s) ", sPacket.tak_dvs_cd);
		// 20210810 ~ADD
#endif

		nRet = m_pclsKoBus->SVC_CloseWnd((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_CloseWnd() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_CloseWnd() Success, nRet(%d) !!", nRet);
		}
	}

	return nRet;
}

/**
 * @brief		Kobus_TK_InqPubPt
 * @details		발권 정보 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_TK_InqPubPt(char *pDate)
{
	int nRet, nLen, nOper;
	char *pBuff;

	nOper = OPER_FILE_ID_KO_TK_INQPUBPT;

	{	/// 고속스(코버스 서버)
		stk_tm_mtckinfo_t sPacket;
		rtk_tm_mtckinfo_t rPacket;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_mtckinfo_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_mtckinfo_t));

#if 1
		// 언어코드
		{
			nLen = CheckBuffer(pBuff = GetLanguageStr(0), sizeof(sPacket.lng_cd) - 1);
			::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
		}

		///< user_no
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
			::CopyMemory(sPacket.user_no, pBuff, nLen);	
		}

		///< 터미널번호	
		{
			nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_KOBUS), sizeof(sPacket.trml_no) - 1);
			::CopyMemory(sPacket.trml_no, pBuff, nLen);	
		}
		///< 창구번호	
		{
			nLen = CheckBuffer(pBuff = GetTrmlWndNo(SVR_DVS_KOBUS), sizeof(sPacket.wnd_no) - 1);
			::CopyMemory(sPacket.wnd_no, pBuff, nLen);	
		}
		///< 발권일자	
		{
			nLen = CheckBuffer(pBuff = pDate, sizeof(sPacket.tak_stt_dt) - 1);
			::CopyMemory(sPacket.tak_stt_dt, pBuff, nLen);	
		}
#endif

		nRet = m_pclsKoBus->SVC_InqPubPt(nOper, (char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_InqPubPt() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_InqPubPt() Success, nRet(%d) !!", nRet);
			if( rPacket.pList != NULL )
			{
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
	}

	return nRet;
}

/**
 * @brief		Kobus_TK_RePubTck
 * @details		코버스 승차권 재발권
 * @param		char *pub_dt			발행일자
 * @param		char *pub_trml_cd		발행터미널번호
 * @param		char *pub_wnd_no		발행창구번호
 * @param		char *pub_sno			발행일련번호
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_TK_RePubTck(char *pub_dt, char *pub_trml_cd, char *pub_wnd_no, char *pub_sno)
{
	int nRet, nLen;
	SYSTEMTIME st;
	char *pBuff;

	::GetLocalTime(&st);

	{	/// 고속스(코버스 서버)
		stk_tm_mrettran_t sPacket;
		rtk_tm_mrettran_t rPacket;

		if(m_pclsKoBus == NULL)
		{
			TR_LOG_OUT(" m_pclsKoBus = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_mrettran_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_mrettran_t));

#if 1
		// 언어코드
		{
			nLen = CheckBuffer(pBuff = GetLanguageStr(0), sizeof(sPacket.lng_cd) - 1);
			::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
		}

		///< user_no
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_KOBUS), sizeof(sPacket.user_no) - 1);
			::CopyMemory(sPacket.user_no, pBuff, nLen);	
		}

		///< 발권일자
		{
			nLen = CheckBuffer(pBuff = pub_dt, sizeof(sPacket.tissu_dt) - 1);
			::CopyMemory(sPacket.tissu_dt, pBuff, nLen);	
			//sprintf(sPacket.tissu_dt, "%04d%02d%02d", st.wYear, st.wMonth, st.wDay);
		}
		///< 발권터미널번호
		{
			nLen = CheckBuffer(pBuff = pub_trml_cd, sizeof(sPacket.tissu_trml_no) - 1);
			::CopyMemory(sPacket.tissu_trml_no, pBuff, nLen);	
		}
		///< 발권창구번호
		{
			nLen = CheckBuffer(pBuff = pub_wnd_no, sizeof(sPacket.tissu_wnd_no) - 1);
			::CopyMemory(sPacket.tissu_wnd_no, pBuff, nLen);	
		}
		///< 발권일련번호
		{
			nLen = CheckBuffer(pBuff = pub_sno, sizeof(sPacket.tissu_sno) - 1);
			::CopyMemory(sPacket.tissu_sno, pBuff, nLen);	
		}
#endif

		nRet = m_pclsKoBus->SVC_RePubTck((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("SVC_RePubTck() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("SVC_RePubTck() Success, nRet(%d) !!", nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tm_mrettran_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(stk_tm_mrettran_t));
		KTC_MemClear(&rPacket, sizeof(rtk_tm_mrettran_t));
#endif
	}

	return nRet;
}

/**
 * @brief		Kobus_Initialize
 * @details		코버스 서버 초기화
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_Initialize(void)
{
	int nRet;

	if(m_pclsKoBus != NULL)
	{
		delete m_pclsKoBus;
		m_pclsKoBus = NULL;
	}
	m_pclsKoBus = new CKoBusSvr();
	nRet = m_pclsKoBus->StartProcess();

	return nRet;
}

/**
 * @brief		Kobus_Terminate
 * @details		코버스 서버 종료
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Kobus_Terminate(void)
{
	int nRet;

	if(m_pclsKoBus != NULL)
	{
		nRet = m_pclsKoBus->EndProcess();
		TR_LOG_OUT("m_pclsKoBus->EndProcess(), nRet(%d) !!", nRet);

		delete m_pclsKoBus;
		m_pclsKoBus = NULL;
		return 0;
	}

	return -1;
}


