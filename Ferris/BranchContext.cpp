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

    $Id: BranchContext.cpp,v 1.9 2011/06/17 21:30:21 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <BranchContext_private.hh>
#include <Resolver_private.hh>

using namespace std;

namespace Ferris
{
#define DUBCORE_DESCRIPTION "dc:description"

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    void
    FerrisBranchRootContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
    }


    void
    FerrisBranchRootContext::priv_read()
    {
        staticDirContentsRAII _raii1( this );

        if( empty() )
        {
            for( RootContextFactory::Droppers_t::iterator iter = RootContextFactory::getDroppers().begin();
                 iter != RootContextFactory::getDroppers().end(); iter++)
            {
                try
                {
                    string rdn = iter->first;
                    LG_CTX_D << "priv_read() rdn:" << rdn << endl;
                    

                    if( iter->second->isTopLevel() )
                    {
                        fh_context delegate = Resolve( rdn + "://" );
                        fh_context child = m_BranchInternalContextCreatorFunctor( this, delegate, rdn );
                    
//                         LG_CTX_D << "priv_read() adding child..." << endl
//                              << " child.name:" << child->getDirName()
//                              << " child.path:" << child->getDirPath()
//                              << " child.url:"  << child->getURL()
//                              << " rdn:" << rdn
//                              << endl;
                        
                        addNewChild( child );
                    }
                }
                catch( exception& e )
                {
                    LG_CTX_W << "priv_read() e:" << e.what() << endl;
                }
            }
        }
    }
    
    FerrisBranchRootContext::FerrisBranchRootContext(
        BranchInternalContextCreatorFunctor_t m_BranchInternalContextCreatorFunctor )
        :
        _Base( 0, "/" ),
        m_BranchInternalContextCreatorFunctor( m_BranchInternalContextCreatorFunctor )
    {
        createStateLessAttributes();
    }
    
    FerrisBranchRootContext::~FerrisBranchRootContext()
    {
    }
    
    void
    FerrisBranchRootContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }

    BranchInternalContextCreatorFunctor_t&
    FerrisBranchRootContext::getCreator()
    {
        return m_BranchInternalContextCreatorFunctor;
    }

    stringset_t&
    FerrisBranchRootContext::getForceLocalAttributeNames()
    {
        static stringset_t ret;
        if( ret.empty() )
        {
            ret.insert("is-active-view");
        }
        return ret;
    }
    
    
    
    class FERRISEXP_DLLLOCAL FerrisBranchRootContext_RootContextDropper
        :
        public RootContextDropper
    {
    public:
        typedef std::map< string, BranchInternalContextCreatorFunctor_t > m_functors_t;
        m_functors_t m_functors;

        typedef std::map< string, fh_context > m_cache_t;
        m_cache_t m_cache;
        
        FerrisBranchRootContext_RootContextDropper()
            {
            }

        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
            {
                string scheme = rf->getContextClass();
//                cerr << "FerrisBranchRootContext_RootContextDropper() scheme:" << scheme << endl;
                
                if( m_cache.end() == m_cache.find( scheme ) )
                {
                    if( m_functors.end() == m_functors.find( scheme ))
                    {
                        LG_CTX_ER << "BAD! can't find information about scheme:" << scheme
                                  << " branch filesystem!" << endl;
                    }
                    m_cache.insert( make_pair( scheme, new FerrisBranchRootContext( m_functors[ scheme ] ) ));
                }
                return m_cache[ scheme ];
            }
    };
    bool FerrisBranchRootContext_Register(
        const std::string& url_scheme,
        BranchInternalContextCreatorFunctor_t m_BranchInternalContextCreatorFunctor )
    {
        static FerrisBranchRootContext_RootContextDropper ___FerrisBranchRootContext_static_init;
        ___FerrisBranchRootContext_static_init.m_functors[ url_scheme ] = m_BranchInternalContextCreatorFunctor;
        RootContextFactory::Register( url_scheme, &___FerrisBranchRootContext_static_init );
    }
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    FerrisBranchInternalContext::FerrisBranchInternalContext(
        Context* theParent,
        const fh_context& theDelegate )
        :
        ChainedViewContext( theDelegate, false )
    {
        setContext( theParent, theDelegate->getDirName() );
        
        createStateLessAttributes();
//         SetupEventConnections();
//         Delegate->emitExistsEventForEachItem();
    }

    FerrisBranchInternalContext::FerrisBranchInternalContext(
        Context* theParent,
        const fh_context& theDelegate,
        const std::string& rdn )
        :
        ChainedViewContext( theDelegate, rdn, false )
    {
        setContext( theParent, rdn );
        
        createStateLessAttributes();
//         SetupEventConnections();
//         Delegate->emitExistsEventForEachItem();

//         cerr << "FerrisBranchInternalContext() ctor. "
//              << " path:" << getDirPath()
//              << " name:" << getDirName()
//              << " rdn:" << rdn
//              << endl;
    }
        
    
    
    FerrisBranchInternalContext::~FerrisBranchInternalContext()
    {
    }

    //
    // Short cut loading each dir unless absolutely needed.
    //
    fh_context
    FerrisBranchInternalContext::priv_getSubContext( const string& rdn )
        throw( NoSuchSubContext )
    {
        try
        {
            LG_CTX_D << "FerrisBranchInternalContext::priv_getSubContext() "
                     << " url:" << getURL()
                     << " rdn:" << rdn
                     << " priv_isBound:" << priv_isSubContextBound( rdn )
                     << endl;
            
            if( priv_isSubContextBound( rdn ) )
            {
                return _Base::priv_getSubContext( rdn );
            }

            if( rdn.empty() )
            {
                // stat failed
                fh_stringstream ss;
                ss << "NoSuchSubContext no rdn given";
                Throw_NoSuchSubContext( tostr(ss), this );
            }

            //
            // Handle a delegate that needs to overmount
            //
            if( !Delegate->isSubContextBound( rdn ) )
            {
               if( Delegate->hasOverMounter() )
               {
                  Delegate->tryToOverMount();
                  Delegate->read();
               }
            }
            
            fh_context dsub = Delegate->getSubContext( rdn );
            fh_context cc = getCreator()( this, dsub, dsub->getDirName() );
            Insert( GetImpl(cc), false );
            return _Base::priv_getSubContext( rdn );
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

    
    stringset_t&
    FerrisBranchInternalContext::getForceLocalAttributeNames()
    {
        static stringset_t ret;
        if( ret.empty() )
        {
            ret.insert("is-dir");
            ret.insert("is-file");
            ret.insert("treeicon");

            ret.insert("name");
            ret.insert("path");
            ret.insert("url");
        }
        return ret;
    }
    
    bool
    FerrisBranchInternalContext::isDir()
    {
        return true;
    }

    

    /**
     * Either the link target's dirName() or what is set with setLocalName()
     */
    const std::string&
    FerrisBranchInternalContext::getDirName() const
    {
        const string& rdn_ddb = _DontDelegateBase::getDirName();
        const string& rdn_target = Delegate->getDirName();

//         LG_CTX_D << "FerrisBranchInternalContext::getDirName() rdn_ddb:" << rdn_ddb
//              << " rdn_target:" << rdn_target
//              << " scheme:" << RootContextFactory::getRootContextClassName( this )
//              << endl;
        
        if( rdn_target == "/" )
        {
            return rdn_ddb;
//             string ret = RootContextFactory::getRootContextClassName( this );
//             return ret;
        }

        
        if( rdn_ddb != rdn_target )
            return rdn_ddb;
        return rdn_target;
    }

    std::string
    FerrisBranchInternalContext::getDirPath() throw (FerrisParentNotSetError)
    {
//         cerr << "FerrisBranchInternalContext::getDirPath() "
//              << " dele-ret:" << _Base::getDirPath() << endl
//              << " nodele-ret:" << _DontDelegateBase::getDirPath()
//              << endl;
        
//         if( isParentBound() )
//         {
//             if( FerrisBranchRootContext* p = dynamic_cast<FerrisBranchRootContext*>(getParent()))
//             {
//                 return "/" + getURLScheme();
//             }
//         }
        return _DontDelegateBase::getDirPath();
    }
    
    
    std::string
    FerrisBranchInternalContext::getURL()
    {
//         cerr << "FerrisBranchInternalContext::getURL() ret:"
//              <<  _DontDelegateBase::getURL()
//              << endl;
        
        return _DontDelegateBase::getURL();
    }

//     std::string
//     FerrisBranchInternalContext::private_getStrAttr( const std::string& rdn,
//                                                             const std::string& def,
//                                                             bool getAllLines,
//                                                             bool throwEx )
//     {
//         stringlist_t& sl = getForceLocalAttributeNames();
//         if( sl.end() != find( sl.begin(), sl.end(), rdn ) )
//             return _DontDelegateBase::private_getStrAttr( rdn, def, getAllLines, throwEx );

//         return _Base::private_getStrAttr( rdn, def, getAllLines, throwEx );
//     }

//     fh_attribute
//     FerrisBranchInternalContext::getAttribute( const string& rdn ) throw( NoSuchAttribute )
//     {
//         stringlist_t& sl = getForceLocalAttributeNames();
//         if( sl.end() != find( sl.begin(), sl.end(), rdn ) )
//             return _DontDelegateBase::getAttribute( rdn );
        
//         return Delegate->getAttribute(rdn);
//     }
//     AttributeCollection::AttributeNames_t&
//     FerrisBranchInternalContext::getAttributeNames( AttributeNames_t& ret )
//     {
//         AttributeCollection::AttributeNames_t t1;
//         AttributeCollection::AttributeNames_t t2;
//         Delegate->getAttributeNames( t1 );
//         _DontDelegateBase::getAttributeNames( t2 );
//         return mergeAttributeNames( ret, t1, t2 );
//     }
//     int
//     FerrisBranchInternalContext::getAttributeCount()
//     {
//         return Delegate->getAttributeCount();
//     }
//     bool
//     FerrisBranchInternalContext::isAttributeBound( const std::string& rdn,
//                                               bool createIfNotThere
//         ) throw( NoSuchAttribute )
//     {
//         stringlist_t& sl = getForceLocalAttributeNames();
//         if( sl.end() != find( sl.begin(), sl.end(), rdn ) )
//             return _DontDelegateBase::isAttributeBound( rdn, createIfNotThere );

//         return Delegate->isAttributeBound( rdn, createIfNotThere );
//     }
    
    fh_context
    FerrisBranchInternalContext::addNewChild( fh_context c )
    {
        Insert( GetImpl(c), false );
        return c;
    }
    
    void
    FerrisBranchInternalContext::UnPageSubContextsIfNeeded()
    {
        if( isTrue( getStrAttr( Delegate, "is-file", "false" )))
            return;
        
//        cerr << "FerrisBranchInternalContext::UnPageSubContextsIfNeeded() url:" << getURL() << endl;
        try
        {
            Delegate->UnPageSubContextsIfNeeded();
        }
        catch( exception& e )
        {
            LG_CTX_I << "non error FerrisBranchInternalContext::UnPageSubContextsIfNeeded() url:" << getURL()
                     << " warning:" << e.what()
                     << endl;
        }
    }

    long
    FerrisBranchInternalContext::guessSize() throw()
    {
        return Delegate->guessSize();
    }

    void
    FerrisBranchInternalContext::read( bool force )
    {
        if( isTrue( getStrAttr( Delegate, "is-file", "false" )))
        {
            LG_CTX_D << "FerrisBranchInternalContext::read() is-file." << endl;
            if( !force && SubContextCount() )
            {
                LG_CTX_D << "FerrisBranchInternalContext::read() is-file. cached" << endl;
                emitExistsEventForEachItemRAII _raii1( this );
                return;
            }

            priv_read_leaf();
        }
        else
        {
            LG_CTX_D << "FerrisBranchInternalContext::read() is-dir." << endl;

            if( !force && Delegate->HaveReadDir )
            {
                if( !getItems().empty() )
                {
                    LG_CTX_D << "FerrisBranchInternalContext::read() is-dir. cached" << endl;
                    HaveReadDir = true;
                    emitExistsEventForEachItemRAII _raii1( this );
                    return;
                }
            }

            LG_CTX_D << "FerrisBranchInternalContext::read() Reading delegate:" << Delegate->getURL()
                 << " delegate.path:" << Delegate->getDirPath()
                 << endl;

            EnsureStartStopReadingIsFiredRAII _raii1( this );
            clearContext();

            sigc::connection c = Delegate->getNamingEvent_Exists_Sig().connect(
                sigc::mem_fun( *this, &_Self::OnExists));
            Delegate->read( true );
            c.disconnect();
            
            HaveReadDir = true;
        }
    }

    fh_stringstream
    FerrisBranchInternalContext::SL_getIsFile( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << !c->isDir();
        return ss;
    }
    
    void
    FerrisBranchInternalContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            tryAddStateLessAttribute( "is-file", &_Self::SL_getIsFile, XSD_BASIC_BOOL );
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }

    FerrisBranchInternalContext*
    FerrisBranchInternalContext::priv_CreateContext( Context* parent, std::string rdn )
    {
        return 0;
    }
    
    void
    FerrisBranchInternalContext::OnDeleted( NamingEvent_Deleted* ev, string olddn, string newdn )
    {
        if( isSubContextBound( olddn ) )
        {
            fh_context c = getSubContext( olddn );
            Remove( GetImpl(getSubContext(olddn)) );
        }
    }

    BranchInternalContextCreatorFunctor_t&
    FerrisBranchInternalContext::getCreator()
    {
        if( isParentBound() )
        {
            Context* p = getParent();
            while( p )
            {
                if( FerrisBranchRootContext* cc = dynamic_cast<FerrisBranchRootContext*>( p ))
                {
                    return cc->getCreator();
                }
                if( !p->isParentBound() )
                    break;
                p = p->getParent();
            }
        }

        fh_stringstream ss;
        ss << "Cant get creator object for branch filesystem. url:" << getURL() << endl;
        Throw_FerrisInternalError( tostr(ss), this );
    }
    
    
    
        struct FerrisBranchInternalContextCreator
        {
            fh_context m_delegate;
            mutable BranchInternalContextCreatorFunctor_t m_createFunction;
            
            FerrisBranchInternalContextCreator(
                BranchInternalContextCreatorFunctor_t& m_createFunction,
                fh_context m_delegate
                )
                :
                m_createFunction( m_createFunction ),
                m_delegate( m_delegate )
                {}
            FerrisBranchInternalContext* create( Context* parent, const std::string& rdn ) const
                {
                    return m_createFunction( parent, m_delegate, rdn );
                }
            void setupExisting( FerrisBranchInternalContext* fc ) const
                {
                    fc->setupState( m_delegate );
                }
            void setupNew( FerrisBranchInternalContext* fc ) const
                {}
        };

    void
    FerrisBranchInternalContext::OnCreated( NamingEvent_Created* ev,
                                            const fh_context& subc,
                                            std::string olddn, std::string newdn )
    {
//         LG_CTX_D << "SortedContext::OnCreated rdn:" << olddn << endl;
//        fh_context subc = ev->getSource()->getSubContext( olddn );

        FerrisBranchInternalContext* fc = 0;
        fc = priv_ensureSubContext( subc->getDirName(), fc,
                                    FerrisBranchInternalContextCreator( getCreator(), subc ),
                                    true );
        
        
//         if( priv_isSubContextBound( olddn ) )
//         {
//             return;
//         }

//         fh_context cc = getCreator()( this, subc, subc->getDirName() );
//         Insert( GetImpl(cc), true );
    }
    
    void
    FerrisBranchInternalContext::OnExists( NamingEvent_Exists* ev,
                                           const fh_context& subc,
                                           string olddn, string newdn )
    {
//        fh_context subc = ev->getSource()->getSubContext( olddn );
        LG_CTX_D << "FerrisBranchInternalContext::OnExists() subc:" << subc->getURL() << endl;

        FerrisBranchInternalContext* fc = 0;
        fc = priv_ensureSubContext( subc->getDirName(), fc,
                                    FerrisBranchInternalContextCreator( getCreator(), subc ) );
        
//         if( !priv_isSubContextBound( olddn ) )
//         {
//             fh_context cc = getCreator()( this, subc, subc->getDirName() );
//             Insert( GetImpl(cc), false );
//         }
    }
    
};
