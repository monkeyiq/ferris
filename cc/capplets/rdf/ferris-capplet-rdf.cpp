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

    $Id: ferris-capplet-rdf.cpp,v 1.4 2010/09/24 21:31:22 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <glib.h>
#include <gtk/gtk.h>

#include <Ferris/Ferris.hh>
#include <Ferris/Medallion.hh>
#include <Ferris/FerrisSemantic.hh>
#include <Ferris/Regex.hh>
#include <FerrisUI/GtkFerris.hh>
#include <FerrisUI/EditStringList.hh>
#include <FerrisUI/Menus.hh>

#include <string>
#include <iomanip>

using namespace std;
using namespace Ferris;
using namespace Ferris::Semantic;
using namespace FerrisUI;

 
const string PROGRAM_NAME = "ferris-capplet-rdf";

GtkWidget* gtk_window;
GtkWidget* gtk_scrolledwindow;
GtkWidget* gtk_table;

//
// main tree
//
enum {

    C_SMUSH_SET_NAME_COLUMN=0,
    C_SMUSH_SET_URL_COLUMN,
    C_SMUSH_SET_COLUMN_COUNT
};
GtkTreeStore*      w_smushset_treemodel;
GtkWidget*         w_smushset_treeview;
GtkTreeViewColumn* w_smushset_cols[C_SMUSH_SET_COLUMN_COUNT];

static fh_editstringlist m_RDFCacheAttrsList = new EditStringList();
GtkWidget* w_RDFCacheAttrsEnabled = 0;

GtkWidget* w_GlobalSmushGroupLeaderRegex = 0;


GtkTreeIter
appendNewBlankItem( GtkTreeIter* parent = 0 )
{
    static int counter = 0;
    GtkTreeIter iter;
    stringstream ss;
    ss << "new-" << (++counter);
    string d = ss.str();

    static int childnum = 0;
    stringstream css;
    css << ""; // "child-" << (++childnum);
    string cd = css.str();
    
    gtk_tree_store_append( w_smushset_treemodel, &iter, parent );
    gtk_tree_store_set( w_smushset_treemodel, &iter,
                        C_SMUSH_SET_NAME_COLUMN, parent ? cd.c_str() : d.c_str(),
                        C_SMUSH_SET_URL_COLUMN, "",
                        -1 );

    return iter;
}




void put( GtkTreeStore* ts,
          GtkTreeIter& giter,
          const std::string& smushsetname )
{
    gtk_tree_store_set( ts, &giter,
                        C_SMUSH_SET_NAME_COLUMN, smushsetname.c_str(),
                        C_SMUSH_SET_URL_COLUMN, "",
                        -1 );
}

// /**
//  * This finds a child in the w_smushset_treemodel list by eid only checking in the
//  * range starting at giter and not going into children. Note that giter
//  * should be set to a valid iterator in the treemodel at the level where
//  * the emblem you seek should be findable.
//  *
//  * return true if the emblem was found and giter points to it at return
//  */
// bool getChildByID( emblemID_t eid, GtkTreeIter& giter )
// {
// //     if( gtk_tree_model_get_iter_first( GTK_TREE_MODEL(w_smushset_treemodel), &giter ) )
// //     {
//         while( true )
//         {
//             string id = treestr( &giter, w_smushset_treeview, C_EMBLEM_ID_COLUMN );
//             if( id == tostr(eid) )
//             {
//                 return true;
//             }
//             if( !gtk_tree_model_iter_next( GTK_TREE_MODEL(w_smushset_treemodel), &giter ) )
//                 break;
//         }
// //    }
//     return false;
// }

// bool childExists( emblemID_t eid, GtkTreeIter giter )
// {
//     GtkTreeIter piter = giter;
//     bool rc = gtk_tree_model_iter_nth_child( GTK_TREE_MODEL( w_smushset_treemodel ),
//                                              &giter,
//                                              &piter,
//                                              0 );
//     return rc && getChildByID( eid, giter );
// }



fh_TreeSmushing m_TreeSmushing = 0;

void SaveSmush( fh_TreeSmushing newTree, fh_SmushSet ss, GtkTreeIter* piter )
{
    GtkTreeIter giter;

    if( gtk_tree_model_iter_children( GTK_TREE_MODEL(w_smushset_treemodel), &giter, piter ))
    {
        SmushSet::m_smushes_t v;
        while( true )
        {
            string earl = treestr( &giter, w_smushset_treeview, C_SMUSH_SET_URL_COLUMN );
            
            v.insert( make_pair( earl, new Regex( earl )));
            
            if( !gtk_tree_model_iter_next( GTK_TREE_MODEL(w_smushset_treemodel), &giter ) )
                break;
        }
        ss->setSmushes( v );
    }
}


void SaveData()
{
    setEDBString( FDB_GENERAL,
                  CFG_RDF_GLOBAL_SMUSH_GROUP_LEADER_K,
                  tostr( GTK_ENTRY(w_GlobalSmushGroupLeaderRegex)));
    
    fh_TreeSmushing newTree = new TreeSmushing();
    GtkTreeIter giter;

//     C_SMUSH_SET_NAME_COLUMN=0,
//     C_SMUSH_SET_URL_COLUMN,
    
     if( gtk_tree_model_get_iter_first( GTK_TREE_MODEL(w_smushset_treemodel), &giter ) )
     {
         while( true )
         {
             string name = treestr( &giter, w_smushset_treeview, C_SMUSH_SET_NAME_COLUMN );

             fh_SmushSet ss = newTree->newSmush( name );
             SaveSmush( newTree, ss, &giter );
             
             if( !gtk_tree_model_iter_next( GTK_TREE_MODEL(w_smushset_treemodel), &giter ) )
                 break;
         }
     }
    
    
    m_TreeSmushing->swap( newTree );
    m_TreeSmushing->sync();


    setEDBString( FDB_GENERAL, CFG_RDFCACHE_ATTRS_ENABLED_K,
                  tostr(gtk_toggle_button_get_active(
                            GTK_TOGGLE_BUTTON(w_RDFCacheAttrsEnabled))));
    
    {
        stringlist_t sl = m_RDFCacheAttrsList->getStringList();
        string d = Util::createCommaSeperatedList( sl );
        setEDBString( FDB_GENERAL, CFG_RDFCACHE_ATTRS_LIST_K, d );
    }
    
}



void LoadData()
{
    m_TreeSmushing = getDefaultImplicitTreeSmushing();
//     {
//         fh_SmushSet ss = m_TreeSmushing->newSmush( "foo" );
//         SmushSet::m_smushes_t v;
//         v.insert( make_pair( "/aaaaa", new Regex( "/aaaaaaaa" )));
//         ss->setSmushes( v );
//     }
//     {
//         fh_SmushSet ss = m_TreeSmushing->newSmush( "bar" );
//         SmushSet::m_smushes_t v;
//         v.insert( make_pair( "/bb", new Regex( "/bbb" )));
//         v.insert( make_pair( "/b2b", new Regex( "/b2bb" )));
//         ss->setSmushes( v );
//     }
    


    const TreeSmushing::m_smushSets_t& a = m_TreeSmushing->getAll();
    TreeSmushing::m_smushSets_t::const_iterator ai = a.begin();
    TreeSmushing::m_smushSets_t::const_iterator ae = a.end();
    
    for( ; ai != ae; ++ai )
    {
        string name = ai->first;
        GtkTreeIter iter = appendNewBlankItem( 0 );
        put( w_smushset_treemodel, iter, name );

//        typedef std::map< std::string, fh_regex > m_smushes_t;
        const SmushSet::m_smushes_t& sm = ai->second->getSmushes();
        SmushSet::m_smushes_t::const_iterator si = sm.begin();
        SmushSet::m_smushes_t::const_iterator se = sm.end();
        for( ; si != se; ++si )
        {
            GtkTreeIter siter = appendNewBlankItem( &iter );
            gtk_tree_store_set( w_smushset_treemodel, &siter,
                                C_SMUSH_SET_NAME_COLUMN, "",
                                C_SMUSH_SET_URL_COLUMN, si->first.c_str(),
                                -1 );
        }
    }
    
    gtk_tree_view_expand_all( GTK_TREE_VIEW( w_smushset_treeview ) );


    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(w_RDFCacheAttrsEnabled),
        isTrue( getEDBString( FDB_GENERAL,
                              CFG_RDFCACHE_ATTRS_ENABLED_K,
                              CFG_RDFCACHE_ATTRS_ENABLED_DEFAULT )));

    {
        string d = getEDBString( FDB_GENERAL,
                                 CFG_RDFCACHE_ATTRS_LIST_K,
                                 CFG_RDFCACHE_ATTRS_LIST_DEFAULT );
        stringlist_t sl;
        Util::parseCommaSeperatedList( d, sl );
        m_RDFCacheAttrsList->setStringList( sl );
    }
    
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
    GtkTreeView*  tv = GTK_TREE_VIEW(  w_smushset_treeview  );
    GtkTreeModel* tm = GTK_TREE_MODEL( w_smushset_treemodel );

    GtkTreeIter titer;
    GtkTreeSelection *sel = gtk_tree_view_get_selection ( tv );
    cerr << "selected:" << gtk_tree_selection_get_selected( sel, 0, 0 ) << endl;
    bool selected = gtk_tree_selection_get_selected ( sel, 0, &titer );
    if( selected )
    {
        while( true )
        {
            GtkTreeIter np;
            gboolean rc = gtk_tree_model_iter_parent( tm, &np, &titer );
            if( !rc )
                break;
            
            titer = np;
        }
    }
    GtkTreeIter iter = appendNewBlankItem( selected ? &titer : 0 );

//     GtkTreePath* path      = gtk_tree_model_get_path( tm, &iter );
//     GtkTreeViewColumn* col = gtk_tree_view_get_column( tv, C_SMUSH_SET_NAME_COLUMN );
//     gtk_tree_view_set_cursor( tv, path, col, true );
//     gtk_tree_path_free( path );
}

void
new_smush_cb( GtkButton *button, gpointer )
{
    GtkTreeView*  tv = GTK_TREE_VIEW(  w_smushset_treeview  );
    GtkTreeModel* tm = GTK_TREE_MODEL( w_smushset_treemodel );
    GtkTreeIter iter = appendNewBlankItem( 0 );
}


void
del_cb( GtkButton *button, gpointer )
{
    list_gtktreeiter_t deliters = getIterList( w_smushset_treeview, true );
    GtkTreeView*  tv = GTK_TREE_VIEW(w_smushset_treeview);
    GtkTreeModel* tm = GTK_TREE_MODEL(w_smushset_treemodel);
    GtkTreeStore* ts = GTK_TREE_STORE(w_smushset_treemodel);
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection ( tv );
    gtk_tree_selection_unselect_all( selection );

    for( list_gtktreeiter_t::iterator iter = deliters.begin(); iter != deliters.end(); ++iter )
    {
        GtkTreeIter giter = *iter;
        gtk_tree_store_remove( ts, &giter );
    }
}


void
edited_cb (GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer user_data )
{
    string v = new_text ? new_text : "";
    int cidx = GPOINTER_TO_INT(user_data);
    cerr << "edited_cb() c:" << cidx << " new_text:" << new_text << endl;

    bool acceptNewValue = true;
    GtkTreeModel *model = GTK_TREE_MODEL (w_smushset_treemodel);
    GtkTreeIter iter;
    GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
    gtk_tree_model_get_iter (model, &iter, path);
    GtkTreeIter piter;
    gboolean haveParent = gtk_tree_model_iter_parent( model, &piter, &iter );

    if( cidx == C_SMUSH_SET_NAME_COLUMN )
    {
        if( haveParent )
            acceptNewValue = false;
    }
    if( cidx == C_SMUSH_SET_URL_COLUMN )
    {
        if( !haveParent )
            acceptNewValue = false;
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
emblems_edited_cb (GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer user_data )
{
//     int cidx = (int)CE_CHECKED_COLUMN;

//     GtkTreeIter iter;
//     GtkTreeSelection *sel = gtk_tree_view_get_selection ( GTK_TREE_VIEW(w_smushset_treeview) );
//     GtkTreeModel* model = GTK_TREE_MODEL( w_smushset_treemodel );
//     gtk_tree_selection_get_selected ( sel, &model, &iter );
//     Agent* d = (Agent*)treeptr( &iter, w_smushset_treeview, C_DIRECTORPTR_COLUMN );
    
//     if( d )
//     {
//         GtkTreeModel *model = GTK_TREE_MODEL (w_urls_treemodel);
//         GtkTreeIter iter;
//         GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
//         gtk_tree_model_get_iter (model, &iter, path);

//         gboolean value;
//         gtk_tree_model_get (model, &iter, cidx, &value, -1);
//         value = !value;
//         gtk_tree_store_set (GTK_TREE_STORE (model), &iter, cidx, value, -1);
//         gtk_tree_path_free (path);

//         std::string seid = treestr( &iter, GTK_WIDGET( w_urls_treeview ), CE_ID_COLUMN );
//         emblemID_t   eid = toType<emblemID_t>( seid );
//         fh_emblem     em = et->getEmblemByID( eid );
//         d->setEmblem( em );
//     }
}

const char* CONTEXTMENU_DATA_GITER    = "contextmenu_data_giter";
const char* CONTEXTMENU_DATA_COLNUM   = "contextmenu_data_colnum";
const char* CONTEXTMENU_DATA_AGENTPTR = "contextmenu_data_agentptr";
const char* CONTEXTMENU_DATA_EID      = "contextmenu_data_eid";


static void context_menu_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
{
//     int     colnum = GPOINTER_TO_INT(g_object_get_data( G_OBJECT(menuitem), CONTEXTMENU_DATA_COLNUM ));
//     Agent*       d = (Agent*)g_object_get_data( G_OBJECT(menuitem), CONTEXTMENU_DATA_AGENTPTR );
//     string       v = tostr( menuitem );
//     bool agentChanged = false;
    
//     if( colnum == C_ALGO_COLUMN )
//     {
//         try
//         {
//             d->setAgentImplemenationName( v );
//             agentChanged = true;
//         }
//         catch( exception& e )
//         {
//             fh_stringstream ss;
//             ss << "There is no suitable agent implementation with the name:" << v << endl
//                << " Reverting to old value";
//             RunErrorDialog( tostr(ss), gtk_window );
//         }
//     }
//     else if( colnum == C_PERS_NAME_COLUMN )
//     {
//         try
//         {
//             emblemID_t eid = GPOINTER_TO_INT(g_object_get_data( G_OBJECT(menuitem),
//                                                                 CONTEXTMENU_DATA_EID ));
//             fh_personality pers = obtainPersonality( eid );
//             d->setPersonality( pers );
//             agentChanged = true;
//         }
//         catch( NoSuchPersonalityException& e )
//         {
//             fh_stringstream ss;
//             ss << "Can't find the personality:" << v << endl
//                << " this is very strange and should never happen!";
//             RunErrorDialog( tostr(ss), gtk_window );
//         }
//         catch( exception& e )
//         {
//             fh_stringstream ss;
//             ss << "There is no suitable agent implementation with the name:" << v << endl
//                << " Reverting to old value";
//             RunErrorDialog( tostr(ss), gtk_window );
//         }
//     }

//     if( agentChanged )
//     {
//         GtkTreeIter* iter = (GtkTreeIter*)g_object_get_data( G_OBJECT(menuitem), CONTEXTMENU_DATA_GITER );
//         put( w_smushset_treemodel, *iter, d );
//     }
}

static gint treeview_otherclick_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    GtkMenu *menu;
    GdkEventButton *event_button;
    static GdkEventButton treeview_otherclick_pressevent;
    
    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

//     if( event->type == GDK_BUTTON_PRESS )
//     {
//         event_button = (GdkEventButton *) event;
//         treeview_otherclick_pressevent = *event_button;
//     }
//     if( event->type == GDK_BUTTON_RELEASE )
//     {
//         event_button = (GdkEventButton *) event;
        
//         if (event_button->button == 3 || event_button->button == 1)
//         {
//             if( abs( (long)(treeview_otherclick_pressevent.x - event_button->x) ) > 10  ||
//                 abs( (long)(treeview_otherclick_pressevent.y - event_button->y) ) > 10 )
//             {
//                 return FALSE;
//             }

//             GtkWidget* child;
//             GtkWidget* m;
//             int i=0;

//             GtkTreeViewColumn* column = 0;
//             GtkTreePath* path = 0;
            
//             if( !gtk_tree_view_get_path_at_pos( GTK_TREE_VIEW(w_smushset_treeview),
//                                                 (int)event_button->x,
//                                                 (int)event_button->y,
//                                                 &path,
//                                                 &column,
//                                                 0, 0 ) || !column )
//             {
//                 return FALSE;
//             }

//             int colnum = C_ALGO_COLUMN;
//             if( column != w_smushset_cols[ C_ALGO_COLUMN ] && column != w_smushset_cols[ C_PERS_NAME_COLUMN ] )
//             {
//                 gtk_tree_path_free( path );
//                 return FALSE;
//             }

            

//             GtkTreeModel *model = GTK_TREE_MODEL (w_smushset_treemodel);
//             static GtkTreeIter iter;
//             gtk_tree_model_get_iter (model, &iter, path);
//             Agent* d = (Agent*)treeptr( &iter, w_smushset_treeview, C_DIRECTORPTR_COLUMN );
//             gtk_tree_path_free (path);
            
//             menu = GTK_MENU( gtk_menu_new() );

//             if( column == w_smushset_cols[ C_ALGO_COLUMN ] )
//             {
//                 colnum = C_ALGO_COLUMN;

//                 stringlist_t sl = d->getAlternateImplementationNames();
//                 for( stringlist_t::iterator si = sl.begin(); si!=sl.end(); ++si )
//                 {
//                     string name = *si;
                    
//                     child = gtk_menu_item_new_with_label( name.c_str() );
//                     gtk_menu_shell_append( GTK_MENU_SHELL(menu), child );

//                     g_object_set_data( G_OBJECT(child), CONTEXTMENU_DATA_GITER,  &iter );
//                     g_object_set_data( G_OBJECT(child), CONTEXTMENU_DATA_COLNUM, GINT_TO_POINTER(colnum) );
//                     g_object_set_data( G_OBJECT(child), CONTEXTMENU_DATA_AGENTPTR, d );
//                     gtk_signal_connect( GTK_OBJECT(child), "activate",
//                                         G_CALLBACK( context_menu_activate_cb ), 0 );
//                 }
                
//             }
//             else if( column == w_smushset_cols[ C_PERS_NAME_COLUMN ] )
//             {
//                 colnum = C_PERS_NAME_COLUMN;

//                 std::list< fh_personality > pl = findAllPersonalities();
//                 std::list< fh_personality >::iterator pi  = pl.begin();
//                 std::list< fh_personality >::iterator end = pl.end();
//                 for( ; pi!=end; ++pi )
//                 {
//                     string name = (*pi)->getName();
//                     emblemID_t eid = (*pi)->getID();
                    
//                     child = gtk_menu_item_new_with_label( name.c_str() );
//                     gtk_menu_shell_append( GTK_MENU_SHELL(menu), child );
                    
//                     g_object_set_data( G_OBJECT(child), CONTEXTMENU_DATA_GITER,  &iter );
//                     g_object_set_data( G_OBJECT(child), CONTEXTMENU_DATA_COLNUM, GINT_TO_POINTER(colnum) );
//                     g_object_set_data( G_OBJECT(child), CONTEXTMENU_DATA_AGENTPTR, d );
//                     g_object_set_data( G_OBJECT(child), CONTEXTMENU_DATA_EID, GINT_TO_POINTER(eid) );
                    
//                     gtk_signal_connect( GTK_OBJECT(child), "activate",
//                                         G_CALLBACK( context_menu_activate_cb ), 0 );
//                 }
//             }
            
// //            gtk_menu_shell_append( GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
//             showPopup( menu );
                
//             /* Tell calling code that we have handled this event; the buck
//              * stops here. */
//             return TRUE;
//         }
//     }
    
    
    return FALSE;
}


void make_window()
{
    GtkWidget* lab;
    GtkWidget* slide;

    int c=0;
    int r=0;

    gtk_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title ( GTK_WINDOW( gtk_window ), "Ferris RDF" );
    gtk_window_set_default_size (GTK_WINDOW (gtk_window), 900, 550);
    gtk_signal_connect(GTK_OBJECT (gtk_window), "destroy", gtk_main_quit, NULL);
    GtkNotebook* main_widget = GTK_NOTEBOOK(gtk_notebook_new());

    {
        GtkWidget* vbx = GTK_WIDGET(gtk_vbox_new(0,0));
        GtkWidget* w = 0;

        ////////////////////
        
        gtk_table = gtk_table_new ( 1, 2, false );
        gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(gtk_table), 0, 0, 0 );
        r = c = 0;
        gtk_table_attach_defaults(GTK_TABLE(gtk_table),
                                  gtk_label_new( "Global Smush Group Leader regex" ),
                                  0, 1, r, r+1 );
        w = w_GlobalSmushGroupLeaderRegex = gtk_entry_new();
        gtk_entry_set_text( GTK_ENTRY(w),
                            getEDBString( FDB_GENERAL,
                                          CFG_RDF_GLOBAL_SMUSH_GROUP_LEADER_K,
                                          CFG_RDF_GLOBAL_SMUSH_GROUP_LEADER_DEFAULT ).c_str());
        gtk_table_attach_defaults(GTK_TABLE(gtk_table), w, 1, 2, r, r+1 );


        ///////////////////

        
        GtkToolbar* tb = GTK_TOOLBAR(gtk_toolbar_new());
        GtkWidget* hbx = GTK_WIDGET(gtk_hbox_new(0,0));
        GtkWidget* b;
        GtkCellRenderer* ren;
        int colnum = 0;

        b = gtk_toolbar_append_item( tb, "New",
                                     "create a new smush url", "",
                                     gtk_image_new_from_stock(
                                         GTK_STOCK_NEW,
                                         GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                     GTK_SIGNAL_FUNC(new_cb),
                                     0 );
        b = gtk_toolbar_append_item( tb, "New Smush",
                                     "create a new smush", "",
                                     gtk_image_new_from_stock(
                                         GTK_STOCK_NEW,
                                         GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                     GTK_SIGNAL_FUNC(new_smush_cb),
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

        w_smushset_treemodel = gtk_tree_store_new( C_SMUSH_SET_COLUMN_COUNT, G_TYPE_STRING, G_TYPE_STRING );
        w_smushset_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL( w_smushset_treemodel ));

        gtk_signal_connect(GTK_OBJECT(w_smushset_treeview), "button_press_event",
                           GTK_SIGNAL_FUNC (treeview_otherclick_cb), 0 );
        gtk_signal_connect(GTK_OBJECT(w_smushset_treeview), "button_release_event",
                           GTK_SIGNAL_FUNC (treeview_otherclick_cb), 0 );
        
        GtkWidget* sw;
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_container_add( GTK_CONTAINER( sw ), GTK_WIDGET( w_smushset_treeview ));
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);
        gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(sw), 1, 1, 0 );
        gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (w_smushset_treeview), TRUE);
        GObject *selection;
        selection = G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (w_smushset_treeview)));
//        gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_MULTIPLE);
        gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW (w_smushset_treeview), true );
        gtk_tree_view_set_enable_search(GTK_TREE_VIEW (w_smushset_treeview), true );
        gtk_tree_view_set_search_column(GTK_TREE_VIEW (w_smushset_treeview), C_SMUSH_SET_NAME_COLUMN );
//        gtk_tree_view_set_reorderable( GTK_TREE_VIEW( w_smushset_treeview ), true );

        
        gtk_notebook_append_page( main_widget, GTK_WIDGET(vbx), gtk_label_new( "Tree Smush Sets" ));


        colnum = C_SMUSH_SET_NAME_COLUMN;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "edited",
                               G_CALLBACK (edited_cb),
                               GINT_TO_POINTER(colnum), 0,
                               GConnectFlags(0));
        w_smushset_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "smush-set-name", ren,
                                                                     "text", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_smushset_treeview ), w_smushset_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_smushset_cols[ colnum ], colnum );
        gtk_tree_sortable_set_sort_column_id( GTK_TREE_SORTABLE( w_smushset_treemodel ),
                                              colnum, GTK_SORT_ASCENDING );

        colnum = C_SMUSH_SET_URL_COLUMN;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "edited",
                               G_CALLBACK (edited_cb),
                               GINT_TO_POINTER(colnum), 0,
                               GConnectFlags(0));
        w_smushset_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "url", ren,
                                                                     "text", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_smushset_treeview ), w_smushset_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_smushset_cols[ colnum ], colnum );
        gtk_tree_sortable_set_sort_column_id( GTK_TREE_SORTABLE( w_smushset_treemodel ),
                                              colnum, GTK_SORT_ASCENDING );

    }
    
    {
        GtkWidget* w = 0;

        gtk_table = gtk_table_new ( 1, 2, false );
        gtk_notebook_append_page( main_widget,
                                  GTK_WIDGET(gtk_table),
                                  gtk_label_new( "RDF Cached Attributes" ));

        r=0; c=0;

        r=0;
        w = w_RDFCacheAttrsEnabled
            = gtk_check_button_new_with_label(
                "Enable Caching of some attributes\n"
                "in a temporary RDF store." );
        gtk_table_attach_defaults(GTK_TABLE(gtk_table), w, c, c+2, r, r+1 );
        ++r; c=0;
        
        m_RDFCacheAttrsList->setColumnLabel( "Extended Attributes" );
        m_RDFCacheAttrsList->setDescriptionLabel( "Names of stateless EA which should\n"
                                                  "be cached for file:// URLs" );
//         gtk_notebook_append_page( main_widget,
//                                   m_RDFCacheAttrsList->getWidget(),
//                                   gtk_label_new( "RDF Cached Attributes" ));
        gtk_table_attach(GTK_TABLE(gtk_table), m_RDFCacheAttrsList->getWidget(),
                         c+0, c+2, r, r+1,
                         GtkAttachOptions(GTK_FILL|GTK_SHRINK|GTK_EXPAND),
                         GtkAttachOptions(GTK_FILL|GTK_SHRINK|GTK_EXPAND),
                         0, 2 );
                                  
    }
    

//     {
//         GtkWidget* w = 0;
//         GtkCellRenderer* ren;
//         int colnum = 0;
        
        
//         gtk_table = gtk_table_new ( 1, 2, false );
//         gtk_notebook_append_page( main_widget,
//                                   GTK_WIDGET(gtk_table),
//                                   gtk_label_new( "Options" ));
        

//         r=0; c=0;

//         w_urls_treemodel = gtk_tree_store_new( CSS_COLUMN_COUNT, G_TYPE_STRING );
//         w_urls_treeview  = gtk_tree_view_new_with_model(GTK_TREE_MODEL( w_urls_treemodel )); 
        
//         GtkWidget* sw;
//         sw = gtk_scrolled_window_new(NULL, NULL);
//         gtk_container_add( GTK_CONTAINER( sw ), GTK_WIDGET( w_urls_treeview ));
//         gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
//                                         GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

//         colnum = CSS_PATH_COLUMN;
//         ren = gtk_cell_renderer_toggle_new();
//         g_object_set(ren, "editable", 1, 0 );
//         g_signal_connect_data( G_OBJECT( ren ), "toggled",
//                                G_CALLBACK (emblems_edited_cb),
//                                GINT_TO_POINTER(colnum), 0,
//                                GConnectFlags(0));
//         w_urls_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "url", ren,
//                                                                              "active", colnum,
//                                                                              NULL);
//         gtk_tree_view_append_column( GTK_TREE_VIEW( w_urls_treeview ), w_urls_cols[ colnum ] );
//         gtk_tree_view_column_set_sort_column_id( w_urls_cols[ colnum ], colnum );

        
        
        
//         gtk_table_attach_defaults(GTK_TABLE(gtk_table), sw, c+0, c+2, r, r+1 );
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

