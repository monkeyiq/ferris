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

    $Id: libipc.cpp,v 1.5 2010/09/24 21:31:38 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <Trimming.hh>

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <pwd.h>
#include <grp.h>
#include <sys/types.h>


#ifndef SEM_STAT
#define SEM_STAT	18
#define SEM_INFO	19
#endif

/* Some versions of libc only define IPC_INFO when __USE_GNU is defined. */
#ifndef IPC_INFO
#define IPC_INFO        3
#endif



using namespace std;
namespace Ferris
{
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short int *array;
        struct seminfo *__buf;
    };

    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };
    


    int getModeFromMetaData( fh_context md )
        {
            string modestr = getStrSubCtx( md, "mode", "-1" );
            int mode = toType<int>( modestr );
            LG_IPCCTX_D << "getModeFromMetaData() modestr:" << modestr
                        << " mode:" << mode
                        << endl;
    
            if( mode == -1 )
            {
                mode = 0
                    | (toint(getStrSubCtx( md, "user-readable", "1" )) ? (SHM_R>>0) : 0)
                    | (toint(getStrSubCtx( md, "user-writable", "1" )) ? (SHM_W>>0) : 0)
                    | (toint(getStrSubCtx( md, "group-readable", "0" )) ? (SHM_R>>3) : 0)
                    | (toint(getStrSubCtx( md, "group-writable", "0" )) ? (SHM_W>>3) : 0)
                    | (toint(getStrSubCtx( md, "other-readable", "0" )) ? (SHM_R>>6) : 0)
                    | (toint(getStrSubCtx( md, "other-writable", "0" )) ? (SHM_W>>6) : 0)
                    | 0;
            }

            bool ignoreUMask = toint(getStrSubCtx( md, "ignore-umask", "0" ));
            if( !ignoreUMask )
            {
                mode_t um = umask( 0 );
                umask( um );

                mode &= (~um);
            }

            LG_IPCCTX_D << "create mode:" << mode << endl;
            return mode;
        }


    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    class FERRISEXP_CTXPLUGIN sysvipcContext
        :
        public StateLessEAHolder< sysvipcContext, leafContext > 
    {
        typedef sysvipcContext                                   _Self;
        typedef StateLessEAHolder< sysvipcContext, leafContext > _Base;

    public:
        static fh_istream SL_getShmKeyStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_iostream SL_getShmUserOwnerNumberStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom );
        static void SL_setShmUserOwnerNumberStream( sysvipcContext* c, const std::string& rdn,
                                                 EA_Atom* atom, fh_istream ss );
        static fh_iostream SL_getShmGroupOwnerNumberStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom );
        static void SL_setShmGroupOwnerNumberStream( sysvipcContext* c, const std::string& rdn,
                                                     EA_Atom* atom, fh_istream ss );
        static fh_istream SL_getShmUserCreatorNumberStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_istream SL_getShmGroupCreatorNumberStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_iostream SL_getShmProtectionRawStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom );
        static void SL_setShmProtectionRawStream( sysvipcContext* c, const std::string& rdn,
                                                     EA_Atom* atom, fh_istream ss );
        static fh_istream SL_getShmProtectionLsStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_istream SL_getShmSeqNumStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom );

        static fh_istream SL_getFalse( sysvipcContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_istream SL_getTrue( sysvipcContext* c, const std::string& rdn, EA_Atom* atom );
        
    protected:

        void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( force || virgin() )
                {
#define SLEA tryAddStateLessAttribute

                    SLEA( "key",                  SL_getShmKeyStream, XSD_BASIC_STRING );
                    SLEA( "user-owner-number",    SL_getShmUserOwnerNumberStream, 
                          SL_getShmUserOwnerNumberStream, SL_setShmUserOwnerNumberStream,
                          FXD_UID_T );
                    SLEA( "group-owner-number",   SL_getShmGroupOwnerNumberStream,
                          SL_getShmGroupOwnerNumberStream, SL_setShmGroupOwnerNumberStream,
                          FXD_GID_T );
                    SLEA( "user-creator-number",  SL_getShmUserCreatorNumberStream,  FXD_UID_T  );
                    SLEA( "group-creator-number", SL_getShmGroupCreatorNumberStream, FXD_GID_T  );
                    SLEA( "protection-raw",       SL_getShmProtectionRawStream,      FXD_MODE_T );
                    SLEA( "protection-ls",        SL_getShmProtectionLsStream,       XSD_BASIC_STRING );
                    SLEA( "mode",                 SL_getShmProtectionRawStream,
                          SL_getShmProtectionRawStream, SL_setShmProtectionRawStream,
                          FXD_MODE_T );
                    SLEA( "sequence-number",      SL_getShmSeqNumStream, XSD_BASIC_INT );
                
#undef SLEA
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        
        virtual struct ipc_perm* getPermissions() = 0;
        virtual int ipcset( struct ipc_perm* p ) = 0;

        int sysv_id;

        int getID()
            {
                return toint( getDirName() );
            }
        
    public:

        void setSysVID( int v )
            {
                sysv_id = v;
            }
        
        
        sysvipcContext( Context* parent, std::string rdn )
            :
            _Base( parent, rdn )
            {
            }

        virtual ~sysvipcContext()
            {
            }
        
        ferris_ios::openmode getSupportedOpenModes()
            {
                return ios::in | ios::out | ios::ate | ios::app | ios::trunc | ios::binary;
            }
        
    };


    fh_istream sysvipcContext::SL_getShmKeyStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom )
    {
        struct ipc_perm* perm = c->getPermissions();
        
        fh_stringstream ss;
        ss << perm->__key;
        return ss;
    }

    fh_iostream sysvipcContext::SL_getShmUserOwnerNumberStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom )
    {
        struct ipc_perm* perm = c->getPermissions();

        fh_stringstream ss;
        ss << perm->uid;
        return ss;
    }

    void sysvipcContext::SL_setShmUserOwnerNumberStream( sysvipcContext* c, const std::string& rdn,
                                         EA_Atom* atom, fh_istream ss )
    {
        uid_t owner = 0;
        if( ss >> owner )
        {
            /* copy data incase call fails */
            struct ipc_perm perm = *c->getPermissions();
            perm.uid = owner;
        
//            int rc = shmctl( perm.__seq, IPC_SET, &ds );
            int rc = c->ipcset( &perm );
            if( rc == -1 )
            {
                fh_stringstream ss;
                ss << "Can not change owner of shared memory to:" << owner
                   << " url: " << c->getURL();
                Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
            }
            
            *c->getPermissions() = perm;
        }
        else
        {
            fh_stringstream ss;
            ss << "Must supply user id to change shared segment owner to"
               << " url: " << c->getURL();
            Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
        }
    }
    

    fh_iostream sysvipcContext::SL_getShmGroupOwnerNumberStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom )
    {
        struct ipc_perm* perm = c->getPermissions();

        fh_stringstream ss;
        ss << perm->gid;
        return ss;
    }

    void sysvipcContext::SL_setShmGroupOwnerNumberStream( sysvipcContext* c, const std::string& rdn,
                                          EA_Atom* atom, fh_istream ss )
    {
        gid_t groupid = 0;
        if( ss >> groupid )
        {
            /* copy data incase call fails */
            struct ipc_perm perm = *c->getPermissions();
            perm.gid = groupid;
        
//            int rc = shmctl( ds.shm_perm.__seq, IPC_SET, &ds );
            int rc = c->ipcset( &perm );
            if( rc == -1 )
            {
                fh_stringstream ss;
                ss << "Can not change groupid of shared memory to:" << groupid
                   << " url: " << c->getURL();
                Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
            }
            *c->getPermissions() = perm;
        }
        else
        {
            fh_stringstream ss;
            ss << "Must supply user id to change shared segment groupid to"
               << " url: " << c->getURL();
            Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
        }
    }
    
    fh_istream sysvipcContext::SL_getShmUserCreatorNumberStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom )
    {
        struct ipc_perm* perm = c->getPermissions();

        fh_stringstream ss;
        ss << perm->cuid;
        return ss;
    }

    fh_istream sysvipcContext::SL_getShmGroupCreatorNumberStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom )
    {
        struct ipc_perm* perm = c->getPermissions();

        fh_stringstream ss;
        ss << perm->cgid;
        return ss;
    }

    fh_iostream sysvipcContext::SL_getShmProtectionRawStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom )
    {
        struct ipc_perm* perm = c->getPermissions();

        fh_stringstream ss;
        ss << perm->mode;
        return ss;
    }

    
    void sysvipcContext::SL_setShmProtectionRawStream( sysvipcContext* c, const std::string& rdn,
                                       EA_Atom* atom, fh_istream ss )
    {
        int mode = -1;
        ss >> mode;
        if( mode == -1 )
        {
            fh_stringstream ss;
            ss << "Must supply new mode for shared segment as octal rw-rw-rw- user/group/other mode"
               << " url: " << c->getURL();
            Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
        }
        

        /* copy data incase call fails */
        struct ipc_perm perm = *c->getPermissions();
        perm.mode = mode;
        
//        int rc = shmctl( ds.shm_perm.__seq, IPC_SET, &ds );
        int rc = c->ipcset( &perm );
        if( rc == -1 )
        {
            fh_stringstream ss;
            ss << "Can not change mode of shared memory to:" << mode
               << " url: " << c->getURL();
            Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
        }
        *c->getPermissions() = perm;
    }

    
    fh_istream sysvipcContext::SL_getShmProtectionLsStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom )
    {
        struct ipc_perm* perm = c->getPermissions();
        int mode = perm->mode;

        fh_stringstream ss;
        ss << "-";

        if( mode & (SHM_R>>0) )  ss << "r";
        else                     ss << "-";
        if( mode & (SHM_W>>0) )  ss << "w";
        else                     ss << "-";
        ss << "-";

        if( mode & (SHM_R>>3) )  ss << "r";
        else                     ss << "-";
        if( mode & (SHM_W>>3) )  ss << "w";
        else                     ss << "-";
        ss << "-";

        if( mode & (SHM_R>>6) )  ss << "r";
        else                     ss << "-";
        if( mode & (SHM_W>>6) )  ss << "w";
        else                     ss << "-";
        ss << "-";
        
        return ss;
    }
    
    fh_istream sysvipcContext::SL_getShmSeqNumStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom )
    {
        struct ipc_perm* perm = c->getPermissions();

        fh_stringstream ss;
        ss << perm->__seq;
        return ss;
    }

    fh_istream sysvipcContext::SL_getTrue( sysvipcContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << 1;
        return ss;
    }

    fh_istream sysvipcContext::SL_getFalse( sysvipcContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << 0;
        return ss;
    }

//     fh_istream sysvipcContext::SL_getSizeFourBytesStream( sysvipcContext* c, const std::string& rdn, EA_Atom* atom )
//     {
//         fh_stringstream ss;
//         ss << 4;
//         return ss;
//     }
    
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    
    
    class FERRISEXP_CTXPLUGIN shmContext
        :
        public StateLessEAHolder< shmContext, sysvipcContext > 
    {
        typedef shmContext                                      _Self;
        typedef StateLessEAHolder< shmContext, sysvipcContext > _Base;

        static fh_istream SL_getShmSizeStream( shmContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_istream SL_getShmATimeStream( shmContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_istream SL_getShmDTimeStream( shmContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_istream SL_getShmCTimeStream( shmContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_istream SL_getShmCreatorPIDStream( shmContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_istream SL_getShmLastOperatorPIDStream( shmContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_istream SL_getShmAttachCountStream( shmContext* c, const std::string& rdn, EA_Atom* atom );
        
        struct shmid_ds ds;

    protected:


        virtual int ipcset( struct ipc_perm* p )
            {
                struct shmid_ds localds = ds;
                localds.shm_perm = *p;
                return shmctl( p->__seq, IPC_SET, &localds );
            }
        
        void DetachMemMap( fh_istream& ss, std::streamsize tellp, void *shmaddr )
            {
                LG_IPCCTX_D << "DetachMemMap() data:" << (void*)shmaddr
                            << " tellp:" << tellp
                            << endl;
//                 for( int i=0; i < 5; ++i )
//                 {
//                     char* p = (char*)shmaddr;
//                     LG_IPCCTX_D << " i:" << i
//                                 << " data[" << i << "]:" << hex << (int)p[i]
//                                 << " data[" << i << "]:" << p[i]
//                                 << endl;
//                 }
                
                
                int rc = shmdt( shmaddr );

                if( rc == -1 )
                {
                    int eno = errno;
                    fh_stringstream ss;
                    ss << "Can not unmap shared memory region url:" << getURL();
                    ThrowFromErrno( eno, tostr(ss), this );
                }
            }

        void IOStreamClosed( fh_istream& ss, std::streamsize tellp )
            {
            }
        
        virtual fh_iostream real_getIOStream( ferris_ios::openmode m, int shmflg )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   exception)
            {
                void* data    = shmat( getID(), 0, shmflg );
                int data_size = ds.shm_segsz;

                if( data == (void*)(-1) )
                {
                    int eno = errno;
                    fh_stringstream ss;
                    ss << "Can not memory map shared memory region url:" << getURL();
                    ThrowFromErrno( eno, tostr(ss), this );
                }

                LG_IPCCTX_D << "real_getIOStream() data:" << (void*)data
                            << " size:" << data_size
                            << endl;
                fh_iostream ret = Factory::MakeMemoryIOStream( data, data_size );
                ret.getCloseSig().connect( bind( sigc::mem_fun( *this, &_Self::DetachMemMap ), data )); 
                return ret;
            }
        
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                return real_getIOStream( m, SHM_RDONLY );
            }
        
        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                fh_iostream ret = real_getIOStream( m, 0 );
                ret.getCloseSig().connect(sigc::mem_fun( *this, &_Self::IOStreamClosed )); 
                return ret;
            }
        

        
        void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
#define SLEA tryAddStateLessAttribute

                    SLEA( "size",                 SL_getShmSizeStream,  FXD_FILESIZE );
                    SLEA( "atime",                SL_getShmATimeStream, FXD_UNIXEPOCH_T );
                    SLEA( "dtime",                SL_getShmDTimeStream, FXD_UNIXEPOCH_T );
                    SLEA( "ctime",                SL_getShmCTimeStream, FXD_UNIXEPOCH_T );
                    SLEA( "creator-pid",          SL_getShmCreatorPIDStream,      FXD_PID );
                    SLEA( "last-operator-pid",    SL_getShmLastOperatorPIDStream, FXD_PID );
                    SLEA( "attach-count",         SL_getShmAttachCountStream,     XSD_BASIC_INT );

                    SLEA( "is-dir",               SL_getFalse, XSD_BASIC_BOOL );
                    SLEA( "is-file",              SL_getTrue,  XSD_BASIC_BOOL );
                    SLEA( "is-special",           SL_getFalse, XSD_BASIC_BOOL );
                    SLEA( "is-link",              SL_getFalse, XSD_BASIC_BOOL );
                    SLEA( "has-holes",            SL_getFalse, XSD_BASIC_BOOL );

                    SLEA( "dontfollow-is-dir",    SL_getFalse, XSD_BASIC_BOOL );
                    SLEA( "dontfollow-is-file",   SL_getTrue,  XSD_BASIC_BOOL );
                    SLEA( "dontfollow-is-special",SL_getFalse, XSD_BASIC_BOOL );
                    SLEA( "dontfollow-is-link",   SL_getFalse, XSD_BASIC_BOOL );
                    SLEA( "dontfollow-has-holes", SL_getFalse, XSD_BASIC_BOOL );
                    
#undef SLEA
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }

        /**
         * Given a shmid (a big number) find the metadata for that segment.
         */
        struct shmid_ds getSegmentData( int required_shmid = -1 )
            {
                if( required_shmid == -1 )
                {
                    required_shmid = sysv_id;
                }

                struct shm_info shm_info;
                struct shminfo shminfo;
                int maxid = shmctl (0, SHM_INFO, (struct shmid_ds *) &shm_info);
                if (maxid < 0)
                {
                    fh_stringstream ss;
                    ss << "kernel not configured for shared memory";
                    Throw_FerrisNotReadableAsContext( tostr(ss), this );
                }

                for (int id = 0; id <= maxid; id++)
                {
                    LG_IPCCTX_D << "max:" << maxid << " id:" << id << endl;
                    struct shmid_ds shmseg;
                    int rc = shmctl( id, SHM_STAT, &shmseg);

//                    cerr << "getSegmentData() sysv_id:" << sysv_id << " rc:" << rc << " max:" << maxid << endl;
                    if ( rc < 0) 
                        continue;

                    if( rc == required_shmid )
                        return shmseg;
                }

                fh_stringstream ss;
                ss << "Can not find information for shmid:" << required_shmid << " url:" << getURL();
//                 cerr << tostr(ss) << endl;
                Throw_FerrisNotReadableAsContext( tostr(ss), this );
            }
        void updateSegmentData()
            {
                ds = getSegmentData();
            }
        
        virtual struct ipc_perm* getPermissions()
            {
                return &ds.shm_perm;
            }
        
    public:
        
         shmContext( Context* parent, std::string rdn, int shmid, const struct shmid_ds& ds_ )
            :
            _Base( parent, rdn ),
            ds( ds_ )
            {
                setSysVID( shmid );
                createStateLessAttributes();
            }

         shmContext( Context* parent, std::string rdn, int shmid )
             :
             _Base( parent, rdn ),
             ds( getSegmentData( shmid ) )
            {
                setSysVID( shmid );
                createStateLessAttributes();
            }
        
        virtual ~shmContext()
            {}

        string getRecommendedEA()
            {
                return "protection-ls,user-owner-name,group-owner-name,size-human-readable,"
                    "atime-display,dtime-display,ctime-display,"
                    "attach-count,last-operator-pid,name";
            }
    };
    


    fh_istream shmContext::SL_getShmSizeStream( shmContext* c, const std::string& rdn, EA_Atom* atom )
    {
        const struct shmid_ds& ds = c->ds;

        fh_stringstream ss;
        ss << ds.shm_segsz;
        return ss;
    }
    
    fh_istream shmContext::SL_getShmATimeStream( shmContext* c, const std::string& rdn, EA_Atom* atom )
    {
        c->updateSegmentData();
        const struct shmid_ds& ds = c->ds;

        fh_stringstream ss;
        ss << ds.shm_atime;
        return ss;
    }
    
    fh_istream shmContext::SL_getShmDTimeStream( shmContext* c, const std::string& rdn, EA_Atom* atom )
    {
        c->updateSegmentData();
        const struct shmid_ds& ds = c->ds;

        fh_stringstream ss;
        ss << ds.shm_dtime;
        return ss;
    }
    
    fh_istream shmContext::SL_getShmCTimeStream( shmContext* c, const std::string& rdn, EA_Atom* atom )
    {
        const struct shmid_ds& ds = c->ds;

        fh_stringstream ss;
        ss << ds.shm_ctime;
        return ss;
    }

    fh_istream shmContext::SL_getShmCreatorPIDStream( shmContext* c, const std::string& rdn, EA_Atom* atom )
    {
        const struct shmid_ds& ds = c->ds;

        fh_stringstream ss;
        ss << ds.shm_cpid;
        return ss;
    }
    
    fh_istream shmContext::SL_getShmLastOperatorPIDStream( shmContext* c, const std::string& rdn, EA_Atom* atom )
    {
        c->updateSegmentData();
        const struct shmid_ds& ds = c->ds;

        fh_stringstream ss;
        ss << ds.shm_lpid;
        return ss;
    }
    
    fh_istream shmContext::SL_getShmAttachCountStream( shmContext* c, const std::string& rdn, EA_Atom* atom )
    {
        c->updateSegmentData();
        const struct shmid_ds& ds = c->ds;

        fh_stringstream ss;
        ss << ds.shm_nattch;
        return ss;
    }
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    
    
class FERRISEXP_CTXPLUGIN shmListContext
    :
        public StateLessEAHolder< shmListContext,
                                  RecommendedEACollectingContext< FakeInternalContext > >
{
    typedef shmListContext _Self;
    typedef StateLessEAHolder< shmListContext,
                               RecommendedEACollectingContext< FakeInternalContext > > _Base;

    friend fh_context SL_ipc_SubCreate_file( fh_context c, fh_context md );
    friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
    
    shmListContext*
    priv_CreateContext( Context* parent, string rdn )
        {
            shmListContext* ret = new shmListContext( parent, rdn );
            return ret;
        }
    

protected:

//     int getModeFromMetaData( fh_context md )
//         {
//             string modestr = getStrSubCtx( md, "mode", "-1" );
//             int mode = toType<int>( modestr );
//             LG_IPCCTX_D << "getModeFromMetaData() modestr:" << modestr
//                         << " mode:" << mode
//                         << endl;
    
//             if( mode == -1 )
//             {
//                 mode = 0
//                     | (toint(getStrSubCtx( md, "user-readable", "1" )) ? (SHM_R>>0) : 0)
//                     | (toint(getStrSubCtx( md, "user-writable", "1" )) ? (SHM_W>>0) : 0)
//                     | (toint(getStrSubCtx( md, "group-readable", "0" )) ? (SHM_R>>3) : 0)
//                     | (toint(getStrSubCtx( md, "group-writable", "0" )) ? (SHM_W>>3) : 0)
//                     | (toint(getStrSubCtx( md, "other-readable", "0" )) ? (SHM_R>>6) : 0)
//                     | (toint(getStrSubCtx( md, "other-writable", "0" )) ? (SHM_W>>6) : 0)
//                     | 0;
//             }

//             bool ignoreUMask = toint(getStrSubCtx( md, "ignore-umask", "0" ));
//             if( !ignoreUMask )
//             {
//                 mode_t um = umask( 0 );
//                 umask( um );

//                 mode &= (~um);
//             }

//             LG_IPCCTX_D << "create mode:" << mode << endl;
//             return mode;
//         }


    virtual void priv_read();

    virtual fh_context SubCreate_file( fh_context c, fh_context md );
    virtual void createStateLessAttributes( bool force = false );
    
    virtual bool supportsRemove() { return true; }
    virtual void priv_remove( fh_context c_ctx );
    
public:

    shmListContext( Context* parent, std::string rdn );
    ~shmListContext();

    void
    priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
        {
            string xsd = "	<elementType name=\"file\">\n"
                "		<elementType name=\"size\" default=\"4096\" >\n"
                "			<dataTypeRef name=\"int\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"ignore-umask\" default=\"0\">\n"
                "			<dataTypeRef name=\"bool\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"user-readable\" default=\"1\">\n"
                "			<dataTypeRef name=\"bool\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"user-writable\" default=\"1\">\n"
                "			<dataTypeRef name=\"bool\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"group-readable\" default=\"0\">\n"
                "			<dataTypeRef name=\"bool\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"group-writable\" default=\"0\">\n"
                "			<dataTypeRef name=\"bool\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"other-readable\" default=\"0\">\n"
                "			<dataTypeRef name=\"bool\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"other-writable\" default=\"0\">\n"
                "			<dataTypeRef name=\"bool\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"mode\" default=\"-1\">\n"
                "			<dataTypeRef name=\"int\"/>\n"
                "		</elementType>\n"
                "	</elementType>\n";

            m["file"] = SubContextCreator(SL_ipc_SubCreate_file, xsd );
        }
};

fh_context SL_ipc_SubCreate_file( fh_context c, fh_context md )
{
    return c->SubCreate_file( c, md );
}

shmListContext::shmListContext( Context* parent, std::string rdn )
    :
    _Base( parent, rdn )
{
    createStateLessAttributes();
}

shmListContext::~shmListContext()
{
}





void
shmListContext::createStateLessAttributes( bool force )
{
    static Util::SingleShot virgin;
    if( virgin() )
    {
//         LG_IPCCTX_D << "shmListContext::createStateLessAttributes() path:" << getDirPath()
//              << " deepest:" << getDeepestTypeInfo().name()
//              << endl;
        
        _Base::createStateLessAttributes( true );
        supplementStateLessAttributes( true );
    }
}

fh_context
shmListContext::SubCreate_file( fh_context c, fh_context md )
{
    int size        = toint(getStrSubCtx( md, "size", "4096" ));
    string v        = "<new>";
    int mode_flags  = getModeFromMetaData( md );
        
    LG_IPCCTX_D << "shmListContext::SubCreate_file() size:" << size << endl;

    int shmid = shmget( IPC_PRIVATE, size, IPC_CREAT | IPC_EXCL | mode_flags );
    if( shmid < 0 )
    {
        fh_stringstream ss;
        ss << "Attempt to create new shared memory failed. "
           << " size: " << size
           << " url:" << c->getURL()
           << endl;
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
    }

    LG_IPCCTX_D << "shmListContext::SubCreate_file() shmid:" << shmid << endl;
    struct shmid_ds shmseg;
    try
    {
//        shmseg = getSegmentData( shmid );
//        shmContext* ret = new shmContext( this, tostr(shmid), shmseg );
        
        shmContext* ret = new shmContext( this, tostr(shmid), shmid );
        Insert( ret, false );
        return ret;
    }
    catch( FerrisCreateSubContextNotSupported& e )
    {
        fh_stringstream ss;
        ss << "Can not find existing new segment. shmid:" << shmid
           << " size: " << size
           << " shmid:" << shmid
           << " url:" << c->getURL()
           << " e:" << e.what();

        int remrc = shmctl( shmid, IPC_RMID, 0 );
        if( remrc < 0 )
        {
            ss << "Can not remove new shared memory id:" << shmid << " !" << endl;
        }
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
    }
}



void
shmListContext::priv_read()
{
    LG_IPCCTX_D << "shmListContext::priv_read() path:" << getDirPath() << endl;

    staticDirContentsRAII _raii1( this );
    
    if( empty() )
    {
        clearContext();
        LG_IPCCTX_D << "shmListContext::priv_read(1) path:" << getDirPath() << endl;
        LG_IPCCTX_D << "shmListContext::priv_read(1) name:" << getDirName() << endl;

        struct shm_info shm_info;
        struct shminfo shminfo;
        int maxid = shmctl (0, SHM_INFO, (struct shmid_ds *) &shm_info);
        if (maxid < 0)
        {
            fh_stringstream ss;
            ss << "kernel not configured for shared memory";
            Throw_FerrisNotReadableAsContext( tostr(ss), this );
        }

        for (int id = 0; id <= maxid; id++)
        {
            LG_IPCCTX_D << "max:" << maxid << " id:" << id << endl;
            struct shmid_ds shmseg;
            int shmid = shmctl( id, SHM_STAT, &shmseg);
            if (shmid < 0) 
                continue;

            LG_IPCCTX_D << " shmid:" << shmid << endl;
            LG_IPCCTX_D << "max:" << maxid << " id:" << id
                        << " shmid:" << shmid
                        << endl;
            shmContext* c = new shmContext( this, tostr(shmid), shmid, shmseg );
            Insert( c, false );
        }
    }
    
    LG_IPCCTX_D << "shmListContext::priv_read(done) path:" << getDirPath() << endl;
}

void
shmListContext::priv_remove( fh_context c_ctx )
{
    shmContext* c = dynamic_cast<shmContext*>( (GetImpl(c_ctx) ) );
    if( !c )
    {
        fh_stringstream ss;
        ss << "Attempt to remove a non shared memory context! url:" << c_ctx->getURL();
        Throw_CanNotDelete( tostr(ss), GetImpl(c_ctx) );
    }

    int shmid = toint( c->getDirName() );
    int rc    = shmctl( shmid, IPC_RMID, 0 );

    if( rc < 0 )
    {
        int eno=errno;
        fh_stringstream ss;
        ss << "Attempt to remove shared memory context failed url:" << c_ctx->getURL()
           << " err:" << errnum_to_string( "", eno );
        Throw_CanNotDelete( tostr(ss), GetImpl(c_ctx) );
    }
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/




/**
 * pp 461 stevens advanced unix prog.
 *
 * We only handle one value per semaphore at the moment, and we drop the sem count by 1 when
 * an i/iostream is requested and restore it by 1 when the stream buffer goes out of scope.
 */
class FERRISEXP_CTXPLUGIN semContext
    :
    public StateLessEAHolder< semContext, sysvipcContext > 
{
    typedef semContext                                      _Self;
    typedef StateLessEAHolder< semContext, sysvipcContext > _Base;

    static fh_istream SL_getSemSizeStream ( semContext* c, const std::string& rdn, EA_Atom* atom );
    static fh_stringstream SL_getSemNumberFreeStream( semContext* c, const std::string& rdn, EA_Atom* atom );
    static void SL_setSemNumberFreeStream( semContext* c, const std::string& rdn,
                                           EA_Atom* atom, fh_istream ss );
    static fh_istream SL_getSemOTimeStream( semContext* c, const std::string& rdn, EA_Atom* atom );
    static fh_istream SL_getSemCTimeStream( semContext* c, const std::string& rdn, EA_Atom* atom );

    struct semid_ds ds;

protected:

    virtual int ipcset( struct ipc_perm* p )
        {
            struct semid_ds localds = ds;
            localds.sem_perm = *p;
            return semctl( p->__seq, 0, IPC_SET, &localds );
        }

    void DropSemCount( fh_istream& ss, std::streamsize tellp )
        {
            SemOp( 1 );
        }

    void SemOp( int adjustment )
        {
            const int sb_sz = ds.sem_nsems+1;
            struct sembuf* sb = new sembuf[ sb_sz ];
            sb[0].sem_num = 0;
            sb[0].sem_op  = adjustment;
            sb[0].sem_flg = SEM_UNDO;
            int rc = semop( sysv_id, static_cast<struct sembuf*>(sb), 1 );
            delete[] sb;
            
            if( rc == -1 )
            {
                int eno = errno;
                fh_stringstream ss;
                ss << "Can not obtain semaphore for semid:" << sysv_id
                   << " url:" << getURL();
                ThrowFromErrno( eno, tostr(ss), this );
            }
        }
    
    virtual fh_iostream real_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   exception)
            {
                SemOp( -1 );

                fh_stringstream ret;
                ret.getCloseSig().connect( sigc::mem_fun( *this, &_Self::DropSemCount ) ); 
                return ret;
            }
        
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                return real_getIOStream( m );
            }
        
        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                fh_iostream ret = real_getIOStream( m );
                return ret;
            }
        

        
        void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
#define SLEA tryAddStateLessAttribute

                    SLEA( "size",                 SL_getSemSizeStream,       FXD_FILESIZE );
                    SLEA( "number-free",          SL_getSemNumberFreeStream,
                          SL_getSemNumberFreeStream, SL_setSemNumberFreeStream,
                          XSD_BASIC_INT );
                    SLEA( "otime",                SL_getSemOTimeStream, FXD_UNIXEPOCH_T );
                    SLEA( "ctime",                SL_getSemCTimeStream, FXD_UNIXEPOCH_T );

                    SLEA( "is-dir",               SL_getFalse, XSD_BASIC_BOOL );
                    SLEA( "is-file",              SL_getTrue,  XSD_BASIC_BOOL );
                    SLEA( "is-special",           SL_getFalse, XSD_BASIC_BOOL );
                    SLEA( "is-link",              SL_getFalse, XSD_BASIC_BOOL );
                    SLEA( "has-holes",            SL_getFalse, XSD_BASIC_BOOL );

                    SLEA( "dontfollow-is-dir",    SL_getFalse, XSD_BASIC_BOOL );
                    SLEA( "dontfollow-is-file",   SL_getTrue,  XSD_BASIC_BOOL );
                    SLEA( "dontfollow-is-special",SL_getFalse, XSD_BASIC_BOOL );
                    SLEA( "dontfollow-is-link",   SL_getFalse, XSD_BASIC_BOOL );
                    SLEA( "dontfollow-has-holes", SL_getFalse, XSD_BASIC_BOOL );
                    
#undef SLEA
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }

        /**
         * Given a shmid (a big number) find the metadata for that segment.
         */
        struct semid_ds getSegmentData( int required_shmid = -1 )
            {
                if( required_shmid == -1 )
                {
                    required_shmid = sysv_id;
                }


                union semun arg;
                struct seminfo seminfo;
                arg.array = (ushort *)  &seminfo;
                int maxid = semctl (0, 0, SEM_INFO, arg);
                if (maxid < 0)
                {
                    fh_stringstream ss;
                    ss << "kernel not configured for sysv semaphores";
                    Throw_FerrisNotReadableAsContext( tostr(ss), this );
                }

                for (int id = 0; id <= maxid; id++)
                {
                    LG_IPCCTX_D << "max:" << maxid << " id:" << id << endl;

                    struct semid_ds semary;
                    arg.buf = (struct semid_ds *) &semary;
                    int semid = semctl (id, 0, SEM_STAT, arg);

                    if( semid == required_shmid )
                        return semary;
        
                }

                
                fh_stringstream ss;
                ss << "Can not find information for shmid:" << required_shmid << " url:" << getURL();
                Throw_FerrisNotReadableAsContext( tostr(ss), this );
            }
        void updateSegmentData()
            {
                ds = getSegmentData();
            }
        
        virtual struct ipc_perm* getPermissions()
            {
                return &ds.sem_perm;
            }
        
    public:
        
    semContext( Context* parent, std::string rdn, int semid, const struct semid_ds& ds_ )
        :
        _Base( parent, rdn ),
        ds( ds_ )
        {
            setSysVID( semid );
            createStateLessAttributes();
        }

         semContext( Context* parent, std::string rdn, int semid )
             :
             _Base( parent, rdn ),
             ds( getSegmentData( semid ) )
            {
                setSysVID( semid );
                createStateLessAttributes();
            }
        
        virtual ~semContext()
            {}

        string getRecommendedEA()
            {
                return "protection-ls,user-owner-name,group-owner-name,number-free,size,"
                    "otime-display,ctime-display,"
                    "name";
            }

    void setSem( int v )
        {
            union semun arg;
            arg.val = v;
            int rc = semctl( sysv_id, 0, SETVAL, arg );
            if( rc < 0 )
            {
                int eno=errno;
                fh_stringstream ss;
                ss << "Cant set the semaphore value to v:" << v
                   << " url: " << getURL()
                   << " err:" << errnum_to_string( "", eno );
                Throw_getIOStreamCloseUpdateFailed( tostr(ss), this );
            }
        }
    
    };
    


    fh_istream semContext::SL_getSemSizeStream( semContext* c, const std::string& rdn, EA_Atom* atom )
    {
        const struct semid_ds& ds = c->ds;

        fh_stringstream ss;
        ss << ds.sem_nsems;
        return ss;
    }

fh_stringstream semContext::SL_getSemNumberFreeStream( semContext* c, const std::string& rdn, EA_Atom* atom )
{
    const struct semid_ds& ds = c->ds;

    union semun arg;
    int rc = semctl( c->sysv_id, 0, GETVAL, arg );

    if( rc < 0 )
    {
        int eno = errno;
        fh_stringstream ss;
        ss << "Can not read number of free semaphores url:" << c->getURL();
        ThrowFromErrno( eno, tostr(ss), c );
    }
    
    fh_stringstream ss;
    ss << rc;
    return ss;
}

void semContext::SL_setSemNumberFreeStream( semContext* c, const std::string& rdn,
                                EA_Atom* atom, fh_istream ss )
{
    int v = 0;
    
    if( ss >> v )
    {
        c->setSem( v );
    }
    else
    {
        fh_stringstream ss;
        ss << "Must supply new value for raw adjustments on the free-count."
           << " url: " << c->getURL();
        Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
    }
}


    
    fh_istream semContext::SL_getSemOTimeStream( semContext* c, const std::string& rdn, EA_Atom* atom )
    {
        c->updateSegmentData();
        const struct semid_ds& ds = c->ds;

        fh_stringstream ss;
        ss << ds.sem_otime;
        return ss;
    }
    
    fh_istream semContext::SL_getSemCTimeStream( semContext* c, const std::string& rdn, EA_Atom* atom )
    {
        const struct semid_ds& ds = c->ds;

        fh_stringstream ss;
        ss << ds.sem_ctime;
        return ss;
    }


/******************************************************************************/
/******************************************************************************/


    
class FERRISEXP_CTXPLUGIN semListContext
    :
        public StateLessEAHolder< semListContext,
                                  RecommendedEACollectingContext< FakeInternalContext > >
{
    typedef semListContext _Self;
    typedef StateLessEAHolder< semListContext,
                               RecommendedEACollectingContext< FakeInternalContext > > _Base;

    semListContext*
    priv_CreateContext( Context* parent, string rdn )
        {
            semListContext* ret = new semListContext( parent, rdn );
            return ret;
        }
    
protected:

    virtual void priv_read();

    virtual fh_context SubCreate_file( fh_context c, fh_context md );
    virtual void createStateLessAttributes( bool force = false )
        {
            static Util::SingleShot virgin;
            if( virgin() )
            {
                _Base::createStateLessAttributes( true );
                supplementStateLessAttributes( true );
            }
        }
        
    virtual bool supportsRemove() { return true; }
    virtual void priv_remove( fh_context c_ctx );
    
public:

    semListContext( Context* parent, std::string rdn );
    ~semListContext()
        {
        }
    
    void
    priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
        {
            string xsd = "	<elementType name=\"file\">\n"
                "		<elementType name=\"size\" default=\"4096\" >\n"
                "			<dataTypeRef name=\"int\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"number-free\" default=\"1\" >\n"
                "			<dataTypeRef name=\"int\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"ignore-umask\" default=\"0\">\n"
                "			<dataTypeRef name=\"bool\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"user-readable\" default=\"1\">\n"
                "			<dataTypeRef name=\"bool\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"user-writable\" default=\"1\">\n"
                "			<dataTypeRef name=\"bool\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"group-readable\" default=\"0\">\n"
                "			<dataTypeRef name=\"bool\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"group-writable\" default=\"0\">\n"
                "			<dataTypeRef name=\"bool\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"other-readable\" default=\"0\">\n"
                "			<dataTypeRef name=\"bool\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"other-writable\" default=\"0\">\n"
                "			<dataTypeRef name=\"bool\"/>\n"
                "		</elementType>\n"
                "		<elementType name=\"mode\" default=\"-1\">\n"
                "			<dataTypeRef name=\"int\"/>\n"
                "		</elementType>\n"
                "	</elementType>\n";

            m["file"] = SubContextCreator(SL_ipc_sem_SubCreate_file, xsd );
        }
};

fh_context SL_ipc_sem_SubCreate_file( fh_context c, fh_context md )
{
    return c->SubCreate_file( c, md );
}


semListContext::semListContext( Context* parent, std::string rdn )
    :
    _Base( parent, rdn )
{
    createStateLessAttributes();
}

fh_context
semListContext::SubCreate_file( fh_context c, fh_context md )
{
    int size        = toint(getStrSubCtx( md, "size", "1" ));
    int nf          = toint(getStrSubCtx( md, "number-free", "1" ));
    string v        = "<new>";
    int mode_flags  = getModeFromMetaData( md );
        
    LG_IPCCTX_D << "semListContext::SubCreate_file() size:" << size << endl;

    int semid = semget( IPC_PRIVATE, size, IPC_CREAT | IPC_EXCL | mode_flags );
    if( semid < 0 )
    {
        fh_stringstream ss;
        ss << "Attempt to create new shared memory failed. "
           << " size: " << size
           << " url:" << c->getURL()
           << endl;
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
    }

    LG_IPCCTX_D << "semListContext::SubCreate_file() semid:" << semid << endl;
    struct semid_ds semseg;
    try
    {
        semContext* ret = new semContext( this, tostr(semid), semid );
        Insert( ret, false );
        ret->setSem( nf );
        return ret;
    }
    catch( FerrisCreateSubContextNotSupported& e )
    {
        fh_stringstream ss;
        ss << "Can not find existing new segment. semid:" << semid
           << " size: " << size
           << " semid:" << semid
           << " url:" << c->getURL()
           << " e:" << e.what();

        int remrc = semctl( semid, 0, IPC_RMID, 0 );
        if( remrc < 0 )
        {
            ss << "Can not remove new semaphore id:" << semid << " !" << endl;
        }
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
    }
}


void
semListContext::priv_read()
{
    LG_IPCCTX_D << "semListContext::priv_read() path:" << getDirPath() << endl;

    staticDirContentsRAII _raii1( this );
    if( empty() )
    {
        clearContext();
        LG_IPCCTX_D << "semListContext::priv_read(1) path:" << getDirPath() << endl;
        LG_IPCCTX_D << "semListContext::priv_read(1) name:" << getDirName() << endl;

        union semun arg;
        struct seminfo seminfo;
        arg.array = (ushort *)  &seminfo;
        int maxid = semctl (0, 0, SEM_INFO, arg);
        if (maxid < 0)
        {
            fh_stringstream ss;
            ss << "kernel not configured for sysv semaphores";
            Throw_FerrisNotReadableAsContext( tostr(ss), this );
        }
        LG_IPCCTX_D << "maxid:" << maxid << endl;

        for (int id = 0; id <= maxid; id++)
        {
            LG_IPCCTX_D << "max:" << maxid << " id:" << id << endl;

            struct semid_ds semary;
            arg.buf = (struct semid_ds *) &semary;
            int semid = semctl (id, 0, SEM_STAT, arg);
        
            if (semid < 0) 
                continue;

            LG_IPCCTX_D << " semid:" << semid << endl;
            LG_IPCCTX_D << "max:" << maxid << " id:" << id
                        << " semid:" << semid
                        << endl;
            semContext* c = new semContext( this, tostr(semid), semid, semary );
            Insert( c, false );
        }
    }
    
    LG_IPCCTX_D << "semListContext::priv_read(done) path:" << getDirPath()
                << " SubContextCount:" << SubContextCount() << endl;
}

void
semListContext::priv_remove( fh_context c_ctx )
{
    semContext* c = dynamic_cast<semContext*>( (GetImpl(c_ctx) ) );
    if( !c )
    {
        fh_stringstream ss;
        ss << "Attempt to remove a non shared memory context! url:" << c_ctx->getURL();
        Throw_CanNotDelete( tostr(ss), GetImpl(c_ctx) );
    }

    int semid = toint( c->getDirName() );
    int rc    = semctl( semid, 0, IPC_RMID, 0 );

    if( rc < 0 )
    {
        int eno=errno;
        fh_stringstream ss;
        ss << "Attempt to remove shared memory context failed url:" << c_ctx->getURL()
           << " err:" << errnum_to_string( "", eno );
        Throw_CanNotDelete( tostr(ss), GetImpl(c_ctx) );
    }
}



/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


extern "C"
{
    fh_context Brew( RootContextFactory* rf )
        throw( RootContextCreationFailed )
        try
    {
        static FakeInternalContext* c = 0;
        const string& root = rf->getInfo( RootContextFactory::ROOT );

        if( !c )
        {
            LG_IPCCTX_D << "Making FakeInternalContext(1) " << endl;
            c = new FakeInternalContext(0, "/");
            
            // Bump ref count.
            static fh_context keeper = c;
            static fh_context keeper2 = keeper;

            c->addNewChild( new shmListContext( c, "shared-memory" ) );
            c->addNewChild( new semListContext( c, "semaphores" ) );

            LG_IPCCTX_D << "Making FakeInternalContext(2) " << endl;
        }

        LG_IPCCTX_D << "Making FakeInternalContext(3) brewing return" << endl;

        fh_context ret = c;

        if( root != "/" )
        {
            string path = rf->getInfo( RootContextFactory::PATH );
            string newpath = root;

            newpath += "/";
            newpath += path;

            rf->AddInfo( RootContextFactory::PATH, newpath );
        }
        return ret;
    }
    catch( exception& e )
    {
        fh_stringstream ss;
        ss << "ipc.brew() e:" << e.what() << endl;
        Throw_RootContextCreationFailed( tostr(ss), 0 );
    }
    
}

};
