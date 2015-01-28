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

    $Id: FerrisOptionMenu.hh,v 1.3 2010/09/24 21:31:05 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRISUI_OPTIONMENU_H_
#define _ALREADY_INCLUDED_FERRISUI_OPTIONMENU_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <glib.h>
#include <gtk/gtk.h>

#include <Ferris/Ferris.hh>
#include <FerrisUI/GtkFerris.hh>

namespace FerrisUI
{

    class FERRISEXP_API FerrisOptionMenu
    {
        GtkWidget* m_userwidget;
        GtkWidget* m_button;
        GtkWidget* m_image;
        GtkWidget* m_menu;
        
        std::string m_label;
        std::string m_iconpath;

        int MaxButtonTextLength;
        int MaxMenuElementsPerContext;
        
        void ensureWidgetsCreated();

        
    public:

        FerrisOptionMenu( const std::string& label,
                          const std::string& iconpath = GTK_STOCK_GO_DOWN );

        GtkWidget* getWidget();
        void setVisible( bool v );
        bool getVisible();

        void setMaxButtonTextLength( int v );

        void setLabel( const std::string& s );
        
        void setMenu( GtkWidget* menu );
        void setMenu( GtkMenu*   menu );
        GtkMenu* getMenu();

        std::string getString();

        void menu_activate( GtkMenuItem *menuitem );
        gint menu_button(GtkWidget *widget, GdkEvent *event );


        /**
         * Monitor the users desire to change the value of the optionmenu. Whenever
         * the value is about to be changed this signal is fired, allowing clients
         * to veto the update.
         *
         * If any signal handler returns true, then the change is not performed.
         *
         * Args are this, old label value, new label value (==getLabel() during handler)
         */
        typedef sigc::signal3< bool,
                               FerrisOptionMenu*,
                               std::string,
                               std::string > LabelChangeSig_t;
        LabelChangeSig_t& getLabelChangeSig()
            {
                return LabelChangeSig;
            }
    protected:
        LabelChangeSig_t LabelChangeSig;
        
    };

    FERRISEXP_API std::string tostr( FerrisOptionMenu* opm );
};
#endif
