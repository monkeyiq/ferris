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

    $Id: libferrispng.cpp,v 1.5 2010/09/24 21:31:57 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>
#include <Ferris/ValueRestorer.hh>

#include <png.h>
#include <unistd.h>

using namespace std;
namespace Ferris
{
    

class PNG_Image : public Image
{
    fh_context m_context;
    png_structp png_ptr;
    png_infop info_ptr;
    png_timep mod_time;
    bool      mod_time_read;

    png_color_16p bgcolor;

//    fh_context m_ctx;
    
    void readImageMetaData( png_infop info );
    void processStream( fh_istream ss );
    void setupGamma( png_infop info );

    bool usingTempRows;
    typedef Loki::SmartPtr< guint32,
                            Loki::RefCounted,
                            Loki::AllowConversion,
                            Loki::AssertCheck,
                            FerrisLoki::FerrisExArraySmartPtrStorage > Row_t;
    typedef list< Row_t > TempRows_t;
    TempRows_t TempRows;
    
    
    virtual void priv_ensureDataLoaded( LoadType loadType = LOAD_ONLY_METADATA );

    
    void freePngStuff();
    void setupPngStuff();
    
public:

    
    PNG_Image( const fh_context& a, fh_istream ss );
    ~PNG_Image();

    void info_callback(png_structp png_ptr, png_infop info);
    void row_callback(png_structp png_ptr, png_bytep new_row,
                      png_uint_32 row_num, int pass);
    void end_callback(png_structp png_ptr, png_infop info);
    int  read_chunk_cb(png_structp ptr, png_unknown_chunkp chunk);
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


static void
png_info_callback(png_structp png_ptr, png_infop info)
{
    PNG_Image* image = (PNG_Image*)png_get_progressive_ptr(png_ptr);
    image->info_callback( png_ptr, info );
}

static void
png_row_callback(png_structp png_ptr, png_bytep new_row,
             png_uint_32 row_num, int pass)
{
    PNG_Image* image = (PNG_Image*)png_get_progressive_ptr(png_ptr);
    image->row_callback( png_ptr, new_row, row_num, pass );
}

static void
png_end_callback(png_structp png_ptr, png_infop info)
{
    PNG_Image* image = (PNG_Image*)png_get_progressive_ptr(png_ptr);
    image->end_callback( png_ptr, info );
}

static int
png_read_chunk_cb(png_structp ptr,
                  png_unknown_chunkp chunk)
{
    PNG_Image* image = (PNG_Image*)png_get_user_chunk_ptr(ptr);
    return image->read_chunk_cb( ptr, chunk );
}


static void
png_error_fn(png_structp png_ptr, png_const_charp error_msg)
{
    cerr << "png error:" << error_msg << endl;
    BackTrace();
    Throw_FerrisPNGImageLoadFailed( error_msg, 0 );
}

static void
png_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
    LG_PNGIMAGE_W << "png_warning_fn() :" << warning_msg << endl;
}



/*
 * NOTE:
 *
 * If you read the file from different
 * routines, you will need to update the jmpbuf field every time you enter
 * a new routine that will call a png_*() function.
 */

    PNG_Image::PNG_Image( const fh_context& a, fh_istream ss )
    :
    Image(ss),
    m_context( a ),
    png_ptr(0),
    info_ptr(0),
    mod_time_read(0),
    usingTempRows(true)
{
    setupPngStuff();
//     processStream(ss);

//     m_ctx = a;
//     cerr << "PNG_Image() c:" << m_ctx->getURL() << endl;
}

PNG_Image::~PNG_Image()
{
    freePngStuff();
    TempRows.clear();
    
//    cerr << "~PNG_Image() c:" << m_ctx->getURL() << endl;
}


void
PNG_Image::freePngStuff()
{
    if( png_ptr && info_ptr )
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        png_ptr = 0;
        info_ptr = 0;
    }
    
        
}
    
void
PNG_Image::setupPngStuff()
{
    png_ptr = png_create_read_struct
        (PNG_LIBPNG_VER_STRING, (png_voidp)this, png_error_fn, png_warning_fn);

    if (!png_ptr)
        Throw_FerrisPNGImageLoadFailed( "png_create_read_struct failed", 0 );

    if (!(info_ptr = png_create_info_struct(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL,
                                (png_infopp)NULL);
        png_ptr = 0;
        Throw_FerrisPNGImageLoadFailed( "png_create_info_struct failed", 0 );
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr,
                                (png_infopp)NULL);
        Throw_FerrisPNGImageLoadFailed( "Initialization of callbacks failed", 0 );
    } 

    png_set_progressive_read_fn( png_ptr, (void *)this,
                                 png_info_callback, png_row_callback, png_end_callback);

    png_set_read_user_chunk_fn(png_ptr, (void*)this, png_read_chunk_cb);
}


void
PNG_Image::readImageMetaData( png_infop info )
{
    png_uint_32 width=0;
    png_uint_32 height=0;
    int bit_depth=0;
    int color_type=0;
    int interlace_method=0;
    int compression_method=0;
    int filter_method=0;
    png_byte channels=1;
    
    png_get_IHDR( png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
                  &interlace_method, &compression_method, &filter_method);

//    cerr << "PNG_Image::readImageMetaData(top)" << endl;
    
    LG_PNGIMAGE_D << "readImageMetaData() w:" << width
                  << " h:" << height << " d:" << bit_depth
                  << endl;

    setWidth( width );
    setHeight( height );
    setDepthPerColor( bit_depth );
    
    if( d==16 )
    {
        LG_PNGIMAGE_W << "Throwing away data, 16 bit to 8 bit conversion!" << endl;
        png_set_strip_16(png_ptr);
    }
    

    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_palette_to_rgb(png_ptr);
    }
    
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }

    if( color_type & PNG_COLOR_MASK_ALPHA )
    {
        setAlpha( bit_depth );
    }

    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        png_set_gray_to_rgb(png_ptr);
    }

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    {
        png_set_tRNS_to_alpha(png_ptr);
    }
    
    png_set_packing(png_ptr);

//    png_set_bgr(png_ptr);
    png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

    channels = png_get_channels( png_ptr, info_ptr );
    setupGamma( info );
    
    if( png_get_valid(png_ptr, info_ptr, PNG_INFO_tIME ))
    {
        png_get_tIME(png_ptr, info_ptr, &mod_time);
        mod_time_read = true;
    }

    bgcolor = 0;
    if( png_get_valid(png_ptr, info_ptr, PNG_INFO_bKGD ))
    {
        png_get_bKGD(png_ptr, info_ptr, &bgcolor);
    }

    setAspectRatio( png_get_pixel_aspect_ratio(png_ptr, info_ptr) );
    
        
}


void
PNG_Image::setupGamma( png_infop info )
{
    double screen_gamma = 0;
    if(const char* gamma_str = getenv("SCREEN_GAMMA"))
    {
        screen_gamma = (double)atof(gamma_str);
    }
    else if (const char* gamma_str = getenv("DISPLAY_GAMMA"))
    {
        screen_gamma = (double)atof(gamma_str);
    }
    else
    {
        LG_PNGIMAGE_W << "Can not determine screen gamma, please set "
                      << "DISPLAY_GAMMA or SCREEN_GAMMA to a floating point number."
                      << " Defaulting to 2.0"
                      << endl;
        screen_gamma = 2.0; /* A good guess for a PC monitor in a dark room */
    }

    if(png_get_gAMA(png_ptr, info_ptr, &gamma))
    {
        png_set_gamma(png_ptr, screen_gamma, gamma);
    }
    else
    {
        gamma = 0.45455;
        png_set_gamma(png_ptr, screen_gamma, 0.45455);
    }
}



void
PNG_Image::processStream( fh_istream ss )
{
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        Throw_FerrisPNGImageLoadFailed( "Reading of image data failed", 0 );
    } 
 
    ensureDataLoaded( LOAD_ONLY_METADATA );
    setValid( true );
}

void
PNG_Image::priv_ensureDataLoaded( LoadType loadType )
{
    const int bytes_size = 4096;
    unsigned char bytes[ bytes_size+1 ];
    bool first_block = true;
    
    LG_PNGIMAGE_D << "PNG_Image::ensureImageDataIsLoaded() loadType:" << loadType
                  << " loading-only-metadata:" << (loadType==LOAD_ONLY_METADATA)
                  << " GotMetaData:" << GotMetaData
                  << endl;
//    cerr << "PNG_Image::ensureImageDataIsLoaded() loadType:" << loadType << endl;

    if( GotMetaData )
    {
        if( usingTempRows )
        {
            usingTempRows = false;

            ensureRGBA_IsAllocated();
            int row_num=0;
            for( TempRows_t::iterator iter = TempRows.begin();
                 iter != TempRows.end(); iter++, row_num++ )
            {
                Row_t r = *iter;
                guint32* src = GetImpl(r);
                guint32* dst = &rgba[ row_num * w ];

                std::copy( src, src+w, dst );
            }

            TempRows.clear();
        }
    }

    
    freePngStuff();
    setupPngStuff();
    LG_PNGIMAGE_D << "starting reading png information1. good:" << ss->good() << endl;

    ss = m_context->getIStream();
    
//     ss.clear();
//     LG_PNGIMAGE_D << "starting reading png information2. good:" << ss->good() << endl;
//     ss.seekg( 0 );
    LG_PNGIMAGE_D << "starting reading png information3. good:" << ss->good() << endl;

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        Throw_FerrisPNGImageLoadFailed( "Reading of image data failed", 0 );
    } 

    LG_PNGIMAGE_D << "starting reading png information. good:" << ss->good() << endl;
    while( ss->good() )
    {
        ss->read( (char*)bytes, bytes_size );
        LG_PNGIMAGE_D << "png read " << ss.gcount() << " bytes. max:" << bytes_size << endl;
        
        if( first_block )
        {
            first_block = false;
            if(png_sig_cmp( bytes, 0, ss->gcount()))
            {
                cerr << "PNG_Image::ensureImageDataIsLoaded() loadType:" << loadType
                     << " loading-only-metadata:" << (loadType==LOAD_ONLY_METADATA)
                     << " is not a PNG file"
                     << endl;
                
                png_destroy_read_struct(&png_ptr, &info_ptr,
                                        (png_infopp)NULL);
                Throw_FerrisPNGImageLoadFailed( "Not a png file!", 0 );
            }
        }

//         cerr << "PNG_Image::ensureImageDataIsLoaded() process data...ss->gcount():" << ss->gcount() << endl;
//         cerr << "bytes:" << hex << (int)bytes[0]
//              << "," << hex << (int)bytes[1]
//              << "," << hex << (int)bytes[2]
//              << "," << hex << (int)bytes[3]
//              << endl;
        
        png_process_data( png_ptr, info_ptr, bytes, ss->gcount() );

        if( GotMetaData && loadType==LOAD_ONLY_METADATA )
        {
            return;
        }
    }
}



void
PNG_Image::info_callback(png_structp png_ptr, png_infop info)
{
    LG_PNGIMAGE_D << "Getting image info" << endl;

    GotMetaData = true;
    readImageMetaData( info );
    png_read_update_info( png_ptr, info );
}

void
PNG_Image::row_callback(
    png_structp png_ptr,
    png_bytep new_row,
    png_uint_32 row_num,
    int pass)
{
//    LG_PNGIMAGE_D << "Processing row:" << row_num << " pass:" << pass << endl;
//     cerr << "Processing row:" << row_num << " pass:" << pass
//          << " rgba:" << toVoid( rgba )
//          << " w:" << w
//          << " usingTemp:" << usingTempRows
//          << endl;

    png_bytep old_row;
    
    if( usingTempRows )
    {
//        guint32* x = new guint32[w];
        Row_t x(new guint32[w]);
        TempRows.push_back( x );
        old_row = (png_bytep)GetImpl( x );
    }
    else
    {
        old_row = (png_bytep)&rgba[ row_num * w ];
    }
    
    png_progressive_combine_row(png_ptr, old_row, new_row);    
}

void
PNG_Image::end_callback(png_structp png_ptr, png_infop info)
{
    LG_PNGIMAGE_D << "Image load complete info" << endl;
}

int
PNG_Image::read_chunk_cb( png_structp ptr, png_unknown_chunkp chunk)
{
    /* The unknown chunk structure contains your
       chunk data: */
//     png_byte name[5];
//     png_byte *data;
//     png_size_t size;
    
    
//       return (-n); /* chunk had an error */
       return (0); /* did not recognize */
//       return (n); /* success */
}






///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


extern "C"
{

    fh_image CreateImageFromContext( const fh_context& c )
    {
        try
        {
//            cerr << "png loader CreateImageFromContext() c:" << c->getURL() << endl;
            
            fh_istream ss = c->getIStream();
            fh_image image = new PNG_Image( c, ss );
            return image;
        }
        catch( exception& e )
        {
            LG_PNGIMAGE_W << "Failed to load png EA, error:" << e.what() << endl;
            throw;
        }
    }
};
 
};
