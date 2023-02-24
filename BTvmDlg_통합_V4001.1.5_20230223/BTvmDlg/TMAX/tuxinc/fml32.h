
/* ------------------------- tuxinc/fml32.h ------------------- */
/*								*/
/*              Copyright (c) 2000 Tmax Soft Co., Ltd		*/
/*                   All Rights Reserved  			*/
/*								*/
/* ------------------------------------------------------------ */

#ifndef _TMAX_FML32_H
#define _TMAX_FML32_H

#include <tuxinc/fml.h>

#define Fbfr32     Fbfr
#define FBFR32     FBFR

#define FMLTYPE32 "FML"

#define Fdel32     Fdel
#define Fpres32    Fpres
#define Fadds32    Fadds
#define Fchgs32    Fchgs
#define Fgets32    Fgets
#define Fgetsa32   Fgetsa
#define Ffinds32   Ffinds


#define Fread32       Fread
#define Fwrite32      Fwrite
#define CFadd32       CFadd
#define CFchg32       CFchg
#define CFfind32      CFfind
#define CFfindocc32   CFfindocc
#define CFget32       CFget
#define CFgetalloc32  CFgetalloc
#define F_error32     F_error
#define Fadd32        Fadd
#define Falloc32      Falloc
#define Fappend32     Fappend
#define Frealloc32    Frealloc
#define Ffree32       Ffree
#define Fboolev32     Fboolev
#define Fvboolev32    Fvboolev
#define Ffloatev32    Ffloatev
#define Fvfloatev32   Fvfloatev
#define Fboolpr32     Fboolpr
#define Fvboolpr32    Fvboolpr
#define Fchg32        Fchg
#define Fchksum32     Fchksum
#define Fcmp32        Fcmp
#define Fconcat32     Fconcat
#define Fcpy32        Fcpy
#define Fdelall32     Fdelall
#define Fdelete32     Fdelete
#define Fextread32    Fextread
#define Ffind32       Ffind
#define Fvals32       Fvals
#define Fvall32       Fvall
#define Ffindocc32    Ffindocc
#define Fget32        Fget
#define Fgetalloc32   Fgetalloc
#define Fldtype32     Fldtype
#define Fldno32       Fldno
#define Fielded32     Fielded
#define Fneeded32     Fneeded
#define Fused32       Fused
#define Fidxused32    Fidxused
#define Funused32     Funused
#define Fsizeof32     Fsizeof
#define Fmkfldid32    Fmkfldid
#define Fieldlen32    Fieldlen
#define Funindex32    Funindex
#define Frstrindex32  Frstrindex
#define Findex32      Findex
#define Finit32       Finit
#define Fjoin32       Fjoin
#define Fojoin32      Fojoin
#define Ffindlast32   Ffindlast
#define Fgetlast32    Fgetlast
#define Flen32        Flen
#define Fmove32       Fmove
#define Fnext32       Fnext
#define Fldid32       Fldid
#define Fname32       Fname
#define Ftype32       Ftype
#define Fnmid_unload32 Fnmid_unload
#define Fidnm_unload32 Fidnm_unload
#define Fnum32        Fnum
#define Foccur32      Foccur
#define Fprint32      Fprint
#define Ffprint32     Ffprint
#define Fproj32       Fproj
#define Fprojcpy32    Fprojcpy
#define Ftypcvt32     Ftypcvt
#define Fupdate32     Fupdate
#define Fvopt32       Fvopt
#define Fvsinit32     Fvsinit
#define Fvnull32      Fvnull
#define Fvselinit32   Fvselinit
#define Fvftos32      Fvftos
#define Fvrefresh32   Fvrefresh
#define Fvstof32      Fvstof
#define Fboolco32     Fboolco
#define Fvboolco32    Fvboolco
#define Fstrerror32   Fstrerror
#define Fvttos32      Fvttos
#define Fvstot32      Fvstot
#define Flen32        Flen

extern __EXPORT int F16to32 ((FBFR32 *dbfr, FBFR *sbfr));
extern __EXPORT int F32to16 ((FBFR *dbfr, FBFR32 *sbfr));
#endif

