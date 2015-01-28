/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2002 Ben Martin

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

    $Id: libcreationeaindexdb4tree_factory.cpp,v 1.4 2010/09/24 21:31:26 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <FerrisCreationPlugin.hh>
#include <Ferris/EAIndexer_private.hh>
#include <Ferris/FactoriesCreationCommon_private.hh>

using namespace std;

namespace Ferris
{
    static bool r =
    RegisterCreationModule(
        "libcreationeaindexdb4tree.so",
        "eaindexdb4tree",
        "	<elementType name=\"eaindexdb4tree\">\n"
        "		<elementType name=\"attribute-name-mapping\" "
        "           default=\"Uncompressed (db4 hash)\">\n"
        "			<dataTypeRef name=\"LexiconClassUncompressedFirstListT\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"forward-value-mapping\" "
        "           default=\"Uncompressed (db4 hash)\">\n"
        "			<dataTypeRef name=\"LexiconClassUncompressedFirstListT\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"reverse-value-mapping\" "
        "           default=\"Uncompressed (db4 hash)\">\n"
        "			<dataTypeRef name=\"ReverseLexiconClassUncompressedFirstListT\"/>\n"
        "		</elementType>\n"
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
        "	</elementType>\n"
        ,
        false
        ,
        ""
        );
};
