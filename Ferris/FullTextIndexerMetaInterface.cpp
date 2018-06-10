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

    $Id: FullTextIndexerMetaInterface.cpp,v 1.13 2010/09/24 21:30:51 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Ferris/FullTextIndexerMetaInterface.hh>
#include <Ferris/FullTextIndexer_private.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/Ferris_private.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/Configuration_private.hh>
#include <STLdb4/stldb4.hh>

#include "config.h"

using namespace std;

namespace Ferris
{
    
    namespace FullTextIndex 
    {
        using namespace ::STLdb4;
        static const string DB_FULLTEXT  = "full-text-index-config.db";
        static const string FULLTEXTROOT = getDotFerrisPath() + "full-text-index";
        static const char* CFG_IDX_DBNAME_K = "cfg-idx-dbname";
        
        class FERRISEXP_DLLLOCAL MetaFullTextIndexerInterfacePriv
        {
        public:
            MetaFullTextIndexerInterfacePriv()
                :
                m_config( 0 ), m_configNotAvailable( false ), m_base( 0 ),
                m_configShouldBeReadOnly( false )
                {}

            stringmap_t     m_configCache;
            stringset_t     m_stopWords;
            fh_database     m_config;
            bool            m_configNotAvailable;
            bool            m_configShouldBeReadOnly;
            fh_database     getConfigDB();
            fh_context      m_base;

            /**
             * Ensure that there is a db4 file there for setConfig() / getConfig() to use
             */
            void ensureConfigFileCreated()
                {
                    if( !m_config )
                    {
                        m_config = ensureFerrisConfigFileExists( m_base->getDirPath(), DB_FULLTEXT );
                        set_db4_string( getConfigDB(), "foo1", "bar" );
                        getConfigDB()->sync();
                    }
                }
        };

        fh_database
        MetaFullTextIndexerInterfacePriv::getConfigDB()
        {
//             cerr << "MetaFullTextIndexerInterfacePriv::getConfigDB(1) m_config:" << toVoid(m_config)
//                  << " m_configNotAvailable:" << m_configNotAvailable
//                  << endl;
            
            if( m_config || m_configNotAvailable )
                return m_config;

            string dbfilename = CleanupURL( m_base->getDirPath() + "/" + DB_FULLTEXT );
//             cerr << "MetaFullTextIndexerInterfacePriv::getConfigDB() dbfilename:"
//                  << dbfilename << endl;
            try
            {
//                m_config = new Database( dbfilename );
                m_config = new Database( dbfilename, "", m_configShouldBeReadOnly );
                
            }
            catch( exception& e )
            {
                cerr << "MetaFullTextIndexerInterfacePriv::getConfigDB() e:" << e.what()
                     << " path:" << dbfilename
                     << endl;
                m_configNotAvailable = true;
            }
            return m_config;
        }

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        
        void
        MetaFullTextIndexerInterface::addDocID( docNumSet_t& output, docid_t v )
        {
            output.insert( v );
        }
        
        
        MetaFullTextIndexerInterface::MetaFullTextIndexerInterface()
            :
            P( new MetaFullTextIndexerInterfacePriv() )
        {
        }

        MetaFullTextIndexerInterface::~MetaFullTextIndexerInterface()
        {
            if( P )
                delete P;
        }
        
        void
        MetaFullTextIndexerInterface::LocalCommonConstruction()
        {
            P->ensureConfigFileCreated();
        }
        
        void
        MetaFullTextIndexerInterface::private_initialize( const std::string& basepath )
        {
            P->m_base = Shell::acquireContext( basepath );
            Setup();
        }
        
        std::string
        MetaFullTextIndexerInterface::getPath()
        {
            return P->m_base->getDirPath();
        }

        std::string
        MetaFullTextIndexerInterface::getURL()
        {
            return P->m_base->getURL();
        }

        bool
        MetaFullTextIndexerInterface::isTSearch2IndexInGivenDatabase( const std::string& wanted_dbname )
        {
            string classname = getConfig( IDXMGR_INDEX_CLASS_K, "" );
            string dbname = getConfig( CFG_IDX_DBNAME_K, "" );

            return classname == "postgresql-tsearch2"
                && dbname == wanted_dbname;
        }
        
        bool
        MetaFullTextIndexerInterface::isCLuceneIndex()
        {
            string classname = getConfig( IDXMGR_INDEX_CLASS_K, "" );
            cerr << "MetaFullTextIndexerInterface::isCLuceneIndex() classname:" << classname << endl;
            return classname == "fulltextindexclucene";
        }

        void*
        MetaFullTextIndexerInterface::BuildCLuceneQuery( const std::string& qstr, void* current_query_vp )
        {
            fh_stringstream ss;
            ss << "Attempt to BuildCLuceneQuery() against"
               << " an index that doesn't support this method"
               << endl;
            Throw_FullTextIndexException( tostr(ss), 0 );
        }
        
        std::string
        MetaFullTextIndexerInterface::getConfig( const std::string& k, const std::string& def, bool throw_for_errors )
        {
            fh_database db = P->getConfigDB();
            
            if( P->m_configNotAvailable && !throw_for_errors )
                return def;

            stringmap_t::const_iterator ci = P->m_configCache.find( k );
            if( ci != P->m_configCache.end() )
                return ci->second;
            
//            cerr << "getConfig() k:" << k << endl;
            string ret = get_db4_string( db, k, def, throw_for_errors );
            P->m_configCache[ k ] = ret;
            return ret;
        }
        
        void
        MetaFullTextIndexerInterface::setConfig( const std::string& k, const std::string& v )
        {
//             cerr << "MetaFullTextIndexerInterface::setConfig() k:" << k
//                  << " v:" << v << endl;

            P->m_configCache[ k ] = v;
            
            fh_database db = P->getConfigDB();
            set_db4_string( db, k, v );
            db->sync();
        }

        bool
        MetaFullTextIndexerInterface::getDropStopWords()
        {
            return isTrue( getConfig( IDXMGR_DROPSTOPWORDS_CLASS_K, "0" ));
        }

        stringset_t&
        MetaFullTextIndexerInterface::getStopWords()
        {
            if( P->m_stopWords.empty() )
            {
                stringlist_t sl = Util::parseCommaSeperatedList( 
                    getConfig( IDXMGR_STOPWORDSLIST_K, IDXMGR_STOPWORDSLIST_DEFAULT ) );
                copy( sl.begin(), sl.end(), inserter( P->m_stopWords, P->m_stopWords.end() ));
            }
            return P->m_stopWords;
        }
        
        bool
        MetaFullTextIndexerInterface::isCaseSensitive()
        {
            return isTrue( getConfig( IDXMGR_CASESEN_CLASS_K, "0" ));
        }
        

        StemMode
        MetaFullTextIndexerInterface::getStemMode()
        {
//             cerr << "MetaFullTextIndexerInterface::getStemMode() val:"
//                  << getConfig( IDXMGR_STEMMER_CLASS_K, "undefined" )
//                  << endl;
            
            return StemMode(
                toType<int>(
                    getConfig( IDXMGR_STEMMER_CLASS_K, tostr(STEM_J_B_LOVINS_68) )));
        }

        bool
        MetaFullTextIndexerInterface::supportsRankedQuery()
        {
            return isTrue(getConfig( IDXMGR_SUPPORTS_RANKED_K, "1" ));
        }

        docNumSet_t&
        MetaFullTextIndexerInterface::ExecuteXapianFullTextQuery(
            const std::string& queryString,
            docNumSet_t& docnums,
            int limit )
        {
            fh_stringstream ss;
            ss << "Attempt to resolve an Xapian query against"
               << " an index that doesn't support this query format"
               << endl;
            Throw_FullTextIndexException( tostr(ss), 0 );
        }

        docNumSet_t&
        MetaFullTextIndexerInterface::ExecuteWebFullTextQuery(
            const std::string& queryString,
            docNumSet_t& docnums,
            int limit )
        {
            fh_stringstream ss;
            ss << "Attempt to resolve a web query against"
               << " an index that doesn't support this query format"
               << endl;
            Throw_FullTextIndexException( tostr(ss), 0 );
        }
        
            
        

        docNumSet_t&
        MetaFullTextIndexerInterface::ExecuteTsearch2FullTextQuery(
            const std::string& queryString,
            docNumSet_t& docnums,
            int limit )
        {
            fh_stringstream ss;
            ss << "Attempt to resolve a TSearch2 query against"
               << " an index that doesn't support this query format"
               << endl;
            Throw_FullTextIndexException( tostr(ss), 0 );
        }
        
        docNumSet_t&
        MetaFullTextIndexerInterface::ExecuteBeagleFullTextQuery( const std::string& queryString,
                                                                  docNumSet_t& docnums,
                                                                  int limit )
        {
            fh_stringstream ss;
            ss << "Attempt to resolve a beagle query against"
               << " an index that doesn't support this query format"
               << endl;
            Throw_FullTextIndexException( tostr(ss), 0 );
        }

        docNumSet_t&
        MetaFullTextIndexerInterface::ExecuteLuceneFullTextQuery( const std::string& queryString,
                                                                  docNumSet_t& docnums,
                                                                  int limit )
        {
            fh_stringstream ss;
            ss << "Attempt to resolve a Lucene query against"
               << " an index that doesn't support this query format"
               << endl;
            Throw_FullTextIndexException( tostr(ss), 0 );
        }
            
        
        
        docNumSet_t&
        MetaFullTextIndexerInterface::ExecuteExternalFullTextQuery(
            const std::string& queryString,
            docNumSet_t& docnums,
            int limit )
        {
            fh_stringstream ss;
            ss << "Attempt to resolve a External query against"
               << " an index that doesn't support this query format"
               << endl;
            Throw_FullTextIndexException( tostr(ss), 0 );
        }
        
        docNumSet_t&
        MetaFullTextIndexerInterface::ExecuteRawFullTextQuery(
            const std::string& queryString,
            docNumSet_t& docnums,
            int limit )
        {
            LG_IDX_D << "MetaFullTextIndexerInterface::ExecuteRawFullTextQuery(1) q:" << queryString << endl;
            try
            {
                return ExecuteXapianFullTextQuery( queryString, docnums, limit );
            }
            catch( exception& e )
            {
                LG_IDX_D << "Exception running xapian fulltext query:" << e.what() << endl;
            }
            LG_IDX_D << "MetaFullTextIndexerInterface::ExecuteRawFullTextQuery(2) q:" << queryString << endl;
            try
            {
                return ExecuteTsearch2FullTextQuery( queryString, docnums, limit );
            }
            catch( exception& e )
            {
                LG_IDX_D << "Exception running tsearch2 fulltext query:" << e.what() << endl;
            }
            LG_IDX_D << "MetaFullTextIndexerInterface::ExecuteRawFullTextQuery(3) q:" << queryString << endl;
            try
            {
                return ExecuteExternalFullTextQuery( queryString, docnums, limit );
            }
            catch( exception& e )
            {
                LG_IDX_D << "Exception running external fulltext query:" << e.what() << endl;
            }
            LG_IDX_D << "MetaFullTextIndexerInterface::ExecuteRawFullTextQuery(4) q:" << queryString << endl;
            try
            {
                return ExecuteLuceneFullTextQuery( queryString, docnums, limit );
            }
            catch( exception& e )
            {
                LG_IDX_D << "Exception running lucene fulltext query:" << e.what() << endl;
            }
            

            fh_stringstream ss;
            ss << "Attempt to resolve a RAW query against"
               << " an index that doesn't support this query format"
               << " queryString:" << queryString
               << " limit:" << limit
               << endl;
            cerr << tostr(ss);
            BackTrace();
            Throw_FullTextIndexException( tostr(ss), 0 );
        }
        
        void
        MetaFullTextIndexerInterface::prepareForInsertions()
        {
        }
        
        bool
        MetaFullTextIndexerInterface::isCustomFerrisIndex()
        {
            return false;
        }

        PathManager*
        MetaFullTextIndexerInterface::tryToCastToPathManager()
        {
            return 0;
        }
        
        void
        MetaFullTextIndexerInterface::executeRankedQuery( fh_context selection,
                                                          std::string query_string,
                                                          int    m_accumulatorsMaxSize,
                                                          int    m_resultSetMaxSize )
        {
        }
        

        void
        MetaFullTextIndexerInterface::removeDocumentsMatchingRegexFromIndex( const std::string& s,
                                                                             time_t mustBeOlderThan )
        {
            stringstream ss;
            ss << "This index does not support the removal of documents.\n";
            Throw_NotSupported( tostr(ss), 0 );
        }
        
        
        bool
        MetaFullTextIndexerInterface::supportsRemove()
        {
            return false;
        }

        void
        MetaFullTextIndexerInterface::purgeDocumentInstancesOlderThan( time_t t )
        {
            stringstream ss;
            ss << "This index does not support the removal of documents.\n";
            Throw_NotSupported( tostr(ss), 0 );
        }
        
        void
        MetaFullTextIndexerInterface::removeByURL( stringlist_t& sl )
        {
            for( stringlist_t::iterator si = sl.begin(); si != sl.end(); ++si )
            {
                removeDocumentsMatchingRegexFromIndex( *si );
            }
        }

        stringlist_t
        MetaFullTextIndexerInterface::getNonResolvableURLsNotToRemoveRegexes()
        {
            string t = getConfig( IDXMGR_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_K,
                                  GET_INDEX_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_DEFAULT() );
            stringlist_t ret;
            Util::parseNullSeperatedList( t, ret );
            return ret;
        }
            
        
        void
        MetaFullTextIndexerInterface::queryFoundNonResolvableURLs( stringlist_t& sl )
        {
            stringlist_t regexes = getNonResolvableURLsNotToRemoveRegexes();
            if( !regexes.empty() )
            {
                erase_any_matches( toregexi( regexes ), sl );
            }
            removeByURL( sl );
        }
        

        
        bool
        MetaFullTextIndexerInterface::getIndexMethodSupportsIsFileNewerThanIndexedVersion()
        {
            return false;
        }
        
        bool
        MetaFullTextIndexerInterface::isFileNewerThanIndexedVersion( const fh_context& c )
        {
            return true;
        }


        MetaFullTextIndexerInterface::SignalThrottle::SignalThrottle( long fireEveryN )
            :
            m_fireEveryN( fireEveryN ),
            m_nextFireAtByte( fireEveryN )
        {
        }
        bool MetaFullTextIndexerInterface::SignalThrottle::operator()( long current )
        {
            bool ret = false;
            if( current > m_nextFireAtByte )
            {
                ret = true;
                m_nextFireAtByte += m_fireEveryN;
            }
            return ret;
        }

        MetaFullTextIndexerInterface::SignalThrottle
        MetaFullTextIndexerInterface::getSignalThrottle()
        {
            SignalThrottle ret( 256*1024 );
            return ret;
        }
       
        
        void
        MetaFullTextIndexerInterface::allWritesComplete()
        {
        }
        void
        MetaFullTextIndexerInterface::setOpenConfigReadOnly( bool v )
        {
            P->m_configShouldBeReadOnly = v;
        }
        

        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        namespace Factory
        {
            fh_idx getDefaultFullTextIndex()
            {
                static fh_idx o = 0;
                if( !o )
                {
                    o = getFullTextIndex( FULLTEXTROOT );
                }
                return o;
            }
            
            /**
             * Get the lexicon, inverted file and document map rooted
             * at the given path.
             */
            fh_idx getFullTextIndex( const std::string& basepath )
            {
                string index_classname = get_db4_string( basepath + "/" + DB_FULLTEXT,
                                                         IDXMGR_INDEX_CLASS_K,
                                                         IDXMGR_INDEX_CLASS_DEFAULT,
                                                         true, true );
                
                ensureFulltextIndexPluginFactoriesAreLoaded();
                fh_idx idx = MetaFullTextIndexerInterfaceFactory::Instance().
                    CreateObject( index_classname );
                idx->private_initialize( basepath );
                idx->LocalCommonConstruction();
                idx->CommonConstruction();
                return idx;
            }
            
            fh_docindexer makeDocumentIndexer( fh_idx idx )
            {
                if( !idx )
                    idx = getDefaultFullTextIndex();
                
                return new DocumentIndexer( idx );
            }
        };

        fh_idx createFullTextIndex( const std::string& index_classname,
                                    fh_context c,
                                    bool caseSensitive,
                                    bool dropStopWords,
                                    StemMode stemMode,
                                    const std::string& lex_class,
                                    fh_context md )
        {
            if( !md )
            {
                md = new CreateMetaDataContext();
            }
            
            if( stemMode != STEM_NONE )
            {
                if( index_classname == "lucene" )
                {
                    if( stemMode != STEM_PORTER )
                    {
                        fh_stringstream ss;
                        ss << "Invalid stemming mode for index module chosen."
                           << " lucene only supports no stemming or the porter algo."
                           << endl;
                        Throw_FullTextIndexException( tostr(ss), GetImpl(c) );
                    }
                }
                if( index_classname == "native" )
                {
                    if( stemMode != STEM_J_B_LOVINS_68 )
                    {
                        fh_stringstream ss;
                        ss << "Invalid stemming mode for index module chosen."
                           << " native only supports no stemming or the lovings_68 algo."
                           << endl;
                        Throw_FullTextIndexException( tostr(ss), GetImpl(c) );
                    }
                }
            }
            
            ensureFulltextIndexPluginFactoriesAreLoaded();
            fh_idx idx = MetaFullTextIndexerInterfaceFactory::Instance().
                CreateObject( index_classname );
            idx->P->m_base = c;
            idx->CreateIndexBeforeConfig( c, caseSensitive, dropStopWords, stemMode,
                                          lex_class, md );
            
            idx->P->ensureConfigFileCreated();
            idx->setConfig( IDXMGR_INDEX_CLASS_K,         index_classname );
            idx->setConfig( IDXMGR_CASESEN_CLASS_K,       tostr(caseSensitive));
            idx->setConfig( IDXMGR_DROPSTOPWORDS_CLASS_K, tostr(dropStopWords));
            idx->setConfig( IDXMGR_STEMMER_CLASS_K,       tostr(stemMode));
            
            idx->CreateIndex( c, caseSensitive, dropStopWords, stemMode,
                              lex_class, md );

            idx->LocalCommonConstruction();
            idx->CommonConstruction();
            return idx;
        }
        

        stringlist_t& getMetaFullTextIndexClassNames()
        {
            static stringlist_t sl;
            return sl;
        }

        bool appendToMetaFullTextIndexClassNames( const std::string& s )
        {
            getMetaFullTextIndexClassNames().push_back( s );
            return true;
        }

        
        /************************************************************/
        /************************************************************/
        /************************************************************/

        MetaFullTextIndexerInterface* CreateFullTextIndexerFromLibrary(
            const std::string& implnameraw )
        {
            std::string libname = AUTOTOOLS_CONFIG_LIBDIR + "/ferris/plugins/fulltextindexers/"
                + implnameraw;

            typedef ::Loki::Functor< MetaFullTextIndexerInterface*,
                ::Loki::NullType > CreateFunc_t;
            MetaFullTextIndexerInterface* (*CreateFuncPtr)();
                
            typedef map< std::string, CreateFunc_t > cache_t;
            static cache_t cache;

            cache_t::iterator ci = cache.find( libname );
            if( ci != cache.end() )
            {
                return ci->second();
            }
                
            GModule* ghandle = g_module_open ( libname.c_str(), G_MODULE_BIND_LAZY );
            if( !ghandle )
            {
                ostringstream ss;
                ss  << "Error, unable to open module file: "
                    << g_module_error () << endl
                    << " implementation is at:" << libname << endl
                    << endl;
                LG_PLUGIN_I << tostr(ss) << endl;
                Throw_GModuleOpenFailed( tostr(ss), 0 );
            }

            if (!g_module_symbol (ghandle, "Create", 
                                  (gpointer*)&CreateFuncPtr))
            {
                ostringstream ss;
                ss  << "Error, unable to resolve factory function in module file: "
                    << g_module_error () << endl
                    << " implementation is at:" << libname << endl
                    << endl;
                LG_PLUGIN_I << tostr(ss) << endl;
                Throw_GModuleOpenFailed( tostr(ss), 0 );
            }

            cache.insert( make_pair( libname, CreateFuncPtr ));
            return CreateFuncPtr();
            
        }
        
        
    };
};

