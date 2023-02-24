// 
// 
// dev_tr_kobus_mem.cpp : kobus memory management
//

#include "stdafx.h"
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <queue>
#include <iostream>
#include <vector>

#include "MyDefine.h"
#include "MyUtil.h"
#include "cmn_util.h"
#include "oper_kos_file.h"
#include "md5.h"
#include "File_Env_ini.h"
#include "dev_tr_main.h"
#include "dev_tr_kobus_mem.h"
#include "dev_tr_mem.h"
#include "dev_tr_main.h"
#include "oper_config.h"

/**
 * @brief		CPubTckKobusMem
 * @details		[코버스] 생성자
 */
CPubTckKobusMem::CPubTckKobusMem()
{

}

/**
 * @brief		CPubTckKobusMem
 * @details		[코버스] 소멸자
 */
CPubTckKobusMem::~CPubTckKobusMem()
{

}

/**
 * @brief		Initialize
 * @details		[코버스] 데이타 초기화
 * @param		None
 * @return		None
 */
void CPubTckKobusMem::Initialize(void)
{
	KTC_MemClear(&base, sizeof(PUBTCK_KO_T));

	KTC_MemClear(&m_tReqAlcn, sizeof(UI_RESP_KO_REQ_ALCN_T));
	KTC_MemClear(&m_vtResAlcnInfo, sizeof(rtk_tm_timinfo_fee_t));
	
	m_vtResAlcnList.clear();
	vector<rtk_tm_timinfo_list_t>().swap(m_vtResAlcnList);

	KTC_MemClear(&m_tReqSats, sizeof(UI_RESP_KO_REQ_SATS_T));
	
	m_vtResSats.clear();
	vector<rtk_cm_setinfo_list_t>().swap(m_vtResSats);

	KTC_MemClear(&m_tReqSatsPcpy, sizeof(m_tReqSatsPcpy));
	
	m_tResSatsPcpy.clear();
	vector<rtk_tw_satspcpy_list_t>().swap(m_tResSatsPcpy);

	KTC_MemClear(&m_tReqTckIssue, sizeof(UI_RESP_KO_TCK_TRAN_T));
	KTC_MemClear(&m_tResTckIssueInfo, sizeof(rtk_tm_tcktran_info_t));
	
	m_tResTckIssueList.clear();
	vector<rtk_tm_tcktran_list_t>().swap(m_tResTckIssueList);

	
	KTC_MemClear(&m_tReqThru, sizeof(UI_RESP_KO_REQ_THRUINFO_T));

	m_vtResThruList.clear();
	vector<rtk_tm_ethruinfo_list_t>().swap(m_vtResThruList);

	m_vtPrtTicket.clear();
	vector<TCK_PRINT_FMT_T>().swap(m_vtPrtTicket);
	m_vtPrtTicketTest.clear();							// 20211116 ADD
	vector<TCK_PRINT_FMT_T>().swap(m_vtPrtTicketTest);	// 20211116 ADD

	// 재발권
	m_vtPbTckReIssue.clear();
	vector<rtk_tm_mrettran_list_t>().swap(m_vtPbTckReIssue);

	n_pbtck_chg_money = n_pbtck_count = n_total_tisu_amt = 0;

	TR_LOG_OUT("%s", "##### Memory release !!");
}		  

/**
 * @brief		MakeAlcnListPacket
 * @details		[코버스] 배차리스트 패킷 작성
 * @param		char *pSend		전송패킷 버퍼
 * @return		전송 데이타 길이
 */
int CPubTckKobusMem::MakeAlcnListPacket(char *pSend)
{
	int nOffset = 0, nCount = 0;
	vector<rtk_tm_timinfo_list_t>::iterator	iter;

	nCount = m_vtResAlcnList.size();

	/// ACK
	pSend[nOffset++] = CHAR_ACK;

	/// 배차정보
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &m_vtResAlcnInfo, sizeof(rtk_tm_timinfo_fee_t));

	/// 배차건수
	nCount = m_vtResAlcnList.size();
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &nCount, sizeof(int));

	/// 배차리스트
	for( iter = m_vtResAlcnList.begin(); iter != m_vtResAlcnList.end(); iter++ )
	{
		nOffset += ::Util_MyCopyMemory(&pSend[nOffset], iter._Ptr, sizeof(rtk_tm_timinfo_list_t));
	}
	return nOffset;
}

/**
 * @brief		MakeThruListPacket
 * @details		[코버스] 경유지 정보 응답 패킷 작성
 * @param		char *pSend		전송패킷 버퍼
 * @return		전송 데이타 길이
 */
int CPubTckKobusMem::MakeThruListPacket(char *pSend)
{
	int nOffset = 0, nCount = 0;
	vector<rtk_tm_ethruinfo_list_t>::iterator	iter;

	nCount = m_vtResThruList.size();

	/// ACK
	pSend[nOffset++] = CHAR_ACK;
	
	/// 경유지 수
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &nCount, 4);

	/// 경유지 정보
	for( iter = m_vtResThruList.begin(); iter != m_vtResThruList.end(); iter++ )
	{
		nOffset += ::Util_MyCopyMemory(&pSend[nOffset], iter._Ptr, sizeof(rtk_tm_ethruinfo_list_t));
	}
	return nOffset;
}

/**
 * @brief		MakeCreditTicketPrtData
 * @details		[코버스] 승차권 데이타 작성
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
/***
int CPubTckKobusMem::MakeCreditTicketPrtData(void)
{
 	int nRet = 0;
	int nWeek;
	SYSTEMTIME stDepr;
	char *pWeekStr[] = { "일", "월", "화", "수", "목", "금", "토", };
 
 	vector<rtk_tm_tcktran_list_t>::iterator iter;
 
	nWeek = 0;
	::ZeroMemory(&stDepr, sizeof(SYSTEMTIME));

	TR_LOG_OUT(" start !!!!");
 	m_vtPrtTicket.clear();
 	for(iter = m_tResTckIssueList.begin(); iter != m_tResTckIssueList.end(); iter++)
 	{
 		TCK_PRINT_FMT_T tPrtInfo;
 
 		::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
 
 		/// (01). 출발지 터미널명 (한글)
 		sprintf(tPrtInfo.depr_trml_ko_nm, "%s", iter->depr_hngl_nm);		
 		/// (02). 출발지 터미널명 (영문)
 		sprintf(tPrtInfo.depr_trml_eng_nm, "%s", iter->depr_eng_nm);	
 		/// (03). 도착지 터미널명 (한글)
		//sprintf(tPrtInfo.arvl_trml_ko_nm, "%s", iter->arvl_hngl_nm);		
 		sprintf(tPrtInfo.arvl_trml_ko_nm, "%s", iter->arvl_trml_abrv_nm);		
 		/// (04). 도착지 터미널명 (영문)
 		sprintf(tPrtInfo.arvl_trml_eng_nm, "%s", iter->arvl_trml_eng_nm);	
 		/// (05). 버스운수사 명
 		nRet = FindBusCacmName(SVR_DVS_KOBUS, m_tReqTckIssue.cacm_cd, tPrtInfo.bus_cacm_nm);	
 		/// (06). 버스등급명
 		nRet = FindBusClsName(SVR_DVS_KOBUS, m_tReqTckIssue.bus_cls_cd, tPrtInfo.bus_cls_nm);		

		CConfigTkMem::GetInstance()->GetKobusCorpInfo(m_tReqTckIssue.bus_cls_cd, tPrtInfo.bus_cacm_full_nm, tPrtInfo.bus_cacm_biz_no);

  		/// (07). 버스티켓종류명
		::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->tck_knd_cd, sizeof(tPrtInfo.bus_tck_knd_cd));
  		nRet = FindBusTckKndName(SVR_DVS_KOBUS, iter->tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 
 		/// (08). 신용카드 승인번호
 		sprintf(tPrtInfo.card_aprv_no, "%s", m_tResTckIssueInfo.card_aprv_no);
 
 		/// (09). 신용카드 승인금액, 요금
 		{
 			int nValue = 0;
 			char szFare[100];
 
			/// 승인금액
 			nValue = *(int *) m_tResTckIssueInfo.aprv_amt;
 			::ZeroMemory(szFare, sizeof(szFare));
 			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.card_aprv_amt, "%s", szFare);	

			/// 요금
			nValue = *(int *) iter->tissu_fee;
			::ZeroMemory(szFare, sizeof(szFare));
			tPrtInfo.n_tisu_amt = nValue;
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.tisu_amt, "%s", szFare);	
		}

 		/// (10). 카드번호
 		{
 			int len, i, k;
 			char Buffer[100];

 			::ZeroMemory(Buffer, sizeof(Buffer));
 
 			len = strlen(this->m_tReqTckIssue.card_no_mask);
 			for(k = 0, i = 0; i < len; i++)
 			{
 				if(this->m_tReqTckIssue.card_no_mask[i] != '-')
 				{
 					Buffer[k++] = this->m_tReqTckIssue.card_no_mask[i] & 0xFF;
 				}
 			}
 			
 			len = strlen(Buffer);
 			if(len > 12)
 			{
 				sprintf(tPrtInfo.card_no, "%.*s******%s", 6, &Buffer[0], &Buffer[12]);
 			}
 			else
 			{
 				sprintf(tPrtInfo.card_no, "%s", "****************");
 			}

			//sprintf(tPrtInfo.card_no, "%s", "****************");

 #if (_KTC_CERTIFY_ > 0)
 			KTC_MemClear(Buffer, sizeof(Buffer));
 #endif
 		}
 
 		/// (11). 결제수단 (현금/신용카드 등)
 		tPrtInfo.n_pym_dvs = PYM_CD_CARD;
		/// 카드 할부 개월수
		tPrtInfo.n_mip_mm_num = m_tReqTckIssue.mip_mm_num;
		if(tPrtInfo.n_mip_mm_num == 0)
		{
 			sprintf(tPrtInfo.pym_dvs, "현장카드(일시불)");	
		}
		else
		{
			sprintf(tPrtInfo.pym_dvs, "현장카드(%d 개월)", tPrtInfo.n_mip_mm_num);	
		}

#if 0
 		/// (12). 실제 출발 일자
		{
			nYear  = Util_Ascii2Long((char *)&m_tReqTckIssue.depr_dt[0], 4);
			nMonth = Util_Ascii2Long((char *)&m_tReqTckIssue.depr_dt[4], 2);
			nDay   = Util_Ascii2Long((char *)&m_tReqTckIssue.depr_dt[6], 2);

			CTime myTime(nYear, nMonth, nDay, 8, 0, 0);
			nWeek = myTime.GetDayOfWeek();
			if( (nWeek - 1) < 7 )
			{
				sprintf(tPrtInfo.atl_depr_dt, "%02d.%02d %s", nMonth, nDay, pWeekStr[nWeek - 1]);
			}
			else
			{
				sprintf(tPrtInfo.atl_depr_dt, "%02d.%02d", nMonth, nDay);
			}
			//sprintf(tPrtInfo.atl_depr_dt, "%.*s.%.*s", 2, &m_tReqTckIssue.depr_dt[4], 2, &m_tReqTckIssue.depr_dt[6]);	
		}
 		/// (13). 실제 출발 시간
 		sprintf(tPrtInfo.atl_depr_time, "%.*s:%.*s", 2, &m_tReqTckIssue.depr_time[0], 2, &m_tReqTckIssue.depr_time[2]);	
#endif

		/// (12). 실제 출발 일자
		{
			stDepr.wYear  = (WORD) Util_Ascii2Long((char *)&m_tReqTckIssue.depr_dt[0], 4);
			stDepr.wMonth = (WORD) Util_Ascii2Long((char *)&m_tReqTckIssue.depr_dt[4], 2);
			stDepr.wDay   = (WORD) Util_Ascii2Long((char *)&m_tReqTckIssue.depr_dt[6], 2);
		}

		/// (13). 실제 출발 일자
		
// 		if( memcmp(m_tReqTckIssue.depr_time, "24", 2) == 0 )
// 		{
// 			int nMinute;
// 			DWORD dwDeprTick;
// 			SYSTEMTIME *pstConv;
// 
// 			stDepr.wHour = stDepr.wMinute = stDepr.wSecond = 0;
// 
// 			dwDeprTick = Util_TickFromDateTime(&stDepr);
// 			dwDeprTick += 86400;
// 
// 			nMinute = Util_Ascii2Long((char *)&m_tReqTckIssue.depr_time[2], 2);
// 			dwDeprTick += (60 * nMinute);
// 
// 			pstConv = Util_DateFromTick(dwDeprTick);
// 			::CopyMemory(&stDepr, pstConv, sizeof(SYSTEMTIME));
// 		}
// 		else
// 		{
// 			stDepr.wHour   = (WORD) Util_Ascii2Long((char *)&m_tReqTckIssue.depr_time[0], 2);
// 			stDepr.wMinute = (WORD) Util_Ascii2Long((char *)&m_tReqTckIssue.depr_time[2], 2);
// 			stDepr.wSecond = 0;
// 		}

		CTime myTime((int)stDepr.wYear, (int)stDepr.wMonth, (int)stDepr.wDay, 8, 0, 0);
		nWeek = myTime.GetDayOfWeek();
		if( (nWeek - 1) < 7 )
		{
			sprintf(tPrtInfo.atl_depr_dt, "%02d.%02d %s", stDepr.wMonth, stDepr.wDay, pWeekStr[nWeek - 1]);
		}
		else
		{
			sprintf(tPrtInfo.atl_depr_dt, "%02d.%02d", stDepr.wMonth, stDepr.wDay);
		}

		/// (13). 실제 출발 시간
		sprintf(tPrtInfo.atl_depr_time, "%.*s:%.*s", 2, &m_tReqTckIssue.depr_time[0], 2, &m_tReqTckIssue.depr_time[2]);
 		
 		/// (14). 좌석번호
 		sprintf(tPrtInfo.sats_no, "%s", iter->sats_no);	
 		/// (15). 승차홈
 		sprintf(tPrtInfo.rdhm_val, "%s", m_tResTckIssueInfo.rot_rdhm_no_val);	
		TR_LOG_OUT("승차홈 = %s", tPrtInfo.rdhm_val);

		// (16). 발권시간
		sprintf(tPrtInfo.pub_time, "%s", this->base.pub_time);
		TR_LOG_OUT("발권시각 = %s", tPrtInfo.pub_time);
		
		/// (17). QR 바코드 관련
 		{
 			/// 1) 발행일자
			//sprintf(tPrtInfo.qr_pub_dt, "%s", m_tReqTckIssue.depr_dt);	
 			sprintf(tPrtInfo.qr_pub_dt, "%s", iter->tissu_dt);	
 			/// 2) 발행터미널코드
 			//sprintf(tPrtInfo.qr_pub_trml_cd, "%s", iter->tissu_trml_no);	
			sprintf(tPrtInfo.qr_pub_trml_cd, "%s", m_tReqTckIssue.trml_no);	
 			///// 3) 창구번호
 			//sprintf(tPrtInfo.qr_pub_wnd_no, "%s", iter->tissu_wnd_no);	
			sprintf(tPrtInfo.qr_pub_wnd_no, "%s", m_tReqTckIssue.wnd_no);	

 			/// 4) 발행일련번호
 			sprintf(tPrtInfo.qr_pub_sno, "%s", iter->tissu_sno);
			/// 5) 출발지 터미널번호
			sprintf(tPrtInfo.qr_depr_trml_no, "%s", m_tReqTckIssue.depr_trml_no);
			/// 6) 도착지 터미널번호
			sprintf(tPrtInfo.qr_arvl_trml_no, "%s", m_tReqTckIssue.arvl_trml_no);
			/// 7) 좌석번호
			sprintf(tPrtInfo.qr_sats_no, "%s", iter->sats_no);

			TR_LOG_OUT("qr발행일자 = %s", tPrtInfo.qr_pub_dt);
			TR_LOG_OUT("qr발행터미널번호 = %s", tPrtInfo.qr_pub_trml_cd);
			TR_LOG_OUT("qr발행창구번호 = %s", tPrtInfo.qr_pub_wnd_no);
			TR_LOG_OUT("qr발행일련번호 = %s", tPrtInfo.qr_pub_sno);
			TR_LOG_OUT("qr출발터미널번호 = %s", tPrtInfo.qr_depr_trml_no);
			TR_LOG_OUT("qr도착터미널번호 = %s", tPrtInfo.qr_arvl_trml_no);
			TR_LOG_OUT("qr좌석번호 = %s", tPrtInfo.qr_sats_no);
 		}
 
 		m_vtPrtTicket.push_back(tPrtInfo);
 
 #if (_KTC_CERTIFY_ > 0)
 		KTC_MemClear(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
 #endif
 
 	}
	TR_LOG_OUT(" end !!!!");

	return 0;
}
****/

/**
 * @brief		MakePasswdChars
 * @details		[코버스] 현장발권 - 구매자정보 문자 만들기
 * @param		char *retBuf		암호화 문자 버퍼
 * @param		char *pSrc			원본 데이타
 * @return		None
 */
void CPubTckKobusMem::MakePasswdChars(char *retBuf, char *pSrc)
{
	int nLen;

	/// 구매자 정보 (개인:핸드폰번호, 법인:사업자번호)
	nLen = strlen(pSrc);
	if(nLen <= 0)
	{
		return;
	}

	if(nLen < 4)
	{
		sprintf(retBuf, "****");
	}
	else
	{
		for(int i = 0; i < nLen; i++)
		{
			if( i < 3 )
			{
				retBuf[i] = pSrc[i];
			}
			else if( i >= (nLen - 4) )
			{
				retBuf[i] = pSrc[i];
			}
			else
			{
				retBuf[i] = '*';
			}
		}
		// ~2020.03.30 modify
	}
}

/**
 * @brief		MakeAllTicketPrtData
 * @details		[코버스] - 승차권 프린트 데이타 포맷 만들기
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int CPubTckKobusMem::MakeAllTicketPrtData(int n_pyn_dvs_cd, int ui_csrc_dvs_cd, int chit_use_dvs)
{
	int nRet = 0;
	int nWeek;
	int len, i, k;
	char Buffer[100];
	SYSTEMTIME stDepr;
	POPER_FILE_CONFIG_T pConfig;

	vector<rtk_tm_tcktran_list_t>::iterator iter;

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	nWeek = 0;
	::ZeroMemory(&stDepr, sizeof(SYSTEMTIME));

	TR_LOG_OUT(" start !!!!");
	m_vtPrtTicket.clear();
	for(iter = m_tResTckIssueList.begin(); iter != m_tResTckIssueList.end(); iter++)
	{
		TCK_PRINT_FMT_T tPrtInfo;

		::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));

		/// 개인/법인 구분코드
		tPrtInfo.ui_csrc_use[0] = m_tReqTckIssue.chit_use_dvs[0] - 0x30;

		/// (01). 출발지 터미널명 (한글)
		sprintf(tPrtInfo.depr_trml_ko_nm, "%s", iter->depr_trml_abrv_nm);		
		
		/// (02). 출발지 터미널명 (영문)
// 		if( memcmp(m_tReqSats.depr_trml_no, GetTrmlCode(SVR_DVS_KOBUS), 3) == 0 )
// 		{	/// 발권 터미널 == 출발 터미널, 자기 터미널..
// 			sprintf(tPrtInfo.depr_trml_eng_nm, "%s", CConfigTkMem::GetInstance()->depr_trml_eng_nm);	
// 		}
// 		else
// 		{	/// 왕복 터미널인 경우...
// 			Find_KobusTrmlName(LANG_ENG, m_tReqSats.depr_trml_no, tPrtInfo.depr_trml_eng_nm);
// 			//sprintf(tPrtInfo.depr_trml_eng_nm, "%s", iter->depr_eng_nm);	
// 		}
		Find_KobusTrmlName(LANG_ENG, m_tReqSats.depr_trml_no, tPrtInfo.depr_trml_eng_nm);
		/// (02). 출발지 터미널 사업자번호
		sprintf(tPrtInfo.depr_trml_biz_no, "%s", pConfig->koTrmlInfo_t.sz_prn_trml_corp_no1);	

		/// (03). 도착지 터미널명 (한글)
		//sprintf(tPrtInfo.arvl_trml_ko_nm, "%s", iter->arvl_hngl_nm);		
		//sprintf(tPrtInfo.arvl_trml_ko_nm, "%s", iter->arvl_trml_abrv_nm); // 20221229 DEL
		// 20221229 ADD~
		/// [코버스고속]전체 환승휴게소 도착지명에 (휴) 또는 휴게소 라고 추가
		/// 대상 환승지 터미널 코드;238,239,315,316,324,325,812,813,823,824,528,529
		if( (strcmp(m_tReqTckIssue.arvl_trml_no, "238") == 0) 
			|| (strcmp(m_tReqTckIssue.arvl_trml_no, "239") == 0)
			|| (strcmp(m_tReqTckIssue.arvl_trml_no, "315") == 0)
			|| (strcmp(m_tReqTckIssue.arvl_trml_no, "316") == 0)
			|| (strcmp(m_tReqTckIssue.arvl_trml_no, "324") == 0)
			|| (strcmp(m_tReqTckIssue.arvl_trml_no, "325") == 0)
			|| (strcmp(m_tReqTckIssue.arvl_trml_no, "812") == 0)
			|| (strcmp(m_tReqTckIssue.arvl_trml_no, "813") == 0)
			|| (strcmp(m_tReqTckIssue.arvl_trml_no, "823") == 0)
			|| (strcmp(m_tReqTckIssue.arvl_trml_no, "824") == 0)
			|| (strcmp(m_tReqTckIssue.arvl_trml_no, "528") == 0)
			|| (strcmp(m_tReqTckIssue.arvl_trml_no, "529") == 0)
			)
		{
			sprintf(tPrtInfo.arvl_trml_ko_nm, "%s휴게소", iter->arvl_trml_abrv_nm);		
		}
		else
		{
			sprintf(tPrtInfo.arvl_trml_ko_nm, "%s", iter->arvl_trml_abrv_nm);		
		}
		// 20221229 ~ADD

		/// (04). 도착지 터미널명 (영문)
		sprintf(tPrtInfo.arvl_trml_eng_nm, "%s", iter->arvl_trml_eng_nm);	

		/// (05). 배차 정보
		{
			/// 배차 출발지 코드
			sprintf(tPrtInfo.alcn_depr_trml_no, "%s", m_tReqTckIssue.alcn_depr_trml_no);
			/// 배차 출발지 이름
			//Find_KobusTrmlName(LANG_KOR, tPrtInfo.alcn_depr_trml_no, tPrtInfo.alcn_depr_trml_ko_nm);
			sprintf(tPrtInfo.alcn_depr_trml_ko_nm, "%s", iter->tdepr_trml_nm);

			/// 배차 도착지 코드
			sprintf(tPrtInfo.alcn_arvl_trml_no, "%s", m_tReqTckIssue.alcn_arvl_trml_no);
			/// 배차 도착지 이름
			//Find_KobusTrmlName(LANG_KOR, tPrtInfo.alcn_arvl_trml_no, tPrtInfo.alcn_arvl_trml_ko_nm);
			sprintf(tPrtInfo.alcn_arvl_trml_ko_nm, "%s", iter->tarvl_trml_nm);

			/// 배차 일시
			sprintf(tPrtInfo.alcn_depr_dt, "%.*s-%.*s-%.*s", 
					4, &m_tReqTckIssue.alcn_depr_dt[0], 
					2, &m_tReqTckIssue.alcn_depr_dt[4], 
					2, &m_tReqTckIssue.alcn_depr_dt[6]);
			sprintf(tPrtInfo.alcn_depr_time, "%.*s:%.*s", 
					2, &m_tReqTckIssue.alcn_depr_time[0],
					2, &m_tReqTckIssue.alcn_depr_time[2]);
		}

		/// (05). 버스운수사 명
		nRet = FindBusCacmName(SVR_DVS_KOBUS, m_tReqTckIssue.cacm_cd, tPrtInfo.bus_cacm_nm, tPrtInfo.bus_cacm_biz_no, "");	
		/// (06). 버스등급명
		nRet = FindBusClsName(SVR_DVS_KOBUS, m_tReqTckIssue.bus_cls_cd, tPrtInfo.bus_cls_nm);		
		/// (07). 버스티켓종류명
		::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->tck_knd_cd, sizeof(tPrtInfo.bus_tck_knd_cd));
		nRet = FindBusTckKndName(SVR_DVS_KOBUS, iter->tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 
		
		/// 거리
		::CopyMemory(tPrtInfo.dist, iter->bus_oprn_dist, sizeof(tPrtInfo.dist));

		//CConfigTkMem::GetInstance()->GetKobusCorpInfo(m_tReqTckIssue.bus_cls_cd, tPrtInfo.bus_cacm_full_nm, tPrtInfo.bus_cacm_biz_no);

		if(n_pyn_dvs_cd == PYM_CD_CARD)
		{
			/// (08). 신용카드 승인번호
			sprintf(tPrtInfo.card_aprv_no, "%s", m_tResTckIssueInfo.card_aprv_no);
		}
		else
		{
			/// (08). 현금영수증 승인번호
			sprintf(tPrtInfo.card_aprv_no, "%s", m_tResTckIssueInfo.csrc_aprv_no);
		}

		/// (09). 신용카드 승인금액, 요금
		{
			int nValue = 0;
			char szFare[100];

			/// 승인금액
			nValue = *(int *) m_tResTckIssueInfo.aprv_amt;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.card_aprv_amt, "%s", szFare);	

			/// 요금
			nValue = *(int *) iter->tissu_fee;
			::ZeroMemory(szFare, sizeof(szFare));
			tPrtInfo.n_tisu_amt = nValue;
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.tisu_amt, "%s", szFare);

			/// 기프트카드 잔액
			nValue = *(int *) m_tResTckIssueInfo.rmn_pnt;
			TR_LOG_OUT("[kobus] 기프트카드 잔액 = %d ....", nValue);
			if(nValue > 0)
			{
				::ZeroMemory(szFare, sizeof(szFare));
				Util_AmountComma(nValue, szFare);
				sprintf(tPrtInfo.card_tr_add_inf, "%s", szFare);
				tPrtInfo.trd_dvs_cd[0] = '3';
			}

		}

		/// (10). 카드번호
		if(n_pyn_dvs_cd == PYM_CD_CARD)
		{
			len = i = k = 0;
			::ZeroMemory(Buffer, sizeof(Buffer));

			len = strlen(this->m_tReqTckIssue.card_no_mask);
			for(k = 0, i = 0; i < len; i++)
			{
				if(this->m_tReqTckIssue.card_no_mask[i] != '-')
				{
					Buffer[k++] = this->m_tReqTckIssue.card_no_mask[i] & 0xFF;
				}
			}

			len = strlen(Buffer);
			if(len > 12)
			{
				sprintf(tPrtInfo.card_no, "%.*s******%s", 6, &Buffer[0], &Buffer[12]);
			}
			else
			{
				sprintf(tPrtInfo.card_no, "%s", "****************");
			}

			//sprintf(tPrtInfo.card_no, "%s", "****************");

#if (_KTC_CERTIFY_ > 0)
			KTC_MemClear(Buffer, sizeof(Buffer));
#endif
		}
		else
		{	/// 현금 - 구매자정보
			len = i = k = 0;
			::ZeroMemory(Buffer, sizeof(Buffer));

			MakePasswdChars(tPrtInfo.card_no, m_tResTckIssueInfo.user_key_val);

			/***
			len = strlen(m_tResTckIssueInfo.user_key_val);
			if(len > 5)
			{
				sprintf(Buffer, "%.*s***%.*s", 
						3, &m_tResTckIssueInfo.user_key_val[0], 
						4, &m_tResTckIssueInfo.user_key_val[len - 4]);
			}
			else
			{
				sprintf(Buffer, "%s", "****************");
			}
			****/
			//sprintf(tPrtInfo.card_no, "%s", Buffer);
		}

		/// (11). 결제수단 (현금/신용카드 등)
		//tPrtInfo.n_pym_dvs = PYM_CD_CARD;
		tPrtInfo.n_pym_dvs = n_pyn_dvs_cd;
		
		switch(n_pyn_dvs_cd)
		{
		case PYM_CD_CASH :
			{
				sprintf(tPrtInfo.pym_dvs, "현금(자진발급)");	
			}
			break;
		case PYM_CD_CSRC :
			{
				if( (ui_csrc_dvs_cd & 0xFF) == 1)
				{	/// 수기입력
					if( (chit_use_dvs & 0xFF) == 0x00)
					{
						sprintf(tPrtInfo.pym_dvs, "현금(소득공제)");	
					}
					else
					{
						sprintf(tPrtInfo.pym_dvs, "현금(지출증빙)");	
					}
				}
				else
				{
					//sprintf(tPrtInfo.pym_dvs, "현금(현금영수증)");	
					if( tPrtInfo.ui_csrc_use[0] == 0x00 )
					{
						sprintf(tPrtInfo.pym_dvs, "현금(소득공제)");	
					}
					else
					{
						sprintf(tPrtInfo.pym_dvs, "현금(지출증빙)");	
					}
				}
			}
			break;
		case PYM_CD_CARD :
			{
				/// 카드 할부 개월수
				tPrtInfo.n_mip_mm_num = m_tReqTckIssue.mip_mm_num;
				if(tPrtInfo.n_mip_mm_num == 0)
				{
					sprintf(tPrtInfo.pym_dvs, "현장카드(일시불)");	
				}
				else
				{
					sprintf(tPrtInfo.pym_dvs, "현장카드(%d 개월)", tPrtInfo.n_mip_mm_num);	
				}
			}
			break;
		case PYM_CD_RF :
			{
				sprintf(tPrtInfo.pym_dvs, "교통카드");	
			}
			break;
		}

		/// (12). 실제 출발일자
		if(GetConfigTicketPapaer() == PAPER_ROLL)
		{	/// 감열지
			int nWeek;
			SYSTEMTIME stDepr;
			char *pWeekStr[] = { "일", "월", "화", "수", "목", "금", "토", };

			stDepr.wYear   = (WORD) Util_Ascii2Long((char *)&m_tReqTckIssue.depr_dt[0], 4);
			stDepr.wMonth  = (WORD) Util_Ascii2Long((char *)&m_tReqTckIssue.depr_dt[4], 2);
			stDepr.wDay    = (WORD) Util_Ascii2Long((char *)&m_tReqTckIssue.depr_dt[6], 2);
			stDepr.wHour   = (WORD) Util_Ascii2Long((char *)&m_tReqTckIssue.depr_time[0], 2);
			stDepr.wMinute = (WORD) Util_Ascii2Long((char *)&m_tReqTckIssue.depr_time[2], 2);
			stDepr.wSecond = 0;

			CTime myTime((int)stDepr.wYear, (int)stDepr.wMonth, (int)stDepr.wDay, 8, 0, 0);
			nWeek = myTime.GetDayOfWeek();

			if( (nWeek - 1) < 7 )
			{
				sprintf(tPrtInfo.atl_depr_dt,   "%04d.%02d.%02d %s", stDepr.wYear, stDepr.wMonth, stDepr.wDay, pWeekStr[nWeek - 1]);
				sprintf(tPrtInfo.atl_depr_time, "%02d:%02d", stDepr.wHour, stDepr.wMinute);
			}
		}
		else
		{
			MakeDepartureDateTime(m_tReqTckIssue.depr_dt, m_tReqTckIssue.depr_time, tPrtInfo.atl_depr_dt, tPrtInfo.atl_depr_time);
		}

		/// (14). 좌석번호
		sprintf(tPrtInfo.sats_no, "%s", iter->sats_no);	
		/// (15). 승차홈
		sprintf(tPrtInfo.rdhm_val, "%s", m_tResTckIssueInfo.rot_rdhm_no_val);	
		TR_LOG_OUT("승차홈 = %s", tPrtInfo.rdhm_val);

		// (16). 발권시간
		sprintf(tPrtInfo.pub_time, "%s", this->base.pub_time);
		TR_LOG_OUT("발권시각 = %s", tPrtInfo.pub_time);

		/// (17). QR 바코드 관련
		{
			/// 1) 발행일자
			//sprintf(tPrtInfo.qr_pub_dt, "%s", m_tReqTckIssue.depr_dt);	
			sprintf(tPrtInfo.qr_pub_dt, "%s", iter->tissu_dt);	
			/// 2) 발행터미널코드
			//sprintf(tPrtInfo.qr_pub_trml_cd, "%s", iter->tissu_trml_no);	
			sprintf(tPrtInfo.qr_pub_trml_cd, "%s", m_tReqTckIssue.trml_no);	
			///// 3) 창구번호
			//sprintf(tPrtInfo.qr_pub_wnd_no, "%s", iter->tissu_wnd_no);	
			sprintf(tPrtInfo.qr_pub_wnd_no, "%s", m_tReqTckIssue.wnd_no);	

			/// 4) 발행일련번호
			sprintf(tPrtInfo.qr_pub_sno, "%s", iter->tissu_sno);
			/// 5) 출발지 터미널번호
			sprintf(tPrtInfo.qr_depr_trml_no, "%s", m_tReqTckIssue.depr_trml_no);
			/// 6) 도착지 터미널번호
			sprintf(tPrtInfo.qr_arvl_trml_no, "%s", m_tReqTckIssue.arvl_trml_no);
			/// 7) 좌석번호
			sprintf(tPrtInfo.qr_sats_no, "%s", iter->sats_no);

			TR_LOG_OUT("qr발행일자 = %s", tPrtInfo.qr_pub_dt);
			TR_LOG_OUT("qr발행터미널번호 = %s", tPrtInfo.qr_pub_trml_cd);
			TR_LOG_OUT("qr발행창구번호 = %s", tPrtInfo.qr_pub_wnd_no);
			TR_LOG_OUT("qr발행일련번호 = %s", tPrtInfo.qr_pub_sno);
			TR_LOG_OUT("qr출발터미널번호 = %s", tPrtInfo.qr_depr_trml_no);
			TR_LOG_OUT("qr도착터미널번호 = %s", tPrtInfo.qr_arvl_trml_no);
			TR_LOG_OUT("qr좌석번호 = %s", tPrtInfo.qr_sats_no);
		}

		m_vtPrtTicket.push_back(tPrtInfo);

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
#endif

	}
	TR_LOG_OUT(" end !!!!");

	return 0;
}

/**
 * @brief		MakeReTicketPrtData
 * @details		[코버스] - 재발행 티켓 데이타 작성
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int CPubTckKobusMem::MakeReTicketPrtData(void)
{
	int nRet = 0;
	int nWeek;
	//int len, i, k;
	//char Buffer[100];
	SYSTEMTIME stDepr;
	POPER_FILE_CONFIG_T pConfig;

	vector<rtk_tm_mrettran_list_t>::iterator iter;

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	nWeek = 0;
	::ZeroMemory(&stDepr, sizeof(SYSTEMTIME));

	TR_LOG_OUT(" start !!!!");
	m_vtPrtTicket.clear();
	for(iter = m_vtPbTckReIssue.begin(); iter != m_vtPbTckReIssue.end(); iter++)
	{
		TCK_PRINT_FMT_T tPrtInfo;

		::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));

#if 0
		/// (01). 출발지 터미널명 (한글)
		sprintf(tPrtInfo.depr_trml_ko_nm, "%s", iter->depr_trml_abrv_nm);		

		/// (02). 출발지 터미널명 (영문)
		// 		if( memcmp(m_tReqSats.depr_trml_no, GetTrmlCode(SVR_DVS_KOBUS), 3) == 0 )
		// 		{	/// 발권 터미널 == 출발 터미널, 자기 터미널..
		// 			sprintf(tPrtInfo.depr_trml_eng_nm, "%s", CConfigTkMem::GetInstance()->depr_trml_eng_nm);	
		// 		}
		// 		else
		// 		{	/// 왕복 터미널인 경우...
		// 			Find_KobusTrmlName(LANG_ENG, m_tReqSats.depr_trml_no, tPrtInfo.depr_trml_eng_nm);
		// 			//sprintf(tPrtInfo.depr_trml_eng_nm, "%s", iter->depr_eng_nm);	
		// 		}
		Find_KobusTrmlName(LANG_ENG, m_tReqSats.depr_trml_no, tPrtInfo.depr_trml_eng_nm);
		/// (02). 출발지 터미널 사업자번호
		sprintf(tPrtInfo.depr_trml_biz_no, "%s", pConfig->koTrmlInfo_t.sz_prn_trml_corp_no1);	

		/// (03). 도착지 터미널명 (한글)
		//sprintf(tPrtInfo.arvl_trml_ko_nm, "%s", iter->arvl_hngl_nm);		
		sprintf(tPrtInfo.arvl_trml_ko_nm, "%s", iter->arvl_trml_abrv_nm);		
		/// (04). 도착지 터미널명 (영문)
		sprintf(tPrtInfo.arvl_trml_eng_nm, "%s", iter->arvl_trml_eng_nm);	
		/// (05). 버스운수사 명
		nRet = FindBusCacmName(SVR_DVS_KOBUS, m_tReqTckIssue.cacm_cd, tPrtInfo.bus_cacm_nm, tPrtInfo.bus_cacm_biz_no, tPrtInfo.bus_cacm_tel_nm);	
		/// (06). 버스등급명
		nRet = FindBusClsName(SVR_DVS_KOBUS, m_tReqTckIssue.bus_cls_cd, tPrtInfo.bus_cls_nm);		
		/// (07). 버스티켓종류명
		::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->tck_knd_cd, sizeof(tPrtInfo.bus_tck_knd_cd));
		nRet = FindBusTckKndName(SVR_DVS_KOBUS, iter->tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 

		/// 거리
		::CopyMemory(tPrtInfo.dist, iter->bus_oprn_dist, sizeof(tPrtInfo.dist));

		//CConfigTkMem::GetInstance()->GetKobusCorpInfo(m_tReqTckIssue.bus_cls_cd, tPrtInfo.bus_cacm_full_nm, tPrtInfo.bus_cacm_biz_no);

		if(n_pyn_dvs_cd == PYM_CD_CARD)
		{
			/// (08). 신용카드 승인번호
			sprintf(tPrtInfo.card_aprv_no, "%s", m_tResTckIssueInfo.card_aprv_no);
		}
		else
		{
			/// (08). 현금영수증 승인번호
			sprintf(tPrtInfo.card_aprv_no, "%s", m_tResTckIssueInfo.csrc_aprv_no);
		}

		/// (09). 신용카드 승인금액, 요금
		{
			int nValue = 0;
			char szFare[100];

			/// 승인금액
			nValue = *(int *) m_tResTckIssueInfo.aprv_amt;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.card_aprv_amt, "%s", szFare);	

			/// 요금
			nValue = *(int *) iter->tissu_fee;
			::ZeroMemory(szFare, sizeof(szFare));
			tPrtInfo.n_tisu_amt = nValue;
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.tisu_amt, "%s", szFare);	
		}

		/// (10). 카드번호
		if(n_pyn_dvs_cd == PYM_CD_CARD)
		{
			len = i = k = 0;
			::ZeroMemory(Buffer, sizeof(Buffer));

			len = strlen(this->m_tReqTckIssue.card_no_mask);
			for(k = 0, i = 0; i < len; i++)
			{
				if(this->m_tReqTckIssue.card_no_mask[i] != '-')
				{
					Buffer[k++] = this->m_tReqTckIssue.card_no_mask[i] & 0xFF;
				}
			}

			len = strlen(Buffer);
			if(len > 12)
			{
				sprintf(tPrtInfo.card_no, "%.*s******%s", 6, &Buffer[0], &Buffer[12]);
			}
			else
			{
				sprintf(tPrtInfo.card_no, "%s", "****************");
			}

			//sprintf(tPrtInfo.card_no, "%s", "****************");

#if (_KTC_CERTIFY_ > 0)
			KTC_MemClear(Buffer, sizeof(Buffer));
#endif
		}
		else
		{	/// 현금 - 구매자정보
			len = i = k = 0;
			::ZeroMemory(Buffer, sizeof(Buffer));

			len = strlen(m_tResTckIssueInfo.user_key_val);
			if(len > 5)
			{
				sprintf(Buffer, "%.*s***%.*s", 
					3, &m_tResTckIssueInfo.user_key_val[0], 
					4, &m_tResTckIssueInfo.user_key_val[len - 4]);
			}
			else
			{
				sprintf(Buffer, "%s", "****************");
			}
			sprintf(tPrtInfo.card_no, "%s", Buffer);
		}

		/// (11). 결제수단 (현금/신용카드 등)
		//tPrtInfo.n_pym_dvs = PYM_CD_CARD;
		tPrtInfo.n_pym_dvs = n_pyn_dvs_cd;

		switch(n_pyn_dvs_cd)
		{
		case PYM_CD_CASH :
			{
				sprintf(tPrtInfo.pym_dvs, "현금(자진발급)");	
			}
			break;
		case PYM_CD_CSRC :
			{
				if( (ui_csrc_dvs_cd & 0xFF) == 1)
				{	/// 수기입력
					if( (chit_use_dvs & 0xFF) == '0')
					{
						sprintf(tPrtInfo.pym_dvs, "현금(소득공제)");	
					}
					else
					{
						sprintf(tPrtInfo.pym_dvs, "현금(지출증빙)");	
					}
				}
				else
				{
					sprintf(tPrtInfo.pym_dvs, "현금(현금영수증)");	
				}
			}
			break;
		case PYM_CD_CARD :
			{
				/// 카드 할부 개월수
				tPrtInfo.n_mip_mm_num = m_tReqTckIssue.mip_mm_num;
				if(tPrtInfo.n_mip_mm_num == 0)
				{
					sprintf(tPrtInfo.pym_dvs, "현장카드(일시불)");	
				}
				else
				{
					sprintf(tPrtInfo.pym_dvs, "현장카드(%d 개월)", tPrtInfo.n_mip_mm_num);	
				}
			}
			break;
		case PYM_CD_RF :
			{
				sprintf(tPrtInfo.pym_dvs, "교통카드");	
			}
			break;
		}

		/// (12). 실제 출발일자
		MakeDepartureDateTime(m_tReqTckIssue.depr_dt, m_tReqTckIssue.depr_time, tPrtInfo.atl_depr_dt, tPrtInfo.atl_depr_time);

		/// (14). 좌석번호
		sprintf(tPrtInfo.sats_no, "%s", iter->sats_no);	
		/// (15). 승차홈
		sprintf(tPrtInfo.rdhm_val, "%s", m_tResTckIssueInfo.rot_rdhm_no_val);	
		TR_LOG_OUT("승차홈 = %s", tPrtInfo.rdhm_val);

		// (16). 발권시간
		sprintf(tPrtInfo.pub_time, "%s", this->base.pub_time);
		TR_LOG_OUT("발권시각 = %s", tPrtInfo.pub_time);

		/// (17). QR 바코드 관련
		{
			/// 1) 발행일자
			//sprintf(tPrtInfo.qr_pub_dt, "%s", m_tReqTckIssue.depr_dt);	
			sprintf(tPrtInfo.qr_pub_dt, "%s", iter->tissu_dt);	
			/// 2) 발행터미널코드
			//sprintf(tPrtInfo.qr_pub_trml_cd, "%s", iter->tissu_trml_no);	
			sprintf(tPrtInfo.qr_pub_trml_cd, "%s", m_tReqTckIssue.trml_no);	
			///// 3) 창구번호
			//sprintf(tPrtInfo.qr_pub_wnd_no, "%s", iter->tissu_wnd_no);	
			sprintf(tPrtInfo.qr_pub_wnd_no, "%s", m_tReqTckIssue.wnd_no);	

			/// 4) 발행일련번호
			sprintf(tPrtInfo.qr_pub_sno, "%s", iter->tissu_sno);
			/// 5) 출발지 터미널번호
			sprintf(tPrtInfo.qr_depr_trml_no, "%s", m_tReqTckIssue.depr_trml_no);
			/// 6) 도착지 터미널번호
			sprintf(tPrtInfo.qr_arvl_trml_no, "%s", m_tReqTckIssue.arvl_trml_no);
			/// 7) 좌석번호
			sprintf(tPrtInfo.qr_sats_no, "%s", iter->sats_no);

			TR_LOG_OUT("qr발행일자 = %s", tPrtInfo.qr_pub_dt);
			TR_LOG_OUT("qr발행터미널번호 = %s", tPrtInfo.qr_pub_trml_cd);
			TR_LOG_OUT("qr발행창구번호 = %s", tPrtInfo.qr_pub_wnd_no);
			TR_LOG_OUT("qr발행일련번호 = %s", tPrtInfo.qr_pub_sno);
			TR_LOG_OUT("qr출발터미널번호 = %s", tPrtInfo.qr_depr_trml_no);
			TR_LOG_OUT("qr도착터미널번호 = %s", tPrtInfo.qr_arvl_trml_no);
			TR_LOG_OUT("qr좌석번호 = %s", tPrtInfo.qr_sats_no);
		}

		m_vtPrtTicket.push_back(tPrtInfo);
#endif

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
#endif

	}
	TR_LOG_OUT(" end !!!!");

	return 0;
}

/**
 * @brief		CMrnpKobusMem
 * @details		[코버스] 생성자
 */
CMrnpKobusMem::CMrnpKobusMem()
{

}

/**
 * @brief		~CMrnpKobusMem
 * @details		[코버스] 소멸자
 */
CMrnpKobusMem::~CMrnpKobusMem()
{

}

/**
 * @brief		Initialize
 * @details		[코버스] - 예매발권 메모리 초기화
 * @param		None
 * @return		항상 = 0
 */
void CMrnpKobusMem::Initialize(void)
{
	KTC_MemClear(&m_ReqList, sizeof(UI_RESP_KO_MRS_REQ_LIST_T));
	
	m_vtMrnpList.clear();
	vector<rtk_tm_mrsinfo_list_t>().swap(m_vtMrnpList);
	
	KTC_MemClear(&m_tUiSelData, sizeof(UI_RESP_KO_MRS_SEL_DATA_T));

	m_vtUiSelList.clear();
	vector<UI_KO_MRS_SEL_LIST_T>().swap(m_vtUiSelList);

	KTC_MemClear(rot_rdhm_no_val, sizeof(rot_rdhm_no_val));

	m_vtResComplete.clear();
	vector<rtk_tm_mrspub_list_t>().swap(m_vtResComplete);

	m_vtPrtTicket.clear();
	vector<TCK_PRINT_FMT_T>().swap(m_vtPrtTicket);

	TR_LOG_OUT("%s", "##### Memory release !!");
}

/**
 * @brief		CMrnpKobusMem::MakeMrnpListPacket
 * @details		[코버스] 예매내역 UI 전송 패킷 작성
 * @param		int nCount			예매내역 건수
 * @param		char *pData			UI 전송 버퍼
 * @return		전송 길이
 */
int CMrnpKobusMem::MakeMrnpListPacket(int nCount, char *pData)
{
	int i, nOffset;
	vector<rtk_tm_mrsinfo_list_t>::iterator	iter;

	i = nOffset = 0; 

	///< 결과
	pData[nOffset++] = CHAR_ACK;
	
	///< 예매건수
	nOffset += Util_MyCopyMemory(&pData[nOffset], &nCount, sizeof(int));	
	
	for( iter = m_vtMrnpList.begin(); iter != m_vtMrnpList.end(); iter++ )
	{
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->mrs_mrnp_no,		sizeof(iter->mrs_mrnp_no		));	///< 예매예약번호_arr
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->mrs_mrnp_sno,		sizeof(iter->mrs_mrnp_sno		));	///< 예매예약일련번호_arr							
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->mrs_mrnp_dt,		sizeof(iter->mrs_mrnp_dt		));	///< 예매예약일자_arr								
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->mrs_mrnp_time,		sizeof(iter->mrs_mrnp_time		));	///< 예매예약시각_arr								
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->mbrs_no,			sizeof(iter->mbrs_no			));	///< 회원번호_arr								
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->depr_trml_no,		sizeof(iter->depr_trml_no		));	///< 출발터미널번호_arr							
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->arvl_trml_no,		sizeof(iter->arvl_trml_no		));	///< 도착터미널번호_arr							
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->alcn_depr_trml_no,	sizeof(iter->alcn_depr_trml_no	));	///< 배차출발터미널번호_arr							
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->alcn_arvl_trml_no,	sizeof(iter->alcn_arvl_trml_no	));	//< 배차도착터미널번호_arr							
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->depr_dt,			sizeof(iter->depr_dt			));	///< 출발일자_arr								
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->depr_time,			sizeof(iter->depr_time			));	///< 출발시각_arr								
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->cacm_cd,			sizeof(iter->cacm_cd			));	///< 운수사코드_arr								
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->bus_cls_cd,			sizeof(iter->bus_cls_cd			));///< 버스등급코드_arr								
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->mrs_chnl_dvs_cd,	sizeof(iter->mrs_chnl_dvs_cd	));///< 예매채널구분코드_arr							
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->pub_chnl_dvs_cd,	sizeof(iter->pub_chnl_dvs_cd	));///< 발행채널구분코드_arr							
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->pyn_dtl_cd,			sizeof(iter->pyn_dtl_cd			));///< 지불상세코드_arr								
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->adlt_num,			sizeof(iter->adlt_num			));///< (int)어른수_arr									
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->adlt_tot_amt,		sizeof(iter->adlt_tot_amt		));///< (int)어른총금액_arr								
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->chld_num,			sizeof(iter->chld_num			));///< (int)어린이수_arr								
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->chld_tot_amt,		sizeof(iter->chld_tot_amt		));///< (int)어린이총금액_arr								
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->teen_num,			sizeof(iter->teen_num			));///< (int)청소년수_arr								
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->teen_tot_amt,		sizeof(iter->teen_tot_amt		));///< (int)청소년총금액_arr								
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->uvsd_num,			sizeof(iter->uvsd_num			));///< (int)대학생수_arr								
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->uvsd_tot_amt,		sizeof(iter->uvsd_tot_amt		));///< (int)대학생총금액_arr				
		// 20210614 ADD
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->sncn_num,			sizeof(iter->sncn_num			));///< (int)경로수_arr			
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->sncn_tot_amt,		sizeof(iter->sncn_tot_amt		));///< (int)경로총금액_arr			
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->dspr_num,			sizeof(iter->dspr_num			));///< (int)장애인수_arr			
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->dspr_tot_amt,		sizeof(iter->dspr_tot_amt		));///< (int)장애인총금액_arr			
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->vtr3_num,			sizeof(iter->vtr3_num			));///< (int)보훈30수_arr			
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->vtr3_tot_amt,		sizeof(iter->vtr3_tot_amt		));///< (int)보훈30총금액_arr			
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->vtr5_num,			sizeof(iter->vtr5_num			));///< (int)보훈50수_arr			
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->vtr5_tot_amt,		sizeof(iter->vtr5_tot_amt		));///< (int)보훈50총금액_arr			
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->vtr7_num,			sizeof(iter->vtr7_num			));///< (int)보훈70수_arr			
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->vtr7_tot_amt,		sizeof(iter->vtr7_tot_amt		));///< (int)보훈70총금액_arr			
		// 20210614 ~ADD
		// 20220713 ADD
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->dfpt_num,			sizeof(iter->dfpt_num			));///< (int)후불수_arr
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->dfpt_tot_amt,		sizeof(iter->dfpt_tot_amt		));///< (int)후불총금액_arr
		// 20220713 ~ADD
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->sats_num,			sizeof(iter->sats_num			));///< (int)좌석수_arr									
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->ride_psb_fcnt,		sizeof(iter->ride_psb_fcnt		));///< 정기정액권 탑승가능횟수_arr						
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->tot_ride_psb_fcnt,	sizeof(iter->tot_ride_psb_fcnt	));///< 정기정액권 총승차가능횟수_arr						
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->rtrp_dvs_cd1,		sizeof(iter->rtrp_dvs_cd1		));///< 왕복구분코드1_arr(1:편도,2:왕복,3:시외우등할인왕복)	
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->depr_hngl_nm,		sizeof(iter->depr_hngl_nm		));///< 출발터미널한글명_arr							
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->arvl_hngl_nm,		sizeof(iter->arvl_hngl_nm		));///< 도착터미널한글명_arr							
	}

	return nOffset;
}

int CMrnpKobusMem::MakeReqIssuePacket(char *pData)
{
	return 0;
}

/**
 * @brief		MakePubMrnpResultPacket
 * @details		[코버스] 예매발권 결과 UI 전송 패킷 작성
 * @param		char *pSPacket		예매발권 결과 데이타 버퍼
 * @return		전송 길이
 */
int CMrnpKobusMem::MakePubMrnpResultPacket(char *pSPacket)
{
	int nOffset, nCount;
	BYTE byACK = CHAR_ACK;

	nOffset = nCount = 0;

	/// 결과
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], &byACK, 1);

	/// 예매발권 건수
	nCount = this->m_vtResComplete.size();
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], &nCount, 4);

	vector<rtk_tm_mrspub_list_t>::iterator	iter;

	/// 예매 발권결과 정보
	for( iter = this->m_vtResComplete.begin(); iter != this->m_vtResComplete.end(); iter++ )
	{
		nOffset += Util_MyCopyMemory(&pSPacket[nOffset], (char *)iter._Ptr, sizeof(rtk_tm_mrspub_list_t));
	}

	return nOffset;
}

/**
 * @brief		MakeTicketPrtData
 * @details		[코버스] 예매발권 승차권 데이타 작성
 * @param		None
 * @return		항상 = 0
 */
int CMrnpKobusMem::MakeTicketPrtData(void)
{
 	int nRet, n_mip_mm_num;
	int nYear, nMonth, nDay, nWeek;
	char *pWeekStr[] = { "일", "월", "화", "수", "목", "금", "토", };
	POPER_FILE_CONFIG_T pConfig;

 	vector<rtk_tm_mrspub_list_t>::iterator iter;

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	nYear = nMonth = nDay = nWeek = 0;

	nRet = n_mip_mm_num = 0;

 	m_vtPrtTicket.clear();
 	for(iter = m_vtResComplete.begin(); iter != m_vtResComplete.end(); iter++)
 	{
 		TCK_PRINT_FMT_T tPrtInfo;
 
 		::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
 
 		/// (01). 출발지 터미널명 (한글)
 		sprintf(tPrtInfo.depr_trml_ko_nm, "%s", iter->depr_trml_abrv_nm);		
 		
		/// (02). 출발지 터미널명 (영문)
		Find_KobusTrmlName(LANG_ENG, iter->depr_trml_no, tPrtInfo.depr_trml_eng_nm);
 		//sprintf(tPrtInfo.depr_trml_eng_nm, "%s", iter->depr_eng_nm);	
		
		/// (02). 출발지 터미널 사업자번호
		sprintf(tPrtInfo.depr_trml_biz_no, "%s", pConfig->koTrmlInfo_t.sz_prn_trml_corp_no1);	

 		/// (03). 도착지 터미널명 (한글)
 		sprintf(tPrtInfo.arvl_trml_ko_nm, "%s", iter->arvl_trml_abrv_nm);		
 		
		/// (04). 도착지 터미널명 (영문)
 		sprintf(tPrtInfo.arvl_trml_eng_nm, "%s", iter->arvl_trml_eng_nm);	
 		
		/// (05). 버스운수사 명
 		nRet = FindBusCacmName(SVR_DVS_KOBUS, iter->cacm_cd, tPrtInfo.bus_cacm_nm, tPrtInfo.bus_cacm_biz_no, tPrtInfo.bus_cacm_tel_nm);
 		
		/// (06). 버스등급명
 		nRet = FindBusClsName(SVR_DVS_KOBUS, iter->bus_cls_cd, tPrtInfo.bus_cls_nm);		
 		
		/// (07). 버스티켓종류명
 		nRet = FindBusTckKndName(SVR_DVS_KOBUS, iter->tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 
		//CConfigTkMem::GetInstance()->GetKobusCorpInfo(iter->bus_cls_cd, tPrtInfo.bus_cacm_full_nm, tPrtInfo.bus_cacm_biz_no);
		::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->tck_knd_cd, sizeof(iter->tck_knd_cd));
 		
		/// (08). 신용카드 승인번호
 		sprintf(tPrtInfo.card_aprv_no, "%s", iter->card_aprv_no);
 		
		/// (09). 신용카드 승인금액
 		{
 			int nValue = 0;
 			char szFare[100];
 
			/// 승인금액
 			nValue = *(int *) iter->ogn_aprv_amt;
 			::ZeroMemory(szFare, sizeof(szFare));
 			Util_AmountComma(nValue, szFare);
 			sprintf(tPrtInfo.card_aprv_amt, "%s", szFare);	

			/// 요금
			nValue = *(int *) iter->tissu_fee;
			tPrtInfo.n_tisu_amt = nValue;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.tisu_amt, "%s", szFare);	
 		}
 		
		/// (10). 카드번호
 		{
 			int len, i, k;
 
 			len = strlen(iter->card_no);
 			for(k = 0, i = 0; i < len; i++)
 			{
 				if(iter->card_no[i] != '-')
 				{
 					tPrtInfo.card_no[k++] = iter->card_no[i] & 0xFF;
 				}
 			}
 		}

 		int n_mip_mm_num = *(int *)iter->mip_mm_num;

		/// (11). 결제수단 (현금/신용카드 등)
		// 200917 추가
		switch(iter->pyn_dtl_cd[0])
		{
		case '1'://신용카드
			tPrtInfo.n_pym_dvs = PYM_CD_CARD;
			if( n_mip_mm_num <= 0)
			{
 				sprintf(tPrtInfo.pym_dvs, "예매카드(일시불)");	
			}
			else
			{
				sprintf(tPrtInfo.pym_dvs, "예매카드(%d개월)", n_mip_mm_num);	
			}
			break;
			
		case 'd'://스마일페이
		case 'e'://티머니페이
		case 'f'://비즈페이
		case 'g'://페이코
			tPrtInfo.n_pym_dvs = PYM_CD_TPAY;
			if( n_mip_mm_num <= 0)
			{
 				sprintf(tPrtInfo.pym_dvs, "페이(일시불)");	
			}
			else
			{
				sprintf(tPrtInfo.pym_dvs, "페이(%d개월)", n_mip_mm_num);	
			}
			break;

		// 20210910 ADD
		//case 'c': // 복합(복합결제 대표코드) -> 조회 시에만, 발권 시에는 이하 코드값 수신
		//case 'd': //스마일페이
		//case 'e': //티머니페이
		//case 'f': //비즈페이
		//case 'g': //payco
		case 'h': //후불단독 
		case 'i': //가상선불 단독 
		case 'j': //마일리지 단독
		case 'k': //마일리지 + 쿠폰
		case 'l': //후불 + 마일리지
		case 'm': //후불 + 쿠폰
		case 'n': //후불 + 마일리지 + 쿠폰
		case 'o': //가상선불 + 마일리지
		case 'p': //가상선불 + 쿠폰
		case 'q': //가상선불 + 마일리지 + 쿠폰
			tPrtInfo.n_pym_dvs = PYM_CD_ETC;
			if( n_mip_mm_num <= 0)
			{
 				sprintf(tPrtInfo.pym_dvs, "복합결제");	
			}
			else
			{
				sprintf(tPrtInfo.pym_dvs, "복합결제(%d개월)", n_mip_mm_num);	
			}
			break;

		default:
			tPrtInfo.n_pym_dvs = PYM_CD_ETC;
 			sprintf(tPrtInfo.pym_dvs, "기타");	
			break;
		// 20210910 ~ADD
		}
 		//tPrtInfo.n_pym_dvs = PYM_CD_CARD;
		
		// 20211019 ADD
		/// 이벤트쿠폰결제금액	
		char szFare[100];
		int nValue = *(int *) iter->cpn_rmn_amt;
		TR_LOG_OUT("이벤트쿠폰결제금액 정보 : %d ", nValue);	// 20211019 ADD for DEBUG
		::ZeroMemory(szFare, sizeof(szFare));
		Util_AmountComma(nValue, szFare);
		sprintf(tPrtInfo.cpn_rmn_amt, "%s", szFare);	

		///	마일리지결제금액		
		nValue = *(int *) iter->mlg_rmn_amt;
		TR_LOG_OUT("마일리지결제금액 정보 : %d ", nValue);	// 20211019 ADD for DEBUG
		::ZeroMemory(szFare, sizeof(szFare));
		Util_AmountComma(nValue, szFare);
		sprintf(tPrtInfo.mlg_rmn_amt, "%s", szFare);	
		// 20211019 ~ADD
		
		if(GetConfigTicketPapaer() == PAPER_ROLL)
		{	/// 감열지
			int nWeek;
			SYSTEMTIME stDepr;
			char *pWeekStr[] = { "일", "월", "화", "수", "목", "금", "토", };

			stDepr.wYear   = (WORD) Util_Ascii2Long((char *)&iter->depr_dt[0], 4);
			stDepr.wMonth  = (WORD) Util_Ascii2Long((char *)&iter->depr_dt[4], 2);
			stDepr.wDay    = (WORD) Util_Ascii2Long((char *)&iter->depr_dt[6], 2);
			stDepr.wHour   = (WORD) Util_Ascii2Long((char *)&iter->depr_time[0], 2);
			stDepr.wMinute = (WORD) Util_Ascii2Long((char *)&iter->depr_time[2], 2);
			stDepr.wSecond = 0;

			CTime myTime((int)stDepr.wYear, (int)stDepr.wMonth, (int)stDepr.wDay, 8, 0, 0);
			nWeek = myTime.GetDayOfWeek();

			if( (nWeek - 1) < 7 )
			{
				sprintf(tPrtInfo.atl_depr_dt,   "%04d.%02d.%02d %s", stDepr.wYear, stDepr.wMonth, stDepr.wDay, pWeekStr[nWeek - 1]);
				sprintf(tPrtInfo.atl_depr_time, "%02d:%02d", stDepr.wHour, stDepr.wMinute);
			}
		}
		else
		{
			MakeDepartureDateTime(iter->depr_dt, iter->depr_time, tPrtInfo.atl_depr_dt, tPrtInfo.atl_depr_time);
		}

 		/// (14). 좌석번호
 		sprintf(tPrtInfo.sats_no, "%s", iter->sats_no);	
 		
		/// (15). 승차홈
 		sprintf(tPrtInfo.rdhm_val, "%s", this->rot_rdhm_no_val);	
 		
		/// (16) 거리 
 		sprintf(tPrtInfo.dist, "%s", iter->bus_oprn_dist);	

		/// (17). QR코드 관련
 		{
			SYSTEMTIME st;

			GetLocalTime(&st);

			/// 발행일자 (8)
 			sprintf(tPrtInfo.qr_pub_dt, "%s", iter->tissu_dt);	
 			/// 발행터미널코드 (3)
 			sprintf(tPrtInfo.qr_pub_trml_cd, "%s", iter->tissu_trml_no);	
 			/// 발행창구번호 (2)
 			sprintf(tPrtInfo.qr_pub_wnd_no, "%s", iter->tissu_wnd_no);	
 			/// 발행일련번호 (4)
 			sprintf(tPrtInfo.qr_pub_sno, "%s", iter->tissu_sno);
			/// 출발 터미널번호
			sprintf(tPrtInfo.qr_depr_trml_no, "%s", iter->depr_trml_no);
			/// 도착 터미널번호
			sprintf(tPrtInfo.qr_arvl_trml_no, "%s", iter->arvl_trml_no);
			/// 좌석번호
			sprintf(tPrtInfo.qr_sats_no, "%s", iter->sats_no);
			// 발권시간
			sprintf(tPrtInfo.pub_time, "(%02d:%02d)", st.wHour, st.wMinute);
 		}
 		m_vtPrtTicket.push_back(tPrtInfo);
 
 #if (_KTC_CERTIFY_ > 0)
 		KTC_MemClear(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
 #endif
 	}

	return 0;
}

/**
 * @brief		CCancRyTkKobusMem
 * @details		[코버스] 환불관련 class 생성자
 */
CCancRyTkKobusMem::CCancRyTkKobusMem()
{
}

/**
 * @brief		~CCancRyTkKobusMem
 * @details		[코버스] 환불관련 class 소멸자
 */
CCancRyTkKobusMem::~CCancRyTkKobusMem()
{

}

/**
 * @brief		Initialize
 * @details		[코버스] 환불 메모리 초기화
 * @param		None
 * @return		항상 = 0
 */
void CCancRyTkKobusMem::Initialize(void)
{
	KTC_MemClear(&tBase, sizeof(CANCRY_KOBUS_BASE_T));

	KTC_MemClear(&tReq, sizeof(stk_tm_ryamtinfo_list_t));
	KTC_MemClear(&tRespInq, sizeof(rtk_tm_ryamtinfo_dt_t));
	KTC_MemClear(&tRespList, sizeof(rtk_tm_ryamtinfo_list_t));
	KTC_MemClear(&tRespOk, sizeof(rtk_tm_reptran_t));
	KTC_MemClear(&tRespWndOk, sizeof(rtk_tm_tckcan_t));

	TR_LOG_OUT("%s", "##### Memory release !!");
}

/**
 * @brief		CalcRefund
 * @details		[코버스] 승차권 환불 계산하기
 * @param		None
 * @return		성공 > 0, 실패 < 0
 */
int CCancRyTkKobusMem::CalcRefund(void)
{
	int					nLen;
	POPER_FILE_CONFIG_T	pConfig;
	PFMT_QRCODE_T		pQRCode;
	char				*pIssTrmlCode;

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();
	pQRCode = (PFMT_QRCODE_T) &tBase.szTicketData;
	
	pIssTrmlCode = GetTrmlCode(SVR_DVS_KOBUS);
	nLen = strlen(pIssTrmlCode);
	if(nLen < 0)
	{
		TR_LOG_OUT("터미널 코드 없음 에러, 터미널코드 = %s ", pIssTrmlCode);
		return -1;
	}

	/// 출발지 터미널코드,  도착지 터미널코드 체크
	{
		if( (memcmp(pQRCode->depr_trml_no, pIssTrmlCode, 3) != 0) && (memcmp(pQRCode->arvl_trml_no, pIssTrmlCode, 3) != 0) )
		{
			TR_LOG_OUT("자기 터미널 코드 아님 에러, 터미널코드 = (%s)(%s) ", pQRCode->depr_trml_no, pQRCode->arvl_trml_no);
			return -9;
		}
	}

	/// (01). 결제금액
	::CopyMemory( &tBase.n_tot_money,		tRespInq.tissu_fee	, 4 );
	/// (02). 취소 수수료 금액
	::CopyMemory( &tBase.n_commission_fare,	tRespInq.ry_pfit	, 4 );
	/// (03). 수수료율
	::CopyMemory( &tBase.n_commission_rate, tRespInq.cmrt		, 4 );
	if(tBase.n_commission_rate > 100)
	{
		TR_LOG_OUT("수수료율 error = %d", tBase.n_commission_rate);
		tBase.n_commission_rate = 100;
	}
	tBase.n_commission_rate = 100 - tBase.n_commission_rate;

	/// (04). ui 결제수단 (1:현금, 2:카드)
	switch( tRespInq.pyn_dtl_cd[0] )
	{
	case '1' :
		tBase.ui_pym_dvs_cd[0] = PYM_CD_CARD;
		break;
	case '2' :
	case '3' :
		tBase.ui_pym_dvs_cd[0] = PYM_CD_CASH;
		break;
	case 'd'://스마일페이
	case 'e'://티머니페이
	case 'f'://비즈페이
	case 'g'://페이코
	// 20230104 ADD~
	case 'r':	/// r:토스페이
	case 's':	/// s:네이버페이
	// 20230104 ~ADD
		tBase.ui_pym_dvs_cd[0] = PYM_CD_TPAY;
		break;
	// 20210910 ADD~
	//case 'c': // 복합(복합결제 대표코드) -> 조회 시에만, 발권 시에는 이하 코드값 수신
	//case 'd': //스마일페이
	//case 'e': //티머니페이
	//case 'f': //비즈페이
	//case 'g': //payco
	case 'h': //후불단독 
	case 'i': //가상선불 단독 
	case 'j': //마일리지 단독
	case 'k': //마일리지 + 쿠폰
	case 'l': //후불 + 마일리지
	case 'm': //후불 + 쿠폰
	case 'n': //후불 + 마일리지 + 쿠폰
	case 'o': //가상선불 + 마일리지
	case 'p': //가상선불 + 쿠폰
	case 'q': //가상선불 + 마일리지 + 쿠폰
		tBase.ui_pym_dvs_cd[0] = PYM_CD_ETC;

		break;
	// 20210910 ~ADD
	default:
		TR_LOG_OUT("결제수단 에러, pyn_dtl_cd = 0x%02X ", tRespInq.pyn_dtl_cd[0]);
		return -1;
	}

	TR_LOG_OUT("[고속 환불_서버 조회 정보]");
	TR_LOG_OUT("1. 결제 금액   = %d", tBase.n_tot_money);
	TR_LOG_OUT("2. 수수료 금액 = %d", tBase.n_commission_fare);
	TR_LOG_OUT("3. 수수료율    = %d", tBase.n_commission_rate);
	if(tBase.ui_pym_dvs_cd[0] == PYM_CD_CARD)
	{
		TR_LOG_OUT("3. 결제수단    = 신용카드");
	}
	else
	{
		TR_LOG_OUT("3. 결제수단    = 현금");
	}

	switch(tBase.ui_pym_dvs_cd[0])
	{
	case PYM_CD_CASH :
	case PYM_CD_CSRC :
		/// 카드전용
		if( pConfig->base_t.sw_cash_pay_yn != 'Y' )
		{	
			TR_LOG_OUT("시외버스_환불 - 현금환불 불가 error !!! ");
			return -99;
		}
		break;
	case PYM_CD_CARD :
		if( pConfig->base_t.sw_ic_pay_yn != 'Y' )
		{	
			TR_LOG_OUT("시외버스_환불 - 카드환불 불가 error !!! ");
			return -99;
		}
		break;
	}

	return 1;
}



