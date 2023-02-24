// 
// 
// Prt_TcketNHS.cpp : 승차권 프린터 (화성) - 신규 프로토콜 (send & receive protocol)
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "dev_prt_ticket_nhs.h"
#include "dev_tr_mem.h"
#include "dev_tr_kobus_mem.h"
#include "dev_tr_main.h"
#include "event_if.h"
#include "oper_config.h"
#include "File_Env_ini.h"

//----------------------------------------------------------------------------------------------------------------------

#define LOG_OUT(fmt, ...)		{ CPrtTicketHS_NP::m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ CPrtTicketHS_NP::m_clsLog.HexaDump(x, y, z); }

//----------------------------------------------------------------------------------------------------------------------

// LeftSubstring - 한글 문자열 길이에 맞게 줄임
#define IsHangle_2021(pchar)  ((unsigned char)(pchar) > 0x7f)

/**
 * @brief		CPrtTicketHS_NP
 * @details		생성자
 */
CPrtTicketHS_NP::CPrtTicketHS_NP()
{
	m_bConnected = FALSE;
	m_hAccMutex = NULL;

	m_bExpOnly = FALSE;
}

/**
 * @brief		~CPrtTicketHS_NP
 * @details		소멸자
 */
CPrtTicketHS_NP::~CPrtTicketHS_NP()
{

}

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
int CPrtTicketHS_NP::Locking(void)
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
int CPrtTicketHS_NP::UnLocking(void)
{
	if(m_hAccMutex != NULL) 
	{
		::ReleaseMutex(m_hAccMutex);
	}

	return 0;	
}

void CPrtTicketHS_NP::LOG_INIT(void)
{
	m_clsLog.SetData(30, "\\Log\\TcketPrt\\HS_NP");
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		TxRxData
 * @details		데이타 send & receive 처리
 * @param		int nCommand	전송 command
 * @param		BYTE *pData		전송 Data
 * @param		WORD wDataLen	전송 Data length
 * @return		항상 : 0
 */
int CPrtTicketHS_NP::TxRxData(BYTE *pData, int nDataLen, BOOL bRxFlag)
{
	int nOffset, nRet, ch;
	DWORD dwTick;
	BYTE bcc = 0;
	int nRetry = 0;

	nOffset = 0;

	/// SUB
	m_szTxBuf[nOffset++] = 0x1A;
	/// #DATA
	nOffset += sprintf((char *)&m_szTxBuf[nOffset], "%s", "#DATA");
	
	/// data length
	m_szTxBuf[nOffset++] = nDataLen & 0xFF;
	/// data
	CopyMemory(&m_szTxBuf[nOffset], pData, nDataLen);
	nOffset += nDataLen;
	/// bcc
	for(int i = 0; i < nDataLen; i++)
	{
		bcc ^= pData[i];
	}
	m_szTxBuf[nOffset++] = bcc;

	LOG_HEXA("send data", m_szTxBuf, nOffset);

	nRet = m_clsComm.SendData(m_szTxBuf, nOffset);
	if(bRxFlag == TRUE)
	{
recv_proc:
		dwTick  = GetTickCount();
		while( Util_CheckExpire(dwTick) < 1000 )	
		{
			ch = m_clsComm.ReadData();
			if(ch < 0) 
			{
				Sleep(1);
				continue;
			}
			ch = ch & 0xFF;
			if( ch != 0x06 )
			{
				if(++nRetry >= 2)
				{
					LOG_OUT("recv failure, ch = %02X ", ch);
					return -2;
				}
				// 재전송
				Sleep(300);
				nRet = m_clsComm.SendData(m_szTxBuf, nOffset);
				goto recv_proc;				
			}
			LOG_OUT("recv success, ch = %02X ", ch);
			return 1;
		}

		if(++nRetry >= 2)
		{
			LOG_OUT("recv timeout !!!");
			return -1;
		}
		Sleep(300);
		// null data 전송
		ZeroMemory(m_szTxBuf, 0);
		nOffset = 8;
		nRet = m_clsComm.SendData(m_szTxBuf, nOffset);
		LOG_OUT("recv timeout retry !!!");
		
		goto recv_proc;
	}
	return 1;
}

/**
 * @brief		GetStatus
 * @details		프린터 상태 명령 전송
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrtTicketHS_NP::GetStatus(void)
{
	int ch;
	DWORD dwTick;
	static int prev_ch = -1;

#if 1
	/// 자동응답 상태 체크 : 신규 프로토콜에서는 사용안함
	//m_clsComm.SendData((BYTE *)"\x1D\x61\x01", 3);
	
	/// 프린터 실시간 상태체크 : 신규 프로토콜에서 사용함
	m_clsComm.SendData((BYTE *)"\x10\x04\x02", 3);

	dwTick = GetTickCount();
	while( Util_CheckExpire(dwTick) < 500 )	
	{
		ch = m_clsComm.ReadData();
		if(ch < 0) 
		{
			continue;
		}
		SetCheckEventCode(EC_TCKPRT_COMM_ERR, FALSE);
			
		ch = ch & 0xFF;

		if(prev_ch != ch)
		{
			// status code....
			LOG_OUT("GetStatus(), ch = %02X ", ch);

			if(ch & 0x01)
			{
				LOG_OUT("용지없음 \n");
				SetCheckEventCode(EC_TCKPRT_NO_PAPER, TRUE);
			}
			else
			{
				SetCheckEventCode(EC_TCKPRT_NO_PAPER, FALSE);
			}

			if(ch & 0x02)
			{
				LOG_OUT("프린트 헤드 업 \n");
				SetCheckEventCode(EC_TCKPRT_HEAD_UP, TRUE);
			}
			else
			{
				SetCheckEventCode(EC_TCKPRT_HEAD_UP, FALSE);
			}

			if(ch & 0x04)
			{
				LOG_OUT("용지 잼 \n");
				SetCheckEventCode(EC_TCKPRT_PAPER_JAM, TRUE);
			}
			else
			{
				SetCheckEventCode(EC_TCKPRT_PAPER_JAM, FALSE);
			}

			if(ch & 0x08)
			{
				LOG_OUT("용지 Near END \n");
			}
			if(ch & 0x10)
			{
				LOG_OUT("프린트 또는 Feeding 중 \n");
			}
			if(ch & 0x20)
			{
				LOG_OUT("컷터 에러(잼)4 \n");
				SetCheckEventCode(EC_TCKPRT_CUTTER_ERR, TRUE);
			}
			else
			{
				SetCheckEventCode(EC_TCKPRT_CUTTER_ERR, FALSE);
			}
			if(ch & 0x80)
			{
				LOG_OUT("보조센서에 용지 있음 \n");
			}

			prev_ch = ch;
		}


		return ch;
	}
	
	SetCheckEventCode(EC_TCKPRT_COMM_ERR, TRUE);
	
	return -1;
#else

	LOG_OUT("start !!!");
	nRet = TxRxData((BYTE *)"\x1D\x61\x01", 3);
	
	return 1;

#endif

}

int CPrtTicketHS_NP::txtPrint(int nX, int nY, int nMode, int nRotate, char *pStr) 
{
	int nOffset = 0;
	int nRet;
	char Buffer[200];

	/// FONT 설정
	switch(nMode)
	{
	case PRT_MODE_VERTI_EXPAND	:
		nRet = TxRxData((BYTE *)"\x1D\x21\x10", 3);
		break;
	case PRT_MODE_HORI_EXPAND	:
		TxRxData((BYTE *)"\x1D\x21\x01", 3);
		break;
	case PRT_MODE_BOTH_EXPAND	:
		TxRxData((BYTE *)"\x1D\x21\x11", 3);
		break;
	case PRT_MODE_SMALL_ON		:
		TxRxData((BYTE *)"\x1B\x4D\x01", 3);
		break;
	case PRT_MODE_SMALL_OFF		:
		TxRxData((BYTE *)"\x1B\x4D\x00", 3);
		break;
	case PRT_MODE_NONE			:
	default:
		TxRxData((BYTE *)"\x1D\x21\x00", 3);
		break;
	}
	/// 전주용 Rotate 모드 설정kh_200717
	switch(nRotate)
	{
	case PRT_ROTATE_JUN		:
		nRet = TxRxData((BYTE *)"\x1B\x54\x01", 3);
		break;
	case PRT_ROTATE_NONE	:
	default:
		nRet = TxRxData((BYTE *)"\x1B\x54\x03", 3);
		break;
	}
	// 문자 프린트
	LOG_OUT("nX(%d), nY(%d), pStr(%s)..", nX, nY, pStr);
	nOffset = sprintf((char *)Buffer, "\x1B\x57%04d%04d%s", nX, nY, pStr);
	TxRxData((BYTE *)Buffer, nOffset);

#if (_KTC_CERTIFY_ > 0)
	KTC_MemClear(m_szTxBuf, sizeof(m_szTxBuf));
#endif

	/// FONT 해제
	switch(nMode)
	{
	case PRT_MODE_SMALL_ON		:
		TxRxData((BYTE *)"\x1B\x4D\x00", 3);
		break;
	default:
		TxRxData((BYTE *)"\x1D\x21\x00", 3);
		break;
	}

	return 0;

	/// 180도 회전
	//switch(nMode)
	//{
	//case PRT_MODE_SMALL_ON		:
	//	TxRxData((BYTE *)"\x1B\x54\x01", 3);
	//	break;
	//default:
	//	TxRxData((BYTE *)"\x1D\x21\x00", 3);
	//	break;
	//}

	//return 0;


}

/**
 * @brief		SetBold
 * @details		Set 인쇄농도 설정
 * @param		int nValue			농도설정값 (0 ~ 5)
 * @return		None
 */
void CPrtTicketHS_NP::SetBold(int nValue)
{
	BYTE szData[] = "\x1D\x28\x4b\x02\x00\x31\x00";

	szData[6] = nValue & 0x05;

	TxRxData((BYTE *)szData, 7);
}

/**
 * @brief		BeginPrint
 * @details		티켓 프린터 시작 / 종료 명령
 * @param		BOOL bStart		시작 or 종료 Flag
 * @return		성공 : > 0, 실패 : < 0
 */
void CPrtTicketHS_NP::BeginPrint(BOOL bStart)
{
	LOG_OUT(" ========================================== ");

	if(bStart == TRUE)
	{
		PTCK_PRT_OPT_T pEnv = (PTCK_PRT_OPT_T) GetEnvTckPrtInfo();

		///< 01. speed
		//TxRxData((BYTE *)"\x1A\x73\x0E", 3);	// 200 mm/sec	// 20220706 DEL
		TxRxData((BYTE *)"\x1A\x73\x13", 3);	// 250 mm/sec	// 20220706 MOD

		///< 01. 농도조절
//		SendData((BYTE *)"\x1D\x28\x4B\x02\x00\x11\x05", 7);
		LOG_OUT("인쇄농도 : %d ", pEnv->nBoldVal);
		SetBold(pEnv->nBoldVal);
		//SetBold(4);
		//SendData((BYTE *)"\x1D\x28\x4B\x02\x00\x31\x05", 7);

		///< 02. PAGE MODE
		TxRxData((BYTE *)"\x1B\x4C", 2);

		///< 03. PAGE 방향 설정 ("T")
		if(m_bExpOnly == TRUE)
		{	/// 전주고속 승차권 
			TxRxData((BYTE *)"\x1B\x54\x01", 3);	
		}
		else
		{
			TxRxData((BYTE *)"\x1B\x54\x03", 3);	
		}
	}
	else
	{
		TxRxData((BYTE *)"\x1b\x0c\x13\x69\x1b\x53", 6);
	}
}


/**
 * @brief		SetDoubleFont
 * @details		Double 폰트 사용유무 설정
 * @param		BOOL bSet			설정유무 Flag
 * @return		None
 */
void CPrtTicketHS_NP::SetDoubleFont(BOOL bSet)
{
	if(bSet == TRUE)
	{
		TxRxData((BYTE *)"\x1D\x21\x11", 3);
	}
	else
	{
		TxRxData((BYTE *)"\x1D\x21\x00", 3);
	}
}

void CPrtTicketHS_NP::SetDoubleFont(int nFont, BOOL bSet)
{
	if(bSet == TRUE)
	{
		switch(nFont)
		{
		case FONT_VERTI_EXPAND:
			TxRxData((BYTE *)"\x1D\x21\x10", 3);
			break;
		case FONT_HORI_EXPAND:
			TxRxData((BYTE *)"\x1D\x21\x01", 3);
			break;
		case FONT_BOTH_EXPAND:
			TxRxData((BYTE *)"\x1D\x21\x11", 3);
			break;
		}
	}
	else
	{
		TxRxData((BYTE *)"\x1D\x21\x00", 3);
	}
}

/**
 * @brief		SetSmallFont
 * @details		Small 폰트 사용유무 설정
 * @param		BOOL bSet			설정유무 Flag
 * @return		None
 */
void CPrtTicketHS_NP::SetSmallFont(BOOL bSet)
{
	if(bSet == TRUE)
	{
		TxRxData((BYTE *)"\x1B\x4D\x01", 3);
	}
	else
	{
		TxRxData((BYTE *)"\x1B\x4D\x00", 3);
	}
}

/**
 * @brief		leftsubstring
 * @details		한글 문자열 길이에 맞게 줄임
 * @param		
 * @return		문자열주소
 */
char* CPrtTicketHS_NP::LeftSubstring(char* sz, int len)
{
    int i = 0;
     
    if ( strlen(sz) <= len ) return sz;
     
    for (i=0; i<len; i++)
    {
        if ( IsHangle_2021(sz[i]) )   
        {
            if ( len-1 < i+1 ) break;
            else  i++;
        }
    }

    // 문자열 끝에 ".."추가
    sz[i] = '.';
    sz[i+1] = '.';
    sz[i+2] = 0;
     
    return sz;
}

/**
 * @brief		IsHangule
 * @details		한글 체크
 * @param		
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrtTicketHS_NP::IsHangule(char *pData, int nDataLen)
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
int CPrtTicketHS_NP::MultiPrint(int nX, int nY, int nMaxMode, int nMinMode, int nMAX, int nRotate, char *pData, BOOL bExpJun)
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
					txtPrint(nX-(i*25)+5, nY, nMinMode, nRotate, Buffer);
				else
					txtPrint(nX-(i*35)+5, nY, nMinMode, nRotate, Buffer);
			}
			else
			{
				if(nMinMode == PRT_MODE_NONE)
					txtPrint(nX+(i*25)+5, nY, nMinMode, nRotate, Buffer);
				else
					txtPrint(nX+(i*35)+5, nY, nMinMode, nRotate, Buffer);

			}
		}
	}
	else
	{
		txtPrint(nX, nY, nMaxMode, nRotate, pData);
	}
	return 0;
}

/**
 * @brief		StartProcess
 * @details		Start
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrtTicketHS_NP::StartProcess(int nCommIdx)
{
	int		nRet;

	m_bConnected = FALSE;

	LOG_INIT();
	EndProcess();//
	nRet = m_clsComm.Open(nCommIdx, CBR_19200, 8, NOPARITY, ONESTOPBIT);
	/// nRet = m_clsComm.Open(nCommIdx, CBR_115200, 8, NOPARITY, ONESTOPBIT); // 20230119 MOD // 20230119 DEL
	if(nRet < 0) 
	{
		//LOG_OUT("[%s:%d] s_clsCdsp.Open() failure, ret(%d)..\n", __FUNCTION__, __LINE__, nRet);
		return -1;
	}

	m_bConnected = TRUE;

	m_hAccMutex = ::CreateMutex(NULL, FALSE, NULL);
	if(m_hAccMutex == NULL) 
	{
		//LOG_OUT("[%s:%d] CreateMutex() failure..\n", __FUNCTION__, __LINE__);
		//m_clsComm.Close();
		EndProcess();
		return -2;
	}

	return 0;
}

/**
 * @brief		EndProcess
 * @details		Start
 * @param		int nCommIdx		COM
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrtTicketHS_NP::EndProcess(void)
{
	if(m_bConnected == TRUE)
	{
		m_clsComm.Close();
	}

	if(m_hAccMutex != NULL)
	{
		CloseHandle(m_hAccMutex);
		m_hAccMutex = NULL;
	}

	m_bConnected = FALSE;

	return 0;
}

/**
 * @brief		Exp_Only_PrintTicket
 * @details		[코버스-고속전용승차권] 티켓 프린트
 * @param		char *pData			프린트 데이타 구조체
 * @return		항상 = 0
 */
void CPrtTicketHS_NP::DrawBox(BOOL bPassenger)
{
	int i, nGap, nX, nY;

	i = nGap = nX = nY = 0;

	if(bPassenger == FALSE)
	{	/// 회수용
		/// 전체 Y좌표 +16 적용
		i = 0;
		nGap = 21;

		txtPrint(195+(nGap*i), 1336, PRT_MODE_NONE, PRT_ROTATE_JUN, "┌───────────────────");  i++;
		txtPrint(195+(nGap*i), 1336, PRT_MODE_NONE, PRT_ROTATE_JUN, "│ ");  i++;
		txtPrint(195+(nGap*i), 1336, PRT_MODE_NONE, PRT_ROTATE_JUN, "│ ");  i++;
		txtPrint(195+(nGap*i), 1336, PRT_MODE_NONE, PRT_ROTATE_JUN, "├───────────────────");  i++;
		txtPrint(195+(nGap*i), 1336, PRT_MODE_NONE, PRT_ROTATE_JUN, "│ ");  i++;
		txtPrint(195+(nGap*i), 1336, PRT_MODE_NONE, PRT_ROTATE_JUN, "│ ");  i++;
		txtPrint(195+(nGap*i), 1336, PRT_MODE_NONE, PRT_ROTATE_JUN, "└───────────────────");  i++;

		nGap = 23;
		nX = 204;
		txtPrint(nX+(nGap*0), 1096, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
		txtPrint(nX+(nGap*1), 1096, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
		txtPrint(nX+(nGap*2), 1096, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
		txtPrint(nX+(nGap*3), 1096, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
		nX = 195;
		txtPrint(nX+(nGap*4), 1096, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
		txtPrint(nX+(nGap*5), 1096, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");

		nX = 204;
		txtPrint(nX+(nGap*0), 960, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
		txtPrint(nX+(nGap*1), 960, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
		txtPrint(nX+(nGap*2), 960, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
		txtPrint(nX+(nGap*3), 960, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
		nX = 195;
		txtPrint(nX+(nGap*4), 960, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
		txtPrint(nX+(nGap*5), 960, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");

		nX = 204;
		txtPrint(nX+(nGap*0), 848, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
		txtPrint(nX+(nGap*1), 848, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
		txtPrint(nX+(nGap*2), 848, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
		txtPrint(nX+(nGap*3), 848, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
		nX = 195;
		txtPrint(nX+(nGap*4), 848, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
		txtPrint(nX+(nGap*5), 848, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
	}
	else
	{	/// 승객용
		///< 4-8. 박스 그리기
		i = 0;
		nGap = 21; 
		nY = 764;
		//nY = 762;
		txtPrint(195+(nGap*i), nY, PRT_MODE_NONE, PRT_ROTATE_JUN, "┌─────────────────────────────");  i++;
		txtPrint(195+(nGap*i), nY, PRT_MODE_NONE, PRT_ROTATE_JUN, "│ ");  i++;
		txtPrint(195+(nGap*i), nY, PRT_MODE_NONE, PRT_ROTATE_JUN, "│ ");  i++;
		txtPrint(195+(nGap*i), nY, PRT_MODE_NONE, PRT_ROTATE_JUN, "├─────────────────────────────");  i++;
		txtPrint(195+(nGap*i), nY, PRT_MODE_NONE, PRT_ROTATE_JUN, "│ ");  i++;
		txtPrint(195+(nGap*i), nY, PRT_MODE_NONE, PRT_ROTATE_JUN, "│ ");  i++;
		txtPrint(195+(nGap*i), nY, PRT_MODE_NONE, PRT_ROTATE_JUN, "└─────────────────────────────");  i++;

		nGap = 23; 
		for(i = 0; i < 5; i++)
		{
			nX = 204;
			switch(i)
			{
//			case 0 : nY = 536; break;
			case 0 : nY = 555; break;
			case 1 : nY = 432; break;
			case 2 : nY = 320; break;
			case 3 : nY = 168; break;
			case 4 : nY = 28; break;
//			case 4 : nY = 37; break;
			}
			//nY = 536;
			txtPrint(nX+(nGap*0), nY, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
			txtPrint(nX+(nGap*1), nY, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
			txtPrint(nX+(nGap*2), nY, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
			txtPrint(nX+(nGap*3), nY, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
			nX = 195;
			txtPrint(nX+(nGap*4), nY, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
			txtPrint(nX+(nGap*5), nY, PRT_MODE_NONE, PRT_ROTATE_JUN, "│");
		}
	}
}

/**
 * @brief		Print_DeprDt
 * @details		[시외버스] 출발일자 항목 프린트
 * @param		char *pData				승차권 프린트 위치등의 데이타
 * @param		char *pPrt				프린트 데이타
 * @return		항상 = 0
 */
int CPrtTicketHS_NP::Print_DeprDt(char *pData, char *pPrt)
{
	int nRet;
	BOOL bFlag;
	PPTRG_INFO_T pInfo;
	PTCK_PRINT_FMT_T pPrtData;
	PTCK_PRT_OPT_T pEnvTckOpt;

	pInfo		= (PPTRG_INFO_T) pData;
	pPrtData	= (PTCK_PRINT_FMT_T) pPrt;
	pEnvTckOpt	= (PTCK_PRT_OPT_T) GetEnvTckPrtInfo();

	bFlag = FALSE;

	TR_LOG_OUT("alcn_way_dvs_cd=[%c], sati_use_yn=[%c], nsats_depr_day_yn[%c], nalcn_depr_day_yn=[%c] ..", 
				pPrtData->alcn_way_dvs_cd[0], pPrtData->sati_use_yn[0], 
				pEnvTckOpt->nsats_depr_day_yn[0], pEnvTckOpt->nalcn_depr_day_yn[0]);

	switch(pPrtData->alcn_way_dvs_cd[0])
	{
	case 'D':	/// 배차
		nRet = CConfigTkMem::GetInstance()->IsSatAlcn(pPrtData->alcn_way_dvs_cd, pPrtData->sati_use_yn);
		if(nRet > 0)
		{	/// 배차 & 좌석제
			bFlag = TRUE;
		}
		else
		{		
			if(pEnvTckOpt->nsats_depr_day_yn[0] == 'Y')
			{	/// 비좌석제 승차권 "출발일" 출력 유무
				bFlag = TRUE;
			}
		}
		break;
	case 'N':	/// 비배차
		if(pEnvTckOpt->nalcn_depr_day_yn[0] == 'Y')
		{	/// 비배차 승차권 "출발일" 출력 유무
			bFlag = TRUE;
		}
		break;
	}

	if(bFlag == TRUE)
	{
		txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->atl_depr_dt);
	}

	return 0;
}

/**
 * @brief		Print_DeprTime
 * @details		[시외버스] 출발시간 항목 프린트
 * @param		char *pData				승차권 프린트 위치등의 데이타
 * @param		char *pPrt				프린트 데이타
 * @return		항상 = 0
 */
int CPrtTicketHS_NP::Print_DeprTime(char *pData, char *pPrt)
{
//	int					nRet;				// 20210826 DEL, heesik.seo
	BOOL				bFlag;
	PPTRG_INFO_T		pInfo;
	PTCK_PRINT_FMT_T	pPrtData;
	PTCK_PRT_OPT_T		pEnvTckOpt;
	PKIOSK_INI_ENV_T	pEnv;

	pInfo		= (PPTRG_INFO_T) pData;
	pPrtData	= (PTCK_PRINT_FMT_T) pPrt;
	pEnvTckOpt	= (PTCK_PRT_OPT_T) GetEnvTckPrtInfo();
	pEnv		= (PKIOSK_INI_ENV_T) GetEnvInfo();

	TR_LOG_OUT("========== 선착순 출력 관련 정보 ==========");
	TR_LOG_OUT("(서버정보) 출발시간 출력여부=(%c), 배차=(%c), 좌석제=(%c) ..", 
				pPrtData->depr_time_prin_yn[0], pPrtData->alcn_way_dvs_cd[0], pPrtData->sati_use_yn[0]);
	TR_LOG_OUT("(무인기정보) 배차-비좌석제 출력여부=(%c), 비배차-비좌석제 선착순 문구 출력여부(%c) ..", 
				pEnv->tTckOpt.nsats_depr_time_yn[0], pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0]);

	bFlag = FALSE;
	if(pPrtData->depr_time_prin_yn[0] == 'Y')
	{
		if(CConfigTkMem::GetInstance()->IsSatAlcn(pPrtData->alcn_way_dvs_cd, pPrtData->sati_use_yn) > 0)
		{	/// 배차이고, 좌석제이면..
			bFlag = TRUE;
		}
		else if( CConfigTkMem::GetInstance()->IsNsatsAlcn(pPrtData->alcn_way_dvs_cd, pPrtData->sati_use_yn) && 
			     (pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y') ) 
		{	/// 배차_비좌석제이고, 비좌석제 승차권 시간출력 유무가 'Y' 이면
			bFlag = TRUE;
		}
		else if( CConfigTkMem::GetInstance()->IsNalcn(pPrtData->alcn_way_dvs_cd) )
		{	/// 비배차 이면
			bFlag = FALSE;
		}
	}

	if(bFlag == TRUE)
	{
		///	출발시간 출력
		txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->atl_depr_time);
	}
	else if( (bFlag == FALSE) && (pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y') )
	{
		/// 선착순 출력
		txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, "선착순");
	}

	return 0;
}

/**
 * @brief		CbusTicketPrint
 * @details		시외버스 승차권 프린트
 * @param		char *pData				프린트 데이타
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrtTicketHS_NP::CbusTicketPrint(char *pData)
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

	i = k = nLen = nRet = 0;

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
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID1_USRID:		/// (1줄) 사용자 ID
					sprintf(Buffer, "%s", GetEnvSvrUserNo(SVR_DVS_CCBUS));//kh200708
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID1_USRNM:		/// (1줄) 사용자 Name
					sprintf(Buffer, "(%s)", pPrtData->user_nm);//kh200708
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID1_WNDNO:		/// (1줄) (창구번호+무인기) kh200710
					sprintf(Buffer, "( %s 무인기 )", pPrtData->qr_pub_wnd_no);
 					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID1_MSG1:			/// (1줄) MSG_1 kh200708
				case PTRG_ID1_MSG2:			/// (1줄) MSG_2
				case PTRG_ID1_MSG3:			/// (1줄) MSG_3
				case PTRG_ID1_MSG4:			/// (1줄) MSG_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;

				case PTRG_ID2_DEPR_KO:		/// (2줄) 출발지(한글)
					nLen = strlen(pPrtData->depr_trml_ko_nm);
					if(nLen > 0)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->depr_trml_ko_nm);
					}
					break;
				case PTRG_ID2_ARVL_KO:		/// (2줄) 도착지(한글)
					nLen = strlen(pPrtData->arvl_trml_ko_nm);
					if(nLen > 0)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->arvl_trml_ko_nm);
					}
					break;
				case PTRG_ID2_DEPR_KO_1:		/// (2줄) 출발지(한글) - 승객용
					nLen = strlen(pPrtData->depr_trml_ko_nm);
					if(nLen > 0)
					{
						MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pInfo->nRotate, pPrtData->depr_trml_ko_nm);
					}
					break;
				case PTRG_ID2_ARVL_KO_1:		/// (2줄) 도착지(한글) - 승객용
					nLen = strlen(pPrtData->arvl_trml_ko_nm);
					if(nLen > 0)
					{
						MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pInfo->nRotate, pPrtData->arvl_trml_ko_nm);
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
								txtPrint(pInfo->nX - (m * 20), pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
							}
						}
						else
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cls_nm);
						}
					}
					break;
				case PTRG_ID2_DEPR_EN:		/// (2줄) 출발지(영문)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->depr_trml_eng_nm);
					break;
				case PTRG_ID2_ARVL_EN:		/// (2줄) 도착지(영문)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->arvl_trml_eng_nm);
					break;
				case PTRG_ID2_THRU_NM:		/// (2줄) 경유지
					nLen = strlen(pPrtData->thru_nm);
					if(nLen > 0)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->thru_nm);
					}
					break;
				case PTRG_ID2_DIST:			/// (2줄) 거리
// 					nLen = strlen(pPrtData->thru_nm);
// 					if(nLen > 0)
// 					{
// 						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->thru_nm);
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
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
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
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					break;

				case PTRG_ID2_ROT_NO:			/// (2줄) 도착지 - 노선번호
					{
						if( strlen(pPrtData->arvl_trml_rot_num) > 0 )
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->arvl_trml_rot_num);
						}
					}
					break;
				
				case PTRG_ID2_PYM_DVS:		/// (2줄) 간략결제수단 kh_200709
					if(pPrtData->n_pym_dvs == PYM_CD_CARD)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->shrt_pym_dvs);
					}
					else if(pPrtData->n_pym_dvs == PYM_CD_TPAY)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->shrt_pym_dvs);
					}
					else
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->shrt_pym_dvs);
					}
					break;

				case PTRG_ID2_ROT_NO_ARVL:			/// (2줄) 회수용 노선번호 + 도착지 kh_200717
					{
						if( strlen(pPrtData->arvl_trml_rot_num) > 0 )
						{
							sprintf(Buffer, "%s %s", pPrtData->arvl_trml_rot_num, pPrtData->arvl_trml_ko_nm);
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
						}
						else
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->arvl_trml_ko_nm);
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
								MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, MAX_HANGUL_CHAR - 8, pInfo->nRotate, Buffer);
							}
							else
							{
								;
							}
						}
						else
						{
							MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, MAX_HANGUL_CHAR - 8, pInfo->nRotate, pPrtData->arvl_trml_ko_nm);
						}
					}
					break;


				case PTRG_ID2_MSG1:			/// (2줄) MSG_1
					// 20211015 ADD
					/// 이벤트쿠폰결제금액	
					if(strlen(pPrtData->evcp_pym_amt) > 0)
					{
						sprintf(Buffer, "쿠폰:%s ", pPrtData->evcp_pym_amt);
						LOG_OUT("MSG_ID=PTRG_ID2_MSG1 : %s, strlen(evcp_pym_amt) : %d", Buffer, strlen(pPrtData->evcp_pym_amt));
						if (strlen(pPrtData->evcp_pym_amt) >0)
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
						}
					}
					break;
					// 20211015 ~ADD
				case PTRG_ID2_MSG2:			/// (2줄) MSG_2
					// 20211015 ADD
					///	마일리지결제금액		
					if(strlen(pPrtData->mlg_pym_amt) > 0)
					{
						sprintf(Buffer, "마일리지:%s ", pPrtData->mlg_pym_amt);
						LOG_OUT("MSG_ID=PTRG_ID2_MSG2 : %s, strlen(mlg_pym_amt) : %d", Buffer, strlen(pPrtData->mlg_pym_amt));
						if (strlen(pPrtData->mlg_pym_amt) >0)
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
						}

					}
					break;
					// 20211015 ~ADD
				case PTRG_ID2_MSG3:			/// (2줄) MSG_3
				case PTRG_ID2_MSG4:			/// (2줄) MSG_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;

					/////////
				case PTRG_ID3_FARE:			/// (3줄) 요금
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->tisu_amt);
					break;
				case PTRG_ID3_PYM_DVS:		/// (3줄) 결제수단
/*					
					// 20221124 ADD~
					LOG_OUT(" >>>>>>>>>> pPrtData->n_pym_dvs = %c", pPrtData->n_pym_dvs);
					switch(pPrtData->n_pym_dvs)
					{
					case PYM_CD_QRPC:
						sprintf(pPrtData->pym_dvs, "%s", "PAYCO카드");
						break;
					case PYM_CD_QRPP:
						sprintf(pPrtData->pym_dvs, "%s", "PAYCO포인트");
						break;
					case PYM_CD_QRTP:
						sprintf(pPrtData->pym_dvs, "%s", "티머니페이");
						break;
					default:
						sprintf(pPrtData->pym_dvs, "%s", "현장카드");
						break;
					}
					LOG_OUT(" >>>>>>>>>> pPrtData->pym_dvs = %s", pPrtData->pym_dvs);
					// 20221124 ~ADD
*/					
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->pym_dvs);
					break;
				case PTRG_ID3_TCK_KND:		/// (3줄) 승차권 종류
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_tck_knd_nm);
					break;
				case PTRG_ID3_BUS_CLS:		/// (3줄) (버스등급) kh_200708
					sprintf(Buffer, "(%s)", pPrtData->bus_cls_nm);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID3_PYM_DVS_VERTI:/// (3줄) 결제수단 세로 kh_200708
					if(pPrtData->n_pym_dvs == PYM_CD_CARD)
					{
						txtPrint(pInfo->nX		, pInfo->nY, pInfo->nMode, pInfo->nRotate, "카");
						txtPrint(pInfo->nX-65	, pInfo->nY, pInfo->nMode, pInfo->nRotate, "드");
					}
					else if(pPrtData->n_pym_dvs == PYM_CD_TPAY)
					{
						txtPrint(pInfo->nX		, pInfo->nY, pInfo->nMode, pInfo->nRotate, "페");
						txtPrint(pInfo->nX-65	, pInfo->nY, pInfo->nMode, pInfo->nRotate, "이");
					}
					else if(pPrtData->n_pym_dvs == PYM_CD_RF)
					{
						txtPrint(pInfo->nX		, pInfo->nY, pInfo->nMode, pInfo->nRotate, "교");
						txtPrint(pInfo->nX-65	, pInfo->nY, pInfo->nMode, pInfo->nRotate, "통");
					}
					// 20211015 ADD
					else if(pPrtData->n_pym_dvs == PYM_CD_COMP)
					{
						txtPrint(pInfo->nX		, pInfo->nY, pInfo->nMode, pInfo->nRotate, "복");
						txtPrint(pInfo->nX-65	, pInfo->nY, pInfo->nMode, pInfo->nRotate, "합");
					}
					// 20211015 ~ADD
					// 20221124 ADD
					else if ( (pPrtData->n_pym_dvs == PYM_CD_QRPC) || (pPrtData->n_pym_dvs == PYM_CD_QRPP) || (pPrtData->n_pym_dvs == PYM_CD_QRTP) )
					{
						txtPrint(pInfo->nX		, pInfo->nY, pInfo->nMode, pInfo->nRotate, "Q");
						txtPrint(pInfo->nX-65	, pInfo->nY, pInfo->nMode, pInfo->nRotate, "R");
					}
					// 20221124 ~ADD
					else
					{
						txtPrint(pInfo->nX		, pInfo->nY, pInfo->nMode, pInfo->nRotate, "현");
						txtPrint(pInfo->nX-65	, pInfo->nY, pInfo->nMode, pInfo->nRotate, "금");
					}
					break;

				case PTRG_ID3_ENMNPP_VERTI:	/// 예매 관련 kh_200708
					switch(pPrtData->tisu_chnl_dvs_cd[0])
					{
					case 'W':
					case 'w':
						txtPrint(pInfo->nX-(48*0), pInfo->nY, pInfo->nMode, pInfo->nRotate, "인");
						txtPrint(pInfo->nX-(48*1), pInfo->nY, pInfo->nMode, pInfo->nRotate, "터");
						txtPrint(pInfo->nX-(48*2), pInfo->nY, pInfo->nMode, pInfo->nRotate, "넷");
						break;
					case 'M':
					case 'm':
						txtPrint(pInfo->nX-(48*0), pInfo->nY, pInfo->nMode, pInfo->nRotate, "모");
						txtPrint(pInfo->nX-(48*1), pInfo->nY, pInfo->nMode, pInfo->nRotate, "바");
						txtPrint(pInfo->nX-(48*2), pInfo->nY, pInfo->nMode, pInfo->nRotate, "일");
					}
					break;

				case PTRG_ID3_BUS_CLS_SHCT_NM: /// 짧은 버스등급
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cls_shct_nm);
					break;					

				case PTRG_ID3_BUS_CLS_SHCT_NM2:/// (짧은 버스등급)
					sprintf(Buffer, "(%s)", pPrtData->bus_cls_shct_nm);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID3_MSG1:			/// (3줄) 메시지_1
				case PTRG_ID3_MSG2:			/// (3줄) 메시지_2
				case PTRG_ID3_MSG3:			/// (3줄) 메시지_3
				case PTRG_ID3_MSG4:			/// (3줄) 메시지_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;

					/////////
				case PTRG_ID5_DEPR_DT:		/// (5줄) 출발일자
					if(GetEnvOperCorp() == KUMHO_OPER_CORP)
					{	/// 금호 사이트
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->atl_depr_dt);
					}
					else
					{
						Print_DeprDt((char *)pInfo, (char *)pPrtData);
					}
					break;
				case PTRG_ID5_DEPR_TIME:	/// (5줄) 출발시간 /kh_200709
					LOG_OUT("[502 - DEPR_TIME] EnvOperCorp       = [%d]", GetEnvOperCorp());
					if(GetEnvOperCorp() == KUMHO_OPER_CORP)		// 금호 사이트
					{	// 배차구분/좌석제에 따른 줄발시간/좌석 표시 옵션별 승차권 출력 // 20211203
						if( pPrtData->alcn_way_dvs_cd[0] == 'D' )			
						{ // 배차(D)
							LOG_OUT("[502 - DEPR_TIME] ALCN_WAY_DVS_CD   = [D]");
							if( pPrtData->sati_use_yn[0] == 'Y' )			
							{ // 좌석제(Y)
								LOG_OUT("[502 - DEPR_TIME] SATI_USE_YN       = [Y]");	// 좌석제(Y)
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->atl_depr_time);
							}
							else
							{ // 비좌석제(N)
								LOG_OUT("[502 - DEPR_TIME] SATI_USE_YN       = [N]");	// 비좌석제(N)
								if( ( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y' )
								 || ( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'N' ) )
								{
									LOG_OUT("[502 - DEPR_TIME] Ticket Option     = [Y][Y] or [Y][N]");
									txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->atl_depr_time);
								}
								else if( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'N' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y' )
								{
									LOG_OUT("[502 - DEPR_TIME] Ticket Option     = [N][Y]");
									txtPrint(pInfo->nX,    pInfo->nY, PRT_MODE_NONE, pInfo->nRotate, "<<선착순 탑승>>");
									txtPrint(pInfo->nX-30, pInfo->nY, PRT_MODE_NONE, pInfo->nRotate, "미탑승분은 다음차 이용");
								}
								else
								{
									LOG_OUT("[502 - DEPR_TIME] Ticket Option     = [N][N]");
									txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, " ");
								}
							}
						}
						else
						{ // 비배차(N)
							LOG_OUT("[502 - DEPR_TIME] ALCN_WAY_DVS_CD   = [N]");	// 비배차
							if( ( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y' )
								|| ( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'N' ) )
							{
								LOG_OUT("[502 - DEPR_TIME] Ticket Option     = [Y][Y] or [Y][N]");
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->atl_depr_time);
							}
							else if( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'N' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y' )
							{
								LOG_OUT("[502 - DEPR_TIME] Ticket Option     = [N][Y]");
								txtPrint(pInfo->nX,    pInfo->nY, PRT_MODE_NONE, pInfo->nRotate, "<<선착순 탑승>>");
								txtPrint(pInfo->nX-30, pInfo->nY, PRT_MODE_NONE, pInfo->nRotate, "미탑승분은 다음차 이용");
							}
							else
							{
								LOG_OUT("[502 - DEPR_TIME] Ticket Option     = [N][N]");
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, " ");
							}
						}
					}
					else // else 금호 사이트
					{	// 배차구분/좌석제에 따른 줄발시간/좌석 표시 옵션별 승차권 출력 // 20211203
						if( pPrtData->alcn_way_dvs_cd[0] == 'D' )			
						{ // 배차(D)
							LOG_OUT("[502 - DEPR_TIME] ALCN_WAY_DVS_CD   = [D]");
							if( pPrtData->sati_use_yn[0] == 'Y' )
							{ // 배차(D), 좌석제(Y)
								LOG_OUT("[502 - DEPR_TIME] SATI_USE_YN       = [Y]");	// 좌석제(Y)
								Print_DeprTime((char *)pInfo, (char *)pPrtData);
							}
							else
							{ // 배차(D), 비좌석제(N)
								LOG_OUT("[502 - DEPR_TIME] SATI_USE_YN       = [N]");	// 비좌석제(N)
								if( ( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y' )
								 || ( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'N' ) )
								{
									LOG_OUT("[502 - DEPR_TIME] Ticket Option     = [Y][Y] or [Y][N]");

									//txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->atl_depr_time); // 20220707 DEL
									
									// 20220707 ADD~	
									// 인천공항 특정노선에 한해 통전망의 출발시간출력여부 및 좌석번호출력여부 옵션 확인
									LOG_OUT(">>>>> 서버배차설정값 출발시간출력 여부 = %c, 좌석번호출력 여부 = %c ", pPrtData->depr_time_prin_yn[0], pPrtData->sats_no_prin_yn[0]); // 20230202 ADD

									if( pPrtData->depr_time_prin_yn[0] == 'Y' )	// 배차설정값 -> 출발시간출력 여부(Y)
									{
										txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->atl_depr_time);
									}
									else										// 출발시간출력 여부(N)
									{
										if( GetEnvOperCorp() == IIAC_OPER_CORP )
										{ // 운영사:2(인천공항)
											LOG_OUT("[502 - DEPR_TIME] Ticket Option     = For 노선번호:<6103> & <6100>");
											LOG_OUT("[502 - DEPR_TIME] Ticket Option     = IIAC_OPER_CORP = [2] & DEPR_TIME_PRIN_YN = [N]");
											//txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, " "); // 20221207 DEL
											// 20221207 ADD~
											// 20221212 요구스펙확인 배차설정값 출발시간출력 여부(N), 좌석번호출력 여부(N) 인 경우에는 '선착순'만 표시로 변경, '배차비좌석/배차설정 NY'인 경우 없음
											if( pPrtData->sats_no_prin_yn[0] == 'N' )
											{ // 좌석번호출력 여부 = N && 출발시간출력 여부 = N && 운영사:2(인천공항)
												LOG_OUT("[502 - DEPR_TIME] Ticket Option = 좌석번호출력 여부 = N && 출발시간출력 여부 = N && 운영사:2(인천공항)");
												txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, "선착순");
											}
											else
											{
												txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, " ");
											}
											// 20221207 ~ADD
										}
										else 
										{ // 운영사:3(일반)
											//txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->atl_depr_time);	// 20230202 DEL
											// 20230202 ADD~
											// 마산시외터미널의 요구사항으로 ENV_OPT.INI ([Y][Y])일지라도 sats_no_prin_yn = 'N'인 경우 시간출력 안함.
											if( pPrtData->sats_no_prin_yn[0] == 'N' ) // 배차설정값 -> 출발시간출력 여부(N), 좌석번호출력 여부(N)
											{
												txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, " ");
											}
											else
											{
												txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->atl_depr_time);
											}
											// 20230202 ~ADD
										}
									}
									// 20220707 ~ADD
								}
								else if( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'N' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y' )
								{
									LOG_OUT("[502 - DEPR_TIME] Ticket Option     = [N][Y]");
									txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, "선착순");
								}
								else
								{
									LOG_OUT("[502 - DEPR_TIME] Ticket Option     = [N][N]");
									txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, " ");
								}
							}
						}
						else
						{ // 비배차(N)
							LOG_OUT("[502 - DEPR_TIME] ALCN_WAY_DVS_CD   = [N]");	// 비배차
							/* // 20230223 DEL~
							if( ( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y' )
								|| ( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'N' ) )
							{
								LOG_OUT("[502 - DEPR_TIME] Ticket Option     = [Y][Y] or [Y][N]");
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->atl_depr_time);
							}
							else if( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'N' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y' )
							{
								LOG_OUT("[502 - DEPR_TIME] Ticket Option     = [N][Y]");
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, "선착순");
							}
							else
							{
								LOG_OUT("[502 - DEPR_TIME] Ticket Option     = [N][N]");
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, " ");
							}
							*/ // 20230223 ~DEL
							// 20230223 MOD~
							if( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y' )
							{
								LOG_OUT("[502 - DEPR_TIME] Ticket Option     = [Y][Y]");
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, "선착순");
							}
							else if( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'N' )
							{
								LOG_OUT("[502 - DEPR_TIME] Ticket Option     = [Y][N]");
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, " ");
							}
							else if( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'N' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y' )
							{
								LOG_OUT("[502 - DEPR_TIME] Ticket Option     = [N][Y]");
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, "선착순");
							}
							else if( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'N' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'N' )
							{
								LOG_OUT("[502 - DEPR_TIME] Ticket Option     = [N][N]");
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, " ");
							}
							else
							{ // 예비 옵션 
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->atl_depr_time);
							}
							// 20230223 ~MOD
						}
					}
					break;

				case PTRG_ID5_SATS_NO:		/// (5줄) 좌석번호 /kh_200709 
					LOG_OUT("[503 - SEATS__NO] EnvOperCorp       = [%d]", GetEnvOperCorp());
					if( pPrtData->sats_no_prin_yn[0] == 'Y' ) {
						LOG_OUT("[503 - SEATS__NO] SATS_NO_PRIN_YN   = [Y]");
					}else {
						LOG_OUT("[503 - SEATS__NO] SATS_NO_PRIN_YN   = [N]");
					}

					if(GetEnvOperCorp() == KUMHO_OPER_CORP) // 운영사:1(광주)
					{	// 배차구분/좌석제에 따른 줄발시간/좌석 표시 옵션별 승차권 출력 // 20211203
						if( pPrtData->alcn_way_dvs_cd[0] == 'D' )			// 배차(D)
						{
							LOG_OUT("[503 - SEATS__NO] ALCN_WAY_DVS_CD   = [D]");
							if( pPrtData->sati_use_yn[0] == 'Y' )			// 좌석제(Y)
							{
								LOG_OUT("[503 - SEATS__NO] SATI_USE_YN       = [Y]");	// 좌석제(Y)
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->sats_no);
							}
							else
							{
								LOG_OUT("[503 - SEATS__NO] SATI_USE_YN       = [N]");	// 비좌석제(Y)
								if( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y' )
								{
									LOG_OUT("[503 - SEATS__NO] Ticket Option     = [Y][Y]");
									txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, "선착순");
								}
								else if( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'N' )
								{
									LOG_OUT("[503 - SEATS__NO] Ticket Option     = [Y][N]");
									txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, " ");
								}
								else if( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'N' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y' )
								{
									LOG_OUT("[503 - SEATS__NO] Ticket Option     = [N][Y]");
									;
								}
								else
								{
									LOG_OUT("[503 - SEATS__NO] Ticket Option     = [N][N]");
									txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, " ");
								}
							}
						}
						else	// 비배차(N)
						{
							LOG_OUT("[503 - SEATS__NO] ALCN_WAY_DVS_CD   = [N]");
							if( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y' )
							{
								LOG_OUT("[503 - SEATS__NO] Ticket Option     = [Y][Y]");
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, "선착순");
							}
							else if( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'N' )
							{
								LOG_OUT("[503 - SEATS__NO] Ticket Option     = [Y][N]");
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, " ");
							}
							else if( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'N' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y' )
							{
								LOG_OUT("[503 - SEATS__NO] Ticket Option     = [N][Y]");
								;
							}
							else
							{
								LOG_OUT("[503 - SEATS__NO] Ticket Option     = [N][N]");
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, " ");
							}
						}
					}
					else // 운영사:광주 이외
					{	// 배차구분/좌석제에 따른 줄발시간/좌석 표시 옵션별 승차권 출력 // 20211203
						if( pPrtData->alcn_way_dvs_cd[0] == 'D' )			
						{ // 배차(D)
							LOG_OUT("[503 - SEATS__NO] ALCN_WAY_DVS_CD   = [D]");
							if( pPrtData->sati_use_yn[0] == 'Y' )			
							{ // 배차(D) 좌석제(Y)
								LOG_OUT("[503 - SEATS__NO] SATI_USE_YN       = [Y]");	// 좌석제(Y)
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->sats_no);
							}
							else
							{ // 배차(D) 비좌석제(N)
								LOG_OUT("[503 - SEATS__NO] SATI_USE_YN       = [N]");	// 비좌석제(N)
								if( pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y' && pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y' )
								{
									LOG_OUT("[503 - SEATS__NO] Ticket Option     = [Y][Y]");
									txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, "선착순");
								}
								else
								{
									LOG_OUT("[503 - SEATS__NO] Ticket Option     = [Y][N] or [N][Y] or [N][N]");
									txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, " ");
								}
							}
						}
						else	
						{ // 비배차(N)
							LOG_OUT("[503 - SEATS__NO] ALCN_WAY_DVS_CD   = [N]");
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, " ");
						}
					}
					break;
				case PTRG_ID5_RDHM_VAL:		/// (5줄) 승차홈
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->rdhm_val);
					break;
				case PTRG_ID5_BUS_CACM:		/// (5줄) 버스회사
					if(GetEnvOperCorp() == KUMHO_OPER_CORP)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cacm_nm);
					}
					else
					{
						if(pPrtData->alcn_way_dvs_cd[0] == 'D')
						{	/// 배차
							///< 4-5. 버스회사
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cacm_nm);
						}
						else
						{	/// 비배차
							if(pEnv->tTckOpt.nalcn_time_cacm_yn[0] == 'Y')
							{	/// 비배차 : 시간, 운수사 출력유무..
								///< 4-5. 버스회사
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cacm_nm);
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
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;

					///////// 
				case PTRG_ID6_APRV_NO:			/// (6줄) 승인번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->card_aprv_no);
					break;
				case PTRG_ID6_TLE_APRV_NO:		/// (6줄) 타이틀:승인번호
					//sprintf(Buffer, "승인번호: %s", pPrtData->card_aprv_no);	
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->card_aprv_no);	
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_APRV_AMT:			/// (6줄) 승인금액
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->card_aprv_amt);
					break;
				case PTRG_ID6_TLE_APRV_AMT:			/// (6줄) 승인금액
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->card_aprv_amt);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_CARD_NO:				/// (6줄) 카드번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->card_no);
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
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_PUB_TIME:				/// (6줄) 발권시간(시:분:초)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->pub_time);
					break;
				case PTRG_ID6_DEPR_NM:				/// (6줄) 출발지
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->depr_trml_ko_nm);
					break;
				case PTRG_ID6_BAR_DATA:				/// (6줄) 바코드 정보 (pub_dt-pub_shct_trml_cd-pub_wnd_no-pub_sno-secu_code)
					sprintf(Buffer, "%s-%s-%s-%s-%s", pPrtData->bar_pub_dt, pPrtData->bar_pub_shct_trml_cd, pPrtData->bar_pub_wnd_no, pPrtData->bar_pub_sno, pPrtData->bar_secu_code);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_TLE_TRML_CORP_NO:		/// (6줄) 가맹점 사업자번호
					//sprintf(Buffer, "가맹점사업자번호:%s", pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_BUS_CACM:			/// (6줄) 버스회사
					if(GetEnvOperCorp() == KUMHO_OPER_CORP)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cacm_nm);
					}
					else
					{
						if(pPrtData->alcn_way_dvs_cd[0] == 'D')
						{	/// 배차
							///< 4-5. 버스회사
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cacm_nm);
						}
						else
						{	/// 비배차
							if(pEnv->tTckOpt.nalcn_time_cacm_yn[0] == 'Y')
							{	/// 비배차 : 시간, 운수사 출력유무..
								///< 4-5. 버스회사
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cacm_nm);
							}
						}
					}

					//txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cacm_nm);
					break;
				case PTRG_ID6_TLE_BUS_CACM:		/// (6줄) 버스회사:값
					if(GetEnvOperCorp() == KUMHO_OPER_CORP)
					{
						sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->bus_cacm_nm);
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					else
					{
						if(pPrtData->alcn_way_dvs_cd[0] == 'D')
						{	/// 배차
							///< 4-5. 버스회사
							sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->bus_cacm_nm);
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
						}
						else
						{	/// 비배차
							if(pEnv->tTckOpt.nalcn_time_cacm_yn[0] == 'Y')
							{	/// 비배차 : 시간, 운수사 출력유무..
								///< 4-5. 버스회사
								sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->bus_cacm_nm);
								txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
							}
						}
					}
					break;

				case PTRG_ID6_TRML_TEL_NO:		/// (6줄) 버스터미널 연락처  kh_200708
					sprintf(Buffer, "%s:%s",pInfo->szMsg, pPrtData->depr_trml_tel_nm);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_TRML_TEL_NO2:		/// (6줄) (버스터미널 연락처) kh_200708
					sprintf(Buffer, "(%s)",pInfo->szMsg, pPrtData->depr_trml_tel_nm);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_SHRT_PUB_TIME:	/// (6줄) 발권시간(시:분) kh_200709
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->shrt_pub_time);
					break;

				case PTRG_ID6_TLE_DEPR_NM:				/// (6줄) 타이틀 : 출발지 kh_200710
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->depr_trml_ko_nm);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_TLE2_APRV_AMT:			/// (6줄) 타이틀 승인금액(:없는ver) kh_200713
					sprintf(Buffer, "%s %s", pInfo->szMsg, pPrtData->card_aprv_amt);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_TRML_CORP_NO:		/// (6줄) 가맹점 사업자번호 kh_200713 
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);
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
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_TLE2_APRV_NO:		/// (6줄) 타이틀 승인번호(:없는ver) kh_200713	
					sprintf(Buffer, "%s %s", pInfo->szMsg, pPrtData->card_aprv_no);	
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_CACM_BIZ_NO:		/// (6줄) 버스회사 사업자번호 kh_200713
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate,pPrtData->depr_trml_biz_no);
					break;

				case PTRG_ID6_TRML_CORP_NO1:		/// (6줄) 터미널 사업자번호 kh_200720 
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no1);
					break;
				
				case PTRG_ID6_CARD_TR_ADD_INF:		/// (6줄) 시외기프트카드 잔액 add by atectn 20210217
					{
						if(pPrtData->trd_dvs_cd[0] == '3')
						{
							sprintf(Buffer, "(%s원)", pPrtData->card_tr_add_inf);	
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
						}
					}
					break;

				case PTRG_ID6_MSG1:			/// (6줄) 메시지1
				case PTRG_ID6_MSG2:			/// (6줄) 메시지1
				case PTRG_ID6_MSG3:			/// (6줄) 메시지1
				//case PTRG_ID6_MSG4:			/// (6줄) 메시지1 // 20221212 DEL
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;

				// 20221212 ADD~
				case PTRG_ID6_MSG4:			/// (6줄) 사용자정의메세지(상주직원정보)
					{
						int nOffset2 = 0;
						::ZeroMemory(Buffer, sizeof(Buffer));

						/// 상주직원 이름
						if( strlen(pPrtData->rsd_nm) > 0 )
						{
							nOffset2 += sprintf(&Buffer[nOffset2], "%s", pPrtData->rsd_nm);
						}

						/// 상주직원 소속명
						if( strlen(pPrtData->rsd_cmpy_nm) > 0 )
						{
							nOffset2 += sprintf(&Buffer[nOffset2], "(%s)", pPrtData->rsd_cmpy_nm);
						}

						/// 상주직원 연락처
						if( strlen(pPrtData->rsd_tel_nm) > 0 )
						{
							nOffset2 += sprintf(&Buffer[nOffset2], "%s", pPrtData->rsd_tel_nm);
						}
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					break;
				// 20221212 ~ADD

				case PTRG_ID6_BAR_CODE:		/// (6줄) 바코드 (높이: 7mm, 가로크기: n=4)
					{
						int nOffset = 0;

						///< 3-1. 위치
						nOffset = 0;
						CopyMemory(Buffer, (BYTE *)"\x1b\x57", 2);
						nOffset += 2;
						nOffset += sprintf(&Buffer[nOffset], "%04d%04d", pInfo->nX, pInfo->nY);
						TxRxData((BYTE *)Buffer, nOffset);

						///< 3-2. barcode height ('h')						*
						//TxRxData((BYTE *)"\x1D\x68\x3C", 3);	// 20221226 DEL
						// 20221226 MOD~ 
						if( GetEnvOperCorp() == IIAC_OPER_CORP )
						{ // 운영사:2(인천공항)
							TxRxData((BYTE *)"\x1D\x68\x28", 3);	// height:60(0x3C)=>40(0x28) x 0.125mm
						}
						else
						{
							TxRxData((BYTE *)"\x1D\x68\x3C", 3);	// height:60(0x3C) x 0.125mm
						}
						///< 3-3. barcode width ('w')
						TxRxData((BYTE *)"\x1D\x77\x04", 3);	// 4배확대
						
						///< 3-4. barcode type ('k') & barcode data
						nOffset = 0;
						CopyMemory(Buffer, (BYTE *)"\x1D\x6B\x07", 3);
						nOffset += 3;
						nOffset += sprintf(&Buffer[nOffset], "i%s%s%s%s%s", 
											pPrtData->bar_pub_dt, 
											pPrtData->bar_pub_shct_trml_cd, 
											pPrtData->bar_pub_wnd_no, 
											pPrtData->bar_pub_sno, 
											pPrtData->bar_secu_code);
						TxRxData((BYTE *)Buffer, nOffset + 1);
					}
					break;
				// 20221012 ADD
				case PTRG_ID6_BAR_CODE_H6_N3:	/// (6줄) 바코드 (높이: 6.25mm, 가로크기: n=3)
					{
						int nOffset = 0;

						///< 3-1. 위치
						nOffset = 0;
						CopyMemory(Buffer, (BYTE *)"\x1b\x57", 2);
						nOffset += 2;
						nOffset += sprintf(&Buffer[nOffset], "%04d%04d", pInfo->nX, pInfo->nY);
						TxRxData((BYTE *)Buffer, nOffset);

						///< 3-2. barcode height ('h')						*
						TxRxData((BYTE *)"\x1D\x68\x32", 3);
						///< 3-3. barcode width ('w')
						TxRxData((BYTE *)"\x1D\x77\x03", 3);	// n = 3
						
						///< 3-4. barcode type ('k') & barcode data
						nOffset = 0;
						CopyMemory(Buffer, (BYTE *)"\x1D\x6B\x07", 3);
						nOffset += 3;
						nOffset += sprintf(&Buffer[nOffset], "i%s%s%s%s%s", 
											pPrtData->bar_pub_dt, 
											pPrtData->bar_pub_shct_trml_cd, 
											pPrtData->bar_pub_wnd_no, 
											pPrtData->bar_pub_sno, 
											pPrtData->bar_secu_code);
						TxRxData((BYTE *)Buffer, nOffset + 1);
					}					break;
				// 20221012 ~ADD	
				// 20211008 ADD // 승차권(승객용)에 QR코드 추가
				case PTRG_ID6_QR_CODE:		/// (6줄) QRCode
					{
						int nOffset = 0;

						///< 3-1. 페이지모드 인자영역 지정 
						//Print("\x1b\x57%04d%04d", pInfo->nX, pInfo->nY);
						nOffset = 0;
						CopyMemory(Buffer, (BYTE *)"\x1b\x57", 2);
						nOffset += 2;
						nOffset += sprintf(&Buffer[nOffset], "%04d%04d", pInfo->nX, pInfo->nY);
						TxRxData((BYTE *)Buffer, nOffset);
						
						///< 3-2. QR 바코드 
						// cmd = x1A
						// cmd = x42
						// n1 = x02    - 2차원 바코드 종류: PDF417(1), QR코드(2)
						// n2 = x08    - 바코드데이터수, ex) qr_pub_dt(발행일, YYYYMMDD)의 경우 8자리
						// n3 = x01    - 바코드크기: Ver1(1), Ver3(3), Ver5(5), Ver9(9) 
						nOffset = 0;
						CopyMemory(Buffer, (BYTE *)"\x1A\x42\x02\x08\x01", 5);	//Cbus
						nOffset += 5;
						nOffset += sprintf(&Buffer[nOffset], "%s", pPrtData->qr_pub_dt);
						TxRxData((BYTE *)Buffer, nOffset);
						//Print("\x1A\x42\x02\x19\x03%s", Buffer);
					}
					break;
				// 20211008 ~ADD // 승차권(승객용)에 QR코드 추가
				}
			}
		}

		BeginPrint(FALSE);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	LOG_OUT(" end !!!");
	LOG_OUT(" ");
	LOG_OUT(" ");

	return 0;
}

/**
 * @brief		KobusTicketPrint
 * @details		코버스 승차권 프린트
 * @param		char *pData				프린트 데이타
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrtTicketHS_NP::KobusTicketPrint(char *pData)
{
	int					i, k, nLen;
	char				Buffer[500];
	PTCK_PRINT_FMT_T	pPrtData;
	POPER_FILE_CONFIG_T pOperConfig;
	PKIOSK_INI_PTRG_T	pPtrgInf;

	pPrtData = (PTCK_PRINT_FMT_T) pData;
	pOperConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	i = k = nLen = 0;

	LOG_OUT(" start !!!\n\n");
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
					sprintf(Buffer, "0000%s", pPrtData->qr_pub_sno);//kh200708
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID1_USRID:		/// (1줄) 사용자 ID
					sprintf(Buffer, "%s", GetEnvSvrUserNo(SVR_DVS_CCBUS));//kh200708
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID1_USRNM:		/// (1줄) 사용자 Name
					sprintf(Buffer, "(%s)", pPrtData->user_nm);//kh200708
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID1_WNDNO:		/// (1줄) (창구번호+무인기) kh_200710
					sprintf(Buffer, "( %s 무인기 )", pPrtData->qr_pub_wnd_no);
 					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID1_MSG1:			/// (1줄) MSG_1 kh_200708
				case PTRG_ID1_MSG2:			/// (1줄) MSG_2
				case PTRG_ID1_MSG3:			/// (1줄) MSG_3
				case PTRG_ID1_MSG4:			/// (1줄) MSG_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;

				case PTRG_ID2_DEPR_KO:		/// (2줄) 출발지(한글)
					nLen = strlen(pPrtData->depr_trml_ko_nm);
					if(nLen > 0)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->depr_trml_ko_nm);
					}
					break;
				case PTRG_ID2_ARVL_KO:		/// (2줄) 도착지(한글)
					nLen = strlen(pPrtData->arvl_trml_ko_nm);
					if(nLen > 0)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->arvl_trml_ko_nm);
					}
					break;
				case PTRG_ID2_DEPR_KO_1:		/// (2줄) 출발지(한글) - 회수용
					nLen = strlen(pPrtData->depr_trml_ko_nm);
					if(nLen > 0)
					{
						MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pInfo->nRotate, pPrtData->depr_trml_ko_nm);
					}
					break;
				case PTRG_ID2_ARVL_KO_1:		/// (2줄) 도착지(한글) - 승객용
					nLen = strlen(pPrtData->arvl_trml_ko_nm);
					if(nLen > 0)
					{
						MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pInfo->nRotate, pPrtData->arvl_trml_ko_nm);
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
								txtPrint(pInfo->nX - (m * 20), pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
							}
						}
						else
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cls_nm);
						}
					}
					break;
				case PTRG_ID2_DEPR_EN:		/// (2줄) 출발지(영문)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->depr_trml_eng_nm);
					break;
				case PTRG_ID2_ARVL_EN:		/// (2줄) 도착지(영문)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->arvl_trml_eng_nm);
					break;
				case PTRG_ID2_THRU_NM:		/// (2줄) 경유지
// 					nLen = strlen(pPrtData->thru_nm);
// 					if(nLen > 0)
// 					{
// 						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->thru_nm);
// 					}
					break;
				case PTRG_ID2_DIST:			/// (2줄) 거리
					// 					nLen = strlen(pPrtData->thru_nm);
					// 					if(nLen > 0)
					// 					{
					// 						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->thru_nm);
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
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
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
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					break;

				case PTRG_ID2_PYM_DVS:		/// (2줄) 간략결제수단 kh_200709
					if(pPrtData->n_pym_dvs == PYM_CD_CARD)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->shrt_pym_dvs);
					}
					else if(pPrtData->n_pym_dvs == PYM_CD_TPAY)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->shrt_pym_dvs);
					}
					else
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->shrt_pym_dvs);
					}
					break;

				case PTRG_ID2_ROT_NO_ARVL:			/// (2줄) 회수용 노선번호 + 도착지 kh_200717
					{
						if( strlen(pPrtData->arvl_trml_rot_num) > 0 )
						{
							sprintf(Buffer, "%s %s", pPrtData->arvl_trml_rot_num, pPrtData->arvl_trml_ko_nm);
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
						}
						else
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->arvl_trml_ko_nm);
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
								MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, MAX_HANGUL_CHAR - 8, pInfo->nRotate, Buffer);
							}
							else
							{
								;
							}
						}
						else
						{
							MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, MAX_HANGUL_CHAR - 8, pInfo->nRotate, pPrtData->arvl_trml_ko_nm);
						}
					}
					break;

				case PTRG_ID2_BUS_CLS_JUN:				/// (2줄) 전주용 버스등급
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
								txtPrint(pInfo->nX + (m * 20), pInfo->nY, PRT_MODE_NONE, pInfo->nRotate, Buffer);
							}
						}
						else
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cls_nm);
						}
					}
					break;

				case PTRG_ID2_MSG1:			/// (2줄) MSG_1
					// 20211019 ADD
					/// 이벤트쿠폰결제금액	
					if(strlen(pPrtData->cpn_rmn_amt) > 0)
					{
						sprintf(Buffer, "쿠폰:%s ", pPrtData->cpn_rmn_amt);
						LOG_OUT("MSG_ID=PTRG_ID2_MSG1 : %s, strlen(cpn_rmn_amt) : %d", Buffer, strlen(pPrtData->cpn_rmn_amt));
						if (strlen(pPrtData->cpn_rmn_amt) >0)
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
						}
					}
					break;
					// 20211019 ~ADD
				case PTRG_ID2_MSG2:			/// (2줄) MSG_2
					// 20211019 ADD
					///	마일리지결제금액		
					if(strlen(pPrtData->mlg_rmn_amt) > 0)
					{
						sprintf(Buffer, "마일리지:%s ", pPrtData->mlg_rmn_amt);
						LOG_OUT("MSG_ID=PTRG_ID2_MSG2 : %s, strlen(mlg_rmn_amt) : %d", Buffer, strlen(pPrtData->mlg_rmn_amt));
						if (strlen(pPrtData->mlg_rmn_amt) >0)
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
						}
					}
					break;
					// 20211019 ~ADD
				case PTRG_ID2_MSG3:			/// (2줄) MSG_3
				case PTRG_ID2_MSG4:			/// (2줄) MSG_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;

					/////////
				case PTRG_ID3_FARE:			/// (3줄) 요금
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->tisu_amt);
					break;
				case PTRG_ID3_PYM_DVS:		/// (3줄) 결제수단
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->pym_dvs);
					break;
				case PTRG_ID3_TCK_KND:		/// (3줄) 승차권 종류
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_tck_knd_nm);
					break;
				case PTRG_ID3_BUS_CLS:		/// (3줄) (버스등급) kh_200708
					sprintf(Buffer, "(%s)", pPrtData->bus_cls_nm);//
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);//
					break;
				case PTRG_ID3_DRAWBOX:		/// (3줄) 전주용 회수용 드로우박스 kh200713
					DrawBox(FALSE);
					DrawBox(TRUE);
					break;

				case PTRG_ID3_MSG1:			/// (3줄) 메시지_1
				case PTRG_ID3_MSG2:			/// (3줄) 메시지_2
				case PTRG_ID3_MSG3:			/// (3줄) 메시지_3
				case PTRG_ID3_MSG4:			/// (3줄) 메시지_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;

					/////////
				case PTRG_ID5_DEPR_DT:		/// (5줄) 출발일자
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->atl_depr_dt);
					break;
				case PTRG_ID5_DEPR_TIME:	/// (5줄) 출발시간
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->atl_depr_time);
					break;
				case PTRG_ID5_SATS_NO:		/// (5줄) 좌석번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->sats_no);
					break;
				case PTRG_ID5_RDHM_VAL:		/// (5줄) 승차홈
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->rdhm_val);
					break;
				case PTRG_ID5_BUS_CACM:		/// (5줄) 버스회사
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cacm_nm);
					break;
				case PTRG_ID5_FIRST_MSG1:	/// (5줄) 선착순 메시지 1
					break;
				case PTRG_ID5_FIRST_MSG2:	/// (5줄) 선착순 메시지 2
					break;
				case PTRG_ID5_MSG1:			/// (5줄) 메시지 1
				case PTRG_ID5_MSG2:			/// (5줄) 메시지 2
				case PTRG_ID5_MSG3:			/// (5줄) 메시지 3
				case PTRG_ID5_MSG4:			/// (5줄) 메시지 4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;

					///////// 
				case PTRG_ID6_APRV_NO:			/// (6줄) 승인번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->card_aprv_no);
					break;
				case PTRG_ID6_TLE_APRV_NO:		/// (6줄) 타이틀:승인번호
					//sprintf(Buffer, "승인번호: %s", pPrtData->card_aprv_no);	
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->card_aprv_no);	
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_APRV_AMT:			/// (6줄) 승인금액
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->card_aprv_amt);
					break;
				case PTRG_ID6_TLE_APRV_AMT:			/// (6줄) 타이틀 : 승인금액
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->card_aprv_amt);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_CARD_NO:				/// (6줄) 카드번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->card_no);
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
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_PUB_TIME:				/// (6줄) 발권시간(시:분:초)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->pub_time);
					break;
				case PTRG_ID6_DEPR_NM:				/// (6줄) 출발지
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->depr_trml_ko_nm);
					break;
// 				case PTRG_ID6_BAR_DATA:				/// (6줄) 바코드 정보 (pub_dt-pub_shct_trml_cd-pub_wnd_no-pub_sno-secu_code)
// 					sprintf(Buffer, "%s-%s-%s-%s-%s", pPrtData->pub_dt, pPrtData->pub_shct_trml_cd, pPrtData->pub_wnd_no, pPrtData->pub_sno, pPrtData->secu_code);
// 					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
// 					break;
				case PTRG_ID6_QR_DATA:				/// (6줄) QR코드 정보 
					sprintf(Buffer, "%s-%s-%s-%s", pPrtData->qr_pub_dt, pPrtData->qr_pub_trml_cd, pPrtData->qr_pub_wnd_no, pPrtData->qr_pub_sno);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_TLE_TRML_CORP_NO:		/// (6줄) 가맹점 사업자번호
					//sprintf(Buffer, "가맹점사업자번호:%s", pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_BUS_CACM:			/// (6줄) 버스회사
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cacm_nm);
					break;
				case PTRG_ID6_TLE_BUS_CACM:		/// (6줄) 버스회사:값
					//sprintf(Buffer, "버스회사:%s", pPrtData->bus_cacm_nm);
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->bus_cacm_nm);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_SHRT_PUB_TIME:	/// (6줄) 발권시간(시:분)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->shrt_pub_time);
					break;

				case PTRG_ID6_DEPR_ARVL_SATS:	///전주전용 출발코드도착코드자리번호 kh_200710
					sprintf(Buffer, "%s%s%s", pPrtData->qr_depr_trml_no, pPrtData->qr_arvl_trml_no, pPrtData->qr_sats_no);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_TLE_DEPR_NM:				/// (6줄) 타이틀 : 출발지 kh_200710
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->depr_trml_ko_nm);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_TLE2_APRV_AMT:			/// (6줄) 타이틀 승인금액(:없는ver) kh_200713
					sprintf(Buffer, "%s %s", pInfo->szMsg, pPrtData->card_aprv_amt);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_TRML_CORP_NO:		/// (6줄) 가맹점 사업자번호 kh_200713 
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);
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
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_TLE2_APRV_NO:		/// (6줄) 타이틀 승인번호(:없는ver) kh_200713	
					sprintf(Buffer, "%s %s", pInfo->szMsg, pPrtData->card_aprv_no);	
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_CACM_BIZ_NO:		/// (6줄) 버스회사 사업자번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate,pPrtData->bus_cacm_biz_no);
					break;

				case PTRG_ID6_TRML_CORP_NO1:		/// (6줄) 터미널 사업자번호 kh_200720 
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no1);
					break;

				case PTRG_ID6_CARD_TR_ADD_INF:		/// (6줄) 코버스 선불카드 잔액	- 2021/02/17 add
					{
						if( pPrtData->trd_dvs_cd[0] == '3' )
						{
							sprintf(Buffer, "(%s %s원)", pInfo->szMsg, pPrtData->card_tr_add_inf);	
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
						}
					}
					break;

				case PTRG_ID6_MSG1:			/// (6줄) 메시지1
				case PTRG_ID6_MSG2:			/// (6줄) 메시지1
				case PTRG_ID6_MSG3:			/// (6줄) 메시지1
				case PTRG_ID6_MSG4:			/// (6줄) 메시지1
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
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
						int nOffset = 0;

						///< 3-1. 페이지모드 인자영역 지정 
						//Print("\x1b\x57%04d%04d", pInfo->nX, pInfo->nY);
						nOffset = 0;
						CopyMemory(Buffer, (BYTE *)"\x1b\x57", 2);
						nOffset += 2;
						nOffset += sprintf(&Buffer[nOffset], "%04d%04d", pInfo->nX, pInfo->nY);
						TxRxData((BYTE *)Buffer, nOffset);
						
						///< 3-2. QR 바코드 
						nOffset = 0;
						CopyMemory(Buffer, (BYTE *)"\x1A\x42\x02\x19\x03", 5);	//Kobus
						nOffset += 5;
						nOffset += sprintf(&Buffer[nOffset], "%s%s%s%s%s%s%s", 
											pPrtData->qr_pub_dt, 
											pPrtData->qr_pub_trml_cd, 
											pPrtData->qr_pub_wnd_no, 
											pPrtData->qr_pub_sno, 
											pPrtData->qr_depr_trml_no, 
											pPrtData->qr_arvl_trml_no, 
											pPrtData->qr_sats_no);
						TxRxData((BYTE *)Buffer, nOffset);
						//Print("\x1A\x42\x02\x19\x03%s", Buffer);
					}
					break;
				}
			}
		}

		BeginPrint(FALSE);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	LOG_OUT(" end !!!");
	LOG_OUT(" ");
	LOG_OUT(" ");

	return 0;
}

/**
 * @brief		TmExpTicketPrint
 * @details		티머니고속 승차권 프린트
 * @param		char *pData				프린트 데이타
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrtTicketHS_NP::TmExpTicketPrint(char *pData)
{
	int					i, k, nLen;
	char				Buffer[500];
	PTCK_PRINT_FMT_T	pPrtData;
	POPER_FILE_CONFIG_T pOperConfig;
	PKIOSK_INI_PTRG_T	pPtrgInf;

	pPrtData = (PTCK_PRINT_FMT_T) pData;
	pOperConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	i = k = nLen = 0;

	LOG_OUT(" start !!!\n\n");
	m_bExpOnly = FALSE;

	try
	{
		PPTRG_INFO_T pPtrg = NULL;

		BeginPrint(TRUE);

		pPtrgInf = (PKIOSK_INI_PTRG_T) INI_GetEnvTckPtrg(SVR_DVS_TMEXP);

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
					{
						DWORD dwValue;

						dwValue = Util_Ascii2Long(pPrtData->qr_pub_sno, strlen(pPrtData->qr_pub_sno));
						sprintf(Buffer, "%08ld", dwValue);
					}
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID1_USRID:		/// (1줄) 사용자 ID
					sprintf(Buffer, "%s", GetEnvSvrUserNo(SVR_DVS_TMEXP));
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID1_USRNM:		/// (1줄) 사용자 Name
					sprintf(Buffer, "(%s)", pPrtData->user_nm);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID1_WNDNO:		/// (1줄) (창구번호+무인기) kh_200710
					sprintf(Buffer, "( %s 무인기 )", pPrtData->qr_pub_wnd_no);
 					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID1_MSG1:			/// (1줄) MSG_1 kh_200708
				case PTRG_ID1_MSG2:			/// (1줄) MSG_2
				case PTRG_ID1_MSG3:			/// (1줄) MSG_3
				case PTRG_ID1_MSG4:			/// (1줄) MSG_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;

				case PTRG_ID2_DEPR_KO:		/// (2줄) 출발지(한글)
					nLen = strlen(pPrtData->depr_trml_ko_nm);
					if(nLen > 0)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->depr_trml_ko_nm);
					}
					break;
				case PTRG_ID2_ARVL_KO:		/// (2줄) 도착지(한글)
					nLen = strlen(pPrtData->arvl_trml_ko_nm);
					if(nLen > 0)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->arvl_trml_ko_nm);
					}
					break;
				case PTRG_ID2_DEPR_KO_1:		/// (2줄) 출발지(한글) - 회수용
					nLen = strlen(pPrtData->depr_trml_ko_nm);
					if(nLen > 0)
					{
						MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pInfo->nRotate, pPrtData->depr_trml_ko_nm);
					}
					break;
				case PTRG_ID2_ARVL_KO_1:		/// (2줄) 도착지(한글) - 승객용
					nLen = strlen(pPrtData->arvl_trml_ko_nm);
					if(nLen > 0)
					{
						MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pInfo->nRotate, pPrtData->arvl_trml_ko_nm);
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
								txtPrint(pInfo->nX - (m * 20), pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
							}
						}
						else
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cls_nm);
						}
					}
					break;
				case PTRG_ID2_DEPR_EN:		/// (2줄) 출발지(영문)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->depr_trml_eng_nm);
					break;
				case PTRG_ID2_ARVL_EN:		/// (2줄) 도착지(영문)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->arvl_trml_eng_nm);
					break;
				case PTRG_ID2_THRU_NM:		/// (2줄) 경유지
// 					nLen = strlen(pPrtData->thru_nm);
// 					if(nLen > 0)
// 					{
// 						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->thru_nm);
// 					}
/* alcn_arvl_trml_ko_nm 추가정보 출력 테스트 중 !!!!!!!!!!!! (20210813)
					::ZeroMemory(Buffer, sizeof(Buffer));
					::CopyMemory(Buffer, pPrtData->alcn_arvl_trml_no, 3);
					//sprintf(Buffer, "alcn_arvl_trml_no(%s) alcn_arvl_trml_ko_nm(%d)", Buffer, strlen(pPrtData->alcn_arvl_trml_ko_nm));
 					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					nLen = strlen(pPrtData->alcn_arvl_trml_ko_nm);
 					if(nLen > 0)
 					{
 						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->alcn_arvl_trml_ko_nm);
 					}
*/
					break;
				case PTRG_ID2_DIST:			/// (2줄) 거리
					nLen = strlen(pPrtData->thru_nm);
					if(nLen > 0)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->thru_nm);
					}
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
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
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
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					break;

				case PTRG_ID2_PYM_DVS:		/// (2줄) 간략결제수단 kh_200709
					if(pPrtData->n_pym_dvs == PYM_CD_CARD)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->shrt_pym_dvs);
					}
					else if(pPrtData->n_pym_dvs == PYM_CD_TPAY)
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->shrt_pym_dvs);
					}
					else
					{
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->shrt_pym_dvs);
					}
					break;

				case PTRG_ID2_ROT_NO_ARVL:			/// (2줄) 회수용 노선번호 + 도착지 kh_200717
					{
						if( strlen(pPrtData->arvl_trml_rot_num) > 0 )
						{
							sprintf(Buffer, "%s %s", pPrtData->arvl_trml_rot_num, pPrtData->arvl_trml_ko_nm);
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
						}
						else
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->arvl_trml_ko_nm);
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
								MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, MAX_HANGUL_CHAR - 8, pInfo->nRotate, Buffer);
							}
						}
						else
						{
							MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, MAX_HANGUL_CHAR - 8, pInfo->nRotate, pPrtData->arvl_trml_ko_nm);
						}
					}
					break;


				case PTRG_ID2_BUS_CLS_JUN:				/// (2줄) 전주용 버스등급
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
								txtPrint(pInfo->nX + (m * 20), pInfo->nY, PRT_MODE_NONE, pInfo->nRotate, Buffer);
							}
						}
						else
						{
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cls_nm);
						}
					}
					break;

				case PTRG_ID2_MSG1:			/// (2줄) MSG_1
				case PTRG_ID2_MSG2:			/// (2줄) MSG_2
				case PTRG_ID2_MSG3:			/// (2줄) MSG_3
				case PTRG_ID2_MSG4:			/// (2줄) MSG_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;

					/////////
				case PTRG_ID3_FARE:			/// (3줄) 요금
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->tisu_amt);
					break;
				case PTRG_ID3_PYM_DVS:		/// (3줄) 결제수단
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->pym_dvs);
					break;
				case PTRG_ID3_TCK_KND:		/// (3줄) 승차권 종류
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_tck_knd_nm);
					break;
				case PTRG_ID3_BUS_CLS:		/// (3줄) (버스등급) kh_200708
					sprintf(Buffer, "(%s)", pPrtData->bus_cls_nm);//
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);//
					break;
				case PTRG_ID3_DRAWBOX:		/// (3줄) 전주용 회수용 드로우박스 kh200713
					DrawBox(FALSE);
					DrawBox(TRUE);
					break;

				case PTRG_ID3_MSG1:			/// (3줄) 메시지_1
				case PTRG_ID3_MSG2:			/// (3줄) 메시지_2
				case PTRG_ID3_MSG3:			/// (3줄) 메시지_3
				case PTRG_ID3_MSG4:			/// (3줄) 메시지_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;

					/////////
				case PTRG_ID5_DEPR_DT:		/// (5줄) 출발일자
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->atl_depr_dt);
					break;
				case PTRG_ID5_DEPR_TIME:	/// (5줄) 출발시간
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->atl_depr_time);
					break;
				case PTRG_ID5_SATS_NO:		/// (5줄) 좌석번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->sats_no);
					break;
				case PTRG_ID5_RDHM_VAL:		/// (5줄) 승차홈
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->rdhm_val);
					break;
				case PTRG_ID5_BUS_CACM:		/// (5줄) 버스회사
					char tmp[64];
					strncpy(tmp, pPrtData->bus_cacm_nm, sizeof(tmp)-1);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, LeftSubstring(tmp, LEN_LEFTSUBS));	// 운송회사명이 길어질 경우 죄석번호 겹침, 한글 4자(8바이트)만 표시
					//txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cacm_nm);
					break;
				case PTRG_ID5_FIRST_MSG1:	/// (5줄) 선착순 메시지 1
					break;
				case PTRG_ID5_FIRST_MSG2:	/// (5줄) 선착순 메시지 2
					break;
				case PTRG_ID5_MSG1:			/// (5줄) 메시지 1
				case PTRG_ID5_MSG2:			/// (5줄) 메시지 2
				case PTRG_ID5_MSG3:			/// (5줄) 메시지 3
				case PTRG_ID5_MSG4:			/// (5줄) 메시지 4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;

					///////// 
				case PTRG_ID6_APRV_NO:			/// (6줄) 승인번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->card_aprv_no);
					break;
				case PTRG_ID6_TLE_APRV_NO:		/// (6줄) 타이틀:승인번호
					//sprintf(Buffer, "승인번호: %s", pPrtData->card_aprv_no);	
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->card_aprv_no);	
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_APRV_AMT:			/// (6줄) 승인금액
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->card_aprv_amt);
					break;
				case PTRG_ID6_TLE_APRV_AMT:			/// (6줄) 타이틀 : 승인금액
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->card_aprv_amt);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_CARD_NO:				/// (6줄) 카드번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->card_no);
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
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_PUB_TIME:				/// (6줄) 발권시간(시:분:초)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->pub_time);
					break;
				case PTRG_ID6_DEPR_NM:				/// (6줄) 출발지
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->depr_trml_ko_nm);
					break;
				case PTRG_ID6_QR_DATA:				/// (6줄) QR코드 정보 
					sprintf(Buffer, "%s-%s-%s-%s", pPrtData->qr_pub_dt, pPrtData->qr_pub_trml_cd, pPrtData->qr_pub_wnd_no, pPrtData->qr_pub_sno);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_TLE_TRML_CORP_NO:		/// (6줄) 가맹점 사업자번호
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_BUS_CACM:			/// (6줄) 버스회사
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->bus_cacm_nm);
					break;
				case PTRG_ID6_TLE_BUS_CACM:		/// (6줄) 버스회사:값
					//sprintf(Buffer, "버스회사:%s", pPrtData->bus_cacm_nm);
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->bus_cacm_nm);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_SHRT_PUB_TIME:	/// (6줄) 발권시간(시:분)
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pPrtData->shrt_pub_time);
					break;

				case PTRG_ID6_DEPR_ARVL_SATS:	///전주전용 출발코드도착코드자리번호 kh_200710
					sprintf(Buffer, "%s%s%s", pPrtData->qr_depr_trml_no, pPrtData->qr_arvl_trml_no, pPrtData->qr_sats_no);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_TLE_DEPR_NM:				/// (6줄) 타이틀 : 출발지 kh_200710
					sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->depr_trml_ko_nm);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_TLE2_APRV_AMT:			/// (6줄) 타이틀 승인금액(:없는ver) kh_200713
					sprintf(Buffer, "%s %s", pInfo->szMsg, pPrtData->card_aprv_amt);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_TRML_CORP_NO:		/// (6줄) 가맹점 사업자번호 kh_200713 
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no);
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
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_TLE2_APRV_NO:		/// (6줄) 타이틀 승인번호(:없는ver) kh_200713	
					sprintf(Buffer, "%s %s", pInfo->szMsg, pPrtData->card_aprv_no);	
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_CACM_BIZ_NO:		/// (6줄) 버스회사 사업자번호
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate,pPrtData->bus_cacm_biz_no);
					break;

				case PTRG_ID6_TRML_CORP_NO1:		/// (6줄) 터미널 사업자번호 kh_200720 
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pOperConfig->ccTrmlInfo_t.sz_prn_trml_corp_no1);
					break;


				case PTRG_ID6_TLE_REG_NO :
					/// 티머니고속 현영전용
					if(pPrtData->n_pym_dvs == PYM_CD_CSRC)
					{
						sprintf(Buffer, "%s:%s", pInfo->szMsg, pPrtData->card_no);
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					break;

				case PTRG_ID6_BUS_CACM_BIZ_NO :	/// (6줄) 운수회사이름(사업자번호)
					sprintf(Buffer, "%s (%s)", pPrtData->bus_cacm_nm, pPrtData->bus_cacm_biz_no);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_QR_DATA_2 :		/// (6줄) QR Data 끝번호
					sprintf(Buffer, "%s%s%s", pPrtData->qr_depr_trml_no, pPrtData->qr_arvl_trml_no, pPrtData->qr_sats_no);
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID6_CARD_TR_ADD_INF:	/// (6줄) 기프트카드 잔액  : 20210217 add by atectn
					{
						if(pPrtData->trd_dvs_cd[0] == '3')
						{
							sprintf(Buffer, "(%s %s원)", pInfo->szMsg, pPrtData->card_tr_add_inf);
							txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
						}
					}
					break;

				case PTRG_ID6_MSG1:			/// (6줄) 메시지1
				case PTRG_ID6_MSG2:			/// (6줄) 메시지1
				case PTRG_ID6_MSG3:			/// (6줄) 메시지1
				case PTRG_ID6_MSG4:			/// (6줄) 메시지1
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;

				case PTRG_ID6_QR_CODE:		/// (6줄) QRCode
					{
						int nOffset = 0;

						///< 3-1. 페이지모드 인자영역 지정 
						//Print("\x1b\x57%04d%04d", pInfo->nX, pInfo->nY);
						nOffset = 0;
						CopyMemory(Buffer, (BYTE *)"\x1b\x57", 2);
						nOffset += 2;
						nOffset += sprintf(&Buffer[nOffset], "%04d%04d", pInfo->nX, pInfo->nY);
						TxRxData((BYTE *)Buffer, nOffset);
						
						///< 3-2. QR 바코드 
						nOffset = 0;
						CopyMemory(Buffer, (BYTE *)"\x1A\x42\x02\x19\x03", 5);	//TmExp
						nOffset += 5;
						nOffset += sprintf(&Buffer[nOffset], "%s%s%s%s%s%s%s", 
											pPrtData->qr_pub_dt, 
											pPrtData->qr_pub_trml_cd, 
											pPrtData->qr_pub_wnd_no, 
											pPrtData->qr_pub_sno, 
											pPrtData->qr_depr_trml_no, 
											pPrtData->qr_arvl_trml_no, 
											pPrtData->qr_sats_no);
						TxRxData((BYTE *)Buffer, nOffset);
						//Print("\x1A\x42\x02\x19\x03%s", Buffer);
					}
					break;
				}
			}
		}

		BeginPrint(FALSE);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	LOG_OUT(" end !!!");
	LOG_OUT(" ");
	LOG_OUT(" ");

	return 0;
}

// 20211116 ADD
/**
 * @brief		TestTicketPrint
 * @details		Test 승차권 프린트
 * @param		char *pData				프린트 데이타
 * @return		성공 : > 0, 실패 : < 0
 */
int CPrtTicketHS_NP::TestTicketPrint(char *pData, int nSvrKind)
{
	int					i, k, nLen, nRet;
	char				Buffer[500];
	POPER_FILE_CONFIG_T pOperConfig;
	PKIOSK_INI_PTRG_T	pPtrgInf;
	PKIOSK_INI_ENV_T	pEnv;

	pOperConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();
	pEnv		= (PKIOSK_INI_ENV_T) GetEnvInfo();

	i = k = nLen = nRet = 0;

	LOG_OUT(" start for TestTicketPrint !!!\n\n");
	m_bExpOnly = FALSE;

	try
	{
		PPTRG_INFO_T pPtrg = NULL;

		BeginPrint(TRUE);

		pPtrgInf = (PKIOSK_INI_PTRG_T) INI_GetEnvTckPtrg( nSvrKind );
		
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
				case PTRG_ID1_PSNO:				// (1줄) 발권 시리얼번호
					sprintf(Buffer, "%s", "00000001");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID1_USRID:			// (1줄) 사용자 ID
					sprintf(Buffer, "%s", GetEnvSvrUserNo(nSvrKind));
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID1_USRNM:			// (1줄) 사용자 Name
					sprintf(Buffer, "%s", "(000)");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID1_WNDNO:			// (1줄) (창구번호+무인기)
					sprintf(Buffer, "%s", "( xx 무인기 )");
 					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;

				case PTRG_ID1_MSG1:				// (1줄) MSG_1
				case PTRG_ID1_MSG2:				// (1줄) MSG_2
				case PTRG_ID1_MSG3:				// (1줄) MSG_3
				case PTRG_ID1_MSG4:				// (1줄) MSG_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;
			//////////////////
				case PTRG_ID2_DEPR_KO:			// (2줄) 출발지(한글)
					if ( nSvrKind == SVR_DVS_CCBUS )
					{
						sprintf(Buffer, "%s", "시외출발지");
					}
					else if ( nSvrKind == SVR_DVS_KOBUS || nSvrKind == SVR_DVS_TMEXP )
					{
						sprintf(Buffer, "%s", "고속출발지");
					}
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID2_ARVL_KO:			// (2줄) 도착지(한글)
					sprintf(Buffer, "%s", "도착지(한글)");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID2_DEPR_KO_1:		// (2줄) 출발지(한글) - 승객용
					if ( nSvrKind == SVR_DVS_CCBUS )
					{
						sprintf(Buffer, "%s", "시외출발지(2줄)");
					}
					else if ( nSvrKind == SVR_DVS_KOBUS || nSvrKind == SVR_DVS_TMEXP )
					{
						sprintf(Buffer, "%s", "고속출발지(2줄)");
					}
					MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID2_ARVL_KO_1:		// (2줄) 도착지(한글) - 승객용
					sprintf(Buffer, "%s", "도착지(2줄)");
					MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, PRT_MODE_NONE, MAX_HANGUL_CHAR - 8, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID2_BUS_CLS:			// (2줄) 버스등급 : 일반, 고속, 심야, 우등, ...
					sprintf(Buffer, "%s", "등급");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID2_DEPR_EN:			// (2줄) 출발지(영문)
					sprintf(Buffer, "%s", "Seoul");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID2_ARVL_EN:			// (2줄) 도착지(영문)
					sprintf(Buffer, "%s", "Dokdo");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID2_THRU_NM:			// (2줄) 경유지
					if ( nSvrKind == SVR_DVS_CCBUS )
					{
						sprintf(Buffer, "%s", "경유지");
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					break;
				case PTRG_ID2_DIST:				// (2줄) 거리
					if ( nSvrKind == SVR_DVS_CCBUS || nSvrKind == SVR_DVS_TMEXP )
					{
						sprintf(Buffer, "%s", "OOOKm");
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					break;
				case PTRG_ID2_DEPR_EN_1:		// (2줄) 출발지(영문)
					sprintf(Buffer, "%s", "Seoul");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID2_ARVL_EN_1:		// (2줄) 도착지(영문)
					sprintf(Buffer, "%s", "Dokdo");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID2_ROT_NO:			// (2줄) 도착지 - 노선번호
					if ( nSvrKind == SVR_DVS_CCBUS )
					{
						sprintf(Buffer, "%s", "1234");
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					break;
				case PTRG_ID2_PYM_DVS:			// (2줄) 간략결제수단 : 카드, 현금, 페이
					sprintf(Buffer, "%s", "결제");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID2_ROT_NO_ARVL:		// (2줄) 회수용 노선번호 + 도착지
					sprintf(Buffer, "%s", "XXXX 도착지");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID2_ROT_NO_ARVL_1:	// (2줄) 승객용 노선번호 + 도착지
					sprintf(Buffer, "%s", "XXXX 도착지(2줄)");
					MultiPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, MAX_HANGUL_CHAR - 8, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID2_MSG1:				// (2줄) MSG_1
					/// 이벤트쿠폰결제금액
					sprintf(Buffer, "%s", "쿠폰:5000");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID2_MSG2:				// (2줄) MSG_2
					///	마일리지결제금액
					sprintf(Buffer, "%s", "마일리지:999999");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID2_MSG3:				// (2줄) MSG_3
				case PTRG_ID2_MSG4:				// (2줄) MSG_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;
			//////////////////
				case PTRG_ID3_FARE:				// (3줄) 요금
					{
						int nValue = 0;
						char szFare[100];

						nValue = 0x59D8;	// 23000
						::ZeroMemory(szFare, sizeof(szFare));
						Util_AmountComma(nValue, szFare);
						sprintf(Buffer, "%s", szFare);	
					}
					//sprintf(Buffer, "%s", "27,000");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID3_PYM_DVS:			// (3줄) 결제수단 : 현장or예매 + 카드or현금or페이
					sprintf(Buffer, "%s", "결제수단");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID3_TCK_KND:			// (3줄) 승차권 종류 : 일반, 우등, 심야 프리미엄
					sprintf(Buffer, "%s", "권종");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID3_BUS_CLS:			// (3줄) (버스등급)
					sprintf(Buffer, "%s", "(등급)");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID3_PYM_DVS_VERTI:	// (3줄) 결제수단 or 버스등급 세로
					if ( nSvrKind == SVR_DVS_CCBUS )
					{
						sprintf(Buffer, "%s", "세");
						txtPrint(pInfo->nX		, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
						sprintf(Buffer, "%s", "로");
						txtPrint(pInfo->nX-48	, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					break;
				case PTRG_ID3_ENMNPP_VERTI:		// 예매 세로 or 예매 관련 세로
					if ( nSvrKind == SVR_DVS_CCBUS )
					{
						sprintf(Buffer, "%s", "인");
						txtPrint(pInfo->nX-(48*0), pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
						sprintf(Buffer, "%s", "터");
						txtPrint(pInfo->nX-(48*1), pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
						sprintf(Buffer, "%s", "넷");
						txtPrint(pInfo->nX-(48*2), pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					break;
				case PTRG_ID3_BUS_CLS_SHCT_NM: /// 짧은 버스등급
					if ( nSvrKind == SVR_DVS_CCBUS )
					{
						sprintf(Buffer, "%s", "N");
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					break;					
				case PTRG_ID3_BUS_CLS_SHCT_NM2:/// (짧은 버스등급)
					if ( nSvrKind == SVR_DVS_CCBUS )
					{
						sprintf(Buffer, "%s", "(S)");
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					break;
				case PTRG_ID3_MSG1:				// (3줄) 메시지_1
				case PTRG_ID3_MSG2:				// (3줄) 메시지_2
				case PTRG_ID3_MSG3:				// (3줄) 메시지_3
				case PTRG_ID3_MSG4:				// (3줄) 메시지_4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;
			//////////////////
				case PTRG_ID5_DEPR_DT:			// (5줄) 출발일자
					sprintf(Buffer, "%s", "YYYY.MM.DD");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID5_DEPR_TIME:		// (5줄) 출발시간
					sprintf(Buffer, "%s", "HH:MM");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID5_SATS_NO:			// (5줄) 좌석번호
					sprintf(Buffer, "%s", "XX");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID5_RDHM_VAL:			// (5줄) 승차홈
					sprintf(Buffer, "%s", "UU");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID5_BUS_CACM:			// (5줄) 버스회사
					sprintf(Buffer, "%s", "버스회사");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID5_FIRST_MSG1:		// (5줄) 선착순 메시지 1
					if ( nSvrKind == SVR_DVS_CCBUS )
					{
						sprintf(Buffer, "%s", "<<선착순 탑승>>");
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					break;
				case PTRG_ID5_FIRST_MSG2:		// (5줄) 선착순 메시지 2
					if ( nSvrKind == SVR_DVS_CCBUS )
					{
						sprintf(Buffer, "%s", "미탑승분은 다음차 이용");
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					break;
				case PTRG_ID5_MSG1:				// (5줄) 메시지 1
				case PTRG_ID5_MSG2:				// (5줄) 메시지 2
				case PTRG_ID5_MSG3:				// (5줄) 메시지 3
				case PTRG_ID5_MSG4:				// (5줄) 메시지 4
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;
			//////////////////
				case PTRG_ID6_APRV_NO:			// (6줄) 승인번호
					sprintf(Buffer, "%s", "45674567");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_TLE_APRV_NO:		// (6줄) 타이틀:승인번호
					sprintf(Buffer, "%s", "승인No:45674567");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_APRV_AMT:			// (6줄) 승인금액
					{	// AmountComma 추가 for 테스트 승차권 출력	// 20211125
						int nValue = 0;
						char szFare[100];

						nValue = 0x59D8;	// 23000
						::ZeroMemory(szFare, sizeof(szFare));
						Util_AmountComma(nValue, szFare);
						sprintf(Buffer, "%s", szFare);	
					}
					//sprintf(Buffer, "%s", "27000");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_TLE_APRV_AMT:		// (6줄) 승인금액
					{	// AmountComma 추가 for 테스트 승차권 출력	// 20211125
						int nValue = 0;
						char szFare[100];

						nValue = 0x59D8;	// 23000
						::ZeroMemory(szFare, sizeof(szFare));
						Util_AmountComma(nValue, szFare);
						sprintf(Buffer, "승인금액:%s", szFare);	
					}
					//sprintf(Buffer, "%s", "승인금액:27,000");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_CARD_NO:			// (6줄) 카드번호
					sprintf(Buffer, "%s", "9876-54**-****-1234");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_TLE_CARD_NO:		// (6줄) 카드번호
					sprintf(Buffer, "%s", "카드No:987654******4321");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_PUB_TIME:			// (6줄) 발권시간(시:분:초)
					sprintf(Buffer, "%s", "(12:34:56)");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_DEPR_NM:			// (6줄) 출발지
					sprintf(Buffer, "%s", "출발지");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_BAR_DATA:			// (6줄) 바코드 정보 (pub_dt-pub_shct_trml_cd-pub_wnd_no-pub_sno-secu_code)
					if ( nSvrKind == SVR_DVS_CCBUS )
					{
						sprintf(Buffer, "%s", "20211000-0500-54-0001-01");
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					break;
				case PTRG_ID6_TLE_TRML_CORP_NO:	// (6줄) 가맹점 사업자번호
					sprintf(Buffer, "%s", "사업자No:321-32-12214");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_BUS_CACM:			// (6줄) 버스회사
					sprintf(Buffer, "%s", "버스회사");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_TLE_BUS_CACM:		// (6줄) 버스회사:값
					sprintf(Buffer, "%s", "버스회사:운수사명");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_QR_DATA:			// (6줄) QR코드 정보 (qr_pub_dt, qr_pub_trml_cd, qr_pub_wnd_no, qr_pub_sno)
					if ( nSvrKind == SVR_DVS_KOBUS || nSvrKind == SVR_DVS_TMEXP )
					{
						sprintf(Buffer, "%s", "20211232-500-40-0001");
						txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					}
					break;
				case PTRG_ID6_TRML_TEL_NO:		// (6줄) 버스터미널 연락처
					sprintf(Buffer, "%s", "000-1234-5678");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_TRML_TEL_NO2:		// (6줄) (버스터미널 연락처)
					sprintf(Buffer, "%s", "(000-1234-5678)");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_SHRT_PUB_TIME:	// (6줄) 발권시간(시:분)
					sprintf(Buffer, "%s", "(HH:MM)");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_TLE_DEPR_NM:		// (6줄) 타이틀 : 출발지
					sprintf(Buffer, "%s", "출발지:출발지명");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_TLE2_APRV_AMT:	// (6줄) 타이틀 승인금액(:없는ver)
					{	// AmountComma 추가 for 테스트 승차권 출력	// 20211125
						int nValue = 0;
						char szFare[100];

						nValue = 0x59D8;	// 23000
						::ZeroMemory(szFare, sizeof(szFare));
						Util_AmountComma(nValue, szFare);
						sprintf(Buffer, "승인금액 %s", szFare);	
					}
					//sprintf(Buffer, "%s", "승인금액 27,000");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_TRML_CORP_NO:		// (6줄) 가맹점 사업자번호
					sprintf(Buffer, "%s", "123-21-12312");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_TLE2_CARD_NO:		// (6줄) 카드번호(:없는ver)
					sprintf(Buffer, "%s", "카드No 000000******0000");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_TLE2_APRV_NO:		// (6줄) 타이틀 승인번호(:없는ver)
					sprintf(Buffer, "%s", "승인No 42390843");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_CACM_BIZ_NO:		// (6줄) 버스회사 사업자번호
					sprintf(Buffer, "%s", "432-43-12354");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_TRML_CORP_NO1:	// (6줄) 터미널 사업자번호
					sprintf(Buffer, "%s", "123-32-12345");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_CARD_TR_ADD_INF:	// (6줄) 시외기프트카드 잔액
					sprintf(Buffer, "%s", "10,100");
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, Buffer);
					break;
				case PTRG_ID6_MSG1:				// (6줄) 메시지1
				case PTRG_ID6_MSG2:				// (6줄) 메시지1
				case PTRG_ID6_MSG3:				// (6줄) 메시지1
				case PTRG_ID6_MSG4:				// (6줄) 메시지1
					txtPrint(pInfo->nX, pInfo->nY, pInfo->nMode, pInfo->nRotate, pInfo->szMsg);
					break;
				case PTRG_ID6_BAR_CODE:			// (6줄) 바코드 (높이: 7mm, 가로크기: n=4)
					{
						int  nOffset = 0;
						char Buf_tmp1[20+1], Buf_tmp2[20+1], Buf_tmp3[20+1], Buf_tmp4[20+1], Buf_tmp5[20+1];

						///< 3-1. 위치
						nOffset = 0;
						CopyMemory(Buffer, (BYTE *)"\x1b\x57", 2);
						nOffset += 2;
						nOffset += sprintf(&Buffer[nOffset], "%04d%04d", pInfo->nX, pInfo->nY);
						TxRxData((BYTE *)Buffer, nOffset);

						///< 3-2. barcode height ('h')
						TxRxData((BYTE *)"\x1D\x68\x3C", 3);
						///< 3-3. barcode width ('w')
						TxRxData((BYTE *)"\x1D\x77\x04", 3);	// 4배확대
						
						///< 3-4. barcode type ('k') & barcode data
						nOffset = 0;
						CopyMemory(Buffer, (BYTE *)"\x1D\x6B\x07", 3);
						nOffset += 3;
						sprintf(Buf_tmp1, "%s", "20211000");
						sprintf(Buf_tmp2, "%s", "0500");
						sprintf(Buf_tmp3, "%s", "54");
						sprintf(Buf_tmp4, "%s", "0001");
						sprintf(Buf_tmp5, "%s", "01");
						nOffset += sprintf(&Buffer[nOffset], "i%s%s%s%s%s", Buf_tmp1, Buf_tmp2, Buf_tmp3, Buf_tmp4, Buf_tmp5);
						TxRxData((BYTE *)Buffer, nOffset + 1);
					}
					break;
				// 20221012 ADD
				case PTRG_ID6_BAR_CODE_H6_N3:	/// (6줄) 바코드 (높이: 6.25mm, 가로크기: n=3)
					{
						int  nOffset = 0;
						char Buf_tmp1[20+1], Buf_tmp2[20+1], Buf_tmp3[20+1], Buf_tmp4[20+1], Buf_tmp5[20+1];

						///< 3-1. 위치
						nOffset = 0;
						CopyMemory(Buffer, (BYTE *)"\x1b\x57", 2);
						nOffset += 2;
						nOffset += sprintf(&Buffer[nOffset], "%04d%04d", pInfo->nX, pInfo->nY);
						TxRxData((BYTE *)Buffer, nOffset);

						///< 3-2. barcode height ('h')
						TxRxData((BYTE *)"\x1D\x68\x32", 3);
						///< 3-3. barcode width ('w')
						TxRxData((BYTE *)"\x1D\x77\x03", 3);	// n = 3
						
						///< 3-4. barcode type ('k') & barcode data
						nOffset = 0;
						CopyMemory(Buffer, (BYTE *)"\x1D\x6B\x07", 3);
						nOffset += 3;
						sprintf(Buf_tmp1, "%s", "20211000");
						sprintf(Buf_tmp2, "%s", "0500");
						sprintf(Buf_tmp3, "%s", "54");
						sprintf(Buf_tmp4, "%s", "0001");
						sprintf(Buf_tmp5, "%s", "01");
						nOffset += sprintf(&Buffer[nOffset], "i%s%s%s%s%s", Buf_tmp1, Buf_tmp2, Buf_tmp3, Buf_tmp4, Buf_tmp5);
						TxRxData((BYTE *)Buffer, nOffset + 1);
					}
					break;
				// 20221012 ~ADD
				case PTRG_ID6_QR_CODE:			// (6줄) QRCode
					{
						int nOffset = 0;
						char Buf_tmp1[20+1], Buf_tmp2[20+1], Buf_tmp3[20+1], Buf_tmp4[20+1], Buf_tmp5[20+1];
						char Buf_tmp6[20+1], Buf_tmp7[20+1];

						///< 3-1. 페이지모드 인자영역 지정 
						nOffset = 0;
						CopyMemory(Buffer, (BYTE *)"\x1b\x57", 2);
						nOffset += 2;
						nOffset += sprintf(&Buffer[nOffset], "%04d%04d", pInfo->nX, pInfo->nY);
						TxRxData((BYTE *)Buffer, nOffset);
						
						///< 3-2. QR 바코드 
						nOffset = 0;
						if ( nSvrKind == SVR_DVS_CCBUS )
						{
							CopyMemory(Buffer, (BYTE *)"\x1A\x42\x02\x08\x01", 5);	//Test
							nOffset += 5;
							nOffset += sprintf(&Buffer[nOffset], "%s", "YYYYMMDD");
						}
						else if ( nSvrKind == SVR_DVS_KOBUS || nSvrKind == SVR_DVS_TMEXP )
						{
							CopyMemory(Buffer, (BYTE *)"\x1A\x42\x02\x19\x03", 5);	//Test
							nOffset += 5;
							sprintf(Buf_tmp1, "%s", "20211232");
							sprintf(Buf_tmp2, "%s", "500");
							sprintf(Buf_tmp3, "%s", "40");
							sprintf(Buf_tmp4, "%s", "0001");
							sprintf(Buf_tmp5, "%s", "500");
							sprintf(Buf_tmp6, "%s", "020");
							sprintf(Buf_tmp7, "%s", "28");
							nOffset += sprintf(&Buffer[nOffset], "%s%s%s%s%s%s%s", Buf_tmp1, Buf_tmp2,
													Buf_tmp3, Buf_tmp4, Buf_tmp5, Buf_tmp6, Buf_tmp7);
						}
						TxRxData((BYTE *)Buffer, nOffset);
					}
					break;
				}
			}
		}

		BeginPrint(FALSE);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	LOG_OUT(" end !!!");
	LOG_OUT(" ");
	LOG_OUT(" ");

	return 0;
}
// 20211116 ~ADD	// TestTicketPrint

