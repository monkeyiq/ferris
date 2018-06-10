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

    $Id: ferris-capplet-general.cpp,v 1.7 2010/09/24 21:31:22 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <glib.h>
#include <gtk/gtk.h>

#include <config.h>
#include <Ferris.hh>
#include <FerrisUI/EditStringList.hh>
#include <Configuration_private.hh>

#include <string>
#include <iomanip>

using namespace std;
using namespace Ferris;
using namespace FerrisUI;

 
const string PROGRAM_NAME = "ferris-capplet-general";

GtkWidget* gtk_window;
GtkWidget* gtk_label;
GtkNotebook* gtk_notebook;

GtkWidget* w_copy_use_sendfile_default;
GtkWidget* w_copy_sendfile_chunksize;

GtkWidget* w_copy_input_in_mmap_mode_by_default;
GtkWidget* w_copy_input_in_mmap_mode_for_objects_larger_than;
GtkWidget* w_copy_output_in_mmap_mode_by_default;
GtkWidget* w_copy_output_in_mmap_mode_for_objects_larger_than;

GtkWidget* w_attributes_to_auto_rea;
GtkWidget* w_attributes_always_available_in_ui_model;
GtkWidget* w_fspot_positive_overlay_regex_string;
GtkWidget* w_attributes_gnu_diff_files;

GtkWidget* w_vm_auto_cleanup;
GtkWidget* w_vm_auto_cleanup_maxfreeatonce;


static fh_editstringlist m_passiveViewList = new EditStringList();

static fh_editstringlist m_xsltStylesheetPathList = new EditStringList();

GtkWidget* w_glob_process_for_file_urls_aswell = 0;
static fh_editstringlist m_globSkipRegexList = new EditStringList();

GtkWidget* w_copy_precalculate = 0;


void LoadData()
{
    string s;

    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(w_copy_use_sendfile_default),
        isTrue( getConfigString( FDB_GENERAL, USE_SENDFILE_IF_POSSIBLE, "0" )));
    gtk_entry_set_text(
        GTK_ENTRY(w_copy_sendfile_chunksize),
        getConfigString( FDB_GENERAL, COPY_INPUT_IN_MMAP_MODE_FOR_OBJECTS_LARGER_THAN, "262144" ).c_str());

    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(w_copy_input_in_mmap_mode_by_default),
        isTrue( getConfigString( FDB_GENERAL, COPY_INPUT_IN_MMAP_MODE_BY_DEFAULT, "" )));
    gtk_entry_set_text(
        GTK_ENTRY(w_copy_input_in_mmap_mode_for_objects_larger_than),
        getConfigString( FDB_GENERAL, COPY_INPUT_IN_MMAP_MODE_FOR_OBJECTS_LARGER_THAN, "" ).c_str());

    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(w_copy_output_in_mmap_mode_by_default),
        isTrue( getConfigString( FDB_GENERAL, COPY_OUTPUT_IN_MMAP_MODE_BY_DEFAULT, "" )));
    gtk_entry_set_text(
        GTK_ENTRY(w_copy_output_in_mmap_mode_for_objects_larger_than),
        getConfigString( FDB_GENERAL, COPY_OUTPUT_IN_MMAP_MODE_FOR_OBJECTS_LARGER_THAN, "" ).c_str());

    gtk_entry_set_text(
        GTK_ENTRY(w_attributes_to_auto_rea),
        getConfigString( FDB_GENERAL,
                      CFG_ATTRIBUTES_TO_AUTO_REA_K,
                      CFG_ATTRIBUTES_TO_AUTO_REA_DEFAULT ).c_str());

    gtk_entry_set_text(
        GTK_ENTRY(w_attributes_always_available_in_ui_model),
        getConfigString( FDB_GENERAL,
                      CFG_ATTRIBUTES_ALWAYS_IN_UI_MODEL_K,
                      CFG_ATTRIBUTES_ALWAYS_IN_UI_MODEL_DEFAULT ).c_str());

    gtk_entry_set_text(
        GTK_ENTRY(w_fspot_positive_overlay_regex_string),
        getConfigString( FDB_GENERAL,
                      CFG_FSPOT_POSITIVE_OVERLAY_REGEX_K,
                      CFG_FSPOT_POSITIVE_OVERLAY_REGEX_DEF ).c_str());
    
    gtk_entry_set_text(
        GTK_ENTRY(w_attributes_gnu_diff_files),
        getConfigString( FDB_GENERAL,
                      CFG_ATTRIBUTES_GNU_DIFF_CMD_FILES_K,
                      CFG_ATTRIBUTES_GNU_DIFF_CMD_FILES_DEFAULT ).c_str());

    
    {
        string d = getConfigString( FDB_GENERAL,
                                 CFG_FORCE_PASSIVE_VIEW_K,
                                 CFG_FORCE_PASSIVE_VIEW_DEFAULT );
        cerr << "load data:" << d << endl;
        stringlist_t sl;
        Util::parseNullSeperatedList( d, sl );
        m_passiveViewList->setStringList( sl );
    }
    

    {
        string d = getConfigString( FDB_GENERAL,
                                 CFG_XSLTFS_STYLESHEET_PATH_K,
                                 CFG_XSLTFS_STYLESHEET_PATH_DEFAULT );
        stringlist_t sl;
        Util::parseNullSeperatedList( d, sl );
        m_xsltStylesheetPathList->setStringList( sl );
    }


    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(w_glob_process_for_file_urls_aswell),
        isTrue( getConfigString( FDB_GENERAL,
                              CFG_GLOB_SKIP_FILE_URLS_K,
                              CFG_GLOB_SKIP_FILE_URLS_DEFAULT )));
    {
        string d = getConfigString( FDB_GENERAL,
                                 CFG_GLOB_SKIP_REGEX_LIST_K,
                                 CFG_GLOB_SKIP_REGEX_LIST_DEFAULT );
        stringlist_t sl;
        Util::parseNullSeperatedList( d, sl );
        m_globSkipRegexList->setStringList( sl );
    }
    

    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(w_copy_precalculate),
        isTrue( getConfigString( FDB_GENERAL,
                              CFG_PRECALCULATE_FOR_COPY_K,
                              CFG_PRECALCULATE_FOR_COPY_DEFAULT )));
    

    
//     {
//         gtk_toggle_button_set_active(
//             GTK_TOGGLE_BUTTON(w_vm_auto_cleanup),
//             isTrue( getConfigString( FDB_GENERAL, "vm-auto-cleanup", "0" )));

//         string t = getConfigString( FDB_GENERAL,
//                                  "vm-auto-cleanup-maxfreeatonce",
//                                  "500" );
//         gtk_range_set_value ( GTK_RANGE(w_vm_auto_cleanup_maxfreeatonce), toint( t ) );
//     }
    
}

void SaveData()
{
    setConfigString( FDB_GENERAL, USE_SENDFILE_IF_POSSIBLE,
                     tostr(gtk_toggle_button_get_active(
                               GTK_TOGGLE_BUTTON(w_copy_use_sendfile_default))));
    setConfigString( FDB_GENERAL, SENDFILE_CHUNK_SIZE,
                     gtk_entry_get_text(
                         GTK_ENTRY(w_copy_sendfile_chunksize)));

    setConfigString( FDB_GENERAL, COPY_INPUT_IN_MMAP_MODE_BY_DEFAULT,
                     tostr(gtk_toggle_button_get_active(
                               GTK_TOGGLE_BUTTON(w_copy_input_in_mmap_mode_by_default))));
    setConfigString( FDB_GENERAL, COPY_INPUT_IN_MMAP_MODE_FOR_OBJECTS_LARGER_THAN,
                     gtk_entry_get_text(
                         GTK_ENTRY(w_copy_input_in_mmap_mode_for_objects_larger_than)));

    setConfigString( FDB_GENERAL, COPY_OUTPUT_IN_MMAP_MODE_BY_DEFAULT,
                     tostr(gtk_toggle_button_get_active(
                               GTK_TOGGLE_BUTTON(w_copy_output_in_mmap_mode_by_default))));
    setConfigString( FDB_GENERAL, COPY_OUTPUT_IN_MMAP_MODE_FOR_OBJECTS_LARGER_THAN,
                     gtk_entry_get_text(
                         GTK_ENTRY(w_copy_output_in_mmap_mode_for_objects_larger_than)));

    setConfigString( FDB_GENERAL, CFG_ATTRIBUTES_TO_AUTO_REA_K,
                     gtk_entry_get_text( GTK_ENTRY(w_attributes_to_auto_rea)));

    setConfigString( FDB_GENERAL, CFG_ATTRIBUTES_ALWAYS_IN_UI_MODEL_K,
                     gtk_entry_get_text( GTK_ENTRY(w_attributes_always_available_in_ui_model)));

    setConfigString( FDB_GENERAL, CFG_FSPOT_POSITIVE_OVERLAY_REGEX_K,
                     gtk_entry_get_text( GTK_ENTRY(w_fspot_positive_overlay_regex_string)));
    
    setConfigString( FDB_GENERAL, CFG_ATTRIBUTES_GNU_DIFF_CMD_FILES_K,
                     gtk_entry_get_text( GTK_ENTRY(w_attributes_gnu_diff_files)));
    
    {
        stringlist_t sl = m_passiveViewList->getStringList();
        string d = Util::createNullSeperatedList( sl );
        cerr << "save data:" << d << endl;
        setConfigString( FDB_GENERAL, CFG_FORCE_PASSIVE_VIEW_K, d );

//         string d2 = getConfigString( FDB_GENERAL, CFG_FORCE_PASSIVE_VIEW_K, CFG_FORCE_PASSIVE_VIEW_DEFAULT );
//         cerr << "loaded again:" << d2 << endl;
    }

    
    {
        stringlist_t sl = m_xsltStylesheetPathList->getStringList();
        string d = Util::createNullSeperatedList( sl );
        setConfigString( FDB_GENERAL, CFG_XSLTFS_STYLESHEET_PATH_K, d );
    }

    
    setConfigString( FDB_GENERAL, CFG_GLOB_SKIP_FILE_URLS_K,
                     tostr(gtk_toggle_button_get_active(
                               GTK_TOGGLE_BUTTON(w_glob_process_for_file_urls_aswell))));
    {
        stringlist_t sl = m_globSkipRegexList->getStringList();
        string d = Util::createNullSeperatedList( sl );
        setConfigString( FDB_GENERAL, CFG_GLOB_SKIP_REGEX_LIST_K, d );
    }

    setConfigString( FDB_GENERAL, CFG_PRECALCULATE_FOR_COPY_K,
                     tostr(gtk_toggle_button_get_active(
                               GTK_TOGGLE_BUTTON(w_copy_precalculate))));

    
    
//     {
//         setConfigString( FDB_GENERAL, "vm-auto-cleanup",
//                       tostr(gtk_toggle_button_get_active(
//                                 GTK_TOGGLE_BUTTON(w_vm_auto_cleanup))));

//         string v = tostr((int)gtk_range_get_value(GTK_RANGE(w_vm_auto_cleanup_maxfreeatonce)));
//         setConfigString( FDB_GENERAL, "vm-auto-cleanup-maxfreeatonce", v );
        
//     }
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
    GtkWidget* w;
    GtkWidget* lab;
    GtkWidget* sw;
    GtkWidget* page;

    gtk_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title ( GTK_WINDOW( gtk_window ), "Ferris general" );
    gtk_window_set_default_size (GTK_WINDOW (gtk_window), 600, 450);
    gtk_signal_connect(GTK_OBJECT (gtk_window), "destroy", gtk_main_quit, NULL);

    int r=0;
    int c=0;
    gtk_notebook = GTK_NOTEBOOK(gtk_notebook_new());
    page = gtk_table_new ( 2, 2, false );


    r=0;
    w = w_copy_use_sendfile_default
        = gtk_check_button_new_with_label(
            "Perfer to Use sendfile(2) above all other copy methods" );
    gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+2, r, r+1 );
    ++r; c=0;

    lab  = gtk_label_new( "Chunk size to copy at once with sendfile(2) in bytes:" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, c, c+1, r, r+1 );
    ++c;
    w = w_copy_sendfile_chunksize = gtk_entry_new_with_max_length(20);
    gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+1, r, r+1 );
    ++r; c=0;

    w = w_copy_input_in_mmap_mode_by_default
        = gtk_check_button_new_with_label(
            "Use memory mapped files for input during copy by default" );
    gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+2, r, r+1 );
    ++r; c=0;

    lab  = gtk_label_new( "Use memory mapped files for for input during copy on files larger than:" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, c, c+1, r, r+1 );
    ++c;
    w = w_copy_input_in_mmap_mode_for_objects_larger_than = gtk_entry_new_with_max_length(20);
    gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+1, r, r+1 );
    ++r; c=0;

    w = w_copy_output_in_mmap_mode_by_default
        = gtk_check_button_new_with_label(
            "Use memory mapped files for output during copy by default" );
    gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+2, r, r+1 );
    ++r; c=0;

    lab  = gtk_label_new( "Use memory mapped files for for output during copy on files larger than:" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, c, c+1, r, r+1 );
    ++c;
    w = w_copy_output_in_mmap_mode_for_objects_larger_than = gtk_entry_new_with_max_length(20);
    gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+1, r, r+1 );
    ++r; c=0;
    
    lab  = gtk_label_new( "" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, c, c+2, r, r+1 );
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET( page ));
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_notebook_append_page( gtk_notebook, sw, gtk_label_new( "memory mapped" ));


    
    
    page = gtk_table_new ( 2, 2, false );
    r=0; c=0;
    lab  = gtk_label_new( "list of attributes which are automatically\n"
                          "added to the recommended-ea when present\n"
                          "(comma seperated: width,height,is-gnome-related)" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, c, c+1, r, r+1 );
    w_attributes_to_auto_rea = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(page), w_attributes_to_auto_rea, c+1, c+2, r, r+1 );

    ++r; c=0;
    lab  = gtk_label_new( "list of attributes which are always\n"
                          "available in GUI displays even when they\n"
                          "have no values\n"
                          "(comma seperated: width,height,is-gnome-related)" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, c, c+1, r, r+1 );
    w_attributes_always_available_in_ui_model = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(page), w_attributes_always_available_in_ui_model, c+1, c+2, r, r+1 );

    ++r; c=0;
    lab  = gtk_label_new( "regex for URLs which should have f-spot metadata overlayed on them\n"
                          "(directories not matching this regex are ignored by the f-spot plugin" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, c, c+1, r, r+1 );
    w_fspot_positive_overlay_regex_string = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(page), w_fspot_positive_overlay_regex_string, c+1, c+2, r, r+1 );

    ++r; c=0;
    lab  = gtk_label_new( "gnu diff command invocation\n"
                          "for generating diffs between two files" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, c, c+1, r, r+1 );
    w_attributes_gnu_diff_files = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(page), w_attributes_gnu_diff_files, c+1, c+2, r, r+1 );
    
    ++r; c=0;
    lab  = gtk_label_new( "" );
    gtk_table_attach_defaults(GTK_TABLE(page), lab, c, c+2, r, r+1 );
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET( page ));
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_notebook_append_page( gtk_notebook, sw, gtk_label_new( "attributes" ));

    m_passiveViewList->setColumnLabel( "regex" );
    m_passiveViewList->setDescriptionLabel( "regexes for URLs that should not be actively viewed" );
    gtk_notebook_append_page( gtk_notebook,
                              m_passiveViewList->getWidget(),
                              gtk_label_new( "active views" ));

    m_xsltStylesheetPathList->setColumnLabel( "paths" );
    m_xsltStylesheetPathList->setDescriptionLabel( "Paths to search for XSLT stylesheets in xsltfs:// evaluation" );
    gtk_notebook_append_page( gtk_notebook,
                              m_xsltStylesheetPathList->getWidget(),
                              gtk_label_new( "xsltfs stylesheets" ));

    
    {
        page = gtk_table_new ( 2, 2, false );

        c=0; r=0;
        w = w_glob_process_for_file_urls_aswell
            = gtk_check_button_new_with_label(
                "Process globs for relative and absolute kernel paths too\n"
                "(Note that bash does this already)" );
        gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+2, r, r+1 );
        ++r; c=0;

        
        m_globSkipRegexList->setColumnLabel( "regular expressions" );
        m_globSkipRegexList->setDescriptionLabel( "URLs which match these regex will not have globs processed" );
        gtk_table_attach_defaults(GTK_TABLE(page),
                                  m_globSkipRegexList->getWidget(),
                                  c, c+2, r, r+1 );

        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET( page ));
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_notebook_append_page( gtk_notebook, sw, gtk_label_new( "globbing" ));
    
    }
    

    {
        page = gtk_table_new ( 2, 2, false );

        c=0; r=0;
        w = w_copy_precalculate
            = gtk_check_button_new_with_label(
                "Calculate size of sources and other metadata\n"
                "before starting a copy operation" );
        gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+2, r, r+1 );
        ++r; c=0;

        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET( page ));
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_notebook_append_page( gtk_notebook, sw, gtk_label_new( "copying" ));
    
    }
    

    
//     {
//         GtkWidget* slide;
//         page = gtk_table_new ( 2, 2, false );

//         r=0; c=0;
//         w = w_vm_auto_cleanup
//             = gtk_check_button_new_with_label(
//                 "Periodically automatically invoke memory cleanup" );
//         gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+2, r, r+1 );
//         ++r; c=0;

//         lab  = gtk_label_new( "Maximum number of unused context objects to free at any one time" );
//         gtk_table_attach_defaults(GTK_TABLE(page), lab, 0, 1, r, r+1 );
//         slide = gtk_hscale_new_with_range ( 10, 500, 50 );
//         gtk_range_set_value ( GTK_RANGE(slide), 500 );
//         gtk_scale_set_value_pos( GTK_SCALE(slide), GTK_POS_RIGHT );
//         w_vm_auto_cleanup_maxfreeatonce = slide;
//         gtk_table_attach_defaults(GTK_TABLE(page), slide, 1, 2, r, r+1 );
//         ++r; c=0;
        
//         sw = gtk_scrolled_window_new(NULL, NULL);
//         gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET( page ));
//         gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
//                                         GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
//         gtk_notebook_append_page( gtk_notebook, sw,
//                                   gtk_label_new( "Memory subsystem" ));
        
//     }
    
    
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

