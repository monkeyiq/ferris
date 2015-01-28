/******************************************************************************
*******************************************************************************
*******************************************************************************

    Ferris UI
    Copyright (C) 2002 Ben Martin

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

    $Id: FerrisOptionMenu.cpp,v 1.2 2010/09/24 21:31:05 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "GtkFerris.hh"
#include "FerrisOptionMenu.hh"

using namespace std;

namespace FerrisUI
{
    static const char* GOBJ_MENU_K = "GOBJ_MENU_K";
    static const char* GOBJ_BASE_K = "GOBJ_BASE_K";
    
    static gint fopm_menu_button(GtkWidget *widget, GdkEvent *event, gpointer user_data )
    {
        FerrisOptionMenu* fopm = (FerrisOptionMenu*)user_data;
        return fopm->menu_button( widget, event );
    }

    static void fopm_menu_activate( GtkMenuItem *menuitem, gpointer user_data )
    {
        FerrisOptionMenu* fopm = (FerrisOptionMenu*)user_data;
        return fopm->menu_activate( menuitem );
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    FerrisOptionMenu::FerrisOptionMenu( const std::string& label,
                                        const std::string& iconpath )
        :
        m_label( label ),
        m_iconpath( iconpath ),
        m_menu( 0 ),
        m_userwidget( 0 ),
        MaxButtonTextLength( -1 ),
        MaxMenuElementsPerContext( 30 ),
        m_button( 0 )
    {
        setLabel( label );
    }

    void
    FerrisOptionMenu::menu_activate( GtkMenuItem *menuitem )
    {
        GtkWidget* button = GTK_WIDGET(g_object_get_data( G_OBJECT(menuitem), GOBJ_BASE_K ));

        setLabel( tostr(menuitem) );
    }

    void
    FerrisOptionMenu::setLabel( const std::string& s )
    {
        if( getLabelChangeSig().emit( this, m_label, s ) )
            return;
        
        m_label = s;
        string l = s;
        if( MaxButtonTextLength > 0 )
            l = s.substr( 0, MaxButtonTextLength );

        if( m_button )
            gtk_button_set_label( GTK_BUTTON(m_button), l.c_str() );
    }
    
    

    gint
    FerrisOptionMenu::menu_button(GtkWidget *widget, GdkEvent *event )
    {
        GtkMenu *menu;
        GdkEventButton *event_button;

        g_return_val_if_fail (widget != NULL, FALSE);
        g_return_val_if_fail (event != NULL, FALSE);

        if( event->type == GDK_BUTTON_PRESS )
        {
            event_button = (GdkEventButton *) event;
            
            if (event_button->button == 1 || event_button->button == 3 )
            {
                GtkMenu* menu = GTK_MENU(g_object_get_data( G_OBJECT(widget), GOBJ_MENU_K ));
                showPopup( menu );
                return true;
            }
        }
        return false;
    }
    
    void
    FerrisOptionMenu::ensureWidgetsCreated()
    {
        GtkWidget* lab;
        GtkWidget* w;
        GtkWidget* b;
        
        if( m_userwidget )
            return;
        
        m_userwidget = gtk_table_new ( 1, 2, false );
        m_button = b = gtk_button_new_with_label( m_label.c_str() );

        gtk_signal_connect(GTK_OBJECT(b), "button_release_event",
                           GTK_SIGNAL_FUNC (fopm_menu_button), this );
        gtk_signal_connect(GTK_OBJECT(b), "button_press_event",
                           GTK_SIGNAL_FUNC (fopm_menu_button), this );

        g_object_set_data( G_OBJECT(m_button), GOBJ_MENU_K, (void*)m_menu );
        gtk_widget_show_all( b );

        m_image = b = GTK_WIDGET( gtk_image_new_from_stock(
                                      GTK_STOCK_GO_DOWN,
                                      GTK_ICON_SIZE_MENU ));
        gtk_signal_connect(GTK_OBJECT(b), "button_release_event",
                           GTK_SIGNAL_FUNC (fopm_menu_button), this );
        gtk_signal_connect(GTK_OBJECT(b), "button_press_event",
                           GTK_SIGNAL_FUNC (fopm_menu_button), this );
        g_object_set_data( G_OBJECT(m_image), GOBJ_MENU_K, (void*)m_menu );

        gtk_table_attach(GTK_TABLE(m_userwidget), GTK_WIDGET( m_button ),
                         0, 1, 0, 1,
                         GtkAttachOptions(GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                         GtkAttachOptions(GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                         0, 0 );
        gtk_table_attach(GTK_TABLE(m_userwidget), GTK_WIDGET( m_image ),
                         1, 2, 0, 1,
                         GtkAttachOptions(0),
                         GtkAttachOptions(0),
                         0, 0 );
        gtk_widget_show_all( m_userwidget );
    }
    
    
    GtkWidget* FerrisOptionMenu::getWidget()
    {
        ensureWidgetsCreated();

        return m_userwidget;
    }

    void
    FerrisOptionMenu::setVisible( bool v )
    {
        ensureWidgetsCreated();
        
        if( v ) gtk_widget_show_all( m_userwidget );
        else    gtk_widget_hide_all( m_userwidget );
    }

    bool
    FerrisOptionMenu::getVisible()
    {
        if( !m_userwidget )
            return false;

        gboolean v = false;
        g_object_get( G_OBJECT(m_button), "visible", &v, 0 );
        return v;
    }

    void
    FerrisOptionMenu::setMaxButtonTextLength( int v )
    {
        MaxButtonTextLength = v;
    }
    
    void
    FerrisOptionMenu::setMenu( GtkWidget* menu )
    {
        setMenu( GTK_MENU(menu) );
    }

    void forea_set_base_k( GtkWidget *widget, gpointer data)
    {
        g_object_set_data( G_OBJECT(widget), GOBJ_BASE_K, data );
    }

    void forea_hookup_activate( GtkWidget *widget, gpointer data )
    {
        g_signal_connect_data( G_OBJECT( widget ), "activate",
                               G_CALLBACK (fopm_menu_activate), data,
                               0, GConnectFlags(0));
    }
    

    
    void
    FerrisOptionMenu::setMenu( GtkMenu*   menu )
    {
        m_menu = GTK_WIDGET(menu);

        ensureWidgetsCreated();
        
        g_object_set_data( G_OBJECT(m_button), GOBJ_MENU_K, (void*)m_menu );
        g_object_set_data( G_OBJECT(m_image),  GOBJ_MENU_K, (void*)m_menu );
        gtk_container_foreach( GTK_CONTAINER(m_menu), 
                               GtkCallback(forea_set_base_k),
                               m_menu );
        gtk_container_foreach( GTK_CONTAINER(m_menu), 
                               GtkCallback(forea_hookup_activate),
                               this );

        /* Break up the menu into submenus if there are too many elements */
        GList* gl;
        gl = gtk_container_get_children( GTK_CONTAINER( m_menu ) );

        if( g_list_length(gl) > MaxMenuElementsPerContext )
        {
            typedef list<GtkMenuItem*> GtkMenuItemList_t;
            GtkMenuItemList_t menuItemList;
            GtkMenuItemList_t newTopLevelMenuItems;

            while( true )
            {
                for( int i=0; i<MaxMenuElementsPerContext && gl; ++i )
                {
                    GtkMenuItem* mi = GTK_MENU_ITEM( gl->data );
                
                    menuItemList.push_back( mi );
                    gtk_container_remove( GTK_CONTAINER( m_menu ), GTK_WIDGET(mi) );
                    gl = g_list_next( gl );
                }

                if( !menuItemList.empty() )
                {
                    int maxlen = MaxButtonTextLength > 0 ? MaxButtonTextLength : string::npos;
                    
                    fh_stringstream labelss;
                    labelss << tostr( menuItemList.front() ).substr(0,maxlen)
                            << " - "
                            << tostr( menuItemList.back() ).substr(0,maxlen);
                    GtkMenuItem* w;
                    w = GTK_MENU_ITEM(gtk_menu_item_new_with_label( tostr(labelss).c_str() ));
                    newTopLevelMenuItems.push_back( w );

                    GtkWidget* submenu = gtk_menu_new();
                    gtk_menu_item_set_submenu( GTK_MENU_ITEM(w), submenu );
                    
                    for( GtkMenuItemList_t::iterator iter = menuItemList.begin();
                         iter != menuItemList.end(); ++iter )
                    {
                        gtk_menu_shell_append( GTK_MENU_SHELL(submenu), GTK_WIDGET(*iter) );
                    }
                    menuItemList.clear();
                }

                if( !gl )
                    break;
            }

            for( GtkMenuItemList_t::iterator iter = newTopLevelMenuItems.begin();
                 iter != newTopLevelMenuItems.end(); ++iter )
            {
                gtk_menu_shell_append( GTK_MENU_SHELL(m_menu), GTK_WIDGET(*iter) );
            }
        }
    }
    
    GtkMenu* FerrisOptionMenu::getMenu()
    {
        return GTK_MENU(m_menu);
    }
    

    std::string
    FerrisOptionMenu::getString()
    {
        return m_label;

        // NOTE: the gtk button may be showing a substr() of what the value is
//         gchararray l;
//         g_object_get( G_OBJECT(m_button), "label", &l, 0 );
//         return l;
    }
    
    

    std::string tostr( FerrisOptionMenu* opm )
    {
        return opm->getString();
    }
    
};
