/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris forwardsweep
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

    $Id: forwardsweep.cpp,v 1.1 2006/12/07 06:58:59 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/


#include "config.h"
#include <Ferris.hh>

using namespace std;
using namespace Ferris;

int main( int argc, char** argv )
{
    string path = "./testdata";
    if( argc >= 2 ) path = argv[1];
    
    fh_context c  = Resolve( path );
//    fh_context c1  = Resolve( path );
//    fh_context c = Factory::MakeSortedContext( c1, "(name)" );
    ContextIterator ci = c->begin();
    cerr << "============== Forward sweep of path:" << path
         << "============" << endl;
    for( ;
         ci != c->end();
         ++ci )
    {
        cerr << "ci:" << ci->getDirName() << endl;
    }
    cerr << "============== Completed forward sweep of path:" << path
         << "============" << endl;
    
    return 0;
}
