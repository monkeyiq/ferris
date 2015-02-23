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

    $Id: libeaindexqtsql.cpp,v 1.1 2006/12/07 06:49:43 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>


#include "EAIndexerMetaInterface.hh"
#include "EAIndexer.hh"
#include "EAQuery.hh"
#include "FactoriesCreationCommon_private.hh"
#include "EAIndexerSQLCommon_private.hh"
#include "../../fulltextindexers/qtsql/QtSQLIndexHelper_private.hh"
#include "FullTextQuery.hh"


using namespace std;

namespace Ferris
{
    namespace EAIndex 
    {
        static const char* CFG_QTSQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K
        = CFG_EAIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K;

        QSqlQuery Execute( QSqlDatabase& db, std::string query )
        {
            return ::Ferris::Index::QTSQLIndexHelper< MetaEAIndexerInterface, docid_t >::Execute( db, query );
        }
        
        static QString q( std::string s )
        {
            return s.c_str();
        }
        
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/

        struct AttrMap
            :
            public Handlable
        {

            QSqlDatabase& m_db;
            int m_sz;
            
            AttrMap( QSqlDatabase& db )
                :
                m_db(db),
                m_sz(-1)
                {
                }

            void updateSizeCache()
                {
                    m_sz = -1;
                    QSqlQuery query = m_db.exec( "select count(*) from attrmap" );
                    m_sz = query.size();
                }
            
            long find( const std::string& an, int att )
                {
                    QSqlQuery query( m_db );
                    query.prepare( "select * from attrmap where attrtype = :att and attrname = :aname" );
                    query.bindValue( ":att",   att );
                    query.bindValue( ":aname", an.c_str() );
                    query.exec();
                    while (query.next())
                    {
                        QSqlRecord rec = query.record();
                        string aid = tostr(query.value(rec.indexOf("attrid")).toString());
                        return toint(aid);
                    }
                    return -1;
                }
            
            long createNextAttrID()
                {
                    updateSizeCache();
                    long ret = m_sz;
                    m_sz++;
                    return ret;
                }
            long ensure( const std::string& an, int att )
                {
                    long aid = find( an, att );
                    if( aid >= 0 )
                        return aid;
                    
                    LG_EAIDX_D << "attrmap.add() an:" << an
                               << " id:" << (m_sz+1)
                               << " attrtype:" << att
                               << endl;

                    QSqlQuery query( m_db );
                    query.prepare( "insert into attrmap (attrtype,attrname) values (:att,:aname)");
                    query.bindValue( ":att",   att );
                    query.bindValue( ":aname", an.c_str() );
                    query.exec();
                    return find( an, att );
                }
                
        };
        FERRIS_SMARTPTR( AttrMap, fh_AttrMap );

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/

        template < class T >
        struct ValueMap
            :
            public Handlable
        {
            typedef T ValueType;

            QSqlDatabase& m_db;
            int m_sz;
            std::string m_tablename;

            ValueMap( QSqlDatabase& db,
                      const std::string& tablename )
                : m_db( db )
                , m_sz( -1 )
                , m_tablename( tablename )
                {
                }

            void updateSizeCache()
                {
                    m_sz = -1;
                    QSqlQuery query( m_db );
                    query.prepare( "select count(*) from :tn" );
                    query.bindValue( ":tn", m_tablename.c_str() );
                    query.exec();
                    m_sz = query.size();
                }
            
            
            long find( const std::string& v )
                {
                    QSqlQuery query( m_db );
                    query.prepare( "select * from :tn where attrvalue = :v" );
                    query.bindValue( ":tn", m_tablename.c_str() );
                    query.bindValue( ":v",   v.c_str() );
                    query.exec();
                    while (query.next())
                    {
                        QSqlRecord rec = query.record();
                        string ret = tostr(query.value(rec.indexOf("valueid")).toString());
                        return toint(ret);
                    }
                    return -1;
                }
            long createNextAttrID()
                {
                    updateSizeCache();
                    long ret = m_sz;
                    m_sz++;
                    return ret;
                }
            long ensure( const std::string& Val )
                {
                    long vid = find( Val );
                    if( vid >= 0 )
                        return vid;
                    
                    
                    vid = createNextAttrID();
                    if( vid < 0 )
                        vid = 1;

                    LG_EAIDX_D << "valuemap.add() tn:" << m_tablename << " val:" << Val << " new vid:" << vid << endl;
                    QSqlQuery query( m_db );
                    stringstream ss;
                    ss << "insert into " << m_tablename << " ( vid, attrvalue ) values ( :newvid, :val )";
                    query.prepare( ss.str().c_str() );
//                    query.prepare( "insert into :tn ( vid, attrvalue ) values ( :newvid, :val )");
//                    query.bindValue( ":tn", m_tablename.c_str() );
                    query.bindValue( ":newvid", (int)vid );
                    query.bindValue( ":val",   QString(Val.c_str()) );
                    query.exec();

                    // {
                    //     stringstream ss;
                    //     ss << "insert into " << m_tablename << " (vid,attrvalue) values ( "
                    //        << vid << ",'" << Val << "');";
                    //     Execute( m_db, ss.str() );
                    // }
                    
                    return vid;
                }

            void dump()
                {
                    cerr << "valuemap begin------------------------------------" << endl;
                    cerr << "--------------------------------------------------" << endl;
                }
        };
        typedef ValueMap< string > ValueMapString_t;
        typedef ValueMap< int >    ValueMapInt_t;
        typedef ValueMap< double > ValueMapDouble_t;
        FERRIS_SMARTPTR( ValueMapString_t, fh_ValueMapString );
        FERRIS_SMARTPTR( ValueMapInt_t,    fh_ValueMapInt );
        FERRIS_SMARTPTR( ValueMapDouble_t, fh_ValueMapDouble );

        typedef ValueMap< time_t > ValueMapTime_t;
        FERRIS_SMARTPTR( ValueMapTime_t, fh_ValueMapTime );

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/

        struct FERRISEXP_DLLLOCAL DocAttrs
            :
            public Handlable
        {
        public:
            QSqlDatabase& m_db;

            
            DocAttrs( QSqlDatabase& db )
                : m_db( db )
                {
                }

            
            void set( long docid, long aid, long vid )
                {
                    LG_EAIDX_D << " docattrs.set() docid:" << docid
                               << " aid:" << aid << " vid:" << vid << endl;

                    try 
                    {
                        QSqlQuery query( m_db );
                        query.prepare( "delete from docattrs where docid = :docid and attrid = :aid and vid = :vid" );
                        query.bindValue( ":docid", q(tostr(docid)) );
                        query.bindValue( ":aid",   q(tostr(aid)) );
                        query.bindValue( ":vid",   q(tostr(vid)) );
                        query.exec();
                    }
                    catch( exception& e )
                    {
                    }
                    QSqlQuery query( m_db );
                    query.prepare( "insert into docattrs (docid,attrid,vid) values (:docid,:aid,:vid)" );
                    query.bindValue( ":docid", q(tostr(docid)) );
                    query.bindValue( ":aid",   q(tostr(aid)) );
                    query.bindValue( ":vid",   q(tostr(vid)) );
                    query.exec();
                }
        };
        FERRIS_SMARTPTR( DocAttrs, fh_DocAttrs );
        
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        class FERRISEXP_DLLLOCAL EAIndexerQTSQL
            :
            public ::Ferris::Index::QTSQLIndexHelper< MetaEAIndexerInterface, docid_t >
        {
            fh_AttrMap        m_attrmap;
            fh_ValueMapString vm_string;
            fh_ValueMapString vm_stringnocase;
            fh_ValueMapInt    vm_int;
            fh_ValueMapDouble vm_double;
            fh_ValueMapTime   vm_time;
            fh_DocAttrs       m_docattrs;
            stringmap_t       m_ExtraColumnsToInlineInDocmap;
            int               m_filesIndexedCount;

            
            
        protected:

            virtual void Setup();
            virtual void CreateIndex( fh_context c,
                                      fh_context md );
            virtual void CommonConstruction();

            virtual std::string asString( IndexableValue& v, AttrType_t att );
            virtual std::string asString( IndexableValue& v )
                {
                    return asString( v, v.getAttrTypeID() );
                }
            
            
            string  getTableName( AttrType_t att );
            
        public:
            EAIndexerQTSQL();
            virtual ~EAIndexerQTSQL();

            virtual void sync();
            virtual void prepareForWrites( int f );
            virtual void allWritesComplete();
            QSqlDatabase& getConnection();

            virtual void reindexingDocument( fh_context c, docid_t docid );
            virtual void addToIndex( fh_context c,
                                     fh_docindexer di );
            void AddSQLOp( fh_stringstream& sqlss,
                           const std::string& eaname,
                           const std::string& opcode,
                           IndexableValue& iv,
                           AttrType_t att,
                           stringset_t& lookupTablesUsed );
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
            virtual std::string resolveDocumentID( docid_t );

            fh_AttrMap  getAttrMap();
            long        ensureValueMap( const IndexableValue& iv );
            fh_DocAttrs getDocAttrs();

            fh_ValueMapString getValueMapString();
            fh_ValueMapString getValueMapCIS();
            fh_ValueMapInt    getValueMapInt();
            fh_ValueMapDouble getValueMapDouble();
            fh_ValueMapTime   getValueMapTime();

            virtual std::string getisFileNewerThanIndexedVersionDBTimeColumnName()
            {
                return "docidtime";
            }
            
            
        };

        /************************************************************/
        /************************************************************/
        /************************************************************/


        EAIndexerQTSQL::EAIndexerQTSQL()
            :
            m_attrmap( 0 ),
            vm_string( 0 ),
            vm_stringnocase( 0 ),
            vm_int( 0 ),
            vm_double( 0 ),
            vm_time( 0 ),
            m_docattrs( 0 ),
            m_filesIndexedCount( 0 )
        {
        }

        

        EAIndexerQTSQL::~EAIndexerQTSQL()
        {
        }

        QSqlDatabase&
        EAIndexerQTSQL::getConnection()
        {
            return m_db;
        }
        

        
        void
        EAIndexerQTSQL::Setup()
        {
            QTSQLSetup();
//            cerr << "EAIndexerQTSQL::Setup()" << endl;
        }



        
        void
        EAIndexerQTSQL::CreateIndex( fh_context c,
                                    fh_context md )
        {
            QTSQLCreateIndex( c, md );
            
            m_ExtraColumnsToInlineInDocmap
                = Util::ParseKeyValueString(
                    getStrSubCtx( md, "extra-columns-to-inline-in-docmap",
                                  CFG_QTSQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_DEFAULT )
                    );
            if( m_ExtraColumnsToInlineInDocmap.end() == m_ExtraColumnsToInlineInDocmap.find("docidtime"))
                m_ExtraColumnsToInlineInDocmap["docidtime"] = "timestamp";
            setConfig( CFG_QTSQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K,
                       Util::CreateKeyValueString( m_ExtraColumnsToInlineInDocmap, "=", ":" )
                );
            LG_EAIDX_D << "Extra columns for docmap:"
                 << Util::CreateKeyValueString( m_ExtraColumnsToInlineInDocmap, "=", ":" )
                 << endl;
            
        

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
                
                for( stringmap_t::const_iterator si = m_ExtraColumnsToInlineInDocmap.begin();
                     si != m_ExtraColumnsToInlineInDocmap.end(); ++si )
                {
                    string colname = EANameToSQLColumnName( si->first );
                    string coltype = si->second;
                    
                    colss << "    " << colname << " " << coltype << ", ";

                    fh_stringstream idxss;
                    idxss << "create index dmecti" << ++n << "idx "
                          << " on docmap ( " << colname << " )";
                    extraDocmapIndexes.push_back( tostr( idxss ) );
                }
                extraDocmapColumns = tostr( colss );
            }
            
            {
                fh_stringstream ss;
                ss << "CREATE TABLE docmap ("
                   << "    URL varchar(" SQL_URL_LENGTH_STR ") NOT NULL default '',"
                   <<      extraDocmapColumns
                   << "    docid INTEGER PRIMARY KEY AUTOINCREMENT "
                   << "    )";
//                cerr << ss.str() << endl;
                
                Execute( tostr(ss) );
            }
            Execute( "create index docmapurlidx  on docmap ( url )" );

            if( !extraDocmapIndexes.empty() )
                Execute( extraDocmapIndexes );
            
            
            // auto_increment is non SQL92
            stringlist_t commands;

            commands.push_back( 
                "CREATE TABLE attrmap ("
                "    attrid INTEGER PRIMARY KEY AUTOINCREMENT,"
                "    attrtype int NOT NULL default 1,"
                "    attrname varchar(" SQL_ATTRNAME_LENGTH_STR ") NOT NULL default '' "
                "    )" );
            
            commands.push_back( 
                "CREATE TABLE docattrs ("
                "    attrid int NOT NULL default 0,"
                "    vid int NOT NULL default 0,"
                "    docid int NOT NULL default 0,"
                "    PRIMARY KEY  (attrid,docid,vid)"
                "    )" );
            
            commands.push_back( 
                (string)("CREATE TABLE strlookup ("
                         "    vid int NOT NULL,"
                         "    attrvalue varchar(")
                + tostr(getMaxValueSize()) + "),"
                + "    PRIMARY KEY  (attrvalue)"
                + "    )" );
                
            commands.push_back( 
                (string)("CREATE TABLE strlookupnocase ("
                         "    vid int NOT NULL,"
                         "    attrvalue varchar(")
                + tostr(getMaxValueSize()) + "),"
                + "    PRIMARY KEY  (attrvalue)"
                + "    )" );
            
                    
            commands.push_back( 
                "CREATE TABLE doublelookup ("
                "    vid int NOT NULL,"
                "    attrvalue real,"
                "    PRIMARY KEY  (attrvalue)"
                "    )" );
            
            commands.push_back( 
                "CREATE TABLE intlookup ("
                "    vid int NOT NULL,"
                "    attrvalue int,"
                "    PRIMARY KEY  (attrvalue)"
                "    )" );
            
            commands.push_back( 
                "CREATE TABLE timelookup ("
                "    vid int NOT NULL,"
                "    attrvalue timestamp,"
                "    PRIMARY KEY  (attrvalue)"
                "    )" );

            Execute( commands );

            /**
             * Create the indexes for using the database effectively.
             */
            static const char* defaultindexing_commands[] = {

                "create index attrnameidx   on attrmap      ( attrtype, attrname )",
                "create index attrididx     on docattrs     ( attrid )",
                "create index vididx        on docattrs     ( vid )",
                "create index avididx       on docattrs     ( attrid,vid )",

                // PK takes care of attrvalue for us, index the vid for joining.
//                 "create index strlookupidx  on strlookup    ( attrvalue )",
//                 "create index strnclkupidx  on strlookupnocase ( attrvalue )",
//                 "create index intvalueidx   on intlookup    ( attrvalue )",
//                 "create index doublelkpidx  on doublelookup ( attrvalue )",
//                 "create index timelkpidx    on timelookup   ( attrvalue )",
                "create index strlookupidx  on strlookup    ( attrvalue,vid )",
                "create index strnclkupidx  on strlookupnocase ( attrvalue,vid )",
                "create index intvalueidx   on intlookup    ( attrvalue,vid )",
                "create index doublelkpidx  on doublelookup ( attrvalue,vid )",
                "create index timelkpidx    on timelookup   ( attrvalue,vid )",
                
                "create index docmapididx   on docmap       ( docid )",
                0
            };
            const char** indexing_commands = defaultindexing_commands;

            Execute( indexing_commands );
            // Don't mix DML and DDL in SQL99 compliant apps.
//            getConnection().CommitAll();
            getConnection().commit();

            // make sure the empty string is awaiting us in the lookup tables.
            {
                stringlist_t ddl;
                ddl.push_back( "insert into strlookup (vid,attrvalue) values (0,'') " );
                ddl.push_back( "insert into strlookupnocase (vid,attrvalue) values (0,'') " );
                Execute( ddl );
//                getConnection().CommitAll();
                getConnection().commit();
            }

            // we always have access to the URL string
            // and map queries to use that string instead of strlookup.
            appendToEANamesIgnore("url");
        }
        
        void
        EAIndexerQTSQL::CommonConstruction()
        {
            m_ExtraColumnsToInlineInDocmap =
                Util::ParseKeyValueString(
                    getConfig( CFG_QTSQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K,
                               CFG_QTSQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_DEFAULT ));
        }
        
        
        void
        EAIndexerQTSQL::sync()
        {
            m_db.commit();
            m_db.transaction();
        }
        void EAIndexerQTSQL::prepareForWrites( int f )
        {
           int rc = getConnection().transaction();
           cerr << "prepareForWrites() rc:" << rc << endl;
           cerr << "has trans feature:" << getConnection().driver()->hasFeature(QSqlDriver::Transactions) << endl;
            
        }
        
        void EAIndexerQTSQL::allWritesComplete()
        {
            cerr << "allWritesComplete()" << endl;
           getConnection().commit();
        }
        
        
        void
        EAIndexerQTSQL::reindexingDocument( fh_context c, docid_t docid )
        {
            LG_EAIDX_D << "EAIndexerQTSQL::reindexingDocument() c:" << c->getURL() << endl;
            Execute( "delete from docattrs where docid = " + tostr(docid) );
        }


        string
        EAIndexerQTSQL::asString( IndexableValue& v, AttrType_t att )
        {
            switch( att )
            {
            case ATTRTYPEID_INT:
                return tostr(convertStringToInteger( v.rawValueString() ));
            case ATTRTYPEID_DBL:
                return v.rawValueString();
            case ATTRTYPEID_TIME:
            {
//                 LG_EAIDX_D << "EAIndexerQTSQL::asString() "
//                            << " v:" << v.rawValueString()
//                            << " time:" << Time::ParseRelativeTimeString( v.rawValueString() )
//                            << " sqltime:" << toSQLTimeString( Time::ParseRelativeTimeString( v.rawValueString() ))
//                            << endl;
                stringstream ss;
                ss << "'"
//                   << toSQLTimeString( Time::ParseRelativeTimeString( v.rawValueString() ))
                   << toSQLTimeString( convertStringToInteger( v.rawValueString() ))
                   << "'";
                return ss.str();
            }
            case ATTRTYPEID_CIS:
            case ATTRTYPEID_STR:
            {
                fh_stringstream ss;
                ss << "'";
                
//                 if( v.rawValueString().empty() )
//                 {
//                     ss << "NULL";
//                 }
                if( v.isCaseSensitive() )
                    ss << stripNullCharacters( v.rawValueString() );
                else
                    ss << stripNullCharacters( tolowerstr()( v.rawValueString() ) );
                ss << "'";
                return tostr(ss);
            }
            }
            return v.rawValueString();
        }

        string 
        EAIndexerQTSQL::getTableName( AttrType_t att )
        {
            switch( att )
            {
            case ATTRTYPEID_INT:
                return "intlookup";
            case ATTRTYPEID_DBL:
                return "doublelookup";
            case ATTRTYPEID_TIME:
                return "timelookup";
            case ATTRTYPEID_STR:
                return "strlookup";
            case ATTRTYPEID_CIS:
                return "strlookupnocase";
            }
            return "New table type not defined!";
        }
        

        template<class InputIterator, class InsertIterator> void 
        bulk_insert_helper_with_fallback(InputIterator beg,
                                         InputIterator end,
                                         size_t buffer_size, 
                                         InsertIterator ins_it)
        {
            std::copy( beg, end, ins_it );
        }

        /**
         * postgresql doesn't support QTSQL v3.0 bulk writes. An attempt at
         * them taints the cursor aswell.
         */
        template<class InputIterator, class InsertIterator>
        void 
        bulk_insert_with_fallback( InputIterator beg,
                                   InputIterator end,
                                   int collection_size,
                                   InsertIterator ins_it)
        {
            if( !collection_size )
                return;

            for( InputIterator iter = beg; iter != end; ++iter )
            {
                *ins_it = iter->second;
            }
        }

        

        void
        EAIndexerQTSQL::addToIndex( fh_context c,
                                    fh_docindexer di )
        {
            LG_EAIDX_D << "EAIndexerQTSQL::addToIndex(top)" << endl;
            
            bool hadError = false;
            string earl   = c->getURL();
            
            int attributesDone = 0;
            int signalWindow   = 0;
            stringlist_t slist = Util::parseCommaSeperatedList( getStrAttr( c, "ea-names", "" ));
            int totalAttributes = slist.size();

            Time::Benchmark bm( "earl:" + earl );
            bm.start();
            
            LG_EAIDX_D << "EAIndexerQTSQL::addToIndex() earl:" << earl << endl;
            long docid  = obtainDocumentID( c );
            LG_EAIDX_D << "EAIndexerQTSQL::addToIndex() docid:" << docid << endl;
            
            typedef list< IndexableValue > ivl_t;
            ivl_t ivl;

            for( stringlist_t::iterator si = slist.begin(); si != slist.end(); ++si )
            {
                try
                {
                    string attributeName = *si;
                    string k = attributeName;
                    string v = "";
                    if( !obtainValueIfShouldIndex( c, di,
                                                   attributeName, v ))
                        continue;

                    LG_EAIDX_D << "EAIndexerQTSQL::addToIndex() attributeName:" << attributeName << endl;
                    LG_EAIDX_D << "addContextToIndex(a) c:" << c->getURL() << endl
                               << " k:" << k << " v:" << v << endl;

                    IndexableValue iv  = getIndexableValue( c, k, v );
                    ivl.push_back( iv );
                }
                catch( exception& e )
                {
                    LG_EAIDX_D << "EXCEPTION e:" << e.what() << endl;
                }
            }

            
            //
            // Add the EA which are ment to be indexed in the docmap table
            //
            {
                stringlist_t insertSQL;
                for( ivl_t::iterator ivi = ivl.begin(); ivi!=ivl.end(); )
                {
                    IndexableValue& iv = *ivi;
                    const std::string& k = iv.rawEANameString();
                    XSDBasic_t     sct = iv.getSchemaType();

                    stringmap_t::const_iterator eci
                        = m_ExtraColumnsToInlineInDocmap.find( k );

                    if( eci != m_ExtraColumnsToInlineInDocmap.end() )
                    {                        
                        fh_stringstream sqlss;
                        sqlss << "update docmap "
                              << " set " << EANameToSQLColumnName(k) << " = " << asString( iv )
                              << " where docid = " << docid << endl;
                        LG_EAIDX_D << "inline docmap attr SQL:" << tostr(sqlss) << endl;
//                         if( k == "width" )
//                         {
//                             cerr << "SQL-WIDTH... " << tostr(sqlss) << endl;
//                         }
                        
                        insertSQL.push_back( tostr( sqlss ) );
                        ivl_t::iterator t = ivi;
                        ++ivi;
                        ivl.erase( t );

                        if( signalWindow > 5 )
                        {
                            signalWindow = 0;
                            di->getProgressSig().emit( c, attributesDone, totalAttributes );
                        }
                        ++attributesDone;
                        ++signalWindow;
                    }
                    else
                    {
                        ++ivi;
                    }
                }
                try
                {
                    Execute( insertSQL );
                }
                catch( std::exception& e )
                {
                    LG_EAIDX_D << "EXCEPTION e:" << e.what() << endl;
                    hadError = true;
                }
            }
            

            //
            // Make the NON INLINED docmap insertions
            //
            // The above loop should have removed all the items that are added
            // inline.
            //
//            typedef map< pair< string, AttrType_t >, AttrMap::myRow  > newAttrMap_t;
            // typedef list< DocAttrs::myRow > newDocAttr_t;
            // typedef map< string, ValueMapString_t::myRow > newValueMapString_t;
            // typedef map< string, ValueMapString_t::myRow > newValueMapCIS_t;
            // typedef map< string, ValueMapInt_t::myRow    > newValueMapInt_t;
            // typedef map< string, ValueMapDouble_t::myRow > newValueMapDouble_t;
            // typedef map< TIMESTAMP_STRUCT, ValueMapTime_t::myRow, lt_ts > newValueMapTime_t;

            // newAttrMap_t        newAttrMap;
            // newDocAttr_t        newDocAttr;
            // newValueMapString_t newValueMapString;
            // newValueMapCIS_t    newValueMapCIS;
            // newValueMapInt_t    newValueMapInt;
            // newValueMapDouble_t newValueMapDouble;
            // newValueMapTime_t   newValueMapTime;

            for( ivl_t::iterator ivi = ivl.begin(); ivi!=ivl.end(); ++ivi )
            {
                IndexableValue& iv = *ivi;
                const std::string& k = iv.rawEANameString();
                XSDBasic_t sct = iv.getSchemaType();
                AttrType_t att = iv.getAttrTypeID();
                if( att == ATTRTYPEID_CIS )
                    att = ATTRTYPEID_STR;

                try
                {
                    LG_EAIDX_D << "addContextToIndex(ea-prep) c:" << c->getURL() << endl
                               << " k:" << k
//                               << " aid:" << aid
                               << " sct:" << sct
                               << " attrTypeID:" << iv.getAttrTypeID()
                               << " v:" << iv.rawValueString()
                               << endl;

                    long aid = getAttrMap()->ensure( k, att );
                    
                    LG_EAIDX_D << "addContextToIndex(have aid) c:" << c->getURL()
                               << " k:" << k
                               << " att:" << iv.getAttrTypeID()
                               << " aid:" << aid
                               << endl;
                    
                    long vid = -1;
                    string v = iv.rawValueString();
                    
                    if( sct == FXD_UNIXEPOCH_T || att == ATTRTYPEID_TIME )
                    {
                        vid = getValueMapTime()->ensure( v );
                    }
                    else if( att == ATTRTYPEID_INT )
                    {
                        vid = getValueMapInt()->ensure( v );
                    }
                    else if( att == ATTRTYPEID_DBL )
                    {
                        vid = getValueMapDouble()->ensure( v );
                    }
                    else
                    {
                        v = stripNullCharacters( v );
                        vid = getValueMapString()->ensure( v );
                        

                        att = ATTRTYPEID_CIS;
                        aid = getAttrMap()->ensure( k, att );
//                        aid = addToIndex_getAID( c, di, k, att, iv, newAttrMap );
                        string lowerstr = tolowerstr()( v );
                        long CSvid = vid;
                        vid = getValueMapCIS()->ensure( lowerstr );
                    }

                    LG_EAIDX_D << "New docattrs row. docid:" << docid
                               << " aid:" << aid << " vid:" << vid << endl;
                    
                    getDocAttrs()->set( docid, aid, vid );
                }
                catch( std::exception& e )
                {
                    hadError = true;
                        
                    LG_EAIDX_D << "EXCEPTION e:" << e.what() << endl;
                    LG_EAIDX_D << "making non-inline ea tables. "
                               << " e:" << e.what() << endl;
                }
                
                if( signalWindow > 5 )
                {
                    signalWindow = 0;
                    di->getProgressSig().emit( c, attributesDone, totalAttributes );
                }
                ++attributesDone;
                ++signalWindow;
            }

            // if( hadError )
            // {
            //     LG_EAIDX_W << "addToIndex() had an error with file:" << c->getURL() << endl;
            //     cerr << "addToIndex() had an error with file:" << c->getURL() << endl;
            //     getConnection().RollbackAll();
            // }
            // else
            //     getConnection().CommitAll();
            touchdocmapdocidtime( c, docid );
//            getConnection().commit();

            getAttrMap()->updateSizeCache();
            getValueMapString()->updateSizeCache();
            getValueMapCIS()->updateSizeCache();
            getValueMapInt()->updateSizeCache();
            getValueMapDouble()->updateSizeCache();
            getValueMapTime()->updateSizeCache();
            
            ++m_filesIndexedCount;

            incrFilesIndexedCount();
            if( getFilesIndexedCount() % 1000 == 999 )
            {
                sync();
            }
        }


        
        void
        EAIndexerQTSQL::AddSQLOp( fh_stringstream& sqlss,
                                 const std::string& eaname,
                                 const std::string& opcode,
                                 IndexableValue& iv,
                                 AttrType_t att,
                                 stringset_t& lookupTablesUsed )
        {
            string lookupTableName = getTableName( att );
            lookupTablesUsed.insert( lookupTableName );

            stringmap_t::const_iterator eci
                = m_ExtraColumnsToInlineInDocmap.find( eaname );

            LG_EAIDX_D << "EAIndexerQTSQL::AddSQLOp() att:" << att << " v:" << iv.rawValueString() << endl;
            LG_EAIDX_D << "EAIndexerQTSQL::AddSQLOp() " << " v.asstring:" << asString( iv, att ) << endl;
            
            if( eaname == "url" )
            {
                att = ATTRTYPEID_CIS;
                if( isCaseSensitive( iv.rawValueString()))
                    att = ATTRTYPEID_STR;

                std::string v = asString( iv, att );
                if( opcode == "like" )
                {
                    IndexableValue t( this, iv.rawEANameString(), "%" + iv.rawValueString() + "%" );
                    v = asString( t, att );
                }
                sqlss << "d.url " << opcode << v << endl;
            }
            else if( eci != m_ExtraColumnsToInlineInDocmap.end() )
            {
                string colname = EANameToSQLColumnName( eci->first );
                string coltype = eci->second;

                sqlss << "d." << colname << " " << opcode << asString( iv, att );
            }
            else
            {
                sqlss << "d.docid in" << nl
                      << "   ("
                      << "    SELECT distinct(docattrs.docid) as docid " << nl
                      << "      FROM docattrs, attrmap , " << lookupTableName << nl
                      << "      WHERE " << nl
                      << "      (( "   << nl
                      << "         " << lookupTableName << ".attrvalue " << opcode << " "
                      << "         " << asString( iv, att ) << " "    << nl
                      << "       and " << lookupTableName << ".vid = docattrs.vid " << nl
                      << "       and " << " attrmap.attrname=\'" << eaname << "\' " << nl
                      << "       and " << " attrmap.attrtype=\'" << att << "\' " << nl
                      << "       and " << " attrmap.attrid=docattrs.attrid "        << nl
                      << "      ))"                                                 << nl
                      << "   )"
                      << endl;
            }
        }

        MetaEAIndexerInterface::AttrType_t
        EAIndexerQTSQL::SQLColumnTypeToAttrType( const std::string& coltype,
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
        EAIndexerQTSQL::AddSQLOpHeur( fh_stringstream& sqlss,
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
                if( *attrTypeIter == ATTRTYPEID_STR )
                    continue;
                
                if( v ) v = false;
                else    sqlss << " " << nl
                              << " OR d.docid in " << nl
                              << " ";
                AddSQLOp( sqlss, eaname, opcode, iv, *attrTypeIter, lookupTablesUsed );
            }
            sqlss << ")" << endl;
        }

        
        docNumSet_t&
        EAIndexerQTSQL::BuildQuery( fh_context q,
                                   docNumSet_t& output,
                                   fh_eaquery qobj,
                                   fh_stringstream& sqlss,
                                   stringset_t& lookupTablesUsed,
                                   bool& queryHasTimeRestriction,
                                   stringset_t& eanamesUsed,
                                   MetaEAIndexerInterface::BuildQuerySQLTermInfo_t& termInfo )
        {
            string token   = getStrAttr( q, "token", "" );
            string tokenfc = foldcase( token );
            fh_attribute orderedtla = q->getAttribute("in-order-insert-list");
            fh_istream   orderedtls = orderedtla->getIStream();
            
            LG_EAIDX_D << "BuildQuery() token:" << tokenfc << endl;
            LG_EAIDX_D << " q.child count:" << q->SubContextCount() << endl;

            string s;
            getline( orderedtls, s );
            LG_EAIDX_D << "BuildQuery() lc:" << s << endl;
            fh_context lc = q->getSubContext( s );

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

            getline( orderedtls, s );
            LG_EAIDX_D << "EAQuery_Heur::BuildQuery() rc:" << s << endl;
            fh_context rc = q->getSubContext( s );

            if( tokenfc == "&" )
            {
                LG_EAIDX_D << " operator &, child count:" << q->SubContextCount() << endl;

                bool v = true;
                sqlss << " (" << nl;
                for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                {
                    if( v ) v = false;
                    else    sqlss << " and " << nl;

//                    sqlss << " SELECT d.docid FROM docmap WHERE ";
                    BuildQuery( *ci, output, qobj, sqlss,
                                lookupTablesUsed, queryHasTimeRestriction,
                                eanamesUsed, termInfo );
                }
                sqlss << " ) " << nl;
            }
            else if( tokenfc == "|" )
            {
                bool v = true;
                sqlss << " (" << nl;
                for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                {
                    if( v ) v = false;
                    else    sqlss << " or " << nl;

//                    sqlss << " SELECT d.docid FROM docmap WHERE ";
                    BuildQuery( *ci, output, qobj, sqlss,
                                lookupTablesUsed, queryHasTimeRestriction,
                                eanamesUsed, termInfo );
                }
                sqlss << " ) " << nl;
            }

            string eaname = getStrAttr( lc, "token", "" );
            eanamesUsed.insert( eaname );
            IndexableValue iv = getIndexableValueFromToken( eaname, rc );
            string value = asString( iv );
//            string comparisonOperator = iv.getComparisonOperator();
            string xLookupTableName = "strlookup";
            AttrType_t attrTypeID = inferAttrTypeID( iv );

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
                    return output;
                }
                
                try
                {
                    LG_EAIDX_D << "ftxpath:" << ftxpath << endl;
                
                    FullTextIndex::fh_idx fidx = 
                        FullTextIndex::Factory::getFullTextIndex( ftxpath );

                    string qstr = iv.rawValueString();

                    LG_EAIDX_D << "shortcut evaluation for qtsql index. "
                               << "running ftx query:" << qstr << " against ftxpath:" << ftxpath << endl;
                    int limit = 0;
//                    fidx->executeRankedQuery( qstr, output, limit );

                    LG_EAIDX_D << "output.sz before:" << output.size() << endl;
                    fidx->addAllDocumentsMatchingTerm( qstr, output, limit );
                    
                    // fh_context q = MountBooleanQueryString( qstr );
                    // fh_context root_of_q = q->getSubContext( *(q->getSubContextNames().begin()) );
                    // ExecuteBooleanQuery( root_of_q, output, fidx, limit );
                    // LG_EAIDX_D << "output.sz after :" << output.size() << endl;
                }
                catch( exception& e )
                {
                    cerr << e.what() << endl;
                }
                return output;
            }
            
            
            if( tokenfc == "==" )
            {
                lookupTablesUsed.insert( xLookupTableName );

                // FIXME: remove table name, add in AttrType from iv, and lookuptables set param
//                 AddSQLOp( sqlss, xLookupTableName, eaname,
//                           "=", valueQuote, value );
                AddSQLOp( sqlss, eaname, "=", iv, iv.getAttrTypeID(), lookupTablesUsed );
            }
            else if( tokenfc == "=~" )
            {
                string opcode = "like";

                if( getDBType() == "postgresql" )
                    opcode = "~";
                
                AddSQLOp( sqlss, eaname, opcode, iv, iv.getAttrTypeID(), lookupTablesUsed );
            }
            else if( tokenfc == ">=" )
            {
                AddSQLOp( sqlss, eaname, ">=", iv, iv.getAttrTypeID(), lookupTablesUsed );
            }
            else if( tokenfc == "<=" )
            {
                AddSQLOp( sqlss, eaname, "<=", iv, iv.getAttrTypeID(), lookupTablesUsed );
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
        EAIndexerQTSQL::BuildQuerySQL(
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
            SQLWherePredicatesSS << tostr(fSQLWherePredicatesSS);

            SQLHeaderSS << "SELECT distinct(d.docid) as docid FROM docmap d " << nl
                        << " WHERE " << nl;
            
            return ret;
        }
                
        docNumSet_t&
        EAIndexerQTSQL::ExecuteQuery( fh_context q,
                                      docNumSet_t& output,
                                      fh_eaquery qobj,
                                      int limit )
        {
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

            stringstream sqlss;
            sqlss << HeaderSS.str() << endl;
            sqlss << whereclauseSS.str() << endl;
            sqlss << TailerSS.str() << endl;
            if( limit )
            {
                sqlss << " limit " << limit << endl;
            }
            
            LG_EAIDX_D << "SQL :" << nl << tostr(sqlss) << endl << endl;

            QSqlQuery query = Execute( sqlss );
            while (query.next())
            {
                QSqlRecord rec = query.record();
                long docid = toint(tostr(query.value(rec.indexOf("docid")).toString()));
                addDocID( output, docid );
            }
            
            return output;
        }
        
        std::string
        EAIndexerQTSQL::resolveDocumentID( docid_t id )
        {
            LG_EAIDX_D << "QTSQLResolveDocumentID(enter) id:" << id
                       << endl;
                    
            std::string columnName = getDocumentMapURLColumnName();
            std::string tableName  = getDocumentMapTableName();
            std::string idName     = getDocumentMapDocIDColumnName();
                    
            stringstream sqlss;
            sqlss << "select " << columnName
                  << " from " << tableName
                  << " where " << idName
                  << " = " << id << "";
            LG_EAIDX_D << "SQL:" << tostr(sqlss) << endl;

            QSqlQuery query = Execute( sqlss );
            while (query.next())
            {
                QSqlRecord rec = query.record();
                string v = tostr(query.value(rec.indexOf(columnName.c_str())).toString());
                return v;
            }
            
            LG_EAIDX_W << "QTSQLResolveDocumentID(error) id:" << id << endl;
                    
            fh_stringstream ess;
            ess << "Failed to resolve document ID:" << id
                << endl;
            Throw_IndexException( tostr( ess ), 0 );
        }

        fh_AttrMap
        EAIndexerQTSQL::getAttrMap()
        {
            if( !isBound( m_attrmap ) )
            {
                m_attrmap = new AttrMap( getConnection() );
            }
            return m_attrmap;
        }
        
        long
        EAIndexerQTSQL::ensureValueMap( const IndexableValue& iv )
        {
            AttrType_t     att = iv.getAttrTypeID();
            const string&  v   = iv.rawValueString();
            XSDBasic_t     sct = iv.getSchemaType();
            
            LG_EAIDX_D << "EAIndexerQTSQL::ensureValueMap() att:" << att
                       << " v:" << v
                       << " sct:" << sct
                       << " is epoch:" << ( sct == FXD_UNIXEPOCH_T )
                       << endl;
            
            if( sct == FXD_UNIXEPOCH_T || att == ATTRTYPEID_TIME )
            {
                getValueMapTime()->ensure( v );
            }
            else if( att == ATTRTYPEID_INT )
            {
                getValueMapInt()->ensure( v );
            }
            else if( att == ATTRTYPEID_DBL )
            {
                getValueMapDouble()->ensure( v );
            }
            else
            {
                if( !v.empty() )
                {
                    getValueMapString()->ensure( v );
                    getValueMapCIS()->ensure(  tolowerstr()( v ) );
                }
            }
        }
        
        fh_DocAttrs
        EAIndexerQTSQL::getDocAttrs()
        {
            if( !isBound( m_docattrs ) )
            {
                m_docattrs = new DocAttrs( getConnection() );
            }
            return m_docattrs;
        }
        
        fh_ValueMapString
        EAIndexerQTSQL::getValueMapString()
        {
            if( !isBound( vm_string ) )
            {
                vm_string = new ValueMapString_t(
                    getConnection(), "strlookup" );
            }
            return vm_string;
        }
        
        fh_ValueMapString
        EAIndexerQTSQL::getValueMapCIS()
        {
            if( !isBound( vm_stringnocase ) )
            {
                vm_stringnocase = new ValueMapString_t(
                    getConnection(), "strlookupnocase" );
            }
            return vm_stringnocase;
        }
        
        fh_ValueMapInt
        EAIndexerQTSQL::getValueMapInt()
        {
            if( !isBound( vm_int ) )
            {
                vm_int = new ValueMapInt_t( getConnection(), "intlookup" );
            }
            return vm_int;
        }
        
        fh_ValueMapDouble
        EAIndexerQTSQL::getValueMapDouble()
        {
            if( !isBound( vm_double ) )
            {
                vm_double = new ValueMapDouble_t( getConnection(), "doublelookup" );
            }
            return vm_double;
        }
        
        fh_ValueMapTime
        EAIndexerQTSQL::getValueMapTime()
        {
            if( !isBound( vm_time ) )
            {
                vm_time = new ValueMapTime_t(
                    getConnection(), "timelookup" );
            }
            return vm_time;
        }
        
        
        
    };
};



extern "C"
{
    FERRISEXP_API Ferris::EAIndex::MetaEAIndexerInterface* Create()
    {
        return new Ferris::EAIndex::EAIndexerQTSQL();
    }
};
