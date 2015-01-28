
Note that this is just some code which was worked on and then moved
here because QImage is part of QtGui which is a largeish library. 

/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

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

    $Id: libferrispostgresqlshared.cpp,v 1.2 2006/12/07 06:49:42 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/


#include <QImage>

QImage contextToQImage( fh_context c )
{
    std::string s = getStrAttr( c, "content", "", true, true );
    QImage ret = QImage::fromData( (const unsigned char*)s.data(), s.length() );
    return ret;
}

int main( int argc, char** argv )
{
    fh_context img = c;
    string earl = c->getURL();
            
    int w, h;
    if( !adjustWidthAndHeightForDesired( toint( getStrAttr( img, "width",  "-1" )),
                                         toint( getStrAttr( img, "height", "-1" )),
                                         w, h, MaxDesiredWidthOrHeight ))
    {
        LG_WEBPHOTO_D << "Scaling image with Qt, no scaling needed.... img:" << earl << endl;
        return c;
    }
    LG_WEBPHOTO_D << "Scaling image with Qt, w:" << w << " h:" << h << endl;

    QImage input = contextToQImage( img );
    if( input.isNull() )
    {
        LG_WEBPHOTO_ER << "Failed to load input image:" << img->getURL() << endl;
        return c;
    }

    QImage output = input.scaled( w, h );

    string dst_url = Shell::getTmpDirPath() + "/libferris-webupload-temp-image.jpg";
    if( !output.save( dst_url.c_str(), "jpg" ))
    {
        LG_WEBPHOTO_ER << "Failed to save image:" << img->getURL() << endl;
        return c;
    }
    fh_context ret = Resolve(  dst_url );
    return ret;
}

return 0;
}
