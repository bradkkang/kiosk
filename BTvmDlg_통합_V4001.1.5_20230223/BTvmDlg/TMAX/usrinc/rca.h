
/* ------------------------- usrinc/rca.h --------------------- */
/*								*/
/*              Copyright (c) 2002 - 2004 Tmax Soft Co., Ltd	*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef _TMAX_RCA_H
#define _TMAX_RCA_H
#ifndef _TMAX_MTLIB
#define _TMAX_MTLIB     1
#endif
#include <usrinc/tmaxapi.h>

#ifndef _WIN32
#define __EXPORT
#endif

#define	RCAH_ERROR	(-1)
#define	RCAH_TIME_OUT	0
#define	RCAH_TMAX_MSG	1
#define	RCAH_UNSOL_MSG	2
#define RCAH_USER_MSG	3

/* ------ type definition ------ */
typedef struct {
    int fd;
    int idx;
    int portno;
    int count;
    int status1;
    int status2;
#ifdef _WIN32
    HANDLE hThread;
    DWORD  tid;
#else
#ifdef _UXW2_THR	
    thread_t tid;
#else
    pthread_t tid;
#endif
#endif
    void *user_data;
    void *system_data;
} *RCAINFO;


#if defined (__cplusplus)
extern "C" {
#endif

#ifndef _TMAX_KERNEL
int __EXPORT thrmain(RCAINFO);
int __EXPORT thrinit(RCAINFO);
int __EXPORT thrdone(RCAINFO);
int __EXPORT tpsvrinit(char *svrname, int svrn);
int __EXPORT tpsvrdone(char *svrname, int svrn);
#endif

/* ----- rcah API ----------- */
int __EXPORT tpsetfd(int);
int __EXPORT tpclrfd(int);
int __EXPORT tpissetfd(int);
int __EXPORT tpschedule(int);
int __EXPORT tpuschedule(int);
int __EXPORT tpremoconnect(char *, int, int);
int __EXPORT tpgetrcahseqno();
void __EXPORT *tpgetrcainfo();

#if defined (__cplusplus)
}
#endif

#endif
