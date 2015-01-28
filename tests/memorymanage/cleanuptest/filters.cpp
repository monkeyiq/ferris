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

    $Id: filters.cpp,v 1.1 2006/12/07 07:03:31 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "vmdebug.cpp"

int main( int argc, char** argv )
{
    CacheManager* cc = getCacheManager();
    ExistingContextsState* st_rootOnly;
    string filterstring = "(name==foo)";
    string complicatedFilterString = "(|(&(type==animal)(skin==fur))(name==wulf))";

    if( argc >= 2 ) filterstring            = argv[1];
    if( argc >= 3 ) complicatedFilterString = argv[2];

    /* Get the root context and save state, that is what should be available at the end */
    {
        printContexts("entered scope");
        fh_context rootc = Resolve( "/" );
        printContexts("have root");
        st_rootOnly = new ExistingContextsState();
    }
    
    {
        cerr << "filter:" << filterstring << endl;
        fh_context fc1 = Factory::MakeFilter( filterstring );
        printContexts("have filter");
    }
    
    printContexts("about to cleanup");
    cc->cleanUp( true );

    {
        fh_context fc2 = Factory::MakeFilter( complicatedFilterString );
        printContexts("have complex filter cleaning while holding ref");
        cc->cleanUp( true );
    }
    
    printContexts("about to cleanup");
    fullyReclaim( "reclaiming final" );

    
    printContexts("end");
    st_rootOnly->equalsCurrent();

    if( !errors )
        cout << "FULL Success" << endl;
    return errors;
}


