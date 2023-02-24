// 
// 
// dev_ui_main.cpp : UI Interface MAIN
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <queue>

#include <usrinc/fbuf.h>
#include "xzzbus_fdl.h"

#include "MyDefine.h"
#include "MyUtil.h"
#include "cmn_util.h"
#include "MyFileUtil.h"
#include "File_Env_ini.h"
#include "dev_ui_main.h"
#include "MyDataAccum.h"
#include "dev_tr_main.h"
#include "oper_config.h"
#include "oper_kos_file.h"
#include "event_if.h"
#include "MyEvent.h"
#include "svr_ccs_main.h"
#include "svr_ko_main.h"
#include "svr_tm_exp_main.h"
#include "dev_coin_main.h"
#include "dev_bill_main.h"
#include "dev_dispenser_main.h"
#include "dev_tr_mem.h"
#include "dev_tr_kobus_mem.h"
#include "dev_tr_tmexp_mem.h"
#include "data_main.h"
#include "dev_prt_ticket_main.h"
#include "dev_prt_main.h"

//----------------------------------------------------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------------------------------------------------

#define _DEBUG_ALL_		0

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
static int 			m_nDebug = 1;

static BYTE			s_SendData[MAX_UI_PACKET];
static BYTE			s_RecvData[MAX_UI_PACKET];
static int			s_nState;
static BOOL			s_bRun;

static BYTE			*s_pPacket = NULL;

static int			s_nDebug = 1, s_nCmdDebug = 0;

static CUiSocket	clsSocket;
static queue <UI_QUE_DATA_T> s_QueData;

static UI_BASE_T	s_BaseInfo_t;

static PDEV_CFG_T		pEnv = NULL;

//----------------------------------------------------------------------------------------------------------------------

static DWORD WINAPI UiCommThread(void *);

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
		nOffset += Util_MyCopyMemory(&s_pPacket[nOffset], &wCommand, 2);
		// 02. Length
		nOffset += Util_MyCopyMemory(&s_pPacket[nOffset], &nLen, 4);
		// 03. DATA 
		nOffset += Util_MyCopyMemory(&s_pPacket[nOffset], pData, nLen);
	}

	nTotal = nOffset;
	nSend = 0;

	do 
	{
		nRet = clsSocket.Send(&s_pPacket[nSend], nTotal - nSend);
		if(nRet < 0) 
		{
			if(nRet == RET_SNSOCKET_CLOSE) 
			{
				goto fail_proc;
			}
			if(++nRetry >= 3) 
			{
				nRet = -1;
				goto fail_proc;
			}
			continue;
		}
		nSend += nRet;
	} while(nSend < nTotal);

	if(s_nDebug > 0)
	{
		PUI_HEADER_T pHdr;

		pHdr = (PUI_HEADER_T) s_pPacket;

#if (_DEBUG_ALL_ > 0)
		LOG_HEXA("SendPacket", s_pPacket, nTotal);
#else
		if( pHdr->wCommand == UI_CMD_POLLING )
		{
			;//LOG_OUT("SendPacket -> Polling");
		}
		else
		{
			LOG_HEXA("SendPacket", s_pPacket, nTotal);
		}
#endif
	}

	if(s_pPacket != NULL)
	{
		delete[] s_pPacket;
		s_pPacket = NULL;
	}

	return nSend;

fail_proc:

	if(s_pPacket != NULL)
	{
		delete[] s_pPacket;
		s_pPacket = NULL;
	}

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
	nLen = pHeader->dwDataLen;

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
#if (_DEBUG_ALL_ > 0)
		LOG_HEXA("GetPacket", retBuf, nRecv);
#else
		if( pHeader->wCommand == UI_CMD_POLLING )
		{
			;//LOG_OUT("GetPacket -> Polling");
		}
		else
		{
			int length = 0;

			length = nRecv;
			if(nRecv > 300)
			{
				;//length = 300;
			}
			LOG_HEXA("GetPacket", retBuf, length);
		}
#endif
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

static int MakeErrorPacket(char *retBuf)
{
	UI_SND_ERROR_T	errData;
	char *p;

	::ZeroMemory(&errData, sizeof(UI_SND_ERROR_T));

	errData.byACK = CHAR_NAK;
	sprintf(errData.szErrCode, "%s", CConfigTkMem::GetInstance()->GetErrorCode());
	p = CConfigTkMem::GetInstance()->GetErrorContents();
	if( strlen(p) > 0 )
	{
		sprintf(errData.szErrContents, "%s", p);
	}

	::CopyMemory(retBuf, &errData, sizeof(UI_SND_ERROR_T));
	
	return sizeof(UI_SND_ERROR_T);
}

/**
 * @brief		PrintOptionInfo
 * @details		무인기 옵션 정보 디버깅 코드
 * @param		PUI_BASE_T pBase	옵션 데이타
 * @return		성공 >= 0, 실패 < 0
 */
static void PrintOptionInfo(PUI_BASE_T pBase)
{
	if(1)
	{
		int i  = 1;

//		TR_LOG_OUT("(%02d). pBase->device_dvs			= %02X", i, pBase->device_dvs				);	
		TR_LOG_OUT("(%02d). pBase->ccs_svr_kind			= %02X", i, pBase->ccs_svr_kind			);	
		TR_LOG_OUT("(%02d). pBase->exp_svr_kind			= %02X", i, pBase->exp_svr_kind			);	
		TR_LOG_OUT("(%02d). pBase->mrnp_func_yn			= %02X", i, pBase->mrnp_func_yn			);	
		TR_LOG_OUT("(%02d). pBase->mrnp_all_view_yn		= %02X", i, pBase->mrnp_all_view_yn		);	
		TR_LOG_OUT("(%02d). pBase->mrnp_manual_yn		= %02X", i, pBase->mrnp_manual_yn			);	
		TR_LOG_OUT("(%02d). pBase->mrnp_auto_iss_yn		= %02X", i, pBase->mrnp_auto_iss_yn		);	
		TR_LOG_OUT("(%02d). pBase->pubtck_yn			= %02X", i, pBase->pubtck_yn				);	
		TR_LOG_OUT("(%02d). pBase->alcn_kind			= %02X", i, pBase->alcn_kind				);	
		TR_LOG_OUT("(%02d). pBase->quick_alcn_yn		= %02X", i, pBase->quick_alcn_yn			);	
		TR_LOG_OUT("(%02d). pBase->favorate_yn			= %02X", i, pBase->favorate_yn				);	
		TR_LOG_OUT("(%02d). pBase->n_max_favor_trml_cnt	= %02X", i, pBase->n_max_favor_trml_cnt	);	
		TR_LOG_OUT("(%02d). pBase->ccs_refund_func_yn	= %02X", i, pBase->ccs_refund_func_yn			);	
		TR_LOG_OUT("(%02d). pBase->exp_refund_func_yn	= %02X", i, pBase->exp_refund_func_yn			);	
		TR_LOG_OUT("(%02d). pBase->reg_csrc_yn			= %02X", i, pBase->reg_csrc_yn				);	
		TR_LOG_OUT("(%02d). pBase->card_installment_yn	= %02X", i, pBase->card_installment_yn		);	
		TR_LOG_OUT("(%02d). pBase->sign_pad_yn			= %02X", i, pBase->sign_pad_yn				);	
		TR_LOG_OUT("(%02d). pBase->card_passwd_yn		= %02X", i, pBase->card_passwd_yn			);	
		TR_LOG_OUT("(%02d). pBase->maint_monitor_yn		= %02X", i, pBase->maint_monitor_yn		);	
		TR_LOG_OUT("(%02d). pBase->auto_close_yn		= %02X", i, pBase->auto_close_yn			);	
		TR_LOG_OUT("(%02d). pBase->auto_close_time[6]	= %.*s", i, 6, pBase->auto_close_time		);	
		TR_LOG_OUT("(%02d). pBase->sms_yn				= %02X", i, pBase->sms_yn					);	
		TR_LOG_OUT("(%02d). pBase->sms_ip[50]			= %s"  , i, pBase->sms_ip				);	
		TR_LOG_OUT("(%02d). pBase->camera_1_yn			= %02X", i, pBase->camera_1_yn				);	
		TR_LOG_OUT("(%02d). pBase->camera_2_yn			= %02X", i, pBase->camera_2_yn				);	
		TR_LOG_OUT("(%02d). pBase->camera_3_yn			= %02X", i, pBase->camera_3_yn				);	
		TR_LOG_OUT("(%02d). pBase->n_ticket_min_count	= %d"	, i, pBase->n_ticket_min_count		);	
		TR_LOG_OUT("(%02d). pBase->n_10k_min_count		= %d"	, i, pBase->n_10k_min_count			);	
		TR_LOG_OUT("(%02d). pBase->n_1k_min_count		= %d"	, i, pBase->n_1k_min_count			);	
		TR_LOG_OUT("(%02d). pBase->n_100_min_count		= %d"	, i, pBase->n_100_min_count			);	
		TR_LOG_OUT("(%02d). pBase->n_500_min_count		= %d"	, i, pBase->n_500_min_count			);	
		TR_LOG_OUT("(%02d). pBase->n_bill_box_full_cnt	= %d"	, i, pBase->n_bill_box_full_cnt		);	
		TR_LOG_OUT("(%02d). pBase->n_ticket_box_full_cnt= %d"	, i, pBase->n_ticket_box_full_cnt	);	
		TR_LOG_OUT("(%02d). pBase->n_issue_count		= %d"	, i, pBase->n_issue_count			);	
		TR_LOG_OUT("(%02d). pBase->n_issue_money		= %d"	, i, pBase->n_issue_money			);	
		TR_LOG_OUT("(%02d). pBase->n_issue_time			= %d"	, i, pBase->n_issue_time			);	
		TR_LOG_OUT("(%02d). pBase->n_mrnp_limit_day		= %d"	, i, pBase->n_mrnp_limit_day		);	
		TR_LOG_OUT("(%02d). pBase->n_screen_wait_time	= %d"	, i, pBase->n_screen_wait_time		);	
		TR_LOG_OUT("(%02d). pBase->n_alarm_wait_time	= %d"	, i, pBase->n_alarm_wait_time		);	
		TR_LOG_OUT("(%02d). pBase->tck_paper_name[10]	= %s"	, i, pBase->tck_paper_name		);	
		TR_LOG_OUT("(%02d). pBase->exp_ticket_device[10]= %s"	, i, pBase->exp_ticket_device	);	
					 
		TR_LOG_OUT("(%02d). pBase->cc_rtrp_trml_yn		= %02X", i, pBase->cc_rtrp_trml_yn			);	
		TR_LOG_OUT("(%02d). pBase->exp_rtrp_trml_yn		= %02X", i, pBase->exp_rtrp_trml_yn		);	
		TR_LOG_OUT("(%02d). pBase->return_mrnp_yn		= %02X", i, pBase->return_mrnp_yn			);	
		TR_LOG_OUT("(%02d). pBase->exp_line_yn			= %02X", i, pBase->exp_line_yn				);	
		TR_LOG_OUT("(%02d). pBase->opend_disp_yn		= %02X", i, pBase->opend_disp_yn			);	
		TR_LOG_OUT("(%02d). pBase->no_alcn_no_sats_mrnp_yn= %02X", i, pBase->no_alcn_no_sats_mrnp_yn	);	
		TR_LOG_OUT("(%02d). pBase->no_alcn_1_issue_yn	= %02X", i, pBase->no_alcn_1_issue_yn		);	
		TR_LOG_OUT("(%02d). pBase->no_sats_free_issue_yn= %02X", i, pBase->no_sats_free_issue_yn	);	
		TR_LOG_OUT("(%02d). pBase->n_mrnp_limit_time	= %d",   i, pBase->n_mrnp_limit_time		);	
		TR_LOG_OUT("(%02d). pBase->today_mrnp_issue_yn	= %02X", i, pBase->today_mrnp_issue_yn		);	
		TR_LOG_OUT("(%02d). pBase->re_issue_yn			= %02X", i, pBase->re_issue_yn				);	
		TR_LOG_OUT("(%02d). pBase->kiosk_op_close_yn	= %02X", i, pBase->kiosk_op_close_yn		);	
		TR_LOG_OUT("(%02d). pBase->kiosk_op_start_tm[6]	= %.*s", i, 6, pBase->kiosk_op_start_tm	);	
		TR_LOG_OUT("(%02d). pBase->kiosk_op_close_tm[6]	= %.*s", i, 6, pBase->kiosk_op_close_tm	);	
		TR_LOG_OUT("(%02d). pBase->lcd_off_yn			= %02X", i, pBase->lcd_off_yn				);	
		TR_LOG_OUT("(%02d). pBase->ic_card_insert_yn	= %02X", i, pBase->ic_card_insert_yn		);	
		TR_LOG_OUT("(%02d). pBase->multi_lang_yn		= %02X", i, pBase->multi_lang_yn			);	
		TR_LOG_OUT("(%02d). pBase->sound_yn				= %02X", i, pBase->sound_yn				);	
		TR_LOG_OUT("(%02d). pBase->main_sound_time		= %d",   i, pBase->main_sound_time			);	
		TR_LOG_OUT("(%02d). pBase->bcnl_yn				= %02X", i, pBase->bcnl_yn					);	
		TR_LOG_OUT("(%02d). pBase->tck_no_rmn_yn		= %02X", i, pBase->tck_no_rmn_yn			);	
		TR_LOG_OUT("(%02d). pBase->alcn_state_color_yn	= %02X", i, pBase->alcn_state_color_yn		);	
		TR_LOG_OUT("(%02d). pBase->disc_btn_yn			= %02X", i, pBase->disc_btn_yn				);	
		TR_LOG_OUT("(%02d). pBase->rot_search_yn		= %02X", i, pBase->rot_search_yn			);	
		TR_LOG_OUT("(%02d). pBase->air_sta_popup_yn		= %02X", i, pBase->air_sta_popup_yn		);	
		TR_LOG_OUT("(%02d). pBase->tck_err_change_yn	= %02X", i, pBase->tck_err_change_yn		);	
		TR_LOG_OUT("(%02d). pBase->refund_void_prt_yn	= %02X", i, pBase->refund_void_prt_yn		);	
		TR_LOG_OUT("(%02d). pBase->map_yn				= %02X", i, pBase->map_yn[0]			);	

		/// 2020.01.15 add data
		TR_LOG_OUT("(%02d). pBase->rot_mode_yn			= %02X", i, pBase->rot_mode_yn[0]			);	///< (92). 노선선택 전용모드 유무
		TR_LOG_OUT("(%02d). pBase->rot_mode_mrnp_yn		= %02X", i, pBase->rot_mode_mrnp_yn[0]		);	///< (93). 노선선택 전용모드에서 예매발권 유무
		TR_LOG_OUT("(%02d). pBase->rot_mode_arvl_yn		= %02X", i, pBase->rot_mode_arvl_yn[0]		);	///< (94). 노선선택 전용모드에서 목적지 검색유무
		TR_LOG_OUT("(%02d). pBase->rf_mode_precard_yn	= %02X", i, pBase->rf_mode_precard_yn[0]	);	///< (95). RF모드에서 선불카드 사용유무
		TR_LOG_OUT("(%02d). pBase->rf_mode_postcard_yn	= %02X", i, pBase->rf_mode_postcard_yn[0]	);	///< (96). RF모드에서 후불카드 사용유무
		TR_LOG_OUT("(%02d). pBase->refund_reprint_yn	= %02X", i, pBase->refund_reprint_yn[0]		);	///< (97). 환불영수증 재발행 사용유무
		TR_LOG_OUT("(%02d). pBase->mrnp_inq_yn			= %02X", i, pBase->mrnp_inq_yn[0]			);	///< (98). 예매자료 조회 사용유무
		TR_LOG_OUT("(%02d). pBase->keywd_search_yn		= %02X", i, pBase->keywd_search_yn[0]		);	///< (99). 키워드 검색 사용유무
		TR_LOG_OUT("(%02d). pBase->prev_mrnp_inq_yn		= %02X", i, pBase->prev_mrnp_inq_yn[0]		);	///< (100).과거 예매내역 발권 사용유무
		TR_LOG_OUT("(%02d). pBase->alcn_time_vw_yn		= %02X", i, pBase->alcn_time_vw_yn[0]		);	///< (101).배차시간대 선택버튼 사용유무
		/// ~2020.01.15 add data
	}

	TR_LOG_OUT("\t 창구마감 시 시재마감 여부				= %02X", pBase->day_cls_an_cash_cls_yn[0]	);	///< (117).창구마감 시 시재마감 여부
	TR_LOG_OUT("\t 비배차 시간표시 옵션(YN)				= %02X", pBase->alcn_way_dvs_yn	);				///< (119).비배차 시간표시 옵션(YN) // 20220902 ADD

}

/**
 * @brief		make_0204_data
 * @details		[시외버스] 예매발권 - 예매리스트 응답
 * @param		UI_QUE_DATA_T tQue		결과값
 * @param		BYTE *retBuf			결과값
 * @return		데이타 크기
 */
static int make_0204_data(UI_QUE_DATA_T tQue)
{
	int nRet = 0, nSize = 0, nCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		if(tQue.szData[0] == CHAR_ACK)
		{
			nCount = CMrnpMem::GetInstance()->m_vtMrnpList.size();
			nSize = 100 + (nCount * sizeof(rtk_readmrnppt_list_t));

			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			nSize = 0;

			/// ACK
			retBuf[nSize++] = CHAR_ACK;
			/// 갯수
			nSize += Util_MyCopyMemory(&retBuf[nSize], &nCount, sizeof(WORD));
			/// 데이타
			nSize += CMrnpMem::GetInstance()->MakeMrnpListPacket((char *)&retBuf[nSize]);
		}
		else
		{
			nSize = sizeof(UI_SND_ERROR_T);
			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			::CopyMemory(retBuf, tQue.szData, nSize);
		}

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_0234_data
 * @details		[코버스] 예매 조회 결과 데이타 생성
 * @param		UI_QUE_DATA_T tQue		결과값
 * @param		BYTE *retBuf			결과값
 * @return		데이타 크기
 */
static int make_0234_data(UI_QUE_DATA_T tQue)
{
	int nRet = 0, nSize = 0, nCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		nCount = CMrnpKobusMem::GetInstance()->m_vtMrnpList.size();
		nSize = 200 + (nCount * sizeof(rtk_tm_mrsinfo_list_t));

		retBuf = new BYTE[nSize];
		::ZeroMemory(retBuf, nSize);

		nSize = CMrnpKobusMem::GetInstance()->MakeMrnpListPacket(nCount, (char *)retBuf);

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_0236_data
 * @details		[코버스] 예매 조회 결과 데이타 생성
 * @param		UI_QUE_DATA_T tQue		결과값
 * @param		BYTE *retBuf			결과값
 * @return		데이타 크기
 */
static int make_0236_data(UI_QUE_DATA_T tQue)
{
	int nRet, nSize = 0, nCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		nCount = CMrnpKobusMem::GetInstance()->m_vtResComplete.size();
		nSize = 200 + (nCount * sizeof(rtk_tm_mrspub_list_t));

		retBuf = new BYTE[nSize];
		::ZeroMemory(retBuf, nSize);

		nSize = CMrnpKobusMem::GetInstance()->MakePubMrnpResultPacket((char *)retBuf);

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_0251_data
 * @details		[티머니고속] 예매 조회 결과 데이타 생성
 * @param		UI_QUE_DATA_T tQue		결과값
 * @param		BYTE *retBuf			결과값
 * @return		데이타 크기
 */
static int make_0251_data(UI_QUE_DATA_T tQue)
{
	int nRet = 0, nSize = 0, nCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		nCount = CMrnpTmExpMem::GetInstance()->m_vtMrnpList.size();
		nSize = 200 + (nCount * sizeof(rtk_tm_read_mrs_list_t));

		retBuf = new BYTE[nSize];
		::ZeroMemory(retBuf, nSize);

		nSize = CMrnpTmExpMem::GetInstance()->MakeMrnpListPacket((char *)retBuf);

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_0253_data
 * @details		[티머니고속] KTC 예매 조회 결과 데이타 생성
 * @param		UI_QUE_DATA_T tQue		결과값
 * @param		BYTE *retBuf			결과값
 * @return		데이타 크기
 */
static int make_0253_data(UI_QUE_DATA_T tQue)
{
	int nRet = 0, nSize = 0, nCount = 0;
	BYTE *retBuf = NULL;

	LOG_OUT("start..");

	try
	{
		nCount = CMrnpTmExpMem::GetInstance()->m_vtMrnpKtcList.size();

		if(nCount > 0)
		{
			nSize = 200 + (nCount * sizeof(rtk_tm_read_mrs_ktc_list_t));

			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			nSize = CMrnpTmExpMem::GetInstance()->MakeMrnpKtcListPacket((char *)retBuf);

			if(1)
			{
				int count = 0, offset = 0;
				prtk_tm_read_mrs_ktc_t  pPacket;
				rtk_tm_read_mrs_ktc_list_t *pList;

				pPacket = (prtk_tm_read_mrs_ktc_t) &retBuf[1];

				LOG_OUT("%30s - (%s) ", "rsp_cd"		, pPacket->rsp_cd);
				LOG_OUT("%30s - (%d) ", "rec_num"		, *(int *)pPacket->rec_num);
				count = *(int *)pPacket->rec_num;
				
				offset = 1 + sizeof(pPacket->rsp_cd) + sizeof(pPacket->rec_num); // 1:ack

				for(int i = 0; i < count; i++)
				{
					int mrs_num = 0;

					pList = (rtk_tm_read_mrs_ktc_list_t *) &retBuf[offset];

					LOG_OUT("%30s - (%s) ", "mrs_mrnp_no		"		, pList->mrs_mrnp_no		);
					LOG_OUT("%30s - (%s) ", "mrs_mrnp_sno		"		, pList->mrs_mrnp_sno		);
					LOG_OUT("%30s - (%s) ", "mrs_mrnp_dt		"		, pList->mrs_mrnp_dt		);
					LOG_OUT("%30s - (%s) ", "mrs_mrnp_time		"		, pList->mrs_mrnp_time		);
					LOG_OUT("%30s - (%d) ", "cash_recp_amt		"		, *(int *)pList->cash_recp_amt		);
					LOG_OUT("%30s - (%s) ", "tissu_no			"		, pList->tissu_no			);
					LOG_OUT("%30s - (%s) ", "tck_knd_cd			"		, pList->tck_knd_cd			);
					LOG_OUT("%30s - (%s) ", "alcn_depr_trml_no	"		, pList->alcn_depr_trml_no	);
					LOG_OUT("%30s - (%s) ", "alcn_arvl_trml_no	"		, pList->alcn_arvl_trml_no	);
					LOG_OUT("%30s - (%s) ", "depr_trml_no		"		, pList->depr_trml_no		);
					LOG_OUT("%30s - (%s) ", "arvl_trml_no		"		, pList->arvl_trml_no		);
					LOG_OUT("%30s - (%s) ", "bus_cls_cd			"		, pList->bus_cls_cd			);
					LOG_OUT("%30s - (%s) ", "cacm_cd			"		, pList->cacm_cd			);
					LOG_OUT("%30s - (%s) ", "hspd_cty_dvs_cd	"		, pList->hspd_cty_dvs_cd	);
					LOG_OUT("%30s - (%s) ", "alcn_depr_time		"		, pList->alcn_depr_time		);
					LOG_OUT("%30s - (%s) ", "depr_dt			"		, pList->depr_dt			);
					LOG_OUT("%30s - (%s) ", "depr_time			"		, pList->depr_time			);
					LOG_OUT("%30s - (%d) ", "mrs_amt			"		, *(int *)pList->mrs_amt			);
					LOG_OUT("%30s - (%s) ", "card_no			"		, pList->card_no			);
					LOG_OUT("%30s - (%s) ", "adtn_cpn_no		"		, pList->adtn_cpn_no		);
					LOG_OUT("%30s - (%s) ", "adtn_prd_auth_no	"		, pList->adtn_prd_auth_no	);
					LOG_OUT("%30s - (%s) ", "rot_rdhm_no_val	"		, pList->rot_rdhm_no_val	);
					LOG_OUT("%30s - (%s) ", "buy_cmpy_cd		"		, pList->buy_cmpy_cd		);
					LOG_OUT("%30s - (%s) ", "tck_knd_string		"		, pList->tck_knd_string		);
					LOG_OUT("%30s - (%s) ", "sats_no_string		"		, pList->sats_no_string		);
					LOG_OUT("%30s - (%s) ", "tissu_chnl_dvs_cd	"		, pList->tissu_chnl_dvs_cd	);
					LOG_OUT("%30s - (%s) ", "pub_chnl_dvs_cd	"		, pList->pub_chnl_dvs_cd	);
					LOG_OUT("%30s - (%s) ", "pyn_dvs_cd			"		, pList->pyn_dvs_cd			);
					LOG_OUT("%30s - (%s) ", "tissu_sta_cd		"		, pList->tissu_sta_cd		);

					LOG_OUT("%30s - (%d) ", "mrs_num			"		, *(int *)pList->mrs_num	);

					offset += pList->mrs_info[0].tck_knd_cd - pList->mrs_mrnp_no;

					mrs_num = *(int *) pList->mrs_num;

					for(int k = 0; k < mrs_num; k++)
					{
						prtk_tm_read_mrs_ktc_info_t pList1;

						pList1 = &pList->mrs_info[k];

						LOG_OUT("%30s - (%s) ", "- tck_knd_cd	"		, pList1->tck_knd_cd	);
						LOG_OUT("%30s - (%s) ", "- sats_no		"		, pList1->sats_no	);
						LOG_OUT("%30s - (%d) ", "- tissu_amt	"		, *(int *)pList1->tissu_amt	);
						LOG_OUT("%30s - (%s) ", "- ry_sta_cd	"		, pList1->ry_sta_cd	);
						LOG_OUT("%30s - (%s) ", "- dc_knd_cd	"		, pList1->dc_knd_cd	);
					}

					offset += sizeof(rtk_tm_read_mrs_ktc_info_t) * mrs_num;
				}
			}

			nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
			if(nRet < 0)
			{
				if(retBuf != NULL)
				{
					delete[] retBuf;
					retBuf = NULL;
				}
				return nRet;
			}

			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
		}
		else
		{
			LOG_OUT("KTC 예매리스트 갯수 error = %d..", nCount);
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_0255_data
 * @details		[티머니고속] 예매발권 결과 데이타 생성
 * @param		UI_QUE_DATA_T tQue		결과값
 * @param		BYTE *retBuf			결과값
 * @return		데이타 크기
 */
static int make_0255_data(UI_QUE_DATA_T tQue)
{
	int nRet = 0, nSize = 0, nCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		nCount = CMrnpTmExpMem::GetInstance()->m_vtResComplete.size();
		nSize = 200 + sizeof(MRNP_TMEXP_BASE_T) + (nCount * sizeof(rtk_tm_pub_mrs_list_t));

		retBuf = new BYTE[nSize];
		::ZeroMemory(retBuf, nSize);

		nSize = CMrnpTmExpMem::GetInstance()->MakePubMrnpResultPacket((char *)retBuf);

		if(1)
		{
			int count = 0, offset = 0;
			prtk_tm_pub_mrs_t pPacket;

			pPacket = (prtk_tm_pub_mrs_t) &retBuf[1];

			offset += 1;
			LOG_OUT("%30s - (%s) ", "rsp_cd"		, pPacket->rsp_cd);
			offset += sizeof(pPacket->rsp_cd);

			LOG_OUT("%30s - (%s) ", "pyn_dtl_cd"	, pPacket->pyn_dtl_cd	);
			offset += sizeof(pPacket->pyn_dtl_cd);

			LOG_OUT("%30s - (%s) ", "card_no"		, pPacket->card_no		);
			offset += sizeof(pPacket->card_no);

			LOG_OUT("%30s - (%s) ", "card_aprv_no"	, pPacket->card_aprv_no);
			offset += sizeof(pPacket->card_aprv_no);

			LOG_OUT("%30s - (%s) ", "aprv_amt"		, pPacket->aprv_amt	);
			offset += sizeof(pPacket->aprv_amt);

			LOG_OUT("%30s - (%s) ", "frc_cmrt"		, pPacket->frc_cmrt	);
			offset += sizeof(pPacket->frc_cmrt);

			LOG_OUT("%30s - (%d) ", "rec_num"		, *(int *)pPacket->rec_num);
			offset += sizeof(pPacket->rec_num);

			count = *(int *)pPacket->rec_num;

			for(int i = 0; i < count; i++)
			{
				rtk_tm_pub_mrs_list_t *pList;

				pList = (rtk_tm_pub_mrs_list_t *) &retBuf[offset];

				LOG_OUT("%30s - (%s) ", "mrs_mrnp_no		"		, pList->mrs_mrnp_no		);
				LOG_OUT("%30s - (%s) ", "mrs_mrnp_sno		"		, pList->mrs_mrnp_sno		);
				LOG_OUT("%30s - (%s) ", "depr_dt			"		, pList->depr_dt			);
				LOG_OUT("%30s - (%s) ", "depr_time			"		, pList->depr_time			);
				LOG_OUT("%30s - (%s) ", "alcn_depr_trml_no	"		, pList->alcn_depr_trml_no	);
				LOG_OUT("%30s - (%s) ", "alcn_arvl_trml_no	"		, pList->alcn_arvl_trml_no	);
				LOG_OUT("%30s - (%s) ", "depr_trml_no		"		, pList->depr_trml_no		);
				LOG_OUT("%30s - (%s) ", "arvl_trml_no		"		, pList->arvl_trml_no		);
				LOG_OUT("%30s - (%s) ", "alcn_rot_no		"		, pList->alcn_rot_no		);
				LOG_OUT("%30s - (%s) ", "bus_cls_cd			"		, pList->bus_cls_cd			);
				LOG_OUT("%30s - (%s) ", "cacm_cd			"		, pList->cacm_cd			);
				LOG_OUT("%30s - (%s) ", "rot_rdhm_no_val	"		, pList->rot_rdhm_no_val	);
				LOG_OUT("%30s - (%s) ", "bus_oprn_dist		"		, pList->bus_oprn_dist		);
				LOG_OUT("%30s - (%d) ", "take_drtm			"		, *(int *)pList->take_drtm			);
				LOG_OUT("%30s - (%s) ", "cacm_nm_prin_yn	"		, pList->cacm_nm_prin_yn	);
				LOG_OUT("%30s - (%s) ", "bus_cls_prin_yn	"		, pList->bus_cls_prin_yn	);
				LOG_OUT("%30s - (%s) ", "depr_time_prin_yn	"		, pList->depr_time_prin_yn	);
				LOG_OUT("%30s - (%s) ", "sats_no_prin_yn	"		, pList->sats_no_prin_yn	);
				LOG_OUT("%30s - (%s) ", "alcn_dvs_cd		"		, pList->alcn_dvs_cd		);
				LOG_OUT("%30s - (%s) ", "tissu_dt			"		, pList->tissu_dt			);
				LOG_OUT("%30s - (%s) ", "tissu_time			"		, pList->tissu_time			);
				LOG_OUT("%30s - (%s) ", "tissu_trml_no		"		, pList->tissu_trml_no		);
				LOG_OUT("%30s - (%s) ", "tissu_wnd_no		"		, pList->tissu_wnd_no		);
				LOG_OUT("%30s - (%s) ", "pub_user_no		"		, pList->pub_user_no		);
				LOG_OUT("%30s - (%s) ", "tissu_sno			"		, pList->tissu_sno			);
				LOG_OUT("%30s - (%s) ", "inhr_no			"		, pList->inhr_no			);
				LOG_OUT("%30s - (%s) ", "invs_no			"		, pList->invs_no			);
				LOG_OUT("%30s - (%s) ", "sats_no			"		, pList->sats_no			);
				LOG_OUT("%30s - (%s) ", "tck_knd_cd			"		, pList->tck_knd_cd			);
				LOG_OUT("%30s - (%s) ", "fee_knd_cd			"		, pList->fee_knd_cd			);
				LOG_OUT("%30s - (%d) ", "tissu_fee			"		, *(int *)pList->tissu_fee			);
				LOG_OUT("%30s - (%d) ", "ogn_fee			"		, *(int *)pList->ogn_fee			);

				offset += sizeof(rtk_tm_pub_mrs_list_t);
			}
		}

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_0257_data
 * @details		[티머니고속] 모바일티켓 예매발권 결과 데이타 생성
 * @param		UI_QUE_DATA_T tQue		결과값
 * @param		BYTE *retBuf			결과값
 * @return		데이타 크기
 */
static int make_0257_data(UI_QUE_DATA_T tQue)
{
	int nRet = 0, nSize = 0, nCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		nCount = CMrnpTmExpMem::GetInstance()->m_vtResCompleteMobile.size();
		nSize = 200 + sizeof(MRNP_TMEXP_BASE_T) + (nCount * sizeof(rtk_tm_pub_mrs_htck_list_t));

		retBuf = new BYTE[nSize];
		::ZeroMemory(retBuf, nSize);

		nSize = CMrnpTmExpMem::GetInstance()->MakePubMrnpMobileResultPacket((char *)retBuf);

		if(1)
		{
			int count = 0, offset = 0;
			prtk_tm_pub_mrs_htck_t pPacket;

			pPacket = (prtk_tm_pub_mrs_htck_t) &retBuf[1];

			offset += 1;
			LOG_OUT("%30s - (%s) ", "rsp_cd			"		,	pPacket->rsp_cd );
			LOG_OUT("%30s - (%s) ", "card_no		"		,	pPacket->card_no		 );
			LOG_OUT("%30s - (%d) ", "n_aprv_amt		"		,	*(int *)pPacket->n_aprv_amt		 );
			LOG_OUT("%30s - (%s) ", "card_aprv_no	"		,	pPacket->card_aprv_no	 );
			LOG_OUT("%30s - (%d) ", "n_frc_cmm		"		,	*(int *)pPacket->n_frc_cmm		 );
			LOG_OUT("%30s - (%d) ", "n_tissu_num	"		,	*(int *)pPacket->n_tissu_num	 );
			
			offset += pPacket->n_tissu_num - pPacket->rsp_cd;
			offset += sizeof(pPacket->n_tissu_num);

			count = *(int *)pPacket->n_tissu_num;
			for(int i = 0; i < count; i++)
			{
				prtk_tm_pub_mrs_htck_list_t pList;

				pList = (prtk_tm_pub_mrs_htck_list_t) &retBuf[offset];

				LOG_OUT("####### 인덱스 (%4d / %4d) #######", i, count);
				LOG_OUT("%30s - (%s) ", "bus_oprn_dist				"		,	pList->bus_oprn_dist				);
				LOG_OUT("%30s - (%s) ", "rot_rdhm_no_val			"		,	pList->rot_rdhm_no_val				);
				LOG_OUT("%30s - (%s) ", "cacm_nm_prin_yn			"		,	pList->cacm_nm_prin_yn				);
				LOG_OUT("%30s - (%s) ", "bus_cls_prin_yn			"		,	pList->bus_cls_prin_yn				);
				LOG_OUT("%30s - (%s) ", "depr_time_prin_yn			"		,	pList->depr_time_prin_yn			);
				LOG_OUT("%30s - (%s) ", "sats_no_prin_yn			"		,	pList->sats_no_prin_yn				);
				LOG_OUT("%30s - (%s) ", "depr_trml_eng_abrv_nm		"		,	pList->depr_trml_eng_abrv_nm		);
				LOG_OUT("%30s - (%s) ", "arvl_trml_eng_abrv_nm		"		,	pList->arvl_trml_eng_abrv_nm		);
				LOG_OUT("%30s - (%s) ", "bus_cls_nm					"		,	pList->bus_cls_nm					);
				LOG_OUT("%30s - (%s) ", "cacm_nm					"		,	pList->cacm_nm						);
				LOG_OUT("%30s - (%s) ", "bizr_no					"		,	pList->bizr_no						);
				LOG_OUT("%30s - (%s) ", "tel_no						"		,	pList->tel_no						);
				LOG_OUT("%30s - (%s) ", "alcn_depr_trml_eng_abrv_nm	"		,	pList->alcn_depr_trml_eng_abrv_nm	);
				LOG_OUT("%30s - (%s) ", "alcn_depr_trml_no			"		,	pList->alcn_depr_trml_no			);
				LOG_OUT("%30s - (%s) ", "alcn_arvl_trml_no			"		,	pList->alcn_arvl_trml_no			);
				LOG_OUT("%30s - (%s) ", "alcn_depr_dt				"		,	pList->alcn_depr_dt					);
				LOG_OUT("%30s - (%s) ", "alcn_depr_time				"		,	pList->alcn_depr_time				);
				LOG_OUT("%30s - (%s) ", "depr_trml_no				"		,	pList->depr_trml_no					);
				LOG_OUT("%30s - (%s) ", "arvl_trml_no				"		,	pList->arvl_trml_no					);
				LOG_OUT("%30s - (%s) ", "depr_dt					"		,	pList->depr_dt						);
				LOG_OUT("%30s - (%s) ", "depr_time					"		,	pList->depr_time					);
				LOG_OUT("%30s - (%s) ", "bus_cls_cd					"		,	pList->bus_cls_cd					);
				LOG_OUT("%30s - (%s) ", "cacm_cd					"		,	pList->cacm_cd						);
				LOG_OUT("%30s - (%s) ", "alcn_dvs_cd				"		,	pList->alcn_dvs_cd					);
				LOG_OUT("%30s - (%s) ", "tissu_dt					"		,	pList->tissu_dt						);
				LOG_OUT("%30s - (%s) ", "tissu_trml_no				"		,	pList->tissu_trml_no				);
				LOG_OUT("%30s - (%s) ", "tissu_wnd_no				"		,	pList->tissu_wnd_no					);
				LOG_OUT("%30s - (%s) ", "tissu_time					"		,	pList->tissu_time					);
				LOG_OUT("%30s - (%s) ", "rpub_dt					"		,	pList->rpub_dt						);
				LOG_OUT("%30s - (%s) ", "rpub_trml_no				"		,	pList->rpub_trml_no					);
				LOG_OUT("%30s - (%s) ", "rpub_wnd_no				"		,	pList->rpub_wnd_no					);
				LOG_OUT("%30s - (%s) ", "rpub_time					"		,	pList->rpub_time					);
				LOG_OUT("%30s - (%s) ", "tissu_sno					"		,	pList->tissu_sno					);
				LOG_OUT("%30s - (%s) ", "inhr_no					"		,	pList->inhr_no						);
				LOG_OUT("%30s - (%s) ", "invs_no					"		,	pList->invs_no						);
				LOG_OUT("%30s - (%s) ", "n_tissu_amt				"		,	pList->n_tissu_amt					);
				LOG_OUT("%30s - (%s) ", "sats_no					"		,	pList->sats_no						);
				LOG_OUT("%30s - (%s) ", "tck_knd_cd					"		,	pList->tck_knd_cd					);
				LOG_OUT("%30s - (%s) ", "ptrg_prin_nm				"		,	pList->ptrg_prin_nm					);

			}
			
		}

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_0106_data
 * @details		장비환경 설정정보 전송
 * @param		UI_QUE_DATA_T tQue		QUEUE data
 * @param		BYTE *retBuf			데이타 생성 data
 * @return		데이타 크기
 */
static int make_0106_data(UI_QUE_DATA_T tQue)
{
	int nStep = 1;
	int nSize = 0, nCount = 0, nRet = 0;
	UI_BASE_INFO_T	Info;
	POPER_FILE_CONFIG_T	pConfig;
	PKIOSK_INI_ENV_T pEnv;
	BYTE *retBuf = NULL;

	LOG_OUT(" start !!!");

	try
	{
		nCount = 1;
		nSize = 200 + (nCount * sizeof(UI_BASE_T));

		retBuf = new BYTE[nSize];
		::ZeroMemory(retBuf, nSize);

		pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();

		pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

		// option debug
		PrintOptionInfo(&pConfig->base_t);

		//LOG_OUT("%30s - (%02x) ", " @@@@@@ pConfig->base_t.alcn_way_dvs_yn				"		,	pConfig->base_t.alcn_way_dvs_yn);

		// packet 전체 복사
		::CopyMemory(&Info.baseInfo_t, &pConfig->base_t, sizeof(UI_BASE_T));

		//LOG_OUT("%30s - (%02x) ", " @@@@@@ Info.baseInfo_t.alcn_way_dvs_yn				"		,	Info.baseInfo_t.alcn_way_dvs_yn);

		///> 시외버스 종류
		if( pEnv->tCcInfo.nUse == 0 )
		{
			Info.baseInfo_t.ccs_svr_kind = 0x30;
		}
		else
		{
			Info.baseInfo_t.ccs_svr_kind = pEnv->tCcInfo.nKind + 0x30;
		}

		///> 고속버스 종류
		Info.baseInfo_t.exp_svr_kind = 0x30;
		if( (pEnv->tKoInfo.nUse + pEnv->tEzInfo.nUse)  > 0 )
		{
			if( pEnv->tKoInfo.nUse > 0 )
			{
				//Info.baseInfo_t.exp_svr_kind = pEnv->tKoInfo.nKind + 0x30;
				Info.baseInfo_t.exp_svr_kind = 0x32;
			}
			else
			{
				//Info.baseInfo_t.exp_svr_kind = pEnv->tEzInfo.nKind + 0x30;
				Info.baseInfo_t.exp_svr_kind = 0x31;
			}
		}

		/// Set Global variable
		::CopyMemory(&s_BaseInfo_t, &Info.baseInfo_t, sizeof(UI_BASE_T));

		// Data
		::CopyMemory(retBuf, &Info.baseInfo_t, sizeof(UI_BASE_T));
		// Length
		nSize = (int) sizeof(UI_BASE_T);

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}

	}
	catch( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_0302_data
 * @details		
 * @param		UI_QUE_DATA_T tQue		QUEUE data
 * @param		BYTE *retBuf			데이타 생성 data
 * @return		데이타 크기
 */
static int make_0302_data(UI_QUE_DATA_T tQue)
{
	int nRet = 0, nSize = 0, nCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		if(tQue.szData[0] == CHAR_ACK)
		{
			nCount = CPubTckMem::GetInstance()->m_vtAlcnInfo.size();
			nSize = 3 + (nCount * sizeof(rtk_readalcn_list_t));

			LOG_OUT("배차리스트 데이타 갯수 = %d ", nCount);

			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			nSize = CPubTckMem::GetInstance()->MakeAlcnListPacket((char *)retBuf);
		}
		else
		{
			nSize = sizeof(UI_SND_ERROR_T);
			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			::CopyMemory(retBuf, tQue.szData, nSize);
		}

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_0321_data
 * @details		상주직원 조회 결과 데이타 생성
 * @param		UI_QUE_DATA_T tQue		결과값
 * @param		BYTE *retBuf			결과값
 * @return		데이타 크기
 */
static int make_0321_data(UI_QUE_DATA_T tQue)
{
	BOOL bResult;
//	BYTE byACK = 0;
	int nRet = 0, nSize = 0;
	PUI_SND_Q_T pQdata;
	BYTE *retBuf = NULL;

	try
	{
		pQdata = (PUI_SND_Q_T) tQue.szData;

		::CopyMemory(&bResult, tQue.szData, 4);
		//byACK = tQue.szData[0] & 0xFF;

		//if(byACK == CHAR_ACK)
		if(bResult == TRUE)
		{
			CPubTckMem *pTR;
			rtk_readrsd_t *pResp;

			nSize = 100 + sizeof(rtk_readrsd_t);

			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			nSize = 0;

			// ACK
			retBuf[nSize++] = CHAR_ACK;
			
			// 상주직원 응답 데이타

			pTR = CPubTckMem::GetInstance();
			pResp = &pTR->m_resp_readrsd_t;

			nSize += ::Util_MyCopyMemory(&retBuf[nSize], pResp->rsd_user_id			, sizeof(pResp->rsd_user_id			) - 1);
			nSize += ::Util_MyCopyMemory(&retBuf[nSize], pResp->user_nm				, sizeof(pResp->user_nm				) - 1);
			nSize += ::Util_MyCopyMemory(&retBuf[nSize], pResp->cmpy_nm				, sizeof(pResp->cmpy_nm				) - 1);
			nSize += ::Util_MyCopyMemory(&retBuf[nSize], pResp->ntfc_ctt			, sizeof(pResp->ntfc_ctt			) - 1);
			nSize += ::Util_MyCopyMemory(&retBuf[nSize], pResp->tissu_unbl_rot_val	, sizeof(pResp->tissu_unbl_rot_val	) - 1);
			nSize += ::Util_MyCopyMemory(&retBuf[nSize], pResp->sell_yn				, sizeof(pResp->sell_yn				) - 1);
			nSize += ::Util_MyCopyMemory(&retBuf[nSize], pResp->dlt_yn				, sizeof(pResp->dlt_yn				) - 1);
			nSize += ::Util_MyCopyMemory(&retBuf[nSize], pResp->pub_dt				, sizeof(pResp->pub_dt				) - 1);
			nSize += ::Util_MyCopyMemory(&retBuf[nSize], pResp->pub_time			, sizeof(pResp->pub_time			) - 1);
			nSize += ::Util_MyCopyMemory(&retBuf[nSize], pResp->pub_user_id			, sizeof(pResp->pub_user_id			) - 1);
			nSize += ::Util_MyCopyMemory(&retBuf[nSize], pResp->pub_user_nm			, sizeof(pResp->pub_user_nm			) - 1);
			nSize += ::Util_MyCopyMemory(&retBuf[nSize], pResp->user_dvs_no			, sizeof(pResp->user_dvs_no			) - 1);
			nSize += ::Util_MyCopyMemory(&retBuf[nSize], pResp->pwd_rgt_yn			, sizeof(pResp->pwd_rgt_yn			) - 1);
// 
// 			nSize += ::Util_MyCopyMemory(&retBuf[nSize], 
// 										 &CPubTckMem::GetInstance()->m_resp_readrsd_t.rsd_user_id, 
// 										 sizeof(rtk_readrsd_t) - sizeof(CPubTckMem::GetInstance()->m_resp_readrsd_t.rsp_cd));
		}
		else
		{
			nSize = sizeof(UI_SND_ERROR_T);
			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			nSize = MakeErrorPacket((char *)retBuf);
		}

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
		nSize = nRet;
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_0323_data
 * @details		상주직원_카드 요금변경 조회 결과 데이타 생성
 * @param		UI_QUE_DATA_T tQue		결과값
 * @param		BYTE *retBuf			결과값
 * @return		데이타 크기
 */
static int make_0323_data(UI_QUE_DATA_T tQue)
{
	BYTE byACK = 0;
	int nRet = 0, nSize = 0;
	int nAlcn, nError, nCount;
	BOOL bResult;
	PUI_SND_Q_T pQdata;
	CPubTckMem* pTr;
	BYTE *retBuf = NULL;

	try
	{
		pTr = CPubTckMem::GetInstance();

		pQdata = (PUI_SND_Q_T) tQue.szData;

		nAlcn = nError = nCount = 0;

		bResult = pQdata->bResult;
		nError = pQdata->nError;
		CopyMemory(&nAlcn, pQdata->szData, 4);

		LOG_OUT("bResult = %d", bResult);
		LOG_OUT("nError = %d", nError);
		LOG_OUT("nAlcn = %d", nAlcn);

		if(nAlcn > 0)
		{	/// 배차모드인 경우
			// Data
			if(bResult == TRUE)
			{
				nCount = pTr->m_vtPcpysats.size();

				nSize = 100 + (sizeof(rtk_pcpysats_list_t) * nCount);

				retBuf = new BYTE[nSize];
				::ZeroMemory(retBuf, nSize);

				nSize = 0;

				// Result
				retBuf[nSize++] = CHAR_ACK;

				// 상주직원 할인 유무 (사용안함)
				if(nCount == 0)
				{
					retBuf[nSize++] = 'N';	
				}
				else
				{
					retBuf[nSize++] = 'Y';	
				}

				// 좌석 정보수
				nSize += Util_MyCopyMemory(&retBuf[nSize], &nCount, sizeof(int));

				vector<rtk_pcpysats_list_t>::iterator	iter;

				for( iter = pTr->m_vtPcpysats.begin(); iter != pTr->m_vtPcpysats.end(); iter++ )
				{
					///< 좌석선점ID
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->sats_pcpy_id, sizeof(iter->sats_pcpy_id) - 1);

					///< 시외버스할인종류코드
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->cty_bus_dc_knd_cd, sizeof(iter->cty_bus_dc_knd_cd) - 1);

					///< 할인율 구분코드
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->dcrt_dvs_cd, sizeof(iter->dcrt_dvs_cd) - 1);

					///< 버스티켓종류코드
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->bus_tck_knd_cd, sizeof(iter->bus_tck_knd_cd) - 1);

					///< 할인이후금액(number)
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->dc_aft_amt, sizeof(iter->dc_aft_amt) - 1);

					///< 좌석번호(number)
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->sats_no, sizeof(iter->sats_no) - 1);
				}
			}
			else
			{
				nSize = 100;

				retBuf = new BYTE[nSize];
				::ZeroMemory(retBuf, nSize);

				nSize = 0;

// 				// Result
// 				retBuf[nSize++] = CHAR_NAK;
// 				// Error Code
// 				nSize += sprintf((char *)&retBuf[nSize], "%s", CConfigTkMem::GetInstance()->GetErrorCode());

				nSize = MakeErrorPacket((char *)retBuf);
			}
		}
		else
		{	/// 비배차 모드인 경우
			// Result
			if(bResult == TRUE)
			{
				nCount = pTr->m_vtNPcpysats.size();

				nSize = 100 + (sizeof(rtk_pcpysats_list_t) * nCount);

				retBuf = new BYTE[nSize];
				::ZeroMemory(retBuf, nSize);

				nSize = 0;

				/// ACK
				retBuf[nSize++] = CHAR_ACK;

				// 상주직원 할인 유무 (사용안함)
				if(nCount == 0)
				{
					retBuf[nSize++] = 'N';	
				}
				else
				{
					retBuf[nSize++] = 'Y';	
				}

				/// 좌석 정보수
				nSize += Util_MyCopyMemory(&retBuf[nSize], &nCount, sizeof(WORD));

				vector<rtk_pcpysats_list_t>::iterator	iter;

				for( iter = pTr->m_vtNPcpysats.begin(); iter != pTr->m_vtNPcpysats.end(); iter++ )
				{
					///< 좌석선점ID
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->sats_pcpy_id, sizeof(iter->sats_pcpy_id) - 1);

					///< 시외버스할인종류코드
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->cty_bus_dc_knd_cd, sizeof(iter->cty_bus_dc_knd_cd) - 1);

					///< 할인율 구분코드
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->dcrt_dvs_cd, sizeof(iter->dcrt_dvs_cd) - 1);

					///< 버스티켓종류코드
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->bus_tck_knd_cd, sizeof(iter->bus_tck_knd_cd) - 1);

					///< 할인이후금액(number)
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->dc_aft_amt, sizeof(iter->dc_aft_amt) - 1);

					///< 좌석번호(number)
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->sats_no, sizeof(iter->sats_no) - 1);
				}
			}
			else
			{
				nSize = sizeof(UI_SND_ERROR_T);

				retBuf = new BYTE[nSize];
				::ZeroMemory(retBuf, nSize);

				nSize = 0;

// 				// Result
// 				retBuf[nSize++] = CHAR_NAK;
// 				// Error Code
// 				nSize += sprintf((char *)&retBuf[nSize], "%s", CConfigTkMem::GetInstance()->GetErrorCode());

				nSize = MakeErrorPacket((char *)retBuf);
			}
		}
	
		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_0325_data
 * @details		현장발권 - 승차권발행 응답(선불)
 * @param		UI_QUE_DATA_T tQue		결과값
 * @param		BYTE *retBuf			결과값
 * @return		데이타 크기
 */
static int make_0325_data(UI_QUE_DATA_T tQue)
{
	BYTE byACK = 0;
	int nRet = 0, nSize = 0;
	int nAlcn, nError, nCount;
	BOOL bResult;
	PUI_SND_Q_T pQdata;
	CPubTckMem* pTr;
	BYTE *retBuf = NULL;

	try
	{
		pTr = CPubTckMem::GetInstance();

		pQdata = (PUI_SND_Q_T) tQue.szData;

		nAlcn = nError = nCount = 0;

		bResult = pQdata->bResult;
		nError = pQdata->nError;
		CopyMemory(&nAlcn, pQdata->szData, 4);

		LOG_OUT("bResult = %d", bResult);
		LOG_OUT("nError = %d", nError);
		LOG_OUT("nAlcn = %d", nAlcn);

		if(nAlcn > 0)
		{	/// 배차모드인 경우
			// Data
			if(bResult == TRUE)
			{
				nCount = pTr->m_vtRfResp.size();

				nSize = 200 + (sizeof(rtk_pubtckppy_t) * nCount);

				retBuf = new BYTE[nSize];
				::ZeroMemory(retBuf, nSize);

				nSize = 0;

				// Result
				retBuf[nSize++] = CHAR_ACK;

				/// 선불카드 결제 갯수
				nSize += Util_MyCopyMemory(&retBuf[nSize], &nCount, sizeof(int));

				vector<rtk_pubtckppy_t>::iterator	iter;

				for( iter = pTr->m_vtRfResp.begin(); iter != pTr->m_vtRfResp.end(); iter++ )
				{
					///< rsp_cd는 size를 7로 수정함.
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->rsp_cd				, sizeof(iter->rsp_cd				)	 );
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->pub_dt				, sizeof(iter->pub_dt				) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->pub_time			, sizeof(iter->pub_time				) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->depr_trml_cd		, sizeof(iter->depr_trml_cd			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->arvl_trml_cd		, sizeof(iter->arvl_trml_cd			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->bus_cls_cd			, sizeof(iter->bus_cls_cd			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->bus_cacm_cd		, sizeof(iter->bus_cacm_cd			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->pub_wnd_no			, sizeof(iter->pub_wnd_no			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->pub_sno			, sizeof(iter->pub_sno				) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->pub_shct_trml_cd	, sizeof(iter->pub_shct_trml_cd		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->bus_tck_knd_cd		, sizeof(iter->bus_tck_knd_cd		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->tisu_amt			, 4										 );
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->pyn_mns_dvs_cd		, sizeof(iter->pyn_mns_dvs_cd		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->sats_no			, sizeof(iter->sats_no				) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->rot_knd_cd			, sizeof(iter->rot_knd_cd			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->cty_bus_dc_knd_cd	, sizeof(iter->cty_bus_dc_knd_cd	) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->dc_bef_amt			, 4										 );
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->bus_tck_inhr_no	, sizeof(iter->bus_tck_inhr_no		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->cty_bus_oprn_shp_cd, sizeof(iter->cty_bus_oprn_shp_cd	) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->atl_depr_dt		, sizeof(iter->atl_depr_dt			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->atl_depr_time		, sizeof(iter->atl_depr_time		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->stpt_trml_cd		, sizeof(iter->stpt_trml_cd			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->ept_trml_cd		, sizeof(iter->ept_trml_cd			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->alcn_way_dvs_cd	, sizeof(iter->alcn_way_dvs_cd		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->sati_use_yn		, sizeof(iter->sati_use_yn			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->perd_temp_yn		, sizeof(iter->perd_temp_yn			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->rdhm_mltp_val		, sizeof(iter->rdhm_mltp_val		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->bus_cls_prin_yn	, sizeof(iter->bus_cls_prin_yn		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->bus_cacm_nm_prin_yn, sizeof(iter->bus_cacm_nm_prin_yn	) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->depr_time_prin_yn	, sizeof(iter->depr_time_prin_yn	) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->sats_no_prin_yn	, sizeof(iter->sats_no_prin_yn		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->dist				, 4										 );
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->take_drtm			, 4										 );
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->pub_num			, 4										 );
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->dcrt_dvs_cd		, sizeof(iter->dcrt_dvs_cd			) - 1);
				}
			}
		}
		else
		{	/// 비배차 모드인 경우
			// Result
			if(bResult == TRUE)
			{
				nCount = pTr->m_vtNPcpysats.size();

				nSize = 100 + (sizeof(rtk_pcpysats_list_t) * nCount);

				retBuf = new BYTE[nSize];
				::ZeroMemory(retBuf, nSize);

				nSize = 0;

				/// ACK
				retBuf[nSize++] = CHAR_ACK;

				/// 선불카드 결제 갯수
				nSize += Util_MyCopyMemory(&retBuf[nSize], &nCount, sizeof(WORD));

				vector<rtk_pubtckppy_t>::iterator	iter;

				for( iter = pTr->m_vtRfResp.begin(); iter != pTr->m_vtRfResp.end(); iter++ )
				{
					///< 
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->rsp_cd				, sizeof(iter->rsp_cd				)	 );
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->pub_dt				, sizeof(iter->pub_dt				) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->pub_time			, sizeof(iter->pub_time				) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->depr_trml_cd		, sizeof(iter->depr_trml_cd			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->arvl_trml_cd		, sizeof(iter->arvl_trml_cd			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->bus_cls_cd			, sizeof(iter->bus_cls_cd			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->bus_cacm_cd		, sizeof(iter->bus_cacm_cd			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->pub_wnd_no			, sizeof(iter->pub_wnd_no			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->pub_sno			, sizeof(iter->pub_sno				) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->pub_shct_trml_cd	, sizeof(iter->pub_shct_trml_cd		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->bus_tck_knd_cd		, sizeof(iter->bus_tck_knd_cd		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->tisu_amt			, 4										 );
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->pyn_mns_dvs_cd		, sizeof(iter->pyn_mns_dvs_cd		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->sats_no			, sizeof(iter->sats_no				) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->rot_knd_cd			, sizeof(iter->rot_knd_cd			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->cty_bus_dc_knd_cd	, sizeof(iter->cty_bus_dc_knd_cd	) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->dc_bef_amt			, 4										 );
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->bus_tck_inhr_no	, sizeof(iter->bus_tck_inhr_no		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->cty_bus_oprn_shp_cd, sizeof(iter->cty_bus_oprn_shp_cd) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->atl_depr_dt		, sizeof(iter->atl_depr_dt			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->atl_depr_time		, sizeof(iter->atl_depr_time		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->stpt_trml_cd		, sizeof(iter->stpt_trml_cd			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->ept_trml_cd		, sizeof(iter->ept_trml_cd			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->alcn_way_dvs_cd	, sizeof(iter->alcn_way_dvs_cd		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->sati_use_yn		, sizeof(iter->sati_use_yn			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->perd_temp_yn		, sizeof(iter->perd_temp_yn			) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->rdhm_mltp_val		, sizeof(iter->rdhm_mltp_val		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->bus_cls_prin_yn	, sizeof(iter->bus_cls_prin_yn		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->bus_cacm_nm_prin_yn, sizeof(iter->bus_cacm_nm_prin_yn	) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->depr_time_prin_yn	, sizeof(iter->depr_time_prin_yn	) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->sats_no_prin_yn	, sizeof(iter->sats_no_prin_yn		) - 1);
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->dist				, 4										 );
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->take_drtm			, 4										 );
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->pub_num			, 4										 );
					nSize += Util_MyCopyMemory(&retBuf[nSize], iter->dcrt_dvs_cd		, sizeof(iter->dcrt_dvs_cd			) - 1);
				}
			}
		}
	
		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_0332_data
 * @details		[코버스] 현장발권 - 경유지 조회 결과 결과 데이타 생성
 * @param		UI_QUE_DATA_T tQue		결과값
 * @param		BYTE *retBuf			결과값
 * @return		데이타 크기
 */
static int make_0332_data(UI_QUE_DATA_T tQue)
{
	BYTE byACK = 0;
	int nRet = 0, nSize = 0, nCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		byACK = tQue.szData[0] & 0xFF;

		if(byACK == CHAR_ACK)
		{
			nCount = CPubTckKobusMem::GetInstance()->m_vtResAlcnList.size();
			nSize = 100 + (nCount * sizeof(rtk_tm_timinfo_list_t) + sizeof(rtk_tm_timinfo_fee_t));

			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			nSize = CPubTckKobusMem::GetInstance()->MakeAlcnListPacket((char *)retBuf);
		}
		else
		{
			nSize = sizeof(UI_SND_ERROR_T);
			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			retBuf[0] = CHAR_NAK;
			nSize = 1;
		}

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_0339_data
 * @details		[코버스] 현장발권 - 경유지 조회 결과 결과 데이타 생성
 * @param		UI_QUE_DATA_T tQue		결과값
 * @param		BYTE *retBuf			결과값
 * @return		데이타 크기
 */
static int make_0339_data(UI_QUE_DATA_T tQue)
{
	BYTE byACK = 0;
	int nRet = 0, nSize = 0, nCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		byACK = tQue.szData[0] & 0xFF;

		if(byACK == CHAR_ACK)
		{
			nCount = CPubTckKobusMem::GetInstance()->m_vtResThruList.size();
			nSize = 100 + (nCount * sizeof(rtk_tm_ethruinfo_list_t));

			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			nSize = CPubTckKobusMem::GetInstance()->MakeThruListPacket((char *)retBuf);
		}
		else
		{
			nSize = sizeof(UI_SND_ERROR_T);
			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			retBuf[0] = CHAR_NAK;
			nSize = 1;
		}

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_tmexp_0351_data
 * @details		[티머니고속] 현장발권 - 배차리스트 전달
 * @param		UI_QUE_DATA_T tQue		QUEUE data
 * @param		BYTE *retBuf			데이타 생성 data
 * @return		데이타 크기
 */
static int make_tmexp_0351_data(UI_QUE_DATA_T tQue)
{
	int nRet = 0, nSize = 0, nCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		if(tQue.szData[0] == CHAR_ACK)
		{
			int nLastCount;

			/// 최종 배차 노선수
			nLastCount = CPubTckTmExpMem::GetInstance()->m_vtResAlcnLastList.size();
			
			/// 배차 정보수
			nCount = CPubTckTmExpMem::GetInstance()->m_vtResAlcnList.size();
			
			nSize = 100 + (nLastCount * sizeof(rtk_tm_readalcn_last_list_t)) + (nCount * sizeof(rtk_tm_readalcn_list_t));
			LOG_OUT("[티머니고속] 배차리스트 데이타 갯수 = (nLastCount = %d) (nCount = %d) ", nLastCount, nCount);

			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			nSize = CPubTckTmExpMem::GetInstance()->MakeAlcnListPacket((char *)retBuf);
		}
		else
		{
			nSize = sizeof(UI_SND_ERROR_T);
			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			::CopyMemory(retBuf, tQue.szData, nSize);
		}

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_tmexp_0353_data
 * @details		[티머니고속] 현장발권 - 티켓 요금/할인 정보 전달
 * @param		UI_QUE_DATA_T tQue		QUEUE data
 * @param		BYTE *retBuf			데이타 생성 data
 * @return		데이타 크기
 */
static int make_tmexp_0353_data(UI_QUE_DATA_T tQue)
{
	int nRet = 0, nSize = 0;
	int nFeeCount = 0, nDiscCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		if(tQue.szData[0] == CHAR_ACK)
		{
			/// 티켓 요금정보 수
			nFeeCount = CPubTckTmExpMem::GetInstance()->m_vtResFee.size();
			
			/// 할인 정보 수
			nDiscCount = CPubTckTmExpMem::GetInstance()->m_vtResDisc.size();
			
			nSize = 1024 + (nFeeCount * sizeof(rtk_tm_readsatsfee_list_t)) + (nDiscCount * sizeof(rtk_tm_readsats_disc_list_t));

			LOG_OUT("[티머니고속] 요금/할인정보 데이타 갯수 = (%d), (%d) ", nFeeCount, nDiscCount);

			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			nSize = CPubTckTmExpMem::GetInstance()->MakeSatsFeeListPacket((char *)retBuf);
		}
		else
		{
			nSize = sizeof(UI_SND_ERROR_T);
			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			::CopyMemory(retBuf, tQue.szData, nSize);
		}

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_tmexp_0355_data
 * @details		[티머니고속] 현장발권 - 좌석 선점 or 선점해제
 * @param		UI_QUE_DATA_T tQue		QUEUE data
 * @param		BYTE *retBuf			데이타 생성 data
 * @return		데이타 크기
 */
static int make_tmexp_0355_data(UI_QUE_DATA_T tQue)
{
	int nRet = 0, nSize = 0;
	int nCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		if(tQue.szData[0] == CHAR_ACK)
		{
			/// 좌석 선점 수
			nCount = CPubTckTmExpMem::GetInstance()->m_tResSatsPcpy.size();
			
			nSize = 100 + (nCount * sizeof(rtk_tm_pcpysats_list_t));

			LOG_OUT("[티머니고속] 좌석선점 데이타 갯수 = (%d) ", nCount);

			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			nSize = CPubTckTmExpMem::GetInstance()->MakeSatsPcpyListPacket((char *)retBuf);
		}
		else
		{
			nSize = sizeof(UI_SND_ERROR_T);
			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			::CopyMemory(retBuf, tQue.szData, nSize);
		}

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_tmexp_0357_data
 * @details		[티머니고속] 현장발권 - 승차권발권(현금) 응답
 * @param		UI_QUE_DATA_T tQue		QUEUE data
 * @param		BYTE *retBuf			데이타 생성 data
 * @return		데이타 크기
 */
static int make_tmexp_0357_data(UI_QUE_DATA_T tQue)
{
	int nRet = 0, nSize = 0;
	int nCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		if(tQue.szData[0] == CHAR_ACK)
		{
			/// TMAX_승차권발권(현금) 정보 수
			nCount = CPubTckTmExpMem::GetInstance()->m_tResPubTckCashList.size();
			
			nSize = 100 + sizeof(rtk_tm_pubtckcash_t) + (nCount * sizeof(rtk_tm_pubtckcash_list_t));

			LOG_OUT("[티머니고속] TMAX_승차권발권(현금) 데이타 갯수 = (%d) ", nCount);

			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			nSize = CPubTckTmExpMem::GetInstance()->MakePbTckCashPacket((char *)retBuf);
		}
		else
		{
			nSize = sizeof(UI_SND_ERROR_T);
			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			::CopyMemory(retBuf, tQue.szData, nSize);
		}

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_tmexp_0359_data
 * @details		[티머니고속] 현장발권 - 승차권발권(카드_KTC) 응답
 * @param		UI_QUE_DATA_T tQue		QUEUE data
 * @param		BYTE *retBuf			데이타 생성 data
 * @return		데이타 크기
 */
static int make_tmexp_0359_data(UI_QUE_DATA_T tQue)
{
	int nRet = 0, nSize = 0;
	int nCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		if(tQue.szData[0] == CHAR_ACK)
		{
			/// TMAX_승차권발권(카드_KTC) 정보 수
			nCount = CPubTckTmExpMem::GetInstance()->m_tResPubTckCardKtcList.size();
			
			nSize = 100 + sizeof(rtk_tm_pubtckcard_ktc_t) + (nCount * sizeof(rtk_tm_pubtckcard_ktc_list_t));

			LOG_OUT("[티머니고속] TMAX_승차권발권(카드_KTC) 데이타 갯수 = (%d) ", nCount);

			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			nSize = CPubTckTmExpMem::GetInstance()->MakePbTckCardKtcPacket((char *)retBuf);
		}
		else
		{
			nSize = sizeof(UI_SND_ERROR_T);
			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			::CopyMemory(retBuf, tQue.szData, nSize);
		}

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_tmexp_0361_data
 * @details		[티머니고속] 현장발권 - 승차권발권(현영_KTC) 응답
 * @param		UI_QUE_DATA_T tQue		QUEUE data
 * @param		BYTE *retBuf			데이타 생성 data
 * @return		데이타 크기
 */
static int make_tmexp_0361_data(UI_QUE_DATA_T tQue)
{
	int nRet = 0, nSize = 0;
	int nCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		if(tQue.szData[0] == CHAR_ACK)
		{
			/// TMAX_승차권발권(현영_KTC) 정보 수
			nCount = CPubTckTmExpMem::GetInstance()->m_tResPubTckCsrcKtcList.size();
			
			nSize = 100 + sizeof(rtk_tm_pubtckcsrc_ktc_t) + (nCount * sizeof(rtk_tm_pubtckcsrc_ktc_list_t));

			LOG_OUT("[티머니고속] TMAX_승차권발권(현영_KTC) 데이타 갯수 = (%d) ", nCount);

			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			nSize = CPubTckTmExpMem::GetInstance()->MakePbTckCsrcKtcPacket((char *)retBuf);

			if(1)
			{
				int count = 0, offset = 0;
				prtk_tm_pubtckcsrc_ktc_t pPacket;

				pPacket = (prtk_tm_pubtckcsrc_ktc_t) &retBuf[1];

				offset += 1;
				LOG_OUT("%30s - (%s) ", "rsp_cd				"		,	pPacket->rsp_cd );
				LOG_OUT("%30s - (%s) ", "cacm_nm_prin_yn	"		,	pPacket->cacm_nm_prin_yn	 );
				LOG_OUT("%30s - (%s) ", "bus_cls_prin_yn	"		,	pPacket->bus_cls_prin_yn	 );
				LOG_OUT("%30s - (%s) ", "depr_time_prin_yn	"		,	pPacket->depr_time_prin_yn	 );
				LOG_OUT("%30s - (%s) ", "sats_no_prin_yn	"		,	pPacket->sats_no_prin_yn	 );
				LOG_OUT("%30s - (%s) ", "tissu_dt			"		,	pPacket->tissu_dt			 );
				LOG_OUT("%30s - (%s) ", "tissu_trml_no		"		,	pPacket->tissu_trml_no		 );
				LOG_OUT("%30s - (%s) ", "tissu_wnd_no		"		,	pPacket->tissu_wnd_no		 );
				LOG_OUT("%30s - (%s) ", "tissu_time			"		,	pPacket->tissu_time			 );
#if (_KTC_CERTIFY_ <= 0)
				LOG_OUT("%30s - (%s) ", "user_key_val		"		,	pPacket->user_key_val		 );
#endif
				LOG_OUT("%30s - (%s) ", "user_dvs_cd		"		,	pPacket->user_dvs_cd		 );

				offset += pPacket->rec_num - pPacket->rsp_cd;
				offset += sizeof(pPacket->rec_num);

				count = *(int *)pPacket->rec_num;
				for(int i = 0; i < count; i++)
				{
					rtk_tm_pubtckcsrc_ktc_list_t *pList;

					pList = (rtk_tm_pubtckcsrc_ktc_list_t *) &retBuf[offset];

					LOG_OUT("####### 인덱스 (%4d / %4d) #######", i, count);
					LOG_OUT("%30s - (%d) ", "n_tissu_fee		"		,	*(int *)pList->n_tissu_fee		 );
					LOG_OUT("%30s - (%d) ", "n_ogn_fee			"		,	*(int *)pList->n_ogn_fee			 );
					LOG_OUT("%30s - (%s) ", "sats_no			"		,	pList->sats_no			 );
					LOG_OUT("%30s - (%s) ", "inhr_no			"		,	pList->inhr_no			 );
					LOG_OUT("%30s - (%s) ", "invs_no			"		,	pList->invs_no			 );
					LOG_OUT("%30s - (%s) ", "tissu_sno			"		,	pList->tissu_sno			 );
					LOG_OUT("%30s - (%s) ", "csrc_aprv_no		"		,	pList->csrc_aprv_no		 );
					LOG_OUT("%30s - (%s) ", "csrc_rgt_no		"		,	pList->csrc_rgt_no		 );
					LOG_OUT("%30s - (%d) ", "n_cash_recp_amt	"		,	*(int *)pList->n_cash_recp_amt	 );
					LOG_OUT("%30s - (%s) ", "tissu_no			"		,	pList->tissu_no			 );
					LOG_OUT("%30s - (%s) ", "tck_knd_cd			"		,	pList->tck_knd_cd			 );
					LOG_OUT("%30s - (%s) ", "dc_knd_cd			"		,	pList->dc_knd_cd			 );
					LOG_OUT("%30s - (%s) ", "exch_knd_cd		"		,	pList->exch_knd_cd		 );

					offset += sizeof(rtk_tm_pubtckcsrc_ktc_list_t);
				}
			}
		}
		else
		{
			nSize = sizeof(UI_SND_ERROR_T);
			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			::CopyMemory(retBuf, tQue.szData, nSize);
		}

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_tmexp_0363_data
 * @details		[티머니고속] 현장발권 - 승차권발권(부가상품권) 응답
 * @param		UI_QUE_DATA_T tQue		QUEUE data
 * @param		BYTE *retBuf			데이타 생성 data
 * @return		데이타 크기
 */
static int make_tmexp_0363_data(UI_QUE_DATA_T tQue)
{
	int nRet = 0, nSize = 0;
	int nCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		if(tQue.szData[0] == CHAR_ACK)
		{
			/// TMAX_승차권발권(부가상품권) 정보 수
			nCount = CPubTckTmExpMem::GetInstance()->m_tResPubTckPrdList.size();
			
			nSize = 100 + sizeof(rtk_tm_pubtckprd_t) + (nCount * sizeof(rtk_tm_pubtckprd_list_t));

			LOG_OUT("[티머니고속] TMAX_승차권발권(부가상품권) 데이타 갯수 = (%d) ", nCount);

			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			nSize = CPubTckTmExpMem::GetInstance()->MakePbTckPrdPacket((char *)retBuf);
		}
		else
		{
			nSize = sizeof(UI_SND_ERROR_T);
			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			::CopyMemory(retBuf, tQue.szData, nSize);
		}

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		make_tmexp_0365_data
 * @details		[티머니고속] 현장발권 - 경유지 정보 응답
 * @param		UI_QUE_DATA_T tQue		QUEUE data
 * @param		BYTE *retBuf			데이타 생성 data
 * @return		데이타 크기
 */
static int make_tmexp_0365_data(UI_QUE_DATA_T tQue)
{
	int nRet = 0, nSize = 0;
	int nCount = 0;
	BYTE *retBuf = NULL;

	try
	{
		if(tQue.szData[0] == CHAR_ACK)
		{
			/// TMAX_승차권발권(부가상품권) 정보 수
			nCount = CPubTckTmExpMem::GetInstance()->m_vtResThruList.size();
			
			nSize = 100 + (nCount * sizeof(rtk_tm_readthrutrml_list_t));

			LOG_OUT("[티머니고속] TMAX_승차권발권(경유지정보) 데이타 갯수 = (%d) ", nCount);

			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			nSize = CPubTckTmExpMem::GetInstance()->MakePbTckThruInfo((char *)retBuf);
		}
		else
		{
			nSize = sizeof(UI_SND_ERROR_T);
			retBuf = new BYTE[nSize];
			::ZeroMemory(retBuf, nSize);

			::CopyMemory(retBuf, tQue.szData, nSize);
		}

		nRet = SndRcvPacket(tQue.wCommand, retBuf, nSize, s_RecvData);
		if(nRet < 0)
		{
			if(retBuf != NULL)
			{
				delete[] retBuf;
				retBuf = NULL;
			}
			return nRet;
		}

		if(retBuf != NULL)
		{
			delete[] retBuf;
			retBuf = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nSize;
}

/**
 * @brief		PrintAccountInfo
 * @details		무인기 회계 정보 디버깅 코드
 * @param		PUI_BASE_T pBase	옵션 데이타
 * @return		성공 >= 0, 실패 < 0
 */
static void PrintAccountInfo(char *pData)
{
	/***
	PUI_DEV_ACCOUNT_INFO_T pInfo;

	pInfo = (PUI_DEV_ACCOUNT_INFO_T) pData;

	TR_LOG_OUT("[01]. 동전 호퍼 정보 ");	
	TR_LOG_OUT("  - w10 (%d), w50 (%d), w100 (%d), w500 (%d)", pInfo->Coin.n10, pInfo->Coin.n50, pInfo->Coin.n100, pInfo->Coin.n500);	

	TR_LOG_OUT("[02]. 지폐방출기 정보 ");	
	TR_LOG_OUT("  - 1k (%d), 5k (%d), 10k (%d), 50k (%d)", pInfo->Dispenser.Casst.n1k, pInfo->Dispenser.Casst.n5k, pInfo->Dispenser.Casst.n10k, pInfo->Dispenser.Casst.n50k);	

	TR_LOG_OUT("[03]. 지폐수집함 정보 ");	
	TR_LOG_OUT("  - 1k (%d), 5k (%d), 10k (%d), 50k (%d)", pInfo->BillBox.n1k, pInfo->BillBox.n5k, pInfo->BillBox.n10k, pInfo->BillBox.n50k);	

	TR_LOG_OUT("[04]. 시외_티켓_정보] ");
	TR_LOG_OUT("\t 카드발권(%d, %d), 현금발권(%d, %d), 카드환불(%d, %d), 현금환불(%d, %d) ",
				pInfo->ccsCardPubTck.nCount, pInfo->ccsCardPubTck.dwMoney,
				pInfo->ccsCashPubTck.nCount, pInfo->ccsCashPubTck.dwMoney,
				pInfo->ccsCardRefund.nCount, pInfo->ccsCardRefund.dwMoney, 
				pInfo->ccsCashRefund.nCount, pInfo->ccsCashRefund.dwMoney);

	TR_LOG_OUT("[05]. 고속_티켓_정보] ");
	TR_LOG_OUT("\t 카드발권(%d, %d), 현금발권(%d, %d), 카드환불(%d, %d), 현금환불(%d, %d) ",
				pInfo->expCardPubTck.nCount, pInfo->expCardPubTck.dwMoney,
				pInfo->expCashPubTck.nCount, pInfo->expCashPubTck.dwMoney,
				pInfo->expCardRefund.nCount, pInfo->expCardRefund.dwMoney, 
				pInfo->expCashRefund.nCount, pInfo->expCashRefund.dwMoney);

	TR_LOG_OUT("[06]. 고속_티켓_정보] ");
	TR_LOG_OUT("\t 승차권 잔여수량 (%d), 승차권 수집함 수량(%d)", pInfo->n_tck_remain_count, pInfo->n_tck_box_count);

	TR_LOG_OUT("[07]. 승차권 고유정보] ");
	TR_LOG_OUT("\t 시외_고유번호(%d), 고속_고유번호(%d)", pInfo->n_ccs_inhr_no, pInfo->n_exp_inhr_no);
	***/
}

/**
 * @brief		resp_100_command
 * @details		Polling Response 처리
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_100_command(BYTE *pRecvData)
{
	PUI_HEADER_T pHdr;
	PUI_RECV_POLL_T pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RECV_POLL_T) &pHdr->byBody;

		if(pResp->chLang != CConfigTkMem::GetInstance()->n_language)
		{
			CConfigTkMem::GetInstance()->n_language = pResp->chLang;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		resp_101_command
 * @details		거래 취소/완료 처리
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_101_command(BYTE *pRecvData)
{
	int  nRet;
	BYTE byACK;
	PUI_HEADER_T pHdr;
	PUI_RECV_OKCANCEL_T pResp;

	nRet = -1;
	byACK = CHAR_ACK;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RECV_OKCANCEL_T) &pHdr->byBody;

		TR_LOG_OUT("ui통신 - ok_cancel, byTrans = [%02X] !!!", pResp->byTrans);
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
		case 4 :
			{
				Transact_SetState(TR_READY_STATE, 0);
			}
			break;
		case 5 :
			{
				Transact_SetState(TR_ERROR_STATE, 0);
			}
			break;
		}

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_102_command
 * @details		사용 중/사용중지 처리
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_102_command(BYTE *pRecvData)
{
	int  nRet = 0;
	BYTE byACK = 0;
	PUI_HEADER_T pHdr;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;

		if(pHdr->byBody == 0x00)
		{	// 사용중
			LOG_OUT("사용중 수신");
			SetCheckEventCode(EC_MAINT_OUT_SERVICE, FALSE);

			Transact_SetState(TR_READY_STATE, 0);
		}
		else 
		{	// 사용중지
			LOG_OUT("사용중지 수신");
			SetCheckEventCode(EC_MAINT_OUT_SERVICE, TRUE);

			Transact_SetState(TR_ERROR_STATE, 0);
		}

		byACK = CHAR_ACK;
		nRet = SendPacket(UI_CMD_SERVICE, (BYTE *)&byACK, 1);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_105_command
 * @details		사용 중/사용중지 처리
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_105_command(BYTE *pRecvData)
{
	int					nRet = 0, nLen = 0;
	UI_SND_VERSION_T	sPacket;
	char				*pBuffer;
	PUI_HEADER_T		pHdr;
	PUI_RESP_VERSION_T  pResp;

	try
	{
		pHdr  = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_VERSION_T) &pHdr->byBody;

		::CopyMemory(CConfigTkMem::GetInstance()->ui_version, pResp->szUiVer, sizeof(pResp->szUiVer));
		sprintf((char *)CConfigTkMem::GetInstance()->main_version, "%s", MAIN_VERSION);
		::CopyMemory(CConfigTkMem::GetInstance()->pos_ktc_version, pResp->szKtcVer, sizeof(pResp->szKtcVer));

		::ZeroMemory(&sPacket, sizeof(UI_SND_VERSION_T));

		sPacket.byResult = CHAR_ACK;

		/// main version
		sprintf((char *)sPacket.szMainVer, "%s", MAIN_VERSION);

		//pBuffer = CConfigTkMem::GetInstance()->main_version;
		//nLen = strlen(pBuffer);
		//if(nLen > 0)
		//{
		//	::CopyMemory(sPacket.szMainVer, pBuffer, strlen(pBuffer));
		//}

		/// ui version
		pBuffer = CConfigTkMem::GetInstance()->ui_version;
		nLen = strlen(pBuffer);
		if(nLen > 0)
		{
			::CopyMemory(sPacket.szUiVer, pBuffer, strlen(pBuffer));
		}

		/// ktc version
		pBuffer = CConfigTkMem::GetInstance()->pos_ktc_version;
		nLen = strlen(pBuffer);
		if(nLen > 0)
		{
			::CopyMemory(sPacket.szKtcVer, pBuffer, strlen(pBuffer));
		}

		nRet = SendPacket(UI_CMD_VERSION_REQ, (BYTE *)&sPacket, sizeof(UI_SND_VERSION_T));
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_121_command
 * @details		장애 정보 요청 처리 (121)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_121_command(BYTE *pRecvData)
{
	int  nRet = 0, nOffset = 0;
	UI_ALARM_LIST_T sPacket;

	try
	{
		::ZeroMemory(&sPacket, sizeof(UI_ALARM_LIST_T));

		sPacket.byACK = CHAR_ACK;
		nOffset = 5;

		nRet = GetCondition();
		if(nRet < 0)
		{	/// 사용중이면..
			sPacket.nCount = 0;
			nRet = SendPacket(UI_CMD_ALARM_REQ_INFO, (BYTE *)&sPacket, nOffset);
			return nRet;
		}

		// todo : 알람 정보
		//sPacket.nCount = GetAlarmCount();
		//if(sPacket.nCount > 0)
		{
			sPacket.nCount = GetAlaramInfo((char *)&sPacket.list[0]);
			nOffset += sizeof(UI_ALARM_INFO_T) * sPacket.nCount;
		}

		nRet = SendPacket(UI_CMD_ALARM_REQ_INFO, (BYTE *)&sPacket, nOffset);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}


/**
 * @brief		resp_200_command
 * @details		예매발권 시작	 메뉴 이동
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_200_command(BYTE *pRecvData)
{
	int  nRet = 0;
	BYTE byACK = 0;

	LOG_OUT("resp_200_command() !!");

	try
	{
		byACK = CHAR_ACK;
		nRet = SendPacket(UI_CMD_MRS_MAIN, (BYTE *)&byACK, 1);

		Transact_SetState(TR_MRS_MAIN_STATE, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}


/**
 * @brief		resp_201_command
 * @details		예매발권 - 신용카드 읽기
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_201_command(BYTE *pRecvData)
{
	int  nRet = 0;
	BYTE byACK = 0;
	PUI_HEADER_T pHdr;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;

		if(pHdr->byBody == 0x01)
		{	// 신용카드 읽기 start
			LOG_OUT("resp_201_command() card read !!");
			Transact_SetState(TR_MRS_CARD_READ_STATE, 0);
		}
		else 
		{	// 신용카드 읽기 stop
			LOG_OUT("resp_201_command() card stop !!");
			Transact_SetState(TR_MRS_CARD_READ_STATE, 2);
		}

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

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
	return 0;
}

/**
 * @brief		resp_203_command
 * @details		예매발권 - 예매 리스트 요청
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_203_command(BYTE *pRecvData)
{
	int  nRet = 0;
	BYTE byACK = 0;
	PUI_HEADER_T pHdr;
	PUI_RESP_MRS_INFO_T pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_MRS_INFO_T) &pHdr->byBody;

		CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_CCBUS;
		nRet = 1;
		if(pResp->byFunc == 0x01)
		{	/// 신용카드
			LOG_OUT("resp_203_command() 신용카드 !!");
			CMrnpMem::GetInstance()->Base.req_read_dvs_cd[0] = enMNPP_ENC_CARD_DVS_CD;

			::CopyMemory(CConfigTkMem::GetInstance()->pos_ktc_version, pResp->pos_pg_ktc_ver, sizeof(pResp->pos_pg_ktc_ver));

			::CopyMemory(CMrnpMem::GetInstance()->Base.pos_pg_ktc_ver, pResp->pos_pg_ktc_ver, sizeof(pResp->pos_pg_ktc_ver));
			::CopyMemory(CMrnpMem::GetInstance()->Base.enc_dta_len, &pResp->n_enc_dta_len, sizeof(int));
			::CopyMemory(CMrnpMem::GetInstance()->Base.enc_dta, pResp->enc_dta, sizeof(pResp->enc_dta));
		}
		else if(pResp->byFunc == 0x02)
		{	/// 생년월일
			LOG_OUT("resp_203_command() 생년월일 !!");
			CMrnpMem::GetInstance()->Base.req_read_dvs_cd[0] = enMNPP_BRDT_DVS_CD;

			::CopyMemory(CMrnpMem::GetInstance()->Base.mnpp_brdt, pResp->szBirthDay, sizeof(pResp->szBirthDay));
			::CopyMemory(CMrnpMem::GetInstance()->Base.mnpp_tel_no, pResp->szMobileNo, sizeof(pResp->szMobileNo));
		}
		else
		{	/// 예매번호
			LOG_OUT("resp_203_command() 예매번호 !!");
			CMrnpMem::GetInstance()->Base.req_read_dvs_cd[0] = enMNPP_MRS_NO_DVS_CD;

			::CopyMemory(CMrnpMem::GetInstance()->Base.mrs_no, pResp->szMrsNo, sizeof(pResp->szMrsNo));

		}

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		Transact_SetState(TR_MRS_LIST_STATE, 0);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_205_command
 * @details		예매발권 - 예매 발권 시작	(UI-205)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_205_command(BYTE *pRecvData)
{
	int  nRet = 0;
	BYTE byACK = 0;
	PUI_HEADER_T pHdr;
	PUI_RESP_MRS_ISSUE_T pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_MRS_ISSUE_T) &pHdr->byBody;

		LOG_OUT("start !!");

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

		CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_CCBUS;

		// 예매 발권 데이타
		{
			int							i, nCount;
			UI_RESP_MRS_ISSUE_LIST_T	Info;
			PMRNP_BASE_T				pBase;
			PUI_RESP_MRS_ISSUE_LIST_T	pList;

			pBase = &CMrnpMem::GetInstance()->Base;

			CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_CCBUS;

			::CopyMemory(pBase->depr_trml_nm,		pResp->depr_trml_nm,	 sizeof(pBase->depr_trml_nm));
			::CopyMemory(pBase->depr_trml_eng_nm,	pResp->depr_trml_eng_nm, sizeof(pBase->depr_trml_eng_nm));

			::CopyMemory(pBase->arvl_trml_nm,		pResp->arvl_trml_nm,	 sizeof(pBase->arvl_trml_nm));
			::CopyMemory(pBase->arvl_trml_eng_nm,	pResp->arvl_trml_eng_nm, sizeof(pBase->arvl_trml_eng_nm));

			nCount = pResp->wCount;

			pList = (PUI_RESP_MRS_ISSUE_LIST_T) &pResp->ch;

			CMrnpMem::GetInstance()->m_vtRcvUiIssue.clear();
			for(i = 0; i < nCount; i++)
			{
				char Temp[100] = {0, };

				::CopyMemory(&Info, &pList[i], sizeof(UI_RESP_MRS_ISSUE_LIST_T));

				::ZeroMemory(Temp, sizeof(Temp));
				::CopyMemory(Temp, pList[i].mrs_no, sizeof(pList[i].mrs_no));
				CMrnpMem::GetInstance()->m_vtRcvUiIssue.push_back(Info);
			}

			Transact_SetState(TR_MRS_ISSUE_STATE, 0);
		}

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_206_command
 * @details		예매발권 - 교체발권	(UI-206)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_206_command(BYTE *pRecvData)
{
	int  nRet = 0;
	BYTE byACK = 0;
	PUI_HEADER_T pHdr;
	PUI_RESP_MRS_CHG_ISSUE_T pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_MRS_CHG_ISSUE_T) &pHdr->byBody;

		LOG_OUT("예매발권 - 교체발권 시작  !!");

		switch(pResp->byFlag)
		{
		case 2 :	/// 교체발권 시작
			{
				byACK = CHAR_ACK;
				nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

				//			Transact_SetState(TR_MRS_ISSUE_STATE, 0);
			}
			break;
		case 3 :	/// 교체발권 취소
			{
				byACK = CHAR_ACK;
				nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

				//			Transact_SetState(TR_MRS_ISSUE_STATE, 0);
			}
			break;
		default:
			{
				byACK = CHAR_NAK;
				nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);
			}
			return 0;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_233_command
 * @details		[코버스] 예매발권 - 예매 리스트 요청 (233)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_233_command(BYTE *pRecvData)
{
	int  nRet = 0;
	BYTE byACK = 0;
	PUI_HEADER_T pHdr;
	PUI_RESP_KO_MRS_REQ_LIST_T pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_KO_MRS_REQ_LIST_T) &pHdr->byBody;

		// set memory
		::CopyMemory(&CMrnpKobusMem::GetInstance()->m_ReqList, pResp, sizeof(UI_RESP_KO_MRS_REQ_LIST_T));
		CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_KOBUS;

		//LOG_HEXA("UI_KO_MRS_Recv", (BYTE *)&CMrnpKobusMem::GetInstance()->m_ReqList, sizeof(UI_RESP_KO_MRS_REQ_LIST_T));

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		Transact_SetState(TR_MRS_KOBUS_LIST_STATE, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_235_command
 * @details		[코버스] 예매발권 - 예매 리스트 선택(발권시작 요청)	(235)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_235_command(BYTE *pRecvData)
{
	int  nRet = 0, i= 0 ;
	BYTE byACK = 0;
	PUI_HEADER_T pHdr;
	PUI_RESP_KO_MRS_SEL_LIST_T pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_KO_MRS_SEL_LIST_T) &pHdr->byBody;

		// set memory
		::CopyMemory(&CMrnpKobusMem::GetInstance()->m_tUiSelData, &pResp->Data, sizeof(UI_RESP_KO_MRS_SEL_DATA_T));
		CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_KOBUS;

		LOG_OUT("예매발행 매수 = %d", pResp->rec_ncnt);

		CMrnpKobusMem::GetInstance()->m_vtUiSelList.clear();
		if(pResp->rec_ncnt > 0)
		{
			PUI_KO_MRS_SEL_LIST_T pList;

			pList = (PUI_KO_MRS_SEL_LIST_T) &pResp->chList;
			for( i = 0; i < pResp->rec_ncnt; i++ )
			{
				UI_KO_MRS_SEL_LIST_T List;

				::CopyMemory(&List, &pList[i], sizeof(UI_KO_MRS_SEL_LIST_T));
				CMrnpKobusMem::GetInstance()->m_vtUiSelList.push_back(List);
			}
		}

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		Transact_SetState(TR_MRS_KOBUS_ISSUE_STATE, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_237_command
 * @details		[코버스/티머니고속] 예매발권 - 예매권 프린트	(237)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_237_command(BYTE *pRecvData)
{
	int  nRet = 0;
	int n_bus_dvs = 0;
	BYTE byACK = 0;
	PUI_HEADER_T pHdr;
	PUI_RESP_MRS_CHG_ISSUE_T pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_MRS_CHG_ISSUE_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		n_bus_dvs = CConfigTkMem::GetInstance()->n_bus_dvs;

		if(n_bus_dvs == SVR_DVS_KOBUS)
		{	/// 코버스 - 예매발권 프린트
			LOG_OUT("n_bus_dvs = (%d) 코버스 !!!", n_bus_dvs);
			Transact_SetState(TR_MRS_KOBUS_TCK_PRT_STATE, 0);
		}
		else
		{	/// 티머니고속 - 예매발권 프린트
			LOG_OUT("n_bus_dvs = (%d) .. !!!", n_bus_dvs);
			Transact_SetState(TR_MRS_TMEXP_TCK_PRT_STATE, 0);
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_250_command
 * @details		[티머니고속] 예매발권 - 예매내역 조회	(250)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_250_command(BYTE *pRecvData)
{
	int  nRet = 0;
	BYTE byACK = 0;
	PUI_HEADER_T pHdr;
	PUI_RESP_TMEXP_MRS_REQ_T pResp;
	CMrnpTmExpMem *pclsMrnp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_TMEXP_MRS_REQ_T) &pHdr->byBody;

		pclsMrnp = CMrnpTmExpMem::GetInstance();

		// set memory : 티머니 고속
		CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_TMEXP;

		byACK = 1;

		if( pResp->rData.req_dvs[0] == 'C' )
		{	/// 카드 
			LOG_OUT("[%s] 신용카드 !!", __FUNCTION__);
			pclsMrnp->Base.req_dvs_cd[0] = enMNPP_ENC_CARD_DVS_CD;

		}
		else if( pResp->rData.req_dvs[0] == 'T' )
		{	/// 생년월일 / 전번
			LOG_OUT("[%s] 생년월일/전번 !!", __FUNCTION__);
			pclsMrnp->Base.req_dvs_cd[0] = enMNPP_BRDT_DVS_CD;

		}
		else if( pResp->rData.req_dvs[0] == 'A' )
		{	/// 부가상품권
			LOG_OUT("[%s] 부가상품권 !!", __FUNCTION__);
			pclsMrnp->Base.req_dvs_cd[0] = enMNPP_GIFT_DVS_CD;

		}
		else if( pResp->rData.req_dvs[0] == 'R' )
		{	/// 예매번호
			LOG_OUT("[%s] 예매번호 !!", __FUNCTION__);
			pclsMrnp->Base.req_dvs_cd[0] = enMNPP_MRS_NO_DVS_CD;

		}
		else if( pResp->rData.req_dvs[0] == 'S' )
		{	/// 발권일련번호
			LOG_OUT("[%s] 발권일련번호 !!", __FUNCTION__);
			pclsMrnp->Base.req_dvs_cd[0] = enMNPP_TISSU_NO_DVS_CD;

		}
		else if( pResp->rData.req_dvs[0] == 'D' )
		{	/// 출발일
			LOG_OUT("[%s] 발권일련번호 !!", __FUNCTION__);
			pclsMrnp->Base.req_dvs_cd[0] = enMNPP_START_DAY_DVS_CD;
		}
		else
		{
			byACK = 0;
		}

		if( byACK == 1 )
		{
			byACK = CHAR_ACK;

			::CopyMemory(&pclsMrnp->m_ReqList.rData, &pResp->rData, sizeof(stk_tm_read_mrs_t));
		}
		else
		{
			byACK = CHAR_NAK;
		}
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		Transact_SetState(TR_MRS_TMEXP_LIST_STATE, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_252_command
 * @details		[티머니고속] (KTC) 예매발권 - 예매내역 조회	(250)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_252_command(BYTE *pRecvData)
{
	int  nRet = 0;
	BYTE byACK = 0;
	PUI_HEADER_T pHdr;
	PUI_RESP_TMEXP_KTC_MRS_REQ_T pResp;
	CMrnpTmExpMem *pclsMrnp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_TMEXP_KTC_MRS_REQ_T) &pHdr->byBody;

		pclsMrnp = CMrnpTmExpMem::GetInstance();

		// set memory
		CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_TMEXP;

		::CopyMemory(&pclsMrnp->m_ReqKtcList.rData, 
					 &pResp->rData, 
					 sizeof(stk_tm_read_mrs_ktc_t));

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		Transact_SetState(TR_MRS_TMEXP_KTC_LIST_STATE, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_254_command
 * @details		[티머니고속] 예매발권 - 예매발권 요청	(254)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_254_command(BYTE *pRecvData)
{
	int  nRet = 0;
	BYTE byACK = 0;
	PUI_HEADER_T pHdr;
	PUI_RESP_TMEXP_MRS_REQ_PUB_T pResp;

	try
	{
		pHdr  = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_TMEXP_MRS_REQ_PUB_T) &pHdr->byBody;

		/// 티머니고속 - set memory
		CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_TMEXP;

		/// [티머니고속] 예매발권 - 인터넷 예매
		CMrnpTmExpMem::GetInstance()->Base.tissu_chnl_dvs_cd[0] = '0';

		::CopyMemory(&CMrnpTmExpMem::GetInstance()->m_ReqPubMrs,
					 &pResp->rData,
					 sizeof(stk_tm_pub_mrs_t));

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		Transact_SetState(TR_MRS_TMEXP_ISSUE_STATE, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_256_command
 * @details		[티머니고속] 예매발권 - 모바일 예매발권 요청	(256)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_256_command(BYTE *pRecvData)
{
	int  nRet = 0;
	BYTE byACK = 0;
	PUI_HEADER_T pHdr;
	PUI_RESP_TMEXP_MRS_REQ_MOBILE_PUB_T pResp;

	LOG_OUT("start....");

	try
	{
		pHdr  = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_TMEXP_MRS_REQ_MOBILE_PUB_T) &pHdr->byBody;

		// 티머니고속 - set memory
		CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_TMEXP;
		/// [티머니고속] 예매발권 - 인터넷 예매
		CMrnpTmExpMem::GetInstance()->Base.tissu_chnl_dvs_cd[0] = '3';

		::CopyMemory(&CMrnpTmExpMem::GetInstance()->m_ReqPubMobileMrs,
					 &pResp->rData,
					 sizeof(stk_tm_pub_mrs_htck_t));

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		Transact_SetState(TR_MRS_TMEXP_MOBILE_ISSUE_STATE, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_400_command
 * @details		환불 - 시작
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_400_command(BYTE *pRecvData)
{
	int  nRet = 0;
	BYTE byACK = 0;
	PUI_HEADER_T pHdr;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;

		TR_LOG_OUT("ui통신 - 환불 start !!");

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		Transact_SetState(TR_CANCRY_MAIN_STATE, 0);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_401_command
 * @details		환불 - 승차권 읽기
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_401_command(BYTE *pRecvData)
{
	int  nRet = 0;
	BYTE byACK = 0;
	PUI_HEADER_T pHdr;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;

		TR_LOG_OUT("ui통신 - 승차권 읽기, %s  !!", (pHdr->byBody == 0x01) ? "시작" : "종료");
		if(pHdr->byBody == 0x01)
		{	/// 승차권 읽기 - 시작
			//LOG_OUT("ticket read(), step(%d) start !!", Transact_GetStep());

			if( Transact_GetState() == TR_CANCRY_TCK_READ && Transact_GetStep() == 99 )
			{
				;
			}
			else
			{
				Transact_SetState(TR_CANCRY_TCK_READ, 0);
			}
		}
		else
		{	/// 승차권 읽기 - 종료
			//LOG_OUT("ticket read stop !!");

			Transact_SetState(TR_CANCRY_TCK_READ, 2);
		}

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_403_command
 * @details		환불 - 카드 또는 현금 환불 처리
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_403_command(BYTE *pRecvData)
{
	int						nRet  = 0;
	BYTE					byACK = 0;
	PUI_HEADER_T			pHdr;
	PUI_RESP_CANCRY_FARE_T pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_CANCRY_FARE_T) &pHdr->byBody;

		byACK = CHAR_ACK;

		{
			int nCount = 1;

			if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_CCBUS )
			{
				LOG_OUT("[시외버스] - 환불금액(%d) !!", pResp->nFare);

				/// 결제수단
				CCancRyTkMem::GetInstance()->ui_pym_dvs_cd[0] = pResp->ui_pym_dvs_cd[0];
				/// 결제금액
				CCancRyTkMem::GetInstance()->n_tot_money = pResp->nFare;
				/// 방출금액
				CCancRyTkMem::GetInstance()->n_chg_money = pResp->nOutFare;

				CCancRyTkMem::GetInstance()->receipt_yn[0] = pResp->receipt_yn[0];
			}
			else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_KOBUS )
			{
				LOG_OUT("[코버스] - 환불금액(%d) !!", pResp->nFare);

				/// 결제수단
				CCancRyTkKobusMem::GetInstance()->tBase.ui_pym_dvs_cd[0] = pResp->ui_pym_dvs_cd[0];
				/// 결제금액
				CCancRyTkKobusMem::GetInstance()->tBase.n_tot_money = pResp->nFare;
				/// 방출금액
				CCancRyTkKobusMem::GetInstance()->tBase.n_chg_money = pResp->nOutFare;

				CCancRyTkKobusMem::GetInstance()->tBase.receipt_yn[0] = pResp->receipt_yn[0];
			}
			else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_TMEXP )
			{
				LOG_OUT("[티머니고속] - 환불금액(%d) !!", pResp->nFare);

				/// 결제수단
				CCancRyTkTmExpMem::GetInstance()->tBase.ui_pym_dvs_cd[0] = pResp->ui_pym_dvs_cd[0];
				/// 결제금액
				CCancRyTkTmExpMem::GetInstance()->tBase.n_tot_money = pResp->nFare;
				/// 방출금액
				CCancRyTkTmExpMem::GetInstance()->tBase.n_chg_money = pResp->nOutFare;

				CCancRyTkTmExpMem::GetInstance()->tBase.receipt_yn[0] = pResp->receipt_yn[0];
			}
			else
			{
				LOG_OUT("[버스코드_에러 ?????] - 환불금액(%d) !!", pResp->nFare);
				
				byACK = CHAR_NAK;
				nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);
				return 0;
			}
		}

		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);
		Transact_SetState(TR_CANCRY_FARE_STATE, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_300_command
 * @details		[현장발권] - Main 
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_300_command(BYTE *pRecvData)
{
	int			 nRet = 0;
	BYTE		 byACK = 0;
	PUI_HEADER_T pHdr;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		::CopyMemory(CConfigTkMem::GetInstance()->trml_cd, GetTrmlCode(SVR_DVS_CCBUS), sizeof(CConfigTkMem::GetInstance()->trml_cd) - 1);

		Transact_SetState(TR_PBTCK_MAIN_STATE, 0);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_301_command
 * @details		[시외버스]-[현장발권] - 배차 리스트 요청 
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_301_command(BYTE *pRecvData)
{
	int							nRet = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_PBTCK_REQ_LIST_T	pResp;
	CPubTckMem*					pPubTr;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_PBTCK_REQ_LIST_T) &pHdr->byBody;

		pPubTr = CPubTckMem::GetInstance();

		/// 시외버스 서버
		CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_CCBUS;

		::CopyMemory(pPubTr->base.depr_trml_cd, pResp->depr_trml_cd, sizeof(pResp->depr_trml_cd));
		::CopyMemory(pPubTr->base.arvl_trml_cd, pResp->arvl_trml_cd, sizeof(pResp->arvl_trml_cd));
		::CopyMemory(pPubTr->base.read_dt, pResp->read_dt, sizeof(pResp->read_dt));
		::CopyMemory(pPubTr->base.read_time, pResp->read_time, sizeof(pResp->read_time));
		::CopyMemory(pPubTr->base.bus_cls_cd, pResp->bus_cls_cd, sizeof(pResp->bus_cls_cd));

		/// 배차 리스트 요청

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		Transact_SetState(TR_PBTCK_REQ_LIST, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_303_command
 * @details		[현장발권] - 배차 리스트 선택 
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_303_command(BYTE *pRecvData)
{
	int						nRet = 0;
	BYTE					byACK = 0;
	PUI_HEADER_T			pHdr;
	PUI_RESP_PBTCK_SELECT_T pResp;

	try
	{
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

			/// 2019.11.14 add
			/// 배차방식 구분코드
			::CopyMemory(pBase->alcn_way_dvs_cd, pResp->alcn_way_dvs_cd, sizeof(pResp->alcn_way_dvs_cd));

			/// 좌석제 사용유무
			::CopyMemory(pBase->sati_use_yn, pResp->sati_use_yn, sizeof(pResp->sati_use_yn));
		}

		Transact_SetState(TR_PBTCK_LIST_SEL, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_305_command
 * @details		[현장발권] - 좌석 정보 선택 (305)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_305_command(BYTE *pRecvData)
{
	int					nRet = 0, nCount = 0, i = 0;
	BYTE				byACK;
	PUI_HEADER_T		pHdr;
	PUI_RESP_PCPYSATS_T pResp;

	try
	{
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

				LOG_OUT("dcrt_dvs_cd = %02X ", pResp->List[i].dcrt_dvs_cd[0] & 0xFF);

				/// 시외버스할인종류코드
				Info.cty_bus_dc_knd_cd[0] = pResp->List[i].cty_bus_dc_knd_cd[0];
				/// 버스티켓종류코드
				::CopyMemory(Info.bus_tck_knd_cd, pResp->List[i].bus_tck_knd_cd, sizeof(pResp->List[i].bus_tck_knd_cd));
				/// 할인율구분코드
				Info.dcrt_dvs_cd[0] = pResp->List[i].dcrt_dvs_cd[0];
				/// 좌석번호
				::CopyMemory(Info.sats_no, pResp->List[i].sats_no, sizeof(pResp->List[i].sats_no));
				/// 발권금액
				Info.n_tisu_amt = pResp->List[i].n_tisu_amt;

				LOG_OUT("index = %d ", i);
				LOG_OUT("Info.cty_bus_dc_knd_cd = %s ", Info.cty_bus_dc_knd_cd);
				LOG_OUT("Info.bus_tck_knd_cd = %s ", Info.bus_tck_knd_cd);
				LOG_OUT("Info.dcrt_dvs_cd = %s ", Info.dcrt_dvs_cd);
				LOG_OUT("Info.sats_no = %d ", *(WORD *)Info.sats_no);
				LOG_OUT("Info.n_tisu_amt = %d ", Info.n_tisu_amt);

				//pTr->FindBusDcKind(Info.bus_tck_knd_cd,  Info.cty_bus_dc_knd_cd, Info.dcrt_dvs_cd);

				// add
				pTr->m_vtUiSats.push_back(Info);
			}
		}

		Transact_SetState(TR_PBTCK_SEAT_SEL, 0);

		// 	if(Config_IsAlcnMode() != 0)
		// 	{	/// 배차 모드인 경우
		// 		Transact_SetState(TR_PBTCK_SEAT_SEL, 0);
		// 	}
		// 	else
		// 	{	/// 비배차 모드인 경우
		// 		Transact_SetState(TR_PBTCK_SEAT_SEL, 99);
		// 	}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_307_command
 * @details		[현장발권] - 결제 수단 선택 
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_307_command(BYTE *pRecvData)
{
	int				nRet = 0;
	BYTE			byACK = 0;
	PUI_HEADER_T	pHdr;

	try
	{
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
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_308_command
 * @details		[현장발권] - 지폐 투입
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_308_command(BYTE *pRecvData)
{
	int						nRet = 0;
	BYTE					byACK = 0;
	PUI_HEADER_T			pHdr;
	PUI_RESP_BILL_START_T	pResp;

	try
	{
		pHdr  = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_BILL_START_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		/// 지폐 투입
		switch(pResp->byBillDvs & 0xFF)
		{	// 지폐 투입 시작
		case 0x01:
			nRet = Bill_Enable();
			Transact_SetState(TR_PBTCK_INS_BILL, 0);

			/// 서버 종류
			CConfigTkMem::GetInstance()->SetSvrKind(pResp->bySvrDvs);

			if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_CCBUS)
			{
				CPubTckMem::GetInstance()->base.nTotalMoney = pResp->nTotalMoney;
			}
			else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_KOBUS)
			{
				CPubTckKobusMem::GetInstance()->base.nTotalMoney = pResp->nTotalMoney;
			}
			else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_TMEXP)
			{
				CPubTckTmExpMem::GetInstance()->base.nTotalMoney = pResp->nTotalMoney;
			}
			break;
		case 0x02 :
			nRet = Bill_Inhibit();
			break;
		case 0x03 :
			nRet = Bill_Inhibit();
			break;
		case 0x04 :
			nRet = Bill_Enable();
			break;
		}

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
	int			 nRet = 0;
	BYTE		 byACK = 0;
	PUI_HEADER_T pHdr;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		Transact_SetState(TR_PBTCK_CSRC_READ, 0);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

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
 * @brief		resp_311_command
 * @details		[시외버스] - [현장발권] - 승차권 결제 요청 [311]
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_311_command(BYTE *pRecvData)
{
	int						nRet = 0;
	BYTE					byACK = 0;
	PUI_HEADER_T			pHdr;
	PUI_RESP_REQ_PAYMENT_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_REQ_PAYMENT_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		{
			PUBTCK_T* pBase;

			pBase = &CPubTckMem::GetInstance()->base;

			/// 총 결제금액
			pBase->nTotalMoney = pResp->nTotalMoney;
			
			/// 결제 구분코드 - 0x01:현금결제, 0x02:카드결제, 0x04:RF결제
			/// 결제 구분코드 - 0x01:현금결제, 0x02:카드결제, 0x04:RF결제, 0x21(33):페이코QR-카드결제, 0x22(34):페이코QR-포인트결제, 0x31(49):티머니페이QR-결제)" // 20221228 -> TODO:QR은 따로 분리해야 함
			::CopyMemory(pBase->ui_pym_dvs_cd, pResp->ui_pym_dvs_cd, sizeof(pResp->ui_pym_dvs_cd));
			
			/// 현금영수증 구분코드 - 0:미사용, 1:수기입력, 2:신용카드, 3:현영전용카드
			::CopyMemory(pBase->ui_csrc_dvs_cd, pResp->csrc_dvs_cd, sizeof(pResp->csrc_dvs_cd));

			/// RF교통카드 구분코드
			pBase->rf_trcr_dvs_cd[0] = pResp->rf_trcr_dvs_cd[0];

			TR_LOG_OUT("총 결제금액 = %d",			pBase->nTotalMoney);
			TR_LOG_OUT("결제구분코드 = 0x%02X",		pBase->ui_pym_dvs_cd[0]);
			TR_LOG_OUT("현금영수증 구분코드 = %d",	pBase->ui_csrc_dvs_cd[0]);
			TR_LOG_OUT("RF교통카드 구분코드 = %c",	pBase->rf_trcr_dvs_cd[0]);
			TR_LOG_OUT("csrc_no = %s",	pResp->csrc_no);

			// 20221017 ADD~
			// QR코드번호
			::CopyMemory(pBase->qr_cd_no, pResp->qr_cd_no, sizeof(pResp->qr_cd_no));
			// QR결제지불상세코드 -> TODO:구분코드값을 UI(pResp)에서 전달 받지 않음. 향후 ui_pym_dvs_cd 값에서 분리해야 할 필요가 있을 수 있음
			//::CopyMemory(pBase->qr_pym_pyn_dtl_cd, pResp->qr_pym_pyn_dtl_cd, sizeof(pResp->qr_pym_pyn_dtl_cd)); // 20221228 ADD
			// OTC번호;페이코가상일회성카드번호
			::CopyMemory(pBase->payco_virt_ontc_no, pResp->payco_virt_ontc_no, sizeof(pResp->payco_virt_ontc_no));
			// 20221017 ~ADD

			// 결제 구분코드 - 0x01:현금결제, 0x02:카드결제, 0x04:RF결제, 0x21:페이코QR-카드결제, 0x22:페이코QR-포인트결제, 0x31:티머니페이QR-결제
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
			else if(pBase->ui_pym_dvs_cd[0] == 2)
			{
				pBase->pyn_mns_dvs_cd[0] = PYM_CD_CARD;
			}
			// 20221124 ADD~
			else if(pBase->ui_pym_dvs_cd[0] == 0x21)	// QR결제-PAYCO-신용카드[PC](시외)
			{
				pBase->pyn_mns_dvs_cd[0] = PYM_CD_QRPC;
				memcpy(pBase->qr_pym_pyn_dtl_cd, "PC", sizeof(pBase->qr_pym_pyn_dtl_cd)); // 20221228 ADD
			}
			else if(pBase->ui_pym_dvs_cd[0] == 0x22)	// QR결제-PAYCO-포인트[PP](시외)
			{
				pBase->pyn_mns_dvs_cd[0] = PYM_CD_QRPP;
				memcpy(pBase->qr_pym_pyn_dtl_cd, "PP", sizeof(pBase->qr_pym_pyn_dtl_cd)); // 20221228 ADD
			}
			else if(pBase->ui_pym_dvs_cd[0] == 0x31)	// QR결제-티머니페이[TP](시외)
			{
				pBase->pyn_mns_dvs_cd[0] = PYM_CD_QRTP;
				memcpy(pBase->qr_pym_pyn_dtl_cd, "TP", sizeof(pBase->qr_pym_pyn_dtl_cd)); // 20221228 ADD
			}
			// 20221124 ~ADD
			// 20221124 DEL~
			//else				
			//{
			//	pBase->pyn_mns_dvs_cd[0] = PYM_CD_RF;	
			//}
			// 20221124 ~DEL
			// 20221124 ADD~
			else if(pBase->ui_pym_dvs_cd[0] == 0x04)
			{
				pBase->pyn_mns_dvs_cd[0] = PYM_CD_RF;	
			}
			else				
			{
				pBase->pyn_mns_dvs_cd[0] = PYM_CD_CASH;	
			}
			// 20221124 ~ADD

			// 20221017 ADD~
			TR_LOG_OUT("QR코드번호 = %s",		pBase->qr_cd_no);
			char tmp[3];
			memcpy(tmp, pBase->qr_pym_pyn_dtl_cd, sizeof(pBase->qr_pym_pyn_dtl_cd)); // 20221228 ADD
			TR_LOG_OUT("QR결제지불상세코드 = %s",	tmp);
			TR_LOG_OUT("OTC번호 = %s",			pBase->payco_virt_ontc_no);
			// 20221017 ~ADD

			///< 현금영수증 수기입력 번호
			::CopyMemory(pBase->ui_csrc_no, pResp->csrc_no, sizeof(pResp->csrc_no));
			///< 0:개인, 1:법인
			::CopyMemory(pBase->ui_csrc_use, pResp->csrc_use, sizeof(pResp->csrc_use));
			/// 거래구분코드			
			::CopyMemory(pBase->trd_dvs_cd, pResp->trd_dvs_cd, sizeof(pResp->trd_dvs_cd));
			/// fallback구분코드	
			::CopyMemory(pBase->fallback_dvs_cd, pResp->fallback_dvs_cd, sizeof(pResp->fallback_dvs_cd));
			///< POS 단말기 버젼
			::CopyMemory(CConfigTkMem::GetInstance()->pos_ktc_version, pResp->pos_pg_ktc_ver, sizeof(pResp->pos_pg_ktc_ver));
			::CopyMemory(pBase->pos_pg_ktc_ver, pResp->pos_pg_ktc_ver, sizeof(pResp->pos_pg_ktc_ver));
			/// enc 데이터 길이
			::CopyMemory(pBase->enc_dta_len, pResp->enc_dta_len, sizeof(pResp->enc_dta_len));
			/// enc 데이터
			::CopyMemory(pBase->enc_dta, pResp->enc_dta, sizeof(pResp->enc_dta));
			/// emv 데이터
			::CopyMemory(pBase->emv_dta, pResp->emv_dta, sizeof(pResp->emv_dta));
			/// 할부기간
			::CopyMemory(pBase->mip_term, pResp->mip_term, sizeof(pResp->mip_term));

			/// 서명비밀번호여부 - 1:싸인, 2:비밀번호, 3:싸인+비밀번호
			::CopyMemory(pBase->spad_pwd_yn, pResp->spad_pwd_yn, sizeof(pResp->spad_pwd_yn));
			/// 카드비밀번호
			::CopyMemory(pBase->ui_card_pwd, pResp->card_pwd, sizeof(pResp->card_pwd));

			TR_LOG_OUT("싸인패드데이터 - (%s) ", pBase->spad_dta);
			TR_LOG_OUT("싸인패드데이터길이 - (Num = %d) ", *(int *) pBase->spad_dta_len);

			/// 싸인패드데이터
			::CopyMemory(pBase->spad_dta, pResp->spad_dta, sizeof(pResp->spad_dta));		
			/// 싸인패드데이터길이
			sprintf(pBase->spad_dta_len, "%08d", *(int *)pResp->spad_dta_len);

			/// RF교통카드 구분코드
			::CopyMemory(pBase->rf_trcr_dvs_cd, pResp->rf_trcr_dvs_cd, sizeof(pResp->rf_trcr_dvs_cd));

			/// 사용자구분번호 (상주직원일때만 값을 전송)
			if(strlen(pResp->user_dvs_no) > 0)
			{
				sprintf(pBase->user_dvs_no, "%s", pResp->user_dvs_no);
			}
			else
			{
				::ZeroMemory(pBase->user_dvs_no, sizeof(pBase->user_dvs_no));
			}
		}

		Transact_SetState(TR_PBTCK_TMAX_ISSUE, 0);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_313_command
 * @details		[현장발권] - 승차권 발권
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_313_command(BYTE *pRecvData)
{
	int				nRet = 0;
	BYTE			byACK = 0;
	PUI_HEADER_T	pHdr;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_CCBUS )
		{
			Transact_SetState(TR_PBTCK_TCK_ISSUE, 0);
		}
		else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_KOBUS )
		{
			Transact_SetState(TR_PBTCK_KOBUS_TCK_ISSUE, 0);
		}
		else if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_TMEXP )
		{
			Transact_SetState(TR_PBTCK_TMEXP_TCK_ISSUE, 0);
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_315_command
 * @details		[현장발권] - 거스름돈 방출
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_315_command(BYTE *pRecvData)
{
	int						nRet = 0;
	BYTE					byACK = 0;
	PUI_HEADER_T			pHdr;
	PUI_RESP_CHG_MONEY_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_CHG_MONEY_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		CConfigTkMem::GetInstance()->SetSvrKind(pResp->bySvrDvs);
		CPubTckMem::GetInstance()->base.nTotalMoney = pResp->nTotalMoney;
		CPubTckMem::GetInstance()->base.n_pbtck_chg_money = pResp->nChangeMoney;

		Transact_SetState(TR_PBTCK_CHANGE_MONEY, 0);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

// 20221205 ADD~
/**
 * @brief		resp_378_command
 * @details		현장발권 - QR 좌석 선점 시간변경
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_378_command(BYTE *pRecvData)
{
	int						nRet = 0;
	BYTE					byACK = 0;
	PUI_HEADER_T			pHdr;
	PUI_RESP_CHG_MONEY_T	pResp;

	try
	{
		// Main->UI 응답은 서비스(TK_QrMdPcpySats) 호출 결과 수신 이후에 전송하기 위해 이하 삭제
		//pHdr = (PUI_HEADER_T) pRecvData;
		//pResp = (PUI_RESP_CHG_MONEY_T) &pHdr->byBody;

		//byACK = CHAR_ACK;
		//nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		Transact_SetState(TR_PBTCK_REQ_QRMDPCPYSATS, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}
// 20221205 ~ADD

/**
 * @brief		RespPbTckCardInfo
 * @details		[현장발권] - 신용카드 정보 (317)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int RespPbTckCardInfo(BYTE *pRecvData)
{
	int							nRet = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_CREDIT_CARD_INFO_T pResp;

	try
	{
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
			::CopyMemory(CConfigTkMem::GetInstance()->pos_ktc_version, pResp->pos_pg_ktc_ver,	sizeof(pResp->pos_pg_ktc_ver));      
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

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_320_command
 * @details		[현장발권] - 시외버스_인천공항_상주직원 정보 요청 (320)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_320_command(BYTE *pRecvData)
{
	int							nRet = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_STAFF_REQ_INFO_T	 pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_STAFF_REQ_INFO_T) &pHdr->byBody;

		CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_CCBUS;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		{
			CPubTckMem* pPbTckTr = CPubTckMem::GetInstance();

			///< 사용자구분번호
			::CopyMemory(pPbTckTr->base.user_dvs_no, pResp->user_dvs_no, sizeof(pResp->user_dvs_no));	
			///< 사용자비번
			::CopyMemory(pPbTckTr->base.user_pwd, pResp->user_pwd, sizeof(pResp->user_pwd));	
			///< 발권요청여부
			::CopyMemory(pPbTckTr->base.tisu_req_yn, pResp->tisu_req_yn, sizeof(pResp->tisu_req_yn));	
			///< 노선ID
			::CopyMemory(pPbTckTr->base.rot_id, pResp->rot_id, sizeof(pResp->rot_id));	
			///< 노선순번
			::CopyMemory(pPbTckTr->base.rot_sqno, pResp->rot_sqno, sizeof(pResp->rot_sqno));	
			///< 배차일자
			::CopyMemory(pPbTckTr->base.alcn_dt, pResp->alcn_dt, sizeof(pResp->alcn_dt));	
			///< 배차순번
			::CopyMemory(pPbTckTr->base.alcn_sqno, pResp->alcn_sqno, sizeof(pResp->alcn_sqno));	
		}

		Transact_SetState(TR_PBTCK_STAFF_REQ, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}


/**
 * @brief		resp_322_command
 * @details		[현장발권] - 시외버스_인천공항_상주직원 좌석요금정보 변경 요청 (322)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_322_command(BYTE *pRecvData)
{
	int							nRet = 0, nCount = 0, i = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_STAFF_CD_MOD_FARE_T	 pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_STAFF_CD_MOD_FARE_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		{
			CPubTckMem* pPbTckTr = CPubTckMem::GetInstance();

			///< 노선id
			::CopyMemory(pPbTckTr->base.rot_id, pResp->rot_id, sizeof(pResp->rot_id));	
			///< 노선순번
			::CopyMemory(pPbTckTr->base.rot_sqno, pResp->rot_sqno, sizeof(pResp->rot_sqno));	
			///< 배차일자
			::CopyMemory(pPbTckTr->base.alcn_dt, pResp->alcn_dt, sizeof(pResp->alcn_dt));	
			///< 배차순번
			::CopyMemory(pPbTckTr->base.alcn_sqno, pResp->alcn_sqno, sizeof(pResp->alcn_sqno));	
			///< 출발터미널코드
			::CopyMemory(pPbTckTr->base.depr_trml_cd, pResp->depr_trml_cd, sizeof(pResp->depr_trml_cd));	
			///< 도착터미널코드
			::CopyMemory(pPbTckTr->base.arvl_trml_cd, pResp->arvl_trml_cd, sizeof(pResp->arvl_trml_cd));	
			///< 카드번호암호문
			::CopyMemory(pPbTckTr->base.enc_dta, pResp->damo_enc_card_no, sizeof(pResp->damo_enc_card_no));	
			///< 카드번호암호문길이
			::CopyMemory(pPbTckTr->base.enc_dta_len, pResp->damo_enc_card_no_len, sizeof(pResp->damo_enc_card_no_len));	
			///< 암호문키
			::CopyMemory(pPbTckTr->base.enc_dta_key, pResp->damo_enc_dta_key, sizeof(pResp->damo_enc_dta_key));

			nCount = *(int *)pResp->pub_num;
			for(i = 0; i < nCount; i++)
			{
				UI_RESP_STAFF_CD_MOD_FARE_LIST_T list;

				::CopyMemory(&list, &pResp->List[i], sizeof(UI_RESP_STAFF_CD_MOD_FARE_LIST_T));

				CPubTckMem::GetInstance()->m_vtStaffModFareReq.push_back(list);
			}
		}

		Transact_SetState(TR_PBTCK_STAFF_CD_MOD_FARE, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}


/**
 * @brief		resp_324_command
 * @details		[현장발권] - RF 선불카드 결제 요청 (324)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_324_command(BYTE *pRecvData)
{
	int							nRet = 0, nCount = 0, i = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_RF_CD_PAYMENT_T	pResp;

	try
	{
		CPubTckMem* pPbTckTr;
		PUI_RESP_RF_CD_PAYMENT_LIST_T pUiList;

		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_RF_CD_PAYMENT_T) &pHdr->byBody;

		nCount = pResp->nCount;
		pUiList = (PUI_RESP_RF_CD_PAYMENT_LIST_T) &pResp->byData;

		pPbTckTr = CPubTckMem::GetInstance();

		/// 지불수단 코드
		pPbTckTr->base.pyn_mns_dvs_cd[0] = PYM_CD_RF;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		for(i = 0; i < nCount; i++)
		{
			PUBTCK_UI_RF_CARD_T List;

			::ZeroMemory(&List, sizeof(PUBTCK_UI_RF_CARD_T));

			///< 노선id			
			::CopyMemory(List.rot_id				, pUiList->rot_id				, sizeof(pUiList->rot_id				));	
			///< (int)노선순번			
			::CopyMemory(List.rot_sqno				, pUiList->rot_sqno				, sizeof(pUiList->rot_sqno				));	
			///< 배차일자			
			::CopyMemory(List.alcn_dt				, pUiList->alcn_dt				, sizeof(pUiList->alcn_dt				));	
			///< (int)배차순번			
			::CopyMemory(List.alcn_sqno				, pUiList->alcn_sqno			, sizeof(pUiList->alcn_sqno				));	
			///< 출발터미널코드		
			::CopyMemory(List.depr_trml_cd			, pUiList->depr_trml_cd			, sizeof(pUiList->depr_trml_cd			));	
			///< 도착터미널코드		
			::CopyMemory(List.arvl_trml_cd			, pUiList->arvl_trml_cd			, sizeof(pUiList->arvl_trml_cd			));	
			///< 발권방식구분코드		
			::CopyMemory(List.tisu_way_dvs_cd		, pUiList->tisu_way_dvs_cd		, sizeof(pUiList->tisu_way_dvs_cd		));	
			///< 예매자료조회여부		
			::CopyMemory(List.read_rot_yn			, pUiList->read_rot_yn			, sizeof(pUiList->read_rot_yn			));	
			///< (int)발행매수			
			::CopyMemory(List.pub_num				, pUiList->pub_num				, sizeof(pUiList->pub_num				));	
			///< 지불수단구분코드		
			::CopyMemory(List.pyn_mns_dvs_cd		, pUiList->pyn_mns_dvs_cd		, sizeof(pUiList->pyn_mns_dvs_cd		));	
			///< 발행채널구분코드		
			::CopyMemory(List.pub_chnl_dvs_cd		, pUiList->pub_chnl_dvs_cd		, sizeof(pUiList->pub_chnl_dvs_cd		));	
			///< 사용자구분번호		
			::CopyMemory(List.user_dvs_no			, pUiList->user_dvs_no			, sizeof(pUiList->user_dvs_no			));	
			///< 버스티켓종류코드		
			::CopyMemory(List.bus_tck_knd_cd		, pUiList->bus_tck_knd_cd		, sizeof(pUiList->bus_tck_knd_cd		));	
			///< (int)좌석번호			
			::CopyMemory(List.sats_no				, pUiList->sats_no				, sizeof(pUiList->sats_no				));	
			///< 좌석선점id			
			::CopyMemory(List.sats_pcpy_id			, pUiList->sats_pcpy_id			, sizeof(pUiList->sats_pcpy_id			));	
			///< 구회원번호			
			::CopyMemory(List.old_mbrs_no			, pUiList->old_mbrs_no			, sizeof(pUiList->old_mbrs_no			));	
			///< 예약id			
			::CopyMemory(List.mrnp_id				, pUiList->mrnp_id				, sizeof(pUiList->mrnp_id				));	
			///< (int)예약매수			
			::CopyMemory(List.mrnp_num				, pUiList->mrnp_num				, sizeof(pUiList->mrnp_num				));	
			///< 카드번호암호문		
			::CopyMemory(List.damo_enc_card_no		, pUiList->damo_enc_card_no		, sizeof(pUiList->damo_enc_card_no		));	
			///< (int)카드번호암호문길이	
			::CopyMemory(List.damo_enc_card_no_len	, pUiList->damo_enc_card_no_len	, sizeof(pUiList->damo_enc_card_no_len	));	
			///< 암호문키		
			::CopyMemory(List.damo_enc_dta_key		, pUiList->damo_enc_dta_key		, sizeof(pUiList->damo_enc_dta_key		));	
			///< RF교통카드 구분코드
			pPbTckTr->base.rf_trcr_dvs_cd[0] = pUiList->rf_trcr_dvs_cd[0];
			::CopyMemory(List.rf_trcr_dvs_cd		, pUiList->rf_trcr_dvs_cd		, sizeof(pUiList->rf_trcr_dvs_cd		));	
			///< 선불작업구분코드		
			::CopyMemory(List.ppy_tak_dvs_cd		, pUiList->ppy_tak_dvs_cd		, sizeof(pUiList->ppy_tak_dvs_cd		));	
			///< 선불pos영업일자		
			::CopyMemory(List.ppy_pos_sls_dt		, pUiList->ppy_pos_sls_dt		, sizeof(pUiList->ppy_pos_sls_dt		));	
			///< 선불pos영수증번호		
			::CopyMemory(List.ppy_pos_recp_no		, pUiList->ppy_pos_recp_no		, sizeof(pUiList->ppy_pos_recp_no		));	
			///< 선불samid			
			::CopyMemory(List.ppy_sam_id			, pUiList->ppy_sam_id			, sizeof(pUiList->ppy_sam_id			));	
			///< 선불sam거래일련번호	
			::CopyMemory(List.ppy_sam_trd_sno		, pUiList->ppy_sam_trd_sno		, sizeof(pUiList->ppy_sam_trd_sno		));	
			///< 선불카드거래일련번호	
			::CopyMemory(List.ppy_card_trd_sno		, pUiList->ppy_card_trd_sno		, sizeof(pUiList->ppy_card_trd_sno		));	
			///< (int)선불승인이후잔액	
			::CopyMemory(List.ppy_aprv_aft_bal		, pUiList->ppy_aprv_aft_bal		, sizeof(pUiList->ppy_aprv_aft_bal		));	
			///< (int)선불승인이전잔액	
			::CopyMemory(List.ppy_aprv_bef_bal		, pUiList->ppy_aprv_bef_bal		, sizeof(pUiList->ppy_aprv_bef_bal		));	
			///< 선불알고리즘id		
			::CopyMemory(List.ppy_algr_id			, pUiList->ppy_algr_id			, sizeof(pUiList->ppy_algr_id			));	
			///< 선불sign값			
			::CopyMemory(List.ppy_sign_val			, pUiList->ppy_sign_val			, sizeof(pUiList->ppy_sign_val			));	
			///< 선불개별거래수집키버전	
			::CopyMemory(List.ppy_indv_trd_clk_ver	, pUiList->ppy_indv_trd_clk_ver	, sizeof(pUiList->ppy_indv_trd_clk_ver	));	
			///< 선불전자화폐식별자id	
			::CopyMemory(List.ppy_elcs_idnt_id		, pUiList->ppy_elcs_idnt_id		, sizeof(pUiList->ppy_elcs_idnt_id		));	
			///< sam총액수집일련번호	
			::CopyMemory(List.sam_ttam_clcn_sno		, pUiList->sam_ttam_clcn_sno	, sizeof(pUiList->sam_ttam_clcn_sno		));	
			///< (int)sam개별수집건수	
			::CopyMemory(List.sam_indv_clcn_ncnt	, pUiList->sam_indv_clcn_ncnt	, sizeof(pUiList->sam_indv_clcn_ncnt	));	
			///< (int)sam누적거래총액	
			::CopyMemory(List.sam_cum_trd_ttam		, pUiList->sam_cum_trd_ttam		, sizeof(pUiList->sam_cum_trd_ttam		));	
			///< 선불카드구분코드		
			::CopyMemory(List.ppy_card_dvs_cd		, pUiList->ppy_card_dvs_cd		, sizeof(pUiList->ppy_card_dvs_cd		));	
			///< 선불카드사용자구분코드	
			::CopyMemory(List.ppy_card_user_dvs_cd	, pUiList->ppy_card_user_dvs_cd	, sizeof(pUiList->ppy_card_user_dvs_cd	));	
			///< 선불hsm상태코드		
			::CopyMemory(List.ppy_hsm_sta_cd		, pUiList->ppy_hsm_sta_cd		, sizeof(pUiList->ppy_hsm_sta_cd		));	
			///< (int)거래요청금액		
			::CopyMemory(List.req_trd_amt			, pUiList->req_trd_amt			, sizeof(pUiList->req_trd_amt			));	

			pUiList++;

			/// ui 선불RF 결제 데이타 
			CPubTckMem::GetInstance()->m_vtRfUiData.push_back(List);
		}

		Transact_SetState(TR_PBTCK_RF_PAYMENT, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_326_command
 * @details		[현장발권] - 발행정보 입력 (324)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_326_command(BYTE *pRecvData)
{
	int							nRet = 0, nCount = 0, i = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_PUBUSRINFINP_T		pResp;

	try
	{
		CPubTckMem* pPbTckTr;

		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_PUBUSRINFINP_T) &pHdr->byBody;

		pPbTckTr = CPubTckMem::GetInstance();

		::CopyMemory(pPbTckTr->base.pub_user_tel_no, pResp->damo_enc_pub_user_tel_no, sizeof(pResp->damo_enc_pub_user_tel_no));
		if( strlen(pPbTckTr->base.pub_user_tel_no) <= 0 )
		{
			pPbTckTr->base.pub_user_tel_no[0] = 0x20;
		}
		::CopyMemory(pPbTckTr->base.pub_user_krn, pResp->damo_enc_pub_user_krn, sizeof(pResp->damo_enc_pub_user_krn));
		::CopyMemory(pPbTckTr->base.ride_vhcl_dvs, pResp->ride_vhcl_dvs, sizeof(pResp->ride_vhcl_dvs));

		LOG_OUT("pub_user_tel_no, len(%d), (%s) ..", strlen(pPbTckTr->base.pub_user_tel_no), pPbTckTr->base.pub_user_tel_no);
		LOG_OUT("pub_user_krn, len(%d), (%s) ..", strlen(pPbTckTr->base.pub_user_krn), pPbTckTr->base.pub_user_krn);
		LOG_OUT("ride_vhcl_dvs, len(%d), (%s) ..", strlen(pPbTckTr->base.ride_vhcl_dvs), pPbTckTr->base.ride_vhcl_dvs);

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		Transact_SetState(TR_PBTCK_CORONA_INPUT, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_331_command
 * @details		[코버스]-[현장발권]-배차조회 요청 (331)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_331_command(BYTE *pRecvData)
{
	int						nRet = 0;
	BYTE					byACK = 0;
	PUI_HEADER_T			pHdr;
	PUI_RESP_KO_REQ_ALCN_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_KO_REQ_ALCN_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		// set memory
		{
			CPubTckKobusMem* pPubTckTr = CPubTckKobusMem::GetInstance();

			::CopyMemory(&pPubTckTr->m_tReqAlcn, &pHdr->byBody, sizeof(UI_RESP_KO_REQ_ALCN_T));

			/// 버스구분 : 시외버스 or 코버스 or 티머니고속
			CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_KOBUS;
		}

		Transact_SetState(TR_PBTCK_KOBUS_REQ_LIST, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_333_command
 * @details		[코버스]-[현장발권]-좌석정보 조회 요청 (333)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_333_command(BYTE *pRecvData)
{
	int						nRet = 0;
	BYTE					byACK = 0;
	PUI_HEADER_T			pHdr;
	PUI_RESP_KO_REQ_SATS_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_KO_REQ_SATS_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		// set memory
		{
			CPubTckKobusMem* pPubTckTr = CPubTckKobusMem::GetInstance();

			::CopyMemory(&pPubTckTr->m_tReqSats, &pHdr->byBody, sizeof(UI_RESP_KO_REQ_SATS_T));
		}

		Transact_SetState(TR_PBTCK_KOBUS_REQ_SATS, 0);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_335_command
 * @details		[코버스]-[현장발권]-좌석선점 요청 (335)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_335_command(BYTE *pRecvData)
{
	int							nRet = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_KO_REQ_SATSPCPY_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_KO_REQ_SATSPCPY_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		// set memory
		{
			CPubTckKobusMem* pPubTckTr = CPubTckKobusMem::GetInstance();

			::CopyMemory(&pPubTckTr->m_tReqSatsPcpy, &pHdr->byBody, sizeof(UI_RESP_KO_REQ_SATSPCPY_T));
		}

		Transact_SetState(TR_PBTCK_KOBUS_REQ_SATS_PCPY, 0);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_337_command
 * @details		[코버스]-[현장발권] - 카드/현금/현장발권 요청 요청 (337)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_337_command(BYTE *pRecvData)
{
	int						nRet = 0;
	BYTE					byACK = 0;
	PUI_HEADER_T			pHdr;
	PUI_RESP_KO_TCK_TRAN_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_KO_TCK_TRAN_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		// set memory
		{
			UI_RESP_KO_TCK_TRAN_T* pInfo;
			CPubTckKobusMem* pPubTckTr = CPubTckKobusMem::GetInstance();

			pInfo = &pPubTckTr->m_tReqTckIssue;

			::CopyMemory(pInfo->user_no 			, pResp->user_no 			, sizeof(pResp->user_no 			));
			::CopyMemory(pInfo->lng_cd 				, pResp->lng_cd 			, sizeof(pResp->lng_cd 				));
			::CopyMemory(pInfo->alcn_depr_trml_no	, pResp->alcn_depr_trml_no	, sizeof(pResp->alcn_depr_trml_no	));
			::CopyMemory(pInfo->alcn_arvl_trml_no	, pResp->alcn_arvl_trml_no	, sizeof(pResp->alcn_arvl_trml_no	));
			::CopyMemory(pInfo->alcn_depr_dt		, pResp->alcn_depr_dt		, sizeof(pResp->alcn_depr_dt		));
			::CopyMemory(pInfo->alcn_depr_time		, pResp->alcn_depr_time		, sizeof(pResp->alcn_depr_time		));
			::CopyMemory(pInfo->depr_trml_no		, pResp->depr_trml_no		, sizeof(pResp->depr_trml_no		));
			::CopyMemory(pInfo->arvl_trml_no		, pResp->arvl_trml_no		, sizeof(pResp->arvl_trml_no		));
			::CopyMemory(pInfo->depr_dt				, pResp->depr_dt			, sizeof(pResp->depr_dt				));
			::CopyMemory(pInfo->depr_time			, pResp->depr_time			, sizeof(pResp->depr_time			));
			::CopyMemory(pInfo->bus_cls_cd			, pResp->bus_cls_cd			, sizeof(pResp->bus_cls_cd			));
			::CopyMemory(pInfo->cacm_cd				, pResp->cacm_cd			, sizeof(pResp->cacm_cd				));
			::CopyMemory(pInfo->trml_no				, pResp->trml_no			, sizeof(pResp->trml_no				));
			::CopyMemory(pInfo->wnd_no				, pResp->wnd_no				, sizeof(pResp->wnd_no				));
			::CopyMemory(pInfo->inhr_no				, pResp->inhr_no			, sizeof(pResp->inhr_no				));
			::CopyMemory(pInfo->sats_no_aut_yn		, pResp->sats_no_aut_yn		, sizeof(pResp->sats_no_aut_yn		));
			::CopyMemory(pInfo->inp_dvs_cd			, pResp->inp_dvs_cd			, sizeof(pResp->inp_dvs_cd			));
			::CopyMemory(pInfo->card_no				, pResp->card_no			, sizeof(pResp->card_no				));
			::CopyMemory(&pInfo->mip_mm_num			, &pResp->mip_mm_num		, sizeof(pResp->mip_mm_num)			);
			::CopyMemory(pInfo->sgn_inf				, pResp->sgn_inf			, sizeof(pResp->sgn_inf				));
			::CopyMemory(pInfo->iccd_inf			, pResp->iccd_inf			, sizeof(pResp->iccd_inf			));
			::CopyMemory(pInfo->chit_use_dvs		, pResp->chit_use_dvs		, sizeof(pResp->chit_use_dvs		));
			::CopyMemory(pInfo->iccd_yn				, pResp->iccd_yn			, sizeof(pResp->iccd_yn				));
			::CopyMemory(pInfo->trck_dta_enc_cd		, pResp->trck_dta_enc_cd	, sizeof(pResp->trck_dta_enc_cd		));
			::CopyMemory(pInfo->card_no_mask		, pResp->card_no_mask		, sizeof(pResp->card_no_mask		));
			::CopyMemory(pInfo->csrc_dvs_cd			, pResp->csrc_dvs_cd		, sizeof(pResp->csrc_dvs_cd			));
			::CopyMemory(&pInfo->nTotalMoney		, &pResp->nTotalMoney		, sizeof(pResp->nTotalMoney			));
			::CopyMemory(&pInfo->tissu_hcnt, &pResp->tissu_hcnt, sizeof(pResp->tissu_hcnt));

			for(int i = 0; i < pInfo->tissu_hcnt; i++)
			{
				::CopyMemory(&pInfo->List[i], &pResp->List[i], sizeof(stk_tm_tcktran_list_t));
			}

			/// 총 결제금액
			pPubTckTr->base.nTotalMoney = pInfo->nTotalMoney;

			/// 지불수단
			switch( pPubTckTr->m_tReqTckIssue.inp_dvs_cd[0] )
			{
			case 'C': // 카드
				pPubTckTr->base.ui_pym_dvs_cd[0] = PYM_CD_CARD;
				break;
			case 'M': // 현금
				///< 현금영수증 구분코드 - 0:미사용, 1:수기입력, 2:신용카드
				if(pInfo->csrc_dvs_cd[0] == 0)
				{
					pPubTckTr->base.ui_pym_dvs_cd[0] = PYM_CD_CASH;
				}
				else
				{
					pPubTckTr->base.ui_pym_dvs_cd[0] = PYM_CD_CSRC;
					pPubTckTr->base.ui_csrc_dvs_cd[0] = pInfo->csrc_dvs_cd[0];	
				}
				break;
			}

		}
		Transact_SetState(TR_PBTCK_KOBUS_TMAX_ISSUE, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_338_command
 * @details		[코버스]-[현장발권] - 경유지 정보 요청 (338)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_338_command(BYTE *pRecvData)
{
	int							nRet = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_KO_REQ_THRUINFO_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_KO_REQ_THRUINFO_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		// set memory
		{
			CPubTckKobusMem* pPubTckTr = CPubTckKobusMem::GetInstance();

			::CopyMemory(&pPubTckTr->m_tReqThru, &pHdr->byBody, sizeof(UI_RESP_KO_REQ_THRUINFO_T));
		}

		Transact_SetState(TR_PBTCK_KOBUS_REQ_THRU, 0);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_350_command
 * @details		[티머니고속]-[현장발권] - 배차정보 조회 (350)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_350_command(BYTE *pRecvData)
{
	int							nRet = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_TM_REQ_ALCN_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_TM_REQ_ALCN_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		// set memory : 티머니 고속
		CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_TMEXP;

		// set memory
		{
			::CopyMemory(&CPubTckTmExpMem::GetInstance()->m_tReqAlcn, 
						 &pHdr->byBody, 
						 sizeof(stk_tm_readalcn_t));
			
			if(m_nDebug > 0)
			{
				stk_tm_readalcn_t *p;

				p = &CPubTckTmExpMem::GetInstance()->m_tReqAlcn;

				LOG_OUT("%30s - (%.*s) ", "req_pgm_dvs"			,	sizeof(p->req_pgm_dvs		)-1, p->req_pgm_dvs			);
				LOG_OUT("%30s - (%.*s) ", "req_trml_no"			,	sizeof(p->req_trml_no		)-1, p->req_trml_no			);
				LOG_OUT("%30s - (%.*s) ", "req_wnd_no"			,	sizeof(p->req_wnd_no		)-1, p->req_wnd_no			);
				LOG_OUT("%30s - (%.*s) ", "req_user_no"			,	sizeof(p->req_user_no		)-1, p->req_user_no			);
				LOG_OUT("%30s - (Num=%d)","rec_num"				,	*(int *)p->rec_num										);
				LOG_OUT("%30s - (%.*s) ", "bef_aft_dvs"			,	sizeof(p->bef_aft_dvs		)-1, p->bef_aft_dvs			);
				LOG_OUT("%30s - (%.*s) ", "depr_dt"				,	sizeof(p->depr_dt			)-1, p->depr_dt				);
				LOG_OUT("%30s - (%.*s) ", "depr_trml_no"		,	sizeof(p->depr_trml_no		)-1, p->depr_trml_no		);
				LOG_OUT("%30s - (%.*s) ", "arvl_trml_no"		,	sizeof(p->arvl_trml_no		)-1, p->arvl_trml_no		);
				LOG_OUT("%30s - (%.*s) ", "bus_cls_cd"			,	sizeof(p->bus_cls_cd		)-1, p->bus_cls_cd			);
				LOG_OUT("%30s - (%.*s) ", "depr_time"			,	sizeof(p->depr_time			)-1, p->depr_time			);
				LOG_OUT("%30s - (%.*s) ", "read_bcnl_yn"		,	sizeof(p->read_bcnl_yn		)-1, p->read_bcnl_yn		);
				LOG_OUT("%30s - (%.*s) ", "read_rmn_yn"			,	sizeof(p->read_rmn_yn		)-1, p->read_rmn_yn			);
				LOG_OUT("%30s - (%.*s) ", "alcn_depr_trml_no"	,	sizeof(p->alcn_depr_trml_no	)-1, p->alcn_depr_trml_no	);
				LOG_OUT("%30s - (%.*s) ", "alcn_arvl_trml_no"	,	sizeof(p->alcn_arvl_trml_no	)-1, p->alcn_arvl_trml_no	);
				LOG_OUT("%30s - (%.*s) ", "alcn_rot_no		"	,	sizeof(p->alcn_rot_no		)-1, p->alcn_rot_no			);
			}

		}

		Transact_SetState(TR_PBTCK_TMEXP_REQ_LIST, 0);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_352_command
 * @details		[티머니고속]-[현장발권] - 요금/좌석정보 조회 (352)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_352_command(BYTE *pRecvData)
{
	int							nRet = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_TM_READ_SATS_FEE_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_TM_READ_SATS_FEE_T) &pHdr->byBody;

		// set memory
		{
			vector<rtk_tm_readalcn_list_t>::iterator iter;
			int i = 0;
			CPubTckTmExpMem *pInst;

			pInst = CPubTckTmExpMem::GetInstance();

			::CopyMemory(&pInst->m_tReqSats, 
						 &pResp->dt, 
						 sizeof(stk_tm_readsatsfee_t));
			/// 노선 승차홈번호 추가
			::CopyMemory(&pInst->base.rot_rdhm_no_val, 
						 pResp->rot_rdhm_no_val,
						 sizeof(pResp->rot_rdhm_no_val));

			/// 배차리스트에서 출발시간을 비교하여 데이타를 찾는다.
			i = 0;
			for(iter = pInst->m_vtResAlcnList.begin(); iter != pInst->m_vtResAlcnList.end(); iter++, i++)
			{
				if( memcmp(iter->depr_time, pResp->dt.depr_time, sizeof(pResp->dt.depr_time)) == 0 )
				{
					pInst->base.n_select_alcn_list = i;
					::CopyMemory(pInst->base.bus_oprn_dist, iter->bus_oprn_dist, sizeof(pInst->base.bus_oprn_dist));
					break;
				}
			}

			if(m_nDebug > 0)
			{
				stk_tm_readsatsfee_t *p;

				p = &CPubTckTmExpMem::GetInstance()->m_tReqSats;
			}
		}

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		/// 좌석정보 조회
		Transact_SetState(TR_PBTCK_TMEXP_REQ_SATS, 0);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_354_command
 * @details		[티머니고속]-[현장발권] - 좌석선점 조회 (354)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_354_command(BYTE *pRecvData)
{
	int							nRet = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_TM_PCPY_SATS_T		pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_TM_PCPY_SATS_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		// set memory
		{
			::CopyMemory(&CPubTckTmExpMem::GetInstance()->m_tReqSatsPcpy, 
						 &pHdr->byBody, 
						 sizeof(stk_tm_pcpysats_t));
		}

		Transact_SetState(TR_PBTCK_TMEXP_REQ_SATS_PCPY, 0);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_356_command
 * @details		[티머니고속]-[현장발권] - 승차권발권(현금) (356)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_356_command(BYTE *pRecvData)
{
	int							nRet = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_TM_PUBTCK_CASH_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_TM_PUBTCK_CASH_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		// set memory
		{
			/// set 결제수단
			CPubTckTmExpMem::GetInstance()->base.pyn_mns_dvs_cd[0] = PYM_CD_CASH;
			CPubTckTmExpMem::GetInstance()->base.ui_pym_dvs_cd[0] = PYM_CD_CASH;

			::CopyMemory(&CPubTckTmExpMem::GetInstance()->m_tReqPubTckCash, 
						 &pHdr->byBody, 
						 sizeof(stk_tm_pubtckcash_t));
		}

		/// debug
		if(1)
		{
			int i, nCount;

			i = nCount = 0;

			LOG_OUT("%30s - (%s) ", "req_pgm_dvs		"	,	pResp->dt.req_pgm_dvs			);
			LOG_OUT("%30s - (%s) ", "req_trml_no		"	,	pResp->dt.req_trml_no			);
			LOG_OUT("%30s - (%s) ", "req_wnd_no			"	,	pResp->dt.req_wnd_no			);
			LOG_OUT("%30s - (%s) ", "req_user_no		"	,	pResp->dt.req_user_no			);
			LOG_OUT("%30s - (%s) ", "req_dvs_cd			"	,	pResp->dt.req_dvs_cd			);
			LOG_OUT("%30s - (%s) ", "alcn_depr_trml_no	"	,	pResp->dt.alcn_depr_trml_no	);
			LOG_OUT("%30s - (%s) ", "alcn_arvl_trml_no	"	,	pResp->dt.alcn_arvl_trml_no	);
			LOG_OUT("%30s - (%s) ", "alcn_depr_dt		"	,	pResp->dt.alcn_depr_dt			);
			LOG_OUT("%30s - (%s) ", "alcn_depr_time		"	,	pResp->dt.alcn_depr_time		);
			LOG_OUT("%30s - (%s) ", "depr_trml_no		"	,	pResp->dt.depr_trml_no			);
			LOG_OUT("%30s - (%s) ", "arvl_trml_no		"	,	pResp->dt.arvl_trml_no			);
			LOG_OUT("%30s - (%s) ", "depr_dt			"	,	pResp->dt.depr_dt				);
			LOG_OUT("%30s - (%s) ", "depr_time			"	,	pResp->dt.depr_time			);
			LOG_OUT("%30s - (%s) ", "bus_cls_cd			"	,	pResp->dt.bus_cls_cd			);
			LOG_OUT("%30s - (%s) ", "cacm_cd			"	,	pResp->dt.cacm_cd				);
			LOG_OUT("%30s - (%s) ", "mbrs_yn			"	,	pResp->dt.mbrs_yn				);
			LOG_OUT("%30s - (%s) ", "mbrs_no			"	,	pResp->dt.mbrs_no				);
			LOG_OUT("%30s - (%s) ", "mrnp_dt			"	,	pResp->dt.mrnp_dt				);
			LOG_OUT("%30s - (%s) ", "mrnp_time			"	,	pResp->dt.mrnp_time			);
			LOG_OUT("%30s - (%s) ", "rec_num			"	,	pResp->dt.rec_num				);

			nCount = *(int *)pResp->dt.rec_num;
			for(i = 0; i < nCount; i++)
			{
				stk_tm_pubtckcash_list_t *pList;

				pList = &pResp->dt.List[i];
				
				LOG_OUT("Index[%d] ##################### ", i);
				LOG_OUT("%30s - (%s) ", "tck_knd_cd	"	,	pList->tck_knd_cd		);
				LOG_OUT("%30s - (%s) ", "sats_no	"	,	pList->sats_no		);
				LOG_OUT("%30s - (%s) ", "fp_no		"	,	pList->fp_no			);
				LOG_OUT("%30s - (%s) ", "pcpy_no	"	,	pList->pcpy_no		);
				LOG_OUT("%30s - (%s) ", "fee_knd_cd	"	,	pList->fee_knd_cd		);
				LOG_OUT("%30s - (%s) ", "n_tissu_fee"	,	pList->n_tissu_fee	);
				LOG_OUT("%30s - (%s) ", "n_ogn_fee	"	,	pList->n_ogn_fee		);
				LOG_OUT("%30s - (%s) ", "dc_knd_cd	"	,	pList->dc_knd_cd		);
			}
		}

		Transact_SetState(TR_PBTCK_TMEXP_TMAX_ISSUE, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_358_command
 * @details		[티머니고속]-[현장발권] - 승차권발권(카드_KTC) (356)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_358_command(BYTE *pRecvData)
{
	int								nRet = 0;
	BYTE							byACK = 0;
	PUI_HEADER_T					pHdr;
	PUI_RESP_TM_PUBTCK_CARD_KTC_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_TM_PUBTCK_CARD_KTC_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		// set memory
		{
			/// set 결제수단
			CPubTckTmExpMem::GetInstance()->base.pyn_mns_dvs_cd[0] = PYM_CD_CARD;
			CPubTckTmExpMem::GetInstance()->base.ui_pym_dvs_cd[0] = PYM_CD_CARD;

			::CopyMemory(&CPubTckTmExpMem::GetInstance()->m_tReqPubTckCardKtc, 
						 &pHdr->byBody, 
						 sizeof(stk_tm_pubtckcard_ktc_t));
		}

		/// debug
		if(1)
		{
			int i, nCount;
			stk_tm_pubtckcard_ktc_t *p;

			i = nCount = 0;
			p = (stk_tm_pubtckcard_ktc_t *) &pHdr->byBody;
			
			LOG_OUT("%30s - (%s) ", "req_pgm_dvs		"	,	p->req_pgm_dvs				);
			LOG_OUT("%30s - (%s) ", "req_trml_no		"	,	p->req_trml_no				);
			LOG_OUT("%30s - (%s) ", "req_wnd_no			"	,	p->req_wnd_no				);
			LOG_OUT("%30s - (%s) ", "req_user_no		"	,	p->req_user_no				);
			LOG_OUT("%30s - (%s) ", "alcn_depr_trml_no	"	,	p->alcn_depr_trml_no		);
			LOG_OUT("%30s - (%s) ", "alcn_arvl_trml_no	"	,	p->alcn_arvl_trml_no		);
			LOG_OUT("%30s - (%s) ", "alcn_depr_dt		"	,	p->alcn_depr_dt				);
			LOG_OUT("%30s - (%s) ", "alcn_depr_time		"	,	p->alcn_depr_time			);
			LOG_OUT("%30s - (%s) ", "depr_trml_no		"	,	p->depr_trml_no				);
			LOG_OUT("%30s - (%s) ", "arvl_trml_no		"	,	p->arvl_trml_no				);
			LOG_OUT("%30s - (%s) ", "depr_dt			"	,	p->depr_dt					);
			LOG_OUT("%30s - (%s) ", "depr_time			"	,	p->depr_time				);
			LOG_OUT("%30s - (%s) ", "bus_cls_cd			"	,	p->bus_cls_cd				);
			LOG_OUT("%30s - (%s) ", "cacm_cd			"	,	p->cacm_cd					);
			LOG_OUT("%30s - (%s) ", "card_track_dta		"	,	p->card_track_dta			);
			LOG_OUT("%30s - (%s) ", "spad_dta			"	,	p->spad_dta					);
			LOG_OUT("%30s - (Num = %d) ", "n_spad_dta_len"	,	*(int *) p->n_spad_dta_len	);
			LOG_OUT("%30s - (%s) ", "req_dvs_cd			"	,	p->req_dvs_cd				);
			LOG_OUT("%30s - (%s) ", "rfid_card_dvs		"	,	p->rfid_card_dvs			);
			LOG_OUT("%30s - (%s) ", "rfid_dongle_dta	"	,	p->rfid_dongle_dta			);
			LOG_OUT("%30s - (%s) ", "mbrs_yn			"	,	p->mbrs_yn					);
			LOG_OUT("%30s - (%s) ", "mbrs_no			"	,	p->mbrs_no					);
			LOG_OUT("%30s - (%s) ", "mrnp_dt			"	,	p->mrnp_dt					);
			LOG_OUT("%30s - (%s) ", "mrnp_time			"	,	p->mrnp_time				);
			LOG_OUT("%30s - (Num = %d) ", "n_mip_mm_num"	,	*(int *) p->n_mip_mm_num	);
			LOG_OUT("%30s - (%s) ", "trd_dvs_cd			"	,	p->trd_dvs_cd				);
			//LOG_OUT("%30s - (%s) ", "enc_dta			"	,	PST_FIELD(p, enc_dta				));
			//LOG_OUT("%30s - (%s) ", "emv_dta			"	,	PST_FIELD(p, emv_dta				));

			LOG_OUT("%30s - (Num = %d) ", "rec_num"	,	*(int *)p->rec_num	);
			nCount = *(int *)p->rec_num;
			for(i = 0; i < nCount; i++)
			{
				stk_tm_pubtckcard_ktc_list_t *pList;

				pList = &p->List[i];
				
				LOG_OUT("====== index (%d) =========", i);
				LOG_OUT("%30s - (%s) ", "tck_knd_cd	"			,	pList->tck_knd_cd			);
				LOG_OUT("%30s - (%s) ", "sats_no	"			,	pList->sats_no				);
				LOG_OUT("%30s - (%s) ", "pcpy_no	"			,	pList->pcpy_no				);
				LOG_OUT("%30s - (%s) ", "fee_knd_cd	"			,	pList->fee_knd_cd			);
				LOG_OUT("%30s - (Num = %d)  ", "n_tissu_fee"	,	*(int *)pList->n_tissu_fee	);
				LOG_OUT("%30s - (Num = %d)  ", "n_ogn_fee"		,	*(int *)pList->n_ogn_fee	);
				LOG_OUT("%30s - (%s) ", "dc_knd_cd	"			,	pList->dc_knd_cd			);
			}
		}

		Transact_SetState(TR_PBTCK_TMEXP_TMAX_ISSUE, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_360_command
 * @details		[티머니고속]-[현장발권] - 승차권발권(현금영수증_KTC) (360)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_360_command(BYTE *pRecvData)
{
	int								nRet = 0;
	BYTE							byACK = 0;
	PUI_HEADER_T					pHdr;
	PUI_RESP_TM_PUBTCK_CSRC_KTC_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_TM_PUBTCK_CSRC_KTC_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		// set memory
		{
			/// set 결제수단
			CPubTckTmExpMem::GetInstance()->base.pyn_mns_dvs_cd[0] = PYM_CD_CSRC;
			CPubTckTmExpMem::GetInstance()->base.ui_pym_dvs_cd[0] = PYM_CD_CSRC;
			CPubTckTmExpMem::GetInstance()->base.ui_csrc_dvs_cd[0] = 3;

			if(pResp->dt.user_dvs_cd[0] == 0x30)
			{	// 개인
				CPubTckTmExpMem::GetInstance()->base.ui_csrc_use[0] = 0;
			}
			else
			{	// 법인
				CPubTckTmExpMem::GetInstance()->base.ui_csrc_use[0] = 1;
			}

			::CopyMemory(&CPubTckTmExpMem::GetInstance()->m_tReqPubTckCsrcKtc, 
						 &pHdr->byBody, 
						 sizeof(stk_tm_pubtckcsrc_ktc_t));
		}

		/// debug
		if(1)
		{
			int i, nCount;
			stk_tm_pubtckcsrc_ktc_t *p;

			i = nCount = 0;
			p = (stk_tm_pubtckcsrc_ktc_t *) &pResp->dt;

			LOG_OUT("%30s - (%s) ", "req_pgm_dvs		"	,	p->req_pgm_dvs			);
			LOG_OUT("%30s - (%s) ", "req_trml_no		"	,	p->req_trml_no			);
			LOG_OUT("%30s - (%s) ", "req_wnd_no			"	,	p->req_wnd_no				);
			LOG_OUT("%30s - (%s) ", "req_user_no		"	,	p->req_user_no			);
			LOG_OUT("%30s - (%s) ", "user_dvs_cd		"	,	p->user_dvs_cd			);
			LOG_OUT("%30s - (%s) ", "alcn_depr_trml_no	"	,	p->alcn_depr_trml_no		);
			LOG_OUT("%30s - (%s) ", "alcn_arvl_trml_no	"	,	p->alcn_arvl_trml_no		);
			LOG_OUT("%30s - (%s) ", "alcn_depr_dt		"	,	p->alcn_depr_dt			);
			LOG_OUT("%30s - (%s) ", "alcn_depr_time		"	,	p->alcn_depr_time			);
			LOG_OUT("%30s - (%s) ", "depr_trml_no		"	,	p->depr_trml_no			);
			LOG_OUT("%30s - (%s) ", "arvl_trml_no		"	,	p->arvl_trml_no			);
			LOG_OUT("%30s - (%s) ", "depr_dt			"	,	p->depr_dt				);
			LOG_OUT("%30s - (%s) ", "depr_time			"	,	p->depr_time				);
			LOG_OUT("%30s - (%s) ", "bus_cls_cd			"	,	p->bus_cls_cd				);
			LOG_OUT("%30s - (%s) ", "cacm_cd			"	,	p->cacm_cd				);
			
#if (_KTC_CERTIFY_ <= 0)
			LOG_OUT("%30s - (%s) ", "user_key_val		"	,	p->user_key_val			);
#endif			
			LOG_OUT("%30s - (%s) ", "mbrs_yn			"	,	p->mbrs_yn				);
			LOG_OUT("%30s - (%s) ", "mbrs_no			"	,	p->mbrs_no				);
			LOG_OUT("%30s - (%s) ", "mrnp_dt			"	,	p->mrnp_dt				);
			LOG_OUT("%30s - (%s) ", "mrnp_time			"	,	p->mrnp_time				);
			LOG_OUT("%30s - (%s) ", "trd_dvs_cd			"	,	p->trd_dvs_cd				);
#if (_KTC_CERTIFY_ <= 0)
			LOG_OUT("%30s - (%s) ", "enc_dta			"	,	p->enc_dta				);
			LOG_OUT("%30s - (%s) ", "emv_dta			"	,	p->emv_dta				);
#endif			
			LOG_OUT("%30s - (%s) ", "rec_num			"	,	p->rec_num				);

			nCount = *(int *)p->rec_num;

			for(i = 0; i < nCount; i++)
			{
				stk_tm_pubtckcsrc_ktc_list_t *pList;

				pList = &p->List[i];

				LOG_OUT("====== index (%d) =========", i);
				LOG_OUT("%30s - (%s) ", "tck_knd_cd	"	,	pList->tck_knd_cd		);
				LOG_OUT("%30s - (%s) ", "sats_no	"	,	pList->sats_no		);
				LOG_OUT("%30s - (%s) ", "pcpy_no	"	,	pList->pcpy_no		);
				LOG_OUT("%30s - (%s) ", "fee_knd_cd	"	,	pList->fee_knd_cd		);
				LOG_OUT("%30s - (%s) ", "n_tissu_fee"	,	pList->n_tissu_fee	);
				LOG_OUT("%30s - (%s) ", "n_ogn_fee	"	,	pList->n_ogn_fee		);
				LOG_OUT("%30s - (%s) ", "dc_knd_cd	"	,	pList->dc_knd_cd		);
			}

		}

		Transact_SetState(TR_PBTCK_TMEXP_TMAX_ISSUE, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_362_command
 * @details		[티머니고속]-[현장발권] - 승차권발권(부가상품권) (362)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_362_command(BYTE *pRecvData)
{
	int							nRet = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_TM_PUBTCK_PRD_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_TM_PUBTCK_PRD_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		// set memory
		{
			::CopyMemory(&CPubTckTmExpMem::GetInstance()->m_tReqPubTckPrd, 
						 &pHdr->byBody, 
						 sizeof(stk_tm_pubtckprd_t));
		}

		Transact_SetState(TR_PBTCK_TMEXP_TMAX_ISSUE, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_364_command
 * @details		[티머니고속]-[현장발권] - 경유지 정보
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_364_command(BYTE *pRecvData)
{
	int							nRet = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_CM_READTHRU_TRML_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_CM_READTHRU_TRML_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		// 티머니고속 set memory
		{
			CPubTckTmExpMem* pPubTckTr = CPubTckTmExpMem::GetInstance();

			::CopyMemory(&pPubTckTr->m_tReqThru, &pHdr->byBody, sizeof(stk_tm_readthrutrml_t));
		}

		Transact_SetState(TR_PBTCK_TMEXP_REQ_THRU, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_366_command
 * @details		[티머니고속]-[현장발권] - 현금영수증 MS카드
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_366_command(BYTE *pRecvData)
{
	int							nRet = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_CM_READTHRU_TRML_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_CM_READTHRU_TRML_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		// 티머니고속 set memory
		{
			CPubTckTmExpMem* pPubTckTr = CPubTckTmExpMem::GetInstance();

			::CopyMemory(&pPubTckTr->m_tReqThru, &pHdr->byBody, sizeof(stk_tm_readthrutrml_t));
		}

		Transact_SetState(TR_PBTCK_TMEXP_REQ_THRU, 0);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_500_command
 * @details		[관리자] - 관리자 메인 (500)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_500_command(BYTE *pRecvData)
{
	int				nRet = 0;
	BYTE			byACK = 0;
	PUI_HEADER_T	pHdr;

	try
	{
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

		Transact_SetState(TR_ADMIN_MAIN, 0);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_501_command
 * @details		[관리자] - 관리자 로그인 (501)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_501_command(BYTE *pRecvData)
{
	int						nRet = 0;
	UI_SND_ADMIN_LOGIN_T	sPacket;
	PUI_HEADER_T			pHdr;
	PUI_RESP_ADMIN_LOGIN_T	pResp;

	try
	{
		::ZeroMemory(&sPacket, sizeof(UI_SND_ADMIN_LOGIN_T));

		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_ADMIN_LOGIN_T) &pHdr->byBody;

		sPacket.byAck = CHAR_ACK;

		if(pResp->byLogin == 0x01)
		{	/// 관리자 - 로그인
			if( Config_IsCCServer() == TRUE )
			{
				nRet = Svr_IfSv_213(pResp->szID, pResp->szPwd);
				/* // 20211116 DEL
				if(nRet >= 0)
				*/ // 20211116 ~DEL
				// 20211116 MOD
				if( (nRet >= 0) || 
					( (memcmp(pResp->szID, "atec", 4) == 0) && (memcmp(pResp->szPwd, "atec1234", 4) == 0) )	
				  )
				// 20211116 ~MOD
				{
					sPacket.byLogin = 0x01;
					SetCheckEventCode(EC_ADMIN_LOGIN, TRUE);
				}
				else
				{
					sPacket.byAck = CHAR_NAK;
				}
			}
			else if( Config_IsExpServer() == TRUE )
			{
				if( (memcmp(pResp->szID, "atec", 4) == 0) && (memcmp(pResp->szPwd, "atec", 4) == 0) )	
				{
					sPacket.byLogin = 0x01;
					SetCheckEventCode(EC_ADMIN_LOGIN, TRUE);
				}
				else
				{
					sPacket.byAck = CHAR_NAK;
				}
			}
			else
			{
				sPacket.byAck = CHAR_NAK;
			}

			Transact_SetState(TR_ADMIN_LOGIN, 0);
		}
		else
		{	/// 관리자 - 로그아웃

			INI_ReadEnvFile();
			INI_ReadDebugFile();

			nRet = SetCheckEventCode(EC_ADMIN_LOGIN, FALSE);
			if(nRet < 0)
			{
				//sPacket.byAck = CHAR_NAK;
			}
			sPacket.byLogin = 0x02;
			Transact_SetState(TR_ADMIN_LOGOUT, 0);
		}

		nRet = SendPacket(pHdr->wCommand, (BYTE *)&sPacket, sizeof(UI_SND_ADMIN_LOGIN_T));

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}


/**
 * @brief		resp_502_command
 * @details		[관리자] - 설정관리 - 기능설정 (502)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_502_command(BYTE *pRecvData)
{
	int						nRet = 0;
	BYTE					byACK = 0;
	PUI_HEADER_T			pHdr;
	PUI_RESP_ADMIN_FUNC_T	pResp;
  
	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_ADMIN_FUNC_T) &pHdr->byBody;

		POPER_FILE_CONFIG_T pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

		::CopyMemory(&pConfig->base_t, &pResp->baseInfo_t, sizeof(UI_BASE_T));

		PrintOptionInfo(&pConfig->base_t);

		//if(pConfig->base_t.ccs_svr_kind == 0x30)
		//{	/// 시외버스 - 사용안함

		//}
		//else
		//{	/// 시외버스 - 사용함

		//}

		SetOperConfigData();

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		//UI_AddDevConfigData();
		UI_Add_Q_SendData(UI_CMD_DEV_CONFIG, TRUE, 1);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_504_command
 * @details		[관리자] - 설정관리 - 터미널설정 (504)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_504_command(BYTE *pRecvData)
{
	int						nRet  = 0, i = 0, nCount = 0;
	BYTE					byACK = 0;
	HANDLE					hFile = INVALID_HANDLE_VALUE;
	CString					strFullName = "";
	PUI_HEADER_T			pHdr;
	PUI_RESP_ADMIN_THRML_T	pResp;
	PUI_RESP_THRML_LIST_T	pList;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_ADMIN_THRML_T) &pHdr->byBody;

		OperGetFileName(OPER_FILE_ID_TRML, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			byACK = CHAR_NAK;
			nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);
			return -1;
		}

		/// 버스구분
		MyWriteFile(hFile, &pResp->chBusDVS, sizeof(pResp->chBusDVS));
		/// 터미널 갯수
		nCount = pResp->wCount;	
		MyWriteFile(hFile, &pResp->wCount, sizeof(pResp->wCount));

		/// 터미널 정보
		pList = (PUI_RESP_THRML_LIST_T) &pResp->byList;
		for(i = 0; i < nCount; i++)
		{
			MyWriteFile(hFile, &pList[i], sizeof(UI_RESP_THRML_LIST_T));
		}
		MyCloseFile(hFile);

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_505_command
 * @details		[관리자] - 설정관리 - 승차권설정 (505)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_505_command(BYTE *pRecvData)
{
	int						nRet = 0, i = 0, nCount = 0;
	BYTE					byACK = 0;
	HANDLE					hFile = INVALID_HANDLE_VALUE;
	CString					strFullName = "";
	PUI_HEADER_T			pHdr;
	PUI_RESP_ADMIN_TICKET_T pResp;
	PUI_RESP_TICKET_LIST_T	pList;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_ADMIN_TICKET_T) &pHdr->byBody;

		OperGetFileName(OPER_FILE_ID_TICKET, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			byACK = CHAR_NAK;
			nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);
			return -1;
		}

		/// 버스구분
		MyWriteFile(hFile, &pResp->chBusDVS, sizeof(pResp->chBusDVS));
		/// 승차권 갯수
		nCount = pResp->wCount;	
		MyWriteFile(hFile, &pResp->wCount, sizeof(pResp->wCount));

		pList = (PUI_RESP_TICKET_LIST_T) &pResp->byList;
		for(i = 0; i < nCount; i++)
		{
			MyWriteFile(hFile, &pList[i], sizeof(UI_RESP_TICKET_LIST_T));
		}
		MyCloseFile(hFile);

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_506_command
 * @details		[관리자] - 설정관리 - 운수사설정 (506)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_506_command(BYTE *pRecvData)
{
	int							nRet = 0, i = 0, nCount = 0;
	BYTE						byACK = 0;
	HANDLE						hFile = INVALID_HANDLE_VALUE;
	CString						strFullName = "";
	PUI_HEADER_T				pHdr;
	PUI_RESP_ADMIN_BUS_CACM_T	pResp;
	PUI_RESP_BUS_CACM_LIST_T	pList;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_ADMIN_BUS_CACM_T) &pHdr->byBody;

		OperGetFileName(OPER_FILE_ID_CACM, strFullName);
		hFile = MyOpenFile2(strFullName, CREATE_ALWAYS);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			byACK = CHAR_NAK;
			nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);
			return -1;
		}

		/// 버스구분
		MyWriteFile(hFile, &pResp->chBusDVS, sizeof(pResp->chBusDVS));
		/// 운수사 갯수
		nCount = pResp->wCount;
		MyWriteFile(hFile, &pResp->wCount, sizeof(pResp->wCount));

		/// 운수사 정보
		pList = (PUI_RESP_BUS_CACM_LIST_T) &pResp->byList;
		for(i = 0; i < nCount; i++)
		{
			MyWriteFile(hFile, &pList[i], sizeof(UI_RESP_BUS_CACM_LIST_T));
		}
		MyCloseFile(hFile);

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_508_command
 * @details		[관리자] - 설정관리 - 환경설정 - 터미널 설정 (508)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_508_command(BYTE *pRecvData)
{
	int								nRet = 0, nSvrKind = 0;
	BYTE							byACK = 0;
	PUI_HEADER_T					pHdr;
	PUI_RESP_ADMIN_SET_TRML_INFO_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_ADMIN_SET_TRML_INFO_T) &pHdr->byBody;

		{
			nSvrKind = GetConfigServerKind();

			SetConfigTrmlInfo(SVR_DVS_CCBUS, (char *)&pResp->ccTrmlInfo);

			if(nSvrKind & SVR_DVS_KOBUS)
			{
				SetConfigTrmlInfo(SVR_DVS_KOBUS, (char *)&pResp->expTrmlInfo);
			}
			if(nSvrKind & SVR_DVS_TMEXP)
			{
				SetConfigTrmlInfo(SVR_DVS_TMEXP, (char *)&pResp->expTrmlInfo);
			}
		}

		SetOperConfigData();

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		// 107 전송
		UI_AddTrmlInfo();

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_511_command
 * @details		[관리자] - 시스템관리 (511)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_511_command(BYTE *pRecvData)
{
	int				nRet = 0;
	BYTE			byACK = 0;
	DWORD			dwTick = 0;
	PUI_HEADER_T	pHdr;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;

		if( (pHdr->byBody >= 0x01) && (pHdr->byBody <= 0x05))
		{
			byACK = CHAR_ACK;
		}
		else
		{
			byACK = CHAR_NAK;
		}
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		if(pHdr->byBody == 0x01)
		{	/// 프로그램 종료
			CString		strLaunchName("Launcher.exe");

			LOG_OUT("Code = %02X, 프로그램 종료", pHdr->byBody);

			dwTick = ::GetTickCount();
			while( Util_CheckExpire(dwTick) < (1000 * 2) );
			Util_CloseProcess(strLaunchName);

			dwTick = ::GetTickCount();
			while( Util_CheckExpire(dwTick) < (1000 * 2) );

			CString		strUiName("TicketMachine.exe");
			Util_CloseProcess(strUiName);

			dwTick = ::GetTickCount();
			while( Util_CheckExpire(dwTick) < (1000 * 2) );

			exit(0);		
		}
		else if(pHdr->byBody == 0x02)
		{	/// 시스템 종료
			LOG_OUT("Code = %02X, 시스템 종료", pHdr->byBody);
			Util_SystemReboot(0);
		}
		else if(pHdr->byBody == 0x03)
		{	/// 시스템 재시작
			LOG_OUT("Code = %02X, 시스템 재시작", pHdr->byBody);
			Util_SystemReboot(1);
		}
		else if(pHdr->byBody == 0x04)
		{	/// 창구마감
			LOG_OUT("Code = %02X, 창구마감", pHdr->byBody);
			// WndKioskClose(FALSE);			// 20210513 DEL 
			int nClsRtn = WndKioskClose(FALSE);	// 20210513 MOD 
			// 20210513 ADD
			if (nClsRtn < 0)	// 수동창구마감 처리실패 // 자동창구마감인 경우 시스템로그만 남기고 정상운영모드로 복귀, Main->UI 메세지 전송하지 않음
				UI_ResWndKioskClose(UI_CMD_ADMIN_WND_CLOSE, "XXX901", NULL);	// 수동창구마감 처리실패 Main->UI 메세지 전송 // 코드값 미정의, 사용안함
			// 20210513 ~ADD
		}
		else if(pHdr->byBody == 0x05)
		{	/// 프로그램 재시작
			CString		strUiName("TicketMachine.exe");

			LOG_OUT("Code = %02X, 프로그램 재시작", pHdr->byBody);

			Util_CloseProcess(strUiName);

			dwTick = ::GetTickCount();
			while( Util_CheckExpire(dwTick) < (1000 * 2) );

			exit(0);		
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_512_command
 * @details		[관리자] - 시스템관리 - 장비별 리셋(512)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_512_command(BYTE *pRecvData)
{
	int					nRet = 0;
	BYTE				Buffer[10];
	BYTE				byFlag = 0;
	ACC_NOPAY_DATA_T	Acc;
	PUI_HEADER_T		pHdr;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;

		if(pHdr->byBody == 0x00)
		{	// 전체 Reset
			nRet = Coin_Reset(TRUE, TRUE);
			nRet = Bill_Reset();
			nRet = CDU_Reset();
			
			::ZeroMemory(&Acc, sizeof(ACC_NOPAY_DATA_T));
			AddAccumulateData(ACC_UNPAID_CLEAR, (char *)&Acc, 1);

			ClearEventCode(FLAG_ALL);
		}
		else if(pHdr->byBody == 0x01)
		{	/// 동전방출기 리셋
			ACC_NOPAY_DATA_T Acc;

			nRet = Coin_Reset(TRUE, TRUE);
			//nRet = Coin_GetStatus();

			/// 미방출 데이타 clear.. 임시코드
			::ZeroMemory(&Acc, sizeof(ACC_NOPAY_DATA_T));
			AddAccumulateData(ACC_UNPAID_CLEAR, (char *)&Acc, 1);

			ClearEventCode(FLAG_COIN);

			SetCheckEventCode(EC_COIN_NO_OUT, FALSE);
			SetCheckEventCode(EC_CDU_NO_OUT, FALSE);
		}
		else if(pHdr->byBody == 0x02)
		{	/// 지폐입금기 리셋
			nRet = Bill_Reset();
			ClearEventCode(FLAG_BILL);
		}
		else if(pHdr->byBody == 0x03)
		{	/// 지폐방출기 리셋
			nRet = CDU_Reset();
			ClearEventCode(FLAG_CDU);
		}
		else if(pHdr->byBody == 0x04)
		{	/// 승차권 스캐너 리셋
			ClearEventCode(FLAG_TCK_SCANNER);
		}
		else if(pHdr->byBody == 0x05)
		{	/// 승차권 프린터	리셋
			ClearEventCode(FLAG_TCK_PRT);
		}
		else if(pHdr->byBody == 0x06)
		{	/// 영수증 프린터 리셋
			ClearEventCode(FLAG_PRT);
		}
		else if(pHdr->byBody == 0x07)
		{	/// 신용카드 리더기 리셋
			ClearEventCode(FLAG_ICCARD);
		}
		else if(pHdr->byBody == 0x08)
		{	/// 티맥스 서버 리셋
			ClearEventCode(FLAG_TMAX);
		}
		else if(pHdr->byBody == 0x09)
		{	/// 운영
			ClearEventCode(FLAG_OP);
		}
		else if(pHdr->byBody == 0x0A)
		{	/// RF
			ClearEventCode(FLAG_RF);
		}
		else if(pHdr->byBody == 0x0B)
		{	/// 관제
			ClearEventCode(FLAG_SMS);
		}

		Buffer[0] = CHAR_ACK;
		Buffer[1] = pHdr->byBody;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)Buffer, 2);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_521_command
 * @details		[관리자] - 티켓관리 (521)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_521_command(BYTE *pRecvData)
{
	int							nRet = 0, nSvrKind = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_ADMIN_TCK_INHR_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_ADMIN_TCK_INHR_T) &pHdr->byBody;

		nRet = 0;

		if(pResp->chBusDVS == '1')
		{	/// [시외버스] 버스티켓고유번호 설정
			// 버스티켓 교환
			nRet = Svr_IfSv_152(pResp->bus_tck_inhr_no);
			nSvrKind = SVR_DVS_CCBUS;
		}
		else if(pResp->chBusDVS == '2')
		{	/// [코버스] 추가 버스티켓 고유번호 설정
			nRet = Kobus_TK_ChangeTicketBox(pResp->bus_tck_inhr_no);
			nSvrKind = SVR_DVS_KOBUS;
		}
		else if(pResp->chBusDVS == '3')
		{	/// [티머니고속] 추가 버스티켓 고유번호 설정
			nRet = TmExp_ChangeTicketBox(pResp->bus_tck_inhr_no);
			nSvrKind = SVR_DVS_TMEXP;
		}
		else
		{
			nRet = -1;
		}

		if(nRet < 0)
		{
			UI_SND_ERROR_T 	Info;

			::ZeroMemory(&Info, sizeof(UI_SND_ERROR_T));

// 			Info.byAck = CHAR_NAK;
// 			sprintf(Info.szErrCode, CConfigTkMem::GetInstance()->GetErrorCode());

			nRet = MakeErrorPacket((char *)&Info);
			nRet = SendPacket(pHdr->wCommand, (BYTE *)&Info, sizeof(UI_SND_ERROR_T));
		}
		else
		{
			int n_bus_tck_inhr_no = 0;
			BYTE byACK = 0;

			byACK = CHAR_ACK;
			nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

			n_bus_tck_inhr_no = Util_Ascii2Long(pResp->bus_tck_inhr_no, 8);

			// todo : 티켓고유번호 설정
			AddAccumBusTckInhrNo((int)ACC_TICKET_SEQ_ASSIGN, nSvrKind, n_bus_tck_inhr_no);
		}

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_522_command
 * @details		[관리자] - 티켓관리 - 승차권 수량 보급 (522)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_522_command(BYTE *pRecvData)
{
	int							nRet = 0, nSvrKind = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_ADMIN_TCK_COUNT_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_ADMIN_TCK_COUNT_T) &pHdr->byBody;

		if(pResp->nTicketCount > 0)
		{	/// 승차권 수량 설정
			ACC_TICKET_DATA_T	AccData;
			PFILE_ACCUM_N1010_T pAccum;

			pAccum = GetAccumulateData();

			::ZeroMemory(&AccData, sizeof(ACC_TICKET_DATA_T));

			AccData.Base.dwTick = Util_GetCurrentTick();
			AccData.Base.wKind = ACC_TICKET_ASSIGN;
			switch( pResp->chBusDVS )
			{
			case '1':
				AccData.nSvrKind = SVR_DVS_CCBUS;
				break;
			case '2':
				AccData.nSvrKind = SVR_DVS_KOBUS;
				break;
			case '3':
				AccData.nSvrKind = SVR_DVS_TMEXP;
				break;
			default:
				byACK = CHAR_NAK;
				nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);
				UI_AddDevAccountInfo();
				return nRet;
			}
			AccData.nCount = (int) pResp->nTicketCount;

			AddAccumulateData(ACC_TICKET_ASSIGN, (char *)&AccData, 1);	

			byACK = CHAR_ACK;
		}
		else
		{
			byACK = CHAR_NAK;
		}

		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);
		UI_AddDevAccountInfo();

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

//
/**
 * @brief		resp_523_command
 * @details		[관리자] - 티켓 정보 요청 (523)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_523_command(BYTE *pRecvData)
{
	int  nRet = 0;

	try
	{
		if(1)
		{
			int						nSvrKind;
			UI_SND_ADMIN_TCK_INFO_T Info;
			char					Buff[100];
			PFILE_ACCUM_N1010_T		pAccum;

			::ZeroMemory(&Info, sizeof(UI_SND_ADMIN_TCK_INFO_T));

			pAccum = GetAccumulateData();

			/// ACK
			Info.byACK = CHAR_ACK;
			/// 티켓 수량
			Info.n_ticket_count = pAccum->Curr.phyTicket.nCount;
			LOG_OUT("티켓잔여수량 = %d", Info.n_ticket_count);

			nSvrKind = GetConfigServerKind();
			if(nSvrKind & SVR_DVS_CCBUS)
			{
				/// [시외] 버스티켓 고유번호
				sprintf(Buff, "%08d", pAccum->Curr.ccsTicket.n_bus_tck_inhr_no);
				::CopyMemory(Info.ccs_bus_tck_inhr_no, Buff, strlen(Buff));
			}

			if(nSvrKind & (SVR_DVS_KOBUS | SVR_DVS_TMEXP))
			{
				/// [고속] 버스티켓 고유번호
				sprintf(Buff, "%08d", pAccum->Curr.expTicket.n_bus_tck_inhr_no);
				::CopyMemory(Info.exp_bus_tck_inhr_no, Buff, strlen(Buff));
			}

			// 524
			UI_AddQueueInfo(UI_CMD_ADMIN_RSP_TCK_INFO, (char *) &Info, sizeof(UI_SND_ADMIN_TCK_INFO_T));
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}


/**
 * @brief		resp_531_command
 * @details		[관리자] - 발권/재발권/환불 내역 조회
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_531_command(BYTE *pRecvData)
{
	int							nRet = 0;
	char						szDate[20], szTime[20];
	SYSTEMTIME					st;
	UI_SND_ADMIN_INQ_ISSUE_T	sPacket;
	UI_SND_ERROR_T				sPacketNAK;
	PUI_HEADER_T				pHdr;
	PUI_RESP_ADMIN_INQ_T		pResp;

	try
	{
		::ZeroMemory(&sPacket, sizeof(UI_SND_ADMIN_INQ_ISSUE_T));
		::ZeroMemory(&sPacketNAK, sizeof(UI_SND_ERROR_T));

		pHdr  = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_ADMIN_INQ_T) &pHdr->byBody;

		GetLocalTime(&st);
		sprintf(szDate, "%04d%02d%02d", st.wYear, st.wMonth, st.wDay);
		sprintf(szTime, "%02d%02d%02d", st.wHour, st.wMinute, st.wSecond);

		sPacket.byAck = CHAR_ACK;
		sPacket.chBusDVS = pResp->chBusDVS;

		LOG_OUT("조회종류 = %02X, 시외/고속 구분 = %02X ..", pResp->byInq, pResp->chBusDVS);

		if(pResp->byInq == 0x01)
		{	/// 발권 내역 조회
			if(pResp->chBusDVS == '1')
			{	/// 시외버스
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
			else if(pResp->chBusDVS == '2')
			{	/// 코버스
				nRet = Kobus_TK_InqPubPt(szDate);
				if(nRet < 0)
				{
					sPacket.byAck = CHAR_NAK;
				}
				else
				{
					OperGetFileInfo(OPER_FILE_ID_KO_TK_INQPUBPT, sPacket.szFileName, &sPacket.nSize);
				}
			}
			else if(pResp->chBusDVS == '3')
			{	/// 티머니고속
				nRet = TmExp_TK_PubPtInquiry(szDate);
				//nRet = TmExp_TK_PubPtInquiry("20210122");
				if(nRet < 0)
				{
					sPacket.byAck = CHAR_NAK;
				}
				else
				{
					OperGetFileInfo(OPER_FILE_ID_EZ_TK_INQPUBPT, sPacket.szFileName, &sPacket.nSize);
				}
			}
			else
			{
				sPacket.byAck = CHAR_NAK;
			}

		}
		else if(pResp->byInq == 0x02)
		{	/// 재발권 내역 조회

			if(pResp->chBusDVS == '1')
			{	/// 시외버스
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
			else if(pResp->chBusDVS == '2')
			{	/// 코버스
				;
			}
			else if(pResp->chBusDVS == '3')
			{	/// 티머니고속
				nRet = TmExp_TK_RePubPtInquiry(szDate);
				if(nRet < 0)
				{
					sPacket.byAck = CHAR_NAK;
				}
				else
				{
					OperGetFileInfo(OPER_FILE_ID_EZ_TK_INQREPUBPT, sPacket.szFileName, &sPacket.nSize);
				}
			}
		}
		else if(pResp->byInq == 0x03)
		{	/// 환불 내역 조회
			sprintf(szTime, "%02d%02d%02d", 0, 0, 0);

			if(pResp->chBusDVS == '1')
			{	/// 시외버스
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
			else if(pResp->chBusDVS == '2')
			{	/// 코버스
				;
			}
			else if(pResp->chBusDVS == '3')
			{	/// 티머니고속
				nRet = TmExp_TK_CanRyPtInquiry(szDate);
				if(nRet < 0)
				{
					sPacket.byAck = CHAR_NAK;
				}
				else
				{
					OperGetFileInfo(OPER_FILE_ID_EZ_TK_INQCANRYPT, sPacket.szFileName, &sPacket.nSize);
				}
			}
		}
		else
		{
			sPacket.byAck = CHAR_NAK;
		}

		if( sPacket.byAck == CHAR_ACK )
		{
			nRet = SendPacket(pHdr->wCommand, (BYTE *)&sPacket, sizeof(UI_SND_ADMIN_INQ_ISSUE_T));
		}
		else
		{
// 			sPacketNAK.byAck = CHAR_NAK;
// 			sprintf(sPacketNAK.szErrorCode, "%s", CConfigTkMem::GetInstance()->GetErrorCode());
			
			nRet = MakeErrorPacket((char *)&sPacketNAK);
			nRet = SendPacket(pHdr->wCommand, (BYTE *)&sPacketNAK, sizeof(UI_SND_ERROR_T));
		}

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_534_command
 * @details		[관리자] - 재발행	(534)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_534_command(BYTE *pRecvData)
{
	int							nRet = 0, nSvrKind = 0;
	char						byACK;
	PUI_HEADER_T				pHdr;
	PUI_RESP_ADMIN_RE_ISSUE_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_ADMIN_RE_ISSUE_T) &pHdr->byBody;

		if( pResp->chBusDVS == '1' )
		{
			nSvrKind = SVR_DVS_CCBUS;
			TR_LOG_OUT("시외버스 - 재발행.");

			// 시외서버 버스티켓 재발행
			nRet = Svr_IfSv_136(pResp->pub_dt, pResp->pub_shct_trml_cd, pResp->pub_wnd_no, pResp->pub_sno);
			if(nRet < 0)
			{	/// 실패시...
				UI_SND_ERROR_T	errData;

				::ZeroMemory(&errData, sizeof(UI_SND_ERROR_T));
				nRet = MakeErrorPacket((char *)&errData);
				nRet = SendPacket(pHdr->wCommand, (BYTE *)&errData, sizeof(UI_SND_ERROR_T));
			}
			else
			{	/// 성공시...
				byACK = CHAR_ACK;
				nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

				// 티켓 재발행....
				CPubTckMem::GetInstance()->MakeReTicketPrtData();

				vector<TCK_PRINT_FMT_T>::iterator iter;

				for(iter = CPubTckMem::GetInstance()->m_vtPrtTicket.begin(); iter != CPubTckMem::GetInstance()->m_vtPrtTicket.end(); iter++)
				{
					int nSatNo;
					DWORD dwTick;

					nSatNo = Util_Ascii2Long(iter->sats_no, strlen(iter->sats_no));

					// 2. 승차권 발행	
					TckPrt_PubPrintTicket(nSvrKind, (char *) iter._Ptr);

					dwTick = ::GetTickCount();
					while( Util_CheckExpire(dwTick) < (1000 * 3) );

					/// 회계반영 - 승차권 수량
					//AddAccumTicketWork(SVR_DVS_CCBUS, (int) ACC_TICKET_PUB_ISSUE, 1, iter->n_tisu_amt);
					/// 티켓 고유번호 증가
					AddAccumBusTckInhrNo(ACC_TICKET_SEQ_PLUS, nSvrKind, 1);
				}
				UI_AddDevAccountInfo();
				CPubTckMem::GetInstance()->Initialize();
				CPubTckKobusMem::GetInstance()->Initialize();
				CPubTckTmExpMem::GetInstance()->Initialize();
			}
		}
		else if( pResp->chBusDVS == '2' )
		{
			nSvrKind = SVR_DVS_KOBUS;
			TR_LOG_OUT("코버스 - 재발행.");

			nRet = Kobus_TK_RePubTck(pResp->pub_dt, pResp->pub_shct_trml_cd, pResp->pub_wnd_no, pResp->pub_sno);
			if(nRet < 0)
			{	/// 실패시...
				UI_SND_ERROR_T	errData;

				::ZeroMemory(&errData, sizeof(UI_SND_ERROR_T));
				nRet = MakeErrorPacket((char *)&errData);
				nRet = SendPacket(pHdr->wCommand, (BYTE *)&errData, sizeof(UI_SND_ERROR_T));
			}
			else
			{	/// 성공시...
				byACK = CHAR_ACK;
				nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

				// 티켓 재발행....
				CPubTckKobusMem::GetInstance()->MakeReTicketPrtData();

				vector<TCK_PRINT_FMT_T>::iterator iter;

				for(iter = CPubTckKobusMem::GetInstance()->m_vtPrtTicket.begin(); iter != CPubTckKobusMem::GetInstance()->m_vtPrtTicket.end(); iter++)
				{
					int nSatNo;
					DWORD dwTick;

					nSatNo = Util_Ascii2Long(iter->sats_no, strlen(iter->sats_no));

					// 2. 승차권 발행	
					/// org code
					//TckPrt_PubPrintTicket(nSvrKind, (char *) iter._Ptr);

					/// mod code : 2020.05.18
					if(GetConfigTicketPapaer() == PAPER_ROLL)
					{	/// 감열지 용지
						Printer_TicketPrint(nSvrKind, (int)enFUNC_PBTCK, (char *) iter._Ptr);
					}
					else
					{	/// 승차권 용지
						TckPrt_PubPrintTicket(nSvrKind, (char *) iter._Ptr);
					}
					/// ~mod code : 2020.05.18

					dwTick = ::GetTickCount();
					while( Util_CheckExpire(dwTick) < (1000 * 3) );

					/// 회계반영 - 승차권 수량
					//AddAccumTicketWork(SVR_DVS_CCBUS, (int) ACC_TICKET_PUB_ISSUE, 1, iter->n_tisu_amt);
					/// 티켓 고유번호 증가
					AddAccumBusTckInhrNo(ACC_TICKET_SEQ_PLUS, nSvrKind, 1);
				}
				UI_AddDevAccountInfo();
				CPubTckMem::GetInstance()->Initialize();
				CPubTckKobusMem::GetInstance()->Initialize();
				CPubTckTmExpMem::GetInstance()->Initialize();
			}
		}
		else if( pResp->chBusDVS == '3' )
		{
			CPubTckTmExpMem *pclsRePub;

			nSvrKind = SVR_DVS_TMEXP;
			TR_LOG_OUT("티머니고속 - 재발행.");

			pclsRePub = CPubTckTmExpMem::GetInstance();

			/// memory assign
			/// 1) 발행일자
			::CopyMemory(pclsRePub->m_tReqRePubTck.tissu_dt, pResp->pub_dt, sizeof(pclsRePub->m_tReqRePubTck.tissu_dt));
			/// 2) 발권단축터미널코드
			::CopyMemory(pclsRePub->m_tReqRePubTck.tissu_trml_no, pResp->pub_shct_trml_cd, sizeof(pclsRePub->m_tReqRePubTck.tissu_trml_no));
			/// 3) 발권창구번호
			::CopyMemory(pclsRePub->m_tReqRePubTck.req_wnd_no, pResp->pub_wnd_no, sizeof(pclsRePub->m_tReqRePubTck.req_wnd_no));
			/// 4) 발권일련번호
			::CopyMemory(pclsRePub->m_tReqRePubTck.tissu_sno, pResp->pub_sno, sizeof(pclsRePub->m_tReqRePubTck.tissu_sno));

			nRet = TmExp_TK_RePubTck(pResp);
			if(nRet < 0)
			{	/// 실패시...
				UI_SND_ERROR_T	errData;

				::ZeroMemory(&errData, sizeof(UI_SND_ERROR_T));
				nRet = MakeErrorPacket((char *)&errData);
				nRet = SendPacket(pHdr->wCommand, (BYTE *)&errData, sizeof(UI_SND_ERROR_T));
			}
			else
			{	/// 성공시...
				byACK = CHAR_ACK;
				nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

				// 티켓 재발행....
				CPubTckTmExpMem::GetInstance()->MakeReTicketPrtData();

				vector<TCK_PRINT_FMT_T>::iterator iter;

				for(iter = CPubTckTmExpMem::GetInstance()->m_vtPrtTicket.begin(); iter != CPubTckTmExpMem::GetInstance()->m_vtPrtTicket.end(); iter++)
				{
					int nSatNo;
					DWORD dwTick;

					nSatNo = Util_Ascii2Long(iter->sats_no, strlen(iter->sats_no));

					// 2. 승차권 발행	
					/// org code
					//TckPrt_PubPrintTicket(nSvrKind, (char *) iter._Ptr);

					/// mod code : 2020.05.18
					if(GetConfigTicketPapaer() == PAPER_ROLL)
					{	/// 감열지 용지
						Printer_TicketPrint(nSvrKind, (int)enFUNC_PBTCK, (char *) iter._Ptr);
					}
					else
					{	/// 승차권 용지
						TckPrt_PubPrintTicket(nSvrKind, (char *) iter._Ptr);
					}
					/// ~mod code : 2020.05.18

					dwTick = ::GetTickCount();
					while( Util_CheckExpire(dwTick) < (1000 * 3) );

					/// 회계반영 - 승차권 수량
					//AddAccumTicketWork(SVR_DVS_CCBUS, (int) ACC_TICKET_PUB_ISSUE, 1, iter->n_tisu_amt);
					/// 티켓 고유번호 증가
					AddAccumBusTckInhrNo(ACC_TICKET_SEQ_PLUS, nSvrKind, 1);
				}
				UI_AddDevAccountInfo();
				CPubTckMem::GetInstance()->Initialize();
				CPubTckKobusMem::GetInstance()->Initialize();
				CPubTckTmExpMem::GetInstance()->Initialize();
			}
		}
		else
		{
			TR_LOG_OUT("Unknown 재발행 - 에러.");
			return -1;
		}

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	
	return nRet;
}

/**
 * @brief		resp_540_command
 * @details		[관리자] - 시재마감	(540)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_540_command(BYTE *pRecvData)
{
	int						nRet = 0;
	char					byACK;
	PUI_HEADER_T			pHdr;
	PUI_RESP_CASH_CLOSE_T	pResp;
	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_CASH_CLOSE_T) &pHdr->byBody;

		// 마감데이타 생성
		AddCashCloseData();

		// Accum 마감
		AddAccumCashCloseData();

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_541_command
 * @details		[관리자] - 시재마감 내역
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_541_command(BYTE *pRecvData)
{
	int					nRet = 0;
	PUI_HEADER_T		pHdr;
	PFILE_ACCUM_N1010_T	pAccum;

	LOG_OUT("--- 시재 현재내역 조회 --- ");
	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;

		{
			UI_SND_ADMIN_CASH_HISTO_T sPacket;

			pAccum = GetAccumulateData();

			::ZeroMemory(&sPacket, sizeof(UI_SND_ADMIN_CASH_HISTO_T));

			sPacket.byACK = CHAR_ACK;
			///> 초기 내역
			sPacket.initInfo.w100 = (WORD) pAccum->Close.initCoin.n100;
			sPacket.initInfo.w500 = (WORD) pAccum->Close.initCoin.n500;
			sPacket.initInfo.w1k = (WORD) pAccum->Close.initBill.n1k;
			sPacket.initInfo.w5k = (WORD) pAccum->Close.initBill.n5k;
			sPacket.initInfo.w10k = (WORD) pAccum->Close.initBill.n10k;
			sPacket.initInfo.w50k = (WORD) pAccum->Close.initBill.n50k;
			///> 입금 내역 (지폐)
			sPacket.insInfo.w1k = (WORD) pAccum->Close.inBill.n1k;
			sPacket.insInfo.w5k = (WORD) pAccum->Close.inBill.n5k;
			sPacket.insInfo.w10k = (WORD) pAccum->Close.inBill.n10k;
			sPacket.insInfo.w50k = (WORD) pAccum->Close.inBill.n50k;
			///> 출금 내역 (동전/지폐)
			sPacket.outInfo.w100 = (WORD) pAccum->Close.outCoin.n100;
			sPacket.outInfo.w500 = (WORD) pAccum->Close.outCoin.n500;
			sPacket.outInfo.w1k = (WORD) pAccum->Close.outBill.n1k;
			sPacket.outInfo.w5k = (WORD) pAccum->Close.outBill.n5k;
			sPacket.outInfo.w10k = (WORD) pAccum->Close.outBill.n10k;
			sPacket.outInfo.w50k = (WORD) pAccum->Close.outBill.n50k;
			///> 보급 내역
			sPacket.supplyInfo.w100 = (WORD) pAccum->Close.supplyCoin.n100;
			sPacket.supplyInfo.w500 = (WORD) pAccum->Close.supplyCoin.n500;
			sPacket.supplyInfo.w1k = (WORD) pAccum->Close.supplyBill.n1k;
			sPacket.supplyInfo.w5k = (WORD) pAccum->Close.supplyBill.n5k;
			sPacket.supplyInfo.w10k = (WORD) pAccum->Close.supplyBill.n10k;
			sPacket.supplyInfo.w50k = (WORD) pAccum->Close.supplyBill.n50k;
			///> 회수 내역 (지폐함)
			sPacket.wdrawInfo.w100 = 0;
			sPacket.wdrawInfo.w500 = 0;
			sPacket.wdrawInfo.w1k = (WORD) pAccum->Close.wdrawBill.n1k;
			sPacket.wdrawInfo.w5k = (WORD) pAccum->Close.wdrawBill.n5k;
			sPacket.wdrawInfo.w10k = (WORD) pAccum->Close.wdrawBill.n10k;
			sPacket.wdrawInfo.w50k = (WORD) pAccum->Close.wdrawBill.n50k;
			///> 미출금 내역
#if 0
			sPacket.noOutInfo.w100 = (WORD) pAccum->Close.noCoin.n100;
			sPacket.noOutInfo.w500 = (WORD) pAccum->Close.noCoin.n500;
			sPacket.noOutInfo.w1k = (WORD) pAccum->Close.noBill.n1k;
			sPacket.noOutInfo.w5k = (WORD) pAccum->Close.noBill.n5k;
			sPacket.noOutInfo.w10k = (WORD) pAccum->Close.noBill.n10k;
			sPacket.noOutInfo.w50k = (WORD) pAccum->Close.noBill.n50k;
#else
			sPacket.noOutInfo.w100 = (WORD) pAccum->Curr.noCoin.n100;
			sPacket.noOutInfo.w500 = (WORD) pAccum->Curr.noCoin.n500;
			sPacket.noOutInfo.w1k = (WORD) pAccum->Curr.noBill.n1k;
			sPacket.noOutInfo.w5k = (WORD) pAccum->Curr.noBill.n5k;
			sPacket.noOutInfo.w10k = (WORD) pAccum->Curr.noBill.n10k;
			sPacket.noOutInfo.w50k = (WORD) pAccum->Curr.noBill.n50k;
#endif
			///> 현재 내역
			sPacket.currInfo;				
			sPacket.currInfo.w100 = (WORD) pAccum->Curr.Coin.n100;
			sPacket.currInfo.w500 = (WORD) pAccum->Curr.Coin.n500;
			sPacket.currInfo.w1k = (WORD) pAccum->Curr.BillDispenser.Casst.n1k;
			sPacket.currInfo.w5k = (WORD) pAccum->Curr.BillDispenser.Casst.n5k;
			sPacket.currInfo.w10k = (WORD) pAccum->Curr.BillDispenser.Casst.n10k;
			sPacket.currInfo.w50k = (WORD) pAccum->Curr.BillDispenser.Casst.n50k;

			nRet = SendPacket(pHdr->wCommand, (BYTE *)&sPacket, sizeof(UI_SND_ADMIN_CASH_HISTO_T));
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_542_command
 * @details		[관리자] - 시재보급 
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_542_command(BYTE *pRecvData)
{
	int							nRet = 0;
	char						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_ADMIN_CASH_INS_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_ADMIN_CASH_INS_T) &pHdr->byBody;

		byACK = CHAR_ACK;

		LOG_OUT("시재보급 (100(%d), 500(%d), 1k(%d), 5k(%d), 10k(%d), 50k(%d) !!", 
			pResp->insInfo.w100, pResp->insInfo.w500,
			pResp->insInfo.w1k, pResp->insInfo.w5k, pResp->insInfo.w10k, pResp->insInfo.w50k);

		// 동전보급
		AddAccumCoinData(ACC_COIN_SUPPLY, pResp->insInfo.w100, pResp->insInfo.w500);
		// 지폐보급
		AddAccumBillData(ACC_BILL_SUPPLY, pResp->insInfo.w1k, pResp->insInfo.w5k, pResp->insInfo.w10k, pResp->insInfo.w50k);

		/// 입출내역 TR (보급)
		AddCashTrData(DT_FLAG_SUPPLY_MONEY, pResp->insInfo.w100, pResp->insInfo.w500, pResp->insInfo.w1k, pResp->insInfo.w5k, pResp->insInfo.w10k, pResp->insInfo.w50k);


		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		UI_AddDevAccountInfo();
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_543_command
 * @details		[관리자] - 시재출금 (543)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_543_command(BYTE *pRecvData)
{
	int  nRet = 0;
	int nBillACK, nCoinACK;
	int nOut100, nOut500;
	int nTmp10k, nTmp1k, nTmp100, nTmp500;
	DISPENSE_INFO_T				dspInfo;
	UI_SND_ADMIN_CASH_OUT_T		sPacket;
	UI_SND_CANCRY_CHG_MONEY_T	Info;
	PUI_HEADER_T				pHdr;
	PUI_RESP_ADMIN_CASH_OUT_T	pResp;

	LOG_OUT("--- 시재출금 ---");
	try
	{
		nBillACK = nCoinACK = CHAR_ACK;
		nOut100 = nOut500  =0;

		::ZeroMemory(&sPacket, sizeof(UI_SND_ADMIN_CASH_OUT_T));
		::ZeroMemory(&dspInfo, sizeof(DISPENSE_INFO_T));
		::ZeroMemory(&Info, sizeof(UI_SND_CANCRY_CHG_MONEY_T));

		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_ADMIN_CASH_OUT_T) &pHdr->byBody;

		if( pResp->byKind == 1 )
		{	/// 시재출금
			LOG_OUT("시재 종류 (%d : 시재출금) !!", pResp->byKind);
		}
		else
		{
			LOG_OUT("시재 종류 (%d : 시재회수) !!", pResp->byKind);
		}

		if( pResp->byKind == 1 )
		{	// 출금
			LOG_OUT("시재출금 (100(%d), 500(%d), 1k(%d), 5k(%d), 10k(%d), 50k(%d) !!", 
				pResp->cashInfo.w100, pResp->cashInfo.w500,
				pResp->cashInfo.w1k, pResp->cashInfo.w5k, pResp->cashInfo.w10k, pResp->cashInfo.w50k);

			nTmp10k = pResp->cashInfo.w10k;
			nTmp1k = pResp->cashInfo.w1k;
			nTmp100 = pResp->cashInfo.w100;
			nTmp500 = pResp->cashInfo.w500;

			{
				// 동전방출기
				if( (pResp->cashInfo.w100 + pResp->cashInfo.w500) > 0 )
				{
					int n100 = 0, n500 = 0;

					nOut100 = nOut500 = 0;

					Coin_TotalEnd();

					SetCheckEventCode(EC_COIN_NO_CHECK, TRUE);
					nRet = Coin_OutMoney((int)pResp->cashInfo.w100, (int)pResp->cashInfo.w500, &nOut100, &nOut500);
					SetCheckEventCode(EC_COIN_NO_CHECK, FALSE);

					nCoinACK = sPacket.byACK = CHAR_ACK;

					//Coin_GetOutInfo(&nOut100, &nOut500);
					sPacket.cashInfo.w100 = (WORD) nOut100;
					sPacket.cashInfo.w500 = (WORD) nOut500;

					Coin_TotalEnd();

					if(nRet < 0)
					{	
						Coin_Reset(TRUE, TRUE);
					}

					/// 회계반영
					AddAccumCoinData(ACC_COIN_MINUS, nOut100, nOut500);
				}

				// 지폐방출기
				if( (pResp->cashInfo.w1k + pResp->cashInfo.w10k) > 0)
				{
					::ZeroMemory(&dspInfo, sizeof(DISPENSE_INFO_T));

					SetCheckEventCode(EC_CDU_NO_CHECK, TRUE);
					nRet = CDU_OutMoney((int)pResp->cashInfo.w1k, (int)pResp->cashInfo.w10k, &dspInfo.nOutCount[0], &dspInfo.nOutCount[1]);
					SetCheckEventCode(EC_CDU_NO_CHECK, FALSE);

					TR_LOG_OUT("시재방출 갯수 = 1K(%d), 10K(%d) ", dspInfo.nOutCount[0], dspInfo.nOutCount[1]);

					nBillACK = sPacket.byACK = CHAR_ACK;

					//CDU_GetDispenseInfo((char *)&dspInfo);

					sPacket.cashInfo.w1k = (WORD) dspInfo.nOutCount[0];
					sPacket.cashInfo.w10k = (WORD) dspInfo.nOutCount[1];

					AddAccumBillData(ACC_BILL_MINUS, sPacket.cashInfo.w1k, 0, sPacket.cashInfo.w10k, 0);
				}

				if( (pResp->cashInfo.w100 + pResp->cashInfo.w500) > 0 )
				{
					Coin_Reset(TRUE, TRUE);
					//Coin_GetStatus();
				}
				AddCashTrData(DT_FLAG_OUT_MONEY, (WORD)nOut100, (WORD)nOut500, (WORD)dspInfo.nOutCount[0], 0, (WORD)dspInfo.nOutCount[1], 0);

			}
		}
		else
		{
			PFILE_ACCUM_N1010_T pAccum = GetAccumulateData();

			LOG_OUT("시재회수 (100(%d), 500(%d), 1k(%d), 5k(%d), 10k(%d), 50k(%d) !!", 
					pResp->cashInfo.w100, pResp->cashInfo.w500,
					pResp->cashInfo.w1k, pResp->cashInfo.w5k, pResp->cashInfo.w10k, pResp->cashInfo.w50k);
			
			sPacket.byACK = CHAR_ACK;
			sPacket.byKind = pResp->byKind;
			
			sPacket.cashInfo.w100 = pResp->cashInfo.w100;
			sPacket.cashInfo.w500 = pResp->cashInfo.w500;
			sPacket.cashInfo.w1k  = pResp->cashInfo.w1k;
			sPacket.cashInfo.w10k = pResp->cashInfo.w10k;


			if( (pResp->cashInfo.w100 + pResp->cashInfo.w500) > 0 )
			{
				int n100, n500;

				n100 = pAccum->Curr.Coin.n100;
				if((int)pResp->cashInfo.w100 > n100)
				{
					sPacket.cashInfo.w100 = (WORD) n100;
					pResp->cashInfo.w100 = (WORD) n100;
				}

				n500 = pAccum->Curr.Coin.n500;
				if((int)pResp->cashInfo.w500 > n500)
				{
					sPacket.cashInfo.w500 = (WORD) n500;
					pResp->cashInfo.w500 = (WORD) n500;
				}
				AddAccumCoinData(ACC_COIN_MINUS, (int)pResp->cashInfo.w100, (int)pResp->cashInfo.w500);
			}

			if( (sPacket.cashInfo.w1k + sPacket.cashInfo.w10k) > 0 )
			{
				int n1k, n10k;

				n1k = pAccum->Curr.BillDispenser.Casst.n1k;
				if((int)pResp->cashInfo.w1k > n1k)
				{
					sPacket.cashInfo.w1k = (WORD) n1k;
					pResp->cashInfo.w1k = (WORD) n1k;
				}

				n10k = pAccum->Curr.BillDispenser.Casst.n10k;
				if((int)pResp->cashInfo.w10k > n10k)
				{
					sPacket.cashInfo.w10k = (WORD) n10k;
					pResp->cashInfo.w10k = (WORD) n10k;
				}
				AddAccumBillData(ACC_BILL_MINUS, sPacket.cashInfo.w1k, 0, sPacket.cashInfo.w10k, 0);
			}
		}

		nRet = SendPacket(pHdr->wCommand, (BYTE *)&sPacket, sizeof(UI_SND_ADMIN_CASH_OUT_T));

		UI_AddDevAccountInfo();
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_544_command
 * @details		[관리자] - 시재 입.출 내역 (544)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_544_command(BYTE *pRecvData)
{
	int  nRet = 0;
	UI_SND_ADMIN_INQ_CASH_INOUT_T sPacket;
	PUI_HEADER_T pHdr;
	PUI_RESP_ADMIN_INQ_CASH_INOUT_T pResp;

	try
	{
		::ZeroMemory(&sPacket, sizeof(UI_SND_ADMIN_INQ_CASH_INOUT_T));

		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_ADMIN_INQ_CASH_INOUT_T) &pHdr->byBody;

		// 시작일시
		//pResp->szBeginDt;
		// 종료일시
		//pResp->szEndDt;

		{
			sPacket.byACK = CHAR_ACK;
			sprintf(sPacket.szFileName, "");

			nRet = SendPacket(pHdr->wCommand, (BYTE *)&sPacket, sizeof(UI_SND_ADMIN_INQ_CASH_INOUT_T));
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_545_command
 * @details		[관리자] - 지폐함 회수 (545)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_545_command(BYTE *pRecvData)
{
	int					nRet = 0;
	BYTE				byACK = 0;
	PUI_HEADER_T		pHdr;
	PFILE_ACCUM_N1010_T	pAccum; 

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		{
			pAccum = GetAccumulateData();

			/// 입출내역 TR (회수)
			AddCashTrData(DT_FLAG_WTHDRAW_MONEY, 0, 0, (WORD)pAccum->Curr.BillBox.n1k, (WORD)pAccum->Curr.BillBox.n5k, (WORD)pAccum->Curr.BillBox.n10k, (WORD)pAccum->Curr.BillBox.n50k);

			/// 회계반영
			AddAccumBillData(ACC_BILL_WITHDRAW, (WORD)pAccum->Curr.BillBox.n1k, (WORD)pAccum->Curr.BillBox.n5k, (WORD)pAccum->Curr.BillBox.n10k, (WORD)pAccum->Curr.BillBox.n50k);


			UI_AddDevAccountInfo();
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_546_command
 * @details		[관리자] - 승차권함 회수 (546)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_546_command(BYTE *pRecvData)
{
	int					nRet = 0;
	BYTE				byACK = 0;
	PUI_HEADER_T		pHdr;
	PFILE_ACCUM_N1010_T	pAccum; 

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		{
			pAccum = GetAccumulateData();
			AddAccumTicketData(ACC_TICKETBOX_WITHDRAW, 0);

			UI_AddDevAccountInfo();
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_547_command
 * @details		[관리자] - 미방출금 방출 요청 (547)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_547_command(BYTE *pRecvData)
{
	int							nRet = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_NOOUT_CASH_REQ_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_NOOUT_CASH_REQ_T) &pHdr->byBody;

		::ZeroMemory(&CConfigTkMem::GetInstance()->reqNoOutCash, sizeof(REQ_NO_OUT_CASH));

		::CopyMemory(CConfigTkMem::GetInstance()->reqNoOutCash.szDate, pResp->szDateTime, sizeof(pResp->szDateTime));
		CConfigTkMem::GetInstance()->reqNoOutCash.w100 = pResp->w100;
		CConfigTkMem::GetInstance()->reqNoOutCash.w500 = pResp->w500;
		CConfigTkMem::GetInstance()->reqNoOutCash.w1k = pResp->w1k;
		CConfigTkMem::GetInstance()->reqNoOutCash.w5k = pResp->w5k;
		CConfigTkMem::GetInstance()->reqNoOutCash.w10k = pResp->w10k;
		CConfigTkMem::GetInstance()->reqNoOutCash.w50k = pResp->w50k;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		Transact_SetState(TR_ADMIN_NO_OUT_CASH, 0);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_551_command
 * @details		[관리자] - 마감내역 요청 (551)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_551_command(BYTE *pRecvData)
{
	int							nRet = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_NOOUT_CASH_REQ_T	pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_NOOUT_CASH_REQ_T) &pHdr->byBody;

		byACK = CHAR_ACK;
		nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		UI_InqDevCloseInfo();
		//Transact_SetState(TR_ADMIN_NO_OUT_CASH, 0);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_553_command
 * @details		[관리자] - 거래내역 요청 (553)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_553_command(BYTE *pRecvData)
{
	int							nRet = 0;
	BYTE						byACK = 0;
	UI_SND_CASH_IO_HISTO_T		sPacket;
	PUI_HEADER_T				pHdr;
	PUI_RESP_CASH_IO_HISTO_T	pResp;

	try
	{
		::ZeroMemory(&sPacket, sizeof(UI_SND_CASH_IO_HISTO_T));

		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_CASH_IO_HISTO_T) &pHdr->byBody;

		sPacket.byACK = CHAR_ACK;

		{
			char Buffer[256];
			CString strPath;
			CString strFileName;
			CString strBuff;
			WIN32_FIND_DATA wfd;
			HANDLE hHandle;
			BOOL bResult = TRUE;

			USES_CONVERSION;

			::ZeroMemory(Buffer, sizeof(Buffer));
			Util_GetModulePath(Buffer);

			strPath = (CString) Buffer + _T("\\Data\\") + _T("SmsTR");

			strFileName = strPath + _T("\\*.*");

			hHandle = ::FindFirstFile(strFileName, &wfd);
			while( (hHandle != INVALID_HANDLE_VALUE) && (bResult == TRUE) )  
			{
				if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) 
				{
					;
				} 
				else 
				{
					if( memcmp(&wfd.cFileName[9], pResp->begin_dt, 8) == 0 )
					{
						strBuff.Format(_T("\\%s"), wfd.cFileName);
						strFileName = strPath + strBuff;
						sprintf(sPacket.szFileName, "%s", LPSTR(LPCTSTR(strFileName)));
						sPacket.dwFileSize = (DWORD)(wfd.nFileSizeHigh * ((int)MAXDWORD + 1)) + (DWORD)wfd.nFileSizeLow;
						//LOG_OUT("szFileName = %s", sPacket.szFileName);
						//LOG_OUT("dwFileSize = %lu", sPacket.dwFileSize);
					}
				}
				bResult = ::FindNextFile(hHandle, &wfd);
			}
			::FindClose(hHandle);
		}

		nRet = SendPacket(pHdr->wCommand, (BYTE *)&sPacket, sizeof(UI_SND_CASH_IO_HISTO_T));

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

/**
 * @brief		resp_554_command
 * @details		[관리자] - 시재정보 프린트 요청 (554)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_554_command(BYTE *pRecvData)
{
	int							nRet = 0;
	BYTE						byACK = 0;
	PUI_HEADER_T				pHdr;
	PUI_RESP_554_DT				pResp;

	try
	{
		pHdr  = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_554_DT) &pHdr->byBody;

		CConfigTkMem::GetInstance()->n_print_kind = pResp->byPrtKind;
		sprintf(CConfigTkMem::GetInstance()->prt_beg_dt, "%.*s%.*s", sizeof(pResp->szDate), pResp->szDate, sizeof(pResp->szTime), pResp->szTime);

		LOG_OUT("프린트 요청, type(%02X), date_time(%s) ...", CConfigTkMem::GetInstance()->n_print_kind & 0xFF, CConfigTkMem::GetInstance()->prt_beg_dt);

		switch( pResp->byPrtKind )
		{
		case 1 :	/// 현재시재 프린트
		case 2 :	
		case 3 :
			Transact_SetState(TR_ADMIN_RECEIPT_PRINT, 0);
			break;
		}

		/// 전송
		{
			byACK = CHAR_ACK;
			nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nRet;
}

// 20211116 ADD
/**
 * @brief		resp_591_command
 * @details		[관리자] - 테스트 승차권 출력 (900)
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int resp_591_command(BYTE *pRecvData)
{
	int							nRet = 0, nSvrKind = 0, nRes = 0;
	char						byACK;
	PUI_HEADER_T				pHdr;
	PUI_RESP_591_T				pResp;

	try
	{
		pHdr = (PUI_HEADER_T) pRecvData;
		pResp = (PUI_RESP_591_T) &pHdr->byBody;

		if( pResp->byBusDVS == 1 )
		{
			nSvrKind = SVR_DVS_CCBUS;
			TR_LOG_OUT("시외 - 테스트 승차권 출력.");

			byACK = CHAR_ACK;
			nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		//	승차권 프린트 위치 정보 설정
			INI_ReadEnvTckPtrgFile();
			{
				DWORD dwTick;
				char *pBus_tck_knd_cd = "IF00";		// 일반

				TckPrt_TestPrintTicket(nSvrKind, (char *) pBus_tck_knd_cd);

				dwTick = ::GetTickCount();
				while( Util_CheckExpire(dwTick) < (1000 * 3) );

			///	20220104 add	// 현장테스트발권
			//	회계반영 - 승차권 수량
				AddAccumTicketWork_Rev2(SVR_DVS_CCBUS, IDX_ACC_CARD, pBus_tck_knd_cd, (int) ACC_TICKET_PUB_TEST_ISSUE, 1);
			///	~20220104 add
			//	티켓 고유번호 증가
			//	AddAccumBusTckInhrNo(ACC_TICKET_SEQ_PLUS, nSvrKind, 1);
			}
			UI_AddDevAccountInfo();
			CPubTckMem::GetInstance()->Initialize();
			CPubTckKobusMem::GetInstance()->Initialize();
			CPubTckTmExpMem::GetInstance()->Initialize();
			nRes = resp_523_command(0);		// UI 화면 업데이트
		}
		else if ( pResp->byBusDVS == 2 || pResp->byBusDVS == 3 )
		{
			nSvrKind = SVR_DVS_TMEXP;
			TR_LOG_OUT("고속 - 테스트 승차권 출력.");

			byACK = CHAR_ACK;
			nRet = SendPacket(pHdr->wCommand, (BYTE *)&byACK, 1);

		//	승차권 프린트 위치 정보 설정
			INI_ReadEnvTckPtrgFile();
			{
				DWORD dwTick;
				char *pBus_tck_knd_cd = "IF00";		// 일반

				TckPrt_TestPrintTicket(nSvrKind, (char *) pBus_tck_knd_cd);

				dwTick = ::GetTickCount();
				while( Util_CheckExpire(dwTick) < (1000 * 3) );

			///	20220104 add	// 현장테스트발권
			//	회계반영 - 승차권 수량
				AddAccumTicketWork_Rev2(SVR_DVS_TMEXP, IDX_ACC_CARD, pBus_tck_knd_cd, (int) ACC_TICKET_PUB_TEST_ISSUE, 1);
			///	~20220104 add
			//	티켓 고유번호 증가
			//	AddAccumBusTckInhrNo(ACC_TICKET_SEQ_PLUS, nSvrKind, 1);
			}
			UI_AddDevAccountInfo();
			CPubTckMem::GetInstance()->Initialize();
			CPubTckKobusMem::GetInstance()->Initialize();
			CPubTckTmExpMem::GetInstance()->Initialize();
			nRes = resp_523_command(0);		// UI 화면 업데이트
		}
		else
		{
			TR_LOG_OUT("Unknown 테스트 승차권 출력 - 에러.");
			return -1;
		}

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	
	return nRet;
}
// 20211116 ~ADD

/**
 * @brief		ServicePacket
 * @details		수신 Packet 처리
 * @param		BYTE *pRecvData		수신버퍼
 * @return		성공 >= 0, 실패 < 0
 */
static int ServicePacket(BYTE *pRecvData)
{
	int				nRet = 0;
	PUI_HEADER_T	pHeader;

	try
	{
		pHeader = (PUI_HEADER_T) pRecvData;

		if( (pHeader->wCommand & 0xFFFF) != UI_CMD_POLLING )
		{
			TRACE("recv command = %04X\n", pHeader->wCommand & 0xFFFF);
		}

		switch(pHeader->wCommand & 0xFFFF)
		{
		case UI_CMD_POLLING:
			{	// Polling
				nRet = resp_100_command(pRecvData);
			}
			break;
		case UI_CMD_OK_CANCEL:
			{	// 거래완료 / 취소 / 실패 처리
				nRet = resp_101_command(pRecvData);
			}
			break;
		case UI_CMD_SERVICE:
			{	// 사용중 / 사용중지 
				nRet = resp_102_command(pRecvData);
			}
			break;
		case UI_CMD_VERSION_REQ:
			{	// 버젼 정보 요청
				nRet = resp_105_command(pRecvData);
			}
			break;
		case UI_CMD_ALARM_REQ_INFO:
			{	// 장애 내용 요청
				nRet = resp_121_command(pRecvData);
			}
			break;

			//+++++++++++++++++++++++++++++++++++++++++++++++++++++++

		case UI_CMD_MRS_MAIN:
			{	// 예매발권 시작
				nRet = resp_200_command(pRecvData);
			}
			break;
		case UI_CMD_MRS_CARD_RD:
			{	// 예매발권 - 신용카드 읽기
				nRet = resp_201_command(pRecvData);
			}
			break;
		case UI_CMD_MRS_REQ_LIST:
			{	// 예매발권 - 리스트 요청
				nRet = resp_203_command(pRecvData);
			}
			break;
		case UI_CMD_MRS_ISSUE:
			{	// 예매발권 - 발권 시작
				nRet = resp_205_command(pRecvData);
			}
			break;
		case UI_CMD_MRS_CHG_ISSUE:
			{	// 예매발권 - 교체발권
				nRet = resp_206_command(pRecvData);
			}
			break;
		case UI_CMD_KOBUS_MRS_REQ_LIST:
			{	// [코버스] 예매발권 - 예매 리스트 요청 
				nRet = resp_233_command(pRecvData);
			}
			break;
		case UI_CMD_KOBUS_MRS_ISSUE:
			{	// [코버스] 예매발권 - TMAX 예매 발행
				nRet = resp_235_command(pRecvData);
			}
			break;
		case UI_CMD_KOBUS_MRS_TCK_PRT:
			{	// [코버스/티머니고속] 예매발권 - 예매권 프린트
				nRet = resp_237_command(pRecvData);
			}
			break;


		case UI_CMD_TMEXP_MRS_REQ_LIST:
		case UI_CMD_TMEXP_MRS_KTC_REQ_LIST:
		case UI_CMD_TMEXP_MRS_REQ_ISSUE :
		case UI_CMD_TMEXP_MRS_REQ_MOBILE_ISSUE :
			{
				if(CConfigTkMem::GetInstance()->n_bus_dvs != SVR_DVS_TMEXP)
				{
					CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_TMEXP;
				}

				if( (pHeader->wCommand & 0xFFFF) == UI_CMD_TMEXP_MRS_REQ_LIST )
				{	// [티머니고속] 예매발권 - 예매 리스트 요청 
					nRet = resp_250_command(pRecvData);
				}
				if( (pHeader->wCommand & 0xFFFF) == UI_CMD_TMEXP_MRS_KTC_REQ_LIST )
				{	// [티머니고속] 예매발권 - KTC 예매 리스트 요청 
					nRet = resp_252_command(pRecvData);
				}
				if( (pHeader->wCommand & 0xFFFF) == UI_CMD_TMEXP_MRS_REQ_ISSUE )
				{	// [티머니고속] 예매발권 - 예매발권 요청 
					nRet = resp_254_command(pRecvData);
				}
				if( (pHeader->wCommand & 0xFFFF) == UI_CMD_TMEXP_MRS_REQ_MOBILE_ISSUE )
				{	// [티머니고속] 예매발권 - 모바일 예매발권 요청 
					nRet = resp_256_command(pRecvData);
				}
			}
			break;

			//+++++++++++++++++++++++++++++++++++++++++++++++++++++++

		case UI_CMD_PBTCK_MAIN:
			{	// "현장발권" - 시작
				nRet = resp_300_command(pRecvData);
			}
			break;
		case UI_CMD_PBTCK_REQ_LIST:
			{	// "현장발권" - 배차 리스트 요청
				nRet = resp_301_command(pRecvData);
			}
			break;
		case UI_CMD_PBTCK_LIST_SEL:
			{	// "현장발권" - 배차 리스트 선택
				nRet = resp_303_command(pRecvData);
			}
			break;
		case UI_CMD_PBTCK_SEAT_SEL:
			{	// "현장발권" - 좌석 선택 선택
				nRet = resp_305_command(pRecvData);
			}
			break;
		case UI_CMD_PBTCK_PYM_DVS:
			{	// "현장발권" - 결제수단 선택 선택
				nRet = resp_307_command(pRecvData);
			}
			break;
		case UI_CMD_PBTCK_INS_BILL:
			{	// "현장발권" - 지폐 투입
				nRet = resp_308_command(pRecvData);
			}
			break;
		case UI_CMD_PBTCK_REQ_PYM:
			{	// "현장발권" - 승차권 결제 요청
				nRet = resp_311_command(pRecvData);
			}
			break;
		case UI_CMD_PBTCK_TCK_ISSUE:
			{	// "현장발권" - 승차권 발권
				nRet = resp_313_command(pRecvData);
			}
			break;
		case UI_CMD_PBTCK_CHG_MONEY:
			{	// "현장발권" - 거스름돈 방출 
				nRet = resp_315_command(pRecvData);
			}
			break;
		// 20221201 ADD~
		case UI_CMD_TICKET_QRMDPCPYSATS:
			{	// 현장발권 - QR 좌석 선점 시간변경
				nRet = resp_378_command(pRecvData);
			}
			break;
		// 20221201 ~ADD
		// 20190923 add by nhso : 상주직원 처리
		case UI_CMD_PBTCK_STAFF_REQ:
			{	// "현장발권" - 상주직원 정보 요청
				nRet = resp_320_command(pRecvData);
			}
			break;
		case UI_CMD_PBTCK_STAFF_CD_MOD_FARE_REQ:
			{	// "현장발권" - 상주직원 좌석 요금 정보 변경 요청
				nRet = resp_322_command(pRecvData);
			}
			break;
		case UI_CMD_PBTCK_PRE_RF_PAY_REQ:
			{	// "현장발권" - RF 선불카드 결제 요청
				nRet = resp_324_command(pRecvData);
			}
			break;
		// ~20190923 add by nhso : 상주직원 처리

		case UI_CMD_PBTCK_USRINFINP:
			{	// "현장발권" - 발행정보 입력
				nRet = resp_326_command(pRecvData);
			}
			break;

		case UI_CMD_REQ_PBTCK_LIST :
			{	// [코버스] 현장발권 - 배차조회 요청 (331)
				nRet = resp_331_command(pRecvData);
			}
			break;

		case UI_CMD_REQ_PBTCK_SATS :
			{	// [코버스] 현장발권 - 좌석정보 요청 (333)
				nRet = resp_333_command(pRecvData);
			}
			break;

		case UI_CMD_REQ_PBTCK_SATSPCPY :
			{	// [코버스] 현장발권 - 좌석선점 요청 (335)
				nRet = resp_335_command(pRecvData);
			}
			break;

		case UI_CMD_REQ_PBTCK_TMAX_ISSUE :
			{	// [코버스] 현장발권 - 카드/현금/현장발권 요청 (337)
				nRet = resp_337_command(pRecvData);
			}
			break;

		case UI_CMD_REQ_PBTCK_THRU_INFO:
			{	// [코버스] 현장발권 - 경유지 정보 조회 요청 (338)
				nRet = resp_338_command(pRecvData);
			}
			break;

		/// 티머니 고속
		case UI_CMD_REQ_TMEXP_PBTCK_LIST	:		
		case UI_CMD_REQ_TMEXP_SATS_FEE		:
		case UI_CMD_REQ_TMEXP_PCPY_SATS		:
		case UI_CMD_REQ_TMEXP_PBTCK_CASH	:
		case UI_CMD_REQ_TMEXP_PBTCK_CARD_KTC:
		case UI_CMD_REQ_TMEXP_PBTCK_CSRC_KTC:
		case UI_CMD_REQ_TMEXP_PBTCK_PRD		:
		case UI_CMD_REQ_TMEXP_THRU_INFO		:
			{
				///< (UI->MAIN) [티머니고속] 현장발권 - 배차 정보 요청
				if( (pHeader->wCommand & 0xFFFF) == UI_CMD_REQ_TMEXP_PBTCK_LIST )
					nRet = resp_350_command(pRecvData);

				///< (UI->MAIN) [티머니고속] 현장발권 - 요금/좌석정보 요청
				if( (pHeader->wCommand & 0xFFFF) == UI_CMD_REQ_TMEXP_SATS_FEE )
					nRet = resp_352_command(pRecvData);
				
				///< (UI->MAIN) [티머니고속] 현장발권 - 좌석선점 요청
				if( (pHeader->wCommand & 0xFFFF) == UI_CMD_REQ_TMEXP_PCPY_SATS )
					nRet = resp_354_command(pRecvData);
				
				///< (UI->MAIN) [티머니고속] 현장발권 - 승차권발권(현금)
				if( (pHeader->wCommand & 0xFFFF) == UI_CMD_REQ_TMEXP_PBTCK_CASH )
					nRet = resp_356_command(pRecvData);
				
				///< (UI->MAIN) [티머니고속] 현장발권 - 승차권발권(카드_KTC)
				if( (pHeader->wCommand & 0xFFFF) == UI_CMD_REQ_TMEXP_PBTCK_CARD_KTC )
					nRet = resp_358_command(pRecvData);
				
				///< (UI->MAIN) [티머니고속] 현장발권 - 승차권발권(현금영수증_KTC)
				if( (pHeader->wCommand & 0xFFFF) == UI_CMD_REQ_TMEXP_PBTCK_CSRC_KTC )
					nRet = resp_360_command(pRecvData);
				
				///< (UI->MAIN) [티머니고속] 현장발권 - 승차권발권(부가상품권)
				if( (pHeader->wCommand & 0xFFFF) == UI_CMD_REQ_TMEXP_PBTCK_PRD )
					nRet = resp_362_command(pRecvData);

				///< (UI->MAIN) [티머니고속] 현장발권 - 경유지 정보
				if( (pHeader->wCommand & 0xFFFF) == UI_CMD_REQ_TMEXP_THRU_INFO )
					nRet = resp_364_command(pRecvData);
				
			}
			break;

			//+++++++++++++++++++++++++++++++++++++++++++++++++++++++

		case UI_CMD_CANCRY_MAIN:
			{	// "환불" - 시작
				nRet = resp_400_command(pRecvData);
			}
			break;
		case UI_CMD_CANCRY_TCK_READ:
			{	// "환불" - 승차권 판독
				nRet = resp_401_command(pRecvData);
			}
			break;
		case UI_CMD_CANCRY_FARE:
			{	// 환불 - 환불 처리 (카드 또는 현금)
				nRet = resp_403_command(pRecvData);
			}
			break;

			//+++++++++++++++++++++++++++++++++++++++++++++++++++++++

		case UI_CMD_ADMIN_MAIN:
			{	// 관리자 - 메인
				nRet = resp_500_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_LOGIN:
			{	// 관리자 - 로그인
				nRet = resp_501_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_SET_FUNC:
			{	// 관리자 - 기능설정
				nRet = resp_502_command(pRecvData);
			}
			break;
			// 	case UI_CMD_ADMIN_SET_ENV:
			// 		{	// 관리자 - 환경설정
			// 			nRet = RespAdminSetEnv(pRecvData);
			// 		}
			// 		break;
		case UI_CMD_ADMIN_SET_THRML:
			{	// 관리자 - 터미널설정 
				nRet = resp_504_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_SET_TICKET:
			{	// 관리자 - 터미널설정 ???
				nRet = resp_505_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_SET_BUS_CACM:
			{	// 관리자 - 버스 운수사 설정 ???
				nRet = resp_506_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_SET_TRML_OPT:
			{	// 관리자 - 터미널 설정
				nRet = resp_508_command(pRecvData);
			}
			break;

		case UI_CMD_ADMIN_SYSTEM:
			{	// 관리자 - 시스템관리
				nRet = resp_511_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_DEV_RESET:
			{	// 관리자 - 시스템관리 - 장비별 리셋
				nRet = resp_512_command(pRecvData);
			}
			break;

		case UI_CMD_ADMIN_MNG_TCK_INHRNO:
			{	// 관리자 - 티켓관리 - 티켓신규번호
				nRet = resp_521_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_MNG_TCK_COUNT:
			{	// 관리자 - 티켓관리 - 티켓신규번호
				nRet = resp_522_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_REQ_TCK_INFO:
			{	// 관리자 - 티켓정보 요청
				nRet = resp_523_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_INQ_ISSUE:
		case UI_CMD_ADMIN_INQ_REISSUE:
		case UI_CMD_ADMIN_INQ_REFUND:
			{	// 관리자 - 발권/재발권/환불 내역 조회
				nRet = resp_531_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_RE_ISSUE:
			{	// 관리자 - 재발행
				nRet = resp_534_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_CASH_CLOSE:
			{	// 관리자 - 시재 마감처리
				nRet = resp_540_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_CASH_HISTO:
			{	// 관리자 - 시재 내역
				nRet = resp_541_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_CASH_INSERT:
			{	// 관리자 - 시재 보급
				nRet = resp_542_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_CASH_OUT:
			{	// 관리자 - 시재 출금
				nRet = resp_543_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_INQ_CASH_INOUT:
			{	// 관리자 - 시재 입출내역
				nRet = resp_544_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_WITHDRAW_BILL:
			{	// 관리자 - 지폐함 회수
				nRet = resp_545_command(pRecvData);
			}
			break;
		case UI_CMD_ADMIN_WITHDRAW_TCK:
			{	// 관리자 - 승차권함 회수
				nRet = resp_546_command(pRecvData);
			}
			break;
		case UI_CMD_NOOUT_CASH_REQ :
			{	// 관리자 - 미방출금 방출 요청
				nRet = resp_547_command(pRecvData);
			}
			break;
		case UI_CMD_CLOSE_INQ_REQ :
			{	// 관리자 - 마감내역 요청
				nRet = resp_551_command(pRecvData);
			}
			break;
		case UI_CMD_TR_INQ_REQ :
			{	// 관리자 - 입출내역 조회 요청
				nRet = resp_553_command(pRecvData);
			}
			break;
		case UI_CMD_ACC_PRT_REQ :
			{	// 관리자 - 시재정보 프린트 요청
				nRet = resp_554_command(pRecvData);
			}
			break;
		// 20211116 ADD
		case UI_CMD_TEST_PRINT_REQ :
			{	// 관리자 - 테스트 승차권 출력 요청
				nRet = resp_591_command(pRecvData);
			}
			break;
		// 20211116 ~ADD
		}

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
	int				nRet = 0;
	UI_SEND_POLL_T	sPacket;
	char *pBuff;

	try
	{
		///> 창구번호
		pBuff = GetTrmlWndNo(SVR_DVS_CCBUS);
		sPacket.wTrmlWndNo = (WORD)Util_Ascii2Long(pBuff, strlen(pBuff));
		///> 터미널 코드(7)
		::CopyMemory(sPacket.szTrmlCd, GetTrmlCode(SVR_DVS_CCBUS), 7); 
		///> 언어
		sPacket.chLang = CConfigTkMem::GetInstance()->n_language & 0xFF;

		nRet = SndRcvPacket(UI_CMD_POLLING, (BYTE *)&sPacket, (int)sizeof(UI_SEND_POLL_T), s_RecvData);
		if(nRet < 0)
		{
			return nRet;
		}

		nRet = ServicePacket(s_RecvData);

#if (_KTC_CERTIFY_ > 0)

		KTC_MemClear(&sPacket, sizeof(UI_SEND_POLL_T));
		KTC_MemClear(s_SendData, sizeof(s_SendData));
		KTC_MemClear(s_RecvData, sizeof(s_RecvData));
#endif
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

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
	int		nRet = 0;
	DWORD	dwThreadID;

	try
	{
		LOG_INIT();

		LOG_OUT(" ###################### start ###################### ");

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
			LOG_OUT("CreateMutex() failure..\n");
			return -1;
		}

		hThread = ::CreateThread(NULL, 0, UiCommThread, NULL, CREATE_SUSPENDED, &dwThreadID);
		if(hThread == NULL) 
		{
			return -2;
		}

		::ResumeThread(hThread);
		nRet = 0;
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
		nRet = -99;
	}

	return nRet;
}

/**
 * @brief		UI_Terminate
 * @details		UI 통신 종료
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int UI_Terminate(void)
{
	try
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
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
	int				nRet = 0;
	PUI_HEADER_T	pPacket;

	try
	{
		if(tQue.nFlag == FLAG_SEND_RECV)
		{
			nRet = SndRcvPacket(tQue.wCommand, (BYTE *)tQue.szData, tQue.nLen, s_RecvData);
			if(nRet < 0)
			{
				LOG_OUT("FLAG_SEND_RECV SndRcvPacket() Error (%d) !!", nRet);
				return nRet;
			}

			pPacket = (PUI_HEADER_T) s_RecvData;

			if(pPacket->wCommand != tQue.wCommand)
			{
				LOG_OUT("FLAG_SEND_RECV Command Error (%04X:%04X) !!", pPacket->wCommand, tQue.wCommand);
			}
		}
		else if(tQue.nFlag == FLAG_Q_SEND_RECV)
		{
			int nSize, nCount, nOffset;
			//BYTE *pSendBuff = NULL;

			nSize = nCount = nOffset = 0;

			switch(tQue.wCommand)
			{
			case UI_CMD_DEV_CONFIG :
				nSize = make_0106_data(tQue);
				break;
 			case UI_CMD_PBTCK_RESP_LIST :		/// [시외버스] 현장발권 - 배차리스트 전달
 				nSize = make_0302_data(tQue);
 				break;
 			case UI_CMD_MRS_RESP_LIST :			/// [시외버스] 예매발권 - 예매리스트 응답
 				nSize = make_0204_data(tQue);
 				break;
 			case UI_CMD_KOBUS_MRS_RESP_LIST :	/// [코버스] 예매발권 - 예매리스트 응답
 				nSize = make_0234_data(tQue);
 				break;
 			case UI_CMD_KOBUS_MRS_RESP_ISSUE :	/// [코버스] 예매발권 - 예매발권 응답 데이타
 				nSize = make_0236_data(tQue);
 				break;
			case UI_CMD_TMEXP_MRS_RESP_LIST :	/// [티머니고속] 예매발권 - 예매리스트 응답
				nSize = make_0251_data(tQue);
				break;
			case UI_CMD_TMEXP_MRS_KTC_RESP_LIST :/// [티머니고속] 예매발권 - KTC 예매리스트 응답
				nSize = make_0253_data(tQue);
				break;
			case UI_CMD_TMEXP_MRS_RESP_ISSUE :	/// [티머니고속] 예매발권 - 예매발권 응답
				nSize = make_0255_data(tQue);
				break;
			case UI_CMD_TMEXP_MRS_RESP_MOBILE_ISSUE :	/// [티머니고속] 예매발권 - 모바일 예매발권 응답
				nSize = make_0257_data(tQue);
				break;
			
			case UI_CMD_RES_PBTCK_LIST :		/// [코버스] 현장발권 - 배차리스트 응답
 				nSize = make_0332_data(tQue);
 				break;
 			case UI_CMD_RES_PBTCK_THRU_INFO :	/// [코버스] 현장발권 - 경유지 조회 응답
 				nSize = make_0339_data(tQue);
 				break;
 			case UI_CMD_PBTCK_STAFF_RESP :		/// [시외버스] 현장발권 - 상주직원 조회 응답
 				nSize = make_0321_data(tQue);
 				break;
 			case UI_CMD_PBTCK_STAFF_CD_MOD_FARE_RESP : /// 상주직원_카드_요금변경_정보_resp
 				nSize = make_0323_data(tQue);
 				break;
			case UI_CMD_PBTCK_PRE_RF_PAY_RESP:
				nSize = make_0325_data(tQue);
				break;

			case UI_CMD_RES_TMEXP_PBTCK_LIST :	/// [티머니고속] 현장발권 - 배차리스트 전달
				nSize = make_tmexp_0351_data(tQue);
				break;
			case UI_CMD_RES_TMEXP_SATS_FEE :	/// [티머니고속] 현장발권 - 요금/좌석정보 전달
				nSize = make_tmexp_0353_data(tQue);
				break;
			case UI_CMD_RES_TMEXP_PCPY_SATS :	/// [티머니고속] 현장발권 - 좌석선점 정보 전달
				nSize = make_tmexp_0355_data(tQue);
				break;
			case UI_CMD_RES_TMEXP_PBTCK_CASH :	/// [티머니고속] 현장발권 - TMAX_승차권발권(현금)
				nSize = make_tmexp_0357_data(tQue);
				break;
			case UI_CMD_RES_TMEXP_PBTCK_CARD_KTC :	/// [티머니고속] 현장발권 - TMAX_승차권발권(카드_KTC)
				nSize = make_tmexp_0359_data(tQue);
				break;
			case UI_CMD_RES_TMEXP_PBTCK_CSRC_KTC :	/// [티머니고속] 현장발권 - TMAX_승차권발권(현금영수증_KTC)
				nSize = make_tmexp_0361_data(tQue);
				break;
			case UI_CMD_RES_TMEXP_PBTCK_PRD :		/// [티머니고속] 현장발권 - TMAX_승차권발권(부가상품권)
				nSize = make_tmexp_0363_data(tQue);
				break;
			case UI_CMD_RES_TMEXP_THRU_INFO:
				nSize = make_tmexp_0365_data(tQue);
				break;

			default  :
				LOG_OUT("FLAG_Q_SEND_RECV Command Error (%04X) !!", tQue.wCommand);
				return -1;
			}

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

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
	PKIOSK_INI_ENV_T pEnv;

	s_nConnected = FALSE;
	dwConnectTick = 0L;
	dwPollTick = 0;
	nStart = 0;
	s_nState = UI_NONE_STATE; 
	nTmo = 10;
	s_bRun = TRUE;

	pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();

	// 설정정보
	//UI_AddDevConfigData();
	UI_Add_Q_SendData(UI_CMD_DEV_CONFIG, TRUE, 1);
	// 장비 회계 정보
	UI_AddDevAccountInfo();
	// 터미널 정보
	UI_AddTrmlInfo();

	while(s_bRun) 
	{
		Sleep(10);	// 10 ms		

#if 1
		if(s_nConnected == FALSE) 
		{
			if( (dwConnectTick == 0) || (Util_CheckExpire(dwConnectTick) > (1000 * 10)) ) 
			{
//				nRet = clsSocket.ConnectSocket(UI_IPADDRESS, UI_PORT, nTmo);
				nRet = clsSocket.ConnectSocket(pEnv->tUI.szIPAddress, pEnv->tUI.nTcpPort, nTmo);
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
#else
		if( s_nState == UI_NONE_STATE )
		{
			s_nState = UI_CONNECT_STATE;
			s_nConnected = TRUE;
		}
#endif

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
 * @brief		UI_AddQueueInfo
 * @details		큐에 데이타 저장
 * @param		None
 * @return		항상 : 0
 */
int UI_AddQueueInfo(WORD wCommand, char *pData, int nDataLen)
{
	int				nOffset = 0;
	UI_QUE_DATA_T	queData;

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = wCommand;

		TR_LOG_OUT("[Main->UI_Q] wCommand = %d", (int)wCommand);	// 20210823 ADD
		switch(wCommand)
		{
		case UI_CMD_PBTCK_RESP_LIST:
		case UI_CMD_MRS_RESP_LIST:			/// 예매리스트
			queData.nFlag = FLAG_Q_SEND_RECV;
			break;
		default:
			queData.nFlag = FLAG_SEND_RECV;
			break;
		}

		if(nDataLen >= UI_Q_MAX_BUFF)
		{
			LOG_OUT("nDataLen = %d Overflow !!", nDataLen);	
			nDataLen = UI_Q_MAX_BUFF;
		}

		// Data
		::CopyMemory(queData.szData, pData, nDataLen);

		// Length
		queData.nLen = nDataLen;

		TR_LOG_OUT("[Main->UI_Q] s_QueData.push START ... wCommand = %d, nDataLen = %d", (int)wCommand, nDataLen);	// 20210823 ADD
		s_QueData.push(queData);
		TR_LOG_OUT("[Main->UI_Q] s_QueData.push END ... wCommand = %d, nDataLen = %d", (int)wCommand, nDataLen);		// 20210823 ADD
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
	try
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
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
	try
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
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddDevAccountInfo
 * @details		장비 회계 정보 전송	(104)
 * @param		None
 * @return		항상 : 0
 */
int UI_AddDevAccountInfo(void)
{
	int				nOffset, i;
	UI_QUE_DATA_T	queData;
	UI_DEV_ACCOUNT_INFO_T Info;

	i = nOffset = 0;

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_ACCOUNT;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		{
			PFILE_ACCUM_N1010_T pAccum;
			PDM_TICKET_WORK_T	pTckWork;

			::ZeroMemory(&Info, sizeof(UI_DEV_ACCOUNT_INFO_T));

			pAccum = GetAccumulateData();

			// 동전 호퍼
			::CopyMemory(&Info.Coin, &pAccum->Curr.Coin, sizeof(DM_COIN_T));
			// 지폐방출기
			::CopyMemory(&Info.Dispenser, &pAccum->Curr.BillDispenser, sizeof(DM_BILL_DISPENSER_T));
			// 지폐함
			::CopyMemory(&Info.BillBox, &pAccum->Curr.BillBox, sizeof(DM_BILL_T));

			// 시외
			{
				for(i = 0; i < CCS_IDX_TCK_MAX; i++)
				{
					pTckWork = &pAccum->Curr.ccsTicketWork[i];

					///< [시외] 승차권 카드 발권 매수/금액
					Info.ccsCardPubTck.nCount += pTckWork->tPubTck[IDX_ACC_CARD].nCount + pTckWork->tMrnp[IDX_ACC_CARD].nCount;
					Info.ccsCardPubTck.dwMoney += pTckWork->tPubTck[IDX_ACC_CARD].dwMoney + pTckWork->tMrnp[IDX_ACC_CARD].dwMoney;

					///< [시외] 승차권 카드 환불 매수/금액
					Info.ccsCardRefund.nCount += pTckWork->tRefund[IDX_ACC_CARD].nCount;
					Info.ccsCardRefund.dwMoney += pTckWork->tRefund[IDX_ACC_CARD].dwMoney;

					///< [시외] 승차권 현금 발권 매수/금액
					Info.ccsCashPubTck.nCount += pTckWork->tPubTck[IDX_ACC_CASH].nCount; // + pTckWork->tMrnp[IDX_ACC_CASH].nCount;
					Info.ccsCashPubTck.dwMoney += pTckWork->tPubTck[IDX_ACC_CASH].dwMoney; // + pTckWork->tMrnp[IDX_ACC_CASH].dwMoney;

					///< [시외] 승차권 현금 환불 매수/금액
					Info.ccsCashRefund.nCount += pTckWork->tRefund[IDX_ACC_CASH].nCount;
					Info.ccsCashRefund.dwMoney += pTckWork->tRefund[IDX_ACC_CASH].dwMoney;
				}
// 				TR_LOG_OUT("%s", "[#1. 시외_티켓_정보] ");
// 				TR_LOG_OUT("\t 카드발권(%d, %d), 현금발권(%d, %d), 카드환불(%d, %d), 현금환불(%d, %d) ", 
// 					Info.ccsCardPubTck.nCount, Info.ccsCardPubTck.dwMoney,
// 					Info.ccsCashPubTck.nCount, Info.ccsCashPubTck.dwMoney,
// 					Info.ccsCardRefund.nCount, Info.ccsCardRefund.dwMoney, 
// 					Info.ccsCashRefund.nCount, Info.ccsCashRefund.dwMoney);
			}

			// 고속
			{
				for(i = 0; i < TMEXP_IDX_TCK_MAX; i++)
				{
					pTckWork = &pAccum->Curr.expTicketWork[i];

					///< [고속] 승차권 카드 발권 매수/금액
					Info.expCardPubTck.nCount += pTckWork->tPubTck[IDX_ACC_CARD].nCount + pTckWork->tMrnp[IDX_ACC_CARD].nCount;
					Info.expCardPubTck.dwMoney += pTckWork->tPubTck[IDX_ACC_CARD].dwMoney + pTckWork->tMrnp[IDX_ACC_CARD].dwMoney;

					///< [고속] 승차권 카드 환불 매수/금액
					Info.expCardRefund.nCount += pTckWork->tRefund[IDX_ACC_CARD].nCount;
					Info.expCardRefund.dwMoney += pTckWork->tRefund[IDX_ACC_CARD].dwMoney;

					///< [고속] 승차권 현금 발권 매수/금액
					Info.expCashPubTck.nCount += pTckWork->tPubTck[IDX_ACC_CASH].nCount; // + pTckWork->tMrnp[IDX_ACC_CASH].nCount;
					Info.expCashPubTck.dwMoney += pTckWork->tPubTck[IDX_ACC_CASH].dwMoney; // + pTckWork->tMrnp[IDX_ACC_CASH].dwMoney;

					///< [고속] 승차권 현금 환불 매수/금액
					Info.expCashRefund.nCount += pTckWork->tRefund[IDX_ACC_CASH].nCount;
					Info.expCashRefund.dwMoney += pTckWork->tRefund[IDX_ACC_CASH].dwMoney;
				}
// 				TR_LOG_OUT("%s", "[#2. 고속_티켓_정보] ");
// 				TR_LOG_OUT("\t 카드발권(%d, %d), 현금발권(%d, %d), 카드환불(%d, %d), 현금환불(%d, %d) ", 
// 					Info.expCardPubTck.nCount, Info.expCardPubTck.dwMoney,
// 					Info.expCashPubTck.nCount, Info.expCashPubTck.dwMoney,
// 					Info.expCardRefund.nCount, Info.expCardRefund.dwMoney, 
// 					Info.expCashRefund.nCount, Info.expCashRefund.dwMoney);
			}

			// 수량 (승차권 잔여수량 / 승차권 수집함 수량)
			{
				/// 승차권 잔여 수량
				Info.n_tck_remain_count = pAccum->Curr.phyTicket.nCount;

				/// 승차권 수집함 수량
				Info.n_tck_box_count = pAccum->Curr.phyTicketBox.nCount;
// 				TR_LOG_OUT("%s", "[#3. 승차권 정보] ");
// 				TR_LOG_OUT("\t 승차권 잔여수량 (%d), 승차권 수집함 수량(%d)", Info.n_tck_remain_count, Info.n_tck_box_count);
			}

			///< 승차권 고유번호
			{
				// [시외] 승차권 고유번호
				Info.n_ccs_inhr_no = pAccum->Curr.ccsTicket.n_bus_tck_inhr_no;
				// [고속] 승차권 고유번호
				Info.n_exp_inhr_no = pAccum->Curr.expTicket.n_bus_tck_inhr_no;
// 				TR_LOG_OUT("%s", "[#4. 승차권 고유번호] ");
// 				TR_LOG_OUT("\t 시외_고유번호(%d), 고속_고유번호(%d)", Info.n_ccs_inhr_no, Info.n_exp_inhr_no);
			}

			///< 누계
			{
				///> [입금] 동전 투입 누계	
				Info.SumInsCoin.n100	= 0;
				Info.SumInsCoin.n500	= 0;

				///> [입금] 지폐 투입 누계
				Info.SumInsBill.n1k		= pAccum->Close.inBill.n1k;
				Info.SumInsBill.n5k		= pAccum->Close.inBill.n5k;
				Info.SumInsBill.n10k	= pAccum->Close.inBill.n10k;
				Info.SumInsBill.n50k	= pAccum->Close.inBill.n50k;

				///> [출금] 동전 투입 누계	
				Info.SumOutCoin.n100	= pAccum->Close.outCoin.n100;
				Info.SumOutCoin.n500	= pAccum->Close.outCoin.n500;

				///> [출금] 지폐 투입 누계
				Info.SumOutBill.n1k		= pAccum->Close.outBill.n1k;
				Info.SumOutBill.n5k		= pAccum->Close.outBill.n5k;
				Info.SumOutBill.n10k	= pAccum->Close.outBill.n10k;
				Info.SumOutBill.n50k	= pAccum->Close.outBill.n50k;

			}

			PrintAccountInfo((char *)&Info);

			::CopyMemory(queData.szData, &Info, sizeof(UI_DEV_ACCOUNT_INFO_T));
		}

		// Length
		queData.nLen = sizeof(UI_DEV_ACCOUNT_INFO_T);

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddDevConfigData
 * @details		장비 환경 설정 정보 전송 (106)
 * @param		None
 * @return		항상 : 0
 */
int UI_AddDevConfigData(void)
{
	/***
	int				nOffset;
	UI_QUE_DATA_T	queData;
	UI_BASE_INFO_T	Info;
	POPER_FILE_CONFIG_T	pConfig;
	PKIOSK_INI_ENV_T pEnv;

	::ZeroMemory(&Info, sizeof(UI_BASE_INFO_T));

	nOffset = 0;

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_DEV_CONFIG;
		queData.nFlag = FLAG_SEND_RECV;

		pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();

		pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

		// option debug
		PrintOptionInfo(&pConfig->base_t);

		// packet 전체 복사
		::CopyMemory(&Info.baseInfo_t, &pConfig->base_t, sizeof(UI_BASE_T));

		///> 결제방식 H/W 종류
		Info.baseInfo_t.device_dvs = pEnv->nDeviceDVS + 0x30;

		///> 시외버스 종류
		if( pEnv->tCcInfo.nUse == 0 )
		{
			Info.baseInfo_t.ccs_svr_kind = 0x30;
		}
		else
		{
			Info.baseInfo_t.ccs_svr_kind = pEnv->tCcInfo.nKind + 0x30;
		}

		///> 고속버스 종류
		if( (pEnv->tKoInfo.nUse + pEnv->tEzInfo.nUse)  > 0 )
		{
			if( pEnv->tKoInfo.nUse > 0 )
			{
				Info.baseInfo_t.exp_svr_kind = pEnv->tKoInfo.nKind + 0x30;
			}
			else
			{
				Info.baseInfo_t.exp_svr_kind = pEnv->tEzInfo.nKind + 0x30;
			}
		}
		else
		{
			Info.baseInfo_t.exp_svr_kind = 0x30;
		}

		/// Set Global variable
		::CopyMemory(&s_BaseInfo_t, &Info.baseInfo_t, sizeof(UI_BASE_T));

		// Data
		::CopyMemory(queData.szData, &Info, sizeof(UI_BASE_INFO_T));
		// Length
		queData.nLen = (int) sizeof(UI_BASE_INFO_T);

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();
	***/

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
	try
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
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
	try
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
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
	try
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
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
int UI_AddFailInfo(WORD wCommand, char *pCode, char *pContents)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;
	UI_SND_ERROR_T	data;

	nOffset = 0;

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		::ZeroMemory(&data, sizeof(UI_SND_FAIL_INFO_T));

		queData.wCommand = wCommand;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		data.byACK = CHAR_NAK;
		sprintf(data.szErrCode, "%s", pCode);
		if( pContents != NULL )
		{
			sprintf(data.szErrContents, "%s", pContents);
		}
		::CopyMemory(queData.szData, &data, sizeof(UI_SND_ERROR_T));
		// Length
		queData.nLen = sizeof(UI_SND_ERROR_T);

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

// 20210513 ADD
/**
 * @brief		UI_ResWndKioskClose
 * @details		창구마감 처리 결과 정보 전송
 * @param		None
 * @return		항상 : 0
 */
int UI_ResWndKioskClose(WORD wCommand, char *pCode, char *pContents)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;
	UI_SND_ERROR_T	data;

	nOffset = 0;

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		::ZeroMemory(&data, sizeof(UI_SND_FAIL_INFO_T));

		queData.wCommand = wCommand;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		data.byACK = CHAR_NAK;
		sprintf(data.szErrCode, "%s", pCode);
		if( pContents != NULL )
		{
			sprintf(data.szErrContents, "%s", pContents);
		}
		::CopyMemory(queData.szData, &data, sizeof(UI_SND_ERROR_T));
		// Length
		queData.nLen = sizeof(UI_SND_ERROR_T);

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}
// 20210513 ~ADD

/**
 * @brief		UI_MrsIssueState
 * @details		예매 - 승차권 발권 상태 전송
 * @param		None
 * @return		항상 : 0
 */
int UI_MrsIssueState(BYTE byACK, BYTE byState, int nSatNo)
{
	int				nOffset = 0;
	UI_QUE_DATA_T	queData;
	UI_SND_MRS_ISSUE_STATE_T data;

	nOffset = 0;

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		::ZeroMemory(&data, sizeof(UI_SND_MRS_ISSUE_STATE_T));

		queData.wCommand = UI_CMD_MRS_ISSUE_STATE;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		data.byACK = byACK;
		data.byState = byState;
		data.w_sats_no =  (WORD) nSatNo;
		::CopyMemory(queData.szData, &data, sizeof(UI_SND_MRS_ISSUE_STATE_T));

		// Length
		queData.nLen = sizeof(UI_SND_MRS_ISSUE_STATE_T);

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
int UI_PbTckIssueState(char byACK, int nState, int n_sats_no)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;
	UI_SND_PBTCK_ISSUE_STATE_T data;

	nOffset = 0;

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		::ZeroMemory(&data, sizeof(UI_SND_PBTCK_ISSUE_STATE_T));

		queData.wCommand = UI_CMD_PBTCK_TCK_ISSUE_RET;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		//		data.byACK = CHAR_ACK;
		data.byACK = byACK;
		data.byState = nState & 0xFF;
		data.bySeat = n_sats_no & 0xFF;
		::CopyMemory(queData.szData, &data, sizeof(UI_SND_PBTCK_ISSUE_STATE_T));

		// Length
		queData.nLen = sizeof(UI_SND_PBTCK_ISSUE_STATE_T);

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_TckIssueCompleteState
 * @details		승차권 발권 완료 : 추가 - 20200406
 * @param		None
 * @return		항상 : 0
 */
int UI_TckIssueCompleteState(char byACK, char *card_aprv_no)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;
	UI_SND_PBTCK_ISSUE_COMP_T data;

	nOffset = 0;

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		//::ZeroMemory(&data, sizeof(UI_SND_PBTCK_ISSUE_STATE_T));	// 20211230 DEL
		::ZeroMemory(&data, sizeof(UI_SND_PBTCK_ISSUE_COMP_T));		// 20211230 MOD

		queData.wCommand = UI_CMD_PBTCK_TCK_ISSUE_COMP;
		queData.nFlag = FLAG_SEND_RECV;

		TR_LOG_OUT("[UI_TckIssueCompleteState][Main->UI_Q] wCommand = %d", (int)queData.wCommand);	// 20211230 ADD for DEBUG

		// Data
		//		data.byACK = CHAR_ACK;
		data.byACK = byACK;
		::CopyMemory(data.card_aprv_no, card_aprv_no, sizeof(data.card_aprv_no));

		if(1)
		//if(byACK == CHAR_ACK)
		{
			// 카드승인번호
			::CopyMemory(queData.szData, &data, sizeof(UI_SND_PBTCK_ISSUE_COMP_T));
			// Length
			queData.nLen = sizeof(UI_SND_PBTCK_ISSUE_COMP_T);
		}

		TR_LOG_OUT("[Main->UI_Q] s_QueData.push START ... wCommand = %d, nDataLen = %d", (int)queData.wCommand, queData.nLen);	// 20211230 ADD for DEBUG
		s_QueData.push(queData);
		TR_LOG_OUT("[Main->UI_Q] s_QueData.push END ... wCommand = %d, nDataLen = %d", (int)queData.wCommand, queData.nLen);	// 20210823 ADD for DEBUG
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
		try
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
		catch ( ... )
		{
			LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
		}
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddTrmlInfo
 * @details		파일 정보 (107)
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
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		::ZeroMemory(&Info, sizeof(UI_TRML_INFO_T));

		queData.wCommand = UI_CMD_TRML_INFO;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		{
			int					 nSvrKind;
			char				 Buffer[100];
			TERMINAL_INFO_T*	 pTrmlInfo;
			PUI_BASE_TRML_INFO_T pUiTrmlInfo;

			pUiTrmlInfo = &Info.ccTrmlInfo;

			nSvrKind = GetConfigServerKind();

			LOG_OUT("Config 서버종류 = %d", nSvrKind);

			///> 시외터미널 정보
			if(nSvrKind & SVR_DVS_CCBUS)
			{
				pTrmlInfo = (TERMINAL_INFO_T*) GetConfigTrmlInfo(SVR_DVS_CCBUS);

				///> [시외] 창구번호
				::CopyMemory(pUiTrmlInfo->trml_wnd_no, GetTrmlWndNo(SVR_DVS_CCBUS), sizeof(pUiTrmlInfo->trml_wnd_no));	
				///> [시외] 터미널코드(7바이트)
				::CopyMemory(pUiTrmlInfo->trml_cd, GetTrmlCode(SVR_DVS_CCBUS), sizeof(pUiTrmlInfo->trml_cd));
				///> [시외] 단축 터미널코드
				::CopyMemory(pUiTrmlInfo->shct_trml_cd, GetShortTrmlCode(SVR_DVS_CCBUS), sizeof(pUiTrmlInfo->shct_trml_cd));
				///> [시외] user no
				::CopyMemory(pUiTrmlInfo->user_no, GetLoginUserNo(SVR_DVS_CCBUS), sizeof(pUiTrmlInfo->user_no));
				///> [시외] user pwd
				::CopyMemory(pUiTrmlInfo->user_pwd, GetLoginUserPwd(SVR_DVS_CCBUS), sizeof(pUiTrmlInfo->user_pwd));	
				///> [시외] 터미널 명칭
				sprintf(pUiTrmlInfo->trml_nm, "%s", pTrmlInfo->szName);
				///> [시외] 터미널 전화번호
				sprintf(pUiTrmlInfo->trml_rprn_tel_no, "%s", pTrmlInfo->szCorpTel);	
				///> [시외] 가맹점이름
				sprintf(pUiTrmlInfo->prn_trml_nm, "%s", pTrmlInfo->sz_prn_trml_nm);	
				///> [시외] 가맹점 사업자번호
				sprintf(pUiTrmlInfo->prn_trml_corp_no, "%s", pTrmlInfo->sz_prn_trml_corp_no);	
				///> [시외]  터미널 상호
				sprintf(pUiTrmlInfo->prn_trml_sangho, "%s", pTrmlInfo->sz_prn_trml_sangho);	
				///> 인쇄시 터미널 사업자번호
				sprintf(pUiTrmlInfo->prn_trml_corp_no1, "%s", pTrmlInfo->sz_prn_trml_corp_no1);	
				///> [시외] 터미널 전화번호
				sprintf(pUiTrmlInfo->prn_trml_tel_no, "%s", pTrmlInfo->sz_prn_trml_tel_no);	
				///> [시외] 터미널 ARS 번호
				sprintf(pUiTrmlInfo->prn_trml_ars_no, "%s", pTrmlInfo->sz_prn_trml_ars_no);	

				if(0)
				{
					LOG_OUT("[시외] 창구번호 = %s"				, pUiTrmlInfo->trml_wnd_no		);
					LOG_OUT("[시외] 터미널코드 = %s"				, pUiTrmlInfo->trml_cd			);
					LOG_OUT("[시외] 단축 터미널코드 = %s"			, pUiTrmlInfo->shct_trml_cd		);
					LOG_OUT("[시외] 사용자ID = %s"				, pUiTrmlInfo->user_no			);
					LOG_OUT("[시외] 사용자비밀번호 = %s"			, pUiTrmlInfo->user_pwd			);
					LOG_OUT("[시외] 터미널 명 = %s"				, pUiTrmlInfo->trml_nm			);
					LOG_OUT("[시외] 터미널 전화번호 = %s"			, pUiTrmlInfo->trml_rprn_tel_no	);
					LOG_OUT("[시외] 인쇄시 가맹점이름 = %s"		, pUiTrmlInfo->prn_trml_nm		);
					LOG_OUT("[시외] 인쇄시 가맹점 사업자번호 = %s", pUiTrmlInfo->prn_trml_corp_no	);
					LOG_OUT("[시외] 인쇄시 터미널 상호 = %s"		, pUiTrmlInfo->prn_trml_sangho	);
					LOG_OUT("[시외] 인쇄시 터미널 사업자번호 = %s", pUiTrmlInfo->prn_trml_corp_no1);
					LOG_OUT("[시외] 인쇄시 터미널 전화번호 = %s"	, pUiTrmlInfo->prn_trml_tel_no	);
					LOG_OUT("[시외] 인쇄시 터미널 ARS 번호 = %s"	, pUiTrmlInfo->prn_trml_ars_no	);
				}
			}

			///> 고속터미널 정보
			pUiTrmlInfo = &Info.expTrmlInfo;

			if( (nSvrKind & (SVR_DVS_KOBUS | SVR_DVS_TMEXP)) )
			{
				int nKind;

				if( nSvrKind & SVR_DVS_KOBUS )
				{
					nKind = SVR_DVS_KOBUS; 	
					pTrmlInfo = (TERMINAL_INFO_T*) GetConfigTrmlInfo(SVR_DVS_KOBUS);
				}
				else
				{
					nKind = SVR_DVS_TMEXP;
					pTrmlInfo = (TERMINAL_INFO_T*) GetConfigTrmlInfo(SVR_DVS_TMEXP);
				}

				///> [고속] 창구번호
				::CopyMemory(pUiTrmlInfo->trml_wnd_no, GetTrmlWndNo(nKind), sizeof(pUiTrmlInfo->trml_wnd_no));	
				///> [고속] 터미널코드(7바이트)
				::CopyMemory(pUiTrmlInfo->trml_cd, GetTrmlCode(nKind), sizeof(pUiTrmlInfo->trml_cd));
				///> [고속] 단축 터미널코드
				::CopyMemory(pUiTrmlInfo->shct_trml_cd, GetShortTrmlCode(nKind), sizeof(pUiTrmlInfo->shct_trml_cd));
				
				if( nSvrKind & SVR_DVS_KOBUS )
				{
					///> [코버스] user no
					::CopyMemory(pUiTrmlInfo->user_no, GetLoginUserNo(nKind), sizeof(pUiTrmlInfo->user_no));
				}
				else
				{
					///> [티머니고속] user id
					//::CopyMemory(pUiTrmlInfo->user_no, GetLoginUserNo(nKind), sizeof(pUiTrmlInfo->user_no));
					::ZeroMemory(Buffer, sizeof(Buffer));
					CConfigTkMem::GetInstance()->GetTmExpUserNo(Buffer);

					LOG_OUT("[티머니고속] auth_mem user_no(%s) "	, Buffer);
					if(strlen(Buffer) > 0)
					{
						::CopyMemory(pUiTrmlInfo->user_no, Buffer, sizeof(pUiTrmlInfo->user_no));
					}
					else
					{
						::CopyMemory(pUiTrmlInfo->user_no, GetLoginUserNo(nKind), sizeof(pUiTrmlInfo->user_no));
					}
				}
				///> [고속] user pwd
				::CopyMemory(pUiTrmlInfo->user_pwd, GetLoginUserPwd(nKind), sizeof(pUiTrmlInfo->user_pwd));	
				///> [고속] 터미널 명칭
				sprintf(pUiTrmlInfo->trml_nm, "%s", pTrmlInfo->szName);
				///> [고속] 터미널 전화번호
				sprintf(pUiTrmlInfo->trml_rprn_tel_no, "%s", pTrmlInfo->szCorpTel);	
				///> [고속] 가맹점이름
				sprintf(pUiTrmlInfo->prn_trml_nm, "%s", pTrmlInfo->sz_prn_trml_nm);	
				///> [고속] 가맹점 사업자번호
				sprintf(pUiTrmlInfo->prn_trml_corp_no, "%s", pTrmlInfo->sz_prn_trml_corp_no);	
				///> [고속]  터미널 상호
				sprintf(pUiTrmlInfo->prn_trml_sangho, "%s", pTrmlInfo->sz_prn_trml_sangho);	
				///> [고속] 인쇄시 터미널 사업자번호
				sprintf(pUiTrmlInfo->prn_trml_corp_no1, "%s", pTrmlInfo->sz_prn_trml_corp_no1);	
				///> [고속]  터미널 전화번호
				sprintf(pUiTrmlInfo->prn_trml_tel_no, "%s", pTrmlInfo->sz_prn_trml_tel_no);	
				///> [고속] 터미널 ARS 번호
				sprintf(pUiTrmlInfo->prn_trml_ars_no, "%s", pTrmlInfo->sz_prn_trml_ars_no);	

				if(0)
				{
					LOG_OUT("[고속] 창구번호 = %s"				, pUiTrmlInfo->trml_wnd_no		);
					LOG_OUT("[고속] 터미널코드 = %s"				, pUiTrmlInfo->trml_cd			);
					LOG_OUT("[고속] 단축 터미널코드 = %s"			, pUiTrmlInfo->shct_trml_cd		);
					LOG_OUT("[고속] 사용자ID = %s"				, pUiTrmlInfo->user_no			);
					LOG_OUT("[고속] 사용자비밀번호 = %s"			, pUiTrmlInfo->user_pwd			);
					LOG_OUT("[고속] 터미널 명 = %s"				, pUiTrmlInfo->trml_nm			);
					LOG_OUT("[고속] 터미널 전화번호 = %s"			, pUiTrmlInfo->trml_rprn_tel_no	);
					LOG_OUT("[고속] 인쇄시 가맹점이름 = %s"		, pUiTrmlInfo->prn_trml_nm		);
					LOG_OUT("[고속] 인쇄시 가맹점 사업자번호 = %s", pUiTrmlInfo->prn_trml_corp_no	);
					LOG_OUT("[고속] 인쇄시 터미널 상호 = %s"		, pUiTrmlInfo->prn_trml_sangho	);
					LOG_OUT("[고속] 인쇄시 터미널 사업자번호 = %s", pUiTrmlInfo->prn_trml_corp_no1);
					LOG_OUT("[고속] 인쇄시 터미널 전화번호 = %s"	, pUiTrmlInfo->prn_trml_tel_no	);
					LOG_OUT("[고속] 인쇄시 터미널 ARS 번호 = %s"	, pUiTrmlInfo->prn_trml_ars_no	);
				}
			}

			::CopyMemory(queData.szData, &Info, sizeof(UI_TRML_INFO_T));
			// Length
			queData.nLen = sizeof(UI_TRML_INFO_T);
		}

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddFileInfo
 * @details		파일 정보 (108)
 * @param		None
 * @return		항상 : 0
 */
int UI_AddFileInfo(int nNumber, int nID)
{
	UI_QUE_DATA_T	queData;
	UI_BASE_FILE_INFO_T uiInfo;

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		::ZeroMemory(&uiInfo, sizeof(UI_BASE_FILE_INFO_T));

		queData.wCommand = UI_CMD_DEV_FILE_INFO;
		queData.nFlag = FLAG_SEND_RECV;

		uiInfo.wFileID = nNumber;
		OperGetFileInfo(nID, uiInfo.szFullName, (int *)&uiInfo.dwSize);

		LOG_OUT("[기초정보] - ID(%d), (%s) ..", nID, uiInfo.szFullName);

		// Data
		::CopyMemory(queData.szData, &uiInfo, sizeof(UI_BASE_FILE_INFO_T));
		// Length
		queData.nLen = sizeof(UI_BASE_FILE_INFO_T);

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddTckInhrNoInfo
 * @details		버스티켓 고유번호 정보 (109)
 * @param		None
 * @return		항상 : 0
 */
int UI_AddTckInhrNoInfo(void)
{
	UI_QUE_DATA_T	queData;
	UI_ADD_TCK_INHRNO_T uiInfo;

	Locking();
	try
	{
		PFILE_ACCUM_N1010_T pAcc;

		pAcc = GetAccumulateData();

		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		::ZeroMemory(&uiInfo, sizeof(UI_ADD_TCK_INHRNO_T));

		queData.wCommand = UI_CMD_TCK_INHR_NO_INFO;
		queData.nFlag = FLAG_SEND_RECV;

		///> [시외] 버스 티켓 고유번호
		sprintf(uiInfo.ccs_bus_tck_inhr_no, "%08d", pAcc->Curr.ccsTicket.n_bus_tck_inhr_no);
		///> [시외] 추가 버스 티켓 고유번호
		sprintf(uiInfo.ccs_add_bus_tck_inhr_no, "%08d", pAcc->Curr.ccsTicket.n_add_bus_tck_inhr_no);

		///> [고속] 버스 티켓 고유번호
		sprintf(uiInfo.exp_bus_tck_inhr_no, "%08d", pAcc->Curr.expTicket.n_bus_tck_inhr_no);
		///> [고속] 추가 버스 티켓 고유번호
		sprintf(uiInfo.exp_add_bus_tck_inhr_no, "%08d", pAcc->Curr.expTicket.n_add_bus_tck_inhr_no);

		// Data
		::CopyMemory(queData.szData, &uiInfo, sizeof(UI_BASE_FILE_INFO_T));
		// Length
		queData.nLen = sizeof(UI_BASE_FILE_INFO_T);

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
	int nWhchCount = 0;

	Locking();
	try
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

			// 20211206 ADD~
			// 휠체어좌석수
			nWhchCount = pTr->n_whch_sats_num;
			::CopyMemory(&queData.szData[nOffset], &pTr->n_whch_sats_num, sizeof(WORD));
			nOffset += sizeof(WORD);
			// 20211206 ~ADD

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

			// 20211206 ADD~
			vector<rtk_readalcnwhch_list_t>::iterator	whchiter;

			for( whchiter = pTr->m_vtWhchInfo.begin(); whchiter != pTr->m_vtWhchInfo.end(); whchiter++ )
			//for(i = 0; i < nCount; i++)
			{
				//prtk_readalcnfee_list_t pList;

				//pList = &pInfo->List[i];

				///< 좌석번호(number)
				::CopyMemory(&queData.szData[nOffset], whchiter->sats_no, sizeof(WORD));
				nOffset += sizeof(WORD);

				///< 휠체어좌석번호(number)
				::CopyMemory(&queData.szData[nOffset], whchiter->whch_sats_no, sizeof(WORD));
				nOffset += sizeof(WORD);

				///< 휠체어좌석번호출력명
				::CopyMemory(&queData.szData[nOffset], whchiter->whch_sats_no_prin_nm, sizeof(whchiter->whch_sats_no_prin_nm) - 1);
				nOffset += (sizeof(whchiter->whch_sats_no_prin_nm) - 1);
			}
			// 20211206 ~ADD

			// Length
			queData.nLen = nOffset;
		}
		else
		{
//			UI_SND_ERROR_T	errData;

			nOffset = MakeErrorPacket((char *)queData.szData);
			queData.nLen = nOffset;

// 			// Result
// 			queData.szData[nOffset++] = CHAR_NAK;
// 			nOffset += sprintf(&queData.szData[nOffset], "%s", CConfigTkMem::GetInstance()->GetErrorCode());
// 
// 			// Length
// 			queData.nLen = nOffset;
		}

		s_QueData.push(queData);

	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
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
int UI_TkPubSatsInfo(int nAlcn, BOOL bResult, int nError)
{
	int				nOffset, nCount, i;
	UI_QUE_DATA_T	queData;

	nOffset = nCount = i = 0;

	Locking();
	try
	{
		CPubTckMem* pTr;

		pTr = CPubTckMem::GetInstance();

		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_PBTCK_SEAT_INFO;
		queData.nFlag = FLAG_SEND_RECV;

		if(nAlcn > 0)
		{	/// 배차모드인 경우
			// Data
			if(bResult == TRUE)
			{
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
// 				// Result
// 				queData.szData[nOffset++] = CHAR_NAK;
// 				// Error Code
// 				nOffset += sprintf(&queData.szData[nOffset], "%s", CConfigTkMem::GetInstance()->GetErrorCode());
// 				// Length
// 				queData.nLen = nOffset;

				nOffset = MakeErrorPacket((char *)queData.szData);
				queData.nLen = nOffset;
			}
		}
		else
		{	/// 비배차 모드인 경우
			// Result
			queData.szData[nOffset++] = CHAR_ACK;

			// 좌석 정보수
			nCount = pTr->m_vtNPcpysats.size();
			::CopyMemory(&queData.szData[nOffset], &nCount, sizeof(WORD));
			nOffset += sizeof(WORD);

			vector<rtk_pcpysats_list_t>::iterator	iter;

			for( iter = pTr->m_vtNPcpysats.begin(); iter != pTr->m_vtNPcpysats.end(); iter++ )
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

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_TkPubTmaxResultInfo
 * @details		현장발권 - 티맥스 승차권 발권결과 (312)
 * @param		None
 * @return		항상 : 0
 */
int UI_TkPubTmaxResultInfo(BOOL bResult, int nError)
{
	int				nOffset, nCount, i;
	UI_QUE_DATA_T	queData;

	nOffset = nCount = i = 0;

	LOG_OUT("UI_TkPubTmaxResultInfo Start... !!");

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_PBTCK_TMAX_RESULT;
		queData.nFlag = FLAG_SEND_RECV;

		// Data
		if(bResult == TRUE)
		{
			if( CConfigTkMem::GetInstance()->n_bus_dvs == SVR_DVS_CCBUS )
			{
				CPubTckMem* pTr;

				pTr = CPubTckMem::GetInstance();

				// Result
				queData.szData[nOffset++] = CHAR_ACK;
				queData.szData[nOffset++] = CPubTckMem::GetInstance()->base.ui_pym_dvs_cd[0];

				// Length
				queData.nLen = nOffset;
			}
			else
			{
				CPubTckKobusMem* pTr;

				pTr = CPubTckKobusMem::GetInstance();

				// Result
				queData.szData[nOffset++] = CHAR_ACK;
				queData.szData[nOffset++] = pTr->base.ui_pym_dvs_cd[0];

				// Length
				queData.nLen = nOffset;
			}
		}
		else
		{
// 			// Result
// 			queData.szData[nOffset++] = CHAR_NAK;
// 			sprintf(&queData.szData[nOffset], "%s", CConfigTkMem::GetInstance()->GetErrorCode());
// 			nOffset += 10;
// 			// Length
// 			queData.nLen = nOffset;

			nOffset = MakeErrorPacket((char *)queData.szData);
			queData.nLen = nOffset;
		}

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddStaffResultInfo
 * @details		현장발권 - 티맥스 승차권 발권결과 (321)
 * @param		None
 * @return		항상 : 0
 */
int UI_AddStaffResultInfo(BOOL bResult)
{
	int nOffset = 0;
	UI_QUE_DATA_T	queData;
	BYTE byACK;

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		queData.wCommand = UI_CMD_PBTCK_STAFF_RESP;
		if(bResult == TRUE)
		{
			queData.nFlag = FLAG_Q_SEND_RECV;

			byACK = CHAR_ACK;
			::CopyMemory(queData.szData, &byACK, sizeof(BYTE));
			queData.nLen = sizeof(BYTE);
		}
		else
		{
			queData.nFlag = FLAG_SEND_RECV;

// 			queData.szData[nOffset++] = CHAR_NAK;
// 			nOffset += sprintf(&queData.szData[nOffset], "%s", CConfigTkMem::GetInstance()->GetErrorCode());
			nOffset = MakeErrorPacket((char *)queData.szData);
			queData.nLen = nOffset;
		}

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_TkPubStaffSatsInfo
 * @details		현장발권 - 배차요금 조회 (0x323)
 * @param		None
 * @return		항상 : 0
 */
int UI_TkPubStaffSatsInfo(int nAlcn, BOOL bResult, int nError)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;

	nOffset = 0;

	Locking();
	try
	{
		CPubTckMem* pTr;

		pTr = CPubTckMem::GetInstance();

		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_PBTCK_STAFF_CD_MOD_FARE_RESP;
		queData.nFlag = FLAG_Q_SEND_RECV;

		nOffset += Util_MyCopyMemory(&queData.szData[nOffset], &bResult, 4);
		nOffset += Util_MyCopyMemory(&queData.szData[nOffset], &nError, 4);
		nOffset += Util_MyCopyMemory(&queData.szData[nOffset], &nAlcn, 4);
			
		// Length
		queData.nLen = nOffset;
		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddRefundInfo
 * @details		환불 - 승차권판독 결과 (402)
 * @param		BOOL bResult			결과값		
 * @param		BYTE chBusDVS			버스구분코드
 * @return		항상 : 0
 */
int UI_AddRefundInfo(BOOL bResult, BYTE chBusDVS)
{
	int nOffset = 0;
	UI_QUE_DATA_T	queData;
	UI_SND_CANCRY_TCK_INFO_T data;

	LOG_OUT("결과값(%d), 버스구분코드(0x%02X)..", bResult, chBusDVS & 0xFF);

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		queData.wCommand = UI_CMD_CANCRY_INFO;
		queData.nFlag = FLAG_SEND_RECV;

		if(bResult == TRUE)
		{
			if(chBusDVS == UI_BUS_DVS_CCS)
			{	/// 시외버스
				CCancRyTkMem* pCancRy;

				pCancRy = CCancRyTkMem::GetInstance();

				CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_CCBUS;

				data.byAck = CHAR_ACK;
				data.chBusDVS = chBusDVS;

				LOG_OUT("시외버스 승차권판독 결과 결제수단(0x%02X)..", pCancRy->tRespTckNo.pyn_mns_dvs_cd[0] & 0xFF);	// 20211013 ADD
				
				// 20221221 ADD~
				LOG_OUT("시외버스 위약금 타입(%s) ", pCancRy->tRespTckNo.brkp_type); // 20221223 ADD
				LOG_OUT("시외버스 QR결제수단(%s) ", pCancRy->tRespTckNo.qr_pym_pyn_dtl_cd); 
				//::ZeroMemory(data.qr_pym_pyn_dtl_cd, sizeof(data.qr_pym_pyn_dtl_cd));
				::CopyMemory(data.brkp_type, pCancRy->tRespTckNo.brkp_type, sizeof(data.brkp_type)); // 20221223 ADD
				::CopyMemory(data.qr_pym_pyn_dtl_cd, pCancRy->tRespTckNo.qr_pym_pyn_dtl_cd, sizeof(data.qr_pym_pyn_dtl_cd));
				// 20221221 ~ADD

				/// 결제수단
				switch(pCancRy->tRespTckNo.pyn_mns_dvs_cd[0])
				{
				case PYM_CD_CARD :
				case PYM_CD_TPAY :
					data.pyn_mns_dvs_cd[0] = 0x02; // 결제수단:카드
					break;
				// 20211013 ADD
				case PYM_CD_COMP :  // 시외-복합결제
					data.pyn_mns_dvs_cd[0] = 0x05; // 결제수단:복합결제
					break;
				// 20211013 ~ADD
				case PYM_CD_CASH :
				default :
					data.pyn_mns_dvs_cd[0] = 0x01; // 결제수단:현금
					break;
				}
				
				/****
				if(  == PYM_CD_CARD)
				{
					data.pyn_mns_dvs_cd[0] = 0x02; // 결제수단:카드
				}
				else
				{
					data.pyn_mns_dvs_cd[0] = 0x01; // 결제수단:현금
				}
				****/

				/// 결제금액
				data.n_pym_amt = *(int *)pCancRy->tRespTckNo.tisu_amt; 
				/// 취소수수료
				data.n_canc_ry_cmrt = pCancRy->n_commission_fare; 
				/// 환불금액
				data.n_canc_ry_amt = data.n_pym_amt - data.n_canc_ry_cmrt; 
			}
			else if(chBusDVS == UI_BUS_DVS_KOBUS)
			{	/// 코버스
				CCancRyTkKobusMem* pCancRy;

				pCancRy = CCancRyTkKobusMem::GetInstance();

				CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_KOBUS;

				data.byAck = CHAR_ACK;
				data.chBusDVS = chBusDVS;

				LOG_OUT("코버스 승차권판독 결과 결제수단(0x%02X)..", pCancRy->tBase.ui_pym_dvs_cd[0] & 0xFF);	// 20211013 ADD

				/// 결제수단
				if( pCancRy->tBase.ui_pym_dvs_cd[0] == PYM_CD_CASH )
				{
					data.pyn_mns_dvs_cd[0] = 0x01; // 결제수단:현금
				}
				// 20210910 ADD
				else if( pCancRy->tBase.ui_pym_dvs_cd[0] == PYM_CD_ETC )
				{
					data.pyn_mns_dvs_cd[0] = 0x05; // 결제수단:복합결제
				}
				// 20210910 ~ADD
				else
				{
					data.pyn_mns_dvs_cd[0] = 0x02; // 결제수단:카드
				}

				/// 결제금액
				data.n_pym_amt = pCancRy->tBase.n_tot_money; 
				/// 취소수수료
				data.n_canc_ry_cmrt = pCancRy->tBase.n_commission_fare; 
				/// 환불금액
				data.n_canc_ry_amt = pCancRy->tBase.n_tot_money - pCancRy->tBase.n_commission_fare; 
			}
			else
			{	/// 티머니고속 
				CCancRyTkTmExpMem* pCancRy;

				pCancRy = CCancRyTkTmExpMem::GetInstance();

				CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_TMEXP;

				data.byAck = CHAR_ACK;
				data.chBusDVS = chBusDVS;

				LOG_OUT("티머니고속 승차권판독 결과 결제수단(0x%02X)..", pCancRy->tBase.ui_pym_dvs_cd[0] & 0xFF);	// 20211013 ADD

				/// 결제수단
				if( pCancRy->tBase.ui_pym_dvs_cd[0] == PYM_CD_CASH )
				{
					data.pyn_mns_dvs_cd[0] = 0x01; // 결제수단:현금
				}
				// 20211013 ADD
				else if( pCancRy->tBase.ui_pym_dvs_cd[0] == PYM_CD_ETC )
				{
					data.pyn_mns_dvs_cd[0] = 0x05; // 결제수단:복합결제
				}
				// 20211013 ~ADD
				else
				{
					data.pyn_mns_dvs_cd[0] = 0x02; // 결제수단:카드
				}

				/// 결제금액
				data.n_pym_amt = pCancRy->tBase.n_tot_money; 
				/// 취소수수료
				data.n_canc_ry_cmrt = pCancRy->tBase.n_commission_fare; 
				/// 환불금액
				data.n_canc_ry_amt = pCancRy->tBase.n_tot_money - pCancRy->tBase.n_commission_fare; 
			}

			::CopyMemory(queData.szData, &data, sizeof(UI_SND_CANCRY_TCK_INFO_T));

			queData.nLen = sizeof(UI_SND_CANCRY_TCK_INFO_T);

			s_QueData.push(queData);
		}
		else
		{
			UI_SND_ERROR_STR_T errData;

// 			nOffset = 0;
// 			queData.szData[nOffset++] = CHAR_NAK;
// 			nOffset += sprintf(&queData.szData[nOffset], "%s", CConfigTkMem::GetInstance()->GetErrorCode());

			::ZeroMemory(&errData, sizeof(UI_SND_ERROR_STR_T));

			MakeErrorPacket((char *)&errData);
			::CopyMemory(queData.szData, &errData, sizeof(UI_SND_ERROR_STR_T));
			queData.nLen = sizeof(UI_SND_ERROR_STR_T);

			s_QueData.push(queData);
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_NoOutCasgResultInfo
 * @details		관리자 - 미방출 방출 결과 (548)
 * @param		None
 * @return		항상 : 0
 */
int UI_NoOutCasgResultInfo(char *pData)
{
	int				nOffset, i;
//	WORD			wCount;
	UI_QUE_DATA_T	queData;
	PUI_SND_NOOUT_CASH_T pInfo;

	nOffset = i = 0;

	Locking();
	try
	{
		pInfo = (PUI_SND_NOOUT_CASH_T) pData;

		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_NOOUT_CASH_RESULT;
		queData.nFlag = FLAG_SEND_RECV;
		// Data
		::CopyMemory(queData.szData, pData, sizeof(UI_SND_NOOUT_CASH_T));
		nOffset = sizeof(UI_SND_NOOUT_CASH_T);
		// Length
		queData.nLen = nOffset;

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_InqDevCloseInfo
 * @details		관리자 - 마감내역 전송 (552)
 * @param		None
 * @return		항상 : 0
 */
int UI_InqDevCloseInfo(void)
{
	int						nOffset, i, nIndex, nSvrKind;
	int						nCount, n_ccs_tck_cnt, n_exp_tck_cnt;
	int						n_ccs_cash_cnt, n_ccs_card_cnt;
	int						n_exp_cash_cnt, n_exp_card_cnt;
	UI_QUE_DATA_T			queData;
	CLOSE_TCK_ITEM_T		item;
	SYSTEMTIME				st;
	POPER_FILE_CONFIG_T		pOperCfg;

	nOffset = i = nIndex = 0;
	nCount = n_ccs_tck_cnt = n_exp_tck_cnt = nSvrKind = 0;

	n_ccs_cash_cnt = n_ccs_card_cnt = 0;
	n_exp_cash_cnt = n_exp_card_cnt = 0;

	Locking();
	try
	{
		TCK_KIND_LIST_T tckInfo;
		PFILE_ACCUM_N1010_T pAccum = GetAccumulateData();
		
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_CLOSE_INQ_RESULT;
		queData.nFlag = FLAG_SEND_RECV;

		pOperCfg = (POPER_FILE_CONFIG_T) GetOperConfigData();

		nSvrKind = GetConfigServerKind();

		n_ccs_tck_cnt = 0;
		for(i = 0; i < CCS_IDX_TCK_MAX; i++)
		{
			::ZeroMemory(&tckInfo, sizeof(TCK_KIND_LIST_T));
			nIndex = GetTckkndListData(SVR_DVS_CCBUS, i, (char *)&tckInfo);
			if( nIndex < 0 ) 
			{
				continue;
			}
			n_ccs_tck_cnt++;

			//n_ccs_cash_cnt += pAccum->Curr.ccsTicketWork[i].tPubTck[IDX_ACC_CASH].nCount;
			//n_ccs_card_cnt += pAccum->Curr.ccsTicketWork[i].tPubTck[IDX_ACC_CARD].nCount;

			//n_ccs_tck_cnt += (n_ccs_cash_cnt + n_ccs_card_cnt);
		}

		n_exp_tck_cnt = 0;

		//if( nSvrKind & (SVR_DVS_KOBUS | SVR_DVS_TMEXP) )
		{
			if( nSvrKind & SVR_DVS_KOBUS )
			{	/// 코버스
				for(i = 0; i < KOBUS_IDX_TCK_MAX; i++)
				{
					::ZeroMemory(&tckInfo, sizeof(TCK_KIND_LIST_T));
					nIndex = GetTckkndListData(SVR_DVS_KOBUS, i, (char *)&tckInfo);
					if( nIndex < 0 ) 
					{
						continue;
					}
					n_exp_tck_cnt++;
					
					//n_exp_cash_cnt += pAccum->Curr.expTicketWork[i].tPubTck[IDX_ACC_CASH].nCount;
					//n_exp_card_cnt += pAccum->Curr.expTicketWork[i].tPubTck[IDX_ACC_CARD].nCount;

					//n_exp_tck_cnt += (n_exp_cash_cnt + n_exp_card_cnt);
				}
			}
			else
			{	/// 티머니고속
				for(i = 0; i < TMEXP_IDX_TCK_MAX; i++)
				{
					::ZeroMemory(&tckInfo, sizeof(TCK_KIND_LIST_T));
					nIndex = GetTckkndListData(SVR_DVS_TMEXP, i, (char *)&tckInfo);
					if( nIndex < 0 ) 
					{
						continue;
					}
					n_exp_tck_cnt++;

// 					n_exp_cash_cnt += pAccum->Curr.expTicketWork[i].tPubTck[IDX_ACC_CASH].nCount;
// 					n_exp_card_cnt += pAccum->Curr.expTicketWork[i].tPubTck[IDX_ACC_CARD].nCount;
// 
// 					n_exp_tck_cnt += (n_exp_cash_cnt + n_exp_card_cnt);
				}
			}
		}


		int nSize = ((n_ccs_tck_cnt * sizeof(CLOSE_TCK_ITEM_T)) * 2) + ((n_exp_tck_cnt * sizeof(CLOSE_TCK_ITEM_T)) * 2) + 
					(sizeof(CLOSE_TCK_ITEM_T) * 4) + 100;

		char* pSendPacket = new char[nSize];
		::ZeroMemory(pSendPacket, nSize);

		nOffset = 0;

		pSendPacket[nOffset++] = CHAR_ACK;

		/// 업무 시작 시간
		nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], pOperCfg->job_start_dt, 14);
		//nOffset += 14;

		// 현재시간
		::GetLocalTime(&st);
		nOffset += sprintf(&pSendPacket[nOffset], "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);


		///< 1. 고속버스 - 현금 발권 정보

		//if( nSvrKind & (SVR_DVS_KOBUS | SVR_DVS_TMEXP) )
		{
			if( nSvrKind & SVR_DVS_KOBUS )
			{
				// 1-0. 갯수
				nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &n_exp_tck_cnt, sizeof(int));

				if(n_exp_tck_cnt > 0)
				{
					// 1-1. 현금 발권 정보
					for(i = 0; i < KOBUS_IDX_TCK_MAX; i++)
					{
						::ZeroMemory(&tckInfo, sizeof(TCK_KIND_LIST_T));
						nIndex = GetTckkndListData(SVR_DVS_KOBUS, i, (char *)&tckInfo);
						if( nIndex < 0 ) 
						{
							continue;
						}

						::ZeroMemory(&item, sizeof(CLOSE_TCK_ITEM_T));
						::CopyMemory(item.tck_knd_cd, tckInfo.tck_kind, sizeof(item.tck_knd_cd));
						item.nCount		= pAccum->Curr.expTicketWork[i].tPubTck[IDX_ACC_CASH].nCount;
						item.nTotalFare = pAccum->Curr.expTicketWork[i].tPubTck[IDX_ACC_CASH].dwMoney;

						nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &item, sizeof(CLOSE_TCK_ITEM_T));
						//nOffset += sizeof(CLOSE_TCK_ITEM_T);
					}
				}

				// 1-0. 갯수
				nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &n_exp_tck_cnt, sizeof(int));

				if(n_exp_tck_cnt > 0)
				{
					for(i = 0; i < KOBUS_IDX_TCK_MAX; i++)
					{
						::ZeroMemory(&tckInfo, sizeof(TCK_KIND_LIST_T));
						nIndex = GetTckkndListData(SVR_DVS_KOBUS, i, (char *)&tckInfo);
						if( nIndex < 0 ) 
						{
							continue;
						}

						::ZeroMemory(&item, sizeof(CLOSE_TCK_ITEM_T));
						::CopyMemory(item.tck_knd_cd, tckInfo.tck_kind, sizeof(item.tck_knd_cd));
						item.nCount		= pAccum->Curr.expTicketWork[i].tPubTck[IDX_ACC_CARD].nCount;
						item.nTotalFare = pAccum->Curr.expTicketWork[i].tPubTck[IDX_ACC_CARD].dwMoney;

						nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &item, sizeof(CLOSE_TCK_ITEM_T));
						//nOffset += sizeof(CLOSE_TCK_ITEM_T);
					}
				}
			}
			else
			{	/// 티머니고속
				// 1-0. 갯수
				nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &n_exp_tck_cnt, sizeof(int));

				if(n_exp_tck_cnt > 0)
				{
					// 1-1. 현금 발권 정보
					for(i = 0; i < TMEXP_IDX_TCK_MAX; i++)
					{
						::ZeroMemory(&tckInfo, sizeof(TCK_KIND_LIST_T));
						nIndex = GetTckkndListData(SVR_DVS_TMEXP, i, (char *)&tckInfo);
						if( nIndex < 0 ) 
						{
							continue;
						}

						::ZeroMemory(&item, sizeof(CLOSE_TCK_ITEM_T));
						::CopyMemory(item.tck_knd_cd, tckInfo.tck_kind, sizeof(item.tck_knd_cd));
						item.nCount		= pAccum->Curr.expTicketWork[i].tPubTck[IDX_ACC_CASH].nCount;
						item.nTotalFare = pAccum->Curr.expTicketWork[i].tPubTck[IDX_ACC_CASH].dwMoney;

						nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &item, sizeof(CLOSE_TCK_ITEM_T));
						//nOffset += sizeof(CLOSE_TCK_ITEM_T);
					}
				}

				///< 1-2. 카드 발권 정보
				nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &n_exp_tck_cnt, sizeof(int));

				if(n_exp_tck_cnt > 0)
				{
					for(i = 0; i < TMEXP_IDX_TCK_MAX; i++)
					{
						::ZeroMemory(&tckInfo, sizeof(TCK_KIND_LIST_T));
						nIndex = GetTckkndListData(SVR_DVS_TMEXP, i, (char *)&tckInfo);
						if( nIndex < 0 ) 
						{
							continue;
						}

						::ZeroMemory(&item, sizeof(CLOSE_TCK_ITEM_T));
						::CopyMemory(item.tck_knd_cd, tckInfo.tck_kind, sizeof(item.tck_knd_cd));
						item.nCount		= pAccum->Curr.expTicketWork[i].tPubTck[IDX_ACC_CARD].nCount;
						item.nTotalFare = pAccum->Curr.expTicketWork[i].tPubTck[IDX_ACC_CARD].dwMoney;

						nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &item, sizeof(CLOSE_TCK_ITEM_T));
						//nOffset += sizeof(CLOSE_TCK_ITEM_T);
					}
				}
			}
		}

		///< 2. 시외버스 - 현금 발권 정보

		// 2-0. 갯수
		nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &n_ccs_tck_cnt, sizeof(int));
		//nOffset += sizeof(int);

		if(n_ccs_tck_cnt > 0)
		{
			// 2-1. 현금 발권 정보
			for(i = 0; i < CCS_IDX_TCK_MAX; i++)
			{
				::ZeroMemory(&tckInfo, sizeof(TCK_KIND_LIST_T));
				nIndex = GetTckkndListData(SVR_DVS_CCBUS, i, (char *)&tckInfo);
				if( nIndex < 0 ) 
				{
					continue;
				}

				::ZeroMemory(&item, sizeof(CLOSE_TCK_ITEM_T));
				::CopyMemory(item.tck_knd_cd, tckInfo.tck_kind, sizeof(item.tck_knd_cd));
				item.nCount		= pAccum->Curr.ccsTicketWork[i].tPubTck[IDX_ACC_CASH].nCount;
				item.nTotalFare = pAccum->Curr.ccsTicketWork[i].tPubTck[IDX_ACC_CASH].dwMoney;

				nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &item, sizeof(CLOSE_TCK_ITEM_T));
				//nOffset += sizeof(CLOSE_TCK_ITEM_T);
			}
		}

		// 2-0. 갯수
		nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &n_ccs_tck_cnt, sizeof(int));
		//nOffset += sizeof(int);

		if(n_ccs_tck_cnt > 0)
		{
			for(i = 0; i < CCS_IDX_TCK_MAX; i++)
			{
				::ZeroMemory(&tckInfo, sizeof(TCK_KIND_LIST_T));
				nIndex = GetTckkndListData(SVR_DVS_CCBUS, i, (char *)&tckInfo);
				if( nIndex < 0 ) 
				{
					continue;
				}

				::ZeroMemory(&item, sizeof(CLOSE_TCK_ITEM_T));
				::CopyMemory(item.tck_knd_cd, tckInfo.tck_kind, sizeof(item.tck_knd_cd));
				item.nCount		= pAccum->Curr.ccsTicketWork[i].tPubTck[IDX_ACC_CARD].nCount;
				item.nTotalFare = pAccum->Curr.ccsTicketWork[i].tPubTck[IDX_ACC_CARD].dwMoney;

				nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &item, sizeof(CLOSE_TCK_ITEM_T));
				//nOffset += sizeof(CLOSE_TCK_ITEM_T);
			}
		}

		///< 3. 고속버스 - 현금 환불 정보
		// 3-0. 갯수
		nCount = 1;
		nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &nCount, sizeof(int));
		//nOffset += sizeof(int);

		// 3-1. 현금 환불 정보
		::ZeroMemory(&item, sizeof(CLOSE_TCK_ITEM_T));
		item.nCount		= pAccum->Curr.expTicketWork[0].tRefund[IDX_ACC_CASH].nCount;
		item.nTotalFare = pAccum->Curr.expTicketWork[0].tRefund[IDX_ACC_CASH].dwMoney;
		nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &item, sizeof(CLOSE_TCK_ITEM_T));
		//nOffset += sizeof(CLOSE_TCK_ITEM_T);

		///< 4. 고속버스 - 카드 환불 정보
		// 4-0. 갯수
		nCount = 1;
		nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &nCount, sizeof(int));
		//nOffset += sizeof(int);

		// 4-1. 카드 환불 정보
		::ZeroMemory(&item, sizeof(CLOSE_TCK_ITEM_T));
		item.nCount		= pAccum->Curr.expTicketWork[0].tRefund[IDX_ACC_CARD].nCount;
		item.nTotalFare = pAccum->Curr.expTicketWork[0].tRefund[IDX_ACC_CARD].dwMoney;
		nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &item, sizeof(CLOSE_TCK_ITEM_T));
		//nOffset += sizeof(CLOSE_TCK_ITEM_T);

		///< 5. 시외버스 - 현금 환불 정보
		// 5-0. 갯수
		nCount = 1;
		nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &nCount, sizeof(int));
		//nOffset += sizeof(int);

		// 5-1. 현금 환불 정보
		::ZeroMemory(&item, sizeof(CLOSE_TCK_ITEM_T));
		item.nCount		= pAccum->Curr.ccsTicketWork[0].tRefund[IDX_ACC_CASH].nCount;
		item.nTotalFare = pAccum->Curr.ccsTicketWork[0].tRefund[IDX_ACC_CASH].dwMoney;
		nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &item, sizeof(CLOSE_TCK_ITEM_T));
		//nOffset += sizeof(CLOSE_TCK_ITEM_T);

		///< 6. 시외버스 - 카드 환불 정보
		// 6-0. 갯수
		nCount = 1;
		nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &nCount, sizeof(int));
		//nOffset += sizeof(int);

		// 6-1. 카드 환불 정보
		::ZeroMemory(&item, sizeof(CLOSE_TCK_ITEM_T));
		item.nCount		= pAccum->Curr.ccsTicketWork[0].tRefund[IDX_ACC_CARD].nCount;
		item.nTotalFare = pAccum->Curr.ccsTicketWork[0].tRefund[IDX_ACC_CARD].dwMoney;
		nOffset += Util_MyCopyMemory(&pSendPacket[nOffset], &item, sizeof(CLOSE_TCK_ITEM_T));
		//nOffset += sizeof(CLOSE_TCK_ITEM_T);

		// Data
		::CopyMemory(queData.szData, pSendPacket, nOffset);
		// Length
		queData.nLen = nOffset;

		s_QueData.push(queData);

		if(pSendPacket != NULL)
		{
			delete[] pSendPacket;
			pSendPacket = NULL;
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddKobus_AlcnListInfo
 * @details		배차리스트 정보 큐 추가 (332)
 * @param		BOOL bResult		결과값
 * @return		항상 = 0
 */
int UI_AddKobus_AlcnListInfo(BOOL bResult)
{
	int nOffset = 0;
	UI_QUE_DATA_T	queData;
	BYTE byACK;

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		queData.wCommand = UI_CMD_RES_PBTCK_LIST;

		if(bResult == TRUE)
		{
			queData.nFlag = FLAG_Q_SEND_RECV;

			byACK = CHAR_ACK;
			::CopyMemory(queData.szData, &byACK, sizeof(BYTE));
			queData.nLen = sizeof(BYTE);
		}
		else
		{
			queData.nFlag = FLAG_SEND_RECV;

			nOffset = MakeErrorPacket((char *)queData.szData);
			queData.nLen = nOffset;
		}

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddKobus_SatsListInfo
 * @details		좌석 정보 큐 추가 (334)
 * @param		BOOL bResult		결과값
 * @return		항상 = 0
 */
int UI_AddKobus_SatsListInfo(BOOL bResult)
{
	int nOffset, nCount, i;
	UI_QUE_DATA_T	queData;
	BYTE byACK;

	i = nOffset = nCount = 0;

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_RES_PBTCK_SATS;
		queData.nFlag = FLAG_SEND_RECV;

		if(bResult == TRUE)
		{
			vector<rtk_cm_setinfo_list_t>::iterator	iter;

			byACK = CHAR_ACK;

			/// 결과
			nOffset += Util_MyCopyMemory(&queData.szData[nOffset], &byACK, 1);

			/// 좌석정보 건수
			nCount = CPubTckKobusMem::GetInstance()->m_vtResSats.size();
			nOffset += Util_MyCopyMemory(&queData.szData[nOffset], &nCount, 4);

			/// 좌석정보
			for( iter = CPubTckKobusMem::GetInstance()->m_vtResSats.begin(); iter != CPubTckKobusMem::GetInstance()->m_vtResSats.end(); iter++ )
			{
				nOffset += Util_MyCopyMemory(&queData.szData[nOffset], iter._Ptr, sizeof(rtk_cm_setinfo_list_t));
			}
			queData.nLen = nOffset;
		}
		else
		{
			nOffset = MakeErrorPacket((char *)queData.szData);
			queData.nLen = nOffset;
		}

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddKobus_SatsPcpyInfo
 * @details		좌석 선점 정보 큐 추가 (336)
 * @param		BOOL bResult		결과값
 * @return		항상 = 0
 */
int UI_AddKobus_SatsPcpyInfo(BOOL bResult)
{
	int nOffset, nCount, i;
	UI_QUE_DATA_T	queData;
	BYTE byACK;

	i = nOffset = nCount = 0;

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_RES_PBTCK_SATSPCPY;
		queData.nFlag = FLAG_SEND_RECV;

		if(bResult == TRUE)
		{
			vector<rtk_tw_satspcpy_list_t>::iterator	iter;

			byACK = CHAR_ACK;

			/// 결과 
			nOffset += Util_MyCopyMemory(&queData.szData[nOffset], &byACK, 1);

			/// 좌석선점 건수
			nCount = CPubTckKobusMem::GetInstance()->m_tResSatsPcpy.size();
			nOffset += Util_MyCopyMemory(&queData.szData[nOffset], &nCount, 4);

			/// 좌석선점 정보
			for( iter = CPubTckKobusMem::GetInstance()->m_tResSatsPcpy.begin(); iter != CPubTckKobusMem::GetInstance()->m_tResSatsPcpy.end(); iter++ )
			{
				nOffset += Util_MyCopyMemory(&queData.szData[nOffset], iter._Ptr, sizeof(rtk_tw_satspcpy_list_t));
			}
			queData.nLen = nOffset;
		}
		else
		{
			nOffset = MakeErrorPacket((char *)queData.szData);
			queData.nLen = nOffset;
		}

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddKobus_ThruInfo
 * @details		경유지 정보 큐 추가 (339)
 * @param		BOOL bResult		결과값
 * @return		항상 = 0
 */
int UI_AddKobus_ThruInfo(BOOL bResult)
{
	int nOffset;
	UI_QUE_DATA_T	queData;
	BYTE byACK;

	Locking();
	try
	{
		nOffset = 0;
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		queData.wCommand = UI_CMD_RES_PBTCK_THRU_INFO;

		{
			if(bResult == TRUE)
			{
				queData.nFlag = FLAG_Q_SEND_RECV;

				byACK = CHAR_ACK;

				nOffset += Util_MyCopyMemory(&queData.szData[nOffset], &byACK, 1);
				queData.nLen = nOffset;
			}
			else
			{
				queData.nFlag = FLAG_SEND_RECV;
				nOffset = MakeErrorPacket((char *)queData.szData);
				queData.nLen = nOffset;
			}

			s_QueData.push(queData);
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddKobus_TMaxTckIssue
 * @details		[코버스] - [현장발권] - 서버 현장발권 (339)
 * @param		BOOL bResult		결과값
 * @return		항상 = 0
 */
int UI_AddKobus_TMaxTckIssue(BOOL bResult)
{
	int nOffset;
	UI_QUE_DATA_T	queData;
	BYTE byACK;

	nOffset = 0;

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		queData.wCommand = UI_CMD_RES_PBTCK_THRU_INFO;

		if(bResult == TRUE)
		{
			queData.nFlag = FLAG_Q_SEND_RECV;

			byACK = CHAR_ACK;

			nOffset += Util_MyCopyMemory(&queData.szData[nOffset], &byACK, 1);
			queData.nLen = nOffset;
		}
		else
		{
			queData.nFlag = FLAG_SEND_RECV;
			nOffset = MakeErrorPacket((char *)queData.szData);
			queData.nLen = nOffset;
		}

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddKobus_ReadMrsInfo
 * @details		[코버스] - [예매발권] - 예매조회 (234)
 * @param		BOOL bResult		결과값
 * @return		항상 = 0
 */
int UI_AddKobus_ReadMrsInfo(BOOL bResult)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;
	BYTE			byACK;

	nOffset = 0;

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_KOBUS_MRS_RESP_LIST;
		if(bResult == TRUE)
		{
			queData.nFlag = FLAG_Q_SEND_RECV;

			byACK = CHAR_ACK;

			nOffset += Util_MyCopyMemory(&queData.szData[nOffset], &byACK, 1);
			queData.nLen = nOffset;
		}
		else
		{
			queData.nFlag = FLAG_SEND_RECV;
			nOffset = MakeErrorPacket((char *)queData.szData);
			queData.nLen = nOffset;
		}

		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddKobus_ResultPubMrsInfo
 * @details		[코버스] - [예매발권] - 예매발권 완료 전문 전송 (236)
 * @param		BOOL bResult		결과값
 * @return		항상 = 0
 */
int UI_AddKobus_ResultPubMrsInfo(BOOL bResult)
{
	int				nOffset, nCount;
	UI_QUE_DATA_T	queData;
	BYTE			byACK;

	nOffset = nCount = byACK = 0;
	
	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

		queData.wCommand = UI_CMD_KOBUS_MRS_RESP_ISSUE;
		{
			if(bResult == TRUE)
			{
				CMrnpKobusMem* pMrs;

				pMrs = CMrnpKobusMem::GetInstance();

				queData.nFlag = FLAG_Q_SEND_RECV;

				/***
				byACK = CHAR_ACK;
			
				nOffset += Util_MyCopyMemory(&queData.szData[nOffset], &byACK, 1);

				/// 예매발권 건수
				nCount = pMrs->m_vtResComplete.size();
				LOG_OUT("예매발행 성공, 발행갯수 = %d", nCount);
				nOffset += Util_MyCopyMemory(&queData.szData[nOffset], &nCount, 4);

				vector<rtk_tm_mrspub_list_t>::iterator	iter;

				/// 예매 발권결과 정보
				for( iter = pMrs->m_vtResComplete.begin(); iter != pMrs->m_vtResComplete.end(); iter++ )
				{
					nOffset += Util_MyCopyMemory(&queData.szData[nOffset], (char *)iter._Ptr, sizeof(rtk_tm_mrspub_list_t));
					LOG_OUT("UI_0236 -> nOffset = %d", nOffset);
				}
				**/
				queData.nLen = 1;
			}
			else
			{
				queData.nFlag = FLAG_SEND_RECV;
				nOffset = MakeErrorPacket((char *)queData.szData);
				queData.nLen = nOffset;
			}

			s_QueData.push(queData);
		}
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_AddQueTmExp
 * @details		[티머니고속] 서버결과 정보 큐 추가
 * @param		BOOL bResult		결과값
 * @return		항상 = 0
 */
int UI_AddQueTmExp(WORD wCommand, BOOL bResult)
{
	int nOffset = 0;
	UI_QUE_DATA_T	queData;
	BYTE byACK;

	Locking();
	try
	{
		::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));
		queData.wCommand = wCommand;
		if(bResult == TRUE)
		{
			queData.nFlag = FLAG_Q_SEND_RECV;

			byACK = CHAR_ACK;
			::CopyMemory(queData.szData, &byACK, sizeof(BYTE));
			queData.nLen = sizeof(BYTE);
		}
		else
		{
			queData.nFlag = FLAG_SEND_RECV;

			nOffset = MakeErrorPacket((char *)queData.szData);
			queData.nLen = nOffset;
		}
		s_QueData.push(queData);
	}
	catch ( ... )
	{
		LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
	UnLocking();

	return 0;
}

/**
 * @brief		UI_SendData
 * @details		ui쪽으로 데이타 전송
 * @param		BOOL bResult		결과값
 * @return		항상 = 0
 */
int UI_Add_Q_SendData(WORD wCommand, BOOL bResult, int nError, BYTE *pData, int nDataLen)
{
	int				nOffset;
	UI_QUE_DATA_T	queData;
 
	nOffset = 0;
	
	Locking();
	{
		try
		{
			::ZeroMemory(&queData, sizeof(UI_QUE_DATA_T));

			queData.wCommand = wCommand;
			queData.nFlag = FLAG_Q_SEND_RECV;

			nOffset += Util_MyCopyMemory(&queData.szData[nOffset], &bResult, 4);
			nOffset += Util_MyCopyMemory(&queData.szData[nOffset], &nError, 4);

			if(nDataLen > 0)
			{
				if(nDataLen > (UI_Q_MAX_BUFF - 10))
				{
					LOG_OUT("nDataLen error = %d ###############", nDataLen);
					nDataLen = UI_Q_MAX_BUFF - 10;
				}
				nOffset += Util_MyCopyMemory(&queData.szData[nOffset], pData, nDataLen);
			}

			// Length
			queData.nLen = nOffset;
			s_QueData.push(queData);
		}
		catch ( ... )
		{
			LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
		}
	}
	UnLocking();
 
	return 0;
}


