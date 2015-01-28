/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris
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

    $Id: Shell.cpp,v 1.16 2010/09/24 21:30:59 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <config.h>

#include "Shell.hh"

#include <Enamel.hh>
#include <Resolver.hh>
#include <Resolver_private.hh>
#include <Ferris.hh>
#include <Ferris_private.hh>
#include <Trimming.hh>

//#include "Ferrisls.hh"
#include "SignalStreams.hh"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <utime.h>

using namespace std;

namespace Ferris
{
    static bool priv_isTransitiveParent( fh_context base, fh_context p )
    {
        if( base->isParentBound() )
        {
            fh_context basep = base->getParent();
            if( basep == p )
                return true;
            return priv_isTransitiveParent( basep, p );
        }
        return false;
    }
    
    bool isTransitiveParent( fh_context base, fh_context p )
    {
        if( base == p || !base || !p )
            return false;
        return priv_isTransitiveParent( base, p );
    }

    static bool running_set_UID = false;
    
    void setRunningSetUID( bool v )
    {
        running_set_UID = v;
    }
    
    bool runningSetUID()
    {
        return running_set_UID;
    }

        bool canResolve( const std::string& s )
        {
            if( starts_with( s, "file:" ) )
            {
                string subs = s.substr( strlen( "file:" ) );
                struct stat buf;
                int rc = lstat( subs.c_str(), &buf );
                return !rc;
            }
            try
            {
                Resolve( s );
                return 1;
            }
            catch( ... )
            {
                return 0;
            }
        }

   std::string canonicalizeMissing( const std::string& earl )
   {
      fh_context rootc = Resolve("/");
      
      Context::SplitPath_t sp = rootc->splitPath( earl );
      Context::SplitPath_t col;
      for( Context::SplitPath_t::iterator si = sp.begin();
           si != sp.end(); ++si )
      {
         string s = *si;
         if( s == "." )
            continue;
         if( s == ".." )
         {
            col.pop_back();
            continue;
         }
         col.push_back(s);
      }
      std::string ret = rootc->unSplitPath( col );
      return ret;
   }
   
   
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    
    /**
     * Save the data from byteContent into the context at rdn with the parent
     * parentURL.
     *
     * One can control if the context is created if it does not exist by setting
     * overwrite=true. If one wants to create a new file then set shouldMonsterName=true
     * and overwrite=false, which will not overwrite existing data and create a unique
     * file with a prefix of the given rdn.
     *
     * If shouldMonsterName=false and overwrite=false then if a context exists with
     * the desired rdn an exception will be thrown.
     *
     * @parentURL Parent of the desired context
     * @rdn_raw Rdn of where to store the data in parentURL
     * @byteContent The data to save
     * @shouldMonsterName Keep chaning the rdn if objects already exist with the
     *                    given rdn
     * @overwrite Overwrite data in existing contexts without prompting
     *
     */
    fh_context saveFile( const std::string& parentURL,
                         const std::string& rdn_raw,
                         const std::string& byteContent,
                         bool shouldMonsterName,
                         bool overwrite ) 
    {
        fh_context parent = Shell::acquireContext( parentURL );
        string rdn        = rdn_raw;
        fh_context c;
        
        if( shouldMonsterName )
        {
            rdn = monsterName( parent, rdn );
        }

        if( parent->isSubContextBound( rdn ) )
        {
            if( overwrite )
            {
                c = parent->getSubContext( rdn );
            }
            else
            {
                fh_stringstream ss;
                ss << "context exists and told not to overwrite existing\n"
                   << "for:" << rdn;
                Throw_ObjectExists( tostr(ss), 0 );
            }
        }
        else
        {
            c = Shell::CreateFile( parent, rdn );
        }
        
        fh_iostream oss = c->getIOStream( ios::trunc );
        oss << byteContent;
        return c;
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    namespace Shell
    {
        ContextCreated_Sig_t& getNullContextCreated_Sig()
        {
            static ContextCreated_Sig_t x;
            return x;
        }
        

        
        std::string quote( const std::string& s )
        {
            static StringQuote sq( StringQuote::SHELL_QUOTING );
            return sq(s);
        }

        fh_context unrollLinks( fh_context c,
                                bool throwForException,
                                int levelOfRecursion )
        {
            string realpath = getStrAttr( c, "realpath", "" );
//             cerr << "unrollLinks() c:" << c->getURL()
//                  << " realpath:" << realpath
//                  << endl;
            
            if( realpath.empty() )
                return c;

            try
            {
                fh_context nextc = Resolve( realpath );
                if( nextc->getURL() == c->getURL() )
                    return c;
                return unrollLinks( nextc, levelOfRecursion - 1 );
            }
            catch( exception& e )
            {
                if( !throwForException )
                    return c;
                
                throw;
            }
        }
            

        string getCWDString()
        {
            return getCWDDirPath();
//             fh_char c_dir(getcwd(0,0)); // FIXME: Linux only.
//             const string& dir = tostr(c_dir);
//             return dir;
        }


        fh_context
        getCWD()
        {
            const string& dir = getCWDString();
            RootContextFactory f;

            f.setContextClass( "file" );
            f.AddInfo( RootContextFactory::ROOT, "/" );
            f.AddInfo( RootContextFactory::PATH, dir );
    
            fh_context ret = f.resolveContext( RESOLVE_EXACT );
            return ret;
        }

        const fh_context&
        setCWD(fh_context& ctx)
        {
            int rc = chdir( ctx->getDirPath().c_str() );
            if( !rc )
                return ctx;

            ostringstream ss;
            ss << "Ferris::setCWD() for path:" << ctx->getDirPath();
            Throw_FerrisSetCWDException( tostr(ss), GetImpl(ctx) );
        }

        void ensureEA( fh_context c,
                       const std::string& name,
                       const std::string& value )
        {
            setStrAttr( c, name, value, true, true );
        }

        static void priv_createEA( fh_context c,
                                   const std::string& name,
                                   const std::string& value,
                                   const std::string& explicitPluginShortName,
                                   bool dontDelegateToOvermountContext )
        {
            fh_mdcontext md = new f_mdcontext();
            fh_mdcontext child = md->setChild( "ea", "" );
            child->setChild( "name",  name );
            child->setChild( "value", value );
            if( dontDelegateToOvermountContext )
            {
                child->setChild( "dont-delegate-to-overmount-context", "1" );
            }
            if( !explicitPluginShortName.empty() )
            {
                child->setChild( "explicit-plugin-name", explicitPluginShortName );
            }
//             cerr << "priv_createEA() name:" << name << " value:" << value
//                  << " value2:" << getStrSubCtx( child, "value", "", true )
//                  << endl;
            c->createSubContext( "", md );
        }
        
        void createEA( fh_context c,
                       const std::string& name,
                       const std::string& value )
        {
            priv_createEA( c, name, value, "", false );
        }

        void createEA( fh_context c,
                       const std::string& name,
                       const std::string& value,
                       const std::string& explicitPluginShortName )
        {
            priv_createEA( c, name, value, explicitPluginShortName, true );
        }
        
        void createEA( fh_context c,
                       const std::string& name,
                       const std::string& value,
                       bool dontDelegateToOvermountContext )
        {
            priv_createEA( c, name, value, "", dontDelegateToOvermountContext );
        }
        
        

        /**
         * If the subcontext is bound, return it, otherwise try to create a
         * new subcontext with the given name and return it.
         *
         * @param parent the parent of the desired context
         * @param rdn the name of the child of parent that is desired
         */
        fh_context acquireSubContext( fh_context parent,
                                      const std::string& rdn,
                                      bool isDir,
                                      int mode,
                                      ContextCreated_Sig_t& sigh )
        {
            parent->read();
            LG_CTX_D << "acquireSubContext parent:" << parent->getURL()
                     << " rdn:" << rdn
                     << " bound:" << parent->isSubContextBound( rdn )
                     << endl;
            if( parent->isSubContextBound( rdn ) )
            {
                return parent->getSubContext( rdn );
            }
            if( isDir )
            {
                return Shell::CreateDir( parent, rdn, false, mode, sigh );
            }
            else
            {
                return Shell::CreateFile( parent, rdn, mode, sigh );
            }
        }

        

        
        fh_context CreateFile( fh_context c,
                               const std::string& n,
                               int mode,
                               ContextCreated_Sig_t& sigh )

        {
            fh_mdcontext md = new f_mdcontext();
            fh_mdcontext child = md->setChild( "file", "" );
            child->setChild( "name", n );
            if( mode )
            {
                child->setChild( "mode", tostr( mode ) );
                child->setChild( "ignore-umask", "1" );
            }
            fh_context newc = c->createSubContext( "", md );
            sigh.emit( newc );

            return newc;
        }

        fh_context CreateDB4( fh_context c,
                              const std::string& n,
                              int mode,
                              ContextCreated_Sig_t& sigh )
        {
            fh_mdcontext md = new f_mdcontext();
            fh_mdcontext child = md->setChild( "db4", "" );
            child->setChild( "name", n );
            if( mode )
            {
                child->setChild( "mode", tostr( mode ) );
                child->setChild( "ignore-umask", "1" );
            }
            fh_context newc = c->createSubContext( "", md );
            sigh.emit( newc );

            return newc;
        }
        fh_context EnsureDB4( const std::string& path, const std::string& n )
        {
            try
            {
                fh_context ret = Resolve( path + "/" + n );
                return ret;
            }
            catch( exception& e )
            {
                return CreateDB4( Resolve(path), n );
            }
            
        }
        

        
        /**
         * Create a link to existingc in a new object with newrdn under the parent newc_parent.
         * Optionally use the URL of the existing object.
         */
        fh_context CreateLink( fh_context existingc,
                               fh_context newc_parent,
                               const std::string& newrdn,
                               bool useURL,
                               bool isSoft,
                               ContextCreated_Sig_t& sigh )
        {
            fh_mdcontext md = new f_mdcontext();
            fh_mdcontext child;
            if( isSoft )
                child = md->setChild( "softlink", "" );
            else
                child = md->setChild( "hardlink", "" );

            string target = existingc->getDirPath();
            if( useURL )
                target = existingc->getURL();
            
            child->setChild( "name",        newrdn );
            child->setChild( "link-target", target );
            fh_context newc   = newc_parent->createSubContext( "", md );
            sigh.emit( newc );
            return newc;
        }

        static fh_context CreateDirOneLevel( fh_context c,
                                             const std::string& n,
                                             int mode,
                                             ContextCreated_Sig_t& sigh )
        {
            LG_CTX_D << "CreateDirOneLevel() c:" << c->getURL() << " n:" << n << endl;
            fh_mdcontext md = new f_mdcontext();
            fh_mdcontext child = md->setChild( "dir", "" );
            child->setChild( "name", n );
            if( mode )
            {
//                cerr << "CreateDirOneLevel() user supplied mode:" << mode << endl;
                child->setChild( "mode", tostr(mode) );
                child->setChild( "ignore-umask", "1" );
            }
            else
            {
//                cerr << "CreateDirOneLevel() using mode 770" << endl;
                child->setChild( "mode", "770" );
//                child->setChild( "ignore-umask", "1" );
            }
            fh_context newc   = c->createSubContext( "", md );
//            cerr << "calling emit on sigh for newc:" << newc->getURL() << endl;
            
            sigh.emit( newc );
            LG_CTX_D << "CreateDirOneLevel() c:" << c->getURL() << " n:" << n
                     << " newc:" << newc->getURL()
                     << endl;
            return newc;
        }

        fh_context CreateDirWithParents( fh_context c,
                                         const std::string& n,
                                         int mode,
                                         ContextCreated_Sig_t& sigh )
        {
            typedef Context::SplitPath_t SplitPath_t;
            SplitPath_t sp = c->splitPath( n );

            for( SplitPath_t::iterator iter = sp.begin(); iter != sp.end(); ++iter )
            {
                LG_CTX_D << "CreateDirWithParents() c:" << c->getURL()
                         << " n:" << n
                         << " iter:" << *iter
                         << " iter.len:" << iter->length()
                         << endl;
                string rdn = *iter;

                if( rdn.empty() )
                    continue;
            
                try
                {
                    if( c->isSubContextBound( rdn ) )
                    {
                        c = c->getSubContext( rdn );
                    }
                    else
                    {
                        fh_context newc = Shell::CreateDirOneLevel( c, rdn, mode, sigh );
                        c = newc;
                    }
                }
                catch( FerrisCreateSubContextFailed& e )
                {
                    LG_CTX_D << "CreateDirWithParents() c:" << c->getURL()
                             << " n:" << n
                             << " iter:" << *iter
                             << " FerrisCreateSubContextFailed:" << e.what()
                             << endl;
                    c = c->getSubContext( rdn );
                }
            }
            return c;
        }
        

        /**
         * Create a directory context.
         *
         * @param c parent of new context
         * @param n rdn of new context
         * @param WithParents if true then 'n' can be a partial path and all directories
         *        in that path will be created under the context 'c'. Handy for creating
         *        directories from a given compile time prefix or ~ etc.
         */
        fh_context CreateDir( fh_context c,
                              const std::string& n,
                              bool WithParents,
                              int mode,
                              ContextCreated_Sig_t& sigh )
        {
            if( !WithParents )
            {
                return CreateDirOneLevel( c, n, mode, sigh );
            }
            return CreateDirWithParents( c, n, mode, sigh );
        }

        /**
         * Create a directory context.
         *
         * @param path the new context to create
         * @param WithParents if true then create all parent directories if they
         *        dont already exist. If false then only one directory may be created.
         */
        fh_context CreateDir( const std::string& path,
                              bool WithParents,
                              int mode,
                              ContextCreated_Sig_t& sigh )
        {
            fh_context rc = Resolve("/");

            if( !WithParents )
            {
                typedef Context::SplitPath_t SplitPath_t;
                SplitPath_t sp = rc->splitPath( path );

                string rdn   = sp.back();
                string cpath = path.substr( 0, path.length() - rdn.length() );
                fh_context c = Resolve( cpath );
                
                return CreateDirOneLevel( c, rdn, mode, sigh );
            }

            string earl   = CleanupURL( path );
            FerrisURL u   = FerrisURL::fromString( earl );
            string scheme = u.getInternalFerisScheme();
            string url    = u.getPath();

            LG_CTX_D << "CreateDir() scheme:" << scheme << " url:" << url << endl;
            
            return CreateDirWithParents( Resolve(scheme + ":///"), url, mode, sigh );
        }

//         //
//         // FIXME: should walk known schemes here too
//         //
//         bool isRelativePath( const std::string& path )
//         {
//             return( !starts_with( path, "/" )
//                     && !starts_with( path, "file:" )
//                     && !starts_with( path, "~" )
//                     && !starts_with( path, "root:" )
//                     && !starts_with( path, "x-ferris:" ) );
//         }
        

//        template<>
        stringlist_t::iterator XparseSeperatedList( const std::string& s,
                                                   stringlist_t& c,
                                                   stringlist_t::iterator out,
                                                   const char sepchar )
        {
//            bool dummy = Loki::Conversion< OutputIterator, stringlist_t::iterator >::exists;
            
            std::string tmp;
            std::stringstream ss(s);
            while( std::getline( ss, tmp, sepchar ))
                if( !tmp.empty() )
                {
                    *++out = tmp;
                }

            return out;
        }
        
        
        fh_context acquireContext( std::string path,
                                   int mode,
                                   bool isDir,
                                   ContextCreated_Sig_t& sigh )
        {
//             cerr << "acquireContext() path:" << path << " isrel:" << isRelativePath(path)
//                  << endl;
//             if( isRelativePath(path) )
//             {
//                 path = getCWDDirPath() + "/" + path;
//             }
            
            LG_CTX_D << "acquireContext(top) path:" << path << endl;
            try
            {
                fh_context c = Resolve( path );
                return c;
            }
            catch( exception& e )
            {
                LG_CTX_D << "acquireContext() e:" << e.what() << endl;
//                 string rootdir = "/";
//                 if( path.find(":") != string::npos )
//                 {
//                     rootdir = path.substr( 0, path.find(":") ) + "://";
//                     path = path.substr( path.find(":")+1 );
//                     PrefixTrimmer trimmer;
//                     trimmer.push_back( "/" );
//                     path = trimmer( path );
//                 }
//                 if( starts_with( path, "~" ))
//                 {
//                     rootdir = "~";
//                     PrefixTrimmer trimmer;
//                     trimmer.push_back( "/" );
//                     trimmer.push_back( "~" );
//                     path = trimmer( path );
//                 }
                
//                 LG_CTX_D << "acquireContext() path:" << path
//                          << " root:" << rootdir
//                          << endl;

//                 fh_context c = 0;
//                 if( isDir )
//                     c = Shell::CreateDir( Resolve( rootdir ), path, true, mode, sigh );
//                 else
//                     c = Shell::CreateFile( Resolve( rootdir ), path, mode, sigh );
//                 return c;



//                cerr << "acquireContext(make) path:" << path << endl;
                



                
                string rdn = "";
//                int slashpos = path.find("/");
                int slashpos = -1;
                fh_context lastc = 0;
                if( string::npos == path.find("/") )
                {
                    lastc = getCWD();
                }
                else
                {
                    if( path.find("/") == 0 )
                        slashpos = 0;
                    
                    while( true )
                    {
                        try
                        {
                            int tpos = path.find( '/', slashpos+1 );
                            LG_CTX_D << "acquireContext(seeking) path:" << path
                                     << " path.substr:" << path.substr( 0, tpos )
                                     << " slashpos:" << slashpos
                                     << " tpos:" << tpos
                                     << endl;
                            fh_context tc = Resolve( path.substr( 0, tpos ));
                            slashpos = tpos;
                            lastc = tc;
                        }
                        catch( ... )
                        {
                            break;
                        }
                    }
                }
                
                LG_CTX_D << "acquireContext(lastc) lastc:" << toVoid(lastc) << endl;
                
                fh_context parent = lastc;
                LG_CTX_D << "acquireContext(lastc) parent:" << toVoid(parent) << endl;
                LG_CTX_D << "acquireContext(lastc) parent:" << parent->getURL() << endl;
                
                rdn = path.substr( slashpos+1, path.length() - slashpos );

                LG_CTX_D << "TOUCH() parent:" << parent->getURL()
                         << " rdn:" << rdn
                         << " slashpos:" << slashpos
                         << endl;
//                 cerr << "TOUCH() parent:" << parent->getURL()
//                      << " rdn:" << rdn
//                      << " slashpos:" << slashpos
//                      << endl;

                /**
                 * Make all non terminal objects directories and the last object a file
                 * if we are requested to make a 'file'
                 */
                fh_context c = parent;
                stringlist_t rdnlist;
                Util::parseSeperatedList( rdn, rdnlist, back_inserter( rdnlist ), '/' );
//                XparseSeperatedList( rdn, rdnlist, back_inserter( rdnlist ), '/' );
                for( stringlist_t::const_iterator si = rdnlist.begin(); si != rdnlist.end(); ++si )
                {
                    string rdn = *si;

                    bool createDir = isDir;
                    stringlist_t::const_iterator nextiter = si;
                    ++nextiter;
                    if( nextiter != rdnlist.end() )
                        createDir = true;
                    
                    LG_CTX_D << "TOUCH(building) c:" << c->getURL()
                             << " rdn:" << rdn
                             << endl;
                    
                    if( createDir )
                        c = Shell::CreateDir( c, rdn, true, mode, sigh );
                    else
                        c = Shell::CreateFile( c, rdn, mode, sigh );
                }
                return c;
            }
        }
        
    };
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    namespace Shell
    {
        string getHomeDirPath_nochecks()
        {
            static bool   cached_ret_set = false;
            static string cached_ret;

            if( !runningSetUID() && cached_ret_set )
                return cached_ret;
            
            string ret;


//            const char *home = g_get_home_dir();
//    const char *home = getenv("HOME");
//            ret = home;
            ret = ferris_g_get_home_dir();

            if( !cached_ret_set )
            {
                cached_ret_set = true;
                cached_ret = ret;
            }

            return ret;
        }

        string getCWDDirPath()
        {
            gchar* cdir = g_get_current_dir();
            string ret = cdir;
            g_free(cdir);
            return ret;
        }

        void setCWDDirPath( const std::string& p )
        {
            if( 0 != chdir( p.c_str() ))
            {
                int eno = errno;
                fh_stringstream ss;
                ss << "Can not setCWDDirPath to p:" << p << endl;
                ThrowFromErrno( eno, tostr(ss), 0 );
            }
        }


        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////

        /**
         *
         * NOTE: This is under debait as to if this class should be used or
         * appendToPath() used to pass the full path to system calls.
         *
         * Sets the CWD to the given value and then restores it when this
         * object is deleted. The use of this class could also hold a lock
         * against other users of setCWDDirPath() so that they have to wait
         * for this object to die.
         */
        class FERRISEXP_DLLLOCAL HoldRestoreCWD
        {
            const std::string& oldCWD;
        public:
            HoldRestoreCWD( const std::string& p );
            ~HoldRestoreCWD();
        };

        
        HoldRestoreCWD::HoldRestoreCWD( const std::string& p )
            :
            oldCWD( getCWDDirPath() )
        {
            setCWDDirPath( p );
        }
        
        HoldRestoreCWD::~HoldRestoreCWD()
        {
            setCWDDirPath( oldCWD );
        }
        
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////
        

        string getTmpDirPath()
        {
            const char *td = getenv("FERRIS_TMP");
//            cerr << "getTmpDirPath() td:" << ( td ? td : "null" ) << endl;
            if( td )
            {
                return td;
            }
            return g_get_tmp_dir();
        }



        string getHomeDirPath()
        {
            string ret = getHomeDirPath_nochecks();
    
    
            if(!ret.length())
            {
                LG_PATHS_ER
                    << "Can not determine home directory! please set HOME and try again"
                    << endl;
                exit(1);
            }
    
            return ret;
        }

        bool contextExists( const std::string& path )
        {
            try
            {
                fh_context c = Resolve( path );
                return true;
            }
            catch(...)
            {
            }
            return false;
        }


        static struct passwd* passwd_lookup_by_name( string name )
        {
            if( name.empty() )
            {
                struct passwd* ptr = getpwuid( getuid() );
                return ptr;
            }

            struct passwd* ptr = getpwnam( name.c_str() );
            return ptr;
            
//             struct passwd* ptr = 0;

//             setpwent();
//             while( ptr = getpwent() )
//             {
//                 if( name == ptr->pw_name )
//                     break;
//             }
//             endpwent();
//             return ptr;
        }

        uid_t getUserID( const std::string& name )
        {
            struct passwd* ptr = passwd_lookup_by_name( name );

            if( ptr )
                return ptr->pw_uid;

            fh_stringstream ss;
            ss << "Cant find user:" << name << " in password database";
            Throw_NoSuchUser( tostr(ss), 0 );
        }
        
        gid_t getGroupID( const std::string& name )
        {
            struct passwd* ptr = passwd_lookup_by_name( name );

            if( ptr )
                return ptr->pw_gid;

            fh_stringstream ss;
            ss << "Cant find user:" << name << " in password database";
            Throw_NoSuchUser( tostr(ss), 0 );
        }

        string getUserName( uid_t id )
        {
            struct passwd* p = getpwuid( id );
            return ( p ? p->pw_name : "" );
        }

        string getGroupName( gid_t id )
        {
            struct group *g = getgrgid( id );
            return( g ? g->gr_name : "" );
        }

        fh_context touch( const std::string& path,
                          bool create,
                          bool isDir,
                          int  mode,
                          bool touchMTime,
                          bool touchATime,
                          time_t useMTime,
                          time_t useATime )
        {
            return touch( path, "",
                          create, isDir, mode,
                          touchMTime, touchATime,
                          useMTime, useATime );
        }
        
        fh_context touch( const std::string& path,
                          const std::string& SELinux_context,
                          bool create,
                          bool isDir,
                          int  mode,
                          bool touchMTime,
                          bool touchATime,
                          time_t useMTime,
                          time_t useATime )
        {
            int rc = 0;
            fh_context c = 0;

            if( create )
            {
                c = acquireContext( path, mode, isDir );
            }
            else
                c = Resolve( path );

            if( !SELinux_context.empty() )
            {
                setStrAttr( c,
                            "dontfollow-selinux-context",
                            SELinux_context,
                            true, true );
            }
            
            struct utimbuf tbuf;
            time_t timenow = Time::getTime();

            if( touchMTime )
            {
                if( useMTime ) tbuf.modtime = useMTime;
                else           tbuf.modtime = timenow;
            }
            else
            {
                tbuf.modtime = toType<time_t>(getStrAttr( c, "mtime", "0", true, true ));
            }

            if( touchATime )
            {
                if( useATime ) tbuf.actime = useATime;
                else           tbuf.actime = timenow;
            }
            else
            {
                tbuf.actime = toType<time_t>(getStrAttr( c, "atime", "0", true, true ));
            }
            
            
                
            
            if( c->getIsNativeContext() )
            {
                rc = utime( c->getDirPath().c_str(), &tbuf );

                if( rc )
                {
                    int eno = errno;
                    fh_stringstream ss;
                    ss << "Can not touch file url:" << c->getURL() << endl;
                    ThrowFromErrno( eno, tostr(ss), 0 );
                }
            }
            
            
            return c;
        }

        int generateTempFD( std::string& templateStr )
        {
            string s = templateStr;
            if( !ends_with( s, "XXXXXX" ) )
            {
                int dotpos = s.rfind( "." );
                if( dotpos == string::npos )
                {
                    s = s + "XXXXXX";
                }
                else
                {
                    s = s.substr( 0, dotpos ) + "-XXXXXX" + s.substr( dotpos );
                }
            }

            int fd = mkstemp( (char*)s.c_str() );

            if( fd == -1 )
            {
                ThrowFromErrno( errno, (string)"error creating tempfile at:" + s );
            }
            templateStr = s;
            return fd;
        }
        
        
        fh_iostream generteTempFile( std::string& templateStr, bool closeFD )
        {
            int fd = generateTempFD( templateStr );
            fh_iostream ret = Factory::MakeFdIOStream( fd, closeFD );
            return ret;
        }

        
        fh_context generateTempDir( std::string& templateStr )
        {
            string s = templateStr;
            if( !starts_with( s, "/tmp" ) )
            {
                stringstream ss;
                ss << "/tmp/" << getuid() << "-" << s;
                s = ss.str();
            }
            
            if( !ends_with( s, "XXXXXX" ) )
            {
                int dotpos = s.rfind( "." );
                if( dotpos == string::npos )
                {
                    s = s + "XXXXXX";
                }
                else
                {
                    s = s.substr( 0, dotpos ) + "-XXXXXX" + s.substr( dotpos );
                }
            }
        
            char* p = mkdtemp( (char*)s.c_str() );
            if( !p )
            {
                ThrowFromErrno( errno, (string)"error creating temp dir at:" + s );
            }

            
            templateStr = p;
            fh_context ret = Resolve( templateStr );
            return ret;
        }
        
        int generateTempFD()
        {
            string s = "/tmp/temp";
            return generateTempFD( s );
        }

        int
        generateTempFD( const char* templateStrReadOnly )
        {
            string s = templateStrReadOnly;
            return generateTempFD( s );
        }
        
        
        fh_iostream generteTempFile( bool closeFD )
        {
            string s = "/tmp/temp";
            return generteTempFile( s, closeFD );
        }
        

        
    };
};
