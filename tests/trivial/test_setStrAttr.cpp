/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: test_setStrAttr.cpp,v 1.1 2006/12/07 06:57:55 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include <Ferris/Ferris.hh>

using namespace std;
using namespace Ferris;

int main( int argc, char** argv )
{
    if( argc < 2 )
    {
        cerr << "usage test_setStrAttr path [value]" << endl
             << " WARNING the file content will be replaced with the given value" << endl;
        exit(1);
    }
    
    string path = argv[1];
    string v    = "freddy";

    if( argc > 2 )
        v = argv[2];
    
    fh_context c = Resolve( path );
    setStrAttr( c, "content", v );
    exit( 0 );
}
