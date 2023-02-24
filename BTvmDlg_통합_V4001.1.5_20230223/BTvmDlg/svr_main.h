// 
// 
// svr_main.h : 시외버스 서버 main 헤더 파일
//

#pragma once

#include "MyLogFile.h"

//----------------------------------------------------------------------------------------------------------------------

enum _enSvrDvsCD_
{
	enCCSVR_208	= 1			,	/// (시외버스) 메시지 코드 조회
	enCCSVR_101				,	/// (시외버스) 컴퓨터 인증
	enCCSVR_127				,	/// (시외버스) 버스티켓 고유번호 조회
	enCCSVR_128				,	/// (시외버스) 로그인 (TK_Login)
	enCCSVR_102				,	/// (시외버스) 공통코드 상세조회 (TK_CmnCdDtl)
	enCCSVR_106				,	/// (시외버스) 버스운수사코드 조회 (TK_ReadBusCacm)
	enCCSVR_107				,	/// (시외버스) 버스등급코드 조회 (TK_ReadBusCls)
	enCCSVR_108				,	/// (시외버스) 버스티켓종류코드 조회 (TK_ReadTckKnd)
	enCCSVR_120				,	/// (시외버스) 터미널코드 조회 (TK_ReadTrmlCd)
	enCCSVR_114				,	/// (시외버스) 사용자 기본 조회 (TK_ReadUserBsc)
	enCCSVR_129				,	/// (시외버스) 알림 조회 (TK_ReadTrmlCd)
	enCCSVR_118				,	/// (시외버스) 터미널 상세 조회 (TK_TrmlWndDtl)
	enCCSVR_119				,	/// (시외버스) 컴퓨터 설정 상세 조회 (TK_CmptStupDtl)
	enCCSVR_122				,	/// (시외버스) 터미널 설정 상세 조회 (TK_TrmlStupDtl)
	enCCSVR_209				,	/// (시외버스) 시외버스 메시지 코드 조회 (TK_ReadCBusMsg)
	enCCSVR_124				,	/// (시외버스) 노선 기본 조회
	enCCSVR_125				,	/// (시외버스) 노선 상세 조회
	enCCSVR_126				,	/// (시외버스) 노선 요금 상세 조회
	enCCSVR_217				,	/// (시외버스) 발매제한 상세
	enCCSVR_268				,	/// (시외버스) 도착터미널 키워드 매핑조회

	enCCSVR_200				,	/// (시외버스) 시외버스 지역코드 조회
	enCCSVR_201				,	/// (시외버스) 시외버스 왕복가능 터미널 조회
	enCCSVR_501				,	/// (시외버스) 암호화키 전송
	enCCSVR_999				,	/// (시외버스) CodeConv.xml

	enKOSVR_CM_READMSG	= 1001,	/// 01. (코버스) 메시지 정보 조회
	enKOSVR_CM_READNTC		,	/// 02. (코버스) RTC
	enKOSVR_TK_AUTHCMPT		,	/// 03. (코버스) 접속 컴퓨터 인증 & 터미널 코드 확인(코버스는 skip)
	enKOSVR_CM_READCMNCD	,	/// 04. (코버스) 공통코드 조회
	

	enKOSVR_TK_READTCKPRTG	,	/// 06. (코버스) 승차권 인쇄정보 조회
	enKOSVR_TK_READOWNRTRML	,	/// 07. (코버스) 자기터미널 정보 조회 (코버스는 skip)
	enKOSVR_CM_READTRML		,	/// 08. (코버스) 터미널 조회(전국터미널)
	enKOSVR_CM_READTRMLINF	,	/// 09. (코버스) 터미널 정보 조회
	enKOSVR_CM_READTRMLSTUP	,	/// 10. (코버스) 터미널환경설정보
	enKOSVR_TM_EWNDINFO		,	/// 11. (코버스) 창구정보 조회
	enKOSVR_CM_READRTRPTRML	,	/// 12. (코버스) 왕복터미널 정보 조회
	enKOSVR_CM_READTCKKND	,	/// 13. (코버스) 승차권 정보 조회 (코버스는 skip)
	enKOSVR_CM_MNGCACM		,	/// 14. (코버스) 운수회사 정보 조회
	enKOSVR_CM_READROTINF	,	/// 15. (코버스) 노선 정보 조회 (코버스는 skip)

	enKOSVR_CM_RDHMINQR		,	/// 05. (코버스) 승차홈 조회
	enKOSVR_RD_CORPINF		,	/// 05. (코버스) 로컬 운송사 정보 조회


	enEZSVR_CM_READMSG		= 2001,	/// (티머니고속) 메시지 정보 조회
	enEZSVR_CM_READNTC		,	/// (티머니고속) 승차권 고유번호, 공지사항 조회
	enEZSVR_TK_AUTHCMPT		,	/// (티머니고속) 접속 컴퓨터 인증 & 터미널 코드 확인
	enEZSVR_CM_READCMNCD	,	/// (티머니고속) 공통코드 조회
	enEZSVR_TK_READTCKPRTG	,	/// (티머니고속) 승차권 인쇄정보 조회
	enEZSVR_TK_READOWNRTRML	,	/// (티머니고속) 자기터미널 정보 조회
	enEZSVR_CM_READTRML		,	/// (티머니고속) 터미널 조회(전국터미널)
	enEZSVR_CM_READTRMLINF	,	/// (티머니고속) 터미널 정보 조회
	enEZSVR_CM_READTRMLSTUP	,	/// (티머니고속) 터미널환경설정보
	enEZSVR_MG_READWND		,	/// (티머니고속) 창구정보 조회
	enEZSVR_CM_READRTRPTRML	,	/// (티머니고속) 왕복터미널 정보 조회
	enEZSVR_CM_READTCKKND	,	/// (티머니고속) 승차권 종류 정보 조회
	enEZSVR_CM_MNGCACM		,	/// (티머니고속) 버스 정보 조회
	enEZSVR_CM_READROTINF	,	/// (티머니고속) 노선 정보 조회
	enEZSVR_CM_READTRMLDRTN	,	/// (티머니고속) 방면 정보 조회
	enEZSVR_CM_READRYRT		,	/// (티머니고속) 환불율 정보 조회
	enEZSVR_CM_END			,	/// (티머니고속) END
};

//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

// queue data
typedef struct 
{
	WORD	wCommand;
	int		nLen;
	BOOL	bCheck;
	char	szData[100];
} CCSVR_QUE_DATA_T, *PCCSVR_QUE_DATA_T;

#pragma pack()

//----------------------------------------------------------------------------------------------------------------------

int Svr_DataDownload(BOOL bCheck);

int Svr_Initialize(void);
int Svr_Terminate(void);

int Svr_AddQueData(WORD wCommand, BOOL bCheck);
