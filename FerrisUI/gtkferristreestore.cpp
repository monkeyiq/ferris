/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris context tree store
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

    $Id: gtkferristreestore.cpp,v 1.18 2010/09/24 21:31:07 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

//
// called for every cell that needs to be rendered
//
// gtk_ferris_tree_store_get_n_columns
// gtk_ferris_tree_store_get_value col:69

// Called for each row in a redisplay
//
// gtk_ferris_tree_store_iter_next c:file:///tmp/ft/2
// gtk_ferris_tree_store_get_path c:file:///tmp/ft/2


#include <gtk/gtktreemodel.h>
#include <gtk/gtktreednd.h>

#include <gtkferristreestore.hh>
#include "GtkFerris.hh"
#include <TreeStoreDriver.hh>

#include <string.h>
#include <gobject/gvaluecollector.h>
#include <sigc++/bind.h>
#include <unistd.h>

#include <map>
#include "Ferris/FerrisStdHashMap.hh"
#include <set>

#include <Ferris_private.hh>
#include <Medallion.hh>

#include <Ferris/ChainedViewContext.hh>

// #ifdef LG_GTKFERRIS_D
// #undef LG_GTKFERRIS_D
// #define LG_GTKFERRIS_D cerr
// #endif

using namespace std;
using namespace Ferris;

//#define DISABLE_SUBTREE_SUPPORT
//#define GTK_SUBTREE_IS_BUGGY

const char* const treeicon_pixbuf_cn = "treeicon-pixbuf";
const char* const emblems_pixbuf_cn  = "emblem:emblems-pixbuf";
const char* const ferris_iconname_pixbuf_cn  = "ferris-iconname-pixbuf";

/********************************************************************************/
/********************************************************************************/
/**   DECLARATIONS **************************************************************/
/********************************************************************************/
/********************************************************************************/

static Context*
deepestDelegate( Context* c )
{
    Context* ret = c;
    
    if( ChainedViewContext* d = dynamic_cast<ChainedViewContext*>(c) )
    {
        while( d && isBound( d->getDelegate() ))
        {
            ret = GetImpl(d->getDelegate());
            d = dynamic_cast<ChainedViewContext*>( GetImpl(d->getDelegate()));
        }
    }
    return ret;
    
//     while( isBound( d->getDelegate() ))
//     {
//         if( ChainedViewContext* t = dynamic_cast<ChainedViewContext*>( GetImpl(d->getDelegate())))
//             d = t;
//         else
//             return d;
//     }
//     return d;
}

string tostr( GtkTreePath* path );


typedef struct _GtkFerrisTreeStore       GtkFerrisTreeStore;
typedef struct _GtkFerrisTreeStoreClass  GtkFerrisTreeStoreClass;
namespace FerrisUI
{
    class FerrisTreeModel_Impl;
};

#define GTK_TYPE_FERRIS_TREE_STORE		   (gtk_ferris_tree_store_get_type ())
#define GTK_FERRIS_TREE_STORE(obj)         (GTK_CHECK_CAST ((obj), GTK_TYPE_FERRIS_TREE_STORE, GtkFerrisTreeStore))
#define GTK_FERRIS_TREE_STORE_CLASS(klass) \
(GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_FERRIS_TREE_STORE, GtkFerrisTreeStoreClass))

#define GTK_IS_FERRIS_TREE_STORE(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_FERRIS_TREE_STORE))

#define GTK_IS_FERRIS_TREE_STORE_CLASS(klass) \
(GTK_CHECK_CLASS_TYPE ((obj), GTK_TYPE_FERRIS_TREE_STORE))

#define GTK_FERRIS_TREE_STORE_GET_CLASS(obj) \
(GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_FERRIS_TREE_STORE, GtkFerrisTreeStoreClass))


struct _GtkFerrisTreeStore
{
    GObject parent;

    FerrisUI::FerrisTreeModel_Impl* d;
    
    gint32 ColumnCount;
    gint stamp;
    
    gpointer root;
    gpointer last;
};

struct _GtkFerrisTreeStoreClass
{
  GObjectClass parent_class;
};


GtkType             gtk_ferris_tree_store_get_type (void);
GtkFerrisTreeStore* gtk_ferris_tree_store_new();

namespace FerrisUI
{

    int mydistance( Context::iterator iter, Context::iterator e )
        {
            int count = 0;
            for( ; iter != e ; ++iter )
            {
                ++count;
            }
            return count;
        }

    /////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////

    
    bool isColumnFixedSize( const std::string& cn )
    {
        static set<string> fixedNames;
        static Util::SingleShot virgin;
        if( virgin() )
        {
            fixedNames.insert("size");
            fixedNames.insert("size-human-readable");
            fixedNames.insert("mtime-display");
            fixedNames.insert("protection-ls");
            fixedNames.insert("protection-raw");

            fixedNames.insert("emblem:emblems-pixbuf");
            fixedNames.insert("treeicon-pixbuf");
        }

        return( fixedNames.end() != fixedNames.find( cn ) );
    }

    
    struct myhash
    {
        size_t operator()( Context* x) const
            {
                return (size_t)x;
            }
    };

    
    typedef vector< fh_context > contexts_t;
    typedef std::vector< std::string > columnNames_t;
    typedef map< Context*, Context::iterator > ContextIterators_t;
//    typedef hash_map< Context*, Context::iterator, myhash > ContextIterators_t;
//    typedef vector< Context::iterator > ContextIterators_t;

    /**
     * The GTK object uses this object quite a bit. Basically we have this nice C++ front
     * that is created with CreateTreeModel() and which can have properly constructed instance
     * data etc.
     */
    class FERRISEXP_DLLLOCAL FerrisTreeModel_Impl
        :
        public FerrisTreeModel
    {
        typedef FerrisTreeModel      _Base;
        typedef FerrisTreeModel_Impl _Self;

        /**
         * If m_root is not bound, set it to the given context
         */
        void setRoot( fh_context c );

        /**
         * Clear the gtk model of all items emitting events so that all views
         * think there are no items in the model
         */
        void clear();
        void clear( fh_context c );


        /**
         * Create the underlying GTK object and setup pointers.
         * NOTE that the class itself cant use this to change the GTK model object
         * because all views using the old model would need to be reset too
         */
        void createGtkModel();

        /**
         * Populate the model with data from the given context
         * called from read(), updateViewForFiltering() and updateViewForSorting() to update
         * what is shown and the ordering
         */
        void populateModelFromContext( fh_context c, bool isRoot );
        
        
    public:
        FerrisTreeModel_Impl();
        ~FerrisTreeModel_Impl();
        
        virtual void setURL( fh_context c );
        virtual void setURL( const std::string& s );
        virtual std::string getURL();
        virtual fh_context getRootContext();
        virtual void read(   bool force = false );
        void read( fh_context c, bool isRoot, bool force );

        virtual GtkTreeModel* getGtkModel();

        virtual void EnsureMonitoringSubContextCreationEvents( fh_context c, bool force = false );
        virtual void StartMonitoringSubContext( fh_context c );
        virtual void StopMonitoringSubContext(  fh_context c );
        
        virtual void OnExists ( NamingEvent_Exists* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn );
        virtual void OnCreated( NamingEvent_Created* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn );
        virtual void OnMedallionUpdated( fh_context c );
        virtual void OnChanged( NamingEvent_Changed* ev, std::string olddn, std::string newdn );
        virtual void OnDeleted( NamingEvent_Deleted* ev, std::string olddn, std::string newdn );
        virtual void OnMoved( NamingEvent_Moved* ev, std::string olddn, std::string newdn );

        /**
         * Used to propergate events about disk reading progress
         */
        void OnExistsDiskRead( NamingEvent_Exists* ev,
                               const fh_context& subc,
                               std::string olddn, std::string newdn );
        

        /**
         * Add the given context to the tree in its correct location
         */
        void add( Context::iterator ci, GtkTreeIter* iter, GtkTreePath* path );
        void add( Context::iterator ci );
        void add( fh_context c );
        
        /**
         * Convert a Gtk tree iterator to a Context::iterator
         */
        Context::iterator TreeIterToContext( GtkTreeIter *iter );

        /**
         * Get a Gtk tree iterator for a Context/Context::iterator
         */
        void ContextToTreeIter( GtkTreeIter *iter, Context::iterator ci, bool checkExistsInContextIterators );
        void ContextToTreeIter( GtkTreeIter *iter, Context::iterator ci );
        void ContextToTreeIter( GtkTreeIter *iter, fh_context c );
        GtkTreeIter toTreeIter( Context::iterator ci );
        GtkTreeIter toTreeIter( fh_context c );
        

        /**
         * Check to see if m_columnNames needs updating, and if so update the collection
         */
        void UpdateColumnNames();
        
        /**
         * Add new columns for any EA that is in 'c' but is not already a column in the model
         */
        void AddAllNewEANames( fh_context c );

        /**
         * Get the type of a column based on its name/index
         */
        virtual GType getColumnType( const string& cn );
        virtual GType getColumnType( int   index );

        /**
         * Get the name of the column at the given index
         */
        const string& getColumnName( int index );

        /**
         * Get the context for a given location in the tree
         */
        fh_context toContext( GtkTreeIter  *iter );
        fh_context toContext( gchar* path_string );
        fh_context toContext( GtkTreePath* path );
        
        

        /**
         * Lookup the number of a column by its name.
         *
         * @param tree_model This
         * @param s The name of the column to find the index of.
         *
         * @return the index from 0 up where the column with name 's' can be found at,
         *         or -1 if no column with the name 's' is being shown.
         */
        gint getColumnNumber( const std::string& s );

        
        /**
         * Set this to check if a directory has children and only display a triangle
         * opener if there are children
         */
        bool m_checkDirsForChildren;
        virtual void setCheckDirsForChildren( bool v )
            {
                m_checkDirsForChildren = v;
            }


        /**
         * Check if there exists one or more subdirs from each context and only dislay
         * a triangle for contexts who have subdirs.
         */
        bool m_checkForExistsSubDir;
        virtual void setCheckForExistsSubDir( bool v )
            {
                m_checkForExistsSubDir = v;
            }
        
        /**
         * Set the number of contexts that are to be read between emission of
         * getAddingContextSig() signal. Normal GtkTreeView use should leave
         * the default (somewhere around 100 at the moment) but other views
         * that are just using the model for its data house can set to emit
         * for each context
         */
        int m_addingContextSignalModulas;
        virtual void setAddingContextSignalModulas( int m )
            {
                m_addingContextSignalModulas = m;
            }
        

        
        
        /**
         * Root context node of the view
         */
        fh_context m_root;

        /**
         * When the root node has changed this flag is set to true so that
         * the schema for stateless attributes can be consulted and have its
         * result cached.
         */
        bool m_root_hasChanged_RecheckStatelessSchemas;

        /**
         * when m_root_hasChanged_RecheckStatelessSchemas is set to true
         * then setup the cache of the type for each column in the view
         * and on later calls to getColumnType() check the cache for the type
         * before assuming a string type.
         */
        typedef map< string, GType > m_columnTypeMap_t;
        m_columnTypeMap_t m_columnTypeMap;

        
        /**
         * Filtered context view
         */
        fh_context m_filteredContext;

        /**
         * Sorted context view of the view
         */
        fh_context m_sortedContext;

        /**
         * Gets the deepest context wrapper for the view.
         * They are always wrapped like this,
         * native
         * filtered
         * sorted
         */
        fh_context getViewContext();

        std::string m_sortingString;
        std::string getSortingString();
        void        setSortingString( const std::string& s, bool revalidate = true );

        /**
         * Called after setSortingString() to update the current view.
         */
        void        updateViewForSorting();

        /**
         * Very much like what sorting has
         */
        std::string m_filterString;
        virtual std::string getFilterString();
        virtual void        setFilterString( const std::string& s, bool revalidate = true );
        void updateViewForFiltering();
        
        
        /**
         * Only monitor the root context one time or none.
         */
        bool m_areMonitoringRoot;

        /**
         * Names of all the current columns, this can be more than just getSubContextNames()
         * because we add columns for [sub]children in the tree. ie. columnNames is the
         * union of all getSubContextNames() for all nodes viewed.
         */
        columnNames_t m_columnNames;

        /**
         * If the client wants m_columnNames to remain a static collection then they
         * can set this to be that collection. Methods which update m_columnNames
         * will make sure it is either empty or contains a copy of m_staticColumnNames
         */
        columnNames_t m_staticColumnNames;
        void setStaticColumnNames( const std::vector< std::string >& v )
            {
                m_staticColumnNames = v;
            }

        void
        appendColumnName( const std::string& s )
            {
                if( !m_staticColumnNames.empty() )
                    ensure_back( m_staticColumnNames, s );

                ensure_back( m_columnNames, s );
            }
        
        
        
        
        /**
         * Because GtkTreeIter doesn't let us keep any data we want, only 3x pointers
         * we keep a cache here and use GtkTreeIter->user_data = &ContextIterators[?];
         */
        ContextIterators_t m_ContextIterators;

        /**
         * URL of the root context
         */
        string              m_url;

        /**
         * The GTK object for this model
         */
        GtkFerrisTreeStore* m_gtkModel;


        /**
         * If true then get_value() returns dummy data
         */
        int m_viewIsBootstrappingNCols;

        /**
         * Playing with setting this in read() to the number of times to ignore
         * get_value()
         */
        guint32 m_bootstrapcount;
        
        /* SEE .hh file */
        virtual void setViewIsBootstrappingNCols( int ncols ) 
            {
                m_viewIsBootstrappingNCols = ncols;
            }
        
    private:

        /**
         * A cache to allow clear() to be called many times sequentially without
         * causing ill effects.
         */
        bool m_cleared;
        
        /**
         * For read operations this tracks how many contexts have been read
         */
        gint m_contextsDone;

        /**
         * For read operations this tracks what the estimated size is
         */
        gint m_subContextCountGuess;

        FerrisTreeModel_Impl( const FerrisTreeModel_Impl& );
        FerrisTreeModel_Impl operator=( const FerrisTreeModel_Impl& );

        struct SignalCollection
        {
            SignalCollection()
                :
                ref_count(0)
            {}
        
            int ref_count;
            sigc::connection ExistsConnection;
//            sigc::connection CreatedConnection;
            sigc::connection ChangedConnection;
            sigc::connection MedallionUpdatedConnection;
            sigc::connection DeletedConnection;
            sigc::connection MovedConnection;
        };
        struct SignalCollectionCreation
        {
            SignalCollectionCreation()
                :
                ref_count(0)
            {}
        
            int ref_count;
            sigc::connection CreatedConnection;
        };
        
        struct ContextCompare : private less<string>
        {
            typedef less<string> _Base;
            
            bool operator()( const fh_context& c1,
                             const fh_context& c2 ) const
                {
                    {
                        Context* cc1 = GetImpl( c1 );
                        Context* cc2 = GetImpl( c2 );
                        return cc1 < cc2;
                    }
//                    return _Base::operator()( c1->getURL(), c2->getURL() );
                }
        };

        typedef std::map< fh_context, SignalCollection, ContextCompare > SigCol_t;
        SigCol_t SigCol;
        typedef std::map< fh_context, SignalCollectionCreation, ContextCompare > SigColCreation_t;
        SigColCreation_t SigColCreation;

        /**
         * For filtered/sorted contexts we keep/use the deepest delegates only
         * to avoid having two fh_context pointers which are for the same file
         * but compare literally as pointers differently
         */
        fh_context SignalCollection_findDeepest( fh_context c )
            {
                if( ChainedViewContext* cvc = dynamic_cast<ChainedViewContext*>(GetImpl(c)))
                    return deepestDelegate( cvc );
                return c;
            }
        SignalCollection& getSignalCollection( fh_context c )
            {
                return SigCol[ SignalCollection_findDeepest(c) ];
            }
        SigCol_t::iterator findSignalCollection( fh_context c )
            {
                return SigCol.find( SignalCollection_findDeepest(c) );
            }
        SignalCollectionCreation& getSignalCollectionCreation( fh_context c )
            {
                return SigColCreation[ SignalCollection_findDeepest(c) ];
            }
        SigColCreation_t::iterator findSignalCollectionCreation( fh_context c )
            {
                return SigColCreation.find( SignalCollection_findDeepest(c) );
            }


//         SignalCollection& getSignalCollection( fh_context c )
//             {
//                 return SigCol[ c->getURL() ];
//             }
//         SigCol_t::iterator findSignalCollection( fh_context c )
//             {
//                 return SigCol.find( c->getURL() );
//             }
    };
};


#define G_NODE(node) ((GNode *)node)
#define VALID_ITER(iter, tree_store) \
(iter!= NULL && tree_store->stamp == iter->stamp && iter->user_data)

static GObjectClass *parent_class = NULL;
using namespace FerrisUI;

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

string tostr( GtkTreePath* path )
{
    if( !path )
    {
        return "<no path>";
    }
    
    char* p = gtk_tree_path_to_string( path );
    string ret = p;
    g_free(p);
    return ret;
}

static void
InvalidateIter( GtkTreeIter  *iter )
{
    if( iter )
    {
        iter->user_data  = 0;
        iter->user_data2 = 0;
        iter->user_data3 = 0;
        iter->stamp      = 0;
    }
}

static void
InvalidateIter( GtkTreeIter& iter )
{
    InvalidateIter( &iter );
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

static GtkTreeModelFlags
gtk_ferris_tree_store_get_flags (GtkTreeModel *tree_model)
{
    g_return_val_if_fail (GTK_IS_FERRIS_TREE_STORE (tree_model), GtkTreeModelFlags(0));
//    return GtkTreeModelFlags(0);
    return GtkTreeModelFlags(GTK_TREE_MODEL_ITERS_PERSIST);
}


static void
gtk_ferris_tree_store_finalize (GObject *object)
{
    GtkFerrisTreeStore *t = GTK_FERRIS_TREE_STORE( object );
    (*parent_class->finalize) (object);
}


static void
gtk_ferris_tree_store_class_init (GtkFerrisTreeStoreClass *c)
{
    parent_class = (GObjectClass*)g_type_class_peek_parent(c);
    GObjectClass *object_class = (GObjectClass *)c;

    object_class->finalize = gtk_ferris_tree_store_finalize;
}

static void
gtk_ferris_tree_store_init (GtkFerrisTreeStore *tree_store)
{
    GtkFerrisTreeStore *t = GTK_FERRIS_TREE_STORE (tree_store);

    tree_store->ColumnCount = 0;
    tree_store->root = g_node_new (NULL);
    for( tree_store->stamp = 0; !tree_store->stamp; )
    {
        tree_store->stamp = g_random_int ();
    }
}


GtkFerrisTreeStore*
gtk_ferris_tree_store_new()
{
    GtkFerrisTreeStore* retval =
        GTK_FERRIS_TREE_STORE(
            g_object_new(GTK_TYPE_FERRIS_TREE_STORE, NULL));
    
    return retval;
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

static gboolean
gtk_ferris_tree_store_drag_data_get( GtkTreeDragSource *drag_source,
                                     GtkTreePath       *path,
                                     GtkSelectionData  *selection_data )
{
    g_return_val_if_fail       (GTK_IS_FERRIS_TREE_STORE (drag_source), 0);
    GtkFerrisTreeStore*    t = GTK_FERRIS_TREE_STORE (drag_source);
    FerrisTreeModel_Impl*  d = t->d;


    LG_GTKFERRIS_D << "gtk_ferris_tree_store_drag_data_get() path:" << tostr(path) << endl;
    
    /* Note that we don't need to handle the GTK_TREE_MODEL_ROW
     * target, because the default handler does it for us, but
     * we do anyway for the convenience of someone maybe overriding the
     * default handler.
     */

    if (gtk_tree_set_row_drag_data (selection_data,
                                    GTK_TREE_MODEL (drag_source),
                                    path))
    {
        return TRUE;
    }
    else
    {
//         if( info == TARGET_ROOTWIN )
//             g_print ("I was dropped on the rootwin\n");
//         else

        char dummydata[100];
        strcpy( dummydata, "I'm the sample" );
        
        if( selection_data->target == gdk_atom_intern ("STRING", FALSE) ||
            selection_data->target == gdk_atom_intern ("text/plain", FALSE) )
        {
//             gtk_selection_data_set( selection_data,
//                                      selection_data->target,
//                                     8,
//                                     (guchar*)dummydata, strlen(dummydata)+1 );

            fh_context c = d->toContext( path );
            gtk_selection_data_set( selection_data,
                                     selection_data->target,
                                    8,
                                    (guchar*)c->getURL().c_str(),
                                    c->getURL().length() + 1 );
        }
    }

    return FALSE;
}

static gboolean
gtk_ferris_tree_store_drag_data_delete( GtkTreeDragSource *drag_source,
                                        GtkTreePath       *path )
{
    g_return_val_if_fail       (GTK_IS_FERRIS_TREE_STORE (drag_source), 0);
    GtkFerrisTreeStore*    t = GTK_FERRIS_TREE_STORE (drag_source);
    FerrisTreeModel_Impl*  d = t->d;
    GtkTreeIter iter;
    
    if (gtk_tree_model_get_iter (GTK_TREE_MODEL (drag_source),
                                 &iter,
                                 path))
    {
        LG_GTKFERRIS_D << " drag_del() path:" << tostr(path) << endl;
        gtk_tree_model_row_deleted( d->getGtkModel(), path );
        
//         gtk_tree_store_remove (GTK_TREE_STORE (drag_source),
//                                &iter);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static void
gtk_ferris_tree_store_drag_source_init (GtkTreeDragSourceIface *iface)
{
    iface->drag_data_delete = gtk_ferris_tree_store_drag_data_delete;
    iface->drag_data_get    = gtk_ferris_tree_store_drag_data_get;
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

static gint
gtk_ferris_tree_store_get_n_columns (GtkTreeModel *tree_model)
{
    g_return_val_if_fail       (GTK_IS_FERRIS_TREE_STORE (tree_model), 0);
    GtkFerrisTreeStore*    t = GTK_FERRIS_TREE_STORE (tree_model);
    FerrisTreeModel_Impl*  d = t->d;
    
    LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_n_columns" << endl;
    d->UpdateColumnNames();
    return d->m_columnNames.size();
}

static GType
gtk_ferris_tree_store_get_column_type (GtkTreeModel *tree_model,
                                       gint          index)
{
    g_return_val_if_fail       (GTK_IS_FERRIS_TREE_STORE (tree_model), 0);
    GtkFerrisTreeStore*    t = GTK_FERRIS_TREE_STORE (tree_model);
    FerrisTreeModel_Impl*  d = t->d;

    LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_column_type index:" << index << endl;
    return d->getColumnType( index );
}

static gboolean
gtk_ferris_tree_store_get_iter( GtkTreeModel *tree_model,
                                GtkTreeIter  *iter,
                                GtkTreePath  *path )
{
    g_return_val_if_fail       (GTK_IS_FERRIS_TREE_STORE (tree_model), 0);
    GtkFerrisTreeStore*    t = GTK_FERRIS_TREE_STORE (tree_model);
    FerrisTreeModel_Impl*  d = t->d;

    LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_iter() path:" << tostr(path) << endl;

    InvalidateIter( iter );

    gint *indices = gtk_tree_path_get_indices( path );
    gint  depth   = gtk_tree_path_get_depth(   path );
    gint  i;

    LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_iter() depth:" << depth
                   << " index[0]:" << indices[0]
                   << endl;
    
    g_return_val_if_fail (depth > 0, FALSE);
    
    if( !isBound( d->getViewContext() ) )
    {
        stringstream ss;
        ss << "gtk_ferris_tree_store_get_iter path:" << tostr(path)
           << " root context is not bound!" << endl;
        LG_GTKFERRIS_D << tostr(ss) << endl;
        return false;
    }

    /*
     * Try to make the usual cases that little bit faster
     */
    if( depth==1 && indices[0] == 0 )
    {
        if( !d->getViewContext()->hasSubContexts() )
            return false;
        
        d->ContextToTreeIter( iter, d->getViewContext()->begin() );
        return true;
    }
    
    

    fh_context parentc = d->getViewContext();
    Context::iterator ci = parentc->begin();
    if( ci == parentc->end() )
        return false;

    for (i = 0; i < depth; ++i )
    {
        gint index = indices[i];

        LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_iter() path:" << tostr(path)
                       << " i:" << i
                       << " index:" << index
                       << " subc:" << parentc->SubContextCount()
                       << endl;
        
        if( index < 0 || index >= parentc->SubContextCount() )
        {
            LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_iter() path:" << tostr(path)
                           << " out-of-range!"
                           << " index:" << index
                           << " child-count:" << parentc->SubContextCount()
                           << endl;
            return false;
        }

        LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_iter() path:" << tostr(path)
                       << " index:" << index
                       << " distance:" << distance( ci, parentc->end() )
//                       << " mydistance:" << mydistance( ci, parentc->end() )
                       << endl;
        
        ci = parentc->begin();
        if( ci == parentc->end() )
            return false;
        if( distance( ci, parentc->end() ) < index )
        {
            LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_iter() index:" << index
                           << " is too large!"
                           << endl;
            return false;
        }
        
        
        ci += index;
        parentc = *ci;
    }

    d->ContextToTreeIter( iter, ci );
    return true;
}

fh_context getRootContext( fh_context c )
{
    while( c->isParentBound() )
    {
        c = c->getParent();
    }
    return c;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


static GtkTreePath*
gtk_ferris_tree_store_get_path( GtkTreeModel *tree_model,
                                GtkTreeIter  *iter )
{
    g_return_val_if_fail       (GTK_IS_FERRIS_TREE_STORE (tree_model), 0);
    GtkFerrisTreeStore*    t = GTK_FERRIS_TREE_STORE (tree_model);
    FerrisTreeModel_Impl*  d = t->d;

    g_return_val_if_fail (iter != NULL, NULL);
    g_return_val_if_fail (VALID_ITER (iter, t), FALSE);

    GtkTreePath *ret = 0;
    string path;

    Context::iterator ci = d->TreeIterToContext( iter );
    if( !isBound( *ci ) )
    {
        LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_path() called with an unbound gtk iter"
                       << endl;
        g_on_error_query(0);
        return 0;
    }

    LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_path(top) ci:" << (*ci)->getURL()
                   << endl;
//    (*ci)->dumpRefDebugData( Factory::fcerr() );
    
    while( ci->isParentBound() )
    {
        /*
         * Walk from the iter context up to the root node appending :offset for
         * each level we walk up
         */
        fh_context p = ci->getParent();
        int offset=0;
        string rdn = ci->getDirName();

//         if( d->m_sortedContext && p->getURL() == d->m_sortedContext->getURL() )
//             p = d->m_sortedContext;

        LG_GTKFERRIS_D << "get_path() ci:" << (*ci)->getURL()
                       << " rdn:" << rdn
                       << " p:" << p->getURL()
                       << " viewp:" << d->getViewContext()->getURL()
                       << endl;

//         /* PURE DEBUG */
//         cerr << "--ci-------------=============-------------------" << endl;
//         (*ci)->dumpOutItems();
//         cerr << "--p-------------=============-------------------" << endl;
//         p->dumpOutItems();
//         cerr << "--d->view-------------=============-------------------" << endl;
//         d->getViewContext()->dumpOutItems();
//         cerr << "--done-------------=============-------------------" << endl;
            
//         /* PURE DEBUG */
//         for( Context::iterator ti = p->begin(); ti != p->end(); ++ti )
//         {
//             cerr << "get_path() eq:" << (ti == ci)
//                            << " *eq:" << (*ti == *ci)
//                            << " ti:" << (*ti)->getURL()
//                            << endl;
//             if( (*ti)->getDirName() == "play" )
//                 (*ti)->dumpOutItems();
//         }
//         for( Context::iterator ti = d->getViewContext()->begin();
//              ti != d->getViewContext()->end(); ++ti )
//         {
//             cerr << "get_path(2) eq:" << (ti == ci)
//                            << " *eq:" << (*ti == *ci)
//                            << " ti:" << (*ti)->getURL()
//                            << endl;
//             (*ti)->dumpOutItems();
//         }

//         cerr << "get_path. testing an inline version of distance()" << endl;
//         {
//             int n = 0;
//             Context::iterator first = p->begin();
//             Context::iterator last  = ci;

//             Context::iterator it = first;
//             while( it != last )
//             {
//                 cerr << "get_path(3) iter:" << toVoid( GetImpl( *it ) )
//                      << " eq:" << (it == last)
//                      << " *eq:" << (*it == *last)
//                      << " ti:" << (*it)->getURL()
//                      << endl;
                
//                 ++it; ++n;
//             }
//         }
        
//         /* PURE DEBUG */
//         if( distance( p->begin(), ci ) < 0 )
//         {
//             cerr << "gtk_get_path() negative distance() p.begin():" << (*p->begin())->getURL() << endl;
//             cerr << "gtk_get_path() negative distance() ci:" << (*ci)->getURL() << endl;
//             cerr << "gtk_get_path() negative distance(), in order traversal of parent follows." << endl;
//             for( Context::iterator ti = p->begin(); ti != p->end(); ++ti )
//             {
//                 cerr << "get_path(5) ti:" << toVoid( GetImpl( *ti ) )
//                      << " ti:" << (*ti)->getURL()
//                      << endl;
//             }
//         }
        
        
//         /* PURE DEBUG */
//         cerr << "gtk_ferris_tree_store_get_path() parent size:" << p->SubContextCount() << endl;
//         Context::iterator founditer = p->find( rdn );
//         if( founditer == p->end() )
//         {
//             LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_path() "
//                            << " founditer == p->end() "
//                            << endl;
//             g_on_error_query(0);
//             return 0;
//         }
//         {
//             int i=0;
//             for( Context::iterator it = p->begin(); it != p->end(); ++it, ++i )
//             {
//                 if( (*it)->getDirName() == (*ci)->getDirName() )
//                 {
//                     cerr << "Found ci in the collection. i:" << i
//                          << endl;
//                 }
//             }
//             cerr << "testing for distance now" << endl;
//             Context::iterator bi = p->begin();
//             i = distance( bi, ci );
//             cerr << "testing for distance now 2" << endl;
            
//         }
        
        offset = distance( p->begin(), ci );
//         offset = 0;
//         Context::iterator tmpiter = p->begin();
//         while( (*tmpiter)->getDirName() != (*ci)->getDirName() && tmpiter != p->end() )
//         {
//             ++offset;
//             ++tmpiter;
//         }
        
        
        LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_path(offset) ci:" << (*ci)->getURL()
                       << " offset:" << offset
                       << " end-offset:" << (distance( p->begin(), p->end()))
                       << endl;
//         {
//             for( Context::iterator dci = p->begin(); dci != p->end(); ++dci )
//             {
//                 LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_path(dump) dci:" << (*dci)->getURL() << endl;
//             }

//             if( d->m_sortedContext )
//             {
//                 LG_GTKFERRIS_D << "------------------------------" << endl;
//                 int o2 = distance( d->m_sortedContext->begin(), ci );
//                 LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_path(offset) ci:" << (*ci)->getURL()
//                                << " o2:" << o2
//                                << endl;
//                 {
//                     Context::iterator dci = d->m_sortedContext->begin();
//                     int o3 = 0;
//                     while( dci != ci && dci != d->m_sortedContext->end() )
//                     {
//                         ++dci;
//                         ++o3;
//                     }
//                     LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_path(offset) ci:" << (*ci)->getURL()
//                                    << " o3:" << o3
//                                    << endl;
                    
//                 }
//                 {
//                     Context::iterator dci = p->begin();
//                     int o4 = 0;
//                     while( dci != ci && dci != p->end() )
//                     {
//                         ++dci;
//                         ++o4;
//                     }
//                     LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_path(offset) ci:" << (*ci)->getURL()
//                                    << " o4:" << o4
//                                    << endl;
                    
//                 }
                
//                 for( Context::iterator dci = d->m_sortedContext->begin(); dci != d->m_sortedContext->end(); ++dci )
//                 {
//                     LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_path(dump) dci:" << (*dci)->getURL() << endl;
//                 }
//                 LG_GTKFERRIS_D << "------------------------------" << endl;
//             }
            
//         }


        


//         {
//             offset = 0;
//             Context::iterator iter = p->begin();
//             Context::iterator end  = p->end();
            
//             for( offset = 0; iter != end; ++iter )
//             {
//                 if( *iter == *ci )
//                     break;
//                 ++offset;
//             }
//             if( iter == end )
//             {
//                 cerr << "ERROR: can't find context iterator for object in the tree!" << endl;
//                 BackTrace();
//             }
//         }

        
//         {
//             fh_context tc = Resolve( d->getViewContext()->getURL() );
            
//             LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_path() " << endl
//                            << "      p.url:" << p->getURL() << endl
//                            << "     vc.url:" << d->getViewContext()->getURL() << endl
//                            << " vc.rr(url):" << tc->getURL() << endl
//                            << "     ci.url:" << (*ci)->getURL() << endl
//                            << endl;
//         }
        
        
        if( path.length() )
        {
            path = ":" + path;
        }
        path = tostr( offset ) + path;

        if( p->getURL() == d->getViewContext()->getURL()
            || (*ci)->getURL() == d->getViewContext()->getURL() )
        {
            break;
        }


        // Handle VirtualSoftLinks properly
        if( ChainedViewContext* dc =
            dynamic_cast<ChainedViewContext*>( GetImpl(d->getViewContext()) ))
        {
//            if( GetImpl( p ) == GetImpl( dc->getDelegate() ) )
            if( GetImpl( p ) == deepestDelegate( dc ) )
            {
                LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_path() is VirtualSoftLink...."
                               << " path:" << path
                               << " p:" << p->getURL()
                               << " given:" << (*d->TreeIterToContext( iter ))->getURL()
                               << " m_root:" << d->getViewContext()->getURL()
                               << endl;
                break;
            }
        }
        
        if( !p->isParentBound() )
        {
            LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_path() !p->isParentBound() "
                           << " path:" << path
                           << " p:" << p->getURL()
                           << " given:" << (*d->TreeIterToContext( iter ))->getURL()
                           << " m_root:" << d->getViewContext()->getURL()
                           << endl;
            cerr << "gtk_ferris_tree_store_get_path() !p->isParentBound() "
                 << " path:" << path
                 << " p:" << p->getURL()
                 << " given:" << (*d->TreeIterToContext( iter ))->getURL()
                 << " m_root:" << d->getViewContext()->getURL()
                 << endl;
            BackTrace();
            g_on_error_query(0);
        }
        
        ci = toContextIterator( p );
    }

//     LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_path(dump start)" << endl;
//     for( Context::iterator i = (*ci)->getParent()->begin();
//          i != (*ci)->getParent()->end(); ++i )
//     {
//         LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_path(dump i):" << (*i)->getURL() << endl;
//     }
    
    LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_path(RET) ci:" << (*ci)->getURL()
                   << " returning path string:" << path
                   << endl;

    return path.length()
        ? gtk_tree_path_new_from_string( path.c_str() )
        : 0;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

/**
 * get the pixbuf that describes this context's type
 */
static GdkPixbuf*
getTreeIconPixbuf( fh_context c, const std::string& n )
{
    typedef map< string, GdkPixbuf* > cache_t;
    static cache_t cache;

    LG_GTKFERRIS_D << "getTreeIconPixbuf() n:" << n << " c:" << c->getURL() << endl;
    LG_GTKFERRIS_D << "rdn:" << n.substr( 0, n.length() - strlen("-pixbuf") ) << endl;
    
    string iconpath = getStrAttr( c,
                                  n.substr( 0, n.length() - strlen("-pixbuf") ),
                                  "icons://ferris-mu-file.png" );
//    cerr << "getTreeIconPixbuf() c:" << c->getURL() << " n:" << n << " icon:" << iconpath << endl;
    LG_GTKFERRIS_D << " icon:" << iconpath << endl;
    
    cache_t::iterator iter = cache.find( iconpath );
    if( iter == cache.end() )
    {
        GdkPixbuf* pb = gdk_pixbuf_new_from_file( resolveToIconPath( iconpath ).c_str(), 0 );
        iter = cache.insert( make_pair( iconpath, pb ) ).first;
    }

    return iter->second;
}


struct ltem
{
  bool operator()( const emblems_t& e1, const emblems_t& e2) const
  {
      int e1sz = e1.size();
      int e2sz = e2.size();

      if( e1sz != e2sz )
          return e1sz < e2sz;

      emblems_t::const_iterator iter1 = e1.begin();
      emblems_t::const_iterator iter2 = e2.begin();

      for( ; iter1 != e2.end(); ++iter1, ++iter2 )
      {
          string u1 = (*iter1)->getUniqueName();
          string u2 = (*iter2)->getUniqueName();
          if( u1 != u2 )
              return u1 < u2;
      }
      return 0;
  }
};


static GdkPixbuf*
getFerrisIconNamePixbuf( fh_context c )
{
    static const int desired_width  = 64;
    static const int desired_height = 64;
    static const GdkInterpType interp_type = GDK_INTERP_HYPER;
    typedef map< string, GdkPixbuf* > cache_t;
    static cache_t cache;

    cerr << "getFerrisIconNamePixbuf() c:" << c->getURL() << endl;
    string iconpath = getStrAttr( c, "ferris-iconname", "icons://ferris-mu-file.png" );
    if( ends_with( iconpath, ".edj" ))
        iconpath = iconpath.substr( 0, iconpath.length() - 4 ) + ".png";
    cerr << " icon:" << iconpath << endl;
    
    cache_t::iterator iter = cache.find( iconpath );
    if( iter == cache.end() )
    {
        GdkPixbuf* pb = gdk_pixbuf_new_from_file( resolveToIconPath( iconpath ).c_str(), 0 );
        GdkPixbuf* scaled = gdk_pixbuf_scale_simple( pb,
                                                     desired_width,
                                                     desired_height,
                                                     interp_type );
        gdk_pixbuf_unref( pb );
        pb = scaled;
            
        iter = cache.insert( make_pair( iconpath, pb ) ).first;
    }

    return iter->second;
}

    
/**
 * Create a composition of all the emblems that this context has
 * the emblem pixbufs are aggressively cached so that showing
 * contexts should still be acceptably fast.
 */
static GdkPixbuf*
getEmblemsCompositionPixbuf( fh_context c )
{
    static const int desired_width  = 24;
    static const int desired_height = 24;
    static const GdkInterpType interp_type = GDK_INTERP_HYPER;
    
//     typedef map< fh_emblem, GdkPixbuf* > emblemPixbufs_t;
//     emblemPixbufs_t emblemPixbufs;
    
//     if( emblemPixbufs.empty() )
//     {
//         fh_etagere et = Factory::getEtagere();
//         emblems_t  el = et->getAllEmblems();
//         for( emblems_t::iterator ei = el.begin(); ei != el.end(); ++ei )
//         {
//             fh_emblem em = *ei;
//             string iconname = resolveToIconPath( em->getIconName() );
//             GdkPixbuf* pb = gdk_pixbuf_new_from_file( iconname.c_str(), 0 );

//             GdkPixbuf* scaled = gdk_pixbuf_scale_simple( pb,
//                                                          desired_width,
//                                                          desired_height,
//                                                          interp_type );
//             emblemPixbufs.insert( make_pair( em, scaled ) );
//             gdk_pixbuf_unref( pb );
//         }
//     }

    // FIXME: medallionCache is useless at current.
    typedef map< emblems_t, GdkPixbuf*, ltem > medallionCache_t;
    medallionCache_t medallionCache;

//     cerr << "getEmblemsCompositionPixbuf(top) c:" << c->getURL()
//          << " get-ea-med:" << getStrAttr( c, "emblem:has-medallion", "foo" )
//          << " has-medallion:" << c->hasMedallion()
//          << endl;
    
    if( !c->hasMedallion() )
    {
        return 0;
    }
    
    emblems_t specificEmblems = c->getMedallion()->getMostSpecificEmblems();
    medallionCache_t::iterator miter = medallionCache.find( specificEmblems );
    if( medallionCache.end() != miter )
    {
        return miter->second;
    }

    if( specificEmblems.empty() )
        return 0;

    LG_GTKFERRIS_D << "getEmblemsCompositionPixbuf(2) c:" << c->getURL()
                   << " specificEmblems.sz:" << specificEmblems.size() << endl;
    //
    // we need to create a new horizontal composition of emblems
    // and add that to the cache.
    //
    GdkPixbuf* pb = gdk_pixbuf_new( GDK_COLORSPACE_RGB, 1, 8,
                                    specificEmblems.size() * desired_width,
                                    desired_height );

    int x_grid_offset = 0;
    for( emblems_t::iterator ei = specificEmblems.begin(); ei != specificEmblems.end(); ++ei )
    {
        fh_emblem em = *ei;
//        GdkPixbuf* em_pixbuf = emblemPixbufs[em];
        GdkPixbuf* em_pixbuf = getEmblemListViewPixbuf( em );

        if( em_pixbuf )
        {
            gdk_pixbuf_copy_area( em_pixbuf, 0, 0, desired_width, desired_height,
                                  pb, x_grid_offset * desired_width, 0 );
            ++x_grid_offset;
        }
    }

    medallionCache[ specificEmblems ] = pb;
    return pb;
}



static void
gtk_ferris_tree_store_get_value( GtkTreeModel *tree_model,
                                 GtkTreeIter  *iter,
                                 gint          column,
                                 GValue       *value )
{
    g_return_if_fail           (GTK_IS_FERRIS_TREE_STORE (tree_model));
    GtkFerrisTreeStore*    t = GTK_FERRIS_TREE_STORE (tree_model);
    FerrisTreeModel_Impl*  d = t->d;

    g_return_if_fail (iter != NULL);
    g_return_if_fail (VALID_ITER (iter, t));

    LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_value( c:" << column << " )" << endl;
    if( column < 0 || column > d->m_columnNames.size() )
    {
        LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_value( c:" << column << " )"
                       << " column number out of range."
                       << " size:" << d->m_columnNames.size()
                       << endl;
        g_value_init( value, G_TYPE_STRING );
        return;
    }

    const string& cn  = d->getColumnName( column );
    GType         gt  = d->getColumnType( cn );
    
    if( isColumnFixedSize( cn ) && d->m_bootstrapcount > 0 )
    {
        --d->m_bootstrapcount;
        LG_GTKFERRIS_D << "++++ get_value() short cut m_bootstrapcount:" << d->m_bootstrapcount << endl;

        switch( gt )
        {
        case G_TYPE_STRING:
            g_value_init( value, G_TYPE_STRING );
            g_value_set_string( value, "" );
            return;
        case G_TYPE_BOOLEAN:
            g_value_init( value, G_TYPE_BOOLEAN );
            g_value_set_boolean( value, 0 );
            return;
        case G_TYPE_OBJECT:
        {
            static string medallion_dummyIcon = "icons://ferris-mu-medallion-max-size-filler.png";
            static GdkPixbuf* medallion_pb = gdk_pixbuf_new_from_file( resolveToIconPath( medallion_dummyIcon ).c_str(), 0 );
            static string treeicon_dummyIcon = "icons://ferris-mu-treeicon-max-size-filler.png";
            static GdkPixbuf* treeicon_pb = gdk_pixbuf_new_from_file( resolveToIconPath( treeicon_dummyIcon ).c_str(), 0 );

            GdkPixbuf* pb = medallion_pb;
            if( cn == "treeicon-pixbuf" )
                pb = treeicon_pb;

            g_value_init( value, G_TYPE_OBJECT );
            g_value_set_object( value, pb );
            return;
        }
        }
    }

    fh_context    ctx = d->toContext( iter );
    LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_value() cn:" << cn << " gt:" << gt << endl;
    LG_GTKFERRIS_D << "gtk_ferris_tree_store_get_value() ctx:" << ctx->getDirPath() << endl;

    if( gt == G_TYPE_OBJECT )
    {
        g_value_init( value, G_TYPE_OBJECT );

        if( cn == emblems_pixbuf_cn )
        {
            g_value_set_object( value, getEmblemsCompositionPixbuf( ctx ) );
        }
        else if( cn == ferris_iconname_pixbuf_cn )
        {
            g_value_set_object( value, getFerrisIconNamePixbuf( ctx ));
        }
        else
        {
            g_value_set_object( value, getTreeIconPixbuf( ctx, cn ));
        }
        return;
    }
    

    string s = getStrAttr( ctx, cn, "" );

    switch( gt )
    {
    case G_TYPE_STRING:
        g_value_init( value, G_TYPE_STRING );
        g_value_set_string( value, s.c_str() );
        return;
    case G_TYPE_BOOLEAN:
        g_value_init( value, G_TYPE_BOOLEAN );
        g_value_set_boolean( value, isTrue(s) );
        return;
    }
}

/*
 * Perform a lookup to get the gtk path from the current iterator
 * (should be fast) and then increment the path and lookup the
 * context from the new path and give that back to them.
 *
 * Note that using Context::items directly is not a good idea because
 * other facades might have sorted/filtered the view and they don't
 * always keep context::items in that order.
 */
static gboolean
gtk_ferris_tree_store_iter_next( GtkTreeModel *tree_model, GtkTreeIter *iter )
{
    g_return_val_if_fail       (GTK_IS_FERRIS_TREE_STORE (tree_model), 0);
    GtkFerrisTreeStore*    t = GTK_FERRIS_TREE_STORE (tree_model);
    FerrisTreeModel_Impl*  d = t->d;
    
    g_return_val_if_fail (VALID_ITER (iter, t), FALSE);

//    Time::Benchmark bm( "tree_store_iter_next" );
    
    Context::iterator ci = d->TreeIterToContext( iter );
    fh_context oldc = *ci;
    ++ci;

    LG_GTKFERRIS_D << "iter_next(called)"
//                   << " n_children:" << gtk_tree_model_iter_n_children( tree_model, 0 )
                   << endl;
    
    if( ci == oldc->getParent()->end() )
    {
        LG_GTKFERRIS_D << "iter_next() found end node"
                       << " last:" << oldc->getURL()
                       << endl;
//         for( Context::iterator ci = oldc->getParent()->begin();
//              ci != oldc->getParent()->end(); ++ci )
//         {
//             LG_GTKFERRIS_D << " ci:" << (*ci)->getURL() << endl;
//         }
        InvalidateIter( iter );
        return false;
    }
    else
    {
        d->EnsureMonitoringSubContextCreationEvents( *ci );
    }
    

    LG_GTKFERRIS_D << "iter_next()"
                   << " last:" << oldc->getURL()
                   << " new:" << (*ci)->getURL()
                   << endl;
    
    d->ContextToTreeIter( iter, ci );
    return true;
}


/*
 * Note that we don't want to use context::items because it might not
 * be sorted as we expect
 */
static gboolean
gtk_ferris_tree_store_iter_children( GtkTreeModel *tree_model,
                                     GtkTreeIter  *iter,
                                     GtkTreeIter  *parent )
{
    try
    {
        
        g_return_val_if_fail       (GTK_IS_FERRIS_TREE_STORE (tree_model), 0);
        GtkFerrisTreeStore*    t = GTK_FERRIS_TREE_STORE (tree_model);
        FerrisTreeModel_Impl*  d = t->d;

        g_return_val_if_fail (VALID_ITER (parent, t), FALSE);
        InvalidateIter( iter );
        LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_children()" << endl;

#ifdef DISABLE_SUBTREE_SUPPORT
        LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_children(no subtree support)" << endl;
        /* FIXME: DEBUG SHORT RETURN */
        return false;
#endif
    

        fh_context p = d->toContext( parent );
        bool originallyHadReadDir = p->getHaveReadDir();
        LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_children() p:" << toVoid(p)
                       << " p:" << p->getURL() << endl;
        if( d->m_root )
            LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_children() root:" << toVoid(d->m_root)
                           << " m_root:" << d->m_root->getURL() << endl;

        if( !originallyHadReadDir )
        {
            LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_children()"
                           << " originallyHadReadDir:" << originallyHadReadDir
                           << " p:" << p->getURL()
                           << endl;
            d->read( p, false, false );

            /*
             * We need to monitor all directory nodes in case they go to/from
             * having zero files in them
             */
            for( Context::iterator i = p->begin(); i != p->end(); ++i )
            {
                fh_context c = *i;

                if( toint(getStrAttr( c, "is-dir", "0" )))
                {
                    LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_children()"
                                   << " start monitoring subcontext"
                                   << " c:" << c->getURL()
                                   << endl;
                    d->StartMonitoringSubContext( c );
                }
            }
        }
    
    
        Context::iterator childi = p->begin();
        if( childi == p->end() )
        {
            LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_children() p:" << p->getURL()
                           << " has no child" << endl;
        
            /* Force the subcontext count guess to return 0 */
            p->read( true );
            GtkTreePath* path = gtk_tree_model_get_path( GTK_TREE_MODEL (t), parent );
            gtk_tree_model_row_has_child_toggled( GTK_TREE_MODEL (t),
                                                  path,
                                                  parent );
            gtk_tree_path_free( path );
        
            return false;
        }

        if( !originallyHadReadDir )
            d->AddAllNewEANames( *childi );

        d->EnsureMonitoringSubContextCreationEvents( *childi );

        d->ContextToTreeIter( iter, childi );
        return true;
    }
    catch( exception& e )
    {
        stringstream ss;
        ss << "Error:" << e.what() << endl;
        RunErrorDialog( ss.str(), 0 );
        return false;
    }
}

static gboolean
gtk_ferris_tree_store_iter_has_child( GtkTreeModel *tree_model,
                                      GtkTreeIter  *iter )
{
    g_return_val_if_fail       (GTK_IS_FERRIS_TREE_STORE (tree_model), 0);
    GtkFerrisTreeStore*    t = GTK_FERRIS_TREE_STORE (tree_model);
    FerrisTreeModel_Impl*  d = t->d;

#ifdef DISABLE_SUBTREE_SUPPORT
    LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_children(no subtree support)" << endl;
    /* FIXME: DEBUG SHORT RETURN */
    return false;
#endif
    
    LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_has_child()" << endl;
    try
    {
        g_return_val_if_fail (VALID_ITER (iter, t), FALSE);

        fh_context iterc = d->toContext( iter );
        LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_has_child() "
                       << " iter:" << iterc->getDirPath()
                       << " m_checkDirsForChildren:" << d->m_checkDirsForChildren
                       << " haschild:"
                       << toint( getStrAttr( iterc, "has-subcontexts-guess", "1" ))
                       << endl;

        if( d->m_checkDirsForChildren )
        {
            return iterc->begin() != iterc->end();
        }

        if( d->m_checkForExistsSubDir )
        {
            LG_GTKFERRIS_D << "tree_store_iter_has_child() c:" << iterc->getURL()
                 << " hasSub:" << toint( getStrAttr( iterc, "exists-subdir", "1" ))
                 << endl;
            return toint( getStrAttr( iterc, "exists-subdir", "1" ));
        }
        return toint( getStrAttr( iterc, "has-subcontexts-guess", "1" ));
    }
    catch(...)
    {
    }
    
    LG_GTKFERRIS_D << "has_child() e" << endl;
    return FALSE;
}

static gint
gtk_ferris_tree_store_iter_n_children( GtkTreeModel *tree_model,
                                       GtkTreeIter  *iter )
{
    g_return_val_if_fail       (GTK_IS_FERRIS_TREE_STORE (tree_model), 0);
    GtkFerrisTreeStore*    t = GTK_FERRIS_TREE_STORE (tree_model);
    FerrisTreeModel_Impl*  d = t->d;

#ifdef DISABLE_SUBTREE_SUPPORT
    LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_children(no subtree support)" << endl;
    /* FIXME: DEBUG SHORT RETURN */
    return 0;
#endif    

    LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_n_children()" << endl;

    fh_context iterc = 0;
    if( !iter )
    {
        return isBound( d->getViewContext() );
    }
    else
    {
        g_return_val_if_fail (VALID_ITER (iter, t), 0);
        iterc = d->toContext( iter );
    }
    LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_n_children()"
                   << " iterc:" << iterc->getDirPath()
                   << endl;
    try
    {
        int ret = distance( iterc->begin(), iterc->end() );
        LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_n_children() ret:" << ret << endl;
        return ret;
    }
    catch(...)
    {
        return 0;
    }
}

static gboolean
gtk_ferris_tree_store_iter_nth_child( GtkTreeModel *tree_model,
                                      GtkTreeIter  *iter,
                                      GtkTreeIter  *parent,
                                      gint          n )
{
    g_return_val_if_fail       (GTK_IS_FERRIS_TREE_STORE (tree_model), 0);
    GtkFerrisTreeStore*    t = GTK_FERRIS_TREE_STORE (tree_model);
    FerrisTreeModel_Impl*  d = t->d;
    
    LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_nth_child( n:" << n << " )" << endl;

    InvalidateIter( iter );

    LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_nth_child() "
                   << " n:" << n
                   << endl;
    if( VALID_ITER (parent, t) )
    {
        LG_GTKFERRIS_D << " parent:" << d->toContext( parent )->getURL() << endl;
    }
    
    
    if( !parent )
    {
        LG_GTKFERRIS_D << " no parent, getting"
                       << " nth:" << n << " child"
                       << " root:" << d->getViewContext()->getURL()
                       << endl;
        
        Context::iterator ci = d->getViewContext()->begin();

//         ///////////////////////////////////////////////
//         // PURE DEBUG
//         {
//             LG_GTKFERRIS_D << "MANUAL distance finding..." << endl;
//             Context::iterator iter = d->getViewContext()->begin();
//             Context::iterator e = d->getViewContext()->end();
//             int count = 0;
//             for( ; iter != e ; ++iter )
//             {
//                 LG_GTKFERRIS_D << "iter:" << (*iter)->getURL() << endl;
//                 ++count;
//             }
//             LG_GTKFERRIS_D << "MANUAL distance from ci to end() is:" << count
//                            << " n is:" << n
//                            << endl;
//         }
        
//         LG_GTKFERRIS_D << "distance from ci to end() is:" << distance( ci, d->getViewContext()->end() )
//                        << " n is:" << n
//                        << endl;

        /*
         * even though n is a 0 based index, we can't include the end()
         * element
         */
        if( ( !n && !d->getViewContext()->hasSubContexts() ) ||
            distance( ci, d->getViewContext()->end() ) < (n+1) )
        {
            LG_GTKFERRIS_D << "Desired element is past the end of the collection!"
                           << "distance from begin() to end() is:"
                           << distance( ci, d->getViewContext()->end() )
                           << " n is:" << n
                           << endl;
            return FALSE;
        }

        if( n )
            ci += n;
        d->ContextToTreeIter( iter, ci );

        if( ci == d->getViewContext()->end() )
        {
            LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_nth_child(ret) "
                           << " n:" << n
                           << " ERROR!!!! returning end()"
                           << endl;
        }
        else
        {
            LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_nth_child(ret) "
                           << " n:" << n
                           << " returning:" << (*ci)->getURL()
                           << endl;
        }
        
        return TRUE;
    }
    
    if(!VALID_ITER (parent, t))
    {
        cerr << "gtk_ferris_tree_store_iter_nth_child() failing. "
             << " parent:" << (void*)parent
             << " stamp:" << parent->stamp
             << " treestamp:" << t->stamp
             << " userdata:" << parent->user_data
             << endl;

        g_on_error_stack_trace(NULL);
    }
    g_return_val_if_fail (VALID_ITER (parent, t), FALSE);

    iter->stamp = 0;
    fh_context parentc = d->toContext( parent );
    LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_nth_child() "
                   << " parentc:" << parentc->getDirPath()
                   << endl;

    if( distance( parentc->begin(), parentc->end() ) < n )
        return false;
    
    Context::iterator ci = parentc->begin();
    ci += n;
    d->ContextToTreeIter( iter, ci );
    return true;
}

static gboolean
gtk_ferris_tree_store_iter_parent( GtkTreeModel *tree_model,
                                   GtkTreeIter  *iter,
                                   GtkTreeIter  *child )
{
    g_return_val_if_fail       (GTK_IS_FERRIS_TREE_STORE (tree_model), 0);
    GtkFerrisTreeStore*    t = GTK_FERRIS_TREE_STORE (tree_model);
    FerrisTreeModel_Impl*  d = t->d;
    
    LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_parent()" << endl;
    g_return_val_if_fail (VALID_ITER (child, t), FALSE);
    fh_context childc = d->toContext( child );

    InvalidateIter( iter );

    if( childc->isParentBound() )
    {
        d->ContextToTreeIter( iter, childc->getParent());
        return TRUE;
    }

    LG_GTKFERRIS_D << "gtk_ferris_tree_store_iter_parent() failing iter:" << (void*)iter << endl;
    return FALSE;
}




/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

static void
gtk_ferris_tree_store_tree_model_init (GtkTreeModelIface *iface)
{
    iface->get_flags       = gtk_ferris_tree_store_get_flags;
    iface->get_n_columns   = gtk_ferris_tree_store_get_n_columns;
    iface->get_column_type = gtk_ferris_tree_store_get_column_type;
    iface->get_iter        = gtk_ferris_tree_store_get_iter;
    iface->get_path        = gtk_ferris_tree_store_get_path;
    iface->get_value       = gtk_ferris_tree_store_get_value;
    iface->iter_next       = gtk_ferris_tree_store_iter_next;
    iface->iter_children   = gtk_ferris_tree_store_iter_children;
    iface->iter_has_child  = gtk_ferris_tree_store_iter_has_child;
    iface->iter_n_children = gtk_ferris_tree_store_iter_n_children;
    iface->iter_nth_child  = gtk_ferris_tree_store_iter_nth_child;
    iface->iter_parent     = gtk_ferris_tree_store_iter_parent;
}

GtkType
gtk_ferris_tree_store_get_type (void)
{
    static GType tree_store_type = 0;

    if (!tree_store_type)
    {
        static const GTypeInfo tree_store_info =
            {
                sizeof (GtkFerrisTreeStoreClass),
                NULL,		/* base_init */
                NULL,		/* base_finalize */
                (GClassInitFunc) gtk_ferris_tree_store_class_init,
                NULL,		/* class_finalize */
                NULL,		/* class_data */
                sizeof (GtkFerrisTreeStore),
                0,              /* n_preallocs */
                (GInstanceInitFunc) gtk_ferris_tree_store_init
            };

        static const GInterfaceInfo tree_model_info =
            {
                (GInterfaceInitFunc) gtk_ferris_tree_store_tree_model_init,
                NULL, NULL
            };

        static const GInterfaceInfo drag_source_info =
            {
                (GInterfaceInitFunc) gtk_ferris_tree_store_drag_source_init,
                NULL,
                NULL
            };
        

        tree_store_type = g_type_register_static (
            G_TYPE_OBJECT, "GtkFerrisTreeStore",
            &tree_store_info, GTypeFlags(0));

        g_type_add_interface_static( tree_store_type, GTK_TYPE_TREE_MODEL, &tree_model_info);
//        g_type_add_interface_static( tree_store_type, GTK_TYPE_TREE_DRAG_SOURCE, &drag_source_info);

    }

    return tree_store_type;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

namespace FerrisUI
{

//     void
//     updateCell( GtkFerrisTreeStore *tree_model,
//                  gchar* path_string,
//                  gint colnum,
//                  string cellname,
//                  string newval )
//     {
//         g_return_if_fail (GTK_IS_FERRIS_TREE_STORE (tree_model));
//         GtkFerrisTreeStore *t = GTK_FERRIS_TREE_STORE (tree_model);
//         GtkTreeIter iter;
//         GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
//         gtk_tree_model_get_iter( GTK_TREE_MODEL(t), &iter, path );

//         fh_context ctx = toContext( t, path_string );
// //         cerr << "context:" << ctx->getDirPath() << endl;
//         fh_attribute a = ctx->getAttribute( cellname );
// //         cerr << "have attr:" << a->getDirPath() << endl;
        
// //         cerr << "setting to :-->:" << newval << ":<--:" << endl;
//         {
//             fh_iostream ss = a->getIOStream();
//             ss << newval;
//         }
        
 
//         cerr << "done reseting..."<<endl;
    

//         bool emit_signal = true;
//         if (emit_signal)
//         {
// //            cerr << "gtk_ferris_tree_store_update_cell(a)..."<<endl;
//             gtk_tree_model_row_changed (GTK_TREE_MODEL (t), path, &iter);
// //            cerr << "gtk_ferris_tree_store_update_cell(b)..."<<endl;
//         }
//         gtk_tree_path_free( path );
// //        cerr << "gtk_ferris_tree_store_update_cell() done."<<endl;
//     }
    
    

//     // FIXME: check if 'c' is in the tree someplace first.
//     GtkTreeIter getIterForContext( GtkFerrisTreeStore *t, fh_context c )
//     {
//         static GtkTreeIter iter;
//         ContextToTreeIter( GTK_TREE_MODEL(t), &iter, c );
//         return iter;
//     }
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    FerrisTreeModel_Impl::FerrisTreeModel_Impl()
        :
        m_gtkModel(0),
        m_contextsDone(0),
        m_subContextCountGuess(0),
        m_viewIsBootstrappingNCols( 1 ),
        m_areMonitoringRoot( false ),
        m_sortingString(""),
        m_filterString(""),
        m_filteredContext(0),
        m_sortedContext(0),
        m_checkDirsForChildren( false ),
        m_checkForExistsSubDir( false ),
        m_addingContextSignalModulas( 300 ),
        m_root_hasChanged_RecheckStatelessSchemas( true ),
        m_cleared( true )
    {
        createGtkModel();
    }

    FerrisTreeModel_Impl::~FerrisTreeModel_Impl()
    {
    }

    void
    FerrisTreeModel_Impl::createGtkModel()
    {
        m_gtkModel    = gtk_ferris_tree_store_new();
        m_gtkModel->d = this;
    }
    
    

    void
    FerrisTreeModel_Impl::clear( fh_context parentc )
    {
        LG_GTKFERRIS_D << "clear() parentc:" << parentc->getURL() << endl;
        if( m_cleared )
            return;
        m_cleared = true;
        
        LG_GTKFERRIS_D << "clear() mydistance begin to end:"
                       << mydistance( parentc->begin(), parentc->end() )
                       << endl;

//         int removeCount = 0;
//         GtkTreeIter iter;
//         if( gtk_tree_model_get_iter_first( getGtkModel(), &iter ) )
//         {
//             while( true )
//             {
//                 GtkTreePath* path = gtk_tree_model_get_path( getGtkModel(), &iter );
//                 gtk_tree_model_row_deleted( getGtkModel(), path );
//                 gtk_tree_path_free( path );

//                 fh_context c = toContext( &iter );
//                 SigCol[ c ].MedallionUpdatedConnection.disconnect();
//                 ++removeCount;
            
//                 if( !gtk_tree_model_iter_next( getGtkModel(), &iter) )
//                     break;
//             }
//         }
        

        
        if( parentc->begin() == parentc->end() )
            return;

        
        int removeCount = 0;
        GtkTreeIter iter;
        Context::iterator ci = parentc->end();
        --ci;
        ContextToTreeIter( &iter, ci );
        GtkTreePath* path = gtk_tree_model_get_path( getGtkModel(), &iter );
        
        while( true )
        {
            g_return_if_fail( path );

            LG_GTKFERRIS_D << " clear() ci:" << (*ci)->getURL()
                           << " path:" << tostr(path)
                           << endl;
            
            gtk_tree_model_row_deleted( getGtkModel(), path );

//            SigCol[ *ci ].MedallionUpdatedConnection.disconnect();
            getSignalCollection( *ci ).MedallionUpdatedConnection.disconnect();

            if( ci == parentc->begin() )
                break;
            
            --ci;
            gtk_tree_path_prev( path );
            ++removeCount;
        }
        gtk_tree_path_free( path );

        getClearedSig().emit( this );
        m_ContextIterators.clear();
        LG_GTKFERRIS_D << " clear(complete) parent:" << parentc->getURL()
                       << " removeCount:" << removeCount << endl;
    }
    
    void
    FerrisTreeModel_Impl::clear()
    {
        if( isBound( m_root ) )
        {
            clear( m_root );
        }
    }
    
    void
    FerrisTreeModel_Impl::setURL( fh_context c )
    {
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::setURL() c:" << c->getURL() << endl;

        if( m_root )
        {
            LG_GTKFERRIS_D << "FerrisTreeModel_Impl::setURL() m_root:" << m_root->getURL() << endl;
            StopMonitoringSubContext( m_root );
            m_areMonitoringRoot = false;
        }
        
        clear();
        m_url  = c->getURL();
        m_root = c;
        m_root_hasChanged_RecheckStatelessSchemas = true;

        m_columnNames.clear();
        UpdateColumnNames();
    }
    
    void
    FerrisTreeModel_Impl::setURL( const std::string& s )
    {
        setURL( Resolve( s ) );
    }

    void
    FerrisTreeModel_Impl::OnExistsDiskRead( NamingEvent_Exists* ev,
                                            const fh_context& subc,
                                            std::string olddn, std::string newdn )
    {
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::OnExistsDiskRead() " << endl;
        
        fh_context c    = ev->getSource();
//        fh_context subc = c->getSubContext(newdn);

        ++m_contextsDone;

        if( (m_contextsDone % 100) == 0 )
        {
            getDiskReadProgressSig().emit( c, m_contextsDone, m_subContextCountGuess );
        }

//        EnsureMonitoringSubContextCreationEvents( subc );
    }

    void
    FerrisTreeModel_Impl::populateModelFromContext( fh_context c, bool isRoot )
    {
        ImplicitIteratorUpdateLock ciLock;

        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::populateModelFromContext() isRoot:" << isRoot
             << " c:" << c->getURL()
             << endl;
        
        /*
         * Add all the data to the gtk model
         */
        m_contextsDone = 0;
        getStartReadingSig().emit( c );
        GtkTreePath* path = gtk_tree_path_new_first();
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::populateModelFromContext(2) isRoot:" << isRoot << endl;
        if( !isRoot )
        {
            LG_GTKFERRIS_D << "FerrisTreeModel_Impl::populateModelFromContext(3) isRoot:" << isRoot << endl;
            Context::iterator ci = toContextIterator( c );
            GtkTreeIter giter = toTreeIter( ci );
            LG_GTKFERRIS_D << "FerrisTreeModel_Impl::populateModelFromContext(3.d)"
                 << " c:" << c->getURL()
                 << " ci:" << ci->getURL()
                 << endl;
            LG_GTKFERRIS_D << "FerrisTreeModel_Impl::populateModelFromContext(4) isRoot:" << isRoot << endl;
            path = gtk_ferris_tree_store_get_path( getGtkModel(), &giter );
            
            if( !path )
            {
                LG_GTKFERRIS_D << "Asked to expand a context that a path in the tree can not be found for!" << endl;
            }
            gtk_tree_path_down(path);
            LG_GTKFERRIS_D << "populateModelFromContext() path:" << tostr(path) << endl;
        }
        
        LG_GTKFERRIS_D << "populateModelFromContext(got path) url:" << c->getURL()
                       << " path:" << tostr(path)
                       << endl;
        int totalItems = 0;

        {
//             cerr << "ADDING ENTRIES,START" << endl;
//             Time::Benchmark bm("Adding entries");

            Context::iterator e = c->end();
            for( Context::iterator ci = c->begin(); ci != e; ++ci )
            {
                LG_GTKFERRIS_D << "populateModelFromContext(adding) url:" << (*ci)->getURL()
                               << " path:" << tostr(path)
                               << endl;
                GtkTreeIter  iter;
                ContextToTreeIter( &iter, ci, false );
                add( ci, &iter, path );
                gtk_tree_path_next( path );
                ++totalItems;
            }

//            cerr << "ADDING ENTRIES,DONE" << endl;
        }
        gtk_tree_path_free( path );
        getStopReadingSig().emit( c );
        LG_GTKFERRIS_D << "populateModelFromContext(done) url:" << c->getURL()
                       << " totalItems:" << totalItems
                       << endl;

    }

    
    
    void
    FerrisTreeModel_Impl::read( fh_context c, bool isRoot, bool force )
    {
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::read(3param) m_root:" << toVoid(m_root)
                       << " root-url:" << m_root->getURL() << endl;
        
//         if( force )
        clear();
        m_subContextCountGuess = c->guessSize();
        m_contextsDone = 0;

        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::read() url:" << c->getURL()
                       << " filter:" << m_filterString
                       << " sorter:" << m_sortingString
                       << endl;

        /* Read the disk and propergate */
        getStartReadingDiskSig().emit( c );
        sigc::connection DiskReadConnection =
            c->getNamingEvent_Exists_Sig().connect(
                sigc::mem_fun( *this, &_Self::OnExistsDiskRead ));
        c->read( force );
        DiskReadConnection.disconnect();
        getStopReadingDiskSig().emit( c );

        /*
         * We never bootstrap shortcut the name column because it could be any size
         */
        m_bootstrapcount = m_viewIsBootstrappingNCols * c->SubContextCount();
        LG_GTKFERRIS_D << "read() c:" << c->getURL()
                       << " setting m_bootstrapcount:" << m_bootstrapcount
                       << " for m_viewIsBootstrappingNCols:" << m_viewIsBootstrappingNCols
                       << " and c->SubContextCount():" << c->SubContextCount()
                       << endl;
        
        if( isRoot )
        {
            /* Make sure the root is set */
            setRoot( c );
        }
        

        /**
         * PURE DEBUG
         * Assert that we are not filtering / sorting a sorted context
         */
        {
            if( SortedContext* sc = dynamic_cast<SortedContext*>( GetImpl(c) ))
            {
                cerr << "WARNING: possible NESTING ERROR Attempt to wrap a sorted context!" << endl;
            }
        }
        
        /*
         * filter the view
         */
        getFilterStartedSig().emit( c );
        if( !m_filterString.empty() )
        {
            fh_context filter = Factory::MakeFilter( m_filterString );
            m_filteredContext = Factory::MakeFilteredContext( c, filter );

            /*
             * We have to read the base context before we drop our handle
             * to it
             */
            c->read();
            c = m_filteredContext;
        }
        else
        {
            m_filteredContext = 0;
        }
        
        
        
        /*
         * Sort the view
         */
        getSortStartedSig().emit( c );
        if( m_sortingString.length() )
        {
            c->read();
            m_sortedContext  = Factory::MakeSortedContext( c, m_sortingString );
            c = m_sortedContext;
        }
        else
        {
            m_sortedContext = 0;
        }

//        cerr << "Calling populateModelFromContext() c:" << c->getURL() << endl;
        populateModelFromContext( c, isRoot );
//        cerr << "Done calling populateModelFromContext() c:" << c->getURL() << endl;

        if( !m_areMonitoringRoot )
        {
            m_areMonitoringRoot = true;
            StartMonitoringSubContext( c );
        }
    }
    
    
    void
    FerrisTreeModel_Impl::read( bool force )
    {
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::read(before) m_root:" << toVoid(m_root)
                       << " root-url:" << m_root->getURL()
                       << " vc:" << getViewContext()
                       << " vc-url:" << getViewContext()->getURL()
                       << endl;
        
        read( m_root, true, force );

        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::read(after) m_root:" << toVoid(m_root)
                       << " root-url:" << m_root->getURL()
                       << " vc:" << getViewContext()
                       << " vc-url:" << getViewContext()->getURL()
                       << endl;
    }
    
    
    
    std::string
    FerrisTreeModel_Impl::getURL()
    {
        return m_url;
    }

    fh_context
    FerrisTreeModel_Impl::getRootContext()
    {
        return getViewContext();
    }
    
    
    
    GtkTreeModel*
    FerrisTreeModel_Impl::getGtkModel()
    {
        return GTK_TREE_MODEL( m_gtkModel );
    }

    void
    FerrisTreeModel_Impl::EnsureMonitoringSubContextCreationEvents( fh_context c, bool force )
    {
        if( !force )
        {
            if( !m_checkDirsForChildren )
            {
                return;
            }
        }
        
        
        LG_GTKFERRIS_D << "EnsureMonitoringSubContextCreationEvents()"
                       << " monitoring c:" << c->getURL()
                       << endl;
        
        SignalCollectionCreation& sigs = getSignalCollectionCreation( c );
        if( !sigs.ref_count )
        {
            sigs.CreatedConnection = c->getNamingEvent_Created_Sig().
                connect( sigc::mem_fun( *this, &_Self::OnCreated ));
            sigs.ref_count++;
        }
    }
    
    

    void
    FerrisTreeModel_Impl::StartMonitoringSubContext( fh_context c )
    {
        SignalCollection& sigs = getSignalCollection( c );
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::StartMonitoringSubContext()"
                       << " c:" << c->getURL()
                       << " rc:" << sigs.ref_count
                       << endl;

        if( !sigs.ref_count )
        {
            LG_GTKFERRIS_D << "FerrisTreeModel_Impl::StartMonitoringSubContext()"
                           << " c:" << c->getURL()
                           << " rc was 0" 
                           << endl;

            sigs.ChangedConnection = c->getNamingEvent_Changed_Sig()
                .connect( sigc::mem_fun( *this, &_Self::OnChanged ));

            EnsureMonitoringSubContextCreationEvents( c, true );
//             sigs.CreatedConnection = c->getNamingEvent_Created_Sig().
//                 connect( sigc::mem_fun( *this, &_Self::OnCreated ));

            sigs.DeletedConnection = c->getNamingEvent_Deleted_Sig().
                connect( sigc::mem_fun( *this, &_Self::OnDeleted ));

            sigs.MovedConnection = c->getNamingEvent_Moved_Sig()
                .connect( sigc::mem_fun( *this, &_Self::OnMoved ));
            
//             sigs.ChangedConnection = c->getNamingEvent_Exists_Sig().
//                 connect( sigc::mem_fun( *this, &_Self::OnExists ));
        }
        
        sigs.ref_count++;
    }
    
    void
    FerrisTreeModel_Impl::StopMonitoringSubContext(  fh_context c )
    {
        LG_GTKFERRIS_D << "StopMonitoringSubContext(top) c.addr:" << toVoid(c) << endl;

        if( !c )
            return;
        
        LG_GTKFERRIS_D << "StopMonitoringSubContext(1) c:" << c->getURL() << endl;
        
//        SigCol_t::iterator iter = SigCol.find(c);
        SigCol_t::iterator iter = findSignalCollection( c );

        LG_GTKFERRIS_D << "StopMonitoringSubContext(2) c:" << c->getURL() << endl;

        //
        // this is bad.
        //
        if( iter == SigCol.end() )
        {
            LG_GTKFERRIS_ER << "StopMonitoringSubContext(2.b) BAD! not found c:" << GetImpl(c) << endl;
            if( ChainedViewContext* cvc = dynamic_cast<ChainedViewContext*>(GetImpl(c)))
                LG_GTKFERRIS_ER << " ddel:" << deepestDelegate( cvc ) << endl;
            {
                fh_stringstream ss;
                c->dumpRefDebugData( ss );
                LG_GTKFERRIS_ER << " DBG:" << tostr(ss)
                                << " SigCol.sz:" << SigCol.size()
                                << endl;
            }
            for( SigCol_t::iterator iter = SigCol.begin(); iter != SigCol.end(); ++iter )
            {
                fh_stringstream ss;
                fh_context c = iter->first;
//                fh_context c = Resolve(iter->first);
                LG_GTKFERRIS_I << "StopMonitoringSubContext(2.c) iter... "
                               << " c:" << GetImpl(c) << endl;
                if( ChainedViewContext* cvc = dynamic_cast<ChainedViewContext*>(GetImpl(c)))
                    LG_GTKFERRIS_I << " ddel:" << deepestDelegate( cvc ) << endl;

                c->dumpRefDebugData( ss );
                LG_GTKFERRIS_ER << " DBG:" << tostr(ss)
                                << " SigCol.sz:" << SigCol.size()
                                << endl;
            }
        }

        if( iter != SigCol.end() )
        {
            SignalCollection& sigs = iter->second;
            
            LG_GTKFERRIS_D << "StopMonitoringSubContext(3) c:" << c->getURL()
                           << " SigCol[c].ref_count:" << sigs.ref_count
                           << endl;
            sigs.ref_count--;
            if( !sigs.ref_count )
            {
                LG_GTKFERRIS_D << "FerrisTreeModel_Impl::StopMonitoringSubContext()"
                               << " c:" << c->getURL() << endl;
//                sigs.ExistsConnection.disconnect();
//                sigs.CreatedConnection.disconnect();
                sigs.ChangedConnection.disconnect();
                sigs.DeletedConnection.disconnect();
            }
        }

        // Creation signal is in another group.
        {
            SigColCreation_t::iterator iter = findSignalCollectionCreation( c );
            if( iter != SigColCreation.end() )
            {
                SignalCollectionCreation& sigs = iter->second;
                sigs.ref_count--;
                if( !sigs.ref_count )
                {
                    sigs.CreatedConnection.disconnect();
                }
            }
        }
    }

    void
    FerrisTreeModel_Impl::OnExists ( NamingEvent_Exists* ev,
                                     const fh_context& subc,
                                     std::string olddn,
                                     std::string newdn )
    {

        fh_context c    = ev->getSource();
//        fh_context subc = c->getSubContext(newdn);
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::OnExists c:" << c->getURL()
                       << " subc:" << subc->getURL()
                       << endl;
        add( subc );
    }
    
    void
    FerrisTreeModel_Impl::OnCreated( NamingEvent_Created* ev,
                                     const fh_context& subc,
                                     std::string olddn,
                                     std::string newdn )
    {
        ImplicitIteratorUpdateLock l;
        
        fh_context c    = ev->getSource();
//        fh_context subc = c->getSubContext(newdn);
        LG_GTKFERRIS_D << "WATCH FerrisTreeModel_Impl::OnCreated c:" << GetImpl(c)
                       << " c:" << c->getURL()
                       << " subc:" << subc->getURL()
                       << " c-is-subc(rdn)-bound:" << c->priv_isSubContextBound( subc->getDirName() )
                       << endl;
        add( subc );


        
/////        EnsureMonitoringSubContextCreationEvents( subc );
        
//         {
//             fh_context nc = c->getSubContext( subc->getDirName() );

// //             cerr << "Dumping subc:" << endl;
// //             subc->dumpRefDebugData( Factory::fcerr() );
// //             cerr << "Dumping nc:" << endl;
// //             subc->dumpRefDebugData( Factory::fcerr() );
            
//             LG_GTKFERRIS_D << "WATCH nc:" << GetImpl(nc)
//                            << endl;
//             GtkTreeIter iter;
//             ContextToTreeIter( &iter, nc );
//             LG_GTKFERRIS_D << "WATCH2 nc:" << GetImpl(nc)
//                            << endl;
//             add( TreeIterToContext( &iter ) );
//             LG_GTKFERRIS_D << "WATCH3 nc:" << GetImpl(nc)
//                            << endl;
//         }
    }

    void
    FerrisTreeModel_Impl::OnMedallionUpdated( fh_context c )
    {
        cerr << "FerrisTreeModel_Impl::OnMedallionUpdated() c:" << c->getURL() << endl;

        GtkTreeIter  iter = toTreeIter( c );
        GtkTreePath* path = gtk_tree_model_get_path( getGtkModel(), &iter );
        g_return_if_fail( path );
        gtk_tree_model_row_changed( getGtkModel(), path, &iter );
        gtk_tree_path_free( path );
    }
    
    void
    FerrisTreeModel_Impl::OnChanged( NamingEvent_Changed* ev,
                                     std::string olddn,
                                     std::string newdn )
    {
        fh_context c = ev->getSource();
//         LG_GTKFERRIS_D << "+++++ Changed c:" << c->getURL()
//                        << " olddn:" << olddn
//                        << endl;
        
//         if( m_root )
//         {
//             LG_GTKFERRIS_D << "+++ m_root:" << m_root->getURL()
//                            << endl;
//         }
        

        if( c == m_root
            || c == m_filteredContext
            || c == m_sortedContext
            || ( m_root && c->getURL() == m_root->getURL() )
            )
        {
            // Handle the root changing in a different manner 
        }
        else if( m_root && c->getURL().length() < m_root->getURL().length() )
        {
            // Notification for a context that is a parent of the root 
        }
        else
        {
            cerr << "+++++ Changed c:" << c->getURL()
                 << " olddn:" << olddn
                 << endl;
            BackTrace();
    
            GtkTreeIter  iter = toTreeIter( c );
            GtkTreePath* path = gtk_tree_model_get_path( getGtkModel(), &iter );
            g_return_if_fail( path );

#ifndef GTK_SUBTREE_IS_BUGGY
            gtk_tree_model_row_has_child_toggled( getGtkModel(), path, &iter );
#endif
            gtk_tree_model_row_changed( getGtkModel(), path, &iter );
        
            gtk_tree_path_free( path );
        }
    }

    void
    FerrisTreeModel_Impl::OnMoved( NamingEvent_Moved* ev, std::string olddn, std::string newdn )
    {
        fh_context rc = ev->getSource();
        fh_context c  = rc->getSubContext(olddn);
        cout << "FerrisTreeModel_Impl::OnMoved() c:" << c->getURL() << endl;
        
    }
    
    void
    FerrisTreeModel_Impl::OnDeleted( NamingEvent_Deleted* ev,
                                     std::string olddn,
                                     std::string newdn )
    {
        fh_context rc = ev->getSource();
        if( rc->isSubContextBound( olddn ) )
        {
            fh_context c  = rc->getSubContext(olddn);
            cout << "FerrisTreeModel_Impl::OnDeleted() c:" << c->getURL() << endl;

            GtkTreeIter  iter = toTreeIter( c );
            GtkTreePath* path = gtk_tree_model_get_path( getGtkModel(), &iter );
            g_return_if_fail( path );

            LG_GTKFERRIS_D << "WATCH REMOVE() "
                           << " url:" << c->getURL()
                           << " path:" << tostr(path)
                           << endl;
            getRemovingContextSig().emit( c );
            gtk_tree_model_row_deleted( getGtkModel(), path );
            gtk_tree_path_free( path );
        }
    }
    
    Context::iterator
    FerrisTreeModel_Impl::TreeIterToContext( GtkTreeIter *iter )
    {
        if( iter->user_data2 )
        {
            Context::iterator* cip = (Context::iterator*)iter->user_data2;
            return *cip;
        }

        cerr << "WARNING: FerrisTreeModel_Impl::TreeIterToContext(using slow)" << endl;
        
        Context* cp = (Context*)( iter->user_data );
//         LG_GTKFERRIS_D << "TreeIterToContext() cp:" << ((void*)cp) << endl;
//         LG_GTKFERRIS_D << "TreeIterToContext() cp:" << cp->getURL() << endl;
        Context::iterator ci = m_ContextIterators[cp];
        
//         LG_GTKFERRIS_D << "TreeIterToContext() ci:" << (*ci)->getURL() << endl;
        return ci;
    }

    void
    FerrisTreeModel_Impl::ContextToTreeIter( GtkTreeIter *iter, Context::iterator ci, bool checkExistsInContextIterators )
    {
        Context* cp = GetImpl( *ci );
        InvalidateIter( iter );

        if( checkExistsInContextIterators )
        {
            ContextIterators_t::iterator tmp = m_ContextIterators.find( cp );
            if( m_ContextIterators.end() != tmp )
            {
                iter->stamp           = m_gtkModel->stamp;
                iter->user_data       = (void*)cp;
                iter->user_data2      = (void*)&tmp->second;
                return;
            }
        }
        
        ContextIterators_t::iterator ins_iter
            = m_ContextIterators.insert( make_pair( cp, ci )).first;

//         LG_GTKFERRIS_D << "ContextToTreeIter(ci) cp void*:" << ((void*)cp) << endl;
//         LG_GTKFERRIS_D << "ContextToTreeIter(ci) cp:" << cp->getURL() << endl;
//         LG_GTKFERRIS_D << "ContextToTreeIter(ci) ci:" << (*ci)->getURL() << endl;

        iter->stamp           = m_gtkModel->stamp;
        iter->user_data       = (void*)cp;
        iter->user_data2      = (void*)&ins_iter->second;
    }
    
    
    void
    FerrisTreeModel_Impl::ContextToTreeIter( GtkTreeIter *iter, Context::iterator ci )
    {
        ContextToTreeIter( iter, ci, true );
    }
    
    void
    FerrisTreeModel_Impl::ContextToTreeIter( GtkTreeIter *iter, fh_context c )
    {
//         cerr << "FerrisTreeModel_Impl::ContextToTreeIter() c:" << c->getURL()
//              << " has p:" << c->isParentBound()
//              << endl;
        fh_context p = c->getParent();
        string rdn   = c->getDirName();

        if( m_sortedContext && p->getURL() == m_sortedContext->getURL() )
        {
            p = m_sortedContext;
            c = p->getSubContext( c->getDirName() );
        }
        
        
        
//         cerr << "ContextToTreeIter(ctx) c:" << c->getURL()
//              << " p:" << p->getURL()
//              << " caddr:" << toVoid(GetImpl(c))
//              << " paddr:" << toVoid(GetImpl(p))
//              << endl;
//        BackTrace();

        Context::iterator ci = Ferris::toContextIterator( c );
        if( ci == p->end() )
        {
            cerr << "A VERY BAD THING, CAN NOT FIND THE CONTEXT ITERATOR FOR A\n"
                 << "GIVEN CONTEXT. BAD THINGS WILL SOON HAPPEN"
                 << endl;
            g_on_error_stack_trace(NULL);
        }
        
        ContextToTreeIter( iter, ci );
    }

    GtkTreeIter
    FerrisTreeModel_Impl::toTreeIter( Context::iterator ci )
    {
        static GtkTreeIter iter;
        ContextToTreeIter( &iter, ci );
        return iter;
    }
    
    GtkTreeIter
    FerrisTreeModel_Impl::toTreeIter( fh_context c )
    {
        static GtkTreeIter iter;
        ContextToTreeIter( &iter, c );
        return iter;
    }

    
    void
    FerrisTreeModel_Impl::UpdateColumnNames()
    {
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::UpdateColumnNames(called)" << endl;
        if( !isBound( m_root ) )
        {
            LG_GTKFERRIS_D << "FerrisTreeModel_Impl::UpdateColumnNames(returning) m_root not set." << endl;
            return;
        }

        if( !m_columnNames.empty() )
        {
            LG_GTKFERRIS_D << "FerrisTreeModel_Impl::UpdateColumnNames() m_columnNames already populated." << endl;
            return;
        }

        if( !m_staticColumnNames.empty() )
        {
            LG_GTKFERRIS_D << "FerrisTreeModel_Impl::UpdateColumnNames() using a static column name list.." << endl;
            m_columnNames = m_staticColumnNames;
            return;
        }
        
        
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::UpdateColumnNames(starting)" << endl;
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::UpdateColumnNames() m_root:" << m_root->getURL() << endl;
        fh_context rc = m_root;
        try
        {
            set<string> unionset;
            
            LG_GTKFERRIS_D << "FerrisTreeModel_Impl::UpdateColumnNames(getting ea-names)" << endl;
            addEAToSet( unionset,  getStrAttr( rc, "ea-names", "" ));
            LG_GTKFERRIS_D << "FerrisTreeModel_Impl::UpdateColumnNames(getting ea-names-union-view)" << endl;
            addEAToSet( unionset,  getStrAttr( rc, "recommended-ea-union-view", "" ));
            LG_GTKFERRIS_D << "FerrisTreeModel_Impl::UpdateColumnNames(getting fixed additions)" << endl;
            addEAToSet( unionset,  getEDBString( FDB_GENERAL,
                                                 CFG_ATTRIBUTES_ALWAYS_IN_UI_MODEL_K,
                                                 CFG_ATTRIBUTES_ALWAYS_IN_UI_MODEL_DEFAULT ));
            unionset.insert( treeicon_pixbuf_cn );
            unionset.insert( emblems_pixbuf_cn );
            unionset.insert( ferris_iconname_pixbuf_cn );
            m_columnNames.clear();
            m_columnNames.reserve( unionset.size() );
            copy( unionset.begin(), unionset.end(), back_inserter( m_columnNames ));
        }
        catch( exception& e )
        {
            LG_GTKFERRIS_D << "error adding column data for ctx:" << rc->getURL() << endl;
        }

        LG_GTKFERRIS_D << "UpdateColumnNames() m_columnNames.size:" << m_columnNames.size()
                       << endl;
    }

    /**
     * Note that what is already in m_columnNames must remain in its current position.
     */
    void
    FerrisTreeModel_Impl::AddAllNewEANames( fh_context c )
    {
        if( !m_staticColumnNames.empty() )
            return;
        
//        BackTrace();
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::AddAllNewEANames c:" << c->getURL() << endl;
        typedef columnNames_t cn_t;
        cn_t        newcn;
        string s;

        set<string> unionset;
        set<string> rawset;
        unionset.insert( m_columnNames.begin(), m_columnNames.end() );
        rawset.insert( m_columnNames.begin(), m_columnNames.end() );
        
        addEAToSet( unionset,  getStrAttr( c, "ea-names", "" ));
        addEAToSet( unionset,  getStrAttr( c, "recommended-ea-union-view", "" ));

        set_difference( unionset.begin(), unionset.end(),
                        rawset.begin(),   rawset.end(),
                        back_inserter( m_columnNames ) );
    }


    GType
    FerrisTreeModel_Impl::getColumnType( const string& cn )
    {
        typedef set<string> strset_t;
        static strset_t boolset;
        static Util::SingleShot virgin;
        if( virgin() )
        {
            boolset.insert("ferris-handles-urls");
            boolset.insert("ferris-opens-many");
            boolset.insert("ferris-ignore-selection");
        }

        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::getColumnType(begin) cn:" << cn << endl;
        
        //
        // After m_root is set to something else it will set
        // m_root_hasChanged_RecheckStatelessSchemas for us.
        // We then rebuild the cache of which columns use different
        // GTypes than string for use later
        //
        if( isBound(m_root) && m_root_hasChanged_RecheckStatelessSchemas )
        {
            m_root_hasChanged_RecheckStatelessSchemas = false;

            m_columnTypeMap.clear();

            int StatelessEACount = 0;
            for( columnNames_t::iterator cni = m_columnNames.begin();
                 cni != m_columnNames.end(); ++cni )
            {
                try
                {
                    string eaname = *cni;
//                    cerr << "FerrisTreeModel_Impl::getColumnType(iter) eaname:" << eaname << endl;
                    if( starts_with( eaname, "schema:" ))
                        continue;
                    
                    if( starts_with( eaname, "emblem:has-fuzzy-" ))
                    {
                        m_columnTypeMap[ eaname ] = G_TYPE_STRING;
                        continue;
                    }
                    if( starts_with( eaname, "emblem:has-" ))
                    {
                        m_columnTypeMap[ eaname ] = G_TYPE_BOOLEAN;
                        continue;
                    }
                    if( starts_with( eaname, "emblem:" ))
                    {
                        m_columnTypeMap[ eaname ] = G_TYPE_STRING;
                        continue;
                    }
                    
                    if( m_root->isStatelessAttributeBound( eaname ) )
                    {
                        ++StatelessEACount;

                        XSDBasic_t sct = getSchemaType( m_root, eaname, XSD_UNKNOWN );
                        sct = maskOffXSDMeta( sct );

                        if( sct == XSD_BASIC_BOOL )
                        {
                            m_columnTypeMap[ eaname ] = G_TYPE_BOOLEAN;
                        }
                    }
                }
                catch(...)
                {
                }
            }

            LG_GTKFERRIS_D << "FerrisTreeModel_Impl::getColumnType(made cache)"
                           << " StatelessEACount:" << StatelessEACount << endl;
        }

        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::getColumnType(end) cn:" << cn << endl;
        
        if( boolset.end() != boolset.find( cn ) )
            return G_TYPE_BOOLEAN;

        if( m_columnTypeMap.end() != m_columnTypeMap.find( cn ) )
        {
            return G_TYPE_BOOLEAN;
        }
        
        if( cn == treeicon_pixbuf_cn )
            return G_TYPE_OBJECT;
        if( cn == emblems_pixbuf_cn )
            return G_TYPE_OBJECT;
        if( cn == ferris_iconname_pixbuf_cn )
            return G_TYPE_OBJECT;
        
        return G_TYPE_STRING;
    }
    
    GType
    FerrisTreeModel_Impl::getColumnType( int   index )
    {
        return getColumnType( getColumnName( index ) );
    }
    
    const string&
    FerrisTreeModel_Impl::getColumnName( int index )
    {
        return m_columnNames[index];
    }
    
    gint
    FerrisTreeModel_Impl::getColumnNumber( const std::string& s )
    {
        UpdateColumnNames();
        columnNames_t an = m_columnNames;
        int i=0;

//         for( columnNames_t::const_iterator iter = an.begin(); iter != an.end(); ++iter, ++i )
//         {
//             LG_GTKFERRIS_D << "getColumnNumber() s:" << s << " iter:" << *iter << endl;
//         }
        
        for( columnNames_t::const_iterator iter = an.begin(); iter != an.end(); ++iter, ++i )
        {
            if( *iter == s )
            {
                return i;
            }
        }

        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::getColumnNumber() HAS FAILED TO FIND COLUMN:" << s << endl;
        for( columnNames_t::const_iterator iter = an.begin(); iter != an.end(); ++iter, ++i )
        {
            LG_GTKFERRIS_D << "  iter:  " << *iter << endl;
        }
        
        
        return -1;
    }


    /**
     * Get the context that is at iter
     */
    fh_context
    FerrisTreeModel_Impl::toContext( GtkTreeIter  *iter )
    {
        return *TreeIterToContext( iter );
    }

    fh_context
    FerrisTreeModel_Impl::toContext( GtkTreePath* path )
    {
        GtkTreeIter iter;
        gtk_tree_model_get_iter( getGtkModel(), &iter, path );
        fh_context ret = toContext( &iter );
        return ret;
    }
    

    /**
     * get the context that is at a given path
     */
    fh_context
    FerrisTreeModel_Impl::toContext( gchar* path_string )
    {
        GtkTreeIter iter;
        GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
        gtk_tree_model_get_iter( getGtkModel(), &iter, path );
        fh_context ret = toContext( &iter );
        gtk_tree_path_free( path );
        return ret;
    }
    
    void
    FerrisTreeModel_Impl::setRoot( fh_context c )
    {
        if( isBound( m_root ) )
            return;

        LG_GTKFERRIS_D << "SETROOT() ++++++------++++++ c:" << c->getURL() << endl;
        
        m_root = c;
        m_root_hasChanged_RecheckStatelessSchemas = true;

//         GtkTreeIter  iter;
//         iter.stamp      = m_gtkModel->stamp;
//         iter.user_data  = 0;
//         iter.user_data2 = 0;
//         iter.user_data3 = 0;
        
//         GtkTreePath* path = gtk_tree_path_new_from_string ("0");
//         if( !path )
//         {
//             LG_GTKFERRIS_D << "failing c:" << c->getURL() << endl;
//             g_return_if_fail( path );
//         }
//         gtk_tree_model_row_inserted( getGtkModel(), path, &iter );
//         gtk_tree_path_free( path );
    }

    void
    FerrisTreeModel_Impl::add( Context::iterator ci, GtkTreeIter* iter, GtkTreePath* path )
    {
//        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::add(top) c:" << (*ci)->getURL() << endl;

        /* Make sure the root is set */
        setRoot( ci->getParent() );

        if( !path )
        {
            cerr << "failing c:" << (*ci)->getURL() << endl;
            g_return_if_fail( path );
        }

//        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::add(pre signal) c:" << (*ci)->getURL() << endl;
        ++m_contextsDone;
        if( (m_contextsDone % m_addingContextSignalModulas) == 0 )
        {
            getAddingContextProgressSig().emit( *ci, m_contextsDone, m_subContextCountGuess );
        }
        

        getAddingContextSig().emit( *ci, m_contextsDone, m_subContextCountGuess );

//         LG_GTKFERRIS_D << "FerrisTreeModel_Impl::add(post signal) c:" << (*ci)->getURL() << endl;
        cerr << "WATCH INSERT add() context:" << (*ci)->getURL()
                       << " at path:" << tostr(path)
                       << endl;

        gtk_tree_model_row_inserted( getGtkModel(), path, iter );
        m_cleared = false;


        getSignalCollection( *ci ).MedallionUpdatedConnection
            = (*ci)->getNamingEvent_MedallionUpdated_Sig().connect( sigc::mem_fun( *this, &_Self::OnMedallionUpdated ) );

        EnsureMonitoringSubContextCreationEvents( *ci );
        
        
        //
        // at rh9/gtk2.2 commented this chunk out
        //
// #ifndef GTK_SUBTREE_IS_BUGGY
//         LG_GTKFERRIS_D << "FerrisTreeModel_Impl::add() context:" << (*ci)->getURL()
//                        << " at path:" << tostr(path)
//                        << " doing subtree work"
//                        << endl;
//         if( m_root != *ci && toint( getStrAttr( *ci, "has-subcontexts-guess", "1" )))
//             gtk_tree_model_row_has_child_toggled( getGtkModel(), path, iter );
// #endif

//         LG_GTKFERRIS_D << "FerrisTreeModel_Impl::add(exit) context:" << (*ci)->getURL()
//                        << endl;
    }
    
    void
    FerrisTreeModel_Impl::add( Context::iterator ci )
    {
        LG_GTKFERRIS_D << "add(top) c:" << (*ci)->getURL() << endl;

        /* Make sure the root is set */
        setRoot( ci->getParent() );

        GtkTreeIter  iter = toTreeIter( ci );
        GtkTreePath* path = gtk_tree_model_get_path( getGtkModel(), &iter );

//        cerr << "add() ci:" << (*ci)->getURL() << " path:" << tostr(path) << endl;
        
        add( ci, &iter, path );
        gtk_tree_path_free( path );
    }
    
    void
    FerrisTreeModel_Impl::add( fh_context c )
    {
        GtkTreeIter iter;
        ContextToTreeIter( &iter, c );
        add( TreeIterToContext( &iter ) );
    }

    fh_context
    FerrisTreeModel_Impl::getViewContext()
    {
        if( isBound( m_sortedContext ))
        {
            return m_sortedContext;
        }
        if( isBound( m_filteredContext ))
        {
            return m_filteredContext;
        }
        return m_root;
    }


    std::string
    FerrisTreeModel_Impl::getSortingString()
    {
        return m_sortingString;
    }

    void
    FerrisTreeModel_Impl::setSortingString( const std::string& s, bool revalidate )
    {
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::setSortingString s:" << s << endl;

        if( s == "" || s == "name" || s == "(name)" )
        {
            if( !isBound( m_sortedContext ) )
            {
                LG_GTKFERRIS_D << "FerrisTreeModel_Impl::setSortingString m_sortedContext not bound" << endl;
                return;
            }
        }
        
        m_sortingString = s;
        if( m_sortingString == "" || m_sortingString == "name" )
            m_sortingString = "(name)";

        if( revalidate )
        {
            LG_GTKFERRIS_D << "FerrisTreeModel_Impl::setSortingString() revalidate" << endl;
            updateViewForSorting();
        }
    }
    
    void
    FerrisTreeModel_Impl::updateViewForSorting()
    {
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::updateViewForSorting(1)" << endl;
        
        if( !isBound( m_root ))
            return;
        
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::updateViewForSorting(2)" << endl;

        fh_context bc = m_root;
        if( isBound(m_filteredContext) )
            bc = m_filteredContext;

        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::updateViewForSorting() "
                       << " m_root:" << toVoid( m_root )
                       << " bc:" << toVoid( bc )
                       << " m_filteredContext:" << toVoid( m_filteredContext )
                       << " m_sorted:" << toVoid( m_sortedContext )
                       << endl;

        
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::updateViewForSorting(1) m_sortingString:"
                       << m_sortingString << endl;
        bc->read();
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::updateViewForSorting(1.b) m_sortingString:"
                       << m_sortingString << endl;
        
        fh_context c = Factory::MakeSortedContext( bc, m_sortingString );
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::updateViewForSorting(1.c)" << endl;
        StopMonitoringSubContext( getViewContext() );
        m_areMonitoringRoot = false;
        m_sortedContext = c;
        StartMonitoringSubContext( c );
        

        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::updateViewForSorting(2)" << endl;
        m_sortedContext->read();
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::updateViewForSorting(3)" << endl;
        
        clear();
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::updateViewForSorting(4)" << endl;
        
        populateModelFromContext( m_sortedContext, true );
    }
    

    std::string
    FerrisTreeModel_Impl::getFilterString()
    {
        return m_filterString;
    }

    /**
     * Set the new filter to use for this view. If this fails then the old
     * filter is applied again.
     */
    void
    FerrisTreeModel_Impl::setFilterString( const std::string& s, bool revalidate )
    {
//         if( s == "" )
//             return;

        string oldfilter = m_filterString;
        try
        {
//             cerr << "FerrisTreeModel_Impl::setFilterString(top)"
//                  << " m_root:" << toVoid( m_root )
//                  << " m_root.rc:" << m_root->getReferenceCount()
//                  << " m_filteredContext:" << toVoid( m_filteredContext )
//                  << " m_sorted:" << toVoid( m_sortedContext )
//                  << endl;
//            m_root->read();
//             cerr << "FerrisTreeModel_Impl::setFilterString(top.2)"
//                  << " m_root:" << toVoid( m_root )
//                  << " m_root.rc:" << m_root->getReferenceCount()
//                  << " m_filteredContext:" << toVoid( m_filteredContext )
//                  << " m_sorted:" << toVoid( m_sortedContext )
//                  << endl;
            
            m_filterString = s;
            if( revalidate && m_root )
                updateViewForFiltering();
        }
        catch( exception& e )
        {
            m_filterString = oldfilter;

            if( revalidate )
                updateViewForFiltering();

            throw;
        }
    }
    

    void
    FerrisTreeModel_Impl::updateViewForFiltering()
    {
        if( !isBound( m_root ))
            return;

        LG_GTKFERRIS_D << "updateViewForFiltering() root:" << m_root->getURL() << endl;
        
        if( m_filterString.empty() )
        {
            m_filteredContext = 0;

            if( isBound(m_sortedContext) )
            {
                updateViewForSorting();
            }
            else
            {
                StopMonitoringSubContext( m_filteredContext );
                m_areMonitoringRoot = false;
                StartMonitoringSubContext( m_root );
                clear();
                populateModelFromContext( m_root, true );
            }
            return;
        }
        
        fh_context bc = m_root;

        
        fh_context filter = Factory::MakeFilter( m_filterString );
        StopMonitoringSubContext( m_filteredContext );
        m_areMonitoringRoot = false;
        m_filteredContext = Factory::MakeFilteredContext( m_root, filter );
        StartMonitoringSubContext( m_filteredContext );

        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::updateViewForFiltering(top)" << endl;
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::updateViewForFiltering() m_filterString:" << m_filterString << endl;
        
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::updateViewForFiltering() m_root:"
                       << (void*)GetImpl(m_root) << endl;
        LG_GTKFERRIS_D << "FerrisTreeModel_Impl::updateViewForFiltering() m_filteredContext:"
                       << (void*)GetImpl(m_filteredContext) << endl;

        for( Context::iterator ci = m_root->begin(); ci != m_root->end(); ++ci )
        {
            LG_GTKFERRIS_D << "updateViewForFiltering(root ci) ci:" << (*ci)->getURL() << endl;            
        }
        
        for( Context::iterator ci = m_filteredContext->begin(); ci != m_filteredContext->end(); ++ci )
        {
            LG_GTKFERRIS_D << "updateViewForFiltering(filter ci) ci:" << (*ci)->getURL() << endl;            
        }
        
        if( isBound(m_sortedContext) )
        {
            LG_GTKFERRIS_D << "updateViewForFiltering() Sorted is bound!" << endl;
            updateViewForSorting();
        }
        else
        {
            LG_GTKFERRIS_D << "FerrisTreeModel_Impl::updateViewForFiltering() clear and set to m_filtered" << endl;
            clear();
            populateModelFromContext( m_filteredContext, true );
        }

    }
    

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    namespace FactoryUI
    {
        fh_ftreemodel CreateTreeModel()
        {
            return new FerrisTreeModel_Impl();
        }
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
};
