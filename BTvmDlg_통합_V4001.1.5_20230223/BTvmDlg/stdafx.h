
// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이 
// 들어 있는 포함 파일입니다.

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#endif

#include "targetver.h"

//#include <vld.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 일부 CString 생성자는 명시적으로 선언됩니다.

// MFC의 공통 부분과 무시 가능한 경고 메시지에 대한 숨기기를 해제합니다.
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 핵심 및 표준 구성 요소입니다.
#include <afxext.h>         // MFC 확장입니다.


#include <afxdisp.h>        // MFC 자동화 클래스입니다.



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // Internet Explorer 4 공용 컨트롤에 대한 MFC 지원입니다.
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // Windows 공용 컨트롤에 대한 MFC 지원입니다.
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC의 리본 및 컨트롤 막대 지원


#include <afxsock.h>            // MFC 소켓 확장




#include <iostream>
#include <algorithm>



#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

/// my define
#define NHS_EXPORT extern "C" __declspec(dllexport)
#define NHS_AFX_EXPORT	AFX_EXT_CLASS
//#define NHS_AFX_EXPORT	__declspec(dllimport)
#define UNUSED_VAR(VAR)		VAR

//#define MAIN_VERSION	"1001"
//#define MAIN_VERSION	"1002"
//#define MAIN_VERSION	"1003"
//#define MAIN_VERSION	"1004"
//#define MAIN_VERSION	"1005"			// ui 통신 패킷길이 2바이트 -> 4바이트로 변경
//#define MAIN_VERSION	"1006"			// 코버스 fdl 파일 변경
//#define MAIN_VERSION	"1007"			// 한전금 디바이스 porting, 인천공항 기능 추가
//#define MAIN_VERSION	"2001_test"		// 금호터미널 통합 S/W
//#define MAIN_VERSION	"2005"			// 금호터미널 통합 S/W
//#define MAIN_VERSION	"2006"			// 금호터미널 통합 S/W 설치 (11/08)
//#define MAIN_VERSION	"2007"			// DAMO 에러 디버깅
//#define MAIN_VERSION	"2008"			// DAMO 에러 디버깅
//#define MAIN_VERSION	"2009"			// DAMO 에러 수정완료
//#define MAIN_VERSION	"2010"			// 휠체어좌석 UI 프로토콜 수정
//#define MAIN_VERSION	"2011"			// 시외 - 배차방식, 좌석제유무 프로토콜 추가
//#define MAIN_VERSION	"2012"			// 미방출 초기화 작업, damo 수정.
//#define MAIN_VERSION	"2013"			// damo 완벽 해결..
//#define MAIN_VERSION	"2014"			// 카드전용 리부팅..
//#define MAIN_VERSION	"2015"			// 자동마감 리부팅 플래그 적용, 현장발권메인에서 창구마감됨.
//#define MAIN_VERSION	"2016"			// 배차조회:발권가능여부N 빼기, 
//#define MAIN_VERSION	"2017"			// 자동마감 시 프린트 유무, 시외 환불인 경우 승차금액이 100원 미만 창구로 유도, 출금시 현금사용유무 추가, 시재출금 시 250개 단위로 쪼갬
//#define MAIN_VERSION	"2018"			// 지폐방출기 - 미방출 오류 수정.
//#define MAIN_VERSION	"2019"			// ui 프로토콜 수정, 106, 502, 311
//#define MAIN_VERSION	"2020"			// 인천공항 T1, T2 Release, 카드전용 환불 추가, 코버스 배차리스트에 배차일자 필드 추가
//#define MAIN_VERSION	"3000"			// 인천공항 T1, T2 Release, 예매발권_노선번호 추가
//#define MAIN_VERSION	"3001"			// 고속버스 환불 적용, 마산/창원 적용
//#define MAIN_VERSION	"3002"			// 시외 환불 수정. (재발행 승차권 환불, 출발시간 5분 후)
//#define MAIN_VERSION	"3002.2"		// 승차권포맷(출발일자/출발시간) 맞춤, 인천공항 RF 설정정보 추가
//#define MAIN_VERSION	"3003"			// 예매리스트 포맷 바꿈, 과거 예매내역 옵션 적용, BillICT 모듈 변경
//#define MAIN_VERSION	"3004.1"		// TR State Logic 수정.
//#define MAIN_VERSION	"3004.2"		// 상주직원 로그인 실패시 다운 수정.
//#define MAIN_VERSION	"3004.3"		// RF선불 프로토콜 추가
//#define MAIN_VERSION	"3004.4"		// RF 결제 실패시 좌석선점 해지, 승차권프린트 현금영수증 부분 수정.
//#define MAIN_VERSION	"3004.5"		// RF 회계정보 수정
//#define MAIN_VERSION	"3004.6"		// 인천공항T2 출발시간 출력
//#define MAIN_VERSION	"3004.7"		// INI 프린터옵션 버퍼 수정, 고속 현금영수증 수기부분 수정.
//#define MAIN_VERSION	"3004.8"		// 구매자 자릿수 수정, 코버스 배차요금(우등등급 시외우등뒷좌석할인요금, 심야우등 시외우등뒷좌석할인요금) 필드 추가
//#define MAIN_VERSION	"3004.9"		// 현금:출발시간, 좌석번호 출력여부 수정.
//#define MAIN_VERSION	"3005.0"		// 고속(지출증빙) 수정
//#define MAIN_VERSION	"3005.1"		// 금호터미널 선착순 출력부분은 기존과 동일하게 원복
//#define MAIN_VERSION	"3005.2"		// 코로나 19 개인정보 입력 추가
//#define MAIN_VERSION	"3005.3"		// 경기도민 전용 서비스 추가(차량구분) - 임시로 space 값
//#define MAIN_VERSION	"3005.4"		// 경기도민 전용 서비스 추가(차량구분) - 정상값
//#define MAIN_VERSION	"3005.5"		// 경기도민 전용 서비스 추가(차량구분) - 전화번호값이 없을 경우, dupkt 함수 안하게 수정
//#define MAIN_VERSION	"3005.6"		// 신규방출기 프로토콜 추가
//#define MAIN_VERSION	"3005.7"		// 지폐입금 사용유무 관련 ini 파일 수정.
//#define MAIN_VERSION	"3005.8"		// 코버스:배차노선번호 추가, 감열지 승차권발권 추가
//#define MAIN_VERSION	"3005.9"		// 감열지 승차권 발권 필드 수정
//#define MAIN_VERSION	"3006.0"		// 감열지 승차권 간격 옵션처리함.- env_opt.ini 간격 필드 추가
//#define MAIN_VERSION	"3006.1"		// 감열지 승차권 출력시, 발권매수 증가 안함, 영수증프린터 좌측 간격 재설정.
//#define MAIN_VERSION	"3006.2"		// 감열지 승차권 부족 Near-End로 체크
//#define MAIN_VERSION	"3006.3-1"		// 예매발권 시, 티머니페이, 티머니비즈페이 결제수단 추가
										// 신규 방출기 오류 시 장애 발생 수정, 지폐 미방출 수정
										// 코버스 - 환승지 배차정보로 수정
//#define MAIN_VERSION	"3006.3.5"		// 승차권 잔량 수정.
//#define MAIN_VERSION	"3006.3.6"		// (코버스) 배차출발지명을 발권응답값으로 처리.
//#define MAIN_VERSION	"3006.3.7"		// DF
//#define MAIN_VERSION	"3007.0"		// DF8 포멧(광주, 인천, 전주, DF3) 적용, 화성프린트 신규 프로토콜 적용
//#define MAIN_VERSION	"3007.1"		// 지폐미입금 -> 입금인식되는부분 수정 (대전복합장애관련)
//#define MAIN_VERSION	"3007.2"		// TPAY 환불 관련 `카드환불 로직 처리로 수정
//#define MAIN_VERSION	"3007.3"		// 화성 뉴프로토콜 적용후 연속발매시 쓰래기값 인쇄되는것 수정 (딜레이 -> 발권) --> (발권 -> 딜레이) 로변경 3000ms -> 3500ms
//#define MAIN_VERSION	"3007.4"		// ENV_OPT.INI에서 setbold 적용 되도록 수정
//#define MAIN_VERSION	"3007.5"		// 금호 배차, 비좌석제 오류 수정
//#define MAIN_VERSION	"3007.6"		// 전주 shrt_pub_time -> pub_time 변경 
//#define MAIN_VERSION	"3007.7"		// HS_NP 실시간 상태체크 적용
//#define MAIN_VERSION	"3007.8"		// HS_NP timeout, txrxdata retry 추가
//#define MAIN_VERSION	"3007.9"		// 디바이스 실시간 체크 추가, 코버스 출발일자 추가,
//#define MAIN_VERSION	"3007.9.1"		// 디바이스 실시간 체크 추가, 코버스 출발일자 제외, 테스트용
//#define MAIN_VERSION	"3008.0"		// 페이 예매시 결제구분 페이로 인쇄
//#define MAIN_VERSION	"3008.1"		// 디바이스 체크 수정
//#define MAIN_VERSION	"3008.2"		// 인쇄포멧 추가 시외 짧은버스등급 ex) 프리미엄 -> 프리, 우등고속 -> 우고 등
//#define MAIN_VERSION	"3008.3"		// 한전금 영수증프린트 오류 수정(페이), eventflag 분류 수정 (bill -> cdu)
//#define MAIN_VERSION	"3008.4"		// 군후불 환불 불가 수정 (trmem calc refund)
//#define MAIN_VERSION	"3008.5"		// 재발행 로직 수정
//#define MAIN_VERSION	"3008.6"		// 티머니고속 예매발권 pyn_dtl_cd 추가
//#define MAIN_VERSION	"4000.1"		// 티머니고속 서비스 최초 시작(충주)..
//#define MAIN_VERSION	"4000.2"		// 티머니고속 - 재발행 기능 수정
//#define MAIN_VERSION	"4000.2"		// 티켓발매 20매 이상 시 오류 수정 : AddSmsTRData() SMS_TR_DATA_T 수정
//#define MAIN_VERSION	"4000.3.1"		// 자사향스캐너 S/W 튜닝. 
//#define MAIN_VERSION	"4000.3.2"		// 환불 오류시 자사향 스캐너 승차권 투입오류 수정.
//#define MAIN_VERSION	"4000.3.3W"		// 환불 오류시 자사향 스캐너 승차권 투입오류 수정.
//#define MAIN_VERSION	"4000.3.4W"		// KTC 요구사항 수정:발권완료, 티머니고속 예매발권 수정:인터넷+모바일예매
//#define MAIN_VERSION	"4000.3.5W"		// KTC 요구사항 수정:card_no 로그
//#define MAIN_VERSION	"4000.3.5W.2"	// 티머니고속 - 운송회사명이 긴 경우 ".." 처리 : dev_prt_ticket_nhs.cpp/h : 승차권 프린터 (화성) 
										// 티머니고속 예매발권 좌석번호 중복 출력 오류 수정 : dev_tr_tmexp_mem.cpp // 20210405, App 이상종료로 NhCmmn.dll 수정 : MyDataTr.h // 20210408
//#define MAIN_VERSION	"4000.3.5W.4"	// 이비카드 발권 티켓 환불 불가, 시외서버에 ‘승차권번호’ 추가 프로토콜 수정적용 : svr_ccs_main.cpp // 20210511
//#define MAIN_VERSION	"4000.3.5W.5"	// 창구마감 1일 1회 처리 옵션 추가 : oper_config.cpp, svr_ko_main.cpp dev_ui_main.cpp // 20210513 
//#define MAIN_VERSION	"4000.3.5W.6"	// 코버스 인터페이스 변경(보훈권종 추가)(KSCC-TSK5-SVC-DE-06_인터페이스설계서_무인발권기-V1.86-20210513) // 20210614
//#define MAIN_VERSION	"4000.3.5W.7"	// 자동창구마감 기능 1회마감옵션 'N'일때 오류 수정 : oper_config.cpp // 20210709
//#define MAIN_VERSION	"4000.3.6"		// 자동 배포용 버전 번호 // 20210716
//#define MAIN_VERSION	"4000.3.7"		// 환불 조회 시, 인터페이스 입력값 서버(코버스) 오류 수정(USER_NO(무인기 번호) 만 입력됨) : svr_ko_main.cpp, dev_tr_cc_cardcash.cpp // 20210723
										// 자동창구마감 초기값, 관리자 옵션, INI 설정값 저장 중복 오류, INI 설정값 로그 추가 : oper_config.cpp // 20210728
//#define MAIN_VERSION	"4000.3.8"		// 코버스 자동창구마감 작업구분코드 추가(KSCC-TSK5-SVC-DE-06_인터페이스설계서_무인발권기-V1.87-20210805) : svr_ko_main.cpp, svr_kobus.cpp// 20210810
//#define MAIN_VERSION	"4000.3.9"		// 발권 처리 중 오류 수정 : dev_tr_cc_cardcash.cpp // 20210817
//#define MAIN_VERSION	"4000.3.10"		// 환불 처리 중 오류 수정 : dev_tr_cc_cardcash.cpp // 20210823
//#define MAIN_VERSION	"4000.3.11"		// 금호터미널 중 배차/비좌석제에서 승차권에 출발시간 출력 : dev_prt_ticket_nhs.cpp // 20210826
//#define MAIN_VERSION	"4000.3.12"		// [시외] 승차권 회수용에 QR 출력 // 20211014 
//#define MAIN_VERSION	"4000.3.13"		// [시외](영어)아동50;Children,중고20;Youth,[코버스고속](다국어)학생할인;대학생 문구 변경 : svr_ccbus.cpp, svr_kobus.cpp, constmsg.xml // 20211006
//#define MAIN_VERSION	"4000.3.14"		// 티머니고속 '환불율 구분코드' 송신 시 오류 수정 : svr_tm_exp_main.cpp //20211018
//#define MAIN_VERSION	"4000.3.15"		// 티머니GO 2.0 개발 관련 복합결제 인터페이스 변경
										// (1) 환불확인증 문구 변경 : dev_prt_hs.cpp, dev_prt_nice_rxd.cpp // 20210910
										// (2) 코버스고속 (KSCC-TSK5-SVC-DE-06_인터페이스설계서_무인발권기-V1.88-20210823) // 20210910
										//     -> RMN_APRV_AMT 추가 (KSCC-TSK5-SVC-DE-06_인터페이스설계서_무인발권기-V1.90-20211019) // 20211019
										// (3) 티머니고속 (TMONEY-EZS-DE06-인터페이스설계서_v1.13.xlsx) : svr_tm_expbus.cpp, dev_tr_tmexp_mem.cpp // 20211013
										// (4) 시외 (시외버스_인터페이스설계서(복합결제...)_20210928.xlsx)  : // 20211015
//#define MAIN_VERSION	"4000.3.16"		// 테스트발권 기능 추가 // 20211116
//#define MAIN_VERSION	"4000.3.17"		// AmountComma 추가 for 시외버스, 코버스, 티머니고속 : dev_prt_main.cpp // 20211125
										// AmountComma 추가 for 테스트 승차권 출력 : dev_prt_ticket_nhs.cpp // 20211125
										// 배차구분/좌석제에 따른 출발시간/좌석 표시 옵션별 승차권 출력 : dev_prt_ticket_nhs.cpp // 20211203
										// 현장테스트발권 티켓잔여수량 // 20220104
//#define MAIN_VERSION	"4000.3.18"		// [시외]휠체어 인터페이스 추가 (XZZ-D-SV-휠체어인터페이스설계서_V12.0_20211112) // 20211206
										// 지폐인식기 장애 오류 표출 내용 수정 // 20220127
//#define MAIN_VERSION	"4000.3.19"		// 인천공항에서 해외카드 사용 시 시외버스 승차권 결제수단 항목 표시문구 변경  "해외카드(수수료율%)"표시 // 20220324
//#define MAIN_VERSION	"4000.3.20"		// 'N7000','N4000' 형식의 노선명에서 노선번호 가져오기, 승차권 출력 추가 // 20220429
//#define MAIN_VERSION	"4000.3.20D"	// 모니터링 기능 추가 // 20220110
										// 승차권부족 Warnning/Error 구분 // 20220310
//#define MAIN_VERSION	"4000.3.21"		// Kobus 창구마감 TAK_STT_DT 값 설정 // 20220531
//#define MAIN_VERSION	"4000.3.22D"	// 현금영수증-수기 및 현금영수증-카드의 승차권데이터 작성 시 출발시각이 아닌 배차출발시각 표시 오류 발생 수정	// 20220622
//#define MAIN_VERSION	"4000.3.23D"	// 발권속도 개선 // 20220706
//#define MAIN_VERSION	"4000.3.24"		// 인천공항T1, T2 배차(D)-비좌석제(N)와 INI파일의 2/Y/N 설정 기준 
										// 통전망 배차설정 (출발시간출력여부) 및 (좌석번호출력여부)에 따른 승차권 인쇄사양 변경 // 20220707
										// [Kobus] (KSCC-TSK5-SVC-DE-06_인터페이스설계서_무인발권기-V1.91-20220707) - 후불권매수, 후불총금액 필드 추가	// UI_4001.1.22 ~	// 20220713
//#define MAIN_VERSION	"4000.3.25"		// 티머니고속 '왕복 가능 터미널 정보 조회' 오류 처리 수정 // 20220825
//#define MAIN_VERSION	"4000.3.26"		// 배차조회 화면에서 비배차 시간 표시 INI 옵션 적용 // 20220902
//#define MAIN_VERSION	"4000.3.27"		// 배차조회 화면에서 비배차 시간 표시 INI 옵션 적용 (oper_config.dat 오류 발생) 시 구조체 MAX 버퍼 값 수정 // 20220915
//#define MAIN_VERSION	"4000.3.28"		// 무인기 관리자 DefaultConfigFile 설정값 변경, 예매번호 조회 : 사용(Y), (5만원이상) 카드할부 사용유무 : 미사용(N)	// 20220922
										// 성남터미널 요청사항 - 승차권(승객용)에 창구용 바코드와 같이 출력 요청	// 20221012
//#define MAIN_VERSION	"4000.3.29"		// 창구(마감)시작일자 오류 수정 // 20220913									
//#define MAIN_VERSION	"4000.3.30"		// (UI 4001.1.31 호환) 
										// 카드승차권 승인취소 문구 추가 // 20221129
										// 티켓종류 추가 (IM10, IM30 등) // 20221130
//#define MAIN_VERSION	"4000.3.31"		// XZZ-D-SV-인터페이스설계서_V12.7-'환불-버스티켓일련번호 조회-좌석제사용여부' 추가-비좌석제 환불수수료 적용 옵션 // 20221201
										//								-'버스티켓 재발행-QR결제지불상세코드' 추가 // 20221201
										// 코버스 고속 창구 마감 오류 수정 - TMaxFBPut(TAK_DVS_CD) 부분 추가 // 20221202
//#define MAIN_VERSION	"4000.3.32"		// XZZ-D-SV-인터페이스설계서_V12.8(매표수정내용)-'위약금 타입,QR결제지불상세코드'->환불확인증 결제수단 출력내용에 QR방식 추가 // 20221220
										// 감열지승차권 QR코드크기 Ver3(x03) 으로 변경요청 (마산고속) // 20221220
//#define MAIN_VERSION	"4001.1.1"		// Ver.4000.3.28 + QR결제기능개발
										// (UI 4002.1.1 이상 호환)
										// PAYCO QR 결제 기능 추가 // 20221017  
										// QR 결제 구분 추가 // 20221124
										// XZZ-D-SV-인터페이스설계서_V12.5(매표) // UI_4002.0.0 이상 호환 // 20221017
										// MyWriteFile Error 수정 (NhCmmn\MyFileUtil.cpp) // 20220914 	
//#define MAIN_VERSION	"4001.1.2"		// Ver "4000.3.29"~"4000.3.32" + "4001.1.1"							
										// (Main 4001.1.2_20221229 + UI 4002.1.1_20221229 이상 호환)
										// XZZ-D-SV-인터페이스설계서_V12.7, XZZ-D-SV-인터페이스설계서_V12.8(매표수정내용)
										// _V12.7 - '환불-버스티켓일련번호 조회-좌석제사용여부' 추가-비좌석제 환불수수료 적용없음 옵션 // 20221201
										// _V12.7 - '버스티켓 재발행-QR결제지불상세코드' 추가 // 20221201
										// _V12.7 - 'QR 좌석 선점' 추가 // 20221205
										// [시외-인천공항] 배차정보설정값(통전망서버)에 따라 시간/선착순 문구 표시 수정 // 20221207
										// [시외-인천공항] 상주직원정보 승차권 출력 // 20221212
										// XZZ-D-SV-인터페이스설계서_V12.8(매표수정내용) - 조회 시, 'QR결제지불상세코드' 추가 // 20221208
										// '환불 - 승차권 읽기 결과(IF_UI_402)(MAIN->UI)'-'QR결제지불상세코드' 추가 // 20221221
										// '환불 - 승차권 읽기 결과(IF_UI_402)(MAIN->UI)'-'위약금 타입' 추가 // 20221223
										// 승차권 회수용 바코드 높이 변경 // 20221226
										// 승차권 발행 - QR결제(TK_PubTckQr) QR결제지불상세코드(QR_PYM_PYN_DTL_CD) (V12.6) 처리 추가 // 20221228
										// [코버스고속] 휴게소 도착지명 수정 // 20221229
										// [티머니고속] 환불-결제수단 추가 // 20230104
//#define MAIN_VERSION	"4001.1.3"		// 인천공항_T2 한정면허 환불율 적용-환불수수료 위약금 타입 적용(A:시외버스, B:인천공항리무진) // 20221222
										// 인천공항_T2 상주직원 무인기에서 환불 불가 처리 추가 // 20230106
										// CBR_19200 -> 115200 으로 변경 // 20230119 -> // 20230131 반영 보류 
										// 감열지_영수증프린터 상태 체크 확인 //20230203 // 20230206 // 20230215 
//#define MAIN_VERSION	"4001.1.4"		// [마산시외] ENV_OPT.INI([Y][Y])일지라도 배차설정값이 출발시간출력(N), 좌석번호출력(N) 인 경우 시간출력 안하도록 수정 // 20230202
										// [고속] 감열지 승차권에 현금(소득공제) 출력내용 추가, 승차권 인쇄용지 정보 SetConfigTicketPapaer 함수 추가 // 20230206 
										// 감열지_영수증프린터 상태 체크 수정(for 한전금 무인기 Hs타입) // 20230215
										// 감열지_영수증프린터 상태 체크 수정(for 한전금 무인기 Rxd타입) // 20230206 // 20230215 //20230216
										// 프린터 종류 정의값을 UI 와 맞춰 다시 원복 // 20230216
//#define MAIN_VERSION	"4001.1.4D"		// 감열지_영수증프린터 상태 체크 임시 수정 // 20230217

#define MAIN_VERSION	"4001.1.5"		// [고속]감열지_영수증프린터 설정값 오류 수정 // 20230220
										// RF교통카드 오류 서버응답 발권실패 시 처리 수정 // 20230222
										// 운영사(2,3)에서 '배차비좌석/비배차'의 경우  시간,선착순 출력내용 수정 // 20230223 

//#define ATEC_POS_VERSION		"KSCCICB-A1000"
#define ATEC_POS_VERSION		"CTBUS-KSKZ001"

#define _KTC_CERTIFY_			1			// ktc 인증
//#define _KTC_CERTIFY_			0

//#define __KTC_PROGRAM__		1			// ktc 인증 프로그램..

//#define _USE_ATEC_SCANNER_	1			// ATEC 스캐너
#define _USE_ATEC_SCANNER_		0			// 위텍 스캐너

#define _SVR_DATA_DOWNLOAD_		1			// 무조건 다운로드
//#define _SVR_DATA_DOWNLOAD_	0			// 파일있으면 다운로드안함

//#define __USE_TMAX__			0
#define __USE_TMAX__			1
#define __DEV_ENV__				1

//#define __DEV_TR_SPLIT__		1

//---------------------------------------------------------------------------------------

#define BILL_SUM(a,b,c,d)		(a * 1000) + (b * 5000) + (c * 10000) + (d * 50000)
#define BILL_1K_SUM(a)			(a * 1000)
#define BILL_5K_SUM(a)			(a * 5000)
#define BILL_10K_SUM(a)			(a * 10000)
#define BILL_50K_SUM(a)			(a * 50000)

#define COIN_SUM(a,b,c,d)		(a * 10) + (b * 50) + (c * 100) + (d * 500)
#define COIN_10_SUM(a)			(a * 10)
#define COIN_50_SUM(a)			(a * 50)
#define COIN_100_SUM(a)			(a * 100)
#define COIN_500_SUM(a)			(a * 500)

