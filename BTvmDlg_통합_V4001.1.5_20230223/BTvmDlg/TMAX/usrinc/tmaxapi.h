
/* ------------------------ usrinc/tmaxapi.h ------------------ */
/*								*/
/*              Copyright (c) 2000 - 2008 Tmax Soft Co., Ltd	*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef _TMAXAPI_H
#define _TMAXAPI_H

#ifndef _CE_MODULE
#include <sys/types.h>
#endif
#include <usrinc/atmi.h>
#ifdef _WIN32
#ifdef _CE_MODULE
#include <winsock.h>
#else
#include <winsock2.h>
#endif  /* _CE_MODULE */
#include <usrinc/svct.h>
#include <usrinc/sdl.h>
#else
#ifndef ORA_PROC
#include <sys/socket.h>
#endif
#include <sys/time.h>
#endif

/* client logout type */
#define CLIENT_CLOSE_NORMAL	0
#define CLIENT_CLOSE_ABNORMAL	1
#define CLIENT_PRUNED		2

/* RQ Sub-queue type */
#define TMAX_ANY_QUEUE		0
#define TMAX_FAIL_QUEUE		1
#define TMAX_REQ_QUEUE		2
#define TMAX_RPLY_QUEUE		3
#define TMAX_MAX_QUEUE          4

extern char _rq_sub_queue_name[TMAX_MAX_QUEUE][XATMI_SERVICE_NAME_LENGTH];

/* RQ related macros */
#define RQ_NAME_LENGTH		16

/* unsolicited msg type */
#define UNSOL_TPPOST		1
#define UNSOL_TPBROADCAST	2
#define UNSOL_TPNOTIFY		3
#define UNSOL_TPSENDTOCLI	4
#define UNSOL_ANY		5

/* RM type */
#define ORACLE_TYPE     	1
#define SYBASE_TYPE     	2
#define INFORMIX_TYPE  		3
#define DB2_TYPE        	4

#define NONXA_MODE		0
#define XA_MODE 		1

/* Check SVCINFO cmds */
#define ISSVC_FORWARDED	0x00000001
#define ISSVC_NOREPLY	0x00000002

/* TPEVCTL ctl_flags */
#define	TPEV_SVC	0x00000001
#define	TPEV_PROC	0x00000002

/* tpgethostaddr flags */
#define	GET_SVR_CON	0x00000000
#define	GET_CUR_IP	0x00000001

/* tmax_sq_get/tmax_sq_put flags */
#define TPSQ_PEEK	0x00001000
#define TPSQ_UPDATE	0x00002000
#define TPSQ_SYSKEY	0x00004000
#define TPSQ_KEYGEN	0x00008000

#define SQ_KEYLIST_T	void*
#define SQ_KEYINFO_T	sq_keyinfo_t
#define SQ_SYSKEY_SIZE	16

struct sq_keyinfo_s {
    long keylen;
    long datalen;
    time_t starttime;
    char *key;
};

typedef struct sq_keyinfo_s sq_keyinfo_t;

struct tpevctl {
    long ctl_flags;
    long post_flags;
    char svc[XATMI_SERVICE_NAME_LENGTH];
    char qname[RQ_NAME_LENGTH];
};

typedef struct tpevctl TPEVCTL;
typedef void __EXPORT Unsolfunc(char *, long, long);
#define TPUNSOLERR      ((Unsolfunc *) -1)

struct tptranid {
    int  info[3];
    int  flags;
};

typedef struct tptranid TPTRANID;

/* Multicast call related structures */
struct svglist {
    int	ns_entry;	/* number of entries of s_list */
    int	nf_entry;	/* number of entries of f_list */
    int *s_list;	/* list of server group numbers */
    int *f_list;	/* list of server group numbers */
};

/* Jun/23/2008 KANAKO TPMCALL_UPDATE <<start>> */
struct svglistx {
    int	ns_entry;	/* number of entries of s_list */
    int	nf_entry;	/* number of entries of f_list */
    int nr_entry;	/* number of entries of r_list */
    int *s_list;	/* list of server group numbers */
    int *f_list;	/* list of server group numbers */
    int *r_list;	/* list of tpurcode of each server group */
};
/* Jun/23/2008 KANAKO TPMCALL_UPDATE <<end>> */

/* My svrinfo */
#ifndef TMAX_NAME_SIZE
#define TMAX_NAME_SIZE          16
#endif

#ifndef MAX_DBSESSIONID_SIZE
#define MAX_DBSESSIONID_SIZE	128
#endif

typedef struct {
    int nodeno;	/* node index */
    int svgi;	/* server group index; unique in the node */
    int svri;	/* server index; unique in the node */
    int spri;	/* server process index; unique in the node */
    int spr_seqno;	/* server process seqno ; unique in the server */
    int min, max;	/* min/max server process number */
    int clhi;	/* for RDP only, corresponding CLH id */
    char nodename[TMAX_NAME_SIZE];
    char svgname[TMAX_NAME_SIZE];
    char svrname[TMAX_NAME_SIZE];
    char reserved_char[TMAX_NAME_SIZE];
    /* for more detail use tmadmin API */
} TMAXSVRINFO;

#define MSGIDLEN        32
#define CORRIDLEN       32

typedef struct {               /* control parameters to queue primitives */
        int flags;             /* indicates which of the values are set */
        int deq_time;          /* absolute/relative  time for dequeuing */
        int priority;          /* enqueue priority */
        int diagnostic;        /* indicates reason for failure */
        char msgid[MSGIDLEN]; /* id of message before which to queue */
        char corrid[CORRIDLEN];/* correlation id used to identify message */
        char replyqueue[TMAX_NAME_SIZE];  /* queue name for reply message */
        char failurequeue[TMAX_NAME_SIZE];/* queue name for failure message */
        CLIENTID cltid;         /* client identifier for originating client */
        int urcode;            /* application user-return code */
        int appkey;            /* application authentication client key */
        int delivery_qos;
        int reply_qos;
        int exp_time;
} TMQCTL;

#ifdef _WIN32
typedef int (__EXPORT *WinTmaxCallback)(WPARAM, LPARAM);
#endif

/* Macro functions */
#define tpalivechk()	tmax_chk_conn(0)

/* mode for IP-based ACL */
#define TMAX_ACL_ALLOW		0
#define TMAX_ACL_DENY		1

/* reserved mask value for IP-based ACL */
#define TMAX_ACL_IPADDR		32

#if defined (__cplusplus)
extern "C" {
#endif

/* ----- unsolicited messaging API ----- */
long __EXPORT tpsubscribe(char *eventexpr, char *filter, TPEVCTL *ctl, long flags);
long __EXPORT tpsubscribe2(char *eventexpr, char *svcname, long flags);
int __EXPORT tpunsubscribe(long sd, long flags);
int __EXPORT tppost(char *eventname, char *data, long len, long flags);
int __EXPORT tpbroadcast(char *lnid, char *usrname, char *cltname, char *data,
	    long len, long flags);
Unsolfunc *__EXPORT tpsetunsol(Unsolfunc *func);
int __EXPORT tpsetunsol_flag(int flag);
int __EXPORT tpgetunsol(int type, char **data, long *len, long flags);
int __EXPORT tpclearunsol(void);
int __EXPORT tpchkunsol(void);

/* ----- RQS API -------- */
int __EXPORT tpenq(char *qname, char *svc, char *data, long len, long flags);
int __EXPORT tpdeq(char *qname, char *svc, char **data, long *len, long flags);
int __EXPORT tpenq_ctl(char *qname, char *svc, TMQCTL *ctl, char *data, long len, long flags);
int __EXPORT tpdeq_ctl(char *qname, char *svc, TMQCTL *ctl, char **data, long *len, long flags);
int __EXPORT tpqstat(char *qname, long type);
int __EXPORT tpqsvcstat(char *qname, char *svc, long type);
int __EXPORT tpextsvcname(char *data, char *svc);
int __EXPORT tpextsvcinfo(char *data, char *svc, int *type, int *errcode);
int __EXPORT tpreissue(char *qname, char *filter, long flags);
char *__EXPORT tpsubqname(int type);

/* ----- server API -------- */
int __EXPORT tpgetminsvr(void);
int __EXPORT tpgetmaxsvr(void);
int __EXPORT tpgetmaxuser(void);
int __EXPORT tpgetsvrseqno(void);
int __EXPORT tpgetmysvrid(void);
int __EXPORT tpgetmysvrno(void);
int __EXPORT tpgetmaxuser(void);
int __EXPORT tpsendtocli(int clid, char *data, long len, long flags);
int __EXPORT tpgetclid(void);
int __EXPORT tpgetpeer_ipaddr(struct sockaddr *name, int *namelen);
int __EXPORT tpchkclid(int clid);
int __EXPORT tmax_clh_maxuser(void);
int __EXPORT tmax_my_svrinfo(TMAXSVRINFO*);
int __EXPORT tmax_cind2clid(int cind);
char *__EXPORT tpgetmynode(int *nodeno);
char *__EXPORT tpgetmysvg(void);
int __EXPORT tpgetmysvgno(void);
int __EXPORT tmax_is_restarted(void);
char *__EXPORT tpgetsvcname(int svci);
int __EXPORT tpsuspendtx(TPTRANID *tranid, long flags);
int __EXPORT tpresumetx(TPTRANID *tranid, long flags);

/* ----- etc API ----------- */
int __EXPORT tp_sleep(int sec);
int __EXPORT tptsleep(struct timeval *timeout);
int __EXPORT tp_usleep(int usec);
int __EXPORT tpset_timeout(int sec);
int __EXPORT tpget_timeout(void);
int __EXPORT tmaxreadenv(char *file, char *label);
char *__EXPORT tpgetenv(char* str);
int __EXPORT tpputenv(char* str);
int __EXPORT tpgetsockname(struct sockaddr *name, int *namelen);
int __EXPORT tpgetpeername(struct sockaddr *name, int *namelen);
int __EXPORT tpgetactivesvr(char *nodename, char **outbufp);
int __EXPORT tperrordetail(int i);
int __EXPORT tpreset(void);
int __EXPORT tptobackup(void);
struct svglist *__EXPORT 
    tpmcall(char *qname, char *svc, char *data, long len, long flags);
struct svglistx *__EXPORT 
    tpmcallx(char *svc, char *data, long len, long flags);
struct svglist *__EXPORT tpgetsvglist(char *svc, long flags);
int __EXPORT tpsvgcall(int svgno, char *qname, 
	char *svc, char *data, long len, long flags);
int __EXPORT tpflush(void);
char *__EXPORT tmaxlastsvc(char *idata, char *odata, long flags);
int __EXPORT tpgetorgnode(int clid);
int __EXPORT tpgetorgclh(int clid);
char *__EXPORT tpgetnodename(int nodeno);
int __EXPORT tpgetnodeno(char *nodename);
int __EXPORT tpgetasize(char *data);
int __EXPORT tpgettype(char *data);
char * __EXPORT tpgetsubtype(char *data);
int __EXPORT tpgetcliaddr(int clid, int *ip, int *port, long flags);
int __EXPORT tmax_chk_conn(int timeout);
int __EXPORT tpgethostaddr(int *ip, int *port, long flags);
char * __EXPORT tpgetdbsessionid(int flags);
int __EXPORT tpcallsvg(int svgno, char *svc, char *idata, long ilen, 
	char **odata, long *olen, long flags);
int __EXPORT tpacallsvg(int svgno, char *svc, char *data, long len, long flags);

#if defined(_WIN32)
int __EXPORT WinTmaxAcall(TPSTART_T *sinfo, HANDLE wHandle, unsigned int msgType,
	char *svc, char *sndbuf, int len, int flags);
int __EXPORT WinTmaxAcall2(TPSTART_T *sinfo, WinTmaxCallback fn,
	char *svc, char *sndbuf, int len, int flags);
#endif

#if !defined(_TMAX_KERNEL) && !defined(_TMAX_RCA_H)
/* ------- User supplied routines ---------- */
int __EXPORT tpsvrinit(int argc, char *argv[]);
int __EXPORT tpsvrdone(void);
void __EXPORT tpsvctimeout(TPSVCINFO *msg);
int __EXPORT _tmax_event_handler(char *progname, int pid, int tid, char *msg, int flags);
int __EXPORT tpsetdbsessionid(char dbsessionid[MAX_DBSESSIONID_SIZE], int flags);
int __EXPORT tpprechk(void);
#endif

/* 
   Internal functions: ONLY BE CALLED FROM AUTOMATICALLY 
   GENERATED STUB FILES. DO NOT DIRECTLY CALL THESE FUNCTIONS.
 */
int __EXPORT get_clhfd(void);
int __EXPORT tmax_chk_svcinfo(int cmd);
int __EXPORT _tmax_main(int argc, char *argv[]);
int __EXPORT _tmax_cob_main(int argc, char *argv[]);
#if defined(_WIN32)
int __EXPORT _tmax_regfn(void *initFn, void *doneFn, void *timeoutFn, void *userMainFn);
int __EXPORT _tmax_regtab(int svcTabSz, _svc_t *svcTab, int funcTabSz, void *funcTab);
int __EXPORT _tmax_regsdl(int _sdl_table_size2, struct _sdl_struct_s *_sdl_table2,
	int _sdl_field_table_size2, struct _sdl_field_s *_sdl_field_table2);
int __EXPORT _tmax_regevthnd(void *evthndFn);
#endif
int __EXPORT _double_encode(char *in, char *out);
int __EXPORT _double_decode(char *in, char *out);
/* --- power builder interface API --- */
int __EXPORT _make_struct_from_pbindata(char *subtype, char *tpidata, int ilen, char *indata);
int __EXPORT _make_field_from_pbindata(char **tpidata, char *indata);
int __EXPORT _make_pbodata_from_struct(char *subtype, char *odata, int olen, char *tpodata);
int __EXPORT _make_pbodata_from_field(char **form, char **odata, char *tpodata);
int __EXPORT _make_pbfodata_from_field(char **fform, char **fodata, char *tpodata);
int __EXPORT tmax_get_db_usrname(char *svgname, char *usrname, int type);
int __EXPORT tmax_get_db_passwd(char *svgname, char *passwd, int type);
int __EXPORT tmax_get_db_tnsname(char *svgname, char *tnsname, int type);

int __EXPORT tmax_is_xa();

/* 4.0 Sp2 Fix1 added */
int __EXPORT tmax_get_svccnt();
int __EXPORT tmax_get_svclist(char *buf, int bufsize);

/* ----- SessionQ API ----------- */
int __EXPORT tmax_sq_put(char *key, long keylenl, char *data, long lenl, long flagsl);
int __EXPORT tmax_sq_get(char *key, long keylenl, char **data, long *lenl, long flagsl);
int __EXPORT tmax_sq_count(void);
int __EXPORT tmax_sq_purge(char *key, long keylenl);
int __EXPORT tmax_sq_keygen(char *key, long keylenl);
SQ_KEYLIST_T __EXPORT tmax_sq_getkeylist(char *key, long keylenl);

int __EXPORT tmax_gq_put(char *key, long keylenl, char *data, long lenl, long flagsl);
int __EXPORT tmax_gq_get(char *key, long keylenl, char **data, long *lenl, long flagsl);
int __EXPORT tmax_gq_count();
int __EXPORT tmax_gq_purge(char *key, long keylenl);
int __EXPORT tmax_gq_keygen(char *key, long keylenl);
SQ_KEYLIST_T __EXPORT tmax_gq_getkeylist(char *key, long keylenl);

int __EXPORT tmax_get_sessionid(void);
int __EXPORT tmax_keylist_count(SQ_KEYLIST_T keylist);
int __EXPORT tmax_keylist_getakey(SQ_KEYLIST_T keylist, int nth, SQ_KEYINFO_T *keyinfo);
int __EXPORT tmax_keylist_free(SQ_KEYLIST_T keylist);

int __EXPORT tpgetsprlist(char *svc, int svgno, int *starti, int *endi, long flagsl);
int __EXPORT tpspracall(char *svcname, int spri, char *data, long lenl, long flagsl);
int __EXPORT tpspracall2(char *svcname, int startspri, int nth, char *data, long lenl, long flagsl);

/* API for IP-based ACL */
int __EXPORT tmax_add_acl(int nodeno, char *ip, int mask, int mode, int flags);

#if defined (__cplusplus)
}
#endif

#endif       /* end of _TMAXAPI_H  */
