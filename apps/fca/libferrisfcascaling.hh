/******************************************************************************
*******************************************************************************
*******************************************************************************

    create a scale from the URLs in the EA index
    Copyright (C) 2005 Ben Martin

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

    $Id: libferrisfcascaling.hh,v 1.4 2010/09/24 21:31:11 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_FCA_SCALING_H_
#define _ALREADY_INCLUDED_FERRIS_FCA_SCALING_H_

#include <Ferris/Ferris.hh>
#include <Ferris/EAIndexer.hh>
#include <Ferris/EAQuery.hh>

#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;
using namespace Ferris::EAIndex;

#include <pqxx/connection>
#include <pqxx/tablewriter>
#include <pqxx/transaction>
#include <pqxx/nontransaction>
#include <pqxx/tablereader>
#include <pqxx/tablewriter>
#include <pqxx/result>
using namespace PGSTD;
using namespace pqxx;

namespace Ferris
{
    extern const char* CFG_IDX_USER_K;
    extern const char* CFG_IDX_HOST_K;
    extern const char* CFG_IDX_PORT_K;
    extern const char* CFG_IDX_DBNAME_K;
    extern const char* CFG_IDX_USER_DEF;
    extern const char* CFG_IDX_HOST_DEF;
    extern const char* CFG_IDX_PORT_DEF;
    extern const char* CFG_IDX_DBNAME_DEF;

    
    std::string cleanAttributeName( const std::string& const_s );
    EAIndex::fh_idx getEAIndex( const char* findexPath_CSTR );

    stringmap_t& readAttributes( stringmap_t& ret, poptContext& optCon, const std::string& treename = "" );

    struct versionltstr
    {
        inline bool operator()(const string& s1, const string& s2) const
            {
                return strverscmp(s1.c_str(), s2.c_str()) < 0;
            }
    };


    std::string guessLookupTableName( work& trans, std::string attrname );

    std::string formalTimeValueForFormalAttribute( std::string v );
};



#endif


