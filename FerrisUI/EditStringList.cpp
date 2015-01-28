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

    $Id: EditStringList.cpp,v 1.2 2010/09/24 21:31:04 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <EditStringList.hh>
#include <GtkFerris.hh>

using namespace std;
using namespace Ferris;

namespace FerrisUI
{
    void EditStringList_add_cb( GtkButton *button, gpointer user_data )
    {
        EditStringList* esl = (EditStringList*)user_data;
        GtkTreeStore*   w_treemodel = esl->w_treemodel;
        GtkWidget*      w_treeview  = esl->w_treeview;
        
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
    
        GtkTreeIter iter = esl->appendNewBlankItem();

        GtkTreePath* path      = gtk_tree_model_get_path( tm, &iter );
        GtkTreeViewColumn* col = gtk_tree_view_get_column( tv, EditStringList::C_STRING_COLUMN );
        gtk_tree_view_set_cursor( tv, path, col, true );
        gtk_tree_path_free( path );
    }

    void EditStringList_del_cb( GtkButton *button, gpointer user_data )
    {
        EditStringList* esl = (EditStringList*)user_data;
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

    void EditStringList_clear_cb( GtkButton *button, gpointer user_data )
    {
        EditStringList* esl = (EditStringList*)user_data;
        esl->clear();
    }
    
    
    void EditStringList_edited_cb (GtkCellRendererText *cell, gchar *path_string,
                                   gchar *new_text, gpointer user_data)
    {
        EditStringList* esl = (EditStringList*)user_data;
        GtkTreeStore*   w_treemodel = esl->w_treemodel;
        GtkWidget*      w_treeview  = esl->w_treeview;
        
        int cidx = (int)EditStringList::C_STRING_COLUMN;
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

    
    EditStringList::EditStringList()
        :
        w_treemodel( 0 ),
        w_treeview( 0 ),
        w_baseWidget( 0 ),
        m_columnLabel( "string" ),
        m_descriptionLabel( "" )
    {
        for( int i=0; i < C_COLUMN_COUNT; ++i )
            w_cols[ i ] = 0;
    }
    
    EditStringList::~EditStringList()
    {
    }

    void
    EditStringList::setDescriptionLabel( const std::string& s )
    {
        m_descriptionLabel = s;
    }
    
    
    void
    EditStringList::setColumnLabel( const std::string& s )
    {
        m_columnLabel = s;
    }
    
    stringlist_t
    EditStringList::getStringList()
    {
        stringlist_t sl;
        list_gtktreeiter_t giters = getIterList( w_treeview );
        cerr << "giters.sz:" << giters.size() << endl;
    
        for( list_gtktreeiter_t::iterator iter = giters.begin(); iter != giters.end(); ++iter )
        {
            GtkTreeIter giter = *iter;
            cerr << "sl:" << treestr( &giter, w_treeview, C_STRING_COLUMN ) << endl;
            sl.push_back( treestr( &giter, w_treeview, C_STRING_COLUMN ) );
        }
        return sl;
    }
    
    void
    EditStringList::setStringList( const stringlist_t& sl )
    {
        clear();
        
        for( stringlist_t::const_iterator si = sl.begin(); si != sl.end(); ++si )
        {
            GtkTreeIter treeiter;
            gtk_tree_store_append( w_treemodel, &treeiter, NULL );
            gtk_tree_store_set( w_treemodel, &treeiter,
                                C_STRING_COLUMN, si->c_str(),
                                -1 );
        }
    }

    void
    EditStringList::clear()
    {
        if( w_treemodel )
            gtk_tree_store_clear( w_treemodel );
    }

    GtkTreeIter
    EditStringList::appendNewBlankItem()
    {
        GtkTreeIter iter;
        const char* d = "";
        gtk_tree_store_append( w_treemodel, &iter, NULL );
        gtk_tree_store_set( w_treemodel, &iter,
                            EditStringList::C_STRING_COLUMN, d,
                            -1 );
        return iter;
    }

    GtkWidget*
    EditStringList::getWidget()
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
                                     GTK_SIGNAL_FUNC(EditStringList_add_cb),
                                     this );
        b = gtk_toolbar_append_item( tb, "Delete",
                                     "delete selected string(s)", "",
                                     gtk_image_new_from_stock(
                                         GTK_STOCK_DELETE,
                                         GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                     GTK_SIGNAL_FUNC(EditStringList_del_cb),
                                     this );
        b = gtk_toolbar_append_item( tb, "Clear",
                                     "clear all items", "",
                                     gtk_image_new_from_stock(
                                         GTK_STOCK_CLEAR,
                                         GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                     GTK_SIGNAL_FUNC(EditStringList_clear_cb),
                                     this );

        gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( tb ));
        gtk_box_pack_start(GTK_BOX(vbx), hbx, 0, 0, 0 );
        gtk_table_attach_defaults(GTK_TABLE(page), GTK_WIDGET(vbx), c, c+colcount, r, r+1 );
        ++r; c=0;
        
        w_treemodel = gtk_tree_store_new( C_COLUMN_COUNT, G_TYPE_STRING );
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
        gtk_tree_view_set_search_column(GTK_TREE_VIEW (w_treeview), C_STRING_COLUMN );

        /* Setup view */
        GtkCellRenderer* ren;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "edited",
                               G_CALLBACK (EditStringList_edited_cb),
                               (gpointer)this, 0,
                               GConnectFlags(0));
        
        w_cols[ C_STRING_COLUMN ] =
            gtk_tree_view_column_new_with_attributes( m_columnLabel.c_str(), ren,
                                                      "text", C_STRING_COLUMN,
                                                      NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ),
                                     w_cols[ C_STRING_COLUMN ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ C_STRING_COLUMN ],
                                                 C_STRING_COLUMN );

        return w_baseWidget;
    }
    
    
};

