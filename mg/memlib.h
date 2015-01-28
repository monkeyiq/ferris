/**************************************************************************
 *
 * memlib.h -- Malloc wrappers
 * Copyright (C) 1994  Neil Sharman
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
 * $Id: memlib.h,v 1.2 2008/06/07 21:30:10 ben Exp $
 *
 **************************************************************************/


#ifndef H_MEMLIB
#define H_MEMLIB
#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility push(default)
#endif

#ifdef STD_MEM

#define Xmalloc malloc
#define Xrealloc realloc
#define Xfree free
#define Xstrdup strdup

#else


#include "sysfuncs.h"

typedef void *(*Malloc_func) (size_t);
typedef void *(*Realloc_func) (void *, size_t);
typedef void (*Free_func) (void *);
typedef char *(*Strdup_func) (const char *);

extern Malloc_func Xmalloc;
extern Realloc_func Xrealloc;
extern Free_func Xfree;
extern Strdup_func Xstrdup;
#endif

#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility pop
#endif
#endif
