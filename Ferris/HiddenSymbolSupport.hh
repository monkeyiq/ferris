/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris streams
    Copyright (C) 2001-2003 Ben Martin

    libferris is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libferris is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libferris.  If not, see <http://www.gnu.org/licenses/>.

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: HiddenSymbolSupport.hh,v 1.2 2010/09/24 21:30:52 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_LIBFERRIS_HIDDENSYMBOLS_H_
#define _ALREADY_INCLUDED_LIBFERRIS_HIDDENSYMBOLS_H_

#ifndef FERRIS_SYMBOL_EXPORT_ALREADY_DONE
#define FERRIS_SYMBOL_EXPORT_ALREADY_DONE

#ifdef FERRISEXP_IMPORT
#undef FERRISEXP_IMPORT
#endif
#ifdef FERRISEXP_EXPORT
#undef FERRISEXP_EXPORT
#endif
#ifdef FERRISEXP_DLLLOCAL
#undef FERRISEXP_DLLLOCAL
#endif
#ifdef FERRISEXP_DLLPUBLIC
#undef FERRISEXP_DLLPUBLIC
#endif
#ifdef FERRISEXP_API
#undef FERRISEXP_API
#endif
#ifdef FERRISEXP_EXCEPTIONAPI
#undef FERRISEXP_EXCEPTIONAPI
#endif
#ifdef FERRISEXP_EXCEPTION
#undef FERRISEXP_EXCEPTION
#endif


// Shared library support
#ifdef WIN32
  #define FERRISEXP_IMPORT __declspec(dllimport)
  #define FERRISEXP_EXPORT __declspec(dllexport)
  #define FERRISEXP_DLLLOCAL
  #define FERRISEXP_DLLPUBLIC
#else
  #define FERRISEXP_IMPORT
  #ifdef GCC_HASCLASSVISIBILITY
    #define FERRISEXP_EXPORT    __attribute__ ((visibility("default")))
    #define FERRISEXP_DLLLOCAL  __attribute__ ((visibility("hidden")))
    #define FERRISEXP_DLLPUBLIC __attribute__ ((visibility("default")))
  #else
    #define FERRISEXP_EXPORT
    #define FERRISEXP_DLLLOCAL 
    #define FERRISEXP_DLLPUBLIC
  #endif
#endif

// Define FERRISEXP_API for DLL builds
#ifdef BUILDING_LIBFERRIS
#define FERRISEXP_API     FERRISEXP_EXPORT
#define FERRISEXP_GMODAPI FERRISEXP_EXPORT
#else
#define FERRISEXP_API 
#define FERRISEXP_GMODAPI
#endif


// Throwable classes must always be visible on GCC in all binaries
#ifdef WIN32
  #define FERRISEXP_EXCEPTIONAPI(api) api
#elif defined(GCC_HASCLASSVISIBILITY)
  #define FERRISEXP_EXCEPTIONAPI(api) FERRISEXP_EXPORT
#else
  #define FERRISEXP_EXCEPTIONAPI(api)
#endif

#define FERRISEXP_EXCEPTION FERRISEXP_EXPORT

#endif

#endif
