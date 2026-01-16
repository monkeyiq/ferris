/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2004 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: libferrisxineea.cpp,v 1.3 2010/11/16 21:30:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>
#include <Ferris/Trimming.hh>

#include <xine.h>
#include <xine/xineutils.h>

using namespace std;

namespace Ferris
{
    /************************************************************/
    /************************************************************/
    /************************************************************/

    class XineStream
        :
        public Handlable
    {
        xine_stream_t *stream;
    public:
        XineStream( xine_t* xine,
                    xine_video_port_t *vo_port,
                    xine_audio_port_t *ao_port,
                    fh_context c )
            :
            stream( 0 )
            {
                string mrl = c->getURL();

//                cerr << "Opening MRL:" << mrl << endl;
                LG_XINE_I << "Opening MRL:" << mrl << endl;
                stream = xine_stream_new(xine, ao_port, vo_port);
                if((!xine_open(stream, mrl.c_str() )))
                {
                }
            }
        
        virtual ~XineStream()
            {
//                cerr << "~XineStream() stream:" << stream << endl;
                if( stream )
                {
                    xine_close( stream );
                    xine_dispose( stream );
                }
            }

        xine_stream_t* raw()
            {
                return stream;
            }
        uint32_t getStreamInfo( int info )
            {
                uint32_t ret = xine_get_stream_info( stream, info );
                return ret;
            }
        string getMetaInfo( int info )
            {
                const char * str = xine_get_meta_info( stream, info );
                if( !str )
                    return "";
                return str;
            }
        
        
    };
    FERRIS_SMARTPTR( XineStream, fh_XineStream );


    static xine_video_port_t *vo_port = 0;
    static xine_audio_port_t *ao_port = 0;
    xine_t* getXine()
    {
        static bool xine_init_failed = false;
        static xine_t* xine = 0;

        if( xine_init_failed )
            return 0;
        
        if( !xine )
        {
            xine = xine_new();
            string configfile = string("") + xine_get_homedir() + "/.xine/config";
            xine_config_load(xine, configfile.c_str());
            xine_init(xine);

            if((vo_port = xine_open_video_driver(
                    xine, 
                    "none", XINE_VISUAL_TYPE_NONE, (void *)0)) == NULL)
            {
                cerr << "I'm unable to initialize null video driver. Giving up." << endl;
                xine_init_failed = true;
                return 0;
            }
            ao_port = xine_open_audio_driver(xine , "null", NULL);
//             if( !ao_port )
//             {
//                 cerr << "I'm unable to initialize null audio driver. Giving up." << endl;
//                 xine_init_failed = true;
//                 return 0;
//             }
        }
        return xine;
    }
    fh_XineStream getXineStream( fh_context c )
    {
        xine_t* xine = getXine();
        if( !xine )
            return 0;

        return new XineStream( xine, vo_port, ao_port, c );
    }
    
    class FERRISEXP_DLLLOCAL EAGenerator_XineMetadata
        :
        public MatchedEAGeneratorFactory
    {
        
    protected:

        virtual void Brew( const fh_context& a );

    public:

        EAGenerator_XineMetadata();

        fh_attribute CreateAttr(
            const fh_context& a,
            const string& rdn,
            fh_context md = 0 );
        
        
        virtual bool isDynamic()
            {
                return false;
            }
    
        bool
        supportsCreateForContext( fh_context c )
            {
                return false;
            }
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    EAGenerator_XineMetadata::EAGenerator_XineMetadata()
        :
        MatchedEAGeneratorFactory()
    {
    }

    void
    EAGenerator_XineMetadata::Brew( const fh_context& a )
    {
        static bool brewing = false;
        if( brewing )
            return;
        Util::ValueRestorer< bool > dummy1( brewing, true );

        LG_XINE_D << "EAGenerator_XineMetadata::Brew() a:'" << a->getDirPath() << "'" << endl;
        
        try
        {
            string earl = a->getURL();
            LG_XINE_D << "...earl0:" << earl << endl;
            if( starts_with( earl, "x-ferris" ) )
                return;

//            cerr << "getting a xine for ctx:" << a->getURL() << endl;
            fh_XineStream xs = getXineStream( a );

            uint32_t w = xs->getStreamInfo( XINE_STREAM_INFO_VIDEO_WIDTH );
            uint32_t h = xs->getStreamInfo( XINE_STREAM_INFO_VIDEO_HEIGHT );
            LG_XINE_D << "EAGenerator_XineMetadata::Brew() a:'" << a->getDirPath() << "'"
                      << " w:" << w << " h:" << h
                      << endl;
//             cerr << "EAGenerator_XineMetadata::Brew() a:'" << a->getDirPath() << "'"
//                  << " w:" << w << " h:" << h
//                  << endl;
//             BackTrace();

            a->addAttribute( "width",
                             tostr(xs->getStreamInfo( XINE_STREAM_INFO_VIDEO_WIDTH )),
                             FXD_WIDTH_PIXELS );
            a->addAttribute( "height",
                             tostr(xs->getStreamInfo( XINE_STREAM_INFO_VIDEO_HEIGHT )),
                             FXD_HEIGHT_PIXELS );

            a->addAttribute( "bitrate",
                             tostr(xs->getStreamInfo( XINE_STREAM_INFO_BITRATE )),
                             XSD_BASIC_INT );
            a->addAttribute( "can-seek",
                             tostr(xs->getStreamInfo( XINE_STREAM_INFO_SEEKABLE )),
                             XSD_BASIC_BOOL );

            a->addAttribute( "video-channel-count",
                             tostr(xs->getStreamInfo( XINE_STREAM_INFO_VIDEO_CHANNELS )),
                             XSD_BASIC_INT );
            a->addAttribute( "video-stream-count",
                             tostr(xs->getStreamInfo( XINE_STREAM_INFO_VIDEO_STREAMS )),
                             XSD_BASIC_INT );
            a->addAttribute( "video-bitrate",
                             tostr(xs->getStreamInfo( XINE_STREAM_INFO_VIDEO_BITRATE )),
                             XSD_BASIC_INT );
            a->addAttribute( "video-fourcc",
                             tostr(xs->getStreamInfo( XINE_STREAM_INFO_VIDEO_FOURCC )),
                             XSD_BASIC_INT );
            a->addAttribute( "xine-can-play-video",
                             tostr(xs->getStreamInfo( XINE_STREAM_INFO_VIDEO_HANDLED )),
                             XSD_BASIC_BOOL );

            
            a->addAttribute( "audio-channel-count",
                             tostr(xs->getStreamInfo( XINE_STREAM_INFO_AUDIO_CHANNELS )),
                             XSD_BASIC_INT );
            a->addAttribute( "audio-bitrate",
                             tostr(xs->getStreamInfo( XINE_STREAM_INFO_AUDIO_BITRATE )),
                             XSD_BASIC_INT );
            a->addAttribute( "audio-fourcc",
                             tostr(xs->getStreamInfo( XINE_STREAM_INFO_AUDIO_FOURCC )),
                             XSD_BASIC_INT );
            a->addAttribute( "xine-can-play-audio",
                             tostr(xs->getStreamInfo( XINE_STREAM_INFO_AUDIO_HANDLED )),
                             XSD_BASIC_BOOL );

            a->addAttribute( "has-video",
                             tostr(xs->getStreamInfo( XINE_STREAM_INFO_HAS_VIDEO )),
                             XSD_BASIC_BOOL );
            a->addAttribute( "has-audio",
                             tostr(xs->getStreamInfo( XINE_STREAM_INFO_HAS_AUDIO )),
                             XSD_BASIC_BOOL );

            a->addAttribute( "title",
                             xs->getMetaInfo( XINE_META_INFO_TITLE ),
                             XSD_BASIC_STRING );
            a->addAttribute( "comment",
                             xs->getMetaInfo( XINE_META_INFO_COMMENT ),
                             XSD_BASIC_STRING );
            a->addAttribute( "artist",
                             xs->getMetaInfo( XINE_META_INFO_ARTIST ),
                             XSD_BASIC_STRING );
            a->addAttribute( "genre",
                             xs->getMetaInfo( XINE_META_INFO_GENRE ),
                             XSD_BASIC_STRING );
            a->addAttribute( "album",
                             xs->getMetaInfo( XINE_META_INFO_ALBUM ),
                             XSD_BASIC_STRING );
            a->addAttribute( "year",
                             xs->getMetaInfo( XINE_META_INFO_YEAR ),
                             XSD_BASIC_STRING );
            
        }
        catch( exception& e )
        {
            LG_XINE_D << "Failed to load EAs, error:" << e.what() << endl;
        }
//        cerr << "dropped a xine for ctx:" << a->getURL() << endl;
    }
    
    fh_attribute
    EAGenerator_XineMetadata::CreateAttr( const fh_context& a,
                                 const string& rdn,
                                 fh_context md )
    {
        fh_stringstream ss;
        ss << "Creating EA is not supported for xine metadata ferris plugin."
           << " context:" << a->getURL()
           << " rdn:" << rdn
           << endl;
        Throw_FerrisCreateAttributeFailed( tostr(ss), 0 );
    }

    extern "C"
    {
        FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
        {
            LG_XINE_D << "CreateRealFactory()" << endl;
            return new EAGenerator_XineMetadata();
        }
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class Xine_Image : public Image
    {
        fh_XineStream xs;
    
        void readImageMetaData();
        void setupGamma()
            {}
        virtual void priv_ensureDataLoaded( LoadType loadType = LOAD_ONLY_METADATA );
    
    
    public:
        Xine_Image( fh_context c );
        ~Xine_Image();

    };

    Xine_Image::Xine_Image( fh_context c )
        :
        Image( fh_istream() )
    {
        xs = getXineStream( c );
        readImageMetaData();
    }

    Xine_Image::~Xine_Image()
    {
    }

    void
    Xine_Image::readImageMetaData()
    {
        uint32_t w = xs->getStreamInfo( XINE_STREAM_INFO_VIDEO_WIDTH );
        uint32_t h = xs->getStreamInfo( XINE_STREAM_INFO_VIDEO_HEIGHT );
        
        setWidth(  w );
        setHeight( h );
        setAlpha(  0 );

        setupGamma();
        setValid( true );
    }

    void
    Xine_Image::priv_ensureDataLoaded( LoadType loadType )
    {
        if( loadType == LOAD_COMPLETE_IMAGE )
        {
            stringstream ss;
            ss << "Xine plugin does not support serializing to single image";
            Throw_CanNotGetStream( ss.str(), 0 );
        }
    }
    extern "C"
    {

        fh_image CreateImageFromContext( const fh_context& c )
        {
            try
            {
                fh_image image = new Xine_Image( c );
                return image;
            }
            catch( exception& e )
            {
                LG_XINE_W << "Failed to load xine image EA, error:" << e.what() << endl;
            }
        }
    };
};

