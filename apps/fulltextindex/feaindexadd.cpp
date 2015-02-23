/******************************************************************************
*******************************************************************************
*******************************************************************************

    eaindex add command line client
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

    $Id: feaindexadd.cpp,v 1.9 2010/09/24 21:31:16 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>


/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris/Ferris.hh>
#include <Ferris/Ferrisls.hh>
#include <Ferris/FullTextIndexer.hh>
#include <Ferris/EAIndexer.hh>
#include <Ferris/EAIndexer_private.hh>
#include <popt.h>
#include <unistd.h>
#include <boost/regex.hpp>

using namespace std;
using namespace Ferris;
using namespace FullTextIndex;

const string PROGRAM_NAME = "feaindexadd";

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

Time::Benchmark m_totalRuntime( "Total indexing time", false );

class Ferrisls_feaindexadd_display;
FERRIS_SMARTPTR( Ferrisls_feaindexadd_display, fh_eaindexadd_display );
class Ferrisls_feaindexadd_display
    :
    public EAIndex::Ferrisls_feaindexadd_display_base
{
    typedef EAIndex::Ferrisls_feaindexadd_display_base _Base;
    
public:
    Ferrisls_feaindexadd_display( EAIndex::fh_idx idx = 0 )
        :
        _Base( idx )
        {
        }
    void ShowAttributes( fh_context ctx );
    void EnteringContext(fh_context ctx);
    void LeavingContext(fh_context ctx);
    int main( int argc, char** argv );
};


void
Ferrisls_feaindexadd_display::ShowAttributes( fh_context ctx )
{
    if( m_verbose )
        cerr << "Indexing context:" << ctx->getURL() << endl;

    if( shouldEAtryToAddToEAIndex() )
    {
        try
        {
            m_indexer->addContextToIndex( ctx );
        }
        catch( exception& e )
        {
            cerr << "feaindexadd error:" << e.what() << endl;
        }
    }
    
    if( shouldEAtryToAddToFulltextIndex() )
    {
        try
        {
            fh_docindexer indexer = getDocumentIndexer();
            indexer->addContextToIndex( ctx );
        }
        catch( exception& e )
        {
            cerr << "feaindexadd error:" << e.what() << endl;
        }
    }
    
    if( m_showTotals )
    {
        cerr << "ShowAttributes() ADDING CONTEXT ctx:" << ctx->getURL() << endl;
        ++m_contextCount;
    }
    
}

void
Ferrisls_feaindexadd_display::EnteringContext(fh_context ctx)
{
    if( m_verbose )
        cerr << "Entering context:" << ctx->getURL() << endl;

    m_indexer->EnteringContext( ctx );
}

void
Ferrisls_feaindexadd_display::LeavingContext(fh_context ctx)
{
    if( m_verbose )
        cerr << "Leaving context:" << ctx->getURL() << endl;

    m_indexer->LeavingContext( ctx );
}

int
Ferrisls_feaindexadd_display::main( int argc, char** argv )
{
    try
    {
        struct poptOption optionsTable[] =
            {
                FERRIS_EAINDEXADD_OPTIONS( this )
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

        m_totalRuntime.start();
        getPoptCollector()->ArgProcessingDone( optCon );

        /******************************/
        /******************************/
        /******************************/

        
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            string srcURL = tmpCSTR;
            try
            {
                LG_EAIDX_D << "adding src:" << srcURL << endl;
                perform( srcURL );
            }
            catch( exception& e )
            {
                cerr << "----------------------" << endl;
                cerr << "for: " << srcURL << endl;
                cerr << "cought error:" << e.what() << endl;
                exit_status = 1;
            }
        }
//        d->sync();
        printTotals();

        m_TotalFilesDoneCount += getFilesIndexedCount();

        m_totalRuntime.stop();
        cerr << "Total contexts indexed:" << m_TotalFilesDoneCount << endl;
        m_totalRuntime.print();

        if( m_TotalFilesDoneCount )
        {
            double avgTimePerContext = m_totalRuntime.getElapsedTime() / m_TotalFilesDoneCount;
            cerr << "Average time per context:" << setprecision(3) << avgTimePerContext << endl;
        }
    }
    catch( exception& e )
    {
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int main( int argc, char** argv )
{
    fh_eaindexadd_display d = new Ferrisls_feaindexadd_display();
    d->main( argc, argv );
    return d->getExitStatus();
}
