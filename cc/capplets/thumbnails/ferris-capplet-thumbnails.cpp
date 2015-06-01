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

    $Id: ferris-capplet-thumbnails.cpp,v 1.4 2010/09/24 21:31:22 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>

#include <Ferris/Ferris.hh>
#include <Ferris/Ferris_private.hh>
#include <FerrisUI/GtkFerris.hh>

#include <string>
#include <iomanip>

using namespace std;
using namespace Ferris;
using namespace FerrisUI;

const string PROGRAM_NAME = "ferris-capplet-thumbnails";

GtkWidget* gtk_window;
GtkWidget* gtk_scrolledwindow;
GtkWidget* gtk_table;
GtkWidget* w_max_width_or_height;
GtkWidget* w_jpeg_iq;


void SaveData()
{
    try
    {
        setEDBString( FDB_GENERAL,
                      CFG_THUMBNAILS_MAX_DESIRED_WIDTH_OR_HEIGHT_K,
                      tostr((int)gtk_range_get_value(GTK_RANGE(w_max_width_or_height))),
                      true, true );

        setEDBString( FDB_GENERAL,
                      CFG_THUMBNAILS_JPEG_IMG_QUALITY_K,
                      tostr((int)gtk_range_get_value(GTK_RANGE(w_jpeg_iq))),
                      true, true );
    }
    catch( exception& e )
    {
        RunErrorDialog( e.what() );
    }
}

void LoadData()
{
    string t;
    
    t = getEDBString( FDB_GENERAL,
                      CFG_THUMBNAILS_MAX_DESIRED_WIDTH_OR_HEIGHT_K,
                      CFG_THUMBNAILS_MAX_DESIRED_WIDTH_OR_HEIGHT_DEFAULT );
    gtk_range_set_value ( GTK_RANGE(w_max_width_or_height), toint( t ) );




    t = getEDBString( FDB_GENERAL,
                      CFG_THUMBNAILS_JPEG_IMG_QUALITY_K,
                      CFG_THUMBNAILS_JPEG_IMG_QUALITY_DEFAULT );
    gtk_range_set_value ( GTK_RANGE(w_jpeg_iq), toint( t ) );
    
    
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
    GtkWidget* lab;
    GtkWidget* slide;

    int c=0;
    int r=0;
    
    gtk_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title ( GTK_WINDOW( gtk_window ), "Ferris indexing" );
    gtk_window_set_default_size (GTK_WINDOW (gtk_window), 450, 450);
    gtk_signal_connect(GTK_OBJECT (gtk_window), "destroy", gtk_main_quit, NULL);

    gtk_table = gtk_table_new ( 4, 2, false );

    r=0;
    lab  = gtk_label_new( "when creating thumbnails this is the\n"
                          "max width or height to give icons\n"
                          "while preserving their aspect ratio" );
    gtk_table_attach_defaults(GTK_TABLE(gtk_table), lab, 0, 1, r, r+1 );
    slide = gtk_hscale_new_with_range ( 16, 192, 16 );
    gtk_range_set_value ( GTK_RANGE(slide),
                          toint(CFG_THUMBNAILS_MAX_DESIRED_WIDTH_OR_HEIGHT_DEFAULT) );
    gtk_scale_set_value_pos( GTK_SCALE(slide), GTK_POS_RIGHT );
    w_max_width_or_height = slide;
    gtk_table_attach_defaults(GTK_TABLE(gtk_table), w_max_width_or_height, 1, 2, r, r+1 );
    
    c=0;
    ++r;
    lab  = gtk_label_new( "image quality for jpeg compression of thumbnails" );
    gtk_table_attach_defaults(GTK_TABLE(gtk_table), lab, 0, 1, r, r+1 );
    slide = gtk_hscale_new_with_range ( 50, 100, 10 );
    gtk_range_set_value ( GTK_RANGE(slide),
                          toint(CFG_THUMBNAILS_JPEG_IMG_QUALITY_DEFAULT) );
    gtk_scale_set_value_pos( GTK_SCALE(slide), GTK_POS_RIGHT );
    w_jpeg_iq = slide;
    gtk_table_attach_defaults(GTK_TABLE(gtk_table), w_jpeg_iq, 1, 2, r, r+1 );


    
//     gtk_scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
//     gtk_scrolled_window_add_with_viewport(
//         GTK_SCROLLED_WINDOW(gtk_scrolledwindow), GTK_WIDGET(gtk_table));
//     gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (gtk_scrolledwindow),
//                                     GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

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
    
//    gtk_box_pack_start(GTK_BOX(vbx), gtk_scrolledwindow, 1, 1, 0 );
    gtk_box_pack_start(GTK_BOX(vbx), gtk_table, 1, 1, 0 );
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

