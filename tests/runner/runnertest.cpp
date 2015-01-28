/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris runner test
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

    $Id: runnertest.cpp,v 1.1 2006/12/07 07:02:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris.hh>
#include <Runner.hh>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "runnertest";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}


int main( int argc, const char** argv )
{
    try 
    {
        cerr << "Enter command to run" << endl;
        string s;
        getline( cin, s );
        cerr << "Running command s:" << s << endl;
        
        fh_runner r = new Runner();
        r->setSpawnFlags( GSpawnFlags(r->getSpawnFlags()
                                      | G_SPAWN_STDOUT_TO_DEV_NULL
                                      | G_SPAWN_STDERR_TO_DEV_NULL
                                      | G_SPAWN_SEARCH_PATH) );
        
//        r->setCommandLine( s );
        r->getArgv().push_back( "bash" );
        r->getArgv().push_back( "-c" );
        r->getArgv().push_back( s );
        
        r->Run();
        int rc = r->getExitStatus();
        cerr << "exit status rc:" << rc << endl;
        exit( rc );
    }
    catch( exception& e )
    {
        cerr << "Error:" << e.what() << endl;
        return 1;
    }
    
    return 0;
}
