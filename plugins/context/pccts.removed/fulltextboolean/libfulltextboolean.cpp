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

    $Id: libfulltextboolean.cpp,v 1.3 2010/09/24 21:31:43 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Common.hh>
#include <FerrisContextPlugin.hh>

#include <tokens.h>
#include <FulltextbooleanParser.h>

#include <ferrisPcctsContext.hh>

using namespace std;

namespace Ferris
{
    extern "C"
    {
//        typedef ferrisPcctsContext<PCCTSLEXERCLASSNAME, FulltextbooleanParser, ANTLRToken> fctx;

        struct myFulltextbooleanParser : public ParserSyntaxHandler< FulltextbooleanParser >
        {
            typedef ParserSyntaxHandler< FulltextbooleanParser > _Base;
            myFulltextbooleanParser(ANTLRTokenBuffer *input)
                :
                _Base( input )
                {}
        };
        typedef ferrisPcctsContext<PCCTSLEXERCLASSNAME, myFulltextbooleanParser, ANTLRToken> fctx;
        
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                fctx::_Self* ret = fctx::Create( 0, rf->getInfo( "Root" ));

                if( rf->getInfo( "StaticString" ).length() )
                {
                    ret->setStaticString( rf->getInfo( "StaticString" ) );
                }
                return ret;
            }
            catch( exception& e )
            {
                fh_stringstream ss;
                ss << "libfulltextboolean.cpp::Brew() cought:" << e.what() << endl;
                Throw_RootContextCreationFailed( tostr(ss), 0 );
            }
        }
    }
    
};


    
