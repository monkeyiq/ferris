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

    $Id: FerrisRDFCore.hh,v 1.2 2010/09/24 21:30:41 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_RDF_CORE_H_
#define _ALREADY_INCLUDED_FERRIS_RDF_CORE_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <FerrisStreams/All.hh>

#include <Soprano/Node>
#include <Soprano/Statement>
#include <Soprano/Model>
#include <Soprano/StatementIterator>
#include <Soprano/NodeIterator>
#include <Soprano/Parser>
#include <Soprano/QueryResultIterator>

#include <iterator>

namespace Ferris
{
    namespace RDFCore
    {
        extern const std::string RDF_FERRIS_BASE;
        
        class StatementIterator;
        class NodeIterator;
        class BindingsIterator;
        class URI;
        class Parser;
        class Model;
        class Statement;
        class Node;
        FERRIS_SMARTPTR( URI,       fh_uri );
        FERRIS_SMARTPTR( Parser,    fh_parser );
        FERRIS_SMARTPTR( Model,     fh_model );
        FERRIS_SMARTPTR( Statement, fh_statement );
        FERRIS_SMARTPTR( Node,      fh_node );

        typedef std::list< fh_node > nodelist_t;
        
        /**
         * A list of statements
         */
        typedef std::list< fh_statement > StatementList_t;

        /**
         * Get the default model used for triples
         */
        FERRISEXP_API fh_model getDefaultFerrisModel();

        /****************************************/
        /****************************************/
        /****************************************/

        FERRISEXP_API bool operator==(const URI& __x, const URI& __y);
        FERRISEXP_API bool operator!=(const URI& __x, const URI& __y);
        class FERRISEXP_API URI
            :
            public Handlable
        {
            NOT_COPYABLE( URI );
            friend bool operator==(const URI& __x, const URI& __y);

            std::string m_uri;
            
        public:

            explicit URI( const std::string& uri );
            virtual ~URI();

            std::string toString() const;
            std::string getPath() const;
        };

        /****************************************/
        /****************************************/
        /****************************************/

        FERRISEXP_API bool operator==(const fh_node& __x, const fh_node& __y);
        FERRISEXP_API bool operator==(const Node& __x, const Node& __y);
        FERRISEXP_API bool operator!=(const Node& __x, const Node& __y);
        class FERRISEXP_API Node
            :
            public Handlable
        {
            Soprano::Node* m_node;
            friend class Model;
            friend class NodeIterator;
            friend class BindingsIterator;
            friend class Statement;
            
            Node( Soprano::Node* n );
            friend bool operator==(const Node& __x, const Node& __y);

        public:

            static fh_node CreateURI( const std::string& uri );
            static fh_node CreateLiteral( const std::string& s );
            static fh_node CreateLiteral( const std::wstring& s );
            static fh_node CreateBlankNode( const std::string& s );
            static fh_node CreateEmptyNode();
            
            virtual ~Node();
            Soprano::Node getRAW();
            
            fh_uri getURI();
            Soprano::Node::Type getType();

            std::string getLiteralValue();
            std::string getLiteralLang();
            std::string getBlankID();

            bool isResource();
            bool isLiteral();
            bool isLiteralXML();
            bool isBlank();

            std::string toString();
        };
        struct fh_node_compare
        {
            bool operator()( fh_node n1, fh_node n2 ) const
                {
                    return n1->toString() < n2->toString();
                }
        };

        /****************************************/
        /****************************************/
        /****************************************/

        FERRISEXP_API bool operator==(const Statement& __x, const Statement& __y);
        FERRISEXP_API bool operator!=(const Statement& __x, const Statement& __y);
        class FERRISEXP_API Statement
            :
            public Handlable
        {
            friend class StatementIterator;
            
            Soprano::Statement* m_statement;
            
            Statement( Soprano::Statement* m_statement );
            Statement( const Statement& );
            Statement& operator=( const Statement& );
            friend bool operator==(const Statement& __x, const Statement& __y);
            
        public:

            Statement();
            Statement( fh_node sub, fh_node pred, fh_node obj );
            
            virtual ~Statement();
            Soprano::Statement getRAW();

            void setSubject( fh_node n );
            void setPredicate( fh_node n );
            void setObject( fh_node n );

            fh_node getSubject();
            fh_node getPredicate();
            fh_node getObject();

            void clear();
            bool isComplete();
            std::string toString();
            bool match( fh_statement partial_st );

            std::string toNTripleString( fh_model m );
            std::string toRDFXMLString( fh_model m );
            std::string toXString( fh_model m, const std::string& format );
        };

        /****************************************/
        /****************************************/
        /****************************************/

        FERRISEXP_API bool operator==(const NodeIterator& __x, const NodeIterator& __y);
        FERRISEXP_API bool operator!=(const NodeIterator& __x, const NodeIterator& __y);
        class FERRISEXP_API NodeIterator
        {
            typedef NodeIterator _Self;
            friend class Model;
            friend class StatementIterator;
            friend bool operator==(const NodeIterator& __x, const NodeIterator& __y);
            
            char m_part;
            Soprano::NodeIterator m_base;
            
            void __advance();
            
        public:

            typedef std::forward_iterator_tag iterator_category;
            typedef fh_node value_type;
            typedef long    difference_type;
            typedef fh_node pointer;
            typedef fh_node reference;

            explicit NodeIterator( Soprano::NodeIterator base = Soprano::NodeIterator() );
            ~NodeIterator();
            reference operator*();
            _Self& operator++() {
                __advance();
                return *this;
            }
            _Self operator++(int) {
                _Self __tmp = *this;
                __advance();
                return __tmp;
            }
            
        };

        
        FERRISEXP_API bool operator==(const StatementIterator& __x, const StatementIterator& __y);
        FERRISEXP_API bool operator!=(const StatementIterator& __x, const StatementIterator& __y);
        class FERRISEXP_API StatementIterator
        {
            typedef StatementIterator _Self;
            friend bool operator==(const StatementIterator& __x, const StatementIterator& __y);
            
            friend class Model;
            friend class NodeIterator;
            friend class Parser;
            
            Soprano::StatementIterator m_iter;
            
        protected:
            
            void __advance();

        public:

            typedef std::forward_iterator_tag iterator_category;
            typedef fh_statement value_type;
            typedef long         difference_type;
            typedef fh_statement pointer;
            typedef fh_statement reference;

            explicit StatementIterator( Soprano::StatementIterator m_iter = Soprano::StatementIterator() );
            ~StatementIterator();
            reference operator*();
            _Self& operator++() {
                __advance();
                return *this;
            }
            _Self operator++(int) {
                _Self __tmp = *this;
                __advance();
                return __tmp;
            }

            NodeIterator iterateObjects () const;
            NodeIterator iteratePredicates () const;
            NodeIterator iterateSubjects () const;
        };


        FERRISEXP_API bool operator==(const BindingsIterator& __x, const BindingsIterator& __y);
        FERRISEXP_API bool operator!=(const BindingsIterator& __x, const BindingsIterator& __y);
        class FERRISEXP_API BindingsIterator
        {
            typedef BindingsIterator _Self;
            friend class Model;

            void __advance();
            Soprano::QueryResultIterator m_iter;
            
        public:

            typedef std::forward_iterator_tag iterator_category;
            typedef long    difference_type;
            
            BindingsIterator( Soprano::QueryResultIterator m_iter = Soprano::QueryResultIterator() );
            ~BindingsIterator();

            _Self& operator++() {
                __advance();
                return *this;
            }
            _Self operator++(int) {
                _Self __tmp = *this;
                __advance();
                return __tmp;
            }
            friend bool operator==(const BindingsIterator& __x, const BindingsIterator& __y);

            int 	bindingCount () const;
            stringlist_t bindingNames () const;
            fh_node operator[] (const std::string& name) const;
            fh_node operator[] (int idx) const;
        };
        
        

        /****************************************/
        /****************************************/
        /****************************************/

        class FERRISEXP_API Model
            :
            public Handlable
        {
            Soprano::Model* m_model;

            Model( Soprano::Model* m );

        public:

            /**
             * load RDF model from metadata given at md
             *
             * md can either be the main directory or the "metadata" subdir.
             */
            static fh_model FromMetadataContext( fh_context md );
            static fh_model FromMetadataContext( const std::string& earl );
            
            /**
             * load an existing RDF store from 'c'
             * in this case, 'c' will or should contain a "metadata" subdirectory
             *
             * optionally, for redland, you can specify the prefix for the db files.
             */
            static fh_model FromRedland( fh_context c, const std::string& dbname = "myrdf" );

            /**
             * load an existing RDF store from the string provided.
             */
            static fh_model FromXMLString( const std::string& rdfxml );

            /**
             * Merge some more RDF/XML with existing model
             */
            void MergeRDFXML( const std::string& rdfxml, fh_uri base_uri = 0 );

            static fh_model MemoryOnlyModel();
            virtual ~Model();

            fh_node       CreateBlankNode();
            
            void          sync();
            int           size();
            
            void          insert( fh_statement st );
            void          insert( StatementIterator begin,
                                  StatementIterator end = StatementIterator() );
            void          insert( fh_node subj, fh_node pred, fh_node obj );
            void          insert( fh_model m );

            StatementIterator begin();
            StatementIterator end();
            
            void       addStatement( fh_statement st );
            fh_ostream write( fh_ostream oss,
                              const std::string& serializerName = "",
                              const std::string& mimeType = "",
                              fh_uri type_uri = 0 );

            StatementIterator find( const std::string& query,
                                    const std::string& queryLanguage = "sparql" );
            BindingsIterator  findBindings( const std::string& query,
                                            const std::string& queryLanguage = "sparql" );
            
            StatementIterator findStatements( fh_statement partial_statement );
            NodeIterator      findSubjects(   fh_node pred, fh_node obj );
            NodeIterator      findPredicates( fh_node subj, fh_node obj );
            NodeIterator      findObjects(    fh_node subj, fh_node pred );
            fh_node           getSubject(     fh_node pred, fh_node obj );
            fh_node           getPredicate(   fh_node subj, fh_node obj );
            fh_node           getObject(      fh_node subj, fh_node pred );
            NodeIterator      getArcsIn(  fh_node n );
            NodeIterator      getArcsOut( fh_node n );
            bool              hasArcIn(   fh_node n, fh_node p );
            bool              hasArcOut(  fh_node n, fh_node p );
            
            bool contains( fh_statement st );
            void erase( fh_statement st );
            void erase( fh_model m );
            void erase( fh_node subj, fh_node pred, fh_node obj );
            void erase( fh_node subj, fh_node pred );
            void erase( fh_node subj, fh_node pred, NodeIterator objiter );
            /**
             * Get rid of all triples such as
             * subj _p1 _o1
             * and any triples with _o1 as subject, and any of those
             * where the object is the subject of another triple.
             */
             void eraseTransitive( fh_node subj );

            /**
             * if there is a list/value for subj+pred remove it
             * set the triple subj, pred, obj after this.
             */
            void set( fh_node subj, fh_node pred, fh_node obj );

            /**
             * Get a tree with all the triples reachable from a single subject.
             */
            StatementList_t& getTransitiveTree( fh_node subj, StatementList_t& ret );


            /**
             * Any triples (?s,?p,?o) starting with source=?s are copied to target=?t
             * (?t,?p,?o) only when (?t,?p,???) does not exist.
             *
             * This is not transative
             */
            void RDFMerge( fh_node source, fh_node target );
            
        };
        
        /****************************************/
        /****************************************/
        /****************************************/


        class FERRISEXP_API Parser
            :
            public Handlable
        {
            const Soprano::Parser* m_parser;

        public:
            Parser( const std::string& parser_name = "",
                    const std::string& mime_type = "",
                    fh_uri type_uri = 0 );
            
            virtual ~Parser();
            const Soprano::Parser* getRAW();
            
            fh_model ParseIntoModel( fh_model m, const std::string& s, fh_uri base_uri = 0 );
            fh_model ParseIntoModel( fh_model m, fh_context c,         fh_uri base_uri = 0 );
            fh_model ParseIntoModel( fh_model m, fh_istream iss,       fh_uri base_uri = 0 );
        };
        
        
        /****************************************/
        /****************************************/
        /****************************************/
        
    };
};
#endif
