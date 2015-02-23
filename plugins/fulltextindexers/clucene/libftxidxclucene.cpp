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

    $Id: libftxidxclucene.cpp,v 1.5 2008/12/19 21:30:13 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>


#include <Ferris/FullTextIndexerMetaInterface.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FullTextIndexer_private.hh>
#include <Ferris/FerrisCLucene.hh>

#include <CLucene.h>
#include <CLucene/util/CLStreams.h>
#include <CLucene/analysis/standard/StandardTokenizer.h>
#include <CLucene/analysis/standard/StandardFilter.h>
#include <CLucene/index/IndexModifier.h>
extern size_t lucene_utf8towcs(wchar_t * result, const char * str, size_t result_length);
extern std::string lucene_wcstoutf8string(const wchar_t* str, size_t strlen);
//typedef wchar_t TCHAR;
typedef char char_t;

using namespace std;

using namespace lucene;
using namespace lucene::index;
using namespace lucene::document;
using namespace lucene::analysis;
using namespace lucene::analysis::standard;
using namespace lucene::search;
using namespace lucene::util;
using namespace lucene::queryParser;


namespace Ferris
{
//     wstring widen( const string& s )
//     {
//         wstring ret;
//         Util::utf8_to_wstring( ret, s );
//         return ret;
//     }
//     string narrow( const wstring& s )
//     {
//         string ret;
//         Util::wstring_to_utf8( ret, s );
//         return ret;
//     }


    
    static Term* newterm( std::string k, std::string v )
    {
        const int sz = 4096;
        TCHAR kt[sz];
        TCHAR vt[sz];

        lucene_utf8towcs( kt, k.c_str(), sz );
        lucene_utf8towcs( vt, v.c_str(), sz );

        return new Term( kt, vt );
    }
    static wstring W( const string& s )
    {
        wstring ret;
        return Util::utf8_to_wstring( ret, s );
    }
    static string N( const wstring& ws )
    {
        string ret;
        return Util::wstring_to_utf8( ret, ws );
    }
    
    static void add( lucene::document::Document* doc, std::string k, std::string v, long tweak )
    {
        const int sz = 4096;
        TCHAR kt[sz];
        const int vtsz = v.size()*3;
        TCHAR* vt = new TCHAR[vtsz];

        lucene_utf8towcs( kt, k.c_str(), sz );
        lucene_utf8towcs( vt, v.c_str(), vtsz );
        
        Field* f = new Field( kt, vt, tweak );
        doc->add( *f );
    }
    static void add( lucene::document::Document* doc, std::string k, std::string v )
    {
        add( doc, k, v, 
                 Field::STORE_YES
                 | Field::INDEX_UNTOKENIZED
                 | Field::INDEX_NONORMS
                 | Field::TERMVECTOR_NO
            );
    }
    
namespace FullTextIndex 
    {
        

        /****************************************/
        /****************************************/
        /****************************************/

        class FullTextIndexerCLucene
            :
            public MetaFullTextIndexerInterface
        {
            map< int, string > m_docIDHash;
            

            Analyzer* newAnalyzer();
            
        protected:

            l_IndexWriter m_writer;
            l_IndexWriter getIndexWriter();
            
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

            FullTextIndexerCLucene();
            ~FullTextIndexerCLucene();

            virtual void sync();
            virtual void prepareForWrites( int f );
            virtual void allWritesComplete();

            docNumSet_t&
            ExecuteLuceneFullTextQuery( const std::string& queryString,
                                        docNumSet_t& docnums,
                                        int limit );
            virtual void* BuildCLuceneQuery( const std::string& qstr, void* current_query );
            
            
            static MetaFullTextIndexerInterface* Create();

            virtual bool getIndexMethodSupportsIsFileNewerThanIndexedVersion()
                { return true; }
            virtual bool isFileNewerThanIndexedVersion( const fh_context& c );
        };

        /************************************************************/
        /************************************************************/
        /************************************************************/



        class MyAnalyzer
            :
            public Analyzer
        {
            bool caseSensitive;
            bool dropStopWords;
            long stemMode;
            list< string > stopTable;
            char** stopWords;

            list< wstring > wstopTable;
            typedef wchar_t wstopWordsCHAR;
            wstopWordsCHAR** wstopWords;


            char**
            stringvec_to_CSTRvec( const list<string>& v )
                {
                    int l = v.size();
                    int i = 0;

                    list<string>::const_iterator ci = v.begin();
                    char** ret = g_new0(char*, l+2 );
                    for( i=0; i < l; ++i )
                    {
                        ret[i] = g_strdup( ci->c_str());
                        ++ci;
                    }
        
                    ret[i] = 0;
                    return ret;
                }
            
        public:
            MyAnalyzer( bool caseSensitive,
                        bool dropStopWords,
                        long stemMode,
                        stringlist_t droplist )
                :
                caseSensitive( caseSensitive ),
                dropStopWords( dropStopWords ),
                stemMode( stemMode ),
                stopWords( 0 ),
                stopTable( droplist )
                {
                    stopWords = stringvec_to_CSTRvec( stopTable );
                        
                    for( stringlist_t::iterator si = droplist.begin(); si != droplist.end(); ++si )
                        wstopTable.push_back( W( *si ) );
                    
                    wstopWords = new wstopWordsCHAR*[ wstopTable.size()+2 ];
                    int i = 0;
                    for( list<wstring>::iterator si = wstopTable.begin(); si != wstopTable.end(); ++si )
                    {
                        wstopWords[ i ] = (wstopWordsCHAR*)si->c_str();
                        ++i;
                    }
                }

            // 0.9.x
            TokenStream* tokenStream(const TCHAR* fieldName, util::Reader* reader)
//            TokenStream& tokenStream(const char_t* fieldName, lucene::util::Reader* reader)
                {
                    TokenStream* ret = new StandardTokenizer( reader->__asBufferedReader() );
                    ret= new StandardFilter( ret, true );
                    if( !caseSensitive )
                        ret = new LowerCaseFilter( ret, true );

                    if( dropStopWords )
                    {
                        ret = new StopFilter( ret, true, (const wchar_t**)wstopWords ); //, stopTable.size() );
                    }
//                     if( stemMode == (jint)STEM_PORTER )
//                         ret = new PorterStemFilter( ret );

                    return ret;
                    
//                    return new LowerCaseTokenizer( reader );
//                    return new StopFilter(new LowerCaseTokenizer(reader), stopTable);
                }
        };

        /**************************************************/
        /**************************************************/

        Analyzer*
        FullTextIndexerCLucene::newAnalyzer()
        {
//            return new StandardAnalyzer();

            stringlist_t sl = Util::parseCommaSeperatedList( 
                getConfig( IDXMGR_STOPWORDSLIST_K, IDXMGR_STOPWORDSLIST_DEFAULT ) );
            return new MyAnalyzer( isCaseSensitive(),
                                   getDropStopWords(),
                                   getStemMode(), sl );
        }
        
        
        FullTextIndexerCLucene::FullTextIndexerCLucene()
            : m_writer(0)
        {
        }

        FullTextIndexerCLucene::~FullTextIndexerCLucene()
        {
            sync();
        }

        l_IndexWriter
        FullTextIndexerCLucene::getIndexWriter()
        {
            if( !m_writer )
            {
                m_writer = new IndexWriter( getPath().c_str(), newAnalyzer(), false );
                m_writer->setMaxFieldLength( 512*1024*1024 );
//                cerr << "IndexWriter::DEFAULT_MAX_FIELD_LENGTH:" << IndexWriter::DEFAULT_MAX_FIELD_LENGTH << endl;
            }
            return m_writer;
        }
        
        void
        FullTextIndexerCLucene::sync()
        {
            try
            {
                if( GetImpl(m_writer) )
                {
                    m_writer->optimize();
                    m_writer->close();
                    m_writer = 0;
                }
            }
            catch( CLuceneError& e )
            {
                if( e.number() == CL_ERR_AlreadyClosed )
                {
                    return;
                }
                cerr << "unexpoected error number:" << e.number() << endl;
                throw;
            }
            catch( ... )
            {
                throw;
            }
        }
        
        void
        FullTextIndexerCLucene::prepareForWrites( int f )
        {
        }
        
        void
        FullTextIndexerCLucene::allWritesComplete()
        {
            sync();
        }
        
        
        
        void
        FullTextIndexerCLucene::Setup()
        {
        }

        void
        FullTextIndexerCLucene::CreateIndexBeforeConfig( fh_context c,
                                                        bool caseSensitive,
                                                        bool dropStopWords,
                                                        StemMode stemMode,
                                                        const std::string& lex_class,
                                                        fh_context md )
        {
            try
            {
                Analyzer* a = new StandardAnalyzer();
                l_IndexWriter writer = new IndexWriter( getPath().c_str(), a, true );
                writer->close();
            }
            catch ( exception& e )
            {
                throw;
            }
        }
        
        
        void
        FullTextIndexerCLucene::CreateIndex( fh_context c,
                                            bool caseSensitive,
                                            bool dropStopWords,
                                            StemMode stemMode,
                                            const std::string& lex_class,
                                            fh_context md )
        {
        }

        
        void
        FullTextIndexerCLucene::addToIndex( fh_context c,
                                           fh_docindexer di )
        {
            LG_IDX_D << "addToIndex() c:" << c->getURL() << endl;

            string s;
            fh_istream iss;
            try
            {
                fh_attribute a = c->getAttribute( "as-text" );
                iss = a->getIStream();
            }
            catch( exception& e )
            {
                LG_IDX_W << "WARNING, Failed to obtain plaintext for url:" << c->getURL()
                         << " error:" << e.what() << endl;
                return;
            }

            streamsize bytesDone    = 0;
            streamsize signalWindow = 0;
            streamsize totalBytes   = toType<streamsize>(getStrAttr( c, "size", "0" ));
            SignalThrottle throttle = getSignalThrottle();
            bool isCaseSensitive = this->isCaseSensitive();

            lucene::document::Document* doc = new lucene::document::Document();

            l_IndexWriter writer = getIndexWriter();
            if( !getenv( "LIBFERRIS_INDEX_NO_REMOVE" ) )
                writer->deleteDocuments( newterm( "ferris-url", c->getURL() ) );
            
            // // out with the old.
            // {
            //     IndexModifier* writer = new IndexModifier( getPath().c_str(), newAnalyzer(), false );
            //     int32_t delCount = writer->deleteDocuments( newterm( "url", c->getURL() ) );
            //     writer->flush();
            //     writer->close();
            // }
            
            
//            l_IndexWriter writer = new IndexWriter( getPath().c_str(), newAnalyzer(), false );

            string mtime = getStrAttr( c, "mtime", "0" );
            add( doc, "url",  c->getURL() );
            add( doc, "ferris-url",  c->getURL() );
            add( doc, "ferris-path", c->getDirPath() );
            add( doc, "ferris-ftx-modified", mtime );

            
            {
                LG_IDX_D << "addToIndex() c:" << c->getURL() <<  endl;

                std::stringstream ss;
                string s;
                while( getline( iss, s ) )
                {
                    ss << s << "\n";
                }
//                 string contentstr = StreamToString( iss );
//                 cerr << "contentstr.sz:" << contentstr.size() << endl;
//                 str.append(contentstr.c_str());

                long tweak = Field::STORE_NO // STORE_COMPRESS
                    | Field::INDEX_TOKENIZED
//                    | Field::TERMVECTOR_YES;
                    | Field::TERMVECTOR_WITH_POSITIONS_OFFSETS;
                
                add( doc, "contents", ss.str(), tweak );
            }
            
            writer->addDocument( doc );
//            writer->optimize();
//            writer->close();
        }

        std::string
        FullTextIndexerCLucene::resolveDocumentID( docid_t id )
        {
            return m_docIDHash[ id ];
        }
        
        void
        FullTextIndexerCLucene::cleanDocumentIDCache()
        {
            m_docIDHash.clear();
        }

        docNumSet_t&
        FullTextIndexerCLucene::ExecuteLuceneFullTextQuery( const std::string& queryString,
                                                            docNumSet_t& docnums,
                                                            int limit )
        {
            LG_IDX_D << "ExecuteLuceneFullTextQuery() query:" << queryString << endl;
            addAllDocumentsMatchingTerm( queryString, docnums, limit );
            return docnums;
        }

        void*
        FullTextIndexerCLucene::BuildCLuceneQuery( const std::string& qstr, void* current_query_vp )
        {
            ::lucene::search::BooleanQuery* query = (BooleanQuery*)(current_query_vp);

            string path = getPath();
            Searcher* searcher = new IndexSearcher( path.c_str() );
            Analyzer* analyzer = newAnalyzer();

            Query* subq = QueryParser::parse( W(qstr).c_str(), _T("contents"), analyzer);
            query->add( subq, false, true, false );
            
            return query;
        }
        
        
        docNumSet_t&
        FullTextIndexerCLucene::addAllDocumentsMatchingTerm(
            const std::string& term_const,
            docNumSet_t& output,
            int limit )
        {
            try
            {
                
                string term = term_const;
                string path = getPath();
                Searcher* searcher = new IndexSearcher( path.c_str() );
                Analyzer* analyzer = newAnalyzer();

// FIXME: FIXME!                
//                 if( getStemMode() == STEM_PORTER )
//                 {
//                     PorterStemmer* s = new PorterStemmer();
//                     term = tostr( s->stem( tojstr( term ) ) );
//                 }
                if( getDropStopWords() )
                {
                    if( getStopWords().count( term_const ) )
                    {
                        cerr << "dropword! term:" << term_const << endl;
                        return output;
                    }
                }

                l_Query query = QueryParser::parse( W(term).c_str(), _T("contents"), analyzer);
                LG_IDX_D << "Searching for:" << term << endl;

                l_Hits hits = searcher->search( GetImpl(query) );
                LG_IDX_D << "total matching documents:" << hits->length() << endl;

                for (int i = 0; i < hits->length(); i++)
                {
                    lucene::document::Document doc = hits->doc(i);
                    int docID     = hits->id( i );
                    output.insert( docID );
                    m_docIDHash[ docID ] = N(doc.get(_T("ferris-url")));
                }
            }
            catch ( exception& e )
            {
                LG_IDX_D << "error:" << e.what() << endl;
                throw;
            }
        }
        
        /**************************************************/
        /**************************************************/

        bool
        FullTextIndexerCLucene::isFileNewerThanIndexedVersion( const fh_context& c )
        {
            bool ret = true;

            time_t ct = getTimeAttr( c, "mtime", 0 );
            if( !ct )
                return ret;

            LG_IDX_D << "isFileNewerThanIndexedVersion() c:" << c->getURL() << endl;


            
            l_Term t = newterm( "ferris-url", c->getURL() );
            l_TermQuery pq = new TermQuery( t );

            auto_ptr<Searcher> searcher( new IndexSearcher( getPath().c_str() ) );
            l_Hits hits = searcher->search( pq );

            LG_IDX_D << "hits:" << hits->length() << " c:" << c->getURL() << endl;
            if( hits->length() )
            {
                time_t cachetime = 0;

                lucene::document::Document doc = hits->doc(0);

                if( !doc.get(_T("ferris-ftx-modified")) )
                    return ret;
                
                string s = N(doc.get(_T("ferris-ftx-modified")));
                if( s.empty() )
                    return ret;
                
                cachetime = toType<time_t>( s );
                LG_IDX_D << " cachetime:" << cachetime << " currenttime:" << ct
                         << " ret:" << (cachetime < ct) << endl;
                return( cachetime < ct );
            }

            return ret;
        }
    };
};


extern "C"
{
    FERRISEXP_API Ferris::FullTextIndex::MetaFullTextIndexerInterface* Create()
    {
        return new Ferris::FullTextIndex::FullTextIndexerCLucene();
    }
};
