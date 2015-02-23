/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2003 Ben Martin

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

    $Id: EditSQLColumns.cpp,v 1.3 2010/09/24 21:31:04 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <EditSQLColumns.hh>
#include <GtkFerris.hh>

using namespace std;
using namespace Ferris;

namespace FerrisUI
{
    static const char* const GOBJ_CELLID_K    = "GOBJ_CELLID_K";
    
    void EditSQLColumns_add_cb( GtkButton *button, gpointer user_data )
    {
        EditSQLColumns* esl = (EditSQLColumns*)user_data;
        GtkTreeStore*   w_treemodel = esl->w_treemodel;
        GtkWidget*      w_treeview  = esl->w_treeview;
        
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
    
        GtkTreeIter iter = esl->appendNewBlankItem();

        GtkTreePath* path      = gtk_tree_model_get_path( tm, &iter );
        GtkTreeViewColumn* col = gtk_tree_view_get_column( tv, EditSQLColumns::C_COLUMN_NAME );
        gtk_tree_view_set_cursor( tv, path, col, true );
        gtk_tree_path_free( path );
    }

    void EditSQLColumns_del_cb( GtkButton *button, gpointer user_data )
    {
        EditSQLColumns* esl = (EditSQLColumns*)user_data;
        GtkTreeStore*   w_treemodel = esl->w_treemodel;
        GtkWidget*      w_treeview  = esl->w_treeview;

        list_gtktreeiter_t deliters = getIterList( w_treeview, true );
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
        GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);
        GtkTreeSelection *selection;

        selection = gtk_tree_view_get_selection ( tv );
        gtk_tree_selection_unselect_all( selection );

        for( list_gtktreeiter_t::iterator iter = deliters.begin(); iter != deliters.end(); ++iter )
        {
            gtk_tree_store_remove( ts, &(*iter) );
        }
    }

    void EditSQLColumns_clear_cb( GtkButton *button, gpointer user_data )
    {
        EditSQLColumns* esl = (EditSQLColumns*)user_data;
        esl->clear();
    }
    
    
    void EditSQLColumns_edited_cb ( GtkCellRendererText *cell, gchar *path_string,
                                           gchar *new_text, gpointer user_data )
    {
        EditSQLColumns* esl = (EditSQLColumns*)user_data;
        GtkTreeStore*   w_treemodel = esl->w_treemodel;
        GtkWidget*      w_treeview  = esl->w_treeview;

        gint cidx = GPOINTER_TO_INT(g_object_get_data( G_OBJECT( cell ), GOBJ_CELLID_K ));
        cerr << "EditSQLColumns_edited_cb() cidx:" << cidx << " new_text:" << new_text << endl;
        GtkTreeModel *model = GTK_TREE_MODEL (w_treemodel);
        GtkTreeIter iter;
        GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
        gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_store_set (GTK_TREE_STORE (model), &iter, cidx, new_text, -1);
        gtk_tree_path_free (path);
    }

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    
    EditSQLColumns::EditSQLColumns()
        :
        w_treemodel( 0 ),
        w_treeview( 0 ),
        w_baseWidget( 0 ),
        m_columnNameLabel( "column" ),
        m_columnTypeLabel( "sql type" ),
        m_descriptionLabel( "" )
    {
        for( int i=0; i < C_COLUMN_COUNT; ++i )
            w_cols[ i ] = 0;
    }
    
    EditSQLColumns::~EditSQLColumns()
    {
    }

    void
    EditSQLColumns::setDescriptionLabel( const std::string& s )
    {
        m_descriptionLabel = s;
    }
    
    
    stringmap_t
    EditSQLColumns::getStringMap()
    {
        stringmap_t ret;

        list_gtktreeiter_t giters = getIterList( w_treeview );
        cerr << "giters.sz:" << giters.size() << endl;
    
        for( list_gtktreeiter_t::iterator iter = giters.begin();
             iter != giters.end(); ++iter )
        {
            GtkTreeIter giter = *iter;
            cerr << "sl:" << treestr( &giter, w_treeview, C_COLUMN_NAME ) << endl;
            ret.insert(
                make_pair( 
                    treestr( &giter, w_treeview, C_COLUMN_NAME ),
                    treestr( &giter, w_treeview, C_COLUMN_TYPE )
                    )
                );
        }
        return ret;
    }
    
    void
    EditSQLColumns::setStringMap( const stringmap_t& sm )
    {
        clear();
        
        for( stringmap_t::const_iterator mi = sm.begin(); mi != sm.end(); ++mi )
        {
            GtkTreeIter treeiter;

//             cerr << "setStringMap() first:" << mi->first
//                  << " second:" << mi->second
//                  << endl;
            
            gtk_tree_store_append( w_treemodel, &treeiter, NULL );
            gtk_tree_store_set( w_treemodel, &treeiter,
                                C_COLUMN_NAME, mi->first.c_str(),
                                C_COLUMN_TYPE, mi->second.c_str(),
                                -1 );
        }
    }

    void
    EditSQLColumns::clear()
    {
        if( w_treemodel )
            gtk_tree_store_clear( w_treemodel );
    }

    GtkTreeIter
    EditSQLColumns::appendNewBlankItem()
    {
        GtkTreeIter iter;
        const char* d = "";
        gtk_tree_store_append( w_treemodel, &iter, NULL );
        gtk_tree_store_set( w_treemodel, &iter,
                            EditSQLColumns::C_COLUMN_NAME, "new",
                            EditSQLColumns::C_COLUMN_TYPE, "int",
                            -1 );
        return iter;
    }

    GtkWidget*
    EditSQLColumns::getWidget()
    {
        if( w_baseWidget )
            return w_baseWidget;
        
        GtkWidget* page = w_baseWidget = gtk_table_new ( 1, 1, false );
        
        int colcount = 1;
        int r=0;
        int c=0;
        GtkWidget* b;
        GtkWidget* lab;
        GtkToolbar* tb = GTK_TOOLBAR(gtk_toolbar_new());
        GtkWidget* vbx = GTK_WIDGET(gtk_vbox_new(0,0));
        GtkWidget* hbx = GTK_WIDGET(gtk_hbox_new(0,0));

        if( !m_descriptionLabel.empty() )
        {
            lab  = gtk_label_new( m_descriptionLabel.c_str() );
            gtk_box_pack_start(GTK_BOX(vbx), lab, 0, 0, 0 );
        }
        
        b = gtk_toolbar_append_item( tb, "New",
                                     "create a new string", "",
                                     gtk_image_new_from_stock(
                                         GTK_STOCK_NEW,
                                         GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                     GTK_SIGNAL_FUNC(EditSQLColumns_add_cb),
                                     this );
        b = gtk_toolbar_append_item( tb, "Delete",
                                     "delete selected string(s)", "",
                                     gtk_image_new_from_stock(
                                         GTK_STOCK_DELETE,
                                         GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                     GTK_SIGNAL_FUNC(EditSQLColumns_del_cb),
                                     this );
        b = gtk_toolbar_append_item( tb, "Clear",
                                     "clear all items", "",
                                     gtk_image_new_from_stock(
                                         GTK_STOCK_CLEAR,
                                         GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                     GTK_SIGNAL_FUNC(EditSQLColumns_clear_cb),
                                     this );

        gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( tb ));
        gtk_box_pack_start(GTK_BOX(vbx), hbx, 0, 0, 0 );
        gtk_table_attach_defaults(GTK_TABLE(page), GTK_WIDGET(vbx), c, c+colcount, r, r+1 );
        ++r; c=0;
        
        w_treemodel = gtk_tree_store_new( C_COLUMN_COUNT, G_TYPE_STRING, G_TYPE_STRING );
        w_treeview  = gtk_tree_view_new_with_model(GTK_TREE_MODEL( w_treemodel )); 
//        gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(w_treeview), 1, 1, 0 );
        GtkWidget* sw;
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_container_add( GTK_CONTAINER( sw ), GTK_WIDGET( w_treeview ));
//        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET( vbx ));
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(sw), 1, 1, 0 );
//         gtk_table_attach(GTK_TABLE(page), sw, c, c+colcount, r, r+1,
//                          GtkAttachOptions(GTK_EXPAND | GTK_SHRINK | GTK_FILL),
//                          GtkAttachOptions(GTK_EXPAND | GTK_SHRINK | GTK_FILL),
//                          0, 0 );

        gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (w_treeview), TRUE);
        GObject *selection;
        selection = G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (w_treeview)));
        gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_MULTIPLE);
        gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW (w_treeview), true );
        gtk_tree_view_set_enable_search(GTK_TREE_VIEW (w_treeview), true );
        gtk_tree_view_set_search_column(GTK_TREE_VIEW (w_treeview), C_COLUMN_NAME );

        /* Setup view */
        GtkCellRenderer* ren;
        int col = 0;

        
        col = C_COLUMN_NAME;
        ren = gtk_cell_renderer_text_new ();
        g_object_set_data( G_OBJECT( ren ), GOBJ_CELLID_K, GINT_TO_POINTER(col) );
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "edited",
                               G_CALLBACK (EditSQLColumns_edited_cb),
                               (gpointer)this, 0,
                               GConnectFlags(0));
        w_cols[ col ] = gtk_tree_view_column_new_with_attributes(
            m_columnNameLabel.c_str(), ren,
            "text", col,
            NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ col ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ col ], col );

        
        col = C_COLUMN_TYPE;
        ren = gtk_cell_renderer_text_new ();
        g_object_set_data( G_OBJECT( ren ), GOBJ_CELLID_K, GINT_TO_POINTER(col) );
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "edited",
                               G_CALLBACK (EditSQLColumns_edited_cb),
                               (gpointer)this, 0,
                               GConnectFlags(0));
        w_cols[ col ] = gtk_tree_view_column_new_with_attributes(
            m_columnTypeLabel.c_str(), ren,
            "text", col,
            NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ col ] );
        
        return w_baseWidget;
    }
    
    
};

