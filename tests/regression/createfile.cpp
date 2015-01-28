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

    $Id: createfile.cpp,v 1.1 2006/12/07 07:03:31 ben Exp $

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
            cerr << "usage " << argv[0] << ": existing-url newrdn" << endl;
            exit(1);
        }
        
        fh_context c = Resolve( argv[1] );
        c->read();
        string rdn   = argv[2];
        
        fh_context newc = Shell::CreateFile( c, rdn );
        
        cerr << "created:" << newc->getURL() << endl;
    }
    catch( exception& e )
    {
        cerr << "ERROR e:" << e.what() << endl;
        exit(1);
    }

//     CacheManager* cache = getCacheManager();
    
//     {
//         fh_stringstream ss;
//         cache->dumpFreeListTo( ss );
//         cerr << tostr(ss) << endl;
//     }
//     cache->cleanUp();
//     cerr << " --- cleaned up --- " << endl;
//     {
//         fh_stringstream ss;
//         cache->dumpFreeListTo( ss );
//         cerr << tostr(ss) << endl;
//     }
    
    
    return 0;
}
