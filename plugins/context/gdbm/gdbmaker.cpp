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

    $Id: gdbmaker.cpp,v 1.3 2010/09/24 21:31:38 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include "config.h"

#include <Ferris.hh>
#include <gdbm.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace Ferris;
using namespace std;

string tostr( datum d )
{
    fh_stringstream ss;
    ss.write( d.dptr, d.dsize );
    return tostr(ss);
}

void free( datum d )
{
    if( d.dptr )
        ::free( (void*)d.dptr );
}

datum tod( const std::string& s )
{
    datum ret;
    ret.dsize = s.length();
    ret.dptr = (char*)malloc( ret.dsize+1 );
    memcpy( ret.dptr, s.c_str(), ret.dsize );
    return ret;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int main( int argc, char** argv )
{
    string outfilename = "/tmp/junk.gdbm";
    if( argc > 1 )
        outfilename = argv[1];

    GDBM_FILE db = gdbm_open ( (char *)outfilename.c_str(), 0, GDBM_NEWDB, S_IRWXU, 0 );

    if( !db )
    {
        cerr << "Failed to open database" << endl;
        exit(1);
    }
    
    
    fh_context c = Resolve( "." );
    for( Context::iterator ci = c->begin(); ci != c->end(); ++ci )
    {
        string k = getStrAttr( *ci, "name", "" );
        string v = getStrAttr( *ci, "content", "", true );
        if( !v.empty() )
        {
            cerr << "Setting k:" << k << " to v.len:" << v.length()
                 << endl;
            datum dk = tod(k);
            datum dv = tod(v);
            gdbm_store( db, dk, dv, GDBM_REPLACE );
            free( dk );
            free( dv );
        }
    }
    
    
    gdbm_close( db );
}
