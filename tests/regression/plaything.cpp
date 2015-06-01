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

    $Id: readea.cpp,v 1.1 2006/12/07 07:03:31 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include <Ferris.hh>
#include <EAIndexer.hh>
#include <EAQuery.hh>
#include <iostream>

using namespace std;
using namespace Ferris;
using namespace Ferris::EAIndex;


void OnExists ( NamingEvent_Exists* ev,
                const fh_context& subc,
                std::string olddn, std::string newdn )
{
    cerr << "OnExists... olddn:" << olddn << endl;
}

int createCount = 0;

void OnCreated ( NamingEvent_Created* ev,
                const fh_context& subc,
                std::string olddn, std::string newdn )
{
    cerr << "OnCreate... count:" << createCount++ << " olddn:" << olddn << endl;
}




int main( int argc, char** argv )
{
    try
    {
        string s = argv[1];
        EAIndex::fh_idx idx = EAIndex::Factory::getDefaultEAIndex();
        int limit = 1;
        fh_context c = EAIndex::ExecuteQueryAsync( s, limit, idx );
        c->getNamingEvent_Exists_Sig() .connect(sigc::ptr_fun( OnExists ));
        c->getNamingEvent_Created_Sig() .connect(sigc::ptr_fun( OnCreated ));

        cerr << "entering main loop" << endl;
        Main::mainLoop();

    }
    catch( exception& e )
    {
        cerr << "ERROR e:" << e.what() << endl;
        exit(1);
    }
    return 0;
}
