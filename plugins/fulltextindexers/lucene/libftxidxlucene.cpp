/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

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

    $Id: libftxidxlucene.cpp,v 1.3 2008/12/19 21:30:13 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#pragma GCC java_exceptions

#include <Ferris/FullTextIndexerMetaInterface.hh>
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



#include <gcj/array.h>

//#include "FullTextIndexerLucene_private.hh"
//#include "FullTextIndexer_private.hh"
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
#include <java/io/FileInputStream.h>
#include <java/util/Hashtable.h>

#include <MyAnalyzer.h>
#include <Ferris/Ferris.hh>
#include <Ferris/FullTextIndexer_private.hh>

#include <string>


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

namespace Ferris
{
    namespace FullTextIndex 
    {
        using namespace ::Ferris::Java;
    
        class FullTextIndexerLucene
            :
            public MetaFullTextIndexerInterface
        {
            map< int, org::apache::lucene::document::Document* > m_docIDHash;
            
            void indexDocs( IndexWriter* writer, File* f );

            Analyzer* newAnalyzer();
            
        protected:
            
            virtual void Setup();
            virtual void CreateIndexBeforeConfig( fh_context c,
                                                  bool caseSensitive,
                                                  bool dropStopWords,
                                                  StemMode stemMode,
                                                  const std::string& lex_class,
                                                  fh_context md );
            virtual void CreateIndex( fh_context c,
                                      bool caseSensitive,
                                      bool dropStopWords,
                                      StemMode stemMode,
                                      const std::string& lex_class,
                                      fh_context md );
            virtual void addToIndex( fh_context c,
                                     fh_docindexer di );

            virtual docNumSet_t& addAllDocumentsMatchingTerm(
                const std::string& term,
                docNumSet_t& output,
                int limit );
            virtual void cleanDocumentIDCache();
            virtual std::string resolveDocumentID( docid_t );

        public:

            FullTextIndexerLucene();

            static MetaFullTextIndexerInterface* Create();
        };

        /************************************************************/
        /************************************************************/
        /************************************************************/


        typedef JArray<jstring>* jStringList;
        
////////////////        // Generates ICE on 3.3.2 20040119

        static jStringList
        tojarray( const stringlist_t& sl )
        {
            jobject empty = tojstr( "" );
            jclass  StringClass = empty->getClass();
            jobjectArray ret = JvNewObjectArray( sl.size(),
                                                 StringClass,
                                                 empty );
            jobject* raw = elements(ret);
            int i=0;
            for( stringlist_t::const_iterator si = sl.begin(); si!=sl.end(); ++si )
            {
                raw[ i ] = tojstr( *si );
                ++i;
            }
            return (JArray<jstring>*) ret;
        }

        
        /**************************************************/
        /**************************************************/

//         class MyAnalyzer
//             :
//             public Analyzer
//         {
//             jboolean caseSensitive;
//             jboolean dropStopWords;
//             jint stemMode;
//             ::java::util::Hashtable* stopTable;
            
//         public:
//             MyAnalyzer( jboolean caseSensitive,
//                         jboolean dropStopWords,
//                         jint stemMode,
//                         jStringList droplist = StopAnalyzer::ENGLISH_STOP_WORDS )
//                 :
//                 caseSensitive( caseSensitive ),
//                 dropStopWords( dropStopWords ),
//                 stemMode( stemMode )
//                 {
// //                    stopTable = StopFilter::makeStopTable( StopAnalyzer::ENGLISH_STOP_WORDS );
//                     stopTable = StopFilter::makeStopTable( droplist );
//                 }
            
//             TokenStream* tokenStream( jstring fieldName, Reader* reader )
//                 {
//                     TokenStream* ret = new StandardTokenizer( reader );
//                     ret = new StandardFilter( ret );
//                     if( !caseSensitive )
//                         ret = new LowerCaseFilter( ret );
//                     if( dropStopWords )
//                         ret = new StopFilter( ret, stopTable );
//                     if( stemMode == (jint)STEM_PORTER )
//                         ret = new PorterStemFilter( ret );

//                     return ret;
                    
// //                    return new LowerCaseTokenizer( reader );
// //                    return new StopFilter(new LowerCaseTokenizer(reader), stopTable);
//                 }
//         };

        /**************************************************/
        /**************************************************/

        Analyzer*
        FullTextIndexerLucene::newAnalyzer()
        {
//            return new StandardAnalyzer();

            // Using MyAnalyzer generates internal compiler error. 3.3.2 20040119
            stringlist_t sl = Util::parseCommaSeperatedList( 
                getConfig( IDXMGR_STOPWORDSLIST_K, IDXMGR_STOPWORDSLIST_DEFAULT ) );

//             cerr << "FullTextIndexerLucene::newAnalyzer() stem:" << getStemMode()
//                  << " (1<<2):" << (1<<2)
//                  << endl;
            
            return new MyAnalyzer( isCaseSensitive(),
                                   getDropStopWords(),
                                   getStemMode(),
                                   tojarray( sl ) );
        }
        
        
        FullTextIndexerLucene::FullTextIndexerLucene()
        {
        }
        
        
        void
        FullTextIndexerLucene::Setup()
        {
            ::Ferris::Factory::ensureJVMCreated();
        }

        void
        FullTextIndexerLucene::CreateIndexBeforeConfig( fh_context c,
                                                        bool caseSensitive,
                                                        bool dropStopWords,
                                                        StemMode stemMode,
                                                        const std::string& lex_class,
                                                        fh_context md )
        {
            ::Ferris::Factory::ensureJVMCreated();
            try
            {
                IndexWriter* writer = new IndexWriter( tojstr( getPath() ),
                                                       newAnalyzer(),
                                                       true );
//                writer->optimize();
                writer->close();
            }
            catch ( Throwable *t )
            {
                System::err->println(tojstr("Unhandled Java exception:"));
                t->printStackTrace();
            }
        }
        
        
        void
        FullTextIndexerLucene::CreateIndex( fh_context c,
                                            bool caseSensitive,
                                            bool dropStopWords,
                                            StemMode stemMode,
                                            const std::string& lex_class,
                                            fh_context md )
        {
        }

        void
        FullTextIndexerLucene::indexDocs( IndexWriter* writer, File* file )
        {
            if( file->isDirectory() )
            {
                JArray< ::java::lang::String *>* jfiles = file->list();
                ::java::lang::String ** files = elements( jfiles );
                for (int i = 0; i < jfiles->length; i++)
                    indexDocs(writer, new File( file, files[ i ] ));
            }
            else
            {
                System::out->println( tojstr("adding ") );
                System::out->println( file->toString() );

                org::apache::lucene::document::Document* doc = new
                    org::apache::lucene::document::Document();

                doc->add(Field::Text(tojstr("path"), file->getPath()));
                doc->add(Field::Keyword(tojstr("modified"),
                                        DateField::timeToString(file->lastModified())));

                FileInputStream* is     = new FileInputStream( file );
                Reader*          reader = new BufferedReader(new InputStreamReader(is));
                doc->add(Field::Text(tojstr("contents"), reader));
                writer->addDocument( doc );
            }
        }
        
        void
        FullTextIndexerLucene::addToIndex( fh_context c,
                                           fh_docindexer di )
        {
            ::Ferris::Factory::ensureJVMCreated();

            try
            {
                Date* start = new Date();
                
                IndexWriter* writer = new IndexWriter( tojstr(getPath()),
                                                       newAnalyzer(),
                                                       false );

                indexDocs( writer, new File( tojstr( c->getDirPath() )));
                
                writer->optimize();
                writer->close();

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

        std::string
        FullTextIndexerLucene::resolveDocumentID( docid_t id )
        {
            org::apache::lucene::document::Document* d = m_docIDHash[ id ];
            jstring jpath = d->get( tojstr("path") );
            if( !jpath )
                jpath = d->get( tojstr( "url" ) );
            return tostr( jpath );
        }
        
        void
        FullTextIndexerLucene::cleanDocumentIDCache()
        {
            m_docIDHash.clear();
        }
        
        docNumSet_t&
        FullTextIndexerLucene::addAllDocumentsMatchingTerm(
            const std::string& term_const,
            docNumSet_t& output,
            int limit )
        {
            try
            {
                
                string term = term_const;
                string path = getPath();
                cerr << "path:" << path << endl;
                Searcher* searcher = new IndexSearcher( tojstr(path) );
                Analyzer* analyzer = newAnalyzer();

                if( getStemMode() == STEM_PORTER )
                {
                    PorterStemmer* s = new PorterStemmer();
                    term = tostr( s->stem( tojstr( term ) ) );
                }
                if( getDropStopWords() )
                {
                    if( getStopWords().count( term_const ) )
                    {
                        cerr << "dropword! term:" << term_const << endl;
                        return output;
                    }
                }

                Query* query = QueryParser::parse(tojstr(term), tojstr("contents"), analyzer);
                System::out->println( tojstr("Searching for: "));
                System::out->println( query->toString( tojstr("contents") ));

                Hits* hits = searcher->search( query );
                System::out->println( hits->length() );
                System::out->println( tojstr(" total matching documents") );

                for (int i = 0; i < hits->length(); i++)
                {
                    org::apache::lucene::document::Document* doc = hits->doc(i);
                    int docID     = hits->id( i );
                    output.insert( docID );
                    m_docIDHash[ docID ] = doc;
                }
            }
            catch (Throwable *t)
            {
                System::err->println(tojstr("Unhandled Java exception:"));
                System::err->println( t->toString() );
                t->printStackTrace();
                if( Throwable* tc = t->getCause () )
                {
                    System::err->println(tojstr("Cause:"));
                    t->printStackTrace();
                }
                throw;
            }
        }
        
        /**************************************************/
        /**************************************************/

        
    };
};


extern "C"
{
    FERRISEXP_API Ferris::FullTextIndex::MetaFullTextIndexerInterface* Create()
    {
        return new Ferris::FullTextIndex::FullTextIndexerLucene();
    }
};
