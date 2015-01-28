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

    $Id: libmbox.cpp,v 1.3 2010/09/24 21:31:44 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Common.hh>
#include <FerrisContextPlugin.hh>

#include <tokens.h>
#include <MBoxParser.h>

#include <ferrisPcctsContext.hh>



extern "C"
{
//    typedef ferrisPcctsContext<DLGLexer, MBoxParser, ANTLRToken> fctx;
    struct myMBoxParser : public ParserSyntaxHandler< MBoxParser >
    {
        typedef ParserSyntaxHandler< MBoxParser > _Base;
        myMBoxParser(ANTLRTokenBuffer *input)
            :
            _Base( input )
            {}
    };
    typedef ferrisPcctsContext<DLGLexer, myMBoxParser, ANTLRToken> fctx;

    FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
        throw( RootContextCreationFailed )
    {
        return fctx::Create( 0, rf->getInfo( "Root" ));
    }
}



