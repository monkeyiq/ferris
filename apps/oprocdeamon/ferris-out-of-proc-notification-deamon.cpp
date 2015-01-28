/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris-out-of-proc-notification-deamon
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

    $Id: ferris-out-of-proc-notification-deamon.cpp,v 1.6 2010/09/24 21:31:19 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
 * return 2 if cant create incoming fifo
 * return 3 if cant open   incoming fifo
 **/


#include <Ferris/Ferris.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/AsyncIO.hh>
#include <Ferris/Daemon.hh>
#include <Ferris/common-ferris-out-of-proc-notification-deamon.hh>
#include <Ferris/PluginOutOfProcNotificationEngine.hh>

#include <popt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

using namespace std;
using namespace Ferris;
using namespace Ferris::OProcMessage;

const string PROGRAM_NAME = "ferris-out-of-proc-notification-deamon";
string TargetDir = "";
unsigned long Verbose = 0;


void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void writePID( pid_t pid = 0 )
{
    stringstream ss;
    ss << pid;
    string path = TargetDir + "/pid";
    int fd = open( path.c_str(), O_WRONLY |  O_CREAT | O_TRUNC, 0777 );
    write( fd, ss.str().c_str(), ss.str().length() );
    close( fd );
    
//     fh_context c = Shell::acquireContext( TargetDir );
//     fh_context s = Shell::acquireSubContext( c, "pid" );
//     setStrAttr( s, "content", tostr( pid ) );
}

void sig_term_cb(int sign)
{
    try
    {
        LG_JOURNAL_W << "closing server at:" << getpid() << endl;
        writePID( 0 );
    }
    catch( ... )
    {}
    exit(0);
}

void sig_usr1_cb(int sign)
{
}

void sig_usr2_cb(int sign)
{
    /* ping */
    if( Verbose )
    {
        cerr << "got a ping! pid:" << getpid() << endl;
        LG_JOURNAL_W << "got a ping! pid:" << getpid() << endl;
    }
}

void
setupSignalHandlers()
{
    struct sigaction newinth;
    newinth.sa_handler = sig_term_cb;
    sigemptyset(&newinth.sa_mask);
    newinth.sa_flags   = SA_RESTART;
    if( -1 == sigaction( SIGTERM, &newinth, NULL))
    {
        cerr << "ERROR: signal handling is not active. " << endl;
        exit(2);
    }

    newinth.sa_handler = sig_usr1_cb;
    sigemptyset(&newinth.sa_mask);
    newinth.sa_flags   = SA_RESTART;
    if( -1 == sigaction( SIGUSR1, &newinth, NULL))
    {
        cerr << "ERROR: signal handling is not active. " << endl;
        exit(2);
    }

    newinth.sa_handler = sig_usr2_cb;
    sigemptyset(&newinth.sa_mask);
    newinth.sa_flags   = SA_RESTART;
    if( -1 == sigaction( SIGUSR2, &newinth, NULL))
    {
        cerr << "ERROR: signal handling is not active. " << endl;
        exit(2);
    }
}

static string  infifopath = "";
static string outfifopath = "";
static fh_context outfifoc = 0;

typedef map< string, int > outgoing_t;
outgoing_t outgoing;

void xml_msg_arrived( fh_xstreamcol h )
{
    cerr << "xml_msg_arrived(begin)" << endl;
    
    const std::string& msg = h->getXMLString();
    stringmap_t&         m = h->getStringMap();

    const string& pid = m[ KEY_OBAND_PID ];
    LG_JOURNAL_W << "ferris-oproc from pid:" << pid << " msg:" << msg << endl;

    cerr << "xml_msg_arrived(1)" << endl;
    
    /*
     * Check for new fifos
     */
    ImplicitIteratorUpdateLock ciLock;
    for( Context::iterator ci = outfifoc->begin(); ci != outfifoc->end(); ++ci )
    {
        string rdn  = (*ci)->getDirName();
        string path = (*ci)->getDirPath();

        if( outgoing.find( rdn ) != outgoing.end() )
            continue;
        
        cerr << "Discovered new outgoing pipe at:" << (*ci)->getURL() << endl;
        int fd = open( path.c_str(), O_RDWR | O_NONBLOCK );
        if( fd == -1 )
        {
            string es = errnum_to_string( "", errno );
            fh_stringstream ss;
            ss << "Can not open fifo for incomming requests at:" << path << endl
               << " reason:" << es << endl;
            LG_JOURNAL_ER << tostr(ss) << endl;
        }

        LG_JOURNAL_D << "Discovered new outgoing pipe at url:" << (*ci)->getURL()
                     << " path:" << path
                     << " rdn:" << rdn
                     << " fd:" << fd
                     << endl;
        
        cerr << "unlink1 :" << path << " rdn:" << rdn << endl;
        if( int r = unlink( path.c_str() ))
        {
            string es = errnum_to_string( "", errno );
            fh_stringstream ss;
            ss << "Can not remove opened fifo for incomming requests at:" << path << endl
               << " reason:" << es << endl;
            LG_JOURNAL_ER << tostr(ss) << endl;
            cerr << tostr(ss) << endl;
        }
        cerr << "unlink2 :" << path << " rdn:" << rdn << endl;
        outgoing[ rdn ] = fd;
        LG_JOURNAL_D << "unlinked new pipe from:" << path
                     << " rdn:" << rdn
                     << " fd:" << fd
                     << endl;
    }
    cerr << "xml_msg_arrived(2)" << endl;
    cerr << "xml_msg_arrived(3) msg:" << msg << endl;

    /*
     * Write to each proc, warn if one has died and remove it from write list.
     */
    for( outgoing_t::iterator fi = outgoing.begin(); fi != outgoing.end(); ++fi )
    {
        string rdn = fi->first;
        int    fd  = fi->second;
        
        LG_JOURNAL_D << "Sending notification rdn:" << rdn << " msg:" << msg << endl;
        
        if( rdn == pid )
        {
            LG_JOURNAL_D << "Detected that message came from pid:" << pid
                         << " not sending it back" << endl;
            continue;
        }

        cerr << "Sending notification rdn:" << rdn << " msg:" << msg << endl;

        ssize_t bwrite = write( fd, msg.data(), msg.length() );
        if( msg.length() != bwrite )
        {
            int en = errno;
            string es = errnum_to_string( "", errno );
            fh_stringstream ss;
            ss << "Error writing rdn:" << rdn << " msg:" << msg << " to proc:" << rdn
               << " bwrite:" << bwrite
               << " msg.length:" << msg.length()
               << " errno:" << en
               << " reason:" << es
               << endl;
            LG_JOURNAL_ER << tostr(ss) << endl;
        }
    }
    
    
// //     for( Context::iterator ci = outfifoc->begin(); ci != outfifoc->end(); ++ci )
// //     {
// //         string rdn = (*ci)->getDirName();
// //         if( rdn != pid )
// //         {
// //             if( Verbose )
// //                 LG_JOURNAL_W << "Sending pid:" << rdn << " msg:" << msg << endl;
// //             fh_iostream ioss = (*ci)->getIOStream();
// //             ioss << msg << flush;
// //         }
// //     }

    
    cerr << "xml_msg_arrived(end)" << endl;
}


int main( int argc, char** argv )
{
    Factory::setDontConnectWithFerrisOutOfProcDeamon( true );
    
    int exit_status = 0;
    
    try
    {
        unsigned long DontStartAsDaemon    = 0;
        unsigned long DontRecreateFIFO     = 0;
        unsigned long NiceValue            = 5;
        const char* TargetDirCSTR          = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "dont-daemon", 'D', POPT_ARG_NONE, &DontStartAsDaemon, 0,
                  "Dont detach as a daemon", "" },

                { "dont-recreate-fifo", 0, POPT_ARG_NONE, &DontRecreateFIFO, 0,
                  "Dont remake client to server fifo", "" },
                
                { "target-directory", 0, POPT_ARG_STRING, &TargetDirCSTR, 0,
                  "(NEEDED) Specify the root for the client to server communication fifos", "" },

                { "nice-value", 0, POPT_ARG_INT, &NiceValue, 0,
                  "Run deamon at nice value (default 5)", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* ");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}

        if (argc < 2 || !TargetDirCSTR )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
        
        {
            fh_context c = Shell::acquireContext( TargetDirCSTR );
            TargetDir = c->getDirPath();
        }

        string lockPath = TargetDir + "/" + "lockfile";
        ExitIfAlreadyRunning( lockPath );
        
        infifopath  = appendToServPath( TargetDir );
        outfifopath = appendFromServPrefix( TargetDir );

        int fd = -1;
        
        cerr << "infifopath:" << infifopath << endl;
        
        if( !DontStartAsDaemon )
        {
            SwitchToDaemonMode();
        }

        setupSignalHandlers();
        LG_JOURNAL_W << "server starting at pid:" << getpid() << endl;
        writePID( getpid() );
        LG_JOURNAL_W << "written PID pid:" << getpid() << endl;
        
        if( DontRecreateFIFO )
        {
            LG_JOURNAL_W << "opening read only incoming pipe at:" << infifopath << endl;
            fd = open( infifopath.c_str(), O_RDONLY );
            if( fd == -1 )
            {
                string es = errnum_to_string( "", errno );
                cerr << "Can not open fifo for incomming requests at:" << infifopath << endl
                     << " reason:" << es << endl
                     << "exiting" << endl;
                LG_JOURNAL_W << "Can not open fifo for incomming requests at:" << infifopath << endl
                             << " reason:" << es << endl
                             << "exiting" << endl;
                return( 3 );
            }
            close( fd );
            fd = open( infifopath.c_str(), O_RDWR | O_NONBLOCK );
            if( fd == -1 )
            {
                string es = errnum_to_string( "", errno );
                cerr << "Can not open fifo for incomming requests at:" << infifopath << endl
                     << " reason:" << es << endl
                     << "exiting" << endl;
                LG_JOURNAL_W << "Can not open fifo for incomming requests at:" << infifopath << endl
                             << " reason:" << es << endl
                             << "exiting" << endl;
                return( 3 );
            }
            LG_JOURNAL_W << "opened read only incoming pipe at:" << infifopath << endl;
        }
        else
        {
            fd = Factory::MakeFIFO( infifopath, true, O_RDWR | O_NONBLOCK );
        }
        outfifoc = Shell::acquireContext( outfifopath );
        
        nice( NiceValue );

        fh_aiohandler aio = new AsyncIOHandler( fd );
        fh_xstreamcol xs = Factory::MakeXMLStreamCol();
        xs->attach( aio );
        xs->getMessageArrivedSig().connect( sigc::ptr_fun( xml_msg_arrived ));
        
        setupSignalHandlers();
        LG_JOURNAL_W << "server starting at pid:" << getpid() << endl;
        writePID( getpid() );

        GMainContext* gmc = g_main_context_default();
        GMainLoop* gml    = g_main_loop_new( gmc, 0 );
        g_main_loop_run( gml );
        g_main_destroy( gml );        
    }
    catch( CreateFIFO& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(2);
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


