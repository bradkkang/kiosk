
/* ------------------------- usrinc/sdl.h --------------------- */
/*								*/
/*              Copyright (c) 2000 - 2004 Tmax Soft Co., Ltd	*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef _TMAX_SDL_H
#define _TMAX_SDL_H

#include <stdio.h>
#include <stdlib.h>
#ifndef _CE_MODULE
#include <sys/types.h>
#endif
#ifdef _WIN32
#ifdef _CE_MODULE
#include <winsock.h>
#else
#include <winsock2.h>
#endif  /* _CE_MODULE */
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
#ifndef THLVAR
#define THLVAR
#endif

#define MAX_FIELD_LENGTH 32

#ifndef TMSTYPELEN 
#define TMSTYPELEN   16
#endif

#define SDL_CHAR      1 
#define SDL_SHORT     2
#define SDL_INT       3
#define SDL_LONG      4
#define SDL_FLOAT     5
#define SDL_DOUBLE    6
#define SDL_STRING    7
#define SDL_CARRAY    8
#define SDL_STRUCT    9   /* for nested struct type */
#define SDL_TYPEDEF   10  /* typedef: same as struct */

#define SDL_CHAR_SIZE     1
#define SDL_SHORT_SIZE    2
#define SDL_INT_SIZE      4
#define SDL_LONG_SIZE     4
#define SDL_FLOAT_SIZE    4
#define SDL_DOUBLE_SIZE   8

#define HOST_SHORT_ALIGN    sizeof(short)
#define HOST_INT_ALIGN      sizeof(int)
#define HOST_LONG_ALIGN     sizeof(long)
#define HOST_FLOAT_ALIGN    sizeof(float)
#if defined(_DOUBLE_4B_ALIGN)
#define HOST_DOUBLE_ALIGN   4
#else
#define HOST_DOUBLE_ALIGN   sizeof(double)
#endif

#define SDL_SHORT_ALIGN    2
#define SDL_INT_ALIGN      4
#define SDL_LONG_ALIGN     4
#define SDL_FLOAT_ALIGN    4
#define SDL_DOUBLE_ALIGN   8


#define SDL_ENCODE "sdl_encode_"
#define SDL_DECODE "sdl_decode_"

#define MAX_NVAL_SIZE           32  /* nval added */

#ifdef _MS_DOS         /* 16 bit computing */
typedef unsigned short         sdl_short;   /* must be 2 bytes */
typedef unsigned long          sdl_int;     /* must be 4 bytes */
typedef unsigned long          sdl_long;    /* must be 4 bytes */

#else
typedef unsigned short         sdl_short;   /* must be 2 bytes */
typedef unsigned int           sdl_int;     /* must be 4 bytes */
typedef unsigned int           sdl_long;    /* must be 4 bytes */

#endif

struct _sdl_hdr_s {
   int    magic;
   int    subtype_count;
   int    field_count;
};

struct _sdl_struct_s {
    char subtype[TMSTYPELEN];
    char *(*sdl_encode) (char *, char *);
    char *(*sdl_decode) (char *, char *);
    int  starti; 
    int  endi; 
    int  fcount;   /* field count: char[5] = 1, int[5] = 5 */
    long  hsize;   /* host size */
    long  nsize;   /* network size */
};   

struct _sdl_field_s {
    char name[MAX_FIELD_LENGTH];
    int  fldkey;
    int  type; 
    char nval[MAX_NVAL_SIZE];  /* null value added */
    int  nsti;    /* for nested structure, subtype index */
    int  asize;   /* array size: for char[10][20], it is 200   */
    int  asize2;  /* for char[10][20], it is 20: needed for PB */
    int  offseth; /* offset in host */
    int  offsetn; /* offset in NW */ 
};   


#ifndef _MTHRCLI_MODULE
extern THLVAR int  _sdl_table_size;
extern THLVAR int  _sdl_table_size2;
extern THLVAR struct _sdl_struct_s *_sdl_table;
extern THLVAR struct _sdl_struct_s _sdl_table2[];

extern THLVAR int  _sdl_field_table_size;
extern THLVAR int  _sdl_field_table_size2;
extern THLVAR struct _sdl_field_s *_sdl_field_table;
extern THLVAR struct _sdl_field_s _sdl_field_table2[];
#endif

/* 
    Internal functions: ONLY BE CALLED FROM AUTOMATICALLY 
    GENERATED STUB FILES. DO NOT DIRECTLY CALL THESE FUNCTIONS.
 */
int _get_subtype_len(char *subtype, int *fcount);
char *_sdl_encode(int i, char *dp1, char *dp2);
char *_sdl_decode(int i, char *dp1, char *dp2);

#endif
