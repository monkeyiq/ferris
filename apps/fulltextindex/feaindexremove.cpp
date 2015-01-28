/******************************************************************************
*******************************************************************************
*******************************************************************************

    findexremove command line client
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

    $Id: feaindexremove.cpp,v 1.4 2010/09/24 21:31:16 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris/Ferris.hh>
#include <Ferris/EAIndexer.hh>
#include <Ferris/EAQuery.hh>

#include "eaindexers_custom_plugin/libeaidxcustomferris.hh"

#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;
using namespace Ferris::EAIndex;

const string PROGRAM_NAME = "feaindexremove";

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

int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        const char* IndexPath_CSTR         = 0;
        const char* Regex_CSTR             = 0;
        const char* DateSTR_CSTR           = 0;
        unsigned long Verbose              = 0;
            
        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "index-path", 'P', POPT_ARG_STRING, &IndexPath_CSTR, 0,
                  "which index to use", "" },

                { "regex", 'r', POPT_ARG_STRING, &Regex_CSTR, 0,
                  "regex for URLs to remove", "" },

                { "older-than", 'd', POPT_ARG_STRING, &DateSTR_CSTR, 0,
                  "remove any instances older than the given time", "now" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* ( -r regex | -d datestring ) ...");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}
            
        if (argc < 0 || !Regex_CSTR )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        EAIndex::fh_idx idx;

        if( IndexPath_CSTR )
        {
            idx = EAIndex::Factory::getEAIndex( IndexPath_CSTR );
        }
        else
        {
            idx = EAIndex::Factory::getDefaultEAIndex();
        }

        time_t tt = 0;
        
        if( DateSTR_CSTR )
        {
            std::string datestr = DateSTR_CSTR;
            struct tm tm = Time::ParseTimeString( datestr );
            tt = mktime( &tm );
        }

        string theRegex = Regex_CSTR;
        idx->removeDocumentsMatchingRegexFromIndex( theRegex, tt );
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


