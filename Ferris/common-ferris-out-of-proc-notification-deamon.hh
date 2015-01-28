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

    $Id: common-ferris-out-of-proc-notification-deamon.hh,v 1.4 2010/09/24 21:31:02 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_COMMON_OPROC_DEAMON_H_
#define _ALREADY_INCLUDED_FERRIS_COMMON_OPROC_DEAMON_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/SignalStreams.hh>

#include <string>

namespace Ferris
{
    /**
     * write/readMessage() put extra data in the message, the Serv namespace
     * is used by the server to get at that extra data.
     *
     * This is mainly used by PluginOutOfProcNotificationEngine to communicate
     * with ferris-out-of-proc-notification-deamon
     */
    namespace OProcMessage
    {
        FERRISEXP_API extern const std::string FERRIS_OUT_OF_PROC_NOTIFICATION_DEAMON_ROOT;
        FERRISEXP_API std::string appendToServPath(
            const std::string& r = FERRIS_OUT_OF_PROC_NOTIFICATION_DEAMON_ROOT );
        FERRISEXP_API std::string appendFromServPrefix(
            const std::string& r = FERRIS_OUT_OF_PROC_NOTIFICATION_DEAMON_ROOT );
        FERRISEXP_API std::string appendStagePrefix( 
            const std::string& r = FERRIS_OUT_OF_PROC_NOTIFICATION_DEAMON_ROOT );
        
        

        FERRISEXP_API extern const char* KEY_COMMAND;
        FERRISEXP_API extern const char* KEY_URL;
        FERRISEXP_API extern const char* KEY_NAME;
        FERRISEXP_API extern const char* KEY_OBAND_PID;
        FERRISEXP_API extern const char* KEY_DATA;
        FERRISEXP_API extern const char* KEY_ESET;

        FERRISEXP_API extern const char* COMMAND_CTX_CREATED;
        FERRISEXP_API extern const char* COMMAND_CTX_DELETED;
        FERRISEXP_API extern const char* COMMAND_CTX_CHANGED;
        FERRISEXP_API extern const char* COMMAND_EA_CREATED;
        FERRISEXP_API extern const char* COMMAND_EA_DELETED;
        FERRISEXP_API extern const char* COMMAND_MEDALLION_UPDATED;
        FERRISEXP_API extern const char* COMMAND_ETAGERE_NEW_EMBLEM;
        
    };
};
#endif
