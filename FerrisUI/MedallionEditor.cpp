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

    $Id: MedallionEditor.cpp,v 1.9 2010/09/24 21:31:05 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <FerrisUI/MedallionEditor.hh>
#include <Ferris/Medallion_private.hh>
#include <Ferris/ValueRestorer.hh>
#include <GtkFerris.hh>
#include <gtkcellrendererprogress.h>

using namespace std;
using namespace Ferris;

namespace FerrisUI
{
    emblemID_t
    MedallionEditor::getCurrentSelectedEmblemID( GtkTreeIter *iter )
    {
        emblemID_t ret = 0;
        string s = treestr( iter, w_treeview, C_EMBLEM_ID_COLUMN );
        if( !s.empty() )
            ret = toType<emblemID_t>( s );
        return ret;
    }
    
    emblemID_t
    MedallionEditor::getCurrentSelectedEmblemID()
    {
        emblemID_t ret = 0;
    
        list_gtktreeiter_t gl = getIterList( w_treeview, true );
        if( !gl.empty() )
        {
            string s = treestr( &gl.front(), w_treeview, C_EMBLEM_ID_COLUMN );
            if( !s.empty() )
                ret = toType<emblemID_t>( s );
        }
        return ret;
    }

    void
    MedallionEditor::update_all_emblems_in_partial_order_rec( GtkTreeModel* model, GtkTreeIter* iter,
                                                              fh_medallion med, fh_emblem em, emblemID_t eid )
    {
        while( true )
        {
            if( eid == getCurrentSelectedEmblemID( iter ) )
            {
                string name = em->getName();
                string desc = em->getDescription();
                double fuzzy = med->getFuzzyBelief( em );
                bool   isAsserted = med->hasEmblem( em );
                bool   isRetracted = med->hasRetractedEmblem( em );

                gtk_tree_store_set( GTK_TREE_STORE(model), iter,
                                    C_ASSERT_COLUMN,      isAsserted, 
                                    C_RETRACT_COLUMN,     isRetracted, 
                                    C_EMBLEM_DESC_COLUMN, desc.c_str(),
                                    C_EMBLEM_FUZZY_COLUMN, fuzzy,
                                    -1 );
//                gtk_tree_store_set (GTK_TREE_STORE (model), iter, cidx, value, -1);
            }
            
            GtkTreeIter childiter;
            if( gtk_tree_model_iter_children( model, &childiter, iter ) )
            {
                update_all_emblems_in_partial_order_rec( model, &childiter, med, em, eid );
            }
            
            if( !gtk_tree_model_iter_next( model, iter ))
                break;
        }
    }
    
    void
    MedallionEditor::update_all_emblems_in_partial_order( GtkTreeModel* model, fh_medallion med, fh_emblem em )
    {
        // Not gtk_store_set all the GTK tree model rows which represent
        // the same emblem in the partial order.
        GtkTreeIter giter;
        if( gtk_tree_model_get_iter_first( model, &giter ) )
        {
            update_all_emblems_in_partial_order_rec( model, &giter, med, em, em->getID() );
        }
    }
    
    void emblem_assert_toggled_cb( GtkCellRendererToggle *cell,
                                   gchar *path_string,
                                   gpointer user_data)
    {
        MedallionEditor* me = (MedallionEditor*)user_data;
        me->emblem_assert_toggled( cell, path_string );
    }
    
    void
    MedallionEditor::emblem_assert_toggled( GtkCellRendererToggle *cell, gchar *path_string )
    {
        try
        {
            
            int cidx = (int)C_ASSERT_COLUMN;

//            cerr << "MedEditor::emblem_toggled() this:" << toVoid( this ) << endl;
        
            GtkTreeIter iter;
            GtkTreeModel* model = GTK_TREE_MODEL (w_treemodel);
            GtkTreePath*  path  = gtk_tree_path_new_from_string (path_string);
            gboolean value;
        
            gtk_tree_model_get_iter (model, &iter, path);
            gtk_tree_model_get (model, &iter, cidx, &value, -1);
            value = !value;

            // update the medallion to reflect this change
            emblemID_t eid = getCurrentSelectedEmblemID( &iter );
            if( eid )
            {
                Util::ValueRestorer<bool> valrest( m_userMedallionUpdateHandled, true );

                fh_emblem    em  = m_et->getEmblemByID( eid );
                fh_medallion med = m_ctx->getMedallion();

//                 cerr << "MedEditor::emblem_toggled() emid:" << eid
//                      << " emblem:" << em->getName()
//                      << " value:" << value
//                      << endl;

//            med->ensureEmblem( em, value );
                if( value )
                    med->addEmblem( em );
                if( !value )
                    med->removeEmblem( em );

                // set GTK+ view
                gtk_tree_store_set (GTK_TREE_STORE (model), &iter, cidx, value, -1);
                update_all_emblems_in_partial_order( model, med, em );
                med->sync();
//                cerr << "Synced" << endl;
            }
            gtk_tree_path_free (path);
        }
        catch( exception& e )
        {
            stringstream ss;
            ss << "Error:" << e.what() << endl;
            RunErrorDialog( ss.str(), 0 );
        }
    }
    
    void emblem_retract_toggled_cb( GtkCellRendererToggle *cell,
                                   gchar *path_string,
                                   gpointer user_data)
    {
        MedallionEditor* me = (MedallionEditor*)user_data;
        me->emblem_retract_toggled( cell, path_string );
    }

    void
    MedallionEditor::emblem_retract_toggled( GtkCellRendererToggle *cell, gchar *path_string )
    {
        try
        {
            
            int cidx = (int)C_RETRACT_COLUMN;

//            cerr << "MedEditor::emblem_toggled() this:" << toVoid( this ) << endl;
        
            GtkTreeIter iter;
            GtkTreeModel* model = GTK_TREE_MODEL (w_treemodel);
            GtkTreePath*  path  = gtk_tree_path_new_from_string (path_string);
            gboolean value;
        
            gtk_tree_model_get_iter (model, &iter, path);
            gtk_tree_model_get (model, &iter, cidx, &value, -1);
            value = !value;

            // update the medallion to reflect this change
            emblemID_t eid = getCurrentSelectedEmblemID( &iter );
            if( eid )
            {
                Util::ValueRestorer<bool> valrest( m_userMedallionUpdateHandled, true );
                
                fh_emblem    em  = m_et->getEmblemByID( eid );
                fh_medallion med = m_ctx->getMedallion();

//                 cerr << "MedEditor::emblem_toggled() emid:" << eid
//                      << " emblem:" << em->getName()
//                      << " value:" << value
//                      << endl;

//            med->ensureEmblem( em, value );
                if( value )
                    med->retractEmblem( em );
                if( !value )
                    med->removeEmblem( em );

                // set GTK+ view
                gtk_tree_store_set (GTK_TREE_STORE (model), &iter, cidx, value, -1);
                update_all_emblems_in_partial_order( model, med, em );
                med->sync();
            }
            gtk_tree_path_free (path);
        }
        catch( exception& e )
        {
            stringstream ss;
            ss << "Error:" << e.what() << endl;
            RunErrorDialog( ss.str(), 0 );
        }
    }

    
    MedallionEditor::MedallionEditor()
        :
        m_ctx( 0 ),
        m_et( Factory::getEtagere() ),
        m_showDesc( false ),
        m_showFuzzyHints( true ),
        w_treemodel( 0 ),
        w_treeview( 0 ),
        w_baseWidget( 0 ),
        m_userMedallionUpdateHandled( false )
    {
    }
    
    MedallionEditor::~MedallionEditor()
    {
    }
    

    void
    MedallionEditor::setShowDescription( bool v )
    {
        m_showDesc = v;
    }

    void
    MedallionEditor::setShowFuzzyHints( bool v )
    {
        m_showFuzzyHints = v;
    }

    struct TreeExpander
    {
        typedef list< GtkTreePath* > paths_t;
        paths_t paths;
        GtkTreePath* m_focusPath;
        GtkTreeViewColumn* m_focusColumn;
        
        static void rememberExpanded_cb( GtkTreeView *tree_view,
                                         GtkTreePath *path,
                                         gpointer user_data)
            {
                TreeExpander* thisp = (TreeExpander*)user_data;
                thisp->rememberExpanded( path );
            }
        void rememberExpanded( GtkTreePath *path )
            {
                paths.push_back( gtk_tree_path_copy( path ));
            }
        GtkWidget* w_treeview;
        TreeExpander( GtkWidget* w_treeview )
            :
            w_treeview( w_treeview ),
            m_focusPath( 0 ),
            m_focusColumn( 0 )
            {
                gtk_tree_view_get_cursor( GTK_TREE_VIEW( w_treeview ), &m_focusPath, &m_focusColumn );
                gtk_tree_view_map_expanded_rows( GTK_TREE_VIEW( w_treeview ), rememberExpanded_cb, this );
            }
        ~TreeExpander()
            {
                for( paths_t::const_iterator pi = paths.begin(); pi != paths.end(); )
                {
                    GtkTreePath* path = *pi;
                    gtk_tree_view_expand_to_path( GTK_TREE_VIEW( w_treeview ), path );
                    ++pi;
                    gtk_tree_path_free( path );
                }
                gtk_tree_view_set_cursor( GTK_TREE_VIEW( w_treeview ), m_focusPath, m_focusColumn, false );
                gtk_tree_path_free( m_focusPath );
            }
    };
    


    
    void
    MedallionEditor::setContext( fh_context c )
    {
        Time::Benchmark bm1("+++setContext(1)");
        TreeExpander treeexpander( w_treeview );
        clear();

        m_ctxConnection.disconnect();
        m_ctx = c;
        m_ctxConnection = m_ctx->getNamingEvent_MedallionUpdated_Sig()
            .connect( sigc::mem_fun( *this, &_Self::OnMedallionUpdated ));
        
        Time::Benchmark bm2("+++setContext(2)");
        update_model();
    }
    
    fh_context
    MedallionEditor::getContext()
    {
        return m_ctx;
    }

    void
    MedallionEditor::OnMedallionUpdated( fh_context c )
    {
        cerr << "MedallionEditor::OnMedallionUpdated(1)"
             << " c:" << c->getURL()
             << " m_ctx:" << m_ctx->getURL()
             << endl;

        if( m_userMedallionUpdateHandled )
            return;

        if( c == m_ctx && w_baseWidget )
        {
            TreeExpander treeexpander( w_treeview );
            clear();
            update_model();
        }
    }
    
    void
    MedallionEditor::clear()
    {
        if( w_treemodel )
            gtk_tree_store_clear( w_treemodel );
    }

    
    
    void
    MedallionEditor::update_model( fh_medallion& med, GtkTreeIter* piter, fh_emblem em )
    {
        Util::ValueRestorer<bool> valrest( m_userMedallionUpdateHandled, true );

        string name = em->getName();
        string icon = em->getIconName();
        string desc = em->getDescription();
        double fuzzy = med->getFuzzyBelief( em );
        bool   isAsserted = med->hasEmblem( em );
        bool   isRetracted = med->hasRetractedEmblem( em );

        GtkTreeIter treeiter;
        gtk_tree_store_append( w_treemodel, &treeiter, piter );
        gtk_tree_store_set( w_treemodel, &treeiter,
                            C_ASSERT_COLUMN,      isAsserted, 
                            C_RETRACT_COLUMN,     isRetracted, 
                            C_EMBLEM_ID_COLUMN,   Ferris::tostr(em->getID()).c_str(),
                            C_EMBLEM_NAME_COLUMN, name.c_str(),
                            C_EMBLEM_ICON_COLUMN, getEmblemListViewPixbuf( em ),
                            C_EMBLEM_DESC_COLUMN, desc.c_str(),
                            C_EMBLEM_FUZZY_COLUMN, fuzzy,
                            -1 );

        emblems_t el = em->getChildren();
        if( !el.empty() )
        {
//            cerr << "MedallionEditor::update_model(rec) el.sz:" << el.size() << endl;

            fh_emblem de = getDummyTreeModelEmblem();
            update_model( med, &treeiter, de );
        }
        
//         emblems_t::iterator _end = el.end();
//         for( emblems_t::iterator ei = el.begin(); ei != _end; ++ei )
//         {
//             fh_emblem child = *ei;
//             update_model( med, &treeiter, child );
//         }
    }


    void on_gtk_row_expanded_cb ( GtkTreeView *treeview,
                                  GtkTreeIter *arg1,
                                  GtkTreePath *arg2,
                                  gpointer     user_data)
    {
        MedallionEditor* me = (MedallionEditor*)user_data;
        me->on_gtk_row_expanded( treeview, arg1, arg2 );
    }
    void MedallionEditor::on_gtk_row_expanded ( GtkTreeView *treeview,
                                                GtkTreeIter *piter,
                                                GtkTreePath *ppath )
    {
//        cerr << "on_gtk_row_expanded()" << endl;

        GtkTreeIter child_iter;
        gboolean rc = gtk_tree_model_iter_nth_child( GTK_TREE_MODEL(w_treemodel),
                                                     &child_iter,
                                                     piter,
                                                     0 );
        
        emblemID_t first_child_eid = getCurrentSelectedEmblemID( &child_iter );
        fh_emblem   de = getDummyTreeModelEmblem();

        if( first_child_eid == de->getID() )
        {
            fh_medallion med = m_ctx->getMedallion();

            emblemID_t eid = getCurrentSelectedEmblemID( piter );
            fh_emblem  pem = m_et->getEmblemByID( eid );
            emblems_t   el = pem->getChildren();

            emblems_t::iterator _end = el.end();
            for( emblems_t::iterator ei = el.begin(); ei != _end; ++ei )
            {
                fh_emblem child = *ei;
                update_model( med, piter, child );
            }
            gtk_tree_store_remove( GTK_TREE_STORE(w_treemodel), &child_iter );
        }
    }
    
    
    void
    MedallionEditor::update_model()
    {
        if( !m_ctx )
            return;
        if( !w_baseWidget )
            return;
        
        
        emblems_t    all = m_et->getAllEmblems();
        fh_medallion med = m_ctx->getMedallion();

        for( emblems_t::const_iterator ei = all.begin(); ei!=all.end(); ++ei )
        {
            fh_emblem em = *ei;
//            cerr << "MedallionEditor::update_model(em iter) em:" << em->getID() << endl;

            if( em->getParents().empty() )
            {
                update_model( med, NULL, em );
            }
        }
    }

    void
    MedallionEditor::OnEmblemCreated( fh_etagere, fh_emblem em )
    {
        fh_medallion med = m_ctx->getMedallion();

        TreeExpander treeexpander( w_treeview );
        clear();
        update_model();
    }
    
    
    

    GtkWidget*
    MedallionEditor::getWidget()
    {
        if( w_baseWidget )
            return w_baseWidget;

        GtkWidget* page = w_baseWidget = gtk_table_new ( 1, 1, false );
        int colcount = 1;
        int r=0;
        int c=0;
        GtkWidget* b;
        GtkWidget* lab;

        w_treemodel = gtk_tree_store_new( C_COLUMN_COUNT,
                                          G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
                                          G_TYPE_STRING, G_TYPE_STRING,
                                          GDK_TYPE_PIXBUF, G_TYPE_STRING,
                                          G_TYPE_DOUBLE );
        
        w_treeview  = gtk_tree_view_new_with_model(GTK_TREE_MODEL( w_treemodel ));
        g_signal_connect_data( G_OBJECT( w_treeview ), "row-expanded",
                               G_CALLBACK (on_gtk_row_expanded_cb), (gpointer)this,
                               0, GConnectFlags(0));
        
        GtkWidget* sw;
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_container_add( GTK_CONTAINER( sw ), GTK_WIDGET( w_treeview ));
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
//        gtk_box_pack_start(GTK_BOX(page), GTK_WIDGET(sw), 1, 1, 0 );
        gtk_table_attach_defaults(GTK_TABLE(page), sw, c+0, c+1, r, r+1 );
        

        gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (w_treeview), TRUE);
        GObject *selection;
        selection = G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (w_treeview)));
        gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_SINGLE);
        gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW (w_treeview), true );
        gtk_tree_view_set_enable_search(GTK_TREE_VIEW (w_treeview), true );
        gtk_tree_view_set_search_column(GTK_TREE_VIEW (w_treeview), C_EMBLEM_NAME_COLUMN );


        /* Setup view */
        GtkCellRenderer* ren;
        int colnum = 0;
        
        colnum = C_ASSERT_COLUMN;
        ren = gtk_cell_renderer_toggle_new();
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "toggled",
                               G_CALLBACK (emblem_assert_toggled_cb), (gpointer)this,
                               0, GConnectFlags(0));

        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "Y", ren,
                                                                     "active", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );


        colnum = C_RETRACT_COLUMN;
        ren = gtk_cell_renderer_toggle_new();
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "toggled",
                               G_CALLBACK (emblem_retract_toggled_cb), (gpointer)this,
                               0, GConnectFlags(0));
        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "N", ren,
                                                                     "active", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );
        

        colnum = C_EMBLEM_ID_COLUMN;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 0, 0 );
        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "id", ren,
                                                                     "text", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ),  w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );


        colnum = C_EMBLEM_ICON_COLUMN;
        ren = gtk_cell_renderer_pixbuf_new();
        g_object_set (G_OBJECT (ren),
                      "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE,
                      NULL);
        g_signal_connect_data( G_OBJECT( ren ), "toggled",
                               G_CALLBACK (emblem_assert_toggled_cb), this,
                               0, GConnectFlags(0));
        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "icon", ren,
                                                                     "pixbuf", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ),  w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );

        if( m_showFuzzyHints )
        {
            colnum = C_EMBLEM_FUZZY_COLUMN;
            ren = gtk_cell_renderer_progress_new();
            g_object_set(ren, "editable", 0, 0 );
            w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes(
                "belief",     ren,
                "percentage", colnum,
                NULL);
            gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ),  w_cols[ colnum ] );
            gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );
        }
        
        colnum = C_EMBLEM_NAME_COLUMN;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 0, 0 );
        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "emblem", ren,
                                                                     "text", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ),  w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );
        gtk_tree_sortable_set_sort_column_id( GTK_TREE_SORTABLE( w_treemodel ),
                                              colnum, GTK_SORT_ASCENDING );

        
        if( m_showDesc )
        {
            colnum = C_EMBLEM_DESC_COLUMN;
            ren = gtk_cell_renderer_text_new ();
            g_object_set(ren, "editable", 0, 0 );
            w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "description", ren,
                                                                         "text", colnum,
                                                                         NULL);
            gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ),  w_cols[ colnum ] );
            gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );
        }

        update_model();
        m_et->getEmblemCreated_Sig().connect( sigc::mem_fun( *this, &_Self::OnEmblemCreated ));

        
        return w_baseWidget;
    }
    
        
    
    
};
