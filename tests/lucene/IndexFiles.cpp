/******************************************************************************
*******************************************************************************
*******************************************************************************

    test adding file(s) using the new indexing API
    Note that these are the rough clients, use ferris/apps/fulltextindex for
    the real clients.

    Copyright (C) 2004 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: IndexFiles.cpp,v 1.2 2008/04/27 21:30:12 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/Ferris.hh>
#include <Ferris/FullTextIndexerMetaInterface.hh>
#include <Ferris/FullTextIndexer.hh>

using namespace std;
using namespace Ferris;
using namespace Ferris::FullTextIndex;

const string PROGRAM_NAME = "IndexFiles";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int main( int argc, const char** argv )
{
    int exit_status = 0;
    const char*   IndexURL_CSTR        = "/tmp/foo-index";
    const char*   IndexClassName_CSTR  = "lucene";
    unsigned int  createIndex          = 0;
    unsigned int  caseSensitive        = 0;
    unsigned int  dropStopWords        = 0;
    
    struct poptOption optionsTable[] =
        {
            { "create", 'c', POPT_ARG_NONE, &createIndex, 0,
              "create new index in dir, wipe out old one if it exists.", "" },
            
            FERRIS_POPT_OPTIONS
            POPT_AUTOHELP
            POPT_TABLEEND
        };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]*");

    if (argc < 2)
    {
        poptPrintHelp(optCon, stderr, 0);
        exit(1);
    }
    
    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
    }

    string IndexClassName = IndexClassName_CSTR;
    string IndexURL       = IndexURL_CSTR;
    StemMode stemMode     = STEM_NONE;

    fh_idx idx = 0;
            
    if( createIndex )
    {
        idx = createFullTextIndex( IndexClassName,
                                   Shell::acquireContext( IndexURL ),
                                   caseSensitive,
                                   dropStopWords,
                                   stemMode,
                                   "FrontCodedBlocks (3-in-4)",
                                   0 );
    }
    else
    {
        idx = FullTextIndex::Factory::getFullTextIndex( IndexURL );
    }
    
    while( const char* tmpCSTR = poptGetArg(optCon) )
    {
        try
        {
            string     srcURL = tmpCSTR;
            fh_context src    = Resolve( srcURL );

            cerr << "IndexFiles: adding to index src:" << srcURL << endl;
            fh_docindexer di = FullTextIndex::Factory::makeDocumentIndexer( idx );
            di->addContextToIndex( src );
        }
        catch( exception& e )
        {
            cerr << "error:" << e.what() << endl;
            exit_status = 1;
        }
    }
    
    poptFreeContext(optCon);
    return exit_status;
}
    
    
