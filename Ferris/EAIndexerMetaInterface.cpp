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

    $Id: EAIndexerMetaInterface.cpp,v 1.30 2011/05/03 21:30:20 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Ferris/EAIndexerMetaInterface.hh>
#include <Ferris/EAIndexer_private.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/Native.hh>
#include <Ferris/Configuration_private.hh>
#include <STLdb4/stldb4.hh>
#include <Ferris/ForwardEAIndexInterface.hh>

// guessComparisonOperatorFromData
#include "FilteredContext_private.hh"

#include <Functor.h>
#include <gmodule.h>

#include "config.h"

using namespace std;

#include <boost/regex.hpp>

namespace Ferris
{
    
    namespace EAIndex 
    {
        using namespace ::STLdb4;

        const string EAINDEXROOT = "~/.ferris/ea-index";
        const string DB_EAINDEX  = "ea-index-config.db";
        
        class FERRISEXP_DLLLOCAL MetaEAIndexerInterfacePriv
        {
        public:
            MetaEAIndexerInterfacePriv()
                :
                m_config( 0 ), m_configNotAvailable( false ), m_base( 0 ),
                m_indexingNativeContext( false ),
                m_configShouldBeReadOnly( false ),
                have_EXPLICIT_EANAMES_TO_INDEX_STRINGLIST( false ),
                virgin_EXPLICIT_EANAMES_TO_INDEX_STRINGLIST( true ),
                m_filesIndexedCount( 0 )
//                 m_dontStoreZeroIntegerAttributes( -1 ),
//                 m_dontStoreEmptyStringAttributes( -1 )
                {}

            stringmap_t     m_configCache;
            fh_database     m_config;
            bool            m_configNotAvailable;
            bool            m_configShouldBeReadOnly;
            bool            m_indexingNativeContext;
            fh_database     getConfigDB();
            fh_context      m_base;
            Time::RelativeTimeOrIntegerStringParser m_timeOrIntParser;
//             int             m_dontStoreZeroIntegerAttributes;
//             int             m_dontStoreEmptyStringAttributes;

            bool             virgin_EXPLICIT_EANAMES_TO_INDEX_STRINGLIST;
            bool             have_EXPLICIT_EANAMES_TO_INDEX_STRINGLIST;
            stringlist_t     m_EXPLICIT_EANAMES_TO_INDEX_STRINGLIST;
            int              m_filesIndexedCount;
            
            /**
             * Ensure that there is a db4 file there for setConfig() / getConfig() to use
             */
            void ensureConfigFileCreated()
                {
                    if( !m_config )
                    {
                        m_config = ensureFerrisConfigFileExists( m_base->getDirPath(), DB_EAINDEX );
                        set_db4_string( getConfigDB(), "foo1", "bar" );
                        getConfigDB()->sync();
                    }
                }
        };

        fh_database
        MetaEAIndexerInterfacePriv::getConfigDB()
        {
            if( m_config || m_configNotAvailable )
                return m_config;

            string dbfilename = CleanupURL( m_base->getDirPath() + "/" + DB_EAINDEX );
//            cerr << "CONFIG FILE AT:" << dbfilename << endl;
            
            try
            {
//                m_config = new Database( dbfilename );

                bool readOnly = m_configShouldBeReadOnly;
//                cerr << "m_configShouldBeReadOnly:" << m_configShouldBeReadOnly << endl;
                if( !access( dbfilename.c_str(), W_OK ) )
                    readOnly = false;
                
                
                m_config = new Database( dbfilename, "", readOnly );

//                 u_int32_t flags = 0;
//                 if( m_configShouldBeReadOnly ) flags |= DB_RDONLY;
//                 else                           flags |= DB_CREATE;
                
//                 m_config = new Database( DB_UNKNOWN, dbfilename, "", flags );
            }
            catch( exception& e )
            {
                m_configNotAvailable = true;
            }
            return m_config;
        }

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        void
        MetaEAIndexerInterface::addDocID( docNumSet_t& output, docid_t v )
        {
            output.insert( v );
        }
        
        MetaEAIndexerInterface::MetaEAIndexerInterface()
            :
            P( new MetaEAIndexerInterfacePriv() )
        {
        }

        MetaEAIndexerInterface::~MetaEAIndexerInterface()
        {
            delete P;
        }
        
        void
        MetaEAIndexerInterface::LocalCommonConstruction()
        {
            P->ensureConfigFileCreated();
        }
        
        void
        MetaEAIndexerInterface::private_initialize( const std::string& basepath )
        {
            P->m_base = Shell::acquireContext( basepath );
            Setup();
        }
        
        std::string
        MetaEAIndexerInterface::getPath()
        {
            return P->m_base->getDirPath();
        }

        std::string
        MetaEAIndexerInterface::getURL()
        {
            return P->m_base->getURL();
        }

        static int TMP_VISITINGCONTEXT_SLEA_COUNT = 0;
        static int TMP_VISITINGCONTEXT_STATEEA_COUNT = 0;
        
        void
        MetaEAIndexerInterface::visitingContext( const fh_context& c )
        {
//             cerr << "MetaEAIndexerInterface::visitingContext() "
//                  << " SLEA#:" << TMP_VISITINGCONTEXT_SLEA_COUNT
//                  << " NONSL#:" << TMP_VISITINGCONTEXT_STATEEA_COUNT
//                  << endl;
            TMP_VISITINGCONTEXT_SLEA_COUNT = 0;
            TMP_VISITINGCONTEXT_STATEEA_COUNT = 0;
             
            Context* cp = GetImpl( c );
            P->m_indexingNativeContext = dynamic_cast< NativeContext* >( cp );
        }

        static bool
        haveEAIndexExplicitWhiteListFromEnv()
        {
            static bool ret = g_getenv ("LIBFERRIS_EAINDEX_EXPLICIT_WHITELIST") != 0;
            return ret;
        }
        
        static stringset_t&
        getEAIndexExplicitWhiteListFromEnv()
        {
            static stringset_t cache;

            if( const gchar* p = g_getenv ("LIBFERRIS_EAINDEX_EXPLICIT_WHITELIST") )
            {
                LG_EAIDX_D << "Should only accept EA in list:" << p << endl;
                
                if( strlen(p) && cache.empty() )
                {
                    Util::parseSeperatedList( p, cache, inserter(cache,cache.end()) );
                }
            }
            return cache;
        }

        static bool
        haveEAIndexExplicitWhiteListRegexFromEnv()
        {
            static bool ret = g_getenv ("LIBFERRIS_EAINDEX_EXPLICIT_WHITELIST_REGEX") != 0;
            return ret;
        }
        
        static fh_rex&
        getEAIndexExplicitWhiteListRegexFromEnv()
        {
            static fh_rex cache = 0;

            if( !cache )
            {
                if( const gchar* p = g_getenv ("LIBFERRIS_EAINDEX_EXPLICIT_WHITELIST_REGEX") )
                {
                    LG_EAIDX_D << "Should only accept EA in list:" << p << endl;
                
                    if( strlen(p) )
                    {
                        cache = toregexh( p, boost::regex::optimize );
                    }
                }
            }
            return cache;
        }
        
        static stringset_t getDigestEANames()
        {
            static stringset_t ret;
            
            if( ret.empty() )
            {
                ret.insert("md2");
                ret.insert("md5");
                ret.insert("sha1");
                ret.insert("mdc2");
                ret.insert("crc32");
                ret.insert("crc");
            }
            return ret;
        }

        static bool
        haveEAIndexExplicitBlackListFromEnv()
        {
            static bool ret = g_getenv ("LIBFERRIS_EAINDEX_EXPLICIT_BLACKLIST") != 0;
            return ret;
        }
        
        static stringset_t&
        getEAIndexExplicitBlackListFromEnv()
        {
            static stringset_t cache;

            if( const gchar* p = g_getenv ("LIBFERRIS_EAINDEX_EXPLICIT_BLACKLIST") )
            {
                LG_EAIDX_D << "Should only accept EA in list:" << p << endl;
                
                if( strlen(p) && cache.empty() )
                {
                    Util::parseSeperatedList( p, cache, inserter(cache,cache.end()) );
                }
            }
            return cache;
        }

        static bool
        haveEAIndexExplicitBlackListDigestsFromEnv()
        {
            static bool ret = g_getenv ("LIBFERRIS_EAINDEX_EXPLICIT_BLACKLIST_DIGESTS") != 0;
            return ret;
        }
        
        bool
        MetaEAIndexerInterface::shouldIndex( fh_context c, fh_docindexer di,
                                             const std::string& attributeName )
        {
//             cerr << "MetaEAIndexerInterface::shouldIndex() di->getIgnoreEANames().sz:"
//                  << di->getIgnoreEANames().size() << endl;
//             cerr << "beg:" << *(di->getIgnoreEANames().begin()) << endl;

            if( haveEAIndexExplicitWhiteListFromEnv() )
            {
                stringset_t& cache = getEAIndexExplicitWhiteListFromEnv();
                
                if( cache.count( attributeName ) )
                {
                    return true;
                }
                return false;
            }

            if( haveEAIndexExplicitWhiteListRegexFromEnv() )
            {
                fh_rex r = getEAIndexExplicitWhiteListRegexFromEnv();
                if( regex_match( attributeName, r ) )
                    return true;
                
                return false;
            }
            

            if( haveEAIndexExplicitBlackListFromEnv() )
            {
                if( getEAIndexExplicitBlackListFromEnv().count( attributeName ) )
                {
                    return false;
                }
            }
            if( haveEAIndexExplicitBlackListDigestsFromEnv() )
            {
                if( getDigestEANames().count( attributeName ) )
                {
                    return false;
                }
            }

            
            
            if( di->getIgnoreEANames().find( attributeName )
                != di->getIgnoreEANames().end() )
            {
                LG_EAIDX_D << "shouldIndex attr:" << attributeName
                           << " is in ignoreEANames, skipping it" << endl;
                return false;
            }

            if( fh_rex r = di->getIgnoreEARegexs().getRegex() )
            {
                if( regex_match( attributeName, r, boost::match_any ) )
                {
//                    cerr << "shouldIndex attr:" << attributeName << " is in ignoreEANames Regex" << endl;
                    return false;
                }
            }

            if( !di->EANamesToIndexContains( attributeName ))
                return false;

            if( !di->EANamesToIndexRegexContains( attributeName ))
                return false;
            
            return true;
        }

        bool
        MetaEAIndexerInterface::obtainValueIfShouldIndex( fh_context c, fh_docindexer di,
                                                          const std::string& attributeName,
                                                          std::string& v )
        {
            bool acceptNullValues = false;
            bool dummy;
            return obtainValueIfShouldIndex( c, di, attributeName, v, acceptNullValues, dummy );
        }
        

        bool
        MetaEAIndexerInterface::obtainValueIfShouldIndex(
            fh_context c, fh_docindexer di,
            const std::string& attributeName,
            std::string& v,
            bool acceptNullValues,
            bool& isNULL )
        {
            LG_EAIDX_D << " start, attr:" << attributeName << endl; 

            if( attributeName == "subtitles-local" )
                return false;
            
            if( !shouldIndex( c, di, attributeName ))
                return false;

            LG_EAIDX_D << " should read, attr:" << attributeName << endl; 
            
            string k = attributeName;
            v.clear();
            bool CanReadValue = true;
            try
            {
                LG_EAIDX_D << "Getting attribute:" << k << endl;
                v = getStrAttr( c, k, "", true, true );
                LG_EAIDX_D << "Got attribute:" << k << endl;
//                 if( k == "ctime" || k == "mtime" || k == "atime" )
//                     LG_EAIDX_D << "Got value:" << v << endl;
            }
            catch( exception& e )
            {
                LG_EAIDX_D << "error getting attribute:" << k
                           << " e:" << e.what()
                           << endl;
                CanReadValue = false;
            }


            if( !CanReadValue )
            {
                if( acceptNullValues )
                {
                    isNULL = true;
                    return true;
                }
                LG_EAIDX_D << "Failed to read attribute:" << k << endl;
                return false;
            }
            if( v.length() > di->getMaximunValueSize() )
            {
                LG_EAIDX_W << "value to large for attribute:" << k
                           << " max allowed:" << di->getMaximunValueSize()
                           << " v.size:" << v.length()
                           << endl;
                if( k == "subtitles" )
                {
                    if( v.length() > 1024*1024 )
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            if( v.empty() )
                return false;
            

//             if( getDontStoreEmptyStringAttributes()
//                 || getDontStoreZeroIntegerAttributes() )
//             {
//                 IndexableValue iv  = getIndexableValue( c, k, v );
//                 AttrType_t att = iv.getAttrTypeID();
                
//                 if( getDontStoreEmptyStringAttributes() &&
//                     ( att == ATTRTYPEID_STR || att == ATTRTYPEID_CIS )
//                     && v.empty() )
//                 {
//                     return false;
//                 }
//                 else if( getDontStoreZeroIntegerAttributes()
//                          && v == "0" )
//                 {
//                     return false;
//                 }
//             }

            isNULL = false;
            return true;
        }

//         static const char* IDXMGR_DONT_STORE_ZERO_INTEGER_ATTRIBUTES_K
//         = "idxmgr_dont_store_zero_integer_attributes_k";
//         static const char* IDXMGR_DONT_STORE_EMPTY_STRING_ATTRIBUTES_K
//         = "idxmgr_dont_store_empty_string_attributes_k";
        
//         bool MetaEAIndexerInterface::getDontStoreZeroIntegerAttributes()
//         {
//             if( P->m_dontStoreZeroIntegerAttributes < 0 )
//             {
//                 P->m_dontStoreZeroIntegerAttributes
//                     = isTrue(
//                         getConfig(
//                             IDXMGR_DONT_STORE_ZERO_INTEGER_ATTRIBUTES_K, "0" ));
//             }
            
//             return P->m_dontStoreEmptyStringAttributes;
//         }
        
//         bool MetaEAIndexerInterface::getDontStoreEmptyStringAttributes()
//         {
//             if( P->m_dontStoreEmptyStringAttributes < 0 )
//             {
//                 P->m_dontStoreEmptyStringAttributes
//                     = isTrue(
//                         getConfig(
//                             IDXMGR_DONT_STORE_EMPTY_STRING_ATTRIBUTES_K, "0" ));
//             }
//             return P->m_dontStoreEmptyStringAttributes;
//         }
        
//         void MetaEAIndexerInterface::setDontStoreZeroIntegerAttributes( bool v )
//         {
//             P->m_dontStoreZeroIntegerAttributes = v;
//             setConfig( IDXMGR_DONT_STORE_ZERO_INTEGER_ATTRIBUTES_K,
//                        tostr( v ) );
//         }
        
//         void MetaEAIndexerInterface::setDontStoreEmptyStringAttributes( bool v )
//         {
//             P->m_dontStoreEmptyStringAttributes = v;
//             setConfig( IDXMGR_DONT_STORE_EMPTY_STRING_ATTRIBUTES_K,
//                        tostr( v ) );
//         }
        
        
        stringlist_t&
        MetaEAIndexerInterface::getEANamesToIndex( fh_context& c, stringlist_t& ret )
        {
            if( haveEAIndexExplicitWhiteListFromEnv() )
            {
                LG_EAIDX_D << "getEANamesToIndex() explicit from env." << endl;
                stringset_t& cache = getEAIndexExplicitWhiteListFromEnv();
                copy( cache.begin(), cache.end(), back_inserter( ret ));
                return ret;
            }
            
            if( P->virgin_EXPLICIT_EANAMES_TO_INDEX_STRINGLIST )
            {
                P->virgin_EXPLICIT_EANAMES_TO_INDEX_STRINGLIST = false;
                string s = getConfig( IDXMGR_EXPLICIT_EANAMES_TO_INDEX_K, "" );
                stringlist_t& sl = P->m_EXPLICIT_EANAMES_TO_INDEX_STRINGLIST;
                Util::parseCommaSeperatedList( s, sl );
                if( !sl.empty() )
                {
                    LG_EAIDX_D << "Have explicit eanames stringlist! s:" << s << endl;
                    P->have_EXPLICIT_EANAMES_TO_INDEX_STRINGLIST = true;
                }
            }
            
            if( P->have_EXPLICIT_EANAMES_TO_INDEX_STRINGLIST )
            {
                stringlist_t& an = P->m_EXPLICIT_EANAMES_TO_INDEX_STRINGLIST;
                copy( an.begin(), an.end(), back_inserter( ret ));
                LG_EAIDX_D << "Have explicit names to index... an.size:" << an.size() << endl;
            }
            else
            {
                AttributeCollection::AttributeNames_t an;
                c->getAttributeNames( an );
                copy( an.begin(), an.end(), back_inserter( ret ));
//            Util::parseCommaSeperatedList( getStrAttr( c, "ea-names", "" ), ret );
            }


            if( haveEAIndexExplicitBlackListFromEnv() || haveEAIndexExplicitBlackListDigestsFromEnv() )
            {
                if( haveEAIndexExplicitBlackListFromEnv() )
                {
                    stringlist_t tmp;

                    stringset_t blackset = getEAIndexExplicitBlackListFromEnv();
                    if( haveEAIndexExplicitBlackListDigestsFromEnv() )
                        blackset.insert( getDigestEANames().begin(), getDigestEANames().end() );
                    
                    for( stringlist_t::iterator iter = ret.begin(); iter != ret.end(); ++iter )
                    {
                        if( !blackset.count( *iter ) )
                        {
                            tmp.push_back( *iter );
                        }
                    }
                    ret.clear();
                    copy( tmp.begin(), tmp.end(), back_inserter( ret ) );
                }
            }

            if( haveEAIndexExplicitWhiteListRegexFromEnv() )
            {
                stringlist_t tmp;
                fh_rex r = getEAIndexExplicitWhiteListRegexFromEnv();

                for( stringlist_t::iterator iter = ret.begin(); iter != ret.end(); ++iter )
                {
                    if( regex_match( *iter, r ) )
                        tmp.push_back( *iter );
                }
                ret.clear();
                copy( tmp.begin(), tmp.end(), back_inserter( ret ) );
            }
            
            return ret;
        }


        
        
        
        std::string
        MetaEAIndexerInterface::getConfig( const std::string& k, const std::string& def, bool throw_for_errors )
        {
            fh_database db = P->getConfigDB();

//            cerr << "getConfig() k:" << k << " P->m_configNotAvailable:" << P->m_configNotAvailable << endl;
            
            if( P->m_configNotAvailable && !throw_for_errors )
                return def;

            stringmap_t::const_iterator ci = P->m_configCache.find( k );
            if( ci != P->m_configCache.end() )
                return ci->second;
            
//            cerr << "getConfig() k:" << k << endl;
            string ret = get_db4_string( db, k, def, throw_for_errors );
            P->m_configCache[ k ] = ret;

//            cerr << "getConfig() k:" << k << " ret.sz:" << ret.length() << endl;
//            cerr << "getConfig() k:" << k << " ret:" << ret << endl;
            
            return ret;
        }
        
        void
        MetaEAIndexerInterface::setConfig( const std::string& k, const std::string& v )
        {
//             cerr << "MetaEAIndexerInterface::setConfig() k:" << k
//                  << " v:" << v << endl;

            P->m_configCache[ k ] = v;
            
            fh_database db = P->getConfigDB();
            set_db4_string( db, k, v );
            db->sync();
        }

        guint64
        MetaEAIndexerInterface::convertStringToInteger( const std::string& v )
        {
            if( v.empty() )
                return 0;
            if( v[0] == '\0' )
                return 0;
            return P->m_timeOrIntParser.convert( v );
        }

        void
        MetaEAIndexerInterface::setFulltextIndex( const std::string& path )
        {
            FullTextIndex::fh_idx fidx = FullTextIndex::Factory::getFullTextIndex( path );
            setFulltextIndex( fidx );
        }
        void
        MetaEAIndexerInterface::setFulltextIndex( FullTextIndex::fh_idx fidx )
        {
            setConfig( IDXMGR_FULLTEXT_INDEX_PATH_K, fidx->getPath() );
        }
        
        FullTextIndex::fh_idx
        MetaEAIndexerInterface::getFulltextIndex()
        {
            string p = getFulltextIndexPath();
            return FullTextIndex::Factory::getFullTextIndex( p );
        }
        
        

        std::string
        MetaEAIndexerInterface::getFulltextIndexPath()
        {
            return getConfig( IDXMGR_FULLTEXT_INDEX_PATH_K, "" );
        }
        
        
        std::string
        MetaEAIndexerInterface::getEANamesIgnore()
        {
//              cerr << "MetaEAIndexerInterface::getEANamesIgnore() s:"
//                   << getConfig( IDXMGR_EANAMES_IGNORE_K, "can't read" )
//                   << endl;
            return getConfig( IDXMGR_EANAMES_IGNORE_K,
                              GET_EAINDEX_EANAMES_NOT_TO_INDEX_DEFAULT() );
        }

        void
        MetaEAIndexerInterface::appendToEANamesIgnore( const std::string& s )
        {
            fh_stringstream ss;
            ss << getEANamesIgnore() << "," << s;
            setConfig( IDXMGR_EANAMES_IGNORE_K, tostr(ss) );
        }
        
        stringlist_t
        MetaEAIndexerInterface::getEANamesIgnoreRegexes()
        {
            string t = getConfig( IDXMGR_EANAMES_REGEX_IGNORE_K,
                                  GET_EAINDEX_EANAMES_REGEX_IGNORE_DEFAULT() );
            stringlist_t ret;
            Util::parseNullSeperatedList( t, ret );
            return ret;
        }

        std::string
        MetaEAIndexerInterface::getEANamesToIndexRegex()
        {
            return getConfig( IDXMGR_EANAMES_TO_INDEX_REGEX_K,
                              GET_EAINDEX_EANAMES_TO_INDEX_REGEX_DEFAULT() );
        }

        
        stringlist_t
        MetaEAIndexerInterface::getNonResolvableURLsNotToRemoveRegexes()
        {
            string t = getConfig( IDXMGR_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_K,
                                  GET_EAINDEX_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_DEFAULT() );
            stringlist_t ret;
            Util::parseNullSeperatedList( t, ret );
            return ret;
        }
            
        
        std::streamsize
        MetaEAIndexerInterface::getMaxValueSize()
        {
            string def = GET_EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT();
            return toType<std::streamsize>(
                getConfig( IDXMGR_MAX_VALUE_SIZE_K, def ));
        }

        
        
        static fh_idx createEAIndexInstance( const std::string& index_classname )
        {
            ensureEAIndexPluginFactoriesAreLoaded();
            
            fh_idx idx = MetaEAIndexerInterfaceFactory::Instance().
                CreateObject( index_classname );
            
            return idx;
        }
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        namespace Factory
        {
            static fh_idx& getDefaultEAIndexSingleton()
            {
                static fh_idx ret = 0;
                return ret;
            }
            
            fh_idx getDefaultEAIndex()
            {
                fh_idx& o = getDefaultEAIndexSingleton();
                if( !o )
                {
                    o = getEAIndex( EAINDEXROOT );
                }
                return o;
            }

            void
            setDefaultEAIndexPath( const char* EAIndexPath_CSTR )
            {
                if( EAIndexPath_CSTR )
                {
                    fh_idx idx = getEAIndex( EAIndexPath_CSTR );
                    getDefaultEAIndexSingleton() = idx;
                }
            }
            
            
            
            /**
             * Get the lexicon, inverted file and document map rooted
             * at the given path.
             */
            fh_idx getEAIndex( const std::string& basepath_const )
            {
                string basepath = basepath_const;
                LG_IDX_D << "getEAIndex(1) basepath:" << basepath << endl;
                if( starts_with( basepath, "eaindexes:" ) )
                {
                    fh_context c = Resolve( basepath );
                    basepath = c->getDirPath();
                    LG_IDX_D << "getEAIndex(2) basepath:" << basepath << endl;
                }
                else if( !starts_with( basepath, "/" ) )
                {
                    try
                    {
                        fh_context c = Resolve( basepath );
                    }
                    catch( exception& e )
                    {
                        try
                        {
                            stringstream ss;
                            ss << "eaindexes://" << basepath;
                            fh_context c = Resolve( ss.str() );
                            basepath = c->getDirPath();
                        }
                        catch( exception& e )
                        {
                        }
                    }
                }
                LG_IDX_D << "getEAIndex(3) basepath:" << basepath << endl;
                
                
                string index_classname = get_db4_string( basepath + "/" + DB_EAINDEX,
                                                         EAIDXMGR_INDEX_CLASS_K,
                                                         EAIDXMGR_INDEX_CLASS_DEFAULT,
                                                         true, true );
                LG_IDX_D << "getEAIndex(3.1) classname:" << index_classname << endl;
                
                fh_idx idx = createEAIndexInstance( index_classname );
                idx->private_initialize( basepath );
                idx->LocalCommonConstruction();
                idx->CommonConstruction();
                return idx;
            }
            
            fh_docindexer makeDocumentIndexer( fh_idx idx )
            {
                if( !idx )
                    idx = getDefaultEAIndex();
                
                return new DocumentIndexer( idx );
            }
        };

        fh_idx createEAIndex( const std::string& index_classname,
                              fh_context c,
                              fh_context md )
        {
            if( !md )
            {
                md = new CreateMetaDataContext();
            }

            LG_IDX_D << "createEAIndex(top)" << endl;
            LG_IDX_D << "createEAIndex() className:" << index_classname
                     << endl;
            
            fh_idx idx = createEAIndexInstance( index_classname );
            idx->P->m_base = c;
            idx->CreateIndexBeforeConfig( c, md );
            
            idx->P->ensureConfigFileCreated();
            idx->setConfig( EAIDXMGR_INDEX_CLASS_K,         index_classname );

            string attributesNotToIndex
                = getStrSubCtx( md, "attributes-not-to-index",
                                GET_EAINDEX_EANAMES_NOT_TO_INDEX_DEFAULT() );
            string attributesNotToIndexRegex
                = getStrSubCtx( md, "attributes-not-to-index-regex",
                                GET_EAINDEX_EANAMES_REGEX_IGNORE_DEFAULT() );
            replace( attributesNotToIndexRegex.begin(), attributesNotToIndexRegex.end(),
                     ',','\0' );
            {
                string s = getStrSubCtx( md, "attributes-not-to-index-regex-append", "" );
                if( !s.empty() )
                {
                    LG_EAIDX_D << "append-attr-notToIndexRegex:" << s << endl;
                    cerr << "append-attr-notToIndexRegex:" << s << endl;
                    replace( s.begin(), s.end(), ',','\0' );
                    stringstream ss;
                    if( !attributesNotToIndexRegex.empty() )
                        ss << "(" << attributesNotToIndexRegex << "|";
                    ss << s;
                    if( !attributesNotToIndexRegex.empty() )
                        ss << ")";
                    attributesNotToIndexRegex = ss.str();
                    cerr << "append-attr-notToIndexRegex2. new attributesNotToIndexRegex:" << attributesNotToIndexRegex << endl;

                    // PURE DEBUG
                    {
                        string t = attributesNotToIndexRegex;
                        replace( t.begin(), t.end(), '\0', ',' );
                        LG_EAIDX_D << "final-appended-attributesNotToIndexRegex:" << t << endl;
                    }
                }
            }
            
            string maxValueSize   =
                getStrSubCtx( md, "max-value-size-to-index",
                              GET_EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT() );

            idx->setConfig( IDXMGR_EANAMES_IGNORE_K,       attributesNotToIndex );
            idx->setConfig( IDXMGR_EANAMES_REGEX_IGNORE_K, attributesNotToIndexRegex );
            idx->setConfig( IDXMGR_MAX_VALUE_SIZE_K,       maxValueSize );

            string attributesToIndexRegex
                = getStrSubCtx( md, "attributes-to-index-regex",
                                GET_EAINDEX_EANAMES_TO_INDEX_REGEX_DEFAULT() );
            idx->setConfig( IDXMGR_EANAMES_TO_INDEX_REGEX_K, attributesToIndexRegex );
            
            idx->CreateIndex( c, md );

            idx->LocalCommonConstruction();
            idx->CommonConstruction();

            return idx;
        }
        
        namespace Factory
        {
            fh_idx createTemporaryEAIndexFederation( ctxlist_t   eaindexctxlist, fh_context md )
            {
                ctxlist_t::iterator iter = eaindexctxlist.begin();
                ctxlist_t::iterator    e = eaindexctxlist.end();

                eaidxlist_t eaindexlist;
                for( ; iter != e; ++iter )
                {
                    fh_idx idx = getEAIndex( (*iter)->getURL() );
                    eaindexlist.push_back( idx );
                }
                return createTemporaryEAIndexFederation( eaindexlist, md );
            }
        
            fh_idx createTemporaryEAIndexFederation( eaidxlist_t eaindexlist, fh_context mdx )
            {
                string ts = "-tmp-eaindexfederation-";
                fh_context tc = Shell::generateTempDir( ts );

                LG_IDX_D << "createTemporaryEAIndexFederation(1)" << endl;
                //
                // make the new federation ea index
                //
                fh_mdcontext md = new f_mdcontext();
                fh_mdcontext child = md->setChild( "eaindexfederation", "" );
                child->setChild( "rdn", "eaindexfederation" );
                child->setChild( "name", ts );
                child->setChild( "primary-write-index-url", "~/.ferris/ea-index" );
                tc->createSubContext( "", md );
                LG_IDX_D << "createTemporaryEAIndexFederation(2)" << endl;

                fh_idx fedidx = getEAIndex( ts );
                LG_IDX_D << "createTemporaryEAIndexFederation(3)" << endl;

                eaidxlist_t::iterator iter = eaindexlist.begin();
                eaidxlist_t::iterator    e = eaindexlist.end();
                for( ; iter != e; ++iter )
                {
                    fh_idx subidx = *iter;
                    addToTemporaryEAIndexFederation( fedidx, subidx );
                }
            
                LG_IDX_D << "createTemporaryEAIndexFederation(done)" << endl;
                return fedidx;
            }
        }

        static const char* CFG_IDX_FEDERATION_URLS_K = "cfg-idx-federation-urls-k";

        static fh_database getDB( EAIndex::fh_idx eidx )
        {
            string dbfilename = CleanupURL( eidx->getPath() + "/" + EAIndex::DB_EAINDEX );
            fh_database db = new Database( dbfilename );
            return db;
        }

        static string getConfig( fh_database db, const std::string& k, const std::string& def )
        {
            string ret = get_db4_string( db, k, def, true );
            return ret;
        }

        static void setConfig( fh_database db, const std::string& k, const std::string& v )
        {
            set_db4_string( db, k, v );
            db->sync();
        }
        
        void addToTemporaryEAIndexFederation( fh_idx fedidx, fh_idx subidx )
        {
            fh_database db = getDB( fedidx );
        
            stringlist_t sl = Util::parseSeperatedList( getConfig( db, CFG_IDX_FEDERATION_URLS_K, "" ) );
            sl.push_back( subidx->getURL() );
            setConfig( db, CFG_IDX_FEDERATION_URLS_K, Util::createSeperatedList( sl ) );
        }
        
        

        stringlist_t& getMetaEAIndexClassNames()
        {
            static stringlist_t sl;
            return sl;
        }

        bool appendToMetaEAIndexClassNames( const std::string& s )
        {
            getMetaEAIndexClassNames().push_back( s );
            return true;
        }

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        static fh_regex getIsCaseSensitiveRegex()
        {
            fh_regex ret = 0;
            if( !ret )
            {
//                ret = new Regex("^[a-zA-Z0-9\\'\\.\\*\\-\\_\\ ]+$");
                ret = new Regex(".*[A-Z]+.*");
            }
            return ret;
        }
        
        MetaEAIndexerInterface::AttrType_t
        MetaEAIndexerInterface::IndexableValue::getAttrTypeID()  const
        {
            return att;
        }
                
        XSDBasic_t
        MetaEAIndexerInterface::IndexableValue::getSchemaType() const
        {
            return sct;
        }

        bool
        MetaEAIndexerInterface::IndexableValue::isCaseSensitive() const
        {
            return getAttrTypeID() != ATTRTYPEID_CIS;
        }
        
        const string&
        MetaEAIndexerInterface::IndexableValue::rawValueString() const
        {
            return value;
        }

        const std::string&
        MetaEAIndexerInterface::IndexableValue::rawEANameString() const
        {
            return eaname;
        }
        
        MetaEAIndexerInterface::IndexableValue::IndexableValue( MetaEAIndexerInterface* midx,
                                                                const std::string& eaname,
                                                                const std::string& value )
            :
            eaname( eaname ),
            value( value ),
            sct( XSD_UNKNOWN )
        {
            att = midx->inferAttrTypeID( eaname, value );
            LG_EAIDX_D << "IndexableValue() eaname:" << eaname
                       << " att:" << att
                      // << " value:" << value
                       << endl;
        }

        struct FERRISEXP_DLLLOCAL StatelessEAMetaData
        {
            MetaEAIndexerInterface::AttrType_t att;
            XSDBasic_t sct;
            StatelessEAMetaData( XSDBasic_t sct = XSD_UNKNOWN,
                                 MetaEAIndexerInterface::AttrType_t att
                                 = MetaEAIndexerInterface::ATTRTYPEID_CIS )
                : sct( sct ), att( att )
                {
                }
        };
        typedef map< string, StatelessEAMetaData > StatelessEAMetaDataCache_t;
        StatelessEAMetaDataCache_t&
        getStatelessEAMetaDataCache()
        {
            static StatelessEAMetaDataCache_t ret;
            return ret;
        }
        

        void
        MetaEAIndexerInterface::ensureStatelessEAMetaDataCachePopulated(
            fh_context& c, const stringset_t& sl )
        {
                for( stringset_t::const_iterator si = sl.begin(); si != sl.end(); ++si )
                {
                    string eaname = *si;
                    MetaEAIndexerInterface::AttrType_t att;
                    XSDBasic_t sct;

                    fh_context sc = c->getSchema( eaname );
                    sct = ::Ferris::getSchemaType( sc, XSD_UNKNOWN );
                    string cop = getSchemaDefaultSort( sc );
                    att = inferAttrTypeID( eaname, "", cop );
                    
                    getStatelessEAMetaDataCache()[ eaname ] =
                        StatelessEAMetaData( sct, att );
                }
        }
        

        void
        MetaEAIndexerInterface::ensureStatelessEAMetaDataCachePopulated( fh_context& c )
        {
            static bool virgin = true;
            if( virgin )
            {
                virgin = false;
                ensureStatelessEAMetaDataCachePopulated(
                    c, getNativeStatelessEANames() );
                ensureStatelessEAMetaDataCachePopulated(
                    c, Context::getContextClassStatelessEANames() );
            }
        }
        
        
        MetaEAIndexerInterface::IndexableValue::IndexableValue( MetaEAIndexerInterface* midx,
                                                                fh_context c,
                                                                const std::string& eaname,
                                                                const std::string& value )
            :
            eaname( eaname ),
            value( value ),
            sct( XSD_UNKNOWN ),
            att( ATTRTYPEID_CIS )
        {
            midx->ensureStatelessEAMetaDataCachePopulated( c );

            bool haveSchemaData = false;
            
            if( midx->P->m_indexingNativeContext )
            {
                StatelessEAMetaDataCache_t::const_iterator sleaiter =
                    getStatelessEAMetaDataCache().find( eaname );
                if( sleaiter != getStatelessEAMetaDataCache().end() )
                {
                    sct = sleaiter->second.sct;
                    att = sleaiter->second.att;
                    haveSchemaData = true;
                    TMP_VISITINGCONTEXT_SLEA_COUNT++;
                }
            }
            if( !haveSchemaData )
            {
                TMP_VISITINGCONTEXT_STATEEA_COUNT++;
//                cerr << "NONSL:" << eaname << endl;
                fh_context sc = c->getSchema( eaname );
                sct = ::Ferris::getSchemaType( sc, XSD_UNKNOWN );
                string cop = getSchemaDefaultSort( sc );
                att = midx->inferAttrTypeID( eaname, value, cop );
            }
            
            
//             LG_EAIDX_D << "IndexableValue() ea:" << eaname
//                        << " value:" << value
//                        << " sct:" << sct
//                        << " att:" << att
//                        << endl;

            
            if( sct == XSD_BASIC_DOUBLE || sct == XSD_BASIC_FLOAT )
                att = ATTRTYPEID_DBL;
            if( sct == FXD_UNIXEPOCH_T )
                att = ATTRTYPEID_TIME;
        }
        
        MetaEAIndexerInterface::AttrTypeList_t
        MetaEAIndexerInterface::getAllAttrTypes()
        {
            AttrTypeList_t ret;

            ret.push_back( ATTRTYPEID_STR );
            ret.push_back( ATTRTYPEID_INT );
            ret.push_back( ATTRTYPEID_DBL );
            ret.push_back( ATTRTYPEID_TIME );

            return ret;
        }
        
        MetaEAIndexerInterface::IndexableValue
        MetaEAIndexerInterface::getIndexableValueFromToken( const std::string& eaname,
                                                            fh_context tc )
        {
            return IndexableValue( this, eaname,
                                   getStrAttr( tc, "token", "" ) );
        }

        MetaEAIndexerInterface::IndexableValue
        MetaEAIndexerInterface::getIndexableValue( fh_context c,
                                                   const std::string& eaname,
                                                   const std::string& value )
        {
            return IndexableValue( this, c, eaname, value );
        }
        
        string
        MetaEAIndexerInterface::asString( IndexableValue& v )
        {
            return asString( v, v.getAttrTypeID() );
        }
        
        string
        MetaEAIndexerInterface::asString( IndexableValue& v, AttrType_t att )
        {
            switch( v.getAttrTypeID() )
            {
            case ATTRTYPEID_INT:
            case ATTRTYPEID_DBL:
            case ATTRTYPEID_TIME:
                return tostr(convertStringToInteger( v.rawValueString() ));
            case ATTRTYPEID_STR:
                if( v.isCaseSensitive() )
                    return v.rawValueString();
                else
                    return tolowerstr()( v.rawValueString() );
            }
            return v.rawValueString();
        }
        

        MetaEAIndexerInterface::AttrType_t
        MetaEAIndexerInterface::inferAttrTypeID( IndexableValue& iv )
        {
            return inferAttrTypeID( iv.rawEANameString(),
                                    iv.rawValueString() );
        }

        MetaEAIndexerInterface::AttrType_t
        MetaEAIndexerInterface::inferAttrTypeID( const std::string& eaname,
                                                 const std::string& value )
        {
            string cop = guessComparisonOperatorFromData( value );
            return inferAttrTypeID( eaname, value, cop );
        }
        
        MetaEAIndexerInterface::AttrType_t
        MetaEAIndexerInterface::inferAttrTypeID( const std::string& eaname,
                                                 const std::string& value,
                                                 const std::string& cop )
        {
            if( !value.empty()
                && value[0] == '\''
                && value[ value.length()-1 ] == '\'' )
            {
                if( getIsCaseSensitiveRegex()->operator()( value ) )
                    return ATTRTYPEID_STR;
                return ATTRTYPEID_CIS;
            }
            
            {
                static stringlist_t timePrefixes;
                if( timePrefixes.empty() )
                {
                    timePrefixes.push_back( "atime" );
                    timePrefixes.push_back( "ctime" );
                    timePrefixes.push_back( "mtime" );
                    timePrefixes.push_back( "multiversion-atime" );
                    timePrefixes.push_back( "multiversion-mtime" );
                    timePrefixes.push_back( "ferris-current-time" );
                }
            
                for( stringlist_t::const_iterator si = timePrefixes.begin();
                     si != timePrefixes.end(); ++si )
                {
                    if( eaname == *si )
                        return ATTRTYPEID_TIME;
                    if( starts_with( eaname, *si ) && ends_with( eaname, "-granularity" ))
                        return ATTRTYPEID_TIME;
                }

                if( starts_with( eaname, "emblem:" ) && ends_with( eaname, "-mtime" ) )
                {
                    return ATTRTYPEID_TIME;
                }
            }
            
            {
                
                static stringlist_t sizePrefixes;
                if( sizePrefixes.empty() )
                {
                    sizePrefixes.push_back( "size" );
                }
                for( stringlist_t::const_iterator si = sizePrefixes.begin();
                     si != sizePrefixes.end(); ++si )
                {
                    if( eaname == *si )
                        return ATTRTYPEID_INT;
                }
            }
            
            

            if( cop == "int" )
                return ATTRTYPEID_INT;
            if( cop == "double" || cop == "float" )
                return ATTRTYPEID_DBL;

            if( getIsCaseSensitiveRegex()->operator()( value ) )
                return ATTRTYPEID_STR;
            
            return ATTRTYPEID_CIS;
        }
        
        bool
        MetaEAIndexerInterface::isCaseSensitive( const std::string s )
        {
            return( getIsCaseSensitiveRegex()->operator()( s ) );
        }
        

        docNumSet_t&
        MetaEAIndexerInterface::BuildQuerySQL( fh_context q,
                                               docNumSet_t& output,
                                               fh_eaquery qobj,
                                               std::stringstream& SQLHeader,
                                               std::stringstream& SQLWherePredicates,
                                               std::stringstream& SQLTailer,
                                               stringset_t& lookupTablesUsed,
                                               bool& queryHasTimeRestriction,
                                               string& DocIDColumn,
                                               stringset_t& eanamesUsed,
                                               BuildQuerySQLTermInfo_t& termInfo  )
        {
            return output;
        }

        bool
        MetaEAIndexerInterface::getIndexMethodSupportsIsFileNewerThanIndexedVersion()
        {
            return false;
        }
        
        bool
        MetaEAIndexerInterface::isFileNewerThanIndexedVersion( const fh_context& c )
        {
            return true;
        }
        
        void
        MetaEAIndexerInterface::removeDocumentsMatchingRegexFromIndex( const std::string& s,
                                                                       time_t mustBeOlderThan )
        {
            stringstream ss;
            ss << "This index does not support the removal of documents.\n";
            Throw_NotSupported( tostr(ss), 0 );
        }

        void
        MetaEAIndexerInterface::retireDocumentsFromIndex(  docNumSet_t& docids )
        {
            stringstream ss;
            ss << "This index does not support the removal of documents.\n";
            Throw_NotSupported( tostr(ss), 0 );
        }
        void
        MetaEAIndexerInterface::retireDocumentsFromIndex( stringset_t& urls )
        {
            stringstream ss;
            ss << "This index does not support the removal of documents.\n";
            Throw_NotSupported( tostr(ss), 0 );
        }
        
        

        
        bool
        MetaEAIndexerInterface::supportsRemove()
        {
            return false;
        }

        void
        MetaEAIndexerInterface::purgeDocumentInstancesOlderThan( time_t t )
        {
            stringstream ss;
            ss << "This index does not support the removal of documents.\n";
            Throw_NotSupported( tostr(ss), 0 );
        }
        
        void
        MetaEAIndexerInterface::removeByURL( stringlist_t& sl )
        {
            LG_EAIDX_D << "MetaEAIndexerInterface::removeByURL() sl.sz:" << sl.size() << endl;
            for( stringlist_t::iterator si = sl.begin(); si != sl.end(); ++si )
            {
                string s = Util::EscapeStringAsRegex( *si );
                LG_EAIDX_D << "MetaEAIndexerInterface::removeByURL() s:" << s << endl;
                cerr << "MetaEAIndexerInterface::removeByURL() s:" << s << endl;
                removeDocumentsMatchingRegexFromIndex( s );
            }
            if( !sl.empty() )
                sync();
        }

        void
        MetaEAIndexerInterface::removeByURL( stringset_t& sl )
        {
            for( stringset_t::iterator si = sl.begin(); si != sl.end(); ++si )
            {
                string s = Util::EscapeStringAsRegex( *si );
                removeDocumentsMatchingRegexFromIndex( s );
            }
            if( !sl.empty() )
                sync();
        }
        
        
        
        void
        MetaEAIndexerInterface::queryFoundNonResolvableURLs( stringlist_t& sl )
        {
            LG_EAIDX_D << "MetaEAIndexerInterface::queryFoundNonResolvableURLs() sl.sz:" << sl.size() << endl;
            stringlist_t regexes = getNonResolvableURLsNotToRemoveRegexes();
            if( !regexes.empty() )
            {
                erase_any_matches( toregexi( regexes ), sl );
            }
            removeByURL( sl );
        }
        
        
        void
        MetaEAIndexerInterface::allWritesComplete()
        {
        }
        void
        MetaEAIndexerInterface::setOpenConfigReadOnly( bool v )
        {
            P->m_configShouldBeReadOnly = v;
        }
        
        fh_fwdeaidx
        MetaEAIndexerInterface::tryToCreateForwardEAIndexInterface()
        {
            return 0;
        }

        stringset_t&
        MetaEAIndexerInterface::getValuesForAttribute( stringset_t& ret, const std::string& eaname, AttrType_t att )
        {
            stringstream ss;
            ss << "no support for listing the values for an attribute for this index type!";
            Throw_NotSupported( tostr(ss), 0 );
        }
        
        
        void
        MetaEAIndexerInterface::precacheDocIDs( docNumSet_t& docnums, std::map< docid_t, std::string >& cache )
        {
            LG_EAIDX_D << "precacheDocIDs() s.size:" << docnums.size() << endl;
        }

        void
        MetaEAIndexerInterface::setFilesIndexedCount( int v )
        {
            P->m_filesIndexedCount = v;
        }
        
        void
        MetaEAIndexerInterface::incrFilesIndexedCount()
        {
            P->m_filesIndexedCount++;
        }

        int
        MetaEAIndexerInterface::getFilesIndexedCount()
        {
            return P->m_filesIndexedCount;
        }
        
        
        
        /************************************************************/
        /************************************************************/
        /************************************************************/
        /************************************************************/

        struct FERRISEXP_DLLLOCAL EAIndexerFromLibraryFailed
        {
            string m_gmodule_error;
            string libname;

            EAIndexerFromLibraryFailed( string m_gmodule_error,
                                        string libname )
                :
                m_gmodule_error( m_gmodule_error ),
                libname( libname )
                {
                }
                
            MetaEAIndexerInterface* AlwaysThrow()
                {
                    ostringstream ss;
                    ss  << "Error, unable to open module file: "
                        << m_gmodule_error << endl
                        << " implementation is at:" << libname << endl
                        << endl;
                    LG_PLUGIN_I << tostr(ss) << endl;
                    Throw_GModuleOpenFailed( tostr(ss), 0 );
                }
        };
            
        MetaEAIndexerInterface* CreateEAIndexerFromLibrary(
            const std::string& implnameraw )
        {
            std::string libname = AUTOTOOLS_CONFIG_LIBDIR + "/ferris/plugins/eaindexers/"
                + implnameraw;

            typedef ::Loki::Functor< MetaEAIndexerInterface*,
                ::Loki::NullType > CreateFunc_t;
            MetaEAIndexerInterface* (*CreateFuncPtr)();
                
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
//                     EAIndexerFromLibraryFailed obj( g_module_error (),
//                                                     libname );
//                     ci = cache.insert( make_pair( libname, obj )).second;
//                     return ci->second();
                    
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
        
        /************************************************************/
        /************************************************************/
        /************************************************************/
        /************************************************************/

    };
};

