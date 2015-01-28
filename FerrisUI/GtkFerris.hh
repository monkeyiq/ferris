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

    $Id: GtkFerris.hh,v 1.12 2011/06/18 21:30:22 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_GTK_FERRIS_H_
#define _ALREADY_INCLUDED_GTK_FERRIS_H_

#include <gmodule.h>

#include <Ferris/HiddenSymbolSupport.hh>

#include <gtk/gtk.h>
#include <Ferris/Ferris.hh>
#include <FerrisUI/gtkferristreestore.hh>
#include <FerrisUI/FerrisOptionMenu.hh>
#include <FerrisUI/UAsyncIO.hh>
#include <Ferris/Ferrisls_AggregateData.hh>

#include <list>
#include <vector>
#include <string>

using namespace Ferris;

namespace FerrisUI
{
    FERRISEXP_API void forea_destroy( GtkWidget *widget, gpointer data );
    FERRISEXP_API std::string tostr( GtkMenuItem* menu_item );
    FERRISEXP_API std::string tostr( GtkTextView* tv );
    FERRISEXP_API std::string tostr( GtkEntry* e );
    FERRISEXP_API void showPopup( GtkMenu *menu );
    /**
     * Set a GTK2 toggle button to the isTrue() value of 's'
     */
    FERRISEXP_API void togButtonSet( GtkWidget* w, const std::string& s );
    /**
     * Set a GTK2 toggle button to v
     */
    FERRISEXP_API void togButtonSet( GtkWidget* w, bool v );

    
    
    /**
     * Resolve out any icons:// prefixes and return a native kernel path
     */
    FERRISEXP_API std::string resolveToIconPath( std::string iconpath );

    /**
     * Make an image button using a given icon URL
     * @see setImageButton()
     */
    FERRISEXP_API GtkWidget* makeImageButton( const std::string& s );

    /**
     * Change the image displayed in this image button
     * @see makeImageButton()
     */
    FERRISEXP_API GtkWidget* setImageButton( GtkWidget* w, const std::string& s );

    /************************************************/
    /************************************************/

    /**
     * Functor that is called when the GTK2 pixbuf is unref()ed for
     * the last time. args are
     * @param width
     * @param height
     * @param dataptr
     */
    typedef Loki::Functor< void, LOKI_TYPELIST_3( int, int, gpointer ) > RGBAtoPixbufFree_f;
    FERRISEXP_API void RGBAtoPixbuf_null( int w, int h, gpointer data );
    FERRISEXP_API void RGBAtoPixbuf_free( int w, int h, gpointer data );
    FERRISEXP_API void RGBAtoPixbuf_delarray( int w, int h, gpointer data );

    /**
     * Create a pixbuf from the 32bit RGBA data at 'data' which has the
     * width and height given. The passed functor will be called when
     * the pixbuf has its last reference removed so that the 'data'
     * can be freed in whatever way it should be.
     */
    FERRISEXP_API GdkPixbuf* RGBAtoPixbuf( int w, int h, gpointer data,
                             RGBAtoPixbufFree_f f = RGBAtoPixbuf_delarray );
    /**
     * Set the GTK2 image object to a pixbuf created in a
     * similar way to RGBAtoPixbuf() from w,h,data
     */
    FERRISEXP_API GtkImage* RGBAtoImage( GtkImage* im, int w, int h, gpointer data,
                           RGBAtoPixbufFree_f f = RGBAtoPixbuf_delarray );
    
    /**
     * Set the GTK2 image object to the image data contained in the
     * given context's attributes. The EA for the context should
     * have width, height, and rgba32 bit image data in them.
     */
    FERRISEXP_API GtkImage* setImageFromRGBAAttribute( GtkImage* im,
                                         fh_context c,
                                         const std::string& rdn_width,
                                         const std::string& rdn_height,
                                         const std::string& rdn_rgba32 );
    
    /**
     * Convenience function that calls setImageFromRGBAAttribute()
     * with the correct EA names for the EXIF thumbnail in TIFF
     * and JPEG/EXIF images.
     */
    FERRISEXP_API GtkImage* setImageFromExifThumb( GtkImage* im, fh_context c );
    

    


    /************************************************/
    /************************************************/
    
    //
    // handy stuff for dealing with gtktree model/views
    //
    typedef std::list< GtkTreeIter > list_gtktreeiter_t;
    /**
     * Get an STL list of all the selected GtkTreeIter* items in the view
     * if useSelection is false then all of the GtkTreeIter* in the view are returned.
     */
    FERRISEXP_API list_gtktreeiter_t getIterList( GtkWidget* w_treeview, bool useSelection = false );
    FERRISEXP_API std::string treestr( GtkTreeIter  iter, GtkWidget* w_treeview, int col );
    FERRISEXP_API std::string treestr( GtkTreeIter *iter, GtkWidget* w_treeview, int col );
    FERRISEXP_API std::string treestr( void* tm_void, GtkTreeIter *iter, int col );
    FERRISEXP_API bool treebool( void* tm_void, GtkTreeIter *iter, int col );
    FERRISEXP_API long treeint( void* tm_void, GtkTreeIter *iter, int col );

    
    FERRISEXP_API int treeint( GtkTreeIter *iter, GtkWidget* w_treeview, int col );
    FERRISEXP_API bool        treebool( GtkTreeIter *iter, GtkWidget* w_treeview, int col );
    FERRISEXP_API gpointer    treeptr( GtkTreeIter *iter, GtkWidget* w_treeview, int col );

    FERRISEXP_API void clearSelection( GtkTreeView* w_treeview );
    
    /**
     * Try to make sure at least one item is selected in the tree. If nothing
     * is currently selected then the item under the mouse event at 'event'
     * will be selected. If nothing was or could be selected then false is returned.
     */
    FERRISEXP_API bool tryToEnsureSomethingIsSelected( GtkWidget* treeview, GdkEvent *event );

    /**
     * Get a pixbuf that is suitable for viewing inline in a tree/list widget
     * This may be a scaled version of the emblem's icon if that icon is too
     * large for normal tree display.
     *
     * This function is mainly used internally at this point.
     */
    FERRISEXP_API GdkPixbuf* getEmblemListViewPixbuf( fh_emblem em );

    
    
    FERRISEXP_API void RunInfoDialog( const std::string& msg, GtkWidget* win = 0 );
    FERRISEXP_API bool RunQuestionDialog( const std::string& msg, GtkWidget* win = 0 );
    FERRISEXP_API void RunErrorDialog( const std::string& msg, GtkWidget* win = 0 );
    

    class FERRISEXP_API FileCompareWindow
    {
    public:
        typedef std::vector<fh_matcher> MatchersList_t;
        MatchersList_t& getAlwaysPermitMatchers();
        MatchersList_t& getAlwaysDenyMatchers();
        
        void addMatchersTo( MatchersList_t& matchersList );
        bool isContextInList( MatchersList_t& matchersList, fh_context c );
        void populateList( fh_context src, fh_context dst = 0 );
        void createMainWindow( const std::string& title = "" );
        virtual void processAllPendingEvents();
        bool processMainWindow();
        
    protected:
        
        enum {
            RPT_DESC       = 0,
            RPT_SRC        = 1,
            RPT_DST        = 2,
            RPT_TARGET     = 2,
            RPT_DSTAUTO    = 3,
            RPT_TARGETAUTO = 3,
            RPT_N_COLUMNS  = 4
        };
        GtkTreeStore*       m_rptmodel;
        GtkWidget*          m_rptview;
        GtkWindow*          m_win;
        bool m_result;
        bool m_looping;
        typedef ::Ferris::Factory::EndingList MatchData_t;
        MatchersList_t AlwaysPermitMatchers;
        MatchersList_t AlwaysDenyMatchers;

        typedef std::list< std::string > DisplayEAList_t;
        
        MatchData_t getMatchingDests();
        virtual void dialog_yes( GtkButton *button );
        virtual void dialog_auto_yes( GtkButton *button );
        virtual void dialog_auto_no( GtkButton *button );
        virtual void dialog_no( GtkButton *button );
        void dialog_auto_toggled(GtkCellRendererToggle *cell, gchar *path_string );

    private:
        DisplayEAList_t getEAList();

        friend void dialog_yes( GtkButton *button, gpointer udata );
        friend void dialog_auto_yes( GtkButton *button, gpointer udata );
        friend void dialog_auto_no( GtkButton *button, gpointer udata );
        friend void dialog_no( GtkButton *button, gpointer udata );
        friend void dialog_auto_toggled(GtkCellRendererToggle *cell,
                                        gchar *path_string, gpointer udata);
        
        };

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    class FERRISEXP_API Quit_Requested
    {
    };


    template <class Iter>
    void populateModelWithSelection( fh_ftreemodel m, Iter b, Iter e )
    {
        fh_context selfactory = Resolve( "selectionfactory://" );
        fh_context selection  = selfactory->createSubContext( "" );
        
        for( ; b != e ; ++b )
        {
            std::string SrcName = *b;
            
            fh_context ctx = Resolve( SrcName );
//             std::cerr << "Adding ctx:" << ctx->getURL()
//                       << " to selection:" << selection->getURL()
//                       << " selfactory:" << selfactory->getURL()
//                       << std::endl;
            selection->createSubContext( "", ctx );
        }
        
        m->setURL( selection );
        m->read();
    }

    
    class FERRISEXP_API GTK_TreeWalkClient
    {
        typedef GTK_TreeWalkClient _Self;
        friend gboolean GTK_TreeWalkClient__window_closing (GtkObject *object, gpointer udata);

    protected:

        guint64 m_highestSeenContextID;
        bool    m_performing;
        Ferrisls_aggregate_t m_totalagg;

        virtual bool shouldPrecacheSourceSize();
        
        Ferrisls_aggregate_t getTotalAggregateData( fh_context src );
        ctxlist_t getAllRemainingSrcsContextList();
        virtual void OnDropped( const ctxlist_t& clist );
        virtual void performActionForSource( fh_context c );
        virtual void updateProgressForAggregateData( const Ferrisls_aggregate_t& agg,
                                                     bool added );
        
        
    public:

        GTK_TreeWalkClient();
        ~GTK_TreeWalkClient();

        void performActionForAllRemainingSources();
        
        void makeMainWindow( const std::string& title = "" );
        void showMainWindow( bool Sloth, bool force = false );
        void processAllPendingEvents();
        void runMainWindow( bool Sloth );

        void makeColumnView( const std::string& cn );
        void makeDefaultColumnViews();

        void addToSkipped( const std::string& desc,  const std::string& reason );
        int  getNumberOfSkipped();

        /**
         * only returns false if the user has selected AutoClose and there are no
         * skipped items and there was no explicit user interaction taken.
         */
        bool getShouldRunMainLoop( bool AutoClose, bool hadUserInteraction );
        
        void updateStartTime();
        void updateElapsedTime();


        GtkWidget*          m_win;
        fh_ftreemodel       m_treemodel;
        GtkWidget*          m_treeview;
        GtkWidget*          m_scrolledwindow;
        GtkWidget*          m_targetlab;
        GtkProgressBar*     m_progress;
        GtkProgressBar*     m_overall_progress;
        GtkWidget*          m_srclab;
        GtkWidget*          m_dstlab;
        GtkWidget*          m_speedlab;
        GtkWidget*          m_currentObjectLab;
        GtkWidget*          m_starttimelab;
        GtkWidget*          m_elapsedtimelab;
        GtkWidget*          m_playpausebut;
        GtkNotebook*        m_notebook;
        enum {
            SKIP_REASON,
            SKIP_DESC,
            SKIP_N_COLUMNS
        };
        GtkTreeStore*       m_skipmodel;
        GtkWidget*          m_skipview;
        
        time_t              actionstart_tt;

        bool                m_running;

        void play_pause_button( GtkButton *button );

        /**
         * calls OnDropped() with a list< fh_context > to make it easy for subclasses
         */
        virtual void drag_data_received(
            GtkWidget          *widget,
            GdkDragContext     *context,
            gint                x,
            gint                y,
            GtkSelectionData   *data,
            guint               info,
            guint               time );
        
    protected:

        typedef gint32 gtk_window_is_closed_t;
        gtk_window_is_closed_t gtk_window_is_closed;

        void resizeBodyElements( GtkWidget* table, int rowCount = 0 );
        void addSeparatorRow( GtkWidget* table, int& r, const char* label );
        virtual void addSourceRow( GtkWidget* table, int& r );
        virtual void addDestinationRow( GtkWidget* table, int& r );
        virtual void addTarget( GtkWidget* table, int& r );
        virtual void addCurrent( GtkWidget* table, int& r );
        virtual void addSpeedRow( GtkWidget* table, int& r );
        virtual void addProgressRow( GtkWidget* table, int& r );
        virtual void addOverallProgressRow( GtkWidget* table, int& r );
        virtual void addStartTimeRow( GtkWidget* table, int& r );
        virtual void addElapsedTimeRow( GtkWidget* table, int& r );
        virtual void addBodyElements( GtkWidget* table, int& r );
        virtual void addSkippingPage();
        virtual void addOtherPages();
        virtual void addVCRControls( GtkWidget* table );

        virtual void setSourceLabel( const std::string& s );
        virtual void setDestinationLabel( const std::string& s );
        virtual void setProgress( double v, const std::string& s );
        virtual void setProgress( double current, double total );
        virtual void setOverallProgress( double v, const std::string& s );
        virtual void setOverallProgress( double current, double total );
        

        /**
         * tryToPlay() / tryToPause() are called when the VCR controls are clicked
         * to play/pause the process. If either function returns false then it is
         * assumed that an error occured and has already been flagged to the user.
         * On a false return the GUI indicators are not changed to any new state.
         */
        virtual bool tryToPlay();
        virtual bool tryToPause();

        void setWindowTitle( const std::string& s );
        
    private:

        void addSmallLabel( GtkWidget* table, GtkWidget* lab, int c, int r );
        void addLargeLabel( GtkWidget* table, GtkWidget* lab, int c, int r );
        
    };
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    
};


namespace Ferris
{
    

class FERRISEXP_API GtkProgressBinder : public sigc::trackable
{
private:

    GtkProgress* w;
    fh_context ctx;
    sigc::connection conn;
    guint32 interval;
    int update_gui;
    guint timer;
    guint32 items_read;

    friend gint GtkProgressBinder_updater(gpointer data);

    
public:

    
    GtkProgressBinder(
        GtkProgress* _w,
        fh_context _ctx,
        guint32 _interval=250,
        int _update_gui=1
        );

    void read_start( NamingEvent_Start_Reading_Context* e );
    void read_stop( NamingEvent_Stop_Reading_Context* e );
    void naming_exists(NamingEvent_Exists* e,
                       const fh_context& subc,
                       std::string o, std::string n);
    
};
 
};
#endif
