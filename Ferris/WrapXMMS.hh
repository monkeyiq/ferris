/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of libferris.

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

    $Id: WrapXMMS.hh,v 1.3 2010/09/24 21:31:01 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_WRAP_XMMS_H_
#define _ALREADY_INCLUDED_FERRIS_WRAP_XMMS_H_

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisHandle.hh>


namespace Ferris
{

    class XMMS
        :
        public Handlable
    {
        fh_runner  theRunner;
        fh_ostream toChildss;
        fh_istream fromChildss;

        void ensureChildXMMSRunning();
        void putCommand( const std::string& s );
        std::string getReply();
    public:
        XMMS();
        virtual ~XMMS();

        void playlistPrev();
        void playlistNext();

        int  getPlaylistPosition();
        void setPlaylistPosition( int plpos );
        
        void playlistAdd( const std::string& url );
        void playlistEnqueue( const std::string& url );
        void playlistClear();

        int getPlaylistLength();
        stringlist_t getPlaylistTitles();
        stringlist_t getPlaylistURLs();
        std::string getPlaylistTitle( int pos );
        std::string getPlaylistURL( int pos );
        time_t      getPlaylistTime( int pos );
        struct PlaylistItem
        {
            std::string title;
            std::string url;
            time_t tt;
            int pos;
            PlaylistItem( std::string title = "", std::string url = "", time_t tt = 0, int pos = 0 )
                :
                title( title ), url( url ), tt( tt ), pos( pos )
                {
                }
        };
        typedef std::list< PlaylistItem > PlaylistItemList_t;
        PlaylistItemList_t getPlaylist();
            
     
        int  getVolume();
        void setVolume( gint v );
        void adjustVolume( gint v );

        bool isPlaying();
        bool isPaused();
        void play();
        void pause();
        void stop();
        void playPause();
        std::string getCurrentSongURL();
        std::string getCurrentSongTitle();

        void seek( int millis );
        void seekAbsolute( double v );

        int getCurrentTime();
        int getTotalTime();
        
    };

    FERRIS_SMARTPTR( XMMS, fh_xmms );
    fh_xmms getDefaultXMMS();
};


#endif
