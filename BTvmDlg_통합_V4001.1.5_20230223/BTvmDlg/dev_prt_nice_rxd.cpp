// 
// 
// dev_prt_nice_rxd.cpp : и瞪旎 - REXOD Щ萼攪	(RX830-H120)
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
 * @details		儅撩濠
 */
CPrinterRxd::CPrinterRxd()
{
	m_hAccMutex = NULL;
	m_bConnected = FALSE;
}

/**
 * @brief		~CPrinterRxd
 * @details		模資濠
 */
CPrinterRxd::~CPrinterRxd()
{
	LOG_OUT(" start !! ");
	EndProcess();
}

/**
 * @brief		LOG_INIT
 * @details		LOG だ橾 蟾晦��
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
 * @return		о鼻 : 0
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
 * @return		о鼻 : 0
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
 * @details		Serial 等檜顫 瞪歎
 * @param		BYTE *pData		瞪歎 等檜顫
 * @param		int nDataLen	瞪歎 等檜顫 望檜
 * @return		瞪歎等檜顫 望檜
 */
int CPrinterRxd::SendData(BYTE *pData, int nDataLen)
{
	return m_clsComm.SendData(pData, nDataLen);
}

/**
 * @brief		GetPacket
 * @details		Serial 等檜顫 熱褐
 * @param		BYTE *retBuf	receive data
 * @return		撩奢 : > 0, 褒ぬ : < 0
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
 * @return		撩奢 : > 0, 褒ぬ : < 0
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
 * @return		о鼻 = 0
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
 * @return		撩奢 : > 0, 褒ぬ : < 0
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
 * @return		撩奢 : > 0, 褒ぬ : < 0
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
 * @return		о鼻 = 0
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
 * @return		о鼻 = 0
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
 * @details		辨雖 瞰欽
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
 * @details		Щ萼攪 鼻鷓 貲滄 瞪歎
 * @param		None
 * @return		撩奢 : > 0, 褒ぬ : < 0
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
		//{	/// 薑鼻
		//	;
		//}

		if(GetConfigTicketPapaer() == PAPER_ROLL)									// 20230206 DEL // 20230217 MOD
		//pEnv = (PDEV_CFG_T) GetEnvPrtReceiptInfo();								// 20230206 ADD // 20230217 DEL
		//if(pEnv->nUse == 1) // 艙熱隸(馬翮雖) Щ萼攪 餌辨 褫暮檜 'USE = 1'檣 唳辦		// 20230206 ADD // 20230217 DEL
		{
			SetCheckEventCode(EC_PRT_ROLL_COMM_ERR, FALSE);

			/// 辨雖橈擠
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
		if(pEnvPrtTck->nUse == 1) // 蝓離掏(謙檜) Щ萼攪 餌辨 褫暮檜 'USE = 1'檣 唳辦	// 20230206 ADD
		{
			SetCheckEventCode(EC_PRT_COMM_ERR, FALSE);

			/// 辨雖橈擠
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
	if(pEnv->nUse == 1) // 艙熱隸(馬翮雖) Щ萼攪 餌辨 褫暮檜 'USE = 1'檣 唳辦		// 20230206 ADD // 20230215 DEL
	//if( (pEnv->nUse == 1) && ( ((prev_ch & 0xFF) == false) && ((ch & 0xFF) == false) ) ) // 艙熱隸(馬翮雖) Щ萼攪 餌辨 褫暮檜 'USE = 1'檣 唳辦		// 20230206 ADD // 20230215 MOD
	{
		SetCheckEventCode(EC_PRT_ROLL_COMM_ERR, TRUE);
	}
	//else // 20230206 DEL
	pEnvPrtTck = (PDEV_CFG_T) GetEnvPrtTicketInfo();							// 20230206 ADD
	//LOG_OUT("pEnvPrtTck->nUse = %d", pEnvPrtTck->nUse);
	if(pEnvPrtTck->nUse == 1) // 蝓離掏(謙檜) Щ萼攪 餌辨 褫暮檜 'USE = 1'檣 唳辦	// 20230206 ADD
	{
		SetCheckEventCode(EC_PRT_COMM_ERR, TRUE);
	}

	UnLocking();

	return -1;
}

/**
 * @brief		Printf
 * @details		Щ萼お 等檜顫 轎溘
 * @param		const char *fmt			僥濠 ん裝
 * @return		о鼻 = 0
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
 * @details		Щ萼お 等檜顫 轎溘
 * @param		const char *fmt			僥濠 ん裝
 * @return		о鼻 = 0
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
 * @details		衛營 薑爾 Щ萼お
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
		Printf("收收收收收收收收收收收收收收收收收收收收收收收收\n");
		SetCharPrtingMode(0x38);
		Printf("	⑷旎 衛營 貲撮憮\n");
		SetCharPrtingMode(0x00);
		Printf("\n");
		
		pTrmlCode = GetTrmlCode(SVR_DVS_CCBUS);
		FindTerminalName(LANG_KOR, pTrmlCode, retBuf);
		Printf("攪 嘐 割 : %s\n", retBuf);

		Printf("璽掘廓�� : %s\n", GetTrmlWndNo(SVR_DVS_CCBUS));
		//	Printf("撲薑橾衛 : 2019-10-15 17:50:55\n");
		Printf("轎溘橾衛 : %04d-%02d-%02d %02d:%02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		SetCharPrtingMode(0x10);
		Printf("收收收收收收收收收收收收收收收收收收收收收收收收\n");
		SetCharPrtingMode(0x00);
		Printf("\n");
		Printf("\n");
		SetCharPrtingMode(0x30);
		Printf("*** 霜瞪 衛營 ***\n");
		SetCharPrtingMode(0x00);
		Printf("\n");

		nValue = (pAccum->Prev.BillBox.n50k * 50000) + (pAccum->Prev.BillBox.n10k * 10000) + (pAccum->Prev.BillBox.n5k * 5000) + (pAccum->Prev.BillBox.n1k * 1000);

		SetCharPrtingMode(0x10);
//		Printf("殮旎睡 識擋 : 75,000 錳\n");
		Printf("殮旎睡 識擋 : %d 錳\n", nValue);
		SetCharPrtingMode(0x00);
		Printf("\n");
		Printf("   欽嬪          熱榆                     旎擋\n");
		Printf("收收收收收收收收收收收收收收收收收收收收收收收收\n");
		Printf("   5虜錳          %4d               %9d\n", pAccum->Prev.BillBox.n50k, pAccum->Prev.BillBox.n50k * 50000);
		Printf("   1虜錳          %4d               %9d\n", pAccum->Prev.BillBox.n10k, pAccum->Prev.BillBox.n10k * 10000);
		Printf("   5繭錳          %4d               %9d\n", pAccum->Prev.BillBox.n5k, pAccum->Prev.BillBox.n5k * 5000);
		Printf("   1繭錳          %4d               %9d\n", pAccum->Prev.BillBox.n1k, pAccum->Prev.BillBox.n1k * 1000);
		Printf("收收收收收收收收收收收收收收收收收收收收收收收收\n");
		Printf("\n");

		nValue = (pAccum->Prev.BillDispenser.Casst.n10k * 10000) + (pAccum->Prev.BillDispenser.Casst.n1k * 1000) + (pAccum->Prev.Coin.n500 * 500) + (pAccum->Prev.Coin.n100 * 100);

		SetCharPrtingMode(0x10);
//		Printf("轎旎睡 識擋 : 5,798,000 錳\n");
		Printf("轎旎睡 識擋 : %d 錳\n", nValue);
		SetCharPrtingMode(0x00);
		Printf("\n");
		Printf("   欽嬪          熱榆                     旎擋\n");
		Printf("收收收收收收收收收收收收收收收收收收收收收收收收\n");
		Printf("   1虜錳          %4d               %9d\n", pAccum->Prev.BillDispenser.Casst.n10k, pAccum->Prev.BillDispenser.Casst.n10k * 10000);
		Printf("   1繭錳          %4d               %9d\n", pAccum->Prev.BillDispenser.Casst.n1k,  pAccum->Prev.BillDispenser.Casst.n1k * 1000);
		Printf("   5寥錳          %4d               %9d\n", pAccum->Prev.Coin.n500				,  pAccum->Prev.Coin.n500 * 500);
		Printf("   1寥錳          %4d               %9d\n", pAccum->Prev.Coin.n100				,  pAccum->Prev.Coin.n100 * 100);
		Printf("收收收收收收收收收收收收收收收收收收收收收收收收\n");
		Printf("\n");
		Printf("\n");
		SetCharPrtingMode(0x30);

		///////////////////////////////////////////////////////////////////////////////////////////

		Printf("*** ⑷營 衛營 ***\n");
		SetCharPrtingMode(0x00);
		Printf("\n");

		nValue = (pAccum->Curr.BillBox.n50k * 50000) + (pAccum->Curr.BillBox.n10k * 10000) + (pAccum->Curr.BillBox.n5k * 5000) + (pAccum->Curr.BillBox.n1k * 1000);

		SetCharPrtingMode(0x10);
//		Printf("殮旎睡 識擋 : 88,000 錳\n");
		Printf("殮旎睡 識擋 : %d 錳\n", nValue);
		SetCharPrtingMode(0x00);
		Printf("\n");
		Printf("   欽嬪          熱榆                     旎擋\n");
		Printf("收收收收收收收收收收收收收收收收收收收收收收收收\n");
//		Printf("   5虜錳             1                   50,000\n");
		Printf("   5虜錳          %4d               %9d\n", pAccum->Curr.BillBox.n50k, pAccum->Curr.BillBox.n50k * 50000);
		Printf("   1虜錳          %4d               %9d\n", pAccum->Curr.BillBox.n10k, pAccum->Curr.BillBox.n10k * 10000);
		Printf("   5繭錳          %4d               %9d\n", pAccum->Curr.BillBox.n5k, pAccum->Curr.BillBox.n5k * 5000);
		Printf("   1繭錳          %4d               %9d\n", pAccum->Curr.BillBox.n1k, pAccum->Curr.BillBox.n1k * 1000);
		Printf("收收收收收收收收收收收收收收收收收收收收收收收收\n");
		Printf("\n");
		
		nValue = (pAccum->Curr.BillDispenser.Casst.n10k * 10000) + (pAccum->Curr.BillDispenser.Casst.n1k * 1000) + (pAccum->Curr.Coin.n500 * 500) + (pAccum->Curr.Coin.n100 * 100);

		SetCharPrtingMode(0x10);
//		Printf("   轎旎睡 識擋 : 5,798,000 錳\n");
		Printf("轎旎睡 識擋 : %d 錳\n", nValue);
		SetCharPrtingMode(0x00);
		Printf("\n");
		Printf("   欽嬪          熱榆                     旎擋\n");
		Printf("收收收收收收收收收收收收收收收收收收收收收收收收\n");
//		Printf("   1虜錳           500                5,000,000\n");
		Printf("   1虜錳          %4d               %9d\n", pAccum->Curr.BillDispenser.Casst.n10k, pAccum->Curr.BillDispenser.Casst.n10k * 10000);
		Printf("   1繭錳          %4d               %9d\n", pAccum->Curr.BillDispenser.Casst.n1k, pAccum->Curr.BillDispenser.Casst.n1k * 1000);
		Printf("   5寥錳          %4d               %9d\n", pAccum->Curr.Coin.n500, pAccum->Curr.Coin.n500 * 500);
		Printf("   1寥錳          %4d               %9d\n", pAccum->Curr.Coin.n100, pAccum->Curr.Coin.n100 * 100);
		Printf("收收收收收收收收收收收收收收收收收收收收收收收收\n");
		Printf("\n");
		Printf("\n");
		Printf("   憮   貲  : ________________________________\n");
		Printf("\n");
		Printf("	衛營貲撮蒂 �挫恛醴� 憮貲夥奧棲棻.\n");


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

	Printf2(0x10, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
	Printf2(0x38, "	葆馬 頂羲 貲撮憮\n");
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

	Printf2(0x00, "攪 嘐 割 : %s\n", retBuf);
	Printf2(0x00, "璽掘廓�� : %s\n", pWndNo);
	//	Printf("撲薑橾衛 : 2019-10-15 17:50:55\n");
	Printf2(0x00, "轎溘橾衛 : %04d-%02d-%02d %02d:%02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	Printf2(0x10, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
	Printf2(0x00, "\n");

	Printf2(0x10, "%s", "[ 1. 轎旎睡 衛營 ]\n");
	Printf2(0x00, "\n");
	Printf2(0x00, "   欽嬪          熱榆                     旎擋\n");
	Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
	Printf2(0x00, "    100錳         %4d               %9d\n", pCoin->n100, COIN_100_SUM(pCoin->n100));
	Printf2(0x00, "    500錳         %4d               %9d\n", pCoin->n500, COIN_500_SUM(pCoin->n500));
	Printf2(0x00, "  1,000錳         %4d               %9d\n", pCDU->n1k, BILL_1K_SUM(pCDU->n1k));
	Printf2(0x00, " 10,000錳         %4d               %9d\n", pCDU->n10k, BILL_10K_SUM(pCDU->n10k));
	nSum[0] = COIN_SUM(0, 0, pAccum->Curr.Coin.n100, pAccum->Curr.Coin.n500) + BILL_SUM(pCDU->n1k, 0, pCDU->n10k, 0);
	Printf2(0x00, " м    啗                            %9d\n",  nSum[0]);
	Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");

	Printf2(0x00, "\n");
	Printf2(0x10, "%s", "[ 2. 殮旎睡 衛營 ]\n");
	Printf2(0x00, "\n");
	Printf2(0x00, "   欽嬪          熱榆                     旎擋\n");
	Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
	Printf2(0x00, "  1,000錳         %4d               %9d\n", pBillBox->n1k, BILL_1K_SUM(pBillBox->n1k));
	Printf2(0x00, "  5,000錳         %4d               %9d\n", pBillBox->n5k, BILL_5K_SUM(pBillBox->n5k));
	Printf2(0x00, " 10,000錳         %4d               %9d\n", pBillBox->n10k, BILL_10K_SUM(pBillBox->n10k));
	Printf2(0x00, " 50,000錳         %4d               %9d\n", pBillBox->n50k, BILL_50K_SUM(pBillBox->n50k));
	nSum[1] = BILL_SUM(pBillBox->n1k, pBillBox->n5k, pBillBox->n10k, pBillBox->n50k);
	Printf2(0x00, " м    啗                            %9d\n", nSum[1]);
	Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");

	Printf2(0x00, "\n");
	Printf2(0x10, "%s", "[ 3. 爾晝頂羲 ]\n");
	Printf2(0x00, "\n");
	Printf2(0x00, "   欽嬪          熱榆                     旎擋\n");
	Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
	Printf2(0x00, "    100錳         %4d               %9d\n", pAccum->Close.supplyCoin.n100, COIN_100_SUM(pAccum->Close.supplyCoin.n100));
	Printf2(0x00, "    500錳         %4d               %9d\n", pAccum->Close.supplyCoin.n500, COIN_500_SUM(pAccum->Close.supplyCoin.n500));
	Printf2(0x00, "  1,000錳         %4d               %9d\n", pAccum->Close.supplyBill.n1k,  BILL_1K_SUM(pAccum->Close.supplyBill.n1k));
	Printf2(0x00, " 10,000錳         %4d               %9d\n", pAccum->Close.supplyBill.n10k,  BILL_10K_SUM(pAccum->Close.supplyBill.n10k));
	nSum[2] = COIN_SUM(0, 0, pAccum->Close.supplyCoin.n100, pAccum->Close.supplyCoin.n500) + BILL_SUM(pAccum->Close.supplyBill.n1k, 0, pAccum->Close.supplyBill.n10k, 0);
	Printf2(0x00, " м    啗                            %9d\n", nSum[2]);
	Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");

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
		Printf2(0x10, "%s", "[ 4. 嫦掏頂羲 (⑷旎) ]\n");
		Printf2(0x00, "\n");
		Printf2(0x00, "   掘碟          熱榆                     旎擋\n");
		Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
		Printf2(0x00, " 衛諼嫦掏         %4d               %9d\n", sumT1.nCount, sumT1.dwMoney);
		Printf2(0x00, " 堅樓嫦掏         %4d               %9d\n", sumT2.nCount, sumT2.dwMoney);
		Printf2(0x00, " м    啗         %4d               %9d\n", sumT1.nCount + sumT2.nCount, sumT1.dwMoney + sumT2.dwMoney);
		Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
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
		Printf2(0x10, "%s", "[ 5. �笑珜遛� (⑷旎) ]\n");
		Printf2(0x00, "\n");
		Printf2(0x00, "   掘碟          熱榆                     旎擋\n");
		Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
		Printf2(0x00, " 衛諼�笑�         %4d               %9d\n", sumT1.nCount, sumT1.dwMoney);
		Printf2(0x00, " 堅樓�笑�         %4d               %9d\n", sumT2.nCount, sumT2.dwMoney);
		Printf2(0x00, " м    啗         %4d               %9d\n", sumT1.nCount + sumT2.nCount, sumT1.dwMoney + sumT2.dwMoney);
		Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");

		Printf2(0x00, "\n");
		Printf2(0x10, " 唸婁 (1 + 2 - 3 - 4 - 5)          %9d錳\n", nSum[0] + nSum[1] - nSum[2] - nSum[3] - nSum[4]);
		Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
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
		TR_LOG_OUT("衛營葆馬 Щ萼お 等檜顫 瓊晦 褒ぬ...");
		return;
	}

	Printf2(0x10, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
	Printf2(0x38, "	衛營葆馬 頂羲 貲撮憮\n");
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

	Printf2(0x00, "攪 嘐 割 : %s\n", retBuf);
	Printf2(0x00, "璽掘廓�� : %s\n", pWndNo);
	//	Printf("撲薑橾衛 : 2019-10-15 17:50:55\n");
	Printf2(0x00, "葆馬橾衛 : %04d-%02d-%02d %02d:%02d:%02d\n", stFile.wYear, stFile.wMonth, stFile.wDay, stFile.wHour, stFile.wMinute, stFile.wSecond);
	Printf2(0x00, "轎溘橾衛 : %04d-%02d-%02d %02d:%02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	Printf2(0x10, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
	Printf2(0x00, "\n");

	Printf2(0x10, "%s", "[ 1. 轎旎睡 衛營 ]\n");
	Printf2(0x00, "\n");
	Printf2(0x00, "   欽嬪          熱榆                     旎擋\n");
	Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
	Printf2(0x00, "    100錳         %4d               %9d\n", pClsTR->currInfo.w100, COIN_100_SUM(pClsTR->currInfo.w100));
	Printf2(0x00, "    500錳         %4d               %9d\n", pClsTR->currInfo.w500, COIN_500_SUM(pClsTR->currInfo.w500));
	Printf2(0x00, "  1,000錳         %4d               %9d\n", pClsTR->currInfo.w1k	, BILL_1K_SUM(pClsTR->currInfo.w1k));
	Printf2(0x00, " 10,000錳         %4d               %9d\n", pClsTR->currInfo.w10k, BILL_10K_SUM(pClsTR->currInfo.w10k));
	nSum[0] = COIN_SUM(0, 0, pClsTR->currInfo.w100, pClsTR->currInfo.w500) + BILL_SUM(pClsTR->currInfo.w1k, 0, pClsTR->currInfo.w10k, 0);
	Printf2(0x00, " м    啗                            %9d\n",  nSum[0]);
	Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");

	Printf2(0x00, "\n");
	Printf2(0x10, "%s", "[ 2. 殮旎睡 衛營 ]\n");
	Printf2(0x00, "\n");
	Printf2(0x00, "   欽嬪          熱榆                     旎擋\n");
	Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
	Printf2(0x00, "  1,000錳         %4d               %9d\n", pClsTR->insInfo.w1k , BILL_1K_SUM(pClsTR->insInfo.w1k));
	Printf2(0x00, "  5,000錳         %4d               %9d\n", pClsTR->insInfo.w5k , BILL_5K_SUM(pClsTR->insInfo.w5k));
	Printf2(0x00, " 10,000錳         %4d               %9d\n", pClsTR->insInfo.w10k, BILL_10K_SUM(pClsTR->insInfo.w10k));
	Printf2(0x00, " 50,000錳         %4d               %9d\n", pClsTR->insInfo.w50k, BILL_50K_SUM(pClsTR->insInfo.w50k));
	nSum[1] = BILL_SUM(pClsTR->insInfo.w1k, pClsTR->insInfo.w5k, pClsTR->insInfo.w10k, pClsTR->insInfo.w50k);
	Printf2(0x00, " м    啗                            %9d\n", nSum[1]);
	Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");

	Printf2(0x00, "\n");
	Printf2(0x10, "%s", "[ 3. 爾晝頂羲 ]\n");
	Printf2(0x00, "\n");
	Printf2(0x00, "   欽嬪          熱榆                     旎擋\n");
	Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
	Printf2(0x00, "    100錳         %4d               %9d\n", pClsTR->supplyInfo.w100,  COIN_100_SUM(pClsTR->supplyInfo.w100));
	Printf2(0x00, "    500錳         %4d               %9d\n", pClsTR->supplyInfo.w500,  COIN_500_SUM(pClsTR->supplyInfo.w500));
	Printf2(0x00, "  1,000錳         %4d               %9d\n", pClsTR->supplyInfo.w1k ,  BILL_1K_SUM(pClsTR->supplyInfo.w1k));
	Printf2(0x00, " 10,000錳         %4d               %9d\n", pClsTR->supplyInfo.w10k,  BILL_10K_SUM(pClsTR->supplyInfo.w10k));
	nSum[2] = COIN_SUM(0, 0, pClsTR->supplyInfo.w100, pClsTR->supplyInfo.w500) + BILL_SUM(pClsTR->supplyInfo.w1k, 0, pClsTR->supplyInfo.w10k, 0);
	Printf2(0x00, " м    啗                            %9d\n", nSum[2]);
	Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");

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
		Printf2(0x10, "%s", "[ 4. 嫦掏頂羲 (⑷旎) ]\n");
		Printf2(0x00, "\n");
		Printf2(0x00, "   掘碟          熱榆                     旎擋\n");
		Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
		Printf2(0x00, " 衛諼嫦掏         %4d               %9d\n", sumT1.nCount, sumT1.dwMoney);
		Printf2(0x00, " 堅樓嫦掏         %4d               %9d\n", sumT2.nCount, sumT2.dwMoney);
		Printf2(0x00, " м    啗         %4d               %9d\n", sumT1.nCount + sumT2.nCount, sumT1.dwMoney + sumT2.dwMoney);
		Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
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
		Printf2(0x10, "%s", "[ 5. �笑珜遛� (⑷旎) ]\n");
		Printf2(0x00, "\n");
		Printf2(0x00, "   掘碟          熱榆                     旎擋\n");
		Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
		Printf2(0x00, " 衛諼�笑�         %4d               %9d\n", sumT1.nCount, sumT1.dwMoney);
		Printf2(0x00, " 堅樓�笑�         %4d               %9d\n", sumT2.nCount, sumT2.dwMoney);
		Printf2(0x00, " м    啗         %4d               %9d\n", sumT1.nCount + sumT2.nCount, sumT1.dwMoney + sumT2.dwMoney);
		Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");

		Printf2(0x00, "\n");
		Printf2(0x10, " 唸婁 (1 + 2 - 3 - 4 - 5)          %9d錳\n", nSum[0] + nSum[1] - nSum[2] - nSum[3] - nSum[4]);
		Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
		Printf2(0x00, "\n");
	}

	PaperFullCut();
}

/**
 * @brief		Print_Refund
 * @details		�笑� 薑爾 Щ萼お
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
	{	/// 衛諼幗蝶
		CCancRyTkMem* pCancTr;
		PFMT_BARCODE_T pBarCode;

		pCancTr = CCancRyTkMem::GetInstance();
		pBarCode = (PFMT_BARCODE_T) pCancTr->szTicketData;

		sprintf(prtInfo.bus_nm, "衛諼幗蝶");

		/// 陛裊薄 檜葷
		sprintf(prtInfo.bizr_nm, pConfig->ccTrmlInfo_t.sz_prn_trml_nm);
		/// 陛裊薄 餌機濠廓��
		sprintf(prtInfo.bizr_no, pConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);

		/// 唸薯熱欽
		switch( pCancTr->tRespTckNo.pyn_mns_dvs_cd[0] )
		{
		case PYM_CD_CASH :
			sprintf(prtInfo.pyn_dvs_nm, "⑷旎");
			break;
		case PYM_CD_CSRC :
			sprintf(prtInfo.pyn_dvs_nm, "⑷旎艙熱隸");
			break;
		case PYM_CD_CARD :
			sprintf(prtInfo.pyn_dvs_nm, "褐辨蘋萄");
			break;
		}

		/// 蝓離掏薑爾
		sprintf(prtInfo.ticket_info, "%.*s-%.*s-%.*s-%.*s-%.*s ", 
			sizeof(pBarCode->pub_dt),			pBarCode->pub_dt, 
			sizeof(pBarCode->pub_shct_trml_cd), pBarCode->pub_shct_trml_cd, 
			sizeof(pBarCode->pub_wnd_no),		pBarCode->pub_wnd_no, 
			sizeof(pBarCode->pub_sno),			pBarCode->pub_sno, 
			sizeof(pBarCode->secu_code),		pBarCode->secu_code);

		/// 嫦掏旎擋
		sprintf(prtInfo.tisu_amt, "%d", *(int *)pCancTr->tRespTckNo.tisu_amt);

		/// �笑甡�
		sprintf(prtInfo.cmrt	, "%d", pCancTr->n_commission_rate);

		/// �笑珣旓�
		sprintf(prtInfo.ry_amt	, "%d", pCancTr->n_chg_money);

		/// 轎嫦橾
		sprintf(prtInfo.depr_dt	, "%.*s-%.*s-%.*s", 
				4, &pCancTr->tRespTckNo.depr_dt[0], 
				2, &pCancTr->tRespTckNo.depr_dt[4], 
				2, &pCancTr->tRespTckNo.depr_dt[6]);

		/// 轎嫦衛除
		sprintf(prtInfo.depr_time	, "%.*s:%.*s", 
				2, &pCancTr->tRespTckNo.depr_time[0], 
				2, &pCancTr->tRespTckNo.depr_time[2]);

		/// 轎嫦雖
		FindTerminalName(LANG_KOR, pCancTr->tRespTckNo.depr_trml_cd, prtInfo.depr_nm);

		/// 紫雜雖
		FindTerminalName(LANG_KOR, pCancTr->tRespTckNo.arvl_trml_cd, prtInfo.arvl_nm);

		/// 幗蝶蛔晝
		FindBusClsName(SVR_DVS_CCBUS, pCancTr->tRespTckNo.bus_cls_cd, prtInfo.bus_cls_nm);

		/// 謝戮廓��
		sprintf(prtInfo.sat_no	, "%d", *(int *)pCancTr->tRespTckNo.sats_no);
	}
	else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_KOBUS )
	{	/// 囀幗蝶 堅樓
		CCancRyTkKobusMem* pCancTr;
		PFMT_QRCODE_T pQRCode;

		pCancTr = CCancRyTkKobusMem::GetInstance();

		pQRCode = (PFMT_QRCODE_T) pCancTr->tBase.szTicketData;

		sprintf(prtInfo.bus_nm, "堅樓幗蝶");

		/// 陛裊薄 檜葷
		sprintf(prtInfo.bizr_nm, pConfig->koTrmlInfo_t.sz_prn_trml_nm);
		/// 陛裊薄 餌機濠廓��
		sprintf(prtInfo.bizr_no, pConfig->koTrmlInfo_t.sz_prn_trml_corp_no);

		/// 唸薯熱欽
		switch( pCancTr->tBase.ui_pym_dvs_cd[0] )
		{
		case 1 :
			sprintf(prtInfo.pyn_dvs_nm, "⑷旎");
			break;
		case 2 :
			sprintf(prtInfo.pyn_dvs_nm, "褐辨蘋萄");
			break;
		}

		/// 蝓離掏薑爾
		sprintf(prtInfo.ticket_info, "%.*s-%.*s-%.*s-%.*s", 
			sizeof(pQRCode->pub_dt),			pQRCode->pub_dt, 
			sizeof(pQRCode->pub_shct_trml_cd),  pQRCode->pub_shct_trml_cd, 
			sizeof(pQRCode->pub_wnd_no),		pQRCode->pub_wnd_no, 
			sizeof(pQRCode->pub_sno),			pQRCode->pub_sno);

		/// 嫦掏旎擋
		sprintf(prtInfo.tisu_amt, "%d", *(int *)pCancTr->tRespInq.tissu_fee);

		/// �笑甡�
		sprintf(prtInfo.cmrt	, "%d", pCancTr->tBase.n_commission_rate);

		/// �笑珣旓�
		sprintf(prtInfo.ry_amt	, "%d", pCancTr->tBase.n_chg_money);

		/// 轎嫦橾
		sprintf(prtInfo.depr_dt	, "%.*s-%.*s-%.*s", 
			4, &pCancTr->tRespList.depr_dt[0], 
			2, &pCancTr->tRespList.depr_dt[4], 
			2, &pCancTr->tRespList.depr_dt[6]);

		/// 轎嫦衛除
		sprintf(prtInfo.depr_time	, "%.*s:%.*s", 
			2, &pCancTr->tRespList.depr_time[0], 
			2, &pCancTr->tRespList.depr_time[2]);

		/// 轎嫦雖
		Find_KobusTrmlName(LANG_KOR, pCancTr->tRespList.depr_trml_no, prtInfo.depr_nm);

		/// 紫雜雖
		Find_KobusTrmlName(LANG_KOR, pCancTr->tRespList.arvl_trml_no, prtInfo.arvl_nm);

		/// 幗蝶蛔晝
		FindBusClsName(SVR_DVS_KOBUS, pCancTr->tRespList.bus_cls_cd, prtInfo.bus_cls_nm);

		/// 謝戮廓��
		sprintf(prtInfo.sat_no	, "%s", pCancTr->tRespList.sats_no);
	}
	else 
	{	/// じ該棲 堅樓
		CCancRyTkTmExpMem* pCancTr;
		PFMT_QRCODE_T pQRCode;

		pCancTr = CCancRyTkTmExpMem::GetInstance();

		pQRCode = (PFMT_QRCODE_T) pCancTr->tBase.szTicketData;

		sprintf(prtInfo.bus_nm, "堅樓幗蝶");

		/// 陛裊薄 檜葷
		sprintf(prtInfo.bizr_nm, pConfig->ezTrmlInfo_t.sz_prn_trml_nm);
		/// 陛裊薄 餌機濠廓��
		sprintf(prtInfo.bizr_no, pConfig->ezTrmlInfo_t.sz_prn_trml_corp_no);

		/// 唸薯熱欽
		switch( pCancTr->tBase.ui_pym_dvs_cd[0] )
		{
		case 1 :
			sprintf(prtInfo.pyn_dvs_nm, "⑷旎");
			break;
		case 2 :
			sprintf(prtInfo.pyn_dvs_nm, "褐辨蘋萄");
			break;
		}

		/// 蝓離掏薑爾
		sprintf(prtInfo.ticket_info, "%.*s-%.*s-%.*s-%.*s", 
			sizeof(pQRCode->pub_dt),			pQRCode->pub_dt, 
			sizeof(pQRCode->pub_shct_trml_cd),  pQRCode->pub_shct_trml_cd, 
			sizeof(pQRCode->pub_wnd_no),		pQRCode->pub_wnd_no, 
			sizeof(pQRCode->pub_sno),			pQRCode->pub_sno);

		/// 嫦掏旎擋
		sprintf(prtInfo.tisu_amt, "%d", *(int *)pCancTr->tRespInq.tissu_fee);

		/// �笑甡�
		sprintf(prtInfo.cmrt	, "%d", pCancTr->tBase.n_commission_rate);

		/// �笑珣旓�
		sprintf(prtInfo.ry_amt	, "%d", pCancTr->tBase.n_chg_money);

		/// 轎嫦橾
		sprintf(prtInfo.depr_dt	, "%.*s-%.*s-%.*s", 
			4, &pCancTr->tRespList.depr_dt[0], 
			2, &pCancTr->tRespList.depr_dt[4], 
			2, &pCancTr->tRespList.depr_dt[6]);

		/// 轎嫦衛除
		sprintf(prtInfo.depr_time	, "%.*s:%.*s", 
			2, &pCancTr->tRespList.depr_time[0], 
			2, &pCancTr->tRespList.depr_time[2]);

		/// 轎嫦雖
		Find_KobusTrmlName(LANG_KOR, pCancTr->tRespList.depr_trml_no, prtInfo.depr_nm);

		/// 紫雜雖
		Find_KobusTrmlName(LANG_KOR, pCancTr->tRespList.arvl_trml_no, prtInfo.arvl_nm);

		/// 幗蝶蛔晝
		FindBusClsName(SVR_DVS_KOBUS, pCancTr->tRespList.bus_cls_cd, prtInfo.bus_cls_nm);

		/// 謝戮廓��
		sprintf(prtInfo.sat_no	, "%s", pCancTr->tRespList.sats_no);
	}
	****/
	Printer_MakeRefundTicketData((char *)&prtInfo);


	if(1)
	{
		Printf2(0x38, "	     %s\n", prtInfo.bus_nm);
		Printf2(0x38, "	   �笑� �挫恔騱n");
		Printf("\n");
		Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");

		Printf2(0x00, "陛裊薄 : %s\n", prtInfo.bizr_nm);
		Printf2(0x00, "陛裊薄 餌機濠廓��: %s\n", prtInfo.bizr_no);
		Printf2(0x00, "轎溘橾衛 : %04d-%02d-%02d %02d:%02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");

		Printf2(0x00, "唸薯熱欽   : %s\n",		prtInfo.pyn_dvs_nm);
		Printf2(0x00, "蝓離掏薑爾 : %s\n",		prtInfo.ticket_info);
		Printf2(0x00, "嫦掏 旎擋  : %s 錳\n",		prtInfo.tisu_amt);
		Printf2(0x00, "熱熱猿徽   : %s%% \n",	prtInfo.cmrt);
		Printf2(0x00, "�笑� 旎擋  : \n");
		Printf2(0x38, "            %s",			prtInfo.ry_amt);
		Printf2(0x00, " 錳\n");
		Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
		Printf2(0x00, "轎 嫦 橾 : %s \n", prtInfo.depr_dt);
		Printf2(0x00, "轎嫦衛除 : %s \n", prtInfo.depr_time);
		Printf2(0x00, "轎 嫦 雖 : %s \n", prtInfo.depr_nm);
		Printf2(0x00, "紫 雜 雖 : %s \n", prtInfo.arvl_nm);
		Printf2(0x00, "幗蝶蛔晝 : %s \n", prtInfo.bus_cls_nm);
		Printf2(0x00, "謝戮廓�� : %s \n", prtInfo.sat_no);
		Printf2(0x00, "識 衙 熱 : 1衙 \n");
		Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
		// 20221129 ADD~
		Printf2(0x01, "   ≦ 熱熱猿 嫦儅 衛 蘋萄 鏃模 譆渠 7橾 模蹂 \n");
		Printf2(0x00, "    唸薯橾濠, 鏃模橾濠, 蘋萄謙盟 蛔縑 評塭 \n");
		Printf2(0x00, "  蘋萄 鏃模朝 譆渠 7橾 薑紫 模蹂腆 熱 氈蝗棲棻.\n");
		Printf2(0x00, "   濠撮и 餌о擎 蘋萄餌煎 僥曖ж衛晦 夥奧棲棻. \n");
		Printf2(0x00, "收收收收收收收收收收收收收收收收收收收收收收收收\n");
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

	/// 擅 space
	nFront = (nMax - nSrcLen) / 2;

	/// 菴 space/
	nRear = nFront;
	if( (nMax - nSrcLen) % 2 )
	{
		nRear++;	
	}

	/// 褒薯 data 盪濰
	/// 1. 擅 space buffer
	if(nFront > 0)
	{
		CopyMemory(&retBuf[nOffset], spBuf, nFront);
		nOffset += nFront;
	}

	/// 2. data 犒餌
	if(nSrcLen > 0)
	{
		CopyMemory(&retBuf[nOffset], pSrc, nSrcLen);
		nOffset += nSrcLen;
	}

	/// 3. 菴 space buffer
	if(nRear > 0)
	{
		CopyMemory(&retBuf[nOffset], spBuf, nRear);
		nOffset += nRear;
	}

	return nOffset;
}

/**
 * @brief		Print_Ticket
 * @details		堅樓 蝓離掏 嫦掏
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
		Printf2(0x10, "收收收收收收收收收收收收收收收收收收收收收收收\r\n");
	}

	///
	{
		Printf2(0x00, "%s", SetSpaceData(8));
		Printf2(0x38, "堅樓幗蝶 蝓離掏");
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
		Printf2(0x10, "收收收收收收收收收收收收收收收收收收收收收收收\r\n");
	}

	///
	{
		Printf2(0x00, "%s轎嫦橾衛\r\n", SetSpaceData(38));
	}

	///	[薑爾] 轎嫦橾衛
	{
		Printf2(0x00, "%s%s %s \r\n\r\n", SetSpaceData(27), pPrtData->atl_depr_dt, pPrtData->atl_depr_time);
	}
	
	/// [薑爾] QR Code
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

	/// [薑爾] QR text
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

	/// じ鰍堅嶸廓��
	{
		Printf2(0x00, "%s", SetSpaceData(18));
		//Printf2(0x00, "001546\r\n");
		Printf2(0x00, "%08d\r\n", pAcc->Curr.expTicket.n_bus_tck_inhr_no);
	}

	///
	{
		Printf2(0x10, "灰式式式式式式式式汗式式式式式式式式汗式式式式汐\r\n");
	}

	///
	{
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(1));		///*

		// 1) 轎嫦雖(и旋)
		nMAX = 14;
		//sprintf(Buffer, "%s", "憮選唳睡");
		sprintf(Buffer, "%s", pPrtData->depr_trml_ko_nm);
		nRet = IsHangul(Buffer, nMAX);
		if(nRet < 0)
		{
			Buffer[nMAX - 1] = 0;
		}
		AlignBuffer(nMAX, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x18, retBuf);

		Printf2(0x00, "%s", SetSpaceData(1));
		Printf2(0x10, "早");

		// 2) 紫雜雖(и旋)
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
		Printf2(0x10, "早");
		
		// 3) 幗蝶蛔晝
		nMAX = 8;
		//sprintf(Buffer, "%s", "Щ葬");
		sprintf(Buffer, "%s", pPrtData->bus_cls_nm);
		nRet = IsHangul(Buffer, nMAX);
		if(nRet < 0)
		{
			Buffer[nMAX - 1] = 0;
		}
		AlignBuffer(nMAX, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x18, retBuf);

		Printf2(0x10, "早\r\n");						///~*
	}

	///
	{
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(1));		///*

		// 1) 轎嫦雖(艙僥)
		//sprintf(Buffer, "%s", "Seoul");
		sprintf(Buffer, "%s", pPrtData->depr_trml_eng_nm);
		AlignBuffer(14, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x00, retBuf);

		Printf2(0x00, "%s", SetSpaceData(1));
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(1));

		// 2) 紫雜雖(艙僥)
		//sprintf(Buffer, "%s", "Ansung");
		sprintf(Buffer, "%s", pPrtData->arvl_trml_eng_nm);
		AlignBuffer(14, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x00, retBuf);
		
		Printf2(0x00, "%s", SetSpaceData(1));
		Printf2(0x10, "早");

		// 3) 蝓離掏謙盟
		nMAX = 8;
		//sprintf(Buffer, "%s", "橾奩");
		sprintf(Buffer, "%s", pPrtData->bus_tck_knd_nm);
		nRet = IsHangul(Buffer, nMAX);
		if(nRet < 0)
		{
			Buffer[nMAX - 1] = 0;
		}
		AlignBuffer(nMAX, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x00, retBuf);

		Printf2(0x10, "早\r\n");
	}

	/// 
	{
		Printf2(0x10, "汍式式式式式式式式江式式式式式式式式江式式式式污\r\n");
	}

	/// 
	{
		//Printf2(0x00, "憮選唳睡 11:30嫦 / 寰撩ч");
		//sprintf(Buffer, "%s %s嫦 / %sч   %s km\r\n\r\n", 
		//		pPrtData->depr_trml_ko_nm, 
		//		pPrtData->atl_depr_time, 
		//		pPrtData->arvl_trml_ko_nm, 
		//		pPrtData->dist);
		sprintf(Buffer, "%s %s嫦 / %sч   %s km\r\n\r\n", 
				pPrtData->alcn_depr_trml_ko_nm, 
				pPrtData->alcn_depr_time, 
				pPrtData->alcn_arvl_trml_ko_nm, 
				pPrtData->dist);
		Printf2(0x00, Buffer);
	}

	/// 
	{
		Printf2(0x00, "蹂旎");
		Printf2(0x00, "%s", SetSpaceData(14));
		//Printf2(0x38, "5,700");
		Printf2(0x38, pPrtData->tisu_amt);
		Printf2(0x00, " 錳\r\n\r\n");
	}

	///
	{
		Printf2(0x10, "灰式式式式式式式式式式式汗式式式式式式式式式式汐\r\n");
	}

	///
	{
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(8));
		Printf2(0x18, "轎嫦橾");
		Printf2(0x00, "%s", SetSpaceData(8));
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(3));
		//Printf2(0x18, "2018.05.02 熱");
		Printf2(0x18, pPrtData->atl_depr_dt);
		Printf2(0x00, "%s", SetSpaceData(4));
		Printf2(0x10, "早\r\n");
	}

	/// 
	{
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x08, "(Date of Depature)");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(20));
		Printf2(0x10, "早\r\n");
	}

	/// 
	{
		Printf2(0x10, "朵式式式式式式汗式式式式池式式式式式汗式式式式此\r\n");
	}

	///
	{
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x18, "轎嫦衛陝");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(1));
		//Printf2(0x18, "11");
		//Printf2(0x30, ":");
		//Printf2(0x18, "30");
		Printf2(0x18, "%c%c", pPrtData->atl_depr_time[0], pPrtData->atl_depr_time[1]);
		Printf2(0x30, "%c", pPrtData->atl_depr_time[2]);
		Printf2(0x18, "%c%c", pPrtData->atl_depr_time[3], pPrtData->atl_depr_time[4]);

		Printf2(0x00, "%s", SetSpaceData(1));
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(3));
		Printf2(0x18, "謝戮");
		Printf2(0x00, "%s", SetSpaceData(3));
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(3));
		//Printf2(0x18, "02");
		sprintf(Buffer, "%s", pPrtData->sats_no);
		AlignBuffer(8, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x18, Buffer);
		Printf2(0x00, "%s", SetSpaceData(3));
		Printf2(0x10, "早\r\n");
	}

	/// 
	{
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(3));
		Printf2(0x08, "(Time)");
		Printf2(0x00, "%s", SetSpaceData(3));
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(8));
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(1));
		Printf2(0x08, "(SeatNo)");
		Printf2(0x00, "%s", SetSpaceData(1));
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(8));
		Printf2(0x10, "早\r\n");
	}

	///
	{
		Printf2(0x10, "朵式式式式式式池式式式式池式式式式式池式式式式此\r\n");
	}

	///
	{
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x18, "遴歎�蜓�");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x10, "早");
		//Printf2(0x18, "旎�ㄟ篲�");
		sprintf(Buffer, "%s", pPrtData->bus_cacm_nm);
		AlignBuffer(8, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x18, retBuf);

		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x18, "蝓離��");
		Printf2(0x00, "%s", SetSpaceData(2));
		Printf2(0x10, "早");

		//sprintf(Buffer, "%s", "31234-��");
		sprintf(Buffer, "%s", pPrtData->rdhm_val);
		AlignBuffer(8, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x18, retBuf);

		Printf2(0x10, "早\r\n");
	}

	///
	{
		Printf2(0x10, "早");
		Printf2(0x08, "(Express Co)");
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(8));
		Printf2(0x10, "早");
		Printf2(0x08, "(platform)");
		Printf2(0x10, "早");
		Printf2(0x00, "%s", SetSpaceData(8));
		Printf2(0x10, "早\r\n");
	}

	///
	{
		Printf2(0x10, "汍式式式式式式江式式式式江式式式式式江式式式式污\r\n");
	}

	///	蝓檣廓��, 蘋萄薑爾
	{
		//Printf2(0x00, "蝓檣廓�� 1234567");
		Printf2(0x00, "蝓檣廓�� %s", pPrtData->card_aprv_no);
		Printf2(0x00, "%s", SetSpaceData(7-4));
		//Printf2(0x00, "褐辨 1234567890123456\r\n");
		Printf2(0x00, "檣隸廓�� %s\r\n", pPrtData->card_no);
	}

	///
	{
		//Printf2(0x00, "蝓檣旎擋 34,700\r\n");
		Printf2(0x00, "蝓檣旎擋 %s\r\n", pPrtData->card_aprv_amt);
	}

	/// 
	{
		//Printf2(0x18, "賅夥橾じ鰍");
		sprintf(Buffer, "%s", SetSpaceData(10));
		if(nFunction == enFUNC_MRS)
		{
			sprintf(Buffer, "賅夥橾じ鰍");
		}
		AlignBuffer(10, Buffer, strlen(Buffer), retBuf, sizeof(retBuf) - 1);
		Printf2(0x18, retBuf);

		Printf2(0x00, "%s", SetSpaceData(8));
		
		if(pPrtData->n_pym_dvs == PYM_CD_CARD)
		{
			sprintf(Buffer, "⑷濰蘋萄");
			if(nFunction == enFUNC_MRS)
			{
				sprintf(Buffer, "褐辨蘋萄");
			}
			Printf2(0x38, Buffer);

			if(pPrtData->n_mip_mm_num == 0)
			{
				Printf2(0x08, " (橾衛碳)\r\n\r\n");
			}
			else
			{
				Printf2(0x08, " (%d 偃錯)\r\n\r\n", pPrtData->n_mip_mm_num);
			}
		}
		else if(pPrtData->n_pym_dvs == PYM_CD_TPAY)
		{
			sprintf(Buffer, "む檜");
			Printf2(0x38, Buffer);
			if(pPrtData->n_mip_mm_num == 0)
			{
				Printf2(0x08, " (橾衛碳)\r\n\r\n");
			}
			else
			{
				Printf2(0x08, " (%d 偃錯)\r\n\r\n", pPrtData->n_mip_mm_num);
			}
		}
		// 20210910 ADD
		else if ( (pPrtData->n_pym_dvs == PYM_CD_ETC) || (pPrtData->n_pym_dvs == PYM_CD_COMP) )
		{
			sprintf(Buffer, "犒м唸薯");
			Printf2(0x38, Buffer);
			/*
			if(pPrtData->n_mip_mm_num == 0)
			{
				Printf2(0x08, " (橾衛碳)\r\n\r\n");
			}
			else
			{
				Printf2(0x08, " (%d 偃錯)\r\n\r\n", pPrtData->n_mip_mm_num);
			}
			*/
		}
		// 20210910 ~ADD
		else
		{
			Printf2(0x38, "⑷旎\r\n\r\n");
		}
	}

	///
	{
		//Printf2(0x08, "≦嶸�膨滶�:渡橾雖薑離縑 ил  [13:06]\r\n");
		Printf2(0x08, "≦嶸�膨滶�:渡橾雖薑離縑 ил  %s\r\n", pPrtData->pub_time);
	}

	///
	{
		Printf2(0x08, "堅樓�蜓�:");
		//Printf2(0x00, "旎�ˉ虒捉穈篲�(輿) 301-81-00353\r\n");
		Printf2(0x00, "%s %s\r\n", pPrtData->bus_cacm_nm, pPrtData->bus_cacm_biz_no);
	}

	///	陛裊薄 薑爾
	{
		//Printf2(0x08, "(輿)縛縑蝶Я 129-86-27172\r\n");
		//sprintf(Buffer, "陛裊薄餌機濠廓��:%s", pOperConfig->koTrmlInfo_t.sz_prn_trml_corp_no);
		Printf2(0x08, "%s %s\r\n", pOperConfig->koTrmlInfo_t.sz_prn_trml_nm, pOperConfig->koTrmlInfo_t.sz_prn_trml_corp_no);
	}

	/// 陛裊薄 瞪�食醽�
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
			Printf2(0x00, "⑷旎艙熱隸僥曖\r\n");
		}
	}

	///	陛裊薄 ARS廓��
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
// 		Printf2(0x38, "寰頂 1688-4700");
	}

	PaperFullCut();
}

/**
 * @brief		StartProcess
 * @details		Start
 * @param		int nCommIdx		鱔褐んお 廓��
 * @return		撩奢 : > 0, 褒ぬ : < 0
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
 * @details		謙猿
 * @param		None
 * @return		о鼻 0
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




