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

    $Id: writecontext.cpp,v 1.1 2006/12/07 07:03:31 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include "config.h"

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
            cerr << "usage " << argv[0] << ": existing-url value" << endl;
            exit(1);
        }
        
        fh_context c = Resolve( argv[1] );
        string v = argv[2];
        
        {
            cout << "--- BEFORE --- " << endl;
            fh_istream ss = c->getIStream();
            copy( istreambuf_iterator<char>(ss),
                  istreambuf_iterator<char>(),
                  ostreambuf_iterator<char>(cout));
        }
        {
            fh_iostream ss = c->getIOStream( ios::trunc | ios::in | ios::out );
            ss << v << flush;
        }
        {
            cout << "--- AFTER  --- " << endl;
            fh_istream ss = c->getIStream();
            copy( istreambuf_iterator<char>(ss),
                  istreambuf_iterator<char>(),
                  ostreambuf_iterator<char>(cout));
        }

        {
            cout << "--- AFTER  --- " << endl;
            fh_istream ss = c->getIStream();
            string aft = StreamToString( ss );
            if( aft != v )
            {
                cerr << "ERROR verification of written value is not desired value."
                     << " desired:" << v
                     << " got:" << aft
                     << endl;
                exit( 1 );
            }
        }
        
        
        cerr << "wrote to file:" << c->getURL() << endl;
    }
    catch( exception& e )
    {
        cerr << "ERROR e:" << e.what() << endl;
        exit(1);
    }
    return 0;
}
