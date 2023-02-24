
/* ---------------------- usrinc/WinTmax.h -------------------- */
/*								*/
/*              Copyright (c) 2000 - 2004 Tmax Soft Co., Ltd	*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef _TMAX_WINTMAX_H
#define _TMAX_WINTMAX_H

#include        <process.h>
#include        <winsock2.h>
#include        <windows.h>

#ifdef __EXPORT
#undef __EXPORT
#endif
#ifdef _TMAX4GL
#define __EXPORT __stdcall
#else
#define __EXPORT __cdecl
#endif

#if defined(__cplusplus)
extern "C" {
#endif
int __EXPORT WinTmaxSend(int recvContext, 
	char *svc, char *data, long len, long flags);
int __EXPORT WinTmaxSetContext(HANDLE winhandle, 
	unsigned int msgtype, int slot);
int __EXPORT WinTmaxStart(TPSTART_T *tpinfo);
int __EXPORT WinTmaxEnd(void);
#if defined(__cplusplus)
}
#endif

#endif
