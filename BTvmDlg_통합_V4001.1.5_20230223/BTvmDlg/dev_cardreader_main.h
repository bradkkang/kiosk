// 
// 
// dev_cardreader_main.h : 신용카드 리더기 main 헤더 파일
//

#pragma once

//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

int CardReader_Polling(void);
int CardReader_GetDeviceInfo(void);
int CardReader_GetStatus(void);

int CardReader_Initialize(void);
int CardReader_Terminate(void);

