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

    $Id: Native.hh,v 1.11 2010/09/24 21:30:55 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_NATIVE_H_
#define _ALREADY_INCLUDED_FERRIS_NATIVE_H_

// this file is not part of the public headers
#include "config.h"


#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris_private.hh>
#include <Ferris/TypeDecl.hh>

#include <string>
#include <list>
#include <map>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef HAVE_LIBFILE
#include <libfile.h>
#endif
#ifdef HAVE_EFSD
#include <libefsd.h>
#endif
#ifdef HAVE_LIBRPM
#include <rpm/rpmdb.h>
#include <rpm/rpmfi.h>
#include <rpm/rpmcli.h>
#include <rpm/rpmerr.h>
#endif
#include <Ferris/RPM_private.hh>

#include <Fampp2.hh>
#include <Fampp2GlibSupport.hh>

#include <Ferris/Attribute.hh>

#include <sys/param.h>
#include <sys/mount.h>

#ifndef PLATFORM_OSX
#include <sys/vfs.h>
#endif

namespace Ferris
{
    fh_stringstream SL_getPreallocationAtTailStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, int m = 0 );

#ifdef HAVE_LIBRPM
    struct rpmState;
    FERRIS_SMARTPTR( rpmState, fh_rpmState );
#endif

    FERRISEXP_DLLLOCAL stringset_t& getNativeStatelessEANames();
    
    class FERRISEXP_API NativeContext
        :
        public StateLessEAHolder< NativeContext >
    {
        typedef NativeContext _Self;
        typedef StateLessEAHolder< NativeContext > _Base;

        friend class Ferrisls_aggregate_data;

        friend fh_stringstream SL_getPreallocationAtTailStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, int m );
        friend void
        SL_updatePreallocationAtTail( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss );

        friend fh_stringstream getTimeStrFTimeStream( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        friend fh_istream SL_getTimeStrFTimeIStream( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        friend fh_iostream SL_getTimeStrFTimeIOStream( NativeContext* c, const std::string& rdn, EA_Atom* atom );

        friend fh_istream SL_getFSType( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        friend fh_istream SL_getFSBlockSize( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        friend fh_istream SL_getFSBlockCount( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        friend fh_istream SL_getFSFreeBlockCount( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        friend fh_istream SL_getFSFreeSize( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        friend fh_istream SL_getFSAvailableBlockCount( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        friend fh_istream SL_getFSFileNodesTotal( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        friend fh_istream SL_getFSFileNodesFree( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        friend fh_istream SL_getFSID( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        friend fh_istream SL_getFSMaxNameLength( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        friend fh_istream SL_getFSName( NativeContext* c, const std::string& rdn, EA_Atom* atom );

        
        friend class NativeVFS_RootContextDropper;

        fh_context getNonKernelLinkTarget();

        
        virtual Context* priv_CreateContext( Context* parent, std::string rdn );
        virtual fh_context priv_getSubContext( const std::string& rdn )
            throw( NoSuchSubContext );

        fh_context
        native_readSubContext( const std::string& rdn, bool created = false, bool checkIfExistsAlready = true )
            throw( NoSuchSubContext, FerrisNotSupportedInThisContext );
        
        
        
        typedef Fampp::fh_fampp_req FamReq_t;
        FamReq_t FamReq;

        void setupFAM();

        std::string OnFamppEvent( std::string filename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev );

        void OnFamppChangedEvent( std::string filename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev );
        void OnFamppDeletedEvent( std::string filename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev );
        void OnFamppStartExecutingEvent( std::string filename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev );
        void OnFamppStopExecutingEvent( std::string filename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev );
        void OnFamppCreatedEvent( std::string filename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev );
        void OnFamppMovedEvent( std::string filename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev );
        void OnFamppAcknowledgeEvent( std::string filename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev );
        void OnFamppExistsEvent( std::string filename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev );
        void OnFamppEndExistEvent( std::string filename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev );


    protected:

//        struct ::statfs getStatFS();
        struct statfs getStatFS();

        virtual void updateMetaData();

        friend struct FollowLinks;
        struct stat sb_follow;
        struct stat sb_dont_follow;
        Version_t sb_follow_version;
        Version_t sb_dont_follow_version;

        guint8
/**/     DontHaveRPMEntry:1, //< if we have already failed to find rpm info for file.
/**/     IsDanglingLink:1,   //< for softlinks for which the target doesn't exist.
                        //< if IsDanglingLink is true then sb_follow==sb_dont_follow.
                        //< if IsDanglingLink then you should report an error attempting
                        //< to access sb_follow data.
/**/     FamppChangedEventConnected:1; //< only monitor changed once per directory
        
        
        
    public:
        
#ifdef HAVE_LIBFILE
        enum native_libfile_detection_bits {
            NATIVE_LIBFILE_MIME = (1<<1),
            NATIVE_LIBFILE_TYPE = (1<<2)
        };
        friend void NativeContext__mimetype_cb( libfile_op* x );
        friend void NativeContext__filetype_cb( libfile_op* x );
//         fh_istream getMimetypeStream( Attribute* attr );
//         fh_istream getFiletypeStream( Attribute* attr );
        
        
        std::string Mimetype;
        std::string Filetype;
//     libfile_op* libfile_mime_client;
//     libfile_op* libfile_type_client;

        libfile_op* getFileMimeClient();
        libfile_op* getFileTypeClient();
    
        Version_t libfile_mime_Version;
        Version_t libfile_type_Version;
//    void setupLibFile(libfile_op*&, int argc, char*const argv[], void (*cb)(libfile_op*));
//    void setupLibFile(native_libfile_detection_bits b);
        void ensureMimeAndFileTypeUpToDate(native_libfile_detection_bits b);

    public:
        /*************************************************************
         * Multiple mimetype engine support
         *************************************************************/
       virtual std::string priv_getMimeType( bool fromContent = false );
       virtual std::string getFileType();
#endif

#ifdef HAVE_EFSD
    public:
        static GMainLoop*      m_efsdGML;
        static EfsdConnection* m_efsdConnection;
        static GIOChannel*     m_efsdChannel;
        static bool            m_waitingForMimetype;
        static bool            m_efsdTimedOut;
        std::string     Mimetype;
    protected:
        void openEFSDConnection();
        virtual std::string priv_getMimeType( bool fromContent = false );
#endif
#ifdef HAVE_KDE3
        std::string     Mimetype;
        virtual std::string priv_getMimeType( bool fromContent = false );
#endif
        
    protected:

        virtual ferris_ios::openmode getSupportedOpenModes();


        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception);
        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception);

        virtual fh_context SubCreate_file( fh_context c, fh_context md );

//        friend fh_context SL_SubCreate_dir ( fh_context c, fh_context md );
        friend fh_context SL_SubCreate_softlink( fh_context c, fh_context md );
        friend fh_context SL_SubCreate_hardlink( fh_context c, fh_context md );
        friend fh_context SL_SubCreate_fifo( fh_context c, fh_context md );
        friend fh_context SL_SubCreate_special( fh_context c, fh_context md );
        friend fh_context SL_SubCreate_db4 ( fh_context c, fh_context md );
        

        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );
    

    
        virtual void priv_read();


        virtual void priv_createAttributes();
        virtual void createStateLessAttributes( bool force = false );
        void adjustREAForAttributeIfPresent( std::string& rea, const std::string& s );
        virtual std::string priv_getRecommendedEA();
        virtual bool getHasSubContextsGuess();

        virtual bool supportsReClaim();
        virtual bool supportsRename();
        virtual bool supportsMonitoring();
        virtual bool priv_supportsShortCutLoading()  { return true; }
        
        
        virtual fh_context priv_rename( const std::string& rdn,
                                        const std::string& newPath,
                                        bool TryToCopyOverFileSystems = true,
                                        bool OverWriteDstIfExists = false );

        virtual bool supportsRemove();
        virtual void priv_remove( fh_context c );

        fh_context SubCreate_dir( fh_context c, fh_context md );
        fh_context SubCreate_softlink( fh_context c, fh_context md );
        fh_context SubCreate_hardlink( fh_context c, fh_context md );
        fh_context SubCreate_fifo( fh_context c, fh_context md );
        fh_context SubCreate_special( fh_context c, fh_context md );

//        fh_istream getLinkTarget( Context*, const std::string&, EA_Atom* attr );
        fh_istream getLinkTargetRelative( Context*, const std::string&, EA_Atom* attr );

        void ProcessAllFamppEvents();

        virtual void tryToFindAttributeByOverMounting( const std::string& eaname = "" );
        
    public:

        NativeContext();
        virtual ~NativeContext();

        virtual bool isDir();
        virtual long priv_guessSize() throw();

        static const std::string CreateObjectType_k;
        static const std::string CreateObjectType_v_Dir;
        static const std::string CreateObjectType_v_File;

        const struct stat& getStat_Follow();
        const struct stat& getStat_DontFollow();

        void bumpVersion();

        void public_updateMetaData()
            {
                updateMetaData();
            }
        
//     protected:

//         virtual void getTypeInfos( TypeInfos_t& l )
//             {
//                 l.push_back( typeid( _Self ) );
//             }
        

        bool public_hasOverMounter()
            {
                return hasOverMounter();
            }


        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
#ifdef HAVE_LIBRPM

        fh_rpmState get_rpm_header( rpmdb db );
        fh_rpmState get_rpmfi( rpmdb db );

        static fh_stringstream SL_getRPMVerifyFlag( NativeContext*,int flag);
        static fh_stringstream SL_getRPMVerifySize( NativeContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMVerifyMode( NativeContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMVerifyMD5( NativeContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMVerifyDevice( NativeContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMVerifyOwner( NativeContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMVerifyGroup( NativeContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMVerifyMTime( NativeContext*,const std::string&,EA_Atom*);

        static fh_stringstream SL_getRPMFileFlag( NativeContext* c, rpmfileAttrs a );
        static fh_stringstream SL_getRPMIsConfig( NativeContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMIsDoc( NativeContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMIsGhost( NativeContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMIsLicense( NativeContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMIsPubkey( NativeContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMIsReadme( NativeContext*,const std::string&,EA_Atom*);

        static fh_stringstream SL_getRPMHeaderString( NativeContext*, int_32 );
        static fh_stringstream SL_getRPMPackage( NativeContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMVersion( NativeContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMRelease( NativeContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMInfoURL( NativeContext*,const std::string&,EA_Atom*);

        static fh_stringstream SL_getRPMVendor( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getRPMDistribution( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getRPMLicense( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getRPMPackager( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getRPMGroup( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getRPMBuildtime( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getRPMSummary( NativeContext* c, const std::string& rdn, EA_Atom* atom );
        
        
#endif

    protected:
        virtual void imageEAGenerator_priv_createAttributes();

        void
        SLEA( const std::string& rdn,
              const StateLessIEA_t&  fi,
              XSDBasic_t sct = XSD_UNKNOWN )
            {
                getNativeStatelessEANames().insert( rdn );
                tryAddStateLessAttribute( rdn, fi, sct );
            }
        void
        SLEAPI( const std::string& rdn,
                const StateLessIEA_PassedInStream_t&  fi,
                XSDBasic_t sct )
            {
                getNativeStatelessEANames().insert( rdn );
                tryAddStateLessAttributePI( rdn, fi, sct );
            }
        void
        SLEA( const std::string& rdn,
              const StateLessIEA_t&  fi,
              const StateLessIOEA_t& fio,
              const StateLessIOClosedEA_t& fioc,
              XSDBasic_t sct = XSD_UNKNOWN )
            {
                getNativeStatelessEANames().insert( rdn );
                tryAddStateLessAttribute( rdn, fi, fio, fioc, sct );
            }
        void
        SLEAPI( const std::string& rdn,
                const StateLessIEA_PassedInStream_t&  fi,
                const StateLessIOClosedEA_t& fioc,
                XSDBasic_t sct )
            {
                getNativeStatelessEANames().insert( rdn );
                tryAddStateLessAttributePI( rdn, fi, fioc, sct );
            }

    public:
        virtual void read( bool force = 0 );

        virtual time_t getMTime();
        
    };

    
    namespace System
    {
        FERRISEXP_DLLLOCAL bool gotRoot();
    };
    
    
    namespace Shell
    {
        FERRISEXP_DLLLOCAL bool canChangeFileToUser( uid_t u );
        FERRISEXP_DLLLOCAL bool canChangeFileToGroup( gid_t g );
    };
    
};

#endif
