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

    $Id: libferrispostgresqlshared.hh,v 1.2 2006/12/07 06:49:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_SHARED_QTSQL_H_
#define _ALREADY_INCLUDED_FERRIS_SHARED_QTSQL_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <TypeDecl.hh>

namespace Ferris
{
    FERRISEXP_EXPORT userpass_t getQtSQLUserPass( const std::string& server );
    FERRISEXP_EXPORT void setQtSQLUserPass( const std::string& server,
                                            const std::string& user, const std::string& pass );
};

#endif
