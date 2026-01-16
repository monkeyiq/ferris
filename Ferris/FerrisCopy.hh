/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris cp
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

    $Id: FerrisCopy.hh,v 1.12 2010/09/24 21:30:35 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_COPY_H_
#define _ALREADY_INCLUDED_FERRIS_COPY_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/Ferrisls.hh>
#include <Ferris/FerrisPopt.hh>
#include <Ferris/FerrisBackup.hh>

#include <errno.h>
#include <sigc++/sigc++.h>

#include <string>


namespace FerrisUI
{
    class FerrisCopy_SignalHandler;
};

namespace Ferris
{

    class FerrisCopy;
    FERRIS_SMARTPTR( FerrisCopy, fh_cp );

    class ContextPopTableCollector;
    FERRIS_SMARTPTR( ContextPopTableCollector, fh_cp_collector );

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    class FERRISEXP_API ContextPopTableCollector
            :
            public basic_PopTableCollector,
            public Handlable
    {
        fh_cp cp;

        const char* DstNameCSTR              ;
        const char* SrcAttr                  ;
        const char* DstAttr                  ;
        unsigned long DstAttrNoCreate        ;
        const char* BackupSuffix             ;
        const char* PerformBackupsWithMode   ;
        const char* HoleCreation             ;
        const char* PreserveAttributeList    ;
        unsigned long Force                  ;
        unsigned long ForceRemovePremptively ;
        unsigned long Interactive            ;
        unsigned long CopySrcWithParents     ;
        unsigned long StripTrailingSlashes   ;
        unsigned long UpdateMode             ;
        unsigned long Verbose                ;
        unsigned long ShowVersion            ;
        unsigned long OneFileSystem          ;
        unsigned long PerformBackups         ;
        unsigned long PreserveFileAttrs      ;
        unsigned long MakeSoftLinksForNonDirs;
        unsigned long MakeSoftLinksForNonDirsAbsolute;
        unsigned long MakeHardLinksForNonDirs;
        unsigned long FollowOnlyForSrcUrl    ;
        unsigned long DontFollowLinks        ;
        unsigned long FollowLinks            ;
        unsigned long Recurse                ;
        unsigned long RecurseAndFlatten      ;
        unsigned long Sloth                  ;
        unsigned long AutoClose              ;
        unsigned long ArchiveMode            ;
        unsigned long ShowMeter              ;
        unsigned long DisableCopyIntoSelfTests;
        unsigned long DstIsDirectory;
        unsigned long SrcIsDirectory;
        unsigned long DoNotTryToOverMountDst;
        unsigned long InputInMemoryMappedMode;
        const char*   AutoInputInMemoryMappedModeSize_CSTR;
        unsigned long OutputInMemoryMappedMode;
        const char*   AutoOutputInMemoryMappedModeSize_CSTR;
        unsigned long useSendfileIfPossible;
        unsigned long SendfileChunkSize;
        unsigned long PreallocateWith_fallocate;
        unsigned long PreallocateWith_ftruncate;
        unsigned long DontFSyncAfterEachFile;
        unsigned long FSyncAfterEachFile;
        
//         unsigned long OutputInDirectMode;
//         const char*   AutoOutputInDirectModeSize_CSTR;

        const char* ExplicitSELinuxType;
        const char* ExplicitSELinuxContext;
        unsigned long CloneSELinuxContext;
        unsigned long PrecacheSourceSize;
        unsigned long DontPrecacheSourceSize;

        unsigned long PreserveRecommendedEA;
        unsigned long DontPreserveRDFEA;
        unsigned long DontPreserveFerrisTypeEA;
        
        
    public:
        ContextPopTableCollector();
        
        virtual void poptCallback(poptContext con,
                                  enum poptCallbackReason reason,
                                  const struct poptOption * opt,
                                  const char * arg,
                                  const void * data);
        
        void reset();
        void ArgProcessingDone( poptContext optCon );
        struct ::poptOption* getTable( fh_cp _cp );
    };


    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    class FERRISEXP_API FerrisCopy
        :
        public Ferrisls_long_display
    {
        typedef Ferrisls_long_display _Base;

        friend class FerrisUI::FerrisCopy_SignalHandler;
        fh_cp_collector Collector;
        
    public:
        
        typedef sigc::signal< void (
                               FerrisCopy&,     // thisobj,
                               fh_context,      // src,
                               fh_context,      // dst,
                               std::string,          // srcDescription,
                               std::string )           // dstDescription
                               > CopyVerboseSignal_t;
        
        CopyVerboseSignal_t& getCopyVerboseSignal();
        
        typedef sigc::signal< void (
                               FerrisCopy&,     // thisobj,
                               std::string,          // srcDescription,
                               std::string )           // reason
                               > SkippingContextSignal_t;
    
        SkippingContextSignal_t& getSkippingContextSignal();

        typedef sigc::signal< bool (
                               FerrisCopy&,     // thisobj,
                               fh_context,      // src
                               fh_context,      // dst
                               std::string,          // srcDescription,
                               std::string )           // dstDescription
                               > AskReplaceContextSignal_t;

        AskReplaceContextSignal_t& getAskReplaceContextSignal();

        typedef sigc::signal< bool (
                               FerrisCopy&,     // thisobj,
                               fh_context,      // src
                               fh_context,      // dst
                               std::string,     // srcDescription,
                               std::string,     // dstDescription
                               fh_attribute )     // dst attr
                               > AskReplaceAttributeSignal_t;

        AskReplaceAttributeSignal_t& getAskReplaceAttributeSignal();

        fh_cp_collector getPoptCollector();
    
    private:
        
        CopyVerboseSignal_t         CopyVerboseSignal;
        SkippingContextSignal_t     SkippingContextSignal;
        AskReplaceContextSignal_t   AskReplaceContextSignal;
        AskReplaceAttributeSignal_t AskReplaceAttributeSignal;
        
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
        bool RecursiveIsInitial;
        Ferrisls ls;

        virtual void workStarting();
        virtual void workComplete();
        std::string dirname( fh_context ctx, std::string s );
        virtual bool ShouldEnterContext(fh_context ctx);
        virtual void EnteringContext(fh_context ctx);
        virtual void LeavingContext(fh_context ctx);
        virtual void ShowAttributes( fh_context ctx );
        virtual void PrintEA( int i,  const std::string& attr, const std::string& EA );

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

        void priv_setSrcContext( fh_context c, bool isInitial = true );
        void priv_setDstContext( const std::string& _dstURL, bool isInitial = true );
        
    public:
        
        typedef sigc::signal< void (
                               FerrisCopy&,     // this
                               std::streamsize, // CurrentPosition
                               std::streamsize, // BlockSize
                               std::streamsize )  // FinalSize (may be -1)
                               > CopyPorgressSignal_t;

        typedef sigc::signal< void (
                               FerrisCopy&,     // this
                               std::streamsize, // CurrentPosition
                               std::streamsize, // BlockSize
                               std::streamsize )  // FinalSize (may be -1)
                               > CopyStartSignal_t;

        typedef sigc::signal< void (
                               FerrisCopy&,     // this
                               std::streamsize, // CurrentPosition
                               std::streamsize, // BlockSize
                               std::streamsize )  // FinalSize (may be -1)
                               > CopyEndSignal_t;
    
        CopyPorgressSignal_t& getCopyPorgressSignal();
        CopyStartSignal_t& getCopyStartSignal();
        CopyEndSignal_t& getCopyEndSignal();

    private:
        CopyPorgressSignal_t CopyPorgressSignal;
        CopyStartSignal_t    CopyStartSignal;
        CopyEndSignal_t      CopyEndSignal;

    
        template< class T >
        T copyTo( fh_istream iss,
                  T oss,
                  const std::streamsize BlockSize = 4096,
                  const std::streamsize FinalSize = -1
            )
            {
                std::streamsize CurrentPosition = 0;
        
                LG_COPY_D << "---------Start copy-----------"
                          << " blocksize:" << BlockSize
                          << " finalsize:" << FinalSize
                          << " CurrentPos:" << CurrentPosition
                          << " oss is good():"<< oss->good()
                          << std::endl;

                getCopyStartSignal().emit( *this, CurrentPosition,
                                           BlockSize, FinalSize );

                char buffer[ BlockSize + 1 ];
                errno = 0;

                while( true )
                {
                    iss.read( buffer, BlockSize );
                    std::streamsize BytesRead = iss.gcount();

                    LG_COPY_D << " iss is good():"<< iss->good() << std::endl;
                    LG_COPY_D << " iss is eof():"<< iss->eof() << std::endl;
                    LG_COPY_D << " iss is bad():"<< iss->bad() << std::endl;
                    LG_COPY_D << " iss is fail():"<< iss->fail() << std::endl;
                    LG_COPY_D << " iss is state:"<< iss->rdstate() << std::endl;
                    LG_COPY_D << " BytesRead:" << BytesRead << std::endl;
            
                    if( iss.eof() )
                    {
                        if( !BytesRead )
                        {
                            /* at end of input */
                            break;
                        }
                        /* Fall through and write the bytes out for the last partial block */
                    }
                    else if( !iss )
                    {
                        /* read error */
                        int eno = errno;
                        fh_stringstream ss;
                        ss << "Failed to copy file."
                           << " src:" << getSourceDescription()
                           << " dst:" << getDestinationDescription()
                           << " read error! ";
//                        cerr << tostr(ss) << std::endl;
                        Throw_CopyFailed( errnum_to_string(tostr(ss), eno), 0 );
                    }
            
                    if( !oss.write( buffer, BytesRead ))
                    {
                        /* write error */
                        int eno = errno;
                        fh_stringstream ss;
                        ss << "Failed to copy file."
                           << " src:" << getSourceDescription()
                           << " dst:" << getDestinationDescription()
                           << " write error! ";
//                        cerr << tostr(ss) << std::endl;
                        Throw_CopyFailed( errnum_to_string(tostr(ss), eno), 0 );
                    }
            
                    CurrentPosition += BytesRead;
                    TotalBytesCopied += BytesRead;
                    LG_COPY_D << " oss is good():"<< oss->good() << std::endl;
                    LG_COPY_D << " oss is state:"<< oss->rdstate() << std::endl;

                    getCopyPorgressSignal().emit( *this,
                                                  CurrentPosition,
                                                  BlockSize,
                                                  FinalSize );
                }

                oss << std::flush;

                if( FinalSize != -1 && CurrentPosition != FinalSize )
                {
                    /* Something strange has happened, not read/write error */

                    fh_stringstream ss;
                    ss << "Failed to copy file."
                       << " src:" << getSourceDescription()
                       << " dst:" << getDestinationDescription()
                       << " CurrentPosition != FinalSize that was desired."
                       << " CurrentPosition:" << CurrentPosition
                       << " FinalSize:" << FinalSize;
                    Throw_CopyFailed( tostr(ss), 0 );
                }
        
        
        
        
//         copy( istreambuf_iterator<char>(iss),
//               istreambuf_iterator<char>(),
//               ostreambuf_iterator<char>(oss));
        
                LG_COPY_D << " iss is good():"<< iss->good() << std::endl;
                LG_COPY_D << " iss is eof():"<< iss->eof() << std::endl;
                LG_COPY_D << " iss is state:"<< iss->rdstate() << std::endl;
                LG_COPY_D << " oss is good():"<< oss->good() << std::endl;
                LG_COPY_D << " oss is state:"<< oss->rdstate() << std::endl;

//         char xch;
//         iss >> xch;
//         if( !iss->eof() )
//         {
//             fh_stringstream ss;
//             ss << "Failed to copy file, path: " << getDirPath()
//                << " iss is not at eof.";
//             Throw_CopyFailed( tostr(ss), this );
//         }

                getCopyEndSignal().emit( *this, CurrentPosition,
                                         BlockSize, FinalSize );
        
                return oss;
            }


        /*
         * return true if we are trying to copy into ourself.
         */
        bool AttemptToCopyIntoSelf( fh_context src, fh_context dstparent );
        void perform_copy();

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

        std::string srcURL;
        /* This is the srcURL that was given originaily for a recursive parse, it is
         * also relative if the srcURL was relative to begin with */
        std::string srcURL_Root; 
        std::string dstURL;

        /* this is cached so that the dstURL can change and then be restored in the
         * next copy() call */
        std::string dstURL_setByUser; 
    
        std::string SrcAttr;
        std::string DstAttr;

        fh_context src;       /* Source context */
        fh_context dstparent; /* parent(dst); needed when dc does not yet exist */
        fh_context dst;       /* destination context */

        bool DstAttrNoCreate;
        bool ForceOverWrite;
        bool ForceRemovePremptively;
        bool Interactive;
        bool Verbose;
        bool UpdateMode;
        bool StripTrailingSlashes;
        bool CopySrcWithParents;
        bool OneFileSystem;
        dev_t CurrentFileSystemDev;
        bool FlattenSpecialToFile;
        bool PreserveObjectMode;
        bool PreserveMTime;
        bool PreserveATime;
        bool PreserveOwner;
        bool PreserveGroup;
        bool FollowOnlyForSrcUrl;
        bool DontFollowLinks;
        bool Recurse;
        bool CopyIntoSelfTests;

        bool perform_fsyncAfterEachFile;
        bool perform_preallocation;
        bool preallocate_with_fallocate;
        bool preallocate_with_ftruncate;

        int m_DestinationFDCache;
        
    protected:        
        std::streamsize TotalBytesCopied;
        std::streamsize TotalBytesToCopy;
        bool Sloth;
        bool AutoClose;

        /**
         * If we are 'AutoClose' we dont auto close if we have
         * interacted with the user
         */
        bool hadUserInteraction;
    
    public:
        enum HoleMode_t
        {
            HOLEMODE_NEVER,
            HOLEMODE_ALWAYS,
            HOLEMODE_HEURISTIC
        };
        /**
         * Signal which is fired for each file/dir to allow easy control over
         * changing the lstat(), selinux, or other metadata for the destination.
         * Signal is fired when destination file has been created and fully
         * setup as per standard options like -Z, -a etc. ie, signal is the
         * last thing to see the destination file.
         */
        typedef sigc::signal< void (
                               FerrisCopy*,     // thisobj,
                               fh_context,      // src,
                               fh_context,      // dst,
                               std::string,     // srcDescription,
                               std::string )      // dstDescription
                               > m_AttributeUpdater_t;
        
    private:
        m_AttributeUpdater_t m_AttributeUpdater;
        bool                 m_AttributeUpdaterInUse;
        HoleMode_t HoleMode;

        typedef std::list< std::string > PreserveAttributeList_t;
        PreserveAttributeList_t PreserveAttributeList;
        
        bool MakeSoftLinksForNonDirs;
        bool MakeSoftLinksForNonDirsAbsolute;
        bool MakeHardLinksForNonDirs;
        bool DstIsDirectory;
        bool SrcIsDirectory;
        bool DoNotTryToOverMountDst;
        bool InputInMemoryMappedMode;
        std::streamsize AutoInputInMemoryMappedModeSize;
        bool OutputInMemoryMappedMode;
        std::streamsize AutoOutputInMemoryMappedModeSize;
        bool m_useSendfileIfPossible;
        long m_sendfileChunkSize;
        
//         bool OutputInDirectMode;
//         std::streamsize AutoOutputInDirectModeSize;

        Util::BackupMaker backupMaker;

        bool        m_CloneSELinuxContext;
        bool        m_PrecacheSourceSize;
        std::string m_ExplicitSELinuxType;
        std::string m_ExplicitSELinuxContext;

        bool m_PreserveRecommendedEA;
        bool m_PreserveRDFEA;
        bool m_PreserveFerrisTypeEA;
        
        void maybeStripTrailingSlashes();

        fh_context priv_CreateObj( fh_context c, std::string s );
        
public:
    
        FerrisCopy();

        bool setFSyncAfterEachFile( bool v );
        bool setPreallocateWith_fallocate( bool v );
        bool setPreallocateWith_ftruncate( bool v );
        bool setCopyIntoSelfTests( bool v );
        void setRecurse( bool v );
        void setSloth( bool v );
        void setAutoClose( bool v );
        void setHoleMode( HoleMode_t m );
        void setPreserveAttributeList( const std::string& v );        
        void setFollowOnlyForSrcUrl( bool v );
        void setDontFollowLinks( bool v );
        void setMakeSoftLinksForNonDirsAbsolute( bool v );
        void setMakeSoftLinksForNonDirs( bool v );
        void setMakeHardLinksForNonDirs( bool v );
        void setPreserveMTime( bool v );
        void setPreserveATime( bool v );
        void setPreserveOwner( bool v );
        void setPreserveGroup( bool v );
        void setPreserveObjectMode( bool v );
        void setFlattenSpecialToFile( bool v );
        void setBackupMaker( const Util::BackupMaker& b );
        void setBackupSuffix( const std::string& s );
        void setExplicitSELinuxContext( const std::string& s );
        void setExplicitSELinuxType( const std::string& s );
        void setCloneSELinuxContext( bool v );
        void setPrecacheSourceSize( bool v );
        bool shouldPrecacheSourceSize();
        void setPreserveRecommendedEA( bool v );
        void setPreserveRDFEA( bool v );
        void setPreserveFerrisTypeEA( bool v );
        void setPerformBackups( const std::string& s );
        void setOneFileSystem( bool v );
        void setForceOverWrite( bool v );
        void setForceRemovePremptively( bool v );
        void setVerbose( bool v );
        void setStripTrailingSlashes( bool v );
        void setCopySrcWithParents( bool v );
        void setInteractive( bool v );
        void setUpdateMode( bool v );
        void setSrcAttr( const std::string& s );
        void setDstAttr( const std::string& s );
        void setDstAttrNoCreate( bool v );
        void setSrcURL( const std::string& s );
        void setDstURL( const std::string& s );
        void setDstIsDirectory( bool v );
        void setSrcIsDirectory( bool v );
        void setDoNotTryToOverMountDst( bool v );
        void setInputInMemoryMappedMode( bool v );
        void setAutoInputInMemoryMappedModeSize( std::streamsize v );
        void setOutputInMemoryMappedMode( bool v );
        void setAutoOutputInMemoryMappedModeSize( std::streamsize v );
        void setUseSendfileIfPossible( bool v );
        void setSendfileChunkSize( long v );
        
        m_AttributeUpdater_t& getAttributeUpdaterSignal();
//         void setOutputInDirectMode( bool v );
//         void setAutoOutputInDirectModeSize( std::streamsize v );
        std::string getDstName();
        
        fh_iostream OpenDestination( fh_attribute& realdst,
                                     std::streamsize inputsz = 0,
                                     bool firstcall = true );

        std::string getEA_DontFollow( fh_context c, const std::string& n, std::string d );
        std::string getEA( fh_context c, const std::string& n, std::string d );

        bool srcIsCmdLineParam();
        fh_context CreateObj( fh_context c, std::string s );

        void PreserveEA( const std::string& eaname, bool tryCreate = 0,
                         const std::string& ExplicitPluginName = "" );
        void PreserveEA();

        fh_ostream WrapForHoles( fh_context src, fh_ostream ss );
        std::string getSourceDescription();
        std::string getDestinationDescription( fh_context c = 0 );
        void perform();
        void copy();

        std::streamsize getTotalBytesCopied();
        std::streamsize getTotalBytesToCopy();
        gdouble getTotalPercentageOfBytesCopied();
        
        
    };

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/


    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    
    namespace Priv
    {
        FERRISEXP_API struct ::poptOption* getCopyPopTableCollector( fh_cp cp );
    };
    
#define FERRIS_COPY_OPTIONS(cp) { 0, 0, POPT_ARG_INCLUDE_TABLE, \
/**/  ::Ferris::Priv::getCopyPopTableCollector(cp),     \
/**/  0, "common copy options:", 0 },

};
#endif
