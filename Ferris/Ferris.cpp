/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of libferris.

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

    $Id: Ferris.cpp,v 1.102 2011/07/31 21:30:48 ben Exp $
*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * Conditions that cause the getCacheManager()->AutoClean() to be called
 *
 * VM.clean.1: start of an active read() -- disabled for now.
 * VM.clean.2: When a context becomes claimable and
 *             A. is not a overmounted context and has children
 *             B. is a overmount and is the root of that overmount and
 *                CoveredContext is not referenced by the user
 * VM.clean.3: Can cleanup a overmount tree when the root node has rc=2
 *
 */
#include <config.h>

#define CERR cerr

#include <Ferris_private.hh>
#include <Cache.hh>
#include <Regex.hh>

#include <FerrisOpenSSL.hh>
#include <ContextContext.hh>
#include <SignalStreams.hh>
#include <Ferris/SyncDelayer.hh>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <iomanip>

#include <Fampp2GlibSupport.hh>

#include <unistd.h>
#include <gmodule.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_GNOMEVFS
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime-utils.h>
#endif

#ifdef HAVE_LIBMAGIC
extern "C" {
#include <magic.h>
};
#endif

#include <General.hh>
#include <Runner.hh>
#include <Resolver_private.hh>
#include <Ferrisls_AggregateData.hh>
#include <FerrisDOM.hh>

#include <Singleton.h>
#include <LokiTypeInfo.h>

// user-owner-name //
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>


#include <FerrisKDE.hh>
#include <CursorAPI.hh>

#include <FerrisAsTextPlugin.hh>
#include <FerrisCreationPlugin.hh>

#include <Personalities.hh>
#include <Trimming.hh>

// medallion notification
#include <PluginOutOfProcNotificationEngine.hh>

//
// To make the XSD schema types we need to know what lexicon classes we have
//
#include <Indexing/IndexPrivate.hh>

/*
 * Have to free PCCTS mounts top down because of InOrder child lists and such.
 */
//#include <plugins/context/pccts/PcctsChildContext.hh>

//
// getSchema()
//
#include <SchemaSupport.hh>


// TESTING ONLY
#include <FilteredContext_private.hh>

#include <FerrisSTL.hh>

#include <Medallion_private.hh>

#ifdef HAVE_LIBTEXTCAT
  extern "C" {
  #include <textcat.h>
  };
#endif

#include <Ferris/FerrisGPG_private.hh>
#include <FerrisEAGeneratorPlugin_private.hh>

#include <CacheManager_private.hh>

#include <Ferris/FerrisBoost.hh>

#include <Ferris/EAIndexerMetaInterface.hh>
#include <Ferris/MetadataServer_private.hh>

#ifdef HAVE_DBUS
#include "DBus_private.hh"
#endif

#ifdef FERRIS_HAVE_LIBZ
#include <zlib.h>
#endif

#include <FerrisSemantic.hh>

using namespace std;
using namespace Ferris::RDFCore;
// using RDFCore::fh_node;
// using RDFCore::fh_model;

// json
#include "FerrisQt_private.hh"

namespace Ferris
{
    // const char* getDotFerrisPathCSTR()
    // {
    //     static std::string s = getDotFerrisPath();
    //     return s.c_str();
    // }
    
    
    std::string getDotFerrisPath()
    {
//        return "~/.ferris/";
        
        static string ret = "";
        static bool v = true;
        if( v )
        {
            v = false;
            ret = Shell::getHomeDirPath_nochecks() + "/.ferris/";
            char* p = getenv("LIBFERRIS_DOT_FERRIS_PATH");
            if( p )
            {
                ret = p;
                ret += "/";
            }
        }
        
        return ret;
    }
    std::string getDotFerrisPartialMatchPath()
    {
        return "/.ferris/";
    }
    

    
    const string FERRIS_CONFIG_APPS_DIR = "/.ferris/apps.db";
    const string FERRIS_CONFIG_EVENT_DIR = "/.ferris/eventbind.db";
    const string FERRIS_CONFIG_MIMEBIND_DIR = "/.ferris/mimebind.db";

    const string EANAME_SL_EMBLEM_ID_PREKEY = "emblem:id-";
    const string EANAME_SL_EMBLEM_ID_FUZZY_PREKEY = "emblem:id-fuzzy-";
    const string EANAME_SL_EMBLEM_PREKEY = "emblem:has-";
    const string EANAME_SL_EMBLEM_TIME_PREKEY = "emblem:";
    const string EANAME_SL_EMBLEM_FUZZY_PREKEY = "emblem:has-fuzzy-";

    static bool ForceOutOfProcessMetadataOff = false;
    
    bool getForceOutOfProcessMetadataOff()
    {
        return ForceOutOfProcessMetadataOff;
    }
    bool setForceOutOfProcessMetadataOff( bool v )
    {
        bool ret = ForceOutOfProcessMetadataOff;
        ForceOutOfProcessMetadataOff = v;
        return ret;
    }
    
    
    bool tryToUseOutOfProcessMetadataServer()
    {
        if( getForceOutOfProcessMetadataOff() )
            return false;

        static const gchar* ENVVAR = g_getenv ("LIBFERRIS_USE_OUT_OF_PROCESS_METADATA");
        if( ENVVAR )
            return true;
        
        static bool ret = false;
        static bool v = true;
        if( v )
        {
            v = false;

            string path = Shell::getHomeDirPath() + "/.ferris/use-out-of-process-metadata";
            int rc = access( path.c_str(), F_OK );
            if( !rc )
            {
                LG_MDSERV_D << "Using some external (out of process) metadata servers"
                            << " because file exists:" << path
                            << endl;
                ret = true;
            }
        }
        
        return ret;
    }
    
    
    /**
     * If an attribute with the given eaname is bound then prepend it to the
     * rea list
     */
    static void
    adjustREAForAttributeIfPresent( Context* c, std::string& rea, const std::string& eaname )
    {
//         cerr << "adjustREAForAttributeIfPresent() eaname:" << eaname
//              << " is there false:" << c->isAttributeBound( eaname, false )
//              << " is there true:" << c->isAttributeBound( eaname, true )
//              << endl;
        
        if( c->isAttributeBound( eaname, true ) )
        {
            rea = eaname + "," + rea;
        }
    }
    
    /**
     * If we are under one of the dotfile db4 files then we monster the
     * recommended EA, otherwise we just use the given value.
     *
     * methods should just return adjustRecommendedEAForDotFiles(this, "name,...");
     * at the end to adjust for db4 config file viewing.
     */
    std::string adjustRecommendedEAForDotFiles( Context* c, const std::string& s )
    {
        string path = c->getDirPath();

        if( string::npos != path.find( "/.ferris/" ) )
        {
            if( string::npos != path.find( FERRIS_CONFIG_APPS_DIR ))
            {
                return "name,ferris-exe,ferris-scheme,ferris-iconname,"
                    "ferris-ignore-selection,ferris-handles-urls,ferris-opens-many";
            }
            else if( string::npos != path.find( FERRIS_CONFIG_MIMEBIND_DIR ))
            {
                return "name,ferris-appname,ferris-iconname";
            }
            else if( string::npos != path.find( "/.ferris/file-clipboard.db/" ))
            {
                return "name,undo,redo";
            }
            else if( string::npos != path.find( "/.ferris/file-clipboard.db" ))
            {
                return "name,action,"
                    "commands-use-sloth,commands-use-auto-close,commands-use-extra-options";
            }
            else if( string::npos != path.find( "/.ferris/schema" ))
            {
                return "name,uname,ferrisenum,ferrisname,possiblesort,defaultsort,defaultvalue,description,uuid";
            }
        }

        if( string::npos != path.find( "/.ego/" ) )
        {
            if( string::npos != path.find( "/.ego/sortmark" ))
            {
                return "name,content";
            }
            else if( string::npos != path.find( "/.ego/openurl" ))
            {
                return "content,mtime-display";
            }
            else if( string::npos != path.find( "/.ego/filters" ))
            {
                return "name,content";
            }
            else if( string::npos != path.find( "/.ego/bookmarks" ))
            {
                return "name,content,ferris-iconname";
            }
        }

        string ret = s;

        static stringlist_t autorea =
            Util::parseCommaSeperatedList(
                getConfigString( FDB_GENERAL,
                                 CFG_ATTRIBUTES_TO_AUTO_REA_K,
                                 CFG_ATTRIBUTES_TO_AUTO_REA_DEFAULT ));
        static bool autorea_is_empty = autorea.empty();
        if( !autorea_is_empty )
        {
            for( stringlist_t::iterator si = autorea.begin(); si != autorea.end(); ++si )
            {
                const std::string tmp = *si;
                
                if( !tmp.empty() )
                    adjustREAForAttributeIfPresent( c, ret, tmp );
            }
        }
        
        
        return ret;
    }
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


/**
 * iterate over the range of a map
 */
class EAGenFactorys_iterator_class
{
    typedef EAGenFactorys_iterator_class _Self;
    typedef Context::m_StatefullEAGenFactorys_t::const_iterator DelegateIterClass;
    
public:
    Context* c;
    bool m_DelegateSF;
    DelegateIterClass m_delegate;

    void shiftIterator( int n )
        {
//            cerr << "shiftIterator n:" << n << " m_DelegateSF:" << m_DelegateSF << endl;

            // Quite a common case.
            if( n == 1 && m_DelegateSF )
            {
                DelegateIterClass e = c->m_StatefullEAGenFactorys.end();
                advance( m_delegate, n );
                if( m_delegate == e )
                {
                    m_DelegateSF = false;
                    m_delegate = c->getStatelessEAGenFactorys().begin();
                }
                return;
            }
            
            if( n > 0 && m_DelegateSF )
            {
                DelegateIterClass e = c->m_StatefullEAGenFactorys.end();
                int d = distance( m_delegate, e );
//                 cerr << "    shiftIterator d:" << d
//                      << " del==end:" << (m_delegate == e )
//                      << endl;
                if( n >= d )
                {
//                    cerr << "    shiftIterator!! d:" << d << endl;
                    n -= d;
                    m_DelegateSF = false;
                    m_delegate = c->getStatelessEAGenFactorys().begin();
                }
            }
            advance( m_delegate, n );
        }
    

    typedef std::iterator_traits<DelegateIterClass>       traits_type;
    typedef traits_type::iterator_category                iterator_category;
    typedef traits_type::value_type                       value_type;
    typedef long                                          difference_type;
    typedef value_type                                    pointer;
    typedef value_type                                    reference;
    
    
    EAGenFactorys_iterator_class( Context* c, bool isEnd = false )
        :
        c( c )
        {
            if( isEnd )
            {
                m_delegate = c->getStatelessEAGenFactorys().end();
                m_DelegateSF = false;
            }
            else
            {
//                 cerr << "EAGenFactorys_iterator_class(ne) sf.sz"
//                      << c->m_StatefullEAGenFactorys.size()
//                      << " sl.sz:" << c->getStatelessEAGenFactorys().size()
//                      << endl;
                if( c->m_StatefullEAGenFactorys.empty() )
                {
                    m_delegate = c->getStatelessEAGenFactorys().end();
                    m_DelegateSF = false;
                }
                else
                {
                    m_delegate = c->m_StatefullEAGenFactorys.begin();
                    m_DelegateSF = true;
                }
            }
        }

    reference operator*() const
        {
            return *m_delegate;
        }
    
    pointer operator->()
        {
            return *m_delegate;
        }

    _Self& operator++()
        {
            shiftIterator(1);
            return *this;
        }
    
    _Self  operator++(int)
        {
            _Self tmp(*this);
            shiftIterator(1);
            return tmp;
        }
    
    _Self& operator--()
        {
            shiftIterator(-1);
            return *this;
        }

    _Self  operator--(int)
        {
            _Self tmp(*this);
            shiftIterator(-1);
            return tmp;
        }

    _Self
    operator+(difference_type n) const
        {
            _Self tmp(*this);
            tmp.shiftIterator(n);
            return tmp;
        }
    
    _Self&
    operator+=(difference_type n)
        {
            shiftIterator(n);
            return *this;
        }

    _Self
    operator-(difference_type n) const
        {
            _Self tmp(*this);
            tmp.shiftIterator( ForceNegative(n) );
            return tmp;
        }

    _Self&
    operator-=(difference_type n)
        {
            shiftIterator( ForceNegative(n) );
            return *this;
        }
};

typedef EAGenFactorys_iterator_class EAGenFactorys_iterator;

bool operator==(const EAGenFactorys_iterator_class& x,
                const EAGenFactorys_iterator_class& y)
{
    return x.m_DelegateSF == y.m_DelegateSF && x.m_delegate == y.m_delegate;
}
    
bool operator!=(const EAGenFactorys_iterator_class& x,
                const EAGenFactorys_iterator_class& y) {
    return !(x == y);
}


string makeFerrisPluginPath( const std::string& dir, const std::string& libname )
{
    stringstream ret;

    static const gchar* LIBFERRIS_PLUGIN_PREFIX = g_getenv ("LIBFERRIS_PLUGIN_PREFIX");
    if( LIBFERRIS_PLUGIN_PREFIX )
        ret << LIBFERRIS_PLUGIN_PREFIX << "/";
    else
        ret << PREFIX << "/lib/ferris/plugins/";
    ret << dir;
    ret << "/";
    if( !libname.empty() )
    {
        ret << "/" << libname;
    }
//    cerr << "makeFerrisPluginPath() ret:" << ret.str() << endl;
    return ret.str();
}


    /**
     * Setup eaindex plugin factories
     */
    void ensureEAIndexPluginFactoriesAreLoaded()
        {
            static bool v = true;
            if( v )
            {
                v = false;

//                string EAIndexersPath = PREFIX + "/lib/ferris/plugins/eaindexers/";
                string EAIndexersPath = makeFerrisPluginPath( "eaindexers" );
                
                DIR *d; struct dirent *e;
                if ((d = opendir (EAIndexersPath.c_str())) == NULL)
                {
                    LG_PLUGIN_ER << "Can not open system plugin dir :" << EAIndexersPath << endl;
                }
                else
                {
                    while ((e = readdir (d)) != NULL)
                    {
                        string fn = e->d_name;
                        LG_PLUGIN_I << "Found:" << fn << endl;
                        if( ends_with( fn, "factory.so" ) )
                        {
                            try
                            {
                                LG_PLUGIN_I << "Loading plugin:" << fn << endl;
                                ostringstream ss;
                                ss << EAIndexersPath << fn;

                                GModule*   ghandle;
                                ghandle = g_module_open ( tostr(ss).c_str(), G_MODULE_BIND_LAZY);
                                if (!ghandle)
                                {
                                    ostringstream ss;
                                    ss  << "Error, unable to open module file, %s"
                                        << g_module_error ()
                                        << endl;
                                    LG_PLUGIN_I << tostr(ss) << endl;
                                    cerr << tostr(ss) << endl;
                                    Throw_GModuleOpenFailed( tostr(ss), 0 );
                                }


                                void (*SetupPluginFunction)() = 0;
                                if (!g_module_symbol (ghandle, "SetupPlugin", 
                                                      (gpointer*)&SetupPluginFunction))
                                {
                                    ostringstream ss;
                                    ss  << "Error, unable to resolve MakeFactory in module file, %s"
                                        << g_module_error()
                                        << endl;
                                    LG_PLUGIN_I << tostr(ss) << endl;
                                    cerr << tostr(ss) << endl;
                                    Throw_GModuleOpenFailed( tostr(ss), 0 );
                                }

                                SetupPluginFunction();
                            }
                            catch( GModuleOpenFailed& e )
                            {
                                LG_PLUGIN_ER << "Failed to load plugin:" << fn << endl;
                            }
                            catch( exception& e )
                            {
                                LG_PLUGIN_ER << "Failed to load plugin:" << fn
                                             << " e:" << e.what() << endl;
                            }
                        }
                    }
                    closedir (d);
                }
            }
        }


    /**
     * Setup fulltext plugin factories
     */
    void ensureFulltextIndexPluginFactoriesAreLoaded()
        {
            static bool v = true;
            if( v )
            {
                v = false;

//                string FTXIndexersPath = PREFIX + "/lib/ferris/plugins/fulltextindexers/";
                string FTXIndexersPath = makeFerrisPluginPath( "fulltextindexers" );

                DIR *d; struct dirent *e;
                if ((d = opendir (FTXIndexersPath.c_str())) == NULL)
                {
                    LG_PLUGIN_ER << "Can not open system plugin dir :" << FTXIndexersPath << endl;
                }
                else
                {
                    while ((e = readdir (d)) != NULL)
                    {
                        string fn = e->d_name;
                        LG_PLUGIN_I << "Found:" << fn << endl;
                        if( ends_with( fn, "factory.so" ) )
                        {
                            try
                            {
                                LG_PLUGIN_I << "Loading plugin:" << fn << endl;
                                ostringstream ss;
                                ss << FTXIndexersPath << fn;

                                GModule*   ghandle;
                                ghandle = g_module_open ( tostr(ss).c_str(), G_MODULE_BIND_LAZY);
                                if (!ghandle)
                                {
                                    ostringstream ss;
                                    ss  << "Error, unable to open module file, %s"
                                        << g_module_error ()
                                        << endl;
                                    LG_PLUGIN_I << tostr(ss) << endl;
                                    cerr << tostr(ss) << endl;
                                    Throw_GModuleOpenFailed( tostr(ss), 0 );
                                }

                                LG_PLUGIN_I << "Loading2 plugin:" << fn << endl;

                                void (*SetupPluginFunction)() = 0;
                                if (!g_module_symbol (ghandle, "SetupPlugin", 
                                                      (gpointer*)&SetupPluginFunction))
                                {
                                    ostringstream ss;
                                    ss  << "Error, unable to resolve MakeFactory in module file, %s"
                                        << g_module_error()
                                        << endl;
                                    LG_PLUGIN_I << tostr(ss) << endl;
                                    cerr << tostr(ss) << endl;
                                    Throw_GModuleOpenFailed( tostr(ss), 0 );
                                }

                                LG_PLUGIN_I << "Loading3 plugin:" << fn << endl;
                                SetupPluginFunction();
                                
                                LG_PLUGIN_I << "Loaded plugin:" << fn << endl;
                            }
                            catch( GModuleOpenFailed& e )
                            {
                                LG_PLUGIN_ER << "Failed to load plugin:" << fn << endl;
                            }
                            catch( exception& e )
                            {
                                LG_PLUGIN_ER << "Failed to load plugin:" << fn
                                             << " e:" << e.what() << endl;
                            }
                        }
                    }
                    closedir (d);
                }
            }
        }


    namespace 
    {
        FullTextIndex::MetaFullTextIndexerInterface* CreateFullTextIndexerGeneric( const char* s )
        {
            return FullTextIndex::CreateFullTextIndexerFromLibrary( s );
        }
    };
    
    void RegisterFulltextIndexPluginAlias( const char* libname,
                                           const std::string& ferristype )
    {
        const std::string MetaIndexClassName = ferristype;

        typedef Loki::Functor< FullTextIndex::MetaFullTextIndexerInterface*, ::Loki::NullType > CreateFunction_t;

        CreateFunction_t f = Loki::BindFirst(
            Loki::Functor< FullTextIndex::MetaFullTextIndexerInterface*, LOKI_TYPELIST_1( const char* ) >
            ( &CreateFullTextIndexerGeneric ), libname );
        
        bool reged = FullTextIndex::MetaFullTextIndexerInterfaceFactory::Instance().
            Register( MetaIndexClassName, f );
        bool regedx = FullTextIndex::appendToMetaFullTextIndexClassNames( MetaIndexClassName );
    }
    
    
    bool RegisterFulltextIndexPlugin( const char* libname,
                                      const std::string& ferristype,
                                      const std::string& xsd,
                                      bool requiresNativeKernelDrive,
                                      const std::string& simpleTypes )
    {
        RegisterFulltextIndexPluginAlias( libname, ferristype );
        
        bool r = RegisterCreationModule(
            "libcreationfulltextindexgeneric.so",
            ferristype,
            xsd,
            requiresNativeKernelDrive,
            simpleTypes );
        return r;
    }



    namespace 
    {
        EAIndex::MetaEAIndexerInterface* CreateEAIndexerGeneric( const char* s )
        {
            return EAIndex::CreateEAIndexerFromLibrary( s );
        }
    };

    void RegisterEAIndexPluginAlias( const char* libname,
                                     const std::string& ferristype )
    {
        const std::string MetaIndexClassName = ferristype;

        typedef Loki::Functor< EAIndex::MetaEAIndexerInterface*, ::Loki::NullType > CreateFunction_t;

        CreateFunction_t f = Loki::BindFirst(
            Loki::Functor< EAIndex::MetaEAIndexerInterface*, LOKI_TYPELIST_1( const char* ) >
            ( &CreateEAIndexerGeneric ), libname );

        
        EAIndex::MetaEAIndexerInterfaceFactory::Instance().
            Register( MetaIndexClassName, f );
        EAIndex::appendToMetaEAIndexClassNames( MetaIndexClassName );
    }
    
    
    bool RegisterEAIndexPlugin( const char* libname,
                                const std::string& ferristype,
                                const std::string& xsd,
                                bool requiresNativeKernelDrive,
                                const std::string& simpleTypes )
    {
        RegisterEAIndexPluginAlias( libname, ferristype );
        
        bool r = RegisterCreationModule(
            "libcreationeaindexgeneric.so",
            ferristype,
            xsd,
            requiresNativeKernelDrive,
            simpleTypes );
    }
    
    
    
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////



    void
    Context::setContext( fh_context parent, const std::string& rdn )
    {
//        cerr << "Context::setContext() this:" << (void*)this << " parent:" << (void*)GetImpl(parent) << endl;
        ParentContext = parent;
        DirName = rdn;
        setAttributeContext( GetImpl(ParentContext) );
        m_forcePassiveViewCacheIsValid = false;
    }

    /**
     * If a VFS module supports renaming contexts based on changing their "name"
     * attribute, then this method should return true;
     */
    bool
    Context::supportsRename()
    {
        return false;
    }

    /**
     * Some extra methods are needed to allow the VM to reclaim a module's
     * context objects and then reify them back on demand. If your module
     * supports these extra operations then return true.
     *
     * default false
     */
    bool
    Context::supportsReClaim()
    {
        return false;
    }

    /**
     * If the module emits events when interesting things happen then
     * override this and return true.
     *
     * @default false
     * @see isActiveView()
     * @see getForcePassiveView()
     */
    bool
    Context::supportsMonitoring()
    {
        return false;
    }

    bool
    Context::disableOverMountingForContext()
    {
        return false;
    }
    
    /**
     * If a subclass overrides priv_getSubContext() to check and make a subcontext
     * without reading the whole dir (see native module) then it should override
     * this method and return true.
     *
     * @return true if supports, false by default
     */
    bool
    Context::priv_supportsShortCutLoading()
    {
        return false;
    }
    
    bool
    Context::supportsShortCutLoading()
    {
        return getOverMountContext()->priv_supportsShortCutLoading();
    }

    
    
    /**
     * Modules involving network IO can override this method and return true.
     *
     * Note that this effects how download-if-mtime-since is treated. For local
     * filesystems the mtime test is done automatically in getIStream(). For
     * remote filesystems the mtime is collected but not directly tested to allow
     * the plugin to test it using the underlying net protocol.
     *
     * Call 
     *
     * @default false
     */
    bool
    Context::isRemote()
    {
        return false;
    }
    
    
    


/**
 * Try to guess how many subcontexts this context has. Some context classes
 * wont allow this operation, in which case they will return 0.
 *
 * This method was added to allow contexts to give a guess for how many
 * children they have. For some contexts this is a relatively fast operation
 * and so a gauge can be presented to the user to show feedback during the
 * read() operation.
 *
 * @see priv_guessSize()
 * @return Either 0 if there are no subcontexts or no guess can be made, or a
 *         rough idea of how many subcontexts *may* exist.
 */
    long
    Context::guessSize()
        throw()
    {
        return getOverMountContext()->priv_guessSize();
    }


/**
 * Default implementation returns 0 so that subclasses of context class do not have
 * to implement this method unless it makes sense for them.
 *
 * @see guessSize()
 * @return 0 always
 */
    long
    Context::priv_guessSize() throw()
    {
        return 0; 
    }

    fh_context SL_SubCreate_alwaysThrow( fh_context c, fh_context md )
    {
        fh_stringstream ss;
        ss << "SL_SubCreate_alwaysThrow called, something major went wrong."
           << " url:" << c->getURL()
           << endl;
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
    }


    std::string getStrSubCtx( const std::string& earl,
                              std::string def,
                              bool getAllLines,
                              bool throw_for_errors )
    {
        try
        {
            fh_context c = Resolve( earl );
            return getStrAttr( c, "content", def, getAllLines, throw_for_errors );
        }
        catch( ... )
        {
            if( throw_for_errors )
                throw;
        }
        return def;
            
    }

    std::string getStrSubCtx( const std::string& earl,
                              std::string subname,
                              std::string def,
                              bool getAllLines,
                              bool throw_for_errors )
    {
        stringstream ss;
        ss << earl << "/" << subname;
        return getStrSubCtx( tostr(ss), def, getAllLines, throw_for_errors );
    }
    
    
    string getStrSubCtx( fh_context c,
                         string subname,
                         string def,
                         bool getAllLines,
                         bool throw_for_errors ) 
    {
        string ret = def;

        try
        {
            fh_context child = c->getSubContext( subname );
            ret = getStrAttr( child, "", def, getAllLines );
        }
        catch(...)
        {
            if( throw_for_errors )
                throw;
        }
        return ret;
    }

    /********************************************************************************/


    

    
    /* FIXME */
    fh_context SL_SubCreate_text ( fh_context c, fh_context md )
    {
        fh_context newc = c->SubCreate_file( c, md );
        return newc;
    }
    
    
    
    fh_context SL_SubCreate_file( fh_context c, fh_context md )
    {
        LG_CTX_D << "SL_SubCreate_file() rdn:" << getStrSubCtx( md, "name", "N/A" ) << endl;
        return c->SubCreate_file( c, md );
    }
    fh_context SL_SubCreate_dir( fh_context c, fh_context md )
    {
        LG_CTX_D << "SL_SubCreate_dir() rdn:" << getStrSubCtx( md, "name", "N/A" ) << endl;
        return c->SubCreate_dir( c, md );
    }

    fh_context
    Context::SubCreate_file( fh_context c, fh_context md )
    {
        LG_CTX_D << "Context::SubCreate_file() c:" << c->getURL() << endl;
        Throw_FerrisCreateSubContextNotSupported("",this);
    }
    fh_context
    Context::SubCreate_dir( fh_context c, fh_context md )
    {
        LG_CTX_D << "Context::SubCreate_dir() c:" << c->getURL() << endl;
        Throw_FerrisCreateSubContextNotSupported("",this);
    }

    fh_context SL_SubCreate_ea( fh_context c, fh_context md )
    {
        return c->SubCreate_ea( c, md );
    }


    
    fh_context
    Context::SubCreate_ea( fh_context c, fh_context md )
    {
        LG_CTX_D << "Context::SubCreate_ea(1)" << endl;
        string error_desc = "";

        string explicitShortName = getStrSubCtx( md, "explicit-plugin-name", "" );
        
        for( int Pri = AttributeCreator::CREATE_PRI_MAX_INTERNAL_USE_ONLY;
             Pri >= AttributeCreator::CREATE_PRI_NOT_SUPPORTED; --Pri )
        {
//                 for( EAGenFactorys_t::const_iterator iter = getEAGenFactorys().begin();
//                  iter != getEAGenFactorys().end();
//                  ++iter )
            ensureEAGenFactorysSetup();
            EAGenFactorys_iterator iterend = EAGenFactorys_iterator( this, true );
            for( EAGenFactorys_iterator iter = EAGenFactorys_iterator( this );
                 iter != iterend; ++iter )
            {
                try
                {
                    string rdn = getStrSubCtx( md, "name", "", true, true );

                    if( !explicitShortName.empty() )
                    {
                        LG_ATTR_D << "explicitShortName:" << explicitShortName
                                  << " eaname:" << rdn
                                  << endl;
                        fh_MatchedEAGeneratorFactory f = *iter;
                        if( StaticGModuleMatchedEAGeneratorFactory* ff
                            = dynamic_cast<StaticGModuleMatchedEAGeneratorFactory*>( GetImpl(f) ) )
                        {
                            if( ff->getShortName() != explicitShortName )
                                continue;
                        }
                    }
                    
                    if( (*iter)->getCreatePriority() == Pri 
                        && (*iter)->supportsCreateForContext(c) )
                    {
                        fh_attribute attr = (*iter)->CreateAttr( c, rdn, md );
                        LG_CTX_D << "Context::SubCreate_ea(l.a)" << endl;

//                         // DEBUG
//                         {
//                             fh_MatchedEAGeneratorFactory f = *iter;
//                             if( StaticGModuleMatchedEAGeneratorFactory* ff
//                                 = dynamic_cast<StaticGModuleMatchedEAGeneratorFactory*>( GetImpl(f) ) )
//                             {
//                                 LG_CTX_D << "Context::SubCreate_ea(l.name):"
//                                          << ff->getShortName() << endl;
//                             }
//                         }
                        
                        
                        string v = getStrSubCtx( md, "value", "", true, false );
                        LG_CTX_D << "Context::SubCreate_ea(l.b) v:" << v << endl;

                        if( v.length() )
                        {
                            fh_iostream oss = attr->getIOStream( ios::in|ios::out|ios::trunc );
                            oss << v << flush;
                        }
                        LG_CTX_D << "Context::SubCreate_ea(l.c)" << endl;
                        return c;
                    }
                }
                catch( exception& e )
                {
                    LG_ATTR_D << "Context::SubCreate_ea e:" << e.what() << endl;
                    error_desc = e.what();
//                     fh_stringstream ss;
//                     ss << "SubCreate_ea() failed creating the EA"
//                        << " url:" << c->getURL()
//                        << " rdn:" << getStrSubCtx( md, "name", "" )
//                        << " e:" << e.what()
//                        << endl;
//                     cerr << tostr(ss) << endl;
//                     Throw_FerrisCreateAttributeFailed( tostr(ss), GetImpl(c) );
                }
            }
        }
        
        fh_stringstream ss;
        ss << "SubCreate_ea() could not find a functional ea generator to create the new EA."
           << " url:" << c->getURL()
           << " rdn:" << getStrSubCtx( md, "name", "" )
           << " e:" << error_desc
           << endl;
        LG_CTX_D << ss.str() << endl;
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
    }

    static stringlist_t& getExtraGenerateSchemaSimpleTypes()
    {
        static stringlist_t o;
        return o;
    }

    bool appendExtraGenerateSchemaSimpleTypes( const std::string& s )
    {
        getExtraGenerateSchemaSimpleTypes().push_back( s );
        return true;
    }
    
    fh_istream
    Context::generateSchema( CreateSubContextSchemaPart_t& m )
    {
        fh_stringstream ss;
        typedef CreateSubContextSchemaPart_t m_t;
        typedef m_t::iterator MI;

        ss << "<schema name=\"http://www.monkeyiq.org/create.xsd\">" << endl
           << endl;

        for( stringlist_t::iterator ci = getExtraGenerateSchemaSimpleTypes().begin();
             ci != getExtraGenerateSchemaSimpleTypes().end(); ++ci )
        {
            ss << *ci << endl;
        }
        
        ss << ""
           << "<simpleType name=\"StringT\">" << endl
           << "    <restriction base=\"string\">" << endl
           << "    </restriction>" << endl
           << "</simpleType>" << endl
           << endl
           << "<simpleType name=\"StringListT\">" << endl
           << "    <list itemType=\"StringT\"/>" << endl
           << "    <restriction>" << endl
           << "        <length value=\"1\"/>" << endl
           << "    </restriction>" << endl
           << "</simpleType>" << endl
           << endl
           << "<simpleType name=\"EditSQLColumnsT\">" << endl
           << "    <list itemType=\"KeyValueStringT\"/>" << endl
           << "    <restriction>" << endl
           << "        <length value=\"1\"/>" << endl
           << "    </restriction>" << endl
           << "</simpleType>" << endl
           << endl
           << "<simpleType name=\"AttributeNameT\">" << endl
           << "    <restriction base=\"string\">" << endl
           << "    </restriction>" << endl
           << "</simpleType>" << endl
           << endl
           << "<simpleType name=\"AttributeNameListT\">" << endl
           << "    <list itemType=\"AttributeNameT\"/>" << endl
           << "</simpleType>" << endl
           << endl
//            << "<simpleType name=\"ColorSpaceT\">" << endl
//            << "    <restriction base=\"string\">" << endl
//            << "        <enumeration value=\"RGB\"/>" << endl
//            << "        <enumeration value=\"Gray\"/>" << endl
//            << "        <enumeration value=\"YUV\"/>" << endl
//            << "        <enumeration value=\"CMYK\"/>" << endl
//            << "    </restriction>" << endl
//            << "</simpleType>" << endl
//            << endl
//            << "<simpleType name=\"ColorSpaceListT\">" << endl
//            << "    <list itemType=\"ColorSpaceT\"/>" << endl
//            << "    <restriction>" << endl
//            << "        <length value=\"1\"/>" << endl
//            << "    </restriction>" << endl
//            << "</simpleType>" << endl
//            << endl
//            << "<simpleType name=\"JPEGColorSpaceT\">" << endl
//            << "    <restriction base=\"string\">" << endl
//            << "        <enumeration value=\"RGB\"/>" << endl
//            << "        <enumeration value=\"Gray\"/>" << endl
//            << "        <enumeration value=\"YUV\"/>" << endl
//            << "        <enumeration value=\"CMYK\"/>" << endl
//            << "        <enumeration value=\"YCCK\"/>" << endl
//            << "    </restriction>" << endl
//            << "</simpleType>" << endl
//            << endl
//            << "<simpleType name=\"JPEGColorSpaceListT\">" << endl
//            << "    <list itemType=\"JPEGColorSpaceT\"/>" << endl
//            << "    <restriction>" << endl
//            << "        <length value=\"1\"/>" << endl
//            << "    </restriction>" << endl
//            << "</simpleType>" << endl
//            << endl
//            << endl
//            << "<simpleType name=\"LicenseT\">" << endl
//            << "    <restriction base=\"string\">" << endl
//            << "        <enumeration value=\"GPL\"/>" << endl
//            << "        <enumeration value=\"LGPL\"/>" << endl
//            << "        <enumeration value=\"BSD\"/>" << endl
//            << "    </restriction>" << endl
//            << "</simpleType>" << endl
//            << endl
//            << "<simpleType name=\"LicenseListT\">" << endl
//            << "    <list itemType=\"LicenseT\"/>" << endl
//            << "    <restriction>" << endl
//            << "        <length value=\"1\"/>" << endl
//            << "    </restriction>" << endl
//            << "</simpleType>" << endl
//            << endl
//            << endl
//            << "<simpleType name=\"SQLStringT\">" << endl
//            << "    <restriction base=\"string\">" << endl
//            << "    </restriction>" << endl
//            << "</simpleType>" << endl
//            << "<simpleType name=\"SQLDateT\">" << endl
//            << "    <restriction base=\"string\">" << endl
//            << "    </restriction>" << endl
//            << "</simpleType>" << endl
//            << "<simpleType name=\"SQLTimeStampT\">" << endl
//            << "    <restriction base=\"string\">" << endl
//            << "    </restriction>" << endl
//            << "</simpleType>" << endl
//            << "<simpleType name=\"SQLDoubleT\">" << endl
//            << "    <restriction base=\"string\">" << endl
//            << "    </restriction>" << endl
//            << "</simpleType>" << endl
//            << endl
//            << endl
//            << "<simpleType name=\"SocketSSLT\">" << endl
//            << "    <restriction base=\"string\">" << endl
//            << "        <enumeration value=\"None\"/>" << endl
//            << "        <enumeration value=\"SSL2\"/>" << endl
//            << "        <enumeration value=\"SSL3\"/>" << endl
//            << "        <enumeration value=\"TLS\"/>" << endl
//            << "    </restriction>" << endl
//            << "</simpleType>" << endl
//            << endl
//            << "<simpleType name=\"SocketSSLListT\">" << endl
//            << "    <list itemType=\"SocketSSLT\"/>" << endl
//            << "    <restriction>" << endl
//            << "        <length value=\"1\"/>" << endl
//            << "    </restriction>" << endl
//            << "</simpleType>" << endl
//            << "<simpleType name=\"TCPIPProtocolT\">" << endl
//            << "    <restriction base=\"string\">" << endl
//            << "        <enumeration value=\"TCP\"/>" << endl
//            << "        <enumeration value=\"UDP\"/>" << endl
//            << "        <enumeration value=\"ICMP\"/>" << endl
//            << "    </restriction>" << endl
//            << "</simpleType>" << endl
//            << endl
//            << "<simpleType name=\"TCPIPProtocolListT\">" << endl
//            << "    <list itemType=\"TCPIPProtocolT\"/>" << endl
//            << "    <restriction>" << endl
//            << "        <length value=\"1\"/>" << endl
//            << "    </restriction>" << endl
//            << "</simpleType>" << endl
//            << endl
//            << endl
           << "<simpleType name=\"LuceneStemmerT\">" << endl
           << "    <restriction base=\"string\">" << endl
           << "        <enumeration value=\"Porter\"/>" << endl
           << "    </restriction>" << endl
           << "    <restriction base=\"string\">" << endl
           << "        <enumeration value=\"none\"/>" << endl
           << "    </restriction>" << endl
           << "</simpleType>" << endl
           << "<simpleType name=\"LuceneStemmerListT\">" << endl
           << "    <list itemType=\"LuceneStemmerT\"/>" << endl
           << "    <restriction>" << endl
           << "        <length value=\"1\"/>" << endl
           << "    </restriction>" << endl
           << "</simpleType>" << endl
           << endl
           << endl
           << "<simpleType name=\"StemmerT\">" << endl
           << "    <restriction base=\"string\">" << endl
           << "        <enumeration value=\"J B Lovins 68\"/>" << endl
           << "    </restriction>" << endl
           << "    <restriction base=\"string\">" << endl
           << "        <enumeration value=\"none\"/>" << endl
           << "    </restriction>" << endl
           << "</simpleType>" << endl
           << "<simpleType name=\"StemmerListT\">" << endl
           << "    <list itemType=\"StemmerT\"/>" << endl
           << "    <restriction>" << endl
           << "        <length value=\"1\"/>" << endl
           << "    </restriction>" << endl
           << "</simpleType>" << endl
           << endl
           << "<simpleType name=\"LexiconClassT\">" << endl;
        {
            stringlist_t sl = FullTextIndex::getLexiconClassNames();
            stringlist_t::iterator sli = find( sl.begin(), sl.end(), "FrontCodedBlocks (3-in-4)" );
            if( sli != sl.end() )
            {
                ss << "    <restriction base=\"string\">" << endl;
                ss << "        <enumeration value=\"" << *sli << "\"/>" << endl;
                ss << "    </restriction>" << endl;
                sl.erase( sli );
            }
            
            for( stringlist_t::iterator i = sl.begin(); i != sl.end(); ++i )
            {
//                 // don't list the alias explicitly
//                 stringlist_t::iterator anb = FullTextIndex::getLexiconAliasNames().begin();
//                 stringlist_t::iterator ane = FullTextIndex::getLexiconAliasNames().end();
//                 if( ane != find( anb, ane, *i ) )
//                     continue;
                
                ss << "    <restriction base=\"string\">" << endl;
                ss << "        <enumeration value=\"" << *i << "\"/>" << endl;
                ss << "    </restriction>" << endl;
            }
            
        }
        ss << "</simpleType>" << endl
           << "<simpleType name=\"LexiconClassListT\">" << endl
           << "    <list itemType=\"LexiconClassT\"/>" << endl
           << "    <restriction>" << endl
           << "        <length value=\"1\"/>" << endl
           << "    </restriction>" << endl
           << "</simpleType>" << endl
           << endl
           << endl
           << "<simpleType name=\"LexiconClassUncompressedFirstT\">" << endl;
        {
            stringlist_t sl = FullTextIndex::getLexiconClassNames();
            stringlist_t::iterator sli = find( sl.begin(), sl.end(), "Uncompressed (db4 hash)" );
            if( sli != sl.end() )
            {
                ss << "    <restriction base=\"string\">" << endl;
                ss << "        <enumeration value=\"" << *sli << "\"/>" << endl;
                ss << "    </restriction>" << endl;
                sl.erase( sli );
            }
            
            for( stringlist_t::iterator i = sl.begin(); i != sl.end(); ++i )
            {
                ss << "    <restriction base=\"string\">" << endl;
                ss << "        <enumeration value=\"" << *i << "\"/>" << endl;
                ss << "    </restriction>" << endl;
            }
            
        }
        ss << "</simpleType>" << endl
           << "<simpleType name=\"LexiconClassUncompressedFirstListT\">" << endl
           << "    <list itemType=\"LexiconClassUncompressedFirstT\"/>" << endl
           << "    <restriction>" << endl
           << "        <length value=\"1\"/>" << endl
           << "    </restriction>" << endl
           << "</simpleType>" << endl
           << endl
           << endl
           << "<simpleType name=\"ReverseLexiconClassUncompressedFirstT\">" << endl;
        {
            stringlist_t sl = FullTextIndex::getReverseLexiconClassNames();
            stringlist_t::iterator sli = find( sl.begin(), sl.end(), "Uncompressed (db4 hash)" );
            if( sli != sl.end() )
            {
                ss << "    <restriction base=\"string\">" << endl;
                ss << "        <enumeration value=\"" << *sli << "\"/>" << endl;
                ss << "    </restriction>" << endl;
                sl.erase( sli );
            }
            
            for( stringlist_t::iterator i = sl.begin(); i != sl.end(); ++i )
            {
                ss << "    <restriction base=\"string\">" << endl;
                ss << "        <enumeration value=\"" << *i << "\"/>" << endl;
                ss << "    </restriction>" << endl;
            }
            
        }
        ss << "</simpleType>" << endl
           << "<simpleType name=\"ReverseLexiconClassUncompressedFirstListT\">" << endl
           << "    <list itemType=\"ReverseLexiconClassUncompressedFirstT\"/>" << endl
           << "    <restriction>" << endl
           << "        <length value=\"1\"/>" << endl
           << "    </restriction>" << endl
           << "</simpleType>" << endl
           << endl
           << endl
//            << "<simpleType name=\"DB4TypeT\">" << endl
//            << "    <restriction base=\"string\">" << endl
//            << "        <enumeration value=\"Btree\"/>" << endl
//            << "        <enumeration value=\"Hash\"/>" << endl
//            << "        <enumeration value=\"Queue\"/>" << endl
//            << "        <enumeration value=\"Recno\"/>" << endl
//            << "    </restriction>" << endl
//            << "</simpleType>" << endl
//            << endl
//            << "<simpleType name=\"DB4TypeListT\">" << endl
//            << "    <list itemType=\"DB4TypeT\"/>" << endl
//            << "    <restriction>" << endl
//            << "        <length value=\"1\"/>" << endl
//            << "    </restriction>" << endl
//            << "</simpleType>" << endl
//            << endl
           << endl;

        /**
         * Allow each module to have its own XSD Simple types to use for
         * restrictions.
         */
        for( MI iter = m.begin(); iter != m.end(); ++iter )
        {
            ss << iter->second.getXSDSimpleTypes() << endl;
        }
        
        ss << endl;
        ss << endl;
        ss << ""
           << "<elementType name=\"CreateSubContext\">" << endl
           << "    <sequence minOccur=\"1\" maxOccur=\"1\">" << endl;

        for( MI iter = m.begin(); iter != m.end(); ++iter )
        {
            ss << "			<elementTypeRef name=\"" << iter->first << "\" />" << endl;
        }
        
        ss << "    </sequence>" << endl
           << "</elementType>" << endl;

        for( MI iter = m.begin(); iter != m.end(); ++iter )
        {
            ss << iter->second.getSchema() << endl;
        }
        
        ss << "</schema>" << endl;
        return ss;
    }

    void
    Context::addEAGeneratorCreateSubContextSchema( CreateSubContextSchemaPart_t& m )
    {

        for( int Pri = AttributeCreator::CREATE_PRI_MAX_INTERNAL_USE_ONLY;
             Pri >= AttributeCreator::CREATE_PRI_NOT_SUPPORTED; --Pri )
        {
//             for( EAGenFactorys_t::const_iterator iter = getEAGenFactorys().begin();
//                  iter != getEAGenFactorys().end();
//                  ++iter )
            ensureEAGenFactorysSetup();
            EAGenFactorys_iterator iterend = EAGenFactorys_iterator( this, true );
            for( EAGenFactorys_iterator iter = EAGenFactorys_iterator( this );
                 iter != iterend; ++iter )
            {
                try
                {
                    LG_CTX_D << "Context::addEAGeneratorCreateSubContextSchema() "
                             << " pri:" << (*iter)->getCreatePriority()
                             << " supports context:" << (*iter)->supportsCreateForContext(this)
                             << endl;
                    
                    if( (*iter)->getCreatePriority() == Pri
                        && (*iter)->supportsCreateForContext(this) )
                    {
                        LG_CTX_D << "Context::addEAGeneratorCreateSubContextSchema(have ea) " << endl;

                        m["ea"] = SubContextCreator(
                            SL_SubCreate_ea,
                            "	<elementType name=\"ea\">\n"
                            "		<elementType name=\"name\" default=\"new ea\">\n"
                            "			<dataTypeRef name=\"string\"/>\n"
                            "		</elementType>\n"
                            "		<elementType name=\"value\" default=\"\">\n"
                            "			<dataTypeRef name=\"string\"/>\n"
                            "		</elementType>\n"
                            "	</elementType>\n");
                        return;
                    }
                }
                catch( exception& e )
                {
                    LG_ATTR_D << "e:" << e.what() << endl;
                }
            }
        }
        
    }
    
    void
    Context::addStandardFileSubContextSchema( CreateSubContextSchemaPart_t& m )
    {
        m["file"] = SubContextCreator(SL_SubCreate_file,
            "	<elementType name=\"file\">\n"
            "		<elementType name=\"name\" default=\"new file\">\n"
            "			<dataTypeRef name=\"string\"/>\n"
            "		</elementType>\n"
            "	</elementType>\n");


        /*******************************************************************************/
        /*******************************************************************************/
        /*******************************************************************************/

        m["txt"] = SubContextCreator(SL_SubCreate_text,
/**/        "   <elementType name=\"txt\" mime-major=\"text\">\n"
/**/        "		<elementType name=\"name\" default=\"new.txt\">\n"
/**/        "			<dataTypeRef name=\"string\"/>\n"
/**/        "		</elementType>\n"
/**/        "	</elementType>\n");


        insertAbstractCreatorModules( m );
    }


    void
    Context::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
//        cerr << "url:" << getURL() << endl;
//        Ferris::BackTrace();
        Throw_FerrisCreateSubContextNotSupported("",this);
    }
    
    void
    Context::FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m,
                                              bool followOvermount )
    {
            try
            {
                fh_context c = this;
                
                if( followOvermount )
                {
                    tryToOverMount( true );
                    c = getOverMountContext();
                    
                }
                
//                 cerr << "Context::FillCreateSubContextSchemaParts() HAVE overmounter"
//                      << " c:" << c->getURL() << endl;
                
                c->priv_FillCreateSubContextSchemaParts( m );
            }
            catch( exception& e )
            {
//                 cerr << "Context::FillCreateSubContextSchemaParts() no overmounter"
//                      << " c:" << getURL() << endl;
                getOverMountContext()->priv_FillCreateSubContextSchemaParts( m );
            }
    }
    
    
    /**
     * Before calling createSubContext() call here to get a DTD for the md param
     * of createSubContext. The document format that is sought for the metadata (md)
     * for createSubContext() should be a valid document under the DTD returned
     * from this function.
     *
     * @returns Stream of the DTD that metadata should follow that is passed to
     * createSubContext()
     */
    fh_istream
    Context::getCreateSubContextSchema()
    {
        if( getOverMountContext() != this )
        {
            return getOverMountContext()->getCreateSubContextSchema();
        }
        ensureEAIndexPluginFactoriesAreLoaded();
        ensureFulltextIndexPluginFactoriesAreLoaded();
        
        CreateSubContextSchemaPart_t m;
        FillCreateSubContextSchemaParts( m );
        return generateSchema( m );
    }
    

/**
 * CreateSubContext method for subclasses to override.
 *
 * Create a new subcontext. This method always throws a not supported exception.
 * This makes read only subcontexts easy to implememnt because they dont have
 * to override this method.
 *
 * @param rdn The relative name of the new context to create
 * @param md  specific information to the creation of a subclass. For example,
 *            when creating a new subcontext in a native filesystem then
 *            the m param may contain information telling the native context
 *            if it should create a file or a directory.
 *
 * @see    createSubContext()
 * @return The new context
 * @throws FerrisCreateSubContextFailed If there was a complication creating the new
 *         context.
 * @throws FerrisCreateSubContextNotSupported If the operation is not supported by this
 *         context.
 */
    fh_context
    Context::priv_createSubContext( const string& rdn, fh_context md )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        LG_CTX_D << "Context::priv_createSubContext(1)" << endl;
        ensureEAIndexPluginFactoriesAreLoaded();
        LG_CTX_D << "Context::priv_createSubContext(1.ea factories done)" << endl;
        ensureFulltextIndexPluginFactoriesAreLoaded();
        LG_CTX_D << "Context::priv_createSubContext(2)" << endl;
        
        try
        {
            CreateSubContextSchemaPart_t m;
            typedef CreateSubContextSchemaPart_t m_t;
            typedef m_t::iterator MI;

//            cerr << "Context::priv_createSubContext(1) url:" << getURL() << endl;
//         cerr << "Context::priv_createSubContext(1) md.addr:" << toVoid(md) << endl;
//         cerr << "Context::priv_createSubContext(1) md.url:" << md->getURL() << endl;
            md->read();
//         cerr << "Context::priv_createSubContext(2) url:" << getURL() << endl;
//         md->dumpOutItems();
//         cerr << "Context::priv_createSubContext(3) url:" << getURL() << endl;

//         SubContextNames_t na = md->getSubContextNames();
//         for( SubContextNames_t::iterator iter = na.begin();
//              iter != na.end(); ++iter )
//         {
//             cerr << *iter << endl;
//             try
//             {
//                 fh_context child = md->getSubContext( *iter );
//                 fh_istream ss = child->getIStream();
//                 cerr << " : " << StreamToString(ss) << endl;
//             }
//             catch( exception& e )
//             {
//                 cerr << " .. err:" << e.what() << endl;
//             }
            
//         }


//             cerr << "Context::priv_createSubContext(top) md:" << md->getURL() << endl;
//             md->dumpOutItems();
//             try
//             {
//                 bool v = md->isSubContextBound("file");
//                 cerr << "v:" << v << endl;
//             }
//             catch(...) {
//             }

//            cerr << "Context::priv_createSubContext(2) url:" << getURL() << endl;
//             md->dumpOutItems();
//             cerr << "Context::priv_createSubContext(2.1) url:" << getURL() << endl;

            /**
             * We always save the ferris-type ea on the context itself, not in the
             * overmount to save time finding it later.
             */
            static bool attemptingToCreateFerrisType = false;

            if( md && md->isSubContextBound("ea") )
            {
                
                fh_context eac = md->getSubContext( "ea" );
                if( eac->isSubContextBound("name"))
                {
                    fh_context nc = eac->getSubContext( "name" );
                    string n = getStrAttr( nc, "content", "" );
                    if( n == "ferris-type" )
                        attemptingToCreateFerrisType = true;
                    if( n == "is-ferris-compressed" )
                        attemptingToCreateFerrisType = true;
                    if( n == "libferris-journal-changes" )
                        attemptingToCreateFerrisType = true;
                    if( eac->isSubContextBound("dont-delegate-to-overmount-context") )
                        attemptingToCreateFerrisType = true;
                }
            }
            
            LG_CTX_D << "Context::priv_createSubContext(3)" << endl;
            FillCreateSubContextSchemaParts( m, !attemptingToCreateFerrisType  );
            LG_CTX_D << "Context::priv_createSubContext(4)" << endl;
//            cerr << "Context::priv_createSubContext(3) url:" << getURL() << endl;

            typedef list< FerrisCreateAttributeFailed > createEAFailedList_t;
            createEAFailedList_t createEAFailedList;
            bool creatingEA = false;
            bool createdPrimaryObject = false;
            if( md->isSubContextBound("ea") )
            {
                creatingEA = true;
            }
            
            for( MI iter = m.begin(); iter != m.end(); ++iter )
            {
//                 cerr << "Context::priv_createSubContext() url:" << getURL()
//                      << " name:" << iter->first 
//                      << endl;
//                cerr << "create-object iter:" << iter->first << endl;
                
                try
                {
                    if( !md->isSubContextBound( iter->first ))
                    {
                        continue;
                    }
                    
                    fh_context child = md->getSubContext( iter->first );
//                     cerr << "calling perform for:" << iter->first
//                          << " child:" << child->getDirPath()
//                          << " this:" << getURL()
//                          << " subname:" << getStrSubCtx( child, "name", "" )
//                          << " subvalue:" << getStrSubCtx( child, "value", "" )
//                          << endl;
//                    md->dumpOutItems();


                    
                    LG_CTX_D << "calling perform for:" << iter->first << endl;
                    fh_context newc = iter->second.perform( this, child );
                    createdPrimaryObject = true;
                    LG_CTX_D << "called perform for:" << iter->first << endl;

//                     cerr << "addToCreateHistory(test) rdn:"
//                          << getStrSubCtx( child, "name", "" )
//                          << endl;

                    LG_CTX_D << "saving to create history..." << endl;
                    // save to history list
                    if( getStrSubCtx( child, "name", "ferris-type" ) != "ferris-type" )
                    {
//                         cerr << "calling addToCreateHistory() for:" << iter->first
//                              << " child:" << child->getDirPath()
//                              << " this:" << getURL()
//                              << " subname:" << getStrSubCtx( child, "name", "" )
//                              << " subvalue:" << getStrSubCtx( child, "value", "" )
//                              << endl;
                        addToCreateHistory( iter->first );
                    }
                    

                    LG_CTX_D << "saving advisory type info..." << endl;
                    // create advisory type info
                    if( !attemptingToCreateFerrisType && iter->first != "ea" )
                    {
                        string newctype = getStrAttr( newc, "ferris-type", "" );
                        if( newctype.empty() && iter->first != "file" && iter->first!="dir" )
                        {
                            
                            Util::ValueRestorer< bool > x( attemptingToCreateFerrisType, true );
                            try
                            {
//                             LG_CTX_D << "Creating ferris-type ea on:" << newc->getURL()
//                                      << " value:" << iter->first
//                                      << endl;
                            //
                            // We use the low level method to make sure that by default the
                            // ferris-type value is stored into an XFS on disk ea.
                            // ie. we dont follow the overmount by default, make it native
                            // if possible
                            //
                                try
                                {
//                                 cerr << "Context::priv_createSubContext(make ft)"
//                                      << " value:" << iter->first
//                                      << " url:" << getURL()
//                                      << " newc:" << newc->getURL()
//                                      << endl;
                                    fh_mdcontext md = new f_mdcontext();
                                    fh_mdcontext child = md->setChild( "ea", "" );
                                    child->setChild( "name",  "ferris-type" );
                                    child->setChild( "value", iter->first );
                                    // Release 1.1.61: this makes ferris-type for all cp -av
                                    // which is incompatable with fileutils.
                                    // newc->priv_createSubContext( "", md );
                                }
                                catch( exception& e )
                                {
                            }
                                
//                             LG_CTX_D << "Created ferris-type ea on:" << newc->getURL()
//                                      << " read back value:" << getStrAttr( newc, "ferris-type","")
//                                      << endl;
                            }
                            catch( exception& e )
                            {
                                LG_CTX_W << "warning, cant set ferris-type ea to:"
                                         << iter->first
                                         << " reason:" << e.what()
                                         << endl;
                            }
                        }
                    }
                    
                    LG_CTX_D << "returning after sucessful create..." << endl;
                    return newc;
                }
                catch( NoSuchSubContext& e )
                {
//                    cerr << "not error. iter:" << iter->first << " not bound" << endl;
                }
                catch( FerrisCreateAttributeFailed& e )
                {
                    LG_CTX_D << "FerrisCreateAttributeFailed. creatingEA:" << creatingEA
                             << " createdPrimaryObject:" << createdPrimaryObject
                             << " e:" << e.what()
                             << endl;
                    
                    if( creatingEA )
                    {
                        if( createdPrimaryObject )
                            throw;
                        
                        createEAFailedList.push_back( e );
                        continue;
                    }
                    else
                    {
                        cerr << "Error creating new object. !!! iter:" << iter->first
                             << " url:" << getURL()
                             << " e:" << e.what() << endl;
                        BackTrace();
                    }
                }
                catch( exception& e )
                {
                    cerr << "Error creating new object. !!! iter:" << iter->first
                         << " url:" << getURL()
                         << " e:" << e.what() << endl;
                    BackTrace();
                }
            }

            if( creatingEA && !createEAFailedList.empty() )
            {
                for( createEAFailedList_t::const_iterator ei = createEAFailedList.begin();
                     ei != createEAFailedList.end(); ++ei )
                {
                }
                throw createEAFailedList.back();
            }
        }
        catch( FerrisCreateSubContextFailed& e )
        {
            throw e;
        }
        catch( exception& e )
        {
//            BackTrace();
//            cerr << "Context::priv_createSubContext() e:" << e.what() << endl;
            Throw_FerrisCreateSubContextFailed( e.what(), this );
        }

//        cerr << "Context::priv_createSubContext(end1)" << endl;
        fh_stringstream ss;
        ss << "url:" << getURL() << " rdn:" << rdn << endl;
//        cerr << "Context::priv_createSubContext(end) error:" << tostr(ss) << endl;
//        BackTrace();
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), 0 ); // this );
    }

    namespace Factory
    {
        /**
         * A cache of the context types that were recently created.
         * use the cursor API to access it.
         */
        fh_context getCreateHistory()
        {
            string CREATEHISTORY_URL = Shell::getHomeDirPath_nochecks() + CREATEHISTORY_RELATIVE;
            string dotferrisPath = Shell::getHomeDirPath_nochecks() + "/.ferris";
            string dbName = "/cache.db";
            string dbPath = dotferrisPath + dbName;
            
            try
            {
//                cerr << "CREATEHISTORY_URL:" << CREATEHISTORY_URL << endl;
                fh_context c = Resolve( dbPath );
            }
            catch( exception& e )
            {
//                 cerr << "can't resolve() going to make it e:" << e.what() << endl;
//                 cerr << "path:" << dotferrisPath << endl;

                fh_context dotferris = Shell::acquireContext( dotferrisPath );
                fh_mdcontext md = new f_mdcontext();
                fh_mdcontext child = md->setChild( "db4", "" );
                child->setChild( "name", dbName );
                fh_context dbc = dotferris->createSubContext( "", md );
            }

            fh_context dbc = Resolve( dbPath );
            if( !dbc->isSubContextBound( "create-history" ) )
            {
                fh_mdcontext    md = new f_mdcontext();
                fh_mdcontext child = md->setChild( "dir", "" );
                child->setChild( "name", "create-history" );
                fh_context x = dbc->createSubContext( "", md );
            }
            
            
            // using acquireContext() is a bad move from here because if it needs
            // to infact create a new context then it will end up calling here again
            // which will call it again to create the cachedb context (loop)
//            fh_context parent = Shell::acquireContext( CREATEHISTORY_URL );
            try
            {
                fh_context dbc = Resolve( CREATEHISTORY_URL );
                return dbc;
            }
            catch( exception& e )
            {
                cerr << "cache context could not be found or created."
                     << " CREATEHISTORY_URL:" << CREATEHISTORY_URL
                     << " e:" << e.what()
                     << endl;
            }
            return 0;
        }
    };
    
    /**
     * Used by priv_createSubContext and subclasses who override priv_createSubContext
     * to add entries to the history list of what types where created.
     *
     * This is used by graphical clients to present a list of the most recent item
     * types that were created for quick creation of more files.
     *
     * @param fileType identifier for the context that was last created
     */
    void
    Context::addToCreateHistory( const std::string& fileType )
    {
#ifdef DONT_TRACK_CREATION_HISTORY
        return;
#endif
        static bool SKIP_TRACK_CREATION_HISTORY =
            g_getenv( "LIBFERRIS_SKIP_TRACKING_CREATION_HISTORY" ) != 0;
        if( SKIP_TRACK_CREATION_HISTORY )
            return;
        
        static bool addingAlready = false;
        if( addingAlready )
            return;
        Util::ValueRestorer< bool > _obj( addingAlready, true );
        
        if( !fileType.empty() )
        {
            fh_context parent  = Factory::getCreateHistory();
            fh_context cursor  = Factory::getCursor( parent );
            string currentLast = getStrAttr( cursor, "content", "" );
            if( currentLast != fileType )
            {
//                 cerr << "Context::addToCreateHistory(1) this:" << getURL()
//                      << " fileType:" << fileType
//                      << endl;
                cursor = Cursor::cursorNext( cursor );
                setStrAttr( cursor, "content", fileType );
            }
        }
    }
    
    
/**
 * Create a new subcontext. This method always throws a not supported exception.
 * This makes read only subcontexts easy to implememnt because they dont have
 * to override this method.
 *
 * @see    priv_createSubContext()
 * @param rdn The relative name of the new context to create
 * @param md  specific information to the creation of a subclass. For example,
 *            when creating a new subcontext in a native filesystem then
 *            the m param may contain information telling the native context
 *            if it should create a file or a directory.
 *
 * @return The new context
 * @throws FerrisCreateSubContextFailed If there was a complication creating the new
 *         context.
 * @throws FerrisCreateSubContextNotSupported If the operation is not supported by this
 *         context.
 */
    fh_context
    Context::createSubContext( const string& rdn, fh_context md )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        try
        {
            const string filename = getLastPartOfName( rdn );

            LG_CTX_D << "Context::createSubContext() url:" << getURL() << endl;
            LG_CTX_D << "Context::createSubContext() md.addr:" << toVoid(md) << endl;
//        md->dumpOutItems();

//         if( !getOverMountContext()->canInsert( filename ) )
//         {
//             ostringstream ss;
//             ss << "Context already exists! filename:" << filename << " rdn:" << rdn;
//             Throw_FerrisCreateSubContextFailed( tostr( ss ), this );
//         }

            if( md && md->isSubContextBound("ea") )
            {
//                cerr << "Context::createSubContext(1)" << endl;
                
                fh_context eac = md->getSubContext( "ea" );
                if( eac->isSubContextBound("name"))
                {
//                    cerr << "Context::createSubContext(2)" << endl;
                    
                    fh_context nc = eac->getSubContext( "name" );
                    string n = getStrAttr( nc, "content", "" );
                    if( n == "ferris-type" || n == "is-ferris-compressed"
                        || n == "libferris-journal-changes" 
                        || eac->isSubContextBound("dont-delegate-to-overmount-context") )
                    {
//                        cerr << "-+-Creating new attribute in cc..." << endl;
                        LG_CTX_D << "-+-Creating new attribute in cc..." << endl;
                        fh_context ret = getCoveredContext()->priv_createSubContext( filename, md );
                        LG_CTX_D << "-+-Created new attribute in cc..." << endl;

                        
                        LG_CTX_D << "cc:" << getCoveredContext() << endl;
                        LG_CTX_D << "omc:" << getOverMountContext() << endl;
                        LG_CTX_D << "attr.name:" << n << endl;
                        LG_CTX_D << "cc.isBound:" << getCoveredContext()->isAttributeBound( n ) << endl;
                        LG_CTX_D << "omc.isBound:" << getOverMountContext()->isAttributeBound( n ) << endl;
                        
                        if( EA_Atom* atom = getCoveredContext()->getAttributePtr( n ) )
                        {
                            LG_CTX_D << "Have atom for cc..." << endl;
                            getOverMountContext()->setAttribute( n, atom, false );
                        }
                        
                        return ret;
                    }
                }
            }
            
            LG_OVERMOUNT_I << "Context::createSubContext() trying to overmount c:" << getDirPath() << endl;
            tryToOverMount( true );
            fh_context ret = getOverMountContext()->priv_createSubContext( filename, md );
            return ret;
        }
        catch( FerrisCreateSubContextFailed& e )
        {
            throw e;
        }
        catch( FerrisCreateSubContextNotSupported& e )
        {
            throw e;
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "Context::createSubContext() rdn:" << rdn
               << " e:" << e.what()
               << endl;
            Throw_FerrisCreateSubContextFailed( tostr(ss), this );
        }
    }

    fh_context
    Context::createSubContext( const std::string& rdn, fh_mdcontext md )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        fh_context c;
        Upcast( c, md );
        return createSubContext( rdn, c );
    }


    

/**
 * Create a new attribute with the given rdn.
 *
 * The attribute has no initial value. ie. The Istream from the attribute with
 * be at eof.
 *
 * @param  rdn Name of new attribute
 * @return New attribute
 *
 * @throws FerrisCreateAttributeFailed Creation failed, though this context does
 *         Support the creation operation.
 * @throws FerrisCreateAttributeNotSupported This context does not support creation of
 *         EA.
 * @throws AttributeAlreadyInUse If there is already an attribute with the given rdn.
 */
    fh_attribute
    Context::createAttribute( const string& rdn )
        throw( FerrisCreateAttributeFailed,
               FerrisCreateAttributeNotSupported,
               AttributeAlreadyInUse )
    {
        if( isAttributeBound( rdn ) )
        {
            ostringstream ss;
            ss << "createAttribute() AttributeAlreadyInUse:" << rdn;
            Throw_AttributeAlreadyInUse( tostr(ss), 0 );
        }

        /*
         * FIXME: should really cache this.
         */

        //
        // FIXME: Need to recode this in light of new attribute API
        //
        for( int Pri = AttributeCreator::CREATE_PRI_MAX_INTERNAL_USE_ONLY; Pri >= 0; --Pri )
        {
//             for( EAGenFactorys_t::const_iterator iter = getEAGenFactorys().begin();
//                  iter != getEAGenFactorys().end();
//                  ++iter )
            ensureEAGenFactorysSetup();
            EAGenFactorys_iterator iterend = EAGenFactorys_iterator( this, true );
            for( EAGenFactorys_iterator iter = EAGenFactorys_iterator( this );
                 iter != iterend; ++iter )
            {
                try
                {
                    if( (*iter)->getCreatePriority() == Pri )
                    {
                        return (*iter)->CreateAttr( this, rdn );
                    }
                }
                catch( exception& e )
                {
                    LG_ATTR_D << "e:" << e.what() << endl;
                }
            }
        }
    
        ostringstream ss;
        ss << "createAttribute() operation not supported by default context rdn:" << rdn;
        Throw_FerrisCreateAttributeNotSupported( tostr(ss), 0 );
    }


/**
 * If the attribute exists already then return it, otherwise create a new
 * attribute with the given rdn and return it.
 *
 * @param rdn Name for attribute sought.
 * @throws FerrisCreateAttributeFailed Creation failed, though this context does
 *         Support the creation operation.
 * @throws FerrisCreateAttributeNotSupported This context does not support creation of
 *         EA.
 */
    fh_attribute
    Context::acquireAttribute( const string& rdn )
        throw( FerrisCreateAttributeFailed,
               FerrisCreateAttributeNotSupported )
    {
        if( isBound(OverMountContext_Delegate) )
        {
            return OverMountContext_Delegate->acquireAttribute( rdn );
        }

        if( isAttributeBound( rdn ) )
        {
            return getAttribute( rdn );
        }
        return createAttribute( rdn );
    }


/**
 * Get a handle for the subcontext with a final path component of rdn.
 *
 * @see   getSubContextNames()
 * @see   isSubContextBound()
 *
 * @param rdn the filename / final path component for the subcontext that
 *        is sought.
 * @throws NoSuchSubContext If there is no subcontext with a name of rdn
 * @return the subcontext with name==rdn
 */
    fh_context
    Context::getSubContext( const string& rdn )
        throw( NoSuchSubContext )
    {
        if( rdn == "." )
        {
            return this;
        }
        if( rdn == ".." )
        {
            return getParent();
        }
        return getOverMountContext()->priv_getSubContext( rdn );
    }


/**
 * Get the names (rdn) of each subcontext.
 *
 * @see getSubContext()
 * @see isSubContextBound()
 *
 * @returns An STL collection of the names of each subcontext.
 */
    Context::SubContextNames_t&
    Context::getSubContextNames()
    {
        if( getOverMountContext() != this )
        {
            return getOverMountContext()->getSubContextNames();
        }

        if( SubContextNamesCacheIsValid )
        {
            LG_CTX_D << "Context::getSubContextNames() cached! "
                     << " path:" << getDirPath()
                     << " SubContextNamesCache.size():" << SubContextNamesCache.size()
                     << endl;
            return SubContextNamesCache;
        }
        
        UnPageSubContextsIfNeeded();
        Items_t& items = getSortedItems();

        SubContextNamesCacheIsValid = true;
        SubContextNamesCache.clear();
        
        LG_CTX_D << "Context::getSubContextNames(remake) "
                 << " path:" << getDirPath()
                 << " items.size():" << items.size()
                 << " SubContextNamesCache.size():" << SubContextNamesCache.size()
                 << endl;
        
        for( Items_t::iterator iter = items.begin();
             iter != items.end(); iter++ )
        {
            if( !(*iter)->HasBeenDeleted )
            {
                LG_CTX_D << "Context::getSubContextNames() adding:" << (*iter)->getDirName() << endl;
                SubContextNamesCache.push_back( (*iter)->getDirName() );
            }
        }

        LG_CTX_D << "Context::getSubContextNames() "
                 << " path:" << getDirPath()
                 << " items.size():" << items.size()
                 << " SubContextNamesCache.size():" << SubContextNamesCache.size()
                 << endl;
    
        return SubContextNamesCache;
    }











/**
 * Create a new context which is covering context cc.
 *
 * The covered context (cc) is used to mount a context over the top of another
 * context.
 *
 * @see   getCoveredContext()
 * @param cc The context to cover.
 *
 */
    Context::Context( Context* cc )
        :
        CoveredContext( cc ),
        NumberOfSubContexts(0),
        OverMountContext_Delegate(0),
        DirOpVersion(0),

        /* Context bitfields */
        Dirty( 0 ),
        AttributesHaveBeenCreated( 0 ),
        HaveDynamicAttributes( 0 ),
        updateMetaData_First_Time( 1 ),
        ensureUpdateMetaDataCalled_virgin(1),
        ReadingDir(0),
        HaveReadDir(0),
        FiredStartReading(0),
        WeAreInFreeList(1),
        EAGenFactorys_isVirgin(1),
        SubContextNamesCacheIsValid( false ),
        ContextWasCreatedNotDiscovered( false ),
        HasBeenDeleted( false ),
        m_overMountAttemptHasAlreadyFailed( false ),
        m_overMountAttemptHasAlreadyFailedEAOnly( false ),
        m_isNativeContext( false ),
        m_ChainedViewContext_Called_SetupEventConnections( false ),
        m_tryToGetImplicitTreeSmushHasFailed_forDirectory( false ),
        m_tryToGetImplicitTreeSmushHasFailed_forURL( false ),
        m_forcePassiveViewCache( false ),
        m_forcePassiveViewCacheIsValid( false ),
        m_holdingReferenceToParentContext( false ),
        AggregateData( 0 ),
        m_handlablesToReleaseWithContext( 0 )
    {
        getCacheManager()->addToFreeList( this );
        getNamingEvent_Stop_Reading_Context_Sig().connect( mem_fun( *this, &Context::ReadDone ));

        LG_CTX_D << "Context() this:" << this << endl;

        
//        createStateLessAttributes();

#ifdef DEBUG_CONTEXT_MEMORY_MANAGEMENT
        addContextToMemoryManagementData(this);
#endif
    }


    Context::Context( Context* parent, const std::string& rdn )
        :
        CoveredContext( 0 ),
        NumberOfSubContexts(0),
        OverMountContext_Delegate(0),
        DirOpVersion(0),

        /* Context bitfields */
        Dirty( 0 ),
        AttributesHaveBeenCreated( 0 ),
        HaveDynamicAttributes( 0 ),
        updateMetaData_First_Time( 1 ),
        ensureUpdateMetaDataCalled_virgin(1),
        HaveReadDir(0),
        FiredStartReading(0),
        WeAreInFreeList(1),
        ReadingDir(0),
        EAGenFactorys_isVirgin(1),
        SubContextNamesCacheIsValid( false ),
        ContextWasCreatedNotDiscovered( false ),
        HasBeenDeleted( false ),
        m_overMountAttemptHasAlreadyFailed( false ),
        m_overMountAttemptHasAlreadyFailedEAOnly( false ),
        m_isNativeContext( false ),
        m_ChainedViewContext_Called_SetupEventConnections( false ),
        m_tryToGetImplicitTreeSmushHasFailed_forDirectory( false ),
        m_tryToGetImplicitTreeSmushHasFailed_forURL( false ),
        m_forcePassiveViewCache( false ),
        m_forcePassiveViewCacheIsValid( false ),
        m_holdingReferenceToParentContext( false ),
        AggregateData( 0 ),
        m_handlablesToReleaseWithContext( 0 )
    {
        getCacheManager()->addToFreeList( this );
        getNamingEvent_Stop_Reading_Context_Sig().connect(sigc::mem_fun( *this, &Context::ReadDone ));

        LG_CTX_D << "Context() this:" << this << endl;

//        createStateLessAttributes();
        setContext( parent, rdn );

#ifdef DEBUG_CONTEXT_MEMORY_MANAGEMENT
        addContextToMemoryManagementData(this);
#endif
    }

    
/**
 * Clean up
 */
    Context::~Context()
    {
        LG_CTX_I << "~Context() this:" << this << " rdn:" << getDirName() << endl;
//         if( ((string)"schema.xml") == getDirName() )
//         {
//             CERR << "~Context() this:" << this << " rdn:" << getDirName() << endl;
//             CERR << "~Context() cc:" << getCoveredContext() << endl;
//             CERR << "~Context() omc:" << getOverMountContext() << endl;
//             CERR << "~Context() ref_count:" << ref_count << endl;
//             CERR << "-- BEGIN DEBUG_dumpcl Context::~Context()--\n";
//             dumpEntireContextListMemoryManagementData( Factory::fcerr() );
//             CERR << "-- END DEBUG_dumpMemc Context::~Context()--\n"
//                  << endl;
//             BackTrace();
//        }
        
        AttributeCountRaisedFromOne_Connection.disconnect();

#ifdef DEBUG_CONTEXT_MEMORY_MANAGEMENT
        remContextToMemoryManagementData(this);
#endif

        s_downloadMTimeSince().erase( this );

        if( m_handlablesToReleaseWithContext )
        {
            m_handlableList_t::iterator e    = m_handlablesToReleaseWithContext->end();
            m_handlableList_t::iterator iter = m_handlablesToReleaseWithContext->begin();
            for( ; iter != e; )
            {
                Handlable* h = *iter;
                ++iter;
                h->Release();
            }
            delete m_handlablesToReleaseWithContext;
        }
        
        

//         clearContext();
//         clearAttributes();
    }

    void
    Context::addHandlableToBeReleasedWithContext( Handlable* h )
    {
        if( !m_handlablesToReleaseWithContext )
            m_handlablesToReleaseWithContext = new m_handlableList_t;

        h->AddRef();
        m_handlablesToReleaseWithContext->push_back( h );
    }
    
    Context::s_downloadMTimeSince_t&
    Context::s_downloadMTimeSince()
    {
        static Context::s_downloadMTimeSince_t* ret = 0;
        if( !ret )
        {
            ret = new Context::s_downloadMTimeSince_t();
        }

        return *ret;
    }

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    /**
     * This is quite tricky. Basically there needs to be the ability to perform some
     * actions on a subcontext given only its rdn. Things like binary_search()ing for
     * an rdn expect that the underlying set<> is sorted by rdn.
     * Thus we keep the main items collection sorted by rdn and everyone is happy.
     * For SortedContext objects a second Items set<> is maintained which is in sorted order
     * so that such things as iterators and getSubContextNames() can operate quickly
     *
     * INVARIANT: getSortedItems().size() == getItems().size()
     */
    Context::Items_t&
    Context::getSortedItems()
    {
        return getItems();
    }
    
    Context::iterator Context::begin() 
    {
        if( getOverMountContext() != this )
        {
            return getOverMountContext()->begin();
        }

        
        UnPageSubContextsIfNeeded();

        if( getSortedItems().empty() )
        {
            /* Give it one more chance to get some items */
            read();

            // read()ing might have caused an overmount.
            if( getOverMountContext() != this )
            {
                return getOverMountContext()->begin();
            }
            
            if( getSortedItems().empty() )
            {
                LG_CTX_N << "Context::begin() getSortedItems() is empty()"
                         << " url:" << getURL()
                         << " items.size():" << getItems().size()
                         << " sorteditems.size():" << getSortedItems().size()
                         << " returning end() "
                         << endl;
                return end();
            }
        }

        if( getSortedItems().size() != getItems().size() )
        {
            LG_CTX_ER << "Context::begin() -- SortedItems.size() != Items.size()"
                      << " SORTING IS PROBABLY BOTCHED!" << endl;
        }

        // lets not try to access begin()
        if( getSortedItems().empty() )
            return end();
        
//        string firstrdn = (*(getSortedItems().begin()))->getDirName();

        string firstrdn = "";
        {
            Items_t::iterator  e = getSortedItems().end();
            Items_t::iterator si = getSortedItems().begin();
            while( si != e && (*si)->HasBeenDeleted )
                ++si;
            if( si == e )
            {
                return end();
            }
            firstrdn = (*si)->getDirName();
        }
        if( firstrdn.empty() )
        {
            return end();
        }
        
        
        
//        string firstrdn = getSubContextNames().front();
        LG_CTX_D << "Context::begin() url:" << getURL()
                 << " first:" << firstrdn
                 << " items.size():" << getItems().size()
                 << " sorteditems.size():" << getSortedItems().size()
                 << endl;
//         cerr << "Context::begin() url:" << getURL()
//              << " first:" << firstrdn
//              << " items.size():" << getItems().size()
//              << " sorteditems.size():" << getSortedItems().size()
//              << endl;

//         cerr << "Context::dumpOutItems( START ) this:" << toVoid(this)
//              << " url:" << getURL() << endl;
//         for( Items_t::iterator iter = getSortedItems().begin();
//              iter != getSortedItems().end(); iter++ )
//         {
//             cerr << "iteration. "
//                  << " first:" << (*iter)->getDirName()
//                  << " bound:" << isBound( *iter )
//                  << " bound2:" << isSubContextBound( (*iter)->getDirName() );
            
//             if( isBound( *iter ) )
//             {
//                 cerr << " path :" << (*iter)->getDirPath();
//             }
//             cerr << endl;
//         }
    
//         cerr << "Context::dumpOutItems( END )" << endl;
        
        return ContextIterator( this, firstrdn );
    }
    
    Context::iterator Context::end() 
    {
        if( getOverMountContext() != this )
        {
            return getOverMountContext()->end();
        }

        UnPageSubContextsIfNeeded();
        return ContextIterator( this, "" );
    }

    Context::reverse_iterator
    Context::rbegin()
    {
        return reverse_iterator(end());
    }
    
    Context::reverse_iterator
    Context::rend()
    {
        return reverse_iterator(begin());
    }

    Context::iterator
    Context::find( const std::string& rdn )
    {
        if( getOverMountContext() != this )
        {
            return getOverMountContext()->find( rdn );
        }

        
        UnPageSubContextsIfNeeded();

        if( !priv_isSubContextBound( rdn ) )
        {
            return end();
        }
        return ContextIterator( this, rdn );
    }
    
    ContextDirOpVersion_t
    Context::getDirOpVersion()
    {
        return getOverMountContext()->DirOpVersion;
    }

    Context::iterator toContextIterator( fh_context c )
    {
//         cerr << "toContextIterator() isbound  c:" << isBound(c) << endl;
//         cerr << "toContextIterator()          c:" << c->getURL() << endl;
//         cerr << "toContextIterator() isbound cp:" << c->isParentBound() << endl;
        fh_context p = c->getParent();

//         cerr << "toContextIterator()          p:" << p->getURL() << endl;
//         cerr << "toContextIterator() found    c:"
//              << (p->find( c->getDirName() )!=p->end()) << endl;
//         cerr << flush;
        
        return p->find( c->getDirName() );
    }

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    static fh_context getRootOfContext( fh_context c )
    {
        while( c->isParentBound() )
        {
            c = c->getParent();
        }
        return c;
    }

    std::string
    Context::getURLScheme()
    {
        fh_context rc = getRootOfContext( this );
//          cerr << "Context::getURLScheme()  p:" << getDirPath() << endl;
//          cerr << "Context::getURLScheme()  n:" << getDirName() << endl;
//          {
//              fh_context c = this;
//              while( c->isParentBound() )
//              {
//                  c = c->getParent();
//                  cerr << "Context::getURLScheme() cp:" << c->getDirPath() << endl;
//                  cerr << "Context::getURLScheme() cn:" << c->getDirName() << endl;
//              }
//          }
//          cerr << "Context::getURLScheme() rp:" << rc->getDirPath() << endl;
//          cerr << "Context::getURLScheme() rn:" << rc->getDirName() << endl;
        string    ret = RootContextFactory::getRootContextClassName(rc);
        return ret;
    }
    
    const std::string&
    Context::getDirName() const
    {
        return DirName;
    }
    
    

    /**
     * This is a little tricky, but for the over mount context itself we return
     * not the URL of the base context. This is so that comparisons on the XML/db4
     * file itself with the getParent()->getURL() of a overmounted context will
     * compare equal.
     */
    std::string
    Context::getURL()
    {
        if( isBound(CoveredContext) )
        {
            return CoveredContext->getURL();
        }
        
        fh_stringstream ss;
        ss << getURLScheme() << "://" << getDirPath();
        return tostr(ss);
    }



/**
 * Get a reference to the collection of subcontexts.
 * This method should always be prefered to using the Items collection
 * directly.
 *
 * @return The STL collection of subcontexts.
 */
    Context::Items_t&
    Context::getItems()
    {
        return Items;
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    struct FERRISEXP_DLLLOCAL DummyContext
        :
        public Context
    {
        void setName( const std::string& name )
            {
                setContext( 0, name );
            }
        virtual Context* priv_CreateContext( Context* parent, std::string rdn )
            {
                BackTrace();
                return 0;
            }
    };
    
    Context::Items_t::iterator
    Context::ctx_lower_bound( Context::Items_t& items, const std::string& rdn )
    {
#ifndef LIBFERRIS__USE_BOOST_MULTI_INDEX_FOR_STORING_SUBCONTEXTS
        static DummyContext* dc_down = new DummyContext;
        static fh_context dc = dc_down;
        dc_down->setName( rdn );

        Items_t::iterator ret = items.lower_bound( dc );
        dc_down->setName( "ctx_lower_bound_helper" );
#else
        Items_t::iterator ret = Items.get<ITEMS_T_BY_NAME_ORDERED_TAG>().lower_bound( rdn );
#endif   
        return ret;
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
           #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>

    /**
     * Find an element in the Items set by its rdn (DirName)
     */
    Context::Items_t::iterator
    Context::getItemIter( const std::string& rdn )
    {
#ifndef LIBFERRIS__USE_BOOST_MULTI_INDEX_FOR_STORING_SUBCONTEXTS
        Items_t::iterator iter = ctx_lower_bound( getItems(), rdn );
#else
        Items_By_Name_Hashed_t::iterator hiter
            = Items.get<ITEMS_T_BY_NAME_UNORDERED_TAG>().find( rdn );
        Items_t::iterator iter = Items.project<ITEMS_T_BY_NAME_ORDERED_TAG>( hiter );
#endif
        
        if( iter == Items.end() || (*iter)->getDirName() != rdn )
        {
            stringstream ss;
            ss << "Context::getItem() url:" << getURL()
               << " items.sz:" << getItems().size()
               << " NoSuchSubContext:" << rdn;
            
//             {
//                 static int fd = 0;
//                 if( !fd )
//                 {
//                     fd = creat("/tmp/getitem-backtrace",S_IRWXU);
//                     BackTrace(fd);
//                 }
//             }
            
            Throw_NoSuchSubContext( tostr(ss), this );
        }
        return iter;
    }

    /**
     * Find an element in the Items set by its rdn (DirName)
     */
    fh_context
    Context::getItem( const std::string& rdn )
    {
        return *getItemIter( rdn );
    }

    void
    Context::eraseItemByName( Items_t& items, Context* c )
    {
#ifndef LIBFERRIS__USE_BOOST_MULTI_INDEX_FOR_STORING_SUBCONTEXTS
        items.erase( c );
#else
        eraseItemByName( items, c->getDirName() );
#endif
    }
    
    
    void
    Context::eraseItemByName( Items_t& items, const std::string& rdn )
    {
#ifdef LIBFERRIS__USE_BOOST_MULTI_INDEX_FOR_STORING_SUBCONTEXTS
        items.get<ITEMS_T_BY_NAME_UNORDERED_TAG>().erase( rdn );
#else
        fh_context c = 0;
        {
            Items_t::iterator iter = ctx_lower_bound( getItems(), rdn );
//             Items_t::iterator iter = ex_lower_bound( getItems(),
//                                                      rdn,
//                                                      items_lookup_compare() );
//             Items_t::iterator iter = lower_bound( getItems().begin(),
//                                                   getItems().end(),
//                                                   rdn,
//                                                   items_lookup_compare() );

            if( iter == Items.end() || (*iter)->getDirName() != rdn )
                return;
            
            c = *iter;
        }
        getItems().erase(c);
#endif
    }
    
    


/**
 * Insert a new context as a subcontext.
 *
 * Currently we trust VFS code to only insert something valid. There is
 * great overhead in calling read_subContext() to see if the VFS really
 * thinks that a child exists. Plus that could be very recursive.
 *
 * @see canInsert()
 *
 * @param ctx     The new context to insert
 * @param created If true then a Created event is fired, otherwise a exists event is fired.
 * @throw SubContextAlreadyInUse If the rdn of ctx is already in use as a subcontext name
 * @return Handle for ctx
 */
    fh_context
    Context::Insert( Context* ctx, bool created, bool emit )
        throw( SubContextAlreadyInUse )
    {
        try
        {
            
        if( getOverMountContext() != this )
        {
            return getOverMountContext()->Insert( ctx, created, emit );
        }

//         cerr << "Context::Insert(1) ctx:" << ctx->getURL()
//              << " c:" << created << " e:" << emit << endl;
        

        if( !ctx )
        {
            LG_CTX_ER << "TRYING TO ADD A NULL CONTEXT!" << endl;

            fh_stringstream ss;
            ss << "Attempt to add a NULL context" << endl;
            Throw_SubContextAlreadyInUse( tostr(ss), ctx );
        }

        try
        {
            LG_CTX_D << "Insert(1)      getDirPath():" << getDirPath() << endl;
            LG_CTX_D << "Insert(1) ctx->getDirPath():" << ctx->getDirPath() << endl;
            LG_CTX_D << "Insert(1) ctx->getDirName():" << ctx->getDirName() << endl;
        }
        catch( exception& e )
        {
//            cerr << " ex:" << e.what() << endl;
        }

        if( !canInsert( ctx->getDirName()) )
        {
            LG_CTX_W << "Insert(throw in use) path:" << getDirPath()
                     << " ctx.name:" << ctx->getDirName()
                     << " ctx.path:" << ctx->getDirPath()
                     << endl;
//             cerr << "Insert(throw in use) p:" << getDirPath()
//                  << " rdn:" << ctx->getDirPath() << endl;
//             BackTrace();
            fh_stringstream ss;
            ss << "Insert() rdn already in use:" << ctx->getDirName();
            Throw_SubContextAlreadyInUse( tostr(ss), this );
        }

        ctx->ContextWasCreatedNotDiscovered = created;

        const string& rdn = ctx->getDirName();
        getItems().insert( ctx );
        const fh_context& ret = ctx;

        
//        getItems()[ rdn ] = ctx;
//        const fh_context& ret = getItems()[ rdn ];
        SubContextNamesCacheIsValid = false;
        
        ++NumberOfSubContexts;
        getOverMountContext()->DirOpVersion++;
        bumpVersion();

        
        
        /*
         * Cascade a reference from us up the tree for this new child node
         */
//         {
//             Context* p = this;
//             while( p )
//             {
//                 p->AddRef();
//                 if( !p->isParentBound() )
//                     break;
//                 p = p->getParent();
//             }
//         }
        

        LG_CTX_D << "Context::Insert() emiting this:" << toVoid(this)
                 << " created:" << created
                 << " emit:" << emit
                 << " path:" << getDirPath()
                 << " rdn:" << rdn
                 << endl;

        if( emit )
        {
            LG_CTX_D << "Context::Insert() emiting created:" << created
                     << " path:" << getDirPath()
                     << " rdn:" << rdn
                     << " items.size():" << getItems().size()
                     << " sorteditems.size():" << getSortedItems().size()
                     << endl;

            if( created )
            {
//                DEBUG_dumpcl("created new itemAAA");
                Emit_Created( 0, ret, rdn, rdn, 0 );
            }
            else
            {
                Emit_Exists( 0, ret, rdn, rdn, 0 );
            }
        }

        debug_ferris_check_for_single_ctx_violation( this, ctx );
        LG_CTX_D << "Insert(exit) path:" << ctx->getDirPath() << endl;
        return ret;
        }
        catch( SubContextAlreadyInUse& )
        {
            throw;
        }
        catch( exception& e )
        {
            LG_CTX_ER << "Context::Insert() url:" << getURL()
                      << " inserting:" << ctx->getURL()
                      << " unexpected error:" << e.what()
                      << endl;
//            BackTrace();
            fh_stringstream ss;
            ss << "Insert() unexpected error:" << e.what() << endl;
            Throw_SubContextAlreadyInUse( tostr(ss), this );
        }
    }


/**
 * Set the context that we are mounting over.
 *
 * @see getOverMountContext()
 * @see clearOverMountContext()
 * @param c Context that we are covering.
 */
    void
    Context::setOverMountContext( const fh_context& c )
    {
        OverMountContext_Delegate = c;
    }



/**
 * If we are mounted over another context the this method returns a pointer
 * to the base context that we are covering.
 *
 * @see setOverMountContext()
 * @see clearOverMountContext()
 * @return Context we are covering or 0
 */
    Context*
    Context::getOverMountContext()
    {
        if( isBound(OverMountContext_Delegate) )
        {
            return GetImpl(OverMountContext_Delegate);
        }
        return this;
    }

    void
    Context::setCoveredContext( const fh_context& c )
    {
        CoveredContext = c;
    }
    
    
    Context*
    Context::getCoveredContext()
    {
        if( isBound(CoveredContext) )
            return GetImpl(CoveredContext);
        return this;
    }
    

/**
 * Clear the context that we are mounting over. This ensures following
 * calls to getOverMountContext() return 0.
 *
 * @see getOverMountContext()
 * @see clearOverMountContext()
 * @param c Context that we are covering.
 */
    void
    Context::clearOverMountContext()
    {
//    OverMountContext_Delegate.unbind();
        OverMountContext_Delegate = 0;
    }

    /**
     * Depth first context deletion
     * @see unOverMount()
     */
    void
    Context::unOverMount_delete( CacheManager::freelist_t& deletedItems, Context* c )
    {
        Private::CacheManagerImpl* cm = Private::getCacheManagerImpl();
        LG_VM_D << " Context::unOverMount_delete() c:" << toVoid(c) << " rdn:" << c->getDirName() << endl;
//        LG_CTX_D << " Context::unOverMount_delete() c:" << c->getDirPath() << endl;
        
        Items_t::iterator iter = c->getItems().begin();
        for( ; iter != c->getItems().end(); )
        {
            string rdn     = (*iter)->getDirName();
            Context* child = GetImpl( *iter );
            ++iter;

            LG_VM_D << " Context::unOverMount_delete() child:" << toVoid(child)
                    << " rdn:" << rdn << endl;
            
            unOverMount_delete( deletedItems, child );
//            c->getItems().erase( child );
            c->eraseItemByName( c->getItems(), child );
            
//            LG_CTX_D << " Context::unOverMount_delete() del child:" << child->getDirPath() << endl;
            cm->removeFromFreeList( child, true );
            cerr << " Context::unOverMount() rem-from-fl:" << (void*)child << endl;
            deletedItems.insert( child );
            Factory::getPluginOutOfProcNotificationEngine().forgetContext( child );
            delete child;
        }
        SubContextNamesCacheIsValid = false;
    }
    

    
    /**
     * Unmount the stacked file system and delete it.
     * This is what the VM system should use to reclaim a stacked filesystem
     */
    void
    Context::unOverMount( CacheManager::freelist_t& deletedItems )
    {
        Context* oc = GetImpl( OverMountContext_Delegate );
        LG_CTX_D << " Context::unOverMount() oc:" << oc->getDirPath() << endl;

        Factory::getPluginOutOfProcNotificationEngine().forgetContext( this );
        Factory::getPluginOutOfProcNotificationEngine().forgetContext( oc );
        
        /*
         * Delete all the children of the overmount first
         */
        unOverMount_delete( deletedItems, oc );
        OverMountContext_Delegate = 0;
        oc->ref_count=ImplementationDetail::MAX_REF_COUNT;
///        Remove( oc );
        Private::CacheManagerImpl* cm = Private::getCacheManagerImpl();
        cm->removeFromFreeList( oc, true );
//        cerr << " Context::unOverMount() rem-from-fl:" << (void*)oc << endl;
        deletedItems.insert( oc );
        delete oc;
        NumberOfSubContexts = 0;
        HaveReadDir = false;
        bumpVersion();
    }
    

    /**
     * This version does not call read() for you if you have not read the dir
     */
    bool
    Context::priv_isSubContextBound( const std::string& rdn, Items_t::iterator& iter )
    {
#ifndef LIBFERRIS__USE_BOOST_MULTI_INDEX_FOR_STORING_SUBCONTEXTS
        iter = ctx_lower_bound( getItems(), rdn );
#else
        Items_By_Name_Hashed_t::iterator hiter
            = Items.get<ITEMS_T_BY_NAME_UNORDERED_TAG>().find( rdn );
        iter = Items.project<ITEMS_T_BY_NAME_ORDERED_TAG>( hiter );
#endif
        
        if( iter == Items.end() || (*iter)->getDirName() != rdn )
        {
            return false;
        }
        return true;
    }
    

    /**
     * This version does not call read() for you if you have not read the dir
     */
    bool
    Context::priv_isSubContextBound( const std::string& rdn )
    {
#ifndef LIBFERRIS__USE_BOOST_MULTI_INDEX_FOR_STORING_SUBCONTEXTS
        Items_t::iterator iter = ctx_lower_bound( getItems(), rdn );
#else
        Items_By_Name_Hashed_t::iterator hiter
            = Items.get<ITEMS_T_BY_NAME_UNORDERED_TAG>().find( rdn );
        Items_t::iterator iter = Items.project<ITEMS_T_BY_NAME_ORDERED_TAG>( hiter );
#endif
        
        if( iter == Items.end() || (*iter)->getDirName() != rdn )
        {
            return false;
        }
        return true;
    }
    

/**
 * Check to see if there is a subcontext with the given rdn.
 *
 * @see getSubContextNames()
 * @see getSubContext()
 *
 * @param rdn The final path component of the subcontext that is sought.
 * @returns true if there is a subcontext with the given rdn.
 */
    bool
    Context::isSubContextBound( const string& rdn )
    {
        if( getOverMountContext() != this )
        {
            return getOverMountContext()->isSubContextBound( rdn );
        }

        LG_CTX_D << "Context::isSubContextBound() url:" << getURL()
                 << " rdn:" << rdn
                 << " HaveReadDir:" << HaveReadDir
                 << " items.sz:" << getItems().size()
                 << endl;
        
//        if( !HaveReadDir )
        if( !HaveReadDir || ReadingDir ) 
        {
            try
            {
                fh_context c = getSubContext( rdn );
                return true;
            }
            catch( ... )
            {
                return false;
            }
        }
        try
        {
            return priv_isSubContextBound( rdn );
        }
        catch( ... )
        {
            return false;
        }
    }

/**
 * Check if a call to Insert() will succeed.
 *
 * @see Insert()
 *
 * @param rdn The name of the new context to insert
 * @return true if a subcontext with name rdn can be inserted.
 */
    bool
    Context::canInsert( const string& rdn )
    {
        return !priv_isSubContextBound( rdn );
    }


/**
 * Debug method to show the subcontexts
 */
    void
    Context::dumpOutItems()
    {
        BackTrace();
        cerr << "Context::dumpOutItems( START ) url:" << getURL()
             << " this:" << toVoid(dynamic_cast<Context*>(this))
             << " cc:"   << toVoid(dynamic_cast<Context*>(GetImpl(CoveredContext)))
             << " omc:"  << toVoid(dynamic_cast<Context*>(getOverMountContext()))
             << endl;

        if( getOverMountContext() != this )
        {
            getOverMountContext()->dumpOutItems();
            return;
        }

//         LG_CTX_D << "Context::dumpOutItems( START ) url:" << getURL() << endl;
//         typedef Context::SubContextNames_t cnt;
//         cnt na = getSubContextNames();
//         for( cnt::iterator iter = na.begin(); iter != na.end(); ++iter )
//         {
//             LG_CTX_D << " iter:" << *iter << endl;
//             cerr << " iter:" << *iter << endl;
//         }
//         LG_CTX_D << "Context::dumpOutItems( START2 ) url:" << getURL() << endl;
//         cerr << "Context::dumpOutItems( START2 ) url:" << getURL() << endl;
        for( Items_t::iterator iter = getItems().begin();
             iter != getItems().end(); iter++ )
        {
            LG_CTX_D << "iteration. "
                     << " first:" << (*iter)->getDirName()
                     << " bound:" << isBound( *iter )
                     << endl;
            
            if( isBound( *iter ) )
            {
//                 LG_CTX_D << " path:" << (*iter)->getDirPath();
//                 cerr << " path:" << (*iter)->getDirPath() << endl;

                LG_CTX_D << " child:" << toVoid(dynamic_cast<Context*>(GetImpl(*iter)))
                         << " cc:"   << toVoid(dynamic_cast<Context*>(GetImpl((*iter)->CoveredContext)))
                         << " omc:"  << toVoid(dynamic_cast<Context*>((*iter)->getOverMountContext()))
                         << " url:" << (*iter)->getURL()
                         << endl;
                cerr << " child:" << toVoid(dynamic_cast<Context*>(GetImpl(*iter)))
                     << " cc:"   << toVoid(dynamic_cast<Context*>(GetImpl((*iter)->CoveredContext)))
                     << " omc:"  << toVoid(dynamic_cast<Context*>((*iter)->getOverMountContext()))
                     << " rdn:" << (*iter)->getDirName()
                     << " url:" << (*iter)->getURL()
                     << endl;
            }
            LG_CTX_D << endl;
        }
    
//         LG_CTX_D << "Context::dumpOutItems( END )" << endl;
         cerr << "Context::dumpOutItems( END )" << endl;
    }

    void
    Context::dumpTree()
    {
        LG_SPIRITCONTEXT_D << "Context::dumpTree children:" << getSubContextCount()
                 << " token:" << getStrAttr( this, "token", "" )
                 << " in-order:" << getStrAttr( this, "in-order-insert-list", "", true )
                 << " url:" << getURL() << endl;
        for( Items_t::iterator iter = getItems().begin();
             iter != getItems().end(); iter++ )
        {
            (*iter)->dumpTree();
        }
    }


/**
 * Remove the given context from the subcontext collection. If ctx is not
 * a subcontext then nothing is done, it is logged as an error to remove
 * a non subcontext however.
 *
 * Note that handles can not be used as method params because of the subtle
 * interactions with the tree memory management code.
 *
 * @see Insert()
 * @param ctx Context to remove.
 *
 */
    void
    Context::Remove( Context* ctx, bool emitdeleted )
    {
        if( !ctx )
        {
            LG_CTX_ER << "TRYING TO REMOVE A NULL CONTEXT!" << endl;
            return;
        }

//        cerr << "Context::Remove(1) this:" << toVoid(this) << " child:" << toVoid(ctx) << endl;

        /*
         * When the VM subsystem is bringing context objects into and out of existance
         * it is wrong to signal deletion because only the fh_context is being deleted, not
         * the underlying file system object.
         */
        if( emitdeleted )
        {
            const string& rdn = ctx->getDirName();
            LG_CTX_D << " rdn:" << rdn << endl;
            LG_CTX_D << " NumberOfSubContexts:" << NumberOfSubContexts << endl;
            Emit_Deleted( 0, rdn, rdn, 0 );
        }
        
        /*
         * Things are done this way to avoid 'reading' the context at rdn.
         * if we set it to zero then we just drop the ref_count. we can
         * then operate on the zero reference with erase() cheaply
         */
        {
            /*
             * We need to make sure that the same item is not placed on the freelist
             * after this call.
             */
//            getItems().erase( ctx );
            eraseItemByName( getItems(), ctx );
            getCacheManager()->removeFromFreeList( ctx, true );
        }
        --NumberOfSubContexts;
        SubContextNamesCacheIsValid = false;
        getOverMountContext()->DirOpVersion++;
        bumpVersion();
        updateMetaData();
    }

/**
 * Wrapper method to allow removal of a subcontext using a handle.
 *
 * Remove the given context from the subcontext collection. If ctx is not
 * a subcontext then nothing is done, it is logged as an error to remove
 * a non subcontext however.
 *
 * @see Insert()
 * @param ctx Context to remove.
 *
 */
    void
    Context::Remove( fh_context ctx, bool emitdeleted )
    {
        if( !isBound(ctx) )
        {
            LG_CTX_ER << "TRYING TO REMOVE A NULL CONTEXT!" << endl;
            return;
        }

        Remove( GetImpl(ctx), emitdeleted );
    }


/**
 * Remove the subcontext with the given rdn if one exists.
 *
 * @see Insert()
 * @param ctxName rdn of subcontext to remove.
 *
 */
    void
    Context::Remove( const string& ctxName, bool emitdeleted )
    {
        try
        {
            Remove( getSubContext( ctxName ), emitdeleted );
        }
        catch(...)
        {}
    }

static bool isFilter( Context* c )
{
    if( dynamic_cast<FilteredContext*>(c) )
        return true;
    return false;
}


static bool isSorter( Context* c )
{
    if( dynamic_cast<SortedContext*>(c) )
        return true;
    return false;
}

    
    void
    Context::dumpRefDebugData( fh_ostream ss )
    {
        ss << "C"
           << " rc:" << ref_count
           << " numSc:" << NumberOfSubContexts
           << " item.sz:" << getItems().size()
           << " claim:" << isReClaimable()
           << " frel:" << WeAreInFreeList
           << " mRC:" << getMinimumReferenceCount()
           << " this:" << (void*)this
           << " isF:" << isFilter(this)
           << " isS:" << isSorter(this);
        if( HasBeenDeleted )
            ss << " DEL";
        if( isInheritingContext( this ) )
            ss << " inh";
        if( ChainedViewContext* cc = dynamic_cast<ChainedViewContext*>( this ) )
        {
            ss << " D:" << toVoid(GetImpl(cc->Delegate));
        }
        if( isBound( CoveredContext ) )
        {
            ss << " cc:" << toVoid( GetImpl( CoveredContext ));
        }
        if( isBound(OverMountContext_Delegate) )
        {
            ss << " om:" << toVoid( GetImpl( OverMountContext_Delegate ));
        }
        
        if( isParentBound() )
            ss << " pnt:" << (void*)getParent();
        if( isBound( ParentContext )
            && isParentBound()
            && GetImpl(ParentContext) != getParent() )
        {
            ss << " pntR:" << (void*)GetImpl(ParentContext);
        }
        

//        ss << " url:" << getURL()

        ss << " n:" << getDirName();
        ss << " p:" << getDirPath()
           << endl;
    }

    
    void
    ContextStreamMemoryManager::StreamIsOpeningHandler( Context* selfp, fh_istream& ss )
    {
        selfp->AddRef();

        sh_t sh = ss.sh;
        sh->getGenericCloseSig().connect(
            sigc::mem_fun( *selfp,
                        &Context::StreamIsClosingHandler ));
    }

    /*
     * It is safest to remove the context then the attribute, because the attribute
     * will always have a reference to its parent, so when that count==0 then the
     * context may be cleaned.
     */
    void
    ContextStreamMemoryManager::StreamIsOpeningHandler(
        Context* selfp, AttributeProxy* a, fh_istream& ss )
    {
        StreamIsOpeningHandler( selfp, ss );
    }
    
    /**
     * Bump the ref_count on the context that this attribute is attached to
     * so that the context is not reclaimed while the stream is outstanding.
     *
     * @param ss The stream that is being returned to the user.
     */
    void
    Context::RegisterStreamWithContextMemoryManagement( fh_istream& ss )
    {
        ContextStreamMemoryManager::StreamIsOpeningHandler( this, ss );
    }
    
    
    /**
     * Decrement the ref_count on the context that this attribute is attached to
     * so that the context knows that one less stream is outstanding.
     *
     * @param h The stream that is being returned to the user.
     */
    void
    Context::StreamIsClosingHandler( FerrisLoki::Handlable* h )
    {
        Release();
    }

    
    
    
/**
 * An object is in a claimable state if it is only referenced by its parent
 * context and its subcontexts, ie. there are no client handles on the given
 * child context.
 *
 * Note that for OverMount contexts there is no parent, but the CoveredContext
 * keeps a reference to the OverMount context, so it has the same reference
 * count numbers as a normal context.
 *
 * invoke this method on a context to see if it can be safely removed.
 *
 * @return true if this context can be removed
 */
    bool
    Context::isReClaimable()
    {
        /*
         * Perform a few assertions just to make sure that the freelist code is working
         * as expected.
         */
        if( ref_count > getMinimumReferenceCount())
        {
            LG_VM_D << "isReClaimable() path:" << getDirPath() << endl
                      << " ref_count:" << ref_count
                      << " getMinimumReferenceCount():" << getMinimumReferenceCount()
                      << " ref_count > getMinimumReferenceCount()"
                      << endl;
            return false;
        }

        return true;
    }
    
    
/**
 * After performing various assertions to ensure that 'child' is still
 * in a claimable state, remove and delete the given subcontext.
 *
 * An object is in a claimable state if it is only referenced by this
 * context and its subcontexts, ie. there are no client handles on the given
 * child context.
 *
 * @see UnPageSubContextsIfNeeded()
 *
 * @param child Subcontext to remove
 * @return true if the subcontext child was removed and deleted.
 */
// Used by garbo collector, so can not create more handles.
    bool
    Context::reclaimContextObject( Context* child )
    {
        string rdn = child->getDirName();

#ifdef FERRIS_DEBUG_VM
        cerr << "Context::reclaimContextObject() "
             << " c:"     << toVoid(child)
             << " path:"   << getDirPath()
             << " child:" << rdn
             << " rc:" << child->ref_count
             << endl;
#endif
        
        LG_VM_D << "Context::reclaimContextObject() "
                << " c:"     << toVoid(child)
                << " path:"   << getDirPath()
                << " child:" << rdn
                << " rc:" << child->ref_count
                << endl;

        /*
         * Chained contexts have two possibilities for their parent:
         * A) point to another chain context in the chained tree
         * B) (root of chained tree) point to the parent of the delegate as their own parent
         * We need to dispose of the root node of a chained tree in a slightly different
         * manner as it isn't a child of anything.
         *
         */
        if( ChainedViewContext* cc = dynamic_cast<ChainedViewContext*>( child ) )
        {
            if( ChainedViewContext* thiscc = dynamic_cast<ChainedViewContext*>( this ) )
            {
                /*
                 * Chained node is part of the tree and is not root node, case A.
                 */
            }
            else
            {
                /*
                 * Chained node is pointing to a non chained node as its parent.
                 */
//                 cerr << "reclaimobj2A  rc:" << cc->ref_count << endl;
//                 cerr << "reclaimobj2A rdn:" << cc->getDirName() << endl;
                Items_t::iterator ci = getItemIter( cc->getDirName() );
//            iterator ci = find( cc->getDirName() );
//                cerr << "reclaimobj2B" << endl;
                if( *ci != cc )
                {
//                     cerr << "reclaimobj2C rc:" << cc->ref_count
//                          << " SC:" << (dynamic_cast<SortedContext*>(cc)!=0)
//                          << endl;

                    /*
                     * Check reference counts for root CC node.
                     */
                    if( dynamic_cast<SortedContext*>( cc ) )
                    {
                        if( cc->ref_count > 2 )
                            return false;
                    }
                    else if( cc->ref_count > 1 )
                        return false;
                
                    /*
                     * Chained node is pointing to a non chained node as its parent.
                     */
#ifdef FERRIS_DEBUG_VM
                    DEBUG_dumpcl("Context::reclaimContextObject(cc root 1)" );
#endif
                    
                    /*
                     * WARNING: if you delete the Delegate object then the ChainedContext
                     * can no longer function. So don't try to dump the context list between
                     * wipe of delegate and deletion of cc
                     */
                    cc->Delegate = 0;
//                    cerr << "Cleared delegate, deleteing object" << endl;
                    delete cc;
#ifdef FERRIS_DEBUG_VM
                    DEBUG_dumpcl("Context::reclaimContextObject(cc root done)");
#endif
                    return true;
                }
            }
        }
        

        if( !child->isReClaimable())
        {
            cerr << "child not claimable!" << endl;
            return false;
        }
        
#ifdef FERRIS_DEBUG_VM
        DEBUG_dumpcl("Context::reclaimContextObject(1)");
        cerr << "reclaimobj4 parent.rc:" << ref_count << endl;
#endif

        /*
         * Remove this child.
         */
        LG_VM_W << "Context::reclaimContextObject() removing impl for "
                 << " path:" << child->getDirPath() << endl;
//    dumpOutItems();

        LG_VM_W << "Context::reclaimContextObject(bump) c:" << toVoid(child) << " rc:" << child->ref_count << endl;

        // Keep a reference to the child so that it is not collected by having a zero rc.
        child->ref_count+=1000;
//         cerr << "reclaimobj4 -- have temp ref" << endl;
//         cerr << "reclaimobj4.1 parent.rc:" << ref_count << endl;
//         cerr << "reclaimobj4.1 this:" << toVoid(this) << " child:" << toVoid(child)  << endl;
        Remove( child, false );
//         cerr << "reclaimobj4.2 parent.rc:" << ref_count << endl;
        // The context still exists, so we keep knowing about it.
        // thus we must counter the decrement done in Remove().
        ++NumberOfSubContexts;
//         cerr << "reclaimobj5 parent.rc:" << ref_count << endl;

        LG_VM_D << "About to delete child:" << rdn << endl;

#ifdef FERRIS_DEBUG_VM
        DEBUG_dumpcl("Context::reclaimContextObject(2) about to drop parent ref");
#endif
        child->ParentContext = 0;

#ifdef FERRIS_DEBUG_VM
        DEBUG_dumpcl("Context::reclaimContextObject(2.2)");
#endif

        if( ChainedViewContext* cc = dynamic_cast<ChainedViewContext*>(child) )
        {
#ifdef FERRIS_DEBUG_VM
            cerr << "reclaimobj7.1 cc:" << toVoid(cc) << endl;
            cerr << "reclaimobj7.1 cc->del:" << toVoid( GetImpl( cc->Delegate ) ) << endl;
#endif
            cc->Delegate = 0;
        }

#ifdef FERRIS_DEBUG_VM
        cerr << "reclaimobj8" << endl;
#endif
//         DEBUG_dumpcl("Context::reclaimContextObject(2.3)");
//         cerr << "reclaimobj9" << endl;
        delete child;

#ifdef FERRIS_DEBUG_VM
        cerr << "reclaimobj10" << endl;
        DEBUG_dumpcl("Context::reclaimContextObject(3)");
        LG_VM_D << "... reclaimContextObject() delete done1.1" << endl;
#endif
        
        bumpVersion();
        getCacheManager()->removeFromFreeList( child, true );
        return true;
    }


/**
 * Either return the context with the given rdn, or create a new context
 * with the given rdn and attach it as a subcontext before returning it.
 *
 * @see priv_getSubContext()
 * @see isSubContextBound()
 * @see bumpVersion()
 *
 * @param rdn The rdn of the context to return/create
 * @param checkIfExistsAlready If the plugin has already checked if the subcontext exists
 *        then it can pass in false here.
 * @return handle to the subcontext with the given rdn
 * @throws NoSuchSubContext
 * @throws FerrisNotSupportedInThisContext If there is no subcontext with the given
 *         rdn and one can not be created.
 */
    fh_context
    Context::priv_readSubContext( const string& rdn, bool created, bool checkIfExistsAlready )
        throw( NoSuchSubContext, FerrisNotSupportedInThisContext )
    {
        try
        {
            LG_CTX_D << "Context::priv_readSubContext(1) url:" << getURL() << endl;
            
            if( checkIfExistsAlready )
            {
                Items_t::iterator subc_iter;
                if( priv_isSubContextBound( rdn, subc_iter ) )
                {
//                  fh_context ret = priv_getSubContext( rdn );
                    fh_context ret = *subc_iter;
                    ret->setHasBeenDeleted( false );
                    Emit_Exists( 0, ret, rdn, rdn, 0 );
                    return ret;
                }
            }

            
//             cerr << "priv_readSubContext() this:" << toVoid(this)
//                  << " rdn:" << rdn
//                  << " not found"
//                  << endl;
//             dumpOutItems();
        
        
            Context* c = priv_CreateContext( this, rdn );
            LG_CTX_D << "Context::priv_readSubContext(2) url:" << getURL() << endl;
            fh_context tmp = c;
            fh_context ret = Insert( c, created );
            LG_CTX_D << "Context::priv_readSubContext(3) url:" << getURL() << endl;
            bumpVersion();
            return ret;
        }
        catch( NoSuchSubContext& e )
        {
            cerr << "Context::priv_readSubContext() e:" << e.what() << endl;
            throw e;
        }
        catch( FerrisNotSupportedInThisContext& e )
        {
            cerr << "Context::priv_readSubContext() e:" << e.what() << endl;
            throw e;
        }
        catch( exception& e )
        {
            cerr << "Context::priv_readSubContext() strange e:" << e.what() << endl;
            BackTrace();
            throw e;
        }
    }

    std::string
    Context::monsterName( const std::string& rdn )
    {
        string ret = rdn;
        string baserdn = rdn;
        int version = 1;
        
        LG_PATHS_D << "monsterName() path: " << getDirPath()
                   << " rdn:" << rdn
                   << endl;

        Items_t::iterator iter;
        while( priv_isSubContextBound( ret, iter ) )
        {
            if( (*iter)->HasBeenDeleted )
                break;

            ostringstream ss;
            ss << baserdn << "--" << version++;
            ret = tostr(ss);
        }

        LG_PATHS_D << "monsterName() path: " << getDirPath()
                   << " rdn:" << rdn
                   << " ret:" << ret
                   << endl;
        return ret;
    }
    

/**
 * Similar to priv_readSubContext() except this method does not create a context
 * and attach it if there is not one already there. If a subcontext exists then
 * it is marked as not deleted. Most of the time plugins will want to just call
 * priv_readSubContext() to ensure the sub context exists and is not marked as
 * deleted.
 *
 * Using priv_discoveredSubContext if a subcontext already exists then it is marked
 * is not deleted and returned. If no such subcontext exists then a null reference is
 * returned. This allows the plugin to create the subcontext using whatever method it
 * likes and the plugin will get a reference to the subtype of Context without having
 * to dynamic_cast<> a generic Context*
 *
 * @see clearContext()
 */
    fh_context
    Context::priv_discoveredSubContext( const string& rdn, bool created )
        throw( NoSuchSubContext, FerrisNotSupportedInThisContext )
    {
        fh_context ret = 0;
        Items_t::iterator subc_iter;
        if( priv_isSubContextBound( rdn, subc_iter ) )
        {
//            ret = priv_getSubContext( rdn );
            ret = *subc_iter;
            ret->setHasBeenDeleted( false );
            bumpVersion();
        }
        return ret;
    }
    
    

    

/**
 * If this context supports dynamic recreation of subcontexts then this
 * method ensures that the subcontexts of this context are all created.
 */
    void
    Context::UnPageSubContextsIfNeeded()
//    throw( NoSuchSubContext, FerrisNotSupportedInThisContext )
    {
//    cerr << " Context::UnPageSubContextsIfNeeded() " << endl;
    
        LG_CTX_D << "UnPageSubContextsIfNeeded(enter) path:" << getDirPath() << endl;
        LG_CTX_D << "UnPageSubContextsIfNeeded() HaveReadDir:" << getHaveReadDir()
                 << " this:" << (void*)this
                 << " omc:" << (void*)getOverMountContext()
                 << " NumberOfSubContexts:" << NumberOfSubContexts
                 << " Items.size():" << Items.size()
                 << " sitems.sz:" << getSortedItems().size()
                 << endl;

        if( !HaveReadDir )
        {
            read(0);
        }
        else if( supportsReClaim() && NumberOfSubContexts != getItems().size() )
        {
            LG_CTX_D << "UnPageSubContextsIfNeeded() nosc:" << NumberOfSubContexts
                     << " sz:" << getItems().size()
                     << endl;
            
            read(1);
        }


        LG_CTX_D << "UnPageSubContextsIfNeeded(exit) path:" << getDirPath() << endl;
    }


    int
    Context::getReadingDir() const
    {
        return ReadingDir;
    }

    int
    Context::getHaveReadDir() const
    {
        return HaveReadDir;
    }


    static fh_context getSchemaCached( const std::string& earl )
    {
        typedef FERRIS_STD_HASH_MAP< string, fh_context > cache_t;
        static cache_t cache;
        cache_t::iterator ci = cache.find( earl );
        if( ci != cache.end() )
            return ci->second;
        
        fh_context ret = Resolve( earl );
        cache[ earl ] = ret;
        return ret;
    }
    

fh_context
Context::getSchema( const std::string& eaname )
{
    if( starts_with( eaname, "schema:" ) )
    {
        return getSchemaCached( "schema://xsd/attributes/schema" );
    }
    
//    CERR << "Context::getSchema() eaname:" << eaname << endl;
    string earl = getStrAttr( this, "schema:" + eaname, "" );
    if( earl.empty() )
    {
        fh_stringstream ss;
        ss << "Can't find the schema for ea:" << eaname
           << " on context:" << getURL() << endl
//           << " eanames:" << getStrAttr( this, "ea-names", "" )
           << endl;
//        cerr << "Schema error:" << tostr(ss) << endl;
//        BackTrace();
        Throw_SchemaNotFoundException( tostr(ss), 0 );
    }

    LG_SCHEMA_D << "Context::getSchema() url:" << getURL() << " ea:" << eaname
                << " gives:" << earl << endl;

    return getSchemaCached( earl );
}

    fh_context
    Context::getSchemaOrDefault( const std::string& eaname, XSDBasic_t sct )
    {
        if( starts_with( eaname, "schema:" ) )
        {
            return getSchemaCached( "schema://xsd/attributes/schema" );
        }
    
        string earl = getStrAttr( this, "schema:" + eaname, "" );
        if( earl.empty() )
        {
            earl = Factory::getSchemaURLForType( sct );
        }
        return getSchemaCached( earl );
    }
    

fh_context
Context::getBranchFileSystem()
{
    LG_CTX_D << "Context::getBranchFileSystem() url:" << getURL() << endl;
    LG_CTX_D << "Context::getBranchFileSystem() attr:"
             << getStrAttr( this, "associated-branches-url", "" )
             << endl;
    
    fh_context ret;
    string earl = getStrAttr( this, "associated-branches-url", "" );
    if( earl.empty() )
    {
        fh_stringstream ss;
        ss << "There is no branch filesystem for context:" << getURL() << endl;
        Throw_BranchFileSystem( tostr(ss), this );
    }
    ret = Resolve( earl );
    return ret;
}




/**
 * Get the subcontext with the given rdn.
 *
 * @see priv_readSubContext()
 * @see isSubContextBound()
 *
 * @param rdn The rdn of the subcontext to get
 * @throws NoSuchSubContext If there is no subcontext with the given rdn.
 * @return the subcontext with the given rdn
 */
    fh_context
    Context::priv_getSubContext( const string& rdn )
        throw( NoSuchSubContext )
    {
        try
        {
            
            LG_CTX_D << "Context::priv_getSubContext( enter ) rdn:" << rdn
                     << " path:" << getDirPath()
                     << " this:" << (void*)this
                     << " omc:" << (void*)getOverMountContext()
                     << " items.sz:" << getItems().size()
                     << " sitems.sz:" << getSortedItems().size()
                     << endl;
            
//             if( priv_isSubContextBound( rdn ))
//                 return getItem( rdn );
            {
                Items_t::iterator subc_iter;
                if( priv_isSubContextBound( rdn, subc_iter ) )
                {
                    return *subc_iter;
                }
            }
            
            LG_CTX_D << "Context::priv_getSubContext( 2 ) rdn:" << rdn << endl;

            UnPageSubContextsIfNeeded();

            LG_CTX_D << "Context::priv_getSubContext( 3 ) rdn:" << rdn << endl;

            if( this != getOverMountContext() )
            {
                LG_CTX_D << "Context::priv_getSubContext( passing on ) path:" << getDirPath() << endl;
                return getOverMountContext()->priv_getSubContext( rdn );
            }

            LG_CTX_D << "Context::priv_getSubContext( 4 ) rdn:" << rdn << endl;
            
//             if( priv_isSubContextBound( rdn ))
//             {
//                 return getItem( rdn );
//             }
            {
                Items_t::iterator subc_iter;
                if( priv_isSubContextBound( rdn, subc_iter ) )
                {
                    return *subc_iter;
                }
            }

            LG_CTX_D << "Context::priv_getSubContext( in trouble 1 ) path:" << getDirPath()
                     << " this:" << (void*)this
                     << " omc:" << (void*)getOverMountContext()
                     << " rdn:" << rdn
                     << " items.sz:" << getItems().size()
                     << " sitems.sz:" << getSortedItems().size()
                     << endl;
            LG_CTX_D << "Context::priv_getSubContext( in trouble 1 ) path:" << getDirPath() << endl;
//             cerr << "Context::priv_getSubContext( in trouble 1 ) path:" << getDirPath() << endl;
//             dumpOutItems();
//             LG_CTX_D << "Context::priv_getSubContext( in trouble 2 ) path:" << getDirPath() << endl;
//             getOverMountContext()->dumpOutItems();
        
        }
        catch( NoSuchSubContext& e )
        {
            ostringstream ss;
            ss << "getSubContext() url:" << getURL() << " NoSuchSubContext:" << rdn;
            Throw_NoSuchSubContext( tostr(ss), this );
        }
        catch( exception& e )
        {
            ostringstream ss;
            ss << "getSubContext() url:" << getURL() << " NoSuchSubContext:" << rdn
               << " e:" << e.what();
            LG_CTX_D << tostr(ss) << endl;
            Throw_NoSuchSubContext( tostr(ss), 0 );
        }

//        BackTrace();
        ostringstream ss;
        ss << "getSubContext() url:" << getURL() << " NoSuchSubContext:" << rdn;
        Throw_NoSuchSubContext( tostr(ss), this );
    }



/**
 * Get a context that is relative to this one.
 *
 * @see getRelativeContext()
 * @see splitPath()
 * @see unSplitPath()
 *
 * @param pa Path split into chunks for relative resolution
 * @param xdn relative dn
 * @param full_xdn
 * @param f Factory for creating the final resolution of the context.
 * @throws NoSuchSubContext If there is no such relative path
 * @returns The relative context
 */
    fh_context
    Context::priv_getRelativeContext(
        SplitPath_t  pa,
        const string& xdn,
        const string& full_xdn,
        RootContextFactory* f
        )
        throw( NoSuchSubContext )
    {
        pair<string,string> p = splitPathAtStart(xdn);

        LG_CTX_D << "Context::priv_getRelativeContext()    xdn:" << xdn << endl;
        LG_CTX_D << "Context::priv_getRelativeContext()  first:" << p.first << endl;
        LG_CTX_D << "Context::priv_getRelativeContext() second:" << p.second << endl;
        LG_CTX_D << "Context::priv_getRelativeContext()     pa:" << unSplitPath(pa) << endl;

        if( p.first == ".." )
        {
            pa.pop_back();
        }
        else if( p.first == "." || p.first == "/" || p.first == "" )
        {
            // no op
        }
        else
        {
            pa.push_back( p.first );
        }
    
        if( p.second.length() )
        {
            return priv_getRelativeContext( pa, p.second, full_xdn, f );
        }


        LG_CTX_D << "----Context::priv_getRelativeContext()------------" << endl;
        LG_CTX_D << "Context::priv_getRelativeContext()      xdn:" << xdn << endl;
        LG_CTX_D << "Context::priv_getRelativeContext() full_xdn:" << full_xdn << endl;
        LG_CTX_D << "Context::priv_getRelativeContext()       pa:" << unSplitPath(pa) << endl;
        
        f->AddInfo( RootContextFactory::PATH, unSplitPath(pa) );
    
        fh_context ret = f->resolveContext( f->getResolveStyle() );
    
        LG_CTX_D << "Context::priv_getRelativeContext() ret:" << isBound(ret) << endl;
        LG_CTX_D << "Context::priv_getRelativeContext() ret:" << ret->getDirPath() << endl;
        LG_CTX_D << "----Context::priv_getRelativeContext(exit)------------" << endl;

        return ret;
    }


/**
 * Get a context that is relative to this one.
 *
 * @see priv_getRelativeContext()
 * @see splitPath()
 * @see unSplitPath()
 *
 * @param xdn relative dn to get
 * @param f Factory for creating the final resolution of the context.
 * @throws NoSuchSubContext If there is no such relative path
 * @returns The relative context
 */
fh_context
Context::getRelativeContext(
    const string& xdn,
    RootContextFactory* f
    )
    throw( NoSuchSubContext )
{
    try
    {
            
        LG_CTX_D << "Context::getRelativeContext()  xdn:" << xdn << endl;
        LG_CTX_D << "Context::getRelativeContext() path:" << getDirPath() << endl;

        SplitPath_t pa = splitPath( getDirPath() );
        bool WeAllocatedFactory = false;

        LG_CTX_D << "Context::getRelativeContext()   pa:" << unSplitPath(pa) << endl;

    
        if( !f )
        {
            WeAllocatedFactory = true;
            f = new RootContextFactory();

            f->setContextClass( "file" );
            f->AddInfo( RootContextFactory::ROOT, "/" );
        }

        fh_context ret(priv_getRelativeContext( pa, xdn, xdn, f ));

        if( WeAllocatedFactory )
        {
            delete f;
        }
    
        LG_CTX_D << "Context::getRelativeContext()  xdn:" << xdn << endl;
        LG_CTX_D << "Context::getRelativeContext()  ret:" << isBound(ret) << endl;
        LG_CTX_D << "Context::getRelativeContext() path:" << ret->getDirPath() << endl;

        return ret;
    }
    catch( exception& e )
    {
        LG_CTX_D << "Context::getRelativeContext() e:" << e.what() << endl;
        Throw_NoSuchSubContext( e.what(), this );
    }
    }


/**
 * Split a given path into a list of chunks each of which contains
 * the rdn relative to the previous list element.
 *
 * This method splits a string like:
 * /usr/local/bin/enlightenment
 * Into a list
 * {usr,local,bin,enlightenment}
 * This is done in accordance with the seperator string from getSeperator().
 *
 * @see unSplitPath()
 * @see splitPathAtStart()
 * @see getSeperator()
 *
 * @param dn The path to split.
 * @return An STL collection containing each of the rdns in order.
 */
    Context::SplitPath_t
    Context::splitPath( const string& dn )
    {
        SplitPath_t ret;

        pair<string,string> p = splitPathAtStart(dn);
        while( p.second.length() )
        {
            ret.push_back( p.first );
            p = splitPathAtStart(p.second);
        }
        ret.push_back( p.first );

        LG_CTX_D << "Context::SplitPath() dn:" << dn << endl;
//    copy( ret.begin(), ret.end(), ostream_iterator<string>(cerr));

    
        return ret;
    }


/**
 * Merge a list of relative paths into a single string.
 * This method is the exact reverse of splitPath().
 *
 * the following should be semantically true
 *
 * Given:
 * string p = "/usr/local/bin/enlightenment";
 * string p2 = unSplitPath( splitPath(p) );
 *
 * Though p2 may be a different string, the resolution of both
 * p and p2 should be the same context.
 *
 * @see splitPath()
 *
 * @param pa Split path to merge
 * @return merged path as a string.
 *
 */
    string
    Context::unSplitPath( const SplitPath_t& pa )
    {
        string s;

//     cerr << "Context::unSplitPath()" << endl;
//     copy( pa.begin(), pa.end(), ostream_iterator<string>(cerr));
    
    
        for( SplitPath_t::const_iterator iter =  pa.begin(); iter != pa.end();  )
        {
            s += *iter;
        
            iter++;
            if( iter != pa.end() )
            {
                s += getSeperator();
            }
        }

        return s;
    }


    


/**
 * Check if this context has subcontexts. Note that the context needs to have
 * been read() before this method is valid.
 *
 * @see guessSize()
 * @see read()
 * @see SubContextCount()
 * @return true if there are subcontexts.
 */
    bool
    Context::hasSubContexts()
    {
        if( getOverMountContext() != this )
        {
            return getOverMountContext()->hasSubContexts();
        }
        return NumberOfSubContexts > 0 || Items.size();
    }


/**
 * Get the actual count of subcontexts of this context.
 * Note that the context needs to have been read() before this method is valid.
 *
 * @see guessSize()
 * @see read()
 * @see hasSubContexts()
 * @return true if there are subcontexts.
 */
    int
    Context::SubContextCount()
    {
//        return getOverMountContext()->Items.size(); //NumberOfSubContexts;

        if( getOverMountContext() != this )
        {
            return getOverMountContext()->SubContextCount();
        }
        
        int ret = 0;
        for(Items_t::const_iterator ci = Items.begin(); ci != Items.end(); ++ci )
        {
            if( !(*ci)->HasBeenDeleted )
                ++ret;
        }
        return ret;
    }


/**
 * Mark all children as being deleted. Subcontexts are not actually removed
 * from this context because the user might still have a fh_context for on of
 * them or a decendant of one of our children. For plugin context modules to
 * work correctly they should call this in their read() and either
 * priv_readSubContext() or priv_discoveredSubContext() for each context that
 * they discover to still exist. Any contexts that are not marked as still existing
 * can then be claimed by the memory manager when the user is not using them.
 *
 * @see priv_readSubContext()
 * @see priv_discoveredSubContext()
 *
 */
    void
    Context::clearContext()
    {
        LG_CTX_D << "Context::clearContext() url:" << getURL() << endl;

        /**
         * Mark each child as deleted so that the context plugin has to rediscover each
         * still existing context.
         */
        for( Items_t::iterator iter = getItems().begin(); iter != getItems().end(); ++iter )
        {
            (*iter)->HasBeenDeleted = true;
        }
// 1.1.61 commented the below 6 lines.        
//         for( Items_t::iterator iter = getItems().begin(); iter != getItems().end(); )
//         {
//             fh_context tmp = *iter;
//             ++iter;
//             Remove( GetImpl(tmp), false );
//         }

        
        
//         /*
//          * Explicitly reclaim each entry
//          * CacheManager::cleanUp() ?
//          */
//         for( Items_t::iterator iter = getItems().begin(); iter != getItems().end(); )
//         {
//             Context* c = GetImpl(*iter);

//             cerr << "Context::clearContext() rc:" << c->ref_count
//                  << " getURL():" << c->getURL()
//                  << endl;
//             getCacheManager()->removeFromFreeList( c );
//             ++iter;
// //            delete c;
//         }

//        getItems().clear();
//        NumberOfSubContexts = 0;
        bumpVersion();

//         /* A slightly cleaner way to update attributes here would be nice */
//         clearAttributes();
//         AttributesHaveBeenCreated = 0;
    }


void
Context::Emit_MedallionUpdated()
{
    LG_CTX_D << "Context::Emit_MedallionUpdated() c:" << getURL() << endl;
    getNamingEvent_MedallionUpdated_Sig().emit( ThisContext() );
}



/**
 * Emit a changed signal. 
 *
 * @see getNamingEvent_Changed_Sig()
 * @see NamingEvent_Changed_Sig_t
 *
 * @param e     Event to emit
 * @param olddn The old rdn of the subcontext for this signal.
 * @param newdn The new rdn of the subcontext for this signal.
 * @param ExtraData Some additional data about this signal. This is usually user
 *                  supplied data
 */
    void
    Context::Emit_Changed( NamingEvent_Changed* e,
                           const string& olddn, const string& newdn, sigc::trackable* ExtraData )
    {
        LG_CTX_D << "--- Context::Emit_Changed() newdn:" << newdn << " path:" << getDirPath() << endl;

        NamingEvent_Changed ev( ThisContext(), ExtraData );
        getNamingEvent_Changed_Sig().emit( &ev, olddn, newdn );
    }


/**
 * Emit a deleted signal. 
 *
 * @see getNamingEvent_Deleted_Sig()
 * @see NamingEvent_Deleted_Sig_t
 *
 * @param e     Event to emit
 * @param olddn The old rdn of the subcontext for this signal.
 * @param newdn The new rdn of the subcontext for this signal.
 * @param ExtraData Some additional data about this signal. This is usually user
 *                  supplied data
 */
    void
    Context::Emit_Deleted( NamingEvent_Deleted* e,
                           string olddn, string newdn, sigc::trackable* ExtraData )
    {
        NamingEvent_Deleted ev( ThisContext(), ExtraData );
        getNamingEvent_Deleted_Sig().emit( &ev, olddn, newdn );
    }

/**
 * Emit a start execute signal. 
 *
 * @see getNamingEvent_Start_Execute_Sig()
 * @see NamingEvent_Start_Execute_Sig_t
 *
 * @param e     Event to emit
 * @param olddn The old rdn of the subcontext for this signal.
 * @param newdn The new rdn of the subcontext for this signal.
 * @param ExtraData Some additional data about this signal. This is usually user
 *                  supplied data
 */
    void
    Context::Emit_Start_Execute( NamingEvent_Start_Execute* e,
                                 string olddn, string newdn, sigc::trackable* ExtraData )
    {
        NamingEvent_Start_Execute ev( ThisContext(), ExtraData );
        getNamingEvent_Start_Execute_Sig().emit( &ev, olddn, newdn );
    }


/**
 * Emit a stop execute signal. 
 *
 * @see getNamingEvent_Stop_Execute_Sig()
 * @see NamingEvent_Stop_Execute_Sig_t
 *
 * @param e     Event to emit
 * @param olddn The old rdn of the subcontext for this signal.
 * @param newdn The new rdn of the subcontext for this signal.
 * @param ExtraData Some additional data about this signal. This is usually user
 *                  supplied data
 */
    void
    Context::Emit_Stop_Execute( NamingEvent_Stop_Execute* e,
                                string olddn, string newdn, sigc::trackable* ExtraData )
    {
        NamingEvent_Stop_Execute ev( ThisContext(), ExtraData );
        getNamingEvent_Stop_Execute_Sig().emit( &ev, olddn, newdn );
    }


/**
 * Emit a created signal. 
 *
 * @see getNamingEvent_Created_Sig()
 * @see NamingEvent_Created_Sig_t
 *
 * @param e     Event to emit
 * @param olddn The old rdn of the subcontext for this signal.
 * @param newdn The new rdn of the subcontext for this signal.
 * @param ExtraData Some additional data about this signal. This is usually user
 *                  supplied data
 */
    void
    Context::Emit_Created( NamingEvent_Created* e,
                           const fh_context& newc,
                           string olddn, string newdn, sigc::trackable* ExtraData )
    {
        LG_CTX_D << "Context::Emit_Created() rdn:" << olddn << endl;
        if( isBound( newc ) )
            LG_CTX_D << "Context::Emit_Created() c:" << toVoid(GetImpl(newc)) << endl;

        NamingEvent_Created ev( ThisContext(), ExtraData );
        getNamingEvent_Created_Sig().emit( &ev, newc, olddn, newdn );
    }


/**
 * Emit a moved signal. 
 *
 * @see getNamingEvent_Moved_Sig()
 * @see NamingEvent_Moved_Sig_t
 *
 * @param e     Event to emit
 * @param olddn The old rdn of the subcontext for this signal.
 * @param newdn The new rdn of the subcontext for this signal.
 * @param ExtraData Some additional data about this signal. This is usually user
 *                  supplied data
 */
    void
    Context::Emit_Moved( NamingEvent_Moved* e,
                         string olddn, string newdn, sigc::trackable* ExtraData )
    {
        NamingEvent_Moved ev( ThisContext(), ExtraData );
        getNamingEvent_Moved_Sig().emit( &ev, olddn, newdn );
    }

    fh_context
    Context::ThisContext()
    {
        fh_context ret = this;
        return ret;
    }
    
/**
 * Emit a exists signal. 
 *
 * @see getNamingEvent_Exists_Sig()
 * @see NamingEvent_Exists_Sig_t
 *
 * @param e     Event to emit
 * @param olddn The old rdn of the subcontext for this signal.
 * @param newdn The new rdn of the subcontext for this signal.
 * @param ExtraData Some additional data about this signal. This is usually user
 *                  supplied data
 */
    void
    Context::Emit_Exists( NamingEvent_Exists* e,
                          const fh_context& newc,
                          string olddn, string newdn, sigc::trackable* ExtraData )
    {
        LG_CTX_D << "Emit_Exists for path:" << getDirPath() 
                 << " olddn:" << olddn << endl;

        NamingEvent_Exists ev( ThisContext(), ExtraData );
        getNamingEvent_Exists_Sig().emit( &ev, newc, olddn, newdn );
    }


/**
 * Emit a start reading context signal. 
 *
 * @see getNamingEvent_Start_Reading_Context_Sig()
 * @see NamingEvent_Start_Reading_Context_Sig_t
 *
 * @param e     Event to emit
 * @param olddn The old rdn of the subcontext for this signal.
 * @param newdn The new rdn of the subcontext for this signal.
 * @param ExtraData Some additional data about this signal. This is usually user
 *                  supplied data
 */
    void
    Context::Emit_Start_Reading_Context( NamingEvent_Start_Reading_Context* e,
                                         sigc::trackable* ExtraData )
    {
        NamingEvent_Start_Reading_Context ev( ThisContext(), ExtraData );
        getNamingEvent_Start_Reading_Context_Sig().emit( &ev );
    }


/**
 * Emit a stop reading context signal. 
 *
 * @see getNamingEvent_Stop_Reading_Context_Sig()
 * @see NamingEvent_Stop_Reading_Context_Sig_t
 *
 * @param e     Event to emit
 * @param olddn The old rdn of the subcontext for this signal.
 * @param newdn The new rdn of the subcontext for this signal.
 * @param ExtraData Some additional data about this signal. This is usually user
 *                  supplied data
 */
void
Context::Emit_Stop_Reading_Context( NamingEvent_Stop_Reading_Context* e,
                                    sigc::trackable* ExtraData )
{
    NamingEvent_Stop_Reading_Context ev( ThisContext(), ExtraData );
    getNamingEvent_Stop_Reading_Context_Sig().emit( &ev );
}


///////////////////////////////////////////////////////////////////////////////
//
//
// These are documented in MutableCollectionEvents
//
//
///////////////////////////////////////////////////////////////////////////////

Context::NamingEvent_MedallionUpdated_Sig_t&
Context::getNamingEvent_MedallionUpdated_Sig()
{
    return NamingEvent_MedallionUpdated_Sig;
}

Context::NamingEvent_Changed_Sig_t&
Context::getNamingEvent_Changed_Sig()
{
    return NamingEvent_Changed_Sig;
}

Context::NamingEvent_Deleted_Sig_t&
Context::getNamingEvent_Deleted_Sig()
{
    return getCoveredContext()->NamingEvent_Deleted_Sig;
}

    Context::NamingEvent_Start_Execute_Sig_t&
    Context::getNamingEvent_Start_Execute_Sig()
    {
        return NamingEvent_Start_Execute_Sig;
    }

    Context::NamingEvent_Stop_Execute_Sig_t&
    Context::getNamingEvent_Stop_Execute_Sig()
    {
        return NamingEvent_Stop_Execute_Sig;
    }

    Context::NamingEvent_Created_Sig_t&
    Context::getNamingEvent_Created_Sig()
    {
        return getCoveredContext()->NamingEvent_Created_Sig;
    }

    Context::NamingEvent_Moved_Sig_t&
    Context::getNamingEvent_Moved_Sig()
    {
        return NamingEvent_Moved_Sig;
    }


    Context::NamingEvent_Exists_Sig_t&
    Context::getNamingEvent_Exists_Sig()
    {
        return getCoveredContext()->NamingEvent_Exists_Sig;
    }

    Context::NamingEvent_Start_Reading_Context_Sig_t&
    Context::getNamingEvent_Start_Reading_Context_Sig()
    {
        return NamingEvent_Start_Reading_Context_Sig;
    }

Context::NamingEvent_Stop_Reading_Context_Sig_t&
Context::getNamingEvent_Stop_Reading_Context_Sig()
{
    return NamingEvent_Stop_Reading_Context_Sig;
}

Context::ContextEvent_Headers_Received_Sig_t&
Context::getContextEvent_Headers_Received_Sig()
{
    return ContextEvent_Headers_Received_Sig;
}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Check if this is the virginal call to updateMetaData().
 *
 * @see updateMetaData()
 * @returns true if updateMetaData() has not been called for this context in
 *          the past.
 */
    gboolean
    Context::getUpdateMetaData_First_Time()
    {
        gboolean ret = updateMetaData_First_Time;
        updateMetaData_First_Time = 0;
        return ret;
    }


/**
 * Called when a context needs to update this metadata. This bumps version so
 * that any data generators attached to the attribute know that they must check
 * to see if a refresh is needed for their value.
 *
 * When the metadata is being updated a changed signal is also emitted.
 */
    void
    Context::updateMetaData()
    {
        ensureUpdateMetaDataCalled_virgin = 0;
        if( !getUpdateMetaData_First_Time() )
        {
            if( !getCacheManager()->insideCleanupCall() )
            {
                LG_CTX_D << "emit_changed... getDirPath()" << getDirPath() << endl;
                Emit_Changed( 0, getDirPath(), getDirPath(), 0 );
            }
        }
    }

/**
 * Ensure that however many times this method is called updateMetaData() is only
 * ever called once.
 */
    void
    Context::ensureUpdateMetaDataCalled()
    {
        if( ensureUpdateMetaDataCalled_virgin )
        {
            updateMetaData();
        }
    }



/**
 * Get the subcontext count as a string. Used in EA generators.
 * return The sub context count string.
 */
    fh_istream
    Context::getSubContextCountStream( Attribute* attr )
    {
        fh_stringstream ss;
        ss << Items.size(); //NumberOfSubContexts;
        return ss;
    }


/**
 * Get the attribute count as a string. Used in EA generators.
 * return The attr count string.
 */
    fh_istream
    Context::getAttributeCountStream( Attribute* attr )
    {
        fh_stringstream ss;
        ss << Attributes.size();
        return ss;
    }


/**
 * Get the dirname as a string. Used in EA generators.
 * return The dirname string.
 */
    fh_istream
    Context::getDirNameStream( Attribute* attr )
    {
        fh_stringstream ss;
        ss << getDirName();
        return ss;
    }

/**
 * Get the dirname extension as a string.
 * The extension is defined as any non '.' characters after the last dot
 * in the string.
 * 
 * Used in EA generators.
 * return The extension of the dirname string.
 */
    fh_istream
    Context::getDirNameExtensionStream( Attribute* attr )
    {
        fh_stringstream ss;
        ss << getNameExtension();
        return ss;
    }


/**
 * Get the dirpath as a string. Used in EA generators.
 * return The dirpath string.
 */
    fh_istream
    Context::getDirPathStream( Attribute* attr )
    {
        fh_stringstream ss;
        ss << getDirPath();
        return ss;
    }

/**
 * Get a string that has a list of all the EA Names that this attribute
 * contains. Each attribute is seperated with a "," character.
 *
 * @see priv_getAttributeNames()
 * @see getAttributeNames()
 * @param attr Not Used.
 * @return string with a list of attribute names seperated with a "," char
 */
    fh_istream
    Context::getEANamesStream( Attribute* attr )
    {
        fh_stringstream ss;
        AttributeNames_t an;
        getAttributeNames( an );
        ss << Util::createCommaSeperatedList( an );
        return ss;
    }


/**
 * Reset the start reading flag to a default of not reading. This call prepares
 * the context to be read and allows a NamingEvent_Start_Reading_Context_Sig_t
 * signal to be emitted using EnsureStartReadingIsFired().
 * 
 * @see EnsureStartReadingIsFired()
 * @see EnsureStopReadingIsFired()
 */
    void
    Context::ClearStartReadingFlag()
    {
        FiredStartReading = 0;
    }

/**
 * If a start reading event has not yet been fired, fire one.
 * 
 * @see EnsureStopReadingIsFired()
 * @see ClearStartReadingFlag()
 */
    void
    Context::EnsureStartReadingIsFired()
    {
        if( !FiredStartReading )
        {
            Emit_Start_Reading_Context( 0, 0 );
            FiredStartReading = 1;
        }
    }

/**
 * If a start reading event has been fired, then fire a stop reading event.
 * 
 * @see EnsureStartReadingIsFired()
 * @see ClearStartReadingFlag()
 */
    void
    Context::EnsureStopReadingIsFired()
    {
        if( FiredStartReading )
        {
            Emit_Stop_Reading_Context( 0, 0 );
            ClearStartReadingFlag();
        }
    }

/**
 * Find a overmounting context for this context.
 *
 * @return The context that is willing to mount over this one.
 */
    fh_context
    Context::findOverMounter( bool attemptingOverMountOnlyToFindEA )
    {
        LG_CTX_D << "findOverMounter() path:" << getDirPath()
                 << " attemptingOverMountOnlyToFindEA:" << attemptingOverMountOnlyToFindEA
                 << endl;

//        BackTrace();
//        sleep(3);

        RootContextFactory f;
        return f.findOverMounter( this, 64, attemptingOverMountOnlyToFindEA );
    }

    /**
     * Check if there is a context that might be able to read this one as
     * a directory.
     */
    bool
    Context::hasOverMounter()
    {
        if( m_overMountAttemptHasAlreadyFailed )
            return false;
        
        RootContextFactory f;
        bool ret = f.hasOverMounter( this );

        //
        // treat not having an overmounter for the context as failure
        //
        if( !ret )
        {
            m_overMountAttemptHasAlreadyFailed = true;
            m_overMountAttemptHasAlreadyFailedEAOnly = true;
        }
        

        return ret;
    }

    void
    Context::setOverMountAttemptHasAlreadyFailed( bool v )
    {
        m_overMountAttemptHasAlreadyFailed = v;
    }
    
    

Attribute::Parent_t
Context::getParent() throw (FerrisParentNotSetError)
{
//     cerr << "Context::getParent() this:" << toVoid(this)
//          << " cc:" << toVoid(CoveredContext)
//          << " om:" << toVoid(OverMountContext_Delegate)
//          << " pbound:" << Attribute::isParentBound()
//          << endl;
//     if( OverMountContext_Delegate )
//     {
//         cerr << " ombound:" << OverMountContext_Delegate->isParentBound() << endl;
//     }
//     if( CoveredContext )
//     {
//         cerr << " ccbound:" << CoveredContext->isParentBound() << endl;
//     }
        
    if( isBound( CoveredContext ) )
    {
        return CoveredContext->getParent();
    }
    Parent_t ret = Attribute::getParent();

    if( ret->CoveredContext )
        return GetImpl(ret->CoveredContext);
    return ret;
}

bool Context::isParentBound()
{
//     cerr << "Context::isParentBound() this:" << toVoid(this)
//          << " cc:" << toVoid(CoveredContext)
//          << " om:" << toVoid(OverMountContext_Delegate)
//          << " pbound:" << Attribute::isParentBound()
//          << endl;
//     if( OverMountContext_Delegate )
//     {
//         cerr << " ombound:" << OverMountContext_Delegate->isParentBound() << endl;
//     }
    
//     if( isBound( CoveredContext ) )
//     {
//         return CoveredContext->isParentBound();
//     }
//    return Attribute::isParentBound();

    return Attribute::isParentBound();
}

/**
 * Look for an overmounter and attach it as the overmount for this
 * context if one exists that is willing to mount this context as
 * a filesystem
 *
 * @param silentIgnore if set then ignore the posibilty that there is no
 *                     overmount available for this context
 * @throws FerrisNotReadableAsContext if it cant be mounted
 */
void
Context::tryToOverMount( bool silentIgnore, bool attemptingOverMountOnlyToFindEA )
{
    LG_OVERMOUNT_D << "Context::tryToOverMount url:" << getURL()
                   << " attemptingOverMountOnlyToFindEA:" << attemptingOverMountOnlyToFindEA
                   << " m_overMountAttemptHasAlreadyFailed:" << m_overMountAttemptHasAlreadyFailed
                   << " m_overMountAttemptHasAlreadyFailedEAOnly:" << m_overMountAttemptHasAlreadyFailedEAOnly
                   << " isBound(OverMountContext_Delegate):" << isBound(OverMountContext_Delegate)
                   << " isBound( CoveredContext ):" << isBound( CoveredContext )
                   << endl;
    
    
    //
    // Protect ourself against trying to overmount the same context many times
    // by pushing new attempts onto pendingOverMounts and poping them after the
    // attempt is complete
    //
    typedef list< Context* > pendingOverMounts_t;
    static pendingOverMounts_t pendingOverMounts;
    pendingOverMounts_t::iterator OurPendingOverMountIter = pendingOverMounts.end();

    try
    {
        if( isBound(OverMountContext_Delegate) || isBound( CoveredContext ))
            return;
        if( m_overMountAttemptHasAlreadyFailed )
            return;
        if( attemptingOverMountOnlyToFindEA && m_overMountAttemptHasAlreadyFailedEAOnly )
            return;

        if( pendingOverMounts.end() != find( pendingOverMounts.begin(),
                                             pendingOverMounts.end(),
                                             this ))
        {
            LG_OVERMOUNT_D << "tryToOverMount(already pending) this:" << toVoid( this )
                           << " OverMountContext_Delegate:" << toVoid( GetImpl( OverMountContext_Delegate ))
                           << " isBound(om):" << isBound(OverMountContext_Delegate)
                           << " CoveredContext:" << toVoid( GetImpl( CoveredContext ))
                           << " m_overMountAttemptHasAlreadyFailed:" << m_overMountAttemptHasAlreadyFailed
                           << endl;
            return;
        }

        if( disableOverMountingForContext() )
        {
            LG_OVERMOUNT_I << "Context::tryToOverMount() forced off by plugin for this url:" << getURL() << endl;
            m_overMountAttemptHasAlreadyFailed = true;
            return;
        }
        

        pendingOverMounts.push_back( this );
        OurPendingOverMountIter = pendingOverMounts.end();
        --OurPendingOverMountIter;
        
//         LG_CTX_D << "tryToOverMount(1) this:" << toVoid( this )
//              << " OverMountContext_Delegate:" << toVoid( GetImpl( OverMountContext_Delegate ))
//              << " isBound(om):" << isBound(OverMountContext_Delegate)
//              << " CoveredContext:" << toVoid( GetImpl( CoveredContext ))
//              << " m_overMountAttemptHasAlreadyFailed:" << m_overMountAttemptHasAlreadyFailed
//              << endl; 
//         BackTrace();

//         cerr << "Context::tryToOverMount() path:" << getDirPath() << endl;
//         BackTrace();

        LG_OVERMOUNT_I << "Context::tryToOverMount() actually trying overmount... path:" << getDirPath() << endl;
        {
            // Short out for testing purposes
//          if( !contains( getURL(), ".ferris" ) )
//              return;
        }
        
        
        fh_context c = findOverMounter( attemptingOverMountOnlyToFindEA );
        if( !c )
        {
            if( attemptingOverMountOnlyToFindEA )
                return;
        }
//         CERR << "tryToOverMount(success) this:" << toVoid( this ) << " success!" << endl;
//         CERR << "tryToOverMount(2) this:" << toVoid( this )
//                  << " OverMountContext_Delegate:" << toVoid( GetImpl( OverMountContext_Delegate ))
//                  << " isBound(om):" << isBound(OverMountContext_Delegate)
//                  << " CoveredContext:" << toVoid( GetImpl( CoveredContext ))
//                  << " m_overMountAttemptHasAlreadyFailed:" << m_overMountAttemptHasAlreadyFailed
//                  << " c:" << toVoid( GetImpl(c) )
//                  << endl;
        
        LG_CTX_D << "tryToOverMount(2) this:" << toVoid( this )
                 << " OverMountContext_Delegate:" << toVoid( GetImpl( OverMountContext_Delegate ))
                 << " isBound(om):" << isBound(OverMountContext_Delegate)
                 << " CoveredContext:" << toVoid( GetImpl( CoveredContext ))
                 << " m_overMountAttemptHasAlreadyFailed:" << m_overMountAttemptHasAlreadyFailed
                 << " c:" << toVoid( GetImpl(c) )
                 << endl;
        setOverMountContext( c );

//        c->setAttributeContext( this, "" );
//        c->setContext( this, "" );
//         c->theParent     = this;
//        c->ParentContext = this;

        if( OurPendingOverMountIter != pendingOverMounts.end() )
            pendingOverMounts.erase( OurPendingOverMountIter );
    }
    catch( FerrisNotReadableAsContext& e )
    {
        LG_CTX_W << "Context::tryToOverMount() url:" << getURL() << " e:" << e.what() << endl;
        if( OurPendingOverMountIter != pendingOverMounts.end() )
            pendingOverMounts.erase( OurPendingOverMountIter );

        if( !attemptingOverMountOnlyToFindEA )
            m_overMountAttemptHasAlreadyFailed = true;
        m_overMountAttemptHasAlreadyFailedEAOnly = true;
        
        if( silentIgnore )
            return;
        throw;
    }
    catch( exception& e )
    {
        if( OurPendingOverMountIter != pendingOverMounts.end() )
            pendingOverMounts.erase( OurPendingOverMountIter );

        if( !attemptingOverMountOnlyToFindEA )
            m_overMountAttemptHasAlreadyFailed = true;
        m_overMountAttemptHasAlreadyFailedEAOnly = true;

        if( silentIgnore )
            return;
        throw;
    }
        
}


/**
 * read an overmounted context
 */
void
Context::readOverMount()
{
    try
    {
        fh_context c = getOverMountContext();
        c->ClearStartReadingFlag();
        c->HaveReadDir=1;
        LG_CTX_D << "Context::readOverMount() set haveReadDir to true url:" << getURL() << endl;
    
        c->getNamingEvent_Stop_Reading_Context_Sig().connect(sigc::mem_fun( *this, &Context::ReadDone ));
        getOverMountContext()->priv_read();
        while(ReadingDir)
        {
            Main::processEvent();
        }
        LG_CTX_D << "Context::readOverMount() url:" << getURL()
                 << " read context:" << toVoid( c )
                 << " size:" << getItems().size()
                 << endl;
    }
    catch( FerrisNotReadableAsContext& e )
    {
        LG_CTX_D << "FerrisNotReadableAsContext url:" << getURL() << endl;
        LG_CTX_D << "FerrisNotReadableAsContext e:" << e.what() << endl;
        ReadingDir = 0;
        throw e;
    }
    catch( exception& e )
    {
        LG_CTX_I << "%%%%%%%%%%%%%%%%%% findOverMounter() catch() set ReadDone:" << getDirPath() << endl;
//                cerr << "e:" << e.what() << endl;
        ReadingDir = 0;
        throw;
    }
    catch( ... )
    {
//             cerr << "%%%%%%%%%%%%%%%%%% findOverMounter() this:" << this << endl;
        LG_CTX_I << "%%%%%%%%%%%%%%%%%% findOverMounter() catch() set ReadDone:" << getDirPath() << endl;
        ReadingDir = 0;
        throw;
    }
}



/**
 * Read this context.
 *
 * Note that this method may cause an overmounter to read over this context.
 *
 * This method does not explicitly have to be called in many cases;
 * getSubContextNames() and getSubContext() will both read the context if it
 * has not been read when they are called.
 *
 * @see NamingEvent_Start_Reading_Context_Sig_t
 * @see NamingEvent_Stop_Reading_Context_Sig_t
 * @see NamingEvent_Exists_Sig_t
 *
 * @see getNamingEvent_Start_Reading_Context_Sig()
 * @see getNamingEvent_Stop_Reading_Context_Sig()
 * @see getNamingEvent_Exists_Sig()
 *
 * @see getSubContextNames()
 * @see getSubContext()
 *
 * @param force If false and read() has been called on this context previously then
 *              the context is not read again.
 */
    void
    Context::read( bool force )
    {
        if( ReadingDir )
        {
            fh_stringstream ss;
            ss << "Warning. Already reading dir:" << getURL() << endl;
            cerr << tostr(ss) << endl;
            LG_CTX_ER << tostr(ss) << endl;
            BackTrace();
            return;
        }

        if( getOverMountContext() != this )
        {
            readOverMount();
            return;
        }
        
        ClearStartReadingFlag();
    
        if( HaveReadDir && isActiveView() && !force )
        {
//             cerr << "Context::read(" << force << ") url:" << getURL() << " already read"
//                  << " items size:" << getItems().size()
//                  << " omitems size:" << getOverMountContext()->getItems().size()
//                  << endl;
//             cerr << "local.dump" << endl;
//             dumpOutItems();
//             cerr << "omc.dump" << endl;
//             getOverMountContext()->dumpOutItems();
            
//             LG_CTX_D << "Warning: Already read dir and not forcing. returning:"
//                      << getDirPath() << endl;
//             LG_CTX_D << " items size:" << getItems().size() << endl;
//             LG_CTX_D << " omitems size:" << getOverMountContext()->getItems().size() << endl;
//             dumpOutItems();
//             getOverMountContext()->dumpOutItems();


            EnsureStartReadingIsFired();
            emitExistsEventForEachItem();
            EnsureStopReadingIsFired();
            return;
        }

        NumberOfSubContexts=0;
        HaveReadDir=1;
        ReadingDir=1;
        LG_CTX_D << "Context::read() set haveReadDir to true. url:" << getURL() << endl;
//        cerr << "Context::read(1) force:" << force << " dir:" << getURL() << endl;

        try
        {
            /*
             * VM.clean.1: start of an active read()
             */
            LG_VM_D << "CALLING getCacheManager()->AutoClean()... "
                    << " from Context::read()" << endl;
            getCacheManager()->AutoClean();
            
//             cerr << "Context::read(" << force << ") url:" << getURL()
//                  << " this:" << toVoid( this )
//                  << " calling priv_read()" << endl;
//            BackTrace();
            
            LG_CTX_N << "Context::read(" << force << ") url:" << getURL()
                     << " calling priv_read() nsc:" << NumberOfSubContexts << endl;
            priv_read();
            while(ReadingDir)
            {
//                Main::processEvent();                   // FAILS (on fedora 9)
//                Main::processAllPending_VFSFD_Events(); // FAILS
//                Main::processAllPendingEvents(); // OK

                g_main_iteration( false );
                Main::processAllPending_VFSFD_Events();
            }
            LG_CTX_N << "Context::read(" << force << ") url:" << getURL()
                     << " completed priv_read() nsc:" << NumberOfSubContexts
                     << " getItems().size():" << getItems().size()
                     << endl;
            NumberOfSubContexts = getItems().size();

            OnReadComplete_setupUserOverlayLinks();
        }
        catch( FerrisNotReadableAsContext& e )
        {
            LG_CTX_D << "Context::read(" << force << ") url:" << getURL()
                     << " FerrisNotReadableAsContext e:" << e.what() << endl;
            LG_OVERMOUNT_I << "Context::read(" << force << ") url:" << getURL()
                           << " FerrisNotReadableAsContext e:" << e.what() << endl;
            ReadingDir=0;
            try 
            {
                tryToOverMount( false );
                readOverMount();
            }
            catch( FerrisNotReadableAsContext& e )
            {
                throw;
            }
        }
        catch( exception& e )
        {
            cerr << "read() e:" << e.what() << endl;
            ReadingDir=0;
            throw;
        }
    }

struct UserOverlayLinkData
{
    string m_target;
    string m_linkname;
    fh_context m_configCtx;

    UserOverlayLinkData( fh_context c )
        :
        m_configCtx( c )
        {
            m_target   = getStrSubCtx( c, "target",    "" );
            m_linkname = getStrSubCtx( c, "link-name", "" );
        }
};


/**
 * The user might want some virtual softlinks to exist automatically,
 * this method is called from read() to set those up.
 *
 * For example, I might want
 * gphoto://camera
 * to point to
 * gphoto://Canon G10
 */
void
Context::OnReadComplete_setupUserOverlayLinks()
{
    LG_USEROVERLAY_D << "OnReadComplete_setupUserOverlayLinks()" << endl;

    //
    // regex dir url: existing-target link-name
    //
    static bool loadedConfig = false;
    typedef list< pair< fh_rex, UserOverlayLinkData > > m_UserOverlayLinkConfig_t;
    static m_UserOverlayLinkConfig_t m_UserOverlayLinkConfig;
    if( !loadedConfig )
    {
        loadedConfig = true;

        try
        {
            fh_context r = Resolve(getDotFerrisPath() + "user-overlay-links.xml/user-overlay-links");
            fh_context byRegex = r->getSubContext("link-by-regex");

            for( Context::iterator ci = byRegex->begin(); ci != byRegex->end(); ++ci )
            {
                fh_context c = *ci;
                string rstr = getStrSubCtx( c, "match", "" );
                LG_USEROVERLAY_D << "vlink rex:" << rstr << endl;
                if( rstr.empty() )
                    continue;
                
                UserOverlayLinkData d( c );
                fh_rex r = toregexh( rstr );
                m_UserOverlayLinkConfig.push_back( make_pair( r, d ));
            }
        }
        catch( exception& e )
        {
            LG_USEROVERLAY_W << "user-overlay-config, error:" << e.what() << endl;
        }
    }
    
    //
    // If any of the regex matches, create the virtual softlink
    //
    if( m_UserOverlayLinkConfig.empty() )
    {
        LG_USEROVERLAY_D << "OnReadComplete_setupUserOverlayLinks() no links specified by user..." << endl;
        return;
    }
    
    
    string earl = getURL();
    LG_USEROVERLAY_D << "testing m_UserOverlayLinkConfig.sz:" << m_UserOverlayLinkConfig.size() << " earl:" << earl << endl;
    for( m_UserOverlayLinkConfig_t::iterator iter = m_UserOverlayLinkConfig.begin();
         iter != m_UserOverlayLinkConfig.end(); ++iter )
    {
        if( regex_match( earl, iter->first ) )
        {
            LG_USEROVERLAY_D << "have match!" << endl;
            
            try
            {
                bool setupEventConnections = true;
                string        rdn = iter->second.m_linkname;
                string targetURL  = iter->second.m_target;

                Items_t::iterator subc_iter;
                if( priv_isSubContextBound( rdn, subc_iter ) )
                {
                    fh_context ret = *subc_iter;
                    ret->setHasBeenDeleted( false );
                    Emit_Exists( 0, ret, rdn, rdn, 0 );
                }
                else
                {
                
                    LG_USEROVERLAY_D << "trying1 to make link rdn:" << rdn << " target:" << targetURL << endl;
                    if( !contains( targetURL, "://" ))
                        targetURL = getURL() + "/" + targetURL;
                    LG_USEROVERLAY_D << "trying2 to make link rdn:" << rdn << " target:" << targetURL << endl;
                    fh_context target = Resolve( targetURL );
                    fh_context parent = this;

                    LG_USEROVERLAY_D << "Creating link rdn:" << rdn << " target:" << target->getURL() << endl;
                    fh_context nc =
                        new VirtualSoftlinkContext( parent, target, rdn,
                                                    setupEventConnections );
                    Insert( GetImpl(nc) );
                }
            }
            catch( exception& e )
            {
                LG_USEROVERLAY_W << "user-overlay-create-link, error:" << e.what() << endl;
            }
        }
    }
}



/**
 * Virtual method for subclasses to override to perform the read operation.
 *
 *
 * Note that this method may cause an overmounter to read over this context.
 *
 * @see read()
 *
 * @see NamingEvent_Start_Reading_Context_Sig_t
 * @see NamingEvent_Stop_Reading_Context_Sig_t
 * @see NamingEvent_Exists_Sig_t
 *
 * @see getNamingEvent_Start_Reading_Context_Sig()
 * @see getNamingEvent_Stop_Reading_Context_Sig()
 * @see getNamingEvent_Exists_Sig()
 *
 * @param force If false and read() has been called on this context previously then
 *              the context is not read again.
 */
    void
    Context::priv_read()
    {
        Throw_FerrisNotReadableAsContext( "", this );
    
//     EnsureStartReadingIsFired();
//     EnsureStopReadingIsFired();
    }



// /**
//  * Perform a syncronous read of this context.
//  *
//  * To not wait for the reading to complete please use read().
//  *
//  * Note that this method may cause an overmounter to read over this context.
//  *
//  * @see read()
//  *
//  * @see NamingEvent_Start_Reading_Context_Sig_t
//  * @see NamingEvent_Stop_Reading_Context_Sig_t
//  * @see NamingEvent_Exists_Sig_t
//  *
//  * @see getNamingEvent_Start_Reading_Context_Sig()
//  * @see getNamingEvent_Stop_Reading_Context_Sig()
//  * @see getNamingEvent_Exists_Sig()
//  *
//  * @param force If false and read() has been called on this context previously then
//  *              the context is not read again.
//  */
//     void
//     Context::readAndWait( bool force )
//     {
// //     cerr << "%%%%%%%%%%%%%%%%%% readAndWait this:" << this << endl;
// //     cerr << "%%%%%%%%%%%%%%%%%% readAndWait path:" << getDirPath() << endl;
// //     cerr << "Context::readAndWait() HaveReadDir :" << HaveReadDir << endl;
// //     cerr << "Context::readAndWait() ReadingDir  :" << ReadingDir << endl;

//         if( ReadingDir )
//         {
//             LG_CTX_ER << "Warning. Already reading dir:" << getDirPath() << endl;
//             cerr << "Warning. Already reading dir:" << getDirPath() << endl;
//             return;
//         }

//         if( HaveReadDir && !force )
//         {
//             LG_CTX_D << "Warning: Already read dir and not forcing. returning:"
//                      << getDirPath() << endl;
//             LG_CTX_D << " items size:" << getItems().size() << endl;
//             LG_CTX_D << " omitems size:" << getOverMountContext()->getItems().size() << endl;
//             dumpOutItems();
//             getOverMountContext()->dumpOutItems();
    
//             emitExistsEventForEachItem();
//             return;
//         }
    

//         HaveReadDir=0;
//         ReadingDir=1;
// //     cerr << "%%%%%%%%%%%%%%%%%% starting to readdir for :" << getDirPath() << endl;
//         read( force );
    
//         while(ReadingDir)
//         {
//             Main::processEvent();
//         }

//     }

/**
 * Access to a flag that is set if we are mid read() operation
 */
    bool Context::areReadingDir()
    {
        return ReadingDir;
    }



/**
 * Emit a Exists event for each item in getItems()
 */
    void
    Context::emitExistsEventForEachItem()
    {
//    Ferris::Util::ValueRestorer<bool> _dummy( ReadingDir, true );
        bool ReadingDirOld = ReadingDir;
        ReadingDir = 1;
        try
        {
            LG_CTX_D << "Context::emitExistsEventForEachItem() rdn:" << getDirPath()
                     << " ReadingDir: " << getReadingDir()
                     << endl;
        
            for( Items_t::iterator iter = getOverMountContext()->getItems().begin();
                 iter != getOverMountContext()->getItems().end();
                 ++iter )
            {
//                 cerr << "Context::emitExistsEventForEachItem() child.rdn:"
//                      << (*iter)->getDirName() << endl;
                if( !(*iter)->HasBeenDeleted )
                {
                    Emit_Exists( 0, *iter, (*iter)->getDirName(), (*iter)->getDirName(), 0 );
                }
            }
        }
        catch( exception& e )
        {
            cerr << "Context::emitExistsEventForEachItem() e:" << e.what() << endl;
            ReadingDir = ReadingDirOld;
            throw;
        }
        catch( ... )
        {
            ReadingDir = ReadingDirOld;
            throw;
        }
        ReadingDir = ReadingDirOld;
    }



/**
 * Convenience for subclasses, this method is already connected to
 * getNamingEvent_Stop_Reading_Context_Sig()
 *
 * @see getNamingEvent_Stop_Reading_Context_Sig()
 * @see NamingEvent_Stop_Reading_Context_Sig_t
 *
 * @see read()
 */
    void
    Context::ReadDone( NamingEvent_Stop_Reading_Context* src )
    {
//    cerr << "%%%%%%%%%%%%%%%%%% ReadDone() for :" << getDirPath() << endl;
        ReadingDir=0;
    }


    Handlable::ref_count_t
    Context::AddRef()
    {
        if( ref_count >= ImplementationDetail::MAX_REF_COUNT )
            return ref_count;
        
//     LG_CTX_D << "Context::add_ref() ref_count:" << ref_count << " path:" << getDirPath() << endl;
//         cerr << "Context::add_ref() rc:" << ref_count << " c:" << toVoid(this)
//              << " mc:" << getMinimumReferenceCount() << endl;

//         CERR << "Context::AddRef() this:" << (void*)this << " rdn:" << getDirName() << " rc:" << ref_count << endl;

        if( Private::haveAnyContextReferenceWatches() )
        {
            if( Private::getContextReferenceWatches().count( this ) )
            {
                cerr << "Context::AddRef()   c:" << toVoid(this)
                     << " rc:" << ref_count 
                     << " mc:" << getMinimumReferenceCount()
                     << endl;
            }
        }

        if( Private::haveAnyContextReferenceWatchesByName() &&
            Private::getContextReferenceWatchesByName().count( getDirName() ) )
        {
            cerr << "Context::AddRef(n)  c:" << toVoid(this)
                 << " rc:" << ref_count 
                 << " mc:" << getMinimumReferenceCount()
                 << endl;
            BackTrace();
        }
        if( Private::haveAnyContextParentReferenceWatches() )
        {
            if( isParentBound() && Private::getContextParentReferenceWatches().count( getParent() ) )
            {
                cerr << "Context::AddRef(P)  c:" << toVoid(this)
                     << " rc:" << ref_count 
                     << " mc:" << getMinimumReferenceCount()
                     << endl;
            }
        }
        
        if( ref_count == getMinimumReferenceCount() )
        {
//             LG_VM_D << "Context::add_ref() ref_count == getMinimumReferenceCount() "
//                     << " ref_count:" << ref_count
//                     << " min:" << getMinimumReferenceCount()
// //                    << " p:" << getDirPath()
//                     << " dirname:" << getDirName()
//                     << endl;
            RemoveOurselfFromFreeList();

//             cerr << "Context::AddRef () PROPERGATE UPWARD for c:" << toVoid(this)
//                  << " p:" << getDirPath()
//                  << " rc:" << ref_count
//                  << " minRC:" << getMinimumReferenceCount()
//                  << endl;

            if( isParentBound() && !m_holdingReferenceToParentContext )
            {
                if( Private::getContextParentReferenceWatches().count( getParent() ) )
                {
                    cerr << "Context::AddRef(q)  c:" << toVoid(this) << " Chaining up..." << endl;
                }
                
                Attribute::getParent()->AddRef();
                m_holdingReferenceToParentContext = true;
            }


//             if( isParentBound() )
//             {
//                 if( Private::getContextParentReferenceWatches().count( getParent() ) )
//                 {
//                     cerr << "Context::AddRef(q)  c:" << toVoid(this) << " Chaining up..." << endl;
//                 }
//                 Attribute::getParent()->AddRef();
//             }
            
        }

        
        Handlable::ref_count_t ret = _Base::AddRef();
        return ret;
    }

    Handlable::ref_count_t
    Context::Release()
    {
//    LG_CTX_D << "Context::rem_ref() rc:" << ref_count << " p:" << getDirPath() << endl;
//         cerr << "Context::rem_ref() rc:" << ref_count << " c:" << toVoid(this)
//              << " mc:" << getMinimumReferenceCount() << endl;


        if( ref_count >= ImplementationDetail::MAX_REF_COUNT )
            return ref_count;
            
        ref_count_t ret = Handlable::Release();

        if( Private::haveAnyContextReferenceWatches() )
        {
            if( Private::getContextReferenceWatches().count( this ) )
            {
                cerr << "Context::Release()  c:" << toVoid(this)
                     << " rc:" << ref_count 
                     << " mc:" << getMinimumReferenceCount()
                     << endl;
            }
        }
        if( Private::haveAnyContextReferenceWatchesByName() 
            && Private::getContextReferenceWatchesByName().count( getDirName() ) )
        {
            cerr << "Context::Release(n) c:" << toVoid(this)
                 << " rc:" << ref_count 
                 << " mc:" << getMinimumReferenceCount()
                 << endl;
            BackTrace();
        }
        if( Private::haveAnyContextParentReferenceWatches()  
            && isParentBound() && Private::getContextReferenceWatches().count( getParent() ) )
        {
            cerr << "Context::Release(P) c:" << toVoid(this)
                 << " rc:" << ref_count 
                 << " mc:" << getMinimumReferenceCount()
                 << endl;
//            BackTrace();
        }
        
        
        try
        {
            if( ret == getMinimumReferenceCount() )
            {
                if( isReClaimable() )
                {
//                     cerr << "Context::Release() PROPERGATE UPWARD for c:" << toVoid(this)
//                          << " p:" << getDirPath()
//                          << " rc:" << ref_count
//                          << " ret:" << ret
//                          << " minRC:" << getMinimumReferenceCount()
//                          << endl;
                    
                    TryToAddOurselfToFreeList();
                    if( isParentBound() && m_holdingReferenceToParentContext )
                    {
//                    cerr << "About to release parent. url:" << getParent()->getURL() << endl;

//                        Context* p = getParent();
                        Context* p = Attribute::getParent();

                        if( isParentBound() && Private::getContextReferenceWatches().count( getParent() ) )
                            cerr << "Context::Release(q) c:" << toVoid(this) << " Chaining up..." << endl;
                        
                        p->Release();
                        m_holdingReferenceToParentContext = false;
                        
//                        if( p->ref_count > p->getMinimumReferenceCount() )
//                              p->Release();
                    }


//                     if( isParentBound() )
//                     {
// //                    cerr << "About to release parent. url:" << getParent()->getURL() << endl;

// //                        Context* p = getParent();
//                         Context* p = Attribute::getParent();

//                         if( isParentBound() && Private::getContextReferenceWatches().count( getParent() ) )
//                             cerr << "Context::Release(q) c:" << toVoid(this) << " Chaining up..." << endl;
                        
//                         if( p->ref_count > p->getMinimumReferenceCount() )
//                               p->Release();
//                     }
                    
                }
            }
        }
        catch( exception& e )
        {
            // Dead Reference Detected (maybe)
//             cerr << "Context::Release() rc:" << ref_count
//                  << " e:" << e.what()
//                  << endl;
            LG_VM_W << "Context::Release() rc:" << ref_count
                    << " e:" << e.what()
                    << endl;
        }
        catch( ... )
        {
//             cerr << "Context::Release() rc:" << ref_count
//                  << " e ... " 
//                  << endl;
            LG_VM_W << "Context::Release() rc:" << ref_count
                    << " e ... " 
                    << endl;
        }
            

        
    
//     LG_VM_D << "Context::rem_ref() path:" << getDirPath()
//          << " rc:" << ref_count
//          << " attr_created:" << AttributesHaveBeenCreated
//          << " attr_count:" << getAttributeCount()
//          << endl;

    
        //
        // When we are only referenced by our attributes, we are dangling,
        // and can die.
        //
//     if( !AttributesHaveBeenCreated )
//         Handlable::rem_ref();
    
//     if( ref_count > getAttributeCount())
//         Handlable::rem_ref();

        return ret > 0 ? ret : 1;
    }


    bool
    Context::all_attributes_have_single_ref_count()
    {
//            if( OverMountContext_Delegate.bound() )
//                return OverMountContext_Delegate->all_attributes_have_single_ref_count();
//    return Attribute::all_attributes_have_single_ref_count();
        return false;
    }


//     fh_istream
//     Context::getDigestStream( std::string DigestName , const std::string& rdn, EA_Atom* atom )
//     {
//         fh_stringstream ret;
    
//         EVP_MD_CTX mdctx;
//         const EVP_MD *md;
//         unsigned char md_value[EVP_MAX_MD_SIZE];
//         unsigned int md_len;
//         string s;
//         int i;
//         char* buf = 0;
//         int optimal_block_size = 1024;
//         static gboolean First_Generate = 1;

//         InitOpenSSL();

//         md = EVP_get_digestbyname(DigestName.c_str());
//         if( !md )
//         {
//             ret << "N/A";
//             return ret;
//         }

//         try {
        
//             fh_istream ifs = getIStream();
//             EVP_DigestInit(&mdctx, md);
//             optimal_block_size = EVP_MD_block_size( md );
//             buf = new char[optimal_block_size+1];

//             while( ifs->good() )
//             {
//                 ifs->read(buf, optimal_block_size);
//                 EVP_DigestUpdate(&mdctx, buf, ifs->gcount());
//             }

//             delete [] buf;
//             if(!ifs->eof())
//             {
//                 ret << "Read error";
//                 return ret;
//             }
            
//             EVP_DigestFinal(&mdctx, md_value, &md_len);

//             radixdump( ret, md_value, md_value + md_len, 16 );
//             return ret;
//         }
//         catch( ... )
//         {
//             s="";
//         }

//         ret << s;
//         return ret;
//     }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Get the subcontext count as a string. Used in EA generators.
 * return The sub context count string.
 */
    fh_istream
    Context::SL_getSubContextCountStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << c->Items.size(); //NumberOfSubContexts;
        return ss;
    }


/**
 * Get the attribute count as a string. Used in EA generators.
 * return The attr count string.
 */
    fh_istream
    Context::SL_getAttributeCountStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << c->getAttributeCount();
        return ss;
    }


    
/**
 * Get the dirname as a string. Used in EA generators.
 * return The dirname string.
 */
    fh_istream
    Context::SL_getDirNameStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        LG_CTX_D << "SL_getDirNameStream() p:" << c->getDirPath() << endl;

        fh_stringstream ss;
        ss << c->getDirName();
        return ss;
    }

    fh_iostream
    Context::SL_getDirNameIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        LG_CTX_D << "SL_getDirNameIOStream() p:" << c->getDirPath() << endl;
        
        if( !c->supportsRename() )
        {
            fh_stringstream ss;
            ss << "Context does not support rename c:" << c->getURL() << endl;
            LG_CTX_D << tostr(ss) << endl;
            Throw_RenameFailed(tostr(ss),c);
        }
        
        fh_stringstream ss;
        ss << c->getDirName();
        return ss;
    }

    void
    Context::SL_RenameContext( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
    {
        string new_rdn = getFirstLine(ss);
        LG_CTX_D << "SL_RenameContext() new_rdn:" << new_rdn << endl;
        c->getParent()->rename( c->getDirName(), new_rdn, false, false );
    }

fh_istream
Context::SL_getParentDirNameStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    if( !c->isParentBound() )
        ss << "";
    else
        ss << c->getParent()->getDirName();
    return ss;
}

fh_istream
Context::SL_getParentURLStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    if( !c->isParentBound() )
        ss << "";
    else
        ss << c->getParent()->getURL();
    return ss;
}


    static bool isAnimationFromMimeType( const std::string& s )
    {
        return ( starts_with( s, "video/" ) || s == "application/x-matroska" );
    }
    

    /**
     * Get the path of an icon to display inline in tree/list views which display
     * the context 'c'
     */
    static fh_istream
    SL_getTreeIconStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( toint( getStrAttr( c, "is-dir", "0" ))) // || c->getHasSubContextsGuess() )
            ss << "icons://ferris-mu-dir.png";
        else
        {
            string name = c->getDirName();
            string mime = c->getMimeType();
            
            if( starts_with( mime, "image/" ))
            {
                ss << "icons://ferris-mu-image.png";
            }
            else if( isAnimationFromMimeType( mime ) )
            {
                ss << "icons://ferris-mu-video.png";
            }
            else if( starts_with( mime, "audio/" ))
            {
                ss << "icons://ferris-mu-audio.png";
            }
            else
                ss << "icons://ferris-mu-file.png";
        }

        return ss;
    }

    
/**
 * Get the dirname extension as a string.
 * The extension is defined as any non '.' characters after the last dot
 * in the string.
 * 
 * Used in EA generators.
 * return The extension of the dirname string.
 */
fh_istream
Context::SL_getDirNameExtensionStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << c->getNameExtension();
    return ss;
}

std::string
Context::getNameExtension()
{
    string dn = getDirName();
    string::size_type pos = dn.rfind('.');
    
    if( string::npos != pos )
    {
        return dn.substr( pos+1 );
    }
    return "";
}




/**
 * Get the dirpath as a string. Used in EA generators.
 * return The dirpath string.
 */
    static fh_istream
    SL_getDirPathStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << c->getDirPath();
        return ss;
    }

    /**
     * get the URL for this context
     */
    static fh_istream
    SL_getURLStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << c->getURL();
        return ss;
    }

    /**
     * Get the names of the EA that ferris recommends clients show the
     * user for this context
     */
    fh_istream
    Context::SL_getRecommendedEAStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << c->getRecommendedEA();
        return ss;
    }

fh_istream
Context::SL_getRecommendedEAShortStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    stringlist_t sl = Util::parseCommaSeperatedList( c->getRecommendedEA() );
    fh_stringstream ss;

    ss << "name,";
    if( !sl.empty() )
    {
        for( stringlist_t::const_iterator si = sl.begin(); si != sl.end(); ++si )
        {
            if( *si != "name" )
            {
                ss << *si;
                break;
            }
        }
    }
    
    return ss;
}

    void addEAToSet( set<string>& theSet, fh_stringstream& ss )
    {
        string s;
        while(getline(ss,s,','))
            theSet.insert(s);
    }
    void addEAToSet( set<string>& theSet, const std::string commaSepEA )
    {
        string s;
        fh_stringstream ss;
        ss << commaSepEA;
        while(getline(ss,s,','))
            theSet.insert(s);
    }
    
/**
 * Get the names of the EA that ferris recommends clients show the
 * user for this context... union of EA from base and overmount context
 */
    fh_istream
    Context::SL_getRecommendedEAUnionStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        /* Merge recommended EA for base and overmounting contexts */
        fh_stringstream ss;

        string   s = c->getRecommendedEA();

        if( !isBound(c->OverMountContext_Delegate) )
        {
            /*  There is only the recommended ea, short out */
            ss << s;
            return ss;
        }
        string oms = c->OverMountContext_Delegate->getRecommendedEA();
        
//         cerr << "SL_getRecommendedEAUnionStream()"
//              << " s:" << s << " oms:" << oms
//              << endl;
                
        set<string> unionset;
        addEAToSet( unionset, s );
        addEAToSet( unionset, oms );
        unique_copy( unionset.begin(), unionset.end(),
                     ostream_iterator<string>(ss,","));
        return ss;
    }


    fh_istream
    Context::getRecommendedEAUnionView()
    {
        fh_stringstream ss;

        set<string> unionset;
        recommendedEAUnionViewAdd( unionset, this );

        LG_GTKFERRIS_D << "Context::SL_getRecommendedEAUnionViewStream() c:" << getURL() << endl;
//        cerr << "Context::SL_getRecommendedEAUnionViewStream() c:" << getURL() << endl;
//        BackTrace();

        try
        {
            for( Context::iterator iter = begin(); iter != end(); ++iter )
            {
                LG_GTKFERRIS_D << "Context::SL_getRecommendedEAUnionViewStream() iter:" << (*iter)->getURL() << endl;
                recommendedEAUnionViewAdd( unionset, GetImpl(*iter) );
            }
//          LG_GTKFERRIS_D << "Context::SL_getRecommendedEAUnionViewStream(done with iter)" << endl;
        }
        catch( exception& e )
        {
        }
        
        unique_copy( unionset.begin(), unionset.end(),
                     ostream_iterator<string>(ss,","));
//        cerr << "SL_getRecommendedEAUnionViewStream() ret:" << tostr(ss) << endl;
        LG_GTKFERRIS_D << "Context::SL_getRecommendedEAUnionViewStream(done)" << endl;
        
        return ss;
    }

    void
    Context::recommendedEAUnionViewAdd( set<string>& theSet, Context* c )
    {
        addEAToSet( theSet, c->getRecommendedEA() );
        if(isBound(c->OverMountContext_Delegate))
            addEAToSet( theSet, c->OverMountContext_Delegate->getRecommendedEA() );
    }
    

    /**
     * set_union() of recommended-ea-union of this context and all of its
     * direct children
     */
    fh_istream
    Context::SL_getRecommendedEAUnionViewStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        return c->getRecommendedEAUnionView();
    }
    
/**
 * Get the names of the EA that ferris recommends clients show the
 * user for this context
 */
    fh_istream
    Context::SL_getHasSubContextsGuessStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;

        if( c->getHaveReadDir() )
            ss << c->SubContextCount();
        else
            ss << c->getHasSubContextsGuess();
        
        return ss;
    }

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    fh_display_aggdata
    getCachedContextAggregateData( fh_context c, int m )
    {
        int aggmode = 0;

        if( isBound( c->AggregateData ) )
        {
            aggmode = c->AggregateData->getMode();
        }

        /* If there is not aggregate data or we need recursive and the existing
         * cache is not recursive, remake the data.
         */
        if( !isBound( c->AggregateData )
            || (( m & AGGDATA_RECURSIVE ) && (!( aggmode & AGGDATA_RECURSIVE )))
            )
        {
            c->AggregateData = getAggregateData( c, aggdata_mode_t(m) );
        }
        return c->AggregateData;
    }

    void
    Context::SL_FlushAggregateData( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
    {
        c->AggregateData = 0;
    }

    
    fh_iostream
    Context::SL_getRecursiveSubcontextCountIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_display_aggdata d = getCachedContextAggregateData( c, AGGDATA_RECURSIVE );

        fh_stringstream ss;
        ss << d->getRecursiveData().count;
        return ss;
    }

    fh_istream
    Context::SL_getRecursiveSubcontextCountStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        return SL_getRecursiveSubcontextCountIOStream( c, rdn, atom );
    }

    fh_iostream
    Context::SL_getRecursiveSubcontextMaxDepthIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_display_aggdata d = getCachedContextAggregateData( c, AGGDATA_RECURSIVE );

        fh_stringstream ss;
        ss << d->getRecursiveData().maxdepth;
        return ss;
    }


    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    static fh_iostream
    SL_getIsImage( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << c->isAttributeBound( "rgba-32bpp", false );
        return ss;
    }

    static fh_iostream
    SL_getIsAnimation( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        string mt = getStrAttr( c, "mimetype", "" );

        ss << isAnimationFromMimeType( mt );
//        ss << c->isAttributeBound( "pgmpipe", false );
        return ss;
    }

    static fh_iostream
    SL_getIsAudio( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        string mt = getStrAttr( c, "mimetype", "" );
        ss << starts_with( mt, "audio" ) || starts_with( mt, "video" );
//        ss << c->isAttributeBound( "a52-sample-rate", false );
        return ss;
    }
    
    static fh_iostream
    SL_getIsSource( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        string name = c->getDirName();
        static boost::regex r = toregex( "\\.cpp^|\\.hh^|\\.pl^|\\.c^|\\.h^|\\.py^|\\.java^" );
        
        fh_stringstream ss;
        ss << regex_search( name, r, boost::match_any );
        return ss;
    }
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/


fh_iostream
Context::SL_getAsJSON( Context* c, const std::string& rdn, EA_Atom* atom )
{
    c->ensureUpdateMetaDataCalled();
    fh_stringstream ss;
    ss << contextToJSON( c );
    return ss;
}


fh_iostream
Context::SL_getAsXML( Context* c, const std::string& rdn, EA_Atom* atom )
{
    c->ensureUpdateMetaDataCalled();
    fh_stringstream ss;
    ss << XML::contextToXML( c );
    return ss;
}

static void
SL_getAsXMLClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    string s = StreamToString( ss );
    XML::updateFromXML( c, s, false );
}

/**
 * load a library and using the Create function make a handler for as-text
 * from a shared library.
 *
 * note that we can't close the library while its fh_AsTextStatelessFunctor
 * is in the cache because we are using code from the library still.
 */
static
fh_istream
SL_getAsText_fromLibrary(  Context* c,
                           const std::string& rdn,
                           EA_Atom* atom,
                           const std::string& libname )
{
    typedef map< string, fh_AsTextStatelessFunctor > cache_t;
    static cache_t cache;
    cache_t::iterator ci = cache.find( libname );

    /**
     * cache it if its not there
     */
    if( ci == cache.end() )
    {
//        const string library_path = FERRIS_AS_TEXT_PLUGIN_DIR + "/" + libname;
        const string library_path = makeFerrisPluginPath( "astext", libname );
        
        GModule* gmod = g_module_open ( library_path.c_str(), G_MODULE_BIND_LAZY );
        fh_AsTextStatelessFunctor (*creator_function)();
    
        if( !gmod )
        {
            fh_stringstream ss;
            ss  << "Error, unable to open module file:" << library_path << " "
                << g_module_error ()
                << endl;
            cerr << tostr(ss) << endl;
            Throw_GModuleOpenFailed( tostr(ss), 0 );
        }

        if (!g_module_symbol( gmod, "Create", (gpointer*)&creator_function ))
        {
            ostringstream ss;
            ss  << "Error, unable to resolve Create in module file:" << library_path
                << " this should never happen. Please report it to the mailing list"
                << " "
                << g_module_error()
                << endl;
            cerr << tostr(ss) << endl;
            Throw_GModuleOpenFailed( tostr(ss), 0 );
        }

        fh_AsTextStatelessFunctor f = creator_function();
        ci = cache.insert( make_pair( library_path, f ) ).first;
    }

    return ci->second->getAsText( c, rdn, atom );
}


fh_istream
Context::SL_getAsText( Context* c, const std::string& rdn, EA_Atom* atom )
{
    c->ensureUpdateMetaDataCalled();
    fh_stringstream ss;
    string libname  = "";
    string mimetype = getStrAttr( c, "mimetype", "" );
    
    if( starts_with( mimetype, "text/plain" ))
    {
        return c->getIStream();
    }

    libname = getLibraryNameFromMime( mimetype );
    if( !libname.empty() )
    {
        return SL_getAsText_fromLibrary( c, rdn, atom, libname );
    }
    
    string ferristype = getStrAttr( c, "ferris-type", "" );
    libname = getLibraryNameFromFerrisType( ferristype );
    if( !libname.empty() )
    {
        return SL_getAsText_fromLibrary( c, rdn, atom, libname );
    }

    fh_context cc = c;
    libname = getLibraryNameFromMatcher( cc );
    if( !libname.empty() )
    {
        return SL_getAsText_fromLibrary( c, rdn, atom, libname );
    }

    if( starts_with( mimetype, "text/" ))
    {
        return c->getIStream();
    }
    
    // failed
    {
        fh_stringstream ss;
        ss << "No conversion to plaintext for context:" << c->getURL()
           << " mime:" << mimetype
           << " ferristype:" << ferristype
           << endl;
        Throw_CanNotGetStream( tostr(ss), c );
    }
}



    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

time_t
Context::getDownloadIfMTimeSince()
{
    return s_downloadMTimeSince()[ this ];
}

void
Context::setDownloadIfMTimeSince( time_t x )
{
    s_downloadMTimeSince()[ this ] = x;
}

    fh_istream
    Context::SL_getFerrisCurrentTimeIStream( Context* c,
                                             const std::string& rdn,
                                             EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << Time::getTime();
        return ss;
    }
    
    
fh_istream
Context::SL_getDownloadIfMTimeSinceIStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    c->ensureUpdateMetaDataCalled();
    fh_stringstream ss;
    ss << c->getDownloadIfMTimeSince();
    return ss;
}

fh_iostream
Context::SL_getDownloadIfMTimeSinceIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    c->ensureUpdateMetaDataCalled();
    fh_stringstream ss;
    ss << c->getDownloadIfMTimeSince();
    return ss;
}

void
Context::SL_downloadIfMTimeSinceClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    time_t x = 0;
    ss >> x;
    c->setDownloadIfMTimeSince( x );
//     cerr << "SL_downloadIfMTimeSinceClosed() c:" << c->getURL()
//          << " x:" << x
//          << " x.str:" << Time::toTimeString( x )
//          << endl;
}

    /**
 * Test if the current value for download-if-mtime-since is older than
 * the current mtime. The mtime is taken either from the given argument or
 * defaults to getting the mtime EA for this context.
 *
 * Mostly used in context plugins to test the modification time of remote data.
 * simply call testDownloadIfMTimeSince( mtime, true ) in a network module to
 * test if the mtime from a network header is new enough to download the data itself.
 *
 * @returns true if either there is no mtime EA or the mtime is newer than
 *          download-if-mtime-since or its a remote context and force is false
 *
 * @param mtime Modification time for this context or 0 to get the mtime via
 *        ea.
 * @param force If this is a remote context one must set force to true in
 *        order to perform the test. default is false, ie. test local data only
 */
bool
Context::testDownloadIfMTimeSince( time_t mtime, bool force )
{
//     cerr << "Context::testDownloadIfMTimeSince() "
//          << " force:" << force
//          << " mtime:" << mtime
//          << " isRemote:" << isRemote()
//          << endl;
    
    if( !force && isRemote() )
        return true;
    
    /*
     * handle download-if-mtime-since EA in a global fashion
     */
    if( time_t dlt = getDownloadIfMTimeSince() )
    {
        if( !mtime )
        {
            if( isAttributeBound( "mtime", false ) )
                mtime = toType< time_t >(getStrAttr( this, "mtime", "0" ));
            else
                return true;
        }

        if( mtime <= dlt )
        {
            fh_stringstream ss;
            ss << "Remote document not modified, no source for:" << getURL() << endl;
            Throw_ContentNotModified( tostr(ss), 0 );
        }
    }
    return true;
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


fh_istream
Context::SL_getIsRemote( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << c->isRemote();
    return ss;
}

fh_iostream
Context::SL_getFerrisPostCopyActionStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << "";
    return ss;
}

void
Context::SL_FerrisPostCopyActionStreamClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    string s = StreamToString( ss );
    try
    {
        fh_context sc = Resolve( s );
        c->priv_postCopyAction( sc );
    }
    catch( exception& e )
    {
    }
}

    void
    Context::priv_postCopyAction( fh_context c )
    {
    }
    


fh_iostream
Context::SL_getFerrisPreCopyActionStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << "";
    return ss;
}

void
Context::SL_FerrisPreCopyActionStreamClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    string s = StreamToString( ss );
    try
    {
        fh_context sc = Resolve( s );
        c->priv_preCopyAction( sc );
    }
    catch( exception& e )
    {
    }
}

    void
    Context::priv_preCopyAction( fh_context c )
    {
    }


fh_iostream
Context::SL_getSubtitlesStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << getStrAttr( c, "subtitles-local", "", true );
    return ss;
}


fh_iostream
Context::SL_getDotEAStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    string aname = "." + rdn;
    ss << getStrAttr( c, aname, "", true );
    return ss;
}

    
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

/**
 * Get a string that has a list of all the EA Names that this attribute
 * contains. Each attribute is seperated with a "," character.
 *
 * @see priv_getAttributeNames()
 * @see getAttributeNames()
 * @param attr Not Used.
 * @return string with a list of attribute names seperated with a "," char
 */
    fh_istream
    Context::SL_getEANamesStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        AttributeCollection::AttributeNames_t an;
        c->getAttributeNames( an );
        ss << Util::createCommaSeperatedList( an );
        return ss;
    }

/**
 * Same as SL_getEANamesStream() but the qualified namespaces like schema: are not returned
 */
    static fh_istream
    SL_getEANamesNoQualStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        AttributeCollection::AttributeNames_t an;
        c->getAttributeNames( an );
        AttributeCollection::AttributeNames_t anfinal;

        for( AttributeCollection::AttributeNames_t::iterator iter = an.begin();
             iter != an.end(); iter++ )
        {
            if( starts_with( *iter, "schema:" ))
                continue;
            anfinal.push_back( *iter );
        }
        
        ss << Util::createCommaSeperatedList( anfinal );
        return ss;
    }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

string digest( fh_istream ifs, string DigestName )
{
    fh_stringstream ret;
    
    EVP_MD_CTX* mdctx;
    const EVP_MD *md;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    string s;
    char* buf = 0;
    int optimal_block_size = 1024;
    static gboolean First_Generate = 1;

    InitOpenSSL();


    md = EVP_get_digestbyname(DigestName.c_str());
    if( !md )
    {
        ret << "N/A";
        return tostr(ret);
    }

    try
    {
        mdctx = EVP_MD_CTX_new();
        EVP_DigestInit(mdctx, md);
        optimal_block_size = EVP_MD_block_size( md );
        buf = new char[optimal_block_size+1];

        while( ifs->good() )
        {
            ifs->read(buf, optimal_block_size);
            EVP_DigestUpdate(mdctx, buf, ifs->gcount());
        }

        delete [] buf;
        if(!ifs->eof())
        {
            ret << "Read error";
            return tostr(ret);
        }
            
        EVP_DigestFinal(mdctx, md_value, &md_len);

        radixdump( ret, md_value, md_value + md_len, 16 );

        EVP_MD_CTX_free(mdctx);
        return tostr(ret);
    }
    catch( ... )
    {
        s="";
    }

    EVP_MD_CTX_free(mdctx);
    ret << s;
    return tostr(ret);
}

string digest( string s, string DigestName )
{
    fh_stringstream ss(s);
    return digest( ss, DigestName );
}


    
    static fh_istream
    SL_getDigestStream( string DigestName, Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ret;
        string s = "";
        try
        {
            if( isFalse( getStrAttr( c, "is-special", "0" )))
            {
                fh_istream ifs = c->getIStream();
                s = digest( ifs, DigestName );
            }
        }
        catch( ... )
        {
            s="";
        }

        ret << s;
        return ret;
            
        
//         fh_stringstream ret;
    
//         EVP_MD_CTX mdctx;
//         const EVP_MD *md;
//         unsigned char md_value[EVP_MAX_MD_SIZE];
//         unsigned int md_len;
//         string s;
//         int i;
//         char* buf = 0;
//         int optimal_block_size = 1024;
//         static gboolean First_Generate = 1;

//         InitOpenSSL();

//         md = EVP_get_digestbyname(DigestName.c_str());
//         if( !md )
//         {
//             ret << "N/A";
//             return ret;
//         }

//         try {
        
//             fh_istream ifs = c->getIStream();
//             EVP_DigestInit(&mdctx, md);
//             optimal_block_size = EVP_MD_block_size( md );
//             buf = new char[optimal_block_size+1];

//             while( ifs->good() )
//             {
//                 ifs->read(buf, optimal_block_size);
//                 EVP_DigestUpdate(&mdctx, buf, ifs->gcount());
//             }

//             delete [] buf;
//             if(!ifs->eof())
//             {
//                 ret << "Read error";
//                 return ret;
//             }
            
//             EVP_DigestFinal(&mdctx, md_value, &md_len);

//             radixdump( ret, md_value, md_value + md_len, 16 );
//             return ret;
//         }
//         catch( ... )
//         {
//             s="";
//         }

//         ret << s;
//         return ret;
    }

#ifdef FERRIS_HAVE_LIBZ
    static fh_istream
    SL_getCRC32IStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ret;
        string s = "";
        try
        {
            if( isFalse( getStrAttr( c, "is-special", "0" )))
            {
                fh_istream ifs = c->getIStream();
                const int bufsz = 4096*16+1;
                char buf[ bufsz + 1 ];
                uLong crc = crc32(0L, Z_NULL, 0);

                while( true )
                {
                    ifs.read( buf, bufsz );
                    int gc = ifs.gcount();
                    if( gc )
                        crc = crc32( crc, (Bytef*)buf, gc );
                    if( !ifs )
                        break;
                }

                ret << hex << setw(8) << setfill('0') << crc;
            }
        }
        catch( ... )
        {
            s="";
        }

        return ret;
    }
#endif    


    bool
    Context::getHasSubContextsGuess()
    {
        return true;
    }

    string getBaseEAName( string tailstr, Context* c, const string& attrName )
    {
        string base_ea_name = "";
        if( int newend = attrName.find( tailstr ) )
        {
            base_ea_name = attrName.substr( 0, newend );
//             cerr << "getBaseEAName() tailstr:" << tailstr
//                  << " base_ea_name:" << base_ea_name
//                  << endl;
        }
        else
        {
            fh_stringstream ss;
            ss << "Problem with finding the base EA name for the timet value!"
               << " Can not get the base ea:" << base_ea_name
               << " to create the ea:" << attrName
               << " output."
               << " path:" << c->getDirPath()
               << endl;
            Throw_CanNotGetStream( tostr(ss), c );
        }

        return base_ea_name;
    }

    fh_istream
    Context::SL_getSizeHumanReadableStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        c->ensureUpdateMetaDataCalled();
        off_t s = 0;
        {
            stringstream szss;
            szss << getStrAttr( c, getBaseEAName( "-human-readable", c, rdn ), "0" );
            szss >> s;
        }

        fh_stringstream ss;
        ss << Util::convertByteString( s );
        return ss;
    }

template<
    const std::string& MediaSizeKey,
    const std::string& MediaSizeDefault,
    const std::string& EAPostfix >
fh_istream
SL_getSizeMediaCountStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    double s = 0;
    {
        stringstream szss;
        szss << getStrAttr( c, getBaseEAName( EAPostfix, c, rdn ), "0" );
        szss >> s;
    }

    double MediaSize = toType<double>( getConfigString(
                                           FDB_GENERAL,
                                           MediaSizeKey,
                                           MediaSizeDefault ));
    
    s = s / MediaSize;
    fh_stringstream ss;
    ss << s;
    return ss;
}



    fh_istream
    Context::SL_getContentIStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        LG_CTX_D << "Context::SL_getContentIStream() c:" << c->getURL() << endl;
        
        EA_Atom_ReadWrite_OpenModeCached* a
            = dynamic_cast<EA_Atom_ReadWrite_OpenModeCached*>( atom );
        if( a )
            return c->getIStream( a->getOpenMode() );

        fh_stringstream ss;
        ss << "SL_getContentIStream() this function should not have been called"
            << " with the context/atom pairing that it was. This is a major bug"
            << " caused in only one place in the code. PLEASE REPORT IT TO MAIL LIST"
           << endl;
        Throw_CanNotGetStream( tostr(ss), c );
    }

    static fh_iostream
    SL_getContentIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
//        cerr << "SL_getContentIOStream() c:" << c->getURL() << endl;

        EA_Atom_ReadWrite_OpenModeCached* a
            = dynamic_cast<EA_Atom_ReadWrite_OpenModeCached*>( atom );
        if( a )
            return c->getIOStream( a->getOpenMode() );

        fh_stringstream ss;
        ss << "SL_getContentIOStream() this function should not have been called"
            << " with the context/atom pairing that it was. This is a major bug"
            << " caused in only one place in the code. PLEASE REPORT IT TO MAIL LIST"
           << endl;
        Throw_CanNotGetStream( tostr(ss), c );
    }

    static void
    SL_getContentClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
    {
    }
    
    
//     fh_istream
//     SL_getMTimeCTimeStream( Context* c, const std::string& rdn, EA_Atom* atom )
//     {
//         c->ensureUpdateMetaDataCalled();
//         fh_stringstream ss;
//         string mtime = getStrAttr( c, "mtime", "" );
//         if( mtime.length() )
//         {
//             time_t t = 0;
//             stringstream mtimess;
//             mtimess << mtime;
//             mtimess >> t;
//             ss << ctime( &t );
//         }
//         return ss;
//     }

//     fh_istream
//     SL_getATimeCTimeStream( Context* c, const std::string& rdn, EA_Atom* atom )
//     {
//         c->ensureUpdateMetaDataCalled();
//         fh_stringstream ss;
//         string mtime = getStrAttr( c, "atime", "" );
//         if( mtime.length() )
//         {
//             time_t t = 0;
//             stringstream mtimess;
//             mtimess << mtime;
//             mtimess >> t;
//             ss << ctime( &t );
//         }
//         return ss;
//     }

    fh_istream
    Context::SL_getXTimeCTimeStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        string eaname = getBaseEAName( "-ctime", c, rdn );

        c->ensureUpdateMetaDataCalled();
        fh_stringstream ss;
        string mtime = getStrAttr( c, eaname, "" );
        if( mtime.length() )
        {
            time_t t = 0;
            stringstream mtimess;
            mtimess << mtime;
            mtimess >> t;
            ss << ctime( &t );
        }
        return ss;
    }

    fh_istream
    Context::SL_getXTimeDayGranularityStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        string eaname = getBaseEAName( "-day-granularity", c, rdn );

        c->ensureUpdateMetaDataCalled();
        fh_stringstream ss;
        string mtime = getStrAttr( c, eaname, "" );
        if( mtime.length() )
        {
            time_t t = 0;
            stringstream mtimess;
            mtimess << mtime;
            mtimess >> t;

            struct tm mytime = *(localtime( &t ));
            mytime.tm_sec  = 0;
            mytime.tm_min  = 0;
            mytime.tm_hour = 0;
            
            ss << mktime( &mytime );
        }
        return ss;
    }

    fh_istream
    Context::SL_getXTimeMonthGranularityStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        string eaname = getBaseEAName( "-month-granularity", c, rdn );

        c->ensureUpdateMetaDataCalled();
        fh_stringstream ss;
        
        string mtime = getStrAttr( c, eaname, "" );
        if( mtime.length() )
        {
            time_t t = 0;
            stringstream mtimess;
            mtimess << mtime;
            mtimess >> t;

            struct tm mytime = *(localtime( &t ));
            mytime.tm_sec  = 0;
            mytime.tm_min  = 0;
            mytime.tm_hour = 0;
            mytime.tm_mday = 1;

            ss << mktime( &mytime );
        }
       return ss;
    }

    fh_istream
    Context::SL_getXTimeYearGranularityStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        string eaname = getBaseEAName( "-year-granularity", c, rdn );

        c->ensureUpdateMetaDataCalled();
        fh_stringstream ss;

        string mtime = getStrAttr( c, eaname, "" );
        if( mtime.length() )
        {
            time_t t = 0;
            stringstream mtimess;
            mtimess << mtime;
            mtimess >> t;

            struct tm mytime = *(localtime( &t ));
            mytime.tm_sec  = 0;
            mytime.tm_min  = 0;
            mytime.tm_hour = 0;
            mytime.tm_mday = 1;
            mytime.tm_mon  = 0;

            ss << mktime( &mytime );
        }
        return ss;
    }

    fh_istream
    Context::SL_getSizeFromContentIStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        streamsize size = 0;
        fh_istream iss = c->getIStream();
        char ch;
        while( iss >> noskipws >> ch )
            ++size;
        fh_stringstream ss;
        ss << size;
        LG_XSLTFS_D << "SL_getSizeFromContentIStream() sz:" << size << endl;
        return ss;
    }
    


    fh_stringstream
    Context::getTimeStrFTimeStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        c->ensureUpdateMetaDataCalled();
        fh_stringstream ret;
        
        string eaname   = getBaseEAName( "-display", c, rdn );
        time_t TT       = 0;
        fh_istream eass = c->getAttribute( eaname )->getIStream();
        if( !(eass >> TT) )
        {
            fh_stringstream ss;
            ss << "Can not get the base ea:" << eaname
               << " to create the ea:" << rdn
               << " output."
               << " path:" << c->getDirPath()
               << endl;
            
            Throw_CanNotGetStream( tostr(ss), c );
        }

//         LG_CTX_D << "getTimeStrFTimeStream() url:" << c->getURL()
//                  << " eaname:" << eaname
//                  << endl;

//         if( !TT )
//         {
//             fh_stringstream ss;
//             ss << "Can not get the base ea:" << eaname
//                << " to create the ea:" << rdn
//                << " output."
//                << " path:" << c->getDirPath()
//                << endl;
            
//             Throw_CanNotGetStream( tostr(ss), c );
//         }

//         LG_CTX_D << "getTimeStrFTimeStream(1) url:" << c->getURL()
//                  << " TT:" << TT
//                  << endl;
        ret << Time::toTimeString( TT );
        return ret;
    }

//     fh_istream
//     SL_getTimeStrFTimeIStream( Context* c, const std::string& rdn, EA_Atom* atom )
//     {
//         LG_CTX_D << "SL_getTimeStrFTimeIStream() url:" << c->getURL() << endl;
//         return getTimeStrFTimeStream( c, attr );
//     }

    fh_stringstream
    Context::SL_getTimeStrFTimeIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        LG_CTX_D << "SL_getTimeStrFTimeIOStream() url:" << c->getURL() << endl;
        return getTimeStrFTimeStream( c, rdn, atom );
    }

    void
    Context::SL_setTimeStrFTimeStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
    {
        string s;
        getline( ss, s );

        LG_CTX_D << "SL_setTimeStrFTimeStream() url:" << c->getURL()
                 << " new data:" << s
                 << endl;

        if( !s.length() )
            return;

        /*
         * This is set to either the current time or to a specific value and
         * used after the if/else to set the time.
         */
        time_t tt = -1;
        
        if( string::npos != s.find('%') )
        {
            Time::setDefaultTimeFormat( s );
            return;
        }
        else if( isTrue(s) )
        {
            tt = Time::getTime();

            LG_CTX_D << "SL_setTimeStrFTimeStream() setting time"
                     << " for:" << rdn
                     << " to current time:" << tt
                     << endl;
        }
        else
        {
            LG_CTX_D << "SL_setTimeStrFTimeStream() setting time"
                     << " for:" << rdn
                     << " to:" << s
                     << endl;

            try
            {
                struct tm tm = Time::ParseTimeString( s, Time::getDefaultTimeFormat() );
                Time::FreshenTime( tm );
                tt = mktime( &tm );
            }
            catch( BadlyFormedTimeString& e )
            {
                fh_stringstream ss;
                ss << "Can not parse given time:" << s
                   << " for updating time attribute:" << rdn
                   << " on url:" << c->getURL();
                cerr << tostr(ss);
                Throw_BadlyFormedTimeString( tostr(ss), c );
            }
        }

        if( tt == -1 )
        {
            fh_stringstream ss;
            ss << "Can not parse given time:" << s
               << " for updating time attribute:" << rdn
               << " on url:" << c->getURL();
            LG_CTX_D << tostr(ss) << endl;
            cerr << tostr(ss) << endl;
            Throw_BadlyFormedTimeString( tostr(ss), c );
        }
            
//         fh_stringstream ss;
//        ss << tt << flush;
        string eaname = getBaseEAName( "-display", c, rdn );
//        setStrAttr( c, eaname, tostr(ss) );
        LG_CTX_D << "SL_setTimeStrFTimeStream() setting time"
                 << " for:" << rdn
                 << " eaname:" << eaname
                 << " time_t:" << tt
                 << endl;
            
        {
            fh_attribute a = c->getAttribute( eaname );
            fh_iostream oss = a->getIOStream();
            oss << tt << flush;
        }
        LG_CTX_D << "SL_setTimeStrFTimeStream(DONE) setting time"
                 << " for:" << rdn
                 << " eaname:" << eaname
                 << " time_t:" << tt
                 << endl;

            
        
    }


    static fh_istream
    SL_getUserOwnerNameStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;

        string eaname = getBaseEAName( "-name", c, rdn );
        eaname+="-number";
        uid_t uid = toType<uid_t>( getStrAttr( c, eaname, "-1" ));
        ss << Shell::getUserName( uid );
        return ss;
    }
    
    static fh_istream
    SL_getGroupOwnerNameStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        string eaname = getBaseEAName( "-name", c, rdn );
        eaname+="-number";
        gid_t gid = toType<gid_t>( getStrAttr( c, eaname, "-1" ));
        ss << Shell::getGroupName( gid );
        return ss;
    }

    static fh_istream
    SL_getGroupOwnerNameFuzzStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        string DefaultGroupID = tostr( Shell::getGroupID() );
        string DefaultGroupName = Shell::getGroupName(Shell::getGroupID());
        ss << DefaultGroupName;
        return ss;
    }
    static fh_istream
    SL_getGroupOwnerNumberFuzzStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        string DefaultGroupID = tostr( Shell::getGroupID() );
        string DefaultGroupName = Shell::getGroupName(Shell::getGroupID());
        ss << DefaultGroupID;
        return ss;
    }
    static fh_istream
    SL_getUserOwnerNumberFuzzStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        string DefaultUserID = tostr( Shell::getUserID() );
        string DefaultUserName = Shell::getUserName(Shell::getUserID());
        ss << DefaultUserID;
        return ss;
    }
    static fh_istream
    SL_getUserOwnerNameFuzzStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        string DefaultUserID = tostr( Shell::getUserID() );
        string DefaultUserName = Shell::getUserName(Shell::getUserID());
        ss << DefaultUserName;
        return ss;
    }
    static fh_istream
    SL_getINodeFuzzStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        static int fakeINode = 1;
        ++fakeINode;
        ss << fakeINode;
        return ss;
    }
    static fh_istream
    SL_getMTimeFuzzStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << Time::getTime();
        return ss;
    }
    static fh_istream
    SL_getProtectionLsFuzzStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << "-rw-------";
        return ss;
    }



    static fh_istream
    SL_getCurrentTimeStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << Time::getTime();
        return ss;
    }


fh_iostream
Context::SL_getAsRDF( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    c->getAsRDFXML( ss );
    return ss;
}

fh_stringstream&
Context::getAsRDFXML( fh_stringstream& ss )
{
    // make sure we don't try to read an attribute which
    // tries to read the as-rdf attribute itself, if this
    // ever happens then its an infinate loop.
    static bool inside_getAsRDFXML = false;
    if( inside_getAsRDFXML )
        return ss;
    Util::ValueRestorer< bool > dummy_x1( inside_getAsRDFXML, true );

    RDFCore::fh_model       m = RDFCore::Model::MemoryOnlyModel();
    RDFCore::fh_node  thisURI = RDFCore::Node::CreateURI( getURL() );

    AttributeCollection::AttributeNames_t an;
    getAttributeNames( an );

    for( AttributeCollection::AttributeNames_t::iterator iter = an.begin();
         iter != an.end(); iter++ )
    {
        std::string   rdn = *iter;

        if( rdn == "as-rdf" || rdn == "as-xml" || rdn == "as-json" || rdn == "content"
            || rdn == "as-text" || starts_with( rdn, "exif:thumbnail") )
            continue;
        
        try
         {
             RDFCore::fh_node pred = RDFCore::Node::CreateURI( RDFCore::RDF_FERRIS_BASE + "/" + rdn );
             try
             {
                 std::string val = getStrAttr( this, rdn, "", true, true );
                 m->insert( thisURI, pred, RDFCore::Node::CreateLiteral( val ) );
             }
             catch( exception& e )
             {
                 fh_stringstream ess;
                 ess << "Error reading attribute:" << rdn
                     << " e:" << e.what()
                     << endl;
                 RDFCore::fh_node pred = RDFCore::Node::CreateURI( RDFCore::RDF_FERRIS_BASE + "/error/" + rdn );
                 m->insert( thisURI, pred, RDFCore::Node::CreateLiteral( tostr(ess) ) );
             }
         }
         catch( exception& e )
         {
             continue;
         }
            
    }
    
    m->write( ss );
    return ss;
}



    /**
     * VFS rename method.
     *
     * @returns The new fh_context or throws an exception explaining why the rename failed.
     */
    fh_context
    Context::priv_rename( const std::string& rdn,
                          const std::string& newPath,
                          bool TryToCopyOverFileSystems,
                          bool OverWriteDstIfExists )
    {
        fh_stringstream ss;
        ss << "Rename method not overriden in subclass. This VFS module is defective."
           << " url:" << getURL();
        Throw_RenameFailed( tostr(ss), this );
    }

    /**
     * Public API: This method moves a child context to another location. Note that
     * the new location can be either in the same context or can be at any new location.
     *
     * An attempt is made to perform the move operation using the underlying module,
     * and if that fails and TryToCopyOverFileSystems is true then an attempt to
     * copy the data across filesystems and then delete the original is performed.
     *
     * If OverWriteDstIfExists is true then the data at the destination is replaced with
     * the contents of the context at rdn.
     *
     * So to rename the following /dir1/file1 to /dir1/file1.old
     * this=dir1, rdn=file1, and newPath = file1.old
     *
     * @returns a handle to the context that was renamed, note that the returned
     *          context may be on a completely different filesystem than the original.
     *
     * @param rdn the name of the subcontext to rename
     * @param newPath New path for the context given by rdn to have.
     * @param TryToCopyOverFileSystems If the new path is on a different filesystem
     *        then try to copy this context to that location and then remove the original
     * @param OverWriteDstIfExists Overwrite the contents of a context at newPath if there
     *        is data already at that location.
     *
     */
    fh_context Context::rename( const std::string& rdn,
                                const std::string& newPath,
                                bool TryToCopyOverFileSystems,
                                bool OverWriteDstIfExists )
    {
        LG_CTX_D << "Context::rename(1) rdn:" << rdn
                 << " newPath:" << newPath
                 << " TryToCopyOverFileSystems:" << TryToCopyOverFileSystems
                 << " OverWriteDstIfExists:" << OverWriteDstIfExists
                 << endl;

        LG_CTX_D << "rename(1.0) currentPath:" << getDirPath() << endl;
        
        if( !HaveReadDir )
        {
            read();
        }
        
        if( getOverMountContext() != this )
        {
            return getOverMountContext()->rename( rdn, newPath,
                                                  TryToCopyOverFileSystems,
                                                  OverWriteDstIfExists );
        }

        LG_CTX_D << "rename(3.0)" << endl;
        LG_CTX_D << "rename(3.0) newPath:" << newPath << endl;
        LG_CTX_D << "rename(3.0) currentDir:" << getDirName() << endl;
        LG_CTX_D << "rename(3.0) currentPath:" << getDirPath() << endl;
        LG_CTX_D << "rename(3.0) final" << endl;

        /*
         * Make sure we don't try to overwrite anything we are not told to
         */
        string fullNewPath = appendToPath( getDirPath(), newPath, true );
        if( !OverWriteDstIfExists && Shell::contextExists( fullNewPath ) )
        {
            fh_stringstream ss;
            ss << "Rename failed because destination already exists "
               << " URL:" << getURL()
               << " src:" << rdn << " dst:" << newPath;
            Throw_RenameFailed( tostr(ss), this );
        }

        if( !TryToCopyOverFileSystems && !supportsRename() )
        {
            fh_stringstream ss;
            ss << "Rename not supported by underlying VFS and told not to try to "
               << "copy the data and then remove source data. URL:" << getURL()
               << endl;
            Throw_RenameFailed( tostr(ss), this );
        }

        /*
         * Try for optimized rename
         */
        if( supportsRename() )
        {
            try
            {
                LG_CTX_D << "Context::rename(2) rdn:" << rdn
                         << " newPath:" << newPath
                         << " TryToCopyOverFileSystems:" << TryToCopyOverFileSystems
                         << " OverWriteDstIfExists:" << OverWriteDstIfExists
                         << endl;
//                 cerr << "Context::rename(2) url:" << getURL()
//                      << " rdn:" << rdn
//                          << " newPath:" << newPath
//                          << " TryToCopyOverFileSystems:" << TryToCopyOverFileSystems
//                          << " OverWriteDstIfExists:" << OverWriteDstIfExists
//                          << endl;

                fh_context ret = priv_rename( rdn, newPath, 
                                              TryToCopyOverFileSystems,
                                              OverWriteDstIfExists );
                return ret;
            }
            catch( exception& e )
            {
                LG_CTX_D << "Context::rename() short cut rename failed, e:" << e.what() <<endl;
                
                if( !TryToCopyOverFileSystems )
                {
                    fh_stringstream ss;
                    ss << "Rename failed. url:" << getURL()
                       << " e:" << e.what()
                       << endl;
                    Throw_RenameFailed( tostr(ss), this );
                }
            }
        }

        LG_CTX_D << "resorting to manual rename." << endl;
        
        /*
         * Create a copy at the destination and then delete the original
         */
        fh_context oldc = getSubContext( rdn );
        fh_context newc = oldc->copyTo( fullNewPath, OverWriteDstIfExists );
        
//         fh_context parent_newc = Resolve( newPath, RESOLVE_PARENT );
//         string newrdn = newPath;
//         if( string::npos != newPath.rfind( "/" ) )
//         {
//             newrdn = newPath.substr( newPath.rfind( "/" )+1 );
//         }
//         fh_context newc = Shell::CreateFile( parent_newc, newrdn );
        
//         {
//             fh_iostream oss = newc->getIOStream();
//             fh_istream iss  = oldc->getIStream();
        
//             LG_CTX_D << "---------Start copy-----------" << endl;
//             LG_CTX_D << " oss is good():"<< oss->good() << endl;
//             copy( istreambuf_iterator<char>(iss),
//                   istreambuf_iterator<char>(),
//                   ostreambuf_iterator<char>(oss));
        
// //             if( iss->eof() ) cerr << "Done." << endl;
// //             else             cerr << "Error." << endl;
        
//             LG_CTX_D << " iss is good():"<< iss->good() << endl;
//             LG_CTX_D << " iss is eof():"<< iss->eof() << endl;
//             LG_CTX_D << " oss is good():"<< oss->good() << endl;
//             LG_CTX_D << " iss is state:"<< iss->rdstate() << endl;
//             LG_CTX_D << " oss is state:"<< oss->rdstate() << endl;

//             char xch;
//             iss >> xch;
//             if( !iss->eof() )
//             {
//                 LG_CTX_I << "Failed to copy file, URL: " << getURL()
//                          << " src:" << rdn
//                          << endl;
//                 fh_stringstream ss;
//                 ss << "Failed to copy file, URL: " << getURL()
//                          << " src:" << rdn;
//                 Throw_RenameFailed( tostr(ss), this );
//             }
            
//         }

        try
        {
            remove( oldc );
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "Rename failed. url:" << getURL()
               << " AFTER CREATING COPY OF SRC, can not remove source context src:" << rdn
               << " remove error:" << e.what()
               << endl;
            Throw_RenameFailed( tostr(ss), this );
        }
        
        return newc;
    }

    
    /**
     * Copy this context to the location newPath, optionally replace the new context
     * data if it exists
     */
    fh_context
    Context::copyTo( const std::string& newPath, bool OverWriteDstIfExists )
    {
        fh_context parent_newc = Resolve( newPath, RESOLVE_PARENT );
        string newrdn = newPath;
        if( string::npos != newPath.rfind( "/" ) )
        {
            newrdn = newPath.substr( newPath.rfind( "/" )+1 );
        }

//         cerr << "Context::copyTo() newPath:" << newPath << " newrdn:" << newrdn
//              << " this:" << getURL()
//              << " parent_newc:" << parent_newc->getURL()
//              << endl;

        if( !OverWriteDstIfExists && parent_newc->isSubContextBound( newrdn ) )
        {
            fh_stringstream ss;
            ss << "Copy failed because new context exists and we are not overwriting."
               << " URL:" << getURL()
               << " newPath:" << newPath;
            Throw_ContextExists( tostr(ss), this );
        }
        
        fh_context newc = Shell::CreateFile( parent_newc, newrdn );
        fh_iostream oss = newc->getIOStream( ios::trunc | ios::out );
        Attribute::copyTo( oss );
        
        return newc;
    }
    
    

    /**
     * If this context supports the remove operation then override this method
     * and return true;
     */
    bool Context::supportsRemove()
    {
        return false;
    }

    /**
     * VFS method to actaully remove the subcontext given from disk or persistent
     * storage. Note that this method does not need to call Remove() or any other
     * ferris related methods, it simply makes the subcontext not exist on external
     * storage and returns or throws and exception to prevent the subcontext from
     * being removed.
     */
    void Context::priv_remove( fh_context c )
    {
        fh_stringstream ss;
        ss << "VFS does not support remove() operation. "
           << "Can not delete c:" << c->getURL()
           << endl;
        Throw_CanNotDelete( tostr(ss), GetImpl(c) );
    }

    /**
     * Public API to remove a subcontext from existance both in RAM and
     * on disk.
     */
    void Context::remove( const std::string& rdn )
    {
        if( !priv_isSubContextBound( rdn ) )
        {
            if( !ReadingDir )
                read();
        }
        return remove( getSubContext( rdn ) );
    }

//     void Context::removeSelf()
//     {
//         fh_context c = this;
//         if( isParentBound() )
//             getParent()->remove( c );
//     }
    
    
    /**
     * Public API to remove a subcontext from existance both in RAM and
     * on disk.
     */
    void Context::remove( fh_context c )
    {
        if( getOverMountContext() != this )
        {
            return getOverMountContext()->remove( c );
        }
        try
        {
            if( !supportsRemove() )
            {
                fh_stringstream ss;
                ss << "Can not remove this context, "
                   << " VFS module does not support removal. "
                   << "c:" << c->getURL() << endl;
                Throw_CanNotDelete( tostr(ss), GetImpl(c) );
            }
            
            priv_remove( c );
            Remove( c );
        }
        catch( exception& e )
        {
            throw;
        }
    }
    

    
    /**
     * VFS called method. This method is here to be called from priv_rename methods
     * that for context classes that do not support notification events.
     *
     * Classes that do support notification events should just return Resolve( newPath )
     * in their priv_rename() methods to indicate success.
     *
     * Call this method when performing a rename operation to change the internal
     * state of ferris to record the new name. Also a moved event is emitted.
     * This method should be called after the rename as been performed.
     */
    fh_context
    Context::contextHasBeenRenamed( const std::string& oldrdn, const std::string& newPath )
    {
        fh_context cc = getItem( oldrdn );

        /* add new location */
        string rdn = newPath;
        if( string::npos != newPath.rfind( "/" ) )
        {
            rdn = newPath.substr( newPath.rfind( "/" )+1 );
        }
        fh_context parent_newc = Resolve( newPath, RESOLVE_PARENT );
//         parent_newc->getItems().insert( cc );
//         fh_context ret = cc;
        fh_context ret = parent_newc->priv_readSubContext( rdn, false, true );
        parent_newc->SubContextNamesCacheIsValid = false;
        parent_newc->bumpVersion();
        Emit_Moved( 0, oldrdn, newPath, 0 );
        
        /* delete child */
        eraseItemByName( getItems(), oldrdn );
//        --NumberOfSubContexts;
        SubContextNamesCacheIsValid = false;
        bumpVersion();

        return ret;
    }
    

    static fh_stringstream
    SL_getMimetypeStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << c->getMimeType();
        return ss;
    }

    static fh_stringstream
    SL_getMimetypeFromContentStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( isFalse( getStrAttr( c, "is-special", "0" )))
            ss << c->getMimeType( true );
        return ss;
    }


    static fh_stringstream
    SL_getMimetypeFromFileCommandStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( isFalse( getStrAttr( c, "is-special", "0" )))
        {
            fh_runner r = new Runner();
            r->pushCommandLineArg( "file" );
            r->pushCommandLineArg( "--brief" );
            r->pushCommandLineArg( "--mime-type" );
            r->pushCommandLineArg( c->getDirPath() );
            r->Run();
            fh_istream outss = r->getStdOut();
            ss << chomp( StreamToString(outss) );
        }
        
        return ss;
    }


    static fh_stringstream
    SL_getFiletypeStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << c->getFileType();
        return ss;
    }


    std::string
    Context::priv_getMimeType( bool fromContent )
    {
//         cerr << "Context::priv_getMimeType fc:" << fromContent
//              << " url:" << getURL()
//              << endl;

#ifdef HAVE_KDE
        return KDE::getMimeType( this, fromContent );
#endif

#ifdef HAVE_GNOMEVFS

        static bool gnome_vfs_init_called = false;
        if( !gnome_vfs_init_called )
        {
            gnome_vfs_init_called = true;
            gnome_vfs_init();
        }
        
        if( !fromContent )
        {
            GnomeVFSFileInfo ginfo;
            GnomeVFSResult gres = gnome_vfs_get_file_info(
                getDirPath().c_str(),
                &ginfo,
                GnomeVFSFileInfoOptions(
                    GNOME_VFS_FILE_INFO_GET_MIME_TYPE        |
                    GNOME_VFS_FILE_INFO_FOLLOW_LINKS         |
                    GNOME_VFS_FILE_INFO_FORCE_FAST_MIME_TYPE )
                );
            if( gres == GNOME_VFS_OK )
            {
                string typestring = ginfo.mime_type;
                return typestring;
            }
            LG_CTX_D << "Context::priv_getMimeType fc:" << fromContent
                     << " got error result:" << gres
                     << endl;

            /*
             * See if we can fallback to getting the data from the context itself.
             */
            if( gres == GNOME_VFS_ERROR_NOT_A_DIRECTORY )
            {
                fromContent = true;
            }
            else
            {
                return "unknown";
            }
        }

        try
        {
            
            if( fromContent )
            {
                const int sz = 4096;
                char data[sz+1];
        
                fh_istream iss = getIStream();
                iss.read( data, sz );

                string typestring = gnome_vfs_get_mime_type_for_data( &data, iss.gcount() );
                LG_CTX_D << "Context::priv_getMimeType fc:" << fromContent
                         << " gcount:" << iss.gcount()
                         << " typestring:" << typestring
                         << endl;
            
                return typestring;
            }
        }
        catch( exception& e )
        {
            LG_CTX_W << "got error while looking for mimetype for:" << getURL()
                     << " e:" << e.what() << endl;
        }
        return "unknown";
#endif
        
#ifdef HAVE_LIBMAGIC
        
        {
            static int open_flags = MAGIC_SYMLINK | MAGIC_MIME | MAGIC_PRESERVE_ATIME;
            static magic_t cookie = 0;
            static bool libMagicNotUsable = false;

            if( !libMagicNotUsable )
            {
                if( !cookie )
                {
//                    cerr << "setting up cookie" << endl;
                    cookie = magic_open( open_flags );
                    if( !cookie )
                    {
                        libMagicNotUsable = true;
                        string emsg = errnum_to_string("Error creating libmagic object", errno );
                        cerr << emsg << endl;
                    }
                
//                    cerr << "setting up cookie db" << endl;
                    int rc = magic_load( cookie, 0 );
                    if( rc == -1 )
                    {
                        libMagicNotUsable = true;
                        cerr << "Error loading libmagic database! " << magic_error(cookie) << endl;
                    }
                }

                
                const char* ret = 0;
                if( m_isNativeContext )
                {
                    string path = getDirPath();
//                    cerr << "is native... path:" << getDirPath().c_str() << endl;
                    ret = magic_file( cookie, path.c_str() );
                }
                else
                {
                    const int buffer_sz = 4096;
                    char buffer[ buffer_sz + 1 ];
                    int buffer_valid_length = buffer_sz;

                    fh_istream iss = getIStream();
                    iss.read( buffer, buffer_sz );
                    buffer_valid_length = iss.gcount();
                
                    ret = magic_buffer( cookie, buffer, buffer_valid_length );
                }
                if( ret )
                {
                    string s = ret;
                    s = s.substr( 0, s.find(';') );
                    return s;
                }
            }
            return "unknown";
        }
#endif
        
        return "";
    }
    
    
    string
    Context::getMimeType( bool fromContent )
    {
        string path = getDirPath();

        if( string::npos != path.find( "/.ego/desktop" ))
        {
            if( isParentBound() && getParent()->getDirName() == "desktop" )
            {
//                if( getHasSubContextsGuess() || begin() != end() )
                if( toint( getStrAttr( this, "is-dir", "0" )))
                {
                    return "desktop/directory";
                }
            }
        }

        if( string::npos != path.find( "/.ferris/apps.db" ))
        {
            return "desktop/application";
        }

//         cerr << "Context::getMimeType fc:" << fromContent
//              << " url:" << getURL()
//              << endl;
        
        string mt = priv_getMimeType( fromContent );
        return mt;
    }

    
    
    string
    Context::getFileType()
    {
        return "";
    }


/************************************************************/
/************************************************************/
/************************************************************/

bool
Context::isActiveView()
{
    LG_CTX_D << "Context::isActiveView() earl:" << getURL()
             << " fp:" << getForcePassiveView()
             << " sm:" << supportsMonitoring()
             << " omc:" << OverMountContext_Delegate
             << " cc:" << CoveredContext
             << endl;
    if( getForcePassiveView() )
        return false;
    
    return supportsMonitoring();
}

bool
Context::getForcePassiveView()
{
    if( m_forcePassiveViewCacheIsValid )
        return m_forcePassiveViewCache;
    
    bool ret = false;

    static Util::SingleShot v;
    static boost::regex r;
    static bool haveRegex = true;
    if( v() )
    {
        stringlist_t sl;
        string d = getConfigString( FDB_GENERAL,
                                    CFG_FORCE_PASSIVE_VIEW_K,
                                    CFG_FORCE_PASSIVE_VIEW_DEFAULT );
        if( d.empty() )
        {
            haveRegex = false;
        }
        else
        {
            sl = Util::parseNullSeperatedList( d );
            r = toregexi( sl );
        }
    }

    if( haveRegex )
    {
        string url = getURL();

        if(boost::regex_search( url, r, boost::match_any))
        {
            ret = true;
        }
    }
    
    m_forcePassiveViewCache = ret;
    m_forcePassiveViewCacheIsValid = true;
    
    return ret;
}

fh_stringstream
Context::SL_getIsActiveView( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << c->isActiveView();
    return ss;
}

fh_stringstream
Context::SL_getForcePassiveView( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << c->getForcePassiveView();
    return ss;
}

fh_stringstream
Context::SL_getIsDir( Context* c, const std::string& rdn, EA_Atom* atom )
{
//    cerr << "Context::SL_getIsDir()" << endl;
    fh_stringstream ss;
    ss << c->isDir();
    return ss;
}

fh_stringstream
Context::SL_getIsDirTryAutoMounting( Context* c, const std::string& rdn, EA_Atom* atom )
{
    LG_OVERMOUNT_D << "SL_getIsDirTryAutoMounting(0) c.n:" << c->getDirName() << endl;
    
    bool v = c->isDir();
    LG_OVERMOUNT_D << "SL_getIsDirTryAutoMounting(1.a) c.n:" << c->getDirName() << " v:" << v << endl;
    LG_OVERMOUNT_D << "SL_getIsDirTryAutoMounting(1.b) cc:" << isBound( c->CoveredContext ) << endl;
    if( !v && isBound( c->CoveredContext ) )
    {
        LG_OVERMOUNT_I << "Context::SL_getIsDirTryAutoMounting() have cc. must be an overmount already..." << endl;
        v = true;
    }
        
    if( !v && !isBound( c->OverMountContext_Delegate ) )
    {
        LG_OVERMOUNT_I << "Context::SL_getIsDirTryAutoMounting() overmounting. c:" << c->getURL() << endl;

        c->tryToOverMount( true );
        v = isBound( c->OverMountContext_Delegate );
    }
    
    LG_OVERMOUNT_D << "SL_getIsDirTryAutoMounting(2) c.n:" << c->getDirName() << " v:" << v << endl;
    fh_stringstream ss;
    ss << v;
    return ss;
}
    

bool
Context::isDir()
{
//     cerr << "Context::isDir() url:" << getURL()
//          << " omc:" << toVoid(OverMountContext_Delegate)
//          << " ret:" << (begin() != end())
//          << endl;
    
    if( isBound(OverMountContext_Delegate) )
    {
        return OverMountContext_Delegate->isDir();
    }

//     if( !runningSetUID() )
//     {
//         if( getMimeType() == MIMETYPE_DIRECTORY )
//             return true;
//     }

    try
    {
        return begin() != end();
    }
    catch( exception& e )
    {
        return false;
    }
}



    fh_stringstream
    Context::SL_getOnlyFerrisMetadataCTimeStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        time_t tt = 0;
        RDFCore::fh_node n = Semantic::tryToGetUUIDNode( c );
        if( n )
        {
            time_t rdftt = Semantic::getUUIDNodeModificationTime( n );
            
            LG_CTX_D << "SL_getFerrisCTimeStream() c:" << c->getURL()
                     << " tt:" << tt
                     << " rdftt:" << rdftt
                     << endl;
            tt = max( tt, rdftt );
        }
        fh_stringstream ss;
        ss << tt;
        return ss;
    }


    fh_stringstream
    Context::SL_getFerrisCTimeStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        LG_CTX_D << "SL_getFerrisCTimeStream(top) c:" << c->getURL() << endl;

        time_t tt = getTimeAttr( c, "ctime", 0 );

        LG_CTX_D << "SL_getFerrisCTimeStream() c:" << c->getURL()
                 << " tt:" << tt
                 << endl;
        
        RDFCore::fh_node n = Semantic::tryToGetUUIDNode( c );
        if( n )
        {
            time_t rdftt = Semantic::getUUIDNodeModificationTime( n );
            
            LG_CTX_D << "SL_getFerrisCTimeStream() c:" << c->getURL()
                     << " tt:" << tt
                     << " rdftt:" << rdftt
                     << endl;
            tt = max( tt, rdftt );
        }
        
        fh_stringstream ss;
        ss << tt;
        return ss;
    }

    fh_stringstream
    Context::SL_getFerrisShouldReindexIfNewerStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        time_t tt = getTimeAttr( c, "mtime", 0 );
        LG_CTX_D << "SL_getFerrisCTimeStream(mtime) tt:" << tt << " c:" << c->getURL() << endl;

        static const gchar* LIBFERRIS_ASSUME_NATIVE_FS_CTIME_IS_VOLATILE = g_getenv ("LIBFERRIS_ASSUME_NATIVE_FS_CTIME_IS_VOLATILE");
        if( LIBFERRIS_ASSUME_NATIVE_FS_CTIME_IS_VOLATILE )
            tt = max( tt, getTimeAttr( c, "only-ferris-metadata-ctime", 0 ) );
        else
            tt = max( tt, getTimeAttr( c, "ferris-ctime", 0 ) );
        
        LG_CTX_D << "SL_getFerrisCTimeStream(ret) tt:" << tt << " c:" << c->getURL() << endl;
        fh_stringstream ss;
        ss << tt;
        return ss;
    }
    
    

    
    fh_istream
    Context::SL_getFollowLinkAsFerrisFSSizeStream(
        Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        string realpath = getStrAttr( c, "link-target", "" );

        LG_CTX_D << "realpath:" << realpath << endl;

        
        if( realpath.empty() )
            ss << getStrAttr( c, "size", "0" );
        else
        {
            fh_context r = Resolve( realpath );
            ss << getStrAttr( r, "follow-link-as-ferris-fs-size", "0" );
        }
        return ss;
    }
    

fh_stringstream
Context::SL_hasEmblem( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_etagere    et  = Factory::getEtagere();

    string emblemName;
    fh_emblem em = 0;
    if( starts_with( rdn, EANAME_SL_EMBLEM_ID_PREKEY ) )
    {
        emblemName = rdn.substr( EANAME_SL_EMBLEM_ID_PREKEY.length() );
        em  = et->getEmblemByID( toint(emblemName) );
    }
    else
    {
        emblemName = rdn.substr( EANAME_SL_EMBLEM_PREKEY.length() );
        em  = et->getEmblemByName( emblemName );
    }
    
    fh_medallion med  = c->getMedallion();

    LG_CTX_D << "SL_hasEmblem() c:" << c->getURL()
             << " em:" << em->getName()
             << " ret:" << med->hasEmblem( em )
             << endl;
    
    fh_stringstream ss;
    ss << med->hasEmblem( em );
    return ss;
}

    void
    Context::SL_hasEmblemStreamClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
    {
        string s = StreamToString( ss );
        bool v = isTrue(s);
        fh_etagere    et  = Factory::getEtagere();

        string emblemName;
        fh_emblem em = 0;
        if( starts_with( rdn, EANAME_SL_EMBLEM_ID_PREKEY ) )
        {
            emblemName = rdn.substr( EANAME_SL_EMBLEM_ID_PREKEY.length() );
            em  = et->getEmblemByID( toint(emblemName) );
        }
        else
        {
            emblemName = rdn.substr( EANAME_SL_EMBLEM_PREKEY.length() );
            em  = et->getEmblemByName( emblemName );
        }
    
        fh_medallion med  = c->getMedallion();

        LG_CTX_D << "SL_hasEmblemClosed() c:" << c->getURL()
                 << " em:" << em->getName()
                 << " v:" << v
                 << endl;
        med->ensureEmblem( em, v );
    }


    
fh_stringstream
Context::SL_hasEmblemFuzzy( Context* c, const std::string& rdn, EA_Atom* atom )
{
    string emblemName;
    fh_etagere    et  = Factory::getEtagere();
    fh_emblem em = 0;
    if( starts_with( rdn, EANAME_SL_EMBLEM_ID_FUZZY_PREKEY ) )
    {
        emblemName = rdn.substr( EANAME_SL_EMBLEM_ID_FUZZY_PREKEY.length() );
        em  = et->getEmblemByID( toint(emblemName) );
    }
    else
    {
        emblemName = rdn.substr( EANAME_SL_EMBLEM_FUZZY_PREKEY.length() );
        em  = et->getEmblemByName( emblemName );
    }
    fh_medallion med  = c->getMedallion();

    LG_CTX_D << "SL_hasEmblemFuzzy() c:" << c->getURL()
             << " em:" << em->getName()
             << " ret:" << med->hasEmblem( em )
             << endl;
    
    fh_stringstream ss;
    ss << med->getFuzzyBelief( em );
    return ss;
}

fh_stringstream
Context::SL_getEmblemTime( Context* c, const std::string& rdn, EA_Atom* atom )
{
    static int rdn_soffset = EANAME_SL_EMBLEM_TIME_PREKEY.length();
    int rdn_eoffset = rdn.rfind('-');
    if( rdn_eoffset != string::npos )
        rdn_eoffset -= rdn_soffset;
        
    fh_stringstream ss;
    string emblemName = rdn.substr( rdn_soffset, rdn_eoffset );
    fh_etagere    et  = Factory::getEtagere();
    fh_emblem     em  = et->getEmblemByName( emblemName );
    fh_medallion med  = c->getMedallion();

//    cerr << "SL_getEmblemTime c:" << c->getURL() << " mtime:" << med->getEmblemTimes( em )->getMTime() << endl;
    
    if( med->hasBelief( em ) )
    {
        fh_medallionBelief bel = med->getBelief( em );
        fh_times t = bel->getTimes();
        if( t ) ss << t->getMTime();
        else    ss << 0;
    }
    else
    {
        ss << 0;
    }
    
    
    return ss;
    
}

fh_stringstream
Context::SL_getEmblemList( int cutoff, Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_etagere    et  = Factory::getEtagere();
    fh_medallion med  = c->getMedallion();
    emblems_t     el  = med->getMostSpecificEmblems( Emblem::limitedViewPri_t( cutoff ) );
    stringlist_t  stringli;
    
    for( emblems_t::iterator ei = el.begin(); ei != el.end(); ++ei )
        stringli.push_back( (*ei)->getUniqueName() );
    stringli.sort();
    
    fh_stringstream ss;
    ss << Util::createCommaSeperatedList( stringli );
    return ss;
}

fh_stringstream
Context::SL_getEmblemListAll( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getEmblemList( Emblem::LIMITEDVIEW_PRI_LOW, c, rdn, atom );
}

fh_stringstream
Context::SL_getEmblemListDefault( Context* c, const std::string& rdn, EA_Atom* atom )
{
    return SL_getEmblemList( Emblem::LIMITEDVIEW_PRI_USER_CONFIG, c, rdn, atom );
}



fh_stringstream
Context::SL_getEmblemUpset( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_etagere    et  = Factory::getEtagere();
    fh_medallion med  = c->getMedallion();
    emblems_t     el  = med->getAllEmblems();
    stringlist_t  stringli;
    
    for( emblems_t::iterator ei = el.begin(); ei != el.end(); ++ei )
        stringli.push_back( (*ei)->getUniqueName() );
    stringli.sort();
    
    fh_stringstream ss;
    ss << Util::createCommaSeperatedList( stringli );
    return ss;
}

fh_stringstream
Context::SL_getHasMedallion( Context* c, const std::string& rdn, EA_Atom* atom )
{
    uid_t uid = Shell::getUserID();
    string eaname = string("medallion.") + tostr(uid);
    fh_stringstream ret;
    ret << c->isAttributeBound( eaname, true );
//    ret << 0;
    return ret;
}
    


fh_stringstream
Context::SL_getLanguageHuman( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
#ifdef HAVE_LIBTEXTCAT
    
    const int buffer_sz = 4096;
    char buffer[ buffer_sz + 1 ];
//    fh_istream iss = c->getIStream();
    try
    {
        fh_attribute a = c->getAttribute("as-text");
        fh_istream iss = a->getIStream( ios::in );
        if( iss.read( buffer, buffer_sz ) )
        {
//            cerr << "Classify count:" << iss->gcount() << endl;
//            cerr.write( buffer, iss->gcount() );
            void* h = textcat_Init( "/etc/libtextcat.conf" );
            ss << textcat_Classify( h, buffer, iss->gcount() );
            textcat_Done(h);
        }
        else
            ss << "unknown";
    }
    catch( exception& e )
    {
        ss << "unknown";
    }
        

#endif
    return ss;
}

fh_stringstream
Context::SL_getNothingStream( Context* c,const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    return ss;
}

void
Context::SL_setNothingStream( Context* c,const std::string& rdn, EA_Atom* atom, fh_istream iss )
{
}

fh_stringstream
Context::SL_getStreamWithNumberOneStream( Context* c,const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << 1;
    return ss;
}
    
   
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

static string toBranchPath( Context* c )
{
    fh_stringstream ss;

    PrefixTrimmer trimmer;
    trimmer.push_back( "/" );
    FerrisURL u = FerrisURL::fromString( c->getURL() );
    ss << u.getScheme() << "/" << trimmer( u.getPath() );
    return tostr(ss);
}

#ifdef FERRIS_HAVE_GPGME

static fh_istream
SL_hasValidSignature( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    try
    {
        string filename = c->getDirName();
        if( ends_with( filename, ".gpg" )
            ||
            ( c->isParentBound() && c->getParent()->priv_isSubContextBound( filename+".sig" ) )
            )
        {
            ss << hasValidSignature( getGPGMEContextSingleton(), c );
        }
        else
        {
            ss << 0;
        }
    }
    catch( exception& e )
    {
//         cerr << "SL_hasValidSignature() e:" << e.what() << endl;
//         throw;
        ss << 0;
    }
    return ss;
}


static fh_istream
SL_getSignaturesURL( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << "branchfs-gpg-signatures://" << toBranchPath( c ) << endl;
    return ss;
}

#endif




static fh_istream
SL_getAttributesBranch( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << "branchfs-attributes://" << toBranchPath( c ) << endl;
    return ss;
}

static fh_istream
SL_getParentsBranch( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << "branchfs-parents://" << toBranchPath( c ) << endl;
    return ss;
}

static fh_istream
SL_getMedallionsBranch( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << "branchfs-medallions://" << toBranchPath( c ) << endl;
    return ss;
}

    static fh_istream
    SL_getRemembranceBranch( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << "branchfs-remembrance://" << toBranchPath( c ) << endl;
        return ss;
    }

                                          


static fh_istream
SL_getAssociatedBranches( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << "branchfs-attributes,branchfs-medallions,branchfs-parents,branchfs-remembrance";

#ifdef FERRIS_HAVE_XFSPROGS
    ss << ",branchfs-extents";
#endif
#ifdef FERRIS_HAVE_GPGME
    ss << ",branchfs-signatures";
#endif    

    return ss;
}

static fh_istream
SL_getAssociatedBranchesURL( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << "branches://" << toBranchPath( c ) << endl;
    return ss;
}

#ifdef FERRIS_HAVE_XFSPROGS
static fh_istream
SL_getExtentsBranch( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << "branchfs-extents://" << toBranchPath( c ) << endl;
    return ss;
}
#endif

    static fh_emblem getGeospatialEmblem( fh_medallion med )
    {
        fh_etagere et = Factory::getEtagere();
        fh_emblem geospatialem = et->getEmblemByName( "libferris-geospatial" );

        emblems_t el = med->getMostSpecificEmblems( Emblem::LIMITEDVIEW_PRI_LOW );
        for( emblems_t::const_iterator ei = el.begin(); ei!=el.end(); ++ei )
        {
            fh_emblem em = *ei;
            emblems_t pl = em->getUpset();
            if( find( pl.begin(), pl.end(), geospatialem ) != pl.end() )
            {
                return em;
            }
        }
        return 0;
    }

    static pair< double, double >
    getLatLong( fh_emblem em )
    {
        return make_pair( em->getDigitalLatitude(), em->getDigitalLongitude() );
//         stringstream ss;
//         ss << em->getDescription();
//         double lat = 0, lon = 0;
//         ss >> lat;
//         ss >> lon;
//         return make_pair( lat, lon );
    }
    
    static fh_iostream
    SL_getLatitudeStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        double v = 0.0;

        fh_stringstream ss;
        fh_emblem em = getGeospatialEmblem( c->getMedallion() );
        if( em )
        {
            v = getLatLong( em ).first;
            ss.precision(12);
            ss << v;
        }
        else
        {
            if( c->isAttributeBound( rdn + "-kea" ) )
                ss << getStrAttr( c, rdn + "-kea", "0" );
            else
            {
                string xrdn = (string)"exif:gps-" + rdn;
                ss << getStrAttr( c, xrdn, "0" );
            }
        }
        return ss;
    }


    static void
    SL_LatitudeStreamClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
    {
        double v = 0.0;
        ss >> v;
        setStrAttr( c, rdn + "-kea", toString(v), 1 );
    }
    

    
    static fh_iostream
    SL_getLongitudeStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        double v = 0.0;

        fh_emblem em = getGeospatialEmblem( c->getMedallion() );
        if( em )
        {
            v = getLatLong( em ).second;
        }
        else
        {
            if( c->isAttributeBound( rdn + "-kea" ) )
                v = toType<double>( getStrAttr( c, rdn + "-kea", "0" ) );
            else
            {
                string xrdn = (string)"exif:gps-" + rdn;
                v = toType<double>( getStrAttr( c, xrdn, "0" ) );
            }
        }
        fh_stringstream ss;
        ss << v;
        return ss;
    }

    static void
    SL_LongitudeStreamClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
    {
        double v = 0.0;
        ss >> v;
        setStrAttr( c, rdn + "-kea", toString(v), 1 );
    }
    
    
    static fh_istream
    SL_getGeoSpatialRangeStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        double v = 0.0;

        fh_emblem em = getGeospatialEmblem( c->getMedallion() );
        if( em )
        {
            v = em->getZoomRange();
        }
        fh_stringstream ss;
        ss << v;
        return ss;
    }


    static fh_istream
    SL_getGeoSpatialEmblemNameStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        string v = "";

        fh_emblem em = getGeospatialEmblem( c->getMedallion() );
        if( em )
        {
            v = em->getName();
        }
        fh_stringstream ss;
        ss << v;
        return ss;
    }

    static fh_istream
    SL_getGeoSpatialEmblemIDStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        int v = 0;

        fh_emblem em = getGeospatialEmblem( c->getMedallion() );
        if( em )
        {
            v = em->getID();
        }
        fh_stringstream ss;
        if( v )
            ss << v;
        return ss;
    }
    
    

    static fh_istream
    SL_getGoogleMapsLocationStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        fh_emblem em = getGeospatialEmblem( c->getMedallion() );
        if( em )
        {
//            ss << em->getDescription();
            ss << em->getDigitalLatitude() << "," << em->getDigitalLongitude();
            string d = em->getDescription();
            if( !d.empty() )
                ss << " (" << d << ")";
        }
        return ss;
    }

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

    void
    Context::supplementFerrisLinkTargetFromAbsolute()
    {
        string attr = "link-target";

        if( isAttributeBound( "link-target-relative", false )
            && !isAttributeBound( attr, false ) )
        {
            addAttribute( attr, this,
                          &Context::getFerrisLinkTargetAbsoluteStream,
                          FXD_URL, false );
        }

        attr = "is-broken-link";
        if( isAttributeBound( "link-target-relative", false )
            && !isAttributeBound( attr, false ) )
        {
            LG_NATIVE_D << "Adding is-broken-link" << endl;
            addAttribute( attr, this,
                          &Context::getFerrisIsBrokenLinkStream,
                          XSD_BASIC_BOOL, false );
        }
        
    }
    
    fh_istream
    Context::getFerrisIsBrokenLinkStream( Context*, const std::string&, EA_Atom* attr )
    {
        fh_stringstream ss;
        bool v = isTrue( getStrAttr( this, "dontfollow-is-link", "0" ));
        if( v )
        {
            string p = getStrAttr( this, "link-target", "" );
            v = p.empty();
        }
        ss << v;
        return ss;
    }
    
    fh_istream
    Context::getFerrisLinkTargetAbsoluteStream( Context*, const std::string&, EA_Atom* attr )
    {
        fh_stringstream ss;

        string earl = getURL();
        string lt = getStrAttr( this, "link-target-relative", "" );
        if( lt.empty() )
        {
            return ss;
        }
        LG_CTX_D << "SL_getFerrisLinkTargetAbsoluteStream() earl:" << earl << endl;
        LG_CTX_D << "SL_getFerrisLinkTargetAbsoluteStream() lt:" << lt << endl;
        

        if( isParentBound() )
        {
            fh_context p = getParent();
            fh_context target = 0;
            
            if( !lt.empty() && lt[0] == '/' )
                target = Resolve( lt );
            else
                target = p->getRelativeContext( lt );
            ss << target->getURL();

            LG_CTX_D << "SL_getFerrisLinkTargetAbsoluteStream() p:" << p->getURL() << endl;
            LG_CTX_D << "SL_getFerrisLinkTargetAbsoluteStream() target:" << target->getURL() << endl;
        }

        return ss;
    }
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

    static string RDF_NODE_FUNC_PFX()
    {
        static string pfx = Semantic::getAttrPrefix();
//        static string pfx = "";
        return pfx;
    }
    
#define RDF_NODE_FUNC( FNAME, URI ) \
    static RDFCore::fh_node FNAME() \
    { \
        static RDFCore::fh_node ret = 0; \
        if( !ret ) \
        { \
            ret = RDFCore::Node::CreateURI( RDF_NODE_FUNC_PFX() + URI ); \
        } \
        return ret; \
    } \


template< class ParentClass >
class Remembrance_Common
    :
    public ParentClass
{
public:
    typedef ParentClass _Base;

    static fh_node getHistoryNode( const fh_context& c )
    {
        fh_model m = RDFCore::getDefaultFerrisModel();
        fh_node hn = ParentClass::getHistoryNode();

        LG_RDF_D << "getHistoryNode() c:" << c->getURL() << endl;
        
        fh_node ret = 0;
        if( fh_node uuidnode = Semantic::tryToGetUUIDNode( c ) )
        {
            LG_RDF_D << "getHistoryNode() c:" << c->getURL()
                     << " uuidnode:" << uuidnode->toString()
                     << " hn:" << hn->toString()
                     << endl;
            ret = m->getObject( uuidnode, hn );
            if( ret )
            {
                LG_RDF_D << "getHistoryNode(have ret) c:" << c->getURL() << endl;
                LG_RDF_D << " uuidnode:" << uuidnode->toString()
                         << " ret:" << ret->toString()
                         << endl;
            }
            else
            {
                LG_RDF_D << "getHistoryNode(!ret) c:" << c->getURL()
                         << " uuidnode:" << uuidnode->toString()
                         << endl;
            }
        }
        
//         fh_node earlnode = Node::CreateURI( c->getURL() );
//         fh_node ret = 0;
//         if( fh_node uuidnode = m->getObject( earlnode, ::Ferris::Semantic::uuidPredNode() ) )
//         {
//             ret = m->getObject( uuidnode, hn );
//         }
        return ret;
    }
    
    static fh_node ensureHistoryNode( const fh_context& c )
    {
        fh_model m  = RDFCore::getDefaultFerrisModel();
        fh_node hn  = ParentClass::getHistoryNode();
        fh_node hsn = m->CreateBlankNode();
        fh_node n = getHistoryNode( c );
        if( !n )
        {
            fh_node uuidnode = Semantic::ensureUUIDNode( c );
//            cerr << "ensureHistoryNode c:" << c->getURL() << " uuidnode:" << toVoid(uuidnode) << endl;
            m->insert( uuidnode, hn, hsn );
            n = m->getObject( uuidnode, hn );
        }
        return n;
    }
};

struct Remembrance_FileViewHistoryBase
{
    RDF_NODE_FUNC( getHistoryNode,               "ferris-file-view-history" );
    RDF_NODE_FUNC( getHistorySubjectNode,        "file-view-history" );
    RDF_NODE_FUNC( getCommandNode,               "view-command" );
    RDF_NODE_FUNC( getTimeNode,                  "view-time" );
    RDF_NODE_FUNC( getMostRecentCommandNode,     "most-recent-view-command" );
    RDF_NODE_FUNC( getMostRecentTimeNode,        "most-recent-view-time" );
    RDF_NODE_FUNC( getMostRecentSubContextsNode, "most-recent-view-sub-contexts" );
};
typedef Remembrance_Common< Remembrance_FileViewHistoryBase > Remembrance_FileViewHistory;


struct Remembrance_FileEditHistoryBase
{
    RDF_NODE_FUNC( getHistoryNode,               "ferris-file-edit-history" );
    RDF_NODE_FUNC( getHistorySubjectNode,        "file-edit-history" );
    RDF_NODE_FUNC( getCommandNode,               "edit-command" );
    RDF_NODE_FUNC( getTimeNode,                  "edit-time" );
    RDF_NODE_FUNC( getMostRecentCommandNode,     "most-recent-edit-command" );
    RDF_NODE_FUNC( getMostRecentTimeNode,        "most-recent-edit-time" );
    RDF_NODE_FUNC( getMostRecentSubContextsNode, "most-recent-edit-sub-contexts" );
};
typedef Remembrance_Common< Remembrance_FileEditHistoryBase > Remembrance_FileEditHistory;

    
    template <class Remembrance>
    fh_iostream
    Context::SL_getIsUnRemembered( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        fh_model m = RDFCore::getDefaultFerrisModel();
        fh_node n = Remembrance::getHistoryNode( c );
        bool ret = !n;
        ss << ret;
        return ss;
    }

    template <class Remembrance>
    void
    Context::SL_setIsUnRemembered( Context* c, const std::string& rdn,
                                   EA_Atom* atom, fh_istream ss )
    {
        fh_model m = RDFCore::getDefaultFerrisModel();
        string s = StreamToString( ss );
        LG_RDF_D << "Context::SL_setIsUnRemembered() s:" << s << endl;
        if( isTrue( s ) )
        {
            if( fh_node n = Remembrance::getHistoryNode( c ) )
            {
                fh_node hn = Remembrance::_Base::getHistoryNode();
                if( fh_node uuidnode = Semantic::tryToGetUUIDNode( c ) )
                {
                    // We need to delete this for all files which are sharing the
                    // same UUID node as they effectively share the same history
                    // too
//                     // See how many files are referencing this one uuidnode
//                     fh_statement q = new Statement();
//                     q->setPredicate( Semantic::uuidPredNode() );
//                     q->setObject( uuidnode );
//                     int refCount = distance( m->findStatements( q ), StatementIterator() );
//                     if( refCount == 1 )
//                     {
//                         cerr << "Only a single reference to UUIDNode, removing the history stuff." << endl;
//                         m->eraseTransitive( n );
//                     }

                    m->eraseTransitive( n );
                    m->erase( uuidnode, hn, n );
                }
            }
        }
    }
    
    

    template <class Remembrance>
    fh_istream
    Context::SL_getLastRememberedSubContexts( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        fh_model m = RDFCore::getDefaultFerrisModel();
        fh_node n = Remembrance::getHistoryNode( c );
        string ret = "";
        if( n )
        {
            fh_node node = m->getObject( n, Remembrance::getMostRecentSubContextsNode() );
            if( node )
            {
                ret = node->toString();
            }
        }
        ss << ret;
        return ss;
    }

    template <class Remembrance>
    fh_istream
    Context::SL_getRemembranceTime( Context* c, const std::string& rdn, EA_Atom* atom )
    {
//        Time::Benchmark bm( "SL_getRemembranceTime(1) c:" + c->getURL() );
        
        fh_stringstream ss;
        fh_model m = RDFCore::getDefaultFerrisModel();
        fh_node n = Remembrance::getHistoryNode( c );
        string ret = "0";
        if( n )
        {
            fh_node node = m->getObject( n, Remembrance::getMostRecentTimeNode() );
            if( node )
            {
                ret = node->toString();
            }
        }
        ss << ret;
        return ss;
    }

    
    template <class Remembrance>
    fh_iostream
    Context::SL_getRemembranceCommand( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        fh_model m = RDFCore::getDefaultFerrisModel();
        fh_node n = Remembrance::getHistoryNode( c );
        string ret = "0";
        if( n )
        {
            fh_node node = m->getObject( n, Remembrance::getMostRecentCommandNode() );
            if( node )
            {
                ret = node->toString();
            }
        }
        ss << ret;
        return ss;
    }

    template <class Remembrance>
    void
    Context::SL_setRemembranceCommand( Context* c,
                                       const std::string& rdn,
                                       EA_Atom* atom, fh_istream ss )
    {
        fh_model m = RDFCore::getDefaultFerrisModel();
        time_t nowtt = Time::getTime();
        fh_node nownode = Node::CreateLiteral( tostr(nowtt) );
        string cmdstr = StreamToString( ss );
        fh_node cmdnode = Node::CreateLiteral( cmdstr );
        fh_node n = Remembrance::ensureHistoryNode( c );

        m->insert( n, Remembrance::getCommandNode(), cmdnode );
        m->insert( n, Remembrance::getTimeNode(),    nownode );

        m->set( n, Remembrance::getMostRecentCommandNode(), cmdnode );
        m->set( n, Remembrance::getMostRecentTimeNode(),    nownode );

        if( c->isParentBound() )
        {
            stringstream ctxlistss;
            ctxlistss << c->getDirName();
            fh_node ctxlistnode = Node::CreateLiteral( ctxlistss.str() );
            fh_context p = c->getParent();
            fh_node pnode = Remembrance::ensureHistoryNode( p );
            m->set( pnode, Remembrance::getMostRecentSubContextsNode(), ctxlistnode );
        }
        
        m->sync();
    }


    
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

    
void
Context::createStateLessAttributesForEmblem( fh_emblem em )
{
//     fh_etagere   et = Factory::getEtagere();
//     fh_emblem   eaOrderingRootEM = private_getAttributeRootEmblem( et );
//     const emblems_t& upset = em->getUpset();
//     if( find( upset.begin(), upset.end(), eaOrderingRootEM ) != upset.end() )
//     {
//         return;
//     }
    if( em->isTransitiveChildOfEAOrderingRootEmblem() )
        return;
//     if( starts_with( em->getName(), "emblem:" ))
//         return;
//     if( starts_with( em->getName(), "schema:" ))
//         return;
    
    string eaname = EANAME_SL_EMBLEM_PREKEY + em->getName();
    ContextClass_SLEA( eaname, &_Self::SL_hasEmblem, &_Self::SL_hasEmblem, &_Self::SL_hasEmblemStreamClosed, XSD_BASIC_BOOL );
    eaname = EANAME_SL_EMBLEM_ID_PREKEY + tostr(em->getID());
    ContextClass_SLEA( eaname, &_Self::SL_hasEmblem, &_Self::SL_hasEmblem, &_Self::SL_hasEmblemStreamClosed, XSD_BASIC_BOOL );

    eaname = EANAME_SL_EMBLEM_FUZZY_PREKEY + em->getName();
    ContextClass_SLEA( eaname, &_Self::SL_hasEmblemFuzzy, XSD_BASIC_DOUBLE );
    eaname = EANAME_SL_EMBLEM_ID_FUZZY_PREKEY + tostr(em->getID());
    ContextClass_SLEA( eaname, &_Self::SL_hasEmblemFuzzy, XSD_BASIC_DOUBLE );
    
    eaname = EANAME_SL_EMBLEM_TIME_PREKEY + em->getName() + "-mtime";
    ContextClass_SLEA( eaname,
                       &_Self::SL_getEmblemTime,
                       XSDBasic_t(FXD_UNIXEPOCH_T | FXDC_READONLY) );
    supplementStateLessAttributes_timet( eaname );
}

    static bool OnEmblemCreated_IGNORE_CREATED_EMBLEMS = false;
    
void
Context::OnEmblemCreated( fh_etagere et, fh_emblem em )
{
    LG_CTX_D << "Context::OnEmblemCreated() em:" << em->getName() << endl;
    cerr << "Context::OnEmblemCreated() em:" << em->getName()
         << " ignore:" << OnEmblemCreated_IGNORE_CREATED_EMBLEMS
         << endl;
    if( OnEmblemCreated_IGNORE_CREATED_EMBLEMS )
        return;
    
    createStateLessAttributesForEmblem( em );
}

stringset_t&
Context::getContextClassStatelessEANames()
{
    static stringset_t ret;
    return ret;
}

void
Context::ContextClass_SLEA( const std::string& rdn,
                            const StateLessIEA_t&  fi,
                            XSDBasic_t sct )
{
    getContextClassStatelessEANames().insert( rdn );
    tryAddStateLessAttribute( rdn, fi, sct );
}
void
Context::ContextClass_SLEA( const std::string& rdn,
                            const StateLessIEA_t&  fi,
                            const StateLessIOEA_t& fio,
                            const StateLessIOClosedEA_t& fioc,
                            XSDBasic_t sct )
{
    getContextClassStatelessEANames().insert( rdn );
    tryAddStateLessAttribute( rdn, fi, fio, fioc, sct );
}



void
Context::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            LG_CTX_D << "Context::createStateLessAttributes() path:" << getDirPath() << endl;

        
            ContextClass_SLEA( "attribute-count", SL_getAttributeCountStream, XSD_BASIC_INT );

            ContextClass_SLEA( "name",
                               SL_getDirNameStream,
                               SL_getDirNameIOStream,
                               SL_RenameContext,
                               XSD_BASIC_STRING );
            ContextClass_SLEA( "parent-name",
                               SL_getParentDirNameStream,
                               XSD_BASIC_STRING );
            ContextClass_SLEA( "parent-url",
                               SL_getParentURLStream,
                               FXD_URL );
            tryAddStateLessAttribute_Contents( "content",
                                               SL_getContentIStream, SL_getContentIOStream,
                                               FXD_BINARY );
            

            ContextClass_SLEA( "is-dir", &_Self::SL_getIsDir, XSD_BASIC_BOOL );
            ContextClass_SLEA( "is-dir-try-automounting",
                               &_Self::SL_getIsDirTryAutoMounting,
                               XSD_BASIC_BOOL );

            ContextClass_SLEA( "ferris-ctime",
                               &_Self::SL_getFerrisCTimeStream,
                               FXD_UNIXEPOCH_T );
            ContextClass_SLEA( "only-ferris-metadata-ctime",
                               &_Self::SL_getOnlyFerrisMetadataCTimeStream,
                               FXD_UNIXEPOCH_T );
            ContextClass_SLEA( "ferris-should-reindex-if-newer",
                               &_Self::SL_getFerrisShouldReindexIfNewerStream,
                               FXD_UNIXEPOCH_T );
            
            ContextClass_SLEA( "follow-link-as-ferris-fs-size",
                               &_Self::SL_getFollowLinkAsFerrisFSSizeStream, FXD_FILESIZE );
            

            ContextClass_SLEA( "mimetype", SL_getMimetypeStream, FXD_MIMETYPE );
            ContextClass_SLEA( "mimetype-from-content",
                                      SL_getMimetypeFromContentStream,
                                      FXD_MIMETYPE );
            ContextClass_SLEA( "mimetype-from-file-command",
                                      SL_getMimetypeFromFileCommandStream,
                                      FXD_MIMETYPE );
#ifdef HAVE_LIBFILE            
            ContextClass_SLEA( "filetype", SL_getFiletypeStream, XSD_BASIC_STRING );
#endif
            
            ContextClass_SLEA( "treeicon",
                                      SL_getTreeIconStream, FXD_URL_IMPLICIT_RESOLVE );
            
            ContextClass_SLEA( "name-extension", SL_getDirNameExtensionStream, XSD_BASIC_STRING );
            ContextClass_SLEA( "path", SL_getDirPathStream, XSD_BASIC_STRING );
            ContextClass_SLEA( "url", SL_getURLStream, FXD_URL );
            ContextClass_SLEA( "recommended-ea", SL_getRecommendedEAStream, FXD_EANAMES );
            ContextClass_SLEA( "recommended-ea-union",
                                      SL_getRecommendedEAUnionStream,
                                      FXD_EANAMES );
            ContextClass_SLEA( "recommended-ea-union-view",
                                      SL_getRecommendedEAUnionViewStream,
                                      FXD_EANAMES );
            ContextClass_SLEA( "recommended-ea-short",
                                      SL_getRecommendedEAShortStream, FXD_EANAMES );
            ContextClass_SLEA( "ea-names", SL_getEANamesStream, FXD_EANAMES );
            ContextClass_SLEA( "ea-names-no-qual", SL_getEANamesNoQualStream, FXD_EANAMES );
            ContextClass_SLEA( "subcontext-count", SL_getSubContextCountStream, XSD_BASIC_INT );
            ContextClass_SLEA( "has-subcontexts-guess",
                               SL_getHasSubContextsGuessStream, XSD_BASIC_BOOL );

            ContextClass_SLEA( "recursive-subcontext-count",
                                      SL_getRecursiveSubcontextCountStream,
                                      SL_getRecursiveSubcontextCountIOStream,
                                      SL_FlushAggregateData,
                                      XSD_BASIC_INT );

            ContextClass_SLEA( "recursive-subcontext-max-depth",
                               SL_getRecursiveSubcontextMaxDepthIOStream,
                               SL_getRecursiveSubcontextMaxDepthIOStream,
                               SL_FlushAggregateData,
                               XSD_BASIC_INT );

            ContextClass_SLEA( "is-image-object",     SL_getIsImage,     XSD_BASIC_BOOL );
            ContextClass_SLEA( "is-animation-object", SL_getIsAnimation, XSD_BASIC_BOOL );
            ContextClass_SLEA( "is-audio-object",     SL_getIsAudio,     XSD_BASIC_BOOL );
            ContextClass_SLEA( "is-source-object",    SL_getIsSource,    XSD_BASIC_BOOL );
            

            typedef list<string> digestNames_t;
            static digestNames_t digestNames;
            if( digestNames.empty() )
            {
                digestNames.push_back("md2");
                digestNames.push_back("md5");
                digestNames.push_back("sha1");
                digestNames.push_back("mdc2");
            }
            
            for( digestNames_t::iterator iter = digestNames.begin();
                 iter != digestNames.end(); ++iter )
            {
                ContextClass_SLEA(
                    *iter,
                    Loki::BindFirst(
                        Loki::Functor<fh_istream, LOKI_TYPELIST_4( string,
                                                              Context*,
                                                              const std::string&,
                                                              EA_Atom* ) >
                        (SL_getDigestStream), *iter ),
                    FXD_DIGEST );
            }

#ifdef FERRIS_HAVE_LIBZ
            ContextClass_SLEA( "crc32", SL_getCRC32IStream, FXD_DIGEST );
#endif

            ContextClass_SLEA( "ferris-current-time",
                               SL_getFerrisCurrentTimeIStream,
                               FXD_UNIXEPOCH_T );
            
            ContextClass_SLEA( "download-if-mtime-since",
                               SL_getDownloadIfMTimeSinceIStream,
                               SL_getDownloadIfMTimeSinceIOStream,
                               SL_downloadIfMTimeSinceClosed,
                               FXD_UNIXEPOCH_T );
            ContextClass_SLEA( "is-remote", SL_getIsRemote, XSD_BASIC_BOOL );


            ContextClass_SLEA( "as-xml",
                               SL_getAsXML, SL_getAsXML,
                               SL_getAsXMLClosed, FXD_XMLDOCSTRING );
            ContextClass_SLEA( "as-json", SL_getAsJSON, XSD_BASIC_STRING );
            ContextClass_SLEA( "as-text", SL_getAsText, XSD_BASIC_STRING );
            ContextClass_SLEA( "as-rdf",  SL_getAsRDF,  FXD_XMLDOCSTRING );

            
            ContextClass_SLEA( "is-active-view",     &_Self::SL_getIsActiveView,     XSD_BASIC_BOOL );
            ContextClass_SLEA( "force-passive-view", &_Self::SL_getForcePassiveView, XSD_BASIC_BOOL );

            fh_etagere et = Factory::getEtagere();
            try
            {
//                 emblems_t emlist = et->getAllEmblemsUniqueName();

//                 for( emblems_t::iterator ei = emlist.begin(); ei != emlist.end(); ++ei )
//                 {
//                     fh_emblem em = *ei;
//                     createStateLessAttributesForEmblem( em );
//                 }
                et->visitAllEmblems(
                    Etagere::f_emblemVisitor( this, &_Self::createStateLessAttributesForEmblem ) );
            }
            catch( exception& e )
            {
                LG_CTX_W << "Problem adding stateless ea for emblems:" << e.what() << endl;
            }
            et->getEmblemCreated_Sig().connect( sigc::mem_fun( *this, &_Self::OnEmblemCreated ));

            ContextClass_SLEA( "emblem:list",    &_Self::SL_getEmblemListAll,     FXD_STRINGLIST );
            ContextClass_SLEA( "emblem:list-ui", &_Self::SL_getEmblemListDefault, FXD_STRINGLIST );
            ContextClass_SLEA( "emblem:upset",   &_Self::SL_getEmblemUpset, FXD_STRINGLIST );
            ContextClass_SLEA( "emblem:has-medallion", &_Self::SL_getHasMedallion, XSD_BASIC_BOOL );

#ifdef HAVE_LIBTEXTCAT
            ContextClass_SLEA( "language-human", &_Self::SL_getLanguageHuman, XSD_BASIC_STRING );
#endif

#ifdef FERRIS_HAVE_GPGME
            ContextClass_SLEA( "has-valid-signature", SL_hasValidSignature, XSD_BASIC_BOOL );
            ContextClass_SLEA( "branchfs-signatures",
                                      SL_getSignaturesURL,
                                      FXD_URL_IMPLICIT_RESOLVE_FILESYSTEM );
#endif            

            ContextClass_SLEA( "branchfs-attributes",
                                      SL_getAttributesBranch,
                                      FXD_URL_IMPLICIT_RESOLVE_FILESYSTEM );
            ContextClass_SLEA( "branchfs-parents",
                                      SL_getParentsBranch,
                                      FXD_URL_IMPLICIT_RESOLVE_FILESYSTEM );
            ContextClass_SLEA( "branchfs-medallions",
                                      SL_getMedallionsBranch,
                                      FXD_URL_IMPLICIT_RESOLVE_FILESYSTEM );
            ContextClass_SLEA( "branchfs-remembrance",
                                      SL_getRemembranceBranch,
                                      FXD_URL_IMPLICIT_RESOLVE_FILESYSTEM );
            ContextClass_SLEA( "associated-branches",
                                      SL_getAssociatedBranches,
                                      FXD_URLLIST );
            ContextClass_SLEA( "associated-branches-url",
                                      SL_getAssociatedBranchesURL,
                                      FXD_URL_IMPLICIT_RESOLVE_FILESYSTEM );

#ifdef FERRIS_HAVE_XFSPROGS
            ContextClass_SLEA( "branchfs-extents",
                                      SL_getExtentsBranch,
                                      FXD_URL_IMPLICIT_RESOLVE_FILESYSTEM );
            
#endif

            ContextClass_SLEA( "latitude",
                               SL_getLatitudeStream,
                               SL_getLatitudeStream,
                               SL_LatitudeStreamClosed,
                               FXD_LATITUDE  );
            ContextClass_SLEA( "longitude",
                               SL_getLongitudeStream,
                               SL_getLongitudeStream,
                               SL_LongitudeStreamClosed,
                               FXD_LONGITUDE );
            ContextClass_SLEA( "geospatial-range",  SL_getGeoSpatialRangeStream,  XSD_BASIC_DOUBLE  );
            ContextClass_SLEA( "geospatial-emblem-name",
                               SL_getGeoSpatialEmblemNameStream,
                               XSD_BASIC_STRING  );
            ContextClass_SLEA( "geospatial-emblem-id",
                               SL_getGeoSpatialEmblemIDStream,
                               XSD_BASIC_INT  );
            ContextClass_SLEA( "google-maps-location", SL_getGoogleMapsLocationStream, XSD_BASIC_STRING );

            
            //
            // EA for handling which files in a collection have been seen and when
            //
            ContextClass_SLEA( "is-unseen",
                               SL_getIsUnRemembered<Remembrance_FileViewHistory>,
                               SL_getIsUnRemembered<Remembrance_FileViewHistory>,
                               SL_setIsUnRemembered<Remembrance_FileViewHistory>,
                               XSD_BASIC_BOOL );
            ContextClass_SLEA( "last-viewed-subcontexts",
                               SL_getLastRememberedSubContexts<Remembrance_FileViewHistory>,
                               XSD_BASIC_STRING );
            ContextClass_SLEA( "file-view-time",
                               SL_getRemembranceTime<Remembrance_FileViewHistory>,
                               FXD_UNIXEPOCH_T );
            ContextClass_SLEA( "file-view-command",
                               SL_getRemembranceCommand<Remembrance_FileViewHistory>,
                               SL_getRemembranceCommand<Remembrance_FileViewHistory>,
                               SL_setRemembranceCommand<Remembrance_FileViewHistory>,
                               XSD_BASIC_STRING );

            ContextClass_SLEA( "is-unedited",
                               SL_getIsUnRemembered<Remembrance_FileEditHistory>,
                               XSD_BASIC_BOOL );
            ContextClass_SLEA( "last-edited-subcontexts",
                               SL_getLastRememberedSubContexts<Remembrance_FileEditHistory>,
                               XSD_BASIC_STRING );
            ContextClass_SLEA( "file-edit-time",
                               SL_getRemembranceTime<Remembrance_FileEditHistory>,
                               FXD_UNIXEPOCH_T );
            ContextClass_SLEA( "file-edit-command",
                               SL_getRemembranceCommand<Remembrance_FileEditHistory>,
                               SL_getRemembranceCommand<Remembrance_FileEditHistory>,
                               SL_setRemembranceCommand<Remembrance_FileEditHistory>,
                               XSD_BASIC_STRING );


            ContextClass_SLEA( "ferris-post-copy-action",
                               SL_getFerrisPostCopyActionStream,
                               SL_getFerrisPostCopyActionStream,
                               SL_FerrisPostCopyActionStreamClosed,
                               XSD_BASIC_STRING );
            ContextClass_SLEA( "ferris-pre-copy-action",
                               SL_getFerrisPreCopyActionStream,
                               SL_getFerrisPreCopyActionStream,
                               SL_FerrisPreCopyActionStreamClosed,
                               XSD_BASIC_STRING );

            ContextClass_SLEA( "subtitles", SL_getSubtitlesStream, XSD_BASIC_STRING );

            ContextClass_SLEA( "video-format",  SL_getDotEAStream, XSD_BASIC_STRING );
            ContextClass_SLEA( "audio-format",  SL_getDotEAStream, XSD_BASIC_STRING );
            ContextClass_SLEA( "video-profile", SL_getDotEAStream, XSD_BASIC_STRING );
            ContextClass_SLEA( "audio-profile", SL_getDotEAStream, XSD_BASIC_STRING );

            
            LG_CTX_D << "setting up context stateless EA (done) " << endl;
        }
    }

    void
    Context::supplementStateLessAttributes_size( std::string an )
    {
        if( isAttributeBound( an, false ) )
        {
            string rdn = an + "-human-readable";
            if( !isAttributeBound( rdn, false ) )
            {
                tryAddStateLessAttribute( rdn, SL_getSizeHumanReadableStream, XSD_BASIC_STRING );
            }
        }
        if( isAttributeBound( an, false ) )
        {
            string rdn = an + CFG_CDROM_SIZE_BYTES_EA_POSTFIX;
            if( !isAttributeBound( rdn, false ) )
            {
                tryAddStateLessAttribute(
                    rdn,
                    SL_getSizeMediaCountStream<
                    CFG_CDROM_SIZE_BYTES_K,
                    CFG_CDROM_SIZE_BYTES_DEFAULT,
                    CFG_CDROM_SIZE_BYTES_EA_POSTFIX >,
                    XSD_BASIC_DOUBLE );
            }
        }
        if( isAttributeBound( an, false ) )
        {
            string rdn = an + CFG_DVD_SIZE_BYTES_EA_POSTFIX;
            if( !isAttributeBound( rdn, false ) )
            {
                tryAddStateLessAttribute(
                    rdn,
                    SL_getSizeMediaCountStream<
                    CFG_DVD_SIZE_BYTES_K,
                    CFG_DVD_SIZE_BYTES_DEFAULT,
                    CFG_DVD_SIZE_BYTES_EA_POSTFIX >,
                    XSD_BASIC_DOUBLE );
            }
        }
    }

    void
    Context::supplementStateLessAttributes_timet( std::string an_orig )
    {
        string an = an_orig;

//         cerr << "TEST - checking for an:" << an << endl;
//         {
//             TypeInfos_t ti;
//             getTypeInfos( ti );
//             cerr << " ti size:" << ti.size() << endl;
//             for( TypeInfos_t::iterator zi = ti.begin(); zi != ti.end(); ++zi )
//             {
//                 cerr << "    zi:" << zi->name() << endl;
//             }
//         }
        

        if( isAttributeBound( an, false ) )
        {
            an = an_orig;
            an += "-display";

//            cerr << "TEST trying to add an:" << an << endl;
            
            if( !isAttributeBound( an, false ) )
            {
//                cerr << "TEST OK! trying to add an:" << an << endl;
                tryAddStateLessAttribute( an,
                                          SL_getTimeStrFTimeIOStream,
                                          SL_getTimeStrFTimeIOStream,
                                          SL_setTimeStrFTimeStream,
                                          FXD_UNIXEPOCH_STRING );
            }

            an = an_orig;
            an += "-ctime";
            
            if( !isAttributeBound( an, false ) )
            {
                tryAddStateLessAttribute( an, SL_getXTimeCTimeStream,
                                          XSDBasic_t(FXD_UNIXEPOCH_STRING | FXDC_READONLY) );
            }

            an = an_orig;
            an += "-day-granularity";
            
            if( !isAttributeBound( an, false ) )
            {
                tryAddStateLessAttribute( an, SL_getXTimeDayGranularityStream,
                                          XSDBasic_t(FXD_UNIXEPOCH_T | FXDC_READONLY) );
            }

            an = an_orig;
            an += "-month-granularity";
            
            if( !isAttributeBound( an, false ) )
            {
                tryAddStateLessAttribute( an, SL_getXTimeMonthGranularityStream,
                                          XSDBasic_t(FXD_UNIXEPOCH_T | FXDC_READONLY) );
            }

            an = an_orig;
            an += "-year-granularity";
            
            if( !isAttributeBound( an, false ) )
            {
                tryAddStateLessAttribute( an, SL_getXTimeYearGranularityStream,
                                          XSDBasic_t(FXD_UNIXEPOCH_T | FXDC_READONLY) );
            }
            
        }
    }
    

/**
 * This is like addAttribute except it does no checking, and can add stateless
 * DONT use this method directly, either use addAttribute() or
 * tryAddStateLessAttribute()
 *
 * returns 1 if the attribute was added
 */
bool Context::setAttribute( const std::string& rdn,
                            EA_Atom* atx,
                            bool addToREA,
                            XSDBasic_t sct,
                            bool isStateLess )
    throw( AttributeAlreadyInUse )
{
    bool r = _Base::setAttribute( rdn, atx, addToREA, sct, isStateLess );

    if( rdn == "last-fetch-time" )
        supplementStateLessAttributes_timet("last-fetch-time");
    if( rdn == "feed-tested-time" )
        supplementStateLessAttributes_timet("feed-tested-time");
    
    return r;
}

bool
Context::getSubContextAttributesWithSameNameHaveSameSchema()
{
    return true;
}



void
Context::supplementStateLessAttributes( bool force )
    {
        string an;
        static bool FUZZ = getenv("LIBFERRIS_FUZZ");
            
//        cerr << "Context::supplementStateLessAttributes() path:" << getURL() << endl;

        if( FUZZ )
        {
            string an;
            
            an = "inode";
            if( !isAttributeBound( an, false ) )
            {
                tryAddStateLessAttribute( an, SL_getINodeFuzzStream, FXD_USERNAME );
            }
            an = "mtime";
            if( !isAttributeBound( an, false ) )
            {
                tryAddStateLessAttribute( an, SL_getMTimeFuzzStream, FXD_UNIXEPOCH_T );
            }
            an = "protection-ls";
            if( !isAttributeBound( an, false ) )
            {
                tryAddStateLessAttribute( an, SL_getProtectionLsFuzzStream, FXD_USERNAME );
            }
        }

        
        supplementStateLessAttributes_timet("atime");
        supplementStateLessAttributes_timet("ctime");
        supplementStateLessAttributes_timet("mtime");
        supplementStateLessAttributes_timet("dtime");
        supplementStateLessAttributes_timet("creation-time");
        supplementStateLessAttributes_timet("expire-time");

        supplementStateLessAttributes_timet("last-fetch-time");
        supplementStateLessAttributes_timet("feed-tested-time");
        supplementStateLessAttributes_timet("download-if-mtime-since");

        supplementStateLessAttributes_timet("recursive-subcontext-oldest-ctime");
        supplementStateLessAttributes_timet("recursive-subcontext-oldest-atime");
        supplementStateLessAttributes_timet("recursive-subcontext-oldest-mtime");

        supplementStateLessAttributes_timet("subcontext-oldest-ctime");
        supplementStateLessAttributes_timet("subcontext-oldest-atime");
        supplementStateLessAttributes_timet("subcontext-oldest-mtime");

        supplementStateLessAttributes_timet("rpm-buildtime");

        supplementStateLessAttributes_timet("file-view-time");

        supplementStateLessAttributes_timet("date-epoch");
        supplementStateLessAttributes_timet("date1-epoch");
        supplementStateLessAttributes_timet("date2-epoch");
        supplementStateLessAttributes_timet("date3-epoch");
        
        if( !isAttributeBound( "size" ) )
        {
            tryAddStateLessAttribute( "size",
                                      SL_getSizeFromContentIStream,
                                      FXD_FILESIZE );
            tryAddStateLessAttribute( "dontfollow-size",
                                      SL_getSizeFromContentIStream,
                                      FXD_FILESIZE );
        }
        
        supplementStateLessAttributes_size("size");
        supplementStateLessAttributes_size("recursive-subcontext-size");
        supplementStateLessAttributes_size("recursive-subcontext-size-in-blocks");
        supplementStateLessAttributes_size("subcontext-size");
        supplementStateLessAttributes_size("subcontext-size-in-blocks");



        {
            string an = "user-owner-number";
            if( isAttributeBound( an, false ) )
            {
                an = "user-owner-name";
                if( !isAttributeBound( an, false ) )
                {
                    tryAddStateLessAttribute( an, SL_getUserOwnerNameStream, FXD_USERNAME );
                }
            }

            if( FUZZ )
            {
                string an = "user-owner-number";
                if( !isAttributeBound( an, false ) )
                {
                    tryAddStateLessAttribute( an, SL_getUserOwnerNumberFuzzStream, FXD_USERNAME );
                }
                an = "user-owner-name";
                if( !isAttributeBound( an, false ) )
                {
                    tryAddStateLessAttribute( an, SL_getUserOwnerNameFuzzStream, FXD_USERNAME );
                }
            }
        }
            

        
        {
            string an = "group-owner-number";
            if( isAttributeBound( an, false ) )
            {
                an = "group-owner-name";
                if( !isAttributeBound( an, false ) )
                {
                    tryAddStateLessAttribute( an, SL_getGroupOwnerNameStream, FXD_GROUPNAME );
                }
            }

            if( FUZZ )
            {
                string an = "group-owner-number";
                if( !isAttributeBound( an, false ) )
                {
                    tryAddStateLessAttribute( an, SL_getGroupOwnerNumberFuzzStream, FXD_GROUPNAME );
                }
                an = "group-owner-name";
                if( !isAttributeBound( an, false ) )
                {
                    tryAddStateLessAttribute( an, SL_getGroupOwnerNameFuzzStream, FXD_GROUPNAME );
                }
            }
        }
        
        {
            string an = "ferris-current-time";
            if( !isAttributeBound( an, false ) )
            {
                tryAddStateLessAttribute( an, SL_getCurrentTimeStream, FXD_UNIXEPOCH_T );
                supplementStateLessAttributes_timet( an );
            }
        }
    }

    void
    Context::createAttributes()
    {
        priv_createAttributes();
    }

    void
    Context::priv_createAttributes()
    {
        LG_CTX_D << "Context::priv_createAttributes(top)" << endl;
//        cerr << "Context::priv_createAttributes(top) path:" << getDirPath() << endl;
    
//    Attribute::priv_createAttributes();
    
//         LG_ATTR_I << "Context::priv_createAttributes() creating EAGenFactories for:"
//                   << getDirPath() << endl;
        LG_CTX_D << "Context::priv_createAttributes(make external) path:" << getDirPath() << endl;
//         cerr << "Context::priv_createAttributes(make external) path:" << getDirPath() << endl;
//         BackTrace();

//         cerr << "Context::priv_createAttributes() "
//              << " path:" << getDirPath()
//              << endl;
//         BackTrace();

//         cerr << "priv_createAttributes... getDirPath:" << getDirPath() << endl;
//         cerr << "priv_createAttributes... getURL:" << getURL() << endl;
//         if( starts_with( getDirPath(), "/tmp/import/ipinfo." ))
//         {
//             return;
//         }
//         if( starts_with( getDirPath(), "ea/name" )
//             || starts_with( getDirPath(), "ea/value" )
//             || starts_with( getDirPath(), "/tmp/import/ipinfo.db" ))
//         {
//             return;
//         }
        
        
        static const gchar* LIBFERRIS_DISABLE_EAGEN_KDE = g_getenv ("LIBFERRIS_DISABLE_EAGEN_KDE");
        static const gchar* LIBFERRIS_DISABLE_EAGEN_RDF = g_getenv ("LIBFERRIS_DISABLE_EAGEN_RDF");
        static const gchar* LIBFERRIS_DISABLE_EAGEN_EARL_REGEX = g_getenv ("LIBFERRIS_DISABLE_EAGEN_EARL_REGEX");

        if( LIBFERRIS_DISABLE_EAGEN_EARL_REGEX )
        {
            static fh_rex r = toregexh( LIBFERRIS_DISABLE_EAGEN_EARL_REGEX );
            string earl = getURL();

            LG_CTX_D << "LIBFERRIS_DISABLE_EAGEN_EARL_REGEX:" << LIBFERRIS_DISABLE_EAGEN_EARL_REGEX << endl;
            LG_CTX_D << "earl:" << earl << endl;
            if( regex_match( earl, r ))
            {
                LG_CTX_D << "LIBFERRIS_DISABLE_EAGEN_EARL_REGEX decides to skip EA creation for earl:" << earl << endl;
                return;
            }
        }

//        cerr << "Context::priv_createAttributes() url:" << getURL() << endl;
//        BackTrace();
        
        /*
         * FIXME: should really cache this.
         */
        for( int Pri = AttributeCreator::CREATE_PRI_MAX_INTERNAL_USE_ONLY; Pri >= 0; --Pri )
        {
//             for( EAGenFactorys_t::const_iterator iter = getEAGenFactorys().begin();
//                  iter != getEAGenFactorys().end(); iter++ )
            ensureEAGenFactorysSetup();
            EAGenFactorys_iterator iterend = EAGenFactorys_iterator( this, true );
            for( EAGenFactorys_iterator iter = EAGenFactorys_iterator( this );
                 iter != iterend; ++iter )
            {
                
                LG_CTX_D << "Context::priv_createAttributes() "
                         << " path:" << getDirPath()
                         << " pri:" << (*iter)->getCreatePriority()
                         << endl;
                
                if( (*iter)->getCreatePriority() == Pri )
                {
                    if( LIBFERRIS_DISABLE_EAGEN_KDE || LIBFERRIS_DISABLE_EAGEN_RDF )
                    {
                        if( StaticGModuleMatchedEAGeneratorFactory* ff
                            = dynamic_cast<StaticGModuleMatchedEAGeneratorFactory*>( GetImpl(*iter) ) )
                        {
                            const std::string& sn = ff->getShortName();
                            if( LIBFERRIS_DISABLE_EAGEN_RDF && sn == "rdf" )
                            {
                                continue;
                            }
                            if( LIBFERRIS_DISABLE_EAGEN_KDE && sn == "kde3_metadata" )
                            {
                                continue;
                            }
                        }
                    }
                    
                    try
                    {
                        LG_CTX_D << "About to trybrew this:" << getURL() << endl;
                        (*iter)->tryBrew( this );
                    }
                    catch( exception& e )
                    {
                        LG_ATTR_D << "e:" << e.what() << endl;
                    }
                }
            }
        }
        
//         cerr << "Context::priv_createAttributes(end) "
//              << " path:" << getDirPath()
//              << endl;
//         LG_ATTR_I << "Context::priv_createAttributes(3) creating EAGenFactories for:"
//                   << getDirPath() << endl;

        imageEAGenerator_priv_createAttributes();
    }


    
    // typedef std::hash_map< std::string, MatchedEAGeneratorFactory* (*)() > StaticEAGenFactorysMap_t;
    void
    Context::getStaticEAGenFactorys( EAGenData& ret, bool& SLEAGenDynamic )
    {
        static s_StatelessEAGenFactorys_t          s_StatelessEAGenFactorys;
        static bool                                s_StatelessEAGenFactorysHasDynamic = false;
        static s_StatefullEAGenFactorysFactorys_t  s_StatefullEAGenFactorysFactorys;

//        string EAGeneratorsPath = PREFIX + "/lib/ferris/plugins/eagenerators/";
        string EAGeneratorsPath = makeFerrisPluginPath( "eagenerators" );
        static bool virgin = true;
        typedef list< MatchedEAGeneratorFactory* (*)() > StatefullEAGenFactorys_t;
        static StatefullEAGenFactorys_t StatefullEAGenFactorys;

        if( virgin )
        {
            virgin = false;

            typedef FERRIS_STD_HASH_MAP< std::string, MatchedEAGeneratorFactory* (*)() > StaticEAGenFactorysMap_t;
            StaticEAGenFactorysMap_t StaticEAGenFactorysMap;
            
            MatchedEAGeneratorFactory* (*MakeFactory)() = 0;
            DIR *d; struct dirent *e;

            if ((d = opendir (EAGeneratorsPath.c_str())) == NULL)
            {
                LG_PLUGIN_ER << "Can not open system plugin dir :" << EAGeneratorsPath << endl;
            }
            else
            {
                while ((e = readdir (d)) != NULL)
                {
                    string fn = e->d_name;
            
                    LG_PLUGIN_I << "Found:" << fn << endl;
                    if( ends_with( fn, "factory.so" ) )
                    {
                        try
                        {
                            LG_PLUGIN_I << "Loading plugin:" << fn << endl;
                    
                            ostringstream ss;
                            ss << EAGeneratorsPath << fn;
                    
                            GModule*   ghandle;
                            ghandle = g_module_open ( tostr(ss).c_str(), G_MODULE_BIND_LAZY);
                            if (!ghandle)
                            {
                                ostringstream ss;
                                ss  << "Error, unable to open module file, %s"
                                    << g_module_error ()
                                    << endl;
                                LG_PLUGIN_I << tostr(ss) << endl;
                                cerr << tostr(ss) << endl;
                                Throw_GModuleOpenFailed( tostr(ss), 0 );
                            }

                            if (!g_module_symbol (ghandle, "MakeFactory", 
                                                  (gpointer*)&MakeFactory))
                            {
                                ostringstream ss;
                                ss  << "Error, unable to resolve MakeFactory in module file, %s"
                                    << g_module_error()
                                    << endl;
                                LG_PLUGIN_I << tostr(ss) << endl;
                                cerr << tostr(ss) << endl;
                                Throw_GModuleOpenFailed( tostr(ss), 0 );
                            }

                            LG_PLUGIN_I << "Adding EA Generator factory object:" << fn << endl;
                            StaticEAGenFactorysMap[fn] = MakeFactory;
                        }
                        catch( GModuleOpenFailed& e )
                        {
                            LG_PLUGIN_ER << "Failed to load plugin:" << fn << endl;
                        }
                    }
                }
                closedir (d);
            }
        

            /*
             * Go through the map and create a factory for each entry.
             */
            for( StaticEAGenFactorysMap_t::const_iterator iter = StaticEAGenFactorysMap.begin();
                 iter != StaticEAGenFactorysMap.end(); ++iter )
            {
                fh_MatchedEAGeneratorFactory f = iter->second();
                
                if( f->hasState() )
                {
                    /* Factory keeps state per context. Must clone generic factory */
                    s_StatefullEAGenFactorysFactorys.push_back( iter->second );
//                    StatefullEAGenFactorys.push_back( iter->second );
                }
                else
                {
                    /* Reuse global reference */
                    s_StatelessEAGenFactorys.push_back( f );
//                    StaticEAGenFactorys.push_back( f );
                }
            }

            AppendAllStaticEAGeneratorFactories_Stateless( s_StatelessEAGenFactorys );

            for( s_StatelessEAGenFactorys_t::const_iterator iter = s_StatelessEAGenFactorys.begin();
                 iter != s_StatelessEAGenFactorys.end(); ++iter )
            {
                fh_MatchedEAGeneratorFactory f = *iter;
                s_StatelessEAGenFactorysHasDynamic |= f->isDynamic();
                
                if( f->isDynamic() )
                {
                    cerr << "Warning, stateless EA Generator is dynamic." << endl;
                }
            }
        }
        SLEAGenDynamic = s_StatelessEAGenFactorysHasDynamic;
        ret.SL = &s_StatelessEAGenFactorys;
        ret.SF = &s_StatefullEAGenFactorysFactorys;
    }
    
const Context::s_StatelessEAGenFactorys_t&
Context::getStatelessEAGenFactorys()
{
    bool SLEAGenDynamic;
    EAGenData D;
    getStaticEAGenFactorys( D, SLEAGenDynamic );
    return *(D.SL);
}



    void
    Context::ensureEAGenFactorysSetup()
    {
        LG_CTX_D << "Context::ensureEAGenFactorysSetup() TOP dn:" << getDirPath() << endl;
        
        if( EAGenFactorys_isVirgin )
        {
            EAGenFactorys_isVirgin = false;
            LG_CTX_D << "Context::ensureEAGenFactorysSetup(virg) dn:" << getDirPath() << endl;

            bool SLEAGenDynamic = false;
            EAGenData D;
            getStaticEAGenFactorys( D, SLEAGenDynamic );
            HaveDynamicAttributes |= SLEAGenDynamic;
            const s_StatefullEAGenFactorysFactorys_t& SF = *(D.SF);
            
            for( s_StatefullEAGenFactorysFactorys_t::const_iterator iter = SF.begin();
                 iter != SF.end(); ++iter )
            {
                fh_MatchedEAGeneratorFactory f = (*iter)();
                
                m_StatefullEAGenFactorys.push_back( f );
                if( f->isDynamic() )
                {
                    HaveDynamicAttributes = true;
                }
            }

            HaveDynamicAttributes |=
                AppendAllStaticEAGeneratorFactories_Statefull( m_StatefullEAGenFactorys );
        }

        LG_CTX_D << " Context::ensureEAGenFactorysSetup() EXIT dn:" << getDirPath() << endl;
    }
    
    
//     /**
//      * We can really only get away with coding this method this way because its only
//      * called from getEAGenFactorys() and only once per context. If we wanted to
//      * generalize it then we would have to keep track of the factories with
//      * hasState()==true and make only one instance of those factories per context.
//      */
//     Context::EAGenFactorys_t
//     Context::getStaticEAGenFactorys()
//     {
//           string EAGeneratorsPath = makeFerrisPluginPath( "eagenerators" );
//         static bool virgin = true;
//         static EAGenFactorys_t StaticEAGenFactorys;
//         typedef list< MatchedEAGeneratorFactory* (*)() > StatefullEAGenFactorys_t;
//         static StatefullEAGenFactorys_t StatefullEAGenFactorys;
//         EAGenFactorys_t ret;

//         if( virgin )
//         {
//             virgin = false;

//             typedef std::hash_map< std::string, MatchedEAGeneratorFactory* (*)() > StaticEAGenFactorysMap_t;
//             StaticEAGenFactorysMap_t StaticEAGenFactorysMap;
            
//             MatchedEAGeneratorFactory* (*MakeFactory)() = 0;
//             DIR *d; struct dirent *e;

//             if ((d = opendir (EAGeneratorsPath.c_str())) == NULL)
//             {
//                 LG_PLUGIN_ER << "Can not open system plugin dir :" << EAGeneratorsPath << endl;
//             }
//             else
//             {
//                 while ((e = readdir (d)) != NULL)
//                 {
//                     string fn = e->d_name;
            
//                     LG_PLUGIN_I << "Found:" << fn << endl;
//                     if( ends_with( fn, "factory.so" ) )
//                     {
//                         try
//                         {
//                             LG_PLUGIN_I << "Loading plugin:" << fn << endl;
                    
//                             ostringstream ss;
//                             ss << EAGeneratorsPath << fn;
                    
//                             GModule*   ghandle;
//                             ghandle = g_module_open ( tostr(ss).c_str(), G_MODULE_BIND_LAZY);
//                             if (!ghandle)
//                             {
//                                 ostringstream ss;
//                                 ss  << "Error, unable to open module file, %s"
//                                     << g_module_error ()
//                                     << endl;
//                                 LG_PLUGIN_I << tostr(ss) << endl;
//                                 cerr << tostr(ss) << endl;
//                                 Throw_GModuleOpenFailed( tostr(ss), 0 );
//                             }

//                             if (!g_module_symbol (ghandle, "MakeFactory", 
//                                                   (gpointer*)&MakeFactory))
//                             {
//                                 ostringstream ss;
//                                 ss  << "Error, unable to resolve MakeFactory in module file, %s"
//                                     << g_module_error()
//                                     << endl;
//                                 LG_PLUGIN_I << tostr(ss) << endl;
//                                 cerr << tostr(ss) << endl;
//                                 Throw_GModuleOpenFailed( tostr(ss), 0 );
//                             }

//                             LG_PLUGIN_I << "Adding EA Generator factory object:" << fn << endl;
//                             StaticEAGenFactorysMap[fn] = MakeFactory;
//                         }
//                         catch( GModuleOpenFailed& e )
//                         {
//                             LG_PLUGIN_ER << "Failed to load plugin:" << fn << endl;
//                         }
//                     }
//                 }
//                 closedir (d);
//             }
        

//             /*
//              * Go through the map and create a factory for each entry.
//              */
//             for( StaticEAGenFactorysMap_t::const_iterator iter = StaticEAGenFactorysMap.begin();
//                  iter != StaticEAGenFactorysMap.end(); ++iter )
//             {
//                 fh_MatchedEAGeneratorFactory f = iter->second();
                
//                 if( f->hasState() )
//                 {
//                     /* Factory keeps state per context. Must clone generic factory */
//                     StatefullEAGenFactorys.push_back( iter->second );
//                 }
//                 else
//                 {
//                     /* Reuse global reference */
//                     StaticEAGenFactorys.push_back( f );
//                 }
                
//             }
//         }

//         copy( StaticEAGenFactorys.begin(), StaticEAGenFactorys.end(), back_inserter( ret ));
        
//         /*
//          * Create a new statefull factory object for each EA Generator that
//          * has interest in this context object
//          */
//         for( StatefullEAGenFactorys_t::iterator iter = StatefullEAGenFactorys.begin();
//              iter != StatefullEAGenFactorys.end(); ++iter )
//         {
// //            const fh_MatchedEAGeneratorFactory& f = *iter;
// //            if( f->hasInterest( this ) )
//                 ret.push_back( (*iter)() );
//         }

//         AppendAllStaticEAGeneratorFactories( ret );

//         return ret;
//     }

//     const Context::EAGenFactorys_t&
//     Context::getEAGenFactorys()
//     {
//         LG_CTX_D << "Context::getEAGenFactorys() TOP dn:" << getDirPath() << endl;
        
//         if( EAGenFactorys_isVirgin )
//         {
//             EAGenFactorys_isVirgin = false;
// //             cerr << "Context::getEAGenFactorys() creating factories -- START this:"
// //                  << toVoid(this) << endl;
//             const EAGenFactorys_t& f = getStaticEAGenFactorys();

//             for( EAGenFactorys_t::const_iterator iter = f.begin();
//                  iter != f.end();
//                  iter++ )
//             {
//                 if( (*iter)->isDynamic() )
//                 {
//                     HaveDynamicAttributes = true;
//                 }

//                 // Note that getStaticEAGenFactorys() will have already made a
//                 // copy of all factories that require state tracking.
// //                 if( (*iter)->hasState() )
// //                 {
// //                     /* Factory keeps state per context. Must clone generic factory */
// //                     MatchedEAGeneratorFactory& f = **iter;
// //                     EAGenFactorys.push_back( f() );
// //                 }
// //                 else
// //                 {
// //                     /* Reuse global reference */
//                     EAGenFactorys.push_back( *iter );
// //                }
//             }
// //             cerr << "Context::getEAGenFactorys() creating factories -- END" << endl;
//         }

//         LG_CTX_D << " Context::getEAGenFactorys() EXIT dn:" << getDirPath() << endl;
// //         cerr << " Context::getEAGenFactorys() EXIT dn:" << getDirPath()
// //              << " EAGenFactorys.size:" << EAGenFactorys.size()
// //              << endl;
//         return EAGenFactorys;
//     }

    bool
    Context::VetoEA()
    {
        return false;
    }

    void
    Context::RemoveOurselfFromFreeList()
    {
        if( !WeAreInFreeList )
            return;
        
        LG_CTX_D << "RemoveOurselfFromFreeList() p:" << getDirPath() << endl;
//         if( !isParentBound() )
//             CERR << "RemoveOurselfFromFreeList() c:" << (void*)this << endl;
            
        WeAreInFreeList = 0;
        getCacheManager()->removeFromFreeList( this );

//         /*
//          * PURE DEBUG
//          */
//         {
//             string ppath = "";
//             if( isParentBound() )
//                 ppath = getParent()->getDirPath();
            
//             fh_stringstream ss;
//             dumpRefDebugData( ss );
//             cerr << "+++ n:" << WeAreInFreeList
//                  << " #:" << NumberOfSubContexts << " cl:" << isReClaimable()
// //                 << " p:" << ppath
//                  << " " << tostr(ss);
//         }
    }



    /**
     * Make sure that we are disposible and then add our self to the free list if
     * we can be claimed.
     */
    void
    Context::TryToAddOurselfToFreeList()
    {
        if( WeAreInFreeList )
            return;

        if( isBound( CoveredContext ) )
        {
//             /* PURE DEBUG */
//             {
//                 fh_stringstream ss;
//                 dumpRefDebugData( ss );
//                 cerr << "tryaddfree1 n:" << WeAreInFreeList
//                      << " #:" << NumberOfSubContexts << " cl:" << isReClaimable()
//                      << " " << tostr(ss);
//             }

            
            /*
             * VM.clean.3: Can cleanup a overmount tree when the root node has rc=2
             */
            if( !isReClaimable() )
            {
                return;
            }
        }
        else
        {
            if( !isParentBound() )
            {
                /*
                 * Allow the reclaiming of PCCTS mounts only when the root is not in use.
                 */
#ifdef PCCTSCTX
                // if( childContext* cc = dynamic_cast<childContext*>(this))
                // {
                //     if( isReClaimable() )
                //     {
                //         getCacheManager()->addToFreeList( this );
                //     }
                // }
                // else
#endif
                if( CreateMetaDataContext* cc = dynamic_cast<CreateMetaDataContext*>( this ) )
                {
                    if( getItems().empty() && isReClaimable() )
                    {
//                         cerr << "adding-to-free-list cmdc:" << (void*)cc
//                              << " rc:" << ref_count
//                              << " mRC:" << getMinimumReferenceCount()
//                              << endl;
//                         BackTrace();
                        getCacheManager()->addToFreeList( this );
                    }
                }
                
                return;
            }
            
    
            if( !isReClaimable() )
                return;
        }
        
        LG_VM_D << "TryToAddOurselfToFreeList() rc:" << ref_count 
                << " this:" << toVoid(this)
                << " path:" << getDirPath() 
                << endl;
    
//        WeAreInFreeList = 1;
        getCacheManager()->addToFreeList( this );

//         /*
//          * PURE DEBUG
//          */
//         {
//             fh_stringstream ss;
//             dumpRefDebugData( ss );
//             cerr << "--- n:" << WeAreInFreeList
//                  << " #:" << NumberOfSubContexts << " cl:" << isReClaimable()
//                  << " " << tostr(ss);
//             if( isBound(CoveredContext) )
//             {
//                 cerr << " covered:" << CoveredContext->getDirPath() << endl;
//             }
// //             {
// //                 fh_stringstream ss;
// //                 dumpEntireContextListMemoryManagementData( ss );
// //                 cerr << "->>-------------------------------------------------------------------------<<-\n"
// //                      << StreamToString(ss)
// //                      << "->>-------------------------------------------------------------------------<<-\n"
// //                      << endl;
// //             }
//         }
        

        /*
         * If we have just become claimable then we possibly allow a reclaim
         * Note that we must have children for this to happen to avoid reclaiming
         * ourself during a read().
         *
         * VM.clean.2: When a context becomes claimable and
         *             A. is not a overmounted context and has children
         *             B. is a overmount and is the root of that overmount and
         *                CoveredContext is not referenced by the user
         */
        if( NumberOfSubContexts )
        {
            Context* root = this;
            while( root->isParentBound() )
                root = root->getParent();

            bool invokeClean = false;

            if( !isBound( root->CoveredContext ) )
            {
                /* Subcase A */
                invokeClean = true;
            }
            else
            {
                /* Subcase B */

                Context* cc = GetImpl(root->CoveredContext);
                if( cc->ref_count == 1 + getMinimumReferenceCount() )
                {
                    /* CoveredContext reference plus the minimum */
                    invokeClean = true;
                }
            }

            if( invokeClean )
            {
                LG_VM_D << "CALLING getCacheManager()->AutoClean()... "
                        << " from Context::TryToAddOurselfToFreeList()" << endl;
                getCacheManager()->AutoClean();
            }
        }
    }

    int
    Context::getMinimumReferenceCount()
    {
        /*
         * VM.clean.3: Can cleanup a overmount tree when the root node has rc=2
         * parent is not bound for a overmount context, so the overmount context
         * reference cancels out the parent reference.
         * Conversely a context that has a overmount on it has the CoveredContext
         * handle of the stacked context pointing back at it, so we need to adjust
         * for the base context of a overmount.
         */
        /*
         * If we leave the base context out of the freelist, when we unmount the overmount
         * then the back reference is dropped and the base will become freeable.
         */
        return 1 + getItems().size();
//         int sz = max( NumberOfSubContexts, gint32(getItems().size()) );
//         return 1 + sz;
    }

    bool
    Context::isAttributeBound( const std::string& rdn, bool createIfNotThere )
        throw( NoSuchAttribute )
    {
        try
        {
            if( SL::isAttributeBound( rdn, createIfNotThere ) )
                return true;
            
            LG_ATTR_D << "Context::isAttributeBound() rdn:" << rdn << endl;
            if( createIfNotThere )
            {
                tryToFindAttributeByOverMounting( rdn );
            }
            LG_ATTR_D << "Context::isAttributeBound(2) rdn:" << rdn << endl;
            if( isBound(OverMountContext_Delegate) )
            {
                return OverMountContext_Delegate->isAttributeBound( rdn,
                                                                    createIfNotThere );
            }

            LG_ATTR_D << "Context::isAttributeBound(4) rdn:" << rdn << endl;
            return SL::isAttributeBound( rdn, createIfNotThere );
        }
        catch( exception& e )
        {
            LG_ATTR_D << "Context::isAttributeBound(e) rdn:" << rdn
                      << " e:" << e.what()
                      << endl;
            stringstream ss;
            ss << "No such attribute:" << rdn
               << " e:" << e.what();
            Throw_NoSuchAttribute( ss.str(), this );
        }
    }
    


    fh_attribute
    Context::getAttribute( const string& rdn ) throw( NoSuchAttribute )
    {
        LG_CTX_D << "Context::getAttribute() :" << rdn << endl;
//        cerr << "Context::getAttribute() :" << rdn << " AttributesHaveBeenCreated:" << AttributesHaveBeenCreated << endl;

        // Handled in getAttribute() now.
//         if( rdn == "." )
//         {
//             return new AttributeProxy( this, "" );
// //            return new AttributeProxy( this );
//         }

        if( rdn == "ea-names" )
            return SL::getAttribute( rdn );

        if( rdn == "recommended-ea" )
        {
            return SL::getAttribute( "recommended-ea-union" );
        }

        if( isBound(OverMountContext_Delegate) )
        {
            try
            {
                return OverMountContext_Delegate->getAttribute( rdn );
            }
            catch( ... )
            {
            
            }
        }

        // Try to hit a stateless EA directly.
        {
            SLAttributes_t* sl = getStateLessAttrs();
            SLAttributes_t::iterator iter = sl->find( rdn );
            if( sl->end() != iter )
            {
                return SL::getAttribute( rdn );
            }
        }
        
        if( !AttributesHaveBeenCreated )
        {
            ensureAttributesAreCreated( rdn );
        }
        
        
        if( !AttributeCollection::isAttributeBound( rdn, false ) )
        {
//             cerr << "Context::getAttribute() overmount seeking attr:" << rdn
//                  << " this.url:" << getURL()
//                  << " OMD_Bound:" << isBound(OverMountContext_Delegate)
//                  << " CC_Bound:" << isBound( CoveredContext )
//                  << " omfailed:" << m_overMountAttemptHasAlreadyFailed
//                  << endl;
//             BackTrace();
            tryToFindAttributeByOverMounting( rdn );
        }
        
        return SL::getAttribute( rdn );
    }

    
void
Context::setIsNativeContext()
{
    m_isNativeContext = true;
}
        
bool
Context::getIsNativeContext() const
{
    if( isBound(CoveredContext) )
    {
        return CoveredContext->getIsNativeContext();
    }
    
    return m_isNativeContext;
}


    
void
Context::setHasDynamicAttributes( bool v )
    {
        HaveDynamicAttributes = v;
    }

/**
 * If we still haven't found the attribute we are looking for then try
 * overmounting and seeing if the attribute exists.
 */
void
Context::tryToFindAttributeByOverMounting( const std::string& eaname )
{
//     if( eaname == "is-ferris-compressed" )
//         cerr << "Context::tryToFindAttributeByOverMounting(1)" << endl;
    
//     cerr << "tryToFindAttributeByOverMounting(top) this" << toVoid( this )
//          << " eaname:" << eaname << endl
//          << " OMD_Bound:" << isBound(OverMountContext_Delegate)
//          << " CC_Bound:" << isBound( CoveredContext )
//          << " OMD:" << toVoid( OverMountContext_Delegate )
//          << " CC:" << toVoid( CoveredContext )
//          << " omfailed:" << m_overMountAttemptHasAlreadyFailed
//          << " ReadingDir:" << ReadingDir
//          << " HaveReadDir:" << HaveReadDir
//          << endl;
//     cerr << "rdn:" << getDirName() << endl;
//     cerr << "path:" << getDirPath() << endl;
//     if(isBound( CoveredContext ))
//         cerr << " CoveredContext.ReadingDir:" << CoveredContext->ReadingDir << endl;
//     if(isBound( OverMountContext_Delegate ))
//         cerr << " OMC.ReadingDir:" << OverMountContext_Delegate->ReadingDir << endl;

//     if(isBound( CoveredContext ))
//         cerr << " CoveredContext.HaveReadDir:" << CoveredContext->HaveReadDir << endl;
//     if(isBound( OverMountContext_Delegate ))
//         cerr << " OMC.HaveReadDir:" << OverMountContext_Delegate->HaveReadDir << endl;

    
    if( ReadingDir )
        return;
    if( HaveReadDir )
        return;

    
    if( m_overMountAttemptHasAlreadyFailed )
        return;
    if( isBound( CoveredContext ) )
        return;
    if( eaname.empty() )
        return;
    
    if( eaname == "ferris-type" )
        return;
    if( eaname == "is-ferris-compressed" )
        return;
    if( eaname == "libferris-journal-changes" )
        return;
    if( starts_with( eaname, "medallion." ) )
        return;
    if( eaname == "emblem:emblems-pixbuf" || eaname == "treeicon-pixbuf" )
        return;
    if( eaname == "link-target" || eaname == "link-target-relative" )
        return;
    if( eaname == "ferris-icon-x" || eaname == "realpath" )
        return;
    // Dont go digging into a file or looking in strange places for XMP index data
    if( eaname == "http://witme.sf.net/libferris-core/xmp-0.1/index-offsets"
        || eaname == "http://witme.sf.net/libferris-core/xmp-0.1/index-mtime" )
    {
        LG_OVERMOUNT_I << "Not overmounting for XMP data." << endl;
        return;
    }
    

    
    static bool attemptingToAcquireAttributesViaOverMountShort = false;
    if( attemptingToAcquireAttributesViaOverMountShort )
        return;
    Util::ValueRestorer< bool > x( attemptingToAcquireAttributesViaOverMountShort, true );

//     LG_ATTR_D << "Context::tryToFindAttributeByOverMounting(top) eaname:" << eaname
//              << " url:" << getURL()
//              << " is-bound:" << isAttributeBound( eaname, false )
//              << " have-delegate:" << isBound(OverMountContext_Delegate)
//              << endl;
    if( isAttributeBound( eaname, false ) )
    {
//         cerr << "Attribute is already bound! eaname:" << eaname
//              << " url:" << getURL()
//              << endl;
//         BackTrace();
        return;
    }
    
    LG_OVERMOUNT_I << "tryToFindAttributeByOverMounting(top) eaname:" << eaname
                   << " isBound:" << isAttributeBound( eaname, false )
                   << " OMD_Bound:" << isBound(OverMountContext_Delegate)
                   << " CC_Bound:" << isBound( CoveredContext )
                   << " omfailed:" << m_overMountAttemptHasAlreadyFailed
                   << endl;
    LG_ATTR_I << "tryToFindAttributeByOverMounting(top) eaname:" << eaname
              << " isBound:" << isAttributeBound( eaname, false )
              << " OMD_Bound:" << isBound(OverMountContext_Delegate)
              << " CC_Bound:" << isBound( CoveredContext )
              << " omfailed:" << m_overMountAttemptHasAlreadyFailed
              << endl;
//    BackTrace();

    LG_ATTR_D << " eaname:" << eaname
             << " isBound:" << isAttributeBound( eaname, false )
             << " OMD_Bound:" << isBound(OverMountContext_Delegate)
             << endl;
    
    
    if( !isAttributeBound( eaname, false ) && !isBound(OverMountContext_Delegate) )
    {
        LG_ATTR_D << "Context::tryToFindAttributeByOverMounting(passed test) eaname:" << eaname
                  << " this:" << getURL()
                  << " HaveDynamicAttributes:" << HaveDynamicAttributes
                  << " isbound:" << isAttributeBound( eaname, false)
                  << " trying overmounting of context to find attribute"
                  << endl;

        static bool attemptingToAcquireAttributesViaOverMount = false;
        if( !attemptingToAcquireAttributesViaOverMount )
        {
            Util::ValueRestorer< bool > x( attemptingToAcquireAttributesViaOverMount, true );

            LG_ATTR_D << "tryToFindAttributeByOverMounting(trying) this" << toVoid( this )
                      << " eaname:" << eaname << endl;
            LG_OVERMOUNT_I << "tryToFindAttributeByOverMounting(trying) this" << toVoid( this )
                           << " eaname:" << eaname << endl;
//             cerr << "tryToFindAttributeByOverMounting(trying) this" << toVoid( this )
//                  << " eaname:" << eaname << endl;
//             BackTrace();
            
            tryToOverMount( true, true );
            if( isBound(OverMountContext_Delegate) )
            {
                // FIXME: Should have a better way to create root attributes.
                LG_ATTR_D << "Context::tryToFindAttributeByOverMounting(about to read) eaname:" << eaname
                     << " HaveDynamicAttributes:" << HaveDynamicAttributes
                     << " isbound:" << isAttributeBound( eaname, false)
                     << " trying overmounting of context to find attribute"
                     << endl;
                try
                {
                    readOverMount();
                }
                catch( exception& e )
                {
                    LG_ATTR_W << "Context::tryToFindAttributeByOverMounting(done with read) eaname:" << eaname
                              << " error reading overmount"
                              << " e:" << e.what()
                              << endl;
                }
                LG_ATTR_D << "Context::tryToFindAttributeByOverMounting(done with read) eaname:" << eaname
                     << " HaveDynamicAttributes:" << HaveDynamicAttributes
                     << " isbound:" << isAttributeBound( eaname, false)
                     << " trying overmounting of context to find attribute"
                     << endl;
            }
        }
    }
}


//     static guint32 getEAGeneratorsAlreadyFullyInvokedBitPattern( StaticGModuleMatchedEAGeneratorFactory* ff )
//     {
//         static guint32 nextID = 1;
// //        typedef map< string, Context::m_eagenerators_already_fully_invoked_cache_t > cache_t;
//         typedef map< string, guint32 > cache_t;
//         static cache_t cache;

//         cache_t::iterator e = cache.end();
//         cache_t::iterator iter = cache.find( ff->getShortName() );

//         if( iter == e )
//         {
//             guint32 ret = 0;
//             ret = nextID;
//             ++nextID;
//             iter = cache.insert( make_pair( ff->getShortName(), ret ) ).first;
//         }
//         return iter->second;
//     }

//     // instead of looking every time.....
//     class CachedPluginInterestedInEA
//     {
//     public:

//         const std::string& getShortName();
//         guint32 getBitf();
//     };
//     typedef map< string, list< CachedPluginInterestedInEA > > CachedPluginInterestedInEALookup_t;
//     CachedPluginInterestedInEALookup_t& getCachedPluginInterestedInEALookup()
//     {
//         static CachedPluginInterestedInEALookup_t ret;
//         return ret;
//     }
    

std::string
Context::getStrAttr_UsingRestrictedPlugins( const std::string& eaname,
                                            const std::set< std::string >& plugins,
                                            bool getAllLines )
{
    Context::ensureEAGenFactorysSetup();
    LG_MDSERV_D << "Context::getStrAttr_UsingRestrictedPlugins() eaname:" << eaname << endl;

    if( isAttributeBound( eaname, false ) )
    {
        LG_MDSERV_D << "Context::getStrAttr_UsingRestrictedPlugins(already bound) eaname:" << eaname << endl;
        string ret = getStrAttr( this, eaname, "", getAllLines, true );
        return ret;
    }
    
    std::set< std::string >::const_iterator plugins_end = plugins.end();

    LG_MDSERV_D << "Context::getStrAttr_UsingRestrictedPlugins(tryBrew block - start) eaname:" << eaname << endl;
    
    EAGenFactorys_iterator iterend = EAGenFactorys_iterator( this, true );
    for( EAGenFactorys_iterator iter = EAGenFactorys_iterator( this );
         iter != iterend; ++iter )
    {
        fh_MatchedEAGeneratorFactory f = *iter;
        if( StaticGModuleMatchedEAGeneratorFactory* ff
            = dynamic_cast<StaticGModuleMatchedEAGeneratorFactory*>( GetImpl(f) ) )
        {
            string n = ff->getShortName();

            if( plugins_end != plugins.find( n ) )
            {
                if( f->hasInterest( this ) )
                    f->tryBrew( this, eaname );
            }
        }
    }

    LG_MDSERV_D << "Context::getStrAttr_UsingRestrictedPlugins(tryBrew block - end) eaname:" << eaname << endl;
    
//    AttributesHaveBeenCreated = 1;
//    HaveDynamicAttributes = 0;

    if( isAttributeBound( eaname, false ) )
    {
        string ret = getStrAttr( this, eaname, "", getAllLines, true );
        LG_MDSERV_D << "Context::getStrAttr_UsingRestrictedPlugins(created) eaname:" << eaname << endl;
        return ret;
    }

    stringstream ss;
    ss << "Can not extract metadata. Perhaps it is handled by a plugin that is "
       << "not enabled for this dbus server";
    LG_MDSERV_I << tostr(ss) << endl;
    Throw_NoSuchAttribute( tostr(ss), this );
}
    
    

void
Context::ensureAttributesAreCreated( const std::string& eaname )
{
    static bool ensureAttributesAreCreatedRecursionGuard = false;
    if( ensureAttributesAreCreatedRecursionGuard )
        return;
    Util::ValueRestorer< bool > x( ensureAttributesAreCreatedRecursionGuard, true );

    bool SeekingMedallion  = !eaname.empty() && starts_with( eaname, "medallion." );
    bool SeekingFerrisType = !eaname.empty() && eaname == "ferris-type";
    bool SeekingEmblemsPixbuf = !eaname.empty() && eaname == "emblem:emblems-pixbuf";
    bool SeekingXMPIndexEA = eaname == "http://witme.sf.net/libferris-core/xmp-0.1/index-offsets"
        || eaname == "http://witme.sf.net/libferris-core/xmp-0.1/index-mtime";
    bool SeekingOnlyRDFandKernelEA = SeekingMedallion || SeekingFerrisType
        || SeekingFerrisType || SeekingEmblemsPixbuf || SeekingXMPIndexEA;
    
    LG_ATTR_D << "Context::ensureAttributesAreCreated(top) eaname:" << eaname
              << " HaveDynamicAttributes:" << HaveDynamicAttributes
              << " isbound:" << isAttributeBound( eaname, false)
              << endl;

    if( isBound(OverMountContext_Delegate) )
    {
        OverMountContext_Delegate->ensureAttributesAreCreated( eaname );
    }

    // Try to make fspot attributes a little quicker.
    if( starts_with( eaname, "fspot:" ) )
    {
        ensureEAGenFactorysSetup();
        EAGenFactorys_iterator iterend = EAGenFactorys_iterator( this, true );
        for( EAGenFactorys_iterator iter = EAGenFactorys_iterator( this );
             iter != iterend; ++iter )
        {
            fh_MatchedEAGeneratorFactory f = *iter;
            if( StaticGModuleMatchedEAGeneratorFactory* ff
                = dynamic_cast<StaticGModuleMatchedEAGeneratorFactory*>( GetImpl(f) ) )
            {
                string n = ff->getShortName();
                
                if(!( n == "f-spot" ))
                    continue;

                if( (*iter)->hasInterest( this ) )
                    (*iter)->tryBrew( this, eaname );
            }
        }

        if( isAttributeBound( eaname, false ) )
        {
            LG_CTX_D   << "Found f-spot attribute directly..." << endl;
            LG_FSPOT_D << "Found f-spot attribute directly..." << endl;
            return;
        }
    }
    
//     if( !SeekingMedallion
//         && !eaname.empty() && !isAttributeBound( eaname, false ) )
//     {
//         for( int Pri = AttributeCreator::CREATE_PRI_MAX_INTERNAL_USE_ONLY; Pri >= 0; --Pri )
//         {
// //             for( EAGenFactorys_t::const_iterator iter = getEAGenFactorys().begin();
// //                  iter != getEAGenFactorys().end(); iter++ )
//             ensureEAGenFactorysSetup();
//             EAGenFactorys_iterator iterend = EAGenFactorys_iterator( this, true );
//             for( EAGenFactorys_iterator iter = EAGenFactorys_iterator( this );
//                  iter != iterend; ++iter )
//             {
//                 if( (*iter)->getCreatePriority() == Pri && !(*iter)->isDynamic() )
//                 {
//                     if( StaticGModuleMatchedEAGeneratorFactory* ff
//                         = dynamic_cast<StaticGModuleMatchedEAGeneratorFactory*>( GetImpl(*iter) ) )
//                     {
//                         cerr << "TESTING eaname:" << eaname << " plugin:" << ff->getShortName() << endl;
//                         m_eagenerators_already_fully_invoked_cache_t bitf
//                             = getEAGeneratorsAlreadyFullyInvokedBitPattern( ff );
//                         cerr << "using bitf:" << bitf << endl;
                        
//                         if( m_eagenerators_already_fully_invoked_cache & bitf ) 
//                         {
//                             cerr << "already invoked! cache:" << m_eagenerators_already_fully_invoked_cache << endl;
//                             continue;
//                         }

//                         cerr << "  compare set sz:" << ff->getExplicitEANamesWhichWillBeCreatedSet().size() << endl;
                        
//                         const stringset_t& sc = ff->getExplicitEANamesWhichWillBeCreatedSet();
//                         if( sc.end() != sc.find( eaname ) )
//                         {
//                             cerr << "  compare OK for:" << ff->getShortName() << endl;
//                             if( (*iter)->hasInterest( this ) )
//                             {
//                                 cerr << "Trying plugin:" << ff->getShortName() << endl;
                                
//                                 (*iter)->tryBrew( this );
//                                 m_eagenerators_already_fully_invoked_cache |= bitf;
//                             }
//                         }
//                     }
//                 }
//             }
//         }

//         if( isAttributeBound( eaname, false ) )
//             return;
//     }

    // a hack...
//     if( eaname == "width" )
//     {
//         imageEAGenerator_priv_createAttributes();
//         if( isAttributeBound( eaname, false ) )
//             return;
//     }
    
    LG_ATTR_D << "Context::ensureAttributesAreCreated(0) eaname:" << eaname
              << " HaveDynamicAttributes:" << HaveDynamicAttributes
              << " isbound:" << isAttributeBound( eaname, false)
              << " SeekingMedallion:" << SeekingMedallion
              << " SeekingOnlyRDFandKernelEA:" << SeekingOnlyRDFandKernelEA
              << endl;

    // The case of image metadata is handled in Context::priv_getImage()
    // here, we handle other types of out of process EA generation like XMP
    // which could be relatively expensive the first time.
    if( tryToUseOutOfProcessMetadataServer() )
    {
        LG_ATTR_D << "Context::ensureAttributesAreCreated(oproc) eaname:" << eaname
                  << " testing if this is an image EA name....." << endl;
        LG_MDSERV_D << "Context::ensureAttributesAreCreated(oproc) eaname:" << eaname
                    << " testing if this is an image EA name....." << endl;

        if( isOutOfProcessMetadataAttribute( eaname ) )
        {
            const stringset_t& oprocNames = getOutOfProcess_EAGeneratorsStaticFactoriesShortNamesToUse();
            
            LG_ATTR_I << "Context::ensureAttributesAreCreated(oproc) eaname:" << eaname
                      << " this should be a out-of-process EA." << endl;
            LG_MDSERV_I << "Context::ensureAttributesAreCreated(oproc) eaname:" << eaname
                        << " this should be a out-of-process EA." << endl;
            
            for( int Pri = AttributeCreator::CREATE_PRI_MAX_INTERNAL_USE_ONLY; Pri >= 0; --Pri )
            {
                ensureEAGenFactorysSetup();
                EAGenFactorys_iterator iterend = EAGenFactorys_iterator( this, true );
                for( EAGenFactorys_iterator iter = EAGenFactorys_iterator( this );
                     iter != iterend; ++iter )
                {
                    fh_MatchedEAGeneratorFactory f = *iter;
                    if( StaticGModuleMatchedEAGeneratorFactory* ff
                        = dynamic_cast<StaticGModuleMatchedEAGeneratorFactory*>( GetImpl(f) ) )
                    {
                        string n = ff->getShortName();

                        if( oprocNames.end() != oprocNames.find( n ) )
                        {
                            LG_MDSERV_D << "Context::ensureAttributesAreCreated(oproc ea) eaname:" << eaname
                                        << " calling for shortName:" << n
                                        << endl;
                            
                            string earl = getURL();
                            string v = Ferris::syncMetadataServerGet( earl, eaname );

                            LG_MDSERV_D << "Context::ensureAttributesAreCreated(oproc ea) eaname:" << eaname
                                        << " have resulting value:" << v
                                        << endl;

                            addAttribute( eaname, v );
                            return;
                        }
                    }
                }
            }
        }
    }
    
    
    
    
    if( !AttributesHaveBeenCreated )
    {
        if( !SeekingOnlyRDFandKernelEA )
        {
//             cerr << "Context::ensureAttributesAreCreated() calling createAttributes() "
//                  << " url:" << getURL() << endl
//                  << " eaname:" << eaname
//                  << " SeekingOnlyRDFandKernelEA:" << SeekingOnlyRDFandKernelEA
//                  << endl;
//             BackTrace();
            
            LG_ATTR_I << "Context::ensureAttributesAreCreated() calling createAttributes() "
                      << " url:" << getURL() << endl
                      << " eaname:" << eaname
                      << " SeekingOnlyRDFandKernelEA:" << SeekingOnlyRDFandKernelEA
                      << endl;
//             cerr << "Context::ensureAttributesAreCreated() calling createAttributes() "
//                  << " url:" << getURL() << endl
//                  << " eaname:" << eaname
//                  << " SeekingOnlyRDFandKernelEA:" << SeekingOnlyRDFandKernelEA
//                  << " AttributesHaveBeenCreated:" << AttributesHaveBeenCreated
//                  << endl;
//            Time::Benchmark bm( (string)"Creating attributes, trigger ea:" + eaname + " for:" + getURL() );
            AttributesHaveBeenCreated = 1;
            createAttributes();
        }
    }

    LG_ATTR_D << "Context::ensureAttributesAreCreated(1) eaname:" << eaname
              << " HaveDynamicAttributes:" << HaveDynamicAttributes
              << " isbound:" << isAttributeBound( eaname, false)
              << " SeekingOnlyRDFandKernelEA:" << SeekingOnlyRDFandKernelEA
              << endl;

    /*
     * Some attributes can be created out of proc, for exmaple XFS/E3 native
     * filesystem EA can be made from another process, we should check in case
     * this has happened.
     */
    if( !isAttributeBound( eaname, false ) &&
        ( !eaname.empty() && HaveDynamicAttributes )
        || SeekingOnlyRDFandKernelEA )
    {
        LG_ATTR_D << "Context::ensureAttributesAreCreated(1.1) eaname:" << eaname
                  << " HaveDynamicAttributes:" << HaveDynamicAttributes
                  << " isbound:" << isAttributeBound( eaname, false )
                  << endl;

        for( int Pri = AttributeCreator::CREATE_PRI_MAX_INTERNAL_USE_ONLY; Pri >= 0; --Pri )
        {
//             for( EAGenFactorys_t::const_iterator iter = getEAGenFactorys().begin();
//                  iter != getEAGenFactorys().end(); iter++ )
            ensureEAGenFactorysSetup();
            EAGenFactorys_iterator iterend = EAGenFactorys_iterator( this, true );
            for( EAGenFactorys_iterator iter = EAGenFactorys_iterator( this );
                 iter != iterend; ++iter )
            {
                if( SeekingOnlyRDFandKernelEA )
                {
                    fh_MatchedEAGeneratorFactory f = *iter;
                    if( StaticGModuleMatchedEAGeneratorFactory* ff
                        = dynamic_cast<StaticGModuleMatchedEAGeneratorFactory*>( GetImpl(f) ) )
                    {
                        string n = ff->getShortName();

                        if(!( n == "rdf" || n == "KernelEA" ))
                            continue;

                        if( (*iter)->getCreatePriority() != Pri )
                            continue;
                        
//                         cerr << "Context::ensureAttributesAreCreated(1.pl) eaname:" << eaname
//                              << " plugin:" << ff->getShortName()
//                              << " url:" << getURL()
//                              << " (*iter)->isDynamic():" << (*iter)->isDynamic()
//                              << " (*iter)->hasInterest( this ) ):" << (*iter)->hasInterest( this )
//                              << endl;
                    }
                }
                
                if( (*iter)->getCreatePriority() == Pri &&
                    (*iter)->isDynamic() || SeekingOnlyRDFandKernelEA )
                {
                    try
                    {
                        LG_ATTR_D << "Context::ensureAttributesAreCreated(1.2) "
                                  << " eaname:" << eaname
                                  << " HaveDynamicAttributes:" << HaveDynamicAttributes
                                  << " isbound:" << isAttributeBound( eaname, false)
                                  << endl;
                            
                        if( (*iter)->hasInterest( this ) )
                            (*iter)->tryBrew( this, eaname );
                    }
                    catch( exception& e )
                    {
                        LG_ATTR_D << "e:" << e.what() << endl;
                    }
                }
            }
            
        }
    }

    /*
     * If we still haven't found the attribute we are looking for then try
     * overmounting and seeing if the attribute exists.
     */
    if( !SeekingOnlyRDFandKernelEA )
    {
        LG_ATTR_D << "Context::ensureAttributesAreCreated(2) eaname:" << eaname
                  << " HaveDynamicAttributes:" << HaveDynamicAttributes
                  << " isbound:" << isAttributeBound( eaname, false)
                  << endl;
        tryToFindAttributeByOverMounting( eaname );
        
        LG_ATTR_D << "Context::ensureAttributesAreCreated(end) eaname:" << eaname
                  << " HaveDynamicAttributes:" << HaveDynamicAttributes
                  << " isbound:" << isAttributeBound( eaname, false)
                  << endl;
    }
    
        
}


AttributeCollection::AttributeNames_t&
Context::getAttributeNames( AttributeNames_t& ret )
    {
        if( isBound(OverMountContext_Delegate) )
        {
            AttributeCollection::AttributeNames_t t1;
            AttributeCollection::AttributeNames_t t2;
            OverMountContext_Delegate->getAttributeNames( t1 );
            SL::getAttributeNames( t2 );
            return mergeAttributeNames( ret, t1, t2 );
        }
    
        return SL::getAttributeNames( ret );
    }

    int
    Context::getAttributeCount()
    {
        AttributeNames_t tmp;

        if( isBound(OverMountContext_Delegate) )
        {
            getAttributeNames( tmp );
            return tmp.size();
        }
        
        return AttributeCollection::getAttributeCount();
    }

    
    
    /**
     * The EA names are ordered based on a subtree of the emblems in your etagere.
     *
     * This gets the root of that tree.
     */
    fh_emblem
    Context::getAttributeRootEmblem()
    {
        LG_CTX_D << "Context::getAttributeRootEmblem(top) c:" << getURL() << endl;
        
        fh_etagere et = Factory::getEtagere();
        fh_emblem ret = private_getAttributeRootEmblem( et );

        LG_CTX_D << "Context::getAttributeRootEmblem(1)" << endl;
        emblems_t downset = ret->getDownset();
        LG_CTX_D << "Context::getAttributeRootEmblem(2)" << endl;

        AttributeCollection::AttributeNames_t an;
        getAttributeNames( an );
        
        stringset_t anset;
        copy( an.begin(), an.end(), inserter( anset, anset.end() ));
//         // PURE DEBUG
//         LG_CTX_D << "Context::getAttributeRootEmblem() c:" << getURL() << endl;
//         cerr << "getAttributeRootEmblem(an1) LISTTOP" << endl;
//         for( stringset_t::iterator si = anset.begin(); si != anset.end(); ++si )
//         {
//             cerr << "getAttributeRootEmblem(an1) " << *si << endl;
//         }
//         cerr << "getAttributeRootEmblem(an1) LISTEND" << endl;
        
        addEAToSet( anset,  getStrAttr( this, "recommended-ea-union-view", "" ));
        LG_CTX_D << "Context::getAttributeRootEmblem(3)" << endl;

//         // PURE DEBUG
//         for( AttributeCollection::AttributeNames_t::const_iterator ani = an.begin();
//              ani!=an.end(); ++ani )
//         {
//             string an = *ani;
//             LG_CTX_D << "Context::getAttributeRootEmblem(an) " << an << endl;
//         }
//         // PURE DEBUG
//         cerr << "getAttributeRootEmblem(an2) LISTTOP" << endl;
//         for( stringset_t::iterator si = anset.begin(); si != anset.end(); ++si )
//         {
//             cerr << "getAttributeRootEmblem(an2) " << *si << endl;
//         }
//         cerr << "getAttributeRootEmblem(an2) LISTEND" << endl;
        
                 
        for( emblems_t::iterator ci = downset.begin(); ci != downset.end(); ++ci )
        {
            fh_emblem em = *ci;

            stringset_t::iterator siter = anset.find( em->getName() );
            if( siter != anset.end() )
                anset.erase( siter );
        }
        LG_CTX_D << "Context::getAttributeRootEmblem(4)" << endl;
        LG_CTX_D << "Context::getAttributeRootEmblem(5) createing.sz:" << anset.size() << endl;

        if( !anset.empty() )
        {
            fh_emblem emblems = ret->obtainChild_EAOrdering( "emblems" );
            fh_emblem emblems_schemas = emblems->obtainChild_EAOrdering( "schemas" );
            fh_emblem emblems_fuzzy   = emblems->obtainChild_EAOrdering( "fuzzy" );
            fh_emblem emblems_time    = emblems->obtainChild_EAOrdering( "time" );
            fh_emblem emblems_has     = emblems->obtainChild_EAOrdering( "has-emblem" );
            fh_emblem emblems_id      = emblems->obtainChild_EAOrdering( "has-id" );
            fh_emblem schemas = ret->obtainChild_EAOrdering( "schemas" );
            fh_emblem schemas_emblems = schemas->obtainChild_EAOrdering( "emblems" );
            fh_emblem branchfs        = ret->obtainChild_EAOrdering( "branchfs" );
            fh_emblem timerelated     = ret->obtainChild_EAOrdering( "time" );
            fh_emblem dontfollow      = ret->obtainChild_EAOrdering( "dontfollow" );
            fh_emblem fs              = ret->obtainChild_EAOrdering( "fs" );
            fh_emblem subcontext      = ret->obtainChild_EAOrdering( "subcontext" );
            fh_emblem rpm             = ret->obtainChild_EAOrdering( "rpm" );
             
            SyncDelayer syncObj1;
            Util::ValueRestorer< bool > x( OnEmblemCreated_IGNORE_CREATED_EMBLEMS, true );
            int EAOrderingEmblemsCreatedCount = 0;
            for( stringset_t::const_iterator si = anset.begin(); si != anset.end(); ++si )
            {
                if( starts_with( *si, "emblem:emblem:" ))
                    continue;
            
                cerr << "creating an ea-ordering for emblem...:" << *si << endl;
                LG_CTX_N << "Context::getAttributeRootEmblem(CREATE) si:" << *si << endl;

                ++EAOrderingEmblemsCreatedCount;
                fh_cemblem e = et->createColdEmblem_EAOrdering( *si );
                if( starts_with( *si, "emblem:" ) )
                {
                    if( starts_with( *si, "emblem:schema:" ) )
                    {
                        link( emblems_schemas, e );
                    }
                    else if( contains( *si, "-fuzzy" ) )
                    {
                        link( emblems_fuzzy, e );
                    }
                    else if( contains( *si, "-mtime" ) )
                    {
                        link( emblems_time, e );
                    }
                    else if( starts_with( *si, "emblem:has-" ) )
                    {
                        link( emblems_has, e );
                    }
                    else if( starts_with( *si, "emblem:id-" ) )
                    {
                        link( emblems_id, e );
                    }
                    else
                    {
                        link( emblems, e );
                    }
                }
                else if( starts_with( *si, "schema:" ) )
                {
                    if( starts_with( *si, "schema:emblem:" ) )
                    {
                        link( schemas_emblems, e );
                    }
                    else
                    {
                        link( schemas, e );
                    }
                }
                else if( starts_with( *si, "branchfs-" ) )
                {
                    link( branchfs, e );
                }
                else if( starts_with( *si, "mtime-" ) || starts_with( *si, "ctime-" ) || starts_with( *si, "atime-" ))
                {
                    link( timerelated, e );
                }
                else if( starts_with( *si, "file-view-time" ) || starts_with( *si, "file-edit-time" ) )
                {
                    link( timerelated, e );
                }
                else if( starts_with( *si, "dontfollow-" ) )
                {
                    link( dontfollow, e );
                }
                else if( starts_with( *si, "fs-" ) )
                {
                    link( fs, e );
                }
                else if( starts_with( *si, "recursive-subcontext-" ) || starts_with( *si, "subcontext-" ) )
                {
                    link( subcontext, e );
                }
                else if( starts_with( *si, "rpm-" ) )
                {
                    link( rpm, e );
                }
                else
                {
                    link( ret, e );
                }
                e->getUniqueName();
                e->forceUpdateTransitiveChildOfEAOrderingRootEmblem();
                cerr << "created an ea-ordering for emblem...:" << *si
                     << " id:" << e->getID()
                     << " si:" << *si
                     << " Name:" << e->getName()
                     << " UName:" << e->getUniqueName()
                     << " isTC:" << e->isTransitiveChildOfEAOrderingRootEmblem()
                     << endl;
            }

            if( EAOrderingEmblemsCreatedCount )
                cerr << "EAOrderingEmblemsCreatedCount:" << EAOrderingEmblemsCreatedCount << endl;
        }
        
        
        LG_CTX_D << "Context::getAttributeRootEmblem(5 calling sync)" << endl;
        et->sync();
        LG_CTX_D << "Context::getAttributeRootEmblem(6 complete)" << endl;

        return ret;
    }
    


std::string
Context::priv_getRecommendedEA()
{
    return "name";
}



    
    string
    Context::getRecommendedEA()
        {
            fh_stringstream ret;

            ret << adjustRecommendedEAForDotFiles( this,
                getOverMountContext()->priv_getRecommendedEA());

//             cerr << "Context::getRecommendedEA()" << endl;
//             cerr << "Context::getRecommendedEA() omc:" << getOverMountContext()->priv_getRecommendedEA() << endl;
//             cerr << "Context::getRecommendedEA()  cc:" << getCoveredContext()->priv_getRecommendedEA() << endl;
//             cerr << "Context::getRecommendedEA() omc:" << toVoid(getOverMountContext()) << endl;
//             cerr << "Context::getRecommendedEA()  cc:" << toVoid(getCoveredContext()) << endl;
//             cerr << "Context::getRecommendedEA()this:" << toVoid(this) << endl;
            
//             set<string> unionset;
//             addEAToSet( unionset, getOverMountContext()->priv_getRecommendedEA() );
//             addEAToSet( unionset, getCoveredContext()->priv_getRecommendedEA() );
//             unique_copy( unionset.begin(), unionset.end(),
//                          ostream_iterator<string>(ret,","));
            

            const fh_context& thisp = this;
            for( int Pri = AttributeCreator::CREATE_PRI_MAX_INTERNAL_USE_ONLY; Pri >= 0; --Pri )
            {
//                 for( EAGenFactorys_t::const_iterator iter = getEAGenFactorys().begin();
//                      iter != getEAGenFactorys().end(); iter++ )
                ensureEAGenFactorysSetup();
                EAGenFactorys_iterator iterend = EAGenFactorys_iterator( this, true );
                for( EAGenFactorys_iterator iter = EAGenFactorys_iterator( this );
                     iter != iterend; ++iter )
                {
                    if( (*iter)->getCreatePriority() == Pri && (*iter)->hasInterest( thisp ) )
                    {
                        (*iter)->augmentRecommendedEA( thisp, ret );
                    }
                }
            }

            if( !starts_with(getURL(),"google:")
                && !starts_with(getURL(),"gdrive:"))
            {
                if( imageEAGenerator_haveLoader() )
                    ret << ",width,height";
            }
            

            return tostr( ret );
        }

// static guint medallion_timer = 0;
// const int medallion_timer_interval = 20000;

// static gint
// medallion_timer_f(gpointer data)
// {
//     typedef map< Context*, fh_medallion > cache_t;
//     cache_t* tptr = (cache_t*)data;
//     cache_t& cache = *tptr;

//     for( cache_t::iterator ci = cache.begin(); ci != cache.end(); )
//     {
//         if( ci->second->getReferenceCount() == 1 )
//         {
//             cache_t::iterator tmp = ci;
//             ++ci;
//             cache.erase( tmp );
//             continue;
//         }
//         ++ci;
//     }

//     return 0;
// }

static void
OnMedallionUpdated( std::string url, Cache< Context*, fh_medallion >* cache )
{
    LG_CTX_D << "OnMedallionUpdated() url:" << url << endl;
    
    typedef Cache< Context*, fh_medallion > cache_t;
    for( cache_t::iterator ci = cache->begin(); ci != cache->end(); ++ci )
    {
        Context* c = ci->first;
        if( c->getURL() == url )
        {
            ci->second->reload();
            c->Emit_Changed( 0, url, url, 0 );
            c->Emit_MedallionUpdated();
            break;
        }
    }
}

    bool
    Context::hasMedallion()
    {
        return isTrue( getStrAttr( this, "emblem:has-medallion", "0" ));
    }
    
    
fh_medallion
Context::getMedallion()
{
    static Cache< Context*, fh_medallion > cache;

    //
    // We are only interested in out-of-proc notifications for
    // medallions that this process is using.
    //
    static bool connected = false;
    if( !connected )
    {
        connected = true;
        Factory::getPluginOutOfProcNotificationEngine().
            getMedallionUpdated_Sig().connect(
                sigc::bind(
                    sigc::ptr_fun( OnMedallionUpdated ), &cache ) );
    }

    if( fh_medallion m = cache.get( this ) )
    {
        return m;
    }
    LG_CTX_D << "new Medallion() url:" << getURL() << endl;
    fh_medallion m = new Medallion( this );
    cache.put( this, m );
    return m;

    
//     typedef map< Context*, fh_medallion > cache_t;
//     static cache_t cache;

//     if( medallion_timer )
//     {
//         g_source_remove( medallion_timer );
//         medallion_timer = 0;
//     }
//     medallion_timer  = g_timeout_add( medallion_timer_interval,
//                                       GSourceFunc(medallion_timer_f), &cache );

//     cache_t::iterator ci = cache.find( this );
//     if( ci != cache.end() )
//     {
//         return ci->second;
//     }

//     fh_medallion m = new Medallion( this );
//     cache[ this ] = m;
//     return m;
}


stringlist_t
Context::getNamespacePrefixes()
{
    stringlist_t sl = _Base::getNamespacePrefixes();
    stringlist_t psl;
    
    if( isParentBound() )
        psl = getParent()->getNamespacePrefixes();

    copy( sl.begin(), sl.end(), back_inserter( psl ));
    return psl;
}

void
Context::readNamespaces()
{
    _Base::readNamespaces();
    if( isParentBound() )
        getParent()->readNamespaces();
    
}


std::string
Context::resolveFerrisXMLNamespace( const std::string& s )
{
    std::string r = _Base::resolveFerrisXMLNamespace( s );
    if( r != s )
        return r;

    if( isParentBound() )
        return getParent()->resolveFerrisXMLNamespace( s );
    return s;
}



    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/



    namespace Main
    {
        gboolean
        EventPending()
        {
            return Fampp::Fampp::Instance().Pending();
        }

        void
        processEvent()
        {
            if( Fampp::Fampp::Instance().Pending() )
                Fampp::Fampp::Instance().NextEvent();
        }

        static void ProcessFAMExceptions()
        {
            while( Fampp::haveMoreExceptions() )
            {
                std::exception e = Fampp::getNextException();
                string msg = e.what();
                if( msg.find("Failed to lstat") )
                {
                    LG_CTX_D << "Not propergating event due to it being most Likely"
                             << " caused by a quick action like date>|x && rm -f x"
                             <<"  Either that or you have a very bad kernel/disk :/ "
                             << " e:" << e.what() << endl;
                    continue;
                }
                
                LG_CTX_W << "ProcessFAMExceptions() rethrow! for e:" << e.what() << endl;
                throw e;

                /*
                 * Can't do any snazzy typeid() stuff because exception lacks vtable.
                 */
                
//                 Loki::TypeInfo einfo = typeid(e);
                
//                 NoSuchSubContext* nssc = 0;
//                 cerr << "TypeInfo e:" << Loki::TypeInfo(typeid(e)).name() << endl;
//                 cerr << "TypeInfo nssc:" << Loki::TypeInfo(typeid(NoSuchSubContext*)).name() << endl;
//                 if( einfo == Loki::TypeInfo(typeid(NoSuchSubContext*)) )
//                 {
//                     NoSuchSubContext* ne = static_cast<NoSuchSubContext*>(&e);
//                     cerr << "Not propergating event due to it being most Likely"
//                          << " caused by a quick action like date>|x && rm -f x"
//                          <<"  Either that or you have a very bad kernel/disk :/ "
//                          << " e:" << e.what() << endl;
//                     continue;
//                 }
                
//                 cerr << "ProcessFAMExceptions() rethrow! for e:" << e.what() << endl;
//                 throw Fampp::getNextException();
            }
        }
        
        
        void
        processAllPending_VFSFD_Events()
        {
            bool hadEvents = false;
            
            while( EventPending() )
            {
                hadEvents = true;
                processEvent();
            }
            
            if( hadEvents )
                ProcessFAMExceptions();
        }

        void processAllPendingEvents()
        {
            while( g_main_pending() )
            {
                g_main_iteration( false );
                processAllPending_VFSFD_Events();
            }
        }
        
        void mainLoop()
        {
            while( true )
            {
                g_main_iteration( true );
                processAllPending_VFSFD_Events();
//                ProcessFAMExceptions();
            }
            
            
//             GMainLoop *loop = g_main_loop_new(0,0);
//             g_main_loop_run( loop );
//             g_main_loop_unref( loop );
        }
    };

// #include <sockinet.h>
// #include <sockstream.h>
// #include <sockunix.h>


// IPStringAttribute::IPStringAttribute(
//     Attribute* parent,
//     const string& rdn,
//     const string& _ip )
//     :
//     StringAttribute( parent, rdn ),
//     ip(_ip)
// {
//     getReturnString_Sig().connect(slot( *this, &IPStringAttribute::str ));
// }

// IPStringAttribute::~IPStringAttribute()
// {
// }

// string
// IPStringAttribute::str()
// {
    
//     if( cache.find(ip) == cache.end() )
//     {
//         sockinetaddr sa(ip.c_str());
// //         cout << "Reverse lookup for:" << ip << endl;
// //         cout << "Reverse lookup is:" << sa.gethostname() << endl;

//         cache[ip] = sa.gethostname();
//     }

//     return cache[ip];
// }





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


// std::string setStrAttr( const char* path,
//                         const std::string& rdn,
//                         const std::string& v,
//                         bool create,
//                         bool throw_for_errors,
//                         bool dontDelegateToOvermountContext )
// {
//     try
//     {
//         fh_context c = Resolve( path );
//         setStrAttr( c, rdn, v, create, throw_for_errors, dontDelegateToOvermountContext );
//     }
//     catch( exception& e )
//     {
//         if( throw_for_errors )
//             throw;
//         return v;
//     }
// }
    
fh_context setChild( fh_context c,
                     const std::string& rdn,
                     const std::string& v )
{
    fh_context child = Shell::acquireSubContext( c, rdn );
    setStrAttr( child, "content", v );
    return child;
}
    
    

    /**
     * Set the attribute with the name of rdn on the context c to have value v.
     *
     * @returns v or throws an error
     */
std::string setStrAttr( fh_context c,
                        const std::string& rdn_ref,
                        const std::string& v,
                        bool create,
                        bool throw_for_errors,
                        bool dontDelegateToOvermountContext ) 
    {
        string rdn = c->expandEAName( rdn_ref, false );
//        cerr << "setStrAttr() rdn:" << rdn << endl;
        
        try
        {
            fh_iostream ss;
            
            if( !rdn.length() || rdn == "." || rdn == "content" )
            {
                ss = c->getIOStream( ios::out | ios::trunc );
            }
            else
            {
                if( !c->isAttributeBound( rdn ) )
                {
                    if( create )
                    {
                        LG_CTX_D << "told to create rdn:" << rdn << " and dont see it at the moment" << endl;
                        LG_CTX_D << " c:" << c->getURL() << endl;
//                         cerr << " c:" << c->getURL()
//                              << " c addr:" << (void*)GetImpl(c)
//                              << endl;
//                        c->dumpAttributeNames();
                        Shell::createEA( c, rdn, v, dontDelegateToOvermountContext );
                        return v;
                    }
                    else
                    {
                        fh_stringstream ss;
                        ss << "Can not update information in attribute:" << rdn
                           << " of context:" << c->getURL()
                           << " because there is no such attribute";
                        Throw_NoSuchAttribute( tostr(ss), GetImpl(c) );
                    }
                }
                
                fh_attribute attr;
                attr = c->getAttribute( rdn );
                ss = attr->getIOStream( ios::out | ios::trunc );
            }

//            cerr << "PUTTING value:" << v << endl;
            ss << v << flush;
            if( !ss )
            {
                LG_CTX_D << "Can not set c:" << c->getURL()
                         << " rdn:" << rdn
                         << " v:" << v
                         << " stream state:" << ss->rdstate()
                         << endl;
            }

            if( rdn_ref == "annotation" )
            {
#ifdef HAVE_DBUS
                string earl = c->getURL();
                LG_DBUS_D << "Emitting AnnotationSaved for earl:" << earl << endl;
                DBusConnection* conn = DBus::getSessionBus();
                DBus::Signal sig( "/", "org.libferris.desktop", "AnnotationSaved");
                sig.push_back( earl );
                sig.push_back( c->getDirPath() );
                sig.send( conn );
                LG_DBUS_D << "Done Emitting AnnotationSaved for earl:" << earl << endl;
#endif
            }

            
        }
        catch( exception& e )
        {
            LG_CTX_D << "Can not set c:" << c->getURL()
                     << " rdn:" << rdn
                     << " v:" << v
                     << " e:" << e.what()
                     << endl;
            if( throw_for_errors )
                throw;
            return v;
        }
//         catch(...)
//         {
//         }
        LG_ATTR_D << "setStrAttr() rdn:" << rdn << " v:" << v << endl;
        return v;
    }
    
    
std::string
    Context::private_getStrAttr( const std::string& rdn_ref,
                                 const std::string& def,
                                 bool getAllLines,
                                 bool throwEx )
    {
        string rdn = expandEAName( rdn_ref, false );

//        cerr << "private_getStrAttr() rdn:" << rdn << " path:" << getDirPath() << endl;
        
        
        EA_Atom* atom = getAttributeIfExists( rdn );
        
//        cerr << "Context::getStrAttr(1) url:" << getURL() << " rdn:" << rdn << endl;

        if( !atom && !AttributesHaveBeenCreated )
        {
            LG_OVERMOUNT_I << "private_getStrAttr() rdn:" << rdn << " not-currently-bound " << endl
                           << " calling ensureAttributesAreCreated(rdn) for url:" << getURL() << endl;
            ensureAttributesAreCreated( rdn );
            atom = getAttributeIfExists( rdn );
        }

        
        // FIXME: we need implicit attribute discovery here, but this is a loop
        if( !atom && getCoveredContext() == this )
        {
            static bool attemptingToAcquireAttributesViaOverMount = false;
            if( !attemptingToAcquireAttributesViaOverMount )
            {
                Util::ValueRestorer< bool > x( attemptingToAcquireAttributesViaOverMount, true );
                tryToFindAttributeByOverMounting( rdn );
            }
        }
        

        if( isBound(OverMountContext_Delegate) && rdn != "recommended-ea" )
        {
//            cerr << "Context::getStrAttr(dele) url:" << getURL() << " rdn:" << rdn << endl;
            return OverMountContext_Delegate->private_getStrAttr( rdn, def, getAllLines, throwEx );
        }

        try
        {
            string s = def;
            
            if( rdn == "recommended-ea" )
            {
                rdn = "recommended-ea-union";
            }


            if( !rdn.length() || rdn=="content" || rdn=="." )
            {
                fh_istream ss = getIStream();
                if( getAllLines )
                    s = StreamToString( ss );
                else
                    getline( ss, s );
            }
            else
            {
                if( !atom )
                    atom = getAttributePtr( rdn );

                // This method of getting the data reuses a static string stream
                // buffer to avoid the noticable expense of creating and returning
                // a stringstream. The locale setup that has to be done each time
                // a stringstream is made/deleted can dominate when sorting by EA
                // for example.
                if( atom && atom->havePassedInSteamRead() )
                {
                    const int ssarray_maxlevels = 1024;
                    static fh_stringstream ssarray[ ssarray_maxlevels ];
                    static int ssarray_index = 0;
                    
//                    LG_EAIDX_D << "Context::private_getStrAttr(X) ss.addr:" << &ssarray[ ssarray_index ] << endl;
//                    LG_EAIDX_D << "Context::private_getStrAttr(X) rdn:" << rdn << endl;
//                    fh_stringstream ss;
//                    static fh_stringstream ss;
                    fh_stringstream& ss = ssarray[ ssarray_index ];
                    Util::ValueRestorer< int > x( ssarray_index );
                    ++ssarray_index;
//                    LG_EAIDX_D << "ssarray_index:" << ssarray_index << endl;
//                    cerr << "ssarray_index:" << ssarray_index << endl;
//                    BackTrace();

                    typedef Loki::SmartPtr< fh_stringstream,
                        Loki::RefLinked, 
                        Loki::AllowConversion,
                        FerrisLoki::FerrisExSmartPointerChecker,
                        FerrisLoki::FerrisExSmartPtrStorage > fh_ss_ptr;
                    fh_ss_ptr fh_ss_ref = 0;
                    
                    if( ssarray_index >= ssarray_maxlevels )
                    {
                        LG_EAIDX_W << "Context::private_getStrAttr() max recursive depth hit!" << endl;
                        cerr << "Context::private_getStrAttr() max recursive depth hit!" << endl;
                        fh_ss_ref = new fh_stringstream();
                        ss = *(GetImpl(fh_ss_ref));
                    }
                    
                    
                    ss.clear();
                    string ssdata;
                    ss.str(ssdata);
                    ss << dec;
                    ss.seekp( 0 );
                    EA_Atom_ReadOnly_PassedInStream* astream = static_cast<EA_Atom_ReadOnly_PassedInStream*>( atom );
                    ferris_ios::openmode m = ios::in;
                    astream->getIStream( this, rdn, m, ss );
                    ss.seekg( 0 );
//                    cerr << "mtimeN ss:" << tostr(ss) << endl;
                    if( getAllLines )
                    {
                        s = StreamToString( ss );
                    }
                    else
                    {
                        getline( ss, s );
                    }
//                    LG_EAIDX_D << "Context::private_getStrAttr(XRET) rdn:" << rdn << endl;
//                    LG_EAIDX_D << "Context::private_getStrAttr() s:" << s << endl;
                }
                else
                {
                    fh_istream ss;
                    if( atom )
                    {
                        ferris_ios::openmode m = ios::in;
                        ss = atom->getIStream( this, rdn, m );
                    }

                    if( getAllLines )
                        s = StreamToString( ss );
                    else
                        getline( ss, s );
                }
            }

            

            
//             if( rdn == "recommended-ea" && isBound(OverMountContext_Delegate) )
//             {
//                 /* Merge recommended EA for base and overmounting contexts */

//                 string oms = OverMountContext_Delegate->private_getStrAttr(
//                     rdn, def, getAllLines, throwEx );

//                 cerr << "Context::getStrAttr(merge) s:" << s
//                      << " oms:" << oms
//                      << endl;
                
                
//                 set<string> unionset;
//                 string tmp;
//                 {
//                     fh_stringstream ss;
//                     ss << s;
//                     while(getline(ss,tmp,','))
//                         unionset.insert(tmp);
//                 }
//                 {
//                     fh_stringstream ss;
//                     ss << oms;
//                     while(getline(ss,tmp,','))
//                         unionset.insert(tmp);
//                 }
//                 fh_stringstream ss;
//                 unique_copy( unionset.begin(), unionset.end(),
//                              ostream_iterator<string>(ss,","));
//                 s = tostr(ss);

//                 cerr << "Context::getStrAttr(merge done) s:" << s
//                      << endl;
//             }
            
            return s;
        }
        catch( exception& e )
        {
            // handled in getAttributePtr() for this method too
//             if( starts_with( rdn, "schema:" ) && isParentBound() )
//             {
//                 return getParent()->private_getStrAttr( "subtree" + rdn,
//                                                         def, getAllLines, throwEx );
//             }
            if( throwEx )
            {
                LG_ATTR_D << "Context::private_getStrAttr(error) url:" << getURL()
                          << " rdn:" << rdn_ref
                          << " def:" << def
                          << " e:" << e.what()
                          << endl;
                throw;
            }
        }
        catch(...)
        {
            if( throwEx )
                throw;
        }
        return def;
        
    }
    

    std::string getStrAttr( std::string earl,
                            const std::string& rdn,
                            const std::string& def,
                            bool getAllLines,
                            bool throw_for_errors )
    {
        try
        {
            fh_context c = Resolve( earl );
            return getStrAttr( c, rdn, def, getAllLines, throw_for_errors );
        }
        catch( exception& e )
        {
            if( throw_for_errors )
                throw;
            return def;
        }
    }
    

    
    std::string getStrAttr( const fh_context& c,
                            const std::string& rdn,
                            const std::string& def,
                            bool getAllLines,
                            bool throw_for_errors ) 
    {
        return getStrAttr( GetImpl(c), rdn, def, getAllLines, throw_for_errors );
    }

    /**
     * Read an attribute from a context and return either its contents or a default
     * value.
     *
     * @param c Context to get attribute from
     * @param rdn attribute name
     * @param default vaule if there is no attribute with the given rdn, or an
     *        error occurs.
     */
    std::string getStrAttr( AttributeCollection* c,
                            const std::string& rdn,
                            const std::string& def,
                            bool getAllLines,
                            bool throw_for_errors ) 
    {
        if( !c )
        {
            cerr << "getStrAttr() called without a context" << endl;
            BackTrace();
            if( throw_for_errors )
            {
                fh_stringstream ss;
                ss << "getStrAttr() called without a context";
                Throw_CanNotGetStream( tostr(ss), 0 );
            }
            return def;
        }
        return ((Context*)c)->private_getStrAttr( rdn, def, getAllLines, throw_for_errors );
        
//         try
//         {
//             fh_istream ss;
//             string s = def;
            
//             if( !rdn.length() )
//             {
//                 ss = (static_cast<Context*>(c))->getIStream();
//             }
//             else
//             {
//                 if( !c->isAttributeBound( rdn ) )
//                 {
//                     return def;
//                 }
//                 ss = c->getAttribute( rdn )->getIStream();
//             }

//             if( getAllLines )
//             {
//                 fh_stringstream tmpss;
//                 copy( istreambuf_iterator<char>(ss),
//                       istreambuf_iterator<char>(),
//                       ostreambuf_iterator<char>(tmpss));
//                 s = tostr( tmpss );
//             }
//             else
//             {
//                 getline( ss, s );
//             }
//             return s;
//         }
//         catch(...)
//         {
//         }
//         return def;
    }

    FERRISEXP_API time_t getTimeAttr( fh_context c,
                                      const std::string& rdn,
                                      time_t v,
                                      bool throw_for_errors )
    {
        time_t mt = toType<time_t>( getStrAttr( c, rdn, tostr(v), false, throw_for_errors ));
        return mt;
    }
    
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    
    void ParseOnly_FERRIS_POPT_OPTIONS( const string& PROGRAM_NAME,
                                        int argc,
                                        const char** argv )
    {
        poptContext optCon;
        int ch=-1;
        struct poptOption optionsTable[] = {
            FERRIS_POPT_OPTIONS
            POPT_TABLEEND
        };

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* ...");
    
        /***/
        while ((ch = poptGetNextOpt(optCon)) >= 0)
        {}

        poptFreeContext(optCon);
    }

bool isTrue( const string& s )
{
    return s=="1" || s=="yes" || s=="true" || s=="1\n" || s=="yes\n" || s=="true\n";
}

    bool isFalse( const std::string& s )
    {
        return !isTrue( s );
    }
    


bool isNumber( const string& s )
{
    for( int i=0; i<s.length(); ++i )
    {
        if( !isdigit( s[i] ) )
            return false;
    }
    return true;
}

static const char* COMPRESSION_SENTINAL = "is-ferris-compressed";
bool
Context::isCompressedContext()
{
    if( isBound( CoveredContext ) )
        return CoveredContext->isCompressedContext();

    LG_CTX_D << "isCompressedContext() this:" << this << endl;
    LG_CTX_D << "isCompressedContext() cc  :" << getCoveredContext() << endl;
    LG_CTX_D << "isCompressedContext() ret:" << isTrue( getStrAttr( this, COMPRESSION_SENTINAL, "0" )) << endl;
    return isTrue( getStrAttr( this, COMPRESSION_SENTINAL, "0" ));
}

    // redlandea::fh_SmushSet
    // Context::tryToGetImplicitTreeSmush()
    // {
    //     using namespace redlandea;
        
    //     if( m_tryToGetImplicitTreeSmushHasFailed_forDirectory )
    //         return 0;

    //     string earl = getURL();
    //     earl += "/";
    //     fh_TreeSmushing ts = getDefaultImplicitTreeSmushing();
    //     const TreeSmushing::m_smushSets_t& a = ts->getAll();
    //     TreeSmushing::m_smushSets_t::const_iterator ai = a.begin();
    //     TreeSmushing::m_smushSets_t::const_iterator ae = a.end();

    //     LG_RDF_D << "Context::tryToGetImplicitTreeSmush() earl:" << earl << endl;
        
    //     for( ; ai != ae ; ++ai )
    //     {
    //         fh_SmushSet ret = ai->second;
    //         const SmushSet::m_smushes_t& sm = ret->getSmushes();
    //         SmushSet::m_smushes_t::const_iterator si = sm.begin();
    //         SmushSet::m_smushes_t::const_iterator se = sm.end();

    //         for( ; si != se; ++si )
    //         {
    //             const string& n = si->first;
    //             fh_regex r = si->second;

    //             LG_RDF_D << "Context::tryToGetImplicitTreeSmush() n:" << n << endl;

    //             if( (*r)( earl ) )
    //             {
    //                 return ret;
    //             }
    //         }
    //     }

    //     m_tryToGetImplicitTreeSmushHasFailed_forDirectory = true;
    //     return 0;
    // }




    Context::EnsureStartStopReadingIsFiredRAII::EnsureStartStopReadingIsFiredRAII( Context* c )
        :
        m_c( c )
    {
        m_c->EnsureStartReadingIsFired();
    }
    Context::EnsureStartStopReadingIsFiredRAII::~EnsureStartStopReadingIsFiredRAII()
    {
        m_c->EnsureStopReadingIsFired();
        m_c->updateMetaData();
    }

    Context::emitExistsEventForEachItemRAII::emitExistsEventForEachItemRAII( Context* c )
        :
        EnsureStartStopReadingIsFiredRAII( c )
    {
    }
    
    Context::emitExistsEventForEachItemRAII::~emitExistsEventForEachItemRAII()
    {
        m_c->emitExistsEventForEachItem();
    }
    
    Context::staticDirContentsRAII::staticDirContentsRAII( Context* c )
        :
        EnsureStartStopReadingIsFiredRAII( c )
    {
        if( !m_c->empty() )
            m_c->emitExistsEventForEachItem();
    }
    
    Context::staticDirContentsRAII::~staticDirContentsRAII()
    {
    }

    void
    Context::priv_ensureSubContext_helper_ins( fh_context c, bool created )
    {
//        cerr << "priv_ensureSubContext_helper_ins() created:" << created << " c:" << c->getURL() << endl;
        fh_context ret = Insert( GetImpl(c), created );
        bumpVersion();
    }


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

fh_context ExecuteQueryAgainstMyRDF( const std::string& sparql )
{
    fh_model m = getDefaultFerrisModel();
    BindingsIterator iter = m->findBindings( sparql );
    BindingsIterator e;
    stringset_t urls;
    for( ; iter != e ; ++iter )
    {
        fh_node n = iter[ "earl" ];
        if( n )
        {
            urls.insert( n->getURI()->toString() );
        }
    }

    fh_context selfactory = Resolve( "selectionfactory://" );
    fh_context selection  = selfactory->createSubContext( "" );
            
    for( stringset_t::iterator iter = urls.begin(); iter != urls.end(); ++iter )
    {
        try
        {
            string earl = *iter;
            fh_context  ctx  = Resolve( earl );
            selection->createSubContext( "", ctx );
        }
        catch( exception& e )
        {
            cerr << "Warning, e:" << e.what() << endl;
        }
    }

    return selection;
}
    
    

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//     ExecAttribute::ExecAttribute(
//         Attribute* parent,
//         const std::string& rdn,
//         const fh_runner& r )
//         :
//         StreamingAttribute( parent, rdn ),
//         Runner( r )
//     {
//         RunnerStaticlyBound = isBound( Runner );
//     }


//     fh_istream
//     ExecAttribute::priv_getIStream( ferris_ios::openmode m )
//         throw (FerrisParentNotSetError,
//                CanNotGetStream,
//                exception)
//     {
//         if( !RunnerStaticlyBound )
//         {
//             Runner = getRunner();
//         }
        
//         if( Runner->Run() )
//         {
//             /* ok */
//             fh_istream filepipe = Runner->getStdOut();
//             return filepipe;
//         }
//         else
//         {
//             /* fail */
//         }
    
//         stringstream ss;
//         ss << "ExecAttribute::priv_getIStream() path:" << getDirPath()
//            << " exec/fork was a failure."
//            << " ERR: " << Runner->getErrorString()
//            << endl;
//         Throw_CanNotGetStream(tostr(ss),this);
//     }


//     fh_runner
//     ExecAttribute::getRunner()
//     {
//         return Runner;
//     }



    std::string getMimeName( fh_context c )
    {
        string ret = getStrAttr( c, "mimetype", "" );
//        cerr << "getMimeName() c:" << c->getURL() << " ret:" << ret << endl;
        if( ret.find(";"))
        {
            ret = ret.substr( 0, ret.find(";") );
        }
        if( ret.find(" "))
        {
            ret = ret.substr( 0, ret.find(" ") );
        }

        

	return ret;
    }


void FerrisInternal::reparentSelectionContext( Context* parent, fh_context NewChildToAdopt, const std::string& rdn )
{
    NewChildToAdopt->setContext( parent, rdn );
}


    namespace ImplementationDetail
    {
        stringset_t& getStaticLinkedRootContextNames()
        {
            static stringset_t ret;
            return ret;
        }
        
        bool appendToStaticLinkedRootContextNames( const std::string& s )
        {
            getStaticLinkedRootContextNames().insert( s );
            return true;
        }
        
        const ::Ferris::Handlable::ref_count_t MAX_REF_COUNT = 65530;
    };

    namespace ImplementationDetail
    {
        std::string getCURLProxyCommandLineOption()
        {
            string proxyname = getConfigString( FDB_GENERAL, "curl-use-proxy-name", "" );
            string proxyport = getConfigString( FDB_GENERAL, "curl-use-proxy-port", "" );
            stringstream ret;
            if( !proxyname.empty() )
            {
                ret << "--proxy " << proxyname;
                if( !proxyport.empty() )
                    ret << ":" << proxyport;
            }
            return tostr( ret );
        }
    };


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    std::string getIOErrorDescription( fh_istream& ss )
    {
        return getIOErrorDescription( ss, "" );
    }
    
    std::string getIOErrorDescription( fh_ostream& ss )
    {
        return getIOErrorDescription( ss, "" );
    }
    
    std::string getIOErrorDescription( fh_iostream& ss )
    {
        return getIOErrorDescription( ss, "" );
    }

    std::string getIOErrorDescription( fh_istream& ss, fh_context c )
    {
        return getIOErrorDescription( ss, c->getURL() );
    }
    
    std::string getIOErrorDescription( fh_ostream& ss, fh_context c )
    {
        return getIOErrorDescription( ss, c->getURL() );
    }
    
    std::string getIOErrorDescription( fh_iostream& ss, fh_context c )
    {
        return getIOErrorDescription( ss, c->getURL() );
    }

    guint64 getIOErrorDescription_getOffset( fh_istream& ss )
    {
//        ss.clear();
        guint64 offset = ss.tellg();
        return offset;
    }
    guint64 getIOErrorDescription_getOffset( fh_ostream& ss )
    {
        guint64 offset = ss.tellp();
        return offset;
    }
    guint64 getIOErrorDescription_getOffset( fh_iostream& ss )
    {
        return (guint64)(-1);
    }
    
    
    template< class T >
    std::string getIOErrorDescription_priv( T& ss,
                                            const std::string& earl,
                                            const std::string& opcode = "" )
    {
        stringstream zz;
        zz << "I/O";
        if( !opcode.empty() )
        {
            zz << " " << opcode;
        }
        zz << " ERROR";

        guint64 offset = getIOErrorDescription_getOffset( ss );
//        cerr << "offset:" << offset << endl;
//         if( offset != (guint64)(-1) )
//         {
//             zz << " at:" << offset;
//         }
        if( !earl.empty() )
            zz << " with url:" << earl;
        zz << " reason";
        return errnum_to_string( zz.str(), errno );
    }
    
    std::string getIOErrorDescription( fh_istream& ss, const std::string& earl )
    {
        if( haveIOError( ss ) )
        {
            return getIOErrorDescription_priv( ss, earl, "READ" );
        }
        return "";
    }
    
    std::string getIOErrorDescription( fh_ostream& ss, const std::string& earl )
    {
        if( haveIOError( ss ) )
        {
            return getIOErrorDescription_priv( ss, earl, "WRITE" );
        }
        return "";
    }
    
    std::string getIOErrorDescription( fh_iostream& ss, const std::string& earl )
    {
        if( haveIOError( ss ) )
        {
            return getIOErrorDescription_priv( ss, earl );
        }
        return "";
    }
    

    
    bool haveIOError( fh_istream&  ss )
    {
        std::basic_streambuf< fh_istream::char_type >* rdbuf = ss.rdbuf();
        typedef ferris_stringbuf< fh_istream::char_type > frdbuf_t;
        frdbuf_t* frdbuf = dynamic_cast< frdbuf_t* >( rdbuf );
        bool isStringStream = ( frdbuf != 0 );
//         cerr << "haveIOError()"
//              << " eof:" << ss.eof()
//              << " good:" << ss.good()
//              << " is-stringstream:" << isStringStream
//              << endl;
        if( isStringStream )
            return ( !ss.good() );
        return ( !ss.eof() || !ss.good() );
    }
    
    bool haveIOError( fh_ostream&  ss )
    {
        return ( !ss.good() );
    }
    
    bool haveIOError( fh_iostream&  ss )
    {
        return ( !ss.good() );
    }
    
    
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


