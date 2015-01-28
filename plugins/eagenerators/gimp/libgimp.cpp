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

    $Id: libgimp.cpp,v 1.2 2010/09/24 21:31:54 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>

#include <math.h>
#include <libgimp/gimp.h>
#include <unistd.h>

using namespace std;


class GIMP_Image_state;

class GIMP_Image : public Image
{
    virtual void priv_ensureDataLoaded( LoadType loadType = LOAD_ONLY_METADATA );

    GIMP_Image_state* state;
    
public:
    GIMP_Image( const string& fn );
    ~GIMP_Image();

    
};





GimpPlugInInfo PLUG_IN_INFO = {
        NULL,   /* init_proc */
        NULL,   /* quit_proc */
        0,  /* query_proc */
        0     /* run_proc */
}; /* PLUG_IN_INFO */


class GIMP_Image_state
{
public:
    gint32 im;
};



void
GIMP_Image::priv_ensureDataLoaded( LoadType loadType )
{
    GotMetaData = true;

    if( loadType == LOAD_COMPLETE_IMAGE )
    {
        gimp_image_convert_rgb(state->im);
        

//        state->im.write( 0, 0, w, h, "RGBA", IntegerPixel, rgba );
    }
    
}

GIMP_Image::GIMP_Image( const string& fn )
    :
    state( new GIMP_Image_state() ),
    Image( fh_istream() )
{
    state->im = gimp_file_load(
        GIMP_RUN_NONINTERACTIVE,
        (gchar*)fn.c_str(),
        (gchar*)fn.c_str());


//     state->im = gimp_image_new(1,1,GIMP_RGB);
//     if(!gimp_image_set_filename( state->im, fn.c_str()))
//     {
//         stringstream ss;
//         ss << "gimp_image_set_filename() failed!";
//         Throw_FerrisGIMPImageLoadFailed( tostr(ss), 0 );
//     }



    GotMetaData = true;
    setWidth ( gimp_image_width (state->im) );
    setHeight( gimp_image_height(state->im) );

    switch( gimp_image_base_type(state->im) )
    {
    case GIMP_RGB:
        setDepthPerColor( 8 );
    case GIMP_GRAY:
        setDepthPerColor( 8 );
    case GIMP_INDEXED:
    {
        gint num_colors=0;
        gimp_image_get_cmap( state->im, &num_colors);
        setDepthPerColor( (int)ceil(num_colors/3) );
    }
    break;
    }
    
//    setAlpha ( false );
//    setGamma ( state->im.gamma() );
//        setAspectRatio( ??? );
}

GIMP_Image::~GIMP_Image()
{
    delete state;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern "C"
{
   fh_image CreateImageFromContext( const fh_context& c )
    {
        fh_image ret;

        try
        {
            // FIXME: should use a fifo here if file is not local!
            string dn = c->getDirPath();
            ret = new GIMP_Image( dn );
        }
        catch( exception& e )
        {
            LG_GIMPIMAGE_W << "Failed to load image gimp EA, error:" << e.what() << endl;
        }
        
        return ret;
    }
    
};


