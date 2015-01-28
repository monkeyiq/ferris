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

    $Id: readea.cpp,v 1.1 2006/12/07 07:03:31 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris.hh>
#include <iostream>

using namespace std;
using namespace Ferris;

int main( int argc, char** argv )
{
    try
    {
        if( argc < 3 )
        {
            cerr << "usage " << argv[0] << ": existing-url ea-name" << endl;
            exit(1);
        }
        
        fh_context c = Resolve( argv[1] );

        try
        {
            c->read();
        }
        catch( exception& e )
        {
            cerr << "Attempt to read c:" << c->getURL() << " failed." << endl;
        }
        
        string k = argv[2];
        cerr << "read url:" << c->getURL() << " ea-name:" << k
             << " is:" << getStrAttr( c, k, "<not-found>" )
             << endl;
    }
    catch( exception& e )
    {
        cerr << "ERROR e:" << e.what() << endl;
        exit(1);
    }
    return 0;
}
