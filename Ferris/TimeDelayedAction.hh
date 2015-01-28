/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of libferris.

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

    $Id: ut_close_signal.cpp,v 1.3 2008/05/24 21:31:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_TIME_DELAYED_ACTION_H_
#define _ALREADY_INCLUDED_FERRIS_TIME_DELAYED_ACTION_H_

#include <Ferris/FerrisHandle.hh>
#include <Ferris/TypeDecl.hh>

namespace Ferris
{
    /**
     * In various cases, there is some work to be done which might be
     * better performed in a few seconds. For example, automatically
     * saving the user input. In this case you might like to reset the
     * timeout if the user starts typing again. On the other hand, you
     * also might like to set a maximal age to fire the signal even if
     * user input is happening. This last case is good for batch processing
     * where you might delay if new work keeps becoming available but
     * perform the work regardless of new material arriving if 5 minutes
     * have gone by since the first resetTimer() call.
     */
    class FERRISEXP_API TimeDelayedAction
        :
        public Handlable
    {
      public:
        TimeDelayedAction();
        virtual ~TimeDelayedAction();

        /**
         * Number of seconds to wait before firing the fired() method/signal
         */
        void setTimeoutSeconds( int v );
        int  getTimeoutSeconds();

        /*
         * If v > 0, then fire the signal even if resetTimer() was called
         * again before the timer expired. Good for batch processing. The
         * default is 0
         */
        void setMaximalTimerAgeSeconds( int v );
        int  getMaximalTimerAgeSeconds();
        
        /**
         * Set the timer to expire in getTimeoutSeconds() from now. If
         * there is already a timer waiting, remove that one first. ie,
         * this method keeps delaying the timer until it is not called
         * for getTimeoutSeconds()
         */
        void resetTimer();
        void stopTimer();

        /**
         * The timer has expired. Do something. The default implementation
         * fires the getFiredSignal.
         */
        virtual void fired();

        typedef sigc::signal0< void > FiredSignal_t;
        FiredSignal_t& getFiredSignal();

        /**
         * This method is private.
         */
        void callFired();
        
      private:
        FiredSignal_t m_FiredSignal;
        int m_timeoutSeconds;
        time_t m_firstResetTimerEpoch;
        int m_maximalTimerAgeSeconds;
        guint m_timerID;
        
    };
    FERRIS_SMARTPTR( TimeDelayedAction, fh_TimeDelayedAction );
};
#endif
