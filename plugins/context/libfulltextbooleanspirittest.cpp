/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2011 Ben Martin

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

    $Id: ferrisPcctsContext.hh,v 1.4 2010/09/24 21:31:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "libfulltextbooleanspirit.cpp"
#include <Resolver_private.hh>

using namespace Ferris;

int main( int argc, char** argv )
{

    RootContextFactory* rf = new RootContextFactory( "fulltextboolean", "/", "" );
    rf->AddInfo( "StaticString", "foo AND bar" );
    fh_context c = Brew( rf );
    
    return 0;
}
