// 
// 
// Prt_TcketHS.cpp : 승차권 프린터 (화성)
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "cmn_util.h"
#include "dev_prt_ticket_hs.h"
#include "dev_tr_mem.h"
#include "dev_tr_kobus_mem.h"
#include "dev_tr_main.h"
#include "event_if.h"
#include "oper_config.h"
#include "File_Env_ini.h"

//----------------------------------------------------------------------------------------------------------------------

#define LOG_OUT(fmt, ...)		{ CPrtTicketHS::m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ CPrtTicketHS::m_clsLog.HexaDump(x, y, z); }

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CPrtTicketHS
 * @details		생성자
 */
CPrtTicketHS::CPrtTicketHS()
{
	m_hThread = NULL;
	m_hAccMutex = NULL;
	m_bConnected = FALSE;
}

/**
 * @brief		~CPrtTicketHS
 * @details		소멸자
 */
CPrtTicketHS::~CPrtTicketHS()
{
	LOG_OUT(" start !! ");
	EndProcess();
}

/**
 * @brief		LOG_INIT
 * @details		LOG 파일 초기화
 * @param		None
 * @return		None
 */
void CPrtTicketHS::LOG_INIT(void)
{
	m_clsLog.SetData(30, "\\Log\\TcketPrt\\Hs");
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
int CPrtTicketHS::Locking(void)
{
	if(m_hAccMutex != NULL) 
	{
		::WaitForSingleObject(m_hAccMutex, INFINITE);
	}

	return 0;	
}

/**
 * @brief		UnLocking
 * @details		IPC UnLock
 * @param		None
 * @return		항상 : 0
 */
int CPrtTicketHS::UnLocking(void)
{
	if(m_hAccMutex != NULL) 
	{
		::ReleaseMutex(m_hAccMutex);
	}

	return 0;	
}

/**
 * @brief		SendData
 * @details		데이타 전송 
 * @param		BYTE *pData		전송 데이타
 * @param		int nDataLen	전송 데이타 길이
 * @return		성공 > 0, 실패 < 0
 */
int CPrtTicketHS::SendData(BYTE *pData, int nDataLen)
{
	//LOG_HEXA("SendData", pData, nDataLen);
	return m_clsComm.SendData(pData, nDataLen);
}

/**
 * @brief		GetStatus
 * @details		프린터 상태 명령 전송
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrtTicketHS::GetStatus(void)
{
	int nRet, ch;
	DWORD dwTick;
	static int prev_ch = -1;

	PurgeComm(m_clsComm.m_hCommHandle, PURGE_TXABORT | PURGE_RXCLEAR);
	Sleep(300);

	nRet = SendData((BYTE *)"\x1D\x61\x01", 3);

	//LOG_OUT("%s", "Start !!!!");

	nRet = 1;

	dwTick = GetTickCount();
	while( Util_CheckExpire(dwTick) < 500 )	
	{
		ch = m_clsComm.ReadData();
		if(ch < 0) 
		{
			continue;
		}

		if(prev_ch != ch)
		{
			prev_ch = ch;
			LOG_OUT("Recv = %02X", ch & 0xFF);
		}

		SetCheckEventCode(EC_TCKPRT_COMM_ERR, FALSE);

		ch = ch & 0xFF;

		//
		if(ch & 0x01)
		{	/// 용지없음
			SetCheckEventCode(EC_TCKPRT_NO_PAPER, TRUE);
			nRet = -1;
		}
		else
		{
			SetCheckEventCode(EC_TCKPRT_NO_PAPER, FALSE);
		}
		//
		if(ch & 0x02)
		{	/// 프린트 헤드 업
			SetCheckEventCode(EC_TCKPRT_HEAD_UP, TRUE);
			nRet = -2;
		}
		else
		{
			SetCheckEventCode(EC_TCKPRT_HEAD_UP, FALSE);
		}
		//
		if(ch & 0x04)
		{	/// 용지 잼
			SetCheckEventCode(EC_TCKPRT_PAPER_JAM, TRUE);
			nRet = -3;
		}
		else
		{
			SetCheckEventCode(EC_TCKPRT_PAPER_JAM, FALSE);
		}
		//
		if(ch & 0x08)
		{	/// 용지 Near END
			SetCheckEventCode(EC_TCKPRT_PAPER_NEAR_END, TRUE);
		}
		else
		{
			SetCheckEventCode(EC_TCKPRT_PAPER_NEAR_END, FALSE);
		}
		//
		if(ch & 0x10)
		{	/// 프린트 또는 Feeding 중
			TRACE("프린트 또는 Feeding 중 \n");
		}
		//
		if(ch & 0x20)
		{	/// 컷터 에러(잼)4
			SetCheckEventCode(EC_TCKPRT_CUTTER_ERR, TRUE);
			nRet = -4;
		}
		else
		{
			SetCheckEventCode(EC_TCKPRT_CUTTER_ERR, FALSE);
		}

		//
		if(ch & 0x80)
		{
			TRACE("보조센서에 용지 있음 \n");
		}

		return nRet;
	}

	LOG_OUT("%s", "Recv Timeout");

	SetCheckEventCode(EC_TCKPRT_COMM_ERR, TRUE);

	return -5;
}

/**
 * @brief		SetBold
 * @details		Set 인쇄농도 설정
 * @param		int nValue			농도설정값 (0 ~ 5)
 * @return		None
 */
void CPrtTicketHS::SetBold(int nValue)
{
	BYTE szData[] = "\x1D\x28\x4b\x02\x00\x31\x00";

	szData[6] = nValue & 0x05;

	SendData((BYTE *)szData, 7);
}


/**
 * @brief		SetDoubleFont
 * @details		Double 폰트 사용유무 설정
 * @param		BOOL bSet			설정유무 Flag
 * @return		None
 */
void CPrtTicketHS::SetDoubleFont(BOOL bSet)
{
	if(bSet == TRUE)
	{
		SendData((BYTE *)"\x1D\x21\x11", 3);
	}
	else
	{
		SendData((BYTE *)"\x1D\x21\x00", 3);
	}
}

void CPrtTicketHS::SetDoubleFont(int nFont, BOOL bSet)
{
	if(bSet == TRUE)
	{
		switch(nFont)
		{
		case FONT_VERTI_EXPAND:
			SendData((BYTE *)"\x1D\x21\x10", 3);
			break;
		case FONT_HORI_EXPAND:
			SendData((BYTE *)"\x1D\x21\x01", 3);
			break;
		case FONT_BOTH_EXPAND:
			SendData((BYTE *)"\x1D\x21\x11", 3);
			break;
		}
	}
	else
	{
		SendData((BYTE *)"\x1D\x21\x00", 3);
	}
}

/**
 * @brief		SetSmallFont
 * @details		Small 폰트 사용유무 설정
 * @param		BOOL bSet			설정유무 Flag
 * @return		None
 */
void CPrtTicketHS::SetSmallFont(BOOL bSet)
{
	if(bSet == TRUE)
	{
		SendData((BYTE *)"\x1B\x4D\x01", 3);
	}
	else
	{
		SendData((BYTE *)"\x1B\x4D\x00", 3);
	}
}

/**
 * @brief		Print
 * @details		문자 출력
 * @param		int nX				x 좌표
 * @param		int nY				y 좌표
 * @param		const char *Format  프린트 문자열
 * @return		항상 = 0
 */
int CPrtTicketHS::Print(const char *Format, ... )
{
	va_list ArgList;
	int nOffset, len, nRet;

	nOffset = len = nRet = 0;
	::ZeroMemory(m_szTxBuf, sizeof(m_szTxBuf));

	va_start(ArgList, Format);
	{
		len = _vscprintf(Format, ArgList) + 1;
		vsprintf_s((char *)&m_szTxBuf[nOffset], len, Format, ArgList);
		nOffset += strlen((char *)m_szTxBuf);
	}
	va_end(ArgList);

	SendData(m_szTxBuf, nOffset);

#if (_KTC_CERTIFY_ > 0)
	KTC_MemClear(m_szTxBuf, sizeof(m_szTxBuf));
#endif

	return 0;
}

/**
 * @brief		txtPrint
 * @details		승차권 프린터에 출력
 * @param		int nX				x 좌표
 * @param		int nY				y 좌표
 * @param		char *pStr			프린트 데이타
 * @return		항상 = 0
 */
int CPrtTicketHS::txtPrint(int nX, int nY, int nMode, char *pStr)
{
	int nOffset = 0;

	/// FONT 설정
	switch(nMode)
	{
	case PRT_MODE_VERTI_EXPAND	:
		SendData((BYTE *)"\x1D\x21\x10", 3);
		break;
	case PRT_MODE_HORI_EXPAND	:
		SendData((BYTE *)"\x1D\x21\x01", 3);
		break;
	case PRT_MODE_BOTH_EXPAND	:
		SendData((BYTE *)"\x1D\x21\x11", 3);
		break;
	case PRT_MODE_SMALL_ON		:
		SendData((BYTE *)"\x1B\x4D\x01", 3);
		break;
	case PRT_MODE_SMALL_OFF		:
		SendData((BYTE *)"\x1B\x4D\x00", 3);
		break;
	case PRT_MODE_NONE			:
	default:
		SendData((BYTE *)"\x1D\x21\x00", 3);
		break;
	}

	// 문자 프린트
	LOG_OUT("좌표(%d, %d) : %s", nX, nY, pStr);

	nOffset += sprintf((char *)&m_szTxBuf[nOffset], "\x1B\x57%04d%04d%s", nX, nY, pStr);
	SendData(m_szTxBuf, nOffset);

#if (_KTC_CERTIFY_ > 0)
	KTC_MemClear(m_szTxBuf, sizeof(m_szTxBuf));
#endif

	/// FONT 해제
	switch(nMode)
	{
	case PRT_MODE_SMALL_ON		:
		SendData((BYTE *)"\x1B\x4D\x00", 3);
		break;
	default:
		SendData((BYTE *)"\x1D\x21\x00", 3);
		break;
	}

	return 0;
}

/**
 * @brief		MultiPrint
 * @details		다중 라인 데이타 출력
 * @param		int nX				x 좌표
 * @param		int nY				y 좌표
 * @param		int nMaxMode		
 * @param		int nMinMode		
 * @param		int nMAX			데이타 최대 길이
 * @param		char *pStr			데이타
 * @return		항상 = 0
 */
int CPrtTicketHS::MultiPrint(int nX, int nY, int nMaxMode, int nMinMode, int nMAX, char *pData, BOOL bExpJun)
{
	int i, nLen, nRow, nOffset, nBufLen, nRet;
	char Buffer[100];

	i = nLen = nRow = nOffset = nBufLen = nRet = 0;

	nLen = strlen(pData);
	LOG_OUT("nMAX = %d, length = %d, data = (%s) ", nMAX, nLen, pData);
	if(nLen > nMAX)
	{
		nRow = nLen / nMAX;
		if( nLen % nMAX )
		{
			nRow += 1;
		}

		if( nRow > 2 )
		{
			nRow = 2;
		}

		nOffset = 0;
		LOG_OUT("nRow = %d ", nRow);
		for(i = 0; i < nRow; i++)
		{
			::ZeroMemory(Buffer, sizeof(Buffer));

			if( (nOffset + (nMAX * (i + 1))) < nLen )
			{
				nBufLen = nMAX;
			}
			else
			{
				nBufLen = nLen - nOffset;
			}

			LOG_OUT("nOffset = %d, i = %d, nBufLen = %d ", nOffset, i, nBufLen);

			nRet = IsHangule(&pData[nOffset], nBufLen);
			LOG_OUT("IsHangule(), nRet = %d ", nRet);
			if(nRet < 0)
			{
				::CopyMemory(Buffer, &pData[nOffset], nBufLen - 1);
				nOffset += (nBufLen - 1);
			}
			else
			{
				::CopyMemory(Buffer, &pData[nOffset], nBufLen);
				nOffset += nBufLen;
			}

			if(bExpJun == FALSE)
			{
				if(nMinMode == PRT_MODE_NONE)
					txtPrint(nX-(i*25)+5, nY, nMinMode, Buffer);
				else
					txtPrint(nX-(i*35)+5, nY, nMinMode, Buffer);
			}
			else
			{
				if(nMinMode == PRT_MODE_NONE)
					txtPrint(nX+(i*25)+5, nY, nMinMode, Buffer);
				else
					txtPrint(nX+(i*35)+5, nY, nMinMode, Buffer);
			}
		}
	}
	else
	{
		txtPrint(nX, nY, nMaxMode, pData);
	}
	return 0;
}

/**
 * @brief		StartProcess
 * @details		Start
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrtTicketHS::StartProcess(int nCommIdx)
{
	int		nRet;
//	DWORD	dwThreadID;

	LOG_INIT();

	m_bConnected = FALSE;

	LOG_OUT("#################### start !!!");

	EndProcess();

	nRet = m_clsComm.Open(nCommIdx, CBR_19200, 8, NOPARITY, ONESTOPBIT, TRUE);
	if(nRet < 0) 
	{
		LOG_OUT("m_clsComm.Open() failure !![%d]", ::GetLastError());
		EndProcess();
		return -1;
	}

	m_hAccMutex = ::CreateMutex(NULL, FALSE, NULL);
	if(m_hAccMutex == NULL) 
	{
		LOG_OUT("CreateMutex() failure !![%d]", ::GetLastError());
		EndProcess();
		return -2;
	}

	m_bConnected = TRUE;

// 	m_hThread = ::CreateThread(NULL, 0, RunThread, this, CREATE_SUSPENDED, &dwThreadID);
// 	if(NULL == m_hThread)
// 	{
// 		LOG_OUT("CreateThread() failure !![%d]", ::GetLastError());
// 		EndProcess();
// 		return -3;
// 	}
// 
// 	::ResumeThread(m_hThread);

	return 0;
}

/**
 * @brief		EndProcess
 * @details		Start
 * @param		int nCommIdx		COM
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrtTicketHS::EndProcess(void)
{
	LOG_OUT("start !!!");
	
	m_bConnected = FALSE;

	if(NULL != m_hThread)
	{
		::WaitForSingleObject(m_hThread, 500);
		::CloseHandle(m_hThread);

		m_hThread = NULL;
	}

	m_clsComm.Close();

	if(m_hAccMutex != NULL)
	{
		CloseHandle(m_hAccMutex);
		m_hAccMutex = NULL;
	}

	return 0;
}

/**
 * @brief		RunThread
 * @details		Start
 * @param		LPVOID lParam		CBillICT instance
 * @return		항상 : 0
 */
DWORD CPrtTicketHS::RunThread(LPVOID lParam)
{
	DWORD	dwTick;
	CPrtTicketHS *pClass = (CPrtTicketHS *)lParam;

	dwTick = ::GetTickCount();
	while(pClass->m_bConnected)
	{
		Sleep(10);

		if(dwTick == 0L)
		{
			dwTick = ::GetTickCount();
		}

		if( Util_CheckExpire(dwTick) >= 500 )	
		{
			dwTick = ::GetTickCount();
		}		
	}

	return 0;
}

/**
 * @brief		BeginPrint
 * @details		티켓 프린터 시작 / 종료 명령
 * @param		BOOL bStart		시작 or 종료 Flag
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrtTicketHS::IsHangule(char *pData, int nDataLen)
{
	int i;
	BOOL bComplete;

	bComplete = TRUE;

	for(i = 0; i < nDataLen; i++)
	{
		if(pData[i] & 0x80)
		{
			bComplete = !bComplete;
		}
	}

	if( bComplete == FALSE )
	{
		return -1;
	}

	return 0;
}


/**
 * @brief		BeginPrint
 * @details		티켓 프린터 시작 / 종료 명령
 * @param		BOOL bStart		시작 or 종료 Flag
 * @return		성공 : > 0, 실패 : < 0
 */
void CPrtTicketHS::BeginPrint(BOOL bStart)
{
	LOG_OUT(" ========================================== ");

	if(bStart == TRUE)
	{
		PTCK_PRT_OPT_T pEnv = (PTCK_PRT_OPT_T) GetEnvTckPrtInfo();

		///< 01. speed
		//SendData((BYTE *)"\x1A\x73\x0E", 3);	// 200 mm/sec	// 20220706 DEL
		SendData((BYTE *)"\x1A\x73\x13", 3);	// 250 mm/sec	// 20220706 MOD

		///< 01. 농도조절
//		SendData((BYTE *)"\x1D\x28\x4B\x02\x00\x11\x05", 7);
		LOG_OUT("인쇄농도 : %d ", pEnv->nBoldVal);
		SetBold(pEnv->nBoldVal);
		//SetBold(4);
		//SendData((BYTE *)"\x1D\x28\x4B\x02\x00\x31\x05", 7);

		///< 02. PAGE MODE
		SendData((BYTE *)"\x1B\x4C", 2);

		///< 03. PAGE 방향 설정 ("T")
		if(m_bExpOnly == TRUE)
		{	/// 전주고속 승차권 
			SendData((BYTE *)"\x1B\x54\x01", 3);	
		}
		else
		{
			SendData((BYTE *)"\x1B\x54\x03", 3);	
		}
	}
	else
	{
		SendData((BYTE *)"\x1b\x0c\x13\x69", 4);
		SendData((BYTE *)"\x1b\x53", 2);
	}
}

/**
 * @brief		CCS_DF3_PrintTicket
 * @details		[시외승차권] 티켓 프린트
 * @param		char *pData		프린트 데이타 구조체
 * @return		항상 = 0
 */
int CPrtTicketHS::CCS_DF3_PrintTicket(char *pData)
{
	int					nLen, nMode, nRet, nSY;
	char				Buffer[500];
	PTCK_PRINT_FMT_T	pPrtData;
	POPER_FILE_CONFIG_T pOperConfig;
	PKIOSK_INI_ENV_T	pEnv;

	pPrtData	= (PTCK_PRINT_FMT_T) pData;
	pOperConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();
	pEnv		= (PKIOSK_INI_ENV_T) GetEnvInfo();

	nLen = nRet = nSY = 0;

//	nSatsNo = Util_Ascii2Long(pPrtData->sats_no, strlen(pPrtData->sats_no));

	TR_LOG_OUT(" start !!!\n\n");
	m_bExpOnly = FALSE;

	try
	{
		PFILE_ACCUM_N1010_T pAcc;
		POPER_FILE_CONFIG_T pOperCfg;

		pAcc = GetAccumulateData();

		pOperCfg = (POPER_FILE_CONFIG_T) GetOperConfigData();

		BeginPrint(TRUE);

		///< 1. 회수용 티켓
		{
			///< 1-1. 티켓고유번호 / user_no
			sprintf(Buffer, "%08d %s", pAcc->Curr.ccsTicket.n_bus_tck_inhr_no, GetEnvSvrUserNo(SVR_DVS_CCBUS));
			txtPrint(455, 320, PRT_MODE_NONE, Buffer);

			///< 2-1. 출발지(한글)
			nLen = strlen(pPrtData->depr_trml_ko_nm);
			if(nLen > 4)
			{
				nMode = PRT_MODE_VERTI_EXPAND;
			}
			else
			{
				nMode = PRT_MODE_BOTH_EXPAND;
			}
//			MultiPrint(412, 104, PRT_MODE_BOTH_EXPAND, PRT_MODE_VERTI_EXPAND, 14, pPrtData->depr_trml_ko_nm);
			MultiPrint(412, 64, nMode, PRT_MODE_NONE, 14, pPrtData->depr_trml_ko_nm);

			///< 2-2. 경유지
			nLen = strlen(pPrtData->thru_nm);
			if(nLen > 0)
			{
				//txtPrint(340+nGap-nMinus, 270, PRT_MODE_NONE, pPrtData->thru_nm);
			}

			///< 2-3. 목적지(한글)
			MultiPrint(412, 356, PRT_MODE_VERTI_EXPAND, PRT_MODE_VERTI_EXPAND, 14, pPrtData->arvl_trml_ko_nm);

			///< 2-4. 결제수단
			if(pPrtData->n_pym_dvs == PYM_CD_CARD)
			{
				txtPrint(408, 608, PRT_MODE_VERTI_EXPAND, "카드");
			}
			else if(pPrtData->n_pym_dvs == PYM_CD_TPAY)
			{
				txtPrint(408, 608, PRT_MODE_VERTI_EXPAND, "페이");
			}
			// 20210910 ADD
			/*
			else if(pPrtData->n_pym_dvs == PYM_CD_ETC)
			{
				txtPrint(408, 608, PRT_MODE_VERTI_EXPAND, "복합");
			}
			*/
			// 20210910 ~ADD
			else
			{
				txtPrint(408, 608, PRT_MODE_VERTI_EXPAND, "현금");
			}

			///< 3-1. 출발지(영문)
//			txtPrint(408 - 45, 104, PRT_MODE_NONE, pPrtData->depr_trml_eng_nm);
			txtPrint(408 - 45, 64, PRT_MODE_NONE, pPrtData->depr_trml_eng_nm);
			
			///< 3-2. 도착지(영문)
			txtPrint(408 - 45, 356, PRT_MODE_NONE, pPrtData->arvl_trml_eng_nm);

			///< 4-1. 요금
			if( strlen(pPrtData->cty_bus_dc_knd_str) > 0 )
			{
				/// 2020.06.28 modify code for 기피좌석 문구 현시안하게 수정
				if( memcmp(pPrtData->cty_bus_dc_knd_str, "기피", 4) == 0 )
				{
					sprintf(Buffer, "(%s%%할인)", pPrtData->dcrt_dvs_str);
				}
				else
				{
					sprintf(Buffer, "(%s%%%s할인)", pPrtData->dcrt_dvs_str, pPrtData->cty_bus_dc_knd_str);
				}
				//sprintf(Buffer, "(%s%%%s할인)", pPrtData->dcrt_dvs_str, pPrtData->cty_bus_dc_knd_str);
				/// ~2020.06.28 modify code
				txtPrint(344-5, 64, PRT_MODE_VERTI_EXPAND, Buffer);
				txtPrint(344-5, 310, PRT_MODE_BOTH_EXPAND, pPrtData->tisu_amt);
			}
			else
			{
				txtPrint(344-5, 290, PRT_MODE_BOTH_EXPAND, pPrtData->tisu_amt);
			}

			///< 4-2. 승차권종류/버스등급
			sprintf(Buffer, "%s(%s)", pPrtData->bus_tck_knd_nm, pPrtData->bus_cls_nm);
			txtPrint(344, 528-20, PRT_MODE_VERTI_EXPAND, Buffer);

			///< 5-1. 출발일
			sprintf(Buffer, "%s", pPrtData->atl_depr_dt);
			txtPrint(221+10, 91, PRT_MODE_VERTI_EXPAND, Buffer);

#if 0
			if( (pPrtData->alcn_way_dvs_cd[0] == 'N') && (pPrtData->sati_use_yn[0] == 'N') )
			{
				///< 5-2. 출발시간
				txtPrint(221+10, 304-30, PRT_MODE_VERTI_EXPAND, "선착순 승차");
			}
			else
			{
				if( pPrtData->alcn_way_dvs_cd[0] != 'N' )
				{
					///< 5-2. 출발시간
					sprintf(Buffer, "%s", pPrtData->atl_depr_time);
					txtPrint(221+10, 304, PRT_MODE_VERTI_EXPAND, Buffer);
				}

				if( pPrtData->sati_use_yn[0] != 'N' )
				{
					///< 5-3. 좌석
					sprintf(Buffer, "%s", pPrtData->sats_no);
					txtPrint(221+10, 505, PRT_MODE_VERTI_EXPAND, Buffer);
				}
			}
#else
			/****
			if( pPrtData->sati_use_yn[0] == 'N' )
			{
				/// 5-2. 출발시간
				txtPrint(221+10, 505, PRT_MODE_VERTI_EXPAND, "선착순");
			}
			else
			{
				/// 5-2. 출발시간
				sprintf(Buffer, "%s", pPrtData->atl_depr_time);
				txtPrint(221+10, 304, PRT_MODE_VERTI_EXPAND, Buffer);
				/// 5-3. 좌석
				sprintf(Buffer, "%s", pPrtData->sats_no);
				txtPrint(221+10, 505, PRT_MODE_VERTI_EXPAND, Buffer);
			}
			****/

			/// 출발시간
			TR_LOG_OUT(" >>>>>>>>>> 회수용-출발시간 >>>>>>>>>> ");
			nRet = CConfigTkMem::GetInstance()->IsPrintFirstCome(pPrtData->alcn_way_dvs_cd, pPrtData->sati_use_yn, pPrtData->depr_time_prin_yn, pPrtData->sats_no_prin_yn); // 20221207 MOD
			if( nRet >= 0 )
			{
				if(nRet == 0)
				{
					///< 4-2. 출발시간
					txtPrint(221+10, 304, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
				}
				else
				{
					///< 4-2. 출발시간
					txtPrint(221+10, 304, PRT_MODE_VERTI_EXPAND, "선착순");
				}
			}

			/// 좌석번호
			nRet = CConfigTkMem::GetInstance()->IsPrintSatsNo(pPrtData->alcn_way_dvs_cd, pPrtData->sats_no_prin_yn);
			if(nRet > 0)
			{
				///< 4-3. 좌석
				txtPrint(221+10, 505, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
			}

#endif
			///< 6-1. 결제수단
			if(pPrtData->n_pym_dvs == PYM_CD_CARD)
			{
				txtPrint(289, 632, PRT_MODE_VERTI_EXPAND, "카");
				txtPrint(224, 632, PRT_MODE_VERTI_EXPAND, "드");
			}
			else if(pPrtData->n_pym_dvs == PYM_CD_TPAY)
			{
				txtPrint(289, 632, PRT_MODE_VERTI_EXPAND, "페");
				txtPrint(224, 632, PRT_MODE_VERTI_EXPAND, "이");
			}
			// 20210910 ADD
			/*
			else if(pPrtData->n_pym_dvs == PYM_CD_ETC)
			{
				txtPrint(289, 632, PRT_MODE_VERTI_EXPAND, "복");
				txtPrint(224, 632, PRT_MODE_VERTI_EXPAND, "합");
			}
			*/
			// 20210910 ~ADD
			else
			{
				txtPrint(289, 632, PRT_MODE_VERTI_EXPAND, "현");
				txtPrint(224, 632, PRT_MODE_VERTI_EXPAND, "금");
			}

			/// 7-1. 바코드 내용 / 발권시간
			sprintf(Buffer, "%s-%s-%s-%s-%s %s", pPrtData->bar_pub_dt, pPrtData->bar_pub_shct_trml_cd, pPrtData->bar_pub_wnd_no, pPrtData->bar_pub_sno, pPrtData->bar_secu_code, pPrtData->pub_time);
			txtPrint(163 - (25*0), 56, PRT_MODE_NONE, Buffer);

			/// 8-1. 결제수단
			sprintf(Buffer, "%s",  pPrtData->pym_dvs);
			txtPrint(102 - (25*1), 56, PRT_MODE_NONE, Buffer);

			/// 8-2. 승인금액
			sprintf(Buffer, "승인금액: %s",  pPrtData->card_aprv_amt);
			txtPrint(102 - (25*2), 56, PRT_MODE_NONE, Buffer);

			/// 8-3. 출발 메시지
			txtPrint(102 - (25*3), 56, PRT_MODE_NONE, "*출발일시  확인");

			/// 9-1. 카드번호
			sprintf(Buffer, "카드No:%s",  pPrtData->card_no);
			txtPrint(102 - (25*1), 360, PRT_MODE_NONE, Buffer);

			/// 9-2. 승인번호
			sprintf(Buffer, "승인No:%s",  pPrtData->card_aprv_no);
			txtPrint(102 - (25*2), 360, PRT_MODE_NONE, Buffer);

			/// 9-3. 분실 메시지
			txtPrint(102 - (25*3), 360, PRT_MODE_NONE, "*분실시 재발급 불가");

			/// 9-4. 예매(발권채널구분코드)- 모바일,인터넷

			LOG_OUT("발권채널구분코드 = 0x%02X ...", pPrtData->tisu_chnl_dvs_cd[0] & 0xFF);

			switch(pPrtData->tisu_chnl_dvs_cd[0])
			{
			case 'W':
			case 'w':
				txtPrint(320-(48*0), 8, PRT_MODE_VERTI_EXPAND, "인");
				txtPrint(320-(48*1), 8, PRT_MODE_VERTI_EXPAND, "터");
				txtPrint(320-(48*2), 8, PRT_MODE_VERTI_EXPAND, "넷");
				break;
			case 'M':
			case 'm':
				txtPrint(320-(48*0), 8, PRT_MODE_VERTI_EXPAND, "모");
				txtPrint(320-(48*1), 8, PRT_MODE_VERTI_EXPAND, "바");
				txtPrint(320-(48*2), 8, PRT_MODE_VERTI_EXPAND, "일");
				break;
// 			default:
// 				txtPrint(320-(48*0), 15, PRT_MODE_VERTI_EXPAND, "테");
// 				txtPrint(320-(48*1), 15, PRT_MODE_VERTI_EXPAND, "스");
// 				txtPrint(320-(48*2), 15, PRT_MODE_VERTI_EXPAND, "트");
// 				break;
			}
		}

		///< 2. 승객용 티켓
		{
			///< 2-1. 티켓고유번호 / user_no
			sprintf(Buffer, "%08d %s", pAcc->Curr.ccsTicket.n_bus_tck_inhr_no, GetEnvSvrUserNo(SVR_DVS_CCBUS));
			txtPrint(455, 984, PRT_MODE_NONE, Buffer);

			/// 2-2. 출발지(한글)
			nLen = strlen(pPrtData->depr_trml_ko_nm);
			if(nLen > 4)
			{
				nMode = PRT_MODE_VERTI_EXPAND;
			}
			else
			{
				nMode = PRT_MODE_BOTH_EXPAND;
			}
//			MultiPrint(412, 720, nMode, PRT_MODE_NONE, 14, pPrtData->depr_trml_ko_nm);
			MultiPrint(412, 696, nMode, PRT_MODE_NONE, 14, pPrtData->depr_trml_ko_nm);

			/// 2-3. 목적지(한글)
			nLen = strlen(pPrtData->arvl_trml_ko_nm);
			if(nLen > 4)
			{
				nMode = PRT_MODE_VERTI_EXPAND;
			}
			else
			{
				nMode = PRT_MODE_BOTH_EXPAND;
			}
			MultiPrint(412, 972, nMode, PRT_MODE_NONE, 14, pPrtData->arvl_trml_ko_nm);

			/// 2-4. 영문 - 출발지
			{
				::ZeroMemory(Buffer, sizeof(Buffer));
				nLen = strlen(pPrtData->depr_trml_eng_nm);
				if(nLen > 20)
				{
					nLen = 20;					
				}
				::CopyMemory(Buffer, pPrtData->depr_trml_eng_nm, nLen);
				txtPrint(408 - 45, 720, PRT_MODE_NONE, Buffer);
			}

			/// 2-5. 영문 - 목적지
			{
				::ZeroMemory(Buffer, sizeof(Buffer));
				nLen = strlen(pPrtData->arvl_trml_eng_nm);
				if(nLen > 20)
				{
					nLen = 20;					
				}
				::CopyMemory(Buffer, pPrtData->arvl_trml_eng_nm, nLen);
				txtPrint(408 - 45, 952+20, PRT_MODE_NONE, Buffer);
			}

			/// 4-1. 요금
			if( strlen(pPrtData->cty_bus_dc_knd_str) > 0 )
			{	/// 시외할인정보

				/// 2020.06.28 modify code for 기피좌석 문구 현시안하게 수정
				if( memcmp(pPrtData->cty_bus_dc_knd_str, "기피", 4) == 0 )
				{
					sprintf(Buffer, "(%s%%할인)", pPrtData->dcrt_dvs_str);
				}
				else
				{
					sprintf(Buffer, "(%s%%%s할인)", pPrtData->dcrt_dvs_str, pPrtData->cty_bus_dc_knd_str);
				}
				//sprintf(Buffer, "(%s%%%s할인)", pPrtData->dcrt_dvs_str, pPrtData->cty_bus_dc_knd_str);
				/// ~2020.06.28 modify code
				txtPrint(344-5, 740, PRT_MODE_VERTI_EXPAND, Buffer);
				txtPrint(344-5, 880+20, PRT_MODE_BOTH_EXPAND, pPrtData->tisu_amt);
			}
			else
			{
				txtPrint(344-5, 880, PRT_MODE_BOTH_EXPAND, pPrtData->tisu_amt);
			}

			/// 4-2. 승차권종류
			txtPrint(344, 1095, PRT_MODE_VERTI_EXPAND, pPrtData->bus_tck_knd_nm);

			/// 5-1. 출발일
			txtPrint(221+10, 725, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_dt);

#if 0
			if( (pPrtData->alcn_way_dvs_cd[0] == 'N') && (pPrtData->sati_use_yn[0] == 'N') )
			{
				/// 5-2. 출발시간
				txtPrint(221+10, 910+20, PRT_MODE_VERTI_EXPAND, "선착순 승차");
			}
			else
			{
				if( pPrtData->alcn_way_dvs_cd[0] != 'N' )
				{
					/// 5-2. 출발시간
					txtPrint(221+10, 910+20, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
				}

				if( pPrtData->sati_use_yn[0] != 'N' )
				{
					/// 5-3. 좌석
					txtPrint(221+10, 1020+20, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
				}
			}
#else
			/****
			if( pPrtData->sati_use_yn[0] == 'N' )
			{
				/// 5-2. 출발시간
				//txtPrint(221+10, 910+20, PRT_MODE_VERTI_EXPAND, "선착순");
			}
			else
			{
				/// 5-2. 출발시간
//				txtPrint(221+10, 910+20, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
				txtPrint(221+10, 890, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
				/// 5-3. 좌석
				txtPrint(221+10, 1010, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
			}
			****/
			/// 출발시간
			TR_LOG_OUT(" >>>>>>>>>> 승객용-출발시간 >>>>>>>>>> ");
			nRet = CConfigTkMem::GetInstance()->IsPrintFirstCome(pPrtData->alcn_way_dvs_cd, pPrtData->sati_use_yn, pPrtData->depr_time_prin_yn, pPrtData->sats_no_prin_yn); // 20221207 MOD
			if( nRet >= 0 )
			{
				if(nRet == 0)
				{
					///< 4-2. 출발시간
					txtPrint(221+10, 890, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
				}
				else
				{
					///< 4-2. 출발시간
					txtPrint(221+10, 890, PRT_MODE_VERTI_EXPAND, "선착순");
				}
			}

			/// 좌석번호
			nRet = CConfigTkMem::GetInstance()->IsPrintSatsNo(pPrtData->alcn_way_dvs_cd, pPrtData->sats_no_prin_yn);
			if(nRet > 0)
			{
				///< 4-3. 좌석
				txtPrint(221+10, 1010, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
			}

#endif
			/// 5-4. 승차홈
//			txtPrint(221+10, 1088+10+30, PRT_MODE_VERTI_EXPAND, pPrtData->rdhm_val);
			txtPrint(221+10, 1088+10, PRT_MODE_VERTI_EXPAND, pPrtData->rdhm_val);

			/// 6-1. 바코드 내용 / 발권시간
			sprintf(Buffer, "%s-%s-%s-%s-%s %s", pPrtData->bar_pub_dt, pPrtData->bar_pub_shct_trml_cd, pPrtData->bar_pub_wnd_no, pPrtData->bar_pub_sno, pPrtData->bar_secu_code, pPrtData->pub_time);
			txtPrint(163 - (25*0), 704-10, PRT_MODE_NONE, Buffer);

			/// 6-2. 운수회사
			sprintf(Buffer, "운수회사: %s", pPrtData->bus_cacm_nm);
			txtPrint(163 - (25*1), 704-10, PRT_MODE_NONE, Buffer);

			/// 6-3. 출발 메시지
			txtPrint(163 - (25*2), 704-10, PRT_MODE_NONE, "*출발일시  확인");
			
			/// 6-4. 결제수단
			sprintf(Buffer, "%s",  pPrtData->pym_dvs);
			txtPrint(163 - (25*3), 704-10, PRT_MODE_NONE, Buffer);
			
			/// 6-5. 승인금액
			sprintf(Buffer, "승인금액:%s",  pPrtData->card_aprv_amt);
			txtPrint(163 - (25*4), 704-10, PRT_MODE_NONE, Buffer);
			
			/// 6-6. 발권회사 정보
			sprintf(Buffer, "%s ARS:%s",  pOperCfg->ccTrmlInfo_t.sz_prn_trml_sangho, pOperCfg->ccTrmlInfo_t.sz_prn_trml_ars_no);
			txtPrint(163 - (25*5), 704-10, PRT_MODE_NONE, Buffer);

			/// 7-1. 분실 메시지
			txtPrint(163 - (25*2), 936-10, PRT_MODE_NONE, "*분실시재발급불가");

			/// 7-2. 카드번호
			sprintf(Buffer, "카드No:%s",  pPrtData->card_no);
			txtPrint(163 - (25*3), 936-50-20, PRT_MODE_NONE, Buffer);

			/// 7-3. 승인번호
			sprintf(Buffer, "승인No:%s",  pPrtData->card_aprv_no);
			txtPrint(163 - (25*4), 936-10, PRT_MODE_NONE, Buffer);
		}

		///< 3. 바코드 프린트
		{
			///< 3-1. 위치
			Print("\x1b\x57%04d%04d", 163 - (25*1), 56);
			///< 3-2. barcode height ('h')						*
			SendData((BYTE *)"\x1D\x68\x3C", 3);
			///< 3-3. barcode width ('w')
			SendData((BYTE *)"\x1D\x77\x04", 3);	// 4배확대
			///< 3-4. barcode type ('k')
			SendData((BYTE *)"\x1D\x6B\x07", 3);
			///< 3-5. barcode data
			sprintf(Buffer, "i%s%s%s%s%s", 
					pPrtData->bar_pub_dt, 
					pPrtData->bar_pub_shct_trml_cd, 
					pPrtData->bar_pub_wnd_no, 
					pPrtData->bar_pub_sno, 
					pPrtData->bar_secu_code);
			SendData((BYTE *)Buffer, strlen(Buffer) + 1);
		}

		BeginPrint(FALSE);
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	TR_LOG_OUT(" end !!!\n\n");

	return 0;
}

/**
 * @brief		CCS_DFAll_PrintTicket
 * @details		[시외-통합승차권] 티켓 프린트
 * @param		char *pData		프린트 데이타 구조체
 * @return		항상 = 0
 */
int CPrtTicketHS::CCS_DFAll_PrintTicket(char *pData)
{
	int					nLen, nGap, nSatsNo, nMinus, nRet;
	char				Buffer[500];
	PTCK_PRINT_FMT_T	pPrtData;
	POPER_FILE_CONFIG_T pOperConfig;

	pPrtData = (PTCK_PRINT_FMT_T) pData;

	pOperConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	nLen = nRet = 0;

	nSatsNo = Util_Ascii2Long(pPrtData->sats_no, strlen(pPrtData->sats_no));

	TR_LOG_OUT(" start !!!\n\n");
	m_bExpOnly = FALSE;

	try
	{
		BeginPrint(TRUE);

		nGap = 30;
		nMinus = 20;
		///< 1. 회수용 티켓
		{
			///< 1-1. 출발지
			nLen = strlen(pPrtData->depr_trml_ko_nm);
			txtPrint(390+nGap-nMinus, 40, PRT_MODE_VERTI_EXPAND, pPrtData->depr_trml_ko_nm);

			///< 1-2. 경유지
			nLen = strlen(pPrtData->thru_nm);
			if(nLen > 0)
			{
				txtPrint(340+nGap-nMinus, 270, PRT_MODE_NONE, pPrtData->thru_nm);
			}

			///< 1-2. 목적지
			nLen = strlen(pPrtData->arvl_trml_ko_nm);
			txtPrint(390+nGap-nMinus, 420, PRT_MODE_VERTI_EXPAND, pPrtData->arvl_trml_ko_nm);

			///< 1-3. 버스등급
			{
				int i, nRow, nOffset;

				i = nRow = nOffset = 0;

				nLen = strlen(pPrtData->bus_cls_nm);
				nRow = nLen / 4;
				if(nRow > 0)
				{
					nOffset = 0;
					for(i = 0; i < nRow; i++)
					{
						::ZeroMemory(Buffer, sizeof(Buffer));
						::CopyMemory(Buffer, &pPrtData->bus_cls_nm[nOffset], 4);
						nOffset += 4;
						txtPrint(390+nGap-(i*20)-nMinus, 700, PRT_MODE_NONE, Buffer);
					}
				}
				else
				{
					txtPrint(390+nGap-nMinus, 700, PRT_MODE_NONE, pPrtData->bus_cls_nm);
				}
			}
			//txtPrint(390+nGap-nMinus, 700, PRT_MODE_NONE, pPrtData->bus_cls_nm);

			///< 2-1. 영문 출발지
			txtPrint(340+nGap-nMinus, 40, PRT_MODE_SMALL_ON, pPrtData->depr_trml_eng_nm);
			///< 2-2. 영문 도착지
			txtPrint(340+nGap-nMinus, 420, PRT_MODE_SMALL_ON, pPrtData->arvl_trml_eng_nm);

			///< 3-1. 요금
			txtPrint(320+nGap-nMinus, 190, PRT_MODE_VERTI_EXPAND, pPrtData->tisu_amt);

			///< 3-2. 현금/신용카드, 승차권종류 
//			sprintf(Buffer, " 원    %s   (무인)  %s", pPrtData->pym_dvs, pPrtData->bus_tck_knd_nm);
			sprintf(Buffer, " 원    %s         %s", pPrtData->pym_dvs, pPrtData->bus_tck_knd_nm);
			txtPrint(300+nGap-nMinus, 270, PRT_MODE_NONE, Buffer);

			///< 4-1. 출발일
			txtPrint(210+nGap-nMinus,  30, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_dt);

			LOG_OUT("배차방식   = %c ", pPrtData->alcn_way_dvs_cd[0]);
			LOG_OUT("좌석제유무 = %c ", pPrtData->sati_use_yn[0]);

// 			if( (pPrtData->alcn_way_dvs_cd[0] == 'N') && ((nSatsNo == 0) || (nSatsNo == 99)) )
// 			{	/// 비배차, 비좌석제는 출발시간/ 좌석번호를 프린트 안함.
// 				;
// 			}
// 			else
// 			{
// 				///< 4-3. 좌석
// 				if( (nSatsNo != 0) && (nSatsNo != 99) )
// 				{	// 0:비배차, 99:비좌석
// 					/// 출발시간
// 					txtPrint(210+nGap-nMinus, 220, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
// 					/// 좌석번호
// 					txtPrint(210+nGap-nMinus, 390, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
// 				}
// 			}

			if(GetEnvOperCorp() == KUMHO_OPER_CORP)
			{	/// 금호터미널 이면..
 				///< 4-2. 출발시간
 				if(pPrtData->sati_use_yn[0] == 'N')
 				{	/// 비좌석제이면, 출발시간, 좌석번호 출력안함.
 					txtPrint(220+nGap-nMinus, 220, PRT_MODE_NONE, "<<선착순 탑승>>");
 					txtPrint(190+nGap-nMinus, 220, PRT_MODE_SMALL_ON, "미탑승분은 다음차 이용");
 				}
 				else
 				{
 					/// 출발시간
 					txtPrint(210+nGap-nMinus, 220, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
 					/// 좌석번호
 					txtPrint(210+nGap-nMinus, 390-20, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
 				}
			}
			else
			{
				/// 2020.03.26 출발시간, 좌석번호 수정
				/// 출발시간
				TR_LOG_OUT(" >>>>>>>>>> DFALL 회수용-출발시간 >>>>>>>>>> ");
				nRet = CConfigTkMem::GetInstance()->IsPrintFirstCome(pPrtData->alcn_way_dvs_cd, pPrtData->sati_use_yn, pPrtData->depr_time_prin_yn, pPrtData->sats_no_prin_yn); // 20221207 MOD
				if( nRet >= 0 )
				{
					if(nRet == 0)
					{
						///< 4-2. 출발시간
						txtPrint(210+nGap-nMinus, 220, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
					}
					else
					{
						///< 4-2. 출발시간
						txtPrint(210+nGap-nMinus, 220, PRT_MODE_VERTI_EXPAND, "선착순");
					}
				}

				/// 좌석번호
				nRet = CConfigTkMem::GetInstance()->IsPrintSatsNo(pPrtData->alcn_way_dvs_cd, pPrtData->sats_no_prin_yn);
				if(nRet > 0)
				{
					///< 4-3. 좌석
					txtPrint(210+nGap-nMinus, 390-20, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
				}
			}

			///< 4-4. 승차홈
			txtPrint(210+nGap-nMinus, 490-20, PRT_MODE_VERTI_EXPAND, pPrtData->rdhm_val);
			///< 4-5. 버스회사
			txtPrint(210+nGap-nMinus, 590, PRT_MODE_VERTI_EXPAND, pPrtData->bus_cacm_nm);

			///< 5-1. 승인번호, 승인금액
			sprintf(Buffer, "승인번호: %s  승인금액: %s  %s", pPrtData->card_aprv_no, pPrtData->card_aprv_amt, pPrtData->card_no);
			txtPrint(150+nGap-nMinus, 30, PRT_MODE_NONE, Buffer);

			///< 6-1. (바코드) 발행시각
			txtPrint(65+nGap-nMinus, 620, PRT_MODE_NONE, pPrtData->pub_time);
			///< 6-2. (바코드) 출발지
			txtPrint(35+nGap-nMinus, 30, PRT_MODE_NONE, pPrtData->depr_trml_ko_nm);
			///< 6-3. (바코드) 바코드정보
			sprintf(Buffer, "%s-%s-%s-%s-%s", pPrtData->bar_pub_dt, pPrtData->bar_pub_shct_trml_cd, pPrtData->bar_pub_wnd_no, pPrtData->bar_pub_sno, pPrtData->bar_secu_code);
			txtPrint(35+nGap-nMinus, 310, PRT_MODE_NONE, Buffer);
		}

		nGap = 30;

		///< 2. 승객용 티켓
		{
			///< 1-1. 출발지
			{
				MultiPrint(390+nGap-nMinus,  780, PRT_MODE_VERTI_EXPAND, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pPrtData->depr_trml_ko_nm);
			}
			///< 1-2. 도착지
			{
				MultiPrint(390+nGap-nMinus, 1000, PRT_MODE_VERTI_EXPAND, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pPrtData->arvl_trml_ko_nm);
			}

			// org code
			//txtPrint(390+nGap, 1030, PRT_MODE_VERTI_EXPAND, pPrtData->arvl_trml_ko_nm);

			///< 2-1. 영문 출발지
			{
				::ZeroMemory(Buffer, sizeof(Buffer));
				nLen = strlen(pPrtData->depr_trml_eng_nm);
				if(nLen > 20)
				{
					nLen = 20;					
				}
				::CopyMemory(Buffer, pPrtData->depr_trml_eng_nm, nLen);
				txtPrint(340+nGap-nMinus,  780, PRT_MODE_SMALL_ON, Buffer);
			}

			{
				::ZeroMemory(Buffer, sizeof(Buffer));
				nLen = strlen(pPrtData->arvl_trml_eng_nm);
				if(nLen > 20)
				{
					nLen = 20;					
				}
				::CopyMemory(Buffer, pPrtData->arvl_trml_eng_nm, nLen);
				txtPrint(340+nGap-nMinus, 1000, PRT_MODE_SMALL_ON, Buffer);
			}

			///< 3-1. 요금				  PRT_MODE_NONE
			sprintf(Buffer, "%s원", pPrtData->tisu_amt);
			txtPrint(310+nGap-nMinus,  840, PRT_MODE_NONE, Buffer);
			///< 3-2. 현금/신용카드, 승차권종류
			sprintf(Buffer, "    %s  %s", pPrtData->pym_dvs, pPrtData->bus_tck_knd_nm);
			txtPrint(300+nGap-nMinus,  900, PRT_MODE_SMALL_ON, Buffer);

			///< 4-1. 출발일
			txtPrint(210+nGap-nMinus,  800 + (160 * 0)+20, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_dt);
			
			/****
			///< 4-2. 출발시간
			if(pPrtData->sati_use_yn[0] == 'N')
			{	/// 비좌석제이면, 출발시간, 좌석번호 출력안함.
				;
			}
			else
			{
				///< 4-2. 출발시각
				txtPrint(210+nGap-nMinus,  800 + (160 * 1)+20, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
				///< 4-3. 좌석
				txtPrint(210+nGap-nMinus,  800 + (160 * 2), PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
			}	
			****/

			/// 출발시간
			TR_LOG_OUT(" >>>>>>>>>> DFALL 승객용-출발시간 >>>>>>>>>> ");
			nRet = CConfigTkMem::GetInstance()->IsPrintFirstCome(pPrtData->alcn_way_dvs_cd, pPrtData->sati_use_yn, pPrtData->depr_time_prin_yn, pPrtData->sats_no_prin_yn); // 20221207 MOD
			if( nRet >= 0 )
			{
				if(nRet == 0)
				{
					///< 4-2. 출발시간
					txtPrint(210+nGap-nMinus,  800 + (160 * 1)+20, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
				}
				else
				{
					///< 4-2. 출발시간
					txtPrint(210+nGap-nMinus,  800 + (160 * 1)+20, PRT_MODE_VERTI_EXPAND, "선착순");
				}
			}

			/// 좌석번호
			nRet = CConfigTkMem::GetInstance()->IsPrintSatsNo(pPrtData->alcn_way_dvs_cd, pPrtData->sats_no_prin_yn);
			if(nRet > 0)
			{
				///< 4-3. 좌석
				txtPrint(210+nGap-nMinus,  800 + (160 * 2), PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
			}

			///< 5-1. 바코드 정보
			sprintf(Buffer, "%s-%s-%s-%s-%s", pPrtData->bar_pub_dt, pPrtData->bar_pub_shct_trml_cd, pPrtData->bar_pub_wnd_no, pPrtData->bar_pub_sno, pPrtData->bar_secu_code);
			txtPrint(150 - (20 * 0)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
			///< 5-2. 승인번호
			sprintf(Buffer, "승인번호: %s %s", pPrtData->card_aprv_no, pPrtData->pub_time);
			txtPrint(150 - (20 * 1)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
			///< 5-3. 승인금액
			if(pPrtData->n_pym_dvs == PYM_CD_CASH)
			{
				sprintf(Buffer, "승인금액: %s   %s", pPrtData->card_aprv_amt, pPrtData->card_no);
			}
			else
			{
				sprintf(Buffer, "승인금액: %s 구매자:%s", pPrtData->card_aprv_amt, pPrtData->card_no);
			}
			txtPrint(150 - (20 * 2)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
			///< 5-4. 가맹점번호
			// org code
			//txtPrint(150 - (20 * 3)+nGap,  800, PRT_MODE_SMALL_ON, "119-81-568654");
			sprintf(Buffer, "가맹점사업자번호:%s", pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);
			txtPrint(150 - (20 * 3)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);

			sprintf(Buffer, "*유효기간 당일 지정차에 한함");
			txtPrint(150 - (20 * 4)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);

//			sprintf(Buffer, "현금영수증 문의 ☏ 126-2");
			sprintf(Buffer, "현금영수증 문의 : 126-2");
			txtPrint(150 - (20 * 5)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);

			///< 5-5. 버스회사 명
			sprintf(Buffer, "버스회사:%s", pPrtData->bus_cacm_nm);
			txtPrint(150 - (20 * 6)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
		}

		///< 3. 바코드 프린트
		{
			///< 3-1. 위치
			Print("\x1b\x57%04d%04d", 80+nGap, 40);
			///< 3-2. barcode height ('h')						*
			SendData((BYTE *)"\x1D\x68\x3C", 3);
			///< 3-3. barcode width ('w')
			SendData((BYTE *)"\x1D\x77\x04", 3);	// 4배확대
			///< 3-4. barcode type ('k')
			SendData((BYTE *)"\x1D\x6B\x07", 3);
			///< 3-5. barcode data
			sprintf(Buffer, "i%s%s%s%s%s", 
					pPrtData->bar_pub_dt, 
					pPrtData->bar_pub_shct_trml_cd, 
					pPrtData->bar_pub_wnd_no, 
					pPrtData->bar_pub_sno, 
					pPrtData->bar_secu_code);
			SendData((BYTE *)Buffer, strlen(Buffer) + 1);
		}

		BeginPrint(FALSE);
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	TR_LOG_OUT(" end !!!\n\n");

	return 0;
}

/**
 * @brief		CCS_DFAll_IIAC_PrintTicket
 * @details		[시외-통합승차권] 인천공항 티켓 프린트
 * @param		char *pData		프린트 데이타 구조체
 * @return		항상 = 0
 */
int CPrtTicketHS::CCS_DFAll_IIAC_PrintTicket(char *pData)
{
	int					nLen, nSatsNo, nRet, nSx, nSY;
	char				Buffer[500];
	PTCK_PRINT_FMT_T	pPrtData;
	POPER_FILE_CONFIG_T pOperConfig;
	PKIOSK_INI_ENV_T	pEnv;

	pPrtData	= (PTCK_PRINT_FMT_T) pData;
	pOperConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();
	pEnv		= (PKIOSK_INI_ENV_T) GetEnvInfo();

	nSx = nSY = nLen = nRet = 0;

	nSatsNo = Util_Ascii2Long(pPrtData->sats_no, strlen(pPrtData->sats_no));

	TR_LOG_OUT(" start !!!\n\n");
	m_bExpOnly = FALSE;

	try
	{
		BeginPrint(TRUE);

		///< 1. 회수용 티켓
		{
			///< 1-1. 출발지
			nLen = strlen(pPrtData->depr_trml_ko_nm);
			txtPrint(400, 40, PRT_MODE_VERTI_EXPAND, pPrtData->depr_trml_ko_nm);

			///< 1-2. 목적지 - 노선명
			if( strlen(pPrtData->arvl_trml_rot_num) > 0 )
			{
				txtPrint(400, 400, PRT_MODE_VERTI_EXPAND, pPrtData->arvl_trml_rot_num);

				///< 1-2. 목적지
				nLen = strlen(pPrtData->arvl_trml_ko_nm);
				MultiPrint(400, 480, PRT_MODE_VERTI_EXPAND, PRT_MODE_NONE, MAX_HANGUL_CHAR, pPrtData->arvl_trml_ko_nm);
			}
			else
			{
				///< 1-2. 목적지
				nLen = strlen(pPrtData->arvl_trml_ko_nm);
				MultiPrint(400, 420, PRT_MODE_VERTI_EXPAND, PRT_MODE_NONE, MAX_HANGUL_CHAR, pPrtData->arvl_trml_ko_nm);
			}

			///< 1-3. 발행일련번호/user_no/사용자명
			//txtPrint(460, 460, PRT_MODE_NONE, "손다림");
// 			if( strlen(pPrtData->user_nm) <= 10 )
// 			{
// 				txtPrint(460, 460, PRT_MODE_NONE, pPrtData->user_nm);
// 			}
// 			else
// 			{
// 				txtPrint(460, 300, PRT_MODE_NONE, pPrtData->user_nm);
// 			}
			sprintf(Buffer, "No. 0000%s %s(%s)", pPrtData->bar_pub_sno, GetEnvSvrUserNo(SVR_DVS_CCBUS), pPrtData->user_nm);
			txtPrint(460, 280, PRT_MODE_NONE, Buffer);
				
			///< 1-4. 버스등급
			/// 버스티켓종류명/버스등급명
			//sprintf(Buffer, "%s(%s)", pPrtData->bus_tck_knd_nm, pPrtData->bus_cls_nm);
			//txtPrint(460, 590, PRT_MODE_NONE, Buffer);

			///< 2-1. 영문 출발지
			txtPrint(350, 40, PRT_MODE_SMALL_ON, pPrtData->depr_trml_eng_nm);
			///< 2-2. 영문 도착지
			txtPrint(350, 420, PRT_MODE_SMALL_ON, pPrtData->arvl_trml_eng_nm);

			///< 3-1. 요금
//			txtPrint(330, 270, PRT_MODE_VERTI_EXPAND, pPrtData->tisu_amt);
			txtPrint(333, 270, PRT_MODE_VERTI_EXPAND, pPrtData->tisu_amt);

			///< 3-2. 요금단위
//			txtPrint(310, 350, PRT_MODE_NONE, " 원");
			txtPrint(320, 350, PRT_MODE_NONE, " 원");

			///< 3-3. 티켓종류(버스등급명)
			sprintf(Buffer, "%s(%s)", pPrtData->bus_tck_knd_nm, pPrtData->bus_cls_nm);
			txtPrint(320, 450, PRT_MODE_NONE, Buffer);

			nSx = 225 - 5;
			nSY = 30;
			
			///< 4-1. 출발일
			txtPrint(nSx,  30+20, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_dt);
			
#if 1 
			/// 인천공항T2 이면...
			if( memcmp(GetTrmlCode(SVR_DVS_CCBUS), "2238202", 7) == 0 )
//			if( memcmp(GetTrmlCode(SVR_DVS_CCBUS), "2238201", 7) == 0 )
			{
				// 6707A, 6705 노선만 선착순 탑승 나오게 하기..
				if( (memcmp(pPrtData->arvl_trml_rot_num, "6707A", 5) == 0) || (memcmp(pPrtData->arvl_trml_rot_num, "6705", 4) == 0) )
				{
					txtPrint(230, 220, PRT_MODE_NONE, "<<선착순 탑승>>");
					txtPrint(200, 220, PRT_MODE_SMALL_ON, "미탑승분은 다음차 이용");
				}
				else
				{
					/***
					if(pPrtData->alcn_way_dvs_cd[0] != 'N')
					{	/// 배차방식이고..
						///< 4-2. 출발시간
						txtPrint(nSx, 220+30, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
					}

					if( pPrtData->sati_use_yn[0] != 'N' )
					{	/// 좌석제이고.
						///< 4-3. 좌석
						txtPrint(nSx, 390, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
					}
					**/
					/// 출발시간
					TR_LOG_OUT(" >>>>>>>>>> DFAll_IIAC 회수용-출발시간 1 >>>>>>>>>> ");
					nRet = CConfigTkMem::GetInstance()->IsPrintFirstCome(pPrtData->alcn_way_dvs_cd, pPrtData->sati_use_yn, pPrtData->depr_time_prin_yn, pPrtData->sats_no_prin_yn); // 20221207 MOD
					if( nRet >= 0 )
					{
						if(nRet == 0)
						{
							///< 4-2. 출발시간
							txtPrint(nSx, 220+30, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
						}
						else
						{
							///< 4-2. 출발시간
							txtPrint(nSx, 220+30, PRT_MODE_VERTI_EXPAND, "선착순");
						}
					}

					/// 좌석번호
					nRet = CConfigTkMem::GetInstance()->IsPrintSatsNo(pPrtData->alcn_way_dvs_cd, pPrtData->sats_no_prin_yn);
					if(nRet > 0)
					{
						///< 4-3. 좌석
						txtPrint(nSx, 390, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
					}
				}
			}
			else
			{
				/*****
				if( (pPrtData->alcn_way_dvs_cd[0] == 'N') && (pPrtData->sati_use_yn[0] == 'N') )
				{	/// 비배차이고, 비좌석제이면
					txtPrint(230, 220, PRT_MODE_NONE, "<<선착순 탑승>>");
					txtPrint(200, 220, PRT_MODE_SMALL_ON, "미탑승분은 다음차 이용");
				}
				else
				{
					if(pPrtData->alcn_way_dvs_cd[0] != 'N')
					{	/// 배차방식이고..
						///< 4-2. 출발시간
						txtPrint(nSx, 220+30, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
					}

					if( pPrtData->sati_use_yn[0] != 'N' )
					{	/// 좌석제이고.
						///< 4-3. 좌석
						txtPrint(nSx, 390, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
					}
				}
				****/
				/// 출발시간
				TR_LOG_OUT(" >>>>>>>>>> DFAll_IIAC 회수용-출발시간 2 >>>>>>>>>> ");
				nRet = CConfigTkMem::GetInstance()->IsPrintFirstCome(pPrtData->alcn_way_dvs_cd, pPrtData->sati_use_yn, pPrtData->depr_time_prin_yn, pPrtData->sats_no_prin_yn); // 20221207 MOD
				if( nRet >= 0 )
				{
					if(nRet == 0)
					{
						///< 4-2. 출발시간
						txtPrint(nSx, 220+30, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
					}
					else
					{
						///< 4-2. 출발시간
						txtPrint(nSx, 220+30, PRT_MODE_VERTI_EXPAND, "선착순");
					}
				}

				/// 좌석번호
				nRet = CConfigTkMem::GetInstance()->IsPrintSatsNo(pPrtData->alcn_way_dvs_cd, pPrtData->sats_no_prin_yn);
				if(nRet > 0)
				{
					///< 4-3. 좌석
					txtPrint(nSx, 390, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
				}
			}
#else
			/// 출발시간
			nRet = CConfigTkMem::GetInstance()->IsPrintFirstCome(pPrtData->alcn_way_dvs_cd, pPrtData->sati_use_yn, pPrtData->depr_time_prin_yn);
			if( nRet >= 0 )
			{
				if(nRet == 0)
				{
					///< 4-2. 출발시간
					txtPrint(nSx, 220+30, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
				}
				else
				{
					///< 4-2. 출발시간
					txtPrint(nSx, 220+30, PRT_MODE_VERTI_EXPAND, "선착순");
				}
			}

			/// 좌석번호
			nRet = CConfigTkMem::GetInstance()->IsPrintSatsNo(pPrtData->alcn_way_dvs_cd, pPrtData->sats_no_prin_yn);
			if(nRet > 0)
			{
				///< 4-3. 좌석
				txtPrint(nSx, 390, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
			}
#endif
			
			///< 4-4. 승차홈
			nLen = strlen(pPrtData->rdhm_val);
			if(nLen > 2)
			{
				txtPrint(nSx, 470, PRT_MODE_VERTI_EXPAND, pPrtData->rdhm_val);
			}
			else
			{
				txtPrint(nSx, 490, PRT_MODE_VERTI_EXPAND, pPrtData->rdhm_val);
			}

			/// 2020.06.28 add code
			/// 비배차 : 시간, 운수회사 출력여부
			if(pPrtData->alcn_way_dvs_cd[0] == 'D')
			{	/// 배차
				///< 4-5. 버스회사
				txtPrint(nSx, 590, PRT_MODE_VERTI_EXPAND, pPrtData->bus_cacm_nm);
			}
			else
			{	/// 비배차
				if(pEnv->tTckOpt.nalcn_time_cacm_yn[0] == 'Y')
				{	/// 비배차 : 시간, 운수사 출력유무..
					///< 4-5. 버스회사
					txtPrint(nSx, 590, PRT_MODE_VERTI_EXPAND, pPrtData->bus_cacm_nm);
				}
			}
			/// ~2020.06.28 add code

			///< 5-1. (바코드) 바코드정보 / 발행시간
			sprintf(Buffer, "%s-%s-%s-%s-%s %s", pPrtData->bar_pub_dt, pPrtData->bar_pub_shct_trml_cd, pPrtData->bar_pub_wnd_no, pPrtData->bar_pub_sno, pPrtData->bar_secu_code, pPrtData->pub_time);
			txtPrint(160 + 10 - (25*0), 30, PRT_MODE_NONE, Buffer);

			///< 5-2. 신용카드
			//txtPrint(170 - (25*1),	30,  PRT_MODE_NONE, "신용카드");
			txtPrint(160 + 10  - (25*1),	30,  PRT_MODE_NONE, pPrtData->pym_dvs);
			///< 5-3. 카드번호
			sprintf(Buffer, "카드No:%s", pPrtData->card_no);
			txtPrint(160 + 10  - (25*1),	280, PRT_MODE_NONE, Buffer);
			///< 5-4. 승인금액
			sprintf(Buffer, "승인금액:%s", pPrtData->card_aprv_amt);
			txtPrint(160 + 10  - (25*2), 30,  PRT_MODE_NONE, Buffer);
			
			if(pPrtData->n_pym_dvs == PYM_CD_RF)
			{
				///< 5-5. RF잔액
				sprintf(Buffer, "잔액:%s", pPrtData->rf_balance_amt);
				txtPrint(160 + 10  - (25*2), 280, PRT_MODE_NONE, Buffer);
			}
			else
			{
				///< 5-5. 승인No
				sprintf(Buffer, "승인No:%s", pPrtData->card_aprv_no);
				txtPrint(160 + 10  - (25*2), 280, PRT_MODE_NONE, Buffer);
			}
			
			///< 5-6. 상주직원
			{
				int nOffset = 0;

				::ZeroMemory(Buffer, sizeof(Buffer));

				/// 상주직원 이름
				if( strlen(pPrtData->rsd_nm) > 0 )
				{
					nOffset += sprintf(&Buffer[nOffset], "%s", pPrtData->rsd_nm);
				}

				/// 상주직원 소속명
				if( strlen(pPrtData->rsd_cmpy_nm) > 0 )
				{
					nOffset += sprintf(&Buffer[nOffset], "(%s)", pPrtData->rsd_cmpy_nm);
				}

				/// 상주직원 연락처
				if( strlen(pPrtData->rsd_tel_nm) > 0 )
				{
					nOffset += sprintf(&Buffer[nOffset], "%s", pPrtData->rsd_tel_nm);
				}
				txtPrint(30, 30, PRT_MODE_NONE, Buffer);
			}
		}

		///< 2. 승객용 티켓
		{
			sprintf(Buffer, "No.0000%s %s(%s)", pPrtData->bar_pub_sno, GetEnvSvrUserNo(SVR_DVS_CCBUS), pPrtData->user_nm);
			if( strlen(Buffer) > 26 )
			{
				Buffer[26] = 0;
			}
			txtPrint(460, 940+10, PRT_MODE_SMALL_ON, Buffer);

			///< 1-1. 출발지
			{
				MultiPrint(400, 800, PRT_MODE_VERTI_EXPAND, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pPrtData->depr_trml_ko_nm);
			}

			//txtPrint(400, 400, PRT_MODE_VERTI_EXPAND, pPrtData->arvl_trml_rot_num);

			///< 1-2. 노선명 & 도착지
			{
				if( strlen(pPrtData->arvl_trml_rot_num) <= 0 )
				{
					MultiPrint(400, 980, PRT_MODE_VERTI_EXPAND, PRT_MODE_NONE, MAX_HANGUL_CHAR - 2, pPrtData->arvl_trml_ko_nm);
				}
				else
				{
					sprintf(Buffer, "%s %s", pPrtData->arvl_trml_rot_num, pPrtData->arvl_trml_ko_nm);
					if( strlen(Buffer) < 10 )
					{
						MultiPrint(400, 980+20, PRT_MODE_VERTI_EXPAND, PRT_MODE_NONE, MAX_HANGUL_CHAR - 2, Buffer);
					}
					else
					{
						MultiPrint(400, 940+20, PRT_MODE_VERTI_EXPAND, PRT_MODE_NONE, MAX_HANGUL_CHAR - 2, Buffer);
					}
				}
			}

			// org code
			//txtPrint(390+nGap, 1030, PRT_MODE_VERTI_EXPAND, pPrtData->arvl_trml_ko_nm);

			///< 2-1. 영문 출발지
			{
				::ZeroMemory(Buffer, sizeof(Buffer));
				nLen = strlen(pPrtData->depr_trml_eng_nm);
				if(nLen > 20)
				{
					nLen = 20;					
				}
				::CopyMemory(Buffer, pPrtData->depr_trml_eng_nm, nLen);
				txtPrint(350,  800, PRT_MODE_SMALL_ON, Buffer);
			}

			///< 2-2. 영문 도착지
			{
				::ZeroMemory(Buffer, sizeof(Buffer));
				nLen = strlen(pPrtData->arvl_trml_eng_nm);
				if(nLen > 20)
				{
					nLen = 20;					
				}
				::CopyMemory(Buffer, pPrtData->arvl_trml_eng_nm, nLen);
				txtPrint(350, 980+20, PRT_MODE_SMALL_ON, Buffer);
			}

			///< 3-1. 요금
			txtPrint(330,  850+20, PRT_MODE_VERTI_EXPAND, pPrtData->tisu_amt);

			///< 3-2. 현금/신용카드, 승차권종류
			txtPrint(320,  930+20, PRT_MODE_NONE, " 원");
			
			///< 3-3. 티켓종류(버스등급명)
			sprintf(Buffer, "%s(%s)", pPrtData->bus_tck_knd_nm, pPrtData->bus_cls_nm);
			txtPrint(320, 1000+20, PRT_MODE_NONE, Buffer);

			///< 4-1. 출발일
			txtPrint(nSx,  800 + (160 * 0) + 30, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_dt);

			/// 인천공항T2 이면...
			if( memcmp(GetTrmlCode(SVR_DVS_CCBUS), "2238202", 7) == 0 )
			{
				// 6707A, 6705 노선만 선탁순 탑승 나오게 하기..
				if( (memcmp(pPrtData->arvl_trml_rot_num, "6707A", 5) == 0) || (memcmp(pPrtData->arvl_trml_rot_num, "6705", 4) == 0) )
				{
					;
				}
				else
				{
					/**************
					if( pPrtData->alcn_way_dvs_cd[0] != 'N' )
					{	/// 배차방식이고...
						///< 4-2. 출발시각
						txtPrint(nSx,  800 + (160 * 1) + 40, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
					}

					if( pPrtData->sati_use_yn[0] != 'N' )
					{	// 좌석제이면...
						///< 4-3. 좌석
						//if( (nSatsNo != 0) && (nSatsNo != 99) )
						{	// 0:비배차, 99:비좌석
							txtPrint(nSx,  800 + (160 * 2)+20, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
						}
					}
					*****************/
					/// 출발시간
					TR_LOG_OUT(" >>>>>>>>>> DFAll_IIAC 승객용-출발시간 1 >>>>>>>>>> ");
					nRet = CConfigTkMem::GetInstance()->IsPrintFirstCome(pPrtData->alcn_way_dvs_cd, pPrtData->sati_use_yn, pPrtData->depr_time_prin_yn, pPrtData->sats_no_prin_yn); // 20221207 MOD
					if( nRet >= 0 )
					{
						if(nRet == 0)
						{
							///< 4-2. 출발시간
							txtPrint(nSx,  800 + (160 * 1) + 40, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
						}
						else
						{
							///< 4-2. 출발시간
							txtPrint(nSx,  800 + (160 * 1) + 40, PRT_MODE_VERTI_EXPAND, "선착순");
							//txtPrint(nSx, 220+30, PRT_MODE_VERTI_EXPAND, "선착순");
						}
					}

					/// 좌석번호
					nRet = CConfigTkMem::GetInstance()->IsPrintSatsNo(pPrtData->alcn_way_dvs_cd, pPrtData->sats_no_prin_yn);
					if(nRet > 0)
					{
						///< 4-3. 좌석
						txtPrint(nSx,  800 + (160 * 2)+20, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
					}
				}
			}
			else 
			{
				/****
				if( (pPrtData->alcn_way_dvs_cd[0] == 'N') && (pPrtData->sati_use_yn[0] == 'N') )
				{	/// 비배차이고, 비좌석제이면, 출발시간, 좌석번호 출력안함.
					;
				}
				else
				{
					if( pPrtData->alcn_way_dvs_cd[0] != 'N' )
					{	/// 배차방식이고...
						///< 4-2. 출발시각
						txtPrint(nSx,  800 + (160 * 1) + 40, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
					}

					if( pPrtData->sati_use_yn[0] != 'N' )
					{	// 좌석제이면...
						///< 4-3. 좌석
						//if( (nSatsNo != 0) && (nSatsNo != 99) )
						{	// 0:비배차, 99:비좌석
							txtPrint(nSx,  800 + (160 * 2)+20, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
						}
					}
				}
				****/
				/// 출발시간
				TR_LOG_OUT(" >>>>>>>>>> DFAll_IIAC 승객용-출발시간 2 >>>>>>>>>> ");
				nRet = CConfigTkMem::GetInstance()->IsPrintFirstCome(pPrtData->alcn_way_dvs_cd, pPrtData->sati_use_yn, pPrtData->depr_time_prin_yn, pPrtData->sats_no_prin_yn); // 20221207 MOD
				if( nRet >= 0 )
				{
					if(nRet == 0)
					{
						///< 4-2. 출발시간
						txtPrint(nSx,  800 + (160 * 1) + 40, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
					}
					else
					{
						///< 4-2. 출발시간
						txtPrint(nSx,  800 + (160 * 1) + 40, PRT_MODE_VERTI_EXPAND, "선착순");
						//txtPrint(nSx, 220+30, PRT_MODE_VERTI_EXPAND, "선착순");
					}
				}

				/// 좌석번호
				nRet = CConfigTkMem::GetInstance()->IsPrintSatsNo(pPrtData->alcn_way_dvs_cd, pPrtData->sats_no_prin_yn);
				if(nRet > 0)
				{
					///< 4-3. 좌석
					txtPrint(nSx,  800 + (160 * 2)+20, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
				}
			}

			///< 5-1. 바코드 정보
			sprintf(Buffer, "%s-%s-%s-%s-%s %s", pPrtData->bar_pub_dt, pPrtData->bar_pub_shct_trml_cd, pPrtData->bar_pub_wnd_no, pPrtData->bar_pub_sno, pPrtData->bar_secu_code, pPrtData->pub_time);
			txtPrint(160 - (20 * 0),  800, PRT_MODE_SMALL_ON, Buffer);

#if 1
			/// 2020.06.28 add code
			///< 5-2. 운수회사
			{
				/// 비배차 : 시간, 운수회사 출력여부
				sprintf(Buffer, "운수회사:%s", pPrtData->bus_cacm_nm);
				if(pPrtData->alcn_way_dvs_cd[0] == 'D')
				{	/// 배차
					///< 5-2. 운수회사
					txtPrint(160 - (20 * 1),  800, PRT_MODE_SMALL_ON, Buffer);
				}
				else
				{	/// 비배차
					if(pEnv->tTckOpt.nalcn_time_cacm_yn[0] == 'Y')
					{	/// 비배차 : 시간, 운수사 출력유무..
						///< 5-2. 운수회사
						txtPrint(160 - (20 * 1),  800, PRT_MODE_SMALL_ON, Buffer);
					}
				}
			}
			/// ~2020.06.28 add code
#else
			///< 5-2. 운수회사
			//sprintf(Buffer, "운수회사:%s %s", pPrtData->bus_cacm_nm, pPrtData->bus_cacm_tel_nm);
			//txtPrint(160 - (20 * 1),  800, PRT_MODE_SMALL_ON, "운수회사:공항리무진");
 			sprintf(Buffer, "운수회사:%s", pPrtData->bus_cacm_nm);
 			txtPrint(160 - (20 * 1),  800, PRT_MODE_SMALL_ON, Buffer);
#endif

			///< 5-3. 자기터미널이름/전화번호
			//sprintf(Buffer, "승인금액: %s   %s", pPrtData->card_aprv_amt, pPrtData->card_no);
			sprintf(Buffer, "%s (%s)", pPrtData->depr_trml_ko_nm, pPrtData->depr_trml_tel_nm);
			//txtPrint(160 - (20 * 2),  800, PRT_MODE_SMALL_ON, "인천공항2터미널 (tel: 010-7777-8888)");
			txtPrint(160 - (20 * 2),  800, PRT_MODE_SMALL_ON, Buffer);

			///< 5-4. 신용카드
			txtPrint(160 - (20 * 4),  800, PRT_MODE_SMALL_ON, pPrtData->pym_dvs);

			sprintf(Buffer, "카드No:%s", pPrtData->card_no);
			txtPrint(160 - (20 * 4),  800+160, PRT_MODE_SMALL_ON, Buffer);

			///< 5-5. 승인금액
			sprintf(Buffer, "승인금액:%s", pPrtData->card_aprv_amt);
			txtPrint(160 - (20 * 5),  800, PRT_MODE_SMALL_ON, Buffer);
			///< 5-6. 승인No
			sprintf(Buffer, "승인No:%s", pPrtData->card_aprv_no);
			txtPrint(160 - (20 * 5),  800+160, PRT_MODE_SMALL_ON, Buffer);
		}

		///< 3. 바코드 프린트
		{
			///< 3-1. 위치
			Print("\x1b\x57%04d%04d", 93, 40);
			///< 3-2. barcode height ('h')						*
			SendData((BYTE *)"\x1D\x68\x3C", 3);
			///< 3-3. barcode width ('w')
			SendData((BYTE *)"\x1D\x77\x04", 3);	// 4배확대
			///< 3-4. barcode type ('k')
			SendData((BYTE *)"\x1D\x6B\x07", 3);
			///< 3-5. barcode data
			sprintf(Buffer, "i%s%s%s%s%s", 
					pPrtData->bar_pub_dt, 
					pPrtData->bar_pub_shct_trml_cd, 
					pPrtData->bar_pub_wnd_no, 
					pPrtData->bar_pub_sno, 
					pPrtData->bar_secu_code);
			SendData((BYTE *)Buffer, strlen(Buffer) + 1);
		}

		BeginPrint(FALSE);
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	TR_LOG_OUT(" end !!!\n\n");

	return 0;
}

/**
 * @brief		Exp_DFAll_PrintTicket
 * @details		[코버스-통합승차권] 티켓 프린트
 * @param		char *pData			프린트 데이타 구조체
 * @return		항상 = 0
 */
int CPrtTicketHS::Exp_DFAll_PrintTicket(char *pData)
{
	int					nLen, nGap, i, nOffset, nSvrKind, nMinus;
	char				Buffer[500];
	PTCK_PRINT_FMT_T	pPrtData;
	POPER_FILE_CONFIG_T pOperConfig;

	pPrtData = (PTCK_PRINT_FMT_T) pData;

	pOperConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	nLen = 0;
	nGap = 30;
	nMinus = 20;

	TR_LOG_OUT(" start !!!");
	m_bExpOnly = FALSE;

	try
	{
		BeginPrint(TRUE);

		///< 1. 회수용 티켓
		{
 			if(GetEnvOperCorp() == IIAC_OPER_CORP)
 			{
 //				sprintf(Buffer, "No. 0000%s %s(%s)", pPrtData->qr_pub_sno, GetEnvSvrUserNo(SVR_DVS_KOBUS), pPrtData->user_nm);
 				sprintf(Buffer, "( %s 무인기 )", pPrtData->qr_pub_wnd_no);
 				txtPrint(460, 400, PRT_MODE_NONE, Buffer);
 			}

			///< 1-1. 출발지
			nLen = strlen(pPrtData->depr_trml_ko_nm);
			txtPrint(390+nGap-nMinus, 40, PRT_MODE_VERTI_EXPAND, pPrtData->depr_trml_ko_nm);
			
			///< 1-2. 목적지
			nLen = strlen(pPrtData->arvl_trml_ko_nm);
			if(nLen <= 8)
			{
				txtPrint(390+nGap-nMinus, 420, PRT_MODE_BOTH_EXPAND, pPrtData->arvl_trml_ko_nm);
			}
			else
			{
				txtPrint(390+nGap-nMinus, 420, PRT_MODE_VERTI_EXPAND, pPrtData->arvl_trml_ko_nm);
			}
			
			///< 1-3. 버스등급
			{
				int nRow;

				nLen = strlen(pPrtData->bus_cls_nm);
				nRow = nLen / 4;
				if(nRow > 0)
				{
					nOffset = 0;
					for(i = 0; i < nRow; i++)
					{
						::ZeroMemory(Buffer, sizeof(Buffer));
						::CopyMemory(Buffer, &pPrtData->bus_cls_nm[nOffset], 4);
						nOffset += 4;
						txtPrint(390+nGap-(i*20)-nMinus, 700, PRT_MODE_NONE, Buffer);
					}
				}
				else
				{
					txtPrint(390+nGap-nMinus, 700, PRT_MODE_NONE, pPrtData->bus_cls_nm);
				}
			}

			///< 2-1. 영문 출발지
			txtPrint(340+nGap-nMinus, 40, PRT_MODE_SMALL_ON, pPrtData->depr_trml_eng_nm);
			///< 2-2. 영문 도착지
			txtPrint(340+nGap-nMinus, 420, PRT_MODE_SMALL_ON, pPrtData->arvl_trml_eng_nm);

			///< 3-1. 요금
			txtPrint(320+nGap-nMinus, 190+20, PRT_MODE_VERTI_EXPAND, pPrtData->tisu_amt);
			///< 3-2. 현금/신용카드, 승차권종류 
			sprintf(Buffer, "원 %s   %s", pPrtData->pym_dvs, pPrtData->bus_tck_knd_nm);
			txtPrint(300+nGap-nMinus, 270+20, PRT_MODE_NONE, Buffer);

			///< 4-1. 출발일
			txtPrint(210+nGap-nMinus,  30, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_dt);
			///< 4-2. 출발시간
			txtPrint(210+nGap-nMinus, 220, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
			///< 4-3. 좌석
			txtPrint(210+nGap-nMinus, 390-30, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
			///< 4-4. 승차홈
			txtPrint(210+nGap-nMinus, 490+10-30, PRT_MODE_VERTI_EXPAND, pPrtData->rdhm_val);
			///< 4-5. 버스회사
			txtPrint(210+nGap-nMinus, 590+10, PRT_MODE_VERTI_EXPAND, pPrtData->bus_cacm_nm);

			nGap = 40;
			// 		///< 5-1. 승인번호, 승인금액
			// 		sprintf(Buffer, "승인번호: %s, 승인금액: %s", pPrtData->card_aprv_no, pPrtData->card_aprv_amt);
			// 		txtPrint(150+nGap, 30, PRT_MODE_NONE, Buffer);
			///< 5-1. 승인번호
			sprintf(Buffer, "승인번호: %s", pPrtData->card_aprv_no);
			txtPrint(180-(nGap*0)-nMinus, 30, PRT_MODE_NONE, Buffer);
			///< 5-2. 승인금액
			sprintf(Buffer, "승인금액: %s", pPrtData->card_aprv_amt);
			txtPrint(180-(nGap*1)-nMinus, 30, PRT_MODE_NONE, Buffer);
			
			if(pPrtData->n_pym_dvs == PYM_CD_CARD)
			{
				///< 5-3. 카드번호
				sprintf(Buffer, "카드번호: %s", pPrtData->card_no);
				txtPrint(180-(nGap*2)-nMinus, 30, PRT_MODE_NONE, Buffer);
			}
			else
			{
				///< 5-3. 구매자
				sprintf(Buffer, "구매자 : %s", pPrtData->card_no);
				txtPrint(180-(nGap*2)-nMinus, 30, PRT_MODE_NONE, Buffer);
			}
			
			///< 5-4. 출발지
			sprintf(Buffer, "%s", pPrtData->depr_trml_ko_nm);
			txtPrint(180-(nGap*3)-nMinus, 30, PRT_MODE_NONE, Buffer);

			///< 6-1. (QR코드) 발행시각
			txtPrint(85+nGap-nMinus, 675, PRT_MODE_NONE, pPrtData->pub_time);
			///< 6-2. (바코드) 바코드정보
			sprintf(Buffer, "%s-%s-%s-%s", pPrtData->qr_pub_dt, pPrtData->qr_pub_trml_cd, pPrtData->qr_pub_wnd_no, pPrtData->qr_pub_sno);
			txtPrint(35+20-nMinus, 490, PRT_MODE_NONE, Buffer);

		}

		nGap = 30;
		///< 2. 승객용 티켓
		{
 			if(GetEnvOperCorp() == IIAC_OPER_CORP)
 			{
 //				sprintf(Buffer, "No. 0000%s %s(%s)", pPrtData->qr_pub_sno, GetEnvSvrUserNo(SVR_DVS_CCBUS), pPrtData->user_nm);
 				sprintf(Buffer, "( %s 무인기 )", pPrtData->qr_pub_wnd_no);
 				if( strlen(Buffer) > 26 )
 				{
 					Buffer[26] = 0;
 				}
 				txtPrint(460, 980, PRT_MODE_NONE, Buffer);
 			}

			///< 1-1. 출발지
			{
				MultiPrint(390+nGap-nMinus,  800, PRT_MODE_VERTI_EXPAND, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pPrtData->depr_trml_ko_nm);
			}

			///< 1-2. 도착지
			{
				if( strlen(pPrtData->arvl_trml_ko_nm) <= 8 )
				{
					txtPrint(390+nGap-nMinus, 1000, PRT_MODE_BOTH_EXPAND, pPrtData->arvl_trml_ko_nm);
				}
				else
				{
					MultiPrint(390+nGap-nMinus, 1000, PRT_MODE_VERTI_EXPAND, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pPrtData->arvl_trml_ko_nm);
				}
			}

			///< 2-1. 영문 출발지
			{
				::ZeroMemory(Buffer, sizeof(Buffer));
				nLen = strlen(pPrtData->depr_trml_eng_nm);
				if(nLen > 20)
				{
					nLen = 20;					
				}
				::CopyMemory(Buffer, pPrtData->depr_trml_eng_nm, nLen);
				txtPrint(340+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
			}

			{
				::ZeroMemory(Buffer, sizeof(Buffer));
				nLen = strlen(pPrtData->arvl_trml_eng_nm);
				if(nLen > 20)
				{
					nLen = 20;					
				}
				::CopyMemory(Buffer, pPrtData->arvl_trml_eng_nm, nLen);
				txtPrint(340+nGap-nMinus, 1000, PRT_MODE_SMALL_ON, Buffer);
			}

			///< 3-1. 요금
//			txtPrint(300+nGap-nMinus,  840, PRT_MODE_SMALL_ON, pPrtData->tisu_amt);
			sprintf(Buffer, "%s원", pPrtData->tisu_amt);
			txtPrint(310+nGap-nMinus,  840+20, PRT_MODE_NONE, Buffer);
			///< 3-2. 현금/신용카드, 승차권종류
			sprintf(Buffer, "    %s %s", pPrtData->pym_dvs, pPrtData->bus_tck_knd_nm);
			txtPrint(300+nGap-nMinus,  900+20, PRT_MODE_SMALL_ON, Buffer);

			///< 4-1. 출발일
			txtPrint(210+nGap-nMinus,  800 + (160 * 0), PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_dt);
			///< 4-2. 출발시각
			txtPrint(210+nGap-nMinus,  800 + (160 * 1) + 10, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
			///< 4-3. 좌석
			txtPrint(210+nGap-nMinus,  800 + (160 * 2), PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);

			///< 5-1. 바코드 정보
			sprintf(Buffer, "%s-%s-%s-%s", pPrtData->qr_pub_dt, pPrtData->qr_pub_trml_cd, pPrtData->qr_pub_wnd_no, pPrtData->qr_pub_sno);
			txtPrint(150 - (20 * 0)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);

			///< 5-2. 승인번호 / 발권시간
			sprintf(Buffer, "승인번호: %s %s", pPrtData->card_aprv_no, pPrtData->pub_time);
			txtPrint(150 - (20 * 1)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
			///< 5-3. 승인금액
			sprintf(Buffer, "승인금액: %s", pPrtData->card_aprv_amt);
			txtPrint(150 - (20 * 2)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
			///< 5-4. 구매자정보
			sprintf(Buffer, "구매자정보: %s", pPrtData->card_no);
			txtPrint(150 - (20 * 3)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
			///< 5-4. 가맹점번호
			nSvrKind = GetConfigServerKind();
			if(nSvrKind & SVR_DVS_KOBUS)
			{
				TR_LOG_OUT("코버스 가맹점번호(%d) ..", pOperConfig->koTrmlInfo_t.sz_prn_trml_corp_no);
				sprintf(Buffer, "가맹점사업자번호:%s", pOperConfig->koTrmlInfo_t.sz_prn_trml_corp_no);
			}
			else
			{
				TR_LOG_OUT("이지고속버스 가맹점번호(%d) ..", pOperConfig->ezTrmlInfo_t.sz_prn_trml_corp_no);
				sprintf(Buffer, "가맹점사업자번호:%s", pOperConfig->ezTrmlInfo_t.sz_prn_trml_corp_no);
			}
			txtPrint(150 - (20 * 4)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
			///< 5-5. 메시지
			txtPrint(150 - (20 * 5)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, "*유효기간 당일 지정차에 한함");

			///< 5-6. 현금영수증 전번
			txtPrint(150 - (20 * 6)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, "현금영수증 문의 : 126-2");

			///< 5-7. 버스회사 명
			sprintf(Buffer, "버스회사:%s", pPrtData->bus_cacm_nm);
			txtPrint(150 - (20 * 7)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
		}

		///< 3. QR코드 프린트
		{
			///< 3-1. 페이지모드 인자영역 지정 
			//Print("\x1b\x57%04d%04d", 180, 560);
			Print("\x1b\x57%04d%04d", 170, 560);
			///< 3-2. QR 바코드 
			sprintf(Buffer, "%s%s%s%s%s%s%s", 
				pPrtData->qr_pub_dt, 
				pPrtData->qr_pub_trml_cd, 
				pPrtData->qr_pub_wnd_no, 
				pPrtData->qr_pub_sno, 
				pPrtData->qr_depr_trml_no, 
				pPrtData->qr_arvl_trml_no, 
				pPrtData->qr_sats_no);
			Print("\x1A\x42\x02\x19\x03%s", Buffer);
		}

		BeginPrint(FALSE);
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}


	TR_LOG_OUT(" end !!!\n\n");

	return 0;
}

/**
 * @brief		Exp_Only_PrintTicket
 * @details		[코버스-고속전용승차권] 티켓 프린트
 * @param		char *pData			프린트 데이타 구조체
 * @return		항상 = 0
 */
void CPrtTicketHS::DrawBox(BOOL bPassenger)
{
	int i, nGap, nX, nY;

	i = nGap = nX = nY = 0;

	if(bPassenger == FALSE)
	{	/// 회수용
		i = 0;
		nGap = 21;

		txtPrint(195+(nGap*i), 1320, PRT_MODE_NONE, "┌───────────────────");  i++;
		txtPrint(195+(nGap*i), 1320, PRT_MODE_NONE, "│ ");  i++;
		txtPrint(195+(nGap*i), 1320, PRT_MODE_NONE, "│ ");  i++;
		txtPrint(195+(nGap*i), 1320, PRT_MODE_NONE, "├───────────────────");  i++;
		txtPrint(195+(nGap*i), 1320, PRT_MODE_NONE, "│ ");  i++;
		txtPrint(195+(nGap*i), 1320, PRT_MODE_NONE, "│ ");  i++;
		txtPrint(195+(nGap*i), 1320, PRT_MODE_NONE, "└───────────────────");  i++;

		nGap = 23;
		nX = 204;
		txtPrint(nX+(nGap*0), 1080, PRT_MODE_NONE, "│");
		txtPrint(nX+(nGap*1), 1080, PRT_MODE_NONE, "│");
		txtPrint(nX+(nGap*2), 1080, PRT_MODE_NONE, "│");
		txtPrint(nX+(nGap*3), 1080, PRT_MODE_NONE, "│");
		nX = 195;
		txtPrint(nX+(nGap*4), 1080, PRT_MODE_NONE, "│");
		txtPrint(nX+(nGap*5), 1080, PRT_MODE_NONE, "│");

		nX = 204;
		txtPrint(nX+(nGap*0), 944, PRT_MODE_NONE, "│");
		txtPrint(nX+(nGap*1), 944, PRT_MODE_NONE, "│");
		txtPrint(nX+(nGap*2), 944, PRT_MODE_NONE, "│");
		txtPrint(nX+(nGap*3), 944, PRT_MODE_NONE, "│");
		nX = 195;
		txtPrint(nX+(nGap*4), 944, PRT_MODE_NONE, "│");
		txtPrint(nX+(nGap*5), 944, PRT_MODE_NONE, "│");

		nX = 204;
		txtPrint(nX+(nGap*0), 832, PRT_MODE_NONE, "│");
		txtPrint(nX+(nGap*1), 832, PRT_MODE_NONE, "│");
		txtPrint(nX+(nGap*2), 832, PRT_MODE_NONE, "│");
		txtPrint(nX+(nGap*3), 832, PRT_MODE_NONE, "│");
		nX = 195;
		txtPrint(nX+(nGap*4), 832, PRT_MODE_NONE, "│");
		txtPrint(nX+(nGap*5), 832, PRT_MODE_NONE, "│");
	}
	else
	{	/// 승객용
		///< 4-8. 박스 그리기
		i = 0;
		nGap = 21; 
		nY = 760;
		//nY = 762;
		txtPrint(195+(nGap*i), nY, PRT_MODE_NONE, "┌─────────────────────────────");  i++;
		txtPrint(195+(nGap*i), nY, PRT_MODE_NONE, "│ ");  i++;
		txtPrint(195+(nGap*i), nY, PRT_MODE_NONE, "│ ");  i++;
		txtPrint(195+(nGap*i), nY, PRT_MODE_NONE, "├─────────────────────────────");  i++;
		txtPrint(195+(nGap*i), nY, PRT_MODE_NONE, "│ ");  i++;
		txtPrint(195+(nGap*i), nY, PRT_MODE_NONE, "│ ");  i++;
		txtPrint(195+(nGap*i), nY, PRT_MODE_NONE, "└─────────────────────────────");  i++;

		nGap = 23; 
		for(i = 0; i < 5; i++)
		{
			nX = 204;
			switch(i)
			{
//			case 0 : nY = 536; break;
			case 0 : nY = 539; break;
			case 1 : nY = 416; break;
			case 2 : nY = 304; break;
			case 3 : nY = 152; break;
			case 4 : nY = 32-4-4; break;
//			case 4 : nY = 37; break;
			}
			//nY = 536;
			txtPrint(nX+(nGap*0), nY, PRT_MODE_NONE, "│");
			txtPrint(nX+(nGap*1), nY, PRT_MODE_NONE, "│");
			txtPrint(nX+(nGap*2), nY, PRT_MODE_NONE, "│");
			txtPrint(nX+(nGap*3), nY, PRT_MODE_NONE, "│");
			nX = 195;
			txtPrint(nX+(nGap*4), nY, PRT_MODE_NONE, "│");
			txtPrint(nX+(nGap*5), nY, PRT_MODE_NONE, "│");
		}
	}
}

/**
 * @brief		Exp_Only_PrintTicket
 * @details		[코버스-고속전용승차권] 티켓 프린트
 * @param		char *pData			프린트 데이타 구조체
 * @return		항상 = 0
 */
int CPrtTicketHS::ExpOnlyPrintTicket(char *pData)
{
	int					i, nLen, nOffset;
	char				Buffer[500];
	PTCK_PRINT_FMT_T	pPrtData;
	POPER_FILE_CONFIG_T pOperConfig;

	pPrtData = (PTCK_PRINT_FMT_T) pData;

	pOperConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

//	nLen = 0;
//	nGap = 30;
//	nMinus = 20;

	TR_LOG_OUT(" start !!!");

	m_bExpOnly = TRUE;

	try
	{
		BeginPrint(TRUE);

		///< 1. 회수용 티켓
		{
			///< 1-0. 박스그리기
			DrawBox(FALSE);

			///< 1-1. 출발지
			nLen = strlen(pPrtData->depr_trml_ko_nm);
			txtPrint(72, 1300, PRT_MODE_VERTI_EXPAND, pPrtData->depr_trml_ko_nm);

			txtPrint(72, 1096-6, PRT_MODE_VERTI_EXPAND, "▷");

			///< 1-2. 목적지
			nLen = strlen(pPrtData->arvl_trml_ko_nm);
			txtPrint(72, 1050, PRT_MODE_VERTI_EXPAND, pPrtData->arvl_trml_ko_nm);
			///< 1-3. 버스등급
			{
				int nRow;

				nLen = strlen(pPrtData->bus_cls_nm);
				nRow = nLen / 4;
				//if(nRow > 0)
				if( (nRow > 0) && (nLen > 4) )
				{
					nOffset = 0;
					for(i = 0; i < nRow; i++)
					{
						::ZeroMemory(Buffer, sizeof(Buffer));
						::CopyMemory(Buffer, &pPrtData->bus_cls_nm[nOffset], 4);
						nOffset += 4;
						txtPrint(72+(i*20), 848, PRT_MODE_NONE, Buffer);
					}
				}
				else
				{
					txtPrint(72, 848, PRT_MODE_VERTI_EXPAND, pPrtData->bus_cls_nm);
				}
			}

			///< 1-4. 승차권 종류
			txtPrint(128+(24*0), 1040, PRT_MODE_NONE, pPrtData->bus_tck_knd_nm);

			///< 1-5. 지불수단
			txtPrint(128+(24*1), 1040, PRT_MODE_NONE, pPrtData->pym_dvs);

			///< 1-6. 타이틀 ("원 (부가가치세포함)")
			txtPrint(128+(24*2), 1040, PRT_MODE_NONE, "원 (부가가치세포함)");

			///< 1-7. 타이틀 ("요금")
			txtPrint(172, 1300, PRT_MODE_NONE, "요금");

			///< 1-8. 요금
			txtPrint(128+(24*1), 1240, PRT_MODE_VERTI_EXPAND, pPrtData->tisu_amt);
			txtPrint(128+(24*1)+1, 1240+1, PRT_MODE_VERTI_EXPAND, pPrtData->tisu_amt);

			///< 중앙
			///< 2-1. 타이틀 ("출발일")
			txtPrint(208, 1256, PRT_MODE_NONE, "출 발 일");

			///< 2-2. 타이틀 ("Date of depature")
			txtPrint(240, 1288, PRT_MODE_NONE, "Date of depature");

			///< 2-3. 타이틀 ("출발시각")
			txtPrint(208, 1048, PRT_MODE_NONE, "출발시각");

			///< 2-4. 타이틀 ("Time")
			txtPrint(240, 1024, PRT_MODE_NONE, "Time");

			///< 2-5. 타이틀 ("좌석")
			txtPrint(208, 920, PRT_MODE_NONE, "좌석");

			///< 2-6. 타이틀 ("Seat No")
			txtPrint(240, 928, PRT_MODE_NONE, "Seat No");

			///< 2-7. 출발일
			txtPrint(282, 1240, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_dt);

			///< 2-8. 출발시각
			txtPrint(282, 1024, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);

			///< 2-9. 좌석번호
			txtPrint(282,  904, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);

			///< 하단
			///< 3-1. 승인금액 / QR Full Code
			sprintf(Buffer, "승인 %s  %s-%s-%s-%s %s%s%s", pPrtData->card_aprv_amt, 
					pPrtData->qr_pub_dt, pPrtData->qr_pub_trml_cd, pPrtData->qr_pub_wnd_no, pPrtData->qr_pub_sno, 
					pPrtData->qr_depr_trml_no, pPrtData->qr_arvl_trml_no, pPrtData->qr_sats_no);
			txtPrint(344+(26*0), 1300+20, PRT_MODE_NONE, Buffer);

			///< 3-2. 고속회사 이름 / 발행시간
			sprintf(Buffer, "고속회사:%s  %s", pPrtData->bus_cacm_nm, pPrtData->pub_time);
			txtPrint(344+(26*1), 1300+20, PRT_MODE_NONE, Buffer);

			///< 3-3. 출발터미널
			sprintf(Buffer, "출발터미널:%s", pPrtData->depr_trml_ko_nm);
			txtPrint(344+(26*2), 1300+20, PRT_MODE_NONE, Buffer);

			///< 3-4. 출발터미널_사업자번호
			txtPrint(344+(26*3), 1300+20, PRT_MODE_NONE, pPrtData->depr_trml_biz_no);

			///< 3-5. 인증번호
			if(pPrtData->n_pym_dvs == PYM_CD_CARD)
			{	/// 신용카드 인 경우
				sprintf(Buffer, "인증 %s", pPrtData->card_no);
				txtPrint(344+(26*4), 1300+20, PRT_MODE_NONE, Buffer);
			}

			///< 3-6. ???
			//sprintf(Buffer, "000168");
			//txtPrint(344+(26*4), 1008, PRT_MODE_NONE, Buffer);

		}

		///< 2. 승객용 티켓
		{
			///< 2-1. 출발지
			{
				MultiPrint(72, 736, PRT_MODE_VERTI_EXPAND, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pPrtData->depr_trml_ko_nm);

// 				nLen = strlen(pPrtData->depr_trml_ko_nm);
// 				LOG_OUT("출발지 (%s), len(%d)", pPrtData->depr_trml_ko_nm, nLen);
// 				if(nLen > MAX_HANGUL_CHAR)
// 				{
// 					::ZeroMemory(Buffer, sizeof(Buffer));
// 
// 					nRet = IsHangule(pPrtData->depr_trml_ko_nm, MAX_HANGUL_CHAR);
// 					if(nRet < 0)
// 					{
// 						::CopyMemory(Buffer, pPrtData->depr_trml_ko_nm, MAX_HANGUL_CHAR - 1);
// 					}
// 					else
// 					{
// 						::CopyMemory(Buffer, pPrtData->depr_trml_ko_nm, MAX_HANGUL_CHAR);
// 					}
// 					txtPrint(72, 736, PRT_MODE_VERTI_EXPAND, Buffer);
// 				}
// 				else
// 				{
// 					txtPrint(72, 736, PRT_MODE_VERTI_EXPAND, pPrtData->depr_trml_ko_nm);
// 				}
			}

			txtPrint(72, 496, PRT_MODE_VERTI_EXPAND, "▷");

			///< 2-2. 도착지
			{
				MultiPrint(72, 464, PRT_MODE_VERTI_EXPAND, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pPrtData->arvl_trml_ko_nm);

// 				nLen = strlen(pPrtData->arvl_trml_ko_nm);
// 				if(nLen > MAX_HANGUL_CHAR)
// 				{
// 					::ZeroMemory(Buffer, sizeof(Buffer));
// 					nRet = IsHangule(pPrtData->arvl_trml_ko_nm, MAX_HANGUL_CHAR);
// 					if(nRet < 0)
// 					{
// 						::CopyMemory(Buffer, pPrtData->arvl_trml_ko_nm, MAX_HANGUL_CHAR - 1);
// 					}
// 					else
// 					{
// 						::CopyMemory(Buffer, pPrtData->arvl_trml_ko_nm, MAX_HANGUL_CHAR);
// 					}
// 					txtPrint(72, 464, PRT_MODE_VERTI_EXPAND, Buffer);
// 				}
// 				else
// 				{
// 					txtPrint(72, 464, PRT_MODE_VERTI_EXPAND, pPrtData->arvl_trml_ko_nm);
// 				}
			}

			///< 2-3. 영문 출발지
			{
				::ZeroMemory(Buffer, sizeof(Buffer));
				nLen = strlen(pPrtData->depr_trml_eng_nm);
				if(nLen > 20)
				{
					nLen = 20;					
				}
				::CopyMemory(Buffer, pPrtData->depr_trml_eng_nm, nLen);
				txtPrint(136, 736, PRT_MODE_SMALL_ON, Buffer);
			}
			
			///< 2-4. 영문 도착지
			{
				::ZeroMemory(Buffer, sizeof(Buffer));
				nLen = strlen(pPrtData->arvl_trml_eng_nm);
				if(nLen > 20)
				{
					nLen = 20;					
				}
				::CopyMemory(Buffer, pPrtData->arvl_trml_eng_nm, nLen);
				txtPrint(136, 464, PRT_MODE_SMALL_ON, Buffer);
			}

			///< 2-5. 버스등급
			{
				int nRow;

				nLen = strlen(pPrtData->bus_cls_nm);
				nRow = nLen / 4;
				if( (nRow > 0) && (nLen > 4) )
				{
					nOffset = 0;
					for(i = 0; i < nRow; i++)
					{
						::ZeroMemory(Buffer, sizeof(Buffer));
						::CopyMemory(Buffer, &pPrtData->bus_cls_nm[nOffset], 4);
						nOffset += 4;
						txtPrint(72+(i*20), 184, PRT_MODE_NONE, Buffer);
					}
				}
				else
				{
					txtPrint(72, 184, PRT_MODE_VERTI_EXPAND, pPrtData->bus_cls_nm);
				}
			}
			//txtPrint(72, 184, PRT_MODE_VERTI_EXPAND, pPrtData->bus_cls_nm);

			///< 2-6. 승차권 종류
			txtPrint(72, 120, PRT_MODE_VERTI_EXPAND, pPrtData->bus_tck_knd_nm);

			///< 2-7. 타이틀("요금")
			txtPrint(172, 744, PRT_MODE_NONE, "요금");

			///< 2-7. 요금
			txtPrint(128+(24*1), 656, PRT_MODE_VERTI_EXPAND, pPrtData->tisu_amt);
			txtPrint(128+(24*1)+1, 656+1, PRT_MODE_VERTI_EXPAND, pPrtData->tisu_amt);

			///< 2-8. 타이틀("원 (부가가치세포함)")
			txtPrint(172, 464, PRT_MODE_NONE, "원 (부가가치세포함)");

			///< 중앙

			///< 3-1. 타이틀("출 발 일")
			txtPrint(208, 690, PRT_MODE_NONE, "출 발 일");

			///< 3-2. 타이틀("출 발 일")
			txtPrint(240, 736, PRT_MODE_NONE, "Date of depature");

			///< 3-3. 타이틀("출발시각")
			txtPrint(208, 520, PRT_MODE_NONE, "출발시각");

			///< 3-4. 타이틀("Time")
			txtPrint(240, 488, PRT_MODE_NONE, "Time");

			///< 3-5. 타이틀("좌석")
			txtPrint(208, 384, PRT_MODE_NONE, "좌석");

			///< 3-6. 타이틀("Seat No")
			txtPrint(240, 392, PRT_MODE_NONE, "Seat No");

			///< 3-7. 타이틀("운송회사")
			txtPrint(208, 272, PRT_MODE_NONE, "운송회사");

			///< 3-8. 타이틀("Express co")
			txtPrint(240, 280, PRT_MODE_NONE, "Express co.");

			///< 3-9. 타이틀("승차홈")
			txtPrint(208, 120, PRT_MODE_NONE, "승차홈");

			///< 3-8. 타이틀("Platform")
			txtPrint(240, 128, PRT_MODE_NONE, "Platform");

			///< 3-9. 출발일
			txtPrint(282, 690, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_dt);
			
			///< 3-10. 출발시각
			txtPrint(282, 512, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);

			///< 3-11. 좌석
			txtPrint(282, 368, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);

			///< 3-12. 운송회사
			txtPrint(282, 264, PRT_MODE_VERTI_EXPAND, pPrtData->bus_cacm_nm);

			///< 3-13. 승차홈
			txtPrint(282, 112, PRT_MODE_VERTI_EXPAND, pPrtData->rdhm_val);

			///< 하단

			///< 4-1. 승인번호 / 승인금액 / 카드번호
			if(pPrtData->n_pym_dvs == PYM_CD_CARD)
			{
				sprintf(Buffer, "승인번호 %s   승인 %s   인증 %s", pPrtData->card_aprv_no, pPrtData->card_aprv_amt, pPrtData->card_no);
			}
			else
			{
				sprintf(Buffer, "승인번호 %s   승인 %s", pPrtData->card_aprv_no, pPrtData->card_aprv_amt);
			}
			txtPrint(344+(26*0), 744, PRT_MODE_NONE, Buffer);

			///< 4-2. 가맹점 사업자번호
			sprintf(Buffer, "가맹점사업자번호:%s", pOperConfig->koTrmlInfo_t.sz_prn_trml_corp_no);
			txtPrint(344+(26*1), 744, PRT_MODE_NONE, Buffer);

			///< 4-3. 지불수단
			txtPrint(344+(26*1), 280, PRT_MODE_VERTI_EXPAND, pPrtData->pym_dvs);
			txtPrint(344+(26*1)+1, 280+1, PRT_MODE_VERTI_EXPAND, pPrtData->pym_dvs);

			///< 4-4. 타이틀 / 발행시간
			sprintf(Buffer, "*유효기간:당일지정차에 한함 %s", pPrtData->pub_time);
			txtPrint(344+(26*2), 744, PRT_MODE_NONE, Buffer);

			///< 4-5. 고속회사 정보
			sprintf(Buffer, "*고속회사:%s %s", pPrtData->bus_cacm_nm, pPrtData->bus_cacm_biz_no);
			txtPrint(344+(26*3), 744, PRT_MODE_NONE, Buffer);

			///< 4-6. QR Data
//			sprintf(Buffer, "%s-%s-%s-%s", pPrtData->qr_pub_dt, pPrtData->qr_depr_trml_no, pPrtData->qr_pub_wnd_no, pPrtData->qr_pub_sno);
			sprintf(Buffer, "%s-%s-%s-%s", pPrtData->qr_pub_dt, pPrtData->qr_pub_trml_cd, pPrtData->qr_pub_wnd_no, pPrtData->qr_pub_sno);
			txtPrint(344+(26*3), 280, PRT_MODE_NONE, Buffer);
			txtPrint(344+(26*3)+1, 280+1, PRT_MODE_NONE, Buffer);

			if(pPrtData->n_pym_dvs != PYM_CD_CARD)
			{
				txtPrint(344+(26*4), 280, PRT_MODE_NONE, "현금영수증문의 ☎126-2");	
			}

			///< 4-7. 출발터미널-사업자번호
			txtPrint(344+(26*4), 744, PRT_MODE_NONE, pPrtData->depr_trml_biz_no);

			///< 4-8. 박스 그리기
			DrawBox(TRUE);
		}

		///< 3. QR코드 프린트
		{
			///< 3-1. 페이지모드 인자영역 지정 
			//Print("\x1b\x57%04d%04d", 180, 560);
			//Print("\x1b\x57%04d%04d", 170, 560);
			Print("\x1b\x57%04d%04d", 365, 900);
			///< 3-2. QR 바코드 
			sprintf(Buffer, "%s%s%s%s%s%s%s", 
					pPrtData->qr_pub_dt, 
					pPrtData->qr_pub_trml_cd, 
					pPrtData->qr_pub_wnd_no, 
					pPrtData->qr_pub_sno, 
					pPrtData->qr_depr_trml_no, 
					pPrtData->qr_arvl_trml_no, 
					pPrtData->qr_sats_no);
			Print("\x1A\x42\x02\x19\x03%s", Buffer);
		}

		BeginPrint(FALSE);
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}


	TR_LOG_OUT(" end !!!\n\n");

	return 0;
}

void CPrtTicketHS::ID_Print(int nSvrKind, int nPart, int nID, char *pData)
{
	PPTRG_INFO_T pPtrg;
	
	pPtrg = (PPTRG_INFO_T) INI_GetEnvTckPtrgInfo(nSvrKind, nPart, nID);
	if(pPtrg != (PPTRG_INFO_T) NULL)
	{
		if(nPart == PTRG_CUST_PART)
		{
			if( (nID == PTRG_ID2_DEPR_KO) || (nID == PTRG_ID2_ARVL_KO) )
			{
				MultiPrint(pPtrg->nX, pPtrg->nY, pPtrg->nMode, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pData);
				return;
			}
		}

		txtPrint(pPtrg->nX, pPtrg->nY, pPtrg->nMode, pData);
	}
}

void CPrtTicketHS::EngTrml_Print(int nSvrKind, int nPart, int nID, char *pData)
{
	int		nLen;
	char	Buffer[500];

	::ZeroMemory(Buffer, sizeof(Buffer));
	nLen = strlen(pData);
	if(nLen > 20)
	{
		nLen = 20;					
	}
	::CopyMemory(Buffer, pData, nLen);
	//txtPrint(340+nGap-nMinus,  780, PRT_MODE_SMALL_ON, Buffer);
	ID_Print(nSvrKind, nPart, nID, Buffer);
}

/***
int CPrtTicketHS::TestCCS_DFAll_PrintTicket(char *pData)
{
	int					nLen, nGap, nSatsNo, nMinus, nRet;
	int					nSvrKind, nPart;
	char				Buffer[500];
	PTCK_PRINT_FMT_T	pPrtData;
	POPER_FILE_CONFIG_T pOperConfig;

	pPrtData = (PTCK_PRINT_FMT_T) pData;

	pOperConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	nLen = nRet = 0;

	nSatsNo = Util_Ascii2Long(pPrtData->sats_no, strlen(pPrtData->sats_no));

	TR_LOG_OUT(" start !!!\n\n");
	m_bExpOnly = FALSE;

	nSvrKind = SVR_DVS_CCBUS;

	try
	{
		PPTRG_INFO_T pPtrg;

		BeginPrint(TRUE);

		nGap = 30;
		nMinus = 20;
		///< 1. 회수용 티켓
		{
			nPart = PTRG_TRML_PART;

			///< 1-1. 출발지
			nLen = strlen(pPrtData->depr_trml_ko_nm);
			ID_Print(nSvrKind, nPart, PTRG_ID2_DEPR_KO, pPrtData->depr_trml_ko_nm);

			///< 1-2. 경유지
			nLen = strlen(pPrtData->thru_nm);
			if(nLen > 0)
			{
				//txtPrint(340+nGap-nMinus, 270, PRT_MODE_NONE, pPrtData->thru_nm);
				ID_Print(nSvrKind, nPart, PTRG_ID2_THRU_NM, pPrtData->thru_nm);
			}

			///< 1-2. 목적지
			nLen = strlen(pPrtData->arvl_trml_ko_nm);
			//txtPrint(390+nGap-nMinus, 420, PRT_MODE_VERTI_EXPAND, pPrtData->arvl_trml_ko_nm);
			ID_Print(nSvrKind, nPart, PTRG_ID2_ARVL_KO, pPrtData->arvl_trml_ko_nm);

			///< 1-3. 버스등급
			{
				int i, nRow, nOffset;

				i = nRow = nOffset = 0;

				nLen = strlen(pPrtData->bus_cls_nm);
				nRow = nLen / 4;
				if(nRow > 0)
				{
					nOffset = 0;
					for(i = 0; i < nRow; i++)
					{
						::ZeroMemory(Buffer, sizeof(Buffer));
						::CopyMemory(Buffer, &pPrtData->bus_cls_nm[nOffset], 4);
						nOffset += 4;
						//txtPrint(390+nGap-(i*20)-nMinus, 700, PRT_MODE_NONE, Buffer);
						// todo ????
					}
				}
				else
				{
					//txtPrint(390+nGap-nMinus, 700, PRT_MODE_NONE, pPrtData->bus_cls_nm);
					ID_Print(nSvrKind, nPart, PTRG_ID2_BUS_CLS, pPrtData->bus_cls_nm);
				}
			}

			///< 2-1. 영문 출발지
			//txtPrint(340+nGap-nMinus, 40, PRT_MODE_SMALL_ON, pPrtData->depr_trml_eng_nm);
			ID_Print(nSvrKind, nPart, PTRG_ID2_DEPR_EN, pPrtData->depr_trml_eng_nm);
			
			///< 2-2. 영문 도착지
			//txtPrint(340+nGap-nMinus, 420, PRT_MODE_SMALL_ON, pPrtData->arvl_trml_eng_nm);
			ID_Print(nSvrKind, nPart, PTRG_ID2_ARVL_EN, pPrtData->arvl_trml_eng_nm);

			///< 3-1. 요금
			//txtPrint(320+nGap-nMinus, 190, PRT_MODE_VERTI_EXPAND, pPrtData->tisu_amt);
			ID_Print(nSvrKind, nPart, PTRG_ID3_FARE, pPrtData->tisu_amt);

			///< 3-2. 현금/신용카드, 승차권종류 
//			sprintf(Buffer, " 원    %s   (무인)  %s", pPrtData->pym_dvs, pPrtData->bus_tck_knd_nm);
			sprintf(Buffer, " 원    %s         %s", pPrtData->pym_dvs, pPrtData->bus_tck_knd_nm);
			//txtPrint(300+nGap-nMinus, 270, PRT_MODE_NONE, Buffer);
			ID_Print(nSvrKind, nPart, PTRG_ID3_ALL_ITM1, Buffer);

			///< 4-1. 출발일
			//txtPrint(210+nGap-nMinus,  30, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_dt);
			ID_Print(nSvrKind, nPart, PTRG_ID5_DEPR_DT, pPrtData->atl_depr_dt);

			LOG_OUT("배차방식   = %c ", pPrtData->alcn_way_dvs_cd[0]);
			LOG_OUT("좌석제유무 = %c ", pPrtData->sati_use_yn[0]);

			///< 4-2. 출발시간
			if(pPrtData->sati_use_yn[0] == 'N')
			{	/// 비좌석제이면, 출발시간, 좌석번호 출력안함.
				txtPrint(220+nGap-nMinus, 220, PRT_MODE_NONE, "<<선착순 탑승>>");
				txtPrint(190+nGap-nMinus, 220, PRT_MODE_SMALL_ON, "미탑승분은 다음차 이용");
			}
			else
			{
				/// 출발시간
				//txtPrint(210+nGap-nMinus, 220, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
				ID_Print(nSvrKind, nPart, PTRG_ID5_DEPR_TIME, pPrtData->atl_depr_time);

				/// 좌석번호
				//txtPrint(210+nGap-nMinus, 390, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
				ID_Print(nSvrKind, nPart, PTRG_ID5_SATS_NO, pPrtData->sats_no);
			}

// 			if( (pPrtData->alcn_way_dvs_cd[0] == 'N') && ((nSatsNo == 0) || (nSatsNo == 99)) )
// 			{	/// 비배차, 비좌석제는 출발시간/ 좌석번호를 프린트 안함.
// 				;
// 			}
// 			else
// 			{
// 				///< 4-3. 좌석
// 				if( (nSatsNo != 0) && (nSatsNo != 99) )
// 				{	// 0:비배차, 99:비좌석
// 					/// 출발시간
// 					txtPrint(210+nGap-nMinus, 220, PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
// 					/// 좌석번호
// 					txtPrint(210+nGap-nMinus, 390, PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
// 				}
// 			}
			
			///< 4-4. 승차홈
			//txtPrint(210+nGap-nMinus, 490, PRT_MODE_VERTI_EXPAND, pPrtData->rdhm_val);
			ID_Print(nSvrKind, nPart, PTRG_ID5_RDHM_VAL, pPrtData->rdhm_val);

			///< 4-5. 버스회사
			//txtPrint(210+nGap-nMinus, 590, PRT_MODE_VERTI_EXPAND, pPrtData->bus_cacm_nm);
			ID_Print(nSvrKind, nPart, PTRG_ID5_BUS_CACM, pPrtData->bus_cacm_nm);

			///< 5-1. 승인번호, 승인금액
			sprintf(Buffer, "승인번호: %s  승인금액: %s  %s", pPrtData->card_aprv_no, pPrtData->card_aprv_amt, pPrtData->card_no);
			//txtPrint(150+nGap-nMinus, 30, PRT_MODE_NONE, Buffer);
			ID_Print(nSvrKind, nPart, PTRG_ID6_APRV_ALL_CARD, Buffer);

			///< 6-1. (바코드) 발권시간
			//txtPrint(65+nGap-nMinus, 620, PRT_MODE_NONE, pPrtData->pub_time);
			ID_Print(nSvrKind, nPart, PTRG_ID6_PUB_TIME, pPrtData->pub_time);

			///< 6-2. (바코드) 출발지
			//txtPrint(35+nGap-nMinus, 30, PRT_MODE_NONE, pPrtData->depr_trml_ko_nm);
			ID_Print(nSvrKind, nPart, PTRG_ID6_DEPR_NM, pPrtData->depr_trml_ko_nm);

			///< 6-3. (바코드) 바코드정보
			sprintf(Buffer, "%s-%s-%s-%s-%s", pPrtData->pub_dt, pPrtData->pub_shct_trml_cd, pPrtData->pub_wnd_no, pPrtData->pub_sno, pPrtData->secu_code);
			//txtPrint(35+nGap-nMinus, 310, PRT_MODE_NONE, Buffer);
			ID_Print(nSvrKind, nPart, PTRG_ID6_BAR_DATA, Buffer);

		}

		///< 2. 승객용 티켓
		{
			nPart = PTRG_CUST_PART;

			///< 1-1. 출발지
			{
				ID_Print(nSvrKind, nPart, PTRG_ID2_DEPR_KO, pPrtData->depr_trml_ko_nm);

// 				pPtrg = (PPTRG_INFO_T) INI_GetEnvTckPtrgInfo(nSvrKind, nPart, PTRG_ID2_DEPR_KO);
// 				if(pPtrg != (PPTRG_INFO_T) NULL)
// 				{
// 					//MultiPrint(390+nGap-nMinus,  780, PRT_MODE_VERTI_EXPAND, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pPrtData->depr_trml_ko_nm);
// 					MultiPrint(pPtrg->nX, pPtrg->nY, pPtrg->nMode, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pPrtData->depr_trml_ko_nm);
			}

			///< 1-2. 도착지
			{
				ID_Print(nSvrKind, nPart, PTRG_ID2_ARVL_KO, pPrtData->arvl_trml_ko_nm);

// 				pPtrg = (PPTRG_INFO_T) INI_GetEnvTckPtrgInfo(nSvrKind, nPart, PTRG_ID2_ARVL_KO);
// 				if(pPtrg != (PPTRG_INFO_T) NULL)
// 				{
// 					//MultiPrint(390+nGap-nMinus, 1000, PRT_MODE_VERTI_EXPAND, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pPrtData->arvl_trml_ko_nm);
// 					MultiPrint(pPtrg->nX, pPtrg->nY, pPtrg->nMode, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pPrtData->arvl_trml_ko_nm);
// 				}
			}

			// org code
			//txtPrint(390+nGap, 1030, PRT_MODE_VERTI_EXPAND, pPrtData->arvl_trml_ko_nm);

			///< 2-1. 영문 출발지
			{
				EngTrml_Print(nSvrKind, nPart, PTRG_ID2_DEPR_EN, pPrtData->depr_trml_eng_nm);

// 				::ZeroMemory(Buffer, sizeof(Buffer));
// 				nLen = strlen(pPrtData->depr_trml_eng_nm);
// 				if(nLen > 20)
// 				{
// 					nLen = 20;					
// 				}
// 				::CopyMemory(Buffer, pPrtData->depr_trml_eng_nm, nLen);
// 				//txtPrint(340+nGap-nMinus,  780, PRT_MODE_SMALL_ON, Buffer);
// 				ID_Print(nSvrKind, nPart, PTRG_ID2_DEPR_EN, Buffer);
			}

			{
				EngTrml_Print(nSvrKind, nPart, PTRG_ID2_ARVL_EN, pPrtData->arvl_trml_eng_nm);

// 				::ZeroMemory(Buffer, sizeof(Buffer));
// 				nLen = strlen(pPrtData->arvl_trml_eng_nm);
// 				if(nLen > 20)
// 				{
// 					nLen = 20;					
// 				}
// 				::CopyMemory(Buffer, pPrtData->arvl_trml_eng_nm, nLen);
// 				//txtPrint(340+nGap-nMinus, 1000, PRT_MODE_SMALL_ON, Buffer);
// 				ID_Print(nSvrKind, nPart, PTRG_ID2_ARVL_EN, Buffer);
			}

			///< 3-1. 요금				  PRT_MODE_NONE
			sprintf(Buffer, "%s원", pPrtData->tisu_amt);
			//txtPrint(310+nGap-nMinus,  840, PRT_MODE_NONE, Buffer);
			ID_Print(nSvrKind, nPart, PTRG_ID3_FARE, Buffer);

			///< 3-2. 현금/신용카드, 승차권종류
			sprintf(Buffer, "    %s  %s", pPrtData->pym_dvs, pPrtData->bus_tck_knd_nm);
			//txtPrint(300+nGap-nMinus,  900, PRT_MODE_SMALL_ON, Buffer);
			ID_Print(nSvrKind, nPart, PTRG_ID3_ALL_ITM2, Buffer);

			///< 4-1. 출발일
			//txtPrint(210+nGap-nMinus,  800 + (160 * 0), PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_dt);
			ID_Print(nSvrKind, nPart, PTRG_ID5_DEPR_DT, pPrtData->atl_depr_dt);
			
			///< 4-2. 출발시간
			if(pPrtData->sati_use_yn[0] == 'N')
			{	/// 비좌석제이면, 출발시간, 좌석번호 출력안함.
				;
			}
			else
			{
				///< 4-2. 출발시각
				//txtPrint(210+nGap-nMinus,  800 + (160 * 1), PRT_MODE_VERTI_EXPAND, pPrtData->atl_depr_time);
				ID_Print(nSvrKind, nPart, PTRG_ID5_DEPR_TIME, pPrtData->atl_depr_time);

				///< 4-3. 좌석
				//txtPrint(210+nGap-nMinus,  800 + (160 * 2), PRT_MODE_VERTI_EXPAND, pPrtData->sats_no);
				ID_Print(nSvrKind, nPart, PTRG_ID5_SATS_NO, pPrtData->sats_no);
			}			

			///< 5-1. 바코드 정보
			sprintf(Buffer, "%s-%s-%s-%s-%s", pPrtData->pub_dt, pPrtData->pub_shct_trml_cd, pPrtData->pub_wnd_no, pPrtData->pub_sno, pPrtData->secu_code);
			//txtPrint(150 - (20 * 0)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
			ID_Print(nSvrKind, nPart, PTRG_ID6_BAR_DATA, Buffer);

			///< 5-2. 승인번호
			sprintf(Buffer, "승인번호: %s %s", pPrtData->card_aprv_no, pPrtData->pub_time);
			//txtPrint(150 - (20 * 1)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
			ID_Print(nSvrKind, nPart, PTRG_ID6_APRV_NO_PBTM, Buffer);
			
			///< 5-3. 승인금액
			if(pPrtData->n_pym_dvs == PYM_CD_CASH)
			{
				sprintf(Buffer, "승인금액: %s   %s", pPrtData->card_aprv_amt, pPrtData->card_no);
			}
			else
			{
				sprintf(Buffer, "승인금액: %s 구매자:%s", pPrtData->card_aprv_amt, pPrtData->card_no);
			}
			//txtPrint(150 - (20 * 2)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
			ID_Print(nSvrKind, nPart, PTRG_ID6_APRV_AMT_CARD, Buffer);

			///< 5-4. 가맹점번호
			// org code
			//txtPrint(150 - (20 * 3)+nGap,  800, PRT_MODE_SMALL_ON, "119-81-568654");
			sprintf(Buffer, "가맹점사업자번호:%s", pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);
			//txtPrint(150 - (20 * 3)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
			ID_Print(nSvrKind, nPart, PTRG_ID6_TRML_CORP_NO, Buffer);

			sprintf(Buffer, "*유효기간 당일 지정차에 한함");
			//txtPrint(150 - (20 * 4)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
			ID_Print(nSvrKind, nPart, PTRG_ID6_MSG1, Buffer);

//			sprintf(Buffer, "현금영수증 문의 ☏ 126-2");
			sprintf(Buffer, "현금영수증 문의 : 126-2");
			//txtPrint(150 - (20 * 5)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
			ID_Print(nSvrKind, nPart, PTRG_ID6_CSRC_TEL, Buffer);

			///< 5-5. 버스회사 명
			sprintf(Buffer, "버스회사:%s", pPrtData->bus_cacm_nm);
			//txtPrint(150 - (20 * 6)+nGap-nMinus,  800, PRT_MODE_SMALL_ON, Buffer);
			ID_Print(nSvrKind, nPart, PTRG_ID6_BUS_CACM, Buffer);
		}

		///< 3. 바코드 프린트
		{
			///< 3-1. 위치
			pPtrg = (PPTRG_INFO_T) INI_GetEnvTckPtrgInfo(nSvrKind, nPart, PTRG_ID6_BAR_CODE);
			if(pPtrg != (PPTRG_INFO_T) NULL)
			{
				//Print("\x1b\x57%04d%04d", 80+nGap, 40);
				Print("\x1b\x57%04d%04d", pPtrg->nX, pPtrg->nY);
			}

			///< 3-2. barcode height ('h')						*
			SendData((BYTE *)"\x1D\x68\x3C", 3);
			///< 3-3. barcode width ('w')
			SendData((BYTE *)"\x1D\x77\x04", 3);	// 4배확대
			///< 3-4. barcode type ('k')
			SendData((BYTE *)"\x1D\x6B\x07", 3);
			///< 3-5. barcode data
			sprintf(Buffer, "i%s%s%s%s%s", 
				pPrtData->pub_dt, 
				pPrtData->pub_shct_trml_cd, 
				pPrtData->pub_wnd_no, 
				pPrtData->pub_sno, 
				pPrtData->secu_code);
			SendData((BYTE *)Buffer, strlen(Buffer) + 1);
		}

		BeginPrint(FALSE);
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	TR_LOG_OUT(" end !!!\n\n");

	return 0;
}
***/

int CPrtTicketHS::CbusTicketPrint(char *pData)
{
	int					i, k, nLen, nRet;
	char				Buffer[500];
	PTCK_PRINT_FMT_T	pPrtData;
	POPER_FILE_CONFIG_T pOperConfig;
	PKIOSK_INI_PTRG_T	pPtrgInf;
	PKIOSK_INI_ENV_T	pEnv;

	pPrtData = (PTCK_PRINT_FMT_T) pData;
	pOperConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();
	pEnv		= (PKIOSK_INI_ENV_T) GetEnvInfo();

	i = k = nLen = 0;

	LOG_OUT(" start !!!\n\n");
	m_bExpOnly = FALSE;

	try
	{
		PPTRG_INFO_T pPtrg = NULL;

		BeginPrint(TRUE);

		pPtrgInf = (PKIOSK_INI_PTRG_T) INI_GetEnvTckPtrg(SVR_DVS_CCBUS);
		
		for(i = 0; i < MAX_PTRG; i++)
		{
			for(k = 0; k < MAX_PTRG_ITEM; k++)
			{
				PPTRG_INFO_T pInfo;

				if( i == PTRG_TRML_PART )
				{	/// 회수용
					pInfo = &pPtrgInf->trmlPtrg[k];
				}
				else 
				{	/// 승객용
					pInfo = &pPtrgInf->custPtrg[k];
				}

				if(pInfo->nUse == 0)
					continue;

				LOG_OUT("\n정보 : nID = %d, %s", pInfo->nID, pInfo->szMsg);

				switch(pInfo->nID)
				{
				case PTRG_ID1_PSNO:			/// (1줄) 발권 시리얼번호
					sprintf(Buffer, "0000%s", pPrtData->bar_pub_sno);//kh200708
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;
				case PTRG_ID1_USRID:		/// (1줄) 사용자 ID
					sprintf(Buffer, "%s", GetEnvSvrUserNo(SVR_DVS_CCBUS));//kh200708
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;
				case PTRG_ID1_USRNM:		/// (1줄) 사용자 Name
					sprintf(Buffer, "(%s)", pPrtData->user_nm);//kh200708
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID1_WNDNO:		/// (1줄) (창구번호+무인기) kh200710
					sprintf(Buffer, "( %s 무인기 )", pPrtData->qr_pub_wnd_no);
 					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID1_MSG1:			/// (1줄) MSG_1 kh200708
				case PTRG_ID1_MSG2:			/// (1줄) MSG_2
				case PTRG_ID1_MSG3:			/// (1줄) MSG_3
				case PTRG_ID1_MSG4:			/// (1줄) MSG_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->szMsg);
					break;

				case PTRG_ID2_DEPR_KO:		/// (2줄) 출발지(한글)
					nLen = strlen(pPrtData->depr_trml_ko_nm);
					if(nLen > 0)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->depr_trml_ko_nm);
					}
					break;
				case PTRG_ID2_ARVL_KO:		/// (2줄) 도착지(한글)
					nLen = strlen(pPrtData->arvl_trml_ko_nm);
					if(nLen > 0)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->arvl_trml_ko_nm);
					}
					break;
				case PTRG_ID2_DEPR_KO_1:		/// (2줄) 출발지(한글) - 승객용
					nLen = strlen(pPrtData->depr_trml_ko_nm);
					if(nLen > 0)
					{
						MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pPrtData->depr_trml_ko_nm);
					}
					break;
				case PTRG_ID2_ARVL_KO_1:		/// (2줄) 도착지(한글) - 승객용
					nLen = strlen(pPrtData->arvl_trml_ko_nm);
					if(nLen > 0)
					{
						MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pPrtData->arvl_trml_ko_nm);
					}
					break;
				case PTRG_ID2_BUS_CLS:		/// (2줄) 버스등급
					{
						int m, nRow, nOffset;

						m = nRow = nOffset = 0;

						nLen = strlen(pPrtData->bus_cls_nm);
						nRow = nLen / 4;
						if(nRow > 0)
						{
							nOffset = 0;
							for(m = 0; m < nRow; m++)
							{
								::ZeroMemory(Buffer, sizeof(Buffer));
								::CopyMemory(Buffer, &pPrtData->bus_cls_nm[nOffset], 4);
								nOffset += 4;
								//txtPrint(390+nGap-(i*20)-nMinus, 700, PRT_MODE_NONE, Buffer);
								txtPrint(pInfo->nX - (m * 20), pInfo->nY, pInfo->nMode, pPrtData->bus_cls_nm);
							}
						}
						else
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->bus_cls_nm);
						}
					}
					break;
				case PTRG_ID2_DEPR_EN:		/// (2줄) 출발지(영문)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->depr_trml_eng_nm);
					break;
				case PTRG_ID2_ARVL_EN:		/// (2줄) 도착지(영문)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->arvl_trml_eng_nm);
					break;
				case PTRG_ID2_THRU_NM:		/// (2줄) 경유지
					nLen = strlen(pPrtData->thru_nm);
					if(nLen > 0)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->thru_nm);
					}
					break;
				case PTRG_ID2_DIST:			/// (2줄) 거리
// 					nLen = strlen(pPrtData->thru_nm);
// 					if(nLen > 0)
// 					{
// 						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->thru_nm);
// 					}
					break;
				case PTRG_ID2_DEPR_EN_1:	/// (2줄) 출발지(영문)
					{
						::ZeroMemory(Buffer, sizeof(Buffer));
						nLen = strlen(pPrtData->depr_trml_eng_nm);
						if(nLen > 20)
						{
							nLen = 20;					
						}
						::CopyMemory(Buffer, pPrtData->depr_trml_eng_nm, nLen);
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					}
					break;
				case PTRG_ID2_ARVL_EN_1:		/// (2줄) 도착지(영문)
					{
						::ZeroMemory(Buffer, sizeof(Buffer));
						nLen = strlen(pPrtData->arvl_trml_eng_nm);
						if(nLen > 20)
						{
							nLen = 20;					
						}
						::CopyMemory(Buffer, pPrtData->arvl_trml_eng_nm, nLen);
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					}
					break;

				case PTRG_ID2_ROT_NO:			/// (2줄) 도착지 - 노선번호
					{
						if( strlen(pPrtData->arvl_trml_rot_num) > 0 )
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->arvl_trml_rot_num);
						}
					}
					break;
				
				case PTRG_ID2_PYM_DVS:		/// (2줄) 간략결제수단 kh_200709
					if(pPrtData->n_pym_dvs == PYM_CD_CARD)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->shrt_pym_dvs);
					}
					else if(pPrtData->n_pym_dvs == PYM_CD_TPAY)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->shrt_pym_dvs);
					}
					else
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->shrt_pym_dvs);
					}
					break;

									case PTRG_ID2_ROT_NO_ARVL:			/// (2줄) 회수용 노선번호 + 도착지 kh_200717
					{
						if( strlen(pPrtData->arvl_trml_rot_num) > 0 )
						{
							sprintf(Buffer, "%s %s", pPrtData->arvl_trml_rot_num, pPrtData->arvl_trml_ko_nm);
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
						}
						else
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->arvl_trml_ko_nm);
						}
					}
					break;

				case PTRG_ID2_ROT_NO_ARVL_1:			/// (2줄) 승객용 노선번호 + 도착지 kh_200717
					{
						if( strlen(pPrtData->arvl_trml_rot_num) > 0 )
						{
							nLen = strlen(pPrtData->arvl_trml_ko_nm);
							if(nLen > 0)
							{
								sprintf(Buffer, "%s %s", pPrtData->arvl_trml_rot_num, pPrtData->arvl_trml_ko_nm);
								MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nMode, MAX_HANGUL_CHAR - 8, Buffer);
							}
							else
							{
								;
							}
						}
						else
						{
							MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nMode, MAX_HANGUL_CHAR - 8, pPrtData->arvl_trml_ko_nm);
						}
					}
					break;


				case PTRG_ID2_MSG1:			/// (2줄) MSG_1
				case PTRG_ID2_MSG2:			/// (2줄) MSG_2
				case PTRG_ID2_MSG3:			/// (2줄) MSG_3
				case PTRG_ID2_MSG4:			/// (2줄) MSG_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->szMsg);
					break;

					/////////
				case PTRG_ID3_FARE:			/// (3줄) 요금
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->tisu_amt);
					break;
				case PTRG_ID3_PYM_DVS:		/// (3줄) 결제수단
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->pym_dvs);
					break;
				case PTRG_ID3_TCK_KND:		/// (3줄) 승차권 종류
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->bus_tck_knd_nm);
					break;
				case PTRG_ID3_BUS_CLS:		/// (3줄) (버스등급) kh_200708
					sprintf(Buffer, "(%s)", pPrtData->bus_cls_nm);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID3_PYM_DVS_VERTI:/// (3줄) 결제수단 세로 kh_200708
					if(pPrtData->n_pym_dvs == PYM_CD_CARD)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, "카");
						txtPrint(pInfo->nX-65, pInfo->nY, pInfo->nMode, "드");
					}
					else if(pPrtData->n_pym_dvs == PYM_CD_TPAY)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, "페");
						txtPrint(pInfo->nX-65, pInfo->nY, pInfo->nMode, "이");
					}
					// 20210910 ADD
					/*
					else if(pPrtData->n_pym_dvs == PYM_CD_ETC)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, "복");
						txtPrint(pInfo->nX-65, pInfo->nY, pInfo->nMode, "합");
					}
					*/
					// 20210910 ~ADD
					else
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, "현");
						txtPrint(pInfo->nX-65, pInfo->nY, pInfo->nMode, "금");
					}
					break;

				case PTRG_ID3_ENMNPP_VERTI:
					switch(pPrtData->tisu_chnl_dvs_cd[0])
					{
					case 'W':
					case 'w':
						txtPrint(pInfo->nX-(48*0), pInfo->nY, pInfo->nMode, "인");
						txtPrint(pInfo->nX-(48*1), pInfo->nY, pInfo->nMode, "터");
						txtPrint(pInfo->nX-(48*2), pInfo->nY, pInfo->nMode, "넷");
						break;
					case 'M':
					case 'm':
						txtPrint(pInfo->nX-(48*0), pInfo->nY, pInfo->nMode, "모");
						txtPrint(pInfo->nX-(48*1), pInfo->nY, pInfo->nMode, "바");
						txtPrint(pInfo->nX-(48*2), pInfo->nY, pInfo->nMode, "일");
					}
					break;

				case PTRG_ID3_MSG1:			/// (3줄) 메시지_1
				case PTRG_ID3_MSG2:			/// (3줄) 메시지_2
				case PTRG_ID3_MSG3:			/// (3줄) 메시지_3
				case PTRG_ID3_MSG4:			/// (3줄) 메시지_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->szMsg);
					break;

					/////////
				case PTRG_ID5_DEPR_DT:		/// (5줄) 출발일자
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->atl_depr_dt);
					break;
				case PTRG_ID5_DEPR_TIME:	/// (5줄) 출발시간 /kh_200709

					if(GetEnvOperCorp() == KUMHO_OPER_CORP)
					{
						if (pPrtData->alcn_way_dvs_cd[0] == 'N') 
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, "<<선착순 탑승>>");
							txtPrint(pInfo->nX-30, pInfo->nY, pInfo->nMode, "미탑승분은 다음차 이용");
						}
						else
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->atl_depr_time);
						}
					}
					else
					{
						nRet = CConfigTkMem::GetInstance()->IsPrintFirstCome(pPrtData->alcn_way_dvs_cd, pPrtData->sati_use_yn, pPrtData->depr_time_prin_yn, pPrtData->sats_no_prin_yn); // 20221207 MOD
						if( nRet >= 0 )
						{
							if(nRet == 0)
							{
								///< 4-2. 출발시간
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->atl_depr_time);
							}
							else
							{
								///< 4-2. 출발시간
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, "선착순");
							}
						}
					}

					//txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->atl_depr_time); //선착순 승차 표시위함 
					break;

				case PTRG_ID5_SATS_NO:		/// (5줄) 좌석번호 /kh_200709 
					if(GetEnvOperCorp() == KUMHO_OPER_CORP)
					{
 						if(pPrtData->sati_use_yn[0] == 'N')
 						{	/// 비좌석제이면, 출발시간, 좌석번호 출력안함.
							;
						}
						else
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->sats_no);
						}
					}
					else
					{
						nRet = CConfigTkMem::GetInstance()->IsPrintSatsNo(pPrtData->alcn_way_dvs_cd, pPrtData->sats_no_prin_yn);
						if(nRet > 0)
						{
							///< 4-3. 좌석
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->sats_no);
						}
					}
					break;
				case PTRG_ID5_RDHM_VAL:		/// (5줄) 승차홈
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->rdhm_val);
					break;
				case PTRG_ID5_BUS_CACM:		/// (5줄) 버스회사
					if(GetEnvOperCorp() == KUMHO_OPER_CORP)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->bus_cacm_nm);
					}
					else
					{
						if(pPrtData->alcn_way_dvs_cd[0] == 'D')
						{	/// 배차
							///< 4-5. 버스회사
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->bus_cacm_nm);
						}
						else
						{	/// 비배차
							if(pEnv->tTckOpt.nalcn_time_cacm_yn[0] == 'Y')
							{	/// 비배차 : 시간, 운수사 출력유무..
								///< 4-5. 버스회사
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->bus_cacm_nm);
							}
						}
					}
					break;
				case PTRG_ID5_FIRST_MSG1:	/// (5줄) 선착순 메시지 1
					break;
				case PTRG_ID5_FIRST_MSG2:	/// (5줄) 선착순 메시지 2
					break;
				case PTRG_ID5_MSG1:			/// (5줄) 메시지 1
				case PTRG_ID5_MSG2:			/// (5줄) 메시지 2
				case PTRG_ID5_MSG3:			/// (5줄) 메시지 3
				case PTRG_ID5_MSG4:			/// (5줄) 메시지 4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->szMsg);
					break;

					///////// 
				case PTRG_ID6_APRV_NO:			/// (6줄) 승인번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->card_aprv_no);
					break;
				case PTRG_ID6_TLE_APRV_NO:		/// (6줄) 타이틀:승인번호
					//sprintf(Buffer, "승인번호: %s", pPrtData->card_aprv_no);	
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->card_aprv_no);	
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID6_APRV_AMT:			/// (6줄) 승인금액
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->card_aprv_amt);
					break;
				case PTRG_ID6_TLE_APRV_AMT:			/// (6줄) 승인금액
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->card_aprv_amt);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID6_CARD_NO:				/// (6줄) 카드번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->card_no);
					break;
				case PTRG_ID6_TLE_CARD_NO:			/// (6줄) 카드번호
					if(pPrtData->n_pym_dvs == PYM_CD_CASH)
					{
						sprintf(Buffer, "%s", pPrtData->card_no);
					}
					else
					{
						sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->card_no);
					}
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;
				case PTRG_ID6_PUB_TIME:				/// (6줄) 발권시간(시:분:초)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->pub_time);
					break;
				case PTRG_ID6_DEPR_NM:				/// (6줄) 출발지
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->depr_trml_ko_nm);
					break;
				case PTRG_ID6_BAR_DATA:				/// (6줄) 바코드 정보 (pub_dt-pub_shct_trml_cd-pub_wnd_no-pub_sno-secu_code)
					sprintf(Buffer, "%s-%s-%s-%s-%s", pPrtData->bar_pub_dt, pPrtData->bar_pub_shct_trml_cd, pPrtData->bar_pub_wnd_no, pPrtData->bar_pub_sno, pPrtData->bar_secu_code);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;
				case PTRG_ID6_TLE_TRML_CORP_NO:		/// (6줄) 가맹점 사업자번호
					//sprintf(Buffer, "가맹점사업자번호:%s", pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;
				case PTRG_ID6_BUS_CACM:			/// (6줄) 버스회사
					if(GetEnvOperCorp() == KUMHO_OPER_CORP)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->bus_cacm_nm);
					}
					else
					{
						if(pPrtData->alcn_way_dvs_cd[0] == 'D')
						{	/// 배차
							///< 4-5. 버스회사
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->bus_cacm_nm);
						}
						else
						{	/// 비배차
							if(pEnv->tTckOpt.nalcn_time_cacm_yn[0] == 'Y')
							{	/// 비배차 : 시간, 운수사 출력유무..
								///< 4-5. 버스회사
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->bus_cacm_nm);
							}
						}
					}

					//txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->bus_cacm_nm);
					break;
				case PTRG_ID6_TLE_BUS_CACM:		/// (6줄) 버스회사:값
					if(GetEnvOperCorp() == KUMHO_OPER_CORP)
					{
						sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->bus_cacm_nm);
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					}
					else
					{
						if(pPrtData->alcn_way_dvs_cd[0] == 'D')
						{	/// 배차
							///< 4-5. 버스회사
							sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->bus_cacm_nm);
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
						}
						else
						{	/// 비배차
							if(pEnv->tTckOpt.nalcn_time_cacm_yn[0] == 'Y')
							{	/// 비배차 : 시간, 운수사 출력유무..
								///< 4-5. 버스회사
								sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->bus_cacm_nm);
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
							}
						}
					}
					break;

				case PTRG_ID6_TRML_TEL_NO:		/// (6줄) 버스터미널 연락처  kh_200708
					sprintf(Buffer, "%s:%s",pInfo->szMsg, pPrtData->depr_trml_tel_nm);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID6_TRML_TEL_NO2:		/// (6줄) (버스터미널 연락처) kh_200708
					sprintf(Buffer, "(%s)",pInfo->szMsg, pPrtData->depr_trml_tel_nm);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID6_SHRT_PUB_TIME:	/// (6줄) 발권시간(시:분) kh_200709
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->shrt_pub_time);
					break;

				case PTRG_ID6_TLE_DEPR_NM:				/// (6줄) 타이틀 : 출발지 kh_200710
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->depr_trml_ko_nm);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID6_TLE2_APRV_AMT:			/// (6줄) 타이틀 승인금액(:없는ver) kh_200713
					sprintf(Buffer, "%s %s", pInfo->szMsg, pPrtData->card_aprv_amt);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID6_TRML_CORP_NO:		/// (6줄) 사업자번호 kh_200713 
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);
					break;
				case PTRG_ID6_TLE2_CARD_NO:			/// (6줄) 카드번호(:없는ver) kh_200713
					if(pPrtData->n_pym_dvs == PYM_CD_CASH)
					{
						sprintf(Buffer, "%s", pPrtData->card_no);
					}
					else
					{
						sprintf(Buffer, "%s %s", pInfo->szMsg, pPrtData->card_no);
					}
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID6_TLE2_APRV_NO:		/// (6줄) 타이틀 승인번호(:없는ver) kh_200713	
					sprintf(Buffer, "%s %s", pInfo->szMsg, pPrtData->card_aprv_no);	
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);

				case PTRG_ID6_CACM_BIZ_NO:		/// (6줄) 버스회사 사업자번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode,pPrtData->bus_cacm_biz_no);
					break;


				case PTRG_ID6_MSG1:			/// (6줄) 메시지1
				case PTRG_ID6_MSG2:			/// (6줄) 메시지1
				case PTRG_ID6_MSG3:			/// (6줄) 메시지1
				case PTRG_ID6_MSG4:			/// (6줄) 메시지1
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->szMsg);
					break;

				case PTRG_ID6_BAR_CODE:		/// (6줄) 바코드 
					{
						///< 3-1. 위치
						Print("\x1b\x57%04d%04d", pInfo->nX, pInfo->nY);
						///< 3-2. barcode height ('h')						*
						SendData((BYTE *)"\x1D\x68\x3C", 3);
						///< 3-3. barcode width ('w')
						SendData((BYTE *)"\x1D\x77\x04", 3);	// 4배확대
						///< 3-4. barcode type ('k')
						SendData((BYTE *)"\x1D\x6B\x07", 3);
						///< 3-5. barcode data
						sprintf(Buffer, "i%s%s%s%s%s", 
								pPrtData->bar_pub_dt, 
								pPrtData->bar_pub_shct_trml_cd, 
								pPrtData->bar_pub_wnd_no, 
								pPrtData->bar_pub_sno, 
								pPrtData->bar_secu_code);
						SendData((BYTE *)Buffer, strlen(Buffer) + 1);
					}
					break;
				}
			}
		}

		BeginPrint(FALSE);
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	TR_LOG_OUT(" end !!!");
	TR_LOG_OUT(" ");
	TR_LOG_OUT(" ");

	return 0;
}

int CPrtTicketHS::KobusTicketPrint(char *pData)
{
	int					i, k, nLen;
	char				Buffer[500];
	PTCK_PRINT_FMT_T	pPrtData;
	POPER_FILE_CONFIG_T pOperConfig;
	PKIOSK_INI_PTRG_T	pPtrgInf;

	pPrtData = (PTCK_PRINT_FMT_T) pData;
	pOperConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	i = k = nLen = 0;

	TR_LOG_OUT(" start !!!\n\n");
	m_bExpOnly = FALSE;

	try
	{
		PPTRG_INFO_T pPtrg = NULL;

		BeginPrint(TRUE);

		pPtrgInf = (PKIOSK_INI_PTRG_T) INI_GetEnvTckPtrg(SVR_DVS_KOBUS);

		for(i = 0; i < MAX_PTRG; i++)
		{
			for(k = 0; k < MAX_PTRG_ITEM; k++)
			{
				PPTRG_INFO_T pInfo;

				if( i == PTRG_TRML_PART )
				{	/// 회수용
					pInfo = &pPtrgInf->trmlPtrg[k];
				}
				else 
				{	/// 승객용
					pInfo = &pPtrgInf->custPtrg[k];
				}

				if(pInfo->nUse == 0)
					continue;

				LOG_OUT("\n정보 : nID = %d, %s", pInfo->nID, pInfo->szMsg);

				switch(pInfo->nID)
				{
				case PTRG_ID1_PSNO:			/// (1줄) 발권 시리얼번호
					sprintf(Buffer, "0000%s", pPrtData->bar_pub_sno);//kh200708
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;
				case PTRG_ID1_USRID:		/// (1줄) 사용자 ID
					sprintf(Buffer, "%s", GetEnvSvrUserNo(SVR_DVS_CCBUS));//kh200708
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;
				case PTRG_ID1_USRNM:		/// (1줄) 사용자 Name
					sprintf(Buffer, "(%s)", pPrtData->user_nm);//kh200708
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID1_WNDNO:		/// (1줄) (창구번호+무인기) kh_200710
					sprintf(Buffer, "( %s 무인기 )", pPrtData->qr_pub_wnd_no);
 					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID1_MSG1:			/// (1줄) MSG_1 kh_200708
				case PTRG_ID1_MSG2:			/// (1줄) MSG_2
				case PTRG_ID1_MSG3:			/// (1줄) MSG_3
				case PTRG_ID1_MSG4:			/// (1줄) MSG_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->szMsg);
					break;

				case PTRG_ID2_DEPR_KO:		/// (2줄) 출발지(한글)
					nLen = strlen(pPrtData->depr_trml_ko_nm);
					if(nLen > 0)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->depr_trml_ko_nm);
					}
					break;
				case PTRG_ID2_ARVL_KO:		/// (2줄) 도착지(한글)
					nLen = strlen(pPrtData->arvl_trml_ko_nm);
					if(nLen > 0)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->arvl_trml_ko_nm);
					}
					break;
				case PTRG_ID2_DEPR_KO_1:		/// (2줄) 출발지(한글) - 회수용
					nLen = strlen(pPrtData->depr_trml_ko_nm);
					if(nLen > 0)
					{
						MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pPrtData->depr_trml_ko_nm);
					}
					break;
				case PTRG_ID2_ARVL_KO_1:		/// (2줄) 도착지(한글) - 승객용
					nLen = strlen(pPrtData->arvl_trml_ko_nm);
					if(nLen > 0)
					{
						MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pPrtData->arvl_trml_ko_nm);
					}
					break;
				case PTRG_ID2_BUS_CLS:		/// (2줄) 버스등급
					{
						int m, nRow, nOffset;

						m = nRow = nOffset = 0;

						nLen = strlen(pPrtData->bus_cls_nm);
						nRow = nLen / 4;
						if(nRow > 0)
						{
							nOffset = 0;
							for(m = 0; m < nRow; m++)
							{
								::ZeroMemory(Buffer, sizeof(Buffer));
								::CopyMemory(Buffer, &pPrtData->bus_cls_nm[nOffset], 4);
								nOffset += 4;
								//txtPrint(390+nGap-(i*20)-nMinus, 700, PRT_MODE_NONE, Buffer);
								txtPrint(pInfo->nX - (m * 20), pInfo->nY, pInfo->nMode, Buffer);
							}
						}
						else
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->bus_cls_nm);
						}
					}
					break;
				case PTRG_ID2_DEPR_EN:		/// (2줄) 출발지(영문)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->depr_trml_eng_nm);
					break;
				case PTRG_ID2_ARVL_EN:		/// (2줄) 도착지(영문)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->arvl_trml_eng_nm);
					break;
				case PTRG_ID2_THRU_NM:		/// (2줄) 경유지
// 					nLen = strlen(pPrtData->thru_nm);
// 					if(nLen > 0)
// 					{
// 						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->thru_nm);
// 					}
					break;
				case PTRG_ID2_DIST:			/// (2줄) 거리
					// 					nLen = strlen(pPrtData->thru_nm);
					// 					if(nLen > 0)
					// 					{
					// 						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->thru_nm);
					// 					}
					break;
				case PTRG_ID2_DEPR_EN_1:	/// (2줄) 출발지(영문)
					{
						::ZeroMemory(Buffer, sizeof(Buffer));
						nLen = strlen(pPrtData->depr_trml_eng_nm);
						if(nLen > 20)
						{
							nLen = 20;					
						}
						::CopyMemory(Buffer, pPrtData->depr_trml_eng_nm, nLen);
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					}
					break;
				case PTRG_ID2_ARVL_EN_1:		/// (2줄) 도착지(영문)
					{
						::ZeroMemory(Buffer, sizeof(Buffer));
						nLen = strlen(pPrtData->arvl_trml_eng_nm);
						if(nLen > 20)
						{
							nLen = 20;					
						}
						::CopyMemory(Buffer, pPrtData->arvl_trml_eng_nm, nLen);
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					}
					break;

				case PTRG_ID2_PYM_DVS:		/// (2줄) 간략결제수단 kh_200709
					if(pPrtData->n_pym_dvs == PYM_CD_CARD)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->shrt_pym_dvs);
					}
					else if(pPrtData->n_pym_dvs == PYM_CD_TPAY)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->shrt_pym_dvs);
					}
					else
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->shrt_pym_dvs);
					}
					break;

				case PTRG_ID2_ROT_NO_ARVL:			/// (2줄) 회수용 노선번호 + 도착지 kh_200717
					{
						if( strlen(pPrtData->arvl_trml_rot_num) > 0 )
						{
							sprintf(Buffer, "%s %s", pPrtData->arvl_trml_rot_num, pPrtData->arvl_trml_ko_nm);
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
						}
						else
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->arvl_trml_ko_nm);
						}
					}
					break;

				case PTRG_ID2_ROT_NO_ARVL_1:			/// (2줄) 승객용 노선번호 + 도착지 kh_200717
					{
						if( strlen(pPrtData->arvl_trml_rot_num) > 0 )
						{
							nLen = strlen(pPrtData->arvl_trml_ko_nm);
							if(nLen > 0)
							{
								sprintf(Buffer, "%s %s", pPrtData->arvl_trml_rot_num, pPrtData->arvl_trml_ko_nm);
								MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nMode, MAX_HANGUL_CHAR - 8, Buffer);
							}
							else
							{
								;
							}
						}
						else
						{
							MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nMode, MAX_HANGUL_CHAR - 8, pPrtData->arvl_trml_ko_nm);
						}
					}
					break;

				case PTRG_ID2_MSG1:			/// (2줄) MSG_1
				case PTRG_ID2_MSG2:			/// (2줄) MSG_2
				case PTRG_ID2_MSG3:			/// (2줄) MSG_3
				case PTRG_ID2_MSG4:			/// (2줄) MSG_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->szMsg);
					break;

				case PTRG_ID3_FARE:			/// (3줄) 요금
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->tisu_amt);
					break;
				case PTRG_ID3_PYM_DVS:		/// (3줄) 결제수단
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->pym_dvs);
					break;
				case PTRG_ID3_TCK_KND:		/// (3줄) 승차권 종류
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->bus_tck_knd_nm);
					break;
				case PTRG_ID3_BUS_CLS:		/// (3줄) (버스등급) kh_200708
					sprintf(Buffer, "(%s)", pPrtData->bus_cls_nm);//
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);//
					break;
				//case PTRG_ID3_DRAWBOX:		/// (3줄) 전주용 회수용 드로우박스 kh200713
				//	break;

				case PTRG_ID3_MSG1:			/// (3줄) 메시지_1
				case PTRG_ID3_MSG2:			/// (3줄) 메시지_2
				case PTRG_ID3_MSG3:			/// (3줄) 메시지_3
				case PTRG_ID3_MSG4:			/// (3줄) 메시지_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->szMsg);
					break;

				case PTRG_ID5_DEPR_DT:		/// (5줄) 출발일자
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->atl_depr_dt);
					break;
				case PTRG_ID5_DEPR_TIME:	/// (5줄) 출발시간
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->atl_depr_time);
					break;
				case PTRG_ID5_SATS_NO:		/// (5줄) 좌석번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->sats_no);
					break;
				case PTRG_ID5_RDHM_VAL:		/// (5줄) 승차홈
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->rdhm_val);
					break;
				case PTRG_ID5_BUS_CACM:		/// (5줄) 버스회사
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->bus_cacm_nm);
					break;
				case PTRG_ID5_FIRST_MSG1:	/// (5줄) 선착순 메시지 1
					break;
				case PTRG_ID5_FIRST_MSG2:	/// (5줄) 선착순 메시지 2
					break;
				case PTRG_ID5_MSG1:			/// (5줄) 메시지 1
				case PTRG_ID5_MSG2:			/// (5줄) 메시지 2
				case PTRG_ID5_MSG3:			/// (5줄) 메시지 3
				case PTRG_ID5_MSG4:			/// (5줄) 메시지 4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->szMsg);
					break;

				case PTRG_ID6_APRV_NO:			/// (6줄) 승인번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->card_aprv_no);
					break;
				case PTRG_ID6_TLE_APRV_NO:		/// (6줄) 타이틀:승인번호
					//sprintf(Buffer, "승인번호: %s", pPrtData->card_aprv_no);	
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->card_aprv_no);	
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID6_APRV_AMT:			/// (6줄) 승인금액
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->card_aprv_amt);
					break;
				case PTRG_ID6_TLE_APRV_AMT:			/// (6줄) 타이틀 : 승인금액
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->card_aprv_amt);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID6_CARD_NO:				/// (6줄) 카드번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->card_no);
					break;

				case PTRG_ID6_TLE_CARD_NO:			/// (6줄) 카드번호
					if(pPrtData->n_pym_dvs == PYM_CD_CASH)
					{
						sprintf(Buffer, "%s", pPrtData->card_no);
					}
					else
					{
						sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->card_no);
					}
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;
				case PTRG_ID6_PUB_TIME:				/// (6줄) 발권시간(시:분:초)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->pub_time);
					break;
				case PTRG_ID6_DEPR_NM:				/// (6줄) 출발지
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->depr_trml_ko_nm);
					break;
// 				case PTRG_ID6_BAR_DATA:				/// (6줄) 바코드 정보 (pub_dt-pub_shct_trml_cd-pub_wnd_no-pub_sno-secu_code)
// 					sprintf(Buffer, "%s-%s-%s-%s-%s", pPrtData->pub_dt, pPrtData->pub_shct_trml_cd, pPrtData->pub_wnd_no, pPrtData->pub_sno, pPrtData->secu_code);
// 					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
// 					break;
				case PTRG_ID6_QR_DATA:				/// (6줄) QR코드 정보 
					sprintf(Buffer, "%s-%s-%s-%s", pPrtData->qr_pub_dt, pPrtData->qr_pub_trml_cd, pPrtData->qr_pub_wnd_no, pPrtData->qr_pub_sno);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;
				case PTRG_ID6_TLE_TRML_CORP_NO:		/// (6줄) 가맹점 사업자번호
					//sprintf(Buffer, "가맹점사업자번호:%s", pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;
				case PTRG_ID6_BUS_CACM:			/// (6줄) 버스회사
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->bus_cacm_nm);
					break;
				case PTRG_ID6_TLE_BUS_CACM:		/// (6줄) 버스회사:값
					//sprintf(Buffer, "버스회사:%s", pPrtData->bus_cacm_nm);
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->bus_cacm_nm);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID6_SHRT_PUB_TIME:	/// (6줄) 발권시간(시:분)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->shrt_pub_time);
					break;

				case PTRG_ID6_DEPR_ARVL_SATS:	///전주전용 출발코드도착코드자리번호 kh_200710
					sprintf(Buffer, "%s%s%s", pPrtData->qr_depr_trml_no, pPrtData->qr_arvl_trml_no, pPrtData->qr_sats_no);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode,Buffer);
					break;

				case PTRG_ID6_TLE_DEPR_NM:				/// (6줄) 타이틀 : 출발지 kh_200710
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->depr_trml_ko_nm);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID6_TLE2_APRV_AMT:			/// (6줄) 타이틀 승인금액(:없는ver) kh_200713
					sprintf(Buffer, "%s %s", pInfo->szMsg, pPrtData->card_aprv_amt);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID6_TRML_CORP_NO:		/// (6줄) 사업자번호 kh_200713 
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pPrtData->depr_trml_biz_no);

				case PTRG_ID6_TLE2_CARD_NO:			/// (6줄) 카드번호(:없는ver) kh_200713
					if(pPrtData->n_pym_dvs == PYM_CD_CASH)
					{
						sprintf(Buffer, "%s", pPrtData->card_no);
					}
					else
					{
						sprintf(Buffer, "%s %s", pInfo->szMsg, pPrtData->card_no);
					}
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);
					break;

				case PTRG_ID6_TLE2_APRV_NO:		/// (6줄) 타이틀 승인번호(:없는ver) kh_200713	
					sprintf(Buffer, "%s %s", pInfo->szMsg, pPrtData->card_aprv_no);	
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, Buffer);

				case PTRG_ID6_CACM_BIZ_NO:		/// (6줄) 버스회사 사업자번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode,pPrtData->bus_cacm_biz_no);
					break;

				case PTRG_ID6_MSG1:			/// (6줄) 메시지1
				case PTRG_ID6_MSG2:			/// (6줄) 메시지1
				case PTRG_ID6_MSG3:			/// (6줄) 메시지1
				case PTRG_ID6_MSG4:			/// (6줄) 메시지1
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->szMsg);
					break;

// 				case PTRG_ID6_BAR_CODE:		/// (6줄) 바코드 
// 					{
// 						///< 3-1. 위치
// 						Print("\x1b\x57%04d%04d", pInfo->nX, pInfo->nY);
// 						///< 3-2. barcode height ('h')						*
// 						SendData((BYTE *)"\x1D\x68\x3C", 3);
// 						///< 3-3. barcode width ('w')
// 						SendData((BYTE *)"\x1D\x77\x04", 3);	// 4배확대
// 						///< 3-4. barcode type ('k')
// 						SendData((BYTE *)"\x1D\x6B\x07", 3);
// 						///< 3-5. barcode data
// 						sprintf(Buffer, "i%s%s%s%s%s", 
// 							pPrtData->pub_dt, 
// 							pPrtData->pub_shct_trml_cd, 
// 							pPrtData->pub_wnd_no, 
// 							pPrtData->pub_sno, 
// 							pPrtData->secu_code);
// 						SendData((BYTE *)Buffer, strlen(Buffer) + 1);
// 					}
// 					break;
				case PTRG_ID6_QR_CODE:		/// (6줄) QRCode
					{
						///< 3-1. 페이지모드 인자영역 지정 
//						Print("\x1b\x57%04d%04d", 170, 560);
						Print("\x1b\x57%04d%04d", pInfo->nX, pInfo->nY);
						///< 3-2. QR 바코드 
						sprintf(Buffer, "%s%s%s%s%s%s%s", 
							pPrtData->qr_pub_dt, 
							pPrtData->qr_pub_trml_cd, 
							pPrtData->qr_pub_wnd_no, 
							pPrtData->qr_pub_sno, 
							pPrtData->qr_depr_trml_no, 
							pPrtData->qr_arvl_trml_no, 
							pPrtData->qr_sats_no);
						Print("\x1A\x42\x02\x19\x03%s", Buffer);
					}
					break;
				}
			}
		}

		BeginPrint(FALSE);
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	TR_LOG_OUT(" end !!!");
	TR_LOG_OUT(" ");
	TR_LOG_OUT(" ");

	return 0;
}
