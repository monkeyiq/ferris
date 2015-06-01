/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris daytime

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

    $Id: daytimeclient.cpp,v 1.3 2008/05/25 21:30:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include <string>
#include <list>
#include <set>
#include <vector>
#include <iterator>
#include <algorithm>
#include <iomanip>

#include <Ferris.hh>
#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;


const string PROGRAM_NAME = "daytime";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


int main( int argc, const char** argv )
{
    const char*   Host = "localhost";
    unsigned long Port = 13;
    
    struct poptOption optionsTable[] = {
        { "server-name", 's', POPT_ARG_STRING, &Host, 0,
          "Name of the server running daytime service",
          "localhost" },

        { "server-port", 'p', POPT_ARG_INT, &Port, 0,
          "Port on the server running daytime service",
          "13" },
        
        FERRIS_POPT_OPTIONS
        POPT_AUTOHELP
        POPT_TABLEEND
   };
    poptContext optCon;


    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* RootName1 ...");

    if (argc < 2) {
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }
    
    
    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {}


    try
    {
        fh_context c = Resolve( "socket:///tcp" );
        cerr << "resolved tcp url" << endl;
        
        fh_mdcontext md = new f_mdcontext();
        fh_mdcontext child = md->setChild( "clientsocket", "" );
        child->setChild( "remote-port",     tostr(Port));
        child->setChild( "remote-hostname", Host );
        
        fh_context socketc = c->createSubContext("", md );
        fh_istream iss     = socketc->getIStream();

        iss->unsetf( ios::skipws );
        copy( istream_iterator<char>(iss), istream_iterator<char>(),
              ostream_iterator<char>(cout) );
    }
    catch( exception& e )
    {
        cerr << "Cought exception:" << e.what() << endl << flush;
    }
    
    poptFreeContext(optCon);
    return 0;
}
