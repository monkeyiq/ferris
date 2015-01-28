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

    $Id: resolveagain.cpp,v 1.1 2006/12/07 07:03:31 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

//
// Simple resolve, clean, resolve, clean and check ref_counts along the way
//
#include <config.h>
#include "vmdebug.cpp"

int main( int argc, char** argv )
{
    ExistingContextsState* st_rootOnly;
    string path = "/";
    int numberOfCleans = 2;

    if( argc >= 2 )
        path = argv[1];
    if( argc >= 3 )
        numberOfCleans = toint( argv[2] );

    /* Get the root context and save state, that is what should be available at the end */
    {
        printContexts("entered scope");
        fh_context rootc = Resolve( "/" );
        printContexts("have root");
        st_rootOnly = new ExistingContextsState();
    }
    
    {
        printContexts("entered scope");
        fh_context c = Resolve( path );
        printContexts("exiting scope");
    }
    
    printContexts("about to clean");
    CacheManager* cc = getCacheManager();
    for( int n=numberOfCleans; n>0; --n )
    {
        cc->cleanUp( true );
    }

    cerr << "asserting that we only have base system contexts now" << endl;
    st_rootOnly->equalsCurrent();

    {
        printContexts("entered scope");
        fh_context c = Resolve( path );
        printContexts("exiting scope");
    }
    printContexts("about to clean again");
    for( int n=numberOfCleans; n>0; --n )
    {
        cc->cleanUp( true );
    }

    
    printContexts("end");
    st_rootOnly->equalsCurrent();

    if( !errors )
        cout << "FULL Success" << endl;
    return errors;
}
