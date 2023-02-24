
/* ----------------------- usrinc/Uunix.h --------------------- */
/*                                                              */
/*         Copyright (c) 2000 - 2002 Tmax Soft Co., Ltd         */
/*                   All Rights Reserved                        */
/*                                                              */
/* ------------------------------------------------------------ */

#ifndef _TMAX_UUNIX_H
#define _TMAX_UUNIX_H

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

/* Compatible Uunixerr values */

#define UUNIXMIN 0
#define UCLOSE	1
#define UCREAT	2
#define UEXEC	3
#define UFCTNL	4
#define UFORK	5
#define ULSEEK	6
#define UMSGCTL	7
#define UMSGGET	8
#define UMSGSND	9
#define UMSGRCV	10
#define UOPEN	11
#define UPLOCK	12
#define UREAD	13
#define USEMCTL	14
#define USEMGET	15
#define USEMOP	16
#define USHMCTL	17
#define USHMGET	18
#define USHMAT	19
#define USHMDT	20
#define USTAT	21
#define UWRITE	22
#define USBRK	23
#define USYSMUL 24
#define UWAIT	25
#define UKILL	26
#define UTIME	27
#define UMKDIR	28
#define ULINK	29
#define UUNLINK	30
#define UUNAME  31
#define UNLIST  32
#define UUNIXMAX 33


#if defined(__cplusplus)
extern "C" {
#endif

int *__EXPORT _tmget_compat_unixerrno_addr(void);
char *__EXPORT Ustrerror(int err);
void __EXPORT Uunix_err(char *msg);

#ifndef Uunixerr
#define Uunixerr (*_tmget_compat_unixerrno_addr())
#endif
extern char * Uunixmsg[];

#if defined(__cplusplus)
}
#endif

#endif
