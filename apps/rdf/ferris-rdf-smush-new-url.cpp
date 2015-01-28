/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris-rdf-smush-new-url.cpp
    Copyright (C) 2005 Ben Martin

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

    $Id: ferris-rdf-smush-new-url.cpp,v 1.5 2010/09/24 21:31:19 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
 * 
*/


#include <Ferris.hh>
#include <FerrisSemantic.hh>
#include <popt.h>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ferris-rdf-smush-new-url";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

unsigned long Verbose              = 0;
unsigned long UnifyUUIDNodes       = 0;
//const char* oldURL_CSTR = 0;

int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "unify", 'u', POPT_ARG_NONE, &UnifyUUIDNodes, 0,
                  "Bind old and new URLs to the same metadata node", "" },
                
//                 { "trunc", 'T', POPT_ARG_NONE, &Truncate, 0,
//                   "truncate output file", "" },

//                 { "ea", 0, POPT_ARG_STRING, &EAName_CSTR, 0,
//                   "put redirection into this EA", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* oldURL newURL");

        /* Now do options processing */
        {
            int c=-1;
            while ((c = poptGetNextOpt(optCon)) >= 0)
            {}
        }
        

        if (argc < 2)
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        stringlist_t srcs;
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            string srcURL = tmpCSTR;
            srcs.push_back( srcURL );
        }

        if( srcs.size() != 2)
        {
            cerr << "please supply old-URL and new-URL for RDF smushing." << endl;
            exit(1);
        }

        stringlist_t::iterator si = srcs.begin();
        string oldearl = *si; ++si;
        string newearl = *si;
        fh_context newc = Resolve( newearl );

        cerr << "oldearl:" << oldearl << " newearl:" << newc->getURL() << endl;
        
        Semantic::myrdfSmush( newc, oldearl, UnifyUUIDNodes );
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


