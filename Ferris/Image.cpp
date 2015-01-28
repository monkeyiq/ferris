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

    $Id: Image.cpp,v 1.13 2010/11/17 21:30:45 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "Ferris.hh"
#include "Image_private.hh"

#include "config.h"
#include "MetadataServer_private.hh"

#include <string.h>

namespace Ferris
{
    using namespace std;


    s_ImageEAGeneratorsExtensionToShortName_t& getImageEAGeneratorsExtensionToShortName()
    {
        static s_ImageEAGeneratorsExtensionToShortName_t* ret = 0;
        if( !ret )
            ret = new s_ImageEAGeneratorsExtensionToShortName_t();
        return *ret;
    }
    
    Context::s_imageEAGenerators_t&
    Context::getImageEAGenerators()
    {
        static s_imageEAGenerators_t ret;
        return ret;
    }

    bool
    Context::imageEAGenerator_haveLoader()
    {
        string nameExtension = tolowerstr()( getNameExtension() );
        s_imageEAGenerators_t::iterator igeniter = getImageEAGenerators().find( nameExtension );
        return (igeniter != getImageEAGenerators().end());
    }

    void
    Context::imageEAGenerator_priv_createAttributes( bool checkForImageLoader )
    {
        bool haveImageLoader = true;
        bool writable        = false;

        if( SL::isAttributeBound( "width", "false" ) )
            return;
        
        if( checkForImageLoader )
        {
            haveImageLoader = false;
        
            string nameExtension = tolowerstr()( getNameExtension() );
            s_imageEAGenerators_t::iterator igeniter = getImageEAGenerators().find( nameExtension );

//         cerr << "imageEAGenerator_priv_createAttributes() ext:" << nameExtension
//              << " have-loader:" << (igeniter != getImageEAGenerators().end())
//              << endl;
    
            if( igeniter != getImageEAGenerators().end() )
            {
                haveImageLoader = true;
                writable = igeniter->second.second;
            }
        }
    
        if( haveImageLoader )
        {

            addAttribute( "depth-per-color",
                          EA_Atom_ReadOnly::GetIStream_Func_t(
                              &Context::imageEAGenerator_getDepthPerColorStream ),
                          XSD_BASIC_INT );
            addAttribute( "depth",
                          EA_Atom_ReadOnly::GetIStream_Func_t(
                              &Context::imageEAGenerator_getDepthStream ),
                          XSD_BASIC_INT );
            addAttribute( "gamma",
                          EA_Atom_ReadOnly::GetIStream_Func_t(
                              &Context::imageEAGenerator_getGammaStream ),
                          XSD_BASIC_DOUBLE );
            addAttribute( "has-alpha",
                          EA_Atom_ReadOnly::GetIStream_Func_t(
                              &Context::imageEAGenerator_getHasAlphaStream ),
                          XSD_BASIC_BOOL );
            addAttribute( "aspect-ratio",
                          EA_Atom_ReadOnly::GetIStream_Func_t(
                              &Context::imageEAGenerator_getAspectRatioStream ),
                          XSD_BASIC_DOUBLE );
            
            if( writable )
            {
                addAttribute( "width",
                              EA_Atom_ReadOnly::GetIStream_Func_t(
                                  &Context::imageEAGenerator_getWidthStream ),
                              EA_Atom_ReadWrite::GetIOStream_Func_t(
                                  &Context::imageEAGenerator_getWidthIOStream ),
                              EA_Atom_ReadWrite::IOStreamClosed_Func_t(
                                  &Context::imageEAGenerator_updateWidthFromStream ),
                              FXD_WIDTH_PIXELS,
                              true );
                addAttribute( "height",
                              EA_Atom_ReadOnly::GetIStream_Func_t(
                                  &Context::imageEAGenerator_getHeightStream ),
                              EA_Atom_ReadWrite::GetIOStream_Func_t(
                                  &Context::imageEAGenerator_getHeightIOStream ),
                              EA_Atom_ReadWrite::IOStreamClosed_Func_t(
                                  &Context::imageEAGenerator_updateHeightFromStream ),
                              FXD_HEIGHT_PIXELS,
                              true );
                addAttribute( "rgba-32bpp",
                              EA_Atom_ReadOnly::GetIStream_Func_t(
                                  &Context::imageEAGenerator_getRGBAStream ),
                              EA_Atom_ReadWrite::GetIOStream_Func_t(
                                  &Context::imageEAGenerator_getRGBAIOStream ),
                              EA_Atom_ReadWrite::IOStreamClosed_Func_t(
                                  &Context::imageEAGenerator_updateFromStream ),
                              FXD_BINARY_RGBA32,
                              0 );
            }
            else
            {
                addAttribute( "width",
                              EA_Atom_ReadOnly::GetIStream_Func_t(
                                  &Context::imageEAGenerator_getWidthStream ),
                              FXD_WIDTH_PIXELS,  true );
                addAttribute( "height",
                              EA_Atom_ReadOnly::GetIStream_Func_t(
                                  &Context::imageEAGenerator_getHeightStream ),
                              FXD_HEIGHT_PIXELS, true );
                addAttribute( "rgba-32bpp",
                              EA_Atom_ReadOnly::GetIStream_Func_t(
                                  &Context::imageEAGenerator_getRGBAStream ),
                              FXD_BINARY_RGBA32 );
            }
            addAttribute( "megapixels",
                          EA_Atom_ReadOnly::GetIStream_Func_t(
                              &Context::imageEAGenerator_getMegapixelsStream ),
                          XSD_BASIC_DOUBLE,  true );
        }
    }



    struct FERRISEXP_DLLLOCAL imageEAGeneratorGModule
    {
        const std::string m_ext;
        const bool        m_writable;
        const std::string m_implname;
        const std::string m_shortname;

        bool m_failed;
    
        imageEAGeneratorGModule(
            const std::string& ext,
            bool writable,
            const std::string& implname,
            const std::string& shortname )
            :
            m_failed( false ),
            m_ext( ext ),
            m_writable( writable ),
            m_implname( implname ),
            m_shortname( shortname )
            {
            }
    
        fh_image resolve( const fh_context& c )
            {
                if( m_failed )
                {
                    ostringstream ss;
                    ss  << "Failed loading image plugin."
                        << " implementation is at:" << m_implname << endl
                        << "No image available for url:" << c->getURL()
                        << endl;
                    LG_PLUGIN_I << tostr(ss) << endl;
                    Throw_GModuleOpenFailed( tostr(ss), 0 );
                }
            
                fh_image (*CreateImageFromContext)( const fh_context& c );
            
                GModule* ghandle = g_module_open ( m_implname.c_str(), G_MODULE_BIND_LAZY );
                if( !ghandle )
                {
                    m_failed = true;
                
                    ostringstream ss;
                    ss  << "Error, unable to open module file: "
                        << g_module_error () << endl
                        << " implementation is at:" << m_implname << endl
                        << "No image available for url:" << c->getURL()
                        << endl;
                    LG_PLUGIN_I << tostr(ss) << endl;
                    Throw_GModuleOpenFailed( tostr(ss), 0 );
                }

                if (!g_module_symbol (ghandle, "CreateImageFromContext", 
                                      (gpointer*)&CreateImageFromContext))
                {
                    m_failed = true;

                    ostringstream ss;
                    ss  << "Error, unable to resolve CreateRealFactory in module file: "
                        << g_module_error () << endl
                        << " implementation is at:" << m_implname << endl
                        << "No image available for url:" << c->getURL()
                        << endl;
                    LG_PLUGIN_I << tostr(ss) << endl;
                    Throw_GModuleOpenFailed( tostr(ss), 0 );
                }

                Context::UnrollRegisteredImageEAGeneratorModule(
                    m_ext,
                    m_writable,
                    m_implname,
                    Context::f_imageEAGenerator( CreateImageFromContext ) );

                return c->getImage();
            }
    };

/**
 * implname -> imageEAGeneratorGModule*
 */
    typedef map< string, imageEAGeneratorGModule* > RegisterImageEAGeneratorModule_gmodules_t;
    static
    RegisterImageEAGeneratorModule_gmodules_t&
    get_RegisterImageEAGeneratorModule_gmodules()
    {
        static RegisterImageEAGeneratorModule_gmodules_t ret;
        return ret;
    }

/**
 * implname -> list of extensions
 */
    typedef map< string, stringlist_t > RegisterImageEAGeneratorModule_extensions_t;
    static
    RegisterImageEAGeneratorModule_extensions_t&
    get_RegisterImageEAGeneratorModule_extensions()
    {
        static RegisterImageEAGeneratorModule_extensions_t ret;
        return ret;
    }

    bool
    Context::RegisterImageEAGeneratorModule(
        const std::string& ext,
        bool writable,
        const std::string& implname,
        const std::string& shortname )
    {

        // we will leak a little ram here because we need to keep
        // the gmodule loaders in scope for entire program run.
        // they are fairly small though.
        RegisterImageEAGeneratorModule_gmodules_t::iterator gmodloaderiter =
            get_RegisterImageEAGeneratorModule_gmodules().find( implname );
        if( gmodloaderiter == get_RegisterImageEAGeneratorModule_gmodules().end() )
        {
            imageEAGeneratorGModule* g = new imageEAGeneratorGModule( ext, writable,
                                                                      implname, shortname );
            gmodloaderiter = get_RegisterImageEAGeneratorModule_gmodules().insert(
                make_pair( implname, g ) ).first;
        }
        get_RegisterImageEAGeneratorModule_extensions()[ implname ].push_back( ext );
    
        string k = tolowerstr()( ext );
        f_imageEAGenerator func( gmodloaderiter->second, &imageEAGeneratorGModule::resolve );
        getImageEAGenerators()[ k ] = make_pair( func, writable );


        getImageEAGeneratorsExtensionToShortName()[ ext ] = make_pair( shortname, writable );
    }

    bool
    Context::UnrollRegisteredImageEAGeneratorModule(
        const std::string& ext,
        bool writable,
        const std::string& implname,
        const f_imageEAGenerator& f )
    {
        string k = tolowerstr()( ext );
        getImageEAGenerators()[ k ] = make_pair( f, writable );

        /**
         * Replace the functor object for all other extensions that this
         * plugin can handle since we have loaded the plugin now.
         */
        stringlist_t& sl = get_RegisterImageEAGeneratorModule_extensions()[ implname ];
        for( stringlist_t::const_iterator si = sl.begin(); si != sl.end(); ++si )
        {
            getImageEAGenerators()[ *si ] = make_pair( f, writable );
        }
    }

    class FERRISEXP_API Image_OutOfProcess : public Image
    {
        fh_context c;
        static fh_istream& getNullStream()
            {
                static fh_stringstream ss;
                return ss;
            }
        
    public:
        Image_OutOfProcess( fh_context c )
            :
            Image( getNullStream() ),
            c( c )
            {
            }

        void setup()
            {
                priv_ensureDataLoaded();
            }
        
        
        virtual void priv_ensureDataLoaded( LoadType loadType = LOAD_ONLY_METADATA )
            {
                LG_MDSERV_D << "priv_ensureDataLoaded() type:" << loadType << endl;

                if( loadType == LOAD_COMPLETE_IMAGE )
                {
                    string earl = c->getURL();
                    LG_MDSERV_D << "priv_ensureDataLoaded(complete) url:" << earl
                                << " w:" << w << " h:" << h << " d:" << d
                                << endl;

                    string eaname = "rgba-32bpp";
                    string v = Ferris::syncMetadataServerGet( earl, eaname );
                    LG_MDSERV_D << "priv_ensureDataLoaded(complete) url:" << earl
                                << " v.sz:" << v.size()
                                << endl;

                    ensureRGBA_IsAllocated();
                    LG_MDSERV_D << "priv_ensureDataLoaded(allocated) url:" << earl
                                << " v.sz:" << v.size()
                                << endl;

                    memset( rgba, 0, w*h*4 );
//                    memcpy( rgba, v.data(), w*h*4 );
                    LG_MDSERV_D << "priv_ensureDataLoaded(copied) url:" << earl
                                << " v.sz:" << v.size()
                                << endl;
                    
//                     stringstream ss;
//                     ss << "Can not load full images out of process. to be implemented." << endl;
//                     Throw_FerrisImageLoadFailed( tostr(ss), 0 );
                    
                    return;
                }

                string v;
                string eaname;
                string earl = c->getURL();

                LG_MDSERV_D << "Image::priv_ensureDataLoaded(oproc) eaname:" << eaname
                            << " starting to read values...." << endl;

                eaname = "width";
                v = Ferris::syncMetadataServerGet( earl, eaname );
                LG_MDSERV_D << "Image::priv_ensureDataLoaded(oproc) eaname:" << eaname
                            << " have resulting value:" << v << endl;
                long w = toint( v );

                eaname = "height";
                v = Ferris::syncMetadataServerGet( earl, eaname );
                LG_MDSERV_D << "Image::priv_ensureDataLoaded(oproc) eaname:" << eaname
                            << " have resulting value:" << v << endl;
                long h = toint( v );

                eaname = "has-alpha";
                v = Ferris::syncMetadataServerGet( earl, eaname );
                LG_MDSERV_D << "Image::priv_ensureDataLoaded(oproc) eaname:" << eaname
                            << " have resulting value:" << v << endl;
                long a = toint( v );
                
                eaname = "gamma";
                v = Ferris::syncMetadataServerGet( earl, eaname );
                LG_MDSERV_D << "Image::priv_ensureDataLoaded(oproc) eaname:" << eaname
                            << " have resulting value:" << v << endl;
                double g = toType<double>( v );

                eaname = "depth-per-color";
                v = Ferris::syncMetadataServerGet( earl, eaname );
                LG_MDSERV_D << "Image::priv_ensureDataLoaded(oproc) eaname:" << eaname
                            << " have resulting value:" << v << endl;
                long d = toint( v );
                
                setWidth(  w );
                setDepthPerColor(  d );
                setHeight( h );
                setAlpha(  a );

                GotMetaData = true;
                setValid( true );
            }
    };
    FERRIS_SMARTPTR( Image_OutOfProcess, fh_Image_OutOfProcess );
    
    fh_image
    Context::priv_getImage()
    {
        string nameExtension = tolowerstr()( getNameExtension() );
        fh_image im = 0;

        LG_MDSERV_D << "Context::priv_getImage() url:" << getURL()
                    << " ext:" << nameExtension << endl;
        
        if( tryToUseOutOfProcessMetadataServer() )
        {
            LG_MDSERV_D << "Context::priv_getImage(maybe oproc?) url:" << getURL()
                        << " ext:" << nameExtension << endl;
            
            typedef s_ImageEAGeneratorsExtensionToShortName_t Mt;
            Mt& genmap = getOutOfProcess_ImageEAGeneratorsExtensionToShortName();
            Mt::iterator giter = genmap.find( nameExtension );
            if( giter != genmap.end() )
            {
                LG_MDSERV_D << "Context::priv_getImage()"
                            << " this is a out-of-process image metadata." << endl;

                string earl = getURL();
                try
                {
                    fh_Image_OutOfProcess im = new Image_OutOfProcess( this );
                    im->setup();
                    
//                     string v = Ferris::syncMetadataServerGet( earl, eaname );

//                     LG_MDSERV_D << "Context::ensureAttributesAreCreated(oproc) eaname:" << eaname
//                                 << " have resulting value:" << v
//                                 << endl;

//                     addAttribute( eaname, v );
                    LG_MDSERV_D << "Context::priv_getImage()"
                                << " have oproc image...." << endl;
                    return im;
                }
                catch( exception& e )
                {
                    LG_ATTR_D << "Error getting attribute from out of process server!"
                              << " earl:" << earl
                              << " e:" << e.what() << endl;
                    LG_MDSERV_I << "Error getting attribute from out of process server!"
                                << " earl:" << earl
                                << " e:" << e.what() << endl;
                    return 0;
                }
                
            }
        }
        
        s_imageEAGenerators_t::iterator igeniter = getImageEAGenerators().find( nameExtension );

        LG_CTX_D << "Context::priv_getImage() im:" << toVoid(im)
                 << " ext:" << getNameExtension()
                 << " have-loader:" << (igeniter != getImageEAGenerators().end())
                 << endl;
    
        if( igeniter != getImageEAGenerators().end() )
        {
            bool writable = igeniter->second.second;
            im = igeniter->second.first( getCoveredContext() );
        }
        return im;
    }

    typedef Cache< Context*, fh_image > s_image_cache_t;
    static s_image_cache_t& getImageCache()
    {
//         static s_image_cache_t ret;
//         static bool v = true;
//         if( v )
//         {
//             v = false;
//             ret.setMaxCollectableSize( 10 );
//             ret.setTimerInterval( 3000 );
//         }
//         return ret;

        static s_image_cache_t* retp = 0;
        if( !retp )
        {
            retp = new s_image_cache_t();
            retp->setMaxCollectableSize( 10 );
            retp->setTimerInterval( 3000 );
        }
        
        return *retp;
    }

    fh_image
    Context::getImageOrNULL()
    {
        fh_image im = getImageCache().get( getCoveredContext() );

        LG_CTX_D << "Context::getImage() im:" << toVoid(im)
                 << " ext:" << getNameExtension()
                 << " url:" << getURL()
                 << " cc:" << toVoid( getCoveredContext() )
                 << " this:" << toVoid(this)
                 << " size:" << distance( getImageCache().begin(), getImageCache().end() )
                 << endl;
    
        if( im )
        {
            return im;
        }

        im = priv_getImage();
        if( im )
        {
            LG_CTX_D << "Context::getImage(put) im:" << toVoid(im)
                     << " ext:" << getNameExtension()
                     << " url:" << getURL()
                     << " cc:" << toVoid( getCoveredContext() )
                     << " this:" << toVoid(this)
                     << endl;
            getImageCache().put( getCoveredContext(), im );
            return im;
        }

        return 0;
    }


    fh_image
    Context::getImage()
    {
        fh_image ret = getImageOrNULL();
        if( isBound( ret ) )
            return ret;
    
        stringstream ss;
        ss << "No handler for given image format." << endl
           << "url:" << getURL();
//        cerr << tostr(ss) << endl;
        Throw_FerrisImageLoadFailed( tostr(ss), 0 );
    }

    void
    Context::taintImage( fh_image im )
    {
        // FIXME
    }



    fh_istream
    Context::imageEAGenerator_getDepthPerColorStream ( Context* c, const std::string&, EA_Atom* attr )
    {
        fh_stringstream ss;
        fh_image im = 0;
        if( c->m_isNativeContext )  im = c->getImageOrNULL();
        else                        im = c->getImage();

        if( im )
            ss << im->getDepthPerColor();
    
        return ss;
    }

    int Context::getImageWidth()
    {
//    cerr << "Context::getImageWidth()" << endl;
        int ret = 0;
    
        fh_image im = 0;
        if( m_isNativeContext )  im = getImageOrNULL();
        else                     im = getImage();

        if( im )
            ret = im->getWidth();
    
        return ret;
    }

    int Context::getImageHeight()
    {
        int ret = 0;
    
        fh_image im = 0;
        if( m_isNativeContext )  im = getImageOrNULL();
        else                     im = getImage();
    
        if( im )
            ret = im->getHeight();

        return ret;
    }


    fh_istream
    Context::imageEAGenerator_getWidthStream( Context* c, const std::string&, EA_Atom* attr )
    {
        LG_ATTR_D << "Context::imageEAGenerator_getWidthStream() c:" << c->getURL() << endl;
        LG_MDSERV_D << "Context::imageEAGenerator_getWidthStream() c:" << c->getURL() << endl;
        fh_stringstream ss;
        string t = getStrAttr( c, "width-local", "" );
        if( !t.empty() )
            ss << t;
        else
        {
            if( int v = c->getImageWidth() )
                ss << v;
        }
        return ss;
    }

    fh_istream
    Context::imageEAGenerator_getMegapixelsStream( Context* c, const std::string&, EA_Atom* attr )
    {
        fh_stringstream ss;
        if( double v = (c->getImageWidth() * c->getImageHeight()/1000000.0) )
            ss << v;
        return ss;
    }


    fh_iostream
    Context::imageEAGenerator_getWidthIOStream(  Context* c, const std::string& rdn, EA_Atom* atom )
    {
        LG_MDSERV_D << "Context::imageEAGenerator_getWidthIOStream() c:" << c->getURL() << endl;
        fh_stringstream ss;
        if( int v = c->getImageWidth() )
            ss << v;
        return ss;
    }

    fh_istream
    Context::imageEAGenerator_getHeightStream( Context* c, const std::string&, EA_Atom* attr )
    {
        LG_MDSERV_D << "Context::imageEAGenerator_getHeightStream()" << endl;
        fh_stringstream ss;
        string t = getStrAttr( c, "height-local", "" );
        if( !t.empty() )
            ss << t;
        else
        {
            if( int v = c->getImageHeight() )
                ss << v;
        }
        return ss;
    }

    fh_iostream
    Context::imageEAGenerator_getHeightIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( int v = c->getImageHeight() )
            ss << v;
        return ss;
    }

    fh_istream
    Context::imageEAGenerator_getDepthStream         ( Context* c, const std::string&, EA_Atom* attr )
    {
        fh_stringstream ss;
        fh_image im = 0;
        if( c->m_isNativeContext )  im = c->getImageOrNULL();
        else                        im = c->getImage();
    
        if( im )
            ss << im->getDepth();
        return ss;
    }

    fh_istream
    Context::imageEAGenerator_getGammaStream         ( Context* c, const std::string&, EA_Atom* attr )
    {
        fh_stringstream ss;
        fh_image im = 0;
        if( c->m_isNativeContext )  im = c->getImageOrNULL();
        else                        im = c->getImage();
    
        if( im )
            ss << im->getGamma();
        return ss;
    }

    fh_istream
    Context::imageEAGenerator_getHasAlphaStream      ( Context* c, const std::string&, EA_Atom* attr )
    {
        fh_stringstream ss;
        fh_image im = 0;
        if( c->m_isNativeContext )  im = c->getImageOrNULL();
        else                        im = c->getImage();
    
        if( im )
            ss << im->hasAlpha();
        return ss;
    }
    
    fh_istream
    Context::imageEAGenerator_getAspectRatioStream   ( Context* c, const std::string&, EA_Atom* attr )
    {
        fh_image im = 0;
        double v = 0.0;
        
        double lw = toType<double>(getStrAttr( c, "width-local", "0" ));
        if( lw )
        {
            double lh = toType<double>(getStrAttr( c, "height-local", "0" ));
            v = lw / std::max( 1.0, lh );
        }
        else
        {
            if( c->m_isNativeContext )  im = c->getImageOrNULL();
            else                        im = c->getImage();
            
            if( im )
                v = im->getAspectRatio();
        }

        fh_stringstream ss;
        ss << v;
        return ss;
    }

    fh_istream
    Context::imageEAGenerator_getRGBAStream          ( Context* c, const std::string&, EA_Atom* attr )
    {
        fh_image im = c->getImage();

        fh_stringstream ss;
        int sz          = im->getRGBASize();
        if( sz )
        {
            guint32* buffer = im->getRGBA();
            if( buffer )
                ss.write( (char*)buffer, sz );
            im->freeRGBA();
        }
        ss.clear();
        return ss;
    }


    void
    Context::imageEAGenerator_updateWidthFromStream(  Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
    {
        fh_image im = c->getImage();
        c->taintImage( im );
        long v;
        ss >> v;
        im->setNewWidth( v );
    }

    void
    Context::imageEAGenerator_updateHeightFromStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
    {
        fh_image im = c->getImage();
        c->taintImage( im );
        long v;
        ss >> v;
        im->setNewHeight( v );
    }

    

    fh_iostream
    Context::imageEAGenerator_getRGBAIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_image im = c->getImage();

        fh_stringstream ss;
        int sz          = im->getRGBASize();
        guint32* buffer = im->getRGBA();

        ss.write( (char*)buffer, sz );
        im->freeRGBA();
        ss.clear();
        return ss;
    }
    
    void
    Context::imageEAGenerator_updateFromStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
    {
        fh_image im = c->getImage();
        im->updateFromStream( c, rdn, atom, ss );
    }


};
