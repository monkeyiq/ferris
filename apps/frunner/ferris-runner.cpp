/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris runner
    Copyright (C) 2008 Ben Martin

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

    $Id: ls.cpp,v 1.12 2008/04/27 21:30:11 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <config.h>

#include <string>
#include <list>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <Ferris.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/Runner.hh>

#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;

#define DEBUG 0

const string PROGRAM_NAME = "ferris-runner";

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
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


int main( int argc, const char** argv )
{
    const char* ShowColumnsCSTR  = "x";
    unsigned long HideHeadings   = 1;
    unsigned long ShowVersion    = 0;
    
    struct poptOption optionsTable[] = {

//         { "show-columns", 0, POPT_ARG_STRING, &ShowColumnsCSTR, 0,
//           "Same as --show-ea",
//           "size,mimetype,name" },

        
//         { "hide-headings", 0, POPT_ARG_NONE, &HideHeadings, 0,
//           "Prohibit the display of column headings", 0 },

        
        /*
         * Other handy stuff
         */

        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },
        
        /*
         * Standard Ferris options
         */
        FERRIS_POPT_OPTIONS

        /**
         * Expansion of strange-url://foo*
         */
        FERRIS_SHELL_GLOB_POPT_OPTIONS
        
//         { "show-all-attributes", 0, POPT_ARG_NONE,
//           &ShowAllAttributes, 0,
//           "Show every available bit of metadata about each object in context",
//           0 },
        POPT_AUTOHELP
        POPT_TABLEEND
   };
    poptContext optCon;


    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* url1 url2 ...");

    if (argc < 1) {
//        poptPrintHelp(optCon, stderr, 0);
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }

    
    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
//         switch (c) {
//         }
    }

    if( ShowVersion )
    {
        cout << "ferrisls version: $Id: ls.cpp,v 1.12 2008/04/27 21:30:11 ben Exp $\n"
             << "ferris   version: " << VERSION << nl
             << "Written by Ben Martin, aka monkeyiq" << nl
             << nl
             << "Copyright (C) 2001 Ben Martin" << nl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }

    stringstream ss;
    while( const char* argCSTR = poptGetArg(optCon) )
    {
        string arg = argCSTR;
        ss << arg << " ";
    }
    cerr << "ss:" << ss.str() << endl;

//    ss << "gfcp -av file:///Video/Backedup/japan_comercial/DrJapan.avi /tmp";
//    ss << "gfcp -av    file:///Video/Backedup/japan_comercial/DrJapan.avi  file:///tmp";
    
//     ss << "gfcp -av ";
//     typedef stringlist_t urllist_t;
//     urllist_t urllist;
//     urllist.push_back("file:///Video/Backedup/japan_comercial/DrJapan.avi");
//     urllist.push_back("file:///tmp");
//     for( urllist_t::iterator iter = urllist.begin();
//          iter != urllist.end(); ++iter )
//     {
//         fh_context src = Resolve( *iter );
//         ss << " " << Shell::quote( src->getParent()->getURL() + "\\/" + src->getDirName() ) << " ";
//     }

    cerr << "cmd:" << ss.str() << endl;
    
    fh_runner r = new Runner();
    r->setCommandLine( ss.str() );
    r->setConnect_ChildStdOut_To_ParentStdOut( true );
    r->setConnect_ChildStdErr_To_ParentStdErr( true );
    r->setSpawnFlags(
        GSpawnFlags(
            G_SPAWN_SEARCH_PATH |
//            G_SPAWN_STDERR_TO_DEV_NULL |
//            G_SPAWN_STDOUT_TO_DEV_NULL |
            r->getSpawnFlags()));
    r->Run();
    
    gint e = r->getExitStatus();
    cerr << "e:" << e << endl;
    
    return 0;
}
