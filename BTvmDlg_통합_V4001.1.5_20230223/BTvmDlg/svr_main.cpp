// 
// 
// svr_main.cpp : 시외버스 서버 main 소스
//
// touch 명령 : copy /B .\*.* +

#include "stdafx.h"
#include <stdio.h>
#include <queue>
#include <iostream>
#include <string.h>
#include <fcntl.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "svr_ccs_main.h"
#include "svr_ko_main.h"
#include "svr_tm_exp_main.h"
#include "oper_config.h"
#include "oper_kos_file.h"
#include "dev_tr_mem.h"
#include "dev_tr_main.h"
#include "event_if.h"
#include "svr_main.h"
#include "File_Env_ini.h"

//----------------------------------------------------------------------------------------------------------------------

using namespace std;
static queue <CCSVR_QUE_DATA_T> s_QueData;

//----------------------------------------------------------------------------------------------------------------------

static BOOL m_bConnect = FALSE;
static HANDLE		hThread = NULL;
static HANDLE		hAccMutex = NULL;
static DWORD		dwThreadID;
static BOOL			s_bRun = FALSE;

static DWORD WINAPI SvrCommThread(void *);

/**
 * @brief		CmdQueueInfo
 * @details		큐에 있는 명령 처리
 * @param		CCSVR_QUE_DATA_T tQue	: 큐 정보
 * @return		항상 = 0
 */
static int CmdQueueInfo(CCSVR_QUE_DATA_T tQue)
{
	int nRet, i, nSize, nOperID;
	char szBasePath[256*4];
	char szFullName[256*4];

	::ZeroMemory(szFullName, sizeof(szFullName));
	nRet = i = nSize = nOperID = 0;

	if( GetEventCode(EC_TMAX_LOGIN_ERR) )
	{
		return -1;
	}

	switch(tQue.wCommand)
	{
	case enCCSVR_999 : // codeconv.xml
		{
 			::ZeroMemory(szBasePath, sizeof(szBasePath));
 			Util_GetModulePath(szBasePath);
 
			//sprintf(szFullName, "%s\\wget.exe --no-check-certificate https://txbus.t-money.co.kr/xml/ConvertCode.xml -O %s\\files\\CodeConv.xml", szBasePath, szBasePath);
 			sprintf(szFullName, "C:\\\"Program Files (x86)\"\\GnuWin32\\bin\\wget.exe --no-check-certificate https://txbus.t-money.co.kr/xml/ConvertCode.xml -O %s\\files\\CodeConv.xml", szBasePath);
// 20220323 For Test
		//	sprintf(szFullName, "C:\\\"Program Files (x86)\"\\GnuWin32\\bin\\wget.exe --no-check-certificate https://txbus.t-money.co.kr/xml/ConvertCodeTest.xml -O %s\\files\\CodeConv.xml", szBasePath);
// 20211201 ADD~ // CodeConv.xml 파일 오류 시, 개발계 서버 테스트용
#if 0
			sprintf(szFullName, "C:\\\"Program Files (x86)\"\\GnuWin32\\bin\\wget.exe --no-check-certificate http://xzzt.koreasmartcard.com/tck/atkos/updatelist/ConvertCode.xml -O %s\\files\\CodeConv.xml", szBasePath);
#endif
// 20211201 ~ADD
			nRet = system(szFullName);
 			TR_LOG_OUT("[%s], ConvertCode Download nRet(%d) !!!! ", szFullName, nRet);
 			if(nRet >= 0)
 			{	/// success
 				UI_AddFileInfo(999, OPER_FILE_ID_0999);
 			}
		}
		break;
	case enCCSVR_208 :
		{
			if(tQue.bCheck == TRUE)
			{
				OperGetFileInfo(OPER_FILE_ID_0208, szFullName, &nSize);
				if(nSize > 0)
				{
					TR_LOG_OUT("Svr_IfSv_208(), File_exist, Skip !!!! ");
					UI_AddFileInfo(208, OPER_FILE_ID_0208);
					break;
				}
			}

			for(i = 0; i < 3; i++)
			{
				nRet = Svr_IfSv_208();	// 시외버스메시지코드 조회 : 1354
				TR_LOG_OUT("[시외서버] - 메시지코드 수신 결과, nRet = %d ", nRet);
				if(nRet < 0)
				{
					continue;
				}

				UI_AddFileInfo(208, OPER_FILE_ID_0208);
				break;
			}
		}
		break;

	case enCCSVR_101:
		{
			nRet = Svr_IfSv_101();	// 접속 컴퓨터 인증 & 터미널 코드 확인
			TR_LOG_OUT("[시외서버] - 접속 컴퓨터 인증 수신 결과, nRet = %d ", nRet);
			
			if( GetEnv_IsRealMode() == 0)
			{	/// TEST 서버용
				if(nRet < 0)
				{
					SetCheckEventCode(EC_TMAX_TEST_AUTH_ERR, TRUE);	
				}
				else
				{
					SetCheckEventCode(EC_TMAX_TEST_AUTH_ERR, FALSE);	
				}
			}
			else
			{	/// Real 서버용
				if(nRet < 0)
				{
					SetCheckEventCode(EC_TMAX_AUTH_ERR, TRUE);	
				}
				else
				{
					SetCheckEventCode(EC_TMAX_AUTH_ERR, FALSE);	
				}
			}
		}
		break;

	case enCCSVR_127:
		{
			nRet = Svr_IfSv_127();	// 버스티켓 고유번호 조회
			TR_LOG_OUT("[시외서버] - 버스티켓 고유번호 수신 결과, nRet = %d ", nRet);
			if(nRet < 0)
			{
				;
			}
			else
			{
				;
			}
		}
		break;

	case enCCSVR_128:
		{
			for(i = 0; i < 3; i++)
			{
				nRet = Svr_IfSv_128();	// 로그인
				TR_LOG_OUT("[시외서버] - 로그인 수신 결과, nRet = %d ", nRet);
				if(nRet < 0)
				{
					continue;
				}
				SetCheckEventCode(EC_TMAX_LOGIN_ERR, FALSE);
				break;
			}

			if(i >= 3)
			{
				SetCheckEventCode(EC_TMAX_LOGIN_ERR, TRUE);
			}
		}
		break;
	case enCCSVR_102:
		{
			if(tQue.bCheck == TRUE)
			{
				OperGetFileInfo(OPER_FILE_ID_0102, szFullName, &nSize);
				if(nSize > 0)
				{
					TR_LOG_OUT("Svr_IfSv_102(), File_exist, Skip !!!! \n");
					UI_AddFileInfo(102, OPER_FILE_ID_0102);
					break;
				}
			}

			nRet = Svr_IfSv_102();	// 공통코드상세
			TR_LOG_OUT("[시외서버] - 공통코드 상세 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(102, OPER_FILE_ID_0102);
			}
		}
		break;

	case enCCSVR_106:
		{
			nRet = Svr_IfSv_106();	// 버스 운수사 코드 조회(서버)
			TR_LOG_OUT("[시외서버] - 버스 운수사 코드 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(106, OPER_FILE_ID_0106);
			}
		}
		break;

	case enCCSVR_107:
		{
			nRet = Svr_IfSv_107();	// 버스등급 조회
			TR_LOG_OUT("[시외서버] - 버스등급 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(107, OPER_FILE_ID_0107);
			}
		}
		break;

	case enCCSVR_108:
		{
			nRet = Svr_IfSv_108();	// 버스티켓종류코드 조회(서버+자체)
			TR_LOG_OUT("[시외서버] - 버스티켓 종류코드 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(108, OPER_FILE_ID_0108);
			}
		}
		break;

	case enCCSVR_120:
		{
			if(tQue.bCheck == TRUE)
			{
 				OperGetFileInfo(OPER_FILE_ID_0120, szFullName, &nSize);
 				if(nSize > 0)
 				{
 					TR_LOG_OUT("Svr_IfSv_120(), File_exist, Skip !!!! ");
 					UI_AddFileInfo(120, OPER_FILE_ID_0120);
 					break;
 				}
			}

			nRet = Svr_IfSv_120();	// 터미널코드 (서버+자체)
			TR_LOG_OUT("[시외서버] - 터미널코드 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(120, OPER_FILE_ID_0120);
			}
		}
		break;

	case enCCSVR_114:
		{
			nRet = Svr_IfSv_114();	// 사용자 기본
			TR_LOG_OUT("[시외서버] - 사용자 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(114, OPER_FILE_ID_0114);
			}
		}
		break;

	case enCCSVR_129:
		{
			nRet = Svr_IfSv_129();	// 터미널 알림 조회 서비스. A03(무인기)꺼만 받아서 저장함.
			TR_LOG_OUT("[시외서버] - 터미널 알림 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(129, OPER_FILE_ID_0129);
			}
		}
		break;

	case enCCSVR_118:
		{
			nRet = Svr_IfSv_118();	// 터미널 상세 조회
			TR_LOG_OUT("[시외서버] - 터미널 상세 조회 수신 결과, nRet = %d ", nRet);
		}
		break;

	case enCCSVR_119:
		{
			nRet = Svr_IfSv_119();	// 컴퓨터 설정 상세 조회
			TR_LOG_OUT("[시외서버] - 컴퓨터 설정 상세 조회 수신 결과, nRet = %d ", nRet);
		}
		break;

	case enCCSVR_122:
		{
			nRet = Svr_IfSv_122();	// 터미널설정상세 조회
			TR_LOG_OUT("[시외서버] - 터미널 설정 상세 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(122, OPER_FILE_ID_0122);
			}
		}
		break;

	case enCCSVR_209:
		{
			if(tQue.bCheck == TRUE)
			{
				OperGetFileInfo(OPER_FILE_ID_0209, szFullName, &nSize);
				if(nSize > 0)
				{
					TR_LOG_OUT("Svr_IfSv_209(), File_exist, Skip !!!! \n");
					UI_AddFileInfo(209, OPER_FILE_ID_0209);
					break;
				}
			}

			nRet = Svr_IfSv_209();	// 버스티켓인쇄상세 조회
			TR_LOG_OUT("[시외서버] - 버스티켓 인쇄 상세 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(209, OPER_FILE_ID_0209);
			}
		}
		break;

	case enCCSVR_124:
		{
			if(tQue.bCheck == TRUE)
			{
				OperGetFileInfo(OPER_FILE_ID_0124, szFullName, &nSize);
				if(nSize > 0)
				{
					TR_LOG_OUT("Svr_IfSv_124(), File_exist, Skip !!!! \n");
					UI_AddFileInfo(124, OPER_FILE_ID_0124);
					break;
				}
			}

			nRet = Svr_IfSv_124();	// 노선 기본 조회
			TR_LOG_OUT("[시외서버] - 노선 기본 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(124, OPER_FILE_ID_0124);
			}
		}
		break;

	case enCCSVR_125:
		{
			if(tQue.bCheck == TRUE)
			{
				OperGetFileInfo(OPER_FILE_ID_0125, szFullName, &nSize);
				if(nSize > 0)
				{
					TR_LOG_OUT("Svr_IfSv_125(), File_exist, Skip !!!! \n");
					SaveThruInfo(GetTrmlCode(SVR_DVS_CCBUS));
					UI_AddFileInfo(125, OPER_FILE_ID_0125);
					break;
				}
			}

			nRet = Svr_IfSv_125();	// 노선 상세 조회
			TR_LOG_OUT("[시외서버] - 노선 상세 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				SaveThruInfo(GetTrmlCode(SVR_DVS_CCBUS));
				UI_AddFileInfo(125, OPER_FILE_ID_0125);
			}
		}
		break;
	case enCCSVR_126:
		{
			if(tQue.bCheck == TRUE)
			{
				OperGetFileInfo(OPER_FILE_ID_0126, szFullName, &nSize);
				if(nSize > 0)
				{
					TR_LOG_OUT("OPER_FILE_ID_0126, File_exist, Skip !!!! \n");
					UI_AddFileInfo(126, OPER_FILE_ID_0126);
					break;
				}
			}

			nRet = Svr_IfSv_126();	// 노선 요금 상세 조회
			TR_LOG_OUT("[시외서버] - 노선 요금 상세 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(126, OPER_FILE_ID_0126);
			}
		}
		break;
 	case enCCSVR_217:
		if(tQue.bCheck == TRUE)
		{
			OperGetFileInfo(OPER_FILE_ID_0217, szFullName, &nSize);
 			if(nSize > 0)
 			{
 				TR_LOG_OUT("Svr_IfSv_217(), File_exist, Skip !!!! \n");
 				UI_AddFileInfo(217, OPER_FILE_ID_0217);
 				break;
 			}
		}

 		nRet = Svr_IfSv_217();	// 발권제한 상세 조회
		TR_LOG_OUT("[시외서버] - 발권제한 상세 조회 수신 결과, nRet = %d ", nRet);
 		if(nRet > 0)
 		{
 			UI_AddFileInfo(217, OPER_FILE_ID_0217);
 		}
 		break;

	case enCCSVR_268:
		if(tQue.bCheck == TRUE)
		{
			OperGetFileInfo(OPER_FILE_ID_0268, szFullName, &nSize);
			if(nSize > 0)
			{
				TR_LOG_OUT("Svr_IfSv_268(), File_exist, Skip !!!! \n");
				UI_AddFileInfo(268, OPER_FILE_ID_0268);
				break;
			}
		}

		nRet = Svr_IfSv_268();	// 키워드 매핑 데이타 조회
		TR_LOG_OUT("[시외서버] - 키워드 매핑 데이타 수신 결과, nRet = %d ", nRet);
		if(nRet > 0)
		{
			UI_AddFileInfo(268, OPER_FILE_ID_0268);
		}
		break;

	case enCCSVR_200:
		{
			nRet = Svr_IfSv_200();	// 시외버스 지역코드 조회
			TR_LOG_OUT("[시외서버] - 지역코드 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(200, OPER_FILE_ID_0200);
			}
		}
		break;
	case enCCSVR_201:
		{
			if(tQue.bCheck == TRUE)
			{
				OperGetFileInfo(OPER_FILE_ID_0201, szFullName, &nSize);
				if(nSize > 0)
				{
					TR_LOG_OUT("왕복 가능 터미널 조회_201, File_exist, Skip !!!! \n");
					UI_AddFileInfo(201, OPER_FILE_ID_0201);
					break;
				}
			}

			nRet = Svr_IfSv_201();	// 왕복 가능 터미널 조회
			TR_LOG_OUT("[시외서버] - 왕복 가능 터미널 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(201, OPER_FILE_ID_0201);
			}
		}
		break;

	case enCCSVR_501:
		{
 			nRet = Svr_IfSv_501();	// 복호화 서비스
			TR_LOG_OUT("Svr_IfSv_501(), nRet = %d ", nRet);

			//nRet = Svr_IfSv_502("AC711EF1E45A33FCA947F7D3CFF2C0A05F965DCC912E50A855505E6F54D05416");	// 복호화 서비스, 300
		}
		break;
	
	case enKOSVR_CM_READMSG	: /// (코버스) 메시지 정보 조회
		{
			nOperID = OPER_FILE_ID_KO_CM_READMSG;

			if(tQue.bCheck == TRUE)
			{
				OperGetFileInfo(nOperID, szFullName, &nSize);
				if(nSize > 0)
				{
					UI_AddFileInfo(nOperID, nOperID);
					break;
				}
			}

			nRet = Kobus_CM_ReadMsg(nOperID);
			TR_LOG_OUT("[코버스] - 메시지 정보 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(nOperID, nOperID);
			}
		}
		break;

	case enKOSVR_CM_READNTC		: /// (코버스) 승차권 고유번호, 공지사항 조회
		{
			nOperID = OPER_FILE_ID_KO_CM_READNTC;

			nRet = Kobus_CM_ReadNtc(nOperID);
			TR_LOG_OUT("[코버스] - RTC 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(nOperID, nOperID);
			}
		}
		break;
	case enKOSVR_TK_AUTHCMPT	: /// (코버스) 접속 컴퓨터 인증 & 터미널 코드 확인(코버스는 skip)
		{
			// skip
		}
		break;
	case enKOSVR_CM_READCMNCD	: /// (코버스) 공통코드 조회
		{
			nOperID = OPER_FILE_ID_KO_CM_READCMNCD;

			if(tQue.bCheck == TRUE)
			{
				OperGetFileInfo(nOperID, szFullName, &nSize);
				if(nSize > 0)
				{
					UI_AddFileInfo(nOperID, nOperID);
					break;
				}
			}

			nRet = Kobus_CM_ReadCmnCd(nOperID);
			TR_LOG_OUT("[코버스] - 공통코드 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(nOperID, nOperID);
			}
		}
		break;
	case enKOSVR_TK_READTCKPRTG	: /// (코버스) 승차권 인쇄정보 조회
		{
			nOperID = OPER_FILE_ID_KO_TK_READTCKPRTG;

			nRet = Kobus_TK_ReadTckPrtg(nOperID);
			TR_LOG_OUT("[코버스] - 승차권 인쇄정보 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(nOperID, nOperID);
			}
		}
		break;
	case enKOSVR_TK_READOWNRTRML: /// (코버스) 자기터미널 정보 조회 (코버스는 skip)
		{
			// skip
		}
		break;
	case enKOSVR_CM_READTRML	: /// (코버스) 터미널 조회(전국터미널) (코버스는 skip)
		{
			/// skip
		}
		break;
	case enKOSVR_CM_READTRMLINF	: /// (코버스) 터미널 정보 조회
		{
			nOperID = OPER_FILE_ID_KO_CM_READTRMLINF;

			if(tQue.bCheck == TRUE)
			{
				OperGetFileInfo(nOperID, szFullName, &nSize);
				if(nSize > 0)
				{
					UI_AddFileInfo(nOperID, nOperID);
					break;
				}
			}

			nRet = Kobus_CM_ReadTrmlInf(nOperID);
			TR_LOG_OUT("[코버스] - 터미널 정보 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(nOperID, nOperID);
			}
		}
		break;
	case enKOSVR_CM_READTRMLSTUP: /// (코버스) 터미널환경설정보 (코버스는 skip)
		{
			/// skip
		}
		break;
	case enKOSVR_TM_EWNDINFO :		/// (코버스) 창구 정보 조회	
		{
			nOperID = OPER_FILE_ID_KO_TM_EWNDINFO;

			nRet = Kobus_MG_ReadWnd(nOperID);
			TR_LOG_OUT("[코버스] - 창구 정보 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(nOperID, nOperID);
			}
		}
		break;
	case enKOSVR_CM_READRTRPTRML: /// (코버스) 왕복터미널 정보 조회
		{
			nOperID = OPER_FILE_ID_KO_CM_READRTRPTRML;

			if(tQue.bCheck == TRUE)
			{
				OperGetFileInfo(nOperID, szFullName, &nSize);
				if(nSize > 0)
				{
					UI_AddFileInfo(nOperID, nOperID);
					break;
				}
			}

			nRet = Kobus_CM_ReadRtrpTrml(nOperID);
			TR_LOG_OUT("[코버스] - 왕복터미널 정보 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(nOperID, nOperID);
			}
		}
		break;
	case enKOSVR_CM_READTCKKND	: /// (코버스) 승차권 정보 조회 (코버스는 skip)
		{
			nOperID = OPER_FILE_ID_KO_CM_READTCKKND;

			if(tQue.bCheck == TRUE)
			{
				OperGetFileInfo(nOperID, szFullName, &nSize);
				if(nSize > 0)
				{
					UI_AddFileInfo(nOperID, nOperID);
					break;
				}
			}

			nRet = Kobus_CM_ReadTckKnd(nOperID);
			TR_LOG_OUT("[코버스] - 승차권 정보 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(nOperID, nOperID);
			}
		}
		break;
	case enKOSVR_CM_MNGCACM		: /// (코버스) 버스 정보 조회
		{
			nOperID = OPER_FILE_ID_KO_CM_MNGCACM;

			if(tQue.bCheck == TRUE)
			{
				OperGetFileInfo(nOperID, szFullName, &nSize);
				if(nSize > 0)
				{
					UI_AddFileInfo(nOperID, nOperID);
					break;
				}
			}

			nRet = Kobus_CM_MngCacm(nOperID);
			TR_LOG_OUT("[코버스] - 버스 정보 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(nOperID, nOperID);
			}
		}
		break;
	case enKOSVR_CM_READROTINF	: /// (코버스) 노선 정보 조회 (코버스는 skip)
		{
			/// skip
		}
		break;
	case enKOSVR_CM_RDHMINQR	: /// (코버스) 승차홈 조회
		{
			nOperID = OPER_FILE_ID_KO_CM_RDHMINQR;

			if(tQue.bCheck == TRUE)
			{
				OperGetFileInfo(nOperID, szFullName, &nSize);
				if(nSize > 0)
				{
					UI_AddFileInfo(nOperID, nOperID);
					break;
				}
			}

			nRet = Kobus_CM_RdhmInqr(nOperID);
			TR_LOG_OUT("[코버스] - 승차홈 조회 수신 결과, nRet = %d ", nRet);
			if(nRet > 0)
			{
				UI_AddFileInfo(nOperID, nOperID);
			}
		}
		break;
	case enKOSVR_RD_CORPINF :
		{
			nOperID = OPER_FILE_ID_KO_RD_CORPINF;
			OperGetFileInfo(nOperID, szFullName, &nSize);	
			TR_LOG_OUT("[코버스] - 로컬 운송회사 정보, nSize = %d ", nSize);
			if(nSize > 0)
			{
				CConfigTkMem::GetInstance()->ReadKobusCorpInfo(szFullName);
			}
		}
		break;

	case enEZSVR_CM_READMSG			: /// (티머니고속) 메시지 정보 조회
	case enEZSVR_CM_READNTC			: /// (티머니고속) 승차권 고유번호, 공지사항 조회
	case enEZSVR_TK_AUTHCMPT		: /// (티머니고속) 접속 컴퓨터 인증 & 터미널 코드 확인
	case enEZSVR_CM_READCMNCD		: /// (티머니고속) 공통코드 조회
	case enEZSVR_TK_READTCKPRTG		: /// (티머니고속) 승차권 인쇄정보 조회
	case enEZSVR_TK_READOWNRTRML	: /// (티머니고속) 자기터미널 정보 조회
	case enEZSVR_CM_READTRML		: /// (티머니고속) 터미널 조회(전국터미널)
	case enEZSVR_CM_READTRMLINF		: /// (티머니고속) 터미널 정보 조회
	case enEZSVR_CM_READTRMLSTUP	: /// (티머니고속) 터미널환경설정보
	case enEZSVR_MG_READWND			: /// (티머니고속) 창구정보 조회
	case enEZSVR_CM_READRTRPTRML	: /// (티머니고속) 왕복터미널 정보 조회
	case enEZSVR_CM_READTCKKND		: /// (티머니고속) 승차권 종류 정보 조회
	case enEZSVR_CM_MNGCACM			: /// (티머니고속) 버스 정보 조회
	case enEZSVR_CM_READROTINF		: /// (티머니고속) 노선 정보 조회
	case enEZSVR_CM_READTRMLDRTN	: /// (티머니고속) 방면 정보 조회
	case enEZSVR_CM_READRYRT		: /// (티머니고속) 환불율 정보 조회
	case enEZSVR_CM_END				: /// (티머니고속) end
		{
			nOperID = (int)tQue.wCommand;

			if(tQue.wCommand != enEZSVR_CM_END)
			{
				if(tQue.bCheck == TRUE)
				{
					OperGetFileInfo(nOperID, szFullName, &nSize);
					if(nSize > 0)
					{
						UI_AddFileInfo(nOperID, nOperID);
						break;
					}
				}
			}

			switch(tQue.wCommand)
			{
			case enEZSVR_CM_READMSG			: /// (티머니고속) 메시지 정보 조회
				nRet = TmExp_CM_ReadMsg(nOperID);
				TR_LOG_OUT("[티머니고속] - 메시지 코드 조회 수신 결과, nRet = %d ", nRet);
				break;
			case enEZSVR_CM_READNTC			: /// (티머니고속) 승차권 고유번호, 공지사항 조회
				nRet = TmExp_CM_ReadNtc(nOperID);
				TR_LOG_OUT("[티머니고속] - 승차권 고유번호, 공지사항 조회 수신 결과, nRet = %d ", nRet);
				break;
			case enEZSVR_TK_AUTHCMPT		: /// (티머니고속) 접속 컴퓨터 인증 & 터미널 코드 확인
				nRet = TmExp_TK_AuthCmpt(nOperID);
				TR_LOG_OUT("[티머니고속] - 접속 컴퓨터 인증 수신 결과, nRet = %d ", nRet);
				if(nRet < 0)
				{
					SetCheckEventCode(EC_TMAX_AUTH_ERR, TRUE);	
				}
				else
				{
					SetCheckEventCode(EC_TMAX_AUTH_ERR, FALSE);	
				}
				break;
			case enEZSVR_CM_READCMNCD		: /// (티머니고속) 공통코드 조회
				nRet = TmExp_CM_ReadCmnCd(nOperID);
				TR_LOG_OUT("[티머니고속] - 공통코드 조회 수신 결과, nRet = %d ", nRet);
				break;
			case enEZSVR_TK_READTCKPRTG		: /// (티머니고속) 승차권 인쇄정보 조회
				nRet = TmExp_TK_ReadTckPrtg(nOperID);
				TR_LOG_OUT("[티머니고속] - 승차권 인쇄정보 조회 수신 결과, nRet = %d ", nRet);
				break;
			case enEZSVR_TK_READOWNRTRML	: /// (티머니고속) 자기터미널 정보 조회
				nRet = TmExp_TK_ReadOwnrTrml(nOperID);
				TR_LOG_OUT("[티머니고속] - 자기터미널 정보 조회 수신 결과, nRet = %d ", nRet);
				break;
			case enEZSVR_CM_READTRML		: /// (티머니고속) 터미널 조회(전국터미널)
				nRet = TmExp_CM_ReadTrml(nOperID);
				TR_LOG_OUT("[티머니고속] - 터미널 조회(전국터미널) 수신 결과, nRet = %d ", nRet);
				break;
			case enEZSVR_CM_READTRMLINF		: /// (티머니고속) 터미널 정보 조회
				nRet = TmExp_CM_ReadTrmlInf(nOperID);
				TR_LOG_OUT("[티머니고속] - 터미널 정보 조회 수신 결과, nRet = %d ", nRet);
				break;
			case enEZSVR_CM_READTRMLSTUP	: /// (티머니고속) 터미널환경설정보
				nRet = TmExp_CM_ReadTrmlStup(nOperID);
				TR_LOG_OUT("[티머니고속] - 터미널환경설정보 수신 결과, nRet = %d ", nRet);
				break;
			case enEZSVR_MG_READWND			: /// (티머니고속) 창구정보 조회
				nRet = TmExp_MG_ReadWnd(nOperID);
				TR_LOG_OUT("[티머니고속] - 창구정보 조회 수신 결과, nRet = %d ", nRet);
				break;
			case enEZSVR_CM_READRTRPTRML	: /// (티머니고속) 왕복터미널 정보 조회
				nRet = TmExp_CM_ReadRtrpTrml(nOperID);
				TR_LOG_OUT("[티머니고속] - 왕복터미널 정보 조회 수신 결과, nRet = %d ", nRet);
				break;
			case enEZSVR_CM_READTCKKND		: /// (티머니고속) 승차권 종류 정보 조회
				nRet = TmExp_CM_ReadTckKnd(nOperID);
				TR_LOG_OUT("[티머니고속] - 승차권 종류 정보 조회 수신 결과, nRet = %d ", nRet);
				break;
			case enEZSVR_CM_MNGCACM			: /// (티머니고속) 운수사 정보 조회
				nRet = TmExp_CM_MngCacm(nOperID);
				TR_LOG_OUT("[티머니고속] - 운수사 정보 조회 수신 결과, nRet = %d ", nRet);
				break;
			case enEZSVR_CM_READROTINF		: /// (티머니고속) 노선 정보 조회
				nRet = TmExp_CM_ReadRotInf(nOperID);
				TR_LOG_OUT("[티머니고속] - 노선 정보 조회 수신 결과, nRet = %d ", nRet);
				break;
			case enEZSVR_CM_READTRMLDRTN	: /// (티머니고속) 방면 정보 조회
				nRet = TmExp_MG_ReadTrmlDrtn(nOperID);
				TR_LOG_OUT("[티머니고속] - 방면 정보 조회 수신 결과, nRet = %d ", nRet);
				break;
			case enEZSVR_CM_READRYRT		: /// (티머니고속) 환불율 정보 조회
				nRet = TmExp_CM_ReadRyrt(nOperID);
				TR_LOG_OUT("[티머니고속] - 환불율 정보 조회 수신 결과, nRet = %d ", nRet);
				break;
			case enEZSVR_CM_END :
				UI_AddTrmlInfo();
				return 0;
			}

			if(nRet > 0)
			{
				UI_AddFileInfo(nOperID, nOperID);
			}
		}
		break;
	}

	return 0;
}

/**
 * @brief		Svr_Initialize
 * @details		서버 모듈 초기화
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_Initialize(void)
{
	int nSvrKind;

	TR_LOG_OUT(" start !!");

	// set version
	{
		strcpy(CConfigTkMem::GetInstance()->main_version, MAIN_VERSION);
	}

	nSvrKind = GetConfigServerKind();

	// 시외버스
	if(nSvrKind & SVR_DVS_CCBUS)
	{
		Svr_CCS_Initialize();
	}

	// 코버스
	if(nSvrKind & SVR_DVS_KOBUS)
	{
		Kobus_Initialize();
	}

	// 티머니 고속버스
	if(nSvrKind & SVR_DVS_TMEXP)
	{
		TMExp_Initialize();
	}

	if(hThread != NULL)
	{
		::CloseHandle(hThread);
		hThread = NULL;
	}

	hAccMutex = ::CreateMutex(NULL, FALSE, NULL);
	if(hAccMutex == NULL) 
	{
		TR_LOG_OUT("[%s:%d] CreateMutex() failure..\n", __FUNCTION__, __LINE__);
		return -1;
	}

	hThread = ::CreateThread(NULL, 0, SvrCommThread, NULL, CREATE_SUSPENDED, &dwThreadID);
	if(hThread == NULL) 
	{
		return -2;
	}

	::ResumeThread(hThread);

	return 0;
}

/**
 * @brief		Svr_Terminate
 * @details		서버 모듈 종료
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Svr_Terminate(void)
{
	int nSvrKind;

	TR_LOG_OUT(" start !!");
	s_bRun = FALSE;

	nSvrKind = GetConfigServerKind();

	// 시외버스
	if(nSvrKind & SVR_DVS_CCBUS)
	{
		Svr_CCS_Terminate();
	}

	// 코버스 고속버스
	if(nSvrKind & SVR_DVS_KOBUS)
	{
		Kobus_Terminate();
	}

	// 티머니 고속버스
	if(nSvrKind & SVR_DVS_TMEXP)
	{
		TMExp_Terminate();
	}

	return 0;
}

/**
 * @brief		Svr_DataDownload
 * @details		티맥스 운영정보 다운로드
 * @param		None
 * @return		항상 = 0
 */
int Svr_DataDownload(BOOL bCheck)
{
#if 1
	int nSvrKind;

	nSvrKind = GetConfigServerKind();
	
	if(nSvrKind & SVR_DVS_CCBUS)
	{	/// 시외버스 
		Svr_AddQueData(enCCSVR_208, bCheck);					/// 01. 시외버스 메시지 코드 조회(TK_ReadCBusMsg)
		Svr_AddQueData(enCCSVR_101, bCheck);					/// 02. 컴퓨터 인증(TK_AuthCmpt)
		Svr_AddQueData(enCCSVR_127, bCheck);					/// 03. 버스티켓 고유번호 조회(TK_ReadTckNo)
		Svr_AddQueData(enCCSVR_128, bCheck);					/// 04. 로그인 (TK_Login)
		Svr_AddQueData(enCCSVR_102, bCheck);					/// 05. 공통코드 상세조회 (TK_CmnCdDtl)
		Svr_AddQueData(enCCSVR_106, bCheck);					/// 06. 버스운수사코드 조회 (TK_ReadBusCacm)
		Svr_AddQueData(enCCSVR_107, bCheck);					/// 07. 버스등급코드 조회 (TK_ReadBusCls)
		Svr_AddQueData(enCCSVR_108, bCheck);					/// 08. 버스티켓종류코드 조회 (TK_ReadTckKnd)
		Svr_AddQueData(enCCSVR_120, bCheck);					/// 09. 버스티켓종류코드 조회 (TK_ReadTrmlCd)
		Svr_AddQueData(enCCSVR_114, bCheck);					/// 10. 사용자 기본 조회 (TK_ReadUserBsc)
		Svr_AddQueData(enCCSVR_129, bCheck);					/// 11. 알림 조회 (TK_ReadTrmlCd)
		Svr_AddQueData(enCCSVR_118, bCheck);					/// 12. 터미널 상세 조회 (TK_TrmlWndDtl)
		Svr_AddQueData(enCCSVR_119, bCheck);					/// 13. 컴퓨터 설정 상세 조회 (TK_CmptStupDtl)
		Svr_AddQueData(enCCSVR_122, bCheck);					/// 14. 터미널 설정 상세 조회 (TK_TrmlStupDtl)
		Svr_AddQueData(enCCSVR_209, bCheck);					/// 15. 프린트 메시지 코드 조회 (TK_ReadPtrgTck)
		Svr_AddQueData(enCCSVR_124, bCheck);					/// 16. 노선 기본 조회(TK_ReadRotBsc)
		Svr_AddQueData(enCCSVR_125, bCheck);					/// 17. 노선 상세 조회(TK_ReadRotDtl)
		
		if( GetConfigOperCorp() == 2 )
		{	/// 인천공항 사이트
			Svr_AddQueData(enCCSVR_126, bCheck);				/// 18. 노선 요금 상세 조회(TK_RotFeeDtl) : 2020/01/10 add by nhso
		}
		Svr_AddQueData(enCCSVR_217, bCheck);					/// 19. 발매제한 상세 조회(TK_TisuLtnDtl)

		if( GetConfigOperCorp() == 2 )
		{	/// 인천공항 사이트
			Svr_AddQueData(enCCSVR_268, bCheck);				/// 20. 도착터미널 키워드 매핑조회(TK_ReadTrmlKwd)
		}
		Svr_AddQueData(enCCSVR_200, bCheck);					/// 21. 시외버스 지역코드 조회(TK_ReadAreaCd)
		Svr_AddQueData(enCCSVR_201, bCheck);					/// 22. 시외버스 왕복 가능 터미널 조회(TK_ReadRtrpTrml) : 2020/01/10 add by nhso
		Svr_AddQueData(enCCSVR_501, bCheck);					/// 23. 암호화키 전송(SV_TrmEncKey)
		Svr_AddQueData(enCCSVR_999, bCheck);					/// 24. CodeConv.xml
	}

	if(nSvrKind & SVR_DVS_KOBUS)
	{	/// 고속버스(코버스 서버)
		Svr_AddQueData(enKOSVR_CM_READMSG		, bCheck);		/// 01. (코버스) 메시지 정보 조회
		Svr_AddQueData(enKOSVR_CM_READNTC		, bCheck);		/// 02. (코버스) Set RTC (파일아님)
		Svr_AddQueData(enKOSVR_TK_AUTHCMPT		, bCheck);		/// 03. (코버스) 접속 컴퓨터 인증 & 터미널 코드 확인(코버스는 skip)
		Svr_AddQueData(enKOSVR_CM_READCMNCD		, bCheck);		/// 04. (코버스) 공통코드 조회
		Svr_AddQueData(enKOSVR_TK_READTCKPRTG	, bCheck);		/// 06. (코버스) 승차권 인쇄정보 조회
		Svr_AddQueData(enKOSVR_TK_READOWNRTRML	, bCheck);		/// 07. (코버스) 자기터미널 정보 조회 (코버스는 skip)
		Svr_AddQueData(enKOSVR_CM_READTRML		, bCheck);		/// 08. (코버스) 터미널 조회(전국터미널) (코버스는 skip)
		Svr_AddQueData(enKOSVR_CM_READTRMLINF	, bCheck);		/// 09. (코버스) 터미널 정보 조회
		Svr_AddQueData(enKOSVR_CM_READTRMLSTUP	, bCheck);		/// 10. (코버스) 터미널환경설정보 (코버스는 skip)
		Svr_AddQueData(enKOSVR_CM_READRTRPTRML	, bCheck);		/// 11. (코버스) 왕복터미널 정보 조회
		Svr_AddQueData(enKOSVR_TM_EWNDINFO		, bCheck);		/// 12. (코버스) 창구정보 조회
		Svr_AddQueData(enKOSVR_CM_READTCKKND	, bCheck);		/// 13. (코버스) 승차권 정보 조회 (공통코드로 승차권 정보 데이타 생성)
		Svr_AddQueData(enKOSVR_CM_MNGCACM		, bCheck);		/// 14. (코버스) 운수회사 정보 조회
		Svr_AddQueData(enKOSVR_CM_READROTINF	, bCheck);		/// 15. (코버스) 노선 정보 조회 (코버스는 skip)
		Svr_AddQueData(enKOSVR_RD_CORPINF		, bCheck);		/// 16. (코버스) 로컬 운송사 코드 정보 조회
		Svr_AddQueData(enKOSVR_CM_RDHMINQR		, bCheck);		/// 17. (코버스) 승차홈 조회
	}

	if(nSvrKind & SVR_DVS_TMEXP)
	{	/// 고속버스(이지 서버)
		bCheck = FALSE;
		Svr_AddQueData(enEZSVR_CM_READMSG		, bCheck	);	/// 01. (티머니고속) 메시지 정보 조회
		Svr_AddQueData(enEZSVR_CM_READNTC		, FALSE		);	/// 02. (티머니고속) 승차권 고유번호, 공지사항 조회
		Svr_AddQueData(enEZSVR_TK_AUTHCMPT		, FALSE		);	/// 03. (티머니고속) 접속 컴퓨터 인증 & 터미널 코드 확인
		Svr_AddQueData(enEZSVR_CM_READCMNCD		, bCheck	);	/// 04. (티머니고속) 공통코드 조회
		Svr_AddQueData(enEZSVR_TK_READTCKPRTG	, bCheck	);	/// 05. (티머니고속) 승차권 인쇄정보 조회
		Svr_AddQueData(enEZSVR_TK_READOWNRTRML	, bCheck	);	/// 06. (티머니고속) 자기터미널 정보 조회
		Svr_AddQueData(enEZSVR_CM_READTRML		, bCheck	);	/// 07. (티머니고속) 터미널 조회(전국터미널)
		Svr_AddQueData(enEZSVR_CM_READTRMLINF	, bCheck	);	/// 08. (티머니고속) 터미널 정보 조회
		Svr_AddQueData(enEZSVR_CM_READTRMLSTUP	, bCheck	);	/// 09. (티머니고속) 터미널환경설정보
		Svr_AddQueData(enEZSVR_MG_READWND		, bCheck	);	/// 10. (티머니고속) 창구정보 조회
		Svr_AddQueData(enEZSVR_CM_READRTRPTRML	, bCheck	);	/// 11. (티머니고속) 왕복터미널 정보 조회
		Svr_AddQueData(enEZSVR_CM_READTCKKND	, bCheck	);	/// 12. (티머니고속) 승차권 종류 정보 조회
		Svr_AddQueData(enEZSVR_CM_MNGCACM		, bCheck	);	/// 13. (티머니고속) 버스 정보 조회
		Svr_AddQueData(enEZSVR_CM_READROTINF	, bCheck	);	/// 14. (티머니고속) 노선 정보 조회
		Svr_AddQueData(enEZSVR_CM_READTRMLDRTN	, bCheck	);	/// 15. (티머니고속) 방면 정보 조회
		Svr_AddQueData(enEZSVR_CM_READRYRT		, bCheck	);	/// 16. (티머니고속) 환불율 정보 조회
		Svr_AddQueData(enEZSVR_CM_END			, bCheck	);	/// 17. (티머니고속) End
	}
#endif

	return 0;
}

/**
 * @brief		SvrCommThread
 * @details		UI 통신 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
DWORD WINAPI SvrCommThread(void *)
{
	int	nSize;
	CCSVR_QUE_DATA_T tQueData;
	PKIOSK_INI_ENV_T	pEnv;
	
	s_bRun = TRUE;

	pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();

	if(pEnv->nIsRealMode == 0 )
	{	/// 테스트 모드
		Svr_DataDownload(TRUE);
	}
	else
	{
		Svr_DataDownload(FALSE);
	}

	while(s_bRun) 
	{
		Sleep(10);	// 10 ms		

		nSize = s_QueData.size();
		if(nSize > 0)
		{
			tQueData = s_QueData.front();
			s_QueData.pop();

			CmdQueueInfo(tQueData);
		}

	} // while(1) 

	return 0L;
}

/**
 * @brief		Svr_AddQueData
 * @details		전송할 데이타를 큐에 넣는다.
 * @param		None
 * @return		항상 : 0
 */
int Svr_AddQueData(WORD wCommand, BOOL bCheck)
{
	CCSVR_QUE_DATA_T qData;

	::ZeroMemory(&qData, sizeof(CCSVR_QUE_DATA_T));

	qData.wCommand = wCommand;
	qData.bCheck = bCheck;
	s_QueData.push(qData);

	return 0;
}


