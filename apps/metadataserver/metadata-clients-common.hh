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

#ifndef _ALREADY_INCLUDED_FERRIS_METADATA_CLIENTS_COMMON_H_
#define _ALREADY_INCLUDED_FERRIS_METADATA_CLIENTS_COMMON_H_

#define NEW_QBUS_IMPL 1
#include "config.h"
#include <Ferris/Ferris.hh>
#include <Ferris/Ferris_private.hh>
#include <Ferris/MatchedEAGenerators.hh>
#include <Ferris/MetadataServer_private.hh>
#include <Ferris/FerrisQt_private.hh>

#include <popt.h>
#include <time.h>

#include <QtDBus>
#include <QDBusArgument>
#include <QCoreApplication>

namespace Ferris
{
    class ExitOnIdle : QObject
    {
        Q_OBJECT
        QCoreApplication* m_app;
        QTimer* m_timer;
    public:
        ExitOnIdle( QCoreApplication* app );
        ~ExitOnIdle();
        
    public slots:

        void resetIdleTimer();
        void exitApp();
    };


    void dbus_error( const QDBusMessage &message, const std::string& desc );
    void dbus_error( const QDBusMessage &message, std::stringstream& ss );
    
    
#define DBUS_MARSHALL_EXCEPTIONS                                        \
    catch( FerrisExceptionBase& e )                                     \
    {                                                                   \
        dbus_error( message, e.what() );                                \
    }                                                                   \
    catch( std::exception& e )                                          \
    {                                                                   \
        dbus_error( message, e.what() );                                \
    }                                                                   \
    catch( ... )                                                        \
    {                                                                   \
        dbus_error( message, "unknown catch all (...) exception!" );    \
    }
    
};

#endif
