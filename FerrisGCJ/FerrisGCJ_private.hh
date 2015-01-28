/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

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

    $Id: FerrisGCJ_private.hh,v 1.2 2010/09/24 21:31:03 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_GCJ_H_
#define _ALREADY_INCLUDED_FERRIS_GCJ_H_

#include <gcj/cni.h>
#include <java/lang/System.h>
#include <java/io/PrintStream.h>
#include <java/lang/Throwable.h>
#include <java/lang/Character.h>
#include <java/lang/StringBuffer.h>

#include <java/io/IOException.h>
#include <java/io/BufferedReader.h>
#include <java/io/InputStreamReader.h>

#include <FerrisStreams/All.hh>
#include <string>

namespace Ferris
{
    using namespace ::java::lang;

    std::string tostr( jstring jstr );
    
    namespace Java
    {
        template <class T>
        ::java::lang::String* tojstr( T v )
        {
            return tojstr( tostr( v ) );
        }
        template <>
        ::java::lang::String* tojstr( const std::string& v );
        template <>
        ::java::lang::String* tojstr( std::string v );
        template <>
        ::java::lang::String* tojstr( const char* v );
    };
    namespace Factory
    {
        void ensureJVMCreated();
    };
};

#endif
