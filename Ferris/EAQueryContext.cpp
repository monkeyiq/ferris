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

    $Id: EAQueryContext.cpp,v 1.6 2010/09/24 21:30:31 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/EAQueryContext.hh>
#include <Ferris/Resolver_private.hh>
#include <Ferris/EAQuery.hh>
#include <Ferris/Ferris_private.hh>
#include <Ferris/EAIndexerMetaInterface.hh>

using namespace std;
using namespace Ferris::EAIndex;

namespace Ferris
{

    void
    EAQueryRootContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
    }
    
    void
    EAQueryRootContext::priv_read()
    {
        staticDirContentsRAII _raii1( this );

        if( empty() )
        {
            // We have to setup the query engines.
            fh_context child = 0;

            child = new FullTextQueryContext_FFilter( this, "filter", false );
            Insert( GetImpl(child), false, false );

            child = new FullTextQueryContext_FFilter( this, "filter-10", false, 10 );
            Insert( GetImpl(child), false, false );

            child = new FullTextQueryContext_FFilter( this, "filter-100", false, 100 );
            Insert( GetImpl(child), false, false );

            child = new FullTextQueryContext_FFilter( this, "filter-500", false, 500 );
            Insert( GetImpl(child), false, false );

            child = new FullTextQueryContext_FFilter( this, "filter-1000", false, 1000 );
            Insert( GetImpl(child), false, false );
            
            child = new FullTextQueryContext_FFilter( this, "filter-shortnames", true );
            Insert( GetImpl(child), false, false );

            child = new FullTextQueryContext_FFilter( this, "filter-shortnames-10", true, 10 );
            Insert( GetImpl(child), false, false );

            child = new FullTextQueryContext_FFilter( this, "filter-shortnames-100", true, 100 );
            Insert( GetImpl(child), false, false );
        }

    }
    
        
    EAQueryRootContext::EAQueryRootContext()
        :
        _Base( 0, "/" )
    {
        createStateLessAttributes();
    }
    
    EAQueryRootContext::~EAQueryRootContext()
    {
    }
    
    fh_context
    EAQueryRootContext::createSubContext( const std::string& rdn, fh_context md )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        fh_stringstream ss;
        ss << "eaquery:// directory can not have new items created in this way" << endl;
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), this );
    }
    

    void
    EAQueryRootContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }


    class FERRISEXP_DLLLOCAL EAQueryRootContext_RootContextDropper
        :
        public RootContextDropper
    {
    public:
        EAQueryRootContext_RootContextDropper()
            {
                ImplementationDetail::appendToStaticLinkedRootContextNames("eaquery");
                RootContextFactory::Register( "eaquery", this );
            }

        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
            {
                static fh_context c = 0;
                if( !isBound(c) )
                {
                    c = new EAQueryRootContext();;
                }
                return c;
            }
    };
    static EAQueryRootContext_RootContextDropper ___EAQueryRootContext_static_init;
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

//     class FERRISEXP_API ChainedViewContext_PathOverride
//         :
//         public ChainedViewContext
//     {
//         typedef Context _BaseNoDelegate;
//         typedef ChainedViewContext _Base;
//         typedef ChainedViewContext_PathOverride _Self;

//     protected:

//     public:

//         ChainedViewContext_PathOverride( const fh_context& parent,
//                                          const fh_context& delegate,
//                                          const std::string& rdn,
//                                          bool setupEventConnections = true )
//             :
//             _Base( parent, delegate, rdn, setupEventConnections )
//             {
//             }
//         virtual ~ChainedViewContext_PathOverride()
//             {
//             }


//         std::string
//         private_getStrAttr( const std::string& rdn,
//                             const std::string& def,
//                             bool getAllLines,
//                             bool throwEx )
//             {
//                 cerr << "CVC_PO private_getStrAttr() rdn:" << rdn << endl;
                
//                 if( rdn == "name" || rdn == "path" || rdn == "url" )
//                     return _BaseNoDelegate::private_getStrAttr( rdn, def, getAllLines, throwEx );
//                 return Delegate->private_getStrAttr( rdn, def, getAllLines, throwEx );
//             }
        

//         virtual std::string getDirName() const
//             {
//                 return Attribute::getDirName();
//             }
        
//         virtual std::string getDirPath() throw (FerrisParentNotSetError)
//             {
//                 return Context::getDirPath();
//             }
        
//         virtual std::string getURL()
//             {
//                 cerr << "ChainedViewContext_PathOverride::getURL()" << endl;
//                 fh_stringstream ss;
//                 ss << getURLScheme() << "://" << getDirPath();
//                 return tostr(ss);
//             }
//     public:
        
//         static void temp( Context* parent, fh_context c, string rdn )
//             {
//                 c->setContext( parent, rdn );
//             }
        
//      };
    
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    
    void
    EAQueryRunnerContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
        m["file"] = SubContextCreator(
            SL_SubCreate_file,
            "	<elementType name=\"file\">\n"
            "		<elementType name=\"name\" default=\"new file\">\n"
            "			<dataTypeRef name=\"string\"/>\n"
            "		</elementType>\n"
            "	</elementType>\n");
    }
    
    void
    EAQueryRunnerContext::priv_read()
    {
        emitExistsEventForEachItemRAII _raii1( this );
    }

    //
    // Treat the attempt to view a context as the initiation of a query
    // if the dir isn't already there then call performQuery() to make it
    // Short cut loading each dir unless absolutely needed.
    //
    fh_context
    EAQueryRunnerContext::priv_getSubContext( const string& rdn )
        throw( NoSuchSubContext )
    {
        try
        {
            if( priv_isSubContextBound( rdn ) )
            {
                return _Base::priv_getSubContext( rdn );
            }

            if( rdn.empty() )
            {
                fh_stringstream ss;
                ss << "NoSuchSubContext no rdn given";
                Throw_NoSuchSubContext( tostr(ss), this );
            }

//            cerr << "runner.priv_getsubctx()" << endl;
            fh_context qc = performQuery( rdn );
            if( !isBound( qc ) )
            {
                fh_stringstream ss;
                ss << "NoSuchSubContext:" << rdn;
                Throw_NoSuchSubContext( tostr(ss), this );
            }
//            cerr << "runner.priv_getsubctx() rdn:" << rdn << " qc->rdn:" << qc->getDirName() << endl;

            FerrisInternal::reparentSelectionContext( this, qc, rdn );
            SelectionContext* selc = dynamic_cast<SelectionContext*>( GetImpl(qc) );
            selc->setReportedRDN( rdn );
            Insert( GetImpl( qc ) );
            
//             fh_context cc = new ChainedViewContext_PathOverride( this, qc, rdn );
//             cerr << "runner.priv_getsubctx() cc rdn:" << cc->getDirName() << " url:" << cc->getURL() << endl;
//             Insert( GetImpl( cc ) );
//             cerr << "runner.priv_getsubctx() after insert()" << endl;
//             dumpOutItems();
            
            return qc;
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

    EAQueryRunnerContext::EAQueryRunnerContext( Context* parent, const std::string& rdn )
        :
        _Base( parent, rdn )
    {
        createStateLessAttributes();
    }
    
    EAQueryRunnerContext::~EAQueryRunnerContext()
    {
    }
    
    fh_context
    EAQueryRunnerContext::createSubContext( const std::string& rdn, fh_context md  )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        string name = rdn;
        if( name.empty() )
        {
            fh_context child = md->getSubContext( "file" );
            name = getStrSubCtx( child, "name", "" );
        }
//        cerr << "runner.createsubctx()" << endl;
        fh_context ret = performQuery( name );

        FerrisInternal::reparentSelectionContext( this, ret, rdn );
        SelectionContext* selc = dynamic_cast<SelectionContext*>( GetImpl(ret) );
        selc->setReportedRDN( name );
        Insert( GetImpl( ret ) );
        return ret;
        
//          fh_context cc = new ChainedViewContext_PathOverride( this, ret, rdn );
//         Insert( GetImpl( cc ) );
//         return cc;
    }
    
    void
    EAQueryRunnerContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/



    FullTextQueryContext_FFilter::FullTextQueryContext_FFilter(
        Context* parent, const std::string& rdn,
        bool useShortedNames,
        int limit )
        :
        _Base( parent, rdn ),
        m_useShortedNames( useShortedNames ),
        m_limit( limit )
    {
        createStateLessAttributes();
    }
    
    FullTextQueryContext_FFilter::~FullTextQueryContext_FFilter()
    {
    }
    
    void
    FullTextQueryContext_FFilter::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }

    
    fh_context
    FullTextQueryContext_FFilter::performQuery( const std::string& rdn )
    {
        fh_context ret = 0;
        LG_EAIDX_D << "EAQueryContext_FFilter::performQuery() rdn:" << rdn
                   << " m_useShortedNames:" << m_useShortedNames
                   << " m_limit:" << m_limit
                   << endl;

        if( m_useShortedNames )
        {
            Ferris::EAIndex::fh_idx idx = Ferris::EAIndex::Factory::getDefaultEAIndex();
            fh_context selfactory = Resolve( "selectionfactory://" );
            fh_context selection  = selfactory->createSubContext( "" );

            if( SelectionContext* sc = dynamic_cast<SelectionContext*>( GetImpl(selection ) ) )
            {
                sc->setSelectionContextRDNConflictResolver(
                    get_SelectionContextRDNConflictResolver_MonsterName() );
            }

            if( shouldRunEAQueryFilesystemAsync() )
            {
                ret = EAIndex::ExecuteQueryAsync( rdn, idx, selection, m_limit );
            }
            else
            {
                ret = EAIndex::ExecuteQuery( rdn, idx, selection, m_limit );
            }
        }
        else
        {
            if( shouldRunEAQueryFilesystemAsync() )
            {
                Ferris::EAIndex::fh_idx idx = Ferris::EAIndex::Factory::getDefaultEAIndex();
                ret = EAIndex::ExecuteQueryAsync( rdn, m_limit, idx );
            }
            else
            {
                ret = EAIndex::ExecuteQuery( rdn, m_limit );
            }
        }
        
        return ret;
    }

    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

};
