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

    $Id: libferrisamarok.cpp,v 1.5 2010/09/24 21:31:32 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/Trimming.hh>
#include <Ferris/General.hh>
#include <Ferris/Cache.hh>
#include <Ferris/Runner.hh>
#include <Ferris/Medallion.hh>
#include <Ferris/SyncDelayer.hh>
#include <Ferris/FerrisQt_private.hh>

#include <QtDBus>
#include <QDBusArgument>

#include <config.h>

#include "player_interface.h"
#include "tracklist_interface.h"

using namespace std;

// Marshall the DBusStatus data into a D-BUS argument
    QDBusArgument &operator<<(QDBusArgument &argument, const DBusStatus &status)
    {
        argument.beginStructure();
        argument << status.Play;
        argument << status.Random;
        argument << status.Repeat;
        argument << status.RepeatPlaylist;
        argument.endStructure();
        return argument;
    }

// Retrieve the DBusStatus data from the D-BUS argument
    const QDBusArgument &operator>>(const QDBusArgument &argument, DBusStatus &status)
    {
        argument.beginStructure();
        argument >> status.Play;
        argument >> status.Random;
        argument >> status.Repeat;
        argument >> status.RepeatPlaylist;
        argument.endStructure();
        return argument;
    }
    

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf );
    };

    player* getPlayer()
    {
        static player* ret = 0;
        if( !ret )
        {
            ret = new player( "org.kde.amarok", "/Player",
                              QDBusConnection::sessionBus(), 0 );
        }
        return ret;
    }

// struct DBusStatus
// {
//     int Play; //Playing = 0, Paused = 1, Stopped = 2
//     int Random; //Linearly = 0, Randomly = 1
//     int Repeat; //Go_To_Next = 0, Repeat_Current = 1
//     int RepeatPlaylist; //Stop_When_Finished = 0, Never_Give_Up_Playing = 1
// };
    
    bool isPlaying( player* p )
    {
        qDBusRegisterMetaType<DBusStatus>();
        
        DBusStatus st = getPlayer()->GetStatus();
        return st.Play == 0;
    }

    string getMetadata( const std::string& k )
    {
        QVariantMap m = getPlayer()->GetMetadata();
        QVariant v = m[k.c_str()];
        return tostr(v.toString());
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
    /******************************************************************************/
    

    typedef leafContextWithSimpleContent amarokControlFileContext;
    
    
    class FERRISEXP_CTXPLUGIN amarokControlVolumeContext
        :
        public amarokControlFileContext
    {
        typedef amarokControlFileContext _Base;
        typedef amarokControlVolumeContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ret;
                ret << getPlayer()->VolumeGet();
                return ret;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
                gint v = toint( s );
                if( 0 <= v && v <= 100 )
                {
                    int vv = (int)v;
                    getPlayer()->VolumeSet( vv );
                }
            }
    public:
        amarokControlVolumeContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~amarokControlVolumeContext()
            {
            }
    };

    class FERRISEXP_CTXPLUGIN amarokControlPlayContext
        :
        public amarokControlFileContext
    {
        typedef amarokControlFileContext _Base;
        typedef amarokControlPlayContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ret;
                ret << isPlaying( getPlayer() );
                return ret;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
                cerr << "xmmsControlPlayContext::OnStreamClosed() s:" << s << endl;
                gint v = isTrue( s );
                if( v ) getPlayer()->Play();
                else    getPlayer()->Pause();
            }
    public:
        amarokControlPlayContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~amarokControlPlayContext()
            {
            }
    };

    class FERRISEXP_CTXPLUGIN amarokControlPauseContext
        :
        public amarokControlFileContext
    {
        typedef amarokControlFileContext _Base;
        typedef amarokControlPauseContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ss;
                ss << !isPlaying(getPlayer());
                return ss;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
                gint v = isTrue( s );
                if( !v ) getPlayer()->Play();
                else     getPlayer()->Pause();
            }
    public:
        amarokControlPauseContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~amarokControlPauseContext()
            {
            }
    };

    class FERRISEXP_CTXPLUGIN amarokControlPlayPauseContext
        :
        public amarokControlFileContext
    {
        typedef amarokControlFileContext _Base;
        typedef amarokControlPlayPauseContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ret;
                ret << isPlaying(getPlayer());
                return ret;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
                gint v = isPlaying(getPlayer());
                if( !v ) getPlayer()->Play();
                else     getPlayer()->Pause();
            }
    public:
        amarokControlPlayPauseContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~amarokControlPlayPauseContext()
            {}
    };
    
    class FERRISEXP_CTXPLUGIN amarokControlDirectoryContext
        :
        public StateLessEAHolder< amarokControlDirectoryContext, FakeInternalContext >
    {
        typedef amarokControlDirectoryContext _Self;
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
                    wc = new amarokControlVolumeContext( this, "volume" );
                    Insert( GetImpl(wc) );
                    
                    wc = new amarokControlPlayContext( this, "play" );
                    Insert( GetImpl(wc) );

                    wc = new amarokControlPauseContext( this, "pause" );
                    Insert( GetImpl(wc) );

                    wc = new amarokControlPlayPauseContext( this, "toggle-play-pause" );
                    Insert( GetImpl(wc) );
                }
            }
        
    public:

        amarokControlDirectoryContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~amarokControlDirectoryContext()
            {
            }
        
    };
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    class FERRISEXP_CTXPLUGIN amarokCurrentURLContext
        :
        public amarokControlFileContext
    {
        typedef amarokControlFileContext _Base;
        typedef amarokCurrentURLContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ss;
                ss << getMetadata( "location" );
                return ss;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
            }
    public:
        amarokCurrentURLContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~amarokCurrentURLContext()
            {
            }
    };

    class FERRISEXP_CTXPLUGIN amarokCurrentTitleContext
        :
        public amarokControlFileContext
    {
        typedef amarokControlFileContext _Base;
        typedef amarokCurrentTitleContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ss;
                ss << getMetadata( "title" );
                return ss;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
            }
    public:
        amarokCurrentTitleContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~amarokCurrentTitleContext()
            {
            }
    };
    
    class FERRISEXP_CTXPLUGIN amarokCurrentTimeContext
        :
        public amarokControlFileContext
    {
        typedef amarokControlFileContext _Base;
        typedef amarokCurrentTimeContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ss;
//                ss << (int)getPlayer().call("trackCurrentTimeMs()");
                return ss;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
            }
    public:
        amarokCurrentTimeContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~amarokCurrentTimeContext()
            {
            }
    };
    
    class FERRISEXP_CTXPLUGIN amarokCurrentTimeTotalContext
        :
        public amarokControlFileContext
    {
        typedef amarokControlFileContext _Base;
        typedef amarokCurrentTimeTotalContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ss;
//                ss << (((int)getPlayer().call("trackTotalTime()"))*1000);
                return ss;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
            }
    public:
        amarokCurrentTimeTotalContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~amarokCurrentTimeTotalContext()
            {
            }
    };
    
    class FERRISEXP_CTXPLUGIN amarokCurrentPercentContext
        :
        public amarokControlFileContext
    {
        typedef amarokControlFileContext _Base;
        typedef amarokCurrentPercentContext _Self;

        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m )
            {
                // double ct = (int)getPlayer().call("trackCurrentTimeMs()");
                // double tt = ((int)getPlayer().call("trackTotalTime()"))*1000;
                // double ret = ct;
                // if( tt )
                //     ret /= tt;
                
                fh_stringstream ss;
//                ss << ret;
                return ss;
            }

        virtual void OnStreamClosed( const std::string& s )
            {
                double v = toType<double>(s);
                if( 0 <= v && v <=100 )
                {
//                    double tt = (int)getPlayer().call("trackTotalTime()");
//                    int seekt = (int)(tt * v);
//                    getPlayer().call("seek(int)", seekt );
                }
            }
    public:
        amarokCurrentPercentContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~amarokCurrentPercentContext()
            {
            }
    };
    

    
    class FERRISEXP_CTXPLUGIN amarokCurrentTrackDirectoryContext
        :
        public StateLessEAHolder< amarokCurrentTrackDirectoryContext, FakeInternalContext >
    {
        typedef amarokCurrentTrackDirectoryContext _Self;
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
                    wc = new amarokCurrentURLContext( this, "url" );
                    Insert( GetImpl(wc) );
                    
                    wc = new amarokCurrentTitleContext( this, "title" );
                    Insert( GetImpl(wc) );

                    wc = new amarokCurrentTimeContext( this, "time" );
                    Insert( GetImpl(wc) );

                    wc = new amarokCurrentTimeTotalContext( this, "total-time" );
                    Insert( GetImpl(wc) );

                    wc = new amarokCurrentPercentContext( this, "percent" );
                    Insert( GetImpl(wc) );
                }
            }
        
    public:

        amarokCurrentTrackDirectoryContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~amarokCurrentTrackDirectoryContext()
            {
            }
        
    };

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    class FERRISEXP_CTXPLUGIN amarokPlaylistFileContext
        :
        public StateLessEAHolder< amarokPlaylistFileContext, leafContext >
    {
        typedef amarokPlaylistFileContext _Self;
        typedef StateLessEAHolder< _Self, leafContext > _Base;

        stringmap_t m;

    protected:

        virtual ferris_ios::openmode getSupportedOpenModes()
            {
                return
                    ios_base::in        |
                    ios_base::binary    ;
            }
        
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            {
//                cerr << "EARL:" << this->m["url"] << endl;
                fh_context c = Resolve( this->m["url"] );
                return c->getIStream(m);
            }
        
        static fh_stringstream
        SL_getTitle( amarokPlaylistFileContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m["title"];
                return ss;
            }

        static fh_stringstream
        SL_getArtist( amarokPlaylistFileContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m["artist"];
                return ss;
            }

        static fh_stringstream
        SL_getAlbum( amarokPlaylistFileContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m["album"];
                return ss;
            }
        
        static fh_stringstream
        SL_getURL( amarokPlaylistFileContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m["url"];
                return ss;
            }
        
        static fh_stringstream
        SL_getTT( amarokPlaylistFileContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m["tt"];
                return ss;
            }

        static fh_stringstream
        SL_getTTStr( amarokPlaylistFileContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << Time::toHMSString( toint(c->m["tt"]), false );
                return ss;
            }
        
        static fh_stringstream
        SL_getPos( amarokPlaylistFileContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m["pos"];
                return ss;
            }

        std::string
        priv_getRecommendedEA()
            {
                static string rea = "name,playtime-display,artist,album,title";
                return rea;
            }
        
        
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
//                    cerr << "createStateLessAttributes(XX)" << endl;
#define SLEA  tryAddStateLessAttribute

                    SLEA( "title",    SL_getTitle, XSD_BASIC_STRING );
                    SLEA( "artist",   SL_getArtist, XSD_BASIC_STRING );
                    SLEA( "album",    SL_getAlbum, XSD_BASIC_STRING );
//                    SLEA( "url",      SL_getURL, XSD_BASIC_STRING );
                    SLEA( "link-target",      SL_getURL, XSD_BASIC_STRING );
                    SLEA( "ferris-delegate-url", SL_getURL, XSD_BASIC_STRING );
                    SLEA( "playtime", SL_getTT, FXD_UNIXEPOCH_T );
                    SLEA( "length",   SL_getTT, FXD_UNIXEPOCH_T );
                    SLEA( "playtime-display", SL_getTTStr, XSD_BASIC_STRING );
                    SLEA( "length-display", SL_getTTStr, XSD_BASIC_STRING );
                    
                    SLEA( "pos",      SL_getPos, XSD_BASIC_INT );
                    SLEA( "position", SL_getPos, XSD_BASIC_INT );

#undef SLEA
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        
    public:

        amarokPlaylistFileContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }
        virtual ~amarokPlaylistFileContext()
            {
            }

        void constructObject( const stringmap_t& sm )
            {
                m = sm;
            }
    };

    class FERRISEXP_CTXPLUGIN amarokPlaylistDirectoryContext
        :
        public StateLessEAHolder< amarokPlaylistDirectoryContext, FakeInternalContext >
    {
        typedef amarokPlaylistDirectoryContext _Self;
        typedef StateLessEAHolder< _Self, FakeInternalContext > _Base;

        
    protected:

        void
        priv_read()
            {
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                LG_AMAROK_D << "priv_read()" << endl;
                clearContext();

                if( getSubContextCount() )
                    return;
                

                tracklist* tl = new tracklist( "org.kde.amarok", "/TrackList",
                                               QDBusConnection::sessionBus(), 0 );
                long maxpos = tl->GetLength();
                LG_AMAROK_D << "maxpos:" << maxpos << endl;
                for( int pos=0; pos < maxpos; ++pos )
                {
                    QVariantMap m = tl->GetMetadata( pos );

                    stringmap_t sm;
                    stringstream rdnss;
                    rdnss << pos;

                    string tt    = tostr( m["time"].toInt() );
                    string earl  = tostr( m["location"].toString() );
                    sm["title"]  = tostr( m["title"].toString() );
                    sm["album"]  = tostr( m["album"].toString() );
                    sm["artist"] = tostr( m["artist"].toString() );
                    sm["url"]    = Util::URLDecode( earl );
                    sm["tt" ]    = tt;
                    sm["size"]   = getStrAttr(earl,"size","0");
                    sm["pos"]    = tostr(pos);
                    LG_AMAROK_D << "pos:" << pos << " earl:" << earl << endl;
                    LG_AMAROK_D << "tt:" << tt << endl;
                    
                    amarokPlaylistFileContext* c = 0;
                    c = priv_ensureSubContext( rdnss.str(), c );
                    c->constructObject( sm );
                }
            }
        
    public:

        amarokPlaylistDirectoryContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~amarokPlaylistDirectoryContext()
            {
            }
        
    };
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
    
    class FERRISEXP_CTXPLUGIN amarokRootContext
        :
        public StateLessEAHolder< amarokRootContext, FakeInternalContext >
    {
        typedef amarokRootContext                           _Self;
        typedef StateLessEAHolder< amarokRootContext, FakeInternalContext > _Base;

    protected:

        void
        priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    fh_context dc = 0;
                    
                    dc = new amarokControlDirectoryContext( this, "control" );
                    Insert( GetImpl(dc) );

                    dc = new amarokCurrentTrackDirectoryContext( this, "current" );
                    Insert( GetImpl(dc) );

                    dc = new amarokPlaylistDirectoryContext( this, "playlist" );
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

        amarokRootContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~amarokRootContext()
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
        

        friend fh_context Brew( RootContextFactory* rf );
        amarokRootContext* priv_CreateContext( Context* parent, string rdn )
            {
                amarokRootContext* ret = new amarokRootContext();
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
        {
            try
            {
                static amarokRootContext c;
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
