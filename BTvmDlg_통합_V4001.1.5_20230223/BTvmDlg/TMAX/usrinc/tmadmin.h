
/* ------------------------ usrinc/tmadmin.h ------------------ */
/*								*/
/*              Copyright (c) 2000 - 2008 Tmax Soft Co., Ltd	*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef _TMADMIN_H
#define _TMADMIN_H
#define _TMADMIN_VERSION	10103	/* 01.01.03 */

#ifdef _WIN32
#ifdef _CE_MODULE
#include        <winsock.h>
#else
#include        <winsock2.h>
#endif
#else
#include <sys/types.h>
#ifndef ORA_PROC
#include <sys/socket.h>
#endif
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

/* 
   TMADMIN COMMANDS 
 */

/* simple commands */
#define TMADM_DISCON		1	/* client disconnect */
#define TMADM_CLIINFO		2	/* client information */
#define TMADM_QPURGE		3	/* server q purge */
#define TMADM_BOOT		4	/* tmboot */
#define TMADM_DOWN		5	/* tmdown */
#define TMADM_SUSPEND		6	/* suspend process */
#define TMADM_RESUME		7	/* resume process */
#define TMADM_RESTAT		8	/* reset statistics */
#define TMADM_CHTRC		9 	/* change trace option */
#define TMADM_CHLOG		10 	/* change log level option */

/* statistics */
#define TMADM_SVC_STAT		11
#define TMADM_SPR_STAT		12
#define TMADM_SVR_STAT		13
#define TMADM_TMGW_STAT		14
#define TMADM_NTMGW_STAT	15
#define TMADM_TMMS_STAT		16
#define TMADM_CLHS_STAT		17
#define TMADM_TMS_STAT		18
#define TMADM_SMTRC			19

/* statistics extension */
#define TMADM_SVC_STAT_EX   111
#define TMADM_SPR_STAT_EX   112

/* configuration */
#define TMADM_TMAX_INFO		20
#define TMADM_DOMAIN_CONF	21
#define TMADM_NODE_CONF		22
#define TMADM_SVG_CONF		23
#define TMADM_SVR_CONF		24
#define TMADM_SVC_CONF		25
#define TMADM_HMS_STAT		26
#define TMADM_HMSCLI_STAT	27

/* other admin cmd */
#define TMADM_RESTART		51	/* restart */
#define TMADM_SET		52	/* set */

/* tmadmin option flags */
#ifndef TPNOFLAGS
#define TPNOFLAGS       0x00000000
#endif
#define TMADM_AFLAG	0x00000001	/* ALL */
#define TMADM_CFLAG	0x00000002	/* CLH or Client */
#define TMADM_FFLAG	0x00000004	/* Force */
#define TMADM_GFLAG	0x00000008	/* SVG or Gateway */
#define TMADM_IFLAG	0x00000010	/* Immediate or Idle */
#define TMADM_NFLAG	0x00000020	/* Not Ready or Node or Number */
#define TMADM_PFLAG	0x00000040	/* Server Process */
#define TMADM_RFLAG	0x00000080	/* RQ */
#define TMADM_SFLAG	0x00000100	/* SVR or SVC */
#define TMADM_TFLAG	0x00000200	/* TMS */
#define TMADM_VFLAG	0x00000400	/* SVR */
#define TMADM_EFLAG	0x00000800	/* TMM (else engine flag) */

#define	TMADM_TMM	0x00000001	/* TMM */
#define	TMADM_CLH	0x00000002	/* CLH */
#define	TMADM_TMS	0x00000004	/* TMS */
#define	TMADM_SVR	0x00000008	/* SVR */

/* ETC macros */
#define MAX_NUM_PORTNO		8	/* >= MAX_LISTEN	*/
/* Ver 3.14.2 : Mar/17/2005 */
#define MAX_NUM_CLH		16	/* >= MAX_CLHS		*/

#ifndef TMAX_NAME_SIZE
#define TMAX_NAME_SIZE		16
#endif
#define TMAX_PATH_LENGTH	256
#define TMAX_CLOPT_LENGTH	256
#define TMAX_DBINFO_SIZE	256
#define TMAX_HOSTNAME_SIZE	256	/* SYS_NMLN at SVR4 */

#define	TMAX_STR_SIZE		16
#define TMAX_TRACE_SIZE		256 	
#define TMAX_LOGLVL_SIZE	16 
#define TMAX_HOSTNAME_SIZE	256
#define TMAX_IPADDR_SIZE	16
#define TMAX_TIME_SIZE		16
#define TMAX_XID_SIZE		12

#define CLITYPE_START		0
#define QSENDER			0
#define PUBLISHER 		1
#define ASYNC_QRECEIVER		2
#define ASYNC_SUBSCRIBER	3
#define ASYNC_DURABLE_SUBSCRIBER	4
#define QRECIEVER		5
#define SUBSCRIBER		6
#define DURABLE_SUBSCRIBER	7
#define CLITYPE_END		7

/* SysMaster Global ID structure */
typedef struct {
	int	gid1;
	int	gid2;
	int	seqno;
} tmax_smgid_t;

/* Fixed header structure */
struct tmadm_header {
	int	version;	/* I: _TMADMIN_VERSION */
	int	size;		/* I: total buffer size */
	int	offset;		/* I: fetch offset */
	int	num_entry;	/* O: number of fetched entries */
	int	num_left;	/* O: number of entries to fetch */
	int	opt_int;	/* I: optional integer argument */
	int	reserve_int[2];			
	/* --- Tmax Ver. 4.00 add '* 2' for svcname */
	char	opt_char[TMAX_NAME_SIZE * 2]; /* I: optinal string */
};


/* TMADM_DOMAIN_CONF return structures */
struct tmadm_domain_conf_body {
	int	no;				/* reserved */
	int	domain_id;
	int	shmkey;
	int	minclh;
	int	maxclh;
	int	maxuser;
	int	tportno;
	int	racport;
	int	cpc;
	int	blocktime;
	int	txtime;
	int	clichkint;
	int	nliveinq;
	int	nclhchkint;
	int	cmtret;
	int	security;
	/* -- client/server table paramters -- */
	int	maxsacall;
	int	maxcacall;
	int	maxconvn;
	int	maxconvs;
	/* --- maximum configuration values --- */
	int	maxnode;
	int	maxsvg;
	int	maxsvr;
	int	maxsvc;
	int	maxspr;
	int	maxtms;
	int	maxcpc;
	int	maxrout;
	int	maxroutsvg;
	int	maxrq;
	int	maxgw;
	int	maxcousin;
	int	maxcousinsvg;
	int	maxbackup;
	int	maxbackupsvg;
	int	maxtotalsvg;
	int	maxprod;
	int	maxfunc;
	/* ------------------------------------ */
	int	reserve_int[2];
	char	name[TMAX_NAME_SIZE];	/* domain name */
	char	owner[TMAX_NAME_SIZE];
	/* ---- added Tmax Ver. 4.00 ----- */
	char	tipsvc[XATMI_SERVICE_NAME_LENGTH];
	char	crypt[TMAX_STR_SIZE];
	int	maxthread;
	char	tmmloglvl[TMAX_STR_SIZE];
	char	clhloglvl[TMAX_STR_SIZE];
	char	tmsloglvl[TMAX_STR_SIZE];
	char	svrloglvl[TMAX_STR_SIZE];
};

struct tmadm_domain_conf {
	/* fixed header */
	struct tmadm_header header;
	/* fixed body */
	struct tmadm_domain_conf_body body;
};


/* TMADM_NODE_CONF return structures */
struct tmadm_node_conf_body {
	int	no;
	int	shmkey;
	int	shmsize;
	int	minclh;
	int	maxclh;
	int	maxuser;
	int	clhqtimeout;
	int	idletime;
	int	clichkint;
	int	racport;
	int	rscpc;
	int	ipcperm;
	unsigned int ip;
	int	maxsvg;
	int	maxsvr;
	int	maxspr;
	int	maxtms;
	int	maxcpc;
	int	reserve_int[2];			/* reserved */
	int	scaport[MAX_NUM_PORTNO];
	int	tmaxport[MAX_NUM_PORTNO];
	char	name[TMAX_NAME_SIZE];		/* local node name */
	char	logoutsvc[TMAX_NAME_SIZE];
	char	realsvr[TMAX_NAME_SIZE];
	char	reserved_str[TMAX_NAME_SIZE];	/* reserved */
	char	tmaxdir[TMAX_PATH_LENGTH];
	char	appdir[TMAX_PATH_LENGTH];
	char	pathdir[TMAX_PATH_LENGTH];
	char	tlogdir[TMAX_PATH_LENGTH];
	char	slogdir[TMAX_PATH_LENGTH];
	char	ulogdir[TMAX_PATH_LENGTH];
	char	envfile[TMAX_PATH_LENGTH];
	char	tmaxhome[TMAX_PATH_LENGTH];	/* tmax install directory */
	char	hostname[TMAX_HOSTNAME_SIZE];	/* physical node name */
	/* ----- other information -----  */
	int	curclh;
	int	clh_maxuser;
	int	svgcount;
	int	svrcount;
	int	svccount;
	int	sprcount;
	int	reserve_int2[2];		/* reserved */
	int	clicount[MAX_NUM_CLH];
	/* ---- added Tmax Ver. 4.00 ----- */
	int 	maxthread;
	char	tmmloglvl[TMAX_STR_SIZE];
	char 	clhloglvl[TMAX_STR_SIZE];
	char 	tmsloglvl[TMAX_STR_SIZE];
	char	svrloglvl[TMAX_STR_SIZE];
	int	extport;
	int	extclhport[MAX_NUM_CLH];
};

struct tmadm_node_conf {
	/* fixed header */
	struct tmadm_header header;
	/* fixed body */
	struct tmadm_node_conf_body body;
};


/* TMADM_SVG_CONF return structures */
struct tmadm_svg_conf_body {
	int	no;
	int	curtms;
	int	mintms;
	int	maxtms;
	int	reserve_int[4];			/* reserved */
	char	name[TMAX_NAME_SIZE];
	char	svgtype[TMAX_NAME_SIZE];
	char	owner[TMAX_NAME_SIZE];
	char	dbname[TMAX_NAME_SIZE];
	char	tmsname[TMAX_NAME_SIZE];
	char	appdir[TMAX_PATH_LENGTH];
	char	ulogdir[TMAX_PATH_LENGTH];
	char	envfile[TMAX_PATH_LENGTH];
	char	openinfo[TMAX_DBINFO_SIZE];
	char	closeinfo[TMAX_DBINFO_SIZE];
	/* ---- added Tmax Ver. 4.00 ----- */
	char	tmstype[TMAX_STR_SIZE];
	int	tmsthreads;
	char	tmsopt[TMAX_CLOPT_LENGTH];
	char	tmsloglvl[TMAX_STR_SIZE];
	char	svrloglvl[TMAX_STR_SIZE];
};

struct tmadm_svg_conf {
	/* fixed header */
	struct tmadm_header header;
	/* variable length body */
	struct tmadm_svg_conf_body svg[1];
};


/* TMADM_SVR_CONF return structures */
struct tmadm_svr_conf_body {
	int	no;
	int	svgno;
	int	cursvr;
	int	minsvr;
	int	maxsvr;
	int	conv;
	int	maxqcount;
	int	asqcount;
	int	maxrstart;
	int	gperiod;
	int	restart;
	int	cpc;
	int	reserve_int[4];			/* reserved */
	char	name[TMAX_NAME_SIZE];
	char	target[TMAX_NAME_SIZE];
	char	svrtype[TMAX_NAME_SIZE];
	char	reserve_str[TMAX_NAME_SIZE];	/* reserved */
	char	clopt[TMAX_CLOPT_LENGTH];
	char	ulogdir[TMAX_PATH_LENGTH];
	/* ---- added Tmax Ver. 4.00 ----- */
	char	svrloglvl[TMAX_STR_SIZE];
};

struct tmadm_svr_conf {
	/* fixed header */
	struct tmadm_header header;
	/* variable length body */
	struct tmadm_svr_conf_body svr[1];
};


/* TMADM_SVR_CONF return structures */
struct tmadm_svc_conf_body {
	int	svctime;
	int	svri;	/* -1 means this node does not support this svc */
	int	reserve_int[2];			/* reserved */
	/* --- modified Tmax Ver. 4.00 TMAX_NAME_SIZE -> XATMI_SERVICE_NAME_LENGTH */
	char	name[XATMI_SERVICE_NAME_LENGTH];
	char    funcname[TMAX_NAME_SIZE];
};

struct tmadm_svc_conf {
	/* fixed header */
	struct tmadm_header header;
	/* variable length body */
	struct tmadm_svc_conf_body svc[1];
};


/* TMADM_TMAX_INFO return structures */
struct tmadm_tmax_info_body {
	int	sysinfo;	/* OS version */
	int	version;	/* major*10000 + minor*100 + patch */
	int	expdate;	/* if demo license year*10000 + month*100 + day,
				   otherwise 0 */
	int	maxuser;	/* if unlimited, maxuser = -1 */
	int	nodecount;
	int	svgcount;
	int	svrcount;
	int	svccount;
	int	rqcount;
	int	gwcount;
	int	rout_gcount;	/* rout group count */
	int	rout_count;
	int	cousin_gcount;	/* cousin group count: svg  */
	int	cousin_count;
	int	backup_gcount;	/* backup group count: svg */
	int	backup_count;
	/* for TopEnd */
	int	prod_count;
	int	func_count;
	int	reserve[6];
};

struct tmadm_node_summary {
	int	no;		/* node number */
	int	port;		/* TPORTNO[0] */
	int	racport;
	int	shmkey;
	int	shmsize;
	int	minclh;
	int	maxclh;
	unsigned int	ip;
	char	name[TMAX_NAME_SIZE];		/* logical node name */
	char	hostname[TMAX_HOSTNAME_SIZE];	/* physical node name */
};

struct tmadm_tmax_info {
	/* fixed header */
	struct tmadm_header header;
	/* fixed body */
	struct tmadm_tmax_info_body body;
	/* variable length body */
	struct tmadm_node_summary node[1];
};


/* TMADM_SVC_STAT return structures */
struct tmadm_svc_stat_body {
	int	no;
	int	clhno;
	int	count;
	int	cq_count;
	int	aq_count;
	int	reserve[3];
	float	average;
	float	q_average;
	/* modified Tmax Ver. 4.00 TMAX_NAME_SIZE -> XATMI_SERVICE_NAME_LENGTH */
	char	name[XATMI_SERVICE_NAME_LENGTH];
	char	status[TMAX_NAME_SIZE];
	/* char	reserve_str[TMAX_NAME_SIZE*2]; */
	float usravg; 
	float usrmin; 
	float usrmax; 
	float sysavg; 
	float sysmin; 
	float sysmax;
};

struct tmadm_svc_stat {
	/* fixed header */
	struct tmadm_header header;
	/* variable length body */
	struct tmadm_svc_stat_body svc[1];
};

/* TMADM_SVC_STAT_EX return structures */
struct tmadm_svc_stat_ex_body {
	int	no;
	int	clhno;
	int	count;
	int	cq_count;
	int	aq_count;
	int	reserve[3];
	float	average;
	float	q_average;
	/* modified Tmax Ver. 4.00 TMAX_NAME_SIZE -> XATMI_SERVICE_NAME_LENGTH */
	char	name[XATMI_SERVICE_NAME_LENGTH];
	char	status[TMAX_NAME_SIZE];
	/* char reserve_str[TMAX_NAME_SIZE*2]; */
	float usravg; 
	float usrmin; 
	float usrmax; 
	float sysavg; 
	float sysmin; 
	float sysmax;
	/* ---- added ex ----- */
	int fail_count;
	int error_count;
	float mintime;
	float maxtime;
};

struct tmadm_svc_stat_ex {
	/* fixed header */
	struct tmadm_header header;
	/* variable length body */
	struct tmadm_svc_stat_ex_body svc[1];
};

/* TMADM_SPR_STAT return structures */
struct tmadm_spr_stat_body {
	int	no;
	int	clhno;
	int	count;
	struct {
	    int sec;
	    int msec;
	}	start_time;
	int	reserve[2];
	float	average;
	char	name[TMAX_NAME_SIZE];
	char	svgname[TMAX_NAME_SIZE];
	char	status[TMAX_NAME_SIZE];
	char	service[TMAX_NAME_SIZE];
	/* ---- added Tmax Ver. 4.0 SP5 ----- */
	tmax_smgid_t gid;
	int	pid;
	float	usravg; 
	float	usrmin; 
	float	usrmax; 
	float	sysavg; 
	float	sysmin; 
	float	sysmax;
};

struct tmadm_spr_stat {
	/* fixed header */
	struct tmadm_header header;
	/* variable length body */
	struct tmadm_spr_stat_body spr[1];
};

/* TMADM_SPR_STAT_EX return structures */
struct tmadm_spr_stat_ex_body {
	int	no;
	int	clhno;
	int	count;
	struct {
	    int sec;
	    int msec;
	}	start_time;
	int	reserve[2];
	float	average;
	char	name[TMAX_NAME_SIZE];
	char	svgname[TMAX_NAME_SIZE];
	char	status[TMAX_NAME_SIZE];
	char	service[TMAX_NAME_SIZE];
	/* ---- added Tmax Ver. 4.0 SP5 ----- */
	tmax_smgid_t gid;
	int	pid;
	float	usravg; 
	float	usrmin; 
	float	usrmax; 
	float	sysavg; 
	float	sysmin; 
	float	sysmax;
	/* ---- added ex ----- */
	int fail_count;
	int error_count;
	float mintime;
	float maxtime;
};

struct tmadm_spr_stat_ex {
	/* fixed header */
	struct tmadm_header header;
	/* variable length body */
	struct tmadm_spr_stat_ex_body spr[1];
};

/* TMADM_SVR_STAT return structures */
struct tmadm_svr_stat_body {
	int	svri;
	int	clhno;
	int	count;
	int	qcount;
	int	qpcount;
	int	emcount;
	int	reserve_int[2];
	char	name[TMAX_NAME_SIZE];
	char	status[TMAX_NAME_SIZE];
	char	reserve_str[TMAX_NAME_SIZE*2];
};

struct tmadm_svr_stat {
	/* fixed header */
	struct tmadm_header header;
	/* variable length body */
	struct tmadm_svr_stat_body svr[1];
};

/* TMADM_TMAXGW_STAT return structures */
struct tmadm_tmgw_stat_body {
	int	no;
	char	name[TMAX_NAME_SIZE];
	char	ctype[TMAX_STR_SIZE];
	char	ctype2[TMAX_STR_SIZE];
	char	hostn[TMAX_HOSTNAME_SIZE];
	char	ipaddr[TMAX_IPADDR_SIZE];
	int	port;
	char 	status[TMAX_STR_SIZE];
};

struct tmadm_tmgw_stat {
	struct	tmadm_header header;
	struct 	tmadm_tmgw_stat_body tmgw[1];
};

/* TMADM_TMMS_STAT return structures */
struct tmadm_tmms_stat_body {
	int 	no;
	char	nodename[TMAX_NAME_SIZE];
	char	time[TMAX_TIME_SIZE];
	char	status[TMAX_STR_SIZE];
};

struct tmadm_tmms_stat {
	struct	tmadm_header header;
	struct 	tmadm_tmms_stat_body tmms[1];
};

/* TMADM_CLHS_STAT return structures */
struct tmadm_clhs_stat_body {
	int 	no;
	char	nodename[TMAX_NAME_SIZE];
	int	clhno;
	int	cpc;
	char	status[TMAX_STR_SIZE];
};

struct tmadm_clhs_stat {
	struct	tmadm_header header;
	struct	tmadm_clhs_stat_body clhs[1];
};

/* TMADM_TMS_STAT return structures */
struct tmadm_tms_stat_body {
	char 	tmsname[TMAX_NAME_SIZE];
	char 	svgname[TMAX_NAME_SIZE];
	int	spri;
	char	status[TMAX_STR_SIZE];
	int	count;
	float 	avg;
	int	cqcount;
	int	thri;
	char	xid[TMAX_XID_SIZE];
	char	xa_status[TMAX_STR_SIZE];
};

struct tmadm_tms_stat {
	struct tmadm_header header;
	struct tmadm_tms_stat_body tms[1];
};

/* TMADM_CLIINFO return structures */
struct tmadm_cliinfo_body {
	int	no;	/* cli index */
	int	clid;	/* CLID */
	int	clhno;
	int	count;	
	int	idle;
	int	reserve_int[3];
	char	status[TMAX_NAME_SIZE];
	char	addr[TMAX_NAME_SIZE];
	char	usrname[TMAX_NAME_SIZE];
	char	reserve_str[TMAX_NAME_SIZE];
};

struct tmadm_cliinfo {
	/* fixed header */
	struct tmadm_header header;
	/* variable length body */
	struct tmadm_cliinfo_body cli[1];
};


/* TMADM_BOOT return structures */
struct tmadm_boot_body {
	int	count;
	int	opt_int[3];
	char	name1[TMAX_NAME_SIZE];
	char	name2[TMAX_NAME_SIZE];
	char	clopt[TMAX_CLOPT_LENGTH];
};

struct tmadm_boot {
	/* fixed header */
	struct tmadm_header header;
	/* fixed body */
	struct tmadm_boot_body args;
};


/* TMADM_DOWN return structures */
struct tmadm_down_body {
	int	count;
	int	opt_int[3];
	char	name1[TMAX_NAME_SIZE];
	char	name2[TMAX_NAME_SIZE];
};

struct tmadm_down {
	/* fixed header */
	struct tmadm_header header;
	/* fixed body */
	struct tmadm_down_body args;
};

/* TMADM_CHTRC return structures */
struct tmadm_chtrc_body {
	char 	nodename[TMAX_NAME_SIZE];
	char	svgname[TMAX_NAME_SIZE];
	char	svrname[TMAX_NAME_SIZE];
	int	spri;
	char	spec[TMAX_TRACE_SIZE];
};

struct tmadm_chtrc {
	struct tmadm_header header;
	struct tmadm_chtrc_body args;
};

/* TMADM_CHLOG return structures */
struct tmadm_chlog_body {
	char	nodename[TMAX_NAME_SIZE];
	char	svgname[TMAX_NAME_SIZE];
	char	svrname[TMAX_NAME_SIZE];
	int	module;
	char	level[TMAX_LOGLVL_SIZE];
};

struct tmadm_chlog {
	struct tmadm_header header;
	struct tmadm_chlog_body args;
};

/* TMADM_RESTART return structures */
struct tmadm_restart_body {
	char	svgname[TMAX_NAME_SIZE];
	char	svrname[TMAX_NAME_SIZE];
};

struct tmadm_restart {
	struct tmadm_header header;
	struct tmadm_restart_body args;
};

/* TMADM_SMTRC return structures */
struct tmadm_smtrc_body {
	int	seqno;
	int	clhno;
	char	status[TMAX_NAME_SIZE];
	char	name[TMAX_NAME_SIZE];
};

struct tmadm_smtrc {
	/* fixed header */
	struct tmadm_header header;
	/* fixed body */
	struct tmadm_smtrc_body trc[1];
};

/* TMADM_SMTRC with TMADM_AFLAG return structures */
struct tmadm_smtrcall_body {
	int     seqno;
	int     clhno;
	char    status[TMAX_NAME_SIZE];
	char    name[TMAX_NAME_SIZE];
	int     spri;
	int     reserved;
	struct timeval curtime;
	struct timeval svctime;
	struct timeval ucputime;
	struct timeval scputime;
};

struct tmadm_smtrcall {
	/* fixed header */
	struct tmadm_header header;
	/* fixed body */
	struct tmadm_smtrcall_body trc[1];
};

/* SysMaster Trace Log structure */
typedef struct {
	tmax_smgid_t gid;
	int     clhno;
	char    status[TMAX_NAME_SIZE];
	char    name[TMAX_NAME_SIZE];
	int     spri;
	int     reserved;
	struct timeval curtime;
	struct timeval svctime;
	struct timeval ucputime;
	struct timeval scputime;
} tmax_smtrclog_t;

/* TMADM_SET return structures */
struct tmadm_set_body {
	char	option[TMAX_NAME_SIZE];
	char	name[TMAX_NAME_SIZE];
	char	field[TMAX_NAME_SIZE];
	int	value;
};

struct tmadm_set {
	struct tmadm_header header;
	struct tmadm_set_body args;
};

struct tmadm_hms_stat_body {
	char svgname[TMAX_NAME_SIZE];
	char destination[TMAX_NAME_SIZE];
	int type;
	int qcount;
	int apqcount;
	int acqcount;
	int cconsumer;
	int cproducer;
	int fdcount; /* fail discard count */
	int tdcount; /* ttl discard count */
};

struct tmadm_hms_stat {
	struct tmadm_header header;
	struct tmadm_hms_stat_body tms[1];
};

struct tmadm_hmscli_stat_body {
	int sesi;
	int clid;
	char cname[TMAX_NAME_SIZE];
	int clitype;
	char listener[XATMI_SERVICE_NAME_LENGTH];
	int qcount;
	int count;
	int fdcount; /* fail discard count */
	int tdcount; /* ttl discard count */
};

struct tmadm_hmscli_stat {
	struct tmadm_header header;
	struct tmadm_hmscli_stat_body cli[1];
};

#if defined (__cplusplus)
extern "C" {
#endif

/*
   tmadmin(TMADM_DISCON, int *clid, TMADM_FFLAG, TPNOFLAGS);
   tmadmin(TMADM_NODE_INFO, struct tmadm_node_info *info, TPNOFLAGS, TPNOFLAGS);
 */
int __EXPORT tmadmin(int cmd, void *arg, int opt, long flags);
int __EXPORT tmgetsmgid(tmax_smgid_t *gid);
int __EXPORT tmget_smtrclog_count(void *handle);
int __EXPORT tmget_smtrclog(void *handle, tmax_smtrclog_t *buf, int *count);

#if defined (__cplusplus)
}
#endif

#endif       /* end of _TMADMIN_H  */
