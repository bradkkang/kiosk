// 
// 
// dev_coin_main.h : 동전방출기(100원, 500원) main 헤더 파일
//

#pragma once

//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

int Coin_Initialize(void);
int Coin_Terminate(void);

int Coin_ChangeMoney(int n100, int n500);
int Coin_OutMoney(int n100, int n500, int *nOut100, int *nOut500);

int Coin_Reset(BOOL b100, BOOL b500);
int Coin_GetStatus(void);
int Coin_GetOutInfo(int *n100, int *n500);
int Coin_TotalEnd(void);

int Coin100_GetStatus(void);
int Coin500_GetStatus(void);

