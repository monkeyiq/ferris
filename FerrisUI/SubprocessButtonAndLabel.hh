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

    $Id: SubprocessButtonAndLabel.hh,v 1.3 2010/09/24 21:31:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_GTK_FERRIS_SUBPROCBUTTON_H_
#define _ALREADY_INCLUDED_GTK_FERRIS_SUBPROCBUTTON_H_

#include <FerrisUI/GtkFerris.hh>

namespace Ferris
{
    class SubprocessButtonAndLabel;
    FERRIS_SMARTPTR( SubprocessButtonAndLabel, fh_SubprocessButtonAndLabel );

    class FERRISEXP_API SubprocessButtonAndLabel
        :
        public Handlable
    {
        fh_runner m_runner;
        fh_istream m_fromChildss;
        guint m_eventSource;

        std::string m_replaceRegex;
        std::string m_replaceRegexFormat;
        std::string m_buttonLabel;
        std::string m_initialLabel;
        std::string m_workingLabel;
        std::string m_killedLabel;
        GtkWidget* m_widget;
        GtkLabel*  m_label;
        GtkButton* m_button;

        enum
        {
            STATE_INITIAL = 1,
            STATE_RUNNING = 2,
            STATE_COMPLETE = 3,
            STATE_ERROR = 4
        };
        int m_state;

        void killChild();

        
    public:
        SubprocessButtonAndLabel( const std::string& buttonLabel );
        SubprocessButtonAndLabel( fh_runner r, const std::string& buttonLabel );
        void setReplaceRegex( const std::string& s );
        void setReplaceRegexFormat( const std::string& s );
        void setInitialLabel( const std::string& s );
        void setWorkingLabel( const std::string& s );
        void setKilledLabel( const std::string& s );
        void resetToInitialState();
        void setToFinalState( const std::string& s );
        GtkWidget* getWidget();
        GtkWidget* getLabel();
        GtkWidget* getButton();

        typedef sigc::signal< fh_runner (
            fh_SubprocessButtonAndLabel,
            fh_runner ) > UpdateRunnerSig_t;
        UpdateRunnerSig_t& getUpdateRunnerSig();
        
         

        /****************************************/
        /****************************************/
        /**  Dont use the below  ****************/
        /****************************************/
        /****************************************/
        
        gboolean complete( GIOChannel *source, GIOCondition condition );
        void clicked( GtkButton *button );
        gint button_release(GtkWidget *widget, GdkEvent *event );
    private:
        UpdateRunnerSig_t UpdateRunnerSig;

    };

};
#endif
