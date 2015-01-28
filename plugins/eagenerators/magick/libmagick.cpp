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

    $Id: libmagick.cpp,v 1.3 2010/09/24 21:31:56 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>
#include <Ferris/Trimming.hh>

#include <MatchedEAGenerators.hh>
#include <Magick++.h>
#include <unistd.h>

#include <iomanip>

#include <FerrisGlue/ImageMagickNamespaceGlue.hh>

using namespace std;
namespace Ferris
{
    

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class MagicK_Image_state
{
public:
    Magick::Image im;
//    Magick::Blob blob;
};

class MagicK_Image
    :
        public Ferris::Image
{
    Context* theContext;
    MagicK_Image_state* state;
    
protected:
    
    virtual void priv_ensureDataLoaded( LoadType loadType = LOAD_ONLY_METADATA );
    virtual void priv_saveImageData( fh_context c, fh_istream imageStream );

public:
    
    MagicK_Image( const fh_context& c );
    ~MagicK_Image();

    
};



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////




void
MagicK_Image::priv_ensureDataLoaded( LoadType loadType )
{
    if( loadType == LOAD_ONLY_METADATA )
    {
        if( !theContext )
        {
            fh_stringstream ss;
            ss << "no context to load meta-data from. Should never happen" << endl;
            Throw_FerrisImageLoadFailed( tostr(ss), 0 );
        }

        string fn = theContext->getDirPath();
        try
        {
            
//        cerr << "MagicK_Image::priv_ensureDataLoaded(1) fn:" << fn << endl;
//         LG_MAGIKIMAGE_W << "Change of fn1:" << fn << endl;
//         fh_istream iss = theContext->getLocalIStream( fn );
//         LG_MAGIKIMAGE_W << "Change of fn2:" << fn << endl;
//        state->im.read( (char*)fn.c_str() );

//////////// std::string is needed for im.read( string )
            if( theContext->getIsNativeContext() )
            {
                const char* fn_CSTR = fn.c_str();
                readImage( state->im, fn_CSTR );
//                state->im.read( fn_CSTR );
            }
            else
            {
                fh_istream iss = theContext->getIStream();
                string iss_str = StreamToString( iss );
                Magick::Blob inBlob( iss_str.data(), iss_str.length() );
                state->im.read( inBlob );
            }
            
        

        setWidth ( state->im.columns() );
        setHeight( state->im.rows() );
//    setAlpha ( false );
        setGamma ( state->im.gamma() );
//        setAspectRatio( ??? );
        setDepthPerColor( state->im.depth() );
        GotMetaData = true;

        Magick::Image tmpimage;
        state->im = tmpimage;
//        cerr << "MagicK_Image::priv_ensureDataLoaded(2) fn:" << fn << endl;
        }
        catch( exception& e )
        {
            cerr << "MagicK_Image::priv_ensureDataLoaded(e) fn:" << fn
                 << " error:" << e.what()
                 << endl;
            throw;
        }
        
    }
    

    if( loadType == LOAD_COMPLETE_IMAGE )
    {
        if( theContext->getIsNativeContext() )
        {
            string fn = theContext->getDirPath();
            const char* fn_CSTR = fn.c_str();
            readImage( state->im, fn_CSTR );
        }
        else
        {
            fh_istream iss = theContext->getIStream();
            string iss_str = StreamToString( iss );
            Magick::Blob inBlob( iss_str.data(), iss_str.length() );
            state->im.read( inBlob );
        }
        
        LG_MAGIKIMAGE_D << "MagicK_Image::priv_ensureDataLoaded() "
                        << " w:" << state->im.columns() << " w2:" << w
                        << " h:" << state->im.rows()    << " h2:" << h
                        << " rgba:" << toVoid(rgba)
                        << endl;

//        state->im.write( 0, 0, w, h, "RGBA", Magick::IntegerPixel, rgba );
        Magick::Blob blob;
        state->im.magick( "RGBA" );
        state->im.write( &blob );
        memcpy( rgba, blob.data(), w*h*4 );
    }
    
}



        template <class RandomAccessIterator>
        void
        radixdump( ostream& os, RandomAccessIterator first, RandomAccessIterator last, long radix=16)
        {
            int i=0;
            
            while( first != last )
            {
                os << setw(2) << setbase(radix) << setfill('0') << (int)*first;
                ++first;
            }
        }
/**
 * Save an image from the raw RGBA32bpp data in imageStream to the context at c
 * in the format dictated by the extension or png otherwise.
 */
void
MagicK_Image::priv_saveImageData( fh_context c, fh_istream imageStream )
{
    try
    {
        int newWidth  = getNewWidth();
        int newHeight = getNewHeight();
        LG_MAGIKIMAGE_D << "MagicK_Image::saveImageData top" << endl;
        string extension = getStrAttr( c, "name-extension", "png" );
        PrefixTrimmer trimmer;
        trimmer.push_back( "." );
        extension = trimmer( extension );

        imageStream.clear();
        imageStream.seekg(0);
        string rawImgDataString = StreamToString( imageStream );
        Magick::Blob inBlob( rawImgDataString.data(), rawImgDataString.length() );

// //         char* start = (char*)rawImgDataString.data();
// //         radixdump( cerr, start, start+20 );
// //         cerr << endl;
        
//         Magick::Blob inBlob;
//         {
//             guint16* a = new guint16[60000];
//             int sz = rawImgDataString.length();
//             for( int i=0; i<sz; ++i )
//             {
//                 a[i] = rawImgDataString[i];
//             }
//             Magick::Blob b( a, sz*2 );
//             inBlob=b;
//         }
        
        Magick::Image tmpimage;
//        tmpimage.size( "40x40");
        {
            stringstream ss;
            ss << newWidth << "x" << newHeight;
            tmpimage.size( tostr(ss) );
            LG_MAGIKIMAGE_D << "MagicK_Image::saveImageDataA sz:" << tostr(ss) << endl;
        }
        tmpimage.depth( 8 );
        tmpimage.magick( "RGBA" );
        tmpimage.read( inBlob );
//         tmpimage.read( newWidth, newHeight, "RGBA",
//                     Magick::CharPixel, rawImgDataString.data() );

        LG_MAGIKIMAGE_D << "MagicK_Image::saveImageData( after read ) c:" << c->getDirPath()
                        << " ex:" << extension
                        << " data.len:" << rawImgDataString.length()
                        << endl;
    
        Magick::Blob outBlob;
//        tmpimage.magick( extension );
        tmpimage.magick( "JPEG" );
        tmpimage.write( &outBlob ); 
        fh_ostream oss = c->getIOStream( ios::trunc );
        oss.write( (const char*)outBlob.data(), outBlob.length() );
    
        state->im = tmpimage;
    }
    catch( Magick::Exception& e )
    {
        LG_MAGIKIMAGE_ER << "MagicK_Image::priv_saveImageData() e:" << e.what() << endl;
    }
    
}

MagicK_Image::MagicK_Image( const fh_context& c )
    :
    theContext( 0 ),
    state( new MagicK_Image_state() ),
    Image( fh_istream() )
{
    theContext = GetImpl( c );
}

MagicK_Image::~MagicK_Image()
{
//    cerr << "~MagicK_Image()" << endl;
    delete state;
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
            fh_image image = new MagicK_Image( c );
//         cerr << "ImageDataEAGenerator_MagicK::Brew() this:" << toVoid(this)
//              << " image:" << toVoid(image)
//              << " ctx:" << toVoid( GetImpl(a) )
//              << " ctx:" << a->getURL()
//              << endl;
            return image;
        }
        catch( exception& e )
        {
            LG_MAGIKIMAGE_W << "Failed to load image magik EA, error:" << e.what() << endl;
        }
    }
};



};
