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

    $Id: RemembranceContext.cpp,v 1.7 2010/09/24 21:30:57 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "Ferris.hh"
#include "FerrisSemantic.hh"
#include "Resolver_private.hh"
#include "Context_private.hh"

using namespace std;

namespace Ferris
{
    using namespace RDFCore;
    using namespace Semantic;

    static const int MAX_NumberOfFilesInView = 30;
    
    /************************************************************/
    /************************************************************/
    /************************************************************/


    /**
     * root context for remembrance://
     */
    class FERRISEXP_DLLLOCAL RemembranceRootContext
        :
        public StateLessEAHolder< RemembranceRootContext, FakeInternalContext >
    {
        typedef RemembranceRootContext _Self;
        typedef StateLessEAHolder< RemembranceRootContext, FakeInternalContext > _Base;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
            }
        

    protected:

        virtual void priv_read();
        
    public:

        RemembranceRootContext()
            :
            _Base( 0, "/" )
            {
                createStateLessAttributes();
            }
    
        
        virtual ~RemembranceRootContext()
            {
            }
        
        virtual fh_context
        createSubContext( const std::string& rdn, fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
            {
                fh_stringstream ss;
                ss << "remembrance:// directory can not have new items created in this way" << endl;
                Throw_FerrisCreateSubContextNotSupported( tostr(ss), this );
            }

        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
            
    };

    class FERRISEXP_DLLLOCAL RemembranceTopLevelContext;
    FERRIS_CTX_SMARTPTR( RemembranceTopLevelContext, fh_RemembranceTopLevelContext );
    class FERRISEXP_DLLLOCAL RemembranceTopLevelContext
        :
        public StateLessEAHolder< RemembranceTopLevelContext, FakeInternalContext >
    {
        typedef StateLessEAHolder< RemembranceTopLevelContext, FakeInternalContext > _Base;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
            }

        fh_node m_historyNode;
        fh_node m_mostRecentTimeNode;
        fh_node m_originalCmdNodePred;

        fh_fcontext
        getDesiredParent( fh_context _c, const std::string& rdn )
            {
                FakeInternalContext* c = 0;
                c = priv_ensureSubContext( rdn, c );
                return c;
//                 if( isSubContextBound( rdn ) )
//                     return dynamic_cast<FakeInternalContext*>(GetImpl(getSubContext( rdn )));

//                 fh_fcontext child = new FakeInternalContext( this, rdn );
//                 addNewChild( child );
//                 return child;
            }
        

        fh_fcontext
        getDesiredParent( fh_context c )
            {
                string mt = getStrAttr( c, "mimetype", "" );
                
                if( starts_with( mt, "audio" ))
                    return getDesiredParent( c, "audio" );
                if( starts_with( mt, "text" ))
                    return getDesiredParent( c, "text" );
                if( starts_with( mt, "video" ))
                    return getDesiredParent( c, "video" );
                
                return this;
            }
        
        
    protected:

        virtual void priv_read();

    public:

        RemembranceTopLevelContext( Context* parent, const std::string& rdn,
                                    fh_node m_historyNode,
                                    fh_node m_mostRecentTimeNode,
                                    fh_node m_originalCmdNodePred )
            :
            _Base( parent, rdn ),
            m_historyNode( m_historyNode ),
            m_mostRecentTimeNode( m_mostRecentTimeNode ),
            m_originalCmdNodePred( m_originalCmdNodePred )
            {
                createStateLessAttributes();
            }
        
        virtual ~RemembranceTopLevelContext()
            {
            }


        virtual fh_context
        createSubContext( const std::string& rdn, fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
            {
                fh_stringstream ss;
                ss << "remembrance:// directory can not have new items created in this way" << endl;
                Throw_FerrisCreateSubContextNotSupported( tostr(ss), this );
            }

        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
    };
    
    class FERRISEXP_DLLLOCAL RemembranceContext;
    FERRIS_SMARTPTR( RemembranceContext, fh_RemembranceContext );
    class FERRISEXP_DLLLOCAL RemembranceContext
        :
        public StateLessEAHolder< RemembranceContext, FakeInternalContext >
    {
        typedef RemembranceContext _Self;
        typedef StateLessEAHolder< RemembranceContext, FakeInternalContext > _Base;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
            }

        fh_node m_urlnode;
        fh_node m_anode;
        fh_node m_originalCmdNodePred;
        time_t  m_tt;

        string removeURLs( const std::string& s );
        
    protected:

        virtual void priv_read();

    public:

        void constructObject( time_t tt,
                              fh_node m_urlnode,
                              fh_node m_anode,
                              fh_node m_originalCmdNodePred )
            {
                m_urlnode = m_urlnode ;
                m_anode = m_anode ;
                m_originalCmdNodePred = m_originalCmdNodePred ;
                m_tt = tt ;
            }
        

        RemembranceContext( Context* parent, const std::string& rdn,
                            time_t tt = 0,
                            fh_node m_urlnode = 0,
                            fh_node m_anode = 0,
                            fh_node m_originalCmdNodePred = 0 )
            :
            _Base( parent, rdn ),
            m_urlnode( m_urlnode ),
            m_anode( m_anode ),
            m_originalCmdNodePred( m_originalCmdNodePred ),
            m_tt( tt )
            {
                createStateLessAttributes();
            }
        
        virtual ~RemembranceContext()
            {
            }


        virtual fh_context
        createSubContext( const std::string& rdn, fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
            {
                fh_stringstream ss;
                ss << "remembrance:// directory can not have new items created in this way" << endl;
                Throw_FerrisCreateSubContextNotSupported( tostr(ss), this );
            }

        static fh_stringstream
        SL_getTT( RemembranceContext* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ret;
                ret << c->m_tt;
                return ret;
            }

        virtual std::string getRecommendedEA()
            {
                return "name,file-view-time-display";
            }
        
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
#define SLEA tryAddStateLessAttribute         
                    SLEA( "file-view-time", &_Self::SL_getTT, FXD_UNIXEPOCH_T );
#undef SLEA
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
    };
    

    
    /************************************************************/
    /************************************************************/
    /************************************************************/

    void
    RemembranceRootContext::priv_read()
    {
        staticDirContentsRAII _raii1( this );

        if( empty() )
        {
            // We have to setup the children the first time..
            fh_context child = 0;

            fh_fcontext hc = new FakeInternalContext( this, "history" );
            Insert( GetImpl(hc), false );
            {
                fh_RemembranceTopLevelContext child;

                string pfx = Semantic::getAttrPrefix();
//                string pfx = "";
                {
                    fh_node historyNode = RDFCore::Node::CreateURI( pfx + "ferris-file-view-history" );
                    fh_node mostRecentTimeNode = RDFCore::Node::CreateURI( pfx + "most-recent-view-time" );
                    fh_node originalCmdNodePred = RDFCore::Node::CreateURI( pfx + "view-command" );

                    child = new RemembranceTopLevelContext( GetImpl(hc), "view",
                                                            historyNode,
                                                            mostRecentTimeNode,
                                                            originalCmdNodePred );
                    hc->addNewChild( child );
                }
                {
                    fh_node historyNode = RDFCore::Node::CreateURI( pfx + "ferris-file-edit-history" );
                    fh_node mostRecentTimeNode = RDFCore::Node::CreateURI( pfx + "most-recent-edit-time" );
                    fh_node originalCmdNodePred = RDFCore::Node::CreateURI( pfx + "edit-command" );

                    child = new RemembranceTopLevelContext( GetImpl(hc), "edit",
                                                            historyNode,
                                                            mostRecentTimeNode,
                                                            originalCmdNodePred );
                    hc->addNewChild( child );
                }
                
            }
            
        }
    }

    void
    RemembranceTopLevelContext::priv_read()
    {
        EnsureStartStopReadingIsFiredRAII _raii1( this );

        LG_CTX_D << "RemembranceTopLevelContext::priv_read()" << endl;
        clearContext();
//        if( empty() )
        {
            typedef map< time_t, pair< fh_node, fh_node > > mrtime_to_node_map_t;
            mrtime_to_node_map_t mrtime_to_node_map;

            fh_model m = RDFCore::getDefaultFerrisModel();
            fh_statement query_statement = new Statement();
            query_statement->setPredicate( m_historyNode );
            StatementIterator iter = m->findStatements( query_statement );

            for( StatementIterator end; iter != end; ++iter )
            {
                string earl = (*iter)->getSubject()->toString();
                LG_CTX_D << "file:" << earl << endl;

                fh_node anode = (*iter)->getObject();
                fh_node mrtime = m->getObject( anode, m_mostRecentTimeNode );
                if( mrtime )
                {
                    LG_CTX_D << "     mrtime:" << mrtime->toString() << endl;
                    time_t tt = toType<time_t>( mrtime->toString() );
                    mrtime_to_node_map[ tt ] = make_pair( (*iter)->getSubject(), anode );
                }
            }
            
            LG_CTX_D << "----------------------------------------" << endl;
            int i = 0;
            for( mrtime_to_node_map_t::reverse_iterator mi = mrtime_to_node_map.rbegin();
                 mi != mrtime_to_node_map.rend() && i < MAX_NumberOfFilesInView ; ++mi, ++i )
            {
                string earl = "";
                time_t tt = mi->first;
                fh_node uuidnode = mi->second.first;
                fh_node n = m->getSubject( Semantic::uuidPredNode(), uuidnode );
                if( !n )
                    continue;
                LG_CTX_D << "n:" << n->toString() << endl;
                
                if( !n->getURI() )
                    continue;
                
                earl = n->getURI()->toString();
                LG_CTX_D << "mrtime:" << mi->first << " url:" << earl << endl;
                
                try
                {
                    fh_context ctx = Resolve( earl );
                    string rdn = ctx->getDirName();

                    fh_fcontext parent = getDesiredParent( ctx );

                    RemembranceContext* c = 0;
                    c = parent->priv_ensureSubContext( rdn, c );
                    c->constructObject( tt, n, mi->second.second,
                                        m_originalCmdNodePred );
//                     fh_RemembranceContext child;
//                     child = new RemembranceContext( GetImpl(parent), rdn, tt,
//                                                     n, mi->second.second,
//                                                     m_originalCmdNodePred );
//                     parent->addNewChild( child );
                }
                catch( exception& e )
                {
                    LG_CTX_W << "warning:" << e.what() << endl;
                }
                catch( ... )
                {
                }
            }
        }
    }

    string
    RemembranceContext::removeURLs( const std::string& s )
    {
        stringstream oss;
        stringstream iss;
        iss << s;
        char ch;

        while( iss >> noskipws >> ch )
        {
            if( ch == '"' )
                break;
            oss << ch;
        }

        return oss.str();
    }
    
    void
    RemembranceContext::priv_read()
    {
        staticDirContentsRAII _raii1( this );
        if( !m_urlnode )
            return;
        
        if( empty() )
        {
            LG_CTX_D << "RemembranceContext::priv_read()" << endl;

            fh_context c = Resolve( m_urlnode->toString() );
            if( c->isParentBound() )
            {
                fh_context child = 0;

                string selfrdn = "self." + getStrAttr( c, "name-extension", "" );
                child = new VirtualSoftlinkContext( this, c, selfrdn );
                addNewChild( child );

                string dirrdn = "parent";
                child = new VirtualSoftlinkContext( this, c->getParent(), dirrdn );
                addNewChild( child );

                fh_model m = RDFCore::getDefaultFerrisModel();

                if( fh_node originalCmdNode = m->getObject( m_anode, m_originalCmdNodePred ) )
                {
                    string originalcmd = originalCmdNode->toString();
                    
                    stringstream redocontentss;
                    redocontentss << "#!/bin/sh" << endl
                                  << "echo " << removeURLs( originalcmd )
                                  << " \"" << c->getDirPath() << "\" " << " | "
                                  << "ferris-internal-remembrance-redo-script.sh" << endl
                                  << endl;
                    string redordn = "redo.fsh";
                    child = new StaticContentLeafContext( this, redordn, redocontentss.str() );
                    addNewChild( child );
                    
                    {
                        stringstream redocontentss;
                        redocontentss << "#!/bin/sh" << endl
                                      << "echo " << originalcmd << " | "
                                      << "ferris-internal-remembrance-redo-batch-script.sh" << endl
                                      << endl;
                        string redordn = "redo-batch.fsh";
                        child = new StaticContentLeafContext( this, redordn, redocontentss.str() );
                        addNewChild( child );
                    }
                }
            }
        }
    }
    
    
    /************************************************************/
    /************************************************************/
    /************************************************************/
    
    class FERRISEXP_DLLLOCAL RemembranceRootContext_RootContextDropper
        :
        public RootContextDropper
    {
    public:
        RemembranceRootContext_RootContextDropper()
            {
                ImplementationDetail::appendToStaticLinkedRootContextNames("remembrance");
                RootContextFactory::Register( "remembrance", this );
            }

        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
            {
                static fh_context c = 0;
                if( !isBound(c) )
                {
                    c = new RemembranceRootContext();
                }
                return c;
            }
    };
    static RemembranceRootContext_RootContextDropper ___RemembranceRootContext_static_init;
};
