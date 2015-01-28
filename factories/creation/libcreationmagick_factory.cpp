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

    $Id: libcreationmagick_factory.cpp,v 1.2 2010/09/24 21:31:26 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <FerrisCreationPlugin.hh>

using namespace std;

namespace Ferris
{
    static bool rst = appendExtraGenerateSchemaSimpleTypes(
        "<simpleType name=\"ColorSpaceT\">"  
        "    <restriction base=\"string\">"  
        "        <enumeration value=\"RGB\"/>"  
        "        <enumeration value=\"Gray\"/>"  
        "        <enumeration value=\"YUV\"/>"  
        "        <enumeration value=\"CMYK\"/>"  
        "    </restriction>"  
        "</simpleType>"  
        "<simpleType name=\"ColorSpaceListT\">"  
        "    <list itemType=\"ColorSpaceT\"/>"  
        "    <restriction>"  
        "        <length value=\"1\"/>"  
        "    </restriction>"  
        "</simpleType>"  
        "<simpleType name=\"JPEGColorSpaceT\">"  
        "    <restriction base=\"string\">"  
        "        <enumeration value=\"RGB\"/>"  
        "        <enumeration value=\"Gray\"/>"  
        "        <enumeration value=\"YUV\"/>"  
        "        <enumeration value=\"CMYK\"/>"  
        "        <enumeration value=\"YCCK\"/>"  
        "    </restriction>"  
        "</simpleType>"  
        "<simpleType name=\"JPEGColorSpaceListT\">"  
        "    <list itemType=\"JPEGColorSpaceT\"/>"  
        "    <restriction>"  
        "        <length value=\"1\"/>"  
        "    </restriction>"  
        "</simpleType>"
        );
    
    static bool rpng =
    RegisterCreationModule(
        "libcreationmagick.so",
        "png",
        "	<elementType name=\"png\" mime-major=\"image\">\n"
        "		<elementType name=\"name\" default=\"new.png\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"width\" default=\"800\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"height\" default=\"600\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"gamma\" default=\"2.0\" "
        "               minInclusive=\"0.8\" maxInclusive=\"2.3\" "
        "               step=\"0.05\" >\n"
        "			<dataTypeRef name=\"real\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"quality\" default=\"75\" "
        "               minInclusive=\"0\" maxInclusive=\"100\" "
        "               step=\"1\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"label\" default=\"\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "       <elementType name=\"colorspace\"> \n"
        "           <dataTypeRef name=\"ColorSpaceListT\"/> \n"
        "		</elementType> \n"
        "    </elementType>\n"
        );

    static bool rjpeg =
    RegisterCreationModule(
        "libcreationmagick.so",
        "jpeg",
        "	<elementType name=\"jpeg\" mime-major=\"image\">\n"
        "		<elementType name=\"name\" default=\"new.jpg\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"width\" default=\"800\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"height\" default=\"600\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"gamma\" default=\"2.0\" "
        "               minInclusive=\"0.8\" maxInclusive=\"2.3\" "
        "               step=\"0.05\" >\n"
        "			<dataTypeRef name=\"real\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"quality\" default=\"75\" "
        "               minInclusive=\"0\" maxInclusive=\"100\" "
        "               step=\"1\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"label\" default=\"\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "       <elementType name=\"colorspace\"> \n"
        "           <dataTypeRef name=\"JPEGColorSpaceListT\"/> \n"
        "		</elementType> \n"
        "    </elementType>\n"
        );

    static bool rgif =
    RegisterCreationModule(
        "libcreationmagick.so",
        "gif",
        "	<elementType name=\"gif\" mime-major=\"image\">\n"
        "		<elementType name=\"name\" default=\"new.gif\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"width\" default=\"40\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"height\" default=\"40\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"gamma\" default=\"2.0\" "
        "               minInclusive=\"0.8\" maxInclusive=\"2.3\" "
        "               step=\"0.05\" >\n"
        "			<dataTypeRef name=\"real\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"quality\" default=\"75\" "
        "               minInclusive=\"0\" maxInclusive=\"100\" "
        "               step=\"1\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"label\" default=\"\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "    </elementType>\n"
        );

    static bool rbmp =
    RegisterCreationModule(
        "libcreationmagick.so",
        "bmp",
        "	<elementType name=\"bmp\" mime-major=\"image\">\n"
        "		<elementType name=\"name\" default=\"new.bmp\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"width\" default=\"40\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"height\" default=\"40\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"gamma\" default=\"2.0\" "
        "               minInclusive=\"0.8\" maxInclusive=\"2.3\" "
        "               step=\"0.05\" >\n"
        "			<dataTypeRef name=\"real\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"quality\" default=\"75\" "
        "               minInclusive=\"0\" maxInclusive=\"100\" "
        "               step=\"1\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"label\" default=\"\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "    </elementType>\n"
        );

    static bool rfits =
    RegisterCreationModule(
        "libcreationmagick.so",
        "fits",
        "	<elementType name=\"fits\" mime-major=\"image\">\n"
        "		<elementType name=\"name\" default=\"new.fits\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"width\" default=\"40\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"height\" default=\"40\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"gamma\" default=\"2.0\" "
        "               minInclusive=\"0.8\" maxInclusive=\"2.3\" "
        "               step=\"0.05\" >\n"
        "			<dataTypeRef name=\"real\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"quality\" default=\"75\" "
        "               minInclusive=\"0\" maxInclusive=\"100\" "
        "               step=\"1\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"label\" default=\"\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "    </elementType>\n");

    static bool rfax =
    RegisterCreationModule(
        "libcreationmagick.so",
        "fax",
        "	<elementType name=\"fax\" mime-major=\"image\">\n"
        "		<elementType name=\"name\" default=\"new.fax\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"width\" default=\"400\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"height\" default=\"700\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "    </elementType>\n");

    static bool rpcx =
    RegisterCreationModule(
        "libcreationmagick.so",
        "pcx",
        "	<elementType name=\"pcx\" mime-major=\"image\">\n"
        "		<elementType name=\"name\" default=\"new.pcx\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"width\" default=\"640\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"height\" default=\"480\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "    </elementType>\n");

    static bool rpnm =
    RegisterCreationModule(
        "libcreationmagick.so",
        "pnm",
        "	<elementType name=\"pnm\" mime-major=\"image\">\n"
        "		<elementType name=\"name\" default=\"new.pnm\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"width\" default=\"640\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"height\" default=\"480\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "    </elementType>\n");

    static bool rtga =
    RegisterCreationModule(
        "libcreationmagick.so",
        "tga",
        "	<elementType name=\"tga\" mime-major=\"image\">\n"
        "		<elementType name=\"name\" default=\"new.tga\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"width\" default=\"640\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"height\" default=\"480\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "    </elementType>\n");

    static bool rtiff =
    RegisterCreationModule(
        "libcreationmagick.so",
        "tiff",
        "	<elementType name=\"tiff\" mime-major=\"image\">\n"
        "		<elementType name=\"name\" default=\"new.tif\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"width\" default=\"640\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"height\" default=\"480\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "    </elementType>\n");

    static bool rsgi =
    RegisterCreationModule(
        "libcreationmagick.so",
        "sgi",
        "	<elementType name=\"sgi\" mime-major=\"image\">\n"
        "		<elementType name=\"name\" default=\"new.sgi\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"width\" default=\"640\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"height\" default=\"480\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "    </elementType>\n");

    static bool rsun =
    RegisterCreationModule(
        "libcreationmagick.so",
        "sun",
        "	<elementType name=\"sun\" mime-major=\"image\">\n"
        "		<elementType name=\"name\" default=\"new.sun\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"width\" default=\"640\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"height\" default=\"480\" >\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "    </elementType>\n");
};
