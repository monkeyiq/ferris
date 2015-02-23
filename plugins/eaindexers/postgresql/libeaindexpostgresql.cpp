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

    $Id: libeaindexpostgresql.cpp,v 1.26 2011/05/03 21:30:21 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <pqxx/connection>
#include <pqxx/tablewriter>
#include <pqxx/transaction>
#include <pqxx/nontransaction>
#include <pqxx/tablereader>
#include <pqxx/tablewriter>

using namespace PGSTD;
using namespace pqxx;

#include "EAIndexerMetaInterface.hh"
#include "ForwardEAIndexInterface.hh"
#include "EAIndexer.hh"
#include "EAQuery.hh"
#include "General.hh"
#include "FactoriesCreationCommon_private.hh"
#include "Ferris/Ferris_private.hh"
#include "Ferris/Medallion.hh"
#include "Ferris/Iterator.hh"
#include "Ferris/FerrisBoost.hh"

#include "EAIndexerSQLCommon_private.hh"

// #undef LG_EAIDX_D
// #define LG_EAIDX_D cerr

// #define DEBUG_ADDING_TO_INDEX 1

using namespace std;

namespace Ferris
{
    bool regexHasNoSpecialChars( const std::string&s )
    {
        static boost::regex rex("^[a-zA-Z0-9 '\\\"]*$");
        boost::smatch matches;
        if( boost::regex_match( s, matches, rex ))
        {
            return true;
        }
        return false;
    }


    
    template < class T >
    string tostr( T x )
    {
        ostringstream oss;
        oss << x;
        return tostr( oss );
    }
    
    namespace EAIndex 
    {
        static const char* CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K
        = CFG_EAIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K;

        static const char* CFG_POSTGRESQLIDX_DOCMAP_COLUMNS_TO_FULLTEXT_INDEX_K
        = "cfg-idx-docmap-columns-to-fulltext-index";
        
        static const char* CFG_IDX_USER_K   = "cfg-idx-user";
        static const char* CFG_IDX_HOST_K   = "cfg-idx-host";
        static const char* CFG_IDX_PORT_K   = "cfg-idx-port";
        static const char* CFG_IDX_DBNAME_K = "cfg-idx-dbname";
        static const char* CFG_IDX_TEMPLATE_DBNAME_K = "cfg-idx-template-dbname";
        static const char* CFG_IDX_USER_DEF   = "";
        static const char* CFG_IDX_HOST_DEF   = "localhost";
        static const char* CFG_IDX_PORT_DEF   = "";
        static const char* CFG_IDX_DBNAME_DEF = "ferriseaindex";
        static const char* CFG_IDX_TEMPLATE_DBNAME_DEF = "";
        static const char* CFG_IDX_USE_GIST_K = "cfg-idx-use-gist";
        static const char* CFG_IDX_USE_GIST_DEF = "0";

        static const char* CFG_IDX_MULTI_INSTANCE_K   = "cfg-idx-multi-instance";
        static const char* CFG_IDX_MULTI_INSTANCE_DEF = "0";

        static const char* CFG_IDX_MULTIVERSION_K = "cfg-idx-multiversion";
        static const char* CFG_IDX_MULTIVERSION_DEF = "0";
        

        typedef list< vector< string > > BulkRows_t;
        
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/

        class EAIndexerPostgresql;
        FERRIS_SMARTPTR( EAIndexerPostgresql, fh_EAIndexerPostgresql );

        class FERRISEXP_API ForwardEAIndexInterfacePostgresql
            :
            public ForwardEAIndexInterface
        {
            fh_EAIndexerPostgresql m_idx;
            typedef FERRIS_STD_HASH_MAP< std::string, std::string > m_urlcache_t;
            FERRIS_NIREF( m_urlcache_t, m_urlcache_ref );
            typedef FERRIS_STD_HASH_MAP< std::string, m_urlcache_ref > m_cache_t;
            m_cache_t m_cache;
            
            void precache();
            
        public:
            ForwardEAIndexInterfacePostgresql( fh_EAIndexerPostgresql idx );

            virtual std::string getStrAttr( Context* c,
                                            const std::string& earl,
                                            const std::string& rdn,
                                            const std::string& def,
                                            bool throw_for_errors = true );
        };
        
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/

        
        static ctxlist_t& getNoExteaTerms()
        {
            static ctxlist_t ret;
            return ret;
        }
        
        class FERRISEXP_DLLLOCAL EAIndexerPostgresql
            :
            public MetaEAIndexerInterface
        {
//            work* m_trans;

            stringmap_t m_ExtraColumnsToInlineInDocmap;
            int         m_filesIndexedSinceAnalyseCount;
            connection* m_connection;
            connection* m_connection_forDBFunctions; //< to call escape() etc any time.
            stringset_t m_DocmapColumnsToFulltextIndex;

            bool        m_useGIST;
            bool        m_multiVersion;
            int         m_queryLimit;
            
            friend class ForwardEAIndexInterfacePostgresql;

            int getLimit()
                {
                    return m_queryLimit;
                }

            string quoteStr( work& trans, const std::string& s )
            {
                stringstream ss;
                ss << "'" << trans.esc( s ) << "'";
                return tostr(ss);

                
//                ss << "'" << sqlesc( s.c_str() ) << "'";
//                ss << "'" << s << "'";

                // replace_all here makes keys which don't match up again when reread.
                //ss << "'" << Util::replace_all( sqlesc( s ), "\\", "\\\\" ) << "'";
                return tostr(ss);


//               std::wstring ret;
//               wchar_t singq = '\'';
//               wstringstream ss;
//                ss << Util::toStreamChar( singq )
//                  << Util::utf8_to_wstring( ret, s )
//                  << Util::toStreamChar( singq );
//               string r;
//               r = Util::wstring_to_utf8( r, ss.str() );
//               return r;
            }
            
            string quoteStr( const std::string& s )
            {
                work trans( *m_connection_forDBFunctions, "as string" );
                string ret = quoteStr( trans, s );
                return ret;
            }

            
            //////////////////////////////////////////////
            //////////////////////////////////////////////
            //////////////////////////////////////////////
            
            // maps attribute name to the number of bits to right shift (>>)
            // for that attribute
            typedef map< string, int > m_bitfColumns_t;
            m_bitfColumns_t m_bitfColumns;
            int getBitFLength()
                {
                    return m_bitfColumns.size() + 5;
                }

            string getBitFunctionName( const std::string& attributeName )
                {
//                     if( starts_with( attributeName, "emblem:has-" ) )
//                     {
//                         fh_etagere et = Factory::getEtagere();
//                         string ename = eaname.substr( strlen("emblem:has-"));
//                         fh_emblem em = et->getEmblemByName( ename );
                        
//                         static string prefix = "ferris_bitfeid_";
//                         string ea = EANameToSQLColumnName( em->getID() );
//                         return prefix + ea;
//                     }
//                     if( starts_with( attributeName, "emblem:id-" ) )
//                     {
//                         fh_etagere et = Factory::getEtagere();
//                         string idstr = eaname.substr( strlen("emblem:id-"));
//                         emblemID_t eid = toType<emblemID_t>( idstr );
//                         fh_emblem em = et->getEmblemByID( eid );
                        
//                         static string prefix = "ferris_bitfeid_";
//                         string ea = EANameToSQLColumnName( em->getID() );
//                         return prefix + ea;
//                     }
                    
                    static string prefix = "ferris_bitf_";
                    string ea = EANameToSQLColumnName( attributeName );
                    return prefix + ea;
                }
            
            //////////////////////////////////////////////
            //////////////////////////////////////////////
            //////////////////////////////////////////////
            
            
            string makeConnectionString( bool includeDBName = true );

            // pair< attrname, attrtype >  maps to attrid
            typedef map< pair< string, int >, int > m_attrMapCache_t;
            m_attrMapCache_t m_attrMapCache;
            m_attrMapCache_t& getAttrMapCache( work& trans );

            typedef map< string, int > m_genericlookupCache_t;
            m_genericlookupCache_t& getLookupCache( work& trans,
                                                    const string& tablename,
                                                    m_genericlookupCache_t& lc,
                                                    int& lcSize );
            
            // maps attrvalue to vid
            typedef map< string, int > m_strlookupCache_t;
            m_strlookupCache_t m_strlookupCache;
            int m_strlookupCacheSize;
            m_strlookupCache_t& getStrLookupCache( work& trans );

            typedef map< string, int > m_timelookupCache_t;
            m_timelookupCache_t m_timelookupCache;
            int m_timelookupCacheSize;
            m_timelookupCache_t& getTimeLookupCache( work& trans );

            typedef map< string, int > m_intlookupCache_t;
            m_intlookupCache_t m_intlookupCache;
            int m_intlookupCacheSize;
            m_intlookupCache_t& getIntLookupCache( work& trans );
            
            typedef map< string, int > m_doublelookupCache_t;
            m_doublelookupCache_t m_doublelookupCache;
            int m_doublelookupCacheSize;
            m_doublelookupCache_t& getDoubleLookupCache( work& trans );
            
            
            template< class Value_t, class Collection_t, class LookupCache_t >
            void UpdateLookupTable( work& trans,
                                    const string& lookupTableName,
                                    Collection_t& LookupValues,
                                    LookupCache_t& LookupCache,
                                    int& LookupCacheSize, 
                                    int docid,
                                    BulkRows_t& docattrsBulkRows
                )
                {
                    typedef typename Collection_t::const_iterator LookupValuesITER;

                    // PURE DEBUG
                    LG_EAIDX_D << "UpdateLookupTable(begin dump)" << endl;
                    for( LookupValuesITER iter = LookupValues.begin(); iter != LookupValues.end(); ++iter )
                    {
                        IndexableValue* ivp = iter->first;
                        string v = asString( *ivp, ivp->getAttrTypeID(), false );
                        LG_EAIDX_D << "d:" << docid << " ....... v:" << v
                                   << " esc(v):" << trans.esc(v)
                                   << " raw(v):" << ivp->rawValueString()
                                   << endl;
                    }
                    LG_EAIDX_D << "UpdateLookupTable(end dump)" << endl;
                    
                    for( LookupValuesITER iter = LookupValues.begin(); iter != LookupValues.end(); ++iter )
                    {
                        IndexableValue* ivp = iter->first;
//                        string v = ivp->rawValueString();
                        string v = asString( *ivp, ivp->getAttrTypeID(), false );

                        LG_EAIDX_D << "d:" << docid << " ....... v:" << v
                                   << " esc(v):" << trans.esc(v)
                                   << endl;
                        
                        switch( ivp->getAttrTypeID() )
                        {
                        case ATTRTYPEID_INT:
                        case ATTRTYPEID_DBL:
                            if( v.empty() )
                                v = "0";
                            break;
                        case ATTRTYPEID_CIS:
                        case ATTRTYPEID_STR:
                        {
//                             v = sqlesc( v );
                            
//                             if( v.find( "\'\'" ) != string::npos )
//                             {
//                                 v = Util::replace_all( v, "\'\'", "" );
//                             }

                            
                            fh_stringstream ss;
                            if( v.find( '\0' ) != string::npos )
                            {
                                v = stripNullCharacters( v );
                                
//                                 string::iterator e = v.end();
//                                 string::iterator i = v.begin();
//                                 for( ; i!=e; ++i )
//                                     if( *i != '\0' )
//                                         ss << *i;
//                                 v = tostr(ss);
                            }
// //                             else
// //                                 ss << v;
// //                             v = tostr(ss);
                        }
                        }
                        
                        
                        long aid = iter->second;
                        long vid = -1;
                        LG_EAIDX_D << "d:" << docid << " ....... v:" << v << " aid:" << aid << endl;
                        LG_EAIDX_D << "Checking x lookup value:" << v << endl;

                        typedef typename LookupCache_t::iterator LookupCacheITER;
                        LookupCacheITER citer = LookupCache.find( v );
                        if( citer != LookupCache.end() )
                        {
                            vid = citer->second;
                        }
                        else
                        {
//                            string sqlv = asString( *ivp, ivp->getAttrTypeID(), true );
                            string sqlv = v;
                            fh_stringstream ss;
                            LG_EAIDX_D << "v:" << v << endl;
                            LG_EAIDX_D << "sqlv1:" << sqlv << endl;

                            switch( ivp->getAttrTypeID() )
                            {
                            case ATTRTYPEID_TIME:
                            case ATTRTYPEID_CIS:
                            case ATTRTYPEID_STR:
                                sqlv = quoteStr( trans, sqlv );
                            }

                            LG_EAIDX_D << "eaname:" << ivp->rawEANameString() << endl;
                            LG_EAIDX_D << "sqlv2:" << sqlv << endl;
                            ++LookupCacheSize;
                            vid = LookupCacheSize;
                            
                            LookupCache.insert( make_pair( v, vid ) );
                            ss << "INSERT INTO " << lookupTableName << " (vid,attrvalue) VALUES ("
                               << vid << "," << sqlv << " );";
                            LG_EAIDX_D << "SQL:" << tostr(ss) << endl;
                            Execute( trans, tostr(ss) );
                        }

                        LG_EAIDX_D << "BULK DOCATTRS... aid:" << aid << " vid:" << vid << " docid:" << docid << endl;
                        vector<string> tuple;
                        tuple.push_back( tostr(aid) );
                        tuple.push_back( tostr(vid) );
                        tuple.push_back( tostr(docid) );
                        docattrsBulkRows.push_back( tuple );
                    }
                }

            void setupBitfColumnsFromString( const std::string& s, bool isNSV = false );

            bool useGIST();
            void create_bitf_to_intvec_function_and_bitf_rdtree_index( work& trans );
            void CreateIndex_addEmblemToBitfColumns( fh_emblem em );
            void CreateIndex_saveBitfColumns();
            void CreateIndex_createBitf4GLFunction( work& trans, m_bitfColumns_t::const_iterator iter );
            void CreateIndex_remakeView_currentVersions( work& trans );

            
        protected:

            void retire_old_docids_from_docmap();
            
            static const char** get_defaultindexing_create_commands();
            static const char** get_defaultindexing_drop_commands();
            
            virtual void Setup();
            virtual void CreateIndex( fh_context c,
                                      fh_context md );
            virtual void CommonConstruction();

            virtual std::string asString( IndexableValue& v, AttrType_t att, bool quote );
            virtual std::string asString( IndexableValue& v, AttrType_t att )
                {
                    return asString( v, att, true );
                }
            virtual std::string asString( IndexableValue& v )
                {
                    return asString( v, v.getAttrTypeID() );
                }
            
            
            string  getTableName( AttrType_t att );

            
        public:
            EAIndexerPostgresql();
            virtual ~EAIndexerPostgresql();

            virtual void sync();
            virtual void compact( fh_ostream oss, bool verbose = false );
            virtual void prepareForWrites( int f );
            virtual void allWritesComplete();

            docid_t obtainURLID( work& trans, fh_context c, const std::string& url );

            virtual void addToIndex( fh_context c,
                                     fh_docindexer di );
            string AddSQLOp_ferrisopcode_to_sqlopcode( const std::string& s, IndexableValue& iv);
            void AddSQLOp( fh_stringstream& sqlss,
                           const std::string& eaname,
                           const std::string& opcode,
                           IndexableValue& iv,
                           AttrType_t att,
                           stringset_t& lookupTablesUsed,
                           const std::string termMergeSQLOpCode = "",
                           const ctxlist_t& extraTerms = getNoExteaTerms() );
            AttrType_t
            SQLColumnTypeToAttrType( const std::string& coltype,
                                     IndexableValue& iv );
            void AddSQLOpHeur( fh_stringstream& sqlss,
                               const std::string& eaname,
                               const std::string& opcode,
                               IndexableValue& iv,
                               stringset_t& lookupTablesUsed );
            virtual docNumSet_t& ExecuteQuery( fh_context q,
                                               docNumSet_t& output,
                                               fh_eaquery qobj,
                                               int limit = 0 );

            string BuildQuery_getToken( fh_context q );
            pair< fh_context, fh_context > BuildQuery_getLeftAndRightContexts( fh_context q );
            string  BuildQuery_getEAName( fh_context q );

            docNumSet_t& BuildQuery_LogicalCombine(
                const std::string& termMergeSQLOpCode,
                fh_context q,
                docNumSet_t& output,
                fh_eaquery qobj,
                fh_stringstream& sqlss,
                stringset_t& lookupTablesUsed,
                bool& queryHasTimeRestriction,
                stringset_t& eanamesUsed,
                MetaEAIndexerInterface::BuildQuerySQLTermInfo_t& termInfo );
            
            virtual docNumSet_t& BuildQuery(
                fh_context q,
                docNumSet_t& output,
                fh_eaquery qobj,
                fh_stringstream& sqlss,
                stringset_t& lookupTablesUsed,
                bool& queryHasTimeRestriction,
                stringset_t& eanamesUsed,
                MetaEAIndexerInterface::BuildQuerySQLTermInfo_t& termInfo );
            virtual docNumSet_t& BuildQuerySQL(
                fh_context q,
                docNumSet_t& output,
                fh_eaquery qobj,
                std::stringstream& SQLHeader,
                std::stringstream& SQLWherePredicates,
                std::stringstream& SQLTailer,
                stringset_t& lookupTablesUsed,
                bool& queryHasTimeRestriction,
                std::string& DocIDColumn,
                stringset_t& eanamesUsed,
                MetaEAIndexerInterface::BuildQuerySQLTermInfo_t& termInfo );
            
            virtual void precacheDocIDs( docNumSet_t& s, std::map< docid_t, std::string >& cache );
            virtual std::string resolveDocumentID( docid_t );

            long        ensureValueMap( const IndexableValue& iv );

            typedef FERRIS_STD_HASH_MAP< string, time_t > m_isFileNewerThanIndexedVersionCache_t;
            m_isFileNewerThanIndexedVersionCache_t m_isFileNewerThanIndexedVersionCache;
            m_isFileNewerThanIndexedVersionCache_t &getIsFileNewerThanIndexedVersionCache();
            virtual bool getIndexMethodSupportsIsFileNewerThanIndexedVersion();
            virtual bool isFileNewerThanIndexedVersion( const fh_context& c );
            void setupIsFileNewerThanIndexedVersionCache();
            virtual void purgeDocumentInstancesOlderThan( time_t t );
            typedef FERRIS_STD_HASH_MAP< string, time_t > m_isFileNewerThanIndexedVersionSkippedDocuments_t;
            m_isFileNewerThanIndexedVersionSkippedDocuments_t m_isFileNewerThanIndexedVersionSkippedDocuments;

            void retireDocumentsFromIndex( docNumSet_t& docids );
            void retireDocumentsFromIndex( stringset_t& urls );
            
            virtual void removeDocumentsMatchingRegexFromIndex( const std::string& s,
                                                                time_t mustBeOlderThan = 0 );
            virtual bool supportsRemove()
                {
                    return true;
                }

            fh_fwdeaidx tryToCreateForwardEAIndexInterface();
            
            
            
            /************************************************************/
            /************************************************************/
            /************************************************************/

            inline result& ExecuteRes( result& ret,
                                       work& work,
                                       const std::string& sql )
                {
                    LG_EAIDX_D << "SQL:" << sql << endl;
                    ret = work.exec( sql.c_str() );
                    return ret;
                }
            
            void Execute( work& work, const std::string& sql )
                {
                    LG_EAIDX_D << "SQL:" << sql << endl;
                    work.exec( sql.c_str() );
                }
            
            void Execute( work& work, const char** commands )
                {
                    for( const char** cmd = commands; *cmd; ++cmd )
                    {
                        LG_EAIDX_D << "SQL:" << *cmd << endl;
                        work.exec( *cmd );
                    }
                }

            void Execute( work& work, const stringlist_t& sl )
                {
                    for( stringlist_t::const_iterator si=sl.begin();si!=sl.end();++si)
                    {
                        LG_EAIDX_D << "SQL:" << *si << endl;
                        work.exec( si->c_str() );
                    }
                }

            bool canColumnHavePotentiallyHugeValue( const std::string& colname );
            void createIndexesForDocmapColumn( work& trans,
                                               const std::string& colname,
                                               const std::string& coltype,
                                               bool fulltextOnly = false );
            
        };

        /************************************************************/
        /************************************************************/
        /************************************************************/

        EAIndexerPostgresql::EAIndexerPostgresql()
            : m_filesIndexedSinceAnalyseCount( 0 )
            , m_connection( 0 )
            , m_useGIST( false )
            , m_multiVersion( false )
            , m_queryLimit( 0 )
        {
        }

        

        EAIndexerPostgresql::~EAIndexerPostgresql()
        {
            if( m_connection )
            {
                delete m_connection;
            }
            if( m_connection_forDBFunctions )
            {
                delete m_connection_forDBFunctions;
            }
        }

        string
        EAIndexerPostgresql::makeConnectionString( bool includeDBName )
        {
            string user   = this->getConfig( CFG_IDX_USER_K,   CFG_IDX_USER_DEF, true );
            string host   = this->getConfig( CFG_IDX_HOST_K,   CFG_IDX_HOST_DEF, true );
            string port   = this->getConfig( CFG_IDX_PORT_K,   CFG_IDX_PORT_DEF, true );
            string dbname = this->getConfig( CFG_IDX_DBNAME_K, CFG_IDX_DBNAME_DEF, true );
            
            fh_stringstream ss;
            if( !user.empty() )
                ss << " user=" << user;
            if( !host.empty() )
                ss << " host=" << host;
            if( !port.empty() )
                ss << " port=" << port;
            if( includeDBName && !dbname.empty() )
                ss << " dbname=" << dbname;

            return tostr(ss);
        }
        
        
        void
        EAIndexerPostgresql::Setup()
        {
            setOpenConfigReadOnly( true );
            string constring = makeConnectionString();
            m_connection = new connection( constring );
            m_connection_forDBFunctions = new connection( constring );
//            m_connection->set_client_encoding( "UNICODE" );
            
            LG_EAIDX_D << "EAIndexerPostgresql::Setup() con:" << constring << endl;
        }

        const char**
        EAIndexerPostgresql::get_defaultindexing_create_commands()
        {
            static const char* defaultindexing_commands[] = {

                "create index attrnameidx   on attrmap      ( attrtype, attrname )",
//                "create index attrididx     on docattrs     ( attrid )",
                "create index vididx        on docattrs     ( vid )",
                "create index avididx       on docattrs     ( attrid,vid )",
                "create index dadocididx    on docattrs     ( docid )",
                "create index strlookupidx  on strlookup    ( attrvalue,vid )",
                "create index strlookupidxcis  on strlookup    ( lower(attrvalue),vid )",
                "create index strlookupidxcisv on strlookup    ( lower(attrvalue) )",
                "create index strlookupidxv    on strlookup    ( vid )",
                "create index intvalueidx   on intlookup    ( attrvalue,vid )",
                "create index doublelkpidx  on doublelookup ( attrvalue,vid )",
                "create index timelkpidx    on timelookup   ( attrvalue,vid )",
//                 "create index strlookupidx2  on strlookup    ( attrvalue )",
//                 "create index intvalueidx2   on intlookup    ( attrvalue )",
//                 "create index doublelkpidx2  on doublelookup ( attrvalue )",
//                 "create index timelkpidx2    on timelookup   ( attrvalue )",
                
///                "create index docmapididx   on docmap       ( docid )",
                0
            };

            return defaultindexing_commands;
        }

        const char**
        EAIndexerPostgresql::get_defaultindexing_drop_commands()
        {
            static const char* defaultindexing_commands[] = {

                "drop index attrnameidx",
                "drop index attrididx",
                "drop index vididx   ",
                "drop index avididx  ",
                "drop index strlookupidx    ",
                "drop index strlookupidxcis ",
                "drop index strlookupidxcisv",
                "drop index intvalueidx   ",
                "drop index doublelkpidx  ",
                "drop index timelkpidx    ",
                0
            };

            return defaultindexing_commands;
        }


        void
        EAIndexerPostgresql::create_bitf_to_intvec_function_and_bitf_rdtree_index( work& trans )
        {
            int bitfSize = getBitFLength();
            stringstream ss;
            ss << "CREATE OR REPLACE FUNCTION docmap_bitf_to_intvec( bit varying ) RETURNS int[] IMMUTABLE STRICT AS ' "
               << "DECLARE "
               << "	bv alias for $1; "
               << "	sz int := " << bitfSize << "-1; "
               << "	ret int[]; "
               << "	tmp int; "
               << "BEGIN "
               << "	tmp := 0; "
               << "	FOR i IN 1..sz LOOP "
               << "		IF (bv::bit(" << bitfSize << ") & (B''1''::bit(" << bitfSize << ") >> i )) = (B''1''::bit(" << bitfSize << ") >> i) "
               << "		THEN "
               << "			tmp := i; "
               << "			EXIT; "
               << "		END IF; "
               << "	END LOOP; "
               << "	IF tmp = 0  "
               << "	THEN "
               << "		RETURN ret; "
               << "	END IF; "
               << "	ret := ARRAY[tmp]; "
               << "	tmp := tmp + 1; "
               << "	FOR i IN tmp..sz LOOP "
               << "		IF (bv::bit(" << bitfSize << ") & (B''1''::bit(" << bitfSize << ") >> i )) = (B''1''::bit(" << bitfSize << ") >> i) "
               << "		THEN "
               << "			ret = array_append(ret,i);  "
               << "		END IF; "
               << "	END LOOP; "
               << "	RETURN ret; "
               << "END; "
               << "' LANGUAGE plpgsql;             ";

            LG_EAIDX_D << "BITF TO INT[] FUNC:" << ss.str() << endl;
            Execute( trans, ss.str() );
            if( useGIST() )
            {
                Execute( trans, "create index dmbitfidxrd on docmap using gist ( docmap_bitf_to_intvec(bitf) );" );
            }
        }
        

        void
        EAIndexerPostgresql::CreateIndex_addEmblemToBitfColumns( fh_emblem em )
        {
            if( em->isTransitiveChildOfEAOrderingRootEmblem() )
                return;
            
            string eaname = EANAME_SL_EMBLEM_PREKEY + em->getName();
            int nextID = m_bitfColumns.size() + 1;

            LG_EAIDX_D << "Checking if emblem ea exists, eaname:" << eaname
                       << " !exists:" << (m_bitfColumns.end() == m_bitfColumns.find( eaname ))
                       << endl;
            
            if( m_bitfColumns.end() == m_bitfColumns.find( eaname ) )
            {
                m_bitfColumns[ eaname ] = nextID;
                ++nextID;
            }

            eaname = EANAME_SL_EMBLEM_ID_PREKEY + tostr(em->getID());
            if( m_bitfColumns.end() == m_bitfColumns.find( eaname ) )
            {
                m_bitfColumns[ eaname ] = nextID;
                ++nextID;
            }

//                    m_ExtraColumnsToInlineInDocmap[ "ferris-should-reindex-if-newer" ] = "timestamp";
    
//                     //
//                     // ':' is used as a record seperator :(
//                     //
//                     eaname = EANAME_SL_EMBLEM_FUZZY_PREKEY + em->getName();
//                     stringmap_t::const_iterator eci
//                         = m_ExtraColumnsToInlineInDocmap.find( eaname );
//                     if( eci == m_ExtraColumnsToInlineInDocmap.end() )
//                     {
//                         m_ExtraColumnsToInlineInDocmap.insert(
//                             make_pair( eaname, "float" ));
//                     }
                    
//                     eaname = EANAME_SL_EMBLEM_TIME_PREKEY + em->getName() + "-mtime";
//                     eci = m_ExtraColumnsToInlineInDocmap.find( eaname );
//                     if( eci == m_ExtraColumnsToInlineInDocmap.end() )
//                     {
//                         m_ExtraColumnsToInlineInDocmap.insert(
//                             make_pair( eaname, "timestamp" ));
//                     }
        }
        
        void
        EAIndexerPostgresql::CreateIndex_saveBitfColumns()
        {
            LG_EAIDX_D << "CreateIndex_saveBitfColumns() sz:" << m_bitfColumns.size() << endl;
            
            typedef map< int, string > rev_t;
            rev_t rev;
            for( m_bitfColumns_t::const_iterator bi = m_bitfColumns.begin();
                 bi != m_bitfColumns.end(); ++bi )
            {
                rev.insert( make_pair( bi->second, bi->first ) );
            }

            stringlist_t tl;
            copy( map_range_iterator( rev.rbegin() ),
                  map_range_iterator( rev.rend() ),
                  front_inserter( tl ) );
            string bitfColumnsRAW = Util::createSeperatedList( tl );
//             LG_EAIDX_D << "CreateIndex_saveBitfColumns() bitfColumnsRAW.len:" << bitfColumnsRAW.length() << endl
//                        << " data:" << bitfColumnsRAW << endl;
            
            setConfig( CFG_POSTGRESQLIDX_COLUMNS_TO_INLINE_IN_BITF_K,
                       bitfColumnsRAW );
            
            {
                setConfig( CFG_POSTGRESQLIDX_COLUMNS_TO_INLINE_IN_BITF_NSV_K,
                           Util::createNullSeperatedList( tl ) );
            }
            
        }


        void
        EAIndexerPostgresql::CreateIndex_createBitf4GLFunction( work& trans, m_bitfColumns_t::const_iterator iter )
        {
            int bitfSize = getBitFLength();
            
            string attributeName = iter->first;
            int    ShiftCount    = iter->second;
            
            stringstream ss;
            ss << "CREATE OR REPLACE FUNCTION " << getBitFunctionName( attributeName )
               << "( bit varying ) RETURNS boolean IMMUTABLE STRICT AS '" << nl
               << "DECLARE" << nl
               << "    bv ALIAS FOR $1;" << nl
               << "BEGIN" << nl
               << "RETURN ( bv::bit(" << bitfSize << ") "
               << "     & (B''1''::bit(" << bitfSize << ") >> " << ShiftCount << ")"
               << "     = (B''1''::bit(" << bitfSize << ") >> " << ShiftCount << "));"
               << "END;" << nl
               << "' LANGUAGE plpgsql;";
            LG_EAIDX_D << "BITF FUNC:" << ss.str() << endl;
            Execute( trans, ss.str() );
        }

        void
        EAIndexerPostgresql::CreateIndex_remakeView_currentVersions( work& trans )
        {
            Execute( trans, "drop view if exists currentVersions;" );
            stringstream ss;
            ss << "create or replace view currentVersions as " << nl
               << "SELECT d.*" << nl
               << "FROM docmap d," << nl
               << "  ( select max(docidtime) as ddtime, urlid" << nl
               << "    from docmap" << nl
               << "    group by urlid" << nl
               << "   ) dd" << nl
               << "WHERE d.urlid=dd.urlid" << nl
               << "AND d.docidtime=dd.ddtime;" << nl;
            Execute( trans, ss.str() );
        }
    
        
        

        void
        EAIndexerPostgresql::CreateIndex( fh_context c, fh_context md )
        {
            string user   = getStrSubCtx( md, "user", CFG_IDX_USER_DEF );
            string host   = getStrSubCtx( md, "host", CFG_IDX_HOST_DEF );
            string port   = getStrSubCtx( md, "port", CFG_IDX_PORT_DEF );
            string dbname = getStrSubCtx( md, "dbname", CFG_IDX_DBNAME_DEF );
            string templatedbname = getStrSubCtx( md, "template-dbname", CFG_IDX_TEMPLATE_DBNAME_DEF );

            m_useGIST = isTrue( getStrSubCtx( md, "use-gist", CFG_IDX_USE_GIST_DEF ) );
            m_useGIST = m_useGIST && !templatedbname.empty();
            m_multiVersion = isTrue( getStrSubCtx( md, "multiversion", CFG_IDX_MULTIVERSION_DEF ));
            
            setConfig( CFG_IDX_USER_K, user );
            setConfig( CFG_IDX_HOST_K, host );
            setConfig( CFG_IDX_PORT_K, port );
            setConfig( CFG_IDX_DBNAME_K, dbname );
            setConfig( CFG_IDX_TEMPLATE_DBNAME_K, templatedbname );
            setConfig( CFG_IDX_USE_GIST_K, tostr(m_useGIST) );
            setConfig( CFG_IDX_MULTIVERSION_K, tostr(m_multiVersion) );
            
            
            
            Util::ParseKeyValueString(
                m_ExtraColumnsToInlineInDocmap,
                getStrSubCtx( md, "extra-columns-to-inline-in-docmap",
                              CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_DEFAULT )
                );
            Util::ParseKeyValueString(
                m_ExtraColumnsToInlineInDocmap,
                getStrSubCtx( md, "extra-columns-to-inline-in-docmap-append",
                              CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_DEFAULT )
                );

            Util::parseCommaSeperatedList( getStrSubCtx( md, "docmap-columns-to-fulltext-index",
                                                         CFG_POSTGRESQLIDX_DOCMAP_COLUMNS_TO_FULLTEXT_INDEX_DEFAULT ),
                                           m_DocmapColumnsToFulltextIndex );
            Util::parseCommaSeperatedList( getStrSubCtx( md, "docmap-columns-to-fulltext-index-append",
                                                         CFG_POSTGRESQLIDX_DOCMAP_COLUMNS_TO_FULLTEXT_INDEX_DEFAULT ),
                                           m_DocmapColumnsToFulltextIndex );
            for( stringset_t::iterator si = m_DocmapColumnsToFulltextIndex.begin();
                 si != m_DocmapColumnsToFulltextIndex.end(); ++si )
            {
                m_DocmapColumnsToFulltextIndex.insert( EANameToSQLColumnName(*si));
            }
            
            
            
            LG_EAIDX_D << "creaing index...A" << endl;
            LG_EAIDX_D << "Extra columns for docmap1:"
                       << Util::CreateKeyValueString( m_ExtraColumnsToInlineInDocmap, "=", ":" )
                       << endl;
            LG_EAIDX_D << "creaing index...B" << endl;
            
            
            {
                LG_EAIDX_D << "creaing index...0" << endl;
                string bitfColumnsRAW = getStrSubCtx(
                    md,
                    CFG_POSTGRESQLIDX_COLUMNS_TO_INLINE_IN_BITF_CREATE_K, 
                    CFG_POSTGRESQLIDX_COLUMNS_TO_INLINE_IN_BITF_DEFAULT );
                setupBitfColumnsFromString( bitfColumnsRAW );

                string bitfColumnsRAWAppend = getStrSubCtx( md, "columns-to-bitf-append", "" );
                if( !bitfColumnsRAWAppend.empty() )
                {
                    setupBitfColumnsFromString( bitfColumnsRAWAppend );
                }
                
                LG_EAIDX_D << "creaing index...01" << endl;
                int nextID = m_bitfColumns.size() + 1;
                fh_etagere et = Ferris::Factory::getEtagere();
                emblems_t  el = et->getAllEmblems();
                LG_EAIDX_D << "creaing index...1" << endl;
                for( emblems_t::const_iterator ei = el.begin(); ei!=el.end(); ++ei )
                {
                    LG_EAIDX_D << "inlining emblem id:" << (*ei)->getID() << " name:" << (*ei)->getName() << endl;
                    CreateIndex_addEmblemToBitfColumns( *ei );
                }

                
                m_ExtraColumnsToInlineInDocmap[ "mtime" ] = "timestamp";
                m_ExtraColumnsToInlineInDocmap[ "ctime" ] = "timestamp";

                CreateIndex_saveBitfColumns();
            }

            LG_EAIDX_D << "creaing index...2" << endl;
            {
                string t = Util::CreateKeyValueString( m_ExtraColumnsToInlineInDocmap, "=", ":" );
                m_ExtraColumnsToInlineInDocmap = Util::ParseKeyValueString( t );
            }
            
            setConfig( CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K,
                       Util::CreateKeyValueString( m_ExtraColumnsToInlineInDocmap, "=", ":" )
                );
            LG_EAIDX_D << "Extra columns for docmap:"
                 << Util::CreateKeyValueString( m_ExtraColumnsToInlineInDocmap, "=", ":" )
                 << endl;

            setConfig( CFG_POSTGRESQLIDX_DOCMAP_COLUMNS_TO_FULLTEXT_INDEX_K,
                       Util::createCommaSeperatedList( m_DocmapColumnsToFulltextIndex )
                );
            
            

//             string DontStoreZeroIntegerAttributes = getStrSubCtx(
//                 md, "dont-store-zero-integer-attributes", "1" );
//             string DontStoreEmptyStringAttributes = getStrSubCtx(
//                 md, "dont-store-empty-string-attributes", "1" );
//             setDontStoreZeroIntegerAttributes( isTrue( DontStoreZeroIntegerAttributes ) );
//             setDontStoreEmptyStringAttributes( isTrue( DontStoreEmptyStringAttributes ) );

            LG_EAIDX_D << "Create database:" << getStrSubCtx( md, "db-exists", "0" ) << endl;
            if( isFalse( getStrSubCtx( md, "db-exists", "0" )))
            {
                stringstream conss;
                conss << makeConnectionString( false );
                conss << " dbname=template1";
                connection dbmaker( tostr(conss) );

                stringstream ss;
                nontransaction trans( dbmaker, "create database" );
                ss << "create database " << dbname;
//                ss << " WITH ENCODING 'UNICODE'";
                if( !templatedbname.empty() )
                {
                    ss << " template " << templatedbname;
                }
                LG_EAIDX_D << "Create db SQL:" << ss.str() << endl;
                trans.exec( ss.str() );
                trans.commit();
            }
            
            Setup();

            try
            {
                work trans( *m_connection, "setup database" );
                Execute( trans,
                         "CREATE TABLE urlmap ("
                         "   URL varchar(" SQL_URL_LENGTH_STR ") NOT NULL,"
                         "   urlid serial,"
                         "   primary key( urlid )"
                         "  ) without oids " );
                Execute( trans, "create index umurlidx    on urlmap ( url )" );
                Execute( trans, "create index umurlidxcis on urlmap ( lower(url) )" );
//                Execute( trans, "create index umurlidxid  on urlmap ( urlid )" );

                // For looking for "bar" anywhere inside the url.
                static const char* fourGL[] = {
                    " "
                    "CREATE OR REPLACE FUNCTION fnshiftstring( varchar ) RETURNS varchar IMMUTABLE AS $fnshiftstringstamp$ \n"
                    "DECLARE  \n"
                    " earlin alias for $1; \n"
                    " a text[]; \n"
                    " exploded text[]; \n"
                    " c record; \n"
                    " s varchar; \n"
                    "BEGIN \n"
                    "  a := regexp_split_to_array( earlin, E'[/[\\]()\\.\\s]+'); \n"
                    "  for c in select * from regexp_split_to_table( earlin, E'[/[\\]()\\.\\s]+') as f LOOP \n"
                    "     s := c.f::varchar; \n"
                    "     exploded := exploded || ARRAY[s::text]; \n"
                    "     FOR i IN 1..10 LOOP \n"
                    "       s := substr( s, 2 ); \n"
                    "       IF char_length( s ) < 3 THEN \n"
                    "          i := 100; \n"
                    "       ELSE \n"
                    "          exploded := exploded || ARRAY[s::text]; \n"
                    "       END IF; \n"
                    "     END LOOP; \n"
                    "  END LOOP; \n"
                    "  return exploded::varchar; \n"
                    "END; \n"
                    "$fnshiftstringstamp$ LANGUAGE plpgsql; \n"
                    "",
                    "CREATE OR REPLACE FUNCTION urlmatch( varchar, varchar, boolean )  \n"
                    "  RETURNS setof int  \n"
                    "  IMMUTABLE AS $urlmatch$ \n"
                    "DECLARE \n"
                    "  qconst alias for $1; \n"
                    "  eaname alias for $2; \n"
                    "  caseSensitive alias for $3; \n"
                    "  q varchar; \n"
                    "  r record; \n"
                    "BEGIN \n"
                    "  q := qconst; \n"
                    "  if not caseSensitive and regexp_replace( q, '[a-zA-Z0-9]*', '') = '' then \n"
                    "    for r in select urlid from urlmap where to_tsvector('simple',fnshiftstring(url)) @@ to_tsquery('simple', q || ':*' ) loop \n"
                    "      return next r.urlid; \n"
                    "    end loop; \n"
                    "  elsif not caseSensitive and regexp_replace( q, '[a-zA-Z0-9]*\.[a-zA-Z0-9]*', '') = '' then \n"
                    "    arr := regexp_split_to_array( q, '\.' );\n"
                    "    for r in select urlid from urlmap \n"
                    "             where to_tsvector('simple',fnshiftstring(url)) @@ to_tsquery('simple', arr[1] || ':*' ) \n"
                    "             and   to_tsvector('simple',fnshiftstring(url)) @@ to_tsquery('simple', arr[2] || ':*' ) \n"
                    "             and   url ~* q\n"
                    "    loop \n"
                    "      return next r.urlid; \n"
                    "    end loop; \n"
                    "  else \n"
                    "    if eaname = 'name' then \n"
                    "       q := '.*/' || qconst || '[^/]*$'; \n"
                    "    end if; \n"
                    "    if caseSensitive then \n"
                    "      for r in select urlid from urlmap where url ~  q loop \n"
                    "        return next r.urlid; \n"
                    "      end loop; \n"
                    "    else \n"
                    "      for r in select urlid from urlmap where url ~* q loop \n"
                    "        return next r.urlid; \n"
                    "      end loop; \n"
                    "    end if; \n"
                    "  end if;     \n"
                    "  return; \n"
                    "END; \n"
                    "$urlmatch$ LANGUAGE plpgsql; \n"
                };
                
                Execute( trans, fourGL );
                Execute( trans,
                         "create index urlmapfnshiftstring on urlmap "
                         " using gin(to_tsvector('simple',fnshiftstring(url)));");

                trans.commit();
            }
            catch( exception& e )
            {
                if( isTrue( getStrSubCtx( md, "db-exists", "0" )))
                {
                    LG_EAIDX_D << "Assuming urlmap already exists in creation."
                               << " e:" << e.what() << endl;
                }
                else
                    throw;
            }
                        
            
            work CreateDBTrans( *m_connection, "setup database" );
            
            
            /*
             * Create a two strings, a docmap extra attributes which contains
             * the create table parts to create the m_ExtraColumnsToInlineInDocmap
             * columns and a indexing create string to index those columns correctly.
             */
            string extraDocmapColumns = "";
            stringlist_t extraDocmapIndexes;

            if( !m_ExtraColumnsToInlineInDocmap.empty() )
            {
                fh_stringstream colss;
                int n = 0;
                LG_EAIDX_D << "m_DocmapColumnsToFulltextIndex.sz:" << m_DocmapColumnsToFulltextIndex.size() << endl;
                
                for( stringmap_t::const_iterator si = m_ExtraColumnsToInlineInDocmap.begin();
                     si != m_ExtraColumnsToInlineInDocmap.end(); ++si )
                {
                    string colname = EANameToSQLColumnName( si->first );
                    string coltype = si->second;
                    
                    colss << "    \"" << colname << "\" " << coltype << ", ";

                    if( !canColumnHavePotentiallyHugeValue( colname ) )
                    {
                        fh_stringstream idxss;
                        idxss << "create index dmecti" << ++n << "idx"
                              << " on docmap ( \"" << colname << "\" )";
                        extraDocmapIndexes.push_back( tostr( idxss ) );
                        
                        if( starts_with( coltype, "varchar" ))
                        {
                            fh_stringstream idxss;
                            idxss << "create index dmecti" << ++n << "idxcis"
                                  << " on docmap ( lower(\"" << colname << "\") )";
                            extraDocmapIndexes.push_back( tostr( idxss ) );
                        }
                    }
                    
                    if(m_DocmapColumnsToFulltextIndex.count(EANameToSQLColumnName(colname)))
                    {
                        fh_stringstream idxss;
                        idxss << "create index dmect" << ++n << "ftx"
                              << " ON docmap USING gin(to_tsvector('english', \"" << colname << "\"));";
                        extraDocmapIndexes.push_back( tostr( idxss ) );
                    }
                }

                extraDocmapColumns = tostr( colss );
            }

            
            fh_stringstream docmapcreatess;
            docmapcreatess << "CREATE TABLE docmap ("
                           << "    urlid int NOT NULL references urlmap,"
                           << "    docid serial,"
                           << "    docidtime timestamp,"
                           <<      extraDocmapColumns
                           << "    bitf bit varying, "
                           << "    PRIMARY KEY  (docid)"
                           << "    ) without oids ";
            {
                string docmapcreateString = tostr(docmapcreatess);
                LG_EAIDX_D << "DOCMAP SQL:" << nl
                           << docmapcreateString
                           << endl;
                Execute( CreateDBTrans, docmapcreateString );
                LG_EAIDX_D << "DOCMAP CREATED!" << endl;
            
                {
                    string docmapmv = Util::replace_all( docmapcreateString,
                                                         "TABLE docmap",
                                                         "TABLE docmap_multiversion" );
                    Execute( CreateDBTrans, docmapmv );
                }
            }
            
            
            extraDocmapIndexes.push_back( "create index dmurlidx      on docmap ( urlid )" );
            extraDocmapIndexes.push_back( "create index docmapidtmidx on docmap ( docidtime )" );
            extraDocmapIndexes.push_back( "create index dmurlidtidx   on docmap ( docidtime,urlid )" );
            extraDocmapIndexes.push_back( "create index dmbitfidx     on docmap ( bitf )" );
            Execute( CreateDBTrans, extraDocmapIndexes );
            
            {
                stringlist_t::iterator si = extraDocmapIndexes.begin();
                stringlist_t::iterator se = extraDocmapIndexes.end();
                for( ; si != se; ++si )
                {
                    string s = *si;
                    s = Util::replace_all( s, "create index d", "create index mvd" );
                    s = Util::replace_all( s, "on docmap",       "on docmap_multiversion" );
                    Execute( CreateDBTrans, s );
                }
            }
            

            // auto_increment is non SQL92
            stringlist_t commands;

            commands.push_back( 
                "CREATE TABLE attrmap ("
                "    attrid   serial,"
                "    attrtype int NOT NULL default 1,"
                "    attrname varchar(" SQL_ATTRNAME_LENGTH_STR ") NOT NULL default '',"
                "    PRIMARY KEY  (attrid)"
                "    ) without oids " );
            
            commands.push_back( 
                "CREATE TABLE docattrs ("
                "    attrid int NOT NULL default 0 references attrmap,"
                "    vid int NOT NULL default 0,"
                "    docid int NOT NULL default 0 references docmap ON DELETE CASCADE,"
                "    PRIMARY KEY  (attrid,docid,vid)"
                "    ) without oids " );

            commands.push_back( "ALTER TABLE docattrs ALTER COLUMN attrid SET STATISTICS 1000" );
            commands.push_back( "ALTER TABLE docattrs ALTER COLUMN vid    SET STATISTICS 1000" );
            commands.push_back( "ALTER TABLE docattrs ALTER COLUMN docid  SET STATISTICS 1000" );

            commands.push_back( 
                "CREATE TABLE docattrs_multiversion ("
                "    attrid int NOT NULL default 0 references attrmap,"
                "    vid int NOT NULL default 0,"
                "    docid int NOT NULL default 0 references docmap_multiversion ON DELETE CASCADE,"
                "    PRIMARY KEY  (attrid,docid,vid)"
                "    ) without oids " );

            commands.push_back( "ALTER TABLE docattrs_multiversion ALTER COLUMN attrid SET STATISTICS 1000" );
            commands.push_back( "ALTER TABLE docattrs_multiversion ALTER COLUMN vid    SET STATISTICS 1000" );
            commands.push_back( "ALTER TABLE docattrs_multiversion ALTER COLUMN docid  SET STATISTICS 1000" );

            
            commands.push_back( 
                (string)("CREATE TABLE strlookup ("
                         "    vid serial,"
                         "    attrvalue varchar(")
                + tostr(getMaxValueSize())
                + ") unique"
                + "    ) without oids " );
            

            commands.push_back( 
                "CREATE TABLE doublelookup ("
                "    vid serial,"
                "    attrvalue real unique"
//              "    PRIMARY KEY  (attrvalue)"
                "    ) without oids " );

            commands.push_back( 
                "CREATE TABLE intlookup ("
                "    vid serial,"
                "    attrvalue numeric unique"
//              "    PRIMARY KEY  (attrvalue)"
                "    ) without oids " );
            
            commands.push_back( 
                "CREATE TABLE timelookup ("
                "    vid serial,"
                "    attrvalue timestamp unique"
//              "    PRIMARY KEY  (attrvalue)"
                "    ) without oids " );

                    ////////////////////////

            commands.push_back( 
                (string)("CREATE TABLE strlookup_input ("
                         "    attrvalue varchar(")
                + tostr(getMaxValueSize())
                + "),"
                + "    aid int"
                + "    ) without oids " );

            commands.push_back( 
                    "CREATE TABLE doublelookup_input ("
                    "    attrvalue real,"
                    "    aid int"
                    "    ) without oids " );

            commands.push_back( 
                    "CREATE TABLE intlookup_input ("
                    "    attrvalue bigint,"
                    "    aid int"
                    "    ) without oids " );

            commands.push_back( 
                    "CREATE TABLE timelookup_input ("
                    "    attrvalue timestamp,"
                    "    aid int"
                    "    ) without oids " );

            Execute( CreateDBTrans, commands );

            /**
             * Create the indexes for using the database effectively.
             */
            const char** indexing_commands = get_defaultindexing_create_commands();
            Execute( CreateDBTrans, indexing_commands );

            // make sure the empty string is awaiting us in the lookup tables.
            {
                stringlist_t ddl;
                ddl.push_back( "insert into strlookup (attrvalue) values ('') " );
                Execute( CreateDBTrans, ddl );
            }

            // Some 4GL to make the SQL a little more useful for the packed booleans
            // in bitf
            {
//                int bitfSize = getBitFLength();
                m_bitfColumns_t::const_iterator    e = m_bitfColumns.end();
                m_bitfColumns_t::const_iterator iter = m_bitfColumns.begin();
                for( ; iter != e ; ++iter )
                {
                    CreateIndex_createBitf4GLFunction( CreateDBTrans, iter );
                }
            }


            // Convert a bit varying -> an int[] so it can be rd-tree indexed.
            create_bitf_to_intvec_function_and_bitf_rdtree_index( CreateDBTrans );
            
            
            {
                static const char* fourGL[] = {
                    " "

//                     "--"
//                     "-- Expects a table strlookup_input with atleast these columns"
//                     "-- create table strlookup_input ( "
//                     "--	attrvalue varchar,"
//                     "--	aid integer );"
//                     "-- And inserts the attrvalue into the strlookup table if it doesn't "
//                     "-- already exist there. The vid for the given attrvalue is taken"
//                     "-- from the strlookup table and using the function arg docid and"
//                     "-- the aid from the tuple a new entry in docattrs is created."
//                     "--"

              
                    "CREATE OR REPLACE FUNCTION strlookup_consume( integer ) RETURNS void AS '"
                    "DECLARE "
                    "	mydocid alias for $1;"
                    "	val varchar default null;"
                    "	myvid integer default -1;"
                    "	myaid integer default -1;"
                    "	myrow RECORD;"
                    "	myvalue varchar;"
                    "BEGIN"
                    "   insert into strlookup (attrvalue)  "
                    "       select distinct(attrvalue) from strlookup_input "
                    "            where attrvalue not in (select attrvalue from strlookup);"
                    "   FOR myrow in select l.attrvalue,l.vid,aid from strlookup_input i, strlookup l "
                    "                       where i.attrvalue=l.attrvalue LOOP "
                    "		myvalue := myrow.attrvalue;"
                    "		myaid   := myrow.aid;"
                    "		myvid   := myrow.vid;"
                    "		insert into docattrs (attrid,vid,docid) values ( myaid, myvid, mydocid );"
                    "	END LOOP;"
                    "	delete from strlookup_input;"
                    
                    "	RETURN;"
                    "END;"
                    "' LANGUAGE plpgsql;",
                    
//                     "--"
//                     "-- Expects a table intlookup_input with atleast these columns"
//                     "-- create table intlookup_input ( "
//                     "--	attrvalue varchar,"
//                     "--	aid integer );"
//                     "-- And inserts the attrvalue into the intlookup table if it doesn't "
//                     "-- already exist there. The vid for the given attrvalue is taken"
//                     "-- from the intlookup table and using the function arg docid and"
//                     "-- the aid from the tuple a new entry in docattrs is created."
//                     "--"
                    "CREATE OR REPLACE FUNCTION intlookup_consume( integer ) RETURNS void AS '"
                    "DECLARE "
                    "	mydocid alias for $1;"
                    "	val integer default null;"
                    "	myvid integer default -1;"
                    "	myaid integer default -1;"
                    "	myrow RECORD;"
                    "	myvalue integer;"
                    "BEGIN"
                    "   insert into intlookup (attrvalue)  "
                    "       select distinct(attrvalue) from intlookup_input "
                    "            where attrvalue not in (select attrvalue from intlookup);"
                    "   FOR myrow in select l.attrvalue,l.vid,aid from intlookup_input i, intlookup l "
                    "                       where i.attrvalue=l.attrvalue LOOP "
                    "		myvalue := myrow.attrvalue;"
                    "		myaid   := myrow.aid;"
                    "		myvid   := myrow.vid;"
                    "		insert into docattrs (attrid,vid,docid) values ( myaid, myvid, mydocid );"
                    "	END LOOP;"
                    "	delete from intlookup_input;"
                    
                    "	RETURN;"
                    "END;"
                    "' LANGUAGE plpgsql;",
                    ""
                    ""
                    "CREATE OR REPLACE FUNCTION doublelookup_consume( integer ) RETURNS void AS '"
                    "DECLARE "
                    "	mydocid alias for $1;"
                    "	val real default null;"
                    "	myvid integer default -1;"
                    "	myaid integer default -1;"
                    "	myrow RECORD;"
                    "	myvalue real;"
                    "BEGIN"
                    "   insert into doublelookup (attrvalue)  "
                    "       select distinct(attrvalue) from doublelookup_input "
                    "            where attrvalue not in (select attrvalue from doublelookup);"
                    "   FOR myrow in select l.attrvalue,l.vid,aid from doublelookup_input i, doublelookup l "
                    "                       where i.attrvalue=l.attrvalue LOOP "
                    "		myvalue := myrow.attrvalue;"
                    "		myaid   := myrow.aid;"
                    "		myvid   := myrow.vid;"
                    "		insert into docattrs (attrid,vid,docid) values ( myaid, myvid, mydocid );"
                    "	END LOOP;"
                    "	delete from doublelookup_input;"
                    
                    "	RETURN;"
                    "END;"
                    "' LANGUAGE plpgsql;",
                    ""
                    ""
                    "CREATE OR REPLACE FUNCTION timelookup_consume( integer ) RETURNS void AS '"
                    "DECLARE "
                    "	mydocid alias for $1;"
                    "	val timestamp default null;"
                    "	myvid integer default -1;"
                    "	myaid integer default -1;"
                    "	myvalue timestamp;"
                    "	myrow RECORD;"
                    "BEGIN"
                    "   insert into timelookup (attrvalue)  "
                    "       select distinct(attrvalue) from timelookup_input "
                    "            where attrvalue not in (select attrvalue from timelookup);"
                    "   FOR myrow in select l.attrvalue,l.vid,aid from timelookup_input i, timelookup l "
                    "                       where i.attrvalue=l.attrvalue LOOP "
                    "		myvalue := myrow.attrvalue;"
                    "		myaid   := myrow.aid;"
                    "		myvid   := myrow.vid;"
                    "		insert into docattrs (attrid,vid,docid) values ( myaid, myvid, mydocid );"
                    "	END LOOP;"
                    "	delete from timelookup_input;"
                    
                    "	RETURN;"
                    "END;"
                    "' LANGUAGE plpgsql;",
                    ""
                    ""
                    "CREATE OR REPLACE FUNCTION xlookup_consume( integer ) RETURNS void AS '"
                    "DECLARE "
                    "	mydocid alias for $1;"
                    "BEGIN"
                    "	perform strlookup_consume( mydocid );"
                    "	perform intlookup_consume( mydocid );"
                    "	perform doublelookup_consume( mydocid );"
                    "	perform timelookup_consume( mydocid );"
                    "	return;"
                    "END;"
                    "' LANGUAGE plpgsql;"
                };
                
                LG_IDX_D << "SQL:" << fourGL << endl;
                Execute( CreateDBTrans, fourGL );
            }
             
            {
                CreateIndex_remakeView_currentVersions( CreateDBTrans );
                
//                 stringstream ss;
//                 ss << "create or replace view currentVersions as " << nl
//                    << "SELECT d.*" << nl
//                    << "FROM docmap d," << nl
//                    << "  ( select max(docidtime) as ddtime, urlid" << nl
//                    << "    from docmap" << nl
//                    << "    group by urlid" << nl
//                    << "   ) dd" << nl
//                    << "WHERE d.urlid=dd.urlid" << nl
//                    << "AND d.docidtime=dd.ddtime;" << nl;
//                 Execute( CreateDBTrans, ss.str() );
            }
            
            
            CreateDBTrans.commit();

            // we always have access to the URL string
            // and map queries to use that string instead of strlookup.
            appendToEANamesIgnore("url");
        }

        bool
        EAIndexerPostgresql::canColumnHavePotentiallyHugeValue( const std::string& colname )
        {
            return colname == "subtitles";
        }
        
        void
        EAIndexerPostgresql::createIndexesForDocmapColumn( work& trans,
                                                           const std::string& colname,
                                                           const std::string& coltype,
                                                           bool fulltextOnly )
        {
            long n = toType<long>( getConfig( "temp-index-last-id", "1" ));

            if( !fulltextOnly && !canColumnHavePotentiallyHugeValue( colname ) )
            {
                
                fh_stringstream idxss;
                idxss << "create index tidx" << n++
                      << " on docmap ( \"" << colname << "\" )";
                trans.exec( tostr(idxss) );
                
                if( starts_with( coltype, "varchar" ))
                {
                    fh_stringstream idxss;
                    idxss << "create index tidx" << n++
                          << " on docmap ( lower(\"" << colname << "\") )";
                    trans.exec( tostr(idxss) );
                }
            }
            
            if(m_DocmapColumnsToFulltextIndex.count(EANameToSQLColumnName(colname)))
            {
                fh_stringstream idxss;
                idxss << "create index tidx" << n++
                      << " ON docmap USING gin(to_tsvector('english', \"" << colname << "\"));";
                trans.exec( tostr(idxss) );
            }

            setConfig( "temp-index-last-id", tostr(n) );
        }
        
        void
        EAIndexerPostgresql::setupBitfColumnsFromString( const std::string& s, bool isNSV )
        {
//            LG_EAIDX_D << "setupBitfColumnsFromString() s.len:" << s.length() << " s:" << s << endl;

            stringlist_t bitflist;
            if( isNSV )
            {
                Util::parseNullSeperatedList( s, bitflist );
            }
            else
            {
                Util::parseCommaSeperatedList( s, bitflist );
            }
            stringlist_t::const_iterator bi = bitflist.begin();
            stringlist_t::const_iterator e  = bitflist.end();
            for( int i = 1; bi != e; ++i, ++bi )
            {
                m_bitfColumns[ *bi ] = i;
            }
        }
        
        void
        EAIndexerPostgresql::CommonConstruction()
        {
            m_ExtraColumnsToInlineInDocmap =
                Util::ParseKeyValueString(
                    getConfig( CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K,
                               CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_DEFAULT )
                    );

            Util::parseCommaSeperatedList( getConfig( CFG_POSTGRESQLIDX_DOCMAP_COLUMNS_TO_FULLTEXT_INDEX_K,
                                                      CFG_POSTGRESQLIDX_DOCMAP_COLUMNS_TO_FULLTEXT_INDEX_DEFAULT ),
                                           m_DocmapColumnsToFulltextIndex );
            
            string nsv = getConfig( CFG_POSTGRESQLIDX_COLUMNS_TO_INLINE_IN_BITF_NSV_K, "" );
            if( !nsv.empty() )
            {
                setupBitfColumnsFromString( nsv, true );
            }
            else
            {
                string bitfColumnsRAW = getConfig(
                    CFG_POSTGRESQLIDX_COLUMNS_TO_INLINE_IN_BITF_K,
                    CFG_POSTGRESQLIDX_COLUMNS_TO_INLINE_IN_BITF_DEFAULT );
                setupBitfColumnsFromString( bitfColumnsRAW );
            }
            
            m_useGIST = isTrue( getConfig( CFG_IDX_USE_GIST_K, CFG_IDX_USE_GIST_DEF ) );
            m_multiVersion = isTrue( getConfig( CFG_IDX_MULTIVERSION_K, CFG_IDX_MULTIVERSION_DEF ) );
        }

        bool
        EAIndexerPostgresql::useGIST()
        {
            return m_useGIST;
        }
        
        
        
        void
        EAIndexerPostgresql::sync()
        {
//            m_isFileNewerThanIndexedVersionCache.clear();
            
//            m_trans->commit();
            
//             try
//             {
//                 cerr << "EAIndexerPostgresql::sync()" << endl;
//                 work tr( *m_connection, "setup database" );
//                 const char** indexing_commands = get_defaultindexing_create_commands();
//                 Execute( tr, indexing_commands );
//                 tr.commit();
//             }
//             catch( exception& e )
//             {}

            
//            #define LG_EAIDX_D cerr

            // This is quite slow for large batches (say, 200,000 files takes about 10 minutes)
            // and doesn't actaully gain us much.
            if( 0 )
            {
                LG_EAIDX_D << "updating docidtimes for unchanged documents sz:" << m_isFileNewerThanIndexedVersionSkippedDocuments.size() << endl;
                work trans( *m_connection, "update docidtimes" );

                {
                    stringstream ss;
                    ss << "create temporary table updates "
                       << " ( earl varchar(" << SQL_URL_LENGTH_STR << "), dt timestamp )"
                       << " on commit drop;";
                    Execute( trans, tostr(ss) );
                }

                for( rs<m_isFileNewerThanIndexedVersionSkippedDocuments_t> r( m_isFileNewerThanIndexedVersionSkippedDocuments );
                     r; ++r )
                {
                    stringstream ss;
                    string earl = r->first;
                    time_t tt = r->second;
                    ss << "insert into updates values ('" << trans.esc(earl) << "'"
                       << ",'" << toSQLTimeString(tt) << "' );";
                    Execute( trans, tostr(ss) );
                }
                
                
                {
                    stringstream ss;
                    ss << "update docmap set docidtime = z.dt from " << nl
                       << "    (select url,c.urlid,c.docid,dt " << nl
                       << "     from updates z, urlmap u, currentversions c " << nl
                       << "     where z.earl = u.url and u.urlid = c.urlid) as z" << nl
                       << "  where docmap.docid = z.docid;" << endl;
                    Execute( trans, tostr(ss) );
                }
                LG_EAIDX_D << "commiting..." << endl;
                trans.commit();
            }

            LG_EAIDX_D << "retire old docids..." << endl;
            retire_old_docids_from_docmap();
            
            LG_EAIDX_D << "perform analyse..." << endl;
            if( m_filesIndexedSinceAnalyseCount > 5000 )
            {
                m_filesIndexedSinceAnalyseCount = 0;
                cerr << "Performing PostgreSQL ANALYZE. filesIndexedCount:" << getFilesIndexedCount() << endl;
                nontransaction trans( *m_connection, "analyse" );
            
                LG_EAIDX_I << "EAIndexerPostgresql::sync() analyse" << endl;
                fh_stringstream ss;
                ss << "ANALYZE";
                trans.exec( tostr(ss) );
            }
            LG_EAIDX_D << "Done..." << endl;
        }
//#define LG_EAIDX_D LG_EAIDX(Ferris::Timber::_SBufT::PRI_DEBUG)

        void
        EAIndexerPostgresql::retireDocumentsFromIndex( stringset_t& urls )
        {
            if( urls.empty() )
                return;

            LG_EAIDX_D << "EAIndexerPostgresql::retireDocumentsFromIndex() urls.sz:" << urls.size() << endl;
            stringstream qss;
            qss << "select docid from urlmap u, docmap d where url in ('";
            qss << Util::createSeperatedList( urls.begin(), urls.end(), "','" );
            qss << "') and d.urlid = u.urlid;";

            docNumSet_t docs;
            {
                work trans( *m_connection, "retire old metadata instances" );
                result res;
                ExecuteRes( res, trans, tostr(qss) );
                for (result::const_iterator c = res.begin(); c != res.end(); ++c)
                {
                    long t;
                    c[0].to(t);
                    docs.insert(t);
                }
            }
            
            LG_EAIDX_D << "EAIndexerPostgresql::retireDocumentsFromIndex() docs.sz:" << docs.size() << endl;
            retireDocumentsFromIndex( docs );
        }
        
        void
        EAIndexerPostgresql::retireDocumentsFromIndex( docNumSet_t& docids )
        {
            if( docids.empty() )
                return;

            LG_EAIDX_D << "moving some explicit urls from docmap and docattrs into _multiversion tables..." << endl;
            work trans( *m_connection, "retire old metadata instances" );

            string docidsbracketed;
            {
                stringstream ss;
                ss << " (";
                ss << Util::createSeperatedList( docids.begin(), docids.end(), ',' );
                ss << " ) ";
                docidsbracketed = ss.str();
            }
            
            
            if( m_multiVersion )
            {
                {
                    stringstream ss;
                    ss << "insert into docmap_multiversion "
                       << "    ( select *  from docmap where docid in " << docidsbracketed << ");";
                    trans.exec( tostr(ss) );
                }
                {
                    stringstream ss;
                    ss << "insert into docattrs_multiversion "
                       << "   ( select *  from docattrs where (docid in " << docidsbracketed << " ));";
                    trans.exec( tostr(ss) );
                }
            }
            {
                // cascade FK will cleanup docattrs for me.
                stringstream ss;
                ss << "delete from docmap where docid in " << docidsbracketed << ";";
                trans.exec( tostr(ss) );
            }
                
            LG_EAIDX_D << "commiting..." << endl;
            trans.commit();
        }
        
        void
        EAIndexerPostgresql::retire_old_docids_from_docmap()
        {
            LG_EAIDX_D << "moving older versions of file metadata from docmap and docattrs into _multiversion tables..." << endl;
            work trans( *m_connection, "retire old metadata instances" );

            if( m_multiVersion )
            {
                {
                    stringstream ss;
                    ss << "insert into docmap_multiversion "
                       << "    ( select *  from docmap where not(docid in ( select docid from currentversions )));";
                    trans.exec( tostr(ss) );
                }
                {
                    stringstream ss;
                    ss << "insert into docattrs_multiversion "
                       << "   ( select *  from docattrs where (docid in ( select docid from docmap_multiversion )));";
                    trans.exec( tostr(ss) );
                }
            }
            {
                // cascade FK will cleanup docattrs for me.
                stringstream ss;
                ss << "delete from docmap where  "
                   << "    not( docid in ( select docid from currentversions ));";
                trans.exec( tostr(ss) );
            }
                
            LG_EAIDX_D << "commiting..." << endl;
            trans.commit();
        }

        
        void
        EAIndexerPostgresql::compact( fh_ostream oss, bool verbose )
        {
            cerr << "running retire..." << endl;
            retire_old_docids_from_docmap();

            cerr << "running lstat() check on each url..." << endl;

            //
            // Remove any URLs which do not resolve into the multiversion
            //
            {
                work trans( *m_connection, "stale urls" );

                typedef map< string, docid_t > urls_t;
                urls_t urls;
                urls_t nonResolvableURLs;
                {
                    stringstream ss;
                    ss << "select url, cv.docid, cv.urlid from urlmap u, currentversions cv where u.urlid=cv.urlid; ";
                    result res;
                    res = ExecuteRes( res, trans, tostr(ss) );
                    for (result::const_iterator c = res.begin(); c != res.end(); ++c)
                    {
                        string earl;
                        docid_t docid;
                        c[0].to(earl);
                        c[1].to(docid);
                        urls[ earl ] = docid;
                    }
                }
                cerr << "urls.sz:" << urls.size() << endl;
                
                {
                    urls_t::iterator iter = urls.begin();
                    urls_t::iterator e    = urls.end();
                    for( ; iter != e; ++iter )
                    {
//                        cerr << "earl:" << iter->first << endl;
                        if( !canResolve( iter->first ) )
                        {
//                            cerr << "------------------is BAD!" << endl;
                            nonResolvableURLs[ iter->first ] = iter->second;
                        }
                    }
                }
                
                cerr << "bad urls.sz:" << nonResolvableURLs.size() << endl;
                if( !nonResolvableURLs.empty() )
                {
                
                    {
                        stringstream ss;
                        ss << "create temporary table nonresolvableurls "
                           << " ( "
//                       << " earl varchar(" << SQL_URL_LENGTH_STR << "),"
                           << " docid int )"
                           << " on commit drop;";
                        Execute( trans, tostr(ss) );
                    }
                    {
                        urls_t::iterator iter = nonResolvableURLs.begin();
                        urls_t::iterator e    = nonResolvableURLs.end();
                        for( ; iter != e; ++iter )
                        {
                            stringstream ss;
                            ss << "insert into nonresolvableurls values (" << iter->second << ");";
//                        ss << "insert into nonresolvableurls values ('" << trans.esc(iter->first) << "'," << iter->second << ");";
                            trans.exec( tostr(ss) );
                        }
                    }

                    cerr << "removing bad urls" << endl;
                    //
                    // remove all the docids in nonresolvableurls.
                    //
                    if( m_multiVersion )
                    {
                        {
                            stringstream ss;
                            ss << "insert into docmap_multiversion "
                               << "    ( select *  from docmap where docid in ( select docid from nonresolvableurls ));";
                            trans.exec( tostr(ss) );
                        }
                        cerr << "removing bad urls 2" << endl;
                        {
                            stringstream ss;
                            ss << "insert into docattrs_multiversion "
                               << "   ( select *  from docattrs where (docid in ( select docid from nonresolvableurls )));";
                            trans.exec( tostr(ss) );
                        }
                        cerr << "removing bad urls 3" << endl;
                    }
                    {
                        // cascade FK will cleanup docattrs for me.
                        stringstream ss;
                        ss << "delete from docmap where  "
                           << "    docid in ( select docid from nonresolvableurls );";
                        trans.exec( tostr(ss) );
                    }
                }
                trans.commit();
            }

            //
            // handle adding a fulltext index on the new inline column
            //
            stringset_t addFullTextIndexForColumnsInline;
            if( const gchar* p = g_getenv ("LIBFERRIS_ADD_FULLTEXT_INDEX_FOR_COLUMNS_INLINE") )
            {
                stringset_t idxcols;
                Util::parseCommaSeperatedList( p, idxcols );
                addFullTextIndexForColumnsInline = idxcols;
                for( stringset_t::iterator si = idxcols.begin(); si != idxcols.end(); ++si )
                {
                    m_DocmapColumnsToFulltextIndex.insert( EANameToSQLColumnName(*si) );
                }
                setConfig( CFG_POSTGRESQLIDX_DOCMAP_COLUMNS_TO_FULLTEXT_INDEX_K,
                           Util::createCommaSeperatedList( m_DocmapColumnsToFulltextIndex )
                    );
            }
            
            //
            // Possibly create a new column in docmap...
            //
            if( const gchar* p = g_getenv ("LIBFERRIS_ADD_COLUMNS_INLINE") )
            {
                work trans( *m_connection, "create column" );
                stringmap_t newInlineColumns;
                Util::ParseKeyValueString( newInlineColumns, p );
                if( !newInlineColumns.empty() )
                {
                    LG_EAIDX_D << "newInlineColumns.sz:" << newInlineColumns.size()
                               << " p:" << p << endl;
                    
                    for( stringmap_t::iterator si = newInlineColumns.begin();
                         si != newInlineColumns.end(); ++si )
                    {
                        string eaname = si->first;
                        string columnName = EANameToSQLColumnName( eaname );
                        string columnType = si->second;

                        // add column
                        {
                            stringstream ss;
                            ss << "alter table docmap add column " << columnName << " " << columnType;
                            LG_EAIDX_D << "SQL:" << tostr(ss) << endl;
                            Execute( trans, tostr(ss) );
                        }

                        // index the new column
                        createIndexesForDocmapColumn( trans, columnName, columnType );
                        addFullTextIndexForColumnsInline.erase( EANameToSQLColumnName(columnName) );
                    }

                    // remake the views
                    {
                        CreateIndex_remakeView_currentVersions( trans );
                    }
                }
                trans.commit();

                LG_EAIDX_D << "adding to internal map, newInlineColumns.sz:" << newInlineColumns.size() << endl;
                for( stringmap_t::iterator mi = newInlineColumns.begin();
                     mi != newInlineColumns.end(); ++mi )
                {
                    m_ExtraColumnsToInlineInDocmap[ mi->first ] = mi->second;
                }
                setConfig( CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K,
                           Util::CreateKeyValueString( m_ExtraColumnsToInlineInDocmap, "=", ":" )
                    );
            }

            //
            // a new fulltext index on an existing inline column
            //
            if( !addFullTextIndexForColumnsInline.empty() )
            {
                work trans( *m_connection, "create ftx index" );
                stringset_t idxcols = addFullTextIndexForColumnsInline;
                for( stringset_t::iterator si = idxcols.begin(); si != idxcols.end(); ++si )
                {
                    string columnName = *si;
                    string columnType = "";
                    bool fulltextIndexOnly = true;
                    createIndexesForDocmapColumn( trans,
                                                  columnName,
                                                  columnType,
                                                  fulltextIndexOnly );
                }
                trans.commit();
            }
            
            

            //
            // Possibly create a new column in the normalized tables strlookup, intlookup etc...
            //
            if( const gchar* p = g_getenv ("LIBFERRIS_ADD_COLUMNS") )
            {
                work trans( *m_connection, "create column" );
                stringmap_t newInlineColumns;
                Util::ParseKeyValueString( newInlineColumns, p );
                if( !newInlineColumns.empty() )
                {
                
                    for( stringmap_t::iterator si = newInlineColumns.begin();
                         si != newInlineColumns.end(); ++si )
                    {
                        string eaname = si->first;
                        string columnName = EANameToSQLColumnName( eaname );
                        string columnType = si->second;
                        AttrType_t att = ATTRTYPEID_CIS;

                        columnType = tolowerstring( columnType );
                        if( starts_with( columnType, "int" ) )
                            att = ATTRTYPEID_INT;
                        if( starts_with( columnType, "numeric" ) )
                            att = ATTRTYPEID_INT;
                        if( starts_with( columnType, "doub" ) )
                            att = ATTRTYPEID_DBL;
                        if( starts_with( columnType, "floa" ) )
                            att = ATTRTYPEID_DBL;
                        if( starts_with( columnType, "real" ) )
                            att = ATTRTYPEID_DBL;

                        // add column...
                        // as the strlookup et al are dynamic, we just have to record
                        // this in the attrmap table.
                        stringstream ss;
                        ss << "INSERT INTO attrmap (attrtype,attrname) VALUES ("
                           << att << "," << quoteStr( trans, eaname ) << " );";
                        Execute( trans, tostr(ss) );
                    }
                }
                trans.commit();
            }
            
            
            //
            // Possibly move some normalized data into docmap...
            //
            if( const gchar* p = g_getenv ("LIBFERRIS_ADD_TO_INLINE") )
            {
                work trans( *m_connection, "denormalize" );
                stringmap_t newInlineColumns;
                int idxseq = m_ExtraColumnsToInlineInDocmap.size() + 10;

                cerr << "moving some EA into the main docmaps table..." << endl;

                Util::ParseKeyValueString( newInlineColumns, p );
                if( !newInlineColumns.empty() )
                {
                    for( stringmap_t::iterator si = newInlineColumns.begin();
                         si != newInlineColumns.end(); ++si )
                    {
                        string eaname = si->first;
                        string columnName = EANameToSQLColumnName( eaname );
                        string columnType = si->second;
                        string attrmapTypes = "";
                        AttrType_t att = ATTRTYPEID_CIS;
                        {
                            IndexableValue iv( this, "eaname", "eavalue" );
                            att = SQLColumnTypeToAttrType( columnType, iv );
                        }
                        if( att >= ATTRTYPEID_STR )
                        {
                            stringstream sqlss;
                            sqlss << " attrmap.attrtype=\'" << att << "\' or  attrmap.attrtype=\'" << ATTRTYPEID_STR << "\' ";
                            attrmapTypes = tostr(sqlss);
                        }
                        else
                        {
                            stringstream sqlss;
                            sqlss << " attrmap.attrtype=\'" << att << "\' ";
                            attrmapTypes = tostr(sqlss);
                        }

                        cerr << "denormalizing EA:" << eaname << endl;
                        
                        // add column
                        {
                            stringstream ss;
                            ss << "alter table docmap add column " << columnName << " " << columnType;
                            LG_EAIDX_D << "SQL:" << tostr(ss) << endl;
                            Execute( trans, tostr(ss) );
                        }

                        // grab the data
                        {
                            stringstream ss;
                            ss << "update docmap " << endl
                               << "set " << columnName << " = l.attrvalue"  << endl;
                            if( starts_with( columnType, "bool" ))
                            {
                                ss << " != '0' " << endl;
                            }
                            ss << "from docattrs da, strlookup l"  << endl
                               << " WHERE"  << endl
                               << " docmap.docid = da.docid and"  << endl
                               << " da.vid = l.vid and"  << endl
                               << " attrid = ( select attrid from attrmap where "  << endl
                               << "   attrmap.attrname='" << eaname << "' "  << endl
                               << "   and ( " << attrmapTypes << " ) );";
                            LG_EAIDX_D << "SQL:" << tostr(ss) << endl;
                            Execute( trans, tostr(ss) );
                        }

                        // index the new column
                        if( !canColumnHavePotentiallyHugeValue( columnName ) )
                        {
                            stringstream ss;
                            ss << "create index dmecti" << ++idxseq << "idx"
                               << " on docmap ( \"" << columnName << "\" )";
                            LG_EAIDX_D << "SQL:" << tostr(ss) << endl;
                            Execute( trans, tostr(ss) );

                            if( starts_with( columnType, "varchar" ))
                            {
                                fh_stringstream ss;
                                ss << "create index dmecti" << ++idxseq << "idxcis"
                                   << " on docmap ( lower(" << columnName << ") )";
                                LG_EAIDX_D << "SQL:" << tostr(ss) << endl;
                                Execute( trans, tostr(ss) );
                            }
                        }
                    }

                    // remake the views
                    {
                        CreateIndex_remakeView_currentVersions( trans );
                    }
                    
                }
                trans.commit();

                for( stringmap_t::iterator mi = newInlineColumns.begin();
                     mi != newInlineColumns.end(); ++mi )
                {
                    m_ExtraColumnsToInlineInDocmap[ mi->first ] = mi->second;
                }
                setConfig( CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K,
                           Util::CreateKeyValueString( m_ExtraColumnsToInlineInDocmap, "=", ":" )
                    );
            }

            
            // main steps.
            // drop bitf rdtree index 
            // move emblems from intlookup into docmap
            // recreate bitf rdtree index and docmap_bitf_to_intvec() function
            // run analyze
            work trans( *m_connection, "compact" );

            fh_etagere et = Ferris::Factory::getEtagere();
            
            // work out the names of emblems which have been added to the system
            // since the eaindex was created.
            // these are like:
            // emblem:id-1127
            // emblem:has-new5
            // not
            //    emblem:has-fuzzy-new5
            //    emblem:id-fuzzy-1127
            //    emblem:has-new5-mtime
            //
            stringset_t attrnames;
            stringset_t emblemsToMove_EANames;
            emblemset_t emblemsToMove;
            {
                result res;
                stringstream ss;
                ss << "select attrname from attrmap where \n"
                   << "lower(attrname) ~ lower('^emblem:(has-|id-).*') \n"
                   << " and not attrname ~ '(^emblem:(id|has)-fuzzy-.*|.*-mtime)'"
                   << " and not attrname ~ '^emblem:has-medallion' ";
                
                res = ExecuteRes( res, trans, tostr(ss) );
                for (result::const_iterator c = res.begin(); c != res.end(); ++c)
                {
                    string t;
                    c[0].to(t);
                    attrnames.insert(t);
                }
            }

            if( !attrnames.empty() )
            {
                if( useGIST() )
                {
                    Execute( trans, "drop index dmbitfidxrd");
                }
            
                LG_EAIDX_D << "attrnames.sz:" << attrnames.size() << endl;
            
                // work out the emblems we want to move into the bitf.
                for( rs<stringset_t> r(attrnames); r; ++r )
                {
                    LG_EAIDX_D << "iter on eanames:" << *r << endl;
                
                    if( starts_with( *r, "emblem:id-" ) )
                    {
                        emblemsToMove_EANames.insert( *r );

                        string eidstr = r->substr(strlen("emblem:id-"));
                        emblemID_t eid = toType<emblemID_t>( eidstr );
                        fh_emblem em = et->getEmblemByID( eid );

                        if( !em )
                        {
                            LG_EAIDX_W << "Can not find emblem id:" << eid << endl;
                            continue;
                        }
                        emblemsToMove.insert( em );
                    }
                    if( starts_with( *r, "emblem:has-" ) )
                    {
                        emblemsToMove_EANames.insert( *r );

                        string n = r->substr(strlen("emblem:has-"));
                        fh_emblem em = et->getEmblemByName( n );
                        if( !em )
                        {
                            LG_EAIDX_W << "Can not find emblem name:" << n << endl;
                            continue;
                        }
                        emblemsToMove.insert( em );
                    }
                }

                LG_EAIDX_D << "Total emblem ea to move:" << emblemsToMove.size() << endl;
                for( rs<stringset_t> r(emblemsToMove_EANames); r; ++r )
                    LG_EAIDX_D << "found new emblem ea:" << *r << endl;

                // create a new place for this information in bitf
                {
                    m_bitfColumns_t::const_iterator    e = m_bitfColumns.end();
                    for( rs<emblemset_t> r(emblemsToMove); r; ++r )
                    {
                        fh_emblem em = *r;

                        CreateIndex_addEmblemToBitfColumns( em );

                        string eaname = EANAME_SL_EMBLEM_PREKEY + em->getName();
                        m_bitfColumns_t::const_iterator iter = e;
                        iter = m_bitfColumns.find( eaname );
                        if( iter != e )
                        {
                            CreateIndex_createBitf4GLFunction( trans, iter );
                        }

                        eaname = EANAME_SL_EMBLEM_ID_PREKEY + tostr(em->getID());
                        iter = m_bitfColumns.find( eaname );
                        if( iter != e )
                        {
                            CreateIndex_createBitf4GLFunction( trans, iter );
                        }
                    }
                }

                // copy across the normalized data into bitf
                {
                    int bitfSize = getBitFLength();
                    stringlist_t updates;
                
                    for( rs<emblemset_t> r(emblemsToMove); r; ++r )
                    {
                        fh_emblem em = *r;
                    
                        stringstream sqlss;
                        sqlss << ""
                              << "SELECT d.urlid as urlid  \n"
                              << "FROM docmap d  \n"
                              << "WHERE  \n"
                              << " \n"
                              << "d.docid in \n"
                              << "   (    SELECT   docattrs.docid   as docid  \n"
                              << "      FROM docattrs, attrmap , intlookup \n"
                              << "      WHERE  \n"
                              << "      ((  \n"
                              << "          intlookup.attrvalue  =           1   \n"
                              << "       and intlookup.vid = docattrs.vid  \n"
                              << "       and  attrmap.attrname='emblem:id-" << em->getID() << "'  \n"
                              << "       and  attrmap.attrtype='1'  \n"
                              << "       and  attrmap.attrid=docattrs.attrid  \n"
                              << "      )) \n"
                              << "   ) \n";
                
                        result res;
                        res = ExecuteRes( res, trans, tostr(sqlss) );

                        m_bitfColumns_t::const_iterator iter = m_bitfColumns.end();

                        stringlist_t eanames;
                        eanames.push_back( EANAME_SL_EMBLEM_PREKEY + em->getName() );
                        eanames.push_back( EANAME_SL_EMBLEM_ID_PREKEY + tostr(em->getID()) );

                        for( rs<stringlist_t> eaiter(eanames); eaiter; ++eaiter )
                        {
                            string eaname = *eaiter;
                            iter = m_bitfColumns.find( eaname );
                            if( iter == m_bitfColumns.end() )
                            {
                                LG_EAIDX_W << "Can't find bitfColumn for ea name:" << eaname << endl;
                                continue;
                            }
                        
                            string attributeName = iter->first;
                            int    ShiftCount    = iter->second;

                            for (result::const_iterator c = res.begin(); c != res.end(); ++c)
                            {
                                string urlid;
                                c[0].to(urlid);
                        
                                stringstream ss;
                                ss << "update docmap "
                                   << " set bitf = "
                                   << " ( bitf::bit(" << bitfSize << ") | "
                                   << "     (B'1'::bit(" << bitfSize << ") >> " << ShiftCount << ")"
                                   << " )"
                                   << " where urlid='" << urlid << "' "
                                   << "";
                                updates.push_back( tostr(ss) );
                            }
                        }
                    }
                    Execute( trans, updates );
                }

                // delete the normalized data
                {
                    cerr << "delete the normalized version of the data..." << endl;
                    for( rs<stringset_t> r(attrnames); r; ++r )
                    {
                        LG_EAIDX_D << "removing emblem name from normalized data table for eaname:" << *r << endl;
                        stringstream ss;
//                         ss << "delete from attrmap where \n"
//                            << "attrname = '" << *r << "';";
                        ss << "update attrmap set attrname = '_moved_inline_' || attrname "
                           << "  where attrname = '" << *r << "';";
                        
                        Execute( trans, tostr(ss) );
                    }
                }
            
                create_bitf_to_intvec_function_and_bitf_rdtree_index( trans );
            }

            cerr << "running analyze..." << endl;
            Execute( trans, "analyze");
            trans.commit();
            CreateIndex_saveBitfColumns();

            
//             {
//                 nontransaction trans( dbmaker, "vacuum" );
//                 Execute( trans, "vacuum");
//             }
        }
        

        void
        EAIndexerPostgresql::prepareForWrites( int f )
        {
            LG_EAIDX_D << "prepareForWrites()" << endl;
            if( f & PREPARE_FOR_WRITES_ISNEWER_TESTS &&
                m_isFileNewerThanIndexedVersionCache.empty() )
            {
                setupIsFileNewerThanIndexedVersionCache();
            }
        }

        void
        EAIndexerPostgresql::allWritesComplete()
        {
            m_isFileNewerThanIndexedVersionCache.clear();
        }
        
            


        
        string
        EAIndexerPostgresql::asString( IndexableValue& v, AttrType_t att, bool quote )
        {
            switch( att )
            {
            case ATTRTYPEID_INT:
                LG_EAIDX_D << "asString(int) eaname:" << v.rawEANameString()
                           << " v.rawValueString:" << v.rawValueString()
                           << " str->int:" << convertStringToInteger( v.rawValueString() )
                           << " tostr(str->int):" << tostr(convertStringToInteger( v.rawValueString() ))
                           << endl;
                return tostr(convertStringToInteger( v.rawValueString() ));
            case ATTRTYPEID_DBL:
            {
                LG_EAIDX_D << "asString(double) eaname:" << v.rawEANameString()
                           << " v.rawValueString:" << v.rawValueString()
                           << endl;
                string ret = v.rawValueString();
                if( ret.empty() )
                    return "0";
                return ret;
            }
            case ATTRTYPEID_TIME:
            {
                LG_EAIDX_D << "asString(time) eaname:" << v.rawEANameString()
                           << " v.rawValueString:" << v.rawValueString()
                           << " strToInt(v.raw):" << convertStringToInteger( v.rawValueString() )
                           << endl;
                stringstream ss;
                // CHANGED: made quoting optional here
                if( quote )
                    ss << "'";
                ss << toSQLTimeString( convertStringToInteger( v.rawValueString() ));
                if( quote )
                    ss << "'";
                return ss.str();
            }
            case ATTRTYPEID_CIS:
            case ATTRTYPEID_STR:
            {
                fh_stringstream ss;
                string str;
                
                if( v.isCaseSensitive() )
                    str = v.rawValueString();
                else
                    str = tolowerstr()( v.rawValueString() );
                
                if( quote )
                    ss << quoteStr( str );
                else
                    ss << str;
                
                return tostr(ss);
            }
            }
            return v.rawValueString();
        }

        string 
        EAIndexerPostgresql::getTableName( AttrType_t att )
        {
            if( const gchar* p = g_getenv ("LIBFERRIS_EAINDEX_FORCE_TABLENAME") )
            {
                string ret = p;
                LG_EAIDX_D << "getTableName() forced to table name:" << ret << endl;
                return ret;
            }
            
            switch( att )
            {
            case ATTRTYPEID_INT:
                return "intlookup";
            case ATTRTYPEID_DBL:
                return "doublelookup";
            case ATTRTYPEID_TIME:
                return "timelookup";
            case ATTRTYPEID_STR:
            case ATTRTYPEID_CIS:
                return "strlookup";
            }
            return "New table type not defined!";
        }

        docid_t
        EAIndexerPostgresql::obtainURLID( work& trans, fh_context c, const std::string& url )
        {
            long urlid = -1;

            LG_EAIDX_D << "EAIndexerPostgresql::obtainURLID(1) " << endl;
            result res;
            res = ExecuteRes( res, trans,
                              "SELECT urlid FROM urlmap WHERE url=" + quoteStr(trans,url) );
            LG_EAIDX_D << "EAIndexerPostgresql::obtainURLID(2) " << endl;
            if( res.empty() )
            {
                Execute( trans, "INSERT INTO urlmap (url) VALUES (" + quoteStr(trans,url) + ")" );
                LG_EAIDX_D << "EAIndexerPostgresql::obtainURLID(3) " << endl;
                res = ExecuteRes( res, trans,
                                  "SELECT urlid FROM urlmap WHERE url=" + quoteStr(trans,url) );
            LG_EAIDX_D << "EAIndexerPostgresql::obtainURLID(4) " << endl;
            }
            LG_EAIDX_D << "EAIndexerPostgresql::obtainURLID(5) " << endl;
            docid_t ret;
            res[0][0].to(ret);
            return ret;
        }

        EAIndexerPostgresql::m_attrMapCache_t&
        EAIndexerPostgresql::getAttrMapCache( work& trans )
        {
//            cerr << "getAttrMapCache(top)" << endl;
            
            if( !m_attrMapCache.empty() )
                return m_attrMapCache;

            tablereader tr( trans, "attrmap" );
            vector<string> row;
            while( tr >> row )
            {
                int attrid = -1;
                int attrtype = 0;
                string attrname;
                int i=0;

                attrid   = toint( row[i] ); ++i;
                attrtype = toint( row[i] ); ++i;
                attrname = row[i];          ++i;
                
                m_attrMapCache[ make_pair( attrname, attrtype ) ] = attrid;
                row.clear();
            }

            tr.complete();
            
            return m_attrMapCache;
        }

        EAIndexerPostgresql::m_genericlookupCache_t&
        EAIndexerPostgresql::getLookupCache( work& trans,
                                             const string& tablename,
                                             m_genericlookupCache_t& lc,
                                             int& lcSize )
        {
            LG_EAIDX_D << "getStrLookupCache(top)" << endl;
            if( !lc.empty() )
                return lc;

            tablereader tr( trans, tablename );
            vector<string> row;
            while( tr >> row )
            {
                string attrvalue = "";
                int vid = 0;
                int i=0;
                vid       = toint( row[i] ); ++i;
                attrvalue = row[i]; ++i;
                
                lc[ attrvalue ] = vid;
                row.clear();
            }
            tr.complete();

            LG_EAIDX_D << "START tablename:" << tablename << " sz:" <<  lc.size() << endl;
            for( m_genericlookupCache_t::const_iterator ami = lc.begin(); ami != lc.end(); ++ami )
            {
                LG_EAIDX_D << "attrvalue:" << ami->first << endl
                           << "sqlesc(v):" << trans.esc(ami->first) << endl
                           << "      vid:" << ami->second << endl;
            }
            LG_EAIDX_D << "tablename:" << tablename << " sz:" <<  lc.size() << endl;
            
            lcSize = lc.size();
            return lc;
        }
        
            
        
        EAIndexerPostgresql::m_strlookupCache_t&
        EAIndexerPostgresql::getStrLookupCache( work& trans )
        {
            return getLookupCache( trans, "strlookup", m_strlookupCache, m_strlookupCacheSize );
        }
        
        EAIndexerPostgresql::m_timelookupCache_t&
        EAIndexerPostgresql::getTimeLookupCache( work& trans )
        {
            return getLookupCache( trans, "timelookup", m_timelookupCache, m_timelookupCacheSize );
        }

        EAIndexerPostgresql::m_intlookupCache_t&
        EAIndexerPostgresql::getIntLookupCache( work& trans )
        {
            return getLookupCache( trans, "intlookup", m_intlookupCache, m_intlookupCacheSize );
        }

        EAIndexerPostgresql::m_doublelookupCache_t&
        EAIndexerPostgresql::getDoubleLookupCache( work& trans )
        {
            return getLookupCache( trans, "doublelookup", m_doublelookupCache, m_doublelookupCacheSize );
        }


        EAIndexerPostgresql::m_isFileNewerThanIndexedVersionCache_t&
        EAIndexerPostgresql::getIsFileNewerThanIndexedVersionCache()
        {
            return m_isFileNewerThanIndexedVersionCache;
        }
        
        
        bool
        EAIndexerPostgresql::getIndexMethodSupportsIsFileNewerThanIndexedVersion()
        {
            return true;
        }
        
        bool
        EAIndexerPostgresql::isFileNewerThanIndexedVersion( const fh_context& c )
        {
            bool ret = true;

            //
            // To avoid race conditions we take the time now() before any part of the
            // isFileNewer check and use that value.
            //
            time_t tt = Time::getTime();
            time_t ct = getTimeAttr( c, "ferris-should-reindex-if-newer", 0 );
            if( !ct )
                return ret;
            LG_EAIDX_D << "isFileNewerThanIndexedVersion() cache.sz:" << getIsFileNewerThanIndexedVersionCache().size() << " ct:" << ct << endl;
//             cerr << "isFileNewerThanIndexedVersion() cache.sz:" << getIsFileNewerThanIndexedVersionCache().size() << " ct:" << ct << endl;
//             cerr << "c:" << c->getURL() << endl;

            string earl = c->getURL();

            m_isFileNewerThanIndexedVersionCache_t::const_iterator ci =
                getIsFileNewerThanIndexedVersionCache().find( earl );
            if( ci != getIsFileNewerThanIndexedVersionCache().end() )
            {
                LG_EAIDX_D << "PG::isNewer() ct:" << ct << " db-version:" << ci->second << endl;
                ret = ci->second < ct;
                if( !ret )
                {
                    m_isFileNewerThanIndexedVersionSkippedDocuments[ earl ] = tt;
                }
//                cerr << "PG::isNewer() ct:" << ct << " db-version:" << ci->second << " ret:" << ret << endl;
                return ret;
            }

            return ret;
        }


        void
        EAIndexerPostgresql::setupIsFileNewerThanIndexedVersionCache()
        {
            LG_EAIDX_D << "setupIsFileNewerThanIndexedVersionCache()" << endl;

            m_isFileNewerThanIndexedVersionSkippedDocuments.clear();
            
            work tr( *m_connection, "setup getIsFileNewerThanIndexedVersionCache" );
            stringstream sqlss;
//             sqlss << "select u.url, d.docidtime"
// //                      << EANameToSQLColumnName("ferris-should-reindex-if-newer")
//                   << " from urlmap u, currentversions d where d.urlid=u.urlid";
            sqlss << "select u.url, d.docidtime"
                  << " from urlmap u, docmap d where d.urlid=u.urlid";
            
            result res;
            res = ExecuteRes( res, tr, sqlss.str() );
                
            for (result::const_iterator c = res.begin(); c != res.end(); ++c)
            {
                string url = "";
                string t;
                time_t mtime = 0;
                    
                c[0].to(url);
                c[1].to(t);

                if( t.empty() )
                    continue;
                    
//                    cerr << "url:" << url << " t:" << t << endl;
                m_isFileNewerThanIndexedVersionCache[ url ] = fromSQLTimeString(t);
            }
        }
            
        void
        EAIndexerPostgresql::removeDocumentsMatchingRegexFromIndex( const std::string& s,
                                                                    time_t mustBeOlderThan )
        {
            docNumSet_t urlids;
            docNumSet_t docids;

            LG_EAIDX_D << "removeDocumentsMatchingRegexFromIndex() regex:" << s << endl;

            {
                work trans( *m_connection, "select docids" );
                stringstream sqlss;
                sqlss << "select d.urlid,d.docid from urlmap u, docmap d\n"
                      << " where url ~ '" << s << "' and u.urlid=d.urlid\n";
                if( mustBeOlderThan )
                {
                    sqlss << " and d.docidtime < '" << toSQLTimeString(mustBeOlderThan) << "' \n";
                }
                sqlss << ";";
                result res;
                res = ExecuteRes( res, trans, tostr( sqlss ).c_str() );
                for (result::const_iterator c = res.begin(); c != res.end(); ++c)
                {
                    long urlid = 0;
                    long docid = 0;
                    c[0].to(urlid);
                    c[1].to(docid);
                    urlids.insert( urlid );
                    docids.insert( docid );
                }
            }

            LG_EAIDX_D << "removeDocumentsMatchingRegexFromIndex()"
                       << " #urlids:" << urlids.size()
                       << " #docids:" << docids.size()
                       << endl;

            {
                // If there is a fulltext index in the same database we might
                // not be able to remove the URL from the urlmap because it is
                // referenced from ftxdocmap;
                work trans( *m_connection, "recheck urlids" );
                stringstream sqlss;
                sqlss << "select urlid from ftxdocmap where urlid in ("
                      << Util::createSeperatedList( urlids.begin(), urlids.end(), ',' )
                      << ");";
                LG_EAIDX_D << "recheck urlids SQL:" << tostr(sqlss) << endl;
                try
                {
                    result res;
                    res = ExecuteRes( res, trans, tostr( sqlss ).c_str() );
                    for (result::const_iterator c = res.begin(); c != res.end(); ++c)
                    {
                        long urlid = 0;
                        c[0].to(urlid);
                        urlids.erase( urlid );
                    }
                }
                catch( exception& e )
                {
                }
            }
            
            string urlidsCommaSep = Util::createSeperatedList( urlids.begin(), urlids.end(), ',' );
            string docidsCommaSep = Util::createSeperatedList( docids.begin(), docids.end(), ',' );
            {
                work trans( *m_connection, "removing document metadata..." );

                if( !docids.empty() )
                {
                    stringstream sqlss;
                    sqlss << "delete from docattrs where docid in ( " << docidsCommaSep << " );";
                    LG_EAIDX_D << "docattrs remove sql:" << tostr(sqlss) << endl;
                    Execute( trans, tostr(sqlss) );
                }
                if( !docids.empty() )
                {
                    stringstream sqlss;
                    sqlss << "delete from docmap where docid in ( " << docidsCommaSep << " );";
                    LG_EAIDX_D << "docmap remove sql:" << tostr(sqlss) << endl;
                    Execute( trans, tostr(sqlss) );
                }
                if( !urlids.empty() )
                {
                    stringstream sqlss;
                    sqlss << "delete from urlmap where urlid in ( " << urlidsCommaSep << " );";
                    LG_EAIDX_D << "urlmap remove sql:" << tostr(sqlss) << endl;
                    Execute( trans, tostr(sqlss) );
                }
                trans.commit();
            }
            LG_EAIDX_D << "removeDocumentsMatchingRegexFromIndex(done.) regex:" << s << endl;
        }
        
        void
        EAIndexerPostgresql::addToIndex( fh_context c, fh_docindexer di )
        {
//            cerr << "EAIndexerPostgresql::addToIndex(top) c:" << c->getURL() << endl;
            LG_EAIDX_I << "EAIndexerPostgresql::addToIndex(top)" << endl;
            
            bool hadError = false;
            string earl   = c->getURL();
            
            LG_EAIDX_D << "EAIndexerPostgresql::addToIndex(1) earl:" << earl << endl;
            
            int attributesDone = 0;
            int signalWindow   = 0;
            stringlist_t slist;
            getEANamesToIndex( c, slist );
            int totalAttributes = slist.size();

            LG_EAIDX_D << "EAIndexerPostgresql::addToIndex(2) earl:" << earl << endl;
            
            Time::Benchmark bm( "earl:" + earl );
            bm.start();

//            work& trans = *m_trans;
            work trans( *m_connection, "adding to index" );
            
            LG_EAIDX_D << "EAIndexerPostgresql::addToIndex() earl:" << earl << endl;
            long docid  = -1;
            long urlid  = obtainURLID( trans, c, earl );
            LG_EAIDX_D << "EAIndexerPostgresql::addToIndex() docid:" << docid
                       << endl;

            typedef map< string, IndexableValue > ivs_t;
            ivs_t ivs;

            for( stringlist_t::iterator si = slist.begin(); si != slist.end(); ++si )
            {
                try
                {
                    string attributeName = *si;
                    string k = attributeName;
                    string v = "";
#ifdef DEBUG_ADDING_TO_INDEX
                    // PURE DEBUG
                    stringstream bmss;
                    bmss << " +++attr+++ addContextToIndex(a) c:" << c->getURL()
                         << " k:" << k << " v:" << v << " --- ";
                    Time::Benchmark bm( tostr(bmss) );
                    FERRIS_LG_BENCHMARK_D( bm, Ferris::Logging::LG_EAIDX::Instance() );
#endif             
                    
                    if( !obtainValueIfShouldIndex( c, di, attributeName, v ))
                        continue;

                    if( k == "subtitles"
                        && m_ExtraColumnsToInlineInDocmap.find( k ) == m_ExtraColumnsToInlineInDocmap.end())
                    {
                        continue;
                    }
                    
                    
//                    cerr << "  addToIndex(EA) k:" << k << endl;
                    LG_EAIDX_D << "EAIndexerPostgresql::addToIndex() attributeName:" << attributeName << endl;
                    LG_EAIDX_I << "addContextToIndex(a) c:" << c->getURL() << endl
                               << " k:" << k << " v:" << v << endl;

//                     if( k=="width" )
//                     {
//                         LG_EAIDX_D << "WIDTH:" << v << endl;
//                     }
                    
                    IndexableValue iv  = getIndexableValue( c, k, v );
                    ivs.insert( make_pair( attributeName, iv ) );


                    if( LG_EAIDX_D_ACTIVE )
                    {
                        XSDBasic_t sct = iv.getSchemaType();
                        AttrType_t att = iv.getAttrTypeID();

                        LG_EAIDX_D << "(reading attrs to cache) attributeName:" << k
                                   << " att:" << att
                                   << " sct:" << sct
                                   << " v:" << v 
                                   << endl;
                    }
                }
                catch( exception& e )
                {
                    LG_EAIDX_D << "EXCEPTION e:" << e.what() << endl;
                }
            }


            //
            // Allow new columns to be indexed explicitly while leaving other data intact
            //
            if( const gchar* p = g_getenv ("LIBFERRIS_UPDATE_COLUMN_LIST") )
            {
                LG_EAIDX_D << "updaing only explicitly selected columns:" << p << endl;
                
                stringset_t cols;
                Util::parseCommaSeperatedList( p, cols );
                for( stringset_t::iterator si = cols.begin(); si != cols.end(); ++si )
                {
                    string eaname = *si;
                    string columnName = EANameToSQLColumnName( eaname );
                    typedef map< string, IndexableValue > ivs_t;
                    ivs_t::iterator ivs_iter = ivs.find( eaname );

                    if( ivs_iter != ivs.end() )
                    {
                        IndexableValue& iv = ivs_iter->second;
                        string value = asString( iv );
                        
                        fh_stringstream sqlss;
                        sqlss << "update docmap "
                              << " set " << columnName << " = " << value
                              << " where urlid = " << urlid;
                        Execute( trans, tostr(sqlss) );
                    }
                }
                trans.commit();
                return;
            }
            
            //
            // Add the EA which are ment to be indexed in the docmap table
            //
            {
                LG_EAIDX_D << "Updating docmap table..." << endl;
                fh_stringstream insertSQL;
                insertSQL << "INSERT INTO docmap "
                          << " ( urlid, docid, docidtime ";
                for( stringmap_t::const_iterator si = m_ExtraColumnsToInlineInDocmap.begin();
                     si != m_ExtraColumnsToInlineInDocmap.end(); ++si )
                {
                    string attributeName = si->first;
                    string colname = EANameToSQLColumnName( attributeName );
                    insertSQL << " ," << "\"" << colname << "\"";
                }
                insertSQL << "  ,bitf )"
                          << " VALUES ("
                          << urlid << ","
                          << "DEFAULT" << ","
                          << "'" << toSQLTimeString( Time::getTime() ) << "'";
                
                for( stringmap_t::const_iterator si = m_ExtraColumnsToInlineInDocmap.begin();
                     si != m_ExtraColumnsToInlineInDocmap.end(); ++si )
                {
                    string attributeName = si->first;
                    string colname = EANameToSQLColumnName( attributeName );
                    string coltype = si->second;
                    LG_EAIDX_D << "attributeName1:" << attributeName << endl;
                    insertSQL << ",";
                    
                    ivs_t::iterator ivi = ivs.find( attributeName );
                    if( ivi == ivs.end() )
                    {
                        insertSQL << "DEFAULT";
                    }
                    else
                    {
                        IndexableValue&   iv = ivi->second;
//                        const std::string& k = iv.rawEANameString();
                        XSDBasic_t       sct = iv.getSchemaType();

                        if( coltype == "timestamp" ) 
                        {
                            insertSQL << asString( iv, ATTRTYPEID_TIME );
                        }
                        else
                        {
                            insertSQL << asString( iv );
                        }
                        
                        LG_EAIDX_D << "adding attributeName:" << attributeName
                                   << " value:" << asString( iv )
                                   << " coltype:" << coltype
                                   << endl;
                        ivs.erase( ivi );
                    }
                }

//                cerr << "getBitFLength:" << getBitFLength() << endl;
                if( m_bitfColumns.empty() )
                {
                    insertSQL << ",default";
                }
                else
                {
                    int bitflen = getBitFLength();
                    bool virgin = true;
                    
                    insertSQL << ",(";
                    for( m_bitfColumns_t::const_iterator bi = m_bitfColumns.begin();
                         bi != m_bitfColumns.end(); ++bi )
                    {
                        string attributeName = bi->first;
                        int    ShiftCount    = bi->second;

                        ivs_t::iterator ivi = ivs.find( attributeName );
                        if( ivi != ivs.end() )
                        {
                            IndexableValue& iv = ivi->second;
                            string           v = iv.rawValueString();

                            LG_EAIDX_D << "bitf col:" << attributeName << " value:" << v << endl;
                            
                            if( !v.empty() && v != "0" && v != "f" )
                            {
                                if( virgin )  virgin = false;
                                else          insertSQL << "|";
                                insertSQL << "(B'1'::bit(" << bitflen << ") >> " << ShiftCount << ")";
                            }
                            ivs.erase( ivi );
                        }
                    }
                    if( virgin )
                        insertSQL << "B'0'";
                    insertSQL << ")";
                }
                
                insertSQL << " ); " << endl;
                try
                {
                    result res;
                    ExecuteRes( res, trans, tostr(insertSQL) );
                }
                catch( exception& e )
                {
                    LG_EAIDX_D << "EXCEPTION e:" << e.what() << endl;
                    hadError = true;
                }

                try
                {
                    fh_stringstream ss;
                    ss << "SELECT max(docid) from docmap where"
                       << " urlid=" << urlid << ";" << endl;
                    result res;
                    ExecuteRes( res, trans, tostr(ss) );
                    res[0][0].to( docid );
                }
                catch( exception& e )
                {
                    LG_EAIDX_D << "EXCEPTION e:" << e.what() << endl;
                    hadError = true;
                }
            }
            LG_EAIDX_D << "Completed docmap table..." << endl;
            
            BulkRows_t timeBulkRows;
            BulkRows_t intBulkRows;
            BulkRows_t doubleBulkRows;
            BulkRows_t strBulkRows;
            
            

            //
            // Get attrmap name -> aid map
            // Declare some bulk insert data caches.
            //
            LG_EAIDX_D << "addToIndex() got docid:" << docid << endl;
            m_attrMapCache_t& amc = getAttrMapCache( trans );

            m_strlookupCache_t& strlc = getStrLookupCache( trans );
            BulkRows_t docattrsBulkRows;
            typedef list< pair< IndexableValue*, long > > StrAttrValues_t;
            StrAttrValues_t StrAttrValues;
            typedef list< pair< IndexableValue*, long > > IntAttrValues_t;
            IntAttrValues_t IntAttrValues;
            typedef list< pair< IndexableValue*, long > > DoubleAttrValues_t;
            DoubleAttrValues_t DoubleAttrValues;
            typedef list< pair< IndexableValue*, long > > TimeAttrValues_t;
            TimeAttrValues_t TimeAttrValues;
            
            //
            // Add the normalized EA values into the lookup, docattrs, attrmap tables.
            //
            // We cache the attribute map on the client side and put four tables
            // onto the server using bulk copy, then call a stored procudure to
            // move those four tables into the database for us.
            // 
            for( ivs_t::iterator ivi = ivs.begin(); ivi!=ivs.end(); ++ivi )
            {
                IndexableValue& iv = ivi->second;
                const std::string& k = iv.rawEANameString();
                LG_EAIDX_D << "attributeName:" << k << endl;
                string             v = iv.rawValueString();

                XSDBasic_t sct = iv.getSchemaType();
                AttrType_t att = iv.getAttrTypeID();
                if( att == ATTRTYPEID_CIS )
                    att = ATTRTYPEID_STR;
                if( sct == FXD_UNIXEPOCH_T )
                    att = ATTRTYPEID_TIME;

                LG_EAIDX_D << "attributeName:" << k
                           << " att:" << att
                           << " sct:" << sct
                           << " v:" << v 
                           << endl;
                
                //
                // Get the attrid for this attribute.
                //
                long aid = -1;
                m_attrMapCache_t::iterator ami = amc.find( make_pair( k, att ) );
                if( ami != amc.end() )
                {
                    aid = ami->second;
                }
                else
                {
                    fh_stringstream ss;
                    
                    ss << "INSERT INTO attrmap (attrtype,attrname) VALUES ("
                       << att << "," << quoteStr( trans, k ) << " );";
                    Execute( trans, tostr(ss) );
                    
                    {
                        fh_stringstream ss;
                        ss << "SELECT attrid from attrmap where"
                           << " attrtype=" << att
                           << " and attrname=" << quoteStr(trans, k) << ";" << endl;
                        result res;
                        ExecuteRes( res, trans, tostr(ss) );
                        res[0][0].to( aid );
                    }
                    
                    amc.insert( make_pair( make_pair( k, att ), aid ) );
//                    LG_EAIDX_D << "Added attribute, k:" << k << " aid:" << aid << endl;
                }

                switch( att )
                {
                case ATTRTYPEID_TIME:
                    TimeAttrValues.push_back( make_pair( &iv, aid ) );
                    break;
                case ATTRTYPEID_INT:
//                     LG_EAIDX_D << "Adding int k:" << k << " v:" << v 
//                                << endl;
                    IntAttrValues.push_back( make_pair( &iv, aid ) );
                    break;
                case ATTRTYPEID_DBL:
//                     LG_EAIDX_D << "Adding double k:" << k << " v:" << v 
//                                << endl;
                    DoubleAttrValues.push_back( make_pair( &iv, aid ) );
                    break;
                case ATTRTYPEID_CIS:
                case ATTRTYPEID_STR:
                    StrAttrValues.push_back( make_pair( &iv, aid ) );
                    break;
                }
            }
            
            {
                UpdateLookupTable<time_t>(
                    trans, "timelookup",
                    TimeAttrValues,
                    getTimeLookupCache( trans ), m_timelookupCacheSize,
                    docid, docattrsBulkRows );

                UpdateLookupTable<int>(
                    trans, "intlookup",
                    IntAttrValues,
                    getIntLookupCache( trans ), m_intlookupCacheSize,
                    docid, docattrsBulkRows );

                UpdateLookupTable<double>(
                    trans, "doublelookup",
                    DoubleAttrValues,
                    getDoubleLookupCache( trans ), m_doublelookupCacheSize,
                    docid, docattrsBulkRows );
                
                {
                    LG_EAIDX_D << "strBulkRows.size:" << strBulkRows.size() << endl;

//                    template< class Collection_t, class Value_t, class LookupCache_t >
                    UpdateLookupTable<const string&>(
                        trans, "strlookup",
                        StrAttrValues,
                        getStrLookupCache( trans ), m_strlookupCacheSize,
                        docid, docattrsBulkRows );
                }
            }

            LG_EAIDX_D << "----------- STARTING BULK DOCATTRS WRITE ------------------------ " << endl;
            tablewriter w( trans, "docattrs" );
            for( BulkRows_t::const_iterator ci = docattrsBulkRows.begin(); ci != docattrsBulkRows.end(); ++ci )
            {
//                 LG_EAIDX_D << " ROW --- START ---" << endl;
//                 for( vector< string >::const_iterator iter = (*ci).begin(); iter != ci->end(); ++iter )
//                     LG_EAIDX_D << " battr:" << *iter << endl;
                
                w << *ci;
            }
            LG_EAIDX_D << "----------- CALLING COMPLETE DOCATTRS WRITE ------------------------ " << endl;
            w.complete();
            LG_EAIDX_D << "----------- COMPLETED BULK DOCATTRS WRITE ------------------------ " << endl;

//          Execute( trans, "select xlookup_consume( " + tostr(docid) + " );" );
            
            if( hadError )
            {
                LG_EAIDX_W << "addToIndex() had an error with file:" << c->getURL() << endl;
                cerr << "addToIndex() had an error with file:" << c->getURL() << endl;
            }
            else
            {
                LG_EAIDX_D << "addToIndex() commiting transaction" << endl;
                trans.commit();
            }

            incrFilesIndexedCount();
            m_filesIndexedSinceAnalyseCount++;
        }

        string
        EAIndexerPostgresql::AddSQLOp_ferrisopcode_to_sqlopcode( const std::string& s, IndexableValue& iv )
        {
            string opcode = s;
            if( opcode == "==" )
                opcode = "=";
            if( opcode == "=~" )
            {
                opcode = "~";
                if( !iv.isCaseSensitive() )
                    opcode = "~*";
            }
            return opcode;
        }

        
        void
            EAIndexerPostgresql::AddSQLOp( fh_stringstream& sqlss,
                                           const std::string& eaname_,
                                           const std::string& opcode_const,
                                           IndexableValue& iv,
                                           AttrType_t att,
                                           stringset_t& lookupTablesUsed,
                                           const std::string termMergeSQLOpCode,
                                           const ctxlist_t& extraTerms )
        {
            string opcode = AddSQLOp_ferrisopcode_to_sqlopcode( opcode_const, iv );
            string eaname = eaname_;

//            cerr << "EAIndexerPostgresql::AddSQLOp(1) eaname:" << eaname << endl;
            if( eaname == "ferris-current-time" )
                eaname = "docidtime";
//            cerr << "EAIndexerPostgresql::AddSQLOp(2) eaname:" << eaname << endl;
            
            
            string SQLDistinctClauseStart = " ";
            string SQLDistinctClauseEnd   = " ";
//             string SQLDistinctClauseStart = " distinct(";
//             string SQLDistinctClauseEnd   = ") ";
            
            if( const gchar* p = g_getenv ("LIBFERRIS_EAINDEX_FORCE_ATTR_TYPE") )
            {
                string type = p;

                if( type == "string" || type=="str" )
                    att = ATTRTYPEID_CIS;
            }
            if( opcode == "~" )
            {
                att = ATTRTYPEID_CIS;
            }
            
            string lookupTableName = getTableName( att );
            lookupTablesUsed.insert( lookupTableName );
            

//             if( opcode == "~" )
//             {
//                 att = ATTRTYPEID_STR;
//             }
        
            string caseSenPrefix = " ";
            string caseSenPostfix = " ";
            LG_EAIDX_D << "AddSQLOp() att:" << att << " isCaseSen:" << iv.isCaseSensitive()
                       << " val:" << asString( iv, att )
                       << endl;
//            if( !iv.isCaseSensitive() ) // && opcode != "~" )
            if( !iv.isCaseSensitive() && opcode != "~*" )
            {
                caseSenPrefix = " lower( ";
                caseSenPostfix = " ) ";
            }

            // allow lookup by docid
            m_ExtraColumnsToInlineInDocmap["docid"] = "int";
            stringmap_t::const_iterator eci
                = m_ExtraColumnsToInlineInDocmap.find( eaname );
            m_bitfColumns_t::const_iterator bfci = m_bitfColumns.find( eaname );

            LG_EAIDX_D << "EAIndexerPostgresql::AddSQLOp() "
                       << " eaname:" << eaname
                       << " is-case-sen:" << iv.isCaseSensitive()
                       << " v:" << iv.rawValueString()
                       << " v.asstring:" << asString( iv, att )
                       << endl;
            
            if( eaname == "url" || eaname == "name" )
            {
                att = ATTRTYPEID_STR;
                std::string q = asString( iv, att, true );
                std::string isCaseSen = "false";
                if( iv.isCaseSensitive() )
                    isCaseSen = "true";

                LG_EAIDX_D << "EAIndexerPostgresql::AddSQLOp() "
                           << " eaname:" << eaname
                           << " opcode:" << opcode
                           << endl;
                LG_EAIDX_D << "EAIndexerPostgresql::AddSQLOp() "
                           << " q:" << q
                           << " isCaseSen:" << isCaseSen
                           << " limit:" << getLimit()
                           << endl;
                
                sqlss << "d.docid in" << endl
                      << "   (" << endl
                      << "    SELECT docid from docmap  " << endl
                      << "    WHERE  urlid in ( " << endl
                      << "      select urlmatch( " << q << ", '" << eaname << "', " << isCaseSen << " ) " << endl
                      << "      )" << endl;
                sqlss << "   )" << endl
                      << endl;
                
                
                // if( eaname == "name"
                //     && regexHasNoSpecialChars( iv.rawValueString() )
                //     && ( opcode == "~*" || opcode == "~" ) )
                // {
                //     if( !iv.isCaseSensitive() )
                //     {
                //         caseSenPrefix = " lower( ";
                //         caseSenPostfix = " ) ";
                //     }
                //     LG_EAIDX_D << "EAIndexerPostgresql::AddSQLOp() fast filename search!" << endl;
                    
                    
                //     // hit the fnshift table for speed.
                //     sqlss << "d.docid in" << nl
                //           << "   (" << nl
                //           << "    SELECT docid from docmap  " << nl
                //           << "    WHERE  urlid in ( " << nl
                //           << "                SELECT urlid from fnshift " << nl
                //           << "                WHERE " << caseSenPrefix << " fn " << caseSenPostfix << nl
                //           << "                            like " << nl
                //           << caseSenPrefix << "'" << asString( iv, att, false ) << "%'" << caseSenPostfix << nl
                //           << "                     ) " << nl;
                //     sqlss << "   )"
                //           << endl;
                // }
                // else
                // {
                //     sqlss << "d.docid in" << nl
                //           << "   ("
                //           << "    SELECT "
                //           << SQLDistinctClauseStart << " docid " << SQLDistinctClauseEnd
                //           << " as docid " << nl
                //           << "    FROM docmap, urlmap " << nl
                //           << "    WHERE ";
                //     sqlss << "           urlmap.urlid = docmap.urlid " << nl
                //           << "    AND " << caseSenPrefix << " urlmap.url " << caseSenPostfix;
                //     if( eaname == "url" )
                //         sqlss << opcode << caseSenPrefix << asString( iv, att ) << caseSenPostfix << nl;
                //     else
                //     {
                //         sqlss << opcode << caseSenPrefix
                //               << "'.*/" << asString( iv, att, false ) << "$'"
                //               << caseSenPostfix << nl;
                //     }
                //     sqlss << "   )"
                //           << endl;
                // }
            }
            else if( eaname == "docidtime" )
            {
                string colname = eaname;

                sqlss << caseSenPrefix << "d.\"" << colname << "\"" << caseSenPostfix << " "
                      << opcode 
                      << caseSenPrefix << asString( iv, att ) << caseSenPostfix;
            }
            else if( eci != m_ExtraColumnsToInlineInDocmap.end() )
            {
                string colname = EANameToSQLColumnName( eci->first );
                static fh_rex e = toregexhi("[0-9a-z ]+");
                if(m_DocmapColumnsToFulltextIndex.count(EANameToSQLColumnName(colname))
                   && regex_match( asString( iv, att, false ), e ))
                {
                    sqlss << "to_tsvector('english',"
                          << "d.\"" << colname << "\") "
                          << " @@ plainto_tsquery(" << asString( iv, att ) <<  ") ";
                }
                else
                {
                    sqlss << caseSenPrefix << "d.\"" << colname << "\"" << caseSenPostfix << " "
                          << opcode 
                          << caseSenPrefix << asString( iv, att ) << caseSenPostfix;
                }
            }
//             else if( starts_with( eaname, "emblem:id-" ) )
//             {
//                 string idstr = eaname.substr( strlen("emblem:id-"));
//                 emblemID_t eid = toType<emblemID_t>( idstr );
//                 fh_etagere et = Factory::getEtagere();
//                 fh_emblem em = et->getEmblemByID( eid );
                
//             }
            else if( bfci != m_bitfColumns.end() )
            {
                int    bitflen    = getBitFLength();
                string eaname     = bfci->first;
                int    ShiftCount = bfci->second;

                //insertSQL << "(B'1'::bit(" << bitflen << ") >> " << ShiftCount << ")";

                if( useGIST() )
                {
                    m_bitfColumns_t::const_iterator iter = m_bitfColumns.end();
                    iter = m_bitfColumns.find( eaname );

                    if( iter != m_bitfColumns.end() )
                    {
                        int    ShiftCount    = iter->second;
                        sqlss << "( docmap_bitf_to_intvec(bitf) @ '{" << ShiftCount << "}' )";
                    }
                    else
                    {
                        sqlss << "( " << getBitFunctionName( eaname ) << "(d.bitf) )";
                    }
                }
                else
                {
                    sqlss << "( " << getBitFunctionName( eaname ) << "(d.bitf) )";
                }
                
//                 sqlss << "(d.bitf::bit(" << bitflen << ") " 
//                       << " & "
//                       << "(B'1'::bit(" << bitflen << ") >> " << ShiftCount << ")"
//                       << " = "
//                       << "(B'1'::bit(" << bitflen << ") >> " << ShiftCount << "))";
            }
            else
            {
                sqlss << "d.docid in" << nl
                      << "   ("
                      << "    SELECT " << SQLDistinctClauseStart << " docattrs.docid " << SQLDistinctClauseEnd
                      << " as docid " << nl
                      << "      FROM docattrs, attrmap , " << lookupTableName << nl
                      << "      WHERE " << nl
                      << "      (( "   << nl;
                if( !extraTerms.empty() )
                    sqlss << " ( " << nl;
                sqlss << "         " << caseSenPrefix << lookupTableName << ".attrvalue " << caseSenPostfix
                      << opcode << " "
                      << "         " << caseSenPrefix << asString( iv, att ) << caseSenPostfix << " "    << nl;
                if( !extraTerms.empty() )
                {
                    for( ctxlist_t::const_iterator ci = extraTerms.begin(); ci != extraTerms.end(); ++ci )
                    {
                        pair< fh_context, fh_context > lcrc_pair = BuildQuery_getLeftAndRightContexts( *ci );
                        fh_context lc = lcrc_pair.first;
                        fh_context rc = lcrc_pair.second;
                        IndexableValue iv = getIndexableValueFromToken( eaname, rc );
                        string opcode = AddSQLOp_ferrisopcode_to_sqlopcode( BuildQuery_getToken( *ci ), iv );

                        string caseSenPrefix = " ";
                        string caseSenPostfix = " ";
                        if( !iv.isCaseSensitive() && opcode != "~*" )
                        {
                            caseSenPrefix = " lower( ";
                            caseSenPostfix = " ) ";
                        }
                        sqlss << " " << termMergeSQLOpCode << " ";
                        sqlss << "         " << caseSenPrefix << lookupTableName << ".attrvalue " << caseSenPostfix
                              << opcode << " "
                              << "         " << caseSenPrefix << asString( iv, att ) << caseSenPostfix << " "    << nl;
                    }
                    sqlss << " ) " << nl;
                }
                sqlss << "       and " << lookupTableName << ".vid = docattrs.vid " << nl
                      << "       and " << " attrmap.attrname=\'" << eaname << "\' " << nl;
                
                if( att == ATTRTYPEID_CIS )
                {
                    sqlss << "       and ( " << " attrmap.attrtype=\'" << att << "\' " << nl
                          << "             or  attrmap.attrtype=\'" << ATTRTYPEID_STR << "\' ) " << nl;
                }
                else
                {
                    sqlss << "       and " << " attrmap.attrtype=\'" << att << "\' " << nl;
                }
                sqlss << "       and " << " attrmap.attrid=docattrs.attrid "        << nl
                      << "      ))"                                                 << nl
                      << "   )"
                      << endl;
            }
        }

        MetaEAIndexerInterface::AttrType_t
        EAIndexerPostgresql::SQLColumnTypeToAttrType( const std::string& coltype,
                                                      IndexableValue& iv )
        {
            AttrType_t att = ATTRTYPEID_CIS;
            if( starts_with( coltype, "int" ))
                att = ATTRTYPEID_INT;
            if( starts_with( coltype, "double" ) || starts_with( coltype, "float" ) )
                att = ATTRTYPEID_DBL;
            if( starts_with( coltype, "timestamp" ))
                att = ATTRTYPEID_TIME;
            if( iv.isCaseSensitive() )
                att = ATTRTYPEID_STR;
            
            return att;
        }
        
        
        void
        EAIndexerPostgresql::AddSQLOpHeur( fh_stringstream& sqlss,
                                           const std::string& eaname,
                                           const std::string& opcode,
                                           IndexableValue& iv,
                                           stringset_t& lookupTablesUsed )
        {
            // FIXME: Heuristic lookup for columns inlined into docmap should
            // only cast to the type of that column,

            stringmap_t::const_iterator eci
                = m_ExtraColumnsToInlineInDocmap.find( eaname );

            if( eci != m_ExtraColumnsToInlineInDocmap.end() )
            {
                string colname = ( eci->first );
                string coltype = eci->second;
                AttrType_t att = SQLColumnTypeToAttrType( coltype, iv );
                AddSQLOp( sqlss, eaname, opcode, iv, att, lookupTablesUsed );
                return;
            }
            
            AttrTypeList_t atl = getAllAttrTypes();
            
            bool v = true;

            sqlss << "d.docid in (" << nl
                  << " SELECT distinct(docattrs.docid) as docid FROM docattrs "
                  << " WHERE  " << nl;

            for( AttrTypeList_t::const_iterator attrTypeIter = atl.begin();
                 attrTypeIter!=atl.end(); ++attrTypeIter )
            {
//                 if( *attrTypeIter == ATTRTYPEID_STR )
//                     continue;
                
                if( v ) v = false;
                else    sqlss << " " << nl
//                              << " OR d.docid in " << nl
                              << " OR  " << nl
                              << " ";
                AddSQLOp( sqlss, eaname, opcode, iv, *attrTypeIter, lookupTablesUsed );
            }
            sqlss << ")" << endl;
        }

        string
        EAIndexerPostgresql::BuildQuery_getToken( fh_context q )
        {
            string token   = getStrAttr( q, "token", "" );
            string tokenfc = foldcase( token );
            return tokenfc;
        }

        pair< fh_context, fh_context >
        EAIndexerPostgresql::BuildQuery_getLeftAndRightContexts( fh_context q )
        {
            fh_attribute orderedtla = q->getAttribute("in-order-insert-list");
            fh_istream   orderedtls = orderedtla->getIStream();
            
//             LG_EAIDX_D << "BuildQuery() token:" << tokenfc << endl;
//             LG_EAIDX_D << " q.child count:" << q->SubContextCount() << endl;

            string s;
            getline( orderedtls, s );
            LG_EAIDX_D << "BuildQuery() lc:" << s << endl;
            fh_context lc = q->getSubContext( s );

            fh_context rc = 0;
            if( getline( orderedtls, s ) && !s.empty() )
            {
                LG_EAIDX_D << "EAQuery_Heur::BuildQuery() rc:" << s << endl;
                rc = q->getSubContext( s );
            }
            
            return make_pair( lc, rc );
        }

        string 
        EAIndexerPostgresql::BuildQuery_getEAName( fh_context q )
        {
            pair< fh_context, fh_context > p = BuildQuery_getLeftAndRightContexts( q );
            fh_context c = p.first;
            string ret   = getStrAttr( c, "token", "" );
            return ret;
        }
        
        docNumSet_t&
            EAIndexerPostgresql::BuildQuery_LogicalCombine(
                const std::string& termMergeSQLOpCode,
                fh_context q,
                docNumSet_t& output,
                fh_eaquery qobj,
                fh_stringstream& sqlss,
                stringset_t& lookupTablesUsed,
                bool& queryHasTimeRestriction,
                stringset_t& eanamesUsed,
                MetaEAIndexerInterface::BuildQuerySQLTermInfo_t& termInfo )
        {
            LG_EAIDX_D << " operator:" << termMergeSQLOpCode << ", child count:" << q->SubContextCount() << endl;

            sqlss << " (" << nl;
            typedef list< fh_context > ctxlist_t;
            ctxlist_t ctxlist;
            for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                ctxlist.push_front( *ci );

            typedef map< string, ctxlist_t > SameAttrOps_t;
            SameAttrOps_t SameAttrOps;
            stringset_t TokensToCombine;
            TokensToCombine.insert( "<=" );
            TokensToCombine.insert( ">=" );
            TokensToCombine.insert( "=~" );
            ctxlist_t InlineEmblemClauses;
            
            //
            // Count up all the terms which are using the same EA and operations which can
            // be merged into one subquery.
            //
            for( ctxlist_t::iterator ci = ctxlist.begin(); ci != ctxlist.end(); ++ci )
            {
                string token = BuildQuery_getToken( *ci );

                // gather up all the emblem names which are stored in the bitf
                // for a single GIST query clause.
                // eg: (|(emblem:has-new18==1)(emblem:has-new17==1))
                if( useGIST() && token == "==" ) 
                {
                    string attr = BuildQuery_getEAName( *ci );
                    if( (starts_with( attr, "emblem:has-" ) || starts_with( attr, "emblem:id-" ))
                        && !contains( attr, "-fuzzy-" ) && !ends_with( attr, "-mtime" ) )
                    {
                        m_bitfColumns_t::const_iterator e    = m_bitfColumns.end();
                        m_bitfColumns_t::const_iterator iter = e;
                        iter = m_bitfColumns.find( attr );
                        if( iter != e )
                        {
                            InlineEmblemClauses.push_back( *ci );
                            continue;
                        }
                    }
                }

                //
                // Standard eaname >= 1 && eaname <= 5 handling
                //
                if( TokensToCombine.find( token ) != TokensToCombine.end() )
                {
                    string attr  = BuildQuery_getEAName( *ci );
                    stringmap_t::const_iterator eci
                        = m_ExtraColumnsToInlineInDocmap.find( attr );
                    if( attr == "url"
                        || attr == "docidtime"
                        || attr == "ferris-current-time"
                        || eci != m_ExtraColumnsToInlineInDocmap.end() )
                    {
                        continue;
                    }
                    
                        
                    SameAttrOps[ attr ].push_back( *ci );
                }

            }

            //
            // Handle the single GIST clause for many emblems.
            //
            if( !InlineEmblemClauses.empty() )
            {
                sqlss << "( docmap_bitf_to_intvec(bitf) @ '{";
                Util::SingleShot v;
                m_bitfColumns_t::const_iterator iter = m_bitfColumns.end();
                
                for( ctxlist_t::iterator ci = InlineEmblemClauses.begin(); ci != InlineEmblemClauses.end(); ++ci )
                {
                    string attr  = BuildQuery_getEAName( *ci );

                    iter = m_bitfColumns.find( attr );

                    int    ShiftCount    = iter->second;
                    if( !v() ) sqlss << ",";
                    sqlss << ShiftCount;
                
                    ctxlist.erase( find( ctxlist.begin(), ctxlist.end(), *ci ) );
                }
                sqlss << "}' )";
            }
            
            //
            // Merge all terms using the same EA which we can into single terms
            //
            bool v = true;
            for( SameAttrOps_t::iterator sai = SameAttrOps.begin();
                 sai != SameAttrOps.end(); ++sai )
            {
                if( sai->second.size() > 1 )
                {
                    string eaname = sai->first;
                    ctxlist_t& cl = sai->second;
                    eanamesUsed.insert( eaname );

                    fh_context c = cl.front();
                    cl.pop_front();
                    ctxlist.erase( find( ctxlist.begin(), ctxlist.end(), c ) );

                    pair< fh_context, fh_context > lcrc_pair = BuildQuery_getLeftAndRightContexts( c );
                    fh_context lc = lcrc_pair.first;
                    fh_context rc = lcrc_pair.second;
                    string tokenfc = BuildQuery_getToken( c );

                    IndexableValue iv = getIndexableValueFromToken( eaname, rc );
                    AddSQLOp( sqlss, eaname, tokenfc, iv, iv.getAttrTypeID(),
                              lookupTablesUsed, termMergeSQLOpCode, cl );
                    v = false;
                        
                    for( ctxlist_t::iterator ci = cl.begin(); ci != cl.end(); ++ci )
                    {
                        ctxlist.erase( find( ctxlist.begin(), ctxlist.end(), *ci ) );
                    }
                }
            }
                
            // Handle remaining sub terms.
            for( ctxlist_t::iterator ci = ctxlist.begin(); ci != ctxlist.end(); ++ci )
            {
                if( v ) v = false;
                else    sqlss << termMergeSQLOpCode << nl;

                BuildQuery( *ci, output, qobj, sqlss,
                            lookupTablesUsed, queryHasTimeRestriction,
                            eanamesUsed, termInfo );
            }
            sqlss << " ) " << nl;

            return output;
        }
             

        
        docNumSet_t&
        EAIndexerPostgresql::BuildQuery(
            fh_context q,
            docNumSet_t& output,
            fh_eaquery qobj,
            fh_stringstream& sqlss,
            stringset_t& lookupTablesUsed,
            bool& queryHasTimeRestriction,
            stringset_t& eanamesUsed,
            MetaEAIndexerInterface::BuildQuerySQLTermInfo_t& termInfo )
        {
//             string token   = getStrAttr( q, "token", "" );
//             string tokenfc = foldcase( token );
            string tokenfc = BuildQuery_getToken( q );
//             fh_attribute orderedtla = q->getAttribute("in-order-insert-list");
//             fh_istream   orderedtls = orderedtla->getIStream();
            
//             LG_EAIDX_D << "BuildQuery() token:" << tokenfc << endl;
//             LG_EAIDX_D << " q.child count:" << q->SubContextCount() << endl;

//             string s;
//             getline( orderedtls, s );
//             LG_EAIDX_D << "BuildQuery() lc:" << s << endl;
//             fh_context lc = q->getSubContext( s );

            pair< fh_context, fh_context > lcrc_pair = BuildQuery_getLeftAndRightContexts( q );
            fh_context lc = lcrc_pair.first;
            fh_context rc = lcrc_pair.second;
            
            if( tokenfc == "!" )
            {
                sqlss << "d.docid in" << nl
                      << "( " << nl
                      << "  SELECT distinct(docattrs.docid) as docid FROM docattrs "
                      << "  WHERE docid not in ";
                sqlss << "  (" << nl
                      << "    SELECT distinct(docattrs.docid) as docid FROM docattrs "
                      << "    WHERE  " << nl
                      << "     " << nl;
                BuildQuery( lc, output, qobj, sqlss,
                            lookupTablesUsed, queryHasTimeRestriction,
                            eanamesUsed, termInfo );
                sqlss << " )) " << nl;
                return output;
            }

//             getline( orderedtls, s );
//             LG_EAIDX_D << "EAQuery_Heur::BuildQuery() rc:" << s << endl;
//             fh_context rc = q->getSubContext( s );

            if( tokenfc == "&" )
            {
                string termMergeSQLOpCode = " and ";
                return BuildQuery_LogicalCombine( termMergeSQLOpCode,
                                                  q,
                                                  output,
                                                  qobj,
                                                  sqlss,
                                                  lookupTablesUsed,
                                                  queryHasTimeRestriction,
                                                  eanamesUsed,
                                                  termInfo );
            }
            else if( tokenfc == "|" )
            {
                string termMergeSQLOpCode = " or ";
                return BuildQuery_LogicalCombine( termMergeSQLOpCode,
                                                  q,
                                                  output,
                                                  qobj,
                                                  sqlss,
                                                  lookupTablesUsed,
                                                  queryHasTimeRestriction,
                                                  eanamesUsed,
                                                  termInfo );
            }

            string eaname = getStrAttr( lc, "token", "" );
            eanamesUsed.insert( eaname );
            IndexableValue iv = getIndexableValueFromToken( eaname, rc );
            string value = asString( iv );
//            string comparisonOperator = iv.getComparisonOperator();
            string xLookupTableName = "strlookup";
            AttrType_t attrTypeID = inferAttrTypeID( iv );

//             cerr << "ADDING TO eanamesUsed:" << eaname
//                  << " val:" << value
//                  << endl;
            
            xLookupTableName = getTableName( attrTypeID );

            termInfo.insert(
                make_pair( eaname,
                           MetaEAIndexerInterface::QueryTermInfo( attrTypeID,
                                                                  xLookupTableName )));

            if( starts_with( eaname, "atime" )
                || starts_with( eaname, "ferris-current-time" ) )
            {
                queryHasTimeRestriction = true;
            }
            if( eaname == "ferris-ftx" || eaname == "ferris-fulltext-search" )
            {
                string ftxpath = getFulltextIndexPath();

                if( ftxpath.empty() )
                {
                    sqlss << " true ";
                    return output;
                }
                
                try
                {
                    LG_EAIDX_D << "ftxpath:" << ftxpath << endl;
                
                    FullTextIndex::fh_idx fidx = 
                        FullTextIndex::Factory::getFullTextIndex( ftxpath );

                    string our_dbname = getConfig( CFG_IDX_DBNAME_K, "" );
                    bool useShortcutDocIDs = fidx->isTSearch2IndexInGivenDatabase( our_dbname );

                    string qstr = iv.rawValueString();
                    docNumSet_t docnums;

                    if( useShortcutDocIDs )
                    {
                        LG_EAIDX_D << "shortcut evaluation for tsearch2 index. "
                                   << "running ftx query:" << qstr << " against ftxpath:" << ftxpath << endl;
                        fidx->ExecuteRawFullTextQuery( qstr, docnums, getLimit() );
                    }
                    else
                    {
                        LG_EAIDX_D << "having to lookup all URLs from fulltext query to integrate cross engine index. "
                                   << "running ftx query:" << qstr << " against ftxpath:" << ftxpath << endl;
                        docNumSet_t tmp;
                        fidx->ExecuteRawFullTextQuery( qstr, tmp, getLimit() );

                        stringset_t earls;
                        {
//                            stringstream sqlins;
                            for( docNumSet_t::const_iterator ti = tmp.begin(); ti!=tmp.end(); ++ti )
                            {
                                docid_t d = *ti;
                                string earl = fidx->resolveDocumentID( d );
                                earls.insert( earl );
//                                 sqlins << "insert into urlmap values ('" << earl << "');" << endl;
                            }

//                             LG_EAIDX_D << "insert SQL:" << tostr(sqlins) << endl;
//                             work trans( *m_connection, "adding foreign urls to index" );
//                             trans.exec( sqlins.str() );
//                             trans.commit();
                        }

                        stringstream sqlss;
                        sqlss << "select urlid from urlmap where url in (''";
//                        copy( earls.begin(), earls.end(),  ostream_iterator<string>(sqlss,",") );
                        for( stringset_t::const_iterator si = earls.begin(); si != earls.end(); ++si )
                        {
                            sqlss << ",'" << *si << "'";
                        }
                        sqlss << ");" << endl;
                        
                        LG_EAIDX_D << "select urlid SQL:" << tostr(sqlss) << endl;
                        work trans( *m_connection, "selecting foreign urls to index" );
                        result res;
                        res = ExecuteRes( res, trans, sqlss.str() );

                        for (result::const_iterator c = res.begin(); c != res.end(); ++c)
                        {
                            long urlid = 0;
                            c[0].to(urlid);
                            docnums.insert( urlid );
                            LG_EAIDX_D << " urlid:" << urlid << endl;
                        }
                        LG_EAIDX_D << "docnums.sz:" << docnums.size() << endl;
                    }
                    
                    
                    sqlss << "d.urlid in" << nl
                          << "   (";
                
                    bool v = true;
                    for( docNumSet_t::const_iterator di = docnums.begin();
                         di != docnums.end(); ++di )
                    {
                        if( v ) v = false;
                        else    sqlss << ",";
                    
                        sqlss << " " << *di;
                    }
                    if( v )
                        sqlss << "-1";
                    sqlss << " ) ";
                }
                catch( exception& e )
                {
                    cerr << e.what() << endl;
                }
                return output;
            }
            if( starts_with( eaname, "multiversion-mtime" )
                || starts_with( eaname, "multiversion-atime" ) 
                )
            {
                queryHasTimeRestriction = true;
                eaname = eaname.substr( strlen( "multiversion-" ));
            }
            
            LG_EAIDX_D << "BuildQuery() att:" << attrTypeID
                       << " iv.att:" << iv.getAttrTypeID()
                       << " isCaseSen:" << iv.isCaseSensitive()
                       << " val:" << value
                       << endl;

            if( tokenfc == "==" )
            {
                lookupTablesUsed.insert( xLookupTableName );
                AddSQLOp( sqlss, eaname, tokenfc, iv, iv.getAttrTypeID(), lookupTablesUsed );
            }
            else if( tokenfc == "=~" )
            {
                AddSQLOp( sqlss, eaname, tokenfc, iv, iv.getAttrTypeID(), lookupTablesUsed );
            }
            else if( tokenfc == ">=" )
            {
                AddSQLOp( sqlss, eaname, tokenfc, iv, iv.getAttrTypeID(), lookupTablesUsed );
            }
            else if( tokenfc == "<=" )
            {
                AddSQLOp( sqlss, eaname, tokenfc, iv, iv.getAttrTypeID(), lookupTablesUsed );
            }
            else if( tokenfc == "=?=" )
            {
                AddSQLOpHeur( sqlss, eaname, "=", iv, lookupTablesUsed );
            }
            else if( tokenfc == ">?=" )
            {
                AddSQLOpHeur( sqlss, eaname, ">=", iv, lookupTablesUsed );
            }
            else if( tokenfc == "<?=" )
            {
                AddSQLOpHeur( sqlss, eaname, "<=", iv, lookupTablesUsed );
            }
        }

        docNumSet_t&
        EAIndexerPostgresql::BuildQuerySQL(
            fh_context q,
            docNumSet_t& output,
            fh_eaquery qobj,
            std::stringstream& SQLHeaderSS,
            std::stringstream& SQLWherePredicatesSS,
            std::stringstream& SQLTailerSS,
            stringset_t& lookupTablesUsed,
            bool& queryHasTimeRestriction,
            string& DocIDColumn,
            stringset_t& eanamesUsed,
            MetaEAIndexerInterface::BuildQuerySQLTermInfo_t& termInfo )
        {
            fh_stringstream fSQLWherePredicatesSS;
            docNumSet_t& ret = BuildQuery( q,
                                           output,
                                           qobj,
                                           fSQLWherePredicatesSS,
                                           lookupTablesUsed,
                                           queryHasTimeRestriction,
                                           eanamesUsed,
                                           termInfo );
            LG_EAIDX_D << "EAIndexerPostgresql::BuildQuerySQL() where:" << tostr(fSQLWherePredicatesSS)
                       << endl;
            SQLWherePredicatesSS << tostr(fSQLWherePredicatesSS);

            if( queryHasTimeRestriction )
            {
                DocIDColumn = "urlid";

//                 SQLHeaderSS << "SELECT urlid" << nl
//                             << "FROM  " << nl
//                             << "  (SELECT docidtime, urlid " << nl
//                             << "     FROM  docmap d " << nl
//                             << "    WHERE  " << nl
//                             << "           " << nl;

                SQLHeaderSS << "SELECT urlid" << nl
                            << "FROM  " << nl
                            << "  (SELECT docidtime, urlid " << nl
                            << "     FROM  ( select * from docmap union select * from docmap_multiversion ) d " << nl
                            << "    WHERE  " << nl
                            << "           " << nl;
                
                
            }
            else
            {
                DocIDColumn = "urlid";
//                 SQLHeaderSS << "SELECT d.urlid as urlid " << nl
//                             << "FROM docmap d, " << nl
//                             << "  ( select max(docidtime) as ddtime, urlid" << nl
//                             << "    from docmap " << nl
//                             << "    group by urlid  " << nl
//                             << "   ) dd " << nl
//                             << "WHERE d.urlid=dd.urlid" << nl
//                             << "AND d.docidtime=dd.ddtime" << nl
//                             << "AND "
//                             << nl;

//                 SQLHeaderSS << "SELECT d.urlid as urlid " << nl
//                             << "FROM currentversions d " << nl
//                             << "WHERE " << nl
//                             << nl;


                SQLHeaderSS << "SELECT d.urlid as urlid " << nl
                            << "FROM docmap d " << nl
                            << "WHERE " << nl
                            << nl;
            }

            if( queryHasTimeRestriction )
            {
                SQLTailerSS << "  ) d" << nl
                            << " GROUP by  urlid";
            }
            
            return ret;
        }
        
        
        docNumSet_t&
        EAIndexerPostgresql::ExecuteQuery( fh_context q,
                                           docNumSet_t& output,
                                           fh_eaquery qobj,
                                           int limit )
        {
            m_queryLimit = limit;
            
            stringset_t eanamesUsed;
            stringset_t lookupTablesUsed;
            bool queryHasTimeRestriction = false;
            std::stringstream HeaderSS;
            std::stringstream whereclauseSS;
            std::stringstream TailerSS;
            string DocIDColumn = "docid";
            MetaEAIndexerInterface::BuildQuerySQLTermInfo_t termInfo;
            
            BuildQuerySQL( q, output, qobj,
                           HeaderSS,
                           whereclauseSS,
                           TailerSS,
                           lookupTablesUsed,
                           queryHasTimeRestriction,
                           DocIDColumn,
                           eanamesUsed,
                           termInfo );

            fh_stringstream sqlss;
            sqlss << HeaderSS.str() << endl;
            sqlss << whereclauseSS.str() << endl;
            sqlss << TailerSS.str() << endl;
            if( limit )
            {
                sqlss << " limit " << limit << endl;
            }
            
            LG_EAIDX_D << "SQL :" << nl << tostr(sqlss) << endl << endl;
            
            nontransaction trans( *m_connection, "query" );
            result res = trans.exec( tostr(sqlss) );

            for (result::const_iterator c = res.begin(); c != res.end(); ++c)
            {
                docid_t d = 0;
                c[0].to(d);
                output.insert( d );
//                LG_EAIDX_D << "urlid:" << d << endl;
            }

            m_queryLimit = 0;
            return output;
        }

        void
        EAIndexerPostgresql::precacheDocIDs( docNumSet_t& docnums, std::map< docid_t, std::string >& cache )
        {
            LG_EAIDX_D << "precacheDocIDs() top..." << endl;
            fh_stringstream sqlss;

            if( docnums.empty() )
                return;
            
            sqlss << "select url,urlid "
                  << " from urlmap "
                  << " where urlid "
                  << " in (";
            bool virgin = true;
            for( docNumSet_t::const_iterator di = docnums.begin();
                 di != docnums.end(); ++di )
            {
                if( virgin )  virgin = false;
                else          sqlss << ",";
                sqlss << *di;
            }
            sqlss << ")";

            LG_EAIDX_D << "precacheDocIDs() SQL:" << tostr(sqlss) << endl;
            
            work trans( *m_connection, "resolve docid cache" );
            result res;
            res = ExecuteRes( res, trans, tostr( sqlss ).c_str() );
            for (result::const_iterator c = res.begin(); c != res.end(); ++c)
            {
                docid_t id = 0;
                string earl;
                c[0].to( earl );
                c[1].to( id );
                cache[id] = earl;
            }
            return;
        }
        
        
        std::string
        EAIndexerPostgresql::resolveDocumentID( docid_t id )
        {
            fh_stringstream sqlss;
                        
            sqlss << "select url "
                  << " from urlmap "
                  << " where urlid "
                  << " = " << id << "";
            LG_EAIDX_D << "SQL:" << tostr(sqlss) << endl;

            work trans( *m_connection, "resolve docid" );
            result res;
            res = ExecuteRes( res, trans, tostr( sqlss ).c_str() );
            if( !res.empty() )
            {
                return res[0][0].c_str();
            }
            
            LG_EAIDX_W << "ResolveDocumentID(error) id:" << id << endl;
            fh_stringstream ess;
            ess << "Failed to resolve document ID:" << id
                << endl;
            Throw_IndexException( tostr( ess ), 0 );
        }
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        void
        EAIndexerPostgresql::purgeDocumentInstancesOlderThan( time_t t )
        {
            retire_old_docids_from_docmap();            

            work w( *m_connection, "delete old document instance data" );

//             {
//                 stringstream sqlss;
//                 sqlss << "delete from docmap where docid in ("
//                       << "  select docid from docmap"
//                       << "  where docid not in "
//                       << "     (select docid from currentversions )"
//                       << "     and docidtime < '" << toSQLTimeString(t) << "'"
//                       << ");" << endl;
//                 Execute( w, tostr(sqlss) );
//             }
            {
                stringstream sqlss;
                sqlss << "delete from docmap_multiversion where  "
                      << "     docidtime < '" << toSQLTimeString(t) << "'"
                      << " " << endl;
                Execute( w, tostr(sqlss) );
            }
            w.commit();
        }

        fh_fwdeaidx
        EAIndexerPostgresql::tryToCreateForwardEAIndexInterface()
        {
            return new ForwardEAIndexInterfacePostgresql( this );
        }
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

//         #undef  LG_EAIDX_D
//         #define LG_EAIDX_D cerr
        
        ForwardEAIndexInterfacePostgresql::ForwardEAIndexInterfacePostgresql( fh_EAIndexerPostgresql idx )
            :
            m_idx( idx )
        {
        }

        void
        ForwardEAIndexInterfacePostgresql::precache()
        {
            LG_EAIDX_D << "precache()" << endl;
            
            stringstream ss;
            ss << "select url,* from docmap d, urlmap u where u.urlid=d.urlid and u.urlid in (";
            ss << Util::createSeperatedList( m_docNumSet.begin(), m_docNumSet.end(), ',' );
            ss << ");";

            LG_EAIDX_D << "precache SQL:" << ss.str() << endl;

            work trans( *(m_idx->m_connection), "precache ea from database" );
            result res;
            res = m_idx->ExecuteRes( res, trans, tostr(ss) );
            for (result::const_iterator c = res.begin(); c != res.end(); ++c)
            {
                string earl;
                c[0].to( earl );
                m_urlcache_ref row = new m_urlcache_t;
                
                for ( result::tuple::const_iterator ti = c->begin(); ti != c->end(); ++ti )
                {
                    string v = ti->c_str();
                    row->insert( make_pair( ti->name(), v ) );
//                    cerr << "precache:" << ti->name() << " -> " << v << endl;
                }

                m_cache[ earl ] = row;
            }
            m_precached = true;
        }
        
        std::string
        ForwardEAIndexInterfacePostgresql::getStrAttr( Context* c,
                                                       const std::string& earl,
                                                       const std::string& rdn,
                                                       const std::string& def,
                                                       bool throw_for_errors )
        {
            if( !m_precached )
                precache();

            LG_EAIDX_D << "forwardcache::getStrAttr() earl:" << earl << " rdn:" << rdn << endl;
            
            m_cache_t::iterator ci = m_cache.find( earl );
            if( ci != m_cache.end() )
            {
                m_urlcache_ref r = ci->second;
                m_urlcache_t::iterator ri = r->find( rdn );
                if( ri != r->end() )
                {
                    return ri->second;
                }
            }
            if( throw_for_errors )
            {
                stringstream ss;
                ss << "No precached attribute from database for url:" << earl
                   << " attribute:" << rdn << endl;
                Throw_NoSuchAttribute( tostr(ss), 0 );
            }
            return def;
        }
        
        
    };
};



extern "C"
{
    FERRISEXP_API Ferris::EAIndex::MetaEAIndexerInterface* Create()
    {
        return new Ferris::EAIndex::EAIndexerPostgresql();
    }
};
