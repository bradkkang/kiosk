// 
// 
// dev_ui_main.cpp : UI Interface MAIN
//

#include "stdafx.h"
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <queue>

#include <usrinc/fbuf.h>
#include "xzzbus_fdl.h"

#include "MyDefine.h"
#include "MyUtil.h"
#include "File_ini.h"
#include "dev_ui_main.h"
#include "MyDataAccum.h"
#include "dev_tr_main.h"
#include "oper_config.h"
#include "oper_kos_file.h"
#include "event_if.h"
#include "svr_main.h"
#include "dev_coin_main.h"
#include "dev_bill_main.h"
#include "dev_dispenser_main.h"
#include "dev_tr_mem.h"

//----------------------------------------------------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------------------------------------------------

#define UI_IPADDRESS	"127.0.0.1"
#define UI_PORT			5005

#define MAX_UI_PACKET	1024 * 10
#define MAX_RECV_TMO	1000

//----------------------------------------------------------------------------------------------------------------------

static CUiLogFile	clsLog;

static BOOL			s_nConnected = FALSE;
static HANDLE		hThread = NULL;
static HANDLE		hAccMutex = NULL;
static DWORD		dwThreadID;

static BYTE			s_SendData[MAX_UI_PACKET];
static BYTE			s_RecvData[MAX_UI_PACKET];
static int			s_nState;
static BOOL			s_bRun;

static BYTE			*s_pPacket;

static int			s_nDebug = 1, s_nCmdDebug = 0;

static CUiSocket	clsSocket;
static queue <UI_QUE_DATA_T> s_QueData;

static PDEV_CFG_T		pEnv = NULL;

//----------------------------------------------------------------------------------------------------------------------

static DWORD WINAPI UiCommThread(void *);

//----------------------------------------------------------------------------------------------------------------------

#define LOG_OUT(fmt, ...)		{ clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ clsLog.LogOut("[%s:%d] ", __FUNCTION__, __LINE__);  clsLog.HexaDump(x, y, z); }

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		LOG_INIT
 * @details		LOG 초기화
 * @param		None
 * @return		항상 = 0
 */
static int LOG_INIT(void)
{
	clsLog.SetData(30, "\\Log\\Ui");
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
 * @brief		SendPacket
 * @details		데이타 전송
 * @param		BYTE *pData		전송 데이타
 * @param		int nLen		전송 데이타길이
 * @return		성공 >= 0, 실패 < 0
 */
static int SendPacket(WORD wCommand, BYTE *pData, int nLen)
{
	int		nRet, nSend, nTotal, nRetry, nOffset, i;
//	BYTE	ch;

	i = nRet = nSend  = nRetry = nTotal = nOffset = 0;
	
	s_pPacket = new BYTE[nLen + 100];
	::ZeroMemory(s_pPacket, nLen + 100);

	// Make Packet
	{
		// 01. Command
		//::CopyMemory(&s_SendData[nOffset], &wCommand, 2);
		::CopyMemory(&s_pPacket[nOffset], &wCommand, 2);
		nOffset += 2;

		// 02. Length
		//::CopyMemory(&s_SendData[nOffset], &nLen, 2);
		::CopyMemory(&s_pPacket[nOffset], &nLen, 2);
		nOffset += 2;

		// 03. DATA 
		//::CopyMemory(&s_SendData[nOffset], pData, nLen);
		::CopyMemory(&s_pPacket[nOffset], pData, nLen);
		nOffset += nLen;
	}

// 	if(nOffset > 100)
// 		Util_TraceHexaDump("UI send", s_pPacket, 100);

	nTotal = nOffset;
	nSend = 0;

	do 
	{
		//nRet = clsSocket.Send(&s_SendData[nSend], nTotal - nSend);
		nRet = clsSocket.Send(&s_pPacket[nSend], nTotal - nSend);
		if(nRet < 0) 
		{
			if(nRet == RET_SNSOCKET_CLOSE) 
			{
				goto fail_proc;
				//return nRet;
			}
			if(++nRetry >= 3) 
			{
				nRet = -1;
				goto fail_proc;
				//return -1;
			}
			continue;
		}
		nSend += nRet;
	} while(nSend < nTotal);

	if(s_nDebug > 0)
	{
		PUI_HEADER_T pHdr;

		//pHdr = (PUI_HEADER_T) &s_SendData;
		pHdr = (PUI_HEADER_T) s_pPacket;

		if( pHdr->wCommand == UI_CMD_POLLING )
		{
			;//LOG_OUT("SendPacket -> Polling");
		}
		else
		{
			LOG_HEXA("SendPacket", s_pPacket, nTotal);
		}
	}

	return nSend;

fail_proc:

	return nRet;
}

/**
 * @brief		GetPacket
 * @details		수신 데이타
 * @param		BYTE *retBuf	receive data
 * @return		성공 : > 0, 실패 : < 0
 */
static int GetPacket(BYTE *retBuf)
{
	int				nRet, nRecv, nOffset, nLen;
	unsigned long	dwTick;
	PUI_HEADER_T	pHeader;

	pHeader = (PUI_HEADER_T) retBuf;

	nRet = nRecv = nOffset = nLen = 0;

	nLen = sizeof(UI_HEADER_T) - 1;
	dwTick = ::GetTickCount();
	while(1) 
	{
		Sleep(5);
		nRet = clsSocket.Recv((BYTE *)pHeader, nLen);
		if(nRet < 0) 
		{
			if(nRet == RET_SNSOCKET_CLOSE) 
			{
				LOG_OUT("1차 GetPacket(), Socket Close !!");
				return nRet;
			}
			if(Util_CheckExpire(dwTick) > MAX_RECV_TMO) 
			{
				LOG_OUT("1차 GetPacket(), Timeout !!");
				return -1;
			}
		} 
		else 
		{
			//LOG_OUT("[%s:%d] 1차 GetPacket(), Receive data length (%d:%d)", __FUNCTION__, __LINE__, nRet, nLen);
			if( nRet == nLen )
			{
				break;
			}
		}
	}

	nOffset = sizeof(UI_HEADER_T) - 1;
	nLen = pHeader->wDataLen;

	dwTick = ::GetTickCount();
	while(1) 
	{
		Sleep(5);
		nRet = clsSocket.Recv(&retBuf[nOffset], nLen);
		if(nRet < 0) 
		{
			if(nRet == RET_SNSOCKET_CLOSE) 
			{
				LOG_OUT("2차 GetPacket(), Socket Close !!");
				return nRet;
			}
			if(Util_CheckExpire(dwTick) > MAX_RECV_TMO) 
			{
				LOG_OUT("2차 GetPacket(), Timeout !!");
				return -1;
			}
		} 
		else 
		{
			//LOG_OUT("[%s:%d] 2차 GetPacket(), Receive data length (%d:%d)", __FUNCTION__, __LINE__, nRet, nLen);
			if( nRet == nLen )
			{
				nOffset += nRet;
				break;
			}
		}
	}

	nRecv = nOffset;

	if(s_nDebug > 0)
	{
		if( pHeader->wCommand == UI_CMD_POLLING )
		{
			;//LOG_OUT("GetPacket -> Polling");
		}
		else
		{
			LOG_HEXA("GetPacket", retBuf, nRecv);
		}
	}

	return nRecv;
}

/**
 * @brief		SndRcvPacket
 * @details		데이타 송신, 수신 처리
 * @param		int nCommand		데이타 전송 Command
 * @param		BYTE *pSendData		데이타 전송 버퍼
 * @param		int nSendLen		데이타 전송 길이
 * @param		BYTE *retBuf		데이타 수신 버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int SndRcvPacket(WORD wCommand, BYTE *pSendData, int nSendLen, BYTE *retBuf)
{
	int				nRet;
//	PUI_HEADER_T	pHeader;

	nRet = SendPacket(wCommand, pSendData, nSendLen);
	nRet = GetPacket(retBuf);
	if(nRet < 0)
	{
		if(nRet == RET_SNSOCKET_CLOSE)
		{
			clsSocket.CloseSocket();
			s_nConnected = FALSE;
		}
		return nRet;
	}

	return nRet;
}

/**
 * @brief		RespOkCancelInfo
 * @details		거래 취소/완료 처리
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespOkCancelInfo(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RECV_OKCANCEL_T pResp;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RECV_OKCANCEL_T) &pHdr->byBody;

	switch(pResp->byTrans)
	{
	case 1 :	/// 예매발권
		if(pResp->byService == 0x01)
		{	// 거래 완료
			Transact_SetState(TR_MRS_COMPLETE_STATE, 0);
		}
		else
		{
			Transact_SetState(TR_MRS_CANCEL_STATE, 0);
		}
		break;
	case 2 :	/// 현장발권
		if(pResp->byService == 0x01)
		{	// 거래 완료
			Transact_SetState(TR_PBTCK_COMPLETE_STATE, 0);
		}
		else
		{
			Transact_SetState(TR_PBTCK_CANCEL_STATE, 0);
		}
		break;
	case 3 :	/// 환불
		if(pResp->byService == 0x01)
		{	// 거래 완료
			Transact_SetState(TR_CANCRY_COMPLETE_STATE, 0);
		}
		else
		{
			Transact_SetState(TR_CANCRY_CANCEL_STATE, 0);
		}
		break;
	}

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	return nRet;
}

/**
 * @brief		RespServiceInfo
 * @details		사용 중/사용중지 처리
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespServiceInfo(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_RECV_SERVICE_T pResp;

	pResp = (PUI_RECV_SERVICE_T) pRecvData;

	if(pResp->byService == 0x00)
	{	// 사용중
 		LOG_OUT("사용중 수신");
		SetCheckEventCode(EC_OUT_SERVICE, FALSE);
	}
	else 
	{	// 사용중지
		LOG_OUT("사용중지 수신");
		SetCheckEventCode(EC_OUT_SERVICE, TRUE);
	}

	byACK = CHAR_ACK;
	nRet = SendPacket(UI_CMD_SERVICE, (BYTE *)&byACK, 1);

	return nRet;
}

/**
 * @brief		RespServiceInfo
 * @details		사용 중/사용중지 처리
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespVersionInfo(BYTE *pRecvData)
{
	int  nRet, nLen;
//	BYTE byACK;
	UI_SND_VERSION_T sPacket;
	char *pBuffer;

	::ZeroMemory(&sPacket, sizeof(UI_SND_VERSION_T));

	sPacket.byResult = CHAR_ACK;

	/// main version
	pBuffer = CConfigTkMem::GetInstance()->main_version;
	nLen = strlen(pBuffer);
	if(nLen > 0)
	{
		::CopyMemory(sPacket.szMainVer, pBuffer, strlen(pBuffer));
	}
	
	/// ui version
	pBuffer = CConfigTkMem::GetInstance()->ui_version;
	nLen = strlen(pBuffer);
	if(nLen > 0)
	{
		::CopyMemory(sPacket.szUiVer, pBuffer, strlen(pBuffer));
	}

	/// etc version
	pBuffer = CConfigTkMem::GetInstance()->etc_version;
	nLen = strlen(pBuffer);
	if(nLen > 0)
	{
		::CopyMemory(sPacket.szEtcVer, pBuffer, strlen(pBuffer));
	}

	nRet = SendPacket(UI_CMD_VERSION_REQ, (BYTE *)&sPacket, sizeof(UI_SND_VERSION_T));

	return nRet;
}

/**
 * @brief		RespMrsMain
 * @details		예매발권 시작	 메뉴 이동
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespMrsMain(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;

	LOG_OUT("RespMrsMain() !!");

	byACK = CHAR_ACK;
	nRet = SendPacket(UI_CMD_MRS_MAIN, (BYTE *)&byACK, 1);

	Transact_SetState(TR_MRS_MAIN_STATE, 0);

	return nRet;
}


/**
 * @brief		RespMrsIcCardRead
 * @details		예매발권 - 신용카드 읽기
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespMrsIcCardRead(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;

	pHdr = (PUI_HEADER_T) pRecvData;

	if(pHdr->byBody == 0x01)
	{	// 신용카드 읽기 start
		LOG_OUT("RespMrsIcCardRead() card read !!");
		Transact_SetState(TR_MRS_CARD_READ_STATE, 0);
	}
	else 
	{	// 신용카드 읽기 stop
		LOG_OUT("RespMrsIcCardRead() card stop !!");
		Transact_SetState(TR_MRS_CARD_READ_STATE, 2);
	}

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	return nRet;
}

/**
 * @brief		RespMrsInput
 * @details		예매발권 - 예매번호 입력
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespMrsInput(BYTE *pRecvData)
{
// 	int  nRet;
// 	BYTE byACK;
// 	PUI_HEADER_T pHdr;
// 	PUI_RESP_MRS_INPUT_T pResp;
// 
// 	pHdr = (PUI_HEADER_T) pRecvData;
// 	pResp = (PUI_RESP_MRS_INPUT_T) &pHdr->byBody;
// 
// 	if(pResp->byFunc == 0x01)
// 	{	/// 예매번호
// 		LOG_OUT("RespMrsInput() 예매번호 !!");
// 		pResp->szMrsNo;
// 	}
// 	else
// 	{	/// 생년월일 & 핸드폰번호
// 		LOG_OUT("RespMrsInput() 생년월일/핸드폰번호 !!");
// 		pResp->szBirthDay;
// 		pResp->szMobileNo;
// 	}
// 
// 	byACK = CHAR_ACK;
// 	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);
// 
// 	Transact_SetState(TR_MRS_INPUT_STATE, 0);
// 
// 	return nRet;
	return 0;
}

/**
 * @brief		RespMrsReqList
 * @details		예매발권 - 예매 리스트 요청
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespMrsReqList(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_MRS_INFO_T pResp;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_MRS_INFO_T) &pHdr->byBody;

	nRet = 1;
	if(pResp->byFunc == 0x01)
	{	/// 신용카드
		LOG_OUT("RespMrsReqList() 신용카드 !!");
		CMrnpMem::GetInstance()->Base.req_read_dvs_cd[0] = enMNPP_ENC_CARD_DVS_CD;

		::CopyMemory(CMrnpMem::GetInstance()->Base.pos_pg_ktc_ver, pResp->pos_pg_ktc_ver, sizeof(pResp->pos_pg_ktc_ver));
		::CopyMemory(CMrnpMem::GetInstance()->Base.enc_dta_len, &pResp->n_enc_dta_len, sizeof(int));
		::CopyMemory(CMrnpMem::GetInstance()->Base.enc_dta, pResp->enc_dta, sizeof(pResp->enc_dta));
	}
	else if(pResp->byFunc == 0x02)
	{	/// 생년월일
		LOG_OUT("RespMrsReqList() 생년월일 !!");
		CMrnpMem::GetInstance()->Base.req_read_dvs_cd[0] = enMNPP_BRDT_DVS_CD;

		::CopyMemory(CMrnpMem::GetInstance()->Base.mnpp_brdt, pResp->szBirthDay, sizeof(pResp->szBirthDay));
		::CopyMemory(CMrnpMem::GetInstance()->Base.mnpp_tel_no, pResp->szMobileNo, sizeof(pResp->szMobileNo));
	}
	else
	{	/// 예매번호
		LOG_OUT("RespMrsReqList() 예매번호 !!");
		CMrnpMem::GetInstance()->Base.req_read_dvs_cd[0] = enMNPP_MRS_NO_DVS_CD;

		::CopyMemory(CMrnpMem::GetInstance()->Base.mrs_no, pResp->szMrsNo, sizeof(pResp->szMrsNo));

	}

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	Transact_SetState(TR_MRS_LIST_STATE, 0);

	return nRet;
}

/**
 * @brief		RespMrsTckIssue
 * @details		예매발권 - 예매 발권 시작	(UI-205)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespMrsTckIssue(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_MRS_ISSUE_T pResp;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_MRS_ISSUE_T) &pHdr->byBody;

	LOG_OUT("RespMrsTckIssue() !!");

	if(pResp->wCount == 0)
	{
		byACK = CHAR_NAK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);
		
		return 0;
	}
	else
	{
		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);
	}

	// 예매 발권 데이타
	{
		int							i, nCount;
		UI_RESP_MRS_ISSUE_LIST_T	Info;
		PMRNP_BASE_T				pBase;
		PUI_RESP_MRS_ISSUE_LIST_T	pList;

		pBase = &CMrnpMem::GetInstance()->Base;

		::CopyMemory(pBase->depr_trml_nm,		pResp->depr_trml_nm,	 sizeof(pBase->depr_trml_nm));
		::CopyMemory(pBase->depr_trml_eng_nm,	pResp->depr_trml_eng_nm, sizeof(pBase->depr_trml_eng_nm));

		::CopyMemory(pBase->arvl_trml_nm,		pResp->arvl_trml_nm,	 sizeof(pBase->arvl_trml_nm));
		::CopyMemory(pBase->arvl_trml_eng_nm,	pResp->arvl_trml_eng_nm, sizeof(pBase->arvl_trml_eng_nm));

		nCount = pResp->wCount;
		
		pList = (PUI_RESP_MRS_ISSUE_LIST_T) &pResp->ch;

		for(i = 0; i < nCount; i++)
		{
			char Temp[100] = {0, };

			::CopyMemory(&Info, &pList[i], sizeof(UI_RESP_MRS_ISSUE_LIST_T));
			
			::ZeroMemory(Temp, sizeof(Temp));
			::CopyMemory(Temp, pList[i].mrs_no, sizeof(pList[i].mrs_no));
			TRACE("i = %d, mrs_no = %s \n", i, Temp);

			CMrnpMem::GetInstance()->m_vtRcvUiIssue.push_back(Info);
		}

		Transact_SetState(TR_MRS_ISSUE_STATE, 0);
	}

	return nRet;
}

/**
 * @brief		RespCancRyMain
 * @details		환불 - 시작
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespCancRyMain(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;

	pHdr = (PUI_HEADER_T) pRecvData;

	LOG_OUT("RespCancRyMain() !!");

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	Transact_SetState(TR_CANCRY_MAIN_STATE, 0);

	return nRet;
}

/**
 * @brief		RespCancRyTckRead
 * @details		환불 - 승차권 읽기
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespCancRyTckRead(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;

	pHdr = (PUI_HEADER_T) pRecvData;

	if(pHdr->byBody == 0x01)
	{	/// 승차권 읽기 - 시작
		LOG_OUT("RespCancRyTckRead() ic_card read start !!");

		Transact_SetState(TR_CANCRY_TCK_READ, 0);
	}
	else
	{	/// 승차권 읽기 - 종료
		LOG_OUT("RespCancRyTckRead() ic_card read stop !!");

		Transact_SetState(TR_CANCRY_TCK_READ, 2);
	}

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);


	return nRet;
}

/**
 * @brief		RespCancRyFare
 * @details		환불 - 카드 또는 현금 환불 처리
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespCancRyFare(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_CANCRY_FARE_T pResp;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_CANCRY_FARE_T) &pHdr->byBody;

	LOG_OUT("RespCancRyFare() 환불금액(%d) !!", pResp->nFare);

	pResp->nOutFare;
	// 환불금액
	//pResp->nFare;

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	{
		int nCount = 1;

		/// 결제수단
		CCancRyTkMem::GetInstance()->ui_pym_dvs_cd[0] = pResp->ui_pym_dvs_cd[0];
		/// 결제금액
		CCancRyTkMem::GetInstance()->n_tot_money = pResp->nFare;
		/// 방출금액
		CCancRyTkMem::GetInstance()->n_chg_money = pResp->nOutFare;
	}
	Transact_SetState(TR_CANCRY_FARE_STATE, 0);

	return nRet;
}

/**
 * @brief		RespPbTckMain
 * @details		[현장발권] - Main 
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespPbTckMain(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;

	pHdr = (PUI_HEADER_T) pRecvData;

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	::CopyMemory(CConfigTkMem::GetInstance()->trml_cd, GetThrmlCode(), sizeof(CConfigTkMem::GetInstance()->trml_cd) - 1);

	Transact_SetState(TR_PBTCK_MAIN_STATE, 0);

	return nRet;
}

/**
 * @brief		RespPbTckReqList
 * @details		[현장발권] - 배차 리스트 요청 
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespPbTckReqList(BYTE *pRecvData)
{
	int nRet = 0;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_PBTCK_REQ_LIST_T pResp;
	CPubTckMem* pPubTr;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_PBTCK_REQ_LIST_T) &pHdr->byBody;

	pPubTr = CPubTckMem::GetInstance();

	pPubTr->Initialize();

	::CopyMemory(pPubTr->base.depr_trml_cd, pResp->depr_trml_cd, sizeof(pResp->depr_trml_cd));
	::CopyMemory(pPubTr->base.arvl_trml_cd, pResp->arvl_trml_cd, sizeof(pResp->arvl_trml_cd));
	::CopyMemory(pPubTr->base.read_dt, pResp->read_dt, sizeof(pResp->read_dt));
	::CopyMemory(pPubTr->base.read_time, pResp->read_time, sizeof(pResp->read_time));
	::CopyMemory(pPubTr->base.bus_cls_cd, pResp->bus_cls_cd, sizeof(pResp->bus_cls_cd));

	/// 배차 리스트 요청

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	Transact_SetState(TR_PBTCK_REQ_LIST, 0);

	return nRet;
}

/**
 * @brief		RespPbTckListSelect
 * @details		[현장발권] - 배차 리스트 선택 
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespPbTckListSelect(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_PBTCK_SELECT_T pResp;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_PBTCK_SELECT_T) &pHdr->byBody;

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	// set data
	{
		PUBTCK_T* pBase;

		pBase = &CPubTckMem::GetInstance()->base;

		/// 노선ID
		::CopyMemory(pBase->rot_id, pResp->rot_id, sizeof(pResp->rot_id));
		/// 노선순번
		::CopyMemory(pBase->rot_sqno, (char *)&pResp->n_rot_sqno, sizeof(pResp->n_rot_sqno));
		/// 배차일자
		::CopyMemory(pBase->alcn_dt, pResp->alcn_dt, sizeof(pResp->alcn_dt));
		/// 배차순번
		::CopyMemory(pBase->alcn_sqno, (char *)&pResp->n_alcn_sqno, sizeof(pResp->n_alcn_sqno));

		/// 출발터미널코드
		::CopyMemory(pBase->depr_trml_cd, pResp->depr_trml_cd, sizeof(pResp->depr_trml_cd));
		/// 도착터미널코드
		::CopyMemory(pBase->arvl_trml_cd, pResp->arvl_trml_cd, sizeof(pResp->arvl_trml_cd));

		/// 출발터미널이름(한글)
		::CopyMemory(pBase->depr_trml_nm, pResp->depr_trml_nm, sizeof(pResp->depr_trml_nm));
		/// 도착터미널코드(한글)
		::CopyMemory(pBase->arvl_trml_nm, pResp->arvl_trml_nm, sizeof(pResp->arvl_trml_nm));

		/// 출발터미널이름(영문)
		::CopyMemory(pBase->depr_trml_eng_nm, pResp->depr_trml_eng_nm, sizeof(pResp->depr_trml_eng_nm));
		/// 도착터미널코드(영문)
		::CopyMemory(pBase->arvl_trml_eng_nm, pResp->arvl_trml_eng_nm, sizeof(pResp->arvl_trml_eng_nm));
	}

	Transact_SetState(TR_PBTCK_LIST_SEL, 0);

	return nRet;
}

/**
 * @brief		RespPbTckSeatSelect
 * @details		[현장발권] - 좌석 정보 선택 (305)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespPbTckSeatSelect(BYTE *pRecvData)
{
	int  nRet, nCount, i;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_PCPYSATS_T pResp;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_PCPYSATS_T) &pHdr->byBody;

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	{
		CPubTckMem* pTr;

		pTr = CPubTckMem::GetInstance();

		// 총 발권금액
		pTr->base.nTotalMoney = (int) pResp->dwTotalTisuAmt;

		/// 노선ID
		::CopyMemory(pTr->base.rot_id, pResp->rot_id, sizeof(pResp->rot_id));
		/// 노선순번
		::CopyMemory(pTr->base.rot_sqno, &pResp->n_rot_sqno, sizeof(int));
		/// 배차일자
		::CopyMemory(pTr->base.alcn_dt, pResp->alcn_dt, sizeof(pResp->alcn_dt));
		/// 배차순번
		::CopyMemory(pTr->base.alcn_sqno, &pResp->n_alcn_sqno, sizeof(int));
		/// 도착터미널코드
		::CopyMemory(pTr->base.arvl_trml_cd, pResp->arvl_trml_cd, sizeof(pResp->arvl_trml_cd));

		/// 발행매수
		pTr->n_ui_sats_num = nCount = (int) pResp->wCount;

		{
			LOG_OUT("총 발권금액 = %d ", pTr->base.nTotalMoney);
			LOG_OUT("노선ID = %.*s ", sizeof(pTr->base.rot_id), pTr->base.rot_id);
			LOG_OUT("노선SNO = %d ", *(int *)pTr->base.rot_sqno);
			LOG_OUT("배차일자 = %.*s ", sizeof(pTr->base.alcn_dt), pTr->base.alcn_dt);
			LOG_OUT("배차순번 = %d ", *(int *)pTr->base.alcn_sqno);
			LOG_OUT("도착터미널코드 = %.*s ", sizeof(pTr->base.arvl_trml_cd), pTr->base.arvl_trml_cd);
			LOG_OUT("발행매수 = %d ", nCount);
		}


		/// 발행정보(=좌석정보)
		pTr->m_vtUiSats.clear();
		for(i = 0; i < nCount; i++)
		{
			UI_SATS_T Info;

			::ZeroMemory(&Info, sizeof(UI_SATS_T));

			/// 버스티켓종류코드
			::CopyMemory(Info.bus_tck_knd_cd, pResp->List[i].bus_tck_knd_cd, sizeof(pResp->List[i].bus_tck_knd_cd));
			/// 좌석번호
			::CopyMemory(Info.sats_no, pResp->List[i].sats_no, sizeof(pResp->List[i].sats_no));
			/// 발권금액
			Info.n_tisu_amt = pResp->List[i].n_tisu_amt;

			LOG_OUT("index = %d ", i);
			LOG_OUT("Info.bus_tck_knd_cd = %s ", Info.bus_tck_knd_cd);
			LOG_OUT("Info.sats_no = %d ", *(WORD *)Info.sats_no);
			LOG_OUT("Info.n_tisu_amt = %d ", Info.n_tisu_amt);

			pTr->FindBusDcKind(Info.bus_tck_knd_cd,  Info.cty_bus_dc_knd_cd, Info.dcrt_dvs_cd);

			// add
			pTr->m_vtUiSats.push_back(Info);
		}
	}

	Transact_SetState(TR_PBTCK_SEAT_SEL, 0);

	return nRet;
}

/**
 * @brief		RespPbTckPaymentSelect
 * @details		[현장발권] - 결제 수단 선택 
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespPbTckPaymentSelect(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;

	pHdr = (PUI_HEADER_T) pRecvData;

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	// 결제수단
	{
		if(pHdr->byBody == 0x01)
		{	// 현금 결제
			CPubTckMem::GetInstance()->base.pyn_mns_dvs_cd[0] = PYM_CD_CASH;
		}
		else
		{	/// 신용카드 결제
			CPubTckMem::GetInstance()->base.pyn_mns_dvs_cd[0] = PYM_CD_CARD;
		}
		//Transact_SetData(CFG_PBTCK_PAY_TYPE, (char *)&pHdr->byBody, 1);
	}
	Transact_SetState(TR_PBTCK_PYM_DVS_SEL, 0);

	return nRet;
}

/**
 * @brief		RespPbTckInsertBill
 * @details		[현장발권] - 지폐 투입
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespPbTckInsertBill(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;

	pHdr = (PUI_HEADER_T) pRecvData;

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	if(pHdr->byBody == 0x01)
	{
		Bill_Enable();
		Transact_SetState(TR_PBTCK_INS_BILL, 0);
	}
	else
	{
		Bill_Inhibit();
	}

	return nRet;
}

/**
 * @brief		RespPbTckCardRead
 * @details		[현장발권] - (현금영수증 또는 신용카드) 읽기 시작/종료
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespPbTckCardRead(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;

	pHdr = (PUI_HEADER_T) pRecvData;

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	Transact_SetState(TR_PBTCK_CSRC_READ, 0);

	return nRet;
}

/**
 * @brief		RespPbTckCsrcNoInput
 * @details		[현장발권] - 현금영수증 번호 입력 (310)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
// static int RespPbTckCsrcNoInput(BYTE *pRecvData)
// {
// 	int  nRet;
// 	BYTE byACK;
// 	PUI_HEADER_T pHdr;
// 	PUI_RESP_CSRC_INPUT_T pResp;
// 
// 	pHdr = (PUI_HEADER_T) pRecvData;
// 	pResp = (PUI_RESP_CSRC_INPUT_T) &pHdr->byBody;
// 
// 	byACK = CHAR_ACK;
// 	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);
// 
// 	{
// 		/// 현금영수증 수기 입력 번호	
// 		::CopyMemory(CPubTckMem::GetInstance()->base.ui_csrc_no, pResp->csrc_no, sizeof(pResp->csrc_no));
// 		/// 0:개인, 1:법인
// 		CPubTckMem::GetInstance()->base.ui_csrc_use[0] = pResp->csrc_use[0];
// 		/// enc_dta_len
// 		::CopyMemory(CPubTckMem::GetInstance()->base.enc_dta_len, &pResp->n_enc_dta_len, sizeof(int));
// 		/// enc_dta
// 		::CopyMemory(CPubTckMem::GetInstance()->base.enc_dta, pResp->enc_dta, sizeof(pResp->enc_dta));
// 	}
// 
// 	Transact_SetState(TR_PBTCK_CSRC_INPUT, 0);
// 
// 	return nRet;
// }

/**
 * @brief		RespPbTckReqPayment
 * @details		[현장발권] - 승차권 결제 요청
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespPbTckReqPayment(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_REQ_PAYMENT_T pResp;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_REQ_PAYMENT_T) &pHdr->byBody;

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	{
		PUBTCK_T* pBase;

		pBase = &CPubTckMem::GetInstance()->base;

		// 총 결제금액
		pBase->nTotalMoney = pResp->nTotalMoney;
		/// 결제 구분코드 1:현금결제, 2:카드결제
		::CopyMemory(pBase->ui_pym_dvs_cd, pResp->ui_pym_dvs_cd, sizeof(pResp->ui_pym_dvs_cd));
		/// 현금영수증 구분코드 - 0:미사용, 1:수기입력, 2:신용카드
		::CopyMemory(pBase->ui_csrc_dvs_cd, pResp->csrc_dvs_cd, sizeof(pResp->csrc_dvs_cd));

		// 1:현금, 2:카드
		if(pBase->ui_pym_dvs_cd[0] == 1)
		{
			if(pBase->ui_csrc_dvs_cd[0] == 0)
			{	/// 미사용
				pBase->pyn_mns_dvs_cd[0] = PYM_CD_CASH;
			}
			else
			{
				pBase->pyn_mns_dvs_cd[0] = PYM_CD_CSRC;
			}
		}
		else
		{
			pBase->pyn_mns_dvs_cd[0] = PYM_CD_CARD;
		}

		///< 현금영수증 수기입력 번호
		::CopyMemory(pBase->ui_csrc_no, pResp->csrc_no, sizeof(pResp->csrc_no));
		///< 0:개인, 1:법인
		::CopyMemory(pBase->ui_csrc_use, pResp->csrc_use, sizeof(pResp->csrc_use));
		/// 거래구분코드			
		::CopyMemory(pBase->trd_dvs_cd, pResp->trd_dvs_cd, sizeof(pResp->trd_dvs_cd));
		/// fallback구분코드	
		::CopyMemory(pBase->fallback_dvs_cd, pResp->fallback_dvs_cd, sizeof(pResp->fallback_dvs_cd));
		///< POS 단말기 버젼
		::CopyMemory(pBase->pos_pg_ktc_ver, pResp->pos_pg_ktc_ver, sizeof(pResp->pos_pg_ktc_ver));
		/// enc 데이터 길이
		::CopyMemory(pBase->enc_dta_len, pResp->enc_dta_len, sizeof(pResp->enc_dta_len));
		/// enc 데이터
		::CopyMemory(pBase->enc_dta, pResp->enc_dta, sizeof(pResp->enc_dta));
		/// emv 데이터
		::CopyMemory(pBase->emv_dta, pResp->emv_dta, sizeof(pResp->emv_dta));
		/// 서명비밀번호여부 - 1:싸인, 2:비밀번호, 3:싸인+비밀번호
		::CopyMemory(pBase->spad_pwd_yn, pResp->spad_pwd_yn, sizeof(pResp->spad_pwd_yn));
		/// 카드비밀번호
		::CopyMemory(pBase->ui_card_pwd, pResp->card_pwd, sizeof(pResp->card_pwd));
		/// 싸인패드데이터
		::CopyMemory(pBase->spad_dta, pResp->spad_dta, sizeof(pResp->spad_dta));		
		/// 싸인패드데이터길이
		sprintf(pBase->spad_dta_len, "%08d", *(int *)pResp->spad_dta_len);
	}

	Transact_SetState(TR_PBTCK_TMAX_ISSUE, 0);

	return nRet;
}

/**
 * @brief		RespPbTckIssue
 * @details		[현장발권] - 승차권 발권
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespPbTckIssue(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;

	pHdr = (PUI_HEADER_T) pRecvData;

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	Transact_SetState(TR_PBTCK_TCK_ISSUE, 0);

	return nRet;
}

/**
 * @brief		RespPbTckChangeMoney
 * @details		[현장발권] - 거스름돈 방출
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespPbTckChangeMoney(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_CHG_MONEY_T pResp;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_CHG_MONEY_T) &pHdr->byBody;

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	CPubTckMem::GetInstance()->base.nTotalMoney = pResp->nTotalMoney;
	CPubTckMem::GetInstance()->base.n_pbtck_chg_money = pResp->nChangeMoney;

	Transact_SetState(TR_PBTCK_CHANGE_MONEY, 0);

	return nRet;
}


/**
 * @brief		RespPbTckCardInfo
 * @details		[현장발권] - 신용카드 정보 (317)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespPbTckCardInfo(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_CREDIT_CARD_INFO_T pResp;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_CREDIT_CARD_INFO_T) &pHdr->byBody;

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	{
		CPubTckMem* pPbTckTr = CPubTckMem::GetInstance();

		// 지불수단코드 : 신용카드
		pPbTckTr->base.pyn_mns_dvs_cd[0] = PYM_CD_CARD;	
		
		/// 거래구분코드
		::CopyMemory(pPbTckTr->base.trd_dvs_cd,			pResp->trd_dvs_cd,		sizeof(pResp->trd_dvs_cd));	
		/// fallback구분코드
		::CopyMemory(pPbTckTr->base.fallback_dvs_cd,	pResp->fallback_dvs_cd, sizeof(pResp->fallback_dvs_cd));       
		/// pos단말기버전정보
		::CopyMemory(pPbTckTr->base.pos_pg_ktc_ver,		pResp->pos_pg_ktc_ver,	sizeof(pResp->pos_pg_ktc_ver));      
		/// enc 데이터 길이		
		::CopyMemory(pPbTckTr->base.enc_dta_len,		pResp->enc_dta_len,		sizeof(pResp->enc_dta_len));
		/// enc 데이터
		::CopyMemory(pPbTckTr->base.enc_dta,			pResp->enc_dta,			sizeof(pResp->enc_dta));
		/// emv 데이터
		::CopyMemory(pPbTckTr->base.emv_dta,			pResp->emv_dta,			sizeof(pResp->emv_dta));
		/// 서명비밀번호여부		
		::CopyMemory(pPbTckTr->base.spad_pwd_yn,		pResp->spad_pwd_yn,		sizeof(pResp->spad_pwd_yn));
		//::CopyMemory(pPbTckTr->base.card pResp->card_pwd			[10];      /// 카드비밀번호			
		/// 싸인패드데이터	
		::CopyMemory(pPbTckTr->base.spad_dta,			pResp->spad_dta,		sizeof(pResp->spad_dta));
		/// 싸인패드데이터길이
		::CopyMemory(pPbTckTr->base.spad_dta_len,		pResp->spad_dta_len,	sizeof(pResp->spad_dta_len));
	}

	Transact_SetState(TR_PBTCK_TMAX_ISSUE, 0);

	return nRet;
}


/**
 * @brief		RespAdminMain
 * @details		[관리자] - 관리자 메인
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminMain(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;

	pHdr = (PUI_HEADER_T) pRecvData;

	if(pHdr->byBody == 0x01)
	{	/// 관리자 기능 시작
		SetCheckEventCode(EC_ADMIN_SERVICE, TRUE);
	}
	else
	{	/// 관리자 기능 종료
		SetCheckEventCode(EC_ADMIN_SERVICE, FALSE);
	}

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	Transact_SetState(TR_PBTCK_CHANGE_MONEY, 0);

	return nRet;
}

/**
 * @brief		RespAdminLogin
 * @details		[관리자] - 관리자 로그인
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminLogin(BYTE *pRecvData)
{
	int  nRet;
	UI_SND_ADMIN_LOGIN_T sPacket;
	PUI_HEADER_T pHdr;
	PUI_RESP_ADMIN_LOGIN_T pResp;

	::ZeroMemory(&sPacket, sizeof(UI_SND_ADMIN_LOGIN_T));

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_ADMIN_LOGIN_T) &pHdr->byBody;

	sPacket.byAck = CHAR_ACK;

	if(pResp->byLogin == 0x01)
	{	/// 관리자 - 로그인
		nRet = Svr_IfSv_213(pResp->szID, pResp->szPwd);
		if(nRet >= 0)
		{
			sPacket.byLogin = 0x01;
			SetCheckEventCode(EC_ADMIN_LOGIN, TRUE);
		}
		else
		{
			sPacket.byAck = CHAR_NAK;
		}
		Transact_SetState(TR_ADMIN_LOGIN, 0);
	}
	else
	{	/// 관리자 - 로그아웃
		nRet = SetCheckEventCode(EC_ADMIN_LOGOUT, FALSE);
		if(nRet < 0)
		{
			sPacket.byAck = CHAR_NAK;
			sPacket.byLogin = 0x02;
		}
		Transact_SetState(TR_ADMIN_LOGOUT, 0);
	}

	nRet = SendPacket(pHdr->wCommand, (BYTE *)&sPacket, sizeof(UI_SND_ADMIN_LOGIN_T));

	return nRet;
}

/**
 * @brief		RespAdminSetFunc
 * @details		[관리자] - 설정관리 - 기능설정
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminSetFunc(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_ADMIN_FUNC_T pResp;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_ADMIN_FUNC_T) &pHdr->byBody;

	POPER_FILE_CONFIG_T pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	///> 예매발권유무 : 사용(Y), 미사용(N)
	pConfig->ReservMenu_t.byUse = (pResp->byMrnpYN == 'Y') ? 1 : 0 ;
	///> 예매내역전체보기 : 사용(Y), 미사용(N)
	pConfig->ReservMenu_t.byAllListView = (pResp->byMrnpAllVwYN == 'Y') ? 1 : 0 ;
	///> 예매수기조회 : 사용(Y), 미사용(N)
	pConfig->ReservMenu_t.byManualView = (pResp->byMrnpManualYN == 'Y') ? 1 : 0 ;			
	///> 당일예매1건 자동발매 : 사용(Y), 미사용(N)
	pConfig->ReservMenu_t.byMrnpAutoIss = (pResp->byMrnpAutoIssYN == 'Y') ? 1 : 0 ;		
	///> 예매1건 세부내역보기 : 사용(Y), 미사용(N)
	pConfig->ReservMenu_t.byMrnpDetailVw = (pResp->byMrnpDetailVwYN == 'Y') ? 1 : 0 ;		
	
	///> 현장발권유무 : 사용(Y), 미사용(N)
	pConfig->IssMenu_t.byUse = (pResp->byPubTckYN == 'Y') ? 1 : 0 ;		
	///> 빠른배차사용유무 : 사용(Y), 미사용(N)
	pConfig->IssMenu_t.byQuickAlcn = (pResp->byQuickAlcnYN == 'Y') ? 1 : 0 ;

	///> 환불유무 : 사용(Y), 미사용(N)
	pConfig->refundMenu_t.byUse = (pResp->byRefundYN == 'Y') ? 1 : 0 ;
	///> 비배차 : 'N', 배차 : 'D'
	pConfig->baseInfo_t.byAlcn = (pResp->byAlcn == 'D') ? 1 : 0;
	///> 결제방식 (카드:'1',현금:'2',카드+현금:'3')
	pConfig->baseInfo_t.byPayMethod = pResp->byPayMethod - 0x30;

	///> 현금영수증 등록 여부, 사용:'Y', 미사용:'N'
	pConfig->baseInfo_t.byRegCashReceipt = (pResp->byRegCashReceiptYN == 'Y') ? 1 : 0 ;
	///> 5만원미만 카드 무서명, 무서명:'Y', 서명:'N'
	pConfig->baseInfo_t.bySign = (pResp->bySignYN == 'Y') ? 1 : 0 ;
	///> 카드비밀번호, 사용:'Y', 미사용:'N'
	pConfig->baseInfo_t.byCardPasswd = (pResp->byCardPasswdYN == 'Y') ? 1 : 0 ;
	///> 관리자모니터 사용유무 : 사용:'Y', 미사용:'N'
	pConfig->baseInfo_t.byMaintMonitor = (pResp->byMaintMonitorYN == 'Y') ? 1 : 0 ;
	///> 자동마감 유무 : 사용(Y), 미사용(N)
	pConfig->baseInfo_t.byAutoCls = (pResp->byAutoClsYN == 'Y') ? 1 : 0 ;

	SetOperConfigData();

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	return nRet;
}

/**
 * @brief		RespAdminSetEnv
 * @details		[관리자] - 설정관리 - 환경설정	(503)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminSetEnv(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_ADMIN_ENV_T pResp;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_ADMIN_ENV_T) &pHdr->byBody;

	POPER_FILE_CONFIG_T pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	///> 승차권 잔여 최소 수량
	pConfig->limitInfo_t.nTicketMinCount = pResp->nTicketMinCount;
	///> 1만원권 잔여 최소 수량
	pConfig->limitInfo_t.n10k_MinCount = pResp->n10k_MinCount;
	///> 1천원권 잔여 최소 수량
	pConfig->limitInfo_t.n1k_MinCount = pResp->n1k_MinCount;
	///> 500원 잔여 최소 수량
	pConfig->limitInfo_t.n500_MinCount = pResp->n500_MinCount;
	///> 100원 잔여 최소 수량
	pConfig->limitInfo_t.n100_MinCount = pResp->n100_MinCount;
	///> 1회 발권 제한 수량
	pConfig->limitInfo_t.nIssCount = pResp->nIssCount;
	///> 1회 발권 제한 금액
	pConfig->limitInfo_t.nIssMoney = pResp->nIssMoney;
	///> 1회 발권 제한 시간(분)
	pConfig->limitInfo_t.nIssTime = pResp->nIssTime;
	///> 자동마감 시간(hhmmss)
	::CopyMemory(pConfig->baseInfo_t.szAutoWndClose, pResp->szAutoCls, sizeof(pResp->szAutoCls));
	///> 화면 대기 시간(초)
	pConfig->limitInfo_t.nScreenWaitTime = pResp->nScreenWaitTime;
	///> 알림창 대기시간(초)
	pConfig->limitInfo_t.nAlarmWaitTime = pResp->nAlarmWaitTime;
	///> 빠른 배차조회 최소 시간(분)
	pConfig->limitInfo_t.nQuickAlcnTime = pResp->nQuickAlcnTime;
	///> DF1, DF2, ...
	sprintf(pConfig->baseInfo_t.szPrtFmt, "%s", pResp->szPrtFmt);

	SetOperConfigData();

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	return nRet;
}

/**
 * @brief		RespAdminSetThrml
 * @details		[관리자] - 설정관리 - 터미널설정
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminSetThrml(BYTE *pRecvData)
{
	int  nRet, i, nCount;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_ADMIN_THRML_T pResp;
	PUI_RESP_THRML_LIST_T pList;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_ADMIN_THRML_T) &pHdr->byBody;

	nCount = pResp->wCount;	

	pList = (PUI_RESP_THRML_LIST_T) &pResp->byList;
	for(i = 0; i < nCount; i++)
	{
		pList[i].szThrmlCode;
		pList[i].wSeqNo;
		pList[i].byUse;
	}

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	return nRet;
}

/**
 * @brief		RespAdminSetTicket
 * @details		[관리자] - 설정관리 - 승차권설정 (505)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminSetTicket(BYTE *pRecvData)
{
	int  nRet, i, nCount;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_ADMIN_TICKET_T pResp;
	PUI_RESP_TICKET_LIST_T pList;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_ADMIN_TICKET_T) &pHdr->byBody;

	nCount = pResp->wCount;	

	pList = (PUI_RESP_TICKET_LIST_T) &pResp->byList;
	for(i = 0; i < nCount; i++)
	{
		pList[i].szCode;	// 버스티켓종류 코드
		pList[i].byUse;		// 사용유무
	}

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	return nRet;
}

/**
 * @brief		RespAdminSetBusCacm
 * @details		[관리자] - 설정관리 - 운수사설정 (506)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminSetBusCacm(BYTE *pRecvData)
{
	int  nRet, i, nCount;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_ADMIN_BUS_CACM_T pResp;
	PUI_RESP_BUS_CACM_LIST_T pList;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_ADMIN_BUS_CACM_T) &pHdr->byBody;

	nCount = pResp->wCount;

	pList = (PUI_RESP_BUS_CACM_LIST_T) &pResp->byList;
	for(i = 0; i < nCount; i++)
	{
		pList[i].szCode;	/// 운수사 코드
		pList[i].wSeq;		/// 순번
	}

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	return nRet;
}

/**
 * @brief		RespAdminSystem
 * @details		[관리자] - 시스템관리 (511)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminSystem(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	
	PUI_HEADER_T pHdr;

	pHdr = (PUI_HEADER_T) pRecvData;

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	if(pHdr->byBody == 0x01)
	{	/// 프로그램 종료
		CString		strUiName("TicketMachine.exe");

		Util_CloseProcess(strUiName);
		Sleep(1000 * 2);
		exit(0);		
	}
	else if(pHdr->byBody == 0x02)
	{	/// 시스템 종료
		Util_SystemReboot(0);
	}
	else if(pHdr->byBody == 0x03)
	{	/// 시스템 재시작
		Util_SystemReboot(1);
	}
	else if(pHdr->byBody == 0x04)
	{	/// 창구마감
		
		// todo : 시재마감 처리....

		nRet = Svr_IfSv_164();
		if(nRet >= 0)
		{
			// 버스티켓 고유번호 조회
			Svr_IfSv_127();
		}
	}

	return nRet;
}

/**
 * @brief		RespAdminDeviceReset
 * @details		[관리자] - 시스템관리 - 장비별 리셋(512)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminDeviceReset(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;

	pHdr = (PUI_HEADER_T) pRecvData;

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	if(pHdr->byBody == 0x01)
	{	/// 동전방출기 리셋
	}
	else if(pHdr->byBody == 0x02)
	{	/// 지폐입금기 리셋
	}
	else if(pHdr->byBody == 0x03)
	{	/// 지폐방출기 리셋
		
	}
	else if(pHdr->byBody == 0x04)
	{	/// 승차권 스캐너 리셋
	}
	else if(pHdr->byBody == 0x05)
	{	/// 승차권 프린터	리셋
	}
	else if(pHdr->byBody == 0x06)
	{	/// 영수증 프린터 리셋
	}
	else if(pHdr->byBody == 0x07)
	{	/// 신용카드 리더기 리셋
	}
	else if(pHdr->byBody == 0x08)
	{	/// 티맥스 서버 리셋
	}

	return nRet;
}

/**
 * @brief		RespAdminMngTicket
 * @details		[관리자] - 티켓관리 (521)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminMngTicket(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_ADMIN_TCK_INHR_T pResp;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_ADMIN_TCK_INHR_T) &pHdr->byBody;

// 	if(pResp->wTicketCount != 0xFFFF)
// 	{	/// 승차권 수량 설정
// 		ACC_TICKET_DATA_T AccData;
// 		PFILE_ACCUM_T pAccum;
// 
// 		pAccum = GetAccumulateData();
// 
// 		::ZeroMemory(&AccData, sizeof(ACC_TICKET_DATA_T));
// 
// 		AccData.Base.dwTick = Util_GetCurrentTick();
// 		AccData.Base.wKind = ACC_TICKET_ASSIGN;
// 		AccData.nCount = (int) pResp->wTicketCount;
// 		AccData.nSeq = (int) pAccum->tCurrTicket.nSeqNo;
// 
// 		AddAccumulateData(ACC_TICKET_ASSIGN, (char *)&AccData, 1);	
// 	}
	
	{	/// 버스티켓고유번호 설정
		// 버스티켓 교환
		nRet = Svr_IfSv_152(pResp->bus_tck_inhr_no);
	}

	byACK = CHAR_ACK;
	if(nRet < 0)
	{
		byACK = CHAR_NAK;
	}
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	return nRet;
}

/**
 * @brief		RespAdminMngTicketCount
 * @details		[관리자] - 티켓관리 - 승차권 수량 보급 (522)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminMngTicketCount(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_ADMIN_TCK_COUNT_T pResp;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_ADMIN_TCK_COUNT_T) &pHdr->byBody;

 	if(pResp->nTicketCount > 0)
 	{	/// 승차권 수량 설정
 		ACC_TICKET_DATA_T AccData;
 		PFILE_ACCUM_T pAccum;
 
 		pAccum = GetAccumulateData();
 
 		::ZeroMemory(&AccData, sizeof(ACC_TICKET_DATA_T));
 
 		AccData.Base.dwTick = Util_GetCurrentTick();
 		AccData.Base.wKind = ACC_TICKET_ASSIGN;
 		AccData.nCount = (int) pResp->nTicketCount;
 		AccData.nSeq = (int) pAccum->tCurrTicket.nSeqNo;
 
 		AddAccumulateData(ACC_TICKET_ASSIGN, (char *)&AccData, 1);	

		byACK = CHAR_ACK;
	}
	else
	{
		byACK = CHAR_NAK;
	}
	
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	return nRet;
}

//
/**
 * @brief		RespAdminReqTicketInfo
 * @details		[관리자] - 티켓 정보 요청 (523)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminReqTicketInfo(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;

	pHdr = (PUI_HEADER_T) pRecvData;

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	{
		UI_SND_ADMIN_TCK_INFO_T Info;
		PFILE_ACCUM_T pAccum;

		::ZeroMemory(&Info, sizeof(UI_SND_ADMIN_TCK_INFO_T));

		pAccum = GetAccumulateData();

		Info.nTicketCount = pAccum->tCurrTicket.nCount;
		sprintf(Info.bus_tck_inhr_no, "%08d", pAccum->tCurrTicket.nSeqNo);

		// 524
		UI_AddQueueInfo(UI_CMD_ADMIN_RSP_TCK_INFO, (char *) &Info, sizeof(UI_SND_ADMIN_TCK_INFO_T));
	}

	return nRet;
}


/**
 * @brief		RespAdminInquiryIssue
 * @details		[관리자] - 발권내역 조회
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminInquiryIssue(BYTE *pRecvData)
{
	int  nRet;
	char szDate[20], szTime[20];
	UI_SND_ADMIN_INQ_ISSUE_T sPacket;
	PUI_HEADER_T pHdr;

	::ZeroMemory(&sPacket, sizeof(UI_SND_ADMIN_INQ_ISSUE_T));

	pHdr = (PUI_HEADER_T) pRecvData;

	sPacket.byAck = CHAR_ACK;

	if(pHdr->byBody == 0x01)
	{	/// 발권 내역 조회
		SYSTEMTIME st;

		GetLocalTime(&st);
		sprintf(szDate, "%04d%02d%02d", st.wYear, st.wMonth, st.wDay);

		nRet = Svr_IfSv_135(szDate);
		if(nRet < 0)
		{
			sPacket.byAck = CHAR_NAK;
		}
		else
		{
			OperGetFileInfo(OPER_FILE_ID_0135, sPacket.szFileName, &sPacket.nSize);
		}
	}
	else if(pHdr->byBody == 0x02)
	{	/// 재발권 내역 조회
		SYSTEMTIME st;

		GetLocalTime(&st);
		sprintf(szDate, "%04d%02d%02d", st.wYear, st.wMonth, st.wDay);
		sprintf(szTime, "%02d%02d%02d", st.wHour, st.wMinute, st.wSecond);

		nRet = Svr_IfSv_157(szDate, szTime);
		if(nRet < 0)
		{
			sPacket.byAck = CHAR_NAK;
		}
		else
		{
			OperGetFileInfo(OPER_FILE_ID_0157, sPacket.szFileName, &sPacket.nSize);
		}
	}
	else if(pHdr->byBody == 0x03)
	{	/// 환불 내역 조회
		SYSTEMTIME st;

		GetLocalTime(&st);
		sprintf(szDate, "%04d%02d%02d", st.wYear, st.wMonth, st.wDay);
		sprintf(szTime, "%02d%02d%02d", st.wHour, st.wMinute, st.wSecond);

		nRet = Svr_IfSv_155(szDate, szTime);
		if(nRet < 0)
		{
			sPacket.byAck = CHAR_NAK;
		}
		else
		{
			OperGetFileInfo(OPER_FILE_ID_0155, sPacket.szFileName, &sPacket.nSize);
		}
	}
	
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&sPacket, sizeof(UI_SND_ADMIN_INQ_ISSUE_T));


	return nRet;
}

/**
 * @brief		RespAdminReIssue
 * @details		[관리자] - 재발행	(532)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminReIssue(BYTE *pRecvData)
{
	int  nRet;
	char byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_ADMIN_RE_ISSUE_T pResp;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_ADMIN_RE_ISSUE_T) &pHdr->byBody;

	// 서버 버스티켓 재발행
	nRet = Svr_IfSv_136(pResp->pub_dt, pResp->pub_shct_trml_cd, pResp->pub_wnd_no, pResp->pub_sno);
	if(nRet >= 0)
	{
		// 티켓 발행....
		byACK = CHAR_ACK;
	}
	else
	{
		byACK = CHAR_NAK;
	}
	
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	return nRet;
}

/**
 * @brief		RespAdminCashHisto
 * @details		[관리자] - 시재내역 
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminCashHisto(BYTE *pRecvData)
{
	int  nRet = 0;
	PUI_HEADER_T pHdr;

	pHdr = (PUI_HEADER_T) pRecvData;

	{
		// todo : 시재내역 데이타 작성 Accum에서 데이타가져오기
		UI_SND_ADMIN_CASH_HISTO_T sPacket;

		::ZeroMemory(&sPacket, sizeof(UI_SND_ADMIN_CASH_HISTO_T));

		sPacket.byACK = CHAR_ACK;
		sPacket.startInfo;
		sPacket.insInfo;
		sPacket.outInfo;
		sPacket.currInfo;

		nRet = SendPacket(pHdr->wCommand, (BYTE *)&sPacket, sizeof(UI_SND_ADMIN_CASH_HISTO_T));
	}
	
	

	return nRet;
}

/**
 * @brief		RespAdminCashInsert
 * @details		[관리자] - 시재보급 
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminCashInsert(BYTE *pRecvData)
{
	int  nRet;
	char byACK;
	PUI_HEADER_T pHdr;
	PUI_RESP_ADMIN_CASH_INS_T pResp;

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_ADMIN_CASH_INS_T) &pHdr->byBody;

	byACK = CHAR_ACK;

	if(pResp->byPymDvs == 0x01)
	{	// 동전보급
		AddAccumCoinData(ACC_COIN_INSERT, pResp->insInfo.w100, pResp->insInfo.w500);
	}
	else
	{	// 지폐보급
		AddAccumBillData(ACC_BILL_INSERT, pResp->insInfo.w1k, pResp->insInfo.w5k, pResp->insInfo.w10k, pResp->insInfo.w50k);
	}
	
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	return nRet;
}

/**
 * @brief		RespAdminCashOut
 * @details		[관리자] - 시재출금 (543)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminCashOut(BYTE *pRecvData)
{
	int  nRet;
	char byACK;
	UI_SND_ADMIN_CASH_OUT_T sPacket;
	PUI_HEADER_T pHdr;
	PUI_RESP_ADMIN_CASH_OUT_T pResp;

	::ZeroMemory(&sPacket, sizeof(UI_SND_ADMIN_CASH_OUT_T));

	pHdr = (PUI_HEADER_T) pRecvData;
	pResp = (PUI_RESP_ADMIN_CASH_OUT_T) &pHdr->byBody;

	byACK = CHAR_ACK;

	{
		// todo : 시재 출금 동작...
		if( (pResp->cashInfo.w100 + pResp->cashInfo.w500) > 0 )
		{
			nRet = Coin_ChangeMoney((int)pResp->cashInfo.w100, (int)pResp->cashInfo.w500);
			if(nRet < 0)
			{
				sPacket.byACK = CHAR_NAK;
			}
			else
			{
				sPacket.byACK = CHAR_ACK;
				sPacket.cashInfo.w100 = pResp->cashInfo.w100;
				sPacket.cashInfo.w500 = pResp->cashInfo.w500;
			}
		}
		if( (pResp->cashInfo.w1k + pResp->cashInfo.w10k) > 0)
		{
			nRet = CDU_Dispense((int)pResp->cashInfo.w1k, (int)pResp->cashInfo.w10k);
			if(nRet < 0)
			{
				sPacket.byACK = CHAR_NAK;
			}
			else
			{
				sPacket.byACK = CHAR_ACK;
				sPacket.cashInfo.w10k = pResp->cashInfo.w10k;
				sPacket.cashInfo.w50k = pResp->cashInfo.w50k;
			}
		}
	}
	
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&sPacket, sizeof(UI_SND_ADMIN_CASH_OUT_T));

	return nRet;
}

/**
 * @brief		RespAdminInqCashInOut
 * @details		[관리자] - 시재 입.출 내역 (544)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminInqCashInOut(BYTE *pRecvData)
{
	int  nRet;
	UI_SND_ADMIN_INQ_CASH_INOUT_T sPacket;
	PUI_HEADER_T pHdr;

	::ZeroMemory(&sPacket, sizeof(UI_SND_ADMIN_INQ_CASH_INOUT_T));

	pHdr = (PUI_HEADER_T) pRecvData;

	{
		sPacket.byACK = CHAR_ACK;
		sprintf(sPacket.szFileName, "");

		nRet = SendPacket(pHdr->wCommand, (BYTE *)&sPacket, sizeof(UI_SND_ADMIN_INQ_CASH_INOUT_T));
	}
	

	return nRet;
}

/**
 * @brief		RespAdminWithdrawBill
 * @details		[관리자] - 지폐함 회수 (545)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminWithdrawBill(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;

	pHdr = (PUI_HEADER_T) pRecvData;

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	AddAccumBillData(ACC_BILL_WITHDRAW, 0, 0, 0, 0);

	return nRet;
}

/**
 * @brief		RespAdminWithdrawTicket
 * @details		[관리자] - 승차권함 회수 (546)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespAdminWithdrawTicket(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;

	pHdr = (PUI_HEADER_T) pRecvData;

	byACK = CHAR_ACK;
	nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	AddAccumBillData(ACC_TICKET_CLEAR, 0, 0, 0, 0);

	return nRet;
}

/**
 * @brief		ServicePacket
 * @details		수신 Packet 처리
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int ServicePacket(BYTE *pRecvData)
{
	int				nRet = 0;
//	WORD			wCommand;
	//UI_SEND_ACK_T	tPacket;
	PUI_HEADER_T	pHeader;

	pHeader = (PUI_HEADER_T) pRecvData;
	
	if( (pHeader->wCommand & 0xFFFF) != UI_CMD_POLLING )
		TRACE("recv command = %04X\n", pHeader->wCommand & 0xFFFF);

	switch(pHeader->wCommand & 0xFFFF)
	{
	case UI_CMD_POLLING:
		{
			//TRACE("Polling = %04X\n", pHeader->wCommand & 0xFFFF);
		}
		break;
	case UI_CMD_OK_CANCEL:
		{	// 겨래완료 / 취소 / 실패 처리
			TRACE("UI_CMD_OK_CANCEL = %04X\n", pHeader->wCommand & 0xFFFF);
			nRet = RespOkCancelInfo(pRecvData);
		}
		break;
	case UI_CMD_SERVICE:
		{	// 사용중 / 사용중지 
			nRet = RespServiceInfo(pRecvData);
		}
		break;
	case UI_CMD_ALARM:
		break;
	case UI_CMD_ACCOUNT:
		break;
	case UI_CMD_VERSION_REQ:
		{	// 버젼 정보 요청
			nRet = RespVersionInfo(pRecvData);
		}
		break;
	case UI_CMD_DEV_CONFIG:
		break;

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++

	case UI_CMD_MRS_MAIN:
		{	// 예매발권 시작
			nRet = RespMrsMain(pRecvData);
		}
		break;
	case UI_CMD_MRS_CARD_RD:
		{	// 예매발권 - 신용카드 읽기
			nRet = RespMrsIcCardRead(pRecvData);
		}
		break;
// 	case UI_CMD_MRS_INPUT:
// 		{	// 예매발권 - 예매번호 입력
// 			nRet = RespMrsInput(pRecvData);
// 		}
// 		break;
	case UI_CMD_MRS_REQ_LIST:
		{	// 예매발권 - 리스트 요청
			nRet = RespMrsReqList(pRecvData);
		}
		break;
	case UI_CMD_MRS_ISSUE:
		{	// 예매발권 - 발권 시작
			nRet = RespMrsTckIssue(pRecvData);
		}
		break;

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++

	case UI_CMD_PBTCK_MAIN:
		{	// "현장발권" - 시작
			nRet = RespPbTckMain(pRecvData);
		}
		break;
	case UI_CMD_PBTCK_REQ_LIST:
		{	// "현장발권" - 배차 리스트 요청
			nRet = RespPbTckReqList(pRecvData);
		}
		break;
	case UI_CMD_PBTCK_LIST_SEL:
		{	// "현장발권" - 배차 리스트 선택
			nRet = RespPbTckListSelect(pRecvData);
		}
		break;
	case UI_CMD_PBTCK_SEAT_SEL:
		{	// "현장발권" - 좌석 선택 선택
			nRet = RespPbTckSeatSelect(pRecvData);
		}
		break;
	case UI_CMD_PBTCK_PYM_DVS:
		{	// "현장발권" - 결제수단 선택 선택
			nRet = RespPbTckPaymentSelect(pRecvData);
		}
		break;
	case UI_CMD_PBTCK_INS_BILL:
		{	// "현장발권" - 지폐 투입
			nRet = RespPbTckInsertBill(pRecvData);
		}
		break;
// 	case UI_CMD_PBTCK_CSRC_READ:
// 		{	// "현장발권" - (현금영수증 or 신용카드) 읽기 시작/종료
// 			nRet = RespPbTckCardRead(pRecvData);
// 		}
// 		break;
// 	case UI_CMD_PBTCK_CSRC_INPUT:
// 		{	// "현장발권" - 현금영수증 번호 입력
// 			nRet = RespPbTckCsrcNoInput(pRecvData);
// 		}
		break;
	case UI_CMD_PBTCK_REQ_PYM:
		{	// "현장발권" - 승차권 결제 요청
			nRet = RespPbTckReqPayment(pRecvData);
		}
		break;
	case UI_CMD_PBTCK_TCK_ISSUE:
		{	// "현장발권" - 승차권 발권
			nRet = RespPbTckIssue(pRecvData);
		}
		break;
	case UI_CMD_PBTCK_CHG_MONEY:
		{	// "현장발권" - 거스름돈 방출 
			nRet = RespPbTckChangeMoney(pRecvData);
		}
		break;
	case UI_CMD_PBTCK_CARD_INFO:
		{	// "현장발권" - 신용카드 정보 
			nRet = RespPbTckCardInfo(pRecvData);
		}
		break;

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++

	case UI_CMD_CANCRY_MAIN:
		{	// "환불" - 시작
			nRet = RespCancRyMain(pRecvData);
		}
		break;
	case UI_CMD_CANCRY_TCK_READ:
		{	// "환불" - 승차권 판독
			nRet = RespCancRyTckRead(pRecvData);
		}
		break;
	case UI_CMD_CANCRY_FARE:
		{	// 환불 - 환불 처리 (카드 또는 현금)
			nRet = RespCancRyFare(pRecvData);
		}
		break;

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++

	case UI_CMD_ADMIN_MAIN:
		{	// 관리자 - 메인
			nRet = RespAdminMain(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_LOGIN:
		{	// 관리자 - 로그인
			nRet = RespAdminLogin(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_SET_FUNC:
		{	// 관리자 - 기능설정
			nRet = RespAdminSetFunc(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_SET_ENV:
		{	// 관리자 - 환경설정
			nRet = RespAdminSetEnv(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_SET_THRML:
		{	// 관리자 - 터미널설정 ???
			nRet = RespAdminSetThrml(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_SET_TICKET:
		{	// 관리자 - 터미널설정 ???
			nRet = RespAdminSetTicket(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_SET_BUS_CACM:
		{	// 관리자 - 버스 운수사 설정 ???
			nRet = RespAdminSetBusCacm(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_SYSTEM:
		{	// 관리자 - 시스템관리
			nRet = RespAdminSystem(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_DEV_RESET:
		{	// 관리자 - 시스템관리 - 장비별 리셋
			nRet = RespAdminDeviceReset(pRecvData);
		}
		break;

	case UI_CMD_ADMIN_MNG_TCK_INHRNO:
		{	// 관리자 - 티켓관리 - 티켓신규번호
			nRet = RespAdminMngTicket(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_MNG_TCK_COUNT:
		{	// 관리자 - 티켓관리 - 티켓신규번호
			nRet = RespAdminMngTicketCount(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_REQ_TCK_INFO:
		{	// 관리자 - 티켓정보 요청
			nRet = RespAdminReqTicketInfo(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_INQ_ISSUE:
		{	// 관리자 - 발권 내역 조회
			nRet = RespAdminInquiryIssue(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_RE_ISSUE:
		{	// 관리자 - 재발행
			nRet = RespAdminReIssue(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_CASH_HISTO:
		{	// 관리자 - 시재 내역
			nRet = RespAdminCashHisto(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_CASH_INSERT:
		{	// 관리자 - 시재 보급
			nRet = RespAdminCashInsert(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_CASH_OUT:
		{	// 관리자 - 시재 출금
			nRet = RespAdminCashOut(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_INQ_CASH_INOUT:
		{	// 관리자 - 시재 입출내역
			nRet = RespAdminInqCashInOut(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_WITHDRAW_BILL:
		{	// 관리자 - 지폐함 회수
			nRet = RespAdminWithdrawBill(pRecvData);
		}
		break;
	case UI_CMD_ADMIN_WITHDRAW_TCK:
		{	// 관리자 - 승차권함 회수
			nRet = RespAdminWithdrawTicket(pRecvData);
		}
		break;
	}



	return nRet;
}

/**
 * @brief		CmdPollingInfo
 * @details		폴링 명령 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int CmdPollingInfo(void)
{
	int				nRet;
	UI_SEND_POLL_T	sPacket;
	char *pBuff;

	// 창구번호
	pBuff = GetThrmlWndNo();
	sPacket.wTrmlWndNo = (WORD)Util_Ascii2Long(pBuff, strlen(pBuff));
	// 터미널 코드(7)
	::CopyMemory(sPacket.szTrmlCd, GetThrmlCode(), 7); 

	nRet = SndRcvPacket(UI_CMD_POLLING, (BYTE *)&sPacket, (int)sizeof(UI_SEND_POLL_T), s_RecvData);
	if(nRet < 0)
	{
		return nRet;
	}

	nRet = ServicePacket(s_RecvData);

	return nRet;
}

/**
 * @brief		UI_Initialize
 * @details		UI 통신 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int UI_Initialize(void)
{
//	int nRet;
	DWORD dwThreadID;

	LOG_INIT();

	LOG_OUT(" start !!");

	UI_Terminate();

	pEnv = (PDEV_CFG_T) GetEnvUIInfo();
	
	if(hThread != NULL)
	{
		::CloseHandle(hThread);
		hThread = NULL;
	}

	hAccMutex = ::CreateMutex(NULL, FALSE, NULL);
	if(hAccMutex == NULL) 
	{
		LOG_OUT("[%s:%d] CreateMutex() failure..\n", __FUNCTION__, __LINE__);
		return -1;
	}

	hThread = ::CreateThread(NULL, 0, UiCommThread, NULL, CREATE_SUSPENDED, &dwThreadID);
	if(hThread == NULL) 
	{
		return -2;
	}

	::ResumeThread(hThread);

	return 0;
}

/**
 * @brief		UI_Terminate
 * @details		UI 통신 종료
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int UI_Terminate(void)
{
	LOG_OUT(" start !!");

	s_bRun = FALSE;

	if(hThread != NULL)	
	{
		::WaitForSingleObject(hThread, 500);
		::CloseHandle(hThread);
		hThread = NULL;
	}

	if(hAccMutex != NULL)
	{
		::CloseHandle(hAccMutex);
		hAccMutex = NULL;
	}
	return 0;
}

/**
 * @brief		CmdQueueInfo
 * @details		Queue 정보 처리
 * @param		UI_QUE_DATA_T tQue		큐 Info
 * @return		성공 >= 0, 실패 < 0
 */
static int CmdQueueInfo(UI_QUE_DATA_T tQue)
{
	int				nRet;
	PUI_HEADER_T	pPacket;

	if(tQue.nFlag == FLAG_SEND_RECV)
	{
		nRet = SndRcvPacket(tQue.wCommand, (BYTE *)tQue.szData, tQue.nLen, s_RecvData);
		if(nRet < 0)
		{
			return nRet;
		}

		pPacket = (PUI_HEADER_T) s_RecvData;

		if(pPacket->wCommand != tQue.wCommand)
		{
			LOG_OUT("Command Error (%04X:%04X) !!", pPacket->wCommand, tQue.wCommand);
		}
	}
	else if(tQue.nFlag == FLAG_Q_SEND_RECV)
	{
		int nSize, nCount;
		BYTE *pSendBuff = NULL;

		nSize = nCount = 0;

		// todo
		if(tQue.wCommand == UI_CMD_PBTCK_RESP_LIST)
		{	/// 현장발권 - 배차리스트 전달
			nCount = CPubTckMem::GetInstance()->m_vtAlcnInfo.size();
			nSize = 3 + (nCount * sizeof(rtk_readalcn_list_t));

			TRACE("배차리스트 데이타 갯수 = %d \n", nCount);
			LOG_OUT("배차리스트 데이타 갯수 = %d ", nCount);

			pSendBuff = new BYTE[nSize];
			::ZeroMemory(pSendBuff, nSize);

			CPubTckMem::GetInstance()->MakeAlcnListPacket((char *)pSendBuff);
		}
		else
		{
			return -1;
		}

		nRet = SndRcvPacket(tQue.wCommand, pSendBuff, nSize, s_RecvData);
		if(nRet < 0)
		{
			delete[] pSendBuff;

			return nRet;
		}
		
		delete[] pSendBuff;

		pPacket = (PUI_HEADER_T) s_RecvData;

		if(pPacket->wCommand != tQue.wCommand)
		{
			LOG_OUT("Command Error (%04X:%04X) !!", pPacket->wCommand, tQue.wCommand);
		}
	}
	else
	{
		nRet = SendPacket(tQue.wCommand, (BYTE *)tQue.szData, tQue.nLen);
	}

	return nRet;
}

/**
 * @brief		UiCommThread
 * @details		UI 통신 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
DWORD WINAPI UiCommThread(void *)
{
	int				nRet, nTmo, nSize, nStart;
	DWORD			dwConnectTick, dwPollTick;
	UI_QUE_DATA_T	tUiData;

	s_nConnected = FALSE;
	dwConnectTick = 0L;
	dwPollTick = 0;
	nStart = 0;
	s_nState = UI_NONE_STATE; 
	nTmo = 10;
	s_bRun = TRUE;

	// 설정정보
	UI_AddDevConfigData();
	// 장비 회계 정보
	UI_AddDevAccountInfo();
	// 터미널 정보
	UI_AddTrmlInfo();

	while(s_bRun) 
	{
		Sleep(10);	// 10 ms		

		if(s_nConnected == FALSE) 
		{
			if( (dwConnectTick == 0) || (Util_CheckExpire(dwConnectTick) > (1000 * 10)) ) 
			{
//				nRet = clsSocket.ConnectSocket(UI_IPADDRESS, UI_PORT, nTmo);
				nRet = clsSocket.ConnectSocket(pEnv->szIPAddress, pEnv->nTcpPort, nTmo);
				if(nRet < 0) 
				{
					dwConnectTick = ::GetTickCount();
					continue;
				}

				LOG_OUT("UI ConnectSocket Succeed!\n");
				s_nState = UI_CONNECT_STATE;
				s_nConnected = TRUE;
			}
			else
			{
				continue;
			}
		}

		switch(s_nState)
		{
		case UI_CONNECT_STATE :
			{
				// polling 전송
				nRet = CmdPollingInfo();
				s_nState = UI_DATA_STATE;
				dwPollTick = ::GetTickCount();
			}
			break;
		case UI_DATA_STATE :
			{
				nSize = s_QueData.size();
				if(nSize > 0)
				{
					tUiData = s_QueData.front();
					s_QueData.pop();

					CmdQueueInfo(tUiData);
					dwPollTick = ::GetTickCount();
				}
				else
				{
					if(Util_CheckExpire(dwPollTick) > 500)
					{
						// polling
						nRet = CmdPollingInfo();
						dwPollTick = ::GetTickCount();
					}
				}
			}
			break;
		}

	} // while(1) 

	if(s_nConnected == TRUE) 
	{
		clsSocket.CloseSocket();
		s_nConnected = FALSE;
	}

	return 0L;
}

/**
 * @brief		UI_AddAlarmInfo
 * @details		큐에 데이타 저장
 * @param		None
 * @return		항상 : 0
 */
int UI_AddQueueInfo(WORD wCommand, char *pData, int nDataLen)
{
	int				nOffset = 0;
	UI_QUE_DATA_T	queData;

	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = wCommand;
		if(wCommand == UI_CMD_PBTCK_RESP_LIST)
		{
			queData.nFlag = FLAG_Q_SEND_RECV;
		}
		else
		{
			queData.nFlag = FLAG_SEND_RECV;
		}

		// Data
		::CopyMemory(queData.szData, pData, nDataLen);

		// Length
		queData.nLen = nDataLen;

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddServiceInfo
 * @details		사용중/사용중지 정보 전송
 * @param		None
 * @return		항상 : 0
 */
int UI_AddServiceInfo(BOOL bService)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;
//	PVM_ENVINI_T	pIniCfg;

	nOffset = 0;

	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_SERVICE;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		if(bService == TRUE)
		{	// 사용중
			queData.szData[0] = 0x00;
		}
		else
		{	// 사용중지
			queData.szData[0] = 0x01;
		}
		// Length
		queData.nLen = 1;

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddAlarmInfo
 * @details		알람/이벤트 정보 전송
 * @param		None
 * @return		항상 : 0
 */
int UI_AddAlarmInfo(char *pData)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;

	nOffset = 0;
	
	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_ALARM;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		::CopyMemory(queData.szData, pData, sizeof(UI_ALARM_INFO_T));

		// Length
		queData.nLen = sizeof(UI_ALARM_INFO_T);

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddDevAccountInfo
 * @details		장비 회계 정보 전송
 * @param		None
 * @return		항상 : 0
 */
int UI_AddDevAccountInfo(void)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;
	UI_DEV_ACCOUNT_INFO_T Info;

	nOffset = 0;

	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_ACCOUNT;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		{
			PFILE_ACCUM_T pAccum;

			::ZeroMemory(&Info, sizeof(UI_DEV_ACCOUNT_INFO_T));

			pAccum = GetAccumulateData();

			::CopyMemory(&Info.Coin, &pAccum->tCurrCoin, sizeof(DM_COIN_T));
			::CopyMemory(&Info.Dispenser, &pAccum->tCurrDispenser, sizeof(DM_BILL_DISPENSER_T));
			::CopyMemory(&Info.BillBox, &pAccum->tCurrBillBox, sizeof(DM_BILL_T));

			Info.nIssueCount = pAccum->tCurrTodayIssue.tCash.tTotal.nCount + pAccum->tCurrTodayIssue.tCredit.tTotal.nCount;
			Info.nIssueCount += pAccum->tCurrResevIssue.tCash.tTotal.nCount + pAccum->tCurrResevIssue.tCredit.tTotal.nCount;

			Info.nTicketRemain = pAccum->tCurrTicket.nCount;

			::CopyMemory(queData.szData, &Info, sizeof(UI_DEV_ACCOUNT_INFO_T));
		}

		// Length
		queData.nLen = sizeof(UI_DEV_ACCOUNT_INFO_T);

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddDevConfigData
 * @details		장비 환경 설정 정보 전송
 * @param		None
 * @return		항상 : 0
 */
int UI_AddDevConfigData(void)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;
	UI_BASE_INFO_T	Info;
	POPER_FILE_CONFIG_T	pConfig;

	::ZeroMemory(&Info, sizeof(UI_BASE_INFO_T));

	nOffset = 0;

	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_DEV_CONFIG;
		queData.nFlag = FLAG_SEND_RECV;

		pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

		///> 0x01(자립형 1), 0x02(자립형 2), 0x04(매립형) 
		Info.byType		 = (pConfig->baseInfo_t.byType == 0x01) ? '1' : '2';
		///> LCD 갯수
//		Info.byLcdCount		 = pConfig->baseInfo_t.byLcdCount;				
		///> 버스구분 (시외:0x01,고속:0x02,시외/고속:0x03)
		switch(pConfig->baseInfo_t.byBusSep)
		{
		case 1:		Info.byBusSep = '1'; break;
		case 2:		Info.byBusSep = '2'; break;
		default:	Info.byBusSep = '3'; break;
		}
		
		///> 예매발권 유무 : 사용(Y), 미사용(N)
		Info.byMrnpYN			= (pConfig->ReservMenu_t.byUse == 0) ? 'N' : 'Y';
		///> 예매내역전체보기 : 사용(Y), 미사용(N)
		Info.byMrnpAllVwYN		= (pConfig->ReservMenu_t.byAllListView == 0) ? 'N' : 'Y';
		///> 예매수기조회 : 사용(Y), 미사용(N)
		Info.byMrnpManualYN		= (pConfig->ReservMenu_t.byManualView == 0) ? 'N' : 'Y';
		///> 당일예매1건 자동발매 : 사용(Y), 미사용(N)
		Info.byMrnpAutoIssYN	= (pConfig->ReservMenu_t.byMrnpAutoIss == 0) ? 'N' : 'Y';
		///> 예매1건 세부내역보기 : 사용(Y), 미사용(N)
		Info.byMrnpDetailVwYN	= (pConfig->ReservMenu_t.byMrnpDetailVw == 0) ? 'N' : 'Y';

		///> 현장발권 유무 : 사용(Y), 미사용(N)
		Info.byPubTckYN		 = (pConfig->IssMenu_t.byUse == 0) ? 'N' : 'Y';
		///> 빠른배차사용유무 : 사용(Y), 미사용(N)
		Info.byQuickAlcnYN	 = (pConfig->IssMenu_t.byQuickAlcn == 0) ? 'N' : 'Y';

		///> 환불 유무 : 사용(Y), 미사용(N)
		Info.byRefundYN		 = (pConfig->refundMenu_t.byUse == 0) ? 'N' : 'Y';

		///> 비배차 : 0x00, 배차 : 0x01
		Info.byAlcn			 = (pConfig->baseInfo_t.byAlcn == 0) ? 'N' : 'D';

		///> 결제방식 (카드:0x01,현금:0x02,카드+현금:0x04)
		switch(pConfig->baseInfo_t.byPayMethod)
		{
		case 1:		Info.byPayMethod = '1'; break;
		case 2:		Info.byPayMethod = '2'; break;
		default:	Info.byPayMethod = '3'; break;
		}

		///> 현금영수증 등록, 미등록:0, 등록:1
		Info.byRegCashReceiptYN = (pConfig->baseInfo_t.byRegCashReceipt == 0) ? 'N' : 'Y';		
		///> 5만원미만 카드 무서명, 무서명:0, 서명:1
		Info.bySignYN = (pConfig->baseInfo_t.bySign == 0) ? 'N' : 'Y';					
		///> 카드비밀번호, 미사용:0, 사용:1
		Info.byCardPasswdYN = (pConfig->baseInfo_t.byCardPasswd == 0) ? 'N' : 'Y';			
		
		///> 관리자모니터 사용유무 : 사용:'Y', 미사용:'N'
		Info.byMaintMonitorYN = (pConfig->baseInfo_t.byMaintMonitor == 0) ? 'N' : 'Y';			
		///> 자동마감 유무 : 사용(1), 미사용(0)
		Info.byAutoClsYN	 = (pConfig->baseInfo_t.byAutoCls == 0) ? 'N' : 'Y';

		///> 승차권 잔여 최소 수량
		Info.nTicketMinCount = pConfig->limitInfo_t.nTicketMinCount;		
		///> 1만원권 잔여 최소 수량
		Info.n10k_MinCount	 = pConfig->limitInfo_t.n10k_MinCount;			
		///> 1천원권 잔여 최소 수량
		Info.n1k_MinCount	 = pConfig->limitInfo_t.n1k_MinCount;			
		///> 100원 잔여 최소 수량
		Info.n100_MinCount	 = pConfig->limitInfo_t.n100_MinCount;			
		///> 500원 잔여 최소 수량
		Info.n500_MinCount	 = pConfig->limitInfo_t.n500_MinCount;			
		///> 1회 발권 제한 수량
		Info.nIssCount		 = pConfig->limitInfo_t.nIssCount;				
		///> 1회 발권 제한 금액
		Info.nIssMoney		 = pConfig->limitInfo_t.nIssMoney;				
		///> 1회 발권 제한 시간(분)
		Info.nIssTime		 = pConfig->limitInfo_t.nIssTime;				
		///> 자동마감 시간(hhmmss)
		sprintf(Info.szClsTime, "%s", pConfig->baseInfo_t.szAutoWndClose);			
		///> 화면 대기 시간(초)
		Info.nScreenWaitTime = pConfig->limitInfo_t.nScreenWaitTime;		
		///> 알림창 대기시간(초)
		Info.nAlarmWaitTime  = pConfig->limitInfo_t.nAlarmWaitTime;			
		///> 빠른배차조회 최소시간(분)
		Info.nQuickAlcnTime = pConfig->limitInfo_t.nQuickAlcnTime;			

		///> 인쇄종류 (DF1, DF2, DF3...)
		sprintf(Info.szPrtKind, "%s", pConfig->baseInfo_t.szPrtFmt);


		// Data
		::CopyMemory(queData.szData, &Info, sizeof(UI_BASE_INFO_T));
		// Length
		queData.nLen = (int) sizeof(UI_BASE_INFO_T);

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddBaseFileInfo
 * @details		무인발매기 기초 파일 정보 전송
 * @param		None
 * @return		항상 : 0
 */
int UI_AddBaseFileInfo(char *pData)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;

	nOffset = 0;

	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_DEV_CONFIG;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		::CopyMemory(queData.szData, pData, sizeof(UI_BASE_FILE_INFO_T));
		// Length
		queData.nLen = (int) sizeof(VM_ENVINI_T);

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_MrsCardReadInfo
 * @details		예매발권 - 신용카드 읽기 응답
 * @param		None
 * @return		항상 : 0
 */
int UI_MrsCardReadInfo(char *pData, int nDataLen)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;

	nOffset = 0;

	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_MRS_CARD_INFO;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		::CopyMemory(queData.szData, pData, nDataLen);
		// Length
		queData.nLen = nDataLen;

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_MrsTckIssueInfo
 * @details		예매발권 - 발권 정보 전송
 * @param		None
 * @return		항상 : 0
 */
int UI_MrsTckIssueInfo(char *pData, int nDataLen)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;

	nOffset = 0;

	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_MRS_ISSUE_STATE;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		::CopyMemory(queData.szData, pData, nDataLen);
		// Length
		queData.nLen = nDataLen;

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_CancRyChangeMoneyInfo
 * @details		환불 - 카드 또는 현금 방출 정보 전송
 * @param		None
 * @return		항상 : 0
 */
int UI_CancRyChangeMoneyInfo(char *pData, int nDataLen)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;

	nOffset = 0;

	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_CANCRY_FARE_INFO;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		::CopyMemory(queData.szData, pData, nDataLen);
		// Length
		queData.nLen = nDataLen;

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddFailInfo
 * @details		환불 - 카드 또는 현금 방출 정보 전송
 * @param		None
 * @return		항상 : 0
 */
int UI_AddFailInfo(WORD wCommand, char *pCode)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;
	UI_SND_FAIL_INFO_T data;

	nOffset = 0;

	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		::ZeroMemory(&data, sizeof(UI_SND_FAIL_INFO_T));

		queData.wCommand = wCommand;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		data.byAck = CHAR_NAK;
		sprintf(data.szErrCode, "%s", pCode);
		::CopyMemory(queData.szData, &data, sizeof(UI_SND_FAIL_INFO_T));
		// Length
		queData.nLen = sizeof(UI_SND_FAIL_INFO_T);

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_MrsIssueState
 * @details		예매 - 승차권 발권 상태 전송
 * @param		None
 * @return		항상 : 0
 */
int UI_MrsIssueState(BYTE byACK, BYTE byState, char *pSatsNo)
{
	int				nOffset = 0;
	UI_QUE_DATA_T	queData;
	UI_SND_MRS_ISSUE_STATE_T data;

	nOffset = 0;

	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		::ZeroMemory(&data, sizeof(UI_SND_MRS_ISSUE_STATE_T));

		queData.wCommand = UI_CMD_MRS_ISSUE_STATE;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		data.byACK = byACK;
		data.byState = byState;
		data.w_sats_no =  *(WORD *) pSatsNo;
		::CopyMemory(queData.szData, &data, sizeof(UI_SND_MRS_ISSUE_STATE_T));

		// Length
		queData.nLen = sizeof(UI_SND_MRS_ISSUE_STATE_T);

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddServiceInfo
 * @details		사용중/사용중지 정보 전송
 * @param		None
 * @return		항상 : 0
 */
int UI_PbTckIssueState(int nState, int n_sats_no)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;
	UI_SND_PBTCK_ISSUE_STATE_T data;

	nOffset = 0;

	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		::ZeroMemory(&data, sizeof(UI_SND_PBTCK_ISSUE_STATE_T));

		queData.wCommand = UI_CMD_PBTCK_TCK_ISSUE_RET;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		data.byACK = CHAR_ACK;
		data.byState = nState & 0xFF;
		data.bySeat = n_sats_no & 0xFF;
		::CopyMemory(queData.szData, &data, sizeof(UI_SND_PBTCK_ISSUE_STATE_T));

		// Length
		queData.nLen = sizeof(UI_SND_PBTCK_ISSUE_STATE_T);

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_CancRyChangeMoneyInfo
 * @details		환불 - 카드 또는 현금 방출 정보 전송
 * @param		None
 * @return		항상 : 0
 */
int UI_TkPubChangeMoneyInfo(char *pData, int nDataLen)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;

	nOffset = 0;

	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_PBTCK_CHG_MONEY_RET;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		::CopyMemory(queData.szData, pData, nDataLen);
		// Length
		queData.nLen = nDataLen;

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddTrmlInfo
 * @details		파일 정보 : cmd 107
 * @param		None
 * @return		항상 : 0
 */
int UI_AddTrmlInfo(void)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;
	UI_TRML_INFO_T	Info;

	nOffset = 0;

	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_TRML_INFO;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		{
			TERMINAL_INFO_T* pTrmlInfo;

			pTrmlInfo = (TERMINAL_INFO_T*) GetConfigThmlInfo();

			::ZeroMemory(&Info, sizeof(UI_TRML_INFO_T));

			::CopyMemory(Info.trml_wnd_no, GetThrmlWndNo(), sizeof(Info.trml_wnd_no));	///> 창구번호
			::CopyMemory(Info.trml_cd, GetThrmlCode(), sizeof(Info.trml_cd));///> 터미널코드(7바이트)
			::CopyMemory(Info.shct_trml_cd, GetShortThrmlCode(), sizeof(Info.shct_trml_cd));///> 단축 터미널코드
			::CopyMemory(Info.user_no, GetLoginUserNo(), sizeof(Info.user_no));///> user no
			::CopyMemory(Info.user_pwd, GetLoginUserPwd(), sizeof(Info.user_pwd));	///> user pwd
			sprintf(Info.trml_nm, "%s", pTrmlInfo->szName);///> 터미널 명
			sprintf(Info.trml_rprn_tel_no, "%s", pTrmlInfo->szCorpTel);	///> 터미널 전화번호

			::CopyMemory(queData.szData, &Info, sizeof(UI_TRML_INFO_T));
			// Length
			queData.nLen = sizeof(UI_TRML_INFO_T);
		}
		
		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddFileInfo
 * @details		파일 정보 : cmd 108
 * @param		None
 * @return		항상 : 0
 */
int UI_AddFileInfo(int nNumber, int nID)
{
	UI_QUE_DATA_T	queData;
	UI_BASE_FILE_INFO_T uiInfo;

	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		::ZeroMemory(&uiInfo, sizeof(UI_BASE_FILE_INFO_T));

		queData.wCommand = UI_CMD_DEV_FILE_INFO;
		queData.nFlag = FLAG_SEND_RECV;

		uiInfo.wFileID = nNumber;
		OperGetFileInfo(nID, uiInfo.szFullName, (int *)&uiInfo.dwSize);

		// Data
		::CopyMemory(queData.szData, &uiInfo, sizeof(UI_BASE_FILE_INFO_T));
		// Length
		queData.nLen = sizeof(UI_BASE_FILE_INFO_T);

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}


/**
 * @brief		UI_TkPubAlcnFareInfo
 * @details		현장발권 - 배차요금 조회 (0x304)
 * @param		None
 * @return		항상 : 0
 */
int UI_TkPubAlcnFareInfo(BOOL bResult, int nError)
{
	int				nOffset, nCount, i;
	UI_QUE_DATA_T	queData;

	nOffset = nCount = i = 0;

	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_PBTCK_FARE_INFO;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		if(bResult == TRUE)
		{
			CPubTckMem* pTr;
			//pmyrtk_readalcnfee_t pInfo;

			//pInfo = (pmyrtk_readalcnfee_t) Transact_GetData(CFG_RSP_ALCN_FEE_DATA);
			pTr = CPubTckMem::GetInstance();

			// Result
			queData.szData[nOffset++] = CHAR_ACK;

			// 좌석배치도 유형코드
			::CopyMemory(&queData.szData[nOffset], pTr->sats_arct_cd, sizeof(pTr->sats_arct_cd) - 1);
			nOffset += (sizeof(pTr->sats_arct_cd) - 1);

			// 좌석상태정보
			::CopyMemory(&queData.szData[nOffset], pTr->sats_sta_inf, sizeof(pTr->sats_sta_inf) - 1);
			nOffset += (sizeof(pTr->sats_sta_inf) - 1);

			// 상주인원카드 할인여부
			::CopyMemory(&queData.szData[nOffset], pTr->rsd_nop_card_dc_yn, sizeof(pTr->rsd_nop_card_dc_yn) - 1);
			nOffset += (sizeof(pTr->rsd_nop_card_dc_yn) - 1);

			// 요금정보수
			nCount = pTr->n_fee_num;
			::CopyMemory(&queData.szData[nOffset], &pTr->n_fee_num, sizeof(WORD));
			nOffset += sizeof(WORD);

			vector<rtk_readalcnfee_list_t>::iterator	iter;

			for( iter = pTr->m_vtFeeInfo.begin(); iter != pTr->m_vtFeeInfo.end(); iter++ )
			//for(i = 0; i < nCount; i++)
			{
				//prtk_readalcnfee_list_t pList;

				//pList = &pInfo->List[i];

				///< 시외버스할인종류코드
				::CopyMemory(&queData.szData[nOffset], iter->cty_bus_dc_knd_cd, sizeof(iter->cty_bus_dc_knd_cd) - 1);
				nOffset += (sizeof(iter->cty_bus_dc_knd_cd) - 1);

				///< 할인율구분코드
				::CopyMemory(&queData.szData[nOffset], iter->dcrt_dvs_cd, sizeof(iter->dcrt_dvs_cd) - 1);
				nOffset += (sizeof(iter->dcrt_dvs_cd) - 1);

				///< 할인구간다중값
				::CopyMemory(&queData.szData[nOffset], iter->dc_rng_mltp_val, sizeof(iter->dc_rng_mltp_val) - 1);
				nOffset += (sizeof(iter->dc_rng_mltp_val) - 1);

				///< 적용제한매수(number)
				::CopyMemory(&queData.szData[nOffset], iter->adpt_ltn_hcnt, sizeof(WORD));
				nOffset += sizeof(WORD);

				///< 버스티켓종류코드
				::CopyMemory(&queData.szData[nOffset], iter->bus_tck_knd_cd, sizeof(iter->bus_tck_knd_cd) - 1);
				nOffset += (sizeof(iter->bus_tck_knd_cd) - 1);

				///< 승차요금(number)
				::CopyMemory(&queData.szData[nOffset], iter->ride_fee, sizeof(int));
				nOffset += sizeof(int);

				///< 할인이후금액(number)
				::CopyMemory(&queData.szData[nOffset], iter->dc_aft_amt, sizeof(int));
				nOffset += sizeof(int);
			}
			// Length
			queData.nLen = nOffset;
		}
		else
		{
			// Result
			queData.szData[nOffset++] = CHAR_NAK;
			nOffset += sprintf(&queData.szData[nOffset], "%s", CConfigTkMem::GetInstance()->GetErrorCode());

			// Length
			queData.nLen = nOffset;
		}

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_TkPubAlcnFareInfo
 * @details		현장발권 - 배차요금 조회 (0x306)
 * @param		None
 * @return		항상 : 0
 */
int UI_TkPubSatsInfo(BOOL bResult, int nError)
{
	int				nOffset, nCount, i;
	UI_QUE_DATA_T	queData;

	nOffset = nCount = i = 0;

	Locking();
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_PBTCK_SEAT_INFO;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		if(bResult == TRUE)
		{
			CPubTckMem* pTr;
			//pmyrtk_readalcnfee_t pInfo;

			//pInfo = (pmyrtk_readalcnfee_t) Transact_GetData(CFG_RSP_ALCN_FEE_DATA);
			pTr = CPubTckMem::GetInstance();

			// Result
			queData.szData[nOffset++] = CHAR_ACK;

			// 좌석 정보수
			nCount = pTr->m_vtPcpysats.size();
			::CopyMemory(&queData.szData[nOffset], &nCount, sizeof(WORD));
			nOffset += sizeof(WORD);

			vector<rtk_pcpysats_list_t>::iterator	iter;

			for( iter = pTr->m_vtPcpysats.begin(); iter != pTr->m_vtPcpysats.end(); iter++ )
			{
				///< 좌석선점ID
				::CopyMemory(&queData.szData[nOffset], iter->sats_pcpy_id, sizeof(iter->sats_pcpy_id) - 1);
				nOffset += (sizeof(iter->sats_pcpy_id) - 1);

				///< 시외버스할인종류코드
				::CopyMemory(&queData.szData[nOffset], iter->cty_bus_dc_knd_cd, sizeof(iter->cty_bus_dc_knd_cd) - 1);
				nOffset += (sizeof(iter->cty_bus_dc_knd_cd) - 1);

				///< 할인율 구분코드
				::CopyMemory(&queData.szData[nOffset], iter->dcrt_dvs_cd, sizeof(iter->dcrt_dvs_cd) - 1);
				nOffset += (sizeof(iter->dcrt_dvs_cd) - 1);

				///< 버스티켓종류코드
				::CopyMemory(&queData.szData[nOffset], iter->bus_tck_knd_cd, sizeof(iter->bus_tck_knd_cd) - 1);
				nOffset += (sizeof(iter->bus_tck_knd_cd) - 1);

				///< 할인이후금액(number)
				::CopyMemory(&queData.szData[nOffset], iter->dc_aft_amt, sizeof(int));
				nOffset += sizeof(int);

				///< 좌석번호(number)
				::CopyMemory(&queData.szData[nOffset], iter->sats_no, sizeof(WORD));
				nOffset += sizeof(WORD);
			}
			// Length
			queData.nLen = nOffset;
		}
		else
		{
			// Result
			queData.szData[nOffset++] = CHAR_NAK;
			// Error Code
			nOffset += sprintf(&queData.szData[nOffset], "%s", CConfigTkMem::GetInstance()->GetErrorCode());
			// Length
			queData.nLen = nOffset;
		}

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_TkPubTmaxResultInfo
 * @details		현장발권 - 티맥스 승차권 발권결과 (0x318)
 * @param		None
 * @return		항상 : 0
 */
int UI_TkPubTmaxResultInfo(BOOL bResult, int nError)
{
	int				nOffset, nCount, i;
	UI_QUE_DATA_T	queData;

	nOffset = nCount = i = 0;

	::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

	queData.wCommand = UI_CMD_PBTCK_TMAX_RESULT;
	queData.nFlag = FLAG_SEND_RECV;

	Locking();
	{
		// Data
		if(bResult == TRUE)
		{
			CPubTckMem* pTr;
			//pmyrtk_readalcnfee_t pInfo;

			//pInfo = (pmyrtk_readalcnfee_t) Transact_GetData(CFG_RSP_ALCN_FEE_DATA);
			pTr = CPubTckMem::GetInstance();

			// Result
			queData.szData[nOffset++] = CHAR_ACK;
			queData.szData[nOffset++] = CPubTckMem::GetInstance()->base.ui_pym_dvs_cd[0];

			// Length
			queData.nLen = nOffset;
		}
		else
		{
			// Result
			queData.szData[nOffset++] = CHAR_NAK;
			nOffset += sprintf(&queData.szData[nOffset], "%s", CConfigTkMem::GetInstance()->GetErrorCode());
			// Length
			queData.nLen = nOffset;
		}

		s_QueData.push(queData);
	}
	UnLocking();

	return 0;
}

int UI_AddRefundInfo(BOOL bResult)
{
	int nOffset = 0;
	UI_QUE_DATA_T	queData;
	UI_SND_CANCRY_TCK_INFO_T data;

	::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

	queData.wCommand = UI_CMD_CANCRY_INFO;
	queData.nFlag = FLAG_SEND_RECV;

	Locking();
	{
		if(bResult == TRUE)
		{
			CCancRyTkMem* pCancRy;

			pCancRy = CCancRyTkMem::GetInstance();

			data.byAck = CHAR_ACK;

			/// 결제수단
			if( pCancRy->tRespTckNo.pyn_mns_dvs_cd[0] == PYM_CD_CARD)
			{
				data.pyn_mns_dvs_cd[0] = 0x02; // 결제수단:카드
			}
			else
			{
				data.pyn_mns_dvs_cd[0] = 0x01; // 결제수단:현금
			}

			/// 결제금액
			data.n_pym_amt = *(int *)pCancRy->tRespTckNo.tisu_amt; 
			/// 취소수수료
			data.n_canc_ry_cmrt = pCancRy->n_commission_fare; 
			/// 환불금액
			data.n_canc_ry_amt = data.n_pym_amt - data.n_canc_ry_cmrt; 

			::CopyMemory(queData.szData, &data, sizeof(UI_SND_CANCRY_TCK_INFO_T));

			queData.nLen = sizeof(UI_SND_CANCRY_TCK_INFO_T);

			s_QueData.push(queData);
		}
		else
		{
			nOffset = 0;

			queData.szData[nOffset++] = CHAR_NAK;
			nOffset += sprintf(&queData.szData[nOffset], "%s", CConfigTkMem::GetInstance()->GetErrorCode());
			queData.nLen = nOffset;
			
			s_QueData.push(queData);
		}
	}
	UnLocking();

	return 0;
}

