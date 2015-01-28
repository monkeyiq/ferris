/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2004 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: libeaindexlucene.cpp,v 1.1 2006/12/07 06:49:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#pragma GCC java_exceptions

#include <gcj/array.h>

#include <FerrisGCJ/FerrisGCJ_private.hh>

#include <gcj/cni.h>
#include <java/lang/System.h>
#include <java/util/Date.h>
#include <java/util/HashMap.h>

#include <java/io/File.h>
#include <java/io/PrintStream.h>
#include <java/lang/Throwable.h>
#include <java/lang/Character.h>
#include <java/lang/StringBuffer.h>

#include <java/io/IOException.h>
#include <java/io/BufferedReader.h>
#include <java/io/InputStreamReader.h>
#include <java/io/FileInputStream.h>
#include <java/util/Hashtable.h>

#include <org/apache/lucene/document/Document.h>
#include <org/apache/lucene/document/Field.h>
#include <org/apache/lucene/document/DateField.h>
#include <org/apache/lucene/analysis/LowerCaseTokenizer.h>
#include <org/apache/lucene/analysis/LowerCaseFilter.h>
#include <org/apache/lucene/analysis/StopAnalyzer.h>
#include <org/apache/lucene/analysis/StopFilter.h>
#include <org/apache/lucene/analysis/standard/StandardTokenizer.h>
#include <org/apache/lucene/analysis/standard/StandardFilter.h>
#include <org/apache/lucene/analysis/PorterStemFilter.h>
#include <org/apache/lucene/analysis/PorterStemmer.h>
#include <org/apache/lucene/analysis/Analyzer.h>
#include <org/apache/lucene/analysis/standard/StandardAnalyzer.h>
#include <org/apache/lucene/document/Document.h>
#include <org/apache/lucene/search/Searcher.h>
#include <org/apache/lucene/search/IndexSearcher.h>
#include <org/apache/lucene/search/Query.h>
#include <org/apache/lucene/search/Hits.h>
#include <org/apache/lucene/queryParser/QueryParser.h>
#include <org/apache/lucene/queryParser/ParseException.h>
#include <org/apache/lucene/index/IndexWriter.h>
#include <org/apache/lucene/search/PhraseQuery.h>
#include <org/apache/lucene/search/WildcardQuery.h>
#include <org/apache/lucene/search/BooleanQuery.h>
#include <org/apache/lucene/search/RangeQuery.h>
#include <org/apache/lucene/index/Term.h>

using namespace std;

using namespace java::lang;
using namespace java::io;
using namespace java::util;
using namespace org::apache::lucene::index;
using namespace org::apache::lucene::document;
using namespace org::apache::lucene::search;
using namespace org::apache::lucene::analysis;
using namespace org::apache::lucene::queryParser;
using namespace org::apache::lucene::analysis::standard;

#include "EAIndexerMetaInterface.hh"
#include "EAIndexer.hh"
#include "EAQuery.hh"
#include "General.hh"

// guessComparisonOperatorFromData
#include "FilteredContext_private.hh"

#include <Ferris/Ferris.hh>
#include <limits>

namespace Ferris
{
    using namespace Java;

    static string padInteger( int v )
    {
        stringstream ss;
        ss << setfill('0') << setw(32) << v;
        return ss.str();
    }
    static string padInteger( const std::string& v )
    {
        return padInteger( toint( v ) );
    }
    
    static string padDouble( double v )
    {
        stringstream ss;
        double l = log10( v );
        if( l >= 28 )
            ss << setfill('0') << "huge" << setw(24) << setprecision(4) << fixed << l;
        else
            ss << setfill('0') << setw(28) << setprecision(4) << fixed << v;
        return ss.str();
    }
    static string padDouble( const std::string& v )
    {
        return padDouble( toType<double>( v ));
    }
    
    static bool isInt( const std::string& s )
    {
        return s == "int" || s == "long";
    }
    
    static bool isDouble( const std::string& s )
    {
        return s == "float" || s == "double";
    }
    static bool isNumeric( const std::string& s )
    {
        return isInt( s ) || isDouble( s );
    }
    

    static org::apache::lucene::index::Term*
    makeTerm(
        const std::string& eaname,
        const std::string& compop,
        const std::string& v )
    {
        if( isInt( compop ))
        {
            return new Term( tojstr( eaname ), tojstr( padInteger(v) ));
        }
        else if( isDouble( compop ))
        {
            return new Term( tojstr( eaname ), tojstr( padDouble(v) ));
        } 

        return new Term( tojstr( eaname ), tojstr( v ));
    }
    

    
    namespace EAIndex 
    {
        class EAIndexerLucene
            :
            public MetaEAIndexerInterface 
        {
            map< int, org::apache::lucene::document::Document* > m_docIDHash;

            string m_ubs;
            ::java::lang::String* m_jubs;

            string& //::java::lang::String*
            getUpperBoundString()
                {
                    if( !m_jubs )
                    {
                        m_ubs.reserve( getMaxValueSize()+1 );
                        for( int i=0; i<getMaxValueSize(); ++i )
                            m_ubs[i] = 0xFF;
                    
                        m_jubs = tojstr( m_ubs );
                    }
//                    return m_jubs;
                    return m_ubs;
                }

            IndexWriter* m_writer;
            IndexWriter* getIndexWriter()
                {
                    if( !m_writer )
                    {
                        m_writer = new IndexWriter( tojstr(getPath()),
                                                    newAnalyzer(),
                                                    false );
                    }
                    return m_writer;
                }
            
            
        protected:
            virtual void Setup();
            virtual void CreateIndexBeforeConfig( fh_context c,
                                                  fh_context md );
            void CreateIndex( fh_context c, fh_context md );
            virtual void CommonConstruction();

        public:
            EAIndexerLucene();
            virtual ~EAIndexerLucene();

            virtual void sync();

            virtual void addToIndex( fh_context c,
                                     fh_docindexer di );

            void BuildQueryLTE( BooleanQuery* query,
                                const std::string& eaname,
                                const std::string& cop,
                                const std::string& v );
            void BuildQueryGRE( BooleanQuery* query,
                                const std::string& eaname,
                                const std::string& cop,
                                const std::string& v );
            virtual BooleanQuery* BuildQuery( fh_context q,
                                              docNumSet_t& output,
                                              fh_eaquery qobj,
                                              BooleanQuery* lq );
            virtual docNumSet_t& ExecuteQuery( fh_context q,
                                               docNumSet_t& output,
                                               fh_eaquery qobj,
                                               int limit = 0 );
            virtual std::string resolveDocumentID( docid_t );
            virtual void cleanDocumentIDCache();

            /**
             * We don't really use the analyzer in this module,
             * we are mainly interested in hanging metadata off
             * URL objects.
             */
            Analyzer* newAnalyzer();
        };

        
        /************************************************************/
        /************************************************************/
        /************************************************************/

        EAIndexerLucene::EAIndexerLucene()
            :
            m_jubs( 0 ),
            m_writer( 0 )
        {
        }


        EAIndexerLucene::~EAIndexerLucene()
        {
            sync();
        }

        Analyzer*
        EAIndexerLucene::newAnalyzer()
        {
            return new StandardAnalyzer();
        }
        
        void
        EAIndexerLucene::Setup()
        {
            ::Ferris::Factory::ensureJVMCreated();
        }

        void
        EAIndexerLucene::CreateIndexBeforeConfig( fh_context c,
                                                  fh_context md )
        {
            ::Ferris::Factory::ensureJVMCreated();
            try
            {
                IndexWriter* writer = new IndexWriter( tojstr( getPath() ),
                                                       newAnalyzer(),
                                                       true );
                writer->close();
            }
            catch (Throwable *t)
            {
                System::err->println(tojstr("Unhandled Java exception:"));
                t->printStackTrace();
            }
        }
        
        void
        EAIndexerLucene::CreateIndex( fh_context c,
                                    fh_context md )
        {
        }
        
        void
        EAIndexerLucene::CommonConstruction()
        {
        }
        
        
        void
        EAIndexerLucene::sync()
        {
            if( m_writer )
            {
                m_writer->optimize();
                m_writer->close();
                m_writer = 0;
            }
        }
        

        
        void
        EAIndexerLucene::addToIndex( fh_context c,
                                     fh_docindexer di )
        {
            ::Ferris::Factory::ensureJVMCreated();

            try
            {
                Date* start = new Date();
                
                IndexWriter* writer = getIndexWriter();

                org::apache::lucene::document::Document* doc = new
                    org::apache::lucene::document::Document();

                typedef AttributeCollection::AttributeNames_t ant;
                ant an;
                c->getAttributeNames( an );
                for( ant::iterator it = an.begin(); it!=an.end(); ++it )
                {
                    string attributeName = *it;
                    string v;

                    if( !obtainValueIfShouldIndex( c, di,
                                                   attributeName, v ))
                        continue;

                    fh_context sc = c->getSchema( attributeName );
                    string comparisonOperator = getSchemaDefaultSort( sc );
                    if( isInt( comparisonOperator ))
                    {
//                        cerr << "Adding k:" << *it << " v:" << padInteger( toint( v )) << endl;
                        doc->add(
                            Field::Keyword( tojstr( attributeName ),
                                            tojstr( padInteger( toint( v )) )
                                ));
                    }
                    else if( isDouble( comparisonOperator ))
                    {
                        doc->add(
                            Field::Keyword( tojstr( attributeName ),
                                            tojstr( padDouble( v ) )
                                ));
                    }
                    else
                    {
                        doc->add(
                            Field::Keyword( tojstr( attributeName ),
                                            tojstr( v )
                                ));
                    }
                }
                
                writer->addDocument( doc );

                Date* end = new Date();
                System::out->println(tojstr(end->getTime() - start->getTime()));
                System::out->println(tojstr(" total milliseconds"));
            }
            catch (Throwable *t)
            {
                System::err->println(tojstr("Unhandled Java exception:"));
                t->printStackTrace();
            }
        }

        typedef org::apache::lucene::index::Term Term;



        void
        EAIndexerLucene::BuildQueryLTE( BooleanQuery* query,
                                        const std::string& eaname,
                                        const std::string& cop,
                                        const std::string& v )
        {
            Term* currentTerm = makeTerm( eaname, cop, v );
            Term* lowerBound  = isNumeric( cop )
                ? makeTerm( eaname, cop, "0" )
                : makeTerm( eaname, cop, "" );
            
            RangeQuery* pq = new RangeQuery( lowerBound, currentTerm, true );
            query->add( pq, true, false );
        }
        

        void
        EAIndexerLucene::BuildQueryGRE( BooleanQuery* query,
                                        const std::string& eaname,
                                        const std::string& cop,
                                        const std::string& v )
        {
            Term* currentTerm = makeTerm( eaname, cop, v );
            Term* upperBound  = 0;
            if( isInt( cop ))
                upperBound = makeTerm( eaname,
                                       cop,
                                       padInteger(numeric_limits<int>::max()) );
            else if( isDouble( cop ))
                upperBound = makeTerm( eaname,
                                       cop,
                                       padDouble(numeric_limits<double>::max()) );
            else
                upperBound = makeTerm( eaname,
                                       cop,
                                       getUpperBoundString() );
                
            RangeQuery* pq = new RangeQuery( currentTerm, upperBound, true );
            query->add( pq, true, false );
        }
    
        
        BooleanQuery*
        EAIndexerLucene::BuildQuery( fh_context q,
                                     docNumSet_t& output,
                                     fh_eaquery qobj,
                                     BooleanQuery* query )
        {
            
            string token   = getStrAttr( q, "token", "" );
            string tokenfc = foldcase( token );
            fh_attribute orderedtla = q->getAttribute("in-order-insert-list");
            fh_istream   orderedtls = orderedtla->getIStream();
            
            cerr << "ExecuteQuery() token:" << tokenfc << endl;
            cerr << " q.child count:" << q->SubContextCount() << endl;

            string s;
            getline( orderedtls, s );
            cerr << "ExecuteQuery() lc:" << s << endl;
            fh_context lc = q->getSubContext( s );

            if( tokenfc == "!" )
            {
                for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                {
                    BooleanQuery* subq = new BooleanQuery();
                    BuildQuery( *ci, output, qobj, subq );
                    query->add( subq, false, true );
                }
                return query;
            }

            getline( orderedtls, s );
            cerr << "EAQuery_Heur::ExecuteQuery() rc:" << s << endl;
            fh_context rc = q->getSubContext( s );            

            if( tokenfc == "&" )
            {
                for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                {
                    BooleanQuery* subq = new BooleanQuery();
                    BuildQuery( *ci, output, qobj, subq );
                    query->add( subq, true, false );
                }
                return query;
            }
            else if( tokenfc == "|" )
            {
                for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                {
                    BooleanQuery* subq = new BooleanQuery();
                    BuildQuery( *ci, output, qobj, subq );
                    query->add( subq, false, false );
                }
                return query;
            }
            

            string eaname = getStrAttr( lc, "token", "" );
            string value  = getStrAttr( rc, "token", "" );
            string comparisonOperator = guessComparisonOperatorFromData( value );
            
            if( tokenfc == "==" )
            {
                PhraseQuery* pq = new PhraseQuery();
                pq->add( makeTerm( eaname, comparisonOperator, value ) );
                query->add( pq, true, false );
            }
            else if( tokenfc == "=?=" )
            {
                BooleanQuery* subq = new BooleanQuery();
                
                PhraseQuery* pq = new PhraseQuery();
                pq->add( makeTerm( eaname, comparisonOperator, value ) );
                subq->add( pq, false, false );

                pq = new PhraseQuery();
                pq->add( makeTerm( eaname, comparisonOperator,
                                   tostr(convertStringToInteger(value) ) ));
                subq->add( pq, false, false );
                query->add( subq, true, false );
                
//                 if( isInt( comparisonOperator ))
//                 {
//                     cerr << "searching eaname:" << eaname
//                          << " v:" << padInteger( toint( value )) << endl;
//                     pq->add( new Term( tojstr( eaname ),
//                                        tojstr( padInteger( value ) ) ));
//                 }
//                 else if( isDouble( comparisonOperator ))
//                 {
//                     pq->add( new Term( tojstr( eaname ),
//                                        tojstr( padDouble( value ) ) ));
//                 }
//                 else
//                 {
//                     pq->add( new Term( tojstr( eaname ),
//                                        tojstr( value ) ));
//                 }
//                 query->add( pq, true, false );
            }
            else if( tokenfc == "=~" )
            {
                WildcardQuery* pq = new WildcardQuery(
                    makeTerm( eaname, comparisonOperator, value ) );
                query->add( pq, true, false );
            }
            else if( tokenfc == "<=" )
            {
                BuildQueryLTE( query, eaname, comparisonOperator, value );
                
//                 Term* currentTerm = makeTerm( eaname, comparisonOperator, value );
//                 Term* lowerBound  = isNumeric( comparisonOperator )
//                     ? makeTerm( eaname, comparisonOperator, "0" )
//                     : makeTerm( eaname, comparisonOperator, "" );
                
// //                 Term* currentTerm = new Term( tojstr( eaname ),
// //                                               tojstr( value ) );
// //                 Term* lowerBound = 0;
// //                 if( isInt( comparisonOperator ))
// //                 {
// //                     cerr << "searching eaname:" << eaname
// //                          << " v:" << padInteger( toint( value )) << endl;
// //                     lowerBound = new Term( tojstr( eaname ),
// //                                            tojstr( padInteger(0) ));
// //                 }
// //                 else if( isDouble( comparisonOperator ))
// //                 {
// //                     lowerBound = new Term( tojstr( eaname ),
// //                                            tojstr( padDouble(0) ));
// //                 } 
// //                 else
// //                 {
// //                     lowerBound = new Term( tojstr( eaname ),
// //                                            tojstr( "" ));
// //                 }
                
//                 RangeQuery* pq = new RangeQuery( lowerBound, currentTerm, true );
//                 query->add( pq, true, false );
            }
            else if( tokenfc == "<?=" )
            {
                BooleanQuery* tq = 0;
                BooleanQuery* subq = new BooleanQuery();
                
                tq = new BooleanQuery();
                BuildQueryLTE( tq, eaname, "int", tostr(convertStringToInteger(value)) );
                subq->add( tq, false, false );

                tq = new BooleanQuery();
                BuildQueryLTE( tq, eaname, "double", tostr(convertStringToInteger(value)) );
                subq->add( tq, false, false );

                tq = new BooleanQuery();
                BuildQueryLTE( tq, eaname, "string", value );
                subq->add( tq, false, false );

                query->add( subq, true, false );
                

                
//                 Term* currentTerm = new Term( tojstr( eaname ),
//                                               tojstr( value ) );
//                 query->add(
//                     new RangeQuery( new Term( tojstr( eaname ), tojstr( padInteger(0) )),
//                                     currentTerm, true ), true, false );
//                 query->add(
//                     new RangeQuery( new Term( tojstr( eaname ), tojstr( padDouble(0) )),
//                                     currentTerm, true ), true, false );
//                 query->add(
//                     new RangeQuery( new Term( tojstr( eaname ), tojstr( "" )),
//                                     currentTerm, true ), true, false );
            }
            else if( tokenfc == ">=" )
            {
                BuildQueryGRE( query, eaname, comparisonOperator, value );
                
//                 Term* currentTerm = makeTerm( eaname, comparisonOperator, value );
//                 Term* upperBound  = 0;
//                 if( isInt( comparisonOperator))
//                     upperBound = makeTerm( eaname,
//                                            comparisonOperator,
//                                            padInteger(numeric_limits<int>::max()) );
//                 else if( isDouble( comparisonOperator ))
//                     upperBound = makeTerm( eaname,
//                                            comparisonOperator,
//                                            padDouble(numeric_limits<double>::max()) );
//                 else
//                     upperBound = makeTerm( eaname,
//                                            comparisonOperator,
//                                            getUpperBoundString() );
                
//                 RangeQuery* pq = new RangeQuery( currentTerm, upperBound, true );
//                 query->add( pq, true, false );
            }
            else if( tokenfc == ">?=" )
            {
                BooleanQuery* tq = 0;
                BooleanQuery* subq = new BooleanQuery();
                
                tq = new BooleanQuery();
                BuildQueryGRE( tq, eaname, "int", tostr(convertStringToInteger(value)) );
                subq->add( tq, false, false );

                tq = new BooleanQuery();
                BuildQueryGRE( tq, eaname, "double", tostr(convertStringToInteger(value)) );
                subq->add( tq, false, false );

                tq = new BooleanQuery();
                BuildQueryGRE( tq, eaname, "string", value );
                subq->add( tq, false, false );

                query->add( subq, true, false );
                
//                 BuildQueryGRE( query, eaname, "int", value );
//                 BuildQueryGRE( query, eaname, "double", value );
//                 BuildQueryGRE( query, eaname, "string", value );
            }
            else
            {
                cerr << "WARNING: Lucene index can not resolve operation:" << tokenfc << endl;
            }
            return query;
        }
        

        
        // FIXME: NOT COMPLETE!!
        docNumSet_t&
        EAIndexerLucene::ExecuteQuery( fh_context q,
                                       docNumSet_t& output,
                                       fh_eaquery qobj,
                                       int limit )
        {
            if( limit )
            {
                stringstream ss;
                ss << "EAIndexerLucene::ExecuteQuery() limit not currently supported for Lucene backend" << endl;
                cerr << tostr(ss);
                LG_EAIDX_I << tostr(ss);
            }
            
            fh_stringstream qss;

            BooleanQuery* query = new BooleanQuery();
            LG_EAIDX_D << "ExecuteQuery() building lucene query " << endl;
            query = BuildQuery( q, output, qobj, query );
            LG_EAIDX_D << "ExecuteQuery() created lucene query " << endl;

//             Analyzer* analyzer = newAnalyzer();
//             Query* query = QueryParser::parse( tojstr( tostr( qss )),
//                                                tojstr(""),
//                                                analyzer );
            Searcher* searcher = new IndexSearcher( tojstr(getPath()) );
            LG_EAIDX_D << "ExecuteQuery() getting hits " << endl;
            Hits* hits = searcher->search( query );
            
            LG_EAIDX_D << "ExecuteQuery() adding hits " << endl;
            long length = hits->length();
            LG_EAIDX_D << "ExecuteQuery() adding hits hits->length():" << length << endl;
            
            System::out->println( hits->length() );
            System::out->println( tojstr(" total matching documents") );

            for (int i = 0; i < hits->length(); i++)
            {
                org::apache::lucene::document::Document* doc = hits->doc(i);
                int docID     = hits->id( i );
                addDocID( output, docID );
                m_docIDHash[ docID ] = doc;
            }
            
            return output;
        }
        
        std::string
        EAIndexerLucene::resolveDocumentID( docid_t id )
        {
            org::apache::lucene::document::Document* d = m_docIDHash[ id ];
            jstring jpath = d->get( tojstr("path") );
            if( !jpath )
                jpath = d->get( tojstr( "url" ) );
            return tostr( jpath );
        }

        void
        EAIndexerLucene::cleanDocumentIDCache()
        {
            m_docIDHash.clear();
        }
            
        
        
    };
};


extern "C"
{
    FERRISEXP_API Ferris::EAIndex::MetaEAIndexerInterface* Create()
    {
        return new Ferris::EAIndex::EAIndexerLucene();
    }
};


