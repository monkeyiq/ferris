/******************************************************************************
*******************************************************************************
*******************************************************************************

    feaindexquery command line client
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

    $Id: ferris-search.cpp,v 1.3 2010/09/24 21:31:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
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
using namespace Ferris::FullTextIndex;
//using namespace Ferris::EAIndex;

const string PROGRAM_NAME = "ferris-search";

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

typedef list< fh_context > results_t;
results_t results;

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


void
tryRunFulltextQuery( fh_idx fidx, const char* q, QueryMode mode )
{
    if( q )
    {
        fh_context result = ExecuteQuery( q, mode, fidx );
        results.push_back( result );
    }
}


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
        unsigned long ShowOnlyCountOfMatches = 0;
        unsigned long BenchQueryResolution = 0;
        const char* ftxBool                = 0;
        const char* ftxRanked              = 0;
        const char* ftxXapian              = 0;
        const char* ftxTSearch2            = 0;
        const char* eaquery                = 0;
        unsigned long CombineWithOR  = 0;
        
        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "or", 0, POPT_ARG_NONE, &CombineWithOR, 0,
                  "combine various searches with logical OR instead of AND", "" },
                
//                 { "index-path", 'P', POPT_ARG_STRING, &IndexPath_CSTR, 0,
//                   "which index to use", "" },

                { "show-only-count", 'N', POPT_ARG_NONE, &ShowOnlyCountOfMatches, 0,
                  "show only the count of files matching the query.", "" },

                { "ftx-boolean", 'B', POPT_ARG_STRING, &ftxBool, 0,
                  "combine boolean full text query", "" },

                { "ftx-ranked", 'R', POPT_ARG_STRING, &ftxRanked, 0,
                  "combine ranked full text query", "" },

                { "ftx-xapian", 'X', POPT_ARG_STRING, &ftxXapian, 0,
                  "combine Xapian full text query", "" },

                { "ftx-tsearch2", 'T', POPT_ARG_STRING, &ftxTSearch2, 0,
                  "combine tsearch2 full text query", "" },

                { "ea", 'E', POPT_ARG_STRING, &eaquery, 0,
                  "combine eaquery full text query", "" },
                
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
            
        if (argc < 2 )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        fh_idx fidx = FullTextIndex::Factory::getDefaultFullTextIndex();
        EAIndex::fh_idx eidx = EAIndex::Factory::getDefaultEAIndex();

        if( eaquery )
        {
            cerr << "eaquery:" << eaquery << endl;
            EAIndex::fh_eaquery q = EAIndex::Factory::makeEAQuery( eaquery, eidx );

            EAIndex::docNumSet_t docset;
            q->ExecuteQueryToDocIDs( docset );
            fh_context result = q->ResolveDocIDs( docset );
            results.push_back( result );
        }
        
        tryRunFulltextQuery( fidx, ftxBool,     QUERYMODE_BOOLEAN );
        tryRunFulltextQuery( fidx, ftxRanked,   QUERYMODE_RANKED );
        tryRunFulltextQuery( fidx, ftxXapian,   QUERYMODE_XAPIAN );
        tryRunFulltextQuery( fidx, ftxTSearch2, QUERYMODE_TSEARCH2 );
        

        results_t combinedResults;
        
        if( CombineWithOR )
        {
            for( results_t::const_iterator ri = results.begin(); ri!=results.end(); ++ri )
            {
                fh_context result = *ri;
                copy( result->begin(), result->end(), back_inserter( combinedResults ));
            }
        }
        else
        {
            if( !results.empty() )
            {
                //
                // Start with the smallest result and remove contexts from that
                // if they are not found in another result.
                //
                fh_context smallestIndividualResult = *results.begin();
                for( results_t::const_iterator ri = results.begin(); ri!=results.end(); ++ri )
                {
                    fh_context result = *ri;
                    if( result->getSubContextCount() <
                        smallestIndividualResult->getSubContextCount() )
                    {
                        smallestIndividualResult = result;
                    }
                }

                copy( smallestIndividualResult->begin(),
                      smallestIndividualResult->end(),
                      back_inserter( combinedResults ) );
                
                for( results_t::const_iterator ri = results.begin(); ri!=results.end(); ++ri )
                {
                    fh_context result = *ri;
                    if( result == smallestIndividualResult )
                        continue;

                    set< fh_context > tmpset;
                    copy( result->begin(), result->end(), inserter( tmpset, tmpset.end() ));
                    
                    for( results_t::iterator cri = combinedResults.begin();
                         cri != combinedResults.end(); )
                    {
                        if( tmpset.find( *cri ) == tmpset.end() )
                        {
                            results_t::iterator t = cri;
                            ++cri;
                            combinedResults.erase( t );
                        }
                        ++cri;
                    }
                    
                }
            }
        }

        cerr << "Result count:" << combinedResults.size() << endl;
        for( results_t::const_iterator ri = combinedResults.begin();
             ri!=combinedResults.end(); ++ri )
        {
            fh_context c = *ri;
            cout << c->getURL() << endl;
        }


//         for( results_t::const_iterator ri = results.begin(); ri!=results.end(); ++ri )
//         {
//             fh_context result = *ri;
//             for( Context::iterator ci = result->begin(); ci != result->end(); ++ci )
//             {
//                 cout << (*ci)->getURL() << endl;
//             }
//         }
        
        
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


