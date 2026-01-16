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

    $Id: Context.hh,v 1.25 2010/09/24 21:30:26 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_CONTEXT_H_
#define _ALREADY_INCLUDED_FERRIS_CONTEXT_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/ChainedViewContext.hh>
#include <Ferris/SignalStreams.hh>
#include <Ferris/Debug.hh>
#include <sigc++/sigc++.h>
#include <sys/utsname.h>

namespace Ferris
{
    FERRISEXP_API std::string monsterName( const fh_context& c, const std::string& rdn );
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    class FERRISEXP_API mtimeNotChangedChecker
    {
        time_t m_old;
        
    public:
        mtimeNotChangedChecker();
        bool operator()( Context* c );
    };



    class FERRISEXP_API leafContext
        :
        public Context
    {
    protected:
    
        Context* priv_CreateContext( Context* parent, std::string rdn );
        virtual void priv_read();

        leafContext();
        leafContext( Context* parent, std::string rdn );
        
    
    public:

        virtual ~leafContext();
    
    };

    class FERRISEXP_API leafContextWithSimpleContent
        :
        public leafContext
    {
        typedef leafContext _Base;
        typedef leafContextWithSimpleContent _Self;
    protected:
        std::string priv_getRecommendedEA();
        virtual ferris_ios::openmode getSupportedOpenModes();

        virtual fh_istream  priv_getIStream( ferris_ios::openmode m );
        void
        priv_OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m );
        fh_iostream priv_getIOStream( ferris_ios::openmode m );

        

        //////////
        
        virtual fh_stringstream
            real_getIOStream( ferris_ios::openmode m ) = 0;
        virtual void OnStreamClosed( const std::string& s ) = 0;
        
        leafContextWithSimpleContent( Context* parent = 0, std::string rdn = "" );
        virtual ~leafContextWithSimpleContent();
        
    public:
        
    };
    

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    class FakeInternalContext;
//    FERRIS_SMARTPTR( FakeInternalContext, fh_fcontext );
    FERRIS_CTX_SMARTPTR( FakeInternalContext, fh_fcontext );

    class FERRISEXP_API FakeInternalContext :
        public Context
    {
        typedef FakeInternalContext _Self;
        
    protected:
    
        FakeInternalContext* priv_CreateContext( Context* parent, std::string rdn );
        virtual void priv_read();

        FakeInternalContext();
    
    public:

        FakeInternalContext( Context* parent, const std::string& rdn );
        virtual ~FakeInternalContext();

        fh_fcontext addNewChild( const std::string& rdn );
        fh_context addNewChild( fh_context c );

        /**
         * If you want a new child of class T which has a constructor( pptr, rdn )
         * this method will make it for you and return an object of that class.
         */
        template< class T, class ParentContext >
        T* addNewChild( ParentContext* pptr, std::string rdn )
            {
                T* child;
                child = new T( pptr, rdn );
                fh_context cc = child;
                FakeInternalContext::addNewChild( cc );
                return child;
            }
        
    };

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////


    /**
     * This class is to be inherited from for classes that wish to create a VFS
     * that has many nested subdirs and needs to be able to create files and EA
     * from relative paths that point to contexts that may not yet exist.
     *
     * For example, calling ensureContextCreated( "usr/bin/myfile" );
     * will make usr and bin if they do not yet exist and then myfile.
     */
    template < class ChildContextClass, class ParentContextClass = FakeInternalContext >
    class FERRISEXP_DLLLOCAL ParentPointingTreeContext
        :
        public ParentContextClass
    {
        typedef ParentPointingTreeContext  _Self;
        typedef ParentPointingTreeContext< ChildContextClass, ParentContextClass > _FullSelf;
        typedef ParentContextClass         _Base;
        
    public:

        FERRIS_CTX_SMARTPTR( ChildContextClass, fh_childc );
        FERRIS_CTX_SMARTPTR( _FullSelf, fh_pptcontext );
        void ParentPointingTreeContext_clearContext()
            {
//                 std::cerr << "ParentPointingTreeContext_clearContext() url:" << _Self::getURL() << std::endl;
//                 ::Ferris::BackTrace();
                _Base::clearContext();
            }
        
    private:

//        fh_pptcontext MainPPTContext;

//         void setMainPPTContext( fh_pptcontext v )
//             {
//                 MainPPTContext = v;
//             }

        /**
         * Create a subcontext emiting "Exists" by default or "created" signal
         */
        fh_childc create( const std::string& rdn, bool created = false )
            {
                if( this->priv_isSubContextBound( rdn ) )
                {
                    fh_context retc = this->priv_getSubContext( rdn );
                    // retc->HasBeenDeleted = false;
                    // retc->bumpVersion();
                    // this->bumpVersion();
                    fh_childc ret = static_cast<ChildContextClass*>(GetImpl(retc));
                    return ret;
                }
                
                fh_childc ret = new ChildContextClass();
                {
//                    fh_context tmp = ret; // Hold a reference

                    
//                     cerr << "  ret:" << ((void*)GetImpl(ret))
//                          << " this:" << ((void*)this)
//                          << "  ret.rc:" << ret->ref_count
//                          << " this.rc:" << ref_count
//                          << endl;
                    LG_CTX_D << " ParentPointingTreeContext::create(1) this:" << (void*)this
                             << " omc:" << (void*)this->getOverMountContext()
                             << " url:" << _Self::getURL()
                             << " rdn:" << rdn
                             << std::endl;

                    std::string monstered_rdn = this->monsterName( rdn );
                    ret->setContext( this, monstered_rdn );

                    // Note that we REALLY can't bump the reference count up
                    // until the child knows its parent object so it can bump
                    // this->getParent() as well as itself.
                    fh_context tmp = ret; // Hold a reference
//                     cerr << " ParentPointingTreeContext::create(2) url:" << getURL()
//                          << " rdn:" << rdn
//                          << " monstered_rdn:" << monstered_rdn
//                          << endl;
//                ret->setMainPPTContext( getMainPPTContext() );
                    LG_CTX_D << "Context::create() ret:" << ret->getURL()
                             << " created:" << created
                             << std::endl;
                    
//                     cerr << "Context::create(1) ret:" << ret->getURL()
//                              << " created:" << created
//                              << std::endl;
                    this->Insert( GetImpl(ret), created, created );
//                    cerr << "Context::create(2) ret:" << ret->getURL()
//                             << " created:" << created
//                             << std::endl;
                    this->bumpVersion();

//                     cerr << " ParentPointingTreeContext::create(3) url:" << getURL()
//                          << " rdn:" << rdn
//                          << std::endl;
//                     cerr << "  ret:" << ((void*)GetImpl(ret))
//                          << " this:" << ((void*)this)
//                          << "  ret.rc:" << ret->ref_count
//                          << " this.rc:" << ref_count
//                          << std::endl;
                }
//                     cerr << " ParentPointingTreeContext::create(end) url:" << getURL()
//                          << " rdn:" << rdn
//                          << std::endl;
//                     cerr << "  ret:" << ((void*)GetImpl(ret))
//                          << " this:" << ((void*)this)
//                          << "  ret.rc:" << ret->ref_count
//                          << " this.rc:" << ref_count
//                          << std::endl;

                return ret;
            }

    protected:

//         fh_pptcontext getMainPPTContext()
//             {
//                 if( isBound( MainPPTContext ))
//                     return MainPPTContext;
                
//                 return this;
//             }

//         ChildContextClass* getMainContext()
//             {
//                 if( isBound( MainPPTContext ))
//                 {
//                     Context* c = GetImpl(MainPPTContext);
//                     return dynamic_cast<ChildContextClass*>(c);
//                 }
                
//                 return dynamic_cast<ChildContextClass*>(this);
//             }

        /**
         *
         * @see getFirstParentOfContextClass()
         */
        ChildContextClass* getBaseContext()
            {
                if( this->getOverMountContext() != this )
                {
                    ChildContextClass* c = dynamic_cast<ChildContextClass*>(
                        this->getOverMountContext());
                    return c->getBaseContext();
                }

                
                ChildContextClass* c = dynamic_cast<ChildContextClass*>(this);
                while( c && c->isParentBound() )
                {
                    Context* p = c->getParent()->getOverMountContext();
                    if(ChildContextClass* nextc = dynamic_cast<ChildContextClass*>( p ))
                    {
                        c = nextc;
                    }
                    else
                    {
                        return c;
                    }
                }
                return c;
            }
        

        virtual void ensureEACreated(
            std::string xdn,
            bool chopXDNatLastSlash,
            fh_context md = 0,
            std::list<std::string> args = std::list< std::string >()
            )
            {
                std::string rdn = xdn;
                
                LG_CTX_D << "ensureEACreated() xdn:" << xdn
                         << " URL:" << this->getURL() << std::endl;

                if( chopXDNatLastSlash )
                {
                    int lastSlashPos = xdn.rfind( "/" );
                    if( std::string::npos != lastSlashPos )
                    {
                        std::string eaname = xdn.substr( lastSlashPos+1 );
                        std::string path   = xdn.substr( 0, lastSlashPos );

                        rdn = eaname;
                    }
                }

//                 cerr << "ensureEACreated() xdn:" << xdn
//                      << " rdn:" << rdn
//                      << " URL:" << getURL() << std::endl;
                
                /* If its already there, do nothing */
                if( this->isAttributeBound( rdn, false ) )
                    {
                        LG_CTX_D << "ensureEACreated() rdn:" << rdn << " already there" << std::endl;
                        return;
                    }
                
                typedef EA_Atom_ReadOnly::GetIStream_Func_t FI;
                typedef EA_Atom_ReadWrite::GetIOStream_Func_t FIO;
                typedef EA_Atom_ReadWrite::IOStreamClosed_Func_t FCL;
                
                this->addAttribute(
                    rdn,
                    FI(this, &_Self::getEAStream),
                    FIO(this, &_Self::getEAStream),
                    FCL(this, &_Self::setEAStream),
                    FXD_BINARY,
                    true );
            }

        class AlreadyEmittedCacheRAII
            :
            public Handlable
        {
            Context* m_c;
        public:
            
            typedef FERRIS_STD_HASH_SET< Context*,
                                         f_hash<Context* const>,
                                         f_equal_to<Context* const>  > m_alreadyEmittedCache_t;
            typedef FERRIS_STD_HASH_MAP< Context*,
                                         m_alreadyEmittedCache_t*,
                                         f_hash<Context* const>,
                                         f_equal_to<Context* const> > s_caches_t;
            AlreadyEmittedCacheRAII( ParentPointingTreeContext* c, bool clearPPTC = true )
                :
                m_c( c )
                {
                    if( clearPPTC )
                        c->ParentPointingTreeContext_clearContext();

                    //s_caches()[c].clear();
                    s_caches_t::iterator sci = s_caches().find( c );
                    if( sci == s_caches().end() )
                    {
                        s_caches()[c] = new m_alreadyEmittedCache_t();
                    }
                    else
                    {
                        s_caches()[c]->clear();
                    }
                    ++getInstanceCount();

//                     std::cerr << "AlreadyEmittedCacheRAII::ctor() sz:" << s_caches().size()
//                               << " recursionCount:" << getInstanceCount()
//                               << " c:" << (void*)c
//                               << std::endl;
                }
            
            ~AlreadyEmittedCacheRAII()
                {
                    --getInstanceCount();
//                     std::cerr << "AlreadyEmittedCacheRAII::dtor() sz:" << s_caches().size()
//                               << " recursionCount:" << getInstanceCount()
//                               << " c:" << (void*)m_c
//                               << std::endl;
                    
                    if( !getInstanceCount() )
                    {
//                         for( s_caches_t::iterator ci = s_caches().begin();
//                              ci != s_caches().end(); ++ci )
//                         {
//                             Context* hc = ci->first;
//                             std::cerr << "AlreadyEmittedCacheRAII::dtor() hc:" << (void*)hc << std::endl;
//                             for( std::set< Context* >::iterator si = ci->second.begin();
//                                  si != ci->second.end();  )
//                             {
//                                 std::cerr << "   AlreadyEmittedCacheRAII::dtor() listc:" << (void*)*si << std::endl;
// //                                std::set< fh_context >::iterator t = si;
//                                 ++si;
// //                                ci->second.erase(t);
//                             }
//                             ci->second.clear();
//                         }




                        
//                        s_caches().clear();
                        s_caches_t::iterator sci = s_caches().begin();
                        s_caches_t::iterator sce = s_caches().end();
                        for( ; sci != sce;  )
                        {
                            m_alreadyEmittedCache_t* d = sci->second;
                            ++sci;
                            delete d;
                        }
                        s_caches().clear();
                        

//                        std::cerr << "   AlreadyEmittedCacheRAII::dtor() cleared!" << std::endl;
                    }
                    else
                    {
//                        s_caches().erase( m_c );
                        s_caches_t::iterator sci = s_caches().find( m_c );
                        if( sci != s_caches().end() )
                        {
                            m_alreadyEmittedCache_t* d = sci->second;
                            s_caches().erase( m_c );
                            delete d;
                        }
                    }
                }

            static int& getInstanceCount()
                {
                    static int ret = 0;
                    return ret;
                }
            
//             static void clearCache( Context* c )
//                 {
//                     s_caches().erase( c );
//                 }

            static m_alreadyEmittedCache_t& getCache( Context* c )
                {
//                    std::cerr << "getCache()" << std::endl;
                    
//                     // DEBUG
//                     {
//                         if( s_caches().end() == s_caches().find( c ) )
//                         {
//                             std::cerr << "ERORR should have RIAA object wrapping this call!\n";
//                             std::cerr << "seeking c:" << c << " earl:" << c->getURL() << std::endl;
//                             for( s_caches_t::iterator si = s_caches().begin();
//                                  si != s_caches().end(); ++si )
//                             {
//                                 std::cerr << "si:" << si->first
//                                           << " earl:" << si->first->getURL() <<  std::endl;
//                             }
//                             BackTrace();
//                         }
//                     }

//                     std::cerr << "AlreadyEmittedCacheRAII::getCache() sz:" << s_caches().size()
//                               << " recursionCount:" << getInstanceCount() 
//                               << " c:" << (void*)c
//                               << std::endl;
                    if( !getInstanceCount() )
                    {
                        std::cerr << "AlreadyEmittedCacheRAII::getCache(call out of scope!)"
                                  << " sz:" << s_caches().size()
                                  << " recursionCount:" << getInstanceCount() 
                                  << " c:" << (void*)c
                                  << std::endl;
                        ::Ferris::BackTrace();
                    }
//                    return s_caches()[ c ];
                    s_caches_t::iterator sci = s_caches().find( c );
                    if( sci != s_caches().end() )
                        return *sci->second;

                    m_alreadyEmittedCache_t* ret = new m_alreadyEmittedCache_t();
                    s_caches().insert( make_pair( c, ret ));
                    return *ret;
                }
        private:

            static s_caches_t& s_caches()
                {
                    static s_caches_t s_caches;
                    return s_caches;
                }
        };
        FERRIS_SMARTPTR( AlreadyEmittedCacheRAII, fh_AlreadyEmittedCacheRAII );
        
//         virtual void clearContext()
//             {
//                 AlreadyEmittedCacheRAII::clearCache( this );
//                 _Base::clearContext();
//             }
        
        void maybeEmitExists( fh_context c )
            {
                if( AlreadyEmittedCacheRAII::getCache( this ).count( GetImpl(c) ) )
                {
                    return;
                }
                AlreadyEmittedCacheRAII::getCache( this ).insert( GetImpl(c) );
                
                LG_CTX_D << "maybeEmitExists() this:" << this->getURL() 
                         << " emitting exists for c:" << c->getURL() << std::endl;
                this->Emit_Exists( 0, c, c->getDirName(), c->getDirName(), 0 );
            }
        
        
        /**
         * Ensure that a subcontext exists, emitting by default "existing" or if created
         * is set to true, "created" signal instead.
         */
        fh_childc ensureContextCreated( const std::string& xdn, bool created = false )
            {
                fh_AlreadyEmittedCacheRAII _sigraii1 = 0;
                if( !AlreadyEmittedCacheRAII::getInstanceCount() )
                {
                    _sigraii1 = new AlreadyEmittedCacheRAII( this, false );
                }
                
                LG_CTX_D << "ensureContextCreated xdn:" << xdn << std::endl;
                
                if( this->canSplitPathAtStart( xdn ) )
                {
                    std::pair<std::string,std::string> p = this->splitPathAtStart( xdn );
                    LG_CTX_D << "ensureContextCreated first:" << p.first
                             << " p.first.length():" << p.first.length()
                             << " second:" << p.second << std::endl;
                    
                    if( p.first.empty() )
                    {
                        return ensureContextCreated( p.second, created );
                    }

                    fh_childc c;
                    Context::Items_t::iterator subc_iter;
                    const std::string& rdn = p.first;
                    
                    if( this->priv_isSubContextBound( rdn, subc_iter ) )
                    {
                        LG_CTX_D << "path:" << this->getDirPath()
                                 << " already exists for:" << rdn
                                 << std::endl;
                        
                        fh_context child =  *subc_iter;
                        c = dynamic_cast<ChildContextClass*>( GetImpl(child) );
                    }
                    else
                    {
                        LG_CTX_D << "path:" << this->getDirPath()
                                 << " making fake dir for:" << rdn
                                 << std::endl;
            
                        c = create( p.first, created );
                    }
                    
                    c->setHasBeenDeleted( false );
                    maybeEmitExists( c );
                    return c->ensureContextCreated( p.second, created );
                }
                else if( !this->priv_isSubContextBound( xdn ) )
                {
                    LG_CTX_D << "ensureContextCreated path:" << this->getDirPath()
                             << " making fake file for xdn:" << xdn
                             << " created:" << created
                             << std::endl;

                    fh_childc ret = create( xdn, created );
                    maybeEmitExists( ret );
//                     cerr << "ensureContextCreated path:" << getDirPath()
//                          << " made fake file for xdn:" << xdn
//                          << std::endl;
//                    dumpOutItems();
                    return ret;
                }
    
                LG_CTX_D << "ensureContextCreated using getSubContext() xdn:" << xdn << std::endl;

                fh_context retc = this->getSubContext( xdn );
                fh_childc r = dynamic_cast<ChildContextClass*>(GetImpl(retc));

                if( !isBound(r) )
                {
                    LG_CTX_ER << "ensureContextCreated down cast failed xdn:" << xdn
                              << " url:" << this->getURL()
                              << std::endl;
//                     cerr << "ensureContextCreated down cast failed xdn:" << xdn
//                          << " url:" << getURL()
//                          << std::endl;
                }
                r->setHasBeenDeleted( false );
                maybeEmitExists( r );
                return r;
                
            }


        /**
         * Create an EA or context depending on if there is a leading "/" char.
         * Emit exists by default or created signal if created=true
         */
        fh_childc ensureEAorContextCreated( const std::string k, bool created = false )
            {
                /* EA Start with a slash and the EA rdn is the last path component */
                if( starts_with(k, "/") )
                {
                    std::string eaname = k;
                    std::string path   = "";
                    
                    int lastSlashPos = k.rfind( "/" );
                    if( std::string::npos != lastSlashPos )
                    {
                        eaname = k.substr( lastSlashPos+1 );
                        path   = k.substr( 0, lastSlashPos );
                    }

                    /* This will work because we trust that the child is who
                     *  they claim in the code
                     */
                    fh_childc c = dynamic_cast<ChildContextClass*>(this);

                    if( path.length() )
                    {
//                         cerr << "ensureEAorContextCreated() k:" << k << std::endl;
//                         cerr << "ensureEAorContextCreated() path:" << path << std::endl;
                        c = ensureContextCreated( path, created );
                    }

                    c->ensureEACreated( eaname, false );
                    return c;
                }
                else
                {
                    return ensureContextCreated( k, created );
                }
            }
        
        
    public:
        
        ParentPointingTreeContext( Context* parent = 0, const std::string& rdn = "" )
            :
            ParentContextClass( parent, rdn )
            {
            }
        
        virtual ~ParentPointingTreeContext()
            {
            }

        
        virtual fh_iostream getEAStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << "Attempt to get a stream from a raw getEAStream() call on "
                   << " a ParentPointingTreeContext. Please report this bug."
                   << " to create the ea:" << rdn
                   << " output."
                   << " path:" << this->getDirPath()
                   << std::endl;
                std::cerr << tostr(ss) << std::endl;
                Throw_CanNotGetStream( tostr(ss), c );
            }


        virtual void setEAStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream )
            {
            }
        
        
    };
    
    

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    

/**
 * Provides a dummy context that can have a bunch of attributes set explicity
 * This is very handy for providing a coded CreateSubContext() call
 */
class FERRISEXP_API CreateMetaDataContext :
        public FakeInternalContext
    {
    protected:

        std::string Body;

        virtual fh_istream getIStream( ferris_ios::openmode m = std::ios::in );
    public:

        CreateMetaDataContext();
        virtual ~CreateMetaDataContext();

        void setAttr( const std::string& rdn, const std::string& v );
        fh_mdcontext setChild( const std::string& rdn, const std::string& v );

        void setBody( std::string s );

    private:
        virtual bool supportsReClaim();
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * This class allows a default value to be drawn from "shadow" and that value
 * to be explicitly overriden using setAttr()
 */
class FERRISEXP_API DelegatingCreateMetaDataContext :
        public CreateMetaDataContext
    {
        typedef CreateMetaDataContext _Base;
    
    protected:

        fh_context shadow;

        DelegatingCreateMetaDataContext(
            DelegatingCreateMetaDataContext* parent,
            const fh_context& _shadow );

        void buildCoveringTree();

        virtual fh_istream getIStream( ferris_ios::openmode m = std::ios::in );
        
    public:

        DelegatingCreateMetaDataContext( const fh_context& _shadow );
        virtual ~DelegatingCreateMetaDataContext();

        virtual fh_attribute getAttribute( const std::string& rdn );
        virtual AttributeNames_t& getAttributeNames( AttributeNames_t& ret );
        virtual int  getAttributeCount();
        virtual bool isAttributeBound( const std::string& rdn,
                                       bool createIfNotThere = true
            );
    
    private:
        virtual bool supportsReClaim();
    };

 



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
    
}

#include <Ferris/ChainedViewContext.hh>

namespace Ferris
{
    namespace Factory
    {
        FERRISEXP_API fh_context
        MakeCachedContext( fh_context& ctx, const std::string& extradata = "" );
    };


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    /**
     * This creates objects of type OurDirectChildrenContextClass when it needs to insert
     * new elements and inherits from ParentCtx.
     *
     * children should call tryAugmentLocalhostNames() to add other text names for the
     * localhost if they wish to add those, they can also add a host with tryAddHost()
     * Both of the above calls will not add any host if TryToCheckServerExists() retuns
     * an error.
     * 
     */
    template <class OurDirectChildrenContextClass, class ParentCtx = FakeInternalContext>
    class FERRISEXP_DLLLOCAL networkRootContext
        :
        public ParentCtx
    {
        typedef networkRootContext<OurDirectChildrenContextClass>  _Self;
        typedef ParentCtx                              _Base;

        
    protected:

        virtual bool canInsert( const std::string& rdn )
            {
                std::string s = TryToCheckServerExists( rdn );
                if( s.length() )
                {
//                    cerr << "networkContext::canInsert() rdn:" << rdn << " NO " << std::endl;
                    return false;
                }
                
                return !_Base::priv_isSubContextBound( rdn );
            }
            

        /**
         * This can return a std::string with length != 0 to indicate that a server
         * name is not ever going to work in the future. This is handy for database systems
         * which can check somewhat quickly if a database is in existance.
         *
         * @returns a std::string describing the problem
         */
        virtual std::string TryToCheckServerExists( const std::string& rdn )
            {
                return "";
            }

    public:
        
        /**
         * Try to add a specific host as a child. This call is
         * impotent for hosts that return an error in
         * TryToCheckServerExists()
         *
         * @throws An error if the TryToCheckServerExists() fails
         */
        void tryAddHost( const std::string& hostName )
            {
                LG_CTX_D << "tryAddHost() hostname:" << hostName << std::endl;

                if( canInsert( hostName ))
                {
                    OurDirectChildrenContextClass* child;
                    child = new OurDirectChildrenContextClass( this, hostName );
                    this->addNewChild( child );
                }
            }
        
        /**
         * Try to add all the text names for all interfaces of the
         * local machine as subdirectories. This call is impotent for
         * hosts that return an error in TryToCheckServerExists()
         */
        void tryAugmentLocalhostNames()
            {
                LG_CTX_D << "adding localhost hostname" << std::endl;

                try
                {
                    tryAddHost( "localhost" );
                    int foo;
                    
                }
                catch( std::exception& e )
                {
                    LG_CTX_W << " can't add local hostname for localhost"
                             << " e:" << e.what() << std::endl;
                }

                struct utsname buf;
                if(! uname( &buf ) )
                {
                    std::string rdn = buf.nodename;
                    
                    LG_CTX_D << "adding localhost rdn:" << rdn << std::endl;
                    try
                    {
                        tryAddHost( rdn );
                    }
                    catch( std::exception& e )
                    {
                        LG_CTX_W << " can't add local hostname for rdn:" << rdn
                                 << " e:" << e.what() << std::endl;
                    }
                }
            }

        
        
        

        virtual fh_context getSubContext( const std::string& rdn )
            {
                if( _Base::priv_isSubContextBound( rdn ))
                {
                    return _Base::getSubContext( rdn );
                }
                
                std::string s = TryToCheckServerExists( rdn );
        
                if( s.length() )
                {
                    fh_stringstream ss;
                    ss << "getSubContext() can never contact the server"
                       << " reason:" << s
                       << " NoSuchSubContext:" << rdn;
                    Throw_NoSuchSubContext( tostr(ss), this );
                }
        
                if( !_Base::priv_isSubContextBound( rdn ))
                {
//                    Context* c = priv_CreateContext( this, rdn );
                    Context* c = new OurDirectChildrenContextClass( this, rdn );
                    bool created = true;
                    const fh_context& ret = this->Insert( c, created );
                    this->bumpVersion();
                    return ret;
                }
                return _Base::getSubContext( rdn );
            }
            

        
        virtual bool isSubContextBound( const std::string& rdn )
            {
                return true;
            }

        networkRootContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }

        networkRootContext( Context* parent, const std::string& rdn, bool bindall )
            :
            _Base( parent, rdn )
            {
                try
                {
                    
                this->AddRef();

                if( bindall )
                {
                    
//                 cerr << " networkRootContext about to start binding all interfaces "
//                      << " rdn:" << rdn
//                      << " bindall:" << bindall
//                      << std::endl;
                
                
                LG_CTX_D << "networkRootContext(enter) rdn:" << rdn << std::endl;
                
                OurDirectChildrenContextClass* child;

                LG_CTX_D << "adding localhost" << std::endl;

                tryAugmentLocalhostNames();
                
                LG_CTX_D << "networkRootContext(done) rdn:" << rdn << std::endl;
                }
                
                }
                catch( std::exception& e )
                {
                    this->Release();
                    throw e;
                }

//                this->dumpOutItems();
                this->Release();
//                 cerr << " networkRootContext DONE binding all interfaces "
//                      << " rdn:" << rdn
//                      << " bindall:" << bindall
//                      << std::endl;
                
            }
        
        virtual ~networkRootContext()
            {
            }
    };


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
    
    /**
     * Context that allows the collection of new recommended EA when EA are
     * being created.
     */
    template <class ParentContext>
    class FERRISEXP_DLLLOCAL RecommendedEACollectingContext
        :
        public ParentContext
    {
        typedef RecommendedEACollectingContext  _Self;
        typedef ParentContext                   _Base;

        fh_stringstream RecommentedEAStream;

    protected:
        
        void appendToRecommentedEA( const std::string& s )
            {
                RecommentedEAStream << "," << s;
            }

        virtual bool setAttribute( const std::string& rdn,
                                   EA_Atom* atx,
                                   bool addToREA,
                                   XSDBasic_t sct = XSD_UNKNOWN,
                                   bool isStateLess = false )
            {
                bool rc = _Base::setAttribute( rdn, atx, addToREA, sct, isStateLess );
                if( rc && addToREA )
                {
                    appendToRecommentedEA( rdn );
                }
                return rc;
            }
        
    public:

        RecommendedEACollectingContext( Context* parent, const std::string& rdn )
            :
            ParentContext( parent, rdn )
            {
                RecommentedEAStream << "name";
            }

        RecommendedEACollectingContext()
            {
                RecommentedEAStream << "name";
            }
        
        virtual ~RecommendedEACollectingContext()
            {}

        virtual std::string getRecommendedEA()
            {
                return adjustRecommendedEAForDotFiles(this, tostr(RecommentedEAStream));
            }
    };
    


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    template <class ChildContext, class ParentContext = FakeInternalContext >
    class Statefull_Recommending_ParentPointingTree_Context
        :
        public 
        ParentPointingTreeContext< ChildContext, RecommendedEACollectingContext< ParentContext > >
    {
    protected:
        
        typedef Statefull_Recommending_ParentPointingTree_Context<
            ChildContext, ParentContext > _Self;
        
        typedef 
        ParentPointingTreeContext< ChildContext, RecommendedEACollectingContext< ParentContext > >
        _Base;

    public:
        
        Statefull_Recommending_ParentPointingTree_Context
        ( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                /*
                 * NB: This line taints the ability to have stateless EA apart
                 * from what context et al already provide.
                 */
                this->createStateLessAttributes();
            }
    };


    template <class ChildContext, class ParentContext = FakeInternalContext >
    class StateLessEAHolding_Recommending_ParentPointingTree_Context
        :
        public 
        StateLessEAHolder<
        /**/ ChildContext,
        /**/ ParentPointingTreeContext<
        /**/     ChildContext,
        /**/     RecommendedEACollectingContext< ParentContext > > >
    {
    protected:
        
        typedef StateLessEAHolding_Recommending_ParentPointingTree_Context<
            ChildContext, ParentContext > _Self;
        
        typedef 
        StateLessEAHolder<
        /**/ ChildContext,
        /**/ ParentPointingTreeContext<
        /**/     ChildContext,
        /**/     RecommendedEACollectingContext< ParentContext > > >
        _Base;


    public:
        
        StateLessEAHolding_Recommending_ParentPointingTree_Context
        ( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
            }
    };
    
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    namespace Factory
    {
        /**
         * Create a context whos parent is "parent" and whos contents is the
         * set union of the children in unionContexts.
         * ie. The returned context will claim to have a parent as given in "parent"
         * and report the union of all the children from unionContexts as its subcontexts.
         *
         * The return context itself is modeled after the first context from unionContexts.
         *
         * parent ----------------------------
         *  |               \                 \
         * unionContexts.1  unionContexts.2  unionContexts.3
         *  |      |         |     |          |     |
         * uc1.a  uc1.b     uc2.a uc2.d      uc3.b uc3.z
         *
         * In the above, parent is the return values parent, unionContexts.n are
         * the contexts in their list order from unionContexts. The return value
         * will look very similar to unionContexts.1. The children of the return
         * value are groomed from ucN.x with those children sharing the same filename
         * culled to only showing the first such child context.
         */
        FERRISEXP_API fh_context MakeUnionContext(
            const fh_context& parent,
            std::list< fh_context > unionContexts );


        /**
         * Create a context whos parent is "parent" and whos contents the result of
         * applying set_difference() starting with unionContexts.1 and removing all
         * contexts that appear in unionContexts.2 ... unionContexts.n
         */
        FERRISEXP_API fh_context MakeSetDifferenceContext(
            const fh_context& parent,
            std::list< fh_context > sdContexts );


        /**
         * Create a context whos parent is "parent" and whos contents the result of
         * applying set_intersection() on all of sdContexts.1 ... sdContexts.n
         */
        FERRISEXP_API fh_context MakeSetIntersectionContext(
            const fh_context& parent,
            std::list< fh_context > sdContexts,
            bool isCannibal = false );

        /**
         * Create a context whos parent is "parent" and whos contents the result of
         * applying set_symmetric_difference() on all of sdContexts.1 ... sdContexts.n
         */
        FERRISEXP_API fh_context MakeSetSymmetricDifferenceContext(
            const fh_context& parent,
            std::list< fh_context > sdContexts );

        /**
         * Wrap the context 'c' in a simple manyToOne context. The return value
         * behaves very much like the context 'c' but is a ManyBaseToOneViewContext
         * subclass so that other ManyBaseToOneViewContext subclasses can have
         * views based off it and cannibalize the return values contexts.
         */
        FERRISEXP_API fh_context MakeManyBaseToOneChainedViewContext(
            const fh_context& c );


        /**
         * Create a context whos parent is "parent" and whos contents is the
         * set union of the children in sdContexts.
         * ie. The returned context will claim to have a parent as given in "parent"
         * and report the union of all the children from unionContexts as its subcontexts.
         *
         * A "diff" style operation is performed on a per child basis and
         * changes relative to the first context in sdContexts are presented as EA.
         *
         * parent -----------
         *  |               \ 
         * sdContexts.1   sdContexts.2 
         *  |      |         |     |   
         * uc1.a  uc1.b     uc2.a uc2.d 
         *
         * In the above, parent is the return values parent, sdContexts.n are
         * the contexts in their list order from sdContexts. The return value
         * will look very similar to sdContexts.1. The children of the return
         * value are groomed from ucN.x with those children sharing the same filename
         * culled to only showing the first such child context.
         *
         * The difference from the first to the second context
         * object added is shown. Interesting attributes such as
         * was-created
         * was-deleted
         * is-same
         * unidiff
         * different-line-count
         * are added to the view and the union of both contexts is presented.
         * Objects that are not in the first context will be shown with was-deleted=1
         * and new objects in the first context wil appear with was-created=1
         */
        FERRISEXP_API fh_context MakeDiffContext(
            const fh_context& parent,
            std::list< fh_context > sdContexts );


        /**
         *
         * Make a context which will happily "create" new files, though not pass that info
         * on to the base context until later. EA can be written and updated for any new children
         * and those updates will not be processed until explicitly told to do so.
         *
         * Note that the passed and returned context is for the parent. For example,
         * passing postgresql://localhost/mydb/mytable as the argument and any new tuples
         * created using the returned context will be delayed until explicitly commited.
         *
         * To process updates and commit then the EA ferris-process-delayed-updates must be set to 1
         *
         */
        FERRISEXP_API fh_context MakeDelayedCommitParentContext( const fh_context& c );
        
    };

    class SelectionContext;
    
    class SelectionContextRDNConflictResolver : public Handlable
    {
    public:
        virtual std::string getRDN( SelectionContext* selc, fh_context newChild ) = 0;
        virtual fh_context createChild( SelectionContext* selc, fh_context newChild ) = 0;
    };
    
    FERRIS_SMARTPTR( SelectionContextRDNConflictResolver, fh_SelectionContextRDNConflictResolver );
    fh_SelectionContextRDNConflictResolver get_SelectionContextRDNConflictResolver_UseURLAsRDN();
    fh_SelectionContextRDNConflictResolver get_SelectionContextRDNConflictResolver_MonsterName();

    

    
    /**
     * This context bends the rules of subcontext creation. Instead of the metadata
     * context being used to describe a new context that should be created, the
     * metadata context is attached as a child.
     *
     * This allows one to create one of these
     * (via the selectionfactory://->createSubContext())
     * and then just use createSubContext on this object to attach a bunch of other
     * contexts as children. Note that if one is viewing this context as a dir then
     * they really should include the URL in the view so that people can see where
     * the different subcontexts really come from.
     */
    class FERRISEXP_API SelectionContext
        :
        public StateLessEAHolder< SelectionContext, FakeInternalContext >
    {
        typedef StateLessEAHolder< SelectionContext, FakeInternalContext > _Base;
        typedef Context _BaseNoDelegate;
        
#ifndef LIBFERRIS_INTERNAL_COMPILING_SWIG_WRAPPER
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );
#endif
        std::string m_reportedRDN;
        fh_SelectionContextRDNConflictResolver m_rdnConflictResolver;
        EAIndex::fh_fwdeaidx m_fwdidx;
        
    protected:

        virtual bool isDir();
        virtual void priv_read();

        static fh_stringstream SL_getNameOnlyStream( SelectionContext* c, const std::string& ean, EA_Atom* atom );
        static fh_stringstream SL_getSelectionAddedOrderIDStream( SelectionContext* c, const std::string& ean, EA_Atom* atom );

        virtual bool isAttributeBound( const std::string& rdn,
                                       bool createIfNotThere = true
            );
        fh_attribute
        getAttribute( const std::string& _rdn );
        
        
    public:

        SelectionContext( Context* parent, const std::string& rdn );
        virtual ~SelectionContext();

        void setSelectionContextRDNConflictResolver( fh_SelectionContextRDNConflictResolver f );
        
        virtual fh_context
        createSubContext( const std::string& rdn,
                          fh_context md = 0 );

        fh_context insert( fh_context c );
        void clear();
        bool empty();
        
        void createStateLessAttributes( bool force = false );

        void setReportedRDN( const std::string& rdn );
        virtual std::string getURL();
        std::string private_getStrAttr( const std::string& rdn,
                                        const std::string& def,
                                        bool getAllLines,
                                        bool throwEx );
        virtual const std::string& getDirName() const;
        virtual std::string getDirPath();

        void setForwardEAIndexInterface( EAIndex::fh_fwdeaidx fidx );
        EAIndex::fh_fwdeaidx getForwardEAIndexInterface();
    };

    FERRIS_CTX_SMARTPTR( SelectionContext, fh_contextlist );

    namespace Factory
    {
        /**
         * Make a context that contains other contexts but doesn't really exist.
         * This is very much like a "selection" as defined by
         * fh_context selfactory = Resolve( "selectionfactory://" );
         * fh_context selection  = selfactory->createSubContext( "" );
         * but the returned value has an explicit type and has insert() etc methods.
         */
        template < class Iter >
        fh_contextlist MakeContextList( Iter begin, Iter end )
        {
            fh_context selfactory = Resolve( "selectionfactory://" );
            fh_context selection  = selfactory->createSubContext( "" );

            for( Iter iter = begin; iter != end; ++iter )
                selection->createSubContext( "", *iter );

            fh_contextlist ret = dynamic_cast<SelectionContext*>( GetImpl( selection ));
            return ret;
        }
        /**
         * Make an empty context list for incremental building with insert()
         */
        FERRISEXP_API fh_contextlist MakeContextList();
        
        

        /**
         * Make a context that is like its delegate except that if an EA is not
         * bound on it then it will forward the request to its parent.
         *
         * This method should be called on the root of a tree and it will create
         * DelegatingEAContext wrappers for all of its children
         *
         * @param c is the delegate to use
         * @return a InheritingEAContext wrapper for c.
         */
        FERRISEXP_API fh_context makeInheritingEAContext( fh_context c );
         
    };

    
};
#endif


