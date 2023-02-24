// 
// 
// dev_scanner_main.h : 승차권 리더기 main 헤더 파일
//

#pragma once

//----------------------------------------------------------------------------------------------------------------------
#define DEV_SCANNER_WT			1		/// 위텍 스캐너
#define DEV_SCANNER_ATEC		2		/// 에이텍

//----------------------------------------------------------------------------------------------------------------------

int Scanner_Initialize(void);
int Scanner_Terminate(void);

int Scanner_SetTicketRead(BOOL bRead);
int Scanner_ReadTicket(void);

int Scanner_Reject(void);
int Scanner_Eject(void);

int Scanner_TPHPrint(void);

int Scanner_GetStatus(int *nStatus);
int Scanner_GetDeviceStatus(void);
