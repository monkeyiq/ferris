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

    $Id: libferrisdjvulibre.cpp,v 1.2 2010/09/24 21:31:54 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <FerrisEAPlugin.hh>
#include <Runner.hh>
// #include <djvu/DjVuDocument.h>
// #include <djvu/DjVuImage.h>

#include <X11/Xlib.h>
#include <Imlib2.h>

#include <iomanip>

using namespace std;

namespace Ferris
{



    class DJVU_Image : public Image
    {
        virtual void priv_ensureDataLoaded( LoadType loadType = LOAD_ONLY_METADATA );
        virtual void priv_saveImageData( fh_context c, fh_istream imageStream );
        
        fh_context theContext;
        
    public:
        DJVU_Image( fh_context c );
        ~DJVU_Image();
    };





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    void
    DJVU_Image::priv_ensureDataLoaded( LoadType loadType )
    {
        LG_DJVUIMAGE_D << "loadType: " << loadType << endl;

        if( loadType == LOAD_ONLY_METADATA )
        {
           int w=0;
           int h=0;
           fh_runner r = new Runner();
           fh_stringstream cmdss;
           cmdss << "djvused " << theContext->getDirPath()
                 << " -e 'select 1; size' "
                 << " ";
           LG_DJVUIMAGE_D << "command:" << tostr(cmdss) << endl;
           r->setCommandLine( tostr(cmdss) );            
           r->setSpawnFlags(
               GSpawnFlags(
                   G_SPAWN_SEARCH_PATH |
                   G_SPAWN_STDERR_TO_DEV_NULL |
                   0 ));
           r->Run();
           fh_istream stdoutss =  r->getStdOut();
           string s;
           getline( stdoutss, s, '=' );
           getline( stdoutss, s, ' ' );
           w = toint( s );
           getline( stdoutss, s, '=' );
           getline( stdoutss, s, ' ' );
           h = toint( s );

           
//             int m_pagenumber = 1;
            
//             GP<DjVuDocument> doc=DjVuDocument::create_wait(
//                 GURL::Filename::UTF8(theContext->getDirPath().c_str()));
//             if (! doc->wait_for_complete_init())
//             {
//                 cerr << "can't load image at:" << theContext->getURL()
//                      << endl;
//             }
            
//             // Create DjVuImage
//             GP<DjVuImage> dimg=doc->get_page( m_pagenumber );
//             if (!dimg || ! dimg->wait_for_complete_decode() )
//             {
//                 cerr << "can't decode page:" << m_pagenumber
//                      << " for image at:" << theContext->getURL() 
//                      << " reason:" << (const char*)dimg->get_long_description()
//                      << endl;
//             }

//             DjVuInfo *info = dimg->get_info();
//             int w = (int)(dimg->get_real_width()  / info->dpi);
//             int h = (int)(dimg->get_real_height() / info->dpi);

           LG_DJVUIMAGE_D << "w:" << w << " h:" << h << endl;
            
            setWidth ( w );
            setHeight( h );
//            setGamma ( dimg->get_gamma() );
            setAlpha ( false );
            setDepthPerColor( 8 );
            GotMetaData = true;
            setValid( true );
        }
    
        if( loadType == LOAD_COMPLETE_IMAGE )
        {
            string tmppnmfile = "/tmp/ferristmp.pnm";
            fh_runner r = new Runner();
            fh_stringstream cmdss;
            cmdss << "ddjvu " << theContext->getDirPath() << " " << tmppnmfile;
//             cmdss << "ddjvu " << " - " << tmppnmfile;
            LG_DJVUIMAGE_D << "image decode command:" << tostr(cmdss)
                 << " theContext:" << theContext->getURL()
                 << endl;
            r->setCommandLine( tostr(cmdss) );            
            r->setSpawnFlags(
                GSpawnFlags(
                    G_SPAWN_SEARCH_PATH |
                    G_SPAWN_STDERR_TO_DEV_NULL |
                    G_SPAWN_STDOUT_TO_DEV_NULL |
                    r->getSpawnFlags() |
                    0 ));
//             r->setConnectStdIn( true );
            r->Run();
            
            // Give the image on stdin
//             fh_ostream stdinss = r->getStdIn();
//             fh_istream djvuss  = theContext->getIStream();
//             copy( istreambuf_iterator<char>(djvuss),
//                   istreambuf_iterator<char>(),
//                   ostreambuf_iterator<char>(stdinss));
// //             int count=0;
// //             ostreambuf_iterator<char> oiter = ostreambuf_iterator<char>(stdinss);
// //             for( istreambuf_iterator<char> i = istreambuf_iterator<char>(djvuss);
// //                  i != istreambuf_iterator<char>();
// //                  ++i, ++oiter, ++count )
// //             {
// //                 *oiter = *i;
// //             }
// //             LG_DJVUIMAGE_D << "copied :" << count << " bytes " << endl;
            
        
//            // Get the pnm on its stdout
//            fh_istream stdoutss =  r->getStdOut();
//            string pnm = StreamToString( stdoutss );

            // Wait for it...
            int es = r->getExitStatus();
            LG_DJVUIMAGE_D << "exit status:" << es << endl;
            
            // Convert pnm to raw RGBA32 data
            Imlib_Image im;
            Imlib_Load_Error imlib2err;
            im = imlib_load_image_with_error_return( tmppnmfile.c_str(), &imlib2err );
            if( imlib2err !=  IMLIB_LOAD_ERROR_NONE )
            {
                ostringstream ss;
                ss << "Error loading imlib2 image from tmp image at:" << tmppnmfile
                   << " reason:" << tostr( imlib2err );
                Throw_FerrisImlib2ImageLoadFailed( tostr(ss), 0 );
            }
            imlib_context_set_image(im);
            ensureRGBA_IsAllocated();
            DATA32* d = imlib_image_get_data_for_reading_only();
            memcpy( rgba, d, w*h*4 );
            imlib_context_set_image(im);
            imlib_free_image();
           
            //convertRGB_to_RGBA();
        }
    }

    /**
     * Save an image from the raw RGBA32bpp data in imageStream to the context at c
     * in djvu format.
     */
    void
    DJVU_Image::priv_saveImageData( fh_context c, fh_istream imageStream )
    {
        try
        {
            string tmppnmfile = "/tmp/ferristmp.pnm";
            int newWidth  = getNewWidth();
            int newHeight = getNewHeight();
            LG_DJVUIMAGE_D << "DJVU_Image::saveImageData top nw:" << newWidth
                 << " nh:" << newHeight
                 << endl;

            imageStream.clear();
            imageStream.seekg(0);
            string rawImgDataString = StreamToString( imageStream );
            Imlib_Image im;
            im = imlib_create_image_using_data( newWidth, newHeight,
                                                (unsigned int*)rawImgDataString.data() );
            imlib_context_set_image(im);
            imlib_save_image( tmppnmfile.c_str() );
            imlib_context_set_image(im);
            imlib_free_image();


            fh_runner r = new Runner();
            fh_stringstream cmdss;
//            cmdss << "ddjvu " << theContext->getDirPath() << " " << tmppnmfile;
            cmdss << "c44 " << tmppnmfile << " " << c->getDirPath();
            LG_DJVUIMAGE_D << "image encode command:" << tostr(cmdss) << endl;
            r->setCommandLine( tostr(cmdss) );            
            r->setSpawnFlags(
                GSpawnFlags(
                    G_SPAWN_SEARCH_PATH |
                    G_SPAWN_STDERR_TO_DEV_NULL |
                    G_SPAWN_STDOUT_TO_DEV_NULL |
                    r->getSpawnFlags() |
                    0 ));
            r->Run();

            // Wait for it...
            r->getExitStatus();

        }
        catch( exception& e )
        {
            LG_DJVUIMAGE_ER << "DJVU_Image::priv_saveImageData() e:" << e.what() << endl;
        }
    }
    
    DJVU_Image::DJVU_Image( fh_context c )
        :
        theContext( c ),
        Image( fh_istream() )
    {
        LG_DJVUIMAGE_D << "ctor" << endl;
//        ensureDataLoaded( LOAD_ONLY_METADATA );
    }

    DJVU_Image::~DJVU_Image()
    {
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
            try
            {
                LG_DJVUIMAGE_D << "CreateImageFromContext() "
                               << " path:" << c->getDirPath() << endl;
        
                fh_image image = new DJVU_Image( c );
                return image;
            }
            catch( exception& e )
            {
                LG_DJVUIMAGE_W << "Failed to load djvu EA, error:" << e.what() << endl;
            }
        }        
    };
};
