/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris hal
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

    $Id: ls.cpp,v 1.12 2008/04/27 21:30:11 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <config.h>
#include <gtk/gtk.h>

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisQt_private.hh>
#include <FerrisUI/GtkFerris.hh>
#include <Ferris/Runner.hh>
#include <popt.h>


#include <QtDBus>
#include <QCoreApplication>

#include "DBusGlue/com_libferris_Volume_Manager.h"
#include "DBusGlue/com_libferris_Volume_Manager.cpp"

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ferris-volume-manager-setup-volume-wizard";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

int RETURN_CODE = 0;
bool running = true;
QCoreApplication* app = 0;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static string HAL_USERDB   = "~/.ferris/hal-user-configuration.db/";
static string HAL_ACTIONDB = "~/.ferris/hal-actions.db/";


enum {
    KV_COLUMN_CHECKED,
    KV_COLUMN_K,
    KV_COLUMN_V,
    KV_N_COLUMNS
};
GtkListStore* kv_model = 0;
GtkTreeView* kv_tree = 0;

void kv_toggled_cb( GtkCellRendererToggle *cell,
                         gchar *path_string,
                         gpointer user_data )
{
    int cidx = (int)KV_COLUMN_CHECKED;

    GtkTreeIter iter;
    GtkTreeModel* model = GTK_TREE_MODEL (kv_model);
    GtkTreePath*  path  = gtk_tree_path_new_from_string (path_string);
    gboolean value;
    gtk_tree_model_get_iter (model, &iter, path);
    gtk_tree_model_get (model, &iter, cidx, &value, -1);
    value = !value;
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, cidx, value, -1);
}

GtkWidget* w_mount = 0;
GtkWidget* w_ego = 0;
GtkWidget* w_copyto = 0;
GtkWidget* w_copyto_dir = 0;
GtkWidget* w_gthumbimport = 0;


void toggle_to_sensitive( GtkToggleButton *togglebutton, gpointer user_data)
{
    GtkWidget* w = GTK_WIDGET( user_data );
    gtk_widget_set_sensitive( w, gtk_toggle_button_get_active(togglebutton) );
}


////

fVolumeManager* getFerrisVolumeManager()
{
    static fVolumeManager* ret = 0;
    if( !ret )
    {
        ret = new fVolumeManager(
            "com.libferris.Volume.Manager",
            "/com/libferris/Volume/Manager", 
            QDBusConnection::sessionBus(), app );
    }
    return ret;
}


gint PageFunc( gint current_page, gpointer data)
{
    return current_page+1;
}

fh_context vol = 0;

void Cancel( GtkAssistant *assistant, gpointer user_data)
{
    cerr << "wizard cancelled!" << endl;
    exit(0);
}

void Apply( GtkAssistant *assistant, gpointer user_data)
{
    cerr << "applying wizard settings..." << endl;
    if( vol )
    {
        fh_context ffc = Shell::acquireContext( HAL_USERDB + "ffilter" );
        fh_context c = Shell::acquireContext( ffc->getURL() + "/" + vol->getDirName() );
        cerr << "Setting up context at:" << c->getURL() << endl;
        
        string ffilter = "";
        {
            int predCount = 0;
            stringstream ss;
            GtkTreeIter iter;
            GtkTreeModel* model = GTK_TREE_MODEL (kv_model);
            if( gtk_tree_model_get_iter_first( model, &iter ) )
            {
                while( true )
                {
                    bool checked;
                    char* k = 0;
                    char* v = 0;
                    
                    gtk_tree_model_get( model, &iter,
                                        KV_COLUMN_CHECKED, &checked,
                                        KV_COLUMN_K, &k,
                                        KV_COLUMN_V, &v,
                                        -1);

                    if( checked )
                    {
                        ss << "(" << k << "==" << v << ")";
                        ++predCount;
                    }
                    
                    if( !gtk_tree_model_iter_next( model, &iter ))
                        break;
                }
            }

            if( predCount > 1 )
                ffilter = (string)"(&" + ss.str() + ")";
            else
                ffilter = ss.str();
        }
        
        

        setChild( c, "active", "1");
        setChild( c, "ffilter", ffilter );
        fh_context ac = Shell::acquireContext( c->getURL() + "/actions" );

        int idx = 1;
        if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_mount) ) )
        {
            setChild( ac, tostr(idx), "mount" );
            ++idx;
        }
        if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_ego) ) )
        {
            setChild( ac, tostr(idx), "els" );
            ++idx;
        }
        if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_copyto) ) )
        {
            fh_context child = setChild( ac, tostr(idx), "copyto" );
            string cdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (w_copyto_dir) );
            {
                stringstream ss;
                ss << "--dst " << cdir;
                setStrAttr( child, "command-arguments", tostr(ss) );
            }
            ++idx;
        }
        if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_gthumbimport) ) )
        {
            setChild( ac, tostr(idx), "gthumb-import" );
            ++idx;
        }
        

        //
        // tell the volume manager to have at it!
        //
        fVolumeManager* fvolmgr = getFerrisVolumeManager();

        cerr << "Handle:" << vol->getURL() << endl;
        QString m = fvolmgr->DevicePlugged( vol->getURL().c_str() );
        if( m.length() )
        {
            FerrisUI::RunErrorDialog( tostr(m) );
        }
    }
    
    exit(0);
}


////

void setup( fh_context ctx )
{
    vol = ctx;
    
    GtkWidget* win = gtk_assistant_new();
    GtkAssistant *assistant = GTK_ASSISTANT( win );
    GtkWidget *page = 0;
    page = GTK_WIDGET( gtk_label_new( "Here you can setup what happens\nwhen this volume is inserted in the future..." ));
    gtk_assistant_append_page( GTK_ASSISTANT( win ), GTK_WIDGET(page));
    gtk_assistant_set_page_title( GTK_ASSISTANT( win ), GTK_WIDGET(page), "Welcome to the libferris auto mount wizard" );
    gtk_assistant_set_page_complete( GTK_ASSISTANT( win ), GTK_WIDGET(page), true );
    gtk_assistant_set_page_type( GTK_ASSISTANT( win ), GTK_WIDGET(page), GTK_ASSISTANT_PAGE_INTRO );

    kv_model = gtk_list_store_new ( KV_N_COLUMNS, G_TYPE_BOOLEAN,
                                    G_TYPE_STRING, G_TYPE_STRING );

    stringlist_t kvlist = Util::parseCommaSeperatedList("name,media.bus,storage.bus,storage.model,storage.vendor,usb_device.vendor,usb_device.product");
    for( stringlist_t::iterator kviter = kvlist.begin(); kviter != kvlist.end(); ++kviter )
    {
        string k = *kviter;
        string v = getStrAttr( ctx, k, "" );
        
        GtkTreeIter iter;
        gtk_list_store_append (kv_model, &iter);
        gtk_list_store_set (kv_model, &iter,
                            KV_COLUMN_CHECKED, 0,
                            KV_COLUMN_K, k.c_str(),
                            KV_COLUMN_V, v.c_str(),
                            -1);
        if( k == "name" )
            gtk_list_store_set (kv_model, &iter,
                                KV_COLUMN_CHECKED, 1, -1 );
            
    }
    kv_tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model (GTK_TREE_MODEL(kv_model)));
    
    {
        GtkTreeViewColumn *column;
        GtkCellRenderer* renderer = 0;
        renderer = gtk_cell_renderer_toggle_new ();
        column = gtk_tree_view_column_new_with_attributes ("Match", renderer,
                                                           "active", KV_COLUMN_CHECKED,
                                                           NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (kv_tree), column);
//        g_object_set(renderer, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( renderer ), "toggled",
                               G_CALLBACK (kv_toggled_cb), (gpointer)0,
                               0, GConnectFlags(0));
        

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes ("Key", renderer,
                                                           "text", KV_COLUMN_K,
                                                           NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (kv_tree), column);

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes ("Value", renderer,
                                                           "text", KV_COLUMN_V,
                                                           NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (kv_tree), column);
        
    }
    page = GTK_WIDGET(kv_tree);
//    page = GTK_WIDGET( gtk_label_new( "part2..." ));
    gtk_assistant_append_page( GTK_ASSISTANT( win ), GTK_WIDGET(page));
    gtk_assistant_set_page_title( GTK_ASSISTANT( win ), GTK_WIDGET(page), "How to identify this Volume?" );
    gtk_assistant_set_page_complete( GTK_ASSISTANT( win ), GTK_WIDGET(page), true );

    
    {
//        mount, els, copyto, gthumb-import

        GtkWidget* l = 0;
        GtkWidget* b = 0;
        GtkWidget* w = gtk_table_new ( 2, 2, false );
        int c=0,r=0;

        b = gtk_check_button_new_with_mnemonic( "_mount");
        w_mount = b;
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(b), true );
        c = 0;
        gtk_table_attach_defaults(GTK_TABLE(w), b, c+0, c+2, r, r+1 );
        ++r;
        
        b = gtk_check_button_new_with_mnemonic( "Open in new _ego window");
        w_ego = b;
        c = 0;
        gtk_table_attach_defaults(GTK_TABLE(w), b, c+0, c+2, r, r+1 );
        ++r;

        b = gtk_check_button_new_with_mnemonic( "Start _copying contents to hard disk with libferris");
        w_copyto = b;
        c = 0;
        gtk_table_attach_defaults(GTK_TABLE(w), b, c+0, c+2, r, r+1 );
        ++r;

        
        l = gtk_label_new("Destination directory");
        c = 0;
        gtk_table_attach_defaults(GTK_TABLE(w), l, c+0, c+1, r, r+1 );
        b = gtk_file_chooser_button_new( "Destination directory",
                                         GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER );
        w_copyto_dir = b;
        gtk_widget_set_sensitive( b, false );
        {
            stringstream ss;
            ss << "/tmp/" << getStrAttr( ctx, "name", "" );
            Shell::acquireContext( ss.str() );
            gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (b), ss.str().c_str());
        }
        c++;
        gtk_table_attach_defaults(GTK_TABLE(w), b, c+0, c+1, r, r+1 );
        ++r;
        g_signal_connect (w_copyto, "toggled",
                          G_CALLBACK (toggle_to_sensitive), w_copyto_dir );


        b = gtk_check_button_new_with_mnemonic( "Import _photos with GPhoto");
        w_gthumbimport = b;
        c = 0;
        gtk_table_attach_defaults(GTK_TABLE(w), b, c+0, c+2, r, r+1 );
        ++r;
        
        
        
        page = GTK_WIDGET(w);
    }
    gtk_assistant_append_page( GTK_ASSISTANT( win ), GTK_WIDGET(page));
    gtk_assistant_set_page_title( GTK_ASSISTANT( win ), GTK_WIDGET(page), "What should be done for this Volume?" );
    gtk_assistant_set_page_complete( GTK_ASSISTANT( win ), GTK_WIDGET(page), true );
    
    
    page = GTK_WIDGET(
        gtk_label_new(
            "Click apply to mount this Volume\nand apply your desired actions on it\nnow and in the future..." ));
    gtk_assistant_append_page( GTK_ASSISTANT( win ), GTK_WIDGET(page));
    gtk_assistant_set_page_title( GTK_ASSISTANT( win ), GTK_WIDGET(page), "Finished!" );
    gtk_assistant_set_page_type( GTK_ASSISTANT( win ), GTK_WIDGET(page), GTK_ASSISTANT_PAGE_CONFIRM );
    gtk_assistant_set_page_complete( GTK_ASSISTANT( win ), GTK_WIDGET(page), true );

    
    gtk_assistant_set_forward_page_func ( assistant, PageFunc, assistant, 0 );
    g_signal_connect ( win, "apply", G_CALLBACK( Apply ), win );
    g_signal_connect ( win, "cancel", G_CALLBACK( Cancel ), win );

    gtk_widget_show_all(win);
    Main::mainLoop();

}

GtkWidget* waitingForDeviceDialog = 0;
void OnCreated( NamingEvent_Created* ev,
                const fh_context& subc,
                std::string olddn, std::string newdn )
{
    fh_context vol = subc;
    cerr << "OnCreated() vol:" << vol->getURL() << endl;
    getFerrisVolumeManager()->setIgnorePlugEvents( false );
    
    gtk_widget_destroy( waitingForDeviceDialog );
    waitingForDeviceDialog = 0;

    setup( vol );
//    _exit(0);
}
        

int main( int argc, char** argv )
{
    gtk_init( &argc, &argv );

    const char* ShowColumnsCSTR  = "x";
    unsigned long HideHeadings   = 1;
    unsigned long ShowVersion    = 0;

    struct poptOption optionsTable[] = {

//         { "show-columns", 0, POPT_ARG_STRING, &ShowColumnsCSTR, 0,
//           "Same as --show-ea",
//           "size,mimetype,name" },

        
//         { "hide-headings", 0, POPT_ARG_NONE, &HideHeadings, 0,
//           "Prohibit the display of column headings", 0 },

        
        /*
         * Other handy stuff
         */

        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },
        
        /*
         * Standard Ferris options
         */
        FERRIS_POPT_OPTIONS

        /**
         * Expansion of strange-url://foo*
         */
        FERRIS_SHELL_GLOB_POPT_OPTIONS
        
//         { "show-all-attributes", 0, POPT_ARG_NONE,
//           &ShowAllAttributes, 0,
//           "Show every available bit of metadata about each object in context",
//           0 },
        POPT_AUTOHELP
        POPT_TABLEEND
   };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* ...");

    if (argc < 1) {
//        poptPrintHelp(optCon, stderr, 0);
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }

    
    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
//         switch (c) {
//         }
    }

    if( ShowVersion )
    {
        cout << PROGRAM_NAME << " version: $Id: ls.cpp,v 1.12 2008/04/27 21:30:11 ben Exp $\n"
             << "ferris   version: " << VERSION << nl
             << "Written by Ben Martin, aka monkeyiq" << nl
             << nl
             << "Copyright (C) 2008 Ben Martin" << nl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }

    stringlist_t volumes;
    while( const char* argCSTR = poptGetArg(optCon) )
    {
        if( argCSTR )
        {
            string arg = argCSTR;
            volumes.push_back( arg );
        }
    }

    try
    {
        if( volumes.empty() )
        {
            fh_context volumectx = Resolve( "hal://devices/volume" );
            sigc::connection CreatedConnection;
            CreatedConnection = volumectx->getNamingEvent_Created_Sig().connect( ptr_fun(OnCreated) );

            
            cerr << "rand:" << getFerrisVolumeManager()->Random() << endl;
            getFerrisVolumeManager()->setIgnorePlugEvents( true );
            
            stringstream ss;
            ss << "Please plug in the volume you wish to configure...";
//            FerrisUI::RunInfoDialog( ss.str() );
            GtkWidget* d = gtk_message_dialog_new
                ( GTK_WINDOW(0),
                  GTK_DIALOG_MODAL,
                  GTK_MESSAGE_INFO,
                  GTK_BUTTONS_OK,
                  ss.str().c_str(),
                  0 );
            waitingForDeviceDialog = d;
            gtk_widget_show_all( d );
            gtk_dialog_run (GTK_DIALOG (d));
            if( waitingForDeviceDialog )
                gtk_widget_destroy( waitingForDeviceDialog );
        }
        else
        {
            for( stringlist_t::iterator vi = volumes.begin(); vi != volumes.end(); ++vi )
            {
                string vol = *vi;
                fh_context c = Resolve( vol );
                setup( c );
            }
        }
    }
    catch( exception& e )
    {
        LG_HAL_W << "broker error:" << e.what() << endl;
        cerr << "cought:" << e.what() << endl;
        exit(1);
    }

    poptFreeContext(optCon);
    return 0;
}

