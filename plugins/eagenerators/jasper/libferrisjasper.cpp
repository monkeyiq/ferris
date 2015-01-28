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

    $Id: libferrisjasper.cpp,v 1.2 2010/09/24 21:31:55 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>
#include <unistd.h>

#if !defined(UINT_FAST8_MIN)
typedef unsigned char uint_fast8_t;
#define UINT_FAST8_MIN	0
#define UINT_FAST8_MAX	255
#endif
#if !defined(INT_FAST64_MIN)
typedef gint64 int_fast64_t;
#define INT_FAST64_MIN	LLONG_MIN
#define INT_FAST64_MAX	LLONG_MAX
#endif
#if !defined(UINT_FAST64_MIN)
typedef guint64 uint_fast64_t;
#define UINT_FAST64_MIN	0
#define UINT_FAST64_MAX	ULLONG_MAX
#endif
typedef unsigned char uchar;


#include <jasper/jasper.h>

using namespace std;
namespace Ferris
{
    
    class Jasper_Image : public Image
    {
        jas_image_t *jasimage;    /** jasimage handle  */
        jas_stream_t *in;      /** input image   */
        char *inopts;
        int infmt;             /** The input image file format. */
        int_fast16_t numcmpts;

        virtual void priv_ensureDataLoaded( LoadType loadType = LOAD_ONLY_METADATA );
    
    public:
        Jasper_Image( const fh_context& c, fh_istream ss );
        ~Jasper_Image();
    };




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Jasper_Image::Jasper_Image( const fh_context& c, fh_istream ss )
    :
    Image(ss),
    jasimage(0),
    inopts(0)
{
    char* fmtname;
    
//    cerr << "Jasper_Image() p:" << c->getDirPath() << endl;
    if (!(in = jas_stream_fopen( c->getDirPath().c_str(), "rb")))
    {
        cerr << "J: cant fopen input image." << endl;
    }
    if ((infmt = jas_image_getfmt(in)) < 0)
    {
        cerr << "J: cant get format of image" << endl;
    }
	if (!(fmtname = jas_image_fmttostr(infmt))) {
        cerr << "cant get fmtname" << endl;
    }
    cerr << "fmtname:" << fmtname << endl;
    if (!(jasimage = jas_image_decode( in, infmt, 0 ))) //inopts)))
    {
        cerr << "J: cant decode image" << endl;
    }

    cerr << "Jasper_Image() image:" << toVoid( jasimage )
         << " in:" << toVoid(in)
         << " numcmpts:" << jas_image_numcmpts(jasimage) 
         << endl;

	jas_stream_close(  in    );
    setValid( true );
    uint_fast16_t cmptno = 0;
//     cerr << "Jasper_Image() 1"  << endl;
//     cerr << "Jasper_Image() raw-w:" << toVoid((jasimage)->cmpts_) << endl;
//     cerr << "Jasper_Image() raw-w:" << toVoid((jasimage)->cmpts_[cmptno]) << endl;
    
//     cerr << "Jasper_Image() w:" << jas_image_cmptwidth ( jasimage, 0 ) << endl;
    setWidth(  jas_image_cmptwidth ( jasimage, cmptno ));
    setHeight( jas_image_cmptheight( jasimage, cmptno ));
    setDepthPerColor( jas_image_cmptprec( jasimage, cmptno ));
    setAlpha( 8 );

    cerr << "Jasper_Image::Jasper_Image()"
         << " w:" << getWidth()
         << " h:" << getHeight()
         << endl;
}

Jasper_Image::~Jasper_Image()
{
	jas_image_destroy( jasimage );
	jas_image_clearfmts();
}




void
Jasper_Image::priv_ensureDataLoaded( LoadType loadType )
{
    const int bytes_size = 4096;
    unsigned char bytes[ bytes_size ];
    bool first_block = true;
    
    LG_JASPERIMAGE_D << "JASPER_Image::ensureImageDataIsLoaded()" << endl;

    uint_fast16_t cmptno = 0;
    uint_fast32_t x=0;
    uint_fast32_t y=0;
    uint_fast32_t width  = getWidth();
    uint_fast32_t height = getHeight();
    jas_matrix_t* jmatrix = 0;

    if (!(jmatrix = jas_matrix_create(height, width)))
    {
        cerr << "Cant make jmatrix for output data" << endl;
    }
    
    int rc = jas_image_readcmpt( jasimage, cmptno, x, y, width, height, jmatrix );
    cerr << "Jasper_Image::priv_ensureDataLoaded() rc:" << rc << endl;

    /*
     * Need to create RGBA 32 bpp image data
     */
//    memcpy( rgba, jmatrix->data_, width, height );
    for( int h = 0; h < getHeight(); ++h )
    {
        for( int w = 0; w < getWidth(); ++w )
        {
            rgba[ w + (h*w) ] = jas_matrix_get( jmatrix, w, h );
        }
    }
    
    jas_matrix_destroy( jmatrix );
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
        static Util::SingleShot virgin;
        if( virgin() )
        {
            jas_init();
        }

        try
        {
            fh_istream ss = c->getIStream();
            fh_image image = new Jasper_Image( c, ss );
            return image;
        }
        catch( exception& e )
        {
//            cerr << "Failed to load jasper EA, error:" << e.what() << endl;
            LG_JASPERIMAGE_W << "Failed to load jasper EA, error:" << e.what() << endl;
        }
    }
};


    
};
