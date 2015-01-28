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

    $Id: ferris-first-time-user-ui.cpp,v 1.3 2011/06/18 21:30:23 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <gmodule.h>

#include <Ferris/Ferris.hh>
#include <../ferris-first-time-user.hh>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ferris-first-time-user-ui";

GtkWidget* gtk_window;
GtkWidget* gtk_notebook;
GtkWidget* gtk_prevbutton;
GtkWidget* gtk_nextbutton;
int LAST_PAGE_NUMBER = 1;

void execute()
{
    cerr << "execute()" << endl;
}

void nb_prev( GtkButton *button, gpointer user_data )
{
    gtk_notebook_prev_page( GTK_NOTEBOOK( gtk_notebook ) );
    gtk_widget_set_sensitive( gtk_nextbutton, 1 );
    gtk_button_set_label( GTK_BUTTON(gtk_nextbutton), GTK_STOCK_GO_FORWARD );
    
    if( !gtk_notebook_get_current_page( GTK_NOTEBOOK( gtk_notebook )) )
    {
        gtk_widget_set_sensitive( gtk_prevbutton, 0 );
    }
}
void nb_next( GtkButton *button, gpointer user_data )
{
    if( LAST_PAGE_NUMBER == gtk_notebook_get_current_page( GTK_NOTEBOOK( gtk_notebook )) )
    {
        execute();
        exit(0);
    }
    
    gtk_notebook_next_page( GTK_NOTEBOOK( gtk_notebook ) );
    gtk_widget_set_sensitive( gtk_prevbutton, 1 );

    if( LAST_PAGE_NUMBER == gtk_notebook_get_current_page( GTK_NOTEBOOK( gtk_notebook )) )
    {
        gtk_button_set_label( GTK_BUTTON(gtk_nextbutton), GTK_STOCK_EXECUTE );
//        gtk_widget_set_sensitive( gtk_nextbutton, 0 );
    }
}


GtkWidget* createPageLabel( string s )
{
    GtkWidget* label = GTK_WIDGET(gtk_label_new(""));
        
    fh_stringstream ss;
//    ss << "<span size=\"large\" weight=\"bold\" background=\"#5555FF\">";
    ss << "<span size=\"large\" weight=\"bold\" >";
    ss << s;
    ss << "</span>";
    
    gtk_label_set_markup( GTK_LABEL(label), tostr(ss).c_str() );
    gtk_label_set_selectable( GTK_LABEL(label), 0 );
    return GTK_WIDGET(label);
}

typedef configAtoms_t::iterator ITER;
typedef list< ConfigAtom* > pageAtoms_t;
typedef pageAtoms_t::iterator PGITER;

void make_page( pageAtoms_t pageAtoms, int maxpp )
{
    GtkWidget* box = GTK_WIDGET(gtk_vbox_new(0,0));
    GtkWidget* table;
    table = gtk_table_new ( 1, 2, true );
    int r = 0;

    for( int pp = -1; pp < maxpp; ++pp )
    {
//        pageAtoms_t::iterator piter = find( pageAtoms.begin(), pageAtoms.end(), pp );
        pageAtoms_t::iterator piter = pageAtoms.begin();
        for( ; piter != pageAtoms.end(); ++piter )
        {
            if( (*piter)->getPageNumber() == pp )
                break;
        }
        
        if( pageAtoms.end() != piter )
        {
            if( pp == -1 )
            {
                gtk_box_pack_start(GTK_BOX(box),
                                   createPageLabel( (*piter)->getDesc() ),
                                   0, 0, 3 );
            }

            GtkWidget* lab = gtk_label_new( (*piter)->getDesc() );
            gtk_table_attach_defaults(GTK_TABLE(table), lab, 0, 1, r, r+1 );
            GtkWidget* obj = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(table), obj, 1, 2, r, r+1 );
        }
    }

    gtk_box_pack_start(GTK_BOX(box), table, 1, 1, 0 );
    gtk_notebook_append_page( GTK_NOTEBOOK( gtk_notebook ), box, 0 );
}

void make_pages()
{
    pageAtoms_t pageAtoms;
    GtkTextBuffer* tb = 0;
    GtkWidget* tv = 0;
    int page = 0;
    int count = 1;
    
    configAtoms_t& ca = Factory::getConfigAtoms();
    while( count )
    {
        int maxpp = 0;
        count = 0;

        /*
         * Create collection of page elements
         */
        for( configAtoms_t::iterator iter = ca.begin(); iter != ca.end(); ++iter )
        {
            if( iter->second.getPageNumber() == page )
            {
                ++count;
//                pageAtoms.insert( make_pair( iter->second.getPagePlacement(), &(*iter) ));
                pageAtoms.push_back( &iter->second );
                maxpp = MAX( maxpp, iter->second.getPagePlacement() );
            }
        }

        make_page( pageAtoms, maxpp );
    }
}


void make_window()
{
    GtkTextBuffer* tb = 0;
    GtkWidget* tv = 0;
    
    gtk_window = gtk_window_new( GTK_WINDOW_TOPLEVEL);
    GtkWindow* w = GTK_WINDOW( gtk_window );
    gtk_window_set_type_hint(    w, GDK_WINDOW_TYPE_HINT_DIALOG );
    gtk_window_set_title(        w, "Welcome to the ferris world!" );
    gtk_window_set_default_size( w, 550, 350);
    gtk_signal_connect(GTK_OBJECT(w), "destroy", gtk_main_quit, NULL);

//    gtk_pagelabel = gtk_label_new("");
    
        
    gtk_notebook = gtk_notebook_new();
    GtkNotebook* nb = GTK_NOTEBOOK( gtk_notebook );
    gtk_notebook_set_show_tabs( nb, 0 );

    tv = gtk_text_view_new();
    tb = gtk_text_view_get_buffer (GTK_TEXT_VIEW (tv));
    gtk_text_buffer_set_text( tb, "About ferris waffle", -1 );
    gtk_text_view_set_editable( GTK_TEXT_VIEW(tv), 0 );
    gtk_text_view_set_cursor_visible( GTK_TEXT_VIEW(tv), 0 );
    {
        GtkWidget* box = GTK_WIDGET(gtk_vbox_new(0,0));
        gtk_box_pack_start(GTK_BOX(box),
                           createPageLabel( "Welcome..." ),
                           0, 0, 3 );
        gtk_box_pack_start(GTK_BOX(box), tv, 1, 1, 3 );
        gtk_notebook_append_page( nb, box, 0 );
    }
    
    make_pages();
    
    tv = gtk_text_view_new();
    tb = gtk_text_view_get_buffer (GTK_TEXT_VIEW (tv));
    gtk_text_buffer_set_text( tb, "Your settings will now be setup."
                              "\n"
                              "\nFor more configuration options please see the gnome control center."
                              "\n"
                              "\nYou may also run the ferris config applets from the command line using "
                              "\nferris-capplet-sqlplus and other ferris-capplet- prefix applications."
                              "\n\nEnjoy.",
                              -1 );
    gtk_text_view_set_editable( GTK_TEXT_VIEW(tv), 0 );
    gtk_text_view_set_cursor_visible( GTK_TEXT_VIEW(tv), 0 );
    {
        GtkWidget* box = GTK_WIDGET(gtk_vbox_new(0,0));
        gtk_box_pack_start(GTK_BOX(box),
                           createPageLabel( "Click to complete..." ),
                           0, 0, 3 );
        gtk_box_pack_start(GTK_BOX(box), tv, 1, 1, 3 );
        gtk_notebook_append_page( nb, box, 0 );
    }
//    gtk_notebook_append_page( nb, tv, 0 );


    GtkWidget* vbx = GTK_WIDGET(gtk_vbox_new(0,0));
    GtkWidget* hbx = GTK_WIDGET(gtk_hbox_new(0,0));
    GtkWidget* b;

    gtk_prevbutton = b = GTK_WIDGET(gtk_button_new_from_stock( GTK_STOCK_GO_BACK ));
    gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
    gtk_signal_connect(GTK_OBJECT (b), "clicked",
                       GTK_SIGNAL_FUNC(nb_prev), NULL);
    gtk_widget_set_sensitive( b, 0 );

    gtk_nextbutton = b = GTK_WIDGET(gtk_button_new_from_stock( GTK_STOCK_GO_FORWARD ));
    gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
    gtk_signal_connect(GTK_OBJECT (b), "clicked",
                       GTK_SIGNAL_FUNC(nb_next), NULL);


    GtkWidget* body_vb = GTK_WIDGET(gtk_vbox_new(0,0));
//     GdkColor color;
//     color.red   = 0x55;
//     color.green = 0x55;
//     color.blue  = 0xFF;
//     gtk_widget_modify_bg( body_vb,  GTK_STATE_NORMAL, &color );
//     gtk_widget_modify_bg( body_vb,  GTK_STATE_INSENSITIVE, &color );
//    gtk_box_pack_start(GTK_BOX(body_vb), GTK_WIDGET(gtk_pagelabel), 0, 0, 3 );
    gtk_box_pack_start(GTK_BOX(body_vb), GTK_WIDGET(nb), 1, 1, 0 );
        
    gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(body_vb), 1, 1, 0 );
    gtk_box_pack_start(GTK_BOX(vbx), hbx, 0, 0, 0 );
    gtk_container_add(GTK_CONTAINER(gtk_window), vbx);
}


int main( int argc, char** argv )
{
    gtk_init( &argc, &argv );
    make_window();
    gtk_widget_show_all( gtk_window );
    gtk_main();
    return 0;
}
