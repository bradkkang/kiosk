// 
// 
// vm_event.cpp : 승차권 리더기 MAIN
//

#include "stdafx.h"
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <queue>
#include <iostream>
#include <fstream>

#include "MyDefine.h"
#include "MyUtil.h"
#include "event_if.h"
#include "MyEvent.h"
#include "dev_ui_main.h"
#include "dev_tr_main.h"

//----------------------------------------------------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------------------------------------------------

static CEventLogFile	clsLog;
static CMyEvent*		pclsEvent = NULL;
static HANDLE			hAccMutex = NULL;


static MYEVENTTABLE_T s_EvCodes[] = 
{
	{ EC_IN_SERVICE,				EVT_WARN,	FLAG_OP,	0UL,	"사용중" }, 
	{ EC_OUT_SERVICE,				EVT_WARN,	FLAG_OP,	0UL,	"사용중지" }, 

	{ EC_MAINT_IN_SERVICE,			EVT_ERROR,	FLAG_OP,	0UL,	"운영중" }, 
	{ EC_MAINT_OUT_SERVICE,			EVT_ERROR,	FLAG_OP,	0UL,	"운영중지" }, 

	{ EC_ADMIN_SERVICE_CLR,			EVT_WARN,	FLAG_OP,	0UL,	"관리자 기능 종료" }, 
	{ EC_ADMIN_SERVICE,				EVT_WARN,	FLAG_OP,	0UL,	"관리자 기능 시작" }, 

	{ EC_ADMIN_LOGOUT,				EVT_WARN,	FLAG_OP,	0UL,	"관리자 로그아웃" }, 
	{ EC_ADMIN_LOGIN,				EVT_WARN,	FLAG_OP,	0UL,	"관리자 로그인" }, 

	{ EC_COIN_100_NO_CLR,			EVT_ERROR,	FLAG_COIN,	0UL,	"100원 동전 없음 해제" }, 
	{ EC_COIN_100_NO,				EVT_ERROR,	FLAG_COIN,	0UL,	"100원 동전 없음" }, 

	{ EC_COIN_100_SHORT_CLR,		EVT_WARN,	FLAG_COIN,	0UL,	"100원 동전 부족 해제" }, 
	{ EC_COIN_100_SHORT,			EVT_WARN,	FLAG_COIN,	0UL,	"100원 동전 부족" }, 

	{ EC_COIN_100_MOTOR_ERR_CLR,	EVT_ERROR,	FLAG_COIN,	0UL,	"100원 동전 모터 에러 해제" }, 
	{ EC_COIN_100_MOTOR_ERR,		EVT_ERROR,	FLAG_COIN,	0UL,	"100원 동전 모터 에러" }, 

	{ EC_COIN_100_PROTOCOL_ERR_CLR,	EVT_ERROR,	FLAG_COIN,	0UL,	"100원 동전 프로토콜 에러 해제" }, 
	{ EC_COIN_100_PROTOCOL_ERR,		EVT_ERROR,	FLAG_COIN,	0UL,	"100원 동전 프로토콜 에러" }, 

	{ EC_COIN_100_OUT_ERR_CLR,		EVT_ERROR,	FLAG_COIN,	0UL,	"100원 동전 방출에러 해제" }, 
	{ EC_COIN_100_OUT_ERR,			EVT_ERROR,	FLAG_COIN,	0UL,	"100원 동전 방출에러" }, 

	{ EC_COIN_100_ENABLE_ERR_CLR,	EVT_ERROR,	FLAG_COIN,	0UL,	"100원 Enable 신호 해제" }, 
	{ EC_COIN_100_ENABLE_ERR,		EVT_ERROR,	FLAG_COIN,	0UL,	"100원 Enable 신호 에러" }, 

	{ EC_COIN_100_NO_OUT_CLR,		EVT_VAR,	FLAG_COIN,	0UL,	"100원 미방출 해제" }, 
	{ EC_COIN_100_NO_OUT,			EVT_VAR,	FLAG_COIN,	0UL,	"100원 미방출" }, 
	// 코인100 통신에러 추가 kh_200902
	{ EC_COIN_100_COMM_ERR_CLR,		EVT_ERROR,	FLAG_COIN,	0UL,	"100원 통신에러 해제" },
	{ EC_COIN_100_COMM_ERR,			EVT_ERROR,	FLAG_COIN,	0UL,	"100원 통신에러" }, 

	{ EC_COIN_500_NO_CLR,			EVT_ERROR,	FLAG_COIN,	0UL,	"500원 동전 없음 해제" }, 
	{ EC_COIN_500_NO,				EVT_ERROR,	FLAG_COIN,	0UL,	"500원 동전 없음" }, 

	{ EC_COIN_500_SHORT_CLR,		EVT_WARN,	FLAG_COIN,	0UL,	"500원 동전 부족 해제" }, 
	{ EC_COIN_500_SHORT,			EVT_WARN,	FLAG_COIN,	0UL,	"500원 동전 부족" }, 

	{ EC_COIN_500_MOTOR_ERR_CLR,	EVT_ERROR,	FLAG_COIN,	0UL,	"500원 동전 모터 에러 해제" }, 
	{ EC_COIN_500_MOTOR_ERR,		EVT_ERROR,	FLAG_COIN,	0UL,	"500원 동전 모터 에러" }, 

	{ EC_COIN_500_PROTOCOL_ERR_CLR,	EVT_ERROR,	FLAG_COIN,	0UL,	"500원 동전 프로토콜 에러 해제" }, 
	{ EC_COIN_500_PROTOCOL_ERR,		EVT_ERROR,	FLAG_COIN,	0UL,	"500원 동전 프로토콜 에러" }, 

	{ EC_COIN_500_OUT_ERR_CLR,		EVT_ERROR,	FLAG_COIN,	0UL,	"500원 동전 방출에러 해제" }, 
	{ EC_COIN_500_OUT_ERR,			EVT_ERROR,	FLAG_COIN,	0UL,	"500원 동전 방출에러" }, 

	{ EC_COIN_500_ENABLE_ERR_CLR,	EVT_ERROR,	FLAG_COIN,	0UL,	"500원 Enable 신호 해제" }, 
	{ EC_COIN_500_ENABLE_ERR,		EVT_ERROR,	FLAG_COIN,	0UL,	"500원 Enable 신호 에러" }, 

	{ EC_COIN_500_NO_OUT_CLR,		EVT_VAR,	FLAG_COIN,	0UL,	"500원 미방출 해제" }, 
	{ EC_COIN_500_NO_OUT,			EVT_VAR,	FLAG_COIN,	0UL,	"500원 미방출" }, 
	// 코인500 통신에러 추가 kh_200902
	{ EC_COIN_500_COMM_ERR_CLR,		EVT_ERROR,	FLAG_COIN,	0UL,	"500원 통신에러 해제" },
	{ EC_COIN_500_COMM_ERR,			EVT_ERROR,	FLAG_COIN,	0UL,	"500원 통신에러" }, 

 	{ EC_COIN_NO_OUT_CLR,			EVT_ERROR,	FLAG_COIN,	0UL,	"동전 미방출 해제" }, 
 	{ EC_COIN_NO_OUT,				EVT_ERROR,	FLAG_COIN,	0UL,	"동전 미방출" }, 

	{ EC_COIN_NO_CHECK_CLR,			EVT_VAR,	FLAG_COIN,	0UL,	"동전 오류 체크안함 해제" }, 
	{ EC_COIN_NO_CHECK,				EVT_VAR,	FLAG_COIN,	0UL,	"동전 오류 체크안함" }, 

// test
// 	{ EC_COIN_NO_OUT_CLR,			EVT_EVENT,	FLAG_COIN,	0UL,	"동전 미방출 해제" }, 
// 	{ EC_COIN_NO_OUT,				EVT_EVENT,	FLAG_COIN,	0UL,	"동전 미방출" }, 


	{ EC_FILE_ID_0100_ERR_CLR,		EVT_ERROR,	FLAG_OP,	0UL,	"공통코드 파일 없음 해제" }, 
	{ EC_FILE_ID_0100_ERR,			EVT_ERROR,	FLAG_OP,	0UL,	"공통코드 파일 없음" }, 

	{ EC_IC_CARD_DETECT_CLR,		EVT_FLAG,	FLAG_OP,	0UL,	"신용카드 Detect 해제" }, 
	{ EC_IC_CARD_DETECT,			EVT_FLAG,	FLAG_OP,	0UL,	"신용카드 Detect" }, 

	{ EC_IC_CARD_IN_CLR,			EVT_FLAG,	FLAG_OP,	0UL,	"신용카드 삽입 상태 해제" }, 
	{ EC_IC_CARD_IN,				EVT_FLAG,	FLAG_OP,	0UL,	"신용카드 삽입 상태" }, 

	{ EC_TMAX_TEST_AUTH_ERR_CLR,	EVT_WARN,	FLAG_TMAX,	0UL,	"컴퓨터 인증 에러 해제" }, 
	{ EC_TMAX_TEST_AUTH_ERR,		EVT_WARN,	FLAG_TMAX,	0UL,	"컴퓨터 인증 에러" }, 
	
	{ EC_TMAX_AUTH_ERR_CLR,			EVT_ERROR,	FLAG_TMAX,	0UL,	"컴퓨터 인증 에러 해제" }, 
	{ EC_TMAX_AUTH_ERR,				EVT_ERROR,	FLAG_TMAX,	0UL,	"컴퓨터 인증 에러" }, 

	{ EC_TMAX_LOGIN_ERR_CLR,		EVT_ERROR,	FLAG_TMAX,	0UL,	"서버 로그인 에러 해제" }, 
	{ EC_TMAX_LOGIN_ERR,			EVT_ERROR,	FLAG_TMAX,	0UL,	"서버 로그인 에러" }, 

	{ EC_TICKET_INSERT_CLR,			EVT_VAR,	FLAG_TCK_SCANNER,	0UL,	"승차권 반환" }, 
	{ EC_TICKET_INSERT,				EVT_VAR,	FLAG_TCK_SCANNER,	0UL,	"승차권 투입" }, 

	{ EC_CDU_1K_SHORT_CLR,			EVT_ERROR,	FLAG_CDU,	0UL,	"1,000원권 부족 해제" }, 
	{ EC_CDU_1K_SHORT,				EVT_ERROR,	FLAG_CDU,	0UL,	"1,000원권 부족" }, 

	{ EC_CDU_10K_SHORT_CLR,			EVT_ERROR,	FLAG_CDU,	0UL,	"10,000원권 부족 해제" }, 
	{ EC_CDU_10K_SHORT,				EVT_ERROR,	FLAG_CDU,	0UL,	"10,000원권 부족" }, 

	{ EC_CDU_1K_NO_OUT_CLR,			EVT_VAR,	FLAG_CDU,	0UL,	"1,000원권 미방출 해제" }, 
	{ EC_CDU_1K_NO_OUT,				EVT_VAR,	FLAG_CDU,	0UL,	"1,000원권 미방출" }, 

	{ EC_CDU_10K_NO_OUT_CLR,		EVT_VAR,	FLAG_CDU,	0UL,	"10,000원권 미방출 해제" }, 
	{ EC_CDU_10K_NO_OUT,			EVT_VAR,	FLAG_CDU,	0UL,	"10,000원권 미방출" }, 

 	{ EC_CDU_NO_OUT_CLR,			EVT_ERROR,	FLAG_CDU,	0UL,	"지폐 미방출 해제" }, 
 	{ EC_CDU_NO_OUT,				EVT_ERROR,	FLAG_CDU,	0UL,	"지폐 미방출" }, 

// test
// 	{ EC_BILL_NO_OUT_CLR,			EVT_EVENT,	FLAG_BILL,	0UL,	"지폐 미방출 해제" }, 
// 	{ EC_BILL_NO_OUT,				EVT_EVENT,	FLAG_BILL,	0UL,	"지폐 미방출" }, 

	{ EC_CDU_NO_CHECK_CLR,			EVT_VAR,	FLAG_CDU,	0UL,	"지폐 미방출 오류 체크안함 해제" }, 
	{ EC_CDU_NO_CHECK,				EVT_VAR,	FLAG_CDU,	0UL,	"지폐 미방출 오류 체크안함" }, 

	{ EC_CDU_DEV_ERR_CLR,			EVT_ERROR,	FLAG_CDU,	0UL,	"지폐방출기 장치 오류 해제" }, 
	{ EC_CDU_DEV_ERR,				EVT_ERROR,	FLAG_CDU,	0UL,	"지폐방출기 장치 오류" }, 
	// 지폐방출기 통신에러 추가 kh_200902
	{ EC_CDU_COMM_ERR_CLR,			EVT_ERROR,	FLAG_CDU,	0UL,	"지폐방출기 장치 통신에러 해제" }, 
	{ EC_CDU_COMM_ERR,				EVT_ERROR,	FLAG_CDU,	0UL,	"지폐방출기 장치 통신에러" }, 


	// 지폐인식기
/* // 20220127 DEL~
	{ EC_BILL_COMM_ERR_CLR,			EVT_ERROR,	FLAG_CDU,	0UL,	"지폐 통신에러 해제" }, 
	{ EC_BILL_COMM_ERR,				EVT_ERROR,	FLAG_CDU,	0UL,	"지폐 통신에러" }, 

	{ EC_BILL_JAM_CLR,				EVT_ERROR,	FLAG_CDU,	0UL,	"지폐걸림 해제" }, 
	{ EC_BILL_JAM,					EVT_ERROR,	FLAG_CDU,	0UL,	"지폐걸림" }, 

	{ EC_BILL_STK_FULL_CLR,			EVT_ERROR,	FLAG_CDU,	0UL,	"지폐 카세트 가득참 해제" }, 
	{ EC_BILL_STK_FULL,				EVT_ERROR,	FLAG_CDU,	0UL,	"지폐 카세트 가득참" }, 

	{ EC_BILL_STK_OUT_CLR,			EVT_ERROR,	FLAG_CDU,	0UL,	"지폐 카세트 분리 해제" }, 
	{ EC_BILL_STK_OUT,				EVT_ERROR,	FLAG_CDU,	0UL,	"지폐 카세트 분리" }, 
*/ // 20220127 ~DEL
// 20220127 MOD~
	{ EC_BILL_COMM_ERR_CLR,			EVT_ERROR,	FLAG_BILL,	0UL,	"지폐입금기 통신에러 해제" }, 
	{ EC_BILL_COMM_ERR,				EVT_ERROR,	FLAG_BILL,	0UL,	"지폐입금기 통신에러" }, 

	{ EC_BILL_JAM_CLR,				EVT_ERROR,	FLAG_BILL,	0UL,	"지폐입금기 지폐걸림 해제" }, 
	{ EC_BILL_JAM,					EVT_ERROR,	FLAG_BILL,	0UL,	"지폐입금기 지폐걸림" }, 

	{ EC_BILL_STK_FULL_CLR,			EVT_ERROR,	FLAG_BILL,	0UL,	"지폐입금기 카세트 가득참 해제" }, 
	{ EC_BILL_STK_FULL,				EVT_ERROR,	FLAG_BILL,	0UL,	"지폐입금기 카세트 가득참" }, 

	{ EC_BILL_STK_OUT_CLR,			EVT_ERROR,	FLAG_BILL,	0UL,	"지폐입금기 카세트 분리 해제" }, 
	{ EC_BILL_STK_OUT,				EVT_ERROR,	FLAG_BILL,	0UL,	"지폐입금기 카세트 분리" }, 
// 20220127 ~MOD

	{ EC_BILL_SENSOR_ERR_CLR,		EVT_ERROR,	FLAG_BILL,	0UL,	"지폐입금기 센서 해제" }, 
	{ EC_BILL_SENSOR_ERR,			EVT_ERROR,	FLAG_BILL,	0UL,	"지폐입금기 센서 에러" }, 


	// 감열지 미사용인 경우
	{ EC_PRT_COMM_ERR_CLR,			EVT_WARN,	FLAG_PRT,	0UL,	"통신에러 해제" }, 
	{ EC_PRT_COMM_ERR,				EVT_WARN,	FLAG_PRT,	0UL,	"통신에러" }, 

	{ EC_PRT_NO_PAPER_CLR,			EVT_WARN,	FLAG_PRT,	0UL,	"용지없음 해제" }, 
	{ EC_PRT_NO_PAPER,				EVT_WARN,	FLAG_PRT,	0UL,	"용지없음" }, 

	{ EC_PRT_HEAD_UP_CLR,			EVT_WARN,	FLAG_PRT,	0UL,	"헤드 업 해제" }, 
	{ EC_PRT_HEAD_UP,				EVT_WARN,	FLAG_PRT,	0UL,	"헤드 업" }, 

	{ EC_PRT_PAPER_JAM_CLR,			EVT_WARN,	FLAG_PRT,	0UL,	"용지 잼 해제" }, 
	{ EC_PRT_PAPER_JAM,				EVT_WARN,	FLAG_PRT,	0UL,	"용지 잼" }, 

	{ EC_PRT_PAPER_NEAR_END_CLR,	EVT_ERROR,	FLAG_PRT,	0UL,	"용지 Near End 해제" }, 
	{ EC_PRT_PAPER_NEAR_END,		EVT_ERROR,	FLAG_PRT,	0UL,	"용지 Near End" }, 

	{ EC_PRT_CUTTER_ERR_CLR,		EVT_WARN,	FLAG_PRT,	0UL,	"컷터 에러 해제" }, 
	{ EC_PRT_CUTTER_ERR,			EVT_WARN,	FLAG_PRT,	0UL,	"컷터 에러" }, 

	{ EC_PRT_BM_ERR_CLR,			EVT_WARN,	FLAG_PRT,	0UL,	"BM Error 에러 해제" }, 
	{ EC_PRT_BM_ERR,				EVT_WARN,	FLAG_PRT,	0UL,	"BM Error 에러" }, 

	{ EC_PRT_OUT_SENS_ERR_CLR,		EVT_WARN,	FLAG_PRT,	0UL,	"Paper Out Sensor 감지 해제" }, 
	{ EC_PRT_OUT_SENS_ERR,			EVT_WARN,	FLAG_PRT,	0UL,	"Paper Out Sensor 감지" }, 

	//// 감열지인 경우
	{ EC_PRT_ROLL_COMM_ERR_CLR,		EVT_ERROR,	FLAG_PRT,	0UL,	"통신에러 해제" }, 
	{ EC_PRT_ROLL_COMM_ERR,			EVT_ERROR,	FLAG_PRT,	0UL,	"통신에러" }, 

	{ EC_PRT_ROLL_NO_PAPER_CLR,		EVT_ERROR,	FLAG_PRT,	0UL,	"용지없음 해제" }, 
	{ EC_PRT_ROLL_NO_PAPER,			EVT_ERROR,	FLAG_PRT,	0UL,	"용지없음" }, 

	{ EC_PRT_ROLL_HEAD_UP_CLR,		EVT_ERROR,	FLAG_PRT,	0UL,	"헤드 업 해제" }, 
	{ EC_PRT_ROLL_HEAD_UP,			EVT_ERROR,	FLAG_PRT,	0UL,	"헤드 업" }, 

	{ EC_PRT_ROLL_PAPER_JAM_CLR,	EVT_ERROR,	FLAG_PRT,	0UL,	"용지 잼 해제" }, 
	{ EC_PRT_ROLL_PAPER_JAM,		EVT_ERROR,	FLAG_PRT,	0UL,	"용지 잼" }, 

	{ EC_PRT_ROLL_PAPER_NEAR_END_CLR,EVT_ERROR,	FLAG_PRT,	0UL,	"용지 Near End 해제" }, 
	{ EC_PRT_ROLL_PAPER_NEAR_END,	EVT_ERROR,	FLAG_PRT,	0UL,	"용지 Near End" }, 

	{ EC_PRT_ROLL_CUTTER_ERR_CLR,	EVT_ERROR,	FLAG_PRT,	0UL,	"컷터 에러 해제" }, 
	{ EC_PRT_ROLL_CUTTER_ERR,		EVT_ERROR,	FLAG_PRT,	0UL,	"컷터 에러" }, 

	{ EC_PRT_ROLL_BM_ERR_CLR,		EVT_ERROR,	FLAG_PRT,	0UL,	"BM Error 에러 해제" }, 
	{ EC_PRT_ROLL_BM_ERR,			EVT_ERROR,	FLAG_PRT,	0UL,	"BM Error 에러" }, 

	{ EC_PRT_ROLL_OUT_SENS_ERR_CLR,	EVT_ERROR,	FLAG_PRT,	0UL,	"Paper Out Sensor 감지 해제" }, 
	{ EC_PRT_ROLL_OUT_SENS_ERR,		EVT_ERROR,	FLAG_PRT,	0UL,	"Paper Out Sensor 감지" }, 

	//
	{ EC_TCKPRT_COMM_ERR_CLR,		EVT_ERROR,	FLAG_TCK_PRT,	0UL,	"승차권프린트 통신에러 해제" }, 
	{ EC_TCKPRT_COMM_ERR,			EVT_ERROR,	FLAG_TCK_PRT,	0UL,	"승차권프린트 통신에러" }, 

	{ EC_TCKPRT_NO_PAPER_CLR,		EVT_ERROR,	FLAG_TCK_PRT,	0UL,	"승차권프린트 용지없음 해제" }, 
	{ EC_TCKPRT_NO_PAPER,			EVT_ERROR,	FLAG_TCK_PRT,	0UL,	"승차권프린트 용지없음" }, 

	{ EC_TCKPRT_HEAD_UP_CLR,		EVT_ERROR,	FLAG_TCK_PRT,	0UL,	"승차권프린트 헤드 업 해제" }, 
	{ EC_TCKPRT_HEAD_UP,			EVT_ERROR,	FLAG_TCK_PRT,	0UL,	"승차권프린트 헤드 업" }, 

	{ EC_TCKPRT_PAPER_JAM_CLR,		EVT_ERROR,	FLAG_TCK_PRT,	0UL,	"승차권프린트 용지 잼 해제" }, 
	{ EC_TCKPRT_PAPER_JAM,			EVT_ERROR,	FLAG_TCK_PRT,	0UL,	"승차권프린트 용지 잼" }, 

	{ EC_TCKPRT_PAPER_NEAR_END_CLR,	EVT_WARN,	FLAG_TCK_PRT,	0UL,	"승차권프린트 용지 Near End 해제" }, 
	{ EC_TCKPRT_PAPER_NEAR_END,		EVT_WARN,	FLAG_TCK_PRT,	0UL,	"승차권프린트 용지 Near End" }, 

	{ EC_TCKPRT_CUTTER_ERR_CLR,		EVT_ERROR,	FLAG_TCK_PRT,	0UL,	"승차권프린트 컷터 에러 해제" }, 
	{ EC_TCKPRT_CUTTER_ERR,			EVT_ERROR,	FLAG_TCK_PRT,	0UL,	"승차권프린트 컷터 에러" }, 

	{ EC_TICKET_SHORT_CLR,			EVT_ERROR,	FLAG_OP,	0UL,	"승차권 부족 해제" }, 
	{ EC_TICKET_SHORT,				EVT_ERROR,	FLAG_OP,	0UL,	"승차권 부족" }, 

	{ EC_JOB_CLOSE_CLR,				EVT_ERROR,	FLAG_OP,	0UL,	"자동 창구마감 해제" }, 
	{ EC_JOB_CLOSE,					EVT_ERROR,	FLAG_OP,	0UL,	"자동 창구마감" }, 

	{ EC_OP_CLOSE_CLR,				EVT_ERROR,	FLAG_OP,	0UL,	"서비스 중지 시간 해제" }, 
	{ EC_OP_CLOSE,					EVT_ERROR,	FLAG_OP,	0UL,	"서비스 중지 시간" }, 

	{ EC_RF_PAYMENT_CLR,			EVT_ERROR,	FLAG_RF,	0UL,	"RF 결제 실패 해제" }, 
	{ EC_RF_PAYMENT_ERR,			EVT_ERROR,	FLAG_RF,	0UL,	"RF 결제 실패" }, 

	{ EC_SCAN_COMM_ERR_CLR,			EVT_ERROR,	FLAG_TCK_SCANNER,	0UL,	"승차권스케너 통신에러 해제" }, 
	{ EC_SCAN_COMM_ERR,				EVT_ERROR,	FLAG_TCK_SCANNER,	0UL,	"승차권스케너 통신에러" }, 
};


//----------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------

#define LOG_OUT(fmt, ...)		{ clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ clsLog.HexaDump(x, y, z); }

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		LOG_INIT
 * @details		LOG 초기화
 * @param		None
 * @return		항상 = 0
 */
static int LOG_INIT(void)
{
	clsLog.SetData(30, "\\Log\\Event");
	clsLog.Initialize();
	clsLog.Delete();

	return 0;
}

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
static int Locking(void)
{
	if(hAccMutex != NULL) 
	{
		::WaitForSingleObject(hAccMutex, INFINITE);
	}

	return 0;	
}

/**
 * @brief		UnLocking
 * @details		IPC UnLock
 * @param		None
 * @return		항상 : 0
 */
static int UnLocking(void)
{
	if(hAccMutex != NULL) 
	{
		::ReleaseMutex(hAccMutex);
	}

	return 0;	
}

/**
 * @brief		Init_Event
 * @details		이벤트 코드 초기화
 * @param		None
 * @return		항상 : 0
 */
int InitEventCode(void)
{
	int nCount;

	LOG_INIT();

	if(pclsEvent != NULL)
	{
		delete pclsEvent;
		pclsEvent = NULL;
	}

	hAccMutex = ::CreateMutex(NULL, FALSE, NULL);
	if(hAccMutex == NULL) 
	{
		//LOG_OUT("m_clsComm.Open() Failure, nRet(%d) ..", nRet);
		return -1;
	}

	pclsEvent = new CMyEvent();

	nCount = sizeof(s_EvCodes) / sizeof(MYEVENTTABLE_T);
	pclsEvent->Initialize(nCount, (BYTE *)s_EvCodes);


	return 0;
}

/**
 * @brief		Terminate_Event
 * @details		이벤트 코드 초기화
 * @param		None
 * @return		항상 : 0
 */
int TerminateEventCode(void)
{
	if(pclsEvent != NULL)
	{
		pclsEvent->Terminate();

		delete pclsEvent;
		pclsEvent = NULL;
	}

	if(hAccMutex != NULL)
	{
		::CloseHandle(hAccMutex);
		hAccMutex = NULL;
	}

	return 0;
}

// 20220110 ADD~
#define PROM_FILE "C:\\Logs\\windows_exporter\\textfile_inputs\\device_status.prom"
#define OUT_FILE "C:\\Logs\\windows_exporter\\textfile_inputs\\device_status.prom.out"
#define PROM_STRING "device_status\{code\=\"%d\"\}"

int Prom_Update(int nCode, BOOL bHappen)
{
    FILE *p_file = NULL;
    char sReadBuff[512], *p, sStatBuff[512];
    char sPromStr[512];
	int find_pos;

	LOG_OUT("@@@@@ Start Prom_Update !!!");

	ifstream ifile;
    ofstream ofile;

    char line[256];
       
    ifile.open(PROM_FILE);  // 파일 열기
	ofile.open(OUT_FILE);

	sprintf(sStatBuff, PROM_STRING, nCode);

    if (ifile.is_open())
    {
		LOG_OUT("@@@@@ PROM_FILE Open success ...!!!");

		bool nFindFlag  = false;
		
		while (ifile.getline(line, sizeof(line)))	// 한 줄씩 리드
        {
            p = strstr(line, sStatBuff);
            if (p != NULL) {
				nFindFlag = true;
				LOG_OUT("@@@@@ PROM_String find !!! : %s", line);

				char sPromStr[512];
				memset(sPromStr, 0x00, sizeof(sPromStr));

				if (bHappen == TRUE)
					sprintf(sPromStr, "%s 1", sStatBuff);
				else
					sprintf(sPromStr, "%s 0", sStatBuff);

				LOG_OUT("@@@@@ MOD sPromStr !!! : %s", sPromStr);
				ofile << sPromStr << endl;	// 변경 내용 출력
			}
			else {
				ofile << line << endl;		// 기존 내용 출력
			}
        }

		if (nFindFlag == false) {
			if (bHappen == TRUE)
				sprintf(sPromStr, "%s 1", sStatBuff);
			else
				sprintf(sPromStr, "%s 0", sStatBuff);

			LOG_OUT("@@@@@ ADD sPromStr !!! : %s", sPromStr);
			ofile << sPromStr << endl;	// 변경 내용 출력
		}
    }
	else if (ofile.is_open()) {     // 파일 생성
		LOG_OUT("@@@@@ outfile Open !!!");

		if (bHappen == TRUE)
			sprintf(sPromStr, "%s 1", sStatBuff);
		else
			sprintf(sPromStr, "%s 0", sStatBuff);

		LOG_OUT("@@@@@ Before outfile write !!! : %s", sPromStr);
		ofile << sPromStr << endl;	// 내용 출력
	}
	else {
		LOG_OUT("@@@@@ PROM_FILE Open fail ...!!!");
	}

    ifile.close(); // 파일 닫기
    ofile.close(); // 파일 닫기

	BOOL bDel = ::DeleteFile( _T(PROM_FILE) );
	BOOL bMove = ::MoveFile( _T(OUT_FILE), _T(PROM_FILE) );

    return 0;
}
// 20220110 ~ADD

/**
 * @brief		SetCheckEventCode
 * @details		이벤트 코드 설정
 * @param		int nCode			이벤트 코드
 * @param		BOOL bHappen		발생/해제 유무
 * @return		항상 : 0, 실패 < 0
 */
int SetCheckEventCode(int nCode, BOOL bHappen)
{
	int nRet;
	PMYEVENTTABLE_T pTbl;

	if(pclsEvent == NULL)
	{
		return -1;
	}

	nRet = -1;

	if(bHappen == TRUE)
	{
		LOG_OUT("Set Event Code... nCode = %d, bHappen = %d ", nCode, bHappen); // 20230222 ADD
		if(!pclsEvent->GetCode(nCode))
		{
			Locking();
			nRet = pclsEvent->SetCode(nCode);
			LOG_OUT("SetCode... nRet = %d", nRet);
			UnLocking();
			if(nRet >= 0)
			{
				pTbl = &s_EvCodes[nRet]; 
			}
		}
	}
	else // 'bHappen == FALSE'인 경우
	{

		// 20230222 ADD~
		if ( (nCode == EC_RF_PAYMENT_CLR)  // SetCheckEventCode(EC_RF_PAYMENT_CLR, FALSE)인 경우, EC_RF_PAYMENT_ERR 알람해제
			|| ( nCode == EC_TCKPRT_COMM_ERR_CLR) ) // SetCheckEventCode(EC_TCKPRT_COMM_ERR_CLR, FALSE)인 경우, EC_TCKPRT_COMM_ERR 알람해제
		{
			nCode = nCode + 1; // 'bHappen == FALSE'인 경우 'nCode 값+1'을 해야 아래에서 'SetCode(nCode - 1)'시 알람이 정상적으로 해제됨 
		}
		// 20230222 ~ADD

		if(pclsEvent->GetCode(nCode))
		{
			if((nCode - 1) >= 0)
			{
				LOG_OUT("Clear Event Code... nCode = %d, bHappen = %d ", nCode, bHappen); // 20230222 ADD
				Locking();
				nRet = pclsEvent->SetCode(nCode - 1);
				// 20230222 ADD~
				//if (nCode == EC_RF_PAYMENT_CLR)
				//	nRet = pclsEvent->SetCode(nCode);			// SetCode(nCode)->장애해제, SetCode(nCode+1)->장애설정
				//else
				//	nRet = pclsEvent->SetCode(nCode - 1);
				// 20230222 ~ADD
				LOG_OUT("SetCode... nRet = %d", nRet);
				UnLocking();
			}
		}
	}

	if(nRet >= 0)
	{
		int nAlarmCode;

		UI_ALARM_INFO_T Info;

		::ZeroMemory(&Info, sizeof(UI_ALARM_INFO_T));

		pTbl = &s_EvCodes[nRet]; 

		if(pTbl->flag & EVT_ERROR)
		{
			Info.byFlag = 0x01;
			Info.byDevice = pTbl->device;

			nAlarmCode = nCode;

			if(bHappen == TRUE)
			{
				Info.byHappen = 0x01;
				sprintf((char *)Info.szCode, "A%03d", nAlarmCode);
				Prom_Update(nAlarmCode, bHappen);	// 20220110 ADD
				LOG_OUT("[장애 발생] - Code(%d : %s), ", nAlarmCode, pTbl->Contents);
			}
			else
			{
				Info.byHappen = 0x02;
				sprintf((char *)Info.szCode, "A%03d", nAlarmCode);
				Prom_Update(nAlarmCode, bHappen);	// 20220110 ADD
				LOG_OUT("[장애 해제] - Code(%d : %s), ", nAlarmCode, pTbl->Contents);
			}
			::CopyMemory(Info.szContents, pTbl->Contents, strlen(pTbl->Contents));

			UI_AddAlarmInfo((char *)&Info);
		}
		else if(pTbl->flag & EVT_WARN)
		{
			Info.byFlag = 0x00;
			Info.byDevice = pTbl->device;
			
			if(bHappen == TRUE)
			{
				Info.byHappen = 0x01;
			}
			else
			{
				Info.byHappen = 0x02;
			}
			
			sprintf((char *)Info.szCode, "W%03d", nCode);
			//::CopyMemory(Info.szCode, &pTbl->evtNo, 4);
			::CopyMemory(Info.szContents, pTbl->Contents, strlen(pTbl->Contents));

			UI_AddAlarmInfo((char *)&Info);
		}

		// SetDevStatus(pTbl->Contents, TRUE); // 20220101 ADD
	}

	return nRet;
}

/**
 * @brief		SetEventCode
 * @details		이벤트 코드 설정
 * @param		int nCode			이벤트 코드
 * @return		항상 : 0, 실패 < 0
 */
int SetEventCode(int nCode)
{
	int nRet;
	MYEVENTTABLE_T	*pEvent;

	Locking();

	if(pclsEvent == NULL)
	{
		UnLocking();
		return -1;
	}

	nRet = pclsEvent->SetCode(nCode);
	if(nRet >= 0)
	{
		pEvent = &s_EvCodes[nRet];
		if(pEvent->flag & EVT_ERROR) 
		{
		}

		if(pEvent->flag & EVT_WARN) 
		{
		}

		if( (pEvent->flag & EVT_ERROR) || (pEvent->flag & EVT_WARN) ) 
		{
			// ui alarm/event 전송
		}
	}
	UnLocking();

	return 0;
}

/**
 * @brief		GetEventCode
 * @details		이벤트 코드 값 가져오기
 * @param		int nCode			이벤트 코드
 * @return		항상 : 0, 실패 < 0
 */
int GetEventCode(int nCode)
{
	int nRet;

	if(pclsEvent == NULL)
	{
		return -1;
	}

	nRet = pclsEvent->GetCode(nCode);

	return nRet;
}

/**
 * @brief		GetCondition
 * @details		사용중 / 사용중지 상태 체크
 * @param		None
 * @return		사용중지 >= 0, 사용중 < 0
 */
int GetCondition(void)
{
	int nRet;

	if(pclsEvent == NULL)
	{
		return -1;
	}

	nRet = pclsEvent->GetCondition();

	return nRet;
}

/**
 * @brief		GetAlarmCount
 * @details		이벤트 에러 갯수
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int GetAlarmCount(void)
{
	int nRet;

	if(pclsEvent == NULL)
	{
		return -1;
	}

	nRet = pclsEvent->GetErrorCount();

	TRACE("GetAlarmCount() - Alarm Count = %d \n", nRet);

	return nRet;
}

/**
 * @brief		ClearEventCode
 * @details		해당 디바이스의 이벤트 코드 clear
 * @param		int nDevice			디바이스 구분
 * @return		None
 */
void ClearEventCode(int nDevice)
{
	int i, nCount;
	PMYEVENTTABLE_T pEvent;

	nCount = (int) (sizeof(s_EvCodes) / sizeof(MYEVENTTABLE_T));

	for(i = 0; i < nCount; i++)
	{
		pEvent = &s_EvCodes[i];

		if(i & 1)
		{
			if( pEvent->device & FLAG_ALL )
			{
				SetCheckEventCode(pEvent->evtNo - 1, FALSE);
			}
			else if( pEvent->device & nDevice )
			{
				SetCheckEventCode(pEvent->evtNo - 1, FALSE);
			}
		}
	}
}

/**
 * @brief		GetAlaramInfo
 * @details		알람 상태의 정보 획득
 * @param		char *pData			알람정보 구조체
 * @return		알람 갯수
 */
int GetAlaramInfo(char *pData)
{
	int i, nCount, nRet, k;
	PMYEVENTTABLE_T pEvent;
	PUI_ALARM_INFO_T pAlarmInfo;

	pAlarmInfo = (PUI_ALARM_INFO_T) pData;

	nCount = (int) (sizeof(s_EvCodes) / sizeof(MYEVENTTABLE_T));

	k = 0;
	for(i = 0; i < nCount; i++)
	{
		pEvent = &s_EvCodes[i];

		if(i & 1)
		{
			if(pEvent->flag & EVT_ERROR)
			{
				nRet = GetEventCode(pEvent->evtNo);
				if(nRet > 0)
				{
					pAlarmInfo[k].byFlag = 0x01;
					pAlarmInfo[k].byDevice = pEvent->device;
					pAlarmInfo[k].byHappen = 0x01;
					sprintf((char *)pAlarmInfo[k].szCode, "A%03d", pEvent->evtNo);
					sprintf((char *)pAlarmInfo[k].szContents, "%s", pEvent->Contents);

					LOG_OUT("Alarm Info = %d, %s ", pEvent->evtNo, pEvent->Contents);

					k++;
				}
			}
		}
	}

	LOG_OUT("Alarm count = %d ", k);
	return k++;
}



