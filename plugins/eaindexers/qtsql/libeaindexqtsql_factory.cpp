/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2004 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: libeaindexqtsql_factory.cpp,v 1.2 2008/07/02 21:30:13 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Ferris/EAIndexerMetaInterface.hh>
#include <Ferris/FerrisCreationPlugin.hh>

#include <Ferris/EAIndexer.hh>
#include <Ferris/FactoriesCreationCommon_private.hh>

using namespace Ferris;
using namespace Ferris::EAIndex;

extern "C"
{
    FERRISEXP_API void SetupPlugin()
    {
        RegisterEAIndexPlugin(
            "libeaindexqtsql.so",
            "eaindexqtsql",
            "	<elementType name=\"eaindexqtsql\">\n"
            "		<elementType name=\"attributes-not-to-index\" default=\""
            + EAIndex::GET_EAINDEX_EANAMES_NOT_TO_INDEX_DEFAULT() + "\">\n"
            "			<dataTypeRef name=\"AttributeNameListT\"/>\n"
            "		</elementType>\n"
            "		<elementType name=\"attributes-not-to-index-regex\" >\n"
            "			<dataTypeRef name=\"StringListT\"/>\n"
            "           <default>" + EAIndex::GET_EAINDEX_EANAMES_REGEX_IGNORE_DEFAULT(1) + "</default>\n"
            "		</elementType>\n"
            "		<elementType name=\"max-value-size-to-index\" default=\""
            + EAIndex::GET_EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT() + "\">\n"
            "			<dataTypeRef name=\"int\"/>\n"
            "		</elementType>\n"
            "		<elementType name=\"qdriver\" default=\"QSQLITE\" >\n"
            "			<dataTypeRef name=\"string\"/>\n"
            "		</elementType>\n"
            "		<elementType name=\"dbname\" default=\"index.db\" >\n"
            "			<dataTypeRef name=\"string\"/>\n"
            "		</elementType>\n"
            "		<elementType name=\"optimize-for-dbtype\" default=\"\" >\n"
            "			<dataTypeRef name=\"OptimizeForEAIndexDBTypeT\"/>\n"
            "		</elementType>\n"
            "		<elementType name=\"user\" default=\"\" >\n"
            "			<dataTypeRef name=\"string\"/>\n"
            "		</elementType>\n"
            "		<elementType name=\"pass\" default=\"\" >\n"
            "			<dataTypeRef name=\"string\"/>\n"
            "		</elementType>\n"
            "		<elementType name=\"multiversion\" default=\"1\" >\n"
            "			<dataTypeRef name=\"bool\"/>\n"
            "		</elementType>\n"

            "		<elementType name=\"extra-columns-to-inline-in-docmap\" >\n"
            "			<dataTypeRef name=\"EditSQLColumnsT\"/>\n"
            "           <default>" CFG_QTSQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_DEFAULT "</default>\n"
            "		</elementType>\n"

            "	</elementType>\n"
            ,
            false
            ,
            "<simpleType name=\"OptimizeForEAIndexDBTypeT\">"  
            "    <restriction base=\"string\">"  
            "        <enumeration value=\"\"/>"  
            "        <enumeration value=\"MySQL\"/>"  
            "        <enumeration value=\"Postgresql\"/>"  
            "    </restriction>"  
            "</simpleType>"  
            );
        
    }
};
        
