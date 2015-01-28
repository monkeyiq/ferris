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

    $Id: ferris-capplet-logging.cpp,v 1.6 2010/09/24 21:31:22 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <glib.h>
#include <gtk/gtk.h>

#include <Ferris.hh>
#include <FerrisUI/GtkFerris.hh>
#include <Ferris_private.hh>
#include <Enamel_priv.hh>

#include <string>
#include <iomanip>

using namespace std;
using namespace Ferris;
using namespace FerrisUI;

 
const string PROGRAM_NAME = "ferris-capplet-logging";

GtkWidget* gtk_window;
GtkWidget* gtk_scrolledwindow;
GtkWidget* gtk_table;
GtkWidget* w_showInPopt;
GtkWidget* w_logToFile;

static const char* ORIGINAL_VALUE_KEY = "ORIGINAL_VALUE_KEY";

struct kv
{
    GtkWidget* lab;
    GtkWidget* slide;
    kv( GtkWidget* l, GtkWidget* s )
        :
        lab(l), slide(s)
        {
        }
};

typedef vector< kv > loginfo_t;
loginfo_t loginfo;

#define DEFAULT_LOGGING_VALUE 511

/**
 * like x<<y but the new places on the right hand side are filled with 1
 */
int left_shift_filled( int v, int shiftsz )
{
    v = v << shiftsz;

    for( int i=shiftsz; i >= 0; --i )
    {
        v |= (1<<i);
    }
    return v;
}


void SaveData(void)
{
//    cerr << "SaveData() filename:" << tostr( GTK_ENTRY(w_logToFile)) << endl;
    
    setEDBString( FDB_GENERAL, SHOW_LOGGING_POPT_OPTIONS_BY_DEFAULT,
                  tostr(gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_showInPopt))));
    setEDBString( FDB_GENERAL, LOGGING_TO_FILENAME_KEY, tostr( GTK_ENTRY(w_logToFile)));

    for( loginfo_t::iterator iter = loginfo.begin(); iter != loginfo.end(); ++iter )
    {
        fh_stringstream vss;
        int val = DEFAULT_LOGGING_VALUE;
        vss << gtk_range_get_value ( GTK_RANGE( iter->slide ));
        vss >> val;
        const string& k = gtk_label_get_text( GTK_LABEL( iter->lab ) );
        setEDBString( FDB_LOGGING, k, tostr( left_shift_filled( 1, val )) );
//        cerr << "k:" << k << " v:" << tostr(vss) << endl;
    }
}

void save_and_quit_cb( GtkButton *button, gpointer user_data )
{
    SaveData();
    gtk_main_quit();
}

void apply_cb( GtkButton *button, gpointer user_data )
{
    SaveData();
}

void quit_cb( GtkButton *button, gpointer user_data )
{
    gtk_main_quit();
}

const char* debuglevel( gdouble dv )
{
    const char* d[] =
        {
            "None",
            "Emergency",
            "Alert",
            "Critical",
            "Error",
            "Warning",
            "Notice",
            "Info",
            "Debug",
            "",
            0
        };

    
    int i = 0;
    int v = static_cast<int>(dv);

    if( i > 8 )
        return "";
    
    for( ; *d[i] && i != v ; ++ i )
    {}


    if( *d[i] )
    {
//    cerr << "debuglevel() i:" << i << " v:" << v << " dv:" << dv << " d[]:" << d[i] << endl;
        return d[i];
    }
    return "";
}

static gchar*
slider_label_cb( GtkScale *scale, gdouble value)
{
    fh_stringstream ss;
    ss << setw(15) << debuglevel( value );
    return g_strdup( tostr(ss).c_str() );
}

static void
all_slider_changed_cb( GtkScale *scale )
{
    gdouble v = gtk_range_get_value ( GTK_RANGE( scale ) );
    
    for( loginfo_t::iterator iter = loginfo.begin();
         iter != loginfo.end(); ++iter )
    {
        gtk_range_set_value ( GTK_RANGE( iter->slide), v );
    }
}


/**
 * unset all 1 bits except the one in the topmost position
 * in:  001010101
 * out: 001000000
 */
int only_set_highest_bit( int v )
{
    int t = v;
    int i = 0;
    
    for( i=0; t ; ++i )
    {
        t >>= 1;
    }
//    --i;
    return i>0 ? ((1 << i)-1) : 1;
}


void make_window()
{
    GtkWidget* w;
    
    gtk_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title ( GTK_WINDOW( gtk_window ), "Ferris logging defaults" );
    gtk_window_set_default_size (GTK_WINDOW (gtk_window), 450, 450);
    gtk_signal_connect(GTK_OBJECT (gtk_window), "destroy", gtk_main_quit, NULL);

    /* Add all the logging information to the tree */
    typedef Ferris::Logging::Loggers_t FL;
    typedef FL::iterator FLI;
    FL& Loggers = Ferris::Logging::getLoggers();
    for( FLI iter = Loggers.begin(); iter != Loggers.end(); ++iter )
    {
        const string& k = iter->first;
        const string& v = getEDBString( FDB_LOGGING, k, tostr(DEFAULT_LOGGING_VALUE) );
        int vi = DEFAULT_LOGGING_VALUE;
        
        fh_stringstream ss;
        ss << v;
        ss >> vi;
        vi = only_set_highest_bit( vi );
//        cerr << "v:" << v << " vi:" << vi << endl;
        
        GtkWidget* lab   = gtk_label_new( k.c_str() );
        GtkWidget* slide = gtk_hscale_new_with_range ( 0, 8, 1 );
        gtk_range_set_value ( GTK_RANGE(slide), Math::log( vi, 2 ) );
        gtk_scale_set_value_pos( GTK_SCALE(slide), GTK_POS_LEFT );
//        g_object_set( G_OBJECT(slide), ORIGINAL_VALUE_KEY, Math::log( vi, 2 ) );
        gtk_signal_connect(GTK_OBJECT (slide), "format-value",
                           GTK_SIGNAL_FUNC(slider_label_cb), NULL);
        
        loginfo.push_back( kv( lab, slide ));
    }



    
    gtk_table = gtk_table_new ( loginfo.size()+2, 2, true );

    /* show data in popt output by default */
    int r = 0;
    w = w_showInPopt = gtk_check_button_new_with_label("Show popt logging options");
    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(w),
        isTrue( getEDBString( FDB_GENERAL, SHOW_LOGGING_POPT_OPTIONS_BY_DEFAULT, "0" )));
    
    gtk_table_attach_defaults(GTK_TABLE(gtk_table), w, 0, 2, r, r+1 );
    ++r;

    
    gtk_table_attach_defaults(GTK_TABLE(gtk_table),
                              gtk_label_new( "Log to file" ),
                              0, 1, r, r+1 );
    w = w_logToFile = gtk_entry_new();
    gtk_entry_set_text( GTK_ENTRY(w),
                        getEDBString( FDB_GENERAL, LOGGING_TO_FILENAME_KEY, "" ).c_str());
    gtk_table_attach_defaults(GTK_TABLE(gtk_table), w, 1, 2, r, r+1 );
    ++r;
    
    /* One ring to control them all */
    GtkWidget* lab   = gtk_label_new( "all" );
    GtkWidget* slide = gtk_hscale_new_with_range ( 0, 8, 1 );
    gtk_range_set_value ( GTK_RANGE(slide), 0 );
    gtk_scale_set_value_pos( GTK_SCALE(slide), GTK_POS_LEFT );
    gtk_signal_connect(GTK_OBJECT (slide), "format-value",
                       GTK_SIGNAL_FUNC(slider_label_cb), NULL);
    gtk_signal_connect(GTK_OBJECT (slide), "value-changed",
                       GTK_SIGNAL_FUNC(all_slider_changed_cb), NULL);
    gtk_table_attach_defaults(GTK_TABLE(gtk_table), lab,   0, 1, r, r+1 );
    gtk_table_attach_defaults(GTK_TABLE(gtk_table), slide, 1, 2, r, r+1 );

    /* each seperate logging facility */
    ++r;
    for( loginfo_t::iterator iter = loginfo.begin();
         iter != loginfo.end(); ++iter, ++r )
    {
        gtk_table_attach_defaults(GTK_TABLE(gtk_table), iter->lab,   0, 1, r, r+1 );
        gtk_table_attach_defaults(GTK_TABLE(gtk_table), iter->slide, 1, 2, r, r+1 );
    }
    
    gtk_scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_add_with_viewport(
        GTK_SCROLLED_WINDOW(gtk_scrolledwindow), GTK_WIDGET(gtk_table));
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (gtk_scrolledwindow),
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* vbx = GTK_WIDGET(gtk_vbox_new(0,0));
    GtkWidget* hbx = GTK_WIDGET(gtk_hbox_new(0,0));

    GtkWidget* b;

    b = gtk_button_new_from_stock( GTK_STOCK_OK );
    gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
    gtk_signal_connect(GTK_OBJECT (b), "clicked",
                       GTK_SIGNAL_FUNC(save_and_quit_cb), NULL);

    b = gtk_button_new_from_stock( GTK_STOCK_APPLY );
    gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
    gtk_signal_connect(GTK_OBJECT (b), "clicked",
                       GTK_SIGNAL_FUNC(apply_cb), NULL);
    
    b = gtk_button_new_from_stock( GTK_STOCK_CANCEL );
    gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
    gtk_signal_connect(GTK_OBJECT (b), "clicked",
                       GTK_SIGNAL_FUNC(quit_cb), NULL);
    
    gtk_box_pack_start(GTK_BOX(vbx), gtk_scrolledwindow, 1, 1, 0 );
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

