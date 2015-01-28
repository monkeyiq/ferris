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

    $Id: libimlib2.cpp,v 1.2 2010/09/24 21:31:55 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>


#include <X11/Xlib.h>
#include <Imlib2.h>

#include <unistd.h>
#include <string.h>

using namespace std;
namespace Ferris
{
    
class Imlib2_Image : public Image
{
    Imlib_Image im;
    
    void readImageMetaData();
    void setupGamma();
    virtual void priv_ensureDataLoaded( LoadType loadType = LOAD_ONLY_METADATA );
    static void staticInit();
    
    
public:
    Imlib2_Image( const string& dn );
    ~Imlib2_Image();

};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

string
tostr( Imlib_Load_Error& e )
{
    switch( e )
    {
    case IMLIB_LOAD_ERROR_NONE:                        return "no error";
    case IMLIB_LOAD_ERROR_FILE_DOES_NOT_EXIST:         return "File does not exist";
    case IMLIB_LOAD_ERROR_FILE_IS_DIRECTORY:           return "File is a dir";
    case IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_READ:   return "read denied";
    case IMLIB_LOAD_ERROR_NO_LOADER_FOR_FILE_FORMAT:   return "no loader to handle image";
    case IMLIB_LOAD_ERROR_PATH_TOO_LONG:               return "path too long";
    case IMLIB_LOAD_ERROR_PATH_COMPONENT_NON_EXISTANT: return "path component non existant";
    case IMLIB_LOAD_ERROR_PATH_COMPONENT_NOT_DIRECTORY:return "path component non dir";
    case IMLIB_LOAD_ERROR_PATH_POINTS_OUTSIDE_ADDRESS_SPACE: return "path out of addr spc";
    case IMLIB_LOAD_ERROR_TOO_MANY_SYMBOLIC_LINKS:     return "too many sym links";
    case IMLIB_LOAD_ERROR_OUT_OF_MEMORY:               return "out of memory";
    case IMLIB_LOAD_ERROR_OUT_OF_FILE_DESCRIPTORS:     return "out of fds";
    case IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_WRITE:  return "permission denied";
    case IMLIB_LOAD_ERROR_OUT_OF_DISK_SPACE:           return "out of disk space";
    case IMLIB_LOAD_ERROR_UNKNOWN:
    default:
        break;
    }

    return "unknown imlib2 error";
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Imlib2_Image::Imlib2_Image( const string& dn )
    :
    Image( fh_istream() )
{
    staticInit();
    
    Imlib_Load_Error imlib2err;
    im = imlib_load_image_with_error_return( dn.c_str(), &imlib2err );
    
    if( imlib2err !=  IMLIB_LOAD_ERROR_NONE )
    {
        ostringstream ss;
        ss << "Error loading imlib2 image:" << tostr( imlib2err );
        Throw_FerrisImlib2ImageLoadFailed( tostr(ss), 0 );
    }

    imlib_context_set_image(im);
    readImageMetaData();
}

Imlib2_Image::~Imlib2_Image()
{
    imlib_context_set_image(im);
    imlib_free_image();
}


void
Imlib2_Image::staticInit()
{
    static bool virgin = true;

    if( virgin )
    {
        virgin = false;

        imlib_set_cache_size( 1024 * 32 );
    }
}


void
Imlib2_Image::readImageMetaData()
{
   imlib_context_set_image(im);
    
    setWidth(  imlib_image_get_width()  );
    setHeight( imlib_image_get_height() );
    setAlpha(  imlib_image_has_alpha() );

//    setDepthPerColor( bit_depth );

    setupGamma();
    setValid( true );
}


void
Imlib2_Image::setupGamma()
{
}

void
Imlib2_Image::priv_ensureDataLoaded( LoadType loadType )
{
    imlib_context_set_image(im);

    if( loadType == LOAD_COMPLETE_IMAGE )
    {
        ensureRGBA_IsAllocated();
        DATA32* d = imlib_image_get_data_for_reading_only();

        memcpy( rgba, d, w*h*4 );
    }
    
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern "C"
{

    fh_image CreateImageFromContext( const fh_context& c )
    {

        try
        {
            string dn = c->getDirPath();
            fh_istream iss = c->getLocalIStream( dn );
            fh_image image = new Imlib2_Image( dn );
            return image;
        }
        catch( exception& e )
        {
            LG_IMLIB2IMAGE_W << "Failed to load imlib2 EA, error:" << e.what() << endl;
        }
    }
};


    
};
