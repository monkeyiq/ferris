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

    $Id: libcreationdevsource_factory.cpp,v 1.2 2010/09/24 21:31:25 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <FerrisCreationPlugin.hh>

using namespace std;

namespace Ferris
{
    static bool rst = appendExtraGenerateSchemaSimpleTypes(
        "<simpleType name=\"LicenseT\">"  
        "    <restriction base=\"string\">"  
        "        <enumeration value=\"GPL\"/>"  
        "        <enumeration value=\"LGPL\"/>"  
        "        <enumeration value=\"BSD\"/>"  
        "    </restriction>"  
        "</simpleType>"  
        "<simpleType name=\"LicenseListT\">"  
        "    <list itemType=\"LicenseT\"/>"  
        "    <restriction>"  
        "        <length value=\"1\"/>"  
        "    </restriction>"  
        "</simpleType>"
        );

    
    static bool rcpp =
    RegisterCreationModule(
        "libcreationdevsource.so",
        "cpp",
        "   <elementType name=\"cpp\" mime-major=\"text\">\n"
        "		<elementType name=\"name\" default=\"new.cpp\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "       <elementType name=\"license\"> \n"
        "           <dataTypeRef name=\"LicenseListT\"/> \n"
        "		</elementType> \n"
        "	</elementType>\n",
        false
        );

    static bool rhh =
    RegisterCreationModule(
        "libcreationdevsource.so",
        "hh",
        "   <elementType name=\"hh\" mime-major=\"text\">\n"
        "		<elementType name=\"name\" default=\"new.hh\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "       <elementType name=\"license\"> \n"
        "           <dataTypeRef name=\"LicenseListT\"/> \n"
        "		</elementType> \n"
        "	</elementType>\n",
        false
        );

    static bool rc =
    RegisterCreationModule(
        "libcreationdevsource.so",
        "c",
        "   <elementType name=\"c\" mime-major=\"text\">\n"
        "		<elementType name=\"name\" default=\"new.c\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "       <elementType name=\"license\"> \n"
        "           <dataTypeRef name=\"LicenseListT\"/> \n"
        "		</elementType> \n"
        "	</elementType>\n",
        false
        );

    static bool rh =
    RegisterCreationModule(
        "libcreationdevsource.so",
        "h",
        "   <elementType name=\"h\" mime-major=\"text\">\n"
        "		<elementType name=\"name\" default=\"new.h\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "       <elementType name=\"license\"> \n"
        "           <dataTypeRef name=\"LicenseListT\"/> \n"
        "		</elementType> \n"
        "	</elementType>\n",
        false
        );



};
