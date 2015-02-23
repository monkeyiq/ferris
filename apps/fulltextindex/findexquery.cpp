/******************************************************************************
*******************************************************************************
*******************************************************************************

    findexquery command line client
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

    $Id: findexquery.cpp,v 1.9 2010/09/24 21:31:17 ben Exp $

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
#include <Ferris/FullTextIndexer.hh>
#include <Ferris/FullTextQuery.hh>
#include <Ferris/EAQuery.hh>

#include "fulltextindexers_custom_plugin/libftxcustomferris.hh"

#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;
using namespace FullTextIndex;

const string PROGRAM_NAME = "findexquery";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void
NoSupportExit( const std::string& msg )
{
    cerr << msg << endl;
    exit( 1 );
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int main( int argc, char** argv )
{
    int exit_status = 0;
    unsigned long FerrisInternalAsyncMessageSlave = 0;

    try
    {
        const char* CreateTypeName_CSTR    = 0;
        const char* IndexPath_CSTR         = 0;
        unsigned long UseRanked            = 0;
        unsigned long UseXapian            = 0;
        unsigned long UseTSearch2          = 0;
        unsigned long UseExternal          = 0;
        unsigned long UseBeagle            = 0;
        unsigned long ShowStats            = 0;
        unsigned long DumpAsXML            = 0;
        unsigned long DumpLexicon          = 0;
        unsigned long DumpInvertedFile     = 0;
        unsigned long DumpDocumentTable    = 0;
        unsigned long DumpIndex            = 0;
        unsigned long DumpIndexConfig      = 0;
        unsigned long DontSortByRank       = 0;
        unsigned long Verbose              = 0;
        unsigned long Quiet                = 0;
        unsigned long Dummy                = 0;
        unsigned long getNumberOfTerms     = 0;
        unsigned long getNumberOfDocuments = 0;
        unsigned long getSizeOfInvertedListChunks      = 0;
        unsigned long getDocumentNumberGapCode         = 0;
        unsigned long getFrequencyOfTermInDocumentCode = 0;
        unsigned long getLexiconClass                  = 0;
        unsigned long Limit = 0;
        unsigned long ShowOnlyCountOfMatches = 0;
        
        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "limit", 'l', POPT_ARG_INT, &Limit, 0,
                  "place a limit on the number of results", "" },

                { "quiet", 'q', POPT_ARG_NONE, &Quiet, 0,
                  "show minimal output where possible", "" },
                
                { "ranked", 'r', POPT_ARG_NONE, &UseRanked, 0,
                  "perform ranked query", "" },

                { "xapian", 'X', POPT_ARG_NONE, &UseXapian, 0,
                  "query is in the xapian query format for use on the xapian backend", "" },

                { "tsearch2", 'T', POPT_ARG_NONE, &UseTSearch2, 0,
                  "query is in the tsearch2 query format for use on the tsearch2 backend", "" },

                { "external", 'E', POPT_ARG_NONE, &UseExternal, 0,
                  "query is in the external query format for use on the external backend", "" },

                { "beagle", 'B', POPT_ARG_NONE, &UseBeagle, 0,
                  "query is in the beagle query format for use on the beagle backend", "" },
                
                { "boolean", 0, POPT_ARG_NONE, &Dummy, 0,
                  "perform boolean query (default)", "" },
                
                { "dont-sort-by-rank", '9', POPT_ARG_NONE, &DontSortByRank, 0,
                  "dont sort ranked query results by rank for display", "" },

                { "stats", '0', POPT_ARG_NONE, &ShowStats, 0,
                  "collect stats from query", "" },

                { "dump-lexicon", 0, POPT_ARG_NONE, &DumpLexicon, 0,
                  "dump out the lexicon", "" },

                { "dump-inverted-file", 0, POPT_ARG_NONE, &DumpInvertedFile, 0,
                  "dump out the index", "" },

                { "dump-document-table", 0, POPT_ARG_NONE, &DumpDocumentTable, 0,
                  "dump out the mapping of docid to document url", "" },

                { "dump-index", 0, POPT_ARG_NONE, &DumpIndex, 0,
                  "dump out the lexicon, inverted file and document map as one XML file", "" },

                { "dump-index-config", 0, POPT_ARG_NONE, &DumpIndexConfig, 0,
                  "dump information about how the index is compressed and its limits", "" },
                
                { "dump-as-xml", 'x', POPT_ARG_NONE, &DumpAsXML, 0,
                  "produce XML output during dump", "" },
                
                { "index-path", 'P', POPT_ARG_STRING, &IndexPath_CSTR, 0,
                  "which index to use", "" },

                { "get-number-of-terms", 0, POPT_ARG_NONE, &getNumberOfTerms, 0,
                  "display the number of distinct terms in the index and quit", "" },

                { "get-number-of-documents", 0, POPT_ARG_NONE, &getNumberOfDocuments, 0,
                  "display the number of documents in the index and quit", "" },

                { "get-size-of-inverted-list-chunks", 0, POPT_ARG_NONE, &getSizeOfInvertedListChunks, 0,
                  "display the max size of inverted list chunks in the index and quit", "" },

                { "get-document-number-gap-code", 0, POPT_ARG_NONE, &getDocumentNumberGapCode, 0,
                  "display the codec used to store document numbers in the index and quit", "" },

                { "get-frequency-of-term-in-document-code", 0, POPT_ARG_NONE, &getFrequencyOfTermInDocumentCode, 0,
                  "display the codec used to store f(d,t) numbers in the index and quit", "" },

                { "get-lexicon-class", 0, POPT_ARG_NONE, &getLexiconClass, 0,
                  "display the classname of the lexicon storage handler for the index and quit", "" },
                
//                 { "create-type", 0, POPT_ARG_STRING, &CreateTypeName_CSTR, 0,
//                   "what form of context to store the chunks in (dir/db4/gdbm/xml/etc)", 0 },

                { "ferris-internal-async-message-slave", 0, POPT_ARG_NONE, &FerrisInternalAsyncMessageSlave, 0,
                  "used by libferris itself to perform async queries through a slave process", "" },

                { "show-only-count", 'N', POPT_ARG_NONE, &ShowOnlyCountOfMatches, 0,
                  "show only the count of files matching the query.", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* query-string-here");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}
            
        if (argc < 2 )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        fh_idx idx;

        if( IndexPath_CSTR )
        {
            idx = FullTextIndex::Factory::getFullTextIndex( IndexPath_CSTR );
        }
        else
        {
            idx = FullTextIndex::Factory::getDefaultFullTextIndex();
        }

        fh_nidx nidx = dynamic_cast<FullTextIndexManagerNative*>(GetImpl( idx ));
        
        /**
         * These single shot methods are mainly handy for testing.
         */
        if( getNumberOfTerms )
        {
            if( !nidx )
                NoSupportExit("Getting the number of terms not supported for this index type");

            fh_ostream      oss = Ferris::Factory::fcout();
            fh_invertedfile inv = nidx->getInvertedFile();
            oss << inv->getNumberOfTerms()
                << endl;
            exit(0);
        }
        if( getNumberOfDocuments )
        {
            if( !nidx )
                NoSupportExit("Getting the number of docs not supported for this index type");
            fh_ostream      oss = Ferris::Factory::fcout();
            oss << nidx->getDocumentMap()->size()
                << endl;
            exit(0);
        }
        if( getSizeOfInvertedListChunks )
        {
            if( !nidx )
                NoSupportExit("Getting the size of list chunks not supported for this index type");
            fh_ostream      oss = Ferris::Factory::fcout();
            oss << nidx->getInvertedSkiplistMaxSize()
                << endl;
            exit(0);
        }
        if( getDocumentNumberGapCode )
        {
            if( !nidx )
                NoSupportExit("Getting the doc gap code not supported for this index type");
            fh_ostream      oss = Ferris::Factory::fcout();
            oss << nidx->getDocumentNumberGapCode()
                << endl;
            exit(0);
        }
        if( getFrequencyOfTermInDocumentCode )
        {
            if( !nidx )
                NoSupportExit("Getting f_t in doc not supported for this index type");
            fh_ostream      oss = Ferris::Factory::fcout();
            oss << nidx->getFrequencyOfTermInDocumentCode()
                << endl;
            exit(0);
        }
        if( getLexiconClass )
        {
            if( !nidx )
                NoSupportExit("Getting the lexicon class name"
                             "not supported for this index type");
            
            fh_ostream      oss = Ferris::Factory::fcout();
            oss << nidx->getLexiconClassName()
                << endl;
            exit(0);
        }

        
        if( DumpLexicon )
        {
            if( !nidx )
                NoSupportExit("dumping Lexicon not supported for this index type");

            fh_lexicon lex = nidx->getLexicon();
            lex->dumpTo( Ferris::Factory::fcout(), DumpAsXML );
            return 0;
        }

        if( DumpInvertedFile )
        {
            if( !nidx )
                NoSupportExit("dumping inverted file not supported for this index type");

            fh_invertedfile inv = nidx->getInvertedFile();
            inv->dumpTo( Ferris::Factory::fcout(), DumpAsXML );
            return 0;
        }

        if( DumpDocumentTable )
        {
            if( !nidx )
                NoSupportExit("dumping document table not supported for this index type");

            nidx->getDocumentMap()->dumpTo( Ferris::Factory::fcout(), DumpAsXML );
            return 0;
        }
        
        if( DumpIndex )
        {
            if( !nidx )
                NoSupportExit("dumping index not supported for this index type");
            
            fh_lexicon      lex = nidx->getLexicon();
            fh_invertedfile inv = nidx->getInvertedFile();
            fh_ostream oss = Ferris::Factory::fcout();
            bool DumpAsXML = true;
            
            oss << "<index type=\"fulltext\" >" << endl;
            lex->dumpTo( oss, DumpAsXML );
            inv->dumpTo( oss, DumpAsXML );
            nidx->getDocumentMap()->dumpTo( oss, DumpAsXML );
            oss << "</index>" << endl;
            return 0;
        }

        if( DumpIndexConfig )
        {
            if( !nidx )
                NoSupportExit("dumping index config not supported for this index type");

            fh_lexicon      lex = nidx->getLexicon();
            fh_invertedfile inv = nidx->getInvertedFile();
            fh_docmap       dm  = nidx->getDocumentMap();
            fh_ostream      oss = Ferris::Factory::fcout();

            oss << "Document number gap code          : " << nidx->getDocumentNumberGapCode() << endl;
            oss << "Frequency of term in document code: " << nidx->getFrequencyOfTermInDocumentCode() << endl;
            oss << "Lexicon class                     : " << nidx->getLexiconClassName() << endl;
            oss << "Dropping stop words               : " << nidx->getDropStopWords() << endl;
            oss << "Case sensitive                    : " << nidx->isCaseSensitive() << endl;
            oss << "Support for ranked query          : " << nidx->supportsRankedQuery() << endl;
            oss << "Size of chunks in inverted lists  : " << nidx->getInvertedSkiplistMaxSize() << endl;
            oss << endl;
            std::set< docid_t > revd = dm->getRevokedDocumentIDs();
            oss << "Revoked document IDs count        : " << revd.size() << endl;
            oss << Util::createSeperatedList( revd.begin(), revd.end() ) << endl;
            oss << endl;
            oss << "Number of..." << endl;
            oss << "   Distinct terms : " << inv->getNumberOfTerms()       << endl;
            oss << "   Documents      : " << nidx->getDocumentMap()->size() << endl;
            oss << endl;
            oss << "Version of..." << endl;
            oss << "   Lexicon      :" << nidx->getLexiconFileVersion() << endl
                << "   Inverted File:" << nidx->getInvertedFileVersion() << endl
                << "   Document Map :" << nidx->getDocumentMapFileVersion() << endl
                << endl;
            
            
            
        }

        string qs = "";

        bool virgin = true;
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            if( virgin ) virgin = false;
            else         qs += " ";

            qs += tmpCSTR;
        }

        
        try
        {
            QueryMode mode = QUERYMODE_USERPREF;

            if( UseRanked )
                mode = QUERYMODE_RANKED;
            else if( UseXapian )
                mode = QUERYMODE_XAPIAN;
            else if( UseTSearch2 )
                mode = QUERYMODE_TSEARCH2;
            else if( UseExternal )
                mode = QUERYMODE_EXTERNAL;
            else if( UseBeagle )
                mode = QUERYMODE_BEAGLE;
            else
                mode = QUERYMODE_BOOLEAN;
                
            fh_context result = ExecuteQuery( qs, mode, idx, Limit );

            if( Quiet )
            {
                if( ShowOnlyCountOfMatches )
                {
                    cout << result->getSubContextCount() << endl;
                    exit(0);
                }
            }
            
            cout << "Found " << result->SubContextCount()
                 << " matches at the following locations:" << endl;

            if( mode == QUERYMODE_RANKED && !DontSortByRank )
            {
                fh_context c = Ferris::Factory::MakeSortedContext( result, ":!#FLOAT:rank" );
                result       = c;
            }

            if( FerrisInternalAsyncMessageSlave )
            {
                fh_ostream oss = ::Ferris::Factory::fcout();
                stringmap_t m;
                m["count"] = tostr(result->getSubContextCount());
                XML::writeMessage( oss, m );
            }

            for( Context::iterator ci = result->begin(); ci != result->end(); ++ci )
            {
                if( FerrisInternalAsyncMessageSlave )
                {
                    fh_ostream oss = ::Ferris::Factory::fcout();
                    stringmap_t m;
                    string earl = (*ci)->getURL();
                    m["earl"] = earl;
                    if( mode == QUERYMODE_RANKED )
                        m["rank"] = getStrAttr( *ci, "rank", "" );
                    LG_IDX_D << "Sending earl:" << earl << endl;
                    XML::writeMessage( oss, m );
                    oss << flush;
                }
                else
                {
                    if( mode == QUERYMODE_RANKED )
                    {
                        string r = getStrAttr( *ci, "rank", "" );
                        if( !r.empty() )
                            cout << r << "\t";
                    }
                    cout << (*ci)->getURL() << endl;
                }
            }
            cout << flush;
        }
        catch( exception& e )
        {
            if( FerrisInternalAsyncMessageSlave )
            {
                fh_ostream oss = ::Ferris::Factory::fcout();
                stringmap_t m;
                m["outofband-error"] = e.what();
                XML::writeMessage( oss, m );
            }
            cerr << "error:" << e.what() << endl;
            exit_status = 1;
        }
    }
    catch( exception& e )
    {
        if( FerrisInternalAsyncMessageSlave )
        {
            fh_ostream oss = ::Ferris::Factory::fcout();
            stringmap_t m;
            m["outofband-error"] = e.what();
            XML::writeMessage( oss, m );
        }
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


