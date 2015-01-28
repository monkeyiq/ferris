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

    $Id: FerrisSlaveProcess.cpp,v 1.4 2011/05/06 21:30:20 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "FerrisSlaveProcess.hh"
#include "FerrisSlaveProcess_private.hh"
#include "ChildStreamServer.hh"

using namespace std;

namespace Ferris
{
    const int CPU_COUNT = 4;
    const int GROUP_SLAVE_PROCESS_COUNT = CPU_COUNT * 1.5;


    
    class FerrisSlaveProcessImpl
        :
        public FerrisSlaveProcess
    {
        std::string m_taskname;
        std::string m_cmdline;
        
        fh_runner     m_runner;
        fh_xstreamcol m_XMLCol;
        fh_childserv  m_ChildServ;

        fh_xstreamcol getXMLCol()
            {
                if( !m_XMLCol )
                {
                    m_XMLCol = ::Ferris::Factory::MakeXMLStreamCol();
                }
                return m_XMLCol;
            }
        fh_childserv getChildServ()
            {
                if( !m_ChildServ )
                {
                    m_ChildServ = new ChildStreamServer();
                }
                return m_ChildServ;
            }
        
        
    public:
        FerrisSlaveProcessImpl( const std::string& taskname,
                                const std::string& cmd );

        fh_runner getRunner();

        ChildCompleteSig_t& getChildCompleteSig();
        MessageArrivedSig_t& getMessageArrivedSig();
    };
    
    FerrisSlaveProcessImpl::FerrisSlaveProcessImpl( const std::string& taskname,
                                                    const std::string& cmd )
        :
        m_runner( 0 ),
        m_XMLCol(0),
        m_ChildServ(0),
        m_taskname( taskname ),
        m_cmdline( cmd )
    {
    }
        
    fh_runner
    FerrisSlaveProcessImpl::getRunner()
    {
        if( !m_runner )
        {
            m_runner = new Runner();
            m_runner->setSpawnFlags(
                GSpawnFlags(
                    G_SPAWN_SEARCH_PATH |
                    G_SPAWN_STDERR_TO_DEV_NULL |
                    G_SPAWN_DO_NOT_REAP_CHILD ));

            // This is needed or we might miss the last bit of output
            m_runner->setSpawnFlags( GSpawnFlags( m_runner->getSpawnFlags() & ~(G_SPAWN_DO_NOT_REAP_CHILD)));
            
            getXMLCol()->attach(m_runner);
            getChildServ()->addChild(m_runner);
            if( !m_cmdline.empty() )
                m_runner->setCommandLine( m_cmdline );
            
        }
        return m_runner;
    }
        
    FerrisSlaveProcessImpl::ChildCompleteSig_t&
    FerrisSlaveProcessImpl::getChildCompleteSig()
    {
        return getChildServ()->getChildCompleteSig();
    }
        
    FerrisSlaveProcessImpl::MessageArrivedSig_t&
    FerrisSlaveProcessImpl::getMessageArrivedSig()
    {
        return getXMLCol()->getMessageArrivedSig();
    }

    fh_FerrisSlaveProcess
    CreateFerrisSlaveProcess( const std::string& taskname, const std::string& cmd )
    {
        return new FerrisSlaveProcessImpl( taskname, cmd );
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    IRunnableSlaveProcessManager::IRunnableSlaveProcessManager( const std::string& processName )
        :
        m_asyncSlave( 0 )
    {
        m_asyncSlave = CreateFerrisSlaveProcess( processName, "" );
        m_asyncSlave->getChildCompleteSig().connect(  sigc::mem_fun( *this, &_Self::OnAsyncChildComeplete ) );
    }

    fh_FerrisSlaveProcess
    IRunnableSlaveProcessManager::getSlaveProcess()
    {
        return m_asyncSlave;
    }
    
    void
    IRunnableSlaveProcessManager::OnAsyncChildComeplete( ChildStreamServer* css, fh_runner r, int status, int estatus )
    {
        LG_BGPROC_D << "OnChildComeplete() estatus:" << estatus << endl;
        /*
         * Make sure that all async IO calls have been accepted.
         */
        Main::processAllPendingEvents();
        Run();
//        gtk_idle_add(BackgroundEAReader_idle_cb, this);	
//        g_timeout_add( 1000, GSourceFunc(BackgroundEAReader_idle_cb), this );
    }
    
    

    /******************************/
    /******************************/
    /******************************/

    GroupIRunnableSlaveProcessManager::GroupIRunnableSlaveProcessManager()
        :
        IRunnableSlaveProcessManager( "group" )
    {
    }
    
    void
    GroupIRunnableSlaveProcessManager::addIRunnable( fh_IRunnableSlaveProcessManager r )
    {
        m_runnables.push_back( r );
    }

    void
    GroupIRunnableSlaveProcessManager::Run()
    {
        LG_BGPROC_D << "Starting children, count:" << m_runnables.size() << endl;
        
        m_runnables_t::iterator iter = m_runnables.begin();
        m_runnables_t::iterator    e = m_runnables.end();
        
        for( ; iter!=e; ++iter )
        {
            (*iter)->Run();
        }
        LG_BGPROC_D << "STARTED children, count:" << m_runnables.size() << endl;
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    BackgroundEAReader::BackgroundEAReader( fh_context target, const stringset_t& eanames )
        :
        IRunnableSlaveProcessManager( "async get ea" ),
        m_eanames( eanames ),
        m_target( target )
    {
        m_eanamesIter = m_eanames.begin();
        m_asyncSlave->getMessageArrivedSig().connect( sigc::mem_fun( *this, &_Self::OnAsyncXMLMessage ) );
    }

    BackgroundEAReader::~BackgroundEAReader()
    {
//        cerr << "+++~BackgroundEAReader()" << endl;
    }
    
    
    void
    BackgroundEAReader::Run()
    {
        const int maxEAPerProcess = 8;
        
        if( m_eanamesIter == m_eanames.end() )
            return;

        stringstream rdnlist;
        for(int j=0 ; m_eanamesIter != m_eanames.end() && j < maxEAPerProcess; ++m_eanamesIter, ++j )
        {
            string rdn = *m_eanamesIter;
            rdnlist << rdn << ",";
        }
//         if( m_eanamesIter != m_eanames.end() )
//             ++m_eanamesIter;
        
        
        fh_stringstream qss;
        qss << "fcat "
            << " --ferris-internal-async-message-slave "
            << " --ferris-internal-async-message-slave-attrs=" << rdnlist.str()
            << " " << m_target->getURL();
//        cerr << "+++cmd:" << tostr(qss) << endl;
        LG_BGPROC_D << "this:" << toVoid(this) << " CMD:" << tostr(qss) << endl;

        m_asyncSlave->getRunner()->setCommandLine( tostr(qss) );
        m_asyncSlave->getRunner()->Run();
        LG_BGPROC_D << "this:" << toVoid(this) << " have started CMD:" << tostr(qss) << endl;
    }

    void
    BackgroundEAReader::OnAsyncXMLMessage( fh_xstreamcol h )
    {
        stringmap_t&   m = h->getStringMap();
        LG_BGPROC_D << "BackgroundEAReader::OnAsyncXMLMessage() m.sz:" << m.size() << endl;
        for( stringmap_t::iterator mi = m.begin(); mi != m.end(); ++mi )
        {
            LG_BGPROC_D << "this:" << toVoid(this) << " OnAsyncXMLMessage() k:" << mi->first << " v:" << mi->second << endl;
        }
            
        if( m.end() != m.find("v") )
        {
            string eaname = m["eaname"];
            string      v = m["v"];
            LG_BGPROC_D << "have eaname:" << eaname << " v:" << v << endl;

            bool isError = false;
            getObtainedEASig().emit( this, eaname, v, isError );
        }
        else if( m.end() != m.find("outofband-error") )
        {
            string emsg = m["outofband-error"];
            string eaname = m["eaname"];
            LG_BGPROC_W << "ERROR:" << emsg << endl;
            bool isError = true;
            getObtainedEASig().emit( this, eaname, emsg, isError );
        }
        else
        {
            LG_BGPROC_W << "strange xml_msg_arrived()" << endl;
        }
    }


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    GroupBackgroundEAReader::GroupBackgroundEAReader( fh_context target, const stringset_t& eanames, int numberOfChildren )
    {
        int eanamessz = eanames.size();
        int numberPerChild = ceil( eanamessz * 1.0 / numberOfChildren );

        stringset_t::iterator iter = eanames.begin();
        stringset_t::iterator    e = eanames.end();

        LG_BGPROC_D << "GroupBackgroundEAReader() eanamessz:" << eanamessz
                   << " numberPerChild:" << numberPerChild
                   << " numberOfChildren:" << numberOfChildren
                   << endl;
        for( int i=0; i < numberOfChildren; ++i )
        {
            stringset_t tmp;
            for(int j=0; iter!=e && j<numberPerChild; ++iter, ++j )
            {
                tmp.insert( *iter );
            }
            fh_BackgroundEAReader r = new BackgroundEAReader( target, tmp );
            addIRunnable( r );
            r->getObtainedEASig().connect( sigc::mem_fun( *this, &_Self::OnChildBackgroundEA ));

            LG_BGPROC_D << "GroupBackgroundEAReader() eanamessz:" << eanamessz
                       << " numberPerChild:" << numberPerChild
                       << " numberOfChildren:" << numberOfChildren
                       << " child:" << i
                       << " childlist.sz:" << tmp.size()
                       << " ealist:" << Util::createCommaSeperatedList( tmp )
                       << endl;
            
        }
    }
    
    GroupBackgroundEAReader::~GroupBackgroundEAReader()
    {
    }

    void
    GroupBackgroundEAReader::OnChildBackgroundEA( BackgroundEAReader* bgr,
                                                  const std::string& rdn,
                                                  const std::string& v,
                                                  bool isError )
    {
        LG_BGPROC_D << "OnChildBackgroundEA() bgr:" << bgr << " rdn:" << rdn << " v:" << v << endl;
        
        getObtainedEASig().emit( bgr, rdn, v, isError );
    }
    
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    namespace Factory
    {
        fh_GroupBackgroundEAReader CreateGroupBackgroundEAReader(
            fh_context target, const stringset_t& eanames, int numberOfChildren )
        {
            if( !numberOfChildren )
                numberOfChildren = GROUP_SLAVE_PROCESS_COUNT;

            return new GroupBackgroundEAReader( target, eanames, numberOfChildren );
        }
    };
    
};

