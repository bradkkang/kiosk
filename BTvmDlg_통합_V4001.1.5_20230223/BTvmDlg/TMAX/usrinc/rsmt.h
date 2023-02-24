
/* ------------------------ usrinc/rsmt.h --------------------- */
/*								*/
/*           Copyright (c) 2002 - 2004 Tmax Soft Co., Ltd	*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef _TMAX_RSMT_H
#define _TMAX_RSMT_H
#ifndef _TMAX_MTLIB
#define _TMAX_MTLIB	1
#endif

#ifndef _WIN32
#include <sys/time.h>
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

typedef struct {
	long	urcode;
	int	errcode;
	int	msgtype;
	int	cd;
	int	len;
	char	*data;
} UCSMSGINFO;

typedef int (__EXPORT *UcsCallback)(UCSMSGINFO*);

#if defined (__cplusplus)
extern "C" {
#endif

#ifndef _TMAX_KERNEL
int __EXPORT usermain(int argc, char *argv[]);
#endif
int __EXPORT tpschedule(int sec);
int __EXPORT tpuschedule(int usec);

/* register and unregister monitoring fds */
int __EXPORT tpsetfd(int fd);
int __EXPORT tpissetfd(int fd);
int __EXPORT tpclrfd(int fd);

/* register and unregister callback function */
int __EXPORT tpregcb(UcsCallback);
int __EXPORT tpunregcb();

/* thread related function */
int __EXPORT tmax_thr_create(void *(*func)(void *), void *argp, int flags);
int __EXPORT tmax_thr_terminate(int thrid, int flags);

#if defined (__cplusplus)
}
#endif


#endif	/* _TMAX_RSMT_H */
