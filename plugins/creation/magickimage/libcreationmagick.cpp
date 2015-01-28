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

    $Id: libcreationmagick.cpp,v 1.2 2010/09/24 21:31:52 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <FerrisCreationPlugin.hh>

#ifdef HAVE_MAGICK
#include <Magick++.h>
#endif


using namespace std;

namespace Ferris
{

    class CreationStatelessFunctorMagick
        :
        public CreationStatelessFunctor
    {
    public:
        virtual fh_context create( fh_context c, fh_context md );
    };

    extern "C"
    {
        fh_CreationStatelessFunctor
        Create()
        {
            return new CreationStatelessFunctorMagick();
        }
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    typedef map< string, ::Magick::ColorspaceType > MagicCSM_t;

    MagicCSM_t& getMagicCSM()
    {
        static MagicCSM_t m;
        bool virgin = true;

        if( virgin )
        {
            virgin = false;

            m["RGB"]  = ::Magick::RGBColorspace;
            m["Gray"] = ::Magick::GRAYColorspace;
            m["YUV"]  = ::Magick::YUVColorspace;
            m["CMYK"] = ::Magick::CMYKColorspace;
        }
        return m;
    }
    
    fh_context
    CreationStatelessFunctorMagick::create( fh_context c, fh_context md )
    {
#ifdef HAVE_MAGICK
 
        fh_context newc = SubCreate_file( c, md );
        string ImageTypeName = md->getDirName();
        LG_FERRISCREATE_D << "Create image at newc->getDirPath:" << newc->getURL() << endl;
        LG_FERRISCREATE_D << "Image type:" << ImageTypeName << endl;
            
        int w = toint( getStrSubCtx( md, "width", "0" ));
        int h = toint( getStrSubCtx( md, "height", "0" ));
        int q = toint( getStrSubCtx( md, "quality", "75" ));
        string label      = getStrSubCtx( md, "label", "" );
        string colorspace = getStrSubCtx( md, "colorspace", "" );
        double gamma      = todouble( getStrSubCtx( md, "gamma", "1" ));

        
        LG_FERRISCREATE_D << "create path:" << newc->getDirPath() << " w:" << w << " h:" << h
                          << " q:" << q
                          << " label:" << label
                          << " gamma:" << gamma
                          << " colorspace:" << colorspace
                          << endl;
        
        ::Magick::Image image( ::Magick::Geometry(w,h), ::Magick::ColorRGB(0,0,0) );
        image.magick( ImageTypeName.c_str() );
        image.quality( q );
        image.gamma( gamma );
        image.label( label.c_str() );
        if( getMagicCSM().find(colorspace) != getMagicCSM().end() )
        {
            image.colorSpace( getMagicCSM()[colorspace] );
        }
        
        image.write( (const char*)newc->getDirPath().c_str() );

        return newc;
#endif        

        fh_stringstream ss;
        ss << "SL_SubCreate() should not have gotten here, you have no "
           << "ImageMagicK at compile time"
           << " url:" << c->getURL()
           << endl;
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
    }
};
