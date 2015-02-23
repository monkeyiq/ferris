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

    $Id: SubprocessButtonAndLabel.cpp,v 1.2 2010/09/24 21:31:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <SubprocessButtonAndLabel.hh>
#include <Ferris/FerrisBoost.hh>

using namespace std;

namespace Ferris
{
    SubprocessButtonAndLabel::SubprocessButtonAndLabel( const std::string& buttonLabel )
        :
        m_runner( 0 ),
        m_state( STATE_INITIAL ),
        m_buttonLabel( buttonLabel ),
        m_initialLabel( "..." ),
        m_workingLabel( "Working..." ),
        m_killedLabel( "Killed." ),
        m_widget( 0 ),
        m_label( 0 ),
        m_button( 0 ),
        m_eventSource( 0 )
    {
    }
    
    SubprocessButtonAndLabel::SubprocessButtonAndLabel( fh_runner r, const std::string& buttonLabel )
        :
        m_runner( r ),
        m_state( STATE_INITIAL ),
        m_buttonLabel( buttonLabel ),
        m_initialLabel( "..." ),
        m_workingLabel( "Working..." ),
        m_killedLabel( "Killed." ),
        m_widget( 0 ),
        m_label( 0 ),
        m_button( 0 ),
        m_eventSource( 0 )
    {
    }
        
    void
    SubprocessButtonAndLabel::setInitialLabel( const std::string& s )
    {
        m_initialLabel = s;
    }
    
    void
    SubprocessButtonAndLabel::setWorkingLabel( const std::string& s )
    {
        m_workingLabel = s;
    }
    
    void
    SubprocessButtonAndLabel::setKilledLabel( const std::string& s )
    {
        m_killedLabel = s;
    }
    
    void
    SubprocessButtonAndLabel::resetToInitialState()
    {
        m_state = STATE_INITIAL;
        gtk_label_set_label( m_label, m_initialLabel.c_str() );
    }

    void
    SubprocessButtonAndLabel::setToFinalState( const std::string& s )
    {
        m_state = STATE_COMPLETE;
        gtk_label_set_label( m_label, s.c_str() );
    }
    
    
    GtkWidget*
    SubprocessButtonAndLabel::getWidget()
    {
        m_widget = gtk_table_new ( 1, 2, false );

        int r = 0;
        gtk_table_attach_defaults(GTK_TABLE(m_widget), getButton(), 0, 1, r, r+1 );
        gtk_table_attach_defaults(GTK_TABLE(m_widget), getLabel(),  1, 2, r, r+1 );

        return m_widget;
    }

    GtkWidget*
    SubprocessButtonAndLabel::getLabel()
    {
        if( !m_label )
        {
            m_label = GTK_LABEL(gtk_label_new( m_initialLabel.c_str() ));
        }
        return GTK_WIDGET(m_label);
    }

    void
    SubprocessButtonAndLabel::killChild()
    {
        if( m_runner )
            m_runner->killChild();
        gtk_label_set_label( m_label, m_killedLabel.c_str() );
        m_state = STATE_INITIAL;
    }

    SubprocessButtonAndLabel::UpdateRunnerSig_t&
    SubprocessButtonAndLabel::getUpdateRunnerSig()
    {
        return UpdateRunnerSig;
    }

    void
    SubprocessButtonAndLabel::setReplaceRegex( const std::string& s )
    {
        m_replaceRegex = s;
    }
    
    void
    SubprocessButtonAndLabel::setReplaceRegexFormat( const std::string& s )
    {
        m_replaceRegexFormat = s;
    }
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    static gboolean
    complete_cb( GIOChannel *source,
                 GIOCondition condition,
                 gpointer user_data )
    {
        SubprocessButtonAndLabel* obj = (SubprocessButtonAndLabel*)user_data;
        return obj->complete( source, condition );
    }

    gboolean
    SubprocessButtonAndLabel::complete( GIOChannel *source,
                                        GIOCondition condition )
    {
        string s;

        if( !m_replaceRegex.empty() )
        {
            fh_stringstream ss;
            std::copy( std::istreambuf_iterator<char>(m_fromChildss),
                       std::istreambuf_iterator<char>(),
                       std::ostreambuf_iterator<char>(ss));

            cerr << "m_replaceRegex:" << m_replaceRegex << endl;
            cerr << "m_replaceRegexFormat:" << m_replaceRegexFormat << endl;
            
            boost::regex rx = toregex( m_replaceRegex );
            s = regex_replace( tostr(ss), rx, m_replaceRegexFormat,
                               boost::match_default | boost::format_first_only );
//                               boost::format_first_only );
//                               boost::format_no_copy | boost::format_first_only );
        }
        else
        {
            getline( m_fromChildss, s );
            LG_GTKFERRIS_D << "SubprocessButtonAndLabel::complete() s:" << s << endl;

            //
            // Drain input
            // 
            fh_stringstream ss;
            std::copy( std::istreambuf_iterator<char>(m_fromChildss),
                       std::istreambuf_iterator<char>(),
                       std::ostreambuf_iterator<char>(ss));
        }
        
        gtk_label_set_label( m_label, s.c_str() );
        m_state = STATE_COMPLETE;
        
        return 0; // dont call again
    }
    
    
    static void clicked_cb( GtkButton *button, gpointer user_data )
    {
        SubprocessButtonAndLabel* obj = (SubprocessButtonAndLabel*)user_data;
        obj->clicked( button );
    }

    void
    SubprocessButtonAndLabel::clicked( GtkButton *button )
    {
        if( m_state == STATE_RUNNING )
        {
            killChild();
            return;
        }
        fh_stringstream ss;

        if( m_eventSource )
        {
             g_source_remove( m_eventSource );
             m_eventSource = 0;
        }

        m_runner = getUpdateRunnerSig().emit( this, m_runner );
        if( !m_runner )
        {
            gtk_label_set_label( m_label, "internal error" );
            m_state = STATE_ERROR;
            return;
        }
        m_runner->setSpawnFlags(
            GSpawnFlags(
                G_SPAWN_SEARCH_PATH |
//                G_SPAWN_FILE_AND_ARGV_ZERO |
                G_SPAWN_STDERR_TO_DEV_NULL |
                m_runner->getSpawnFlags()));
        m_runner->Run();
        m_fromChildss = m_runner->getStdOut();
        LG_GTKFERRIS_D << "SubprocessButtonAndLabel::running proc" << endl;

        //
        // We need to attach a callback for when data is available from the
        // child.
        //
        guint result;
        GIOChannel* channel;
        GIOCondition cond = GIOCondition(G_IO_IN | G_IO_ERR | G_IO_PRI);

        channel = g_io_channel_unix_new( m_runner->getStdOutFd() );
        m_eventSource = g_io_add_watch( channel, cond, complete_cb, this );
        g_io_channel_unref (channel);
        
        gtk_label_set_label( m_label, m_workingLabel.c_str() );
        m_state = STATE_RUNNING;
    }
    
    static gint button_release_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
    {
        SubprocessButtonAndLabel* obj = (SubprocessButtonAndLabel*)user_data;
        obj->button_release( widget, event );
    }
    
    gint
    SubprocessButtonAndLabel::button_release(GtkWidget *widget, GdkEvent *event )
    {
        GdkEventButton *event_button;

        g_return_val_if_fail (widget != NULL, FALSE);
        g_return_val_if_fail (event != NULL, FALSE);
        
        if (event->type == GDK_BUTTON_PRESS)
        {
        }
        if( event->type == GDK_BUTTON_RELEASE )
        {
            event_button = (GdkEventButton *) event;

            if (event_button->button == 3 && m_state == STATE_RUNNING )
            {
                killChild();
            }
        }
        return FALSE;
    }
    
    
    
    GtkWidget*
    SubprocessButtonAndLabel::getButton()
    {
        if( !m_button )
        {
            GtkWidget* w = gtk_button_new_with_label( m_buttonLabel.c_str() );
            m_button = GTK_BUTTON( w );
            gtk_signal_connect(GTK_OBJECT (w), "clicked",
                               GTK_SIGNAL_FUNC(clicked_cb), this );
            gtk_signal_connect(GTK_OBJECT(w), "button_release_event",
                               GTK_SIGNAL_FUNC (button_release_cb), this );
            gtk_widget_add_events( GTK_WIDGET( w ),
                                   GdkEventMask( GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK ));
            
        }
        
        return GTK_WIDGET(m_button);
    }
    
    
        
    
};
