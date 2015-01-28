/**************************************************************************
 *
 * longlong.h -- Use of GCC's long long integer types
 * Copyright (C) 1999 Tim A.H. Bell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 **************************************************************************/

#ifndef H_LONGLONG
#define H_LONGLONG
#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility push(default)
#endif

#include "config.h"

/*
  Use GCC's "long long" integer types for certain variables, to avoid
  overflowing 32 bit integers.

  Other files affected:
  
    lib:
        bitio_random.c
        bitio_random.h
	
    src/text:
        ivf.pass1.c
        ivf.pass2.c
        build.h
        mg_passes.c
        text.h
    
*/

/* #define TESTING_OVERFLOW */

#ifdef TESTING_OVERFLOW

/* Test the overflow detection by using tiny (16-bit) types */
typedef unsigned short int mg_ullong;
typedef short int mg_llong;

#define ULL_FS "u"
#define LL_FS "d"

#elif defined __GNUC__ && ! defined DISABLE_LONG_LONG

/* We're using GCC, so it's okay to use "long long" (64-bit) types */
#define USE_LONG_LONG

typedef unsigned long long int mg_ullong;
typedef long long int mg_llong;

#define ULL_FS "llu"
#define LL_FS "lld"

#else

/* Not using GCC, so fall back on plain "long" (32-bit) types */
typedef unsigned long int mg_ullong;
typedef long int mg_llong;

#define ULL_FS "lu"
#define LL_FS "ld"

#endif /* __GNUC__ */

#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility pop
#endif
#endif
