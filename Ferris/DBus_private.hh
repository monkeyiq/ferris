/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2007 Ben Martin

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

    $Id: DBus_private.hh,v 1.3 2010/09/24 21:30:28 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_DBUS_PRIV_H_
#define _ALREADY_INCLUDED_FERRIS_DBUS_PRIV_H_

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <FerrisLoki/loki/LokiTypeInfo.h>
#include <string>
#include <iostream>
#include <sstream>

namespace Ferris
{
    namespace DBus
    {
        DBusConnection* getSessionBus();

        class Signal
        {
            DBusMessage* m_msg;
            std::string m_path;
            std::string m_interface;
            std::string m_name;

            void maybe_destroy_msg();
        
        public:
            Signal( std::string& path, std::string &interface, std::string& name );
            Signal( const char* path, const char* interface, const char* name );
            virtual ~Signal();
            void clear();
            void send( DBusConnection *conn );
            void append( const std::string& s );

            template < class T1 >
            void append( T1& arg1 )
                {
                    std::stringstream ss1;
                    ss1 << arg1;
                    std::string a1 = ss1.str();
                    append( (const std::string&)a1 );
                }
            template < class T >
            void push_back( T& v ) 
                {
                    return append( v );
                }
            void push_back( const std::string& v ) 
                {
                    return append( v );
                }
        };

        /************************************************************/
        /************************************************************/
        /************************************************************/

//         template <class T1, class T2, class T3>
//         void dbus_get( DBusMessage *message, T1&, T2&, T3& )
//         {
//             std::cerr << "Generic dbus_get(3) called. Programming error!" << std::endl;
//         }

        void dbus_get( DBusMessage *message, std::string& ret1, std::string& ret2 );
        void dbus_get( DBusMessage *message, std::string& ret1, std::string& ret2, std::string& ret3 );
        
    };
};
#endif
