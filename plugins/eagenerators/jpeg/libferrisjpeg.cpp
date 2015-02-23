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

    $Id: libferrisjpeg.cpp,v 1.4 2010/09/24 21:31:55 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>

#include <jpeglib.h>
#include <jerror.h>
#include <setjmp.h>
#include <stdio.h>
#include <unistd.h>
#include <memory>

#include <iomanip>

#ifdef OSX
namespace std
{
template<typename _Tp>
inline void
_Destroy(_Tp* __pointer)
{ __pointer->~_Tp(); }

  template<typename _T1, typename... _Args>
    inline void
    _Construct(_T1* __p, _Args&&... __args)
    { ::new(static_cast<void*>(__p)) _T1(std::forward<_Args>(__args)...); }

};
#endif

using namespace std;
namespace Ferris
{


class JPEG_Image_state;

class JPEG_Image : public Image
{
    fh_context m_context;
    JPEG_Image_state* state;

    void readImageMetaData();
    void processStream( fh_istream ss );
    void setupGamma();

    virtual void priv_ensureDataLoaded( LoadType loadType = LOAD_ONLY_METADATA );

    void setupErrorHanlder();
    void setupIOHanlder();
    
    void freeJpgStuff();
    void setupJpgStuff();
    
public:
    JPEG_Image( fh_context ctx, fh_istream ss );
    ~JPEG_Image();

    const static int INPUT_BUFFER_SIZE;
    
};





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


struct ferris_jpeg_source_mgr_t
{
    struct jpeg_source_mgr pub;

    bool start_of_file;
    JOCTET * buffer;

    JPEG_Image* image;
    fh_istream  ss;
    
};

typedef struct ferris_jpeg_source_mgr_t ferris_jpeg_source_mgr;
typedef ferris_jpeg_source_mgr* ferris_jpeg_source_ptr;


static void
ferris_jpeg_init_source (j_decompress_ptr cinfo)
{
    ferris_jpeg_source_ptr src = (ferris_jpeg_source_ptr)cinfo->src;

    src->start_of_file       = true;
    src->pub.bytes_in_buffer = 0;
    src->pub.next_input_byte = NULL;
}

static boolean
ferris_jpeg_fill_input_buffer (j_decompress_ptr cinfo)
{
    ferris_jpeg_source_ptr src = (ferris_jpeg_source_ptr)cinfo->src;

    src->ss->read( (char*)src->buffer, JPEG_Image::INPUT_BUFFER_SIZE);
    size_t nbytes = src->ss->gcount();
    
    if (nbytes <= 0)
    {
        if (src->start_of_file)
        {
            LG_JPEGIMAGE_D << "Trying to read an empty file!" << endl;
            Throw_FerrisJPEGImageLoadFailed("Trying to read an empty file!",0);
            return FALSE;
            //ERREXIT(cinfo, JERR_INPUT_EMPTY);
        }
        
        WARNMS(cinfo, JWRN_JPEG_EOF);
        /* Insert a fake EOI marker */
        src->buffer[0] = (JOCTET) 0xFF;
        src->buffer[1] = (JOCTET) JPEG_EOI;
        nbytes = 2;
    }

    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = nbytes;
    src->start_of_file = false;
    
    return TRUE;
}


static void
ferris_jpeg_skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
    ferris_jpeg_source_ptr src = (ferris_jpeg_source_ptr)cinfo->src;
    
    if (num_bytes > 0)
    {
        while (num_bytes > (long) src->pub.bytes_in_buffer)
        {
            num_bytes -= (long) src->pub.bytes_in_buffer;
            ferris_jpeg_fill_input_buffer(cinfo);
            /* note we assume that fill_input_buffer will never return FALSE,
             * so suspension need not be handled.
             */
        }
        src->pub.next_input_byte += (size_t) num_bytes;
        src->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
}


static void
ferris_jpeg_term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}



///////////////////////////////////////////////////////////////////////////////

struct my_error_mgr {
    struct jpeg_error_mgr pub;	/* "public" fields */
    jmp_buf setjmp_buffer;	/* for return to caller */
    string emsg;
};

class JPEG_Image_state
{
public:
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
};
typedef struct my_error_mgr * my_error_ptr;


const int JPEG_Image::INPUT_BUFFER_SIZE = 4096;


void
JPEG_Image::readImageMetaData()
{
}

void
JPEG_Image::processStream( fh_istream ss )
{
	jpeg_start_decompress(&state->cinfo);
}


void
JPEG_Image::setupGamma()
{
}


        template <class RandomAccessIterator>
        void
        radixdump( ostream& os, RandomAccessIterator first, RandomAccessIterator last, long radix=16)
        {
            int i=0;
            
            while( first != last )
            {
                os << setw(2)
                   << setbase(radix)
                   << setfill('0')
                   << (int)*first;
                ++first;
            }
        }


void
JPEG_Image::priv_ensureDataLoaded( LoadType loadType )
{
    LG_JPEGIMAGE_D << "loadType: " << loadType << endl;

//     cerr << "priv_ensureDataLoaded state:" << toVoid( state ) << endl;
//     cerr << "priv_ensureDataLoaded state->jerr.setjmp_buffer:" << toVoid( state->jerr.setjmp_buffer ) << endl;

    if( !state || !state->jerr.setjmp_buffer )
    {
        stringstream ss;
        ss << "can't load jpeg image" << endl;
        Throw_FerrisJPEGImageLoadFailed( ss.str(), 0 );
    }
    
    if (setjmp(state->jerr.setjmp_buffer)) {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object, close the input file, and return.
         */
        GotMetaData = true;
        string emsg = state->jerr.emsg;
//        cerr << "priv_ensureDataLoaded emsg:" << emsg << endl;
        freeJpgStuff();
        Throw_FerrisJPEGImageLoadFailed( emsg, 0 );
    }

    if( loadType == LOAD_ONLY_METADATA )
    {
        jpeg_read_header(&state->cinfo, TRUE);
        jpeg_calc_output_dimensions(&state->cinfo);

        setWidth ( state->cinfo.output_width );
        setHeight( state->cinfo.output_height );
        setAlpha ( false );
//        setGamma ( ??? );
//        setAspectRatio( ??? );
        setDepthPerColor( 8 );

        GotMetaData = true;
    }
    
    if( loadType == LOAD_COMPLETE_IMAGE )
    {
        freeJpgStuff();
        LG_JPEGIMAGE_D << "resetting ss to url:" << m_context->getURL() << endl;
        ss = m_context->getIStream();
//         ss.clear();
//         ss.seekg( 0 );
        setupJpgStuff();
        memset( rgba, 0, w*h*4 );

        if (setjmp(state->jerr.setjmp_buffer)) {
            /* If we get here, the JPEG code has signaled an error.
             * We need to clean up the JPEG object, close the input file, and return.
             */
            string emsg = state->jerr.emsg;
            freeJpgStuff();
            Throw_FerrisJPEGImageLoadFailed( emsg, 0 );
        }
        jpeg_read_header(&state->cinfo, TRUE);
        jpeg_calc_output_dimensions(&state->cinfo);
        
        LG_JPEGIMAGE_D << "loading complete image ... " << endl;
        
        jpeg_start_decompress(&state->cinfo);

        while (state->cinfo.output_scanline < state->cinfo.output_height)
        {
            guint32 rownum = state->cinfo.output_scanline;
            
            JSAMPLE* scanline = (JSAMPLE*)&rgba[ rownum*w ];
            int nread = jpeg_read_scanlines(&state->cinfo, &scanline, 1);

//             cerr << "loading jpeg line:" << rownum << endl;
//             cerr << "some pixels:" << endl;
//             radixdump( cerr, (guint8*)&rgba[rownum], (guint8*)&rgba[rownum+16] );
//             cerr << endl;
        }
        
        jpeg_finish_decompress(&state->cinfo);
//         cerr << "some pixels1:" << endl;
//         radixdump( cerr, (guint8*)&rgba[0], (guint8*)&rgba[16] );
//         cerr << endl;

//         for( int j=0; j<h; ++j )
//         {
//             for( int i=w-1; i>=0; --i )
//             {
//                 guint8* p = (guint8*)&rgba[j*w];
//                 p[i*4+0] = 255; // B
//                 p[i*4+1] = 0; // G
//                 p[i*4+2] = 0; // R
//                 p[i*4+3] = 0; // A
//             }
//         }
        
        
        convertRGB_to_RGBA();
    }
    
}

    JPEG_Image::JPEG_Image( fh_context ctx, fh_istream ss )
    :
    state( 0 ),
    Image(ss),
    m_context( ctx )
{
    LG_JPEGIMAGE_D << "ctor" << endl;

    setupJpgStuff();

//     ensureDataLoaded( LOAD_ONLY_METADATA );
//     setValid( true );
}

JPEG_Image::~JPEG_Image()
{
//    cerr << "~JPEG_Image()" << endl;
    freeJpgStuff();
}


void
JPEG_Image::freeJpgStuff()
{
    if( state )
    {
        jpeg_destroy_decompress(&state->cinfo);
        if( state->cinfo.src )
        {
            ferris_jpeg_source_ptr src = (ferris_jpeg_source_ptr) state->cinfo.src;
            std::_Destroy( &src->ss );
        }
        
        delete state;
        state = 0;
    }
}

void
JPEG_Image::setupJpgStuff()
{
    state = new JPEG_Image_state();
    state->cinfo.err = jpeg_std_error(&state->jerr.pub);
	jpeg_create_decompress(&state->cinfo);
    
    LG_JPEGIMAGE_D << "have setup decompress and std_error" << endl;

    setupErrorHanlder();
    setupIOHanlder();
}



static void
ferris_jpeg_error_exit( j_common_ptr cinfo )
{
    my_error_ptr myerr = (my_error_ptr) cinfo->err;
    char buffer[JMSG_LENGTH_MAX];
    
    (*cinfo->err->format_message) (cinfo, buffer);
    LG_JPEGIMAGE_ER << buffer << endl;
    stringstream ss;
    ss << buffer << endl;
    myerr->emsg = tostr(ss);

    /* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
}


static void
ferris_jpeg_output_message (j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];
  (*cinfo->err->format_message) (cinfo, buffer);
  LG_JPEGIMAGE_D << buffer << endl;
}



void
JPEG_Image::setupErrorHanlder()
{
    LG_JPEGIMAGE_D << "Setting up a custom error handler." << endl;

    state->cinfo.err->error_exit      = ferris_jpeg_error_exit;
    state->cinfo.err->output_message  = ferris_jpeg_output_message;
}

void
JPEG_Image::setupIOHanlder()
{
    LG_JPEGIMAGE_D << "Setting up a custom IO handler." << endl;
    
    ferris_jpeg_source_ptr src = (ferris_jpeg_source_ptr) state->cinfo.src;
    
    if ( !(state->cinfo.src) )
    {
        state->cinfo.src = (struct jpeg_source_mgr *)
            (*state->cinfo.mem->alloc_small) ((j_common_ptr) &state->cinfo,
                                        JPOOL_PERMANENT,
                                        sizeof(ferris_jpeg_source_mgr));
        src = (ferris_jpeg_source_ptr) state->cinfo.src;
        src->buffer = (JOCTET *)
            (*state->cinfo.mem->alloc_small) ((j_common_ptr) &state->cinfo,
                                        JPOOL_PERMANENT,
                                        INPUT_BUFFER_SIZE * sizeof(JOCTET));
    }

    src->pub.init_source        = ferris_jpeg_init_source;
    src->pub.fill_input_buffer  = ferris_jpeg_fill_input_buffer;
    src->pub.skip_input_data    = ferris_jpeg_skip_input_data;
    src->pub.resync_to_restart  = jpeg_resync_to_restart; /* use default method */
    src->pub.term_source        = ferris_jpeg_term_source;
    
    src->pub.bytes_in_buffer = 0;
    src->pub.next_input_byte = NULL;

    src->image = this;

    std::_Construct( &src->ss, ss );
}




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
            LG_JPEGIMAGE_D << "ImageDataEAGenerator_JPEG::Brew() "
                           << " path:" << c->getDirPath() << endl;
        
            fh_istream ss = c->getIStream();
            fh_image image = new JPEG_Image( c, ss );
            return image;
        }
        catch( exception& e )
        {
            LG_JPEGIMAGE_W << "Failed to load jpeg EA, error:" << e.what() << endl;
        }
    }
};



 
};
