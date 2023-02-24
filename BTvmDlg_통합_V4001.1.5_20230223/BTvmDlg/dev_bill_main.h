// 
// 
// dev_bill_main.h : ÁöÆó main Çì´õ ÆÄÀÏ
//

#pragma once

//----------------------------------------------------------------------------------------------------------------------
#define DEV_BILL_ICT	1				/// Bill ICT ¸ðµ¨
#define DEV_BILL_MEI	2				/// Bill MEI ¸ðµ¨
#define DEV_BILL_ONEP	3				/// Bill ¿øÇÃ·¯½º ¸ðµ¨

//----------------------------------------------------------------------------------------------------------------------

enum _en_print_mode_
{
	BILL_STATE_NONE			= 0	,
	BILL_STATE_RESET			,
	BILL_STATE_ENABLE			,
	BILL_STATE_INHIBIT			,
};

//----------------------------------------------------------------------------------------------------------------------

int Bill_Initialize(void);
int Bill_Terminate(void);

int Bill_Reset(void);
int Bill_GetStatus(void);
int Bill_GetActionStatus(void);
int Bill_Enable(void);
int Bill_Inhibit(void);
int Bill_GetMoney(int *n1k, int *n5k, int *n10k, int *n50k);
int Bill_TotalEnd(void);
