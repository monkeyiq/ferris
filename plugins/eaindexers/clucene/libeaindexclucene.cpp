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

    $Id: libeaindexclucene.cpp,v 1.2 2006/12/07 06:49:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "EAIndexerMetaInterface.hh"
#include "EAIndexer.hh"
#include "EAQuery.hh"
#include "General.hh"

// guessComparisonOperatorFromData
#include "FilteredContext_private.hh"

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisBoost.hh>
#include <limits>
#include <Ferris/FerrisCLucene.hh>

#include <CLucene.h>
#include <CLucene/search/WildcardQuery.h>
extern size_t lucene_utf8towcs(wchar_t * result, const char * str, size_t result_length);
extern std::string lucene_wcstoutf8string(const wchar_t* str, size_t strlen);

using namespace std;
using namespace lucene;
using namespace lucene::index;
using namespace lucene::document;
using namespace lucene::analysis;
using namespace lucene::analysis::standard;
using namespace lucene::search;
using namespace lucene::queryParser;

#define URL_REGEX_WORD_LENGTH 20

namespace Ferris
{
    static bool regexHasNoSpecialChars( const std::string&s )
    {
        static boost::regex rex("^[a-zA-Z0-9 '\\\"]*$");
        boost::smatch matches;
        if( boost::regex_match( s, matches, rex ))
        {
            return true;
        }
        return false;
    }

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
    
//     static bool isInt( const std::string& s )
//     {
//         return s == "int" || s == "long";
//     }
    static bool isInt( EAIndex::MetaEAIndexerInterface::AttrType_t att )
    {
        return EAIndex::MetaEAIndexerInterface::ATTRTYPEID_INT;
    }
    
//     static bool isDouble( const std::string& s )
//     {
//         return s == "float" || s == "double";
//     }
    static bool isDouble( EAIndex::MetaEAIndexerInterface::AttrType_t att )
    {
        return EAIndex::MetaEAIndexerInterface::ATTRTYPEID_DBL;
    }
    
    static bool isNumeric( EAIndex::MetaEAIndexerInterface::AttrType_t att )
    {
        return EAIndex::MetaEAIndexerInterface::ATTRTYPEID_INT
            || EAIndex::MetaEAIndexerInterface::ATTRTYPEID_DBL;
    }

    static Term* newterm( std::string k, std::string v )
    {
        const int sz = 4096;
        TCHAR kt[sz];
        TCHAR vt[sz];

        lucene_utf8towcs( kt, k.c_str(), sz );
        lucene_utf8towcs( vt, v.c_str(), sz );

        return new Term( kt, vt );
    }
    
    static void add( lucene::document::Document* doc, std::string k, std::string v )
    {
        const int sz = 4096;
        TCHAR kt[sz];
        TCHAR vt[sz];

        lucene_utf8towcs( kt, k.c_str(), sz );
        lucene_utf8towcs( vt, v.c_str(), sz );
        
        Field* f = new Field( kt, vt,
                 Field::STORE_YES
                 | Field::INDEX_UNTOKENIZED
                 | Field::INDEX_NONORMS
                 | Field::TERMVECTOR_NO
            );
        doc->add( *f );
    }

    std::string STR( const TCHAR* v )
    {
        int len = 0;
        for( const TCHAR* p = v; *p; ++p )
            ++len;
        std::string ret = lucene_wcstoutf8string( v, len );
        return ret;
    }
    

//     static lucene::index::Term*
//     makeTerm(
//         const std::string& eaname,
//         const std::string& compop,
//         const std::string& v )
//     {
//         if( isInt( compop ))
//         {
//             return new Term( eaname.c_str(), padInteger(v).c_str() );
//         }
//         else if( isDouble( compop ))
//         {
//             return new Term( eaname.c_str(), padDouble(v).c_str() );
//         } 

//         return new Term( eaname.c_str(), v.c_str() );
//     }
    static lucene::index::Term*
    makeTerm(
        const std::string& eaname,
        EAIndex::MetaEAIndexerInterface::AttrType_t att,
        const std::string& v )
    {
        LG_EAIDX_D << "makeTerm() eaname:" << eaname << " type:" << att
                   << " int-type:" << EAIndex::MetaEAIndexerInterface::ATTRTYPEID_INT
                   << " v:" << v << endl;
        if( att == EAIndex::MetaEAIndexerInterface::ATTRTYPEID_INT)
        {
            LG_EAIDX_D << "makeTerm() padded as int:" << padInteger(v) << endl;
        }
        
        switch( att )
        {
        case EAIndex::MetaEAIndexerInterface::ATTRTYPEID_INT:
            return newterm( eaname, padInteger(v) );
        case EAIndex::MetaEAIndexerInterface::ATTRTYPEID_TIME:
            return newterm( eaname, padInteger(v) );
        case EAIndex::MetaEAIndexerInterface::ATTRTYPEID_DBL:
            return newterm( eaname, padDouble(v) );
        }
        
        return newterm( eaname, v );
    }
    

    
    namespace EAIndex 
    {
        class EAIndexerCLucene
            :
            public MetaEAIndexerInterface 
        {
            map< int, string > m_docIDHash;

            string m_ubs;

            string& //::java::lang::String*
            getUpperBoundString()
                {
                    if( m_ubs.empty() )
                    {
                        m_ubs.reserve( getMaxValueSize()+1 );
                        for( int i=0; i<getMaxValueSize(); ++i )
                            m_ubs[i] = 0xFF;
                    
                    }
                    return m_ubs;
                }

            l_IndexWriter m_writer;
            l_IndexWriter getIndexWriter()
                {
                    if( !m_writer )
                    {
                        m_writer = new IndexWriter( getPath().c_str(), newAnalyzer(), false );
                    }
                    return m_writer;
                }

            Searcher* m_SearcherFor_isFileNewerThanIndexedVersion;
            Searcher* getSearcherFor_isFileNewerThanIndexedVersion()
            {
                if( !m_SearcherFor_isFileNewerThanIndexedVersion )
                {
                    m_SearcherFor_isFileNewerThanIndexedVersion = new IndexSearcher( getPath().c_str() );
                }
                return m_SearcherFor_isFileNewerThanIndexedVersion;
            }
            
            
        protected:
            virtual void Setup();
            virtual void CreateIndexBeforeConfig( fh_context c,
                                                  fh_context md );
            void CreateIndex( fh_context c, fh_context md );
            virtual void CommonConstruction();

        public:
            EAIndexerCLucene();
            virtual ~EAIndexerCLucene();

            virtual void sync();

            virtual void addToIndex( fh_context c,
                                     fh_docindexer di );

            void BuildQueryLTE( BooleanQuery* query,
                                const std::string& eaname,
                                AttrType_t att,
                                const std::string& v );
            void BuildQueryGRE( BooleanQuery* query,
                                const std::string& eaname,
                                AttrType_t att,
                                const std::string& v );
            BooleanQuery* BuildQuery( fh_context q,
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

            
            virtual bool getIndexMethodSupportsIsFileNewerThanIndexedVersion()
                { return true; }
            virtual bool isFileNewerThanIndexedVersion( const fh_context& c );
        };

        
        /************************************************************/
        /************************************************************/
        /************************************************************/

        EAIndexerCLucene::EAIndexerCLucene()
            : m_writer( 0 )
            , m_SearcherFor_isFileNewerThanIndexedVersion( 0 )
        {
        }


        EAIndexerCLucene::~EAIndexerCLucene()
        {
            sync();
        }

        Analyzer*
        EAIndexerCLucene::newAnalyzer()
        {
            return new StandardAnalyzer();
        }
        
        void
        EAIndexerCLucene::Setup()
        {
        }

        void
        EAIndexerCLucene::CreateIndexBeforeConfig( fh_context c,
                                                   fh_context md )
        {
            try
            {
                bool create = isFalse( getStrSubCtx( md, "db-exists", "0" ));
                
                IndexWriter* writer = new IndexWriter( getPath().c_str(), newAnalyzer(), create );
                writer->close();
            }
            catch (exception& e)
            {
                throw;
            }
        }
        
        void
        EAIndexerCLucene::CreateIndex( fh_context c,
                                    fh_context md )
        {
        }
        
        void
        EAIndexerCLucene::CommonConstruction()
        {
        }
        
        
        void
        EAIndexerCLucene::sync()
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
        EAIndexerCLucene::addToIndex( fh_context c,
                                      fh_docindexer di )
        {
            try
            {
                LG_EAIDX_D << "addToIndex() c:" << c->getURL() << endl;

                lucene::document::Document* doc = new lucene::document::Document();

                // {
                //     l_IndexReader writer = IndexReader::open( getPath().c_str() );

                //     {
                //         stringset_t retainnames;
                //         retainnames.insert("contents");
                //         retainnames.insert("ferris-ftx-modified");

                //         l_Term t = newterm( "ferris-url", c->getURL() );
                //         l_TermQuery pq = new TermQuery( t );
                //         auto_ptr<Searcher> searcher( new IndexSearcher( getPath().c_str() ) );
                //         l_Hits hits = searcher->search( pq );
                //         LG_EAIDX_D << "Found old docid hits:" << hits->length() << endl;
                //         if( hits->length() )
                //         {
                //             lucene::document::Document& olddoc = hits->doc(0);

                //             LG_EAIDX_D << "has contents:" << (olddoc.get( _T("contents") )!=0) << endl;
                //             LG_EAIDX_D << "olddoc.tostring:" << olddoc.toString() << endl;
                //             DocumentFieldEnumeration* iter = olddoc.fields();
                //             while( iter->hasMoreElements() )
                //             {
                //                 Field* f = iter->nextElement();
                //                 LG_EAIDX_D << "found f:" << f->name() << endl;
                //                 if( retainnames.count( (const char*)f->name() ) )
                //                 {
                //                     LG_EAIDX_D << "Retaining f:" << f->name() << endl;
                //                     doc->add( *f );
                //                 }
                //             }
                //         }
                //     }
                    

                    
                //     int32_t delCount = writer->deleteDocuments(
                //         newterm( "ferris-url", c->getURL() ) );
                //     writer->close();
                // }

                
                l_IndexWriter writer = getIndexWriter();
                if( !getenv( "LIBFERRIS_INDEX_NO_REMOVE" ) )
                    writer->deleteDocuments( newterm( "ferris-url", c->getURL() ) );

//                typedef AttributeCollection::AttributeNames_t ant;
//                ant an;
//                c->getAttributeNames( an );
                stringlist_t an;
                getEANamesToIndex( c, an );
                int totalAttributes = an.size();

                for( stringlist_t::iterator it = an.begin(); it!=an.end(); ++it )
                {
                    string attributeName = *it;
                    string v;

                    if( !obtainValueIfShouldIndex( c, di,
                                                   attributeName, v ))
                        continue;

                    LG_EAIDX_D << "addToIndex() adding k:" << attributeName << endl;
                    cerr << "addToIndex() adding k:" << attributeName << endl;

                    try
                    {
                        IndexableValue iv  = getIndexableValue( c, attributeName, v );
                        AttrType_t att = iv.getAttrTypeID();

                        if( isInt( att ))
                        {
//                        cerr << "Adding k:" << *it << " v:" << padInteger( toint( v )) << endl;
                            add( doc, attributeName, padInteger( toint( v )) );
                            
                            //  doc->add( 
                                // *Field::Keyword( (char*)attributeName.c_str(),
                                //                 padInteger( toint( v )).c_str()
                                //     ));
                        }
                        else if( isDouble( att ))
                        {
                            add( doc, attributeName, padDouble( v ) );
                            
                            // doc->add(
                            //     *Field::Keyword( (char*)attributeName.c_str(),
                            //                     padDouble( v ).c_str()
                            //         ));
                        }
                        else
                        {
                            add( doc, attributeName, foldcase(v) );
                            
                            // doc->add(
                            //     *Field::Keyword( (char*)attributeName.c_str(),
                            //                     foldcase(v).c_str()
                            //         ));

                            string t = attributeName + "-cs";
                            add( doc, t, v );
                            // doc->add(
                            //     *Field::Keyword( (char*)t.c_str(),
                            //                     v.c_str()
                            //         ));
                        }
                    }
                    catch( exception& e )
                    {
                        add( doc, attributeName, foldcase(v) );
                        // doc->add(
                        //     *Field::Keyword( (char*)attributeName.c_str(),
                        //                     foldcase(v).c_str()
                        //         ));

                        string t = attributeName + "-cs";
                        add( doc, t, v );
                        // doc->add(
                        //     *Field::Keyword( (char*)t.c_str(),
                        //                     v.c_str()
                        //         ));
                    }
                }

                string mtime = getStrAttr( c, "mtime", "0" );
                add( doc, "url",                foldcase(c->getURL()) );
                add( doc, "ferris-url",         c->getURL() );
                add( doc, "ferris-modified",    mtime );
                add( doc, "url-cs",             c->getURL() );
                add( doc, "ferris-url-cs",      c->getURL() );
                add( doc, "ferris-modified-cs", mtime );

                static int LIBFERRIS_DISABLE_URL_PREFILTER = 1;
                // static int LIBFERRIS_DISABLE_URL_PREFILTER
                //     = g_getenv ("LIBFERRIS_DISABLE_URL_PREFILTER") > 0;

                // Allow for simple regex evaulation using shifted term indexing on url
                if( !LIBFERRIS_DISABLE_URL_PREFILTER )
                {
                    typedef std::set< std::string > col_t;
                    std::string earl = c->getURL();
                    col_t sl;
                    Util::parseSeperatedList( earl, sl, '/' );
                    col_t next;
                    for( int i = 0; i < URL_REGEX_WORD_LENGTH; ++i )
                    {
                        for( col_t::iterator si = sl.begin(); si != sl.end(); ++si )
                        {
//                            cerr << "i:" << i << " s:" << *si << endl;
                            std::string s = *si;
                            if( s.empty() )
                                continue;
                            std::stringstream keyss;
                            keyss << "url" << "__prefixed";// << i;
                            add( doc, keyss.str(), foldcase( s ));
                            next.insert( s.substr(1) );
                        }
                        std::swap( sl, next );
                    }
                }
                
                
                LG_EAIDX_D << "addToIndex(addDoc) c:" << c->getURL() << endl;
                writer->addDocument( doc );
//                writer->optimize();
//                writer->close();
//                m_writer = 0;

                incrFilesIndexedCount();
                if( getFilesIndexedCount() % 1000 == 999 )
                {
                    sync();
                }
            }
            catch (exception& e)
            {
                throw;
            }
            catch( CLuceneError& e )
            {
                cerr << "getting docid, unexpected error number:" << e.number() << endl;
                cerr << "outer scope, unexpected error:" << e.what() << endl;
            }
            
        }

        typedef lucene::index::Term Term;



        void
        EAIndexerCLucene::BuildQueryLTE( BooleanQuery* query,
                                         const std::string& eaname,
                                         AttrType_t att,
                                         const std::string& v )
        {
            Term* currentTerm = makeTerm( eaname, att, v );
            Term* lowerBound  = isNumeric( att )
                ? makeTerm( eaname, att, "0" )
                : makeTerm( eaname, att, "" );
            
            RangeQuery* pq = new RangeQuery( lowerBound, currentTerm, true );
            query->add( pq, true, true, false );
        }
        

        void
        EAIndexerCLucene::BuildQueryGRE( BooleanQuery* query,
                                         const std::string& eaname,
                                         AttrType_t att,
                                         const std::string& v )
        {
            Term* currentTerm = makeTerm( eaname, att, v );
            Term* upperBound  = 0;
            if( isInt( att ))
                upperBound = makeTerm( eaname,
                                       att,
                                       padInteger(numeric_limits<int>::max()) );
            else if( isDouble( att ))
                upperBound = makeTerm( eaname,
                                       att,
                                       padDouble(numeric_limits<double>::max()) );
            else
                upperBound = makeTerm( eaname,
                                       att,
                                       getUpperBoundString() );
                
            RangeQuery* pq = new RangeQuery( currentTerm, upperBound, true );
            query->add( pq, true, true, false );
        }
    
        
        BooleanQuery*
        EAIndexerCLucene::BuildQuery( fh_context q,
                                      docNumSet_t& output,
                                      fh_eaquery qobj,
                                      BooleanQuery* query )
        {
            string token   = getStrAttr( q, "token", "" );
            string tokenfc = foldcase( token );
            fh_attribute orderedtla = q->getAttribute("in-order-insert-list");
            fh_istream   orderedtls = orderedtla->getIStream();
            
             cerr << "ExecuteQuery() token:" << tokenfc << endl;
//             cerr << " q.child count:" << q->SubContextCount() << endl;

            string s;
            getline( orderedtls, s );
//            cerr << "ExecuteQuery() lc:" << s << endl;
            fh_context lc = q->getSubContext( s );

            if( tokenfc == "!" )
            {
                BooleanQuery* negatedq = new BooleanQuery();
                query->add( negatedq, true, true, false );
                
                WildcardQuery* pq = new WildcardQuery(
                    makeTerm( "url", ATTRTYPEID_CIS, "*" ) );
                negatedq->add( pq, true, true, false );
                
                for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                {
                    BooleanQuery* subq = new BooleanQuery();
                    BuildQuery( *ci, output, qobj, subq );
                    negatedq->add( subq, true, false, true );
                }
                return query;
            }

            getline( orderedtls, s );
//            cerr << "EAQuery_Heur::ExecuteQuery() rc:" << s << endl;
            fh_context rc = q->getSubContext( s );            

            if( tokenfc == "&" )
            {
                for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                {
                    BooleanQuery* subq = new BooleanQuery();
                    BuildQuery( *ci, output, qobj, subq );
                    query->add( subq, true, true, false );
                }
                return query;
            }
            else if( tokenfc == "|" )
            {
                for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                {
                    BooleanQuery* subq = new BooleanQuery();
                    BuildQuery( *ci, output, qobj, subq );
                    query->add( subq, true, false, false );
                }
                return query;
            }
            

            string eaname = getStrAttr( lc, "token", "" );
            IndexableValue iv = getIndexableValueFromToken( eaname, rc );
            AttrType_t att = iv.getAttrTypeID();
            string value = asString( iv );
//            string value  = getStrAttr( rc, "token", "" );
//            string comparisonOperator = guessComparisonOperatorFromData( value );
//            value = foldcase( value );
            {
                static const boost::regex rx = toregex( "[^a-z0-9.*? ]" );
                if( regex_search( value, rx ) )
                {
                    eaname = eaname + "-cs";
                }
            }
            LG_EAIDX_D << "After case sensitive adjustments..."
                       << "...eaname:" << eaname << " value:" << value << endl;


            if( eaname == "ferris-ftx" || eaname == "ferris-fulltext-search" )
            {
                string ftxpath = getFulltextIndexPath();

                if( ftxpath.empty() )
                {
                    return query;
                }
                
                try
                {
                    LG_EAIDX_D << "ftxpath:" << ftxpath << endl;
                
                    FullTextIndex::fh_idx fidx = 
                        FullTextIndex::Factory::getFullTextIndex( ftxpath );
                    bool useShortcutDocIDs = fidx->isCLuceneIndex();
                    
//                     {
//                         docNumSet_t tmp;
//                         fidx->ExecuteLuceneFullTextQuery( "alice", tmp );
//                     }
                    
                    
//                    string our_dbname = getConfig( CFG_IDX_DBNAME_K, "" );
//                    bool useShortcutDocIDs = fidx->isCLuceneIndexInGivenDatabase( our_dbname );

                    string qstr = iv.rawValueString();
                    docNumSet_t docnums;

                    if( useShortcutDocIDs )
                    {
                        LG_EAIDX_D << "shortcut evaluation for cluene index. "
                                   << "running ftx query:" << qstr << " against ftxpath:" << ftxpath << endl;
                        query = (BooleanQuery*)fidx->BuildCLuceneQuery( qstr, query );
                        return query;
                    }
                    else
                    {
                        LG_EAIDX_D << "having to lookup all URLs from fulltext query to integrate cross engine index. "
                                   << "running ftx query:" << qstr << " against ftxpath:" << ftxpath << endl;
                        docNumSet_t tmp;
                        LG_IDX_D << "a qstr:" << qstr << endl;
                        fidx->ExecuteRawFullTextQuery( qstr, tmp, 0 );

                        LG_IDX_D << "Resolving urlids...sz:" << tmp.size() << endl;
                        stringset_t earls;
                        {
                            for( docNumSet_t::const_iterator ti = tmp.begin(); ti!=tmp.end(); ++ti )
                            {
                                docid_t d = *ti;
                                string earl = fidx->resolveDocumentID( d );
                                earls.insert( earl );
                            }
                        }

                        
                        for( stringset_t::const_iterator si = earls.begin(); si != earls.end(); ++si )
                        {
                            string earl = *si;
                            LG_IDX_D << "Adding matching url:" << earl << endl;
                            l_Term t = newterm( "ferris-url", earl );
                            l_TermQuery pq = new TermQuery( t );
                            query->add( pq, false, false, false );
                        }
                        LG_IDX_D << "Done...." << endl;
                    }
                }
                catch( exception& e )
                {
                    cerr << e.what() << endl;
                }
                return query;
            }

            
            if( tokenfc == "==" )
            {
                if( isNumeric( att ) )
                {
                    Term* currentTerm = makeTerm( eaname, att, value );
                    RangeQuery* pq = new RangeQuery( currentTerm, currentTerm, true );
                    query->add( pq, true, true, false );
                }
                else
                {
                    PhraseQuery* pq = new PhraseQuery();
                    pq->add( makeTerm( eaname, att, value ) );
                    query->add( pq, true, true, false );

                    // ensure no single term optimization is done.
                    // This is to avoid an issue in clucene-core-0.9.15
                    query->add( new PhraseQuery(), true, false, false );
                }
            }
            else if( tokenfc == "=?=" )
            {
                BooleanQuery* subq = new BooleanQuery();
                
                PhraseQuery* pq = new PhraseQuery();
                pq->add( makeTerm( eaname, att, value ) );
                subq->add( pq, true, false, false );

                pq = new PhraseQuery();
                pq->add( makeTerm( eaname, att,
                                   tostr(convertStringToInteger(value) ) ));
                subq->add( pq, true, false, false );
                query->add( subq, true, true, false );
            }
            else if( tokenfc == "=~" )
            {
                bool hasNoSpecialChars = regexHasNoSpecialChars( value );

                static int LIBFERRIS_DISABLE_URL_PREFILTER = 1;
                // static int LIBFERRIS_DISABLE_URL_PREFILTER
                //     = g_getenv ("LIBFERRIS_DISABLE_URL_PREFILTER") > 0;
                if( LIBFERRIS_DISABLE_URL_PREFILTER )
                    hasNoSpecialChars = 0;
                
                value = Util::replace_all( value, ".*", "*" );
                value = Util::replace_all( value, ".", "?" );
                if( hasNoSpecialChars && eaname == "url" && !value.empty() )
                {
                    LG_EAIDX_D << "quick resolution... value:" << value << endl;
                    stringstream vss;
                    vss << value << "*";
                    std::stringstream keyss;
                    keyss << eaname << "__prefixed";// << i;
                        
//                    cerr << "qr vss:" << vss.str() << endl;
                    WildcardQuery* pq = new WildcardQuery(
                        makeTerm( keyss.str(), att, vss.str() ) );
                    query->add( pq, true, true, false );
                }
                else
                {
                    cerr << "adding wildcard on value:" << value << endl;
                    if( string::npos == value.find( "*" ) )
                        value = (string)"*" + value + "*";
                    WildcardQuery* pq = new WildcardQuery(
                        makeTerm( eaname, att, value ) );
                    query->add( pq, true, true, false );
                }
            }
            else if( tokenfc == "<=" )
            {
                BuildQueryLTE( query, eaname, att, value );
            }
            else if( tokenfc == "<?=" )
            {
                BooleanQuery* tq = 0;
                BooleanQuery* subq = new BooleanQuery();
                
                tq = new BooleanQuery();
                BuildQueryLTE( tq, eaname, ATTRTYPEID_INT, tostr(convertStringToInteger(value)) );
                subq->add( tq, true, false, false );

                tq = new BooleanQuery();
                BuildQueryLTE( tq, eaname, ATTRTYPEID_DBL, tostr(convertStringToInteger(value)) );
                subq->add( tq, true, false, false );

                tq = new BooleanQuery();
                BuildQueryLTE( tq, eaname, ATTRTYPEID_CIS, value );
                subq->add( tq, true, false, false );

                query->add( subq, true, true, false );
            }
            else if( tokenfc == ">=" )
            {
                BuildQueryGRE( query, eaname, att, value );
            }
            else if( tokenfc == ">?=" )
            {
                BooleanQuery* tq = 0;
                BooleanQuery* subq = new BooleanQuery();
                
                tq = new BooleanQuery();
                BuildQueryGRE( tq, eaname, ATTRTYPEID_INT, tostr(convertStringToInteger(value)) );
                subq->add( tq, true, false, false );

                tq = new BooleanQuery();
                BuildQueryGRE( tq, eaname, ATTRTYPEID_DBL, tostr(convertStringToInteger(value)) );
                subq->add( tq, true, false, false );

                tq = new BooleanQuery();
                BuildQueryGRE( tq, eaname, ATTRTYPEID_CIS, value );
                subq->add( tq, true, false, false );

                query->add( subq, true, true, false );
            }
            else
            {
                cerr << "WARNING: Lucene index can not resolve operation:" << tokenfc << endl;
            }
            return query;
        }
        

//         l_BooleanQuery
//         foo( l_BooleanQuery q )
//         {
//             PhraseQuery* pq = new PhraseQuery();
//             pq->add( makeTerm( "size", EAIndex::MetaEAIndexerInterface::ATTRTYPEID_INT, "0" ) );
//             q->add( pq, true, true, false );

//             return q;
//         }
        
        
        // FIXME: NOT COMPLETE!!
        docNumSet_t&
        EAIndexerCLucene::ExecuteQuery( fh_context q,
                                        docNumSet_t& output,
                                        fh_eaquery qobj,
                                        int limit )
        {
            try
            {
                
                if( limit )
                {
                    stringstream ss;
                    ss << "EAIndexerCLucene::ExecuteQuery() limit not currently supported for Lucene backend" << endl;
                    cerr << tostr(ss);
                    LG_EAIDX_I << tostr(ss);
                }
            
                fh_stringstream qss;
            
//             {
//                 l_BooleanQuery query = new BooleanQuery();
//                 {
//                     PhraseQuery* pq = new PhraseQuery();
//                     pq->add( makeTerm( "size", EAIndex::MetaEAIndexerInterface::ATTRTYPEID_INT, "0" ) );
//                     query->add( pq, true, true, false );
//                 }
//             }

//             cerr << ".................1" << endl;
//             {
//                 l_BooleanQuery query;
//                 query = new BooleanQuery();
//                 {
// //                    l_BooleanQuery q = foo( query );
// //                    query = q;
//                 }
//                 cerr << ".................2" << endl;
//                 Searcher* searcher = new IndexSearcher( getPath().c_str() );
//                 cerr << ".................2.A" << endl;
//                 l_Hits hits = searcher->search( GetImpl(query) );
//                 cerr << ".................2.B" << endl;
//             }
//             cerr << ".................3" << endl;
            
            
            
                LG_EAIDX_D << "ExecuteQuery() building lucene query " << endl;
                BooleanQuery* outerq = new BooleanQuery();
                outerq->setMaxClauseCount( 50000 );
                l_BooleanQuery query = BuildQuery( q, output, qobj, outerq );
//             BooleanQuery* query = _CLNEW BooleanQuery();
//             {
// //                 Term* t = new Term( "ferris-url", "fdff" );
// //                 TermQuery* pq = new TermQuery( t );
//                PhraseQuery* pq = new PhraseQuery();
//                pq->add( new Term( "size", "0"  ) );
//                 query->add( pq, false, true, false );
//                 query->add( pq, false, true, false );
//             }
             
                LG_EAIDX_D << "ExecuteQuery() created lucene query " << endl;
                cerr << "ExecuteQuery() created lucene query " << endl;

//             Analyzer* analyzer = newAnalyzer();
//             Query* query = QueryParser::parse( tojstr( tostr( qss )),
//                                                tojstr(""),
//                                                analyzer );
                Searcher* searcher = new IndexSearcher( getPath().c_str() );
                LG_EAIDX_D << "ExecuteQuery() getting hits " << endl;
                l_Hits hits = searcher->search( GetImpl(query) );
            
                LG_EAIDX_D << "ExecuteQuery() adding hits " << endl;
                long length = hits->length();
                LG_EAIDX_D << "ExecuteQuery() adding hits hits.Length():" << length << endl;
                cerr << "ExecuteQuery() adding hits hits.Length():" << length << endl;
            
                for (int i = 0; i < hits->length(); i++)
                {
                    try
                    {
                        lucene::document::Document& doc = hits->doc(i);
                        int docID     = hits->id( i );

                        if( !doc.get(_T("ferris-url")) )
                        {
                            LG_EAIDX_W << "no URL recorded for id:" << docID << endl;
                            continue;
                        }

                        addDocID( output, docID );
                        LG_EAIDX_D << "id:" << docID << endl;
                        LG_EAIDX_D << "url:" << STR(doc.get(_T("ferris-url"))) << endl;
                        m_docIDHash[ docID ] = STR(doc.get(_T("ferris-url")));
                    }
                    catch( exception& e )
                    {
                    }
                    catch( CLuceneError& e )
                    {
                        cerr << "getting docid, unexpected error number:" << e.number() << endl;
                    }
                    catch( ... )
                    {
                    }
                }
            
                return output;
            }
            catch( CLuceneError& e )
            {
                cerr << "outer scope, unexpected error number:" << e.number() << endl;
                cerr << "outer scope, unexpected error:" << e.what() << endl;
                throw;
            }
            catch( ... )
            {
                throw;
            }
            return output;
        }
        
        std::string
        EAIndexerCLucene::resolveDocumentID( docid_t id )
        {
            return m_docIDHash[ id ];
        }

        void
        EAIndexerCLucene::cleanDocumentIDCache()
        {
            m_docIDHash.clear();
        }
            

        /****************************************/
        /****************************************/
        /****************************************/

        
        bool
        EAIndexerCLucene::isFileNewerThanIndexedVersion( const fh_context& c )
        {
            LG_EAIDX_D << "isFileNewerThanIndexedVersion(top) c:" << c->getURL() << endl;
            
            bool ret = true;

            time_t ct = getTimeAttr( c, "mtime", 0 );
            if( !ct )
                return ret;

            LG_EAIDX_D << "isFileNewerThanIndexedVersion() ct:" << ct << " c:" << c->getURL() << endl;


            
            l_Term t = newterm( "ferris-url", c->getURL() );
            l_TermQuery pq = new TermQuery( t );

//            auto_ptr<Searcher> searcher( new IndexSearcher( getPath().c_str() ) );
            Searcher* searcher = getSearcherFor_isFileNewerThanIndexedVersion();
            l_Hits hits = searcher->search( pq );

            LG_EAIDX_D << "isFileNewerThanIndexedVersion(T2) c:" << c->getURL() << endl;
            
            LG_EAIDX_D << "hits:" << hits->length() << " c:" << c->getURL() << endl;
            if( hits->length() )
            {
                time_t cachetime = 0;

                lucene::document::Document doc = hits->doc(0);

                if( !doc.get( _T("ferris-modified")))
                    return ret;
                string s = STR(doc.get( _T("ferris-modified")));
                
                cachetime = toType<time_t>( s );
                LG_EAIDX_D << " cachetime:" << cachetime << " currenttime:" << ct
                           << " ret:" << (cachetime < ct) << endl;
                return( cachetime < ct );
            }
            
            return ret;
        }
        
        
    };
};


extern "C"
{
    FERRISEXP_API Ferris::EAIndex::MetaEAIndexerInterface* Create()
    {
        return new Ferris::EAIndex::EAIndexerCLucene();
    }
};


