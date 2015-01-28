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

    $Id: GtkFerris.cpp,v 1.14 2010/09/24 21:31:05 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/


#include <GtkFerris.hh>
#include <gtkferriscellrenderertext.hh>
#include <gtkferristreestore.hh>

#include <General.hh>
#include <FilteredContext.hh>
#include <Resolver_private.hh>
#include <Medallion.hh>
#include <ValueRestorer.hh>

#include <unistd.h>
#include <glib.h>
#include <gtk/gtk.h>

using namespace std;
using namespace Ferris;


namespace FerrisUI
{
    void forea_destroy( GtkWidget *widget, gpointer data )
    {
        gtk_container_remove( GTK_CONTAINER(data), widget );
        gtk_widget_destroy( widget );
    }

    std::string tostr( GtkMenuItem* menu_item )
    {
        if (GTK_BIN (menu_item)->child)
        {
            GtkWidget *child = GTK_BIN (menu_item)->child;

            /* do stuff with child */
            if (GTK_IS_LABEL (child))
            {
                gchar *text;
                gtk_label_get (GTK_LABEL (child), &text);
                return text;
            }
        }
        return "";
    }

    std::string tostr( GtkEntry* e )
    {
        const char* c = gtk_entry_get_text( e );
        if( !c )
            return "";
        return c;
    }
    
    std::string tostr( GtkTextView* tv )
    {
        GtkTextIter startiter;
        GtkTextIter enditer;
        GtkTextBuffer* buffy = gtk_text_view_get_buffer( tv );
            
        gtk_text_buffer_get_start_iter( buffy, &startiter );
        gtk_text_buffer_get_end_iter(   buffy, &enditer );

        gchar* cstr = gtk_text_buffer_get_text( buffy, &startiter, &enditer, false );
        string ret = cstr;
        free(cstr);
        return ret;
    }


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    static
    void collect_cb_fe( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer udata)
    {
        list_gtktreeiter_t* x = (list_gtktreeiter_t*)udata;
        x->push_back( *iter );
    }

    static
    gboolean collectall_cb_fe( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer udata)
    {
        list_gtktreeiter_t* x = (list_gtktreeiter_t*)udata;
        x->push_back( *iter );
        return 0;
    }
    
    list_gtktreeiter_t getIterList( GtkWidget* w_treeview, bool useSelection )
    {
        GtkTreeModel* w_treemodel = gtk_tree_view_get_model( GTK_TREE_VIEW(w_treeview) );
        list_gtktreeiter_t giters;
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
        GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);
        GtkTreeSelection *selection;

        if( useSelection )
        {
            selection = gtk_tree_view_get_selection ( tv );
            gtk_tree_selection_selected_foreach(
                selection, GtkTreeSelectionForeachFunc(collect_cb_fe), &giters );
        }
        else
        {
            gtk_tree_model_foreach( w_treemodel,
                                    GtkTreeModelForeachFunc(collectall_cb_fe),
                                    &giters );
        }
        return giters;
    }

    std::string
    treestr( GtkTreeIter  iter, GtkWidget* w_treeview, int col )
    {
        return treestr( &iter, w_treeview, col );
    }
    
    
    string
    treestr( GtkTreeIter *iter, GtkWidget* w_treeview, int col )
    {
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* w_treemodel = gtk_tree_view_get_model( GTK_TREE_VIEW(w_treeview) );
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
        GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);

        GValue value;
        memset( &value, 0, sizeof(value));
        gtk_tree_model_get_value( tm, iter, col, &value );
        const char* cstr = g_value_get_string(&value);
        string ret = cstr ? cstr : "";
        g_value_unset ( &value );

        return ret;
    }

    string
    treestr( void* tm_void, GtkTreeIter *iter, int col )
    {
        GtkTreeModel* tm = GTK_TREE_MODEL(tm_void);

        GValue value;
        memset( &value, 0, sizeof(value));
        gtk_tree_model_get_value( tm, iter, col, &value );
        const char* ret_CSTR = g_value_get_string(&value);
        string ret = "";
        if( ret_CSTR )
            ret = ret_CSTR;
        g_value_unset ( &value );

        return ret;
    }
    bool
    treebool( void* tm_void, GtkTreeIter *iter, int col )
    {
        GtkTreeModel* tm = GTK_TREE_MODEL(tm_void);

        GValue value;
        memset( &value, 0, sizeof(value));
        gtk_tree_model_get_value( tm, iter, col, &value );
        bool ret = g_value_get_boolean(&value);
        g_value_unset ( &value );
        return ret;
    }
    long
    treeint( void* tm_void, GtkTreeIter *iter, int col )
    {
        GtkTreeModel* tm = GTK_TREE_MODEL(tm_void);

        GValue value;
        memset( &value, 0, sizeof(value));
        gtk_tree_model_get_value( tm, iter, col, &value );
        long ret = g_value_get_int(&value);
        g_value_unset ( &value );
        return ret;
    }
    

    int
    treeint( GtkTreeIter *iter, GtkWidget* w_treeview, int col )
    {
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* w_treemodel = gtk_tree_view_get_model( GTK_TREE_VIEW(w_treeview) );
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
        GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);

        GValue value;
        memset( &value, 0, sizeof(value));
        gtk_tree_model_get_value( tm, iter, col, &value );
        int ret = g_value_get_int(&value);
        g_value_unset ( &value );

        return ret;
    }

    bool
    treebool( GtkTreeIter *iter, GtkWidget* w_treeview, int col )
    {
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* w_treemodel = gtk_tree_view_get_model( GTK_TREE_VIEW(w_treeview) );
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
        GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);

        GValue value;
        memset( &value, 0, sizeof(value));
        gtk_tree_model_get_value( tm, iter, col, &value );
        bool ret = g_value_get_boolean(&value);
        g_value_unset ( &value );

        return ret;
    }
    
    gpointer
    treeptr( GtkTreeIter *iter, GtkWidget* w_treeview, int col )
    {
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* w_treemodel = gtk_tree_view_get_model( GTK_TREE_VIEW(w_treeview) );
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
        GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);

        GValue value;
        memset( &value, 0, sizeof(value));
        gtk_tree_model_get_value( tm, iter, col, &value );
        gpointer ret = g_value_get_pointer(&value);
        g_value_unset ( &value );

        return ret;
    }

    void clearSelection( GtkTreeView* w_treeview )
    {
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* w_treemodel = gtk_tree_view_get_model( GTK_TREE_VIEW(w_treeview) );
        GtkTreeSelection *selection = gtk_tree_view_get_selection ( tv );
        gtk_tree_selection_unselect_all( selection );
    }
    
    

    bool tryToEnsureSomethingIsSelected( GtkWidget* tree, GdkEvent *event )
    {
        list_gtktreeiter_t giters = getIterList( tree, true );
        if( giters.empty() )
        {
            if (event->type == GDK_BUTTON_PRESS)
            {
                GdkEventButton *evbutton = (GdkEventButton *) event;
            
                GtkTreeIter iter;
                GtkTreePath *path = 0;
                bool exists = gtk_tree_view_get_path_at_pos (
                    GTK_TREE_VIEW(tree),
                    static_cast<gint>(evbutton->x),
                    static_cast<gint>(evbutton->y),
                    &path, 0, 0, 0 );
                if( !exists )
                    return false;
                
                gtk_tree_view_set_cursor( GTK_TREE_VIEW(tree), path, 0, 0 );
                gtk_tree_path_free (path);
            }
        }
        return true;
    }

    static void
    getEmblemListViewPixbuf_AddEmblem( fh_emblem em, map< fh_emblem, GdkPixbuf* >& cache )
    {
        static const int desired_width  = 24;
        static const int desired_height = 24;
        static const GdkInterpType interp_type = GDK_INTERP_HYPER;

        static string unknown_iconname = resolveToIconPath( "icons://unknown.png" );
        static GdkPixbuf* unknown_pb = 0;
        if( !unknown_pb )
        {
            GdkPixbuf* pb = gdk_pixbuf_new_from_file( unknown_iconname.c_str(), 0 );
            GdkPixbuf* scaled = gdk_pixbuf_scale_simple( pb,
                                                         desired_width,
                                                         desired_height,
                                                         interp_type );
            gdk_pixbuf_unref( pb );
            unknown_pb = scaled;
        }
        
        
        cache[ em ] = 0;
        string iconname = resolveToIconPath( em->getIconName() );
        if( !iconname.empty() )
        {
            if( iconname == unknown_iconname )
            {
                cache[ em ] = unknown_pb;
            }
            else
            {
                LG_GTKFERRIS_D << "creating pixbuf for emblem icon:" << iconname << endl;
                GdkPixbuf* pb = gdk_pixbuf_new_from_file( iconname.c_str(), 0 );
                GdkPixbuf* scaled = gdk_pixbuf_scale_simple( pb,
                                                             desired_width,
                                                             desired_height,
                                                             interp_type );
                cache[ em ] = scaled;
                gdk_pixbuf_unref( pb );
            }
        }
        
    }
    
    static void
    getEmblemListViewPixbuf_Update( fh_etagere et,
                                    fh_emblem em,
                                    map< fh_emblem, GdkPixbuf* >* cache )
    {
        getEmblemListViewPixbuf_AddEmblem( em, *cache );
    }
    
    GdkPixbuf* getEmblemListViewPixbuf( fh_emblem em )
    {
        typedef map< fh_emblem, GdkPixbuf* > emblemPixbufs_t;
        static emblemPixbufs_t emblemPixbufs;

        if( emblemPixbufs.empty() )
        {
//            cerr << "getEmblemListViewPixbuf() adding all icons to cache" << endl;
            fh_etagere et = Factory::getEtagere();
            emblems_t  el = et->getAllEmblems();
            for( emblems_t::iterator ei = el.begin(); ei != el.end(); ++ei )
            {
                fh_emblem em = *ei;
                getEmblemListViewPixbuf_AddEmblem( em, emblemPixbufs );
            }
        }

        fh_etagere et = Factory::getEtagere();
        et->getEmblemCreated_Sig().connect(
            bind(
                sigc::ptr_fun( getEmblemListViewPixbuf_Update ), &emblemPixbufs ));
        
        return emblemPixbufs[ em ];
    }
    

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    void showPopup( GtkMenu *menu )
    {
        gtk_container_foreach( GTK_CONTAINER(menu), 
                               GtkCallback(gtk_widget_show_all),
                               0 );
        gtk_menu_popup (menu, NULL, NULL, NULL, NULL, 0, 0 );
    }


    void togButtonSet( GtkWidget* w, const std::string& s )
    {
        return togButtonSet( w, isTrue( s ));
    }

    void togButtonSet( GtkWidget* w, bool v )
    {
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( w ), v );
    }

    
//     static void qdia_clicked( GtkDialog *dialog, gint response_id, gpointer ud )
//     {
//         gint32* userdata = (guint32*)ud;
//         *userdata = response_id;
//     }

    void RunInfoDialog( const std::string& msg, GtkWidget* win )
    {
            GtkWidget* d = gtk_message_dialog_new
                ( GTK_WINDOW(win),
                  GTK_DIALOG_MODAL,
                  GTK_MESSAGE_INFO,
                  GTK_BUTTONS_OK,
                  msg.c_str(),
                  0 );

            gtk_widget_show_all( d );
            gtk_dialog_run (GTK_DIALOG (d));
            gtk_widget_destroy( d );
    }
    
    
    bool RunQuestionDialog( const std::string& msg, GtkWidget* win )
        {
            gint32 userdata = 0;
            
            GtkWidget* d = gtk_message_dialog_new
                ( GTK_WINDOW(win),
                  GTK_DIALOG_MODAL,
                  GTK_MESSAGE_QUESTION,
                  GTK_BUTTONS_YES_NO,
                  msg.c_str(),
                  0 );
            
//             g_signal_connect_data( G_OBJECT( d ),
//                                    "response",
//                                    G_CALLBACK (qdia_clicked),
//                                    &userdata,
//                                    0, GConnectFlags(0));

            gtk_widget_show_all( d );

            gint result = gtk_dialog_run (GTK_DIALOG (d));
            switch (result)
            {
            case GTK_RESPONSE_ACCEPT:
            case GTK_RESPONSE_OK:
            case GTK_RESPONSE_YES:
            case GTK_RESPONSE_APPLY:
                userdata = 1;
                break;

            case GTK_RESPONSE_REJECT:
            case GTK_RESPONSE_DELETE_EVENT:
            case GTK_RESPONSE_CANCEL:
            case GTK_RESPONSE_CLOSE:
            case GTK_RESPONSE_NO:
                userdata = 0;
                break;
            }
            
//            Ferris::Main::mainLoop();
            gtk_widget_destroy( d );
            return userdata;
        }

    void RunErrorDialog( const std::string& msg, GtkWidget* win )
    {
            GtkWidget* d = gtk_message_dialog_new
                ( GTK_WINDOW(win),
                  GTK_DIALOG_MODAL,
                  GTK_MESSAGE_ERROR,
                  GTK_BUTTONS_OK,
                  msg.c_str(),
                  0 );

            gtk_widget_show_all( d );
            gtk_dialog_run (GTK_DIALOG (d));
            gtk_widget_destroy( d );
    }

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    void dialog_yes( GtkButton *button, gpointer udata )
    {
        FileCompareWindow* g = (FileCompareWindow*)udata;
        g->dialog_yes( button );
    }

    void dialog_auto_yes( GtkButton *button, gpointer udata )
    {
        FileCompareWindow* g = (FileCompareWindow*)udata;
        g->dialog_auto_yes( button );
    }
    void dialog_auto_no( GtkButton *button, gpointer udata )
    {
        FileCompareWindow* g = (FileCompareWindow*)udata;
        g->dialog_auto_no( button );
    }

    void dialog_no( GtkButton *button, gpointer udata )
    {
        FileCompareWindow* g = (FileCompareWindow*)udata;
        g->dialog_no( button );
    }

    void dialog_auto_toggled(GtkCellRendererToggle *cell, gchar *path_string, gpointer udata)
    {
        FileCompareWindow* g = (FileCompareWindow*)udata;
        g->dialog_auto_toggled( cell, path_string );
    }

    FileCompareWindow::MatchersList_t&
    FileCompareWindow::getAlwaysPermitMatchers()
    {
        return AlwaysPermitMatchers;
    }
    
    FileCompareWindow::MatchersList_t&
    FileCompareWindow::getAlwaysDenyMatchers()
    {
        return AlwaysDenyMatchers;
    }
    
    
    FileCompareWindow::DisplayEAList_t
    FileCompareWindow::getEAList()
    {
        DisplayEAList_t ret;
        ret.push_back( "url" );
        ret.push_back( "size-human-readable" );
        ret.push_back( "dontfollow-size-human-readable" );
        ret.push_back( "mtime-display" );
        ret.push_back( "ctime-display" );
        ret.push_back( "atime-display" );
        ret.push_back( "user-owner-name" );
        ret.push_back( "group-owner-name" );
        ret.push_back( "readable" );
        ret.push_back( "writable" );
        ret.push_back( "runable" );
        ret.push_back( "is-image-object" );
        ret.push_back( "is-animation-object" );
        ret.push_back( "is-audio-object" );
        ret.push_back( "is-source-object" );
        ret.push_back( "protection-ls" );
        ret.push_back( "realpath" );
        ret.push_back( "filesystem-filetype" );
        ret.push_back( "dontfollow-filesystem-filetype" );
        ret.push_back( "mimetype" );
        ret.push_back( "filetype" );
        return ret;
    }
        
    FileCompareWindow::MatchData_t
    FileCompareWindow::getMatchingDests()
    {
        GtkTreeIter giter;
        GtkTreeModel* model = GTK_TREE_MODEL( m_rptmodel );
        MatchData_t MatchData;

        if (!gtk_tree_model_get_iter_first (model, &giter))
        {
            cerr << "error getting list data" << endl;
            return MatchData;
        }

        while( true )
        {
            char* desc;
            char* dst;
            int   dst_ticked;
                
            gtk_tree_model_get( model, &giter,
                                RPT_DESC, &desc,
                                RPT_TARGET, &dst,
                                RPT_TARGETAUTO, &dst_ticked,
                                -1);

            if( dst_ticked )
            {
//                cerr << "adding desc:" << desc << " dst:" << dst << " to matchers" << endl;
                MatchData.push_back( make_pair( string(desc), string(dst) ));
            }
                
            if(!(gtk_tree_model_iter_next (model, &giter)))
                break;
        }

        return MatchData;
    }


    void
    FileCompareWindow::addMatchersTo( MatchersList_t& matchersList )
    {
        const MatchData_t& endlist = getMatchingDests();
        if( !endlist.empty() )
        {
            fh_matcher m = ::Ferris::Factory::ComposeEqualsMatcher( endlist );
            matchersList.push_back( m );
        }
    }

    bool
    FileCompareWindow::isContextInList( MatchersList_t& matchersList, fh_context c )
    {
//        cerr << "matchersList.size():" << matchersList.size() << " c.url:" << c->getURL() << endl;
        
        if( !matchersList.empty() )
        {
            for( MatchersList_t::iterator iter = matchersList.begin();
                 iter != matchersList.end(); ++iter )
            {
                fh_matcher m = *iter;
                if( m( c ) )
                {
                    return true;
                }
            }
        }
        return false;
    }
        
    void
    FileCompareWindow::dialog_yes( GtkButton *button )
    {
        m_result  = true;
        m_looping = false;
    }
        
    void
    FileCompareWindow::dialog_auto_yes( GtkButton *button )
    {
        addMatchersTo( AlwaysPermitMatchers );
        m_result  = true;
        m_looping = false;
    }

    void
    FileCompareWindow::dialog_auto_no( GtkButton *button )
    {
        addMatchersTo( AlwaysDenyMatchers );
        m_result  = false;
        m_looping = false;
    }
    
    void
    FileCompareWindow::dialog_no( GtkButton *button )
    {
        m_result  = false;
        m_looping = false;
    }
        
    void
    FileCompareWindow::dialog_auto_toggled(GtkCellRendererToggle *cell, gchar *path_string )
    {
        GtkTreeModel* model = GTK_TREE_MODEL( m_rptmodel );
        GtkTreeIter iter;
        GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
        gboolean value;
        
        gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_model_get (model, &iter, RPT_TARGETAUTO, &value, -1);
        
        value = !value;
        gtk_tree_store_set (GTK_TREE_STORE (model), &iter, RPT_TARGETAUTO, value, -1);
        
        gtk_tree_path_free (path);
    }


    void
    FileCompareWindow::populateList( fh_context src, fh_context dst )
    {
        DisplayEAList_t DisplayEAList = getEAList();
        for( DisplayEAList_t::iterator iter = DisplayEAList.begin();
             iter != DisplayEAList.end(); ++iter )
        {
            fh_context target = src;
            GtkTreeIter giter;
            string s = "";
            string d = "";
                    
            if( isBound( dst ) )
            {
                s = getStrAttr( src, *iter, "" );
                d = getStrAttr( dst, *iter, "" );
            }
            else
            {
                GtkTreeViewColumn* col = 0;
                
                col = gtk_tree_view_get_column( GTK_TREE_VIEW(m_rptview), RPT_TARGET );
                gtk_tree_view_column_set_title( col, "target" );
                col = gtk_tree_view_get_column( GTK_TREE_VIEW(m_rptview), RPT_SRC );
                gtk_tree_view_column_set_visible( col, false );
                
                d = getStrAttr( target, *iter, "" );
            }
                    
            gtk_tree_store_append( m_rptmodel, &giter, 0 );
            gtk_tree_store_set( m_rptmodel, &giter,
                                RPT_DESC, iter->c_str(),
                                RPT_SRC,  s.c_str(),
                                RPT_DST,  d.c_str(),
                                RPT_DSTAUTO, 0,
                                -1 );
        }
    }
        

    void
    FileCompareWindow::createMainWindow( const std::string& title )
    {
        m_win = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
        gtk_window_set_title( GTK_WINDOW(m_win), title.c_str() );
        gtk_window_set_default_size (GTK_WINDOW (m_win), 800, 490);
                
        GtkWidget* vbx = GTK_WIDGET(gtk_vbox_new(0,0));
        GtkWidget* hbx = GTK_WIDGET(gtk_hbox_new(0,0));

        GtkTreeViewColumn* column;
        GtkCellRenderer*   cell_renderer;

        m_rptmodel = gtk_tree_store_new (RPT_N_COLUMNS,
                                         G_TYPE_STRING,
                                         G_TYPE_STRING,
                                         G_TYPE_STRING,
                                         G_TYPE_BOOLEAN );

        m_rptview  = gtk_tree_view_new_with_model (GTK_TREE_MODEL (m_rptmodel));
        cell_renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes ("desc",
                                                           cell_renderer,
                                                           "text", RPT_DESC,
                                                           NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (m_rptview), column);
        gtk_tree_view_column_set_sizing ( column, GTK_TREE_VIEW_COLUMN_AUTOSIZE );
        column = gtk_tree_view_column_new_with_attributes ("src",
                                                           cell_renderer,
                                                           "text", RPT_SRC,
                                                           NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (m_rptview), column);
        gtk_tree_view_column_set_sizing ( column, GTK_TREE_VIEW_COLUMN_AUTOSIZE );
        column = gtk_tree_view_column_new_with_attributes ("dst",
                                                           cell_renderer,
                                                           "text", RPT_DST,
                                                           NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (m_rptview), column);
        gtk_tree_view_column_set_sizing ( column, GTK_TREE_VIEW_COLUMN_AUTOSIZE );

        cell_renderer = gtk_cell_renderer_toggle_new ();
        g_object_set(cell_renderer,
                     "xalign", 0.0,
                     "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE,
                     0 );
        column = gtk_tree_view_column_new_with_attributes ("auto",
                                                           cell_renderer,
                                                           "active", RPT_TARGETAUTO,
                                                           NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (m_rptview), column);
        gtk_tree_view_column_set_sizing ( column, GTK_TREE_VIEW_COLUMN_AUTOSIZE );
        g_signal_connect_data( G_OBJECT( cell_renderer ), "toggled",
                               G_CALLBACK ( FerrisUI::dialog_auto_toggled ), this,
                               0, GConnectFlags(0));

                
        GtkWidget* sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(sw),
                                               GTK_WIDGET( m_rptview ));
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

        gtk_box_pack_start(GTK_BOX(vbx), sw, 1, 1, 0 );

        /******************************************************************************/
        GtkWidget* b;

        b = gtk_button_new_from_stock( GTK_STOCK_YES );
        gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
        gtk_signal_connect(GTK_OBJECT (b), "clicked",
                           GTK_SIGNAL_FUNC( FerrisUI::dialog_yes ), this );

        b = gtk_button_new_with_label( "auto yes" );
        gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
        gtk_signal_connect(GTK_OBJECT (b), "clicked",
                           GTK_SIGNAL_FUNC( FerrisUI::dialog_auto_yes ), this );

        b = gtk_button_new_with_label( "auto no" );
        gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
        gtk_signal_connect(GTK_OBJECT (b), "clicked",
                           GTK_SIGNAL_FUNC( FerrisUI::dialog_auto_no ), this );

        b = gtk_button_new_from_stock( GTK_STOCK_NO );
        gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
        gtk_signal_connect(GTK_OBJECT (b), "clicked",
                           GTK_SIGNAL_FUNC( FerrisUI::dialog_no ), this );
            
        /******************************************************************************/

        gtk_box_pack_start(GTK_BOX(vbx), hbx, 0, 0, 0 );
        gtk_container_add(GTK_CONTAINER(m_win), GTK_WIDGET( vbx ) );

    }

    void
    FileCompareWindow::processAllPendingEvents()
        {
            while (gtk_events_pending ())
                gtk_main_iteration ();
        }
    
    bool
    FileCompareWindow::processMainWindow()
    {
        gtk_widget_show_all( GTK_WIDGET(m_win) );

        m_result = false;
        m_looping = true;
            
        while( m_looping )
        {
            processAllPendingEvents();
            g_usleep(50);
        }
        gtk_widget_destroy ( GTK_WIDGET(m_win) );
        return m_result;
    }

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    gboolean GTK_TreeWalkClient__window_closing(
        GtkObject *object,
        gpointer udata)
    {
        typedef GTK_TreeWalkClient::gtk_window_is_closed_t d_t;
        d_t* d = (d_t*)udata;
        *d = 1;
        return 0;
    }
    
    GTK_TreeWalkClient::GTK_TreeWalkClient()
        :
        gtk_window_is_closed( false ),
        m_win( 0 ),
        m_treemodel( FactoryUI::CreateTreeModel() ),
        m_starttimelab( 0 ),
        m_elapsedtimelab( 0 ),
        m_srclab( 0 ),
        m_dstlab( 0 ),
        m_targetlab( 0 ),
        m_speedlab( 0 ),
        m_progress( 0 ),
        m_overall_progress( 0 ),
        m_currentObjectLab( 0 ),
        m_running( true ),
        m_highestSeenContextID( 0 ),
        m_performing( false )
    {
        m_totalagg.reset();
    }

    GTK_TreeWalkClient::~GTK_TreeWalkClient()
    {
    }

    bool
    GTK_TreeWalkClient::shouldPrecacheSourceSize()
    {
        return false;
    }
    
    
    Ferrisls_aggregate_t
    GTK_TreeWalkClient::getTotalAggregateData( fh_context src )
        {
            Ferrisls ls;
            fh_display_aggdata d = createDisplayAggregateData( &ls );
            
            if( isTrue( getStrAttr( src, "is-dir", "0" )))
                d->ShowAttributes( src );
            ls.setURL( src->getURL() );
            ls();
            
            const Ferrisls_aggregate_t& data = d->getData( AGGDATA_RECURSIVE );
            return data;
        }

    void
    GTK_TreeWalkClient::OnDropped( const ctxlist_t& clist )
    {
        cerr << "OnDropped(top)" << endl;
        for( ctxlist_t::const_iterator ci = clist.begin(); ci != clist.end(); ++ci )
        {
            cerr << "dropped ctx:" << (*ci)->getURL() << endl;
        }
        cerr << "OnDropped(bot)" << endl;

        for( ctxlist_t::const_iterator ci = clist.begin(); ci != clist.end(); ++ci )
        {
            fh_context ctx = *ci;
            m_treemodel->getRootContext()->createSubContext( "", ctx );
            if( m_performing )
            {
                m_totalagg = m_totalagg + getTotalAggregateData( ctx );
                guint64 sz = m_totalagg.sizeFilesOnly;
                LG_COPY_D << "total bytes in source:" << Util::convertByteString(sz) << endl;
                updateProgressForAggregateData( m_totalagg, true );
            }
        }

        if( !m_performing )
        {
            performActionForAllRemainingSources();
        }
    

        cerr << "selection-dump(top)" << endl;
        for( Context::iterator ci = m_treemodel->getRootContext()->begin(); ci != m_treemodel->getRootContext()->end(); ++ci )
        {
            cerr << "sel ctx:" << (*ci)->getURL() << endl;
            cerr << "ID:" << getStrAttr( *ci, "selection-added-order-id", "" ) << endl;
        }
        cerr << "selection-dump(bot)" << endl;
    }

    ctxlist_t 
    GTK_TreeWalkClient::getAllRemainingSrcsContextList()
    {
        ctxlist_t ret;
        GtkTreeIter giter;
        GtkTreeModel* m = m_treemodel->getGtkModel();

        if( gtk_tree_model_get_iter_first (m, &giter) )
        {
            while( true )
            {
                fh_context c = m_treemodel->toContext( &giter );

                guint64 cid = toType<guint64>(getStrAttr( c, "selection-added-order-id", "" ));
                if( cid > m_highestSeenContextID )
                    ret.push_back( c );
            
                if( !gtk_tree_model_iter_next( m, &giter ))
                    break;
            }
        }
        return ret;
    }
    
    void
    GTK_TreeWalkClient::performActionForSource( fh_context c )
    {
    }

    void
    GTK_TreeWalkClient::updateProgressForAggregateData( const Ferrisls_aggregate_t& agg,
                                                        bool added )
    {
    }
    
    
    void
    GTK_TreeWalkClient::performActionForAllRemainingSources()
    {
        Util::ValueRestorer< bool > x( m_performing, true );

        GtkTreeIter giter;
        GtkTreeModel* m = m_treemodel->getGtkModel();

        ctxlist_t clist = getAllRemainingSrcsContextList();
    
//     {
//         cerr << "copyAllRemainingSources(dump start)" << endl;
//         for( ctxlist_t::iterator ci = clist.begin(); ci != clist.end(); ++ci )
//         {
//             fh_context src = *ci;
//             cerr << "src:" << src->getURL() << endl;
//         }
//         cerr << "copyAllRemainingSources(dump end)" << endl;
//     }
    
    
        if( shouldPrecacheSourceSize() )
        {
            m_totalagg.reset();

            for( ctxlist_t::iterator ci = clist.begin(); ci != clist.end(); ++ci )
            {
                fh_context src = *ci;
                LG_COPY_D << "Precaching the size and other metadata for the src:" << src->getURL() << endl;

                m_totalagg = m_totalagg + getTotalAggregateData( src );
            }

            guint64 sz = m_totalagg.sizeFilesOnly;
            LG_COPY_D << "total bytes in source:" << Util::convertByteString(sz) << endl;
            updateProgressForAggregateData( m_totalagg, false );
        }

        //
        // Actually copy the data.
        //
        for( ctxlist_t::iterator ci = clist.begin(); ci != clist.end(); ++ci )
        {
            fh_context c = *ci;

            performActionForSource( c );
        
            m_highestSeenContextID = max(
                m_highestSeenContextID,
                toType<guint64>(getStrAttr( c, "selection-added-order-id", "" )));
        }

        //
        // Process any thing that was dropped while we were copying.
        //
        clist = getAllRemainingSrcsContextList();
        if( !clist.empty() )
        {
            performActionForAllRemainingSources();
        }
        
    }

    
    
    enum
    {
        TARGET_FERRIS_URL_LIST = 0,
        TARGET_STRING
    };

    static GtkTargetEntry row_targets[] = {
        { (gchar*)"text/ferris-url-list", 0,        TARGET_FERRIS_URL_LIST },
        { (gchar*)"STRING",               0,        TARGET_STRING },
        { (gchar*)"text/plain",           0,        TARGET_STRING },
        { (gchar*)"text/uri-list",        0,        TARGET_STRING },
    };

    static void  
    twc_drag_data_received(
        GtkWidget          *widget,
        GdkDragContext     *context,
        gint                x,
        gint                y,
        GtkSelectionData   *data,
        guint               info,
        guint               time,
        gpointer            udata)
    {
        GTK_TreeWalkClient* twc = (GTK_TreeWalkClient*)udata;
        twc->drag_data_received( widget, context, x, y, data, info, time );
    }

    void  
    GTK_TreeWalkClient::drag_data_received(
        GtkWidget          *widget,
        GdkDragContext     *context,
        gint                x,
        gint                y,
        GtkSelectionData   *data,
        guint               info,
        guint               time )
    {
        bool drag_finished = false;
        
        if ((data->length < 0) || (data->format != 8))
        {
            gtk_drag_finish (context, FALSE, FALSE, time);
            return;
        }

        LG_GTKFERRIS_D << "drag_data_received()" << endl;

        try
        {
            if( TARGET_FERRIS_URL_LIST == info || TARGET_STRING == info )
            {
                string s;
                fh_stringstream ss;
                ss << ((char*)data->data);
                gtk_drag_finish (context, TRUE, FALSE, time);
                drag_finished = true;

                ctxlist_t dropped;
                while( getline( ss, s ) && !s.empty() )
                {
                    try
                    {
                        fh_context c = Resolve( s );
                        dropped.push_back(c);
                    }
                    catch( exception& e )
                    {
                        fh_stringstream ss;
                        ss << "Error:" << e.what();
                        RunErrorDialog( tostr(ss) );
                    }
                }
                OnDropped( dropped );
            }
            else
            {
                fh_stringstream ss;
                ss << "Got info:" << info << " data:" << ((char*)data->data) << endl;
                RunInfoDialog( tostr(ss) );
            }
        }
        catch( exception& e )
        {
            if( !drag_finished )
            {
                drag_finished = true;
                gtk_drag_finish (context, TRUE, FALSE, time);
            }
            
            fh_stringstream ss;
            ss << "Failed to set icon\n"
               << "reason:" << e.what() << endl;
//            cerr << tostr(ss);
            FerrisUI::RunErrorDialog( tostr(ss) );
        }

        if( !drag_finished )
            gtk_drag_finish (context, TRUE, FALSE, time);

    }
    
    
    
    
    void
    GTK_TreeWalkClient::makeMainWindow( const std::string& title )
    {
        m_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_signal_connect(GTK_OBJECT (m_win), "destroy",
                           GTK_SIGNAL_FUNC(GTK_TreeWalkClient__window_closing),
                           &gtk_window_is_closed );

        gtk_window_set_default_size (GTK_WINDOW (m_win), 700, 400);
        gtk_window_set_title( GTK_WINDOW(m_win), title.c_str() );

        m_treeview = gtk_tree_view_new();

        gtk_drag_dest_set ( GTK_WIDGET(m_treeview),
                            GTK_DEST_DEFAULT_ALL,
                            row_targets,
                            G_N_ELEMENTS (row_targets),
                            GdkDragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE));
        gtk_signal_connect( GTK_OBJECT(m_treeview), "drag_data_received",
                            GTK_SIGNAL_FUNC( twc_drag_data_received ), this );

        
        gtk_tree_view_set_model( GTK_TREE_VIEW(m_treeview), m_treemodel->getGtkModel());

        gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (m_treeview), TRUE);
        GObject *selection;
        selection = G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (m_treeview)));
        gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_MULTIPLE);
        gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW (m_treeview), false );
        gtk_tree_view_set_expander_column ( GTK_TREE_VIEW(m_treeview), 0 );
        gtk_tree_view_columns_autosize(GTK_TREE_VIEW (m_treeview));
        
        m_scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_add_with_viewport(
            GTK_SCROLLED_WINDOW(m_scrolledwindow),
            GTK_WIDGET(m_treeview));
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (m_scrolledwindow),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);

        GtkWidget* vbx = GTK_WIDGET(gtk_vbox_new(0,0));
        GtkWidget* hbx = GTK_WIDGET(gtk_hbox_new(0,0));

        int r = -1;
        GtkWidget* lab;
        GtkWidget* table = gtk_table_new ( 1, 3, false );
        gtk_table_set_col_spacing( GTK_TABLE(table),
                                   1,
                                   gtk_table_get_default_col_spacing( GTK_TABLE( table)) + 10 );
        
        addBodyElements(    table,  r );
        resizeBodyElements( table, -1 );
        
        gtk_box_pack_start(GTK_BOX(vbx), m_scrolledwindow, 1, 1, 0 );
        gtk_box_pack_start(GTK_BOX(vbx), table, 0, 0, 0 );
        gtk_box_pack_start(GTK_BOX(vbx), hbx, 0, 0, 0 );

        m_notebook = GTK_NOTEBOOK(gtk_notebook_new());
        gtk_notebook_append_page( m_notebook, vbx, gtk_label_new( "general" ));
        
        addOtherPages();
        gtk_container_add(GTK_CONTAINER(m_win), GTK_WIDGET(m_notebook) );
    }
    
    void
    GTK_TreeWalkClient::showMainWindow( bool Sloth, bool force )
    {
        if( force || !Sloth )
        {
            gtk_widget_show_all(m_win);
        }
    }

    void
    GTK_TreeWalkClient::processAllPendingEvents()
    {
        while (gtk_events_pending ())
            gtk_main_iteration ();

        if( gtk_window_is_closed )
        {
            throw Quit_Requested();
        }
    }
    
    void
    GTK_TreeWalkClient::runMainWindow( bool Sloth )
    {
        if( m_srclab )   gtk_label_set_text( GTK_LABEL(m_srclab), "" );
        if( m_dstlab )   gtk_label_set_text( GTK_LABEL(m_dstlab), "" );
        if( m_targetlab) gtk_label_set_text( GTK_LABEL(m_targetlab), "" );
        if( m_speedlab ) gtk_label_set_text( GTK_LABEL(m_speedlab), "" );
        if( m_progress )
            {
                gtk_progress_bar_set_fraction( m_progress, 1.0 );
                gtk_progress_bar_set_text( m_progress, "Done." );
            }
        if( m_overall_progress )
            {
                gtk_progress_bar_set_fraction( m_overall_progress, 1.0 );
                gtk_progress_bar_set_text( m_overall_progress, "Done." );
            }
        gtk_widget_set_sensitive( m_playpausebut, false );
        
        updateElapsedTime();
        gboolean vis = 0;
        g_object_get( m_win, "visible", &vis, 0 );
        if( vis )
            while( !gtk_window_is_closed )
            {
                processAllPendingEvents();
                g_usleep(50);
            }
    }

    void
    GTK_TreeWalkClient::updateStartTime()
    {
        actionstart_tt = Time::getTime();
        gtk_label_set_text( GTK_LABEL(m_starttimelab),
                            Time::toTimeString( actionstart_tt ).c_str() );
    }
    

    void
    GTK_TreeWalkClient::updateElapsedTime()
    {
        fh_stringstream ss;
        double timedelta = difftime( Time::getTime(), actionstart_tt );
        double mod = 3600;
        if( timedelta >= mod )
        {
            double n = fmod( timedelta, mod );
            ss << ((timedelta - n)/mod) << ":";
            timedelta = n;
        }
        mod = 60;
        if( timedelta >= mod )
        {
            double n = fmod( timedelta, mod );
            ss << ((timedelta - n)/mod) << ":";
            timedelta = n;
        }
        ss << timedelta;
                        
        gtk_label_set_text( GTK_LABEL(m_elapsedtimelab), tostr(ss).c_str() );
    }
    
    void
    GTK_TreeWalkClient::resizeBodyElements( GtkWidget* table, int rowCount )
    {
        if( !rowCount )
            return;
        
        guint rows = 0;
        guint columns = 2;

        g_object_get( G_OBJECT(table),
                      "n-rows",    &rows, 
                      "n-columns", &columns,
                      0 );
        rows += rowCount;
        gtk_table_resize( GTK_TABLE(table), rows, columns );
    }

    /**
     * Add a label that is to be as small as possible
     */
    void
    GTK_TreeWalkClient::addSmallLabel( GtkWidget* table, GtkWidget* lab, int c, int r )
    {
        gtk_misc_set_alignment( GTK_MISC( lab ), 1.0, 0.5 );
//        gtk_table_attach_defaults(GTK_TABLE(table), lab, 0, 1, r, r+1 );
        gtk_table_attach(GTK_TABLE(table), lab, c, c+1, r, r+1,
                         GtkAttachOptions(GTK_FILL), GtkAttachOptions(0), 0, 0 );
    }

    /**
     * Add a label that is to expand to take up the rest of the X area
     */
    void
    GTK_TreeWalkClient::addLargeLabel( GtkWidget* table, GtkWidget* lab, int c, int r )
    {
        gtk_misc_set_alignment( GTK_MISC( lab ), 0.0, 0.5 );
        gtk_table_attach(GTK_TABLE(table), lab, 1, 2, r, r+1,
                         GtkAttachOptions(GTK_FILL|GTK_SHRINK|GTK_EXPAND), GtkAttachOptions(0), 5, 0 );
    }
    
    
    void
    GTK_TreeWalkClient::addSeparatorRow( GtkWidget* table, int& r, const char* label )
    {
        resizeBodyElements( table, 1 );
        ++r;

        GtkWidget* lab  = gtk_label_new( label );
        addSmallLabel( table, lab, 0, r );

        lab = gtk_hseparator_new();
//        addLargeLabel( table, lab, 1, r );
//        gtk_misc_set_alignment( GTK_MISC( lab ), 0.0, 0.5 );
        gtk_table_attach(GTK_TABLE(table), lab, 1, 2, r, r+1,
                         GtkAttachOptions(GTK_FILL|GTK_SHRINK|GTK_EXPAND), GtkAttachOptions(0), 5, 0 );
    }

    void
    GTK_TreeWalkClient::setSourceLabel( const std::string& s )
    {
        gtk_label_set_text( GTK_LABEL(m_srclab), s.c_str() );
    }
    
    void
    GTK_TreeWalkClient::setDestinationLabel( const std::string& s )
    {
        gtk_label_set_text( GTK_LABEL(m_dstlab), s.c_str() );
    }
    
    
    void
    GTK_TreeWalkClient::addSourceRow( GtkWidget* table, int& r )
    {
        resizeBodyElements( table, 1 );
        ++r;
        GtkWidget* lab  = gtk_label_new( "Src:" );
        addSmallLabel( table, lab, 0, r );
        
        lab = m_srclab = gtk_label_new("");
        gtk_label_set_ellipsize( GTK_LABEL(lab), PANGO_ELLIPSIZE_START );
        addLargeLabel( table, lab, 1, r );
    }

    void
    GTK_TreeWalkClient::addDestinationRow( GtkWidget* table, int& r )
    {
        resizeBodyElements( table, 1 );
        ++r;
        GtkWidget* lab  = gtk_label_new( "Dst:" );
        addSmallLabel( table, lab, 0, r );

        lab = m_dstlab = gtk_label_new("");
        gtk_label_set_ellipsize( GTK_LABEL(lab), PANGO_ELLIPSIZE_START );
        addLargeLabel( table, lab, 1, r );
    }

    void
    GTK_TreeWalkClient::addTarget( GtkWidget* table, int& r )
    {
        resizeBodyElements( table, 1 );
        ++r;
        GtkWidget* lab  = gtk_label_new( "Target:" );
        addSmallLabel( table, lab, 0, r );

        lab = m_targetlab = gtk_label_new("");
        addLargeLabel( table, lab, 1, r );
    }
    
    void
    GTK_TreeWalkClient::addCurrent( GtkWidget* table, int& r )
    {
        resizeBodyElements( table, 1 );
        ++r;
        GtkWidget* lab  = gtk_label_new( "Current:" );
        addSmallLabel( table, lab, 0, r );

        lab = m_currentObjectLab = gtk_label_new("");
        addLargeLabel( table, lab, 1, r );
    }
    
    void
    GTK_TreeWalkClient::addSpeedRow( GtkWidget* table, int& r )
    {
        resizeBodyElements( table, 1 );
        ++r;
        GtkWidget* lab  = gtk_label_new( "Speed:" );
        addSmallLabel( table, lab, 0, r );

        lab = m_speedlab = gtk_label_new("");
        addLargeLabel( table, lab, 1, r );
    }
    
    void
    GTK_TreeWalkClient::addProgressRow( GtkWidget* table, int& r )
    {
        resizeBodyElements( table, 1 );
        ++r;
        m_progress = GTK_PROGRESS_BAR(gtk_progress_bar_new());
        gtk_progress_bar_set_orientation( m_progress, GTK_PROGRESS_LEFT_TO_RIGHT );
        gtk_progress_bar_set_text( m_progress, "Init." );
        gtk_progress_set_percentage( GTK_PROGRESS(m_progress), 0 );
        gtk_table_attach_defaults(GTK_TABLE(table), GTK_WIDGET(m_progress), 0, 3, r, r+1 );
    }

    void
    GTK_TreeWalkClient::setProgress( double current, double total )
    {
            double v = total ? current / total : 0;
            stringstream ss;
            ss << current << " / " << total;
            setProgress( v, ss.str() );
    }
    
    
    void
    GTK_TreeWalkClient::setProgress( double v, const std::string& s )
    {
        if( GtkProgressBar* p = m_progress )
        {
            gtk_progress_bar_set_fraction( p, v );
            gtk_progress_bar_set_text( p, s.c_str() );
        }
    }
    
    void
    GTK_TreeWalkClient::setOverallProgress( double v, const std::string& s )
    {
        if( GtkProgressBar* p = m_overall_progress )
        {
            gtk_progress_bar_set_fraction( p, v );
            gtk_progress_bar_set_text( p, s.c_str() );
        }
    }
    
    void
    GTK_TreeWalkClient::setOverallProgress( double current, double total )
    {
            double v = total ? current / total : 0;
            stringstream ss;
            ss << current << " / " << total;
            setOverallProgress( v, ss.str() );
    }
    
    

    void
    GTK_TreeWalkClient::addOverallProgressRow( GtkWidget* table, int& r )
    {
        resizeBodyElements( table, 1 );
        ++r;
        m_overall_progress = GTK_PROGRESS_BAR(gtk_progress_bar_new());
        gtk_progress_bar_set_orientation( m_overall_progress, GTK_PROGRESS_LEFT_TO_RIGHT );
        gtk_progress_bar_set_text( m_overall_progress, "x" );
        gtk_progress_set_percentage( GTK_PROGRESS(m_overall_progress), 0 );
        gtk_table_attach_defaults(GTK_TABLE(table), GTK_WIDGET(m_overall_progress), 0, 3, r, r+1 );
    }
    
    void
    GTK_TreeWalkClient::addStartTimeRow( GtkWidget* table, int& r )
    {
        resizeBodyElements( table, 1 );
        ++r;
        GtkWidget* lab = gtk_label_new( "Started:" );
        addSmallLabel( table, lab, 0, r );

        lab = m_starttimelab = gtk_label_new("");
        addLargeLabel( table, lab, 1, r );
        updateStartTime();
    }
    
    void
    GTK_TreeWalkClient::addElapsedTimeRow( GtkWidget* table, int& r )
    {
        resizeBodyElements( table, 1 );
        ++r;
        GtkWidget* lab = gtk_label_new( "Elapsed:" );
        addSmallLabel( table, lab, 0, r );

        lab = m_elapsedtimelab = gtk_label_new("");
        addLargeLabel( table, lab, 1, r );
    }

    void play_pause_button_cb( GtkButton *button, gpointer udata )
    {
        GTK_TreeWalkClient* c = (GTK_TreeWalkClient*)udata;
        c->play_pause_button( button );
    }

    void
    GTK_TreeWalkClient::play_pause_button( GtkButton *button )
    {
        if( m_running )
            if( !tryToPause() )
                return;
        if( !m_running )
            if( !tryToPlay() )
                return;
        
        m_running = !m_running;
        if( m_running )
            gtk_button_set_label( GTK_BUTTON(m_playpausebut), "playing");
        else
            gtk_button_set_label( GTK_BUTTON(m_playpausebut), "paused");
    }
    
    
    /**
     * Add process control buttons.
     * This includes a play/pause toggle. Note that the default handling of play/pause
     * is to nest a gtk main loop, so the subclass doesn't normally have to do anything
     * to handle play/pause provided that it can recover from a pause of any time happening
     * (eg. network related stuff should recover using TCP/IP, though it might be a nice
     *  thing to notify the peer of a pause explicitly)
     */ 
    void
    GTK_TreeWalkClient::addVCRControls( GtkWidget* table )
    {
        guint rows = 0;
        g_object_get( G_OBJECT(table), "n-rows",    &rows,  0 );

        GtkWidget* vbx = GTK_WIDGET(gtk_vbox_new(0,0));
        GtkWidget* b;

        
        b = m_playpausebut = makeImageButton( "icons://Button-Pause.png" );
        gtk_signal_connect(GTK_OBJECT (b), "clicked",
                           GTK_SIGNAL_FUNC( FerrisUI::play_pause_button_cb ), this );
        
        gtk_box_pack_start(GTK_BOX(vbx), b, 0, 0, 0 );
        gtk_table_attach(GTK_TABLE(table), GTK_WIDGET(vbx), 2, 3, 0, rows,
                         GtkAttachOptions(GTK_FILL), GtkAttachOptions(0), 0, 0 );
    }

    bool
    GTK_TreeWalkClient::tryToPlay()
    {
        m_running = true;
        setImageButton( m_playpausebut, "icons://Button-Pause.png" );
        gtk_main_quit();
        return false;
    }
    
    bool
    GTK_TreeWalkClient::tryToPause()
    {
        m_running = false;
        setImageButton( m_playpausebut, "icons://Button-Play.png" );
        gtk_main();
        return false;
    }

    void
    GTK_TreeWalkClient::setWindowTitle( const std::string& title )
    {
        gtk_window_set_title( GTK_WINDOW(m_win), title.c_str() );
    }
    
    void
    GTK_TreeWalkClient::addBodyElements( GtkWidget* table, int& r )
    {
        resizeBodyElements( table, 0 );
        addStartTimeRow(    table, r );
        addElapsedTimeRow(  table, r );
    }
    
    void
    GTK_TreeWalkClient::addSkippingPage()
    {
        GtkWidget* skippage = GTK_WIDGET(gtk_hbox_new(0,0));

        GtkTreeViewColumn* column;
        GtkCellRenderer*   cell_renderer;

        m_skipmodel = gtk_tree_store_new (SKIP_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
        m_skipview  = gtk_tree_view_new_with_model (GTK_TREE_MODEL (m_skipmodel));
        cell_renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes ("reason",
                                                           cell_renderer,
                                                           "text", SKIP_REASON,
                                                           NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (m_skipview), column);

        column = gtk_tree_view_column_new_with_attributes ("skipped",
                                                           cell_renderer,
                                                           "text", SKIP_DESC,
                                                           NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (m_skipview), column);

        GtkWidget* sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(sw),
                                               GTK_WIDGET( m_skipview ));
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

        gtk_container_add(GTK_CONTAINER(skippage), GTK_WIDGET( sw ));
        gtk_notebook_append_page( m_notebook, skippage, gtk_label_new( "skipped" ));
    }

    bool
    GTK_TreeWalkClient::getShouldRunMainLoop( bool AutoClose, bool hadUserInteraction )
    {
        if( AutoClose && !getNumberOfSkipped() && !hadUserInteraction )
        {
            return false;
        }
        return true;
    }
    
    
    void
    GTK_TreeWalkClient::addOtherPages()
    {
        addSkippingPage();
    }

    void
    GTK_TreeWalkClient::addToSkipped(
        const std::string& desc,
        const std::string& reason )
    {
        GtkTreeIter iter;
        gtk_tree_store_append( m_skipmodel, &iter, 0 );
        gtk_tree_store_set( m_skipmodel, &iter,
                            SKIP_REASON, reason.c_str(),
                            SKIP_DESC, desc.c_str(),
                            -1 );
    }
    
    static gboolean
    count_f( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
    {
        guint32* count = (guint32*)data;
        *count = *count + 1;
    }
    
    int
    GTK_TreeWalkClient::getNumberOfSkipped()
    {
        guint32 count = 0;
        
        gtk_tree_model_foreach( GTK_TREE_MODEL(m_skipmodel),
                                GtkTreeModelForeachFunc(count_f),
                                gpointer( &count ));
        return count;
    }
    

    
    void
    GTK_TreeWalkClient::makeDefaultColumnViews()
    {
        makeColumnView( "size-human-readable" );
        makeColumnView( "mtime-display" );
        makeColumnView( "url" );
    }
    
    void
    GTK_TreeWalkClient::makeColumnView( const std::string& cn )
    {
        int colnum = m_treemodel->getColumnNumber( cn );

        LG_GTKFERRIS_D << "GTK_TreeWalkClient::makeColumnView() "
                       << " cn:" << cn
                       << " colnum:" << colnum
                       << endl;
        
        if( colnum != -1 )
        {
            GtkCellRenderer* r = gtk_ferris_cell_renderer_text_new ();
            g_object_set(r, "editable", 0, 0 );

            GtkTreeViewColumn* c = gtk_tree_view_column_new_with_attributes(
                cn.c_str(), GTK_CELL_RENDERER( r ), NULL);

            gtk_tree_view_column_set_attributes( c, r, "text", colnum, 0 );
            gtk_tree_view_append_column( GTK_TREE_VIEW( m_treeview ), c );
            gtk_tree_view_column_set_reorderable( c, 1 );
            gtk_tree_view_column_set_clickable( c, 1 );
            gtk_cell_renderer_text_set_fixed_height_from_font(
                GTK_CELL_RENDERER_TEXT(r), 1 );

            if( cn == "url" )
            {
                gtk_tree_view_column_set_sizing ( c, GTK_TREE_VIEW_COLUMN_AUTOSIZE );
                gtk_tree_view_column_set_resizable( c, true );
            }
            else
            {
                GtkFerrisCellRendererText* ren = GTK_FERRIS_CELL_RENDERER_TEXT(r);
                
                if( cn == "size-human-readable" )
                {
                    gtk_cell_renderer_text_set_fixed_size_string( ren, "123456Mb" );
                }
                else if( cn == "protection-ls" )
                {
                    gtk_cell_renderer_text_set_fixed_size_string( ren, "-rwxrwxrwx" );
                }
                else if( cn == "mtime-display" )
                {
                    gtk_cell_renderer_text_set_fixed_size_string( ren, "00 MMM 00 00:00 " );
                }
            }
        }
        else
        {
            cerr << "Attempt to add column:" << cn << " failed because model does"
                 << " not know about the column"
                 << endl;
        }
    }
    

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    
};



namespace Ferris
{

gint
GtkProgressBinder_updater(gpointer data)
{
    GtkProgressBinder* binder = (GtkProgressBinder*)data;
    
    gtk_progress_set_value( GTK_PROGRESS( binder->w ), binder->items_read );

//    cerr << "GtkProgressBinder_updater()\n\n";
    
    
    return 1;
}





GtkProgressBinder::GtkProgressBinder(
    GtkProgress* _w,
    fh_context _ctx,
    guint32 _interval,
    int _update_gui
    )
    :
    w(_w), ctx(_ctx), interval(_interval), update_gui(_update_gui)
{
    int size = ctx->guessSize();
//    cerr << "size:" << size << endl;
//    sleep(2);
            
    gtk_progress_configure( GTK_PROGRESS(w), 0, 0, size );

    ctx->getNamingEvent_Start_Reading_Context_Sig().connect(
        sigc::mem_fun( *this, &GtkProgressBinder::read_start ));

    ctx->getNamingEvent_Stop_Reading_Context_Sig().connect(
        sigc::mem_fun( *this, &GtkProgressBinder::read_stop ));
        
    conn = ctx->getNamingEvent_Exists_Sig().connect(
        sigc::mem_fun( *this, &GtkProgressBinder::naming_exists ));

    
}

void
GtkProgressBinder::read_start( NamingEvent_Start_Reading_Context* e )
{
    timer = gtk_timeout_add( interval, GtkProgressBinder_updater, this);
    items_read=0;
}


void
GtkProgressBinder::read_stop( NamingEvent_Stop_Reading_Context* e )
{
    gtk_timeout_remove( timer );
    gtk_progress_set_value( GTK_PROGRESS( w ), items_read );
}



void
GtkProgressBinder::naming_exists(NamingEvent_Exists* e,
                                 const fh_context& subc,
                                 string o, string n)
{
//     static int c = 0;
//     cerr << "naming_exists! c:" << c << " chunk_size:" << chunk_size << endl;
    
    ++items_read;
//     if(!(c % chunk_size ))
//     {
//         cerr << "!!!!!!!!! SETTING VALUE! " << endl;
        
//         if( update_gui )
//         {
//             cerr << "GUI UPDATE" << endl << endl;
                    
//             while (gtk_events_pending())
//                 gtk_main_iteration();
//         }
                
//     }
            
}

 
};

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
namespace FerrisUI
{

    static string getRealPrefix( const std::string& path )
    {
        fh_context c = Resolve( path );
        return c->getDirPath();
    }

    static string tryRemovePrefix( const std::string& path, const std::string& prefix )
    {
        string ret = path;
        
        if( starts_with( path, prefix ) )
        {
            ret = getRealPrefix( prefix ) + "/" + path.substr( prefix.length() );
        }
        return ret;
    }
    
    
    string resolveToIconPath( string iconpath )
    {
        if( starts_with( iconpath, "gtk-" ) )
            return iconpath;
        
        iconpath = tryRemovePrefix( iconpath, "icons://" );
        iconpath = tryRemovePrefix( iconpath, "gnomeicons://" );

        if( !iconpath.empty() )
            iconpath = CleanupURL( iconpath );
        
//         if( starts_with( iconpath, "icons://" ) ||
//             starts_with( iconpath, "gnomeicons://" ))
//         {
//             try
//             {
//                 fh_context c = Resolve( iconpath );
//                 iconpath = c->getDirPath();
//                 cerr << "makeMenuItem adjustedicon:" << iconpath << endl;
//             }
//             catch(...)
//             {
//                 iconpath = "";
//             }
//         }
        if( starts_with( iconpath, "file:" ))
        {
            iconpath = iconpath.substr( 5 );
        }
        
        
        return iconpath;
    }

    GtkWidget* makeImageButton( const std::string& s )
    {
        GtkWidget* b = gtk_button_new();
        gtk_container_add(
            GTK_CONTAINER(b),
            gtk_image_new_from_file(
                resolveToIconPath( s ).c_str() ));
        return GTK_WIDGET(b);
    }    

    GtkWidget*
    setImageButton( GtkWidget* w, const std::string& s )
    {
        GtkWidget* c = gtk_bin_get_child( GTK_BIN(w) );
        if( GTK_IS_IMAGE( c ) )
        {
            gtk_image_set_from_file( GTK_IMAGE( c ),
                                     resolveToIconPath( s ).c_str() );
        }
        return GTK_WIDGET(w);
    }

    /************************************************/
    /************************************************/

    struct RGBAtoPixbuf_Data
    //        :
    //        public Loki::SmallObject<>
    {
        int w;
        int h;
        RGBAtoPixbufFree_f f;

        RGBAtoPixbuf_Data(  int w, int h, const RGBAtoPixbufFree_f& f )
            :
            w(w), h(h), f(f)
            {
            }
    };
    
    static void RGBAtoPixbuf_GdkPixbufDestroyNotify(guchar *pixels, gpointer data)
    {
        RGBAtoPixbuf_Data* d = (RGBAtoPixbuf_Data*)data;

        d->f( d->w, d->h, pixels );
        delete d;
    }

    void RGBAtoPixbuf_null( int w, int h, gpointer data )
    {
    }
    void RGBAtoPixbuf_free( int w, int h, gpointer data )
    {
        free( data );
    }
    void RGBAtoPixbuf_delarray( int w, int h, gpointer data )
    {
        unsigned char* dp = (unsigned char*)data;
        delete [] dp;
    }
    
    GdkPixbuf* RGBAtoPixbuf( int w, int h, gpointer data,
                             RGBAtoPixbufFree_f f )
    {
        GdkColorspace colorspace = GDK_COLORSPACE_RGB;
        gboolean has_alpha = 1;
        int bits_per_sample = 8;
        int rowstride = w * 4;
        RGBAtoPixbuf_Data* d = new RGBAtoPixbuf_Data( w, h, f );
        
        {
            guint8* d = (guint8*)data;
            gint64  wh = w*h*4;
            
//             EPEG_ARGB 
//             EPEG_RGBA 
            for( guint64 i = 0; i < wh; i+=4 )
            {
                swap( d[ i + 0 ], d[ i + 2 ] );
            }
        }
        
        GdkPixbuf* ret = gdk_pixbuf_new_from_data(
            (const guchar *)data,
            colorspace,
            has_alpha,
            bits_per_sample,
            w,
            h,
            rowstride,
            RGBAtoPixbuf_GdkPixbufDestroyNotify, d );

        return ret;
    }

    GtkImage* RGBAtoImage( GtkImage* im, int w, int h, gpointer data,
                           RGBAtoPixbufFree_f f )
    {
        GdkPixbuf* pb = RGBAtoPixbuf( w, h, data, f );
        cerr << "RGBAtoImage() im:" << toVoid(im)
             << " w:" << w << " h:" << h
             << " data:" << toVoid( data )
             << " pb:" << toVoid( pb )
             << endl;
        gtk_image_set_from_pixbuf( im, pb );
        gdk_pixbuf_unref( pb );
    }

    GtkImage* setImageFromRGBAAttribute( GtkImage* im,
                                         fh_context c,
                                         const std::string& rdn_width,
                                         const std::string& rdn_height,
                                         const std::string& rdn_rgba32 )
    {
        bool hadError = false;
        int width  = -1;
        int height = -1;
        unsigned char* data = 0;
        try
        {
            width  = toint( getStrAttr( c, rdn_width, "-1" ));
            height = toint( getStrAttr( c, rdn_height, "-1" ));

            cerr << "setImageFromRGBAAttribute() c:" << c->getURL()
                 << " width:" << width
                 << " height:" << height
                 << endl;
            
            if( width < 0 || height < 0 )
                hadError = true;
            else
            {
                cerr << "setImageFromRGBAAttribute() c:" << c->getURL()
                     << " rdn_rgba32:" << rdn_rgba32
                     << endl;
                
                fh_attribute a = c->getAttribute( rdn_rgba32 );
                fh_istream iss = a->getIStream();
                const int datasz = width * height * 4;
                cerr << "setImageFromRGBAAttribute() c:" << c->getURL()
                     << " datasz:" << datasz
                     << endl;

                data = new unsigned char[ datasz + 1 ];
                if( !data )
                {
                    cerr << "setImageFromRGBAAttribute(e) c:" << c->getURL()
                         << " datasz:" << datasz
                         << endl;
                    hadError = true;
                }
                else
                {
                    cerr << "setImageFromRGBAAttribute() c:" << c->getURL()
                         << " reading data..."
                         << endl;
                    
                    iss.read( (char*)data, datasz );

                    cerr << "setImageFromRGBAAttribute() c:" << c->getURL()
                         << " read:" << iss.gcount()
                         << endl;
                    if( iss.gcount() != datasz )
                    {
                        cerr << "setImageFromRGBAAttribute(e) c:" << c->getURL()
                             << " iss.gcount():" << iss.gcount()
                             << endl;
                        hadError = true;
                    }
                    else
                    {
                        RGBAtoImage( im, width, height, data );
                    }
                }
            }
        }
        catch( exception& e )
        {
            cerr << "setImageFromRGBAAttribute(e) c:" << c->getURL()
                 << " e:" << e.what()
                 << endl;
            
            hadError = true;
        }

        cerr << "setImageFromRGBAAttribute(ending) c:" << c->getURL()
             << " hadError:" << hadError
             << endl;
        
        if( hadError || width == -1 || height == -1 )
        {
            cerr << "setImageFromRGBAAttribute(hadError) c:" << c->getURL()
                 << " w:" << width
                 << " h:" << height
                 << endl;
            
            if( data )
                delete [] data;
            gtk_image_set_from_stock( im,
                                      GTK_STOCK_MISSING_IMAGE,
                                      GTK_ICON_SIZE_BUTTON);
            return im;
        }
        return im;
    }
    
    GtkImage* setImageFromExifThumb( GtkImage* im, fh_context c )
    {
        return setImageFromRGBAAttribute( im, c,
                                          "exif:thumbnail-width",
                                          "exif:thumbnail-height",
                                          "exif:thumbnail-rgba-32bpp" );
    }
    

    /************************************************/
    /************************************************/
    
};
