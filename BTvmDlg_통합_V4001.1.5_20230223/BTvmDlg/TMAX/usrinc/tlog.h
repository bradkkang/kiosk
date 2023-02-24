
/* --------------------- usrinc/tlog.h ------------------------ */
/*								*/
/*              Copyright (c) 2003 - 2004 Tmax Soft Co., Ltd	*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef TMAX_TLOG_H
#define TMAX_TLOG_H

#include <sys/types.h>
#include <errno.h>
#ifndef _WIN32
#include <sys/time.h>
#endif
#include <time.h>

/* ----- tloglib macros ----- */
#ifndef NULL
#define NULL	0L
#endif
#define GATEWAY_NAME_SIZE	16

/* ----- modified from XA spec ----- */
#define TLOG_XIDDATASIZE     128     /* size in bytes */
#define TLOG_MAXGTRIDSIZE    64      /* maximum size in bytes of gtrid */
#define TLOG_MAXBQUALSIZE    64      /* maximum size in bytes of bqual */

typedef struct {
    long formatID;          /* format identifier */
    long gtrid_length;      /* value not to exceed 64 */
    long bqual_length;      /* value not to exceed 64 */
    char data[TLOG_XIDDATASIZE];
} TXID;

typedef struct {
    int formatID;          /* format identifier */
    int gtrid_length;      /* value not to exceed 64 */
    int bqual_length;      /* value not to exceed 64 */
    char data[TLOG_XIDDATASIZE];
} TXID32;

typedef struct {
    int formatID[2];          /* format identifier */
    int gtrid_length[2];      /* value not to exceed 64 */
    int bqual_length[2];      /* value not to exceed 64 */
    char data[TLOG_XIDDATASIZE];
} TXID64;

/* ----- tlog type flags ----- */
#define TLOG_LOCAL		0	/* local tmax domain txlog */
#define TLOG_REMOTE		1	/* tmax domain gateway txlog */
#define TLOG_NONTMAX		2	/* non-tmax domain gateway txlog */

/* ----- tlog decision ----- */
#define TXDEC_INVALID		-1	/* invalid entry */
#define TXDEC_COMMIT		0	/* commit */
#define TXDEC_ROLLBACK		1	/* rollback */
#define TXDEC_ABNORMAL_ROLLBACK	2	/* rollback due to svr/cli down or
					   failure during the prepare phase */
#define TXDEC_REMOTE		3	/* tx initiated from a remote domain */
#define TXDEC_PREPARE		4	/* prepare phase */

/* ----- tlog_find flags ----- */
#define TLOG_START		0x0000	/* read from the start */
#define TLOG_NEXT		0x0001	/* get next log entry */
#define TLOG_TIME		0x0002	/* find an entry logged later than or
					   equal to the ltime field */
#define TLOG_XID		0x0004	/* find an entry with matching xid */
#define TLOG_REMOTE_XID		0x0008	/* find an entry with matching 
					   remote_xid */
#define TLOG_GATEWAY		0x0010	/* find an entry from the matching
					   gateway_name field */
#define TLOG_DECISION		0x0020	/* find an entry with same decision */

/* ----- tlog API return value ----- */
#define TLOG_OK			0	/* non-negative value means OK */
#define TLOG_INVAL		-1	/* invalid arguments */
#define TLOG_NOTFOUND		-2	/* not found */
#define TLOG_ESYSTEM		-3	/* system error, refer to errno */

/* ----- tlog_file_t flag value ----- */
#define TLOG_EOF		0x00000001
#define TLOG_BYTESWAP		0x00000002
#define TLOG_TIME64		0x00000004
#define TLOG_64BIT		0x00000008


typedef struct {
    int	decision;
    TXID xid;
    time_t ltime;
    /* following fields have meaning only for TLOG_REMOTE & TLOG_NONTMAX */
    TXID remote_xid;
    char gateway_name[GATEWAY_NAME_SIZE];
} tlog_entry_t;


/* tlog_file_t is for internal use only. do not reference or modify
   information contained in tlog_file_t */
typedef struct {
    int fd;
    int type;
    int starti;
    int	curi;
    int num_entry;
    int flag;
    time_t ltime;
    int magic;
} tlog_file_t;


/* ------------------- function prototypes ------------------- */
int tlog_open(char *name, tlog_file_t *log, int flags);
int tlog_close(tlog_file_t *log);
int tlog_find(tlog_file_t *log, tlog_entry_t *entry, int flags);
int tlog_nodeno(TXID *xid);

#endif	/* TMAX_TLOG_H */
