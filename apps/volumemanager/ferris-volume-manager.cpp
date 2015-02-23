/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris volume manager
    Copyright (C) 2008 Ben Martin

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

    $Id: ls.cpp,v 1.12 2008/04/27 21:30:11 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <config.h>

#include <string>
#include <list>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <Ferris.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/FilteredContext.hh>
#include <Ferris/FerrisQt_private.hh>
#include <Ferris/Runner.hh>

#include <popt.h>
#include <signal.h>
#include <unistd.h>


#include "DBusGlue/com_libferris_Volume_Manager_adaptor.h"
#include "DBusGlue/com_libferris_Volume_Manager_adaptor.cpp"

#ifdef OSX
#define MAJOR(x) x 
#define MINOR(x) x
#else
#include <linux/kdev_t.h>
#endif

using namespace std;
using namespace Ferris;

#define DEBUG 0

const string PROGRAM_NAME = "ferris-volume-manager";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

int RETURN_CODE = 0;
bool running = true;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static const char* DBUS_SERVER_NAME = "com.libferris.Volume.Manager";
static const char* DBUS_SERVER_PATH = "/com/libferris/Volume/Manager";

fh_context volumectx = 0;
fh_context usbctx = 0;

static string HAL_USERDB   = "~/.ferris/hal-user-configuration.db/";
static string HAL_ACTIONDB = "~/.ferris/hal-actions.db/";

namespace Ferris
{
    namespace FDO
    {
    //     class HalDeviceProxy;
    //     class HalManagerProxy;

    //     class HalManagerProxy
    //         : public DBus::InterfaceProxy,
    //           public DBus::ObjectProxy
    //     {
    //     public:

    //         HalManagerProxy( DBus::Connection& connection )
    //             : DBus::InterfaceProxy("org.freedesktop.Hal.Manager"),
    //               DBus::ObjectProxy(connection, "/org/freedesktop/Hal/Manager", "org.freedesktop.Hal")
    //             {
    //             }
            
    //     };
    // };

    
    class VolumeManager
        :
        public fVolumeManager
    {
        typedef VolumeManager _Self;
        bool m_ignorePlugEvents;
        sigc::connection CreatedConnection;
        

    protected:

        fh_context getHALUserDB()
            {
                static fh_context ret = 0;
                if( !ret )
                {
                    ret = Resolve( HAL_USERDB );
                    ret->read();

                    fh_context t = Resolve( HAL_ACTIONDB );
                    t->read();
                }
                return ret;
            }

        fh_context getVolumeReactionContext( fh_context vol )
            {
                fh_context userdb = getHALUserDB();
                try
                {
                    LG_HAL_D << "volume:" << vol->getDirName() << endl;
                    if( userdb->isSubContextBound( vol->getDirName() ))
                    {
                        fh_context c = userdb->getSubContext( vol->getDirName() );
                        return c;
                    }
                    if( userdb->isSubContextBound( "ffilter" ))
                    {
                        LG_HAL_D << "have ffilter dir... volume:" << vol->getDirName() << endl;
                        fh_context ffc = userdb->getSubContext( "ffilter" );
                        ffc->read(1);
                        for( Context::iterator ffiter = ffc->begin(); ffiter!=ffc->end(); ++ffiter )
                        {
                            fh_context ret = *ffiter;
                            LG_HAL_D << "ffiter:" << ret->getURL() << endl;
                            string ffilter = getStrSubCtx( ret, "ffilter", "" );
                            LG_HAL_D << "testing ffilter:" << ffilter << endl;

                            fh_context z = Factory::MakeFilter( ffilter );
                            fh_matcher zm = Factory::MakeMatcherFromContext( z );

                            if( zm( vol ) )
                            {
                                LG_HAL_D << "ffilter matches! f:" << ffilter << endl;
                                return ret;
                            }
                        }
                    }
                }
                catch( exception& e )
                {
                    LG_HAL_D << "exception:" << e.what() << endl;
                    cerr << "exception:" << e.what() << endl;
                }
                return 0;
            }

    public:
        virtual void priv_DevicePlugged( fh_context vol )
            {
                LG_HAL_D << "OnCreated() m_ignorePlugEvents:" << m_ignorePlugEvents << " vol:" << vol->getURL() << endl;
                if( m_ignorePlugEvents )
                    return;

                if( contains( vol->getURL(), "/devices/usb" ))
                {
                    
                }
                
                stringlist_t sl = Util::parseCommaSeperatedList("name,media.bus,storage.bus,storage.model,storage.vendor");
                for( stringlist_t::iterator si = sl.begin(); si != sl.end(); ++si )
                {
                    cerr << "attr:" << *si << " value:" << getStrAttr( vol, *si, "" ) << endl;
                }

                try
                {
                    cerr << "volume:" << vol->getDirName() << endl;
                    if( fh_context c = getVolumeReactionContext( vol ) )
                    {
                        cerr << "have explicit volume actions setup...:" << c->getURL() << endl;
                        c = c->getSubContext( "actions" );
                        cerr << "iterating explicit volume actions for...:" << c->getURL() << endl;
                        for( int i=1; i<100; ++i )
                        {
                            if( !c->isSubContextBound( tostr(i) ))
                            {
                                break;
                            }

                            fh_context ac = c->getSubContext( tostr(i) );
                            cerr << "action:" << ac->getURL() << endl;

                            string actionURL = HAL_ACTIONDB + getStrAttr( ac, "content", "" );
                            fh_context x =  Resolve( actionURL );
                            string displayName = getStrAttr( x,  "display-name", "" );
                            string command     = getStrAttr( x,  "command", "" );
                            string cargs       = getStrAttr( ac, "command-arguments", "" );

                            cerr << "actionURL:" << actionURL << endl
                                 << " x:" << x->getURL() << endl
                                 << " displayName:" << displayName << " command:" << command << endl;

                            fh_runner r = new Runner();
                            r->setSpawnFlags(
                                GSpawnFlags(
                                    G_SPAWN_SEARCH_PATH
                                    | r->getSpawnFlags()));
                            {
                                stringstream ss;
                                ss << command << " " << cargs << " " << vol->getURL();
                                r->setCommandLine( ss.str() );
                                cerr << "cmd:" << ss.str() << endl;
                            }
                            
                            r->Run();
                            gint e = r->getExitStatus();
                        }
                    }
                    else
                    {
                        if( isTrue( getStrSubCtx( "~/.ferris/hal-ignore-unknown", "0" ) ) )
                        {
                        }
                        else
                        {
                            // We don't know what to do here!
                            fh_stringstream ss;
                            ss << "ferris-volume-manager-setup-volume-wizard " << vol->getURL() << endl;
                            cerr << "No handler for volume... running wizard:" << ss.str() << endl;
                            fh_runner r = new Runner();
                            r->setCommandLine( ss.str() );
                            r->setSpawnFlags(
                                GSpawnFlags(
                                    G_SPAWN_SEARCH_PATH
                                    | r->getSpawnFlags()));
                            r->Run();
                            gint e = r->getExitStatus();
                        }
                    }
                }
                catch( exception& e )
                {
                    cerr << "exception:" << e.what() << endl;
                }
            }

        
        virtual QString DevicePlugged( const QString & qhaldev )
            {
                string haldev = tostr( qhaldev );
                fh_context vol = Resolve( haldev );
                priv_DevicePlugged( vol );
//                 VolumeManager_DevicePlugged_Data* d = new VolumeManager_DevicePlugged_Data;
//                 d->vm = this;
//                 d->vol = vol;
//                 g_timeout_add_seconds( 1, GSourceFunc(VolumeManager_DevicePlugged), d );
            }
        
        virtual void OnCreated( NamingEvent_Created* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn )
            {
                fh_context vol = subc;
                priv_DevicePlugged( vol );
            }

        void setIgnorePlugEvents( bool v )
            {
                m_ignorePlugEvents = v;
            }
        
        
    public:
        VolumeManager( QObject *parent )
            :
            fVolumeManager(parent),
            m_ignorePlugEvents( false )
            {
                
                CreatedConnection = volumectx->getNamingEvent_Created_Sig().connect(sigc::mem_fun( *this, &_Self::OnCreated ));
                usbctx->getNamingEvent_Created_Sig().connect(sigc::mem_fun( *this, &_Self::OnCreated ));
                
            }
        
        ~VolumeManager()
            {
            }

        virtual int Random(  )
            {
                int ret = rand();
                cerr << "Random() ret:" << ret << endl;
                return ret;
            }

        fh_context findVolumeByDevID( dev_t devid )
            {
                fh_context ret = 0;
                
                fh_context c = volumectx;
                for( Context::iterator ci = c->begin(); ci!=c->end(); ++ci )
                {
                    dev_t major = toType<dev_t>(getStrAttr( *ci, "block.major", "0" ));
                    dev_t minor = toType<dev_t>(getStrAttr( *ci, "block.minor", "0" ));

                    if( major == MAJOR( devid ) && minor == MINOR( devid ) )
                    {
                        ret = *ci;
                        return ret;
                    }
                }
                return ret;
            }
        
        virtual bool isEjectable( int devid )
            {
                bool ret = false;

                cerr << "isEjectable() devid:" << devid << endl;

                fh_context dc = findVolumeByDevID( devid );
                if( isBound( dc ) )
                {
                    ret = true;
                    cerr << "isEjectable(YES) ctx:" << dc->getURL() << endl;
                    return ret;
                }
                cerr << "isEjectable(NO) devid:" << devid << endl;
                return ret;
            }

        virtual QString Eject( int devid )
            {
                QString ret = "";

                try
                {
                    LG_HAL_D << "Eject() devid:" << devid << endl;

                    fh_context dc = findVolumeByDevID( devid );
                    if( isBound( dc ) )
                    {
                        string extra_options = "";
                        LG_HAL_D << "trying to eject ctx:" << dc->getURL() << endl;
                        getStrAttr( dc, "eject", "",1,1 );
                    }
                }
                catch( exception& e )
                {
                    string msg = e.what();
                    LG_HAL_D << "Eject, error:" << msg << endl;
                    ret = msg.c_str();
                }
                return ret;
            }

        virtual QString Unmount( int devid )
            {
                QString ret = "";

                try
                {
                    LG_HAL_D << "Eject() devid:" << devid << endl;

                    fh_context dc = findVolumeByDevID( devid );
                    if( isBound( dc ) )
                    {
                        string extra_options = "";
                        LG_HAL_D << "Ejecting ctx:" << dc->getURL() << endl;
                        getStrAttr( dc, "unmount", "",1,1 );
                    }
                }
                catch( exception& e )
                {
                    ret = e.what();
                }
                return ret;
            }
        virtual QString Mount( int devid, const QString& qmp )
            {
                string mp = tostr(qmp);
                QString ret = "";

                try
                {
                    cerr << "Eject() devid:" << devid << endl;

                    fh_context dc = findVolumeByDevID( devid );
                    if( isBound( dc ) )
                    {
                        string extra_options = "";
                        cerr << "Ejecting ctx:" << dc->getURL() << endl;
                        setStrAttr( dc, "mount", mp );
                    }
                }
                catch( exception& e )
                {
                    ret = e.what();
                }
                return ret;
            }
    };
    };
    
    
};

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int main( int argc, const char** argv )
{
    string LockFileURL = "~/.ferris/volume-manager-lock";
    
    const char* ShowColumnsCSTR  = "x";
    unsigned long HideHeadings   = 1;
    unsigned long ShowVersion    = 0;
    unsigned long DontStartAsDaemon    = 0;
        
    struct poptOption optionsTable[] = {

//         { "show-columns", 0, POPT_ARG_STRING, &ShowColumnsCSTR, 0,
//           "Same as --show-ea",
//           "size,mimetype,name" },

        
//         { "hide-headings", 0, POPT_ARG_NONE, &HideHeadings, 0,
//           "Prohibit the display of column headings", 0 },

        { "dont-daemon", 'D', POPT_ARG_NONE, &DontStartAsDaemon, 0,
          "Dont detach as a daemon", "" },
        
        
        /*
         * Other handy stuff
         */

        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },
        
        /*
         * Standard Ferris options
         */
        FERRIS_POPT_OPTIONS

        /**
         * Expansion of strange-url://foo*
         */
        FERRIS_SHELL_GLOB_POPT_OPTIONS
        
//         { "show-all-attributes", 0, POPT_ARG_NONE,
//           &ShowAllAttributes, 0,
//           "Show every available bit of metadata about each object in context",
//           0 },
        POPT_AUTOHELP
        POPT_TABLEEND
   };
    poptContext optCon;


    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* ...");

    if (argc < 1) {
//        poptPrintHelp(optCon, stderr, 0);
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }

    
    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
//         switch (c) {
//         }
    }

    if( ShowVersion )
    {
        cout << "ferris-volume-manager version: $Id: ls.cpp,v 1.12 2008/04/27 21:30:11 ben Exp $\n"
             << "ferris   version: " << VERSION << nl
             << "Written by Ben Martin, aka monkeyiq" << nl
             << nl
             << "Copyright (C) 2008 Ben Martin" << nl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }

    while( const char* argCSTR = poptGetArg(optCon) )
    {
        if( argCSTR )
        {
            string arg = argCSTR;
        }
    }

    cerr << "DontStartAsDaemon:" << DontStartAsDaemon << endl;
    if( !DontStartAsDaemon )
    {
        stringstream ss;
        ss << "/tmp/libferris-lock-" << getuid() << "-volume-manager-lock";
        LockFileURL = ss.str();
        fh_context c = Shell::acquireContext( LockFileURL, 0, false );
        setStrAttr( c, "content", tostr(getpid()) );
        ExitIfAlreadyRunning( LockFileURL );
        SwitchToDaemonMode();
    }

    try
    {
        cerr << "ferris volume manager starting up..." << endl;
        QCoreApplication* app = new QCoreApplication( argc, (char**)argv );

        volumectx = Resolve( "hal://devices/volume" );
        usbctx    = Resolve( "hal://devices/usb" );

        Ferris::FDO::VolumeManager* dobj = new Ferris::FDO::VolumeManager( app );
        Main::mainLoop();
        
//        while( running )
//            g_main_iteration( true );

    }
    catch( NoSuchContextClass& e )
    {
        LG_MDSERV_W << "broker error:" << e.what() << endl;
        cerr << "e:" << e.what() << endl;
        exit(1);
    }
    catch( exception& e )
    {
        LG_MDSERV_W << "broker error:" << e.what() << endl;
        cerr << "cought:" << e.what() << endl;
        exit(1);
    }

    
    poptFreeContext(optCon);
    return 0;
}
