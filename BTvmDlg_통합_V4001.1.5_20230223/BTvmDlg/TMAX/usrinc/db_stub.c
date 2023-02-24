
/* --------------------- usrinc/db_stub.c --------------------- */
/*                                                              */
/*              Copyright (c) 2000 - 2004 Tmax Soft Co., Ltd    */
/*                   All Rights Reserved                        */
/*                                                              */
/* ------------------------------------------------------------ */


#include        <usrinc/xa.h>

extern struct xa_switch_t xaosw;   /* for oracle */

int
_tmax_xasw_init(struct xa_switch_t *xaswp, int dynamic)
{
	*xaswp = xaosw;

	return 1;
}
