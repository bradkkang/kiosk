
/* ------------------------ tuxinc/atmi.h --------------------- */
/*								*/
/*              Copyright (c) 2002 Tmax Soft Co., Ltd		*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef _TMAX_TUXATMI_H
#define _TMAX_TUXATMI_H

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

#if !defined(_TMAX_KERNEL) && !defined(_TMAX_UTIL)

#define _(a) a

/* Flags to tpinit() for Tuxeedo compatability */
#define TPU_MASK	0x00000007	
#define TPU_SIG		0x00000001	
#define TPU_DIP		0x00000002	
#define TPU_IGN		0x00000004	
#define TPSA_FASTPATH	0x00000008	
#define TPSA_PROTECTED	0x00000010	
#ifndef _TMAX_ATMI_H
#if defined(_TMAX_MTLIB) || defined(_MCONTEXT)
#define TPMULTICONTEXTS 0x00000040
#endif
#endif

#ifndef _TMAX_ATMI_H
#if defined(_TMAX_MTLIB) || defined(_MCONTEXT)
#define TPSINGLECONTEXT  0
#define TPINVALIDCONTEXT -1
#define TPNULLCONTEXT    -2
#endif
#endif

/* ---------- flags from API ----- */
/* Most Significant Two Bytes are reserved for internal use */
#ifndef TPNOFLAGS
#define TPNOFLAGS       0x00000000
#endif
#define TPNOBLOCK	0x00000001
#define TPSIGRSTRT      0x00000002
#define TPNOREPLY	0x00000004
#define TPNOTRAN	0x00000008
#define TPTRAN		0x00000010
#define TPNOTIME	0x00000020
#define TPNOGETANY	0x00000040
#define TPGETANY	0x00000080
#define TPNOCHANGE	0x00000100
#define TPBLOCK	        0x00000200
#define TPCONV		0x00000400
#define TPFLOWCONTROL	(TPCONV)
#define TPSENDONLY	0x00000800
#define TPRECVONLY	0x00001000
#define TPUDP           0x00002000
#define TPRQS		0x00004000
#define TPFUNC		0x00008000	/* API dependent functional flag */
#define	TPABSOLUTE	(TPFUNC)
#define TPACK		(TPFUNC)

/* --- flags used in tpstart() --- */
#define TPUNSOL_MASK	0x00000007	
#define TPUNSOL_HND     0x00000001
#define TPUNSOL_IGN     0x00000002
#define TPUNSOL_POLL    0x00000004
#define TPUNIQUE	0x00000010
#define TPONLYONE	0x00000020

/* --- flags used in tpreturn() --- */
#define TPFAIL		0x0001
#define TPSUCCESS	0x0002
#define TPEXIT	        0x0004
#define TPDOWN		0x0008
#define TP_FORWARD      0x0010          /* Internal use only */

/* ------ flags for reply type check ----- */
#define TPREQ           0
#define TPERR           -1

/* -------- for Tuxedo Compatability ------- */
/* Flags to tpscmt() - Valid TP_COMMIT_CONTROL characteristic values */
#define TP_CMT_LOGGED	0x01	/* return after commit decision is logged */
#define TP_CMT_COMPLETE	0x02	/* return after commit has completed */

/* Return values to tpchkauth() */
#define TPNOAUTH	0	/* no authentication */
#define TPSYSAUTH	1	/* system authentication */
#define TPAPPAUTH	2	/* system and application authentication */

/* unsolicited msg type */
#define TPPOST          1
#define TPBROADCAST     2
#define TPNOTIFY        3
#define TPSENDTOCLI     4

#define XATMI_SERVICE_NAME_LENGTH 32	/* where x must be > 15 */

#ifndef _TMAX_ATMI_H
struct clid_t {
    long clientdata[4];
};
typedef struct clid_t CLIENTID;


struct tpsvcinfo {
    char name[XATMI_SERVICE_NAME_LENGTH];
    char *data;
    long len;
    long flags;
    int	cd;
    CLIENTID cltid;
};
typedef struct tpsvcinfo TPSVCINFO;
#endif


#ifdef _WIN32
#if defined(__cplusplus)
extern "C" {
#endif
long *__EXPORT _tmget_tpurcode_addr(void);
long __EXPORT gettpurcode(void);
#define tpurcode (*_tmget_tpurcode_addr())
#if defined(__cplusplus)
}
#endif

#else
extern long tpurcode;
#endif


/* ---- flags used in conv[]: don't use dummy flags ----*/
#define TPEV_DISCONIMM	0x00000001
#define TPEV_SVCERR	0x00000002
#define TPEV_SVCFAIL	0x00000004
#define TPEV_SVCSUCC	0x00000008
#define TPEV_SENDONLY	0x00000020
#define TPCONV_DUMMY1	0x00000800  /* don't use this flag: TPSENDONLY */
#define TPCONV_DUMMY2	0x00001000  /* don't use this flag: TPRECVONLY */
#define TPCONV_OUT	0x00010000
#define TPCONV_IN	0x00020000


#define X_OCTET		"X_OCTET"
#define X_C_TYPE	"X_C_TYPE"
#define X_COMMON	"X_COMMON"

#define TMTYPEFAIL	-1
#define TMTYPESUCC      0


#ifndef MAXTIDENT
#define MAXTIDENT	16 /* max len of identifier */
#endif

#ifndef MAX_PASSWD_LENGTH
#define MAX_PASSWD_LENGTH  16
#endif
#ifndef MAX_MNAME_LENGTH
#define MAX_MNAME_LENGTH  16
#endif


#ifndef _TMAX_ATMI_H
struct	tpstart_t {
  char	usrname[MAXTIDENT+2];	/* usr name */
  char	cltname[MAXTIDENT+2];	/* application client name */
  char  dompwd[MAX_PASSWD_LENGTH+2]; /* domain password */
  char  usrpwd[MAX_PASSWD_LENGTH+2]; /* passwd for usrid */
  int   flags;
};
typedef	struct	tpstart_t TPSTART_T;
#endif

typedef void __EXPORT Unsolfunc(char *, long, long);
#define TPUNSOLERR	((Unsolfunc *) -1)

/* START EVENT BROKER MESSAGES */
#define TPEVSERVICE	0x00000001
#define TPEVQUEUE	0x00000002
#define TPEVTRAN	0x00000004
#define TPEVPERSIST	0x00000008

/* --- tperrno values --- */
#define TPMINVAL        0 
#define TPEABORT        1
#define TPEBADDESC      2
#define TPEBLOCK        3
#define TPEINVAL        4
#define TPELIMIT        5
#define TPENOENT        6
#define TPEOS           7
#define TPEPERM         8
#define TPEPROTO        9
#define TPESVCERR       10
#define TPESVCFAIL      11
#define TPESYSTEM       12
#define TPETIME         13
#define TPETRAN         14
#define TPGOTSIG        15
#define TPERMERR        16
#define TPEITYPE        17
#define TPEOTYPE        18
#define TPERELEASE      19
#define TPEHAZARD       20
#define TPEHEURISTIC    21
#define TPEEVENT        22
#define TPEMATCH        23
#define TPEDIAGNOSTIC   24
#define TPEMIB          25
#define TPMAXVAL        26

#endif /* !defined(_TMAX_KERNEL) && !defined(_TMAX_UTIL) */


struct	tpinit_t {
	char	usrname[MAXTIDENT+2];	/* client user name */
	char	cltname[MAXTIDENT+2];	/* application client name */
	char	passwd[MAXTIDENT+2];	/* application password */
	char	grpname[MAXTIDENT+2];	/* client group name */
	long	flags;			/* initialization flags */
	long	datalen;		/* length of app specific data */
	long	data;			/* placeholder for app data */
};
typedef	struct	tpinit_t TPINIT;

#define TPINITNEED(u)	(((u) > sizeof(long)) \
				? (sizeof(TPINIT) - sizeof(long) + (u)) \
				: (sizeof(TPINIT)))


/* START QUEUED MESSAGES ADD-ON */
#define TMQNAMELEN	15
#define TMMSGIDLEN	32
#define TMCORRIDLEN	32

struct tpqctl_t {		/* control parameters to queue primitives */
	long flags;		/* indicates which of the values are set */
	long deq_time;		/* absolute/relative  time for dequeuing */
	long priority;		/* enqueue priority */
	long diagnostic;	/* indicates reason for failure */
	char msgid[TMMSGIDLEN];	/* id of message before which to queue */
	char corrid[TMCORRIDLEN];/* correlation id used to identify message */
	char replyqueue[TMQNAMELEN+1];	/* queue name for reply message */
	char failurequeue[TMQNAMELEN+1];/* queue name for failure message */
	CLIENTID cltid;		/* client identifier for originating client */
	long urcode;		/* application user-return code */
	long appkey;		/* application authentication client key */
};
typedef struct tpqctl_t TPQCTL;

#if !defined(_TMAX_KERNEL) && !defined(_TMAX_UTIL)
/* Subscription Control structure */
#ifndef _TMAXAPI_H
struct tpevctl_t {
	long flags;
	char name1[XATMI_SERVICE_NAME_LENGTH];
	char name2[XATMI_SERVICE_NAME_LENGTH];
	TPQCTL qctl;
};

typedef struct tpevctl_t TPEVCTL;
#endif
#endif

/* structure elements that are valid - set in flags */
#ifndef TPNOFLAGS
#define TPNOFLAGS	0x00000
#endif
#define	TPQCORRID	0x00001		/* set/get correlation id */
#define	TPQFAILUREQ	0x00002		/* set/get failure queue */
#define	TPQBEFOREMSGID	0x00004		/* enqueue before message id */
#define	TPQGETBYMSGID	0x00008		/* dequeue by msgid */
#define	TPQMSGID	0x00010		/* get msgid of enq/deq message */
#define	TPQPRIORITY	0x00020		/* set/get message priority */
#define	TPQTOP		0x00040		/* enqueue at queue top */
#define	TPQWAIT		0x00080		/* wait for dequeuing */
#define	TPQREPLYQ	0x00100		/* set/get reply queue */
#define	TPQTIME_ABS	0x00200		/* set absolute time */
#define	TPQTIME_REL	0x00400		/* set absolute time */
#define	TPQGETBYCORRID	0x00800		/* dequeue by corrid */
#define	TPQPEEK		0x01000		/* peek */


#if defined (__cplusplus)
extern "C" {
#endif

/* --------- Tuxedo Transaction API ----------- */
int __EXPORT tpinit(TPINIT *tpinfo);
int __EXPORT tpterm(void);
int __EXPORT tpbegin(unsigned long timeout, long flags);
int __EXPORT tpcommit(long flags);
int __EXPORT tpabort(long flags);
int __EXPORT tpopen(void);
int __EXPORT tpclose(void);
int __EXPORT tpscmt(long flags);
int __EXPORT tpgetlev(void);

/* ----- client API ----- */
int __EXPORT tpstart(TPSTART_T *);
int __EXPORT tpend();
char *__EXPORT  tpalloc(char *type,char *subtype, long size);
char *__EXPORT  tprealloc(char *ptr, long size);
long __EXPORT tptypes(char *ptr,char *type,char *subtype);
void __EXPORT tpfree(char *ptr);
int __EXPORT tpcall(char *svc, char *idata, long ilen, char **odata, 
       	      long *olen, long flags);
int __EXPORT tpacall(char *svc, char *data, long len, long flags);
char *__EXPORT  _tmget_compat_strerror(int err_no);
int __EXPORT tpgetrply(int *cd, char **data, long *len, long flags);
int __EXPORT tpcancel(int cd);
int *__EXPORT  _tmget_compat_errno_addr(void);
int __EXPORT _tmget_compat_errno(void);
#if !defined(_TMAX_KERNEL) && !defined(_TMAX_UTIL)
#ifndef _TMAX_ATMI_H
#define tperrno (*_tmget_compat_errno_addr())
#define tpstrerror(err)	(_tmget_compat_strerror(err))
#endif
#endif

/* ----- unsolicited messaging API ----- */
long __EXPORT tpsubscribe(char *eventexpr, char *filter, TPEVCTL *ctl, long flags);
int __EXPORT tpunsubscribe(long sd, long flags);
int __EXPORT tppost(char *eventname, char *data, long len, long flags);
int __EXPORT tpbroadcast(char *lnid, char *usrname, char *cltname, char *data,
           long len, long flags);
Unsolfunc *__EXPORT tpsetunsol(Unsolfunc *func);
int __EXPORT tpsetunsol_flag(int flag);
int __EXPORT tpgetunsol(int type, char **data, long *len, long flags);
int __EXPORT tpclearunsol(void);

/* ----- conversational API ----- */
int __EXPORT tpconnect(char *svc, char *data, long lenl, long flagsl);
int __EXPORT tpdiscon(int cd);
int __EXPORT tpsend(int cd, char *data, long lenl, long flagsl, long *revent);
int __EXPORT tprecv(int cd, char **data, long *len, long flagsl, long *revent);

/* ----- server API -------- */
void __EXPORT tpreturn(int rval, long rcode, char *data, long len, long flags);
void __EXPORT tpforward(char *svc, char *data, long len, long flags);

/* ------ Tuxedo RQ API (Not supported) ------ */
int __EXPORT tpenqueue (char *qspace, char *qname, TPQCTL *ctl, char *data, 
	   long len, long flags);
int __EXPORT tpdequeue (char *qspace, char *qname, TPQCTL *ctl, char **data, 
	   long *len, long flags);

/* ----- etc API ------------- */
int __EXPORT tperrordetail(int);
int __EXPORT tpchkauth(void);
int __EXPORT tpgprio(void);
int __EXPORT tpsprio(int prio, long flags);
char *__EXPORT tuxgetenv(char *);
int __EXPORT tuxputenv(char *);
int __EXPORT tuxreadenv(char *, char *);

/* ----- multithread/multicontext API ----- */
int __EXPORT tpsetctxt(int ctxtid, long flags);
int __EXPORT tpgetctxt(int *ctxtid, long flags);

#if defined (__cplusplus)
}
#endif

#endif       /* end of _TMAX_TUXATMI_H  */
