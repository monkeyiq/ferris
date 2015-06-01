/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris forwardsweepcre
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

    $Id: forwardsweepcre.cpp,v 1.1 2006/12/07 06:58:59 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include <Ferris.hh>
#include <unistd.h>

using namespace std;
using namespace Ferris;

/* File to create mid walk */
const char* const spannerfile = "./testdata/file3.0";

/* Number of items to iterate before creation */
int               spannerfile_count = 2;

struct Cleanup
{
    ~Cleanup()
        {
            unlink( spannerfile );
        }
};


int main( int argc, char** argv )
{
    string path = "./testdata";
    if( argc >= 2 ) path = argv[1];

    Cleanup _finalobj;
    fh_context c = Resolve( path );

    ContextIterator ci = c->begin();
    for( int i=0;
         ci != c->end();
         ++ci, ++i )
    {
        cerr << "ci:" << ci->getDirName() << endl;

        if( i == spannerfile_count )
        {
            {
                fh_stringstream ss;
                ss << " date > " << spannerfile;
                system( tostr(ss).c_str() );
            }
            cerr << "Just created the spannerfile:" << spannerfile << endl;
            {
                fh_stringstream ss;
                ss << " ls -l " << spannerfile;
                system( tostr(ss).c_str() );
            }
        }
    }
    
    return 0;
}
