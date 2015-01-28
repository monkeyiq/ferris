/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001-2003 Ben Martin

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

    $Id: libferrissoprano.cpp,v 1.2 2010/09/24 21:31:47 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/Context_private.hh> // VirtualSoftlinkContext
#include <Ferris/FerrisSemantic.hh>
#include <Ferris/DublinCore.hh>
#include <Resolver_private.hh>

using namespace std;
using namespace Ferris::RDFCore;

namespace Ferris
{
    typedef std::list< fh_node > nodelist_t;
    
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };


    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    const std::string RDF_FERRIS_BASE = "http://witme.sf.net/libferris-core/0.1";

    /**
     * This is the base class for an RDF repository. For example, we handle
     * the "x.rdf" file itself and create RedlandStatementContext objects
     * for the first tear of triples that we are presenting. Each
     * RedlandStatementContext that we create handles drilling down into the
     * triples further.
     */
    class FERRISEXP_CTXPLUGIN RedlandRootContext
        :
        public FakeInternalContext
    {
        typedef RedlandRootContext    _Self;
        typedef FakeInternalContext   _Base;
        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );

        fh_model m_model;
        fh_node  m_baseSubject;
        
    protected:

        /**
         * some testing to see if this is context has children.
         * The default impl checks the mimetype and then sees if begin()==end()
         */
        virtual bool isDir()
            {
                return true;
            }
        
        
        virtual ferris_ios::openmode getSupportedOpenModes()
            {
                return ios::in | ios::out | ios::binary | ios::ate | ios::trunc;
            }
        
        static fh_context my_SubCreate_file( fh_context c, fh_context md )
            {
                if( RedlandRootContext* bc = dynamic_cast<RedlandRootContext*>( GetImpl(c) ))
                {
                    return bc->SubCreate_file( c, md );
                }
                Throw_FerrisCreateSubContextNotSupported("", GetImpl(c) );
            }

        static fh_context my_SubCreate_ea( fh_context c, fh_context md )
            {
                if( RedlandRootContext* bc = dynamic_cast<RedlandRootContext*>( GetImpl(c) ))
                {
                    return bc->SubCreate_ea( c, md );
                }
                Throw_FerrisCreateSubContextNotSupported("", GetImpl(c) );
            }

        static fh_context my_SubCreate_QV( fh_context c, fh_context md )
            {
                if( RedlandRootContext* bc = dynamic_cast<RedlandRootContext*>( GetImpl(c) ))
                {
                    return bc->SubCreate_QV( c, md );
                }
                Throw_FerrisCreateSubContextNotSupported("", GetImpl(c) );
            }
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
                m["file"] = SubContextCreator( _Self::my_SubCreate_file,
                                               "	<elementType name=\"file\">\n"
                                               "		<elementType name=\"name\" default=\"http://mysite.com/newpred\">\n"
                                               "			<dataTypeRef name=\"string\"/>\n"
                                               "		</elementType>\n"
                                               "		<elementType name=\"object\" default=\"http://mysite.com/newobj\">\n"
                                               "			<dataTypeRef name=\"string\"/>\n"
                                               "		</elementType>\n"
                                               "	</elementType>\n");
                m["dir"] = SubContextCreator( _Self::my_SubCreate_file,
                                               "	<elementType name=\"dir\">\n"
                                               "		<elementType name=\"name\" default=\"http://mysite.com/newpred\">\n"
                                               "			<dataTypeRef name=\"string\"/>\n"
                                               "		</elementType>\n"
                                               "		<elementType name=\"object\" default=\"http://mysite.com/newobj\">\n"
                                               "			<dataTypeRef name=\"string\"/>\n"
                                               "		</elementType>\n"
                                               "	</elementType>\n");
                m["ea"] = SubContextCreator( _Self::my_SubCreate_ea,
                                               "	<elementType name=\"ea\">\n"
                                               "		<elementType name=\"name\" default=\"http://mysite.com/newpred\">\n"
                                               "			<dataTypeRef name=\"string\"/>\n"
                                               "		</elementType>\n"
                                               "		<elementType name=\"object\" default=\"new literal\">\n"
                                               "			<dataTypeRef name=\"string\"/>\n"
                                               "		</elementType>\n"
                                               "	</elementType>\n");

                m["queryview"] = SubContextCreator( _Self::my_SubCreate_QV,
/**/                "	<elementType name=\"queryview\">\n"
/**/                "		<elementType name=\"name\" default=\"splinter-triples\">\n"
/**/                "			<dataTypeRef name=\"string\"/>\n"
/**/                "		</elementType>\n"
/**/                "		<elementType name=\"subject\" >\n"
/**/                "			<dataTypeRef name=\"string\"/>\n"
/**/                "		</elementType>\n"
// /**/                "		<elementType name=\"predicate\" >\n"
// /**/                "			<dataTypeRef name=\"string\"/>\n"
// /**/                "		</elementType>\n"
// /**/                "		<elementType name=\"object\" >\n"
// /**/                "			<dataTypeRef name=\"string\"/>\n"
// /**/                "		</elementType>\n"
/**/                "	</elementType>\n");
                
            }
        
        virtual fh_context SubCreate_file( fh_context c, fh_context md );
        virtual fh_context SubCreate_ea( fh_context c, fh_context md );
        virtual fh_context SubCreate_QV( fh_context c, fh_context md );

        virtual bool supportsRemove()
            {
                return true;
            }
    
        virtual void priv_remove( fh_context c_ctx );
        

        virtual RedlandRootContext* priv_CreateContext( Context* parent, std::string rdn )
            {
                RedlandRootContext* ret = new RedlandRootContext( parent, rdn );
                return ret;
            }


        //
        // Used in read() and create() to add the predicate as a subdir and object as a subsubdir
        // returns the object context
        //
        fh_context addStatement( fh_statement st );
        
        std::string  priv_read_GetFirstRDFAboutAttribute();
        virtual bool priv_read_TryRootSubject( const std::string& uri, bool bindEvenIfNoResults = false );
        virtual void priv_read();
//        virtual void priv_createAttributes();

        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
//                 LG_RDF_D << "+++ rdf::priv_getIStream URL:" << getURL() << endl;
//                 BackTrace();
                return getCoveredContext()->priv_getIStream( m );
            }
        
        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                return getCoveredContext()->priv_getIOStream( m );
            }
        
    public:

        RedlandRootContext( Context*            parent,
                            const std::string&  rdn,
                            fh_model      model = 0,
                            fh_node       basesubj = 0 ); 
        virtual ~RedlandRootContext();

        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }

        void
        addNewAttribute( const std::string& predstr, XSDBasic_t sct = XSD_UNKNOWN )
            {
                {
                    fh_stringstream tmp = getRDFAttributeStream( this, predstr, 0 );
                    LG_RDF_D << "addNewAttribute() this:" << getURL()
                         << " pred:" << predstr
                         << " value:" << tostr(tmp)
                         << endl;
                }
                addAttribute( predstr,
                              this, &_Self::getRDFAttributeStream,
                              this, &_Self::getRDFAttributeStream,
                              this, &_Self::OnRDFAttributeStreamClosed,
                              sct );
            }

        std::pair< fh_node, fh_node > getSubjPred( Context* c, const std::string& rdn )
            {
                if( !getBaseSubject() )
                    read();

                string predstr = rdn;
                fh_node subj = getBaseSubject();
                fh_node pred = Node::CreateURI( predstr );

                return make_pair( subj, pred );
            }

        nodelist_t getObjects( const std::string& predstr )
            {
                nodelist_t ret;

                if( !getBaseSubject() )
                    read();
                
                fh_node subj = getBaseSubject();
                fh_node pred = Node::CreateURI( predstr );

                NodeIterator iter = getModel()->findObjects( subj, pred );
                copy( iter, NodeIterator(), back_inserter( ret ));
                
                return ret;
            }
        
        fh_stringstream
        getRDFAttributeStream( Context* c,
                               const std::string& rdn,
                               EA_Atom* atom )
            {
                fh_stringstream ret;

                LG_RDF_D << "getRDFAttributeStream() rdn:" << rdn << endl;
                
                if( RedlandRootContext* selfp = dynamic_cast<RedlandRootContext*>(c))
                {
                    nodelist_t nl = selfp->getObjects( rdn );
                    LG_RDF_D << "getRDFAttributeStream(2) rdn:" << rdn
                         << " nl.size:" << nl.size()
                         << endl;
                    
                    if( !nl.empty() )
                    {
                        fh_node obj = nl.front();

                        LG_RDF_D << "getRDFAttributeStream(3) rdn:" << rdn
                             << " obj:" << obj->toString()
                             << endl;

                        if( obj->isLiteral() )
                        {
                            ret << obj->getLiteralValue();
                        }
                    }
                }
                
                return ret;
            }

        void
        OnRDFAttributeStreamClosed( Context* c,
                                    const std::string& rdn,
                                    EA_Atom* atom,
                                    fh_istream ss )
            {
                string k = rdn;

                if( RedlandRootContext* selfp = dynamic_cast<RedlandRootContext*>(c))
                {
                    try
                    {
                        nodelist_t nl = selfp->getObjects( rdn );
                        if( !nl.empty() )
                        {
                            fh_node obj = nl.front();
                            if( obj->isLiteral() )
                            {
                                fh_node newobj = Node::CreateLiteral( StreamToString( ss ) );
                                
                                pair< fh_node, fh_node > p = getSubjPred( c, rdn );
                                fh_model m = selfp->getModel();
                                
                                m->erase(  p.first, p.second, obj );
                                m->insert( p.first, p.second, newobj );
                            }
                        }
                    }
                    catch( exception& e )
                    {
                        fh_stringstream ss;
                        ss << "Update of EA failed for:" << c->getURL()
                           << " reason:" << e.what() << endl;
                        Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
                    }
                }
            }
        
        /********************/
        /********************/
        /********************/

        fh_model getModel()
            {
                return m_model;
            }
        
        fh_node getBaseSubject()
            {
                if( !m_baseSubject )
                    read();
                
                return m_baseSubject;
            }
    };

    /**
     * This context represents a single triple in the RDF store.
     * Arcs that have a literal as an object become EA, arcs to
     * uri's become subdirectories.
     */
    class FERRISEXP_CTXPLUGIN RedlandStatementContext
        :
        public ParentPointingTreeContext< RedlandStatementContext, FakeInternalContext >
    {
        typedef RedlandStatementContext  _Self;
        typedef ParentPointingTreeContext< RedlandStatementContext,
                                           FakeInternalContext >      _Base;

        fh_statement m_st;
        
    protected:

        /**
         * When we are a predicate context, we have no m_st of our own because
         * we are not a complete triple. This method gets the object of our
         * parent (handling if the parent is a rootContext).
         * Thus
         *   subj = getParentsObject
         *   pred = Node( getDirName() )
         *    obj will range over the begin() to end() of this context.
         */
        fh_node getParentsObject();
        

        RedlandStatementContext* getParentWithObject( fh_node n );
        virtual void priv_read();
        virtual void priv_createAttributes();

        virtual RedlandStatementContext* priv_CreateContext( Context* parent, std::string rdn )
            {
                RedlandStatementContext* ret = new RedlandStatementContext( parent, rdn );
                return ret;
            }

        RedlandStatementContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_st( 0 )
            {
            }

        void
        addNewAttribute( const std::string& predstr, XSDBasic_t sct = XSD_UNKNOWN )
            {
                {
                    fh_stringstream tmp = getRDFAttributeStream( this, predstr, 0 );
                    LG_RDF_D << "addNewAttribute() this:" << getURL()
                         << " pred:" << predstr
                         << " value:" << tostr(tmp)
                         << endl;
                }
                
                addAttribute( predstr,
                              this, &_Self::getRDFAttributeStream,
                              this, &_Self::getRDFAttributeStream,
                              this, &_Self::OnRDFAttributeStreamClosed,
                              sct );
            }

        static fh_context my_SubCreate_file( fh_context c, fh_context md )
            {
                if( RedlandStatementContext* bc = dynamic_cast<RedlandStatementContext*>( GetImpl(c) ))
                {
                    return bc->SubCreate_file( c, md );
                }
                Throw_FerrisCreateSubContextNotSupported("", GetImpl(c) );
            }

        static fh_context my_SubCreate_ea( fh_context c, fh_context md )
            {
                if( RedlandStatementContext* bc = dynamic_cast<RedlandStatementContext*>( GetImpl(c) ))
                {
                    return bc->SubCreate_ea( c, md );
                }
                Throw_FerrisCreateSubContextNotSupported("", GetImpl(c) );
            }

        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
                if( m_st )
                {
                    m["file"] = SubContextCreator( _Self::my_SubCreate_file,
                                                   "	<elementType name=\"file\">\n"
                                                   "		<elementType name=\"name\" default=\"http://mysite.com/newpred\">\n"
                                                   "			<dataTypeRef name=\"string\"/>\n"
                                                   "		</elementType>\n"
                                                   "		<elementType name=\"object\" default=\"http://mysite.com/newobj\">\n"
                                                   "			<dataTypeRef name=\"string\"/>\n"
                                                   "		</elementType>\n"
                                                   "	</elementType>\n");
                    m["dir"] = SubContextCreator( _Self::my_SubCreate_file,
                                                  "	<elementType name=\"dir\">\n"
                                                  "		<elementType name=\"name\" default=\"http://mysite.com/newpred\">\n"
                                                  "			<dataTypeRef name=\"string\"/>\n"
                                                  "		</elementType>\n"
                                                  "		<elementType name=\"object\" default=\"http://mysite.com/newobj\">\n"
                                                  "			<dataTypeRef name=\"string\"/>\n"
                                                  "		</elementType>\n"
                                                  "	</elementType>\n");
                    m["ea"] = SubContextCreator( _Self::my_SubCreate_ea,
                                                 "	<elementType name=\"ea\">\n"
                                                 "		<elementType name=\"name\" default=\"http://mysite.com/newpred\">\n"
                                                 "			<dataTypeRef name=\"string\"/>\n"
                                                 "		</elementType>\n"
                                                 "		<elementType name=\"object\" default=\"new literal\">\n"
                                                 "			<dataTypeRef name=\"string\"/>\n"
                                                 "		</elementType>\n"
                                                 "	</elementType>\n");
                }
                else
                {
                    m["file"] = SubContextCreator( _Self::my_SubCreate_file,
                                                   "	<elementType name=\"file\">\n"
                                                   "		<elementType name=\"name\" default=\"http://mysite.com/newpred\">\n"
                                                   "			<dataTypeRef name=\"string\"/>\n"
                                                   "		</elementType>\n"
                                                   "	</elementType>\n");
                    m["dir"] = SubContextCreator( _Self::my_SubCreate_file,
                                                  "	<elementType name=\"dir\">\n"
                                                  "		<elementType name=\"name\" default=\"http://mysite.com/newpred\">\n"
                                                  "			<dataTypeRef name=\"string\"/>\n"
                                                  "		</elementType>\n"
                                                  "	</elementType>\n");
                }
            }

        virtual fh_context SubCreate_file( fh_context c, fh_context md );
        virtual fh_context SubCreate_ea( fh_context c, fh_context md );

        virtual bool supportsRemove()
            {
                return true;
            }
    
        virtual void priv_remove( fh_context c_ctx );

        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ss;
                string form = "/ntriples";
                ss << getStrAttr( this, RDF_FERRIS_BASE + form, "", true, true );
                return ss;
            }

        fh_stringstream
        getXFormatStream( Context* c,
                          const std::string& rdn,
                          EA_Atom* atom,
                          const std::string& format )
            {
                fh_stringstream ret;
                if( RedlandStatementContext* selfp = dynamic_cast<RedlandStatementContext*>(c))
                {
                    if( selfp->m_st )
                    {
                        ret << selfp->m_st->toXString( getModel(), format ) << endl;
                    }
                    else
                    {
                        fh_node subj = getParentsObject();
                        fh_node pred = Node::CreateURI( getDirName() );
                        NodeIterator iter = getModel()->findObjects( subj, pred );
                        for( ; iter != NodeIterator(); ++iter )
                        {
                            fh_node obj = *iter;
                            fh_statement st = new Statement( subj, pred, obj );
                            ret << st->toXString( getModel(), format ) << endl;
                        }
                    }
                }
                return ret;
            }

        fh_stringstream
        getNTriplesStream( Context* c,
                           const std::string& rdn,
                           EA_Atom* atom )
            {
                return getXFormatStream( c, rdn, atom, "ntriples" );
            }
        
        fh_stringstream
        getRDFXMLStream( Context* c,
                           const std::string& rdn,
                           EA_Atom* atom )
            {
                return getXFormatStream( c, rdn, atom, "rdfxml" );
            }
        
    public:

        RedlandStatementContext( Context* parent,
                                 const std::string& rdn,
                                 fh_statement st )
            :
            _Base( parent, rdn ),
            m_st( st )
            {
//                 const stringlist_t& dca = getUnqualifiedDublinCoreAttributeNames();
//                 for( stringlist_t::const_iterator si = dca.begin(); si != dca.end(); ++si )
//                 {
//                     addNewAttribute( dcURI, *si, XSD_BASIC_STRING );
//                 }
//                 addNewAttribute( dcURI, "mtime", FXD_UNIXEPOCH_T );
//                 addNewAttribute( dcURI, "atime", FXD_UNIXEPOCH_T );
//                 addNewAttribute( dcURI, "ctime", FXD_UNIXEPOCH_T );
                
                {
                    fh_stringstream ss = getSubjectStream( this, "", 0 );
                    LG_RDF_D << "uri:" << getURL() << endl
                         << "   " << RDF_FERRIS_BASE << "/subject has value:" << tostr(ss) << endl;
                }
                
                addAttribute( RDF_FERRIS_BASE + "/ntriples",
                              this, &_Self::getNTriplesStream,
                              XSD_BASIC_STRING );
                addAttribute( RDF_FERRIS_BASE + "/rdfxml",
                              this, &_Self::getRDFXMLStream,
                              XSD_BASIC_STRING );
                addAttribute( RDF_FERRIS_BASE + "/subject",
                              this, &_Self::getSubjectStream,
                              XSD_BASIC_STRING );
                addAttribute( RDF_FERRIS_BASE + "/predicate",
                              this, &_Self::getPredicateStream,
                              XSD_BASIC_STRING );
                addAttribute( RDF_FERRIS_BASE + "/object",
                              this, &_Self::getObjectStream,
                              XSD_BASIC_STRING );
            }

        //
        // Used in read() and create() to add the predicate as a subdir and object as a subsubdir
        // returns the object context
        //
        fh_context addStatement( fh_statement st );
        
        fh_stringstream
        getSubjectStream( Context* c,
                          const std::string& rdn,
                          EA_Atom* atom )
            {
                fh_stringstream ret;
                if( RedlandStatementContext* selfp = dynamic_cast<RedlandStatementContext*>(c))
                {
                    if( selfp->m_st )
                    {
                        ret << selfp->m_st->getSubject()->getURI()->toString();
                    }
                }
                return ret;
            }

        fh_stringstream
        getPredicateStream( Context* c,
                            const std::string& rdn,
                            EA_Atom* atom )
            {
                fh_stringstream ret;
                if( RedlandStatementContext* selfp = dynamic_cast<RedlandStatementContext*>(c))
                {
                    if( selfp->m_st )
                    {
                        ret << selfp->m_st->getPredicate()->getURI()->toString();
                    }
                }
                return ret;
            }

        fh_stringstream
        getObjectStream( Context* c,
                         const std::string& rdn,
                         EA_Atom* atom )
            {
                LG_RDF_D << "getObjectStream(top) c:" << c->getURL() << endl;
                
                fh_stringstream ret;
                if( RedlandStatementContext* selfp = dynamic_cast<RedlandStatementContext*>(c))
                {
                    LG_RDF_D << "getObjectStream(2) c:" << c->getURL()
                             << " statement:" << selfp->m_st
                             << endl;
                    
                    if( selfp->m_st )
                    {
                        fh_statement st  = selfp->m_st;
                        fh_node      obj = st->getObject();

                        LG_RDF_D << "getObjectStream() c:" << c->getURL()
                                 << " obj:" << isBound( obj )
                                 << endl;
                        
                        if( obj->isResource() )
                        {
                            ret << obj->getURI()->toString();
                        }
                        else
                        {
                            ret << obj->toString();
                        }
                    }
                }
                return ret;
            }
        
        

        std::pair< fh_node, fh_node > getSubjPred( Context* c, const std::string& rdn )
            {
                string predstr = rdn;

                fh_node subj = m_st->getObject();
                fh_node pred = Node::CreateURI( predstr );

                return make_pair( subj, pred );
            }
        
        nodelist_t getObjects( const std::string& predstr )
            {
                nodelist_t ret;

                if( !m_st )
                    return ret;
                
                fh_node subj = m_st->getObject();
                fh_node pred = Node::CreateURI( predstr );

                NodeIterator iter = getModel()->findObjects( subj, pred );
                copy( iter, NodeIterator(), back_inserter( ret ));
                
                return ret;
            }
        
        fh_stringstream
        getRDFAttributeStream( Context* c,
                               const std::string& rdn,
                               EA_Atom* atom )
            {
                fh_stringstream ret;

                LG_RDF_D << "getRDFAttributeStream() rdn:" << rdn << endl;
                
                if( RedlandStatementContext* selfp = dynamic_cast<RedlandStatementContext*>(c))
                {
                    nodelist_t nl = selfp->getObjects( rdn );
                    LG_RDF_D << "getRDFAttributeStream(2) rdn:" << rdn
                         << " nl.size:" << nl.size()
                         << endl;
                    
                    if( !nl.empty() )
                    {
                        fh_node obj = nl.front();

                        LG_RDF_D << "getRDFAttributeStream(3) rdn:" << rdn
                             << " obj:" << obj->toString()
                             << endl;

                        if( obj->isLiteral() )
                        {
                            ret << obj->getLiteralValue();
                        }
                    }
                }
                
                return ret;
            }

        void
        OnRDFAttributeStreamClosed( Context* c,
                                    const std::string& rdn,
                                    EA_Atom* atom,
                                    fh_istream ss )
            {
                string k = rdn;

                if( RedlandStatementContext* selfp = dynamic_cast<RedlandStatementContext*>(c))
                {
                    try
                    {
                        nodelist_t nl = selfp->getObjects( rdn );
                        if( !nl.empty() )
                        {
                            fh_node obj = nl.front();
                            if( obj->isLiteral() )
                            {
                                fh_node newobj = Node::CreateLiteral( StreamToString( ss ) );
                                
                                pair< fh_node, fh_node > p = getSubjPred( c, rdn );
                                fh_model m = selfp->getModel();
                                
                                m->erase(  p.first, p.second, obj );
                                m->insert( p.first, p.second, newobj );
                                m->sync();
                            }
                        }
                    }
                    catch( exception& e )
                    {
                        fh_stringstream ss;
                        ss << "Update of EA failed for:" << c->getURL()
                           << " reason:" << e.what() << endl;
                        Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
                    }
                }
            }
        
        
        virtual ~RedlandStatementContext()
            {
            }

        fh_statement getSatement()
            {
                return m_st;
            }

        RedlandRootContext* getRedlandRootContext()
            {
                Context* pp = getBaseContext()->getParent();
                
                RedlandRootContext* ret = dynamic_cast<RedlandRootContext*>( pp->getOverMountContext() );
                if( !ret )
                {
                    LG_XML_ER << "getRedlandRootContext() failed. should never happen!" << endl;
                    BackTrace();
                }
                return ret;
            }
        
        fh_model getModel()
            {
                return getRedlandRootContext()->getModel();
            }
        
    };
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    RedlandRootContext::RedlandRootContext( Context* parent,
                                            const std::string& rdn,
                                            fh_model model,
                                            fh_node  baseSubj )
        :
        _Base( parent, rdn ),
        m_model( model ),
        m_baseSubject( baseSubj )
    {
        LG_RDF_D << "RedlandRootContext() parent:" << toVoid( parent )
                 << " rdn:" << rdn
                 << " model:" << toVoid( model )
                 << endl;
        
        if( parent )
        {
            LG_RDF_D << "RedlandRootContext() parent:" << parent->getURL()
                     << " rdn:" << rdn
                     << endl;
        }
//         LG_RDF_D << "RedlandRootContext() rdn:" << rdn << endl;
//         BackTrace();

        createStateLessAttributes();

        if( model && m_baseSubject )
        {
            try
            {
                fh_node subj = m_baseSubject;
                NodeIterator iter = getModel()->getArcsOut( subj );
                m_baseSubject = subj;
            
                for( ; iter != NodeIterator(); ++iter )
                {
                    fh_node pred = *iter;
                    NodeIterator objiter = getModel()->findObjects( subj, pred );
                    for( ; objiter != NodeIterator(); ++objiter )
                    {
                        fh_node obj = *objiter;
                        fh_statement st = new Statement( subj, pred, obj );
                        addStatement( st );
                    }
                }
            }
            catch( exception& e )
            {}
        }
    }

    RedlandRootContext::~RedlandRootContext()
    {
        if( getModel() )
            getModel()->sync();
    }
    

    fh_context
    RedlandRootContext::SubCreate_file( fh_context c, fh_context md )
    {
        string rdn     = getStrSubCtx( md, "name", "" );
        string objuri  = getStrSubCtx( md, "object", "" );
        string predstr = rdn;

        LG_RDF_D << "RedlandRootContext::create_file() c:" << c->getURL()
                 << " rdn:" << rdn
                 << " objuri:" << objuri
                 << endl;
        
        if( !m_baseSubject )
            read();
        if( !m_baseSubject )
        {
            fh_stringstream ss;
            ss << "Attempt to create EA for RDF repository when base subject can not be determined."
               << " url:" << getURL();
            Throw_FerrisCreateSubContextFailed( tostr(ss), this );
        }
        
        fh_node subj = m_baseSubject;
        fh_node pred = Node::CreateURI( predstr );
        fh_node obj  = Node::CreateURI( objuri );

        fh_statement st = new Statement( subj, pred, obj );
        getModel()->insert( st );
        getModel()->sync();

//         Context* tmp = new RedlandStatementContext( this, predstr, st );
//         fh_context child = tmp;
//         Insert( tmp );
        fh_context child = addStatement( st );
        return child;
    }

    fh_context
    RedlandRootContext::SubCreate_ea( fh_context c, fh_context md )
    {
        string rdn      = getStrSubCtx( md, "name", "" );
        string objvalue = getStrSubCtx( md, "object", "", true );

        LG_RDF_D << "RedlandRootContext::create_ea() c:" << c->getURL()
                 << " rdn:" << rdn
                 << " objvalue:" << objvalue
                 << endl;
        
        if( objvalue.empty() )
            objvalue = getStrSubCtx( md, "value", "", true );
        string predstr  = rdn;
        bool   tryToCreateEAInParent = false;

        if( rdn == "rdf-root-subject-uri" )
        {
            tryToCreateEAInParent = true;
        }
        
        try
        {
            if( !tryToCreateEAInParent )
                if( !m_baseSubject )
                    read();
        }
        catch( exception& e )
        {}

        if( tryToCreateEAInParent )
        {
            fh_mdcontext mymd = new f_mdcontext();
            fh_mdcontext child = mymd->setChild( "ea", "" );
            child->setChild( "name",  rdn );
            child->setChild( "value", objvalue );

            LG_RDF_D << "RRC::SubCreate_ea() child name:" << child->getDirName()
                 << " child.path:" << child->getDirPath()
                 << " child.url:" << child->getURL()
                 << " rdn:" << rdn
                 << " objvalue:" << objvalue
                 << " value:" << getStrSubCtx( md, "value", "", true )
                 << endl;
//             cerr << "RRC::SubCreate_ea() md follows..." << endl;
//             mymd->dumpOutItems();
            
            LG_RDF_D << "Returning control to context to try to make the EA" << endl;
            fh_context ret = Context::SubCreate_ea( c->getCoveredContext(), child );
            return ret;
        }

        
        if( !m_baseSubject )
        {
            fh_stringstream ss;
            ss << "Attempt to create EA for RDF repository when base subject can not be determined."
               << " url:" << getURL();
            Throw_FerrisCreateAttributeFailed( tostr(ss), this );
        }
        
        fh_node subj = m_baseSubject;
        fh_node pred = Node::CreateURI( predstr );
        fh_node obj  = Node::CreateLiteral( objvalue );

        fh_statement st = new Statement( subj, pred, obj );
        getModel()->insert( st );
        getModel()->sync();

        addNewAttribute( pred->getURI()->toString(), XSD_BASIC_STRING );
        return c;
    }

    fh_context
    RedlandRootContext::SubCreate_QV( fh_context c, fh_context md )
    {
        fh_model     m = getModel();
        fh_context ret = 0;

        if( !m )
        {
            m = Model::FromMetadataContext( getCoveredContext() );
        }
        
        string rdn     = getStrSubCtx( md, "name", "" );
        string subjstr = getStrSubCtx( md, "subject", "" );
//         string predstr = getStrSubCtx( md, "predicate", "" );
//         string objestr = getStrSubCtx( md, "object", "", true );

        LG_RDF_D << "RedlandRootContext::create_qv() c:" << c->getURL()
                 << " rdn:" << rdn
                 << " subjstr:" << subjstr
                 << endl;
        
        fh_node subj = Node::CreateURI( subjstr );
//         fh_node pred = Node::CreateURI( predstr );
//         fh_node obje = Node::CreateURI( objestr );

        
//         fh_statement base = m->find( subj, pred, obje );
//         if( !m->contains( base ) )
//         {
//             fh_stringstream ss;
//             ss << "Can not create a query view because given statement is not found in repository."
//                << " subj:" << subj->toString() << endl
//                << " pred:" << pred->toString() << endl
//                << " obj:" << obje->toString() << endl
//                << " URL::" << getURL()
//                << endl;
//             Throw_FerrisCreateSubContextFailed( tostr(ss), 0 );
//         }

        try
        {
            LG_RDF_D << "SubCreate_QV() model:" << toVoid( m )
                 << " subj:" << subj->toString()
                 << endl;
            RedlandRootContext* child = new RedlandRootContext( this, rdn, m, subj );
            ret = child;

            if( dynamic_cast<Context*>( this ) != getCoveredContext() )
                ret->setCoveredContext( getCoveredContext() );
            
            addNewChild( child );
        }
        catch( exception& e )
        {
            Throw_FerrisCreateSubContextFailed( e.what(), 0 );
        }

        return ret;
    }
    
    void
    RedlandRootContext::priv_remove( fh_context c_ctx )
    {
        LG_RDF_D << "RedlandRootContext::redland_remove(top) url:" << getURL() << endl;
                
        RedlandStatementContext* c = dynamic_cast<RedlandStatementContext*>( (GetImpl(c_ctx) ) );
        if( !c )
        {
            fh_stringstream ss;
            ss << "Attempt to remove a non RDF context from within a redland file!"
               << " url:" << c_ctx->getURL();
            Throw_CanNotDelete( tostr(ss), GetImpl(c_ctx) );
        }
        string url    = c->getURL();
        string rdn    = c->getDirName();

        try
        {
            fh_node subj = getBaseSubject();
            fh_node pred = Node::CreateURI( rdn );
            NodeIterator objiter = getModel()->findObjects( subj, pred );
            getModel()->erase( subj, pred, objiter );
            getModel()->sync();
        }
        catch( exception& e )
        {
            std::ostringstream ss;
            ss << "Can not delete child:" << url
               << " reason:" << e.what() << endl;
            Throw_CanNotDelete( tostr(ss), 0 );
        }
    }
    
    fh_context
    RedlandRootContext::addStatement( fh_statement st )
    {
        fh_context ret = 0;
        fh_node    subj = st->getSubject();
        fh_node    pred = st->getPredicate();
        fh_node    obj  = st->getObject();

        if( obj->isLiteral() )
        {
            LG_RDF_D << "RedlandRootContext::priv_read(loop, attr)"
                 << " url:" << getURL() << endl
                 << "    pred:" << pred->getURI()->toString()
                 << "     obj:" << obj->toString()
                 << endl;
            addNewAttribute( pred->getURI()->toString(), XSD_BASIC_STRING );
            return 0;
        }
        
        if( !priv_isSubContextBound( pred->getURI()->toString() ) )
        {
            LG_RDF_D << "priv_read() making directory for predicate:" << pred->getURI()->toString() << endl;
            fh_context child = new RedlandStatementContext( this, pred->getURI()->toString(), 0 );
            addNewChild( child );
        }
                
        fh_context child = priv_getSubContext( pred->getURI()->toString() );
        
//         cerr << "RedlandRootContext::addStatement()"
//              << " url:" << getURL() << endl
//              << "    subj:" << subj->getURI()->toString()
//              << "    pred:" << pred->getURI()->toString()
//              << "     obj:" << obj->toString()
//              << " obj-type:" << obj->getType()
//              << endl;
        
        string rdn = obj->isBlank() ? obj->toString() : obj->getURI()->toString();
        
        if( !child->priv_isSubContextBound( rdn ) )
        {
            fh_context subchild = new RedlandStatementContext( GetImpl(child), rdn, st );
            child->Insert( GetImpl( subchild ));
            LG_RDF_D << "priv_read(made child) pred-child url:" << child->getURL() << endl
                 << " subchild-url:" << subchild->getURL() << endl
                 << " subchild-parent-url:" << subchild->getParent()->getURL()
                 << endl;
            ret = subchild;
        }

        return ret;
    }
    
    bool
    RedlandRootContext::priv_read_TryRootSubject( const std::string& subj_uri, bool bindEvenIfNoResults )
    {
        bool ret = false;

        LG_RDF_D << "RedlandRootContext::priv_read_TryRootSubject() subj:" << subj_uri
                 << " have-base-node:" << isBound( m_baseSubject )
                 << " bindEvenIfNoResults:" << bindEvenIfNoResults
                 << endl;
        
        if( subj_uri.empty() )
            return ret;
        if( m_baseSubject )
            return ret;

        try
        {
            fh_node subj = Node::CreateURI( subj_uri );
            NodeIterator enditer = NodeIterator();
            NodeIterator iter = getModel()->getArcsOut( subj );
            m_baseSubject = subj;
            
            LG_RDF_D << "priv_read_TryRootSubject() has-predicates:" << (iter != NodeIterator())
                     << " subj_uri:" << subj_uri << endl;
            
            for( ; iter != enditer; ++iter )
            {
                LG_RDF_D << "RedlandRootContext::priv_read( loop ) url:" << getURL() << endl;

                fh_node pred = *iter;
                NodeIterator objiter = getModel()->findObjects( subj, pred );
                for( ; objiter != enditer; ++objiter )
                {
                    fh_node obj = *objiter;

                    LG_RDF_D << "RedlandRootContext::priv_read( loop ) url:" << getURL()
                         << " pred:" << pred->toString() << endl
                         << " obj:"  << obj->toString() << endl
                         << " obj->isResource():" << obj->isResource()
                         << " obj->isLiteral():" << obj->isLiteral()
                         << endl;
//                     if( !obj->isResource() )
//                         continue;

                    /*
                     * Redland root context will make an EA for each object that is a literal
                     * which we pass it.
                     */
                    fh_statement st = new Statement( subj, pred, obj );
                    addStatement( st );
                    ret = true;
                }
            }
        }
        catch( exception& e )
        {}

        if( !bindEvenIfNoResults && !ret )
        {
            LG_RDF_D << "RedlandRootContext::priv_read_TryRootSubject(reset) subj:" << subj_uri
                 << " have-base-node:" << isBound( m_baseSubject )
                 << " bindEvenIfNoResults:" << bindEvenIfNoResults
                 << endl;
            m_baseSubject = 0;
        }
        
        
        return ret || isBound(m_baseSubject);
    }
    
    std::string
    RedlandRootContext::priv_read_GetFirstRDFAboutAttribute()
    {
        if( !ends_with( getDirPath(), ".rdf" ) || !ends_with( getDirPath(), ".xml" ) )
            return "";
        
        std::string ret;

        fh_istream iss = getIStream();
        std::string s;
        while( getline( iss, s ) )
        {
            LG_RDF_D << "priv_read_GetFirstRDFAboutAttribute() s:" << s << endl;
            string rdfabout = "rdf:about=\"";
            if( contains( s, rdfabout ))
            {
                int start_pos = s.find( rdfabout );
                start_pos += rdfabout.length();

                int end_pos = s.find( "\"", start_pos );
                ret = s.substr( start_pos, end_pos - start_pos );

                LG_RDF_D << "GetFirstRDFAboutAttribute() ret:" << ret << endl;
                return ret;
            }
        }
        
        return ret;
    }

//     void
//     RedlandRootContext::priv_createAttributes()
//     {
//         string subjstr = getDirName();
                
//         fh_node subj = getBaseSubject();
//         NodeIterator iter = getModel()->getArcsOut( subj );

//         for( ; iter != NodeIterator(); ++iter )
//         {
//             fh_node pred = *iter;
//             fh_node obj  = getModel()->getObject( subj, pred );
//             if( obj->isLiteral() )
//             {
//                 LG_RDF_D << "RedlandRootContext::priv_read(loop, attr)"
//                      << " url:" << getURL() << endl
//                      << "    pred:" << pred->getURI()->toString()
//                      << "     obj:" << obj->toString()
//                      << endl;
//                 addNewAttribute( pred->getURI()->toString(), XSD_BASIC_STRING );
//             }
//         }
//     }

    
    void
    RedlandRootContext::priv_read()
    {
        emitExistsEventForEachItemRAII _raii1( this );

        LG_RDF_D << "RedlandRootContext::priv_read() url:" << getURL() << endl;
        
        if( empty() && !m_model && !m_baseSubject )
        {
//            clearContext();

            LG_RDF_D << "RedlandRootContext::priv_read( reading ) url:" << getURL()
                     << " dirname:" << getDirName()
                     << " getCoveredContext()->dirname:" << getCoveredContext()->getDirName()
                     << " base-uri-ea-this:" << getStrAttr( this, "rdf-root-subject-uri", "" )
                     << " base-uri-ea-cov:" << getStrAttr( getCoveredContext(), "rdf-root-subject-uri", "" )
                     << endl
                     << "    dn-uri:" << RDF_FERRIS_BASE << "/" << getCoveredContext()->getDirName()
                     << endl;

            try
            {
                if( !m_model )
                    m_model = Model::FromMetadataContext( getCoveredContext() );

                LG_RDF_D << "RedlandRootContext::priv_read( reading ) url:" << getURL()
                         << " modelPath:" << getCoveredContext()->getURL()
                         << " have model:" << isBound( m_model ) << endl;

                if( !m_model )
                    return;

                if( !priv_read_TryRootSubject( getURL() )
                    && !priv_read_TryRootSubject( RDF_FERRIS_BASE + "/rdf-root-subject-uri" )
                    && !priv_read_TryRootSubject( getStrAttr( this, "rdf-root-subject-uri", "" ), true ) 
//                     && !priv_read_TryRootSubject( RDF_FERRIS_BASE
//                                                   + "/rdf-root-subject-uri" ) 
                    && !priv_read_TryRootSubject( RDF_FERRIS_BASE
                                                  + "/" + getCoveredContext()->getDirName() ) 
                    && !priv_read_TryRootSubject( priv_read_GetFirstRDFAboutAttribute() ) 
                    )
                {
                    fh_stringstream ss;
                    ss << "Error reading list of triples in RDF repository, can't find root triple"
                       << " for url:" << getURL() << endl;

                    cerr << tostr(ss) << endl;
                    BackTrace();
                    Throw_CanNotReadContext( tostr(ss), this );
                }
            }
            catch(exception& e)
            {
                fh_stringstream ss;
                ss << "Error reading list of triples in RDF repository e:" << e.what() << endl;
                Throw_CanNotReadContext( tostr(ss), this );
            }
        }
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

//     XMLBaseContext*
//     RedlandStatementContext::ensureCreated( const std::string& rdn )
//     {
//         Util::ValueBumpDrop<ref_count_t> dummy( ref_count );

//         if( !isSubContextBound( rdn ) )
//         {
//             RedlandStatementContext* ret = (RedlandStatementContext*)CreateContext( this, monsterName( rdn ));
//             Insert( ret );
//             return ret;
//         }
//         return (RedlandStatementContext*)GetImpl(getSubContext( rdn ));
//     }

    fh_node
    RedlandStatementContext::getParentsObject()
    {
        if( m_st )
            return m_st->getObject();

        fh_node ret = 0;
        
        // This is a predicate, they just want to create a new file using the
        // same subject and predicate as this context has.
        if( RedlandStatementContext* pc = dynamic_cast<RedlandStatementContext*>( getParent() ))
        {
            LG_RDF_D << "RedlandStatementContext::SubCreate_file(1)" << endl;
            if( pc->m_st )
                ret = pc->m_st->getObject();
        }
        if( RedlandRootContext* pc = dynamic_cast<RedlandRootContext*>( getParent()->getOverMountContext() ))
        {
            LG_RDF_D << "RedlandStatementContext::SubCreate_file(2)" << endl;
            if( pc->getBaseSubject() )
                ret = pc->getBaseSubject();
        }
        return ret;
    }

    fh_context
    RedlandStatementContext::SubCreate_file( fh_context c, fh_context md )
    {
        string nameuri  = getStrSubCtx( md, "name",   "" );
        string objuri   = getStrSubCtx( md, "object", "" );

        fh_node subj = 0;
        fh_node pred = 0;
        
        if( !m_st )
        {
            objuri = nameuri;
            pred = Node::CreateURI( getDirName() );

            LG_RDF_D << "RedlandStatementContext::SubCreate_file() no statement, pred:" << pred->toString() << endl;
            LG_RDF_D << "RedlandStatementContext::SubCreate_file() parent:" << getParent()->getURL() << endl;
            
            // This is a predicate, they just want to create a new file using the
            // same subject and predicate as this context has.
            if( RedlandStatementContext* pc = dynamic_cast<RedlandStatementContext*>( getParent() ))
            {
                LG_RDF_D << "RedlandStatementContext::SubCreate_file(1)" << endl;
                if( pc->m_st )
                    subj = pc->m_st->getObject();
            }
            if( RedlandRootContext* pc = dynamic_cast<RedlandRootContext*>( getParent()->getOverMountContext() ))
            {
                LG_RDF_D << "RedlandStatementContext::SubCreate_file(2)" << endl;
                if( pc->getBaseSubject() )
                    subj = pc->getBaseSubject();
            }
        }
        else
        {
            subj = m_st->getObject();
            pred = Node::CreateURI( nameuri );
        }
        
        if( !subj || !pred )
        {
            fh_stringstream ss;
            ss << "Attempt to create file in RDF repository when subject and predicate can not be determined."
               << " url:" << getURL();
            Throw_FerrisCreateSubContextFailed( tostr(ss), this );
        }
        
        fh_node obj  = Node::CreateURI( objuri );

        LG_RDF_D << "RedlandStatementContext::SubCreate_file() subj:" << subj->toString() << endl;
        LG_RDF_D << "RedlandStatementContext::SubCreate_file() pred:" << pred->toString() << endl;
        LG_RDF_D << "RedlandStatementContext::SubCreate_file()  obj:" <<  obj->toString() << endl;
        
        fh_statement st = new Statement( subj, pred, obj );
        getModel()->insert( st );
        getModel()->sync();

        
        fh_context child = addStatement( st );
        return child;
    }

    fh_context
    RedlandStatementContext::SubCreate_ea( fh_context c, fh_context md )
    {
        string rdn      = getStrSubCtx( md, "name", "" );
        string objvalue = getStrSubCtx( md, "object", "" );
        string predstr  = rdn;

        LG_RDF_D << "RedlandStatementContext::SubCreate_ea() rdn:" << rdn << endl;
        LG_RDF_D << "RedlandStatementContext::SubCreate_ea() objvalue:" << objvalue << endl;
        
        if( !m_st )
        {
            fh_stringstream ss;
            ss << "Attempt to create EA for RDF repository which is not a statement."
               << " url:" << getURL();
            Throw_FerrisCreateAttributeFailed( tostr(ss), this );
        }
        
        fh_node subj = m_st->getObject();
        fh_node pred = Node::CreateURI( predstr );
        fh_node obj  = Node::CreateLiteral( objvalue );

        fh_statement st = new Statement( subj, pred, obj );
        getModel()->insert( st );
        getModel()->sync();

        addNewAttribute( pred->getURI()->toString(), XSD_BASIC_STRING );
        return c;
    }

    void
    RedlandStatementContext::priv_remove( fh_context c_ctx )
    {
        string url = c_ctx->getURL();
        
        LG_RDF_D << "RedlandStatementContext::redland_remove(top) url:" << getURL() << endl;
                
        RedlandStatementContext* c = dynamic_cast<RedlandStatementContext*>( (GetImpl(c_ctx) ) );
        if( !c )
        {
            fh_stringstream ss;
            ss << "Attempt to remove a non RDF context from within a redland file!"
               << " url:" << c_ctx->getURL();
            Throw_CanNotDelete( tostr(ss), GetImpl(c_ctx) );
        }
        
        try
        {
            NodeIterator objiter = NodeIterator();
            fh_node subj = 0;
            fh_node pred = 0;
            
            if( m_st )
            {
                subj = m_st->getObject();
                pred = Node::CreateURI( c->getDirName() );
                objiter = getModel()->findObjects( subj, pred );
                getModel()->erase( subj, pred, objiter );
            }
            else
            {
                subj = getParentsObject();
                pred = Node::CreateURI( getDirName() );
                getModel()->erase( subj, pred, Node::CreateURI( c->getDirName() ) );
            }

            LG_RDF_D << "RedlandStatementContext::redland_remove(top) url:" << getURL() << endl
                 << " subj:" << subj->toString() << endl
                 << " pred:" << pred->toString() << endl
                 << endl;

            getModel()->sync();
        }
        catch( exception& e )
        {
            std::ostringstream ss;
            ss << "Can not delete child:" << url
               << " reason:" << e.what() << endl;
            Throw_CanNotDelete( tostr(ss), 0 );
        }
    }
    
    RedlandStatementContext*
    RedlandStatementContext::getParentWithObject( fh_node n )
    {
        Context* p = getParent();
        while( RedlandStatementContext* pc = dynamic_cast<RedlandStatementContext*>( p ) )
        {
            /* PURE DEBUG */
            if( pc->m_st && n && pc->m_st->getObject() )
            {
                LG_RDF_D << "getParentWithObject() url:" << getURL() << endl;
                LG_RDF_D << "  n==pc->obj:" << ( n == pc->m_st->getObject() );
                LG_RDF_D << "  n.str==pc->obj.str:" << (n->toString() == pc->m_st->getObject()->toString()) ;
                if( pc->m_st->getObject()->getURI() )
                LG_RDF_D << "  n.uri==subject.uri:" << (n->toString() == pc->m_st->getObject()->getURI()->toString())
                     << endl;
                LG_RDF_D << "  pc:" << pc->getURL() << endl
                     << "  n:" << n->toString() << endl
                     << "  pc->subj:" << pc->m_st->getSubject()->toString() << endl
                     << "  pc->obj:" << pc->m_st->getObject()->toString() << endl;
            }
            
            if( pc->m_st && n->toString() == pc->m_st->getObject()->toString() )
//                 && pc->m_st->getObject()->getURI()
//                 && n->toString() == pc->m_st->getObject()->getURI()->toString() )
            {
                LG_RDF_D << "getParentWithObject(YES) url:" << getURL() << endl
                     << "  n==pc->obj:" << ( n == pc->m_st->getObject() )
                     << "  n.str==pc->obj.str:" << (n->toString() == pc->m_st->getObject()->toString()) 
//                      << "  n.uri==subject.uri:" << (n->toString() == pc->m_st->getObject()->getURI()->toString())
//                      << endl
                     << "  pc:" << pc->getURL() << endl
                     << "  n:" << n->toString() << endl
                     << "  pc->subj:" << pc->m_st->getSubject()->toString() << endl
                     << "  pc->obj:" << pc->m_st->getObject()->toString() << endl;
                
                return pc;
            }
            p = p->getParent();
        }
        return 0;
    }

    void
    RedlandStatementContext::priv_createAttributes()
    {
        string subjstr = getDirName();
                
        fh_node subj = Node::CreateURI( subjstr );
        NodeIterator iter = getModel()->getArcsOut( subj );

        for( ; iter != NodeIterator(); ++iter )
        {
            fh_node pred = *iter;
            fh_node obj  = getModel()->getObject( subj, pred );
            if( obj->isLiteral() )
            {
                LG_RDF_D << "RedlandStatementContext::priv_read(loop, attr)"
                     << " url:" << getURL() << endl
                     << "    pred:" << pred->getURI()->toString()
                     << "     obj:" << obj->toString()
                     << endl;
                addNewAttribute( pred->getURI()->toString(), XSD_BASIC_STRING );
            }
        }
    }


    fh_context
    RedlandStatementContext::addStatement( fh_statement st )
    {
        fh_context ret = 0;
        fh_node   subj = st->getSubject();
        fh_node   pred = st->getPredicate();
        fh_node   obj  = st->getObject();
        
        if( !priv_isSubContextBound( pred->getURI()->toString() ) )
        {
            fh_context child = new RedlandStatementContext( this, pred->getURI()->toString(), 0 );
            addNewChild( child );
        }
                
        fh_context child = priv_getSubContext( pred->getURI()->toString() );

        string rdn = obj->getURI()->toString();
        if( !child->priv_isSubContextBound( rdn ) )
        {
            if( RedlandStatementContext* pc = getParentWithObject( obj ) )
            {
                LG_RDF_D << "Making a *link* context. This:" << getURL() << endl
                     << "   subj:" << subj->toString()
                     << "   pred:" << pred->toString()
                     << "    obj:" << obj->toString()
                     << "   link creation for url:" << pc->getURL() << endl;
                // insert a "link" to the parent context //
                fh_context cvc = new VirtualSoftlinkContext( child, pc );
                child->Insert( GetImpl( cvc ));
                ret = cvc;
            }
            else
            {
                fh_context subchild = new RedlandStatementContext( GetImpl(child), rdn, st );
                child->Insert( GetImpl( subchild ));
                ret = subchild;
            }
        }

        return ret;
    }
    
    void
    RedlandStatementContext::priv_read()
    {
        LG_RDF_D << "RedlandStatementContext::priv_read(top) url:" << getURL() << endl;

        staticDirContentsRAII _raii1( this );

        if( empty() )
        {
            if( !m_st )
            {
                emitExistsEventForEachItem();
                return;
            }
            
            LG_RDF_D << "RedlandStatementContext::priv_read(go) url:" << getURL() << endl;
            try
            {
                string subjstr = getDirName();
                fh_node subj = Node::CreateURI( subjstr );
                NodeIterator prediter = getModel()->getArcsOut( subj );
//            NodeIterator    findObjects(    fh_node subj, fh_node pred );

                LG_RDF_D << "RedlandStatementContext::priv_read() url:" << getURL() << endl
                     << " subject:" << subjstr 
                     << " has-items:" << ( prediter != NodeIterator() )
                     << endl;
                
                for( ; prediter != NodeIterator(); ++prediter )
                {
                    fh_node pred = *prediter;
                    NodeIterator objiter = getModel()->findObjects( subj, pred );
                    for( ; objiter != NodeIterator(); ++objiter )
                    {
                        fh_node obj = *objiter;

                        if( !obj->isResource() )
                            continue;
                    
                        fh_statement st = new Statement( subj, pred, obj );

                        LG_RDF_D << "RedlandStatementContext::priv_read(loop) url:" << getURL() << endl;
                        LG_RDF_D << "      pred:" << pred->toString() << endl;
                        LG_RDF_D << "       obj:" <<  obj->toString() << endl;
                        LG_RDF_D << "is-literal:" <<  obj->isLiteral() << endl;
                        

                        addStatement( st );
                        
//                         if( !priv_isSubContextBound( pred->getURI()->toString() ) )
//                         {
//                             fh_context child = new RedlandStatementContext( this, pred->getURI()->toString(), 0 );
//                             addNewChild( child );
//                         }
                
//                         fh_context child = priv_getSubContext( pred->getURI()->toString() );

//                         string rdn = obj->getURI()->toString();
//                         if( !child->priv_isSubContextBound( rdn ) )
//                         {
//                             if( RedlandStatementContext* pc = getParentWithObject( obj ) )
//                             {
//                                 LG_RDF_D << "Making a *link* context. This:" << getURL() << endl
//                                      << "   subj:" << subj->toString()
//                                      << "   pred:" << pred->toString()
//                                      << "    obj:" << obj->toString()
//                                      << "   link creation for url:" << pc->getURL() << endl;
//                                 // insert a "link" to the parent context //
//                                 fh_context cvc = new VirtualSoftlinkContext( child, pc );
//                                 child->Insert( GetImpl( cvc ));
//                             }
//                             else
//                             {
//                                 fh_context subchild = new RedlandStatementContext( GetImpl(child), rdn, st );
//                                 child->Insert( GetImpl( subchild ));
//                             }
                            
//                         }





                        
                        
//                         if( obj->isLiteral() )
//                         {
// //                         LG_RDF_D << "RedlandStatementContext::priv_read(loop, attr)" << endl;
// //                         addNewAttribute( pred->getURI()->toString(), XSD_BASIC_STRING );
//                         }
//                         else
//                         {
//                             LG_RDF_D << "RedlandStatementContext::priv_read(loop, child)" << endl;

//                             if( RedlandStatementContext* pc = getParentWithObject( obj ) )
//                             {
//                                 // insert a "link" to the parent context //
//                                 fh_context cvc = new VirtualSoftlinkContext( this, pc );
//                                 addNewChild( cvc );
//                             }
//                             else
//                             {
//                                 fh_context child = new RedlandStatementContext( this,
//                                                                                 pred->getURI()->toString(),
//                                                                                 st );
//                                 addNewChild( child );
//                             }
//                         }
                    }
                }
            }
            catch(exception& e)
            {
                fh_stringstream ss;
                ss << "Error reading triples in RDF repository e:" << e.what() << endl;
                Throw_CanNotReadContext( tostr(ss), this );
            }
        }
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {

            static RedlandRootContext raw_obj(0,"/");
            LG_RDF_D << "Brew() root:" << rf->getInfo( "Root" )
                     << " path:" << rf->getInfo( "Path" )
                     << endl;
            
            fh_context ret = raw_obj.CreateContext( 0, rf->getInfo( "Root" ));
            return ret;
        }
    }
 
};
