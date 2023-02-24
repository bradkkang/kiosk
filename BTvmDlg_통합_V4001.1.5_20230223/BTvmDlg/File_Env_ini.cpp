
#include "stdafx.h"
#include <stdio.h>
#include <string.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "MyFileUtil.h"
#include "oper_config.h"
#include "File_Env_ini.h"
#include "dev_tr_main.h"
#include "event_if.h"

//----------------------------------------------------------------------------------------------------------------------

#define GetProfString(A, B, C, D, E, F)		GetPrivateProfileString(A, B, C, D, E, F)
#define GetProfInt(A, B, C, D)				GetPrivateProfileInt(A, B, C, D)

#define SetProfString(A, B, C, D)			WritePrivateProfileString(A, B, C, D)

//----------------------------------------------------------------------------------------------------------------------

static KIOSK_INI_ENV_T		s_tENV;
static KIOSK_INI_TEST_T		s_tDebug;
static KIOSK_INI_PTRG_T		s_tPtrg[MAX_PTRG];

static char					szIniFile[256];
static char					szIniSvrFile[256];
static char					szIniOptFile[256];
static char					szIniPtrgFile[256];
static char					szDebugFile[256];

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		GetEnvInfo
 * @details		INI 파일정보 전달
 * @param		None
 * @return		ENV data
 */
void* GetEnvInfo(void)
{
	return &s_tENV;
}

/**
 * @brief		GetEnvOperCorp
 * @details		INI 파일 - 운영회사 정보
 * @param		None
 * @return		운영회사 정보
 */
int GetEnvOperCorp(void)
{
	return s_tENV.nOperCorp;
}

/**
 * @brief		GetEnvSvrUserNo
 * @details		ENV_SERVER.INI - User NO 값 가져오기
 * @param		None
 * @return		user no
 */
char *GetEnvSvrUserNo(int nSvrKind)
{
	if(nSvrKind == SVR_DVS_CCBUS)
	{
		return s_tENV.tCcInfo.szUserNo;
	}

	if(nSvrKind == SVR_DVS_KOBUS)
	{
		return s_tENV.tKoInfo.szUserNo;
	}

	if(nSvrKind == SVR_DVS_TMEXP)
	{
		return s_tENV.tEzInfo.szUserNo;
	}

	return (char *)NULL;
}

/**
 * @brief		GetEnvSvrUserPWD
 * @details		ENV_SERVER.INI - User NO 값 가져오기
 * @param		None
 * @return		운영회사 정보
 */
char *GetEnvSvrUserPWD(int nSvrKind)
{
	if(nSvrKind == SVR_DVS_CCBUS)
	{
		return s_tENV.tCcInfo.szUserPwd;
	}

	if(nSvrKind == SVR_DVS_KOBUS)
	{
		return s_tENV.tKoInfo.szUserPwd;
	}

	if(nSvrKind == SVR_DVS_TMEXP)
	{
		return s_tENV.tEzInfo.szUserPwd;
	}

	return (char *)NULL;
}

/**
 * @brief		GetEnvCoin100Info
 * @details		COIN_100 정보 전달
 * @param		None
 * @return		COIN 100원 정보 
 */
char* GetEnvCoin100Info(void)
{
	return (char *)&s_tENV.tCoin100;
}

/**
 * @brief		GetEnvCoin500Info
 * @details		COIN_500 정보 전달
 * @param		None
 * @return		COIN 500원 정보 
 */
char* GetEnvCoin500Info(void)
{
	return (char *)&s_tENV.tCoin500;
}

/**
 * @brief		GetEnvBillInfo
 * @details		지폐 입금기 정보 전달
 * @param		None
 * @return		BILL 정보 
 */
char* GetEnvBillInfo(void)
{
	return (char *)&s_tENV.tBill;
}

/**
 * @brief		GetEnvDispenserInfo
 * @details		지폐방출기 정보 전달
 * @param		None
 * @return		지폐방출기 정보 
 */
char* GetEnvDispenserInfo(void)
{
	return (char *)&s_tENV.tDispenser;
}

/**
 * @brief		GetEnvPrtTicketInfo
 * @details		승차권 발권기 정보 전달
 * @param		None
 * @return		승차권 발권기 정보 
 */
char* GetEnvPrtTicketInfo(void)
{
	return (char *)&s_tENV.tPrtTicket;
}

/**
 * @brief		GetEnvPrtReceiptInfo
 * @details		영수증프린터 정보 전달
 * @param		None
 * @return		영수증프린터 정보 
 */
char* GetEnvPrtReceiptInfo(void)
{
	return (char *)&s_tENV.tPrtReceipt;
}

/**
 * @brief		GetEnvTicketReaderInfo
 * @details		승차권 리더기 정보 전달
 * @param		None
 * @return		COIN 500원 정보 
 */
char* GetEnvTicketReaderInfo(void)
{
	return (char *)&s_tENV.tTicketReader;
}

/**
 * @brief		GetEnvCardReaderInfo
 * @details		신용카드 리더기 정보 전달
 * @param		None
 * @return		COIN 500원 정보 
 */
char* GetEnvCardReaderInfo(void)
{
	return (char *)&s_tENV.tCardReader;
}

/**
 * @brief		GetEnvUIInfo
 * @details		UI 정보 전달
 * @param		None
 * @return		UI 정보 
 */
char* GetEnvUIInfo(void)
{
	return (char *)&s_tENV.tUI;
}

/**
 * @brief		GetEnvTckPrtInfo
 * @details		ENV_OPT.ini 티켓프린터 옵션 정보 가져오기
 * @param		None
 * @return		UI 정보 
 */
char* GetEnvTckPrtInfo(void)
{
	return (char *)&s_tENV.tTckOpt;
}

/**
 * @brief		GetEnvPrinterInfo
 * @details		ENV_OPT.ini 영수증프린터 옵션 정보 가져오기
 * @param		None
 * @return		UI 정보 
 */
char* GetEnvPrinterInfo(void)
{
	return (char *)&s_tENV.tPrtOpt;
}

/**
 * @brief		GetEnvPrinterInfo
 * @details		ENV_OPT.ini 영수증프린터 옵션 정보의 Real Time Check 정보 가져오기
 * @param		None
 * @return		체크 > 0, 채크안함 < 0
 */
int IsEnvPrinterRealCheck(void)
{
	if( s_tENV.tPrtOpt.RealTimeCheckYN[0] != 'Y' )
		return -1;

	return 1;
}

/**
 * @brief		GetEnvServerInfo
 * @details		ENV_SERVER.INI 파일 버스 서버 구분 정보 가져오기
 * @param		None
 * @return		서버종류
 */
int GetEnvServerInfo(void)
{
	int nRet = 0;

	if(s_tENV.tCcInfo.nUse > 0)
	{
		if(s_tENV.tCcInfo.nKind == 1)
		{
			nRet += 1;	
		}
	}

	if(s_tENV.tKoInfo.nUse > 0)
	{
		if(s_tENV.tKoInfo.nKind == 2)
		{
			nRet += 2;	
		}
	}

	if(s_tENV.tEzInfo.nUse > 0)
	{
		if(s_tENV.tEzInfo.nKind == 4)
		{
			nRet += 4;	
		}
	}

	return nRet;
}

/**
 * @brief		GetEnv_IsRealMode
 * @details		무인기 디바이스 종류 가져오기 (카드전용, 현금겸용)
 * @param		None
 * @return		무인기 디바이스 정보
 */
int GetEnv_IsRealMode(void)
{
	return s_tENV.nIsRealMode;
}

/**
 * @brief		INI_ReadEnvOptionFile
 * @details		ENV_OPT.INI 파일 읽기
 * @param		None
 * @return		항상 : 0
 */
int INI_ReadEnvOptionFile(void)
{
	///> 서비스 시간
	{
		///> 무인기 서비스 시간 사용유무
		GetProfString(SECTION_NM_SERVICE, KEY_NM_SVC_USE_YN, "N", s_tENV.tService.szUseYN, sizeof(s_tENV.tService.szUseYN) - 1, szIniOptFile);

		///> 무인기 서비스 시작시간
		GetProfString(SECTION_NM_SERVICE, KEY_NM_SVC_BEGIN_TIME, "0550", s_tENV.tService.szBegTime, sizeof(s_tENV.tService.szBegTime) - 1, szIniOptFile);

		///> 무인기 서비스 종료시간
		GetProfString(SECTION_NM_SERVICE, KEY_NM_SVC_END_TIME, "2240", s_tENV.tService.szEndTime, sizeof(s_tENV.tService.szEndTime) - 1, szIniOptFile);

		///> 무인기 비배차 시간표시 옵션 (기본값;"N", 없음)																												// 20220902 ADD
		GetProfString(SECTION_NM_SERVICE, KEY_ALCN_WAY_DVS_YN, "N", s_tENV.tService.szAlcnWayDvsFlag, sizeof(s_tENV.tService.szAlcnWayDvsFlag) - 1, szIniOptFile);	// 20220902 ADD

		TR_LOG_OUT(" - INI_운영시간 사용유무 : %s ..", s_tENV.tService.szUseYN);
		TR_LOG_OUT(" - INI_운영 시작시간     : %s ..", s_tENV.tService.szBegTime);
		TR_LOG_OUT(" - INI_운영 종료시간     : %s ..", s_tENV.tService.szEndTime);

		TR_LOG_OUT(" - INI_운영 비배차 시간표시 옵션     : %s ..", s_tENV.tService.szAlcnWayDvsFlag);																	// 20220902 ADD
	}

	///> 창구마감 - reboot_flag	
	{
		///> 창구마감 YN
		GetProfString(SECTION_NM_WND_CLOSE, KEY_NM_AUTO_CLS_YN, "Y", s_tENV.tService.szAutoClseYN, sizeof(s_tENV.tService.szAutoClseYN) - 1, szIniOptFile);
		///> 창구마감 시, rebooting YN
		GetProfString(SECTION_NM_WND_CLOSE, KEY_NM_REBOOT_FLAG, "Y", s_tENV.tService.szRebootFlag, sizeof(s_tENV.tService.szRebootFlag) - 1, szIniOptFile);
		///> 창구마감 시, 마감 프린트 YN
		GetProfString(SECTION_NM_WND_CLOSE, KEY_NM_CLSPRINT_FLAG, "N", s_tENV.tService.szPrintFlag, sizeof(s_tENV.tService.szPrintFlag) - 1, szIniOptFile);
		///> 창구마감 시, 1일 1마감 사용 YN // 20210513 ADD
		GetProfString(SECTION_NM_WND_CLOSE, KEY_NM_DAYCLOSE_FLAG, "N", s_tENV.tService.szDayClsFlag, sizeof(s_tENV.tService.szDayClsFlag) - 1, szIniOptFile);
	}

	///> 프린터 옵션
	{
		///> (1). 티켓 프린트 농도 설정
		s_tENV.tTckOpt.nBoldVal = GetProfInt(SECTION_NM_TCK_OPT, KEY_NM_TCKPRT_BOLD, 0, szIniOptFile);
		TR_LOG_OUT(" - INI_프린터 농도값 : %d ..", s_tENV.tTckOpt.nBoldVal);

		///> (2). 티켓 프린트 출발일 출력 포맷 설정
		GetProfString(SECTION_NM_TCK_OPT, KEY_NM_TCKPRT_DEPR_FMT, "MMDD", s_tENV.tTckOpt.depr_date_fmt, sizeof(s_tENV.tTckOpt.depr_date_fmt) - 1, szIniOptFile);
		TR_LOG_OUT(" - INI_프린터 출발일자 FMT : %s ..", s_tENV.tTckOpt.depr_date_fmt);

		///> (3). 티켓 프린트 할부 사용유무
		GetProfString(SECTION_NM_TCK_OPT, KEY_NM_TCKPRT_MIP_YN, "Y", s_tENV.tTckOpt.mip_yn, sizeof(s_tENV.tTckOpt.mip_yn) - 1, szIniOptFile);
		TR_LOG_OUT(" - INI_프린터 할부 사용유무 : %s ..", s_tENV.tTckOpt.mip_yn);

		s_tENV.tTckOpt.n_receipt_left_gap = GetProfInt(SECTION_NM_TCK_OPT, KEY_NM_TCKPRT_LEFT_GAP, 2, szIniOptFile);
		TR_LOG_OUT(" - INI_프린터 감열지 왼쪽 간격 : %d ..", s_tENV.tTckOpt.n_receipt_left_gap);
		

		///> (4). 비좌석제 승차권 "출발일" 출력 유무
		GetProfString(SECTION_NM_TCK_OPT, KEY_NM_NSATS_DEPR_DAY_YN, "Y", s_tENV.tTckOpt.nsats_depr_day_yn, sizeof(s_tENV.tTckOpt.nsats_depr_day_yn) - 1, szIniOptFile);
		TR_LOG_OUT(" - INI_비좌석제 승차권 [출발일] 출력 유무 : %s ..", s_tENV.tTckOpt.nsats_depr_day_yn);

		///> (5). 비좌석제 승차권 "시간" 출력 유무
		GetProfString(SECTION_NM_TCK_OPT, KEY_NM_NSATS_DEPR_TIME_YN, "N", s_tENV.tTckOpt.nsats_depr_time_yn, sizeof(s_tENV.tTckOpt.nsats_depr_time_yn) - 1, szIniOptFile);
		TR_LOG_OUT(" - INI_비좌석제 승차권 [시간] 출력 유무 : %s ..", s_tENV.tTckOpt.nsats_depr_time_yn);

		///> (6). 비배차, 비좌석제 선착순탑승문구 출력 유무
		GetProfString(SECTION_NM_TCK_OPT, KEY_NM_NALCN_NSATS_FIRST_CM_YN, "Y", s_tENV.tTckOpt.nalcn_nsats_first_cm_yn, sizeof(s_tENV.tTckOpt.nalcn_nsats_first_cm_yn) - 1, szIniOptFile);
		TR_LOG_OUT(" - INI_비배차, 비좌석제 선착순탑승문구 출력 유무 : %s ..", s_tENV.tTckOpt.nalcn_nsats_first_cm_yn);

		/// 2020.06.25 add code
		///> (7). 비배차: "시간", "운수사" 출력 유무
		GetProfString(SECTION_NM_TCK_OPT, KEY_NM_NALCN_TIME_CACM_YN, "Y", s_tENV.tTckOpt.nalcn_time_cacm_yn, sizeof(s_tENV.tTckOpt.nalcn_time_cacm_yn) - 1, szIniOptFile);
		TR_LOG_OUT(" - INI_비배차: [시간], [운수사] 출력 유무 : %s ..", s_tENV.tTckOpt.nalcn_time_cacm_yn);

		///> (8). 비배차 승차권 "시간" 출력 유무
		GetProfString(SECTION_NM_TCK_OPT, KEY_NM_NALCN_DEPR_DAY_YN, "Y", s_tENV.tTckOpt.nalcn_depr_day_yn, sizeof(s_tENV.tTckOpt.nalcn_depr_day_yn) - 1, szIniOptFile);
		TR_LOG_OUT(" - INI_비배차 승차권 [출발일] 출력 유무 : %s ..", s_tENV.tTckOpt.nalcn_depr_day_yn);
	}

	///> 영수증프린터 옵션
	{
		///> (1). 실시간 상태 체크
		GetProfString(SECTION_NM_PRINTER_OPT, KEY_NM_REALCHECK_YN, "Y", s_tENV.tPrtOpt.RealTimeCheckYN, sizeof(s_tENV.tPrtOpt.RealTimeCheckYN) - 1, szIniOptFile);
	}

	///> 지폐출금시, 지폐 사용유무
	{
		s_tENV.tCashOUT.nIs100 = GetProfInt(SECTION_NM_CASH_USE, KEY_NM_W100, 1, szIniOptFile);
		s_tENV.tCashOUT.nIs500 = GetProfInt(SECTION_NM_CASH_USE, KEY_NM_W500, 1, szIniOptFile);
		s_tENV.tCashOUT.nIs1K  = GetProfInt(SECTION_NM_CASH_USE, KEY_NM_W1K, 1, szIniOptFile);
		s_tENV.tCashOUT.nIs10K = GetProfInt(SECTION_NM_CASH_USE, KEY_NM_W10K, 1, szIniOptFile);

		TR_LOG_OUT(" - INI_지폐출금시, 현금사용유무 : 100(%d), 500(%d), 1K(%d), 10K(%d)..", s_tENV.tCashOUT.nIs100, s_tENV.tCashOUT.nIs500, s_tENV.tCashOUT.nIs1K, s_tENV.tCashOUT.nIs10K);
	}

	///> 지폐입금시, 지폐 사용유무
	{
		s_tENV.tCashIN.nIs1K  = GetProfInt(SECTION_NM_CASH_IN_USE, KEY_NM_W1K, 1, szIniOptFile);
		s_tENV.tCashIN.nIs5K  = GetProfInt(SECTION_NM_CASH_IN_USE, KEY_NM_W5K, 1, szIniOptFile);
		s_tENV.tCashIN.nIs10K = GetProfInt(SECTION_NM_CASH_IN_USE, KEY_NM_W10K, 1, szIniOptFile);
		s_tENV.tCashIN.nIs50K = GetProfInt(SECTION_NM_CASH_IN_USE, KEY_NM_W50K, 1, szIniOptFile);

		TR_LOG_OUT(" - INI_지폐입금시, 현금사용유무 : 1k(%d), 5k(%d), 10k(%d), 50k(%d)..", s_tENV.tCashIN.nIs1K, s_tENV.tCashIN.nIs5K, s_tENV.tCashIN.nIs10K, s_tENV.tCashIN.nIs50K);
	}

	/// 환불 
	{
		/// 출발시간(초) 추가 
		s_tENV.tRefund.nAddDeprTm  = GetProfInt(SECTION_NM_REFUND, KEY_NM_ADD_DEPR_TIME, 0, szIniOptFile);
		///> 비좌석제 환불수수료 적용 옵션 (Y:기본값(환불수수료 기본율 적용), N:환불수수료 적용안함, INI에 정의된 값 없으면 기본값 적용)												// 20221201 ADD
		GetProfString(SECTION_NM_REFUND, KEY_REFUND_SATI_USE_YN, "Y", s_tENV.tRefund.szRefundSatiUseFlag, sizeof(s_tENV.tRefund.szRefundSatiUseFlag) - 1, szIniOptFile);	// 20221201 ADD

		TR_LOG_OUT(" - INI_환불_추가_출발시간 : nAddDeprTm(%d)..", s_tENV.tRefund.nAddDeprTm);
		TR_LOG_OUT(" - INI_환불_비좌석제 환불수수료 적용 옵션 : nAddDeprTm(%d)..", s_tENV.tRefund.szRefundSatiUseFlag);	// 20221201 ADD
	}

	return 0;
}

/**
 * @brief		INI_ReadEnvServerFile
 * @details		ENV_SERVER.INI 파일 읽기
 * @param		None
 * @return		항상 : 0
 */
int INI_ReadEnvServerFile(void)
{
	///> 시외버스 서버_CCBUS_INFO
	{
		///> 사용유무
		s_tENV.tCcInfo.nUse			= GetProfInt(SECTION_NM_CCBUS_INFO, KEY_NM_USE, 1, szIniSvrFile);
		///> 종류
		s_tENV.tCcInfo.nKind		= GetProfInt(SECTION_NM_CCBUS_INFO, KEY_NM_MODEL, 1, szIniSvrFile);
		///> 장비번호
		s_tENV.tCcInfo.nTrmlWndNo	= GetProfInt(SECTION_NM_CCBUS_INFO, KEY_NM_TRML_WND_NO, 1, szIniSvrFile);
		///> 터미널 번호(7자리)
		s_tENV.tCcInfo.nTrmlCD		= GetProfInt(SECTION_NM_CCBUS_INFO, KEY_NM_TRML_CD, 1, szIniSvrFile);
		///> 단축터미널 번호(4자리)
		s_tENV.tCcInfo.nShctTrmlCD	= GetProfInt(SECTION_NM_CCBUS_INFO, KEY_NM_SHCT_TRML_CD, 1, szIniSvrFile);
		///> 사용자 No
		GetProfString(SECTION_NM_CCBUS_INFO, KEY_NM_USER_NO, "1111", s_tENV.tCcInfo.szUserNo, sizeof(s_tENV.tCcInfo.szUserNo) - 1, szIniSvrFile);
		///> 사용자 비번
		GetProfString(SECTION_NM_CCBUS_INFO, KEY_NM_USER_PWD, "2222", s_tENV.tCcInfo.szUserPwd, sizeof(s_tENV.tCcInfo.szUserPwd) - 1, szIniSvrFile);
		///> 관리자 No
		GetProfString(SECTION_NM_CCBUS_INFO, KEY_NM_MAINT_NO, "atectn", s_tENV.tCcInfo.szMaintNo, sizeof(s_tENV.tCcInfo.szMaintNo) - 1, szIniSvrFile);
		///> 관리자 비번
		GetProfString(SECTION_NM_CCBUS_INFO, KEY_NM_MAINT_PWD, "atectn", s_tENV.tCcInfo.szMaintPwd, sizeof(s_tENV.tCcInfo.szMaintPwd) - 1, szIniSvrFile);
	}

	///> 코버스 서버_KOBUS_INFO 
	{
		///> 사용유무
		s_tENV.tKoInfo.nUse			= GetProfInt(SECTION_NM_KOBUS_INFO, KEY_NM_USE, 0, szIniSvrFile);
		///> 종류
		s_tENV.tKoInfo.nKind		= GetProfInt(SECTION_NM_KOBUS_INFO, KEY_NM_MODEL, 1, szIniSvrFile);
		///> 장비번호
		s_tENV.tKoInfo.nTrmlWndNo	= GetProfInt(SECTION_NM_KOBUS_INFO, KEY_NM_TRML_WND_NO, 0, szIniSvrFile);
		///> 터미널 번호(7자리)
		s_tENV.tKoInfo.nTrmlCD		= GetProfInt(SECTION_NM_KOBUS_INFO, KEY_NM_TRML_CD, 0, szIniSvrFile);
		///> 단축터미널 번호(4자리)
		s_tENV.tKoInfo.nShctTrmlCD	= GetProfInt(SECTION_NM_KOBUS_INFO, KEY_NM_SHCT_TRML_CD, 0, szIniSvrFile);
		///> 사용자 No
		GetProfString(SECTION_NM_KOBUS_INFO, KEY_NM_USER_NO, "1111", s_tENV.tKoInfo.szUserNo, sizeof(s_tENV.tKoInfo.szUserNo) - 1, szIniSvrFile);
		///> 사용자 비번
		GetProfString(SECTION_NM_KOBUS_INFO, KEY_NM_USER_PWD, "2222", s_tENV.tKoInfo.szUserPwd, sizeof(s_tENV.tKoInfo.szUserPwd) - 1, szIniSvrFile);
		///> 관리자 No
		GetProfString(SECTION_NM_KOBUS_INFO, KEY_NM_MAINT_NO, "atectn", s_tENV.tKoInfo.szMaintNo, sizeof(s_tENV.tKoInfo.szMaintNo) - 1, szIniSvrFile);
		///> 관리자 비번
		GetProfString(SECTION_NM_KOBUS_INFO, KEY_NM_MAINT_PWD, "atectn", s_tENV.tKoInfo.szMaintPwd, sizeof(s_tENV.tKoInfo.szMaintPwd) - 1, szIniSvrFile);
	}

	///> 티머니고속 서버_EZ_INFO 
	{
		///> 사용유무
		s_tENV.tEzInfo.nUse			= GetProfInt(SECTION_NM_EZBUS_INFO, KEY_NM_USE, 0, szIniSvrFile);
		///> 종류
		s_tENV.tEzInfo.nKind		= GetProfInt(SECTION_NM_EZBUS_INFO, KEY_NM_MODEL, 1, szIniSvrFile);
		///> 장비번호
		s_tENV.tEzInfo.nTrmlWndNo	= GetProfInt(SECTION_NM_EZBUS_INFO, KEY_NM_TRML_WND_NO, 1, szIniSvrFile);
		///> 터미널 번호(7자리)
		s_tENV.tEzInfo.nTrmlCD		= GetProfInt(SECTION_NM_EZBUS_INFO, KEY_NM_TRML_CD, 1, szIniSvrFile);
		///> 단축터미널 번호(4자리)
		s_tENV.tEzInfo.nShctTrmlCD	= GetProfInt(SECTION_NM_EZBUS_INFO, KEY_NM_SHCT_TRML_CD, 1, szIniSvrFile);
		///> 사용자 No
		GetProfString(SECTION_NM_EZBUS_INFO, KEY_NM_USER_NO, "1111", s_tENV.tEzInfo.szUserNo, sizeof(s_tENV.tEzInfo.szUserNo) - 1, szIniSvrFile);
		///> 사용자 비번
		GetProfString(SECTION_NM_EZBUS_INFO, KEY_NM_USER_PWD, "2222", s_tENV.tEzInfo.szUserPwd, sizeof(s_tENV.tEzInfo.szUserPwd) - 1, szIniSvrFile);
		///> 관리자 No
		GetProfString(SECTION_NM_EZBUS_INFO, KEY_NM_MAINT_NO, "atectn", s_tENV.tEzInfo.szMaintNo, sizeof(s_tENV.tEzInfo.szMaintNo) - 1, szIniSvrFile);
		///> 관리자 비번
		GetProfString(SECTION_NM_EZBUS_INFO, KEY_NM_MAINT_PWD, "atectn", s_tENV.tEzInfo.szMaintPwd, sizeof(s_tENV.tEzInfo.szMaintPwd) - 1, szIniSvrFile);
	}

	return 0;
}

static int INI_ParsigPtrgData(char *pData, char *retData)
{
	char			*tp, *cp;
	char			szData[100];
	PPTRG_INFO_T	pInfo;

	pInfo = (PPTRG_INFO_T) retData;

	tp = pData;

	// ID
	cp = strstr(tp, ";");
	if(cp)
	{
		::ZeroMemory(szData, sizeof(szData));
		memcpy(szData, tp, cp - tp);
		pInfo->nID = Util_Ascii2Long(szData, strlen(szData));
		tp = cp + 1;
	}

	// X
	cp = strstr(tp, ";");
	if(cp)
	{
		::ZeroMemory(szData, sizeof(szData));
		memcpy(szData, tp, cp - tp);
		pInfo->nX = Util_Ascii2Long(szData, strlen(szData));
		tp = cp + 1;
	}

	// Y
	cp = strstr(tp, ";");
	if(cp)
	{
		::ZeroMemory(szData, sizeof(szData));
		memcpy(szData, tp, cp - tp);
		pInfo->nY = Util_Ascii2Long(szData, strlen(szData));
		tp = cp + 1;
	}

	// Mode
	cp = strstr(tp, ";");
	if(cp)
	{
		::ZeroMemory(szData, sizeof(szData));
		memcpy(szData, tp, cp - tp);
		pInfo->nMode = Util_Ascii2Long(szData, strlen(szData));
		tp = cp + 1;
	}

	// Bold
	cp = strstr(tp, ";");
	if(cp)
	{
		::ZeroMemory(szData, sizeof(szData));
		memcpy(szData, tp, cp - tp);
		pInfo->nBold = Util_Ascii2Long(szData, strlen(szData));
		tp = cp + 1;
	}

	// Rotate kh_200716
	cp = strstr(tp, ";");
	if(cp)
	{
		::ZeroMemory(szData, sizeof(szData));
		memcpy(szData, tp, cp - tp);
		pInfo->nRotate = Util_Ascii2Long(szData, strlen(szData));
		tp = cp + 1;
	}

	// Msg
	cp = strstr(tp, ";");
	if(cp)
	{
		::ZeroMemory(szData, sizeof(szData));
		memcpy(szData, tp, cp - tp);
		sprintf(pInfo->szMsg, "%s", szData);
		tp = cp + 1;
	}

	/// data 유무
	pInfo->nUse = TRUE;

	return 0;
}

/**
 * @brief		INI_ReadEnvTckPtrgFile
 * @details		ENV_PTRG_XXX.INI 승차권 프린트 정보 파일 읽기
 * @param		None
 * @return		항상 : 0
 */
int INI_ReadEnvTckPtrgFile(void)
{
	int						i, k, nCount;
	DWORD					dwRet;
	char					szBuffer[256], szKeyNm[20];
	PPTRG_INFO_T			pInfo;
	POPER_FILE_CONFIG_T		pConfig;

	i = k = nCount = 0;

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	::ZeroMemory(szIniPtrgFile, sizeof(szIniPtrgFile));
	{
		Util_GetModulePath(szIniPtrgFile);

		strcat(szIniPtrgFile, "\\ENV_TCKPTRG.INI");
	}

	::ZeroMemory(&s_tPtrg[0], sizeof(KIOSK_INI_PTRG_T) * MAX_PTRG);

	for(i = 0; i < MAX_PTRG; i++)
	{
		nCount = 0;

		/// 회수용 티켓 부분
		for(k = 0; k < MAX_PTRG_ITEM; k++)
		{
			::ZeroMemory(szBuffer, sizeof(szBuffer));
			sprintf(szKeyNm, "MSG_%02d", k + 1);

			if(i == 0)
			{
				dwRet = GetProfString(SECTION_NM_CCS_PTRG_TRML, szKeyNm, "", szBuffer, sizeof(szBuffer) - 1, szIniPtrgFile);
			}
			else
			{
				dwRet = GetProfString(SECTION_NM_EXP_PTRG_TRML, szKeyNm, "", szBuffer, sizeof(szBuffer) - 1, szIniPtrgFile);
			}
			
			if(dwRet > 0)
			{
				pInfo = &s_tPtrg[i].trmlPtrg[nCount];
				INI_ParsigPtrgData(szBuffer, (char *)pInfo);

				//TR_LOG_OUT("회수용, idx(%02d), ID(%d), XY(%4d:%4d), Mode(%d), Bold(%d), (%s) ..", 
				//			nCount, pInfo->nID, pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nBold, pInfo->szMsg);
				nCount++;
			}
		}

		if(nCount <= 0)
		{
			TR_LOG_OUT("회수용, 인쇄 티켓 정보 없음 \n\n");
		}
		else
		{
			TR_LOG_OUT("회수용, 총 Item 갯수 (%d) \n\n", nCount);
		}

		nCount = 0;

		/// 승객용 티켓부분
		for(k = 0; k < MAX_PTRG_ITEM; k++)
		{
			::ZeroMemory(szBuffer, sizeof(szBuffer));
			sprintf(szKeyNm, "MSG_%02d", k + 1);
			if(i == 0)
			{
				dwRet = GetProfString(SECTION_NM_CCS_PTRG_CUST, szKeyNm, "", szBuffer, sizeof(szBuffer) - 1, szIniPtrgFile);
			}
			else
			{
				dwRet = GetProfString(SECTION_NM_EXP_PTRG_CUST, szKeyNm, "", szBuffer, sizeof(szBuffer) - 1, szIniPtrgFile);
			}

			if(dwRet > 0)
			{
				pInfo = &s_tPtrg[i].custPtrg[nCount];
				INI_ParsigPtrgData(szBuffer, (char *)pInfo);

				//TR_LOG_OUT("승객용, idx(%02d), ID(%d), XY(%4d:%4d), Mode(%d), Bold(%d), (%s) ..", 
				//			nCount, pInfo->nID, pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nBold, pInfo->szMsg);
				nCount++;
			}
		}

		if(nCount <= 0)
		{
			TR_LOG_OUT("승객용, 인쇄 티켓 정보 없음 \n\n");
		}
		else
		{
			TR_LOG_OUT("승객용, 총 Item 갯수 (%d) \n\n", nCount);
		}
	}

#if 0
	for(k = 0; k < MAX_PTRG_ITEM; k++)
	{
		PPTRG_INFO_T pInfo1;
		PPTRG_INFO_T pInfo2;

		pInfo1 = &s_tPtrg[0].trmlPtrg[k];
		TR_LOG_OUT("Dbg_회수용, idx(%02d), Use(%d), ID(%d), X(%d), Y(%d), Mode(%d) Rotate(%d) ..", k, pInfo1->nUse, pInfo1->nID, pInfo1->nX, pInfo1->nY, pInfo1->nMode, pInfo1->nRotate);

		pInfo2 = &s_tPtrg[0].custPtrg[k];
		TR_LOG_OUT("Dbg_승객용, idx(%02d), Use(%d), ID(%d), X(%d), Y(%d), Mode(%d) Rotate(%d) ..\n", k, pInfo2->nUse, pInfo2->nID, pInfo2->nX, pInfo2->nY, pInfo2->nMode, pInfo1->nRotate);
	}
	TR_LOG_OUT("\n\n");
#endif

	return 0;
}

/**
 * @brief		INI_GetEnvTckPtrgInfo
 * @details		ENV_PTRG_XXX.INI 해당 ID 정보 가져오기
 * @param		int nPtrgPart		회수용 or 승객용 정보 구분
 * @param		int nPtrgID			해당 ID 값
 * @return		성공 : 승차권 인쇄 정보, 실패 : NULL
 */
char *INI_GetEnvTckPtrgInfo(int nSvrKind, int nPtrgPart, int nPtrgID)
{
	int		i, nIndex;
	PPTRG_INFO_T pInfo;

	//TR_LOG_OUT("[#1] nSvrKind=%d, nPtrgPart=%d, nPtrgID=%d !!!", nSvrKind, nPtrgPart, nPtrgID);

	switch(nSvrKind)
	{
	case SVR_DVS_CCBUS :
		nIndex = 0;
		break;
	case SVR_DVS_KOBUS :
		nIndex = 1;
		break;
	case SVR_DVS_TMEXP :
		nIndex = 2;
		break;
	default:
		return NULL;
	}

	if(nSvrKind == SVR_DVS_CCBUS)
	{
		nIndex = 0;
	}
	else if(nSvrKind == SVR_DVS_KOBUS)
	{
		nIndex = 1;
	}
	else if(nSvrKind == SVR_DVS_TMEXP)
	{
		nIndex = 2;
	}

	for(i = 0; i < MAX_PTRG_ITEM; i++)
	{
		if(nPtrgPart == PTRG_TRML_PART)
		{
			pInfo = &s_tPtrg[nIndex].trmlPtrg[i];
		}
		else
		{
			pInfo = &s_tPtrg[nIndex].custPtrg[i];
		}

		TR_LOG_OUT("[#2] nIndex=%d, pInfo->nUse=%d, pInfo->nID=%d !!!", nIndex, pInfo->nUse, pInfo->nID);

		if( (pInfo->nUse == TRUE) && (pInfo->nID == nPtrgID) )
		{
			return (char *)pInfo;
		}
	}

	return (char *)NULL;
}

char *INI_GetEnvTckPtrg(int nSvrKind)
{
	int nIndex = -1;
	PKIOSK_INI_PTRG_T pInfo = NULL;

	/***
	switch(nSvrKind)
	{
	case SVR_DVS_CCBUS :
		pInfo = &s_tPtrg[0];
		break;
	case SVR_DVS_KOBUS :
		pInfo = &s_tPtrg[1];
		break;
	case SVR_DVS_TMEXP :
		pInfo = &s_tPtrg[2];
		break;
	}
	***/
	switch(nSvrKind)
	{
	case SVR_DVS_CCBUS :
		pInfo = &s_tPtrg[0];
		break;
	case SVR_DVS_KOBUS :
	case SVR_DVS_TMEXP :
		pInfo = &s_tPtrg[1];
		break;
	}

	return (char *)pInfo;
}

/**
 * @brief		INI_ReadEnvFile
 * @details		ENV.INI 파일 읽기
 * @param		None
 * @return		항상 : 0
 */
int INI_ReadEnvFile(void)
{
	int nRet;
	CString strFullName;
	PDEV_CFG_T	pDevice;

	::ZeroMemory(&s_tENV, sizeof(KIOSK_INI_ENV_T));

	::ZeroMemory(szIniFile, sizeof(szIniFile));
	{
		Util_GetModulePath(szIniFile);
		strcat(szIniFile, "\\ENV.INI");
	}

	::ZeroMemory(szIniSvrFile, sizeof(szIniSvrFile));
	{
		Util_GetModulePath(szIniSvrFile);
		strcat(szIniSvrFile, "\\ENV_SERVER.INI");
	}

	::ZeroMemory(szIniOptFile, sizeof(szIniOptFile));
	{
		Util_GetModulePath(szIniOptFile);
		strcat(szIniOptFile, "\\ENV_OPT.INI");
	}
	
	strFullName = (CString)	szIniFile;
	nRet = MyAccessFile(strFullName);
	if(nRet < 0)
	{
		TRACE("szIniFile = %s File Not Found !!!!\n", szIniFile);
		return nRet;
	}
	  
	///> 프로토콜 버젼
	s_tENV.nProtoVer = GetProfInt(SECTION_NM_MACHINE, KEY_NM_PROTO_VERSION, 1, szIniFile);
	///> 제조사
	GetProfString(SECTION_NM_MACHINE, KEY_NM_KIOSK_HW_DVS, "A", s_tENV.szDeviceDVS, sizeof(s_tENV.szDeviceDVS) - 1, szIniFile);
	///> [HW] IC카드 리더기 사용여부	
	GetProfString(SECTION_NM_MACHINE, KEY_NM_HW_IC_USE_YN, "Y", s_tENV.szHwIcUseYN, sizeof(s_tENV.szHwIcUseYN) - 1, szIniFile);
	///> [HW] 현금 리더기 사용여부	
	GetProfString(SECTION_NM_MACHINE, KEY_NM_HW_CASH_USE_YN, "Y", s_tENV.szHwCashUseYN, sizeof(s_tENV.szHwCashUseYN) - 1, szIniFile);
	///> [HW] RF 리더기 사용여부	
	GetProfString(SECTION_NM_MACHINE, KEY_NM_HW_RF_USE_YN, "Y", s_tENV.szHwRfUseYN, sizeof(s_tENV.szHwRfUseYN) - 1, szIniFile);
	///> 운영사 정보
	s_tENV.nOperCorp = GetProfInt(SECTION_NM_MACHINE, KEY_NM_OPER_CORP, 1, szIniFile);
	///> 서버구분 - 0:테스트 서버, 1:리얼 서버
	s_tENV.nIsRealMode = GetProfInt(SECTION_NM_MACHINE, KEY_NM_IS_REAL_MODE, 0, szIniFile);
	
	///> 결제구분 : 1(카드전용), 2(현금전용), 3(현금+카드)
	{
		///> [결제] IC 카드리더기 사용유무
		GetProfString(SECTION_NM_MACHINE, KEY_NM_SW_IC_USE_YN, "Y", s_tENV.szSwIcUseYN, sizeof(s_tENV.szSwIcUseYN) - 1, szIniFile);
		///> [결제] IC 카드리더기 사용유무
		GetProfString(SECTION_NM_MACHINE, KEY_NM_SW_CASH_USE_YN, "Y", s_tENV.szSwCashUseYN, sizeof(s_tENV.szSwCashUseYN) - 1, szIniFile);
		///> [결제] IC 카드리더기 사용유무
		GetProfString(SECTION_NM_MACHINE, KEY_NM_SW_RF_USE_YN, "Y", s_tENV.szSwRfUseYN, sizeof(s_tENV.szSwRfUseYN) - 1, szIniFile);
	}

	// (01). 동전방출기_100원
	{
		pDevice = &s_tENV.tCoin100;

		///> 사용유무
		pDevice->nUse = GetProfInt(SECTION_NM_DEV_COIN_100, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetProfInt(SECTION_NM_DEV_COIN_100, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetProfInt(SECTION_NM_DEV_COIN_100, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (02). 동전방출기_500원
	{
		pDevice = &s_tENV.tCoin500;

		///> 사용유무
		pDevice->nUse = GetProfInt(SECTION_NM_DEV_COIN_500, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetProfInt(SECTION_NM_DEV_COIN_500, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetProfInt(SECTION_NM_DEV_COIN_500, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (03). 지폐입금기
	{
		pDevice = &s_tENV.tBill;

		///> 사용유무
		pDevice->nUse = GetProfInt(SECTION_NM_DEV_BILL, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetProfInt(SECTION_NM_DEV_BILL, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetProfInt(SECTION_NM_DEV_BILL, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (04). 지폐방출기
	{
		pDevice = &s_tENV.tDispenser;

		///> 사용유무
		pDevice->nUse = GetProfInt(SECTION_NM_DEV_DISPENSER, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetProfInt(SECTION_NM_DEV_DISPENSER, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetProfInt(SECTION_NM_DEV_DISPENSER, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (05). 승차권 발권기
	{
		pDevice = &s_tENV.tPrtTicket;

		///> 사용유무
		pDevice->nUse = GetProfInt(SECTION_NM_DEV_TICKET_PRT, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetProfInt(SECTION_NM_DEV_TICKET_PRT, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetProfInt(SECTION_NM_DEV_TICKET_PRT, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (06). 영수증 프린터
	{
		pDevice = &s_tENV.tPrtReceipt;

		///> 사용유무
		pDevice->nUse = GetProfInt(SECTION_NM_DEV_RECEIPT_PRT, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetProfInt(SECTION_NM_DEV_RECEIPT_PRT, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetProfInt(SECTION_NM_DEV_RECEIPT_PRT, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (07). 승차권 리더기
	{
		pDevice = &s_tENV.tTicketReader;

		///> 사용유무
		pDevice->nUse = GetProfInt(SECTION_NM_DEV_TICKET_READER, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetProfInt(SECTION_NM_DEV_TICKET_READER, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetProfInt(SECTION_NM_DEV_TICKET_READER, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (08). 신용카드 리더기
	{
		pDevice = &s_tENV.tCardReader;

		///> 사용유무
		pDevice->nUse = GetProfInt(SECTION_NM_DEV_CARD_READER, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetProfInt(SECTION_NM_DEV_CARD_READER, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetProfInt(SECTION_NM_DEV_CARD_READER, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (09). RF
	{
		pDevice = &s_tENV.tRF;

		///> 사용유무
		pDevice->nUse = GetProfInt(SECTION_NM_DEV_RF, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetProfInt(SECTION_NM_DEV_RF, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetProfInt(SECTION_NM_DEV_RF, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (10). UI 통신
	{
		pDevice = &s_tENV.tUI;

		///> IP Address
		GetProfString(SECTION_NM_DEV_UI, KEY_NM_IP, "127.0.0.1", pDevice->szIPAddress, sizeof(pDevice->szIPAddress) - 1, szIniFile);
		///> TCP 포트
		pDevice->nTcpPort = GetProfInt(SECTION_NM_DEV_UI, KEY_NM_TCP_PORT, 0, szIniFile);
	}

	/// 옵션 정보 
	INI_ReadEnvOptionFile();

	/// 서버 정보
	INI_ReadEnvServerFile();

	/// 승차권 프린트 정보
	INI_ReadEnvTckPtrgFile();

	return 0;
}

/**
 * @brief		INI_ReadDebugFile
 * @details		Debug.INI 파일 읽기
 * @param		None
 * @return		항상 : 0
 */
int INI_ReadDebugFile(void)
{
	int nRet;
	CString strFullName;

	::ZeroMemory(&s_tDebug, sizeof(KIOSK_INI_TEST_T));

	Util_GetModulePath(szDebugFile);
	strcat(szDebugFile, "\\Debug.INI");

	strFullName = (CString)	szDebugFile;
	nRet = MyAccessFile(strFullName);
	if(nRet < 0)
	{
		TR_LOG_OUT("szDebugFile = %s File Not Found !!!!", szDebugFile);
		return nRet;
	}

	///> 1. 현장발권 
	{
		///> 배차_테스트
		s_tDebug.pbTck.nAlcn = GetProfInt(SECTION_NM_DBG_PBTCK, KEY_NM_DBG_ALCN, 0, szDebugFile);
	}

	///> 2. 환불
	{
		///> 소인여부
		s_tDebug.reFund.bBranding = GetProfInt(SECTION_NM_DBG_REFUND, KEY_NM_DBG_BRANDING, 0, szDebugFile);
		///> 소인 이미지 파일명
		GetProfString(SECTION_NM_DBG_REFUND, KEY_NM_DBG_BRANDING_IMG, "void.bmp", s_tDebug.reFund.szBrandImg, sizeof(s_tDebug.reFund.szBrandImg) - 1, szDebugFile);
	}

	return 0;
}

/**
 * @brief		GetDebugInfo
 * @details		디버그 정보 가져오기
 * @param		None
 * @return		항상 : 0
 */
void *GetDebugInfo(int nLevel)
{
	if( nLevel == 0 )
	{
		return &s_tDebug;
	}
	else if( nLevel == 1 )
	{
		return &s_tDebug.pbTck;
	}
	else if( nLevel == 2 )
	{
		return &s_tDebug.reFund;
	}

	return &s_tDebug;
}

/**
 * @brief		WriteEnvIniFile
 * @details		ENV.INI 데이타 저장
 * @param		char *pValue
 * @return		항상 : 0
 */
void WriteEnvServerIniFile(char *pData)
{
	char Buffer[100];
//	CString strData;
	POPER_FILE_CONFIG_T pOperCfg;

	pOperCfg = (POPER_FILE_CONFIG_T) pData;

	///> 1. 시외서버
	{
		/// (1-01). 사용유무
		s_tENV.tCcInfo.nUse = 0;
		if( pOperCfg->base_t.ccs_svr_kind != 0x30 )
		{
			s_tENV.tCcInfo.nUse = 1;
		}
		sprintf(Buffer, "%d", s_tENV.tCcInfo.nUse);
		WritePrivateProfileString(SECTION_NM_CCBUS_INFO, KEY_NM_USE, Buffer, szIniSvrFile);

		/// (1-02). 종류
		sprintf(Buffer, "%d", 1);
		WritePrivateProfileString(SECTION_NM_CCBUS_INFO, KEY_NM_MODEL, Buffer, szIniSvrFile);

		/// (1-03). 창구번호
		sprintf(Buffer, "%s", pOperCfg->ccTrmlInfo_t.szWndNo);
		WritePrivateProfileString(SECTION_NM_CCBUS_INFO, KEY_NM_TRML_WND_NO, Buffer, szIniSvrFile);

		/// (1-04). 터미널번호 (7)
		sprintf(Buffer, "%s", pOperCfg->ccTrmlInfo_t.szCode7);
		WritePrivateProfileString(SECTION_NM_CCBUS_INFO, KEY_NM_TRML_CD, Buffer, szIniSvrFile);

		/// (1-05). 단축 터미널번호 (4)
		sprintf(Buffer, "%s", pOperCfg->ccTrmlInfo_t.szCode4);
		WritePrivateProfileString(SECTION_NM_CCBUS_INFO, KEY_NM_SHCT_TRML_CD, Buffer, szIniSvrFile);

		/// (1-06). user_no
		sprintf(Buffer, "%s", pOperCfg->ccTrmlInfo_t.szUserNo);
		WritePrivateProfileString(SECTION_NM_CCBUS_INFO, KEY_NM_USER_NO, Buffer, szIniSvrFile);

		/// (1-07). user_pwd
		sprintf(Buffer, "%s", pOperCfg->ccTrmlInfo_t.szUserPwd);
		WritePrivateProfileString(SECTION_NM_CCBUS_INFO, KEY_NM_USER_PWD, Buffer, szIniSvrFile);
	}

	///> 2. 코버스 고속서버
	{
		/// (2-01). 사용유무
		s_tENV.tKoInfo.nUse = 0;
		if( pOperCfg->base_t.exp_svr_kind == 0x32 )
		{
			s_tENV.tKoInfo.nUse = 1;
		}
		sprintf(Buffer, "%d", s_tENV.tKoInfo.nUse);
		WritePrivateProfileString(SECTION_NM_KOBUS_INFO, KEY_NM_USE, Buffer, szIniSvrFile);

		/// (2-02). 종류
		sprintf(Buffer, "%d", 2);
		WritePrivateProfileString(SECTION_NM_KOBUS_INFO, KEY_NM_MODEL, Buffer, szIniSvrFile);

		/// (2-03). 창구번호
		sprintf(Buffer, "%s", pOperCfg->koTrmlInfo_t.szWndNo);
		WritePrivateProfileString(SECTION_NM_KOBUS_INFO, KEY_NM_TRML_WND_NO, Buffer, szIniSvrFile);

		/// (2-04). 터미널번호 (7)
		sprintf(Buffer, "%s", pOperCfg->koTrmlInfo_t.szCode7);
		WritePrivateProfileString(SECTION_NM_KOBUS_INFO, KEY_NM_TRML_CD, Buffer, szIniSvrFile);

		/// (2-05). 단축 터미널번호 (4)
		sprintf(Buffer, "%s", pOperCfg->koTrmlInfo_t.szCode4);
		WritePrivateProfileString(SECTION_NM_KOBUS_INFO, KEY_NM_SHCT_TRML_CD, Buffer, szIniSvrFile);

		/// (2-06). user_no
		sprintf(Buffer, "%s", pOperCfg->koTrmlInfo_t.szUserNo);
		WritePrivateProfileString(SECTION_NM_KOBUS_INFO, KEY_NM_USER_NO, Buffer, szIniSvrFile);

		/// (2-07). user_pwd
		sprintf(Buffer, "%s", pOperCfg->koTrmlInfo_t.szUserPwd);
		WritePrivateProfileString(SECTION_NM_KOBUS_INFO, KEY_NM_USER_PWD, Buffer, szIniSvrFile);
	}

	///> 3. 이지 고속서버
	{
		/// (3-01). 사용유무
		s_tENV.tEzInfo.nUse = 0;
		if( pOperCfg->base_t.exp_svr_kind == 0x31 )
		{
			s_tENV.tEzInfo.nUse = 1;
		}
		sprintf(Buffer, "%d", s_tENV.tEzInfo.nUse);
		WritePrivateProfileString(SECTION_NM_EZBUS_INFO, KEY_NM_USE, Buffer, szIniSvrFile);

		/// (3-02). 종류
		sprintf(Buffer, "%d", 4);
		WritePrivateProfileString(SECTION_NM_EZBUS_INFO, KEY_NM_MODEL, Buffer, szIniSvrFile);

		/// (3-03). 창구번호
		sprintf(Buffer, "%s", pOperCfg->ezTrmlInfo_t.szWndNo);
		WritePrivateProfileString(SECTION_NM_EZBUS_INFO, KEY_NM_TRML_WND_NO, Buffer, szIniSvrFile);

		/// (3-04). 터미널번호 (7)
		sprintf(Buffer, "%s", pOperCfg->ezTrmlInfo_t.szCode7);
		WritePrivateProfileString(SECTION_NM_EZBUS_INFO, KEY_NM_TRML_CD, Buffer, szIniSvrFile);

		/// (3-05). 단축 터미널번호 (4)
		sprintf(Buffer, "%s", pOperCfg->ezTrmlInfo_t.szCode4);
		WritePrivateProfileString(SECTION_NM_EZBUS_INFO, KEY_NM_SHCT_TRML_CD, Buffer, szIniSvrFile);

		/// (3-06). user_no
		sprintf(Buffer, "%s", pOperCfg->ezTrmlInfo_t.szUserNo);
		WritePrivateProfileString(SECTION_NM_EZBUS_INFO, KEY_NM_USER_NO, Buffer, szIniSvrFile);

		/// (3-07). user_pwd
		sprintf(Buffer, "%s", pOperCfg->ezTrmlInfo_t.szUserPwd);
		WritePrivateProfileString(SECTION_NM_EZBUS_INFO, KEY_NM_USER_PWD, Buffer, szIniSvrFile);
	}
}


/**
 * @brief		WriteEnvIniFile
 * @details		ENV.INI 데이타 저장
 * @param		char *pValue
 * @return		항상 : 0
 */
void WriteEnvIniFile(char *pData)
{
	char Buffer[100];
	CString strData;
	POPER_FILE_CONFIG_T pOperCfg;

	pOperCfg = (POPER_FILE_CONFIG_T) pData;

	/// [결제] IC 카드리더기 사용유무
	{
		//::ZeroMemory(Buffer, sizeof(Buffer));
		sprintf(Buffer, "%c", pOperCfg->base_t.sw_ic_pay_yn);
		strData.Format(_T("%c"), pOperCfg->base_t.sw_ic_pay_yn);
		TR_LOG_OUT("Buffer - sw_ic_pay_yn = %s", Buffer);
		//Buffer[0] = pOperCfg->base_t.sw_ic_pay_yn & 0xFF;
		//WritePrivateProfileString(SECTION_NM_MACHINE, KEY_NM_SW_IC_USE_YN, Buffer, szIniFile);
		WritePrivateProfileString(SECTION_NM_MACHINE, KEY_NM_SW_IC_USE_YN, strData, szIniFile);
	}

	/// [결제] 현금 사용유무
	{
		//::ZeroMemory(Buffer, sizeof(Buffer));
		sprintf(Buffer, "%c", pOperCfg->base_t.sw_cash_pay_yn);
		strData.Format(_T("%c"), pOperCfg->base_t.sw_cash_pay_yn);
		TR_LOG_OUT("Buffer - sw_cash_pay_yn = %s", Buffer);
		//Buffer[0] = pOperCfg->base_t.sw_cash_pay_yn & 0xFF;
		WritePrivateProfileString(SECTION_NM_MACHINE, KEY_NM_SW_CASH_USE_YN, strData, szIniFile);
	}

	/// [결제] RF 사용유무
	{
		//::ZeroMemory(Buffer, sizeof(Buffer));
		sprintf(Buffer, "%c", pOperCfg->base_t.sw_rf_pay_yn);
		TR_LOG_OUT("Buffer - sw_rf_pay_yn = %s", Buffer);
		//Buffer[0] = pOperCfg->base_t.sw_rf_pay_yn & 0xFF;
		WritePrivateProfileString(SECTION_NM_MACHINE, KEY_NM_SW_RF_USE_YN, Buffer, szIniFile);
	}

	WriteEnvServerIniFile(pData);

	INI_ReadEnvFile();
}

/**
 * @brief		Init_EnvIniFile
 * @details		ENV.INI 파일 초기화
 * @param		None
 * @return		항상 : 0
 */
int Init_EnvIniFile(void)
{
	// ENV.ini 파일 
	INI_ReadEnvFile();

	INI_ReadDebugFile();

	return 0;
}

/**
 * @brief		Term_EnvIniFile
 * @details		종료
 * @param		None
 * @return		항상 : 0
 */
int Term_EnvIniFile(void)
{
	return 0;
}

/**
 * @brief		CheckServiceTime
 * @details		무인기 서비스 시간 체크
 * @param		None
 * @return		항상 : 0
 */
int CheckServiceTime(void)
{
	int			nRet, nLcdAct;
	SYSTEMTIME	st;
	char		szBuffer[50];
	int			nSTime, nETime, nCTime;
	char		*pSTime;
	char		*pETime;

	nRet = 0;
	nLcdAct = -1;

	if(s_tENV.tService.szUseYN[0] != 'Y')
	{
		//TR_LOG_OUT("s_tENV.tService.szUseYN[0] = (%c)..", s_tENV.tService.szUseYN[0]);
		return 0;
	}

	::GetLocalTime(&st);
	sprintf(szBuffer, "%02d%02d", st.wHour, st.wMinute);
	nCTime = Util_Ascii2Long(szBuffer, strlen(szBuffer));

	pSTime = s_tENV.tService.szBegTime;
	nSTime = Util_Ascii2Long(pSTime, strlen(pSTime));

	pETime = s_tENV.tService.szEndTime;
	nETime = Util_Ascii2Long(pETime, strlen(pETime));

#if 1

	if( nSTime < nETime )
	{
		if( (nCTime >= nSTime) && (nCTime < nETime) )
		{
			/// in 서비스 시간
			nRet = SetCheckEventCode(EC_OP_CLOSE, FALSE);
			if(nRet >= 0)
			{
				/// LCD ON
				nLcdAct = 1;
			}
		}
		else
		{
			/// out 서비스 시간
			nRet = SetCheckEventCode(EC_OP_CLOSE, TRUE);
			if(nRet >= 0)
			{
				/// LCD OFF
				nLcdAct = 0;
			}
		}
	}
	else
	{
		/***
		if( nCTime >= nSTime )
		{	/// in 서비스 시간
			nRet = SetCheckEventCode(EC_OP_CLOSE, FALSE);
			if(nRet >= 0)
			{
				/// LCD ON
				nLcdAct = 1;
			}
		}
		else
		{
			if( nCTime < nETime )
			{	/// in 서비스 시간
				nRet = SetCheckEventCode(EC_OP_CLOSE, FALSE);
				if(nRet >= 0)
				{
					/// LCD ON
					nLcdAct = 1;
				}
			}
			else
			{	/// out 서비스 시간
				nRet = SetCheckEventCode(EC_OP_CLOSE, TRUE);
				if(nRet >= 0)
				{
					/// LCD OFF
					nLcdAct = 0;
				}
			}
		}
		**/
		if( (nETime <= nCTime) && (nCTime < nSTime) )
		{	/// out 서비스 시간
			nRet = SetCheckEventCode(EC_OP_CLOSE, TRUE);
			if(nRet >= 0)
			{
				/// LCD OFF
				nLcdAct = 0;
			}
		}
		else
		{	/// in 서비스 시간
			nRet = SetCheckEventCode(EC_OP_CLOSE, FALSE);
			if(nRet >= 0)
			{
				/// LCD ON
				nLcdAct = 1;
			}
		}

	}

	if( nLcdAct != -1 )
	{
		TR_LOG_OUT("#1 시작시간(%04d), 종료시간(%04d), 현재시간(%04d), nLcdAct(%d)..", nSTime, nETime, nCTime, nLcdAct);

		if( nLcdAct == 1 )
		{
			Util_MonitorControl(1);
		}
		else
		{
			Util_MonitorControl(0);
		}
		TR_LOG_OUT("#2 시작시간(%04d), 종료시간(%04d), 현재시간(%04d), nLcdAct(%d)..", nSTime, nETime, nCTime, nLcdAct);
	}

#endif

	return 0;
}

/**
 * @brief		INI_Use10K
 * @details		지폐 출금시, 10,000원 사용유무
 * @param		None
 * @return		사용 > 0, 미사용 <= 0
 */
int INI_Use10K(void)
{
	return s_tENV.tCashOUT.nIs10K;
}

/**
 * @brief		INI_Use1K
 * @details		지폐 입금시, 1,000원 사용유무
 * @param		None
 * @return		사용 > 0, 미사용 <= 0
 */
int INI_UseCashIn1K(void)
{
	return s_tENV.tCashIN.nIs1K;
}

/**
 * @brief		INI_Use5K
 * @details		지폐 입금시, 5,000원 사용유무
 * @param		None
 * @return		사용 > 0, 미사용 <= 0
 */
int INI_UseCashIn5K(void)
{
	return s_tENV.tCashIN.nIs5K;
}

/**
 * @brief		INI_Use10K
 * @details		지폐 입금시, 10,000원 사용유무
 * @param		None
 * @return		사용 > 0, 미사용 <= 0
 */
int INI_UseCashIn10K(void)
{
	return s_tENV.tCashIN.nIs10K;
}

/**
 * @brief		INI_Use50K
 * @details		지폐 입금시, 50,000원 사용유무
 * @param		None
 * @return		사용 > 0, 미사용 <= 0
 */
int INI_UseCashIn50K(void)
{
	return s_tENV.tCashIN.nIs50K;
}

