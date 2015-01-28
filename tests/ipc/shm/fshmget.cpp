/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris 
    Copyright (C) 2002 Ben Martin

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

    $Id: fshmget.cpp,v 1.1 2006/12/07 07:02:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/Ferris.hh>

using namespace std;
using namespace Ferris;

int perform( int argc, char** argv )
{
    int sz = toint( argv[2] );
    
    cerr << "resolve url:" << argv[1] << endl;
    fh_context c = Resolve( argv[1] );
    {
        fh_istream ss = c->getIStream();

        cerr << "reading " << sz << " bytes of data" << endl;
        for( int i=0; i<sz; ++i )
        {
            char ch = 0;
            if( ss >> noskipws >> ch )
            {
                cerr << " i:" << i
                     << " ch[" << i << "]:" << hex << (int)ch
                     << " ch[" << i << "]:" << ch
                     << endl;
            }
        }
    }
    cerr << "Stream closed." << endl;
    return 0;
}

int main( int argc, char** argv )
{
    if( argc < 3 )
    {
        cerr << "Usage: " << argv[0] << " shm-url size " << endl;
        exit(1);
    }

    return perform( argc, argv );
}
