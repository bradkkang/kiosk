
/* ----------------------- tuxinc/fml.h ----------------------- */
/*								*/
/*              Copyright (c) 2000 Tmax Soft Co., Ltd		*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef _TMAX_FML_H
#define _TMAX_FML_H

#include <tuxinc/atmi.h>
#include <usrinc/fbuf.h>

#ifdef _TUXEDO
#define FDL "FML"
#else
#define FDL "FIELD"
#endif

/* --- type definitions --- */
#define FBFR   FBUF
#define pFBFR  pFBUF
#define FLDID  FLDKEY
#if !defined(_TMAX_KERNEL) && !defined(_TMAX_UTIL)
#define Ferror (*_tmget_compat_Ferror_addr())
#else
#define Ferror fberror
#endif

typedef int FLDOCC;

/* --- size values --- */
#define MAXFBLEN	0xfffc		/* Maximum FBFR length */

#define FSTDXINT	16		/* Default indexing interval */
#define FMAXNULLSIZE	2660
#define FVIEWCACHESIZE	10
#define FVIEWNAMESIZE	33

/* operations presented to _Fmodidx function */
#define FADD	1
#define FDLMOD	2
#define FDEL	3

/* Flag options used in Fvstof() --- ? not found ? */
#define F_OFFSET	1
#define F_SIZE		2
#define F_PROP		4			/* P */
#define F_FTOS		8			/* S */
#define F_STOF		16			/* F */
#define F_BOTH		(F_STOF | F_FTOS)	/* S,F */
#define F_OFF		0			/* Z */
#define F_LENGTH        32                      /* L */
#define F_COUNT         64                      /* C */
#define F_NONE          128                 /* NONE flag for null value */

/* These are used in Fvstof() */
#define FCONCAT		FBCONCAT
#define FJOIN		FBJOIN
#define FOJOIN		FBOJOIN
#define FUPDATE		FBUPDATE

/* field types */
#define FLD_SHORT	SDL_SHORT
#define FLD_LONG	SDL_LONG
#define FLD_CHAR	SDL_CHAR
#define FLD_FLOAT	SDL_FLOAT
#define FLD_DOUBLE	SDL_DOUBLE
#define FLD_STRING	SDL_STRING
#define FLD_CARRAY	SDL_CARRAY
#define FLD_INT   	SDL_INT


/* invalid field id - returned from functions where field id not found */
#define BADFLDID 	(FLDID)BADFLDKEY
/* define an invalid field id used for first call to Fnext */
#define FIRSTFLDID 	(FLDID)FIRSTFLDKEY

/* --- Field Error Codes --- */
/* --- Edited by 2003/01/10 --- */
#define FMINVAL 	FBEMINNO/* bottom of error message codes */
#define FALIGN 		1 	/* fielded buffer not aligned */
#define FNOTFLD 	2 	/* buffer not fielded */
#define FNOSPACE 	3 	/* no space in fielded buffer */
#define FNOTPRES 	4 	/* field not present */
#define FBADFLD 	5 	/* unknown field number or type */
#define FTYPERR 	6 	/* illegal field type */
#define FEUNIX 		7 	/* UNIX system call error */
#define FBADNAME 	8 	/* unknown field name */
#define FMALLOC 	9 	/* malloc failed */
#define FSYNTAX 	10 	/* bad syntax in boolean expression */
#define FFTOPEN 	11 	/* cannot find or open field table */
#define FFTSYNTAX 	12 	/* syntax error in field table */
#define FEINVAL 	13 	/* invalid argument to function */
#define FBADTBL 	14 	/* destructive concurrent access to field table */
#define FBADVIEW 	15 	/* cannot find or get view */
#define FVFSYNTAX 	16 	/* syntax error in viewfile */
#define FVFOPEN 	17 	/* cannot find or open viewfile */
#define FBADACM 	18 	/* ACM contains negative value */
#define FNOCNAME 	19 	/* cname not found */
#define FEBADOP 	20 	/* invalid field type */
#define FMAXVAL  	FBEMAXNO/* top of error message codes */


/* ------- Field ID Mapping Functions --------- */
#define Fldid(name)     fbget_fldkey(name)
#define Fname(fldkey)   fbget_fldname(fldkey)
#define Fldno(fldkey)   fbget_fldno(fldkey)
#define Fldtype(fldkey) fbget_fldtype(fldkey)
#define Ftype(fldkey)   fbget_strfldtype(fldkey) 
#define Fmkfldid(type, no) fbmake_fldkey(type, no)
#define Fextread(fbuf, iop) fbextread(fbuf, iop)


/* ------- Buffer Allocation Functions ------ */
#define Fielded(fbuf)   fbisfbuf(fbuf)
#define Fneeded(count, len)   fbcalcsize(count, len)
#define Finit(fbuf, len)   fbinit(fbuf, len)
#define Falloc(count, len) fballoc(count, len)
#define Ffree(fbuf)        fbfree(fbuf)
#define Fsizeof(fbuf)      fbget_fbsize(fbuf)
#define Funused(fbuf)      fbget_unused(fbuf)
#define Fused(fbuf)        fbget_used(fbuf)
#define Frealloc(fbuf, ncount, nlen) fbrealloc(fbuf, ncount, nlen)

 
/* ------- field access and modification functions ----- */
#define Fadd(fbuf, fldkey, val, len)   fbput(fbuf, fldkey, val, len)
#define Fappend(fbuf, fldkey, val, len) fbput(fbuf, fldkey, val, len)
#define Fchg(fbuf, fldkey, nth, val, len) fbchg_tu(fbuf, fldkey, nth, val, len)
#define Fdel(fbuf, fldkey, nth)        fbdelete(fbuf, fldkey, nth)
#define Fdelall(fbuf, fldkey)          fbdelall(fbuf, fldkey)
#define Fdelete(fbuf, fldkey)          fbdelall_tu(fbuf, fldkey)
#define Ffind(fbuf, fldkey, nth, len)  fbgetval(fbuf, fldkey, nth, len)
#define Ffindlast(fbuf, fldkey, nth, len)  fbgetval_last_tu(fbuf, fldkey, nth, len)
#define Ffindocc(fbuf, fldkey, val, len) fbgetnth(fbuf, fldkey, val, len)
#define Fget(fbuf, fldkey, nth, loc, mlen) fbget_tu(fbuf, fldkey, nth, loc, mlen)
#define Fgetalloc(fbuf, fldkey, nth, elen) fbgetalloc_tu(fbuf, fldkey, nth, elen)
#define Fgetlast(fbuf, fldkey, nth, loc, mlen) fbgetlast_tu(fbuf, fldkey, nth, loc, mlen)
#define Fnext(fbuf, fldkey, nth, val, len) fbnext_tu(fbuf, fldkey, nth, val, len)
#define Fnum(fbuf)          fbfldcount(fbuf)
#define Foccur(fbuf, fldkey) fbkeyoccur(fbuf, fldkey)
#define Fpres(fbuf, fldkey, nth) fbispres(fbuf, fldkey, nth)
#define Fvals(fbuf, fldkey, nth) fbgetvals_tu(fbuf, fldkey, nth)
#define Fvall(fbuf, fldkey, nth) fbgetvall_tu(fbuf, fldkey, nth)
#define Flen(fbuf, fldkey, nth) fbgetlen(fbuf, fldkey, nth)


/* --------- Conversion Functions -------- */
#define CFadd(fbuf, fldkey, val, len, type) fbputt(fbuf, fldkey, val, len, type)
#define CFchg(fbuf, fldkey, nth, val, len, type) fbchg_tut(fbuf, fldkey, nth, val, len, type)
#define CFget(fbuf, fldkey, nth, loc, len, type) fbget_tut(fbuf, fldkey, nth, loc, len, type)
#define CFgetalloc(fbuf, fldkey, nth, type, elen) fbgetalloc_tut(fbuf, fldkey, nth, type, elen)
#define CFfind(fbuf, fldkey, nth, len, type)  fbgetvalt(fbuf, fldkey, nth, len, type)
#define CFfindocc(fbuf, fldkey, val, len, type) fbgetntht(fbuf, fldkey, val, len, type)
#define Ftypcvt(tolen, totype, fromval, fromtype, fromlen) fbtypecvt(tolen, totype, fromval, fromtype, fromlen)


/* --------- VIEWS Functions ------ */
#define Fvftos(fbuf, cstruct, stname)  fbftos(fbuf, cstruct, stname)
#define Fvstof(fbuf, cstruct, mode, stname)  fbstof(fbuf, cstruct, mode, stname)
#define Fvnull(cstruct, cname, nth, stname) fbsnull(cstruct, cname, nth, stname)
#define Fvsinit(cstruct, stname) fbstinit(cstruct, stname)
#define Fvselinit(cstruct, cname, stname) fbstelinit(cstruct, cname, stname)


/* --------- Buffer Operation Functions ------ */
#define Fmove(dest, src)       fbbufop(dest, src, FBMOVE)
#define Fcpy(dest, src)        fbbufop(dest, src, FBCOPY)
#define Fcmp(dest, src)        fbbufop(dest, src, FBCOMP)
#define Fconcat(dest, src)     fbbufop(dest, src, FBCONCAT)
#define Fjoin(dest, src)       fbbufop(dest, src, FBJOIN)
#define Fojoin(dest, src)      fbbufop(dest, src, FBOJOIN)
#define Fupdate(dest, src)     fbbufop(dest, src, FBUPDATE)
#define Fproj(dest, fldkey)    fbbufop_proj(dest, NULL, fldkey)
#define Fprojcpy(dest, src, fldkey) fbbufop_proj(dest, src, fldkey)


/* --------- Indexing Functions ------ */
#define Fidxused(fbuf)         0
#define Findex(fbuf, intval)   1 
#define Frstrindex(fbuf, numidx) 1
#define Funindex(fbuf)         0


/* --------- I/O Functions ------ */
#define Fread(fbuf, iop)      fbread(fbuf, iop)
#define Fwrite(fbuf, iop)     fbwrite(fbuf, iop)
#define Fchksum(fbuf)         -1
#define Fprint(fbuf)          fbprint(fbuf)
#define Ffprint(fbuf, iop)        fbfprint(fbuf, iop)
#if !defined(_TMAX_KERNEL) && !defined(_TMAX_UTIL)
#define Fstrerror(errno)	(_tmget_compat_strFerror(errno))
#define F_error(msg)   		print_Ferror(msg)
#define getFerror()    		getFerror2()
#else
#define Fstrerror(errno)      fbstrerror(errno)
#define F_error(msg)   print_fberror(msg)
#define getFerror()    getfberror()
#endif

/* -------- Etc Functions -------- */
/* add string value to buf */
#define Fadds(fb,id,uval) CFadd((fb),(id),(uval),0,FLD_STRING)

/* change ocurrance .. supplied val is a string */
#define Fchgs(fb,id,oc,uval) CFchg((fb),(id),(oc),(uval),0,FLD_STRING)

/* get value, as a string into usr buffer */
#define Fgets(fb,id,oc,ubuf) CFget((fb),(id),(oc),(ubuf),0,FLD_STRING)

/* get value, as a string, into malloced buffer */
#define Fgetsa(fb,id,oc,xtra) ((char *)CFgetalloc((fb),(id),(oc), FLD_STRING,xtra))

/* return pointer to string value of field */
#define Ffinds(fb,id,oc) ((char *)CFfind((fb),(id),(oc),0,FLD_STRING))

/* unload fldkey table */
#define Fnmid_unload()	fbnmkey_unload()
#define Fidnm_unload()	fbkeynm_unload()

#endif
