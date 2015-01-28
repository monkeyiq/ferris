/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris internal metadata dbus stuff
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

    $Id: ferris-internal-metadata-worker.cpp,v 1.4 2008/04/27 21:30:11 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "metadata-clients-common.hh"

namespace Ferris
{
    ExitOnIdle::ExitOnIdle( QCoreApplication* app )
        :
        m_app(app)
    {
        m_timer = new QTimer(this);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(exitApp()));
        resetIdleTimer();
    }
    ExitOnIdle::~ExitOnIdle()
    {
        if(m_timer)
            m_timer->stop();
    }
        
    void
    ExitOnIdle::resetIdleTimer()
    {
        m_timer->stop();
        m_timer->start( 30 * 1000 );
    }
    void
    ExitOnIdle::exitApp()
    {
        std::cout << endl;
        std::cerr << "been idle too long, exiting..." << endl;
        m_app->exit(0);
    }

    //////////////////////////
    //////////////////////////
    //////////////////////////

    void dbus_error( const QDBusMessage &message, const std::string& desc )
    {
        QDBusMessage reply = message.createErrorReply( QDBusError::Failed, desc.c_str() );
        QDBusConnection::sessionBus().send( reply );
    }
    void dbus_error( const QDBusMessage &message, std::stringstream& ss )
    {
        dbus_error( message, ss.str() );
    }
    
    
};

#include "metadata-clients-common.moc"
