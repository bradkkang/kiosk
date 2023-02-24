// 
// 
// dev_tr_tmexp_mem.cpp : tmoney express bus memory management
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
#include "dev_tr_tmexp_mem.h"
#include "dev_tr_mem.h"
#include "dev_tr_main.h"
#include "oper_config.h"
#include "cmn_util.h"

/**
 * @brief		CPubTckTmExpMem
 * @details		[티머니고속] 생성자
 */
CPubTckTmExpMem::CPubTckTmExpMem()
{

}

/**
 * @brief		CPubTckTmExpMem
 * @details		[티머니고속] 소멸자
 */
CPubTckTmExpMem::~CPubTckTmExpMem()
{

}

/**
 * @brief		Initialize
 * @details		[티머니고속] 데이타 초기화
 * @param		None
 * @return		None
 */
void CPubTckTmExpMem::Initialize(void)
{
	/// 기본
	KTC_MemClear(&base, sizeof(PUBTCK_TMEXP_T));

	/// 배차
	KTC_MemClear(&m_tReqAlcn, sizeof(stk_tm_readalcn_t));
	m_vtResAlcnLastList.clear();
	vector<rtk_tm_readalcn_last_list_t>().swap(m_vtResAlcnLastList);
	m_vtResAlcnList.clear();
	vector<rtk_tm_readalcn_list_t>().swap(m_vtResAlcnList);

	/// 좌석
	KTC_MemClear(&m_tReqSats, sizeof(stk_tm_readsatsfee_t));
	m_vtResFee.clear();
	vector<rtk_tm_readsatsfee_list_t>().swap(m_vtResFee);
	m_vtResDisc.clear();
	vector<rtk_tm_readsats_disc_list_t>().swap(m_vtResDisc);

	/// 선점
	KTC_MemClear(&m_tReqSatsPcpy, sizeof(stk_tm_pcpysats_t));
	m_tResSatsPcpy.clear();
	vector<rtk_tm_pcpysats_list_t>().swap(m_tResSatsPcpy);

	/// 발권-현금
	KTC_MemClear(&m_tReqPubTckCash, sizeof(stk_tm_pubtckcash_t));
	KTC_MemClear(&m_tResPubTckCash, sizeof(rtk_tm_pubtckcash_t));
	m_tResPubTckCashList.clear();
	vector<rtk_tm_pubtckcash_list_t>().swap(m_tResPubTckCashList);

	/// 발권-KTC카드
	KTC_MemClear(&m_tReqPubTckCardKtc, sizeof(stk_tm_pubtckcard_ktc_t));
	KTC_MemClear(&m_tResPubTckCardKtc, sizeof(rtk_tm_pubtckcard_ktc_t));
	m_tResPubTckCardKtcList.clear();
	vector<rtk_tm_pubtckcard_ktc_list_t>().swap(m_tResPubTckCardKtcList);

	/// 발권-KTC_현금영수증
	KTC_MemClear(&m_tReqPubTckCsrcKtc, sizeof(stk_tm_pubtckcsrc_ktc_t));
	KTC_MemClear(&m_tResPubTckCsrcKtc, sizeof(rtk_tm_pubtckcsrc_ktc_t));
	m_tResPubTckCsrcKtcList.clear();
	vector<rtk_tm_pubtckcsrc_ktc_list_t>().swap(m_tResPubTckCsrcKtcList);

	/// 발권-카드
	KTC_MemClear(&m_tReqPubTckCard, sizeof(stk_tm_pubtckcard_t));
	KTC_MemClear(&m_tResPubTckCard, sizeof(rtk_tm_pubtckcard_t));
	m_tResPubTckCardList.clear();
	vector<rtk_tm_pubtckcard_list_t>().swap(m_tResPubTckCardList);

	/// 발권-현금영수증
	KTC_MemClear(&m_tReqPubTckCsrc, sizeof(stk_tm_pubtckcsrc_t));
	KTC_MemClear(&m_tResPubTckCsrc, sizeof(rtk_tm_pubtckcsrc_t));
	m_tResPubTckCsrcList.clear();
	vector<rtk_tm_pubtckcsrc_list_t>().swap(m_tResPubTckCsrcList);

	/// 발권-부가상품권
	KTC_MemClear(&m_tReqPubTckPrd, sizeof(stk_tm_pubtckprd_t));
	KTC_MemClear(&m_tResPubTckPrd, sizeof(rtk_tm_pubtckprd_t));
	m_tResPubTckPrdList.clear();
	vector<rtk_tm_pubtckprd_list_t>().swap(m_tResPubTckPrdList);

	/// 경유지 정보
	KTC_MemClear(&m_tReqThru, sizeof(stk_tm_readthrutrml_t));
	m_vtResThruList.clear();
	vector<rtk_tm_readthrutrml_list_t>().swap(m_vtResThruList);

	/// 인쇄
	m_vtPrtTicket.clear();
	vector<TCK_PRINT_FMT_T>().swap(m_vtPrtTicket);
	m_vtPrtTicketTest.clear();								// 20211116 ADD
	vector<TCK_PRINT_FMT_T>().swap(m_vtPrtTicketTest);		// 20211116 ADD

	n_pbtck_chg_money = n_pbtck_count = n_total_tisu_amt = 0;

	/// 재발행
	KTC_MemClear(&m_tReqRePubTck, sizeof(stk_tm_rpub_tck_t));
	KTC_MemClear(&m_tRespRePubTck, sizeof(rtk_tm_rpub_tck_t));

	TR_LOG_OUT("%s", "##### Memory release !!");
}		  

/**
 * @brief		MakeAlcnListPacket
 * @details		[티머니고속] 배차리스트 패킷 작성
 * @param		char *pSend		전송패킷 버퍼
 * @return		전송 데이타 길이
 */
int CPubTckTmExpMem::MakeAlcnListPacket(char *pSend)
{
	int nOffset = 0, nCount = 0;
	vector<rtk_tm_readalcn_last_list_t>::iterator	iter_last;
	vector<rtk_tm_readalcn_list_t>::iterator		iter;

	nCount = m_vtResAlcnList.size();

	/// ACK
	pSend[nOffset++] = CHAR_ACK;

	/// rsp_cd
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], base.rsp_cd, sizeof(base.rsp_cd));
	/// 이전이후 구분
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], base.bef_aft_dvs, sizeof(base.bef_aft_dvs));
	
	/// 최종배차 노선수
	nCount = m_vtResAlcnLastList.size();
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &nCount, 4);
	for( iter_last = m_vtResAlcnLastList.begin(); iter_last != m_vtResAlcnLastList.end(); iter_last++ )
	{
		nOffset += ::Util_MyCopyMemory(&pSend[nOffset], iter_last._Ptr, sizeof(rtk_tm_readalcn_last_list_t));
	}

	/// 배차건수
	nCount = m_vtResAlcnList.size();
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &nCount, sizeof(int));
	/// 배차리스트
	for( iter = m_vtResAlcnList.begin(); iter != m_vtResAlcnList.end(); iter++ )
	{
		nOffset += ::Util_MyCopyMemory(&pSend[nOffset], iter._Ptr, sizeof(rtk_tm_readalcn_list_t));
	}

	return nOffset;
}

/**
 * @brief		MakeSatsFeeListPacket
 * @details		[티머니고속] 티켓 요금/할인 정보 응답 패킷 작성
 * @param		char *pSend		전송패킷 버퍼
 * @return		전송 데이타 길이
 */
int CPubTckTmExpMem::MakeSatsFeeListPacket(char *pSend)
{
	int nOffset = 0, nFeeCount = 0, nDiscCount = 0;
	vector<rtk_tm_readsatsfee_list_t>::iterator		iter_Fee;
	vector<rtk_tm_readsats_disc_list_t>::iterator	iter_Disc;

	nFeeCount = m_vtResFee.size();
	nDiscCount = m_vtResDisc.size();

	/// ACK
	pSend[nOffset++] = CHAR_ACK;
	
	/// RSP_CD
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], base.rsp_cd, sizeof(base.rsp_cd));

	/// 총 좌석수
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], base.n_tot_sats_num, sizeof(int));

	/// 잔여 좌석수
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], base.n_rmn_sats_num, sizeof(int));

	/// 좌석다중값
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], base.sats_mltp_val, sizeof(base.sats_mltp_val));

	/// 터미널 좌석할당값
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], base.trml_sats_asgt_val, sizeof(base.trml_sats_asgt_val));

	/// 1. 티켓 정보 수
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &nFeeCount, sizeof(int));
	/// 티켓 정보
	for( iter_Fee = m_vtResFee.begin(); iter_Fee != m_vtResFee.end(); iter_Fee++ )
	{
		nOffset += ::Util_MyCopyMemory(&pSend[nOffset], iter_Fee._Ptr, sizeof(rtk_tm_readsatsfee_list_t));
	}

	/// 2. 할인 정보 수
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &nDiscCount, sizeof(int));
	/// 할인 정보
	for( iter_Disc = m_vtResDisc.begin(); iter_Disc != m_vtResDisc.end(); iter_Disc++ )
	{
		nOffset += ::Util_MyCopyMemory(&pSend[nOffset], iter_Disc._Ptr, sizeof(rtk_tm_readsats_disc_list_t));
	}

	return nOffset;
}

/**
 * @brief		MakeSatsPcpyListPacket
 * @details		[티머니고속] 좌석선점 정보 응답 패킷 작성
 * @param		char *pSend		전송패킷 버퍼
 * @return		전송 데이타 길이
 */
int CPubTckTmExpMem::MakeSatsPcpyListPacket(char *pSend)
{
	int nOffset = 0, nCount = 0;
	vector<rtk_tm_pcpysats_list_t>::iterator		iter;

	nCount = m_tResSatsPcpy.size();

	/// ACK
	pSend[nOffset++] = CHAR_ACK;
	
	/// RSP_CD
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], base.rsp_cd, sizeof(base.rsp_cd));

	/// 정보 수
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &nCount, sizeof(int));

	/// 좌석선점 정보
	for( iter = m_tResSatsPcpy.begin(); iter != m_tResSatsPcpy.end(); iter++ )
	{
		nOffset += ::Util_MyCopyMemory(&pSend[nOffset], iter._Ptr, sizeof(rtk_tm_pcpysats_list_t));
	}

	return nOffset;
}


/**
 * @brief		MakePbTckCashPacket
 * @details		[티머니고속] 승차권발권(현금) 정보 응답 패킷 작성 (UI_357)
 * @param		char *pSend		전송패킷 버퍼
 * @return		전송 데이타 길이
 */
int CPubTckTmExpMem::MakePbTckCashPacket(char *pSend)
{
	int nOffset = 0, nCount = 0;
	vector<rtk_tm_pubtckcash_list_t>::iterator		iter;
	prtk_tm_pubtckcash_t	pPacket;

	nCount = m_tResPubTckCashList.size();

	/// ACK
	pSend[nOffset++] = CHAR_ACK;
	
	/// RSP_CD
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], base.rsp_cd, sizeof(base.rsp_cd));

	/// 운수사 출력여부 ~ 사용자키값
	pPacket = &m_tResPubTckCash;
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &m_tResPubTckCash.cacm_nm_prin_yn, pPacket->rec_num - pPacket->cacm_nm_prin_yn);

	/// 정보 수
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &nCount, sizeof(int));

	/// 승차권발권(현금) 정보 
	for( iter = m_tResPubTckCashList.begin(); iter != m_tResPubTckCashList.end(); iter++ )
	{
		nOffset += ::Util_MyCopyMemory(&pSend[nOffset], iter._Ptr, sizeof(rtk_tm_pubtckcash_list_t));
	}

	return nOffset;
}

/**
 * @brief		MakePbTckCardKtcPacket
 * @details		[티머니고속] 승차권발권(카드_KTC) 정보 응답 패킷 작성 (UI_359)
 * @param		char *pSend		전송패킷 버퍼
 * @return		전송 데이타 길이
 */
int CPubTckTmExpMem::MakePbTckCardKtcPacket(char *pSend)
{
	int nOffset = 0, nCount = 0;
	vector<rtk_tm_pubtckcard_ktc_list_t>::iterator		iter;
	prtk_tm_pubtckcard_ktc_t	pPacket;

	nCount = m_tResPubTckCardKtcList.size();

	/// ACK
	pSend[nOffset++] = CHAR_ACK;
	
	/// RSP_CD
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], base.rsp_cd, sizeof(base.rsp_cd));

	/// 5)카드번호 ~ 18)가용포인트(기프트카드)
	pPacket = &m_tResPubTckCardKtc;
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &m_tResPubTckCardKtc.card_no, pPacket->rec_num - pPacket->card_no);

	/// 정보 수
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &nCount, sizeof(int));

	/// 승차권발권(카드_KTC) 정보 
	for( iter = m_tResPubTckCardKtcList.begin(); iter != m_tResPubTckCardKtcList.end(); iter++ )
	{
		nOffset += ::Util_MyCopyMemory(&pSend[nOffset], iter._Ptr, sizeof(rtk_tm_pubtckcard_ktc_list_t));
	}

	return nOffset;
}

/**
 * @brief		MakePbTckCsrcKtcPacket
 * @details		[티머니고속] 승차권발권(현금영수증_ktc) 정보 응답 패킷 작성 (UI_361)
 * @param		char *pSend		전송패킷 버퍼
 * @return		전송 데이타 길이
 */
int CPubTckTmExpMem::MakePbTckCsrcKtcPacket(char *pSend)
{
	int nOffset = 0, nCount = 0;
	vector<rtk_tm_pubtckcsrc_ktc_list_t>::iterator		iter;
	prtk_tm_pubtckcsrc_ktc_t	pPacket;

	nCount = m_tResPubTckCsrcKtcList.size();

	/// 3) ACK
	pSend[nOffset++] = CHAR_ACK;
	
	/// 4) RSP_CD
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], base.rsp_cd, sizeof(base.rsp_cd));

	/// 5)운수사명 출력여부 ~ 14)사용자구분코드
	pPacket = &m_tResPubTckCsrcKtc;
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &m_tResPubTckCsrcKtc.cacm_nm_prin_yn, pPacket->rec_num - pPacket->cacm_nm_prin_yn);

	/// 15) 정보 수
	nCount = *(int *) pPacket->rec_num;
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &nCount, sizeof(int));

	/// 승차권발권(현영_KTC) 정보 
	for( iter = m_tResPubTckCsrcKtcList.begin(); iter != m_tResPubTckCsrcKtcList.end(); iter++ )
	{
		nOffset += ::Util_MyCopyMemory(&pSend[nOffset], iter._Ptr, sizeof(rtk_tm_pubtckcsrc_ktc_list_t));
	}

	return nOffset;
}

/**
 * @brief		MakePbTckPrdPacket
 * @details		[티머니고속] 승차권발권(부가상품권) 정보 응답 패킷 작성 (UI_363)
 * @param		char *pSend		전송패킷 버퍼
 * @return		전송 데이타 길이
 */
int CPubTckTmExpMem::MakePbTckPrdPacket(char *pSend)
{
	int nOffset = 0, nCount = 0;
	vector<rtk_tm_pubtckprd_list_t>::iterator		iter;
	prtk_tm_pubtckprd_t	pPacket;

	nCount = m_tResPubTckPrdList.size();

	/// 3) ACK
	pSend[nOffset++] = CHAR_ACK;
	
	/// 4) RSP_CD
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], base.rsp_cd, sizeof(base.rsp_cd));

	/// 5) 부가상품권번호 ~ 14) 발권번호
	pPacket = &m_tResPubTckPrd;
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &m_tResPubTckPrd.adtn_cpn_no, pPacket->rec_num - pPacket->adtn_cpn_no);

	/// 15) 정보 수
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &nCount, sizeof(int));

	/// 승차권발권(부가상품권) 정보 
	for( iter = m_tResPubTckPrdList.begin(); iter != m_tResPubTckPrdList.end(); iter++ )
	{
		nOffset += ::Util_MyCopyMemory(&pSend[nOffset], iter._Ptr, sizeof(rtk_tm_pubtckprd_list_t));
	}

	return nOffset;
}

/**
 * @brief		MakePbTckThruInfo
 * @details		[티머니고속] 승차권발권(경유지 정보) 응답 패킷 작성 (UI_365)
 * @param		char *pSend		전송패킷 버퍼
 * @return		전송 데이타 길이
 */
int CPubTckTmExpMem::MakePbTckThruInfo(char *pSend)
{
	int nOffset = 0, nCount = 0;
	vector<rtk_tm_readthrutrml_list_t>::iterator		iter;
	//prtk_tm_pubtckprd_t	pPacket;

	nCount = m_vtResThruList.size();

	/// 3) ACK
	pSend[nOffset++] = CHAR_ACK;
	
	/// 4) RSP_CD
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], base.rsp_cd, sizeof(base.rsp_cd));

	/// 5) 정보 수
	nOffset += ::Util_MyCopyMemory(&pSend[nOffset], &nCount, sizeof(int));

	/// 6) 승차권발권(경유지 정보) 조회
	for( iter = m_vtResThruList.begin(); iter != m_vtResThruList.end(); iter++ )
	{
		nOffset += ::Util_MyCopyMemory(&pSend[nOffset], iter._Ptr, sizeof(rtk_tm_readthrutrml_list_t));
	}

	return nOffset;
}

/**
 * @brief		MakeCashTicketPrtData
 * @details		[티머니고속] (현금) 승차권 데이타 작성
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CPubTckTmExpMem::MakeCashTicketPrtData(void)
{
	int nRet = 0, nLen = 0;
	int nSatsNo = 0;
	int nRot = -1;
	char *pBuff;
	POPER_FILE_CONFIG_T pConfig;

	vector<rtk_tm_pubtckcash_list_t>::iterator iter;

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	TR_LOG_OUT(" [티머니고속_현금] 승차권 프린트 데이타 작성 !!!");
	TR_LOG_OUT(" start !!!!");

	m_vtPrtTicket.clear();
	for(iter = m_tResPubTckCashList.begin(); iter != m_tResPubTckCashList.end(); iter++)
	{
		TCK_PRINT_FMT_T tPrtInfo;
		stk_tm_pubtckcash_t *pReq;
		rtk_tm_pubtckcash_t *pResp;

		::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
		pReq = &m_tReqPubTckCash;
		pResp = &m_tResPubTckCash;

		/// (01). 좌석번호출력여부		
		tPrtInfo.sats_no_prin_yn[0] = pResp->sats_no_prin_yn[0];

		/// (02). 출발시간 출력여부		
		tPrtInfo.depr_time_prin_yn[0] = pResp->depr_time_prin_yn[0];

		/// (03). 거리
		if(strlen(base.bus_oprn_dist) > 0)
		{
			sprintf(tPrtInfo.thru_nm, "%.*skm", sizeof(tPrtInfo.thru_nm) - 1, base.bus_oprn_dist);
		}
		/// (04). 출발지 터미널명 (한글)
		Find_TmExpTrmlName(LANG_KOR, pReq->depr_trml_no, tPrtInfo.depr_trml_ko_nm);
		/// (05). 출발지 터미널명 (영문)
		Find_TmExpTrmlName(LANG_ENG, pReq->depr_trml_no, tPrtInfo.depr_trml_eng_nm);
		/// (06). 출발지 터미널 전화번호
		{
			nLen = CheckBuffer(pBuff = pConfig->ezTrmlInfo_t.sz_prn_trml_tel_no, sizeof(tPrtInfo.depr_trml_tel_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_tel_nm, pBuff, nLen);
			}
		}
		/// (07). 도착지 터미널명 (한글)
		Find_TmExpTrmlName(LANG_KOR, pReq->arvl_trml_no, tPrtInfo.arvl_trml_ko_nm);
		/// (08). 도착지 터미널명 (영문)
		Find_TmExpTrmlName(LANG_ENG, pReq->arvl_trml_no, tPrtInfo.arvl_trml_eng_nm);
		/// (09). 버스운수사 명
		nRet = FindBusCacmName(SVR_DVS_TMEXP, pReq->cacm_cd, tPrtInfo.bus_cacm_nm, tPrtInfo.bus_cacm_biz_no, tPrtInfo.bus_cacm_tel_nm);	
		/// (10). 버스등급명
		nRet = FindBusClsName(SVR_DVS_TMEXP, pReq->bus_cls_cd, tPrtInfo.bus_cls_nm);		
		/// (11). 버스티켓종류명
		::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->tck_knd_cd, sizeof(iter->tck_knd_cd));
		nRet = FindBusTckKndName(SVR_DVS_TMEXP, iter->tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 
		/// (12). 현금영수증 승인번호
		sprintf(tPrtInfo.card_aprv_no, "%s", iter->csrc_aprv_no);
		/// (13). 현금 승인금액, 요금
		{
			int nValue = 0;
			char szFare[100];

			/// 승인금액
			nValue = base.nTotalMoney;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.card_aprv_amt, "%s", szFare);	

			/// 발권금액
			nValue = *(int *) iter->n_tissu_fee;
			tPrtInfo.n_tisu_amt = nValue; 
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.tisu_amt, "%s", szFare);	
		}
		/// (14). 카드번호
		//sprintf(tPrtInfo.card_no, "%s", card_resp.card_no);
		/// (15). 결제수단 (현금/신용카드 등)
		tPrtInfo.n_pym_dvs = PYM_CD_CASH;
		sprintf(tPrtInfo.pym_dvs, "현금(자진발급)");	
		sprintf(tPrtInfo.shrt_pym_dvs, "현금");

		/// (16). 좌석번호
		sprintf(tPrtInfo.sats_no, "%s", iter->sats_no);	

		/// (17). 출발 일자, 출발시간
		MakeDepartureDateTime(pReq->depr_dt, 
							  pReq->depr_time, 
							  tPrtInfo.atl_depr_dt, 
							  tPrtInfo.atl_depr_time);

		/// (18). 승차홈
		sprintf(tPrtInfo.rdhm_val, "%s", base.rot_rdhm_no_val);	
		/// (19). 발권시간
		sprintf(tPrtInfo.pub_time, "(%.*s:%.*s:%.*s)", 
				2, &pResp->tissu_time[0], 
				2, &pResp->tissu_time[2], 
				2, &pResp->tissu_time[4]);
		/// (20). QR 바코드 관련
		{
			/// 1) 발행일자
			//sprintf(tPrtInfo.qr_pub_dt, "%s", m_tReqTckIssue.depr_dt);	
			sprintf(tPrtInfo.qr_pub_dt, "%s", pResp->tissu_dt);	
			/// 2) 발행터미널코드
			//sprintf(tPrtInfo.qr_pub_trml_cd, "%s", iter->tissu_trml_no);	
			sprintf(tPrtInfo.qr_pub_trml_cd, "%s", pResp->tissu_trml_no);	
			/// 3) 창구번호
			//sprintf(tPrtInfo.qr_pub_wnd_no, "%s", iter->tissu_wnd_no);	
			sprintf(tPrtInfo.qr_pub_wnd_no, "%s", pResp->tissu_wnd_no);	
			/// 4) 발행일련번호
			sprintf(tPrtInfo.qr_pub_sno, "%s", iter->tissu_sno);
			/// 5) 출발지 터미널번호
			sprintf(tPrtInfo.qr_depr_trml_no, "%s", pReq->depr_trml_no);
			/// 6) 도착지 터미널번호
			sprintf(tPrtInfo.qr_arvl_trml_no, "%s", pReq->arvl_trml_no);
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

		/// (21). 항목 출력여부
		{
			///< 버스등급 출력여부		
			tPrtInfo.bus_cls_prin_yn[0]		= pResp->bus_cls_prin_yn[0];
			///< 버스운수사명 출력여부	
			tPrtInfo.bus_cacm_nm_prin_yn[0] = pResp->cacm_nm_prin_yn[0];
			///< 출발시각 출력여부
			//tPrtInfo.depr_time_prin_yn[0]	= cash_resp.depr_time_prin_yn[0];
			///< 좌석번호 출력여부
			//tPrtInfo.sats_no_prin_yn[0]		= cash_resp.sats_no_prin_yn[0];
		}

		/// (22). 사용자 이름
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->user_nm, sizeof(tPrtInfo.user_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.user_nm, pBuff, nLen);
			}
		}

		m_vtPrtTicket.push_back(tPrtInfo);

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
#endif
	}

	return 0;
}

/**
 * @brief		MakeCsrcTicketPrtData
 * @details		[티머니고속] (현금영수증-수기) 승차권 데이타 작성
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CPubTckTmExpMem::MakeCsrcTicketPrtData(void)
{
	int nRet = 0, nLen = 0;
	char *pBuff;
	POPER_FILE_CONFIG_T pConfig;

	vector<rtk_tm_pubtckcsrc_ktc_list_t>::iterator iter;

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	TR_LOG_OUT(" [티머니고속_현금영수증_수기] 승차권 프린트 데이타 작성 !!!");
	TR_LOG_OUT(" start !!!!");

	m_vtPrtTicket.clear();
	for(iter = m_tResPubTckCsrcKtcList.begin(); iter != m_tResPubTckCsrcKtcList.end(); iter++)
	{
		TCK_PRINT_FMT_T tPrtInfo;
		stk_tm_pubtckcsrc_ktc_t *pReq;
		rtk_tm_pubtckcsrc_ktc_t *pResp;

		::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
		pReq = &m_tReqPubTckCsrcKtc;
		pResp = &m_tResPubTckCsrcKtc;

		/// (01). 좌석번호출력여부		
		tPrtInfo.sats_no_prin_yn[0] = pResp->sats_no_prin_yn[0];
		/// (02). 출발시간 출력여부		
		tPrtInfo.depr_time_prin_yn[0] = pResp->depr_time_prin_yn[0];
		/// (03). 거리
		if(strlen(base.bus_oprn_dist) > 0)
		{
			sprintf(tPrtInfo.thru_nm, "%.*skm", sizeof(tPrtInfo.thru_nm) - 1, base.bus_oprn_dist);
		}
		/// (04). 출발지 터미널명 (한글)
		Find_TmExpTrmlName(LANG_KOR, pReq->depr_trml_no, tPrtInfo.depr_trml_ko_nm);
		/// (05). 출발지 터미널명 (영문)
		Find_TmExpTrmlName(LANG_ENG, pReq->depr_trml_no, tPrtInfo.depr_trml_eng_nm);
		/// (06). 출발지 터미널 전화번호
		{
			nLen = CheckBuffer(pBuff = pConfig->ezTrmlInfo_t.sz_prn_trml_tel_no, sizeof(tPrtInfo.depr_trml_tel_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_tel_nm, pBuff, nLen);
			}
		}
		/// (07). 도착지 터미널명 (한글)
		Find_TmExpTrmlName(LANG_KOR, pReq->arvl_trml_no, tPrtInfo.arvl_trml_ko_nm);
		/// (08). 도착지 터미널명 (영문)
		Find_TmExpTrmlName(LANG_ENG, pReq->arvl_trml_no, tPrtInfo.arvl_trml_eng_nm);
		/// (09). 버스운수사 명
		nRet = FindBusCacmName(SVR_DVS_TMEXP, pReq->cacm_cd, tPrtInfo.bus_cacm_nm, tPrtInfo.bus_cacm_biz_no, tPrtInfo.bus_cacm_tel_nm);	
		/// (10). 버스등급명
		nRet = FindBusClsName(SVR_DVS_TMEXP, pReq->bus_cls_cd, tPrtInfo.bus_cls_nm);		
		/// (11). 버스티켓종류명
		::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->tck_knd_cd, sizeof(iter->tck_knd_cd));
		nRet = FindBusTckKndName(SVR_DVS_TMEXP, iter->tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 
		/// (12). 현금영수증 승인번호
		sprintf(tPrtInfo.card_aprv_no, "%s", iter->csrc_aprv_no);
		/// (13). 승인금액, 요금
		{
			int nValue = 0;
			char szFare[100];

			/// 승인금액
			//nValue = *(int *) iter->tisu_amt;
			nValue = base.nTotalMoney;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.card_aprv_amt, "%s", szFare);	

			/// 요금(발권금액)
			nValue = *(int *) iter->n_tissu_fee;
			tPrtInfo.n_tisu_amt = nValue; 
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.tisu_amt, "%s", szFare);	
		}
		/// (14). 사용자정보
		CMN_MakePasswdChars(tPrtInfo.card_no, m_tResPubTckCsrcKtc.user_key_val);
		//sprintf(tPrtInfo.card_no, "%s", m_tResPubTckCsrcKtc.user_key_val);

		/// (15). 결제수단 / 현금영수증 구분코드 - 0:미사용, 1:수기입력, 2:신용카드, 3:현영전용카드
		tPrtInfo.n_pym_dvs = PYM_CD_CSRC;
		if( base.ui_csrc_dvs_cd[0] == 1 ) 
		{	/// 수기입력
			if( base.ui_csrc_use[0] == 0 )
			{	/// 개인
				sprintf(tPrtInfo.pym_dvs, "현금(소득공제)");	
			}
			else
			{	/// 법인
				sprintf(tPrtInfo.pym_dvs, "현금(지출증빙)");	
			}
		}
		else
		{	/// 현금영수증 카드
			//sprintf(tPrtInfo.pym_dvs, "현금(현금영수증)");	
			if( base.ui_csrc_use[0] == 0 )
			{	/// 개인
				sprintf(tPrtInfo.pym_dvs, "현금(소득공제)");	
			}
			else
			{	/// 법인
				sprintf(tPrtInfo.pym_dvs, "현금(지출증빙)");	
			}
		}
		sprintf(tPrtInfo.shrt_pym_dvs, "카드");
		/// (16). 좌석번호
		sprintf(tPrtInfo.sats_no, "%s", iter->sats_no);	
		/// (17). 출발 일자, 출발시간
#if 1	// 20220622 ADD~	// 출발시각이 아닌 배차출발시각 표시 오류 발생으로 수정
		MakeDepartureDateTime(pReq->depr_dt, pReq->depr_time, tPrtInfo.atl_depr_dt, tPrtInfo.atl_depr_time);
#else
		MakeDepartureDateTime(pReq->alcn_depr_dt, 
							  pReq->alcn_depr_time, 
							  tPrtInfo.atl_depr_dt, 
							  tPrtInfo.atl_depr_time);
#endif	// 20220622 ~ADD
		/// (18). 승차홈
		sprintf(tPrtInfo.rdhm_val, "%s", base.rot_rdhm_no_val);	
		/// (19). 발권시간
		sprintf(tPrtInfo.pub_time, "(%.*s:%.*s:%.*s)", 
				2, &pResp->tissu_time[0], 
				2, &pResp->tissu_time[2], 
				2, &pResp->tissu_time[4]);
		/// (20). QR 바코드 관련
		{
			/// 1) 발행일자
			//sprintf(tPrtInfo.qr_pub_dt, "%s", m_tReqTckIssue.depr_dt);	
			sprintf(tPrtInfo.qr_pub_dt, "%s", pResp->tissu_dt);	
			/// 2) 발행터미널코드
			//sprintf(tPrtInfo.qr_pub_trml_cd, "%s", iter->tissu_trml_no);	
			sprintf(tPrtInfo.qr_pub_trml_cd, "%s", pResp->tissu_trml_no);	
			///// 3) 창구번호
			//sprintf(tPrtInfo.qr_pub_wnd_no, "%s", iter->tissu_wnd_no);	
			sprintf(tPrtInfo.qr_pub_wnd_no, "%s", pResp->tissu_wnd_no);	

			/// 4) 발행일련번호
			sprintf(tPrtInfo.qr_pub_sno, "%s", iter->tissu_sno);
			/// 5) 출발지 터미널번호
			sprintf(tPrtInfo.qr_depr_trml_no, "%s", pReq->depr_trml_no);
			/// 6) 도착지 터미널번호
			sprintf(tPrtInfo.qr_arvl_trml_no, "%s", pReq->arvl_trml_no);
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
		/// (21). 항목 출력여부
		{
			///< 버스등급 출력여부		
			tPrtInfo.bus_cls_prin_yn[0]		= pResp->bus_cls_prin_yn[0];
			///< 버스운수사명 출력여부	
			tPrtInfo.bus_cacm_nm_prin_yn[0] = pResp->cacm_nm_prin_yn[0];
			///< 출발시각 출력여부
			//tPrtInfo.depr_time_prin_yn[0]	= card_resp.depr_time_prin_yn[0];
			///< 좌석번호 출력여부
			//tPrtInfo.sats_no_prin_yn[0]		= card_resp.sats_no_prin_yn[0];
		}
		/// (22). 사용자 이름
		{
			int nLen;
			char *pBuff;

			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->user_nm, sizeof(tPrtInfo.user_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.user_nm, pBuff, nLen);
			}
		}

		TR_LOG_OUT(" [티머니고속_현금영수증] 승차권 프린트 데이타 add_q !!!");

		m_vtPrtTicket.push_back(tPrtInfo);

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
#endif
	}

	return 0;
}

/**
 * @brief		MakeCsrcCardTicketPrtData
 * @details		[티머니고속] (현금영수증-카드) 승차권 데이타 작성
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CPubTckTmExpMem::MakeCsrcCardTicketPrtData(void)
{
	int nRet = 0, nLen = 0;
	char *pBuff;
	POPER_FILE_CONFIG_T pConfig;
	vector<rtk_tm_pubtckcsrc_ktc_list_t>::iterator iter;

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	TR_LOG_OUT(" [티머니고속_현금영수증_카드] 승차권 프린트 데이타 작성 !!!");
	TR_LOG_OUT(" start !!!!");

	m_vtPrtTicket.clear();
	for(iter = m_tResPubTckCsrcKtcList.begin(); iter != m_tResPubTckCsrcKtcList.end(); iter++)
	{
		TCK_PRINT_FMT_T tPrtInfo;
		stk_tm_pubtckcsrc_ktc_t *pReq;
		rtk_tm_pubtckcsrc_ktc_t *pResp;

		::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));

		pReq = &m_tReqPubTckCsrcKtc;
		pResp = &m_tResPubTckCsrcKtc;

		/// (01). 좌석번호출력여부		
		tPrtInfo.sats_no_prin_yn[0] = pResp->sats_no_prin_yn[0];
		/// (02). 출발시간 출력여부		
		tPrtInfo.depr_time_prin_yn[0] = pResp->depr_time_prin_yn[0];
		/// (03). 거리
		if(strlen(base.bus_oprn_dist) > 0)
		{
			sprintf(tPrtInfo.thru_nm, "%.*skm", sizeof(tPrtInfo.thru_nm) - 1, base.bus_oprn_dist);
		}
		/// (04). 출발지 터미널명 (한글)
		Find_TmExpTrmlName(LANG_KOR, pReq->depr_trml_no, tPrtInfo.depr_trml_ko_nm);
		/// (05). 출발지 터미널명 (영문)
		Find_TmExpTrmlName(LANG_ENG, pReq->depr_trml_no, tPrtInfo.depr_trml_eng_nm);
		/// (06). 출발지 터미널 전화번호
		{
			nLen = CheckBuffer(pBuff = pConfig->ezTrmlInfo_t.sz_prn_trml_tel_no, sizeof(tPrtInfo.depr_trml_tel_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_tel_nm, pBuff, nLen);
			}
		}
		/// (07). 도착지 터미널명 (한글)
		Find_TmExpTrmlName(LANG_KOR, pReq->arvl_trml_no, tPrtInfo.arvl_trml_ko_nm);
		/// (08). 도착지 터미널명 (영문)
		Find_TmExpTrmlName(LANG_ENG, pReq->arvl_trml_no, tPrtInfo.arvl_trml_eng_nm);
		/// (09). 버스운수사 명
		nRet = FindBusCacmName(SVR_DVS_TMEXP, pReq->cacm_cd, tPrtInfo.bus_cacm_nm, tPrtInfo.bus_cacm_biz_no, tPrtInfo.bus_cacm_tel_nm);	
		/// (10). 버스등급명
		nRet = FindBusClsName(SVR_DVS_TMEXP, pReq->bus_cls_cd, tPrtInfo.bus_cls_nm);		
		/// (11). 버스티켓종류명
		::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->tck_knd_cd, sizeof(iter->tck_knd_cd));
		nRet = FindBusTckKndName(SVR_DVS_TMEXP, iter->tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 
		/// (12). 현금영수증_카드 승인번호
		sprintf(tPrtInfo.card_aprv_no, "%s", iter->csrc_aprv_no);
		/// (13). 승인금액, 요금
		{
			int nValue = 0;
			char szFare[100];

			/// 승인금액
			//nValue = *(int *) iter->tisu_amt;
			nValue = base.nTotalMoney;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.card_aprv_amt, "%s", szFare);	

			/// 요금(발권금액)
			nValue = *(int *) iter->n_tissu_fee;
			tPrtInfo.n_tisu_amt = nValue; 
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.tisu_amt, "%s", szFare);	
		}
		/// (14). 카드번호
		//sprintf(tPrtInfo.card_no, "%s", csrc_resp.card_no);
		/// (15). 결제수단 
		tPrtInfo.n_pym_dvs = PYM_CD_CSRC;
		if(tPrtInfo.ui_csrc_use[0] == 0x00 )
		{	
			sprintf(tPrtInfo.pym_dvs, "현금(소득공제)");	
		}
		else
		{
			sprintf(tPrtInfo.pym_dvs, "현금(지출증빙)");	
		}
		sprintf(tPrtInfo.shrt_pym_dvs, "현영");
		/// (16). 좌석번호
		sprintf(tPrtInfo.sats_no, "%d", iter->sats_no);	
		/// (17). 출발 일자, 출발시간
#if 1	// 20220622 ADD~	// 출발시각이 아닌 배차출발시각 표시 오류 발생으로 수정
		MakeDepartureDateTime(pReq->depr_dt, pReq->depr_time, tPrtInfo.atl_depr_dt, tPrtInfo.atl_depr_time);
#else
		MakeDepartureDateTime(pReq->alcn_depr_dt, 
							  pReq->alcn_depr_time, 
							  tPrtInfo.atl_depr_dt, 
							  tPrtInfo.atl_depr_time);
#endif	// 20220622 ~ADD
		/// (18). 승차홈
		sprintf(tPrtInfo.rdhm_val, "%s", base.rot_rdhm_no_val);	
		// (19). 발권시간
		sprintf(tPrtInfo.pub_time, "(%.*s:%.*s:%.*s)", 
				2, &pResp->tissu_time[0], 
				2, &pResp->tissu_time[2], 
				2, &pResp->tissu_time[4]);
		/// (20). QR 바코드 관련
		{
			/// 1) 발행일자
			//sprintf(tPrtInfo.qr_pub_dt, "%s", m_tReqTckIssue.depr_dt);	
			sprintf(tPrtInfo.qr_pub_dt, "%s", pResp->tissu_dt);	
			/// 2) 발행터미널코드
			//sprintf(tPrtInfo.qr_pub_trml_cd, "%s", iter->tissu_trml_no);	
			sprintf(tPrtInfo.qr_pub_trml_cd, "%s", pResp->tissu_trml_no);	
			///// 3) 창구번호
			//sprintf(tPrtInfo.qr_pub_wnd_no, "%s", iter->tissu_wnd_no);	
			sprintf(tPrtInfo.qr_pub_wnd_no, "%s", pResp->tissu_wnd_no);	

			/// 4) 발행일련번호
			sprintf(tPrtInfo.qr_pub_sno, "%s", iter->tissu_sno);
			/// 5) 출발지 터미널번호
			sprintf(tPrtInfo.qr_depr_trml_no, "%s", pReq->depr_trml_no);
			/// 6) 도착지 터미널번호
			sprintf(tPrtInfo.qr_arvl_trml_no, "%s", pReq->arvl_trml_no);
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

		/// (21). 항목 출력여부
		{
			///< 버스등급 출력여부		
			tPrtInfo.bus_cls_prin_yn[0]		= pResp->bus_cls_prin_yn[0];
			///< 버스운수사명 출력여부	
			tPrtInfo.bus_cacm_nm_prin_yn[0] = pResp->cacm_nm_prin_yn[0];
			///< 출발시각 출력여부
			//tPrtInfo.depr_time_prin_yn[0]	= pResp->depr_time_prin_yn[0];
			///< 좌석번호 출력여부
			//tPrtInfo.sats_no_prin_yn[0]		= pResp->sats_no_prin_yn[0];
		}
		/// (22). 사용자 이름
		{
			int nLen;
			char *pBuff;

			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->user_nm, sizeof(tPrtInfo.user_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.user_nm, pBuff, nLen);
			}
		}

		m_vtPrtTicket.push_back(tPrtInfo);

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
#endif
	}

	return 0;
}

/**
 * @brief		MakeCreditTicketPrtData
 * @details		[티머니고속] (신용카드_KTC) 승차권 데이타 작성
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CPubTckTmExpMem::MakeCreditTicketPrtData(void)
{
 	int nRet = 0;
	int nLen;
	char *pBuff;
	POPER_FILE_CONFIG_T pConfig;
 	vector<rtk_tm_pubtckcard_ktc_list_t>::iterator iter;

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	TR_LOG_OUT(" [티머니고속_카드] 승차권 프린트 데이타 작성 !!!");
	TR_LOG_OUT(" start !!!!");

 	m_vtPrtTicket.clear();
 	for(iter = m_tResPubTckCardKtcList.begin(); iter != m_tResPubTckCardKtcList.end(); iter++)
 	{
 		TCK_PRINT_FMT_T tPrtInfo;
		//stk_tm_pubtckcard_ktc_t *pInfo;
		stk_tm_pubtckcard_ktc_t *pReq;
		rtk_tm_pubtckcard_ktc_t *pResp;
 
 		::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
		pReq = &m_tReqPubTckCardKtc;
		pResp = &m_tResPubTckCardKtc;
 
		/// (01). 좌석번호출력여부		
		tPrtInfo.sats_no_prin_yn[0] = pResp->sats_no_prin_yn[0];
		/// (02). 출발시간 출력여부		
		tPrtInfo.depr_time_prin_yn[0] = pResp->depr_time_prin_yn[0];
		/// (03). 거리
		if(strlen(base.bus_oprn_dist) > 0)
		{
			sprintf(tPrtInfo.thru_nm, "%.*skm", sizeof(tPrtInfo.thru_nm) - 1, base.bus_oprn_dist);
		}
 		/// (04). 출발지 터미널명 (한글)
		Find_TmExpTrmlName(LANG_KOR, pReq->depr_trml_no, tPrtInfo.depr_trml_ko_nm);
 		/// (05). 출발지 터미널명 (영문)
		Find_TmExpTrmlName(LANG_ENG, pReq->depr_trml_no, tPrtInfo.depr_trml_eng_nm);
		/// (06). 출발지 터미널 전화번호
		{
			nLen = CheckBuffer(pBuff = pConfig->ezTrmlInfo_t.sz_prn_trml_tel_no, sizeof(tPrtInfo.depr_trml_tel_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_tel_nm, pBuff, nLen);
			}
		}
 		/// (07). 도착지 터미널명 (한글)
 		Find_TmExpTrmlName(LANG_KOR, pReq->arvl_trml_no, tPrtInfo.arvl_trml_ko_nm);
 		/// (08). 도착지 터미널명 (영문)
		Find_TmExpTrmlName(LANG_ENG, pReq->arvl_trml_no, tPrtInfo.arvl_trml_eng_nm);
 		/// (09). 버스운수사 명
 		nRet = FindBusCacmName(SVR_DVS_TMEXP, pReq->cacm_cd, tPrtInfo.bus_cacm_nm, tPrtInfo.bus_cacm_biz_no, tPrtInfo.bus_cacm_tel_nm);	
 		/// (10). 버스등급명
 		nRet = FindBusClsName(SVR_DVS_TMEXP, pReq->bus_cls_cd, tPrtInfo.bus_cls_nm);		
  		/// (11). 버스티켓종류명
		::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->tck_knd_cd, sizeof(tPrtInfo.bus_tck_knd_cd));
  		nRet = FindBusTckKndName(SVR_DVS_TMEXP, iter->tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 
 		/// (12). 신용카드 승인번호
 		sprintf(tPrtInfo.card_aprv_no, "%s", pResp->card_aprv_no);
 		/// (13). 신용카드 승인금액, 요금
 		{
 			int nValue = 0;
 			char szFare[100];
 
			/// 승인금액
 			nValue = *(int *) pResp->n_aprv_amt;
 			::ZeroMemory(szFare, sizeof(szFare));
 			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.card_aprv_amt, "%s", szFare);	

			/// 요금
			nValue = *(int *) iter->n_tissu_fee;
			::ZeroMemory(szFare, sizeof(szFare));
			tPrtInfo.n_tisu_amt = nValue;
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.tisu_amt, "%s", szFare);	

			/// 기프트카드 잔액
			nValue = 0;
			nValue = *(int *) pResp->able_point;
			TR_LOG_OUT("[tm_exp] 기프트카드 잔액 = %d ....", nValue);
			if(nValue > 0)
			{
				::ZeroMemory(szFare, sizeof(szFare));
				Util_AmountComma(nValue, szFare);
				sprintf(tPrtInfo.card_tr_add_inf, "%s", szFare);	
				tPrtInfo.trd_dvs_cd[0] = '3';
			}
		}

 		/// (14). 카드번호
 		{
 			int len, i, k;
 			char Buffer[100];

 			::ZeroMemory(Buffer, sizeof(Buffer));
 
 			len = strlen(pResp->card_no);
 			for(k = 0, i = 0; i < len; i++)
 			{
 				if(pResp->card_no[i] != '-')
 				{
 					Buffer[k++] = pResp->card_no[i] & 0xFF;
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
 #if (_KTC_CERTIFY_ > 0)
 			KTC_MemClear(Buffer, sizeof(Buffer));
 #endif
 		}
 
 		/// (15). 결제수단 / 할부 개월 수 
 		tPrtInfo.n_pym_dvs = PYM_CD_CARD;
		tPrtInfo.n_mip_mm_num = *(int *)pReq->n_mip_mm_num;
		if(tPrtInfo.n_mip_mm_num == 0)
		{
 			sprintf(tPrtInfo.pym_dvs, "현장카드(일시불)");	
		}
		else
		{
			sprintf(tPrtInfo.pym_dvs, "현장카드(%d 개월)", tPrtInfo.n_mip_mm_num);	
		}
		sprintf(tPrtInfo.shrt_pym_dvs, "카드");
		/// (16). 좌석번호
		sprintf(tPrtInfo.sats_no, "%s", iter->sats_no);	

		/****
		/// (17). 실제 출발 일자
		{
			stDepr.wYear  = (WORD) Util_Ascii2Long((char *)&pReq->depr_dt[0], 4);
			stDepr.wMonth = (WORD) Util_Ascii2Long((char *)&pReq->depr_dt[4], 2);
			stDepr.wDay   = (WORD) Util_Ascii2Long((char *)&pReq->depr_dt[6], 2);
		}

		/// (13). 실제 출발 일자
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
		sprintf(tPrtInfo.atl_depr_time, "%.*s:%.*s", 2, &pReq->depr_time[0], 2, &pReq->depr_time[2]);
		***/
		/// (17). 출발 일자, 출발시간
		MakeDepartureDateTime(pReq->depr_dt, 
							  pReq->depr_time, 
							  tPrtInfo.atl_depr_dt, 
							  tPrtInfo.atl_depr_time);
 		/// (18). 승차홈
 		sprintf(tPrtInfo.rdhm_val, "%s", this->base.rot_rdhm_no_val);	
		// (19). 발권시간
		sprintf(tPrtInfo.pub_time, "(%.*s:%.*s:%.*s)", 
				2, &pResp->tissu_time[0], 
				2, &pResp->tissu_time[2], 
				2, &pResp->tissu_time[4]);
		/// (20). QR 바코드 관련
 		{
 			/// 1) 발행일자
			//sprintf(tPrtInfo.qr_pub_dt, "%s", m_tReqTckIssue.depr_dt);	
 			sprintf(tPrtInfo.qr_pub_dt, "%s", pResp->tissu_dt);	
 			/// 2) 발행터미널코드
 			//sprintf(tPrtInfo.qr_pub_trml_cd, "%s", iter->tissu_trml_no);	
			sprintf(tPrtInfo.qr_pub_trml_cd, "%s", pResp->tissu_trml_no);	
 			/// 3) 창구번호
 			//sprintf(tPrtInfo.qr_pub_wnd_no, "%s", iter->tissu_wnd_no);	
			sprintf(tPrtInfo.qr_pub_wnd_no, "%s", pResp->tissu_wnd_no);	
 			/// 4) 발행일련번호
 			sprintf(tPrtInfo.qr_pub_sno, "%s", iter->tissu_sno);
			/// 5) 출발지 터미널번호
			sprintf(tPrtInfo.qr_depr_trml_no, "%s", pReq->depr_trml_no);
			/// 6) 도착지 터미널번호
			sprintf(tPrtInfo.qr_arvl_trml_no, "%s", pReq->arvl_trml_no);
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
		/// (21). 항목 출력여부
		{
			///< 버스등급 출력여부		
			tPrtInfo.bus_cls_prin_yn[0]		= pResp->bus_cls_prin_yn[0];
			///< 버스운수사명 출력여부	
			tPrtInfo.bus_cacm_nm_prin_yn[0] = pResp->cacm_nm_prin_yn[0];
			///< 출발시각 출력여부
			//tPrtInfo.depr_time_prin_yn[0]	= cash_resp.depr_time_prin_yn[0];
			///< 좌석번호 출력여부
			//tPrtInfo.sats_no_prin_yn[0]		= cash_resp.sats_no_prin_yn[0];
		}
		/// (22). 사용자 이름
		{
			int nLen;
			char *pBuff;

			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->user_nm, sizeof(tPrtInfo.user_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.user_nm, pBuff, nLen);
			}
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
 * @brief		FindPubTckListData
 * @details		[티머니고속] - 발행내역 데이타 찾기
 * @param		char *tissu_dt		발행일자
 * @param		char *tissu_sno		발행일련번호
 * @return		성공 >= 0, 실패 < 0
 */
int CPubTckTmExpMem::FindPubTckListData(char *tissu_dt, char *tissu_sno, char *retBuf)
{
	int i, nFind, nCount;
	vector<rtk_tm_pub_inq_list_t>::iterator iter;

	i = nCount = 0;
	nFind = -1;

	nCount = m_vtPbTckInqList.size();
	if(nCount <= 0)
	{
		return -1;
	}

	for(iter = m_vtPbTckInqList.begin(); iter != m_vtPbTckInqList.end(); iter++)
	{
		if( memcmp(iter->tissu_dt, tissu_dt, strlen(iter->tissu_dt)) == 0 )
		{
			if( memcmp(iter->tissu_sno, tissu_sno, strlen(iter->tissu_sno)) == 0 )
			{
				::CopyMemory(retBuf, iter._Ptr, sizeof(rtk_tm_pub_inq_list_t));
				nFind = i;
			}
		}

	}
	return nFind;
}

/**
 * @brief		MakeReTicketPrtData
 * @details		[티머니고속] - 재발행 티켓 데이타 작성
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int CPubTckTmExpMem::MakeReTicketPrtData(void)
{
	int						nRet = 0;
	int						nWeek;
	int						nLen, i, k;
	char					Buffer[100];
	SYSTEMTIME				stDepr;
	rtk_tm_pub_inq_list_t	tList;
	POPER_FILE_CONFIG_T		pConfig;
	char					*pBuff;

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	nWeek = 0;
	::ZeroMemory(&stDepr, sizeof(SYSTEMTIME));

	::ZeroMemory(&tList, sizeof(rtk_tm_pub_inq_list_t));
	nRet = FindPubTckListData(m_tReqRePubTck.tissu_dt, m_tReqRePubTck.tissu_sno, (char *)&tList);
	if(nRet < 0)
	{
		m_vtPrtTicket.clear();
		return -1;
	}

	TR_LOG_OUT(" start !!!!");

	TR_LOG_OUT(" tList => depr[%s] arvk[%s] cacm[%s] blscd[%s] tck[%s]", 
			tList.depr_trml_no,
			tList.arvl_trml_no,
			tList.cacm_cd,
			tList.bus_cls_cd,
			tList.tck_knd_cd);

	m_vtPrtTicket.clear();
	//for(iter = m_vtPbTckReIssue.begin(); iter != m_vtPbTckReIssue.end(); iter++)
	{
 		TCK_PRINT_FMT_T tPrtInfo;
 
 		::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
 
		/// (01). 좌석번호출력여부		
		tPrtInfo.sats_no_prin_yn[0] = m_tRespRePubTck.sats_no_prin_yn[0];
		/// (02). 출발시간 출력여부		
		tPrtInfo.depr_time_prin_yn[0] = m_tRespRePubTck.depr_time_prin_yn[0];
		/// (03). 거리
		//if(strlen(base.bus_oprn_dist) > 0)
		//{
		//	sprintf(tPrtInfo.thru_nm, "%.*skm", sizeof(tPrtInfo.thru_nm) - 1, base.bus_oprn_dist);
		//}
 		/// (04). 출발지 터미널명 (한글)
		Find_TmExpTrmlName(LANG_KOR, tList.depr_trml_no, tPrtInfo.depr_trml_ko_nm);
 		/// (05). 출발지 터미널명 (영문)
		Find_TmExpTrmlName(LANG_ENG, tList.depr_trml_no, tPrtInfo.depr_trml_eng_nm);
		/// (06). 출발지 터미널 전화번호
		{
			nLen = CheckBuffer(pBuff = pConfig->ezTrmlInfo_t.sz_prn_trml_tel_no, sizeof(tPrtInfo.depr_trml_tel_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_tel_nm, pBuff, nLen);
			}
		}
 		/// (07). 도착지 터미널명 (한글)
 		Find_TmExpTrmlName(LANG_KOR, tList.arvl_trml_no, tPrtInfo.arvl_trml_ko_nm);
 		/// (08). 도착지 터미널명 (영문)
		Find_TmExpTrmlName(LANG_ENG, tList.arvl_trml_no, tPrtInfo.arvl_trml_eng_nm);
 		/// (09). 버스운수사 명
 		nRet = FindBusCacmName(SVR_DVS_TMEXP, tList.cacm_cd, tPrtInfo.bus_cacm_nm, tPrtInfo.bus_cacm_biz_no, tPrtInfo.bus_cacm_tel_nm);	
 		/// (10). 버스등급명
 		nRet = FindBusClsName(SVR_DVS_TMEXP, tList.bus_cls_cd, tPrtInfo.bus_cls_nm);		
  		/// (11). 버스티켓종류명
		::CopyMemory(tPrtInfo.bus_tck_knd_cd, tList.tck_knd_cd, sizeof(tPrtInfo.bus_tck_knd_cd));
  		nRet = FindBusTckKndName(SVR_DVS_TMEXP, tList.tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 

		if( tList.pyn_dtl_cd[0] == '1' )
		{	/// 신용카드
			/// (12). 신용카드 승인번호
			sprintf(tPrtInfo.card_aprv_no, "%s", tList.card_aprv_no);
			/// (14). 카드번호
			{
				int len, i, k;
				char Buffer[100];

				::ZeroMemory(Buffer, sizeof(Buffer));

				len = strlen(tList.card_no);
				for(k = 0, i = 0; i < len; i++)
				{
					if(tList.card_no[i] != '-')
					{
						Buffer[k++] = tList.card_no[i] & 0xFF;
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
#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(Buffer, sizeof(Buffer));
#endif
			}

			/// (15). 결제수단 / 할부 개월 수 
			tPrtInfo.n_pym_dvs = PYM_CD_CARD;
			tPrtInfo.n_mip_mm_num = 0;
			if(tPrtInfo.n_mip_mm_num == 0)
			{
				sprintf(tPrtInfo.pym_dvs, "현장카드(일시불)");	
			}
			else
			{
				sprintf(tPrtInfo.pym_dvs, "현장카드(%d 개월)", tPrtInfo.n_mip_mm_num);	
			}
			sprintf(tPrtInfo.shrt_pym_dvs, "카드");
		}
		else if( (tList.pyn_dtl_cd[0] == '2') || (tList.pyn_dtl_cd[0] == '3') )
		{	/// 현금 
			/// (12). 승인번호
			sprintf(tPrtInfo.card_aprv_no, "%s", tList.csrc_aprv_no);
			
			/// 결제수단
			if(tList.pyn_dtl_cd[0] == '2')
			{
				tPrtInfo.n_pym_dvs = PYM_CD_CASH;
				sprintf(tPrtInfo.pym_dvs, "현금(자진발급)");	
				sprintf(tPrtInfo.shrt_pym_dvs, "현금");
			}
			else
			{	
				tPrtInfo.n_pym_dvs = PYM_CD_CASH;
				sprintf(tPrtInfo.pym_dvs, "현금(수기)");	
				sprintf(tPrtInfo.shrt_pym_dvs, "현영");
				// 수기만 해당...
				CMN_MakePasswdChars(tPrtInfo.card_no, m_tResPubTckCsrcKtc.user_key_val);
			}
		}

 		/// (13). 승인금액, 발권요금
 		{
 			int nValue = 0;
 			char szFare[100];
 
			/// 승인금액
 			nValue = *(int *) tList.tissu_fee;
 			::ZeroMemory(szFare, sizeof(szFare));
 			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.card_aprv_amt, "%s", szFare);	

			/// 요금
			nValue = *(int *) tList.tissu_fee;
			::ZeroMemory(szFare, sizeof(szFare));
			tPrtInfo.n_tisu_amt = nValue;
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.tisu_amt, "%s", szFare);	
		}

 
		/// (16). 좌석번호
		sprintf(tPrtInfo.sats_no, "%s", tList.sats_no);	

		/// (17). 출발 일자, 출발시간
		MakeDepartureDateTime(tList.depr_dt, 
							  tList.depr_time, 
							  tPrtInfo.atl_depr_dt, 
							  tPrtInfo.atl_depr_time);
 		/// (18). 승차홈
 		sprintf(tPrtInfo.rdhm_val, "%s", m_tRespRePubTck.rot_rdhm_no_val);	
		// (19). 발권시간
		sprintf(tPrtInfo.pub_time, "(%.*s:%.*s:%.*s)", 
				2, &tList.tissu_time[0], 
				2, &tList.tissu_time[2], 
				2, &tList.tissu_time[4]);
		/// (20). QR 바코드 관련
 		{
 			/// 1) 발행일자
			//sprintf(tPrtInfo.qr_pub_dt, "%s", m_tReqTckIssue.depr_dt);	
 			sprintf(tPrtInfo.qr_pub_dt, "%s", tList.tissu_dt);	
 			/// 2) 발행터미널코드
 			//sprintf(tPrtInfo.qr_pub_trml_cd, "%s", iter->tissu_trml_no);	
			sprintf(tPrtInfo.qr_pub_trml_cd, "%s", tList.tissu_trml_no);	
 			/// 3) 창구번호
 			//sprintf(tPrtInfo.qr_pub_wnd_no, "%s", iter->tissu_wnd_no);	
			sprintf(tPrtInfo.qr_pub_wnd_no, "%s", tList.tissu_wnd_no);	
 			/// 4) 발행일련번호
 			sprintf(tPrtInfo.qr_pub_sno, "%s", tList.tissu_sno);
			/// 5) 출발지 터미널번호
			sprintf(tPrtInfo.qr_depr_trml_no, "%s", tList.depr_trml_no);
			/// 6) 도착지 터미널번호
			sprintf(tPrtInfo.qr_arvl_trml_no, "%s", tList.arvl_trml_no);
			/// 7) 좌석번호
			sprintf(tPrtInfo.qr_sats_no, "%s", tList.sats_no);

			TR_LOG_OUT("qr발행일자 = %s", tPrtInfo.qr_pub_dt);
			TR_LOG_OUT("qr발행터미널번호 = %s", tPrtInfo.qr_pub_trml_cd);
			TR_LOG_OUT("qr발행창구번호 = %s", tPrtInfo.qr_pub_wnd_no);
			TR_LOG_OUT("qr발행일련번호 = %s", tPrtInfo.qr_pub_sno);
			TR_LOG_OUT("qr출발터미널번호 = %s", tPrtInfo.qr_depr_trml_no);
			TR_LOG_OUT("qr도착터미널번호 = %s", tPrtInfo.qr_arvl_trml_no);
			TR_LOG_OUT("qr좌석번호 = %s", tPrtInfo.qr_sats_no);
 		}
		/// (21). 항목 출력여부
		{
			///< 버스등급 출력여부		
			tPrtInfo.bus_cls_prin_yn[0]		= m_tRespRePubTck.bus_cls_prin_yn[0];
			///< 버스운수사명 출력여부	
			tPrtInfo.bus_cacm_nm_prin_yn[0] = m_tRespRePubTck.cacm_nm_prin_yn[0];
			///< 출발시각 출력여부
			//tPrtInfo.depr_time_prin_yn[0]	= cash_resp.depr_time_prin_yn[0];
			///< 좌석번호 출력여부
			//tPrtInfo.sats_no_prin_yn[0]		= cash_resp.sats_no_prin_yn[0];
		}
		/// (22). 사용자 이름
		{
			int nLen;
			char *pBuff;

			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->user_nm, sizeof(tPrtInfo.user_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.user_nm, pBuff, nLen);
			}
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
 * @brief		CMrnpTmExpMem
 * @details		[티머니고속] 생성자
 */
CMrnpTmExpMem::CMrnpTmExpMem()
{

}

/**
 * @brief		~CMrnpTmExpMem
 * @details		[티머니고속] 소멸자
 */
CMrnpTmExpMem::~CMrnpTmExpMem()
{

}

/**
 * @brief		Initialize
 * @details		[티머니고속] - 예매발권 메모리 초기화
 * @param		None
 * @return		항상 = 0
 */
void CMrnpTmExpMem::Initialize(void)
{
	KTC_MemClear(&Base, sizeof(MRNP_TMEXP_BASE_T));

	KTC_MemClear(&m_ReqList, sizeof(UI_RESP_TMEXP_MRS_REQ_T));
	m_vtMrnpList.clear();
	vector<rtk_tm_read_mrs_list_t>().swap(m_vtMrnpList);

	KTC_MemClear(&m_ReqKtcList, sizeof(UI_RESP_TMEXP_KTC_MRS_REQ_T));
	m_vtMrnpKtcList.clear();
	vector<rtk_tm_read_mrs_ktc_list_t>().swap(m_vtMrnpKtcList);

	KTC_MemClear(&m_ReqPubMrs, sizeof(UI_RESP_TMEXP_MRS_REQ_PUB_T));
	m_vtResComplete.clear();
	vector<rtk_tm_pub_mrs_list_t>().swap(m_vtResComplete);

	KTC_MemClear(&m_ReqPubMobileMrs, sizeof(UI_RESP_TMEXP_MRS_REQ_MOBILE_PUB_T));
	m_vtResCompleteMobile.clear();
	vector<rtk_tm_pub_mrs_htck_list_t>().swap(m_vtResCompleteMobile);

	m_vtPrtTicket.clear();
	vector<TCK_PRINT_FMT_T>().swap(m_vtPrtTicket);

	TR_LOG_OUT("%s", "##### Memory release !!");
}

/**
 * @brief		MakeMrnpListPacket
 * @details		[티머니고속] 예매내역 UI 전송 패킷 작성
 * @param		char *pData			UI 전송 버퍼
 * @return		전송 길이
 */
int CMrnpTmExpMem::MakeMrnpListPacket(char *pData)
{
	int i, nOffset, k, nCount;
	vector<rtk_tm_read_mrs_list_t>::iterator	iter;

	i = nOffset = 0; 

	///< 결과
	pData[nOffset++] = CHAR_ACK;

	///< 결과
	nOffset += ::Util_MyCopyMemory(&pData[nOffset], Base.rsp_cd, sizeof(Base.rsp_cd));
	
	///< 예매건수
	nCount = m_vtMrnpList.size();
	nOffset += Util_MyCopyMemory(&pData[nOffset], &nCount, sizeof(int));	
	
	for( iter = m_vtMrnpList.begin(); iter != m_vtMrnpList.end(); iter++ )
	{
		nOffset += Util_MyCopyMemory(&pData[nOffset], 
									 iter->mrs_mrnp_no,		
									 iter->mrs_num - iter->mrs_mrnp_no);


		/// 예매매수
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->mrs_num, sizeof(iter->mrs_num));
		Util_MyCopyMemory(&nCount, iter->mrs_num, sizeof(iter->mrs_num));
		
		/// 예매 정보
		for(k = 0; k < nCount; k++)
		{
			nOffset += Util_MyCopyMemory(&pData[nOffset], 
										 &iter->mrs_info[k],		
										 sizeof(rtk_tm_read_mrs_info_t));
		}
	}

	return nOffset;
}

/**
 * @brief		MakeMrnpKtcListPacket
 * @details		[티머니고속] KTC 예매내역 UI 전송 패킷 작성
 * @param		int nCount			예매내역 건수
 * @param		char *pData			UI 전송 버퍼
 * @return		전송 길이
 */
int CMrnpTmExpMem::MakeMrnpKtcListPacket(char *pData)
{
	int i, nOffset, k, nCount;
	vector<rtk_tm_read_mrs_ktc_list_t>::iterator	iter;

	i = nOffset = 0; 

	///< 결과
	pData[nOffset++] = CHAR_ACK;
	
	///< 결과
	nOffset += ::Util_MyCopyMemory(&pData[nOffset], Base.rsp_cd, sizeof(Base.rsp_cd));

	///< 예매건수
	nCount = m_vtMrnpKtcList.size();
	nOffset += Util_MyCopyMemory(&pData[nOffset], &nCount, sizeof(int));	
	
	for( iter = m_vtMrnpKtcList.begin(); iter != m_vtMrnpKtcList.end(); iter++ )
	{
		nOffset += Util_MyCopyMemory(&pData[nOffset], 
									 iter->mrs_mrnp_no,		
									 iter->mrs_num - iter->mrs_mrnp_no);


		/// 예매매수
		nOffset += Util_MyCopyMemory(&pData[nOffset], iter->mrs_num, sizeof(iter->mrs_num));
		Util_MyCopyMemory(&nCount, iter->mrs_num, sizeof(iter->mrs_num));
		
		/// KTC_예매정보
		for(k = 0; k < nCount; k++)
		{
			nOffset += Util_MyCopyMemory(&pData[nOffset], 
										 &iter->mrs_info[k],		
										 sizeof(rtk_tm_read_mrs_ktc_info_t));
		}
	}

	return nOffset;
}

int CMrnpTmExpMem::MakeReqIssuePacket(char *pData)
{
	return 0;
}

/**
 * @brief		MakePubMrnpResultPacket
 * @details		[티머니고속] 예매발권 결과 UI 전송 패킷 작성
 * @param		char *pSPacket		예매발권 결과 데이타 버퍼
 * @return		전송 길이
 */
int CMrnpTmExpMem::MakePubMrnpResultPacket(char *pSPacket)
{
	int nOffset, nCount;
	BYTE byACK = CHAR_ACK;
	vector<rtk_tm_pub_mrs_list_t>::iterator	iter;

	nOffset = nCount = 0;

	/// 결과
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], &byACK, 1);

	/// rsp_code
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], Base.rsp_cd, sizeof(Base.rsp_cd));
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], this->Base.pyn_dtl_cd	, sizeof(this->Base.pyn_dtl_cd		));
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], this->Base.card_no		, sizeof(this->Base.card_no			));
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], this->Base.card_aprv_no, sizeof(this->Base.card_aprv_no	));
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], this->Base.aprv_amt	, sizeof(this->Base.aprv_amt		));
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], this->Base.frc_cmrt	, sizeof(this->Base.frc_cmrt		));


	/// 예매발권 건수
	nCount = this->m_vtResComplete.size();
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], &nCount, 4);


	/// 예매 발권결과 정보
	for( iter = this->m_vtResComplete.begin(); iter != this->m_vtResComplete.end(); iter++ )
	{
		nOffset += Util_MyCopyMemory(&pSPacket[nOffset], (char *)iter._Ptr, sizeof(rtk_tm_pub_mrs_list_t));
	}

	return nOffset;
}

/**
 * @brief		MakePubMrnpMobileResultPacket
 * @details		[티머니고속] 모바일 예매발권 결과 UI 전송 패킷 작성
 * @param		char *pSPacket		예매발권 결과 데이타 버퍼
 * @return		전송 길이
 */
int CMrnpTmExpMem::MakePubMrnpMobileResultPacket(char *pSPacket)
{
	int nOffset, nCount;
	BYTE byACK = CHAR_ACK;
	vector<rtk_tm_pub_mrs_htck_list_t>::iterator	iter;

	nOffset = nCount = 0;

	/// 결과
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], &byACK, 1);

	/// rsp_code
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], Base.rsp_cd, sizeof(Base.rsp_cd));

	/// rsp_data
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], this->Base.card_no		, sizeof(this->Base.card_no			));
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], this->Base.aprv_amt	, sizeof(this->Base.aprv_amt		));
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], this->Base.card_aprv_no, sizeof(this->Base.card_aprv_no	));
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], this->Base.frc_cmm		, sizeof(this->Base.frc_cmm			));

	/// 예매발권 건수
	nCount = this->m_vtResCompleteMobile.size();
	nOffset += Util_MyCopyMemory(&pSPacket[nOffset], &nCount, 4);


	/// 예매 발권결과 정보
	for( iter = this->m_vtResCompleteMobile.begin(); iter != this->m_vtResCompleteMobile.end(); iter++ )
	{
		nOffset += Util_MyCopyMemory(&pSPacket[nOffset], (char *)iter._Ptr, sizeof(rtk_tm_pub_mrs_htck_list_t));
	}

	return nOffset;
}

/**
 * @brief		GetPynDtlCdByMrnpNo
 * @details		[티머니고속] 예매번호로 지불상세코드 찾기
 * @param		char *mrs_mrnp_no		예매번호
 * @param		char *retBuf			지불상세코드 버퍼
 * @return		성공 >= 0, 실패 < 0
 */
int CMrnpTmExpMem::GetPynDtlCdByMrnpNo(char *mrs_mrnp_no, char *retBuf)
{
	int nCount, nLen, nLen1, nFind, i;
	vector<rtk_tm_read_mrs_list_t>::iterator iter;
	vector<rtk_tm_read_mrs_ktc_list_t>::iterator iter_ktc;

	i = nCount = nLen = nLen1 = 0;
	nFind = -1;

	nLen = strlen(mrs_mrnp_no);
	nCount = m_vtMrnpList.size();
	TR_LOG_OUT("#0. m_vtMrnpList.size()(%d) ..", nCount);
	if(nCount > 0)
	{
		i = 0;
		for(iter = m_vtMrnpList.begin(); iter != m_vtMrnpList.end(); iter++)
		{
			nLen1 = strlen(iter->mrs_mrnp_no);
			TR_LOG_OUT("#1. mrs_mrnp_no (%s:%s), nLen(%d:%d) ..", mrs_mrnp_no, iter->mrs_mrnp_no, nLen, nLen1);
			TR_LOG_OUT("#2. i=%d, iter->pyn_dtl_cd(0x%02X, %c) ..", i, iter->pyn_dtl_cd[0] & 0xFF, iter->pyn_dtl_cd[0]);
			if(nLen == nLen1)
			{
				if( memcmp(mrs_mrnp_no, iter->mrs_mrnp_no, nLen) == 0 )
				{
					retBuf[0] = iter->pyn_dtl_cd[0];
					nFind = i;
					break;
				}
			}
			i++;
		}
	}
	else
	{
		nCount = m_vtMrnpKtcList.size();
		TR_LOG_OUT("#10. m_vtMrnpKtcList.size()(%d) ..", nCount);
		if(nCount > 0)
		{
			i = 0;
			for(iter_ktc = m_vtMrnpKtcList.begin(); iter_ktc != m_vtMrnpKtcList.end(); iter_ktc++)
			{
				nLen1 = strlen(iter_ktc->mrs_mrnp_no);
				TR_LOG_OUT("#11. mrs_mrnp_no (%s:%s), nLen(%d:%d) ..", mrs_mrnp_no, iter_ktc->mrs_mrnp_no, nLen, nLen1);
				TR_LOG_OUT("#12. i=%d, iter->pyn_dtl_cd(0x%02X, %c) ..", i, iter_ktc->pyn_dtl_cd[0] & 0xFF, iter_ktc->pyn_dtl_cd[0]);
				if(nLen == nLen1)
				{
					if( memcmp(mrs_mrnp_no, iter_ktc->mrs_mrnp_no, nLen) == 0 )
					{
						retBuf[0] = iter_ktc->pyn_dtl_cd[0];
						nFind = i;
						break;
					}
				}
				i++;
			}
		}
	}

	return nFind;
}

/**
 * @brief		MakeTicketPrtData
 * @details		[티머니고속] 예매발권 승차권 데이타 작성
 * @param		None
 * @return		항상 = 0
 */
int CMrnpTmExpMem::MakeTicketPrtData(void)
{
 	int nRet, n_mip_mm_num;
	int nYear, nMonth, nDay, nWeek;
	char *pWeekStr[] = { "일", "월", "화", "수", "목", "금", "토", };
	POPER_FILE_CONFIG_T pConfig;


	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	nYear = nMonth = nDay = nWeek = 0;

	nRet = n_mip_mm_num = 0;

 	m_vtPrtTicket.clear();

	if(m_vtResCompleteMobile.size() > 0)
	{
		vector<rtk_tm_pub_mrs_htck_list_t>::iterator iter;

		for(iter = m_vtResCompleteMobile.begin(); iter != m_vtResCompleteMobile.end(); iter++)
 		{
 			TCK_PRINT_FMT_T tPrtInfo;
			char pyn_dtl_cd[1];
 
 			::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
 
 			/// (01). 출발지 터미널명 (한글)
			Find_TmExpTrmlName(LANG_KOR, iter->depr_trml_no, tPrtInfo.depr_trml_ko_nm);
 		
			/// (02). 출발지 터미널명 (영문)
			Find_TmExpTrmlName(LANG_ENG, iter->depr_trml_no, tPrtInfo.depr_trml_eng_nm);

			/// (03). 도착지 터미널명 (한글)
			Find_TmExpTrmlName(LANG_KOR, iter->arvl_trml_no, tPrtInfo.arvl_trml_ko_nm);

			/// (04). 도착지 터미널명 (영문)
			Find_TmExpTrmlName(LANG_ENG, iter->arvl_trml_no, tPrtInfo.arvl_trml_eng_nm);

			/// (05). 출발지 터미널 사업자번호
			sprintf(tPrtInfo.depr_trml_biz_no, "%s", pConfig->ezTrmlInfo_t.sz_prn_trml_corp_no1);	

			/// (05). 버스운수사 명
			nRet = FindBusCacmName(SVR_DVS_TMEXP, iter->cacm_cd, tPrtInfo.bus_cacm_nm, tPrtInfo.bus_cacm_biz_no, tPrtInfo.bus_cacm_tel_nm);	
			//sprintf(tPrtInfo.bus_cacm_nm, "%s", iter->cacm_nm);
 		
			/// (06). 버스등급명
			//sprintf(tPrtInfo.bus_cls_nm, "%s", iter->bus_cls_nm);
			nRet = FindBusClsName(SVR_DVS_TMEXP, iter->bus_cls_cd, tPrtInfo.bus_cls_nm);		
 		
			/// (07). 버스티켓종류명
 			nRet = FindBusTckKndName(SVR_DVS_TMEXP, iter->tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 
			//CConfigTkMem::GetInstance()->GetKobusCorpInfo(iter->bus_cls_cd, tPrtInfo.bus_cacm_full_nm, tPrtInfo.bus_cacm_biz_no);
			::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->tck_knd_cd, sizeof(iter->tck_knd_cd));
 		
			/// (08). 신용카드 승인번호
 			sprintf(tPrtInfo.card_aprv_no, "%s", this->Base.card_aprv_no);
 		
			/// (09). 신용카드 승인금액
 			{
 				int nValue = 0;
 				char szFare[100];
 
				/// 승인금액
 				nValue = *(int *) this->Base.aprv_amt;
 				::ZeroMemory(szFare, sizeof(szFare));
 				Util_AmountComma(nValue, szFare);
 				sprintf(tPrtInfo.card_aprv_amt, "%s", szFare);	

				/// 요금
				nValue = *(int *) iter->n_tissu_amt;
				tPrtInfo.n_tisu_amt = nValue;
				::ZeroMemory(szFare, sizeof(szFare));
				Util_AmountComma(nValue, szFare);
				sprintf(tPrtInfo.tisu_amt, "%s", szFare);	
 			}
 		
			/// (10). 카드번호
 			{
 				int len, i, k;
 
 				len = strlen(this->Base.card_no);
 				for(k = 0, i = 0; i < len; i++)
 				{
 					if(this->Base.card_no[i] != '-')
 					{
 						tPrtInfo.card_no[k++] = this->Base.card_no[i] & 0xFF;
 					}
 				}
 			}
 		
			/// (11). 결제수단 (현금/신용카드 등)
			pyn_dtl_cd[0] = 0;

			//m_ReqPubMobileMrs.rData.mrs_mrnp_no;

			/// 2021.04.05 modify by atectn 
			//nRet = GetPynDtlCdByMrnpNo(iter->sats_no, pyn_dtl_cd);
			nRet = GetPynDtlCdByMrnpNo(m_ReqPubMobileMrs.rData.mrs_mrnp_no, pyn_dtl_cd);
			TR_LOG_OUT("GetPynDtlCdByMrnpNo(), nRet(%d), pyn_dtl_cd[%c] ...", nRet, pyn_dtl_cd[0]);
			/// ~2021.04.05 modify by atectn 
			if(nRet >= 0)
			{
				switch(pyn_dtl_cd[0])
				{
				case 'e' :	/// 티머니페이
				case 'f' :	/// 비즈페이
					tPrtInfo.n_pym_dvs = PYM_CD_TPAY;
					sprintf(tPrtInfo.pym_dvs, "페이");	
					break;
				// 20211013 ADD
				case 'h' :	/// 후불단독
				case 'i' :	/// 가상선불단독
				case 'j' :	/// 마일리지단독
				case 'k' :	/// 마일리지+쿠폰
				case 'l' :	/// 후불+마일리지
				case 'm' :	/// 후불+쿠폰
				case 'n' :	/// 후불+마일리지+쿠폰
				case 'o' :	/// 가상선불+마일리지
				case 'p' :	/// 가상선불+쿠폰
				case 'q' :	/// 가상선불+마일리지+쿠폰
					tPrtInfo.n_pym_dvs = PYM_CD_ETC;
					sprintf(tPrtInfo.pym_dvs, "복합결제");	
					break;
				// 20211013 ~ADD
				default:
					tPrtInfo.n_pym_dvs = PYM_CD_CARD;
					sprintf(tPrtInfo.pym_dvs, "예매카드");	
					break;
				}
			}
			else
			{
				tPrtInfo.n_pym_dvs = PYM_CD_CARD;
				sprintf(tPrtInfo.pym_dvs, "예매카드");	
			}

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
 			sprintf(tPrtInfo.rdhm_val, "%s", iter->rot_rdhm_no_val);	
 		
			/// (16) 거리 
			if( strlen(iter->bus_oprn_dist) > 0 )
			{
				sprintf(tPrtInfo.thru_nm, "%.*skm", sizeof(tPrtInfo.thru_nm) - 1, iter->bus_oprn_dist);	
			}

			/// (17). QR코드 관련
 			{
				//SYSTEMTIME st;
			
				//GetLocalTime(&st);

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
				//sprintf(tPrtInfo.pub_time, "(%02d:%02d)", st.wHour, st.wMinute);
				sprintf(tPrtInfo.pub_time, "(%.*s:%.*s)", 2, &iter->tissu_time[0], 2, &iter->tissu_time[2]);
 		
				TR_LOG_OUT("qr발행일자 = %s",		tPrtInfo.qr_pub_dt);
				TR_LOG_OUT("qr발행터미널번호 = %s",	tPrtInfo.qr_pub_trml_cd);
				TR_LOG_OUT("qr발행창구번호 = %s",	tPrtInfo.qr_pub_wnd_no);
				TR_LOG_OUT("qr발행일련번호 = %s",	tPrtInfo.qr_pub_sno);
				TR_LOG_OUT("qr출발터미널번호 = %s",	tPrtInfo.qr_depr_trml_no);
				TR_LOG_OUT("qr도착터미널번호 = %s",	tPrtInfo.qr_arvl_trml_no);
				TR_LOG_OUT("qr좌석번호 = %s",		tPrtInfo.qr_sats_no);
			}
 			m_vtPrtTicket.push_back(tPrtInfo);
 
#if (_KTC_CERTIFY_ > 0)
 			KTC_MemClear(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
#endif
 		}
		m_vtResCompleteMobile.clear();
	}
	else
	{
		vector<rtk_tm_pub_mrs_list_t>::iterator iter;

		if(m_vtResComplete.size() > 0)
		{
			for(iter = m_vtResComplete.begin(); iter != m_vtResComplete.end(); iter++)
			{
				TCK_PRINT_FMT_T tPrtInfo;
				char pyn_dtl_cd[1];

				::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));

				/// (01). 출발지 터미널명 (한글)
				Find_TmExpTrmlName(LANG_KOR, iter->depr_trml_no, tPrtInfo.depr_trml_ko_nm);

				/// (02). 출발지 터미널명 (영문)
				Find_TmExpTrmlName(LANG_ENG, iter->depr_trml_no, tPrtInfo.depr_trml_eng_nm);

				/// (03). 도착지 터미널명 (한글)
				Find_TmExpTrmlName(LANG_KOR, iter->arvl_trml_no, tPrtInfo.arvl_trml_ko_nm);

				/// (04). 도착지 터미널명 (영문)
				Find_TmExpTrmlName(LANG_ENG, iter->arvl_trml_no, tPrtInfo.arvl_trml_eng_nm);

				/// (05). 출발지 터미널 사업자번호
				sprintf(tPrtInfo.depr_trml_biz_no, "%s", pConfig->ezTrmlInfo_t.sz_prn_trml_corp_no1);	

				/// (05). 버스운수사 명
				nRet = FindBusCacmName(SVR_DVS_TMEXP, iter->cacm_cd, tPrtInfo.bus_cacm_nm, tPrtInfo.bus_cacm_biz_no, tPrtInfo.bus_cacm_tel_nm);
				//sprintf(tPrtInfo.bus_cacm_nm, "%s", iter->cacm_nm);

				/// (06). 버스등급명
				nRet = FindBusClsName(SVR_DVS_TMEXP, iter->bus_cls_cd, tPrtInfo.bus_cls_nm);		
				//sprintf(tPrtInfo.bus_cls_nm, "%s", iter->bus_cls_nm);

				/// (07). 버스티켓종류명
				nRet = FindBusTckKndName(SVR_DVS_TMEXP, iter->tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 
				//CConfigTkMem::GetInstance()->GetKobusCorpInfo(iter->bus_cls_cd, tPrtInfo.bus_cacm_full_nm, tPrtInfo.bus_cacm_biz_no);
				::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->tck_knd_cd, sizeof(iter->tck_knd_cd));

				/// (08). 신용카드 승인번호
				sprintf(tPrtInfo.card_aprv_no, "%s", this->Base.card_aprv_no);

				/// (09). 신용카드 승인금액
				{
					int nValue = 0;
					char szFare[100];

					/// 승인금액
					nValue = *(int *) this->Base.aprv_amt;
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

					len = strlen(this->Base.card_no);
					for(k = 0, i = 0; i < len; i++)
					{
						if(this->Base.card_no[i] != '-')
						{
							tPrtInfo.card_no[k++] = this->Base.card_no[i] & 0xFF;
						}
					}
				}

				/// (11). 결제수단 (현금/신용카드 등)
				pyn_dtl_cd[0] = 0;

				//m_ReqPubMobileMrs.rData.mrs_mrnp_no;

				/// 2021.04.05 modify by atectn 
				//nRet = GetPynDtlCdByMrnpNo(iter->sats_no, pyn_dtl_cd);
				nRet = GetPynDtlCdByMrnpNo(m_ReqPubMrs.rData.mrs_mrnp_no, pyn_dtl_cd);
				TR_LOG_OUT("GetPynDtlCdByMrnpNo(), nRet(%d), pyn_dtl_cd[%c] ...", nRet, pyn_dtl_cd[0]);
				/// ~2021.04.05 modify by atectn 
				if(nRet >= 0)
				{
					switch(pyn_dtl_cd[0])
					{
					case 'e' :	/// 티머니페이
					case 'f' :	/// 비즈페이
						tPrtInfo.n_pym_dvs = PYM_CD_TPAY;
						sprintf(tPrtInfo.pym_dvs, "페이");	
						break;
					default:
						tPrtInfo.n_pym_dvs = PYM_CD_CARD;
						sprintf(tPrtInfo.pym_dvs, "예매카드");	
						break;
					}
				}
				else
				{
					tPrtInfo.n_pym_dvs = PYM_CD_CARD;
					sprintf(tPrtInfo.pym_dvs, "예매카드");	
				}

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
				sprintf(tPrtInfo.rdhm_val, "%s", iter->rot_rdhm_no_val);	

				/// (16) 거리 
				if( strlen(iter->bus_oprn_dist) > 0 )
				{
					sprintf(tPrtInfo.thru_nm, "%.*skm", sizeof(tPrtInfo.thru_nm) - 1, iter->bus_oprn_dist);	
				}

				/// (17). QR코드 관련
				{
					//SYSTEMTIME st;

					//GetLocalTime(&st);

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
					//sprintf(tPrtInfo.pub_time, "(%02d:%02d)", st.wHour, st.wMinute);
					sprintf(tPrtInfo.pub_time, "(%.*s:%.*s)", 2, &iter->tissu_time[0], 2, &iter->tissu_time[2]);

					TR_LOG_OUT("qr발행일자 = %s",		tPrtInfo.qr_pub_dt);
					TR_LOG_OUT("qr발행터미널번호 = %s",	tPrtInfo.qr_pub_trml_cd);
					TR_LOG_OUT("qr발행창구번호 = %s",	tPrtInfo.qr_pub_wnd_no);
					TR_LOG_OUT("qr발행일련번호 = %s",	tPrtInfo.qr_pub_sno);
					TR_LOG_OUT("qr출발터미널번호 = %s",	tPrtInfo.qr_depr_trml_no);
					TR_LOG_OUT("qr도착터미널번호 = %s",	tPrtInfo.qr_arvl_trml_no);
					TR_LOG_OUT("qr좌석번호 = %s",		tPrtInfo.qr_sats_no);
				}
				m_vtPrtTicket.push_back(tPrtInfo);

#if (_KTC_CERTIFY_ > 0)
				KTC_MemClear(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
#endif
			}
			m_vtResComplete.clear();
		}
	}
	return 0;
}

/**
 * @brief		CCancRyTkTmExpMem
 * @details		[티머니고속] 환불관련 class 생성자
 */
CCancRyTkTmExpMem::CCancRyTkTmExpMem()
{
}

/**
 * @brief		~CCancRyTkTmExpMem
 * @details		[티머니고속] 환불관련 class 소멸자
 */
CCancRyTkTmExpMem::~CCancRyTkTmExpMem()
{

}

/**
 * @brief		Initialize
 * @details		[티머니고속] 환불 메모리 초기화
 * @param		None
 * @return		항상 = 0
 */
void CCancRyTkTmExpMem::Initialize(void)
{
	KTC_MemClear(&tBase, sizeof(CANCRY_TMEXP_BASE_T));

	/// 환불조회
	KTC_MemClear(&tReqInq, sizeof(stk_tm_read_bus_tckno_t));
	KTC_MemClear(&tRespInq, sizeof(rtk_tm_read_bus_tckno_t));

	/// 환불
	KTC_MemClear(&tReqRef, sizeof(stk_tm_cancrytck_t));
	m_vtRespRefList.clear();
	vector<rtk_tm_cancrytck_list_t>().swap(m_vtRespRefList);

	TR_LOG_OUT("%s", "##### Memory release !!");
}

/**
 * @brief		CheckRefund
 * @details		[티머니고속] 승차권 환불 유무 체크
 * @param		None
 * @return		성공 > 0, 실패 < 0
 */
int CCancRyTkTmExpMem::CheckRefund(void)
{
	int					nLen, nCount;
	POPER_FILE_CONFIG_T	pConfig;
	PFMT_QRCODE_T		pQRCode;
	char				*pIssTrmlCode;

	nLen = nCount = 0;

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();
	pQRCode = (PFMT_QRCODE_T) &tBase.szTicketData;
	
	/// 1. 터미널코드 체크
	pIssTrmlCode = GetTrmlCode(SVR_DVS_TMEXP);
	nLen = strlen(pIssTrmlCode);
	if(nLen < 0)
	{
		TR_LOG_OUT("터미널 코드 없음 에러, 터미널코드 = %s ", pIssTrmlCode);
		return -1;
	}

	/// 2. 출발지 터미널코드,  도착지 터미널코드 체크
	{
		if( (memcmp(pQRCode->depr_trml_no, pIssTrmlCode, 3) != 0) && (memcmp(pQRCode->arvl_trml_no, pIssTrmlCode, 3) != 0) )
		{
			TR_LOG_OUT("[티머니고속] 자기 터미널 코드 아님 에러, 터미널코드 = (%s)(%s) ", pQRCode->depr_trml_no, pQRCode->arvl_trml_no);
			return -2;
		}
	}

	/// 3. 발권상태 체크 - CMN_CD:C037
	{
		BOOL bError;

		switch( tRespInq.tissu_sta_cd[0] )
		{
		case '3' :	/// 발권
		case '4' :	/// 예약발권
		case '5' :	/// 예약현금
		case '6' :	/// 예매발권
		case '8' :	/// 홈티켓
			bError = FALSE;
			break;
		case '1' :	/// 예약
		case '2' :	/// 예매
		case '7' :	/// 가예매
		default:
			bError = TRUE;
			break;
		}

		if(bError == TRUE)
		{
			TR_LOG_OUT("[티머니고속] 발권상태 정보 에러, 코드 = 0x%02X ", tRespInq.tissu_sta_cd[0]);
			return -3;
		}
	}


	/***
	/// 4. 환불종류 코드 체크
	{
		
		{
			BOOL bError;

			switch( tRespInq->ry_knd_cd[0] )	/// CMN_CD:C067
			{
			case '0' :	/// 미할인
			case '1' :	/// 사전
			case '3' :	/// 뒷좌석할인
				bError = FALSE;
				break;
			case '2' :	/// 5인 단체할인
			case '4' :	/// 왕복할인
			case '5' :	/// 4인 단체할인
			default : 
				bError = TRUE;
				break;
			}

			if(bError == TRUE)
			{
				TR_LOG_OUT("[티머니고속] 환불할인율 정보 에러, 코드 = 0x%02X ", iter->ry_knd_cd[0]);
				return -99;
			}
		}
	}
	***/

	/// 4. 환불상태 코드 CMN_CD(?????)
	{
		switch( tRespInq.ry_sta_cd[0] )	
		{
		case '0' :
			break;
		case '3' : // 환불
		default:
			TR_LOG_OUT("[티머니고속] 환불상태코드 에러, 코드 = 0x%02X ", tRespInq.ry_sta_cd[0]);
			return -99;
		}
	}

	/// 지불상세코드 : CMN_CD(C075)
	{
		switch( tRespInq.pyn_dtl_cd[0] )
		{
		case '1' :	/// 카드
			tBase.ui_pym_dvs_cd[0] = PYM_CD_CARD;
			break;
		case '2' :	/// 현금 
		// 20230104 MOD~
		//case '3' :	/// 계좌이체
		//case '4' :	/// 현금영수증
		case '3' :	/// 3:현금영수증
		case '4' :	/// 4:계좌이체
		// 20230104 ~MOD
			tBase.ui_pym_dvs_cd[0] = PYM_CD_CASH;
			break;
		// 20211013 ADD~
		case 'h' :	/// 후불단독
		case 'i' :	/// 가상선불단독
		case 'j' :	/// 마일리지단독
		case 'k' :	/// 마일리지+쿠폰
		case 'l' :	/// 후불+마일리지
		case 'm' :	/// 후불+쿠폰
		case 'n' :	/// 후불+마일리지+쿠폰
		case 'o' :	/// 가상선불+마일리지
		case 'p' :	/// 가상선불+쿠폰
		case 'q' :	/// 가상선불+마일리지+쿠폰
		// 20230104 ADD~
		case 'd' :	/// d:스마일페이
		case 'e' :	/// e:티머니페이
		case 'f' :	/// f:Biz페이
		case 'g' :	/// g:PAYCO
		case 'r' :	/// r:토스페이
		case 's' :	/// s:네이버페이
		// 20230104 ~ADD
			tBase.ui_pym_dvs_cd[0] = PYM_CD_ETC;	// 복합결제
			break;
		// 20211013 ~ADD
		default:
			TR_LOG_OUT("결제수단 에러, pyn_dtl_cd = 0x%02X ", tRespInq.pyn_dtl_cd[0]);
			return -99;
		}

		switch(tBase.ui_pym_dvs_cd[0])
		{
		case PYM_CD_CASH :
		case PYM_CD_CSRC :
			/// 카드전용
			if( pConfig->base_t.sw_cash_pay_yn != 'Y' )
			{	
				TR_LOG_OUT("티머니고속_환불 - 현금환불 불가 error !!! ");
				return -99;
			}
			break;
		case PYM_CD_CARD :
			if( pConfig->base_t.sw_ic_pay_yn != 'Y' )
			{	
				TR_LOG_OUT("티머니고속_환불 - 카드환불 불가 error !!! ");
				return -99;
			}
			break;
		}
	}

	{
		/// 발권금액
		::CopyMemory( &tBase.n_tot_money, tRespInq.tissu_fee, 4 );

		/// 환불차익
		::CopyMemory( &tBase.n_commission_fare,	tRespInq.ry_pfit, 4 );
	
		tBase.n_commission_rate = 0;
	}
	
	TR_LOG_OUT("[티머니고속 환불_서버 조회 정보]");
	TR_LOG_OUT("1. 환불금액    = %d", tBase.n_tot_money);
	TR_LOG_OUT("2. 환불차익    = %d", tBase.n_commission_fare);
	TR_LOG_OUT("3. 할인반환금액 = %d", tBase.n_commission_rate);
	if(tBase.ui_pym_dvs_cd[0] == PYM_CD_CARD)
	{
		TR_LOG_OUT("3. 결제수단    = 신용카드");
	}
	// 20211013 ADD
	else if(tBase.ui_pym_dvs_cd[0] == PYM_CD_ETC)
	{
		TR_LOG_OUT("3. 결제수단    = 복합결제");
	}
	// 20211013 ~ADD
	else
	{
		TR_LOG_OUT("3. 결제수단    = 현금");
	}


	return 1;
}

/**
 * @brief		CalcRefund
 * @details		[티머니고속] 승차권 환불 계산하기
 * @param		None
 * @return		성공 > 0, 실패 < 0
 */
int CCancRyTkTmExpMem::CalcRefund(void)
{
	DWORD				dwCurrTck, dwDeprTck, dwPubTck;
	int					n_toal_fare;
	int					nBaseMinute = 10;		// 10분
	SYSTEMTIME			st;
	char				szCurrDT[20];
	PKIOSK_INI_ENV_T	pEnv;
	char				*pDate;
	char				*pTime;
	POPER_FILE_CONFIG_T	pConfig;

	TR_LOG_OUT("start !!!");

	pEnv	= (PKIOSK_INI_ENV_T) GetEnvInfo();
	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	dwCurrTck = dwDeprTck = dwPubTck = 0;
	tBase.n_commission_rate = 0; 

	/// (0) 현재일시
	GetLocalTime(&st);
	dwCurrTck = Util_GetCurrentTick();
	sprintf(szCurrDT, "%04d%02d%02d", st.wYear, st.wMonth, st.wDay);

	TR_LOG_OUT("(01)_현재일시 - (%04d-%02d-%02d %02d:%02d:%02d), Tick(%ld)..", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, dwCurrTck);
	TR_LOG_OUT("결제수단(%c), IC카드 사용유무(%c), 현금사용유무(%c) ", tBase.ui_pym_dvs_cd[0], pConfig->base_t.sw_ic_pay_yn, pConfig->base_t.sw_cash_pay_yn);

	switch(tBase.ui_pym_dvs_cd[0])
	{
	case PYM_CD_CASH :
	case PYM_CD_CSRC :
		/// 카드전용
		if( pConfig->base_t.sw_cash_pay_yn != 'Y' )
		{	
			TR_LOG_OUT("티머니고속_환불 - 현금환불 불가 error !!! ");
			return -99;
		}
		break;
	case PYM_CD_CARD :
	case PYM_CD_TPAY :
		if( pConfig->base_t.sw_ic_pay_yn != 'Y' )
		{	
			TR_LOG_OUT("티머니고속_환불 - 카드환불 불가 error !!! ");
			return -99;
		}
		break;
	// 20211013 ADD
	case PYM_CD_ETC :
		break;
	// 20211013 ~ADD
	default:
		TR_LOG_OUT("티머니고속_환불 - 결제수단 오류 환불 불가 error !!! ");
		return -99;
	}

	if(1)
	{	/// 
		/// (1) 출발일시
		pDate = (char *)&tRespInq.depr_dt;
		pTime = (char *)&tRespInq.depr_time;
		dwDeprTck = Util_TickFromString(pDate, pTime);
		TR_LOG_OUT("(02)_출발일시 - (%s %s), Tick(%ld)..", pDate, pTime, dwDeprTck);

		/// (2) 발행일시
		pDate = (char *)&tReqInq.tissu_dt;
		pTime = (char *)&tRespInq.tissu_time;
		dwPubTck = Util_TickFromString(pDate, pTime);
		TR_LOG_OUT("(03)_발행일시 - (%s %s), Tick(%ld)..", pDate, pTime, dwPubTck);

		nBaseMinute = 60;	/// 1시간

		if( dwDeprTck > dwCurrTck )
		{
			/// 당일발권 && 1시간 이내이면
			//if( (memcmp(szCurrDT, tReqTckNo.pub_dt, 8) == 0) && ((dwCurrTck - dwPubTck) < (60*60)) )

			/// 발권 1시간 이내이면
			if( (dwCurrTck - dwPubTck) < (60*60) )
			{
				tBase.n_commission_rate = 0; 
				TR_LOG_OUT("#1_발권 1시간 이내 0%% 적용 ..");
			}
			else
			{
				/// 출발시간이 1일 이전이면..
				if( (dwDeprTck - dwCurrTck) > (24 * 60 * 60) )
				{
					tBase.n_commission_rate = 0; 
					TR_LOG_OUT("#2_출발시간 1일 이내 0%% 적용..");
				}
				else if( (int)(dwDeprTck - dwCurrTck) > (int)(nBaseMinute * 60) )
				{	/// @@@@ 금호터미널과 다름...
					tBase.n_commission_rate = 5; 
					TR_LOG_OUT("#3_출발 1일 ~ 1시간 이전 5%% 적용 ..");
				}
				else
				{
					tBase.n_commission_rate = 10; 
					TR_LOG_OUT("#4_출발 전 1시간 이내 10%% 적용 ..");
				}
			}
		}
		else 
		{	/// 출발이후이면
			DWORD dwArvlDtm, dwArvlTick;

			pDate = (char *) &tRespInq.arvl_dtm[0];
			pTime = (char *) &tRespInq.arvl_dtm[8];
			dwArvlTick = Util_TickFromString(pDate, pTime);

			/// 도착시간 이내이면...
			if( dwCurrTck <= dwArvlTick )
			{
				tBase.n_commission_rate = 30; 
				TR_LOG_OUT("#5_출발이후 도착시간 이내 30%% 적용..");
			}
			else
			{
				TR_LOG_OUT("#6_ 도착시간 지남 (%s) ..", tRespInq.arvl_dtm);
				return -1;
			}
		}
	}

	tBase.n_tot_disc_rate = 100 - tBase.n_commission_rate;
	TR_LOG_OUT("(03)_수수료율 - (%d)..", tBase.n_commission_rate);

	/// 환불금액 계산
	/***
	{
		// 결제금액
		n_toal_fare = *(int *)tRespInq.ry_amt;

		TR_LOG_OUT("(04)_결제금액 - (%d)..", n_toal_fare);

		// 수수료 금액
		tBase.n_commission_fare = (int)((float)tBase.n_toal_fare * (float)((float)tBase.n_commission_rate / (float)100.0));
		TR_LOG_OUT("(05)_수수료 금액 - (%d)..", tBase.n_commission_fare);

		/// 지불수단이 카드가 아니고, 결제금액이 100원 미만이면..
		if( (tRespTckNo.pyn_mns_dvs_cd[0] != PYM_CD_CARD) && 
			(tRespTckNo.pyn_mns_dvs_cd[0] != PYM_CD_TPAY) && 
			(n_toal_fare % 100) > 0 ) // tpay 추가 kh
			//		if( (n_toal_fare % 100) > 0 )
		{
			TR_LOG_OUT("결제 금액이 100원미만 발생..");
			return -99;
		}

		if(n_commission_rate > 0)
		{
			n_commission_fare = n_commission_fare - (n_commission_fare % 100);
		}

		if(n_commission_fare < 0)
		{
			n_commission_fare = 0;
		}

		TR_LOG_OUT("(05)_수수료 금액 - (%d)..", n_commission_fare);
		TR_LOG_OUT("(06)_결제 금액 - (%d)..", n_toal_fare - n_commission_fare);
	}
	***/

	return 1;
}


