// 
// 
// dev_prt_nice_rxd.cpp : 한전금 - REXOD 프린터	(RX830-H120)
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "dev_prt_nice_rxd.h"
#include "dev_prt_main.h"
#include "event_if.h"
#include "MyDataAccum.h"
#include "dev_tr_mem.h"
#include "dev_tr_kobus_mem.h"
#include "dev_tr_tmexp_mem.h"
#include "dev_tr_main.h"
#include "oper_kos_file.h"
#include "oper_config.h"
#include "File_Env_ini.h"
#include "cmn_util.h"
#include "data_main.h"

//----------------------------------------------------------------------------------------------------------------------

#define LOG_OUT(fmt, ...)		{ CPrinterRxd::m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ CPrinterRxd::m_clsLog.HexaDump(x, y, z); }

static PDEV_CFG_T		pEnv = NULL;		// 20230206 ADD
static PDEV_CFG_T		pEnvPrtTck = NULL;	// 20230206 ADD

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CPrinterRxd
 * @details		생성자
 */
CPrinterRxd::CPrinterRxd()
{
	m_hAccMutex = NULL;
	m_bConnected = FALSE;
}

/**
 * @brief		~CPrinterRxd
 * @details		소멸자
 */
CPrinterRxd::~CPrinterRxd()
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
void CPrinterRxd::LOG_INIT(void)
{
	m_clsLog.SetData(30, "\\Log\\Prt\\Rxd");
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
int CPrinterRxd::Locking(void)
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
int CPrinterRxd::UnLocking(void)
{
	if(m_hAccMutex != NULL) 
	{
		::ReleaseMutex(m_hAccMutex);
	}

	return 0;	
}

/**
 * @brief		SendData
 * @details		Serial 데이타 전송
 * @param		BYTE *pData		전송 데이타
 * @param		int nDataLen	전송 데이타 길이
 * @return		전송데이타 길이
 */
int CPrinterRxd::SendData(BYTE *pData, int nDataLen)
{
	return m_clsComm.SendData(pData, nDataLen);
}

/**
 * @brief		GetPacket
 * @details		Serial 데이타 수신
 * @param		BYTE *retBuf	receive data
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrinterRxd::GetPacket(BYTE *retBuf)
{
	static int	nState = 0;
	static int	nRecv = 0;
	int	ch;

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
		case 0 :	
			nRecv = 0;
			retBuf[nRecv++] = ch;
			nState++;
			break;

		case 1:		
			retBuf[nRecv++] = ch;
			goto got_packet;
		}
	}

got_packet:
	
	nState = 0;

	LOG_HEXA("GetPacket", retBuf, nRecv);

	return 2;
}

/**
 * @brief		InitPrinter
 * @details		Initialize Printer
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrinterRxd::InitPrinter(void)
{
	int nRet;

	Locking();
	{
		BYTE pSendData[] = "\x1B\x40\x1B\x32\x1B\x21\x00\x1D\x50\x01\x1B\x52\x0D";

		nRet = SendData(pSendData, 13);
	}
	UnLocking();
	return 0;
}

/**
 * @brief		SetQRCode
 * @details		Set QR Code
 * @param		char *pData			QR Data
 * @return		항상 = 0
 */
int CPrinterRxd::SetQRCode(char *pData)
{
	int nRet, nLen, nOffset;

	nRet = nOffset = 0;
	Locking();
	{
		BYTE Buffer[256];

		/// 1 : left margin 209
		nRet = SendData((BYTE *)"\x1D\x4C\xC8\x00", 4);

		/// 2 : QR Code Print (1D 6C xL xH R EM sL sH data...)
		CopyMemory(&Buffer[nOffset], "\x1D\x6C\x00\x00\x00\x07", 6);
		nOffset += 6;
		
		nLen = strlen(pData);
		CopyMemory(&Buffer[nOffset], &nLen, 2);
		nOffset += 2;

		nOffset += sprintf((char *)&Buffer[nOffset], "%s", pData);
		nRet = SendData(Buffer, nOffset);

		// 3
		//nRet = SendData((BYTE *)"\x1D\x6C\x00\x00\x00\x07\x0A\x00", 8);
		// 3 : left margin zero
		nRet = SendData((BYTE *)"\x1D\x4C\x00\x00", 4);

	}
	UnLocking();
	return 0;
}

/**
 * @brief		SetCharPrtingMode
 * @details		Set character printing mode
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrinterRxd::SetCharPrtingMode(BYTE byVal)
{
	int nRet;

	Locking();
	{
		BYTE Buffer[4] = "\x1B\x21\x00";

		Buffer[2] = byVal;

		nRet = SendData(Buffer, 3);
	}
	UnLocking();
	return 0;
}

/**
 * @brief		SetPrintPosition
 * @details		Set absolute print position
 * @param		WORD	wPos		position 
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrinterRxd::SetPrintPosition(WORD wPos)
{
	int nRet;

	Locking();
	{
		BYTE Buffer[10] = "\x1B\x24\x00";

		::CopyMemory(&Buffer[2], &wPos, 2);

		nRet = SendData(Buffer, 4);
	}
	UnLocking();
	return 0;
}

/**
 * @brief		LineFeed
 * @details		Print and feed n lines
 * @param		int nVal			n lines
 * @return		항상 = 0
 */
int CPrinterRxd::SetImageData(WORD x, WORD y, BYTE *pImgData, int nLen)
{
	int nRet, nOffset;

	nRet = nOffset = 0;

	Locking();
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
	UnLocking();
	return 0;
}

/**
 * @brief		LineFeed
 * @details		Print and feed n lines
 * @param		int nVal			n lines
 * @return		항상 = 0
 */
int CPrinterRxd::LineFeed(int nVal)
{
	int nRet;

	Locking();
	{
		BYTE Buffer[4] = "\x1B\x64\x00";

		Buffer[2] = nVal & 0xFF;

		nRet = SendData(Buffer, 3);
	}
	UnLocking();
	return 0;
}

/**
 * @brief		FullCut
 * @details		Full cut
 * @param		None
 * @return		None
 */
void CPrinterRxd::FullCut(void)
{		
	int nRet;

	Locking();
	{
		BYTE Buffer[] = "\x1B\x69";

		nRet = SendData(Buffer, 2);
	}
	UnLocking();
}

/**
 * @brief		PaperFullCut
 * @details		용지 절단
 * @param		None
 * @return		None
 */
void CPrinterRxd::PaperFullCut(void)
{
	LineFeed(6);
	FullCut();
}

BYTE *CPrinterRxd::SetSpaceData(int nSpaceLen)
{
	memset(m_szTemp, 0x20, sizeof(m_szTemp));
	m_szTemp[nSpaceLen] = 0;
	return m_szTemp;
}

/**
 * @brief		GetStatus
 * @details		프린터 상태 명령 전송
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrinterRxd::GetStatus(void)
{
	int nRet, nError;
	DWORD dwTick;
	BYTE byStatus = 0;
	BYTE pSendData[] = "\x1B\x48";

	//LOG_OUT("start... ");

	Locking();

	nRet = SendData(pSendData, 2);
	//LOG_OUT("SendData... nRet = %d", nRet);

	nError = 0;
	dwTick = GetTickCount();
	while( Util_CheckExpire(dwTick) < 500 )	
	{
		nRet = GetPacket(m_szRxBuf);
		if(nRet < 0)
		{
			continue;
		}

		Util_Ascii2BCD((char *)m_szRxBuf, 2, &byStatus);
		byStatus = byStatus & 0xFF;
		LOG_OUT("GetStatus(), resp = [%02X, %02X], [%02X] ..", m_szRxBuf[0], m_szRxBuf[1], byStatus & 0xFF);

		//if( memcmp("00", m_szRxBuf, 2) == 0 )
		//{	/// 정상
		//	;
		//}

		if(GetConfigTicketPapaer() == PAPER_ROLL)									// 20230206 DEL // 20230217 MOD
		//pEnv = (PDEV_CFG_T) GetEnvPrtReceiptInfo();								// 20230206 ADD // 20230217 DEL
		//if(pEnv->nUse == 1) // 영수증(감열지) 프린터 사용 옵션이 'USE = 1'인 경우		// 20230206 ADD // 20230217 DEL
		{
			SetCheckEventCode(EC_PRT_ROLL_COMM_ERR, FALSE);

			/// 용지없음
			//if( memcmp("01", m_szRxBuf, 2) == 0 )
			if(byStatus & 0x01)
			{	
				SetCheckEventCode(EC_PRT_ROLL_NO_PAPER, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_ROLL_NO_PAPER, FALSE);
			}

			/// HEAD-UP Sensor
			//if( (memcmp("02", m_szRxBuf, 2) == 0) || (memcmp("03", m_szRxBuf, 2) == 0) )
			if( byStatus & 0x02 )
			{	
				SetCheckEventCode(EC_PRT_ROLL_HEAD_UP, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_ROLL_HEAD_UP, FALSE);
			}

			/// Cutter Sensor
			//if( memcmp("04", m_szRxBuf, 2) == 0 )
			if( byStatus & 0x04 )
			{	
				SetCheckEventCode(EC_PRT_ROLL_CUTTER_ERR, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_ROLL_CUTTER_ERR, FALSE);
			}

			/// Near-End Sensor
			//if( memcmp("10", m_szRxBuf, 2) == 0 )
			if( byStatus & 0x10 )
			{	
				SetCheckEventCode(EC_PRT_ROLL_PAPER_NEAR_END, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_ROLL_PAPER_NEAR_END, FALSE);
			}

			/// Paper JAM
			//if( memcmp("20", m_szRxBuf, 2) == 0 )
			if( byStatus & 0x20 )
			{	
				SetCheckEventCode(EC_PRT_ROLL_PAPER_JAM, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_ROLL_PAPER_JAM, FALSE);
			}

			/// Paper out Sensor
			//if( memcmp("40", m_szRxBuf, 2) == 0 )
			if( byStatus & 0x40 )
			{	
				SetCheckEventCode(EC_PRT_ROLL_OUT_SENS_ERR, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_ROLL_OUT_SENS_ERR, FALSE);
			}

			/// BM Error
//			if( memcmp("80", m_szRxBuf, 2) == 0 )
			if( byStatus & 0x80 )
			{	
				SetCheckEventCode(EC_PRT_ROLL_BM_ERR, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_ROLL_BM_ERR, FALSE);
			}
		}
		//else // 20230206 DEL
		pEnvPrtTck = (PDEV_CFG_T) GetEnvPrtTicketInfo();							// 20230206 ADD
		if(pEnvPrtTck->nUse == 1) // 승차권(종이) 프린터 사용 옵션이 'USE = 1'인 경우	// 20230206 ADD
		{
			SetCheckEventCode(EC_PRT_COMM_ERR, FALSE);

			/// 용지없음
			//if( memcmp("01", m_szRxBuf, 2) == 0 )
			if( byStatus & 0x01 )
			{	
				SetCheckEventCode(EC_PRT_NO_PAPER, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_NO_PAPER, FALSE);
			}

			/// HEAD-UP Sensor
			//if( (memcmp("02", m_szRxBuf, 2) == 0) || (memcmp("03", m_szRxBuf, 2) == 0) )
			if( byStatus & 0x02 )
			{	
				SetCheckEventCode(EC_PRT_HEAD_UP, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_HEAD_UP, FALSE);
			}

			/// Cutter Sensor
			//if( memcmp("04", m_szRxBuf, 2) == 0 )
			if( byStatus & 0x04 )
			{	
				SetCheckEventCode(EC_PRT_CUTTER_ERR, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_CUTTER_ERR, FALSE);
			}

			/// Near-End Sensor
			//if( memcmp("10", m_szRxBuf, 2) == 0 )
			if( byStatus & 0x10 )
			{	
				SetCheckEventCode(EC_PRT_PAPER_NEAR_END, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_PAPER_NEAR_END, FALSE);
			}

			/// Paper JAM
			//if( memcmp("20", m_szRxBuf, 2) == 0 )
			if( byStatus & 0x20 )
			{	
				SetCheckEventCode(EC_PRT_PAPER_JAM, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_PAPER_JAM, FALSE);
			}

			/// Paper out Sensor
			//if( memcmp("40", m_szRxBuf, 2) == 0 )
			if( byStatus & 0x40 )
			{	
				SetCheckEventCode(EC_PRT_OUT_SENS_ERR, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_OUT_SENS_ERR, FALSE);
			}

			/// BM Error
			//if( memcmp("80", m_szRxBuf, 2) == 0 )
			if( byStatus & 0x80 )
			{	
				SetCheckEventCode(EC_PRT_BM_ERR, TRUE);
				nError = TRUE;
			}
			else
			{
				SetCheckEventCode(EC_PRT_BM_ERR, FALSE);
			}
		}

		UnLocking();

		if(nError == TRUE)
		{
			return -2;
		}

		return 1;
	}
	
	//if(GetConfigTicketPapaer() == PAPER_ROLL)									// 20230206 DEL
	pEnv = (PDEV_CFG_T) GetEnvPrtReceiptInfo();									// 20230206 ADD
	//LOG_OUT("pEnv->nUse = %d", pEnv->nUse);
	if(pEnv->nUse == 1) // 영수증(감열지) 프린터 사용 옵션이 'USE = 1'인 경우		// 20230206 ADD // 20230215 DEL
	//if( (pEnv->nUse == 1) && ( ((prev_ch & 0xFF) == false) && ((ch & 0xFF) == false) ) ) // 영수증(감열지) 프린터 사용 옵션이 'USE = 1'인 경우		// 20230206 ADD // 20230215 MOD
	{
		SetCheckEventCode(EC_PRT_ROLL_COMM_ERR, TRUE);
	}
	//else // 20230206 DEL
	pEnvPrtTck = (PDEV_CFG_T) GetEnvPrtTicketInfo();							// 20230206 ADD
	//LOG_OUT("pEnvPrtTck->nUse = %d", pEnvPrtTck->nUse);
	if(pEnvPrtTck->nUse == 1) // 승차권(종이) 프린터 사용 옵션이 'USE = 1'인 경우	// 20230206 ADD
	{
		SetCheckEventCode(EC_PRT_COMM_ERR, TRUE);
	}

	UnLocking();

	return -1;
}

/**
 * @brief		Printf
 * @details		프린트 데이타 출력
 * @param		const char *fmt			문자 포맷
 * @return		항상 = 0
 */
int CPrinterRxd::Printf(const char *fmt, ...)
{
	int		len, nRet, off, cnt;
	va_list ap;
	char	Buffer[1024 + 10];

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

	return 0;
}

/**
 * @brief		Printf2
 * @details		프린트 데이타 출력
 * @param		const char *fmt			문자 포맷
 * @return		항상 = 0
 */
int CPrinterRxd::Printf2(int nCharMode, const char *fmt, ...)
{
	int		len, nRet, off, cnt;
	va_list ap;
	char	Buffer[1024 + 10];

	SetCharPrtingMode(nCharMode & 0xFF);
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
	SetCharPrtingMode(0);

	return 0;
}

/**
 * @brief		Print_Account
 * @details		시재 정보 프린트
 * @param		None
 * @return		None
 */
void CPrinterRxd::Print_Account(void)
{
	int nValue = 0;

	LOG_OUT("start !!!");
	{
		SYSTEMTIME			st;
		char				retBuf[200];
		PFILE_ACCUM_N1010_T	pAccum;
		char				*pTrmlCode;

		pAccum = (PFILE_ACCUM_N1010_T) GetAccumulateData();

		GetLocalTime(&st);

		::ZeroMemory(retBuf, sizeof(retBuf));

		SetCharPrtingMode(0x00);
		SetCharPrtingMode(0x10);
		Printf("━━━━━━━━━━━━━━━━━━━━━━━━\n");
		SetCharPrtingMode(0x38);
		Printf("	현금 시재 명세서\n");
		SetCharPrtingMode(0x00);
		Printf("\n");
		
		pTrmlCode = GetTrmlCode(SVR_DVS_CCBUS);
		FindTerminalName(LANG_KOR, pTrmlCode, retBuf);
		Printf("터 미 널 : %s\n", retBuf);

		Printf("창구번호 : %s\n", GetTrmlWndNo(SVR_DVS_CCBUS));
		//	Printf("설정일시 : 2019-10-15 17:50:55\n");
		Printf("출력일시 : %04d-%02d-%02d %02d:%02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		SetCharPrtingMode(0x10);
		Printf("━━━━━━━━━━━━━━━━━━━━━━━━\n");
		SetCharPrtingMode(0x00);
		Printf("\n");
		Printf("\n");
		SetCharPrtingMode(0x30);
		Printf("*** 직전 시재 ***\n");
		SetCharPrtingMode(0x00);
		Printf("\n");

		nValue = (pAccum->Prev.BillBox.n50k * 50000) + (pAccum->Prev.BillBox.n10k * 10000) + (pAccum->Prev.BillBox.n5k * 5000) + (pAccum->Prev.BillBox.n1k * 1000);

		SetCharPrtingMode(0x10);
//		Printf("입금부 총액 : 75,000 원\n");
		Printf("입금부 총액 : %d 원\n", nValue);
		SetCharPrtingMode(0x00);
		Printf("\n");
		Printf("   단위          수량                     금액\n");
		Printf("━━━━━━━━━━━━━━━━━━━━━━━━\n");
		Printf("   5만원          %4d               %9d\n", pAccum->Prev.BillBox.n50k, pAccum->Prev.BillBox.n50k * 50000);
		Printf("   1만원          %4d               %9d\n", pAccum->Prev.BillBox.n10k, pAccum->Prev.BillBox.n10k * 10000);
		Printf("   5천원          %4d               %9d\n", pAccum->Prev.BillBox.n5k, pAccum->Prev.BillBox.n5k * 5000);
		Printf("   1천원          %4d               %9d\n", pAccum->Prev.BillBox.n1k, pAccum->Prev.BillBox.n1k * 1000);
		Printf("━━━━━━━━━━━━━━━━━━━━━━━━\n");
		Printf("\n");

		nValue = (pAccum->Prev.BillDispenser.Casst.n10k * 10000) + (pAccum->Prev.BillDispenser.Casst.n1k * 1000) + (pAccum->Prev.Coin.n500 * 500) + (pAccum->Prev.Coin.n100 * 100);

		SetCharPrtingMode(0x10);
//		Printf("출금부 총액 : 5,798,000 원\n");
		Printf("출금부 총액 : %d 원\n", nValue);
		SetCharPrtingMode(0x00);
		Printf("\n");
		Printf("   단위          수량                     금액\n");
		Printf("━━━━━━━━━━━━━━━━━━━━━━━━\n");
		Printf("   1만원          %4d               %9d\n", pAccum->Prev.BillDispenser.Casst.n10k, pAccum->Prev.BillDispenser.Casst.n10k * 10000);
		Printf("   1천원          %4d               %9d\n", pAccum->Prev.BillDispenser.Casst.n1k,  pAccum->Prev.BillDispenser.Casst.n1k * 1000);
		Printf("   5백원          %4d               %9d\n", pAccum->Prev.Coin.n500				,  pAccum->Prev.Coin.n500 * 500);
		Printf("   1백원          %4d               %9d\n", pAccum->Prev.Coin.n100				,  pAccum->Prev.Coin.n100 * 100);
		Printf("━━━━━━━━━━━━━━━━━━━━━━━━\n");
		Printf("\n");
		Printf("\n");
		SetCharPrtingMode(0x30);

		///////////////////////////////////////////////////////////////////////////////////////////

		Printf("*** 현재 시재 ***\n");
		SetCharPrtingMode(0x00);
		Printf("\n");

		nValue = (pAccum->Curr.BillBox.n50k * 50000) + (pAccum->Curr.BillBox.n10k * 10000) + (pAccum->Curr.BillBox.n5k * 5000) + (pAccum->Curr.BillBox.n1k * 1000);

		SetCharPrtingMode(0x10);
//		Printf("입금부 총액 : 88,000 원\n");
		Printf("입금부 총액 : %d 원\n", nValue);
		SetCharPrtingMode(0x00);
		Printf("\n");
		Printf("   단위          수량                     금액\n");
		Printf("━━━━━━━━━━━━━━━━━━━━━━━━\n");
//		Printf("   5만원             1                   50,000\n");
		Printf("   5만원          %4d               %9d\n", pAccum->Curr.BillBox.n50k, pAccum->Curr.BillBox.n50k * 50000);
		Printf("   1만원          %4d               %9d\n", pAccum->Curr.BillBox.n10k, pAccum->Curr.BillBox.n10k * 10000);
		Printf("   5천원          %4d               %9d\n", pAccum->Curr.BillBox.n5k, pAccum->Curr.BillBox.n5k * 5000);
		Printf("   1천원          %4d               %9d\n", pAccum->Curr.BillBox.n1k, pAccum->Curr.BillBox.n1k * 1000);
		Printf("━━━━━━━━━━━━━━━━━━━━━━━━\n");
		Printf("\n");
		
		nValue = (pAccum->Curr.BillDispenser.Casst.n10k * 10000) + (pAccum->Curr.BillDispenser.Casst.n1k * 1000) + (pAccum->Curr.Coin.n500 * 500) + (pAccum->Curr.Coin.n100 * 100);

		SetCharPrtingMode(0x10);
//		Printf("   출금부 총액 : 5,798,000 원\n");
		Printf("출금부 총액 : %d 원\n", nValue);
		SetCharPrtingMode(0x00);
		Printf("\n");
		Printf("   단위          수량                     금액\n");
		Printf("━━━━━━━━━━━━━━━━━━━━━━━━\n");
//		Printf("   1만원           500                5,000,000\n");
		Printf("   1만원          %4d               %9d\n", pAccum->Curr.BillDispenser.Casst.n10k, pAccum->Curr.BillDispenser.Casst.n10k * 10000);
		Printf("   1천원          %4d               %9d\n", pAccum->Curr.BillDispenser.Casst.n1k, pAccum->Curr.BillDispenser.Casst.n1k * 1000);
		Printf("   5백원          %4d               %9d\n", pAccum->Curr.Coin.n500, pAccum->Curr.Coin.n500 * 500);
		Printf("   1백원          %4d               %9d\n", pAccum->Curr.Coin.n100, pAccum->Curr.Coin.n100 * 100);
		Printf("━━━━━━━━━━━━━━━━━━━━━━━━\n");
		Printf("\n");
		Printf("\n");
		Printf("   서   명  : ________________________________\n");
		Printf("\n");
		Printf("	시재명세를 확인후에 서명바랍니다.\n");


		PaperFullCut();
	}
}

void CPrinterRxd::Print_Account2(void)
{
	SYSTEMTIME			st;
	char				retBuf[200];
	int					i, k;
	int					nSum[10] = {0, };
	PFILE_ACCUM_N1010_T	pAccum;
	char				*pTrmlCode;
	PDM_COIN_T			pCoin;
	PABILL_T			pCDU;
	PDM_BILL_T			pBillBox;
	PKIOSK_INI_ENV_T	pEnv;
	char				*pWndNo;

	pAccum = (PFILE_ACCUM_N1010_T) GetAccumulateData();

	pCoin		= &pAccum->Curr.Coin;
	pBillBox	= &pAccum->Curr.BillBox;
	pCDU		= &pAccum->Curr.BillDispenser.Casst;

	GetLocalTime(&st);

	::ZeroMemory(retBuf, sizeof(retBuf));

	pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();

	pTrmlCode = NULL;
	pWndNo = NULL;

	Printf2(0x10, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	Printf2(0x38, "	마감 내역 명세서\n");
	Printf2(0x00, "\n");

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
		;
	}

	Printf2(0x00, "터 미 널 : %s\n", retBuf);
	Printf2(0x00, "창구번호 : %s\n", pWndNo);
	//	Printf("설정일시 : 2019-10-15 17:50:55\n");
	Printf2(0x00, "출력일시 : %04d-%02d-%02d %02d:%02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	Printf2(0x10, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	Printf2(0x00, "\n");

	Printf2(0x10, "%s", "[ 1. 출금부 시재 ]\n");
	Printf2(0x00, "\n");
	Printf2(0x00, "   단위          수량                     금액\n");
	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	Printf2(0x00, "    100원         %4d               %9d\n", pCoin->n100, COIN_100_SUM(pCoin->n100));
	Printf2(0x00, "    500원         %4d               %9d\n", pCoin->n500, COIN_500_SUM(pCoin->n500));
	Printf2(0x00, "  1,000원         %4d               %9d\n", pCDU->n1k, BILL_1K_SUM(pCDU->n1k));
	Printf2(0x00, " 10,000원         %4d               %9d\n", pCDU->n10k, BILL_10K_SUM(pCDU->n10k));
	nSum[0] = COIN_SUM(0, 0, pAccum->Curr.Coin.n100, pAccum->Curr.Coin.n500) + BILL_SUM(pCDU->n1k, 0, pCDU->n10k, 0);
	Printf2(0x00, " 합    계                            %9d\n",  nSum[0]);
	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

	Printf2(0x00, "\n");
	Printf2(0x10, "%s", "[ 2. 입금부 시재 ]\n");
	Printf2(0x00, "\n");
	Printf2(0x00, "   단위          수량                     금액\n");
	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	Printf2(0x00, "  1,000원         %4d               %9d\n", pBillBox->n1k, BILL_1K_SUM(pBillBox->n1k));
	Printf2(0x00, "  5,000원         %4d               %9d\n", pBillBox->n5k, BILL_5K_SUM(pBillBox->n5k));
	Printf2(0x00, " 10,000원         %4d               %9d\n", pBillBox->n10k, BILL_10K_SUM(pBillBox->n10k));
	Printf2(0x00, " 50,000원         %4d               %9d\n", pBillBox->n50k, BILL_50K_SUM(pBillBox->n50k));
	nSum[1] = BILL_SUM(pBillBox->n1k, pBillBox->n5k, pBillBox->n10k, pBillBox->n50k);
	Printf2(0x00, " 합    계                            %9d\n", nSum[1]);
	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

	Printf2(0x00, "\n");
	Printf2(0x10, "%s", "[ 3. 보급내역 ]\n");
	Printf2(0x00, "\n");
	Printf2(0x00, "   단위          수량                     금액\n");
	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	Printf2(0x00, "    100원         %4d               %9d\n", pAccum->Close.supplyCoin.n100, COIN_100_SUM(pAccum->Close.supplyCoin.n100));
	Printf2(0x00, "    500원         %4d               %9d\n", pAccum->Close.supplyCoin.n500, COIN_500_SUM(pAccum->Close.supplyCoin.n500));
	Printf2(0x00, "  1,000원         %4d               %9d\n", pAccum->Close.supplyBill.n1k,  BILL_1K_SUM(pAccum->Close.supplyBill.n1k));
	Printf2(0x00, " 10,000원         %4d               %9d\n", pAccum->Close.supplyBill.n10k,  BILL_10K_SUM(pAccum->Close.supplyBill.n10k));
	nSum[2] = COIN_SUM(0, 0, pAccum->Close.supplyCoin.n100, pAccum->Close.supplyCoin.n500) + BILL_SUM(pAccum->Close.supplyBill.n1k, 0, pAccum->Close.supplyBill.n10k, 0);
	Printf2(0x00, " 합    계                            %9d\n", nSum[2]);
	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

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

		Printf2(0x00, "\n");
		Printf2(0x10, "%s", "[ 4. 발권내역 (현금) ]\n");
		Printf2(0x00, "\n");
		Printf2(0x00, "   구분          수량                     금액\n");
		Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		Printf2(0x00, " 시외발권         %4d               %9d\n", sumT1.nCount, sumT1.dwMoney);
		Printf2(0x00, " 고속발권         %4d               %9d\n", sumT2.nCount, sumT2.dwMoney);
		Printf2(0x00, " 합    계         %4d               %9d\n", sumT1.nCount + sumT2.nCount, sumT1.dwMoney + sumT2.dwMoney);
		Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
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

		Printf2(0x00, "\n");
		Printf2(0x10, "%s", "[ 5. 환불내역 (현금) ]\n");
		Printf2(0x00, "\n");
		Printf2(0x00, "   구분          수량                     금액\n");
		Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		Printf2(0x00, " 시외환불         %4d               %9d\n", sumT1.nCount, sumT1.dwMoney);
		Printf2(0x00, " 고속환불         %4d               %9d\n", sumT2.nCount, sumT2.dwMoney);
		Printf2(0x00, " 합    계         %4d               %9d\n", sumT1.nCount + sumT2.nCount, sumT1.dwMoney + sumT2.dwMoney);
		Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

		Printf2(0x00, "\n");
		Printf2(0x10, " 결과 (1 + 2 - 3 - 4 - 5)          %9d원\n", nSum[0] + nSum[1] - nSum[2] - nSum[3] - nSum[4]);
		Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		Printf2(0x00, "\n");
	}

	PaperFullCut();
}

void CPrinterRxd::Print_Account3(char *pDate, char *pTime)
{
	SYSTEMTIME			st;
	SYSTEMTIME			stFile;
	char				retBuf[200];
	int					nRet, i, k;
	int					nSum[10] = {0, };
	CLOSE_TR_DATA_T		TrData;
	PFILE_ACCUM_N1010_T	pAccum;
	char				*pTrmlCode;
	PKIOSK_INI_ENV_T	pEnv;
	char				*pWndNo;
	PCLOSE_TR_DATA_T	pClsTR;

	pAccum = (PFILE_ACCUM_N1010_T) GetAccumulateData();

	GetLocalTime(&st);

	::ZeroMemory(retBuf, sizeof(retBuf));

	pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();

	pTrmlCode = NULL;
	pWndNo = NULL;

	::ZeroMemory(&TrData, sizeof(CLOSE_TR_DATA_T));
	pClsTR = (PCLOSE_TR_DATA_T) &TrData;

	nRet = SearchCashCloseData(pDate, pTime, (char *)pClsTR);
	if(nRet < 0)
	{
		TR_LOG_OUT("시재마감 프린트 데이타 찾기 실패...");
		return;
	}

	Printf2(0x10, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	Printf2(0x38, "	시재마감 내역 명세서\n");
	Printf2(0x00, "\n");

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

	Printf2(0x00, "터 미 널 : %s\n", retBuf);
	Printf2(0x00, "창구번호 : %s\n", pWndNo);
	//	Printf("설정일시 : 2019-10-15 17:50:55\n");
	Printf2(0x00, "마감일시 : %04d-%02d-%02d %02d:%02d:%02d\n", stFile.wYear, stFile.wMonth, stFile.wDay, stFile.wHour, stFile.wMinute, stFile.wSecond);
	Printf2(0x00, "출력일시 : %04d-%02d-%02d %02d:%02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	Printf2(0x10, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	Printf2(0x00, "\n");

	Printf2(0x10, "%s", "[ 1. 출금부 시재 ]\n");
	Printf2(0x00, "\n");
	Printf2(0x00, "   단위          수량                     금액\n");
	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	Printf2(0x00, "    100원         %4d               %9d\n", pClsTR->currInfo.w100, COIN_100_SUM(pClsTR->currInfo.w100));
	Printf2(0x00, "    500원         %4d               %9d\n", pClsTR->currInfo.w500, COIN_500_SUM(pClsTR->currInfo.w500));
	Printf2(0x00, "  1,000원         %4d               %9d\n", pClsTR->currInfo.w1k	, BILL_1K_SUM(pClsTR->currInfo.w1k));
	Printf2(0x00, " 10,000원         %4d               %9d\n", pClsTR->currInfo.w10k, BILL_10K_SUM(pClsTR->currInfo.w10k));
	nSum[0] = COIN_SUM(0, 0, pClsTR->currInfo.w100, pClsTR->currInfo.w500) + BILL_SUM(pClsTR->currInfo.w1k, 0, pClsTR->currInfo.w10k, 0);
	Printf2(0x00, " 합    계                            %9d\n",  nSum[0]);
	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

	Printf2(0x00, "\n");
	Printf2(0x10, "%s", "[ 2. 입금부 시재 ]\n");
	Printf2(0x00, "\n");
	Printf2(0x00, "   단위          수량                     금액\n");
	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	Printf2(0x00, "  1,000원         %4d               %9d\n", pClsTR->insInfo.w1k , BILL_1K_SUM(pClsTR->insInfo.w1k));
	Printf2(0x00, "  5,000원         %4d               %9d\n", pClsTR->insInfo.w5k , BILL_5K_SUM(pClsTR->insInfo.w5k));
	Printf2(0x00, " 10,000원         %4d               %9d\n", pClsTR->insInfo.w10k, BILL_10K_SUM(pClsTR->insInfo.w10k));
	Printf2(0x00, " 50,000원         %4d               %9d\n", pClsTR->insInfo.w50k, BILL_50K_SUM(pClsTR->insInfo.w50k));
	nSum[1] = BILL_SUM(pClsTR->insInfo.w1k, pClsTR->insInfo.w5k, pClsTR->insInfo.w10k, pClsTR->insInfo.w50k);
	Printf2(0x00, " 합    계                            %9d\n", nSum[1]);
	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

	Printf2(0x00, "\n");
	Printf2(0x10, "%s", "[ 3. 보급내역 ]\n");
	Printf2(0x00, "\n");
	Printf2(0x00, "   단위          수량                     금액\n");
	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
	Printf2(0x00, "    100원         %4d               %9d\n", pClsTR->supplyInfo.w100,  COIN_100_SUM(pClsTR->supplyInfo.w100));
	Printf2(0x00, "    500원         %4d               %9d\n", pClsTR->supplyInfo.w500,  COIN_500_SUM(pClsTR->supplyInfo.w500));
	Printf2(0x00, "  1,000원         %4d               %9d\n", pClsTR->supplyInfo.w1k ,  BILL_1K_SUM(pClsTR->supplyInfo.w1k));
	Printf2(0x00, " 10,000원         %4d               %9d\n", pClsTR->supplyInfo.w10k,  BILL_10K_SUM(pClsTR->supplyInfo.w10k));
	nSum[2] = COIN_SUM(0, 0, pClsTR->supplyInfo.w100, pClsTR->supplyInfo.w500) + BILL_SUM(pClsTR->supplyInfo.w1k, 0, pClsTR->supplyInfo.w10k, 0);
	Printf2(0x00, " 합    계                            %9d\n", nSum[2]);
	Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

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

		Printf2(0x00, "\n");
		Printf2(0x10, "%s", "[ 4. 발권내역 (현금) ]\n");
		Printf2(0x00, "\n");
		Printf2(0x00, "   구분          수량                     금액\n");
		Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		Printf2(0x00, " 시외발권         %4d               %9d\n", sumT1.nCount, sumT1.dwMoney);
		Printf2(0x00, " 고속발권         %4d               %9d\n", sumT2.nCount, sumT2.dwMoney);
		Printf2(0x00, " 합    계         %4d               %9d\n", sumT1.nCount + sumT2.nCount, sumT1.dwMoney + sumT2.dwMoney);
		Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
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

		Printf2(0x00, "\n");
		Printf2(0x10, "%s", "[ 5. 환불내역 (현금) ]\n");
		Printf2(0x00, "\n");
		Printf2(0x00, "   구분          수량                     금액\n");
		Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		Printf2(0x00, " 시외환불         %4d               %9d\n", sumT1.nCount, sumT1.dwMoney);
		Printf2(0x00, " 고속환불         %4d               %9d\n", sumT2.nCount, sumT2.dwMoney);
		Printf2(0x00, " 합    계         %4d               %9d\n", sumT1.nCount + sumT2.nCount, sumT1.dwMoney + sumT2.dwMoney);
		Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

		Printf2(0x00, "\n");
		Printf2(0x10, " 결과 (1 + 2 - 3 - 4 - 5)          %9d원\n", nSum[0] + nSum[1] - nSum[2] - nSum[3] - nSum[4]);
		Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		Printf2(0x00, "\n");
	}

	PaperFullCut();
}

/**
 * @brief		Print_Refund
 * @details		환불 정보 프린트
 * @param		None
 * @return		None
 */
void CPrinterRxd::Print_Refund(void)
{
	SYSTEMTIME			st;
	prt_refund_t		prtInfo;
	POPER_FILE_CONFIG_T pConfig;

	LOG_OUT("start !!!");

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

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
	{	/// 티머니 고속
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
	****/
	Printer_MakeRefundTicketData((char *)&prtInfo);


	if(1)
	{
		Printf2(0x38, "	     %s\n", prtInfo.bus_nm);
		Printf2(0x38, "	   환불 확인증\n");
		Printf("\n");
		Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

		Printf2(0x00, "가맹점 : %s\n", prtInfo.bizr_nm);
		Printf2(0x00, "가맹점 사업자번호: %s\n", prtInfo.bizr_no);
		Printf2(0x00, "출력일시 : %04d-%02d-%02d %02d:%02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");

		Printf2(0x00, "결제수단   : %s\n",		prtInfo.pyn_dvs_nm);
		Printf2(0x00, "승차권정보 : %s\n",		prtInfo.ticket_info);
		Printf2(0x00, "발권 금액  : %s 원\n",		prtInfo.tisu_amt);
		Printf2(0x00, "수수료율   : %s%% \n",	prtInfo.cmrt);
		Printf2(0x00, "환불 금액  : \n");
		Printf2(0x38, "            %s",			prtInfo.ry_amt);
		Printf2(0x00, " 원\n");
		Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		Printf2(0x00, "출 발 일 : %s \n", prtInfo.depr_dt);
		Printf2(0x00, "출발시간 : %s \n", prtInfo.depr_time);
		Printf2(0x00, "출 발 지 : %s \n", prtInfo.depr_nm);
		Printf2(0x00, "도 착 지 : %s \n", prtInfo.arvl_nm);
		Printf2(0x00, "버스등급 : %s \n", prtInfo.bus_cls_nm);
		Printf2(0x00, "좌석번호 : %s \n", prtInfo.sat_no);
		Printf2(0x00, "총 매 수 : 1매 \n");
		Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		// 20221129 ADD~
		Printf2(0x01, "   ※ 수수료 발생 시 카드 취소 최대 7일 소요 \n");
		Printf2(0x00, "    결제일자, 취소일자, 카드종류 등에 따라 \n");
		Printf2(0x00, "  카드 취소는 최대 7일 정도 소요될 수 있습니다.\n");
		Printf2(0x00, "   자세한 사항은 카드사로 문의하시기 바랍니다. \n");
		Printf2(0x00, "━━━━━━━━━━━━━━━━━━━━━━━━\n");
		// 20221129 ~ADD
		PaperFullCut();
	}
}

int CPrinterRxd::AlignBuffer(int nMax, char *pSrc, int nSrcLen, char *retBuf, int retLen)
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

/**
 * @brief		Print_Ticket
 * @details		고속 승차권 발권
 * @param		None
 * @return		None
 */
void CPrinterRxd::Print_Ticket(int nSvrKind, int nFunction, char *pData)
{
	int					nRet, nMAX;
	char				Buffer[100];
	char				retBuf[100];
	PTCK_PRINT_FMT_T	pPrtData;
	POPER_FILE_CONFIG_T pOperConfig;
	PFILE_ACCUM_N1010_T pAcc;

	pPrtData = (PTCK_PRINT_FMT_T) pData;
	pOperConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();
	pAcc = GetAccumulateData();

	InitPrinter();

	///
	{
		Printf2(0x10, "━━━━━━━━━━━━━━━━━━━━━━━\r\n");
	}

	///
	{
		Printf2(0x00, "%s", SetSpaceData(8));
		Printf2(0x38, "고속버스 승차권");
		Printf2(0x00, "%s\r\n", SetSpaceData(8));
	}

	///
	{
		Printf2(0x00, "%s", SetSpaceData(10));
		Printf2(0x38, "Boarding Pass");
		Printf2(0x00, "%s\r\n", SetSpaceData(10));
	}

	///
	{
		Printf2(0x10, "━━━━━━━━━━━━━━━━━━━━━━━\r\n");
	}

	///
	{
		Printf2(0x00, "%s출발일시\r\n", SetSpaceData(38));
	}

	///	[정보] 출발일시
	{
		Printf2(0x00, "%s%s %s \r\n\r\n", SetSpaceData(27), pPrtData->atl_depr_dt, pPrtData->atl_depr_time);
	}
	
	/// [정보] QR Code
	{
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
		Printf2(0x00, "%s", SetSpaceData(18));
		//Printf2(0x00, "001546\r\n");
		Printf2(0x00, "%08d\r\n", pAcc->Curr.expTicket.n_bus_tck_inhr_no);
	}

	///
	{
		Printf2(0x10, "┎────────┰────────┰────┒\r\n");
	}

	///
	{
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
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(1));		///*

		// 1) 출발지(영문)
		//sprintf(Buffer, "%s", "Seoul");
		sprintf(Buffer, "%s", pPrtData->depr_trml_eng_nm);
		AlignBuffer(14, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x00, retBuf);

		Printf2(0x00, "%s", SetSpaceData(1));
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(1));

		// 2) 도착지(영문)
		//sprintf(Buffer, "%s", "Ansung");
		sprintf(Buffer, "%s", pPrtData->arvl_trml_eng_nm);
		AlignBuffer(14, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
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
		Printf2(0x10, "┖────────┸────────┸────┚\r\n");
	}

	/// 
	{
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
		Printf2(0x00, "요금");
		Printf2(0x00, "%s", SetSpaceData(14));
		//Printf2(0x38, "5,700");
		Printf2(0x38, pPrtData->tisu_amt);
		Printf2(0x00, " 원\r\n\r\n");
	}

	///
	{
		Printf2(0x10, "┎───────────┰──────────┒\r\n");
	}

	///
	{
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
		Printf2(0x10, "┠──────┰────╂─────┰────┨\r\n");
	}

	///
	{
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
		Printf2(0x10, "┠──────╂────╂─────╂────┨\r\n");
	}

	///
	{
		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x18, "운송회사");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x10, "┃");
		//Printf2(0x18, "금호고속");
		sprintf(Buffer, "%s", pPrtData->bus_cacm_nm);
		AlignBuffer(8, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x18, retBuf);

		Printf2(0x10, "┃");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x18, "승차홈");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x10, "┃");

		//sprintf(Buffer, "%s", "31234-호");
		sprintf(Buffer, "%s", pPrtData->rdhm_val);
		AlignBuffer(8, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x18, retBuf);

		Printf2(0x10, "┃\r\n");
	}

	///
	{
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
		Printf2(0x10, "┖──────┸────┸─────┸────┚\r\n");
	}

	///	승인번호, 카드정보
	{
		//Printf2(0x00, "승인번호 1234567");
		Printf2(0x00, "승인번호 %s", pPrtData->card_aprv_no);
		Printf2(0x00, "%s", SetSpaceData(7-4));
		//Printf2(0x00, "신용 1234567890123456\r\n");
		Printf2(0x00, "인증번호 %s\r\n", pPrtData->card_no);
	}

	///
	{
		//Printf2(0x00, "승인금액 34,700\r\n");
		Printf2(0x00, "승인금액 %s\r\n", pPrtData->card_aprv_amt);
	}

	/// 
	{
		//Printf2(0x18, "모바일티켓");
		sprintf(Buffer, "%s", SetSpaceData(10));
		if(nFunction == enFUNC_MRS)
		{
			sprintf(Buffer, "모바일티켓");
		}
		AlignBuffer(10, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x18, retBuf);

		Printf2(0x00, "%s", SetSpaceData(8));
		
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
			sprintf(Buffer, "복합결제");
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
		else
		{
			Printf2(0x38, "현금\r\n\r\n");
		}
	}

	///
	{
		//Printf2(0x08, "※유효기간:당일지정차에 한함  [13:06]\r\n");
		Printf2(0x08, "※유효기간:당일지정차에 한함  %s\r\n", pPrtData->pub_time);
	}

	///
	{
		Printf2(0x08, "고속회사:");
		//Printf2(0x00, "금호속리산고속(주) 301-81-00353\r\n");
		Printf2(0x00, "%s %s\r\n", pPrtData->bus_cacm_nm, pPrtData->bus_cacm_biz_no);
	}

	///	가맹점 정보
	{
		//Printf2(0x08, "(주)엔에스피 129-86-27172\r\n");
		//sprintf(Buffer, "가맹점사업자번호:%s", pOperConfig->koTrmlInfo_t.sz_prn_trml_corp_no);
		Printf2(0x08, "%s %s\r\n", pOperConfig->koTrmlInfo_t.sz_prn_trml_nm, pOperConfig->koTrmlInfo_t.sz_prn_trml_corp_no);
	}

	/// 가맹점 전화번호
	{
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

/**
 * @brief		StartProcess
 * @details		Start
 * @param		int nCommIdx		통신포트 번호
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrinterRxd::StartProcess(int nCommIdx)
{
	int		nRet;
//	DWORD	dwThreadID;

	LOG_INIT();
	m_bConnected = FALSE;
	LOG_OUT("start !!!");

	EndProcess();

	nRet = m_clsComm.Open(nCommIdx, CBR_38400, 8, NOPARITY, ONESTOPBIT);
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
 * @details		종료
 * @param		None
 * @return		항상 0
 */
int CPrinterRxd::EndProcess(void)
{
	LOG_OUT("start !!!");
	
	m_bConnected = FALSE;

	m_clsComm.Close();
	if(m_hAccMutex != NULL)
	{
		CloseHandle(m_hAccMutex);
		m_hAccMutex = NULL;
	}

	return 0;
}




