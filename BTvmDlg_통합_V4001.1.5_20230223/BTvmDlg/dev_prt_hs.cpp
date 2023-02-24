// 
// 
// dev_prt_hs.cpp : 영수증프린터 (화성)
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "dev_prt_hs.h"
#include "dev_tr_main.h"
#include "dev_tr_mem.h"
#include "dev_tr_kobus_mem.h"
#include "dev_tr_tmexp_mem.h"
#include "oper_config.h"
#include "oper_kos_file.h"
#include "event_if.h"
#include "dev_prt_main.h"
#include "File_Env_ini.h"
#include "cmn_util.h"
#include "data_main.h"

//----------------------------------------------------------------------------------------------------------------------

#define LOG_OUT(fmt, ...)		{ CPrinterHS::m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ CPrinterHS::m_clsLog.HexaDump(x, y, z); }

static PDEV_CFG_T		pEnv = NULL;		// 20230206 ADD
static PDEV_CFG_T		pEnvPrtTck = NULL;	// 20230206 ADD

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CPrinterHS
 * @details		생성자
 */
CPrinterHS::CPrinterHS()
{
	m_hThread = NULL;
	m_hAccMutex = NULL;
	m_bConnected = FALSE;
}

/**
 * @brief		~CPrinterHS
 * @details		소멸자
 */
CPrinterHS::~CPrinterHS()
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
void CPrinterHS::LOG_INIT(void)
{
	m_clsLog.SetData(30, "\\Log\\Prt\\Hs");
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
int CPrinterHS::Locking(void)
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
int CPrinterHS::UnLocking(void)
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
int CPrinterHS::SendData(BYTE *pData, int nDataLen)
{
	return m_clsComm.SendData(pData, nDataLen);
}

/**
 * @brief		GetPacket
 * @details		get data
 * @param		BYTE *retBuf	receive data
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrinterHS::GetPacket(BYTE *retBuf)
{
	static WORD	wPktLen;
	static int	nState, nRecv, nDataCount;
	BYTE byBCC;
	int	ch, i;

	while(1) 
	{ 
		ch = m_clsComm.ReadData();
		if(ch < 0) 
		{
			return ch;
		}

		ch = ch & 0xFF;

		switch(nState) 
		{
		case 0 :	// stx
			nState = nRecv = wPktLen = nDataCount = 0;
			if(ch == CHAR_STX)
			{
				wPktLen = nDataCount = 0;
				nState++;
			}
			break;

		case 1:		// address_high
			retBuf[nRecv++] = ch;
			wPktLen = (ch << 8);
			nState++;
			break;

		case 2 :	// address_low
			retBuf[nRecv++] = ch;
			wPktLen |= (ch & 0xFF);
			nState++;
			break;

		case 3 :	// command
			retBuf[nRecv++] = ch;
			nState++;
			break;

		case 4 :	// data
			retBuf[nRecv++] = ch;
			nState++;
			break;

		case 5 :	// bcc
			retBuf[nRecv++] = ch;
			nState++;
			break;

		case 6 :	// etx
			if(ch == CHAR_ETX)
			{
				goto got_packet;
			}
			break;
		}
	}

got_packet:
	
	nState = wPktLen = nDataCount = 0;

	byBCC = 0;
	for(i = 1; i < nRecv; i++)
	{
		byBCC ^= retBuf[i];
	}

	if(byBCC != retBuf[nRecv - 1]) 
	{
		//LOG_OUT("[%s:%d] checksum ERROR[%02X, %02X]..\n", __FUNCTION__, __LINE__, byBcc, retBuf[nRecv - 1]);
		return -1;
	}
	
	//LOG_HEXA("GetPacket", __FUNCTION__, __LINE__, retBuf, nRecv);

	return nRecv;
}

/**
 * @brief		InitPrinter
 * @details		Initialize Printer
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrinterHS::InitPrinter(void)
{
	int nRet;

	//Locking();
	{
		BYTE pSendData[] = "\x1B\x40\x1B\x32\x1B\x21\x00\x1D\x50\x01\x1B\x52\x0D";

		nRet = SendData(pSendData, 13);
	}
	//UnLocking();
	return 0;
}

/**
 * @brief		SetQRCode
 * @details		Set QR Code
 * @param		char *pData			QR Data
 * @return		항상 = 0
 */
int CPrinterHS::SetQRCode(char *pData)
{
	int nRet, nOffset;

	nRet = nOffset = 0;
	//Locking();
	{
 		BYTE Buffer[256];
 
		/// 1 : 정렬 : 가운데
		nRet = SendData((BYTE *)"\x1B\x61\x01", 3);

		/// packet
 		CopyMemory(&Buffer[nOffset], "\x1A\x42\x02", 3);
		nOffset += 3;

		/// data length
		Buffer[nOffset++] = strlen(pData);

		/// QR Version(=QR 코드 크기)
		//Buffer[nOffset++] = 0x05;	// 20220926 DEL // 20221220 DEL
		Buffer[nOffset++] = 0x03;	// 20220926 MOD // [마산고속] 감열지 승차권 QR코드 사이즈 작게 수정 요청사항

		/// data
		nOffset += sprintf((char *)&Buffer[nOffset], "%s", pData);
		nRet = SendData(Buffer, nOffset);

		/// 1 : 정렬 : 왼쪽
		nRet = SendData((BYTE *)"\x1B\x61\x00", 3);

	}
	//UnLocking();
	return 0;
}

/**
 * @brief		SetCharPrtingMode
 * @details		Set character printing mode
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrinterHS::SetCharPrtingMode(BYTE byVal)
{
	int nRet;

	//Locking();
	{
		BYTE Buffer[4] = "\x1B\x21\x00";

		Buffer[2] = byVal;

		nRet = SendData(Buffer, 3);
	}
	//UnLocking();
	return 0;
}

/**
 * @brief		SetPrintPosition
 * @details		Set absolute print position
 * @param		WORD	wPos		position 
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrinterHS::SetPrintPosition(WORD wPos)
{
	int nRet;

	//Locking();
	{
		BYTE Buffer[10] = "\x1B\x24\x00";

		::CopyMemory(&Buffer[2], &wPos, 2);

		nRet = SendData(Buffer, 4);
	}
	//UnLocking();
	return 0;
}

/**
 * @brief		LineFeed
 * @details		Print and feed n lines
 * @param		int nVal			n lines
 * @return		항상 = 0
 */
int CPrinterHS::SetImageData(WORD x, WORD y, BYTE *pImgData, int nLen)
{
	int nRet, nOffset;

	nRet = nOffset = 0;

	//Locking();
	{
		BYTE Buffer[256] = "\x1D\x76\x30";

		nOffset = 3;
		/// m - 0(normal), 1(Horizon 2x), 2(Vertical 2x), 3(Horizon, Vertical 2x)
		Buffer[nOffset++] = 0x02;
		
		/// x
		::CopyMemory(&Buffer[nOffset], &x, 2);
		nOffset += 2;

		/// y
		::CopyMemory(&Buffer[nOffset], &y, 2);
		nOffset += 2;

		// Image Data
		::CopyMemory(&Buffer[nOffset], pImgData, nLen);
		nOffset += nLen;

		nRet = SendData(Buffer, nOffset);
	}
	//UnLocking();
	return 0;
}

/**
 * @brief		LineFeed
 * @details		Print and feed n lines
 * @param		int nVal			n lines
 * @return		항상 = 0
 */
int CPrinterHS::LineFeed(int nVal)
{
	int nRet;

	//Locking();
	{
		BYTE Buffer[4] = "\x1B\x64\x00";

		Buffer[2] = nVal & 0xFF;

		nRet = SendData(Buffer, 3);
	}
	//UnLocking();
	return 0;
}

/**
 * @brief		GetStatus
 * @details		프린터 상태 명령 전송
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrinterHS::GetStatus(void)
{
	int nRet, ch, nError;
	DWORD dwTick;
	static int prev_ch = -1;

	//LOG_OUT("start... ");
	//PurgeComm(m_clsComm.m_hCommHandle, PURGE_TXABORT | PURGE_RXCLEAR);
	//Sleep(300);

	int nloop = 0; // 20230206 ADD

	nRet = m_clsComm.SendData((BYTE *)"\x1D\x61\x01", 3);
	//LOG_OUT("SendData... nRet = %d", nRet);

	nError = FALSE;
	dwTick = ::GetTickCount();
	while( Util_CheckExpire(dwTick) < 500 )
	{
		// 20230206 ADD~
		//if ( (nloop == 0) || (nloop >= 20) )
		//{
		//	// Comm 읽기전 또는 20회(약 5초) Read 후에 프린터 강제 리셋 => // 20230209 TEST결과_장애 신호가 Clear ??? 되어서 늦게 또는 안올라오는 경우도 있음 
		//	nRet = m_clsComm.SendData((BYTE *)"\x10\x05\x03", 3);	
		//	Sleep(300);												
		//	nloop = 0;
		//}
		//nloop++;
		// 20230206 ~ADD

		ch = m_clsComm.ReadData();
		if(ch < 0) 
		{
			continue;
		}

		ch = ch & 0xFF;
		
		// 20230206 ADD~
		//if ( (nloop == 1) || (nloop >= 20) )
		//	LOG_OUT("nloop = %d, prev_ch = %02X, ch = %02X", nloop, prev_ch & 0xFF, ch & 0xFF);
		// 20230206 ~ADD

		if(prev_ch != ch)
		{
			prev_ch = ch;
			LOG_OUT("Recv = %02X", ch & 0xFF);
		}

		if(GetConfigTicketPapaer() == PAPER_ROLL)									// 20230206 DEL // 20230217 MOD
		//pEnv = (PDEV_CFG_T) GetEnvPrtReceiptInfo();								// 20230206 ADD // 20230217 DEL
		//if(pEnv->nUse == 1) // 영수증(감열지) 프린터 사용 옵션이 'USE = 1'인 경우		// 20230206 ADD // 20230217 DEL
		{	/// 감열지 사용인 경우
			//LOG_OUT("감열지 사용...");
			//
			SetCheckEventCode(EC_PRT_ROLL_COMM_ERR, FALSE);

			if(ch & 0x01)
			{//용지 없음
				SetCheckEventCode(EC_PRT_ROLL_NO_PAPER, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_ROLL_NO_PAPER, FALSE);
			}
			//
			if(ch & 0x02)
			{
				//TRACE("프린트 헤드 업 \n");
				SetCheckEventCode(EC_PRT_ROLL_HEAD_UP, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_ROLL_HEAD_UP, FALSE);
			}
		
			if(ch & 0x04)
			{
				//TRACE("용지 잼 \n");
				SetCheckEventCode(EC_PRT_ROLL_PAPER_JAM, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_ROLL_PAPER_JAM, FALSE);
			}

			//
			if(ch & 0x08)
			{
				//TRACE("용지 Near END \n");
				SetCheckEventCode(EC_PRT_ROLL_PAPER_NEAR_END, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_ROLL_PAPER_NEAR_END, FALSE);
			}

			//
			if(ch & 0x10)
			{
				TRACE("프린트 또는 Feeding 중 \n");
			}
			//
			if(ch & 0x20)
			{
				//TRACE("컷터 에러(잼)4 \n");
				SetCheckEventCode(EC_PRT_ROLL_CUTTER_ERR, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_ROLL_CUTTER_ERR, FALSE);
			}

			//
			if(ch & 0x80)
			{
				TRACE("보조센서에 용지 있음 \n");
			}
		}
		//else // 20230206 DEL
		pEnvPrtTck = (PDEV_CFG_T) GetEnvPrtTicketInfo();							// 20230206 ADD
		if(pEnvPrtTck->nUse == 1) // 승차권(종이) 프린터 사용 옵션이 'USE = 1'인 경우	// 20230206 ADD
		{	/// 감열지 미사용인 경우
			//LOG_OUT("감열지 미사용...");
			//
			SetCheckEventCode(EC_PRT_COMM_ERR, FALSE);

			if(ch & 0x01)
			{
				SetCheckEventCode(EC_PRT_NO_PAPER, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_NO_PAPER, FALSE);
			}
			//
			if(ch & 0x02)
			{
				//TRACE("프린트 헤드 업 \n");
				SetCheckEventCode(EC_PRT_HEAD_UP, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_HEAD_UP, FALSE);
			}
		
			if(ch & 0x04)
			{
				//TRACE("용지 잼 \n");
				SetCheckEventCode(EC_PRT_PAPER_JAM, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_PAPER_JAM, FALSE);
			}

			//
			if(ch & 0x08)
			{
				//TRACE("용지 Near END \n");
				SetCheckEventCode(EC_PRT_PAPER_NEAR_END, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_PAPER_NEAR_END, FALSE);
			}

			//
			if(ch & 0x10)
			{
				TRACE("프린트 또는 Feeding 중 \n");
			}
			//
			if(ch & 0x20)
			{
				//TRACE("컷터 에러(잼)4 \n");
				SetCheckEventCode(EC_PRT_CUTTER_ERR, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_CUTTER_ERR, FALSE);
			}

			//
			if(ch & 0x80)
			{
				TRACE("보조센서에 용지 있음 \n");
			}
		}

		//LOG_OUT("end... ");
		if(nError == TRUE)
		{
			return -2;
		}

		return 1;
	}

	//if(GetConfigTicketPapaer() == PAPER_ROLL)									// 20230206 DEL
	pEnv = (PDEV_CFG_T) GetEnvPrtReceiptInfo();									// 20230206 ADD
	//LOG_OUT("pEnv->nUse = %d", pEnv->nUse);
	//if(pEnv->nUse == 1) // 영수증(감열지) 프린터 사용 옵션이 'USE = 1'인 경우		// 20230206 ADD // 20230215 DEL
	if( (pEnv->nUse == 1) && ( ((prev_ch & 0xFF) == false) && ((ch & 0xFF) == false) ) ) // 영수증(감열지) 프린터 사용 옵션이 'USE = 1'인 경우		// 20230206 ADD // 20230215 MOD
	{ // 감열지_영수증프린터 상태 체크 수정(for 한전금 무인기 Hs타입) // 20230215
		LOG_OUT(" >>>>> prev_ch = %02X, ch = %02X", prev_ch & 0xFF, ch & 0xFF);
		SetCheckEventCode(EC_PRT_ROLL_COMM_ERR, TRUE);
	}
	//else // 20230206 DEL
	pEnvPrtTck = (PDEV_CFG_T) GetEnvPrtTicketInfo();							// 20230206 ADD
	//LOG_OUT("pEnvPrtTck->nUse = %d", pEnvPrtTck->nUse);
	if(pEnvPrtTck->nUse == 1) // 승차권(종이) 프린터 사용 옵션이 'USE = 1'인 경우	// 20230206 ADD
	{
		SetCheckEventCode(EC_PRT_COMM_ERR, TRUE);
	}

	return -1;
}

/**
 * @brief		SetFontSize
 * @details		프린터 폰트크기 설정
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrinterHS::SetFontSize(int nValue)
{
	m_szTxBuf[0] = 0x1D;
	m_szTxBuf[1] = 0x21;
	m_szTxBuf[2] = nValue & 0xFF;

	m_clsComm.SendData(m_szTxBuf, 3);

	return 0;
}

int CPrinterHS::SetBold(int nValue)
{
	m_szTxBuf[0] = 0x1B;
	m_szTxBuf[1] = 0x45;
	m_szTxBuf[2] = nValue & 0xFF;

	m_clsComm.SendData(m_szTxBuf, 3);

	return 0;
}

void CPrinterHS::PaperFullCut(void)
{		
	/// Feeding : 150*0.125 = 18mm 
	m_clsComm.SendData((BYTE*)"\x1B\x4A\x96", 3);

	/// Full Cut
	m_clsComm.SendData((BYTE*)"\x1D\x56\x00", 3);	
}

int CPrinterHS::Printf(const char *fmt, ...)
{
	int		len, nRet, off, cnt;
	va_list ap;
	char	Buffer[1024 + 10];

	{
		::ZeroMemory(Buffer, sizeof(Buffer));

		off = 0;
		va_start(ap, fmt);
		len = _vscprintf(fmt, ap) + 1;
		cnt = vsprintf_s(&Buffer[off], len, fmt, ap);
		va_end(ap);

		len = (int) strlen(Buffer);
		if( len > 0 ) 
		{
			nRet = m_clsComm.SendData((BYTE *)Buffer, len);
			Sleep(10);
		}
	}

	return 0;
}

/**
 * @brief		Printf2
 * @details		프린트 데이타 출력
 * @param		const char *fmt			문자 포맷
 * @return		항상 = 0
 */
int CPrinterHS::Printf2(int nCharMode, const char *fmt, ...)
{
	int		len, nRet, off, cnt, mode;
	va_list ap;
	char	Buffer[1024 + 10];

	mode = 0;

	if(nCharMode & 0x08)
	{
		SetBold(1);
	}

	if(nCharMode & 0x10)
	{	/// 세로 확대
		mode |= 0x01;
	}
	if(nCharMode & 0x20)
	{	/// 가로 확대
		mode |= 0x10;
	}

	SetFontSize(mode & 0xFF);
	{
		::ZeroMemory(Buffer, sizeof(Buffer));

		off = 0;
		va_start(ap, fmt);
		len = _vscprintf(fmt, ap) + 1;
		cnt = vsprintf_s(&Buffer[off], len, fmt, ap);
		va_end(ap);

		len = (int) strlen(Buffer);
		if( len > 0 ) 
		{
			nRet = m_clsComm.SendData((BYTE *)Buffer, len);
			Sleep(10);
		}

		if(nCharMode & 0x08)
		{
			SetBold(0);
		}
	}
	SetFontSize(0);

	LOG_OUT("Mode(%x), pStr(%s)..", nCharMode, fmt);

	return 0;
}

BYTE *CPrinterHS::SetSpaceData(int nSpaceLen)
{
	memset(m_szTemp, 0x20, sizeof(m_szTemp));
	m_szTemp[nSpaceLen] = 0;
	return m_szTemp;
}

//#define FIRST_GAP	2
void CPrinterHS::SetLeftMargin(int n)
{
	if(n != 0)
	{
		Printf2(0x00, "%s", SetSpaceData(n));
	}
}

/**
 * @brief		StartProcess
 * @details		Start
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrinterHS::StartProcess(int nCommIdx)
{
	int		nRet;
	//DWORD	dwThreadID;

	LOG_INIT();

	m_bConnected = FALSE;

	LOG_OUT("start !!!");

	EndProcess();

	nRet = m_clsComm.Open(nCommIdx, CBR_19200, 8, NOPARITY, ONESTOPBIT);
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

	return 0;
}

/**
 * @brief		EndProcess
 * @details		Start
 * @param		int nCommIdx		COM
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrinterHS::EndProcess(void)
{
	LOG_OUT("end !!!");
	
	m_bConnected = FALSE;

	m_clsComm.Close();

	if(m_hAccMutex != NULL)
	{
		CloseHandle(m_hAccMutex);
		m_hAccMutex = NULL;
	}

	return 0;
}


/**
 * @brief		RefundTicket
 * @details		환불영수증 출력
 * @param		int nSvrKind		서버종류
 * @return		항상 = 0
 */
void CPrinterHS::Print_Refund(void)
{
	SYSTEMTIME st;
	prt_refund_t prtInfo;
	POPER_FILE_CONFIG_T pConfig;
	PTCK_PRT_OPT_T pTckOpt;

	LOG_OUT("start !!!");

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	pTckOpt = (PTCK_PRT_OPT_T) GetEnvTckPrtInfo();
	FIRST_GAP = pTckOpt->n_receipt_left_gap;

	::ZeroMemory(&prtInfo, sizeof(prt_refund_t));

	GetLocalTime(&st);

	/****
	if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_CCBUS )
	{	/// 시외버스
		CCancRyTkMem* pCancTr;
		PFMT_BARCODE_T pBarCode;

		pCancTr = CCancRyTkMem::GetInstance();
		pBarCode = (PFMT_BARCODE_T) pCancTr->szTicketData;

		sprintf(prtInfo.bus_nm, "시외버스");

		/// 가맹점 이름
		sprintf(prtInfo.bizr_nm, pConfig->ccTrmlInfo_t.sz_prn_trml_nm);
		/// 가맹점 사업자번호
		sprintf(prtInfo.bizr_no, pConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);

		/// 결제수단
		switch( pCancTr->tRespTckNo.pyn_mns_dvs_cd[0] )
		{
		case PYM_CD_CASH :
			sprintf(prtInfo.pyn_dvs_nm, "현금");
			break;
		case PYM_CD_CSRC :
			sprintf(prtInfo.pyn_dvs_nm, "현금영수증");
			break;
		case PYM_CD_CARD :
			sprintf(prtInfo.pyn_dvs_nm, "신용카드");
			break;
		case PYM_CD_TPAY :
			sprintf(prtInfo.pyn_dvs_nm, "페이");
		}

		/// 승차권정보
		sprintf(prtInfo.ticket_info, "%.*s-%.*s-%.*s-%.*s-%.*s ", 
			sizeof(pBarCode->pub_dt),			pBarCode->pub_dt, 
			sizeof(pBarCode->pub_shct_trml_cd), pBarCode->pub_shct_trml_cd, 
			sizeof(pBarCode->pub_wnd_no),		pBarCode->pub_wnd_no, 
			sizeof(pBarCode->pub_sno),			pBarCode->pub_sno, 
			sizeof(pBarCode->secu_code),		pBarCode->secu_code);

		/// 발권금액
		sprintf(prtInfo.tisu_amt, "%d", *(int *)pCancTr->tRespTckNo.tisu_amt);

		/// 환불율
		sprintf(prtInfo.cmrt	, "%d", pCancTr->n_commission_rate);

		/// 환불금액
		sprintf(prtInfo.ry_amt	, "%d", pCancTr->n_chg_money);

		/// 출발일
		sprintf(prtInfo.depr_dt	, "%.*s-%.*s-%.*s", 
			4, &pCancTr->tRespTckNo.depr_dt[0], 
			2, &pCancTr->tRespTckNo.depr_dt[4], 
			2, &pCancTr->tRespTckNo.depr_dt[6]);

		/// 출발시간
		sprintf(prtInfo.depr_time	, "%.*s:%.*s", 
			2, &pCancTr->tRespTckNo.depr_time[0], 
			2, &pCancTr->tRespTckNo.depr_time[2]);

		/// 출발지
		FindTerminalName(LANG_KOR, pCancTr->tRespTckNo.depr_trml_cd, prtInfo.depr_nm);

		/// 도착지
		FindTerminalName(LANG_KOR, pCancTr->tRespTckNo.arvl_trml_cd, prtInfo.arvl_nm);

		/// 버스등급
		FindBusClsName(SVR_DVS_CCBUS, pCancTr->tRespTckNo.bus_cls_cd, prtInfo.bus_cls_nm);

		/// 좌석번호
		sprintf(prtInfo.sat_no	, "%d", *(int *)pCancTr->tRespTckNo.sats_no);
	}
	else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_KOBUS )
	{	/// 코버스 고속
		CCancRyTkKobusMem* pCancTr;
		PFMT_QRCODE_T pQRCode;

		pCancTr = CCancRyTkKobusMem::GetInstance();

		pQRCode = (PFMT_QRCODE_T) pCancTr->tBase.szTicketData;

		sprintf(prtInfo.bus_nm, "고속버스");

		/// 가맹점 이름
		sprintf(prtInfo.bizr_nm, pConfig->koTrmlInfo_t.sz_prn_trml_nm);
		/// 가맹점 사업자번호
		sprintf(prtInfo.bizr_no, pConfig->koTrmlInfo_t.sz_prn_trml_corp_no);

		/// 결제수단
		switch( pCancTr->tBase.ui_pym_dvs_cd[0] )
		{
		case 1 :
			sprintf(prtInfo.pyn_dvs_nm, "현금");
			break;
		case 2 :
			sprintf(prtInfo.pyn_dvs_nm, "신용카드");
			break;
		case 'd'://스마일페이
		case 'e'://티머니페이
		case 'f'://비즈페이
		case 'g'://페이코
			sprintf(prtInfo.pyn_dvs_nm, "페이");
			
		}

		/// 승차권정보
		sprintf(prtInfo.ticket_info, "%.*s-%.*s-%.*s-%.*s", 
			sizeof(pQRCode->pub_dt),			pQRCode->pub_dt, 
			sizeof(pQRCode->pub_shct_trml_cd),  pQRCode->pub_shct_trml_cd, 
			sizeof(pQRCode->pub_wnd_no),		pQRCode->pub_wnd_no, 
			sizeof(pQRCode->pub_sno),			pQRCode->pub_sno);

		/// 발권금액
		sprintf(prtInfo.tisu_amt, "%d", *(int *)pCancTr->tRespInq.tissu_fee);

		/// 환불율
		sprintf(prtInfo.cmrt	, "%d", pCancTr->tBase.n_commission_rate);

		/// 환불금액
		sprintf(prtInfo.ry_amt	, "%d", pCancTr->tBase.n_chg_money);

		/// 출발일
		sprintf(prtInfo.depr_dt	, "%.*s-%.*s-%.*s", 
			4, &pCancTr->tRespList.depr_dt[0], 
			2, &pCancTr->tRespList.depr_dt[4], 
			2, &pCancTr->tRespList.depr_dt[6]);

		/// 출발시간
		sprintf(prtInfo.depr_time	, "%.*s:%.*s", 
			2, &pCancTr->tRespList.depr_time[0], 
			2, &pCancTr->tRespList.depr_time[2]);

		/// 출발지
		Find_KobusTrmlName(LANG_KOR, pCancTr->tRespList.depr_trml_no, prtInfo.depr_nm);

		/// 도착지
		Find_KobusTrmlName(LANG_KOR, pCancTr->tRespList.arvl_trml_no, prtInfo.arvl_nm);

		/// 버스등급
		FindBusClsName(SVR_DVS_KOBUS, pCancTr->tRespList.bus_cls_cd, prtInfo.bus_cls_nm);

		/// 좌석번호
		sprintf(prtInfo.sat_no	, "%s", pCancTr->tRespList.sats_no);
	}
	else 
	{	/// 티머니고속
		CCancRyTkTmExpMem* pCancTr;
		PFMT_QRCODE_T pQRCode;

		pCancTr = CCancRyTkTmExpMem::GetInstance();

		pQRCode = (PFMT_QRCODE_T) pCancTr->tBase.szTicketData;

		sprintf(prtInfo.bus_nm, "고속버스");

		/// 가맹점 이름
		sprintf(prtInfo.bizr_nm, pConfig->ezTrmlInfo_t.sz_prn_trml_nm);
		/// 가맹점 사업자번호
		sprintf(prtInfo.bizr_no, pConfig->ezTrmlInfo_t.sz_prn_trml_corp_no);

		/// 결제수단
		switch( pCancTr->tBase.ui_pym_dvs_cd[0] )
		{
		case 1 :
			sprintf(prtInfo.pyn_dvs_nm, "현금");
			break;
		case 2 :
			sprintf(prtInfo.pyn_dvs_nm, "신용카드");
			break;
		case 'd'://스마일페이
		case 'e'://티머니페이
		case 'f'://비즈페이
		case 'g'://페이코
			sprintf(prtInfo.pyn_dvs_nm, "페이");

		}

		/// 승차권정보
		sprintf(prtInfo.ticket_info, "%.*s-%.*s-%.*s-%.*s", 
			sizeof(pQRCode->pub_dt),			pQRCode->pub_dt, 
			sizeof(pQRCode->pub_shct_trml_cd),  pQRCode->pub_shct_trml_cd, 
			sizeof(pQRCode->pub_wnd_no),		pQRCode->pub_wnd_no, 
			sizeof(pQRCode->pub_sno),			pQRCode->pub_sno);

		/// 발권금액
		sprintf(prtInfo.tisu_amt, "%d", *(int *)pCancTr->tRespInq.tissu_fee);

		/// 환불율
		sprintf(prtInfo.cmrt	, "%d", pCancTr->tBase.n_commission_rate);

		/// 환불금액
		sprintf(prtInfo.ry_amt	, "%d", pCancTr->tBase.n_chg_money);

		/// 출발일
		sprintf(prtInfo.depr_dt	, "%.*s-%.*s-%.*s", 
			4, &pCancTr->tRespInq.depr_dt[0], 
			2, &pCancTr->tRespInq.depr_dt[4], 
			2, &pCancTr->tRespInq.depr_dt[6]);

		/// 출발시간
		sprintf(prtInfo.depr_time	, "%.*s:%.*s", 
			2, &pCancTr->tRespInq.depr_time[0], 
			2, &pCancTr->tRespInq.depr_time[2]);

		/// 출발지
		Find_KobusTrmlName(LANG_KOR, pCancTr->tRespInq.depr_trml_no, prtInfo.depr_nm);

		/// 도착지
		Find_KobusTrmlName(LANG_KOR, pCancTr->tRespInq.arvl_trml_no, prtInfo.arvl_nm);

		/// 버스등급
		FindBusClsName(SVR_DVS_KOBUS, pCancTr->tRespInq.bus_cls_cd, prtInfo.bus_cls_nm);

		/// 좌석번호
		sprintf(prtInfo.sat_no	, "%s", pCancTr->tRespInq.sats_no);
	}
	***/
	Printer_MakeRefundTicketData((char *)&prtInfo);


	if(1)
	{
		SetLeftMargin(FIRST_GAP);	Printf2(0x11, "	     %s\n", prtInfo.bus_nm);
		SetLeftMargin(FIRST_GAP);	Printf2(0x11, "	   환불 확인증\n");
		SetLeftMargin(FIRST_GAP);	Printf("\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "가맹점 : %s\n", prtInfo.bizr_nm);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "가맹점 사업자번호: %s\n", prtInfo.bizr_no);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "출력일시 : %04d-%02d-%02d %02d:%02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "결제수단   : %s\n",		prtInfo.pyn_dvs_nm);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "승차권정보 : %s\n",		prtInfo.ticket_info);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "발권 금액  : %s 원\n",		prtInfo.tisu_amt);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "수수료율   : %s%% \n",	prtInfo.cmrt);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "환불 금액  : \n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x11, "            %s",			prtInfo.ry_amt);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 원\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "출 발 일 : %s \n", prtInfo.depr_dt);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "출발시간 : %s \n", prtInfo.depr_time);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "출 발 지 : %s \n", prtInfo.depr_nm);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "도 착 지 : %s \n", prtInfo.arvl_nm);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "버스등급 : %s \n", prtInfo.bus_cls_nm);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "좌석번호 : %s \n", prtInfo.sat_no);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "총 매 수 : 1매 \n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		// 20221129 ADD~
		SetLeftMargin(FIRST_GAP);	Printf2(0x01, "   ※ 수수료 발생 시 카드 취소 최대 7일 소요 \n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "    결제일자, 취소일자, 카드종류 등에 따라 \n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "  카드 취소는 최대 7일 정도 소요될 수 있습니다.\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "   자세한 사항은 카드사로 문의하시기 바랍니다. \n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		// 20221129 ~ADD
		PaperFullCut();
	}
}

/**
 * @brief		Print_Account2
 * @details		마감내역 출력
 * @param		None
 * @return		항상 = 0
 */
void CPrinterHS::Print_Account2(void)
{
	SYSTEMTIME		st;
	char			retBuf[200];
	int				i, k;
	int				nSum[10] = {0, };
	char			szCount[100], szValue[100];
	PFILE_ACCUM_N1010_T	pAccum;
	char			*pTrmlCode;
	PDM_COIN_T		pCoin;
	PABILL_T		pCDU;
	PDM_BILL_T		pBillBox;
	PKIOSK_INI_ENV_T pEnv;
	char			*pWndNo;
	PTCK_PRT_OPT_T pTckOpt;

 	pAccum = (PFILE_ACCUM_N1010_T) GetAccumulateData();

	pCoin		= &pAccum->Curr.Coin;
	pBillBox	= &pAccum->Curr.BillBox;
	pCDU		= &pAccum->Curr.BillDispenser.Casst;

	GetLocalTime(&st);

	::ZeroMemory(retBuf, sizeof(retBuf));
	::ZeroMemory(szCount, sizeof(szCount));
	::ZeroMemory(szValue, sizeof(szValue));

	pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();
	pTckOpt = (PTCK_PRT_OPT_T) GetEnvTckPrtInfo();
	FIRST_GAP = pTckOpt->n_receipt_left_gap;

	pTrmlCode = NULL;
	pWndNo = NULL;

	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x11, "	마감 내역 명세서\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");

	if( pEnv->tCcInfo.nUse > 0 )
	{
		pTrmlCode = GetTrmlCode(SVR_DVS_CCBUS);
		FindTerminalName(LANG_KOR, pTrmlCode, retBuf);
		pWndNo = GetTrmlWndNo(SVR_DVS_CCBUS);
	}
	else if( pEnv->tKoInfo.nUse > 0 )
	{
		pTrmlCode = GetTrmlCode(SVR_DVS_KOBUS);
		Find_KobusTrmlName(LANG_KOR, pTrmlCode, retBuf);
		pWndNo = GetTrmlWndNo(SVR_DVS_KOBUS);
	}
	else
	{
		/// 2021.03.25 add code
		pTrmlCode = GetTrmlCode(SVR_DVS_TMEXP);
		Find_TmExpTrmlName(LANG_KOR, pTrmlCode, retBuf);
		pWndNo = GetTrmlWndNo(SVR_DVS_TMEXP);
		/// ~2021.03.25 add code
	}

	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "터 미 널 : %s\n", retBuf);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "창구번호 : %s\n", pWndNo);
	//	Printf("설정일시 : 2019-10-15 17:50:55\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "출력일시 : %04d-%02d-%02d %02d:%02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");

	SetLeftMargin(FIRST_GAP);	Printf2(0x01, "%s", "[ 1. 출금부 시재 ]\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "   단위          수량                     금액\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	Util_AmountComma(pCoin->n100, szCount); Util_AmountComma(COIN_100_SUM(pCoin->n100), szValue); 
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "    100원         %4d               %9d\n", pCoin->n100, COIN_100_SUM(pCoin->n100));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "    500원         %4d               %9d\n", pCoin->n500, COIN_500_SUM(pCoin->n500));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "  1,000원         %4d               %9d\n", pCDU->n1k, BILL_1K_SUM(pCDU->n1k));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 10,000원         %4d               %9d\n", pCDU->n10k, BILL_10K_SUM(pCDU->n10k));
	nSum[0] = COIN_SUM(0, 0, pAccum->Curr.Coin.n100, pAccum->Curr.Coin.n500) + BILL_SUM(pCDU->n1k, 0, pCDU->n10k, 0);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 합    계                            %9d\n",  nSum[0]);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x01, "%s", "[ 2. 입금부 시재 ]\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "   단위          수량                     금액\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "  1,000원         %4d               %9d\n", pBillBox->n1k, BILL_1K_SUM(pBillBox->n1k));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "  5,000원         %4d               %9d\n", pBillBox->n5k, BILL_5K_SUM(pBillBox->n5k));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 10,000원         %4d               %9d\n", pBillBox->n10k, BILL_10K_SUM(pBillBox->n10k));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 50,000원         %4d               %9d\n", pBillBox->n50k, BILL_50K_SUM(pBillBox->n50k));
	nSum[1] = BILL_SUM(pBillBox->n1k, pBillBox->n5k, pBillBox->n10k, pBillBox->n50k);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 합    계                            %9d\n", nSum[1]);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x01, "%s", "[ 3. 보급내역 ]\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "   단위          수량                     금액\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "    100원         %4d               %9d\n", pAccum->Close.supplyCoin.n100, COIN_100_SUM(pAccum->Close.supplyCoin.n100));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "    500원         %4d               %9d\n", pAccum->Close.supplyCoin.n500, COIN_500_SUM(pAccum->Close.supplyCoin.n500));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "  1,000원         %4d               %9d\n", pAccum->Close.supplyBill.n1k,  BILL_1K_SUM(pAccum->Close.supplyBill.n1k));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 10,000원         %4d               %9d\n", pAccum->Close.supplyBill.n10k,  BILL_10K_SUM(pAccum->Close.supplyBill.n10k));
	nSum[2] = COIN_SUM(0, 0, pAccum->Close.supplyCoin.n100, pAccum->Close.supplyCoin.n500) + BILL_SUM(pAccum->Close.supplyBill.n1k, 0, pAccum->Close.supplyBill.n10k, 0);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 합    계                            %9d\n", nSum[2]);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

	{
		ACCOUNT_FIELD_T sumT1;
		ACCOUNT_FIELD_T sumT2;

		::ZeroMemory(&sumT1, sizeof(ACCOUNT_FIELD_T));
		for(i = 0; i < CCS_IDX_TCK_MAX; i++)
		{
			PDM_TICKET_WORK_T pCCS = &pAccum->Curr.ccsTicketWork[i];

			//for(k = 0; k < 2; k++)
			{
				k = IDX_ACC_CASH;

				sumT1.nCount  += pCCS->tPubTck[k].nCount;
				sumT1.dwMoney += pCCS->tPubTck[k].dwMoney;
			}
		}

		::ZeroMemory(&sumT2, sizeof(ACCOUNT_FIELD_T));
		for(i = 0; i < TMEXP_IDX_TCK_MAX; i++)
		{
			PDM_TICKET_WORK_T pExp = &pAccum->Curr.expTicketWork[i];

			//for(k = 0; k < 2; k++)
			{
				k = IDX_ACC_CASH;

				sumT2.nCount  += pExp->tPubTck[k].nCount;
				sumT2.dwMoney += pExp->tPubTck[k].dwMoney;
			}
		}

		nSum[3] = (int)sumT1.dwMoney + (int)sumT2.dwMoney;

		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x01, "%s", "[ 4. 발권내역 (현금) ]\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "   구분          수량                     금액\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 시외발권         %4d               %9d\n", sumT1.nCount, sumT1.dwMoney);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 고속발권         %4d               %9d\n", sumT2.nCount, sumT2.dwMoney);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 합    계         %4d               %9d\n", sumT1.nCount + sumT2.nCount, sumT1.dwMoney + sumT2.dwMoney);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	}

	{
		ACCOUNT_FIELD_T sumT1;
		ACCOUNT_FIELD_T sumT2;

		::ZeroMemory(&sumT1, sizeof(ACCOUNT_FIELD_T));
		for(i = 0; i < CCS_IDX_TCK_MAX; i++)
		{
			PDM_TICKET_WORK_T pCCS = &pAccum->Curr.ccsTicketWork[i];

			//for(k = 0; k < 2; k++)
			{
				k = IDX_ACC_CASH;

				sumT1.nCount  += pCCS->tRefund[k].nCount;
				sumT1.dwMoney += pCCS->tRefund[k].dwMoney;
			}
		}

		::ZeroMemory(&sumT2, sizeof(ACCOUNT_FIELD_T));
		for(i = 0; i < TMEXP_IDX_TCK_MAX; i++)
		{
			PDM_TICKET_WORK_T pExp = &pAccum->Curr.expTicketWork[i];

			//for(k = 0; k < 2; k++)
			{
				k = IDX_ACC_CASH;

				sumT2.nCount  += pExp->tRefund[k].nCount;
				sumT2.dwMoney += pExp->tRefund[k].dwMoney;
			}
		}
		nSum[4] = (int)sumT1.dwMoney + (int)sumT2.dwMoney;

		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x01, "%s", "[ 5. 환불내역 (현금) ]\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "   구분          수량                     금액\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 시외환불         %4d               %9d\n", sumT1.nCount, sumT1.dwMoney);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 고속환불         %4d               %9d\n", sumT2.nCount, sumT2.dwMoney);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 합    계         %4d               %9d\n", sumT1.nCount + sumT2.nCount, sumT1.dwMoney + sumT2.dwMoney);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x01, " 결과 (1 + 2 - 3 - 4 - 5)          %9d원\n", nSum[0] + nSum[1] - nSum[2] - nSum[3] - nSum[4]);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
	}

	PaperFullCut();
}

/**
 * @brief		Print_Account3
 * @details		시재 마감내역 출력 (CashCls 폴더에서 찾아서 출력한다)
 * @param		None
 * @return		항상 = 0
 */
void CPrinterHS::Print_Account3(char *pDate, char *pTime)
{
	SYSTEMTIME			st;
	SYSTEMTIME			stFile;
	char				retBuf[200];
	int					nRet, i, k;
	int					nSum[10] = {0, };
	char				szCount[100], szValue[100];
	CLOSE_TR_DATA_T		TrData;
	PKIOSK_INI_ENV_T	pEnv;
	char				*pWndNo;
	PTCK_PRT_OPT_T		pTckOpt;
	PCLOSE_TR_DATA_T	pClsTR;
	PFILE_ACCUM_N1010_T	pAccum;
	char				*pTrmlCode;

 	pAccum = (PFILE_ACCUM_N1010_T) GetAccumulateData();

	GetLocalTime(&st);

	::ZeroMemory(retBuf, sizeof(retBuf));
	::ZeroMemory(szCount, sizeof(szCount));
	::ZeroMemory(szValue, sizeof(szValue));

	pEnv		= (PKIOSK_INI_ENV_T) GetEnvInfo();
	pTckOpt		= (PTCK_PRT_OPT_T) GetEnvTckPrtInfo();
	FIRST_GAP	= pTckOpt->n_receipt_left_gap;

	pTrmlCode	= NULL;
	pWndNo		= NULL;

	::ZeroMemory(&TrData, sizeof(CLOSE_TR_DATA_T));
	pClsTR = (PCLOSE_TR_DATA_T) &TrData;

	nRet = SearchCashCloseData(pDate, pTime, (char *)pClsTR);
	if(nRet < 0)
	{
		TR_LOG_OUT("시재마감 프린트 데이타 찾기 실패...");
		return;
	}

	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x11, "	시재마감 내역 명세서\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");

	if( pEnv->tCcInfo.nUse > 0 )
	{
		pTrmlCode = GetTrmlCode(SVR_DVS_CCBUS);
		FindTerminalName(LANG_KOR, pTrmlCode, retBuf);
		pWndNo = GetTrmlWndNo(SVR_DVS_CCBUS);
	}
	else if( pEnv->tKoInfo.nUse > 0 )
	{
		pTrmlCode = GetTrmlCode(SVR_DVS_KOBUS);
		Find_KobusTrmlName(LANG_KOR, pTrmlCode, retBuf);
		pWndNo = GetTrmlWndNo(SVR_DVS_KOBUS);
	}
	else
	{
		/// 2021.03.25 add code
		pTrmlCode = GetTrmlCode(SVR_DVS_TMEXP);
		Find_TmExpTrmlName(LANG_KOR, pTrmlCode, retBuf);
		pWndNo = GetTrmlWndNo(SVR_DVS_TMEXP);
		/// ~2021.03.25 add code
	}

	stFile.wYear	= (WORD) Util_Ascii2Long(&pClsTR->szEndDateTm[0],  4);
	stFile.wMonth	= (WORD) Util_Ascii2Long(&pClsTR->szEndDateTm[4],  2);
	stFile.wDay		= (WORD) Util_Ascii2Long(&pClsTR->szEndDateTm[6],  2);
	stFile.wHour	= (WORD) Util_Ascii2Long(&pClsTR->szEndDateTm[8],  2);
	stFile.wMinute	= (WORD) Util_Ascii2Long(&pClsTR->szEndDateTm[10], 2);
	stFile.wSecond	= (WORD) Util_Ascii2Long(&pClsTR->szEndDateTm[12], 2);

	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "터 미 널 : %s\n", retBuf);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "창구번호 : %s\n", pClsTR->szWndNo);
	//	Printf("설정일시 : 2019-10-15 17:50:55\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "마감일시 : %04d-%02d-%02d %02d:%02d:%02d\n", stFile.wYear, stFile.wMonth, stFile.wDay, stFile.wHour, stFile.wMinute, stFile.wSecond);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "출력일시 : %04d-%02d-%02d %02d:%02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");

	SetLeftMargin(FIRST_GAP);	Printf2(0x01, "%s", "[ 1. 출금부 시재 ]\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "   단위          수량                     금액\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	Util_AmountComma(pClsTR->currInfo.w100, szCount); Util_AmountComma(COIN_100_SUM(pClsTR->currInfo.w100), szValue); 
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "    100원         %4d               %9d\n", pClsTR->currInfo.w100, COIN_100_SUM(pClsTR->currInfo.w100));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "    500원         %4d               %9d\n", pClsTR->currInfo.w500, COIN_500_SUM(pClsTR->currInfo.w500));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "  1,000원         %4d               %9d\n", pClsTR->currInfo.w1k, BILL_1K_SUM(pClsTR->currInfo.w1k));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 10,000원         %4d               %9d\n", pClsTR->currInfo.w10k, BILL_10K_SUM(pClsTR->currInfo.w10k));
	nSum[0] = COIN_SUM(0, 0, pClsTR->currInfo.w100, pClsTR->currInfo.w500) + BILL_SUM(pClsTR->currInfo.w1k, 0, pClsTR->currInfo.w10k, 0);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 합    계                            %9d\n",  nSum[0]);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x01, "%s", "[ 2. 입금부 시재 ]\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "   단위          수량                     금액\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "  1,000원         %4d               %9d\n", pClsTR->insInfo.w1k,  BILL_1K_SUM(pClsTR->insInfo.w1k));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "  5,000원         %4d               %9d\n", pClsTR->insInfo.w5k,  BILL_5K_SUM(pClsTR->insInfo.w5k));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 10,000원         %4d               %9d\n", pClsTR->insInfo.w10k, BILL_10K_SUM(pClsTR->insInfo.w10k));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 50,000원         %4d               %9d\n", pClsTR->insInfo.w50k, BILL_50K_SUM(pClsTR->insInfo.w50k));
	nSum[1] = BILL_SUM(pClsTR->insInfo.w1k, pClsTR->insInfo.w5k, pClsTR->insInfo.w10k, pClsTR->insInfo.w50k);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 합    계                            %9d\n", nSum[1]);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x01, "%s", "[ 3. 보급내역 ]\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "   단위          수량                     금액\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "    100원         %4d               %9d\n", pClsTR->supplyInfo.w100, COIN_100_SUM(pClsTR->supplyInfo.w100));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "    500원         %4d               %9d\n", pClsTR->supplyInfo.w500, COIN_500_SUM(pClsTR->supplyInfo.w500));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "  1,000원         %4d               %9d\n", pClsTR->supplyInfo.w1k , BILL_1K_SUM(pClsTR->supplyInfo.w1k));
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 10,000원         %4d               %9d\n", pClsTR->supplyInfo.w10k, BILL_10K_SUM(pClsTR->supplyInfo.w10k));
	nSum[2] = COIN_SUM(0, 0, pClsTR->supplyInfo.w100, pClsTR->supplyInfo.w500) + BILL_SUM(pClsTR->supplyInfo.w1k, 0, pClsTR->supplyInfo.w10k, 0);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 합    계                            %9d\n", nSum[2]);
	SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

	{
		ACCOUNT_FIELD_T sumT1;
		ACCOUNT_FIELD_T sumT2;

		::ZeroMemory(&sumT1, sizeof(ACCOUNT_FIELD_T));
		for(i = 0; i < CCS_IDX_TCK_MAX; i++)
		{
			PDM_TICKET_WORK_T pCCS = &pAccum->MnuCls.ccsTicketWork[i];

			//for(k = 0; k < 2; k++)
			{
				k = IDX_ACC_CASH;

				sumT1.nCount  += pCCS->tPubTck[k].nCount;
				sumT1.dwMoney += pCCS->tPubTck[k].dwMoney;
			}
		}

		::ZeroMemory(&sumT2, sizeof(ACCOUNT_FIELD_T));
		for(i = 0; i < TMEXP_IDX_TCK_MAX; i++)
		{
			PDM_TICKET_WORK_T pExp = &pAccum->MnuCls.expTicketWork[i];

			//for(k = 0; k < 2; k++)
			{
				k = IDX_ACC_CASH;

				sumT2.nCount  += pExp->tPubTck[k].nCount;
				sumT2.dwMoney += pExp->tPubTck[k].dwMoney;
			}
		}

		nSum[3] = (int)sumT1.dwMoney + (int)sumT2.dwMoney;

		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x01, "%s", "[ 4. 발권내역 (현금) ]\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "   구분          수량                     금액\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 시외발권         %4d               %9d\n", sumT1.nCount, sumT1.dwMoney);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 고속발권         %4d               %9d\n", sumT2.nCount, sumT2.dwMoney);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 합    계         %4d               %9d\n", sumT1.nCount + sumT2.nCount, sumT1.dwMoney + sumT2.dwMoney);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	}

	{
		ACCOUNT_FIELD_T sumT1;
		ACCOUNT_FIELD_T sumT2;

		::ZeroMemory(&sumT1, sizeof(ACCOUNT_FIELD_T));
		for(i = 0; i < CCS_IDX_TCK_MAX; i++)
		{
			PDM_TICKET_WORK_T pCCS = &pAccum->MnuCls.ccsTicketWork[i];

			//for(k = 0; k < 2; k++)
			{
				k = IDX_ACC_CASH;

				sumT1.nCount  += pCCS->tRefund[k].nCount;
				sumT1.dwMoney += pCCS->tRefund[k].dwMoney;
			}
		}

		::ZeroMemory(&sumT2, sizeof(ACCOUNT_FIELD_T));
		for(i = 0; i < TMEXP_IDX_TCK_MAX; i++)
		{
			PDM_TICKET_WORK_T pExp = &pAccum->MnuCls.expTicketWork[i];

			//for(k = 0; k < 2; k++)
			{
				k = IDX_ACC_CASH;

				sumT2.nCount  += pExp->tRefund[k].nCount;
				sumT2.dwMoney += pExp->tRefund[k].dwMoney;
			}
		}
		nSum[4] = (int)sumT1.dwMoney + (int)sumT2.dwMoney;

		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x01, "%s", "[ 5. 환불내역 (현금) ]\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "   구분          수량                     금액\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 시외환불         %4d               %9d\n", sumT1.nCount, sumT1.dwMoney);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 고속환불         %4d               %9d\n", sumT2.nCount, sumT2.dwMoney);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, " 합    계         %4d               %9d\n", sumT1.nCount + sumT2.nCount, sumT1.dwMoney + sumT2.dwMoney);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x01, " 결과 (1 + 2 - 3 - 4 - 5)          %9d원\n", nSum[0] + nSum[1] - nSum[2] - nSum[3] - nSum[4]);
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		SetLeftMargin(FIRST_GAP);	Printf2(0x00, "\n");
	}

	PaperFullCut();
}

int CPrinterHS::AlignBuffer(int nMax, char *pSrc, int nSrcLen, char *retBuf, int retLen)
{
	int nFront, nRear, nOffset;
	char spBuf[100];

	nFront = nRear = nOffset = 0;

	/// space buffer
	memset(spBuf, 0x20, sizeof(spBuf));
	spBuf[sizeof(spBuf) - 1] = 0;

	/// 
	memset(retBuf, 0, retLen);

	if( nMax < nSrcLen )
	{
		nSrcLen = nMax;
	}

	/// 앞 space
	nFront = (nMax - nSrcLen) / 2;

	/// 뒤 space/
	nRear = nFront;
	if( (nMax - nSrcLen) % 2 )
	{
		nRear++;	
	}

	/// 실제 data 저장
	/// 1. 앞 space buffer
	if(nFront > 0)
	{
		CopyMemory(&retBuf[nOffset], spBuf, nFront);
		nOffset += nFront;
	}

	/// 2. data 복사
	if(nSrcLen > 0)
	{
		CopyMemory(&retBuf[nOffset], pSrc, nSrcLen);
		nOffset += nSrcLen;
	}

	/// 3. 뒤 space buffer
	if(nRear > 0)
	{
		CopyMemory(&retBuf[nOffset], spBuf, nRear);
		nOffset += nRear;
	}

	return nOffset;
}

//SetLeftMargin(FIRST_GAP);

/**
 * @brief		Print_Ticket
 * @details		승차권 출력
 * @param		char *pData			승차권 정보
 * @return		항상 = 0
 */
void CPrinterHS::Print_Ticket(int nSvrKind, int nFunction, char *pData)
{
	int nRet, nMAX;
	char Buffer[100];
	char retBuf[100];
	PTCK_PRT_OPT_T pTckOpt;
	PTCK_PRINT_FMT_T pPrtData;
	POPER_FILE_CONFIG_T pOperConfig;
	PFILE_ACCUM_N1010_T pAcc;

	pPrtData = (PTCK_PRINT_FMT_T) pData;
	pOperConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();
	pAcc = GetAccumulateData();

	InitPrinter();

	pTckOpt = (PTCK_PRT_OPT_T) GetEnvTckPrtInfo();

	FIRST_GAP = pTckOpt->n_receipt_left_gap;

	LOG_OUT("FIRST_GAP = %d", FIRST_GAP);

	///
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x10, "━━━━━━━━━━━━━━━━━━━━━━━\r\n");
	}

	///
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x00, "%s", SetSpaceData(8));
		Printf2(0x38, "고속버스 승차권");
		Printf2(0x00, "%s\r\n", SetSpaceData(8));
	}

	///
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x00, "%s", SetSpaceData(10));
		Printf2(0x38, "Boarding Pass");
		Printf2(0x00, "%s\r\n", SetSpaceData(10));
	}

	///
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x10, "━━━━━━━━━━━━━━━━━━━━━━━\r\n");
	}

	///
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x00, "%s출발일시\r\n", SetSpaceData(38));
	}

	///	[정보] 출발일시
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x00, "%s%s %s \r\n\r\n", SetSpaceData(27), pPrtData->atl_depr_dt, pPrtData->atl_depr_time);
	}
	
	/// [정보] QR Code
	{
		SetLeftMargin(FIRST_GAP);
		sprintf(Buffer, "%s%s%s%s%s%s%s", 
				pPrtData->qr_pub_dt, 
				pPrtData->qr_pub_trml_cd, 
				pPrtData->qr_pub_wnd_no, 
				pPrtData->qr_pub_sno, 
				pPrtData->qr_depr_trml_no, 
				pPrtData->qr_arvl_trml_no, 
				pPrtData->qr_sats_no);
		//SetQRCode("1234567890");
		SetQRCode(Buffer);
		Printf2(0x00, "%s\r\n", SetSpaceData(1));
	}

	/// [정보] QR text
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x00, "%s", SetSpaceData(8));
		sprintf(Buffer, "%s-%s-%s-%s %s%s%s\r\n", 
				pPrtData->qr_pub_dt, 
				pPrtData->qr_pub_trml_cd, 
				pPrtData->qr_pub_wnd_no, 
				pPrtData->qr_pub_sno, 
				pPrtData->qr_depr_trml_no, 
				pPrtData->qr_arvl_trml_no, 
				pPrtData->qr_sats_no);

		//Printf2(0x00, "20180502-010-02-0042 01013002\r\n");
		Printf2(0x00, Buffer);
	}

	/// 티켓고유번호
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x00, "%s", SetSpaceData(18));
		//Printf2(0x00, "001546\r\n");
		Printf2(0x00, "%08d\r\n", pAcc->Curr.expTicket.n_bus_tck_inhr_no);
	}

	///
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x10, "┎────────┰────────┰────┒\r\n");
	}

	///
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(1));		///*

		// 1) 출발지(한글)
		nMAX = 14;
		//sprintf(Buffer, "%s", "서울경부");
		sprintf(Buffer, "%s", pPrtData->depr_trml_ko_nm);
		nRet = IsHangul(Buffer, nMAX);
		if(nRet < 0)
		{
			Buffer[nMAX - 1] = 0;
		}
		AlignBuffer(nMAX, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x18, retBuf);

		Printf2(0x00, "%s", SetSpaceData(1));
		Printf2(0x10, "┃");

		// 2) 도착지(한글)
		nMAX = 14;
		Printf2(0x00, "%s", SetSpaceData(1));
		sprintf(Buffer, "%s", pPrtData->arvl_trml_ko_nm);
		nRet = IsHangul(Buffer, nMAX);
		if(nRet < 0)
		{
			Buffer[nMAX - 1] = 0;
		}
		AlignBuffer(nMAX, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x18, retBuf);

		Printf2(0x00, "%s", SetSpaceData(1));
		Printf2(0x10, "┃");
		
		// 3) 버스등급
		nMAX = 8;
		//sprintf(Buffer, "%s", "프리");
		sprintf(Buffer, "%s", pPrtData->bus_cls_nm);
		nRet = IsHangul(Buffer, nMAX);
		if(nRet < 0)
		{
			Buffer[nMAX - 1] = 0;
		}
		AlignBuffer(nMAX, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x18, retBuf);

		Printf2(0x10, "┃\r\n");						///~*
	}

	///
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(1));		///*

		// 1) 출발지(영문)
		nMAX = 14;
		//sprintf(Buffer, "%s", "Seoul");
		sprintf(Buffer, "%s", pPrtData->depr_trml_eng_nm);
		AlignBuffer(nMAX, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x00, retBuf);

		Printf2(0x00, "%s", SetSpaceData(1));
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(1));

		// 2) 도착지(영문)
		nMAX = 14;
		//sprintf(Buffer, "%s", "Ansung");
		sprintf(Buffer, "%s", pPrtData->arvl_trml_eng_nm);
		AlignBuffer(nMAX, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x00, retBuf);
		
		Printf2(0x00, "%s", SetSpaceData(1));
		Printf2(0x10, "┃");

		// 3) 승차권종류
		nMAX = 8;
		//sprintf(Buffer, "%s", "일반");
		sprintf(Buffer, "%s", pPrtData->bus_tck_knd_nm);
		nRet = IsHangul(Buffer, nMAX);
		if(nRet < 0)
		{
			Buffer[nMAX - 1] = 0;
		}
		AlignBuffer(nMAX, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x00, retBuf);

		Printf2(0x10, "┃\r\n");
	}

	/// 
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x10, "┖────────┸────────┸────┚\r\n");
	}

	/// 
	{
		SetLeftMargin(FIRST_GAP);
		//Printf2(0x00, "서울경부 11:30발 / 안성행");
		//sprintf(Buffer, "%s %s발 / %s행   %s km\r\n\r\n", 
		//		pPrtData->depr_trml_ko_nm, 
		//		pPrtData->atl_depr_time, 
		//		pPrtData->arvl_trml_ko_nm, 
		//		pPrtData->dist);
		sprintf(Buffer, "%s %s발 / %s행   %s km\r\n\r\n", 
				pPrtData->alcn_depr_trml_ko_nm, 
				pPrtData->alcn_depr_time, 
				pPrtData->alcn_arvl_trml_ko_nm, 
				pPrtData->dist);
		Printf2(0x00, Buffer);
	}

	/// 
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x00, "요금");
		Printf2(0x00, "%s", SetSpaceData(14));
		//Printf2(0x38, "5,700");
		Printf2(0x38, pPrtData->tisu_amt);
		Printf2(0x00, " 원\r\n\r\n");
	}

	///
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x10, "┎───────────┰──────────┒\r\n");
	}

	///
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(8));
		Printf2(0x18, "출발일");
		Printf2(0x00, "%s", SetSpaceData(8));
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(3));
		//Printf2(0x18, "2018.05.02 수");
		Printf2(0x18, pPrtData->atl_depr_dt);
		Printf2(0x00, "%s", SetSpaceData(4));
		Printf2(0x10, "┃\r\n");
	}

	/// 
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x08, "(Date of Depature)");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(20));
		Printf2(0x10, "┃\r\n");
	}

	/// 
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x10, "┠──────┰────╂─────┰────┨\r\n");
	}

	///
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x18, "출발시각");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(1));
		//Printf2(0x18, "11");
		//Printf2(0x30, ":");
		//Printf2(0x18, "30");
		Printf2(0x18, "%c%c", pPrtData->atl_depr_time[0], pPrtData->atl_depr_time[1]);
		Printf2(0x30, "%c", pPrtData->atl_depr_time[2]);
		Printf2(0x18, "%c%c", pPrtData->atl_depr_time[3], pPrtData->atl_depr_time[4]);

		Printf2(0x00, "%s", SetSpaceData(1));
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(3));
		Printf2(0x18, "좌석");
		Printf2(0x00, "%s", SetSpaceData(3));
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(3));
		//Printf2(0x18, "02");
		sprintf(Buffer, "%s", pPrtData->sats_no);
		AlignBuffer(8, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x18, Buffer);
		Printf2(0x00, "%s", SetSpaceData(3));
		Printf2(0x10, "┃\r\n");
	}

	/// 
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(3));
		Printf2(0x08, "(Time)");
		Printf2(0x00, "%s", SetSpaceData(3));
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(8));
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(1));
		Printf2(0x08, "(SeatNo)");
		Printf2(0x00, "%s", SetSpaceData(1));
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(8));
		Printf2(0x10, "┃\r\n");
	}

	///
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x10, "┠──────╂────╂─────╂────┨\r\n");
	}

	///
	{
		nMAX = 8;
		SetLeftMargin(FIRST_GAP);
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x18, "운송회사");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x10, "┃");
		//Printf2(0x18, "금호고속");
		sprintf(Buffer, "%s", pPrtData->bus_cacm_nm);
		nRet = IsHangul(Buffer, nMAX);
		if(nRet < 0)
		{
			Buffer[nMAX - 1] = 0;
		}
		AlignBuffer(8, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x18, retBuf);

		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x18, "승차홈");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x10, "┃");

		//sprintf(Buffer, "%s", "31234-호");
		nMAX = 8;
		sprintf(Buffer, "%s", pPrtData->rdhm_val);
		nRet = IsHangul(Buffer, nMAX);
		if(nRet < 0)
		{
			Buffer[nMAX - 1] = 0;
		}
		AlignBuffer(8, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x18, retBuf);

		Printf2(0x10, "┃\r\n");
	}

	///
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x10, "┃");
		Printf2(0x08, "(Express Co)");
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(8));
		Printf2(0x10, "┃");
		Printf2(0x08, "(platform)");
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(8));
		Printf2(0x10, "┃\r\n");
	}

	///
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x10, "┖──────┸────┸─────┸────┚\r\n");
	}

	///	승인번호, 카드정보
	{
		SetLeftMargin(FIRST_GAP);
		//Printf2(0x00, "승인번호 1234567");
		Printf2(0x00, "승인번호 %s", pPrtData->card_aprv_no);
		Printf2(0x00, "%s", SetSpaceData(7-4));
		//Printf2(0x00, "신용 1234567890123456\r\n");
		Printf2(0x00, "인증번호 %s\r\n", pPrtData->card_no);
	}

	///
	{
		SetLeftMargin(FIRST_GAP);
		//Printf2(0x00, "승인금액 34,700\r\n");
		Printf2(0x00, "승인금액 %s\r\n", pPrtData->card_aprv_amt);
	}

	/// 
	{
		SetLeftMargin(FIRST_GAP);
		//Printf2(0x18, "모바일티켓");
		sprintf(Buffer, "%s", SetSpaceData(10));
		if(nFunction == enFUNC_MRS)
		{
			sprintf(Buffer, "모바일티켓");
		}
		AlignBuffer(10, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x18, retBuf);

		Printf2(0x00, "%s", SetSpaceData(8));
		
		// 20230206 ADD~
		//PUBTCK_T* pBase;
		//pBase = &CPubTckMem::GetInstance()->base;
		//LOG_OUT(" >>>>> 지불수단 = %d ", pBase->ui_pym_dvs_cd[0]);
		//LOG_OUT(" >>>>> 현금영수증 구분코드 - 0:미사용, 1:수기입력, 2:신용카드 = %d ", pBase->ui_csrc_dvs_cd[0]);
		LOG_OUT(" >>>>> 지불수단(pPrtData->n_pym_dvs)(PYM_CD_CSRC:'B',현금영수증) = %d ", pPrtData->n_pym_dvs);
		// 20230206 ~ADD

		if(pPrtData->n_pym_dvs == PYM_CD_CARD)
		{
			sprintf(Buffer, "현장카드");
			if(nFunction == enFUNC_MRS)
			{
				sprintf(Buffer, "신용카드");
			}
			Printf2(0x38, Buffer);

			if(pPrtData->n_mip_mm_num == 0)
			{
				Printf2(0x08, " (일시불)\r\n\r\n");
			}
			else
			{
				Printf2(0x08, " (%d 개월)\r\n\r\n", pPrtData->n_mip_mm_num);
			}
		}
		else if(pPrtData->n_pym_dvs == PYM_CD_TPAY)
		{
			sprintf(Buffer, "페이");
			Printf2(0x38, Buffer);
			if(pPrtData->n_mip_mm_num == 0)
			{
				Printf2(0x08, " (일시불)\r\n\r\n");
			}
			else
			{
				Printf2(0x08, " (%d 개월)\r\n\r\n", pPrtData->n_mip_mm_num);
			}
		}
		// 20210910 ADD
		else if ( (pPrtData->n_pym_dvs == PYM_CD_ETC) || (pPrtData->n_pym_dvs == PYM_CD_COMP) )
		{
			sprintf(Buffer, "복합결제\r\n\r\n");
			Printf2(0x38, Buffer);
			/*
			if(pPrtData->n_mip_mm_num == 0)
			{
				Printf2(0x08, " (일시불)\r\n\r\n");
			}
			else
			{
				Printf2(0x08, " (%d 개월)\r\n\r\n", pPrtData->n_mip_mm_num);
			}
			*/
		}
		// 20210910 ~ADD
		// 20221124 ADD~
		else if (pPrtData->n_pym_dvs == PYM_CD_QRPC)
		{
			sprintf(Buffer, "PAYCO카드\r\n\r\n");
			Printf2(0x38, Buffer);
		}
		else if (pPrtData->n_pym_dvs == PYM_CD_QRPP)
		{
			sprintf(Buffer, "PAYCO포인트\r\n\r\n");
			Printf2(0x38, Buffer);
		}
		else if (pPrtData->n_pym_dvs == PYM_CD_QRTP)
		{
			sprintf(Buffer, "티머니페이\r\n\r\n");
			Printf2(0x38, Buffer);
		}
		// 20221124 ~ADD
		// 20230206 ADD~
		// PYM_CD_CASH				'A'			///< 현금
		// PYM_CD_CSRC				'B'			///< 현금영수증
		// PYM_CD_CARD				'C'			///< 신용카드
		// PYM_CD_RF				'D'			///< RF
 		//else if ( (pBase->ui_csrc_dvs_cd[0] == PYM_CD_CSRC) /// 지불수단
		//	|| ( (pBase->ui_csrc_dvs_cd[0] == 1) || (pBase->ui_csrc_dvs_cd[0] == 2) ) ) ///< 현금영수증 구분코드 - 0:미사용, 1:수기입력, 2:신용카드
		else if (pPrtData->n_pym_dvs == PYM_CD_CSRC) // 지불수단(pPrtData->n_pym_dvs)(PYM_CD_CSRC:'B',현금영수증)
		{
			//LOG_OUT(" >>>>> 현금영수증 구분코드 == 1:수기입력, 2:신용카드 ");
			LOG_OUT(" >>>>> 지불수단 구분코드 == PYM_CD_CSRC:'B' ");
			//Printf2(0x38, "현금(소득공제)\r\n\r\n");	// 20230206 DEL
			Printf2(0x08, "현금(소득공제)\r\n\r\n");		// 20230206 MOD
		}
		// 20230206 ~ADD
		else
		{
			LOG_OUT(" >>>>> 현금영수증 구분코드 == 0:미사용 ");
			//Printf2(0x38, "현금\r\n\r\n");		// 20230206 DEL
			Printf2(0x08, "현금\r\n\r\n");		// 20230206 MOD
		}
	}

	///
	{
		SetLeftMargin(FIRST_GAP);
		//Printf2(0x08, "※유효기간:당일지정차에 한함  [13:06]\r\n");
		Printf2(0x08, "※유효기간:당일지정차에 한함  %s\r\n", pPrtData->pub_time);
	}

	///
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x08, "고속회사:");
		//Printf2(0x00, "금호속리산고속(주) 301-81-00353\r\n");
		Printf2(0x00, "%s %s\r\n", pPrtData->bus_cacm_nm, pPrtData->bus_cacm_biz_no);
	}

	///	가맹점 정보
	{
		SetLeftMargin(FIRST_GAP);
		//Printf2(0x08, "(주)엔에스피 129-86-27172\r\n");
		//sprintf(Buffer, "가맹점사업자번호:%s", pOperConfig->koTrmlInfo_t.sz_prn_trml_corp_no);
		Printf2(0x08, "%s %s\r\n", pOperConfig->koTrmlInfo_t.sz_prn_trml_nm, pOperConfig->koTrmlInfo_t.sz_prn_trml_corp_no);
	}

	/// 가맹점 전화번호
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x08, "Tel:");
		//Printf2(0x00, "02-6282-0114");
		Printf2(0x00, "%s", pOperConfig->koTrmlInfo_t.sz_prn_trml_tel_no);

		if(pPrtData->n_pym_dvs == PYM_CD_CARD)
		{
			Printf2(0x00, "%s\r\n", SetSpaceData(14));
		}
		else
		{
			Printf2(0x00, "%s", SetSpaceData(14));
			Printf2(0x00, "현금영수증문의\r\n");
		}
	}

	///	가맹점 ARS번호
	{
		SetLeftMargin(FIRST_GAP);
		Printf2(0x08, "ARS:");
		//Printf2(0x00, "02-2088-2635");
		Printf2(0x00, "%s", pOperConfig->koTrmlInfo_t.sz_prn_trml_ars_no);

		if(pPrtData->n_pym_dvs == PYM_CD_CARD)
		{
			Printf2(0x00, "%s\r\n", SetSpaceData(25));
		}
		else
		{
			Printf2(0x00, "%s", SetSpaceData(25));
			Printf2(0x00, "126-2\r\n");
		}
	}

	///
	{
// 		SetLeftMargin(FIRST_GAP);
// 		Printf2(0x00, "%s", SetSpaceData(18));
// 		Printf2(0x38, "안내 1688-4700");
	}

	PaperFullCut();
}
