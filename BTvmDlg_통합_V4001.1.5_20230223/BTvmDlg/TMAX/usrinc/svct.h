
/* --------------------- usrinc/svct.h ------------------------ */
/*								*/
/*              Copyright (c) 2000 - 2004 Tmax Soft Co., Ltd	*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef TMAX_SVCT_H
#define TMAX_SVCT_H

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

#ifndef NULL
#define NULL	0L
#endif

/* ----- svc registeration type ----- */
#define SVC_REG_STATIC		0
#define SVC_REG_UNKNOWN		-1
#define SVC_REG_DYNAMIC		-100

/* ----- svc autotran values ----- */
#define SVC_AUTOTRAN_DFLT	-1
#define SVC_AUTOTRAN_NO		0
#define SVC_AUTOTRAN_YES	1

/* ----- svc timeout values ----- */
#define SVC_TIMEOUT_DFLT	-1
#define SVC_TIMEOUT_INFINITE	0

typedef struct {	/* header from clh to server */
  char  *name;
#ifdef _CBL_MODULE
  int  (*func) (void);
#elif defined(_WIN32)
  void  (__EXPORT *func) (TPSVCINFO *);
#else
  void  (*func) (TPSVCINFO *);
#endif
  int   svci;
  int   autotran;
  int   svctime;
} _svc_t;

#endif
