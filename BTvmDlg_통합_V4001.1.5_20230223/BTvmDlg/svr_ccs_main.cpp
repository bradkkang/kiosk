// 
// 
// svr_main.cpp : 시외버스 서버 main 소스
//

#include "stdafx.h"
#include <stdio.h>
#include <queue>
#include <iostream>
#include <string.h>
#include <fcntl.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "cmn_util.h"
#include "xzzbus_fdl.h"
#include "svr_main.h"
#include "svr_ccbus.h"
#include "svr_ccbus_st.h"
#include "svr_kobus.h"
#include "svr_kobus_st.h"
#include "dev_tr_main.h"
#include "dev_tr_mem.h"
#include "oper_config.h"
#include "oper_kos_file.h"
#include "damo_ktc.h"
#include "event_if.h"

//----------------------------------------------------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------------------------------------------------

static CCBusServer*  m_pclsSvr = NULL;			/// 시외버스 서버

static char tmpBuffer[1025];

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
 * @brief		MakeHeaderData
 * @details		
 * @param		None
 * @return		None
 */
static void MakeHeaderData(pstk_head_t pHdr)
{
	int nLen;
	char* pBuff;

	///< 요청 프로그램 구분
	{
		nLen = CheckBuffer(pBuff = GetPgmDVS(), sizeof(pHdr->req_pgm_dvs) - 1);
		::CopyMemory(pHdr->req_pgm_dvs, pBuff, nLen);	
	}
	
	///< 요청 터미널 코드
	{
		nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_CCBUS), sizeof(pHdr->req_trml_cd) - 1);
		::CopyMemory(pHdr->req_trml_cd, pBuff, nLen);	
	}
	
	///< 요청 창구번호
	{
		nLen = CheckBuffer(pBuff = GetTrmlWndNo(SVR_DVS_CCBUS), sizeof(pHdr->req_trml_wnd_no) - 1);
		::CopyMemory(pHdr->req_trml_wnd_no, pBuff, nLen);	
	}

	///< 요청 장비 IP
	{
		nLen = CheckBuffer(pBuff = GetMyIpAddress(), sizeof(pHdr->req_trml_user_ip) - 1);
		::CopyMemory(pHdr->req_trml_user_ip, pBuff, nLen);	
	}
}

static void MakeHeaderShortData(pstk_head_shrt_t pHdr)
{
	int nLen;
	char* pBuff;

	///< 요청 프로그램 구분
	{
		nLen = CheckBuffer(pBuff = GetPgmDVS(), sizeof(pHdr->req_pgm_dvs) - 1);
		::CopyMemory(pHdr->req_pgm_dvs, pBuff, nLen);	
	}
	///< 요청 터미널 코드
	{
		nLen = CheckBuffer(pBuff = GetShortTrmlCode(SVR_DVS_CCBUS), sizeof(pHdr->req_shct_trml_cd) - 1);
		::CopyMemory(pHdr->req_shct_trml_cd, pBuff, nLen);	
	}
	///< 요청 창구번호
	{
		nLen = CheckBuffer(pBuff = GetTrmlWndNo(SVR_DVS_CCBUS), sizeof(pHdr->req_trml_wnd_no) - 1);
		::CopyMemory(pHdr->req_trml_wnd_no, pBuff, nLen);	
	}
	///< 요청 장비 IP
	{
		nLen = CheckBuffer(pBuff = GetMyIpAddress(), sizeof(pHdr->req_trml_user_ip) - 1);
		::CopyMemory(pHdr->req_trml_user_ip, pBuff, nLen);	
	}
}

/**
 * @brief		MakeHeaderIdData
 * @details		TMAX 시외 헤더 패킷 생성
 * @param		pstk_head_id_t pHdr			헤더패킷
 * @return		성공 : >= 0, 실패 : < 0
 */
static void MakeHeaderIdData(pstk_head_id_t pHdr)
{
	int nLen;
	char* pBuff;

	///< 요청 프로그램 구분
	{
		nLen = CheckBuffer(pBuff = GetPgmDVS(), sizeof(pHdr->req_pgm_dvs) - 1);
		//TR_LOG_OUT(" @@@@@@@@@@ GetPgmDVS() pBuff(%s), nLen(%d) ", pBuff, nLen);
		::CopyMemory(pHdr->req_pgm_dvs, pBuff, nLen);	
	}
	///< 요청 터미널 코드
	{
		nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_CCBUS), sizeof(pHdr->req_trml_cd) - 1);
		::CopyMemory(pHdr->req_trml_cd, pBuff, nLen);	
	}
	///< 요청 창구번호
	{
		nLen = CheckBuffer(pBuff = GetTrmlWndNo(SVR_DVS_CCBUS), sizeof(pHdr->req_trml_wnd_no) - 1);
		::CopyMemory(pHdr->req_trml_wnd_no, pBuff, nLen);	
	}
	///< 요청 장비 IP
	{
		nLen = CheckBuffer(pBuff = GetMyIpAddress(), sizeof(pHdr->req_trml_user_ip) - 1);
		::CopyMemory(pHdr->req_trml_user_ip, pBuff, nLen);	
	}
	///< 요청 터미널 사용자 ID
	{
		nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->req_trml_user_id, sizeof(pHdr->req_trml_user_id) - 1);
		::CopyMemory(pHdr->req_trml_user_id, pBuff, nLen);	
	}
}

/**
 * @brief		Svr_IfSv_100
 * @details		공통 코드 조회 (TK_ReadCmnCd)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_100(char *retBuf)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readcmncd_t	sPacket;
		rtk_readcmncd_t	rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_readcbusmsg_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readcbusmsg_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc100((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_100() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_100() Success, nRet(%d) !!", nRet);
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_101
 * @details		접근 컴퓨터 인증 (TK_AuthCmpt)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_101(void)
{
	int nRet, nLen;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_authcmpt_t	sPacket;
		rtk_authcmpt_t	rPacket;
		char *pBuff;

		::ZeroMemory(&sPacket, sizeof(stk_authcmpt_t));
		::ZeroMemory(&rPacket, sizeof(rtk_authcmpt_t));

		MakeHeaderShortData(&sPacket.hdr_st);

		///< 조회 구분
		{
			::CopyMemory(sPacket.req_dvs, "2", 1);	
		}
		///< 접근 컴퓨터 정보
		{
			nLen = CheckBuffer(pBuff = GetMyMacAddress(), sizeof(sPacket.acs_cmpt_inf) - 1);
			::CopyMemory(sPacket.acs_cmpt_inf, pBuff, nLen);	
		}

		nRet = m_pclsSvr->TMaxSvc101((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_101() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_101() Success, nRet(%d) !!", nRet);
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_102
 * @details		공통코드 상세조회 (TK_CmnCdDtl)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_102(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_cmncddtl_t	sPacket;
		rtk_cmncddtl_t	rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_cmncddtl_t));
		::ZeroMemory(&rPacket, sizeof(rtk_cmncddtl_t));

		MakeHeaderData(&sPacket.hdr);
		
		nRet = m_pclsSvr->TMaxSvc102((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_102() Fail, nRet(%d) !!", nRet);
		}
		else
		{
// 			int nSize, nCount;
// 
 			TR_LOG_OUT("Svr_IfSv_102() Success, nRet(%d) !!", nRet);
// 
// 			nCount = nRet;
// 			nSize = sizeof(rtk_cmncddtl_t) - sizeof(char *) + (sizeof(rtk_cmncddtl_list_t) * nCount);
// 			
// 			nRet = OperWriteFile(OPER_FILE_ID_0102, (char *)&rPacket, nSize);
		}

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_106
 * @details		버스운수사코드 조회 (TK_ReadBusCacm)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_106(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readbuscacm_t	sPacket;
		rtk_readbuscacm_t	rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_readbuscacm_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readbuscacm_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc106((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_106() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_106() Success, nRet(%d) !!", nRet);
		}

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_107
 * @details		버스등급코드 조회 (TK_ReadBusCls)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_107(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readbuscls_t	sPacket;
		rtk_readbuscls_t	rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_readbuscls_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readbuscls_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc107((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_107() Fail, nRet(%d) !!", nRet);
		}
		else
		{
// 			int nSize, nCount;
// 
 			TR_LOG_OUT("Svr_IfSv_107() Success, nRet(%d) !!", nRet);
// 			nCount = nRet;
// 			nSize = sizeof(rtk_readbuscls_t) - sizeof(char *) + (sizeof(rtk_readbuscls_list_t) * nCount);
// 			nRet = OperWriteFile(OPER_FILE_ID_0107, (char *)&rPacket, nSize);

//			UI_AddFileInfo(107, OPER_FILE_ID_0107);

// 			{
// 				UI_BASE_FILE_INFO_T uiInfo;
// 
// 				::ZeroMemory(&uiInfo, sizeof(UI_BASE_FILE_INFO_T));
// 
// 				uiInfo.wFileID = (WORD) 107;
// 
// 				OperGetFileInfo(OPER_FILE_ID_0107, uiInfo.szFullName, (int *)&uiInfo.dwSize);
// 				UI_AddQueueInfo(UI_CMD_DEV_FILE_INFO, (char *)&uiInfo, sizeof(UI_BASE_FILE_INFO_T));
// 			}
		}

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_108
 * @details		버스티켓종류코드 조회 (TK_ReadTckKnd)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_108(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readtckknd_t	sPacket;
		rtk_readtckknd_t	rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_readtckknd_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readtckknd_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc108((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_108() Fail, nRet(%d) !!", nRet);
		}
		else
		{
 			TR_LOG_OUT("Svr_IfSv_108() Success, nRet(%d) !!", nRet);
		}

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_114
 * @details		사용자 기본 조회 (TK_ReadUserBsc)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_114(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readuserbsc_t	sPacket;
		rtk_readuserbsc_t	rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_readuserbsc_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readuserbsc_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc114((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_114() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			int nSize, nCount;

			TR_LOG_OUT("Svr_IfSv_114() Success, nRet(%d) !!", nRet);
			nCount = nRet;

			nSize = sizeof(rtk_readuserbsc_t) - sizeof(char *) + (sizeof(rtk_readuserbsc_list_t) * nCount);

			nRet = OperWriteFile(OPER_FILE_ID_0114, (char *)&rPacket, nSize);
		}

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_118
 * @details		터미널 상세 조회 (TK_TrmlWndDtl)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_118(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_trmwnddtl_t	sPacket;
		rtk_trmwnddtl_t	rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_trmwnddtl_t));
		::ZeroMemory(&rPacket, sizeof(rtk_trmwnddtl_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc118((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_118() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_118() Success, nRet(%d) !!", nRet);
		}

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_119
 * @details		컴퓨터 설정 상세 조회 (TK_CmptStupDtl)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_119(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_cmptstupdtl_t	sPacket;
		rtk_cmptstupdtl_t	rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_cmptstupdtl_t));
		::ZeroMemory(&rPacket, sizeof(rtk_cmptstupdtl_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc119((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_119() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_119() Success, nRet(%d) !!", nRet);
		}

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}


/**
 * @brief		Svr_IfSv_120
 * @details		버스티켓종류코드 조회 (TK_ReadTrmlCd)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_120(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readtrmlcd_t	sPacket;
		rtk_readtrmlcd_t	rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_readtrmlcd_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readtrmlcd_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc120((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_120() Fail, nRet(%d) !!", nRet);
		}
		else
		{
 			TR_LOG_OUT("Svr_IfSv_120() Success, nRet(%d) !!", nRet);
		}

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_122
 * @details		터미널 설정 상세 조회 (TK_TrmlStupDtl)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_122(void)
{
	int nRet;//, nLen;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_trmlstupdtl_t	sPacket;
		rtk_trmlstupdtl_t	rPacket;
//		char *pBuff;

		::ZeroMemory(&sPacket, sizeof(stk_trmlstupdtl_t));
		::ZeroMemory(&rPacket, sizeof(rtk_trmlstupdtl_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc122((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_122() Fail, nRet(%d) !!", nRet);
		}
		else
		{
// 			int nSize, nCount;
// 
 			TR_LOG_OUT("Svr_IfSv_122() Success, nRet(%d) !!", nRet);
// 
// 			nCount = nRet;
// 
// 			nSize = sizeof(rtk_trmlstupdtl_t) - sizeof(char *) + (sizeof(rtk_trmlstupdtl_list_t) * nCount);
// 
// 			OperWriteFile(OPER_FILE_ID_0122, (char *)&rPacket, nSize);
		}

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_124
 * @details		노선 기본 조회 (TK_ReadRotBsc)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_124(void)
{
	int nRet;//, nLen;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readrotbsc_t	sPacket;
		rtk_readrotbsc_t	rPacket;
//		char *pBuff;

		::ZeroMemory(&sPacket, sizeof(stk_readrotbsc_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readrotbsc_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc124((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_124() Fail, nRet(%d) !!", nRet);
		}
		else
		{
// 			int nSize, nCount;
// 
 			TR_LOG_OUT("Svr_IfSv_124() Success, nRet(%d) !!", nRet);
// 
// 			nCount = nRet;
// 
// 			nSize = sizeof(rtk_readrotbsc_t) - sizeof(char *) + (sizeof(rtk_readrotbsc_list_t) * nCount);
// 
// 			OperWriteFile(OPER_FILE_ID_0124, (char *)&rPacket, nSize);
		}

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_125
 * @details		노선 상세 조회 (TK_ReadRotDtl)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_125(void)
{
	int nRet;//, nLen;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readrotdtl_t	sPacket;
		rtk_readrotdtl_t	rPacket;
//		char *pBuff;

		::ZeroMemory(&sPacket, sizeof(stk_readrotdtl_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readrotdtl_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc125((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_125() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_125() Success, nRet(%d) !!", nRet);
		}

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_126
 * @details		노선 요금 상세 조회 (TK_RotFeeDtl)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_126(void)
{
	int nRet;//, nLen;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_rotfeedtl_t		sPacket;
		rtk_rotfeedtl_t		rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_rotfeedtl_t));
		::ZeroMemory(&rPacket, sizeof(rtk_rotfeedtl_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc126((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_126() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_126() Success, nRet(%d) !!", nRet);
		}

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_127
 * @details		버스티켓 고유번호 조회 (TK_ReadTckNo)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_127(void)
{
	int nRet;//, nLen;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readtckno_t	sPacket;
		rtk_readtckno_t	rPacket;
//		char *pBuff;

		::ZeroMemory(&sPacket, sizeof(stk_readtckno_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readtckno_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc127((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_127() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_127() Success, nRet(%d) !!", nRet);

			// *
			//Transact_SetData((DWORD)BUS_TCK_INHR_NO, rPacket.bus_tck_inhr_no, sizeof(rPacket.bus_tck_inhr_no) - 1);
			//Transact_SetData((DWORD)ADD_BUS_TCK_INHR_NO, rPacket.add_bus_tck_inhr_no, sizeof(rPacket.add_bus_tck_inhr_no) - 1);
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_128
 * @details		로그인 (TK_Login)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_128(void)
{
	int nRet, nLen;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_login_t	sPacket;
		rtk_login_t	rPacket;
		char *pBuff;
		PTERMINAL_INFO_T pThrmlInfo;

		pThrmlInfo = (PTERMINAL_INFO_T) GetConfigTrmlInfo(SVR_DVS_CCBUS);

		::ZeroMemory(&sPacket, sizeof(stk_login_t));
		::ZeroMemory(&rPacket, sizeof(rtk_login_t));

		MakeHeaderData(&sPacket.hdr);

		///< 사용자 번호
		{
			nLen = CheckBuffer(pBuff = GetLoginUserNo(SVR_DVS_CCBUS), sizeof(sPacket.user_no) - 1);
			::CopyMemory(sPacket.user_no, pBuff, nLen);	
		}
		///< 요청 터미널 사용자 비밀번호
		{
			nLen = CheckBuffer(pBuff = GetLoginUserPwd(SVR_DVS_CCBUS), sizeof(sPacket.req_trml_user_pwd) - 1);
			::CopyMemory(sPacket.req_trml_user_pwd, pBuff, nLen);	
		}
		///< 버스티켓 고유번호
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->bus_tck_inhr_no, sizeof(sPacket.bus_tck_inhr_no) - 1);
			::CopyMemory(sPacket.bus_tck_inhr_no, pBuff, nLen);
		}
		///< 추가 버스티켓 고유번호
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->add_bus_tck_inhr_no, sizeof(sPacket.add_bus_tck_inhr_no) - 1);
			::CopyMemory(sPacket.add_bus_tck_inhr_no, pBuff, nLen);
		}

		nRet = m_pclsSvr->TMaxSvc128((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_128() Fail, nRet(%d) !!", nRet);
			if(nRet == -99)
			{	// 자동 로그아웃된 경우임. 다시 로그인한다. 
				nRet = m_pclsSvr->TMaxSvc128((char *)&sPacket, (char *)&rPacket);
			}
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_128() Success, nRet(%d) !!", nRet);
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_129
 * @details		알림 조회 (TK_ReadTrmlCd)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_129(void)
{
	int nRet;//, nLen;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readntfc_t	sPacket;
		rtk_readntfc_t	rPacket;
//		char *pBuff;

		::ZeroMemory(&sPacket, sizeof(stk_readntfc_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readntfc_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc129((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_129() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			int nSize, nCount;
			TR_LOG_OUT("Svr_IfSv_129() Success, nRet(%d) !!", nRet);

			nCount = nRet;

			nSize = sizeof(rtk_readntfc_t) - sizeof(char *) + (sizeof(rtk_readntfc_list_t) * nCount);

			OperWriteFile(OPER_FILE_ID_0129, (char *)&rPacket, nSize);
		}

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_200
 * @details		시외버스 지역코드 조회 (TK_ReadAreaCd)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_200(void)
{
	int nRet;//, nLen;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readareacd_t	sPacket;
		rtk_readareacd_t	rPacket;
//		char *pBuff;

		::ZeroMemory(&sPacket, sizeof(stk_readareacd_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readareacd_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc200((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_200() Fail, nRet(%d) !!", nRet);
		}
		else
		{
 			TR_LOG_OUT("Svr_IfSv_200() Success, nRet(%d) !!", nRet);
		}	

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	TR_LOG_OUT("Svr_IfSv_200(), nRet(%d) !!", nRet);

	return nRet;
}

/**
 * @brief		Svr_IfSv_201
 * @details		왕복가능 터미널 조회 (TK_ReadRtrpTrml)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_201(void)
{
	int nRet;//, nLen;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readrtrptrml_t	sPacket;
		rtk_readrtrptrml_t	rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_readrtrptrml_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readrtrptrml_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc201((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_201() Fail, nRet(%d) !!", nRet);
		}
		else
		{
 			TR_LOG_OUT("Svr_IfSv_201() Success, nRet(%d) !!", nRet);
		}	

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	TR_LOG_OUT("Svr_IfSv_201(), nRet(%d) !!", nRet);

	return nRet;
}

/**
 * @brief		Svr_IfSv_208
 * @details		시외버스 메시지 코드 조회 (TK_ReadCBusMsg)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_208(void)
{
	int nRet;//, nLen;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readcbusmsg_t	sPacket;
		rtk_readcbusmsg_t	rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_readcbusmsg_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readcbusmsg_t));

		MakeHeaderData(&sPacket.hdr);

		nRet = m_pclsSvr->TMaxSvc208((char *)&sPacket, (char *)&rPacket);

		if( rPacket.pList != NULL )
		{
			delete[] rPacket.pList; 
			rPacket.pList = NULL;
		}
	}

	TR_LOG_OUT("Svr_IfSv_208(), nRet(%d) !!", nRet);

	return nRet;
}

/**
 * @brief		Svr_IfSv_209
 * @details		시외버스 메시지 코드 조회 (TK_ReadCBusMsg)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_209(void)
{
	int nRet, nLen;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readptrgtck_t	sPacket;
		rtk_readptrgtck_t	rPacket;
		char *pBuff;

		::ZeroMemory(&sPacket, sizeof(stk_readptrgtck_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readptrgtck_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 시외버스 프린터 종류 코드
		{
			nLen = CheckBuffer(pBuff = GetTckPrinterType(), sizeof(sPacket.cty_bus_ptr_knd_cd) - 1);
			::CopyMemory(sPacket.cty_bus_ptr_knd_cd, pBuff, nLen);	
		}

		nRet = m_pclsSvr->TMaxSvc209((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_209() Failure, nRet(%d) !!", nRet);
		}
		else
		{
// 			int nSize, nCount;
// 
 			TR_LOG_OUT("Svr_IfSv_209() Success, nRet(%d) !!", nRet);
// 
// 			nCount = nRet;
// 
// 			nSize = sizeof(rtk_readptrgtck_t) - sizeof(char *) + (sizeof(rtk_readptrgtck_list_t) * nCount);
// 
// 			OperWriteFile(OPER_FILE_ID_0209, (char *)&rPacket, nSize);
		}
	}

	TR_LOG_OUT("Svr_IfSv_209(), nRet(%d) !!", nRet);

	return nRet;
}

/**
 * @brief		Svr_IfSv_130
 * @details		배차조회 (TK_ReadAlcn)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_130(void)
{
	int nRet, nLen;

	TR_LOG_OUT(" start(배차조회) !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readalcn_t	sPacket;
		rtk_readalcn_t	rPacket;
		SYSTEMTIME stCurrDT;
		char *pBuff;
		PTERMINAL_INFO_T pThmlInfo;
		CPubTckMem* pTr;

		GetLocalTime(&stCurrDT);
		
		pThmlInfo = (PTERMINAL_INFO_T) GetConfigTrmlInfo(SVR_DVS_CCBUS);

		// 현장발권 class
		pTr = CPubTckMem::GetInstance();

		::ZeroMemory(&sPacket, sizeof(stk_readalcn_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readalcn_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		::ZeroMemory(sPacket.hdr_id.req_pgm_dvs, sizeof(sPacket.hdr_id.req_pgm_dvs));
		sprintf(sPacket.hdr_id.req_pgm_dvs, "TKT");

		///< 출발터미널코드
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->trml_cd, sizeof(sPacket.depr_trml_cd) - 1);
			::CopyMemory(sPacket.depr_trml_cd, pBuff, nLen);
		}
		
		///< 도착터미널코드
		{
			nLen = CheckBuffer(pBuff = pTr->base.arvl_trml_cd, sizeof(sPacket.arvl_trml_cd) - 1);
			::CopyMemory(sPacket.arvl_trml_cd, pBuff, nLen);
		}

		///< 요청 조회 대상 일자
		{
			nLen = CheckBuffer(pBuff = pTr->base.read_dt, sizeof(sPacket.read_dt) - 1);
			::CopyMemory(sPacket.read_dt, pBuff, nLen);
		}

		///< 요청 조회 대상 시각
		{
			nLen = CheckBuffer(pBuff = pTr->base.read_time, sizeof(sPacket.read_time) - 1);
			::CopyMemory(sPacket.read_time, pBuff, nLen);
		}
		
		///< 버스등급코드
		{
			nLen = CheckBuffer(pBuff = pTr->base.bus_cls_cd, sizeof(sPacket.bus_cls_cd) - 1);
			::CopyMemory(sPacket.bus_cls_cd, pBuff, nLen);
		}

		///< 이전이후구분
		{
			::CopyMemory(sPacket.bef_aft_dvs, "D", sizeof(sPacket.bef_aft_dvs) - 1);	
		}

		///< 요청 레코드 수
		{
			nLen = 99999;
			::CopyMemory(sPacket.req_rec_num, &nLen, 4);
		}

		nRet = m_pclsSvr->TMaxSvc130((char *)&sPacket, (char *)&rPacket);
		if(nRet <= 0)
		{
			TR_LOG_OUT("Svr_IfSv_130() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_130() Success, nRet(%d) !!", nRet);

			// 배차 리스트 저장
			//Transact_SetData((DWORD) CFG_RSP_ALCN_DATA, (char *)rPacket.pList, nRet);

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
 * @brief		Svr_IfSv_131
 * @details		배차요금조회 (TK_ReadAlcnFee)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_131(void)
{
	int nRet, nLen;

	TR_LOG_OUT(" start(배차요금조회) !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readalcnfee_t	sPacket;
		rtk_readalcnfee_t	rPacket;
		SYSTEMTIME stCurrDT;
		char *pBuff;
		PTERMINAL_INFO_T pThmlInfo;
		CPubTckMem* pTr;

		pTr = CPubTckMem::GetInstance();

		GetLocalTime(&stCurrDT);
		pThmlInfo = (PTERMINAL_INFO_T) GetConfigTrmlInfo(SVR_DVS_CCBUS);

		::ZeroMemory(&sPacket, sizeof(stk_readalcnfee_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readalcnfee_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 노선ID	
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_id, sizeof(sPacket.rot_id) - 1);
			::CopyMemory(sPacket.rot_id, pBuff, nLen);
		}
		///< 노선순번
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_sqno, sizeof(sPacket.rot_sqno) - 1);
			::CopyMemory(sPacket.rot_sqno, pBuff, nLen);
		}
		///< 배차일자
		{
			nLen = CheckBuffer(pBuff = pTr->base.alcn_dt, sizeof(sPacket.alcn_dt) - 1);
			::CopyMemory(sPacket.alcn_dt, pBuff, strlen(pBuff));
		}
		///< 배차순번
		{
			//nLen = CheckBuffer(pBuff = pTr->base.alcn_sqno, sizeof(sPacket.alcn_sqno) - 1);
			::CopyMemory(sPacket.alcn_sqno, pTr->base.alcn_sqno, 4);
		}
		///< 출발터미널코드
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->trml_cd, sizeof(sPacket.depr_trml_cd) - 1);
			::CopyMemory(sPacket.depr_trml_cd, pBuff, nLen);
		}
		///< 도착터미널코드
		{
			nLen = CheckBuffer(pBuff = pTr->base.arvl_trml_cd, sizeof(sPacket.arvl_trml_cd) - 1);
			::CopyMemory(sPacket.arvl_trml_cd, pBuff, nLen);
		}

		nRet = m_pclsSvr->TMaxSvc131((char *)&sPacket, (char *)&rPacket);
		if(nRet <= 0)
		{
			TR_LOG_OUT("Svr_IfSv_131() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_131() Success, nRet(%d) !!", nRet);
			
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
 * @brief		Svr_IfSv_202
 * @details		현장발권 - 좌석 선점/해제 (TK_PcpySats)
 * @param		BOOL bSats		선점여부 Flag
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_202(BOOL bSats)
{
	int nRet, nLen, i, nCount;

	TR_LOG_OUT(" start(좌석 선점/해제) !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_pcpysats_t	sPacket;
		rtk_pcpysats_t	rPacket;
		SYSTEMTIME stCurrDT;
		char *pBuff; 
		CPubTckMem* pTr;

		/// 현장발권 mem info
		pTr = CPubTckMem::GetInstance();

		GetLocalTime(&stCurrDT);

		::ZeroMemory(&sPacket, sizeof(stk_pcpysats_t));
		::ZeroMemory(&rPacket, sizeof(rtk_pcpysats_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 선점구분 (1 : 선점 / 2: 해제)
		if(bSats == TRUE)
		{
			sprintf(sPacket.sats_pcpy_dvs, "%s", "1");
		}
		else
		{
			sprintf(sPacket.sats_pcpy_dvs, "%s", "2");
		}
		///< 노선ID
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_id, sizeof(sPacket.rot_id) - 1);
			::CopyMemory(sPacket.rot_id, pBuff, nLen);	
		}
		///< 노선순번(number)
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_sqno, sizeof(sPacket.rot_sqno) - 1);
			::CopyMemory(sPacket.rot_sqno, pBuff, nLen);	
		}
		///< 배차일자
		{
			nLen = CheckBuffer(pBuff = pTr->base.alcn_dt, sizeof(sPacket.alcn_dt) - 1);
			::CopyMemory(sPacket.alcn_dt, pBuff, nLen);	
		}
		///< 배차순번
		{
// 			nLen = CheckBuffer(pBuff = pTr->base.alcn_sqno, sizeof(sPacket.alcn_sqno) - 1);
// 			::CopyMemory(sPacket.alcn_sqno, pBuff, nLen);	
			::CopyMemory(sPacket.alcn_sqno, pTr->base.alcn_sqno, 4);
		}
		///< 출발터미널코드
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->trml_cd, sizeof(sPacket.depr_trml_cd) - 1);
			::CopyMemory(sPacket.depr_trml_cd, pBuff, nLen);
		}
		///< 도착터미널코드
		{
			nLen = CheckBuffer(pBuff = pTr->base.arvl_trml_cd, sizeof(sPacket.arvl_trml_cd) - 1);
			::CopyMemory(sPacket.arvl_trml_cd, pBuff, nLen);
		}

		if(bSats == TRUE)
		{	// 좌석 선점
			vector<UI_SATS_T>::iterator iter;

			nCount = pTr->m_vtUiSats.size();

			///< 좌석 선점 수
			::CopyMemory(sPacket.sats_pcpy_num, &nCount, 4);
			iter = CPubTckMem::GetInstance()->m_vtUiSats.begin();
			for(i = 0; i < nCount; i++)
			{
				///< 좌석선점id (선점시에만 보냄)	
				//memset(sPacket.List[i].sats_pcpy_id, 0x20, sizeof(sPacket.List[i].sats_pcpy_id) - 1);	
				sPacket.List[i].sats_pcpy_id[0] = 0x20;
				///< 시외버스할인종류코드	
				sPacket.List[i].cty_bus_dc_knd_cd[0] = iter->cty_bus_dc_knd_cd[0];
				///< 할인율구분코드		
				sPacket.List[i].dcrt_dvs_cd[0] = iter->dcrt_dvs_cd[0];	
				///< 왕복출발일자			
				memset(sPacket.List[i].rtrp_depr_dt, 0x00, sizeof(sPacket.List[i].rtrp_depr_dt) - 1);	
				///< 버스티켓종류코드		
				::CopyMemory(sPacket.List[i].bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(sPacket.List[i].bus_tck_knd_cd) - 1);	
				///< 좌석번호			
				::CopyMemory(sPacket.List[i].sats_no, iter->sats_no, sizeof(sPacket.List[i].sats_no) - 1);	

				iter++;
			}
		}
		else
		{	// 좌석 선점 해제
			vector<rtk_pcpysats_list_t>::iterator iter;

			nCount = pTr->m_vtPcpysats.size();

			///< 좌석 선점 수
			::CopyMemory(sPacket.sats_pcpy_num, &nCount, sizeof(WORD));
			iter = CPubTckMem::GetInstance()->m_vtPcpysats.begin();
			for(i = 0; i < nCount; i++)
			{
				///< 좌석선점id	
				::CopyMemory(sPacket.List[i].sats_pcpy_id, iter->sats_pcpy_id, sizeof(sPacket.List[i].sats_pcpy_id) - 1);	
				///< 시외버스할인종류코드	
				sPacket.List[i].cty_bus_dc_knd_cd[0] = iter->cty_bus_dc_knd_cd[0];
				///< 할인율구분코드		
				sPacket.List[i].dcrt_dvs_cd[0] = iter->dcrt_dvs_cd[0];	
				///< 왕복출발일자			
				memset(sPacket.List[i].rtrp_depr_dt, 0x00, sizeof(sPacket.List[i].rtrp_depr_dt) - 1);	
				///< 버스티켓종류코드		
				::CopyMemory(sPacket.List[i].bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(sPacket.List[i].bus_tck_knd_cd) - 1);	
				///< 좌석번호			
				::CopyMemory(sPacket.List[i].sats_no, iter->sats_no, sizeof(sPacket.List[i].sats_no) - 1);
				iter++;
			}
		}

		nRet = m_pclsSvr->TMaxSvc202((char *)&sPacket, (char *)&rPacket);
		if(nRet <= 0)
		{
			TR_LOG_OUT("TMaxSvc202() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("TMaxSvc202() Success, nRet(%d) !!", nRet);
			
			// 좌석 선점/해제 리스트 저장
			//Transact_SetData((DWORD) CFG_RSP_PCPYSATS_DATA, (char *)rPacket.pList, nRet);

			if( rPacket.pList != NULL )
			{
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
	}

	return nRet;
}

// 20221205 ADD~
/**
 * @brief		Svr_IfSv_278
 * @details		현장발권 - QR 좌석 선점 변경 (TK_QrMdPcpySats)
 * @param		NONE
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_278(void)
{
	int nRet, nLen, nCount;
	int i = 0;

	TR_LOG_OUT(" start(QR 좌석 선점 변경) !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_pcpysats_t	sPacket;
		rtk_pcpysats_t	rPacket;
		SYSTEMTIME stCurrDT;
		char *pBuff; 
		CPubTckMem* pTr;

		/// 현장발권 mem info
		pTr = CPubTckMem::GetInstance();

		GetLocalTime(&stCurrDT);

		::ZeroMemory(&sPacket, sizeof(stk_pcpysats_t));
		::ZeroMemory(&rPacket, sizeof(rtk_pcpysats_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		// QR 결제인 경우, 좌석 선점 시간을 연장(기존 창구/무인 좌석선점 3분 / QR 결제 좌석선점 : 10분)
		{
			stk_qrmdpcpysats_t	sQrPacket;
			rtk_qrmdpcpysats_t	rQrPacket;

			::ZeroMemory(&sQrPacket, sizeof(stk_qrmdpcpysats_t));
			::ZeroMemory(&rQrPacket, sizeof(rtk_qrmdpcpysats_t));

			MakeHeaderIdData(&sQrPacket.hdr_id);

			// 좌석 선점 수
			nCount = pTr->m_vtPcpysats.size();
			::CopyMemory(sQrPacket.sats_pcpy_num, &nCount, 4);

			// 좌석선점ID
			vector<rtk_pcpysats_list_t>::iterator	iter;

			for( iter = pTr->m_vtPcpysats.begin(); iter != pTr->m_vtPcpysats.end(); iter++ )
			{
				// 좌석선점ID
				//nSize += Util_MyCopyMemory(&retBuf[nSize], iter->sats_pcpy_id, sizeof(iter->sats_pcpy_id) - 1);

				nLen = CheckBuffer(pBuff = iter->sats_pcpy_id, sizeof(iter->sats_pcpy_id) - 1);
				::CopyMemory(sQrPacket.List[i++].sats_pcpy_id, pBuff, nLen);
			}
			
			nRet = m_pclsSvr->TMaxSvc278((char *)&sQrPacket, (char *)&rQrPacket);
			if(nRet <= 0)
			{
				TR_LOG_OUT("TMaxSvc278() Fail, nRet(%d) !!", nRet);
			}
			else
			{
				TR_LOG_OUT("TMaxSvc278() Success, nRet(%d) !!", nRet);
			
				//if( rQrPacket.pList != NULL )
				//{
				//	delete[] rQrPacket.pList;
				//	rQrPacket.pList = NULL;
				//}
			}

		}
	}

	return nRet;
}
// 20221205 ~ADD

/**
 * @brief		Svr_IfSv_501
 * @details		암호화키 전송 (SV_TrmEncKey)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_501(void)
{
	int nRet;//, i, nLen;

	TR_LOG_OUT(" start(암호화키 전송) !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		ssv_trmenckey_t	sPacket;
		rsv_trmenckey_t	rPacket;
		//char *pBuff; 

		::ZeroMemory(&sPacket, sizeof(ssv_trmenckey_t));
		::ZeroMemory(&rPacket, sizeof(rsv_trmenckey_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		nRet = m_pclsSvr->TMaxSvc501((char *)&sPacket, (char *)&rPacket);
		if(nRet <= 0)
		{
			TR_LOG_OUT("Svr_IfSv_501() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_501() Success, nRet(%d) !!", nRet);
			
			// 암호화 키 데이타 저장(*)
			//Transact_SetData((DWORD) CFG_RSP_TRMENCKEY_DATA, (char *)&rPacket, sizeof(rsv_trmenckey_t));
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_132
 * @details		현장발권 - 현금 승차권 발행 (TK_PubTckCash)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_132(void)
{
	int nRet, i, nLen, nCount;

	TR_LOG_OUT(" start(승차권 발행 - 현금) !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_pubtckcash_t	sPacket;
		rtk_pubtckcash_t	rPacket;
		SYSTEMTIME stCurrDT;
		char *pBuff; 
		CPubTckMem* pTr;

		/// 현장발권 mem info
		pTr = CPubTckMem::GetInstance();

		GetLocalTime(&stCurrDT);

		//pList = (pstk_pcpysats_list_t) pData;

		::ZeroMemory(&sPacket, sizeof(stk_pubtckcash_t));
		::ZeroMemory(&rPacket, sizeof(rtk_pubtckcash_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 노선ID
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_id, sizeof(sPacket.rot_id) - 1);
			::CopyMemory(sPacket.rot_id, pBuff, nLen);
		}
		///< 노선순번
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_sqno, sizeof(sPacket.rot_sqno) - 1);
			::CopyMemory(sPacket.rot_sqno, pBuff, nLen);
		}
		///< 배차일자
		{
			nLen = CheckBuffer(pBuff = pTr->base.alcn_dt, sizeof(sPacket.alcn_dt) - 1);
			::CopyMemory(sPacket.alcn_dt, pBuff, nLen);
		}
		///< 배차순번
		{
// 			nLen = CheckBuffer(pBuff = pTr->base.alcn_sqno, sizeof(sPacket.alcn_sqno) - 1);
// 			::CopyMemory(sPacket.alcn_sqno, pBuff, nLen);
			::CopyMemory(sPacket.alcn_sqno, pTr->base.alcn_sqno, 4);
		}
		///< 출발터미널코드
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->trml_cd, sizeof(sPacket.depr_trml_cd) - 1);
			::CopyMemory(sPacket.depr_trml_cd, pBuff, nLen);
		}
		///< 도착터미널코드
		{
			nLen = CheckBuffer(pBuff = pTr->base.arvl_trml_cd, sizeof(sPacket.arvl_trml_cd) - 1);
			::CopyMemory(sPacket.arvl_trml_cd, pBuff, nLen);
		}
		
		///< 지불수단구분코드	
		{
			nLen = CheckBuffer(pBuff = pTr->base.pyn_mns_dvs_cd, sizeof(sPacket.pyn_mns_dvs_cd) - 1);
			::CopyMemory(sPacket.pyn_mns_dvs_cd, pBuff, nLen);
		}
		///< 발행채널구분코드		
		{
			sPacket.pub_chnl_dvs_cd[0] = 'K';	
		}
		///< 발권방식구분코드		
		{
			sPacket.tisu_way_dvs_cd[0] = 'N';	
		}

		///< 사용자구분번호 (상주직원일 경우만 값이 있음)
		{
			if( strlen(pTr->base.user_dvs_no) <= 0 )
			{
				//memset(sPacket.user_dvs_no, 0x20, sizeof(sPacket.user_dvs_no) - 1);
				sPacket.user_dvs_no[0] = 0x20;
			}
			else
			{
				nLen = CheckBuffer(pBuff = pTr->base.user_dvs_no, sizeof(sPacket.user_dvs_no) - 1);
				::CopyMemory(sPacket.user_dvs_no, pBuff, nLen);
			}
		}

		///< *발행매수			
		{
			nCount = pTr->m_vtPcpysats.size();
			::CopyMemory(sPacket.pub_num, &nCount, sizeof(int));
		}

		///< 승차권 개별 발행 정보
		{
			vector<rtk_pcpysats_list_t>::iterator iter;

			iter = CPubTckMem::GetInstance()->m_vtPcpysats.begin();
			for(i = 0; i < nCount; i++)
			{
				/// 버스티켓종류코드
				::CopyMemory(sPacket.List[i].bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(sPacket.List[i].bus_tck_knd_cd) - 1);
				/// 좌석 선점ID
				::CopyMemory(sPacket.List[i].sats_pcpy_id, iter->sats_pcpy_id, sizeof(sPacket.List[i].sats_pcpy_id) - 1);
				/// 좌석번호
				::CopyMemory(sPacket.List[i].sats_no, iter->sats_no, sizeof(sPacket.List[i].sats_no) - 1);

				iter++;
			}
		}

		nRet = m_pclsSvr->TMaxSvc132((char *)&sPacket, (char *)&rPacket);
		if(nRet <= 0)
		{
			TR_LOG_OUT("Svr_IfSv_132() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_132() Success, nRet(%d) !!", nRet);
			
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
 * @brief		Svr_IfSv_134
 * @details		현장발권 - 현금영수증 승차권 발행 (TK_PubTckCsrc)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_134(void)
{
	int nRet, i, nLen, nCount;

	TR_LOG_OUT(" start(승차권 발행 - 현금영수증) !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_pubtckcsrc_t	sPacket;
		rtk_pubtckcsrc_t	rPacket;
		char *pBuff; 
		CPubTckMem* pTr;

		/// 현장발권 mem info
		pTr = CPubTckMem::GetInstance();

		::ZeroMemory(&sPacket, sizeof(stk_pubtckcsrc_t));
		::ZeroMemory(&rPacket, sizeof(rtk_pubtckcsrc_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 노선ID
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_id, sizeof(sPacket.rot_id) - 1);
			::CopyMemory(sPacket.rot_id, pBuff, nLen);
		}
		///< 노선순번
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_sqno, sizeof(sPacket.rot_sqno) - 1);
			::CopyMemory(sPacket.rot_sqno, pBuff, nLen);
		}
		///< 배차일자
		{
			nLen = CheckBuffer(pBuff = pTr->base.alcn_dt, sizeof(sPacket.alcn_dt) - 1);
			::CopyMemory(sPacket.alcn_dt, pBuff, nLen);
		}
		///< 배차순번
		{
// 			nLen = CheckBuffer(pBuff = pTr->base.alcn_sqno, sizeof(sPacket.alcn_sqno) - 1);
// 			::CopyMemory(sPacket.alcn_sqno, pBuff, nLen);
			::CopyMemory(sPacket.alcn_sqno, pTr->base.alcn_sqno, 4);
		}
		///< 출발터미널코드
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->trml_cd, sizeof(sPacket.depr_trml_cd) - 1);
			::CopyMemory(sPacket.depr_trml_cd, pBuff, nLen);
		}
		///< 도착터미널코드
		{
			nLen = CheckBuffer(pBuff = pTr->base.arvl_trml_cd, sizeof(sPacket.arvl_trml_cd) - 1);
			::CopyMemory(sPacket.arvl_trml_cd, pBuff, nLen);
		}
		
		///< 발권방식구분코드		
		{
			sPacket.tisu_way_dvs_cd[0] = 'N';	
		}
		///< 지불수단구분코드	
		{
			nLen = CheckBuffer(pBuff = pTr->base.pyn_mns_dvs_cd, sizeof(sPacket.pyn_mns_dvs_cd) - 1);
			::CopyMemory(sPacket.pyn_mns_dvs_cd, pBuff, nLen);
		}
		///< 발행채널구분코드		
		{
			sPacket.pub_chnl_dvs_cd[0] = 'K';	
		}

		{
			int csrc_len = 0, enc_len = 0, nLen = 0;
			char *pKeyStr;

			///< 암호문 키
			//pKeyStr = Transact_GetData(DAMO_ENC_DTA_KEY);
			pKeyStr = CConfigTkMem::GetInstance()->damo_enc_dta_key;
			TR_LOG_OUT("damo_enc_dta_key (%s)..", pKeyStr);

			///< 현금영수증인증번호암호문
			csrc_len = strlen(pTr->base.ui_csrc_no);
			TR_LOG_OUT("현금영수증번호 (%s), len(%d)..", pTr->base.ui_csrc_no, csrc_len);
			if(csrc_len > 0)
			{
#if 1
				::ZeroMemory(tmpBuffer, sizeof(tmpBuffer));
				enc_len = Dukpt(pKeyStr, pTr->base.ui_csrc_no, csrc_len, tmpBuffer);
				if(enc_len > 0)
				{
					;//TR_LOG_HEXA("암호화 데이타 #1..", (BYTE *)tmpBuffer, enc_len);
				}
				else
				{
					nRet = InitDamoCrypto();
					TR_LOG_OUT("retry InitDamoCrypto(), nRet(%d) !!!!", nRet);
					
					::ZeroMemory(tmpBuffer, sizeof(tmpBuffer));
					enc_len = Dukpt(pKeyStr, pTr->base.ui_csrc_no, csrc_len, tmpBuffer);
					if(enc_len > 0)
					{
						TR_LOG_HEXA("암호화 데이타_retry..", (BYTE *)tmpBuffer, enc_len);
					}
					else
					{
						TR_LOG_OUT("암호화 데이타 셍성 에러 !!!!");
					}
				}
				Util_Hex2String(tmpBuffer, enc_len, sPacket.damo_enc_csrc_auth_no, &nLen);
#else
				//TermDamoCrypto();
				nRet = InitDamoCrypto();
				TR_LOG_OUT("retry InitDamoCrypto(), nRet(%d) !!!!", nRet);

				::ZeroMemory(tmpBuffer, sizeof(tmpBuffer));
				enc_len = Dukpt(pKeyStr, pTr->base.ui_csrc_no, csrc_len, tmpBuffer);
				if(enc_len > 0)
				{
					TR_LOG_HEXA("암호화 데이타 #1..", (BYTE *)tmpBuffer, enc_len);
					Util_Hex2String(tmpBuffer, enc_len, sPacket.damo_enc_csrc_auth_no, &nLen);
				}
				else
				{
					TR_LOG_OUT("암호화 데이타 셍성 에러 !!!!");
				}
				TermDamoCrypto();
#endif
			}

			///< 현금영수증인증번호암호문 길이
			enc_len = strlen(sPacket.damo_enc_csrc_auth_no);
			::CopyMemory(sPacket.damo_enc_csrc_auth_no_len, &enc_len, 4);

			enc_len = strlen(pKeyStr);
			::CopyMemory(sPacket.damo_enc_dta_key, pKeyStr, enc_len);
		}

		if(pTr->base.ui_csrc_use[0] == 1)
		{
			sPacket.cprt_yn[0] = 'Y';///< 법인					
		}
		else
		{
			sPacket.cprt_yn[0] = 'N';///< 개인					
		}

		///< 사용자구분번호 (상주직원일 경우만 값이 있음)
		{
			if( strlen(pTr->base.user_dvs_no) <= 0 )
			{
				sPacket.user_dvs_no[0] = 0x20;
			}
			else
			{
				nLen = CheckBuffer(pBuff = pTr->base.user_dvs_no, sizeof(sPacket.user_dvs_no) - 1);
				::CopyMemory(sPacket.user_dvs_no, pBuff, nLen);
			}
		}

		///< *발행매수			
		{
			nCount = pTr->m_vtPcpysats.size();
			::CopyMemory(sPacket.pub_num, &nCount, 4);
		}

		///< 승차권 개별 발행 정보
		{
			vector<rtk_pcpysats_list_t>::iterator iter;

			iter = CPubTckMem::GetInstance()->m_vtPcpysats.begin();
			for(i = 0; i < nCount; i++)
			{
				/// 버스티켓종류코드
				::CopyMemory(sPacket.List[i].bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(sPacket.List[i].bus_tck_knd_cd) - 1);
				/// 좌석 선점ID
				::CopyMemory(sPacket.List[i].sats_pcpy_id, iter->sats_pcpy_id, sizeof(sPacket.List[i].sats_pcpy_id) - 1);
				/// 좌석번호
				::CopyMemory(sPacket.List[i].sats_no, iter->sats_no, sizeof(sPacket.List[i].sats_no) - 1);

				iter++;
			}
		}

		nRet = m_pclsSvr->TMaxSvc134((char *)&sPacket, (char *)&rPacket);
		if(nRet <= 0)
		{
			TR_LOG_OUT("Svr_IfSv_134() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_134() Success, nRet(%d) !!", nRet);
			
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_pubtckcsrc_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(stk_pubtckcsrc_t));
		KTC_MemClear(&rPacket, sizeof(rtk_pubtckcsrc_t));
#endif

	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_150
 * @details		현장발권 - 현금영수증 사후등록 (TK_Crrp)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_150(void)
{
	int nRet, i, nCount;

	TR_LOG_OUT(" start(승차권 발행 - 현금영수증 사후등록) !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_crrp_t	sPacket;
		rtk_crrp_t	rPacket;
		CPubTckMem* pTr;

		/// 현장발권 mem info
		pTr = CPubTckMem::GetInstance();

		::ZeroMemory(&sPacket, sizeof(stk_crrp_t));
		::ZeroMemory(&rPacket, sizeof(rtk_crrp_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		::ZeroMemory(sPacket.hdr_id.req_pgm_dvs, sizeof(sPacket.hdr_id.req_pgm_dvs));
		sprintf(sPacket.hdr_id.req_pgm_dvs, "TKT");

		///< 현금영수증인증번호 - 
		{
			//nLen = CheckBuffer(pBuff = pTr->csrc_resp., sizeof(sPacket.csrc_aprv_no) - 1);
			//::CopyMemory(sPacket.csrc_aprv_no, pBuff, nLen);
		}

		///< 암호화 데이타
		{
			int csrc_len = 0, enc_len = 0, nLen = 0;
			char *pKeyStr;

			///< 암호문 키
			pKeyStr = CConfigTkMem::GetInstance()->damo_enc_dta_key;

			///< 현금영수증인증번호암호문
			csrc_len = strlen(pTr->base.ui_csrc_no);
			if(csrc_len > 0)
			{
				::ZeroMemory(tmpBuffer, sizeof(tmpBuffer));
				enc_len = Dukpt(pKeyStr, pTr->base.ui_csrc_no, csrc_len, tmpBuffer);
				Util_Hex2String(tmpBuffer, enc_len, sPacket.damo_enc_csrc_auth_no, &nLen);
			}

			///< 현금영수증인증번호암호문 길이
			enc_len = strlen(sPacket.damo_enc_csrc_auth_no);
			::CopyMemory(sPacket.damo_enc_csrc_auth_no_len, &enc_len, 4);

			///< 암호문키
			enc_len = strlen(pKeyStr);
			::CopyMemory(sPacket.damo_enc_dta_key, pKeyStr, enc_len);
		}

		///< 법인여부				
		if(pTr->base.ui_csrc_use[0] == 1)
		{
			sPacket.cprt_yn[0] = 'Y';///< 법인					
		}
		else
		{
			sPacket.cprt_yn[0] = 'N';///< 개인					
		}

		vector<rtk_pubtckcsrc_list_t>::iterator iter;

		nCount = pTr->m_vtPbTckCsrc.size();


		::CopyMemory(sPacket.rgt_num, &nCount, 4);

		i = 0;
		for(iter = pTr->m_vtPbTckCsrc.begin(); iter != pTr->m_vtPbTckCsrc.end(); iter++)
		{
			/// 발행일자
			sprintf(sPacket.List[i].pub_dt			, "%s", pTr->csrc_resp.pub_dt);	
			/// 발행단축터미널코드
			sprintf(sPacket.List[i].pub_shct_trml_cd, "%s", pTr->csrc_resp.pub_shct_trml_cd);	
			/// 발행창구번호
			sprintf(sPacket.List[i].pub_wnd_no		, "%s", pTr->csrc_resp.pub_wnd_no);	
			/// 발행일련번호
			sprintf(sPacket.List[i].pub_sno			, "%04d", *(int *)iter->pub_sno);
			i++;
		}

		nRet = m_pclsSvr->TMaxSvc150((char *)&sPacket, (char *)&rPacket);
		if(nRet <= 0)
		{
			TR_LOG_OUT("TMaxSvc150() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("TMaxSvc150() Success, nRet(%d) !!", nRet);
		}

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(stk_crrp_t));
		KTC_MemClear(&rPacket, sizeof(rtk_crrp_t));
#endif

	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_221
 * @details		현장발권 - 승차권 발행(KTC) (TK_KtcPbTckCard)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_221(BOOL bReadRotYN)
{
	int nRet, i, nLen, nCount;

	TR_LOG_OUT(" start(승차권 발행 - KTC) !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_ktcpbtckcard_t	sPacket;
		rtk_ktcpbtckcard_t	rPacket;
		char *pBuff; 
		CPubTckMem* pTr;

		/// 현장발권 mem info
		pTr = CPubTckMem::GetInstance();

		::ZeroMemory(&sPacket, sizeof(stk_ktcpbtckcard_t));
		::ZeroMemory(&rPacket, sizeof(rtk_ktcpbtckcard_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 예매자료조회여부	
		if(bReadRotYN == TRUE)
		{
			sPacket.read_rot_yn[0] = 'Y';
		}
		else
		{
			sPacket.read_rot_yn[0] = 'N';
		}
		///< 노선ID
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_id, sizeof(sPacket.rot_id) - 1);
			::CopyMemory(sPacket.rot_id, pBuff, nLen);
		}
		///< 노선순번
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_sqno, sizeof(sPacket.rot_sqno) - 1);
			::CopyMemory(sPacket.rot_sqno, pBuff, nLen);
		}
		///< 배차일자
		{
			nLen = CheckBuffer(pBuff = pTr->base.alcn_dt, sizeof(sPacket.alcn_dt) - 1);
			::CopyMemory(sPacket.alcn_dt, pBuff, nLen);
		}
		///< 배차순번
		{
// 			nLen = CheckBuffer(pBuff = pTr->base.alcn_sqno, sizeof(sPacket.alcn_sqno) - 1);
// 			::CopyMemory(sPacket.alcn_sqno, pBuff, nLen);
			::CopyMemory(sPacket.alcn_sqno, pTr->base.alcn_sqno, 4);
		}
		///< 출발터미널코드
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->trml_cd, sizeof(sPacket.depr_trml_cd) - 1);
			::CopyMemory(sPacket.depr_trml_cd, pBuff, nLen);
		}
		///< 도착터미널코드
		{
			nLen = CheckBuffer(pBuff = pTr->base.arvl_trml_cd, sizeof(sPacket.arvl_trml_cd) - 1);
			::CopyMemory(sPacket.arvl_trml_cd, pBuff, nLen);
		}
		
		///< 지불수단구분코드	
		{
			nLen = CheckBuffer(pBuff = pTr->base.pyn_mns_dvs_cd, sizeof(sPacket.pyn_mns_dvs_cd) - 1);
			::CopyMemory(sPacket.pyn_mns_dvs_cd, pBuff, nLen);
		}
		///< 발행채널구분코드		
		{
			sPacket.pub_chnl_dvs_cd[0] = 'K';	
		}
		///< 발권방식구분코드		
		{
			sPacket.tisu_way_dvs_cd[0] = 'N';	
		}

// 		///< 카드번호암호문		
// 		char damo_enc_card_no	[1024+1]
// 		///< *카드번호암호문길이		
// 		char damo_enc_card_no_len[4+1];	
// 		///< 암호문키			
// 		char damo_enc_dta_key	[32+1];	
		///< 거래구분코드	('1':IC, '2':Fallback, '3':MS)
		{
			nLen = CheckBuffer(pBuff = pTr->base.trd_dvs_cd, sizeof(sPacket.trd_dvs_cd) - 1);
			::CopyMemory(sPacket.trd_dvs_cd, pBuff, nLen);
		}
		///< FallBack구분코드	
		{
			nLen = CheckBuffer(pBuff = pTr->base.fallback_dvs_cd, sizeof(sPacket.fallback_dvs_cd) - 1);
			::CopyMemory(sPacket.fallback_dvs_cd, pBuff, nLen);
		}
		///< 은련카드여부			
		{
			//nLen = CheckBuffer(pBuff = pTr->base.union_pay_yn, sizeof(sPacket.union_pay_yn) - 1);
			//::CopyMemory(sPacket.union_pay_yn, pBuff, nLen);
			sPacket.union_pay_yn[0] = 'N';
		}
		///< POS단말기버전정보		
		{
			::CopyMemory(sPacket.pos_pg_ktc_ver, ATEC_POS_VERSION, 13);	
//			sprintf(sPacket.pos_pg_ktc_ver, "%s", CConfigTkMem::GetInstance()->pos_ktc_version);	
		}
		///< *ENC 데이터 길이	
		{
			nLen = CheckBuffer(pBuff = pTr->base.enc_dta_len, sizeof(sPacket.enc_dta_len) - 1);
			::CopyMemory(sPacket.enc_dta_len, pBuff, nLen);
		}
		///< ENC 데이터	
		{
			nLen = CheckBuffer(pBuff = pTr->base.enc_dta, sizeof(sPacket.enc_dta) - 1);
			::CopyMemory(sPacket.enc_dta, pBuff, nLen);
		}
		///< EMV 데이터			
		{
			if(sPacket.trd_dvs_cd[0] != '1')
			{
				//memset(sPacket.emv_dta, 0x20, sizeof(sPacket.emv_dta) - 1);
				sPacket.emv_dta[0] = 0x20;
			}
			else
			{
				nLen = CheckBuffer(pBuff = pTr->base.emv_dta, sizeof(sPacket.emv_dta) - 1);
				::CopyMemory(sPacket.emv_dta, pBuff, nLen);
			}
		}
		///< *할부기간			
		{
			sPacket.mip_term[0] = pTr->base.mip_term[0];
		}
		///< 싸인패드데이터		
		{
			nLen = CheckBuffer(pBuff = pTr->base.spad_dta, sizeof(sPacket.spad_dta) - 1);
			::CopyMemory(sPacket.spad_dta, pBuff, nLen);
		}
		///< 싸인패드데이터길이		
		{
			nLen = CheckBuffer(pBuff = pTr->base.spad_dta_len, sizeof(sPacket.spad_dta_len) - 1);
			::CopyMemory(sPacket.spad_dta_len, pBuff, nLen);
		}
		///< 사용자구분번호 (상주직원일 경우만 값이 있음)
		{
			if( strlen(pTr->base.user_dvs_no) <= 0 )
			{
				//memset(sPacket.user_dvs_no, 0x20, sizeof(sPacket.user_dvs_no) - 1);
				sPacket.user_dvs_no[0] = 0x20;
			}
			else
			{
				nLen = CheckBuffer(pBuff = pTr->base.user_dvs_no, sizeof(sPacket.user_dvs_no) - 1);
				::CopyMemory(sPacket.user_dvs_no, pBuff, nLen);
			}
		}
		///< 구회원번호			
		{
			//memset(sPacket.old_mbrs_no, 0x20, sizeof(sPacket.old_mbrs_no) - 1);
			sPacket.old_mbrs_no[0] = 0x20;
		}

		///< RF교통카드 구분코드
		{
			if( pTr->base.pyn_mns_dvs_cd[0] == PYM_CD_RF )
			{
				sPacket.rf_trcr_dvs_cd[0] = pTr->base.rf_trcr_dvs_cd[0];
			}
			else
			{
				sPacket.rf_trcr_dvs_cd[0] = 0x20;
			}
		}

		///< *발행매수			
		{
			nCount = pTr->m_vtPcpysats.size();
			::CopyMemory(sPacket.pub_num, &nCount, 4);
		}

		///< 승차권 개별 발행 정보
		{
			vector<rtk_pcpysats_list_t>::iterator iter;

			iter = CPubTckMem::GetInstance()->m_vtPcpysats.begin();
			for(i = 0; i < nCount; i++)
			{
				/// 버스티켓종류코드
				::CopyMemory(sPacket.List[i].bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(sPacket.List[i].bus_tck_knd_cd) - 1);
				/// 좌석 선점ID
				::CopyMemory(sPacket.List[i].sats_pcpy_id, iter->sats_pcpy_id, sizeof(sPacket.List[i].sats_pcpy_id) - 1);
				/// 좌석번호
				::CopyMemory(sPacket.List[i].sats_no, iter->sats_no, sizeof(sPacket.List[i].sats_no) - 1);
				/// 예약매수
				::ZeroMemory(sPacket.List[i].mrnp_num, sizeof(sPacket.List[i].mrnp_num));
				/// 예약ID
				sPacket.List[i].mrnp_id[0] = 0x20;

				iter++;
			}
		}

		nRet = m_pclsSvr->TMaxSvc221((char *)&sPacket, (char *)&rPacket);
		if(nRet <= 0)
		{
			TR_LOG_OUT("Svr_IfSv_221() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_221() Success, nRet(%d) !!", nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_ktcpbtckcard_list_t) * nRet);
#endif			
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(stk_ktcpbtckcard_t));
		KTC_MemClear(&rPacket, sizeof(rtk_ktcpbtckcard_t));
#endif
	}

	return nRet;
}

// 20221017 ADD~
/**
 * @brief		Svr_IfSv_274
 * @details		현장발권 - 승차권 발행 요청 - QR결제 전용(KIOSK) (TK_PubTckQr)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_274(void)
{
	int nRet, i, nLen, nCount;

	TR_LOG_OUT(" start(승차권 발행 - QR결제) !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_pbtckqr_t	sPacket;
		rtk_pbtckqr_t	rPacket;
		char *pBuff; 
		CPubTckMem* pTr;

		/// 현장발권 mem info
		pTr = CPubTckMem::GetInstance();

		::ZeroMemory(&sPacket, sizeof(stk_pbtckqr_t));
		::ZeroMemory(&rPacket, sizeof(rtk_pbtckqr_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 노선ID
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_id, sizeof(sPacket.rot_id) - 1);
			::CopyMemory(sPacket.rot_id, pBuff, nLen);
		}
		///< 노선순번
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_sqno, sizeof(sPacket.rot_sqno) - 1);
			::CopyMemory(sPacket.rot_sqno, pBuff, nLen);
		}
		///< 배차일자
		{
			nLen = CheckBuffer(pBuff = pTr->base.alcn_dt, sizeof(sPacket.alcn_dt) - 1);
			::CopyMemory(sPacket.alcn_dt, pBuff, nLen);
		}
		///< 배차순번
		{
// 			nLen = CheckBuffer(pBuff = pTr->base.alcn_sqno, sizeof(sPacket.alcn_sqno) - 1);
// 			::CopyMemory(sPacket.alcn_sqno, pBuff, nLen);
			::CopyMemory(sPacket.alcn_sqno, pTr->base.alcn_sqno, 4);
		}
		///< 출발터미널코드
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->trml_cd, sizeof(sPacket.depr_trml_cd) - 1);
			::CopyMemory(sPacket.depr_trml_cd, pBuff, nLen);
		}
		///< 도착터미널코드
		{
			nLen = CheckBuffer(pBuff = pTr->base.arvl_trml_cd, sizeof(sPacket.arvl_trml_cd) - 1);
			::CopyMemory(sPacket.arvl_trml_cd, pBuff, nLen);
		}
		
		/// ///////////////////////////////////////////////////////////////////////////////
		// QR코드번호
		nLen = CheckBuffer(pBuff = pTr->base.qr_cd_no, sizeof(sPacket.qr_cd_no) - 1);
		::CopyMemory(sPacket.qr_cd_no, pBuff, nLen);
		// QR결제지불상세코드
		nLen = CheckBuffer(pBuff = pTr->base.qr_pym_pyn_dtl_cd, sizeof(sPacket.qr_pym_pyn_dtl_cd) - 1); // 20221228 ADD
		::CopyMemory(sPacket.qr_pym_pyn_dtl_cd, pBuff, nLen);											// 20221228 ADD
		// OTC번호;페이코가상일회성카드번호
		nLen = CheckBuffer(pBuff = pTr->base.payco_virt_ontc_no, sizeof(sPacket.payco_virt_ontc_no) - 1);
		::CopyMemory(sPacket.payco_virt_ontc_no, pBuff, nLen);
		/// ///////////////////////////////////////////////////////////////////////////////

		///< *할부기간			
		{
			sPacket.mip_term[0] = pTr->base.mip_term[0];
		}
		///< 싸인패드데이터		
		{
			nLen = CheckBuffer(pBuff = pTr->base.spad_dta, sizeof(sPacket.spad_dta) - 1);
			::CopyMemory(sPacket.spad_dta, pBuff, nLen);
		}
		///< 싸인패드데이터길이		
		{
			nLen = CheckBuffer(pBuff = pTr->base.spad_dta_len, sizeof(sPacket.spad_dta_len) - 1);
			::CopyMemory(sPacket.spad_dta_len, pBuff, nLen);
		}

		///< *발행매수			
		{
			nCount = pTr->m_vtPcpysats.size();
			::CopyMemory(sPacket.pub_num, &nCount, 4);
		}

		///< 승차권 개별 발행 정보
		{
			vector<rtk_pcpysats_list_t>::iterator iter;

			iter = CPubTckMem::GetInstance()->m_vtPcpysats.begin();
			for(i = 0; i < nCount; i++)
			{
				/// 버스티켓종류코드
				::CopyMemory(sPacket.List[i].bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(sPacket.List[i].bus_tck_knd_cd) - 1);
				/// 발권금액
				::CopyMemory(sPacket.List[i].tisu_amt, iter->dc_aft_amt, sizeof(iter->dc_aft_amt));
				/// 좌석 선점ID
				::CopyMemory(sPacket.List[i].sats_pcpy_id, iter->sats_pcpy_id, sizeof(sPacket.List[i].sats_pcpy_id) - 1);
				/// 좌석번호
				::CopyMemory(sPacket.List[i].sats_no, iter->sats_no, sizeof(sPacket.List[i].sats_no) - 1);

				iter++;
			}
		}

		nRet = m_pclsSvr->TMaxSvc274((char *)&sPacket, (char *)&rPacket);
		if(nRet <= 0)
		{
			TR_LOG_OUT("TMaxSvc274() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("TMaxSvc274() Success, nRet(%d) !!", nRet);
			if( rPacket.pList != NULL )
			{
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
	}

	return nRet;
}
// 20221017 ~ADD

/**
 * @brief		Svr_IfSv_222
 * @details		현장발권 - 승차권 발행 - 현금영수증(KTC)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_222(void)
{
	int nRet, i, nLen, nCount;

	TR_LOG_OUT(" start (현장발권 - 승차권 발행 - 현금영수증 (KTC) !! ");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_ktcpbtckcsrc_t	sPacket;
		rtk_ktcpbtckcsrc_t	rPacket;
		char *pBuff; 
		CPubTckMem* pTr;

		/// 현장발권 mem info
		pTr = CPubTckMem::GetInstance();

		::ZeroMemory(&sPacket, sizeof(stk_ktcpbtckcsrc_t));
		::ZeroMemory(&rPacket, sizeof(rtk_ktcpbtckcsrc_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 노선ID
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_id, sizeof(sPacket.rot_id) - 1);
			::CopyMemory(sPacket.rot_id, pBuff, nLen);
		}
		///< 노선순번
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_sqno, sizeof(sPacket.rot_sqno) - 1);
			::CopyMemory(sPacket.rot_sqno, pBuff, nLen);
		}
		///< 배차일자
		{
			nLen = CheckBuffer(pBuff = pTr->base.alcn_dt, sizeof(sPacket.alcn_dt) - 1);
			::CopyMemory(sPacket.alcn_dt, pBuff, nLen);
		}
		///< 배차순번
		{
// 			nLen = CheckBuffer(pBuff = pTr->base.alcn_sqno, sizeof(sPacket.alcn_sqno) - 1);
// 			::CopyMemory(sPacket.alcn_sqno, pBuff, nLen);
			::CopyMemory(sPacket.alcn_sqno, pTr->base.alcn_sqno, 4);
		}
		///< 출발터미널코드
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->trml_cd, sizeof(sPacket.depr_trml_cd) - 1);
			::CopyMemory(sPacket.depr_trml_cd, pBuff, nLen);
		}
		///< 도착터미널코드
		{
			nLen = CheckBuffer(pBuff = pTr->base.arvl_trml_cd, sizeof(sPacket.arvl_trml_cd) - 1);
			::CopyMemory(sPacket.arvl_trml_cd, pBuff, nLen);
		}
		///< 발권방식구분코드		
		{
			sPacket.tisu_way_dvs_cd[0] = 'N';	
		}
		///< 지불수단구분코드	
		{
			nLen = CheckBuffer(pBuff = pTr->base.pyn_mns_dvs_cd, sizeof(sPacket.pyn_mns_dvs_cd) - 1);
			::CopyMemory(sPacket.pyn_mns_dvs_cd, pBuff, nLen);
		}
		///< 발행채널구분코드		
		{
			sPacket.pub_chnl_dvs_cd[0] = 'K';	
		}
		///< POS단말기버전정보		
		{
			::CopyMemory(sPacket.pos_pg_ktc_ver, ATEC_POS_VERSION, 13);	
//			sprintf(sPacket.pos_pg_ktc_ver, "%s", CConfigTkMem::GetInstance()->pos_ktc_version);	
		}
		///< ENC 데이터 길이	
		{
			nLen = CheckBuffer(pBuff = pTr->base.enc_dta_len, sizeof(sPacket.enc_dta_len) - 1);
			::CopyMemory(sPacket.enc_dta_len, pBuff, nLen);
		}
		///< ENC 데이터	
		{
			nLen = CheckBuffer(pBuff = pTr->base.enc_dta, sizeof(sPacket.enc_dta) - 1);
			::CopyMemory(sPacket.enc_dta, pBuff, nLen);
		}
		///< 법인여부 CPRT_YN
		{
			if( pTr->base.ui_csrc_use[0] == 0 )
			{	/// 개인
				sPacket.cprt_yn[0] = 'N';
			}
			else
			{	/// 법인
				sPacket.cprt_yn[0] = 'Y';
			}
		}
		///< 사용자구분번호 (상주직원일 경우만 값이 있음)
		{
			if( strlen(pTr->base.user_dvs_no) <= 0 )
			{
				//memset(sPacket.user_dvs_no, 0x20, sizeof(sPacket.user_dvs_no) - 1);
				sPacket.user_dvs_no[0] = 0x20;
			}
			else
			{
				nLen = CheckBuffer(pBuff = pTr->base.user_dvs_no, sizeof(sPacket.user_dvs_no) - 1);
				::CopyMemory(sPacket.user_dvs_no, pBuff, nLen);
			}
		}
		///< 구회원번호			
		{
			//memset(sPacket.old_mbrs_no, 0x20, sizeof(sPacket.old_mbrs_no) - 1);
			sPacket.old_mbrs_no[0] = 0x20;
		}
		///< *발행매수			
		{
			nCount = pTr->m_vtPcpysats.size();
			::CopyMemory(sPacket.pub_num, &nCount, sizeof(int));
		}

		///< 승차권 개별 발행 정보
		{
			vector<rtk_pcpysats_list_t>::iterator iter;

			iter = CPubTckMem::GetInstance()->m_vtPcpysats.begin();
			for(i = 0; i < nCount; i++)
			{
				/// 버스티켓종류코드
				::CopyMemory(sPacket.List[i].bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(sPacket.List[i].bus_tck_knd_cd) - 1);
				/// 좌석 선점ID
				::CopyMemory(sPacket.List[i].sats_pcpy_id, iter->sats_pcpy_id, sizeof(sPacket.List[i].sats_pcpy_id) - 1);
				/// 좌석번호
				::CopyMemory(sPacket.List[i].sats_no, iter->sats_no, sizeof(sPacket.List[i].sats_no) - 1);
				
				/// 예약매수
				::ZeroMemory(sPacket.List[i].mrnp_num, sizeof(sPacket.List[i].mrnp_num));
				/// 예약ID
				sPacket.List[i].mrnp_id[0] = 0x20;

				iter++;
			}
		}

		nRet = m_pclsSvr->TMaxSvc222((char *)&sPacket, (char *)&rPacket);
		if(nRet <= 0)
		{
			TR_LOG_OUT("Svr_IfSv_222() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_222() Success, nRet(%d) !!", nRet);
			
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
 * @brief		Svr_IfSv_225
 * @details		현장발권 - 인천공항 상주직원 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_225(void)
{
	int nRet, nLen;

	TR_LOG_OUT(" start (현장발권 - 인천공항 상주직원 조회 !! ");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readrsd_t	sPacket;
		rtk_readrsd_t	rPacket;
		char *pBuff; 
		CPubTckMem* pTr;

		/// 현장발권 mem info
		pTr = CPubTckMem::GetInstance();

		::ZeroMemory(&sPacket, sizeof(stk_readrsd_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readrsd_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 조회창구 구분
		{
			sPacket.read_wnd_dvs[0] = 'K';	/// 무인발매기
		}
		///< 사용자 구분번호
		{
			nLen = CheckBuffer(pBuff = pTr->base.user_dvs_no, sizeof(sPacket.user_dvs_no) - 1);
			::CopyMemory(sPacket.user_dvs_no, pBuff, nLen);
		}
		///< 사용자 비밀번호
		{
			nLen = CheckBuffer(pBuff = pTr->base.user_pwd, sizeof(sPacket.user_pwd) - 1);
			::CopyMemory(sPacket.user_pwd, pBuff, nLen);
		}
		///< 발권 요청 여부
		{
			nLen = CheckBuffer(pBuff = pTr->base.tisu_req_yn, sizeof(sPacket.tisu_req_yn) - 1);
			::CopyMemory(sPacket.tisu_req_yn, pBuff, nLen);
		}

		///< 노선ID
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_id, sizeof(sPacket.rot_id) - 1);
			::CopyMemory(sPacket.rot_id, pBuff, nLen);
		}
		///< 노선순번
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_sqno, sizeof(sPacket.rot_sqno) - 1);
			::CopyMemory(sPacket.rot_sqno, pBuff, nLen);
		}
		///< 배차일자
		{
			nLen = CheckBuffer(pBuff = pTr->base.alcn_dt, sizeof(sPacket.alcn_dt) - 1);
			::CopyMemory(sPacket.alcn_dt, pBuff, nLen);
		}
		///< 배차순번
		{
			::CopyMemory(sPacket.alcn_sqno, pTr->base.alcn_sqno, 4);
		}

		nRet = m_pclsSvr->SVC_TK_ReadRsd((char *)&sPacket, (char *)&rPacket);
		if(nRet <= 0)
		{
			TR_LOG_OUT("[현장발권] 인천공항 상주직원 조회, Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("[현장발권] 인천공항 상주직원 조회, Success, nRet(%d) !!", nRet);
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_228
 * @details		현장발권 - 인천공항 상주직원 카드 좌석요금 정보 변경
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_228(void)
{
	int nRet, i, nLen, nCount;
	int enc_len = 0;

	TR_LOG_OUT(" start (현장발권 - 인천공항 상주직원 카드 좌석요금 정보 변경 !! ");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_modpcpysats_t	sPacket;
		rtk_modpcpysats_t	rPacket;
		char *pBuff; 
		CPubTckMem* pTr;
		char *pKeyStr;

		/// 현장발권 mem info
		pTr = CPubTckMem::GetInstance();

		::ZeroMemory(&sPacket, sizeof(stk_modpcpysats_t));
		::ZeroMemory(&rPacket, sizeof(rtk_modpcpysats_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 노선ID
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_id, sizeof(sPacket.rot_id) - 1);
			::CopyMemory(sPacket.rot_id, pBuff, nLen);
		}
		///< 노선순번
		{
			nLen = CheckBuffer(pBuff = pTr->base.rot_sqno, sizeof(sPacket.rot_sqno) - 1);
			::CopyMemory(sPacket.rot_sqno, pBuff, nLen);
		}
		///< 배차일자
		{
			nLen = CheckBuffer(pBuff = pTr->base.alcn_dt, sizeof(sPacket.alcn_dt) - 1);
			::CopyMemory(sPacket.alcn_dt, pBuff, nLen);
		}
		///< 배차순번
		{
			::CopyMemory(sPacket.alcn_sqno, pTr->base.alcn_sqno, 4);
		}

		///< 출발터미널코드
		{
			nLen = CheckBuffer(pBuff = pTr->base.depr_trml_cd, sizeof(sPacket.depr_trml_cd) - 1);
			::CopyMemory(sPacket.depr_trml_cd, pBuff, nLen);
		}
		///< 도착터미널코드
		{
			nLen = CheckBuffer(pBuff = pTr->base.arvl_trml_cd, sizeof(sPacket.arvl_trml_cd) - 1);
			::CopyMemory(sPacket.arvl_trml_cd, pBuff, nLen);
		}

		///< 카드번호 암호문
		{
			pKeyStr = CConfigTkMem::GetInstance()->damo_enc_dta_key;

			nLen = 0;
			::ZeroMemory(tmpBuffer, sizeof(tmpBuffer));
			enc_len = Dukpt(pKeyStr, pTr->base.enc_dta, *(int *)pTr->base.enc_dta_len, tmpBuffer);
			if(enc_len > 0)
			{
				Util_Hex2String(tmpBuffer, enc_len, sPacket.damo_enc_card_no, &nLen);
			}
		}
		///< 카드번호 암호문길이
		{
			//nLen = CheckBuffer(pBuff = pTr->base.enc_dta_len, sizeof(sPacket.damo_enc_card_no_len) - 1);
			::ZeroMemory(pTr->base.enc_dta_len, sizeof(pTr->base.enc_dta_len));
			::CopyMemory(pTr->base.enc_dta_len, &nLen, 4);
			::CopyMemory(sPacket.damo_enc_card_no_len, &nLen, 4);
		}

		///< 암호문키
		{
			//nLen = CheckBuffer(pBuff = pTr->base.enc_dta_key, sizeof(sPacket.damo_enc_dta_key) - 1);
			::CopyMemory(sPacket.damo_enc_dta_key, pKeyStr, strlen(pKeyStr));
		}

		/// 발행 매수
		{
			vector<UI_RESP_STAFF_CD_MOD_FARE_LIST_T>::iterator iter;

			nCount = pTr->m_vtStaffModFareReq.size();
			::CopyMemory(sPacket.pub_num, &nCount, 4);

			i = 0;
			for(iter = pTr->m_vtStaffModFareReq.begin(); iter != pTr->m_vtStaffModFareReq.end(); iter++, i++)
			{
				///< 좌석선점ID
				::CopyMemory(sPacket.List[i].sats_pcpy_id, iter->sats_pcpy_id, sizeof(sPacket.List[i].sats_pcpy_id) - 1);
				///< 버스티켓종류코드
				::CopyMemory(sPacket.List[i].bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(sPacket.List[i].bus_tck_knd_cd) - 1);
				///< 좌석번호
				::CopyMemory(sPacket.List[i].sats_no, iter->sats_no, sizeof(sPacket.List[i].sats_no) - 1);
			}
		}

		nRet = m_pclsSvr->TMaxSvc228((char *)&sPacket, (char *)&rPacket);
		if(nRet <= 0)
		{
			TR_LOG_OUT("[현장발권] 인천공항 상주직원 조회, Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("[현장발권] 인천공항 상주직원 조회, Success, nRet(%d) !!", nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_modpcpysats_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
	}

	return nRet;
}


/**
 * @brief		Svr_IfSv_137
 * @details		예매 리스트 조회 (TK_ReadMrnpPt)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_137(void)
{
	int nRet, nLen;//, nCount;

	TR_LOG_OUT(" start(예매 리스트 요청) !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readmrnppt_t	sPacket;
		rtk_readmrnppt_t	rPacket;
		char *pBuff; 
		CMrnpMem* pTr;

		/// 에매발권 mem info
		pTr = CMrnpMem::GetInstance();

		::ZeroMemory(&sPacket, sizeof(stk_readmrnppt_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readmrnppt_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 요청조회구분
		{
			sPacket.req_read_dvs_cd[0] = pTr->Base.req_read_dvs_cd[0];
		}
		///< 조회창구구분
		{
			sPacket.read_wnd_dvs[0] = 'K';
		}

		// 카드번호
		{
			if(sPacket.req_read_dvs_cd[0] == enMNPP_ENC_CARD_DVS_CD)
			{
				///< 카드번호
				//memset(sPacket.card_no, 0x20, sizeof(sPacket.card_no) - 1);
				memset(sPacket.card_no, 0x20, 1);

				///< enc_dta_len
				::CopyMemory(sPacket.enc_dta_len, pTr->Base.enc_dta_len, 4);

				///< enc_dta
				nLen = CheckBuffer(pBuff = pTr->Base.enc_dta, sizeof(sPacket.enc_dta) - 1);
				::CopyMemory(sPacket.enc_dta, pBuff, nLen);
			}
			else
			{
				///< 카드번호
				::CopyMemory(sPacket.card_no, pTr->Base.card_no, sizeof(pTr->Base.card_no) - 1);
			}
		}
		///< POS단말기버전정보		
		{
			::CopyMemory(sPacket.pos_pg_ktc_ver, ATEC_POS_VERSION, 13);	
//			sprintf(sPacket.pos_pg_ktc_ver, "%s", CConfigTkMem::GetInstance()->pos_ktc_version);	
		}
		///< 예약자 전화번호
		{
			nLen = CheckBuffer(pBuff = pTr->Base.mnpp_tel_no, sizeof(sPacket.mnpp_tel_no) - 1);
			::CopyMemory(sPacket.mnpp_tel_no, pBuff, nLen);	
		}
		///< 예약자 생년월일
		{
			nLen = CheckBuffer(pBuff = pTr->Base.mnpp_brdt, sizeof(sPacket.mnpp_brdt) - 1);
			::CopyMemory(sPacket.mnpp_brdt, pBuff, nLen);	
		}
		///< 예매번호
		{
			nLen = CheckBuffer(pBuff = pTr->Base.mrs_no, sizeof(sPacket.mrs_no) - 1);
			::CopyMemory(sPacket.mrs_no, pBuff, nLen);	
		}

		nRet = m_pclsSvr->TMaxSvc137((char *)&sPacket, (char *)&rPacket);
		if(nRet <= 0)
		{
			TR_LOG_OUT("Svr_IfSv_137() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_137() Success, nRet(%d) !!", nRet);

			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_readmrnppt_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(stk_readmrnppt_t));
		KTC_MemClear(&rPacket, sizeof(rtk_readmrnppt_t));
#endif

	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_138
 * @details		예매발권 (TK_PubMrnp)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_138(void)
{
	int nRet;//, nLen;//, nCount;

	TR_LOG_OUT(" start(예매 리스트 요청) !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_pubmrnp_t	sPacket;
		rtk_pubmrnp_t	rPacket;
		CMrnpMem* pTr;

		/// 에매발권 mem info
		pTr = CMrnpMem::GetInstance();

		::ZeroMemory(&sPacket, sizeof(stk_pubmrnp_t));
		::ZeroMemory(&rPacket, sizeof(rtk_pubmrnp_t));

		// header
		MakeHeaderIdData(&sPacket.hdr_id);

		// body
		CMrnpMem::GetInstance()->MakeReqIssuePacket((char *)&sPacket);

		nRet = m_pclsSvr->TMaxSvc138((char *)&sPacket, (char *)&rPacket);
		if(nRet <= 0)
		{
			TR_LOG_OUT("Svr_IfSv_138() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_138() Success, nRet(%d) !!", nRet);

			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_pubmrnp_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&sPacket, sizeof(stk_pubmrnp_t));
		KTC_MemClear(&rPacket, sizeof(rtk_pubmrnp_t));
#endif
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_213
 * @details		관리자 로그인 (TK_LoginKosMgr)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_213(char *pID, char *pPasswd)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_loginkosmgr_t sPacket;
		rtk_loginkosmgr_t rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_loginkosmgr_t));
		::ZeroMemory(&rPacket, sizeof(rtk_loginkosmgr_t));

		MakeHeaderData(&sPacket.hdr);

		///< 사용자 ID
		{
			::CopyMemory(sPacket.user_no, pID, strlen(pID));	
		}
		///< 사용자비밀번호
		{
			::CopyMemory(sPacket.req_trml_user_pwd, pPasswd, strlen(pPasswd));	
		}

		nRet = m_pclsSvr->TMaxSvc213((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_213() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_213() Success, nRet(%d) !!", nRet);
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_217
 * @details		발권제한 상세 (TK_TisuLtnDtl)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_217(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_tisultndtl_t sPacket;
		rtk_tisultndtl_t rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_tisultndtl_t));
		::ZeroMemory(&rPacket, sizeof(rtk_tisultndtl_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		nRet = m_pclsSvr->TMaxSvc217((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_217() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_217() Success, nRet(%d) !!", nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_tisultndtl_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_268
 * @details		도착터미널 키워드 매핑 조회 (TK_ReadTrmlKwd)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_268(void)
{
	int nRet;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readtrmlkwd_t sPacket;
		rtk_readtrmlkwd_t rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_readtrmlkwd_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readtrmlkwd_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		nRet = m_pclsSvr->TMaxSvc268((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_268() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_268() Success, nRet(%d) !!", nRet);
			if( rPacket.pList != NULL )
			{
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(rPacket.pList, sizeof(rtk_readtrmlkwd_list_t) * nRet);
#endif
				delete[] rPacket.pList;
				rPacket.pList = NULL;
			}
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_164
 * @details		창구마감 (TK_ClosWnd)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_164(void)
{
	int nRet;
//	char *pBuff;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_closwnd_t		sPacket;
		rtk_closwnd_t		rPacket;
		PFILE_ACCUM_N1010_T pAccum;

		pAccum = (PFILE_ACCUM_N1010_T) GetAccumulateData();

		::ZeroMemory(&sPacket, sizeof(stk_closwnd_t));
		::ZeroMemory(&rPacket, sizeof(rtk_closwnd_t));

		MakeHeaderIdData(&sPacket.hdr);

		///< 현재버스티켓고유번호	
		{
//			sprintf(sPacket.prs_bus_tck_inhr_no, "%s", "00000000");	
			sprintf(sPacket.prs_bus_tck_inhr_no, "%08d", pAccum->Curr.ccsTicket.n_bus_tck_inhr_no);	
		}
		///< 추가버스티켓고유번호
		{
//			sprintf(sPacket.add_bus_tck_inhr_no, "%s", "00000000");	
			if(pAccum->Curr.ccsTicket.n_add_bus_tck_inhr_no <= 0)
			{
				sprintf(sPacket.add_bus_tck_inhr_no, "%08d", 1);	
			}
			else
			{
				sprintf(sPacket.add_bus_tck_inhr_no, "%08d", pAccum->Curr.ccsTicket.n_add_bus_tck_inhr_no);	
			}
		}

		nRet = m_pclsSvr->TMaxSvc164((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_164() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_164() Success, nRet(%d) !!", nRet);
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_152
 * @details		버스티켓 교환 (TK_ExcBusTck)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_152(char *add_bus_tck_inhr_no)
{
	int nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_excbustck_t sPacket;
		rtk_excbustck_t rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_excbustck_t));
		::ZeroMemory(&rPacket, sizeof(rtk_excbustck_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 현재버스티켓고유번호	
		{
			sprintf(sPacket.bus_tck_inhr_no, "%s", add_bus_tck_inhr_no);	
		}
		///< 추가버스티켓고유번호
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->add_bus_tck_inhr_no, sizeof(sPacket.add_bus_tck_inhr_no) - 1);
			::CopyMemory(sPacket.add_bus_tck_inhr_no, pBuff, nLen);
		}

		nRet = m_pclsSvr->TMaxSvc152((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_152() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_152() Success, nRet(%d) !!", nRet);
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_135
 * @details		발행내역 조회 (재발행용) (TK_ReadPubPtR)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_135(char *pDate)
{
	int nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readpubptr_t sPacket;
		rtk_readpubptr_t rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_readpubptr_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readpubptr_t));

		MakeHeaderIdData(&sPacket.hdr_id);
		
		///< 조회 대상 일자	
		{
			SYSTEMTIME st;

			GetLocalTime(&st);

			//sprintf(sPacket.inqr_dt, "%s", pDate);	
			sprintf(sPacket.inqr_dt, "%04d%02d%02d", st.wYear, st.wMonth, st.wDay);	
		}
		///< 조회 구분
		{
			sPacket.inqr_dvs[0] = 'T';
		}
		///< 조회 대상 창구번호
		{
			nLen = CheckBuffer(pBuff = GetTrmlWndNo(SVR_DVS_CCBUS), sizeof(sPacket.inqr_wnd_no) - 1);
			::CopyMemory(sPacket.inqr_wnd_no, pBuff, strlen(pBuff));	
		}
		///< 사용자ID
		{
			//pBuff = Transact_GetData((DWORD)REQ_TRML_USER_ID);
			pBuff = CConfigTkMem::GetInstance()->req_trml_user_id;
			::CopyMemory(sPacket.user_id, pBuff, strlen(pBuff));	
		}

		nRet = m_pclsSvr->TMaxSvc135((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_135() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_135() Success, nRet(%d) !!", nRet);
		}

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_157
 * @details		재발행내역 조회 (TK_ReadRtckPt)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_157(char *pDate, char *pTime)
{
	int nRet;//, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readrtckpt_t sPacket;
		rtk_readrtckpt_t rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_readrtckpt_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readrtckpt_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 요청 조회 대상일	
		{
			SYSTEMTIME st;

			GetLocalTime(&st);
			sprintf(sPacket.read_dt, "%04d%02d%02d", st.wYear, st.wMonth, st.wDay);
		}
		///< 요청 조회 대상 시각
		{
			sprintf(sPacket.read_time, "000000");
		}
		///< 출발터미널코드
		{
			pBuff = GetTrmlCode(SVR_DVS_CCBUS);
			::CopyMemory(sPacket.depr_trml_cd, pBuff, strlen(pBuff));
		}
		///< 도착터미널코드
		{
			sprintf(sPacket.arvl_trml_cd, "0000000");
		}

		nRet = m_pclsSvr->TMaxSvc157((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_157() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_157() Success, nRet(%d) !!", nRet);
		}

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_155
 * @details		환불 내역 조회 (TK_ReadRyPt)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_155(char *pDate, char *pTime)
{
	int nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readrypt_t sPacket;
		rtk_readrypt_t rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_readrypt_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readrypt_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 요청 조회 대상일	
		{
			sprintf(sPacket.read_dt, "%s", pDate);
		}
		///< 요청 조회 대상 시각
		{
			sprintf(sPacket.read_time, "%s", pTime);
		}
		///< 출발터미널코드
		{
			nLen = CheckBuffer(pBuff = GetTrmlCode(SVR_DVS_CCBUS), sizeof(sPacket.depr_trml_cd) - 1);
			::CopyMemory(sPacket.depr_trml_cd, pBuff, strlen(pBuff));
		}
		///< 도착터미널코드
		{
			sprintf(sPacket.arvl_trml_cd, "%s", "0000000");
		}

		nRet = m_pclsSvr->TMaxSvc155((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_155() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_155() Success, nRet(%d) !!", nRet);
		}

		if(rPacket.pList != NULL)
		{
			delete[] rPacket.pList;
			rPacket.pList = NULL;
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_136
 * @details		버스티켓 재발행 (TK_RPubTck)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_136(char *pub_dt, char *pub_shct_trml_cd, char *pub_wnd_no, char *pub_sno)
{
	int nRet;//, nLen;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_rpubtck_t sPacket;
		rtk_rpubtck_t rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_rpubtck_t));
		::ZeroMemory(&rPacket, sizeof(rtk_rpubtck_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 발행일자		
		{
			::CopyMemory(sPacket.pub_dt, pub_dt, sizeof(sPacket.pub_dt) - 1);	
		}
		///< 발행단축터미널코드
		{
			::CopyMemory(sPacket.pub_shct_trml_cd, pub_shct_trml_cd, sizeof(sPacket.pub_shct_trml_cd) - 1);	
		}
		///< 발행창구번호		
		{
			::CopyMemory(sPacket.pub_wnd_no, pub_wnd_no, sizeof(sPacket.pub_wnd_no) - 1);	
		}
		///< 발행일련번호		
		{
			//int n_sno;

			//n_sno = Util_Ascii2Long(pub_sno, sizeof(sPacket.pub_sno) - 1);

			::CopyMemory(sPacket.pub_sno, pub_sno, sizeof(int));	
		}

		nRet = m_pclsSvr->TMaxSvc136((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_136() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_136() Success, nRet(%d) !!", nRet);
		}

	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_145
 * @details		버스티켓 취소/환불 (TK_CancRyTck)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_145(void)
{
	int nRet;//, nLen;
//	char *pBuff;
	int nCount, nLen, i;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_cancrytck_t sPacket;
		rtk_cancrytck_t rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_cancrytck_t));
		::ZeroMemory(&rPacket, sizeof(rtk_cancrytck_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 데이타 복사
		{
			CCancRyTkMem::GetInstance()->MakeTmaxRefundData((char *)&sPacket);
			//::CopyMemory(&sPacket.List[0], &CCancRyTkMem::GetInstance()->tReqRefund, sizeof(REQ_TICKET_REFUND_T));	
			
			// 이비카드 발권 티켓 환불 불가, 시외서버에 ‘승차권번호’ 추가 프로토콜 수정적용 (2021-05-11)
			nCount = *(WORD *)&sPacket.ryrt_num;
			nLen = strlen((char *)&sPacket.List[0].eb_bus_tck_sno);
			TR_LOG_OUT("eb_bus_tck_sno strlen : (%d)", nLen);
			if ((nLen > 8) && (nCount > 0))
			{
				for(i = 0; i < nCount; i++)
				{
					::CopyMemory(&sPacket.List[i].eb_bus_tck_sno, &CCancRyTkMem::GetInstance()->tReqTckNo, sizeof(REQ_TICKET_NO_T));
					TR_LOG_OUT("EB버스티켓일련번호; eb_bus_tck_sno : (%s), i/nCount : (%d/%d)", &sPacket.List[i].eb_bus_tck_sno, i, nCount);
				}
			} // (2021-05-11)
		}

		nRet = m_pclsSvr->TMaxSvc145((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("TMaxSvc145() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("TMaxSvc145() Success, nRet(%d) !!", nRet);

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
 * @brief		Svr_IfSv_158
 * @details		버스티켓 일련번호 조회 (TK_ReadBusTckNo)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_158(void)
{
	int nRet;//, nLen;
//	char *pBuff;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_readbustckno_t sPacket;
		rtk_readbustckno_t rPacket;

		::ZeroMemory(&sPacket, sizeof(stk_readbustckno_t));
		::ZeroMemory(&rPacket, sizeof(rtk_readbustckno_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 데이타 복사
		{
			//CCancRyTkMem* pCancTr;

			//pCancTr = CCancRyTkMem::GetInstance();

			/////< 발행시스템구분코드	
			//::CopyMemory(sPacket.pub_sys_dvs_cd, pCancTr->tReqTckNo.pub_sys_dvs_cd, sizeof(sPacket.pub_sys_dvs_cd) - 1);
			/////< 발행일자		
			//::CopyMemory(sPacket.pub_dt, pCancTr->tReqTckNo.pub_dt, sizeof(sPacket.pub_dt) - 1);
			/////< 발행단축터미널코드	
			//::CopyMemory(sPacket.pub_shct_trml_cd, pCancTr->tReqTckNo.pub_shct_trml_cd, sizeof(sPacket.pub_shct_trml_cd) - 1);
			/////< 발행창구번호		
			//::CopyMemory(sPacket.pub_wnd_no, pCancTr->tReqTckNo.pub_wnd_no, sizeof(sPacket.pub_wnd_no) - 1);
			/////< 발행일련번호		
			//::CopyMemory(sPacket.pub_sno, pCancTr->tReqTckNo.pub_sno, sizeof(sPacket.pub_sno) - 1);
			/////< eb버스티켓일련번호
			//::CopyMemory(sPacket.eb_bus_tck_sno, pCancTr->tReqTckNo.eb_bus_tck_sno, sizeof(sPacket.eb_bus_tck_sno) - 1);

			::CopyMemory(sPacket.pub_sys_dvs_cd, &CCancRyTkMem::GetInstance()->tReqTckNo, sizeof(REQ_TICKET_NO_T));	
		}

		nRet = m_pclsSvr->TMaxSvc158((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("TMaxSvc158() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("TMaxSvc158() Success, nRet(%d) !!", nRet);

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
 * @brief		Svr_IfSv_260
 * @details		승차권 발행(RF_선불카드) (TK_PubTckPpy)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_260(char *pParamData)
{
	int nRet, nLen;
	char *pBuff;
	PUBTCK_UI_RF_CARD_T *pUiData;

	TR_LOG_OUT(" start !!");

	pUiData = (PUBTCK_UI_RF_CARD_T *) pParamData;

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_pubtckppy_t sPacket;
		rtk_pubtckppy_t rPacket;


		::ZeroMemory(&sPacket, sizeof(stk_pubtckppy_t));
		::ZeroMemory(&rPacket, sizeof(rtk_pubtckppy_t));

		MakeHeaderIdData(&sPacket.hdr_id);


		///< 데이타 복사
		{
			int enc_len;
			CPubTckMem* pTr = CPubTckMem::GetInstance();
			char *pKeyStr;

			///< 노선id			
			{
				nLen = CheckBuffer(pBuff = pUiData->rot_id, sizeof(sPacket.rot_id	) - 1);
				::CopyMemory(sPacket.rot_id, pBuff, nLen);
			}
			///< (int)노선순번			
			{
				nLen = CheckBuffer(pBuff = pUiData->rot_sqno, sizeof(sPacket.rot_sqno	) - 1);
				::CopyMemory(sPacket.rot_sqno, pBuff, nLen);
			}
			///< 배차일자			
			{
				nLen = CheckBuffer(pBuff = pUiData->alcn_dt, sizeof(sPacket.alcn_dt	) - 1);
				::CopyMemory(sPacket.alcn_dt, pBuff, nLen);
			}
			///< (int)배차순번			
			{
				nLen = CheckBuffer(pBuff = pUiData->alcn_sqno, sizeof(sPacket.alcn_sqno	) - 1);
				::CopyMemory(sPacket.alcn_sqno, pBuff, nLen);
			}
			///< 출발터미널코드		
			{
				nLen = CheckBuffer(pBuff = pUiData->depr_trml_cd, sizeof(sPacket.depr_trml_cd	) - 1);
				::CopyMemory(sPacket.depr_trml_cd, pBuff, nLen);
			}
			///< 도착터미널코드		
			{
				nLen = CheckBuffer(pBuff = pUiData->arvl_trml_cd, sizeof(sPacket.arvl_trml_cd	) - 1);
				::CopyMemory(sPacket.arvl_trml_cd, pBuff, nLen);
			}
			///< 발권방식구분코드		
			{
				sPacket.tisu_way_dvs_cd[0] = 'N';
			}
			///< 예매자료조회여부		
			{
				sPacket.read_rot_yn[0] = 'N';
			}
			///< (int)발행매수			
			{
				nLen = 1;
				::CopyMemory(sPacket.pub_num, &nLen, 4);
			}
			///< 지불수단구분코드		
			{
				sPacket.pyn_mns_dvs_cd[0] = 'P';
			}
			///< 발행채널구분코드		
			{
				sPacket.pub_chnl_dvs_cd[0] = 'K';
			}
			///< 사용자구분번호		
			{
				sPacket.user_dvs_no[0] = 0x20;
			}
			///< 버스티켓종류코드		
			{
				nLen = CheckBuffer(pBuff = pUiData->bus_tck_knd_cd, sizeof(sPacket.bus_tck_knd_cd	) - 1);
				::CopyMemory(sPacket.bus_tck_knd_cd, pBuff, nLen);
			}
			///< (int)좌석번호			
			{
				::CopyMemory(sPacket.sats_no, pUiData->sats_no, sizeof(pUiData->sats_no));
			}
			///< 좌석선점id			
			{
				nLen = CheckBuffer(pBuff = pUiData->sats_pcpy_id, sizeof(sPacket.sats_pcpy_id	) - 1);
				::CopyMemory(sPacket.sats_pcpy_id	, pBuff, nLen);
			}
			///< 구회원번호			
			{
				sPacket.old_mbrs_no[0] = 0x20;
			}
			///< 예약id			
			{
				sPacket.mrnp_id[0] = 0x20;
			}
			///< (int)예약매수			
			{
				sPacket.mrnp_num[0] = 0x00;
			}
			///< 카드번호암호문		
			{
				int rf_card_no_len;

				pKeyStr = CConfigTkMem::GetInstance()->damo_enc_dta_key;

				nLen = 0;
				::ZeroMemory(tmpBuffer, sizeof(tmpBuffer));

				rf_card_no_len = strlen(pUiData->damo_enc_card_no);
				//TR_LOG_OUT("[RF_선불카드] len(%d), enc_card_no(%s) ...\n", rf_card_no_len, pUiData->damo_enc_card_no);
				enc_len = Dukpt(pKeyStr, pUiData->damo_enc_card_no, rf_card_no_len, tmpBuffer);
				if(enc_len > 0)
				{
					Util_Hex2String(tmpBuffer, enc_len, sPacket.damo_enc_card_no, &nLen);
				}
			}
			///< (int)카드번호암호문길이	
			{
				::ZeroMemory(pUiData->damo_enc_card_no_len, sizeof(pUiData->damo_enc_card_no_len));
				::CopyMemory(pUiData->damo_enc_card_no_len, &nLen, 4);
				::CopyMemory(sPacket.damo_enc_card_no_len, &nLen, 4);
			}
			///< 암호문키		
			{
				::CopyMemory(sPacket.damo_enc_dta_key, pKeyStr, strlen(pKeyStr));
			}
			///< 선불작업구분코드		
			{
				nLen = CheckBuffer(pBuff = pUiData->ppy_tak_dvs_cd, sizeof(sPacket.ppy_tak_dvs_cd	) - 1);
				::CopyMemory(sPacket.ppy_tak_dvs_cd, pBuff, nLen);
			}
			///< 선불pos영업일자		
			{
				nLen = CheckBuffer(pBuff = pUiData->ppy_pos_sls_dt, sizeof(sPacket.ppy_pos_sls_dt	) - 1);
				::CopyMemory(sPacket.ppy_pos_sls_dt, pBuff, nLen);
			}
			///< 선불pos영수증번호		
			{
				nLen = CheckBuffer(pBuff = pUiData->ppy_pos_recp_no, sizeof(sPacket.ppy_pos_recp_no	) - 1);
				::CopyMemory(sPacket.ppy_pos_recp_no, pBuff, nLen);
			}
			///< 선불samid			
			{
				nLen = CheckBuffer(pBuff = pUiData->ppy_sam_id, sizeof(sPacket.ppy_sam_id	) - 1);
				::CopyMemory(sPacket.ppy_sam_id, pBuff, nLen);
			}
			///< 선불sam거래일련번호	
			{
				nLen = CheckBuffer(pBuff = pUiData->ppy_sam_trd_sno, sizeof(sPacket.ppy_sam_trd_sno	) - 1);
				::CopyMemory(sPacket.ppy_sam_trd_sno, pBuff, nLen);
			}
			///< 선불카드거래일련번호	
			{
				nLen = CheckBuffer(pBuff = pUiData->ppy_card_trd_sno, sizeof(sPacket.ppy_card_trd_sno	) - 1);
				::CopyMemory(sPacket.ppy_card_trd_sno, pBuff, nLen);
			}
			///< (int)선불승인이후잔액	
			{
				nLen = CheckBuffer(pBuff = pUiData->ppy_aprv_aft_bal, sizeof(sPacket.ppy_aprv_aft_bal	) - 1);
				::CopyMemory(sPacket.ppy_aprv_aft_bal, pBuff, nLen);
			}
			///< (int)선불승인이전잔액	
			{
				nLen = CheckBuffer(pBuff = pUiData->ppy_aprv_bef_bal, sizeof(sPacket.ppy_aprv_bef_bal	) - 1);
				::CopyMemory(sPacket.ppy_aprv_bef_bal, pBuff, nLen);
			}
			///< 선불알고리즘id		
			{
				nLen = CheckBuffer(pBuff = pUiData->ppy_algr_id, sizeof(sPacket.ppy_algr_id	) - 1);
				::CopyMemory(sPacket.ppy_algr_id, pBuff, nLen);
			}
			///< 선불sign값			
			{
				nLen = CheckBuffer(pBuff = pUiData->ppy_sign_val, sizeof(sPacket.ppy_sign_val	) - 1);
				::CopyMemory(sPacket.ppy_sign_val, pBuff, nLen);
			}
			///< 선불개별거래수집키버전	
			{
				nLen = CheckBuffer(pBuff = pUiData->ppy_indv_trd_clk_ver, sizeof(sPacket.ppy_indv_trd_clk_ver	) - 1);
				::CopyMemory(sPacket.ppy_indv_trd_clk_ver, pBuff, nLen);
			}
			///< 선불전자화폐식별자id	
			{
				nLen = CheckBuffer(pBuff = pUiData->ppy_elcs_idnt_id, sizeof(sPacket.ppy_elcs_idnt_id	) - 1);
				::CopyMemory(sPacket.ppy_elcs_idnt_id, pBuff, nLen);
			}
			///< sam총액수집일련번호	
			{
				nLen = CheckBuffer(pBuff = pUiData->sam_ttam_clcn_sno, sizeof(sPacket.sam_ttam_clcn_sno	) - 1);
				::CopyMemory(sPacket.sam_ttam_clcn_sno, pBuff, nLen);
			}
			///< (int)sam개별수집건수	
			{
				nLen = CheckBuffer(pBuff = pUiData->sam_indv_clcn_ncnt, sizeof(sPacket.sam_indv_clcn_ncnt	) - 1);
				::CopyMemory(sPacket.sam_indv_clcn_ncnt, pBuff, nLen);
			}
			///< (int)sam누적거래총액	
			{
				nLen = CheckBuffer(pBuff = pUiData->sam_cum_trd_ttam, sizeof(sPacket.sam_cum_trd_ttam	) - 1);
				::CopyMemory(sPacket.sam_cum_trd_ttam, pBuff, nLen);
			}
			///< 선불카드구분코드		
			{
				nLen = CheckBuffer(pBuff = pUiData->ppy_card_dvs_cd, sizeof(sPacket.ppy_card_dvs_cd	) - 1);
				::CopyMemory(sPacket.ppy_card_dvs_cd, pBuff, nLen);
			}
			///< 선불카드사용자구분코드	
			{
				nLen = CheckBuffer(pBuff = pUiData->ppy_card_user_dvs_cd, sizeof(sPacket.ppy_card_user_dvs_cd	) - 1);
				::CopyMemory(sPacket.ppy_card_user_dvs_cd, pBuff, nLen);
			}
			///< 선불hsm상태코드		
			{
				nLen = CheckBuffer(pBuff = pUiData->ppy_hsm_sta_cd, sizeof(sPacket.ppy_hsm_sta_cd	) - 1);
				::CopyMemory(sPacket.ppy_hsm_sta_cd, pBuff, nLen);
			}
			///< (int)거래요청금액		
			{
				nLen = CheckBuffer(pBuff = pUiData->req_trd_amt, sizeof(sPacket.req_trd_amt	) - 1);
				::CopyMemory(sPacket.req_trd_amt, pBuff, nLen);
			}
		}

		nRet = m_pclsSvr->TMaxSvc260((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("TMaxSvc260() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("TMaxSvc260() Success, nRet(%d) !!", nRet);
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_269
 * @details		발행자 정보 입력 (TK_PubUsrInfInp)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_269(void)
{
	int nRet, nLen, i;
	//char *pBuff;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		stk_pubusrinfinp_t sPacket;
		rtk_pubusrinfinp_t rPacket;


		::ZeroMemory(&sPacket, sizeof(stk_pubusrinfinp_t));
		::ZeroMemory(&rPacket, sizeof(rtk_pubusrinfinp_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 데이타 복사
		{
			int enc_len;
			CPubTckMem* pTr = CPubTckMem::GetInstance();
			char *pKeyStr;

			///< 암호문 키
			//pKeyStr = Transact_GetData(DAMO_ENC_DTA_KEY);
			pKeyStr = CConfigTkMem::GetInstance()->damo_enc_dta_key;

			///< 1-0. 발행자 전화번호
			{
				nLen = strlen(pTr->base.pub_user_tel_no);
// 				if(nLen <= 0)
// 				{
// 					pTr->base.pub_user_tel_no[0] = 0x20;
// 					nLen = 1;
// 				}

				if(nLen < 2)
				{
					if(nLen > 0)
					{
						::CopyMemory(sPacket.damo_enc_pub_user_tel_no, pTr->base.pub_user_tel_no, nLen);
					}
				}
				else
				{
					::ZeroMemory(tmpBuffer, sizeof(tmpBuffer));
					enc_len = Dukpt(pKeyStr, pTr->base.pub_user_tel_no, nLen, tmpBuffer);
					Util_Hex2String(tmpBuffer, enc_len, sPacket.damo_enc_pub_user_tel_no, &nLen);
				}
			}

			///< 1-1. 발행자 전화번호 길이
			{
				enc_len = strlen(sPacket.damo_enc_pub_user_tel_no);
				::CopyMemory(sPacket.damo_enc_pub_user_tel_no_len, &enc_len, 4);
			}

			///< 1-2. 발행자 여권번호 or 주민번호
			{
				nLen = strlen(pTr->base.pub_user_krn);
				if(nLen > 0)
				{
					::ZeroMemory(tmpBuffer, sizeof(tmpBuffer));
					enc_len = Dukpt(pKeyStr, pTr->base.pub_user_krn, nLen, tmpBuffer);
					Util_Hex2String(tmpBuffer, enc_len, sPacket.damo_enc_pub_user_krn, &nLen);
				}
			}

			///< 1-3. 발행자 여권번호 or 주민번호 길이
			{
				enc_len = strlen(sPacket.damo_enc_pub_user_krn);
				::CopyMemory(sPacket.damo_enc_pub_user_krn_len, &enc_len, 4);
			}

			///< 암호문키		
			{
				enc_len = strlen(pKeyStr);
				::CopyMemory(sPacket.damo_enc_dta_key, pKeyStr, enc_len);
			}
		
			///< 승차 차량구분
			{
				::CopyMemory(sPacket.ride_vhcl_dvs, pTr->base.ride_vhcl_dvs, sizeof(pTr->base.ride_vhcl_dvs));
				//sPacket.ride_vhcl_dvs[0] = 0x20;
				//sPacket.ride_vhcl_dvs[1] = 0x20;
			}

			vector<TCK_PRINT_FMT_T>::iterator iter;

			///< 발행매수
			int nCount = pTr->m_vtPrtTicket.size();
			::CopyMemory(sPacket.pub_num, &nCount, 4);

			TR_LOG_OUT(" nCount = %d", nCount);

			for(i = 0, iter = pTr->m_vtPrtTicket.begin(); iter != pTr->m_vtPrtTicket.end(); iter++, i++)
			{
				int n_pub_sno;
				pstk_pubusrinfinp_list_t pList = &sPacket.List[i];

				///< 발행일자
				::CopyMemory(pList->pub_dt,				iter->bar_pub_dt, sizeof(pList->pub_dt) - 1);
				///< 발행단축터미널코드
				::CopyMemory(pList->pub_shct_trml_cd,	iter->bar_pub_shct_trml_cd, sizeof(pList->pub_shct_trml_cd) - 1);
				///< 발행창구번호
				::CopyMemory(pList->pub_wnd_no,			iter->bar_pub_wnd_no, sizeof(pList->pub_wnd_no) - 1);
				///< 발행일련번호
				//::CopyMemory(pList->pub_sno,			iter->bar_pub_sno, sizeof(pList->pub_sno) - 1);
				n_pub_sno = Util_Ascii2Long(iter->bar_pub_sno, strlen(iter->bar_pub_sno));
				::CopyMemory(pList->pub_sno,			&n_pub_sno, 4);
			}
		}

		TR_LOG_OUT(" m_pclsSvr->TMaxSvc269 .....");
		nRet = m_pclsSvr->TMaxSvc269((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("TMaxSvc269() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("TMaxSvc269() Success, nRet(%d) !!", nRet);
		}
	}

	return nRet;
}

/**
 * @brief		Svr_IfSv_502
 * @details		복호화 서비스 (SV_TrmDecrDta)
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_IfSv_502(char *p_damo_enc_dta)
{
	int nRet, nLen;
	char *pBuff;

	TR_LOG_OUT(" start !!");

	if(m_pclsSvr == NULL)
	{
		TR_LOG_OUT(" m_pclsSvr == NULL !!");
		return -1;
	}

	{
		ssv_trmdecrdta_t sPacket;
		rsv_trmdecrdta_t rPacket;

		::ZeroMemory(&sPacket, sizeof(ssv_trmdecrdta_t));
		::ZeroMemory(&rPacket, sizeof(rsv_trmdecrdta_t));

		MakeHeaderIdData(&sPacket.hdr_id);

		///< 암호화데이타	
		{
			::CopyMemory(sPacket.damo_enc_dta, p_damo_enc_dta, strlen(p_damo_enc_dta));	
		}

		///< 암호문키
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->damo_enc_dta_key, sizeof(sPacket.damo_enc_dta_key) - 1);
			::CopyMemory(sPacket.damo_enc_dta_key, pBuff, nLen);	
		}

		///< 암호문키 길이
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->damo_enc_dta_key_len, sizeof(sPacket.damo_enc_dta_key_len) - 1);
			::CopyMemory(sPacket.damo_enc_dta_key_len, pBuff, nLen);	
		}


		nRet = m_pclsSvr->TMaxSvc502((char *)&sPacket, (char *)&rPacket);
		if(nRet < 0)
		{
			TR_LOG_OUT("Svr_IfSv_502() Fail, nRet(%d) !!", nRet);
		}
		else
		{
			TR_LOG_OUT("Svr_IfSv_502() Success, nRet(%d) !!", nRet);
		}

	}

	return nRet;
}

int Svr_CCS_Initialize(void)
{
	int nRet;

	if(m_pclsSvr != NULL)
	{
		delete m_pclsSvr;
		m_pclsSvr = NULL;
	}
	m_pclsSvr = new CCBusServer();
	nRet = m_pclsSvr->StartProcess();

	return nRet;
}

int Svr_CCS_Terminate(void)
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

