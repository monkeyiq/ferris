/******************************************************************************
*******************************************************************************
*******************************************************************************

    RDF core functions, use soprano behind a custom API

    Copyright (C) 2009 Ben Martin

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

    $Id: FerrisRDFCore.cpp,v 1.9 2010/09/24 21:30:41 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "Ferris.hh"

#include "FerrisRDFCore.hh"
#include "FerrisQt_private.hh"

#include "Enamel.hh"

#include "FerrisQt_private.hh"
#include <Soprano/PluginManager>
#include <Soprano/Model>
#include <Soprano/Soprano>
#include <Soprano/Backend>
#include <Soprano/BackendSetting>
#include <Soprano/Client/DBusClient>
#include <Soprano/Client/DBusModel>

#include "FerrisKDE.hh"

#include "config.h"
#ifdef HAVE_NEPOMUK
#include <KDE/Nepomuk/ResourceManager>
#endif

//#undef  LG_RDF_D
//#define  LG_RDF_D cerr

namespace Ferris
{
    std::string tostr( RDFCore::fh_node n )
    {
        if( !isBound(n) )
            return "null node";
        return n->getURI()->toString();
    }
    
    namespace RDFCore
    {
        const std::string RDF_FERRIS_BASE = "http://witme.sf.net/libferris-core/0.1";
        
        using namespace std;
        using Soprano::LiteralValue;
        
        /****************************************/
        /****************************************/
        /****************************************/

        bool operator==(const URI& __x, const URI& __y)
        {
            return __x.toString() != __y.toString();
        }
        bool operator!=(const URI& __x, const URI& __y)
        {
            return !(__x == __y);
        }
            

        URI::URI( const std::string& uri )
            :
            m_uri( uri )
        {
        }
        
        URI::~URI()
        {
        }
        
        std::string
        URI::toString() const
        {
            return m_uri;
        }
        
        std::string
        URI::getPath() const
        {
            return m_uri;
        }
        

        /****************************************/
        /****************************************/
        /****************************************/

        bool operator==(const fh_node& __x, const fh_node& __y)
        {
            return *__x == *__y;
        }
        bool operator==(const Node& __x, const Node& __y)
        {
            return *__x.m_node == *__y.m_node;
        }
        bool operator!=(const Node& __x, const Node& __y)
        {
            return !(__x == __y);
        }

        Node::Node( Soprano::Node* n )
            :
            m_node( n )
        {
        }


        fh_node Node::CreateURI( const std::string& uri )
        {
            Soprano::Node* n = new Soprano::Node( QUrl( uri.c_str() ) );
            return new Node( n );
        }
        
        fh_node Node::CreateLiteral( const std::string& s )
        {
            Soprano::Node* n = new Soprano::Node( LiteralValue( s.c_str() ) );
            return new Node( n );
        }
        
        fh_node Node::CreateLiteral( const std::wstring& s )
        {
            Soprano::Node* n = new Soprano::Node( LiteralValue( (const char*)s.c_str() ) );
            return new Node( n );
        }
        
        fh_node Node::CreateBlankNode( const std::string& s )
        {
            Soprano::Node n = Soprano::Node::createBlankNode( QString( s.c_str() ) );
            return new Node( new Soprano::Node(n) );
        }

        fh_node Node::CreateEmptyNode()
        {
            Soprano::Node* n = new Soprano::Node();
            return new Node( n );
        }
            
        

        Node::~Node()
        {
        }

        Soprano::Node
        Node::getRAW()
        {
            return *m_node;
        }
        
        fh_uri
        Node::getURI()
        {
            return new URI(tostr( m_node->uri().toString() ));
        }
        
        Soprano::Node::Type
        Node::getType()
        {
            return m_node->type();
        }
        
        std::string
        Node::getLiteralValue()
        {
            return tostr( m_node->literal().toString() );
        }
        
        std::string
        Node::getLiteralLang()
        {
            return tostr( m_node->language() );
        }
        
        std::string
        Node::getBlankID()
        {
            return tostr( m_node->identifier() );
        }
        
        bool
        Node::isResource()
        {
            return m_node->isResource();
        }
    
        bool
        Node::isLiteral()
        {
            return m_node->isLiteral();
        }
        
        bool
        Node::isLiteralXML()
        {
            return m_node->isLiteral();
        }
        
        bool
        Node::isBlank()
        {
            return m_node->isBlank();
        }
        
        std::string
        Node::toString()
        {
            return tostr( m_node->toString() );
        }
        
     
        /****************************************/
        /****************************************/
        /****************************************/

        bool operator==(const Statement& __x, const Statement& __y)
        {
            return *__x.m_statement == *__y.m_statement;
        }
        
        bool operator!=(const Statement& __x, const Statement& __y)
        {
            return !(__x == __y);
        }

        Statement::Statement( Soprano::Statement* m_statement )
            :
            m_statement( m_statement )
        {
        }

        Statement::Statement()
            :
            m_statement( new Soprano::Statement(
                             Soprano::Node(),
                             Soprano::Node(),
                             Soprano::Node() ))
        {
        }
        
        Statement::Statement( fh_node sub, fh_node pred, fh_node obj )
            :
            m_statement( new Soprano::Statement( sub->getRAW(),
                                                 pred->getRAW(),
                                                 obj->getRAW() ) )
        {
        }
        
        
        Statement::~Statement()
        {
        }
        
        Soprano::Statement
        Statement::getRAW()
        {
            return *m_statement;
        }
        

        void
        Statement::setSubject( fh_node n )
        {
            m_statement->setSubject( n->getRAW() );
        }
        
        void
        Statement::setPredicate( fh_node n )
        {
            m_statement->setPredicate( n->getRAW() );
        }
        
        void
        Statement::setObject( fh_node n )
        {
            m_statement->setObject( n->getRAW() );
        }

        fh_node
        Statement::getSubject()
        {
            return new Node( new Soprano::Node(m_statement->subject()) );
        }
        
        fh_node
        Statement::getPredicate()
        {
            return new Node( new Soprano::Node(m_statement->predicate()) );
        }
            
        fh_node
        Statement::getObject()
        {
            return new Node( new Soprano::Node(m_statement->object()) );
        }
            
        
        
        void
        Statement::clear()
        {
            m_statement->setSubject( Soprano::Node() );
            m_statement->setPredicate( Soprano::Node() );
            m_statement->setObject( Soprano::Node() );
        }
        
        bool
        Statement::isComplete()
        {
            return m_statement->isValid();
        }
        
        std::string
        Statement::toString()
        {
            stringstream ret;
            ret << getSubject()->toString();
            ret << " ";
            ret << getPredicate()->toString();
            ret << " ";
            ret << getObject()->toString();
            return ret.str();
        }
        
        bool
        Statement::match( fh_statement partial_st )
        {
            return getSubject()   == getSubject()
                && getPredicate() == getPredicate()
                && getObject()    == getObject();
            
        }
        
        std::string
        Statement::toNTripleString( fh_model m )
        {
            // FIXME
            return "";
        }
        
        std::string
        Statement::toRDFXMLString( fh_model m )
        {
            // FIXME
            return "";
        }
        
        std::string
        Statement::toXString( fh_model m, const std::string& format )
        {
            // FIXME
            return "";
        }
        
        
        

        /****************************************/
        /****************************************/
        /****************************************/

        FERRISEXP_API bool operator==(const StatementIterator& __x, const StatementIterator& __y)
        {
            // cerr << "__x.valid:" << __x.m_iter.isValid() << endl;
            // cerr << "__y.valid:" << __y.m_iter.isValid() << endl;

            if( (__x.m_iter.isValid() && !__y.m_iter.isValid())
                ||
                (!__x.m_iter.isValid() && __y.m_iter.isValid()) )
            {
                return false;
            }
            if( !__x.m_iter.isValid() && !__y.m_iter.isValid() )
                return true;
            
            return *(__x.m_iter) == *(__y.m_iter);
        }
        
        FERRISEXP_API bool operator!=(const StatementIterator& __x, const StatementIterator& __y)
        {
            return !(__x == __y);
        }
        
        StatementIterator::StatementIterator( Soprano::StatementIterator m_iter )
            :
            m_iter( m_iter )
        {
            if( m_iter.isValid() )
                m_iter.next();
        }

        StatementIterator::~StatementIterator()
        {
        }

        void
        StatementIterator::__advance()
        {
            bool v = m_iter.next();
//            cerr << "__advance() v:" << v << endl;
            if( !v )
            {
                m_iter = Soprano::StatementIterator();
            }
        }
        
        
        StatementIterator::reference
        StatementIterator::operator*()
        {
            return new Statement( new Soprano::Statement(*m_iter) );
        }


        NodeIterator
        StatementIterator::iterateObjects () const
        {
            // if( !m_iter.isValid() )
            // {
            //     LG_RDF_D << "StatementIterator::iterateObjects() not valid" << endl;
            //     return NodeIterator();
            // }
            
            // LG_RDF_D << "StatementIterator::iterateObjects() IS valid" << endl;
            return NodeIterator(m_iter.iterateObjects());
        }

        NodeIterator
        StatementIterator::iteratePredicates () const
        {
            return NodeIterator(m_iter.iteratePredicates());
        }
            
        NodeIterator
        StatementIterator::iterateSubjects () const
        {
            return NodeIterator(m_iter.iterateSubjects());
        }
        
        /****************************************/

        bool operator==(const NodeIterator& __x, const NodeIterator& __y)
        {
//            return __x.m_base.current().toString() == __y.m_base.current().toString();

            // LG_RDF_D << "operator==() x.valid:" << __x.m_base.isValid()
            //          << " y.valid:" << __y.m_base.isValid()
            //          << endl;
            
            if( (__x.m_base.isValid() && !__y.m_base.isValid())
                ||
                (!__x.m_base.isValid() && __y.m_base.isValid()) )
            {
                return false;
            }
            if( !__x.m_base.isValid() && !__y.m_base.isValid() )
                return true;
            
            return *(__x.m_base) == *(__y.m_base);
            
        }
        bool operator!=(const NodeIterator& __x, const NodeIterator& __y)
        {
            return !(__x == __y);
        }
        

        NodeIterator::NodeIterator( Soprano::NodeIterator base )
            :
            m_base( base )
        {
            // if( m_base.isValid() )
            // m_base.next();
        }

        NodeIterator::~NodeIterator()
        {
        }

        NodeIterator::reference
        NodeIterator::operator*()
        {
            return new Node( new Soprano::Node(*m_base) );
        }
        
                
        void
        NodeIterator::__advance()
        {
            bool v = m_base.next();
            if( !v )
            {
                // LG_RDF_D << "NodeIterator::__advance() no next!" << endl;
                m_base = Soprano::NodeIterator();
            }
        }


        //////
        //////
        //////

        FERRISEXP_API bool operator==(const BindingsIterator& __x, const BindingsIterator& __y)
        {
            if( (__x.m_iter.isValid() && !__y.m_iter.isValid())
                ||
                (!__x.m_iter.isValid() && __y.m_iter.isValid()) )
            {
                return false;
            }
            if( !__x.m_iter.isValid() && !__y.m_iter.isValid() )
                return true;

            return false;
//            return __x.m_iter == __y.m_iter;
        }
        FERRISEXP_API bool operator!=(const BindingsIterator& __x, const BindingsIterator& __y)
        {
            return !(__x == __y);
        }
        
        BindingsIterator::BindingsIterator( Soprano::QueryResultIterator m_iter )
            :
            m_iter( m_iter )
        {
            if( m_iter.isValid() )
                m_iter.next();
        }
        BindingsIterator::~BindingsIterator()
        {
        }
        void
        BindingsIterator::__advance()
        {
            bool v = m_iter.next();
            if( !v )
            {
                m_iter = Soprano::QueryResultIterator();
            }
        }
        
        int
        BindingsIterator::bindingCount () const
        {
            return m_iter.bindingCount();
        }
        
        stringlist_t
        BindingsIterator::bindingNames () const
        {
            QStringList ql = m_iter.bindingNames();
            stringlist_t ret;
            for( QStringList::iterator qiter = ql.begin(); qiter != ql.end(); ++qiter )
                ret.push_back( tostr( *qiter ) );
            return ret;
        }
        
        fh_node
        BindingsIterator::operator[] (const std::string& name) const
        {
            Soprano::Node n = m_iter[ name.c_str() ];
            return new Node( new Soprano::Node( n ) );
        }
        
        fh_node
        BindingsIterator::operator[] (int idx) const
        {
            Soprano::Node n = m_iter[ idx ];
            return new Node( new Soprano::Node( n ) );
        }
        
        
        
        /****************************************/
        /****************************************/
        /****************************************/

        Model::Model( Soprano::Model* m )
            :
            m_model( m )
        {
        }

        fh_model
        Model::FromMetadataContext( const std::string& earl )
        {
            fh_context md = Shell::acquireContext( earl );
            return FromMetadataContext( md );
        }
        
        void addSettingIfPresent( fh_context md,
                                  const std::string& filename,
                                  Soprano::BackendOption option,
                                  QList<Soprano::BackendSetting>& ret )
        {
            try
            {
                ret.append(
                    Soprano::BackendSetting(
                        option,
                        getStrSubCtx( md, filename, "", true, true ).c_str() ) );
            }
            catch(...)
            {
            }
        }
        
        fh_model
        Model::FromMetadataContext( fh_context md )
        {
            if( md->isSubContextBound( "metadata" ) )
                md = md->getSubContext( "metadata" );

            string backendName = getStrSubCtx( md, "backend", "redland" );
            string dbpath = md->getParent()->getDirPath();
            Soprano::Model* model = 0;
            LG_RDF_D << "FromMetadataContext() backend:" << backendName << " dbpath:" << dbpath << endl;
            
            const Soprano::Backend* backend = Soprano::PluginManager::instance()->discoverBackendByName( backendName.c_str() );
            QList<Soprano::BackendSetting> settings;
            settings.append( Soprano::BackendSetting( Soprano::BackendOptionStorageDir, dbpath.c_str() ) );

            // QStringList userSettings = backendSettings.split( ';', QString::SkipEmptyParts );
            // foreach( const QString& setting, userSettings ) {
            //     QStringList keyValue = setting.split( '=' );
            //     if ( keyValue.count() != 2 ) {
            //         errStream << "Invalid backend setting: " << setting;
            //         return 2;
            //     }
            //     settings << BackendSetting( keyValue[0], keyValue[1] );
            // }
//            cerr << "backendName:" << backendName << endl;
            if( backendName == "virtuoso" )
            {
                addSettingIfPresent( md, "host",     Soprano::BackendOptionHost,     settings );
                addSettingIfPresent( md, "port",     Soprano::BackendOptionPort,     settings );
                addSettingIfPresent( md, "username", Soprano::BackendOptionUsername, settings );
                addSettingIfPresent( md, "password", Soprano::BackendOptionPassword, settings );
            }
            else if( backendName != "sesame2" )
            {
                settings << Soprano::BackendSetting( "name", "myrdf" );
            }

            // quiet down soprano qDebug messages
            installQTMsgHandler();
            
            if( isTrue( getStrSubCtx( md, "nepomuk-use-default-model", "0" ) ))
            {
#ifdef HAVE_NEPOMUK
                LG_RDF_D << "using default nepomuk model..." << endl;
                KDE::ensureKDEApplication();
                Nepomuk::ResourceManager::instance()->init();
                model = Nepomuk::ResourceManager::instance()->mainModel();
#else
                cerr << "ERROR, nepomuk model requested but code not compiled to support it!" << endl;
#endif
            }
            else
            {
                string NepomukModelName = getStrSubCtx( md, "nepomuk-model-name", "" );
                if( !NepomukModelName.empty() )
                {
                    LG_RDF_D << "using dbus client...model:" << NepomukModelName << endl;
//                    cerr << "using dbus client...model:" << NepomukModelName << endl;
                    static Soprano::Client::DBusClient* client = new Soprano::Client::DBusClient( "org.kde.NepomukStorage" );
                    model = client->createModel( NepomukModelName.c_str(), settings );
                    if ( !model )
                    {
                        cerr << "Failed to create Model: " << client->lastError() << endl;
                    }
                }
                else
                {
                    LG_RDF_D << "using local storage..." << endl;
//                    cerr << "using local storage..." << endl;
                    model = backend->createModel( settings );
                    if ( !model )
                    {
                        cerr << "Failed to create Model: " << backend->lastError() << endl;
                    }
                }
            }
            

            return new Model( model );
        }
        
        
        fh_model
        Model::MemoryOnlyModel()
        {
            Soprano::Model* m = Soprano::createModel();
            fh_model ret = new Model( m );
            return ret;
        }


        fh_model
        Model::FromRedland( fh_context c, const std::string& dbname )
        {
            fh_context md = Shell::acquireSubContext( c, "metadata" );
            return FromMetadataContext( md );
        }
        
        
        fh_model
        Model::FromXMLString( const std::string& rdfxml )
        {
            fh_model   model   = MemoryOnlyModel();
            fh_parser  parser  = new Parser();
            parser->ParseIntoModel( model, rdfxml, new URI( "file:///dev/null" ) );
            return model;
        }
        
        void
        Model::MergeRDFXML( const std::string& rdfxml, fh_uri base_uri )
        {
            fh_parser  parser  = new Parser();
            parser->ParseIntoModel( this, rdfxml, base_uri );
        }
        
        Model::~Model()
        {
        }
        

        fh_node
        Model::CreateBlankNode()
        {
            Soprano::Node n = m_model->createBlankNode();
            return new Node( new Soprano::Node(n) );
        }
        
        void
        Model::sync()
        {
        }
        
        int
        Model::size()
        {
            return m_model->statementCount();
        }
        

        void
        Model::insert( fh_statement st )
        {
            return addStatement( st );
        }
        
        void
        Model::insert( StatementIterator iter, StatementIterator end )
        {
            for( ; iter != end ; ++iter )
            {
                insert( *iter );
            }
        }

        void
        Model::insert( fh_node subj, fh_node pred, fh_node obj )
        {
            insert( new Statement( subj, pred, obj ) );
        }

        void
        Model::insert( fh_model m )
        {
            insert( m->begin(), m->end() );
        }

        StatementIterator
        Model::begin()
        {
            return StatementIterator( m_model->listStatements() );
        }
        
        StatementIterator
        Model::end()
        {
            return StatementIterator();
        }
        
        void
        Model::addStatement( fh_statement st )
        {
            LG_RDF_D << "addStatement()... " << endl
                     << "  subj:" << st->getSubject()->toString()
                     << "  pred:" << st->getPredicate()->toString()
                     << "  obj:" << st->getObject()->toString()
                     << endl;
            // cerr << "addStatement()... " << endl
            //          << "  subj:" << st->getSubject()->toString()
            //          << "  pred:" << st->getPredicate()->toString()
            //          << "  obj:" << st->getObject()->toString()
            //          << endl;
            // BackTrace();
            
            m_model->addStatement( st->getSubject()->getRAW(),
                                   st->getPredicate()->getRAW(),
                                   st->getObject()->getRAW() );
        }

        StatementIterator
        Model::find( const std::string& query, const std::string& queryLanguage )
        {
            Soprano::Query::QueryLanguage ql = Soprano::Query::QueryLanguageSparql;
            
            Soprano::QueryResultIterator qri = m_model->executeQuery( query.c_str(), ql );
            Soprano::StatementIterator    si = qri.iterateStatements();
            return StatementIterator( si );
        }

        BindingsIterator
        Model::findBindings( const std::string& query,
                             const std::string& queryLanguage )
        {
            Soprano::Query::QueryLanguage ql = Soprano::Query::QueryLanguageSparql;
            Soprano::QueryResultIterator qri = m_model->executeQuery( query.c_str(), ql );
            return BindingsIterator( qri );
        }
        
        
        
        fh_ostream
        Model::write( fh_ostream oss,
                      const std::string& serializerName,
                      const std::string& mimeType,
                      fh_uri type_uri )
        {
            QByteArray* ba = new QByteArray( 0 );
            QTextStream stream( ba );

            Soprano::StatementIterator data = m_model->listStatements();
            QString serialization = "application/x-nquads";
            
            const Soprano::Serializer* serializer
                = Soprano::PluginManager::instance()->discoverSerializerForSerialization( Soprano::mimeTypeToSerialization( serialization ),
                                                                                          serialization );
            stringstream emsg;
            if ( serializer ) {
                //
                // As we support a bunch of default prefixes in queries, we do so with serialization
                //
                serializer->addPrefix( "rdf", Soprano::Vocabulary::RDF::rdfNamespace() );
                serializer->addPrefix( "rdfs", Soprano::Vocabulary::RDFS::rdfsNamespace() );
                serializer->addPrefix( "xsd", Soprano::Vocabulary::XMLSchema::xsdNamespace() );
                serializer->addPrefix( "nrl", Soprano::Vocabulary::NRL::nrlNamespace() );
                serializer->addPrefix( "nao", Soprano::Vocabulary::NAO::naoNamespace() );

                if ( serializer->serialize( data, stream, Soprano::mimeTypeToSerialization( serialization ), serialization ) )
                {
                    oss << *ba << flush;
                    delete ba;
                    return oss;
                }
                else
                {
                    emsg << "Failed to export statements: " << serializer->lastError() << endl;
                }
            }
            else
            {
                emsg << "Could not find serializer plugin for serialization " << serialization << endl;
            }

            delete ba;
            Throw_ParseError( emsg.str(), 0 );
        }
        

        StatementIterator
        Model::findStatements( fh_statement partial_statement )
        {
            StatementIterator ret( m_model->listStatements( partial_statement->getRAW() ));
            return ret;
        }
        

        NodeIterator
        Model::findSubjects(   fh_node pred, fh_node obj )
        {
            LG_RDF_D << "findSubjects() pred:" << toVoid( pred )
                     << " obj:" << toVoid( obj )
                     << endl;

            StatementIterator si = findStatements(
                new Statement( Node::CreateEmptyNode(),
                               pred,
                               obj ));
            return NodeIterator( si.iterateSubjects() );
        }
            
        NodeIterator
        Model::findPredicates( fh_node subj, fh_node obj )
        {
            LG_RDF_D << "findPredicates() subj:" << toVoid( subj )
                     << " obj:" << toVoid( obj ) <<endl;

            StatementIterator si = findStatements(
                new Statement( subj,
                               Node::CreateEmptyNode(),
                               obj ));
            return NodeIterator( si.iteratePredicates() );
        }
            
        NodeIterator
        Model::findObjects( fh_node subj, fh_node pred )
        {
            LG_RDF_D << "findObjects() subj:" << toVoid( subj )
                     << " pred:" << toVoid( pred )
                     << endl;
            StatementIterator si = findStatements(
                new Statement( subj,
                               pred,
                               Node::CreateEmptyNode()
                    ));
            return NodeIterator( si.iterateObjects() );
        }

        fh_node
        Model::getSubject(     fh_node pred, fh_node obj )
        {
            Soprano::Statement st( Soprano::Node(),
                                   pred->getRAW(),
                                   obj->getRAW() );
            Soprano::StatementIterator it = m_model->listStatements( st );
            QList< Soprano::Statement > all = it.allStatements();
            if( !all.empty() )
            {
                Soprano::Statement s = *all.begin();
                return new Node( new Soprano::Node(s.subject()) );
            }
            return 0; // Node::CreateEmptyNode();
        }
        
        fh_node
        Model::getPredicate( fh_node subj, fh_node obj )
        {
            Soprano::Statement st( subj->getRAW(),
                                   Soprano::Node(),
                                   obj->getRAW() );
            Soprano::StatementIterator it = m_model->listStatements( st );
            QList< Soprano::Statement > all = it.allStatements();
            if( !all.empty() )
            {
                Soprano::Statement s = *all.begin();
                return new Node( new Soprano::Node(s.predicate()) );
            }
            return 0; // Node::CreateEmptyNode();
        }
        
        fh_node
        Model::getObject( fh_node subj, fh_node pred )
        {
            if( !m_model ) 
            {
                return 0;
            }
            
//             cerr << "Model::getObject()..." << endl
//                  << "  subj:" << subj->toString()
//                  << "  pred:" << pred->toString()
// //                 << "  obj:" << uuidnode->toString()
//                  << endl;
            
            Soprano::Statement st( subj->getRAW(),
                                   pred->getRAW(),
                                   Soprano::Node() );
            
            Soprano::StatementIterator it = m_model->listStatements( st );
            
            QList< Soprano::Statement > all = it.allStatements();
            if( !all.empty() )
            {
                Soprano::Statement s = *all.begin();
                return new Node( new Soprano::Node(s.object()) );
            }

            // trying to be faster.
//             Soprano::NodeIterator ni = it.iterateObjects();
//             if( ni.next() )
// //            if( ni.isValid() )
//             {
//                 return new Node( new Soprano::Node( *ni ) );
//             }
            
            return 0; //Node::CreateEmptyNode();
        }

        NodeIterator
        Model::getArcsIn(  fh_node n )
        {
//             LG_RDF_D "getArcsIn() n:" << toVoid( n )
//                  << endl;

            Soprano::Statement st( Soprano::Node(),
                                   Soprano::Node(),
                                   n->getRAW() );
            StatementIterator it( m_model->listStatements( st ) );
            return NodeIterator( it.iteratePredicates() );
        }

        NodeIterator
        Model::getArcsOut( fh_node n )
        {
            LG_RDF_D << "Model::getArcsOut() this:" << toVoid( this )
                     << " n:" << toVoid( n )
//                  << " n.impl:" << toVoid( GetImpl( n ))
//                  << " raw:" << toVoid( getRAW() )
//                  << " n.raw:" << toVoid( n->getRAW() )
                     << endl;

            // // DEBUG
            // {
            //     LG_RDF_D << "subject:" << tostr(n->getRAW().toString()) << endl;
            //     Soprano::Statement st( n->getRAW(),
            //                            Soprano::Node(),
            //                            Soprano::Node() );
            //     Soprano::StatementIterator it( m_model->listStatements( st ) );
            //     // if( it.isValid() )
            //     //     it.next();
            //     for( int i=0; i<10; ++i)
            //     {
            //         Soprano::Statement z = *it;
            //         LG_RDF_D << "z.subj:" << tostr(z.subject().toString()) << endl
            //                  << endl;
            //         it.next();
            //     }
            // }
            // LG_RDF_D << "Model::getArcsOut() *********************************" << endl;
           
            Soprano::Statement st( n->getRAW(),
                                   Soprano::Node(),
                                   Soprano::Node() );
            StatementIterator it( m_model->listStatements( st ) );
            return NodeIterator( it.iteratePredicates() );
        }

        bool
        Model::hasArcIn(   fh_node n, fh_node p )
        {
            Soprano::Statement st( Soprano::Node(),
                                   p->getRAW(),
                                   n->getRAW() );
            Soprano::StatementIterator it( m_model->listStatements( st ) );
            QList< Soprano::Statement > all = it.allStatements();
            return !all.empty();
        }
        
        bool
        Model::hasArcOut(  fh_node n, fh_node p )
        {
            Soprano::Statement st( n->getRAW(),
                                   p->getRAW(),
                                   Soprano::Node() );
            Soprano::StatementIterator it( m_model->listStatements( st ) );
            QList< Soprano::Statement > all = it.allStatements();
            return !all.empty();
        }
        
        bool
        Model::contains( fh_statement st )
        {
            return m_model->containsStatement( st->getRAW() );
        }
        
        void
        Model::erase( fh_statement st )
        {
            LG_RDF_D << "Model::erase(st) pred:" << st->getPredicate()->toString()
                     << " obj:" << st->getObject()->toString()
                     << endl;
            m_model->removeAllStatements( st->getRAW() );
        }

        void
        Model::erase( fh_node subj, fh_node pred, fh_node obj )
        {
            m_model->removeStatement( subj->getRAW(), pred->getRAW(), obj->getRAW() );
//            erase( new Statement( subj, pred, obj ) );
        }

        void
        Model::erase( fh_node subj, fh_node pred )
        {
            m_model->removeAllStatements( subj->getRAW(), pred->getRAW(), Soprano::Node() );
                        
            // NodeIterator end   = NodeIterator();
            // NodeIterator oiter = findObjects( subj, pred );
            // LG_RDF_D << "Model::erase() oiter == end:" << (oiter == end) << endl;
            // list< fh_statement > stack;
            // for( ; oiter != end; ++oiter )
            // {
            //     LG_RDF_D << "erase, found object:" << (*oiter)->toString() << endl;
            //     fh_statement st = new Statement( subj, pred, *oiter );
            //     stack.push_back( st );
            // }
            // LG_RDF_D << "Model::erase(s,p) found count:" << stack.size() << endl;

            // for( list< fh_statement >::iterator iter = stack.begin();
            //      iter != stack.end(); ++iter )
            // {
            //     erase( *iter );
            // }
        }

        void
        Model::erase( fh_node subj, fh_node pred, NodeIterator iter )
        {
            nodelist_t nl;
            for( ; iter != NodeIterator(); ++iter )
            {
                LG_RDF_D << "Model::erase(objiter) obj:" << (*iter)->toString() << endl;
                nl.push_back( *iter );
            }
            
            LG_RDF_D << "Model::erase( object iter ) nl.size:" << nl.size() << endl;
            
            for( nodelist_t::iterator ni = nl.begin(); ni != nl.end(); )
            {
                fh_node n = *ni;
                ++ni;
                erase( subj, pred, n );
            }
        }
        
        void
        Model::erase( fh_model m )
        {
            StatementIterator iter = m->begin();
            StatementIterator    e = m->end();

            for( ; iter!=e; ++iter )
            {
                erase( *iter );
            }
        }

        void
        Model::eraseTransitive( fh_node subj )
        {
            StatementList_t sl;
            LG_RDF_D << "Model::eraseTransitive() subj:" << subj->toString() << endl;
            
            getTransitiveTree( subj, sl );
            LG_RDF_D << "Model::eraseTransitive() triples to remove count:" << sl.size() << endl;
            for( StatementList_t::iterator si = sl.begin(); si!=sl.end(); ++si )
            {
                LG_RDF_D << "Erasing subj:" << (*si)->getObject()->toString()
                         << " pred:" << (*si)->getPredicate()->toString()
                         << " obj:" << (*si)->getObject()->toString()
                         << endl;
                erase( *si );
            }
        }
        
        

        void
        Model::set( fh_node subj, fh_node pred, fh_node obj )
        {
            LG_RDF_D << "Model::set() obj:" << obj->toString() << endl;
            erase( subj, pred );
            insert( subj, pred, obj );
        }

        StatementList_t&
        Model::getTransitiveTree( fh_node subj, StatementList_t& ret )
        {
            cerr << "Model::getTransitiveTree() subj:" << tostr(subj) << endl;
            LG_RDF_D << "Model::getTransitiveTree() subj:" << tostr(subj) << endl;
            if( tostr(subj).empty() )
                return ret;
            
            NodeIterator end   = NodeIterator();
            NodeIterator piter = getArcsOut( subj );
            for( ; piter != end; ++piter )
            {
                LG_RDF_D << "Model::getTransitiveTree() pred:" << tostr(*piter) << endl;
                NodeIterator oiter = findObjects( subj, *piter );
                for( ; oiter != end; ++oiter )
                {
                    fh_statement st = new Statement( subj, *piter, *oiter );
                    ret.push_back( st );

                    LG_RDF_D << "Model::getTransitiveTree() st.o:" << tostr(subj)
                             << " st.p:" << tostr(*piter)
                             << " st.o:" << tostr(*oiter)
                             << endl;
                    if( tostr(*oiter).empty() )
                        continue;
                    if( tostr(*piter) == "http://witme.sf.net/libferris.web/rdf/ferris-attr/rdf-cache-ea-names" )
                        continue;
                    getTransitiveTree( *oiter, ret );
                }
            }
            return ret;
        }
        

        void
        Model::RDFMerge( fh_node source, fh_node target )
        {
            fh_node subj = source;
    
            NodeIterator end;
            NodeIterator piter = getArcsOut( subj );
            for( ; piter != end; ++piter )
            {
                NodeIterator oiter = findObjects( subj, *piter );
                for( ; oiter != end; ++oiter )
                {
                    fh_statement st = new Statement( target, *piter, *oiter );

                    NodeIterator io = findObjects( target, *piter );
                    if( io == end )
                    {
                        insert( st );
                    }
                }
            }
        }
        
        
        /****************************************/
        /****************************************/
        /****************************************/

        Parser::Parser( const std::string& parser_name,
                        const std::string& mime_type,
                        fh_uri type_uri )
            :
            m_parser( Soprano::PluginManager::instance()->discoverParserForSerialization( Soprano::SerializationRdfXml ) )
        {
            if( !m_parser )
            {
                LG_RDF_ER << "Failed to create parser!" << endl;
            }
        }

        Parser::~Parser()
        {
            if( m_parser )
                delete m_parser;
        }

        const Soprano::Parser*
        Parser::getRAW()
        {
            return m_parser;
        }

        fh_model
        Parser::ParseIntoModel( fh_model m, const std::string& s, fh_uri base_uri )
        {
            string base_uri_string = "";
            if( isBound( base_uri ) )
                base_uri_string = base_uri->toString();
            
            Soprano::StatementIterator it = m_parser->parseString( s.c_str(),
                                                                   QUrl( base_uri_string.c_str() ),
                                                                   Soprano::SerializationRdfXml );
            m->insert( StatementIterator( it ) );
            return m;
        }
    

        fh_model
        Parser::ParseIntoModel( fh_model m, fh_context c, fh_uri base_uri )
        {
            fh_istream iss = c->getIStream();
            return ParseIntoModel( m, iss, base_uri );
        }
        
        fh_model
        Parser::ParseIntoModel( fh_model m, fh_istream iss, fh_uri base_uri )
        {
            std::string s = StreamToString( iss );
            return ParseIntoModel( m, s, base_uri );
        }
    
        
        /****************************************/
        /****************************************/
        /****************************************/

        
        FERRISEXP_API fh_model getDefaultFerrisModel()
        {
            static fh_model m = 0;

            if( m )
                return m;

            
            //
            // Allow user customization of which storage to use for their RDF
            //
            bool usingCustomStore = false;
            try
            {
//                 fh_context rdfdbc = Resolve( getDotFerrisPath() + "rdfdb" );
//                 std::string storage_name = getStrSubCtx( rdfdbc, "ferris-storage-name", "" );
//                 std::string db_name      = getStrSubCtx( rdfdbc, "ferris-db-name", getDotFerrisPath() + "rdfdb/myrdf" );
//                 std::string db_options   = getStrSubCtx( rdfdbc, "ferris-db-options", "" );

//                 if( !storage_name.empty() )
//                 {
// //                     cerr << "storage_name:" << storage_name
// //                          << " db_name:" << db_name
// //                          << " db_options:" << db_options
// //                          << endl;
                
//                     usingCustomStore = !storage_name.empty();
//                     m = Model::FromMetadataContext( storage_name, db_name, db_options );
//                     return m;
//                 }
            }
            catch( exception& e )
            {
                if( usingCustomStore )
                    throw;
            }

//            fh_context dbc = Shell::acquireContext( getDotFerrisPath() + "rdf-soprano-db/metadata" );
            fh_context dbc = Shell::acquireContext( getDotFerrisPath() + "rdfdb/metadata" );
            m = Model::FromMetadataContext( dbc );
            return m;
        }
        
    };
};
