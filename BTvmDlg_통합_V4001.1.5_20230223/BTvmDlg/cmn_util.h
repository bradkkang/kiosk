// 
// 
// cmn_util.h : 공통유틸 헤더 파일
//

#pragma once


int IsHangul(char *pData, int nDataLen);
char* GetLanguageStr(int nIndex);
int KTC_MemClear(void *pData, int nSize);
int SubtractValue(int nValue, int nSubValue);
void CMN_MakePasswdChars(char *retBuf, char *pSrc);

