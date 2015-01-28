/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2010 Ben Martin

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

    $Id: EAIndexer.hh,v 1.11 2009/01/09 21:30:11 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "TimeDelayedAction.hh"
#include <Ferris/Ferris.hh>

namespace Ferris
{
    TimeDelayedAction::TimeDelayedAction()
        : m_timeoutSeconds( 10 )
        , m_timerID( 0 )
        , m_maximalTimerAgeSeconds( 0 )
    {
    }

    TimeDelayedAction::~TimeDelayedAction()
    {
        stopTimer();
    }
    
    void TimeDelayedAction::setTimeoutSeconds( int v )
    {
        m_timeoutSeconds = v;
    }
    
    int
    TimeDelayedAction::getTimeoutSeconds()
    {
        return m_timeoutSeconds;
    }

    static gint timer_cb( gpointer user_data )
    {
        TimeDelayedAction* x = (TimeDelayedAction*)user_data;
        x->callFired();
        return 0;
    }

    void
    TimeDelayedAction::callFired()
    {
        m_firstResetTimerEpoch = 0;
        fired();
    }
    
    
    void
    TimeDelayedAction::setMaximalTimerAgeSeconds( int v )
    {
        m_maximalTimerAgeSeconds = v;
    }
    
    int
    TimeDelayedAction::getMaximalTimerAgeSeconds()
    {
        return m_maximalTimerAgeSeconds;
    }
    
    
    void
    TimeDelayedAction::resetTimer()
    {
        stopTimer();

        if( !m_firstResetTimerEpoch )
            m_firstResetTimerEpoch = Time::getTime();
        else
        {
            time_t now = Time::getTime();
            if( m_firstResetTimerEpoch + m_maximalTimerAgeSeconds < now )
            {
                callFired();
                return;
            }
        }
                
        m_timerID = g_timeout_add_seconds( m_timeoutSeconds,
                                           GSourceFunc(timer_cb), this );
    }

    void
    TimeDelayedAction::stopTimer()
    {
        if( m_timerID )
            g_source_remove( m_timerID );
        m_timerID = 0;
    }
    
    
    void
    TimeDelayedAction::fired()
    {
        getFiredSignal().emit();
    }
    
    TimeDelayedAction::FiredSignal_t&
    TimeDelayedAction::getFiredSignal()
    {
        return m_FiredSignal;
    }
    
};
