
/* ----------------------- tuxinc/macro.h --------------------- */
/*								*/
/*              Copyright (c) 2000 Tmax Soft Co., Ltd		*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef _TMAX_TUXFML_H
#define _TMAX_TUXFML_H

#if defined (__cplusplus)
#include <iostream.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <tuxinc/atmi.h>
#include <tuxinc/fml32.h>
#include <usrinc/userlog.h>
#include <tuxinc/Usysflds.h>

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

/* ------ macro ------ */
#define GET(x,y,z)	{ \
	*(z) = 0x00; \
	Fget((FBFR*)(transf), (x), (y), (char*)(z), NULL); }

#define PUT(x,y,z)	{ \
	Fchg((FBFR*)(transf), (x), (y), (char*)(z), 0); }


#define GETVAR(x,y,z)	{ \
	(z.arr)[0] = 0x00; \
	Fget((FBFR*)(transf), (x), (y), (char*)(z.arr), NULL); \
	(z.len) = strlen((char*)(z.arr)); \
	(z.arr)[(z.len)] = 0x00; }

#define PUTVAR(x,y,z)	{ \
	(z.arr)[(z.len)] = 0x00; \
	Fchg((FBFR*)(transf), (x), (y), (char*)(z.arr), 0); }

#define TPRETURN(x,y)	{ \
	Fchg((FBFR*)(transf), STATLIN, 0, (char*)(x), 0); \
	tpreturn(TPSUCCESS, (y), (char*)(transf), 0, 0); }

#define TPRETURN_ERROR(x,y)	{ \
	Fchg((FBFR*)(transf), STATLIN, 0, (char*)(x), 0); \
	tpreturn(TPFAIL, (y), (char*)(transf), 0, 0); }

#endif
