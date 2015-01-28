/******************************************************************************
*******************************************************************************
*******************************************************************************

    feaindex-remove-old-instances command line client
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

    $Id: feaindex-remove-old-instances.cpp,v 1.3 2010/09/24 21:31:16 ben Exp $

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

#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;
using namespace Ferris::EAIndex;

const string PROGRAM_NAME = "feaindex-remove-old-instances";

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
        const char* DateSTR_CSTR           = 0;
        unsigned long Verbose              = 0;
            
        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "index-path", 'P', POPT_ARG_STRING, &IndexPath_CSTR, 0,
                  "which index to use", "" },

                { "older-than", 'd', POPT_ARG_STRING, &DateSTR_CSTR, 0,
                  "when a document has two or more instances in the index, remove any older than the given time", "now" },
                
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
        {}
            
        if (argc < 0 )
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

        time_t tt = Time::getTime();
        
        if( DateSTR_CSTR )
        {
            std::string datestr = DateSTR_CSTR;
            struct tm tm = Time::ParseTimeString( datestr );
            tt = mktime( &tm );
        }
        idx->purgeDocumentInstancesOlderThan( tt );
        
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


