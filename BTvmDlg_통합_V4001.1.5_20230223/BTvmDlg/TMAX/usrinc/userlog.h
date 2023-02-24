
/* ------------------------- usrinc/userlog.h ----------------- */
/*								*/
/*              Copyright (c) 2000 - 2004 Tmax Soft Co., Ltd	*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef _TMAX_USERLOG_H
#define _TMAX_USERLOG_H

#ifndef _CE_MODULE
#include <time.h>
#endif

#ifdef __EXPORT
#undef __EXPORT
#endif
#ifdef _WIN32
#ifdef _TMAX4GL
#define __EXPORT __stdcall
#else
#define __EXPORT __cdecl
#endif
#else
#define __EXPORT
#endif

#if defined (__cplusplus)
extern "C" {
#endif

int __EXPORT userlog(const char *fmt, ...);
int __EXPORT UserLog(const char *fmt, ...);
#ifndef _CE_MODULE 
int __EXPORT userlog2(char *data, int len);
#endif
int __EXPORT ulogsync();

#if defined (__cplusplus)
}
#endif

#endif
