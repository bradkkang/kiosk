/* --------------------- usrinc/entgw.h ----------------------- */
/*                                                              */
/*              Copyright (c) 2002 - 2004 Tmax Soft Co., Ltd    */
/*                   All Rights Reserved                        */
/*                                                              */
/* ------------------------------------------------------------ */

#ifndef _TMAX_ENTGW_H
#define _TMAX_ENTGW_H

#include        <usrinc/fbuf.h>
#include	<usrinc/tmaxrpc/base_fdl.h>

/*---------------   argument types  ---------------*/
/* put int/short/long/float/double/char */
#define TMAXRPC_PUT		0
/* put int/short/long/float/double/char/void fixed array (1-dim)*/
#define TMAXRPC_PUT_ARR		1
/* put int/short/long/float/double/char/void fixed array(2-dim)*/
#define TMAXRPC_PUT_ARR2D	2
/* put int/short/long/float/double/char/void constrained array 1-dim*/
#define TMAXRPC_PUT_CON		3
/* put int/short/long/float/double/char/void constrained array 2-dim*/
#define TMAXRPC_PUT_CON2D	4
/* put 1-dim null-terminated char array */
#define TMAXRPC_PUT_STR		5
/* put 2-dim null-terminated char array. only for tmax */
#define TMAXRPC_PUT_NSTR	6

/* get int/short/long/float/double/char */
#define TMAXRPC_GET		100
/* get int/short/long/float/double/char/void fixed array (1-dim)*/
#define TMAXRPC_GET_ARR		101
/* get int/short/long/float/double/char/void fixed array(2-dim)*/
#define TMAXRPC_GET_ARR2D	102
/* get int/short/long/float/double/char/void constrained array 1-dim*/
#define TMAXRPC_GET_CON		103
/* get int/short/long/float/double/char/void constrained array 2-dim*/
#define TMAXRPC_GET_CON2D	104
/* get 1-dim null-terminated char array */
#define TMAXRPC_GET_STR		105
/* get 2-dim null-terminated char array. only for tmax */
#define TMAXRPC_GET_NSTR	106
/* get adhoc result */
#define TMAXRPC_GET_SQLALL	107

#define TMAXRPC_END		-1

/* ------------------ gateway direction ------------------ */
#define OUT_ENT_SVR		0
#define IN_TMAX_SVR		1
#define SVR_ANYDIR		2
#define SVR_NOWHERE		-1


/* ------------------------------------------------------- */

typedef struct {
    int	 method;
    char *tag;
    FLDKEY key;
    char *tag2;
    char *tag3;
    int	 n, m;
} ent_arg_t;


typedef struct {
    int	 ind;
    char *funcname;
    ent_arg_t *argtab;
} ent_func_t;


typedef struct {
    int	 ind;
    int	 direction;
    char *svrname;
    ent_func_t *functab;
    void *table;
} ent_svr_t;

#ifdef _WIN32
int __EXPORT _tmax_reg_svrtab(ent_svr_t *ent_svrtab);
int __EXPORT _tmax_main(int argc, char *argv[]);
#endif
#endif
