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

    $Id: FullTextIndexer.cpp,v 1.8 2010/09/24 21:30:51 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "FullTextIndexer.hh"
#include "FullTextIndexer_private.hh"
#include "Indexing/IndexPrivate.hh"
#include <Configuration_private.hh>
#include "Trimming.hh"
#include "Iterator.hh"
#include "Medallion_private.hh"
#include "Medallion.hh"

#include <mg/stem.h>
#include <mg/bitio_m.h>
#include <mg/bitio_m_mem.h>
#include <mg/bitio_mem.h>
#include <mg/bitio_gen.h>

#include <errno.h>

#include <iostream>
#include <iterator>
#include <algorithm>
#include <numeric>

// #undef  LG_IDX_D
// #define LG_IDX_D cerr
#define DEBUG    true

using namespace std;

namespace Ferris
{
    namespace FullTextIndex 
    {
        
        AddToFullTextIndexProgress_Sig_t& getNullAddToFullTextIndexProgress_Sig()
        {
            static AddToFullTextIndexProgress_Sig_t o;
            return o;
        }


        const std::string IDXMGR_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_K = "idxmgr-nonresolvable-not-to-remove-regex-k";
        
        std::string GET_INDEX_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_DEFAULT( bool commaSeperated )
        {
            string def = (std::string)"" + '\0';
            string ret = getEDBString( GET_FDB_GENERAL(), "EAINDEX_NONRESOLVABLE_NOT_TO_REMOVE_REGEX", def );

            if( commaSeperated )
                replace( ret.begin(), ret.end(), '\0', ',' );
            
            return ret;
            
        }
        

        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        string Lexicon::NULL_VALUE_TERM = "<null>";
        string Lexicon::UNREADABLE_VALUE_TERM = "<unreadable value>";

        
        Lexicon::Lexicon( fh_idx idx, PathManager* path_mgr )
            :
            m_idx( idx ),
            m_path_mgr( path_mgr ),
            m_usingNullValue( false ),
            m_usingUnreadableValue( false )
            
        {
            if( m_idx && !m_path_mgr )
                m_path_mgr = m_idx->tryToCastToPathManager();
//             if( m_idx && !m_path_mgr )
//                 m_path_mgr = GetImpl(m_idx);
        }

        Lexicon::~Lexicon()
        {
        }

        void
        Lexicon::setIndex( fh_idx idx )
        {
            m_idx = idx;
            
            if( m_idx && !m_path_mgr )
                m_path_mgr = m_idx->tryToCastToPathManager();
//             if( m_idx && !m_path_mgr )
//                 m_path_mgr = GetImpl(m_idx);
        }
        
        void
        Lexicon::dumpTo( fh_ostream oss, bool asXML, const std::string& name )
        {
            if( asXML ) oss << "<lexicon direction=\"forward\" name=\"" << name << "\" >" << endl;

            try
            {
                bool haveVisitedNull = !m_usingNullValue;
                
                std::string term = getFirstTerm();

//                 LG_IDX_D << "Starting Lexicon::dumpTo() name:" << name
//                      << " empty:" << term.empty()
//                      << " visitedNull:" << haveVisitedNull
//                      << endl;
                
                while( !term.empty() || !haveVisitedNull )
                {
                    termid_t id = lookup( term );
//                     LG_IDX_D << "term.empty():" << term.empty()
//                          << " tid:" << id
//                          << " haveVisitedNull:" << haveVisitedNull
//                          << " un:" << m_usingNullValue
//                          << endl;
                    
                    if( id == NULL_VALUE_TERMID )
                    {
//                         if( !m_usingNullValue )
//                             break;
                        haveVisitedNull = true;
                    }
                    
                    if( m_usingNullValue && id == NULL_VALUE_TERMID )
                    {
                        oss << "<lexterm isnull=\"1\" id=\"" << id << "\">"
                            << "</lexterm>" << endl;
                    }
                    else if( m_usingUnreadableValue && id == UNREADABLE_VALUE_TERMID )
                    {
                        oss << "<lexterm unreadable=\"1\" id=\"" << id << "\">"
                            << "</lexterm>" << endl;
                    }
                    else
                    {
                        if( asXML )
                            oss << "<lexterm id=\"" << id << "\">" << term << "</lexterm>" << endl;
                        else
                            oss << term << ", " << id << endl;
                    }
                    
                    term = getNextTerm( term );
                }
            }
            catch( exception& e )
            {
                oss << "<error> Error traversing data:" << e.what() << " </error>" << endl;
            }
            if( asXML ) oss << "</lexicon>" << endl;
        }

        void
        Lexicon::setUsingNullValue( bool v )
        {
            m_usingNullValue = true;
        }
        
        void
        Lexicon::setUsingUnreadableValue( bool v )
        {
            m_usingUnreadableValue = true;
        }
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        class FERRISEXP_DLLLOCAL Lexicon_Cache
            :
            public Lexicon
        {
            typedef map< string, termid_t > m_cache_t;
            m_cache_t m_cache;
            fh_lexicon theDelegate;
            
        protected:

            virtual void setIndex( fh_idx idx )
                {
                    Lexicon::setIndex( idx );
                    theDelegate->setIndex( idx );
                }
            
        public:
            
            Lexicon_Cache( fh_lexicon l )
                :
                Lexicon( l->m_idx, l->m_path_mgr ),
                theDelegate( l )
                {
                }
            
            virtual ~Lexicon_Cache()
                {
                }

            virtual std::string getFirstTerm()
                {
                    return theDelegate->getFirstTerm();
                }
            virtual std::string getNextTerm( const std::string& s )
                {
                    return theDelegate->getNextTerm( s );
                }

            virtual void insert( const std::string& term, termid_t termid )
                {
                    m_cache[ term ] = termid;
                    theDelegate->insert( term, termid );
                }
            
            virtual termid_t lookup( const std::string& term )
                {
                    m_cache_t::iterator ci = m_cache.find( term );
                    if( ci != m_cache.end() )
                        return ci->second;
                    termid_t ret = theDelegate->lookup( term );
                    m_cache[ term ] = ret;
                    return ret;
                }

            virtual void dumpTo( fh_ostream oss, bool asXML, const std::string& name )
                {
                    theDelegate->dumpTo( oss, asXML, name );
                }

            virtual void sync()
                {
                    theDelegate->sync();
                }
            
            virtual void prepareForInsertions()
                {
                    theDelegate->prepareForInsertions();
                }
        };
        
        
        fh_lexicon wrapWithCache( fh_lexicon l )
        {
            fh_lexicon ret = new Lexicon_Cache( l );
            return ret;
        }
        
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        ReverseLexicon::ReverseLexicon()
            :
            m_path_mgr( 0 ),
            m_filename( "" ),
            m_usingNullValue( false ),
            m_usingUnreadableValue( false )
        {
        }
            
        ReverseLexicon::~ReverseLexicon()
        {
            sync();
        }

        void
        ReverseLexicon::dumpTo( fh_ostream oss, bool asXML )
        {
            if( asXML ) oss << "<lexicon direction=\"reverse\" >" << endl;

            try
            {
                termid_t id = getFirstTerm();
                while( id )
                {
                    string term = lookup( id );

                    if( m_usingNullValue && id == Lexicon::NULL_VALUE_TERMID )
                    {
                        oss << "<lexterm isnull=\"1\" id=\"" << id << "\">"
                            << "</lexterm>" << endl;
                    }
                    else if( m_usingUnreadableValue && id == Lexicon::UNREADABLE_VALUE_TERMID )
                    {
                        oss << "<lexterm unreadable=\"1\" id=\"" << id << "\">"
                            << "</lexterm>" << endl;
                    }
                    else
                    {
                        if( asXML )
                            oss << "<lexterm id=\"" << id << "\">" << term << "</lexterm>" << endl;
                        else
                            oss << term << ", " << id << endl;
                    }
                    
                    id = getNextTerm( id );
                }
            }
            catch( exception& e )
            {
                oss << "<error> Error traversing data:" << e.what() << " </error>" << endl;
            }
            if( asXML ) oss << "</lexicon>" << endl;
        }
        
        void
        ReverseLexicon::setUsingNullValue( bool v )
        {
            m_usingNullValue = true;
        }
        
        void
        ReverseLexicon::setUsingUnreadableValue( bool v )
        {
            m_usingUnreadableValue = true;
        }
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        class FERRISEXP_DLLLOCAL ReverseLexicon_Cache
            :
            public ReverseLexicon
        {
            typedef map< termid_t, string > m_cache_t;
            m_cache_t m_cache;
            fh_revlexicon theDelegate;
            
        protected:
            virtual termid_t getFirstTerm()
                {
                    return theDelegate->getFirstTerm();
                }
            
            virtual termid_t getNextTerm( termid_t t )
                {
                    return theDelegate->getNextTerm( t );
                }

        public:

            ReverseLexicon_Cache( fh_revlexicon l )
                :
                ReverseLexicon(),
                theDelegate( l )
                {
                    m_path_mgr = l->m_path_mgr;
                    m_filename = l->m_filename;
                }
            
            virtual void insert( const std::string& s, termid_t id )
                {
                    m_cache[ id ] = s;
                    theDelegate->insert( s, id );
                }

            virtual std::string lookup( termid_t id )
                {
                    m_cache_t::iterator ci = m_cache.find( id );
                    if( ci != m_cache.end() )
                    {
                        return ci->second;
                    }
                    string ret = theDelegate->lookup( id );
                    m_cache[ id ] = ret;
                    return ret;
                }

            virtual bool exists( termid_t id )
                {
                    m_cache_t::iterator ci = m_cache.find( id );
                    if( ci != m_cache.end() )
                    {
                        return true;
                    }
                    bool ret = theDelegate->exists( id );
                    return ret;
                }
            
            virtual void dumpTo( fh_ostream oss, bool asXML )
                {
                    theDelegate->dumpTo( oss, asXML );
                }
            
            virtual void sync()
                {
                    theDelegate->sync();
                }

            virtual void prepareForInsertions()
                {
                    theDelegate->prepareForInsertions();
                }
        };
        
        
        fh_revlexicon wrapWithCache( fh_revlexicon l )
        {
            fh_revlexicon ret = new ReverseLexicon_Cache( l );
        }
         

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        DocumentIndexer::DocumentIndexer( fh_idx idx )
            :
            m_idx( idx ),
            m_dontCheckIfAlreadyThere( false ),
            m_filesIndexedCount( 0 ),
            m_haveCalledPrepareForWrites( false ),
            m_AddEvenIfAlreadyCurrent( false ),
            m_haveReportedIndexDoesNotSupportAddEvenIfAlreadyCurrent( false )
        {
            if( m_idx )
                m_idx->prepareForInsertions();
            
            getTokenBuffer.resize( LARGEST_TOKEN );
        }

        DocumentIndexer::~DocumentIndexer()
        {
            if( m_idx )
                m_idx->allWritesComplete();

            if( m_idx )
                m_idx->sync();
        }
        
        AddToFullTextIndexProgress_Sig_t&
        DocumentIndexer::getProgressSig()
        {
            return m_progressSig;
        }
        

        streamsize
        DocumentIndexer::getBytesCompleted()
        {
            return m_bytesDone;
        }
        
        string
        DocumentIndexer::getToken( fh_istream& iss )
        {
            int i = 0;
            char c;
            bool tokenHasBytes    = false;
            bool isAllNumeric     = true;
            int  length           = 0;

            while( iss.get(c) )
            {
                ++m_bytesDone;
                
//                LG_IDX_D << c;
                if( isalnum( c ) )
                {
                    // dont allow leading digits and trailing alphas
                    if( tokenHasBytes && isAllNumeric && !isdigit( c ) )
                    {
                        break;
                    }
                    
                    tokenHasBytes = true;
                    getTokenBuffer[i++] = c;
                    
                    isAllNumeric = isAllNumeric && isdigit( c );

                    ++length;
                    if( length >= 255 )
                        break;
                    if( isAllNumeric && length >= 4 )
                        break;

                    if( i == LARGEST_TOKEN )
                        break;
                    
                    continue;
                }

                if( tokenHasBytes )
                    break;
            }

            return getTokenBuffer.substr( 0, i );
        }
        
//         string
//         DocumentIndexer::getToken( fh_istream& iss )
//         {
//             stringstream ss;
//             char c;
//             bool tokenHasBytes    = false;
//             bool isAllNumeric     = true;
//             int  length           = 0;
            
//             while( iss >> noskipws >> c )
//             {
//                 ++m_bytesDone;
                
// //                LG_IDX_D << c;
//                 if( isalnum( c ) )
//                 {
//                     // dont allow leading digits and trailing alphas
//                     if( tokenHasBytes && isAllNumeric && !isdigit( c ) )
//                     {
//                         break;
//                     }
                    
//                     tokenHasBytes = true;
//                     ss << c;
                        
//                     isAllNumeric = isAllNumeric && isdigit( c );

//                     ++length;
//                     if( length >= 255 )
//                         break;
//                     if( isAllNumeric && length >= 4 )
//                         break;
                    
//                     continue;
//                 }

//                 if( tokenHasBytes )
//                     break;
//             }

//             return tostr(ss);
//         }

        void
        DocumentIndexer::setDontCheckIfAlreadyThere( bool v )
        {
            m_dontCheckIfAlreadyThere = v;
        }

        bool
        DocumentIndexer::getDontCheckIfAlreadyThere()
        {
            return m_dontCheckIfAlreadyThere;
        }
        
        int
        DocumentIndexer::getFilesIndexedCount()
        {
            return m_filesIndexedCount;
        }
            

        

        void
        DocumentIndexer::sync()
        {
            m_idx->sync();
//            m_idx->sync( this m_filesIndexedCount );
        }
        
        void
        DocumentIndexer::addContextToIndex( fh_context c )
        {
            if( !m_haveCalledPrepareForWrites )
            {
                m_haveCalledPrepareForWrites = true;
                
                int f = MetaFullTextIndexerInterface::PREPARE_FOR_WRITES_NONE;
                if( !m_AddEvenIfAlreadyCurrent )
                    f |= MetaFullTextIndexerInterface::PREPARE_FOR_WRITES_ISNEWER_TESTS;
                
                m_idx->prepareForWrites( f );
            }

            if( !m_AddEvenIfAlreadyCurrent )
            {
                bool sup = m_idx->getIndexMethodSupportsIsFileNewerThanIndexedVersion();
                if( !sup && !m_haveReportedIndexDoesNotSupportAddEvenIfAlreadyCurrent )
                {
                    m_haveReportedIndexDoesNotSupportAddEvenIfAlreadyCurrent = true;
                    cerr << "WARNING: index doesn't support isNewer() checks" << endl;
                }
                    
                if( sup )
                {
                    bool isNewer = m_idx->isFileNewerThanIndexedVersion( c );
                    if( !isNewer )
                    {
                        LG_IDX_D << "Skipping because index is current for c:"
                                 << c->getURL() << endl;
//                         if( m_verbose )
//                         {
//                             cerr << "Skipping because index is current for c:"
//                                  << c->getURL() << endl;
//                         }
                        return;
                    }
                }

                if( fh_emblem em = getShouldSkipIndexingEmblem() )
                {
                    if( c->hasMedallion() )
                    {
                        fh_medallion med  = c->getMedallion();
                        if( med && med->hasEmblem( em ) )
                        {
                            LG_IDX_D << "Context has should-skip-indexing emblem... skipping c:" << c->getURL() << endl;
                            cerr << "Context has should-skip-indexing emblem... skipping c:" << c->getURL() << endl;
                            return;
                        }
                    }
                }
            }
            
            ++m_filesIndexedCount;
            m_bytesDone = 0;
            m_idx->addToIndex( c, this );
        }
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/


        
        
        
        


//         fh_idx createFullTextIndexNative( fh_context c,
//                                           bool caseSensitive,
//                                           bool dropStopWords,
//                                           StemMode stemMode,
//                                           const std::string& lex_class )
//         {
//             fh_idx idx;
//             idx = new FullTextIndexManager( c->getURL(),
//                                             caseSensitive,
//                                             dropStopWords,
//                                             stemMode,
//                                             lex_class );
//             return idx;
//         }

        stringlist_t& getLexiconClassNames()
        {
            static stringlist_t sl;
            return sl;
        }

bool appendToLexiconClassNames( const std::string& s )
{
    getLexiconClassNames().push_back( s );
    return true;
}

stringlist_t& getLexiconAliasNames()
{
    static stringlist_t sl;
    return sl;
}

bool appendToLexiconAliasNames( const std::string& s )
{
    getLexiconAliasNames().push_back( s );
    return true;
}


        stringlist_t& getReverseLexiconClassNames()
        {
            static stringlist_t sl;
            return sl;
        }
        bool appendToReverseLexiconClassNames( const std::string& s )
        {
            getReverseLexiconClassNames().push_back( s );
            return true;
        }



        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        std::string stem( const std::string& s, StemMode sm )
        {
            switch( sm )
            {
            case STEM_NONE: return s;
            case STEM_J_B_LOVINS_68:
            {
                if( s.empty() )
                    return s;

                int slen = s.length();
                static string Buffer;
                static int BufferSz = 0;
                if( BufferSz < slen )
                {
                    BufferSz = slen;
                    Buffer.resize( slen+1 );
                }
                Buffer[0] = (char)slen;
                strncpy( (char*)Buffer.data() + 1, s.data(), slen );
                ferrismg_stem( (unsigned char*)Buffer.data() );

//                 LG_IDX_D << "oringial:" << s
//                      << " stem:" << Buffer.substr( 1, (int)Buffer[0] )
//                      << endl;
                
                return Buffer.substr( 1, (int)Buffer[0] );
                
                
//                 int slen = s.length();
//                 stringstream ss;
//                 ss << (char)slen;
//                 ss << s;

//                 string mystring = tostr(ss);
//                 ferrismg_stem( (unsigned char*)mystring.c_str() );
            
// // //                 string ret;
// // //                 char c = mystring[0];
// // //                 ret.reserve( (int)c );
// // //                 copy( mystring.begin()+1, mystring.begin()+1+(int)c,
// // //                       back_inserter(ret) );

// // // //         LG_IDX_D << "stem() mystring:" << mystring
// // // //              << " s:" << s
// // // //              << " result.len:" << (int)c << " res:" << ret
// // // //              << endl;
// // //                 return ret;

//                 return mystring.substr( 1, (int)mystring[0] );
            }
            }
            LG_IDX_ER << "Bad stemmer chosen!" << endl;
            return s;
        }


std::string foldcase( const std::string& s, wordcase c )
{
    static string ret;
    bool hitSpace = true;

    ret.resize( s.length() );
    int i = 0;
    
    for( string::const_iterator iter = s.begin(); iter != s.end(); ++iter )
    {
        switch( c )
        {
        case CASE_UPPER:
            ret[i] = (char)toupper( *iter );
            break;
                
        case CASE_LOWER:
            ret[i] = (char)tolower( *iter );
            break;
                
        case CASE_CAPPED:
            if( hitSpace )
                ret[i] = (char)toupper( *iter );
            else
                ret[i] = (char)tolower( *iter );

            hitSpace = *iter == ' ' || *iter == '\t';
            break;
        }
        ++i;
    }
    return ret;
}

//        The above version is faster.
//        it does 1.6 seconds on the war of the worlds
//        the below does 2.5
//         std::string foldcase( const std::string& s, wordcase c )
//         {
//             fh_stringstream ret;
//             bool hitSpace = true;

//             for( string::const_iterator iter = s.begin(); iter != s.end(); ++iter )
//             {
//                 switch( c )
//                 {
//                 case CASE_UPPER:
//                     ret << (char)toupper( *iter );
//                     break;
                
//                 case CASE_LOWER:
//                     ret << (char)tolower( *iter );
//                     break;
                
//                 case CASE_CAPPED:
//                     if( hitSpace )
//                         ret << (char)toupper( *iter );
//                     else
//                         ret << (char)tolower( *iter );

//                     hitSpace = *iter == ' ' || *iter == '\t';
//                     break;
//                 }
//             }
//             return tostr(ret);
//         }
        
        /************************************************************/
        /************************************************************/
        /************************************************************/

        UniqSortedTerms::UniqSortedTerms( fh_istream& iss,
                                          fh_docindexer di,
                                          bool isCaseSensitive,
                                          StemMode stemmer,
                                          const stringset_t& stopwords,
                                          bool DropStopWords )
        {
            string s;
            while( !(s = di->getToken( iss )).empty() )
            {
                if( !isCaseSensitive )
                {
                    s = foldcase( s );
                }

                s = stem( s, stemmer );
                    
                if( DropStopWords )
                {
                    if( stopwords.count( s ) )
                    {
                        continue;
                    }
                }
                    
                ++m_uniqTerms[s];
            }
            reset();
                
        }

        void
        UniqSortedTerms::reset()
        {
            m_end = m_uniqTerms.end();
            m_cur = m_uniqTerms.begin();
        }
            
        bool
        UniqSortedTerms::next( string& s, int& termCount )
        {
            if( m_cur == m_end )
            {
                termCount=0;
                s="";
                return false;
            }

            s         = m_cur->first;
            termCount = m_cur->second;
            ++m_cur;
            return true;
        }


    };
};

