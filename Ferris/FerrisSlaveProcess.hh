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

    $Id: FerrisSlaveProcess.hh,v 1.2 2010/09/24 21:30:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_SLAVE_PROCESS_H_
#define _ALREADY_INCLUDED_FERRIS_SLAVE_PROCESS_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/AsyncIO.hh>

namespace Ferris
{
    class FerrisSlaveProcess
        :
        public Handlable
    {
    public:
        virtual fh_runner getRunner() = 0;
        
        typedef sigc::signal< void ( ChildStreamServer*, fh_runner, int, int ) > ChildCompleteSig_t;
        virtual ChildCompleteSig_t& getChildCompleteSig() = 0;

        typedef sigc::signal< void ( fh_xstreamcol ) > MessageArrivedSig_t;
        virtual MessageArrivedSig_t& getMessageArrivedSig() = 0;
    };

    
    fh_FerrisSlaveProcess
    CreateFerrisSlaveProcess( const std::string& taskname,
                              const std::string& cmd );

    /********************************************************************************/
    /********************************************************************************/

    /**
     * This is a manager for some background process. For example BackgroundEAReader.
     * The main goal of the class is to spawn a new background process in Run() and
     * handle interaction with it, perhaps using a fh_FerrisSlaveProcess and handling
     * interaction with a OnAsyncXMLMessage callback.
     */
    class IRunnableSlaveProcessManager;
    FERRIS_SMARTPTR( IRunnableSlaveProcessManager, fh_IRunnableSlaveProcessManager );
    
    class IRunnableSlaveProcessManager
        : public Handlable
    {
        typedef IRunnableSlaveProcessManager _Self;
    protected:
        fh_FerrisSlaveProcess m_asyncSlave;
        IRunnableSlaveProcessManager( const std::string& processName );
        void OnAsyncChildComeplete( ChildStreamServer* css, fh_runner r, int status, int estatus );
        fh_FerrisSlaveProcess getSlaveProcess();
    public:
        virtual void Run() = 0;
    };
    

    class GroupIRunnableSlaveProcessManager;
    FERRIS_SMARTPTR( GroupIRunnableSlaveProcessManager, fh_GroupIRunnableSlaveProcessManager );
    class GroupIRunnableSlaveProcessManager
        : public IRunnableSlaveProcessManager
    {
    protected:
        GroupIRunnableSlaveProcessManager();
        void addIRunnable( fh_IRunnableSlaveProcessManager r );
        
    public:
        
        virtual void Run();

    private:

        typedef std::list< fh_IRunnableSlaveProcessManager > m_runnables_t;
        m_runnables_t m_runnables;
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class BackgroundEAReader;
    FERRIS_SMARTPTR( BackgroundEAReader, fh_BackgroundEAReader );
    class GroupBackgroundEAReader;
    FERRIS_SMARTPTR( GroupBackgroundEAReader, fh_GroupBackgroundEAReader );
    
    class BackgroundEAReaderSignals
    {
    public:
        typedef sigc::signal< void (
                               BackgroundEAReader*, // this
                               const std::string&,  // rdn,
                               const std::string&,  // value,
                               bool ) >               // if( true ) value is error for not reading rdn
        ObtainedEASig_t;
        ObtainedEASig_t& getObtainedEASig()
            {
                return m_ObtainedEASig;
            }
    private:
        ObtainedEASig_t       m_ObtainedEASig;
    };
    
    class BackgroundEAReader
        : public IRunnableSlaveProcessManager,
          public BackgroundEAReaderSignals
    {
        typedef BackgroundEAReader _Self;
        
    public:
        
        BackgroundEAReader( fh_context target, const stringset_t& eanames );
        ~BackgroundEAReader();


        virtual void Run();
        
    private:

        void OnAsyncXMLMessage( fh_xstreamcol h );

        fh_context            m_target;
        stringset_t           m_eanames;
        stringset_t::iterator m_eanamesIter;
    };

    /******************************/
    /******************************/
    /******************************/

    class GroupBackgroundEAReader
        : public GroupIRunnableSlaveProcessManager,
          public BackgroundEAReaderSignals
    {
        typedef GroupBackgroundEAReader _Self;
        
    public:
        
        GroupBackgroundEAReader( fh_context target, const stringset_t& eanames, int numberOfChildren = 6 );
        ~GroupBackgroundEAReader();


    private:

        void OnChildBackgroundEA( BackgroundEAReader* bgr,
                                  const std::string& rdn,
                                  const std::string& v,
                                  bool isError );
    };

    namespace Factory
    {
        fh_GroupBackgroundEAReader CreateGroupBackgroundEAReader(
            fh_context target, const stringset_t& eanames, int numberOfChildren = 0 );
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

};
#endif
