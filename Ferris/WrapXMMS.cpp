/******************************************************************************
*******************************************************************************
*******************************************************************************

    Ego
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

    $Id: WrapXMMS.cpp,v 1.4 2010/09/24 21:31:01 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "WrapXMMS.hh"
#include <Ferris/Runner.hh>

#include <string>

using namespace std;

namespace Ferris
{

    void
    XMMS::ensureChildXMMSRunning()
    {
        if( theRunner )
            return;

        
        fh_runner r = new Runner();
        theRunner = r;

        r->setCommandLine( "ferris-internal-xmms-server -D -D" );
        r->setSpawnFlags(
            GSpawnFlags(
                G_SPAWN_SEARCH_PATH |
                G_SPAWN_STDERR_TO_DEV_NULL |
                G_SPAWN_DO_NOT_REAP_CHILD |
                r->getSpawnFlags()));
        r->setConnectStdIn( true );
        r->Run();

        
        toChildss   = r->getStdIn();
        fromChildss = r->getStdOut();
    }
    
    void
    XMMS::putCommand( const std::string& s )
    {
        ensureChildXMMSRunning();
        toChildss << s << endl << flush;
    }
    
    string
    XMMS::getReply()
    {
        string s;
        getline( fromChildss, s );
        return s;
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    XMMS::XMMS()
        :
        theRunner( 0 )
    {
    }
    

    XMMS::~XMMS()
    {
        if( theRunner )
            theRunner->killChild();
    }
    
    void XMMS::playlistPrev()
    {
        putCommand( "playlist-prev" );
        getReply();
    }
    
    void XMMS::playlistNext()
    {
        putCommand( "playlist-next" );
        getReply();
    }

    int
    XMMS::getPlaylistPosition()
    {
        putCommand( "playlist-get-position" );
        return toint(getReply());
    }
    
    void
    XMMS::setPlaylistPosition( int plpos )
    {
        putCommand( "playlist-set-position " + tostr(plpos) );
        getReply();
    }

    void
    XMMS::playlistEnqueue( const std::string& url )
    {
        putCommand( "playlist-enqueue " + url );
        getReply();
    }
    
    void
    XMMS::playlistAdd( const std::string& url )
    {
        putCommand( "playlist-add " + url );
        getReply();
    }
    
    void
    XMMS::playlistClear()
    {
        putCommand( "playlist-clear" );
        getReply();
    }

    int
    XMMS::getPlaylistLength()
    {
        putCommand( "playlist-get-length" );
        return toint(getReply());
    }

    stringlist_t
    XMMS::getPlaylistTitles()
    {
        stringlist_t ret;
        int len = getPlaylistLength();

        for( int i=0; i<len; ++i )
        {
            ret.push_back( getPlaylistTitle( i ) );
        }
        return ret;
    }
    stringlist_t
    XMMS::getPlaylistURLs()
    {
        stringlist_t ret;
        int len = getPlaylistLength();

        for( int i=0; i<len; ++i )
        {
            ret.push_back( getPlaylistURL( i ) );
        }
        return ret;
    }

    XMMS::PlaylistItemList_t
    XMMS::getPlaylist()
    {
        PlaylistItemList_t ret;
        int len = getPlaylistLength();

        for( int i=0; i<len; ++i )
        {
            string title = getPlaylistTitle( i );
            string url   = getPlaylistURL( i );
            time_t  tt   = getPlaylistTime( i );
            ret.push_back( PlaylistItem( title, url, tt, i ) );
        }
        return ret;
    }
    
    
    
    string
    XMMS::getPlaylistTitle( int pos )
    {
        putCommand( "playlist-nth-title " + tostr(pos) );
        return getReply();
    }
    string
    XMMS::getPlaylistURL( int pos )
    {
        putCommand( "playlist-nth-url " + tostr(pos) );
        return getReply();
    }
    time_t
    XMMS::getPlaylistTime( int pos )
    {
        putCommand( "playlist-nth-time " + tostr(pos) );
        return toType<time_t>(getReply());
    }
    

    
    
    int
    XMMS::getVolume()
    {
        putCommand( "volume-get" );
        return toint( getReply() );
    }
    
    void
    XMMS::setVolume( gint v )
    {
        putCommand( "volume-set " + tostr(v) );
        getReply();
    }
    
    void
    XMMS::adjustVolume( gint v )
    {
        putCommand( "volume-adjust " + tostr(v) );
        getReply();
    }

    bool
    XMMS::isPlaying()
    {
        putCommand( "is-playing" );
        return toint(getReply());
    }
    
    bool
    XMMS::isPaused()
    {
        putCommand( "is-paused" );
        return toint(getReply());
    }
    

    void
    XMMS::play()
    {
        putCommand( "play" );
        getReply();
    }
    
    void
    XMMS::pause()
    {
        putCommand( "pause" );
        getReply();
    }
    
    void
    XMMS::stop()
    {
        putCommand( "stop" );
        getReply();
    }

    void
    XMMS::playPause()
    {
        putCommand( "play-pause" );
        getReply();
    }
    
    
    string
    XMMS::getCurrentSongURL()
    {
        putCommand( "current-file" );
        return getReply();
    }
    
    string
    XMMS::getCurrentSongTitle()
    {
        putCommand( "current-title" );
        return getReply();
    }

    void
    XMMS::seek( int millis )
    {
        cerr << "seeking to offset:" << millis << endl;
        fh_stringstream ss;
        ss << "seek " << millis;
        putCommand( tostr(ss) );
        getReply();
    }

    void
    XMMS::seekAbsolute( double v )
    {
        cerr << "seeking to percent:" << v << endl;
        fh_stringstream ss;
        ss << "seek-absolute " << v;
        putCommand( tostr(ss) );
        getReply();
    }
    
    
    int
    XMMS::getCurrentTime()
    {
        putCommand( "current-time" );
        return toint(getReply());
    }
    
    int
    XMMS::getTotalTime()
    {
        putCommand( "current-total-time" );
        return toint(getReply());
    }
    
    
    

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

    fh_xmms getDefaultXMMS()
    {
        static fh_xmms x = 0;
        if( !x )
        {
            x = new XMMS();
        }
        return x;
    }

};
