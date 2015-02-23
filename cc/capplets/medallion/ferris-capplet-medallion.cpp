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

    $Id: ferris-capplet-medallion.cpp,v 1.6 2010/09/24 21:31:22 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <glib.h>
#include <gtk/gtk.h>

#include <Ferris/Ferris.hh>
#include <Ferris/Medallion.hh>
#include <Ferris/Medallion_private.hh>
#include <FerrisUI/GtkFerris.hh>
#include <FerrisUI/EditStringList.hh>
#include <FerrisUI/Menus.hh>

#include <string>
#include <iomanip>

using namespace std;
using namespace Ferris;
using namespace FerrisUI;

 
const string PROGRAM_NAME = "ferris-capplet-medallion";

GtkWidget* gtk_window;
GtkWidget* gtk_scrolledwindow;
GtkWidget* gtk_table;
GtkWidget* w_eanames_ignore;
static fh_editstringlist m_eanamesRegexIgnoreList = new EditStringList();
GtkWidget* w_max_value_size;



//
// main tree
//
enum {
    C_EMBLEM_ID_COLUMN=0,
    C_EMBLEM_NAME_COLUMN,
    C_EMBLEM_ICONNAME_COLUMN,
    C_EMBLEM_LIMITEDPRI_COLUMN,
    C_EMBLEM_DESC_COLUMN,
    C_EMBLEM_DLAT,
    C_EMBLEM_DLONG,
    C_EMBLEM_DRANGE,
    C_COLUMN_COUNT
};
GtkTreeStore*      w_treemodel;
GtkWidget*         w_treeview;
GtkTreeViewColumn* w_cols[C_COLUMN_COUNT];


//
// parents list
//
enum {
    CP_CHECKED_COLUMN=0,
    CP_NAME_COLUMN,
    CP_COLUMN_COUNT
};
GtkTreeStore*      w_parents_treemodel;
GtkWidget*         w_parents_treeview;
GtkTreeViewColumn* w_parents_cols[CP_COLUMN_COUNT];

//
// icon page
//
GtkWidget*         w_icontable;
const int          m_numberOfIconsAcross = 4;


//
// global settings page
//
GtkWidget*         w_globaltable = 0;
GtkWidget*         w_lowestEmblemPriToShow = 0;

fh_etagere et = 0;

void OnEmblemChildAdded( fh_emblem pe, fh_emblem em );
emblemID_t getCurrentSelectedEmblemID();


GtkTreeIter
appendNewBlankItem( GtkTreeIter* parent = 0 )
{
    GtkTreeIter iter;
    const char* d = "<new>";
    gtk_tree_store_append( w_treemodel, &iter, parent );
    gtk_tree_store_set( w_treemodel, &iter,
                        C_EMBLEM_NAME_COLUMN, d,
                        -1 );
    return iter;
}

/**
 * Get giters to all the entries in the treeview with the given id
 */
list_gtktreeiter_t
findAllByID( emblemID_t eid )
{
    string seid = tostr(eid);
    
    list_gtktreeiter_t ret;
    list_gtktreeiter_t all = getIterList( w_treeview, false );
    for( list_gtktreeiter_t::iterator li = all.begin(); li!=all.end(); ++li )
    {
        GtkTreeIter giter = *li;
        string s = treestr( &giter, w_treeview, C_EMBLEM_ID_COLUMN );
        if( s == seid )
        {
            ret.push_back( giter );
        }
    }
    return ret;
}

void
tv_iconName_changed( fh_emblem em, string oldv, string newv )
{
    list_gtktreeiter_t li = findAllByID( em->getID() );
    cerr << "tv_iconName_changed(t) em:" << em->getName()
         << " li.sz:" << li.size()
         << " iconname:" << newv
         << endl;
    
    for( list_gtktreeiter_t::iterator gi = li.begin(); gi!=li.end(); ++gi )
    {
        GtkTreeIter giter = *gi;
        gtk_tree_store_set( w_treemodel, &giter,
                            C_EMBLEM_ICONNAME_COLUMN, newv.c_str(),
                            -1 );
    }

    cerr << "tv_iconName_changed(b) em:" << em->getName()
         << " li.sz:" << li.size()
         << " iconname:" << newv
         << endl;
}


void put( GtkTreeStore* ts, GtkTreeIter& giter, fh_emblem em )
{
    gtk_tree_store_set( ts, &giter,
                        C_EMBLEM_ID_COLUMN,         tostr(em->getID()).c_str(),
                        C_EMBLEM_NAME_COLUMN,       em->getName().c_str(),
                        C_EMBLEM_ICONNAME_COLUMN,   em->getIconName().c_str(),
                        C_EMBLEM_LIMITEDPRI_COLUMN, tostr(em->getLimitedViewPriority()).c_str(),
                        C_EMBLEM_DESC_COLUMN,       em->getDescription().c_str(),
                        C_EMBLEM_DLAT,              em->getDigitalLatitude(),
                        C_EMBLEM_DLONG,             em->getDigitalLongitude(),
                        C_EMBLEM_DRANGE,            em->getZoomRange(),
                        -1 );
    em->getIconName_Changed_Sig().connect( sigc::ptr_fun( tv_iconName_changed ) );
}

/**
 * This finds a child in the w_treemodel list by eid only checking in the
 * range starting at giter and not going into children. Note that giter
 * should be set to a valid iterator in the treemodel at the level where
 * the emblem you seek should be findable.
 *
 * return true if the emblem was found and giter points to it at return
 */
bool getChildByID( emblemID_t eid, GtkTreeIter& giter )
{
//     if( gtk_tree_model_get_iter_first( GTK_TREE_MODEL(w_treemodel), &giter ) )
//     {
        while( true )
        {
            string id = treestr( &giter, w_treeview, C_EMBLEM_ID_COLUMN );
            if( id == tostr(eid) )
            {
                return true;
            }
            if( !gtk_tree_model_iter_next( GTK_TREE_MODEL(w_treemodel), &giter ) )
                break;
        }
//    }
    return false;
}

bool childExists( emblemID_t eid, GtkTreeIter giter )
{
    GtkTreeIter piter = giter;
    bool rc = gtk_tree_model_iter_nth_child( GTK_TREE_MODEL( w_treemodel ),
                                             &giter,
                                             &piter,
                                             0 );
    return rc && getChildByID( eid, giter );
}




void SaveData()
{
    et->sync();
    
    if( w_lowestEmblemPriToShow )
        et->setLowestEmblemPriorityToShow(
            (Emblem::limitedViewPri_t)(
                int(
                    gtk_range_get_value( GTK_RANGE( w_lowestEmblemPriToShow )))));
    
    
//     set_db4_string( EAINDEXROOT + "/" + DB_EAINDEX,
//                     IDXMGR_EANAMES_IGNORE_K,
//                     gtk_entry_get_text(GTK_ENTRY(w_eanames_ignore)) );

//     stringlist_t sl = m_eanamesRegexIgnoreList->getStringList();
//     string d = Util::createNullSeperatedList( sl );
//     set_db4_string( EAINDEXROOT + "/" + DB_EAINDEX,
//                     IDXMGR_EANAMES_REGEX_IGNORE_K, d );

    

//     set_db4_string( EAINDEXROOT + "/" + DB_EAINDEX,
//                     IDXMGR_MAX_VALUE_SIZE_K,
//                     tostr((int)gtk_range_get_value(GTK_RANGE(w_max_value_size))));
}



/**
 * Given the GTK+ tree iterator piter and the emblem for that iterator
 * parent_em creates all the transitive children of parent_em in the GTK
 * world
 */
void
LoadData_makeAllChildren( GtkTreeIter piter, fh_emblem parent_em )
{
    emblems_t el = parent_em->getChildren();
    for( emblems_t::iterator ei = el.begin(); ei != el.end(); ++ei )
    {
        fh_emblem em = *ei;
        em->getAddedChild_Sig().connect( sigc::ptr_fun( OnEmblemChildAdded ) );

        GtkTreeIter giter = appendNewBlankItem( &piter );
        put( w_treemodel, giter, em );
        LoadData_makeAllChildren( giter, em );
    }
}

emblemID_t
getEID( GtkTreeIter* iter )
{
    return toType<emblemID_t>(treestr( iter, w_treeview, C_EMBLEM_ID_COLUMN ));
}


void on_gtk_row_expanded_cb ( GtkTreeView *treeview,
                              GtkTreeIter *piter,
                              GtkTreePath *arg2,
                              gpointer     user_data)
{
    GtkTreeIter child_iter;
    gboolean rc = gtk_tree_model_iter_nth_child( GTK_TREE_MODEL(w_treemodel),
                                                 &child_iter,
                                                 piter,
                                                 0 );

    emblemID_t first_child_eid = getEID( &child_iter );
    fh_emblem   de = getDummyTreeModelEmblem();

    if( first_child_eid == de->getID() )
    {
        emblemID_t eid = getEID( piter );
        cerr << "row expanded eid:" << eid << endl;
        fh_emblem  pem = et->getEmblemByID( eid );
        emblems_t   el = pem->getChildren();

        emblems_t::iterator _end = el.end();
        for( emblems_t::iterator ei = el.begin(); ei != _end; ++ei )
        {
            fh_emblem child = *ei;
            
//            cerr << "on_gtk_row_expanded_cb() child:" << child->getName() << endl;
            
            child->getAddedChild_Sig().connect( sigc::ptr_fun( OnEmblemChildAdded ) );
            GtkTreeIter giter = appendNewBlankItem( piter );
            put( w_treemodel, giter, child );

            if( !child->getChildren().empty() )
            {
                GtkTreeIter diter = appendNewBlankItem( &giter );
                put( w_treemodel, diter, de );
            }
        }

        gtk_tree_model_iter_nth_child( GTK_TREE_MODEL( w_treemodel ),
                                       &child_iter, piter, 0 );
        if( getChildByID( de->getID(), child_iter ) )
        {
            gtk_tree_store_remove( GTK_TREE_STORE(w_treemodel), &child_iter );
        }
    }
}

void LoadData()
{
    emblems_t el = et->getAllEmblems();
    fh_emblem de = getDummyTreeModelEmblem();
    
    for( emblems_t::iterator ei = el.begin(); ei != el.end(); ++ei )
    {
        fh_emblem em = *ei;
        em->getAddedChild_Sig().connect( sigc::ptr_fun( OnEmblemChildAdded ) );

        if( em->getParents().empty() )
        {
            GtkTreeIter iter = appendNewBlankItem( 0 );
            put( w_treemodel, iter, em );

            
//            LoadData_makeAllChildren( iter, em );
            emblems_t el = em->getChildren();
            if( !el.empty() )
            {
                GtkTreeIter giter = appendNewBlankItem( &iter );
                put( w_treemodel, giter, de );
            }
        }
    }

//    gtk_tree_view_expand_all( GTK_TREE_VIEW( w_treeview ) );

    if( w_lowestEmblemPriToShow )
        gtk_range_set_value( GTK_RANGE( w_lowestEmblemPriToShow ),
                             et->getLowestEmblemPriorityToShow());
    
}

void
OnEmblemChildAdded( fh_emblem pe, fh_emblem em )
{
    cerr << "OnEmblemChildAdded() pe:" << pe->getID() << " em:" << em->getID()
         << " parents.sz:" << em->getParents().size() << endl;
    list< GtkTreeIter > new_giters;

    // Add a new entry to the GTK tree under the parent
    list_gtktreeiter_t plist = findAllByID( pe->getID() );
    cerr << " plist.sz:" << plist.size() << endl;
        
    for( list_gtktreeiter_t::iterator li = plist.begin(); li!=plist.end(); ++li )
    {
        GtkTreeIter piter = *li;
        
        if( childExists( em->getID(), piter ))
        {
            continue;
        }
            
        cerr << "adding new item in main view for em:" << em->getID() << endl;
        GtkTreeIter giter = appendNewBlankItem( &piter );
        put( w_treemodel, giter, em );
        new_giters.push_back( giter );
        LoadData_makeAllChildren( giter, em );
    }

    // If it used to have no parents then we have to remove top level
    // tree entries for the emblem
    if( em->getParents().size() <= 1  )
    {
        // If the emblem was not a child of any other emblem
        // we should remove it from the top level of the GTK tree
        cerr << "MOVED FROM TOP LEVEL INTO TREE!" << endl;
        GtkTreeIter giter;
        if( gtk_tree_model_get_iter_first( GTK_TREE_MODEL(w_treemodel), &giter ) )
        {
            if( getChildByID( em->getID(), giter ) )
            {
                gtk_tree_store_remove( w_treemodel, &giter );
            }
        }

        //
        // now we should find the new giter and select that one so that the user
        // is still editing the same emblem
        //
        if( !new_giters.empty() )
        {
            GtkTreeView*  tv = GTK_TREE_VIEW(  w_treeview  );
            GtkTreeModel* tm = GTK_TREE_MODEL( w_treemodel );
            GtkTreeIter giter = new_giters.front();
            GtkTreePath* path = gtk_tree_model_get_path( tm, &giter );
            GtkTreeViewColumn *focus_column = 0;
            gtk_tree_view_expand_to_path( tv, path );
            gtk_tree_view_set_cursor( tv, path, focus_column, false );
            gtk_tree_path_free( path );
        }
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


void finishEdit( void* tvp )
{
    GtkTreeView* tv = GTK_TREE_VIEW( tvp );
    GtkTreePath*        path = 0;
    GtkTreeViewColumn*   tvc = 0;

    gtk_tree_view_get_cursor( tv, &path, &tvc );
    gtk_tree_view_set_cursor( tv,  path,  tvc, false );
}



void
new_cb( GtkButton *button, gpointer )
{
    GtkTreeView*  tv = GTK_TREE_VIEW(  w_treeview  );
    GtkTreeModel* tm = GTK_TREE_MODEL( w_treemodel );


    finishEdit( w_treeview );
    
    GtkTreeIter iter = appendNewBlankItem();

    fh_cemblem em = et->createColdEmblem( "new emblem" );
    em->getAddedChild_Sig().connect( sigc::ptr_fun( OnEmblemChildAdded ) );
    put( w_treemodel, iter, em );
    
    GtkTreePath* path      = gtk_tree_model_get_path( tm, &iter );
    GtkTreeViewColumn* col = gtk_tree_view_get_column( tv, C_EMBLEM_NAME_COLUMN );
    gtk_tree_view_set_cursor( tv, path, col, true );
    gtk_tree_path_free( path );
}

void
add_cb( GtkButton *button, gpointer user_data )
{
    emblemID_t original_eid = getCurrentSelectedEmblemID();
    new_cb( button, user_data );

    if( !original_eid )
        return;
    
    GtkTreeView*  tv = GTK_TREE_VIEW(  w_treeview  );
    GtkTreeModel* tm = GTK_TREE_MODEL( w_treemodel );

    emblemID_t current_eid = getCurrentSelectedEmblemID();

    cerr << "original_eid:" << original_eid << " current_eid:" << current_eid << endl;
    fh_emblem original_em = et->getEmblemByID( original_eid );
    fh_emblem  current_em = et->getEmblemByID(  current_eid );

    if( original_em && current_em )
    {
        cerr << "link()ing original:" << original_em->getID() << " to " << current_em->getID() << endl;
        link( original_em, current_em );
    }

//     // remove old top level
//     GtkTreeIter giter;
//     if( gtk_tree_model_get_iter_first( GTK_TREE_MODEL(w_treemodel), &giter ) )
//     {
//         if( getChildByID( current_em->getID(), giter ) )
//         {
//             gtk_tree_store_remove( w_treemodel, &giter );
//         }
//     }

//     // select the right emblem in the main list
//     list_gtktreeiter_t pl = findAllByID( original_em->getID() );

//     cerr << "original:" << original_em->getID() << " current:" << current_em->getID()
//          << " pl.sz:" << pl.size()
//          << endl;
    
//     for( list_gtktreeiter_t::iterator pi = pl.begin(); pi!=pl.end(); ++pi )
//     {
//         GtkTreeIter piter = *pi;
//         if( getChildByID( current_em->getID(), piter ) )
//         {
//             cerr << "have found current in parent" << endl;
//             GtkTreeIter citer = piter;
//             GtkTreeModel* tm = GTK_TREE_MODEL( w_treemodel );
//             GtkTreePath* path = gtk_tree_model_get_path( tm, &citer );
//             GtkTreeViewColumn *focus_column = 0;
//             gtk_tree_view_expand_to_path( tv, path );
//             gtk_tree_view_set_cursor( tv, path, focus_column, false );
//             gtk_tree_path_free( path );
//             break;
//         }
//     }
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

    for( list_gtktreeiter_t::iterator iter = deliters.begin(); iter != deliters.end(); ++iter )
    {
        GtkTreeIter giter = *iter;
        string seid = treestr( &giter, w_treeview, C_EMBLEM_ID_COLUMN );
        gtk_tree_store_remove( ts, &giter );

        if( !seid.empty() )
        {
            emblemID_t eid = toType<emblemID_t>( seid );
            cerr << "del_cb() eid:" << eid << endl;
            fh_emblem   em = et->getEmblemByID( eid );
            if( em )
            {
                list_gtktreeiter_t others = findAllByID( eid );
                copy( others.begin(), others.end(), back_inserter(deliters) );
                et->erase( em );
            }
        }
    }
    et->sync();
}

emblemID_t
getCurrentSelectedEmblemID()
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
edited_cb (GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer user_data )
{
    string v = new_text ? new_text : "";
    double dv = 0.0;
    int cidx = GPOINTER_TO_INT(user_data);
    cerr << "edited_cb() c:" << cidx << " new_text:" << new_text << endl;

    if( cidx >= C_EMBLEM_DLAT && cidx <= C_EMBLEM_DRANGE )
    {
        dv = toType<double>(v);
    }
    
    GtkTreeModel *model = GTK_TREE_MODEL (w_treemodel);
    GtkTreeIter iter;
    GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
    gtk_tree_model_get_iter (model, &iter, path);
    if( cidx >= C_EMBLEM_DLAT && cidx <= C_EMBLEM_DRANGE )
    {
        gtk_tree_store_set (GTK_TREE_STORE (model), &iter, cidx, dv, -1);
    }
    else
    {
        gtk_tree_store_set (GTK_TREE_STORE (model), &iter, cidx, new_text, -1);
    }
    gtk_tree_path_free (path);

    emblemID_t eid = getCurrentSelectedEmblemID();
    cerr << "edited_cb() eid:" << eid << endl;
    if( !eid )
        return;
    
    if( fh_emblem em = et->getEmblemByID( eid ) )
    {
        if( cidx == C_EMBLEM_NAME_COLUMN )
        {
            em->setName( v );
        }
        else if( cidx == C_EMBLEM_ICONNAME_COLUMN )
        {
            em->setIconName( v );
        }
        else if( cidx == C_EMBLEM_LIMITEDPRI_COLUMN )
        {
            int iv = toint( v );
            iv = MAX( iv, Emblem::LIMITEDVIEW_PRI_LOW );
            iv = MIN( iv, Emblem::LIMITEDVIEW_PRI_HI );
            Emblem::limitedViewPri_t lp = Emblem::limitedViewPri_t(iv);
            cerr << "limited pri:" << lp << endl;
            em->setLimitedViewPriority( lp );
        }
        else if( cidx == C_EMBLEM_DESC_COLUMN )
        {
            em->setDescription( v );
        }
        else if( cidx == C_EMBLEM_DLAT )
        {
            em->setDigitalLatitude( dv );
        }
        else if( cidx == C_EMBLEM_DLONG )
        {
            em->setDigitalLongitude( dv );
        }
        else if( cidx == C_EMBLEM_DRANGE )
        {
            em->setZoomRange( dv );
        }
    }
}

void
parents_edited_cb (GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer user_data )
{
    int cidx = (int)CP_CHECKED_COLUMN;
    cerr << "parents_edited_cb cidx:" << cidx << endl;
    
    GtkTreeModel *model = GTK_TREE_MODEL (w_parents_treemodel);
    GtkTreeIter iter;
    GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
    gboolean value;
        
    gtk_tree_model_get_iter (model, &iter, path);
    gtk_tree_model_get (model, &iter, cidx, &value, -1);
    value = !value;
    gtk_tree_store_set (GTK_TREE_STORE (model), &iter, cidx, value, -1);
    gtk_tree_path_free (path);

    // We need to adjust the GTK+ treeview to reflect the
    // new parent status
    fh_emblem em            = et->getEmblemByID( getCurrentSelectedEmblemID() );
    string    parentName    = treestr ( &iter, w_parents_treeview, CP_NAME_COLUMN );
    fh_emblem pe            = et->getEmblemByName( parentName );
    int       oldParentSize = em->getParents().size();

    cerr << "parents_edited_cb() updating the main tree..." << endl
         << " oldParentSize:" << oldParentSize << " value:" << value
         << " pe:" << pe->getID() << " em:" << em->getID() << endl;

    list< GtkTreeIter > new_giters;
    
    if( value )
    {
        // Add a new entry to the GTK tree under the parent
        list_gtktreeiter_t plist = findAllByID( pe->getID() );
        cerr << " plist.sz:" << plist.size() << endl;
        
        for( list_gtktreeiter_t::iterator li = plist.begin(); li!=plist.end(); ++li )
        {
            GtkTreeIter piter = *li;

            if( childExists( em->getID(), piter ))
            {
                cerr << "child already exists" << endl;
                continue;
            }
            
            cerr << "adding new item in main view for em:" << em->getID() << endl;

            GtkTreeIter giter = appendNewBlankItem( &piter );
            put( w_treemodel, giter, em );
            new_giters.push_back( giter );
            LoadData_makeAllChildren( giter, em );
        }
    }
    else
    {
        // remove the entry from the GTK tree for this old parent

        list_gtktreeiter_t plist = findAllByID( pe->getID() );
        cerr << " plist.sz:" << plist.size() << endl;

        for( list_gtktreeiter_t::iterator li = plist.begin(); li!=plist.end(); ++li )
        {
            GtkTreeIter piter = *li;
            GtkTreeIter giter;
            bool rc = gtk_tree_model_iter_nth_child( GTK_TREE_MODEL( w_treemodel ),
                                                     &giter,
                                                     &piter,
                                                     0 );
            if( !rc )
            {
                cerr << "parent has no first child." << endl;
                continue;
            }
            
            
            if( getChildByID( em->getID(), giter ) )
            {
                gtk_tree_store_remove( w_treemodel, &giter );
            }
            else
            {
                cerr << "no child found with id:" << em->getID() << endl;
            }
        }
    }
    
    // update the backend
    if( value && !em->hasParent( pe ) )
    {
        link( pe, em );
    }
    if( !value && em->hasParent( pe ) )
    {
        unlink( pe, em );
    }

    if( value && !oldParentSize )
    {
        // If the emblem was not a child of any other emblem
        // we should remove it from the top level of the GTK tree
        cerr << "MOVED FROM TOP LEVEL INTO TREE!" << endl;
        GtkTreeIter giter;
        if( gtk_tree_model_get_iter_first( GTK_TREE_MODEL(w_treemodel), &giter ) )
        {
            if( getChildByID( em->getID(), giter ) )
            {
                gtk_tree_store_remove( w_treemodel, &giter );
            }
        }

        //
        // now we should find the new giter and select that one so that the user
        // is still editing the same emblem
        //
        if( !new_giters.empty() )
        {
            GtkTreeView*  tv = GTK_TREE_VIEW(  w_treeview  );
            GtkTreeModel* tm = GTK_TREE_MODEL( w_treemodel );
            GtkTreeIter giter = new_giters.front();
            GtkTreePath* path = gtk_tree_model_get_path( tm, &giter );
            GtkTreeViewColumn *focus_column = 0;
            gtk_tree_view_expand_to_path( tv, path );
            gtk_tree_view_set_cursor( tv, path, focus_column, false );
            gtk_tree_path_free( path );
        }
    }
    if( !value && oldParentSize==1 )
    {
        // We move emblems that have no parents to the top
        // level of the GTK tree
        cerr << "MOVED FROM TREE INTO TOP LEVEL!" << endl;
        GtkTreeIter giter = appendNewBlankItem();
        put( w_treemodel, giter, em );
        LoadData_makeAllChildren( giter, em );

        // Select the moved entry in the treelist

        GtkTreeView*  tv = GTK_TREE_VIEW(  w_treeview  );
        GtkTreeModel* tm = GTK_TREE_MODEL( w_treemodel );
        GtkTreePath* path = gtk_tree_model_get_path( tm, &giter );
        GtkTreeViewColumn *focus_column = 0;
        gtk_tree_view_set_cursor( tv, path, focus_column, false );
        gtk_tree_path_free( path );
    }
    
}

void
changed_cb( GtkTreeSelection *sel, gpointer )
{
    static emblemID_t old = 0;
    emblemID_t current = getCurrentSelectedEmblemID();

    cerr << "tv_changed_cb() old:" << old << " new:" << current << endl;
        
    if( old != current )
    {
        // saving the old is now done in parents_edited_cb()
        {
        }
        

        if( current )
        {
            // load the new selection's data
            fh_emblem em = et->getEmblemByID( current );
            emblems_t el = em->getPossibleParents();
            emblems_t parents = em->getParents();

            gtk_tree_store_clear( w_parents_treemodel );
            for( emblems_t::const_iterator ci = el.begin(); ci!=el.end(); ++ci )
            {
                GtkTreeIter iter;
                bool isParent = find( parents.begin(), parents.end(), *ci ) != parents.end();
                string name = (*ci)->getName().c_str();
                const char* pname = name.c_str();
//                 cerr << "possible parent:" << (*ci)->getID()
//                      << " name:" << (*ci)->getName()
//                      << " pname:" << pname
//                      << " isParent:" << isParent
//                      << endl;

                gtk_tree_store_append( w_parents_treemodel, &iter, NULL );
                gtk_tree_store_set( w_parents_treemodel, &iter,
                                    CP_CHECKED_COLUMN,   isParent,
                                    CP_NAME_COLUMN,      pname,
                                    -1 );
            }
        }
        
        old = current;
    }
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void store_filename( GtkFileSelection *, GtkFileSelection *file_selector )
{
    cerr << "store_filename(begin)" << endl;

    
    emblemID_t eid = (emblemID_t)GPOINTER_TO_INT(g_object_get_data( G_OBJECT(file_selector), "EMID" ));
    fh_emblem   em = et->getEmblemByID( eid );
    
    const gchar *selected_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
    cerr << "Selected filename:" << selected_filename << endl;
    em->setIconName( selected_filename );

    
    cerr << "store_filename(end)" << endl;
}

void
iconpage_changeicon_cb( GtkButton *button, gpointer user_data )
{
    fh_emblem em = (Emblem*)user_data;
    GtkWidget *file_selector = gtk_file_selection_new ("Please select a file for editing.");

    gtk_file_selection_set_filename ( GTK_FILE_SELECTION (file_selector),
                                      resolveToIconPath( em->getIconName() ).c_str() );
    g_object_set_data( G_OBJECT(file_selector), "EMID", GINT_TO_POINTER(em->getID()) );
    g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                      "clicked",
                      G_CALLBACK (store_filename),
                      file_selector );
   			   
    g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                              "clicked",
                              G_CALLBACK (gtk_widget_destroy), 
                              (gpointer) file_selector); 

    g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                              "clicked",
                              G_CALLBACK (gtk_widget_destroy),
                              (gpointer) file_selector); 
   
    gtk_widget_show (file_selector);
}

void
iconpage_iconName_changed( fh_emblem em, string oldv, string newv, GtkWidget* w_icon )
{
    cerr << "iconpage_iconName_changed(start)" << endl;
    setImageButton( w_icon, newv );
    cerr << "iconpage_iconName_changed(end)" << endl;
}

void
createIconPageEntry( fh_emblem em )
{
    static int m_lastCreatedIconColumnNumber = 0;
    static guint r = 0;
    guint c = m_lastCreatedIconColumnNumber++;

    if( c >= m_numberOfIconsAcross )
    {
        c = 0;
        m_lastCreatedIconColumnNumber = 1;
        ++r;
    }
    
//    g_object_get( G_OBJECT(w_icontable), "n-rows", &r, 0 );
//  g_object_get( G_OBJECT(w_icontable), "n-columns", &c, 0 );

    GtkWidget* vbx = GTK_WIDGET(gtk_vbox_new(0,0));

    string iconName = resolveToIconPath( em->getIconName() );
    GtkWidget* w_icon = makeImageButton( em->getIconName() );
    GtkWidget* w_lab  = gtk_label_new( em->getName().c_str() );

    gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(w_icon), 0, 0, 0 );
    gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(w_lab),  0, 0, 0 );

    gtk_table_attach_defaults(GTK_TABLE(w_icontable), vbx, c+0, c+1, r, r+1 );

    gtk_signal_connect(GTK_OBJECT( w_icon ), "clicked", GTK_SIGNAL_FUNC(iconpage_changeicon_cb), GetImpl(em) );

    em->getIconName_Changed_Sig().connect( bind( sigc::ptr_fun( iconpage_iconName_changed ), w_icon ) );
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

static const char* const GOBJ_CAPPLET_EID_K = "GOBJ_CAPPLET_EID";

class mffs_AddParentEmblem
    :
    public MenuFromFilesystem
{
protected:

    GtkWidget* makeLeafMenuItem( fh_context c, const std::string& label, std::string iconpath )
        {
            GtkWidget* ret = 0;
            emblemID_t eid = getCurrentSelectedEmblemID();
            
            if( fh_emblem em = et->getEmblemByID( eid ) )
            {
                fh_emblem currentp = et->getEmblemByUniqueName( label );
                emblems_t pp = em->getPossibleParents();
                if( !contains( pp, currentp ) )
                    return 0;
        
                ret = makeMenuItem( label, iconpath, true );
                emblems_t em_parents = em->getParents();
                bool hasParent = contains( em_parents, currentp );
                gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( ret ), hasParent );
                g_object_set_data( G_OBJECT(ret), GOBJ_CAPPLET_EID_K, (void*)currentp->getID() );
            }
            
            return ret;
        }
};
void
AddParentEmblem_leaf( MenuFromFilesystem*, fh_context c, GtkMenuItem* mi )
{
    if( c )
        cerr << "AddParentEmblem_leaf() c:" << c->getURL() << endl;

    emblemID_t menu_eid = (emblemID_t)GPOINTER_TO_INT(
        g_object_get_data( G_OBJECT(mi), GOBJ_CAPPLET_EID_K ));
    fh_emblem  menu_em  = et->getEmblemByID( menu_eid );
    cerr << "AddParentEmblem_leaf() menu_eid:" << menu_eid << endl;

    emblemID_t eid = getCurrentSelectedEmblemID();
    fh_emblem   em = et->getEmblemByID( eid );

    cerr << "AddParentEmblem_leaf() eid:" << eid << endl;
    
    if( em && menu_em )
    {
        cerr << "Should be link()ing parent:" << menu_em->getID() << " to:" << em->getID() << endl;
        link( menu_em, em );
    }
}

/********************************************************************************/
/********************************************************************************/

class mffs_AddChildEmblem
    :
    public MenuFromFilesystem
{
protected:

    GtkWidget* makeLeafMenuItem( fh_context c, const std::string& label, std::string iconpath )
        {
            GtkWidget* ret = 0;
            emblemID_t eid = getCurrentSelectedEmblemID();
            
            if( fh_emblem em = et->getEmblemByID( eid ) )
            {
                fh_emblem currentch = et->getEmblemByUniqueName( label );
                emblems_t pc = em->getPossibleChildren();
                if( !contains( pc, currentch ) )
                    return 0;
        
                ret = makeMenuItem( label, iconpath, true );
                emblems_t em_children = em->getParents();
                bool hasChild = contains( em_children, currentch );
                gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( ret ), hasChild );
                g_object_set_data( G_OBJECT(ret), GOBJ_CAPPLET_EID_K, (void*)currentch->getID() );
            }
            
            return ret;
        }
};
void
AddChildEmblem_leaf( MenuFromFilesystem*, fh_context c, GtkMenuItem* mi )
{
    if( c )
        cerr << "AddChildEmblem_leaf() c:" << c->getURL() << endl;

    emblemID_t menu_eid = (emblemID_t)GPOINTER_TO_INT(
        g_object_get_data( G_OBJECT(mi), GOBJ_CAPPLET_EID_K ));
    fh_emblem  menu_em  = et->getEmblemByID( menu_eid );
    cerr << "AddChildEmblem_leaf() menu_eid:" << menu_eid << endl;

    emblemID_t eid = getCurrentSelectedEmblemID();
    fh_emblem   em = et->getEmblemByID( eid );

    cerr << "AddChildEmblem_leaf() eid:" << eid << endl;
    
    if( em && menu_em )
    {
        cerr << "Should be link()ing parent:" << em->getID() << " to:" << menu_em->getID() << endl;
        link( em, menu_em );
    }
}

/********************************************************************************/
/********************************************************************************/

static gint main_treeview_button_cb( GtkWidget *widget, GdkEvent *event, gpointer user_data )
{
    GtkMenu *menu;
    GdkEventButton *event_button;

    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

    if( event->type == GDK_BUTTON_RELEASE )
    {
        event_button = (GdkEventButton *) event;

        if (event_button->button == 3)
        {
            GtkWidget* child;
            GtkWidget* m;
            int i=0;

            if( !tryToEnsureSomethingIsSelected( w_treeview, event ) )
            {
                return 0;
            }

            try
            {
                menu = GTK_MENU( gtk_menu_new() );
                fh_context etagerec = Resolve("etagere://");

                static mffs_AddParentEmblem parentm;
                parentm.setup( menu, etagerec, AddParentEmblem_leaf, "add parent" );
                parentm.perform();

                static mffs_AddChildEmblem childm;
                childm.setup( menu, etagerec, AddChildEmblem_leaf, "add child" );
                childm.perform();
                
                showPopup( menu );

                // Tell calling code that we have handled this event;
                // the buck stops here.
                return TRUE;
            }
            catch( exception& e )
            {
                cerr << "e:" << e.what() << endl;
                RunErrorDialog( e.what(), 0 );
            }
        }
    }
    return 0;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void on_row_inserted( GtkTreeModel *treemodel, GtkTreePath *arg1, GtkTreeIter *arg2, gpointer user_data )
{
    GtkTreeIter iter;
    gtk_tree_model_get_iter (treemodel, &iter, arg1);
    string n = treestr( &iter, GTK_WIDGET(treemodel), C_EMBLEM_NAME_COLUMN );
    cerr << "insert path n:" << n << endl;


    n = treestr( arg2, GTK_WIDGET(treemodel), C_EMBLEM_NAME_COLUMN );
    cerr << "insert iter n:" << n << endl;
}

void on_row_deleted( GtkTreeModel *treemodel, GtkTreePath *arg1, gpointer user_data )
{
    GtkTreeIter iter;
    gtk_tree_model_get_iter (treemodel, &iter, arg1);
    string n = treestr( &iter, GTK_WIDGET(treemodel), C_EMBLEM_NAME_COLUMN );
    cerr << "Deleted n:" << n << endl;
}


void make_window()
{
    GtkWidget* lab;
    GtkWidget* slide;

    int c=0;
    int r=0;
    
    gtk_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title ( GTK_WINDOW( gtk_window ), "Ferris medallions" );
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
    gtk_frame_set_label( GTK_FRAME (details_frame), "details" );
    gtk_notebook_append_page( main_widget, GTK_WIDGET(paned), gtk_label_new( "structure" ));

    ////////////////////////////
    // make the treeview side //
    ////////////////////////////
    {
        GtkToolbar* tb = GTK_TOOLBAR(gtk_toolbar_new());
        GtkWidget* vbx = GTK_WIDGET(gtk_vbox_new(0,0));
        GtkWidget* hbx = GTK_WIDGET(gtk_hbox_new(0,0));
        GtkWidget* b;
        GtkCellRenderer* ren;
        int colnum = 0;

        b = gtk_toolbar_append_item( tb, "New",
                                     "create a new emblem", "",
                                     gtk_image_new_from_stock(
                                         GTK_STOCK_NEW,
                                         GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                     GTK_SIGNAL_FUNC(new_cb),
                                     0 );
        b = gtk_toolbar_append_item( tb, "add",
                                     "create a new emblem as a child of selected one", "",
                                     gtk_image_new_from_stock(
                                         GTK_STOCK_NEW,
                                         GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                     GTK_SIGNAL_FUNC(add_cb),
                                     0 );
        b = gtk_toolbar_append_item( tb, "Delete",
                                     "delete or unparent selected emblem(s)", "",
                                     gtk_image_new_from_stock(
                                         GTK_STOCK_DELETE,
                                         GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                     GTK_SIGNAL_FUNC(del_cb),
                                     0 );

        gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( tb ));
        gtk_box_pack_start(GTK_BOX(vbx), hbx, 0, 0, 0 );

        w_treemodel = gtk_tree_store_new( C_COLUMN_COUNT,
                                          G_TYPE_STRING, G_TYPE_STRING,
                                          G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                          G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE );
        w_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL( w_treemodel ));
        g_signal_connect_data( G_OBJECT( w_treeview ), "row-expanded",
                               G_CALLBACK (on_gtk_row_expanded_cb), (gpointer)0,
                               0, GConnectFlags(0));

        gtk_signal_connect(GTK_OBJECT(w_treeview), "button_press_event",
                           GTK_SIGNAL_FUNC (main_treeview_button_cb), 0 );
        gtk_signal_connect(GTK_OBJECT(w_treeview), "button_release_event",
                           GTK_SIGNAL_FUNC (main_treeview_button_cb), 0 );

        
        GtkWidget* sw;
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_container_add( GTK_CONTAINER( sw ), GTK_WIDGET( w_treeview ));
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(sw), 1, 1, 0 );
        gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (w_treeview), TRUE);
        GObject *selection;
        selection = G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (w_treeview)));
//        gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_MULTIPLE);
        gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW (w_treeview), true );
        gtk_tree_view_set_enable_search(GTK_TREE_VIEW (w_treeview), true );
        gtk_tree_view_set_search_column(GTK_TREE_VIEW (w_treeview), C_EMBLEM_NAME_COLUMN );
        g_signal_connect_data( G_OBJECT( selection ), "changed",
                               G_CALLBACK (changed_cb), 0, 0, GConnectFlags(0));

        gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_MULTIPLE);

        
        gtk_container_add( GTK_CONTAINER( tree_frame ), vbx );

        colnum = C_EMBLEM_ID_COLUMN;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 0, 0 );
        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "id", ren,
                                                                     "text", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );

        colnum = C_EMBLEM_NAME_COLUMN;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "edited",
                               G_CALLBACK (edited_cb),
                               (gpointer)colnum, 0,
                               GConnectFlags(0));
        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "emblem", ren,
                                                                     "text", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );
        gtk_tree_sortable_set_sort_column_id( GTK_TREE_SORTABLE( w_treemodel ),
                                              colnum, GTK_SORT_ASCENDING );
        
        colnum = C_EMBLEM_ICONNAME_COLUMN;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "edited",
                               G_CALLBACK (edited_cb),
                               (gpointer)colnum, 0,
                               GConnectFlags(0));
        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "iconname", ren,
                                                                     "text", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );


        colnum = C_EMBLEM_LIMITEDPRI_COLUMN;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "edited",
                               G_CALLBACK (edited_cb),
                               (gpointer)colnum, 0,
                               GConnectFlags(0));
        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "limited pri", ren,
                                                                     "text", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );
        

        colnum = C_EMBLEM_DESC_COLUMN;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "edited",
                               G_CALLBACK (edited_cb),
                               (gpointer)colnum, 0,
                               GConnectFlags(0));
        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "description", ren,
                                                                     "text", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );


        
        colnum = C_EMBLEM_DLAT;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "edited",
                               G_CALLBACK (edited_cb),
                               (gpointer)colnum, 0,
                               GConnectFlags(0));
        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "latitude", ren,
                                                                     "text", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );

        colnum = C_EMBLEM_DLONG;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "edited",
                               G_CALLBACK (edited_cb),
                               (gpointer)colnum, 0,
                               GConnectFlags(0));
        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "longitude", ren,
                                                                     "text", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );

        colnum = C_EMBLEM_DRANGE;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "edited",
                               G_CALLBACK (edited_cb),
                               (gpointer)colnum, 0,
                               GConnectFlags(0));
        w_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "zoom-range", ren,
                                                                     "text", colnum,
                                                                     NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_cols[ colnum ], colnum );
        
        
    }
    

    //
    // Add the "details" section for editing emblems
    //
    {
        GtkWidget* w = 0;
        GtkCellRenderer* ren;
        int colnum = 0;
        
        
        gtk_table = gtk_table_new ( 1, 2, false );
        gtk_container_add( GTK_CONTAINER( details_frame ), GTK_WIDGET(gtk_table) );

        r=0; c=0;

        w_parents_treemodel = gtk_tree_store_new( CP_COLUMN_COUNT,
                                                  G_TYPE_BOOLEAN, G_TYPE_STRING );
        w_parents_treeview  = gtk_tree_view_new_with_model(GTK_TREE_MODEL( w_parents_treemodel )); 
        
        GtkWidget* sw;
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_container_add( GTK_CONTAINER( sw ), GTK_WIDGET( w_parents_treeview ));
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

        colnum = CP_CHECKED_COLUMN;
        ren = gtk_cell_renderer_toggle_new();
        g_object_set(ren, "editable", 1, 0 );
        g_signal_connect_data( G_OBJECT( ren ), "toggled",
                               G_CALLBACK (parents_edited_cb),
                               (gpointer)colnum, 0,
                               GConnectFlags(0));
        w_parents_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "is-parent", ren,
                                                                             "active", colnum,
                                                                             NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_parents_treeview ), w_parents_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_parents_cols[ colnum ], colnum );


        colnum = CP_NAME_COLUMN;
        ren = gtk_cell_renderer_text_new ();
        g_object_set(ren, "editable", 0, 0 );
        w_parents_cols[ colnum ] = gtk_tree_view_column_new_with_attributes( "emblem", ren,
                                                                             "text", colnum,
                                                                             NULL);
        gtk_tree_view_append_column( GTK_TREE_VIEW( w_parents_treeview ), w_parents_cols[ colnum ] );
        gtk_tree_view_column_set_sort_column_id( w_parents_cols[ colnum ], colnum );
        
        
        gtk_table_attach_defaults(GTK_TABLE(gtk_table), sw, c+0, c+2, r, r+1 );
    }
    
//     {
//         w_icontable  = gtk_table_new ( 0, m_numberOfIconsAcross, false );
//         GtkWidget* sw;
//         sw = gtk_scrolled_window_new(NULL, NULL);
//         gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw),
//                                               GTK_WIDGET( w_icontable ));
//         gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
//                                         GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
//         gtk_notebook_append_page( main_widget, GTK_WIDGET(sw), gtk_label_new( "icons" ));

//         emblems_t el = et->getAllEmblems();
    
//         for( emblems_t::iterator ei = el.begin(); ei != el.end(); ++ei )
//         {
//             fh_emblem em = *ei;
//             createIconPageEntry( em );
//         }
//     }


    // general settings for emblems
    {
        int row_count = 1;
        int col_count = 2;
        int r = 0;
        int c = 0;
        
        w_globaltable = gtk_table_new ( col_count, row_count, false );
        GtkWidget* sw;
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw),
                                              GTK_WIDGET( w_globaltable ));
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_notebook_append_page( main_widget, GTK_WIDGET(sw),
                                  gtk_label_new( "global settins" ));

        GtkWidget* lab;
        GtkWidget* slide;
        
        lab   = gtk_label_new( "lowest emblem priority to show" );
        slide = gtk_hscale_new_with_range ( Emblem::LIMITEDVIEW_PRI_LOW,
                                            Emblem::LIMITEDVIEW_PRI_HI,
                                            toint( CFG_LOWEST_EMBLEM_PRI_TO_SHOW_DEFAULT ) );
        w_lowestEmblemPriToShow = slide;
        gtk_scale_set_value_pos( GTK_SCALE(slide), GTK_POS_LEFT );

        gtk_table_attach_defaults(GTK_TABLE(w_globaltable), lab,   0, 1, r, r+1 );
        gtk_table_attach_defaults(GTK_TABLE(w_globaltable), slide, 1, 2, r, r+1 );
    }
    

    
    
    
//     r=0;
//     lab  = gtk_label_new( "list of attributes not to index\n"
//                           "(comma seperated: as-xml,as-text,whatever)" );
//     gtk_table_attach_defaults(GTK_TABLE(gtk_table), lab, 0, 1, r, r+1 );
//     w_eanames_ignore = gtk_entry_new();
//     gtk_table_attach_defaults(GTK_TABLE(gtk_table), w_eanames_ignore, 1, 2, r, r+1 );


//     c=0;
//     ++r;
//     m_eanamesRegexIgnoreList->setColumnLabel( "regex" );
//     m_eanamesRegexIgnoreList->setDescriptionLabel( "regex for attributes not to index" );
//     gtk_table_attach_defaults(GTK_TABLE(gtk_table),
//                               m_eanamesRegexIgnoreList->getWidget(), 0, 2, r, r+1 );

//     c=0;
//     ++r;
//     lab  = gtk_label_new( "max attribute value size to index" );
//     gtk_table_attach_defaults(GTK_TABLE(gtk_table), lab, 0, 1, r, r+1 );
//     slide = gtk_hscale_new_with_range ( 0, 1024, 50 );
//     gtk_range_set_value ( GTK_RANGE(slide), toint(IDXMGR_MAX_VALUE_SIZE_DEFAULT) );
//     gtk_scale_set_value_pos( GTK_SCALE(slide), GTK_POS_RIGHT );
//     w_max_value_size = slide;
//     gtk_table_attach_defaults(GTK_TABLE(gtk_table), w_max_value_size, 1, 2, r, r+1 );


    

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

        et = Factory::getEtagere();
        
//        gtk_idle_add( fire_ls_cb, (void*)&ls );
        make_window();
        LoadData();

//         gtk_tree_view_set_reorderable( GTK_TREE_VIEW( w_treeview ), true );
//         g_signal_connect (G_OBJECT( w_treemodel ),
//                           "row-deleted",
//                           G_CALLBACK (on_row_deleted),
//                           0 );
//         g_signal_connect (G_OBJECT( w_treemodel ),
//                           "row-inserted",
//                           G_CALLBACK (on_row_inserted),
//                           0 );
        
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

