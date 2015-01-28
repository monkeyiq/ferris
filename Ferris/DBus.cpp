/******************************************************************************
*******************************************************************************
*******************************************************************************

    DBUS private support functions.  

    Copyright (C) 2007 Ben Martin

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

    $Id: DBus.cpp,v 1.2 2010/09/24 21:30:28 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/DBus_private.hh>
#include <FerrisException.hh>

using namespace std;

namespace Ferris
{
    namespace DBus
    {
        DBusConnection* getSessionBus()
        {
            DBusError err;
            DBusConnection *conn;
            dbus_error_init (&err);
            conn = dbus_bus_get (DBUS_BUS_SESSION, &err);
            if (!conn)
            {
                LG_DBUS_W << "DBUS: Can not get session bus" << endl;
                stringstream ss;
                ss << "Failed to get session bus. error:" << err.name
                   << " message:" << err.message << endl;
                Throw_DBusConnectionException( ss.str(), 0 );
            }
            return conn;
        }
        
        void
        Signal::maybe_destroy_msg()
        {
            if( m_msg )
            {
                dbus_message_unref (m_msg);
                m_msg = 0;
            }
        }
        
        Signal::Signal( string& path, string &interface, string& name )
            :
            m_msg( 0 ),
            m_path( path ),
            m_interface( interface ),
            m_name( name )
        {
            m_msg = dbus_message_new_signal(
                path.c_str(), interface.c_str(), name.c_str() );
        }
        Signal::Signal( const char* path, const char* interface, const char* name )
            :
            m_msg( 0 ),
            m_path( path ),
            m_interface( interface ),
            m_name( name )
        {
            m_msg = dbus_message_new_signal( path, interface, name );
        }
        Signal::~Signal()
        {
            maybe_destroy_msg();
        }

        void
        Signal::clear()
        {
            maybe_destroy_msg();
            m_msg = dbus_message_new_signal(
                m_path.c_str(), m_interface.c_str(), m_name.c_str() );
        }
        
        
        void
        Signal::send( DBusConnection *conn )
        {
            if (!dbus_connection_send( conn, m_msg, NULL ))
            {
                LG_DBUS_W << "DBUS: error sending message" << endl;
                fprintf (stderr, "error sending message\n");
            }
        }
        
        
        void
        Signal::append( const string& s )
        {
            const char* str1 = s.c_str();
            dbus_message_append_args( m_msg,
                                      DBUS_TYPE_STRING, &str1,
                                      DBUS_TYPE_INVALID  );
        }

        /************************************************************/
        /************************************************************/
        /************************************************************/

        void dbus_get( DBusMessage *message, std::string& ret1, std::string& ret2 )
        {
            const char* str1 = 0;
            const char* str2 = 0;
            dbus_bool_t rc = dbus_message_get_args( message, 0,
                                                    DBUS_TYPE_STRING, &str1,
                                                    DBUS_TYPE_STRING, &str2,
                                                    DBUS_TYPE_INVALID  );
            ret1 = str1;
            ret2 = str2;
        }

        void dbus_get( DBusMessage *message, std::string& ret1, std::string& ret2, std::string& ret3 )
        {
            const char* str1 = 0;
            const char* str2 = 0;
            const char* str3 = 0;
            dbus_bool_t rc = dbus_message_get_args( message, 0,
                                                    DBUS_TYPE_STRING, &str1,
                                                    DBUS_TYPE_STRING, &str2,
                                                    DBUS_TYPE_STRING, &str3,
                                                    DBUS_TYPE_INVALID  );
            ret1 = str1;
            ret2 = str2;
            ret3 = str3;
        }
        
    };
};
