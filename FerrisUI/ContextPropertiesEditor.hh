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

    $Id: ContextPropertiesEditor.hh,v 1.4 2010/09/24 21:31:04 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRISUI_CONTEXT_PROPERTIES_EDITOR_H_
#define _ALREADY_INCLUDED_FERRISUI_CONTEXT_PROPERTIES_EDITOR_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <glib.h>
#include <gtk/gtk.h>

#include <string>
#include <vector>

#include <Ferris/Ferris.hh>
#include <Ferris/Runner.hh>
#include <FerrisUI/GtkFerris.hh>
#include <FerrisUI/SubprocessButtonAndLabel.hh>

using namespace std;
using namespace Ferris;
using namespace FerrisUI;

namespace FerrisUI
{

    class FERRISEXP_API ContextPropertiesEditor
        :
        public sigc::trackable
    {
        typedef ContextPropertiesEditor _Self;
        
        /**
         * Context being edited
         */
        fh_context m_context;

        /**
         * Main top level widget that is returned to the user in getWidget()
         */
        GtkWidget* m_userWidget;

        /**
         * Signal handlers should ignore updates to data if this is true. It allows
         * setContext() to setup the GUI and have the event handlers no try to
         * change the context's data from the data that the widgets have just been
         * set to in setContext()
         */
        bool m_manuallySetting;

//        GtkWidget* m_menu;

        GtkWidget* m_table;
        GtkLabel* m_url;
        GtkLabel* m_mimetype;
        GtkImage* m_mimetypeIcon;
        GtkImage* m_explicitIcon;
        GtkImage* m_thumbIcon;
        
        GtkLabel* m_owner;
        FerrisOptionMenu* m_group;
        GtkWidget* m_owner_read;
        GtkWidget* m_owner_write;
        GtkWidget* m_owner_exec;

        GtkWidget* m_group_read;
        GtkWidget* m_group_write;
        GtkWidget* m_group_exec;

        GtkWidget* m_other_read;
        GtkWidget* m_other_write;
        GtkWidget* m_other_exec;
        
        GtkLabel* m_protection_ls_text;
        GtkLabel* m_protection_ls_octal;
        
        GtkEntry* m_atime;
        GtkImage* m_atime_is_valid;
        GtkLabel* m_ctime;
        GtkEntry* m_mtime;
        GtkImage* m_mtime_is_valid;

        fh_SubprocessButtonAndLabel m_du_sh;
//         fh_runner  m_du_sh_runner;
//         fh_istream m_du_sh_fromChildss;
//         GtkButton* m_recalc_du_sh;
//         GtkLabel*  m_du_sh;
        
        GtkWidget* makeProtectionCheckBox();
        /**
         * If m_userWidget==0 then this method will create the gtk widgets, otherwise
         * it does nothing
         */
        void ensureWidgetsCreated();

        /**
         * Read the data from the context and update the widgets for protection
         * (used for when the r/w/x checkboxes are toggled
         */
        void updateProtectionLabels();

        /**
         * Modify the top menu based on what is selected
         */
        void updateMenuShell();

        /**
         * Updates the group widget from the data of the m_context
         */
        void updateGroupWidget();

        /**
         * if true then the url is shown with the rest of the data
         */
        bool m_showURL;
        
    public:

        ContextPropertiesEditor();

        void setContext( fh_context c );
        fh_context getContext();

        GtkWidget* getWidget();

        void setShowURL( bool v );


        
        void mode_toggled( GtkToggleButton *togglebutton );

        string getTimeEAName( GtkEntry* entry );
        void updateTime( GtkEntry *entry,
                         const std::string& eaname,
                         const std::string& val );
        void time_activated( GtkEntry *entry );
        void time_changed( GtkEditable* editable );

        bool OnChangeGroup( FerrisOptionMenu* fopm, string oldLabel, string newLabel );

        void mimeicon_drag_data_received(
            GtkWidget          *widget,
            GdkDragContext     *context,
            gint                x,
            gint                y,
            GtkSelectionData   *data,
            guint               info,
            guint               time );
        

//         void recalc_du_sh( GtkButton *button );
//         gboolean recalc_du_sh_complete( GIOChannel *source, GIOCondition condition );
//         gint du_sh_button_release( GtkWidget *widget, GdkEvent *event );

        fh_runner getRunner_du_sh( fh_SubprocessButtonAndLabel sp, fh_runner );
        
    };
};
#endif
