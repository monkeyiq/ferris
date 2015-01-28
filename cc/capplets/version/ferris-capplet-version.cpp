/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

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

    $Id: ferris-capplet-version.cpp,v 1.4 2010/09/24 21:31:23 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <glib.h>
#include <gtk/gtk.h>

#include <config.h>
#include <Ferris.hh>
#include <Enamel_priv.hh>

#include <string>
#include <iomanip>

#include <ferris-capplet-version.hh>

using namespace std;
using namespace Ferris;

 
const string PROGRAM_NAME = "ferris-capplet-version";

GtkWidget* gtk_window;
GtkWidget* gtk_label;
GtkNotebook* gtk_notebook;

void save_and_quit_cb( GtkButton *button, gpointer user_data )
{
    gtk_main_quit();
}

void quit_cb( GtkButton *button, gpointer user_data )
{
    gtk_main_quit();
}


void make_window()
{
    GtkWidget* sw;
    GtkWidget* page;

    gtk_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title ( GTK_WINDOW( gtk_window ), "Ferris versions" );
    gtk_window_set_default_size (GTK_WINDOW (gtk_window), 600, 450);
    gtk_signal_connect(GTK_OBJECT (gtk_window), "destroy", gtk_main_quit, NULL);

    int r=0;
    gtk_notebook = GTK_NOTEBOOK(gtk_notebook_new());
    page = gtk_table_new ( 1, 1, false );
    {
        fh_stringstream ss;
        ss << "ferris-capplet-version: $Id: ferris-capplet-version.cpp,v 1.4 2010/09/24 21:31:23 ben Exp $\n"
           << "ferris         version: " << VERSION << nl
#ifdef G_DISABLE_ASSERT
           << "assert checks: off" << nl
#else
           << "assert checks: on" << nl
#endif
           << "Written by Ben Martin, aka monkeyiq" << nl
           << nl
           << "Copyright (C) 2001 Ben Martin" << nl
           << "This is free software; see the source for copying conditions.  There is NO\n"
           << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
            ;
        gtk_label = gtk_label_new( tostr(ss).c_str() );
        gtk_table_attach_defaults(GTK_TABLE(page), gtk_label, 0, 1, r, r+1 );
    }
    

    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET( page ));
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_notebook_append_page( gtk_notebook, sw, gtk_label_new( "main" ));

    r=0;
    page = gtk_table_new ( 1, 1, false );
    {
        fh_stringstream ss;
        fh_stringstream iss;
        iss << FERRIS_MODULE_COMPILE_LIST;
        string s;
        ss << "Modules compiled:" << nl << nl;
        while(getline( iss, s, ' '))
            ss << s << nl;
        gtk_label = gtk_label_new( tostr(ss).c_str() );
        gtk_table_attach_defaults(GTK_TABLE(page), gtk_label, 0, 1, r, r+1 );
    }
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET( page ));
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_notebook_append_page( gtk_notebook, sw, gtk_label_new( "modules" ));

    r=0;
    page = gtk_table_new ( 1, 1, false );
    {
        fh_stringstream ss;
        ss << "This program is free software; you can redistribute it and/or modify" << nl
           << "it under the terms of the GNU General Public License as published by" << nl
           << "the Free Software Foundation; either version 2 of the License, or" << nl
           << "(at your option) any later version." << nl
           << "" << nl
           << "This program is distributed in the hope that it will be useful," << nl
           << "but WITHOUT ANY WARRANTY; without even the implied warranty of" << nl
           << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" << nl
           << "GNU General Public License for more details." << nl
           << "" << nl
           << "You should have received a copy of the GNU General Public License" << nl
           << "along with this program; if not, write to the Free Software" << nl
           << "Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA" << nl
           << "" << nl
           << "For more details see the COPYING file in the root directory of this" << nl
           << "distribution." << nl
            ;
        gtk_label = gtk_label_new( tostr(ss).c_str() );
        gtk_table_attach_defaults(GTK_TABLE(page), gtk_label, 0, 1, r, r+1 );
    }
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET( page ));
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_notebook_append_page( gtk_notebook, sw, gtk_label_new( "license" ));
    
    
    r=0;
    page = gtk_table_new ( 1, 1, false );
    {
        fh_stringstream ss;
        ss << "IOSIZE:" << FERRIS_IOSIZE << nl
           << FERRIS_IOSIZE_WARNING << nl
            ;
        gtk_label = gtk_label_new( tostr(ss).c_str() );
        gtk_table_attach_defaults(GTK_TABLE(page), gtk_label, 0, 1, r, r+1 );
    }
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET( page ));
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_notebook_append_page( gtk_notebook, sw, gtk_label_new( "misc" ));


    /* ok / cancel part */

    GtkWidget* vbx = GTK_WIDGET(gtk_vbox_new(0,0));
    GtkWidget* hbx = GTK_WIDGET(gtk_hbox_new(0,0));

    GtkWidget* b;

    b = gtk_button_new_from_stock( GTK_STOCK_OK );
    gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
    gtk_signal_connect(GTK_OBJECT (b), "clicked",
                       GTK_SIGNAL_FUNC(save_and_quit_cb), NULL);

    b = gtk_button_new_from_stock( GTK_STOCK_CANCEL );
    gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
    gtk_signal_connect(GTK_OBJECT (b), "clicked",
                       GTK_SIGNAL_FUNC(quit_cb), NULL);
    
    gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(gtk_notebook), 1, 1, 0 );
    gtk_box_pack_start(GTK_BOX(vbx), hbx, 0, 0, 0 );
    
    gtk_container_add(GTK_CONTAINER(gtk_window), vbx);

}

int main( int argc, char** argv )
{
    poptContext optCon;

    const char* copt   = "copt";
    unsigned long iopt = 0;

    try
    {
        struct poptOption optionsTable[] = {
//             { "root-context-class", 0, POPT_ARG_STRING, &RootContextClass, 0,
//               "Name of the class that handles reading the root context", "Native" },
            
            FERRIS_POPT_OPTIONS
            POPT_AUTOHELP
            POPT_TABLEEND
        };

        gtk_init( &argc, &argv );

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* RootName1 ...");

        if (argc < 1) {
//          poptPrintHelp(optCon, stderr, 0);
            poptPrintUsage(optCon, stderr, 0);
            exit(1);
        }


        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {
        }

//        gtk_idle_add( fire_ls_cb, (void*)&ls );
        make_window();
        gtk_widget_show_all( gtk_window );
        gtk_main();
    }
    catch( exception& e )
    {
        cerr << PROGRAM_NAME << ": cought e:" << e.what() << endl;
        exit(1);
    }
        
    
    poptFreeContext(optCon);
    return 0;
}

