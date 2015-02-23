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

    $Id: feaindexquery.cpp,v 1.12 2011/05/08 21:30:52 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/EAIndexer.hh>
#include <Ferris/EAQuery.hh>
#include "Ferris/FerrisBoost.hh"
#include "Ferris/Medallion.hh"

#include "eaindexers_custom_plugin/libeaidxcustomferris.hh"
#include "eaindexers_custom_plugin/libeaidxcustomferrisdb4tree.hh"


#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;
using namespace Ferris::EAIndex;

const string PROGRAM_NAME = "feaindexquery";

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

namespace Ferris
{
    namespace FullTextIndex 
    {
        class Lexitest
        {
        public:
            static int TestForwardReverseIntegrity(
                fh_lexicon    fv,
                fh_revlexicon rv,
                fh_ostream    oss )
                {
                    cerr << "TestForwardReverseIntegrity()" << endl;
                    termid_t tid  = 0;
                    string   term = "";

                    for( term = fv->getFirstTerm();
                         !term.empty();
                         term = fv->getNextTerm( term ) )
                    {
                        tid = fv->lookup( term );

                        if( term != rv->lookup( tid ) )
                        {
                            oss << "failed to find term from walk of forward lexicon"
                                << " in reverse lexicon using lookup()"
                                << " term:" << term
                                << " tid:" << tid
                                << " got:" << rv->lookup( tid )
                                << endl;
                        }
                    }


                    for( tid = rv->getFirstTerm();
                         tid;
                         tid = rv->getNextTerm( tid ) )
                    {
                        term = rv->lookup( tid );

                        if( tid != fv->lookup( term ) )
                        {
                            oss << "failed to find term from walk of reverse lexicon"
                                << " in forward lexicon using lookup()"
                                << " term:" << term
                                << " tid:" << tid
                                << " got:" << rv->lookup( tid )
                                << endl;
                        }
                    }

                    return 0;
                }
        };
    };
};

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int main( int argc, char** argv )
{
    int exit_status = 0;
    unsigned long FerrisInternalAsyncMessageSlave = 0;
    
    try
    {
        const char* IndexPath_CSTR         = 0;
        unsigned long Verbose              = 0;
        unsigned long Quiet                = 0;
        unsigned long DumpAttributeNameMap = 0;
        unsigned long DumpIndex            = 0;
        unsigned long DumpIndexConfig      = 0;
        unsigned long DumpAsXML            = 0;
        unsigned long DumpForwardValueMap  = 0;
        unsigned long DumpReverseValueMap  = 0;
        unsigned long DumpDocumentTable    = 0;
        unsigned long TestValuemapInteg    = 0;
        unsigned long ResolveOnlyToURLStrings = 0;
        unsigned long BenchDocIDResolution = 0;
        unsigned long SkipDocIDResolution = 0;
        unsigned long ShowOnlyCountOfMatches = 0;

        unsigned long BenchQueryResolution = 0;
        unsigned long Limit = 0;
        
        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "limit", 'l', POPT_ARG_INT, &Limit, 0,
                  "place a limit on the number of results", "" },
                
                { "quiet", 'q', POPT_ARG_NONE, &Quiet, 0,
                  "show minimal output where possible", "" },

                { "resolve-only-to-url-strings", 'Z', POPT_ARG_NONE, &ResolveOnlyToURLStrings, 0,
                  "do not attempt to get libferris filesystem objects for results", "" },
                
                { "dump-attribute-name-map", 0, POPT_ARG_NONE, &DumpAttributeNameMap, 0,
                  "dump attribute name map lexicon", "" },

                { "dump-forward-value-map", 0, POPT_ARG_NONE, &DumpForwardValueMap, 0,
                  "dump the forward value map lexicon", "" },

                { "dump-reverse-value-map", 0, POPT_ARG_NONE, &DumpReverseValueMap, 0,
                  "dump the reverse value map lexicon", "" },
                
                { "dump-index", 0, POPT_ARG_NONE, &DumpIndex, 0,
                  "dump out all the parts of the ea index as one XML file", "" },

                { "dump-index-config", 0, POPT_ARG_NONE, &DumpIndexConfig, 0,
                  "dump information about how the index is compressed and its limits", "" },
                
                { "dump-as-xml", 'x', POPT_ARG_NONE, &DumpAsXML, 0,
                  "produce XML output during dump", "" },
                
                { "index-path", 'P', POPT_ARG_STRING, &IndexPath_CSTR, 0,
                  "which index to use", "" },

                { "test-valuemap-integrity", 0, POPT_ARG_NONE, &TestValuemapInteg, 0,
                  "test to make sure forward and reverse value maps are identical in content",
                  "" },
                
                { "dump-document-table", 0, POPT_ARG_NONE, &DumpDocumentTable, 0,
                  "dump out the mapping of docid to document url", "" },

                { "bench-docid-resolution", 0, POPT_ARG_NONE, &BenchDocIDResolution, 0,
                  "benchmark the time taken to resolve the docids in the result set.", "" },

                { "skip-docid-resolution", 0, POPT_ARG_NONE, &SkipDocIDResolution, 0,
                  "skip resolving the docids in the result set.", "" },

                { "show-only-count", 'N', POPT_ARG_NONE, &ShowOnlyCountOfMatches, 0,
                  "show only the count of files matching the query.", "" },
                
                
                { "bench-query-resolution", 0, POPT_ARG_NONE, &BenchQueryResolution, 0,
                  "benchmark the time taken to resolve the query to its docids.", "" },


                { "ferris-internal-async-message-slave", 0, POPT_ARG_NONE, &FerrisInternalAsyncMessageSlave, 0,
                  "used by libferris itself to perform async queries through a slave process", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* ffilter1 ffilter2 ...");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}
            
        if (argc < 2 )
        {
            poptPrintHelp(optCon, stderr, 0);
            cerr << endl << "primary usage: " << argv[0] << " ffilter" << endl;
            cerr << "for example: " << argv[0] << " (name==fred.txt)" << endl;
            exit(1);
        }

        stringlist_t queries;
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            string qs  = tmpCSTR;
            queries.push_back( qs );
        }

        // if we dont use emblems, dont even look at loading them
        {
            bool usingEmblems = false;
            static boost::regex rex("emblem:");
            
            for( stringlist_t::iterator qiter = queries.begin(); qiter != queries.end(); ++qiter )
            {
                string qs  = *qiter;
                boost::smatch matches;
                if( boost::regex_match( qs, matches, rex ))
                {
                    usingEmblems = true;
                    break;
                }
            }
//            cerr << "usingEmblems:" << usingEmblems << endl;
            if( !usingEmblems )
            {
                setSkipLoadingEmblems( !usingEmblems );
            }
        }
        
        
        

        if( Verbose )
            cerr << "connecting to index..." << endl;

        EAIndex::fh_idx idx;

        if( IndexPath_CSTR )
        {
            idx = EAIndex::Factory::getEAIndex( IndexPath_CSTR );
        }
        else
        {
            idx = EAIndex::Factory::getDefaultEAIndex();
        }
        fh_db4idx     db4idx     = dynamic_cast<EAIndexManagerDB4*    >(GetImpl( idx ));
        fh_db4treeidx db4treeidx = dynamic_cast<EAIndexManagerDB4Tree*>(GetImpl( idx ));

        if( Verbose )
            cerr << "connected to index..." << endl;
        
        if( DumpAttributeNameMap )
        {
            if( !db4idx )
                NoSupportExit("Operation not supported for this index format.");
            
            fh_lexicon lex = db4idx->getAttributeNameMap();
            lex->dumpTo( Ferris::Factory::fcout(), DumpAsXML );
            return 0;
        }
        if( DumpForwardValueMap )
        {
//             if( !db4idx )
//                 NoSupportExit("Operation not supported for this index format.");

//             fh_lexicon lex = db4idx->getSchemaValueMap();
//             lex->dumpTo( Ferris::Factory::fcout(), DumpAsXML );
//             return 0;
        }
        if( DumpReverseValueMap )
        {
//             if( !db4idx )
//                 NoSupportExit("Operation not supported for this index format.");

//             fh_revlexicon rlex = db4idx->getReverseValueMap();
//             rlex->dumpTo( Ferris::Factory::fcout(), DumpAsXML );
//             return 0;
        }
        if( DumpDocumentTable )
        {
            if( !db4idx )
                NoSupportExit("Operation not supported for this index format.");

            db4idx->getDocumentMap()->dumpTo( Ferris::Factory::fcout(), DumpAsXML );
            return 0;
        }

        if( DumpIndex )
        {
            if( db4idx )
            {
                fh_lexicon       lex_an = db4idx->getAttributeNameMap();
                fh_ostream oss = Ferris::Factory::fcout();
                bool DumpAsXML = true;
                
                oss << "<index type=\"ea\" >" << endl;
                lex_an->dumpTo( oss, DumpAsXML, "attribute" );
                db4idx->getInvertedFile( EAIndexManagerDB4::INV_INT )->dumpTo( oss, DumpAsXML );
                db4idx->getInvertedFile( EAIndexManagerDB4::INV_DOUBLE )->dumpTo( oss, DumpAsXML );
                db4idx->getInvertedFile( EAIndexManagerDB4::INV_CIS )->dumpTo( oss, DumpAsXML );
                db4idx->getInvertedFile( EAIndexManagerDB4::INV_STRING )->dumpTo( oss, DumpAsXML );
                db4idx->getDocumentMap()->dumpTo( oss, DumpAsXML );
                oss << "</index>" << endl;
                return 0;
            }
            if( db4treeidx )
            {
                fh_lexicon       lex_an = db4treeidx->getAttributeNameMap();
                fh_ostream oss = Ferris::Factory::fcout();
                bool DumpAsXML = true;
                
                oss << "<index type=\"ea\" >" << endl;
                lex_an->dumpTo( oss, DumpAsXML, "attribute" );
                db4treeidx->getInvertedFile( EAIndex::INV_INT )->dumpTo( oss, DumpAsXML );
                db4treeidx->getInvertedFile( EAIndex::INV_DOUBLE )->dumpTo( oss, DumpAsXML );
                db4treeidx->getInvertedFile( EAIndex::INV_CIS )->dumpTo( oss, DumpAsXML );
                db4treeidx->getInvertedFile( EAIndex::INV_STRING )->dumpTo( oss, DumpAsXML );
                db4treeidx->getDocumentMap()->dumpTo( oss, DumpAsXML );
                oss << "</index>" << endl;
                return 0;
            }

            NoSupportExit("Operation not supported for this index format.");
        }
        
        
        if( DumpIndexConfig )
        {
            if( !db4idx )
                NoSupportExit("Operation not supported for this index format.");

            fh_ostream      oss = Ferris::Factory::fcout();

            oss << "attribute name class   :" << db4idx->getAttributeNameMapClassName() << endl;
            oss << "ea names to ignore     :" << db4idx->getEANamesIgnore() << endl;
            stringlist_t sl = db4idx->getEANamesIgnoreRegexes();
            for( stringlist_t::iterator si = sl.begin(); si != sl.end(); ++si )
                oss << "ea names by regex to ignore:" << *si << endl;
            oss << "dgap code              :" << db4idx->getDocumentNumberGapCode() << endl;
            oss << "max attribute size to index:" << db4idx->getMaxValueSize() << endl;

            EAIndex::fh_invertedfile inv = db4idx->getInvertedFile(
                EAIndexManagerDB4::INV_STRING );
            oss << " --- settings in string inverted file --- " << endl;
            oss << "number of entries      :" << inv->getNumberOfItems() << endl;
                
            return 0;
        }

        if( TestValuemapInteg )
        {
//             if( !db4idx )
//                 NoSupportExit("Operation not supported for this index format.");

//             fh_lexicon    fv  = db4idx->getSchemaValueMap();
//             fh_revlexicon rv  = db4idx->getReverseValueMap();
//             fh_ostream    oss = Ferris::Factory::fcout();

//             return Lexitest::TestForwardReverseIntegrity( fv, rv, oss );
        }

        for( stringlist_t::iterator qiter = queries.begin(); qiter != queries.end(); ++qiter )
        {
            string qs  = *qiter;

//            fh_context result = ExecuteQuery( qs, idx );
            fh_eaquery q = EAIndex::Factory::makeEAQuery( qs, idx );

            if( Verbose )
                cerr << "running query..." << endl;
            
            EAIndex::docNumSet_t docset;
            {
                Time::fh_benchmark bm = BenchQueryResolution
                    ? new Time::Benchmark( "Resolving query to docids" ) : 0;
                
                q->ExecuteQueryToDocIDs( docset, Limit );
            }

            if( Quiet )
            {
                if( ShowOnlyCountOfMatches )
                {
                    cout << docset.size() << endl;
                    continue;
                }
            }

            if( FerrisInternalAsyncMessageSlave )
            {
                LG_EAIDX_D << "feaindexquery, as slave" << endl;
                fh_ostream oss = ::Ferris::Factory::fcout();
                stringmap_t m;
                m["count"] = tostr(docset.size());
                XML::writeMessage( oss, m );
                LG_EAIDX_D << "feaindexquery, sent count:" << docset.size() << endl;

                m.clear();
                m["docids"] = Util::createSeperatedList( docset.begin(), docset.end() );
                XML::writeMessage( oss, m );
                LG_EAIDX_D << "feaindexquery, sent docids..." << endl;
                
                fh_context result = q->ResolveDocIDs( docset );
                for( Context::iterator ci = result->begin(); ci != result->end(); ++ci )
                {
                    m.clear();
                    m["earl"] = (*ci)->getURL();
                    XML::writeMessage( oss, m );
                }
                oss << flush;
                LG_EAIDX_D << "feaindexquery, sent earls..." << endl;
            }
            else
            {
                
                fh_context result = 0;
                cout << "Found " << docset.size()
                     << " matches at the following locations:" << endl;

                if( ShowOnlyCountOfMatches )
                    continue;
            
                if( SkipDocIDResolution )
                    continue;

                if( ResolveOnlyToURLStrings )
                {
                    stringset_t result;
                    q->ResolveDocIDsToURLs( docset, result );
                    stringset_t::iterator ci = result.begin();
                    stringset_t::iterator  e = result.end();
                    for( ; ci != e; ++ci )
                    {
                        cout << *ci << endl;
                    }
                }
                else
                {
                    Time::fh_benchmark bm = 0;
                    if( BenchDocIDResolution )
                    {
                        bm = new Time::Benchmark( "Resolving docids" );
                    }
                    result = q->ResolveDocIDs( docset );
            
                    for( Context::iterator ci = result->begin(); ci != result->end(); ++ci )
                    {
                        cout << (*ci)->getURL() << endl;
                    }
                }
            }
        }
    }
    catch( exception& e )
    {
        string emsg = e.what();
        LG_EAIDX_D << "error:" << emsg << endl;
        if( FerrisInternalAsyncMessageSlave )
        {
            fh_ostream oss = ::Ferris::Factory::fcout();
            stringmap_t m;
            m["outofband-error"] = emsg;
            XML::writeMessage( oss, m );
        }
        cerr << "error:" << emsg << endl;
        exit(1);
    }
    return exit_status;
}


