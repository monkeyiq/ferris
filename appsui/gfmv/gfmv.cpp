/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris mv GTK2 client
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

    $Id: gfmv.cpp,v 1.5 2011/06/18 21:30:23 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <gmodule.h>

#include <algorithm>
#include <iomanip>

#include <FerrisCopy_private.hh>
#include <FerrisMove.hh>
#include <General.hh>

#include <FerrisUI/gtkferriscellrenderertext.hh>
#include <FerrisUI/gtkferristreestore.hh>
#include <FerrisUI/TreeStoreDriver.hh>
#include <FerrisUI/GtkFerris.hh>
#include <FerrisUI/FerrisCopyUI.hh>
#include <FerrisUI/FerrisRemoveUI.hh>

#include <popt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

#include <glib.h>
#include <gtk/gtk.h>

namespace FerrisUI
{
    using namespace std;
    using namespace Ferris;


    static int alarmed = 0;

    static void
    sig_alarm( int signo )
    {
        alarmed = 1;
        return;
    }

    const string PROGRAM_NAME = "gfmv";

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

    class FerrisMove_GTK;

    FERRIS_SMARTPTR( FerrisMove_GTK, fh_mv_gtk );


    class FerrisMove_GTK
        :
        public FerrisMv,
        public GTK_TreeWalkClient
    {
        typedef FerrisCopy       _Base;
        typedef FerrisMove_GTK   _Self;

        bool alwaysPreserveExisting;
        int alarmsPerSecond;

        
    public:

        FerrisMove_GTK()
            :
            alwaysPreserveExisting( false ),
            alarmsPerSecond(4),
            fcp_cache( 0 ),
            frm_cache( 0 ),
            CopySigHandler( 0 ),
            RemoveSigHandler( 0 )
            {
                makeMainWindow( PROGRAM_NAME );
            }

        virtual ~FerrisMove_GTK()
            {
            }


        void addBodyElements( GtkWidget* table, int& r )
            {
                addSourceRow(      table, r );
                addDestinationRow( table, r );
                m_currentObjectLab = m_srclab;
//                addCurrent(        table, r );
                addStartTimeRow(   table, r );
                addElapsedTimeRow( table, r );
                addSpeedRow(       table, r );
                addProgressRow(    table, r );
                addVCRControls(    table );
            }

        /******************************************************************************/
        /******************************************************************************/
        /******************************************************************************/

        fh_cp fcp_cache;
        fh_rm frm_cache;
        FERRIS_SMARTPTR( FerrisCopy_SignalHandler, CopySigHandler_t );
        CopySigHandler_t CopySigHandler;
        FERRIS_SMARTPTR( FerrisRm_SignalHandler, RemoveSigHandler_t );
        RemoveSigHandler_t RemoveSigHandler;

        virtual fh_cp getCopyObject()
            {
                if( !isBound( fcp_cache ) )
                {
                    fcp_cache = new FerrisCopy_TTY();
                    CopySigHandler = new FerrisCopy_SignalHandler( this, GetImpl(fcp_cache) );
                    CopySigHandler->restartTimer();
                }
                return fcp_cache;
            }
    
        virtual fh_rm getRemoveObject()
            {
                if( !isBound( frm_cache ) )
                {
                    frm_cache = FerrisRm::CreateObject();
                    RemoveSigHandler = new FerrisRm_SignalHandler( this, GetImpl(frm_cache) );
                }
                return frm_cache;
            }

        void
        OnSkippingContext( FerrisMv& thisobj,
                                     string srcDescription,
                                     string dstDescription,
                                     string reason )
            {
                addToSkipped( srcDescription, reason );
            }
        
        /******************************************************************************/
        /******************************************************************************/
        /******************************************************************************/
        /******************************************************************************/
        /******************************************************************************/
    
        virtual bool handleInteractiveMode( const std::string& oldrdn, const std::string& newrdn )
            {
                if( Interactive )
                {
                    gtk_label_set_text( GTK_LABEL(m_srclab), oldrdn.c_str() );
                    gtk_label_set_text( GTK_LABEL(m_dstlab), newrdn.c_str() );
                    cerr << "handleInteractiveMode() oldrdn:" << oldrdn
                         << " newrdn:" << newrdn
                         << endl;
                    
                    fh_context src = Resolve( getSrcURL() );
//                     cerr << " src:" << src->getURL() << endl;
//                     cerr << "newrdn:" << newrdn << endl;
                    try {
                        fh_context dst = Resolve( newrdn );
//                         cerr << " dst:" << dst->getURL() << endl;
                        getCopyObject();
                        hadUserInteraction = true;
                        bool rc = CopySigHandler->fcwin.perform( src, dst );
                        cerr << "handleInteractiveMode() rc:" << rc << endl;
                        return rc;
                    }
                    catch( NoSuchSubContext& e )
                    {
                    }
                }
                return true;
            }
    
        virtual bool handleVerboseMode( const std::string& oldrdn, const std::string& newrdn )
            {
                gtk_label_set_text( GTK_LABEL(m_srclab), oldrdn.c_str() );
                gtk_label_set_text( GTK_LABEL(m_dstlab), newrdn.c_str() );
                processAllPendingEvents();
                return false;
            }
    
        int main( int argc, char** argv );
    };

    int
    FerrisMove_GTK::main( int argc, char** argv )
    {
        const char* DstNameCSTR              = 0;
        struct poptOption optionsTable[] =
            {
                { "target-directory", 0, POPT_ARG_STRING, &DstNameCSTR, 0,
                  "Specify destination explicity, all remaining URLs are assumed to be source files",
                  "DIR" },
                    
                FERRIS_MOVE_OPTIONS( this )
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

            cerr << "gfmv getting file names" << endl;
            while( const char* RootNameCSTR = poptGetArg(optCon) )
            {
                string RootName = RootNameCSTR;
                cerr << "Adding potential source:" << RootName << endl;
                srcs.push_back( RootName );
            }

            if( srcs.empty() )
            {
                cerr << "No objects need to be moved\n" << endl;
                poptPrintHelp(optCon, stderr, 0);
                exit(1);
            }
            
            /*
             * Setup the destination from either explicit command line or last arg
             */
            string DstName;
            if( DstNameCSTR )
            {
                DstName = DstNameCSTR;
            }
            else
            {
                DstName = srcs.back();
                srcs.pop_back();
            }
                
            setDstURL( DstName );
        
            {
                fh_stringstream ss;
                ss << "dst: " << DstName;
                gtk_label_set_text( GTK_LABEL(m_dstlab), g_strdup( tostr(ss).c_str() ) );
            }
        
            /************************************************************/
            /************************************************************/
            /************************************************************/

            bool MovingToDir = srcs.size() > 1;
            try
            {
                fh_context c = Resolve( DstName );
                if( toint( getStrAttr( c, "is-dir", "0" )))
                {
                    MovingToDir = true;
                }
            }
            catch( NoSuchSubContext& e )
            {
            }
            setMovingToDir( MovingToDir );
            
            populateModelWithSelection( m_treemodel, srcs.begin(), srcs.end() );
            makeDefaultColumnViews();
            processAllPendingEvents();

            if( signal( SIGALRM, sig_alarm ) == SIG_ERR )
            {
                cerr << "Can not attach sigalarm" << endl;
                exit(1);
            }
      
            for( srcs_t::iterator iter = srcs.begin(); iter != srcs.end(); ++iter )
            {
                string SrcName = *iter;
                cerr << "src:" << SrcName << endl;
                    
                setSrcURL( SrcName );
                move();
            }

            cerr << "gfmv, AutoClose:" << AutoClose
                 << " hadUserInteraction:" << hadUserInteraction
                 << endl;
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
            GtkWidget* dialog = gtk_message_dialog_new (
                0,
                GtkDialogFlags(GTK_DIALOG_MODAL),
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_CLOSE,
                tostr(ss).c_str()
                );
            gtk_dialog_run (GTK_DIALOG (dialog));
            gtk_widget_destroy (dialog);
            exit(1);
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "Error: " << e.what() << endl;
            cerr << tostr(ss);
            GtkWidget* dialog = gtk_message_dialog_new (
                0,
                GtkDialogFlags(GTK_DIALOG_MODAL),
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_CLOSE,
                tostr(ss).c_str()
                );
            gtk_dialog_run (GTK_DIALOG (dialog));
            gtk_widget_destroy (dialog);
            exit(1);
        }
        poptFreeContext(optCon);
        return 0;
    }
 
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
};

int main( int argc, char** argv )
{
    gtk_init( &argc, &argv );
    FerrisUI::fh_mv_gtk obj = new FerrisUI::FerrisMove_GTK();
    return obj->main( argc, argv );
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
