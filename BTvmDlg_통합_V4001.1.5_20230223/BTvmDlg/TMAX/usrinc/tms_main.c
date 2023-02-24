
/* --------------------- usrinc/tms_main.c -------------------- */
/*                                                              */
/*              Copyright (c) 2000 - 2004 Tmax Soft Co., Ltd    */
/*                   All Rights Reserved                        */
/*                                                              */
/* ------------------------------------------------------------ */

#include <usrinc/tmaxapi.h>
#include <usrinc/tx.h>

extern __declspec(dllimport) int (*_tmax_xasw_init_func)(struct xa_switch_t *xaswp, int dynamic);
extern int _tmax_xasw_init(struct xa_switch_t *xaswp, int dynamic);

int
main(int argc, char *argv[])
{
	_tmax_xasw_init_func = _tmax_xasw_init;
	return _tmax_main(argc, argv);
}

