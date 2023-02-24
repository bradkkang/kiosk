
/* --------------------- usrinc/fbuf.h ------------------------ */
/*								*/
/*              Copyright (c) 2000 - 2004 Tmax Soft Co., Ltd	*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef _TMAX_FBUF_H
#define _TMAX_FBUF_H

#include  <stdio.h>
#include  <usrinc/sdl.h>

/* field types */
#define FB_CHAR	        SDL_CHAR
#define FB_SHORT	SDL_SHORT
#define FB_INT   	SDL_INT
#define FB_LONG	        SDL_LONG
#define FB_FLOAT	SDL_FLOAT
#define FB_DOUBLE	SDL_DOUBLE
#define FB_STRING	SDL_STRING
#define FB_CARRAY	SDL_CARRAY

#define BADFLDKEY       0
#define FIRSTFLDKEY     0

/* ---- type definition --- */
typedef int     FLDKEY;
typedef int     FLDLEN;
typedef struct  fbuf_s FBUF;
typedef FBUF*   pFBUF;


/* ----- fb op mode ----- */
#define FBMOVE          1
#define FBCOPY          2
#define FBCOMP          3
#define FBCONCAT        4
#define FBJOIN          5
#define FBOJOIN         6
#define FBUPDATE        7

/* ------- fberror code ----- */
#define FBEMINNO	0
#define FBEBADFB        3
#define FBEINVAL	4
#define FBELIMIT	5
#define FBENOENT	6
#define FBEOS	        7
#define FBEBADFLD       8
#define FBEPROTO	9
#define FBENOSPACE	10
#define FBEMALLOC	11
#define FBESYSTEM	12
#define FBETYPE	        13
#define FBEMATCH	14
#define FBEBADSTRUCT	15
/* --- reserved for tux compat. --- */
#define FBEALIGN	1
#define FBEFTSYNTAX	2
#define FBEVFSYNTAX	16
#define FBEVFOPEN	17
#define FBEBADACM	18
#define FBENOCNAME	19
#define	FBEBADTBL	20
#define FBEMAXNO	21

#define FBGET(fbuf, fldid, loc)    fbget(fbuf, fldid, loc, 0, 0)
#define FBPUT(fbuf, fldid, val)    fbget(fbuf, fldid, loc, -1, 0)

/* --------- External declarations ----------  */
#if defined(_WIN32) || defined(_TMAX_MTLIB) || defined(_MCONTEXT)
#if defined(__cplusplus)
extern "C" {
#endif
/*
   Internal functions: ONLY BE CALLED FROM AUTOMATICALLY
   GENERATED STUB FILES. DO NOT DIRECTLY CALL THESE FUNCTIONS.
 */
int *__EXPORT _tmget_fberror_addr(void);
#ifdef _TMAX_KERNEL
extern THLVAR int fberror;
#else
#define fberror	 (*_tmget_fberror_addr())
#endif

#if defined(__cplusplus)
}
#endif

#else
extern int fberror;
#endif


/* ------- FBUF API ------- */
#if defined(__cplusplus)
extern "C" {
#endif

int  __EXPORT getfberror(void);
int  __EXPORT getfberrno(void);
int __EXPORT fbget(FBUF *fbuf, FLDKEY fldkey, char *loc, int *fldlen);
int __EXPORT fbput(FBUF *fbuf, FLDKEY fldkey, char *value, int fldlen);
int __EXPORT fbinsert(FBUF *fbuf, FLDKEY fldkey, int nth, char *value, int fldlen);
int __EXPORT fbupdate(FBUF *fbuf, FLDKEY fldkey, int nth, char *value, int fldlen);
int __EXPORT fbdelete(FBUF *fbuf, int fldkey, int nth);
char *__EXPORT  fbgetval(FBUF *fbuf, FLDKEY fldkey, int nth, FLDLEN *len);
int __EXPORT fbgetnth(FBUF *fbuf, FLDKEY fldkey, char *value, FLDLEN len);
int __EXPORT fbkeyoccur(FBUF *fbuf, FLDKEY fldkey);
int __EXPORT fbgetf(FBUF *fbuf, FLDKEY fldkey, char *loc, int *fldlen, int *pos);
int __EXPORT fbgetc(FBUF *fbuf, FLDKEY fldkey, char *loc, FLDLEN *len, int *pos, int totype);

int __EXPORT fbdelall(FBUF *fbuf, int fldkey);
int __EXPORT fbfldcount(FBUF *fbuf);
int __EXPORT fbispres(FBUF *fbuf, FLDKEY fldkey, int nth);
char *__EXPORT fbgetvals(FBUF *fbuf, FLDKEY fldkey, int nth);
int __EXPORT fbgetvali(FBUF *fbuf, FLDKEY fldkey, int nth);

char *__EXPORT fbtypecvt(FLDLEN *tolen, int totype, char *fromval, int fromtype, 
         FLDLEN fromlen);
int __EXPORT fbputt(FBUF *fbuf, FLDKEY fldkey, char *value, FLDLEN len, int type);
char *__EXPORT fbgetvalt(FBUF *fbuf, FLDKEY fldkey, int nth, FLDLEN *len, int totype);
int __EXPORT fbgetntht(FBUF *fbuf, FLDKEY fldkey, char *value, FLDLEN len, int fromtype);

FLDKEY  __EXPORT fbget_fldkey(char *name);
char *__EXPORT fbget_fldname(FLDKEY fldkey);
int  __EXPORT fbget_fldno(FLDKEY fldkey);
int  __EXPORT fbget_fldtype(FLDKEY fldkey);
char *__EXPORT fbget_strfldtype(FLDKEY fldkey);
FLDKEY __EXPORT fbmake_fldkey(int type, int no);
void __EXPORT fbnmkey_unload();
void __EXPORT fbkeynm_unload();

int __EXPORT fbisfbuf(FBUF *fbuf);
long __EXPORT fbcalcsize(int count, FLDLEN datalen);
int __EXPORT fbinit(FBUF *fbuf, FLDLEN buflen);
FBUF *__EXPORT  fballoc(int count, int len);
int __EXPORT fbfree(FBUF *fbuf);
long __EXPORT fbget_fbsize(FBUF *fbuf);
long __EXPORT fbget_unused(FBUF *fbuf);
long __EXPORT fbget_used(FBUF *fbuf);
FBUF *__EXPORT  fbrealloc(FBUF *fbuf, int ncount, int nlen);

int __EXPORT fbbufop(FBUF *dest, FBUF *src, int mode);
int __EXPORT fbbufop_proj(FBUF *dest, FBUF *src, FLDKEY *fldkey);

int __EXPORT fbread(FBUF *fbuf, FILE *iop);
int __EXPORT fbwrite(FBUF *fbuf, FILE *iop);
int __EXPORT fbprint(FBUF *fbuf);
int __EXPORT fbfprint(FBUF *fbuf, FILE *iop);

int __EXPORT fbchg_tu(FBUF *fbuf, FLDKEY fldkey, int nth, char *value, int fldlen);
int __EXPORT fbdelall_tu(FBUF *fbuf, int *fldkey);
char *__EXPORT fbgetval_last_tu(FBUF *fbuf, FLDKEY fldkey, int *nth, FLDLEN *len);
int __EXPORT fbget_tu(FBUF *fbuf, FLDKEY fldkey, int nth, char *loc, int *maxlen);
char *__EXPORT fbgetalloc_tu(FBUF *fbuf, FLDKEY fldkey, int nth, int *extralen);
int __EXPORT fbgetlast_tu(FBUF *fbuf, FLDKEY fldkey, int *nth, char *loc, int *maxlen);
int __EXPORT fbnext_tu(FBUF *fbuf, FLDKEY *fldkey, int *nth, char *value, int *len);
char *__EXPORT fbgetvals_tu(FBUF *fbuf, FLDKEY fldkey, int nth);
long __EXPORT fbgetvall_tu(FBUF *fbuf, FLDKEY fldkey, int nth);
int __EXPORT fbchg_tut(FBUF *fbuf, FLDKEY fldkey, int nth, char *value, FLDLEN len, 
         int type);
int __EXPORT fbget_tut(FBUF *fbuf, FLDKEY fldkey, int nth, char *loc, FLDLEN *len, 
         int type);
char *__EXPORT fbgetalloc_tut(FBUF *fbuf, FLDKEY fldkey, int nth, int totype, 
          FLDLEN *extralen);
int  __EXPORT fbgetlen(FBUF *fbuf, FLDKEY fldkey, int nth);

int __EXPORT fbftos(FBUF *fbuf, char *cstruct, char *stname);
int __EXPORT fbstof(FBUF *fbuf, char *cstruct, int mode, char *stname);
int __EXPORT fbsnull(char *cstruct, char *cname, int nth, char *stname);
int __EXPORT fbstinit(char *cstruct, char *stname);
int __EXPORT fbstelinit(char *cstruct, char *cname, char *stname);

char *__EXPORT fbstrerror(int err_no);

int __EXPORT fbextread(FBUF *fbuf, FILE *iop);

char *__EXPORT  _tmget_compat_strFerror(int err_no);
int *__EXPORT  _tmget_compat_Ferror_addr(void);

/*
   Internal functions: ONLY BE CALLED FROM AUTOMATICALLY
   GENERATED STUB FILES. DO NOT DIRECTLY CALL THESE FUNCTIONS.
 */
char *__EXPORT _find_matching_fldkey(FBUF *fbuf, FLDKEY fldkey, int nth);
int __EXPORT _get_alen_from_curpos(char *cur);
int __EXPORT _get_fldlen_from_curpos(char *cur);

#if defined(__cplusplus)
}
#endif


#endif
