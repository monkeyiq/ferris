/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2005 Ben Martin

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

    $Id: libferrisxmms.cpp,v 1.5 2010/09/24 21:31:49 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/Trimming.hh>
#include <Ferris/General.hh>
#include <Ferris/Cache.hh>
//#include <Ferris/Runner.hh>
#include <Ferris/Medallion.hh>
#include <Ferris/SyncDelayer.hh>
#include <Ferris/WrapXMMS.hh>


#include <config.h>

using namespace std;

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    fh_xmms getXMMS()
    {
        return getDefaultXMMS();
    }
    
    
    
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    
    
    
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    typedef leafContextWithSimpleContent xmmsControlFileContext;
    
    
    
    class FERRISEXP_CTXPLUGIN xmmsControlVolumeContext
        :
        public xmmsControlFileContext
    {
        typedef xmmsControlFileContext _Base;
        typedef xmmsControlVolumeContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ss;
                ss << getXMMS()->getVolume();
                return ss;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
                gint v = toint( s );
                if( 0 <= v && v <= 100 )
                    getXMMS()->setVolume( v );
            }
    public:
        xmmsControlVolumeContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~xmmsControlVolumeContext()
            {
            }
    };

    class FERRISEXP_CTXPLUGIN xmmsControlPlayContext
        :
        public xmmsControlFileContext
    {
        typedef xmmsControlFileContext _Base;
        typedef xmmsControlPlayContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ss;
                ss << getXMMS()->isPlaying();
                return ss;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
//                cerr << "xmmsControlPlayContext::OnStreamClosed() s:" << s << endl;
                gint v = isTrue( s );
                if( v ) getXMMS()->play();
                else    getXMMS()->pause();
            }
    public:
        xmmsControlPlayContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~xmmsControlPlayContext()
            {
            }
    };

    class FERRISEXP_CTXPLUGIN xmmsControlPauseContext
        :
        public xmmsControlFileContext
    {
        typedef xmmsControlFileContext _Base;
        typedef xmmsControlPauseContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ss;
                ss << getXMMS()->isPaused();
                return ss;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
                gint v = isTrue( s );
                if( v ) getXMMS()->pause();
                else    getXMMS()->play();
            }
    public:
        xmmsControlPauseContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~xmmsControlPauseContext()
            {
            }
    };

    class FERRISEXP_CTXPLUGIN xmmsControlPlayPauseContext
        :
        public xmmsControlFileContext
    {
        typedef xmmsControlFileContext _Base;
        typedef xmmsControlPlayPauseContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
//                cerr << "get toggle play/pause is-playing:" << getXMMS()->isPlaying() << endl;
                fh_stringstream ss;
                ss << getXMMS()->isPlaying();
                return ss;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
                cerr << "toggle play/pause s:" << s << endl;
                getXMMS()->playPause();
            }
    public:
        xmmsControlPlayPauseContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~xmmsControlPlayPauseContext()
            {
            }
    };
    
    class FERRISEXP_CTXPLUGIN xmmsControlDirectoryContext
        :
        public StateLessEAHolder< xmmsControlDirectoryContext, FakeInternalContext >
    {
        typedef xmmsControlDirectoryContext _Self;
        typedef StateLessEAHolder< _Self, FakeInternalContext > _Base;

    protected:

        std::string
        priv_getRecommendedEA()
            {
                static string rea = "name,content";
                return rea;
            }
        
        void
        priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    fh_context wc = 0;
                    wc = new xmmsControlVolumeContext( this, "volume" );
                    Insert( GetImpl(wc) );
                    
                    wc = new xmmsControlPlayContext( this, "play" );
                    Insert( GetImpl(wc) );

                    wc = new xmmsControlPauseContext( this, "pause" );
                    Insert( GetImpl(wc) );

                    wc = new xmmsControlPlayPauseContext( this, "toggle-play-pause" );
                    Insert( GetImpl(wc) );
                }
            }
        
    public:

        xmmsControlDirectoryContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~xmmsControlDirectoryContext()
            {
            }
        
    };
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    class FERRISEXP_CTXPLUGIN xmmsCurrentURLContext
        :
        public xmmsControlFileContext
    {
        typedef xmmsControlFileContext _Base;
        typedef xmmsCurrentURLContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ss;
                ss << getXMMS()->getCurrentSongURL();
                return ss;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
            }
    public:
        xmmsCurrentURLContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~xmmsCurrentURLContext()
            {
            }
    };

    class FERRISEXP_CTXPLUGIN xmmsCurrentTitleContext
        :
        public xmmsControlFileContext
    {
        typedef xmmsControlFileContext _Base;
        typedef xmmsCurrentTitleContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ss;
                ss << getXMMS()->getCurrentSongTitle();
                return ss;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
            }
    public:
        xmmsCurrentTitleContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~xmmsCurrentTitleContext()
            {
            }
    };
    
    class FERRISEXP_CTXPLUGIN xmmsCurrentTimeContext
        :
        public xmmsControlFileContext
    {
        typedef xmmsControlFileContext _Base;
        typedef xmmsCurrentTimeContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ss;
                ss << getXMMS()->getCurrentTime();
                return ss;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
            }
    public:
        xmmsCurrentTimeContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~xmmsCurrentTimeContext()
            {
            }
    };
    
    class FERRISEXP_CTXPLUGIN xmmsCurrentTimeTotalContext
        :
        public xmmsControlFileContext
    {
        typedef xmmsControlFileContext _Base;
        typedef xmmsCurrentTimeTotalContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ss;
                ss << getXMMS()->getTotalTime();
                return ss;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
            }
    public:
        xmmsCurrentTimeTotalContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~xmmsCurrentTimeTotalContext()
            {
            }
    };
    
    class FERRISEXP_CTXPLUGIN xmmsCurrentPercentContext
        :
        public xmmsControlFileContext
    {
        typedef xmmsControlFileContext _Base;
        typedef xmmsCurrentPercentContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                double tt = getXMMS()->getTotalTime();
                double ct = getXMMS()->getCurrentTime();
                double ret = ct;
                if( tt )
                    ret /= tt;
                
                fh_stringstream ss;
                ss << ret;
                return ss;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
                double v = toType<double>(s);
                if( 0 <= v && v <=100 )
                    getXMMS()->seekAbsolute( v );
            }
    public:
        xmmsCurrentPercentContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~xmmsCurrentPercentContext()
            {
            }
    };
    

    
    class FERRISEXP_CTXPLUGIN xmmsCurrentTrackDirectoryContext
        :
        public StateLessEAHolder< xmmsCurrentTrackDirectoryContext, FakeInternalContext >
    {
        typedef xmmsCurrentTrackDirectoryContext _Self;
        typedef StateLessEAHolder< _Self, FakeInternalContext > _Base;

    protected:

        std::string
        priv_getRecommendedEA()
            {
                static string rea = "name,content";
                return rea;
            }
        
        void
        priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    fh_context wc = 0;
                    wc = new xmmsCurrentURLContext( this, "url" );
                    Insert( GetImpl(wc) );
                    
                    wc = new xmmsCurrentTitleContext( this, "title" );
                    Insert( GetImpl(wc) );

                    wc = new xmmsCurrentTimeContext( this, "time" );
                    Insert( GetImpl(wc) );

                    wc = new xmmsCurrentTimeTotalContext( this, "total-time" );
                    Insert( GetImpl(wc) );

                    wc = new xmmsCurrentPercentContext( this, "percent" );
                    Insert( GetImpl(wc) );
                }
            }
        
    public:

        xmmsCurrentTrackDirectoryContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~xmmsCurrentTrackDirectoryContext()
            {
            }
        
    };

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    class FERRISEXP_CTXPLUGIN xmmsPlaylistFileContext
        :
        public StateLessEAHolder< xmmsPlaylistFileContext, leafContext >
    {
        typedef xmmsPlaylistFileContext _Self;
        typedef StateLessEAHolder< _Self, leafContext > _Base;

        XMMS::PlaylistItem m_p;

    protected:

        virtual ferris_ios::openmode getSupportedOpenModes()
            {
                return
                    ios_base::in        |
                    ios_base::binary    ;
            }
        
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
//                cerr << "EARL:" << m_p.url << endl;
                fh_context c = Resolve( m_p.url );
                return c->getIStream(m);
            }

        
        static fh_stringstream
        SL_getTitle( xmmsPlaylistFileContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_p.title;
                return ss;
            }

        static fh_stringstream
        SL_getURL( xmmsPlaylistFileContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_p.url;
                return ss;
            }
        
        static fh_stringstream
        SL_getTT( xmmsPlaylistFileContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_p.tt;
                return ss;
            }

        static fh_stringstream
        SL_getTTStr( xmmsPlaylistFileContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << Time::toHMSString( c->m_p.tt/1000, false );
                return ss;
            }
        
        static fh_stringstream
        SL_getPos( xmmsPlaylistFileContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_p.pos;
                return ss;
            }

        std::string
        priv_getRecommendedEA()
            {
                static string rea = "name,playtime-display,title";
                return rea;
            }
        
        
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
#define SLEA  tryAddStateLessAttribute

                    SLEA( "title",    SL_getTitle, XSD_BASIC_STRING );
//                    SLEA( "url",      SL_getURL, XSD_BASIC_STRING );
                    SLEA( "ferris-delegate-url", SL_getURL, XSD_BASIC_STRING );
                    SLEA( "link-target", SL_getURL, XSD_BASIC_STRING );
                    SLEA( "playtime", SL_getTT, FXD_UNIXEPOCH_T );
                    SLEA( "length",   SL_getTT, FXD_UNIXEPOCH_T );
                    SLEA( "playtime-display", SL_getTTStr, XSD_BASIC_STRING );
                    SLEA( "length-display", SL_getTTStr, XSD_BASIC_STRING );
                    
                    SLEA( "pos",      SL_getPos, XSD_BASIC_INT );
                    SLEA( "position", SL_getPos, XSD_BASIC_INT );

#undef SLEA
                }
            }
        
    public:

        xmmsPlaylistFileContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~xmmsPlaylistFileContext()
            {
            }

        void constructObject( XMMS::PlaylistItem& p )
            {
                m_p = p;
            }
    };

    class FERRISEXP_CTXPLUGIN xmmsPlaylistDirectoryContext
        :
        public StateLessEAHolder< xmmsPlaylistDirectoryContext, FakeInternalContext >
    {
        typedef xmmsPlaylistDirectoryContext _Self;
        typedef StateLessEAHolder< _Self, FakeInternalContext > _Base;

        fh_xmms getXMMS()
            {
                return getDefaultXMMS();
            }
        
    protected:

        void
        priv_read()
            {
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                LG_CTX_D << "priv_read()" << endl;
                clearContext();

                typedef XMMS::PlaylistItemList_t PlaylistItemList_t;
                typedef PlaylistItemList_t::iterator ITER;
                PlaylistItemList_t pl = getXMMS()->getPlaylist();

                for( ITER pi=pl.begin(); pi!=pl.end(); ++pi )
                {
                    stringstream rdnss;
                    rdnss << pi->pos;
                    xmmsPlaylistFileContext* c = 0;
                    c = priv_ensureSubContext( rdnss.str(), c );
                    c->constructObject( *pi );
                }
            }
        
    public:

        xmmsPlaylistDirectoryContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~xmmsPlaylistDirectoryContext()
            {
            }
        
    };
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
    
    class FERRISEXP_CTXPLUGIN xmmsRootContext
        :
        public StateLessEAHolder< xmmsRootContext, FakeInternalContext >
    {
        typedef xmmsRootContext                           _Self;
        typedef StateLessEAHolder< xmmsRootContext, FakeInternalContext > _Base;

    protected:

        void
        priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    fh_context dc = 0;
                    
                    dc = new xmmsControlDirectoryContext( this, "control" );
                    Insert( GetImpl(dc) );

                    dc = new xmmsCurrentTrackDirectoryContext( this, "current" );
                    Insert( GetImpl(dc) );

                    dc = new xmmsPlaylistDirectoryContext( this, "playlist" );
                    Insert( GetImpl(dc) );
                    
                    
                }
            }

        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        
    public:

        xmmsRootContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~xmmsRootContext()
            {
            }
        virtual std::string priv_getRecommendedEA()
            {
                return "name";
            }
        virtual std::string getRecommendedEA()
            {
                return "name";
            }
        

        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
        xmmsRootContext* priv_CreateContext( Context* parent, string rdn )
            {
                xmmsRootContext* ret = new xmmsRootContext();
                ret->setContext( parent, rdn );
                return ret;
            }
        
        
    };







    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                static xmmsRootContext c;
                fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
                return ret;
            }
            catch( exception& e )
            {
                LG_XSLTFS_ER << "Brew() e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

};
