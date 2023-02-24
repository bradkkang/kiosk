// 
// 
// dev_tr_main.cpp : transaction
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
#include "dev_tr_mem.h"
#include "oper_kos_file.h"
#include "oper_config.h"
#include "md5.h"
#include "File_Env_ini.h"
#include "dev_tr_main.h"

/**
 * @brief		CPubTckMem
 * @details		[시외버스] 현장발권 데이타 메모리 정보 생성 Class
 */
CPubTckMem::CPubTckMem()
{

}

/**
 * @brief		~CPubTckMem
 * @details		[시외버스] 현장발권 데이타 메모리 정보 소멸 Class
 */
CPubTckMem::~CPubTckMem()
{

}

/**
 * @brief		Initialize
 * @details		[시외버스] 현장발권 데이타 메모리 변수 초기화
 * @param		None
 * @return		None
 */
void CPubTckMem::Initialize(void)
{
#if (_KTC_CERTIFY_ > 0)
	int i = 0;

	KTC_MemClear(&base			, sizeof(PUBTCK_T));
	KTC_MemClear(&card_resp		, sizeof(PUBTCK_CARD_T));
	KTC_MemClear(&cash_resp		, sizeof(PUBTCK_CASH_T));
	KTC_MemClear(&csrc_resp		, sizeof(PUBTCK_CSRC_T));
	KTC_MemClear(&csrc_card_resp, sizeof(PUBTCK_CSRC_CARD_T));
	
	/// RF선불 UI 요청 리스트
	m_vtRfUiData.clear();
	vector<PUBTCK_UI_RF_CARD_T>().swap(m_vtRfUiData);

	/// RF선불 UI 요청 리스트
	m_vtRfResp.clear();
	vector<rtk_pubtckppy_t>().swap(m_vtRfResp);

	n_fee_num =  0;

	// 배차리스트
	m_vtAlcnInfo.clear();
	vector<rtk_readalcn_list_t>().swap(m_vtAlcnInfo);

	// 배차요금조회
	m_vtFeeInfo.clear();
	vector<rtk_readalcnfee_list_t>().swap(m_vtFeeInfo);

	// 20211206 ADD~
	// 배차요금조회
	n_whch_sats_num = 0;
	m_vtWhchInfo.clear();
	vector<rtk_readalcnwhch_list_t>().swap(m_vtWhchInfo);
	// 20211206 ~ADD

	// 좌석정보
	m_vtUiSats.clear();
	vector<UI_SATS_T>().swap(m_vtUiSats);

	m_vtPcpysats.clear();
	vector<rtk_pcpysats_list_t>().swap(m_vtPcpysats);

	m_vtNPcpysats.clear();
	vector<rtk_pcpysats_list_t>().swap(m_vtNPcpysats);
	
	// 상주직원
	KTC_MemClear(&m_resp_readrsd_t, sizeof(rtk_readrsd_t));

	m_vtStaffModFareReq.clear();
	vector<UI_RESP_STAFF_CD_MOD_FARE_LIST_T>().swap(m_vtStaffModFareReq);

	// 신용카드 - 승차권 발권
	m_vtPbTckCard.clear();
	vector<rtk_ktcpbtckcard_list_t>().swap(m_vtPbTckCard);
		
	// 현금 - 승차권 발권
	m_vtPbTckCash.clear();
	vector<rtk_pubtckcash_list_t>().swap(m_vtPbTckCash);

	// 수기 현금영수증 - 승차권 발권
	m_vtPbTckCsrc.clear();
	vector<rtk_pubtckcsrc_list_t>().swap(m_vtPbTckCsrc);

	// 현금영수증카드 - 승차권 발권
	m_vtPbTckCsrcCard.clear();
	vector<rtk_ktcpbtckcsrc_list_t>().swap(m_vtPbTckCsrcCard);

	// 승차권 
	m_vtPrtTicket.clear();
	vector<TCK_PRINT_FMT_T>().swap(m_vtPrtTicket);
	
	// 테스트 승차권 
	m_vtPrtTicketTest.clear();							// 20211116 ADD
	vector<TCK_PRINT_FMT_T>().swap(m_vtPrtTicketTest);	// 20211116 ADD

	// 승차권 재발행
	m_vtPbTckReIssue.clear();
	vector<rtk_rpubtck_t>().swap(m_vtPbTckReIssue);

	TR_LOG_OUT("%s", "##### Memory release !!");


#else
	int i = 0;

	KTC_MemClear(&base			, sizeof(PUBTCK_T));
	KTC_MemClear(&card_resp		, sizeof(PUBTCK_CARD_T));
	KTC_MemClear(&cash_resp		, sizeof(PUBTCK_CASH_T));
	KTC_MemClear(&csrc_resp		, sizeof(PUBTCK_CSRC_T));
	KTC_MemClear(&csrc_card_resp, sizeof(PUBTCK_CSRC_CARD_T));

	/// RF선불 UI 요청 리스트
	m_vtRfUiData.clear();
	vector<PUBTCK_UI_RF_CARD_T>().swap(m_vtRfUiData);

	/// RF선불 UI 요청 리스트
	m_vtRfResp.clear();
	vector<rtk_pubtckppy_t>().swap(m_vtRfResp);

	n_fee_num =  0;

	// 배차리스트
	m_vtAlcnInfo.clear();
	vector<rtk_readalcn_list_t>().swap(m_vtAlcnInfo);

	// 배차요금조회
	m_vtFeeInfo.clear();
	vector<rtk_readalcnfee_list_t>().swap(m_vtFeeInfo);

	// 20211206 ADD~
	// 배차요금조회
	n_whch_sats_num = 0;
	m_vtWhchInfo.clear();
	vector<rtk_readalcnwhch_list_t>().swap(m_vtWhchInfo);
	// 20211206 ~ADD

	// 좌석정보
	m_vtUiSats.clear();
	vector<UI_SATS_T>().swap(m_vtUiSats);

	m_vtPcpysats.clear();
	vector<rtk_pcpysats_list_t>().swap(m_vtPcpysats);

	m_vtNPcpysats.clear();
	vector<rtk_pcpysats_list_t>().swap(m_vtNPcpysats);

	// 상주직원
	KTC_MemClear(&m_resp_readrsd_t, sizeof(rtk_readrsd_t));

	m_vtStaffModFareReq.clear();
	vector<UI_RESP_STAFF_CD_MOD_FARE_LIST_T>().swap(m_vtStaffModFareReq);

	// 신용카드 - 승차권 발권
	m_vtPbTckCard.clear();
	vector<rtk_ktcpbtckcard_list_t>().swap(m_vtPbTckCard);

	// 현금 - 승차권 발권
	m_vtPbTckCash.clear();
	vector<rtk_pubtckcash_list_t>().swap(m_vtPbTckCash);

	// 수기 현금영수증 - 승차권 발권
	m_vtPbTckCsrc.clear();
	vector<rtk_pubtckcsrc_list_t>().swap(m_vtPbTckCsrc);

	// 현금영수증카드 - 승차권 발권
	m_vtPbTckCsrcCard.clear();
	vector<rtk_ktcpbtckcsrc_list_t>().swap(m_vtPbTckCsrcCard);

	// 승차권 
	m_vtPrtTicket.clear();
	vector<TCK_PRINT_FMT_T>().swap(m_vtPrtTicket);

	// 승차권 재발행
	m_vtPbTckReIssue.clear();
	vector<rtk_rpubtck_t>().swap(m_vtPbTckReIssue);

	TR_LOG_OUT("%s", "##### Memory release !!");
#endif
}		  

/**
 * @brief		MakeAlcnListPacket
 * @details		[시외버스] 배차 조회 데이타 패킷 만들기
 * @param		char *pSend		UI 전송용 버퍼
 * @return		배차 갯수
 */
int CPubTckMem::MakeAlcnListPacket(char *pSend)
{
	int nOffset = 0, nCount = 0;
	vector<rtk_readalcn_list_t>::iterator	iter;

	nCount = m_vtAlcnInfo.size();

	/// ACK
	pSend[nOffset++] = CHAR_ACK;
	/// 배차수
	::CopyMemory(&pSend[nOffset], &nCount, 2);
	nOffset += 2;

	for( iter = m_vtAlcnInfo.begin(); iter != m_vtAlcnInfo.end(); iter++ )
	{
		///< 노선ID
		::CopyMemory(&pSend[nOffset], iter->rot_id, sizeof(iter->rot_id) - 1);
		nOffset += (sizeof(iter->rot_id) - 1);

		///< 노선순번(number)
		::CopyMemory(&pSend[nOffset], iter->rot_sqno, sizeof(int));
		nOffset += sizeof(int);

		///< 배차일자
		::CopyMemory(&pSend[nOffset], iter->alcn_dt, sizeof(iter->alcn_dt) - 1);
		nOffset += sizeof(iter->alcn_dt) - 1;

		///< 배차순번(number)
		::CopyMemory(&pSend[nOffset], iter->alcn_sqno, sizeof(int));
		nOffset += sizeof(int);

		///< 실제출발일자
		::CopyMemory(&pSend[nOffset], iter->atl_depr_dt, sizeof(iter->atl_depr_dt) - 1);
		nOffset += sizeof(iter->atl_depr_dt) - 1;

		///< 실제출발시각
		::CopyMemory(&pSend[nOffset], iter->atl_depr_time, sizeof(iter->atl_depr_time) - 1);
		nOffset += sizeof(iter->atl_depr_time) - 1;

		///< 출발시각
		::CopyMemory(&pSend[nOffset], iter->depr_time, sizeof(iter->depr_time) - 1);
		nOffset += sizeof(iter->depr_time) - 1;

		///< 노선명
		::CopyMemory(&pSend[nOffset], iter->rot_nm, sizeof(iter->rot_nm) - 1);
		nOffset += sizeof(iter->rot_nm) - 1;

		///< 노선종류코드
		::CopyMemory(&pSend[nOffset], iter->rot_bus_dvs_cd, sizeof(iter->rot_bus_dvs_cd) - 1);
		nOffset += sizeof(iter->rot_bus_dvs_cd) - 1;

		///< 버스등급코드
		::CopyMemory(&pSend[nOffset], iter->bus_cls_cd, sizeof(iter->bus_cls_cd) - 1);
		nOffset += sizeof(iter->bus_cls_cd) - 1;

		///< 버스운수사코드
		::CopyMemory(&pSend[nOffset], iter->bus_cacm_cd, sizeof(iter->bus_cacm_cd) - 1);
		nOffset += sizeof(iter->bus_cacm_cd) - 1;

		///< 배차방식구분코드
		::CopyMemory(&pSend[nOffset], iter->alcn_way_dvs_cd, sizeof(iter->alcn_way_dvs_cd) - 1);
		nOffset += sizeof(iter->alcn_way_dvs_cd) - 1;

		///< 좌석제사용여부
		::CopyMemory(&pSend[nOffset], iter->sati_use_yn, sizeof(iter->sati_use_yn) - 1);
		nOffset += sizeof(iter->sati_use_yn) - 1;

		///< 정기임시여부
		::CopyMemory(&pSend[nOffset], iter->perd_temp_yn, sizeof(iter->perd_temp_yn) - 1);
		nOffset += sizeof(iter->perd_temp_yn) - 1;

		///< 결행여부
		::CopyMemory(&pSend[nOffset], iter->bcnl_yn, sizeof(iter->bcnl_yn) - 1);
		nOffset += sizeof(iter->bcnl_yn) - 1;

		///< 거리(number)
		::CopyMemory(&pSend[nOffset], iter->dist, sizeof(int));
		nOffset += sizeof(int);

		///< 소요시간(number)
		::CopyMemory(&pSend[nOffset], iter->take_drtm, sizeof(int));
		nOffset += sizeof(int);

		///< 승차홈다중값
		::CopyMemory(&pSend[nOffset], iter->rdhm_mltp_val, sizeof(iter->rdhm_mltp_val) - 1);
		nOffset += (sizeof(iter->rdhm_mltp_val) - 1);

		///< 좌석 수(number)
		::CopyMemory(&pSend[nOffset], iter->sats_num, sizeof(WORD));
		nOffset += sizeof(WORD);

		///< 잔여좌석수(number)
		::CopyMemory(&pSend[nOffset], iter->rmn_scnt, sizeof(WORD));
		nOffset += sizeof(WORD);

		///< 무표수(number)
		::CopyMemory(&pSend[nOffset], iter->ocnt, sizeof(WORD));
		nOffset += sizeof(WORD);

		///< 터미널발권가능여부
		::CopyMemory(&pSend[nOffset], iter->trml_tisu_psb_yn, sizeof(iter->trml_tisu_psb_yn) - 1);
		nOffset += (sizeof(iter->trml_tisu_psb_yn) - 1);

		///< 시외버스운행형태코드
		::CopyMemory(&pSend[nOffset], iter->cty_bus_oprn_shp_cd, sizeof(iter->cty_bus_oprn_shp_cd) - 1);
		nOffset += (sizeof(iter->cty_bus_oprn_shp_cd) - 1);

		///< 할인가능유무
		::CopyMemory(&pSend[nOffset], iter->dc_psb_yn, sizeof(iter->dc_psb_yn) - 1);
		nOffset += (sizeof(iter->dc_psb_yn) - 1);

		// 20211206 ADD~
		///< 휠체어발권여부
		::CopyMemory(&pSend[nOffset], iter->whch_tissu_yn, sizeof(iter->whch_tissu_yn) - 1);
		nOffset += (sizeof(iter->whch_tissu_yn) - 1);
		// 20211206 ~ADD
	}
//	return m_vtAlcnInfo.size();
	return nOffset;
}

/**
 * @brief		FindBusDcKind
 * @details		[시외버스] 할인율 구분코드 찾기
 * @param		char *bus_tck_knd_cd	티켓 종류 코드
 * @param		char *cty_bus_dc_knd_cd	시외버스 할인구분코드
 * @param		char *ret_dcrt_dvs_cd	할인율 구분코드 
 * @return		성공 >= 배열인덱스, 실패 < 0
 */
int CPubTckMem::FindBusDcKind(char *bus_tck_knd_cd, char *cty_bus_dc_knd_cd, char *ret_dcrt_dvs_cd)
{
	int i;

	vector<rtk_readalcnfee_list_t>::iterator iter;

	i = 0;
	for( iter = m_vtFeeInfo.begin(); iter != m_vtFeeInfo.end(); iter++ )
	{
		// 버스티켓 종류
		if( memcmp(bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(iter->bus_tck_knd_cd) - 1) == 0 )
		{
			// 시외버스할인종류코드
			if(cty_bus_dc_knd_cd[0] == iter->cty_bus_dc_knd_cd[0])
			{
				// 할인율 구분코드
				ret_dcrt_dvs_cd[0] = iter->dcrt_dvs_cd[0];
				return i;
			}
		}
	}

	return -1;
}

/**
 * @brief		MakePasswdChars
 * @details		[시외버스] 현장발권 - 구매자정보 문자 만들기
 * @param		char *retBuf		암호화 문자 버퍼
 * @param		char *pSrc			원본 데이타
 * @return		None
 */
void CPubTckMem::MakePasswdChars(char *retBuf, char *pSrc)
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
		// 2020.03.30 modify
		// 		if(nLen > 9)
		// 		{
		// 			sprintf(retBuf, "%.*s***%.*s", 3, &pSrc[0], 4, &pSrc[nLen - 4]);
		// 		}
		// 		else
		// 		{
		// 			sprintf(retBuf, "%.*s******", 3, &pSrc[0]);
		// 		}

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
 * @brief		Make_IIAC_RsdData
 * @details		[시외버스] 상주직원 데이타 만들기
 * @param		char *pData		프린트 데이타 포맷
 * @return		None
 */
void CPubTckMem::Make_IIAC_RsdData(char *pData)
{
	int nLen;
	char *pBuff;
	PTCK_PRINT_FMT_T pPrtData;

	pPrtData = (PTCK_PRINT_FMT_T) pData;

	/// 상주직원 이름
	{
		nLen = CheckBuffer(pBuff = base.rsd_nm, sizeof(pPrtData->rsd_nm) - 1);
		if(nLen > 0)
		{
			CopyMemory(pPrtData->rsd_nm, pBuff, nLen);
		}
	}

	/// 상주직원 소속명
	{
		nLen = CheckBuffer(pBuff = base.rsd_cmpy_nm, sizeof(pPrtData->rsd_cmpy_nm) - 1);
		if(nLen > 0)
		{
			CopyMemory(pPrtData->rsd_cmpy_nm, pBuff, nLen);
		}
	}

	/// 상주직원 연락처
	{
		nLen = CheckBuffer(pBuff = base.rsd_tel_nm, sizeof(pPrtData->rsd_tel_nm) - 1);
		if(nLen > 0)
		{
			sprintf(pPrtData->rsd_tel_nm, "%.*s****", nLen - 4, pBuff);
		}
	}
}

/**
 * @brief		MakeCashTicketPrtData
 * @details		[시외버스] 현장발권 - 현금 자진발급 결제, 승차권 데이타 만들기
 * @param		None
 * @return		항상 = 0
 */
int CPubTckMem::MakeCashTicketPrtData(void)
{
	int nRet = 0, nLen = 0;
	int nSatsNo = 0;
	int nRot = -1;
	char *pBuff;
	POPER_FILE_CONFIG_T pConfig;

	vector<rtk_pubtckcash_list_t>::iterator iter;

	TR_LOG_OUT(" [시외_현금] 승차권 프린트 데이타 작성 !!!");

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	m_vtPrtTicket.clear();
	for(iter = m_vtPbTckCash.begin(); iter != m_vtPbTckCash.end(); iter++)
	{
		TCK_PRINT_FMT_T tPrtInfo;

		::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));

		/// 경유지 정보
		CConfigTkMem::GetInstance()->SearchThruInfo(base.rot_id, tPrtInfo.thru_nm);

		/// 배차방식
		tPrtInfo.alcn_way_dvs_cd[0] = base.alcn_way_dvs_cd[0];

		/// 좌석제 사용유무
		tPrtInfo.sati_use_yn[0] = base.sati_use_yn[0];

		/// 좌석번호출력여부		
		tPrtInfo.sats_no_prin_yn[0] = cash_resp.sats_no_prin_yn[0];

		/// 출발시간 출력여부		
		tPrtInfo.depr_time_prin_yn[0] = cash_resp.depr_time_prin_yn[0];

		TR_LOG_OUT("배차[%c], 좌석제[%c] ..", tPrtInfo.alcn_way_dvs_cd[0], tPrtInfo.sati_use_yn[0]);

		/// (01). 출발지 터미널명 (한글)
		{
			//sprintf(tPrtInfo.depr_trml_ko_nm, "%s", base.depr_trml_nm);		
			nLen = CheckBuffer(pBuff = base.depr_trml_nm, sizeof(tPrtInfo.depr_trml_ko_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_ko_nm, pBuff, nLen);
			}
		}
		/// (02). 출발지 터미널명 (영문)
		{
			//sprintf(tPrtInfo.depr_trml_eng_nm, "%s", base.depr_trml_eng_nm);	
			nLen = CheckBuffer(pBuff = base.depr_trml_eng_nm, sizeof(tPrtInfo.depr_trml_eng_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_eng_nm, pBuff, nLen);
			}
		}

		/// (03). 출발지 터미널 전화번호
		{
			nLen = CheckBuffer(pBuff = pConfig->ccTrmlInfo_t.sz_prn_trml_tel_no, sizeof(tPrtInfo.depr_trml_tel_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_tel_nm, pBuff, nLen);
			}
		}

		/// (04). 도착지 터미널명 (한글)
		{
			//sprintf(tPrtInfo.arvl_trml_ko_nm, "%s", base.arvl_trml_nm);		
			nLen = CheckBuffer(pBuff = base.arvl_trml_nm, sizeof(tPrtInfo.arvl_trml_ko_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.arvl_trml_ko_nm, pBuff, nLen);
			}
		}
		/// (05). 도착지 터미널명 (영문)
		{
			//sprintf(tPrtInfo.arvl_trml_eng_nm, "%s", base.arvl_trml_eng_nm);	
			nLen = CheckBuffer(pBuff = base.arvl_trml_eng_nm, sizeof(tPrtInfo.arvl_trml_eng_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.arvl_trml_eng_nm, pBuff, nLen);
			}
		}

		/// (06). 노선번호 : 인천공항
		{
			Find_CCS_RotName(base.rot_id, tPrtInfo.arvl_trml_rot_num);
		}

		/// (06). 버스운수사 명
		nRet = FindBusCacmName(SVR_DVS_CCBUS, cash_resp.bus_cacm_cd, tPrtInfo.bus_cacm_nm, "", tPrtInfo.bus_cacm_tel_nm);	

		/// (06). 버스등급명
		nRet = FindBusClsName(SVR_DVS_CCBUS, cash_resp.bus_cls_cd, tPrtInfo.bus_cls_nm);		

		/// (06-1). 짧은버스등급명
		nRet = FindBusClsShctName(SVR_DVS_CCBUS, cash_resp.bus_cls_cd, tPrtInfo.bus_cls_shct_nm);	

		/// (07). 버스티켓종류명
		nRet = FindBusTckKndName(SVR_DVS_CCBUS, iter->bus_tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 
		::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(iter->bus_tck_knd_cd));
		
		/// (08). 현금영수증 승인번호
		sprintf(tPrtInfo.card_aprv_no, "%s", iter->csrc_aprv_no);

		/// (09). 현금 승인금액, 요금
		{
			int nValue = 0;
			char szFare[100];

			/// 승인금액
			nValue = base.nTotalMoney;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.card_aprv_amt, "%s", szFare);	

			/// 발권금액
			nValue = *(int *) iter->tisu_amt;
			tPrtInfo.n_tisu_amt = nValue; 
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.tisu_amt, "%s", szFare);	

			// 시외버스할인종류 
			TR_LOG_OUT("시외버스 할인율 정보 = [%c] ..", iter->cty_bus_dc_knd_cd[0]);
			if( iter->cty_bus_dc_knd_cd[0] != 'Z')
			{	/// 'Z' : 할인율 없음
				// 할인율 문자열
				nRet = ConvertDCRT(iter->dcrt_dvs_cd, tPrtInfo.dcrt_dvs_str);
				// 시외버스할인종류 문자열
				nRet = ConvertDCKnd(iter->cty_bus_dc_knd_cd, tPrtInfo.cty_bus_dc_knd_str);

				TR_LOG_OUT("시외버스 할인율 정보, 할인값(%s), 할인종류(%s) ..", tPrtInfo.dcrt_dvs_str, tPrtInfo.cty_bus_dc_knd_str);
			}
		}
		/// (10). 카드번호
		//sprintf(tPrtInfo.card_no, "%s", card_resp.card_no);

		/// (11). 결제수단 (현금/신용카드 등)
		tPrtInfo.n_pym_dvs = PYM_CD_CASH;
		//sprintf(tPrtInfo.pym_dvs, "현금(자진발급)");	
		sprintf(tPrtInfo.pym_dvs, "자진발급");			// 20211014, 승차권 출력문구 변경
		sprintf(tPrtInfo.shrt_pym_dvs, "현금");	
		
		/// (14). 좌석번호
		nSatsNo = *(int *)iter->sats_no;
		sprintf(tPrtInfo.sats_no, "%d", *(int *)iter->sats_no);	

		/// (12). 출발 일자, 출발시간
		MakeDepartureDateTime(cash_resp.atl_depr_dt, cash_resp.atl_depr_time, tPrtInfo.atl_depr_dt, tPrtInfo.atl_depr_time);

		/// (15). 승차홈
		sprintf(tPrtInfo.rdhm_val, "%s", cash_resp.rdhm_mltp_val);	

		/// (16). 바코드 관련 : 발행일자(8)-단축터미널코드(4)-창구번호(2)-발행일련번호(4)-securityCode(2)
		{
			char szTicket[100];
			SYSTEMTIME st;

			GetLocalTime(&st);

			/// 발행일자
			sprintf(tPrtInfo.bar_pub_dt, "%s", cash_resp.pub_dt);	
			/// 발행단축터미널코드
			sprintf(tPrtInfo.bar_pub_shct_trml_cd, "%s", cash_resp.pub_shct_trml_cd);	
			/// 발행창구번호
			sprintf(tPrtInfo.bar_pub_wnd_no, "%s", cash_resp.pub_wnd_no);	
			/// 발행일련번호
			sprintf(tPrtInfo.bar_pub_sno, "%04d", *(int *)iter->pub_sno);

			sprintf(szTicket, "%s%s%s%s", tPrtInfo.bar_pub_dt, tPrtInfo.bar_pub_shct_trml_cd, tPrtInfo.bar_pub_wnd_no, tPrtInfo.bar_pub_sno);
			GetTicketSecurityCode((BYTE *)szTicket, strlen(szTicket), tPrtInfo.bar_secu_code);

			/// 발권시간
			sprintf(tPrtInfo.pub_time, "(%02d:%02d:%02d)", st.wHour, st.wMinute, st.wSecond);
			sprintf(tPrtInfo.shrt_pub_time, "(%02d:%02d)", st.wHour, st.wMinute);

			TR_LOG_OUT("바코드 체크섬 값 = [%s] \n", tPrtInfo.bar_secu_code);
		}

		/// (17). 항목 출력여부
		{
			///< 버스등급 출력여부		
			tPrtInfo.bus_cls_prin_yn[0]		= cash_resp.bus_cls_prin_yn[0];
			///< 버스운수사명 출력여부	
			tPrtInfo.bus_cacm_nm_prin_yn[0] = cash_resp.bus_cacm_nm_prin_yn[0];
			///< 출발시각 출력여부
			//tPrtInfo.depr_time_prin_yn[0]	= cash_resp.depr_time_prin_yn[0];
			///< 좌석번호 출력여부
			//tPrtInfo.sats_no_prin_yn[0]		= cash_resp.sats_no_prin_yn[0];
		}

		// 20211008 ADD - 승차권(승객용)에 QR코드 추가
		/// (20). QR 바코드 관련 - 현장발권 자진발급(현금)
		{
			/// 1) 발행일자
			sprintf(tPrtInfo.qr_pub_dt, "%s", cash_resp.pub_dt);	
			TR_LOG_OUT("qr발행일자 = %s", tPrtInfo.qr_pub_dt);
		}
		// 20211008 ~ADD - 승차권(승객용)에 QR코드 추가

		/// 사용자 이름
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->user_nm, sizeof(tPrtInfo.user_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.user_nm, pBuff, nLen);
			}
		}

		/// 인천공항 - 상주직원 데이타 만들기
		Make_IIAC_RsdData((char *)&tPrtInfo);

		m_vtPrtTicket.push_back(tPrtInfo);

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
#endif
	}

	return 0;
}

/**
 * @brief		MakeCsrcTicketPrtData
 * @details		[시외버스] 현장발권 현금영수증 결제, 승차권 데이타 만들기
 * @param		None
 * @return		항상 = 0
 */
int CPubTckMem::MakeCsrcTicketPrtData(void)
{
	int nRet = 0, nLen = 0;
	char *pBuff;
	POPER_FILE_CONFIG_T pConfig;

	vector<rtk_pubtckcsrc_list_t>::iterator iter;

	TR_LOG_OUT(" [시외_현금영수증] 승차권 프린트 데이타 작성 !!!");

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	m_vtPrtTicket.clear();
	for(iter = m_vtPbTckCsrc.begin(); iter != m_vtPbTckCsrc.end(); iter++)
	{
		TCK_PRINT_FMT_T tPrtInfo;

		::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));

		/// 개인/법인 구분코드
		tPrtInfo.ui_csrc_use[0] = base.ui_csrc_use[0];

		/// 경유지 정보
		CConfigTkMem::GetInstance()->SearchThruInfo(base.rot_id, tPrtInfo.thru_nm);

		/// 배차방식
		tPrtInfo.alcn_way_dvs_cd[0] = csrc_resp.alcn_way_dvs_cd[0];

		/// 좌석제 사용유무
		tPrtInfo.sati_use_yn[0] = base.sati_use_yn[0];
		//tPrtInfo.sati_use_yn[0] = csrc_resp.sats_no_prin_yn[0];

		/// 좌석번호출력여부		
		tPrtInfo.sats_no_prin_yn[0] = csrc_resp.sats_no_prin_yn[0];

		/// 출발시간 출력여부		
		tPrtInfo.depr_time_prin_yn[0] = csrc_resp.depr_time_prin_yn[0];

		/// (01). 출발지 터미널명 (한글)
		{
			//sprintf(tPrtInfo.depr_trml_ko_nm, "%s", base.depr_trml_nm);		
			nLen = CheckBuffer(pBuff = base.depr_trml_nm, sizeof(tPrtInfo.depr_trml_ko_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_ko_nm, pBuff, nLen);
			}
		}
		/// (02). 출발지 터미널명 (영문)
		{
			//sprintf(tPrtInfo.depr_trml_eng_nm, "%s", base.depr_trml_eng_nm);	
			nLen = CheckBuffer(pBuff = base.depr_trml_eng_nm, sizeof(tPrtInfo.depr_trml_eng_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_eng_nm, pBuff, nLen);
			}
		}

		/// (03). 출발지 터미널 전화번호
		{
			nLen = CheckBuffer(pBuff = pConfig->ccTrmlInfo_t.sz_prn_trml_tel_no, sizeof(tPrtInfo.depr_trml_tel_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_tel_nm, pBuff, nLen);
			}
		}

		/// (04). 도착지 터미널명 (한글)
		{
			//sprintf(tPrtInfo.arvl_trml_ko_nm, "%s", base.arvl_trml_nm);		
			nLen = CheckBuffer(pBuff = base.arvl_trml_nm, sizeof(tPrtInfo.arvl_trml_ko_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.arvl_trml_ko_nm, pBuff, nLen);
			}
		}
		/// (05). 도착지 터미널명 (영문)
		{
			//sprintf(tPrtInfo.arvl_trml_eng_nm, "%s", base.arvl_trml_eng_nm);	
			nLen = CheckBuffer(pBuff = base.arvl_trml_eng_nm, sizeof(tPrtInfo.arvl_trml_eng_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.arvl_trml_eng_nm, pBuff, nLen);
			}
		}

		/// (06). 노선번호 : 인천공항
		{
			Find_CCS_RotName(base.rot_id, tPrtInfo.arvl_trml_rot_num);
		}

		/// (05). 버스운수사 명
		nRet = FindBusCacmName(SVR_DVS_CCBUS, csrc_resp.bus_cacm_cd, tPrtInfo.bus_cacm_nm, "", tPrtInfo.bus_cacm_tel_nm);	
		/// (06). 버스등급명
		nRet = FindBusClsName(SVR_DVS_CCBUS, csrc_resp.bus_cls_cd, tPrtInfo.bus_cls_nm);		
		/// (07). 버스티켓종류명
		nRet = FindBusTckKndName(SVR_DVS_CCBUS, iter->bus_tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 
		::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(iter->bus_tck_knd_cd));

		/// (08). 현금영수증 승인번호
		sprintf(tPrtInfo.card_aprv_no, "%s", iter->csrc_aprv_no);

		/// (09). 승인금액, 요금
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
			nValue = *(int *) iter->tisu_amt;
			tPrtInfo.n_tisu_amt = nValue; 
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.tisu_amt, "%s", szFare);	

			// 시외버스할인종류 
			TR_LOG_OUT("시외버스 할인율 정보 = [%c] ..", iter->cty_bus_dc_knd_cd[0]);
			if( iter->cty_bus_dc_knd_cd[0] != 'Z')
			{	/// 'Z' : 할인율 없음
				// 할인율 문자열
				nRet = ConvertDCRT(iter->dcrt_dvs_cd, tPrtInfo.dcrt_dvs_str);
				// 시외버스할인종류 문자열
				nRet = ConvertDCKnd(iter->cty_bus_dc_knd_cd, tPrtInfo.cty_bus_dc_knd_str);

				TR_LOG_OUT("시외버스 할인율 정보, 할인값(%s), 할인종류(%s) ..", tPrtInfo.dcrt_dvs_str, tPrtInfo.cty_bus_dc_knd_str);
			}
		}
		/// (10). 카드번호
		//sprintf(tPrtInfo.card_no, "%s", csrc_resp.card_no);

		/// (11). 결제수단 (현금/신용카드 등)
		tPrtInfo.n_pym_dvs = PYM_CD_CSRC;

		/// 현금영수증 구분코드 - 0:미사용, 1:수기입력, 2:신용카드, 3:현영전용카드
		if( base.ui_csrc_dvs_cd[0] == 1 ) 
		{	/// 수기입력
			if( base.ui_csrc_use[0] == 0 )
			{	/// 개인
				//sprintf(tPrtInfo.pym_dvs, "현금(소득공제)");
				sprintf(tPrtInfo.pym_dvs, "소득공제");			// 20211014, 승차권 출력문구 변경
			}
			else
			{	/// 법인
				//sprintf(tPrtInfo.pym_dvs, "현금(지출증빙)");
				sprintf(tPrtInfo.pym_dvs, "지출증빙");			// 20211014, 승차권 출력문구 변경
			}
		}
		else
		{	/// 현금영수증 카드
			//sprintf(tPrtInfo.pym_dvs, "현금(현금영수증)");	
			if( base.ui_csrc_use[0] == 0 )
			{	/// 개인
				//sprintf(tPrtInfo.pym_dvs, "현금(소득공제)");
				sprintf(tPrtInfo.pym_dvs, "소득공제");			// 20211014, 승차권 출력문구 변경
			}
			else
			{	/// 법인
				//sprintf(tPrtInfo.pym_dvs, "현금(지출증빙)");
				sprintf(tPrtInfo.pym_dvs, "지출증빙");			// 20211014, 승차권 출력문구 변경
			}
		}
		sprintf(tPrtInfo.shrt_pym_dvs, "현영");	

		/// 구매자 정보
		#if 1	// 20211020 수기입력오류 response 대응
		if (iter->pyn_mns_dvs_cd[0] == PYM_CD_CSRC)
		{
			MakePasswdChars(tPrtInfo.card_no, csrc_resp.csrc_auth_no);
		}
		else	// 수기입력오류 발생 시 통전망에서 자진발급('A')으로 내려준다.
		{
			sprintf(tPrtInfo.pym_dvs, "자진발급");				// 20211014, 승차권 출력문구 변경
		//	MakePasswdChars(tPrtInfo.card_no, csrc_resp.csrc_auth_no);
		}
		#else
		MakePasswdChars(tPrtInfo.card_no, csrc_resp.csrc_auth_no);
		#endif

		/// (12). 출발 일자, 출발시간
		MakeDepartureDateTime(csrc_resp.atl_depr_dt, csrc_resp.atl_depr_time, tPrtInfo.atl_depr_dt, tPrtInfo.atl_depr_time);

		/// (14). 좌석번호
		sprintf(tPrtInfo.sats_no, "%d", *(int *)iter->sats_no);	
		/// (15). 승차홈
		sprintf(tPrtInfo.rdhm_val, "%s", csrc_resp.rdhm_mltp_val);	

		/// (16). 바코드 관련
		{
			char szTicket[100];
			SYSTEMTIME st;

			GetLocalTime(&st);

			/// 발행일자
			sprintf(tPrtInfo.bar_pub_dt, "%s", csrc_resp.pub_dt);	
			/// 발행단축터미널코드
			sprintf(tPrtInfo.bar_pub_shct_trml_cd, "%s", csrc_resp.pub_shct_trml_cd);	
			/// 발행창구번호
			sprintf(tPrtInfo.bar_pub_wnd_no, "%s", csrc_resp.pub_wnd_no);	
			/// 발행일련번호
			sprintf(tPrtInfo.bar_pub_sno, "%04d", *(int *)iter->pub_sno);

			sprintf(szTicket, "%s%s%s%s", tPrtInfo.bar_pub_dt, tPrtInfo.bar_pub_shct_trml_cd, tPrtInfo.bar_pub_wnd_no, tPrtInfo.bar_pub_sno);
			GetTicketSecurityCode((BYTE *)szTicket, strlen(szTicket), tPrtInfo.bar_secu_code);
			TR_LOG_OUT("바코드 체크섬 값 = [%s] \n", tPrtInfo.bar_secu_code);

			/// 발권시간
			sprintf(tPrtInfo.pub_time, "(%02d:%02d:%02d)", st.wHour, st.wMinute, st.wSecond);
			sprintf(tPrtInfo.shrt_pub_time, "(%02d:%02d)", st.wHour, st.wMinute);//kh200709
		}

		/// (17). 항목 출력여부
		{
			///< 버스등급 출력여부		
			tPrtInfo.bus_cls_prin_yn[0]		= csrc_resp.bus_cls_prin_yn[0];
			///< 버스운수사명 출력여부	
			tPrtInfo.bus_cacm_nm_prin_yn[0] = csrc_resp.bus_cacm_nm_prin_yn[0];
			///< 출발시각 출력여부
			//tPrtInfo.depr_time_prin_yn[0]	= card_resp.depr_time_prin_yn[0];
			///< 좌석번호 출력여부
			//tPrtInfo.sats_no_prin_yn[0]		= card_resp.sats_no_prin_yn[0];
		}

		// 20211008 ADD - 승차권(승객용)에 QR코드 추가
		/// (20). QR 바코드 관련 - 현장발권 현금영수증
		{
			/// 1) 발행일자
			sprintf(tPrtInfo.qr_pub_dt, "%s", csrc_resp.pub_dt);	
			TR_LOG_OUT("qr발행일자 = %s", tPrtInfo.qr_pub_dt);
		}
		// 20211008 ~ADD - 승차권(승객용)에 QR코드 추가

		/// 사용자 이름
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->user_nm, sizeof(tPrtInfo.user_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.user_nm, pBuff, nLen);
			}
		}

		/// 인천공항 - 상주직원 데이타 만들기
		Make_IIAC_RsdData((char *)&tPrtInfo);

		m_vtPrtTicket.push_back(tPrtInfo);

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
#endif
	}

	return 0;
}

/**
 * @brief		MakeCsrcTicketPrtData
 * @details		[시외버스] 현장발권 현금영수증 카드 결제, 승차권 데이타 만들기
 * @param		None
 * @return		항상 = 0
 */
int CPubTckMem::MakeCsrcCardTicketPrtData(void)
{
	int nRet = 0, nLen = 0;
	char *pBuff;
	POPER_FILE_CONFIG_T pConfig;

	vector<rtk_ktcpbtckcsrc_list_t>::iterator iter;

	TR_LOG_OUT(" [시외_현금영수증_카드] 승차권 프린트 데이타 작성 !!!");

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	m_vtPrtTicket.clear();
	for(iter = m_vtPbTckCsrcCard.begin(); iter != m_vtPbTckCsrcCard.end(); iter++)
	{
		TCK_PRINT_FMT_T tPrtInfo;

		::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));

		/// 경유지 정보
		CConfigTkMem::GetInstance()->SearchThruInfo(base.rot_id, tPrtInfo.thru_nm);

		/// 개인/법인 구분코드
		tPrtInfo.ui_csrc_use[0] = base.ui_csrc_use[0];

		/// 배차방식
		tPrtInfo.alcn_way_dvs_cd[0] = csrc_card_resp.alcn_way_dvs_cd[0];

		/// 좌석제 사용유무
		tPrtInfo.sati_use_yn[0] = csrc_card_resp.sati_use_yn[0];

		/// 좌석번호출력여부		
		tPrtInfo.sats_no_prin_yn[0] = csrc_card_resp.sats_no_prin_yn[0];

		/// 출발시간 출력여부		
		tPrtInfo.depr_time_prin_yn[0] = csrc_card_resp.depr_time_prin_yn[0];

		/// (01). 출발지 터미널명 (한글)
		{
			//sprintf(tPrtInfo.depr_trml_ko_nm, "%s", base.depr_trml_nm);		
			nLen = CheckBuffer(pBuff = base.depr_trml_nm, sizeof(tPrtInfo.depr_trml_ko_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_ko_nm, pBuff, nLen);
			}
		}
		/// (02). 출발지 터미널명 (영문)
		{
			//sprintf(tPrtInfo.depr_trml_eng_nm, "%s", base.depr_trml_eng_nm);	
			nLen = CheckBuffer(pBuff = base.depr_trml_eng_nm, sizeof(tPrtInfo.depr_trml_eng_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_eng_nm, pBuff, nLen);
			}
		}

		/// (03). 출발지 터미널 전화번호
		{
			nLen = CheckBuffer(pBuff = pConfig->ccTrmlInfo_t.sz_prn_trml_tel_no, sizeof(tPrtInfo.depr_trml_tel_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_tel_nm, pBuff, nLen);
			}
		}

		/// (04). 도착지 터미널명 (한글)
		{
			//sprintf(tPrtInfo.arvl_trml_ko_nm, "%s", base.arvl_trml_nm);		
			nLen = CheckBuffer(pBuff = base.arvl_trml_nm, sizeof(tPrtInfo.arvl_trml_ko_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.arvl_trml_ko_nm, pBuff, nLen);
			}
		}
		/// (05). 도착지 터미널명 (영문)
		{
			//sprintf(tPrtInfo.arvl_trml_eng_nm, "%s", base.arvl_trml_eng_nm);	
			nLen = CheckBuffer(pBuff = base.arvl_trml_eng_nm, sizeof(tPrtInfo.arvl_trml_eng_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.arvl_trml_eng_nm, pBuff, nLen);
			}
		}

		/// (06). 노선번호 : 인천공항
		{
			Find_CCS_RotName(base.rot_id, tPrtInfo.arvl_trml_rot_num);
		}

		/// (05). 버스운수사 명
		nRet = FindBusCacmName(SVR_DVS_CCBUS, csrc_card_resp.bus_cacm_cd, tPrtInfo.bus_cacm_nm, "", tPrtInfo.bus_cacm_tel_nm);	
		/// (06). 버스등급명
		nRet = FindBusClsName(SVR_DVS_CCBUS, csrc_card_resp.bus_cls_cd, tPrtInfo.bus_cls_nm);
		/// (06-1). 짧은버스등급명
		nRet = FindBusClsShctName(SVR_DVS_CCBUS, csrc_card_resp.bus_cls_cd, tPrtInfo.bus_cls_shct_nm);
		/// (07). 버스티켓종류명
		nRet = FindBusTckKndName(SVR_DVS_CCBUS, iter->bus_tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 
		::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(iter->bus_tck_knd_cd));
		/// (08). 현금영수증 승인번호
		sprintf(tPrtInfo.card_aprv_no, "%s", iter->csrc_aprv_no);

		/// (09). 승인금액, 요금
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
			nValue = *(int *) iter->tisu_amt;
			tPrtInfo.n_tisu_amt = nValue; 
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.tisu_amt, "%s", szFare);	

			// 시외버스할인종류 
			TR_LOG_OUT("시외버스 할인율 정보 = [%c] ..", iter->cty_bus_dc_knd_cd[0]);
			if( iter->cty_bus_dc_knd_cd[0] != 'Z')
			{	/// 'Z' : 할인율 없음
				// 할인율 문자열
				nRet = ConvertDCRT(iter->dcrt_dvs_cd, tPrtInfo.dcrt_dvs_str);
				// 시외버스할인종류 문자열
				nRet = ConvertDCKnd(iter->cty_bus_dc_knd_cd, tPrtInfo.cty_bus_dc_knd_str);

				TR_LOG_OUT("시외버스 할인율 정보, 할인값(%s), 할인종류(%s) ..", tPrtInfo.dcrt_dvs_str, tPrtInfo.cty_bus_dc_knd_str);
			}
		}
		/// (10). 카드번호
		//sprintf(tPrtInfo.card_no, "%s", csrc_resp.card_no);

		/// (11). 결제수단 (현금/신용카드 등)
		tPrtInfo.n_pym_dvs = PYM_CD_CSRC;
		//sprintf(tPrtInfo.pym_dvs, "현금(현금영수증)");	

		if(tPrtInfo.ui_csrc_use[0] == 0x00 )
		{	
			//sprintf(tPrtInfo.pym_dvs, "현금(소득공제)");
			sprintf(tPrtInfo.pym_dvs, "소득공제");			// 20211014, 승차권 출력문구 변경
		}
		else
		{
			//sprintf(tPrtInfo.pym_dvs, "현금(지출증빙)");
			sprintf(tPrtInfo.pym_dvs, "지출증빙");			// 20211014, 승차권 출력문구 변경
		}
		sprintf(tPrtInfo.shrt_pym_dvs, "현영");	

		/// 구매자 정보
		MakePasswdChars(tPrtInfo.card_no, csrc_card_resp.csrc_auth_no);

#if 0
		/// (12). 실제 출발 일자
		sprintf(tPrtInfo.atl_depr_dt, "%.*s.%.*s", 2, &csrc_card_resp.atl_depr_dt[4], 2, &csrc_card_resp.atl_depr_dt[6]);	

		/// (13). 실제 출발 시간
		if( memcmp(csrc_card_resp.atl_depr_time, "24", 2) == 0 )
		{
			sprintf(tPrtInfo.atl_depr_time, "00:%.*s", 2, &csrc_card_resp.atl_depr_time[2]);	
		}
		else
		{
			sprintf(tPrtInfo.atl_depr_time, "%.*s:%.*s", 2, &csrc_card_resp.atl_depr_time[0], 2, &csrc_card_resp.atl_depr_time[2]);	
		}
#endif
		/// (12). 출발 일자, 출발시간
		MakeDepartureDateTime(csrc_card_resp.atl_depr_dt, csrc_card_resp.atl_depr_time, tPrtInfo.atl_depr_dt, tPrtInfo.atl_depr_time);

		/// (14). 좌석번호
		sprintf(tPrtInfo.sats_no, "%d", *(int *)iter->sats_no);	
		/// (15). 승차홈
		sprintf(tPrtInfo.rdhm_val, "%s", csrc_card_resp.rdhm_mltp_val);	

		/// (16). 바코드 관련
		{
			char szTicket[100];
			SYSTEMTIME st;

			GetLocalTime(&st);

			/// 발행일자
			sprintf(tPrtInfo.bar_pub_dt, "%s", csrc_card_resp.pub_dt);	
			/// 발행단축터미널코드
			sprintf(tPrtInfo.bar_pub_shct_trml_cd, "%s", csrc_card_resp.pub_shct_trml_cd);	
			/// 발행창구번호
			sprintf(tPrtInfo.bar_pub_wnd_no, "%s", csrc_card_resp.pub_wnd_no);	
			/// 발행일련번호
			sprintf(tPrtInfo.bar_pub_sno, "%04d", *(int *)iter->pub_sno);

			sprintf(szTicket, "%s%s%s%s", tPrtInfo.bar_pub_dt, tPrtInfo.bar_pub_shct_trml_cd, tPrtInfo.bar_pub_wnd_no, tPrtInfo.bar_pub_sno);
			GetTicketSecurityCode((BYTE *)szTicket, strlen(szTicket), tPrtInfo.bar_secu_code);
			TR_LOG_OUT("바코드 체크섬 값 = [%s] \n", tPrtInfo.bar_secu_code);

			/// 발권시간
			sprintf(tPrtInfo.pub_time, "(%02d:%02d:%02d)", st.wHour, st.wMinute, st.wSecond);
			sprintf(tPrtInfo.shrt_pub_time, "(%02d:%02d)", st.wHour, st.wMinute);//kh200709
		}

		/// (17). 항목 출력여부
		{
			///< 버스등급 출력여부		
			tPrtInfo.bus_cls_prin_yn[0]		= csrc_card_resp.bus_cls_prin_yn[0];
			///< 버스운수사명 출력여부	
			tPrtInfo.bus_cacm_nm_prin_yn[0] = csrc_card_resp.bus_cacm_nm_prin_yn[0];
			///< 출발시각 출력여부
			tPrtInfo.depr_time_prin_yn[0]	= csrc_card_resp.depr_time_prin_yn[0];
			///< 좌석번호 출력여부
			tPrtInfo.sats_no_prin_yn[0]		= csrc_card_resp.sats_no_prin_yn[0];
		}

		// 20211008 ADD - 승차권(승객용)에 QR코드 추가
		/// (20). QR 바코드 관련 - 현장발권 현금영수증 카드
		{
			/// 1) 발행일자
			sprintf(tPrtInfo.qr_pub_dt, "%s", csrc_card_resp.pub_dt);	
			TR_LOG_OUT("qr발행일자 = %s", tPrtInfo.qr_pub_dt);
		}
		// 20211008 ~ADD - 승차권(승객용)에 QR코드 추가

		/// 사용자 이름
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->user_nm, sizeof(tPrtInfo.user_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.user_nm, pBuff, nLen);
			}
		}

		/// 인천공항 - 상주직원 데이타 만들기
		Make_IIAC_RsdData((char *)&tPrtInfo);

		m_vtPrtTicket.push_back(tPrtInfo);

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
#endif
	}

	return 0;
}

/**
 * @brief		MakeCreditTicketPrtData
 * @details		[시외버스] 현장발권 신용카드 결제, 승차권 데이타 만들기
 * @param		None
 * @return		항상 = 0
 */
int CPubTckMem::MakeCreditTicketPrtData(BOOL bRF)
{
	int nRet = 0, nLen = 0;
	char *pBuff;
	PKIOSK_INI_ENV_T pEnv;
	POPER_FILE_CONFIG_T pConfig;

	vector<rtk_ktcpbtckcard_list_t>::iterator iter;

	if(bRF == FALSE)
	{
		TR_LOG_OUT(" [시외_신용카드] 승차권 프린트 데이타 작성 !!!");
	}
	else
	{
		TR_LOG_OUT(" [시외_RF후불카드] 승차권 프린트 데이타 작성 !!!");
	}

	pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();
	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	m_vtPrtTicket.clear();
	for(iter = m_vtPbTckCard.begin(); iter != m_vtPbTckCard.end(); iter++)
	{
		TCK_PRINT_FMT_T tPrtInfo;

		::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));

		/// 경유지 정보
		CConfigTkMem::GetInstance()->SearchThruInfo(base.rot_id, tPrtInfo.thru_nm);

		/// 배차방식
		tPrtInfo.alcn_way_dvs_cd[0] = card_resp.alcn_way_dvs_cd[0];

		/// 좌석제 사용유무
		tPrtInfo.sati_use_yn[0] = card_resp.sati_use_yn[0];

		/// 좌석번호출력여부		
		tPrtInfo.sats_no_prin_yn[0] = card_resp.sats_no_prin_yn[0];

		/// 출발시간 출력여부		
		tPrtInfo.depr_time_prin_yn[0] = card_resp.depr_time_prin_yn[0];

		/// (01). 출발지 터미널명 (한글)
		{
			//sprintf(tPrtInfo.depr_trml_ko_nm, "%s", base.depr_trml_nm);		
			nLen = CheckBuffer(pBuff = base.depr_trml_nm, sizeof(tPrtInfo.depr_trml_ko_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_ko_nm, pBuff, nLen);
			}
		}
		/// (02). 출발지 터미널명 (영문)
		{
			//sprintf(tPrtInfo.depr_trml_eng_nm, "%s", base.depr_trml_eng_nm);	
			nLen = CheckBuffer(pBuff = base.depr_trml_eng_nm, sizeof(tPrtInfo.depr_trml_eng_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_eng_nm, pBuff, nLen);
			}
		}

		/// (03). 출발지 터미널 전화번호
		{
			nLen = CheckBuffer(pBuff = pConfig->ccTrmlInfo_t.sz_prn_trml_tel_no, sizeof(tPrtInfo.depr_trml_tel_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_tel_nm, pBuff, nLen);
			}
		}

		/// (04). 도착지 터미널명 (한글)
		{
			//sprintf(tPrtInfo.arvl_trml_ko_nm, "%s", base.arvl_trml_nm);		
			nLen = CheckBuffer(pBuff = base.arvl_trml_nm, sizeof(tPrtInfo.arvl_trml_ko_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.arvl_trml_ko_nm, pBuff, nLen);
			}
		}
		/// (05). 도착지 터미널명 (영문)
		{
			//sprintf(tPrtInfo.arvl_trml_eng_nm, "%s", base.arvl_trml_eng_nm);	
			nLen = CheckBuffer(pBuff = base.arvl_trml_eng_nm, sizeof(tPrtInfo.arvl_trml_eng_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.arvl_trml_eng_nm, pBuff, nLen);
			}
		}

		/// (06). 노선번호 : 인천공항
		{
			Find_CCS_RotName(base.rot_id, tPrtInfo.arvl_trml_rot_num);
		}

		/// (05). 버스운수사 명
		nRet = FindBusCacmName(SVR_DVS_CCBUS, card_resp.bus_cacm_cd, tPrtInfo.bus_cacm_nm, "", tPrtInfo.bus_cacm_tel_nm);	
		/// (06). 버스등급명
		nRet = FindBusClsName(SVR_DVS_CCBUS, card_resp.bus_cls_cd, tPrtInfo.bus_cls_nm);		
		/// (06-1). 짧은버스등급명
		nRet = FindBusClsShctName(SVR_DVS_CCBUS, card_resp.bus_cls_cd, tPrtInfo.bus_cls_shct_nm);
		
		/// (07). 버스티켓종류명
		nRet = FindBusTckKndName(SVR_DVS_CCBUS, iter->bus_tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 
		::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(iter->bus_tck_knd_cd));

		/// (08). 신용카드 승인번호
		sprintf(tPrtInfo.card_aprv_no, "%s", card_resp.card_aprv_no);

		/// (09). 신용카드 승인금액, 요금
		{
			int nValue = 0;
			char szFare[100];

			/// 승인금액
			nValue = *(int *) card_resp.card_aprv_amt;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			if(base.trd_dvs_cd[0] == '3')
			{	/// enc가 있는 ms 카드이며, 카드사 발급한 선불카드임.
				sprintf(tPrtInfo.card_aprv_amt, "%s", szFare);	
			}
			else
			{
				sprintf(tPrtInfo.card_aprv_amt, "%s", szFare);	
			}

			/// 요금 (발권금액)
			nValue = *(int *) iter->tisu_amt;
			tPrtInfo.n_tisu_amt = nValue; 
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.tisu_amt, "%s", szFare);	

			// 시외버스할인종류 
			TR_LOG_OUT("시외버스 할인율 정보 = [%c] ..", iter->cty_bus_dc_knd_cd[0]);
			if( iter->cty_bus_dc_knd_cd[0] != 'Z')
			{	/// 'Z' : 할인율 없음
				// 할인율 문자열
				nRet = ConvertDCRT(iter->dcrt_dvs_cd, tPrtInfo.dcrt_dvs_str);
				// 시외버스할인종류 문자열
				nRet = ConvertDCKnd(iter->cty_bus_dc_knd_cd, tPrtInfo.cty_bus_dc_knd_str);

				TR_LOG_OUT("시외버스 할인율 정보, 할인값(%s), 할인종류(%s) ..", tPrtInfo.dcrt_dvs_str, tPrtInfo.cty_bus_dc_knd_str);
			}
		}

		/// (10). 카드번호
		{
			int len, i, k;
			char Buffer[100];

			::ZeroMemory(Buffer, sizeof(Buffer));

			len = strlen(this->card_resp.card_no);
			for(k = 0, i = 0; i < len; i++)
			{
				if(this->card_resp.card_no[i] != '-')
				{
					Buffer[k++] = this->card_resp.card_no[i] & 0xFF;
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

		// 20220324 ADD~
		{
			double iRate = 0;

			iRate = *(int *)card_resp.frc_cmrt;
			iRate = iRate * 0.01;

			sprintf(tPrtInfo.issu_crcm_cd, "%s", card_resp.issu_crcm_cd);
			sprintf(tPrtInfo.frc_cmrt, "%g", iRate);

			TR_LOG_OUT("[현장발권] 발급카드사코드 = %s", tPrtInfo.issu_crcm_cd);
			TR_LOG_OUT("[현장발권] 가맹점수수료율 = %s", tPrtInfo.frc_cmrt);
		}
		// 20220324 ~ADD
		/// (11). 결제수단 (현금/신용카드 등)
		//tPrtInfo.n_pym_dvs = PYM_CD_CARD;	// 20221124 DEL
		//sprintf(tPrtInfo.pym_dvs, "신용카드");	

		// 20221124 ADD~
		char sPrt_pym_dvs[256];
		memset(sPrt_pym_dvs, 0x00, sizeof(sPrt_pym_dvs));
		TR_LOG_OUT(" >>>>>>>>>> base.pyn_mns_dvs_cd[0] = %c", base.pyn_mns_dvs_cd[0]);
		switch(base.pyn_mns_dvs_cd[0])
		{
		case PYM_CD_QRPC:
			sprintf(sPrt_pym_dvs, "%s", "PAYCO카드");
			sprintf(tPrtInfo.shrt_pym_dvs, "QR");
			tPrtInfo.n_pym_dvs = PYM_CD_QRPC;					
			break;
		case PYM_CD_QRPP:
			sprintf(sPrt_pym_dvs, "%s", "PAYCO포인트");
			sprintf(tPrtInfo.shrt_pym_dvs, "QR");
			tPrtInfo.n_pym_dvs = PYM_CD_QRPP;
			break;
		case PYM_CD_QRTP:
			sprintf(sPrt_pym_dvs, "%s", "티머니페이");
			sprintf(tPrtInfo.shrt_pym_dvs, "QR");
			tPrtInfo.n_pym_dvs = PYM_CD_QRTP;
			break;
		default:
			sprintf(sPrt_pym_dvs, "%s", "현장카드");
			sprintf(tPrtInfo.shrt_pym_dvs, "카드");
			tPrtInfo.n_pym_dvs = PYM_CD_CARD;
			break;
		}
		TR_LOG_OUT(" >>>>>>>>>> sPrt_pym_dvs = %s", sPrt_pym_dvs);
		// 20221124 ~ADD

		tPrtInfo.n_mip_mm_num = *(WORD *) base.mip_term;

		if(bRF == FALSE)
		{
			if( pEnv->tTckOpt.mip_yn[0] == 'Y' )
			{
				if(tPrtInfo.n_mip_mm_num == 0)
				{
					//sprintf(tPrtInfo.pym_dvs, "현장카드(일시불)");			// 20221124 DEL
					sprintf(tPrtInfo.pym_dvs, "%s(일시불)", sPrt_pym_dvs);	// 20221124 MOD
				}
				else
				{
					//sprintf(tPrtInfo.pym_dvs, "현장카드(%d 개월)", tPrtInfo.n_mip_mm_num);			// 20221124 DEL
					sprintf(tPrtInfo.pym_dvs, "%s(%d 개월)", sPrt_pym_dvs, tPrtInfo.n_mip_mm_num);	// 20221124 MOD
				}
			}
			else
			{
				#if 1	// 20220324 ADD~
				if ( ( ( memcmp("C017", tPrtInfo.issu_crcm_cd, 4) == 0 ) || ( memcmp("C028", tPrtInfo.issu_crcm_cd, 4) == 0 )
					|| ( memcmp("C029", tPrtInfo.issu_crcm_cd, 4) == 0 ) || ( memcmp("C030", tPrtInfo.issu_crcm_cd, 4) == 0 )
					|| ( memcmp("C031", tPrtInfo.issu_crcm_cd, 4) == 0 ) || ( memcmp("C032", tPrtInfo.issu_crcm_cd, 4) == 0 ) )
					&& GetEnvOperCorp() == IIAC_OPER_CORP )
				{
					sprintf(tPrtInfo.pym_dvs, "해외카드(%s%%)", tPrtInfo.frc_cmrt);
				}
				else
				{
					//sprintf(tPrtInfo.pym_dvs, "현장카드");			// 20221124 DEL
					sprintf(tPrtInfo.pym_dvs, "%s", sPrt_pym_dvs);	// 20221124 MOD
				}
				#else	// 20220324 ~ADD
				sprintf(tPrtInfo.pym_dvs, "현장카드");
				#endif	// 20220324 ADD
			}
		}
		else
		{
			sprintf(tPrtInfo.pym_dvs, "교통카드(후불)");
		}
		TR_LOG_OUT(" >>>>>>>>>> tPrtInfo.pym_dvs = %s", tPrtInfo.pym_dvs);

		//sprintf(tPrtInfo.shrt_pym_dvs, "카드");					// 20221124 DEL

		/// (12). 출발 일자, 출발시간
		MakeDepartureDateTime(card_resp.atl_depr_dt, card_resp.atl_depr_time, tPrtInfo.atl_depr_dt, tPrtInfo.atl_depr_time);

		/// (14). 좌석번호
		sprintf(tPrtInfo.sats_no, "%02d", *(int *)iter->sats_no);	
		/// (15). 승차홈
		sprintf(tPrtInfo.rdhm_val, "%s", card_resp.rdhm_mltp_val);	

		/// (16). 바코드 관련
		{
			char szTicket[100];
			SYSTEMTIME st;

			GetLocalTime(&st);

			/// 발행일자
			sprintf(tPrtInfo.bar_pub_dt, "%s", card_resp.pub_dt);	
			/// 발행단축터미널코드
			sprintf(tPrtInfo.bar_pub_shct_trml_cd, "%s", card_resp.pub_shct_trml_cd);	
			/// 발행창구번호
			sprintf(tPrtInfo.bar_pub_wnd_no, "%s", card_resp.pub_wnd_no);	
			/// 발행일련번호
			sprintf(tPrtInfo.bar_pub_sno, "%04d", *(int *)iter->pub_sno);

			sprintf(szTicket, "%s%s%s%s", tPrtInfo.bar_pub_dt, tPrtInfo.bar_pub_shct_trml_cd, tPrtInfo.bar_pub_wnd_no, tPrtInfo.bar_pub_sno);
			GetTicketSecurityCode((BYTE *)szTicket, strlen(szTicket), tPrtInfo.bar_secu_code);
			TR_LOG_OUT("바코드 체크섬 값 = [%s] \n", tPrtInfo.bar_secu_code);

			/// 발권시간
			sprintf(tPrtInfo.pub_time, "(%02d:%02d:%02d)", st.wHour, st.wMinute, st.wSecond);
			sprintf(tPrtInfo.shrt_pub_time, "(%02d:%02d)", st.wHour, st.wMinute);//kh200709
		}

		// 20211008 ADD - 승차권(승객용)에 QR코드 추가
		/// (20). QR 바코드 관련 - 현장발권 신용카드
		{
			/// 1) 발행일자
			sprintf(tPrtInfo.qr_pub_dt, "%s", card_resp.pub_dt);	
			TR_LOG_OUT("qr발행일자 = %s", tPrtInfo.qr_pub_dt);
		}
		// 20211008 ~ADD - 승차권(승객용)에 QR코드 추가

		/// (17). 항목 출력여부
		{
			///< 버스등급 출력여부		
			tPrtInfo.bus_cls_prin_yn[0]		= card_resp.bus_cls_prin_yn[0];
			///< 버스운수사명 출력여부	
			tPrtInfo.bus_cacm_nm_prin_yn[0] = card_resp.bus_cacm_nm_prin_yn[0];
			///< 출발시각 출력여부
			tPrtInfo.depr_time_prin_yn[0]	= card_resp.depr_time_prin_yn[0];
			///< 좌석번호 출력여부
			tPrtInfo.sats_no_prin_yn[0]		= card_resp.sats_no_prin_yn[0];
		}

		// 기프트카드 잔액 추가
		{
			int nValue = 0;
			char szFare[100];

			tPrtInfo.trd_dvs_cd[0] = base.trd_dvs_cd[0];
			
#if 0
			nValue = (int) Util_Ascii2Long(card_resp.card_tr_add_inf, strlen(card_resp.card_tr_add_inf));
			TR_LOG_HEXA("card_resp.card_tr_add_inf", (BYTE *)card_resp.card_tr_add_inf, strlen(card_resp.card_tr_add_inf));
			TR_LOG_OUT("[시외] 기프트카드 잔액 = (%d):(%s) ....", nValue, card_resp.card_tr_add_inf);
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.card_tr_add_inf, "%s", szFare);
#else
			/// 시외는 "잔액 : 100원" 이런식으로 tmax에서 내려옴
			sprintf(tPrtInfo.card_tr_add_inf, "%s", card_resp.card_tr_add_inf);
#endif
			//tPrtInfo.n_remain_card_fare = 1000;
		}

		/// 사용자 이름
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->user_nm, sizeof(tPrtInfo.user_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.user_nm, pBuff, nLen);
			}
		}

		/// 인천공항 - 상주직원 데이타 만들기
		Make_IIAC_RsdData((char *)&tPrtInfo);

		/* // 20221207 DEL~
		TR_LOG_OUT(" ========== 선착순 출력 관련 정보  ========================================");
		TR_LOG_OUT(" ========== [시외버스] 현장발권 신용카드 결제, 승차권 데이타 만들기 ==========");
		//TR_LOG_OUT(" (서버정보) 출발시간 출력여부=(%c), 배차=(%c), 좌석제=(%c) ..", tPrtInfo.depr_time_prin_yn[0], tPrtInfo.alcn_way_dvs_cd[0], tPrtInfo.sati_use_yn[0]); // 20221207 DEL
		// 20221207 MOD~
		TR_LOG_OUT(" (서버 배차정보) 배차=(%c), 좌석제=(%c) ..", tPrtInfo.alcn_way_dvs_cd[0], tPrtInfo.sati_use_yn[0]);
		TR_LOG_OUT(" (서버 배차설정) 출발시간 출력여부=(%c), 좌석번호 출력여부=(%c) ..", tPrtInfo.depr_time_prin_yn[0], tPrtInfo.sats_no_prin_yn[0]);
		// 20221207 ~MOD
		//TR_LOG_OUT(" (무인기정보) 배차-비좌석제 출력여부=(%c), 비배차-비좌석제 출력여부(%c) ..", 	pEnv->tTckOpt.nsats_depr_time_yn[0], pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0]); // 20221207 DEL
		TR_LOG_OUT(" (무인기 설정정보) 비좌석제 승차권 시간 출력여부=(%c), 선착순탑승 문구 출력여부(%c) .."
			, pEnv->tTckOpt.nsats_depr_time_yn[0], pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0]); // 20221207 MOD
		*/ // 20221207 ~DEL

		m_vtPrtTicket.push_back(tPrtInfo);

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
#endif

	}

	return 0;
}

/**
 * @brief		MakeRfCardTicketPrtData
 * @details		[시외버스] 현장발권 RF교통카드 결제, 승차권 데이타 만들기
 * @param		None
 * @return		항상 = 0
 */
int CPubTckMem::MakeRfCardTicketPrtData(void)
{
	int nRet = 0, nLen = 0;
	int nValue = 0;
	char szFare[100];
	char *pBuff;
	PKIOSK_INI_ENV_T pEnv;
	POPER_FILE_CONFIG_T pConfig;

	vector<rtk_pubtckppy_t>::iterator iter;
	vector<PUBTCK_UI_RF_CARD_T>::iterator iterUi;

	TR_LOG_OUT(" [시외_RF카드] 승차권 프린트 데이타 작성 !!!");

	pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();
	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	::ZeroMemory(szFare, sizeof(szFare));

	m_vtPrtTicket.clear();
	for(iter = m_vtRfResp.begin(), iterUi = m_vtRfUiData.begin(); 
		(iter != m_vtRfResp.end()) && (iterUi != m_vtRfUiData.end()) ; 
		iter++, iterUi++
	   )
	{
		TCK_PRINT_FMT_T tPrtInfo;

		::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));

		{
			int n_rsp_cd = Util_Ascii2Long(&iter->rsp_cd[3], 3);
			if( n_rsp_cd != 146 )
			{
				continue;
			}
		}

		/// 경유지 정보
		CConfigTkMem::GetInstance()->SearchThruInfo(base.rot_id, tPrtInfo.thru_nm);

		/// 배차방식
		tPrtInfo.alcn_way_dvs_cd[0] = iter->alcn_way_dvs_cd[0];

		/// 좌석제 사용유무
		tPrtInfo.sati_use_yn[0] = iter->sati_use_yn[0];

		/// 좌석번호출력여부		
		tPrtInfo.sats_no_prin_yn[0] = iter->sats_no_prin_yn[0];

		/// 출발시간 출력여부		
		tPrtInfo.depr_time_prin_yn[0] = iter->depr_time_prin_yn[0];

		/// (01). 출발지 터미널명 (한글)
		{
			//sprintf(tPrtInfo.depr_trml_ko_nm, "%s", base.depr_trml_nm);		
			nLen = CheckBuffer(pBuff = base.depr_trml_nm, sizeof(tPrtInfo.depr_trml_ko_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_ko_nm, pBuff, nLen);
			}
		}
		/// (02). 출발지 터미널명 (영문)
		{
			//sprintf(tPrtInfo.depr_trml_eng_nm, "%s", base.depr_trml_eng_nm);	
			nLen = CheckBuffer(pBuff = base.depr_trml_eng_nm, sizeof(tPrtInfo.depr_trml_eng_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_eng_nm, pBuff, nLen);
			}
		}

		/// (03). 출발지 터미널 전화번호
		{
			nLen = CheckBuffer(pBuff = pConfig->ccTrmlInfo_t.sz_prn_trml_tel_no, sizeof(tPrtInfo.depr_trml_tel_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_tel_nm, pBuff, nLen);
			}
		}

		/// (04). 도착지 터미널명 (한글)
		{
			//sprintf(tPrtInfo.arvl_trml_ko_nm, "%s", base.arvl_trml_nm);		
			nLen = CheckBuffer(pBuff = base.arvl_trml_nm, sizeof(tPrtInfo.arvl_trml_ko_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.arvl_trml_ko_nm, pBuff, nLen);
			}
		}
		/// (05). 도착지 터미널명 (영문)
		{
			//sprintf(tPrtInfo.arvl_trml_eng_nm, "%s", base.arvl_trml_eng_nm);	
			nLen = CheckBuffer(pBuff = base.arvl_trml_eng_nm, sizeof(tPrtInfo.arvl_trml_eng_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.arvl_trml_eng_nm, pBuff, nLen);
			}
		}

		/// (06). 노선번호 : 인천공항
		{
			Find_CCS_RotName(base.rot_id, tPrtInfo.arvl_trml_rot_num);
		}

		/// (05). 버스운수사 명
		nRet = FindBusCacmName(SVR_DVS_CCBUS, iter->bus_cacm_cd, tPrtInfo.bus_cacm_nm, "", tPrtInfo.bus_cacm_tel_nm);	
		/// (06). 버스등급명
		nRet = FindBusClsName(SVR_DVS_CCBUS, iter->bus_cls_cd, tPrtInfo.bus_cls_nm);
		/// (06-1). 버스등급명
		nRet = FindBusClsShctName(SVR_DVS_CCBUS, iter->bus_cls_cd, tPrtInfo.bus_cls_shct_nm);
 		/// (07). 버스티켓종류명
		nRet = FindBusTckKndName(SVR_DVS_CCBUS, iter->bus_tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 
		::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(iter->bus_tck_knd_cd));
	
		/// (08). RF 카드 잔액
		{
			TR_LOG_HEXA("RF 카드 잔액", (BYTE *)iterUi->ppy_aprv_aft_bal, sizeof(iterUi->ppy_aprv_aft_bal));
			::CopyMemory(&nValue, iterUi->ppy_aprv_aft_bal, 4);
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.rf_balance_amt, "%s", szFare);	
		}

		/// (09). 신용카드 승인금액, 요금
		{
			/// 승인금액
			nValue = *(int *) iter->tisu_amt;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.card_aprv_amt, "%s", szFare);	

			/// 요금 (발권금액)
// 			nValue = *(int *) iter->tisu_amt;
 			tPrtInfo.n_tisu_amt = nValue; 
// 			::ZeroMemory(szFare, sizeof(szFare));
// 			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.tisu_amt, "%s", szFare);	

			// 시외버스할인종류 
			TR_LOG_OUT("시외버스 할인율 정보 = [%c] ..", iter->cty_bus_dc_knd_cd[0]);
			if( iter->cty_bus_dc_knd_cd[0] != 'Z')
			{	/// 'Z' : 할인율 없음
				// 할인율 문자열
				nRet = ConvertDCRT(iter->dcrt_dvs_cd, tPrtInfo.dcrt_dvs_str);
				// 시외버스할인종류 문자열
				nRet = ConvertDCKnd(iter->cty_bus_dc_knd_cd, tPrtInfo.cty_bus_dc_knd_str);

				TR_LOG_OUT("시외버스 할인율 정보, 할인값(%s), 할인종류(%s) ..", tPrtInfo.dcrt_dvs_str, tPrtInfo.cty_bus_dc_knd_str);
			}
		}

		/// (10). 카드번호
		{
			int len, i, k;
			char Buffer[100];

			::ZeroMemory(Buffer, sizeof(Buffer));

			len = strlen(iterUi->damo_enc_card_no);
			for(k = 0, i = 0; i < len; i++)
			{
				if(iterUi->damo_enc_card_no[i] != '-')
				{
					Buffer[k++] = iterUi->damo_enc_card_no[i] & 0xFF;
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

		/// (11). 결제수단 (RF선불카드)
		tPrtInfo.n_pym_dvs = PYM_CD_RF;
		tPrtInfo.n_mip_mm_num = *(WORD *) base.mip_term;
		sprintf(tPrtInfo.pym_dvs, "교통카드(선불)");	
		sprintf(tPrtInfo.shrt_pym_dvs, "교통");	

		/// (12). 출발 일자, 출발시간
		MakeDepartureDateTime(iter->atl_depr_dt, iter->atl_depr_time, tPrtInfo.atl_depr_dt, tPrtInfo.atl_depr_time);

		/// (14). 좌석번호
		sprintf(tPrtInfo.sats_no, "%02d", *(int *)iter->sats_no);	
		/// (15). 승차홈
		sprintf(tPrtInfo.rdhm_val, "%s", iter->rdhm_mltp_val);	

		/// (16). 바코드 관련
		{
			char szTicket[100];
			SYSTEMTIME st;

			GetLocalTime(&st);

			/// 발행일자
			sprintf(tPrtInfo.bar_pub_dt, "%s", iter->pub_dt);	
			/// 발행단축터미널코드
			sprintf(tPrtInfo.bar_pub_shct_trml_cd, "%s", iter->pub_shct_trml_cd);	
			/// 발행창구번호
			sprintf(tPrtInfo.bar_pub_wnd_no, "%s", iter->pub_wnd_no);	
			/// 발행일련번호
			sprintf(tPrtInfo.bar_pub_sno, "%04d", *(int *)iter->pub_sno);

			sprintf(szTicket, "%s%s%s%s", tPrtInfo.bar_pub_dt, tPrtInfo.bar_pub_shct_trml_cd, tPrtInfo.bar_pub_wnd_no, tPrtInfo.bar_pub_sno);
			GetTicketSecurityCode((BYTE *)szTicket, strlen(szTicket), tPrtInfo.bar_secu_code);
			TR_LOG_OUT("바코드 체크섬 값 = [%s] \n", tPrtInfo.bar_secu_code);

			/// 발권시간
			sprintf(tPrtInfo.pub_time, "(%02d:%02d:%02d)", st.wHour, st.wMinute, st.wSecond);
			sprintf(tPrtInfo.shrt_pub_time, "(%02d:%02d)", st.wHour, st.wMinute);//kh200709
		}

		/// (17). 항목 출력여부
		{
			///< 버스등급 출력여부		
			tPrtInfo.bus_cls_prin_yn[0]		= iter->bus_cls_prin_yn[0];
			///< 버스운수사명 출력여부	
			tPrtInfo.bus_cacm_nm_prin_yn[0] = iter->bus_cacm_nm_prin_yn[0];
			///< 출발시각 출력여부
			tPrtInfo.depr_time_prin_yn[0]	= iter->depr_time_prin_yn[0];
			///< 좌석번호 출력여부
			tPrtInfo.sats_no_prin_yn[0]		= iter->sats_no_prin_yn[0];
		}

		// 20211008 ADD - 승차권(승객용)에 QR코드 추가
		/// (20). QR 바코드 관련 - 현장발권 교통카드(RF)
		{
			/// 1) 발행일자
			sprintf(tPrtInfo.qr_pub_dt, "%s", iter->pub_dt);	
			TR_LOG_OUT("qr발행일자 = %s", tPrtInfo.qr_pub_dt);
		}
		// 20211008 ~ADD - 승차권(승객용)에 QR코드 추가

		/// 사용자 이름
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->user_nm, sizeof(tPrtInfo.user_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.user_nm, pBuff, nLen);
			}
		}

		/// 인천공항 - 상주직원 데이타 만들기
		Make_IIAC_RsdData((char *)&tPrtInfo);

		m_vtPrtTicket.push_back(tPrtInfo);

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
#endif
	}

	return 0;
}

/**
 * @brief		MakeReTicketPrtData
 * @details		[시외버스] 승차권 재발행 티켓 데이타 만들기
 * @param		None
 * @return		항상 = 0
 */
int CPubTckMem::MakeReTicketPrtData(void)
{
	int nRet = 0;

	vector<rtk_rpubtck_t>::iterator iter;

	m_vtPrtTicket.clear();
	for(iter = m_vtPbTckReIssue.begin(); iter != m_vtPbTckReIssue.end(); iter++)
	{
		TCK_PRINT_FMT_T tPrtInfo;

		::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));

		/// 경유지 정보
		//CConfigTkMem::GetInstance()->SearchThruInfo(base.rot_id, tPrtInfo.thru_nm);

		/// 배차방식
		tPrtInfo.alcn_way_dvs_cd[0] = iter->alcn_way_dvs_cd[0];

		/// 좌석제 사용유무
		tPrtInfo.sati_use_yn[0] = iter->sati_use_yn[0];

		/// 좌석번호출력여부		
		tPrtInfo.sats_no_prin_yn[0] = iter->sats_no_prin_yn[0];

		/// 출발시간 출력여부		
		tPrtInfo.depr_time_prin_yn[0] = iter->depr_time_prin_yn[0];

		/// (01). 출발지 터미널명 (한글)
		FindTerminalName(LANG_KOR, iter->depr_trml_cd, tPrtInfo.depr_trml_ko_nm);
		
		/// (02). 출발지 터미널명 (영문)
		FindTerminalName(LANG_ENG, iter->depr_trml_cd, tPrtInfo.depr_trml_eng_nm);

		/// (03). 도착지 터미널명 (한글)
		FindTerminalName(LANG_KOR, iter->arvl_trml_cd, tPrtInfo.arvl_trml_ko_nm);
		/// (04). 도착지 터미널명 (영문)
		FindTerminalName(LANG_ENG, iter->arvl_trml_cd, tPrtInfo.arvl_trml_eng_nm);

		/// (05). 버스운수사 명
		nRet = FindBusCacmName(SVR_DVS_CCBUS, iter->bus_cacm_cd, tPrtInfo.bus_cacm_nm, "", tPrtInfo.bus_cacm_tel_nm);	
		/// (06). 버스등급명
		nRet = FindBusClsName(SVR_DVS_CCBUS, iter->bus_cls_cd, tPrtInfo.bus_cls_nm);
		/// (06-1). 버스등급명
		nRet = FindBusClsShctName(SVR_DVS_CCBUS, iter->bus_cls_cd, tPrtInfo.bus_cls_shct_nm);
 		/// (07). 버스티켓종류명
 		nRet = FindBusTckKndName(SVR_DVS_CCBUS, iter->bus_tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 
		::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(iter->bus_tck_knd_cd));
		/// (08). 신용카드 승인번호
		if(iter->pyn_mns_dvs_cd[0] == PYM_CD_CARD)
		{
			sprintf(tPrtInfo.card_aprv_no, "%s", iter->card_aprv_no);
		}
		else
		{
			sprintf(tPrtInfo.card_aprv_no, "%s", iter->csrc_aprv_no);
		}

		/// (09). 신용카드 승인금액
		{
			int nValue = 0;
			char szFare[100];

			/// 금액(발권금액)
			nValue = *(int *) iter->tisu_amt;
			tPrtInfo.n_tisu_amt = nValue; 
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.card_aprv_amt, "%s", szFare);	
			sprintf(tPrtInfo.tisu_amt, "%s", szFare);	

			// 시외버스할인종류 
			TR_LOG_OUT("시외버스 할인율 정보 = [%c] ..", iter->cty_bus_dc_knd_cd[0]);
			if( iter->cty_bus_dc_knd_cd[0] != 'Z')
			{	/// 'Z' : 할인율 없음
				// 할인율 문자열
				nRet = ConvertDCRT(iter->dcrt_dvs_cd, tPrtInfo.dcrt_dvs_str);
				// 시외버스할인종류 문자열
				nRet = ConvertDCKnd(iter->cty_bus_dc_knd_cd, tPrtInfo.cty_bus_dc_knd_str);

				TR_LOG_OUT("시외버스 할인율 정보, 할인값(%s), 할인종류(%s) ..", tPrtInfo.dcrt_dvs_str, tPrtInfo.cty_bus_dc_knd_str);
			}
		}

		/// (10). 카드번호
		if(iter->pyn_mns_dvs_cd[0] == PYM_CD_CARD)
		{
			int len, i, k;
			char Buffer[100];

			::ZeroMemory(Buffer, sizeof(Buffer));

			len = strlen(iter->card_no);
			for(k = 0, i = 0; i < len; i++)
			{
				if(iter->card_no[i] != '-')
				{
					Buffer[k++] = iter->card_no[i] & 0xFF;
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

		// 20220324 ADD~
		{
			double iRate = 0;

			iRate = *(int *)iter->frc_cmrt;
			iRate = iRate * 0.01;

			sprintf(tPrtInfo.issu_crcm_cd, "%s", iter->issu_crcm_cd);
			sprintf(tPrtInfo.frc_cmrt, "%g", iRate);

			TR_LOG_OUT("[승차권재발행] 발급카드사코드 = %s", tPrtInfo.issu_crcm_cd);
			TR_LOG_OUT("[승차권재발행] 가맹점수수료율 = %s", tPrtInfo.frc_cmrt);
		}
		// 20220324 ~ADD

		/// (11). 결제수단 (현금/신용카드 등)
		tPrtInfo.n_pym_dvs = iter->pyn_mns_dvs_cd[0];
		switch(iter->pyn_mns_dvs_cd[0]) // == PYM_CD_CARD)
		{
		case PYM_CD_CASH:
			sprintf(tPrtInfo.pym_dvs, "현금");	
			sprintf(tPrtInfo.shrt_pym_dvs, "현금");	
			break;
		case PYM_CD_CSRC:
			sprintf(tPrtInfo.pym_dvs, "현금영수증");	
			sprintf(tPrtInfo.shrt_pym_dvs, "현영");	
			break;
		case PYM_CD_CARD:
			#if 1	// 20220324 ADD~
			if ( ( ( memcmp("C017", tPrtInfo.issu_crcm_cd, 4) == 0 ) || ( memcmp("C028", tPrtInfo.issu_crcm_cd, 4) == 0 )
				|| ( memcmp("C029", tPrtInfo.issu_crcm_cd, 4) == 0 ) || ( memcmp("C030", tPrtInfo.issu_crcm_cd, 4) == 0 )
				|| ( memcmp("C031", tPrtInfo.issu_crcm_cd, 4) == 0 ) || ( memcmp("C032", tPrtInfo.issu_crcm_cd, 4) == 0 ) )
				&& GetEnvOperCorp() == IIAC_OPER_CORP )
			{
				sprintf(tPrtInfo.pym_dvs, "해외카드(%s%%)", tPrtInfo.frc_cmrt);
			}
			else
			{
				sprintf(tPrtInfo.pym_dvs, "신용카드");	
			}
			#else	// 20220324 ~ADD
			sprintf(tPrtInfo.pym_dvs, "신용카드");	
			#endif	// 20220324 ADD
			sprintf(tPrtInfo.shrt_pym_dvs, "카드");	
			break;
		}

		// 20221208 ADD~
		/// (11-1). 결제수단 (QR결제 등)
		TR_LOG_OUT(" >>>>>>>>>> iter->qr_pym_pyn_dtl_cd = %s", iter->qr_pym_pyn_dtl_cd);

		if (strcmp(iter->qr_pym_pyn_dtl_cd, "PC") == 0)
		{
			sprintf(tPrtInfo.pym_dvs, "%s", "PAYCO카드");
			sprintf(tPrtInfo.shrt_pym_dvs, "QR");
			tPrtInfo.n_pym_dvs = PYM_CD_QRPC;					
		}
		else if (strcmp(iter->qr_pym_pyn_dtl_cd, "PP") == 0)
		{
			sprintf(tPrtInfo.pym_dvs, "%s", "PAYCO포인트");
			sprintf(tPrtInfo.shrt_pym_dvs, "QR");
			tPrtInfo.n_pym_dvs = PYM_CD_QRPP;
		}
		else if (strcmp(iter->qr_pym_pyn_dtl_cd, "TP") == 0)
		{
			sprintf(tPrtInfo.pym_dvs, "%s", "티머니페이");
			sprintf(tPrtInfo.shrt_pym_dvs, "QR");
			tPrtInfo.n_pym_dvs = PYM_CD_QRTP;
		}
		//else
		//{
		//	sprintf(tPrtInfo.pym_dvs, "%s", "신용카드");
		//	sprintf(tPrtInfo.shrt_pym_dvs, "카드");
		//	tPrtInfo.n_pym_dvs = PYM_CD_CARD;
		//}
		// 20221208 ~ADD

		/// (12). 실제 출발 일자
		MakeDepartureDateTime(iter->atl_depr_dt, iter->atl_depr_time, tPrtInfo.atl_depr_dt, tPrtInfo.atl_depr_time);
		
		/// (14). 좌석번호
		sprintf(tPrtInfo.sats_no, "%d", *(int *)iter->sats_no);	
		/// (15). 승차홈
		sprintf(tPrtInfo.rdhm_val, "%s", iter->rdhm_mltp_val);	

		/// (16). 바코드 관련
		{
			char szTicket[100];

			/// 발행일자
			sprintf(tPrtInfo.bar_pub_dt, "%s", iter->pub_dt);	
			/// 발행단축터미널코드
			sprintf(tPrtInfo.bar_pub_shct_trml_cd, "%s", iter->pub_shct_trml_cd);	
			/// 발행창구번호
			sprintf(tPrtInfo.bar_pub_wnd_no, "%s", iter->pub_wnd_no);	
			/// 발행일련번호
			sprintf(tPrtInfo.bar_pub_sno, "%04d", *(int *)iter->pub_sno);

			sprintf(szTicket, "%s%s%s%s", tPrtInfo.bar_pub_dt, tPrtInfo.bar_pub_shct_trml_cd, tPrtInfo.bar_pub_wnd_no, tPrtInfo.bar_pub_sno);
			GetTicketSecurityCode((BYTE *)szTicket, strlen(szTicket), tPrtInfo.bar_secu_code);
		}

		/// (17). 항목 출력여부
		{
			///< 버스등급 출력여부		
			tPrtInfo.bus_cls_prin_yn[0]		= iter->bus_cls_prin_yn[0];
			///< 버스운수사명 출력여부	
			tPrtInfo.bus_cacm_nm_prin_yn[0] = iter->bus_cacm_nm_prin_yn[0];
			///< 출발시각 출력여부
			tPrtInfo.depr_time_prin_yn[0]	= iter->depr_time_prin_yn[0];
			///< 좌석번호 출력여부
			tPrtInfo.sats_no_prin_yn[0]		= iter->sats_no_prin_yn[0];
		}

		// 20211008 ADD - 승차권(승객용)에 QR코드 추가
		/// (20). QR 바코드 관련 - 승차권 재발행
		{
			/// 1) 발행일자
			sprintf(tPrtInfo.qr_pub_dt, "%s", iter->pub_dt);	
			TR_LOG_OUT("qr발행일자 = %s", tPrtInfo.qr_pub_dt);
		}
		// 20211008 ~ADD - 승차권(승객용)에 QR코드 추가

		// 선불카드인 경우 때문에 추가
		//{
		//	tPrtInfo.trd_dvs_cd[0] = base.trd_dvs_cd[0];
		//	//tPrtInfo.n_remain_card_fare = *(int *)card_resp.card_tr_add_inf;
		//	strcpy(tPrtInfo.card_tr_add_inf, card_resp.card_tr_add_inf);
		//	//tPrtInfo.n_remain_card_fare = 1000;
		//}

		m_vtPrtTicket.push_back(tPrtInfo);

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
#endif
	}

	return 0;
}

/**
 * @brief		ErasePcpySatsInfo
 * @details		[시외버스] 좌석선점 정보 지우기
 * @param		char *sat_no		좌석정보
 * @return		항상 = 0
 */
int CPubTckMem::ErasePcpySatsInfo(char *sat_no)
{
	int nCount = 0;
	vector<rtk_pcpysats_list_t> vec;
	vector<rtk_pcpysats_list_t>::iterator iter;

	nCount = m_vtPcpysats.size();

	//for(iter = m_vtPcpysats.begin(); iter != m_vtPcpysats.end(); nCount++)
	while(nCount > 0)
	{
		iter = m_vtPcpysats.begin();

		TR_LOG_OUT("count(%d), pcpy_sats_no info (%d) ...", nCount, *(int *)sat_no);

		if( memcmp(iter->sats_no, sat_no, sizeof(iter->sats_no) - 1) == 0 )
		{
			TR_LOG_OUT("delete sats_no info (%d) ...", *(int *)sat_no);
			//iter = vec.erase(iter);
			m_vtPcpysats.erase(iter);
		}
		else
		{
			//iter++;
		}
		nCount--;
	}

	return 0;
}


/**
 * @brief		CMrnpMem
 * @details		[시외버스] 예매발권 데이타 메모리 정보 생성 Class
 */
CMrnpMem::CMrnpMem()
{

}

/**
 * @brief		~CMrnpMem
 * @details		[시외버스] 예매발권 데이타 메모리 정보 소멸 Class
 */
CMrnpMem::~CMrnpMem()
{

}

/**
 * @brief		Initialize
 * @details		[시외버스] 예매발권 데이타 초기화
 * @param		None
 * @return		항상 = 0
 */
void CMrnpMem::Initialize(void)
{
#if (_KTC_CERTIFY_ > 0)

	KTC_MemClear(&Base			, sizeof(MRNP_BASE_T));
	KTC_MemClear(&res_complete	, sizeof(PUBMRNP_RES_T));

	n_mrs_num = n_mrs_pub_num =  0;

	// 예매리스트
	m_vtMrnpList.clear();
	vector<rtk_readmrnppt_list_t>().swap(m_vtMrnpList);

	m_vtRcvUiIssue.clear();
	vector<UI_RESP_MRS_ISSUE_LIST_T>().swap(m_vtRcvUiIssue);


	// 승차권 
	m_vtPrtTicket.clear();
	vector<TCK_PRINT_FMT_T>().swap(m_vtPrtTicket);

	// 예매발권 - 수신결과
	m_vtResComplete.clear();
	vector<rtk_pubmrnp_list_t>().swap(m_vtResComplete);

	TR_LOG_OUT("%s", "##### Memory release !!");

#else
	::ZeroMemory(&Base			, sizeof(MRNP_BASE_T));
	::ZeroMemory(&res_complete	, sizeof(PUBMRNP_RES_T));

	n_mrs_num = n_mrs_pub_num =  0;

	// 예매리스트
	m_vtMrnpList.clear();

	m_vtRcvUiIssue.clear();

	// 승차권 
	m_vtPrtTicket.clear();

	// 예매발권 - 수신결과
	m_vtResComplete.clear();
#endif

}

/**
 * @brief		MakeMrnpListPacket
 * @details		[시외버스] 예매발권 리스트 정보 패킷 만들기
 * @param		char *pData			UI 전송용 예매리스트 데이타
 * @return		예매리스트 갯수
 */
int CMrnpMem::MakeMrnpListPacket(char *pData)
{
	int nCount, i;
	UI_SND_MRS_LIST_T* pList;

	pList = (UI_SND_MRS_LIST_T*) pData;

	vector<rtk_readmrnppt_list_t>::iterator	iter;

	i = nCount = 0; 
	for( iter = m_vtMrnpList.begin(); iter != m_vtMrnpList.end(); iter++ )
	{
		///< 01). 예매번호
		::CopyMemory(pList[i].mrs_no, iter->mrs_no, sizeof(pList[i].mrs_no));
		///< 02). 발권id
		::CopyMemory(pList[i].tisu_id, iter->tisu_id, sizeof(pList[i].tisu_id));
		///< 03). 발권일자
		::CopyMemory(pList[i].tisu_dt, iter->tisu_dt, sizeof(pList[i].tisu_dt));
		///< 04). 발권시각
		::CopyMemory(pList[i].tisu_time, iter->tisu_time, sizeof(pList[i].tisu_time));

		///< 05). 발권채널구분코드
		::CopyMemory(pList[i].tisu_chnl_dvs_cd	, iter->tisu_chnl_dvs_cd	, sizeof(pList[i].tisu_chnl_dvs_cd	));
		///< 06). 발행채널구분코드
		::CopyMemory(pList[i].pub_chnl_dvs_cd	, iter->pub_chnl_dvs_cd		, sizeof(pList[i].pub_chnl_dvs_cd	));
		///< 07). 발행일자
		::CopyMemory(pList[i].pub_dt			, iter->pub_dt				, sizeof(pList[i].pub_dt			));
		///< 08). 발행시간
		::CopyMemory(pList[i].pub_time			, iter->pub_time			, sizeof(pList[i].pub_time			));
		///< 09). 승차요금	
		::CopyMemory(pList[i].tisu_amt			, iter->tisu_amt			, sizeof(pList[i].tisu_amt			));
		///< 10). 결제승인 ID
		::CopyMemory(pList[i].pym_aprv_id		, iter->pym_aprv_id			, sizeof(pList[i].pym_aprv_id		));
		///< 11). 출발터미널코드
		::CopyMemory(pList[i].depr_trml_cd		, iter->depr_trml_cd		, sizeof(pList[i].depr_trml_cd		));
		///< 12). 도착터미널코드
		::CopyMemory(pList[i].arvl_trml_cd		, iter->arvl_trml_cd		, sizeof(pList[i].arvl_trml_cd		));
		///< 13). 노선ID
		::CopyMemory(pList[i].rot_id			, iter->rot_id				, sizeof(pList[i].rot_id			));
		///< 14). 노선순번
		::CopyMemory(pList[i].rot_sqno			, iter->rot_sqno			, sizeof(pList[i].rot_sqno			));
		///< 15). 노선명
		::CopyMemory(pList[i].rot_nm			, iter->rot_nm				, sizeof(pList[i].rot_nm			));
		///< 16). 출발일자
		::CopyMemory(pList[i].depr_dt			, iter->depr_dt				, sizeof(pList[i].depr_dt			));
		///< 17). 출발시각
		::CopyMemory(pList[i].depr_time			, iter->depr_time			, sizeof(pList[i].depr_time			));
		///< 18). 버스등급코드			
		::CopyMemory(pList[i].bus_cls_cd		, iter->bus_cls_cd			, sizeof(pList[i].bus_cls_cd		));
		///< 19). 버스티켓종류코드		
		::CopyMemory(pList[i].bus_tck_knd_cd	, iter->bus_tck_knd_cd		, sizeof(pList[i].bus_tck_knd_cd	));
		///< 20). 좌석번호
		::CopyMemory(pList[i].sats_no			, iter->sats_no				, sizeof(pList[i].sats_no			));
		///< 21). 카드번호
		::CopyMemory(pList[i].card_no			, iter->card_no				, sizeof(pList[i].card_no			));
		///< 22). 시외버스시스템구분코드	
		::CopyMemory(pList[i].cty_bus_sys_dvs_cd, iter->cty_bus_sys_dvs_cd	, sizeof(pList[i].cty_bus_sys_dvs_cd));
		///< 23). 발권시스템구분코드
		::CopyMemory(pList[i].tisu_sys_dvs_cd	, iter->tisu_sys_dvs_cd		, sizeof(pList[i].tisu_sys_dvs_cd	));
		///< 24). 시외버스할인종류코드
		::CopyMemory(pList[i].cty_bus_dc_knd_cd	, iter->cty_bus_dc_knd_cd	, sizeof(pList[i].cty_bus_dc_knd_cd	));
		///< 25). 할인율 구분코드
		::CopyMemory(pList[i].dcrt_dvs_cd		, iter->dcrt_dvs_cd			, sizeof(pList[i].dcrt_dvs_cd		));
		///< 26). 할인 이전금액
		::CopyMemory(pList[i].dc_bef_amt		, iter->dc_bef_amt			, sizeof(pList[i].dc_bef_amt		));
		///< 27). 배차일자
		::CopyMemory(pList[i].alcn_dt			, iter->alcn_dt				, sizeof(pList[i].alcn_dt			));
		///< 28). 배차순번
		::CopyMemory(pList[i].alcn_sqno			, iter->alcn_sqno			, sizeof(pList[i].alcn_sqno			));
		///< 29). 재발행 매수
		::CopyMemory(pList[i].rpub_num			, iter->rpub_num			, sizeof(pList[i].rpub_num			));

// 
// 
// 		::CopyMemory(pList[i].depr_dt, iter->depr_dt, sizeof(pList[i].depr_dt));
// 		::CopyMemory(pList[i].depr_time, iter->depr_time, sizeof(pList[i].depr_time));
// 		::CopyMemory(pList[i].depr_trml_cd, iter->depr_trml_cd, sizeof(pList[i].depr_trml_cd));
// 		::CopyMemory(pList[i].arvl_trml_cd, iter->arvl_trml_cd, sizeof(pList[i].arvl_trml_cd));
// 		///< 소요시간
// 		pList[i].n_take_drtm = 0;	
// 		///< 버스운수사코드
// 		//::CopyMemory(pList[i].bus_cacm_cd, iter->		[4];	
// 		::CopyMemory(pList[i].bus_cls_cd, iter->bus_cls_cd, sizeof(pList[i].bus_cls_cd));
// 		::CopyMemory(pList[i].bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(pList[i].bus_tck_knd_cd));
// 		pList[i].n_tisu_amt = *(int *)iter->tisu_amt;
// 		pList[i].w_sats_no = *(WORD *)iter->sats_no;	
// 		pList[i].pub_chnl_dvs_cd[0] = iter->pub_chnl_dvs_cd[0];
// 		pList[i].cty_bus_sys_dvs_cd[0] = iter->cty_bus_sys_dvs_cd[0];
// 		pList[i].tisu_sys_dvs_cd[0] = iter->tisu_sys_dvs_cd[0];	
// 
// 		/// 20191217 add 인천공항 노선때문
// 		///< 노선id
// 		::CopyMemory(pList[i].rot_id,	iter->rot_id, sizeof(pList[i].rot_id));
// 		///< 노선순번
// 		::CopyMemory(pList[i].rot_sqno, iter->rot_sqno, sizeof(pList[i].rot_sqno));
// 		///< 노선명
// 		::CopyMemory(pList[i].rot_nm,	iter->rot_nm, sizeof(pList[i].rot_nm));

		nCount++;
		i++;
	}

	return nCount * sizeof(UI_SND_MRS_LIST_T);
}

/**
 * @brief		MakeReqIssuePacket
 * @details		[시외버스] 예매발권 요청 정보 패킷 만들기
 * @param		char *pData			UI 전송용 예매발권 데이타
 * @return		예매발권 데이타 건수
 */
int CMrnpMem::MakeReqIssuePacket(char *pData)
{
	int nCount, i;//, nLen;
	pstk_pubmrnp_t	pPacket;

	vector<UI_RESP_MRS_ISSUE_LIST_T>::iterator iter;

	pPacket = (pstk_pubmrnp_t) pData;

	nCount = m_vtRcvUiIssue.size();
	iter = m_vtRcvUiIssue.begin();
	for(i = 0; i < nCount; i++)
	{
		if(i == 0)
		{
			///< 발행채널 구분코드
			{
				if(iter->pub_chnl_dvs_cd[0] == 0)
				{
					pPacket->pub_chnl_dvs_cd[0] = 'K';
				}
				else
				{
					pPacket->pub_chnl_dvs_cd[0] = iter->pub_chnl_dvs_cd[0];
				}

				Base.pub_chnl_dvs_cd[0] = pPacket->pub_chnl_dvs_cd[0];
			}
			///< 예매번호
			{
				::CopyMemory(pPacket->mrs_no, iter->mrs_no, sizeof(pPacket->mrs_no) - 1);	
			}
			///< 카드번호
			{
				//nLen = CheckBuffer(pBuff = iter->card, sizeof(sPacket.mrs_no) - 1);
				//::CopyMemory(sPacket.mrs_no, pBuff, nLen);	
				sprintf(pPacket->card_no, "%s", " ");
			}
			///< 시외버스시스템 구분코드
			{
				//nLen = CheckBuffer(pBuff = iter->cty_bus_sys_dvs_cd, sizeof(sPacket.cty_bus_sys_dvs_cd) - 1);
				::CopyMemory(pPacket->cty_bus_sys_dvs_cd, iter->cty_bus_sys_dvs_cd, sizeof(pPacket->cty_bus_sys_dvs_cd) - 1);	
			}
			///< 발권시스템 구분코드
			{
				::CopyMemory(pPacket->tisu_sys_dvs_cd, iter->tisu_sys_dvs_cd, sizeof(pPacket->tisu_sys_dvs_cd) - 1);	
			}
			///< 조회 창구구분
			{
				pPacket->read_wnd_dvs[0] = 'K';
			}

			//Base.tisu_chnl_dvs_cd[0] = pPacket->tisu_chnl_dvs_cd[0];

			///< 발행매수
			{
				::CopyMemory(pPacket->pub_num, (char *)&nCount, 4);	
			}
		}

		///< 발권ID
		{
			::CopyMemory(pPacket->List[i].tisu_id, iter->tisu_id, sizeof(pPacket->List[i].tisu_id) - 1);
		}
		iter++;
	}

	return nCount;
}

/**
 * @brief		MakeTicketPrtData
 * @details		[시외버스] 예매발권 승차권 프린트 데이타 만들기
 * @param		None
 * @return		예매발권 데이타 건수
 */
int CMrnpMem::MakeTicketPrtData(void)
{
	int nRet = 0, nLen = 0;
	char *pBuff;
	POPER_FILE_CONFIG_T pConfig;

	vector<rtk_pubmrnp_list_t>::iterator iter;
	vector<UI_RESP_MRS_ISSUE_LIST_T>::iterator ui_iter;

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	m_vtPrtTicket.clear();
	ui_iter = m_vtRcvUiIssue.begin();
	for(iter = m_vtResComplete.begin(); iter != m_vtResComplete.end(); iter++)
	{
		TCK_PRINT_FMT_T tPrtInfo;

		::ZeroMemory(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));

// 		/// (01). 출발지 터미널명 (한글)
// 		sprintf(tPrtInfo.depr_trml_ko_nm, "%s", this->Base.depr_trml_nm);		
// 		/// (02). 출발지 터미널명 (영문)
// 		sprintf(tPrtInfo.depr_trml_eng_nm, "%s", this->Base.depr_trml_eng_nm);	
// 		/// (03). 도착지 터미널명 (한글)
// 		sprintf(tPrtInfo.arvl_trml_ko_nm, "%s", this->Base.arvl_trml_nm);		
// 		/// (04). 도착지 터미널명 (영문)
// 		sprintf(tPrtInfo.arvl_trml_eng_nm, "%s", this->Base.arvl_trml_eng_nm);	

		/// 배차방식
		tPrtInfo.alcn_way_dvs_cd[0] = this->res_complete.alcn_way_dvs_cd[0];

		/// 좌석제 사용유무
		tPrtInfo.sati_use_yn[0] = this->res_complete.sati_use_yn[0];

		/// 좌석번호출력여부		
		tPrtInfo.sats_no_prin_yn[0] = this->res_complete.sats_no_prin_yn[0];

		/// 출발시간 출력여부		
		tPrtInfo.depr_time_prin_yn[0] = this->res_complete.depr_time_prin_yn[0];

		/// 발권채널구분코드
		tPrtInfo.tisu_chnl_dvs_cd[0] = Base.tisu_chnl_dvs_cd[0];

		/// (01). 출발지 터미널명 (한글)
		{
			nLen = CheckBuffer(pBuff = this->Base.depr_trml_nm, sizeof(tPrtInfo.depr_trml_ko_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_ko_nm, pBuff, nLen);
			}
		}
		/// (02). 출발지 터미널명 (영문)
		{
			nLen = CheckBuffer(pBuff = this->Base.depr_trml_eng_nm, sizeof(tPrtInfo.depr_trml_eng_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_eng_nm, pBuff, nLen);
			}
		}

		/// (03). 출발지 터미널 전화번호
		{
			nLen = CheckBuffer(pBuff = pConfig->ccTrmlInfo_t.sz_prn_trml_tel_no, sizeof(tPrtInfo.depr_trml_tel_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.depr_trml_tel_nm, pBuff, nLen);
			}
		}

		/// (04). 도착지 터미널명 (한글)
		{
			//sprintf(tPrtInfo.arvl_trml_ko_nm, "%s", base.arvl_trml_nm);		
			nLen = CheckBuffer(pBuff = this->Base.arvl_trml_nm, sizeof(tPrtInfo.arvl_trml_ko_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.arvl_trml_ko_nm, pBuff, nLen);
			}
		}
		/// (05). 도착지 터미널명 (영문)
		{
			//sprintf(tPrtInfo.arvl_trml_eng_nm, "%s", base.arvl_trml_eng_nm);	
			nLen = CheckBuffer(pBuff = this->Base.arvl_trml_eng_nm, sizeof(tPrtInfo.arvl_trml_eng_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.arvl_trml_eng_nm, pBuff, nLen);
			}
		}

		/// (06). 노선번호 : 인천공항
		{
			Find_CCS_RotName(ui_iter->rot_id, tPrtInfo.arvl_trml_rot_num);
		}

		/// (05). 버스운수사 명
		nRet = FindBusCacmName(SVR_DVS_CCBUS, this->res_complete.bus_cacm_cd, tPrtInfo.bus_cacm_nm, "", tPrtInfo.bus_cacm_tel_nm);	
		/// (06). 버스등급명
		nRet = FindBusClsName(SVR_DVS_CCBUS, this->res_complete.bus_cls_cd, tPrtInfo.bus_cls_nm);	
		/// (06-1). 짧은버스등급명
		nRet = FindBusClsShctName(SVR_DVS_CCBUS, this->res_complete.bus_cls_cd, tPrtInfo.bus_cls_shct_nm);	
		/// (07). 버스티켓종류명
		nRet = FindBusTckKndName(SVR_DVS_CCBUS, iter->bus_tck_knd_cd, tPrtInfo.bus_tck_knd_nm); 
		::CopyMemory(tPrtInfo.bus_tck_knd_cd, iter->bus_tck_knd_cd, sizeof(iter->bus_tck_knd_cd));
		/// (08). 신용카드 승인번호
		sprintf(tPrtInfo.card_aprv_no, "%s", this->res_complete.card_aprv_no);
		/// (09). 신용카드 승인금액
		{
			int nValue = 0;
			char szFare[100];

			nValue = *(int *) res_complete.card_aprv_amt;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.card_aprv_amt, "%s", szFare);	

			nValue = *(int *) iter->tisu_amt;
			::ZeroMemory(szFare, sizeof(szFare));
			Util_AmountComma(nValue, szFare);
			sprintf(tPrtInfo.tisu_amt, "%s", szFare);	

			// 시외버스할인종류 
			TR_LOG_OUT("시외버스 할인율 정보 = [%c] ..", iter->cty_bus_dc_knd_cd[0]);
			if( iter->cty_bus_dc_knd_cd[0] != 'Z')
			{	/// 'Z' : 할인율 없음
				// 할인율 문자열
				nRet = ConvertDCRT(iter->dcrt_dvs_cd, tPrtInfo.dcrt_dvs_str);
				// 시외버스할인종류 문자열
				nRet = ConvertDCKnd(iter->cty_bus_dc_knd_cd, tPrtInfo.cty_bus_dc_knd_str);

				TR_LOG_OUT("시외버스 할인율 정보, 할인값(%s), 할인종류(%s) ..", tPrtInfo.dcrt_dvs_str, tPrtInfo.cty_bus_dc_knd_str);
			}
		}
		/// (10). 카드번호
		{
			int len, i, k;

			len = strlen(this->res_complete.card_no);
			for(k = 0, i = 0; i < len; i++)
			{
				if(this->res_complete.card_no[i] != '-')
				{
					tPrtInfo.card_no[k++] = this->res_complete.card_no[i] & 0xFF;
				}
			}
		}

		// 20220324 ADD~
		{
			double iRate = 0;

			iRate = *(int *)res_complete.frc_cmrt;
			iRate = iRate * 0.01;

			sprintf(tPrtInfo.issu_crcm_cd, "%s", res_complete.issu_crcm_cd);
			sprintf(tPrtInfo.frc_cmrt, "%g", iRate);

			TR_LOG_OUT("[예매발권] 발급카드사코드 = %s", tPrtInfo.issu_crcm_cd);
			TR_LOG_OUT("[예매발권] 가맹점수수료율 = %s", tPrtInfo.frc_cmrt);
		}
		// 20220324 ~ADD

		TR_LOG_OUT("결제수단 정보 = [%c] ..", res_complete.pyn_mns_dvs_cd[0]);		// 20211015 ADD

		/// (11). 결제수단 (현금/신용카드 등)
		if(res_complete.pyn_mns_dvs_cd[0] == PYM_CD_TPAY)
		{
			tPrtInfo.n_pym_dvs = PYM_CD_TPAY;
			sprintf(tPrtInfo.pym_dvs, "페이");
			sprintf(tPrtInfo.shrt_pym_dvs, "페이");	

		}
		// 20211015 ADD
		else if(res_complete.pyn_mns_dvs_cd[0] == PYM_CD_COMP)
		{
			tPrtInfo.n_pym_dvs = PYM_CD_COMP;
			sprintf(tPrtInfo.pym_dvs, "복합결제");	
			sprintf(tPrtInfo.shrt_pym_dvs, "복합");	

		}
		// 20211015 ~ADD
		else
		{
			tPrtInfo.n_pym_dvs = PYM_CD_CARD;
			#if 1	// 20220324 ADD~
			if ( ( ( memcmp("C017", tPrtInfo.issu_crcm_cd, 4) == 0 ) || ( memcmp("C028", tPrtInfo.issu_crcm_cd, 4) == 0 )
				|| ( memcmp("C029", tPrtInfo.issu_crcm_cd, 4) == 0 ) || ( memcmp("C030", tPrtInfo.issu_crcm_cd, 4) == 0 )
				|| ( memcmp("C031", tPrtInfo.issu_crcm_cd, 4) == 0 ) || ( memcmp("C032", tPrtInfo.issu_crcm_cd, 4) == 0 ) )
				&& GetEnvOperCorp() == IIAC_OPER_CORP )
			{
				sprintf(tPrtInfo.pym_dvs, "해외카드(%s%%)", tPrtInfo.frc_cmrt);
			}
			else
			{		// 20220324 ~ADD
				sprintf(tPrtInfo.pym_dvs, "예매카드");
			}		// 20220324 ADD~
			#else
			sprintf(tPrtInfo.pym_dvs, "예매카드");	
			#endif	// 20220324 ~ADD
			sprintf(tPrtInfo.shrt_pym_dvs, "카드");	

		}

		// 20211015 ADD
		/// 이벤트쿠폰결제금액	
		TR_LOG_OUT("이벤트쿠폰결제금액 정보 : %s ", this->res_complete.evcp_pym_amt);	// 20211015 ADD for DEBUG
		char szFare[100];
		int nValue = *(int *) this->res_complete.evcp_pym_amt;
		::ZeroMemory(szFare, sizeof(szFare));
		Util_AmountComma(nValue, szFare);
		sprintf(tPrtInfo.evcp_pym_amt, "%s", szFare);	

		///	마일리지결제금액		
		TR_LOG_OUT("마일리지결제금액 정보 : %s ", this->res_complete.mlg_pym_amt);		// 20211015 ADD for DEBUG
		nValue = *(int *) this->res_complete.mlg_pym_amt;
		::ZeroMemory(szFare, sizeof(szFare));
		Util_AmountComma(nValue, szFare);
		sprintf(tPrtInfo.mlg_pym_amt, "%s", szFare);	
		// 20211015 ~ADD

		tPrtInfo.n_mip_mm_num = 99;
		/// (12). 실제 출발 일자
		MakeDepartureDateTime(res_complete.atl_depr_dt, res_complete.atl_depr_time, tPrtInfo.atl_depr_dt, tPrtInfo.atl_depr_time);

		//sprintf(tPrtInfo.atl_depr_time, "%.*s:%.*s", 2, &res_complete.atl_depr_time[0], 2, &res_complete.atl_depr_time[2]);	
		/// (14). 좌석번호
		sprintf(tPrtInfo.sats_no, "%02d", *(int *)iter->sats_no);	
		/// (15). 승차홈
		sprintf(tPrtInfo.rdhm_val, "%s", res_complete.rdhm_mltp_val);	
		/// (16). 바코드 관련
		{
			char szTicket[100];
			SYSTEMTIME st;

			GetLocalTime(&st);

			/// 발행일자
			sprintf(tPrtInfo.bar_pub_dt, "%s", res_complete.pub_dt);	

			/// 발행단축터미널코드
			sprintf(tPrtInfo.bar_pub_shct_trml_cd, "%s", res_complete.pub_shct_trml_cd);	

			/// 발행창구번호
			sprintf(tPrtInfo.bar_pub_wnd_no, "%s", res_complete.pub_wnd_no);	
			/// 발행일련번호
			sprintf(tPrtInfo.bar_pub_sno, "%04d", *(int *)iter->pub_sno);

			sprintf(szTicket, "%s%s%s%s", tPrtInfo.bar_pub_dt, tPrtInfo.bar_pub_shct_trml_cd, tPrtInfo.bar_pub_wnd_no, tPrtInfo.bar_pub_sno);
			GetTicketSecurityCode((BYTE *)szTicket, strlen(szTicket), tPrtInfo.bar_secu_code);	

			// 발권시간
			sprintf(tPrtInfo.pub_time, "(%02d:%02d)", st.wHour, st.wMinute);
			sprintf(tPrtInfo.shrt_pub_time, "(%02d:%02d)", st.wHour, st.wMinute);//kh200709
		}

		// 20211008 ADD - 승차권(승객용)에 QR코드 추가
		/// (20). QR 바코드 관련 - 승차권 예매발권
		{
			/// 1) 발행일자
			sprintf(tPrtInfo.qr_pub_dt, "%s", res_complete.pub_dt);	
			TR_LOG_OUT("qr발행일자 = %s", tPrtInfo.qr_pub_dt);
		}
		// 20211008 ~ADD - 승차권(승객용)에 QR코드 추가

		/// 사용자 이름
		{
			nLen = CheckBuffer(pBuff = CConfigTkMem::GetInstance()->user_nm, sizeof(tPrtInfo.user_nm) - 1);
			if(nLen > 0)
			{
				CopyMemory(tPrtInfo.user_nm, pBuff, nLen);
			}
		}

		m_vtPrtTicket.push_back(tPrtInfo);
		ui_iter++;

#if (_KTC_CERTIFY_ > 0)
		KTC_MemClear(&tPrtInfo, sizeof(TCK_PRINT_FMT_T));
#endif
	}

	return 0;
}

/**
 * @brief		CCancRyTkMem
 * @details		시외버스 환불처리 클래스 생성자
 */
CCancRyTkMem::CCancRyTkMem()
{

}

/**
 * @brief		CCancRyTkMem
 * @details		시외버스 환불처리 클래스 소멸자
 */
CCancRyTkMem::~CCancRyTkMem()
{

}

/**
 * @brief		Initialize
 * @details		[시외버스_환불] 메모리 초기화 
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
void CCancRyTkMem::Initialize(void)
{
#if (_KTC_CERTIFY_ > 0)

	KTC_MemClear(&tReqTckNo, sizeof(REQ_TICKET_NO_T));
	KTC_MemClear(&tRespTckNo, sizeof(RESP_TICKET_NO_T));
	KTC_MemClear(&tReqRefund, sizeof(REQ_TICKET_REFUND_T));
	
	::ZeroMemory(ui_pym_dvs_cd, sizeof(ui_pym_dvs_cd));
	n_tot_money = 0;
	n_chg_money = 0;
	n_commission_fare = 0;
	n_commission_rate = 0;
	KTC_MemClear(szTicketData, sizeof(szTicketData));

	m_vtRespTckNoList.clear();
	vector<rtk_readbustckno_list_t>().swap(m_vtRespTckNoList);

	m_vtRespRefundList.clear();
	vector<rtk_cancrytck_list_t>().swap(m_vtRespRefundList);

#else
	::ZeroMemory(&tReqTckNo, sizeof(REQ_TICKET_NO_T));
	::ZeroMemory(&tRespTckNo, sizeof(RESP_TICKET_NO_T));
	::ZeroMemory(&tReqRefund, sizeof(REQ_TICKET_REFUND_T));

	::ZeroMemory(ui_pym_dvs_cd, sizeof(ui_pym_dvs_cd));
	n_tot_money = 0;
	n_chg_money = 0;
	n_commission_fare = 0;
	n_commission_rate = 0;
	::ZeroMemory(szTicketData, sizeof(szTicketData));

	m_vtRespTckNoList.clear();
	m_vtRespRefundList.clear();
#endif

	TR_LOG_OUT("%s", "##### Memory release !!");
}

/**
 * @brief		CalcRefund
 * @details		[시외버스_환불] 승차권 환불 계산하기 
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int CCancRyTkMem::CalcRefund(void)
{
	DWORD				dwCurrTck, dwDeprTck, dwPubTck;
	int					n_toal_fare;
//	int					nBaseMinute = 60;		// 60분
	int					nBaseMinute = 10;		// 10분
	SYSTEMTIME			st;
	char				szCurrDT[20];
	PKIOSK_INI_ENV_T	pEnv;
	char				*pDate;
	char				*pTime;
	POPER_FILE_CONFIG_T	pConfig;

	TR_LOG_OUT("start !!!");

	pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();
	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	dwCurrTck = dwDeprTck = dwPubTck = 0;
	n_commission_rate = 0; 

	/// (0) 현재일시
	GetLocalTime(&st);
	dwCurrTck = Util_GetCurrentTick();
	sprintf(szCurrDT, "%04d%02d%02d", st.wYear, st.wMonth, st.wDay);
	TR_LOG_OUT("(01)_현재일시 - (%04d-%02d-%02d %02d:%02d:%02d), Tick(%ld)..", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, dwCurrTck);

	TR_LOG_OUT(" >>> 결제수단(%c), IC카드 사용유무(%c), 현금사용유무(%c) ", tRespTckNo.pyn_mns_dvs_cd[0], pConfig->base_t.sw_ic_pay_yn, pConfig->base_t.sw_cash_pay_yn);

	// 20230106 ADD~
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 상주직원 무인기에서 환불 불가 
	/// { SVR_DVS_CCBUS		,	CCS_IDX_TCK_IR70	,	"IR70",		"상주직원RF"	  },	
	/// { SVR_DVS_CCBUS		,	CCS_IDX_TCK_IE50	,	"IE50",		"상주직원"	  },	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	TR_LOG_OUT(" >>> 버스티켓종류코드(%s) ", tRespTckNo.bus_tck_knd_cd);
	if ( (GetEnvOperCorp() == IIAC_OPER_CORP)	// 인천공항 T1, T2 인 경우
		&& ( ( memcmp(tRespTckNo.bus_tck_knd_cd, "IR70", sizeof(tRespTckNo.bus_tck_knd_cd)-1) == 0 )
			|| ( memcmp(tRespTckNo.bus_tck_knd_cd, "IE50", sizeof(tRespTckNo.bus_tck_knd_cd)-1) == 0 ) ) )
	{
		TR_LOG_OUT("시외버스_환불 - 상주직원 무인기에서 환불 불가 error !!! ");
		return -98; // 환불이 불가능한 승차권입니다.
	}
	// 20230106 ~ADD

	switch(tRespTckNo.pyn_mns_dvs_cd[0])
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
	case PYM_CD_TPAY :
		if( pConfig->base_t.sw_ic_pay_yn != 'Y' )
		{	
			TR_LOG_OUT("시외버스_환불 - 카드환불 불가 error !!! ");
			return -99;
		}
		break;
	// 20211015 ADD
	case PYM_CD_COMP :
		break;
	// 20211015 ~ADD
	default:
		TR_LOG_OUT("시외버스_환불 - 결제수단 오류 환불 불가 error !!! ");
		return -99;
	}

	// 20221201 ADD~
	BYTE bRefundSatiUseFlag = pEnv->tRefund.szRefundSatiUseFlag[0]; // 비좌석제 환불수수료 적용 옵션 (Y:기본값(환불수수료 기본율 적용), N:환불수수료 적용안함, INI에 정의된 값 없으면 기본값 'Y'적용)

	TR_LOG_OUT(" >>> 비좌석제 환불수수료 적용 옵션(Y:0x59, N:0x4E) = 0x%02X", bRefundSatiUseFlag);
	TR_LOG_OUT(" >>> 좌석제사용여부(Y:0x59, N:0x4E) = 0x%02X", tRespTckNo.sati_use_yn[0]);
	TR_LOG_OUT(" >>> 환불수수료 위약금 타입(A:시외버스, B:인천공항리무진)(A:0x41, B:0x42) = 0x%02X", tRespTckNo.brkp_type[0]); // 20221222 ADD
	
	if( (tRespTckNo.sati_use_yn[0] == 'N') &&  (bRefundSatiUseFlag == 'N') ) // 비좌석제('N') + 비좌석제 환불수수료 적용 안함('N') 이면 수수료 적용 안함(0%)
	{
		TR_LOG_OUT(" >>>>>> 비좌석제 환불수수료 적용 안함 !!! ");
		n_commission_rate = 0; 
	}
	// 20221201 ~ADD
	//if( GetEnvOperCorp() == KUMHO_OPER_CORP)		// 20221201 DEL
	else if( GetEnvOperCorp() == KUMHO_OPER_CORP)	// 20221201 MOD
	{	/// 금호터미널인 경우
		nBaseMinute = 10;		// 10분

		/// (1) 158_response 출발일시
		pDate = (char *)&tRespTckNo.depr_dt;
		pTime = (char *)&tRespTckNo.depr_time;
		dwDeprTck = Util_TickFromString(pDate, pTime);
//		dwDeprTck += (60 * 5);
		dwDeprTck += pEnv->tRefund.nAddDeprTm;
		TR_LOG_OUT("(02)_출발일시 - (%s %s) (%d)초 더하기... dwDeprTck(%ld)(%ld), dwCurrTck(%ld)..", pDate, pTime, pEnv->tRefund.nAddDeprTm, dwDeprTck - 300, dwDeprTck, dwCurrTck);

		/// (2) 158_request_response 발행일시
		pDate = (char *)&tReqTckNo.pub_dt;
		pTime = (char *)&tRespTckNo.pub_time;
		dwPubTck = Util_TickFromString(pDate, pTime);
		TR_LOG_OUT("(03)_발행일시 - (%s %s), Tick(%ld)..", pDate, pTime, dwPubTck);

		/// 비배차는 당일이 아니면 환불불가
		if( tRespTckNo.alcn_way_dvs_cd[0] == 'N' )
		{
			if( memcmp(szCurrDT, tRespTckNo.depr_dt, 8) )
			{
				//TR_LOG_OUT("비배차 승차권은 당일에 한해서만 환불 가능, 출발일자(%s), 현재일자(%s) ..", szCurrDT, tRespTckNo.depr_dt);
				//return -2;
			}
		}

		if( dwDeprTck > dwCurrTck )
		{
			if( (dwCurrTck - dwPubTck) < (60*60) )
			{	/// 발권 1시간 이내이면	
				n_commission_rate = 0; 
				TR_LOG_OUT("#1_발권시간 1시간 이내 : n_commission_rate = %d ..", n_commission_rate);
			}
			else
			{
				/// 출발시간이 1일 이전이면..
				if( (dwDeprTck - dwCurrTck) > (24 * 60 * 60) )
				{
					n_commission_rate = 0; 
					TR_LOG_OUT("#2_출발시간이 1일 이전 : n_commission_rate = %d ..", n_commission_rate);
				}
				else if( (int)(dwDeprTck - dwCurrTck) > (int)(nBaseMinute * 60) )
				{	
					n_commission_rate = 5; 
					TR_LOG_OUT("#3_출발1일 ~ 1시간이전 : n_commission_rate = %d ..", n_commission_rate);
				}
				else
				{
					n_commission_rate = 10; 
					TR_LOG_OUT("#4_출발전 1시간이내 : n_commission_rate = %d ..", n_commission_rate);
				}
			}
		}
		else 
		{
			/// 출발 이후 6시간 이내이면...
			if( (dwCurrTck - dwDeprTck) < (6 * 60 * 60) )
			{
				n_commission_rate = 30; 
				TR_LOG_OUT("#5_출발후 6시간 이내 : n_commission_rate = %d ..", n_commission_rate);
			}
			else
			{
				TR_LOG_OUT("#6_ 출발후 6시간 지남 ..");
				return -1;
			}
		}
	}
	// 20221222 ADD~
	else if (GetEnvOperCorp() == IIAC_OPER_CORP)	// 인천공항 T1, T2 인 경우
	{
		//nBaseMinute = 60;	/// 1시간

		/// (1) 158_response 출발일시
		pDate = (char *)&tRespTckNo.depr_dt;
		pTime = (char *)&tRespTckNo.depr_time;
		dwDeprTck = Util_TickFromString(pDate, pTime);
		TR_LOG_OUT("(02)_출발일시 - (%s %s), Tick(%ld)..", pDate, pTime, dwDeprTck);

		/// (2) 158_request_response 발행일시
		pDate = (char *)&tReqTckNo.pub_dt;
		pTime = (char *)&tRespTckNo.pub_time;
		dwPubTck = Util_TickFromString(pDate, pTime);
		TR_LOG_OUT("(03)_발행일시 - (%s %s), Tick(%ld)..", pDate, pTime, dwPubTck);

		if (tRespTckNo.brkp_type[0] == 'A')			// 환불수수료 위약금 타입 = 시외버스('A')
		{
			TR_LOG_OUT(" >>>>>> 환불수수료 위약금 타입 = 시외버스('A') !!! ");
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/// >>>>>> 환불수수료 위약금 타입 : 시외버스('A')
			/// #1_출발 전
			/// #1.1_출발일 2일 전(48시간) 까지 or 예매,발권 후 1시간 이내 : 0 %
			/// #1.2_예매 1시간 경과 후
			/// #1.2.1_출발시간이 1일 이전 : 0 %
			/// #1.2.2_출발일 전일(1일)부터 ~ 당일 출발 1시간 이내 : 5 %
			/// #1.2.3_출발전 1시간 이내 : 10 %
			/// #2_출발 후
			/// #2.1_출발후 6시간 이전 : 30 %
			/// #2.2_출발후 6시간 이후 : 환불 불가 
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/// #1_출발 전
			if( dwDeprTck > dwCurrTck )
			{
				/// #1.1_출발일 2일 전(48시간) 까지 or 예매,발권 후 1시간 이내 : 0 %
				if( ( (dwDeprTck - dwCurrTck) > (2 * 24 * 60 * 60) )
					|| ( (dwCurrTck - dwPubTck) <= (60 * 60) ) )
				{
					n_commission_rate = 0; 
					TR_LOG_OUT("#1.1_출발일 2일 전(48시간) 까지 or 예매,발권 후 1시간 이내 : n_commission_rate = %d ..", n_commission_rate);
				}
				/// #1.2_예매 1시간 경과 후
				else 
				{
					/// #1.2.1_출발시간이 1일 이전 : 0 %
					if( (dwDeprTck - dwCurrTck) > (24 * 60 * 60) )
					{
						n_commission_rate = 0; 
						TR_LOG_OUT("#1.2.1_예매 1시간 경과 후, 출발시간이 1일 이전 : n_commission_rate = %d ..", n_commission_rate);
					}
					/// #1.2.2_출발일 전일(1일)부터 ~ 당일 출발 1시간 이내 : 5 %
					else if( (int)(dwDeprTck - dwCurrTck) > (int)(60 * 60) )
					{	/// @@@@ 금호터미널과 다름...
						n_commission_rate = 5; 
						TR_LOG_OUT("#1.2.2_예매 1시간 경과 후, 출발일 전일(1일)부터 ~ 당일 출발 1시간 이내 : n_commission_rate = %d ..", n_commission_rate);
					}
					/// #1.2.3_출발전 1시간 이내 : 10 %
					else
					{
						n_commission_rate = 10; 
						TR_LOG_OUT("#1.2.3_예매 1시간 경과 후, 출발전 1시간 이내 : n_commission_rate = %d ..", n_commission_rate);
					}
				}
			}
			/// #2_출발 후
			else 
			{
				/// #2.1_출발후 6시간 이전 : 30 %
				if( (dwCurrTck - dwDeprTck) < (6 * 60 * 60) )
				{
					n_commission_rate = 30; 
					TR_LOG_OUT("#2.1_출발후 6시간 이전 : n_commission_rate = %d ..", n_commission_rate);
				}
				/// #2.2_출발후 6시간 이후 : 환불 불가 
				else
				{
					TR_LOG_OUT("##2.2_출발후 6시간 이후 : 환불 불가");
					return -1;
				}
			}
		}
		else if (tRespTckNo.brkp_type[0] == 'B')	// 환불수수료 위약금 타입 = 인천공항리무진('B')
		{
			TR_LOG_OUT(" >>>>>> 환불수수료 위약금 타입 = 인천공항리무진('B') !!! ");
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/// >>>>>> 환불수수료 위약금 타입 = 인천공항리무진('B')
			/// #1_출발 전
			/// #1.1_출발일 2일 전(48시간) 까지 or : 0 %
			/// #1.2_예매 후 1시간 이내 and 출발시간 1시간 이전 : 0 %
			/// #1.3_예매 후 1시간 이내 and 출발시간 1시간 이내 ~ 출발전 : 0 %
			/// #1.4_예매 1시간 경과 후
			/// #1.4.1_출발시간이 1시간 이전 : 10 %
			/// #1.4.2_출발시간 1시간부터 ~ 출발전 15분까지 : 20 %
			/// #1.4.3_출발전 15분 이내 : 30 %
			/// #2_출발 후
			/// #2.1_출발후 3시간 이전 : 50 %
			/// #2.2_출발후 3시간 이후 : 환불 불가 
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/// #1_출발 전
			if( dwDeprTck > dwCurrTck )
			{
				/// #1.1_출발일 2일 전(48시간) 까지 or : 0 %
				/// #1.2_예매 후 1시간 이내 and 출발시간 1시간 이전 : 0 %
				if( ( (dwDeprTck - dwCurrTck) > (2 * 24 * 60 * 60) )
					|| ( ( (dwCurrTck - dwPubTck) <= (60 * 60) ) && ( (int)(dwDeprTck - dwCurrTck) > (int)(60 * 60) ) ) )
				{
					n_commission_rate = 0; 
					TR_LOG_OUT("#1.1_출발일 2일 전(48시간) 까지 or (#1.2_예매 후 1시간 이내 and 출발시간 1시간 이전) : n_commission_rate = %d ..", n_commission_rate);
				}
				/// #1.3_예매 후 1시간 이내 and 출발시간 1시간 이내 ~ 출발전 : 0 %
				else if( ( (dwCurrTck - dwPubTck) <= (60 * 60) ) && ( (int)(dwDeprTck - dwCurrTck) > 0 ) )
				{
					n_commission_rate = 0; 
					TR_LOG_OUT("#1.3_예매 후 1시간 이내 and 출발시간 1시간 이내 ~ 출발전 : n_commission_rate = %d ..", n_commission_rate);
				}
				/// #1.4_예매 1시간 경과 후
				else
				{
					/// #1.4.1_출발시간이 1시간 이전 : 10 %
					if( (dwDeprTck - dwCurrTck) > (60 * 60) )
					{
						n_commission_rate = 10; 
						TR_LOG_OUT("#1.4.1_예매 1시간 경과 후, 출발시간이 1시간 이전 : n_commission_rate = %d ..", n_commission_rate);
					}
					/// #1.4.2_출발시간 1시간부터 ~ 출발전 15분까지 : 20 %
					else if( (int)(dwDeprTck - dwCurrTck) > (int)(15 * 60) )
					{	/// @@@@ 금호터미널과 다름...
						n_commission_rate = 20; 
						TR_LOG_OUT("#1.4.2_예매 1시간 경과 후, 출발시간 1시간부터 ~ 출발전 15분까지 : n_commission_rate = %d ..", n_commission_rate);
					}
					/// #1.4.3_출발전 15분 이내 : 30 %
					else
					{
						n_commission_rate = 30; 
						TR_LOG_OUT("#1.4.3_예매 1시간 경과 후, 출발전 15분 이내 : n_commission_rate = %d ..", n_commission_rate);
					}
				}
			}
			/// #2_출발 후
			else 
			{
				/// #2.1_출발후 3시간 이전 : 50 %
				if( (dwCurrTck - dwDeprTck) < (3 * 60 * 60) )
				{
					n_commission_rate = 50; 
					TR_LOG_OUT("#2.1_출발후 3시간 이전 : n_commission_rate = %d ..", n_commission_rate);
				}
				/// #2.2_출발후 3시간 이후 : 환불 불가 
				else
				{
					TR_LOG_OUT("##2.2_출발후 3시간 이후 : 환불 불가");
					return -1;
				}
			}
		}
	}
	// 20221222 ~ADD
	else // 금호터미널, 인천공항 아닌 경우
	{	/// 
		nBaseMinute = 60;	/// 1시간

		/// (1) 158_response 출발일시
		pDate = (char *)&tRespTckNo.depr_dt;
		pTime = (char *)&tRespTckNo.depr_time;
		dwDeprTck = Util_TickFromString(pDate, pTime);
		TR_LOG_OUT("(02)_출발일시 - (%s %s), Tick(%ld)..", pDate, pTime, dwDeprTck);

		/// (2) 158_request_response 발행일시
		pDate = (char *)&tReqTckNo.pub_dt;
		pTime = (char *)&tRespTckNo.pub_time;
		dwPubTck = Util_TickFromString(pDate, pTime);
		TR_LOG_OUT("(03)_발행일시 - (%s %s), Tick(%ld)..", pDate, pTime, dwPubTck);

		if( dwDeprTck > dwCurrTck )
		{
			/// 당일발권 && 1시간 이내이면
			//if( (memcmp(szCurrDT, tReqTckNo.pub_dt, 8) == 0) && ((dwCurrTck - dwPubTck) < (60*60)) )

			/// 발권 1시간 이내이면
			if( (dwCurrTck - dwPubTck) < (60*60) )
			{
				n_commission_rate = 0; 
				TR_LOG_OUT("#1_발권시간 1시간 이내 : n_commission_rate = %d ..", n_commission_rate);
			}
			else
			{
				/// 출발시간이 1일 이전이면..
				if( (dwDeprTck - dwCurrTck) > (24 * 60 * 60) )
				{
					n_commission_rate = 0; 
					TR_LOG_OUT("#2_출발시간이 1일 이전 : n_commission_rate = %d ..", n_commission_rate);
				}
				else if( (int)(dwDeprTck - dwCurrTck) > (int)(nBaseMinute * 60) )
				{	/// @@@@ 금호터미널과 다름...
					n_commission_rate = 5; 
					TR_LOG_OUT("#3_출발1일 ~ 1시간이전 : n_commission_rate = %d ..", n_commission_rate);
				}
				else
				{
					n_commission_rate = 10; 
					TR_LOG_OUT("#4_출발전 1시간이내 : n_commission_rate = %d ..", n_commission_rate);
				}
			}
		}
		else 
		{
			/// 출발 이후 6시간 이내이면...
			if( (dwCurrTck - dwDeprTck) < (6 * 60 * 60) )
			{
				n_commission_rate = 30; 
				TR_LOG_OUT("#5_출발후 6시간 이내 : n_commission_rate = %d ..", n_commission_rate);
			}
			else
			{
				TR_LOG_OUT("#6_ 출발후 6시간 지남 ..");
				return -1;
			}
		}
	}

	TR_LOG_OUT("(03)_수수료율 - (%d)..", n_commission_rate);

	/// 환불금액 계산
	{
		// 결제금액
		n_toal_fare = *(int *)tRespTckNo.tisu_amt;

		TR_LOG_OUT("(04)_결제금액 - (%d)..", n_toal_fare);

		// 수수료 금액
		n_commission_fare = (int)((float)n_toal_fare * (float)((float)n_commission_rate / (float)100.0));
		TR_LOG_OUT("(05)_수수료 금액 - (%d)..", n_commission_fare);
		
		/// 지불수단이 카드가 아니고, 결제금액이 100원 미만이면..
		if( (tRespTckNo.pyn_mns_dvs_cd[0] != PYM_CD_CARD) && (tRespTckNo.pyn_mns_dvs_cd[0] != PYM_CD_TPAY) && (n_toal_fare % 100) > 0 ) // tpay 추가 kh
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

	return 1;
}

// - 취소환불구분코드   CANC_RY_DVS_CD
// 		; "C" : 취소 , "R" " 환불, 취소면 위약금이 없이 100% 처리가 됩니다. 유인발매기에는 취소와 환불메뉴가 별도로 있습니다. 환불이면 아래 환불율 구분코드를 입력 하셔야 합니다. 
// 	- 환불율 구분코드     RYRT_DVS_CD
// 		; 환불금액 기준입니다. "0" : 100%, "9" : 90%, "8" : 80%, "7" : 95%, "6" : 70%, "5" : 50%, "4" : 0%
// 	- 지불수단구분코드   PYN_MNS_DVS_CD
// 		; "A" : 현금(자진발급), "B" : 현금영수증, "C" : 신용카드, "P" : 교통카드(선불), "D" : 교통카드(후불), "F" : 무임, "Y" : 군후불 이고 교통카드(선불)은 환불 서비스가 다릅니다. 
// 	- 발권금액  TISU_AMT
// 		; 발권한 금액 입니다. (승차권에 인쇄된 금액)

/**
 * @brief		MakeTmaxRefundData
 * @details		[시외버스_환불] 환불 데이타 생성
 * @param		char *pData		환불 데이타 
 * @return		항상 = 0
 */
int CCancRyTkMem::MakeTmaxRefundData(char *pData)
{
	int nCount = 1, i;
	pstk_cancrytck_t pPacket;

	pPacket = (pstk_cancrytck_t) pData;

	///< 취소환불구분코드	 -> ; "C" : 취소 , "R" " 환불, 취소면 위약금이 없이 100% 처리가 됩니다. 유인발매기에는 취소와 환불메뉴가 별도로 있습니다. 환불이면 아래 환불율 구분코드를 입력 하셔야 합니다. 
	pPacket->canc_ry_dvs_cd[0] = 'R';
	///< 환불매수		
	::CopyMemory(pPacket->ryrt_num, &nCount, sizeof(tReqRefund.ryrt_num) - 1);

	for(i = 0; i < nCount; i++)
	{
		int nRate;
		pstk_cancrytck_list_t pList;

		pList = &pPacket->List[i];

		///< 발권시스템구분코드	
		pList->tisu_sys_dvs_cd[0] = tRespTckNo.tisu_sys_dvs_cd[0];
		///< 발행시스템구분코드	
		pList->pub_sys_dvs_cd[0] = tRespTckNo.pub_sys_dvs_cd[0];
		///< 발행일자		
		::CopyMemory(pList->pub_dt, tReqTckNo.pub_dt, sizeof(pList->pub_dt) - 1);
		///< 발행단축터미널코드	
		::CopyMemory(pList->pub_shct_trml_cd, tReqTckNo.pub_shct_trml_cd, sizeof(pList->pub_shct_trml_cd) - 1);
		///< 발행창구번호		
		::CopyMemory(pList->pub_wnd_no, tReqTckNo.pub_wnd_no, sizeof(pList->pub_wnd_no) - 1);
		///< 발행일련번호		
		::CopyMemory(pList->pub_sno, tReqTckNo.pub_sno, sizeof(pList->pub_sno) - 1);
		///< EB버스티켓일련번호	
		::CopyMemory(pList->eb_bus_tck_sno, tReqTckNo.eb_bus_tck_sno, sizeof(pList->eb_bus_tck_sno) - 1);
		
		///< 환불율구분코드 -> "0" : 100%, "9" : 90%, "8" : 80%, "7" : 95%, "6" : 70%, "5" : 50%, "4" : 0%
		nRate = 100 - n_commission_rate;
		switch(	nRate )
		{
		case 100 : 
			pList->ryrt_dvs_cd[0] = '0';
			break;
		case 90 : 
			pList->ryrt_dvs_cd[0] = '9';
			break;
		case 80 : 
			pList->ryrt_dvs_cd[0] = '8';
			break;
		case 95 : 
			pList->ryrt_dvs_cd[0] = '7';
			break;
		case 70 : 
			pList->ryrt_dvs_cd[0] = '6';
			break;
		case 50 : 
			pList->ryrt_dvs_cd[0] = '5';
			break;
		case 0 : 
		default:
			pList->ryrt_dvs_cd[0] = '4';
			break;
		}
		///< 지불수단구분코드 ; "A" : 현금(자진발급), "B" : 현금영수증, "C" : 신용카드, "P" : 교통카드(선불), "D" : 교통카드(후불), "F" : 무임, "Y" : 군후불 이고 교통카드(선불)은 환불 서비스가 다릅니다. 
		pList->pyn_mns_dvs_cd[0] = tRespTckNo.pyn_mns_dvs_cd[0];
		///< 발권금액		
		::CopyMemory(pList->tisu_amt, &n_tot_money, 4);
	}

	return 0;
}


CConfigTkMem::CConfigTkMem()
{

}

CConfigTkMem::~CConfigTkMem()
{

}

/**
 * @brief		SetErrorCode
 * @details		에러코드 저장하기
 * @param		char *pErrorCode	에러코드
 * @return		None
 */
void CConfigTkMem::SetErrorCode(char *pErrorCode)
{
	sprintf(rsp_cd, "%s", pErrorCode);
	::ZeroMemory(rsp_str, sizeof(rsp_str));
}

/**
 * @brief		SetErrorCode
 * @details		에러코드, 에러내용 저장하기
 * @param		char *pErrorCode	에러코드
 * @param		char *pErrStr		에러내용
 * @return		None
 */
void CConfigTkMem::SetErrorCode(char *pErrorCode, char *pErrStr)
{
	if(pErrorCode == (char *)NULL)
	{
		::ZeroMemory(rsp_cd, sizeof(rsp_cd));
	}
	else
	{
		sprintf(rsp_cd, "%s", pErrorCode);
	}

	if(pErrStr == (char *)NULL)
	{
		::ZeroMemory(rsp_str, sizeof(rsp_str));
	}
	else
	{
		sprintf(rsp_str, "%s", pErrStr);
	}
}

/**
 * @brief		GetErrorCode
 * @details		에러코드 가져오기
 * @param		None
 * @return		에러코드
 */
char* CConfigTkMem::GetErrorCode(void)
{
	return rsp_cd;
}

/**
 * @brief		GetErrorContents
 * @details		에러내용 가져오기
 * @param		None
 * @return		에러코드
 */
char *CConfigTkMem::GetErrorContents(void)
{
	return rsp_str;		
}

/**
 * @brief		CConfigTkMem::GetUiSvrKind
 * @details		서버종류 정보 가져오기
 * @param		None
 * @return		UI에서 사용하는 서버종류 값
 */
int CConfigTkMem::GetUiSvrKind(void)
{
	int nRet = 0;

	switch(n_bus_dvs)
	{
	case SVR_DVS_CCBUS:
		nRet = '1';
		break;
	case SVR_DVS_KOBUS:
		nRet = '2';
		break;
	case SVR_DVS_TMEXP:
		nRet = '3';
		break;
	}

	return nRet;
}

/**
 * @brief		CConfigTkMem::SetSvrKind
 * @details		서버종류 정보 설정하기
 * @param		BYTE bySvr			UI에서 사용하는 서버종류 값
 * @return		None
 */
void CConfigTkMem::SetSvrKind(BYTE bySvr)
{
	switch(bySvr)
	{
	case '1':
		CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_CCBUS;
		break;
	case '2':
		CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_KOBUS;
		break;
	case '3':
		CConfigTkMem::GetInstance()->n_bus_dvs = SVR_DVS_TMEXP;
		break;
	default:
		TR_LOG_OUT("서버종류 설정오류..(0x%02X)", bySvr & 0xFF);
		break;
	}
}

/**
 * @brief		SearchThruInfo
 * @details		[시외] 경유지 찾기
 * @param		char *pRotID		노선ID
 * @param		char *retBuf		경유지 이름 
 * @return		성공 >= 0,  실패 < 0
 */
int CConfigTkMem::SearchThruInfo(char *pRotID, char *retBuf)
{
	int i;

	TR_LOG_OUT("경유지_0 정보 - 노선ID(%s)..", pRotID);

	for(i = 0; i < sizeof(m_tThruInfo) / sizeof(CCS_THRU_INFO_T); i++)
	{
		if( memcmp(pRotID, m_tThruInfo[i].rot_id, sizeof(m_tThruInfo[i].rot_id) - 1) == 0 )
		{
			if( memcmp(m_tThruInfo[i].trml_cd, "5856701", 7 ) == 0 )
			{	/// 남악 경유..
				sprintf(retBuf, "%s", "남악 경유");
				TR_LOG_OUT("경유지_1 정보 - (%s)..", retBuf);
				return 0;	
			}
			if( memcmp(m_tThruInfo[i].trml_cd, "6119902", 7 ) == 0 )
			{	/// 문화동 경유
				sprintf(retBuf, "%s", "문화동 경유");
				TR_LOG_OUT("경유지_2 정보 - (%s)..", retBuf);
				return 1;	
			}
			if( memcmp(m_tThruInfo[i].trml_cd, "6111601", 7 ) == 0 )
			{	/// 운암동 경유
				sprintf(retBuf, "%s", "운암동 경유");
				TR_LOG_OUT("경유지_3 정보 - (%s)..", retBuf);
				return 2;	
			}
		}
	}

	//sprintf(retBuf, "%s", "테스트 경유");
	return -1;
}

/**
 * @brief		ReadKobusCorpInfo
 * @details		[고속] 로컬 운송회사 정보 파일 읽기
 * @param		char *pFileFullName	파일이름
 * @return		성공 >= 0,  실패 < 0
 */
int CConfigTkMem::ReadKobusCorpInfo(char *pFileFullName)
{
	CStdioFile file(pFileFullName, CFile::modeRead | CFile::typeText);
	int		i;
	CString str;
	KO_CORP_INFO_T List;
	char	szAnsiBuff[256];
	char	szTmp[256];
	char	*pUtfBuff;
	char	*tp;
	char	*cp;

	m_vtKoCorpList.clear();

	while(1)
	{
		if( !file.ReadString(str) )
		{
			break;
		}

		::ZeroMemory(&List, sizeof(KO_CORP_INFO_T));
		::ZeroMemory(szAnsiBuff, sizeof(szAnsiBuff));

		pUtfBuff = (LPSTR)((LPCTSTR)str);

		Util_Utf8ToAnsi(pUtfBuff, szAnsiBuff);

		tp = szAnsiBuff;

		for(i = 0; i < 9; i++)
		{
			::ZeroMemory(szTmp, sizeof(szTmp));

			cp = strstr(tp, ",");
			if(cp)
			{
				while(1) 
				{
					if( (*tp == 0x09) || (*tp == 0x20) )
					{
						tp++;
					}
					else
					{
						break;
					}
				}

				switch(i)
				{
				case 0 :
					memcpy(List.code, tp, cp-tp);
					break;
				case 1 :
					memcpy(List.full_nm, tp, cp-tp);
					break;
				case 2 :
					memcpy(List.ko_nm, tp, cp-tp);
					break;
				case 3 :
					memcpy(List.en_nm, tp, cp-tp);
					break;
				case 4 :
					memcpy(List.chn_nm, tp, cp-tp);
					break;
				case 5 :
					memcpy(List.jp_nm, tp, cp-tp);
					break;
				case 6 :
					memcpy(List.biz_no, tp, cp-tp);
					break;
				case 7 :
					memcpy(List.use, tp, cp-tp);
					break;
				}
				tp = cp + 1;
			}
		}

		TR_LOG_OUT("List.code(%s), List.full_nm(%s), List.biz_no(%s)", List.code, List.full_nm, List.biz_no);
		m_vtKoCorpList.push_back(List);
	}
	file.Close();
	return 0;		
}

/**
 * @brief		ReadKobusCorpInfo
 * @details		[고속] 로컬 운송회사 정보 파일 읽기
 * @param		char *pFileFullName	파일이름
 * @return		성공 >= 0,  실패 < 0
 */
int CConfigTkMem::GetKobusCorpInfo(char *pCode, char *retNm, char *retBizNo)
{
	int i;
	vector<KO_CORP_INFO_T>::iterator	iter;

	i = 0;
	for( iter = m_vtKoCorpList.begin(); iter != m_vtKoCorpList.end(); iter++, i++ )
	{
		TR_LOG_OUT("운수사 code 찾기 (%s):(%s)", pCode, iter->code);

		if( memcmp(pCode, iter->code, strlen(pCode)) == 0 )
		{
			sprintf(retNm, "%s", iter->full_nm);
			sprintf(retBizNo, "%s", iter->biz_no);
			return i;
		}
	}

	return -1;
}

/**
 * @brief		IsSatAlcn
 * @details		배차/좌석 정보 체크
 * @param		char *alcn_way_dvs_cd	배차 정보
 * @param		char *sati_use_yn		좌석 사용유무 정보
 * @return		성공 : > 0, 실패 : < 0
 */
int CConfigTkMem::IsSatAlcn(char *alcn_way_dvs_cd, char *sati_use_yn)
{
	if( (alcn_way_dvs_cd == NULL) || (sati_use_yn == NULL) )
	{
		return -1;
	}

	if( (alcn_way_dvs_cd[0] == 'D') && (sati_use_yn[0] == 'Y') )
	{	/// 배차이고, 좌석제이면..
		return 1;
	}

	return 0;
}

/**
 * @brief		IsNsatsAlcn
 * @details		비배차/좌석 정보 체크
 * @param		char *alcn_way_dvs_cd	배차 정보
 * @param		char *sati_use_yn		좌석 사용유무 정보
 * @return		성공 : > 0, 실패 : < 0
 */
int CConfigTkMem::IsNsatsAlcn(char *alcn_way_dvs_cd, char *sati_use_yn)
{
	if( (alcn_way_dvs_cd == NULL) || (sati_use_yn == NULL) )
	{
		return -1;
	}

	if( (alcn_way_dvs_cd[0] == 'D') && (sati_use_yn[0] != 'Y') )
	{	/// 배차이고, 비좌석제이면..
		return 1;
	}

	return 0;
}

/**
 * @brief		IsNalcn
 * @details		비배차 정보 체크
 * @param		char *alcn_way_dvs_cd	배차 정보
 * @param		char *sati_use_yn		좌석 사용유무 정보
 * @return		성공 : > 0, 실패 : < 0
 */
int CConfigTkMem::IsNalcn(char *alcn_way_dvs_cd)
{
	if( alcn_way_dvs_cd == NULL )
	{
		return -1;
	}

	if( alcn_way_dvs_cd[0] != 'D' )
	{	/// 비배차이면..
		return 1;
	}

	return 0;
}

/**
 * @brief		IsPrintFirstCome
 * @details		비배차 정보 체크
 * @param		char *alcn_way_dvs_cd		배차정보
 * @param		char *sati_use_yn			좌석사용유무 정보
 * @param		char *depr_time_prin_yn		출발시간 출력유무 정보
 * @return		출력 : > 0, 미출력 : < 0
 */
int CConfigTkMem::IsPrintFirstCome(char *alcn_way_dvs_cd, char *sati_use_yn, char *depr_time_prin_yn, char *sats_no_prin_yn) // 20221207 MOD
{
	BOOL bFlag = FALSE;
	char pyn_mns_dvs_cd = 0;
	char ui_csrc_dvs_cd = 0;
//	char depr_time_prin_yn = 0;
	PKIOSK_INI_ENV_T pEnv;

	pEnv = (PKIOSK_INI_ENV_T) GetEnvInfo();

	pyn_mns_dvs_cd = CPubTckMem::GetInstance()->base.pyn_mns_dvs_cd[0];
	ui_csrc_dvs_cd = CPubTckMem::GetInstance()->base.ui_csrc_dvs_cd[0];

	// 20221207 MOD~
	TR_LOG_OUT(" ========== 선착순 출력 관련 정보  ========================================");
	TR_LOG_OUT(" ========== [시외버스] 현장발권 신용카드 결제, 승차권 데이타 만들기 ==========");
	TR_LOG_OUT(" (서버 배차정보) 배차=(%c), 좌석제=(%c) ..", alcn_way_dvs_cd[0], sati_use_yn[0]);
	TR_LOG_OUT(" (서버 배차설정) 출발시간 출력여부=(%c), 좌석번호 출력여부=(%c) ..", depr_time_prin_yn[0], sats_no_prin_yn[0]);
	TR_LOG_OUT(" (무인기 설정정보) 비좌석제 승차권 시간 출력여부=(%c), 선착순탑승 문구 출력여부(%c) .."
		, pEnv->tTckOpt.nsats_depr_time_yn[0], pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0]); 
	// 20221207 ~MOD

#if 1 // 20221207 ADD
/// ////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 출발시각출력여부
	if ( depr_time_prin_yn[0] == 'Y' )									// 배차설정값:출발시간출력(depr_time_prin_yn)(Y), 좌석번호출력(sats_no_prin_yn)(_)
	{	/// 출발시각출력여부 'Y'이면...
		if ( (IsSatAlcn(alcn_way_dvs_cd, sati_use_yn) == 1) )			// 배차정보  :배차(alcn_way_dvs_cd)(D), 좌석제(sati_use_yn)(Y)
		{	/// 배차이고, 좌석제이면..
			return 0; /// 출발시간 출력
		}
		else if( IsNsatsAlcn(alcn_way_dvs_cd, sati_use_yn)				// 배차정보  :배차(alcn_way_dvs_cd)(D), 비좌석제(sati_use_yn)(N)
			&& (pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y') )			// INI설정값 :비좌석제승차권시간출력(nsats_depr_time_yn)(Y), 비배차비좌석제선착순탑승문구출력(nalcn_nsats_first_cm_yn)(_)
		{	/// 배차_비좌석제이고, 비좌석제승차권시간출력 유무가 'Y' 이면..
			return 0; /// 출발시간 출력
		}
		else if( IsNalcn(alcn_way_dvs_cd) )								// 배차정보  :비배차(alcn_way_dvs_cd)(N), 좌석제(sati_use_yn)(_)
		{	/// 비배차 이면
			if( pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y' )		// INI설정값 :비좌석제승차권시간출력(nsats_depr_time_yn)(_), 비배차비좌석제선착순탑승문구출력(nalcn_nsats_first_cm_yn)(Y)
			{	/// 비배차, 비좌석제 선착순탑승문구 출력 'Y'이면...
				return 1; /// 선착순 문구 출력
			}
		}
	}
	// 20221207 ADD~ // 배차정보설정값(통전망서버)에 따라 시간/선착순 문구 표시
	// 20221212 요구스펙확인 배차설정값 NN 인 경우에는 '선착순'만 표시로 변경, '배차비좌석/배차설정 NY'인 경우 없음
	else if ( ((depr_time_prin_yn[0] == 'N') && (sats_no_prin_yn[0] == 'N')) 	// 배차설정값:출발시간출력(depr_time_prin_yn)(N), 좌석번호출력(sats_no_prin_yn)(N) 
		&& ((alcn_way_dvs_cd[0] == 'D') && (sati_use_yn[0] == 'N'))				// 배차정보  :배차(alcn_way_dvs_cd)(D), 비좌석제(sati_use_yn)(N)
		&& ((pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y')						// INI설정값 :비좌석제승차권시간출력(nsats_depr_time_yn)(Y), 비배차비좌석제선착순탑승문구출력(nalcn_nsats_first_cm_yn)(N)
			&& (pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'N')) )				
	{	/// 출발시각출력여부 'N'이고...
		return 1; /// 선착순 문구 출력
	}
	// 20221207 ~ADD
/// ////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ////////////////////////////////////////////////////////////////////////////////////////////////////////
#else // 20221207 ADD
	/// 출발시각출력여부
	if( depr_time_prin_yn[0] == 'Y' )
	{
		if(IsSatAlcn(alcn_way_dvs_cd, sati_use_yn) > 0)
		{	/// 배차이고, 좌석제이면..
			bFlag = TRUE;
		}
		else if( IsNsatsAlcn(alcn_way_dvs_cd, sati_use_yn) && (pEnv->tTckOpt.nsats_depr_time_yn[0] == 'Y') ) 
		{	/// 배차_비좌석제이고, 비좌석제 승차권 시간출력 유무가 'Y' 이면
			bFlag = TRUE;
		}
		else if( IsNalcn(alcn_way_dvs_cd) )
		{	/// 비배차 이면
			bFlag = FALSE;
		}
	}

	if(bFlag == TRUE)
	{	/// 출발시간 출력
		return 0;
	}
	else if( (bFlag == FALSE) && (pEnv->tTckOpt.nalcn_nsats_first_cm_yn[0] == 'Y') )
	{	/// 비배차, 비좌석제 선착순탑승문구 출력 'Y'이면, 선착순 문구 출력
		return 1;
	}
#endif // 20221207 ADD

	/// 문구출력안함
	return -1;
}

/**
 * @brief		IsPrintSatsNo
 * @details		좌석번호 프린트 유무
 * @param		char *alcn_way_dvs_cd		배차정보
 * @param		char *sats_no_prin_yn		좌석번호 프린트 유무 정보
 * @return		출력 : > 0, 미출력 : <= 0
 */
int CConfigTkMem::IsPrintSatsNo(char *alcn_way_dvs_cd, char *sats_no_prin_yn)
{
	if( (alcn_way_dvs_cd == NULL) || (sats_no_prin_yn == NULL) )
	{
		return -1;
	}

	if( (alcn_way_dvs_cd[0] == 'D') && (sats_no_prin_yn[0] == 'Y') )
	{	/// 배차이고, 좌석번호출력여부 = Y 이면 => 좌석번호 출력
		return 1;
	}

	return 0;
}

/**
 * @brief		SetTmExpUserNo
 * @details		티머니고속 서버인증 사용자No 저장
 * @param		char *user_no				서버인증 사용자No
 * @return		항상 = 0
 */
int CConfigTkMem::SetTmExpUserNo(char *user_no)
{
	::CopyMemory(m_tTmExp.req_user_no, user_no, sizeof(m_tTmExp.req_user_no));

	return 0;
}

/**
 * @brief		GetTmExpUserNo
 * @details		티머니고속 서버인증 사용자No 가져오기
 * @param		char *user_no				서버인증 사용자No
 * @return		항상 = 0
 */
int CConfigTkMem::GetTmExpUserNo(char *user_no)
{
	::CopyMemory(user_no, m_tTmExp.req_user_no, sizeof(m_tTmExp.req_user_no));

	return 0;
}
