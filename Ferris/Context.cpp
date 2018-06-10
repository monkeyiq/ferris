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

    $Id: Context.cpp,v 1.26 2010/09/24 21:30:26 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Context.hh>
#include <Ferris.hh>
#include <Ferris_private.hh>
#include <Runner.hh>
#include <Iterator.hh>
#include <General.hh>
#include <Context_private.hh>
#include <Resolver_private.hh>
#include "ForwardEAIndexInterface.hh"
#include "Trimming.hh"

// Used in the union context for quick unions on filtered views
#include <FilteredContext_private.hh>

#include <unistd.h>

// stat()
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

#ifndef LIBFERRIS__USE_BOOST_MULTI_INDEX_FOR_STORING_SUBCONTEXTS
#define ITEMS_KEY_COMP( ITEMS ) ITEMS.key_comp()
#else
#define ITEMS_KEY_COMP( ITEMS ) ITEMS.get<ITEMS_T_BY_NAME_ORDERED_TAG>().key_comp()
#endif


namespace Ferris
{
//    FERRIS_SMARTPTR( SortedContext, fh_sortedContext );
    FERRIS_CTX_SMARTPTR( SortedContext, fh_sortedContext );

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


    /**
     * Keep appending version numbers to rdn until the context 'c'
     * can have a new item inserted with the rdn, return the changed
     * rdn.
     */
    string monsterName( const fh_context& c, const std::string& rdn )
    {
        return c->monsterName( rdn );
    }

    mtimeNotChangedChecker::mtimeNotChangedChecker()
        :
        m_old( 0 )
    {
    }
    bool
    mtimeNotChangedChecker::operator()( Context* c )
    {
        if( !c->getIsNativeContext() )
            return false;
        
        struct stat buf;
        int rc = stat( c->getDirPath().c_str(), &buf );
        if( rc != 0 )
        {
            return false;
        }

        time_t o = m_old;
        time_t n = buf.st_mtime;
        m_old = n;

        return o && o == n;
    }



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    Context*
    leafContext::priv_CreateContext( Context* parent, string rdn )
    {
        LG_CTX_ER << "leafContext::priv_CreateContext() Should never happen!" << endl;
        Throw_FerrisCanNotCreateLeafOfLeaf( "", this );
    }
    
    void
    leafContext::priv_read()
    {
        LG_CTX_D << "leafContext::priv_read() url:" << getURL() << endl;
        emitExistsEventForEachItemRAII _raii1( this );
    }

    leafContext::leafContext()
        :
        Context()
    {
    }

    leafContext::leafContext( Context* parent, std::string rdn )
        :
        Context( parent, rdn )
    {
    }
    
    leafContext::~leafContext()
    {
    }


////////////////////////


        std::string
        leafContextWithSimpleContent::priv_getRecommendedEA()
            {
                static string rea = "name,content";
                return rea;
            }
        
        ferris_ios::openmode
        leafContextWithSimpleContent::getSupportedOpenModes()
            {
                return
                    ios_base::in        |
                    ios_base::out       |
                    std::ios::trunc     |
                    ios_base::binary    ;
            }
        

        

        fh_istream
        leafContextWithSimpleContent::priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ret;
                if( !( m & std::ios::trunc ) )
                    return real_getIOStream( m );
                return ret;
            }

        

        
//         void
//         leafContextWithSimpleContent::OnStreamClosed( const std::string& s )
//             {
//             }

        void
        leafContextWithSimpleContent::priv_OnStreamClosed(
            fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
//                cerr << "priv_OnStreamClosed()" << endl;
//                 if( !(m & std::ios::out) )
//                     return;
                AdjustForOpenMode_Closing( ss, m, tellp );
                const string s = StreamToString(ss);
                OnStreamClosed( s );
            }
        



        fh_iostream
        leafContextWithSimpleContent::priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ret;
                if( !( m & std::ios::trunc ) )
                    ret = real_getIOStream( m );
                ret->getCloseSig().connect(
                    sigc::bind(
                        sigc::mem_fun(*this, &_Self::priv_OnStreamClosed ), m ));
//                cerr << "priv_getIOStream()" << endl;
                return ret;
            }
        



        
        leafContextWithSimpleContent::leafContextWithSimpleContent(
            Context* parent, std::string rdn )
            :
            leafContext( parent, rdn )
            {
            }
            
        leafContextWithSimpleContent::~leafContextWithSimpleContent()
            {
            }
    
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


StaticContentLeafContext::StaticContentLeafContext( Context* parent,
                                                    std::string rdn,
                                                    const std::string& content )
    :
    _Base( parent, rdn ),
    m_content( content )
{
    createStateLessAttributes();
    LG_CTX_D << "StaticContentLeafContext() rdn:" << rdn << endl
             << "content:" << content << endl << endl;
    
}
    
    StaticContentLeafContext::~StaticContentLeafContext()
    {
    }
    
    fh_istream
    StaticContentLeafContext::getIStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               std::exception)
    {
        fh_stringstream ret;
        ret << m_content;
        return ret;
    }

    void
    StaticContentLeafContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }
    
    
    

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



    FakeInternalContext*
    FakeInternalContext::priv_CreateContext( Context* parent, string rdn )
    {
        FakeInternalContext* ret = new FakeInternalContext( parent, rdn );
        return ret;
    }

    
    void
    FakeInternalContext::priv_read()
    {
        emitExistsEventForEachItemRAII _raii1( this );
    }

    FakeInternalContext::FakeInternalContext( Context* parent, const string& rdn )
        :
        Context()
    {
//         if( parent )
//             cerr << "FakeInternalContext::FakeInternalContext() parent:" << parent->getURL()
//                  << " rdn:" << rdn
//                  << endl;
        
        setContext( parent, rdn );

//         if( parent )
//             cerr << "FakeInternalContext::FakeInternalContext() parent:" << parent->getURL()
//                  << " dirName:" << getDirName()
//                  << endl;
    }

    FakeInternalContext::FakeInternalContext()
        :
        Context()
    {
    }

    FakeInternalContext::~FakeInternalContext()
    {
    }
    
    fh_fcontext
    FakeInternalContext::addNewChild( const string& rdn )
    {
        return priv_CreateContext( this, rdn );
    }

    fh_context
    FakeInternalContext::addNewChild( fh_context c )
    {
        const bool created = false;
        const bool emit = true;
        Insert( GetImpl(c), created, emit );
        return c;
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



    CreateMetaDataContext::CreateMetaDataContext()
    {
        setContext( 0, "" );
        setAttributesHaveBeenCreated();
//        cerr << "CreateMetaDataContext()" << endl;
    }

    
    CreateMetaDataContext::~CreateMetaDataContext()
    {
//        cerr << "DTOR!!! CreateMetaDataContext()" << endl;
    }


    void
    CreateMetaDataContext::setAttr( const std::string& rdn, const std::string& v )
    {
        addAttribute( rdn, v, FXD_BINARY );
    }

    
    void
    CreateMetaDataContext::setBody( std::string s )
    {
        Body = s;
    }

    fh_istream
    CreateMetaDataContext::getIStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               exception)
    {
        fh_stringstream ss;
        ss << Body;
        ss->seekp(0);
        RegisterStreamWithContextMemoryManagement( ss );
        return ss;
    }
    
    
    fh_mdcontext
    CreateMetaDataContext::setChild( const std::string& rdn, const std::string& v )
    {
        {
            Items_t::iterator iter;
            if( priv_isSubContextBound( rdn, iter ) )
            {
                CreateMetaDataContext* child = dynamic_cast<CreateMetaDataContext*>( GetImpl(*iter) );
                child->setBody( v );
                return child;
            }
        }
        
        CreateMetaDataContext* child = new CreateMetaDataContext();
        child->setContext( this, rdn );
        addNewChild( child );
        child->setBody( v );
        return child;
    }

    bool
    CreateMetaDataContext::supportsReClaim()
    {
        return true;
    }
    

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    DelegatingCreateMetaDataContext::
    DelegatingCreateMetaDataContext( const fh_context& _shadow )
        :
        shadow( _shadow )
    {
        setContext( shadow->getParent(), shadow->getDirName() );
        buildCoveringTree();
    }

    DelegatingCreateMetaDataContext::DelegatingCreateMetaDataContext(
        DelegatingCreateMetaDataContext* parent,
        const fh_context& _shadow
        )
        :
        shadow( _shadow )
    {
        setContext( parent, shadow->getDirName() );
    }

    void
    DelegatingCreateMetaDataContext::buildCoveringTree()
    {
        typedef Context::SubContextNames_t cnt;
        cnt cn = shadow->getSubContextNames();
        for( cnt::iterator iter = cn.begin(); iter != cn.end(); ++iter )
        {
            fh_context iterc = shadow->getSubContext( *iter );
            DelegatingCreateMetaDataContext* child;
            child = new DelegatingCreateMetaDataContext( this, iterc );
            addNewChild( child );
            child->buildCoveringTree();
        }
    }
    
    DelegatingCreateMetaDataContext::~DelegatingCreateMetaDataContext()
    {
    }


    fh_istream
    DelegatingCreateMetaDataContext::getIStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               exception)
    {
        if( Body.length() )
        {
            fh_stringstream ss;
            ss << Body;
            ss->seekp(0);
            RegisterStreamWithContextMemoryManagement( ss );
            return ss;
        }
        return shadow->getIStream();
    }
    
    fh_attribute
    DelegatingCreateMetaDataContext::getAttribute( const std::string& rdn ) throw( NoSuchAttribute )
    {
        if( _Base::isAttributeBound( rdn ) )
        {
            return _Base::getAttribute( rdn );
        }
    
        if( isBound( shadow ) )
        {
            return shadow->getAttribute(rdn);
        }

        return _Base::getAttribute(rdn);
    }

    bool
    DelegatingCreateMetaDataContext::isAttributeBound(
        const std::string& rdn,
        bool createIfNotThere )
        throw( NoSuchAttribute )
    {
        if( isBound( shadow ) )
        {
            return
                shadow->isAttributeBound( rdn )
                || _Base::isAttributeBound( rdn );
        }
        return _Base::isAttributeBound( rdn );
    }
    
    
    
    AttributeCollection::AttributeNames_t&
    DelegatingCreateMetaDataContext::getAttributeNames( AttributeNames_t& ret )
    {
        if( isBound(shadow) )
        {
            AttributeCollection::AttributeNames_t t1;
            AttributeCollection::AttributeNames_t t2;
            _Base::getAttributeNames( t1 );
            shadow->getAttributeNames( t2 );
            return mergeAttributeNames( ret, t1, t2 );
        }
        
        return _Base::getAttributeNames( ret );
    }

    int
    DelegatingCreateMetaDataContext::getAttributeCount()
    {
        AttributeNames_t tmp;
        getAttributeNames( tmp );
        return tmp.size();
    }

    bool
    DelegatingCreateMetaDataContext::supportsReClaim()
    {
        return true;
    }
 


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



    gint CachedContext_sweeper(gpointer data);
    
    class FERRISEXP_DLLLOCAL CachedContext : public ChainedViewContext
    {
        typedef CachedContext      _Self;
        typedef ChainedViewContext _Base;

        guint   expungeInterval_ID;
        guint32 expungeInterval;
        static const char* const EXPUNGEINTERVAL_KEY;
        static const int expungeInterval_default = 3000; // milli seconds
        static const int expungeInterval_min     = 500;  // milli seconds
        void setup()
            {
                expungeInterval_ID = 0;
                createStateLessAttributes();
                Delegate->getNamingEvent_Changed_Sig().connect( sigc::mem_fun( *this, &_Self::OnChanged));

                if( Delegate->supportsMonitoring() )
                    expungeInterval = 0;

                if( expungeInterval > 0 )
                {
                    expungeInterval = min( expungeInterval_min, expungeInterval );
                    expungeInterval_ID = g_timeout_add(
                        expungeInterval,
                        GSourceFunc(CachedContext_sweeper),
                        this );
                }
            }
        
        
    public:

        CachedContext( const fh_context& ctx, const std::string& extradata )
            :
            ChainedViewContext( ctx ),
            expungeInterval( expungeInterval_default )
            {
                StringMap_t m = Util::ParseKeyValueString( extradata );
                if( m.end() != m.find( EXPUNGEINTERVAL_KEY ) )
                {
                    string s = m[EXPUNGEINTERVAL_KEY];
                    expungeInterval = Time::ParseSimpleIntervalString( s );
                }
                setup();
            }

        CachedContext( const fh_context& ctx, guint32 expungeInterval = 0 )
            :
            ChainedViewContext( ctx ),
            expungeInterval( expungeInterval )
            {
                setup();
            }

        ~CachedContext()
            {
                if( expungeInterval_ID )
                {
                    g_source_remove( expungeInterval_ID );
                }
            }
        
        void clearCaches()
            {
                IStreamCache.clear();
                IOStreamCache.clear();
            }
        
        void
        createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }

        void
        priv_createAttributes()
            {
                _Base::priv_createAttributes();
            }

        
        /*************************************************************
         *
         * Some attribute magic
         *
         *************************************************************/
//         typedef Loki::AssocVector< std::string, EA_Atom* > CacheAttributes_t;
//         CacheAttributes_t CacheAttributes;

//        typedef Loki::AssocVector< std::string, fh_stringstream > StreamCache_t;
        typedef std::map< std::string, fh_stringstream > StreamCache_t;
        StreamCache_t IStreamCache;
        StreamCache_t IOStreamCache;

        string toCachedAttrName( string s )
            {
                return "cached-" + s;
            }

        string fromCachedAttrName( string s )
            {
                return s.substr( strlen("cached-") );
            }
        
        fh_istream GetIStream( Context* c, const std::string& _rdn, EA_Atom* atom )
            {
                string rdn = fromCachedAttrName( _rdn );
                LG_CTX_D << "CacheContext::GetIStream for rdn:" << rdn
                         << " this:" << getURL()
                         << " delegate:" << Delegate->getURL()
                         << " c:" << c->getURL()
                         << endl;

                StreamCache_t* cache = &IStreamCache;

                /* Cache hit? */
                StreamCache_t::iterator iter = cache->find( rdn );
                if( cache->end() != iter )
                {
                    fh_stringstream ret;
                    fh_istream input = iter->second;
                    input.clear();
                    input.seekg( 0 );
                    copy( istreambuf_iterator<char>(input),
                          istreambuf_iterator<char>(),
                          ostreambuf_iterator<char>(ret));
                    LG_CTX_D << "CacheContext::GetIStream cache hit for rdn:" << rdn
                         << " data:" << tostr(ret)
                         << endl;
                    return ret;
                }

                /* Create new cache and return copy */
                fh_attribute attr = Delegate->getAttribute( rdn );
                fh_istream delegatess = attr->getIStream();
                
                fh_stringstream cachess;
                copy( istreambuf_iterator<char>(delegatess),
                      istreambuf_iterator<char>(),
                      ostreambuf_iterator<char>(cachess));
                cache->insert( make_pair( rdn, cachess ) );

                LG_CTX_D << "CacheContext::GetIStream get new data for rdn:" << rdn
                     << " data:" << tostr(cachess)
                     << endl;
                

                delegatess.clear();
                delegatess.seekg( 0 );
                
                /* Just return the original to them */
                return delegatess;
            }
        
        fh_iostream GetIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                StreamCache_t* cache = &IOStreamCache;

                /* Cache hit? */
                StreamCache_t::iterator iter = cache->find( rdn );
                if( cache->end() != iter )
                {
                    fh_stringstream ret;
                    copy( istreambuf_iterator<char>(iter->second),
                          istreambuf_iterator<char>(),
                          ostreambuf_iterator<char>(ret));
                    return ret;
                }

                /* Create new cache and return copy */
                fh_attribute attr = Delegate->getAttribute( rdn );
                fh_iostream delegatess = attr->getIOStream();
                
                fh_stringstream cachess;
                copy( istreambuf_iterator<char>(delegatess),
                      istreambuf_iterator<char>(),
                      ostreambuf_iterator<char>(cachess));
                cache->insert( make_pair( rdn, cachess ) );

                /* Copy data so that we always have to do work in IOStreamClosed */
                fh_stringstream ret;
                copy( istreambuf_iterator<char>(delegatess),
                      istreambuf_iterator<char>(),
                      ostreambuf_iterator<char>(ret));
                return ret;
            }
        
        void IOStreamClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream iss )
            {
                fh_attribute attr = Delegate->getAttribute( rdn );
                fh_iostream  oss  = attr->getIOStream();

                copy( istreambuf_iterator<char>(iss),
                      istreambuf_iterator<char>(),
                      ostreambuf_iterator<char>(oss));
            }

        virtual bool isAttributeBound( const std::string& rdn,
                                       bool createIfNotThere = true
            ) throw( NoSuchAttribute )
            {
                return Delegate->isAttributeBound( rdn, createIfNotThere );
            }

        XSDBasic_t getUnderlyingEAType( const std::string& rdn )
            {
                return getSchemaType( this, rdn, FXD_BINARY );
            }
        
        virtual std::string private_getStrAttr( const std::string& const_rdn,
                                                const std::string& def = "",
                                                bool getAllLines = false ,
                                                bool throwEx = false )
            {
                if( !const_rdn.length() || const_rdn == "." || const_rdn == "content" )
                    return _Base::private_getStrAttr( const_rdn, def, getAllLines, throwEx );

                string rdn = toCachedAttrName( const_rdn );
                LG_CTX_D << "CacheContext::getAttribute rdn:" << rdn << endl;

                /* Try for cache hit */
                if( EA_Atom* atom = getAttributeIfExists( rdn ) )
                {
                    LG_CTX_D << "CacheContext::getAttribute cached for rdn:" << rdn << endl;
                    return _Base::private_getStrAttr( rdn, def, getAllLines, throwEx );
                }

                /* Create new caching EA, attach it, and return attribute */
                LG_CTX_D << "CacheContext::getAttribute create for rdn:" << rdn << endl;
                addAttribute( rdn,
                              this, &_Self::GetIStream,
                              this, &_Self::GetIOStream,
                              this, &_Self::IOStreamClosed,
                              getUnderlyingEAType( const_rdn ),
                              0 );

                return _Base::private_getStrAttr( rdn, def, getAllLines, throwEx );
            }
        
        
        
        fh_attribute
        getAttribute( const std::string& _rdn ) throw( NoSuchAttribute )
            {
                std::string rdn = _rdn;
                
                if( !rdn.length() || rdn == "." )
                {
                    rdn = "content";
                }
                if( rdn == "content" )
                {
                    return _Base::getAttribute( rdn );
                }

                string rawrdn = rdn;
                rdn = toCachedAttrName( rdn );
                LG_CTX_D << "CacheContext::getAttribute rdn:" << rdn << endl;

                /* Try for cache hit */
                if( EA_Atom* atom = getAttributeIfExists( rdn ) )
                {
                    LG_CTX_D << "CacheContext::getAttribute cached for rdn:" << rdn << endl;
                    return new AttributeProxy( this, atom, rdn );
                }

                LG_CTX_D << "CacheContext::getAttribute create for rdn:" << rdn << endl;
                /* Create new caching EA, attach it, and return attribute */
                addAttribute( rdn,
                              this, &_Self::GetIStream,
                              this, &_Self::GetIOStream,
                              this, &_Self::IOStreamClosed,
                              getUnderlyingEAType( rawrdn ),
                              0 );
                if( EA_Atom* atom = getAttributeIfExists( rdn ) )
                {
                    return new AttributeProxy( this, atom, rdn );
                }
                
                std::stringstream ss;
                ss << "NoSuchAttribute() for attr:" << rdn << endl;
                Throw_NoSuchAttribute( tostr(ss), this );
            }

        virtual void OnChanged( NamingEvent_Changed* ev,  std::string olddn, std::string newdn )
            {
                LG_CTX_D << "cachecontext::OnChanged c:" << getURL() << endl;
                LG_CTX_D << "clearing the cache." << endl;

                clearCaches();
                Emit_Changed( 0, getDirPath(), getDirPath(), 0 );
            }
        
        virtual void OnExists ( NamingEvent_Exists* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn )
            {
//                fh_context subc = ev->getSource()->getSubContext( olddn );

                LG_CTX_D << "cachecontext::OnExists new:" << subc->getURL() << endl;
                string rdn = subc->getDirName();

                bool created = false;
                if( !priv_discoveredSubContext( rdn, created ) )
                {
                    if( canInsert( rdn ) )
                    {
                        CachedContext* cc = new CachedContext( subc, expungeInterval );
                        Insert( cc, false, !Delegate->areReadingDir() );
                        LG_CTX_D << "cachecontext::Inserted cc:" << cc->getURL() << endl;
                    }
                }
            }

        void
        OnCreated( NamingEvent_Created* ev,
                   const fh_context& subc,
                   std::string olddn, std::string newdn )
            {
//                fh_context subc = ev->getSource()->getSubContext( olddn );
                LG_CTX_D << "cachecontext::OnCreated c:" << subc->getURL() << endl;

                bool created = true;
                if( !priv_discoveredSubContext( subc->getDirName(), created ) )
                {
                    CachedContext* cc = new CachedContext( subc, expungeInterval );
                    Insert( cc, 1, 1 );
                }
            }

        void
        OnDeleted( NamingEvent_Deleted* ev, string olddn, string newdn )
            {
//                Emit_Deleted( ev, newdn, olddn, 0 );
                Remove( olddn );
            }

        virtual void read( bool force = 0 )
            {
                if( !force && Delegate->HaveReadDir && Delegate->isActiveView() )
                {
                    if( !getItems().empty() )
                    {
                        LG_CTX_D << "CacheContext::read( bool force " << force << " )"
                                 << " shorting out attempt to read() because we are already read."
                                 << endl;
                        HaveReadDir = true;
                        emitExistsEventForEachItem();
                        return;
                    }
                }
        
                clearContext();
                Delegate->read( force );
                emitExistsEventForEachItem();
            }
        
        virtual long guessSize() throw()
            {
                return Delegate->guessSize();
            }

    
    protected:

        virtual void emitExistsEventForEachItem()
            {
                for( Items_t::iterator iter = getItems().begin();
                 iter != getItems().end(); ++iter )
            {
                Emit_Exists( 0, *iter, (*iter)->getDirName(), (*iter)->getDirName(), 0 );
            }
                
//                 SubContextNames_t na = getSubContextNames();
//                 for( SubContextNames_t::iterator iter = na.begin();
//                      iter != na.end(); ++iter )
//                 {
//                     Emit_Exists( 0, *iter, *iter, 0 );
//                 }
            }
        virtual void UnPageSubContextsIfNeeded()
            {
                LG_CTX_D << "CachedContext::UnPageSubContextsIfNeeded()" << endl;
            }
        
        virtual Context* priv_CreateContext( Context* parent, std::string rdn )
            {
                LG_CTX_ER << "priv_CreateContext() should never happen" << endl;
            }
    };

    const char* const CachedContext::EXPUNGEINTERVAL_KEY = "updatetime";
    
    FERRIS_SMARTPTR( CachedContext, fh_cachedcontext );

    gint CachedContext_sweeper(gpointer data)
    {
        CachedContext* cc = (CachedContext*)data;
        cc->clearCaches();
        return 1;
    }
    
    
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    namespace Factory
    {
        /**
         * Wrap a context into a caching context. The return value appears just like
         * the one passed in except that calls to getAttribute() etc will return cached
         * data that is only updated when changed events occur from the wrapped context
         *
         * @param extradata This is extra info from the URL string that sets params for
         *        this cache. For example updatetime=300s would tell the cache to reclaim
         *        cached data memory after 300s thus a subsequent read of that attribute
         *        would require delegation to the underlying context again.
         *        This is in the form key=value&key2=value[...][/].
         *        NOTE: unused at the moment.
         */
        fh_context MakeCachedContext( fh_context& ctx, const std::string& extradata )
        {
            CachedContext* ret = new CachedContext( ctx, extradata );

            // ret is a overlay root node
            ret->setIsChainedViewContextRoot();
            
            return ret;
        }
        
    };

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    

/**
 * Create a sorted context that uses the base context ctx and exposes
 * the contexts from the base context in sorted order
 *
 * @param ctx Base context
 * @param sorter Sorting Predicate
 */
    SortedContext::SortedContext(
        const fh_context& ctx,
        const std::string& s )
        :
        ChainedViewContext(ctx, false),
        LazySorting( false )
    {
        thisIterator = getItems().end();
        
#ifndef LIBFERRIS__USE_BOOST_MULTI_INDEX_FOR_STORING_SUBCONTEXTS
        ItemsSorted = Items_t( ContextSetCompare(s) );
#else
        Items_t::ctor_args_list args_list=
            boost::make_tuple(

                boost::make_tuple(
                    boost::multi_index::identity<fh_context>(),
                    ContextSetCompare(s)
                    ),

                // ctor_args for hash index are unchanged.
                Items_t::index<ITEMS_T_BY_NAME_UNORDERED_TAG>::type::ctor_args()
                
                );
        ItemsSorted = Items_t( args_list );
#endif
        
        createStateLessAttributes();
        SetupEventConnections();
//        Delegate->emitExistsEventForEachItem();
    }

    /**
     * The parent passes in a reference to its SortedItems set<> so that
     * we can copy the sorting predicate to our SortedItems
     */
    SortedContext::SortedContext(
        const fh_context& ctx,
        const Items_t& sorteditemscopy )
        :
        ChainedViewContext(ctx, false),
        LazySorting( false )
    {
        thisIterator = getItems().end();
        
#ifndef LIBFERRIS__USE_BOOST_MULTI_INDEX_FOR_STORING_SUBCONTEXTS
        ItemsSorted = Items_t( sorteditemscopy.key_comp() );
#else
        Items_t::ctor_args_list args_list=
            boost::make_tuple(
                
                boost::make_tuple(
                    boost::multi_index::identity<fh_context>(),
                    sorteditemscopy.get<ITEMS_T_BY_NAME_ORDERED_TAG>().key_comp()
                    ),
                
                // ctor_args for hash index are unchanged.
                Items_t::index<ITEMS_T_BY_NAME_UNORDERED_TAG>::type::ctor_args()
                );
        ItemsSorted = Items_t( args_list );
#endif
        
        createStateLessAttributes();
//        SetupEventConnections();
    }

    void
    SortedContext::Remove( Context*   ctx, bool emitdeleted )
    {
        // Note, we could have gotten here based on a OnDeleted Signal handler,
        // we are removing a context from ourselves which may trigger another OnDeleted
        // signal handler. This other signal handler might want to use a ContextIterator
        // which triggers a revalidate() and wants to use SortedItems one last time.
        //
        // We thus must remove ctx *after* calling the base method.
        _Base::Remove( ctx, emitdeleted );
//        getSortedItems().erase( ctx );
        eraseItemByName( getSortedItems(), ctx );
        
    }
    
    
    void
    SortedContext::createStateLessAttributes( bool force )
    {
        static Util::SingleShot virgin;
        if( virgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }

    SortedContext::Items_t&
    SortedContext::getSortedItems()
    {
        return ItemsSorted;
    }

    /**
     * Because we keep a second index to all children then we must bump the
     * reference count.
     */
    int
    SortedContext::getMinimumReferenceCount()
    {
        return _Base::getMinimumReferenceCount() + 1;
    }
    

    void
    SortedContext::setLazySorting( bool v )
    {
        LazySorting = v;
        sort();
    }
    
    void
    SortedContext::sort()
    {
        if( !LazySorting )
            return;

        LazySortCache.clear();
        LazySortCache = _Base::getSubContextNames();
    }

    Context::SubContextNames_t&
    SortedContext::getSubContextNames()
    {
        if( LazySorting ) return LazySortCache;
        return _Base::getSubContextNames();
    }
    
    
    

/**
 * When a context is deleted from the base context it also gets removed from the filtering
 * context.
 */
    void
    SortedContext::OnDeleted( NamingEvent_Deleted* ev, string olddn, string newdn )
    {
        
//         Emit_Deleted( ev, newdn, olddn, 0 );
//         Remove( ev->getSource()->getSubContext( olddn ) );

//         cerr << "SortedContext::OnDeleted url:" << getURL()
//              << " oldrdn:" << olddn << endl;
//         dumpOutItems();
        
        if( isSubContextBound( olddn ) )
        {
            fh_context c = getSubContext( olddn );
            Remove( GetImpl(getSubContext(olddn)) );
//            ItemsSorted.erase( GetImpl(c) );
            eraseItemByName( ItemsSorted, GetImpl(c) );
        }
        
        if( LazySorting )
        {
            LazySortCache.remove( olddn );
        }
    }

    void
    SortedContext::addToSorted( fh_context subc, bool created, bool emit )
    {
        try
        {
            LG_CTX_D << "SortedContext::addToSorted(1)" << endl;
            
//            cerr << "addToSorted() subc:" << subc->getURL() << endl;
            fh_sortedContext nc = new SortedContext( subc, ItemsSorted );
            LG_CTX_D << "SortedContext::addToSorted(2)" << endl;
            nc->setContext( this, subc->getDirName() );
            LG_CTX_D << "SortedContext::addToSorted(3)" << endl;

//         cerr << "SortedContext::addToSorted"
//              << " this:" << toVoid(this)
//              << " created:" << created
//              << " e:" << emit
//              << " nc:" << toVoid(nc)
//              << " nc.url:" << nc->getURL()
//              << endl;


            //
            // NOTE: Insert() will possibly emit a signal that other things are watching,
            // they in turn might want to get an iterator to this new context, which
            // in turn will rely on thisIterator being set properly.
            //
            LG_CTX_D << "SortedContext::addToSorted(4)" << endl;
            nc->thisIterator = ItemsSorted.insert( nc ).first;
            LG_CTX_D << "SortedContext::addToSorted(4.2)" << endl;
            Insert( GetImpl(nc), created, emit );
            LG_CTX_D << "SortedContext::addToSorted(5)" << endl;

            if( LazySorting )
                LazySortCache.push_back( nc->getDirName() );
        }
        catch( exception& e )
        {
            cerr << "SortedContext::addToSorted() url:" << getURL() << endl 
                 << " child:" << subc->getURL() << endl
                 << " e:" << e.what() << endl;
            throw;
        }
    }

    
    
    void
    SortedContext::OnCreated( NamingEvent_Created* ev,
                              const fh_context& subc,
                              std::string olddn, std::string newdn )
    {
        LG_CTX_D << "SortedContext::OnCreated() rdn:" << olddn << endl;
//         cerr << "SortedContext::OnCreated rdn:" << olddn << endl;
//        fh_context subc = ev->getSource()->getSubContext( olddn );

        bool created = true;
        if( !priv_discoveredSubContext( subc->getDirName(), created ) )
        {
            addToSorted( subc, 1, 1 );
        }
    }
    

/**
 * Add the newly discovered context to the view if it passes the matcher.
 */
    void
    SortedContext::OnExists( NamingEvent_Exists* ev,
                             const fh_context& subc,
                             string olddn, string newdn )
    {
        LG_SORT_I
            << "SortedContext::OnExists() enter... newdn:" << newdn
            << " Delegate->areReadingDir():" << Delegate->areReadingDir()
            << endl;
    
//        fh_context subc = ev->getSource()->getSubContext( olddn );

        /*
         * Emit exists events, only if we are not reading the dir at this point.
         * (read operations emit events in sorted order)
         */
        string rdn = subc->getDirName();

        LG_SORT_I << "SortedContext::OnExists(1)... newdn:" << newdn << endl;
         
        bool created = false;
        if( !priv_discoveredSubContext( rdn, created ) )
        {
            LG_SORT_I << "SortedContext::OnExists(2)... newdn:" << newdn << endl;
            if( canInsert( rdn ) )
            {
                LG_SORT_I << "SortedContext::OnExists(3)... newdn:" << newdn << endl;
                LG_SORT_D
                    << "SortedContext::OnExists() adding... newdn:" << newdn
                    << " emit:" << (!Delegate->areReadingDir())
                    << endl;
                addToSorted( subc, false, !Delegate->areReadingDir() );
                LG_SORT_I << "SortedContext::OnExists(4)... newdn:" << newdn << endl;
            }
        }
    }


/**
 * Disallow and log any attempt to directly create a new context.
 * All other methods should delegate the creation of new subcontexts to
 * the underlying base context and from there the events will inform
 * this context of the creation and we will in turn filter that new
 * context.
 */
    Context*
    SortedContext::priv_CreateContext( Context* parent, string rdn )
    {
        LG_CTX_ER << "priv_CreateContext() should never happen" << endl;
    }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * We emit in sorted order.
 */
    void
    SortedContext::read( bool force )
    {
        if( !force && Delegate->HaveReadDir && Delegate->isActiveView() )
        {
            if( !getItems().empty() )
            {
                LG_SORT_I << "SortedContext::read( bool force " << force << " )"
                          << " quick return "
                          << endl;
                HaveReadDir = true;
                emitExistsEventForEachItem();
                return;
            }
        }
        
        LG_SORT_I << "SortedContext::read( bool force " << force << " )"
                  << " Delegate:" << Delegate->getURL()
                  << endl;

        clearContext();
        ensureEventConnections();
        Delegate->read( true );
        emitExistsEventForEachItem();
    }


    void
    SortedContext::emitExistsEventForEachItem()
    {
        try
        {
            LG_SORT_D << "SortedContext::emitExistsEventForEachItem()" << endl;
            for( Items_t::iterator iter = getSortedItems().begin();
                 iter != getSortedItems().end(); ++iter )
            {
                LG_SORT_D << "SortedContext::emitExists... rdn:" << (*iter)->getDirName() << endl;
                Emit_Exists( 0, *iter, (*iter)->getDirName(), (*iter)->getDirName(), 0 );
            }
            
//             SubContextNames_t na = getSubContextNames();
//             for( SubContextNames_t::iterator iter = na.begin();
//                  iter != na.end(); ++iter )
//             {
//                 Emit_Exists( 0, *iter, *iter, 0 );
//             }
        }
        catch( FerrisNotReadableAsContext& e )
        {
        }
    }

    void
    SortedContext::UnPageSubContextsIfNeeded()
    {
        LG_CTX_D << "SortedContext::UnPageSubContextsIfNeeded()" << endl;
        Delegate->UnPageSubContextsIfNeeded();
    }



    long
    SortedContext::guessSize() throw()
    {
        return Delegate->guessSize();
    }


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    VirtualSoftlinkContext::VirtualSoftlinkContext( const fh_context& parent,
                                                    const fh_context& target,
                                                    bool setupEventConnections )
        :
        _Base( parent, target, setupEventConnections )
    {
        createStateLessAttributes();

            addAttribute("is-native", this, &_Self::getFalseStream, XSD_BASIC_BOOL );
            addAttribute("is-dir",    this, &_Self::getFalseStream, XSD_BASIC_BOOL );
            addAttribute("is-file",   this, &_Self::getFalseStream, XSD_BASIC_BOOL );
            addAttribute("is-special",this, &_Self::getFalseStream, XSD_BASIC_BOOL );
            addAttribute("is-link",   this, &_Self::getTrueStream,  XSD_BASIC_BOOL );

            addAttribute("dontfollow-is-native", this, &_Self::getFalseStream, XSD_BASIC_BOOL );
            addAttribute("dontfollow-is-dir",    this, &_Self::getFalseStream, XSD_BASIC_BOOL );
            addAttribute("dontfollow-is-file",   this, &_Self::getFalseStream, XSD_BASIC_BOOL );
            addAttribute("dontfollow-is-special",this, &_Self::getFalseStream, XSD_BASIC_BOOL );
            addAttribute("dontfollow-is-link",   this, &_Self::getTrueStream,  XSD_BASIC_BOOL );

            addAttribute("link-target", this, &_Self::getDelegateURLStream,  XSD_BASIC_STRING );
            addAttribute("realpath",    this, &_Self::getDelegateURLStream,  XSD_BASIC_STRING );
        
    }

    VirtualSoftlinkContext::VirtualSoftlinkContext( const fh_context& parent,
                                                    const fh_context& target,
                                                    const std::string& localName,
                                                    bool setupEventConnections )
        :
        _Base( parent, target, localName, setupEventConnections )
    {
        createStateLessAttributes();

            addAttribute("is-native", this, &_Self::getFalseStream, XSD_BASIC_BOOL );
            addAttribute("is-dir",    this, &_Self::getFalseStream, XSD_BASIC_BOOL );
            addAttribute("is-file",   this, &_Self::getFalseStream, XSD_BASIC_BOOL );
            addAttribute("is-special",this, &_Self::getFalseStream, XSD_BASIC_BOOL );
            addAttribute("is-link",   this, &_Self::getTrueStream,  XSD_BASIC_BOOL );

            addAttribute("dontfollow-is-native", this, &_Self::getFalseStream, XSD_BASIC_BOOL );
            addAttribute("dontfollow-is-dir",    this, &_Self::getFalseStream, XSD_BASIC_BOOL );
            addAttribute("dontfollow-is-file",   this, &_Self::getFalseStream, XSD_BASIC_BOOL );
            addAttribute("dontfollow-is-special",this, &_Self::getFalseStream, XSD_BASIC_BOOL );
            addAttribute("dontfollow-is-link",   this, &_Self::getTrueStream,  XSD_BASIC_BOOL );

            addAttribute("link-target", this, &_Self::getDelegateURLStream,  XSD_BASIC_STRING );
            addAttribute("realpath",    this, &_Self::getDelegateURLStream,  XSD_BASIC_STRING );
        
            LG_CTX_D << "VirtualSoftlinkContext(ctx,ln) ctx.url:" << target->getURL()
                     << " ln:" << localName
                     << " getDirName:" << getDirName()
                     << endl;
        
    }
    
    VirtualSoftlinkContext::~VirtualSoftlinkContext()
    {
    }

    std::string
    VirtualSoftlinkContext::getRecommendedEA()
    {
        return Delegate->getRecommendedEA() + ",realpath";
    }
    
    /**
     * Either the link target's dirName() or what is set with setLocalName()
     */
    const std::string&
    VirtualSoftlinkContext::getDirName() const
    {
        const string& rdn_ddb = _DontDelegateBase::getDirName();
        const string& rdn_target = Delegate->getDirName();

//         cerr << "VirtualSoftlinkContext::getDirName() "
//              << " rdn_ddb:" << rdn_ddb
//              << " rdn_target:" << rdn_target
//              << endl;
        
        if( rdn_ddb != rdn_target )
            return rdn_ddb;
        return rdn_target;
    }


    std::string
    VirtualSoftlinkContext::getDirPath() throw (FerrisParentNotSetError)
    {
        return _DontDelegateBase::getDirPath();
    }
    
    
    std::string
    VirtualSoftlinkContext::getURL()
    {
        return _DontDelegateBase::getURL();
    }
    
    void
    VirtualSoftlinkContext::read( bool force )
    {
        EnsureStartStopReadingIsFiredRAII _raii1( this );
        
        if( getHaveReadDir() )
        {
            LG_CTX_D << "VirtualSoftlinkContext::read(have read) url:" << getURL() << endl;
            emitExistsEventForEachItem();
        }
        else
        {
            LG_CTX_D << "VirtualSoftlinkContext::read(reading) url:" << getURL() << endl;
            Delegate->read( force );
            emitExistsEventForEachItem();
        }
    }
    
    /**
     * Disallow and log any attempt to directly create a new context.
     * All other methods should delegate the creation of new subcontexts to
     * the underlying base context and from there the events will inform
     * this context of the creation and we will in turn filter that new
     * context.
     */
    Context*
    VirtualSoftlinkContext::priv_CreateContext( Context* parent, string rdn )
    {
        LG_CTX_ER << "priv_CreateContext() should never happen" << endl;
    }
    
    void
    VirtualSoftlinkContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }
        
    fh_istream
    VirtualSoftlinkContext::getDelegateURLStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << Delegate->getURL();
        return ss;
    }

    fh_istream
    VirtualSoftlinkContext::getFalseStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << "0";
        return ss;
    }
    
    fh_istream
    VirtualSoftlinkContext::getTrueStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << "1";
        return ss;
    }
    

    stringset_t&
    VirtualSoftlinkContext::getForceLocalAttributeNames()
    {
        static stringset_t ret;
        if( ret.empty() )
        {
//             ret.push_back("is-native");
//             ret.push_back("is-dir");
//             ret.push_back("is-file");
//             ret.push_back("is-special");
//             ret.push_back("is-link");

//             ret.push_back("dontfollow-is-native");
//             ret.push_back("dontfollow-is-dir");
//             ret.push_back("dontfollow-is-file");
//             ret.push_back("dontfollow-is-special");
//             ret.push_back("dontfollow-is-link");

            ret.insert("link-target");
            ret.insert("realpath");
            ret.insert("name");
            ret.insert("url");
            ret.insert("is-active-view");
        }
        return ret;
    }

//     std::string
//     VirtualSoftlinkContext::private_getStrAttr( const std::string& rdn,
//                                                 const std::string& def,
//                                                 bool getAllLines,
//                                                 bool throwEx )
//     {
//         stringlist_t& sl = getForceLocalAttributeNames();
//         if( sl.end() != find( sl.begin(), sl.end(), rdn ) )
//             return _DontDelegateBase::private_getStrAttr( rdn, def, getAllLines, throwEx );

//         return _Base::private_getStrAttr( rdn, def, getAllLines, throwEx );
//     }
    
    
    
//     fh_attribute
//     VirtualSoftlinkContext::getAttribute( const string& rdn ) throw( NoSuchAttribute )
//     {
//         stringlist_t& sl = getForceLocalAttributeNames();
//         if( sl.end() != find( sl.begin(), sl.end(), rdn ) )
//             return _DontDelegateBase::getAttribute( rdn );
        
//         return Delegate->getAttribute(rdn);
//     }
//     AttributeCollection::AttributeNames_t&
//     VirtualSoftlinkContext::getAttributeNames( AttributeNames_t& ret )
//     {
//         AttributeCollection::AttributeNames_t t1;
//         AttributeCollection::AttributeNames_t t2;
//         Delegate->getAttributeNames( t1 );
//         _DontDelegateBase::getAttributeNames( t2 );
//         return mergeAttributeNames( ret, t1, t2 );
//     }
//     int
//     VirtualSoftlinkContext::getAttributeCount()
//     {
//         return Delegate->getAttributeCount();
//     }
//     bool
//     VirtualSoftlinkContext::isAttributeBound( const std::string& rdn,
//                                               bool createIfNotThere
//         ) throw( NoSuchAttribute )
//     {
//         stringlist_t& sl = getForceLocalAttributeNames();
//         if( sl.end() != find( sl.begin(), sl.end(), rdn ) )
//             return _DontDelegateBase::isAttributeBound( rdn, createIfNotThere );

//         return Delegate->isAttributeBound( rdn, createIfNotThere );
//     }
        
    


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    
    fh_istream
    SL_getNothingStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << "";
        return ss;
    }

    class FERRISEXP_DLLLOCAL DirNameContext_Base
        :
        public StateLessEAHolder< DirNameContext_Base, ChainedViewContext >
    {
        typedef StateLessEAHolder< DirNameContext_Base, ChainedViewContext > _Base;
        typedef DirNameContext_Base _Self;
        typedef Context _BaseNoDelegate;

        guint64 m_AddedOrderID;

        stringset_t& getEANamesNotToDelegate()
            {
                static stringset_t ret;
                static Util::SingleShot v;
                if( v )
                {
                    ret.insert( "name" );
                    ret.insert( "path" );
                    ret.insert( "url" );
                    ret.insert( "name-only"  );
                    ret.insert( "selection-added-order-id" );
                }
                return ret;
            }
        
                        
        bool isIndexAttribute( const std::string& rdn )
            {
                return ( starts_with( rdn, "idx:" ) || starts_with( rdn, "index:" ) );
            }

        string stripIndexAttributePredix( const std::string& rdn )
            {
                PrefixTrimmer trimmer;
                trimmer.push_back( "idx:" );
                trimmer.push_back( "index:" );
                string ret = trimmer( rdn );
                return ret;
            }
        
        XSDBasic_t getUnderlyingEAType( const std::string& rdn )
            {
                return FXD_BINARY; // getSchemaType( this, rdn, FXD_BINARY );
            }
        
    protected:

        EAIndex::fh_fwdeaidx getForwardIndex()
            {
                if( SelectionContext* p = dynamic_cast<SelectionContext*>( getParent() ) )
                {
                    return p->getForwardEAIndexInterface();
                }
                return 0;
            }
        
        static fh_stringstream
        SL_getNameOnlyStream( _Self* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->Delegate->getDirName();
                return ss;
            }

        static fh_stringstream
        SL_getSelectionAddedOrderIDStream( DirNameContext_Base* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_AddedOrderID;
                return ss;
            }
        
        void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
#define SLEA tryAddStateLessAttribute         
                    SLEA( "name-only", &_Self::SL_getNameOnlyStream, XSD_BASIC_STRING );
                    SLEA( "selection-added-order-id", &_Self::SL_getSelectionAddedOrderIDStream, XSD_BASIC_INT );
#undef SLEA
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }

        std::string private_getStrAttr( const std::string& rdn,
                                        const std::string& def,
                                        bool getAllLines,
                                        bool throwEx )
            {
                if( isIndexAttribute( rdn ) )
                {
                    if( EAIndex::fh_fwdeaidx fidx = getForwardIndex() )
                    {
                        string trdn = stripIndexAttributePredix( rdn );
                        
//                         cerr << "private_getStrAttr(A) rdn:" << rdn << endl
//                              << " rdn.trimmed:" << trdn
//                              << " earl:" << getURL() << endl;
                        return fidx->getStrAttr( this, getURL(), trdn, def, throwEx );
                    }
                }
                if( getEANamesNotToDelegate().count( rdn ) )
                    return _BaseNoDelegate::private_getStrAttr( rdn, def, getAllLines, throwEx );
                return _Base::private_getStrAttr( rdn, def, getAllLines, throwEx );
            }

        void setupAddedOrderID()
            {
                static guint64 cache = 0;
                ++cache;
                m_AddedOrderID = cache;
            }

      public:

        DirNameContext_Base( const fh_context& ctx,
                            bool setupEventConnections = true )
            : _Base( ctx ),
              m_AddedOrderID( 0 )
            {
                setupAddedOrderID();
            }
        
        DirNameContext_Base( const fh_context& parent,
                             const fh_context& delegate,
                             const std::string& rdn,
                             bool setupEventConnections = true )
            : _Base( parent, delegate, rdn, setupEventConnections ),
              m_AddedOrderID( 0 )
            {
                setupAddedOrderID();
            }
            
            
        
        

    public:

        virtual bool
        isAttributeBound( const std::string& rdn,
                          bool createIfNotThere = true ) throw( NoSuchAttribute )
            {
                if( isIndexAttribute( rdn ) )
                    return true;
                
                if( getEANamesNotToDelegate().count( rdn ) )
                    return _BaseNoDelegate::isAttributeBound( rdn, createIfNotThere );
                return _Base::isAttributeBound( rdn, createIfNotThere );
            }

        fh_istream GetIStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                if( EAIndex::fh_fwdeaidx fidx = getForwardIndex() )
                {
//                    cerr << "private_getStrAttr(B) rdn:" << rdn << " earl:" << getURL() << endl;
                    ss << fidx->getStrAttr( this, getURL(), rdn, "", 1 );
                }
                return ss;
            }
        
        virtual fh_attribute
        getAttribute( const std::string& rdn ) throw( NoSuchAttribute )
            {
                if( isIndexAttribute( rdn ) )
                {
                    if( EAIndex::fh_fwdeaidx fidx = getForwardIndex() )
                    {
                        /* Try for cache hit */
                        if( EA_Atom* atom = getAttributeIfExists( rdn ) )
                        {
                            LG_CTX_D << "CacheContext::getAttribute cached for rdn:" << rdn << endl;
                            return new AttributeProxy( this, atom, rdn );
                        }

                        LG_CTX_D << "CacheContext::getAttribute create for rdn:" << rdn << endl;
                        /* Create new caching EA, attach it, and return attribute */
                        addAttribute( rdn, this, &_Self::GetIStream, getUnderlyingEAType( rdn ), 0 );
                        if( EA_Atom* atom = getAttributeIfExists( rdn ) )
                        {
                            return new AttributeProxy( this, atom, rdn );
                        }
                
                        std::stringstream ss;
                        ss << "NoSuchAttribute() for attr:" << rdn << endl;
                        Throw_NoSuchAttribute( tostr(ss), this );
                    }
                }

                cerr << "getAttribute() no dele:" << getEANamesNotToDelegate().count( rdn ) << " rdn:" << rdn << endl;
                if( getEANamesNotToDelegate().count( rdn ) )
                    return _BaseNoDelegate::getAttribute( rdn );
                return _Base::getAttribute( rdn );
            }
        
    };
    
    /**
     * This class proxies for another context but changes the getDirName() function
     * to return the URL of the context instead. By doing this the SelectionContext
     * can have many children with the same rdn but only one child with the same URL.
     *
     * This lets folks copy files like
     * /mnt/cdrom/file1
     * /tmp/file1
     * To some place without the selection becoming haywire because there are two files
     * with the same rdn as its children.
     */
    class FERRISEXP_DLLLOCAL URLIsDirNameContext
        :
        public DirNameContext_Base
    {
        typedef DirNameContext_Base _Base;
        typedef URLIsDirNameContext _Self;
        typedef Context _BaseNoDelegate;

        mutable string m_urlCache;
        
    protected:

        _Self*
        priv_CreateContext( Context* parent, string rdn )
            {
                LG_CTX_ER << "URLIsDirNameContext::priv_CreateContext() Should never happen!" << endl;
                Throw_FerrisCanNotCreateLeafOfLeaf( "", this );
            }

    public:

        URLIsDirNameContext( fh_context parent, const fh_context& delegate )
            :
            _Base( delegate )
            {
                setContext( parent, delegate->getDirName() );
//                setAttributeContext( parent, Delegate->getURL() );
                createStateLessAttributes();
            }
        
        virtual ~URLIsDirNameContext()
            {}

        virtual const std::string& getDirName() const
            {
                m_urlCache = Delegate->getURL();
                return m_urlCache;
            }

        Parent_t
        getParent() throw (FerrisParentNotSetError)
            {
                return Context::getParent();
            }
        
    };
    

    class FERRISEXP_DLLLOCAL Context_SelectionContextRDNConflictResolver_MonsterName
        :
        public DirNameContext_Base
    {
        typedef DirNameContext_Base  _Base;
        typedef URLIsDirNameContext  _Self;
        typedef Context _BaseNoDelegate;

    protected:

        _Self*
        priv_CreateContext( Context* parent, string rdn )
            {
                LG_CTX_ER << "Context_SelectionContextRDNConflictResolver_MonsterName::priv_CreateContext() Should never happen!" << endl;
                Throw_FerrisCanNotCreateLeafOfLeaf( "", this );
            }
        
    public:

        Context_SelectionContextRDNConflictResolver_MonsterName(
            fh_context parent, const fh_context& delegate, const std::string& rdn )
            :
            _Base( parent, delegate, rdn )
            {
                setContext( parent, rdn );
                createStateLessAttributes();
            }
        
        virtual ~Context_SelectionContextRDNConflictResolver_MonsterName()
            {}

        virtual const std::string& getDirName() const
            {
                return Context::getDirName();
            }

        virtual std::string getURL()
            {
                fh_stringstream ss;
                ss << getURLScheme() << "://" << getDirPath();
                return tostr(ss);
            }
        
        virtual std::string getDirPath()  throw (Ferris::FerrisParentNotSetError)
            {
                return Context::getDirPath();
            }
        
        Parent_t
        getParent() throw (FerrisParentNotSetError)
            {
                return Context::getParent();
            }
        
    };
    

    

    /**
     * This is the root context for selectionfactory://
     *
     * The main reason for this context is to create selectionContext subcontexts
     * and attach them to ourself.
     */
    class FERRISEXP_DLLLOCAL SelectionFactoryContext
        :
        public FakeInternalContext
    {
        guint32 newFileNumber;
        
        friend class SelectionFactoryContext_RootContextDropper;

    protected:

        virtual void priv_read()
            {
                emitExistsEventForEachItemRAII _raii1( this );
            }
        
    protected:

        virtual bool isDir()
            {
                return true;
            }
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );

        SelectionContext*
        priv_CreateContext( Context* parent, string rdn )
            {
                SelectionContext* ret = new SelectionContext( this, rdn );
//                ret->setContext( this, rdn );
                return ret;
            }

        
    public:

        SelectionFactoryContext();
        virtual ~SelectionFactoryContext();

        virtual fh_context
        createSubContext( const std::string& rdn,
                          fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );

        std::string getURL()
            {
//                 cerr << "selectionfactoryctx::getURL() have-parent:" << isParentBound()
//                      << " path:" << getDirPath() << endl;
//                 BackTrace();
                fh_stringstream ss;
                ss << getURLScheme() << "://" << getDirPath();
                return tostr(ss);
            }
        
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class SelectionContextRDNConflictResolver_UseURLAsRDN : public SelectionContextRDNConflictResolver
    {
    public:
        std::string getRDN( SelectionContext* selc, fh_context newChild );
        fh_context createChild( SelectionContext* selc, fh_context newChild );
    };
    
    class SelectionContextRDNConflictResolver_MonsterName : public SelectionContextRDNConflictResolver
    {
    public:
        std::string getRDN( SelectionContext* selc, fh_context newChild );
        fh_context createChild( SelectionContext* selc, fh_context newChild );
    };
    
    std::string
    SelectionContextRDNConflictResolver_UseURLAsRDN::getRDN(
        SelectionContext* selc, fh_context newChild )
    {
        return newChild->getURL();
    }
    
    fh_context
    SelectionContextRDNConflictResolver_UseURLAsRDN::createChild(
        SelectionContext* selc, fh_context newChild )
    {
        fh_context ret = new URLIsDirNameContext( selc, newChild );
        return ret;
    }
    
    
    std::string
    SelectionContextRDNConflictResolver_MonsterName::getRDN(
        SelectionContext* selc, fh_context newChild )
    {
        string ret = monsterName( selc, newChild->getDirName() );
        return ret;
    }
    
    fh_context
    SelectionContextRDNConflictResolver_MonsterName::createChild(
        SelectionContext* selc, fh_context newChild )
    {
        string rdn = monsterName( selc, newChild->getDirName() );
//        cerr << "SelectionContextRDNConflictResolver_MonsterName::createChild() rdn:" << rdn << endl;
        fh_context ret = new Context_SelectionContextRDNConflictResolver_MonsterName( selc, newChild, rdn );
        return ret;
    }
    
    
    fh_SelectionContextRDNConflictResolver get_SelectionContextRDNConflictResolver_UseURLAsRDN()
    {
        static fh_SelectionContextRDNConflictResolver ret = 0;
        if( !ret )
        {
            ret = new SelectionContextRDNConflictResolver_UseURLAsRDN();
        }
        return ret;
    }

    fh_SelectionContextRDNConflictResolver get_SelectionContextRDNConflictResolver_MonsterName()
    {
        static fh_SelectionContextRDNConflictResolver ret = 0;
        if( !ret )
        {
            ret = new SelectionContextRDNConflictResolver_MonsterName();
        }
        return ret;
    }
    
    
    SelectionContext::SelectionContext( Context* parent, const std::string& rdn )
        :
        _Base( parent, rdn ),
        m_reportedRDN( rdn ),
        m_fwdidx( 0 )
    {
        m_rdnConflictResolver = get_SelectionContextRDNConflictResolver_UseURLAsRDN();
        setContext( parent, rdn );
        createStateLessAttributes();

//        cerr << "SelectionContext::ctor rdn:" << rdn << " url:" << getURL() << endl;
    }

    SelectionContext::~SelectionContext()
    {
    }

    void
    SelectionContext::setSelectionContextRDNConflictResolver( fh_SelectionContextRDNConflictResolver f )
    {
        m_rdnConflictResolver = f;
    }
    
    

    fh_context
    SelectionContext::createSubContext( const std::string& rdn, fh_context md )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        fh_context ret;

        //
        // check if the context at 'md' has already been added to this col
        //
        try
        {
            fh_context item = getItem( m_rdnConflictResolver->getRDN( this, md ) );
            if( ChainedViewContext* rawitem
                = dynamic_cast<ChainedViewContext*>( GetImpl(item) ))
            {
                if( GetImpl( rawitem->Delegate ) == GetImpl( md ) )
                    return item;
            }
        }
        catch( exception& e )
        {
        }
        
        
        {
///            cerr << "SelectionContext::createSubContext() md:" << md->getURL() << endl;
            fh_context childc = m_rdnConflictResolver->createChild( this, md );
            ret = Insert( GetImpl(childc), true );
        }
        return ret;
        
//         //
//         // check if the context at 'md' has already been added to this col
//         //
//         try
//         {
//             fh_context item = getItem( md->getURL() );
//             if( ChainedViewContext* rawitem
//                 = dynamic_cast<ChainedViewContext*>( GetImpl(item) ))
//             {
//                 if( GetImpl( rawitem->Delegate ) == GetImpl( md ) )
//                     return item;
//             }
//         }
//         catch( exception& e )
//         {
//         }
        
        
//         {
//             fh_context childc = new URLIsDirNameContext( this, md );
//             ret = Insert( GetImpl(childc) );
//         }
//         return ret;
    }
    
    void
    SelectionContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
        m["file"] = SubContextCreator(
            SL_SubCreate_file,
            "	<elementType name=\"file\">\n"
            "		<elementType name=\"name\" default=\"new file\">\n"
            "			<dataTypeRef name=\"string\"/>\n"
            "		</elementType>\n"
            "	</elementType>\n");
    }

    fh_stringstream
    SelectionContext::SL_getNameOnlyStream( SelectionContext* c, const std::string& ean, EA_Atom* atom )
    {
//        cerr << "SelectionContext::SL_getNameOnlyStream()" << endl;
        fh_stringstream ss;
        ss << c->_BaseNoDelegate::private_getStrAttr( "name", "", 1, 1 );
        return ss;
    }

    
    void
    SelectionContext::createStateLessAttributes( bool force  )
    {
        static Util::SingleShot virgin;
        if( virgin() )
        {
//            cerr << "SelectionContext::createStateLessAttributes()" << endl;
#define SLEA tryAddStateLessAttribute         
            SLEA( "mtime", SL_getNothingStream, FXD_UNIXEPOCH_T );
            SLEA( "atime", SL_getNothingStream, FXD_UNIXEPOCH_T );
            SLEA( "ctime", SL_getNothingStream, FXD_UNIXEPOCH_T );
            SLEA( "size",  SL_getNothingStream, FXD_FILESIZE );
            SLEA( "dontfollow-size", SL_getNothingStream, FXD_FILESIZE );
            SLEA( "name-only", SL_getNameOnlyStream, XSD_BASIC_STRING );
#undef SLEA
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }

    void
    SelectionContext::setReportedRDN( const std::string& rdn )
    {
        m_reportedRDN = rdn;
    }

    bool
    SelectionContext::isAttributeBound( const std::string& rdn,
                                        bool createIfNotThere ) throw( NoSuchAttribute )
    {
        if( rdn == "name" || rdn == "path" || rdn == "url" || rdn == "name-only" )
            return _BaseNoDelegate::isAttributeBound( rdn, createIfNotThere );
        return _Base::isAttributeBound( rdn, createIfNotThere );
    }
    
    fh_attribute
    SelectionContext::getAttribute( const std::string& rdn ) throw( NoSuchAttribute )
    {
//        cerr << "SelectionContext::ga() rdn:" << rdn << endl;
        if( rdn == "name" || rdn == "path" || rdn == "url" || rdn == "name-only" )
            return _BaseNoDelegate::getAttribute( rdn );
        return _Base::getAttribute( rdn );
    }
    
    

    std::string
    SelectionContext::private_getStrAttr( const std::string& rdn,
                                          const std::string& def,
                                          bool getAllLines,
                                          bool throwEx )
    {
//        cerr << "selc private_getStrAttr() rdn:" << rdn << endl;
        
        if( rdn == "name" || rdn == "path" || rdn == "url" || rdn == "name-only" )
            return _BaseNoDelegate::private_getStrAttr( rdn, def, getAllLines, throwEx );
        return _Base::private_getStrAttr( rdn, def, getAllLines, throwEx );
    }

    const std::string& SelectionContext::getDirName() const
            {
                return m_reportedRDN;
            }
        
    std::string SelectionContext::getDirPath() throw (FerrisParentNotSetError)
            {
//                 cerr << "SelectionContext::getDirPath(top)" << endl;
//                 if( isParentBound() )
//                 {
//                     cerr << "p.path:" << getParent()->getDirPath() << endl;
//                     cerr << "p.name:" << getParent()->getDirName() << endl;
//                 }
//                 cerr << "this.name:" << getDirName() << endl;
                
                return Context::getDirPath();
            }
        
    std::string SelectionContext::getURL()
            {
//                cerr << "SelectionContext::getURL()" << endl;
                fh_stringstream ss;
                ss << getURLScheme() << "://" << getDirPath();
                return tostr(ss);
            }
    
//     std::string
//     SelectionContext::getURL()
//     {
//         cerr << "selectionctx::getURL() have-parent:" << isParentBound()
//              << " rdn:" << getDirName() << " path:" << getDirPath() << endl;
//         cerr << "selectionctx::getURL() base.url:" << _Base::getURL() << endl;

//         if( fh_context c = getCoveredContext() )
//             cerr << " cc rdn:" << c->getDirName() << " path:" << c->getDirPath() << endl;
//         if( fh_context c = getOverMountContext() )
//             cerr << " om rdn:" << c->getDirName() << " path:" << c->getDirPath() << endl;
            
//         if( isParentBound() )
//         {
//             fh_context p = getParent();
//             cerr << " parent  have-parent:" << p->isParentBound()
//                  << " rdn:" << p->getDirName() << " path:" << p->getDirPath() << endl;
//             if( fh_context c = p->getCoveredContext() )
//                 cerr << " pcc rdn:" << c->getDirName() << " path:" << c->getDirPath() << endl;
//             if( fh_context c = p->getOverMountContext() )
//                 cerr << " pom rdn:" << c->getDirName() << " path:" << c->getDirPath() << endl;
//         }
        
//         BackTrace();
//         cerr << endl << endl;
        
//         fh_stringstream ss;
//         ss << getURLScheme() << "://" << getDirPath();
//         return tostr(ss);
//     }
    

    bool
    SelectionContext::isDir()
    {
        return true;
    }
    
    void
    SelectionContext::priv_read()
    {
        emitExistsEventForEachItemRAII _raii1( this );
    }

    fh_context
    SelectionContext::insert( fh_context c )
    {
        createSubContext( "", c );
        return c;
    }

    void
    SelectionContext::clear()
    {
        Items.clear();
    }

    bool
    SelectionContext::empty()
    {
        return begin() == end();
    }
    

    void
    SelectionContext::setForwardEAIndexInterface( EAIndex::fh_fwdeaidx fidx )
    {
        m_fwdidx = fidx;
        LG_EAIDX_D << "SelectionContext::setForwardEAIndexInterface() have fidx:" << isBound(fidx) << endl;
    }

    EAIndex::fh_fwdeaidx
    SelectionContext::getForwardEAIndexInterface()
    {
        return m_fwdidx;
    }
    
    
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    SelectionFactoryContext::SelectionFactoryContext()
        :
        newFileNumber(1)
    {
        setContext( 0, "/" );
        createStateLessAttributes();
    }


    SelectionFactoryContext::~SelectionFactoryContext()
    {
    }


    fh_context
    SelectionFactoryContext::createSubContext( const std::string& rdn, fh_context md )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
//         cerr << "SelectionFactoryContext::createSubContext()"
//              << " newFileNumber:" << newFileNumber << endl;
        
        string filename = tostr(newFileNumber++);
        fh_context ret = priv_readSubContext( filename, true );

//         cerr << "SelectionFactoryContext::createSubContext()"
//              << " newFileNumber:" << newFileNumber
//              << " this:" << getURL()
//              << " ret:" << ret->getURL()
//              << " ret.rdn:" << ret->getDirName()
//              << endl;
        return ret;
    }
    

    void
    SelectionFactoryContext::priv_FillCreateSubContextSchemaParts(
        CreateSubContextSchemaPart_t& m )
    {
//        cerr << "SelectionFactoryContext::priv_FillCreateSubContextSchemaParts()" << endl;
        
        m["file"] = SubContextCreator(
            SL_SubCreate_file,
            "	<elementType name=\"file\">\n"
            "		<elementType name=\"name\" default=\"new file\">\n"
            "			<dataTypeRef name=\"string\"/>\n"
            "		</elementType>\n"
            "	</elementType>\n");
    }
    
    class FERRISEXP_DLLLOCAL SelectionFactoryContext_RootContextDropper
        :
        public RootContextDropper
    {
    public:
        SelectionFactoryContext_RootContextDropper()
            {
                RootContextFactory::Register( "selectionfactory", this );
            }

        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
            {
                return new SelectionFactoryContext();
            }
    };
    static SelectionFactoryContext_RootContextDropper ___SelectionFactoryContext_static_init;


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * This is the root context for docidea://
     *
     * This allows clients to store just hte docid and send it to the server to have
     * operations performed like 'cat'
     */
    class FERRISEXP_DLLLOCAL EAIndexDocIDContext
        :
        public FakeInternalContext
    {
        guint32 newFileNumber;
        
        friend class EAIndexDocIDContext_RootContextDropper;

    protected:

        virtual void priv_read()
            {
                emitExistsEventForEachItemRAII _raii1( this );
            }
        
    protected:

        virtual bool isDir()
            {
                return true;
            }
        
        SelectionContext*
        priv_CreateContext( Context* parent, string rdn )
            {
                SelectionContext* ret = new SelectionContext( this, rdn );
                return ret;
            }

        
    public:

        EAIndexDocIDContext();
        virtual ~EAIndexDocIDContext();

        std::string getURL()
            {
                fh_stringstream ss;
                ss << getURLScheme() << "://" << getDirPath();
                return tostr(ss);
            }

        //
        // Short cut loading each dir unless absolutely needed.
        //
        fh_context priv_getSubContext( const string& rdn )
            throw( NoSuchSubContext )
        {
            try
            {
                Items_t::iterator isSubContextBoundCache;
                if( priv_isSubContextBound( rdn, isSubContextBoundCache ) )
                {
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

                stringstream urlss;
                urlss << "eaquery://filter/(docid==" << rdn << ")";
//                cerr << "url:" << urlss.str() << endl;
                fh_context c = Resolve( urlss.str() );
                cerr << "c:" << c->getURL() << endl;
                if( !c->getSubContextCount() )
                {
                    fh_stringstream ss;
                    ss << "NoSuchSubContext no matching document for given docid:" << rdn;
                    Throw_NoSuchSubContext( tostr(ss), this );
                }
                    
                c = (*c->begin());

                fh_context newc = new Context_SelectionContextRDNConflictResolver_MonsterName( this, c, rdn );
                addNewChild( newc );
                
                // cerr << "newc:" << newc->getURL() << endl;
                // cerr << "     rdn:" << rdn << endl;
                // cerr << "newc.rdn:" << newc->getDirName() << endl;
                // cerr << "newc.attr.rdn:" << getStrAttr( newc, "name", "<none>" ) << endl;
                return newc;
            }
            catch( NoSuchSubContext& e )
            {
                throw e;
            }
            catch( exception& e )
            {
                string s = e.what();
                Throw_NoSuchSubContext( s, this );
            }
            catch(...)
            {}
            fh_stringstream ss;
            ss << "NoSuchSubContext:" << rdn;
            Throw_NoSuchSubContext( tostr(ss), this );
        }        
    };
    
    EAIndexDocIDContext::EAIndexDocIDContext()
    {
        setContext( 0, "/" );
        createStateLessAttributes();
    }


    EAIndexDocIDContext::~EAIndexDocIDContext()
    {
    }


    class FERRISEXP_DLLLOCAL EAIndexDocIDContext_RootContextDropper
        :
        public RootContextDropper
    {
    public:
        EAIndexDocIDContext_RootContextDropper()
            {
                RootContextFactory::Register( "docidea", this );
            }

        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
            {
                return new EAIndexDocIDContext();
            }
    };
    static EAIndexDocIDContext_RootContextDropper ___EAIndexDocIDContext_static_init;

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    void
    ManyBaseToOneViewContext::common_setup( fh_context parent,
                                            fh_context ctx,
                                            bool isCannibal )
    {
        m_hasSetupBeenCalled = false;
        m_isCannibal = isCannibal;
        setContext( GetImpl( parent ), ctx->getDirName() );
        appendToBaseContexts( ctx );
    }
    
    ManyBaseToOneViewContext::ManyBaseToOneViewContext( fh_context parent,
                                                        fh_context ctx,
                                                        bool isCannibal )
        :
        ChainedViewContext( ctx, false )
    {
        common_setup( parent, ctx, isCannibal );
    }

    ManyBaseToOneViewContext::ManyBaseToOneViewContext( Context* parent,
                                                        const std::string& rdn )
        :
        ChainedViewContext( parent, false )
    {
    }
    
    
    
    ManyBaseToOneViewContext::~ManyBaseToOneViewContext()
    {
    }

    Context*
    ManyBaseToOneViewContext::getUnderlyingContext( fh_context c )
    {
        Context* ret = GetImpl(c);
        
        if( ChainedViewContext* rawc = dynamic_cast<ChainedViewContext*>( ret ))
        {
            ret = GetImpl( rawc->Delegate );
        }
        
        return ret;
    }

    
    void
    ManyBaseToOneViewContext::appendToBaseContexts( fh_context c )
    {
        if( c )
            m_baseContexts.push_back( c );
    }

    void
    ManyBaseToOneViewContext::setup()
    {
        m_hasSetupBeenCalled = true;
        
        for( m_baseContexts_t::iterator bi = m_baseContexts.begin();
             bi != m_baseContexts.end(); ++bi )
        {
            SetupEventConnections( *bi );
        }
    }

    bool
    ManyBaseToOneViewContext::shouldInsertContext( const fh_context& c, bool created )
    {
        return true;
    }
    
    void
    ManyBaseToOneViewContext::OnDeleted( NamingEvent_Deleted* ev, string olddn, string newdn )
    {
        if( isSubContextBound( olddn ) )
        {
            Emit_Deleted( ev, newdn, olddn, 0 );
            Remove( ev->getSource()->getSubContext( olddn ) );
        }
    }

    void
    ManyBaseToOneViewContext::OnExists( NamingEvent_Exists* ev,
                                        const fh_context& subc,
                                        string olddn, string newdn )
    {
//        fh_context subc = ev->getSource()->getSubContext( olddn );

        if( shouldInsertContext( subc, false ) )
        {
            cascadedInsert( GetImpl(subc), false );
        }
        else
        {
            LG_CTX_D << "OnExists() not inserting FOR N-to-1 context:" << subc->getDirPath() << endl;
        }
    }

    void
    ManyBaseToOneViewContext::OnCreated( NamingEvent_Created* ev,
                                         const fh_context& subc,
                                         std::string olddn, std::string newdn )
    {
//        fh_context subc = ev->getSource()->getSubContext( olddn );

        if( shouldInsertContext( subc, true ) )
        {
            cascadedInsert( GetImpl(subc), true );
        }
        else
        {
            LG_CTX_D << "OnCreated() not inserting FOR N-to-1 context:" << subc->getDirPath() << endl;
        }
    }

    void
    ManyBaseToOneViewContext::read( bool force )
    {
        if( ReadingDir )
            return;

        EnsureStartStopReadingIsFiredRAII _raii1( this );
        ReadingDirRAII _raiird1( this, true );
        
        if( !HaveReadDir )
        {
            setup();
            HaveReadDir = true;
        }
        else
        {
            emitExistsEventForEachItem();
        }
    }

    bool
    ManyBaseToOneViewContext::allBaseContextsUseSameSorting()
    {
        m_baseContexts_t::iterator lastiter = m_baseContexts.begin();
        m_baseContexts_t::iterator bi =     m_baseContexts.begin();
        bool differentSorting = false;
        for( ++bi; bi != m_baseContexts.end(); ++bi )
        {
            if( ITEMS_KEY_COMP( (*bi)->getItems() )
                != ITEMS_KEY_COMP( (*lastiter)->getItems() ) )
            {
                differentSorting = true;
                break;
            }
        }
        return differentSorting;
    }

    void
    ManyBaseToOneViewContext::cannibalRemove( Context* c )
    {
//         cerr << "ManyBaseToOneViewContext::cannibalRemove()"
//              << " filter:" << getStrAttr( this, "filter", "" )
//              << " c:" << c->getURL() << endl;
        Remove( c, false );
    }

    void
    ManyBaseToOneViewContext::updateViewForCannibalizm()
    {
        for( m_baseContexts_t::iterator bi = m_baseContexts.begin();
             bi != m_baseContexts.end(); ++bi )
        {
            fh_context c = *bi;
            ManyBaseToOneViewContext* bc
                = dynamic_cast<ManyBaseToOneViewContext*>( GetImpl(c) );
            if( !bc )
                continue;
            
            for( Items_t::iterator iter = getItems().begin(); iter != getItems().end(); ++iter )
            {
                bc->cannibalRemove( getUnderlyingContext( *iter ) );
            }
        }
    }
    


    bool
    ManyBaseToOneViewContext::hasSetupBeenCalled()
    {
        return m_hasSetupBeenCalled;
    }
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    UnionContext::UnionContext( fh_context parent, fh_context ctx, bool isCannibal )
        :
        _Base( parent, ctx, isCannibal ),
        m_unionIsOverFilteredViewsOnly( false )
    {}
    
    UnionContext::~UnionContext()
    {}

    bool
    UnionContext::shouldInsertContext( const fh_context& c, bool created )
    {
        LG_CTX_D << "UnionContext::shouldInsertContext() c:" << c->getURL()
                 << " this:" << getURL()
                 << " ret:" << !priv_isSubContextBound( c->getDirName() )
                 << endl;
        return !priv_discoveredSubContext( c->getDirName(), created );
    }
    
    void
    UnionContext::cascadedInsert( Context* c, bool created )
    {
        fh_context cc = new UnionContext( this, c );
        Insert( GetImpl(cc), created );
    }

    void
    UnionContext::setup()
    {
        if( hasSetupBeenCalled() )
            return;
        
        bool differentSorting = allBaseContextsUseSameSorting();
        LG_CTX_D << "UnionContext::setup() base.sz:" << m_baseContexts.size()
                 << " differentSorting:" << differentSorting
                 << endl;

        //
        // Use set_union() if possible.
        //
        if( !differentSorting )
        {
            //
            // FIXME: if the underlying contexts use filtering or sorting then this method will
            //        not work. We need to be able to get to the "raw" context, ie the Context*
            //        for the transitive 'Delegate' if we are operating on ChainedContext subclasses
            //


            /**
             * If all the base contexts are filtered contexts with the same path then turn on
             * raw ptr comparison optimization
             */
            {
                m_unionIsOverFilteredViewsOnly = true;
                string neededPath = Delegate->getDirPath();
                
                for( m_baseContexts_t::iterator bi = m_baseContexts.begin(); bi != m_baseContexts.end(); ++bi )
                {
                    fh_context c = *bi;
                    FilteredContext* rawc = dynamic_cast<FilteredContext*>( GetImpl(c) );
                    if( !rawc || rawc->getDirPath() != neededPath )
                    {
                        m_unionIsOverFilteredViewsOnly = false;
                        break;
                    }
                }
            }
            

            
            if( m_unionIsOverFilteredViewsOnly )
            {
                typedef set< Context* > Tmp_t;
                Tmp_t  tmp1;
                Tmp_t  tmp2;
                Tmp_t* tin  = &tmp1;
                Tmp_t* tout = &tmp2;

                LG_CTX_D << " m_unionIsOverFilteredViewsOnly:" << m_unionIsOverFilteredViewsOnly << endl;
                
                m_baseContexts_t::iterator bi = m_baseContexts.begin();
                (*bi)->read();
                for( Items_t::iterator iter = (*bi)->getItems().begin(); iter != (*bi)->getItems().end(); ++iter )
                {
                    tin->insert( getUnderlyingContext( *iter ) );
                }
    
                for( ++bi; bi != m_baseContexts.end(); ++bi )
                {
                    Tmp_t z;
                    (*bi)->read();
                    for( Items_t::iterator iter = (*bi)->getItems().begin(); iter != (*bi)->getItems().end(); ++iter )
                    {
                        z.insert( getUnderlyingContext( *iter ) );
                    }
                
                    set_union( tin->begin(), tin->end(),
                               z.begin(), z.end(),
                               inserter( *tout, tout->end() ) );

//                     cerr << "union working... DUMP START (using raw ptrs)" << endl;
//                     for( Tmp_t::iterator ti = tout->begin(); ti != tout->end(); ++ti )
//                     {
//                         cerr << "union working... c:" << (*ti)->getURL() << " v:" << toVoid( GetImpl(*ti) ) << endl;
//                     }

                    swap( tin, tout );
                    tout->clear();
                }

//                 // PURE DEBUG
//                 for( Tmp_t::iterator ti = tin->begin(); ti != tin->end(); ++ti )
//                 {
//                     cerr << "union result... c:" << (*ti)->getURL() << " v:" << toVoid( GetImpl(*ti) ) << endl;
//                 }

                
                
                for( Tmp_t::iterator ti = tin->begin(); ti != tin->end(); ++ti )
                {
                    cascadedInsert( *ti, false );
                }
            }
            else
            {
                
                typedef list< fh_context > Tmp_t;
                Tmp_t  tmp1;
                Tmp_t  tmp2;
                Tmp_t* tin  = &tmp1;
                Tmp_t* tout = &tmp2;
            
                m_baseContexts_t::iterator bi = m_baseContexts.begin();
                (*bi)->read();
                copy( (*bi)->getItems().begin(), (*bi)->getItems().end(), back_inserter( *tin ) );
    
                for( ++bi; bi != m_baseContexts.end(); ++bi )
                {
                    (*bi)->read();
                    ContextSetCompare csc;
                    set_union( tin->begin(), tin->end(),
                               (*bi)->getItems().begin(), (*bi)->getItems().end(),
                               back_inserter( *tout ),
                               csc );

//                     cerr << "union working... DUMP START" << endl;
//                     for( Tmp_t::iterator ti = tout->begin(); ti != tout->end(); ++ti )
//                     {
//                         cerr << "union working... c:" << (*ti)->getURL() << " v:" << toVoid( GetImpl(*ti) ) << endl;
//                     }

                    swap( tin, tout );
                    tout->clear();
                }

                for( Tmp_t::iterator ti = tin->begin(); ti != tin->end(); ++ti )
                {
                    cascadedInsert( GetImpl(*ti) , false );
                }
            }
        }
        else
        {
            for( m_baseContexts_t::iterator bi = m_baseContexts.begin();
                 bi != m_baseContexts.end(); bi ++ )
            {
                (*bi)->read();
                Items_t& bitems = (*bi)->getItems();

                for( Items_t::iterator iter = bitems.begin(); iter != bitems.end(); ++iter )
                {
                    LG_CTX_D << "UnionContext::setup() different sorting iter:" << (*iter)->getURL() << endl;
                
                    if( shouldInsertContext( *iter, false ) )
                        cascadedInsert( GetImpl(*iter) , false );
                }
            }
        }
        
        _Base::setup();
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    DifferenceContext::DifferenceContext( fh_context parent, fh_context ctx, bool isCannibal )
        :
        _Base( parent, ctx, isCannibal )
    {}
    
    DifferenceContext::~DifferenceContext()
    {}

    void
    DifferenceContext::cascadedInsert( Context* c, bool created )
    {
        fh_context cc = new DifferenceContext( this, c );
        Insert( GetImpl(cc), created );
    }

    bool
    DifferenceContext::shouldInsertContext( const fh_context& c, bool created )
    {
        string rdn = c->getDirName();

        // dont bind it twice for ourself.
        if( priv_discoveredSubContext( rdn, created ) )
            return false;

        m_baseContexts_t::iterator bi = m_baseContexts.begin();
        
        for( ++bi; bi != m_baseContexts.end(); ++bi )
        {
            if( (*bi)->isSubContextBound( rdn ))
                return false;
        }
        return true;
    }
    
    void
    DifferenceContext::setup()
    {
        if( hasSetupBeenCalled() )
            return;

        LG_CTX_D << "DifferenceContext::setup() base.sz:" << m_baseContexts.size() << endl;

        bool differentSorting = allBaseContextsUseSameSorting();
        typedef list< fh_context > Tmp_t;
        Tmp_t  tmp1;
        Tmp_t  tmp2;
        Tmp_t* tin  = &tmp1;
        Tmp_t* tout = &tmp2;
            
        m_baseContexts_t::iterator bi = m_baseContexts.begin();
        (*bi)->read();
        copy( (*bi)->getItems().begin(), (*bi)->getItems().end(), back_inserter( *tin ) );
    
        for( ++bi; bi != m_baseContexts.end(); ++bi )
        {
            (*bi)->read();
            if( differentSorting )
            {
                //
                // We have to create a temporary set which is sorted by name and use that ordering in
                // each set_intersection() call
                //
                typedef map< string, fh_context > SortedTemp_t;
                SortedTemp_t SortedTemp;
                Items_t& bitems = (*bi)->getItems();

                for( Items_t::iterator iter = bitems.begin(); iter != bitems.end(); ++iter )
                {
                    SortedTemp[ (*iter)->getDirName() ] = *iter;
                }

                ContextSetCompare csc;
                set_difference( tin->begin(), tin->end(),
                                map_range_iterator( SortedTemp.begin() ),
                                map_range_iterator( SortedTemp.end() ),
                                back_inserter( *tout ),
                                csc );
            }
            else
            {
                set_difference( tin->begin(), tin->end(),
                                (*bi)->getItems().begin(), (*bi)->getItems().end(),
                                back_inserter( *tout ),
                                ITEMS_KEY_COMP( (*bi)->getItems() ) );
            }
            swap( tin, tout );
            tout->clear();
        }

        for( Tmp_t::iterator ti = tin->begin(); ti != tin->end(); ++ti )
        {
//            if( shouldInsertContext( *ti ) )
                cascadedInsert( GetImpl(*ti) , false );
        }
        
        _Base::setup();
        
//         Delegate->read();
        
//         for( Items_t::iterator iter = Delegate->getItems().begin();
//              iter != Delegate->getItems().end(); ++iter )
//         {
//             LG_CTX_D << "DifferenceContext::setup() iter:" << (*iter)->getURL() << endl;
            
//             if( shouldInsertContext( *iter ) )
//                 cascadedInsert( GetImpl(*iter) , false );
//         }
        
//         _Base::setup();
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    SetIntersectionContext::SetIntersectionContext( fh_context parent,
                                                    fh_context ctx,
                                                    bool isCannibal )
        :
        _Base( parent, ctx, isCannibal )
    {}
    
    SetIntersectionContext::~SetIntersectionContext()
    {}

    void
    SetIntersectionContext::cascadedInsert( Context* c, bool created )
    {
        fh_context cc = new SetIntersectionContext( this, c );
        Insert( GetImpl(cc), created );
    }

    bool
    SetIntersectionContext::shouldInsertContext( const fh_context& c, bool created )
    {
        string rdn = c->getDirName();

        // dont bind it twice for ourself.
        if( priv_discoveredSubContext( rdn, created ) )
            return false;

        // only bind it if its bound in every child
        bool ret = true;
        for( m_baseContexts_t::iterator bi = m_baseContexts.begin(); bi != m_baseContexts.end(); ++bi )
        {
            if( !(*bi)->isSubContextBound( rdn ))
            {
                return false;
            }
        }
        return ret;
    }
    
    void
    SetIntersectionContext::setup()
    {
        if( hasSetupBeenCalled() )
            return;

        LG_CTX_D << "SetIntersectionContext::setup() base.sz:" << m_baseContexts.size() << endl;

        bool differentSorting = allBaseContextsUseSameSorting();
        typedef list< fh_context > Tmp_t;
        Tmp_t  tmp1;
        Tmp_t  tmp2;
        Tmp_t* tin  = &tmp1;
        Tmp_t* tout = &tmp2;

        m_baseContexts_t::iterator bi = m_baseContexts.begin();
        (*bi)->read();
        copy( (*bi)->getItems().begin(), (*bi)->getItems().end(), back_inserter( *tin ) );
    
        for( ++bi; bi != m_baseContexts.end(); ++bi )
        {
            (*bi)->read();
            if( differentSorting )
            {
                LG_CTX_D << "SetIntersectionContext::setup() they use different sorting!" << endl;
                //
                // We have to create a temporary set which is sorted by name and use that ordering in
                // each set_intersection() call
                //
                typedef map< string, fh_context > SortedTemp_t;
                SortedTemp_t SortedTemp;
                Items_t& bitems = (*bi)->getItems();

                for( Items_t::iterator iter = bitems.begin(); iter != bitems.end(); ++iter )
                {
                    SortedTemp[ (*iter)->getDirName() ] = *iter;
                }

                ContextSetCompare csc;
                set_intersection( tin->begin(), tin->end(),
                                  map_range_iterator( SortedTemp.begin() ),
                                  map_range_iterator( SortedTemp.end() ),
                                  back_inserter( *tout ),
                                  csc );
            }
            else
            {
                set_intersection( tin->begin(), tin->end(),
                                  (*bi)->getItems().begin(), (*bi)->getItems().end(),
                                  back_inserter( *tout ),
                                  ITEMS_KEY_COMP( (*bi)->getItems() ) );
                
//                 for( Tmp_t::iterator ti = tout->begin(); ti != tout->end(); ++ti )
//                 {
//                     cerr << "intersect working... c:" << (*ti)->getURL() << endl;
//                 }
            }
            tin->clear();
            swap( tin, tout );
        }

        for( Tmp_t::iterator ti = tin->begin(); ti != tin->end(); ++ti )
        {
//            if( shouldInsertContext( *ti ) )
                cascadedInsert( GetImpl(*ti) , false );

                if( m_isCannibal )
                {
//                    cerr << "SetIntersectionContext::setup() cannibal start sz:"
//                         << m_baseContexts.size() << endl;
                    
                    for( m_baseContexts_t::iterator bi = m_baseContexts.begin();
                         bi != m_baseContexts.end(); ++bi )
                    {
                        fh_context c = *bi;
                        if( ManyBaseToOneViewContext* bc
                            = dynamic_cast<ManyBaseToOneViewContext*>( GetImpl(c) ))
                        {
                            bc->cannibalRemove( GetImpl(*ti) );
                        }
                    }
//                    cerr << "SetIntersectionContext::setup() cannibal end" << endl;
                }
        }
        
        _Base::setup();
        updateViewForCannibalizm();
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    SetSymmetricDifferenceContext::SetSymmetricDifferenceContext(
        fh_context parent,
        fh_context ctx,
        bool isCannibal )
        :
        _Base( parent, ctx, isCannibal )
    {}
    
    SetSymmetricDifferenceContext::~SetSymmetricDifferenceContext()
    {}

    void
    SetSymmetricDifferenceContext::cascadedInsert( Context* c, bool created )
    {
        fh_context cc = new SetSymmetricDifferenceContext( this, c );
        Insert( GetImpl(cc), created );
    }

    bool
    SetSymmetricDifferenceContext::shouldInsertContext( const fh_context& c, bool created )
    {
        string rdn = c->getDirName();

        // dont bind it twice for ourself.
        if( priv_discoveredSubContext( rdn, created ) )
            return false;

        //
        // set_sym_diff should be true if the new context is only bound in exactly
        // one of the base contexts
        //
        int count = 0;
        for( m_baseContexts_t::iterator bi = m_baseContexts.begin(); bi != m_baseContexts.end(); ++bi )
        {
            if( (*bi)->isSubContextBound( rdn ))
                ++count;
        }
        return count == 1;
    }
    
    void
    SetSymmetricDifferenceContext::setup()
    {
        if( hasSetupBeenCalled() )
            return;

        LG_CTX_D << "SetSymmetricDifferenceContext::setup() base.sz:" << m_baseContexts.size() << endl;

        bool differentSorting = allBaseContextsUseSameSorting();
        typedef list< fh_context > Tmp_t;
        Tmp_t  tmp1;
        Tmp_t  tmp2;
        Tmp_t* tin  = &tmp1;
        Tmp_t* tout = &tmp2;
            
        for( m_baseContexts_t::iterator bi = m_baseContexts.begin(); bi != m_baseContexts.end(); ++bi )
        {
            (*bi)->read();
            if( differentSorting )
            {
                //
                // We have to create a temporary set which is sorted by name and use that ordering in
                // each set_intersection() call
                //
                typedef map< string, fh_context > SortedTemp_t;
                SortedTemp_t SortedTemp;
                Items_t& bitems = (*bi)->getItems();

                for( Items_t::iterator iter = bitems.begin(); iter != bitems.end(); ++iter )
                {
                    SortedTemp[ (*iter)->getDirName() ] = *iter;
                }

                ContextSetCompare csc;
                set_symmetric_difference( tin->begin(), tin->end(),
                                          map_range_iterator( SortedTemp.begin() ),
                                          map_range_iterator( SortedTemp.end() ),
                                          back_inserter( *tout ),
                                          csc );
            }
            else
            {
                LG_CTX_D << "set_symmetric_difference() same sorting order. bi:" << (*bi)->getURL() << endl;
                set_symmetric_difference( tin->begin(), tin->end(),
                                          (*bi)->getItems().begin(), (*bi)->getItems().end(),
                                          back_inserter( *tout ),
                                          ITEMS_KEY_COMP( (*bi)->getItems() ) );

//                 for( Tmp_t::iterator ti = tout->begin(); ti != tout->end(); ++ti )
//                 {
//                     cerr << "sym_diff working... c:" << (*ti)->getURL() << endl;
//                 }
                
            }
            swap( tin, tout );
            tout->clear();
        }

        for( Tmp_t::iterator ti = tin->begin(); ti != tin->end(); ++ti )
        {
            LG_CTX_D << "sym_diff setup():" << (*ti)->getURL() << endl;
        }

        for( Tmp_t::iterator ti = tin->begin(); ti != tin->end(); ++ti )
        {
//            if( shouldInsertContext( *ti ) )
                cascadedInsert( GetImpl(*ti) , false );
        }
        
        _Base::setup();
    }


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    ManyBaseToOneChainedViewContext::ManyBaseToOneChainedViewContext(
        fh_context parent,
        fh_context ctx,
        bool isCannibal )
        :
        _Base( parent, ctx, isCannibal )
    {}
    
    ManyBaseToOneChainedViewContext::~ManyBaseToOneChainedViewContext()
    {}
    

    void
    ManyBaseToOneChainedViewContext::cascadedInsert( Context* c, bool created )
    {
        fh_context cc = new ManyBaseToOneChainedViewContext( this, c );
        Insert( GetImpl(cc), created );
    }

    bool
    ManyBaseToOneChainedViewContext::shouldInsertContext( const fh_context& c, bool created )
    {
        return priv_discoveredSubContext( c->getDirName(), created );
    }

    void
    ManyBaseToOneChainedViewContext::setup()
    {
        if( hasSetupBeenCalled() )
            return;

        for( m_baseContexts_t::iterator bi = m_baseContexts.begin(); bi != m_baseContexts.end(); ++bi )
        {
            (*bi)->read();
            Items_t& bitems = (*bi)->getItems();
            for( Items_t::iterator iter = bitems.begin(); iter != bitems.end(); ++iter )
            {
                cascadedInsert( GetImpl(*iter) , false );
            }
        }
        
        _Base::setup();
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    DiffContext::DiffContext( fh_context parent, fh_context ctx, bool isCannibal )
        :
//        _Base( parent, ctx, isCannibal )
        _Base( GetImpl(parent), (std::string)"" )
    {
        Delegate = ctx;
        setContext( 0, ctx->getDirName() );
        common_setup( parent, ctx, isCannibal );
        
        m_runidiff = new Runner();
        m_runidiff->setSpawnFlags(
            GSpawnFlags(
                G_SPAWN_SEARCH_PATH |
                G_SPAWN_LEAVE_DESCRIPTORS_OPEN |
//                G_SPAWN_FILE_AND_ARGV_ZERO |
                G_SPAWN_STDERR_TO_DEV_NULL |
//                         G_SPAWN_STDOUT_TO_DEV_NULL |
                m_runidiff->getSpawnFlags()));
    }

    DiffContext::~DiffContext()
    {}

    std::string
    DiffContext::getURL()
    {
        fh_stringstream ss;
        ss << "diff://";
        if( !m_baseContexts.empty() )
        {
            m_baseContexts_t::iterator bi = m_baseContexts.begin();
            ss << (*bi)->getURL();
            ++bi;
            if( bi != m_baseContexts.end() )
            {
                ss << (*bi)->getURL();
            }
        }
        return tostr(ss);
    }
    
    DiffContext*
    DiffContext::getParentDiffContext()
    {
        Context* c = getParent();
        return dynamic_cast<DiffContext*>(c);
    }
    
    
    bool
    DiffContext::haveTwoContexts( DiffContext* c )
    {
        bool   ret = false;
        string rdn = c->getDirName();
        
        if( !m_baseContexts.empty() )
        {
            m_baseContexts_t::iterator bi = m_baseContexts.begin();
            if( (*bi)->isSubContextBound( rdn ))
            {
                ++bi;
                if( (*bi)->isSubContextBound( rdn ))
                {
                    ret = true;
                }
            }
        }
        return ret;
    }
    

    fh_context DiffContext::getFirstContext( DiffContext* c )
    {
        if( m_baseContexts.empty() )
            return 0;
        
        m_baseContexts_t::iterator bi = m_baseContexts.begin();
        return (*bi)->getSubContext( c->getDirName() );
    }
    
    fh_context DiffContext::getSecondContext( DiffContext* c )
    {
        if( m_baseContexts.size() < 2 )
            return 0;
        
        m_baseContexts_t::iterator bi = m_baseContexts.begin();
        ++bi;
        return (*bi)->getSubContext( c->getDirName() );
    }
    
    
    fh_stringstream
    DiffContext::SL_wasCreated( DiffContext* c, const std::string& ean, EA_Atom* atom )
    {
        bool ret   = false;
        string rdn = c->getDirName();
        DiffContext*      p              = c->getParentDiffContext();
        m_baseContexts_t& m_baseContexts = p->m_baseContexts;
        fh_runner&        m_runidiff     = p->m_runidiff;
        
        if( !m_baseContexts.empty() )
        {
            m_baseContexts_t::iterator bi = m_baseContexts.begin();
            ++bi;
            if( !(*bi)->isSubContextBound( rdn ))
            {
                ret = true;
            }
        }
        
        fh_stringstream ss;
        ss << ret;
        return ss;
    }
    
    fh_stringstream
    DiffContext::SL_wasDeleted( DiffContext* c, const std::string& ean, EA_Atom* atom )
    {
        bool ret = false;

        string rdn = c->getDirName();
        DiffContext*      p              = c->getParentDiffContext();
        m_baseContexts_t& m_baseContexts = p->m_baseContexts;
        fh_runner&        m_runidiff     = p->m_runidiff;

        if( !m_baseContexts.empty() )
        {
            m_baseContexts_t::iterator bi = m_baseContexts.begin();
            if( !(*bi)->isSubContextBound( rdn ))
            {
                LG_CTX_D << "DiffContext::SL_wasDeleted bi:" << (*bi)->getURL()
                         << " rdn:" << rdn
                         << " c:" << c->getURL() << endl;
                
                ret = true;
            }
        }
        
        fh_stringstream ss;
        ss << ret;
        return ss;
    }

    fh_stringstream
    DiffContext::SL_isSame( DiffContext* c, const std::string& ean, EA_Atom* atom )
    {
        bool ret = false;
        DiffContext*      p              = c->getParentDiffContext();
        m_baseContexts_t& m_baseContexts = p->m_baseContexts;
        fh_runner&        m_runidiff     = p->m_runidiff;

        if( p->haveTwoContexts( c ) )
        {
            fh_stringstream cmdss;
            cmdss << getConfigString( FDB_GENERAL, 
                                      CFG_ATTRIBUTES_GNU_DIFF_CMD_FILES_K,
                                      CFG_ATTRIBUTES_GNU_DIFF_CMD_FILES_DEFAULT )
                  << " " << p->getFirstContext( c )->getDirPath()
                  << " " << p->getSecondContext( c )->getDirPath();

            LG_CTX_D << "diff command:" << tostr(cmdss) << endl;
            m_runidiff->setCommandLine( tostr(cmdss) );
            m_runidiff->Run();
            gint e = m_runidiff->getExitStatus();
            LG_CTX_D << "diff ret:" << e << endl;
            
            // e == 0 for no diff, e == 1 for diff, e == 2 for error
            ret = (e==0);
        }

        LG_CTX_D << "DiffContext::SL_isSame() c:" << c->getURL() << " ret:" << ret << endl;
        
        fh_stringstream ss;
        ss << ret;
        return ss;
    }

    fh_stringstream
    DiffContext::SL_isSameBytes( DiffContext* c, const std::string& rdn, EA_Atom* atom )
    {
        bool ret = true;
        DiffContext*      p              = c->getParentDiffContext();
        m_baseContexts_t& m_baseContexts = p->m_baseContexts;
        fh_runner&        m_runidiff     = p->m_runidiff;

        if( !p->haveTwoContexts( c ) )
        {
            ret = false;
        }
        else
        {
            fh_context fc = p->getFirstContext( c );
            fh_context sc = p->getSecondContext( c );
            
            fh_istream fiss = fc->getIStream();
            fh_istream siss = sc->getIStream();
            string fs;
            string ss;
                
            // compare front (fiss) with second (siss) istreams
            while( getline( fiss, fs ) && getline( siss, ss ) )
            {
                if( fs != ss )
                {
                    ret = false;
                    break;
                }
            }
        }
        
        fh_stringstream ss;
        ss << ret;
        return ss;
    }
    
    fh_stringstream
    DiffContext::SL_getUniDiff_Native( DiffContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        DiffContext*      p              = c->getParentDiffContext();
        m_baseContexts_t& m_baseContexts = p->m_baseContexts;
        fh_runner&        m_runidiff     = p->m_runidiff;

        fh_stringstream cmdss;
        cmdss << getConfigString( FDB_GENERAL, 
                                  CFG_ATTRIBUTES_GNU_DIFF_CMD_FILES_K,
                                  CFG_ATTRIBUTES_GNU_DIFF_CMD_FILES_DEFAULT )
              << " " << p->getFirstContext( c )->getDirPath()
              << " " << p->getSecondContext( c )->getDirPath();
        m_runidiff->setCommandLine( tostr(cmdss) );
        m_runidiff->Run();
            
        fh_istream outss = m_runidiff->getStdOut();
            
        for( string s; getline( outss, s );  )
        {
            LG_CTX_D << "diff line:" << s << endl;
            ss << s << endl;
        }

        LG_CTX_D << "SL_getUniDiff cmd:" << tostr(cmdss)
                 << " out:" << tostr(ss)
                 << endl;

        gint e = m_runidiff->getExitStatus();
        // e == 0 for no diff, e == 1 for diff, e == 2 for error
        LG_CTX_D << "SL_getUniDiff() cmd:" << tostr(cmdss)
                 << " e:" << e
                 << endl;
            
//        if( e == 1 )

        return ss;
    }
    
    fh_stringstream
    DiffContext::SL_getUniDiff_Remote( DiffContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        DiffContext*      p              = c->getParentDiffContext();
        m_baseContexts_t& m_baseContexts = p->m_baseContexts;
        fh_runner&        m_runidiff     = p->m_runidiff;

        fh_context c1 = p->getFirstContext( c );
        fh_context c2 = p->getSecondContext( c );

#ifdef HAVE_NO_DEV_FD_INTERFACE        
            fh_stringstream ss;
            ss << "System lacks support for /dev/fd so unidiffs can only be"
               << " done for local files"
               << " for c1:" << c1->getURL()
               << " c2:" << c2->getURL() << endl;
            Throw_CanNotGetStream( tostr(ss), c );
#endif
        
        
        //
        // This seems a little tricky at first,
        // basically we create two fcat subprocs: one for each of the files
        // then we use /dev/fd/ to pass open file descriptors to diff which
        // are connected to the stdout of each of the fcat commands.
        // We then read the stdout of the diff proc to be the unidiff result
        //
    
        fh_runner fcat_runner1 = new Runner();
        fcat_runner1->setSpawnFlags(
            GSpawnFlags(
                G_SPAWN_SEARCH_PATH | G_SPAWN_STDERR_TO_DEV_NULL | fcat_runner1->getSpawnFlags()));
        fh_runner fcat_runner2 = new Runner();
        fcat_runner2->setSpawnFlags( GSpawnFlags( fcat_runner1->getSpawnFlags()));
        
        fcat_runner1->setCommandLine( "fcat " + c1->getURL() );
        fcat_runner1->Run();
        int fcat_stdout_fd1 = fcat_runner1->getStdOutFd();
        fcat_runner2->setCommandLine( "fcat " + c2->getURL() );
        fcat_runner2->Run();
        int fcat_stdout_fd2 = fcat_runner2->getStdOutFd();
    
        //
        // now we need to hookup the fcat fds to the args of diff using /dev/fd/
        // and then read diffs result
        //
        fh_stringstream cmdss;
        cmdss << getConfigString( FDB_GENERAL, 
                                  CFG_ATTRIBUTES_GNU_DIFF_CMD_FILES_K,
                                  CFG_ATTRIBUTES_GNU_DIFF_CMD_FILES_DEFAULT )
              << " /dev/fd/" << fcat_stdout_fd1
              << " /dev/fd/" << fcat_stdout_fd2;
        m_runidiff->setCommandLine( tostr(cmdss) );
        m_runidiff->Run();
        
        fh_istream outss = m_runidiff->getStdOut();
        for( string s; getline( outss, s );  )
        {
            LG_CTX_D << "diff line:" << s << endl;
            ss << s << endl;
        }
        gint e = m_runidiff->getExitStatus();
        // e == 0 for no diff, e == 1 for diff, e == 2 for error
        if( e > 1 )
        {
            // Oh no.
            fh_stringstream ss;
            ss << "Error getting unidiff e:" << e
               << " for c1:" << c1->getURL()
               << " c2:" << c2->getURL() << endl;
            Throw_CanNotGetStream( tostr(ss), c );
        }

        return ss;
    }
    
    fh_stringstream
    DiffContext::SL_getUniDiff( DiffContext* c, const std::string& rdn, EA_Atom* atom )
    {
        DiffContext*      p              = c->getParentDiffContext();
        m_baseContexts_t& m_baseContexts = p->m_baseContexts;
        fh_runner&        m_runidiff     = p->m_runidiff;
        
        if( p->haveTwoContexts( c ) )
        {
            if( isTrue( getStrAttr( p->getFirstContext( c ), "is-native", "no" ))
                && isTrue( getStrAttr( p->getSecondContext( c ), "is-native", "no" )) )
            {
                return SL_getUniDiff_Native( c, rdn, atom );
            }
            else
            {
                return SL_getUniDiff_Remote( c, rdn, atom );
            }
        }

        fh_stringstream ss;
        return ss;
    }
    
    fh_stringstream
    DiffContext::SL_getDifferentLineCount( DiffContext* c, const std::string& rdn, EA_Atom* atom )
    {
        DiffContext*      p              = c->getParentDiffContext();
        m_baseContexts_t& m_baseContexts = p->m_baseContexts;
        fh_runner&        m_runidiff     = p->m_runidiff;
        int lineCount = 0;
        string s;
        
        fh_stringstream unidiff = SL_getUniDiff( c, rdn, atom );
        while( getline( unidiff, s ) )
        {
            if( starts_with( s, "+++ " ) || starts_with( s, "--- " ) )
                continue;
            
            if( starts_with( s, "+" ) || starts_with( s, "-" ) )
            {
                LG_CTX_D << "lc++:" << s << endl;
                ++lineCount;
            }
        }
        
        fh_stringstream ss;
        ss << lineCount;
        return ss;
    }

    fh_stringstream
    DiffContext::SL_getLinesAddedCount( DiffContext* c, const std::string& rdn, EA_Atom* atom )
    {
        DiffContext*      p              = c->getParentDiffContext();
        m_baseContexts_t& m_baseContexts = p->m_baseContexts;
        fh_runner&        m_runidiff     = p->m_runidiff;
        int lineCount = 0;
        string s;
        
        fh_stringstream unidiff = SL_getUniDiff( c, rdn, atom );
        while( getline( unidiff, s ) )
        {
            if( starts_with( s, "+" ) && !starts_with( s, "+++" ) )
            {
                ++lineCount;
            }
        }
        
        fh_stringstream ss;
        ss << lineCount;
        return ss;
    }
    
    fh_stringstream
    DiffContext::SL_getLinesRemovedCount( DiffContext* c, const std::string& rdn, EA_Atom* atom )
    {
        DiffContext*      p              = c->getParentDiffContext();
        m_baseContexts_t& m_baseContexts = p->m_baseContexts;
        fh_runner&        m_runidiff     = p->m_runidiff;
        int lineCount = 0;
        string s;
        
        fh_stringstream unidiff = SL_getUniDiff( c, rdn, atom );
        while( getline( unidiff, s ) )
        {
            if( starts_with( s, "-" ) && !starts_with( s, "---" ) )
            {
                ++lineCount;
            }
        }
        
        fh_stringstream ss;
        ss << lineCount;
        return ss;
    }
        

    
    
    void
    DiffContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
#define SLEA  tryAddStateLessAttribute

            SLEA( "was-created",  &DiffContext::SL_wasCreated,      XSD_BASIC_BOOL );
            SLEA( "was-deleted",  &DiffContext::SL_wasDeleted,      XSD_BASIC_BOOL );
            SLEA( "is-same",      &DiffContext::SL_isSame,          XSD_BASIC_BOOL );
            SLEA( "is-same-bytes",&DiffContext::SL_isSameBytes,     XSD_BASIC_BOOL );
            SLEA( "unidiff",      &DiffContext::SL_getUniDiff,      XSD_BASIC_STRING );
            SLEA( "different-line-count",
                  &DiffContext::SL_getDifferentLineCount, XSD_BASIC_INT );
            SLEA( "lines-added-count",   &DiffContext::SL_getLinesAddedCount,   XSD_BASIC_INT );
            SLEA( "lines-removed-count", &DiffContext::SL_getLinesRemovedCount, XSD_BASIC_INT );

#undef SLEA

            /************************************************************/
            /************************************************************/
            /************************************************************/
        
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }
    
    
    bool
    DiffContext::shouldInsertContext( const fh_context& c, bool created )
    {
        return _Base::shouldInsertContext( c, created );
    }
    
    void
    DiffContext::cascadedInsert( Context* c, bool created )
    {
        fh_context cc = new DiffContext( this, c );
        Insert( GetImpl(cc), created );
    }

    void
    DiffContext::setup()
    {
        createStateLessAttributes();
        _Base::setup();
    }


    std::string
    DiffContext::private_getStrAttr( const std::string& rdn,
                                     const std::string& def,
                                     bool getAllLines,
                                     bool throwEx )
    {
        std::string ret = def;

        LG_CTX_D << "DiffContext::private_getStrAttr(top) rdn:" << rdn << endl;
        
        if( rdn == "ea-names" )
        {
            LG_CTX_D << "DiffContext::private_getStrAttr(ea-names)" << endl;
            
            AttributeCollection::AttributeNames_t an;
            getAttributeNames( an );
            if( an.empty() )   return def;
            if( !getAllLines ) return *(an.begin());

            fh_stringstream ss;
            ss << Util::createCommaSeperatedList( an );
            return tostr(ss);
        }
        
        try
        {
            ret = Delegate->private_getStrAttr( rdn, def, getAllLines, true );
            return ret;
        }
        catch( exception& e )
        {
            return _DontDelegateBase::private_getStrAttr( rdn, def, getAllLines, throwEx );
        }
    }
    
    fh_attribute
    DiffContext::getAttribute( const string& rdn ) throw( NoSuchAttribute )
    {
        fh_attribute ret = 0;

        LG_CTX_D << "DiffContext::getAttribute(top) rdn:" << rdn << endl;
        
        if( rdn == "ea-names" )
        {
            fh_attribute ret = _DontDelegateBase::getAttribute( rdn );
            fh_istream   iss = ret->getIStream();
            return ret;
        }
        
        try
        {
            ret = Delegate->getAttribute( rdn );
            if( ret )
            {
                return ret;
            }
        }
        catch( exception& e )
        {}

        return _DontDelegateBase::getAttribute( rdn );
    }
    
    AttributeCollection::AttributeNames_t&
    DiffContext::getAttributeNames( AttributeNames_t& ret )
    {
        LG_CTX_D << "DiffContext::getAttributeNames()" << endl;

        AttributeCollection::AttributeNames_t t1;
        AttributeCollection::AttributeNames_t t2;
        Delegate->getAttributeNames( t1 );
        _DontDelegateBase::getAttributeNames( t2 );
        return mergeAttributeNames( ret, t1, t2 );
    }
    
    int
    DiffContext::getAttributeCount()
    {
        AttributeCollection::AttributeNames_t tmp;
        getAttributeNames( tmp );
        return tmp.size();
    }

    bool
    DiffContext::isAttributeBound( const std::string& rdn, bool createIfNotThere )
        throw( NoSuchAttribute )
    {
        bool ret = Delegate->isAttributeBound( rdn, createIfNotThere );
        if( !ret )
        {
            return _DontDelegateBase::isAttributeBound( rdn, createIfNotThere );
        }
        return ret;
    }    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    namespace Factory
    {
        template <class T>
        fh_context MakeManyToOneContext( const fh_context& parent,
                                         std::list< fh_context > contextList,
                                         bool isCannibal = false )
        {
            if( contextList.empty() )
            {
                fh_stringstream ss;
                ss << "Can't make a many to one context without any contextlist to use"
                   << " as a model" << endl;
                Throw_BadParam( tostr(ss), 0 );
            }

            std::list< fh_context >::iterator li = contextList.begin();
            fh_context self   = *li;
            T* ret = new T( parent, self, isCannibal );

            for( ++li; li != contextList.end(); ++li )
            {
                ret->appendToBaseContexts( *li );
            }
            
            ret->setup();
            return ret;
        }

        fh_context MakeUnionContext( const fh_context& parent,
                                     std::list< fh_context > unionContexts )
        {
            if( unionContexts.empty() )
            {
                fh_stringstream ss;
                ss << "Can't make a union context without any unionContexts to use"
                   << " as a model" << endl;
                Throw_BadParam( tostr(ss), 0 );
            }
            return MakeManyToOneContext< UnionContext >( parent, unionContexts );
        }
        
        fh_context MakeSetDifferenceContext( const fh_context& parent,
                                             std::list< fh_context > sdContexts )
        {
            if( sdContexts.empty() )
            {
                fh_stringstream ss;
                ss << "Can't make a set difference context without any sdContexts to use"
                   << " as a model" << endl;
                Throw_BadParam( tostr(ss), 0 );
            }
            return MakeManyToOneContext< DifferenceContext >( parent, sdContexts );
        }

        fh_context MakeSetIntersectionContext( const fh_context& parent,
                                               std::list< fh_context > sdContexts,
                                               bool isCannibal )
        {
            if( sdContexts.empty() )
            {
                fh_stringstream ss;
                ss << "Can't make a set intersection context without any sdContexts to use"
                   << " as a model" << endl;
                Throw_BadParam( tostr(ss), 0 );
            }
            return MakeManyToOneContext< SetIntersectionContext >( parent, sdContexts, isCannibal );
        }

        fh_context MakeSetSymmetricDifferenceContext( const fh_context& parent,
                                                      std::list< fh_context > sdContexts )
        {
            if( sdContexts.empty() )
            {
                fh_stringstream ss;
                ss << "Can't make a set symmetric difference context without any sdContexts to use"
                   << " as a model" << endl;
                Throw_BadParam( tostr(ss), 0 );
            }
            return MakeManyToOneContext< SetSymmetricDifferenceContext >( parent, sdContexts );
        }

        fh_context MakeManyBaseToOneChainedViewContext( const fh_context& c )
        {
            std::list< fh_context > sdContexts;
            sdContexts.push_back( c );
            return MakeManyToOneContext< ManyBaseToOneChainedViewContext >
                ( c->getParent(), sdContexts );
        }
        
        fh_context MakeDiffContext( const fh_context& parent,
                                    std::list< fh_context > sdContexts )
        {
            if( sdContexts.size() < 2 )
            {
                fh_stringstream ss;
                ss << "Can't make a set diff context without 2 or more sdContexts to use"
                   << " as a model" << endl;
                Throw_BadParam( tostr(ss), 0 );
            }
            return MakeManyToOneContext< DiffContext >( parent, sdContexts );
        }
        
        
        
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * A context that will pass on requests for attributes to the parent
     * if they are not bound at this context.
     */
    class FERRISEXP_DLLLOCAL InheritingEAContext
        :
        public ChainedViewContext
    {
    public:

        InheritingEAContext( Context* theParent, const fh_context& ctx );
        virtual ~InheritingEAContext();

        virtual void OnDeleted( NamingEvent_Deleted* ev, std::string olddn, std::string newdn );
        virtual void OnExists ( NamingEvent_Exists* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn );
        virtual void OnCreated( NamingEvent_Created* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn );

        virtual void read( bool force = 0 );

        // Setup is only to be called from the factory function!
        void setup();

    protected:

        virtual Context* priv_CreateContext( Context* parent, std::string rdn );
        void wrapInsert( const fh_context& c, bool created = false )
            {
                string rdn = c->getDirName();
                
                if( !priv_discoveredSubContext( rdn, created ) )
                {
                    fh_context cc = new InheritingEAContext( this, c );
                    Insert( GetImpl(cc), created );
                }
            }
        


        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /*** Handle ability to have local attributes aswell as those of delegate ********/
        /********************************************************************************/
        
        int getNumberOfLocalAttributes();
        std::list< std::string >& getLocalAttributeNames();
        
        virtual std::string private_getStrAttr( const std::string& rdn,
                                                const std::string& def = "",
                                                bool getAllLines = false ,
                                                bool throwEx = false );
    public:
        
        virtual fh_attribute getAttribute( const std::string& rdn ) throw( NoSuchAttribute );
        virtual AttributeNames_t& getAttributeNames( AttributeNames_t& ret );
        virtual int  getAttributeCount();
        virtual bool isAttributeBound( const std::string& rdn,
                                       bool createIfNotThere = true
            ) throw( NoSuchAttribute );
    };

    FERRISEXP_DLLLOCAL bool isInheritingContext( Context* c )
    {
        if( dynamic_cast<InheritingEAContext*>(c) )
            return true;
        return false;
    }

    InheritingEAContext::InheritingEAContext( Context* theParent,
                                              const fh_context& ctx )
        :
        ChainedViewContext(ctx, false)
    {
        Context* p = theParent;
        if( !p && ctx->isParentBound() )
            p = ctx->getParent();
        
        setContext( p, ctx->getDirName() );
    }
    
    InheritingEAContext::~InheritingEAContext()
    {
    }
    
/** 
 * Perform initial setup of items based on the matching predicate. Note that the
 * caller *MUST* hold a reference to the object for this call to work.
 */
    void
    InheritingEAContext::setup()
    {
        LG_FILTERPARSE_D << "======= InheritingEAContext::setup() ==========" << endl;
        SubContextNames_t ls = Delegate->getSubContextNames();

        for( SubContextNames_t::iterator iter = ls.begin(); iter != ls.end(); iter++ )
        {
            try {
                LG_FILTERPARSE_D << "Testing name:" << *iter << endl;
                fh_context c = Delegate->getSubContext( *iter );
                wrapInsert( c );
            }
            catch(NoSuchSubContext& e)
            {
                LG_CTX_ER << "InheritingEAContext::InheritingEAContext() "
                          << "Context:" << *iter
                          << " advertised but not presentable!"
                          << " e:" << e.what()
                          << endl;
            }
        }

        SetupEventConnections();
    }

    void
    InheritingEAContext::read( bool force )
    {
        if( ReadingDir )
            return;

        EnsureStartStopReadingIsFiredRAII _raii1( this );
        ReadingDirRAII __raiird1( this, true );

        if( !HaveReadDir )
        {
            HaveReadDir = true;
            Delegate->read();
            setup();
        }
        else
        {
            emitExistsEventForEachItem();
        }
    }

    void
    InheritingEAContext::OnDeleted( NamingEvent_Deleted* ev, string olddn, string newdn )
    {
        Emit_Deleted( ev, newdn, olddn, 0 );
        Remove( ev->getSource()->getSubContext( olddn ) );
    }
    
    void
    InheritingEAContext::OnCreated( NamingEvent_Created* ev,
                                    const fh_context& subc,
                                    std::string olddn, std::string newdn )
    {
        LG_CTX_D << "InheritingEAContext::OnCreated() " << endl;
//        fh_context subc = ev->getSource()->getSubContext( olddn );
        wrapInsert( subc, true );
    }

    void
    InheritingEAContext::OnExists ( NamingEvent_Exists* ev,
                                    const fh_context& subc,
                                    string olddn, string newdn )
    {
//        fh_context subc = ev->getSource()->getSubContext( olddn );
        wrapInsert( subc );
    }


/**
 * Disallow and log any attempt to directly create a new context.
 * All other methods should delegate the creation of new subcontexts to
 * the underlying base context and from there the events will inform
 * this context of the creation and we will in turn filter that new
 * context.
 */
    Context*
    InheritingEAContext::priv_CreateContext( Context* parent, string rdn )
    {
        LG_CTX_ER << "priv_CreateContext() should never happen" << endl;
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    std::string
    InheritingEAContext::private_getStrAttr( const std::string& rdn,
                                             const std::string& def,
                                             bool getAllLines,
                                             bool throwEx )
    {
//        cerr << "InheritingEAContext::private_getStrAttr() rdn:" << rdn << endl;

        std::string ret = def;
        
        try
        {
            ret = Delegate->private_getStrAttr( rdn, def, getAllLines, true );
            return ret;
        }
        catch( exception& e )
        {
            if( !isParentBound() )
                throw;
            return getParent()->private_getStrAttr( rdn, def, getAllLines, throwEx );
        }
    }
    
    fh_attribute
    InheritingEAContext::getAttribute( const string& rdn ) throw( NoSuchAttribute )
    {
//        cerr << "InheritingEAContext::getAttribute() rdn:" << rdn << endl;
        
        fh_attribute ret = 0;
        
        try
        {
            ret = Delegate->getAttribute( rdn );
            if( ret )
            {
//                 cerr << "InheritingEAContext::getAttribute(found in del) rdn:" << rdn
//                      << " value:" << getStrAttr( Delegate, rdn, "<n>" )
//                      << endl;
                return ret;
            }
        }
        catch( exception& e )
        {
            if( !isParentBound() )
                throw;
        }

        if( !isParentBound() )
        {
            fh_stringstream ss;
            ss << "No attribute found for ea:" << rdn
               << " on context:" << getURL() << endl;
            Throw_NoSuchAttribute( tostr(ss), 0 );
        }
//        cerr << "InheritingEAContext::getAttribute(passing to pnt) rdn:" << rdn << endl;
        return getParent()->getAttribute( rdn );
    }
    
    AttributeCollection::AttributeNames_t&
    InheritingEAContext::getAttributeNames( AttributeNames_t& ret )
    {
        if( isParentBound() )
        {
            AttributeCollection::AttributeNames_t t1;
            AttributeCollection::AttributeNames_t t2;
            Delegate->getAttributeNames( t1 );
            getParent()->getAttributeNames( t2 );
            return mergeAttributeNames( ret, t1, t2 );
        }
        return Delegate->getAttributeNames( ret );
    }
    
    int
    InheritingEAContext::getAttributeCount()
    {
        AttributeCollection::AttributeNames_t tmp;
        getAttributeNames( tmp );
        return tmp.size();
    }

    bool
    InheritingEAContext::isAttributeBound( const std::string& rdn, bool createIfNotThere )
        throw( NoSuchAttribute )
    {
//         cerr << "InheritingEAContext::isAttributeBound() rdn:" << rdn << endl;
        
        bool ret = Delegate->isAttributeBound( rdn, createIfNotThere );
//         // PURE DEBUG
//         if( starts_with( rdn, "gtk" ) )
//         {
//             cerr << "InheritingEAContext::isAttributeBound(DEBUG) rdn:" << rdn
//                  << " this:" << getURL()
//                  << " Delegate:" << Delegate->getURL() << endl
//                  << " d.value:" << getStrAttr( Delegate, rdn, "<nothing>" )
//                  << " ret:" << ret
//                  << " parent:"  << ( isParentBound() ? getParent()->getURL() : "" )
//                  << " p.value:" << ( isParentBound() ? getStrAttr( getParent(), rdn, "<n>" ) : "" )
//                  << endl;
//             if( starts_with( rdn, "gtk-cell-renderer-name" ))
//                 cerr << "ea-names:" << getStrAttr( Delegate, "ea-names", "" ) << endl;
//         }
        
        if( !ret && isParentBound() )
        {
//             cerr << "InheritingEAContext::isAttributeBound(trying parent) rdn:" << rdn << endl;
            return getParent()->isAttributeBound( rdn, createIfNotThere );
        }
        return ret;
    }    
    

    
    namespace Factory
    {
        fh_contextlist MakeContextList()
        {
            std::list< fh_context > dummy;
            return MakeContextList( dummy.begin(), dummy.end() );
        }
        fh_context makeInheritingEAContext( fh_context ctx )
        {
            
            InheritingEAContext* c = new InheritingEAContext( 0, ctx );
            fh_context ret;
            Upcast( ret, c );
            c->setup();
            c->setIsChainedViewContextRoot();
            return ret;
        }
    };


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class DelayedCommitParentContext;
    class FERRISEXP_API DelayedCommitContext
        :
        public leafContext
    {
        typedef DelayedCommitContext _Self;
        typedef leafContext _Base;

        typedef map< string, string > m_attributesToUpdate_t;
        m_attributesToUpdate_t m_attributesToUpdate;
        
    protected:
    public:
        DelayedCommitContext( Context* parent, std::string rdn, fh_context md );
        virtual ~DelayedCommitContext();

        ferris_ios::openmode
        getSupportedOpenModes()
            {
                return ios::in | ios::out | ios::binary | ios::ate | ios::trunc;
            }

        virtual fh_iostream real_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ss;
                return ss;
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
                return real_getIOStream( m );
            }
        
        
        virtual fh_context
        createSubContext( const std::string& rdn,
                          fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );

        virtual fh_context SubCreate_ea( fh_context c, fh_context md )
            {
                LG_PG_W << "XXXXXXX SubCreate_ea() " << endl;
            }
        
        
        void
        priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
                LG_PG_D << "priv_FillCreateSubContextSchemaParts(SETUP)" << endl;
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
            }

        fh_iostream process_delayed_updates_GetIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                return ss;
            }
        
        void process_delayed_updates_IOStreamClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream iss )
            {
                // now we must commit updates to the base context...

                LG_PG_D << "IOStreamClosed() MUST COMMIT" << endl;

                for( m_attributesToUpdate_t::iterator iter = m_attributesToUpdate.begin();
                     iter != m_attributesToUpdate.end(); ++iter )
                {
                    LG_PG_D << "m_attributesToUpdate[" << iter->first << "] = " << iter->second << endl;
                }

                fh_context p = getParent();
                ChainedViewContext* pcvc = dynamic_cast<ChainedViewContext*>( GetImpl(p) );
                fh_context del = pcvc->getDelegate();
                LG_PG_D << "del:" << del->getURL() << endl;
                

                fh_mdcontext md = new f_mdcontext();
                fh_mdcontext child = md->setChild( "tuple", "" );
                for( m_attributesToUpdate_t::iterator iter = m_attributesToUpdate.begin();
                     iter != m_attributesToUpdate.end(); ++iter )
                {
                    child->setChild( iter->first, iter->second );
                }
                del->createSubContext( "", md );
                LG_PG_D << "IOStreamClosed() COMMITED!" << endl;
            }


        fh_iostream attributesToUpdate_GetIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                LG_PG_D << "attributesToUpdate_GetIOStream() rdn:" << rdn << endl;
                return ss;
            }
        
        void attributesToUpdate_IOStreamClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream iss )
            {
                LG_PG_D << "attributesToUpdate_IOStreamClosed() c:" << c->getURL() << endl;
                const string s = StreamToString(iss);
                LG_PG_D << "attributesToUpdate_IOStreamClosed() c:" << c->getURL()
                        << " rdn:" << rdn << " s:" << s << endl;
                m_attributesToUpdate[ rdn ] = s;
            }
        
    };

    DelayedCommitContext::DelayedCommitContext( Context* parent, std::string rdn, fh_context md )
        :
        _Base( parent, rdn )
    {
        addAttribute( (string)"ferris-process-delayed-updates",
                      this, &_Self::process_delayed_updates_GetIOStream,
                      this, &_Self::process_delayed_updates_GetIOStream,
                      this, &_Self::process_delayed_updates_IOStreamClosed );
        LG_PG_D << "DelayedCommitContext() rdn:" << rdn << " parent:" << parent->getURL() << endl;
    }
    
    DelayedCommitContext::~DelayedCommitContext()
    {
    }
    
    fh_context
    DelayedCommitContext::createSubContext( const std::string& _rdn, fh_context md )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        LG_PG_D << "DelayedCommitContext::createSubContext(TOP) _rdn:" << _rdn << endl;
        fh_context mdf = md->getSubContext("ea");
        string rdn = getStrSubCtx( mdf, "name", "" );
        string v   = getStrSubCtx( mdf, "value", "" );
        LG_PG_D << "DelayedCommitContext::createSubContext(TOP)  rdn:" <<  rdn << " v:" << v << endl;

        m_attributesToUpdate[ rdn ] = v;
        addAttribute( rdn,
                      this, &_Self::attributesToUpdate_GetIOStream,
                      this, &_Self::attributesToUpdate_GetIOStream,
                      this, &_Self::attributesToUpdate_IOStreamClosed );
        return this;
    }
    

    /****************************************/
    /****************************************/
    /****************************************/
    
    class FERRISEXP_API DelayedCommitParentContext
        :
        public ChainedViewContext
    {
        typedef DelayedCommitParentContext _Self;
        typedef ChainedViewContext         _Base;
    protected:
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );
    public:
        DelayedCommitParentContext( const fh_context& ctx );
        virtual ~DelayedCommitParentContext();

//         virtual fh_context
//         createSubContext( const std::string& rdn, fh_context md = 0 )
//             throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );

        virtual bool isDir()
            {
                return true;
            }
        

        virtual void read( bool force = 0 );

        virtual fh_context
        createSubContext( const std::string& rdn,
                          fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );
            
        virtual bool supportsRemove()
            {
                return true;
            }

        virtual void priv_remove( fh_context c_ctx )
            {
                LG_PG_D << "priv_remove() c_ctx:" << c_ctx->getURL() << endl;
                string rdn = c_ctx->getDirName();
                if( Delegate->isSubContextBound( rdn ))
                {
                    fh_context subc = Delegate->getSubContext( rdn );
                    Delegate->priv_remove( subc );
                }
            }
        
    };

    DelayedCommitParentContext::DelayedCommitParentContext( const fh_context& ctx )
        :
        ChainedViewContext( ctx )
    {
        LG_PG_D << "DelayedCommitParentContext(1) c:" << ctx->getURL() << endl;
        createStateLessAttributes();
        LG_PG_D << "DelayedCommitParentContext(2) c:" << ctx->getURL() << endl;
    }
    
    DelayedCommitParentContext::~DelayedCommitParentContext()
    {
    }
    
    fh_context
    DelayedCommitParentContext::createSubContext( const std::string& _rdn, fh_context md )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        if( md->isSubContextBound("ea"))
            return this;
        
        fh_context mdf = md->getSubContext("file");
        string rdn = getStrSubCtx( mdf, "name", "" );
        LG_PG_D << "DelayedCommitParentContext::createSubContext(TOP) _rdn:" << _rdn << endl;
        LG_PG_D << "DelayedCommitParentContext::createSubContext(TOP)  rdn:" <<  rdn << endl;
        
        try
        {
//             LG_PG_D << "DelayedCommitParentContext::createSubContext(1) rdn:" << rdn << endl;
//             fh_context dc = Delegate->createSubContext( rdn, md );
//             LG_PG_D << "DelayedCommitParentContext::createSubContext(2) rdn:" << rdn << endl;
//             fh_context ret = new DelayedCommitContext( dc, this );
//             LG_PG_D << "DelayedCommitParentContext::createSubContext(3) rdn:" << rdn
//                     << " dc:" << GetImpl(dc) << " ret:" << GetImpl(ret)
//                     << endl;
//             return ret;


            LG_PG_D << "DelayedCommitParentContext::createSubContext(1) rdn:" << rdn << endl;
//            fh_context dc = Delegate->createSubContext( rdn, md );
            LG_PG_D << "DelayedCommitParentContext::createSubContext(2) rdn:" << rdn << endl;
            fh_context ret = new DelayedCommitContext( this, rdn, md );
            Insert( GetImpl( ret ), true );
            LG_PG_D << "DelayedCommitParentContext::createSubContext(3) rdn:" << rdn
//                    << " dc:" << GetImpl(dc)
                    << " ret:" << GetImpl(ret)
                    << " ret.url:" << ret->getURL()
                    << endl;
            return ret;
            
        }
        catch( exception& e )
        {
            LG_PG_D << "error:" << e.what() << endl;
            throw;
        }
    }
    
    void
    DelayedCommitParentContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
        Delegate->priv_FillCreateSubContextSchemaParts( m );
    }
    
    void
    DelayedCommitParentContext::read( bool force )
    {
        LG_PG_D << "DelayedCommitParentContext::priv_read()" << endl;
        
        if( !force && Delegate->HaveReadDir && Delegate->isActiveView() )
        {
            if( !getItems().empty() )
            {
                LG_CTX_D << "CacheContext::read( bool force " << force << " )"
                         << " shorting out attempt to read() because we are already read."
                         << endl;
                HaveReadDir = true;
                emitExistsEventForEachItem();
                return;
            }
        }
        
        clearContext();
        Delegate->read( force );
        emitExistsEventForEachItem();
    }
    
    
    
    namespace Factory
    {
        fh_context MakeDelayedCommitParentContext( const fh_context& c )
        {
            LG_PG_D << "MakeDelayedCommitParentContext() c:" << c->getURL() << endl;
            return new DelayedCommitParentContext( c );
        }
    };
    
    
    
};

