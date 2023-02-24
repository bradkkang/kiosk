// 
// 
// dev_tr_kobus_mem.h : kobus transact memory 헤더 파일
//

#pragma once

#include <iostream>
#include <vector>

#include "dev_ui_main.h"
#include "svr_kobus_st.h"
#include "dev_tr_mem.h"

//----------------------------------------------------------------------------------------------------------------------

#define MAX_LIST	100

using namespace std;

//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

typedef struct 
{
	int	 nTotalMoney			;	///< 총 결제금액
	int  n_pbtck_chg_money		;	///< 방출금액

	ABILL_T insBill				;	///< 투입금액
	ABILL_T outBill				;	///< 방출지폐
	ACOIN_T outCoin				;	///< 방출동전

	char ui_pym_dvs_cd		[1]	;	///< 현금결제, 카드결제
	char ui_csrc_dvs_cd		[1]	;	///< 현금영수증 구분코드 - 0:미사용, 1:수기입력, 2:신용카드
	char ui_csrc_no			[20+1];	///< 현금영수증번호
	char ui_csrc_use		[1+1];	///< 현금영수증 용도 (0:개인, 1:법인)
	char ui_card_pwd		[10+1];	///< 카드비밀번호

	///> UI 배차조회 요청
	char user_no 			[6+1]	;	///< 사용자번호 	
	char lng_cd 			[2+1]	;	///< 언어코드 		
	char depr_trml_cd		[7+1]	;	///< 출발터미널코드		
	char arvl_trml_cd		[7+1]	;	///< 도착터미널코드		
	char depr_dt			[8+1]	;	///< 요청 조회 대상일		
	char depr_time			[6+1]	;	///< 요청 조회 대상 시각		
	char bus_cls_cd			[3+1]	;	///< 버스등급코드

	char pub_time			[6+1]	;	///< 발권시각

} PUBTCK_KO_T, *PPUBTCK_KO_T;

/// 환불
typedef struct  
{
	BOOL bStart;

	char ui_pym_dvs_cd[1];			/// ui 결제수단 (1:현금, 2:카드)
	int n_tot_money;				/// 결제금액
	int n_chg_money;				/// 방출금액
	int n_commission_fare;			/// 취소 수수료 금액
	int n_commission_rate;			/// 수수료율
	char receipt_yn	[1]	;			///< 환불영수증 출력여부

	char szTicketData[1024];		/// 승차권 데이타

	int n_tissu_trml_no;			/// 발권 터미널번호

} CANCRY_KOBUS_BASE_T, *PCANCRY_KOBUS_BASE_T;

#pragma pack()

//----------------------------------------------------------------------------------------------------------------------


///< 코버스 - 현장발권
class CPubTckKobusMem
{
private:
	CPubTckKobusMem();
	~CPubTckKobusMem();

public:

	PUBTCK_KO_T base;

	/// 1. 배차조회 정보
		/// ui_recv 배차조회 요청
		UI_RESP_KO_REQ_ALCN_T			m_tReqAlcn;
		/// tmax_recv 배차조회 정보 & List
		rtk_tm_timinfo_fee_t			m_vtResAlcnInfo;
		/// tmax_recv 배차조회 정보 결과
		vector<rtk_tm_timinfo_list_t>	m_vtResAlcnList;  

	/// 2. 좌석정보
		/// ui_recv 좌석정보조회 요청
		UI_RESP_KO_REQ_SATS_T			m_tReqSats;
		// 좌석정보
		vector<rtk_cm_setinfo_list_t>	m_vtResSats;  

	/// 3. 좌석선점 정보
		/// ui_recv 좌석선점 정보 요청
		UI_RESP_KO_REQ_SATSPCPY_T		m_tReqSatsPcpy;

		///> tmax_recv 좌석선점정보
		vector<rtk_tw_satspcpy_list_t>	m_tResSatsPcpy;  

	/// 4. 카드발권 
		/// ui_recv 카드발권 요청
		UI_RESP_KO_TCK_TRAN_T			m_tReqTckIssue;

		///> tmax_recv 카드발권
		rtk_tm_tcktran_info_t			m_tResTckIssueInfo;
		vector<rtk_tm_tcktran_list_t>	m_tResTckIssueList;  


	/// 5. 경유지 정보
		/// ui_recv 경유지 정보 요청
		UI_RESP_KO_REQ_THRUINFO_T		m_tReqThru;
		/// tmax_recv 경유지 정보 결과
		vector<rtk_tm_ethruinfo_list_t>	m_vtResThruList;  


	int n_pbtck_chg_money			;	/// 거스름돈
	int n_pbtck_count				;	/// 발행매수
	int n_total_tisu_amt			;	/// 총 발행금액

	// 코버스 - 승차권 인쇄
	vector<TCK_PRINT_FMT_T>			m_vtPrtTicket;
	
	// 테스트 승차권 
	vector<TCK_PRINT_FMT_T>			m_vtPrtTicketTest;	// 20211116 ADD

	// 코버스 - 재발권
	vector<rtk_tm_mrettran_list_t> m_vtPbTckReIssue;

public:
	static CPubTckKobusMem* GetInstance()
	{
		static CPubTckKobusMem m_instance;

		return &m_instance;
	}

	void Initialize(void);

	int MakeAlcnListPacket(char *pSend);
	int MakeThruListPacket(char *pSend);
	int FindBusDcKind(char *bus_tck_knd_cd, char *ret_bus_dc_knd_cd, char *ret_dcrt_dvs_cd);
	void MakePasswdChars(char *retBuf, char *pSrc);
	int MakeAllTicketPrtData(int n_pyn_dvs_cd, int ui_csrc_dvs_cd, int chit_use_dvs);
	int MakeReTicketPrtData(void);
};

///< 코버스 - 예매발권
class CMrnpKobusMem
{
public:
	CMrnpKobusMem();
	~CMrnpKobusMem();


	// [코버스] ui_recv, tmax_send 예매리스트 요청
	UI_RESP_KO_MRS_REQ_LIST_T		m_ReqList;			

	// [코버스] tmax_recv 예매발권 리스트 수신
	vector<rtk_tm_mrsinfo_list_t>	m_vtMrnpList;			

	// [코버스] ui_recv 예매내역 선택 (발권시작 요청)
	UI_RESP_KO_MRS_SEL_DATA_T		m_tUiSelData;
	vector<UI_KO_MRS_SEL_LIST_T>	m_vtUiSelList;		

	// [코버스] tmax_recv 예매발행
	char							rot_rdhm_no_val[100+1];	///> 승차홈번호	
	vector<rtk_tm_mrspub_list_t>	m_vtResComplete;

	vector<TCK_PRINT_FMT_T>		m_vtPrtTicket;

	int MakeMrnpListPacket(int nCount, char *pData);
	int MakeReqIssuePacket(char *pData);
	///> 예매발권 응답데이타 UI 전송 패킷 만들기
	int MakePubMrnpResultPacket(char *pData);
	int MakeTicketPrtData(void);


public:
	static CMrnpKobusMem* GetInstance()
	{
		static CMrnpKobusMem m_instance;

		return &m_instance;
	}

	void Initialize(void);
};

/////////////////////////////////////////////////////////////////////////////////////////////
///< 코버스 - 환불

class CCancRyTkKobusMem
{
public:
	CCancRyTkKobusMem();
	~CCancRyTkKobusMem();

	CANCRY_KOBUS_BASE_T					tBase;

 	stk_tm_ryamtinfo_list_t				tReq;				/// (KSK_021) 티켓 환불 송신
	rtk_tm_ryamtinfo_dt_t				tRespInq;			/// (KSK_021) 응답 데이타
	rtk_tm_ryamtinfo_list_t				tRespList;			/// (KSK_021) 응답 데이타 list
 
	rtk_tm_reptran_t					tRespOk;			/// (KSK_003) (무인기) 환불처리 완료 응답
	rtk_tm_tckcan_t						tRespWndOk;			/// (KSK_022) (창구) 환불처리 완료 응답

public:
	static CCancRyTkKobusMem* GetInstance()
	{
		static CCancRyTkKobusMem		m_instance;

		return &m_instance;
	}

	void Initialize(void);

	int CalcRefund(void);
};


