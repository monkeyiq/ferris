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

    $Id: Resolver.cpp,v 1.28 2011/10/22 21:30:21 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Resolver.hh>
#include <Resolver_private.hh>
#include <SM.hh>
#include <MatchedEAGenerators.hh>
#include <Ferris.hh>
#include <FerrisBoost.hh>
#include <General.hh>
#include <Trimming.hh>
#include <ContextSetCompare_private.hh>

#include <sys/types.h>
#include <dirent.h>

// context VM debug
#include <Ferris_private.hh>

#include <fnmatch.h>

#include "config.h"

using namespace std;

#define CERR cerr

namespace Ferris
{

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

/**
 * A Context dropper that loads its context from a GModule.
 */
    class FERRISEXP_DLLLOCAL gmodule_RootContextDropper
        :
        public RootContextDropper
    {
        string     ModuleName;
        GModule*   ghandle;
        GModule*   ghandle_factory;

        bool m_tryToOverMountToFindEA;

        fh_ommatchers (*module_GetOverMountMatchers)(RootContextFactory* rf);
        string        (*module_getName)();
        bool          (*module_isTopLevel)();
        fh_context    (*module_Brew)( RootContextFactory* rf );
    


        void initFactoryModule()
            {
//                cerr << "opening ModuleName:" << ModuleName << endl;
                ghandle_factory = g_module_open ( ModuleName.c_str(), G_MODULE_BIND_LAZY);
            
                if (!ghandle_factory)
                {
                    ostringstream ss;
                    ss  << "Error, unable to open module file:" << ModuleName << " "
                        << g_module_error ()
                        << endl;
                    cerr << tostr(ss);
                    Throw_GModuleOpenFailed( tostr(ss), 0 );
                }

                if (!g_module_symbol (ghandle_factory, "getName", 
                                      (gpointer*)&module_getName))
                {
                    ostringstream ss;
                    ss  << "Error, unable to resolve getName in module file:" << ModuleName
                        << " "
                        << g_module_error()
                        << endl;
                    cerr << tostr(ss);
                    Throw_GModuleOpenFailed( tostr(ss), 0 );
                }

                if (!g_module_symbol (ghandle_factory, "GetOverMountMatchers", 
                                      (gpointer*)&module_GetOverMountMatchers))
                {
                    ostringstream ss;
                    ss  << "Error, unable to resolve GetOverMountMatchers in module file:"
                        << ModuleName << " "
                        << g_module_error()
                        << endl;
                    cerr << tostr(ss) << endl;
                    Throw_GModuleOpenFailed( tostr(ss), 0 );
                }

                module_isTopLevel = 0;
                g_module_symbol (ghandle_factory, "isTopLevel", (gpointer*)&module_isTopLevel);

                m_tryToOverMountToFindEA = 0;
                bool (*module_tryToOverMountToFindEA)() = 0;
                if( g_module_symbol (ghandle_factory, "tryToOverMountToFindEA",
                                     (gpointer*)&module_tryToOverMountToFindEA) )
                {
                    m_tryToOverMountToFindEA = module_tryToOverMountToFindEA();
                }
            }

        void ensureModuleLoaded()
            {
                if( !ghandle_factory )
                {
                    initFactoryModule();
                }
            }

    
        void ensureImplementationModuleLoaded()
            {
                if( !ghandle )
                {
                    string implname = ModuleName;
                    string ending   = "_factory.so";

                    implname.replace( implname.find(ending), ending.length(), ".so" );
                    LG_PLUGIN_I << "Linking in implementaion of:" << implname << endl;

                    ghandle = g_module_open ( implname.c_str(), G_MODULE_BIND_LAZY);
                    if (!ghandle)
                    {
                        ostringstream ss;
                        ss  << "Error, unable to open module file:" << implname
                            << " module error:" << g_module_error ()
                            << endl;
                        LG_PLUGIN_I << tostr(ss) << endl;
                        Throw_GModuleOpenFailed( tostr(ss), 0 );
                    }

                    if (!g_module_symbol (ghandle, "Brew", 
                                          (gpointer*)&module_Brew))
                    {
                        ostringstream ss;
                        ss  << "Error, unable to resolve Brew in module file, %s"
                            << g_module_error()
                            << endl;
                        LG_PLUGIN_I << tostr(ss) << endl;
                        Throw_GModuleOpenFailed( tostr(ss), 0 );
                    }
                }
            }
    
    
    public:
    
        gmodule_RootContextDropper( string _ModuleName )
            :
            ModuleName( _ModuleName ),
            ghandle_factory(0),
            ghandle(0)
            {
//            RootContextFactory::Register("edb", this);
            }

        virtual fh_ommatchers GetOverMountMatchers(RootContextFactory* rf)
            {
                ensureModuleLoaded();
                return module_GetOverMountMatchers(rf);
            }

        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
            {
                try
                {
                    LG_PLUGIN_I << "About to brew a context for :" << getName() << endl;
//                    LG_PLUGIN_I << "root:" << rf->getInfo( RootContextFactory::ROOT ) << endl;
                    
                    ensureImplementationModuleLoaded();
                    return module_Brew(rf);
                }
                catch( RootContextCreationFailed& e )
                {
                    LG_PLUGIN_I << "Cought and rethrow :" << e.what() << endl;
                    throw;
                }
                catch( exception& e )
                {
                    LG_PLUGIN_I << "Cought :" << e.what() << endl;

                    stringstream ss;
                    ss << "gmodule_RootContextDropper::Brew() cought e:" << e.what() << endl;
                    Throw_RootContextCreationFailed( tostr(ss), 0 );
                }
            }

        virtual bool isTopLevel()
            {
                ensureModuleLoaded();
                if( module_isTopLevel )
                    return module_isTopLevel();
                return RootContextDropper::isTopLevel();
            }

        virtual bool tryToOverMountToFindEA()
            {
                return m_tryToOverMountToFindEA;
            }
        
        string getName()
            {
                ensureModuleLoaded();
                return module_getName();
            }
    
    };
    

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

/**
 * Prepared for root resolution on the native filesystem by default.
 */
    RootContextFactory::RootContextFactory()
        :
        Style(RESOLVE_EXACT),
        ContextClass("file")
    {
        AddInfo( ROOT, "/" );
    }

    RootContextFactory::~RootContextFactory()
    {
    }

    RootContextFactory::RootContextFactory( const string& module,
                                            const string& root,
                                            const string& path,
                                            ResolveStyle s )
        :
        Style(s),
        ContextClass( module )
    {
        AddInfo( ROOT, root );
        AddInfo( PATH, path );
        LG_FACTORY_D << "RootContextFactory::RootContextFactory() Style:" << Style << endl;
    }






/**
 * Get the info bound by key.
 *
 * @param key The key to lookup
 * @return data bound to key
 */
    string
    RootContextFactory::getInfo( string key )
    {
        return BindData[ key ];
    }


/**
 * Set the name of the context class to resolve to.
 *
 * @param cl Class name, for example "Native"
 */
    void
    RootContextFactory::setContextClass( string cl )
        throw( NoSuchContextClass )
    {
        ContextClass = cl;
        if( getDroppers().end() == getDroppers().find( ContextClass ) )
        {
            ostringstream ss;
            ss << "setContextClass() NoSuchContextClass:" << cl;
            Throw_NoSuchContextClass( tostr(ss), 0 );
        }
    }

    std::string
    RootContextFactory::getContextClass()
    {
        return ContextClass;
    }
    
        


/**
 * Bind the given value to the key
 *
 * @see PATH
 * @see ROOT
 * 
 * @param key Key to bind data to
 * @param val Val to bind to key
 */
    void
    RootContextFactory::AddInfo( string key, string val )
    {
        BindData[ key ] = val;
    }


/**
 * get the cache of root context objects keyed on the name of the context class.
 */
    RootContextFactory::RootContextCache_t&
    RootContextFactory::getRootContextCache()
    {
        static RootContextCache_t RootContextCache;
        return RootContextCache;
    }


/**
 * Get the root context for this resolution.
 */
    fh_context
    RootContextFactory::getRootContext()
        throw( RootContextCreationFailed )    
    {
        string ContextClassName = ContextClass;
        string FilesystemArgs = "";
        
        LG_FACTORY_D << "RootContextFactory::getRootContext() " << endl;
        LG_FACTORY_D << "RootContextFactory::getRootContext() cc:" << ContextClassName << endl;

        if( starts_with( ContextClassName, "xsltfs?" ) )
        {
//            cerr << "ContextClass:" << ContextClass << endl;
            int p = ContextClassName.find( '?' );
            if( p != string::npos )
                FilesystemArgs = ContextClassName.substr( p+1 );
            ContextClassName = "xsltfs";
        }

        bool isStaticDOM = false;
        if( ContextClassName == "xml" )
        {
            if( !getInfo( "StaticString" ).empty() || !getInfo( "StaticDOM" ).empty() )
                isStaticDOM = true;
        }
        
        if( !isStaticDOM &&
            FilesystemArgs.empty()
            && isBound(getRootContextCache()[ ContextClassName ]) )
        {
            LG_FACTORY_D << "RootContextFactory::getRootContext() GET cache ContextClassName:"
                         << ContextClassName << " obj addr :"
                         << toVoid(getRootContextCache()[ ContextClassName ]) << endl;
            LG_FACTORY_D << "RootContextFactory::getRootContext() "
                         << " root path:"
                         << getRootContextCache()[ ContextClassName ]->getDirPath()
                         << endl;

            return getRootContextCache()[ ContextClassName ];
        }

        LG_FACTORY_D << ">>>>>>>>>> Brew(enter) for ContextClassName:" << ContextClassName
                     << " FilesystemArgs:" << FilesystemArgs
                     << endl;
        if( getDroppers().end() == getDroppers().find( ContextClassName ) )
        {
            fh_stringstream ss;
            ss << "Can not create the root context for context class:"
               << ContextClassName << " because there is no RootContextDropper"
               << " for a class with name:" << ContextClassName;
//             cerr << tostr(ss);
//             BackTrace();
            Throw_RootContextCreationFailed( tostr(ss), 0 );
        }
        RootContextDropper* dropper = getDroppers()[ ContextClassName ];
//        cerr << "RootContextFactory::getRootContext() staticstring:" << getInfo( "StaticString" ) << endl;
        fh_context obj = dropper->Brew( this );
        LG_FACTORY_D << ">>>>>>>>>> Brew( call done. ) for ContextClassName :" << ContextClassName << endl;

        if( !FilesystemArgs.empty() )
            setStrAttr( obj, "filesystem-args", FilesystemArgs );
        
        LG_FACTORY_D << "RootContextFactory::getRootContext() setting cache ContextClassName:"
                     << ContextClassName << " obj addr :" << toVoid(obj) << endl;

//     cerr << "RootContextFactory::getRootContext() setting cache ContextClassName:"
//          << ContextClassName << " obj addr :" << toVoid(obj) << endl;
//     cerr << "RootContextFactory::getRootContext() setting cache path: "
//          << obj->getDirPath() << "   name:" << obj->getDirName() << endl;
//     sleep(5);

        /*
         * FIXME: This is a messy solution to the "mount a string as a ffilter fs"
         */
        if( !isStaticDOM && FilesystemArgs.empty()
            && ContextClassName != "ffilter"
            && ContextClassName != "ffilter2"
            && ContextClassName != "ffilter.pccts"
            && ContextClassName != "ffilter.spirit"
            && ContextClassName != "fulltextboolean"
            && ContextClassName != "fulltextboolean2"
            && ContextClassName != "fulltextboolean.pccts"
            && ContextClassName != "fulltextboolean.spirit"
            )
        {
//             cerr << "Adding to RootContextCache ContextClassName:" << ContextClassName
//                  << " ctx:" << GetImpl(obj)
//                  <<  " ref_count:" << obj->ref_count
//                  << endl;

            obj->ref_count = ImplementationDetail::MAX_REF_COUNT;

//             cerr << "Adding2 to RootContextCache ContextClassName:" << ContextClassName
//                  << " ctx:" << GetImpl(obj)
//                  << " rdn:" << obj->getDirName()
//                  <<  " ref_count:" << obj->ref_count
//                  << endl;
            getRootContextCache()[ ContextClassName ] = obj;
        }
        return obj;
    }

    typedef multimap< Context*, string > AdditionalRootContextClassNames_t;
    AdditionalRootContextClassNames_t&
    getAdditionalRootContextClassNames()
    {
        static AdditionalRootContextClassNames_t ret;
        return ret;
    }
    
    void
    RootContextFactory::pushAdditionalRootContextClassName( string n, Context* c )
    {
        getAdditionalRootContextClassNames().insert( make_pair( c, n ) );
    }
    

/**
 * Given the root context of a tree find out the name of the context
 * dropper that will create the same root object
 *
 * @param c The root of a context tree
 */
    const std::string
    RootContextFactory::getRootContextClassName( const Context* c )
    {
        typedef RootContextCache_t::iterator RCCI;

        RootContextCache_t& cache = getRootContextCache();

        for( RCCI iter = cache.begin(); iter != cache.end(); ++iter )
        {
            if( GetImpl(iter->second) == c )
            {
                LG_FACTORY_D << "getRootContextClassName() looked up root:" << iter->first << endl;
                return iter->first;
            }
        }

//        cerr << "getRootContextClassName() c:" << toVoid(c) << endl;

        AdditionalRootContextClassNames_t::iterator iter = getAdditionalRootContextClassNames().find( (Context*)c );
        if( iter != getAdditionalRootContextClassNames().end() )
        {
            return iter->second;
        }
        
//         {
//             fh_context c = Resolve("root://");
//             Context::iterator e  = c->end();
//             Context::iterator ci = c->begin();
//             cerr << "TRY TO FIND:" << c->getDirPath() << endl;
//             for( ; ci!=e; ++ci )
//             {
//                 cerr << "ci:" << (*ci)->getDirPath() << endl;
//                 if( GetImpl(*ci) == c )
//                 {
//                     return "file";
//                 }
//             }
//         }
        
        
//        LG_FACTORY_D << "getRootContextClassName() Failing to lookup root:" << c->getDirPath() << endl;
        return "x-ferris";
    }
    
    const std::string
    RootContextFactory::getRootContextClassName( fh_context c )
    {
        return getRootContextClassName( GetImpl( c ) );
    }

    /*
     * 1.1.12: allow uris to be file names in all contexts.
     * Mainly of interest for RDF mounted contexts
     * 
     * To handle resolution of filenames that have "/" in them we should
     * check if larger parts of 'rest' are resolvable as a direct child
     * of 'ctx'
     *
     * return 0 if URI as dirname fails to resolve.
     */
    fh_context
    RootContextFactory::priv_resolveContext_tryURIAsDirName( fh_context ctx,
                                                             const string& rdn,
                                                             const string& rest,
                                                             pair<string,string>& split )
    {
        if( !split.second.empty() )
        {
            string rdnslash = split.first;
            pair<string,string> npair = split;

//             cerr << "priv_resolveContext(1) rdnslash:" << rdnslash << endl;
//             cerr << "priv_resolveContext(1) npair.first:" << npair.first << endl;
//             cerr << "priv_resolveContext(1) npair.second:" << npair.second << endl;
                    
            while( !npair.second.empty() )
            {
                npair = ctx->splitPathAtStart( npair.second );
                rdnslash = rdnslash + "/" + npair.first;

//                 cerr << "priv_resolveContext(loop) rdnslash:" << rdnslash << endl;
//                 cerr << "priv_resolveContext(loop) npair.first:" << npair.first << endl;
//                 cerr << "priv_resolveContext(loop) npair.second:" << npair.second << endl;
                        
                if( ctx->isSubContextBound( rdnslash ) )
                {
//                    cerr << "priv_resolveContext(found/rdn 0) rdnslash:" << rdnslash << endl;
                            
                    if( !npair.second.empty() )
                    {
//                         cerr << "priv_resolveContext(found/rdnA) rdnslash:" << rdnslash << endl;
//                         cerr << "priv_resolveContext(found/rdnA) npair.first:" << npair.first << endl;
//                         cerr << "priv_resolveContext(found/rdnA) npair.second:" << npair.second << endl;
                                
                        fh_context subctx = ctx;
                        subctx = ctx->getSubContext( rdnslash );

                        return priv_resolveContext(
                            subctx,
                            rdn,
                            npair.second );
                    }
                    else
                    {
//                         cerr << "priv_resolveContext(found/rdnB) rdnslash:" << rdnslash << endl;
//                         cerr << "priv_resolveContext(found/rdnB) npair.first:" << npair.first << endl;
//                         cerr << "priv_resolveContext(found/rdnB) npair.second:" << npair.second << endl;

                        switch( Style )
                        {
                        case RESOLVE_EXACT:
                        case RESOLVE_CLOSEST:
                            if( npair.first.length() )
                            {
                                return ctx->getSubContext( rdnslash );
                            }
                            return ctx;
                        case RESOLVE_PARENT:
                            return ctx;
                        }
                        fh_stringstream ss;
                        ss << "Unsupported resolve style passed style:" << Style << endl;
                        Throw_NoSuchSubContext( tostr(ss), GetImpl(ctx) );
                    }
                }
            }

//             cerr << "priv_resolveContext(e) ctx:" << ctx->getURL() << endl;
//             cerr << "priv_resolveContext(e) getHaveReadDir:" << ctx->getHaveReadDir() << endl;
//             ctx->dumpOutItems();
                    
//                     ctx->read();
//                     cerr << "priv_resolveContext(e2) ctx:" << ctx->getURL() << endl;
//                     cerr << "priv_resolveContext(e2) getHaveReadDir:" << ctx->getHaveReadDir() << endl;
//                     ctx->dumpOutItems();
//                     cerr << "priv_resolveContext(e) rdnslash:" << rdnslash << endl;
//                     cerr << "priv_resolveContext(e) npair.first:" << npair.first << endl;
//                     cerr << "priv_resolveContext(e) npair.second:" << npair.second << endl;
        }
        return 0;
    }
    
    /**
     * Resolve the context found by rdn using the working set
     * ctx and rest. Note that the resolve style given in
     * Style is used to effect how the resolution is carried out.
     *
     * @param ctx
     * @param rdn rdn of context sought
     * @param rest
     * @return Context with rdn == getDirPath()
     */
    fh_context
    RootContextFactory::priv_resolveContext(
        fh_context ctx,
        const string& rdn,
        const string& rest
        )
//    throw( NoSuchSubContext )
    {
//         CERR << "priv_resolveContext() ctx:" << GetImpl(ctx)
//              << " rc:" << ctx->ref_count
//              << " rdn:" << rdn
//              << " rest:" << rest
//              << endl;

        LG_FACTORY_D << "==================================================" << endl;
        LG_FACTORY_D << "priv_resolveContext() getDirPath:" << ctx->getDirPath() << endl;
        LG_FACTORY_D << "priv_resolveContext() getURL:" << ctx->getURL() << endl;
        LG_FACTORY_D << "priv_resolveContext() rdn:"  << rdn << endl;
        LG_FACTORY_D << "priv_resolveContext() rest:" << rest << endl;
    
        pair<string,string> split = ctx->splitPathAtStart( rest );

        LG_FACTORY_D << "priv_resolveContext() split.first :" << split.first << endl;
        LG_FACTORY_D << "priv_resolveContext() split.second:" << split.second<< endl;
        LG_FACTORY_D << "==================================================" << endl;

//         if( ContextClass == "xsltfs" )
//         {
//             if( string::npos != split.first.find( "?" ) )
//             {
//                 int andslashpos = split.second.find( "&/" );
//                 if( andslashpos == string::npos )
//                 {
//                     split.first += split.second;
//                     split.second = "";
//                 }
//                 else
//                 {
//                     split.first += split.second.substr( 0, andslashpos );
//                     split.second = split.second.substr( andslashpos + 2 );
//                 }
//             }
//         }
        
        
        /*
         * The root node is a little qwerky, we need to test this so that we
         * dont read / for no reason
         */
        if( split.first.empty() && (rest == "/" || rest == "//") )
        {
            return ctx;
        }
        
        try
        {
            LG_FACTORY_D << "priv_resolveContext() getDirPath:" << ctx->getDirPath() << endl;

//             cerr << "split.first:" << split.first << " split.first.empty():" << split.first.empty() << endl;
//             cerr << "split.sec:" << split.second << " split.sec.empty():" << split.second.empty() << endl;
//             cerr << "rest.empty:" << rest.empty() << " rest:" << rest << endl;
            
            
            /*
             * If the next path component of 'rest' is not a child of the
             * current working context we have problems.
             */
            if( !ctx->isSubContextBound( split.first ) )
            {
                bool shouldStillRead = true;
                
                /*
                 * The root node is a little qwerky, we need to test this so that we
                 * dont read / for no reason
                 */
                if( rest == "/" && split.first.empty() && split.second.empty() )
                {
                    return ctx;
                }

                LG_FACTORY_D << "priv_resolveContext() first is not bound." << endl;
                /*
                 * is a closest match acceptable?
                 */
                if( split.second.empty() && Style != RESOLVE_EXACT )
                {
                    LG_FACTORY_D << "priv_resolveContext() ending for non EXACT:" << endl;
                    LG_FACTORY_D << "priv_resolveContext() name:" << ctx->getDirName() << endl;
                    LG_FACTORY_D << "priv_resolveContext() path:" << ctx->getDirPath() << endl;
                    LG_FACTORY_D << "priv_resolveContext() addr:" << toVoid(ctx) << endl;
                    return ctx;
                }

                if( fh_context r = priv_resolveContext_tryURIAsDirName( ctx, rdn, rest, split ) )
                    return r;

                
                if( ctx->supportsShortCutLoading() )
                {
                    /*
                     * Because we no longer cleanup // to / and other things so that
                     * URI's can form directory names, we have to handle the case that
                     * there was a "/rdn" and split.first is empty, split.second is "rdn"
                     */
                    if( split.first.empty() && !split.second.empty() )
                    {
                        return priv_resolveContext( ctx, split.second, split.second );
                    }

                    //
                    // If the filesystem does shortcut loading and the user wants
                    // to hit an overlay virtual softlink directly, we need
                    // to give the context a chance to set that up.
                    //
                    if( !hasOverMounter( ctx ) )
                    {
                        ctx->OnReadComplete_setupUserOverlayLinks();
                        LG_USEROVERLAY_D << "rdn:" << rdn
                                         << " rest:" << rest
                                         << " first:" << split.first
                                         << " sec:" << split.second
                                         << endl;
                        if( ctx->isSubContextBound( split.first ) )
                        {
                            if( fh_context r = priv_resolveContext( ctx, rdn, rest ) )
                            {
                                LG_USEROVERLAY_D << "have r:" << r->getURL() << endl;
                                return r;
                            }
                        }
                    }
                                        
                    // if the module supports ShortCutLoading then the above call to
                    // isSubContextBound() which calls priv_getSubContext() would have
                    // checked for sure that the subcontext does not exist,
                    // this we know now that the object we are seeking can't exist.
                    //
                    // We have to check if there is an overmounter willing to mount
                    // this object that might provide children also before fully
                    // giving up

                    LG_FACTORY_D << "priv_resolveContext() supports short cut loading." << endl;
                    if( !hasOverMounter( ctx ) )
                    {
                        static const gchar* LIBFERRIS_ENABLE_ATTRIBUTE_RESOLUTION =
                            g_getenv ("LIBFERRIS_ENABLE_ATTRIBUTE_RESOLUTION");

                        //
                        // It gets more fun. If the user wants file@eaname to work we
                        // can't assume failure if a short cut loading path has failed
                        // FIXME:
                        //
//                        if( !LIBFERRIS_ENABLE_ATTRIBUTE_RESOLUTION )
                        {
                            fh_stringstream ss;
                            ss << "Context does not exist. rdn:" << split.first
                               << " split.second:" << split.second
                               << " rest:" << rest
                               << " parent:" << ctx->getURL() << endl;
                            LG_FACTORY_D << "priv_resolveContext() no overmounter, ss:" << tostr(ss) << endl;
//                        BackTrace();
                            Throw_NoSuchSubContext( tostr(ss), GetImpl(ctx) );
                        }
                    }

                    // we know there is an overmounter, so overmount it and try
                    // to hit the path directly if the overmounting plugin supports
                    // short cut loading.
                    ctx->tryToOverMount();
                    if( ctx->getOverMountContext()->supportsShortCutLoading() )
                    {
                        if( ctx->isSubContextBound( split.first ) )
                        {
                            shouldStillRead = false;
                        }
                    }
                }
                
                LG_FACTORY_D << "priv_resolveContext() reading path:" << ctx->getDirPath()
                             << " because subcontext:" << split.first
                             << " could not be found."
                             << " first:" << split.first
                             << " sec:" << split.second
                             << " rest:" << rest
                             << endl;

                if( shouldStillRead )
                    ctx->read();

                LG_FACTORY_D << "priv_resolveContext(2) read path:" << ctx->getDirPath()
                             << " because subcontext:" << split.first
                             << " could not be found."
                             << " first:" << split.first
                             << " sec:" << split.second
                             << " rest:" << rest
                             << endl;

                
                /*
                 * Try again for URI as filename because we may have overmounted
                 * a new context.
                 */
                if( fh_context r = priv_resolveContext_tryURIAsDirName( ctx, rdn, rest, split ) )
                    return r;
            }
            
            
        
            LG_FACTORY_D << "priv_resolveContext() getDirPath:" << ctx->getDirPath() << endl;

            if( split.second.length() )
            {
                fh_context subctx = ctx;
                if( split.first.length() )
                {
                    LG_FACTORY_D << "priv_resolveContext() split.first:" << split.first << endl;
                    subctx = ctx->getSubContext( split.first );
                }
            
                LG_FACTORY_D << "priv_resolveContext() x dirname:" << subctx->getDirName() << endl;
                LG_FACTORY_D << "priv_resolveContext() x dirpath:" << subctx->getDirPath() << endl;

                return priv_resolveContext(
                    subctx,
                    rdn,
                    split.second);
            }
            else
            {
                LG_FACTORY_D << "priv_resolveContext() finalizing... split.first:"
                             << split.first << endl;
                LG_FACTORY_D << "priv_resolveContext() ctx->path:" << ctx->getDirPath() << endl;
                LG_FACTORY_D << "priv_resolveContext() ctx->url:" << ctx->getURL() << endl;
                LG_FACTORY_D << "priv_resolveContext() closest|exact split.first:"
                             << split.first << endl;

                
                switch( Style )
                {
                case RESOLVE_EXACT:

                    if( split.first.length() )
                    {
//                            if( !ctx->isSubContextBound( split.first ) )
                        try
                        {
                           //
                           // libferris 1.4.3: adding the ability to convert
                           // url/context@attribute into
                           // branchfs-attributes://url/context/attribute
                           //
                           static const gchar* LIBFERRIS_ENABLE_ATTRIBUTE_RESOLUTION =
                              g_getenv ("LIBFERRIS_ENABLE_ATTRIBUTE_RESOLUTION");

                           LG_FACTORY_D << "LIBFERRIS_ENABLE_ATTRIBUTE_RESOLUTION:" << (LIBFERRIS_ENABLE_ATTRIBUTE_RESOLUTION!=0) << endl;
                           LG_FACTORY_D << "split.first:" << split.first << endl;
                           LG_FACTORY_D << "split.second:" << split.second << endl;
                           LG_FACTORY_D << "rest:" << rest << endl;
                           
                           if( LIBFERRIS_ENABLE_ATTRIBUTE_RESOLUTION
                               && string::npos != split.first.find("@") )
                           {
                              stringpair_t p = ::Ferris::split( split.first, "@" );
                              stringstream earlss;
                              earlss << "branchfs-attributes:"
                                     << ctx->getURLScheme() << "/" << ctx->getDirPath()
                                     << "/" << p.first << "/" << p.second
                                     << flush;
                              LG_FACTORY_D << "brachfs url:" << earlss.str() << endl;

                              fh_context ret = Resolve( earlss.str() );
                              LG_FACTORY_D << "priv_resolveContext() ret:" << ret->getURL() << endl;
                              return ret;
                           }
                           
                           fh_context ret = ctx->getSubContext( split.first );
                           LG_FACTORY_D << "priv_resolveContext() ret:" << ret->getURL() << endl;
                           return ret;
                        }
                        catch( NoSuchSubContext& e )
                        {
//                             cerr << "RESOLVE_EXACT url:" << ctx->getURL()
//                                  << " rdn:" << split.first
//                                  << endl;
#ifdef FERRIS_DEBUG_RESOLVE
                            ctx->dumpOutItems();
                            DEBUG_dumpcl( "Context does not exist" );
#endif
                            fh_stringstream ss;
                            ss << "Context does not exist. rdn:" << split.first
                               << " parent:" << ctx->getURL() << endl;
                            LG_FACTORY_D << tostr(ss);
//                            BackTrace();
                            
                            Throw_NoSuchSubContext( tostr(ss), GetImpl(ctx) );
                        }
                    }
                    return ctx;

                case RESOLVE_CLOSEST:

                    if( split.first.length() )
                    {
                        return ctx->getSubContext( split.first );
                    }
                    return ctx;
                
                
                case RESOLVE_PARENT:
                    LG_FACTORY_D << "priv_resolveContext() term on RESOLVE_PARENT:\n";
                    LG_FACTORY_D << "priv_resolveContext() name:" << ctx->getDirName() << endl;
                    LG_FACTORY_D << "priv_resolveContext() path:" << ctx->getDirPath() << endl;
                    LG_FACTORY_D << "priv_resolveContext() addr:" << toVoid(ctx) << endl;
                    return ctx;
                }
                LG_FACTORY_D << "priv_resolveContext() should not be reached\n";
            }
        }
    
        catch( NoSuchSubContext& e )
        {
            LG_FACTORY_D << "priv_resolveContext() NoSuchSubContext:" << e.what() << endl;
            switch( Style )
            {
            case RESOLVE_CLOSEST: return ctx;
            }
            throw;
        }
        catch( exception& e )
        {
            LG_FACTORY_ER << "priv_resolveContext() exception:" << e.what() << endl;
        }
        catch( ... )
        {
            LG_FACTORY_ER << "priv_resolveContext() ...\n";
        }
        
        {
            fh_stringstream ss;
            ss << "Should never happen!"
               << " rdn:" << rdn
               << " rest:" << rest
               << endl;
            Throw_NoSuchSubContext( tostr(ss), GetImpl(ctx) );
        }
    }

    const string RootContextFactory::PATH = "Path";
    const string RootContextFactory::ROOT = "Root";
    const string RootContextFactory::SERVERNAME = "ServerName";

    static bool isRelPath( string p )
    {
        return (p.length() && p[0] != '/');
    }

    fh_context
    RootContextFactory::resolveContext_relative_file_path( ResolveStyle s )
    {
        string path = getInfo( PATH );
        string root = getInfo( ROOT );
        /*
         * Get CWD is always non relative, thus it will not come back in here
         * and getRootContext() will be set by the next line.
         */
        fh_context rc = Shell::getCWD();
        string relativePath = path.length() ? path : root;

        AddInfo( PATH, relativePath );
        AddInfo( ROOT, "/" );

        fh_context ret = rc->getRelativeContext( relativePath, this );
//      cerr << "RootContextFactory::resolveContext() ret:" << isBound(ret) << endl;
        return ret;
    }
    
    

/**
 * Using the preset data from setContextClass() and AddInfo() resolve
 * to a Context using the resolution style given.
 *
 *
 * @param s How exact to make the resolution.
 * @return The context
 */
    fh_context
    RootContextFactory::resolveContext( ResolveStyle s )
    {
        CacheManager::fh_insideResolve resolveGuard = getCacheManager()->getInsideResolve();
        
        Style = s;
        string path = getInfo( PATH );
        string root = getInfo( ROOT );

        LG_FACTORY_D << "RootContextFactory::resolveContext()" << endl;
        LG_FACTORY_D << "RootContextFactory::resolveContext() path:" << path << endl;
        LG_FACTORY_D << "RootContextFactory::resolveContext() root:" << root << endl;
        LG_FACTORY_D << "RootContextFactory::resolveContext() Style:" << Style << endl;
        LG_FACTORY_D << "RootContextFactory::resolveContext() s:" << s << endl;
        LG_FACTORY_D << "RootContextFactory::resolveContext() ContextClass:" << ContextClass << endl;

        /*
         * Resolve relative paths. This is a little mucky.
         */
        if( ContextClass=="Native" || ContextClass=="file"
            && (isRelPath(path) || isRelPath(root)))
        {
            return resolveContext_relative_file_path( s );
        }

        fh_context ctx = 0;
        try
        {
            ctx = getRootContext();
        }
        catch( RootContextCreationFailed& e )
        {
            if( isRelPath(path) || isRelPath(root) )
            {
                return resolveContext_relative_file_path( s );
            }
            throw;
        }
        

        LG_FACTORY_D << "resolveContext() root path:" << ctx->getDirPath() << endl;
//     cerr << "resolveContext() root :" << root << endl;
//     cerr << "resolveContext() path :" << path << endl;
    
        try
        {
            string rdn = getInfo( "Path" );

            /**
             * They just want the root context, so our work is complete before
             * it really begins.
             */
            if( rdn.empty() )
            {
                switch(Style)
                {
                case RESOLVE_CLOSEST:
                case RESOLVE_EXACT:
                    return ctx;
                
                default:
                    Throw_NoSuchSubContext("No parent of root context", 0 );
                }
            }


//        cerr << "resolveContext() calling priv() len:" << rdn.length() << endl;

            /*
             * The user has given us a path to resolve, we pass that to
             * priv_resolveContext() to do the actual work.
             */
            string trimmed = ctx->trimEdgeSeps( rdn );
            LG_FACTORY_D << "resolveContext() trimmed:" << trimmed << endl;
            LG_FACTORY_D << "resolveContext() dirname:" << ctx->getDirName() << endl;
            LG_FACTORY_D << "resolveContext() dirpath:" << ctx->getDirPath() << endl;
//        sleep(4);
            LG_FACTORY_D << "resolveContext() calling priv()" << endl;

//             CERR << "calling priv_resolveContext() ctx:" << GetImpl(ctx)
//                  << " rc:" << ctx->ref_count
//                  << " rdn:" << rdn
//                  << endl;
            fh_context ret = priv_resolveContext( ctx, rdn, trimmed );
            return ret;
        }
        catch( exception& e )
        {
            LG_FACTORY_ER << "resolveContext() e:" << e.what() << endl;
            throw;
        }
        catch( ... )
        {
            LG_FACTORY_ER << "resolveContext() e:... " << endl;
            throw;
        }

//    return ResolveData;
        return 0;
    }

    

/**
 * Bind the context class name CtxName to the context dropper given.
 *
 * @param CtxName context class name
 * @param dropper A class that can drop new contexts.
 */
    void
    RootContextFactory::Register( string CtxName, RootContextDropper* dropper )
    {
        if( !dropper )
        {
            LG_CTX_ER << "RootContextFactory::Register() without valid dropper for"
                      << " ctx:" << CtxName << endl;
        }
        
//    cerr << "RootContextFactory::Register() CtxName:" << CtxName << endl;
        getDroppers()[ CtxName ] = dropper;
    }


    /**
     * Scan the context library directory looking for factory objects and add then
     * all to droppers.
     *
     * Note that this method only performs the scan one time for the lifetime of
     * the factory.
     *
     * @param droppers Where to put any new context droppers found.
     *
     */
    void
    RootContextFactory::ensureGModuleFactoriesLoaded( Droppers_t& droppers )
    {
        static bool virgin = true;
//        string SystemPluginDir = PREFIX + "/lib/ferris/plugins/context/";
        string SystemPluginDir = makeFerrisPluginPath( "context" );

        // FIXME:
//    LG_PLUGIN_D << "ensureGModuleFactoriesLoaded()" << endl;

        if( virgin )
        {
            virgin = false;

            DIR *d;
            struct dirent *e;

	
            stringlist_t pluginlist;
            if ((d = opendir ( SystemPluginDir.c_str() )) == NULL)
            {
                LG_PLUGIN_ER << "Can not open system plugin dir :" << SystemPluginDir << endl;
                return;
            }
            while ((e = readdir (d)) != NULL)
            {
                string fn = e->d_name;
                pluginlist.push_back(fn);
            }
            closedir (d);


            for( stringlist_t::iterator pi = pluginlist.begin();
                 pi != pluginlist.end(); ++pi )
            {
                string fn = *pi;
                
//              cerr << "fn:" << fn << endl;
//              LG_PLUGIN_I << "Found:" << fn << endl;

                const string factory_tail = "factory.so";
                if( ends_with( fn, factory_tail ) )
                {
                    try
                    {
//                     LG_PLUGIN_I << "Loading plugin:" << fn << endl;
                
                        ostringstream ss;
                        ss << SystemPluginDir << fn;
                        gmodule_RootContextDropper* drop = new
                            gmodule_RootContextDropper( tostr(ss) );
                    
                        droppers[ drop->getName() ] = drop;
//                        cerr << "Loaded new context class:" << drop->getName() << endl;
                    }
                    catch( GModuleOpenFailed& e )
                    {
//                     LG_PLUGIN_ER << "Failed to load plugin" << endl;
//                     LG_PLUGIN_ER << "Failed to load plugin:" << fn << endl;

//                     cerr << "Failed to load plugin" << endl;
                        LG_PLUGIN_W << "Failed to load plugin:" << fn
                                    << " e:" << e.what()
                                    << endl;
                        cerr << "Failed to load plugin:" << fn
                             << " e:" << e.what()
                             << endl;
                    }
                }
            }
        }

//         cerr << "RootContextFactory::ensureGModuleFactoriesLoaded(begin)" << endl;
//         typedef Droppers_t::iterator DI;
//         for( DI di = droppers.begin(); di!=droppers.end(); ++di )
//         {
//             cerr << " name:" << di->first << " object:" << toVoid(di->second) << endl;
//         }
//         cerr << "RootContextFactory::ensureGModuleFactoriesLoaded(end)" << endl << endl;
        
        
    }



/**
 * Get the collection of droppers that is keyed on a string context class name
 */
    RootContextFactory::Droppers_t&
    RootContextFactory::getDroppers()
    {
        static Droppers_t Droppers;
        ensureGModuleFactoriesLoaded( Droppers );
        return Droppers;
    }

    void
    RootContextFactory::setBaseOverMountContext( fh_context c )
    {
        BaseOverMountContext = c;
    }

    const fh_context&
    RootContextFactory::getBaseOverMountContext()
    {
        return BaseOverMountContext;
    }


/**
 * Find out if there is a possible overmount for this context.
 *
 * This is primarily used in Context::getHasSubContextsGuess() to check
 * if a context that is not reable natively having subcontexts if there
 * would be a possible overmounter to allow reading the context as a
 * dir.
 */
    bool
    RootContextFactory::hasOverMounter( const fh_context& ctx )
    {
//        cerr << "RootContextFactory::hasOverMounter( enter ) c:" << ctx->getURL() << endl;
        
        bool ret = false;
    
        for( Droppers_t::const_iterator iter = getDroppers().begin();
             iter != getDroppers().end(); iter++ )
        {
//             cerr << "RootContextFactory::hasOverMounter() first:" << iter->first
//                  << " sec:" << toVoid(iter->second) << endl;
            
            fh_ommatchers m = iter->second->GetOverMountMatchers( this );
        
            for( fh_ommatchers::iterator mat_iter = m.begin();
                 mat_iter != m.end(); mat_iter++ )
            {
                LG_OVERMOUNT_D << "testing:" << iter->first << endl;
                LG_OVERMOUNT_D << "ctx:"     << ctx->getDirPath() << endl;

                fh_ommatcher& f = *mat_iter;
                if( f( ctx ) )
                {
//                    cerr << "RootContextFactory::hasOverMounter( exit ) c:" << ctx->getURL() << endl;
                    return true;
                }
            }
        }
//        cerr << "RootContextFactory::hasOverMounter( exitf ) c:" << ctx->getURL() << endl;
        return ret;
    }

    /**
     * Core of findOverMounter which was abstracted to a private method so that
     * the main public findOverMounter() method can choose which ordering to
     * test the droppers in
     *
     * @param ctx Base context to try to mount over
     * @param followLinkTTL how many times to dereference softlinks before giving up
     * @param dropperName the first  part of a Droppers_t entry
     * @param dropper     the second part of a Droppers_t entry
     *
     * @return Context that is mounted over ctx or un unBound() context on failure.
     */
    fh_context
    RootContextFactory::findOverMounter_TryDropper( fh_context ctx,
                                                    int followLinkTTL,
                                                    const std::string&  dropperName,
                                                    RootContextDropper* dropper,
                                                    bool attemptingOverMountOnlyToFindEA )
    {
//         cerr << "TryDropper ctx:" << ctx->getURL()
//              << " attemptingOverMountOnlyToFindEA:" << attemptingOverMountOnlyToFindEA
//              << " dropper->tryToOverMountToFindEA:" << dropper->tryToOverMountToFindEA()
//              << endl;
        
        if( attemptingOverMountOnlyToFindEA )
        {
            if( !dropper->tryToOverMountToFindEA())
            {
                LG_OVERMOUNT_D << "findOverMounter_TryDropper() only looking for ea... returning."
                               << " attemptingOverMountOnlyToFindEA:" << attemptingOverMountOnlyToFindEA
                               << endl;
                return 0;
            }
        }
        
        fh_ommatchers m = dropper->GetOverMountMatchers( this );

//         cerr << "RootContextFactory::TryDropper() name:" << dropperName
//              << " number of matchers:" << m.size()
//              << endl;
        
        for( fh_ommatchers::iterator mat_iter = m.begin();
             mat_iter != m.end(); mat_iter++ )
        {
            LG_OVERMOUNT_D << "testing:" << dropperName << endl;
            LG_OVERMOUNT_D << "ctx:"     << ctx->getDirPath() << endl;

//             cerr << "testing:" << dropperName << endl;
//             cerr << "ctx:"     << ctx->getDirPath() << endl;
            
            fh_ommatcher& f = *mat_iter;
            if( f( ctx ) )
            {
//                    cerr << "Overmount: Got a match on:" << dropperName << endl;
                LG_OVERMOUNT_I << "Got a match on:" << dropperName << endl;
//                cerr << "Got a match on:" << dropperName << endl;

                BaseOverMountScope scope( this, ctx );
                setContextClass( dropperName );
                AddInfo( RootContextFactory::ROOT, ctx->getDirPath() );
//                    (*mat_iter)->setup( this );

                fh_context ret = dropper->Brew( this );
                ret->setCoveredContext( ctx );
                return ret;
            }
        }
        
        return 0;
    }
    


    /**
     * Find a context that is willing to mount itself over the given context.
     * 
     * This method is also used by context to find an overmounter.
     *
     * @param ctx Base context to try to mount over
     * @param followLinkTTL how many times to dereference softlinks before giving up
     *
     * @return Context that is mounted over ctx
     */
    fh_context
    RootContextFactory::findOverMounter( fh_context ctx,
                                         int followLinkTTL,
                                         bool attemptingOverMountOnlyToFindEA )
    {
        LG_OVERMOUNT_I << "RootContextFactory::findOverMounter() path:" << ctx->getDirPath()
                       << " attemptingOverMountOnlyToFindEA:" << attemptingOverMountOnlyToFindEA
                       << endl;
        

        Droppers_t::const_iterator iter = getDroppers().find( "rdf" );
        Droppers_t::const_iterator end  = getDroppers().end();
        
        if( iter != end )
        {
            LG_OVERMOUNT_D << "RootContextFactory::findOverMounter(rdf) path:" << ctx->getDirPath()
                           << " iter->first:" << iter->first
                           << endl;

            const std::string dropperName = iter->first;
            RootContextDropper*   dropper = iter->second;
            fh_context c = findOverMounter_TryDropper( ctx,
                                                       followLinkTTL,
                                                       dropperName,
                                                       dropper,
                                                       attemptingOverMountOnlyToFindEA );
            if( c )
                return c;
        }
        LG_OVERMOUNT_D << "RootContextFactory::findOverMounter(other) path:" << ctx->getDirPath() << endl;
        
        for( iter = getDroppers().begin(); iter != end; iter++ )
        {
            LG_OVERMOUNT_D << "RootContextFactory::findOverMounter(x) path:" << ctx->getDirPath()
                           << " iter->first:" << iter->first
                           << endl;
            
            const std::string dropperName = iter->first;
            RootContextDropper*   dropper = iter->second;
            fh_context c = findOverMounter_TryDropper( ctx, followLinkTTL,
                                                       dropperName,
                                                       dropper,
                                                       attemptingOverMountOnlyToFindEA );
            if( c )
                return c;

//             fh_ommatchers m = iter->second->GetOverMountMatchers( this );

//             for( fh_ommatchers::iterator mat_iter = m.begin();
//                  mat_iter != m.end(); mat_iter++ )
//             {
//                 LG_OVERMOUNT_D << "testing:" << iter->first << endl;
//                 LG_OVERMOUNT_D << "ctx:"     << ctx->getDirPath() << endl;

//                 fh_ommatcher& f = *mat_iter;
//                 if( f( ctx ) )
//                 {
// //                    cerr << "Overmount: Got a match on:" << iter->first << endl;
//                     LG_OVERMOUNT_I << "Got a match on:" << iter->first << endl;

//                     BaseOverMountScope scope( this, ctx );
//                     setContextClass( iter->first );
//                     AddInfo( RootContextFactory::ROOT, ctx->getDirPath() );
// //                    (*mat_iter)->setup( this );

//                     fh_context ret = iter->second->Brew( this );
//                     ret->setCoveredContext( ctx );
//                     return ret;
//                 }
//             }
        }

        if( followLinkTTL )
        {
            string realpath = getStrAttr( ctx, "realpath", "" );
            if( !realpath.empty() )
            {
                try
                {
                    fh_context targetc = Resolve( realpath );

                    //
                    // Dont follow links to ourself.
                    //
                    if( targetc->getURL() != ctx->getURL() )
                    {
                        return findOverMounter( targetc, --followLinkTTL );
                    }
                }
                catch( exception& e )
                {
                    ostringstream ss;
                    ss << "RootContextFactory::findOverMounter()"
                       << " can not follow link when attempting to locate overmounter path:" 
                       << ctx->getDirPath() << endl;
                    LG_OVERMOUNT_D << tostr(ss) << endl;
                    Throw_FerrisNotReadableAsContext( tostr(ss), 0 );
                }
            }
        }
        
        ostringstream ss;
        ss << "RootContextFactory::findOverMounter() NO OVERMOUNTER path:"
           << ctx->getDirPath() << endl;

        LG_OVERMOUNT_D << tostr(ss);
        Throw_FerrisNotReadableAsContext( tostr(ss), 0 );
    }

    

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

//     string CleanupURL( const string& s )
//     {
//         string ret = s;
    
//         if( !s.length() )
//         {
//             fh_stringstream ss;
//             ss << "Zero length URL given.";
//             Throw_MalformedURL( tostr(ss), 0 );
//         }
//         if( s[0] == '~' )
//         {
//             ret = "file://";
//             ret += Shell::getHomeDirPath_nochecks();
//             ret += "/";
//             if( string::npos != s.find("/") )
//             {
//                 ret += s.substr( s.find("/") );
//             }
//         }
//         else if( s[0] == '.' )
//         {
//             ret = "file://";
//             ret += Shell::getCWDDirPath();
//             ret += "/";
//             ret += s;
//         }
//         else if( s[0] == '/' )
//         {
//             ret = "file://";
//             ret += s;
//         }
//         else if( string::npos == s.find(":") )
//         {
//             ret = "file://";
//             ret += Shell::getCWDDirPath();
//             ret += "/";
//             ret += s;
//         }

// //        cerr << "before /./ reduction ret:" << ret << endl;
//         int loc = ret.find("/./");
//         while( loc != string::npos )
//         {
//             fh_stringstream ss;
//             ss << ret.substr( 0, loc ) << "/" << ret.substr( loc+3 );
//             ret = tostr(ss);
//             loc = ret.find("/./");
//         }

// //        cerr << "after /./ reduction ret:" << ret << endl;
        
//         // Remove unescaped '//' to be '/'
//         {
//             int loc = ret.find("//");
//             while( loc != string::npos )
//             {
//                 fh_stringstream ss;
//                 ss << ret.substr( 0, loc ) << "/" << ret.substr( loc+2 );
//                 ret = tostr(ss);
//                 loc = ret.find("//");
//             }
//         }

//         if( starts_with( ret, "file:" ))
//         {
//             ret = ret.substr( 5 );
//         }
        
//         return ret;
//     }

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

    string
    FerrisURL::getScheme()
    {
        return scheme;
    }
    
    string
    FerrisURL::getInternalFerisScheme()
    {
        string xferris = "x-ferris";
        
        if( starts_with( getScheme(), xferris ))
        {
            return "file";
            // string ret = getScheme().substr( xferris.length() );
            // return ret;
        }
        return getScheme();
    }

    std::string
    FerrisURL::tostr()
    {
        return getScheme() + "://" + getPath();
    }

    std::string
    FerrisURL::getURL()
    {
        return this->tostr();
    }
    
    string
    FerrisURL::getPath()
    {
        return path;
    }

    FerrisURL
    FerrisURL::fromString( const string& s )
    {
        FerrisURL ret;
        string::size_type colonidx = s.find( ":" );
        if( colonidx == string::npos )
        {
            ret.scheme = "file";
            ret.path = s;
        }
        else
        {
            ret.scheme = s.substr( 0, colonidx );
            ret.path = s.substr( colonidx+1 );
        }
        return ret;
    }

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
    
    ostream& operator<<(ostream& ss, FerrisURL& u )
    {
        return ss << " scheme:" << u.getScheme() << " path:" << u.getPath() << endl;
    }

    /**
     * Given a URL like earl="prefix://params/other"
     * where prefix is contained the the prefix function arg,
     * we remove the params part of the URL and put that into
     * extraData.
     *
     * the string earl is modified to have the "prefix://params"
     * portion removed and the FerrisURL 'u' is updated to reflect
     * the new earl value.
     */
    static void
    Resolve_AdjustForWrap( const std::string& dirty_earl,
                           string& earl,
                           FerrisURL& u,
                           const string& prefix,
                           string& extraData )
    {
        string s = earl.substr( string(prefix).length() );
        int endOfExtraData = s.find( "/" );

        if( endOfExtraData == string::npos )
        {
            fh_stringstream ss;
            ss << "Malformed " << prefix << " in URL:" << dirty_earl << endl;
            Throw_MalformedURL( tostr(ss), 0 );
        }
            
        extraData = s.substr( 0, endOfExtraData );

        /* Adjust the earl not to have this prefix & extradata stuff in it */
        string rest = s.substr( endOfExtraData );
        PrefixTrimmer trimmer;
        trimmer.push_back( "/" );
        earl = trimmer( rest );
        earl = CleanupURL( earl );
        
//         cerr << " rest:" << rest
//              << " earl:" << earl
//              << endl;
        u = FerrisURL::fromString( earl );
    }

    static string
    Resolve_FirstManyToOneURL( string& earl, list< FerrisURL >& urllist )
    {
        if( earl.empty() )
            return earl;
        
        int colonpos  = earl.find(":");
        if( colonpos == string::npos )
            return "";

        // For finding the start of the 3rd and on URL we need to skip one colon
        if( !urllist.empty() )
        {
            int npos = earl.substr( colonpos+1 ).find(":");
            if( npos == string::npos )
            {
                string ret = earl;
                earl = "";
                return ret;
            }
            colonpos  = colonpos + npos;
        }
                    
        string tmp        = earl.substr( 0, colonpos );
        int secondURLpos  = tmp.rfind("/");
        if( secondURLpos == string::npos )
        {
//             cerr << "Resolve_FirstManyToOneURL(X) earl:" << earl
//                  << " secondURLpos:" << secondURLpos
//                  << " colonpos:" << colonpos
//                  << " urllist.empty():" << urllist.empty()
//                  << endl;
            return earl;
        }

        string ret = earl.substr( 0, secondURLpos );
//         cerr << "Resolve_FirstManyToOneURL() earl:" << earl
//              << " ret:" << ret
//              << " secondURLpos:" << secondURLpos
//              << " colonpos:" << colonpos
//              << endl;
        earl = earl.substr( secondURLpos+1 );
        return ret;
    }

    static void
    Resolve_FirstManyToOneURL_PostAssert( list< FerrisURL >& urllist, int min, string prefix )
    {
        typedef list< FerrisURL > urllist_t;
        
        if( urllist.empty() || urllist.size() < min )
        {
            cerr << "no urls for " << prefix << endl;
        }

        for( urllist_t::iterator uiter = urllist.begin(); uiter != urllist.end(); ++uiter )
        {
            cerr << prefix << ":// list of contexts:" << uiter->getURL() << endl;
        }
    }
    
    fh_context Resolve( const std::string earl, ResolveStyle rs, int ex )
    {
        CacheManager::fh_insideResolve resolveGuard = getCacheManager()->getInsideResolve();
        
        fh_context ret = Resolve( earl, rs );
        
        if( ex & RESOLVEEX_UNROLL_LINKS )
        {
            ret = Shell::unrollLinks( ret );
        }
        
        return ret;
    }
    
    fh_context Resolve( const std::string dirty_earl, ResolveStyle rs )
    {
        CacheManager::fh_insideResolve resolveGuard = getCacheManager()->getInsideResolve();
        //
        // This is a little nasty, in XPath there is a really big difference between
        // a leading "/" and a leading "//"
        // /  --> start at the top
        // // --> any subtree from the top
        //
        string earl = dirty_earl;
        if( earl.find( "xpath:/" ) == string::npos )
            earl = CleanupURL( earl, false, true );

        LG_CTX_D << "Resolve() dirty_earl:" << dirty_earl
                 << " clean_earl:" << earl
                 << endl;
        
        FerrisURL u = FerrisURL::fromString( earl );
        bool   CacheWrapper_Add        = false;
        string CacheWrapper_extraData  = "";

        bool   InheritEAWrapper_Add       = false;
        string InheritEAWrapper_extraData = "";
        
        bool   FilterWrapper_Add       = false;
        string FilterWrapper_extraData = "";
        string FilterWrapper_FilterString = "";
        const string FILTERWRAPPER_FILTERSTRING_KEY = "predicate";
        fh_context FilterWrapper_FilterObject       = 0;

        typedef list< FerrisURL > urllist_t;
        bool   UnionWrapper_Add         = false;
        urllist_t UnionWrapper_urllist;
        bool   SetDiffWrapper_Add       = false;
        urllist_t SetDiffWrapper_urllist;
        bool   SetIntersectWrapper_Add  = false;
        urllist_t SetIntersectWrapper_urllist;
        bool   SetSymmetricDiffWrapper_Add  = false;
        urllist_t SetSymmetricDiffWrapper_urllist;
        bool   DiffWrapper_Add         = false;
        urllist_t DiffWrapper_urllist;
        bool   URLTrWrapper_Add        = false;
        urllist_t URLTrWrapper_urllist;

        
        
        bool   SortingWrapper_Add       = false;
        string SortingWrapper_SortString = "";
        fh_sorter SortingWrapper_sorter;


        //
        // A single file as a filesystem. Specific use is in a FUSE mount
        // for example for an xsltfs:// where you are really only interested
        // in a single file which is a read/write result of an XSLT.
        //
        string single_file_filesystem = "single-file-filesystem:";
        if( starts_with( earl, single_file_filesystem ) )
        {
            string fileURL = earl.substr( single_file_filesystem.length() );
            PrefixTrimmer trimmer;
            trimmer.push_back( "/" );
            fileURL = trimmer( fileURL );
            LG_CTX_D << "fileURL:" << fileURL << endl;

            fh_context fc = Resolve( fileURL );
            fh_context selfactory = Resolve( "selectionfactory://" );
            fh_context selection  = selfactory->createSubContext( "" );
            if( SelectionContext* sc = dynamic_cast<SelectionContext*>( GetImpl(selection ) ) )
            {
                sc->setSelectionContextRDNConflictResolver(
                    get_SelectionContextRDNConflictResolver_MonsterName() );
            }

            LG_CTX_D << "--------Setting the child...------------" << endl;
            selection->createSubContext( "", fc );

            LG_CTX_D << "selection.url :" << selection->getURL() << endl;
            LG_CTX_D << "selection.path:" << selection->getDirPath() << endl;
            LG_CTX_D << "selection.rdn :" << selection->getDirName() << endl;
            LG_CTX_D << "selection.child.url :" << (*selection->begin())->getURL() << endl;
            LG_CTX_D << "selection.child.path:" << (*selection->begin())->getDirPath() << endl;
            LG_CTX_D << "selection.child.rdn :" << (*selection->begin())->getDirName() << endl;

            LG_CTX_D << "selection.child.rdn:" << (*selection->begin())->getDirName() << endl;
            
            for( Context::iterator ci = selection->begin(); ci != selection->end(); ++ci )
            {
                LG_CTX_D << "ci.rdn:" << (*ci)->getDirName() << endl;
            }
            return selection;
        }
        
        string cacheprefix = "cachecontext:";
        if( starts_with( earl, cacheprefix ) )
        {
            CacheWrapper_Add = true;
            Resolve_AdjustForWrap( dirty_earl, earl, u,
                                   cacheprefix, CacheWrapper_extraData );
        }

        
        string inheritEAprefix = "inheritea:";
        if( starts_with( earl, inheritEAprefix ) )
        {
            InheritEAWrapper_Add = true;
            Resolve_AdjustForWrap( dirty_earl, earl, u,
                                   inheritEAprefix, InheritEAWrapper_extraData );
//             cerr << "After finding inheritea:// prefix."
//                  << " dirty:" << dirty_earl << endl
//                  << " earl:" << earl << endl
//                  << " InheritEAWrapper_extraData:" << InheritEAWrapper_extraData
//                  << endl;
        }


        
        /*
         * Try to perform as much work as possible here so that we don't bother
         * resolving the actual URL if the filter is botched.
         */
        string sortingprefix = "sort:";
        if( starts_with( earl, sortingprefix ) )
        {
            SortingWrapper_Add = true;
            Resolve_AdjustForWrap( dirty_earl, earl, u,
                                   sortingprefix, SortingWrapper_SortString );
            PostfixTrimmer trimmer;
            trimmer.push_back( "/" );
            SortingWrapper_SortString = trimmer( SortingWrapper_SortString );
            
//             cerr << "Found request, Sorting by -->" << SortingWrapper_SortString
//                  << "<--" << endl;
            SortingWrapper_sorter = Factory::MakeSorter( SortingWrapper_SortString );
        }


        string filterprefix = "filter:";
        if( starts_with( earl, filterprefix ) )
        {
            FilterWrapper_Add = true;
            Resolve_AdjustForWrap( dirty_earl, earl, u,
                                   filterprefix, FilterWrapper_extraData );

//             StringMap_t m = Util::ParseKeyValueString( FilterWrapper_extraData );
//             FilterWrapper_FilterString = m[ FILTERWRAPPER_FILTERSTRING_KEY ];
            FilterWrapper_FilterString = FilterWrapper_extraData;

            LG_FACTORY_D
                << "Filtering though resolve string, filter string is:" << FilterWrapper_extraData
                << " FilterWrapper_FilterString:" << FilterWrapper_FilterString
                << " earl:" << earl << endl;

            FilterWrapper_FilterObject = Factory::MakeFilter( FilterWrapper_FilterString );
        }

        string unionprefix = "union:";
        if( starts_with( earl, unionprefix ) )
        {
            UnionWrapper_Add = true;
            string ex;
            Resolve_AdjustForWrap( dirty_earl, earl, u, unionprefix, ex );

//             cerr << "Wrapping for union://. ex:" << ex
//                  << " dirty_earl:" << dirty_earl
//                  << " earl:" << earl
//                  << endl;
            
            while( true )
            {
//                 cerr << "Wrapping for union:// while(top)"
//                      << " earl:" << earl
//                      << endl;
                string nextu = Resolve_FirstManyToOneURL( earl, UnionWrapper_urllist );
                if( nextu.empty() )
                    break;

//                 cerr << "Wrapping for union:// while(after FirstManyToOneURL)"
//                      << " earl:" << earl
//                      << " next:" << nextu
//                      << endl;
                
                FerrisURL x = FerrisURL::fromString( nextu );
                UnionWrapper_urllist.push_back( x );

                
//                 cerr << "Wrapping for union:// while(top)"
//                      << " earl:" << earl
//                      << endl;
//                 int colonpos      = earl.find(":");
//                 if( colonpos == string::npos )
//                     break;

//                 // For finding the start of the 3rd and on URL we need to skip one colon
//                 if( !UnionWrapper_urllist.empty() )
//                     colonpos      = colonpos + earl.substr( colonpos+1 ).find(":");
//                 if( colonpos == string::npos )
//                     break;
                    
//                 string tmp        = earl.substr( 0, colonpos );
//                 int secondURLpos  = tmp.rfind("/");
//                 if( secondURLpos == string::npos )
//                 {
//                     if( !earl.empty() )
//                     {
//                         cerr << "Wrapping for union:// ending while()"
//                              << " earl:" << earl
//                              << " colonpos:" << colonpos
//                              << endl;
//                         FerrisURL x = FerrisURL::fromString( earl );
//                         UnionWrapper_urllist.push_back( x );
//                     }
//                     break;
//                 }
//                 string secondURLString = earl.substr( secondURLpos+1 );
            
//                 cerr << "Wrapping for union:// while()"
//                      << " earl:" << earl
//                      << " secondURLString:" << secondURLString
//                      << " firstURLString:" << earl.substr( 0, secondURLpos )
//                      << endl;

//                 FerrisURL x = FerrisURL::fromString( earl.substr( 0, secondURLpos ) );
//                 UnionWrapper_urllist.push_back( x );
//                 earl = secondURLString;
            }

            Resolve_FirstManyToOneURL_PostAssert( UnionWrapper_urllist, 1, unionprefix );
            
//             if( UnionWrapper_urllist.empty() )
//             {
//                 cerr << "no urls for union" << endl;
//             }

//             for( urllist_t::iterator uiter = UnionWrapper_urllist.begin();
//                  uiter != UnionWrapper_urllist.end(); ++uiter )
//             {
//                 cerr << "union:// list of contexts:" << uiter->getURL() << endl;
//             }
            
            urllist_t::iterator uiter = UnionWrapper_urllist.begin();
            u = *uiter;
            earl = u.getURL();
//            UnionWrapper_urllist.erase( uiter );
        }


        string setdiffprefix = "setdifference:";
        if( starts_with( earl, setdiffprefix ) )
        {
            SetDiffWrapper_Add = true;
            string ex;
            Resolve_AdjustForWrap( dirty_earl, earl, u, setdiffprefix, ex );

            while( true )
            {
                string nextu = Resolve_FirstManyToOneURL( earl, SetDiffWrapper_urllist );
                if( nextu.empty() )
                    break;

                FerrisURL x = FerrisURL::fromString( nextu );
                SetDiffWrapper_urllist.push_back( x );
            }

            Resolve_FirstManyToOneURL_PostAssert( SetDiffWrapper_urllist, 1, setdiffprefix );
            urllist_t::iterator uiter = SetDiffWrapper_urllist.begin();
            u = *uiter;
            earl = u.getURL();
        }

        string setintersectionprefix = "setintersection:";
        if( starts_with( earl, setintersectionprefix ) )
        {
            SetIntersectWrapper_Add = true;
            string ex;
            Resolve_AdjustForWrap( dirty_earl, earl, u, setintersectionprefix, ex );

            while( true )
            {
                string nextu = Resolve_FirstManyToOneURL( earl, SetIntersectWrapper_urllist );
                if( nextu.empty() )
                    break;
                
                FerrisURL x = FerrisURL::fromString( nextu );
                SetIntersectWrapper_urllist.push_back( x );
            }

            Resolve_FirstManyToOneURL_PostAssert( SetIntersectWrapper_urllist, 1, setintersectionprefix );
            urllist_t::iterator uiter = SetIntersectWrapper_urllist.begin();
            u = *uiter;
            earl = u.getURL();
        }


        string setsymdiffprefix = "setsymdifference:";
        if( starts_with( earl, setsymdiffprefix ) )
        {
            SetSymmetricDiffWrapper_Add = true;
            string ex;
            Resolve_AdjustForWrap( dirty_earl, earl, u, setsymdiffprefix, ex );

            while( true )
            {
                string nextu = Resolve_FirstManyToOneURL( earl, SetSymmetricDiffWrapper_urllist );
                if( nextu.empty() )
                    break;

//                 cerr << "Wrapping for symdiff:// while(after FirstManyToOneURL)"
//                      << " earl:" << earl
//                      << " next:" << nextu
//                      << endl;
                
                FerrisURL x = FerrisURL::fromString( nextu );
                SetSymmetricDiffWrapper_urllist.push_back( x );
            }

            Resolve_FirstManyToOneURL_PostAssert( SetSymmetricDiffWrapper_urllist, 1, setsymdiffprefix );
            urllist_t::iterator uiter = SetSymmetricDiffWrapper_urllist.begin();
            u = *uiter;
            earl = u.getURL();
        }
        

        string diffprefix = "diff:";
        if( starts_with( earl, diffprefix ) )
        {
            DiffWrapper_Add = true;
            string ex;
            Resolve_AdjustForWrap( dirty_earl, earl, u, diffprefix, ex );

            while( true )
            {
                string nextu = Resolve_FirstManyToOneURL( earl, DiffWrapper_urllist );
                if( nextu.empty() )
                    break;

//                 cerr << "Wrapping for diff:// while(after FirstManyToOneURL)"
//                      << " earl:" << earl
//                      << " next:" << nextu
//                      << endl;
                
                FerrisURL x = FerrisURL::fromString( nextu );
                DiffWrapper_urllist.push_back( x );
            }

            Resolve_FirstManyToOneURL_PostAssert( DiffWrapper_urllist, 1, diffprefix );
            urllist_t::iterator uiter = DiffWrapper_urllist.begin();
            u = *uiter;
            earl = u.getURL();
        }

//         string urltrprefix = "urltr:";
//         if( starts_with( earl, urltrprefix ) )
//         {
//             URLTrWrapper_Add = true;
//             string ex;
//             Resolve_AdjustForWrap( dirty_earl, earl, u, diffprefix, ex );

//             while( true )
//             {
//                 string nextu = Resolve_FirstManyToOneURL( earl, URLTrWrapper_urllist );
//                 if( nextu.empty() )
//                     break;

// //                 cerr << "Wrapping for diff:// while(after FirstManyToOneURL)"
// //                      << " earl:" << earl
// //                      << " next:" << nextu
// //                      << endl;
                
//                 FerrisURL x = FerrisURL::fromString( nextu );
//                 URLTrWrapper_urllist.push_back( x );
//             }

//             Resolve_FirstManyToOneURL_PostAssert( URLTrWrapper_urllist, 2, diffprefix );
//             u = URLTrWrapper_urllist.back();
//             earl = u.getURL();
//         }
        
        
        
        
        
        LG_FACTORY_D << "Resolve() dirty_earl:" << dirty_earl << endl;
        LG_FACTORY_D << "Resolve()       earl:" << earl << endl;
        LG_FACTORY_D << "Resolve()          u:" << u << endl;
        LG_FACTORY_D << "Resolve()         rs:" << rs << endl;

        fh_context ret = 0;

//         cerr << "Resolve() internal scheme:" << u.getInternalFerisScheme()
//              << " path:" << u.getPath()
//              << endl;

        //
        // For XPath resolution we want to pass the expression right
        // to the xpath:// handler as one string rather than try to
        // parse it into a path ourself.
        //
        if( u.getInternalFerisScheme() == "xpath" && !u.getPath().empty() )
        {
            FerrisURL u2 = FerrisURL::fromString( earl );
//             cerr << "Resolve(xpath) internal scheme:" << u2.getInternalFerisScheme()
//                  << " path:" << u2.getPath()
//                  << " earl:" << earl
//                  << endl;
            
            fh_context xroot = RootContextFactory( u.getInternalFerisScheme(),
                                                   "/",
                                                   "",
                                                   rs ).resolveContext(rs);
            ret = xroot->getSubContext( u.getPath() );
        }
        else
        {
//             cerr << "Resolve( non xpath ) scheme:" << u.getInternalFerisScheme()
//                  << " path:" << u.getPath()
//                  << endl;

            static const gchar* LIBFERRIS_ENABLE_ATTRIBUTE_RESOLUTION_ENV =
                g_getenv ("LIBFERRIS_ENABLE_ATTRIBUTE_RESOLUTION");
            bool enableAttributeResolution = rs & ENABLE_ATTRIBUTE_RESOLUTION
                || LIBFERRIS_ENABLE_ATTRIBUTE_RESOLUTION_ENV;

            string eaname = "";
            string path = u.getPath();
            if( enableAttributeResolution )
            {
                int ampos = path.rfind("@");
                if( string::npos != ampos )
                {
                    LG_FACTORY_D << "path:" <<  path << endl;
                    eaname = path.substr(ampos+1);
                    path   = path.substr(0,ampos);
                }
            }
            
            ret = RootContextFactory( u.getInternalFerisScheme(),
                                      "/",
                                      path,
                                      rs ).resolveContext(rs);
//            DEBUG_dumpcl("Resolve() after real work" );

            
            if( enableAttributeResolution && !eaname.empty() )
            {
                stringstream earlss;
                earlss << "branchfs-attributes:"
                       << ret->getURLScheme() << "/" << ret->getDirPath()
                       << "/" << eaname
                       << flush;
                LG_FACTORY_D << "brachfs url:" << earlss.str() << endl;
                ret = Resolve( earlss.str() );
            }
        }
        

//         cerr << "Resolve() add cache:" << CacheWrapper_Add
//              << " ED:" << CacheWrapper_extraData
//              << " earl:" <<  earl
//              << endl;

        if( FilterWrapper_Add )
        {
            ret = Factory::MakeFilteredContext( ret, FilterWrapper_FilterObject );

//             typedef list<fh_context> clist_t;
//             clist_t clist;
//             {
//                 fh_context tc = ret;
//                 while( tc->isParentBound() )
//                 {
//                     clist.push_back( tc );
//                     tc = tc->getParent();
//                 }
//             }
            
//             fh_context c  = getRootOfContext( ret );
//             fh_context fc = Factory::MakeFilteredContext( c, FilterWrapper_FilterObject );

//             for( clist::reverse_iterator iter = clist.rbegin(); iter != clist.rend(); ++iter )
//             {
//                 fc = fc->getSubContext( iter->getDirName() );
//             }
        }
        

        if( SortingWrapper_Add )
            ret = Factory::MakeSortedContext( ret, SortingWrapper_sorter );

        if( InheritEAWrapper_Add )
            ret = Factory::makeInheritingEAContext( ret );
        
        if( CacheWrapper_Add )
            ret = Factory::MakeCachedContext( ret, CacheWrapper_extraData );

        if( UnionWrapper_Add )
        {
            std::list< fh_context > unionContexts;
            for( urllist_t::iterator uiter = UnionWrapper_urllist.begin();
                 uiter != UnionWrapper_urllist.end(); ++uiter )
            {
                unionContexts.push_back( Resolve( uiter->getURL() ) );
            }
            fh_context parent = ret->getParent();
            ret = Factory::MakeUnionContext( parent, unionContexts );
        }

        if( SetDiffWrapper_Add )
        {
            std::list< fh_context > clist;
            for( urllist_t::iterator uiter = SetDiffWrapper_urllist.begin();
                 uiter != SetDiffWrapper_urllist.end(); ++uiter )
            {
                clist.push_back( Resolve( uiter->getURL() ) );
            }
            fh_context parent = ret->getParent();
            ret = Factory::MakeSetDifferenceContext( parent, clist );
        }

        if( SetIntersectWrapper_Add )
        {
            std::list< fh_context > clist;
            for( urllist_t::iterator uiter = SetIntersectWrapper_urllist.begin();
                 uiter != SetIntersectWrapper_urllist.end(); ++uiter )
            {
                clist.push_back( Resolve( uiter->getURL() ) );
            }
            fh_context parent = ret->getParent();
            ret = Factory::MakeSetIntersectionContext( parent, clist );
        }

        if( SetSymmetricDiffWrapper_Add )
        {
            std::list< fh_context > clist;
            for( urllist_t::iterator uiter = SetSymmetricDiffWrapper_urllist.begin();
                 uiter != SetSymmetricDiffWrapper_urllist.end(); ++uiter )
            {
                clist.push_back( Resolve( uiter->getURL() ) );
            }
            fh_context parent = ret->getParent();
            ret = Factory::MakeSetSymmetricDifferenceContext( parent, clist );
        }

        if( DiffWrapper_Add )
        {
            std::list< fh_context > clist;
            for( urllist_t::iterator uiter = DiffWrapper_urllist.begin();
                 uiter != DiffWrapper_urllist.end(); ++uiter )
            {
                clist.push_front( Resolve( uiter->getURL() ) );
            }
            fh_context parent = ret->getParent();
            ret = Factory::MakeDiffContext( parent, clist );
        }

//         if( URLTrWrapper_Add )
//         {
//             std::list< fh_context > clist;
//             for( urllist_t::iterator uiter = URLTrWrapper_urllist.begin();
//                  uiter != URLTrWrapper_urllist.end(); ++uiter )
//             {
//                 clist.push_front( Resolve( uiter->getURL() ) );
//             }
//             fh_context parent = ret->getParent();
//             ret = Factory::MakeDiffContext( parent, clist );
//         }

        
        
//         cerr << "Resolve(end) add cache:" << CacheWrapper_Add
//              << " ED:" << CacheWrapper_extraData
//              << " earl:" <<  earl
//              << " ret:" << ret->getURL()
//              << " ret.sz:" << ret->SubContextCount()
//              << endl;
        
        return ret;
    }


//     namespace Factory
//     {

//         fh_context Resolve( ConfigLocation cl, std::string extrapath )
//         {
//             fh_stringstream ss;
//             switch( cl )
//             {
//             case CONFIGLOC_EVENTBIND:
//                 ss << "~/.ferris/eventbind" << extrapath;
//                 break;
            
//             case CONFIGLOC_MIMEBIND:
//                 ss << "~/.ferris/mimebind" << extrapath;
//                 break;
            
//             case CONFIGLOC_APPS:
//                 ss << "~/.ferris/apps" << extrapath;
//                 break;
            
//             case CONFIGLOC_ICONS:
//                 ss << "~/.ferris/icons" << extrapath;
//                 break;
//             }

//             if( tostr(ss).length() )
//             {
//                 return ::Ferris::Resolve( tostr(ss) );
//             }
        
//             ss << "Unknown config location:" << (int)cl << endl;
//             Throw_UnknownConfigLocation( tostr(ss), 0 );
//         }
    
//         fh_context ResolveMime( std::string majort, std::string minort )
//         {
//             fh_stringstream ss;
//             ss << "/" << majort << "/" << minort;
//             return Resolve( CONFIGLOC_MIMEBIND, tostr(ss) );
//         }

//         fh_context ResolveIcon( std::string s )
//         {
//             fh_stringstream ss;
//             ss << "/" << s;
//             return Resolve( CONFIGLOC_ICONS, tostr(ss) );
//         }
    
    
//     };





// fh_context Resolve( const std::string& module,
//                     const std::string& root,
//                     const std::string& path )
// {
//     return RootContextFactory( module, root, path ).resolveContext();
// }

// static fh_context getDotFerris()
// {
//     fh_stringstream rss;
//     rss << Shell::getHomeDirPath_nochecks() << "/.ferris";
//     cerr << "getDotFerris() rss:" << tostr(rss) << endl;
//     return Resolve( "Native", "/", tostr(rss) );
// }

// static void unlinkDotFerrisFile( const std::string& s )
// {
//     fh_stringstream rss;
//     rss << Shell::getHomeDirPath_nochecks() << "/.ferris/" << s;
//     cerr << "unlink() :" << tostr(rss) << endl;
//     unlink( tostr(rss).c_str() );
// }

// const string MEDALLION_TEMP_NAME = "medallion_temp.xml";

// fh_context fromMedallion( std::string medstr )
// {
//     fh_context dfc = getDotFerris();

//     LG_FACTORY_D << "fromMedallion(string) medstr:" << medstr << endl;
//     LG_FACTORY_D << "dfc path:" << dfc->getDirPath() << endl;
    
//     fh_mdcontext md = new f_mdcontext();
//     md->setAttr( "CreateObjectType", "File" );
//     unlinkDotFerrisFile( MEDALLION_TEMP_NAME );
//     fh_context medc = dfc->createSubContext( MEDALLION_TEMP_NAME, md );

//     LG_FACTORY_D << "medc path:" << medc->getDirPath() << endl;
    
//     {
//         fh_iostream tcss = medc->getIOStream();
//         tcss << medstr;
//     }

//     return fromMedallion( medc );
// }

// fh_context fromMedallion( fh_context c )
// {
//     LG_FACTORY_D << "fromMedallion(c) c.path:" << c->getDirPath() << endl;

//     string s;

//     while( c->getDirName() != "medallion" )
//     {
//         c = c->getSubContext( "medallion" );
//     }
    

//     string rccName = getStrAttr( c, "medrootcontext", "");
//     string root    = getStrAttr( c, "medroot", "");
//     string path    = getStrAttr( c, "medpath", "");
    
//     LG_FACTORY_D << "rootcontext:" << rccName << endl;
//     LG_FACTORY_D << "       root:" << root << endl;
//     LG_FACTORY_D << "       path:" << path << endl;
//     sleep(3);

//     return Resolve( rccName, root, path );
    
                                   
// //     string s;
// //     fh_istream iss = c->getIStream();
// //     fh_stringstream ss;

// //     std::copy( std::istreambuf_iterator<char>(iss),
// //                std::istreambuf_iterator<char>(),
// //                std::ostreambuf_iterator<char>(ss));

// //     return fromMedallion( tostr(ss) );
// }


/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

    static unsigned long globalDisableExpansion = 0;

    const fh_rex& hasShellGlobRegex()
    {
        static const fh_rex rex = toregexh( "(\\*|\\?|\\[|\\]|[?*+!@]\\()" );
        return rex;
    }
    
    
    static stringlist_t& expandShellGlobs( stringlist_t& ret,
                                           stringlist_t& splitPath,
                                           stringlist_t::iterator iter,
                                           stringlist_t::iterator& e,
                                           fh_context c )
    {
        if( iter == e )
        {
            LG_GLOB_D << "expandShellGlobs(iter==e) c:" << c->getURL() << endl;
            ret.push_back( c->getURL() );
            return ret;
        }

        LG_GLOB_D << "expandShellGlobs(called)    c:" << c->getURL() << endl;
        LG_GLOB_D << "expandShellGlobs(called) iter:" << *iter << endl;
        
        stringlist_t::iterator iternext = iter;
        ++iternext;
        
        if( !regex_search( *iter, hasShellGlobRegex(), boost::match_any ) )
        {
            LG_GLOB_D << "expandShellGlobs(no glob)  c:" << c->getURL() << endl;
            fh_context nextc = 0;
            try
            {
               nextc = c->getSubContext( *iter );
            }
            catch( exception& e )
            {
               c->read(true);
               nextc = c->getSubContext( *iter );
               if( !nextc )
                  throw;
            }
            LG_GLOB_D << "expandShellGlobs(no glob) nc:" << nextc->getURL() << endl;
            return expandShellGlobs( ret, splitPath, iternext, e, nextc );
        }

        if( *iter == "*" )
            c->read();
        bool haveFoundMatch = false;
        Context::SubContextNames_t& scn = c->getSubContextNames();
        Context::SubContextNames_t::iterator si = scn.begin();
        Context::SubContextNames_t::iterator se = scn.end();
        if( si == se ) 
        {
           c->read();
           scn = c->getSubContextNames();
           si = scn.begin();
           se = scn.end();
        }
        string pattern = *iter;
        int flags = FNM_PERIOD;
        LG_GLOB_D << "expandShellGlobs(si==se):" << (si==se) << " pattern:" << pattern << endl;
        
        for( ; si != se; ++si )
        {
            string filename = *si;
            int rc = fnmatch( pattern.c_str(), filename.c_str(), flags );
            if( !rc )
            {
                LG_GLOB_D << "expandShellGlobs(globm)  c:" << c->getURL() << endl;
                fh_context nextc = c->getSubContext( filename );
                LG_GLOB_D << "expandShellGlobs(globm) nc:" << nextc->getURL() << endl;

                // If we have something we can't decend into
                // and we have more path to go then this can't be a match
                if( !isTrue( getStrAttr( nextc, "is-dir", "false" )))
                {
                    if( iternext != e )
                        continue;
                }
                haveFoundMatch = true;
                expandShellGlobs( ret, splitPath, iternext, e, nextc );
            }
            else if( rc == FNM_NOMATCH )
            {}
            else
            {
                LG_GLOB_D << "Error globbing at:" << *iter << endl;
                return ret;
            }
        }

        //
        // allow something like xsltfs://foo/bar?params/subdir1
        // to resolve bar?params as the directory name
        //
        if( !haveFoundMatch )
        {
//             if( starts_with( c->getURL(), "xsltfs" ) )
//             {
//             }
            
            LG_GLOB_D << "expandShellGlobs(no match)  c:" << c->getURL() << endl;
            fh_context nextc = c->getSubContext( *iter );
            LG_GLOB_D << "expandShellGlobs(no match) nc:" << nextc->getURL() << endl;
            return expandShellGlobs( ret, splitPath, iternext, e, nextc );
        }
        
        return ret;
    }
    
    
    stringlist_t& expandShellGlobs( stringlist_t& ret, const std::string& s )
    {
        bool DisableExpansion = globalDisableExpansion;

        LG_GLOB_D << "expandShellGlobs(0) DisableExpansion:" << DisableExpansion << endl;

        static bool LIBFERRIS_DISABLE_GLOBBING = false;
        static bool LIBFERRIS_DISABLE_GLOBBING_SETUP = false;
        if( !LIBFERRIS_DISABLE_GLOBBING_SETUP )
        {
            LIBFERRIS_DISABLE_GLOBBING_SETUP = true;
            if( const gchar* p = g_getenv ("LIBFERRIS_DISABLE_GLOBBING") )
            {
                if( strlen(p) && p[0]=='1' )
                {
                    LIBFERRIS_DISABLE_GLOBBING = true;
                }
            }
        }
        
        if( !DisableExpansion )
        {
            if( starts_with( s, "xpath:" ) )
                DisableExpansion = true;
            if( starts_with( s, "http:" ) )
                DisableExpansion = true;
        }
        
        LG_GLOB_D << "expandShellGlobs(1) DisableExpansion:" << DisableExpansion << endl;
        
        // if there are no special characters then we are already done.
        if( !DisableExpansion )
        {
            if( !regex_search( s, hasShellGlobRegex(), boost::match_any ) )
            {
                DisableExpansion = true;
            }
        }
        
        LG_GLOB_D << "expandShellGlobs(2) DisableExpansion:" << DisableExpansion << endl;

        // see if the user has settings to not expand this URL
        if( !DisableExpansion )
        {
            if( isTrue( getEDBString( FDB_GENERAL,
                                      CFG_GLOB_SKIP_FILE_URLS_K,
                                      CFG_GLOB_SKIP_FILE_URLS_DEFAULT )))
            {
//                 // disable for anything that is absolute, relative or a file://
//                 // url
//                 static const fh_rex rex = toregexh( "^[A-z-]+:(?<!file:)" );

                // disable for anything that is absolute, relative.
                // anything:// or a file:// are to be handled by libferris
                static const fh_rex rex = toregexh( "^[A-z-]+:" );
                if( !regex_search( s, rex, boost::match_any ) )
                {
                    DisableExpansion = true;
                }
            }
        }

        LG_GLOB_D << "expandShellGlobs(3) DisableExpansion:" << DisableExpansion << endl;
        
        if( !DisableExpansion )
        {
            static fh_rex rex = 0;
            static bool sl_v = true;
            if( sl_v )
            {
                sl_v = false;
                stringlist_t sl;
                
                string d = getEDBString( FDB_GENERAL,
                                         CFG_GLOB_SKIP_REGEX_LIST_K,
                                         CFG_GLOB_SKIP_REGEX_LIST_DEFAULT );
                LG_GLOB_D << "expandShellGlobs() disable regex:" << d << endl;
                if( !d.empty() )
                {
                    Util::parseNullSeperatedList( d, sl );
                    rex = toregexh( sl );
                }
            }

            if( rex )
            {
                LG_GLOB_D << "expandShellGlobs() s:" << s << endl;
                LG_GLOB_D << "expandShellGlobs() have-disable-regex:" << rex << endl;
                if( regex_search( s, rex, boost::match_any ) )
                {
                    DisableExpansion = true;
                }
            }
        }
        

        LG_GLOB_D << "expandShellGlobs(4) DisableExpansion:" << DisableExpansion << endl;
        
        
        if( DisableExpansion )
        {
            LG_GLOB_I << "DisableExpansion is set... s:" << s << endl;
            ret.push_back( s );
        }
        else
        {
            LG_GLOB_I << "expanding globs for... s:" << s << endl;
            string path = s;
            fh_context c = Resolve("file://");

            if( !path.empty() && path.find("/") == string::npos )
            {
                string cwd = Shell::getCWDString();
                path = cwd + "/" + path;
            }
            
            int pos = path.find(":");
            if( pos != string::npos )
            {
                path = s.substr( pos+1 );
                string scheme = s.substr( 0, pos );
                LG_GLOB_D << "scheme:" << scheme << "  path:" << path << endl;
                c = Resolve( scheme + "://" );
            }
            
            stringlist_t splitPath;
            Util::parseSeperatedList( path, splitPath, '/' );
            if( splitPath.empty() )
            {
                LG_GLOB_W << "Warning: attempt to resolve an empty path" << endl;
            }
            else
            {
                stringlist_t::iterator si = splitPath.begin();
                stringlist_t::iterator e  = splitPath.end();
                expandShellGlobs( ret, splitPath, si, e, c );
            }
        }
        
        
        return ret;
    }

    struct ::poptOption* getExpandShellGlobsPopTable()
    {
        static struct poptOption optionsTable[] =
            {
                { "ferris-glob-disable", 0, POPT_ARG_NONE, &globalDisableExpansion, 0,
                  "Don't perform any special glob expansion", "" },
                POPT_TABLEEND
            };
        return optionsTable;
    }
    
    stringlist_t& expandShellGlobs( stringlist_t& ret, poptContext& optCon )
    {
        while( const char* RootNameCSTR = poptGetArg(optCon) )
        {
            string RootName = RootNameCSTR;
            expandShellGlobs( ret, RootName );
        }
        return ret;
    }
    
    
    

};
