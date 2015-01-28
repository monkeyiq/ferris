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

    $Id: libftxidxxapian.cpp,v 1.5 2008/12/19 21:30:14 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/version.hpp>

#include <xapian.h>
#include <xapian/queryparser.h>

#include <Ferris/FullTextIndexerMetaInterface.hh>

#include <Ferris/Ferris.hh>
#include <Ferris/FullTextIndexer_private.hh>
#include <Ferris/FerrisStdHashMap.hh>

#include "libferrisxapianeashared.hh"

#include <string>
using namespace Xapian;
using namespace std;

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


namespace Ferris
{
    namespace FullTextIndex 
    {
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        class FERRISEXP_DLLLOCAL FullTextIndexerXAPIAN
            :
            public MetaFullTextIndexerInterface
        {
            typedef FullTextIndexerXAPIAN _Self;
            
            WritableDatabase m_database;
            typedef map< int, string > m_docidmap_t;
            m_docidmap_t m_docidmap;
            typedef map< string, int > m_revdocidmap_t;
            m_revdocidmap_t m_revdocidmap;
            int m_docidmap_nextid;
            
        protected:

            string getDBPath();
            Stem   getStemmer();
            bool   useStemmer();
            
            
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
            virtual void CommonConstruction();
            
            virtual void addToIndex( fh_context c,
                                     fh_docindexer di );

            virtual docNumSet_t& addAllDocumentsMatchingTerm(
                const std::string& term,
                docNumSet_t& output,
                int limit );
            virtual std::string resolveDocumentID( docid_t );

        public:

            FullTextIndexerXAPIAN();
            virtual ~FullTextIndexerXAPIAN();

            virtual void sync();
            virtual void prepareForWrites( int f );
            
            virtual
            docNumSet_t&
            ExecuteXapianFullTextQuery( const std::string& queryString,
                                        docNumSet_t& docnums,
                                        int limit );
            
            
            docNumSet_t& addAllDocumentsMatchingMSet( MSet& matches, docNumSet_t& output );

            virtual bool getIndexMethodSupportsIsFileNewerThanIndexedVersion();
            virtual bool isFileNewerThanIndexedVersion( const fh_context& c );
        };

        
        static const char* CFG_XAPIANIDX_DBNAME_K          = "cfg-xapianidx-dbname";
        static const char* CFG_XAPIANIDX_DBNAME_DEFAULT    = "fxapianidx.fidx";
        static const char* CFG_XAPIANIDX_STEMLANG_K        = "cfg-xapianidx-stemlang";
        static const char* CFG_XAPIANIDX_STEMLANG_DEFAULT  = "english";
//        static const char* CFG_XAPIANIDX_STEMLANG_DEFAULT  = "english (en)";

        /************************************************************/
        /************************************************************/
        /************************************************************/

        FullTextIndexerXAPIAN::FullTextIndexerXAPIAN()
            :
            m_docidmap_nextid( 1 )
        {
        }

        FullTextIndexerXAPIAN::~FullTextIndexerXAPIAN()
        {
        }


        void
        FullTextIndexerXAPIAN::sync()
        {
            m_database.flush();
        }
        
        void
        FullTextIndexerXAPIAN::prepareForWrites( int f )
        {
            if( f & PREPARE_FOR_WRITES_ISNEWER_TESTS )
            {
            }
        }
        

        string
        FullTextIndexerXAPIAN::getDBPath()
        {
            string dbname = this->getConfig( CFG_XAPIANIDX_DBNAME_K,
                                             CFG_XAPIANIDX_DBNAME_DEFAULT );
            fh_stringstream ss;
            ss << getPath() << "/" << dbname;
            return tostr(ss);
        }

        Stem
        FullTextIndexerXAPIAN::getStemmer()
        {
            string s = this->getConfig( CFG_XAPIANIDX_STEMLANG_K,
                                        CFG_XAPIANIDX_STEMLANG_DEFAULT );
            if( s.empty() )
                s = CFG_XAPIANIDX_STEMLANG_DEFAULT;
            
            try
            {
                Stem stem( s );
                return stem;
            }
            catch( Xapian::InvalidArgumentError& e )
            {
                cerr << e.get_msg() << endl;
                throw;
            }
        }
        
        bool
        FullTextIndexerXAPIAN::useStemmer()
        {
            string s = this->getConfig( CFG_XAPIANIDX_STEMLANG_K,
                                        CFG_XAPIANIDX_STEMLANG_DEFAULT );
            if( s.empty() )
                s = CFG_XAPIANIDX_STEMLANG_DEFAULT;
            return !s.empty() && s != "none";
        }
        
        
        
        void
        FullTextIndexerXAPIAN::Setup()
        {
            string dbpath = getDBPath();

            try {
                // Open the database
                m_database = WritableDatabase( dbpath, DB_CREATE_OR_OPEN );
            } catch (const Error &error) {
                cerr << "Exception: "  << error.get_msg() << endl;
                throw;
            }
        }

        void
        FullTextIndexerXAPIAN::CreateIndexBeforeConfig( fh_context c,
                                                      bool caseSensitive,
                                                      bool dropStopWords,
                                                      StemMode stemMode,
                                                      const std::string& lex_class,
                                                      fh_context md )
        {
        }
        
        
        void
        FullTextIndexerXAPIAN::CreateIndex( fh_context c,
                                            bool caseSensitive,
                                            bool dropStopWords,
                                            StemMode stemMode,
                                            const std::string& lex_class,
                                            fh_context md )
        {

            string dbname   = getStrSubCtx( md, "dbname", "" );
            string stemlang = getStrSubCtx( md, "stemmer-language", "" );
            setConfig( CFG_XAPIANIDX_DBNAME_K,   dbname );
            setConfig( CFG_XAPIANIDX_STEMLANG_K, stemlang );

            Setup();
        }

        void
        FullTextIndexerXAPIAN::CommonConstruction()
        {
        }

        void
        FullTextIndexerXAPIAN::addToIndex( fh_context c, fh_docindexer di )
        {
            LG_IDX_D << "addToIndexDocTermsClass() c:" << c->getURL() << endl;
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
//                iss = c->getIStream();
            }
            streamsize bytesDone    = 0;
            streamsize signalWindow = 0;
            streamsize totalBytes   = toType<streamsize>(getStrAttr( c, "size", "0" ));

            bool isCaseSensitive = this->isCaseSensitive();
            bool DropStopWords   = getDropStopWords();
            StemMode stemMode    = getStemMode();
            Stem     stemmer     = getStemmer();

            string docidtime = tostr(Time::getTime());
            Xapian::Document doc;
            setupNewDocument( doc, c, docidtime );
            
            while( !(s = di->getToken( iss )).empty() )
            {
                LG_IDX_D << "addToIndex() term1:" << s << endl;
                if( !isCaseSensitive )
                    s = tolowerstr()( s );
                if( useStemmer() )
                    s = stemmer.stem_word( s );
                LG_IDX_D << "addToIndex() term2:" << s << endl;
                
                doc.add_posting( s, iss.tellg() );
            }

            string urlterm = makeURLTerm( c );
            Xapian::docid did = m_database.replace_document( urlterm, doc );
        }

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        std::string
        FullTextIndexerXAPIAN::resolveDocumentID( docid_t id )
        {
            return m_docidmap[ id ];
        }

        docNumSet_t&
        FullTextIndexerXAPIAN::addAllDocumentsMatchingMSet( MSet& matches, docNumSet_t& output,
                                                            int limit )
        {
            return common_addAllDocumentsMatchingMSet( matches, output,
                                                       m_docidmap,
                                                       m_revdocidmap,
                                                       m_docidmap_nextid,
                                                       addDocIDFunctor( this, &_Self::addDocID ) );
        }
        
        
        docNumSet_t&
        FullTextIndexerXAPIAN::addAllDocumentsMatchingTerm(
            const std::string& term_const,
            docNumSet_t& output,
            int limit )
        {
            std::string term = term_const;
            
            try
            {
                LG_IDX_D << "FullTextIndexerXAPIAN::addAllDocumentsMatchingTerm() term:" << term << endl;
                // Start an enquire session
                Enquire enquire( m_database );

                Stem stemmer = getStemmer();
                vector<string> stemmed_terms;

                if( !this->isCaseSensitive() )
                    term = tolowerstr()( term );
                
                if( useStemmer() )
                    term = stemmer.stem_word( term );
                
                stemmed_terms.push_back( term );
	    
                // Build a query by OR-ing together all the terms
                Query query(Query::OP_OR, stemmed_terms.begin(), stemmed_terms.end());
                LG_IDX_D << "Performing query `" << query.get_description() << "'" << endl;

                // Give the query object to the enquire session
                enquire.set_query(query);

                // Get the top 500 results of the query
                MSet matches = enquire.get_mset(0, 500);
                addAllDocumentsMatchingMSet( matches, output );
            } catch (const Error &error) {
                cerr << "Exception: "  << error.get_msg() << endl;
                throw;
            }
            
            return output;
        }

        
        docNumSet_t&
        FullTextIndexerXAPIAN::ExecuteXapianFullTextQuery( const std::string& queryString,
                                                           docNumSet_t& output,
                                                           int limit )
        {
            Enquire enquire( m_database );
            try
            {
                
                Xapian::QueryParser parser;
                parser.set_database( m_database );
                parser.set_default_op(Xapian::Query::OP_OR);

                if( useStemmer() )
                {
//                    parser.set_stemming_options("english", false, NULL);
                    parser.set_stemmer(Xapian::Stem("english"));
                    parser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);                    
                }
                enquire.set_query(parser.parse_query( queryString ));
            }
            catch (const char * error_msg)
            {
                fh_stringstream ss;
                ss << "Couldn't parse query: " << error_msg << endl;
                Throw_FullTextIndexException( tostr(ss), 0 );
            }

            // Get the top 500 results of the query
            MSet matches = enquire.get_mset(0, 500);
            addAllDocumentsMatchingMSet( matches, output );
            
            return output;
        }
        
        
        /**************************************************/
        /**************************************************/


        bool
        FullTextIndexerXAPIAN::getIndexMethodSupportsIsFileNewerThanIndexedVersion()
        {
            return true;
        }
        bool
        FullTextIndexerXAPIAN::isFileNewerThanIndexedVersion( const fh_context& c )
        {
            return common_isFileNewerThanIndexedVersion( m_database, c );
        }

        
        
    };
};

extern "C"
{
    FERRISEXP_API Ferris::FullTextIndex::MetaFullTextIndexerInterface* Create()
    {
        return new Ferris::FullTextIndex::FullTextIndexerXAPIAN();
    }
};
