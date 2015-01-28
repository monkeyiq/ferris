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

    $Id: FerrisGCJ.cpp,v 1.2 2010/09/24 21:31:03 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisGCJ_private.hh>

namespace Ferris
{
    std::string tostr( jstring jstr )
    {
        jsize s = JvGetStringUTFLength ( jstr );
        std::string ret;
        ret.resize( s );
        JvGetStringUTFRegion( jstr, 0, s, (char*)ret.data() );
        return ret;
    }
    
    namespace Java
    {
        template <>
        ::java::lang::String* tojstr( const std::string& v )
        {
            return JvNewStringLatin1( v.c_str() );
        }
        template <>
        ::java::lang::String* tojstr( std::string v )
        {
            return JvNewStringLatin1( v.c_str() );
        }
        template <>
        ::java::lang::String* tojstr( const char* v )
        {
            return JvNewStringLatin1( v );
        }
    };
    namespace Factory
    {
        void ensureJVMCreated()
        {
            static bool v = true;

            if( v )
            {
                v = false;
                
                JvCreateJavaVM(NULL);
                JvAttachCurrentThread(NULL, NULL);
                JvInitClass(&System::class$);
            }
        }
    };
};
