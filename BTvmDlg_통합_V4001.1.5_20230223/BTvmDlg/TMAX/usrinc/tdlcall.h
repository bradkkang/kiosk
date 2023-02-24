
/* ------------------------- tdlcall.h ------------------------ */
/*								*/
/*           Copyright (c) 2004 Tmax Soft Co., Ltd		*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef _TMAX_TDLCALL_H
#define _TMAX_TDLCALL_H

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

#ifndef _WIN32
#include <dlfcn.h>
#include <unistd.h>
#include <sys/time.h>
#endif
/* ----- tdlcall version  ----- */
#define TDL_MAJOR_VERSION	5
#define TDL_MINOR_VERSION	0
#define TDL_PATCH_VERSION	0

/* ----- tdlcall() return values ----- */
#define TDL_OK			0
#define TDL_OPEN_ERROR		-1	/* dlopen fail */
#define TDL_SYM_ERROR		-2	/* dlsym fail */
#define TDL_CLOSE_ERROR		-3	/* dlclose fail */
#define TDL_SYSTEM_ERROR	-4	/* etc system call error */
#define TDL_INT_ERROR		-5	/* TDL lib internal error */
#define TDL_ENOFUNC		-6	/* funcname not found */
#define TDL_ENV_ERROR		-7	/* environment not found */
#define TDL_VER_ERROR		-8	/* shared version mismatch */
#define TDL_ARG_ERROR		-9	/* invalid arguments */
#define TDL_ENOLIB		-10	/* library not found */
#define TDL_TRAN_ERROR		-11	/* transaction error */
#define TDL_EINACTIVE		-12	/* inactive funcation */
#define TDL_MAX_ERROR		-13	

/* ----- tdlxxxx utility exit values ----- */
#define TDL_EXIT_OK		0
#define TDL_EXIT_EARGS		1	/* invalid argument or environment */
#define TDL_EXIT_ESHM		2	/* shared memory related error */
#define TDL_EXIT_ERUNDIR	3	/* runtime directory related error */
#define TDL_EXIT_EMODDIR	4	/* module directory related error */
#define TDL_EXIT_ELOCK		5	/* rundir lock error */
#define TDL_EXIT_ENOENT		6	/* no such entry */
#define TDL_EXIT_ESYSTEM	7	/* system error */	
#define TDL_EXIT_EOS		8	/* os error */	
#define TDL_EXIT_ERAC		9	/* rac error */	
#define TDL_EXIT_EPERM		10	/* operaiton not permitted */

/* ----- tdlcall() flags values ----- */
#define TDL_NOFLAGS	0x00000000
#define TDL_TRAN	0x00000001

/* ----- tdlclose() flags values ----- */
#define TDLCLOSE_HARD 0x00000001

/* ----- tdlcallva() return type ----- */
#define TDL_VA_CHAR		1
#define TDL_VA_SHORT	2
#define TDL_VA_INT		3
#define TDL_VA_LONG		4
#define TDL_VA_FLOAT	5
#define TDL_VA_DOUBLE	6
#define TDL_VA_PVOID	7
#define TDL_VA_VOID		8

/* ----- misc values ----- */
#ifdef _VERSION_1
#define TDL_FUNCNAME_SIZE       16
#define TDL_FUNCNAME_SIZE_2     8
#else
#define TDL_FUNCNAME_SIZE       64
#define TDL_FUNCNAME_SIZE_2     32
#endif

/* ----- misc macros ----- */
#define dlcall(a, b, c, d)	tdlcall(a, b, c, d)

/* for Openframe */
# define MAX_TRACE_INDEX   2048
# ifndef TMAX_NAME_SIZE
#  define TMAX_NAME_SIZE    16	    /* ref. usrinc/tmadmin.h */
# endif
# ifndef MAX_DBSESSIONID_SIZE
#  define MAX_DBSESSIONID_SIZE 128
# endif

/* for Openframe */
typedef struct {
    int     sec;
    int     msec;
} tdl_time_t;

/* for Openframe */
typedef struct {
    int     gid1;
    int     gid2;
    int     seqno;
} tdl_smgid_t;

/* for Openframe */
typedef struct {
    int	    pid;
    tdl_smgid_t smid; /* Global Id(8), Branch Id(2), Sequence No(2) */
    char    dbsid[MAX_DBSESSIONID_SIZE];
    int     index;
    struct {
	char        type;                     /* 'C':Command, 'M':Module */
	char        type_name[TMAX_NAME_SIZE];
	tdl_time_t   estamp;                 /* Entry Timestamp */
	tdl_time_t   xstamp;                 /* eXit Timestamp */
	tdl_time_t   ecstamp;                /* Entry CPU Timestamp */
	tdl_time_t   xcstamp;                /* eXit CPU Timestamp */
    } trc[MAX_TRACE_INDEX];
} tdl_trace_t;

/* ----- global variables for Openframe ----- */
extern tdl_trace_t *tdltraceinfo;

/* ----- function definitions ----- */
#if defined (__cplusplus)
extern "C" {
#endif

typedef long (*TdlFunc)(void *args);			/* callee function */
typedef long (*TdlFunc2)(int argc, char *argv[]);	/* callee function2 */
typedef long (*TdlFunc3)(void *input, void *output);	/* callee function3 */
int tdlcall(char *funcname, void *args, long *urcode, int flags);
int tdlcall2(char *libname, char *funcname, void *args, long *urcode, int flags);
int tdlcall2v(char *libname, char *funcname, int argc, char *argv[], long *urcode, int flags);
int tdlcall2s(char *libname, char *funcname, void *input, void *output, long *urcode, int flags);
int tdlcallva(char *funcname, long urcode, int flags, int rettype, void *retval, int argc, ...);
int tdlcallva2(char *libname, char *funcname, long urcode, int flags, int rettype, void *retval, int argc, ...);
char* tdlerror(int retval);
unsigned int tdlgetseqno(void);
int tdlstart(void);
int tdlend(void);
int tdlsuspend(void);
int tdlresume(int sd);
int tdlclose(char *name, int flags);
int tdlload(char *funcname, int flags);
int tdlload2(char *libname, char *funcname, int flags);

/* APIs for Openframe */
int tdlinit(int flags);
int tdldone(int flags);
int tdlfind(char *funcname, int flags);
int tdlfind2(char *libname, char *funcname, int flags);
int tdlstat(char *funcname, struct timeval svc_time, struct timeval cpu_time);
int tdlstat2(char *libname, char *funcname, struct timeval svc_time, struct timeval cpu_usrtime, struct timeval cpu_systime);
int tdltrace(int pid, tdl_trace_t *trace, int flags);

#if defined (__cplusplus)
}
#endif

#endif	/* _TMAX_TDLCALL_H */
