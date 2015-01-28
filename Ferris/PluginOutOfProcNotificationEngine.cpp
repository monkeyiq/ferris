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

    $Id: PluginOutOfProcNotificationEngine.cpp,v 1.12 2010/09/24 21:30:56 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "PluginOutOfProcNotificationEngine.hh"
#include "common-ferris-out-of-proc-notification-deamon.hh"
#include "Runner.hh"
#include "FerrisDOM.hh"
#include "Medallion.hh"
#include "ValueRestorer.hh"

#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
using namespace Ferris::OProcMessage;

namespace Ferris
{
    static bool s_dontConnectWithFerrisOutOfProcDeamon = false;

    static bool dontConnectWithFerrisOutOfProcDeamon()
    {
        static bool readEnvVar = false;
        if( !readEnvVar )
        {
            readEnvVar = true;
            if( !s_dontConnectWithFerrisOutOfProcDeamon )
            {
                s_dontConnectWithFerrisOutOfProcDeamon =
                    (g_getenv ("LIBFERRIS_DONT_CONNECT_WITH_OUT_OF_PROC_DAEMON") != 0);
            }
        }
        return s_dontConnectWithFerrisOutOfProcDeamon;
    }
    
    
    void
    IHandleOutOfProcEANotification::OnOutOfProcEACreationNotification( const std::string& eaname,
                                                                       bool isDataValid,
                                                                       const std::string& data )
    {
    }
    
    void
    IHandleOutOfProcEANotification::OnOutOfProcEADeletionNotification( const std::string& eaname )
    {
    }
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    OutOfProcNotificationEngine::OutOfProcNotificationEngine()
        :
        m_fromserv_aio( 0 ),
        outgoingss( Factory::fcnull() ),
        m_basedir( FERRIS_OUT_OF_PROC_NOTIFICATION_DEAMON_ROOT )
    {
    }

    OutOfProcNotificationEngine::~OutOfProcNotificationEngine()
    {
        string fromserv = appendFromServPrefix( m_basedir ) + tostr(getpid());
        ::unlink( fromserv.c_str() );
    }

    void
    OutOfProcNotificationEngine::setBaseDir( const std::string& v )
    {
        m_basedir = v;
    }
    
    std::string
    OutOfProcNotificationEngine::getBaseDir()
    {
        return m_basedir;
    }
    
    
    void
    OutOfProcNotificationEngine::xml_msg_arrived( fh_xstreamcol h )
    {
        stringmap_t&   m = h->getStringMap();
//        cerr << "OutOfProcNotificationEngine::xml_msg_arrived()" << endl;
    }

    OutOfProcNotificationEngine::clientID_t
    OutOfProcNotificationEngine::getClientID()
    {
        return getpid();
    }
    
    pid_t
    OutOfProcNotificationEngine::getServerPID()
    {
        fh_context c = Shell::acquireContext( m_basedir );
        fh_context s = Shell::acquireSubContext( c, "pid" );
        string pid = getStrAttr( s, "content", "0" );
        return toType<pid_t>( pid );
    }


    void
    OutOfProcNotificationEngine::ensureServerRunning()
    {
        if( dontConnectWithFerrisOutOfProcDeamon() )
            return;

        
        bool serverRunning = true;
        static pid_t pid = 0;
        
        if( !pid )
            pid = getServerPID();
        
        if( pid )
        {
            if( kill( pid, SIGUSR2 ) )
            {
                if( ESRCH == errno )
                {
                    pid = 0;
                    serverRunning = false;
                    LG_JOURNAL_D << "Server ping failed! starting server..."
                                 << " it should have been at pid:" << pid  << endl;
                }
            }
        }
        else 
            serverRunning = false;

        
        LG_JOURNAL_D << "PluginOutOfProcNotificationEngine::ensureServerRunning(neartop)"
                     << " pid:" << pid
                     << " client-pid:" << tostr(getpid())
                     << " serverRunning:" << serverRunning
                     << " isBound(m_fromserv_aio):" << isBound(m_fromserv_aio)
                     << endl;

        bool needToSetupIO = false;
        
        if( !isBound(m_fromserv_aio) )
        {
            needToSetupIO = true;

            Shell::acquireContext( appendStagePrefix( m_basedir ) );
            /*
             * Create the fifo in the staging area, when we have it open we move it
             * to the incoming dir so that the server can open its end and unlink it.
             */
            string stagePath = appendStagePrefix( m_basedir )    + tostr(getpid());
            string inUsePath = appendFromServPrefix( m_basedir ) + tostr(getpid());
            int fd = Factory::MakeFIFO( stagePath );
            ::rename( stagePath.c_str(), inUsePath.c_str() );

            LG_JOURNAL_D << "PluginOutOfProcNotificationEngine::ensureServerRunning()"
                         << " client-pid:" << tostr(getpid())
                         << " serv->us fd:" << fd
                         << endl;

//             cerr << "PluginOutOfProcNotificationEngine::ensureServerRunning()"
//                  << " pid:" << tostr(getpid())
//                  << " serv->us fd:" << fd
//                  << endl;
            
            m_fromserv_aio = new AsyncIOHandler( fd );
            m_fromserv_xs  = Factory::MakeXMLStreamCol();
            m_fromserv_xs->attach( m_fromserv_aio );
            m_fromserv_xs->getMessageArrivedSig().connect( sigc::mem_fun( *this, &_Self::xml_msg_arrived ) );
            
//             m_fromserv_aio->setFunctor(
//                 AsyncIOHandler::AsyncIOFunctor_t(
//                     this, &_Self::serv_aio_handler ));
        }
        
            
        
        if( !serverRunning )
        {
            LG_JOURNAL_D << "PluginOutOfProcNotificationEngine::ensureServerRunning()"
                         << " starting server!"
                         << endl;
            
            Factory::MakeFIFO( appendToServPath( m_basedir ), false );
            
            fh_runner r = new Runner();
            fh_stringstream cmdss;
            cmdss << "ferris-out-of-proc-notification-deamon "
                  << " --dont-recreate-fifo "
                  << " --target-directory=" << m_basedir
                  << " ";
            r->setCommandLine( tostr(cmdss) );
            LG_JOURNAL_D << "PluginOutOfProcNotificationEngine::ensureServerRunning()"
                         << " cmd:" << tostr(cmdss)
                         << endl;
            
            r->setSpawnFlags(
                GSpawnFlags(
                    G_SPAWN_SEARCH_PATH |
                    G_SPAWN_STDERR_TO_DEV_NULL |
                    G_SPAWN_STDOUT_TO_DEV_NULL |
                    0 ));
            r->Run();

            LG_JOURNAL_D << "PluginOutOfProcNotificationEngine::ensureServerRunning()"
                         << " opening a write only fd to the server at:" 
                         << appendToServPath( m_basedir ) << endl;
            int fd = open( appendToServPath( m_basedir ).c_str(), O_WRONLY );
            close( fd );
            LG_JOURNAL_D << "PluginOutOfProcNotificationEngine::ensureServerRunning()"
                         << " COMPLETED opening a write only fd to the server." << endl;
        }

        if( needToSetupIO )
        {
            LG_JOURNAL_D << "PluginOutOfProcNotificationEngine::ensureServerRunning()"
                         << " setting up to server iostream"
                         << " at:" << appendToServPath( m_basedir )
                         << endl;
            fh_ofstream fss( appendToServPath( m_basedir ) );
            outgoingss = fss;
            
        }
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    OutOfProcNotificationEngineDebug::OutOfProcNotificationEngineDebug()
        :
        m_messageLogStream( Factory::fcerr() ),
        m_messageCount( 0 )
    {
    }
    
    OutOfProcNotificationEngineDebug::~OutOfProcNotificationEngineDebug()
    {
    }

    void
    OutOfProcNotificationEngineDebug::xml_msg_arrived( fh_xstreamcol h )
    {
        stringmap_t&   m = h->getStringMap();
        fh_stringstream ss;
        
        ss << "--- start of message:" << m_messageCount << endl;
        for( stringmap_t::iterator mi = m.begin(); mi != m.end(); ++mi )
        {
            ss << "key:" << mi->first << " value:" << mi->second << endl;
        }
        ss << "--- end of message:" << m_messageCount << " ---" << endl;

        m_messageLogStream << tostr(ss) << flush;
        LG_JOURNAL_D << tostr(ss) << flush;
        
        ++m_messageCount;
    }
    
    void
    OutOfProcNotificationEngineDebug::connect()
    {
        ensureServerRunning();
    }
    
    
    void
    OutOfProcNotificationEngineDebug::setOutputStream( fh_ostream oss )
    {
        m_messageLogStream = oss;
    }
    
    void
    OutOfProcNotificationEngineDebug::sendMessage( stringmap_t& m )
    {
        XML::writeMessage( outgoingss, m );
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    PluginOutOfProcNotificationEngine::PluginOutOfProcNotificationEngine()
        :
        m_ignoreCreatedSignals( false ),
        m_ignoreDeletedSignals( false )
    {
//        cerr << "PluginOutOfProcNotificationEngine(ctor)" << endl;
    }

    PluginOutOfProcNotificationEngine::~PluginOutOfProcNotificationEngine()
    {
//        cerr << "~PluginOutOfProcNotificationEngine(~dtor)" << endl;
    }
    
    void
    PluginOutOfProcNotificationEngine::connectSignals( fh_context c )
    {
        m_monitoredContexts_t::iterator mi = m_monitoredContexts.find( GetImpl(c) );
        if( mi == m_monitoredContexts.end() )
        {
            m_monitoredContexts.insert( GetImpl(c) );

            LG_JOURNAL_D << "connectSignals() c:" << toVoid(GetImpl(c)) << endl;
            c->getNamingEvent_Created_Sig().connect( sigc::mem_fun( *this, &_Self::OnCreated ));
            c->getNamingEvent_Exists_Sig() .connect( sigc::mem_fun( *this, &_Self::OnExists ));
            c->getNamingEvent_Deleted_Sig().connect( sigc::mem_fun( *this, &_Self::OnDeleted ));
        }
    }

    void
    PluginOutOfProcNotificationEngine::signalContextDeleted( fh_context p, const std::string& olddn )
    {
        stringmap_t m;
        m[ KEY_COMMAND ]   = COMMAND_CTX_DELETED;
        m[ KEY_URL     ]   = p->getURL();
        m[ KEY_NAME    ]   = olddn;
        m[ KEY_OBAND_PID ] = tostr(getpid());
        XML::writeMessage( outgoingss, m );

        m_monitoredContexts.erase( GetImpl(p) );
    }
    
    
    void
    PluginOutOfProcNotificationEngine::OnDeleted( NamingEvent_Deleted* ev,
                                                  std::string olddn,
                                                  std::string newdn )
    {
        if( m_ignoreDeletedSignals )
            return;
        
        fh_context c = ev->getSource();
        
        LG_JOURNAL_D << "PluginOutOfProcNotificationEngine::OnDeleted() url:" << c->getURL()
                     << " olddn:" << olddn
                     << endl;

        signalContextDeleted( c, olddn );
    }

    void
    PluginOutOfProcNotificationEngine::OnExists( NamingEvent_Exists* ev,
                                                 const fh_context& subc,
                                                 std::string olddn,
                                                 std::string newdn )
    {
//        fh_context subc = ev->getSource()->getSubContext( olddn );
        connectSignals( subc );
    }

    void
    PluginOutOfProcNotificationEngine::signalContextChanged( fh_context c,
                                                             const std::string& data, bool isDataValid )
    {
        /*
         * tell the server
         */
        stringmap_t m;
        m[ KEY_COMMAND ] = COMMAND_CTX_CHANGED;
        m[ KEY_URL     ] = c->getURL();
        m[ KEY_NAME    ] = c->getDirName();
        m[ KEY_OBAND_PID ] = tostr(getpid());
        if( isDataValid )
            m[ KEY_DATA ] = data;
            
        LG_JOURNAL_D << "PluginOutOfProcNotificationEngine::signalContextChanged()"
                     << " url:" << c->getURL() << endl;
        XML::writeMessage( outgoingss, m );
    }
    
    void
    PluginOutOfProcNotificationEngine::signalContextCreated( fh_context subc,
                                                             const std::string& data,
                                                             bool isDataValid )
    {
        LG_JOURNAL_D << "PluginOutOfProcNotificationEngine::signalContextCreated()"
                     << " url:" << subc->getURL()
                     << " m_ignoreCreatedSignals:" << m_ignoreCreatedSignals
                     << endl;
        
        if( m_ignoreCreatedSignals )
            return;
        
        /*
         * tell the server
         */
        stringmap_t m;
        m[ KEY_COMMAND ] = COMMAND_CTX_CREATED;
        m[ KEY_URL     ] = subc->getParent()->getURL();
        m[ KEY_NAME    ] = subc->getDirName();
        m[ KEY_OBAND_PID ] = tostr(getpid());
        if( isDataValid )
            m[ KEY_DATA ] = data;
            
        LG_JOURNAL_D << "PluginOutOfProcNotificationEngine::signalContextCreated()"
                     << " url:" << subc->getURL() << endl;
        XML::writeMessage( outgoingss, m );
    }
    void
    PluginOutOfProcNotificationEngine::signalContextCreated( fh_context subc )
    {
        signalContextCreated( subc, "", false );
    }

    
    
    void
    PluginOutOfProcNotificationEngine::OnCreated( NamingEvent_Created* ev,
                                                  const fh_context& subc,
                                                  std::string olddn,
                                                  std::string newdn )
    {
        LG_JOURNAL_D << "PluginOutOfProcNotificationEngine::OnCreated() olddn:" << olddn << endl;
        
        if( m_ignoreCreatedSignals )
            return;
        
//        fh_context subc = ev->getSource()->getSubContext( olddn );
        connectSignals( subc );
        signalContextCreated( subc );
    }

    void
    PluginOutOfProcNotificationEngine::xml_msg_arrived( fh_xstreamcol h )
    {
        stringmap_t&   m = h->getStringMap();

//        cerr << "PluginOutOfProcNotificationEngine::xml_msg_arrived( top )" << endl;
        
        if( m[ KEY_COMMAND ] == COMMAND_CTX_CREATED )
        {
            string url  = m[ KEY_URL  ];
            string name = m[ KEY_NAME ];

            LG_JOURNAL_D << "Got notification of created context:" << url << " for:" << name << endl;
            
            fh_context c = Resolve( url );

            if( IHandleOutOfProcContextCreationNotification* h =
                dynamic_cast<IHandleOutOfProcContextCreationNotification*>( c->getOverMountContext() ) )
            {
                stringmap_t::const_iterator si = m.find( KEY_DATA );
                bool isDataValid = (si != m.end());
                Util::ValueRestorer<bool> r( m_ignoreCreatedSignals, true );
                h->OnOutOfProcContextCreationNotification( name, isDataValid, isDataValid ? si->second : "" );
            }
            else
            {
                LG_JOURNAL_W << "CANT DOWNCAST url:" << c->getURL()
                             << " to IHandleOutOfProcContextCreationNotification interface!"
                             << endl;
            }
        }
        if( m[ KEY_COMMAND ] == COMMAND_CTX_CHANGED )
        {
            string url  = m[ KEY_URL  ];
            string name = m[ KEY_NAME ];
            LG_JOURNAL_D << "Got notification of changed context:" << url << " for:" << name << endl;
            fh_context c = Resolve( url );

            if( IHandleOutOfProcContextChangedNotification* h =
                dynamic_cast<IHandleOutOfProcContextChangedNotification*>( c->getOverMountContext() ) )
            {
                Util::ValueRestorer<bool> r( m_ignoreCreatedSignals, true );
                h->OnOutOfProcContextChangedNotification( name, true, m[ KEY_DATA ] );
            }
            else
            {
                LG_JOURNAL_W << "CANT DOWNCAST url:" << c->getURL()
                             << " to IHandleOutOfProcContextCreationNotification interface!"
                             << endl;
            }
        }
        if( m[ KEY_COMMAND ] == COMMAND_CTX_DELETED )
        {
            string url  = m[ KEY_URL  ];
            string name = m[ KEY_NAME ];

            LG_JOURNAL_D << "notification of deleted context:" << url << " for:" << name << endl;
            
            fh_context c = Resolve( url );

            if( IHandleOutOfProcContextDeletionNotification* h =
                dynamic_cast<IHandleOutOfProcContextDeletionNotification*>( c->getOverMountContext() ) )
            {
                Util::ValueRestorer<bool> r( m_ignoreDeletedSignals, true );
                h->OnOutOfProcContextDeletionNotification( name );
            }
            else
            {
                LG_JOURNAL_W << "CANT DOWNCAST url:" << c->getURL()
                             << " to IHandleOutOfProcContextDeletionNotification interface!"
                             << endl;
            }
        }
        else if( m[ KEY_COMMAND ] == COMMAND_EA_CREATED || m[ KEY_COMMAND ] == COMMAND_EA_DELETED )
        {
            string url  = m[ KEY_URL  ];
            string name = m[ KEY_NAME ];
            fh_context c = Resolve( url );
            if( IHandleOutOfProcEANotification* h =
                dynamic_cast<IHandleOutOfProcEANotification*>( c->getOverMountContext() ) )
            {
                stringmap_t::const_iterator si = m.find( KEY_DATA );
                bool isDataValid = (si != m.end());

                Util::ValueRestorer<bool> r( m_ignoreEASignals, true );
                if( m[ KEY_COMMAND ] == COMMAND_EA_CREATED )
                    h->OnOutOfProcEACreationNotification( name, isDataValid, isDataValid ? si->second : "" );
                else if( m[ KEY_COMMAND ] == COMMAND_EA_DELETED )
                    h->OnOutOfProcEADeletionNotification( name );
            }
            else
            {
                LG_JOURNAL_W << "CANT DOWNCAST url:" << c->getURL()
                             << " to IHandleOutOfProcEANotification interface!"
                             << endl;
            }
        }
        else if( m[ KEY_COMMAND ] == COMMAND_MEDALLION_UPDATED )
        {
            string url  = m[ KEY_URL  ];
//             cerr << "PluginOutOfProcNotificationEngine::xml_msg_arrived( medallion )"
//                  << " url:" << url << endl;
            
            getMedallionUpdated_Sig().emit( url );
        }
        else if( m[ KEY_COMMAND ] == COMMAND_ETAGERE_NEW_EMBLEM )
        {
            set< emblemID_t > eset;
            Util::parseSeperatedList( m[KEY_ESET],
                                      eset, inserter( eset, eset.end() ) );
            fh_etagere et = Factory::getEtagere();
            et->OnOutOfProcNewEmblemNotification( eset );
        }
    }

    void
    PluginOutOfProcNotificationEngine::forgetContext( Context* base )
    {
        cerr << "PluginOutOfProcNotificationEngine::forgetContext() base:" << (void*)base << endl;

        m_monitoredContexts.erase( base );
    }
    
    void
    PluginOutOfProcNotificationEngine::watchTree( fh_context base )
    {
        if( dontConnectWithFerrisOutOfProcDeamon() )
            return;
        
        LG_JOURNAL_D << "watchTree() Adding watch to base:" << base->getURL() << endl;

        /*
         * We need to tell the server about this new base context and see if there
         * is already someone monitoring it.
         */
        ensureServerRunning();

        LG_JOURNAL_D << "watchTree() Server is running base:" << base->getURL() << endl;

        connectSignals( base );
    }

    void
    PluginOutOfProcNotificationEngine::ensureServerRunning()
    {
        _Base::ensureServerRunning();
    }

    

    void
    PluginOutOfProcNotificationEngine::signalEACreated( fh_context c, const std::string& eaname,
                                                        const std::string& data, bool isDataValid )
    {
        if( dontConnectWithFerrisOutOfProcDeamon() )
            return;

        stringmap_t m;
        m[ KEY_COMMAND ] = COMMAND_EA_CREATED;
        m[ KEY_URL     ] = c->getURL();
        m[ KEY_NAME    ] = eaname;
        m[ KEY_OBAND_PID ] = tostr(getpid());
        if( isDataValid )
            m[ KEY_DATA ] = data;
        
        LG_JOURNAL_D << "PluginOutOfProcNotificationEngine::signalEACreated url:" << c->getURL()
                     << " eaname:" << eaname
                     << endl;
        XML::writeMessage( outgoingss, m );
    }
    void
    PluginOutOfProcNotificationEngine::signalEACreated( fh_context c, const std::string& eaname )
    {
        signalEACreated( c, eaname, "", false );
    }
    
    void
    PluginOutOfProcNotificationEngine::signalEADeleted( fh_context c, const std::string& eaname )
    {
        if( dontConnectWithFerrisOutOfProcDeamon() )
            return;

        stringmap_t m;
        m[ KEY_COMMAND ] = COMMAND_EA_DELETED;
        m[ KEY_URL     ] = c->getURL();
        m[ KEY_NAME    ] = eaname;
        m[ KEY_OBAND_PID ] = tostr(getpid());
        LG_JOURNAL_D << "PluginOutOfProcNotificationEngine::signalEADeleted url:" << c->getURL()
                     << " eaname:" << eaname
                     << endl;
        XML::writeMessage( outgoingss, m );
    }

    void
    PluginOutOfProcNotificationEngine::signalMedallionUpdated( fh_context c )
    {
        if( dontConnectWithFerrisOutOfProcDeamon() )
            return;

        ensureServerRunning();

        stringmap_t m;
        m[ KEY_COMMAND ] = COMMAND_MEDALLION_UPDATED;
        m[ KEY_URL     ] = c->getURL();
        m[ KEY_OBAND_PID ] = tostr(getpid());

//         cerr << "PluginOutOfProcNotificationEngine::signalMedallionUpdated(1)"
//              << " url:" << c->getURL()
//              << endl;
//         LG_JOURNAL_D << "PluginOutOfProcNotificationEngine::signalMedallionUpdated()"
//                      << " url:" << c->getURL()
//                      << endl;

        XML::writeMessage( outgoingss, m );

//         cerr << "PluginOutOfProcNotificationEngine::signalMedallionUpdated(2)"
//              << " url:" << c->getURL()
//              << endl;
    }

    void
    PluginOutOfProcNotificationEngine::signalEtagereNewEmblems( fh_etagere et,
                                                                std::set< emblemID_t >& eset )
    {
        if( dontConnectWithFerrisOutOfProcDeamon() )
            return;

        ensureServerRunning();

        if( et == Factory::getEtagere() )
        {
            stringmap_t m;
//            cerr << "signalEtagereNewEmblems eset.sz:" << eset.size() << endl;
            string esetstr = Util::createSeperatedList( eset.begin(), eset.end() );
//            cerr << "signalEtagereNewEmblems eset.str:" << esetstr << endl;
            m[ KEY_COMMAND ] = COMMAND_ETAGERE_NEW_EMBLEM;
            m[ KEY_ESET    ] = esetstr;
            m[ KEY_OBAND_PID ] = tostr(getpid());
        
            XML::writeMessage( outgoingss, m );
        }
    }
    

    
    PluginOutOfProcNotificationEngine::MedallionUpdated_Sig_t&
    PluginOutOfProcNotificationEngine::getMedallionUpdated_Sig()
    {
        if( dontConnectWithFerrisOutOfProcDeamon() )
            return MedallionUpdated_Sig;

        ensureServerRunning();
        return MedallionUpdated_Sig;
    }
    


    namespace Factory
    {
        PluginOutOfProcNotificationEngine& getPluginOutOfProcNotificationEngine()
        {
            typedef Loki::SingletonHolder<
                PluginOutOfProcNotificationEngine,
                Loki::CreateUsingNew,
                Loki::PhoenixSingleton > S;
            return S::Instance();
        }

        void
        setDontConnectWithFerrisOutOfProcDeamon( bool v )
        {
            s_dontConnectWithFerrisOutOfProcDeamon = v;
        }
        
        
    }


};
