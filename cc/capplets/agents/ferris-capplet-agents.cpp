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

    $Id: ferris-capplet-agents.cpp,v 1.5 2010/09/24 21:31:21 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <glib.h>
#include <gtk/gtk.h>

#include <Ferris/Ferris.hh>
#include <Ferris/Medallion.hh>
#include <Ferris/Agent.hh>
#include <Ferris/Agent_private.hh>
#include <FerrisUI/GtkFerris.hh>
#include <FerrisUI/EditStringList.hh>
#include <FerrisUI/Menus.hh>

#include <string>
#include <iomanip>

using namespace std;
using namespace Ferris;
using namespace Ferris::AI;
using namespace FerrisUI;

 
const string PROGRAM_NAME = "ferris-capplet-agents";

GtkWidget* gtk_window;
GtkWidget* gtk_scrolledwindow;
GtkWidget* gtk_table;

//
// main tree
//
enum {

    C_AGENTNAME_COLUMN=0,
    C_STATEDIR_COLUMN,
    C_ALGO_COLUMN,
//     C_EMBLEM_ID_COLUMN,
//     C_EMBLEM_NAME_COLUMN,
    C_PERS_ID_COLUMN,
    C_PERS_NAME_COLUMN,

    C_DIRECTORPTR_COLUMN,
    
    C_COLUMN_COUNT
};
GtkTreeStore*      w_treemodel;
GtkWidget*         w_treeview;
GtkTreeViewColumn* w_cols[C_COLUMN_COUNT];

//
// emblems list
//
enum {
    CE_CHECKED_COLUMN=0,
    CE_ID_COLUMN,
    CE_NAME_COLUMN,
    CE_COLUMN_COUNT
};
GtkTreeStore*      w_emblems_treemodel;
GtkWidget*         w_emblems_treeview;
GtkTreeViewColumn* w_emblems_cols[CE_COLUMN_COUNT];

fh_etagere et = 0;

// emblemID_t getCurrentSelectedEmblemID();


// gpointer
// treeptr( GtkTreeIter *iter, GtkWidget* w_treeview, int col )
// {
//     GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
//     GtkTreeModel* w_treemodel = gtk_tree_view_get_model( GTK_TREE_VIEW(w_treeview) );
//     GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
//     GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);

//     GValue value;
//     memset( &value, 0, sizeof(value));
//     gtk_tree_model_get_value( tm, iter, col, &value );
//     gpointer p = g_value_get_pointer(&value);
//     gpointer ret = p;
//     g_value_unset ( &value );

//     return ret;
// }


GtkTreeIter
appendNewBlankItem( GtkTreeIter* parent = 0 )
{
    GtkTreeIter iter;
    const char* d = "<new>";
    gtk_tree_store_append( w_treemodel, &iter, parent );
    gtk_tree_store_set( w_treemodel, &iter,
                        C_AGENTNAME_COLUMN,   d,
                        -1 );
    return iter;
}

// /**
//  * Get giters to all the entries in the treeview with the given id
//  */
// list_gtktreeiter_t
// findAllByID( emblemID_t eid )
// {
//     string seid = tostr(eid);
    
//     list_gtktreeiter_t ret;
//     list_gtktreeiter_t all = getIterList( w_treeview, false );
//     for( list_gtktreeiter_t::iterator li = all.begin(); li!=all.end(); ++li )
//     {
//         GtkTreeIter giter = *li;
//         string s = treestr( &giter, w_treeview, C_EMBLEM_ID_COLUMN );
//         if( s == seid )
//         {
//             ret.push_back( giter );
//         }
//     }
//     return ret;
// }



void put( GtkTreeStore* ts, GtkTreeIter& giter, fh_agent d )
{
    fh_emblem        em = d->getEmblem();
    fh_personality pers = d->getPersonality();

    d->AddRef();
    gtk_tree_store_set( ts, &giter,
                        C_AGENTNAME_COLUMN,         d->getName().c_str(),
                        C_STATEDIR_COLUMN,          d->getStateDir().c_str(),
                        C_ALGO_COLUMN,              d->getAgentImplemenationName().c_str(),
                        C_PERS_ID_COLUMN,           tostr(pers->getID()).c_str(),
                        C_PERS_NAME_COLUMN,         pers->getName().c_str(),
                        C_DIRECTORPTR_COLUMN,       GetImpl(d),
                        -1 );
}

// /**
//  * This finds a child in the w_treemodel list by eid only checking in the
//  * range starting at giter and not going into children. Note that giter
//  * should be set to a valid iterator in the treemodel at the level where
//  * the emblem you seek should be findable.
//  *
//  * return true if the emblem was found and giter points to it at return
//  */
// bool getChildByID( emblemID_t eid, GtkTreeIter& giter )
// {
// //     if( gtk_tree_model_get_iter_first( GTK_TREE_MODEL(w_treemodel), &giter ) )
// //     {
//         while( true )
//         {
//             string id = treestr( &giter, w_treeview, C_EMBLEM_ID_COLUMN );
//             if( id == tostr(eid) )
//             {
//                 return true;
//             }
//             if( !gtk_tree_model_iter_next( GTK_TREE_MODEL(w_treemodel), &giter ) )
//                 break;
//         }
// //    }
//     return false;
// }

// bool childExists( emblemID_t eid, GtkTreeIter giter )
// {
//     GtkTreeIter piter = giter;
//     bool rc = gtk_tree_model_iter_nth_child( GTK_TREE_MODEL( w_treemodel ),
//                                              &giter,
//                                              &piter,
//                                              0 );
//     return rc && getChildByID( eid, giter );
// }




void SaveData()
{
    et->sync();
    
}


void LoadData()
{
    stringlist_t& sl = getAgentNames();

    for( stringlist_t::iterator si = sl.begin(); si!=sl.end(); ++si )
    {
        cerr << "agent name:" << *si << endl;
        
        fh_agent d = getAgent( *si );
        d->AddRef(); // Using it in the GTK+ treemodel

        GtkTreeIter iter = appendNewBlankItem( 0 );
        put( w_treemodel, iter, d );
    }
    
//    gtk_tree_view_expand_all( GTK_TREE_VIEW( w_treeview ) );

    
}

void
save_and_quit_cb( GtkButton *button, gpointer user_data )
{
    SaveData();
    gtk_main_quit();
}

void
quit_cb( GtkButton *button, gpointer user_data )
{
    gtk_main_quit();
}



void
new_cb( GtkButton *button, gpointer )
{
    GtkTreeView*  tv = GTK_TREE_VIEW(  w_treeview  );
    GtkTreeModel* tm = GTK_TREE_MODEL( w_treemodel );
    
    GtkTreeIter iter = appendNewBlankItem();

    fh_personality pers = Factory::getGenericClassifierAgentPersonality();
    fh_emblem        em = et->getEmblemByID( pers->getID() );

    fh_agent d = createBinaryAgent( "new agent",
                                    "bogofilter",
                                    "/tmp/my-new-agent",
                                    em, pers );
    put( w_treemodel, iter, d );
    
    GtkTreePath* path      = gtk_tree_model_get_path( tm, &iter );
    GtkTreeViewColumn* col = gtk_tree_view_get_column( tv, C_AGENTNAME_COLUMN );
    gtk_tree_view_set_cursor( tv, path, col, true );
    gtk_tree_path_free( path );
}


void
del_cb( GtkButton *button, gpointer )
{
    list_gtktreeiter_t deliters = getIterList( w_treeview, true );
    GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
    GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
    GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection ( tv );
    gtk_tree_selection_unselect_all( selection );

//     for( list_gtktreeiter_t::iterator iter = deliters.begin(); iter != deliters.end(); ++iter )
//     {
//         GtkTreeIter giter = *iter;
//         string seid = treestr( &giter, w_treeview, C_EMBLEM_ID_COLUMN );
//         gtk_tree_store_remove( ts, &giter );

//         if( !seid.empty() )
//         {
//             emblemID_t eid = toType<emblemID_t>( seid );
//             fh_emblem   em = et->getEmblemByID( eid );
//             if( em )
//             {
//                 list_gtktreeiter_t others = findAllByID( eid );
//                 copy( others.begin(), others.end(), back_inserter(deliters) );
//                 et->erase( em );
//             }
//         }
//     }
    et->sync();
}


void
edited_cb (GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer user_data )
{
    string v = new_text ? new_text : "";
    int cidx = GPOINTER_TO_INT(user_data);
    cerr << "edited_cb() c:" << cidx << " new_text:" << new_text << endl;

    bool acceptNewValue = true;
    GtkTreeModel *model = GTK_TREE_MODEL (w_treemodel);
    GtkTreeIter iter;
    GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
    gtk_tree_model_get_iter (model, &iter, path);
    
    Agent* d = (Agent*)treeptr( &iter, w_treeview, C_DIRECTORPTR_COLUMN );
    if( d )
    {
        if( cidx == C_AGENTNAME_COLUMN )
        {
            d->setName( v );
        }
        else if( cidx == C_STATEDIR_COLUMN )
        {
            d->setStateDir( v );
        }
        else if( cidx == C_ALGO_COLUMN )
        {
            try
            {
                d->setAgentImplemenationName( v );
            }
            catch( exception& e )
            {
                acceptNewValue = false;
                fh_stringstream ss;
                ss << "There is no suitable agent implementation with the name:" << new_text << endl
                   << " Reverting to old value";
                RunErrorDialog( tostr(ss), gtk_window );
            }
        }
        else if( cidx == C_PERS_NAME_COLUMN )
        {
            try
            {
                fh_personality pers = findPersonality( v );
                d->setPersonality( pers );
            }
            catch( exception& e )
            {
                acceptNewValue = false;
                fh_stringstream ss;
                ss << "There is no personality with the name:" << new_text << endl
                   << " Reverting to old value";
                RunErrorDialog( tostr(ss), gtk_window );
            }
        }
    }

    if( acceptNewValue )
    {
        gtk_tree_store_set (GTK_TREE_STORE (model), &iter, cidx, new_text, -1);
    }
    gtk_tree_path_free (path);
}



/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void
changed_cb( GtkTreeSelection *sel, gpointer )
{
    GtkTreeIter iter;
    GtkTreeModel* model = GTK_TREE_MODEL( w_treemodel );
    
    gtk_tree_selection_get_selected ( sel, &model, &iter );
    Agent* d = (Agent*)treeptr( &iter, w_treeview, C_DIRECTORPTR_COLUMN );
    if( d )
    {
        gtk_tree_store_clear( w_emblems_treemodel );
        fh_emblem active_em = d->getEmblem();

        emblems_t el = et->getAllEmblems();
        for( emblems_t::iterator ei = el.begin(); ei!=el.end(); ++ei )
        {
            string name = (*ei)->getName().c_str();
            string id   = tostr((*ei)->getID());
            const char* pname = name.c_str();
            const char* pid   =   id.c_str();
            bool isParent =  *ei == active_em;

            gtk_tree_store_append( w_emblems_treemodel, &iter, NULL );
            gtk_tree_store_set( w_emblems_treemodel, &iter,
                                CE_CHECKED_COLUMN,   isParent,
                                CE_ID_COLUMN,        pid,
                                CE_NAME_COLUMN,      pname,
                                -1 );
        }
    }
}

gboolean emblems_uncheck_cb( GtkTreeModel *model,
                             GtkTreePath *path,
                             GtkTreeIter *iter,
                             gpointer data )
{
    gtk_tree_store_set (GTK_TREE_STORE (model), iter, CE_CHECKED_COLUMN, 0, -1);
    return 0;
}

void
emblems_edited_cb (GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer user_data )
{
    int cidx = GPOINTER_TO_INT(CE_CHECKED_COLUMN);

    GtkTreeIter iter;
    GtkTreeSelection *sel = gtk_tree_view_get_selection ( GTK_TREE_VIEW(w_treeview) );
    GtkTreeModel* model = GTK_TREE_MODEL( w_treemodel );
    gtk_tree_selection_get_selected ( sel, &model, &iter );
    Agent* d = (Agent*)treeptr( &iter, w_treeview, C_DIRECTORPTR_COLUMN );
    
    if( d )
    {
        GtkTreeModel *model = GTK_TREE_MODEL (w_emblems_treemodel);
        GtkTreeIter iter;
        GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
        gtk_tree_model_get_iter (model, &iter, path);

        gboolean value;
        gtk_tree_model_get (model, &iter, cidx, &value, -1);
        value = !value;
        gtk_tree_model_foreach( GTK_TREE_MODEL( model ), emblems_uncheck_cb, 0 );
        gtk_tree_store_set (GTK_TREE_STORE (model), &iter, cidx, value, -1);
        gtk_tree_path_free (path);

        std::string seid = treestr( &iter, GTK_WIDGET( w_emblems_treeview ), CE_ID_COLUMN );
        emblemID_t   eid = toType<emblemID_t>( seid );
        fh_emblem     em = et->getEmblemByID( eid );
        d->setEmblem( em );
    }
}

const char* CONTEXTMENU_DATA_GITER    = "contextmenu_data_giter";
const char* CONTEXTMENU_DATA_COLNUM   = "contextmenu_data_colnum";
const char* CONTEXTMENU_DATA_AGENTPTR = "contextmenu_data_agentptr";
const char* CONTEXTMENU_DATA_EID      = "contextmenu_data_eid";


static void context_menu_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
{
    int     colnum = GPOINTER_TO_INT(g_object_get_data( G_OBJECT(menuitem), CONTEXTMENU_DATA_COLNUM ));
    Agent*       d = (Agent*)g_object_get_data( G_OBJECT(menuitem), CONTEXTMENU_DATA_AGENTPTR );
    string       v = tostr( menuitem );
    bool agentChanged = false;
    
    if( colnum == C_ALGO_COLUMN )
    {
        try
        {
            d->setAgentImplemenationName( v );
            agentChanged = true;
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "There is no suitable agent implementation with the name:" << v << endl
               << " Reverting to old value";
            RunErrorDialog( tostr(ss), gtk_window );
        }
    }
    else if( colnum == C_PERS_NAME_COLUMN )
    {
        try
        {
            emblemID_t eid = GPOINTER_TO_INT(g_object_get_data( G_OBJECT(menuitem),
                                                                CONTEXTMENU_DATA_EID ));
            fh_personality pers = obtainPersonality( eid );
            d->setPersonality( pers );
            agentChanged = true;
        }
        catch( NoSuchPersonalityException& e )
        {
            fh_stringstream ss;
            ss << "Can't find the personality:" << v << endl
               << " this is very strange and should never happen!";
            RunErrorDialog( tostr(ss), gtk_window );
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "There is no suitable agent implementation with the name:" << v << endl
               << " Reverting to old value";
            RunErrorDialog( tostr(ss), gtk_window );
        }
    }

    if( agentChanged )
    {
        GtkTreeIter* iter = (GtkTreeIter*)g_object_get_data( G_OBJECT(menuitem), CONTEXTMENU_DATA_GITER );
        put( w_treemodel, *iter, d );
    }
}

static gint treeview_otherclick_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    GtkMenu *menu;
    GdkEventButton *event_button;
    static GdkEventButton treeview_otherclick_pressevent;
    
    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

    if( event->type == GDK_BUTTON_PRESS )
    {
        event_button = (GdkEventButton *) event;
        treeview_otherclick_pressevent = *event_button;
    }
    if( event->type == GDK_BUTTON_RELEASE )
    {
        event_button = (GdkEventButton *) event;
        
        if (event_button->button == 3 || event_button->button == 1)
        {
            if( abs( (long)(treeview_otherclick_pressevent.x - event_button->x) ) > 10  ||
                abs( (long)(treeview_otherclick_pressevent.y - event_button->y) ) > 10 )
            {
                return FALSE;
            }

            GtkWidget* child;
            GtkWidget* m;
            int i=0;

            GtkTreeViewColumn* column = 0;
            GtkTreePath* path = 0;
            
            if( !gtk_tree_view_get_path_at_pos( GTK_TREE_VIEW(w_treeview),
                                                (int)event_button->x,
                                                (int)event_button->y,
                                                &path,
                                                &column,
                                                0, 0 ) || !column )
            {
                return FALSE;
            }

            int colnum = C_ALGO_COLUMN;
            if( column != w_cols[ C_ALGO_COLUMN ] && column != w_cols[ C_PERS_NAME_COLUMN ] )
            {
                gtk_tree_path_free( path );
                return FALSE;
            }

            

            GtkTreeModel *model = GTK_TREE_MODEL (w_treemodel);
            static GtkTreeIter iter;
            gtk_tree_model_get_iter (model, &iter, path);
            Agent* d = (Agent*)treeptr( &iter, w_treeview, C_DIRECTORPTR_COLUMN );
            gtk_tree_path_free (path);
            
            menu = GTK_MENU( gtk_menu_new() );

            if( column == w_cols[ C_ALGO_COLUMN ] )
            {
                colnum = C_ALGO_COLUMN;

                stringlist_t sl = d->getAlternateImplementationNames();
                for( stringlist_t::iterator si = sl.begin(); si!=sl.end(); ++si )
                {
                    string name = *si;
                    
                    child = gtk_menu_item_new_with_label( name.c_str() );
                    gtk_menu_shell_append( GTK_MENU_SHELL(menu), child );

                    g_object_set_data( G_OBJECT(child), CONTEXTMENU_DATA_GITER,  &iter );
                    g_object_set_data( G_OBJECT(child), CONTEXTMENU_DATA_COLNUM, GINT_TO_POINTER(colnum) );
                    g_object_set_data( G_OBJECT(child), CONTEXTMENU_DATA_AGENTPTR, d );
                    gtk_signal_connect( GTK_OBJECT(child), "activate",
                                        G_CALLBACK( context_menu_activate_cb ), 0 );
                }
                
            }
            else if( column == w_cols[ C_PERS_NAME_COLUMN ] )
            {
                colnum = C_PERS_NAME_COLUMN;

                std::list< fh_personality > pl = findAllPersonalities();
                std::list< fh_personality >::iterator pi  = pl.begin();
                std::list< fh_personality >::iterator end = pl.end();
                for( ; pi!=end; ++pi )
                {
                    string name = (*pi)->getName();
                    emblemID_t eid = (*pi)->getID();
                    
                    child = gtk_menu_item_new_with_label( name.c_str() );
                    gtk_menu_shell_append( GTK_MENU_SHELL(menu), child );
                    
                    g_object_set_data( G_OBJECT(child), CONTEXTMENU_DATA_GITER,  &iter );
                    g_object_set_data( G_OBJECT(child), CONTEXTMENU_DATA_COLNUM, GINT_TO_POINTER(colnum) );
                    g_object_set_data( G_OBJECT(child), CONTEXTMENU_DATA_AGENTPTR, d );
                    g_object_set_data( G_OBJECT(child), CONTEXTMENU_DATA_EID, GINT_TO_POINTER(eid) );
                    
                    gtk_signal_connect( GTK_OBJECT(child), "activate",
                                        G_CALLBACK( context_menu_activate_cb ), 0 );
                }
            }
            
//            gtk_menu_shell_append( GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
            showPopup( menu );
                
            /* Tell calling code that we have handled this event; the buck
             * stops here. */
            return TRUE;
        }
    }
    
    
    return FALSE;
}


void make_window()
{
    GtkWidget* lab;
    GtkWidget* slide;

    int c=0;
    int r=0;

    gtk_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title ( GTK_WINDOW( gtk_window ), "Ferris agents" );
    gtk_window_set_default_size (GTK_WINDOW (gtk_window), 900, 550);
    gtk_signal_connect(GTK_OBJECT (gtk_window), "destroy", gtk_main_quit, NULL);
    GtkNotebook* main_widget = GTK_NOTEBOOK(gtk_notebook_new());

    GtkPaned* paned = GTK_PANED(gtk_hpaned_new());
    GtkWidget* tree_frame    = gtk_frame_new (NULL);
    GtkWidget* details_frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (tree_frame),    GTK_SHADOW_IN);
    gtk_frame_set_shadow_type (GTK_FRAME (details_frame), GTK_SHADOW_IN);
    gtk_paned_pack1 (GTK_PANED (paned), tree_frame,    true, false );
    gtk_paned_pack2 (GTK_PANED (paned), details_frame, true, false );
    gtk_frame_set_label( GTK_FRAME (details_frame), "emblem(s) to assign" );
    gtk_notebook_append_page( main_widget, GTK_WIDGET(paned), gtk_label_new( "instances" ));
    

    {
        GtkToolbar* tb = GTK_TOOLBAR(gtk_toolbar_new());
        GtkWidget* vbx = GTK_WIDGET(gtk_vbox_new(0,0));
        GtkWidget* hbx = GTK_WIDGET(gtk_hbox_new(0,0));
        GtkWidget* b;
        GtkCellRenderer* ren;
        int colnum = 0;

        b = gtk_toolbar_append_item( tb, "New",
                                     "create a new agent", "",
                                     gtk_image_new_from_stock(
                                         GTK_STOCK_NEW,
                                         GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                     GTK_SIGNAL_FUNC(new_cb),
                                     0 );
        b = gtk_toolbar_append_item( tb, "Delete",
                                     "delete selected agent(s)", "",
                                     gtk_image_new_from_stock(
                                         GTK_STOCK_DELETE,
                                         GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                     GTK_SIGNAL_FUNC(del_cb),
                                     0 );

        gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( tb ));
        gtk_box_pack_start(GTK_BOX(vbx), hbx, 0, 0, 0 );

        w_treemodel = gtk_tree_store_new( C_COLUMN_COUNT,
                                          G_TYPE_STRING, 
                                          G_TYPE_STRING, G_TYPE_STRING,
                                          G_TYPE_STRING, G_TYPE_STRING,
                                          G_TYPE_POINTER );
        w_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL( w_treemodel ));

        gtk_signal_connect(GTK_OBJECT(w_treeview), "button_press_event",
                           GTK_SIGNAL_FUNC (treeview_otherclick_cb), 0 );
        gtk_signal_connect(GTK_OBJECT(w_treeview), "button_release_event",
                           GTK_SIGNAL_FUNC (treeview_otherclick_cb), 0 );
        
        
        GtkWidget* sw;
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_container_add( GTK_CONTAINER( sw ), GTK_WIDGET( w_treeview ));
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);
        gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(sw), 1, 1, 0 );
        gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (w_treeview), TRUE);
        GObject *selection;
        selection = G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (w_treeview)));
//        gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_MULTIPLE);
        gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW (w_treeview), true );
        gtk_tree_view_set_enable_search(GTK_TREE_VIEW (w_treeview), true );
        gtk_tree_view_set_search_column(GTK_TREE_VIEW (w_treeview), C_AGENTNAME_COLUMN );
        g_signal_connect_data( G_OBJECT( selection ), "changed",
                               G_CALLBACK (changed_cb), 0, 0, GConnectFlags(0));
//        gtk_tree_view_set_reorderable( GTK_TREE_VIEW( w_treeview ), true );

        
        gtk_container_add( GTK_CONTAINER( tree_frame ), vbx );


        colnum = C_AGENTNAME_COLUMN;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "edited",
                               G_CALLBACK (edited_cb),
                               GINT_TO_POINTER(colnum), 0,
                               GConnectFlags(0));
        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "agent-name", ren,
                                                                     "text", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );
        gtk_tree_sortable_set_sort_column_id( GTK_TREE_SORTABLE( w_treemodel ),
                                              colnum, GTK_SORT_ASCENDING );

        colnum = C_STATEDIR_COLUMN;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "edited",
                               G_CALLBACK (edited_cb),
                               GINT_TO_POINTER(colnum), 0,
                               GConnectFlags(0));
        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "state dir", ren,
                                                                     "text", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );

        colnum = C_ALGO_COLUMN;
        ren = gtk_cell_renderer_text_new ();
//         g_object_set(ren, "editable", 1, 0 );
//         g_signal_connect_data( G_OBJECT( ren ), "edited",
//                                G_CALLBACK (edited_cb),
//                                GINT_TO_POINTER(colnum), 0,
//                                GConnectFlags(0));
        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "implemenation name", ren,
                                                                     "text", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );

//         colnum = C_PERS_ID_COLUMN;
//         ren = gtk_cell_renderer_text_new ();
//         g_object_set(ren, "editable", 0, 0 );
//         w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "personality-id to use", ren,
//                                                                      "text", colnum,
//                                                                      NULL);
//         gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ colnum ] );
//         gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );

        colnum = C_PERS_NAME_COLUMN;
        ren = gtk_cell_renderer_text_new ();
//        g_object_set(ren, "editable", 1, 0 );
//         g_signal_connect_data( G_OBJECT( ren ), "edited",
//                                G_CALLBACK (edited_cb),
//                                GINT_TO_POINTER(colnum), 0,
//                                GConnectFlags(0));
        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "personality to use", ren,
                                                                     "text", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );
    }


    //
    // Add the "details" section for editing which emblems the agents are to use
    //
    {
        GtkWidget* w = 0;
        GtkCellRenderer* ren;
        int colnum = 0;
        
        
        gtk_table = gtk_table_new ( 1, 2, false );
        gtk_container_add( GTK_CONTAINER( details_frame ), GTK_WIDGET(gtk_table) );

        r=0; c=0;

        w_emblems_treemodel = gtk_tree_store_new( CE_COLUMN_COUNT,
                                                  G_TYPE_BOOLEAN,
                                                  G_TYPE_STRING, G_TYPE_STRING );
        w_emblems_treeview  = gtk_tree_view_new_with_model(GTK_TREE_MODEL( w_emblems_treemodel )); 
        
        GtkWidget* sw;
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_container_add( GTK_CONTAINER( sw ), GTK_WIDGET( w_emblems_treeview ));
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

        colnum = CE_CHECKED_COLUMN;
        ren = gtk_cell_renderer_toggle_new();
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "toggled",
                               G_CALLBACK (emblems_edited_cb),
                               GINT_TO_POINTER(colnum), 0,
                               GConnectFlags(0));
        w_emblems_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "is-parent", ren,
                                                                             "active", colnum,
                                                                             NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_emblems_treeview ), w_emblems_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_emblems_cols[ colnum ], colnum );


        colnum = CE_ID_COLUMN;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 0, 0 );
        w_emblems_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "emblem-id", ren,
                                                                             "text", colnum,
                                                                             NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_emblems_treeview ), w_emblems_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_emblems_cols[ colnum ], colnum );

        
        colnum = CE_NAME_COLUMN;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 0, 0 );
        w_emblems_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "emblem", ren,
                                                                             "text", colnum,
                                                                             NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_emblems_treeview ), w_emblems_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_emblems_cols[ colnum ], colnum );
        
        
        gtk_table_attach_defaults(GTK_TABLE(gtk_table), sw, c+0, c+2, r, r+1 );
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
    
//    gtk_box_pack_start(GTK_BOX(vbx), gtk_scrolledwindow, 1, 1, 0 );
    gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(main_widget), 1, 1, 0 );
    gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(hbx), 0, 0, 0 );
    
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
        poptSetOtherOptionHelp(optCon, "[OPTIONS]*  ...");

        if (argc < 1) {
//          poptPrintHelp(optCon, stderr, 0);
            poptPrintUsage(optCon, stderr, 0);
            exit(1);
        }


        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {
        }

        et = Factory::getEtagere();
        
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

