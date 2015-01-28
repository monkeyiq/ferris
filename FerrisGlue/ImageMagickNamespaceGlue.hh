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

    $Id: ImageMagickNamespaceGlue.hh,v 1.2 2010/09/24 21:31:03 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_IMAGEMAGICK_NAMESPACE_GLUE_H_
#define _ALREADY_INCLUDED_FERRIS_IMAGEMAGICK_NAMESPACE_GLUE_H_

#include <config.h>
#ifdef HAVE_MAGICK
#include <Magick++.h>

namespace Ferris
{
    /**
     * This whole directory is for glue code. This method allows an
     * image to be loaded from a native path using the read( string )
     * method of ImageMagick++ which may be compiled to use the
     * native compilers std::string instead of STLport::string.
     */
    Magick::Image& readImage( Magick::Image& im, const char* filepath_CSTR );
};

#endif
#endif
