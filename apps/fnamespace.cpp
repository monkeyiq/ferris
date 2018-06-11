/******************************************************************************
*******************************************************************************
*******************************************************************************

    fnamespace command line client
    Copyright (C) 2003 Ben Martin

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

    $Id: fnamespace.cpp,v 1.3 2010/09/24 21:31:18 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris/Ferris.hh>

#include <popt.h>
#include <unistd.h>
#include <SignalStreams.hh>
#include <iterator>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "fnamespace";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int main( int argc, char** argv )
{
    int exit_status = 0;
    const char* Prefix_CSTR      = 0;
    const char* URI_CSTR         = 0;
    unsigned long SetMode        = 0;
    unsigned long GetMode        = 0;
    unsigned long ListMode       = 0;
    unsigned long Verbose        = 0;
    
    try
    {

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "prefix", 'p', POPT_ARG_STRING, &Prefix_CSTR, 0,
                  "prefix to use", "" },

                { "uri", 'a', POPT_ARG_STRING, &URI_CSTR, 0,
                  "URI to set prefix to use", "" },
                
                { "set", 's', POPT_ARG_NONE, &SetMode, 0,
                  "set URI for given prefix", "" },

                { "get", 'g', POPT_ARG_NONE, &GetMode, 0,
                  "get URI for given prefix", "" },
                
                { "list-prefixes", 'l', POPT_ARG_NONE, &ListMode, 0,
                  "list active prefixes and their resolution for a context", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* src1 src2 ...");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {
//         switch (c) {
//         }
        }

        if (argc < 2 )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
        if( SetMode && ( !Prefix_CSTR || !URI_CSTR ))
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
        if( GetMode && !Prefix_CSTR )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
            
        
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            try
            {
                string srcURL = tmpCSTR;
                fh_context c = Resolve( srcURL );

                if( ListMode )
                {
                    stringlist_t sl = c->getNamespacePrefixes();

                    cerr << "listing prefixes in use for context," << endl
                         << " c:" << c->getURL() << endl;
                    
                    for( stringlist_t::iterator si = sl.begin(); si!=sl.end(); ++si )
                    {
                        cerr << *si << " = " << c->resolveNamespace( *si ) << endl;
                    }
                }
                else if( SetMode )
                {
                    c->setNamespace( Prefix_CSTR, URI_CSTR );
                }
                else if( GetMode )
                {
                    c->resolveNamespace( Prefix_CSTR );
                }
            }
            catch( exception& e )
            {
                cerr << "error:" << e.what() << endl;
                exit_status = 1;
            }
        }
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


