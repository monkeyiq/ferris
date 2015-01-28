/******************************************************************************
*******************************************************************************
*******************************************************************************

    index add command line client
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

    $Id: findexadd.cpp,v 1.7 2010/09/24 21:31:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris/Ferris.hh>
#include <Ferris/FullTextIndexer.hh>
#include <Ferris/EAIndexer.hh>
#include <Ferris/EAIndexer_private.hh>
#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;
using namespace FullTextIndex;

const string PROGRAM_NAME = "findexadd";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


void progressf( fh_context c, streamsize bytesDone, streamsize totalBytes )
{
    cerr << c->getURL() << " " << bytesDone << " / " << totalBytes << endl;
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int exit_status = 0;
int TotalFilesDoneCount = 0;
unsigned long DontCheckIfAlreadyThere = 0;
unsigned long Verbose              = 0;
unsigned long userSelectedTotalFilesToIndexPerRun = 0;

void addToIndexFromFileList( fh_idx& idx, fh_istream& fiss )
{
    string srcURL;
            
    fh_docindexer indexer = FullTextIndex::Factory::makeDocumentIndexer( idx );
    indexer->setDontCheckIfAlreadyThere( DontCheckIfAlreadyThere );
    if( Verbose )
    {
        indexer->getProgressSig().connect( sigc::ptr_fun( progressf ) );
    }

    while( getline( fiss, srcURL ))
    {
        try
        {
            if( srcURL.empty() )
                continue;

            fh_context c  = Resolve( srcURL );

            if( idx->getIndexMethodSupportsIsFileNewerThanIndexedVersion() )
            {
                if( !idx->isFileNewerThanIndexedVersion( c ) )
                {
                    if( Verbose )
                    {
                        cerr << "Skipping:" << srcURL << endl;
                    }
                    continue;
                }
            }
            
            indexer->addContextToIndex( c );
            ++TotalFilesDoneCount;

            if( userSelectedTotalFilesToIndexPerRun
                && TotalFilesDoneCount >= userSelectedTotalFilesToIndexPerRun )
            {
                cerr << "Have reached the selected max number of files to index for this run..." << endl;
                return;
            }
        }
        catch( exception& e )
        {
            cerr << "cought error:" << e.what() << endl;
            exit_status = 1;
        }
    }
}

int main( int argc, char** argv )
{
    try
    {
//        const char* CreateTypeName_CSTR    = 0;
        const char* IndexPath_CSTR         = 0;
        const char* FilelistFile_CSTR      = 0;
        unsigned long FilelistStdin        = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "skip-already-indexed-check", 'S', POPT_ARG_NONE, &DontCheckIfAlreadyThere, 0,
                  "don't check if the context is already indexed, just add it.", "" },
                
                { "index-path", 'P', POPT_ARG_STRING, &IndexPath_CSTR, 0,
                  "which index to use", "" },

                { "filelist-file", 'f', POPT_ARG_STRING, &FilelistFile_CSTR, 0,
                  "file containing the URLs of the files to index (eg. made by find . >foo)", "" },

                { "filelist-stdin", '1', POPT_ARG_NONE, &FilelistStdin, 0,
                  "read filenames from stdin to index", "" },

                { "total-files-to-index-per-run", 'N', POPT_ARG_INT, &userSelectedTotalFilesToIndexPerRun, 0,
                  "only add this many files to the index and then exit. Note that skipped files do not count towards this total, files must be really (re)indexed to count.", "" },
                
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

        if( FilelistFile_CSTR )
        {
            string filelistFile = FilelistFile_CSTR;
            fh_ifstream fiss( filelistFile );
            addToIndexFromFileList( idx, fiss );
        }
        if( FilelistStdin )
        {
            fh_istream fiss = Ferris::Factory::fcin();
            addToIndexFromFileList( idx, fiss );
        }
        
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            try
            {
                string srcURL = tmpCSTR;
                fh_context c  = Resolve( srcURL );

                fh_docindexer indexer = FullTextIndex::Factory::makeDocumentIndexer( idx );
                indexer->setDontCheckIfAlreadyThere( DontCheckIfAlreadyThere );
                if( Verbose )
                    indexer->getProgressSig().connect( sigc::ptr_fun( progressf ) );
                indexer->addContextToIndex( c );
                ++TotalFilesDoneCount;
            }
            catch( exception& e )
            {
                cerr << "cought error:" << e.what() << endl;
                exit_status = 1;
            }
        }
        idx->sync();
        cerr << "Total contexts indexed:" << TotalFilesDoneCount << endl;
    }
    catch( exception& e )
    {
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


