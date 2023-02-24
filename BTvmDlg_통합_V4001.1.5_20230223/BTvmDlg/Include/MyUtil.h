// MyUtil.h : Utility function
// 
// 
//

#pragma once

#include "stdafx.h"

#define MY_BCD_DATETIME		1
#define MY_SYS_DATETIME		2

NHS_EXPORT BOOL Util_CloseProcess(CString& strProcessName);

NHS_EXPORT void Util_SystemReboot(int Type);
NHS_EXPORT int Util_Ascii2BCD(char *str, int size, BYTE *pBCD);
NHS_EXPORT void Util_Bin2BCD(DWORD lVal, BYTE *pBCD, int size);
NHS_EXPORT BYTE Util_BinToBcd(BYTE Bin);
NHS_EXPORT DWORD Util_BCD2Bin(BYTE *pBCD, int size);
NHS_EXPORT int Util_BCD2Dec(BYTE bBcd);
NHS_EXPORT void Util_BCD2Ascii(BYTE *dest_msg, BYTE *src_msg, BYTE length_of_src);
NHS_EXPORT DWORD Util_SwapLong(DWORD dwData);
NHS_EXPORT WORD Util_SwapShort(WORD wData);
NHS_EXPORT DWORD Util_Ascii2Long(char *str, int size);
NHS_EXPORT BOOL Util_IsHex2Asc(const BYTE *pbAsc, int nLeng);
NHS_EXPORT int Util_Asc2Hex(const BYTE *pbSrcAsc, int nSrcLeng, BYTE *pbDestBcd);
NHS_EXPORT void Util_Hex2String(char* hex, int hsize, char* asc, int* asize);

NHS_EXPORT int Util_CheckExpire(DWORD dwTick);
NHS_EXPORT void Util_GetLocalTimeBCD(BYTE* retBuf);
NHS_EXPORT int Util_CheckDateTime(int nFormat, BYTE* DateTime);
NHS_EXPORT DWORD Util_TickFromString(char *pDate, char *pTime);
NHS_EXPORT DWORD Util_TickFromDateTime(SYSTEMTIME *pDate);
NHS_EXPORT DWORD Util_TickFromDate(WORD wYear, WORD wMonth, WORD wDay, WORD wHour, WORD wMinute, WORD wSecond);
NHS_EXPORT SYSTEMTIME *Util_DateFromTick(DWORD tick);
NHS_EXPORT DWORD Util_GetCurrentTick(void);

NHS_EXPORT int Util_DebugOut(int nDebug, char *fmt, ...);
NHS_EXPORT int Util_TraceHexaDump(char *pTitle, BYTE *data, int nLen);
NHS_EXPORT char* Util_AddNullString(char *pData, int nDataLen);
NHS_EXPORT void Util_AmountComma(int nAmount, char *retBuf);

NHS_EXPORT int Util_GetModulePath(char *pPath);

NHS_EXPORT int Util_GetIpAddress(CString &strIP);
NHS_EXPORT int Util_GetMacAddress(CString &strMACAddress, CString &strIP);

// convert character 
NHS_EXPORT int Util_Unicode2UTF8(wchar_t *strUni, char *strUtf8);
NHS_EXPORT int Util_Ansi2UTF8(char *szAnsi, char *retUtf8);
NHS_EXPORT int Util_UTF8_2_Ansi(LPCTSTR pUTF8, char* pAnsi);
NHS_EXPORT int Util_Utf8ToAnsi(const char *pszCode, char *retBuf);
NHS_EXPORT int Util_UTF8_2_Unicode(LPCTSTR pUTF8, wchar_t* strUnicode);
NHS_EXPORT int Util_Unicode2Multibyte(wchar_t* strUnicode, char *strMultibyte);
NHS_EXPORT int Util_Multibyte2Unicode(char *strMultibyte, wchar_t* strUnicode);

NHS_EXPORT int Util_MemClear(void *pData, int nSize);
NHS_EXPORT int Util_GetFindCharCount(CString strData, char chChar);
NHS_EXPORT int Util_StringSplit(CString strVal, char chFindChar, CStringArray& retStr);

NHS_EXPORT int Util_MyCopyMemory(void *pSource, void *pTarget, int nLength);
NHS_EXPORT int Util_RunCmd(char *pCommand);
NHS_EXPORT int Util_SetLocalTime(char *pDateTime);

NHS_EXPORT int Util_MonitorControl(int onoff);
