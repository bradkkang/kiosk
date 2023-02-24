// 
// 
// svr_tm_exp_main.h : 티머니 고속 서버 main 헤더 파일
//

#pragma once

//----------------------------------------------------------------------------------------------------------------------

void TmExp_MakeHeader(char *pData, BOOL b_req_user_no = FALSE);

/// 기초정보
int TmExp_CM_ReadMsg(int nOperID);
int TmExp_CM_ReadNtc(int nOperID);
int TmExp_TK_AuthCmpt(int nOperID);
int TmExp_CM_ReadCmnCd(int nOperID);
int TmExp_TK_ReadTckPrtg(int nOperID);
int TmExp_MG_ReadTrmlDrtn(int nOperID);
int TmExp_TK_ReadOwnrTrml(int nOperID);
int TmExp_CM_ReadTrml(int nOperID);
int TmExp_CM_ReadTrmlInf(int nOperID);
int TmExp_CM_ReadTrmlStup(int nOperID);
int TmExp_MG_ReadWnd(int nOperID);
int TmExp_CM_ReadRtrpTrml(int nOperID);
int TmExp_CM_ReadTckKnd(int nOperID);
int TmExp_CM_MngCacm(int nOperID);
int TmExp_CM_ReadRotInf(int nOperID);
/// ~기초정보

int TmExp_ChangeTicketBox(char *pbus_tck_inhr_no);
int TmExp_TK_CloseWnd(void);

/// 현장발권
int TmExp_MG_ReadAlcn(void);
int TmExp_CM_ReadSatsFee(void);
int TmExp_TK_PcpySats(void);
int TmExp_TK_PcpySatsCancel(void);

int TmExp_TK_PubTckCash(void);
int TmExp_TK_PubTckCardKTC(void);
int TmExp_TK_PubTckCsrcKTC(void);
int TmExp_TK_PubTckPrd(void);
int TmExp_CM_ReadThruTrml(void);
int TmExp_CM_ReadRyrt(int nOperID);


/// ~현장발권

/// 예매발권
int TmExp_TK_ReadMrs(void);
int TmExp_TK_KtcReadMrs(void);
int TmExp_TK_PubMrs(void);
int TmExp_TK_PubMrsMobile(void);
/// ~예매발권

/// 환불
int TmExp_TK_ReadBusTckno(BOOL bFirst);
int TmExp_TK_CancRyTck(void);
/// ~환불

/// 재발행
int TmExp_TK_RePubTck(void *pData);

int TmExp_TK_PubPtInquiry(char *pDate);
int TmExp_TK_RePubPtInquiry(char *pDate);
int TmExp_TK_CanRyPtInquiry(char *pDate);


int TMExp_Initialize(void);
int TMExp_Terminate(void);

