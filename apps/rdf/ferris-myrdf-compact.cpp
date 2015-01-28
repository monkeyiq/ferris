/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris-myrdf-compact
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

    $Id: ferris-myrdf-compact.cpp,v 1.4 2010/09/24 21:31:19 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
 * 
*/


#include <Ferris.hh>
#include <FerrisRDFCore.hh>
#include <popt.h>

#include <Configuration_private.hh>
#include <EAIndexer_private.hh>
#include <STLdb4/stldb4.hh>
using namespace ::STLdb4;

using namespace std;
using namespace Ferris;
using namespace Ferris::RDFCore;

const string PROGRAM_NAME = "ferris-myrdf-compact";

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

unsigned long Verbose             = 0;
const char* RedlandEAIndexURLCSTR = 0;

int main( int argc, char** argv )
{
    int exit_status = 0;

    try
    {

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "rdf-eaindex-url", 'P', POPT_ARG_STRING, &RedlandEAIndexURLCSTR, 0,
                  "use rdf ea index model at given location", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* queryfile");

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

        fh_model m = getDefaultFerrisModel();
        if( RedlandEAIndexURLCSTR )
        {
            static const char* CFG_IDX_DBNAME_K = "cfg-idx-dbname";
            static const char* CFG_IDX_DBNAME_DEF = "ferriseaindex";
            static const char* CFG_IDX_DBOPTIONS_K = "cfg-idx-dboptions";
            static const char* CFG_IDX_DBOPTIONS_DEF = "hash-type='bdb',contexts='yes',index-predicates='yes',";
            static const char* CFG_IDX_STORAGENAME_K = "cfg-idx-storage-name";
            static const char* CFG_IDX_STORAGENAME_DEF = "hashes";

            // string dbfilename = CleanupURL( (string)RedlandEAIndexURLCSTR
            //                                 + "/" + EAIndex::DB_EAINDEX );
            // fh_database db = new Database( dbfilename );
            
            // string storage_name = get_db4_string( db, CFG_IDX_STORAGENAME_K, "", true );
            // string db_name      = get_db4_string( db, CFG_IDX_DBNAME_K,      "", true );
            // string db_options   = get_db4_string( db, CFG_IDX_DBOPTIONS_K,   "", true );

            fh_context md = Resolve( (string)RedlandEAIndexURLCSTR + "/metadata" );
            m = Model::FromMetadataContext( md );
        }

        //
        // If these cache nodes are deleted then smushing
        // will be very slow again. A tradeoff between waste
        // of cheap disk on the chance that they are needed again.
        // 
//         fh_statement st = new Statement();
//         st->setPredicate( smushCacheUUIDPredNode() );
//         StatementIterator e;
//         StatementIterator iter = m->findStatements( st );
//         for( ; iter!=e; ++iter )
//         {
//             fh_statement s = *iter;
//             m->erase( s );
//         }
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


