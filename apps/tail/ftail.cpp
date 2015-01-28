/******************************************************************************
*******************************************************************************
*******************************************************************************

    ftail
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

    $Id: ftail.cpp,v 1.3 2010/09/24 21:31:20 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
 * 
 *
 *
 *
 *
*/


#include <Ferris.hh>
#include <Ferris_private.hh>
#include <popt.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#ifdef STLPORT
#else
#include <ext/algorithm>
using namespace __gnu_cxx;
#endif

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ftail";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

unsigned long Verbose              = 0;
unsigned long Quiet                = 0;
long LastBytes           = 0;
long LastLines           = 10;
long FollowRetryOpen     = 0;
long FollowNameRetry     = 0;
long FollowUnchangedStats = 0;
long FollowTerminateAfterPIDDies = 0;
double FollowSleepInterval = 1.0;

long Follow = 0;
//const char* Follow = 0;

bool readLastNLines( fh_istream iss, fh_ostream oss, streamsize filesz, int N )
{
//    cerr << "readLastNLines() n:" << N << endl;
    
    string s;
    stringlist_t buffer;
    int sz = 0;
    while( getline( iss,s ) )
    {
        buffer.push_back( s );

        if( sz < N )
            ++sz;
        else
        {
            buffer.pop_front();
        }
    }
    for( stringlist_t::const_iterator bi = buffer.begin(); bi!=buffer.end(); ++bi )
    {
        oss << *bi << nl;
    }
    oss << flush;
    return true;
}


bool readLastNBytes( fh_istream iss, fh_ostream oss, streamsize filesz, int N )
{
    char ch;
    
    if( filesz > 0 )
    {
        iss.seekg( filesz - N );
        for( int i = 0; i < N; ++i )
        {
            if( !(iss >> noskipws >> ch) )  break;
            if( !(oss << ch) )  exit( 1 );
        }
        oss << flush;
        return true;
    }
    
    typedef list< char > buffer_t;
    buffer_t buffer;
    for( int i = 0; i < N; ++i )
    {
        if( !(iss >> noskipws >> ch) )
            break;

        buffer.push_back( ch );
    }
    if( buffer.size() < N )
        return false;
                    
    while( iss >> noskipws >> ch )
    {
        buffer.push_back( ch );
        buffer.pop_front();
    }

    for( buffer_t::const_iterator bi = buffer.begin(); bi!=buffer.end(); ++bi )
    {
        oss << *bi;
    }
    oss << flush;
    return true;
}

struct FileInfo
{
    fh_context c;
    fh_istream iss;
    fh_ostream oss;
    bool ignore;
    streamsize sz;
    int n_unchanged_stats;
    int device;
    int inode;
    string earl;
    
    FileInfo( fh_context c,
              fh_istream iss,
              fh_ostream oss,
              const string& earl = "" )
        :
        c( c ),
        iss( iss ),
        oss( oss ),
        earl( earl ),
        ignore( false ),
        sz( 0 ),
        n_unchanged_stats( 0 ),
        device( 0 ),
        inode( 0 )
        {
            if( c )
            {
                device = toint( getStrAttr( c, "device", "0" ));
                inode  = toint( getStrAttr( c, "inode", "0" ));
                this->earl   = c->getURL();
            }
        }

    void recheck()
        {
            int ndevice = toint( getStrAttr( c, "device", "0" ));
            int ninode  = toint( getStrAttr( c, "inode", "0" ));

            if( !ndevice || !ninode )
            {
//                cerr << "file removed:" << c->getURL() << endl;
                device = ndevice;
                inode  = ninode;
            }
            
            if( device != ndevice || inode != ninode )
            {
//                cerr << "recheck reopening file" << endl;
                device = ndevice;
                inode  = ninode;
                c      = Resolve( c->getURL() );
                iss    = c->getIStream(); // ios::binary | ios::ate );
                sz     = 0;
//                cerr << "file created:" << c->getURL() << endl;
            }
            n_unchanged_stats = 0;
        }

    void FollowFile( FileInfo*& lastf, bool& any_changes )
        {
            if( FollowRetryOpen && !c )
            {
                try
                {
                    fh_context c   = Resolve( earl );
                    fh_istream iss = c->getIStream();
                    streamsize filesz = toType<streamsize>(getStrAttr( c, "size", "", true, true ));
                    c = c;
                    iss = iss;
                    sz = 0;
                }
                catch( exception& e )
                {}
            }
            
            if( ignore || !c )
                return;

            streamsize nsz = toType<streamsize>( getStrAttr( c, "size", "0" ) );
//            cerr << "c:" << c->getURL() << " nsz:" << nsz << " sz:" << sz  << endl;
            if( nsz == sz )
            {
                n_unchanged_stats++;
                if( FollowUnchangedStats < n_unchanged_stats )
                {
//                    if( !strcmp( Follow, "name") )
                    {
                        recheck();
                    }
                }
                return;
            }

            any_changes = true;
            n_unchanged_stats = 0;

            if( nsz < sz )
            {
                cerr << "file truncated:" << c->getURL() << endl;
                iss.clear();
                iss.seekg( nsz );
                sz = nsz;
                return;
            }

            if( this != lastf )
            {
                oss << "==> " << c->getURL() << "<==" << endl;
            }

//            cerr << "Copying chars iss->oss count:" << (nsz - sz ) << endl;
            iss.clear();
            char ch;
            int count = nsz - sz;
            for( int i = 0; i < count; ++i )
            {
                if( iss >> noskipws >> ch )
                    oss << ch;
            }
            oss << flush;
            lastf = this;
            sz = nsz;
        }
    
};
typedef list< FileInfo* > FileInfoList_t;
FileInfoList_t FileInfoList;

FileInfo* lastf = 0;
bool any_changes = false;




void FollowLoop()
{
    
    while( true )
    {
//        cerr << "Follow loop...FileInfoList.sz:" << FileInfoList.size() << endl;
        any_changes = false;

        bool PIDHasExited = (FollowTerminateAfterPIDDies != 0
                             && kill (FollowTerminateAfterPIDDies, 0) != 0
                             && errno != EPERM);
        
        for( FileInfoList_t::iterator fi = FileInfoList.begin();
             fi != FileInfoList.end(); ++fi )
        {
            FileInfo* f = *fi;

            f->FollowFile( lastf, any_changes );
        }

        Main::processAllPending_VFSFD_Events();
        Main::processAllPendingEvents();
        
        if( !any_changes )
        {
            Time::Sleep( FollowSleepInterval );
        }

        if( PIDHasExited )
            exit( 0 );
    }
}


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "quiet", 'q', POPT_ARG_NONE, &Quiet, 0,
                  "never print headers giving file names", "" },
                { "silent", 0, POPT_ARG_NONE, &Quiet, 0,
                  "never print headers giving file names", "" },

                { "bytes", 'c', POPT_ARG_INT, &LastBytes, 0,
                  "output the last N bytes.", "" },

                { "lines", 'n', POPT_ARG_INT, &LastLines, 0,
                  "output the last N lines, instead of the last 10", "" },

                { "retry", 0, POPT_ARG_INT, &FollowRetryOpen, 0,
                  "keep trying to open a file even if it is inaccessible (implies -f)", "" },

                { "follow", 'f', POPT_ARG_NONE, &Follow, 0,
                  "output appended data as the file grows", "" },

                { 0, 'F', POPT_ARG_NONE, &FollowNameRetry, 0,
                  "same as --follow=name --retry", "" },
                
                { "max-unchanged-stats", 0, POPT_ARG_INT, &FollowUnchangedStats, 0,
                  "with  --follow=name,  reopen  a  FILE which has not changed size after N (default 5) iterations", "" },
                
                { "pid", 0, POPT_ARG_INT, &FollowTerminateAfterPIDDies, 0,
                  "with -f, terminate after process ID, PID dies", "" },

                { "sleep-interval", 's', POPT_ARG_DOUBLE, &FollowSleepInterval, 0,
                  "with -f, sleep for approximately S seconds (default 1.0) between iterations.", "" },

                // FIXME: follow with inotify option?

                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* src1 src2 ...");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {
//         switch (c) {
//         }
        }

        if (argc < 1)
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
        if( FollowNameRetry )
        {
            FollowRetryOpen=1;
            Follow=1;
        }

        stringlist_t srcs;
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            string srcURL = tmpCSTR;
            srcs.push_back( srcURL );
        }
        if( srcs.empty() )
        {
            srcs.push_back("-");
        }

        int srcs_sz = srcs.size();
        fh_ostream oss = Factory::MakeFdOStream( STDOUT_FILENO );
        for( stringlist_t::const_iterator si = srcs.begin(); si != srcs.end(); ++si )
        {
            string earl = *si;

            if( srcs_sz > 1 && !Quiet )
            {
                oss << "==> " << earl << " <==" << endl;
            }

            fh_context c = 0;
            streamsize filesz = -1;
            fh_istream iss;
            if( earl == "-" )
            {
                iss = Factory::MakeFdIStream( STDIN_FILENO );
            }
            else
            {
                try
                {
                    c = Resolve( earl );
                    iss = c->getIStream();
                    filesz = toType<streamsize>(getStrAttr( c, "size", "", true, true ));
                }
                catch( NoSuchSubContext& e )
                {
                    if( !FollowRetryOpen )
                    {
                        cerr << "Failed to open file:" << earl << endl;
                        continue;
                    }
                    cerr << "Adding later earl:" << earl << endl;
                    FileInfoList.push_back( new FileInfo( 0, iss, oss, earl ) );
                }
                catch( exception& e )
                {
                    if( !FollowRetryOpen )
                        throw;
                    cerr << "Adding later earl:" << earl << endl;
                    FileInfoList.push_back( new FileInfo( 0, iss, oss, earl ) );
                }
            }

            if( LastBytes )
            {
                readLastNBytes( iss, oss, filesz, LastBytes );
            }
            else if( LastLines != 0 )
            {
                readLastNLines( iss, oss, filesz, LastLines );
            }

            if( Follow && c )
            {
                FileInfoList.push_back( new FileInfo( c, iss, oss ) );
            }
        }

        lastf = FileInfoList.front();
        
        //
        // Main -f loop
        //
        if( Follow && !FileInfoList.empty() )
        {
            FollowLoop();
        }
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


