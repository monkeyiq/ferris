/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris feaindex/findex GTK2 client
    Copyright (C) 2008 Ben Martin

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

    $Id: gfcp.cpp,v 1.6 2008/12/23 21:30:12 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <gmodule.h>

#include <Ferris/Ferris.hh>
#include <Ferris/Ferrisls.hh>
#include <Ferris/FullTextIndexer.hh>
#include <Ferris/EAIndexer.hh>
#include <Ferris/EAIndexer_private.hh>
#include <ValueRestorer.hh>
#include <Ferris/Ferrisls_AggregateData.hh>

#include <FerrisUI/gtkferriscellrenderertext.hh>
#include <FerrisUI/gtkferristreestore.hh>
#include <FerrisUI/TreeStoreDriver.hh>
#include <FerrisUI/GtkFerris.hh>
#include <FerrisUI/FerrisCopyUI.hh>

#include <popt.h>
#include <unistd.h>

#include <glib.h>
#include <gtk/gtk.h>

using namespace std;
using namespace Ferris;
using namespace FerrisUI;
using namespace FullTextIndex;


const string PROGRAM_NAME = "gfindexadd";


void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

class IndexAddGTK;
FERRIS_SMARTPTR( IndexAddGTK, fh_IndexAddGTK );



class IndexAddGTK
    :
    public EAIndex::Ferrisls_feaindexadd_display_base,
    public GTK_TreeWalkClient
{
    typedef EAIndex::Ferrisls_feaindexadd_display_base _Base;
    typedef IndexAddGTK   _Self;

    long m_currentFileNumber;
    
public:

    IndexAddGTK( EAIndex::fh_idx idx = 0 )
        :
        _Base( idx ),
        m_currentFileNumber( 0 )
        {
            makeMainWindow( PROGRAM_NAME );
        }

    virtual ~IndexAddGTK()
        {
        }


    void addBodyElements( GtkWidget* table, int& r )
        {
            addSeparatorRow(   table, r, "Current");
            addProgressRow(    table, r );
            addSourceRow(      table, r );
//            addElapsedTimeRow( table, r );
//            addSpeedRow(       table, r );
            addSeparatorRow(   table, r, "Overall");
            addStartTimeRow(   table, r );
            addOverallProgressRow(    table, r );
            addVCRControls(    table );
        }


    void performActionForSource( fh_context c );
    int main( int argc, char** argv );

    void
    updateProgressForAggregateData( const Ferrisls_aggregate_t& agg,
                                    bool added )
        {
            setOverallProgress( m_currentFileNumber, agg.filecount );
        }

    virtual bool shouldPrecacheSourceSize()
        {
            return true;
        }

    void OnEAProgress( fh_context c, streamsize current, streamsize total )
        {
            setProgress( current, total );
            processAllPendingEvents();
        }
    
};

void
IndexAddGTK::performActionForSource( fh_context ctx )
{
    LG_EAIDX_D << "IndexAddGTK::performActionForSource() ctx:" << ctx->getURL() << endl;
//    perform( c->getURL() );

    if( m_verbose )
        setSourceLabel( ctx->getURL() );
    setOverallProgress( m_currentFileNumber++, m_totalagg.filecount );
    processAllPendingEvents();
    
    if( shouldEAtryToAddToEAIndex() )
    {
        try
        {
            m_indexer->addContextToIndex( ctx );
        }
        catch( exception& e )
        {
            addToSkipped( ctx->getURL(), e.what() );
        }
    }
    
    if( shouldEAtryToAddToFulltextIndex() )
    {
        try
        {
            cerr << "adding to fulltext index too! c:" << ctx->getURL() << endl;
            fh_docindexer indexer = getDocumentIndexer();
            indexer->addContextToIndex( ctx );
        }
        catch( exception& e )
        {
            addToSkipped( ctx->getURL(), e.what() );
        }
    }
    
    if( m_showTotals )
    {
        cerr << "ShowAttributes() ADDING CONTEXT ctx:" << ctx->getURL() << endl;
        ++m_contextCount;
    }
    
}



int
IndexAddGTK::main( int argc, char** argv )
{
    const char* DstNameCSTR              = 0;
    struct poptOption optionsTable[] =
        {
            FERRIS_EAINDEXADD_OPTIONS( this )
            FERRIS_POPT_OPTIONS
            POPT_AUTOHELP
            POPT_TABLEEND
        };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* src1 src2 ... dst");

    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
//                 switch (c) {
//                 }
    }

    getPoptCollector()->ArgProcessingDone( optCon );
    setVerbose( true );
    
    if (argc < 3)
    {
        poptPrintHelp(optCon, stderr, 0);
        exit(1);
    }

    showMainWindow( m_sloth );
    
    try
    {
        typedef vector<string> srcs_t;
        srcs_t srcs;
                
        while( const char* RootNameCSTR = poptGetArg(optCon) )
        {
            string RootName = RootNameCSTR;
            srcs.push_back( RootName );
        }
                
        {
            stringstream ss;
            ss << PROGRAM_NAME;
            setWindowTitle( ss.str() );
        }
        
        /************************************************************/
        /************************************************************/
        /************************************************************/

        populateModelWithSelection( m_treemodel, srcs.begin(), srcs.end() );
        makeDefaultColumnViews();
        processAllPendingEvents();

        m_indexer->getProgressSig().connect( sigc::mem_fun( *this, &_Self::OnEAProgress ) );
        
//        sighandler.restartTimer();
        
        performActionForAllRemainingSources();
        
        if( getShouldRunMainLoop( m_autoClose, m_hadUserInteraction ))
        {
            runMainWindow( m_sloth );
        }
    }
    catch( Quit_Requested& e )
    {
        cerr << "cought quit request, exiting." << endl;
        exit(0);
    }
    catch( NoSuchContextClass& e )
    {
        fh_stringstream ss;
        ss << "Invalid context class given e:" << e.what() << endl;
        cerr << tostr(ss);
        RunErrorDialog( tostr(ss) );
        exit(1);
    }
    catch( exception& e )
    {
        fh_stringstream ss;
        ss << "Error: " << e.what() << endl;
        cerr << tostr(ss);
        RunErrorDialog( tostr(ss) );
        exit(1);
    }
    poptFreeContext(optCon);
    return 0;
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

int main( int argc, char** argv )
{
    gtk_init( &argc, &argv );
    fh_IndexAddGTK obj = new IndexAddGTK();
    return obj->main( argc, argv );
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/



