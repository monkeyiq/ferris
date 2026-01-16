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

    $Id: MatchedEAGenerators.cpp,v 1.5 2010/09/24 21:30:54 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <MatchedEAGenerators.hh>
// #ifdef HAVE_MAGICK
// #include <Magick++.h>
// #endif

using namespace std;

namespace Ferris
{
    

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    bool
    AttributeCreator::supportsCreateForContext( fh_context c )
    {
        return false;
    }
    

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

MatchedEAGeneratorFactory::MatchedEAGeneratorFactory( const fh_matcher& ma )
    :
    theMatcher(ma)
{
}

MatchedEAGeneratorFactory::MatchedEAGeneratorFactory()
{
}

bool
MatchedEAGeneratorFactory::hasInterest( const fh_context& ctx )
{
    return theMatcher( ctx );
}


void
MatchedEAGeneratorFactory::tryBrew( const fh_context& ctx )
{
//     cerr << "tryBrew "
//          << " ctx name:" << ctx->getDirName()
//          << " ctx path:" << ctx->getDirPath()
//          << endl;
    
    if( hasInterest( ctx ) )
    {
//        cerr << "matcher is GOOD for ctx:" << ctx->getURL() << endl;
        Brew( ctx );
    }
}

bool
MatchedEAGeneratorFactory::isDynamic()
{
    return false;
}

bool
MatchedEAGeneratorFactory::hasState()
{
    return false;
}



bool
MatchedEAGeneratorFactory::tryBrew( const fh_context& ctx, const std::string& eaname )
{
    return false;
}

AttributeCreator::CreatePri_t
MatchedEAGeneratorFactory::getCreatePriority()
{
    return CREATE_PRI_NOT_SUPPORTED;
}

fh_attribute
MatchedEAGeneratorFactory::CreateAttr(
    const fh_context& a,
    const string& rdn,
    fh_context md )
{
    ostringstream ss;
    ss << "MatchedEAGeneratorFactory::CreateAttr() operation not supported rdn:" << rdn;
    Throw_FerrisCreateAttributeNotSupported( tostr(ss), 0 );
}

void
MatchedEAGeneratorFactory::augmentRecommendedEA( const fh_context& a, fh_stringstream& ss )
{
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <gmodule.h>

void
GModuleMatchedEAGeneratorFactory::ensureGModuleIsLoaded()
{
    if( !ghandle )
    {
        LG_PLUGIN_I << "Linking in implementaion of:" << ImplName << endl;

        ghandle = g_module_open ( ImplName.c_str(), G_MODULE_BIND_LAZY);
        if (!ghandle)
        {
            ostringstream ss;
            ss  << "Error, unable to open module file: "
                << g_module_error ()
                << endl;
//            cerr << tostr(ss) << endl;
            LG_PLUGIN_I << tostr(ss) << endl;
            Throw_GModuleOpenFailed( tostr(ss), 0 );
        }

        if (!g_module_symbol (ghandle, "CreateRealFactory", 
                              (gpointer*)&CreateRealFactory))
        {
            ostringstream ss;
            ss  << "Error, unable to resolve CreateRealFactory in module file: "
                << g_module_error()
                << " ImplName:" << ImplName
                << endl;
//            cerr << tostr(ss) << endl;
            LG_PLUGIN_I << tostr(ss) << endl;
            Throw_GModuleOpenFailed( tostr(ss), 0 );
        }

        RealFactory = CreateRealFactory();
    }
}


void
GModuleMatchedEAGeneratorFactory::Brew( const fh_context& ctx )
{
    ensureGModuleIsLoaded();
    RealFactory->Brew( ctx );
}

GModuleMatchedEAGeneratorFactory::GModuleMatchedEAGeneratorFactory(
    const fh_matcher& ma,
    const string& implname )
    :
    MatchedEAGeneratorFactory( ma ),
    ImplName( implname ),
    ghandle( 0 ),
    ghandle_factory( 0 ),
    RealFactory( 0 ),
    CreatePriCache( CREATE_PRI_NOT_SUPPORTED ),
    module_isDynamic( 0 ),
    module_hasState( 0 ),
    getCreatePri( 0 )
{
//    cerr << "GModuleMatchedEAGeneratorFactory() impl:" << implname << endl;
}

static string getFactoryName( const string& s )
{
    string ret = s;
    ret.replace( ret.find(".so"), 3, "_factory.so" );
    return ret;
}

void
GModuleMatchedEAGeneratorFactory::ensureFactoryModuleVTable()
{
    if( !ghandle_factory )
    {
        string symbolName;
        string FactoryName = getFactoryName( ImplName );
        LG_PLUGIN_D << "Linking in implementaion of:" << FactoryName << endl;

        ghandle_factory = g_module_open ( FactoryName.c_str(), G_MODULE_BIND_LAZY);
        if (!ghandle_factory)
        {
            ostringstream ss;
            ss  << "Error, unable to open module file, "
                << g_module_error ()
                << endl;
            LG_PLUGIN_ER << tostr(ss) << endl;
            Throw_GModuleOpenFailed( tostr(ss), 0 );
        }


        symbolName = "isDynamic";
        if(!g_module_symbol (ghandle_factory, symbolName.c_str(),  (gpointer*)&module_isDynamic))
        {
            LG_PLUGIN_ER << "Cant find symbol:" << symbolName
                         << " in module:" << FactoryName
                         << endl;
        }

        symbolName = "hasState";
        if(!g_module_symbol (ghandle_factory, symbolName.c_str(),  (gpointer*)&module_hasState))
        {
            LG_PLUGIN_ER << "Cant find symbol:" << symbolName
                         << " in module:" << FactoryName
                         << endl;
        }
        
        symbolName = "getCreatePri";
        if(!g_module_symbol (ghandle_factory, symbolName.c_str(),  (gpointer*)&getCreatePri))
        {
            LG_PLUGIN_ER << "Cant find symbol:" << symbolName
                         << " in module:" << FactoryName
                         << endl;
        }

    }
}


bool
GModuleMatchedEAGeneratorFactory::tryBrew(
    const fh_context& ctx,
    const std::string& eaname )
{
    ensureGModuleIsLoaded();
    return RealFactory->tryBrew( ctx, eaname );
}

bool
GModuleMatchedEAGeneratorFactory::isDynamic()
{
    bool ret = _Base::isDynamic();
    
    ensureFactoryModuleVTable();
    if( module_isDynamic )
    {
        ret = module_isDynamic();
    }
    
    return ret;
}


bool
GModuleMatchedEAGeneratorFactory::hasState()
{
    bool ret = _Base::hasState();
    
    ensureFactoryModuleVTable();
    if( module_hasState )
    {
        ret = module_hasState();
    }
    
    return ret;
}

    



AttributeCreator::CreatePri_t
GModuleMatchedEAGeneratorFactory::getCreatePriority()
{
    CreatePri_t ret = _Base::getCreatePriority();
    
    ensureFactoryModuleVTable();
    if( getCreatePri )
    {
        ret = getCreatePri();
    }
    
    return ret;
}


fh_attribute
GModuleMatchedEAGeneratorFactory::CreateAttr(
    const fh_context& a,
    const string& rdn,
    fh_context md )
{
    ensureGModuleIsLoaded();
    return RealFactory->CreateAttr( a, rdn, md );
}

bool
GModuleMatchedEAGeneratorFactory::supportsCreateForContext( fh_context c )
{
    ensureGModuleIsLoaded();
    return RealFactory->supportsCreateForContext( c );
}

void
GModuleMatchedEAGeneratorFactory::augmentRecommendedEA( const fh_context& a, fh_stringstream& ss )
{
    ensureGModuleIsLoaded();
    return RealFactory->augmentRecommendedEA( a, ss );
}



/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/



StaticGModuleMatchedEAGeneratorFactory::StaticGModuleMatchedEAGeneratorFactory(
    const fh_matcher& ma,
    const std::string& implname,
    const std::string& shortname,
    bool isDynamic,
    bool hasState,
    AttributeCreator::CreatePri_t CreatePri )
    :
    GModuleMatchedEAGeneratorFactory( ma, implname ),
    m_shortName( shortname ),
    m_isDynamic( isDynamic ),
    m_hasState( hasState )
{
    CreatePriCache = CreatePri;
}

StaticGModuleMatchedEAGeneratorFactory*
StaticGModuleMatchedEAGeneratorFactory::clone()
{
    return new StaticGModuleMatchedEAGeneratorFactory( theMatcher,
                                                       ImplName,
                                                       m_shortName,
                                                       m_isDynamic,
                                                       m_hasState,
                                                       CreatePriCache );
}


bool
StaticGModuleMatchedEAGeneratorFactory::isDynamic()
{
    return m_isDynamic;
}
    
bool
StaticGModuleMatchedEAGeneratorFactory::hasState()
{
    return m_hasState;
}
    
AttributeCreator::CreatePri_t
StaticGModuleMatchedEAGeneratorFactory::getCreatePriority()
{
    return CreatePriCache;
}

    const std::string&
    StaticGModuleMatchedEAGeneratorFactory::getShortName()
    {
        return m_shortName;
    }

    
    
    

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


Image::Image( fh_istream _ss )
    :
    ss(_ss),
    GotMetaData( false ),
    rgba( 0 ),
    aspect_ratio( 0 ),
    is_valid( false ),
    gamma(0.45455),
    w(0),
    h(0),
    d(0),
    a(0),
    failed_to_load( false )
{
//     if( !ss.get_rep() )
//     {
//         stringstream ss;
//         ss << "IStream is invalid!";
//         Throw_FerrisImageLoadFailed( tostr(ss), 0 );
//     }
}

Image::~Image()
{
    freeRGBA();
}

void
Image::setNewWidth( guint32 v )
{
    newWidth = v;
}

void
Image::setNewHeight( guint32 v )
{
    newHeight = v;
}



// fh_istream
// Image::getWidthStream( Context*, const std::string&, EA_Atom* attr )
// {
//     fh_stringstream ss;
//     ss << getWidth();
//     return ss;
// }

// fh_iostream
// Image::getWidthIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
// {
//     fh_stringstream ss;
//     ss << getWidth();
//     return ss;
// }


// void
// Image::updateWidthFromStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
// {
//     ss >> newWidth;
// }

// fh_istream
// Image::getHeightStream( Context*, const std::string&, EA_Atom* attr )
// {
//     fh_stringstream ss;
//     ss << getHeight();
//     return ss;
// }

// fh_iostream
// Image::getHeightIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
// {
//     fh_stringstream ss;
//     ss << getHeight();
//     return ss;
// }

// void
// Image::updateHeightFromStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
// {
//     ss >> newHeight;
// }

guint32 Image::getNewWidth()
{
    return newWidth ? newWidth : getWidth();
}

guint32 Image::getNewHeight()
{
    return newHeight ? newHeight : getHeight();
}


// fh_istream
// Image::getDepthPerColorStream( Context*, const std::string&, EA_Atom* attr )
// {
//     fh_stringstream ss;
//     ss << getDepthPerColor();
//     return ss;
// }

// fh_istream
// Image::getDepthStream( Context*, const std::string&, EA_Atom* attr )
// {
//     fh_stringstream ss;
//     ss << getDepth();
//     return ss;
// }

// fh_istream
// Image::getGammaStream( Context*, const std::string&, EA_Atom* attr )
// {
//     fh_stringstream ss;
//     ss << getGamma();
//     return ss;
// }

// fh_istream
// Image::getHasAlphaStream( Context*, const std::string&, EA_Atom* attr )
// {
//     fh_stringstream ss;
//     ss << hasAlpha();
//     return ss;
// }

// fh_istream
// Image::getAspectRatioStream( Context*, const std::string&, EA_Atom* attr )
// {
//     fh_stringstream ss;
//     ss << getAspectRatio();
//     return ss;
// }


void
Image::freeRGBA()
{
    if( rgba )
    {
        delete [] rgba;
        rgba = 0;
    }
}

// fh_istream
// Image::getRGBAStream( Context* c, const std::string&, EA_Atom* attr )
// {
//     cerr << "Image::getRGBAStream() c:" << c->getURL()
//          << " image:" << toVoid(this)
//          << endl;
//     fh_stringstream ss;
//     int sz          = getRGBASize();
//     guint32* buffer = getRGBA();

//     cerr << "Image::getRGBAStream(2) c:" << c->getURL()
//          << " image:" << toVoid(this)
//          << " sz:" << sz
//          << " buffer:" << toVoid(buffer)
//          << endl;
//     BackTrace();

//     ss.write( (char*)buffer, sz );
//     freeRGBA();
//     ss.clear();
//     return ss;
// }

// fh_iostream
// Image::getRGBAIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
// {
// //     cerr << "Image::getRGBAIOStream() c:" << c->getURL()
// //          << " image:" << toVoid(this) << endl;
//     fh_stringstream ss;
//     int sz          = getRGBASize();
//     guint32* buffer = getRGBA();

//     ss.write( (char*)buffer, sz );
//     freeRGBA();
//     ss.clear();
//     return ss;
// }

// //fh_context c, fh_istream imageStream )
// void test_saveImageData( fh_context c, fh_istream imageStream )
// {
// //    cerr << "test_saveImageData()" << endl;
// }


void
Image::updateFromStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
//     cerr << "Image::updateFromStream() c:" << c->getURL()
//          << " image:" << toVoid(this) << endl;

//     cerr << "Image::updateFromStream(1) c:" << c->getDirPath() << " rdn:" << rdn << endl;
    fh_context ctx = c;
    fh_istream tmpxx = ss;
//    test_saveImageData( c, ss );

    priv_ensureDataLoaded();
    priv_saveImageData( ctx, ss );
}



guint32
Image::getWidth()
{
    ensureDataLoaded( LOAD_ONLY_METADATA );
    return (guint32)w;
}

guint32
Image::getHeight()
{
    ensureDataLoaded( LOAD_ONLY_METADATA );
    return (guint32)h;
}

int
Image::getDepth()
{
    ensureDataLoaded( LOAD_ONLY_METADATA );
    return d*3+a;
}

int
Image::getDepthPerColor()
{
    ensureDataLoaded( LOAD_ONLY_METADATA );
    return d;
}



bool
Image::hasAlpha()
{
    ensureDataLoaded( LOAD_ONLY_METADATA );
    return a;
}

double
Image::getGamma()
{
    ensureDataLoaded( LOAD_ONLY_METADATA );
    return gamma;
}


guint32*
Image::getRGBA()
{
    ensureDataLoaded( LOAD_COMPLETE_IMAGE );
    return rgba;
}


guint32
Image::getRGBASize()
{
    ensureDataLoaded( LOAD_ONLY_METADATA );
    return w * h * sizeof(gint32);
}

void
Image::ensureRGBA_IsAllocated()
{
    if( !rgba )
    {
        rgba = new guint32 [w * h];
//        cerr << "ensureRGBA_IsAllocated() allocating rgba, got:" << toVoid( rgba ) << endl;
    }
}


void
Image::ensureDataLoaded( LoadType loadType )
{
    if( failed_to_load )
    {
//        cerr << "Image::ensureDataLoaded(failed already) t:" << loadType << endl;
        return;
    }
    
    
    if( loadType==LOAD_ONLY_METADATA && GotMetaData )
    {
//        cerr << "Image::ensureDataLoaded(gotMD) t:" << loadType << endl;
        return;
    }
    
//    cerr << "Image::ensureDataLoaded(gotMD) need to do work. t:" << loadType << endl;
    if( loadType==LOAD_COMPLETE_IMAGE )
    {
//        cerr << "Image::ensureDataLoaded(loading IMG data) t:" << loadType << endl;
        ensureRGBA_IsAllocated();
    }

//     cerr << "Image::ensureDataLoaded() t:" << loadType << endl;
//     BackTrace();
    try
    {
        priv_ensureDataLoaded( loadType );
    }
    catch( exception& e )
    {
        failed_to_load = true;
        throw;
    }
}

void
Image::priv_saveImageData( fh_context c, fh_istream imageStream )
{
    fh_stringstream ss;
    ss << "Plugin has no support for saving image data to context:" << c->getURL() << endl;
    cerr << tostr(ss) << endl;
    Throw_FerrisImageSaveFailed( tostr(ss), 0 );
}





double
Image::getAspectRatio()
{
    if( !aspect_ratio )
    {
        ensureDataLoaded( LOAD_ONLY_METADATA );
        aspect_ratio = 1.0 * getWidth() / MAX( getHeight(), 1 );
    }
    return aspect_ratio;
}

bool
Image::isValid()
{
    ensureDataLoaded( LOAD_ONLY_METADATA );
    return is_valid;
}


/**
 * converts
 * w=3
 * RGBRGBRGB
 * RGBARGBARGBA
 *
 * BRRG
 * BARG
 */
void
Image::convert_scanline_RGB_to_RGBA( guint32* scanline32, guint8 alpha )
{
    guint8* scanline = (guint8*)scanline32;
    guint8* input = scanline;
    
    gint32 i = 0;

    for( i=w; i > 0; )
    {
        --i;

//         guint32* target = &scanline[ 4*i ];
//         guint32* source = &scanline[ 3*i ];

//         *target = *source & 0xFFFFFF00;
//         *target = *target | alpha;
        
//        cerr << "Image::convert_scanline_RGB_to_RGBA() i:" << i << " h:" << h << " w:" << w << endl;
        input = &scanline[ i*3 ];
        
        scanline32[ i ] =
            //  A | R | G | B
            (alpha << 24) | (input[0] << 16) | (input[1] << 8) | (input[2]);
//            (alpha << 24) | ((input[0]) << 16) | ((input[1]) << 8) | (input[2]);

//         scanline[ (4*i) + 0 ] = scanline[ (3*i) + 0 ];
//         scanline[ (4*i) + 1 ] = alpha;
//         scanline[ (4*i) + 2 ] = scanline[ (3*i) + 2 ];
//         scanline[ (4*i) + 3 ] = scanline[ (3*i) + 3 ];

// //         scanline[ (4*i) + 3 ] = alpha;
// //         scanline[ (4*i) + 2 ] = scanline[ (3*i) + 2 ];
// //         scanline[ (4*i) + 1 ] = scanline[ (3*i) + 1 ];
// //         scanline[ (4*i) + 0 ] = scanline[ (3*i) + 0 ];


// // //         scanline[ (4*i) + 4 ] = alpha;
// // //         scanline[ (4*i) + 4 ] = scanline[ (3*i) + 3 ];
// // //         scanline[ (4*i) + 4 ] = scanline[ (3*i) + 3 ];
// // //         scanline[ (4*i) + 4 ] = scanline[ (3*i) + 3 ];
    }
//    scanline[ 4 ] = alpha;
}



void
Image::convertRGB_to_RGBA()
{
    guint32 line=0;

//    cerr << "Image::convertRGB_to_RGBA() line:" << line << " h:" << h << " w:" << w << endl;
    
    for( ; line < h; ++line )
    {
//        cerr << "Image::convertRGB_to_RGBA() line:" << line << " h:" << h << endl;
        guint32* scanline = &rgba[ w * line ];
        convert_scanline_RGB_to_RGBA( scanline );
    }
    
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
 
};
