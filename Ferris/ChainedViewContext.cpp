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

    $Id: ChainedViewContext.cpp,v 1.15 2010/09/24 21:30:25 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Context.hh>
#include <ChainedViewContext.hh>
#include <Ferris_private.hh>
#include <FilteredContext_private.hh> // for debug msgs (dynamic_cast<> in dump())

using namespace std;


namespace Ferris
{
    fh_context getUnderlyingContext( fh_context c )
    {
        Context* ret = GetImpl(c);

        while( ChainedViewContext* rawc = dynamic_cast<ChainedViewContext*>( ret ))
        {
            if( rawc->Delegate )
                ret = GetImpl( rawc->Delegate );
            else
                break;
        }
        
        return ret;
    }


/**
 * Create a new decorator for Context objects.
 *
 * @see SetupEventConnections()
 *
 * @param ctx Context to delegate actions too.
 * @param setupEventConnections true if events should be setup right away. This param
 *        exists so that subclasses can perform work in their ctor before enabling
 *        event connections on the base class.
 */
    ChainedViewContext::ChainedViewContext(
        const fh_context& ctx,
        bool setupEventConnections )
        :
        Delegate(ctx)
    {
        if( setupEventConnections )
        {
            SetupEventConnections();
        }

        if( isBound( ctx ) )
            setContext( 0, ctx->getDirName() );
        else
            setContext( 0, "" );
//        remContextToMemoryManagementData( this );
    }

/**
 * Create a new decorator for Context objects.
 *
 * @see SetupEventConnections()
 *
 * @param ctx Context to delegate actions too.
 * @param rdn Filename to use instead of ctx->getDirName()
 * @param setupEventConnections true if events should be setup right away. This param
 *        exists so that subclasses can perform work in their ctor before enabling
 *        event connections on the base class.
 */
    ChainedViewContext::ChainedViewContext(
        const fh_context& ctx,
        const std::string& rdn,
        bool setupEventConnections )
        :
        Delegate(ctx)
    {
        if( setupEventConnections )
        {
            SetupEventConnections();
        }

        setContext( 0, rdn  );
//        remContextToMemoryManagementData( this );
    }

    
/**
 * Create a new decorator for Context objects.
 *
 * @see SetupEventConnections()
 *
 * @param parent   Parent for new context to have
 * @param delegate Context to delegate actions too.
 * @param setupEventConnections true if events should be setup right away. This param
 *        exists so that subclasses can perform work in their ctor before enabling
 *        event connections on the base class.
 */
    ChainedViewContext::ChainedViewContext(
        const fh_context& parent,
        const fh_context& delegate,
        bool setupEventConnections,
        bool callSetContextWithDelegateRdn )
        :
        Delegate(delegate)
    {
        if( setupEventConnections )
        {
            SetupEventConnections();
        }

        if( callSetContextWithDelegateRdn )
            setContext( parent, delegate->getDirName() );
//        remContextToMemoryManagementData( this );
    }

/**
 * Create a new decorator for Context objects.
 *
 * @see SetupEventConnections()
 *
 * @param parent   Parent for new context to have
 * @param delegate Context to delegate actions too.
 * @param rdn Filename to use instead of ctx->getDirName()
 * @param setupEventConnections true if events should be setup right away. This param
 *        exists so that subclasses can perform work in their ctor before enabling
 *        event connections on the base class.
 */
    ChainedViewContext::ChainedViewContext(
        const fh_context& parent,
        const fh_context& delegate,
        const std::string& rdn,
        bool setupEventConnections )
        :
        Delegate(delegate)
    {
        if( setupEventConnections )
        {
            SetupEventConnections();
        }
        setContext( parent, rdn  );
    }
    

    /**
     * Prevent bad deletes
     */
    ChainedViewContext::~ChainedViewContext()
    {
//        remContextToMemoryManagementData( this );
    }

    
    
    void
    ChainedViewContext::createStateLessAttributes( bool force )
    {
        static Util::SingleShot virgin;
        if( force || virgin() )
        {
            tryAddStateLessAttribute( "ferris-delegate-url",
                                      &_Self::SL_getDelegateURLStream,
                                      FXD_URL );
            tryAddStateLessAttribute( "ferris-delegate-path",
                                      &_Self::SL_getDelegatePathStream,
                                      FXD_URL );
            Context::createStateLessAttributes( true );
            Context::supplementStateLessAttributes( true );
        }
    }

    fh_stringstream
    ChainedViewContext::SL_getDelegateURLStream( Context* gc, const std::string& rdn, EA_Atom* atom )
    {
        ChainedViewContext* c = (ChainedViewContext*)gc;
        
        fh_stringstream ss;
        if( isBound( c->Delegate ) )
        {
            ss << c->Delegate->getURL();
        }
        return ss;
    }

    fh_stringstream
    ChainedViewContext::SL_getDelegatePathStream( Context* gc, const std::string& rdn, EA_Atom* atom )
    {
        ChainedViewContext* c = (ChainedViewContext*)gc;
        
        fh_stringstream ss;
        if( isBound( c->Delegate ) )
        {
            ss << c->Delegate->getDirPath();
        }
        return ss;
    }
    
    
    bool
    ChainedViewContext::getIsNativeContext() const
    {
        if( Delegate )
            return Delegate->getIsNativeContext();
        return _Base::getIsNativeContext();
    }
    
    void
    ChainedViewContext::setIsChainedViewContextRoot()
    {
        ref_count++;
        
    }
    
    void
    ChainedViewContext::SetupEventConnections( fh_context c )
    {
        if( !c )
            c = Delegate;

        if( m_ChainedViewContext_Called_SetupEventConnections )
            return;
        m_ChainedViewContext_Called_SetupEventConnections = true;

        LG_SORT_I << "SetupEventConnections() c:" << c->getURL() << endl;
        
        c->getNamingEvent_Deleted_Sig().connect(sigc::mem_fun( *this, &ChainedViewContext::OnDeleted));
        c->getNamingEvent_Exists_Sig().connect(sigc::mem_fun( *this, &ChainedViewContext::OnExists));
        c->getNamingEvent_Created_Sig().connect(sigc::mem_fun( *this, &ChainedViewContext::OnCreated));
        c->getNamingEvent_Changed_Sig().connect(sigc::mem_fun( *this, &ChainedViewContext::OnChanged));
        c->getNamingEvent_Start_Reading_Context_Sig().connect(
            sigc::mem_fun( *this, &ChainedViewContext::OnStartReading));
        c->getNamingEvent_Stop_Reading_Context_Sig().connect(
            sigc::mem_fun( *this, &ChainedViewContext::OnStopReading));
    }

    void
    ChainedViewContext::ensureEventConnections()
    {
        SetupEventConnections( 0 );
    }
    
    

/**
 * Called when a context in the base context has been deleted.
 *
 * We update local state and then emit the signal ourself to allow chaining.
 */
    void
    ChainedViewContext::OnDeleted( NamingEvent_Deleted* ev, string olddn, string newdn )
    {
//        Emit_Deleted( ev, newdn, olddn, 0 );
//        Remove( ev->getSource()->getSubContext( olddn ) );
        Remove( olddn );
    }

/**
 * Called when a new context has been discovered in the base context.
 *
 * We insert the new context and emit ourself for chaining.
 */
    void
    ChainedViewContext::OnExists ( NamingEvent_Exists* ev,
                                   const fh_context& subc,
                                   string olddn, string newdn )
    {
        LG_CTX_D << "ChainedViewContext::OnExists() olddn:" << olddn
                 << " url:" << ev->getSource()->getSubContext( olddn )->getURL()
                 << endl;

        fh_context c = ev->getSource();

        bool created = false;
        if( !priv_discoveredSubContext( subc->getDirName(), created ) )
        {
            Insert( GetImpl( subc ) );
        }
        
        
        
//         if( !priv_isSubContextBound( olddn ))
//         {
// //            Insert( GetImpl( c->getSubContext( olddn )) );
//             Insert( GetImpl( subc ) );
//             LG_CTX_D << "ChainedViewContext::OnExists() olddn:" << olddn
//                      << " url:" << ev->getSource()->getSubContext( olddn )->getURL()
//                      << " inserted."
//                      << endl;
//         }
        
// //    Emit_Exists( ev, newdn, olddn, 0 );
    }

    void
    ChainedViewContext::OnCreated( NamingEvent_Created* ev,
                                   const fh_context& subc,
                                   std::string olddn, std::string newdn )
    {
        LG_CTX_D << "ChainedViewContext::OnCreated" << endl;
        bool created = true;
        if( !priv_discoveredSubContext( subc->getDirName(), created ) )
        {
            Insert( GetImpl( subc ), 1, 1 );
        }
        
//        Insert( GetImpl( subc ), 1, 1 );
    }

    void
    ChainedViewContext::OnChanged( NamingEvent_Changed* ev,
                                   std::string olddn, std::string newdn )
    {
        LG_CTX_D << "ChainedViewContext::OnChanged old:" << olddn << " new:" << newdn
                 << " earl:" << getURL() << endl;
        Emit_Changed( ev, getURL(), getURL(), 0 );
    }
    


/**
 * For subclasses to override if they like.
 */
    void
    ChainedViewContext::OnStartReading( NamingEvent_Start_Reading_Context* )
    {
    }

/**
 * For subclasses to override if they like.
 */
    void
    ChainedViewContext::OnStopReading ( NamingEvent_Stop_Reading_Context* )
    {
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Delegating Attribute Methods.
    //
    ///////////////////////////////////////////////////////////////////////////////

/**
 * Passed right through to the base context.
 *
 * @see Context::getIStream()
 */
    fh_istream
    ChainedViewContext::getIStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               exception)
    {
        return Delegate->getIStream( m );
    }
    
/**
 * Passed right through to the base context.
 *
 * @see Context::getLocalIStream()
 */
    fh_istream
    ChainedViewContext::getLocalIStream( string& new_dn, ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               exception)
    {
        return Delegate->getLocalIStream( new_dn, m );
    }

/**
 * Passed right through to the base context.
 *
 * @see Context::getIOStream()
 */
    fh_iostream
    ChainedViewContext::getIOStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               CanNotGetStream,
               exception)
    {
        return Delegate->getIOStream( m );
    }


/**
 * If we have a parent we return it as normal,
 * If we have no parent, then we pass through to the base context.
 * 
 *
 * @see Context::getParent()
 */
    Attribute::Parent_t
    ChainedViewContext::getParent() throw (FerrisParentNotSetError)
    {
        return _Base::isParentBound() ? _Base::getParent() : Delegate->getParent();
    }
    

/**
 * If either we have a parent or the delegate does.
 *
 * @see Context::isParentBound()
 */
    bool
    ChainedViewContext::isParentBound()
    {
        if( !Delegate )
            return  _Base::isParentBound();
        
        return _Base::isParentBound() || Delegate->isParentBound();
    }
    
/**
 * Passed right through to the base context.
 *
 * @see Context::getDirName()
 */
    const std::string&
    ChainedViewContext::getDirName() const
    {
//        cerr << "ChainedViewContext::getDirName() ret:" << Delegate->getDirName() << endl;
        return Delegate->getDirName();
    }

    string
    ChainedViewContext::getDirPath() throw (FerrisParentNotSetError)
    {
        return Delegate->getDirPath();
    }

    std::string
    ChainedViewContext::getURL()
    {
        return Delegate->getURL();
    }


    /**
     * Subclasses can override this and supply a list of EA names which
     * by default should be taken from this context instead of Delegate.
     *
     * By default there are no local names.
     */
    stringset_t&
    ChainedViewContext::getForceLocalAttributeNames()
    {
        static stringset_t sl;
        return sl;
    }

    stringset_t&
    ChainedViewContext::getAugmentedForceLocalAttributeNames()
    {
        stringset_t& sl = getForceLocalAttributeNames();
        sl.insert("ferris-delegate-url");
        sl.insert("ferris-delegate-path");
        return sl;
    }
    
    


    /**
     * EA names not in getAugmentedForceLocalAttributeNames()
     * are passed right through to the base context.
     *
     * @see Context::getStrAttr()
     */
    std::string
    ChainedViewContext::private_getStrAttr( const std::string& rdn,
                                            const std::string& def,
                                            bool getAllLines,
                                            bool throwEx )
    {
        stringset_t& sl = getAugmentedForceLocalAttributeNames();
        if( !sl.empty() )
        {
            if( sl.find( rdn ) != sl.end() )
                return _Base::private_getStrAttr( rdn, def, getAllLines, throwEx );
        }
        
        return Delegate->private_getStrAttr( rdn, def, getAllLines, throwEx );
    }

    
    
    /**
     * EA names not in getAugmentedForceLocalAttributeNames()
     * are passed right through to the base context.
     *
     * @see Context::getAttribute()
     */
    fh_attribute
    ChainedViewContext::getAttribute( const string& rdn ) throw( NoSuchAttribute )
    {
        stringset_t& sl = getAugmentedForceLocalAttributeNames();
        if( !sl.empty() )
        {
            if( sl.find( rdn ) != sl.end() )
                return _Base::getAttribute( rdn );
        }
        return Delegate->getAttribute(rdn);
    }
    
    /**
     * EA names not in getAugmentedForceLocalAttributeNames()
     * are passed right through to the base context.
     *
     * @see Context::getAttributeNames()
     */
    AttributeCollection::AttributeNames_t&
    ChainedViewContext::getAttributeNames( AttributeNames_t& ret )
    {
        stringset_t& sl = getAugmentedForceLocalAttributeNames();
        if( !sl.empty() )
        {
            AttributeCollection::AttributeNames_t t1;
            AttributeCollection::AttributeNames_t t2;
            Delegate->getAttributeNames( t1 );
            _Base::getAttributeNames( t2 );
            return mergeAttributeNames( ret, t1, t2 );
        }
        
        return Delegate->getAttributeNames( ret );
    }
    
    /**
     * EA names not in getAugmentedForceLocalAttributeNames()
     * are passed right through to the base context.
     *
     * @see Context::getAttributeCount()
     */
    int
    ChainedViewContext::getAttributeCount()
    {
        stringset_t& sl = getAugmentedForceLocalAttributeNames();
        if( !sl.empty() )
        {
            AttributeNames_t tmp;
            getAttributeNames( tmp );
            return tmp.size();
        }
        return Delegate->getAttributeCount();
    }

    /**
     * EA names not in getAugmentedForceLocalAttributeNames()
     * are passed right through to the base context.
     *
     * @see Context::isAttributeBound()
     */
    bool
    ChainedViewContext::isAttributeBound( const std::string& rdn,
                                          bool createIfNotThere
        ) throw( NoSuchAttribute )
    {
        stringset_t& sl = getAugmentedForceLocalAttributeNames();
        if( !sl.empty() )
        {
            if( sl.find( rdn ) != sl.end() )
                return _Base::isAttributeBound( rdn, createIfNotThere );
        }

        if( !Delegate )
            return false;
        return Delegate->isAttributeBound( rdn, createIfNotThere );
    }
    
    

/**
 * Handlable::AddRef();
 */
    Handlable::ref_count_t
    ChainedViewContext::AddRef()
    {
        if( ref_count >= ImplementationDetail::MAX_REF_COUNT )
            return ref_count;
        return Handlable::AddRef();
    }
    
/**
 * Handlable::Release();
 */
    Handlable::ref_count_t
    ChainedViewContext::Release()
    {
        if( ref_count >= ImplementationDetail::MAX_REF_COUNT )
            return ref_count;
        return Handlable::Release();
    }

/**
 * Always false;
 */ 
    bool
    ChainedViewContext::all_attributes_have_single_ref_count()
    {
        return false;
    }

///////////////////////////////////////////////////////////////////////////////
//
// Delegating Context Methods.
//
///////////////////////////////////////////////////////////////////////////////

    fh_attribute
    ChainedViewContext::createAttribute( const string& rdn )
        throw( FerrisCreateAttributeFailed,
               FerrisCreateAttributeNotSupported,
               AttributeAlreadyInUse )
    {
        return Delegate->createAttribute( rdn );
    }


    fh_attribute
    ChainedViewContext::acquireAttribute( const string& rdn )
        throw( FerrisCreateAttributeFailed,
               FerrisCreateAttributeNotSupported )
    {
        return Delegate->acquireAttribute( rdn );
    }



/**
 * Passed right through to the base context.
 *
 * @see Context::createSubContext()
 */
    fh_context
    ChainedViewContext::createSubContext( const string& rdn, fh_context md ) 
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        LG_CTX_D << "ChainedViewContext::createSubContext() rdn:" << rdn << endl;
        return Delegate->createSubContext( rdn, md );
    }

    fh_context
    ChainedViewContext::createSubContext( const std::string& rdn, fh_mdcontext md )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        LG_CTX_D << "ChainedViewContext::createSubContext() rdn:" << rdn << endl;
//        fh_context c = GetImpl( md );
        fh_context c;
        Upcast( c, md );
        return createSubContext( rdn, c );
    }



/**
 * Passed right through to the base context.
 *
 * @see Context::getRelativeContext()
 */
    fh_context
    ChainedViewContext::getRelativeContext( const string& xdn, RootContextFactory* f )
        throw( NoSuchSubContext )
    {
        return Delegate->getRelativeContext(xdn,f);
    }
        
// Context::SubContextNames_t
// ChainedViewContext::getSubContextNames()
// {
//     return Delegate->getSubContextNames();
// }
    
// fh_context
// ChainedViewContext::getSubContext( const string& rdn ) throw( NoSuchSubContext )
// {
//     return Delegate->getSubContext(rdn);
// }
        
// bool
// ChainedViewContext::isSubContextBound( const string& rdn )
// {
//     return Delegate->isSubContextBound(rdn);
// }
    
/**
 * Nothing. Subclasses might add flavor to this if they want to force a read()
 * delegation.
 */
    void
    ChainedViewContext::read( bool force )
    {
    }
    

/**
 * Always 0
 */
    long
    ChainedViewContext::guessSize() throw()
    {
        return 0;
    }

    /**
     * Always true
     */
    bool
    ChainedViewContext::supportsReClaim()
    {
        return true;
    }

    void
    ChainedViewContext::dumpOutItems()
    {
        #define TARGETSTREAM cerr
        
        TARGETSTREAM << "ChainedViewContext::dumpOutItems( START ) url:" << getURL()
                     << " this:" << toVoid(dynamic_cast<Context*>(this))
                     << " cc:"   << toVoid(dynamic_cast<Context*>(GetImpl(CoveredContext)))
                     << " omc:"  << toVoid(dynamic_cast<Context*>(getOverMountContext()))
                     << " Delegate:" << toVoid(dynamic_cast<Context*>(GetImpl(Delegate)))
                     << endl;
        if( dynamic_cast<SortedContext*>(this) )
            TARGETSTREAM << " is-sorter " << endl;
        if( dynamic_cast<FilteredContext*>(this) )
            TARGETSTREAM << " is-filter " << endl;
        
        if( getOverMountContext() != this )
        {
            getOverMountContext()->dumpOutItems();
            return;
        }

        for( Items_t::iterator iter = getItems().begin();
             iter != getItems().end(); iter++ )
        {
            if( isBound( *iter ) )
            {
//                 LG_CTX_D << " path:" << (*iter)->getDirPath();
//                 cerr << " path:" << (*iter)->getDirPath() << endl;

                ChainedViewContext* cvc = dynamic_cast<ChainedViewContext*>(GetImpl(Delegate));
                
                TARGETSTREAM
                    << " child:" << toVoid(dynamic_cast<Context*>(GetImpl(*iter)))
                    << " cc:"   << toVoid(dynamic_cast<Context*>(GetImpl((*iter)->CoveredContext)))
                    << " omc:"  << toVoid(dynamic_cast<Context*>((*iter)->getOverMountContext()));
                if( cvc )
                    TARGETSTREAM << " Delegate:" << toVoid(dynamic_cast<Context*>(cvc));
                if( dynamic_cast<SortedContext*>(GetImpl(*iter)) )
                    TARGETSTREAM << " is-sorter " << endl;
                if( dynamic_cast<FilteredContext*>(GetImpl(*iter)) )
                    TARGETSTREAM << " is-filter " << endl;
                TARGETSTREAM
                    << " url:" << (*iter)->getURL()
                    << endl;
            }
            else
            {
                TARGETSTREAM << "iteration. "
                             << " first:" << (*iter)->getDirName()
                             << " bound:" << isBound( *iter )
                             << endl;
            }
        }
        TARGETSTREAM << endl;
        TARGETSTREAM << "ChainedViewContext::dumpOutItems( END )" << endl;
    }
    
    void
    ChainedViewContext::setDelegate( fh_context c )
    {
        Delegate = c;
    }
    
    fh_context
    ChainedViewContext::getDelegate()
    {
        return Delegate;
    }

    Context*
    ChainedViewContext::priv_CreateContext( Context* parent, std::string rdn )
    {
        return new ChainedViewContext( 0, rdn );
    }
    
};
