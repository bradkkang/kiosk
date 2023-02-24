
#include "stdafx.h"
#include <stdio.h>
#include <string.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "MyFileUtil.h"
#include "oper_config.h"
#include "File_ini.h"

//----------------------------------------------------------------------------------------------------------------------

static VM_ENVINI_T		s_tENV;
static char				szIniFile[256];

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
 * @brief		ReadEnvIniFile
 * @details		ENV.INI 파일 읽기
 * @param		None
 * @return		항상 : 0
 */
int ReadEnvIniFile(void)
{
	int nRet;
	CString strFullName;
	PDEV_CFG_T	pDevice;

	::ZeroMemory(&s_tENV, sizeof(VM_ENVINI_T));

	Util_GetModulePath(szIniFile);
	strcat(szIniFile, "\\ENV.INI");

	TRACE("szIniFile = %s \n", szIniFile);

	strFullName = (CString)	szIniFile;
	nRet = MyAccessFile(strFullName);
	if(nRet < 0)
	{
		TRACE("szIniFile = %s File Not Found !!!!\n", szIniFile);
		return nRet;
	}

	///> 발매기 TYPE
	s_tENV.nType = GetPrivateProfileInt(SECTION_NM_MACHINE, KEY_NM_TYPE, 1, szIniFile);

	///> LCD 갯수
	s_tENV.nLcdCount = GetPrivateProfileInt(SECTION_NM_MACHINE, KEY_NM_LCD_COUNT, 1, szIniFile);

	///> 장비번호
	s_tENV.nTerminalWndNo = GetPrivateProfileInt(SECTION_NM_MACHINE, KEY_NM_TERMINAL_WND_NO, 170, szIniFile);

	///> 터미널 번호(7자리)
	s_tENV.nTerminalCode = GetPrivateProfileInt(SECTION_NM_MACHINE, KEY_NM_TERMINAL_CODE, 2224201, szIniFile);

	///> 단축터미널 번호(4자리)
	s_tENV.nShortTerminalCode = GetPrivateProfileInt(SECTION_NM_MACHINE, KEY_NM_SHORT_TERMINAL_CODE, 100, szIniFile);

	///> 버스 구분
	s_tENV.nBusSep = GetPrivateProfileInt(SECTION_NM_MACHINE, KEY_NM_BUS_SEP, 1, szIniFile);

	///> 결제방식
	s_tENV.nPayment = GetPrivateProfileInt(SECTION_NM_MACHINE, KEY_NM_PAY_METHOD, 1, szIniFile);

	///> User No
	GetPrivateProfileString(SECTION_NM_MACHINE, KEY_NM_USER_NO, "1111", s_tENV.szUserNo, sizeof(s_tENV.szUserNo) - 1, szIniFile);
	///> User Pwd
	GetPrivateProfileString(SECTION_NM_MACHINE, KEY_NM_USER_PWD, "2222", s_tENV.szUserPwd, sizeof(s_tENV.szUserPwd) - 1, szIniFile);

	// S/W FUNCTION
	{
		///> 환불 기능
		s_tENV.nIsRefund = GetPrivateProfileInt(SECTION_NM_SW_FUNCTION, KEY_NM_REFUND, 1, szIniFile);
		///> 현장발권 기능
		s_tENV.nIsPubIssue = GetPrivateProfileInt(SECTION_NM_SW_FUNCTION, KEY_NM_PUBTCK, 1, szIniFile);
		///> 예매발권 기능
		s_tENV.nIsMrnp = GetPrivateProfileInt(SECTION_NM_SW_FUNCTION, KEY_NM_MRNP, 1, szIniFile);
	}

	// (01). 동전방출기_100원
	{
		pDevice = &s_tENV.tCoin100;

		///> 사용유무
		pDevice->nUse = GetPrivateProfileInt(SECTION_NM_DEV_COIN_100, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetPrivateProfileInt(SECTION_NM_DEV_COIN_100, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetPrivateProfileInt(SECTION_NM_DEV_COIN_100, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (02). 동전방출기_500원
	{
		pDevice = &s_tENV.tCoin500;

		///> 사용유무
		pDevice->nUse = GetPrivateProfileInt(SECTION_NM_DEV_COIN_500, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetPrivateProfileInt(SECTION_NM_DEV_COIN_500, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetPrivateProfileInt(SECTION_NM_DEV_COIN_500, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (03). 지폐입금기
	{
		pDevice = &s_tENV.tBill;

		///> 사용유무
		pDevice->nUse = GetPrivateProfileInt(SECTION_NM_DEV_BILL, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetPrivateProfileInt(SECTION_NM_DEV_BILL, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetPrivateProfileInt(SECTION_NM_DEV_BILL, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (04). 지폐방출기
	{
		pDevice = &s_tENV.tDispenser;

		///> 사용유무
		pDevice->nUse = GetPrivateProfileInt(SECTION_NM_DEV_DISPENSER, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetPrivateProfileInt(SECTION_NM_DEV_DISPENSER, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetPrivateProfileInt(SECTION_NM_DEV_DISPENSER, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (05). 승차권 발권기
	{
		pDevice = &s_tENV.tPrtTicket;

		///> 사용유무
		pDevice->nUse = GetPrivateProfileInt(SECTION_NM_DEV_TICKET_PRT, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetPrivateProfileInt(SECTION_NM_DEV_TICKET_PRT, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetPrivateProfileInt(SECTION_NM_DEV_TICKET_PRT, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (06). 영수증 프린터
	{
		pDevice = &s_tENV.tPrtReceipt;

		///> 사용유무
		pDevice->nUse = GetPrivateProfileInt(SECTION_NM_DEV_RECEIPT_PRT, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetPrivateProfileInt(SECTION_NM_DEV_RECEIPT_PRT, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetPrivateProfileInt(SECTION_NM_DEV_RECEIPT_PRT, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (07). 승차권 리더기
	{
		pDevice = &s_tENV.tTicketReader;

		///> 사용유무
		pDevice->nUse = GetPrivateProfileInt(SECTION_NM_DEV_TICKET_READER, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetPrivateProfileInt(SECTION_NM_DEV_TICKET_READER, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetPrivateProfileInt(SECTION_NM_DEV_TICKET_READER, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (08). 신용카드 리더기
	{
		pDevice = &s_tENV.tCardReader;

		///> 사용유무
		pDevice->nUse = GetPrivateProfileInt(SECTION_NM_DEV_CARD_READER, KEY_NM_USE, 0, szIniFile);
		///> 모델번호 
		pDevice->nModel = GetPrivateProfileInt(SECTION_NM_DEV_CARD_READER, KEY_NM_MODEL, 0, szIniFile);
		///> 통신포트 
		pDevice->nPort = GetPrivateProfileInt(SECTION_NM_DEV_CARD_READER, KEY_NM_COM_PORT, 0, szIniFile);
	}

	// (09). UI 통신
	{
		pDevice = &s_tENV.tUI;

		///> IP Address
		GetPrivateProfileString(SECTION_NM_DEV_UI, KEY_NM_IP, "127.0.0.1", pDevice->szIPAddress, sizeof(pDevice->szIPAddress) - 1, szIniFile);
		///> TCP 포트
		pDevice->nTcpPort = GetPrivateProfileInt(SECTION_NM_DEV_UI, KEY_NM_TCP_PORT, 0, szIniFile);
	}

	return 0;
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
	POPER_FILE_CONFIG_T pOperCfg;

	pOperCfg = (POPER_FILE_CONFIG_T) pData;

	/// LCD 갯수
	{
		sprintf(Buffer, "%d", pOperCfg->baseInfo_t.byMaintMonitor);
		WritePrivateProfileString(SECTION_NM_MACHINE, KEY_NM_LCD_COUNT, Buffer, szIniFile);
	}
	/// 결제수단
	{
		sprintf(Buffer, "%d", pOperCfg->baseInfo_t.byPayMethod & 0xFF);		
		WritePrivateProfileString(SECTION_NM_MACHINE, KEY_NM_PAY_METHOD, Buffer, szIniFile);
	}

	/// 창구번호
	{
		sprintf(Buffer, "%s", pOperCfg->ccThmlInfo_t.szWndNo);
		WritePrivateProfileString(SECTION_NM_MACHINE, KEY_NM_TERMINAL_WND_NO, Buffer, szIniFile);
	}

	/// 터미널코드(7)
	{
		sprintf(Buffer, "%s", pOperCfg->ccThmlInfo_t.szCode7);
		WritePrivateProfileString(SECTION_NM_MACHINE, KEY_NM_TERMINAL_CODE, Buffer, szIniFile);
	}

	/// 단축터미널코드(4)
	{
		sprintf(Buffer, "%s", pOperCfg->ccThmlInfo_t.szCode4);
		WritePrivateProfileString(SECTION_NM_MACHINE, KEY_NM_SHORT_TERMINAL_CODE, Buffer, szIniFile);
	}

	/// 사용자번호
	{
		sprintf(Buffer, "%s", pOperCfg->ccThmlInfo_t.szUserNo);
		WritePrivateProfileString(SECTION_NM_MACHINE, KEY_NM_USER_NO, Buffer, szIniFile);
	}

	/// 사용자 비밀번호
	{
		sprintf(Buffer, "%s", pOperCfg->ccThmlInfo_t.szUserPwd);
		WritePrivateProfileString(SECTION_NM_MACHINE, KEY_NM_USER_PWD, Buffer, szIniFile);
	}

	/// 환불 사용유무
	{
		sprintf(Buffer, "%d", pOperCfg->refundMenu_t.byUse & 0xFF);		
		WritePrivateProfileString(SECTION_NM_SW_FUNCTION, KEY_NM_REFUND, Buffer, szIniFile);
	}

	/// 현장발권 사용유무
	{
		sprintf(Buffer, "%d", pOperCfg->IssMenu_t.byUse & 0xFF);		
		WritePrivateProfileString(SECTION_NM_SW_FUNCTION, KEY_NM_PUBTCK, Buffer, szIniFile);
	}

	/// 예매발권 사용유무
	{
		sprintf(Buffer, "%d", pOperCfg->ReservMenu_t.byUse & 0xFF);		
		WritePrivateProfileString(SECTION_NM_SW_FUNCTION, KEY_NM_MRNP, Buffer, szIniFile);
	}
}

/**
 * @brief		Init_IniFile
 * @details		ENV.INI 파일 초기화
 * @param		None
 * @return		항상 : 0
 */
int Init_IniFile(void)
{
	// MachineSet.ini 파일 
	ReadEnvIniFile();

	return 0;
}



