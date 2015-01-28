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

    $Id: libcreationfulltextindex_factory.cpp,v 1.2 2010/09/24 21:31:26 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <FerrisCreationPlugin.hh>

using namespace std;

namespace Ferris
{
    static bool r =
    RegisterCreationModule(
        "libcreationfulltextindex.so",
        "fulltextindex",
        "	<elementType name=\"fulltextindex\">\n"
        "		<elementType name=\"stemmer\" default=\"J_B_LOVINS_68\">\n"
        "			<dataTypeRef name=\"StemmerListT\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"lexicon-class\" default=\"Uncompressed\">\n"
        "			<dataTypeRef name=\"LexiconClassListT\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"case-sensitive\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"drop-stop-words\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"inverted-skiplist-max-size\" default=\"4096\" "
        "               minInclusive=\"5\" maxInclusive=\"10000\" "
        "               step=\"100\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"document-number-gap-code\" default=\"Golomb\" >\n"
        "			<dataTypeRef name=\"DocumentNumberGapCodeListT\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"frequency-of-term-in-document-code\" default=\"Gamma\" >\n"
        "			<dataTypeRef name=\"FrequencyOfTermInDocumentCodeListT\"/>\n"
        "		</elementType>\n"
        "	</elementType>\n"
        ,
        false
        ,
//        "        <enumeration value=\"Interpolative\"/>"  

        "<simpleType name=\"DocumentNumberGapCodeT\">"  
        "    <restriction base=\"string\">"  
        "        <enumeration value=\"Golomb\"/>"  
        "        <enumeration value=\"Interpolative\"/>"  
        "        <enumeration value=\"Gamma\"/>"  
        "        <enumeration value=\"Delta\"/>"  
        "        <enumeration value=\"None\"/>"  
        "    </restriction>"  
        "</simpleType>"  
        "<simpleType name=\"DocumentNumberGapCodeListT\">"  
        "    <list itemType=\"DocumentNumberGapCodeT\"/>"  
        "    <restriction>"  
        "        <length value=\"1\"/>"  
        "    </restriction>"  
        "</simpleType>"  
        "<simpleType name=\"FrequencyOfTermInDocumentCodeT\">"  
        "    <restriction base=\"string\">"  
        "        <enumeration value=\"Gamma\"/>"  
        "        <enumeration value=\"Delta\"/>"  
        "        <enumeration value=\"None\"/>"  
        "    </restriction>"  
        "</simpleType>"  
        "<simpleType name=\"FrequencyOfTermInDocumentCodeListT\">"  
        "    <list itemType=\"FrequencyOfTermInDocumentCodeT\"/>"  
        "    <restriction>"  
        "        <length value=\"1\"/>"  
        "    </restriction>"  
        "</simpleType>"  
        );
};
