
/* --------------------- usrinc/hlinkapi.h -------------------- */
/*                                                              */
/*              Copyright (c) 2000 Tmax Soft Co., Ltd           */
/*                   All Rights Reserved                        */
/*                                                              */
/* ------------------------------------------------------------ */

#ifndef _TMAX_HLINKAPI_H_
#define _TMAX_HLINKAPI_H_

#include <time.h>

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

/* DATA LOGGING TYPE */
#define TMAX_REQUEST            1
#define TMAX_RESPONSE           2
#define RGW_REQUEST             3
#define RGW_RESPONSE            4
#define BID_MESSAGE             5
#define ROP_MESSAGE             6
#define TMAX_PGMREQUEST         7     /* same TMAX_REQUEST */
#define TMAX_PGMRESPONSE        8     /* same TMAX_RESPONSE */

/* struct for data logging */
struct logheader {
    int    type;
    int    len;
    int    errcode;
    time_t time;
    int    seconds;
    char   luname[8];
};
typedef struct logheader LOGHEADER;

/* HOSTLINK type */
#define SVRTYPE_LU0             1
#define SVRTYPE_LU62S           2
#define SVRTYPE_LU62R           3
#define SVRTYPE_CTG             4
#define SVRTYPE_CSKL            5
#define SVRTYPE_IMSLSTN         6
#define SVRTYPE_IMSOTMA         7

/* LU62S server function type */
#define LU62S_DPL               1
#define LU62S_DTP               2

/* LU direction */
#define INBOUND_LU              1
#define OUTBOUND_LU             2

/* LInkline status */
#define INACTIVE                0
#define ACTIVE                  1

/* send status */
#define HOST_SEND               1
#define HOST_SEND_TIMEOUT       2

struct hlprocinfo {
    int    svrno;
    int    msgsize;
    int    function;
    int    cpc;
    int    buffering;
    int    innum;
    int    outnum;
    int    line_status;
    char   svrname[20];
    char   linkname[8];
    char   trxid[4];
    char   svrlist[128];
};
typedef struct hlprocinfo HLPROCINFO;

/* struct for host link lu info */
struct hlluinfo {
    char   luname[8];
    char   wsname[8];
    char   lutype[12];
    char   svcname[16];
    char   tpname[64];
    char   pluname[16];
    char   modename[8];
    int    status;            /* 0x50: LU-LU, 0x51: LU-SSCP, 0x52: DOWN, 
	                         0x53: CSDN,  0x54: ACTLU,   0x55: INACTLU, 
	                         0x56: NSPE */
    int    send;              /* 1 : host send (lu0) */
    int    direction;         /* 0 : inbound lu, 1=outbound lu (lu0) */
    int    session;           /* session number (lu6.2) */
    int    available;         /* available session number (lu6.2) */
    int    count;             /* process count */
};
typedef struct hlluinfo HLLUINFO;

/* struct for host link session info : lu6.2 */
struct hlsessinfo {
    int    status;            /* 1: READY, 2: BUSY */
    char   luname[8];
    char   local_tp[16];
    unsigned char tp_id[8];
    char   remote_tp[16];
    char   pgmname[8];
    char   wsname[8];
    unsigned int conv_id;
    time_t time;
};
typedef struct hlsessinfo HLSESSINFO;

#define HOST_TRANS_LENGTH    8
#define HOST_PROGRAM_LENGTH  8
#define HOST_USERID_SIZE     8
#define HOST_PASSWD_SIZE     8
#define TPGWINFO_SIZE        sizeof(struct tpgwinfo)

struct tpgwinfo {
    char  svc[XATMI_SERVICE_NAME_LENGTH];   /* relay or tpacall service name */
    char  trxid[HOST_TRANS_LENGTH];         /* host transaction id. */
    char  pgmname[HOST_PROGRAM_LENGTH];     /* host program name */
    char  userid[HOST_USERID_SIZE];         /* host user Id. */
    char  passwd[HOST_PASSWD_SIZE];         /* host user password */
    char  resvd[16];                        /* host program name */
};
typedef struct tpgwinfo TPGWINFO_T;

/* for openframe's header */
#define OFH_TOTAL_SIZE		16
#define OFH_TPN_OFFSET		0
#define OFH_TPN_SIZE		8

#if defined (__cplusplus)
extern "C" {
#endif

int __EXPORT tpgethlinksvr(int shmkey, int svrtype);
int __EXPORT tpgethlinkproc(int svrtype, HLPROCINFO *info);
HLLUINFO *__EXPORT tpgethlinkluinfo(char *svrname, int *lunum);
HLSESSINFO *__EXPORT tpgethlinkssinfo(char *luname, int *ssnum);

#if defined (__cplusplus)
}
#endif


#endif  /* _TMAX_HLINKAPI_H_ */

