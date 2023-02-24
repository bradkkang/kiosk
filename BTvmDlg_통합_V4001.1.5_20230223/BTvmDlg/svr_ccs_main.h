// 
// 
// svr_ccs_main.h : 시외버스 서버 main 헤더 파일
//

#pragma once

//----------------------------------------------------------------------------------------------------------------------

int Svr_IfSv_100(void);
int Svr_IfSv_101(void);

int Svr_IfSv_127(void);
int Svr_IfSv_128(void);

int Svr_IfSv_213(char *pID, char *pPasswd);
int Svr_IfSv_164(void);
int Svr_IfSv_152(char *add_bus_tck_inhr_no);

int Svr_IfSv_135(char *pDate);
int Svr_IfSv_157(char *pDate, char *pTime);
int Svr_IfSv_155(char *pDate, char *pTime);
int Svr_IfSv_136(char *pub_dt, char *pub_shct_trml_cd, char *pub_wnd_no, char *pub_sno);

int Svr_IfSv_130(void);					/// 현장발권 - 배차리스트 요청
int Svr_IfSv_131(void);					/// 현장발권 - 배차요금 조회 
int Svr_IfSv_202(BOOL bSats);			/// 현장발권 - 좌석 선점/해제

int Svr_IfSv_132(void);					/// 현장발권 - 현금 승차권 발행
int Svr_IfSv_134(void);					/// 현장발권 - 현금영수증 승차권 발행
int Svr_IfSv_221(BOOL bReadRotYN);		/// 현장발권 - 신용카드 승차권 발행(KTC)
int Svr_IfSv_222(void);					/// 현장발권 - 현금영수증 승차권 발행(KTC)

int Svr_IfSv_278(void);					/// 현장발권 - QR 좌석 선점 변경 (TK_QrMdPcpySats) // 20221205 ADD
int Svr_IfSv_274(void);					/// 현장발권 - 승차권 발행 요청 - QR결제 전용(KIOSK) (TK_PubTckQr) // 20221017 ADD

int Svr_IfSv_268(void);

int Svr_IfSv_137(void);					/// 예매발권 조회
int Svr_IfSv_138(void);					/// 예매발권

int Svr_IfSv_158(void);					/// 환불 - 버스티켓일련번호
int Svr_IfSv_145(void);					/// 환불 - 버스티켓일련번호

int Svr_IfSv_150(void);

int Svr_IfSv_102(void);
int Svr_IfSv_106(void);
int Svr_IfSv_107(void);
int Svr_IfSv_108(void);
int Svr_IfSv_120(void);
int Svr_IfSv_114(void);
int Svr_IfSv_129(void);
int Svr_IfSv_118(void);
int Svr_IfSv_119(void);
int Svr_IfSv_122(void);
int Svr_IfSv_209(void);
int Svr_IfSv_124(void);
int Svr_IfSv_125(void);
int Svr_IfSv_126(void);

int Svr_IfSv_200(void);
int Svr_IfSv_201(void);
int Svr_IfSv_208(void);

int Svr_IfSv_217(void);
int Svr_IfSv_225(void);
int Svr_IfSv_228(void);
int Svr_IfSv_260(char *pParamData);
int Svr_IfSv_269(void);


int Svr_IfSv_501(void);
int Svr_IfSv_502(char *p_damo_enc_dta);

int Svr_CCS_Initialize(void);
int Svr_CCS_Terminate(void);

