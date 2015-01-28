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

    $Id: RootContext.cpp,v 1.6 2010/09/24 21:30:57 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <RootContext_private.hh>

// required for NativeVFS_RootContextDropper
#include <Resolver_private.hh>


using namespace std;

namespace Ferris
{
    struct FERRISEXP_DLLLOCAL RootRootContext : public FakeInternalContext
    {
        RootRootContext( Context* parent, const std::string& rdn )
            :
            FakeInternalContext( parent, rdn )
            {
            }
        virtual ~RootRootContext()
            {
            }

        virtual void priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    typedef RootContextFactory::Droppers_t Droppers_t;
                    const   Droppers_t& d = RootContextFactory::getDroppers();
                    typedef Droppers_t::const_iterator iterT;
                    RootContextFactory* rfac = new RootContextFactory();
                    
                    for( iterT iter = d.begin(); iter != d.end(); iter++)
                    {
                        string urlScheme = iter->first;
                        
                        LG_CTX_D << "Testing ctx -->:" << urlScheme << ":<-- "
                             << " isBound():" << priv_isSubContextBound( urlScheme )
                             << endl;
                        if( priv_isSubContextBound( urlScheme ) || urlScheme == "root" )
                            continue;

                        if( iter->second->isTopLevel() )
                        {
                            LG_CTX_D << "Adding ctx:" << urlScheme << endl;
                            RootContextDropper* f = iter->second;
                            rfac->setContextClass( urlScheme );
                            fh_context delegate = f->Brew( rfac );
                            LG_CTX_D << "Delegate.rdn:" << delegate->getDirName() << endl;
                        
                            Context* child = new RootContext( this,
                                                              GetImpl(delegate),
                                                              urlScheme );
                            Insert( child );

                            // explicit leak here.
                            child->AddRef();
                            child->AddRef();
                            RootContextFactory::pushAdditionalRootContextClassName(
                                urlScheme, GetImpl(delegate) );
                            
                            LG_CTX_D << " Child.name:" << child->getDirName()
                                 << " child.path:" << child->getDirPath()
                                 << " child.url:"  << child->getURL()
                                 << " urlScheme:" << urlScheme
                                 << " child:" << toVoid( child )
                                 << " delegate:" << toVoid( delegate )
                                 << endl;
//                             cerr << " Child.name:" << child->getDirName()
//                                  << " child.path:" << child->getDirPath()
//                                  << " child.url:"  << child->getURL()
//                                  << " urlScheme:" << urlScheme
//                                  << " child:" << toVoid( child )
//                                  << " delegate:" << toVoid( delegate )
//                                  << endl;

//                             child->setOverMountContext( delegate );
//                             delegate->setCoveredContext( child );
//                            child->readOverMount();

//                            delegate->read();

//                             LG_CTX_D << "rrc::read() --- delegate.begin to delegate.end --- " << endl;
//                             for( Context::iterator ci = delegate->begin();
//                                  ci != delegate->end(); ++ci )
//                             {
//                                 LG_CTX_D << "ci.name:" << (*ci)->getDirName()
//                                      << " ci.url:" << (*ci)->getURL()
//                                      << endl;
//                             }                            
                        }
                    }

//                    dumpOutItems();

                    stringset_t& sl = ImplementationDetail::getStaticLinkedRootContextNames();

                    for( stringset_t::const_iterator si = sl.begin(); si!=sl.end(); ++si )
                    {
                        string urlScheme = *si;
                        
                        if( !priv_isSubContextBound( urlScheme ) )
                        {
//                            cerr << "making scheme:" << urlScheme << endl;

                            fh_context delegate = Resolve( urlScheme + "://" );
                            LG_CTX_D << "Delegate.rdn:" << delegate->getDirName() << endl;
                            
                            Context* child = new RootContext( this,
                                                              GetImpl(delegate),
                                                              urlScheme );
                            Insert( child );
                        }
                    }
                }
                


                for( Context::iterator ci = begin(); ci != end(); ++ci )
                {
                    LG_CTX_D << "ci.name:" << (*ci)->getDirName()
                         << " ci.url:" << (*ci)->getURL()
                         << endl;
                }
                fh_context x = getSubContext( "file" );
                LG_CTX_D << "direct test url:" << x->getURL() << endl;
            }
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /** INIT STUFF ******************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    RootContext::RootContext( Context* parent,
                              Context* delegate,
                              const std::string& rdn )
        :
        ChainedViewContext( parent, delegate, false, false )
    {
        LG_CTX_D << "RootContext() rdn:" << rdn << endl;
        if( rdn.empty() )  setContext( parent, delegate->getDirName() );
        else               setContext( parent, rdn );

        SetupEventConnections();
        createStateLessAttributes();
    }
    
    Context*
    RootContext::priv_CreateContext( Context* parent, string rdn )
    {
        RootContext* ret = new RootContext( parent, 0 );
        ret->setContext( parent, rdn );
        return ret;
    }

//     void
//     RootContext::OnExists ( NamingEvent_Exists* ev,  string olddn, string newdn )
//     {
//         LG_CTX_D << "RootContext::OnExists() olddn:" << olddn
//              << " url:" << ev->getSource()->getSubContext( olddn )->getURL()
//              << endl;

//         fh_context c = ev->getSource();
//         if( !priv_isSubContextBound( olddn ))
//         {
//             Insert( GetImpl(c->getSubContext( olddn )) );
//             LG_CTX_D << "RootContext::OnExists() olddn:" << olddn
//                  << " url:" << ev->getSource()->getSubContext( olddn )->getURL()
//                  << " inserted."
//                  << endl;
//         }
//     }

    void
    RootContext::read( bool force )
    {
//         if( OverMountContext_Delegate )
//             OverMountContext_Delegate->read();

        LG_CTX_D << "rc::read(top) force:" << force
             << " ReadingDir:" << ReadingDir
             << " HaveReadDir:" << HaveReadDir
             << endl;
        if( OverMountContext_Delegate )
        {
            LG_CTX_D  << " omc.ReadingDir:" << OverMountContext_Delegate->ReadingDir
                      << " omc.HaveReadDir:" << OverMountContext_Delegate->HaveReadDir
                      << endl;
        }
        
        if( ReadingDir )
            return;
        if( HaveReadDir && !force )
        {
            EnsureStartStopReadingIsFiredRAII _raii1( this );
            return;
        }
        
        {
            EnsureStartStopReadingIsFiredRAII _raii1( this );
            ReadingDirRAII _raiird1( this, true );
            Delegate->read( force );
            HaveReadDir = 1;
        }
        
        LG_CTX_D << "RootContext::read() --------------- DEBUG starting ------------" << endl;
        LG_CTX_D << " this:" << toVoid( this ) << endl;
        LG_CTX_D << " omc:" << toVoid( OverMountContext_Delegate ) << endl;
        LG_CTX_D << " cc:" << toVoid( CoveredContext ) << endl;
        
//        dumpOutItems();

        LG_CTX_D << "RootContext::read() --- this.begin to this.end --- " << endl;
        for( Context::iterator ci = begin(); ci != end(); ++ci )
        {
            LG_CTX_D << "ci.name:" << (*ci)->getDirName()
                 << " ci.url:" << (*ci)->getURL()
                 << endl;
        }

        if( OverMountContext_Delegate )
        {
            LG_CTX_D << " this.url:" << getURL() << endl;
            LG_CTX_D << "  omc.url:" << OverMountContext_Delegate->getURL() << endl;
//            OverMountContext_Delegate->dumpOutItems();
            LG_CTX_D << "RootContext::read() --- omc.begin to omc.end --- " << endl;
            for( Context::iterator ci = OverMountContext_Delegate->begin();
                 ci != OverMountContext_Delegate->end(); ++ci )
            {
                LG_CTX_D << "ci.name:" << (*ci)->getDirName()
                     << " ci.url:" << (*ci)->getURL()
                     << endl;
            }
        }
        
        
    }
    
    const std::string&
    RootContext::getDirName() const
    {
        return _NonChainedBase::getDirName();
    }

    string
    RootContext::getDirPath() throw (FerrisParentNotSetError)
    {
        return _NonChainedBase::getDirPath();
    }

    std::string
    RootContext::getURL()
    {
//         LG_CTX_D << "RootContext::getURL() ncb:" << _NonChainedBase::getURL()
//              << " del:" << Delegate->getURL()
//              << endl;

//         cerr << "RootContext::getURL() ncb:" << _NonChainedBase::getURL()
//              << " del:" << Delegate->getURL()
//              << endl;
        
        return _NonChainedBase::getURL();
    }

    std::string
    RootContext::getURLScheme()
    {
//        cerr << "RootContext::getURLScheme()" << endl;
        return _Base::getURLScheme();
    }
    

    std::set< std::string >&
    RootContext::getPreferLocalAttributeNames()
    {
        static std::set< std::string > ret;
        if( ret.empty() )
        {
            ret.insert( "name" );
            ret.insert( "path" );
            ret.insert( "url" );
        }
        return ret;
    }
    bool
    RootContext::isAttributeLocal( const std::string& s )
    {
        std::set< std::string >& localNames = getPreferLocalAttributeNames();
        return localNames.end() != localNames.find( s );
    }
    
    
    std::string
    RootContext::private_getStrAttr( const std::string& rdn,
                                     const std::string& def,
                                     bool getAllLines,
                                     bool throwEx )
    {
        if( isAttributeLocal( rdn ) )
            return _NonChainedBase::private_getStrAttr( rdn, def, getAllLines, throwEx );
        return _Base::private_getStrAttr( rdn, def, getAllLines, throwEx );
    }

    fh_attribute
    RootContext::getAttribute( const std::string& rdn ) throw( NoSuchAttribute )
    {
        if( isAttributeLocal( rdn ) )
            return _NonChainedBase::getAttribute( rdn );
        return _Base::getAttribute( rdn );
    }

    bool
    RootContext::isAttributeBound(
        const std::string& rdn,
        bool createIfNotThere )
        throw( NoSuchAttribute )
    {
        if( isAttributeLocal( rdn ) )
            return _NonChainedBase::isAttributeBound( rdn, createIfNotThere );
        return _Base::isAttributeBound( rdn, createIfNotThere );
    }
    
    
    
    AttributeCollection::AttributeNames_t&
    RootContext::getAttributeNames( AttributeNames_t& ret )
    {
        AttributeCollection::AttributeNames_t t1;
        AttributeCollection::AttributeNames_t t2;
        _Base::getAttributeNames( t1 );
        std::set< std::string >& d = getPreferLocalAttributeNames();
        copy( d.begin(),   d.end(), back_inserter( t2 ));
        return mergeAttributeNames( ret, t1, t2 );
    }

    int
    RootContext::getAttributeCount()
    {
        AttributeNames_t tmp;
        getAttributeNames( tmp );
        return tmp.size();
    }
    
    
    class RootContextVFS_RootContextDropper : public RootContextDropper
    {
    public:
        RootContextVFS_RootContextDropper()
            {
                RootContextFactory::Register("root", this);
            }

        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
            {
                static fh_context rr = 0;
                if( !rr )
                {
                    rr = new RootRootContext( 0, "/" );
                }
                
//                 for( RootContextFactory::Droppers_t::iterator iter = rf->getDroppers().begin();
//                      iter != rf->getDroppers().end(); iter++)
//                 {
// //                LG_CTX_D << "Adding ctx:" << iter->first << endl;
//                     RootContextDropper* f = iter->second;
//                     fh_context delegate = f->Brew( 0 );
//                     Context* child = new RootContext( rr,
//                                                       GetImpl(delegate),
//                                                       f );
//                     rr->Insert( child );
//                 }
            
                return rr;
            }
    
    
    };

    static RootContextVFS_RootContextDropper ___RootContextVFS_static_init;
    
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
};
