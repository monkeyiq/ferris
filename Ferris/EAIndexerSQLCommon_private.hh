/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2004 Ben Martin

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

    $Id: EAIndexerSQLCommon_private.hh,v 1.3 2010/09/24 21:30:31 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_SQL_COMMON_H_
#define _ALREADY_INCLUDED_FERRIS_SQL_COMMON_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <string>

namespace Ferris
{
    FERRISEXP_API extern const char* CFG_EAIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K;

#define  SQL_URL_LENGTH_INT      16384
#define  SQL_MIMETYPE_LENGTH_INT   512
#define  SQL_ATTRNAME_LENGTH_INT   512
    
#define  SQL_URL_LENGTH_STR      "16384"
#define  SQL_MIMETYPE_LENGTH_STR   "512"
#define  SQL_ATTRNAME_LENGTH_STR   "512"
    FERRISEXP_API std::string GET_SQL_STRVAL_LENGTH_STR();
    
    FERRISEXP_API std::string stripNullCharacters( const std::string& s );
    FERRISEXP_API std::string EANameToSQLColumnName( const std::string& s );
    FERRISEXP_API std::string toSQLTimeString( time_t t );
    FERRISEXP_API time_t      fromSQLTimeString( const std::string& s );
    

};
#endif

