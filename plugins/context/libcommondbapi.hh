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

    $Id: libcommondbapi.hh,v 1.14 2010/09/24 21:31:33 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_COMMONDB_H_
#define _ALREADY_INCLUDED_FERRIS_COMMONDB_H_

#include <FerrisContextPlugin.hh>
#include <Trimming.hh>
#include <Configuration_private.hh>
#include <PluginOutOfProcNotificationEngine.hh>

#include <sigc++/sigc++.h>
#include <sigc++/slot.h>
#include <sigc++/bind.h>

#include <errno.h>

#include <map>

namespace Ferris
{
    using namespace std;
    
    FERRISEXP_EXPORT std::string ensureStartsWithSlash( const std::string& s );

    template < class ChildContextType >
    class CommonDBContext
        :
        public StateLessEAHolding_Recommending_ParentPointingTree_Context< ChildContextType >,
        public IHandleOutOfProcContextCreationNotification,
        public IHandleOutOfProcContextDeletionNotification,
        public IHandleOutOfProcEANotification
    {
        typedef CommonDBContext<ChildContextType> _Self;
        typedef StateLessEAHolding_Recommending_ParentPointingTree_Context< ChildContextType > _Base;

    protected:

        /*********************************************************************************/
        /*********************************************************************************/
        /*
         * These are the methods that a child must provide
         */
        virtual std::string getValue( const std::string& k ) = 0;
        virtual void setValue( const std::string& k, const std::string& v ) = 0;
        /**
         * get a list of all the EA names for this context
         */
        virtual stringlist_t getEAKeys() = 0;

        /**
         * For cross process notification to (easily) work, the db file must be
         * read first. Otherwise, explicit notifications with the exact correct
         * path must be given to the OutOfProcessNotificationEngine
         */
        void ensureRead()
            {
                this->read(0);
            }
        
    public:
        /**
         * if a subcontext class wants priv_supportsShortCutLoading()
         * then it needs to override that method and return true;
         * and implement this method to setup the EA for a shortcut laoded context
         */
        virtual void setupEAForShortCutLoadedContext()
            {
            }
    protected:
        
        /*********************************************************************************/
        /*********************************************************************************/
        // These are functions that you may wish to override, but the defaults
        // will work aswell
        //
        virtual std::string getValue()
            {
                return getValue( dbKey() );
            }
            
        virtual void setValue( const std::string& v )
            {
                setValue( dbKey(), v );
            }
            
        virtual void OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
                LG_COMMONDB_D << "OnStreamClosed()" << endl;
                
                AdjustForOpenMode_Closing( ss, m, tellp );
                
                const std::string s = StreamToString(ss);
                LG_COMMONDB_D << "OnStreamClosed() len:" << s.length()
                              << endl;
//                if( s.length() > 4 )
//                 LG_COMMONDB_D << "OnStreamClosed() first bytes are..."
//                               << s[0] << s[1] << s[2] << s[3] << endl;

                setValue( s );
            }

        virtual fh_stringstream real_getIOStream()
            {
                fh_stringstream ss;

                std::string s = getValue();
                LG_COMMONDB_D << "real_getIOStream() " << endl;
//                 if( s.length() > 4 )
//                     LG_COMMONDB_D << "real_getIOStream() s:" << s[0] << s[1] <<s[2] << s[3] << endl;
                ss << s;
                return ss;
            }
            


        /******************************************************************************/
        /******************************************************************************/
        /******************************************************************************/
    protected:
        
        static fh_iostream SL_getEAStream( CommonDBContext* c, const std::string& rdn, EA_Atom* atom )
            {
                return c->getEAStream( c, rdn, atom );
            }
            
        static void SL_setEAStream( CommonDBContext* c, const std::string& rdn, EA_Atom* atom, fh_istream iss )
            {
                c->setEAStream( c, rdn, atom, iss );
            }
        
        static fh_context SL_commondb_SubCreate_dir(  fh_context c, fh_context md )
            {
                return c->SubCreate_file( c, md );
            }
            
        static fh_context SL_commondb_SubCreate_file( fh_context c, fh_context md )
            {
                return c->SubCreate_file( c, md );
            }
            
        static fh_context SL_commondb_SubCreate_ea(   fh_context c, fh_context md )
            {
                LG_COMMONDB_D << "SL_commondb_SubCreate_ea() c:" << c->getURL() << endl;

                if( CommonDBContext* edbc = dynamic_cast<CommonDBContext*>(GetImpl(c)))
                {
                    std::string rdn      = getStrSubCtx( md, "name", "" );
                    std::string v        = getStrSubCtx( md, "value", "" );
                    std::string dbKey    = edbc->dbKey( edbc );

                    fh_stringstream kss;
//                     if( dbKey.length() ) kss << ensureStartsWithSlash( dbKey ) << "/" << rdn;
//                     else                 kss << "/" << rdn;
                    kss << edbc->buildEAKey( dbKey, rdn );
                    
                    std::string k        = tostr(kss);

                    LG_COMMONDB_D << "SL_commondb_SubCreate_ea() c:" << c->getURL()
                                  << " k:" << k
                                  << " v:" << v
                                  << " rdn:" << rdn
                                  << endl;
        
                    edbc->setValue( k, v );
                    edbc->ensureEACreated( rdn, false );

                    Factory::getPluginOutOfProcNotificationEngine().signalEACreated( c, rdn );
        
//         cerr << " edbc:" << (void*)GetImpl(c) << endl;
//         cerr << "SL ... isAttributeBound(rdn) :" << c->isAttributeBound( rdn, false ) << endl;
//         {
//             fh_istream ss = edbc->getAttribute( rdn )->getIStream();
//             std::string s;
//             getline( ss, s );
//             cerr << " read s1:" << s << endl;
//             cerr << " read s2:" << getStrAttr( edbc, rdn, "no" ) << endl;
//             cerr << " read s3:" << getStrAttr( c,    rdn, "no" ) << endl;
//         }
        
                }

//    cerr << " SL ... c:" << (void*)GetImpl(c) << endl;
                return c;
            }

//         static fh_istream
//         SL_getIsDirStream( Context* c, const std::string& rdn, EA_Atom* atom )
//             {
//                 fh_stringstream ss;
//                 ss << c->hasSubContexts();
//                 return ss;
//             }

        static fh_istream
        SL_getSizeStream( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                std::string v = c->getValue();
                ss << v.length();
                return ss;
            }
        
        static fh_istream
        SL_getMimeTypeStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
//                ss << "application/db4-data";
                ss << c->getMimeType();
                return ss;
            }

        static fh_istream
        SL_getHasSubContextsGuessStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->hasSubContexts();
                return ss;
            }
        
        void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
#define SLEA this->tryAddStateLessAttribute
                    SLEA( "size",                  SL_getSizeStream );
//                    SLEA( "is-dir",                SL_getIsDirStream );
                    SLEA( "mimetype",              SL_getMimeTypeStream );
                    SLEA( "has-subcontexts-guess", SL_getHasSubContextsGuessStream );
                    SLEA( "mtime",                 SL_getEAStream, SL_getEAStream, SL_setEAStream );
                    SLEA( "ctime",                 SL_getEAStream, SL_getEAStream, SL_setEAStream );
                    SLEA( "atime",                 SL_getEAStream, SL_getEAStream, SL_setEAStream );
#undef SLEA
                    _Base::createStateLessAttributes( true );
                    this->supplementStateLessAttributes( true );
                }
            }

        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            {
                return real_getIOStream();
            }
            
        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ret = real_getIOStream();
                ret->getCloseSig().connect( sigc::bind( sigc::mem_fun( *this, &CommonDBContext::OnStreamClosed ), m )); 

                LG_COMMONDB_D << "priv_getIOStream() url:" << this->getURL()
                              << " tellp:" << ret->tellp()
                              << " tellg:"  << ret->tellg()
                              << endl;
    
                return ret;
            }



        /**
         * Given a path anywhere in the edb, create a file for that path. note that the
         * path can have relative parts in it. The path must already have a dir to exist in!
         */
        fh_context create_commondb_file( CommonDBContext* c, std::string dirty_path )
            {
                LG_COMMONDB_D << "CommonDBContext::create_commondb_file(top) c:" << c->getURL() << endl
                              << " dirty_path:" << dirty_path
                              << endl;
                
                fh_context dbc     = Resolve( dbFileName() );
                std::string dbfn        = dbc->getURL();
                LG_COMMONDB_D << "CommonDBContext::create_commondb_file(2) c:" << c->getURL()
                              << " path:" << dirty_path
                              << endl;
                if( !starts_with( dirty_path, "file:" ) )
                {
                    dirty_path = (string)"file://" + dirty_path;
                }
                
                fh_context parent  = Resolve( dirty_path, RESOLVE_PARENT );
                LG_COMMONDB_D << "CommonDBContext::create_commondb_file(2.1) c:" << c->getURL() << endl;
                std::string     parentp = parent->getURL();

                LG_COMMONDB_D << "CommonDBContext::create_commondb_file(3) c:" << c->getURL() << endl
                              << " dirty_path:" << dirty_path << endl
                              << " parentp:" << parentp << endl;
                
                fh_stringstream ss;
                ss << parentp << "/" << dirty_path.substr( parentp.length()+1 );
                std::string path = tostr(ss);

                LG_COMMONDB_D << "CommonDBContext::create_commondb_file(4) c:" << c->getURL() << endl
                              << " path:" << path << endl;
                
                if( starts_with( path, dbfn ) )
                {
                    path = path.substr( dbfn.length() + 1 );
                }
                LG_COMMONDB_D << "CommonDBContext::create_commondb_file() c:" << c->getURL() << endl
                              << " dirty_path:" << dirty_path << endl
                              << " path:" << path << endl
                              << " dbfn:" << dbfn << endl
                              << " base context:" << this->getBaseContext()->getURL()
                              << endl;
                
                fh_context newc = Shell::CreateFile( this->getBaseContext(), path );
                return newc;
            }


        virtual fh_context SubCreate_file( fh_context c, fh_context md )
            {
                std::string rdn      = getStrSubCtx( md, "name", "" );
                std::string v        = "";

//                ensureRead();
                LG_COMMONDB_D << "CommonDBContext::SubCreate_file rdn:" << rdn
                              << " c:" << c->getURL()
                              << endl;
    
                if( CommonDBContext* edbc = dynamic_cast<CommonDBContext*>(GetImpl(c)))
                {
                    fh_stringstream kss;
                    kss << edbc->dbKey( edbc ) << "/" << rdn;
                    std::string k = tostr(kss);

                    PrefixTrimmer pretrimmer;
                    pretrimmer.push_back( "/" );
                    k = pretrimmer( k );

                    LG_COMMONDB_D << "db::subcreate_file... c:" << c->getURL()
                                  << " db:" << edbc->dbFileName()
                                  << " k:" << k
                                  << " v:" << v
                                  << " dbkey:" << edbc->dbKey( GetImpl(c) )
                                  << " rdn:" << rdn
                                  << endl;

                    edbc->setValue( k, v );

                    fh_context childc = edbc->ensureContextCreated( rdn, true );

//                    cerr << "edbc->getBaseContext()  url:" << edbc->getBaseContext()->getURL() << endl;
//                    cerr << "edbc->getBaseContext() read:" << edbc->getBaseContext()->getHaveReadDir() << endl;
                    if( !edbc->getBaseContext()->getHaveReadDir() )
                    {
                        Factory::getPluginOutOfProcNotificationEngine().ensureServerRunning();
                        Factory::getPluginOutOfProcNotificationEngine().signalContextCreated( childc );
                    }
                    return childc;
//         return priv_readSubContext( rdn, true );
                }

                fh_stringstream ss;
                ss << "Attempt to create an edb EA attribute on a context that is not an edb module."
                   << " url:" << c->getURL()
                   << endl;
                Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
            }

        virtual bool supportsMonitoring()
            {
                return true;
            }

        virtual bool
        supportsRename()
            {
                return true;
            }

        virtual fh_context
        priv_rename( const std::string& rdn,
                     const std::string& newPathRelative,
                     bool TryToCopyOverFileSystems,
                     bool OverWriteDstIfExists )
            {
                std::string oldPath = this->appendToPath( this->getDirPath(), rdn );
                std::string newPath = this->appendToPath( this->getDirPath(), newPathRelative, true );

                try
                {
                    LG_COMMONDB_D << "priv_rename() rdn:" << rdn 
                                  << " url:" << this->getURL() << endl
                                  << " oldPath:" << oldPath << endl
                                  << " newPath:" << newPath << endl
                                  << " newPathRelative:" << newPathRelative << endl
                                  << " TryToCopyOverFileSystems:" << TryToCopyOverFileSystems
                                  << " OverWriteDstIfExists:" << OverWriteDstIfExists
                                  << " dbFileName:" << dbFileName()
                                  << endl;

                    fh_context dbc = Resolve( dbFileName() );
                    LG_COMMONDB_D << "priv_rename(2) url:" << this->getURL() << endl;
                    std::string dbfn = dbc->getDirPath();
                    LG_COMMONDB_D << "priv_rename(3) url:" << this->getURL() << endl;
                    LG_COMMONDB_D << "priv_rename(3) newPath:" << newPath << endl;

                    fh_context newc = create_commondb_file( this->getBaseContext(), newPath );
                    LG_COMMONDB_D << "priv_rename(4) url:" << this->getURL() << endl;
                    LG_COMMONDB_D << "priv_rename(4) newc:" << newc->getURL() << endl;
                    {
                        fh_iostream ioss   = newc->getIOStream( ios::trunc | ios::out );
                        fh_context rdn_ctx = this->getSubContext( rdn );
                        fh_attribute attr  = rdn_ctx->getAttribute( "." );

                        LG_COMMONDB_D << "Have ioss, at seeks"
                                      << " get:" << ioss->tellg()
                                      << " put:" << ioss->tellp()
                                      << " isgood:" << ioss->good()
                                      << endl;
                
                        ioss = attr->copyTo( ioss );
                        if( !ioss.good() )
                        {
                            fh_stringstream ss;
                            ss << "Rename attempt failed. URL:" << this->getURL()
                               << " error writing to new destination. "
                               << " src:" << rdn
                               << " dst:" << newPath;
                            Throw_RenameFailed( tostr(ss), this );
                        }
                    }

                    /* Rename all the EA for the old context too */
                    CommonDBContext* c = 0;
                    {
                        fh_context c_ctx = this->getSubContext( rdn );
                        c = dynamic_cast<CommonDBContext*>( (GetImpl(c_ctx) ) );
                    }
                    if( !c )
                    {
                        fh_stringstream ss;
                        ss << "Rename attempt failed. URL:" << this->getURL()
                           << " src:" << rdn << " dst:" << newPath
                           << " attempt to rename a non ISAM db subcontext."
                           << endl;
                        Throw_RenameFailed( tostr(ss), this );
                    }

                
                    stringlist_t ealist = c->getEAKeys();
//                 cerr << " c:" << c->getDirPath()
//                      << " listsize:" << ealist.size()
//                      << endl;
                    for( stringlist_t::iterator iter = ealist.begin(); iter != ealist.end(); ++iter )
                    {
                        int size=0;
                        std::string oldeaname = *iter;
                        std::string eaname    = oldeaname.substr( oldeaname.rfind( "/" )+1 );

                        fh_stringstream neweanamess;
                        neweanamess << newc->getDirPath() << "/" << eaname;
                        std::string neweaname = tostr(neweanamess);
                        neweaname = neweaname.substr( dbfn.length() );
                    

                        LG_COMMONDB_D << "rename EA. oldeaname:" << oldeaname
                                      << " neweaname:" << neweaname
                                      << " eaname:" << eaname
                                      << endl;

                        std::string data = getValue( oldeaname );
                        setValue( neweaname, data );
                    }

                    LG_COMMONDB_D << "now just remove old, return new" << endl;
                
                    /* Remove old context and return the new */
                    this->remove( rdn );

                    LG_COMMONDB_D << "removed old... resolving new path:" << newPath << endl;
                    return Resolve( newPath );
                }
                catch( std::exception& e )
                {
                    fh_stringstream ss;
                    ss << "Rename attempt failed. URL:" << this->getURL()
                       << " src:" << rdn << " dst:" << newPath
                       << " e:" << e.what()
                       << endl;
                    LG_COMMONDB_D << tostr(ss);
                    Throw_RenameFailed( tostr(ss), this );
                }
            }

//        virtual bool priv_supportsShortCutLoading()
//            { return true; }

        FERRIS_CTX_SMARTPTR( ChildContextType, fh_childc );
        
        virtual fh_context priv_getSubContext( const std::string& rdn )
            {
                if( !this->supportsShortCutLoading() )
                    return _Base::priv_getSubContext( rdn );
                
                try
                {
                    LG_COMMONDB_D << "priv_getSubContext() p:" << this->getDirPath()
                                  << " rdn:" << rdn
                                  << " this:" << this->getURL()
                                  << endl;

                    Context::Items_t::iterator isSubContextBoundCache;
                    if( this->priv_isSubContextBound( rdn, isSubContextBoundCache ) )
                    {
                        LG_COMMONDB_D << "priv_getSubContext(bound already) p:" << this->getDirPath()
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
                    
                          

                    string k = dbKey( this, rdn );

                    try
                    {
                        LG_COMMONDB_D << "priv_getSubContext(getting key) k:" << k << endl;
                        getValue( k );

                        LG_COMMONDB_D << "priv_getSubContext(got key) k:" << k << endl;

                        fh_childc ret = this->getBaseContext()->ensureEAorContextCreated( k );

//                         cerr << "priv_getSubContext() p:" << this->getDirPath()
//                              << " rdn:" << rdn
//                              << " this:" << this->getURL()
//                              << " ret:" << ret->getURL()
//                              << endl;
                        
                        ret->setupEAForShortCutLoadedContext();
                        
//                     return ret;
                        if( this->priv_isSubContextBound( rdn, isSubContextBoundCache ) )
                        {
                            return *isSubContextBoundCache;
                        }
                        
                        fh_stringstream ss;
                        ss << "NoSuchSubContext k:" << k;
                        Throw_NoSuchSubContext( tostr(ss), this );
                    }
                    catch( Db4Exception& e )
                    {
                       fh_stringstream ss;
                        ss << "NoSuchSubContext k:" << k;
                        Throw_NoSuchSubContext( tostr(ss), this );
                    }
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

    
        
        
            
    public:

        CommonDBContext()
            :
            _Base( 0, "" )
            {
                createStateLessAttributes();
            }
            
        virtual ~CommonDBContext()
            {
            }
        
        virtual bool getSubContextAttributesWithSameNameHaveSameSchema()
            {
                return false;
            }

        std::string dbFileName()
            {
                CommonDBContext* c = this->getBaseContext();
                std::string s = c->getDirPath();
                return s;
            }

        int getOnDiskFormat()
            {
                return 1;
                
//                 CommonDBContext* c = this->getBaseContext();
//                 typedef std::map< CommonDBContext*, int > cache_t;
//                 static cache_t cache;
//                 typename cache_t::iterator ci = cache.find( c );
//                 if( ci != cache.end() )
//                     return ci->second;

//                 string vstr = "1";
//                 try
//                 {
//                     vstr = getValue( "///ferris-metadata-format-version" );
//                 }
//                 catch( ... )
//                 {}
//                 if( vstr.empty() )
//                     vstr = "1";
//                 int v = toint(vstr);
//                 cache[ c ] = v;
//                 return v;
            }

        std::string eaKeySeparator()
            {
                if( getOnDiskFormat() < 2 )
                    return "/";
                return "//";
            }
        std::string buildEAKey( const std::string& earl, const std::string& eaname = "" )
            {
                stringstream ss;
                if( !earl.empty() )
                {
                    PrePostTrimmer trimmer;
                    trimmer.push_back( "/" );
                    ss << "/" << trimmer( earl );
                }
                if( !eaname.empty() )
                {
                    ss << eaKeySeparator() << eaname;
                }
                
                return ss.str();
            }
        
        std::string eaKey( Attribute* a = 0, const std::string& rdn = "" )
            {
                return buildEAKey( dbKey( a ), rdn );
            }
        
        std::string dbKey( Attribute* a = 0, const std::string& rdn = "" )
            {
                if( !a )
                    a = this;
    
                std::string path     = a->getDirPath();
                if( rdn.length() )
                {
                    PostfixTrimmer trimmer;
                    trimmer.push_back( "/" );
                    path = trimmer( path );
                    path += "/";
                    path += rdn;
                }
                std::string edbfn    = dbFileName();
                std::string k        = path.substr( edbfn.length() );

                PrefixTrimmer pretrimmer;
                pretrimmer.push_back( "/" );
                k = pretrimmer( k );

                LG_COMMONDB_D << "dbKey() path:" << path
                              << " edbfn:" << edbfn << " k:" << k << endl;
                return k;
            }

        
        fh_iostream getEAStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
//                std::string k        = ensureStartsWithSlash( dbKey( c, rdn ) );
                std::string k        = eaKey( c, rdn );

                LG_COMMONDB_D << "getEAStream() path:" << c->getDirPath()
                              << " rdn:" << rdn
                              << " k:" << k
                              << endl;

                fh_stringstream ss;
                try
                {
                    ss << getValue( k );
                }
                catch( exception& e )
                {
                    static bool FUZZ = getenv("LIBFERRIS_FUZZ");
                    LG_COMMONDB_D << "FUZZ:" << FUZZ << endl;
                    LG_COMMONDB_D << " rdn:" << rdn << endl;
                
                    if( FUZZ && rdn == "mtime" )
                    {
                        ss << Time::getTime();
                        return ss;
                    }
                    throw;
                }
                return ss;
            }
            
        void setEAStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream zz )
            {
//                std::string k        = ensureStartsWithSlash( dbKey( c, rdn ) );
                std::string k        = eaKey( c, rdn );
                std::string v        = StreamToString( zz );

                LG_COMMONDB_D << "setEAStream() url:" << c->getURL()
                              << " k:" << k << " v:" << v
                              << endl;

                setValue( k, v );
            }
            

        virtual ferris_ios::openmode getSupportedOpenModes()
            {
                return ios::in | ios::out | ios::binary | ios::ate | ios::trunc;
            }
    
//         virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
//             {
//                 m["dir"] = SubContextCreator( _Self::SL_commondb_SubCreate_dir,
//                                              "	<elementType name=\"dir\">\n"
//                                              "		<elementType name=\"name\" default=\"new directory\">\n"
//                                              "			<dataTypeRef name=\"string\"/>\n"
//                                              "		</elementType>\n"
//                                              "	</elementType>\n");
//                 m["file"] = SubContextCreator(SL_commondb_SubCreate_file,
//                                               "	<elementType name=\"file\">\n"
//                                               "		<elementType name=\"name\" default=\"new file\">\n"
//                                               "			<dataTypeRef name=\"string\"/>\n"
//                                               "		</elementType>\n"
//                                               "	</elementType>\n");

//                 /* Note that EA created for the root of the db4 file will not show up
//                  *  until after the db4 has been read()
//                  */
//                 m["ea"] = SubContextCreator(
//                     SL_commondb_SubCreate_ea,
//                     "	<elementType name=\"ea\">\n"
//                     "		<elementType name=\"name\" default=\"new ea\">\n"
//                     "			<dataTypeRef name=\"string\"/>\n"
//                     "		</elementType>\n"
//                     "		<elementType name=\"value\" default=\"\">\n"
//                     "			<dataTypeRef name=\"string\"/>\n"
//                     "		</elementType>\n"
//                     "	</elementType>\n");
//             }


        virtual void OnOutOfProcContextCreationNotification( const std::string& rdn,
                                                             bool isDataValid,
                                                             const std::string& data )
            {
                LG_JOURNAL_D << "db4::OnContextCreated() this:" << this->getURL()
                             << " rdn:" << rdn
                             << endl;
            
                this->ensureContextCreated( rdn, true );
            }
        virtual void OnOutOfProcContextDeletionNotification( const std::string& rdn )
            {
                if( this->isSubContextBound( rdn ))
                {
                    fh_context sub = this->getSubContext( rdn );
                    this->Remove( sub, true );
                }
            }
        virtual void OnOutOfProcEACreationNotification( const std::string& eaname,
                                                        bool isDataValid,
                                                        const std::string& data )
            {
                LG_JOURNAL_D << "OnOutOfProcEACreationNotification() ea:" << eaname << endl;
                this->ensureEACreated( eaname, false );
            }
        
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
};
#endif
