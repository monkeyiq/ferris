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

    $Id: EAIndexerSQLCommon.cpp,v 1.7 2010/09/24 21:30:31 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "EAIndexerSQLCommon_private.hh"
#include "General.hh"
#include "EAIndexer_private.hh"
#include "Ferris/FerrisBoost.hh"

using namespace std;

namespace Ferris
{
    const char* CFG_EAIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K
    = "cfg-odbcidx-extra-columns-to-inline-in-docmap";
    
    string
    stripNullCharacters( const std::string& s )
    {
        if( s.find( '\0' ) == string::npos )
            return s;
        
        stringstream ss;
        remove_copy_if( s.begin(), s.end(),
                        ostreambuf_iterator<char>(ss),
                        bind2nd(equal_to<char>(), '\0') );
        return ss.str();
    }

    string
    EANameToSQLColumnName( const std::string& s )
    {
        static const boost::regex rx = toregex( "[- ,/\\@&#:.+'()]" );
//        string ret = regex_replace( s, rx, "_", boost::match_default | boost::format_all );
        string ret = replaceg( s, rx, "_" );
        return ret;
        
        return
            Util::replace_all(
                Util::replace_all(
                    Util::replace_all(
                        Util::replace_all(
                            Util::replace_all(
                                Util::replace_all(
                                    Util::replace_all(
                                        Util::replace_all(
                                            Util::replace_all(
                                                Util::replace_all(
                                                    Util::replace_all( s,
                                                                       ' ', '_' ),
                                                    ',', '_' ),
                                                '-', '_' ),
                                            '/', '_' ),
                                        '\\', '_' ),
                                    "@", "_at_" ),
                                '&', '_' ),
                            '#', '_' ),
                        ' ', '_' ),
                    ':', '_' ),
                '.', '_' );
    }

    std::string toSQLTimeString( time_t t )
    {
        return Time::toTimeString( t, "%Y-%m-%d %H:%M:%S" );
    }

    time_t fromSQLTimeString( const std::string& s_const )
    {
        string s = s_const;
        if( string::npos != s.rfind('.') )
            s = s.substr( 0, s.rfind('.') );
        
        struct tm tm = Time::ParseTimeString( s, "%Y-%m-%d %H:%M:%S", false );
        time_t tt = mktime( &tm );
        return tt;
    }
    
    std::string GET_SQL_STRVAL_LENGTH_STR()
    {
        return EAIndex::GET_EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT();
    }
};
