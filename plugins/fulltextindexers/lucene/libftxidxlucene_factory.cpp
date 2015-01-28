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

    $Id: libftxidxlucene_factory.cpp,v 1.2 2008/07/02 21:30:15 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/FullTextIndexerMetaInterface.hh>
#include <Ferris/FerrisCreationPlugin.hh>


        
extern "C"
{
    FERRISEXP_API void SetupPlugin()
    {
        Ferris::RegisterFulltextIndexPlugin(
            "libftxidxlucene.so",
            "fulltextindexlucene",
            "	<elementType name=\"fulltextindexlucene\">\n"
            "		<elementType name=\"stemmer\" default=\"Porter\">\n"
            "			<dataTypeRef name=\"LuceneStemmerListT\"/>\n"
            "		</elementType>\n"
//         "		<elementType name=\"lexicon-class\" default=\"Uncompressed\">\n"
//         "			<dataTypeRef name=\"LexiconClassListT\"/>\n"
//         "		</elementType>\n"
            "		<elementType name=\"case-sensitive\" default=\"0\">\n"
            "			<dataTypeRef name=\"bool\"/>\n"
            "		</elementType>\n"
            "		<elementType name=\"drop-stop-words\" default=\"0\">\n"
            "			<dataTypeRef name=\"bool\"/>\n"
            "		</elementType>\n"
//         "		<elementType name=\"inverted-skiplist-max-size\" default=\"512\" "
//         "               minInclusive=\"5\" maxInclusive=\"10000\" "
//         "               step=\"100\" >\n"
//         "			<dataTypeRef name=\"int\"/>\n"
//         "		</elementType>\n"
//         "		<elementType name=\"document-number-gap-code\" default=\"Golomb\" >\n"
//         "			<dataTypeRef name=\"DocumentNumberGapCodeListT\"/>\n"
//         "		</elementType>\n"
//         "		<elementType name=\"frequency-of-term-in-document-code\" default=\"Gamma\" >\n"
//         "			<dataTypeRef name=\"FrequencyOfTermInDocumentCodeListT\"/>\n"
//         "		</elementType>\n"
            "	</elementType>\n"
            ,
            false
            ,
            ""
            );
        
    }
};
