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

    $Id: libffilter.cpp,v 1.3 2010/09/24 21:31:43 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Common.hh>
#include <FerrisContextPlugin.hh>

#include <tokens.h>
#include <FFilterParser.h>

#include <ferrisPcctsContext.hh>

using namespace std;

namespace Ferris
{
    extern "C"
    {
//        typedef ferrisPcctsContext<DLGLexer, FFilterParser, ANTLRToken> fctx;

        struct myFFilterParser : public ParserSyntaxHandler< FFilterParser >
        {
            typedef ParserSyntaxHandler< FFilterParser > _Base;
            myFFilterParser(ANTLRTokenBuffer *input)
                :
                _Base( input )
                {}
        };
        typedef ferrisPcctsContext<DLGLexer, myFFilterParser, ANTLRToken> fctx;

        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
//                 cerr << "libffilter.brew() static str:"
//                      << rf->getInfo( "StaticString" )
//                      << endl;
                
                fctx::_Self* ret = fctx::Create( 0, rf->getInfo( "Root" ));

//#ifdef FERRIS_DEBUG_VM
                LG_VM_D << "ffilter::Brew() ret:" << toVoid(ret)
                        << " filter-string:" << rf->getInfo( "StaticString" )
                        << endl;
//#endif                
                
                if( rf->getInfo( "StaticString" ).length() )
                {
                    ret->setStaticString( rf->getInfo( "StaticString" ) );
                }
                return ret;
            }
            catch( exception& e )
            {
                fh_stringstream ss;
                ss << "libffilter.cpp::Brew() cought:" << e.what() << endl;
                cerr << "error:" << e.what() << endl;
                Throw_RootContextCreationFailed( tostr(ss), 0 );
            }
        }
    }
    
};


    
