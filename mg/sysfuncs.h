/* System dependent definitions for GNU tar.
   Copyright (C) 1994 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * [TS:Aug/95]
 * Based on code from system.h file in GNU's tar package 
 *
 */

#ifndef SYSFUNCS_H
#define SYSFUNCS_H
#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility push(default)
#endif

#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* Declare alloca.  AIX requires this to be the first thing in the file.  */

#ifdef __GNUC__
#define alloca __builtin_alloca
#else
#if HAVE_ALLOCA_H
#include <alloca.h>
#else
#ifdef _AIX
#pragma alloca
#else
#ifndef alloca
char *alloca ();
#endif
#endif
#endif
#endif

#include <sys/types.h>

/* ------------------------------------------------- */
/* [TS:Aug/95]
 * Some extra include files that mg needs
 */
#include <stdlib.h>
#include <math.h>

#ifndef log2
# ifndef M_LN2
#  define M_LN2 0.69314718055994530942
# endif
# define log2(x) (log(x)/M_LN2)
#endif


#ifndef HAVE_SETBUFFER
# define setbuffer(f, b, s)  setvbuf(f, b, _IOFBF, s)
#endif

#ifdef __MSDOS__
# define SHORT_SUFFIX 1
#endif



#include <assert.h>

/* ------------------------------------------------- */

/* Declare a generic pointer type.  */
#if __STDC__ || defined(__TURBOC__)
#define voidstar void *
#else
#define voidstar char *
#endif

/* Declare ISASCII.  */

#include <ctype.h>

#if STDC_HEADERS
#define ISASCII(Char) 1
#else
#ifdef isascii
#define ISASCII(Char) isascii (Char)
#else
#if HAVE_ISASCII
#define ISASCII(Char) isascii (Char)
#else
#define ISASCII(Char) 1
#endif
#endif
#endif


/* ------------------------------------------------- */
/* [TS:Aug/95] 
 * The following is copied from GNU Autoconf.Info (1.1)   
 * It sets up the approriate string and memory functions.
 */

#if STDC_HEADERS || HAVE_STRING_H
#include <string.h>
   /* An ANSI string.h and pre-ANSI memory.h might conflict */
#if !STDC_HEADERS && HAVE_MEMORY_H
#include <memory.h>
#endif /* not STDC_HEADERS and HAVE_MEMORY_H */
#ifndef index
#define index strchr
#endif
#ifndef rindex
#define rindex strrchr
#endif
#ifndef bcopy
#define bcopy(s, d, n) memcpy((d), (s), (n))
#endif
#ifndef bcmp
#define bcmp(s1, s2, n) memcmp((s1), (s2), (n))
#endif
#ifndef bzero
#define bzero(s, n) memset((s), 0, (n))
#endif
#else /* not STDC_HEADERS and not HAVE_STRING_H */
#include <strings.h>
   /* memory.h and strings.h conflict on some systems */
#endif
/* ------------------------------------------------- */


/* Declare errno.  */

#include <errno.h>
#ifndef errno
extern int errno;
#endif

/* Declare open parameters.  */

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#else
#include <sys/file.h>
#endif

#ifndef	O_BINARY
#define O_BINARY 0
#endif
#ifndef O_CREAT
#define O_CREAT 0
#endif
#ifndef	O_NDELAY
#define O_NDELAY 0
#endif
#ifndef	O_RDONLY
#define O_RDONLY 0
#endif
#ifndef O_RDWR
#define O_RDWR 2
#endif

/* Declare file status routines and bits.  */

#include <sys/stat.h>

#ifdef STAT_MACROS_BROKEN
#undef S_ISBLK
#undef S_ISCHR
#undef S_ISDIR
#undef S_ISFIFO
#undef S_ISLNK
#undef S_ISMPB
#undef S_ISMPC
#undef S_ISNWK
#undef S_ISREG
#undef S_ISSOCK
#endif

/* On MSDOS, there are missing things from <sys/stat.h>.  */
#ifdef __MSDOS__
#define S_ISUID 0
#define S_ISGID 0
#define S_ISVTX 0
#endif

#ifndef S_ISREG			/* POSIX.1 stat stuff missing */
#define mode_t unsigned short
#endif
#if !defined(S_ISBLK) && defined(S_IFBLK)
#define S_ISBLK(Mode) (((Mode) & S_IFMT) == S_IFBLK)
#endif
#if !defined(S_ISCHR) && defined(S_IFCHR)
#define S_ISCHR(Mode) (((Mode) & S_IFMT) == S_IFCHR)
#endif
#if !defined(S_ISDIR) && defined(S_IFDIR)
#define S_ISDIR(Mode) (((Mode) & S_IFMT) == S_IFDIR)
#endif
#if !defined(S_ISREG) && defined(S_IFREG)
#define S_ISREG(Mode) (((Mode) & S_IFMT) == S_IFREG)
#endif
#if !defined(S_ISFIFO) && defined(S_IFIFO)
#define S_ISFIFO(Mode) (((Mode) & S_IFMT) == S_IFIFO)
#endif
#if !defined(S_ISLNK) && defined(S_IFLNK)
#define S_ISLNK(Mode) (((Mode) & S_IFMT) == S_IFLNK)
#endif
#if !defined(S_ISSOCK) && defined(S_IFSOCK)
#define S_ISSOCK(Mode) (((Mode) & S_IFMT) == S_IFSOCK)
#endif
#if !defined(S_ISMPB) && defined(S_IFMPB)	/* V7 */
#define S_ISMPB(Mode) (((Mode) & S_IFMT) == S_IFMPB)
#define S_ISMPC(Mode) (((Mode) & S_IFMT) == S_IFMPC)
#endif
#if !defined(S_ISNWK) && defined(S_IFNWK)	/* HP/UX */
#define S_ISNWK(Mode) (((Mode) & S_IFMT) == S_IFNWK)
#endif

#if !defined(S_ISCTG) && defined(S_IFCTG)	/* contiguous file */
#define S_ISCTG(Mode) (((Mode) & S_IFMT) == S_IFCTG)
#endif
#if !defined(S_ISVTX)
#define S_ISVTX 0001000
#endif

#ifndef _POSIX_SOURCE
#include <sys/param.h>
#endif

/* Include <unistd.h> before any preprocessor test of _POSIX_VERSION.  */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


/* Declare standard functions.  */

#ifdef STDC_HEADERS
#include <stdlib.h>
#else
voidstar malloc ();
voidstar realloc ();
char *getenv ();
#endif

#include <stdio.h>

#ifndef _POSIX_VERSION
#ifdef __MSDOS__
#include <io.h>
#else
off_t lseek ();
#endif
#endif

#include <mg/pathmax.h>

/* Until getoptold function is declared in getopt.h, we need this here for
   struct option.  */
#include <getopt.h>

#ifdef WITH_DMALLOC
#undef HAVE_VALLOC
#define DMALLOC_FUNC_CHECK
#include <dmalloc.h>
#endif

/* Prototypes for external functions.  */

#ifndef __P
#if PROTOTYPES
#define __P(Args) Args
#else
#define __P(Args) ()
#endif
#endif

#if HAVE_LOCALE_H
#include <locale.h>
#endif
#if !HAVE_SETLOCALE
#define setlocale(Category, Locale)
#endif

#if ENABLE_NLS
#include <libintl.h>
#define _(Text) gettext (Text)
#else
#define textdomain(Domain)
#define _(Text) Text
#endif

/* Library modules.  */

#ifdef HAVE_VPRINTF
void error __P ((int, int, const char *,...));
#else
void error ();
#endif

#ifndef HAVE_STRSTR
char *strstr __P ((const char *, const char *));
#endif

/* [RB/TS:Oct/95] commented out. Used by lib/gmalloc.c - but it defines it anyway ???
 *#ifndef HAVE_VALLOC
 *#define valloc(Size) malloc (Size)
 *#endif
 */

voidstar xmalloc __P ((size_t));
voidstar xrealloc __P ((voidstar, size_t));
char *xstrdup __P ((const char *));

/* [TS:Aug/95] 
 * These were required by files which referred 
 * to these as function pointers. 
 * e.g. huffman.c - fread,  mgfelics.c - fgetc
 */
#ifndef HAVE_FREAD_DECL
extern size_t   fread __P ((void *, size_t, size_t, FILE *));
#endif
#ifndef HAVE_FGETC_DECL
extern int      fgetc __P ((FILE *));
#endif


#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility pop
#endif
#endif /* SYSFUNCS_H */
