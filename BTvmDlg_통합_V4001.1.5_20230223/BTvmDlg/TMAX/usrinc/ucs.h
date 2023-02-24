
/* ------------------------ usrinc/ucs.h ---------------------- */
/*								*/
/*           Copyright (c) 1999 - 2004 Tmax Soft Co., Ltd	*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef _TMAX_UCS_H
#define _TMAX_UCS_H

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

#define	UCS_ERROR	(-1)
#define	UCS_TMAX_MSG	1
#define UCS_USER_MSG	2

#ifndef _TMAX_RSMT_H
typedef struct {
	long	urcode;
	int	errcode;
	int	msgtype;
	int	cd;
	int	len;
	char	*data;
} UCSMSGINFO;

typedef int (__EXPORT *UcsCallback)(UCSMSGINFO*);
#define CTX_USR_SIZE    256
#ifdef _TMAX_KERNEL
typedef struct ctx_s CTX_T;
#else
typedef struct {
	int   version[4];
	char  data[CTX_USR_SIZE - 16];
} CTX_T;
#endif

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

/* register and unregister monitoring writable fds */
int __EXPORT tpsetfd_w(int fd);
int __EXPORT tpissetfd_w(int fd);
int __EXPORT tpclrfd_w(int fd);

/* register and unregister callback function */
int __EXPORT tpregcb(UcsCallback);
int __EXPORT tpunregcb();

int __EXPORT tpsvrdown();
int __EXPORT tprelay(char *svc, char *data, long len, long flags, CTX_T *ctxp);
CTX_T * __EXPORT tpsavectx();
int __EXPORT tpgetctx(CTX_T *ctxp);
int __EXPORT tpcancelctx(CTX_T *ctxp);

#if defined (__cplusplus)
}
#endif
#endif	/* _TMAX_RSMT_H */


#endif	/* _TMAX_UCS_H */
