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

    $Id: Menus.cpp,v 1.7 2010/09/24 21:31:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "Menus.hh"
#include "GtkFerris.hh"

using namespace std;

namespace FerrisUI
{
    string getMenuItemLabelString( GtkMenuItem* m )
    {
        string ret = "";
        GtkWidget* ch = gtk_bin_get_child( GTK_BIN(m) );
        GtkLabel*l = GTK_LABEL( ch );
        if( l )
        {
            if( const gchar* lt = gtk_label_get_text( l ) )
                ret = lt;
        }
        return ret;
    }
    bool isMenuItemUsed( GtkMenuShell* m, const string& label )
    {
        bool ret = false;
        
        GList* gl = gtk_container_get_children( GTK_CONTAINER(m) );
        while( gl )
        {
            GtkWidget* subitem = GTK_WIDGET(gl->data);

//             cerr << "isMenuItemUsed() label:" << label
//                  << " mi.label:" <<  getMenuItemLabelString( GTK_MENU_ITEM( subitem ) )
//                  << endl;
            
            if( starts_with( getMenuItemLabelString( GTK_MENU_ITEM( subitem ) ), label )
                || starts_with( getMenuItemLabelString( GTK_MENU_ITEM( subitem ) ), (string)"< " + label )
                )
                return true;
            gl = g_list_next( gl );
        }
        return ret;
    }
    
    
    bool hasParentMenuItem( GtkMenuItem* m )
    {
        GtkWidget* par = GTK_WIDGET(m);
        g_object_get( m, "parent", &par, 0 );
        par = gtk_menu_get_attach_widget( GTK_MENU(par) );
        return GTK_IS_MENU_ITEM( par );
    }
    
    GtkMenuItem* getParentMenuItem( GtkMenuItem* m )
    {
        GtkWidget* par = GTK_WIDGET(m);
        g_object_get( m, "parent", &par, 0 );
        par = gtk_menu_get_attach_widget( GTK_MENU(par) );
        return GTK_MENU_ITEM( par );
    }

    GtkMenuItem* getRootMenuItem( GtkMenuItem* mi )
    {
        GtkMenuItem* t = getParentMenuItem( mi );
        if( !t )
            return mi;
        return getRootMenuItem( t );
    }
    
    string makeMenuPath( GtkMenuItem* mi, GtkWidget* rootm )
    {
        string path = tostr( mi );
        if( gtk_menu_item_get_submenu( mi ) == (GtkWidget*)rootm )
            path = "";
        
        while( gtk_menu_item_get_submenu( mi ) != (GtkWidget*)rootm &&
               hasParentMenuItem( mi ) )
        {
            mi = getParentMenuItem( mi );
            if( gtk_menu_item_get_submenu( mi ) == (GtkWidget*)rootm )
                break;
            
            path = tostr( mi ) + "/" + path;
        }
        return path;
    }
    
    GtkWidget* makeStockMenuItem( const gchar *stock_id )
    {
        return gtk_image_menu_item_new_from_stock( stock_id, 0 );
    }

    
    GtkWidget* makeMenuItem( const std::string& label,
                             std::string iconpath,
                             bool isCheckedMenuItem )
    {
        GtkWidget* child;
        if( isCheckedMenuItem )
        {
            child = gtk_check_menu_item_new_with_label( label.c_str() );
        }
        else
        {
            child = gtk_image_menu_item_new_with_label( label.c_str() );
        }
        
        
        if( !iconpath.empty() )
        {
            iconpath = resolveToIconPath( iconpath );

            GtkWidget* imagewid = 0;
            if( starts_with( iconpath, "gtk-" ))
            {
                imagewid = gtk_image_new_from_stock( iconpath.c_str(), GTK_ICON_SIZE_MENU );
            }
            else
            {
                imagewid = gtk_image_new_from_file( iconpath.c_str() );
            }
            
            gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(child),
                                           imagewid );
        }

        return child;
    }
    GtkWidget* makeNonCheckedMenuItem( const std::string& label,
                                       std::string iconpath )
    {
        return makeMenuItem( label, iconpath );
    }
    GtkWidget* makeCheckedMenuItem( const std::string& label, std::string iconpath )
    {
        return makeMenuItem( label, iconpath, true );
    }
    
    
    

    GtkWidget*
    makeSubMenu( GtkWidget* m, 
                 const std::string& labstr,
                 bool PutInSubMenu,
                 const std::string& iconpath )
    {
        GtkWidget* child;

        if( PutInSubMenu )
        {
            GtkWidget* menu = m;

            child = makeMenuItem( labstr, iconpath );
            gtk_menu_shell_append( GTK_MENU_SHELL(menu), child );
            m = gtk_menu_new();
            gtk_menu_item_set_submenu( GTK_MENU_ITEM(child), m );
        }
        return m;
    }

    GtkWidget* createMenuItem( GtkWidget* m,
                               GCallback cb,
                               const std::string& label,
                               const std::string& icon,
                               gpointer user_data )
    {
        GtkWidget* child;
                                 
        child = makeMenuItem( label, icon );
        g_signal_connect_data( G_OBJECT( child ), "activate",
                               G_CALLBACK (cb), user_data,
                               0, GConnectFlags(0));
        gtk_menu_shell_append( GTK_MENU_SHELL(m), child );
        return child;
    }


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    static gboolean DropReference_cb(GtkObject *object, gpointer udata)
    {
        Handlable* h = (Handlable*)udata;
        h->Release();
        return 0;
    }

    /**
     * bump the reference count of the handlable and have it drop again
     * when the GTK object is destroyed.
     */
    static void BindHandlableToGObject( void* o, Handlable* h )
    {
        h->AddRef();
        gtk_signal_connect(GTK_OBJECT (o), "destroy",
                           GTK_SIGNAL_FUNC(DropReference_cb), h );
    }

    /********************************************************************************/
    /********************************************************************************/
    
    const char* const GOBJ_MFFS_CONTEXT_K = "GOBJ_MFFS_CONTEXT_K";
    const char* const GOBJ_MFFS_THIS_K    = "GOBJ_MFFS_THIS_K";
    const char* const GOBJ_MFFS_MENULOADED_K    = "GOBJ_MFFS_MENULOADED_K";
    
    static void leafCallback_null( MenuFromFilesystem*, fh_context c, GtkMenuItem* )
    {
        if( !c )
            cerr << "Error leaf callback not set and NO CONTEXT PARAM GIVEN" << endl;
        else
            cerr << "Warning leaf callback not set. c:" << c->getURL() << endl;
    }

    static void internalCallback_null( MenuFromFilesystem*, fh_context c, GtkMenuItem* )
    {
    }
    
    static void
    lazy_make_menu_from_filesystem_cb( GtkWidget* topmenu, gpointer user_data )
    {
        try
        {
            GtkMenuItem *menuitem = GTK_MENU_ITEM(
                gtk_menu_get_attach_widget( GTK_MENU(topmenu) ));
            
            if( g_object_get_data( G_OBJECT(menuitem), GOBJ_MFFS_MENULOADED_K ) )
                return;
        
            GtkMenuItem* m       = menuitem;
            GtkWidget* subm      = gtk_menu_item_get_submenu( m );
            Context*   c         = (Context*)g_object_get_data( G_OBJECT(m), GOBJ_MFFS_CONTEXT_K );
            MenuFromFilesystem* selfp = (MenuFromFilesystem*)g_object_get_data( G_OBJECT(m), GOBJ_MFFS_THIS_K );
            cerr << "lazy_make_menu_from_filesystem_cb() c:" << c->getURL() << endl;
            cerr << "lazy_make_menu_from_filesystem_cb() self:" << toVoid(selfp) << endl;
            cerr << "lazy_make_menu_from_filesystem_cb() udata:" << toVoid(user_data) << endl;
            cerr << "lazy_make_menu_from_filesystem_cb() subm:" << toVoid(subm) << endl;

            selfp->makeOneLevelOfMenu( subm, c );
            gtk_widget_show_all( subm );
            gtk_menu_item_set_submenu( menuitem, subm );
        
            g_object_set_data( G_OBJECT(menuitem), GOBJ_MFFS_MENULOADED_K, (void*)1 );
            g_object_set_data( G_OBJECT(subm),     GOBJ_MFFS_MENULOADED_K, (void*)1 );
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "Problem lazy loading filesystem menu\n" << e.what();
            cerr << tostr(ss);
            FerrisUI::RunErrorDialog( tostr(ss) );
        }
    }

    static void
    MenuFromFilesystem_callLeafFunctor( GtkMenuItem *menuitem, gpointer user_data )
    {
        MenuFromFilesystem* selfp = (MenuFromFilesystem*)user_data;
        selfp->callLeafFunctor( menuitem );
    }

    static void
    MenuFromFilesystem_callInternalFunctor( GtkMenuItem *menuitem, gpointer user_data )
    {
        MenuFromFilesystem* selfp = (MenuFromFilesystem*)user_data;
        selfp->callInternalFunctor( menuitem );
    }
    
    MenuFromFilesystem::MenuFromFilesystem( fh_context c )
        :
        m_ctx( c ),
        m_topmenu( 0 ),
        m_leafCallback( leafCallback_null ),
        m_InternalCallback( internalCallback_null ),
        m_haveInternalCallback( false ),
        m_useSubMenu( false ),
        m_root_label( "" ),
        m_root_iconpath( "" )
    {
    }

    MenuFromFilesystem::MenuFromFilesystem( void* menu,
                                            fh_context c,
                                            const Callback_t& f,
                                            const std::string& label,
                                            const std::string& iconpath )
        :
        m_ctx( c ),
        m_topmenu( 0 ),
        m_leafCallback( leafCallback_null ),
        m_InternalCallback( internalCallback_null ),
        m_haveInternalCallback( false ),
        m_useSubMenu( false ),
        m_root_label( "" ),
        m_root_iconpath( "" )
    {
        setMenuToAttachTo( GTK_WIDGET( menu ) );
        setAttachInSubMenu( label, iconpath );
        setLeafCallback( f );
    }
    

    void
    MenuFromFilesystem::setAttachChildrenDirectly( bool v )
    {
        m_useSubMenu = v;
    }
    
    void
    MenuFromFilesystem::setup( void* menu,
                               fh_context c,
                               const Callback_t& f,
                               const std::string& label,
                               const std::string& iconpath )
    {
        m_ctx = c;
        setMenuToAttachTo( GTK_WIDGET( menu ) );
        setAttachInSubMenu( label, iconpath );
        setLeafCallback( f );
    }

    void
    MenuFromFilesystem::setup( void* menu,
                               fh_context c,
                               const Callback_t& internal,
                               const Callback_t& leaf,
                               const std::string& label,
                               const std::string& iconpath )
    {
        m_ctx = c;
        setMenuToAttachTo( GTK_WIDGET( menu ) );
        setAttachInSubMenu( label, iconpath );
        setInternalCallback( internal );
        setLeafCallback( leaf );
    }
    
    
    
    
    MenuFromFilesystem::~MenuFromFilesystem()
    {
        cerr << "~MenuFromFilesystem() ---------------------------------------" << endl;
    }

    bool
    MenuFromFilesystem::shouldVetoContext( fh_context c )
    {
        return false;
    }
    
    bool
    MenuFromFilesystem::isLeaf( fh_context c )
    {
        try
        {
            c->read();
            cerr << "MenuFromFilesystem::isLeaf(2) c:" << c->getURL()
                 << " ret:" << !c->hasSubContexts()
                 << endl;
            return !c->hasSubContexts();
        }
        catch( exception& e )
        {
            LG_GTKFERRIS_D << "isLeaf() for c:" << c->getURL()
                           << " got error:" << e.what() << endl;
        }
        return true;
    }

    
    GtkWidget*
    MenuFromFilesystem::makeLeafMenuItem( fh_context c,
                                          const std::string& label,
                                          std::string iconpath,
                                          bool isInternalLeaf )
    {
        return makeMenuItem( label, iconpath );
    }
    
    GtkWidget*
    MenuFromFilesystem::makeInternalMenuItem( fh_context c,
                                              const std::string& label,
                                              std::string iconpath )
    {
        return makeMenuItem( label, iconpath );
    }
        
    void
    MenuFromFilesystem::setMenuToAttachTo( GtkWidget* m )
    {
        m_topmenu = GTK_WIDGET( m );
    }
    
    void
    MenuFromFilesystem::setMenuToAttachTo( GtkMenu* m )
    {
        m_topmenu = GTK_WIDGET( m );
    }
    

    void
    MenuFromFilesystem::setLeafCallback( const Callback_t& f )
    {
        m_leafCallback = f;
    }

    void
    MenuFromFilesystem::setInternalCallback( const Callback_t& f )
    {
        m_InternalCallback = f;
        m_haveInternalCallback = true;
    }
    
    
    void
    MenuFromFilesystem::setAttachInSubMenu( const std::string& label, const std::string& iconpath )
    {
        m_useSubMenu    = true;
        m_root_label    = label;
        m_root_iconpath = iconpath;
    }


    void
    MenuFromFilesystem::setup_menuobject( void* o, fh_context c )
    {
        g_object_set_data( G_OBJECT(o), GOBJ_MFFS_CONTEXT_K,  GetImpl(c) );
        g_object_set_data( G_OBJECT(o), GOBJ_MFFS_THIS_K,     this       );
    }

    void
    MenuFromFilesystem::callLeafFunctor( GtkMenuItem *menuitem )
    {
        fh_context c = (Context*)g_object_get_data( G_OBJECT(menuitem), GOBJ_MFFS_CONTEXT_K );
        cerr << "callLeafFunctor() this:" << toVoid(this) << endl;
        cerr << "callLeafFunctor() c:" << toVoid(GetImpl(c)) << endl;
        cerr << "callLeafFunctor() c:" << c->getURL() << endl;
        m_leafCallback( this, c, menuitem );
    }

    void
    MenuFromFilesystem::callInternalFunctor( GtkMenuItem *menuitem )
    {
        fh_context c = (Context*)g_object_get_data( G_OBJECT(menuitem), GOBJ_MFFS_CONTEXT_K );
        cerr << "callInternalFunctor() this:" << toVoid(this) << endl;
        cerr << "callInternalFunctor() c:" << toVoid(GetImpl(c)) << endl;
        cerr << "callInternalFunctor() c:" << c->getURL() << endl;
        m_InternalCallback( this, c, menuitem );
    }
    
    void
    MenuFromFilesystem::makeOneLevelOfMenu( GtkWidget* m, fh_context dir )
    {
        GtkWidget* child;
        int i=0;

        LG_GTKFERRIS_D << "makeOneLevelOfMenu() m:" << toVoid(m) << " dir:" << dir->getURL() << endl;

        bool first = m_haveInternalCallback;
        for( Context::iterator ci = dir->begin(); ci != dir->end(); ++ci )
        {
            if( first )
            {
                first = false;

                string label    = "< " + dir->getDirName() + " >";
                if( !isMenuItemUsed( GTK_MENU_SHELL(m), dir->getDirName() ) )
                {
                    string iconpath = getStrAttr( dir, "ferris-iconname", "" );
                    GtkWidget* child = makeLeafMenuItem( dir, label, iconpath, true );

                    if( child )
                    {
                        gtk_menu_shell_append( GTK_MENU_SHELL(m), child );
                        g_signal_connect_data( G_OBJECT( child ), "activate",
                                               G_CALLBACK (MenuFromFilesystem_callInternalFunctor), this,
                                               0, GConnectFlags(0));
                        BindHandlableToGObject( child, GetImpl(dir) );
                        setup_menuobject( child, dir );
                    }
                }
            }
            
            fh_context c    = *ci;
            string label    = c->getDirName();

            if( shouldVetoContext( c ) )
                continue;

            if( !isMenuItemUsed( GTK_MENU_SHELL(m), label ) )
            {
                string iconpath = getStrAttr( c, "ferris-iconname", "" );
                iconpath        = resolveToIconPath( iconpath );

                LG_GTKFERRIS_D << "makeOneLevelOfMenu(iter) m:" << m 
                               << " label:" << label
                               << " icon:" << iconpath
                               << " c:" << c->getURL()
                               << endl;
            
                if( isLeaf( c ) )
                {
                    LG_GTKFERRIS_D << "makeOneLevelOfMenu(leaf) m:" << m 
                                   << " label:" << label
                                   << " icon:" << iconpath
                                   << endl;
                
                    // Add this leaf node
                    child = makeLeafMenuItem( c, label, iconpath );

                    if( child )
                    {
                        cerr << "Attaching leaf. this:" << toVoid(this) << endl;
                        gtk_menu_shell_append( GTK_MENU_SHELL(m), child );
                        g_signal_connect_data( G_OBJECT( child ), "activate",
                                               G_CALLBACK (MenuFromFilesystem_callLeafFunctor), this,
                                               0, GConnectFlags(0));
                        BindHandlableToGObject( child, GetImpl(c) );
                        setup_menuobject( child, c );
                    }
                }
                else
                {
                    LG_GTKFERRIS_D << "makeOneLevelOfMenu(internal) m:" << m 
                                   << " label:" << label
                                   << " icon:" << iconpath
                                   << endl;
                    
                    // Make a submenu for directories 
                    makeInternalMenuItem( m, c, label, iconpath );
                }
            }
        }
    }
    
    void
    MenuFromFilesystem::makeInternalMenuItem( GtkWidget* m, fh_context c,
                                              const std::string& submenulabel,
                                              const std::string& submenuicon )
    {
        GtkWidget* child;
        int i=0;

        cerr << "makeInternalMenuItem(). this:" << toVoid(this) << endl;
        
        GtkWidget* menu = m;
        child           = makeInternalMenuItem( c, submenulabel, submenuicon );
        gtk_menu_shell_append( GTK_MENU_SHELL(menu), child );
        menu = gtk_menu_new();
        gtk_menu_item_set_submenu( GTK_MENU_ITEM(child), menu );
            
        g_signal_connect_data( G_OBJECT( menu ), "show",
                               G_CALLBACK (lazy_make_menu_from_filesystem_cb), this,
                               0, GConnectFlags(0));
        
        BindHandlableToGObject( menu, GetImpl(c) );
        setup_menuobject( child, c );
        setup_menuobject( menu,  c );
    }

    void
    MenuFromFilesystem::perform()
    {
        LG_GTKFERRIS_D << "MenuFromFilesystem::perform() m_useSubMenu:" << m_useSubMenu << endl;
        
        if( m_useSubMenu )
        {
            makeInternalMenuItem( m_topmenu, m_ctx, m_root_label, m_root_iconpath );
        }
        else
        {
            makeOneLevelOfMenu( m_topmenu, m_ctx );
        }
    }
    
        
    
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
//     static const char* const GOBJ_CONTEXT_K    = "GOBJ_CONTEXT_K";
//     static const char* const GOBJ_ROOTAPPW_K   = "GOBJ_ROOTAPPW_K";
//     static const char* const GOBJ_MENUCB_K     = "GOBJ_MENUCB_K";
//     static const char* const GOBJ_MENUD_K      = "GOBJ_MENUD_K";
//     static const char* const GOBJ_MENULOADED_K = "GOBJ_MENULOADED_K";
//     static const char* const GOBJ_LEAFMAKER_K  = "GOBJ_LEAFMAKER_K";

//     static void setup_menuobject( void* o, fh_context c, GtkWidget* menu,
//                                   GCallback menu_cb, gpointer menu_cb_userdata,
//                                   makeLeafMenuItem_f leafMaker )
//     {
//         g_object_set_data( G_OBJECT(o), GOBJ_CONTEXT_K,  GetImpl(c) );
//         g_object_set_data( G_OBJECT(o), GOBJ_ROOTAPPW_K, menu );
//         g_object_set_data( G_OBJECT(o), GOBJ_MENUCB_K,    (void*)menu_cb );
//         g_object_set_data( G_OBJECT(o), GOBJ_MENUD_K,     (void*)menu_cb_userdata );
//         g_object_set_data( G_OBJECT(o), GOBJ_LEAFMAKER_K, (void*)leafMaker );
//     }

//     fh_context
//     getContextFromFilesystemMenu( GtkMenuItem* mi )
//     {
//         Context* c = (Context*)g_object_get_data( G_OBJECT(mi), GOBJ_CONTEXT_K );
//         return c;
//     }
    
    
//     static void
//     lazy_make_menu_from_filesystem_cb( GtkWidget* topmenu, gpointer user_data )
//     {
//         try
//         {
//             GtkMenuItem *menuitem = GTK_MENU_ITEM(
//                 gtk_menu_get_attach_widget( GTK_MENU(topmenu) ));
            
//             if( g_object_get_data( G_OBJECT(menuitem), GOBJ_MENULOADED_K ) )
//                 return;
        
//             GtkMenuItem* m       = menuitem;
//             GtkWidget* subm      = gtk_menu_item_get_submenu( m );
//             GtkWidget* rootm     = (GtkWidget*)g_object_get_data( G_OBJECT(m), GOBJ_ROOTAPPW_K );
//             GCallback  menu_cb   = (GCallback) g_object_get_data( G_OBJECT(m), GOBJ_MENUCB_K );
//             gpointer   menu_cb_d = (gpointer)  g_object_get_data( G_OBJECT(m), GOBJ_MENUD_K );
//             Context*   c         = (Context*)g_object_get_data( G_OBJECT(m), GOBJ_CONTEXT_K );
//             makeLeafMenuItem_f leafMaker = (makeLeafMenuItem_f)g_object_get_data(G_OBJECT(m), GOBJ_LEAFMAKER_K );

//             makeMenuFromFilesystem( subm, c, menu_cb, menu_cb_d, leafMaker );
//             gtk_widget_show_all( subm );
//             gtk_menu_item_set_submenu( menuitem, subm );
        
//             g_object_set_data( G_OBJECT(menuitem), GOBJ_MENULOADED_K, (void*)1 );
//             g_object_set_data( G_OBJECT(subm),     GOBJ_MENULOADED_K, (void*)1 );
//         }
//         catch( exception& e )
//         {
//             fh_stringstream ss;
//             ss << "Problem lazy loading filesystem menu\n" << e.what();
//             cerr << tostr(ss);
//             FerrisUI::RunErrorDialog( tostr(ss) );
//         }
//     }

//     void makeMenuFromFilesystem( GtkWidget* m,
//                                  fh_context dir,
//                                  GCallback menu_cb,
//                                  gpointer menu_cb_userdata,
//                                  makeLeafMenuItem_f leafMaker )
//     {
//         GtkWidget* child;
//         int i=0;

//         cerr << "makeMenuFromFilesystem() m:" << m << " dir:" << dir->getURL() << endl;
        
//         for( Context::iterator ci = dir->begin(); ci != dir->end(); ++ci )
//         {
//             fh_context c  = *ci;
//             string label  = c->getDirName();
//             string iconpath = getStrAttr( c, "ferris-iconname", "" );
//             iconpath = resolveToIconPath( iconpath );

//             cerr << "makeMenuFromFilesystem() m:" << m 
//                  << " label:" << label
//                  << " icon:" << iconpath
//                  << endl;

//             c->read();
//             if( c->hasSubContexts() )
//             {
//                 // Make a submenu for directories 

//                 makeMenuFromFilesystem( m, c, menu_cb, menu_cb_userdata, label, iconpath, leafMaker );
// //                private_makeLazyAppsMenu( m, root_of_apps_menu, c, menu_cb, tobj );
//             }
//             else
//             {
//                 // Add this leaf node
//                 child = leafMaker( label, iconpath );
// //                child = makeMenuItem( label, iconpath );

//                 if( child )
//                 {
//                     gtk_menu_shell_append( GTK_MENU_SHELL(m), child );
//                     g_signal_connect_data( G_OBJECT( child ), "activate",
//                                            G_CALLBACK (menu_cb), menu_cb_userdata,
//                                            0, GConnectFlags(0));
//                     BindHandlableToGObject( child, GetImpl(c) );
//                     setup_menuobject( child, c, m, menu_cb, menu_cb_userdata, leafMaker );
//                 }
//             }
//         }
//     }


//     void makeMenuFromFilesystem( GtkWidget* m,
//                                  fh_context c,
//                                  GCallback menu_cb,
//                                  gpointer menu_cb_userdata,
//                                  const std::string& submenulabel,
//                                  const std::string& submenuicon,
//                                  makeLeafMenuItem_f leafMaker )
//     {
//         GtkWidget* child;
//         int i=0;

//         GtkWidget* menu = m;
//         child           = makeMenuItem( submenulabel, "" );
//         gtk_menu_shell_append( GTK_MENU_SHELL(menu), child );
//         menu = gtk_menu_new();
//         gtk_menu_item_set_submenu( GTK_MENU_ITEM(child), menu );
            
//         g_signal_connect_data( G_OBJECT( menu ), "show",
//                                G_CALLBACK (lazy_make_menu_from_filesystem_cb), 0,
//                                0, GConnectFlags(0));

//         BindHandlableToGObject( menu, GetImpl(c) );
//         setup_menuobject( child, c, menu, menu_cb, menu_cb_userdata, leafMaker );
//         setup_menuobject( menu,  c, menu, menu_cb, menu_cb_userdata, leafMaker );
//     }
//     void makeMenuFromFilesystem( GtkMenu* m,
//                                  fh_context c,
//                                  GCallback menu_cb,
//                                  gpointer menu_cb_userdata,
//                                  const std::string& submenulabel,
//                                  const std::string& submenuicon,
//                                  makeLeafMenuItem_f leafMaker )
//     {
//         makeMenuFromFilesystem( GTK_WIDGET(m),
//                                 c, menu_cb, menu_cb_userdata,
//                                 submenulabel, submenuicon,
//                                 leafMaker );
//     }
    
    

    
};
