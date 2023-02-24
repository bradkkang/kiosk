
/* ------------------------ usrinc/usignal.h ------------------ */
/*								*/
/*              Copyright (c) 2000 - 2004 Tmax Soft Co., Ltd	*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef _TMAX_USIGNAL_H
#define _TMAX_USIGNAL_H

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

#define TMAX_DEFER_GET  0
#define TMAX_DEFER_SET  1
#define TMAX_DEFER_INC  2

#if defined (__cplusplus)
extern "C" {
#endif

typedef void __EXPORT Sigfunc(int);

int __EXPORT _tmax_signal_control();
int __EXPORT _tmax_defer_sigs(int method, int value);
Sigfunc *__EXPORT _tmax_signal(int signo, Sigfunc *func);

/*
   Following macros are provided for compatibility.
 */
#define UDEFERSIGS()	_tmax_defer_sigs(TMAX_DEFER_INC, 1)
#define URESUMESIGS()	_tmax_defer_sigs(TMAX_DEFER_INC, -1)
#define USDEFERLEVEL(level)	_tmax_defer_sigs(TMAX_DEFER_SET, level)
#define UGDEFERLEVEL(level)	_tmax_defer_sigs(TMAX_DEFER_GET, 0)
#define UENSURESIGS()	_tmax_defer_sigs(TMAX_DEFER_SET, 0)

#define Usiginit()	_tmax_signal_control()
#define Usignal(signo, func)	_tmax_signal(signo, func)
#define USIGTYP		void

#if defined (__cplusplus)
}
#endif

#endif       /* end of _TMAX_USIGNAL_H  */
