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

    $Id: FullTextContext.cpp,v 1.5 2010/09/24 21:30:50 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <FullTextContext.hh>
#include <Resolver_private.hh>
#include <Ferris/FullTextQuery.hh>
#include <Ferris/Ferris_private.hh>

using namespace std;
using namespace Ferris::FullTextIndex;


namespace Ferris
{

    void
    FullTextQueryRootContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
    }
    
    void
    FullTextQueryRootContext::priv_read()
    {
        staticDirContentsRAII _raii1( this );

        if( empty() )
        {
            // We have to setup the query engines.
            fh_context child = 0;

            LG_IDX_D << "FullTextQueryRootContext::priv_read() setting up root" << endl;

            child = new FullTextQueryContext_Ranked( this, "ranked" );
            Insert( GetImpl(child), false, false );

            child = new FullTextQueryContext_Boolean( this, "boolean" );
            Insert( GetImpl(child), false, false );

//             LG_IDX_D << "priv_read() child ppath:" << child->getParent()->getDirPath() << endl;
//             LG_IDX_D << "priv_read() child paddr:" << toVoid(child->getParent()) << endl;
//             LG_IDX_D << "priv_read() child u:" << child->getURL() << endl;
//             LG_IDX_D << "priv_read() child p:" << child->getDirPath() << endl;
//             LG_IDX_D << "priv_read() child r:" << child->getDirName() << endl;

//             LG_IDX_D << "priv_read() setup root complete" << endl;
//             dumpOutItems();
        }
    }
    
        

    FullTextQueryRootContext::FullTextQueryRootContext()
        :
        _Base( 0, "/" )
    {
        createStateLessAttributes();
    }
    
    FullTextQueryRootContext::~FullTextQueryRootContext()
    {
    }
    
    fh_context
    FullTextQueryRootContext::createSubContext( const std::string& rdn, fh_context md )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        fh_stringstream ss;
        ss << "fulltextquery:// directory can not have new items created in this way" << endl;
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), this );
    }
    

    void
    FullTextQueryRootContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }


    class FERRISEXP_DLLLOCAL FullTextQueryRootContext_RootContextDropper
        :
        public RootContextDropper
    {
    public:
        FullTextQueryRootContext_RootContextDropper()
            {
                ImplementationDetail::appendToStaticLinkedRootContextNames("fulltextquery");
                RootContextFactory::Register( "fulltextquery", this );
            }

        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
            {
                static fh_context c = 0;
                if( !isBound(c) )
                {
                    c = new FullTextQueryRootContext();;
                }
                return c;
            }
    };
    static FullTextQueryRootContext_RootContextDropper ___FullTextQueryRootContext_static_init;
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    
    void
    FullTextQueryRunnerContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
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
    FullTextQueryRunnerContext::priv_read()
    {
        emitExistsEventForEachItemRAII _raii1( this );
    }

    //
    // Treat the attempt to view a context as the initiation of a query
    // if the dir isn't already there then call performQuery() to make it
    // Short cut loading each dir unless absolutely needed.
    //
    fh_context
    FullTextQueryRunnerContext::priv_getSubContext( const string& rdn )
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

            fh_context qc = performQuery( rdn );
            if( !isBound( qc ) )
            {
                fh_stringstream ss;
                ss << "NoSuchSubContext:" << rdn;
                Throw_NoSuchSubContext( tostr(ss), this );
            }

            FerrisInternal::reparentSelectionContext( this, qc, rdn );
            SelectionContext* selc = dynamic_cast<SelectionContext*>( GetImpl(qc) );
            selc->setReportedRDN( rdn );
            Insert( GetImpl( qc ) );
            
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

    FullTextQueryRunnerContext::FullTextQueryRunnerContext( Context* parent, const std::string& rdn )
        :
        _Base( parent, rdn )
    {
        createStateLessAttributes();
    }
    
    FullTextQueryRunnerContext::~FullTextQueryRunnerContext()
    {
    }
    
    fh_context
    FullTextQueryRunnerContext::createSubContext( const std::string& rdn, fh_context md  )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        string name = rdn;
        if( name.empty() )
        {
            fh_context child = md->getSubContext( "file" );
            name = getStrSubCtx( child, "name", "" );
        }
        fh_context ret = performQuery( name );
        return ret;
    }
    
    void
    FullTextQueryRunnerContext::createStateLessAttributes( bool force )
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



    FullTextQueryContext_Ranked::FullTextQueryContext_Ranked( Context* parent, const std::string& rdn )
        :
        _Base( parent, rdn )
    {
        createStateLessAttributes();
    }
    
    FullTextQueryContext_Ranked::~FullTextQueryContext_Ranked()
    {
    }
    
    void
    FullTextQueryContext_Ranked::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }
    
    fh_context
    FullTextQueryContext_Ranked::performQuery( const std::string& rdn )
    {
        fh_context ret = 0;
        LG_IDX_D << "FullTextQueryContext_Ranked::performQuery() rdn:" << rdn << endl;

        fh_ftquery q = Ferris::FullTextIndex::Factory::makeFullTextQuery( rdn, QUERYMODE_RANKED );
        if( shouldRunFullTextQueryFilesystemAsync() )
        {
            ret = q->executeAsync();
            ret->addHandlableToBeReleasedWithContext( GetImpl(q) );
        }
        else
        {
            ret = q->execute();
        }
        
        
        return ret;
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/



    FullTextQueryContext_Boolean::FullTextQueryContext_Boolean( Context* parent, const std::string& rdn )
        :
        _Base( parent, rdn )
    {
        createStateLessAttributes();
    }
    
    FullTextQueryContext_Boolean::~FullTextQueryContext_Boolean()
    {
    }
    
    void
    FullTextQueryContext_Boolean::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }
    
    fh_context
    FullTextQueryContext_Boolean::performQuery( const std::string& rdn )
    {
        fh_context ret = 0;
        LG_IDX_D << "FullTextQueryContext_Boolean::performQuery() rdn:" << rdn << endl;

        fh_ftquery q = Ferris::FullTextIndex::Factory::makeFullTextQuery( rdn, QUERYMODE_BOOLEAN );
        if( shouldRunFullTextQueryFilesystemAsync() )
        {
            ret = q->executeAsync();
            ret->addHandlableToBeReleasedWithContext( GetImpl(q) );
        }
        else
        {
            ret = q->execute();
        }
        
        return ret;
    }
    


};
