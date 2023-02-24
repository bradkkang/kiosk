// 
// 
// svr_ko_main.h : 고속버스(코버스) 서버 main 헤더 파일
//

#pragma once

//----------------------------------------------------------------------------------------------------------------------

int Kobus_CM_ReadMsg(int nOperID);
int Kobus_CM_ReadNtc(int nOperID);
int Kobus_TK_AuthCmpt(void);
int Kobus_CM_ReadCmnCd(int nOperID);
int Kobus_CM_RdhmInqr(int nOperID);
int Kobus_TK_ReadTckPrtg(int nOperID);
int Kobus_CM_ReadTrmlInf(int nOperID);
int Kobus_MG_ReadWnd(int nOperID);
int Kobus_CM_ReadRtrpTrml(int nOperID);
int Kobus_CM_ReadTckKnd(int nOperID);
int Kobus_CM_MngCacm(int nOperID);

int Kobus_CM_ReadRotInf(void);


// 예매조회
int Kobus_TK_ReadMrs(void);
// 예매발행
int Kobus_TK_PubMrs(void);

// 배차조회
int Kobus_MG_ReadAlcn(void);
// 좌석조회
int Kobus_CM_ReadSatsFee(void);
// 좌석선점/해제
int Kobus_TK_PcpySats(void);
int Kobus_TK_PcpySatsCancel(void);
// 현장발권
int Kobus_TK_PubTck(void);

// 환불 대상 금액 조회
int Kobus_TK_RyAmtInfo(void);

// 환불 처리 - 무인기 발권한거
int Kobus_TK_RepTran(void);
// 환불 처리 - 창구 발권한거
int Kobus_TK_TckCan(void);

int Kobus_TK_ChangeTicketBox(char *pbus_tck_inhr_no);
int Kobus_TK_CloseWnd(void);
int Kobus_TK_InqPubPt(char *pDate);
int Kobus_TK_RePubTck(char *pub_dt, char *pub_trml_cd, char *pub_wnd_no, char *pub_sno);

int Kobus_Initialize(void);
int Kobus_Terminate(void);

