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

    $Id: Resolver_private.hh,v 1.7 2010/09/24 21:30:57 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Resolver.hh>
#include <Ferris/SM.hh>
#include <Ferris/MatchedEAGenerators.hh>
#include <Ferris/Ferris.hh>

#ifndef _ALREADY_INCLUDED_FERRIS_RESOLVER_PRIVATE_H_
#define _ALREADY_INCLUDED_FERRIS_RESOLVER_PRIVATE_H_

namespace Ferris
{

    struct FERRISEXP_GMODAPI FerrisURL
    {
        std::string scheme;
        std::string path;

        std::string getScheme();
        std::string getInternalFerisScheme();
        std::string getPath();

        std::string tostr();
        std::string getURL();

        static FerrisURL fromString( const std::string& s );
    };

    typedef fh_matcher fh_ommatcher;
    typedef std::list<fh_ommatcher> fh_ommatchers;
    
    class FERRISEXP_DLLLOCAL RootContextDropper
    {
    public:
        virtual fh_context Brew(RootContextFactory* rf)
            throw( RootContextCreationFailed ) = 0;

        /**
         * This returns a predicate that if a context matches against then
         * an overmounting may be performed using this plugin handler ontop
         * of the existing context.
         */
        virtual fh_ommatchers GetOverMountMatchers(RootContextFactory* rf)
            {
                fh_ommatchers ret;
                return ret;
            }

        /**
         * If a context is top level, like http:// ftp:// or file:// then it
         * should override this method and return true. The default is false.
         *
         * Note that this method will become a extern "C" funtion in plugin
         * factory libraries and if no such function is present then the default
         * is assumed.
         *
         * Top level context classes appear also under root:// which provides
         * a single top node for looking into many different URL schemes.
         */
        virtual bool isTopLevel()
            {
                return false;
            }

        /**
         * Some overmount formats can never support top level EA so trying
         * to overmount then to discover if an attribute is bound or not is
         * a waste of time. For example, a directory full of tar.gz files
         * with a few .anx and .ogg files the tar files will never be able
         * to support an "artist", "license" or "creator" EA to the top level
         * context by being overmounted.
         */
        virtual bool tryToOverMountToFindEA()
            {
                return false;
            }
        
        
    };


    class ContextVFS_RootContextDropper;
    class FerrisGPGSignaturesRootContext;
    class FerrisBranchRootContext;
    
/**
 *
 * Factory that bootstraps to context objects.
 *
 * <pre>
 * EXAMPLE USE
 * 
 * RootContextFactory fac;
 * fac.setContextClass( "ffilter" );
 * fac.AddInfo( RootContextFactory::ROOT, v );
 * fac.AddInfo( RootContextFactory::PATH, ""   );
 * fh_context c = fac.resolveContext( RootContextFactory::RESOLVE_EXACT );
 * </pre>
 */
    class FERRISEXP_GMODAPI RootContextFactory
    {
    public:

        typedef ::Ferris::ResolveStyle ResolveStyle;
        
    private:
        /**
         * What level of matching we are to perform
         */
        ResolveStyle Style;

    public:
    

        /**
         * Get how the resolution has been currently set.
         */
        ResolveStyle getResolveStyle()
            {
                return Style;
            }

        virtual ~RootContextFactory();
        
        /**
         * Path of the context we seek
         *
         * Data that can be added with AddInfo
         */
        static const std::string PATH;

        /**
         * Where to start resolving from within the context.
         *
         * Data that can be added with AddInfo
         */
        static const std::string ROOT;


        /**
         * If a remote resolution is required (a context on another machine)
         * then set this to the name/ip address of the remote machine
         *
         * Data that can be added with AddInfo
         */
        static const std::string SERVERNAME;
    
    protected:

        /**
         * We have a tight binding with the dropper for context://
         * and root:// because they both expose what context classes
         * are available as a filesystem
         */
        friend class ContextVFS_RootContextDropper;
        friend class RootContextVFS_RootContextDropper;
        friend struct RootRootContext;
        friend class FerrisGPGSignaturesRootContext;
        friend class FerrisBranchRootContext;
        
        /**
         * What class we are using to make context objects
         */
        std::string ContextClass;

        /**
         * User params given for the binding process
         *
         * @see PATH
         * @see ROOT
         */
        std::map< std::string, std::string > BindData;

        /**
         * Used to lookup a dropper from a context class name
         */
        typedef std::map< std::string, RootContextDropper* > Droppers_t;

        static Droppers_t& getDroppers();

        static void ensureGModuleFactoriesLoaded( Droppers_t& droppers );

        virtual fh_context priv_resolveContext_tryURIAsDirName( fh_context ctx,
                                                                const std::string& rdn,
                                                                const std::string& rest,
                                                                std::pair<std::string,std::string>& split );
        
        virtual fh_context priv_resolveContext(
            fh_context ctx,
            const std::string& rdn,
            const std::string& rest
            );

        /**
         * Cache of root context objects keyed on the name of the context class.
         */
        typedef std::map< std::string, fh_context > RootContextCache_t;
        static RootContextCache_t& getRootContextCache();

        fh_context BaseOverMountContext;
        void setBaseOverMountContext( fh_context c );

        /**
         * A struct that is ment to be created on the stack to set the
         * setBaseOverMountContext() to a given context and clear the
         * overmount context when the object is destroyed.
         *
         * INTERNAL FERRIS USE ONLY
         */
        struct BaseOverMountScope
        {
            RootContextFactory* f;
            BaseOverMountScope( RootContextFactory* _f, const fh_context& c )
                :
                f(_f)
                {
                    f->setBaseOverMountContext( c );
                }
        
            ~BaseOverMountScope()
                {
                    f->setBaseOverMountContext( 0 );
                }
        };

        fh_context findOverMounter_TryDropper( fh_context ctx,
                                               int followLinkTTL,
                                               const std::string&  dropperName,
                                               RootContextDropper* dropper,
                                               bool attemptingOverMountOnlyToFindEA );
    
    public:

        RootContextFactory();

        RootContextFactory( const std::string& module,
                            const std::string& root,
                            const std::string& path,
                            ResolveStyle s = RESOLVE_EXACT );
    
        void setContextClass( std::string cl ) throw( NoSuchContextClass );
        std::string getContextClass();

        void AddInfo( std::string key, std::string val );

        fh_context getRootContext() throw( RootContextCreationFailed );
        fh_context resolveContext_relative_file_path( ResolveStyle s = RESOLVE_EXACT );
        fh_context resolveContext( ResolveStyle s = RESOLVE_EXACT );

        // For package use only atm. Register a new vfs switch fs 
        static void Register( std::string CtxName, RootContextDropper* dropper );
        std::string getInfo( std::string key );

        /*
         * Used by context to find an overmounter.
         */
        fh_context findOverMounter( fh_context ctx,
                                    int followLinkTTL = 100,
                                    bool attemptingOverMountOnlyToFindEA = false );
        bool hasOverMounter( const fh_context& ctx );
    
        const fh_context& getBaseOverMountContext();


        static const std::string getRootContextClassName( fh_context c );
        static const std::string getRootContextClassName( const Context* c );

        static void pushAdditionalRootContextClassName( std::string n, Context* c );
        
    };

#define STATICLINKED_ROOTCTX_DROPPER( URISCHEME, KLASS )                \
    class FERRISEXP_DLLLOCAL KLASS ## _RootContextDropper               \
        :                                                               \
        public RootContextDropper                                       \
    {                                                                   \
    public:                                                             \
        KLASS ## _RootContextDropper()                                  \
            {                                                           \
                ImplementationDetail::appendToStaticLinkedRootContextNames( URISCHEME ); \
                RootContextFactory::Register( URISCHEME, this );        \
            }                                                           \
                                                                        \
        fh_context Brew( RootContextFactory* rf )                       \
            throw( RootContextCreationFailed )                          \
            {                                                           \
                static fh_context c = 0;                                \
                if( !isBound(c) )                                       \
                {                                                       \
                    c = new KLASS();                                    \
                }                                                       \
                return c;                                               \
            }                                                           \
    };                                                                  \
    static KLASS ## _RootContextDropper ___ ## KLASS ## _RootContextDropper;


};
#endif
