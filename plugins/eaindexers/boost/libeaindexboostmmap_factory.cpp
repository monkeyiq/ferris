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

    $Id: libeaindexboostmmap_factory.cpp,v 1.1 2009/01/31 11:21:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

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
            "libeaindexboostmmap.so",
            "eaindexboostmmap",
            "	<elementType name=\"eaindexboostmmap\">\n"
            "		<elementType name=\"attributes-not-to-index\" default=\""
            + GET_EAINDEX_EANAMES_NOT_TO_INDEX_DEFAULT() + "\">\n"
            "			<dataTypeRef name=\"AttributeNameListT\"/>\n"
            "		</elementType>\n"
            "		<elementType name=\"attributes-not-to-index-regex\" >\n"
            "			<dataTypeRef name=\"StringListT\"/>\n"
            "           <default>" + GET_EAINDEX_EANAMES_REGEX_IGNORE_DEFAULT(1) + "</default>\n"
            "		</elementType>\n"
            "		<elementType name=\"attributes-to-index-regex\" "
            "          default=\"" + GET_EAINDEX_EANAMES_TO_INDEX_REGEX_DEFAULT() + "\" >\n"
            "			<dataTypeRef name=\"string\"/>\n"
            "		</elementType>\n"
            "		<elementType name=\"max-value-size-to-index\" default=\""
            + GET_EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT() + "\">\n"
            "			<dataTypeRef name=\"int\"/>\n"
            "		</elementType>\n"
            "	</elementType>\n"
            ,
            false
            ,
            ""
            );

        RegisterEAIndexPluginAlias( "libeaindexboostmmap.so", "boostmmap" );

    }
};
