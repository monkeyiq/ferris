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

    $Id: common-ferris-out-of-proc-notification-deamon.cpp,v 1.4 2010/09/24 21:31:02 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "common-ferris-out-of-proc-notification-deamon.hh"

#include "Resolver_private.hh"

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

namespace Ferris
{
    namespace OProcMessage
    {
        const string FERRIS_OUT_OF_PROC_NOTIFICATION_DEAMON_ROOT = "~/.ferris/oprocnotify";

        string appendToServPath( const std::string& r )
        {
            string ret = r + "/incoming";
            ret = CleanupURL( ret );

            if( starts_with( ret, "file:" ) )
                ret = ret.substr( 5 );

            return ret;
        }
        
        string appendFromServPrefix( const std::string& r )
        {
            string ret = r + "/outgoing/";
            ret = CleanupURL( ret );

            if( starts_with( ret, "file:" ) )
                ret = ret.substr( 5 );

            return ret;
        }

        string appendStagePrefix( const std::string& r )
        {
            string ret = r + "/stage/";
            ret = CleanupURL( ret );

            if( starts_with( ret, "file:" ) )
                ret = ret.substr( 5 );

            return ret;
        }
        
        const char* KEY_COMMAND   = "command";
        const char* KEY_URL       = "url";
        const char* KEY_NAME      = "name";
        const char* KEY_OBAND_PID = "out-of-band-pid";
        const char* KEY_DATA = "data";
        const char* KEY_ESET = "eset";

        const char* COMMAND_CTX_CREATED = "context-created";
        const char* COMMAND_CTX_DELETED = "context-deleted";
        const char* COMMAND_CTX_CHANGED = "context-changed";
        const char* COMMAND_EA_CREATED  = "ea-created";
        const char* COMMAND_EA_DELETED  = "ea-deleted";
        const char* COMMAND_MEDALLION_UPDATED = "medallion-updated";
        const char* COMMAND_ETAGERE_NEW_EMBLEM = "etagere-new-emblem";
    };
};
