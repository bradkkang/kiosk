// 
// 
// dev_tr_tmexp_mem.h : tmoney express bus transact memory 헤더 파일
//

#pragma once

#include <iostream>
#include <vector>

#include "dev_ui_main.h"
#include "svr_tm_expbus_st.h"
#include "svr_kobus_st.h"
#include "dev_tr_mem.h"

//----------------------------------------------------------------------------------------------------------------------

#define MAX_LIST	100

using namespace std;

//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

typedef struct 
{
	int	 nTotalMoney			;		///< 총 결제금액
	int  n_pbtck_chg_money		;		///< 방출금액

	int  n_select_alcn_list		;		///< 배차리스트 선택된 index

	ABILL_T insBill				;		///< 투입금액
	ABILL_T outBill				;		///< 방출지폐
	ACOIN_T outCoin				;		///< 방출동전

	char ui_pym_dvs_cd		[1]	;		///< 현금결제, 카드결제
	char ui_csrc_dvs_cd		[1]	;		///< 현금영수증 구분코드 - 0:미사용, 1:수기입력, 2:신용카드
	char ui_csrc_no			[20	+1];	///< 현금영수증번호
	char ui_csrc_use		[1	+1];	///< 현금영수증 용도 (0x00:개인, 0x01:법인)
	char ui_card_pwd		[10	+1];	///< 카드비밀번호

	///> UI 배차조회 요청
	char user_no 			[6	+1]	;	///< 사용자번호 	
	char lng_cd 			[2	+1]	;	///< 언어코드 		
	char depr_trml_cd		[7	+1]	;	///< 출발터미널코드		
	char arvl_trml_cd		[7	+1]	;	///< 도착터미널코드		
	char depr_dt			[8	+1]	;	///< 요청 조회 대상일		
	char depr_time			[6	+1]	;	///< 요청 조회 대상 시각		
	char bus_cls_cd			[3	+1]	;	///< 버스등급코드

	char pub_time			[6	+1]	;	///< 발권시각

	char inhr_no			[8	+1]	;	///< 고유번호 (v)

	char bus_oprn_dist		[11 +1]	;	///< 운행거리		

	///

	char pyn_mns_dvs_cd		[1	+1];	///< 지불수단구분코드		


	char rsp_cd				[6	+1]	;	///< rsp_cd
	char bef_aft_dvs		[1	+1]	;	///< 이전이후 구분

	char n_tot_sats_num		[4	+0]	;	///< (Number)총좌석수		
	char n_rmn_sats_num		[4	+0]	;	///< (Number)잔여좌석수		
	char sats_mltp_val		[500+1]	;	///< 좌석다중값		
	char trml_sats_asgt_val	[300+1]	;	///< 터미널좌석할당값	

	char rot_rdhm_no_val	[100+1]	;	///< 노선승차홈번호값

} PUBTCK_TMEXP_T, *PPUBTCK_TMEXP_T;

/// 환불
typedef struct  
{
	BOOL bStart;

	char ui_pym_dvs_cd		[1	+0]	;	/// ui 결제수단 (1:현금, 2:카드)
	int n_tot_money					;	/// 결제금액
	int n_chg_money					;	/// 방출금액
	int n_commission_fare			;	/// 취소 수수료 금액
	int n_commission_rate			;	/// 수수료율
	int n_tot_disc_rate				;	/// 최종 
	char receipt_yn			[1	+0]	;	///< 환불영수증 출력여부

	char szTicketData		[1024]	;	/// 승차권 데이타

	int n_tissu_trml_no				;	/// 발권 터미널번호

} CANCRY_TMEXP_BASE_T, *PCANCRY_TMEXP_BASE_T;

#pragma pack()

//----------------------------------------------------------------------------------------------------------------------


///< 티머니 고속버스 - 현장발권
class CPubTckTmExpMem
{
private:
	CPubTckTmExpMem();
	~CPubTckTmExpMem();

public:

	PUBTCK_TMEXP_T base;

	/// 1. 배차조회 정보
		/// [티머니고속] ui_recv 배차조회 요청
		stk_tm_readalcn_t				m_tReqAlcn;

		/// tmax_recv 배차조회 정보 - 최종배차 정보(1)
		vector<rtk_tm_readalcn_last_list_t>	 m_vtResAlcnLastList;
		/// [티머니고속] tmax_recv 배차조회 정보 결과(2)
		vector<rtk_tm_readalcn_list_t>	m_vtResAlcnList;  

	/// 2. 좌석정보
		/// [티머니고속] ui_recv 요금/좌석상태 조회 요청
		stk_tm_readsatsfee_t			m_tReqSats;
		// [티머니고속] 요금정보
		vector<rtk_tm_readsatsfee_list_t>	m_vtResFee;  
		// [티머니고속] 좌석정보
		vector<rtk_tm_readsats_disc_list_t>	m_vtResDisc;  

	/// 3. 좌석선점 정보
		/// [티머니고속] ui_recv 좌석선점 정보 요청
		stk_tm_pcpysats_t					m_tReqSatsPcpy;

		///> [티머니고속] tmax_recv 좌석선점정보
		vector<rtk_tm_pcpysats_list_t>	m_tResSatsPcpy;  

	/// 4. 발권 
		///> tmax_send_recv 승차권발권(현금) - v
		stk_tm_pubtckcash_t						m_tReqPubTckCash;
		rtk_tm_pubtckcash_t						m_tResPubTckCash;
		vector<rtk_tm_pubtckcash_list_t>		m_tResPubTckCashList;  

		///> tmax_send_recv 승차권발권(KTC_카드) - v
		stk_tm_pubtckcard_ktc_t					m_tReqPubTckCardKtc;
		rtk_tm_pubtckcard_ktc_t					m_tResPubTckCardKtc;
		vector<rtk_tm_pubtckcard_ktc_list_t>	m_tResPubTckCardKtcList;  

		///> tmax_send_recv 승차권발권(KTC_신용카드_현금영수증) - v
		stk_tm_pubtckcsrc_ktc_t					m_tReqPubTckCsrcKtc;
		rtk_tm_pubtckcsrc_ktc_t					m_tResPubTckCsrcKtc;
		vector<rtk_tm_pubtckcsrc_ktc_list_t>	m_tResPubTckCsrcKtcList;  

		///> tmax_recv 승차권발권(카드) - v
		stk_tm_pubtckcard_t						m_tReqPubTckCard;
		rtk_tm_pubtckcard_t						m_tResPubTckCard;
		vector<rtk_tm_pubtckcard_list_t>		m_tResPubTckCardList;  

		///> tmax_recv 승차권발권(현금영수증) - v
		stk_tm_pubtckcsrc_t						m_tReqPubTckCsrc;
		rtk_tm_pubtckcsrc_t						m_tResPubTckCsrc;
		vector<rtk_tm_pubtckcsrc_list_t>		m_tResPubTckCsrcList;  

		///> tmax_recv 승차권발권(부가상품권) - v
		stk_tm_pubtckprd_t						m_tReqPubTckPrd;
		rtk_tm_pubtckprd_t						m_tResPubTckPrd;
		vector<rtk_tm_pubtckprd_list_t>			m_tResPubTckPrdList;  

	/// 5. 경유지 정보
		/// ui_recv 경유지 정보 요청
		stk_tm_readthrutrml_t					m_tReqThru;
		/// tmax_recv 경유지 정보 결과
		vector<rtk_tm_readthrutrml_list_t>		m_vtResThruList;  


	int n_pbtck_chg_money			;	/// 거스름돈
	int n_pbtck_count				;	/// 발행매수
	int n_total_tisu_amt			;	/// 총 발행금액

	// 티머니고속 - 승차권 인쇄
	vector<TCK_PRINT_FMT_T>						m_vtPrtTicket;
	vector<TCK_PRINT_FMT_T>						m_vtPrtTicketTest;  // 20211116 ADD

	// 티머니고속 발행내역 조회
	vector<rtk_tm_pub_inq_list_t>				m_vtPbTckInqList;
	
	/// 재발행
	stk_tm_rpub_tck_t							m_tReqRePubTck;
	rtk_tm_rpub_tck_t							m_tRespRePubTck;


public:
	static CPubTckTmExpMem* GetInstance()
	{
		static CPubTckTmExpMem m_instance;

		return &m_instance;
	}

	void Initialize(void);

	int MakeAlcnListPacket(char *pSend);
	int MakeSatsFeeListPacket(char *pSend);
	int MakeSatsPcpyListPacket(char *pSend);
	
	int MakePbTckCashPacket(char *pSend);
	int MakePbTckCardKtcPacket(char *pSend);
	int MakePbTckCsrcKtcPacket(char *pSend);
	int MakePbTckPrdPacket(char *pSend);
	int MakePbTckThruInfo(char *pSend);

	int FindPubTckListData(char *tissu_dt, char *tissu_sno, char *retBuf);
	int FindBusDcKind(char *bus_tck_knd_cd, char *ret_bus_dc_knd_cd, char *ret_dcrt_dvs_cd);
	
	/// 현금
	int MakeCashTicketPrtData(void);
	/// 현금영수증 - 수기
	int MakeCsrcTicketPrtData(void);
	/// 현금영수증 - 카드
	int MakeCsrcCardTicketPrtData(void);
	/// 신용카드
	int MakeCreditTicketPrtData(void);


	int MakeReTicketPrtData(void);
};

///< 티머니 고속버스 - 예매발권

#pragma pack(1)

typedef struct  
{
	char rsp_cd						[6		+1];	///< rsp_code
	char req_dvs_cd					[1		+1];	///< 

	char pyn_dtl_cd					[1		+1];	///< 지불상세코드
	char card_no					[100	+1];	///< 카드번호	
	char card_aprv_no				[100	+1];	///< 카드승인번호
	char aprv_amt					[4		+0];	///< (Number)승인금액	
	char frc_cmrt					[3		+1];	///< 가맹점수수료율
	char frc_cmm					[4		+0];	///< 가맹점수수료

	char tissu_chnl_dvs_cd			[1		+1];	///< 발권채널구분코드	 : 모바일티켓('3')인지, 인터넷티켓인지 구분

} MRNP_TMEXP_BASE_T, *PMRNP_TMEXP_BASE_T;

#pragma pack()

class CMrnpTmExpMem
{
public:
	CMrnpTmExpMem();
	~CMrnpTmExpMem();

	MRNP_TMEXP_BASE_T				Base;

	// [티머니고속] ui_recv, tmax_send 예매리스트 요청
	UI_RESP_TMEXP_MRS_REQ_T			m_ReqList;			
	// [티머니고속] tmax_recv 예매발권 리스트 수신
	vector<rtk_tm_read_mrs_list_t>	m_vtMrnpList;			

	// [티머니고속] KTC 예매에 대한 사항
		UI_RESP_TMEXP_KTC_MRS_REQ_T			m_ReqKtcList;			
		// [티머니고속] tmax_recv KTC 예매발권 리스트 수신
		vector<rtk_tm_read_mrs_ktc_list_t>	m_vtMrnpKtcList;			

		// [티머니고속] ui_recv, tmax_send 예매발권 요청
		UI_RESP_TMEXP_MRS_REQ_PUB_T			m_ReqPubMrs;
		// [티머니고속] ui_send, tmax_recv 예매발권 response
		vector<rtk_tm_pub_mrs_list_t>		m_vtResComplete;			

		UI_RESP_TMEXP_MRS_REQ_MOBILE_PUB_T	m_ReqPubMobileMrs;
		vector<rtk_tm_pub_mrs_htck_list_t>	m_vtResCompleteMobile;			

	vector<TCK_PRINT_FMT_T>				m_vtPrtTicket;

	int MakeMrnpListPacket(char *pData);
	int MakeMrnpKtcListPacket(char *pData);

	int MakeReqIssuePacket(char *pData);
	///> 예매발권 응답데이타 UI 전송 패킷 만들기
	int MakePubMrnpResultPacket(char *pData);
	
	///> 모바일티켓 예매발권 응답데이타 UI 전송 패킷 만들기
	int MakePubMrnpMobileResultPacket(char *pData);

	int MakeTicketPrtData(void);
	int GetPynDtlCdByMrnpNo(char *sats_no, char *retBuf);


public:
	static CMrnpTmExpMem* GetInstance()
	{
		static CMrnpTmExpMem m_instance;

		return &m_instance;
	}

	void Initialize(void);
};

/////////////////////////////////////////////////////////////////////////////////////////////
///< 티머니고속 - 환불

class CCancRyTkTmExpMem
{
public:
	CCancRyTkTmExpMem();
	~CCancRyTkTmExpMem();

	CANCRY_TMEXP_BASE_T					tBase;

	/// 환불 조회
	stk_tm_read_bus_tckno_t				tReqInq;
	rtk_tm_read_bus_tckno_t				tRespInq;
	//vector<rtk_tm_read_tckno_list_t>	m_vtRespInqList;

	/// 환불
	stk_tm_cancrytck_t					tReqRef;
	vector<rtk_tm_cancrytck_list_t>		m_vtRespRefList;

public:
	static CCancRyTkTmExpMem* GetInstance()
	{
		static CCancRyTkTmExpMem		m_instance;

		return &m_instance;
	}

	void Initialize(void);

	int CheckRefund(void);
	int CalcRefund(void);
};


