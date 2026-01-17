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

    $Id: gtkferristreestore.hh,v 1.5 2010/09/24 21:31:07 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __GTK_FERRIS_TREE_STORE_H__
#define __GTK_FERRIS_TREE_STORE_H__

#include <Ferris/HiddenSymbolSupport.hh>

#include <gtk/gtktreemodel.h>
#include <gtk/gtktreesortable.h>
#include <stdarg.h>

#include <string>
#include <vector>
#include <list>
#include <map>


#include <Ferris/TypeDecl.hh>
#include <Ferris/Ferris.hh>

namespace FerrisUI
{
    using namespace Ferris;
    class FerrisTreeModel;

    FERRIS_SMARTPTR( FerrisTreeModel, fh_ftreemodel );
    
    namespace FactoryUI
    {
        /**
         * Create a FerrisTreeModel. This method will return a subclass
         * of the abstract class FerrisTreeModel (keeps data in .cpp file)
         */
        FERRISEXP_API fh_ftreemodel CreateTreeModel();
    };

    /**
     * A little hack for bootstrapping the tree slightly faster
     *
     * A view should count up the number of columns that it is viewing that are
     * fixed size and set setViewIsBootstrappingNCols() to that count.
     */
    FERRISEXP_API bool isColumnFixedSize( const std::string& cn );

    /**
     * Abstraction of a custom GTK+2 Tree Model.
     *
     * Events are emitted during interesting times like reading a dir etc.
     */
    class FERRISEXP_API FerrisTreeModel
        :
        public ::Ferris::Handlable
    {
        friend fh_ftreemodel FactoryUI::CreateTreeModel();
        
    public:

        virtual ~FerrisTreeModel()
            {}
        
        virtual void setURL( fh_context c ) = 0;
        virtual void setURL( const std::string& s ) = 0;
        virtual std::string getURL() = 0;
        virtual fh_context getRootContext() = 0;
        virtual void read(   bool force = false ) = 0;

        /**
         * Clear the gtk model of all items emitting events so that all views
         * think there are no items in the model
         */
        virtual void clear() = 0;
        
        
        virtual GtkTreeModel* getGtkModel() = 0;

        /**
         * Gets the deepest context wrapper for the view.
         * They are always wrapped like this,
         * native
         * filtered
         * sorted
         */
        virtual fh_context getViewContext() = 0;

        /**
         * Get the description of what sorting is currently being
         * performed. 
         */
        virtual std::string getSortingString() = 0;
        virtual void        setSortingString( const std::string& s, bool revalidate = true ) = 0;

        /**
         * Update what filtering is to occur for the view
         */
        virtual std::string getFilterString() = 0;
        virtual void        setFilterString( const std::string& s, bool revalidate = true ) = 0;


        /**
         * Only allow the user to select from these fixed columns.
         */
        virtual void setStaticColumnNames( const std::vector< std::string >& v ) = 0;

        /**
         * Append a new column if it is not already known
         */
        virtual void appendColumnName( const std::string& s ) = 0;

        /**
         * These methods allow the view to cheat by having the model return
         * simple inline data instead of querying the VFS for the real data.
         *
         * These are used because even though a fixed cell size is set for some
         * data the treeview always reads the value of *every* cell before display
         * thus for a large dir lots of cells are read that are not displayed.
         *
         * The model doesn't know how many columns are actaully being displayed,
         * so you must set that as ncols
         */
        virtual void setViewIsBootstrappingNCols( int ncols ) = 0;

        
        virtual void ContextToTreeIter( GtkTreeIter *iter, Context::iterator ci ) = 0;
        virtual void ContextToTreeIter( GtkTreeIter *iter, fh_context c ) = 0;
        virtual GtkTreeIter toTreeIter( fh_context c ) = 0;


        /**
         * Get the context for a given location in the tree
         */
        virtual fh_context toContext( GtkTreeIter  *iter ) = 0;
        virtual fh_context toContext( gchar* path_string ) = 0;
        virtual fh_context toContext( GtkTreePath* path )  = 0;
        
        

        /**
         * Lookup the number of a column by its name.
         *
         * @param tree_model This
         * @param s The name of the column to find the index of.
         *
         * @return the index from 0 up where the column with name 's' can be found at,
         *         or -1 if no column with the name 's' is being shown.
         */
        virtual gint getColumnNumber( const std::string& s ) = 0;
        
        
        /**
         * Set this to check if a directory has children and only display a triangle
         * opener if there are children
         */
        virtual void setCheckDirsForChildren( bool v ) = 0;

        /**
         * Check if there exists one or more subdirs from each context and only dislay
         * a triangle for contexts who have subdirs.
         */
        virtual void setCheckForExistsSubDir( bool v ) = 0;

        /**
         * Set the number of contexts that are to be read between emission of
         * getAddingContextProgressSig() signal. Normal GtkTreeView use should leave
         * the default (somewhere around 100 at the moment) but other views
         * that are just using the model for its data house can set to emit
         * at different intervals
         */
        virtual void setAddingContextSignalModulas( int m ) = 0;
        
        /**
         * These are reference counted, you must match a Start() with a Stop()
         */
        virtual void StartMonitoringSubContext( fh_context c ) = 0;
        virtual void StopMonitoringSubContext(  fh_context c ) = 0;

        typedef sigc::signal< void ( fh_context) > StartReadingDiskSig_t;
                                     typedef sigc::signal< void ( fh_context) > StopReadingDiskSig_t;
        typedef sigc::signal< void ( fh_context, long, long) > DiskReadProgressSig_t;
        typedef sigc::signal< void ( fh_context) > StartReadingSig_t;
        typedef sigc::signal< void ( fh_context) > StopReadingSig_t;
        typedef sigc::signal< void ( fh_context, long, long) > AddingContextSig_t;
        typedef sigc::signal< void ( fh_context) > RemovingContextSig_t;
        typedef sigc::signal< void ( fh_context) > FilterStartedSig_t;
        typedef sigc::signal< void ( fh_context) > SortStartedSig_t;

        typedef sigc::signal< void ( FerrisTreeModel*) > ClearedSig_t;

        StartReadingDiskSig_t& getStartReadingDiskSig() { return StartReadingDiskSig; }
        StopReadingDiskSig_t&  getStopReadingDiskSig()  { return StopReadingDiskSig; }
        DiskReadProgressSig_t& getDiskReadProgressSig() { return DiskReadProgressSig; }
        StartReadingSig_t&     getStartReadingSig()     { return StartReadingSig; }
        StopReadingSig_t&      getStopReadingSig()      { return StopReadingSig; }
        AddingContextSig_t&    getAddingContextProgressSig() { return AddingContextProgressSig; }
        AddingContextSig_t&    getAddingContextSig()    { return AddingContextSig; }
        RemovingContextSig_t&  getRemovingContextSig()  { return RemovingContextSig; }
        FilterStartedSig_t&    getFilterStartedSig()    { return FilterStartedSig; }
        SortStartedSig_t&      getSortStartedSig()      { return SortStartedSig; }
        ClearedSig_t&          getClearedSig()          { return ClearedSig; }
        
    protected:
        FerrisTreeModel()
            {}
        
    private:
        FerrisTreeModel( const FerrisTreeModel& );
//        FerrisTreeModel operator=( const FerrisTreeModel& );

        StartReadingDiskSig_t          StartReadingDiskSig;
        StopReadingDiskSig_t           StopReadingDiskSig;
        DiskReadProgressSig_t          DiskReadProgressSig;
        StartReadingSig_t              StartReadingSig;
        StopReadingSig_t               StopReadingSig;
        AddingContextSig_t             AddingContextProgressSig;
        AddingContextSig_t             AddingContextSig;
        RemovingContextSig_t           RemovingContextSig;
        FilterStartedSig_t             FilterStartedSig;
        SortStartedSig_t               SortStartedSig;
        ClearedSig_t                   ClearedSig;

    public:
        /**
         * Get the type of a column based on its name/index
         */
        virtual GType getColumnType( const std::string& cn ) = 0;
        virtual GType getColumnType( int   index ) = 0;

    };

};
#endif
