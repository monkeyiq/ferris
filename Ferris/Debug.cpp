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

    $Id: Debug.cpp,v 1.3 2010/09/24 21:30:29 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Debug.hh>
#include <execinfo.h>
#include <cxxabi.h>
#include <string.h>
#include <stdlib.h>

#include <string>

namespace Ferris
{
    void BackTrace( int fd )
    {
        const int arraysz = 500;
        void* array[arraysz];
        size_t size;

        size = backtrace( array, arraysz );
        write( fd, "_________________________\n",
               strlen("_________________________\n") );
        write( fd, "raw symbol backtrace...\n",
               strlen("raw symbol backtrace...\n") );
        backtrace_symbols_fd( array, size, fd );

        write( fd, "\n\ndemangled symbol backtrace...\n",
               strlen("\n\ndemangled symbol backtrace...\n") );
        if( char** symbarray = backtrace_symbols( array, size ) )
        {
            size_t outsz = 4096;
            char* out = (char*)malloc( outsz+1 );
            for( int i=0; i < size; ++i )
            {
//                cerr << "sym:" << symbarray[i] << endl;
                std::string s = symbarray[i];
                std::string mangled = s.substr( 0, s.find(' ') );
                mangled = mangled.substr( mangled.find('(')+1 );
                mangled = mangled.substr( 0, mangled.rfind(')') );
                mangled = mangled.substr( 0, mangled.rfind('+') );
//                cerr << "mangled:" << mangled << endl;
                
                int status = 0;
                char* unmangled = __cxxabiv1::__cxa_demangle( mangled.c_str(), out, &outsz, &status );
//                char* unmangled = __cxxabiv1::__cxa_demangle( mangled.c_str(), 0, 0, &status );
                if( !status )
                {
                    out = unmangled;
//                    cerr << unmangled << endl;
                    write( fd, out, strlen(out) );
                    write( fd, "\n", 1 );
                }
                else
                {
//                    cerr << "status:" << status << endl;
                    write( fd, mangled.c_str(), mangled.length() );
                    write( fd, "\n", 1 );
                }
            }
            free( out );
            free( symbarray );
        }

        write( fd, "_________________________\n",
               strlen("_________________________\n") );
        
    }
    
    void BackTrace_OLD( int fd )
    {
        const int arraysz = 500;
        void* array[arraysz];
        size_t size;

        size = backtrace( array, arraysz );
        backtrace_symbols_fd( array, size, fd );
    }
};
