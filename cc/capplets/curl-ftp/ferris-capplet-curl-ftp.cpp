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

    $Id: ferris-capplet-curl-ftp.cpp,v 1.4 2010/09/24 21:31:22 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <glib.h>
#include <gtk/gtk.h>

#include <Ferris.hh>
#include <Ferris_private.hh>
#include <Enamel_priv.hh>

#include <string>
#include <iomanip>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ferris-capplet-curl";

GtkWidget* gtk_window;
GtkWidget* gtk_table;
GtkNotebook* gtk_notebook;
GtkWidget* gtk_proxypage;
GtkWidget* gtk_sslpage;
GtkWidget* gtk_agentpage;
GtkWidget* gtk_miscpage;

GtkWidget* w_useproxy;
GtkWidget* w_proxyname;
GtkWidget* w_proxyport;
GtkWidget* w_proxyuserpass;
GtkWidget* w_tunnel;
GtkWidget* w_usessl;
GtkWidget* w_sslversion;
GtkWidget* w_useragent;
GtkWidget* w_clientif;
GtkWidget* w_maxredir;
GtkWidget* w_verbose;
GtkWidget* w_usenetrc;
GtkWidget* w_timeout;

void LoadData()
{
    string s;

    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(w_useproxy),
        isTrue( getEDBString( FDB_GENERAL, "curl-use-proxy", "" )));
    
    gtk_entry_set_text(
        GTK_ENTRY(w_proxyname),
        getEDBString( FDB_GENERAL, "curl-use-proxy-name", "" ).c_str());
    gtk_entry_set_text(
        GTK_ENTRY(w_proxyport),
        getEDBString( FDB_GENERAL, "curl-use-proxy-port", "" ).c_str());
    gtk_entry_set_text(
        GTK_ENTRY(w_proxyuserpass),
        getEDBString( FDB_GENERAL, "curl-use-proxy-userpass", "" ).c_str());

    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(w_tunnel),
        isTrue( getEDBString( FDB_GENERAL, "curl-use-proxy-tunnel", "" )));

    gtk_entry_set_text(
        GTK_ENTRY(w_timeout),
        getEDBString( FDB_GENERAL, "curl-transfer-timeout", "" ).c_str());

    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(w_usessl),
        isTrue( getEDBString( FDB_GENERAL, "curl-use-ssl", "0" )));

    gtk_range_set_value (
        GTK_RANGE(w_sslversion),
        toint(getEDBString( FDB_GENERAL, "curl-use-ssl-version-number", "3" )));
    
    gtk_entry_set_text(GTK_ENTRY(w_useragent), 
                       getEDBString( FDB_GENERAL, "curl-agent-name", "" ).c_str());
    gtk_entry_set_text(GTK_ENTRY(w_clientif), 
                       getEDBString( FDB_GENERAL, "curl-client-interface", "" ).c_str());

    gtk_range_set_value (
        GTK_RANGE(w_maxredir),
        toint(getEDBString( FDB_GENERAL, "curl-max-redirects", "8" )));

    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(w_verbose),
        isTrue( getEDBString( FDB_GENERAL, "curl-verbose", "0" )));

    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(w_usenetrc),
        isTrue( getEDBString( FDB_GENERAL, "curl-use-netrc", "1" )));
    
}

void SaveData()
{
    setEDBString( FDB_GENERAL, "curl-use-proxy",
                  tostr(gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_useproxy))));
    setEDBString( FDB_GENERAL, "curl-use-proxy-name",
                  gtk_entry_get_text(GTK_ENTRY(w_proxyname)));
    setEDBString( FDB_GENERAL, "curl-use-proxy-port",
                  gtk_entry_get_text(GTK_ENTRY(w_proxyport)));
    setEDBString( FDB_GENERAL, "curl-use-proxy-userpass",
                  gtk_entry_get_text(GTK_ENTRY(w_proxyuserpass)));
    setEDBString( FDB_GENERAL, "curl-use-proxy-tunnel",
                  tostr(gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_tunnel))));

     /************/

    setEDBString( FDB_GENERAL, "curl-transfer-timeout",
                  gtk_entry_get_text(GTK_ENTRY(w_timeout)));

    /************/
    
    setEDBString( FDB_GENERAL, "curl-use-ssl",
                  tostr(gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_usessl))));
    setEDBString( FDB_GENERAL, "curl-use-ssl-version-number",
                  tostr((int)gtk_range_get_value(GTK_RANGE(w_sslversion))));
     
     /************/

    setEDBString( FDB_GENERAL, "curl-agent-name",
                  gtk_entry_get_text(GTK_ENTRY(w_useragent)));
    setEDBString( FDB_GENERAL, "curl-client-interface",
                  gtk_entry_get_text(GTK_ENTRY(w_clientif)));
    setEDBString( FDB_GENERAL, "curl-max-redirects",
                  tostr((int)gtk_range_get_value(GTK_RANGE(w_maxredir))));

     /************/

    setEDBString( FDB_GENERAL, "curl-verbose",
                  tostr(gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_verbose))));
    setEDBString( FDB_GENERAL, "curl-use-netrc",
                  tostr(gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_usenetrc))));
}


void save_and_quit_cb( GtkButton *button, gpointer user_data )
{
    SaveData();
    gtk_main_quit();
}

void quit_cb( GtkButton *button, gpointer user_data )
{
    gtk_main_quit();
}


void make_window()
{
    GtkWidget* page;
    GtkWidget* lab;
    GtkWidget* slide;
    GtkWidget* sw;
    int r=0;

    gtk_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title ( GTK_WINDOW( gtk_window ), "Ferris curl" );
    gtk_window_set_default_size (GTK_WINDOW (gtk_window), 450, 450);
    gtk_signal_connect(GTK_OBJECT (gtk_window), "destroy", gtk_main_quit, NULL);
    gtk_notebook = GTK_NOTEBOOK(gtk_notebook_new());

    r=0;
    page = gtk_table_new ( 5, 2, false );
    lab  = gtk_label_new( "" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, 0, 1, r, r+1 );
    w_useproxy = gtk_check_button_new_with_label("use proxy");
    gtk_table_attach_defaults(GTK_TABLE(page), w_useproxy, 1, 2, r, r+1 );
    ++r;
    lab  = gtk_label_new( "proxy server" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, 0, 1, r, r+1 );
    w_proxyname = gtk_entry_new_with_max_length(200);
    gtk_table_attach_defaults(GTK_TABLE(page), w_proxyname, 1, 2, r, r+1 );
    ++r;
    lab  = gtk_label_new( "proxy port" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, 0, 1, r, r+1 );
    w_proxyport = gtk_entry_new_with_max_length(6);
    gtk_table_attach_defaults(GTK_TABLE(page), w_proxyport, 1, 2, r, r+1 );
    ++r;
    lab  = gtk_label_new( "proxy user/pass\n[user name]:[password]" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, 0, 1, r, r+1 );
    w_proxyuserpass = gtk_entry_new_with_max_length(6);
    gtk_table_attach_defaults(GTK_TABLE(page), w_proxyuserpass, 1, 2, r, r+1 );
    ++r;
    lab  = gtk_label_new( "" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, 0, 1, r, r+1 );
    w_tunnel = gtk_check_button_new_with_label("tunnel");
    gtk_table_attach_defaults(GTK_TABLE(page), w_tunnel, 1, 2, r, r+1 );
    ++r;
    lab  = gtk_label_new( "transfer timeout (sec)" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, 0, 1, r, r+1 );
    w_timeout = gtk_entry_new_with_max_length(9);
    gtk_table_attach_defaults(GTK_TABLE(page), w_timeout, 1, 2, r, r+1 );
    
    
    sw = gtk_proxypage = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET( page ));
//    GTK_POLICY_AUTOMATIC 
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_NEVER, GTK_POLICY_NEVER);
    gtk_notebook_append_page( gtk_notebook, sw, gtk_label_new( "proxy" ));


    r=0;
    page = gtk_table_new ( 2, 2, false );
    lab  = gtk_label_new( "" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, 0, 1, r, r+1 );
    w_usessl = gtk_check_button_new_with_label("use ssl");
    gtk_table_attach_defaults(GTK_TABLE(page), w_usessl, 1, 2, r, r+1 );
    ++r;
    lab  = gtk_label_new( "ssl version" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, 0, 1, r, r+1 );
    slide = gtk_hscale_new_with_range ( 2, 4, 1 );
    gtk_range_set_value ( GTK_RANGE(slide), 3 );
    gtk_scale_set_value_pos( GTK_SCALE(slide), GTK_POS_RIGHT );
    w_sslversion = slide;
    gtk_table_attach_defaults(GTK_TABLE(page), w_sslversion, 1, 2, r, r+1 );
    
    

    sw = gtk_sslpage = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET( page ));
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_NEVER, GTK_POLICY_NEVER);
    gtk_notebook_append_page( gtk_notebook, sw, gtk_label_new( "ssl" ));

    
    r=0;
    page = gtk_table_new ( 3, 2, false );

    lab  = gtk_label_new( "user-agent" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, 0, 1, r, r+1 );
    w_useragent = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(page), w_useragent, 1, 2, r, r+1 );
    ++r;
    lab  = gtk_label_new( "client interface" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, 0, 1, r, r+1 );
    w_clientif = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(page), w_clientif, 1, 2, r, r+1 );
    ++r;
    lab  = gtk_label_new( "max redirects" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, 0, 1, r, r+1 );
    slide = gtk_hscale_new_with_range ( 0, 65, 1 );
    gtk_range_set_value ( GTK_RANGE(slide), 8 );
    gtk_scale_set_value_pos( GTK_SCALE(slide), GTK_POS_RIGHT );
    w_maxredir = slide;
    gtk_table_attach_defaults(GTK_TABLE(page), w_maxredir, 1, 2, r, r+1 );
    
    
    sw = gtk_agentpage = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET( page ));
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_NEVER, GTK_POLICY_NEVER);
    gtk_notebook_append_page( gtk_notebook, sw, gtk_label_new( "agent" ));

    r=0;
    page = gtk_table_new ( 2, 1, false );

    w_verbose = gtk_check_button_new_with_label("verbose");
    gtk_table_attach_defaults(GTK_TABLE(page), w_verbose, 0, 1, r, r+1 );
    ++r;
    w_usenetrc = gtk_check_button_new_with_label("use ~/.netrc");
    gtk_table_attach_defaults(GTK_TABLE(page), w_usenetrc, 0, 1, r, r+1 );

    
    sw = gtk_miscpage = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET( page ));
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_NEVER, GTK_POLICY_NEVER);
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
        LoadData();
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

