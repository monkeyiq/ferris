/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris mv
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

    $Id: FerrisMove.hh,v 1.4 2010/09/24 21:30:40 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_MOVE_H_
#define _ALREADY_INCLUDED_FERRIS_MOVE_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/Ferrisls.hh>
#include <Ferris/FerrisPopt.hh>
#include <Ferris/FerrisBackup.hh>
#include <Ferris/FerrisCopy.hh>
#include <Ferris/FerrisRemove.hh>

#include <string>

namespace Ferris
{

    class FerrisMv;
    FERRIS_SMARTPTR( FerrisMv, fh_mv );

    class MovePopTableCollector;
    FERRIS_SMARTPTR( MovePopTableCollector, fh_mv_collector );

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    class FERRISEXP_API MovePopTableCollector
        :
        public basic_PopTableCollector,
        public Handlable
    {
        fh_mv mv;
        
        unsigned long Force;
        unsigned long Interactive;
        unsigned long Verbose;
        unsigned long ShowVersion;
        unsigned long Sloth;
        unsigned long AutoClose;
        unsigned long ShowMeter;
        unsigned long UpdateMode;
        unsigned long StripTrailingSlashes;
        unsigned long PerformBackups;
        unsigned long ArchiveMode;
        const char* BackupSuffix;
        const char* PerformBackupsWithMode;
        const char* ExplicitSELinuxContext;
        const char* ExplicitSELinuxType;
        unsigned long CloneSELinuxContext;
        unsigned long DstIsDirectory;
        
    public:
        MovePopTableCollector();
        
        virtual void poptCallback(poptContext con,
                                  enum poptCallbackReason reason,
                                  const struct poptOption * opt,
                                  const char * arg,
                                  const void * data);
        
        void reset();
        void ArgProcessingDone( poptContext optCon );
        struct ::poptOption* getTable( fh_mv _mv );
    };

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    class FERRISEXP_API FerrisMv
        :
        public Handlable
    {
        typedef FerrisMv _Self;
        
    public:

        FerrisMv();
    
        std::string getSrcURL();
        void setSrcURL( const std::string& s );
        void setDstURL( const std::string& s );
        void move();

        void setMovingToDir( bool v );
        void setBackupSuffix( const std::string& s );
        void setExplicitSELinuxContext( const std::string& s );
        void setExplicitSELinuxType( const std::string& s );
        void setCloneSELinuxContext( bool v );
        void setPerformBackups( const std::string& s );
        void setForce( bool v );
        void setInteractive( bool v );
        void setVerbose( bool v );
        void setUpdateMode( bool v );
        void setStripTrailingSlashes( bool v );
        void setShowMeter( bool v );
        void setSloth( bool v );
        void setAutoClose( bool v );
        void setDstIsDirectory( bool v );

        fh_mv_collector getPoptCollector();

        typedef sigc::signal< void (
                               FerrisMv&,            // thisobj,
                               std::string,          // srcDescription,
                               std::string,          // dstDescription,
                               std::string)           // reason
                               > SkippingContextSignal_t;
    
        SkippingContextSignal_t& getSkippingContextSignal();
        
        typedef FerrisCopy::m_AttributeUpdater_t m_AttributeUpdater_t;
        m_AttributeUpdater_t& getAttributeUpdaterSignal();
        
    protected:

        bool m_AttributeUpdaterInUse;
        
        bool MovingToDir;
        bool Force;
        bool Interactive;
        bool Verbose;
        bool UpdateMode;
        bool StripTrailingSlashes;
        bool ShowMeter;
        bool Sloth;
        bool AutoClose;
        bool DstIsDirectory;
        bool SpecialCaseDstIsDirectory;
        
        /**
         * If we are 'AutoClose' we dont auto close if we have
         * interacted with the user
         */
        bool hadUserInteraction;

        

        virtual void
        OnSkippingContext( FerrisMv& thisobj,
                           std::string srcDescription,
                           std::string dstDescription,
                           std::string reason );

    private:


        std::string srcURL;
        std::string dstURL;
        fh_mv_collector Collector;

        void handleSingleFileBackup( const std::string& p );
        bool SameFileSystem( const std::string& sname, const std::string& dname  );
        Util::BackupMaker backupMaker;

        std::string getSrcName();
        virtual bool handleUpdateMode( const std::string& oldrdn, const std::string& newrdn );
        /**
         * Returns 1 if things should proceed and 0 if this context should be skipped
         */
        virtual bool handleInteractiveMode( const std::string& oldrdn, const std::string& newrdn );
        virtual bool handleVerboseMode( const std::string& oldrdn, const std::string& newrdn );

        virtual fh_cp getCopyObject();
        virtual fh_rm getRemoveObject();
        void crossVolumeMove();
        void maybeStripTrailingSlashes();

        SkippingContextSignal_t SkippingContextSignal;

        std::string m_ExplicitSELinuxContext;
        std::string m_ExplicitSELinuxType;
        bool m_CloneSELinuxContext;
    };
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    namespace Priv
    {
        FERRISEXP_API struct ::poptOption* getMovePopTableCollector( fh_mv mv );
    };
    
#define FERRIS_MOVE_OPTIONS(mv) { 0, 0, POPT_ARG_INCLUDE_TABLE, \
/**/  ::Ferris::Priv::getMovePopTableCollector(mv),     \
/**/  0, "common move options:", 0 },
    
};
#endif
