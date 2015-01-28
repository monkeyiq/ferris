/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris hal
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
#include <Ferris/Ferris.hh>
#include <Ferris/Runner.hh>
#include <popt.h>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ferris-internal-hal-els";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

int RETURN_CODE = 0;
bool running = true;

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
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* ...");

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
        cout << PROGRAM_NAME << " version: $Id: ls.cpp,v 1.12 2008/04/27 21:30:11 ben Exp $\n"
             << "ferris   version: " << VERSION << nl
             << "Written by Ben Martin, aka monkeyiq" << nl
             << nl
             << "Copyright (C) 2008 Ben Martin" << nl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }

    stringlist_t volumes;
    while( const char* argCSTR = poptGetArg(optCon) )
    {
        if( argCSTR )
        {
            string arg = argCSTR;
            volumes.push_back( arg );
        }
    }

    try
    {
        for( stringlist_t::iterator vi = volumes.begin(); vi != volumes.end(); ++vi )
        {
            string vol = *vi;
            fh_context c = Resolve( vol );
            string mp = getStrAttr( c, "volume.mount_point", "" );

            if( mp.empty() )
            {
                LG_HAL_D << "no mountpoint!" << endl;
            }
            
            fh_runner r = new Runner();
            {
                stringstream ss;
                ss << "Ego-read-dir"  << " \"" << mp << "\"";
                r->setCommandLine( ss.str() );
                LG_HAL_D << "els cmd:" << ss.str() << endl;
            }
            
            r->Run();
            gint e = r->getExitStatus();
        }
    }
    catch( exception& e )
    {
        LG_HAL_W << "broker error:" << e.what() << endl;
        cerr << "cought:" << e.what() << endl;
        exit(1);
    }

    poptFreeContext(optCon);
    return 0;
}

