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

    $Id: Menus.hh,v 1.6 2010/09/24 21:31:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRISUI_MENUS_H_
#define _ALREADY_INCLUDED_FERRISUI_MENUS_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <gtk/gtk.h>
#include <Ferris/Ferris.hh>

namespace FerrisUI
{
    using namespace Ferris;

    /**
     * Does the given menu item have a parent
     * or is it a top level menu item
     */
    FERRISEXP_API bool hasParentMenuItem( GtkMenuItem* m );

    /**
     * Get the parent menuitem of the menuitem if it exists
     * or 0 otherwise
     */
    FERRISEXP_API GtkMenuItem* getParentMenuItem( GtkMenuItem* m );

    FERRISEXP_API GtkMenuItem* getRootMenuItem( GtkMenuItem* mi );
    
    /**
     * Trace out a path from mi back to rootm.
     * the string path will be of the form
     * something/else/mi
     */
    FERRISEXP_API std::string makeMenuPath( GtkMenuItem* mi, GtkWidget* rootm );

    /**
     * Make a menuitem from a given stock icon name
     */
    FERRISEXP_API GtkWidget* makeStockMenuItem( const gchar *stock_id );

    /**
     * Create a menu item with the given label and icon.
     * note that icons paths can include ~/ and icons://
     * type ferris URLs.
     *
     * @param isCheckedMenuItem set to true if the menuitem should be a checked item
     *         instead of a normal one.
     */
    FERRISEXP_API GtkWidget* makeMenuItem( const std::string& label,
                                           std::string iconpath,
                                           bool isCheckedMenuItem = false );

    /**
     * If PutInSubMenu is true then create a submenu with the
     * given label and icon and place it into m and return
     * the newly created menu
     */
    FERRISEXP_API GtkWidget* makeSubMenu( GtkWidget* m, 
                                          const std::string& label,
                                          bool PutInSubMenu,
                                          const std::string& iconpath );

    /**
     * Make a new menu item with optional icon and attach it to the menu
     * and callback
     */
    FERRISEXP_API GtkWidget* createMenuItem( GtkWidget* m,
                                             GCallback cb,
                                             const std::string& label,
                                             const std::string& icon,
                                             gpointer user_data );


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    class MenuFromFilesystem;
    FERRIS_SMARTPTR( MenuFromFilesystem, fh_mffs );

    
    
    /**
     * Create a GTK+ menu from a filesystem. This allows one to veto the creation
     * of many menuitems and give a projection of the filesystem at runtime based
     * on external information to this class.
     */
    class FERRISEXP_API MenuFromFilesystem
        :
        public Handlable
    {
        typedef MenuFromFilesystem _Self;

        fh_context m_ctx;
        GtkWidget* m_topmenu;

        bool        m_useSubMenu;
        std::string m_root_label;
        std::string m_root_iconpath;

        void setup_menuobject( void* o, fh_context c );

    protected:

        /**
         * test to see if this context should have a submenu or be a leaf in
         * the menu.
         * by default if the context has children then it has a submenu
         */
        virtual bool isLeaf( fh_context c );

        /**
         * If the context should not appear in the menu then a subclass
         * can return true here and it will be ignored.
         */
        virtual bool shouldVetoContext( fh_context c );

        /**
         * Make a leaf menuitem widget. All attaching and callbacks are added for
         * you by the caller. You must set the icon/label from the args and return
         * the widget. If you wish to not display this context then return 0.
         *
         * @param isInternalLeaf is true when this is a leaf menu item representing
         *                       the selection of an internal menu item.
         */
        virtual GtkWidget* makeLeafMenuItem( fh_context c,
                                             const std::string& label,
                                             std::string iconpath,
                                             bool isInternalLeaf = false );

        /**
         * Make a leaf menuitem widget. All attaching and callbacks are added for
         * you by the caller. You must set the icon/label from the args and return
         * the widget. If you wish to not display this context then return 0.
         */
        virtual GtkWidget* makeInternalMenuItem( fh_context c, const std::string& label, std::string iconpath );
        
    public:

        typedef Loki::Functor< void, LOKI_TYPELIST_3( MenuFromFilesystem*, fh_context, GtkMenuItem* ) > Callback_t;
        
        /**
         * Make a menu filesystem. Set c to the base of the menu tree.
         */
        MenuFromFilesystem( fh_context c = 0 );
        virtual ~MenuFromFilesystem();

        /**
         * Build a full menu maker in one call
         */
        MenuFromFilesystem( void* menu,
                            fh_context c,
                            const Callback_t& f,
                            const std::string& label = "",
                            const std::string& iconpath = "" );

        /**
         * One call setup, args are the same for constructor but
         * this call is available for object reuse.
         */ 
        void setup( void* menu,
                    fh_context c,
                    const Callback_t& f,
                    const std::string& label = "",
                    const std::string& iconpath = "" );
        void setup( void* menu,
                    fh_context c,
                    const Callback_t& internal,
                    const Callback_t& leaf,
                    const std::string& label,
                    const std::string& iconpath = "" );

        /**
         * Place begin() to end() into the menu itself instead of making a submenu
         * for the root context.
         */
        void setAttachChildrenDirectly( bool v );
        
        /**
         * This is the menu we will generate a menu tree for.
         */
        void setMenuToAttachTo( GtkWidget* m );
        void setMenuToAttachTo( GtkMenu* m );

        /**
         * Set the callback functor that is made when a leaf
         * menu item is selected.
         */
        void setLeafCallback( const Callback_t& f );

        /**
         * When the menu is created, attach children of the root into a submenu
         * instead of right into the given menu
         * @param label the label for the top level menuitem that the submenu
         *              will be attached to. If not given then the roots getDirName() will be
         *              used.
         * @param iconpath icon for the menuitem at root.
         */
        void setAttachInSubMenu( const std::string& label = "", const std::string& iconpath = "" );
        

        /**
         * Create the menu and attach it to the menu set with setMenuToAttachTo()
         */
        void perform();


        /********************************************************************************/
        /********************************************************************************/
        /*** INTERNAL USE ONLY **********************************************************/
        /********************************************************************************/
        void callLeafFunctor( GtkMenuItem *menuitem );
        void callInternalFunctor( GtkMenuItem *menuitem );

        /**
         * Creates a new menuitem in 'm' with the given label/icon to display
         * the contents of 'c' when it is shown
         */
        void makeInternalMenuItem( GtkWidget* m, fh_context c,
                                   const std::string& submenulabel,
                                   const std::string& submenuicon );

        /**
         * Make a single level of menu placing items from dir into m
         */
        void makeOneLevelOfMenu( GtkWidget* m, fh_context dir );

        
    private:
        Callback_t m_leafCallback;
        bool       m_haveInternalCallback;
        Callback_t m_InternalCallback;

    public:
        
        /**
         * Set the callback functor that is made when an internal
         * menu item is selected.
         */
        void setInternalCallback( const Callback_t& f );


    };
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
//     /**
//      * Callbacks attached to menus created with makeMenuFromFilesystem()
//      * can get at the context for the menuitem that was activated using
//      * this function.
//      */
//     fh_context
//     getContextFromFilesystemMenu( GtkMenuItem* mi );


//     typedef GtkWidget* (*makeLeafMenuItem_f)( const std::string& label,
//                                             std::string iconpath );
//     GtkWidget* makeNonCheckedMenuItem( const std::string& label, std::string iconpath );
//     GtkWidget* makeCheckedMenuItem( const std::string& label, std::string iconpath );
    
    
//     /**
//      * Create a menu in a lazy way (ie only create it as its explored).
//      *
//      * This is slightly different from the overloaded version in that it
//      * dumps each child of 'c' right into the menushell 'm'. See the overloaded
//      * version for more info
//      *
//      * @param m a menu shell to add to
//      * @param c is the filesystem to show
//      * @param menu_cb is the activation function for menu selection
//      * @param menu_cb_userdata is passed to menu_cb
//      * @param leafMaker a function pointer that takes the menuitem label string and
//      *                  iconpath and returns a GtkMenuItem or one of its subtypes
//      *                  see makeNonCheckedMenuItem() and makeCheckedMenuItem()
//      *                  NOTE: the leafMaker can return 0 if this menuitem should be skipped
//      */
//     void makeMenuFromFilesystem( GtkWidget* m,
//                                  fh_context c,
//                                  GCallback menu_cb,
//                                  gpointer menu_cb_userdata = 0,
//                                  makeLeafMenuItem_f leafMaker = makeNonCheckedMenuItem );

//     /**
//      * Create a menu in a lazy way (ie only create it as its explored).
//      *
//      * This creates a new entry in the menushell 'm' for the context 'c' itself
//      * using the given icon and label. When leaf nodes in the menu structure are
//      * activated then menu_cb() will be called with menu_cb_userdata as an arg.
//      * menu_cb() will not be called for directories.
//      *
//      * @param m a menu shell to add to
//      * @param c is the filesystem to show
//      * @param menu_cb is the activation function for menu selection
//      * @param menu_cb_userdata is passed to menu_cb
//      * @param submenulabel if not an empty string is the name of a submenu
//      *        that this menu will be placed in instead of the top level menu 'm'
//      * @param leafMaker a function pointer that takes the menuitem label string and
//      *                  iconpath and returns a GtkMenuItem or one of its subtypes
//      *                  see makeNonCheckedMenuItem() and makeCheckedMenuItem()
//      *                  NOTE: the leafMaker can return 0 if this menuitem should be skipped
//      */
//     void makeMenuFromFilesystem( GtkWidget* m,
//                                  fh_context c,
//                                  GCallback menu_cb,
//                                  gpointer menu_cb_userdata,
//                                  const std::string& submenulabel,
//                                  const std::string& submenuicon,
//                                  makeLeafMenuItem_f leafMaker = makeNonCheckedMenuItem );
//     void makeMenuFromFilesystem( GtkMenu* m,
//                                  fh_context c,
//                                  GCallback menu_cb,
//                                  gpointer menu_cb_userdata,
//                                  const std::string& submenulabel,
//                                  const std::string& submenuicon,
//                                  makeLeafMenuItem_f leafMaker = makeNonCheckedMenuItem );
    
};

#endif

