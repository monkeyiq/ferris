/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris cp GTK2 client
    Copyright (C) 2002 Ben Martin

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

    $Id: gfcp.cpp,v 1.9 2011/06/18 21:30:22 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <gmodule.h>

#include <algorithm>
#include <iomanip>

#include <FerrisCopy.hh>
#include <General.hh>
#include <Ferris/Ferrisls_AggregateData.hh>

#include <FerrisUI/gtkferriscellrenderertext.hh>
#include <FerrisUI/gtkferristreestore.hh>
#include <FerrisUI/TreeStoreDriver.hh>
#include <FerrisUI/GtkFerris.hh>
#include <FerrisUI/FerrisCopyUI.hh>

#include <popt.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <sys/time.h>
//#include <signal.h>
#include <unistd.h>

#include <glib.h>
#include <gtk/gtk.h>

#include <boost/regex.hpp>

using namespace std;
using namespace Ferris;
using namespace FerrisUI;


const string PROGRAM_NAME = "gfcp";


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

class FerrisCopy_GTK;
FERRIS_SMARTPTR( FerrisCopy_GTK, fh_cp_gtk );
class FerrisCopy_GTK
    :
    public FerrisCopy,
    public GTK_TreeWalkClient
{
    typedef FerrisCopy       _Base;
    typedef FerrisCopy_GTK   _Self;

    FerrisCopy_SignalHandler      sighandler;
    friend class FerrisCopy_SignalHandler;

public:

    FerrisCopy_GTK()
        :
        sighandler( this, this )
        {
            makeMainWindow( PROGRAM_NAME );
        }

    virtual ~FerrisCopy_GTK()
        {}

    void addBodyElements( GtkWidget* table, int& r )
        {
            addSeparatorRow(   table, r, "Current");
            addProgressRow(    table, r );
            addSourceRow(      table, r );
            addDestinationRow( table, r );
            addElapsedTimeRow( table, r );
            addSpeedRow(       table, r );
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
        guint64 sz = agg.sizeFilesOnly;
        
        if( added )
        {
            TotalBytesToCopy += sz;
        }
        else
        {
            TotalBytesCopied = 0;
            TotalBytesToCopy = sz;
        }
    }
    virtual bool shouldPrecacheSourceSize()
        {
            return FerrisCopy::shouldPrecacheSourceSize();
        }
    
    

};


void
FerrisCopy_GTK::performActionForSource( fh_context c )
{
    setSrcURL( c->getURL() );
    copy();
}


int
FerrisCopy_GTK::main( int argc, char** argv )
{
    const char* DstNameCSTR              = 0;
    struct poptOption optionsTable[] =
        {
            { "target-directory", 0, POPT_ARG_STRING, &DstNameCSTR, 0,
              "Specify destination explicity, all remaining URLs are assumed to be source files",
              "DIR" },
                    
            FERRIS_COPY_OPTIONS( this )
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

    showMainWindow( Sloth );
    
    try
    {
        typedef vector<string> srcs_t;
        srcs_t srcs;
                
        while( const char* RootNameCSTR = poptGetArg(optCon) )
        {
            string RootName = RootNameCSTR;
            srcs.push_back( RootName );
        }
                
        /*
         * Setup the destination from either explicit command line or last arg
         */
        string DstName;
        if( DstNameCSTR )
        {
            setDstIsDirectory( true );
            DstName = DstNameCSTR;
        }
        else
        {
            DstName = srcs.back();
            srcs.pop_back();
        }
                
//        cerr << "dst:" << DstName << endl;
        setDstURL( DstName );
        {
            stringstream ss;
            ss << PROGRAM_NAME << " to " << DstName;
            setWindowTitle( ss.str() );
        }
        
        {
            fh_stringstream ss;
            ss << "dst: " << DstName;
            gtk_label_set_text( GTK_LABEL(m_dstlab), g_strdup( tostr(ss).c_str() ) );
        }
        
        /************************************************************/
        /************************************************************/
        /************************************************************/

        populateModelWithSelection( m_treemodel, srcs.begin(), srcs.end() );
        makeDefaultColumnViews();
        processAllPendingEvents();

        sighandler.restartTimer();
        
//         for( srcs_t::iterator iter = srcs.begin(); iter != srcs.end(); ++iter )
//         {
//             string SrcName = *iter;
// //            cerr << "src:" << SrcName << endl;
                    
//             setSrcURL( SrcName );
//             copy();
//         }

        performActionForAllRemainingSources();
        
        if( getShouldRunMainLoop( AutoClose, hadUserInteraction ))
        {
            runMainWindow( Sloth );
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
//    cerr << "gfcp main() " << endl;
    gtk_init( &argc, &argv );
    fh_cp_gtk obj = new FerrisCopy_GTK();
    return obj->main( argc, argv );
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/



