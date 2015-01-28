/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris rm GTK2 client
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

    $Id: gfrm.cpp,v 1.5 2011/06/18 21:30:23 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <gmodule.h>

#include <algorithm>
#include <iomanip>

#include <FerrisRemove_private.hh>
#include <General.hh>

#include <FerrisUI/gtkferriscellrenderertext.hh>
#include <FerrisUI/gtkferristreestore.hh>
#include <FerrisUI/TreeStoreDriver.hh>
#include <FerrisUI/GtkFerris.hh>
#include <FerrisUI/FerrisRemoveUI.hh>

#include <popt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

#include <glib.h>
#include <gtk/gtk.h>

using namespace std;
using namespace Ferris;
using namespace FerrisUI;

const string PROGRAM_NAME = "gfrm";

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

class FerrisRm_GTK;

FERRIS_SMARTPTR( FerrisRm_GTK, fh_rm_gtk );


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


class FerrisRm_GTK
    :
    public FerrisRm,
    public GTK_TreeWalkClient
{
    typedef FerrisRm     _Base;
    typedef FerrisRm_GTK _Self;


    FerrisRm_SignalHandler sighandler;
    
public:

    FerrisRm_GTK()
        :
        sighandler( this, this )
        {
            makeMainWindow( PROGRAM_NAME );
        }

    virtual ~FerrisRm_GTK()
        {
        }

    void addBodyElements( GtkWidget* table, int& r )
        {
            addTarget(         table, r );
            addCurrent(        table, r );
            addStartTimeRow(   table, r );
            addElapsedTimeRow( table, r );
            addVCRControls(    table );
        }
    
    virtual void OnRemoveVerbose( FerrisRm& thisref, fh_context target, std::string desc )
        {}
    virtual void OnSkipping( FerrisRm& thisref, std::string desc,  std::string reason )
        {}
    bool OnAskRemove( FerrisRm& thisref, fh_context target, std::string desc )
        {}
    
    int main( int argc, char** argv );
};


int
FerrisRm_GTK::main( int argc, char** argv )
{
    struct poptOption optionsTable[] =
        {
            FERRIS_REMOVE_OPTIONS( this )
            FERRIS_POPT_OPTIONS
            POPT_AUTOHELP
            POPT_TABLEEND
        };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* url1 url2 ...");

    /* Now do options processing */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
    }

    getPoptCollector()->ArgProcessingDone( optCon );
    setVerbose( true );
    
    if (argc < 2)
    {
        poptPrintHelp(optCon, stderr, 0);
        exit(1);
    }

    showMainWindow( Sloth );
    
    try
    {
        typedef vector<string> targets_t;
        targets_t targets;
                
        while( const char* RootNameCSTR = poptGetArg(optCon) )
        {
            string RootName = RootNameCSTR;
            targets.push_back( RootName );
        }
        
        populateModelWithSelection( m_treemodel, targets.begin(), targets.end() );
        makeDefaultColumnViews();
        processAllPendingEvents();

        for( targets_t::iterator iter = targets.begin(); iter != targets.end(); ++iter )
        {
            string RootName = *iter;
            
            setTarget( RootName );
            gtk_label_set_text( GTK_LABEL(m_targetlab), RootName.c_str() );
            time_t actionstart_tt;
            actionstart_tt = Time::getTime();
            gtk_label_set_text( GTK_LABEL(m_starttimelab),
                                Time::toTimeString( actionstart_tt ).c_str() );
            try
            {
                remove();
            }
//             catch( Skip_Top_Level_Object& e )
//             {
//                 GtkTreeIter iter;
//                 gtk_tree_store_append( m_skipmodel, &iter, 0 );
//                 gtk_tree_store_set( m_skipmodel, &iter,
//                                     SKIP_REASON, "user requested to skip tree removal",
//                                     SKIP_DESC, RootName.c_str(),
//                                     -1 );
//             }
            catch( ... )
            {
                throw;
            }
        }
        
        /* Give them the GUI forever */
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

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int main( int argc, char** argv )
{
    gtk_init( &argc, &argv );
    fh_rm_gtk obj = new FerrisRm_GTK();
    return obj->main( argc, argv );
}
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
