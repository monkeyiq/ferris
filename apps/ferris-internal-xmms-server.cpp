/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris-internal-xmms-server -- imported from ego 11 nov 2006
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

    $Id: ferris-internal-xmms-server.cpp,v 1.4 2010/09/24 21:31:20 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <string>
#include <iostream>
#include <sstream>
#include <xmmsctrl.h>
#include <unistd.h>
#include <syslog.h>
#include <popt.h>
#include <stdlib.h>

using namespace std;

const string PROGRAM_NAME = "ferris-internal-xmms-server";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

gint session = 0;

bool starts_with( const string& s, const string& starting )
{
    return !s.compare( 0, starting.length(), starting );
}

void runCommand( const std::string& command )
{
    if( starts_with( command, "current-file" ))
    {
        gint plpos = xmms_remote_get_playlist_pos( session );
        gchar* fn  = xmms_remote_get_playlist_file( session, plpos );
        cout << fn << endl << flush;
    }
    else if( starts_with( command, "current-title" ))
    {
        gint plpos = xmms_remote_get_playlist_pos( session );
        gchar* fn  = xmms_remote_get_playlist_title( session, plpos );
        cout << (fn?fn:"") << endl << flush;
    }
    else if( starts_with( command, "playlist-nth-title" ))
    {
        stringstream ss(command);
        string cmd;
        int plpos = 0;
        ss >> cmd;
        ss >> plpos;
        gchar* fn  = xmms_remote_get_playlist_title( session, plpos );
        cout << (fn?fn:"") << endl << flush;
    }
    else if( starts_with( command, "playlist-nth-url" ))
    {
        stringstream ss(command);
        string cmd;
        int plpos = 0;
        ss >> cmd;
        ss >> plpos;
        gchar* fn  = xmms_remote_get_playlist_file( session, plpos );
        cout << (fn?fn:"") << endl << flush;
    }
    else if( starts_with( command, "playlist-nth-time" ))
    {
        stringstream ss(command);
        string cmd;
        int plpos = 0;
        ss >> cmd;
        ss >> plpos;
        gint tt  = xmms_remote_get_playlist_time( session, plpos );
        cout << tt << endl << flush;
    }
    else if( starts_with( command, "current-time" ))
    {
        gint p = xmms_remote_get_output_time( session );
        cout << p << endl << flush;
    }
    else if( starts_with( command, "current-total-time" ))
    {
        gint plpos = xmms_remote_get_playlist_pos(  session );
        gint p     = xmms_remote_get_playlist_time( session, plpos );
        cout << p << endl << flush;
    }
    else if( starts_with( command, "playlist-get-position" ))
    {
        int plpos = xmms_remote_get_playlist_pos( session );
        cout << plpos << endl << flush;
    }
    else if( starts_with( command, "playlist-get-length" ))
    {
        gint len = xmms_remote_get_playlist_length( session );
        cout << len << endl << flush;
    }
    else if( starts_with( command, "playlist-set-position" ))
    {
        stringstream ss(command);
        string cmd;
        int plpos = 0;
        ss >> cmd;
        ss >> plpos;
        
        xmms_remote_set_playlist_pos( session, plpos );
        cout << "OK" << endl << flush;
    }
    else if( starts_with( command, "playlist-prev" ))
    {
        xmms_remote_playlist_prev( session );
        cout << "OK" << endl << flush;
    }
    else if( starts_with( command, "playlist-next" ))
    {
        xmms_remote_playlist_next( session );
        cout << "OK" << endl << flush;
    }
    else if( starts_with( command, "playlist-clear" ))
    {
        xmms_remote_playlist_clear( session );
        cout << "OK" << endl << flush;
    }
    else if( starts_with( command, "playlist-add" ))
    {
        cerr << "playlist-add cmd!" << endl;
        
        stringstream ss(command);
        string cmd;
        string url;
        ss >> cmd;
        getline( ss, url );

        while( url.length() && url[0] == ' ' )
            url = url.substr( 1 );

        gint plpos = xmms_remote_get_playlist_pos( session );
        cerr << "adding:" << url << " at:" << plpos << endl;
        
        xmms_remote_playlist_ins_url_string( session, (gchar*)url.c_str(), plpos );
        xmms_remote_set_playlist_pos( session, plpos );
        cout << "OK" << endl << flush;
    }
    else if( starts_with( command, "playlist-enqueue" ))
    {
        stringstream ss(command);
        string cmd;
        string url;
        ss >> cmd;
        getline( ss, url );

        while( url.length() && url[0] == ' ' )
            url = url.substr( 1 );

        gint plpos = xmms_remote_get_playlist_pos( session );
        xmms_remote_playlist_ins_url_string( session, (gchar*)url.c_str(), plpos+1 );
        cout << "OK" << endl << flush;
    }
    else if( starts_with( command, "volume-adjust" ))
    {
        stringstream ss(command);
        string cmd;
        int offset = 0;
        ss >> cmd;
        ss >> offset;
        gint vl = 0;
        gint vr = 0;
        
        xmms_remote_get_volume( session, &vl, &vr );
        vl += offset;
        vr += offset;
        xmms_remote_set_volume( session, vl, vr);
        cout << "OK" << endl << flush;
    }
    else if( starts_with( command, "volume-set" ))
    {
        stringstream ss(command);
        string cmd;
        int v = 0;
        ss >> cmd;
        ss >> v;
        xmms_remote_set_volume( session, v, v );
        cout << "OK" << endl << flush;
    }
    else if( starts_with( command, "volume-get" ))
    {
        stringstream ss(command);
        string cmd;
        int offset = 0;
        ss >> cmd;
        ss >> offset;
        gint vl = 0;
        gint vr = 0;
        
        xmms_remote_get_volume( session, &vl, &vr );
        cout << vl << endl << flush;
    }
    else if( starts_with( command, "pause" ))
    {
        xmms_remote_pause( session );
        cout << "OK" << endl << flush;
    }
    else if( starts_with( command, "play-pause" ))
    {
        xmms_remote_play_pause( session );
        cout << "OK" << endl << flush;
    }
    else if( starts_with( command, "play" ))
    {
        xmms_remote_play( session );
        cout << "OK" << endl << flush;
    }
    else if( starts_with( command, "stop" ))
    {
        xmms_remote_stop( session );
        cout << "OK" << endl << flush;
    }
    else if( starts_with( command, "seek-absolute" ))
    {
        stringstream ss(command);
        string cmd;
        double v = 0;
        ss >> cmd;
        ss >> v;

        gint plpos = xmms_remote_get_playlist_pos(  session );
        gint p     = xmms_remote_get_playlist_time( session, plpos );
        xmms_remote_jump_to_time( session, (int)(v*p) );
        cout << "OK" << endl << flush;
    }
    else if( starts_with( command, "seek" ))
    {
        stringstream ss(command);
        string cmd;
        int len = 5;
        ss >> cmd;
        ss >> len;

        gint p = xmms_remote_get_output_time( session );
//        cerr << "current:" << p << " new:" << (p+len) << endl;
        xmms_remote_jump_to_time( session, p + len );
        cout << "OK" << endl << flush;
    }
    else if( starts_with( command, "is-playing" ))
    {
        gboolean ret = xmms_remote_is_playing( session );
        cout << ret << endl << flush;
    }
    else if( starts_with( command, "is-paused" ))
    {
        gboolean ret = xmms_remote_is_paused( session );
        cout << ret << endl << flush;
    }
    
    
    
}


int main( int argc, const char** argv )
{
    unsigned long logAllCommands = 0;
    const char*   foo_CSTR  = 0;

    struct poptOption optionsTable[] = {

        { "log-all", 'D', POPT_ARG_NONE, &logAllCommands, 0,
          "log all commands to syslog", "" },
        
        POPT_AUTOHELP
        POPT_TABLEEND
   };
    poptContext optCon;


    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]*  ...");

    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
    }
    
//     if (argc < 1 )
//     {
//         poptPrintUsage(optCon, stderr, 0);
//         exit(1);
//     }

    if( logAllCommands )
        syslog( LOG_USER|LOG_ERR, "starting up" );
    
    string s;
    while( true )
    {
        if( !getline( cin, s ) )
        {
            stringstream ss;
            ss << "Error getting info from client! exiting. got:" << s << endl;
            cerr << ss.str();
            syslog( LOG_USER|LOG_ERR, ss.str().c_str() );
            if( logAllCommands )
                syslog( LOG_USER|LOG_ERR, "closing down" );
            exit(1);
        }
        else
        {
            if( logAllCommands )
                syslog( LOG_USER|LOG_ERR, s.c_str() );
            runCommand( s );
        }
    }

    if( logAllCommands )
        syslog( LOG_USER|LOG_ERR, "closing down" );
    
}
