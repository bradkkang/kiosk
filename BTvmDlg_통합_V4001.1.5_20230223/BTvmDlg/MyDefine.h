
#pragma once

//----------------------------------------------------------------------------------------------------------------------

#define CHAR_SOH				0x01    
#define CHAR_STX				0x02    
#define CHAR_ETX				0x03    
#define CHAR_EOT				0x04    
#define CHAR_ENQ				0x05    
#define CHAR_ACK				0x06    
#define CHAR_CR					0x0D
#define CHAR_LF					0x0A
#define CHAR_DLE				0x10    
#define CHAR_XON				0x11
#define CHAR_DC2				0x12
#define CHAR_XOFF				0x13
#define CHAR_NAK				0x15  
#define CHAR_SYN				0x16
#define CHAR_ETB				0x17
#define CHAR_CAN				0x18
#define CHAR_ESC				0x1B
#define CHAR_POL				0x70
#define CHAR_SEL				0x73
#define CHAR_SI					0x0F
#define CHAR_SO					0x0E
#define CHAR_US					0x1F
#define CHAR_RS					0x1E
#define CHAR_CAN				0x18
#define CHAR_ESC				0x1B
#define CHAR_ID					0x30
#define CHAR_SYNC				0xFC
#define CHAR_FA					0xFA
#define PKT_ESCAPE				0xFD
#define PKT_STX					0x7E
#define PKT_XOR					0x40

//----------------------------------------------------------------------------------------------------------------------

///> 버스 서버 구분
#define SVR_DVS_NONE			0x00		/// None
#define SVR_DVS_CCBUS			0x01		/// 시외버스 서버
#define SVR_DVS_KOBUS			0x02		/// 고속버스(KOBUS 고속서버)
#define SVR_DVS_TMEXP			0x04		/// 고속버스(티머니고속)

///> 언어 구분
enum _enKIOSK_LANG_
{
	LANG_DVS_KO		=	0	,				/// 한국어
	LANG_DVS_ENG			,				/// 영어
	LANG_DVS_CHI			,				/// 중국어
	LANG_DVS_JPN			,				/// 일본어
	LANG_DVS_MAX			,
};

///> 버스 구분
enum _enBUS_DVS_
{
	enBUS_DVS_CCS		= 0	,				/// 시외버스
	enBUS_DVS_EXP			,				/// 고속버스
	enBUS_DVS_MAX			,					
};

///> 기능 구분
enum _enFUNC_DVS_
{
	enFUNC_PBTCK		= 0	,				/// 현장발권
	enFUNC_MRS				,				/// 예매발권
	enFUNC_REFUND			,				/// 환불
	enFUNC_MAX				,					
};

/// 시외버스 - 승차권 종류
enum __enCcsTicketKind__ 
{
	CCS_IDX_TCK_IU10	= 0,	//	IU10,  대학생10
	CCS_IDX_TCK_IV50,			//	IV50   보훈50
	CCS_IDX_TCK_IS00,			//	IS00   군후불
	CCS_IDX_TCK_IG00,			//	IG00   어른
	CCS_IDX_TCK_IC50,			//	IC50   아동50
	CCS_IDX_TCK_IV30,			//	IV30   보훈30
	CCS_IDX_TCK_IM20,			//	IM20   중고20
	CCS_IDX_TCK_IF00,			//	IF00   무임
	CCS_IDX_TCK_IV70,			//	IV70   보훈70
	CCS_IDX_TCK_IP20,			//	IP20   봉사20%
	CCS_IDX_TCK_MT02,			//	MT02   월권할증
	CCS_IDX_TCK_MT00,			//	MT00   월권

	CCS_IDX_TCK_IR00,			//	IR00   어른(교통카드)
	CCS_IDX_TCK_IR70,			//	IR70   상주직원RF
	CCS_IDX_TCK_IE50,			//	IE50   상주직원

	CCS_IDX_TCK_IM10,			//	IM10   중고10	// 20221130 ADD
	CCS_IDX_TCK_IM30,			//	IM30   중고30	// 20221130 ADD
	CCS_IDX_TCK_IM50,			//	IM50   중고50	// 20221130 ADD
	CCS_IDX_TCK_IS20,			//	IS20   군후20	// 20221130 ADD

	CCS_IDX_TCK_MAX	 = 30,			
};

/// 코버스 - 승차권 종류
enum __enKobusTicketKind__ 
{
	KOBUS_IDX_TCK_ADULT	= 0		,	//	일반
	KOBUS_IDX_TCK_CHILD			,	//	초등생
	KOBUS_IDX_TCK_TRIP_30		,	//	보훈30
	KOBUS_IDX_TCK_TRIP_50		,	//	보훈50
	KOBUS_IDX_TCK_TRIP_70		,	//	보훈70
	KOBUS_IDX_TCK_FREE			,	//	경로
	KOBUS_IDX_TCK_POSTPAY		,	//	후불권
	KOBUS_IDX_TCK_SOLDIER		,	//	군인20
	KOBUS_IDX_TCK_POSTPAY_20	,	//	후불20
	KOBUS_IDX_TCK_STUDENT_DISC	,	//	학생할인
	KOBUS_IDX_TCK_TEEN			,	//	중고생
	KOBUS_IDX_TCK_FOREIGNER		,	//	할인권
	KOBUS_IDX_TCK_FREEPASS		,	//	프리패스
	KOBUS_IDX_TCK_SEASON		,	//	정기권

	KOBUS_IDX_TCK_MAX			= 30,			
};

/// 티머니고속 - 승차권 종류
enum __enTmExpTicketKind__ 
{
	TMEXP_IDX_TCK_FREE	= 0		,	//	무임
	TMEXP_IDX_TCK_ADULT			,	//	어른
	TMEXP_IDX_TCK_CHILD			,	//	초등생		 
	TMEXP_IDX_TCK_TRIP_30		,	//	보훈30		 
	TMEXP_IDX_TCK_TRIP_50		,	//	보훈50		 
	TMEXP_IDX_TCK_SR_20			,	//	군인20		 
	TMEXP_IDX_TCK_SH			,	//	군후		 
	TMEXP_IDX_TCK_SH20			,	//	군후20		 
	TMEXP_IDX_TCK_UNIV			,	//	대학생		 
	TMEXP_IDX_TCK_STDU			,	//	중고생		 
	TMEXP_IDX_TCK_FPASS			,	//	프리패스	 
	TMEXP_IDX_TCK_FWEEK			,	//	프리주말	 
	TMEXP_IDX_TCK_HANDI			,	//	장애인		 
	TMEXP_IDX_TCK_DISC_20		,	//	할인20		 
	TMEXP_IDX_TCK_STORE_1		,	//	정액권(우등)
	TMEXP_IDX_TCK_STORE_2		,	//	정액권(일반)
	TMEXP_IDX_TCK_SEASON_1		,	//	정기권(우등)
	TMEXP_IDX_TCK_SEASON_2		,	//	정기권(일반)
	TMEXP_IDX_TCK_SEASON		,	//	정기권
	TMEXP_IDX_TCK_OLD			,	//	경로

	TMEXP_IDX_TCK_MAX			= 30,			
};


///> 결제수단
#define PAY_NONE				0x30		/// None
#define PAY_ONLY_CARD			0x31		/// 카드전용
#define PAY_ONLY_CASH			0x32		/// 현금전용
#define PAY_ONLY_RF				0x33		/// 현금전용
#define PAY_CARD_CASH			0x34		/// 현금겸용
#define PAY_CARD_RF				0x35		/// RF

#define enMNPP_BRDT_DVS_CD		0x33		/// 예매자 생년월일 & 예매자 전화번호
#define enMNPP_MRS_NO_DVS_CD	0x34		/// 예매번호
#define enMNPP_ENC_CARD_DVS_CD	0x35		/// 암호화된 카드번호
#define enMNPP_GIFT_DVS_CD		0x36		/// 부가상품권
#define enMNPP_TISSU_NO_DVS_CD	0x37		/// 발권일련번호
#define enMNPP_START_DAY_DVS_CD	0x38		/// 출발일

// 지불수단코드
#define PYM_CD_CASH				'A'			///< 현금
#define PYM_CD_CSRC				'B'			///< 현금영수증
#define PYM_CD_CARD				'C'			///< 신용카드
#define PYM_CD_RF				'D'			///< RF
#define PYM_CD_TPAY				'T'			///< 티머니페이, 티머니비즈페이
#define PYM_CD_ETC				'5'			///< 복합결제(코버스고속)	// 20210910 ADD
#define PYM_CD_COMP				'L'			///< 복합결제(시외)		// 20211015 ADD
#define PYM_CD_QRPC				'Q'			///< QR결제-PAYCO-신용카드[PC](시외)	// 20221017 ADD
#define PYM_CD_QRPP				'R'			///< QR결제-PAYCO-포인트[PP](시외)	// 20221124 ADD
#define PYM_CD_QRTP				'U'			///< QR결제-티머니페이[TP](시외)		// 20221124 ADD

#define LANG_KOR				0
#define LANG_ENG				1
#define LANG_JPN				2
#define LANG_CHINA				3

// 20230206 DEL~ // 20230216 MOD~
#define PAPER_ROLL				0x31		///< 감열지
#define PAPER_TICKET			0x32		///< 승차권
// 20230206 ~DEL // 20230216 ~MOD
/* // 20230206 MOD~ // 20230216 DEL~
#define PAPER_ROLL				0x32		///< 감열지
#define PAPER_TICKET			0x31		///< 승차권
*/ // 20230206 ~MOD // 20230216 DEL
//----------------------------------------------------------------------------------------------------------------------

#define _KC_CERTI_		1		///< KC 인증

//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

typedef struct  
{
	int n10;
	int n50;
	int n100;
	int n500;
} COIN_DATA_T;

typedef struct  
{
	int n1k;
	int n5k;
	int n10k;
	int n50k;
} BILL_DATA_T;

typedef struct  
{
	int	nCount;
	DWORD dwMoney;
} ACCOUNT_FIELD_T, *PACCOUNT_FIELD_T;

#pragma pack()




//----------------------------------------------------------------------------------------------------------------------

