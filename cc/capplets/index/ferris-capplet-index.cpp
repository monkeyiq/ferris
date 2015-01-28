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

    $Id: ferris-capplet-index.cpp,v 1.4 2010/09/24 21:31:22 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <glib.h>
#include <gtk/gtk.h>

#include <Ferris/Ferris.hh>
#include <Ferris/Ferris_private.hh>
#include <Ferris/Enamel_priv.hh>
#include <Ferris/EAIndexer_private.hh>
#include <Ferris/FullTextIndexer_private.hh>
#include <FerrisUI/EditStringList.hh>

#include <string>
#include <iomanip>

using namespace std;
using namespace Ferris;
using namespace FerrisUI;
using namespace Ferris::EAIndex;

 
const string PROGRAM_NAME = "ferris-capplet-index";

GtkWidget* gtk_window;
GtkWidget* gtk_scrolledwindow;
GtkNotebook* gtk_notebook;
GtkWidget* gtk_table;
GtkWidget* w_eanames_ignore;
static fh_editstringlist m_eanamesRegexIgnoreList = new EditStringList();
GtkWidget* w_max_value_size;

static fh_editstringlist m_NonresolvableNotToRemoveRegexList = new EditStringList();

static fh_editstringlist m_stopWordList = new EditStringList();

bool LoadToDefaults = false;

const string INDEXROOT = "~/.ferris/full-text-index";
const string DB_INDEX  = "full-text-index-config.db";

void SaveData()
{
    string v = gtk_entry_get_text(GTK_ENTRY(w_eanames_ignore));
    set_db4_string( EAINDEXROOT + "/" + DB_EAINDEX,
                    IDXMGR_EANAMES_IGNORE_K,
                    v );
    SET_EAINDEX_EANAMES_NOT_TO_INDEX_DEFAULT( v );

    stringlist_t sl = m_eanamesRegexIgnoreList->getStringList();
    string d = Util::createNullSeperatedList( sl );
    set_db4_string( EAINDEXROOT + "/" + DB_EAINDEX,
                    IDXMGR_EANAMES_REGEX_IGNORE_K, d );
    SET_EAINDEX_EANAMES_REGEX_IGNORE_DEFAULT( d );
    
    v = tostr((int)gtk_range_get_value(GTK_RANGE(w_max_value_size)));
    set_db4_string( EAINDEXROOT + "/" + DB_EAINDEX,
                    IDXMGR_MAX_VALUE_SIZE_K,
                    v );
    SET_EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT( v );
    
    {
        stringlist_t sl = m_stopWordList->getStringList();
        string d = Util::createCommaSeperatedList( sl );
        set_db4_string( EAINDEXROOT + "/" + DB_EAINDEX,
                        IDXMGR_STOPWORDSLIST_K, d );
    }
    
    {
        stringlist_t sl = m_NonresolvableNotToRemoveRegexList->getStringList();
        string v = Util::createNullSeperatedList( sl );
        
        set_db4_string( EAINDEXROOT + "/" + DB_EAINDEX,
                        EAIndex::IDXMGR_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_K,
                        v );
        set_db4_string( INDEXROOT + "/" + DB_INDEX,
                        EAIndex::IDXMGR_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_K,
                        v );
        SET_EAINDEX_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_DEFAULT( v );
    }
}

void LoadData()
{
    string t;
    
    t = get_db4_string( EAINDEXROOT + "/" + DB_EAINDEX,
                        IDXMGR_EANAMES_IGNORE_K,
                        GET_EAINDEX_EANAMES_NOT_TO_INDEX_DEFAULT(),
                        false );
    gtk_entry_set_text( GTK_ENTRY( w_eanames_ignore ), t.c_str());


    t = get_db4_string( EAINDEXROOT + "/" + DB_EAINDEX,
                        IDXMGR_EANAMES_REGEX_IGNORE_K,
                        GET_EAINDEX_EANAMES_REGEX_IGNORE_DEFAULT(),
                        false );
    stringlist_t sl;
    Util::parseNullSeperatedList( t, sl );
    m_eanamesRegexIgnoreList->setStringList( sl );


    t = get_db4_string( EAINDEXROOT + "/" + DB_EAINDEX,
                        IDXMGR_MAX_VALUE_SIZE_K,
                        GET_EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT(),
                        false );
    gtk_range_set_value ( GTK_RANGE(w_max_value_size), toint( t ) );
    
    {
        t = get_db4_string( EAINDEXROOT + "/" + DB_EAINDEX,
                            IDXMGR_STOPWORDSLIST_K,
                            IDXMGR_STOPWORDSLIST_DEFAULT,
                            false );
        stringlist_t sl;
        sl = Util::parseCommaSeperatedList( t );
        m_stopWordList->setStringList( sl );
    }

    if( LoadToDefaults )
    {
        t = GET_EAINDEX_EANAMES_NOT_TO_INDEX_DEFAULT();
        gtk_entry_set_text( GTK_ENTRY( w_eanames_ignore ), t.c_str());
        
        t = GET_EAINDEX_EANAMES_REGEX_IGNORE_DEFAULT();
        stringlist_t sl;
        Util::parseNullSeperatedList( t, sl );
        m_eanamesRegexIgnoreList->setStringList( sl );
        
        t = GET_EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT();
        gtk_range_set_value ( GTK_RANGE(w_max_value_size), toint( t ) );

        {
            t = IDXMGR_STOPWORDSLIST_DEFAULT;
            stringlist_t sl;
            sl = Util::parseCommaSeperatedList( t );
            m_stopWordList->setStringList( sl );
        }
    }

    
    {
        t = GET_EAINDEX_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_DEFAULT();
        stringlist_t sl;
        Util::parseNullSeperatedList( t, sl );
        m_NonresolvableNotToRemoveRegexList->setStringList( sl );
    }
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

    gtk_notebook = GTK_NOTEBOOK(gtk_notebook_new());

    gtk_table = gtk_table_new ( 4, 2, false );

    r=0;
    lab  = gtk_label_new( "list of attributes not to index\n"
                          "(comma seperated: as-xml,as-text,whatever)" );
    gtk_table_attach_defaults(GTK_TABLE(gtk_table), lab, 0, 1, r, r+1 );
    w_eanames_ignore = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(gtk_table), w_eanames_ignore, 1, 2, r, r+1 );


    c=0;
    ++r;
    m_eanamesRegexIgnoreList->setColumnLabel( "regex" );
    m_eanamesRegexIgnoreList->setDescriptionLabel( "regex for attributes not to index" );
    gtk_table_attach_defaults(GTK_TABLE(gtk_table),
                              m_eanamesRegexIgnoreList->getWidget(), 0, 2, r, r+1 );

    c=0;
    ++r;
    lab  = gtk_label_new( "max attribute value size to index" );
    gtk_table_attach_defaults(GTK_TABLE(gtk_table), lab, 0, 1, r, r+1 );
    slide = gtk_hscale_new_with_range ( 0, 1024, 50 );
    gtk_range_set_value ( GTK_RANGE(slide), toint(GET_EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT()) );
    gtk_scale_set_value_pos( GTK_SCALE(slide), GTK_POS_RIGHT );
    w_max_value_size = slide;
    gtk_table_attach_defaults(GTK_TABLE(gtk_table), w_max_value_size, 1, 2, r, r+1 );


    
//     gtk_scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
//     gtk_scrolled_window_add_with_viewport(
//         GTK_SCROLLED_WINDOW(gtk_scrolledwindow), GTK_WIDGET(gtk_table));
//     gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (gtk_scrolledwindow),
//                                     GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    gtk_notebook_append_page( gtk_notebook, gtk_table, gtk_label_new( "attribute" ));

    gtk_table = gtk_table_new ( 2, 2, false );

    c=0;
    r=0;
    m_stopWordList->setColumnLabel( "Stop Word" );
    m_stopWordList->setDescriptionLabel( "Words not to index" );
    gtk_table_attach_defaults(GTK_TABLE(gtk_table),
                              m_stopWordList->getWidget(), 0, 2, r, r+1 );
    
    gtk_notebook_append_page( gtk_notebook, gtk_table, gtk_label_new( "full text" ));

    
    {
        gtk_table = gtk_table_new ( 2, 2, false );

        c=0;
        ++r;
        m_NonresolvableNotToRemoveRegexList->setColumnLabel( "regex" );
        m_NonresolvableNotToRemoveRegexList->setDescriptionLabel( "regex of URLs which are "
                                                                  "allowed not to resolve\n"
                                                                  "in index query results" );
        gtk_table_attach_defaults(GTK_TABLE(gtk_table),
                                  m_NonresolvableNotToRemoveRegexList->getWidget(), 0, 2, r, r+1 );
        
        gtk_notebook_append_page( gtk_notebook, gtk_table, gtk_label_new( "common" ));
    }
    

    
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
            { "load-default-values", 0, POPT_ARG_NONE, &LoadToDefaults, 0,
              "Set all widgets to defaults values ignoring any saved configuration", "" },
            
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

