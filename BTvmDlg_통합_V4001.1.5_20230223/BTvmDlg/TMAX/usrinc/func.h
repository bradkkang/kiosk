
/* ---------------------- usrinc/func.h ----------------------- */
/*								*/
/*              Copyright (c) 2000 - 2004 Tmax Soft Co., Ltd	*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef TMAX_FUNC_H
#define TMAX_FUNC_H

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

typedef struct { /* header from clh to server */
  char  *name;
  void  (__EXPORT *func) (tp_ms_inf_t *info, 
		  tp_ms_area_t *input,
		  tp_ms_area_t *work,
		  tp_ms_area_t *output,
		  tp_ms_area_t *save);
  int   svci;
} _func_t;

extern _func_t _func_tab[];
extern int _func_tab_size;

#endif
