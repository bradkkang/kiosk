// 
// 
// dev_dispenser_main.h : 지폐방출기 main 헤더 파일
//

#pragma once

//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

/**
 * @details		지폐 방출 정보 구조체
 */
typedef struct
{
	int	nOutCount[4];			
	int	nRejectCount[4];		
} DISPENSE_INFO_T, *PDISPENSE_INFO_T;

#pragma pack()

//----------------------------------------------------------------------------------------------------------------------

int CDU_SensorStatus(void);
int CDU_GetStatus(void);
int CDU_GetVersion(void);
int CDU_Dispense(int n1k, int n10k);
int CDU_OutMoney(int n1k, int n10k, int* nOut1k, int* nOut10k);
int CDU_TestDispense(int n1k, int n10k);
int CDU_GetDispenseInfo(char *pData);
int CDU_SetInfo(void);
int CDU_Reset(void);

int CDU_Initialize(void);
int CDU_Terminate(void);

