/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001-2005 Ben Martin

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

    $Id: ImageMagickNamespaceGlue.cpp,v 1.2 2010/09/24 21:31:03 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/


//#include <string>
#include "ImageMagickNamespaceGlue.hh"
#ifdef HAVE_MAGICK

#ifdef _STLP_STRING
#error "Using STLPort std::string during compilation"
#endif

namespace Ferris
{
    Magick::Image& readImage( Magick::Image& im, const char* filepath_CSTR )
    {
//         fh_context theContext = Resolve( filepath_CSTR );
//         fh_istream iss = theContext->getIStream();
//         string iss_str = StreamToString( iss );
//         Magick::Blob inBlob( iss_str.data(), iss_str.length() );
//         im.read( inBlob );
        
//        std::string s = filepath_CSTR;
        im.read( filepath_CSTR );
        return im;
    }
};
#endif    


