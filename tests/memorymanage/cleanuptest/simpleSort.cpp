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

    $Id: simpleSort.cpp,v 1.1 2006/12/07 07:03:31 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "vmdebug.cpp"

int main( int argc, char** argv )
{
    CacheManager* cc = getCacheManager();
    ExistingContextsState* st_rootOnly;

    string path = "/tmp";
    string sorter = "name";
    
    if( argc >= 2 ) path   = argv[1];
    if( argc >= 3 ) sorter = argv[2];

    /* Get the root context and save state, that is what should be available at the end */
    {
        printContexts("entered scope");
        fh_context rootc = Resolve( "/" );
        printContexts("have root");
        st_rootOnly = new ExistingContextsState();
    }

    {
        cerr << "sorter:" << sorter << endl;
        
        fh_sorter sobj = Factory::MakeSorter( sorter );
        fh_context rawc = Resolve( path );
        printContexts("have rawc");
        rawc->read();
        printContexts("have raw (read)");
        
        {
            fh_context sc = Factory::MakeSortedContext( rawc, sobj );
            printContexts("have sorted context");
        }

        printContexts("dropped sorter, about to cleanup 1");
        cc->cleanUp( true );
        printContexts("dropped sorter, about to cleanup 2");
        cc->cleanUp( true );

        printContexts("dropping raw context scope");
    }
    
    printContexts("open scope, about to cleanup");
    fullyReclaim( "reclaiming at end" );
    st_rootOnly->equalsCurrent();
    if( !errors )
        cout << "FULL Success" << endl;
    return errors;
}
