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

    $Id: tdbmaker.cpp,v 1.3 2010/09/24 21:31:49 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris.hh>
#include <tdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

using namespace Ferris;
using namespace std;


    string tostr( TDB_DATA d )
        {
            fh_stringstream ss;
            ss.write( (const char*)d.dptr, d.dsize );
            return tostr(ss);
        }

    void free( TDB_DATA d )
        {
            if( d.dptr )
                ::free( (void*)d.dptr );
        }

TDB_DATA tod( const std::string& s )
        {
            TDB_DATA ret;
            ret.dsize = s.length();
            ret.dptr = (unsigned char*)malloc( ret.dsize+1 );
            memcpy( ret.dptr, s.c_str(), ret.dsize );
            return ret;
        }

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int main( int argc, char** argv )
{
    string outfilename = "/tmp/junk.tdb";
    if( argc > 1 )
        outfilename = argv[1];

    TDB_CONTEXT* db = tdb_open ( (char *)outfilename.c_str(), 0,
                                 TDB_NOLOCK | TDB_CLEAR_IF_FIRST,
                                 O_RDWR | O_CREAT | O_LARGEFILE, S_IRUSR|S_IWUSR );

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
            TDB_DATA dk = tod(k);
            TDB_DATA dv = tod(v);
            tdb_store( db, dk, dv, TDB_REPLACE );
            free( dk );
            free( dv );
        }
    }
    
    
    tdb_close( db );
}
