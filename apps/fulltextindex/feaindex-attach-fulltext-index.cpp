/******************************************************************************
*******************************************************************************
*******************************************************************************

    feaindex-attach-fulltext-index command line client
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

    $Id: feaindex-attach-fulltext-index.cpp,v 1.4 2010/09/24 21:31:15 ben Exp $

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
#include <Ferris/FullTextIndexer.hh>
#include <Ferris/FullTextQuery.hh>

#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;
//using namespace Ferris::FullTextIndex;
//using namespace Ferris::EAIndex;

const string PROGRAM_NAME = "feaindex-attach-fulltext-index";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

void
NoSupportExit( const std::string& msg )
{
    cerr << msg << endl;
    exit( 1 );
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
//        const char* IndexPath_CSTR         = 0;
        unsigned long Verbose              = 0;
        const char* EAIndexPath_CSTR       = 0;
        const char* FulltextIndexPath_CSTR = 0;
        
        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "ea-index-path", 'P', POPT_ARG_STRING, &EAIndexPath_CSTR, 0,
                  "Path to the EA index", "" },

                { "fulltext-index-path", 'F', POPT_ARG_STRING, &FulltextIndexPath_CSTR, 0,
                  "Path to the fulltext index to attach to the EA index", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* ");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}
            
        if (argc < 3 )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        if( !EAIndexPath_CSTR || !FulltextIndexPath_CSTR )
        {
            cerr << "You must supply atleast -P and -F" << endl;
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
        
        
        FullTextIndex::fh_idx fidx =
            FullTextIndex::Factory::getFullTextIndex( FulltextIndexPath_CSTR );
        EAIndex::fh_idx eidx =
            EAIndex::Factory::getEAIndex( EAIndexPath_CSTR );

        eidx->setFulltextIndex( fidx );
//        eidx->setFulltextIndex( FulltextIndexPath_CSTR );
        
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


