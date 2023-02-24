// 
// 
// svr_tmexp_main.cpp : 티머니고속 서버 main 소스
//

#include "stdafx.h"
#include <stdio.h>
#include <queue>
#include <iostream>
#include <string.h>
#include <fcntl.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "xzzbus_fdl.h"
#include "svr_main.h"
#include "svr_tm_expbus.h"
#include "svr_tm_expbus_st.h"
#include "svr_tm_exp_main.h"
#include "dev_tr_main.h"
#include "dev_tr_mem.h"
#include "dev_tr_tmexp_mem.h"
#include "oper_config.h"
#include "oper_kos_file.h"
#include "damo_ktc.h"
#include "event_if.h"
#include "cmn_util.h"
#include "File_Env_ini.h"
#include "dev_ui_main.h"

//----------------------------------------------------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------------------------------------------------

static CTmExpBusServer*  m_pclsSvr = NULL;			/// 시외버스 서버

static char tmpBuffer[1025];

//----------------------------------------------------------------------------------------------------------------------


/**
 * @brief		TMExp_Initialize
 * @details		티머니고속 서비스 초기화
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TMExp_Initialize(void)
{
	int nRet;

	if(m_pclsSvr != NULL)
	{
		delete m_pclsSvr;
		m_pclsSvr = NULL;
	}
	m_pclsSvr = new CTmExpBusServer();
	nRet = m_pclsSvr->StartProcess();

	return nRet;
}

/**
 * @brief		TMExp_Terminate
 * @details		티머니고속 서비스 종료
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TMExp_Terminate(void)
{
	int nRet;

	if(m_pclsSvr != NULL)
	{
		nRet = m_pclsSvr->EndProcess();
		TR_LOG_OUT("m_pclsSvr->EndProcess(), nRet(%d) !!", nRet);

		delete m_pclsSvr;
		m_pclsSvr = NULL;
		return 0;
	}

	return -1;
}

/**
 * @brief		TmExp_MakeHeader
 * @details		티머니고속 - make header packet
 * @param		char *pData			전송 데이타
 * @param		BOOL b_req_user_no	사용자번호 flag
 * @return		성공 : >= 0, 실패 : < 0
 */
void TmExp_MakeHeader(char *pData, BOOL b_req_user_no)
{
	int nLen;
	char *pBuff;
	pstk_tm_head_t pHdr;

	pHdr = (pstk_tm_head_t) pData;

	///< 요청 프로그램 구분
	{
		nLen = CheckBuffer(pBuff = GetReqPgmDVS(SVR_DVS_TMEXP), sizeof(pHdr->req_pgm_dvs) - 1);
		::CopyMemory(pHdr->req_pgm_dvs, pBuff, nLen);	
	}

	///< 요청 터미널번호
	{
		nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_TMEXP), sizeof(pHdr->req_trml_no) - 1);
		::CopyMemory(pHdr->req_trml_no, pBuff, nLen);	
	}

	///< 요청 창구번호
	{
		nLen = CheckBuffer(pBuff = GetTrmlWndNo(SVR_DVS_TMEXP), sizeof(pHdr->req_wnd_no) - 1);
		::CopyMemory(pHdr->req_wnd_no, pBuff, nLen);	
	}

	///< 요청 사용자번호 (인증에서 받은 user_no값) 
	if(b_req_user_no == TRUE)
	{
		if(1)
		{
			CConfigTkMem *pclsCfgMem;

			pclsCfgMem = CConfigTkMem::GetInstance();

			nLen = CheckBuffer(pBuff = pclsCfgMem->m_tTmExp.req_user_no, sizeof(pHdr->req_user_no) - 1);
			::CopyMemory(pHdr->req_user_no, pBuff, nLen);	
		}
		else
		{
			nLen = CheckBuffer(pBuff = GetEnvSvrUserNo(SVR_DVS_TMEXP), sizeof(pHdr->req_user_no) - 1);
		}
	}
}

/**
 * @brief		TmExp_CM_ReadMsg
 * @details		티머니고속 - 기초정보_1 - 메시지정보 
 * @param		int nOperID			운영파일 ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_CM_ReadMsg(int nOperID)
{
	int i, nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_readmsg_t sPacket;
		rtk_tm_readmsg_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		for(i = LANG_DVS_KO; i < LANG_DVS_MAX; i++)
		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_readmsg_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_readmsg_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket);

			///< lng_cd
			{
				nLen = CheckBuffer(pBuff = GetLanguageStr(i), sizeof(sPacket.lng_cd) - 1);
				::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
			}

			nRet = m_pclsSvr->SVC_CM_ReadMsg(nOperID, i, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-메시지 조회 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-메시지 조회 Success, nRet(%d) !!", nRet);
				if(rPacket.pList != NULL)
				{
					delete[] rPacket.pList;
					rPacket.pList = NULL;
				}
			}
		}
		nRet = m_pclsSvr->m_nRecNcnt1;
		TR_LOG_OUT("티머니고속-메시지 조회 최종 결과, nRet(%d) !!", nRet);
	}

	return nRet;
}

/**
 * @brief		TmExp_CM_ReadNtc
 * @details		티머니고속 - 기초정보_2 - 승차권 고유번호, 공지사항 조회 
 * @param		int nOperID			운영파일 ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_CM_ReadNtc(int nOperID)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_readntc_t sPacket;
		rtk_tm_readntc_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_readntc_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_readntc_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket);

			nRet = m_pclsSvr->SVC_CM_ReadNtc(nOperID, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-승차권고유번호 조회 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-승차권고유번호 조회 Success, nRet(%d) !!", nRet);
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
 * @brief		TmExp_TK_AuthCmpt
 * @details		티머니고속 - 기초정보_3 - 접속 컴퓨터 인증 & 터미널 코드 확인
 * @param		int nOperID			운영파일 ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_AuthCmpt(int nOperID)
{
	int nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_authcmpt_t sPacket;
		rtk_tm_authcmpt_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_authcmpt_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_authcmpt_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket);

			///< 컴퓨터ID (= MAC Address)
			{
				nLen = CheckBuffer(pBuff = GetMyMacAddress(), sizeof(sPacket.cmpt_id) - 1);
				::CopyMemory(sPacket.cmpt_id, pBuff, nLen);	
			}

			///< 유저 ID
			{
				nLen = CheckBuffer(pBuff = GetEnvSvrUserNo(SVR_DVS_TMEXP), sizeof(sPacket.user_id) - 1);
				::CopyMemory(sPacket.user_id, pBuff, nLen);	
			}

			///< 유저 비밀번호 
			{
				nLen = CheckBuffer(pBuff = GetEnvSvrUserPWD(SVR_DVS_TMEXP), sizeof(sPacket.user_pwd) - 1);
				::CopyMemory(sPacket.user_pwd, pBuff, nLen);	
			}

			///< 티켓 고유번호
			{
				int n_bus_tck_inhr_no = GetAccumINHR_No(SVR_DVS_TMEXP);

				sprintf(sPacket.inhr_no, "%08d", n_bus_tck_inhr_no);
			}

			nRet = m_pclsSvr->SVC_TK_AuthCmpt(nOperID, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-접속인증 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-접속인증 Success, nRet(%d) !!", nRet);
			}
		}
	}

	return nRet;
}

/**
 * @brief		TmExp_CM_ReadCmnCd
 * @details		티머니고속 - 기초정보_4 - 공통코드 조회
 * @param		int nOperID			운영파일 ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_CM_ReadCmnCd(int nOperID)
{
	int i, nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_readcmncd_t sPacket;
		rtk_tm_readcmncd_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		for(i = LANG_DVS_KO; i < LANG_DVS_MAX; i++)
		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_readcmncd_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_readcmncd_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket, TRUE);

			///< lng_cd
			{
				nLen = CheckBuffer(pBuff = GetLanguageStr(i), sizeof(sPacket.lng_cd) - 1);
				::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
			}

			nRet = m_pclsSvr->SVC_CM_ReadCmnCd(nOperID, i, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-공통코드 조회 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-공통코드 조회 Success, nRet(%d) !!", nRet);
				if(rPacket.pList != NULL)
				{
					delete[] rPacket.pList;
					rPacket.pList = NULL;
				}
			}
		}
		nRet = m_pclsSvr->m_nRecNcnt1;
		TR_LOG_OUT("티머니고속-공통코드 조회 최종 결과, nRet(%d) !!", nRet);
	}

	return nRet;
}

/**
 * @brief		TmExp_TK_ReadTckPrtg
 * @details		티머니고속 - 기초정보_5 - 승차권 인쇄정보 조회
 * @param		int nOperID			운영파일 ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_ReadTckPrtg(int nOperID)
{
	int nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_readtckprtg_t sPacket;
		rtk_tm_readtckprtg_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		/// 감열지인 경우, 이 전문을 이지 소스에서는 전송안함.

		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_readtckprtg_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_readtckprtg_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket, TRUE);

			///< 내 TRML_NO
			{
				nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_TMEXP), sizeof(sPacket.trml_no) - 1);
				::CopyMemory(sPacket.trml_no, pBuff, nLen);	
			}

			///< PAPR_TCK_DVS_CD : 종이티켓구분코드
			{
				sprintf(sPacket.papr_tck_dvs_cd, "%s", "020");	
			}

			///< PTR_KND_CD : 프린트 종류(4 or 2)
			{
				sprintf(sPacket.ptr_knd_cd, "%s", "4");	
			}

			nRet = m_pclsSvr->SVC_TK_ReadTckPrtg(nOperID, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-승차권 인쇄정보 조회 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-승차권 인쇄정보 조회 Success, nRet(%d) !!", nRet);
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
 * @brief		TmExp_MG_ReadTrmlDrtn
 * @details		티머니고속 - 기초정보_6 - 방면정보 조회
 * @param		int nOperID			운영파일 ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_MG_ReadTrmlDrtn(int nOperID)
{
	int nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_readtrmldrtn_t sPacket;
		rtk_tm_readtrmldrtn_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		/// 감열지인 경우, 이 전문을 이지 소스에서는 전송안함.

		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_readtrmldrtn_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_readtrmldrtn_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket, TRUE);

			///< 내 TRML_NO
			{
				nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_TMEXP), sizeof(sPacket.trml_no) - 1);
				::CopyMemory(sPacket.trml_no, pBuff, nLen);	
			}

			nRet = m_pclsSvr->SVC_MG_ReadTrmlDrtn(nOperID, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-방면정보 조회 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-방면정보 조회 Success, nRet(%d) !!", nRet);
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
 * @brief		TmExp_TK_ReadOwnrTrml
 * @details		티머니고속 - 기초정보_7 - 자기터미널 정보 조회
 * @param		int nOperID			운영파일 ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_ReadOwnrTrml(int nOperID)
{
	int nRet;
	//char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_readownrtrml_t sPacket;
		rtk_tm_readownrtrml_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_readownrtrml_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_readownrtrml_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket);

			nRet = m_pclsSvr->SVC_TK_ReadOwnrTrml(nOperID, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-자기터미널 정보 조회 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-자기터미널 정보 조회 Success, nRet(%d) !!", nRet);
			}
		}
	}

	return nRet;
}

/**
 * @brief		TmExp_CM_ReadTrml
 * @details		티머니고속 - 기초정보_8 - 전국 터미널 정보 조회
 * @param		int nOperID			운영파일 ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_CM_ReadTrml(int nOperID)
{
	int nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_readtrml_t sPacket;
		rtk_tm_readtrml_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		for(int i = LANG_DVS_KO; i < LANG_DVS_MAX; i++)
		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_readtrml_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_readtrml_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket, TRUE);

			///< lng_cd
			{
				nLen = CheckBuffer(pBuff = GetLanguageStr(i), sizeof(sPacket.lng_cd) - 1);
				::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
			}

			nRet = m_pclsSvr->SVC_CM_ReadTrml(nOperID, i, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-전체터미널 정보 조회 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-전체터미널 정보 조회 Success, nRet(%d) !!", nRet);
				if(rPacket.pList != NULL)
				{
					delete[] rPacket.pList;
					rPacket.pList = NULL;
				}
			}
		}
		nRet = m_pclsSvr->m_nRecNcnt1;
		TR_LOG_OUT("티머니고속-전체터미널 정보 조회 최종 결과, nRet(%d) !!", nRet);
	}

	return nRet;
}

/**
 * @brief		TmExp_CM_ReadTrmlInf
 * @details		티머니고속 - 기초정보_9 - 터미널 정보 조회
 * @param		int nOperID			운영파일 ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_CM_ReadTrmlInf(int nOperID)
{
	int nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_readtrmlinf_t sPacket;
		rtk_tm_readtrmlinf_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_readtrmlinf_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_readtrmlinf_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket, TRUE);

			///< 이전이후 구분
			{
				sprintf(sPacket.bef_aft_dvs, "D");	
			}

			///< 사용여부
			{
				sprintf(sPacket.use_yn, "Y");	
			}

			///< 정보 수
			{
				int cnt = 9999;
				::CopyMemory(sPacket.rec_num, &cnt, sizeof(int));	
			}

			///< 도착터미널 번호
			{
				sprintf(sPacket.arvl_trml_no, "000");	
			}

			///< 터미널 번호
			{
				nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_TMEXP), sizeof(sPacket.trml_no) - 1);
				::CopyMemory(sPacket.trml_no, pBuff, nLen);	
			}

			nRet = m_pclsSvr->SVC_CM_ReadTrmlInf(nOperID, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-전체터미널 정보 조회 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-전체터미널 정보 조회 Success, nRet(%d) !!", nRet);
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
 * @brief		TmExp_CM_ReadTrmlStup
 * @details		티머니고속 - 기초정보_10 - 터미널 환경설정 정보
 * @param		int nOperID			운영파일 ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_CM_ReadTrmlStup(int nOperID)
{
	int nRet;
	//char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_readtrmlstup_t sPacket;
		rtk_tm_readtrmlstup_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_readtrmlstup_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_readtrmlstup_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket, TRUE);

			nRet = m_pclsSvr->SVC_CM_ReadTrmlStup(nOperID, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-터미널 환경설정정보 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-터미널 환경설정정보 Success, nRet(%d) !!", nRet);
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
 * @brief		TmExp_MG_ReadWnd
 * @details		티머니고속 - 기초정보_11 - 창구 정보 조회
 * @param		int nOperID			운영파일 ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_MG_ReadWnd(int nOperID)
{
	int nRet;
	//char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_readwnd_t sPacket;
		rtk_tm_readwnd_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_readwnd_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_readwnd_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket, TRUE);

			nRet = m_pclsSvr->SVC_MG_ReadWnd(nOperID, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-창구정보 조회 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-창구정보 조회 Success, nRet(%d) !!", nRet);
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
 * @brief		TmExp_CM_ReadRtrpTrml
 * @details		티머니고속 - 기초정보_12 - 왕복 가능 터미널 조회
 * @param		int nOperID			운영파일 ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_CM_ReadRtrpTrml(int nOperID)
{
	int nRet;
	//char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_readrtrptrml_t sPacket;
		rtk_tm_readrtrptrml_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_readrtrptrml_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_readrtrptrml_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket, TRUE);

			nRet = m_pclsSvr->SVC_CM_ReadRtrpTrml(nOperID, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-왕복 가능 터미널 조회 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-왕복 가능 터미널 조회 Success, nRet(%d) !!", nRet);
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
 * @brief		TmExp_CM_ReadTckKnd
 * @details		티머니고속 - 기초정보_13 - 승차권 종류 정보 조회
 * @param		int nOperID			운영파일 ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_CM_ReadTckKnd(int nOperID)
{
	int nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_readtckknd_t sPacket;
		rtk_tm_readtckknd_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_readtckknd_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_readtckknd_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket, TRUE);

			///< 사용여부
			{
				sprintf(sPacket.use_yn, "Y");	
			}

			///< 요청터미널 수
			{
				int cnt = 1;
				::CopyMemory(sPacket.req_trml_num, &cnt, sizeof(int));	
			}

			///< 터미널 번호
			{
				nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_TMEXP), sizeof(sPacket.trml_no) - 1);
				::CopyMemory(sPacket.trml_no, pBuff, nLen);	
			}

			nRet = m_pclsSvr->SVC_CM_ReadTckKnd(nOperID, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-승차권 종류 정보 조회 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-승차권 종류 정보 조회 Success, nRet(%d) !!", nRet);
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
 * @brief		TmExp_CM_MngCacm
 * @details		티머니고속 - 기초정보_14 - 운송회사 정보
 * @param		int nOperID			운영파일 ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_CM_MngCacm(int nOperID)
{
	int nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_mngcacm_t sPacket;
		rtk_tm_mngcacm_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		for(int i = LANG_DVS_KO; i < LANG_DVS_MAX; i++)
		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_mngcacm_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_mngcacm_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket, TRUE);

			///< 사용여부
			{
				sprintf(sPacket.use_yn, "Y");	
			}

			///< 운수사코드
			{
				sprintf(sPacket.cacm_cd, "00");	
			}

			///< lng_cd
			{
				nLen = CheckBuffer(pBuff = GetLanguageStr(i), sizeof(sPacket.lng_cd) - 1);
				::CopyMemory(sPacket.lng_cd, pBuff, nLen);	
			}

			///< 정보 수
			{
				int cnt = 1000;
				::CopyMemory(sPacket.rec_num, &cnt, sizeof(int));	
			}

			///< 이전이후구분
			{
				sprintf(sPacket.bef_aft_dvs, "D");	
			}

			///< 요청터미널 수
			{
				int cnt = 1;
				::CopyMemory(sPacket.req_trml_num, &cnt, sizeof(int));	
			}

			///< 터미널 번호
			{
				nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_TMEXP), sizeof(sPacket.trml_no) - 1);
				::CopyMemory(sPacket.trml_no, pBuff, nLen);	
			}

			nRet = m_pclsSvr->SVC_CM_MngCacm(nOperID, i, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-운송회사 정보 조회 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-운송회사 정보 조회 Success, nRet(%d) !!", nRet);
				if(rPacket.pList != NULL)
				{
					delete[] rPacket.pList;
					rPacket.pList = NULL;
				}
			}
		}
		nRet = m_pclsSvr->m_nRecNcnt1;
		TR_LOG_OUT("티머니고속-운송회사 정보 조회 최종 결과, nRet(%d) !!", nRet);
	}

	return nRet;
}

/**
 * @brief		TmExp_CM_ReadRotInf
 * @details		티머니고속 - 기초정보_15 - 노선 정보 조회
 * @param		int nOperID			운영파일 ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_CM_ReadRotInf(int nOperID)
{
	int nRet;
	//char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_readrotinf_t sPacket;
		rtk_tm_readrotinf_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_readrotinf_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_readrotinf_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket, TRUE);

			///< 출발터미널번호
			{
				sprintf(sPacket.depr_trml_no, "000");	
			}

			///< 도착터미널번호
			{
				sprintf(sPacket.arvl_trml_no, "000");	
			}

			nRet = m_pclsSvr->SVC_CM_ReadRotInf(nOperID, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-노선 정보 조회 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-노선 정보 조회 Success, nRet(%d) !!", nRet);
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
 * @brief		TmExp_CM_ReadRyrt
 * @details		티머니고속 - 기초정보_16 - 환불율 정보 조회
 * @param		int nOperID			운영파일 ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_CM_ReadRyrt(int nOperID)
{
	int nRet;
	//char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_read_ryrt_t sPacket;
		rtk_tm_read_ryrt_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_read_ryrt_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_read_ryrt_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket, TRUE);

			sPacket.use_yn[0] = 'Y';

			nRet = m_pclsSvr->SVC_CM_ReadRyrt(nOperID, (char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-환불율 정보 조회 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-환불율 정보 조회 Success, nRet(%d) !!", nRet);
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
 * @brief		TmExp_ChangeTicketBox
 * @details		티머니고속 - 승차권 박스 교환
 * @param		char *pbus_tck_inhr_no		고유번호
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_ChangeTicketBox(char *pbus_tck_inhr_no)
{
	int nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_excbustck_t sPacket;
		rtk_tm_excbustck_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_excbustck_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_excbustck_t));

			///< make header
			TmExp_MakeHeader((char *)&sPacket, TRUE);

			///< 교환 고유번호
			{
				nLen = CheckBuffer(pBuff = pbus_tck_inhr_no, sizeof(sPacket.ecg_inhr_no) - 1);
				::CopyMemory(sPacket.ecg_inhr_no, pBuff, nLen);	
			}

			nRet = m_pclsSvr->SVC_ChangeTicketBox((char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-승차권 박스 교환 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-승차권 박스 교환 Success, nRet(%d) !!", nRet);
			}
		}
	}

	return nRet;
}

/**
 * @brief		TmExp_TK_CloseWnd
 * @details		티머니고속 - 창구 마감
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_CloseWnd(void)
{
	int nRet;
	//char *pBuff;
	PFILE_ACCUM_N1010_T pAccum;

	TR_LOG_OUT(" start !!");

	{	/// 티머니 고속 서버스
		stk_tm_closwnd_t sPacket;
		rtk_tm_closwnd_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		{
			::ZeroMemory(&sPacket, sizeof(stk_tm_closwnd_t));
			::ZeroMemory(&rPacket, sizeof(rtk_tm_closwnd_t));

			pAccum = (PFILE_ACCUM_N1010_T) GetAccumulateData();

			///< make header
			TmExp_MakeHeader((char *)&sPacket, TRUE);

			///< 고유번호	
			{
				sprintf(sPacket.inhr_no, "%08d", pAccum->Curr.expTicket.n_bus_tck_inhr_no);
			}

			nRet = m_pclsSvr->SVC_CloseWnd((char *)&sPacket, (char *)&rPacket);
			if(nRet < 0)
			{
				TR_LOG_OUT("티머니고속-창구마감 Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("티머니고속-창구마감 Success, nRet(%d) !!", nRet);
			}
		}
	}

	return nRet;
}

/**
 * @brief		TmExp_MG_ReadAlcn
 * @details		[현장발권] 배차 조회  [1/4]
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_MG_ReadAlcn(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_tm_readalcn_t sPacket;
		rtk_tm_readalcn_t rPacket;
//		CPubTckKobusMem* pPubTr;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_readalcn_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_readalcn_t));

		// ui recv 데이타 복사
		::CopyMemory(&sPacket, &CPubTckTmExpMem::GetInstance()->m_tReqAlcn, sizeof(stk_tm_readalcn_t));

		nRet = m_pclsSvr->SVC_MG_ReadAlcn((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("티머니고속-현장발권 배차조회  Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("티머니고속-현장발권 배차조회 Success, nRet(%d) !!", nRet);
		}

		if(rPacket.pLastList != NULL)
		{
			TR_LOG_OUT("[DEBUG]  rPacket.pLastList != NULL ");
			delete[] rPacket.pLastList;
			rPacket.pLastList = NULL;
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
 * @brief		TmExp_CM_ReadSatsFee
 * @details		[현장발권] 좌석 정보 조회  [1/4]
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_CM_ReadSatsFee(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_tm_readsatsfee_t sPacket;
		rtk_tm_readsatsfee_t rPacket;
//		CPubTckKobusMem* pPubTr;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_readsatsfee_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_readsatsfee_t));

		/// [티머니고속] 좌석 정보 ui recv 데이타 복사
		::CopyMemory(&sPacket, &CPubTckTmExpMem::GetInstance()->m_tReqSats, sizeof(stk_tm_readsatsfee_t));

		nRet = m_pclsSvr->SVC_CM_ReadSatsFee((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("티머니고속-좌석정보 조회  Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("티머니고속-좌석정보 조회 Success, nRet(%d) !!", nRet);
		}

		if(rPacket.pList != NULL)
		{
			TR_LOG_OUT("[DEBUG]  rPacket.pList != NULL ");
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}

		if(rPacket.pDiscList != NULL)
		{
			TR_LOG_OUT("[DEBUG]  rPacket.pDiscList != NULL ");
			delete[] rPacket.pDiscList;
			rPacket.pDiscList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		TmExp_TK_PcpySats
 * @details		[현장발권] 좌석 선점 [3/4]
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_PcpySats(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_tm_pcpysats_t sPacket;
		rtk_tm_pcpysats_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_pcpysats_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_pcpysats_t));

		// [티머니고속] 좌석선점 ui recv 데이타 복사
		::CopyMemory(&sPacket, &CPubTckTmExpMem::GetInstance()->m_tReqSatsPcpy, sizeof(stk_tm_pcpysats_t));

		nRet = m_pclsSvr->SVC_TK_PcpySats((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("티머니고속-좌석 선점 Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("티머니고속-좌석 선점 Success, nRet(%d) !!", nRet);
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
 * @brief		TmExp_TK_PcpySatsCancel
 * @details		[현장발권] 좌석 선점 해제 [3/4]
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_PcpySatsCancel(void)
{
	int nRet, i, nCount;

	nRet = i = nCount = 0;

	TR_LOG_OUT(" start !!");

	{	/// 고속스(코버스 서버)
		stk_tm_pcpysats_t sPacket;
		rtk_tm_pcpysats_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		::ZeroMemory(&sPacket, sizeof(stk_tm_pcpysats_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_pcpysats_t));

		// [티머니고속] 좌석선점 ui recv 데이타 복사
		{
			vector<rtk_tm_pcpysats_list_t>::iterator iter;
			CPubTckTmExpMem *pTmExpMem;

			pTmExpMem = CPubTckTmExpMem::GetInstance();

			::CopyMemory(&sPacket, &pTmExpMem->m_tReqSatsPcpy, sizeof(stk_tm_pcpysats_t));
			sPacket.req_dvs_cd[0] = 'N';

			nCount = pTmExpMem->m_tResSatsPcpy.size();
			if(nCount > 0)
			{
				i = 0;
				for(iter = pTmExpMem->m_tResSatsPcpy.begin(); iter != pTmExpMem->m_tResSatsPcpy.end(); iter++)
				{
					ZeroMemory(&sPacket.List[i], sizeof(stk_tm_pcpysats_list_t));

					::CopyMemory(sPacket.List[i].pcpy_no, iter->pcpy_no, sizeof(sPacket.List[i].pcpy_no));	
					::CopyMemory(sPacket.List[i].sats_no, iter->sats_no, sizeof(sPacket.List[i].sats_no));	
					i++;
				}
			}
			else
			{
				ZeroMemory(&sPacket.List[0], sizeof(stk_tm_pcpysats_list_t) * 100);
			}
		}

		nRet = m_pclsSvr->SVC_TK_PcpySatsCancel((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("티머니고속-좌석 선점 해제 Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("티머니고속-좌석 선점 해제 Success, nRet(%d) !!", nRet);
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
 * @brief		TmExp_TK_PubTck
 * @details		[현장발권] 현장발권(자진발급)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_PubTckCash(void)
{
	int nRet, nCount;

	{	/// 고속스(코버스 서버)
		stk_tm_pubtckcash_t sPacket;
		rtk_tm_pubtckcash_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		nCount = 0;

		::ZeroMemory(&sPacket, sizeof(stk_tm_pubtckcash_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_pubtckcash_t));

		// [티머니고속] 현장발권(현금) - ui recv 데이타 복사
		::CopyMemory(&sPacket, &CPubTckTmExpMem::GetInstance()->m_tReqPubTckCash, sizeof(stk_tm_pubtckcash_t));

		nRet = m_pclsSvr->SVC_TK_PubTckCash((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("티머니고속-현장발권(현금) Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("티머니고속-현장발권(현금) Success, nRet(%d) !!", nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(stk_tm_pubtckcash_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(sPacket));
		KTC_MemClear(&rPacket, sizeof(rPacket));
#endif
	}

	return nRet;
}

/**
 * @brief		TmExp_TK_PubTckCardKTC
 * @details		[현장발권] 현장발권(카드_KTC)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_PubTckCardKTC(void)
{
	int nRet, nCount;

	{	/// 고속스(코버스 서버)
		stk_tm_pubtckcard_ktc_t sPacket;
		rtk_tm_pubtckcard_ktc_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		nCount = 0;

		::ZeroMemory(&sPacket, sizeof(stk_tm_pubtckcard_ktc_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_pubtckcard_ktc_t));

		// [티머니고속] 현장발권(카드_KTC) - ui recv 데이타 복사
		::CopyMemory(&sPacket, &CPubTckTmExpMem::GetInstance()->m_tReqPubTckCardKtc, sizeof(stk_tm_pubtckcard_ktc_t));

		nRet = m_pclsSvr->SVC_TK_PubTckCard_KTC((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("티머니고속-현장발권(카드_KTC) Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("티머니고속-현장발권(카드_KTC) Success, nRet(%d) !!", nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tm_pubtckcard_ktc_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(sPacket));
		KTC_MemClear(&rPacket, sizeof(rPacket));
#endif

	}

	return nRet;
}

/**
 * @brief		TmExp_TK_PubTckCsrcKTC
 * @details		[현장발권] 현장발권(현금영수증_KTC)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_PubTckCsrcKTC(void)
{
	int nRet, nCount;

	{	/// 고속스(코버스 서버)
		stk_tm_pubtckcsrc_ktc_t sPacket;
		rtk_tm_pubtckcsrc_ktc_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		nCount = 0;

		::ZeroMemory(&sPacket, sizeof(stk_tm_pubtckcsrc_ktc_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_pubtckcsrc_ktc_t));

		// [티머니고속] 현장발권(현금영수증_KTC) - ui recv 데이타 복사
		::CopyMemory(&sPacket, &CPubTckTmExpMem::GetInstance()->m_tReqPubTckCsrcKtc, sizeof(stk_tm_pubtckcsrc_ktc_t));

		nRet = m_pclsSvr->SVC_TK_PubTckCsrc_KTC((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("티머니고속-현장발권(현금영수증_KTC) Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("티머니고속-현장발권(현금영수증_KTC) Success, nRet(%d) !!", nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tm_pubtckcsrc_ktc_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(sPacket));
		KTC_MemClear(&rPacket, sizeof(rPacket));
#endif
	}

	return nRet;
}

/**
 * @brief		TmExp_TK_PubTckPrd
 * @details		[현장발권] 현장발권(부가상품권)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_PubTckPrd(void)
{
	int nRet, nCount;

	{	/// 고속스(코버스 서버)
		stk_tm_pubtckprd_t sPacket;
		rtk_tm_pubtckprd_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		nCount = 0;

		::ZeroMemory(&sPacket, sizeof(stk_tm_pubtckprd_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_pubtckprd_t));

		// [티머니고속] 현장발권(부가상품권) - ui recv 데이타 복사
		::CopyMemory(&sPacket, &CPubTckTmExpMem::GetInstance()->m_tReqPubTckPrd, sizeof(stk_tm_pubtckprd_t));

		nRet = m_pclsSvr->SVC_TK_PubTckPrd((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("티머니고속-현장발권(부가상품권) Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("티머니고속-현장발권(부가상품권) Success, nRet(%d) !!", nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tm_pubtckprd_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(sPacket));
		KTC_MemClear(&rPacket, sizeof(rPacket));
#endif
	}

	return nRet;
}

/**
 * @brief		TmExp_CM_ReadThruTrml
 * @details		[티머니고속] 현장발권 : 경유지 정보 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_CM_ReadThruTrml(void)
{
	int nRet, nCount;

	{	/// 고속스(코버스 서버)
		stk_tm_readthrutrml_t sPacket;
		rtk_tm_readthrutrml_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		nCount = 0;

		::ZeroMemory(&sPacket, sizeof(stk_tm_readthrutrml_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_readthrutrml_t));

		// [티머니고속] 현장발권:경유지 정보 
		{
			int nRecNum = 999;
			stk_tm_readthrutrml_t *pInfo;
			stk_tm_readsatsfee_t *pRef;

			pInfo = &CPubTckTmExpMem::GetInstance()->m_tReqThru;
			//pRef  = &CPubTckTmExpMem::GetInstance()->m_tReqAlcn;
			pRef  = &CPubTckTmExpMem::GetInstance()->m_tReqSats;

			{
				TR_LOG_OUT("%30s - (%s) ", "req_pgm_dvs"		, pRef->req_pgm_dvs			);
				TR_LOG_OUT("%30s - (%s) ", "req_trml_no"		, pRef->req_trml_no			);
				TR_LOG_OUT("%30s - (%s) ", "req_wnd_no"			, pRef->req_wnd_no			);
				TR_LOG_OUT("%30s - (%s) ", "req_user_no"		, pRef->req_user_no			);
				TR_LOG_OUT("%30s - (%s) ", "alcn_depr_trml_no"	, pRef->alcn_depr_trml_no	);
				TR_LOG_OUT("%30s - (%s) ", "alcn_arvl_trml_no"	, pRef->alcn_arvl_trml_no	);
				TR_LOG_OUT("%30s - (%s) ", "alcn_rot_no"		, pRef->alcn_rot_no			);
				TR_LOG_OUT("%30s - (%s) ", "depr_trml_no"		, pRef->depr_trml_no		);
				TR_LOG_OUT("%30s - (%s) ", "arvl_trml_no"		, pRef->arvl_trml_no		);
				TR_LOG_OUT("%30s - (%s) ", "alcn_depr_dt"		, pRef->alcn_depr_dt		);
				TR_LOG_OUT("%30s - (%s) ", "alcn_depr_time"		, pRef->alcn_depr_time		);
				TR_LOG_OUT("%30s - (%s) ", "depr_dt"			, pRef->depr_dt				);
				TR_LOG_OUT("%30s - (%s) ", "depr_time"			, pRef->depr_time			);
				TR_LOG_OUT("%30s - (%s) ", "bus_cls_cd"			, pRef->bus_cls_cd			);
				TR_LOG_OUT("%30s - (%s) ", "cacm_cd"			, pRef->cacm_cd				);
			}

			::CopyMemory(pInfo->req_pgm_dvs			, pRef->req_pgm_dvs,		sizeof(pInfo->req_pgm_dvs) - 1) ;
			::CopyMemory(pInfo->req_trml_no			, pRef->req_trml_no,		sizeof(pInfo->req_trml_no) - 1) ;
			::CopyMemory(pInfo->req_wnd_no			, pRef->req_wnd_no,			sizeof(pInfo->req_wnd_no) - 1) ;
			::CopyMemory(pInfo->req_user_no			, pRef->req_user_no,		sizeof(pInfo->req_user_no) - 1) ;
			pInfo->req_trml_num[0]	= 0x01;
			::CopyMemory(pInfo->trml_no				, pRef->depr_trml_no,		sizeof(pInfo->trml_no) - 1) ;
			pInfo->bef_aft_dvs[0] = 'D';
			::CopyMemory(pInfo->rec_num				, &nRecNum,					sizeof(int)) ;
			::CopyMemory(pInfo->drtn_cd				, "00",						2) ;
			pInfo->hspd_cty_dvs_cd[0] = '0';
			
			::CopyMemory(pInfo->alcn_depr_trml_no	, pRef->alcn_depr_trml_no,	sizeof(pInfo->alcn_depr_trml_no) - 1) ;
			::CopyMemory(pInfo->alcn_arvl_trml_no	, pRef->alcn_arvl_trml_no,	sizeof(pInfo->alcn_arvl_trml_no) - 1) ;
			::CopyMemory(pInfo->alcn_rot_no			, pRef->alcn_rot_no,		sizeof(pInfo->alcn_rot_no) - 1) ;
			::CopyMemory(pInfo->arvl_trml_no		, "000", 3) ;

			::CopyMemory(&sPacket, pInfo, sizeof(stk_tm_readthrutrml_t));

		}

		nRet = m_pclsSvr->SVC_CM_ReadThruTrml((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("티머니고속-현장발권(경유지정보 조회) Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("티머니고속-현장발권(경유지정보 조회) Success, nRet(%d) !!", nRet);
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
 * @brief		TmExp_TK_ReadMrs
 * @details		[티머니고속] 예매발권 : 예매내역 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_ReadMrs(void)
{
	int nRet, nCount;
	char *pTitle = "티머니고속-예매내역 조회";

	{	
		stk_tm_read_mrs_t sPacket;
		rtk_tm_read_mrs_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		nCount = 0;

		::ZeroMemory(&sPacket, sizeof(stk_tm_read_mrs_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_read_mrs_t));

		// [티머니고속] 예매발권: 예매내역 요청 - ui recv 데이타 복사
		::CopyMemory(&sPacket, &CMrnpTmExpMem::GetInstance()->m_ReqList.rData, sizeof(stk_tm_read_mrs_t));

		nRet = m_pclsSvr->SVC_TK_ReadMrs((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("%s Fail, nRet(%d) !!", pTitle, nRet);
		}
		else
		{
			TR_LOG_OUT("%s Success, nRet(%d) !!", pTitle, nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tm_read_mrs_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(sPacket));
		KTC_MemClear(&rPacket, sizeof(rPacket));
#endif
	}

	return nRet;
}

/**
 * @brief		TmExp_TK_KtcReadMrs
 * @details		[티머니고속] 예매발권 : KTC 예매내역 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_KtcReadMrs(void)
{
	int nRet, nCount;
	char *pTitle = "티머니고속-예매내역조회(KTC예매내역 조회)";

	{	
		stk_tm_read_mrs_ktc_t sPacket;
		rtk_tm_read_mrs_ktc_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		nCount = 0;

		::ZeroMemory(&sPacket, sizeof(stk_tm_read_mrs_ktc_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_read_mrs_ktc_t));

		// [티머니고속] 예매발권: KTC 예매내역 요청 - ui recv 데이타 복사
		::CopyMemory(&sPacket, 
					 &CMrnpTmExpMem::GetInstance()->m_ReqKtcList.rData, 
					 sizeof(stk_tm_read_mrs_ktc_t));

		nRet = m_pclsSvr->SVC_TK_KtcReadMrs((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("%s Fail, nRet(%d) !!", pTitle, nRet);
		}
		else
		{
			TR_LOG_OUT("%s Success, nRet(%d) !!", pTitle, nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tm_read_mrs_ktc_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(sPacket));
		KTC_MemClear(&rPacket, sizeof(rPacket));
#endif
}

	return nRet;
}

/**
 * @brief		TmExp_TK_PubMrs
 * @details		[티머니고속] 예매발권 : 예매발권 tmax
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_PubMrs(void)
{
	int nRet, nCount;
	char *pTitle = "티머니고속-예매발권";

	{	
		stk_tm_pub_mrs_t sPacket;
		rtk_tm_pub_mrs_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		nCount = 0;

		::ZeroMemory(&sPacket, sizeof(stk_tm_pub_mrs_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_pub_mrs_t));

		// [티머니고속] 예매발권: 예매발권 요청 - ui recv 데이타 복사
		::CopyMemory(&sPacket, 
					 &CMrnpTmExpMem::GetInstance()->m_ReqPubMrs.rData, 
					 sizeof(stk_tm_pub_mrs_t));

		nRet = m_pclsSvr->SVC_TK_PubMrs((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("%s Fail, nRet(%d) !!", pTitle, nRet);
		}
		else
		{
			TR_LOG_OUT("%s Success, nRet(%d) !!", pTitle, nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tm_pub_mrs_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(sPacket));
		KTC_MemClear(&rPacket, sizeof(rPacket));
#endif
	}

	return nRet;
}

/**
 * @brief		TmExp_TK_PubMrsMobile
 * @details		[티머니고속] 예매발권 : 모바일티켓 예매발권 tmax
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_PubMrsMobile(void)
{
	int nRet, nCount;
	char *pTitle = "티머니고속-모바일 예매발권";

	{	
		stk_tm_pub_mrs_htck_t sPacket;
		rtk_tm_pub_mrs_htck_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		nCount = 0;

		::ZeroMemory(&sPacket, sizeof(stk_tm_pub_mrs_htck_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_pub_mrs_htck_t));

		// [티머니고속] 예매발권: 모바일 예매발권 요청 - ui recv 데이타 복사
		::CopyMemory(&sPacket, 
					 &CMrnpTmExpMem::GetInstance()->m_ReqPubMobileMrs.rData, 
					 sizeof(stk_tm_pub_mrs_htck_t));

		nRet = m_pclsSvr->SVC_TK_PubMrsMobile((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("%s Fail, nRet(%d) !!", pTitle, nRet);
		}
		else
		{
			TR_LOG_OUT("%s Success, nRet(%d) !!", pTitle, nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tm_pub_mrs_htck_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(sPacket));
		KTC_MemClear(&rPacket, sizeof(rPacket));
#endif
	}

	return nRet;
}

/**
 * @brief		TmExp_TK_ReadBusTckno
 * @details		[티머니고속] 환불 - 승차권 정보 조회 tmax
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_ReadBusTckno(BOOL bFirst)
{
	int nRet, nCount;
	char *pTitle = "티머니고속-승차권 정보 조회";

	{	
		stk_tm_read_bus_tckno_t sPacket;
		rtk_tm_read_bus_tckno_t rPacket;
		CCancRyTkTmExpMem *pCancRy;
		
		pCancRy = CCancRyTkTmExpMem::GetInstance();

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		nCount = 0;

		::ZeroMemory(&sPacket, sizeof(stk_tm_read_bus_tckno_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_read_bus_tckno_t));

		TmExp_MakeHeader((char *)&sPacket, TRUE);

		/// 발권 일자
		::CopyMemory(sPacket.tissu_dt		, pCancRy->tReqInq.tissu_dt		, sizeof(sPacket.tissu_dt));
		/// 발권 터미널번호
		::CopyMemory(sPacket.tissu_trml_no	, pCancRy->tReqInq.tissu_trml_no, sizeof(sPacket.tissu_trml_no));
		/// 발권 창구번호
		::CopyMemory(sPacket.tissu_wnd_no	, pCancRy->tReqInq.tissu_wnd_no	, sizeof(sPacket.tissu_wnd_no));
		/// 발권 일련번호
		::CopyMemory(sPacket.tissu_sno		, pCancRy->tReqInq.tissu_sno	, sizeof(sPacket.tissu_sno));

		/// 작업구분코드
		sPacket.tak_dvs_cd[0] = 'R';
		/// 환불율구분코드
		sPacket.ryrt_dvs_cd[0] = '0';
		if(bFirst != TRUE)
		{
			Oper_GetTmExpRefundCode(CCancRyTkTmExpMem::GetInstance()->tBase.n_tot_disc_rate, sPacket.ryrt_dvs_cd);
		}

		TR_LOG_OUT("티머니고속-환불율 구분코드, BEFORE @@@@@ SVC_TK_ReadBusTckno() sPacket.ryrt_dvs_cd[0] : 0x%02X ", sPacket.ryrt_dvs_cd[0]);	// 20211018 ADD

		nRet = m_pclsSvr->SVC_TK_ReadBusTckno((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("%s Fail, nRet(%d) !!", pTitle, nRet);
		}
		else
		{
			TR_LOG_OUT("%s Success, nRet(%d) !!", pTitle, nRet);
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(sPacket));
		KTC_MemClear(&rPacket, sizeof(rPacket));
#endif
	}

	return nRet;
}

/**
 * @brief		TmExp_TK_CancRyTck
 * @details		[티머니고속] 환불 tmax
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_CancRyTck(void)
{
	int nRet, nCount;
	char *pTitle = "티머니고속-승차권 환불";

	{	
		stk_tm_cancrytck_t sPacket;
		rtk_tm_cancrytck_t rPacket;
		CCancRyTkTmExpMem *pCancRy;

		pCancRy = CCancRyTkTmExpMem::GetInstance();

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		nCount = 0;

		::ZeroMemory(&sPacket, sizeof(stk_tm_cancrytck_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_cancrytck_t));

		TmExp_MakeHeader((char *)&sPacket, TRUE);

		/// 요청구분코드 : 'C'(취소), 'R'(환불)
		sPacket.req_dvs_cd[0] = 'R';

		/// 정보 수
		nCount = 1;
		::CopyMemory(sPacket.rec_num, &nCount, sizeof(int));

		/// List
		{
			/// 발권 일자
			::CopyMemory(sPacket.List[0].tissu_dt, pCancRy->tReqInq.tissu_dt, sizeof(sPacket.List[0].tissu_dt));
			/// 발권 터미널번호
			::CopyMemory(sPacket.List[0].tissu_trml_no, pCancRy->tReqInq.tissu_trml_no, sizeof(sPacket.List[0].tissu_trml_no));
			/// 발권 창구번호
			::CopyMemory(sPacket.List[0].tissu_wnd_no, pCancRy->tReqInq.tissu_wnd_no, sizeof(sPacket.List[0].tissu_wnd_no));
			/// 발권 일련번호
			::CopyMemory(sPacket.List[0].tissu_sno, pCancRy->tReqInq.tissu_sno, sizeof(sPacket.List[0].tissu_sno));
			/// 환불율 구분코드 : 임시코드.....
			sPacket.List[0].ryrt_dvs_cd[0] = '0';
		}

		// 20211018 ADD
		Oper_GetTmExpRefundCode(CCancRyTkTmExpMem::GetInstance()->tBase.n_tot_disc_rate, sPacket.List[0].ryrt_dvs_cd);

		TR_LOG_OUT("티머니고속-환불율 구분코드, BEFORE @@@@@ SVC_TK_CancRyTck() sPacket.List[0].ryrt_dvs_cd[0] : 0x%02X ", sPacket.List[0].ryrt_dvs_cd[0]	);
		// 20211018 ~ADD

		nRet = m_pclsSvr->SVC_TK_CancRyTck((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("%s Fail, nRet(%d) !!", pTitle, nRet);
		}
		else
		{
			TR_LOG_OUT("%s Success, nRet(%d) !!", pTitle, nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tm_cancrytck_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(sPacket));
		KTC_MemClear(&rPacket, sizeof(rPacket));
#endif
	}

	return nRet;
}

/**
 * @brief		TmExp_TK_RePubTck
 * @details		[티머니고속] 재발행 tmax
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_RePubTck(void *pData)
{
	int nRet, nCount;
	char *pTitle = "티머니고속-재발행";

	{	
		int n_pub_sno = 0;
		stk_tm_rpub_tck_t sPacket;
		rtk_tm_rpub_tck_t rPacket;
		PUI_RESP_ADMIN_RE_ISSUE_T pResp;

		pResp = (PUI_RESP_ADMIN_RE_ISSUE_T) pData;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		nCount = 0;

		::ZeroMemory(&sPacket, sizeof(stk_tm_rpub_tck_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_rpub_tck_t));

		TmExp_MakeHeader((char *)&sPacket, TRUE);
		// [티머니고속] 재발행 요청 - ui recv 데이타 복사

		/// 1) 발행일자
		::CopyMemory(sPacket.tissu_dt, pResp->pub_dt, sizeof(sPacket.tissu_dt) - 1);
		/// 2) 발권단축터미널코드
		::CopyMemory(sPacket.tissu_trml_no, pResp->pub_shct_trml_cd, sizeof(sPacket.tissu_trml_no) - 1);
		/// 3) 발권창구번호
		::CopyMemory(sPacket.tissu_wnd_no, pResp->pub_wnd_no, sizeof(sPacket.tissu_wnd_no) - 1);
		/// 4) 발권일련번호
		n_pub_sno = *(int *) pResp->pub_sno;
		sprintf(sPacket.tissu_sno, "%04d", n_pub_sno);
		
		::CopyMemory(&CPubTckTmExpMem::GetInstance()->m_tReqRePubTck, &sPacket, sizeof(stk_tm_rpub_tck_t));

		nRet = m_pclsSvr->SVC_TK_RePubTck((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("%s Fail, nRet(%d) !!", pTitle, nRet);
		}
		else
		{
			TR_LOG_OUT("%s Success, nRet(%d) !!", pTitle, nRet);
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(sPacket));
		KTC_MemClear(&rPacket, sizeof(rPacket));
#endif
	}

	return nRet;
}

/**
 * @brief		TmExp_TK_PubPtInquiry
 * @details		[티머니고속] 발행내역 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_PubPtInquiry(char *pDate)
{
	int nRet, nCount, nOper;
	char *pTitle = "티머니고속-발권내역조회";

	nRet = nCount = nOper = 0;
	nOper = OPER_FILE_ID_EZ_TK_INQPUBPT;

	{	
		stk_tm_pub_inq_t sPacket;
		rtk_tm_pub_inq_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		nCount = 9999;

		::ZeroMemory(&sPacket, sizeof(stk_tm_pub_inq_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_pub_inq_t));

		TmExp_MakeHeader((char *)&sPacket, TRUE);

		::CopyMemory(sPacket.rec_num, &nCount, 4);

		sPacket.sort_dvs_cd[0] = 'A';	
		sprintf(sPacket.tak_dt, "%s", pDate);
		sprintf(sPacket.tak_time, "000000");
		::CopyMemory(sPacket.depr_trml_no, sPacket.req_trml_no, sizeof(sPacket.depr_trml_no));
		::CopyMemory(sPacket.tak_wnd_no, sPacket.req_wnd_no, sizeof(sPacket.tak_wnd_no));
		::CopyMemory(sPacket.tak_user_no, sPacket.req_user_no, sizeof(sPacket.tak_user_no));
		sprintf(sPacket.arvl_trml_no, "000");
		sprintf(sPacket.tissu_dt, "%s", pDate);
		::CopyMemory(sPacket.tissu_trml_no, sPacket.req_trml_no, sizeof(sPacket.tissu_trml_no));
		::CopyMemory(sPacket.tissu_wnd_no, sPacket.req_wnd_no, sizeof(sPacket.tissu_wnd_no));
		sprintf(sPacket.tissu_sno, "0000");
		sPacket.bef_aft_dvs[0] = 'D';

		nRet = m_pclsSvr->SVC_CM_PubInquiry(nOper, (char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("%s Fail, nRet(%d) !!", pTitle, nRet);
		}
		else
		{
			TR_LOG_OUT("%s Success, nRet(%d) !!", pTitle, nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tm_pub_inq_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(sPacket));
		KTC_MemClear(&rPacket, sizeof(rPacket));
#endif
	}

	return nRet;
}

/**
 * @brief		TmExp_TK_RePubPtInquiry
 * @details		[티머니고속] 재발행내역 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_RePubPtInquiry(char *pDate)
{
	int nRet, nCount, nOper;
	char *pTitle = "티머니고속-재발행내역조회";

	nOper = OPER_FILE_ID_EZ_TK_INQREPUBPT;

	{	
		stk_tm_rpub_inq_t sPacket;
		rtk_tm_rpub_inq_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		nCount = 9999;

		::ZeroMemory(&sPacket, sizeof(stk_tm_rpub_inq_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_rpub_inq_t));

		// [티머니고속] 재발행 요청 - ui recv 데이타 복사
		::ZeroMemory(&sPacket, sizeof(stk_tm_pub_inq_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_pub_inq_t));

		TmExp_MakeHeader((char *)&sPacket, TRUE);

		::CopyMemory(sPacket.rec_num, &nCount, 4);

		/// 정렬구분코드
		sPacket.sort_dvs_cd[0]	= 'A';
		/// 작업일자
		sprintf(sPacket.tak_dt, "%s", pDate);
		/// 작업시각
		sprintf(sPacket.tak_time, "000000");
		/// 작업 창구번호
		::CopyMemory(sPacket.tak_wnd_no, sPacket.req_wnd_no, sizeof(sPacket.tak_wnd_no));
		/// 작업 사용자번호
		::CopyMemory(sPacket.tak_user_no, sPacket.req_user_no, sizeof(sPacket.tak_user_no));
		/// 출발터미널 번호
		::CopyMemory(sPacket.depr_trml_no, sPacket.req_trml_no, sizeof(sPacket.depr_trml_no));
		/// 도착터미널 번호
		sprintf(sPacket.arvl_trml_no, "000");
		/// 발권일자
		sprintf(sPacket.tissu_dt, "%s", pDate);
		/// 발권터미널번호
		::CopyMemory(sPacket.tissu_trml_no, sPacket.req_trml_no, sizeof(sPacket.tissu_trml_no));
		/// 발권창구번호
		::CopyMemory(sPacket.tissu_wnd_no, sPacket.req_wnd_no, sizeof(sPacket.tissu_wnd_no));
		/// 발권일련번호
		sprintf(sPacket.tissu_sno, "0000");
		/// 이전이후 구분
		sPacket.bef_aft_dvs[0] = 'D';

		nRet = m_pclsSvr->SVC_CM_RePubInquiry(nOper, (char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("%s Fail, nRet(%d) !!", pTitle, nRet);
		}
		else
		{
			TR_LOG_OUT("%s Success, nRet(%d) !!", pTitle, nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tm_pub_inq_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(sPacket));
		KTC_MemClear(&rPacket, sizeof(rPacket));
#endif
	}

	return nRet;
}

/**
 * @brief		TmExp_TK_CanRyPtInquiry
 * @details		[티머니고속] 환불내역 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int TmExp_TK_CanRyPtInquiry(char *pDate)
{
	int nRet, nCount, nOper;
	char *pTitle = "티머니고속-환불내역조회";

	nRet = nCount = nOper = 0;
	nOper = OPER_FILE_ID_EZ_TK_INQCANRYPT;

	{	
		stk_tm_canry_inq_t sPacket;
		rtk_tm_canry_inq_t rPacket;

		if(m_pclsSvr == NULL)
		{
			TR_LOG_OUT(" m_pclsSvr = NULL !!");
			return -2;
		}

		nCount = 9999;

		::ZeroMemory(&sPacket, sizeof(stk_tm_canry_inq_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tm_canry_inq_t));

		TmExp_MakeHeader((char *)&sPacket, TRUE);

		::CopyMemory(sPacket.rec_num, &nCount, 4);

		sPacket.tak_dvs_cd[0]		= 'R';	 /// ????
		sPacket.sort_dvs_cd[0]		= 'A';
		sprintf(sPacket.tak_dt, "%s", pDate);
		sprintf(sPacket.tak_time, "000000");
		::CopyMemory(sPacket.tak_wnd_no, sPacket.req_wnd_no, sizeof(sPacket.tak_wnd_no));
		::CopyMemory(sPacket.tak_user_no, sPacket.req_user_no, sizeof(sPacket.tak_user_no));
		::CopyMemory(sPacket.depr_trml_no, sPacket.req_trml_no, sizeof(sPacket.depr_trml_no));
		sprintf(sPacket.arvl_trml_no, "000");
		sprintf(sPacket.tissu_dt, "%s", pDate);
		::CopyMemory(sPacket.tissu_trml_no, sPacket.req_trml_no, sizeof(sPacket.tissu_trml_no));
		::CopyMemory(sPacket.tissu_wnd_no, sPacket.req_wnd_no, sizeof(sPacket.tissu_wnd_no));
		sprintf(sPacket.tissu_sno, "0000");
		sPacket.bef_aft_dvs[0]		= 'D';


		nRet = m_pclsSvr->SVC_CM_CanRyInquiry(nOper, (char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("%s Fail, nRet(%d) !!", pTitle, nRet);
		}
		else
		{
			TR_LOG_OUT("%s Success, nRet(%d) !!", pTitle, nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tm_canry_inq_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(sPacket));
		KTC_MemClear(&rPacket, sizeof(rPacket));
#endif
	}

	return nRet;
}
