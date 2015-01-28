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

    $Id: RPMContext.cpp,v 1.5 2010/09/24 21:30:56 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <RPMContext_private.hh>
#include <RPM_private.hh>
#include <Ferris/Resolver_private.hh>
#include <Ferris/Trimming.hh>
#include <ValueRestorer.hh>

using namespace std;

namespace Ferris
{
    extern std::string adjustRecommendedEAForDotFiles( Context* c, const std::string& s );
    
    void
    RPMRootContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
    }
    
    std::string
    RPMRootContext::getGroupName()
    {
        string p = getDirPath();

        PrefixTrimmer trimmer;
        trimmer.push_back( "/" );
        p = trimmer( p );

        PostfixTrimmer ptrimmer;
        ptrimmer.push_back( "/" );
        p = ptrimmer( p );

        return p;
    }

//     static void showTree( fh_context c )
//     {
//         try
//         { c->read(); }
//         catch(...)
//         {}

//         if( c->begin() == c->end() )
//             return;
        
//         cerr << "--- enter c:" << c->getURL() << " --- " << endl;
//         for( Context::iterator ci = c->begin(); ci != c->end(); ++ci )
//         {
//             cerr << " child:" << (*ci)->getURL()
//                  << " nameEA:" << getStrAttr( *ci,"name","<>" ) << endl;
//         }
//         for( Context::iterator ci = c->begin(); ci != c->end(); ++ci )
//         {
//             showTree( *ci );
//         }
//         cerr << "--- exit c:" << c->getURL() << " --- " << endl;
//     }
    
    void
    RPMRootContext::priv_read()
    {
        static bool addingAlready = false;
        if( addingAlready )
        {
            emitExistsEventForEachItemRAII _raii1( this );
            return;
        }
        Util::ValueRestorer< bool > _obj( addingAlready, true );
        EnsureStartStopReadingIsFiredRAII _raii1( this );
        AlreadyEmittedCacheRAII _raiiec( this );
        
        {
            rpmdb db = get_rpmdb();
            rpmdb_releaser dum1( db );

            if( this == getBaseContext() && SubContextCount() == 0 )
            {
                // create a tree for the sorted unique group names
                fh_context child = 0;

                stringlist_t grlist;
            
                rpmdbMatchIterator mi = rpmdbInitIterator(db, (rpmTag)RPMDBI_PACKAGES, 0, 0 );
                for( Header header; header = rpmdbNextIterator(mi); )
                {
                    char *group;
                    rpmHeaderGetEntry(header, RPMTAG_GROUP, NULL, (void **)&group, NULL);
                    string sg = group;
                    PostfixTrimmer trimmer;
                    trimmer.push_back( "\n" );
                    sg = trimmer( sg );

                    grlist.push_back( sg );
                }
                rpmdbFreeIterator( mi );

                grlist.sort();
                grlist.erase( unique( grlist.begin(), grlist.end() ), grlist.end() );
            
                for( stringlist_t::iterator si = grlist.begin(); si != grlist.end(); ++si )
                {
                    string xdn = *si;
                    ensureContextCreated( xdn );
                }
            }
            if( this != getBaseContext() && SubContextCount() == 0 )
            {
                rpmts ts = NULL;
                rpmdbMatchIterator mi = rpmdbInitIterator(db, RPMTAG_GROUP, getGroupName().c_str(), 0);

                for( Header header; header = rpmdbNextIterator(mi); )
                {
                    const char *name, *version, *release;
                    rpmHeaderGetEntry(header, RPMTAG_NAME, NULL, (void **)&name, NULL);
                    rpmHeaderGetEntry(header, RPMTAG_VERSION,NULL, (void **)&version,NULL);
                    rpmHeaderGetEntry(header, RPMTAG_RELEASE,NULL, (void **)&release,NULL);
                    LG_CTX_D << "  name:" << name << " version:" << version
                             << " release:" << release << endl;

                    fh_stringstream rdnss;
                    rdnss << name << "-" << version << "-" << release;
                    string rdn = tostr(rdnss);

                    fh_context child = new RPMPackageContext( this, rdn );
                    Insert( GetImpl(child), false, false );

                    LG_CTX_D << "added pkg:" << child->getURL() << endl;
                }
                rpmdbFreeIterator( mi );
            }
        }
    }

    
    
        
    RPMRootContext::RPMRootContext()
        :
        _Base( 0, "/" )
    {
        createStateLessAttributes();
    }
    
    RPMRootContext::~RPMRootContext()
    {
    }
    
    fh_context
    RPMRootContext::createSubContext( const std::string& rdn, fh_context md )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        fh_stringstream ss;
        ss << "rpm:// directory can not have new items created in this way" << endl;
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), this );
    }
    

    void
    RPMRootContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }


    class FERRISEXP_DLLLOCAL RPMRootContext_RootContextDropper
        :
        public RootContextDropper
    {
    public:
        RPMRootContext_RootContextDropper()
            {
                ImplementationDetail::appendToStaticLinkedRootContextNames("rpm");
                RootContextFactory::Register( "rpm", this );
            }

        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
            {
                static fh_context c = 0;
                if( !isBound(c) )
                {
                    c = new RPMRootContext();
                }
                return c;
            }
    };
    static RPMRootContext_RootContextDropper ___RPMRootContext_static_init;
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    void
    RPMPackageContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
    }

    string
    RPMPackageContext::getPackageName()
    {
        string s = getDirName();
        return s.substr( 0, s.find("-"));
    }
    
    void
    RPMPackageContext::read( bool force )
    {
        if( m_readingDir )
            return;

        Util::ValueRestorer< bool > dummy_x1( m_readingDir, true );
        priv_read();
    }
    
    void
    RPMPackageContext::priv_read()
    {
        if( !empty() )
        {
            emitExistsEventForEachItemRAII _raii1( this );
            return;
        }

        LG_CTX_D << "priv_read() url:" << getURL() << endl;
//        BackTrace();

        typedef Loki::SmartPtr< RootContextFactory, 
            Loki::RefLinked, 
            Loki::AllowConversion, 
            FerrisLoki::FerrisExSmartPointerChecker, 
            Loki::DefaultSPStorage >  fh_rcf;
        fh_rcf rfac = new RootContextFactory();
        rfac->setContextClass( "rpm" );
        rfac->AddInfo( RootContextFactory::ROOT, "/" );
                
        rpmts ts = NULL;
        rpmdb db = get_rpmdb();
        rpmdb_releaser dum1( db );
        rpmdbMatchIterator mi = rpmdbInitIterator(db, RPMTAG_NAME, getPackageName().c_str(), 0);

        EnsureStartStopReadingIsFiredRAII _raii1( this );

        
        for( Header header; header = rpmdbNextIterator(mi); )
        {
            const char *name, *version, *release;
            rpmHeaderGetEntry(header, RPMTAG_NAME, NULL, (void **)&name, NULL);
            rpmHeaderGetEntry(header, RPMTAG_VERSION,NULL, (void **)&version,NULL);
            rpmHeaderGetEntry(header, RPMTAG_RELEASE,NULL, (void **)&release,NULL);
            LG_CTX_D << "  name:" << name << " version:" << version
                     << " release:" << release << endl;

            rpmfi fi = rpmfiNew( ts, header, RPMTAG_BASENAMES, 1 );
            LG_CTX_D << "fi.count:" << rpmfiFC( fi ) << endl;
            for( rpmfiInit( fi, 0 ); rpmfiNext(fi) >= 0; )
            {
                try
                {
                    string path = rpmfiFN(fi);
                    LG_CTX_D << "RPMPATH:" << path << endl;
                    fh_context delegate = Resolve( path );
                    
                    //
                    // Make sure that the child directory entries exist and
                    // place the delegate wrapping rpmContext at the right
                    // place in the tree.
                    //
                    typedef std::list< fh_context > clist_t;
                    clist_t cl;
                    if( delegate->isParentBound() )
                    {
                        for( fh_context p = delegate->getParent(); p; p = p->getParent() )
                        {
                            LG_CTX_D << "parent:" << p->getDirPath()
                                     << " name:" << p->getDirName()
                                     << endl;
                            if( p->getDirPath() != "/" )
                                cl.push_front( p );
                            if( !p->isParentBound() )
                                break;
                        }
                    }

                    //
                    // make parent directories
                    // we cache the parent for the new context in 'p' because
                    // we are walking from least to most specific directory,
                    // ie, the root outwards
                    //
                    fh_context p = this;
                    for( clist_t::iterator ci = cl.begin(); ci != cl.end(); ++ci )
                    {
                        LG_CTX_D << "making parent context:" << (*ci)->getURL() << endl;
                                
                        try
                        {
                            string path = (*ci)->getDirPath();
                            PrefixTrimmer trimmer;
                            trimmer.push_back( "/" );
                            path = trimmer( path );
                            path = "./" + path;
                            fh_context rc = getRelativeContext( path, rfac );
                            LG_CTX_D << "getRelativeContext(1) path:" << path
                                     << " rc:" << rc->getURL()
                                     << " rc.path:" << rc->getDirPath()
                                     << " this:" << getURL()
                                     << endl;
                                    
                            p = rc;
                        }
                        catch( exception& e )
                        {
                            LG_CTX_D << "making parent context:" << (*ci)->getURL()
                                     << " name:" << (*ci)->getDirName()
                                     << " p:" << p->getURL()
                                     << " p.path:" << p->getDirPath()
                                     << " e:" << e.what()
                                     << endl;
                            fh_context child = new RPMContext( p, *ci );

                            p->Insert( GetImpl(child), false, false );
                            p = child;
                            LG_CTX_D << "MADE parent context:" << (*ci)->getURL()
                                 << " name:" << (*ci)->getDirName()
                                 << " p:" << p->getURL()
                                 << " p.path:" << p->getDirPath()
                                 << endl;
                        }
                    }

                    // make the RPMContext itself and insert it at the correct
                    // place in the tree
                    {
                        string path = cl.back()->getDirPath();
                        PrefixTrimmer trimmer;
                        trimmer.push_back( "/" );
                        path = trimmer( path );

                        p = getRelativeContext( path, rfac );
                        LG_CTX_D << "making delegate context:" << delegate->getURL()
                             << " p:" << p->getURL()
                             << " path:" << path
                             << " is-bound:" << p->isSubContextBound( delegate->getDirName() )
                             << endl;

                        fh_context child = new RPMContext( p, delegate );
                        p->Insert( GetImpl(child), false, false );
                    }
                            
                }
                catch( exception& e )
                {
                    cerr << " ...e:" << e.what() << endl;
                }

//                         static int xxcount = 0;
//                         if( xxcount >= 10 )
//                             break;
//                         ++xxcount;
                        
            }
            rpmfiFree( fi );
        }
        rpmdbFreeIterator( mi );
    }

    
    fh_stringstream
    RPMPackageContext::SL_getRPMHeaderString( RPMPackageContext* c, int_32 tag )
    {
        rpmdb db = get_rpmdb();
        rpmdb_releaser dum1( db );
    
        fh_stringstream ss;
        const char *cstr;

        rpmTag match_type = RPMTAG_BASENAMES;
        rpmdbMatchIterator mi = rpmdbInitIterator(db, RPMTAG_NAME,
                                                  c->getPackageName().c_str(), 0);
        if( mi )
        {
            Header rh = rpmdbNextIterator(mi);
            if( rh )
            {
                rpmHeaderGetEntry( rh, tag, NULL, (void **)&cstr, NULL);
                if( cstr )
                    ss << cstr;
            }
        }
        return ss;
    }
    
    fh_stringstream
    RPMPackageContext::SL_getRPMPackage( RPMPackageContext* c,const std::string&,EA_Atom*)
    {
        return SL_getRPMHeaderString( c, RPMTAG_NAME );
    }
    
    fh_stringstream
    RPMPackageContext::SL_getRPMVersion( RPMPackageContext* c,const std::string&,EA_Atom*)
    {
        return SL_getRPMHeaderString( c, RPMTAG_VERSION );
    }
    
    fh_stringstream
    RPMPackageContext::SL_getRPMRelease( RPMPackageContext* c,const std::string&,EA_Atom*)
    {
        return SL_getRPMHeaderString( c, RPMTAG_RELEASE );
    }
    
    fh_stringstream
    RPMPackageContext::SL_getRPMInfoURL( RPMPackageContext* c,const std::string&,EA_Atom*)
    {
        return SL_getRPMHeaderString( c, RPMTAG_URL );
    }
    
    
    RPMPackageContext::RPMPackageContext( const fh_context& parent,
                                          const std::string& rdn )
        :
        _Base( GetImpl( parent ), rdn ),
        m_readingDir( false )
    {
        if( isStateLessEAVirgin() )
        {
#define SLEA  tryAddStateLessAttribute         
            SLEA( "rpm-package",       &_Self::SL_getRPMPackage,    XSD_BASIC_STRING );
            SLEA( "rpm-version",       &_Self::SL_getRPMVersion,    XSD_BASIC_STRING );
            SLEA( "rpm-release",       &_Self::SL_getRPMRelease,    XSD_BASIC_STRING );
            SLEA( "rpm-info-url",      &_Self::SL_getRPMInfoURL,    FXD_URL_IMPLICIT_RESOLVE );
#undef SLEA
            
            createStateLessAttributes( true );
        }
    }
    
    RPMPackageContext::~RPMPackageContext()
    {
    }
    
    fh_context
    RPMPackageContext::createSubContext( const std::string& rdn, fh_context md )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        fh_stringstream ss;
        ss << "rpm:// directory can not have new items created in this way" << endl;
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), this );
    }
    
    void
    RPMPackageContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }

    RPMPackageContext*
    RPMPackageContext::priv_CreateContext( Context* parent, std::string rdn )
    {
        return 0;
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    
    void
    RPMContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
    }
    
    void
    RPMContext::read( bool )
    {
        NumberOfSubContexts=getItems().size();
        HaveReadDir=1;

        emitExistsEventForEachItemRAII _raii1( this );
    }

    RPMContext::RPMContext( const fh_context& parent, const fh_context& delegate )
        :
        _Base( parent, delegate, false )
    {
        createStateLessAttributes();
        LG_CTX_D << "RPMContext::RPMContext() url:" << getURL() << endl;
        
    }
    
    RPMContext::~RPMContext()
    {
    }
    
    fh_context
    RPMContext::createSubContext( const std::string& rdn, fh_context md  )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        fh_stringstream ss;
        ss << "rpm:// directory can not have new items created in this way" << endl;
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), this );
    }
    
    void
    RPMContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }

    std::string
    RPMContext::getDirPath() throw (FerrisParentNotSetError)
    {
        if( !isParentBound() )
            return _Base::getDirPath();

        return getParent()->getDirPath() + "/" + getDirName();
    }
    
    std::string
    RPMContext::getURL()
    {
        return "rpm://" + getDirPath();
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

};
