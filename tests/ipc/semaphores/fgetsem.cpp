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

    $Id: fgetsem.cpp,v 1.1 2006/12/07 07:02:16 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/Ferris.hh>
#include <unistd.h>

using namespace Ferris;
using namespace std;

int go( int argc, char** argv )
{
    fh_stringstream ss;
    ss << "ipc://semaphores/" << argv[1];
    cerr << "getting context for " << tostr(ss) << endl;

    fh_context c = Resolve( tostr(ss) );
    fh_istream sem = c->getIStream();

    sleep( 10 );
    return 0;
}


int
main( int argc, char** argv )
{
    if( argc < 2 )
    {
        cerr << "Usage: " << argv[0] << " semid " << endl;
        exit(1);
    }

    int rc = go( argc, argv );
    return rc;
}

