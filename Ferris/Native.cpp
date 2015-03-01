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

    $Id: Native.cpp,v 1.42 2010/11/17 21:30:46 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <config.h>

#include <fstream>
#include <locale>

#include <Native.hh>
#include <General.hh>
#include <Ferris_private.hh>
#include <xfsutil.hh>

#include <sigc++/sigc++.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <utime.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>

#include <glib.h>

#include "Ferris/FerrisCreationPlugin.hh"
#include "Ferris/FerrisBoost.hh"

#ifdef FERRIS_HAVE_LIBCAP
#include <sys/capability.h>
#endif

#include <StatfsUtilities.hh>
#include <SchemaSupport.hh>


#ifdef FERRIS_HAVE_XFS
#include <libxfs.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <ctype.h>
#endif

#ifdef HAVE_LIBSELINUX
extern "C" {
#include <selinux/context.h>
#include <selinux/selinux.h>
};
#endif

#include <errno.h>
#include <stdio.h>

// required for NativeVFS_RootContextDropper
#include <Resolver_private.hh>

#include "FerrisKDE.hh"

#include "FSParser_private.hh"

#ifdef PLATFORM_OSX
  #ifndef MAJOR
    #define MAJOR(x) x
  #endif
  #ifndef MINOR
    #define MINOR(x) x
  #endif
#else
  #include <linux/kdev_t.h>
#endif

// #undef LG_NATIVE_D
// #define LG_NATIVE_D cerr

#define CERR cerr

using namespace std;

#include <FerrisQt_private.hh>
#include "DBusGlue/com_libferris_Volume_Manager.h"
#include "DBusGlue/com_libferris_Volume_Manager.cpp"

#ifndef PLATFORM_OSX
  #define PERMIT_FAM
#endif

namespace Ferris
{
    template <class charT, class Traits>
    basic_ostream<charT, Traits>&
    operator<< (basic_ostream<charT, Traits>& os, const fsid_t& s )
    {
        guint64 p = 0, sec = 0;

#ifdef PLATFORM_OSX        
        p   = s.val[0];
        sec = s.val[1];
#else
        p   = s.__val[0];
        sec = s.__val[1];
#endif
        
        sec <<= 32;
        p |= sec;

        os << p;
//        os << s.__val[0] << " " << s.__val[1];
        return os;
    }

    

    struct FERRISEXP_DLLLOCAL FollowLinks
    {
        static const struct stat& getStat( NativeContext* c )
            {
                const struct stat& ret = c->getStat_Follow();
                if( c->IsDanglingLink )
                {
                    stringstream ss;
                    ss << "Attempt tp read dangling link at path:" << c->getURL();
                    Throw_CanNotDereferenceDanglingSoftLink( tostr(ss), c );
                }
                return ret;
            }
        static int chown( const char * p, uid_t u, gid_t g )
            {
                return ::chown( p, u, g );
            }
#ifdef HAVE_LIBSELINUX
        static int Xgetfilecon( const std::string& path, security_context_t& con )
            {
                return getfilecon( path.c_str(), &con );
            }
        static int Xsetfilecon( const std::string& path, security_context_t& con )
            {
                return setfilecon( path.c_str(), con );
            }
#endif
    };

    struct FERRISEXP_DLLLOCAL DontFollowLinks
    {
        static const struct stat& getStat( NativeContext* c )
            {
                return c->getStat_DontFollow();
            }
        static int chown( const char * p, uid_t u, gid_t g )
            {
                return ::lchown( p, u, g );
            }
#ifdef HAVE_LIBSELINUX
        static int Xgetfilecon( const std::string& path, security_context_t& con )
            {
                return lgetfilecon( path.c_str(), &con );
            }
        static int Xsetfilecon( const std::string& path, security_context_t& con )
            {
                return lsetfilecon( path.c_str(), con );
            }
#endif
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

#ifdef HAVE_LIBSELINUX

    template< class FileConFactory >
    class SELinuxFileContext
    {
        security_context_t con;
        string identity;
        string role;
        string ftype;

    public:
        SELinuxFileContext( Context* c )
            :
            con( 0 ),
            identity( "" ),
            ftype( "" )
            {
                string path = c->getDirPath();
                
                int sz = FileConFactory::Xgetfilecon( path, con );
                if( sz <= 0 )
                {
                    int ec = errno;
                    fh_stringstream ss;
                    ss << "Can't get SELinux context. " << " file: " << path;
//                    cerr << "error:" << tostr(ss) << endl;
                    ThrowFromErrno( ec, tostr(ss), c );
                }

//                cerr << "con:" << con << endl;
                stringstream ss;
                ss << con;
                getline( ss, identity, ':' );
                getline( ss, role, ':' );
                getline( ss, ftype );
            }
        ~SELinuxFileContext()
            {
                if( con )
                    freecon( con );
            }

        const string& getIdentity()
            {
                return identity;
            }
        const string& getType()
            {
                return ftype;
            }
        const string& getRole()
            {
                return role;
            }
        const char* getContextString()
            {
                return con;
            }
    };

    template < class DataFactory >
    fh_stringstream
    SL_getSELinuxIdentity( NativeContext* c, const std::string& rdn, EA_Atom* atom )
    {
        SELinuxFileContext< DataFactory > sec( c );
        fh_stringstream ss;
        ss << sec.getIdentity();
        return ss;
    }

    template < class DataFactory >
    fh_stringstream
    SL_getSELinuxType( NativeContext* c, const std::string& rdn, EA_Atom* atom )
    {
        SELinuxFileContext< DataFactory > sec( c );
        fh_stringstream ss;
        ss << sec.getType();
        return ss;
    }

    template <class DataFactory>
    void
    SL_setSELinuxType( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
    {
        string v;
        
        if( ss >> v )
        {
            SELinuxFileContext< DataFactory > sec( c );
            stringstream conss;
            conss << sec.getIdentity() << ":" << sec.getRole() << ":" << v;
            string tmp = conss.str();
            LG_NATIVE_D << "con:" << conss.str() 
                        << " file: " << c->getDirPath()
                        << endl;
            security_context_t con = (char*)tmp.c_str();
            int rc = DataFactory::Xsetfilecon( c->getDirPath(), con );
            if( rc != 0 )
            {
                int ec = errno;
                fh_stringstream ss;
                ss << "Native context can not set the SELinux security context:" << con
                   << " file: " << c->getDirPath();
                LG_NATIVE_D << tostr(ss)
                            << " reason:" << errnum_to_string( "", ec )
                            << endl;
                ThrowFromErrno( ec, tostr(ss), c );
            }
            c->bumpVersion();
            return;
        }

        /* fail */
        {
            fh_stringstream ss;
            ss << "Native context can not set the SELinux security context:" << v
               << " file: " << c->getDirPath();
            Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
        }
    }
    
    template < class DataFactory >
    fh_stringstream
    SL_getSELinuxContext( NativeContext* c, const std::string& rdn, EA_Atom* atom )
    {
        SELinuxFileContext< DataFactory > sec( c );
        fh_stringstream ss;
        ss << sec.getContextString();
        return ss;
    }

    template <class DataFactory>
    void
    SL_setSELinuxContext( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
    {
//        SELinuxFileContext< DataFactory > sec( c );
        string v;
        
        if( ss >> v )
        {
            security_context_t con = (char*)v.c_str();
            int rc = DataFactory::Xsetfilecon( c->getDirPath(), con );
            if( rc != 0 )
            {
                int ec = errno;
                fh_stringstream ss;
                ss << "Native context can not set the SELinux security context:" << v
                   << " file: " << c->getDirPath();
                ThrowFromErrno( ec, tostr(ss), c );
            }
            c->bumpVersion();
            return;
        }

        /* fail */
        {
            fh_stringstream ss;
            ss << "Native context can not set the SELinux security context:" << v
               << " file: " << c->getDirPath();
            Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
        }
    }
    
#endif

    template < class DataFactory >
    fh_stringstream
    SL_getEpisode( NativeContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;

        
        FSParser p;
        bool rc = p.parse( c->getDirName() );
        if( rc )
        {
            ss << p.getE();
        }
        else
        {
            ss << "";
        }
        return ss;
    }

    template < class DataFactory >
    fh_stringstream
    SL_getSeries( NativeContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;

        FSParser p;
        bool rc = p.parse( c->getDirName() );
//        cerr << "SL_getSeries() ret:" << p.getName() << " rc:" << rc << " file:" << c->getDirName() << endl;
        if( rc )
        {
            ss << p.getName();
        }
        else
        {
            ss << "";
        }
        return ss;
    }
    

    template < class DataFactory >
    fh_stringstream
    SL_getExpectedCRC32( NativeContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        string n = c->getDirName();
        boost::regex rex("(.*\\[)([A-F0-9][A-F0-9][A-F0-9][A-F0-9][A-F0-9][A-F0-9][A-F0-9][A-F0-9])(\\].*)");
        boost::smatch matches;
        if(boost::regex_match( n, matches, rex ))
        {
            if( matches.size() == 4 )
            {
                ss << tolowerstr()( matches[2] );
            }
        }
        
//         FSParser p;
//         bool rc = p.parse( c->getDirName() );
//         if( rc )
//         {
//             ss << p.getChecksum();
//         }
//         else
//         {
//             ss << "";
//         }
        return ss;
    }

    template < class DataFactory >
    fh_stringstream
    SL_getCRC32IsValid( NativeContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;

        string expected = getStrAttr( c, "crc32-expected", "" );
        if( expected.empty() )
            ss << "1";
        else
            ss << ( getStrAttr( c, "crc32", "1" ) == expected );
        return ss;
    }
    

    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    

    namespace System
    {

    
/**
 * Check of the process has one specific capability
 */
#ifdef FERRIS_HAVE_LIBCAP
        bool
        HaveCap( cap_value_t v )
        {
            {
                cap_t caps = cap_get_proc();
                cap_flag_t flag = CAP_PERMITTED;
                cap_flag_value_t value;
                int rc = cap_get_flag( caps, v, flag, &value );
                if( rc == 0 )
                {
                    return( value == CAP_SET );
                }
            }
            return getuid() == 0;
        }
#endif

        bool gotRoot()
        {
            return getuid() == 0;
        }
    };

    namespace Shell
    {
        
        /*
         * like canChangeFileToGroup() but operates on uid.
         */
        bool canChangeFileToUser( uid_t u )
        {
            if( getuid() == 0 )
            {
                return true;
            }

            if( struct passwd* pass = getpwuid( getuid() ) )
            {
                if( u == pass->pw_uid )
                {
                    return true;
                }
            }

#ifdef FERRIS_HAVE_LIBCAP
            return ::Ferris::System::HaveCap( CAP_CHOWN );
#else
            return ::Ferris::System::gotRoot();
#endif
        }



/*
 * Check if the running process can chance a file to have group id "g".
 * Checks are done on the uid, the groups the user is in, and if the
 * user has that capability
 */
        bool canChangeFileToGroup( gid_t g )
        {
            LG_NATIVE_D << "canChangeFileToGroup() g:" << g << endl;
            
            if( getuid() == 0 )
            {
                return true;
            }

            const int grsz = NGROUPS_MAX;
            gid_t grlist[ NGROUPS_MAX ];
    
            if( struct passwd* pass = getpwuid( getuid() ) )
            {
                LG_NATIVE_D << "canChangeFileToGroup() g:" << g
                            << " pass->pw_gid:" << pass->pw_gid
                            << endl;
                if( g == pass->pw_gid )
                {
                    return true;
                }
            }

            int grmax = getgroups( grsz, grlist );
            if( -1 != grmax )
            {
                for( int i=0; i < grmax; ++i )
                {
                    LG_NATIVE_D << "canChangeFileToGroup() g:" << g
                                << " grlist[i]:" << grlist[i] 
                                << endl;
                    if( grlist[i] == g )
                    {
                        return true;
                    }
                }
            }
    
#ifdef FERRIS_HAVE_LIBCAP
            return ::Ferris::System::HaveCap( CAP_CHOWN );
#else
            return ::Ferris::System::gotRoot();
#endif
        }
    };
    
    
    

    const string NativeContext::CreateObjectType_k      = "CreateObjectType";
    const string NativeContext::CreateObjectType_v_Dir  = "Dir";
    const string NativeContext::CreateObjectType_v_File = "File";


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    Context*
    NativeContext::priv_CreateContext( Context* parent, string rdn )
    {
        LG_NATIVE_D << "NativeContext::CreateContext() rdn: " << rdn << endl;
        NativeContext* ret = new NativeContext();
        ret->setContext( parent, rdn );
        return ret;
    }

    //
    // Short cut loading each dir unless absolutely needed.
    //
    fh_context
    NativeContext::priv_getSubContext( const string& rdn )
        throw( NoSuchSubContext )
    {
        try
        {
            LG_NATIVE_D << "NativeContext::priv_getSubContext() p:" << getDirPath()
                        << " rdn:" << rdn
                        << endl;

            Items_t::iterator isSubContextBoundCache;
            if( priv_isSubContextBound( rdn, isSubContextBoundCache ) )
            {
                LG_NATIVE_D << "NativeContext::priv_getSubContext(bound already) p:" << getDirPath()
                            << " rdn:" << rdn
                            << endl;
//                return _Base::priv_getSubContext( rdn );
                return *isSubContextBoundCache;
            }

            if( rdn.empty() )
            {
                fh_stringstream ss;
                ss << "NoSuchSubContext no rdn given";
                Throw_NoSuchSubContext( tostr(ss), this );
            }
            else if( rdn[0] == '/' )
            {
                fh_stringstream ss;
                ss << "NoSuchSubContext no files start with unescaped '/' as filename";
                Throw_NoSuchSubContext( tostr(ss), this );
            }
            

            
            struct stat tsb;
            string fqfn = appendToPath( getDirPath(), rdn );
            int rv = lstat( fqfn.c_str(), &tsb);

            LG_NATIVE_D << "NativeContext::priv_getSubContext() p:" << getDirPath()
                        << " fqfn:" << fqfn
                        << " rv:" << rv
                        << endl;
            
            if( rv != 0 )
            {
                // stat failed
                fh_stringstream ss;
                ss << "NoSuchSubContext:" << rdn;
                Throw_NoSuchSubContext( tostr(ss), this );
            }

            
            
            bool created = false;
//             cerr << "NativeContext::priv_getSubContext() path:" << getDirPath()
//                  << " rdn:" << rdn
//                  << " using priv_readSubContext()" << endl;
            
            {
                fh_context ret = native_readSubContext( rdn, created, false );

//                 cerr << "NativeContext::priv_getSubContext url:" << getURL()
//                      << " child.sz:" << getItems().size() << endl;
//                 DEBUG_dumpcl( "priv_getSubContext shortcut reading" );
                
//                 cerr << "NativeContext::priv_getSubContext(2) this:" << toVoid(this) << endl;
//                 cerr << "NativeContext::priv_getSubContext(3) path:" << getDirPath()
//                      << " rdn:" << rdn
//                      << endl;
                return ret;
            }
            
//             LG_NATIVE_D << "NativeContext::priv_getSubContext(2) p:" << getDirPath() << endl;
//             Context* c = priv_CreateContext( this, rdn );
//             LG_NATIVE_D << "NativeContext::priv_getSubContext(3) p:" << getDirPath() << endl;
//             const fh_context ret = Insert( c, created );

//             LG_NATIVE_D << "NativeContext::priv_getSubContext(4) p:" << getDirPath() << endl;
//             bumpVersion();
//             LG_NATIVE_D << "NativeContext::priv_getSubContext(5) p:" << getDirPath()
//                         << " ret:" << ret->getURL()
//                         << endl;
//             return ret;
        }
        catch( NoSuchSubContext& e )
        {
            throw e;
        }
        catch( exception& e )
        {
            string s = e.what();
//            cerr << "NativeContext::priv_getSubContext() e:" << e.what() << endl;
            Throw_NoSuchSubContext( s, this );
        }
        catch(...)
        {}
        fh_stringstream ss;
        ss << "NoSuchSubContext:" << rdn;
        Throw_NoSuchSubContext( tostr(ss), this );
    }

    
    fh_context
    NativeContext::native_readSubContext( const string& rdn, bool created, bool checkIfExistsAlready )
        throw( NoSuchSubContext, FerrisNotSupportedInThisContext )
    {
        LG_NATIVE_D << "native_readSubContext() rdn:" << rdn
                    << " created:" << created
                    << " checkIfExists:" << checkIfExistsAlready
                    << " this:" << toVoid(dynamic_cast<Context*>(this))
                    << " isnative:" << toVoid(dynamic_cast<NativeContext*>(this))
                    << " cc:"   << toVoid(dynamic_cast<Context*>(GetImpl(CoveredContext)))
                    << " omc:"  << toVoid(dynamic_cast<Context*>(getOverMountContext()))
                    << endl;
        
        Version_t v = getVersion();
        fh_context ret = priv_readSubContext( rdn, created, checkIfExistsAlready );

        if( v != getVersion() )
        {
            // This will gradually build up outgoing requests for new monitors
            // when this method itself is called in response to an incoming event.
            // As such you can get a situation where the server's write queue is full
            // and the server blocks waiting for us.
            // in 1.1.81 moved this to a post read() block
//             try
//             {
// #ifdef PERMIT_FAM
//                 if( !FamppChangedEventConnected )
//                 {
//                     if( !FamReq )
//                     {
//                         FamReq = Fampp::MonitorDirectory( getDirPath() );
//                     }
                    
//                     typedef NativeContext NC;
//                     const FamReq_t& R = FamReq;
//                     R->getSig<Fampp::FamppChangedEvent>().connect(slot( *this, &NC::OnFamppChangedEvent));
//                     FamppChangedEventConnected = true;
//                 }
// #endif
//             }
//             catch( Fampp::FamppDirMonitorInitFailedException& e)
//             {
//                 ostringstream ss;
//                 ss << "FamppDirMonitorInitFailedException for path:" << getDirPath();
//                 cerr << tostr(ss) << endl;
//                 Throw_CanNotMonitorDirWithFAM( tostr(ss), this, e );
//             }
        }

        {
            string dotferrisPath = Shell::getHomeDirPath_nochecks() + "/.ferris";
//             CERR << "isDir():" << ret->isDir()
//                  << " is-native:" << ret->getIsNativeContext()
//                  << " rdn:" << rdn
//                  << " path:" << ret->getDirPath() << endl;
            string path = ret->getDirPath();
        
            if( !ret->isDir() &&
                path != "static" && !path.empty()
                && !starts_with( path, dotferrisPath ) )
            {
                ret->RDFCacheAttributes_priv_createAttributes();
            }
        }
        
        return ret;
    }
    


    class FERRISEXP_DLLLOCAL NativeVFS_RootContextDropper
        :
        public RootContextDropper
    {
    public:
        NativeVFS_RootContextDropper()
            {
                RootContextFactory::Register("file", this);
            }

        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
            {
                LG_NATIVE_D << "RootContextDropper::Brew() top" << endl;
                static NativeContext* nc = 0;
                string rootName;
                
                try 
                {
                    if( !nc )
                    {
                        nc = new NativeContext();
                        LG_NATIVE_D << "RootContextDropper::Brew() 2" << endl;
                        nc->setContext( 0, "static" );
                        LG_NATIVE_D << "RootContextDropper::Brew() 3" << endl;
                        nc->AddRef();
//                    cerr << "base non-freeable nativecontext:" << (void*)dynamic_cast<Context*>(nc) << endl;
                    }
                    LG_NATIVE_D << "RootContextDropper::Brew() 1" << endl;
                
//                  static NativeContext nc;
//                  nc.setContext( 0, "static" );
//                  nc.AddRef();
//                  nc.AddRef();
                    rootName = rf->getInfo( "Root" );
                }
                catch( exception& e )
                {
                    LG_NATIVE_D << "RootContextDropper::Brew() e0:" << e.what() << endl;
                    stringstream ss;
                    ss << "Failed to make file:// handler:" << e.what();
                    cerr << tostr(ss) << endl;
                    Throw_RootContextCreationFailed( tostr(ss), 0 );
                }
                
                LG_NATIVE_D << "RootContextDropper::Brew() 2" << endl;
                
                try
                {
                    LG_NATIVE_D << "RootContextDropper::Brew() 3" << endl;
#ifdef PERMIT_FAM
                    Fampp::RegisterFamppWithGLib();
#endif
                    LG_NATIVE_D << "RootContextDropper::Brew() 4" << endl;
                }
                catch( Fampp::FamppOpenFailedException& e )
                {
                    LG_NATIVE_D << "RootContextDropper::Brew() e1:" << e.what() << endl;
                    stringstream ss;
                    ss << "Can not open connection to FAM. e:" << e.what();
                    cerr << tostr(ss) << endl;
                    Throw_RootContextCreationFailed( tostr(ss), 0 );
                }
                catch( exception& e )
                {
                    LG_NATIVE_D << "RootContextDropper::Brew() e2:" << e.what() << endl;
                    stringstream ss;
                    ss << "Cought an unknown exception setting up FAM. " << e.what();
                    cerr << tostr(ss) << endl;
                    Throw_RootContextCreationFailed( tostr(ss), 0 );
                }
                try
                {
                    LG_NATIVE_D << "RootContextDropper::Brew() 6" << endl;
                    return nc->CreateContext( 0, rootName );
//                    return nc.CreateContext( 0, rootName );
                }
                catch( exception& e )
                {
                    LG_NATIVE_D << "RootContextDropper::Brew() e3:" << e.what() << endl;
                    Throw_RootContextCreationFailed( e.what(), 0 );
                }
            }

        virtual bool isTopLevel()
            {
                return true;
            }
    };

    static NativeVFS_RootContextDropper ___NativeVFS_static_init;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////




NativeContext::NativeContext()
    :
#ifdef HAVE_LIBFILE
    libfile_mime_Version(0),
    libfile_type_Version(0),
//     libfile_mime_client(0),
//     libfile_type_client(0),
#endif
#ifdef HAVE_EFSD
//     m_efsdConnection( 0 ),
//     m_efsdGML( 0 ),
//     m_efsdChannel( 0 ),
#endif
    FamReq(0),
    sb_follow_version(0),
    sb_dont_follow_version(0),
    DontHaveRPMEntry( false ),
    IsDanglingLink( false ),
    FamppChangedEventConnected( false )
{
    setIsNativeContext();
    createStateLessAttributes();
}


NativeContext::~NativeContext()
{
//     cerr << "~NativeContext() path:" << getDirPath() << endl;
//     cerr << "~NativeContext() name:" << getDirName() << endl;

#ifdef HAVE_EFSD
    if( m_efsdChannel )
        g_io_channel_unref(  m_efsdChannel );
#endif

    try
    {
//        cerr << "~NativeContext() " << endl;
//        LG_NATIVE_D << "~NativeContext() path:" << getDirPath() << endl;
//        LG_NATIVE_D << "~NativeContext() name:" << getDirName() << endl;
//         libfile_free( libfile_mime_client );
//         libfile_free( libfile_type_client );
    }
    catch( exception& e )
    {
        LG_NATIVE_ER << "~NativeContext() e:" << e.what() << endl;
    }
    catch( ... )
    {
        LG_NATIVE_ER << "~NativeContext() e: ..." << endl;
    }
}

fh_context
NativeContext::SubCreate_fifo( fh_context c, fh_context md )
{
    string rdn    = getStrSubCtx( md, "name", "" );

    LG_NATIVE_D << "SubCreate_fifo rdn:" << rdn
                << " url:" << getURL()
                << endl;
    
    if( !rdn.length() )
    {
        fh_stringstream ss;
        ss << "SL_SubCreate_fifo() can not find the new rdn for creation operation."
           << "c:" << c->getURL()
           << endl;
        
        Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
    }

    const string filename = rdn;
    const string fqfn = appendToPath(getDirPath(),filename);
    mode_t mode = getModeFromMetaData( md );
    mode_t oldumask = 0;
    bool ignoreUMask = toint(getStrSubCtx( md, "ignore-umask", "0" ));

    if( ignoreUMask ) oldumask = umask( 0 );
    int rc = mkfifo( fqfn.c_str(), mode );
    if( ignoreUMask ) umask( oldumask );
    
    if( rc != 0 )
    {
        string es = errnum_to_string("", errno);
        fh_stringstream ss;
        ss << "SL_SubCreate_fifo() can not make new dir"
           << " fully qualified name:" << fqfn
           << " reason:" << es
           << endl;
        Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
    }

     return native_readSubContext( filename, true );
}

fh_context
NativeContext::SubCreate_special( fh_context c, fh_context md )
{
    string rdn    = getStrSubCtx( md, "name", "" );
    dev_t devtype = toType<dev_t>(getStrSubCtx( md, "device", "" ));
    LG_NATIVE_D << "SubCreate_special rdn:" << rdn
                << " url:" << getURL()
                << endl;
    
    if( !rdn.length() )
    {
        fh_stringstream ss;
        ss << "SL_SubCreate_special() can not find the new rdn for creation operation."
           << "c:" << c->getURL()
           << endl;
        
        Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
    }

    const string filename = rdn;
    const string fqfn = appendToPath(getDirPath(),filename);
    mode_t mode = getModeFromMetaData( md );
    mode_t oldumask = 0;
    bool ignoreUMask = toint(getStrSubCtx( md, "ignore-umask", "0" ));

    if( ignoreUMask ) oldumask = umask( 0 );
    int rc = mknod( fqfn.c_str(), mode, devtype );
    if( ignoreUMask ) umask( oldumask );
    
    if( rc != 0 )
    {
        string es = errnum_to_string("", errno);
        fh_stringstream ss;
        ss << "SL_SubCreate_special() can not make new dir"
           << " fully qualified name:" << fqfn
           << " reason:" << es
           << endl;
        Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
    }

     return native_readSubContext( filename, true );
}

void
NativeContext::ProcessAllFamppEvents()
{
    Main::processAllPending_VFSFD_Events();
}

fh_context
NativeContext::SubCreate_softlink( fh_context c, fh_context md )
{
    string rdn    = getStrSubCtx( md, "name", "" );
    string target = getStrSubCtx( md, "link-target", "" );

    LG_NATIVE_D << "SubCreate_softlink rdn:" << rdn
                << " target:" << target
                << " url:" << getURL()
                << endl;
    
    if( !rdn.length() )
    {
        fh_stringstream ss;
        ss << "SL_SubCreate_softlink() can not find the new rdn for creation operation."
           << " c:" << c->getURL()
           << " rdn:" << rdn
           << " target:" << target
           << endl;
        
        Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
    }

    if( !target.length() )
    {
        fh_stringstream ss;
        ss << "SL_SubCreate_softlink() can not find the target for creation operation."
           << " c:" << c->getURL()
           << " rdn:" << rdn
           << " target:" << target
           << endl;
        
        Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
    }
    
    const string filename = rdn;
    const string fqfn = appendToPath(getDirPath(),filename);

    int rc = symlink( target.c_str(), fqfn.c_str() );
    if( rc != 0 )
    {
        string es = errnum_to_string("", errno);
        fh_stringstream ss;
        ss << "SL_SubCreate_softlink() can not make new dir"
           << " fully qualified name:" << fqfn
           << " reason:" << es
           << endl;
        Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
    }

     return native_readSubContext( filename, true );
}

fh_context
NativeContext::SubCreate_hardlink( fh_context c, fh_context md )
{
    string rdn    = getStrSubCtx( md, "name", "" );
    string target = getStrSubCtx( md, "link-target", "" );

    LG_NATIVE_D << "SubCreate_hardlink rdn:" << rdn
                << " target:" << target
                << " url:" << getURL()
                << endl;
    
    if( !rdn.length() )
    {
        fh_stringstream ss;
        ss << "SL_SubCreate_hardlink() can not find the new rdn for creation operation."
           << "c:" << c->getURL()
           << endl;
        
        Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
    }

    if( !target.length() )
    {
        fh_stringstream ss;
        ss << "SL_SubCreate_hardlink() can not find the target for creation operation."
           << "c:" << c->getURL()
           << endl;
        
        Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
    }
    
    const string filename = rdn;
    const string fqfn = appendToPath(getDirPath(),filename);

    int rc = link( target.c_str(), fqfn.c_str() );
    if( rc != 0 )
    {
        string es = errnum_to_string("", errno);
        fh_stringstream ss;
        ss << "SL_SubCreate_hardlink() can not make new dir"
           << " fully qualified name:" << fqfn
           << " reason:" << es
           << endl;
        Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
    }

     return native_readSubContext( filename, true );
}


fh_context
NativeContext::SubCreate_dir ( fh_context c, fh_context md )
{
    string rdn = getStrSubCtx( md, "name", "" );

    LG_NATIVE_D << "SubCreate_dir rdn:" << rdn
                << " url:" << getURL()
                << endl;
    
    
    if( !rdn.length() )
    {
        fh_stringstream ss;
        ss << "SL_SubCreate_dir() no rdn supplied for creation operation."
           << " c:" << c->getURL()
           << endl;
        
        Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
    }

    const string filename = rdn;
    const string fqfn = appendToPath( getDirPath(), filename );
    mode_t mode = getModeFromMetaData( md );
    mode_t oldumask = 0;
    bool ignoreUMask = toint(getStrSubCtx( md, "ignore-umask", "0" ));

    LG_NATIVE_D << "SubCreate_dir rdn:" << rdn
                << " url:" << getURL()
                << " ignoreUMask:" << ignoreUMask
                << " mode:" << mode
                << " 0x1FF:" << ((int)0x1FF)
                << endl;

//     cerr << "SubCreate_dir rdn:" << rdn
//                 << " url:" << getURL()
//                 << " ignoreUMask:" << ignoreUMask
//                 << " mode:" << mode
//                 << " 0x1FF:" << ((int)0x1FF)
//                 << endl;
    

    if( ignoreUMask ) oldumask = umask( 0 );
    int rc = mkdir( fqfn.c_str(), mode ); //0x1FF );
    if( ignoreUMask ) umask( oldumask );

    if( rc != 0 )
    {
        string es = errnum_to_string("", errno);
        fh_stringstream ss;
        ss << "SL_SubCreate_dir() can not make new dir"
           << " fully qualified name:" << fqfn
           << " reason:" << es
           << endl;
        Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
    }

     return native_readSubContext( filename, true );
}

fh_context SL_SubCreate_fifo( fh_context c, fh_context md )
{
    if( NativeContext* nc = dynamic_cast<NativeContext*>(GetImpl(c)))
    {
        return nc->SubCreate_fifo( c, md );
    }
    fh_stringstream ss;
    ss << "SL_SubCreate_fifo() wrong type of context passed in!"
       << " URL:" << c->getURL()
       << endl;
    Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
}

fh_context SL_SubCreate_special( fh_context c, fh_context md )
{
    if( NativeContext* nc = dynamic_cast<NativeContext*>(GetImpl(c)))
    {
        return nc->SubCreate_special( c, md );
    }
    fh_stringstream ss;
    ss << "SL_SubCreate_special() wrong type of context passed in!"
       << " URL:" << c->getURL()
       << endl;
    Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
}

fh_context SL_SubCreate_softlink( fh_context c, fh_context md )
{
    if( NativeContext* nc = dynamic_cast<NativeContext*>(GetImpl(c)))
    {
        return nc->SubCreate_softlink( c, md );
    }
    fh_stringstream ss;
    ss << "SL_SubCreate_softlink() wrong type of context passed in!"
       << " URL:" << c->getURL()
       << endl;
    Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
}

fh_context SL_SubCreate_hardlink( fh_context c, fh_context md )
{
    if( NativeContext* nc = dynamic_cast<NativeContext*>(GetImpl(c)))
    {
        return nc->SubCreate_hardlink( c, md );
    }
    fh_stringstream ss;
    ss << "SL_SubCreate_hardlink() wrong type of context passed in!"
       << " URL:" << c->getURL()
       << endl;
    Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
}

// fh_context SL_SubCreate_dir( fh_context c, fh_context md )
// {
//     if( NativeContext* nc = dynamic_cast<NativeContext*>(GetImpl(c)))
//     {
//         return nc->SubCreate_dir( c, md );
//     }
//     fh_stringstream ss;
//     ss << "SL_SubCreate_dir() wrong type of context passed in!"
//        << " URL:" << c->getURL()
//        << endl;
//     Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
// }



fh_context
NativeContext::SubCreate_file( fh_context c, fh_context md )
{
    LG_NATIVE_D << "NativeContext::SubCreate_file() c:" << c->getURL() << endl;
    string rdn = getStrSubCtx( md, "name", "" );
//    guint64 preallocate = toType<guint64>( getStrSubCtx( md, "preallocate", "0" ) );
    guint64 preallocate = Util::convertByteString( getStrSubCtx( md, "preallocate", "0" ) );

    if( !rdn.length() )
    {
        fh_stringstream ss;
        ss << "Attempt to create file with no name" << endl;
        Throw_FerrisCreateSubContextFailed( tostr(ss), this );
    }
    
    const string filename = rdn;
    const string fqfn = appendToPath(getDirPath(),filename);

    LG_NATIVE_D << "NativeContext::createSubContext() rdn:" << rdn << endl;
    LG_NATIVE_D << "NativeContext::createSubContext() filename:" << filename << endl;
    LG_NATIVE_D << "NativeContext::createSubContext() fqfn:" << fqfn << endl;

    int oflags  = O_CREAT|O_EXCL|O_WRONLY;
//    mode_t mode = S_IRUSR | S_IWUSR;
    LG_NATIVE_D << "NativeContext::createSubContext(2)" << endl;
    mode_t mode = getModeFromMetaData( md );
    mode_t oldumask = 0;
    LG_NATIVE_D << "NativeContext::createSubContext(3)" << endl;
    bool ignoreUMask = toint(getStrSubCtx( md, "ignore-umask", "0" ));

    LG_NATIVE_D << "Native::create file() fqfn:" << fqfn << " mode:" << mode << endl;
    
    
    if( ignoreUMask ) oldumask = umask( 0 );
    int fd = open( fqfn.c_str(), oflags, mode );
    if( ignoreUMask ) umask( oldumask );

    
    if( -1 == fd )
    {
        string es = errnum_to_string("", errno);
        fh_stringstream ss;
        ss << "SL_SubCreate_file() can not make new file"
           << " fully qualified name:" << fqfn
           << " reason:" << es
           << endl;
        LG_NATIVE_D << tostr(ss) << endl;
        Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
    }

    
#ifdef FERRIS_HAVE_XFS
    if( preallocate )
    {
        xfs_flock64_t flck;

        flck.l_whence = SEEK_SET;
        flck.l_start  = 0LL;
        flck.l_len    = preallocate;

        ioctl(fd, XFS_IOC_RESVSP64, &flck);
        LG_NATIVE_D << "path:" << getDirPath() << " preallocate:" << preallocate << endl;
    }
#endif

    close(fd);
    return native_readSubContext( filename, true );
}

void
NativeContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
{
    addStandardFileSubContextSchema(m);
    addEAGeneratorCreateSubContextSchema(m);

    /*
     * We support some more funky stuff for XFS partitions.
     */
    string XFSFileArgs = "";
    
    if( isXFS( this ) )
    {
        XFSFileArgs = ""
            "		<elementType name=\"preallocate\" default=\"0\">\n"
            "			<dataTypeRef name=\"int\"/>\n"
            "		</elementType>\n";
    }
    
    string FileArgs = ""
        "	<elementType name=\"file\">\n"
        "		<elementType name=\"name\" default=\"new file\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
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
        "		<elementType name=\"user-executable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"group-readable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"group-writable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"group-executable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"other-readable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"other-writable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"other-executable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"mode\" default=\"-1\">\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n";
    FileArgs += XFSFileArgs + "	</elementType>\n";
        
    m["file"] = SubContextCreator( SL_SubCreate_file, FileArgs );

    m["dir"] = SubContextCreator(
        SL_SubCreate_dir,
        "	<elementType name=\"dir\">\n"
        "		<elementType name=\"name\" default=\"new directory\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
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
        "		<elementType name=\"user-executable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"group-readable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"group-writable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"group-executable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"other-readable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"other-writable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"other-executable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"mode\" default=\"-1\">\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "	</elementType>\n");


    m["softlink"] = SubContextCreator(
        SL_SubCreate_softlink,
        "	<elementType name=\"softlink\">\n"
        "		<elementType name=\"name\" default=\"new link\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"link-target\" default=\"\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "	</elementType>\n");

    m["hardlink"] = SubContextCreator(
        SL_SubCreate_hardlink,
        "	<elementType name=\"hardlink\">\n"
        "		<elementType name=\"name\" default=\"new link\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"link-target\" default=\"\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "	</elementType>\n");
    
    m["fifo"] = SubContextCreator(
        SL_SubCreate_fifo,
        "	<elementType name=\"fifo\">\n"
        "		<elementType name=\"name\" default=\"new fifo\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
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
        "		<elementType name=\"user-executable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"group-readable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"group-writable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"group-executable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"other-readable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"other-writable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"other-executable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"mode\" default=\"-1\">\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "	</elementType>\n");

    m["special"] = SubContextCreator(
        SL_SubCreate_special,
        "	<elementType name=\"special\">\n"
        "		<elementType name=\"name\" default=\"new special\">\n"
        "			<dataTypeRef name=\"string\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"device\" >\n"
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
        "		<elementType name=\"user-executable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"group-readable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"group-writable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"group-executable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"other-readable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"other-writable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"other-executable\" default=\"0\">\n"
        "			<dataTypeRef name=\"bool\"/>\n"
        "		</elementType>\n"
        "		<elementType name=\"mode\" default=\"-1\">\n"
        "			<dataTypeRef name=\"int\"/>\n"
        "		</elementType>\n"
        "	</elementType>\n");

    insertAllCreatorModules( m );
}


ferris_ios::openmode
NativeContext::getSupportedOpenModes()
{
    LG_NATIVE_D << "getSupportedOpenModes() url:" << getURL() << endl;
    return ios::in | ios::out | ios::ate | ios::app | ios::trunc | ios::binary;
}


fh_context
NativeContext::getNonKernelLinkTarget()
{
    const struct stat& sb = getStat_DontFollow();
    if(S_ISLNK(sb.st_mode))
    {
        LG_NATIVE_D << "isNonKernelLink(is-link) URL:" << getURL() << endl;

        string linkTarget = getStrAttr( this, "link-target", "" );
        if( !linkTarget.empty() )
        {
            LG_NATIVE_D << "isNonKernelLink(is-link) linkTarget:" << linkTarget << endl;
            struct stat tsb;
            int rv = lstat( linkTarget.c_str(), &tsb);
            if( rv != 0 )
            {
                // The link doesn't really exist as far as the kernel is concerned.
                LG_NATIVE_D << "priv_getIStream(is-ferris-link) URL:" << getURL()
                            << " linkTarget:" << linkTarget
                            << endl;
                fh_context lc = Resolve( linkTarget );
                return lc;
            }
        }
    }
    return 0;
}
    
    
fh_istream
NativeContext::priv_getIStream( ferris_ios::openmode m )
    throw (FerrisParentNotSetError,
           CanNotGetStream,
           exception)
{
    LG_NATIVE_D << "+++ NativeContext::priv_getIStream URL:" << getURL() << endl;

    if( fh_context lc = getNonKernelLinkTarget() )
    {
        return lc->getIStream();
    }
    
//     if( isCompressedContext( this ))
//     {
//         return Factory::getCompressedChunkIOStream( this );
//     }
    
    fh_ifstream fs( getDirPath().c_str(), ferris_ios::maskOffFerrisOptions(m) );

    LG_NATIVE_D << "NativeContext::priv_getIStream() "
                << " path:" << getDirPath()
                << " m:" << m
                << " m.masked:" << ferris_ios::maskOffFerrisOptions( m )
                << " fs:" << fs
                << endl;

#ifndef PLATFORM_OSX
    if( m & ferris_ios::o_mmap )
    {
        LG_NATIVE_D << "About to create a memory mapped file for:" << getURL()
                    << " fd:" << fs->rdbuf()->fd()
                    << " m:" << m
                    << endl;
        fh_istream ret = Factory::MakeMMapIStream( fs->rdbuf()->fd(), m, getURL() );
        LG_NATIVE_D << "made memory mapped stream for fd:" << fs->rdbuf()->fd() << endl;
        return ret;
    }
#endif
    
    return fs;
}


fh_iostream
NativeContext::priv_getIOStream( ferris_ios::openmode m )
    throw (FerrisParentNotSetError,
           AttributeNotWritable,
           CanNotGetStream,
           exception)
{
    if( fh_context lc = getNonKernelLinkTarget() )
    {
        return lc->getIOStream();
    }

//     if( isCompressedContext( this ))
//     {
//         return Factory::getCompressedChunkIOStream( this );
//     }

//     if( m & ferris_ios::o_direct )
//     {
//         fh_fstream fs( getDirPath().c_str(), ferris_ios::maskOffFerrisOptions( m ) );
//         int fd = dup( fs->rdbuf()->fd() );

//         cerr << "NativeContext::priv_getIOStream() "
//              <<" setting O_DIRECT on fd:" << fd << endl;

//         int rc = fcntl( fd, F_SETFL, O_DIRECT );
//         if( rc )
//         {
//             int e = errno;
//             cerr << "Can not set O_DIRECT for fd:" << fd
//                  << " reason:" << errnum_to_string( "", e ) << endl;
//         }

//         cerr << "NativeContext::priv_getIOStream() "
//              << " returning a fd iostream on a fixed dup fd from a normal stream"
//              << endl;
//         fh_iostream ret = Factory::MakeFdIOStream( fd );
//         return ret;
//     }
    
    fh_fstream fs( getDirPath().c_str(), m );
    LG_NATIVE_D << "NativeContext::priv_getIOStream() "
                << " path:" << getDirPath()
                << " m:" << m
                << " m.masked:" << ferris_ios::maskOffFerrisOptions( m )
                << " fs:" << fs
                << endl;
    if( !fs->good() )
    {
//        const struct stat& sb = getStat_Follow();
        if( m & ios::out )
        {
            int rc = access( getDirPath().c_str(), W_OK );
            if( rc )
            {
                fh_stringstream ss;
                ss << "Request for output support on an object that is not writable" << endl;
                Throw_CanNotGetStream( tostr(ss), this );
            }
        }
    }
    
    return fs;
}

void
NativeContext::tryToFindAttributeByOverMounting( const std::string& eaname )
{
    LG_CTXREC_D << "NativeContext::tryToFindAttributeByOverMounting()"
                << " eaname:" << eaname
                << endl;
    
    if( isDir() )
        return;
    if( IsDanglingLink )
        return;
    
    _Base::tryToFindAttributeByOverMounting( eaname );
}

    void
    NativeContext::read( bool force )
    {
        LG_NATIVE_I << "NativeContext::read() force:" << force
                    << " HaveReadDir:" << HaveReadDir
                    << " ReadingDir:" << ReadingDir
                    << " getOverMountContext():" << getOverMountContext()
                    << " this:" << this
                    << " url:" << getDirPath()
                    << endl;
        
        //
        // If the inode of an already read directory
        // has changed then we can assume a new kernel filesystem might
        // be mounted here.
        //
        if( HaveReadDir && !ReadingDir && getOverMountContext() == this )
        {
            ino_t oldINode = sb_follow.st_ino;
            struct stat sb;
            stat( getDirPath().c_str(), &sb );
            force = (oldINode != sb.st_ino);
            
            LG_NATIVE_I << "NativeContext::read() force:" << force
                        << " old:" << oldINode
                        << " new:" << sb.st_ino
                        << endl;
            
        }
        LG_NATIVE_I << "NativeContext::read() force:" << force
                    << " path:" << getDirPath()
                    << endl;
        return _Base::read( force );
    }
    
    

void
NativeContext::priv_read()
{
    LG_NATIVE_I << "NativeContext::priv_read() "
                << " this:" << (void*)this
                << " path:" << getDirPath() 
                << " isActiveView():" << isActiveView()
                << endl;
    LG_CTXREC_D << "NativeContext::priv_read() "
                << " this:" << (void*)this
                << " path:" << getDirPath() << endl;
    updateMetaData();

    ino_t oldINode = sb_follow.st_ino;
    // This is where to insert the mapping //
    const struct stat& sb = getStat_Follow();

    if( IsDanglingLink )
    {
        LG_NATIVE_D << "NativeContext::priv_read( dangling link ) p:" << getDirPath() << endl;
        LG_CTXREC_D << "NativeContext::priv_read( dangling link ) p:" << getDirPath() << endl;

        stringstream ss;
        ss << "Attempt to read dangling link at path:" << getDirPath();
        Throw_FerrisNotReadableAsContext( tostr(ss), this );
    }
    
    LG_NATIVE_D << "NativeContext::priv_read() "
                << " this:" << (void*)this
                << " path:" << getDirPath() 
                <<  " S_ISDIR(sb.st_mode):" << S_ISDIR(sb.st_mode)
                << endl;
        
    if(S_ISDIR(sb.st_mode))
    {
        LG_NATIVE_D << "NativeContext::read(isDir=yes) p:" << getDirPath() << endl;
        LG_CTXREC_D << "NativeContext::read(isDir=yes) p:" << getDirPath() << endl;

        // don't try to overmount a directory NativeContext.
        m_overMountAttemptHasAlreadyFailed=1;
        clearContext();
        
#ifdef PERMIT_FAM
        if( isActiveView() )
        {
            EnsureStartReadingIsFired();
            LG_NATIVE_D << "starting active view using fam on url:" << getURL() << endl;
            LG_CTXREC_D << "starting active view using fam on url:" << getURL() << endl;
            setupFAM();
        }
        else
#endif
        {
            EnsureStartStopReadingIsFiredRAII _raii1( this );
            
            // the user doesn't want FAM overhead for this view, just read it
            // as it stands right now.

            LG_NATIVE_D << "+++warning: passive view for url:" << getURL()
                        << " areReadingDir():" << areReadingDir()
                        << endl;

            DIR *d;
            struct dirent *e;
            if ((d = opendir (getDirPath().c_str())) == NULL)
            {
                LG_NATIVE_D << "opendir() failed..." << endl;
                stringstream ss;
                ss << "can not open path:" << getDirPath();
                Throw_FerrisNotReadableAsContext( tostr(ss), this );
            }
            while( e = readdir(d) )
            {
                string fn = e->d_name;
                if( fn == "." || fn == ".." )
                    continue;
                
                LG_NATIVE_D << "passive fn:" << fn << endl;
                native_readSubContext( fn, false, true );
            }
            closedir (d);
            NumberOfSubContexts = getItems().size();
        }
    }
    else
    {
        LG_NATIVE_D << "NativeContext::priv_read( else ) p:" << getDirPath() << endl;
        LG_CTXREC_D << "NativeContext::priv_read(!dir) "
                    << " this:" << (void*)this
                    << " path:" << getDirPath() << endl;

        stringstream ss;
        ss << "Native/FerrisNotReadableAsContext for path:" << getDirPath();


//         throw FerrisNotReadableAsContext(
//             FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ),
//             (::Ferris::Factory::fcnull()), (tostr(ss)), (this));

            
        
//        cerr << tostr(ss) << endl;
//        BackTrace();
        Throw_FerrisNotReadableAsContext( tostr(ss), this );
    }
}

template <class StatDataFactory>
fh_stringstream&
SL_getUserOwnerNumberStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << sb.st_uid;
    return ss;
}

template <class StatDataFactory>
void
SL_setUserOwnerNumberStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    LG_NATIVE_D << "SL_setUserOwnerNumberStream() c:" << c->getURL()
                << endl;
    
    const struct stat& sb = StatDataFactory::getStat(c);

    uid_t owner = -1;
    gid_t group = -1;
    if(ss >> owner)
    {
        LG_NATIVE_D << "Setting user to:" << owner << " for url:" << c->getURL() << endl;

        if( !Shell::canChangeFileToUser( owner ))
        {
            fh_stringstream ss;
            ss << "Native context can not set the user number to:" << owner
               << " permission denied"
               << " file: " << c->getDirPath();
            Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
        }
        
        int rc = StatDataFactory::chown( c->getDirPath().c_str(), owner, group );
        if( rc != 0 )
        {
            int ec = errno;
            fh_stringstream ss;
            ss << "Native context can not set the owner to:" << owner
               << " file: " << c->getDirPath();
            ThrowFromErrno( ec, tostr(ss), c );
        }
        c->bumpVersion();
        return;
    }

    /* fail */
    {
        fh_stringstream ss;
        ss << "Native context can not set the owner number to:" << owner
           << " failed reading new file owner number"
           << " file: " << c->getDirPath();
        Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
    }
}

template <class StatDataFactory>
fh_stringstream&
SL_getUserOwnerNameStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    struct passwd* p = getpwuid(sb.st_uid);
    ss << ( p ? p->pw_name : "N/A" );
    return ss;
}


template <class StatDataFactory>
void
SL_setUserOwnerNameStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    LG_NATIVE_D << "SL_setUserOwnerNameStream() c:" << c->getURL() << endl;
    const struct stat& sb = StatDataFactory::getStat(c);

    uid_t owner = -1;
    gid_t group = -1;
    string userstr = "";
    
    if(ss >> userstr && userstr.length() )
    {
        struct passwd* p = getpwnam( userstr.c_str() );
        if( !p )
        {
            fh_stringstream ss;
            ss << "Native context can not set the owner to:" << userstr
               << " because there is no matching owner number for that name"
               << " file: " << c->getDirPath();
            Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
        }
        owner = p->pw_uid;
       
        LG_NATIVE_D << "Setting owner to:" << userstr
                    << " owner number:" << owner
                    << " for url:" << c->getURL() << endl;

        if( !Shell::canChangeFileToUser( owner ))
        {
            fh_stringstream ss;
            ss << "Native context can not set the user number to:" << owner
               << " permission denied"
               << " file: " << c->getDirPath();
            Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
        }
        
        int rc = StatDataFactory::chown( c->getDirPath().c_str(), owner, group );
        if( rc != 0 )
        {
            int ec = errno;
            fh_stringstream ss;
            ss << "Native context can not set the owner to:" << owner
               << " file: " << c->getDirPath();
            ThrowFromErrno( ec, tostr(ss), c );
        }
        c->bumpVersion();
        return;
    }

    /* fail */
    {
        fh_stringstream ss;
        ss << "Native context can not set the owner number to:" << owner
           << " failed reading new file owner number"
           << " file: " << c->getDirPath();
        Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
    }
}



// template <class StatDataFactory>
// fh_stringstream
// SL_getSizeStream_OLD( NativeContext* c, const std::string& rdn, EA_Atom* atom )
// {
//     const struct stat& sb = StatDataFactory::getStat(c);
//     fh_stringstream ss;
//     ss << sb.st_size;
//     return ss;
// }

template <class StatDataFactory>
fh_stringstream&
SL_getSizeStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << sb.st_size;
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getIsEjectableStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    bool v = false;
    
    static boost::regex rex = toregex("file:[/]+media/");
    boost::smatch matches;
    string earl = c->getURL();
    LG_HAL_D << "SL_getIsEjectableStream() url:" << earl << endl;
    if(boost::regex_search( earl, rex ))
    {
        LG_HAL_D << "SL_getIsEjectableStream() regex has passed for:" << earl << endl;
        try
        {
            const struct stat& sb = StatDataFactory::getStat(c);
            dev_t dt = sb.st_dev;

            fVolumeManager* fvolmgr = new fVolumeManager( 
                "com.libferris.Volume.Manager",
                "/com/libferris/Volume/Manager",
                QDBusConnection::sessionBus() );

            v = fvolmgr->isEjectable( dt );
            LG_HAL_D << "SL_getIsEjectableStream() v:" << v << " url:" << earl << endl;
        }
        catch( exception& e )
        {
//        cerr << "SL_getIsEjectableStream() e:" << e.what() << endl;
            throw;
        }
    }
    ss << v;
    return ss;
}

    static void eject( NativeContext* c )
    {
        try
        {
            const struct stat& sb = DontFollowLinks::getStat(c);
            dev_t dt = sb.st_dev;

            fh_stringstream zz;
            SL_getIsEjectableStream<DontFollowLinks>( c, "is-ejectable", 0, zz );
            if( isTrue( tostr(zz) ))
            {
//                cerr << "Ejecting disk at c:" << c->getURL() << endl;

                fVolumeManager* fvolmgr = new fVolumeManager( 
                    "com.libferris.Volume.Manager",
                    "/com/libferris/Volume/Manager",
                    QDBusConnection::sessionBus() );
                
                fh_stringstream ss;
                ss << tostr(fvolmgr->Eject( dt ));
                LG_HAL_D << "native.eject ss:" << ss.str() << endl;
                if( !tostr(ss).empty() )
                {
                    Throw_FerrisHALException( ss.str(), c );
                }
            
            }
        }
        catch( exception& e )
        {
//        cerr << "SL_getIsEjectableStream() e:" << e.what() << endl;
            throw;
        }
    }
    

void
SL_updateEjectStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    eject( c );
}

template <class StatDataFactory>
fh_stringstream&
SL_getEjectStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    eject( c );
    return ss;
}


    
void
SL_updateNullStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
}

    static void unmount( NativeContext* c )
    {
        try
        {
            const struct stat& sb = DontFollowLinks::getStat(c);
            dev_t dt = sb.st_dev;

            fh_stringstream zz;
            SL_getIsEjectableStream<DontFollowLinks>( c, "is-ejectable", 0, zz );
            if( isTrue( tostr(zz) ))
            {
                LG_HAL_D << "unmount disk at c:" << c->getURL() << endl;

                fVolumeManager* fvolmgr = new fVolumeManager( 
                    "com.libferris.Volume.Manager",
                    "/com/libferris/Volume/Manager",
                    QDBusConnection::sessionBus() );

                fh_stringstream ss;
                ss << tostr(fvolmgr->Unmount( dt ));
                if( !tostr(ss).empty() )
                {
                    Throw_FerrisHALException( ss.str(), c );
                }
            }
        }
        catch( exception& e )
        {
//        cerr << "SL_getIsEjectableStream() e:" << e.what() << endl;
            throw;
        }
    }
    
void
SL_updateUnmountStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    unmount(c);
}
    
    
template <class StatDataFactory>
fh_stringstream&
SL_getUnmountStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    unmount(c);
    return ss;
}
    
    
fh_stringstream&
SL_getIsNative( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    ss << "1";
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
n_SL_getIsDir( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << (S_ISDIR(sb.st_mode) ? 1 : 0);
    return ss;
}

bool
NativeContext::isDir()
{
    try
    {
        const struct stat& sb = FollowLinks::getStat( this );
        return S_ISDIR(sb.st_mode);
    }
    catch( CanNotDereferenceDanglingSoftLink& e )
    {
        return false;
    }
}



template <class StatDataFactory>
fh_stringstream&
SL_getIsFile( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << (S_ISREG(sb.st_mode) ? 1 : 0);
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getIsSpecial( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);

    int v = 0;
    if( !S_ISREG(sb.st_mode) && !S_ISDIR(sb.st_mode) )
        v = 1;

    ss << v;
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getIsLink( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);

    int v = 0;
    if( S_ISLNK(sb.st_mode) )
        v = 1;

    ss << v;
    return ss;
}


template <class StatDataFactory>
fh_stringstream&
SL_getHasHoles( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);

    int v = 0;
    
    if( S_ISREG(sb.st_mode) )
    {
        int blksize = sb.st_blksize ? sb.st_blksize : 1;
        
        if( sb.st_size / blksize > sb.st_blocks )
        {
            v = 1;
        }
    }
    
    ss << v;
    return ss;
}

    template <class StatDataFactory>
    fh_stringstream&
    SL_getIsSetUID( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
    {
        const struct stat& sb = StatDataFactory::getStat(c);
        int v = sb.st_mode & S_ISUID;
        ss << v;
        return ss;
    }

    template <class StatDataFactory>
    fh_stringstream&
    SL_getIsSetGID( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
    {
        const struct stat& sb = StatDataFactory::getStat(c);
        int v = sb.st_mode & S_ISGID;
        ss << v;
        return ss;
    }

    template <class StatDataFactory>
    fh_stringstream&
    SL_getIsSticky( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
    {
        const struct stat& sb = StatDataFactory::getStat(c);
        int v = sb.st_mode & S_ISVTX;
        ss << v;
        return ss;
    }
    

    

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

fh_stringstream&
SL_getRealPath( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    int path_max;
    
#ifdef PATH_MAX
    path_max = PATH_MAX;
#else
    path_max = pathconf (path, _PC_PATH_MAX);
    if (path_max <= 0)
        path_max = 4096;
#endif

    char *resolved_path = (char*)malloc( path_max + 1 );
    if( !resolved_path )
    {
        throw bad_alloc();
    }
    
    if( !realpath( c->getDirPath().c_str(), resolved_path ))
    {
        int eno = errno;
        free( resolved_path );
        fh_stringstream ss;
        ss << "Can not get realpath(3) path:" << c->getDirPath() << endl;
        Throw_CanNotGetStream( errnum_to_string( tostr(ss), eno ), c );
    }
    
    ss << resolved_path;
    free( resolved_path );
    return ss;
}
    
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

template <class StatDataFactory>
fh_stringstream&
SL_getFileMode( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    LG_NATIVE_D << "getting mode:" << sb.st_mode << " for url:" << c->getURL() << endl;
    LG_NATIVE_D << "SL_getFileMode() getting mode:" << sb.st_mode << " oct:"
                << oct << (sb.st_mode & ~S_IFMT) << endl;
    ss << oct << (sb.st_mode & ~S_IFMT);
    LG_NATIVE_D << "SL_getFileMode  c:" << c->getURL()
                << " modestr:" << tostr(ss) 
                << " mode:" << (sb.st_mode & ~S_IFMT)
                << endl;
//    BackTrace();
    return ss;
}

template <class StatDataFactory>
void
SL_setFileMode( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    LG_NATIVE_D << "SL_setFileMode 1 " << endl;
    const struct stat& sb = StatDataFactory::getStat(c);
//     ::mode_t m = 0;
//     if( ss >> m );
    string modestr = "";
    modestr = StreamToString(ss);
    ::mode_t m = Factory::MakeInitializationMode( modestr );
    LG_NATIVE_D << "SL_setFileMode c:" << c->getURL()
                << " modestr:" << modestr
                << " mode:" << m
                << endl;
    if( m )
    {
        LG_NATIVE_D << "Setting mode to:" << m << " for url:" << c->getURL() << endl;
        int rc = chmod( c->getDirPath().c_str(), m );
        if( rc != 0 )
        {
            int ec = errno;
            fh_stringstream ss;
            ss << "Native context can not set the file mode to m:" << m
               << " file: " << c->getDirPath();
            ThrowFromErrno( ec, tostr(ss), c );
        }
        c->bumpVersion();
        c->public_updateMetaData();
        return;
    }

    LG_NATIVE_D << "SL_setFileMode 2 " << endl;
    /* fail */
    {
        fh_stringstream ss;
        ss << "Native context can not set the file mode to m:" << m
           << " failed reading new file mode"
           << " file: " << c->getDirPath();
        Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
    }
}




template <class StatDataFactory>
fh_stringstream&
SL_getUserReadable( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << ( sb.st_mode & S_IRUSR ? 1 : 0);
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getUserWritable( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << ( sb.st_mode & S_IWUSR ? 1 : 0);
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getUserExecutable( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << ( sb.st_mode & S_IXUSR ? 1 : 0);
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getGroupReadable( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << ( sb.st_mode & S_IRGRP ? 1 : 0);
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getGroupWritable( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << ( sb.st_mode & S_IWGRP ? 1 : 0);
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getGroupExecutable( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << ( sb.st_mode & S_IXGRP ? 1 : 0);
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getOtherReadable( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << ( sb.st_mode & S_IROTH ? 1 : 0);
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getOtherWritable( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << ( sb.st_mode & S_IWOTH ? 1 : 0);
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getOtherExecutable( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << ( sb.st_mode & S_IXOTH ? 1 : 0);
    return ss;
}

/******************************************************************************/
/******************************************************************************/

template <class StatDataFactory>
fh_stringstream&
SL_getDeviceStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << sb.st_dev;
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getDeviceHexStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << hex << sb.st_dev;
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getDeviceMajorStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << MAJOR(sb.st_dev);
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getDeviceMinorStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << MINOR(sb.st_dev);
    return ss;
}


template <class StatDataFactory>
fh_stringstream&
SL_getDeviceMajorHexStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << hex << MAJOR(sb.st_dev);
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getDeviceMinorHexStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << hex << MINOR(sb.st_dev);
    return ss;
}


template <class StatDataFactory>
fh_stringstream&
SL_getINodeStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << sb.st_ino;
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getProtectionRawStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << sb.st_mode;
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getReadable( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    int rc = access( c->getDirPath().c_str(), R_OK );
    ss << (rc==0);
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getWritable( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    int rc = access( c->getDirPath().c_str(), W_OK );
    ss << (rc==0);
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getRunableStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    int rc = access( c->getDirPath().c_str(), X_OK );
    ss << (rc==0);
    return ss;
}


template <class StatDataFactory>
fh_stringstream&
SL_getDeletableStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    string earl = c->getURL();
    fh_context cc = c;
    
    const struct stat& sb = StatDataFactory::getStat(c);
    if( S_ISLNK(sb.st_mode) )
    {
        earl = getStrAttr( c, "link-target", earl );
        cc = Resolve( earl );
    }
    if( !cc->isParentBound() )
    {
        ss << 0;
    }
    else
    {
        fh_context p = cc->getParent();
        int rc = access( p->getDirPath().c_str(), W_OK | X_OK );
        ss << (rc==0);
    }
    return ss;
}



template <class StatDataFactory>
fh_stringstream&
SL_getProtectionLsStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& oss )
{
    const struct stat& sb = StatDataFactory::getStat(c);

    if     ( sb.st_mode == S_IFLNK  )          oss << "l";
    else if( sb.st_mode == S_IFSOCK )     oss << "s";
    else if( sb.st_mode == S_IFBLK  )     oss << "b";
    else if( sb.st_mode == S_IFDIR  )     oss << "d";
    else if( sb.st_mode == S_IFCHR  )     oss << "c";
    else if( sb.st_mode == S_IFIFO  )     oss << "p";
    else                                 oss << "-";

    if( sb.st_mode & S_IRUSR )      oss << "r";
    else                            oss << "-";
    if( sb.st_mode & S_IWUSR )      oss << "w";
    else                            oss << "-";
    if( sb.st_mode & S_ISUID )      oss << "s";
    else if( sb.st_mode & S_IXUSR ) oss << "x";
    else                            oss << "-";
    
    if( sb.st_mode & S_IRGRP )      oss << "r";
    else                            oss << "-";
    if( sb.st_mode & S_IWGRP )      oss << "w";
    else                            oss << "-";
    if( sb.st_mode & S_ISGID )      oss << "s";
    else if( sb.st_mode & S_IXGRP ) oss << "x";
    else                            oss << "-";
    
    if( sb.st_mode & S_IROTH )      oss << "r";
    else                            oss << "-";
    if( sb.st_mode & S_IWOTH )      oss << "w";
    else                            oss << "-";
    if( sb.st_mode & S_ISVTX )      oss << "T";
    else if( sb.st_mode & S_IXOTH ) oss << "x";
    else                            oss << "-";
    
    return oss;
}



template <class StatDataFactory>
fh_stringstream&
SL_getHardLinkCountStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << sb.st_nlink;
    return ss;
}




template <class StatDataFactory>
fh_stringstream&
SL_getGroupOwnerNumberStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << sb.st_gid;
    return ss;
}

template <class StatDataFactory>
void
SL_setGroupOwnerNumberStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    LG_NATIVE_D << "SL_setGroupOwnerNumberStream() c:" << c->getURL()
                << endl;
    
    const struct stat& sb = StatDataFactory::getStat(c);

    uid_t owner = -1;
    gid_t group = -1;
    if(ss >> group)
    {
        LG_NATIVE_D << "Setting group to:" << group << " for url:" << c->getURL() << endl;

        if( !Shell::canChangeFileToGroup( group ))
        {
            fh_stringstream ss;
            ss << "Native context can not set the group number to:" << group
               << " permission denied"
               << " file: " << c->getDirPath();
            Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
        }
        
        int rc = StatDataFactory::chown( c->getDirPath().c_str(), owner, group );
        if( rc != 0 )
        {
            int ec = errno;
            fh_stringstream ss;
            ss << "Native context can not set the group to:" << group
               << " file: " << c->getDirPath();
            ThrowFromErrno( ec, tostr(ss), c );
        }
        c->bumpVersion();
        return;
    }

    /* fail */
    {
        fh_stringstream ss;
        ss << "Native context can not set the group number to:" << group
           << " failed reading new file group number"
           << " file: " << c->getDirPath();
        Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
    }
}

template <class StatDataFactory>
fh_stringstream&
SL_getGroupOwnerNameStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    struct group *g = getgrgid(sb.st_gid);
    ss << ( g ? g->gr_name : "N/A" );
    return ss;
}

template <class StatDataFactory>
void
SL_setGroupOwnerNameStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    LG_NATIVE_D << "SL_setGroupOwnerNameStream() c:" << c->getURL() << endl;
    const struct stat& sb = StatDataFactory::getStat(c);

    uid_t owner = -1;
    gid_t group = -1;
    string groupstr = "";
    
    if(ss >> groupstr && groupstr.length() )
    {
        struct group* gre = getgrnam( groupstr.c_str() );
        if( !gre )
        {
            fh_stringstream ss;
            ss << "Native context can not set the group to:" << groupstr
               << " because there is no matching group number for that name"
               << " file: " << c->getDirPath();
            Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
        }
        
        group = gre->gr_gid;
        
        LG_NATIVE_D << "Setting group to:" << groupstr
                    << " groupnum:" << group
                    << " for url:" << c->getURL() << endl;

        if( !Shell::canChangeFileToGroup( group ))
        {
            fh_stringstream ss;
            ss << "Native context can not set the group number to:" << group
               << " permission denied"
               << " file: " << c->getDirPath();
            Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
        }
        
        int rc = StatDataFactory::chown( c->getDirPath().c_str(), owner, group );
        if( rc != 0 )
        {
            int ec = errno;
            fh_stringstream ss;
            ss << "Native context can not set the group to:" << group
               << " file: " << c->getDirPath();
            ThrowFromErrno( ec, tostr(ss), c );
        }
        c->bumpVersion();
        return;
    }

    /* fail */
    {
        fh_stringstream ss;
        ss << "Native context can not set the group number to:" << group
           << " failed reading new file group number"
           << " file: " << c->getDirPath();
        Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
    }
}


    
    
template <class StatDataFactory>
fh_stringstream&
SL_getDeviceTypeStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << sb.st_rdev;
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getDeviceMajorTypeStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << MAJOR(sb.st_rdev);
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getDeviceMinorTypeStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << MINOR(sb.st_rdev);
    return ss;
}
    
template <class StatDataFactory>
fh_stringstream&
SL_getBlockSizeStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << sb.st_blksize;
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getBlockCountStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << sb.st_blocks;
    return ss;
}

    
    
template <class StatDataFactory>
fh_stringstream&
SL_getCTimeRawStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << sb.st_ctime;
//    LG_EAIDX_D << "SL_getCTimeRawStream()    ctime:" << sb.st_ctime << endl;
//    LG_EAIDX_D << "SL_getCTimeRawStream() ss.str  :" << ss.str() << endl;
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getATimeRawStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    /* the file can be accessed at any time, so the safest method is to
     *  clear away the cache for this context
     */
    c->bumpVersion();
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << sb.st_atime;
    return ss;
}

template <class StatDataFactory>
fh_stringstream&
SL_getMTimeRawStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);
    ss << sb.st_mtime;
    return ss;
}

    
void
SL_setXTimeRawStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss, struct utimbuf utb )
{
    LG_NATIVE_D << "SL_setXTimeRawStream() setatime url:" << c->getURL()
                << " atime:" << utb.actime
                << " mtime:" << utb.modtime
                << endl;

    if( utb.actime == -1  || utb.modtime == -1 ) 
    {
        fh_stringstream ss;
        ss << "invalid time_t given"
           << " for updating time attribute:" << rdn
           << " on url:" << c->getURL();
        Throw_BadlyFormedTime( tostr(ss), c );
    }
    

    if( 0 != utime( c->getDirPath().c_str(), &utb ) )
    {
        int ec = errno;
        fh_stringstream ss;
        ss << "Native context can not set access and modification time."
           << " file: " << c->getDirPath();
        ThrowFromErrno( ec, tostr(ss), c );
    }
    c->bumpVersion();
}

template <class StatDataFactory>
void
SL_setATimeRawStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);

    time_t t = -1;
    ss >> t;

    struct utimbuf utb;
    utb.actime = t;
    utb.modtime = sb.st_mtime;

    SL_setXTimeRawStream( c, rdn, atom, ss, utb );
}

template <class StatDataFactory>
void
SL_setMTimeRawStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    LG_NATIVE_D << "SL_setMTimeRawStream()" << endl;
    const struct stat& sb = StatDataFactory::getStat(c);

    time_t t = -1;
    ss >> t;

    struct utimbuf utb;
    utb.actime = sb.st_atime;
	utb.modtime = t;

    SL_setXTimeRawStream( c, rdn, atom, ss, utb );
}

template <class StatDataFactory>
fh_stringstream&
SL_getFilesystemFiletypeStream( NativeContext* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
{
    const struct stat& sb = StatDataFactory::getStat(c);

    if(S_ISREG(sb.st_mode))        ss << "regular file";
    else if(S_ISDIR(sb.st_mode))   ss << "directory";
    else if(S_ISCHR(sb.st_mode))   ss << "character device";
    else if(S_ISBLK(sb.st_mode))   ss << "block device";
    else if(S_ISFIFO(sb.st_mode))  ss << "fifo";
    else if(S_ISLNK(sb.st_mode))   ss << "symbolic link";
    else if(S_ISSOCK(sb.st_mode))  ss << "socket";
    else                           ss << "unknown";
    
    return ss;
}


// template <class StatDataFactory>
// fh_stringstream
// SL_getMimetypeStream( NativeContext* c, const std::string& rdn, EA_Atom* atom )
// {
//     const struct stat& sb = StatDataFactory::getStat(c);
//     fh_stringstream ss;
//     c->ensureMimeAndFileTypeUpToDate(NativeContext::NATIVE_LIBFILE_MIME);
//     ss << c->Mimetype;
//     return ss;
// }

// template <class StatDataFactory>
// fh_stringstream
// SL_getFiletypeStream( NativeContext* c, const std::string& rdn, EA_Atom* atom )
// {
//     const struct stat& sb = StatDataFactory::getStat(c);
//     fh_stringstream ss;
//     c->ensureMimeAndFileTypeUpToDate(NativeContext::NATIVE_LIBFILE_TYPE);
//     ss << c->Filetype;
//     return ss;
// }




/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


    
fh_istream
SL_getFSName( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << Util::getFileSystemTypeString( c->getStatFS() );
    return ss;
}

fh_istream
SL_getFSMaxNameLength( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    struct statfs s = c->getStatFS();
    
    fh_stringstream ss;
#ifdef PLATFORM_OSX
    ss << MAXPATHLEN;
#else
    ss << s.f_namelen;
#endif
    return ss;
}

fh_istream
SL_getFSID( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    struct statfs s = c->getStatFS();

//     cerr << "SL_getFSID() ID1:" << s.f_fsid << endl;
//     cerr << "SL_getFSID() ID.sz:" << sizeof(s.f_fsid) << endl;
//     cerr << "s.f_fsid.__val[0] SIZE:" << sizeof( s.f_fsid.__val[0] ) << endl;
    
//     {
//         guint64 p   = s.f_fsid.__val[0];
//         guint64 sec = s.f_fsid.__val[1];
//         sec <<= 32;
//         p |= sec;
// //        guint64* p = static_cast<guint64*>(&s.f_fsid.__val[0]);
//         cerr << "SL_getFSID() ID2 p:" << p << " sec:" << sec << endl;
//     }
    
    
    
    fh_stringstream ss;
    ss << s.f_fsid;
    return ss;
}

fh_istream
SL_getFSFileNodesFree( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    struct statfs s = c->getStatFS();
    
    fh_stringstream ss;
    ss << s.f_ffree;
    return ss;
}

fh_istream
SL_getFSFileNodesTotal( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    struct statfs s = c->getStatFS();
    
    fh_stringstream ss;
    ss << s.f_files;
    return ss;
}

fh_istream
SL_getFSAvailableBlockCount( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    struct statfs s = c->getStatFS();
    
    fh_stringstream ss;
    ss << s.f_bavail;
    return ss;
}

fh_istream
SL_getFSFreeBlockCount( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    struct statfs s = c->getStatFS();
    
    fh_stringstream ss;
    ss << s.f_bfree;
    return ss;
}

    fh_istream
    SL_getFSFreeSize( NativeContext* c, const std::string& rdn, EA_Atom* atom )
    {
        struct statfs s = c->getStatFS();
        
        fh_stringstream ss;
        if( s.f_bfree )
            ss << s.f_bsize / s.f_bfree;
        else
            ss << 0;
        return ss;
    }

    
    

fh_istream
SL_getFSBlockCount( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    struct statfs s = c->getStatFS();
    
    fh_stringstream ss;
    ss << s.f_blocks;
    return ss;
}

fh_istream
SL_getFSBlockSize( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    struct statfs s = c->getStatFS();
    
    fh_stringstream ss;
    ss << s.f_bsize;
    return ss;
}

fh_istream
SL_getFSType( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    struct statfs s = c->getStatFS();
    
    fh_stringstream ss;
    ss << s.f_type;
    return ss;
}

//struct ::statfs
struct statfs
NativeContext::getStatFS()
{
    struct statfs buf;
    int rc = statfs( getDirPath().c_str(), &buf );
    if( rc != 0 )
    {
        int eno = errno;
        fh_stringstream ss;
        ss << "Can not statfs() path:" << getDirPath() << endl;
        cerr << tostr(ss) << endl;
        ThrowFromErrno( eno, tostr(ss), this );
    }
    return buf;
}



/******************************************************************************/
/******************************************************************************/
/******************************************************************************/



string
NativeContext::priv_getRecommendedEA()
{
    const static string rea = "size-human-readable,protection-ls,mtime-display,name";
    return adjustRecommendedEAForDotFiles( this, rea );
}

bool
NativeContext::getHasSubContextsGuess()
{
    bool ret = false;
    
    ensureUpdateMetaDataCalled();
    const struct stat& sb = getStat_Follow();

    if( IsDanglingLink )
    {
        return false;
    }
    
    if( sb.st_mode & S_IFDIR )
    {
        DIR *d; struct dirent *e;
        if (d = opendir( getDirPath().c_str() ))
        {
            while( true )
            {
                e = readdir (d);
                if( e && (!strcmp( e->d_name, "." ) || !strcmp( e->d_name, "..") ))
                    continue;
                ret = e;
                break;
            }
            closedir (d);
        }
    }
//     LG_NATIVE_D << "NativeContext::getHasSubContextsGuess() url:" << getURL()
//          << " ret:" << ret
//          << " hasOverMounter:" << hasOverMounter()
//          << endl;
    return ret ? ret : hasOverMounter();
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

static void setExistsSubDir_Cache( Context*c, time_t c_ctime, bool hasSubContext )
{
    fh_stringstream keyss;
    keyss << "exists-subdir-" << Util::replace_all( c->getURL(), '/', '-' );

    setEDBString( FDB_CACHE, tostr(keyss), tostr( hasSubContext ));
    keyss << "-ctime";
    setEDBString( FDB_CACHE, tostr(keyss), tostr( c_ctime ));
}

static int getExistsSubDir_Cache( Context*c, time_t c_ctime )
{
    fh_stringstream keyss;
    keyss << "exists-subdir-" << Util::replace_all( c->getURL(), '/', '-' );

    try
    {
        int ret   = toint(getEDBString( FDB_CACHE, tostr(keyss), "0", true, true ));
        keyss << "-ctime";
        time_t ct = toType<time_t>(getEDBString( FDB_CACHE, tostr(keyss), "0", true, true ));

        /* If the time the cache was created is still fresh then return cache */
        if( ct >= c_ctime )
        {
            return ret;
        }
    }
    catch(...)
    {}

    return -1;
}



/**
 * Return 1 if there is atleast one subdirectory of this node.
 */
fh_stringstream
SL_getExistsSubDir( NativeContext* c, const std::string& rdn, EA_Atom* atom, int m = 0 )
{
    bool hasSubContext = false;

    const struct stat& cstat = c->getStat_DontFollow();
    time_t c_ctime = cstat.st_ctime;

    int rc = -1; // getExistsSubDir_Cache( c, c_ctime );
    
    if( rc != -1 )
    {
        hasSubContext = rc;
    }
    else
    {
        if( DIR *d = opendir( c->getDirPath().c_str() ) )
        {
            while( struct dirent *de = readdir(d) )
            {
                const char* rdn = de->d_name;
            
                if( !strcmp( rdn, "." ) || !strcmp( rdn, "..") )
                    continue;

                fh_stringstream pathss;
                pathss << c->getDirPath() << "/" << rdn;
                
                struct stat sbuf;
                if( 0 != lstat( tostr(pathss).c_str(), &sbuf ))
                {
                    /* error */
                    hasSubContext = false;
                    break;
                }
            
                if(S_ISDIR(sbuf.st_mode))
                {
                    hasSubContext = true;
                    break;
                }
            }
        
            closedir(d);

            /* This could blow up if there are no subcontexts made by the overmounter */
            if( !hasSubContext )
            {
                hasSubContext = c->public_hasOverMounter() > 0;
            }
            
        }
    }
    
    
//    setExistsSubDir_Cache( c, c_ctime, hasSubContext );
    
    fh_stringstream ss;
    ss << hasSubContext;
    return ss;
}

fh_stringstream
SL_getExistsSubDir_gcc43_bug( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getExistsSubDir( c, rdn, atom );
}
    


/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

fh_stringstream
SL_getXSubcontextHardLinkCountIOStream( Context* c, const std::string& rdn,
                                        EA_Atom* atom, int m = 0  )
{
    fh_display_aggdata d = getCachedContextAggregateData( c, m );

    fh_stringstream ss;
    ss << d->getData( m ).hardlinkcount;
    return ss;
}

fh_stringstream
SL_getXSubcontextSizeIOStream( Context* c, const std::string& rdn,
                               EA_Atom* atom, int m = 0  )
{
    fh_display_aggdata d = getCachedContextAggregateData( c, m );

    fh_stringstream ss;
    ss << d->getData( m ).size;
    return ss;
}

fh_stringstream
SL_getXSubcontextSizeInBlocksIOStream( Context* c, const std::string& rdn,
                                       EA_Atom* atom, int m = 0  )
{
    fh_display_aggdata d = getCachedContextAggregateData( c, m );
    
    fh_stringstream ss;
    ss << d->getData( m ).sizeinblocks;
    return ss;
}

fh_stringstream
SL_getXSubcontextFileCountIOStream( Context* c, const std::string& rdn,
                                    EA_Atom* atom, int m = 0  )
{
    fh_display_aggdata d = getCachedContextAggregateData( c, m );
    
    fh_stringstream ss;
    ss << d->getData( m ).filecount;
    return ss;
}

fh_stringstream
SL_getXSubcontextDirCountIOStream( Context* c, const std::string& rdn,
                                   EA_Atom* atom, int m = 0  )
{
    fh_display_aggdata d = getCachedContextAggregateData( c, m );
    
    fh_stringstream ss;
    ss << d->getData( m ).dircount;
    return ss;
}

fh_stringstream
SL_getXSubcontextOldestMTimeRawIOStream( Context* c, const std::string& rdn,
                                         EA_Atom* atom, int m = 0  )
{
    fh_display_aggdata d = getCachedContextAggregateData( c, m );
    
    fh_stringstream ss;
    ss << d->getData( m ).oldestmtime;
    return ss;
}

fh_stringstream
SL_getXSubcontextOldestCTimeRawIOStream( Context* c, const std::string& rdn,
                                         EA_Atom* atom, int m = 0  )
{
    fh_display_aggdata d = getCachedContextAggregateData( c, m );

    fh_stringstream ss;
    ss << d->getData( m ).oldestctime;
    return ss;
}

fh_stringstream
SL_getXSubcontextOldestATimeRawIOStream( Context* c, const std::string& rdn, EA_Atom* atom, int m = 0  )
{
    fh_display_aggdata d = getCachedContextAggregateData( c, m );

    fh_stringstream ss;
    ss << d->getData( m ).oldestatime;
    return ss;
}

fh_stringstream
SL_getXSubcontextOldestMTimeURLIOStream( Context* c, const std::string& rdn, EA_Atom* atom, int m = 0  )
{
    fh_display_aggdata d = getCachedContextAggregateData( c, m );

    fh_stringstream ss;
    ss << d->getData( m ).oldestmtime_url;
    return ss;
}

fh_stringstream
SL_getXSubcontextOldestCTimeURLIOStream( Context* c, const std::string& rdn,
                                         EA_Atom* atom, int m = 0  )
{
    fh_display_aggdata d = getCachedContextAggregateData( c, m );

    fh_stringstream ss;
    ss << d->getData( m ).oldestctime_url;
    return ss;
}

fh_stringstream
SL_getXSubcontextOldestATimeURLIOStream( Context* c, const std::string& rdn,
                                         EA_Atom* atom, int m = 0 )
{
    fh_display_aggdata d = getCachedContextAggregateData( c, m );

    fh_stringstream ss;
    ss << d->getData( m ).oldestatime_url;
    return ss;
}

fh_stringstream
SL_getPreallocationAtTailStream( NativeContext* c, const std::string& rdn,
                                 EA_Atom* atom, int m )
{
    guint64 prealloc = 0;

#ifdef FERRIS_HAVE_XFS
#endif
    
    fh_stringstream ss;
    ss << prealloc;
    return ss;
}
fh_stringstream
SL_getPreallocationAtTailStream_gcc43_bug( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getPreallocationAtTailStream( c, rdn, atom, 0 );
}

void
SL_updatePreallocationAtTail( NativeContext* c, const std::string& rdn, EA_Atom* atom,

                              fh_istream ss )
{
    guint64 newprealloc = toType<guint64>( getFirstLine(ss) );
#ifdef FERRIS_HAVE_XFS
#endif
}



/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

fh_stringstream
SL_getSubcontextHardLinkCountIOStream( Context* c, const std::string& rdn,
                                       EA_Atom* atom, int m = 0 )
{
    return SL_getXSubcontextHardLinkCountIOStream( c, rdn, atom, 0 );
}
fh_stringstream
SL_getSubcontextHardLinkCountIOStream_gcc43_bug( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextHardLinkCountIOStream( c, rdn, atom, 0 );
}
    

    
fh_stringstream
SL_getRecursiveSubcontextHardLinkCountIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextHardLinkCountIOStream( c, rdn, atom, AGGDATA_RECURSIVE );
}

fh_stringstream
SL_getSubcontextSizeIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextSizeIOStream( c, rdn, atom, 0 );
}

fh_stringstream
SL_getRecursiveSubcontextSizeIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextSizeIOStream( c, rdn, atom, AGGDATA_RECURSIVE );
}

fh_stringstream
SL_getSubcontextSizeInBlocksIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextSizeInBlocksIOStream( c, rdn, atom, 0 );
}

fh_stringstream
SL_getRecursiveSubcontextSizeInBlocksIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextSizeInBlocksIOStream( c, rdn, atom, AGGDATA_RECURSIVE );
}

fh_stringstream
SL_getSubcontextFileCountIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextFileCountIOStream( c, rdn, atom, 0 );
}

fh_stringstream
SL_getRecursiveSubcontextFileCountIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextFileCountIOStream( c, rdn, atom, AGGDATA_RECURSIVE );
}

fh_stringstream
SL_getSubcontextDirCountIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextDirCountIOStream( c, rdn, atom, 0 );
}

fh_stringstream
SL_getRecursiveSubcontextDirCountIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextDirCountIOStream( c, rdn, atom, AGGDATA_RECURSIVE );
}

fh_stringstream
SL_getSubcontextOldestMTimeRawIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextOldestMTimeRawIOStream( c, rdn, atom, 0 );
}

fh_stringstream
SL_getRecursiveSubcontextOldestMTimeRawIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextOldestMTimeRawIOStream( c, rdn, atom, AGGDATA_RECURSIVE );
}

fh_stringstream
SL_getSubcontextOldestCTimeRawIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextOldestCTimeRawIOStream( c, rdn, atom, 0 );
}

fh_stringstream
SL_getRecursiveSubcontextOldestCTimeRawIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextOldestCTimeRawIOStream( c, rdn, atom, AGGDATA_RECURSIVE );
}

fh_stringstream
SL_getSubcontextOldestATimeRawIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextOldestATimeRawIOStream( c, rdn, atom, 0 );
}

fh_stringstream
SL_getRecursiveSubcontextOldestATimeRawIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextOldestATimeRawIOStream( c, rdn, atom, AGGDATA_RECURSIVE );
}

fh_stringstream
SL_getSubcontextOldestMTimeURLIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextOldestMTimeURLIOStream( c, rdn, atom, 0 );
}

fh_stringstream
SL_getRecursiveSubcontextOldestMTimeURLIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextOldestMTimeURLIOStream( c, rdn, atom, AGGDATA_RECURSIVE );
}

fh_stringstream
SL_getSubcontextOldestCTimeURLIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextOldestCTimeURLIOStream( c, rdn, atom, 0 );
}

fh_stringstream
SL_getRecursiveSubcontextOldestCTimeURLIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextOldestCTimeURLIOStream( c, rdn, atom, AGGDATA_RECURSIVE );
}

fh_stringstream
SL_getSubcontextOldestATimeURLIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextOldestATimeURLIOStream( c, rdn, atom, 0 );
}

fh_stringstream
SL_getRecursiveSubcontextOldestATimeURLIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getXSubcontextOldestATimeURLIOStream( c, rdn, atom, AGGDATA_RECURSIVE );
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

#ifdef HAVE_LIBRPM

//
// Note that use of get_rpm_header() and get_rpmfi() both rely on a
// open rpmdb handle. Such a handle can be created using
//
// rpmdb db = get_rpmdb();
// rpmdb_releaser dum1( db );
// and the releaser will release the handle for you
//
struct rpmState : public CacheHandlable
{
    rpmfi fi;
    rpmdbMatchIterator mi;
    Header hdr;

    rpmState();
    ~rpmState();
            
};

rpmState::rpmState()
    :
    mi( 0 ),
    fi( 0 ),
    hdr( 0 )
{
}

rpmState::~rpmState()
{
//    cerr << "~rpmState() " << endl;
    
    if( fi )
        rpmfiFree( fi );
    if( hdr )
    {
    }
    if( mi )
        rpmdbFreeIterator( mi );
}


//     template< class Key, class Value >
//     class FERRISEXP_API Cache

static Cache< NativeContext*, fh_rpmState >& getRpmStateCache()
{
    typedef Cache< NativeContext*, fh_rpmState > cache_t;
    static cache_t cache;
    static bool v = true;
    if( v )
    {
        v = false;
        cache.setMaxCollectableSize( 20 );
        cache.setTimerInterval( 3000 );
    }
    return cache;
}


fh_rpmState
NativeContext::get_rpm_header( rpmdb db )
{
    if( DontHaveRPMEntry )
        return 0;
    
    {
        fh_rpmState st = getRpmStateCache().get( this );
        if( st && st->mi && st->hdr )
        {
//            cerr << "Cache hit!" << endl;
            return st;
        }
    }
    
    fh_rpmState st = new rpmState();
    rpmTag match_type = RPMTAG_BASENAMES;
    st->mi = rpmdbInitIterator( db,
                                match_type,
                                getDirPath().c_str(),
                                0 );
    if( st->mi )
    {
        st->hdr = rpmdbNextIterator( st->mi );
        getRpmStateCache().put( this, st );
        return st;
    }

    DontHaveRPMEntry = true;
    return 0;
}

fh_rpmState
NativeContext::get_rpmfi( rpmdb db )
{
    fh_rpmState st = get_rpm_header( db );
    if( !st || !st->hdr )
        return 0;

    if( st->fi )
    {
        rpmfiFree( st->fi );
    }
    
    Header h = st->hdr;
    rpmts ts = NULL;
    st->fi = rpmfiNew( ts, h, RPMTAG_BASENAMES, 1 );

    string path = getDirPath();
    for( rpmfiInit( st->fi, 0 ); rpmfiNext( st->fi ) >= 0; )
    {
        const char* fileName  = rpmfiFN( st->fi );
//         cerr << "get_rpmfi() filename:" << fileName
//              << " getDirPath():" << path
//              << endl;
        
        if( path == fileName )
        {
            return st;
        }
    }
    
    LG_NATIVE_D << "get_rpmfi() cant find rpmfi for path:" << path << endl;
    return st;
}


fh_stringstream
NativeContext::SL_getRPMVerifyFlag( NativeContext* c, int flag )
{
    fh_stringstream ss;
    if( c->DontHaveRPMEntry )
        return ss;

    rpmdb db = get_rpmdb();
    rpmdb_releaser dum1( db );

    bool verifyOK = true;
    rpmVerifyAttrs res;
    rpmVerifyAttrs omitMask = (rpmVerifyAttrs)~(flag);
    rpmts ts = NULL;
    fh_rpmState st = c->get_rpmfi( db );
    if( st && st->fi )
    {
        rpmfi fi = st->fi;
        int e = rpmVerifyFile( ts, fi, &res, omitMask );
        if( e )
        {
//            cerr << "verify e:" << e << endl;
            verifyOK = false;
        }
        else
        {
            if( res & flag )
                verifyOK = false;
        }
    }
    else
        c->DontHaveRPMEntry = 1;

    ss << verifyOK;
    return ss;
}


fh_stringstream
NativeContext::SL_getRPMVerifySize( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getRPMVerifyFlag( c, RPMVERIFY_FILESIZE );
}

fh_stringstream
NativeContext::SL_getRPMVerifyMode( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getRPMVerifyFlag( c, RPMVERIFY_MODE );
}
    
fh_stringstream
NativeContext::SL_getRPMVerifyMD5( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getRPMVerifyFlag( c, RPMVERIFY_MD5 );
}

fh_stringstream
NativeContext::SL_getRPMVerifyDevice( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getRPMVerifyFlag( c, RPMVERIFY_RDEV );
}

fh_stringstream
NativeContext::SL_getRPMVerifyOwner( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getRPMVerifyFlag( c, RPMVERIFY_USER );
}

fh_stringstream
NativeContext::SL_getRPMVerifyGroup( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getRPMVerifyFlag( c, RPMVERIFY_GROUP );
}

fh_stringstream
NativeContext::SL_getRPMVerifyMTime( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getRPMVerifyFlag( c, RPMVERIFY_MTIME );
}

fh_stringstream
NativeContext::SL_getRPMFileFlag( NativeContext* c, rpmfileAttrs a )
{
    fh_stringstream ss;
    if( c->DontHaveRPMEntry )
        return ss;

    rpmdb db = get_rpmdb();
    rpmdb_releaser dum1( db );

    fh_rpmState st = c->get_rpmfi( db );
    if( st && st->fi )
    {
        rpmfi fi = st->fi;
        rpmfileAttrs fileAttrs = (rpmfileAttrs)rpmfiFFlags(fi);
        bool v = fileAttrs & a;
        ss << v;
    }
    else
        c->DontHaveRPMEntry = 1;

    return ss;
}


fh_stringstream
NativeContext::SL_getRPMIsConfig( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getRPMFileFlag( c, RPMFILE_CONFIG );
}

fh_stringstream
NativeContext::SL_getRPMIsDoc( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getRPMFileFlag( c, RPMFILE_DOC );
}

fh_stringstream
NativeContext::SL_getRPMIsGhost( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getRPMFileFlag( c, RPMFILE_GHOST );
}

fh_stringstream
NativeContext::SL_getRPMIsLicense( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getRPMFileFlag( c, RPMFILE_LICENSE );
}

fh_stringstream
NativeContext::SL_getRPMIsPubkey( NativeContext* c,const std::string&,EA_Atom*)
{
    return SL_getRPMFileFlag( c, RPMFILE_PUBKEY );
}

fh_stringstream
NativeContext::SL_getRPMIsReadme( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getRPMFileFlag( c, RPMFILE_README );
}

fh_stringstream
NativeContext::SL_getRPMHeaderString( NativeContext* c, int_32 tag )
{
    fh_stringstream ss;
    if( c->DontHaveRPMEntry )
    {
        LG_NATIVE_D << "getRPMHeaderString, no entry url:" << c->getURL() << " tag:" << tag << endl;
        return ss;
    }
    
    rpmdb db = get_rpmdb();
    rpmdb_releaser dum1( db );
    
    const char *cstr;
    fh_rpmState st = c->get_rpm_header( db );
    if( st && st->hdr )
    {
        Header h = st->hdr;
        rpmHeaderGetEntry( h, tag, NULL, (void **)&cstr, NULL);
        LG_NATIVE_D << "getRPMHeaderString, cstr:" << (cstr!=0)
                    << " url:" << c->getURL() << " tag:" << tag << endl;
        if( cstr )
            ss << cstr;
    }
    else
    {
        LG_NATIVE_D << "getRPMHeaderString, setting no entry url:" << c->getURL() << " tag:" << tag << endl;
        c->DontHaveRPMEntry = 1;
    }
    
    return ss;
}

fh_stringstream
NativeContext::SL_getRPMPackage( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getRPMHeaderString( c, RPMTAG_NAME );
}

fh_stringstream
NativeContext::SL_getRPMVersion( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getRPMHeaderString( c, RPMTAG_VERSION );
}

fh_stringstream
NativeContext::SL_getRPMRelease( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getRPMHeaderString( c, RPMTAG_RELEASE );
}

fh_stringstream
NativeContext::SL_getRPMInfoURL( NativeContext* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getRPMHeaderString( c, RPMTAG_URL );
}

    fh_stringstream
    NativeContext::SL_getRPMVendor( NativeContext* c, const std::string& rdn, EA_Atom* atom )
    {
        return SL_getRPMHeaderString( c, RPMTAG_VENDOR );
    }

    fh_stringstream
    NativeContext::SL_getRPMDistribution( NativeContext* c, const std::string& rdn, EA_Atom* atom )
    {
        return SL_getRPMHeaderString( c, RPMTAG_DISTRIBUTION );
    }

    fh_stringstream
    NativeContext::SL_getRPMLicense( NativeContext* c, const std::string& rdn, EA_Atom* atom )
    {
        return SL_getRPMHeaderString( c, RPMTAG_LICENSE );
    }

    fh_stringstream
    NativeContext::SL_getRPMPackager( NativeContext* c, const std::string& rdn, EA_Atom* atom )
    {
        return SL_getRPMHeaderString( c, RPMTAG_PACKAGER );
    }
    
    fh_stringstream
    NativeContext::SL_getRPMGroup( NativeContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream tmp = SL_getRPMHeaderString( c, RPMTAG_GROUP );
        fh_stringstream ret;
        string s;
        getline( tmp, s );
        ret << s;
        return ret;
    }

    fh_stringstream
    NativeContext::SL_getRPMBuildtime( NativeContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ret;
        fh_stringstream tmpss = SL_getRPMHeaderString( c, RPMTAG_BUILDTIME );
        string s = tostr(tmpss);
        if( s.length() )
        {
            time_t* tv = (time_t*)s.data();
            ret << (*tv);
        }
        return ret;
    }

    fh_stringstream
    NativeContext::SL_getRPMSummary( NativeContext* c, const std::string& rdn, EA_Atom* atom )
    {
        return SL_getRPMHeaderString( c, RPMTAG_SUMMARY );
    }
    
    


#endif
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

stringset_t&
getNativeStatelessEANames()
{
    static stringset_t ret;
    return ret;
}

void
NativeContext::createStateLessAttributes( bool force )
{
    static Util::SingleShot virgin;
    if( virgin() )
    {
        LG_NATIVE_D << "NativeContext::createStateLessAttributes()" << endl;

        
        SLEAPI( "is-native",  SL_getIsNative,                     XSD_BASIC_BOOL );
        SLEAPI( "is-file",    SL_getIsFile<FollowLinks>,          XSD_BASIC_BOOL );
        SLEAPI( "is-special", SL_getIsSpecial<FollowLinks>,       XSD_BASIC_BOOL );
        SLEAPI( "is-link",    SL_getIsLink<DontFollowLinks>,      XSD_BASIC_BOOL );
        SLEAPI( "has-holes",  SL_getHasHoles<FollowLinks>,        XSD_BASIC_BOOL );
        SLEAPI( "is-setuid",  SL_getIsSetUID<FollowLinks>,          XSD_BASIC_BOOL );
        SLEAPI( "is-setgid",  SL_getIsSetGID<FollowLinks>,          XSD_BASIC_BOOL );
        SLEAPI( "is-sticky",  SL_getIsSticky<FollowLinks>,          XSD_BASIC_BOOL );
        SLEAPI( "dontfollow-is-dir", n_SL_getIsDir<DontFollowLinks>,         XSD_BASIC_BOOL );
        SLEAPI( "dontfollow-is-file", SL_getIsFile<DontFollowLinks>,       XSD_BASIC_BOOL );
        SLEAPI( "dontfollow-is-special", SL_getIsSpecial<DontFollowLinks>, XSD_BASIC_BOOL );
        SLEAPI( "dontfollow-is-link", SL_getIsLink<DontFollowLinks>,       XSD_BASIC_BOOL );
        SLEAPI( "dontfollow-has-holes", SL_getHasHoles<DontFollowLinks>,   XSD_BASIC_BOOL );
        SLEAPI( "dontfollow-is-setuid",  SL_getIsSetUID<DontFollowLinks>,          XSD_BASIC_BOOL );
        SLEAPI( "dontfollow-is-setgid",  SL_getIsSetGID<DontFollowLinks>,          XSD_BASIC_BOOL );
        SLEAPI( "dontfollow-is-sticky",  SL_getIsSticky<DontFollowLinks>,          XSD_BASIC_BOOL );

        SLEAPI( "realpath", SL_getRealPath, XSD_BASIC_STRING );
        
        SLEAPI( "mode",
                SL_getFileMode<DontFollowLinks>, 
                SL_setFileMode<DontFollowLinks>,
                FXD_MODE_T );
        SLEAPI( "user-readable",   SL_getUserReadable<FollowLinks>,    XSD_BASIC_BOOL );
        SLEAPI( "user-writable",   SL_getUserWritable<FollowLinks>,    XSD_BASIC_BOOL );
        SLEAPI( "user-executable", SL_getUserExecutable<FollowLinks>,  XSD_BASIC_BOOL );
        SLEAPI( "group-readable",  SL_getGroupReadable<FollowLinks>,   XSD_BASIC_BOOL );
        SLEAPI( "group-writable",  SL_getGroupWritable<FollowLinks>,   XSD_BASIC_BOOL );
        SLEAPI( "group-executable",SL_getGroupExecutable<FollowLinks>, XSD_BASIC_BOOL );
        SLEAPI( "other-readable",  SL_getOtherReadable<FollowLinks>,   XSD_BASIC_BOOL );
        SLEAPI( "other-writable",  SL_getOtherWritable<FollowLinks>,   XSD_BASIC_BOOL );
        SLEAPI( "other-executable",SL_getOtherExecutable<FollowLinks>, XSD_BASIC_BOOL );

        SLEAPI( "readable",   SL_getReadable<FollowLinks>,      XSD_BASIC_BOOL );
        SLEAPI( "writable",   SL_getWritable<FollowLinks>,      XSD_BASIC_BOOL );
        SLEAPI( "runable",    SL_getRunableStream<FollowLinks>, XSD_BASIC_BOOL );
        SLEAPI( "deletable",  SL_getDeletableStream<FollowLinks>, XSD_BASIC_BOOL );
        SLEAPI( "protection-raw", SL_getProtectionRawStream<FollowLinks>, FXD_MODE_T );
        SLEAPI( "protection-ls", SL_getProtectionLsStream<FollowLinks>,   FXD_MODE_STRING_T );
        SLEAPI( "dontfollow-protection-raw", SL_getProtectionRawStream<DontFollowLinks>, FXD_MODE_T );
        SLEAPI( "dontfollow-protection-ls", SL_getProtectionLsStream<DontFollowLinks>,   FXD_MODE_STRING_T );

//         SLEA( "mtime-old",
//               SL_getMTimeRawStream<FollowLinks>,
//               SL_getMTimeRawStream<FollowLinks>,
//               SL_setMTimeRawStream<FollowLinks>,
//               FXD_UNIXEPOCH_T );
        SLEAPI( "mtime",
                SL_getMTimeRawStream<FollowLinks>,
                SL_setMTimeRawStream<FollowLinks>,
                FXD_UNIXEPOCH_T );
        SLEAPI( "dontfollow-mtime", SL_getMTimeRawStream<DontFollowLinks>, FXD_UNIXEPOCH_T );
        SLEAPI( "atime",
                SL_getATimeRawStream<FollowLinks>,
                SL_setATimeRawStream<FollowLinks>,
                FXD_UNIXEPOCH_T );
        SLEAPI( "dontfollow-atime", SL_getATimeRawStream<DontFollowLinks>, FXD_UNIXEPOCH_T );
        SLEAPI( "ctime",                SL_getCTimeRawStream<FollowLinks>,     FXD_UNIXEPOCH_T );
        SLEAPI( "dontfollow-ctime",     SL_getCTimeRawStream<DontFollowLinks>, FXD_UNIXEPOCH_T );
        SLEAPI( "inode",                SL_getINodeStream<FollowLinks>,        FXD_INODE_T );
        SLEAPI( "dontfollow-inode",     SL_getINodeStream<DontFollowLinks>,    FXD_INODE_T );
        SLEAPI( "hard-link-count",      SL_getHardLinkCountStream<FollowLinks>,XSD_BASIC_INT );
        SLEAPI( "dontfollow-hard-link-count",
                SL_getHardLinkCountStream<DontFollowLinks>, XSD_BASIC_INT );
        SLEAPI( "filesystem-filetype",
                SL_getFilesystemFiletypeStream<FollowLinks>,     XSD_BASIC_STRING );
        SLEAPI( "dontfollow-filesystem-filetype",
                SL_getFilesystemFiletypeStream<DontFollowLinks>, XSD_BASIC_STRING ); 
        SLEAPI( "group-owner-number",
                SL_getGroupOwnerNumberStream<FollowLinks>,
                SL_setGroupOwnerNumberStream<FollowLinks>,
                FXD_GID_T );
        SLEAPI( "dontfollow-group-owner-number",
                SL_getGroupOwnerNumberStream<DontFollowLinks>, FXD_GID_T );
        SLEAPI( "group-owner-name",
                SL_getGroupOwnerNameStream<FollowLinks>,
                SL_setGroupOwnerNameStream<FollowLinks>, FXD_GROUPNAME );
        SLEAPI( "dontfollow-group-owner-name",
                SL_getGroupOwnerNameStream<DontFollowLinks>, FXD_GROUPNAME );
        SLEAPI( "user-owner-number",
                SL_getUserOwnerNumberStream<FollowLinks>,
                SL_setUserOwnerNumberStream<FollowLinks>, FXD_UID_T );
        SLEAPI( "dontfollow-user-owner-number",
                SL_getUserOwnerNumberStream<DontFollowLinks>, FXD_UID_T );
        SLEAPI( "user-owner-name",
                SL_getUserOwnerNameStream<FollowLinks>,
                SL_setUserOwnerNameStream<FollowLinks>, FXD_USERNAME );
        SLEAPI( "dontfollow-user-owner-name",
                SL_getUserOwnerNameStream<DontFollowLinks>,
                SL_setUserOwnerNameStream<DontFollowLinks>, FXD_USERNAME );
        SLEAPI( "device-type",            SL_getDeviceTypeStream<FollowLinks>, XSD_BASIC_INT );
        SLEAPI( "device-type-major",      SL_getDeviceMajorTypeStream<FollowLinks>, XSD_BASIC_INT );
        SLEAPI( "device-type-minor",      SL_getDeviceMinorTypeStream<FollowLinks>, XSD_BASIC_INT );
        SLEAPI( "dontfollow-device-type", SL_getDeviceTypeStream<DontFollowLinks>, XSD_BASIC_INT );
        SLEAPI( "dontfollow-device-type-major", SL_getDeviceMajorTypeStream<DontFollowLinks>, XSD_BASIC_INT );
        SLEAPI( "dontfollow-device-type-minor", SL_getDeviceMinorTypeStream<DontFollowLinks>, XSD_BASIC_INT );
        SLEAPI( "device",                  SL_getDeviceStream<FollowLinks>, XSD_BASIC_INT );
        SLEAPI( "device-hex",              SL_getDeviceHexStream<FollowLinks>, XSD_BASIC_STRING );
        SLEAPI( "device-major",            SL_getDeviceMajorStream<FollowLinks>, XSD_BASIC_INT );
        SLEAPI( "device-minor",            SL_getDeviceMinorStream<FollowLinks>, XSD_BASIC_INT );
        SLEAPI( "device-major-hex",        SL_getDeviceMajorHexStream<FollowLinks>, XSD_BASIC_STRING );
        SLEAPI( "device-minor-hex",        SL_getDeviceMinorHexStream<FollowLinks>, XSD_BASIC_STRING );
        SLEAPI( "dontfollow-device",       SL_getDeviceStream<DontFollowLinks>, XSD_BASIC_INT );
        SLEAPI( "dontfollow-device-hex",   SL_getDeviceHexStream<DontFollowLinks>, XSD_BASIC_STRING );
        SLEAPI( "dontfollow-device-major", SL_getDeviceMajorStream<DontFollowLinks>, XSD_BASIC_INT );
        SLEAPI( "dontfollow-device-minor", SL_getDeviceMinorStream<DontFollowLinks>, XSD_BASIC_INT );
        SLEAPI( "dontfollow-device-major-hex", SL_getDeviceMajorHexStream<DontFollowLinks>, XSD_BASIC_STRING );
        SLEAPI( "dontfollow-device-minor-hex", SL_getDeviceMinorHexStream<DontFollowLinks>, XSD_BASIC_STRING );
        SLEAPI( "block-size",             SL_getBlockSizeStream<FollowLinks>, XSD_BASIC_INT );
        SLEAPI( "dontfollow-block-size",  SL_getBlockSizeStream<DontFollowLinks>, XSD_BASIC_INT );
        SLEAPI( "block-count",            SL_getBlockCountStream<FollowLinks>, XSD_BASIC_INT );
        SLEAPI( "dontfollow-block-count", SL_getBlockCountStream<DontFollowLinks>, XSD_BASIC_INT );
//        SLEA( "size-old", SL_getSizeStream_OLD<FollowLinks>, FXD_FILESIZE );
        SLEAPI( "size", SL_getSizeStream<FollowLinks>, FXD_FILESIZE );
        SLEAPI( "dontfollow-size", SL_getSizeStream<DontFollowLinks>, FXD_FILESIZE );
        
//         SLEA( "mimetype", SL_getMimetypeStream<FollowLinks> );
//         SLEA( "filetype", SL_getFiletypeStream<FollowLinks> );

        SLEAPI( "is-ejectable", SL_getIsEjectableStream<DontFollowLinks>, XSD_BASIC_BOOL );
        SLEAPI( "eject",
                SL_getEjectStream<DontFollowLinks>,
                SL_updateEjectStream,
                XSD_BASIC_BOOL );
        SLEAPI( "unmount",
                SL_getUnmountStream<DontFollowLinks>,
                SL_updateUnmountStream,
                XSD_BASIC_BOOL );
        

#ifdef FERRIS_HAVE_XFS
        SLEA( "preallocation-at-tail",
              SL_getPreallocationAtTailStream_gcc43_bug,
              SL_getPreallocationAtTailStream_gcc43_bug,
              SL_updatePreallocationAtTail,
              FXD_FILESIZE );
#endif

        /*******************************************************************************/
        /*******************************************************************************/
        /*******************************************************************************/
        
        SLEA( "exists-subdir", SL_getExistsSubDir_gcc43_bug, XSD_BASIC_BOOL );
        
        /*******************************************************************************/
        /*******************************************************************************/
        /*******************************************************************************/

        SLEA( "subcontext-hardlink-count",
              SL_getSubcontextHardLinkCountIOStream_gcc43_bug,
              SL_getSubcontextHardLinkCountIOStream_gcc43_bug,
              SL_FlushAggregateData,
              XSD_BASIC_INT );
        
        SLEA( "subcontext-size",
              SL_getSubcontextSizeIOStream,
              SL_getSubcontextSizeIOStream,
              SL_FlushAggregateData,
              FXD_FILESIZE );

        SLEA( "subcontext-size-in-blocks",
              SL_getSubcontextSizeInBlocksIOStream,
              SL_getSubcontextSizeInBlocksIOStream,
              SL_FlushAggregateData,
              XSD_BASIC_INT );
              
        SLEA( "subcontext-file-count",
              SL_getSubcontextFileCountIOStream,
              SL_getSubcontextFileCountIOStream,
              SL_FlushAggregateData,
              XSD_BASIC_INT );

        SLEA( "subcontext-dir-count",
              SL_getSubcontextDirCountIOStream,
              SL_getSubcontextDirCountIOStream,
              SL_FlushAggregateData,
              XSD_BASIC_INT );

        SLEA( "subcontext-oldest-mtime",
              SL_getSubcontextOldestMTimeRawIOStream,
              SL_getSubcontextOldestMTimeRawIOStream,
              SL_FlushAggregateData,
              FXD_UNIXEPOCH_T );

        SLEA( "subcontext-oldest-ctime",
              SL_getSubcontextOldestCTimeRawIOStream,
              SL_getSubcontextOldestCTimeRawIOStream,
              SL_FlushAggregateData,
              FXD_UNIXEPOCH_T );

        SLEA( "subcontext-oldest-atime",
              SL_getSubcontextOldestATimeRawIOStream,
              SL_getSubcontextOldestATimeRawIOStream,
              SL_FlushAggregateData,
              FXD_UNIXEPOCH_T );

        SLEA( "subcontext-oldest-mtime-url",
              SL_getSubcontextOldestMTimeURLIOStream,
              SL_getSubcontextOldestMTimeURLIOStream,
              SL_FlushAggregateData,
              FXD_URL );

        SLEA( "subcontext-oldest-ctime-url",
              SL_getSubcontextOldestCTimeURLIOStream,
              SL_getSubcontextOldestCTimeURLIOStream,
              SL_FlushAggregateData,
              FXD_URL );

        SLEA( "subcontext-oldest-atime-url",
              SL_getSubcontextOldestATimeURLIOStream,
              SL_getSubcontextOldestATimeURLIOStream,
              SL_FlushAggregateData,
              FXD_URL );
        
        /*******************************************************************************/
        /*******************************************************************************/
        /*******************************************************************************/
        
        SLEA( "recursive-subcontext-hardlink-count",
              SL_getRecursiveSubcontextHardLinkCountIOStream,
              SL_getRecursiveSubcontextHardLinkCountIOStream,
              SL_FlushAggregateData, XSD_BASIC_INT );

        SLEA( "recursive-subcontext-size",
              SL_getRecursiveSubcontextSizeIOStream,
              SL_getRecursiveSubcontextSizeIOStream,
              SL_FlushAggregateData, FXD_FILESIZE );

        SLEA( "recursive-subcontext-size-in-blocks",
              SL_getRecursiveSubcontextSizeInBlocksIOStream,
              SL_getRecursiveSubcontextSizeInBlocksIOStream,
              SL_FlushAggregateData, XSD_BASIC_INT );
        
        SLEA( "recursive-subcontext-file-count",
              SL_getRecursiveSubcontextFileCountIOStream,
              SL_getRecursiveSubcontextFileCountIOStream,
              SL_FlushAggregateData, XSD_BASIC_INT );

        SLEA( "recursive-subcontext-dir-count",
              SL_getRecursiveSubcontextDirCountIOStream,
              SL_getRecursiveSubcontextDirCountIOStream,
              SL_FlushAggregateData, XSD_BASIC_INT );

        SLEA( "recursive-subcontext-oldest-mtime",
              SL_getRecursiveSubcontextOldestMTimeRawIOStream,
              SL_getRecursiveSubcontextOldestMTimeRawIOStream,
              SL_FlushAggregateData, FXD_UNIXEPOCH_T );

        SLEA( "recursive-subcontext-oldest-ctime",
              SL_getRecursiveSubcontextOldestCTimeRawIOStream,
              SL_getRecursiveSubcontextOldestCTimeRawIOStream,
              SL_FlushAggregateData, FXD_UNIXEPOCH_T );

        SLEA( "recursive-subcontext-oldest-atime",
              SL_getRecursiveSubcontextOldestATimeRawIOStream,
              SL_getRecursiveSubcontextOldestATimeRawIOStream,
              SL_FlushAggregateData, FXD_UNIXEPOCH_T );

        SLEA( "recursive-subcontext-oldest-mtime-url",
                                  SL_getRecursiveSubcontextOldestMTimeURLIOStream,
                                  SL_getRecursiveSubcontextOldestMTimeURLIOStream,
                                  SL_FlushAggregateData, FXD_URL );

        SLEA( "recursive-subcontext-oldest-ctime-url",
                                  SL_getRecursiveSubcontextOldestCTimeURLIOStream,
                                  SL_getRecursiveSubcontextOldestCTimeURLIOStream,
                                  SL_FlushAggregateData, FXD_URL );

        SLEA( "recursive-subcontext-oldest-atime-url",
                                  SL_getRecursiveSubcontextOldestATimeURLIOStream,
                                  SL_getRecursiveSubcontextOldestATimeURLIOStream,
                                  SL_FlushAggregateData, FXD_URL );

        /************************************************************/
        /************************************************************/
        /************************************************************/

        SLEA( "fs-type",                  SL_getFSType,           FXD_FSID_T );
        SLEA( "fs-block-size",            SL_getFSBlockSize,      FXD_LONG );
        SLEA( "fs-block-count",           SL_getFSBlockCount,     FXD_LONG );
        SLEA( "fs-free-block-count",      SL_getFSFreeBlockCount, FXD_LONG );
        SLEA( "fs-free-size",             SL_getFSFreeSize,       FXD_LONG );
        SLEA( "fs-available-block-count", SL_getFSAvailableBlockCount, FXD_LONG );
        SLEA( "fs-file-nodes-total",      SL_getFSFileNodesTotal, FXD_LONG );
        SLEA( "fs-file-nodes-free",       SL_getFSFileNodesFree,  FXD_LONG );
        SLEA( "fs-id",                    SL_getFSID,             FXD_FSID_T );
        SLEA( "fs-file-name-length-maximum", SL_getFSMaxNameLength, FXD_LONG );
        SLEA( "fs-name",                  SL_getFSName,           XSD_BASIC_STRING );

        /************************************************************/
        /************************************************************/
        /************************************************************/

#ifdef HAVE_LIBRPM
        SLEA( "rpm-verify-size",   &_Self::SL_getRPMVerifySize,    XSD_BASIC_BOOL );
        SLEA( "rpm-verify-mode",   &_Self::SL_getRPMVerifyMode,    XSD_BASIC_BOOL );
        SLEA( "rpm-verify-md5",    &_Self::SL_getRPMVerifyMD5,     XSD_BASIC_BOOL );
        SLEA( "rpm-verify-device", &_Self::SL_getRPMVerifyDevice,  XSD_BASIC_BOOL );
        SLEA( "rpm-verify-owner",  &_Self::SL_getRPMVerifyOwner,   XSD_BASIC_BOOL );
        SLEA( "rpm-verify-group",  &_Self::SL_getRPMVerifyGroup,   XSD_BASIC_BOOL );
        SLEA( "rpm-verify-mtime",  &_Self::SL_getRPMVerifyMTime,   XSD_BASIC_BOOL );

        SLEA( "rpm-is-config",     &_Self::SL_getRPMIsConfig,   XSD_BASIC_BOOL );
        SLEA( "rpm-is-doc",        &_Self::SL_getRPMIsDoc,      XSD_BASIC_BOOL );
        SLEA( "rpm-is-ghost",      &_Self::SL_getRPMIsGhost,    XSD_BASIC_BOOL );
        SLEA( "rpm-is-license",    &_Self::SL_getRPMIsLicense,  XSD_BASIC_BOOL );
        SLEA( "rpm-is-pubkey",     &_Self::SL_getRPMIsPubkey,   XSD_BASIC_BOOL );
        SLEA( "rpm-is-readme",     &_Self::SL_getRPMIsReadme,   XSD_BASIC_BOOL );

        SLEA( "rpm-package",       &_Self::SL_getRPMPackage,    XSD_BASIC_STRING );
        SLEA( "rpm-version",       &_Self::SL_getRPMVersion,    XSD_BASIC_STRING );
        SLEA( "rpm-release",       &_Self::SL_getRPMRelease,    XSD_BASIC_STRING );
        SLEA( "rpm-info-url",      &_Self::SL_getRPMInfoURL,    FXD_URL_IMPLICIT_RESOLVE );

        SLEA( "rpm-vendor",        &_Self::SL_getRPMVendor,        XSD_BASIC_STRING );
        SLEA( "rpm-distribution",  &_Self::SL_getRPMDistribution,  XSD_BASIC_STRING );
        SLEA( "rpm-license",       &_Self::SL_getRPMLicense,       XSD_BASIC_STRING );
        SLEA( "rpm-packager",      &_Self::SL_getRPMPackager,      XSD_BASIC_STRING );
        SLEA( "rpm-group",         &_Self::SL_getRPMGroup,         XSD_BASIC_STRING );
        SLEA( "rpm-buildtime",     &_Self::SL_getRPMBuildtime,     FXD_UNIXEPOCH_T );
        SLEA( "rpm-summary",       &_Self::SL_getRPMSummary,       XSD_BASIC_STRING );
        
#endif // HAVE_LIBRPM

        /************************************************************/
        /************************************************************/
        /************************************************************/

        SLEA( "depth-per-color",
              EA_Atom_ReadOnly::GetIStream_Func_t(
                  &Context::imageEAGenerator_getDepthPerColorStream ),
              XSD_BASIC_INT );
        SLEA( "depth",
              EA_Atom_ReadOnly::GetIStream_Func_t(
                  &Context::imageEAGenerator_getDepthStream ),
              XSD_BASIC_INT );
        SLEA( "gamma",
              EA_Atom_ReadOnly::GetIStream_Func_t(
                  &Context::imageEAGenerator_getGammaStream ),
              XSD_BASIC_DOUBLE );
        SLEA( "has-alpha",
              EA_Atom_ReadOnly::GetIStream_Func_t(
                  &Context::imageEAGenerator_getHasAlphaStream ),
              XSD_BASIC_BOOL );
        SLEA( "aspect-ratio",
              EA_Atom_ReadOnly::GetIStream_Func_t(
                  &Context::imageEAGenerator_getAspectRatioStream ),
              XSD_BASIC_DOUBLE );
            
        SLEA( "width",
              EA_Atom_ReadOnly::GetIStream_Func_t(
                  &Context::imageEAGenerator_getWidthStream ),
              EA_Atom_ReadWrite::GetIOStream_Func_t(
                  &Context::imageEAGenerator_getWidthIOStream ),
              EA_Atom_ReadWrite::IOStreamClosed_Func_t(
                  &Context::imageEAGenerator_updateWidthFromStream ),
              FXD_WIDTH_PIXELS );

        SLEA( "height",
              EA_Atom_ReadOnly::GetIStream_Func_t(
                  &Context::imageEAGenerator_getHeightStream ),
              EA_Atom_ReadWrite::GetIOStream_Func_t(
                  &Context::imageEAGenerator_getHeightIOStream ),
              EA_Atom_ReadWrite::IOStreamClosed_Func_t(
                  &Context::imageEAGenerator_updateHeightFromStream ),
              FXD_HEIGHT_PIXELS );
        SLEA( "rgba-32bpp",
              EA_Atom_ReadOnly::GetIStream_Func_t(
                  &Context::imageEAGenerator_getRGBAStream ),
              EA_Atom_ReadWrite::GetIOStream_Func_t(
                  &Context::imageEAGenerator_getRGBAIOStream ),
              EA_Atom_ReadWrite::IOStreamClosed_Func_t(
                  &Context::imageEAGenerator_updateFromStream ),
              FXD_BINARY_RGBA32 );


#ifdef HAVE_LIBSELINUX
        
        SLEA( "selinux-identity", SL_getSELinuxIdentity<FollowLinks>, XSD_BASIC_STRING );
        SLEA( "selinux-type",
              SL_getSELinuxType<FollowLinks>,
              SL_getSELinuxType<FollowLinks>,
              SL_setSELinuxType<FollowLinks>,
              XSD_BASIC_STRING );
        SLEA( "dontfollow-selinux-type",
              SL_getSELinuxType<FollowLinks>,
              SL_getSELinuxType<FollowLinks>,
              SL_setSELinuxType<FollowLinks>,
              XSD_BASIC_STRING );
        SLEA( "selinux-context",
              SL_getSELinuxContext<FollowLinks>,
              SL_getSELinuxContext<FollowLinks>,
              SL_setSELinuxContext<FollowLinks>,
              XSD_BASIC_STRING );
        SLEA( "dontfollow-selinux-context",  
              SL_getSELinuxContext<DontFollowLinks>,
              SL_getSELinuxContext<DontFollowLinks>,
              SL_setSELinuxContext<DontFollowLinks>,
              XSD_BASIC_STRING );
        
#endif

        SLEA( "episode",          SL_getEpisode<DontFollowLinks>, XSD_BASIC_INT );
        SLEA( "series",           SL_getSeries <DontFollowLinks>, XSD_BASIC_STRING );
        SLEA( "crc32-expected",   SL_getExpectedCRC32 <DontFollowLinks>, XSD_BASIC_STRING );
        SLEA( "crc32-is-valid",   SL_getCRC32IsValid <DontFollowLinks>, XSD_BASIC_STRING );

            
        
        /************************************************************/
        /************************************************************/
        /************************************************************/
        
        _Base::createStateLessAttributes( true );
        supplementStateLessAttributes( true );
    }
}

    static string ferris_readlinkat( int dirfd, const std::string& path )
    {
        stringstream ret;

        size_t bufsiz = PATH_MAX;
        char* buf = (char*)malloc( bufsiz+1 );

//        cerr << "ferris_readlinkat() path:" << path << endl;
        
        while( true )
        {
            errno = 0;
            
            int rc = readlinkat( dirfd, path.c_str(), buf, bufsiz );
            int eno = errno;

            if( errno == ENAMETOOLONG )
            {
                bufsiz *= 2;
                buf = (char*)realloc( buf, bufsiz+1 );
                continue;
            }

            if( rc == -1 )
            {
                fh_stringstream ss;
                ss << "Error calling readlink() for:" << path;
                free( buf );

                if( dirfd != AT_FDCWD )
                    close( dirfd );

                ThrowFromErrno( eno, tostr(ss), 0 );
            }

            buf[ rc ] = '\0';
            break;
        }

        ret << buf;
        free( buf );
        if( dirfd != AT_FDCWD )
            close( dirfd );

//        cerr << "ferris_readlinkat() ret:" << ret.str() << endl;
        
        return ret.str();
    }


    static string ferris_readlink( const std::string& path )
    {
        stringstream ret;

        size_t bufsiz = PATH_MAX;
        char* buf = (char*)malloc( bufsiz+1 );

        while( true )
        {
            errno = 0;
            
            int rc = readlink( path.c_str(), buf, bufsiz );
            int eno = errno;

            if( errno == ENAMETOOLONG )
            {
                bufsiz *= 2;
                buf = (char*)realloc( buf, bufsiz+1 );
                continue;
            }

            if( rc == -1 )
            {
                fh_stringstream ss;
                ss << "Error calling readlink() for:" << path;
                free( buf );


                ThrowFromErrno( eno, tostr(ss), 0 );
            }

            buf[ rc ] = '\0';
            break;
        }

        ret << buf;
        free( buf );
        
        return ret.str();
    }
    

fh_istream
NativeContext::getLinkTargetRelative( Context* c, const std::string& rdn, EA_Atom* atom )
{
    string path = c->getDirPath();
    fh_stringstream ss;
#ifdef FERRIS_HAVE_READLINKAT
    ss << ferris_readlinkat( AT_FDCWD, path );
#else
    ss << ferris_readlink( path );
#endif
    return ss;
}

    

// fh_istream
// NativeContext::getLinkTargetRelative( Context* c, const std::string& rdn, EA_Atom* atom )
// {
//     string path = c->getDirPath();

//     string parentpath = "/";
//     if( c->isParentBound() )
//     {
//         parentpath = c->getParent()->getDirPath();
//     }

//     LG_NATIVE_D << "getLinkTarget() parent_path:" << parentpath << endl;

//     int parentfd = open( parentpath.c_str(), O_RDONLY );
//     if( parentfd == -1 )
//     {
//         int eno = errno;
//         fh_stringstream ss;
//         ss << "Error opening directory:" << parentpath << endl;
//         ThrowFromErrno( eno, tostr(ss), this );
//     }

//     fh_stringstream ss;
//     ss << ferris_readlinkat( parentfd, path );
//     return ss;
// }



void
NativeContext::priv_createAttributes()
{
    
    LG_NATIVE_D << "NativeContext::priv_createAttributes() path:" << getDirPath() << endl;

//    ensureUpdateMetaDataCalled();


    const struct stat& sb = getStat_DontFollow();
    if(S_ISLNK(sb.st_mode))
    {
        LG_NATIVE_D << "NativeContext::priv_createAttributes() path:" << getDirPath()
                    << " is a softlink"
                    << endl;
        addAttribute("link-target-relative", this,
                     &NativeContext::getLinkTargetRelative,
                     FXD_URL, true );
        supplementFerrisLinkTargetFromAbsolute();
    }
    
    Context::priv_createAttributes();

//    cerr << "\n\n-------------- NativeContext::createAttributes() --------------\n\n";
//    dumpAttrRefCounts();
}


long
NativeContext::priv_guessSize() throw()
{
    long ret = 0;

    if( DIR *d = opendir( getDirPath().c_str() ) )
    {
        while( struct dirent *de = readdir(d) )
        {
            ret++;
        }
        
        closedir(d);
    }
    return ret;
}




void 
NativeContext::setupFAM()
{
    LG_NATIVE_I << "About to get FAM to Monitor dir name:" << getDirName() << endl;
    LG_NATIVE_I << "About to get FAM to Monitor dir path:" << getDirPath() << endl;
    
    try
    {
        LG_NATIVE_D << "setupFAM() dir:" << getDirPath() << " FamReq:" << FamReq
//                  << " reqnum:" << FAMREQUEST_GETREQNUM( &FamReq->getRequest() )
                    << endl;
        if( FamReq )
        {
            LG_NATIVE_D << "have existing famreq... rc:" << FamReq->getReferenceCount() << endl;
            FamReq = 0;
        }
        
        FamReq = Fampp::MonitorDirectory( getDirPath() );

        LG_NATIVE_I << "fam request setup for dir path:" << getDirPath() << endl;
        
        typedef NativeContext NC;
        const FamReq_t& R = FamReq;

        
//        LG_NATIVE_D << "Registering change event for:" << getDirPath() << endl;
        R->getSig<Fampp::FamppChangedEvent>().connect(mem_fun( *this, &NC::OnFamppChangedEvent));
        R->getSig<Fampp::FamppDeletedEvent>().connect(mem_fun( *this, &NC::OnFamppDeletedEvent));
        R->getSig<Fampp::FamppStartExecutingEvent>().connect(mem_fun( *this, &NC::OnFamppStartExecutingEvent));
        R->getSig<Fampp::FamppStopExecutingEvent>().connect(mem_fun( *this, &NC::OnFamppStopExecutingEvent));
        R->getSig<Fampp::FamppCreatedEvent>().connect(mem_fun( *this, &NC::OnFamppCreatedEvent));
        R->getSig<Fampp::FamppMovedEvent>().connect(mem_fun( *this, &NC::OnFamppMovedEvent));
        R->getSig<Fampp::FamppAcknowledgeEvent>().connect(mem_fun( *this, &NC::OnFamppAcknowledgeEvent));
        R->getSig<Fampp::FamppExistsEvent>().connect(mem_fun( *this, &NC::OnFamppExistsEvent));
        R->getSig<Fampp::FamppEndExistEvent>().connect(mem_fun( *this, &NC::OnFamppEndExistEvent));

        
        LG_NATIVE_D << "end famreq... rc:" << FamReq->getReferenceCount() << endl;
        LG_NATIVE_D << "Setup fam...done." << endl;
        LG_NATIVE_I << "Setup fam...done." << endl;
    }
    catch( Fampp::FamppDirMonitorInitFailedException& e)
    {
        ostringstream ss;
        ss << "FamppDirMonitorInitFailedException for path:" << getDirPath();
        Throw_CanNotMonitorDirWithFAM( tostr(ss), this, e );
    }
}

string
NativeContext::OnFamppEvent( string fqfilename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev )
{
    LG_NATIVE_D << "NativeContext::famEv() DirName:" << getDirName() << endl;
    LG_NATIVE_D << "NativeContext::famEv() DirPath:" << getDirPath() << endl;
    LG_NATIVE_D <<"  fqfilename : " << fqfilename << endl;
    LG_NATIVE_D << " code       : " << ev->getFAMCode() << endl;

    pair<string,string> split = splitPathAtEnd( fqfilename );

    string filename = split.second;
    if( !filename.length() )
        filename = split.first;

    return filename;
}


void
NativeContext::OnFamppChangedEvent( string fqfilename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev )
{
    string filename = OnFamppEvent( fqfilename, req, ev );
    LG_NATIVE_D << "NativeContext::OnFamppChangedEvent() fqfn:" << fqfilename << endl;
    
    // This is interesting. We can assume that subcontexts are
    // homogenous because with overmount we create a new root context
    // for the overmounted read.
    fh_context ctx = getSubContext( filename );

    LG_NATIVE_D << "NativeContext::OnFamppChangedEvent() fn:" << filename
                << " isbound:" << isBound(ctx)
                << endl;
    
    bumpVersion();
    if(NativeContext* nc = (NativeContext*)GetImpl(ctx))
    {
        LG_NATIVE_D << "NativeContext::OnFamppChangedEvent(nc) url:" << nc->getURL() << endl;
        nc->bumpVersion();
        nc->updateMetaData();
    }
    updateMetaData();
}

void
NativeContext::OnFamppDeletedEvent( string fqfilename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev )
{
    string filename = OnFamppEvent( fqfilename, req, ev );

    Remove( filename );
}

void
NativeContext::OnFamppStartExecutingEvent( string fqfilename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev )
{
    string filename = OnFamppEvent(fqfilename, req, ev );

    Emit_Start_Execute( 0, filename, filename, 0 );
}

void
NativeContext::OnFamppStopExecutingEvent( string fqfilename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev )
{
    string filename = OnFamppEvent( fqfilename, req, ev );

    Emit_Stop_Execute( 0, filename, filename, 0 );
}

void
NativeContext::OnFamppCreatedEvent( string fqfilename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev )
{
    string filename = OnFamppEvent( fqfilename, req, ev );
    LG_NATIVE_D << "NativeContext::OnFamppCreatedEvent() fn:" << filename << endl;
    native_readSubContext( filename, true );
}

void
NativeContext::OnFamppMovedEvent( string fqfilename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev )
{
    string filename = OnFamppEvent( fqfilename, req, ev );
    LG_NATIVE_D << "NativeContext::OnFamppMovedEvent() fqfilename:" << fqfilename << endl;
    Emit_Moved( 0, "", filename, 0 );
}

void
NativeContext::OnFamppAcknowledgeEvent( string fqfilename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev )
{
    string filename = OnFamppEvent( fqfilename, req, ev );


}

void
NativeContext::OnFamppExistsEvent( string fqfilename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev )
{
    string filename = OnFamppEvent( fqfilename, req, ev );

    LG_NATIVE_I << "FAMExists fqfilename:" <<  fqfilename << endl;
    LG_NATIVE_D << "FAMExists   filename:" <<    filename << endl;
    LG_NATIVE_D << "FAMExists getDirName():" << getDirName() << endl;
    LG_NATIVE_D << "FAMExists getDirPath():" << getDirPath() << endl;

    if( fqfilename == getDirPath() )
        return;

//     LG_NATIVE_D << "is-bound:" << priv_isSubContextBound( filename ) << endl;
//     LG_NATIVE_D << "items-size:" << getItems().size() << endl;
//     for( Items_t::iterator iter = getItems().begin(); iter != getItems().end(); ++iter )
//     {
//         fh_context c = *iter;
//         LG_NATIVE_D << "listing...c:" << c->getURL() << endl;
//     }
//     LG_NATIVE_D << "EOL" << endl;
    
    LG_NATIVE_I << "FAMExists   filename:" <<    filename << endl;
    fh_context childc = native_readSubContext( filename, false );
    if( NativeContext* nc = dynamic_cast<NativeContext*>( GetImpl( childc ) ) )
    {
        nc->IsDanglingLink = false;
    }
}

void
NativeContext::OnFamppEndExistEvent( string fqfilename, Fampp::fh_fampp_req req, Fampp::fh_fampp_ev ev )
{
	LG_NATIVE_I << "OnFamppEndExistEvent fqfilename:" << fqfilename << endl;

    string filename = OnFamppEvent( fqfilename, req, ev );

    LG_NATIVE_I << "__________________FAMEndExist________________________" << endl;
//  dumpOutItems();

    EnsureStopReadingIsFired();
    updateMetaData();

//     LG_NATIVE_D << "OnFamppEndExistEvent() NumberOfSubContexts:" << NumberOfSubContexts
//          << " sz:" << getItems().size()
//          << " url:" << getURL()
//          << endl;
    NumberOfSubContexts = getItems().size();


    try
    {
#ifdef PERMIT_FAM
        if( !FamppChangedEventConnected )
        {
            if( !FamReq )
            {
                FamReq = Fampp::MonitorDirectory( getDirPath() );
            }
                    
            typedef NativeContext NC;
            const FamReq_t& R = FamReq;
            R->getSig<Fampp::FamppChangedEvent>().connect(mem_fun( *this, &NC::OnFamppChangedEvent));
            FamppChangedEventConnected = true;
        }
#endif
    }
    catch( Fampp::FamppDirMonitorInitFailedException& e)
    {
        ostringstream ss;
        ss << "FamppDirMonitorInitFailedException for path:" << getDirPath();
        cerr << tostr(ss) << endl;
        Throw_CanNotMonitorDirWithFAM( tostr(ss), this, e );
    }
}

const struct stat&
NativeContext::getStat_Follow()
{
    LG_NATIVE_D << "NativeContext::getStat_Follow() this:" << (void*)this
                << " active:" << isActiveView()
                << " sb_follow_version:" << sb_follow_version
                << " getVersion():" << getVersion()
                << " url:" << getURL()
                << endl;
    LG_CTXREC_D << "getStat_Follow() " << endl;
    
    if( isActiveView() && sb_follow_version == getVersion() )
    {
        LG_NATIVE_D << "NativeContext::getStat_Follow(cache) "
                    << " IsDanglingLink:" << (bool)IsDanglingLink
                    << " sz:" << sb_follow.st_size
                    << " url:" << getURL()
                    << endl;
        if( IsDanglingLink )
            return sb_dont_follow;
        
        return sb_follow;
    }
    
//    LG_NATIVE_D << "NativeContext::getStat_Follow() path:" << getDirPath() << endl;
    int rv = stat( getDirPath().c_str(), &sb_follow );

    LG_NATIVE_D << "NativeContext::getStat_Follow() rv:" << rv
                << " sz:" << sb_follow.st_size
                << endl;

    if( rv == 0 )
    {
        IsDanglingLink = false;
    }
    if( rv != 0 )
    {
        int eno = errno;
        LG_NATIVE_W << "stat failed" << endl;
        LG_NATIVE_W << "name:" << getDirName() << endl;
        LG_NATIVE_W << "path:" << getDirPath() << endl;

        sb_follow_version = getVersion();
        IsDanglingLink = true;
        return sb_dont_follow;
//         fh_stringstream ss;
//         ss << "Failed to stat url:" << getURL();
//         LG_CTXREC_D << "getStat_Follow(e) e:" << tostr(ss) << endl;
//         ThrowFromErrno( eno, tostr(ss), this );
    }

    sb_follow_version = getVersion();
    return sb_follow;
}

const struct stat&
NativeContext::getStat_DontFollow()
{
    if( isActiveView() && sb_dont_follow_version == getVersion() )
    {
        return sb_dont_follow;
    }

    LG_NATIVE_D << "lstat() path:" << getDirPath() << endl;
    int rv = lstat( getDirPath().c_str(), &sb_dont_follow);

    if( rv != 0 )
    {
        int eno = errno;
        LG_NATIVE_W << "stat failed" << endl;
        LG_NATIVE_W << "name:" << getDirName() << endl;
        LG_NATIVE_W << "path:" << getDirPath() << endl;

        fh_stringstream ss;
        ss << "Failed to lstat url:" << getURL();
        ThrowFromErrno( eno, tostr(ss), this );
    }

    LG_NATIVE_D << "lstat() path:" << getDirPath()
                << " size:" << sb_dont_follow.st_size
                << endl;
    
    /* If the lstat() is not a link then we have the stat() data too */
    if( !S_ISLNK(sb_dont_follow.st_mode))
    {
        sb_follow         = sb_dont_follow;
        sb_follow_version = getVersion();
    }
    
    sb_dont_follow_version = getVersion();
    return sb_dont_follow;
}

void
NativeContext::updateMetaData()
{
    LG_NATIVE_D << "updateMetaData() path:" << getDirPath() << endl;
    getStat_DontFollow();
    getStat_Follow();
    
    Context::updateMetaData();
    LG_NATIVE_D << "updateMetaData(end) path:" << getDirPath() << endl;
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


#ifdef HAVE_LIBFILE

void
NativeContext__mimetype_cb( libfile_op* x )
{
    char* tmp=0;

    
    NativeContext* nc = (NativeContext*)libfile_get_udata(x);

    if( tmp = libfile_get_mimetype(x) )
    {
        nc->Mimetype = tmp;
    }
    else
    {
        nc->Mimetype = "";
        LG_NATIVE_D << " NativeContext__mimetype_cb() "
                    << " path:" << nc->getDirPath()
                    << " MIME RESOLUTION HAS FAILED"
                    << endl;
    }

    LG_NATIVE_D << "NativeContext__mimetype_cb() "
                << " path:" << nc->getDirPath()
                << " mimetype:" << nc->Mimetype
                << endl;
}

void
NativeContext__filetype_cb( libfile_op* x )
{
    char* tmp=0;
    NativeContext* nc = (NativeContext*)libfile_get_udata(x);

    if( tmp = libfile_get_filetype(x) )
    {
        nc->Filetype = tmp;
    }
    
}

static void
libfile_filename_cb( libfile_op* x )
{
//     char* inname = libfile_get_filename(x);
}

libfile_op*
NativeContext::getFileMimeClient()
{
    static libfile_op* mc = 0;

    if( !mc )
    {
        int argc = 1;
        char*const argv[] = { "-i" };

        if( !(mc = libfile_new()) )
        {
            LG_NATIVE_D << "NativeContext::getFileMimeClient() "
                        << " can not make a new libfile instance."
                        << " path:" << getDirPath()
                        << endl;
            return mc;
        }

        libfile_set_program_name( mc, "libferris.so" );
        int rv = libfile_set_params( mc, argc, argv );

        if( rv != LIBFILE_E_RETURN_SUCCESS )
        {
            LG_NATIVE_D << "NativeContext::getFileMimeClient() "
                        << " libfile_set_params() failed"
                        << " path:" << getDirPath()
                        << endl;
            return 0;
        }
        
        libfile_set_mimetype_callback( mc, NativeContext__mimetype_cb );
        libfile_set_filetype_callback( mc, NativeContext__mimetype_cb );
        libfile_set_udata( mc, this );
        libfile_set_filename_callback( mc, NativeContext__mimetype_cb );
    }

    libfile_set_udata( mc, this );
    return mc;
}

libfile_op*
NativeContext::getFileTypeClient()
{
    static libfile_op* mc = 0;

    if( !mc )
    {
        int argc = 1;
        char*const argv[] = { " " };

        if( !(mc = libfile_new()) )
        {
            LG_NATIVE_D << "NativeContext::getFileTypeClient() "
                        << " can not make a new libfile instance."
                        << " path:" << getDirPath()
                        << endl;
            return mc;
        }

        libfile_set_program_name( mc, "libferris.so" );
        int rv = libfile_set_params( mc, argc, argv );

        if( rv != LIBFILE_E_RETURN_SUCCESS )
        {
            LG_NATIVE_D << "NativeContext::getFileTypeClient() "
                        << " libfile_set_params() failed"
                        << " path:" << getDirPath()
                        << endl;
            return 0;
        }
        
        libfile_set_mimetype_callback( mc, NativeContext__filetype_cb );
        libfile_set_filetype_callback( mc, NativeContext__filetype_cb );
        libfile_set_udata( mc, this );
        libfile_set_filename_callback( mc, NativeContext__filetype_cb );
    }

    libfile_set_udata( mc, this );
    return mc;
}


// void
// NativeContext::setupLibFile(
//     libfile_op*& x,
//     int argc, char*const argv[],
//     void (*cb)(libfile_op*)
//     )
// {
//     int rv = 0;

//     if(x)
//     {
//         libfile_set_udata( x, this );
//         return;
//     }
    
//     if( !(x = libfile_new()) )
//     {
//         LG_NATIVE_D << "NativeContext::setupLibFile() can not make a new libfile instance."
//                     << " path:" << getDirPath()
//                     << endl;
//         return;
//     }

//     libfile_set_program_name( x, "libferris.so" );
//     rv = libfile_set_params( x, argc, argv );

//     libfile_set_mimetype_callback( x, cb );
//     libfile_set_filetype_callback( x, cb );
//     libfile_set_udata( x, this );
//     libfile_set_filename_callback( x, libfile_filename_cb );
// }



// void
// NativeContext::setupLibFile(native_libfile_detection_bits b)
// {
//     LG_NATIVE_D << "NativeContext::setupLibFile()" << endl;

//     if( b & NATIVE_LIBFILE_MIME )
//     {
//         int argc = 1;
//         char*const argv[] = { "-i" };
//         setupLibFile( libfile_mime_client, argc, argv, NativeContext__mimetype_cb );
//         libfile_mime_Version = 0;
//     }
//     if( b & NATIVE_LIBFILE_TYPE )
//     {
//         int argc = 1;
//         char*const argv[] = { " " };
//         setupLibFile( libfile_type_client, argc, argv, NativeContext__filetype_cb );
//         libfile_type_Version = 0;
//     }
// }


void
NativeContext::ensureMimeAndFileTypeUpToDate( native_libfile_detection_bits b )
{
    int rv=LIBFILE_E_RETURN_SUCCESS;

    LG_NATIVE_D << "NativeContext::ensureMimeAndFileTypeUpToDate()" << endl;
//    LG_NATIVE_D << "ensureMimeAndFileTypeUpToDate() libfile_Version:" << libfile_Version << endl;
    LG_NATIVE_D << "ensureMimeAndFileTypeUpToDate()      getVersion:" << getVersion() << endl;
    
    if( b & NATIVE_LIBFILE_MIME && libfile_mime_Version < getVersion())
    {
        if( libfile_op* mc = getFileMimeClient() )
        {
            libfile_clear_sources( mc );
            libfile_append_source( mc, getDirPath().c_str() );
//            cerr << "NativeContext::ensureMimeAndFileTypeUpToDate() url:" << getURL() << endl;
            rv = libfile_perform( mc );
            libfile_mime_Version = getVersion();
            LG_NATIVE_D << "ensureMimeAndFileTypeUpToDate() rv:" << rv << endl;
        }
    }
        
    if( b & NATIVE_LIBFILE_TYPE && libfile_type_Version < getVersion())
    {
        if( libfile_op* mc = getFileTypeClient() )
        {
            libfile_clear_sources( mc );
            libfile_append_source( mc, getDirPath().c_str() );
            rv = libfile_perform( mc );
            libfile_type_Version = getVersion();
        }
    }

    if( !Mimetype.length() || Mimetype == "data" )
    {
        if( ends_with( getDirName(), ".png" ))
        {
            Mimetype = "image/png";
        }
        if( ends_with( getDirName(), ".mp3" ))
        {
            Mimetype = "audio/mp3";
        }
        if( ends_with( getDirName(), ".ogg" ))
        {
            Mimetype = "audio/ogg";
        }

        string path = getDirPath();
        if( string::npos != path.find( FERRIS_CONFIG_APPS_DIR ))
        {
            Mimetype = "ferris/application";
        }
        if( string::npos != path.find( FERRIS_CONFIG_EVENT_DIR ))
        {
            Mimetype = "ferris/event";
        }
        if( string::npos != path.find( FERRIS_CONFIG_MIMEBIND_DIR ))
        {
            Mimetype = "ferris/mime";
        }
    }

    if( ends_with( getDirName(), ".avi" ))
    {
        Mimetype = "video/avi";
    }
    if( ends_with( getDirName(), ".mpg" ))
    {
        Mimetype = "video/mpg";
    }
    if( ends_with( getDirName(), ".mpeg" ))
    {
        Mimetype = "video/mpg";
    }
    if( ends_with( getDirName(), ".desktop" ))
    {
        Mimetype = "text/application-metadata";
    }
    if( ends_with( getDirName(), ".db" ))
    {
        Mimetype = "application/db4";
    }
    if( ends_with( getDirName(), ".mov" ))
    {
        Mimetype = "video/quicktime";
    }


    if( Mimetype == "video/mpeg" )
    {
        Mimetype = "video/mpg";
    }
    

    // Handled in Ferris.cpp now.
//     string path = getDirPath();

//     if( string::npos != path.find( "/.ego/desktop" ))
//     {
//         if( isParentBound() && getParent()->getDirName() == "desktop" )
//             if( SL_getIsDir<FollowLinks>( this,"",0 ) )
//             {
//                 Mimetype = "desktop/directory";
//             }
//     }
}

string
NativeContext::priv_getMimeType( bool fromContent )
{
    const struct stat& sb = FollowLinks::getStat(this);
    ensureMimeAndFileTypeUpToDate(NativeContext::NATIVE_LIBFILE_MIME);
    return Mimetype;
}

string
NativeContext::getFileType()
{
    const struct stat& sb = FollowLinks::getStat(this);
    ensureMimeAndFileTypeUpToDate(NativeContext::NATIVE_LIBFILE_TYPE);
    return Filetype;
}
#endif


#ifdef HAVE_EFSD

GMainLoop*      NativeContext::m_efsdGML            = 0;
EfsdConnection* NativeContext::m_efsdConnection     = 0;
GIOChannel*     NativeContext::m_efsdChannel        = 0;
bool            NativeContext::m_waitingForMimetype = false;
bool            NativeContext::m_efsdTimedOut       = false;

static gboolean
mimeCreated_cb( GIOChannel *source,
                GIOCondition condition,
                gpointer user_data )
{
    EfsdCmdId id;
    EfsdEvent ee;

    NativeContext* c = (NativeContext*)user_data;
    GMainLoop* gml    = c->m_efsdGML;
    c->m_waitingForMimetype = false;

    while( efsd_events_pending(c->m_efsdConnection) )
    {
        if( efsd_next_event( c->m_efsdConnection, &ee) >= 0)
        {
            switch (ee.type)
            {
            case EFSD_EVENT_REPLY:
                switch (ee.efsd_reply_event.command.type)
                {
                case EFSD_CMD_GETFILETYPE:
                    if (ee.efsd_reply_event.errorcode == 0)
                    {
//                         printf("Gettype event %i on %s\n", 
//                                ee.efsd_reply_event.command.efsd_file_cmd.id,
//                                ee.efsd_reply_event.command.efsd_file_cmd.files[0]);		 
//                         if (ee.efsd_reply_event.errorcode == 0)
//                         {
//                             printf("filetype is %s\n", (char*)ee.efsd_reply_event.data);
//                         }
                        
//                     printf("Filetype of file %s is %s\n",
//                            efsd_reply_filename(&ee),
//                            (char*)ee.efsd_reply_event.data);
                        c->Mimetype = (char*)ee.efsd_reply_event.data;
                    }
                    break;
                }
            }
            efsd_event_cleanup(&ee);
        }
    }
    g_main_loop_quit( gml );
    return 0;
}
static gboolean mimeCreated_to( gpointer user_data )
{
    NativeContext* c = (NativeContext*)user_data;
    GMainLoop* gml    = c->m_efsdGML;

    c->m_efsdTimedOut;
    LG_NATIVE_W << "Timeout waiting for efsd to generate mime type. url:" << c->getURL() << endl;
    g_main_loop_quit( gml );
    return 0;
}

void
NativeContext::openEFSDConnection()
{
    if( !m_efsdConnection && !(m_efsdConnection = efsd_open()))
    {
        /* Oops. Couldn't establish connection.
         * Is Efsd really running ?
         */
        static Util::SingleShot virgin;
        if( virgin() )
        {
            /* only warn once */
            LG_NATIVE_ER << "Can not open connection to efsd." << endl;
        }
        return;
    }
    int efsdfd    = efsd_get_connection_fd( m_efsdConnection );
    m_efsdChannel = g_io_channel_unix_new( efsdfd );
}

string
NativeContext::priv_getMimeType( bool fromContent )
{
//     const struct stat& sb = FollowLinks::getStat(this);
//     ensureMimeAndFileTypeUpToDate(NativeContext::NATIVE_LIBFILE_MIME);
//     return Mimetype;

    EfsdCmdId id;
    EfsdEvent ee;

    Mimetype = "document/unknown";

    openEFSDConnection();
    if( !m_efsdConnection )
        return Mimetype;

    
    // I found some files that would cause no result to ever be sent back
    // so rather than hanging forever, we give efsd a one second timeout to
    // get back to us, if it doesn't then we move on.
    GIOCondition cond = GIOCondition(G_IO_IN | G_IO_ERR | G_IO_PRI);
    guint srcID   = g_io_add_watch( m_efsdChannel, cond, mimeCreated_cb, this );

    m_waitingForMimetype = true;
    m_efsdTimedOut       = false;


    /* Request the filetype: */

    if ( (id = efsd_get_filetype( m_efsdConnection, (char*)getDirPath().c_str() )) < 0)
    {
        /* Could not send command. Continue accordingly. */
        LG_NATIVE_W << "Can not send efsd command to obtain the mime type of context:"
                    << getURL() << endl;
        return Mimetype;
    }

    
    {
        GMainContext* gmc = g_main_context_default();
        GMainLoop* gml    = m_efsdGML = g_main_loop_new( gmc, 0 );
        
        g_timeout_add( 1500, GSourceFunc(mimeCreated_to), this );
        g_main_loop_run( gml );
        g_source_remove( srcID );
//        g_main_loop_unref( gml );
        g_main_destroy( gml );
    }
    
    
    // Churn over this efsd.
    if( m_efsdTimedOut )
    {
        if( m_efsdConnection )
            efsd_close( m_efsdConnection );
        m_efsdConnection = 0;
        openEFSDConnection();
    }
    
 
    //
    // Clean up the extended mimetype to just two levels 
    //
    int slash = Mimetype.find( '/' );
    if( slash != string::npos )
    {
        slash = Mimetype.find( '/', slash+1 );
        if( slash != string::npos )
        {
            Mimetype = Mimetype.substr( 0, slash );
        }
    }
    
    return Mimetype;
}
#endif
#ifdef HAVE_KDE3

std::string
NativeContext::priv_getMimeType( bool fromContent )
{
    Mimetype = "document/unknown";

//  	KMimeType::Ptr type = KMimeType::findByURL( getDirPath().c_str() );
//     Mimetype = type->name();
// 	if (type->name() == KMimeType::defaultMimeType())
//         cerr << "Could not find out type" << endl;
// 	else
//         cerr << "Type: " << type->name() << endl;

//     KMimeMagicResult* result = KMimeMagic::self()->findFileType( getDirPath().c_str() );
//     Mimetype = result->mimeType();
//     return Mimetype;

    return KDE::getMimeType( this, fromContent );
}

#endif



// fh_istream
// NativeContext::getMimetypeStream( Attribute* attr )
// {
//     fh_stringstream ss;
//     ensureMimeAndFileTypeUpToDate(NATIVE_LIBFILE_MIME);
//     ss << Mimetype;
//     return ss;
// }

// fh_istream
// NativeContext::getFiletypeStream( Attribute* attr )
// {
//     fh_stringstream ss;
//     ensureMimeAndFileTypeUpToDate(NATIVE_LIBFILE_TYPE);
//     ss << Filetype;
//     return ss;
// }


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


bool
NativeContext::supportsReClaim()
{
    return true;
}

bool
NativeContext::supportsMonitoring()
{
    return true;
}

bool
NativeContext::supportsRename()
{
    return true;
}

bool
NativeContext::supportsRemove()
{
    return true;
}

void
NativeContext::priv_remove( fh_context c )
{
    LG_NATIVE_D << "Native::remove() path:" << c->getDirPath() << endl;

    c->ensureUpdateMetaDataCalled();

    int rc = ::remove( c->getDirPath().c_str() );
    if( rc != 0 )
    {
        string es = errnum_to_string( "", errno );
        fh_stringstream ss;
        ss << "Native context can not remove object: " << c->getURL()
           << " reason:" << es 
           << endl;
        Throw_CanNotDelete( tostr(ss), GetImpl(c) );
    }

    ProcessAllFamppEvents();
}

fh_context
NativeContext::priv_rename( const std::string& rdn,
                            const std::string& newPathRelative,
                            bool TryToCopyOverFileSystems,
                            bool OverWriteDstIfExists )
{
    string oldPath = appendToPath( getDirPath(), rdn );
    string newPath = appendToPath( getDirPath(), newPathRelative, true );
    
    LG_NATIVE_D << "NativeContext::priv_rename() url:" << getURL()
                << " rdn:" << rdn
                << " oldPath:" << oldPath
                << " newPath:" << newPath
                << " TryToCopyOverFileSystems:" << TryToCopyOverFileSystems
                << " OverWriteDstIfExists:" << OverWriteDstIfExists
                << endl;

    
    if( OverWriteDstIfExists )
    {
        try
        {
            fh_context c = Resolve( newPath );
        }
        catch( exception& e )
        {
            int rc = ::remove( newPath.c_str() );
            if( rc != 0 && errno != ENOENT )
            {
                int eno = errno;
                string es = errnum_to_string( "", errno );
                LG_NATIVE_D << "priv_rename() rc:" << rc << " e#:" << eno << endl;
                fh_stringstream ss;
                ss << "Native context can not remove object: " << newPath
                   << " to make way for rename of:" << oldPath
                   << " reason:" << es 
                   << endl;
                Throw_CanNotDelete( tostr(ss), this );
            }
        }
    }

    if( starts_with( newPath, "file:" ) )
        newPath = newPath.substr( 5 );

    LG_NATIVE_D << "NativeContext::priv_rename() Calling system rename function"
                << " old:" << oldPath
                << " new:" << newPath
                << endl;
    
    if( !::rename( oldPath.c_str(), newPath.c_str() ) )
    {
        /* ok */
        LG_NATIVE_D << "NativeContext::priv_rename(ok)"
                    << " old:" << oldPath
                    << " new:" << newPath
                    << endl;
        
        if( !isActiveView() )
        {
            LG_NATIVE_D << "NativeContext::priv_rename(not active view)"
                        << " old:" << oldPath
                        << " new:" << newPath
                        << endl;
            
//            cerr << "passive view. rdn:" << rdn << " newPathRelative:" << newPathRelative << endl;
            return contextHasBeenRenamed( rdn, newPathRelative );
        }

        LG_NATIVE_D << "NativeContext::priv_rename(active view, updating with process-all-events) "
                    << " old:" << oldPath
                    << " new:" << newPath
                    << endl;

//         // Try to force fam/gamin to update the new directory
//         try
//         {
//             fh_context parent_newc = Resolve( newPath, RESOLVE_PARENT );
//             parent_newc->SubContextNamesCacheIsValid = false;
//             parent_newc->bumpVersion();
//         }
//         catch( exception& e )
//         {
//         }
        
//        cerr << "active view at url:" << getURL() << " updating internal ferris state." << endl;

        //
        // This is a charming race condition. Sometimes the kernel itself will not
        // have the rename() effects in the fam/gamin queue for us at this stage.
        // what can we do?
        //
        double RaceSleepTimeLeft = 30;
        double RaceSleepTimeDropSize = 0.2;
        while( RaceSleepTimeLeft )
        {
            ProcessAllFamppEvents();

            try
            {
                LG_NATIVE_D << "NativeContext::priv_rename(fam/gamin queue) "
                            << " RaceSleepTimeLeft:" << RaceSleepTimeLeft 
                            << " old:" << oldPath
                            << " new:" << newPath
                            << endl;
                fh_context ret = Resolve( newPath );
                break;
            }
            catch( exception& e )
            {}

            LG_NATIVE_D << "NativeContext::priv_rename(sleeping) "
                        << " RaceSleepTimeLeft:" << RaceSleepTimeLeft 
                        << " old:" << oldPath
                        << " new:" << newPath
                        << endl;
            RaceSleepTimeLeft -= RaceSleepTimeDropSize;
            Time::Sleep( RaceSleepTimeDropSize );
        }
        

        LG_NATIVE_D << "NativeContext::priv_rename(getting ret) "
                    << " old:" << oldPath
                    << " new:" << newPath
                    << endl;
        fh_context ret = Resolve( newPath );
        LG_NATIVE_D << "NativeContext::priv_rename(returning) "
                    << " old:" << oldPath
                    << " new:" << newPath
                    << endl;
        return ret;
    }
    int e = errno;
    
    /* failed */
    fh_stringstream ss;
    ss << "Rename attempt failed. URL:" << getURL() << " src:" << rdn << " dst:" << newPath;
    Throw_RenameFailed( errnum_to_string( tostr(ss), e ), this );
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void
NativeContext::imageEAGenerator_priv_createAttributes()
{
}


    time_t
    NativeContext::getMTime()
    {
        if( getDirPath().empty() )
            return 0;

        try
        {
//            cerr << "NativeContext::getMTime().. path:" << getDirPath() << endl;
            const struct stat& sb = getStat_DontFollow();
            return sb.st_mtime;
        }
        catch( exception& e )
        {
            return 0;
        }
    }
    


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void
NativeContext::bumpVersion()
{
    _Base::bumpVersion();
}


};

