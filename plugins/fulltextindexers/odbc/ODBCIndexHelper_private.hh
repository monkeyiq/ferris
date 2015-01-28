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

    $Id: ODBCIndexHelper_private.hh,v 1.3 2010/09/24 21:31:57 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_ODBC_INDEX_HELPER_H_
#define _ALREADY_INCLUDED_FERRIS_ODBC_INDEX_HELPER_H_

#include <string>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Ferris.hh>
#include <FerrisLoki/loki/Factory.h>
#include <DTL.h>

#include "EAIndexerSQLCommon_private.hh"

namespace Ferris
{
    using namespace std;
    using namespace dtl;
    
    namespace Index 
    {
            struct URLMap
                :
                public Handlable
            {
                struct myRow
                {
                    int urlid;
//                    dtl::tcstring< 4096 > url;
                    string url;
    
                    myRow( int urlid = 0, string url = "" )
                        :
                        urlid( urlid ), url( url )
                        {
                        }
                };
                class myBCA
                {
                public:
                    void operator()(BoundIOs &cols, myRow &row)
                        {
                            cols["urlid"]   >> row.urlid;
                            cols["url"]     >> row.url;
                        }
                };
            
                typedef dtl::DBView<myRow> DBV;
                DBV view;
                dtl::IndexedDBView<DBV> indexed_view;
                dtl::IndexedDBView<DBV>::iterator idxview_it;
                dtl::IndexedDBView<DBV>::iterator end_it;
                int sz;
            
                URLMap( DBConnection& con, FetchMode fm )
                    :
                    view( DBV::Args()
                          .tables("urlmap")
                          .bca( myBCA() )
                          .conn( con )
                        ),
                    indexed_view(
                        IndexedDBView<DBV>::Args().view( view )
                        .indexes( "UNIQUE PrimaryIndex; urlid;" )
                        .bound( BOUND )
                        .key_mode( USE_ALL_FIELDS )
                        .fetch_mode( fm )
                        .fetch_records( 1000 )),
                    idxview_it( indexed_view.end() ),
                    end_it( idxview_it )
                    {
                        sz = indexed_view.size();
                    }
            
                string find( int urlid )
                    {
                        idxview_it = indexed_view.find( urlid );
                        if( end_it != idxview_it )
                            return idxview_it->url;
                        return "";
                    }
            };
            FERRIS_SMARTPTR( URLMap, fh_URLMap );

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        /**
         * This class needs to use getConfig() and setConfig() which are part
         * of the MetaInterface for each of the fulltext and eaindex classes.
         * Its easiest for this class to be slotted in as a parent of
         * fulltext/eaindex odbc handlers and have the metainterface as a parent
         * class of this class so that the config() methods in the metainterface
         * are directly available to this class.
         */
        template < class _Base, class DOCID_T >
        class ODBCIndexHelper
            :
            public _Base
        {
            DBConnection* m_connection;

            const char* CFG_ODBCIDX_DSN_K()
                {
                    return "cfg-odbcidx-dsn";
                }
            
            const char* CFG_ODBCIDX_USER_K()
                {
                    return "cfg-odbcidx-user";
                }
            
            const char* CFG_ODBCIDX_DBTYPE_K()
                {
                    return "cfg-odbcidx-dbtype";
                }
            
            const char* CFG_ODBCIDX_DBTYPE_DEFAULT()
                {
                    return "";
                }
            
            const char* CFG_ODBCIDX_USEDTLBULK_K()
                {
                    return "cfg-odbcidx-use-dtl-bulk";
                }
            

        protected:

            string getDocumentMapTableName()
                {
                    return "docmap";
                }

            string getURLMapTableName()
                {
                    return "urlmap";
                }
            
            string getURLMapURLColumnName()
                {
                    return "URL";
                }
            
            string getDocumentMapURLColumnName()
                {
                    return "URL";
                }

            string getDocumentMapDocIDColumnName()
                {
                    return "docid";
                }

            string getURLMapURLIDColumnName()
                {
                    return "urlid";
                }

            string getDocumentMapDocIDTimeColumnName()
                {
                    return "docidtime";
                }
            
            /**************************************************/
            /**************************************************/
            /**************************************************/


            fh_URLMap m_urlmap;
            fh_URLMap getURLMap()
                {
                    if( !m_urlmap )
                    {
                        m_urlmap = new URLMap( getConnection(),
                                               getFetchMode() );
                    }
                    return m_urlmap;
                }
            
            /**************************************************/
            /**************************************************/
            /**************************************************/
            
            
        public:

            ODBCIndexHelper()
                :
                m_connection( 0 ),
                m_urlmap( 0 )
                {
                }
            
            ~ODBCIndexHelper()
                {
                    if( m_connection )
                        m_connection->CommitAll();
                }
            
            void ODBCSetup()
                {
                    string user = this->getConfig( CFG_ODBCIDX_USER_K(), "", true );
            
                    fh_stringstream ss;
                    ss << "dsn="   << this->getConfig( CFG_ODBCIDX_DSN_K(), "", true );
                    if( !user.empty() )
                        ss << ";user=" << user << ";";
                    m_connection = new DBConnection();
                    m_connection->Connect( tostr( ss ) );
                }
            
            void ODBCCreateIndex( fh_context c, fh_context md )
                {
                    // raw DSN is something like this
                    // 'server=localhost;driver=MySQL;user=x;password=y'

                    string dsn    = getStrSubCtx( md, "dsn", "" );
                    string dbtype = getStrSubCtx( md, "optimize-for-dbtype", "" );
                    string user   = getStrSubCtx( md, "user", "" );
                    string useDTLBulk = getStrSubCtx( md, "use-odbc3-bulkload", "0" );
                    setConfig( CFG_ODBCIDX_DSN_K(),    dsn );
                    setConfig( CFG_ODBCIDX_DBTYPE_K(), tolowerstr()( dbtype ) );
                    setConfig( CFG_ODBCIDX_USER_K(), user );
                    setConfig( CFG_ODBCIDX_USEDTLBULK_K(), useDTLBulk );
                    fh_stringstream ss;
                    ss << "dsn="   << dsn;
                    if( !user.empty() )
                        ss << ";user=" << user << ";";
            
                    m_connection = new DBConnection();
                    m_connection->Connect( tostr( ss ) );
                }
            
            DBConnection& getConnection()
                {
                    return *m_connection;
                }

            FetchMode
            getFetchMode()
                {
                    string dbtype = getDBType();
                    
                    if( dbtype == "mysql" )
                        return SINGLE_FETCH;
                    
                    return BULK_FETCH;
                }
            
            void ODBCsync()
                {
                    if( m_connection )
                        m_connection->CommitAll();
                }
            
            bool useDTLBulk()
                {
                    return toType< bool >( getConfig( CFG_ODBCIDX_USEDTLBULK_K(), "", true ) );
                }
            
            void Execute( const std::string& sql )
                {
                    DBStmt( sql, getConnection() ).Execute();
                }
            
            void Execute( const char** commands )
                {
                    for( const char** cmd = commands; *cmd; ++cmd )
                    {
                        Execute( *cmd );
                    }
                }

            void Execute( const stringlist_t& sl )
                {
                    for( stringlist_t::const_iterator si=sl.begin();si!=sl.end();++si)
                        Execute( *si );
                }
            
            string getDBType()
                {
                    return getConfig( CFG_ODBCIDX_DBTYPE_K(), "", true );
                }

            long getDocumentID( const std::string& earl,
                                bool multiVersion,
                                long& urlid )
                {
                    string columnName = getDocumentMapDocIDColumnName();
                    
                    fh_stringstream sqlss;
                    if( multiVersion )
                    {
                        sqlss << "select dm.docid as docid, um.urlid as urlid"
                              << " from " << getDocumentMapTableName() << " as dm, "
                              <<             getURLMapTableName() << " as um"
                              << " where " << getURLMapURLColumnName()
                              << " = '" << earl << "'"
                              << " and dm.urlid = um.urlid"
                              << " order by "
                              << getDocumentMapDocIDTimeColumnName() << " desc";
                    }
                    else
                    {
                        sqlss << "select * from " << getDocumentMapTableName()
                              << " where " << getDocumentMapURLColumnName()
                              << " = '" << earl << "'";
                    }
//                    cerr << "SQL1:" << tostr( sqlss ) << endl;
                    
                    DynamicDBView<> view( DynamicDBView<>::Args()
                                          .tables( tostr(sqlss) )
                                          .conn( getConnection() ));
//                    cerr << "SQL2:" << tostr( sqlss ) << endl;
            
                    long docid = -1;
                    for( DynamicDBView<>::sql_iterator print_it = view.begin();
                         print_it != view.end(); print_it++)
                    {
                        variant_row row = *print_it;
                        string docidstr = row[ columnName ];
                        docid = toType<long>( docidstr );
                        try
                        {
                            if( multiVersion )
                            {
                                string s = row[ "urlid" ];
                                urlid = toType<long>( s );
                            }
                        }
                        catch( exception& e )
                        {
                            cerr << "ERR:" << e.what() << endl;
                            throw;
                        }
                        break;
                    }

                    return docid;
                }

            /**
             * Called whenever the same document is being indexed
             * a subsequent time. This allows subclasses to remove
             * old data from tables for a simpler update.
             */
            virtual void reindexingDocument( fh_context c, DOCID_T docid ) = 0;

            /**
             * check if the document is already in the index, if it is
             * we have to remove it from the join table and use its
             * document ID, otherwise we have to create a new entry
             * in the docmap table and retain the docid for inserting
             * entries into the join table.
             */
            DOCID_T obtainDocumentID( fh_context c, bool multiVersion = false )
                {
                    long urlid = 0;
                    const std::string& earl = c->getURL();

                    LG_EAIDX_D << "obtainDocumentID(1) multiVersion:" << multiVersion
                               << " c:" << earl << endl;
                    
                    long docid = getDocumentID( earl, multiVersion, urlid );
                    LG_EAIDX_D << "obtainDocumentID(2) multiVersion:" << multiVersion
                               << " c:" << earl
                               << " docid:" << docid
                               << endl;
                    {
                        bool URLWasAlreadyIndexed = docid >= 0;
                        
                        if( URLWasAlreadyIndexed )
                        {
                            reindexingDocument( c, docid );
                            if( !multiVersion )
                            {
                                LG_EAIDX_D << "obtainDocumentID() not multiversioning" << endl;
                                return docid;
                            }
                        }
                        

                        fh_stringstream sqlss;
                        sqlss << "select count(" << getDocumentMapDocIDColumnName()
                              << ") as n from "
                              << getDocumentMapTableName();
                        DynamicDBView<> view( DynamicDBView<>::Args()
                                              .tables( tostr(sqlss) )
                                              .conn( getConnection() ));
            
                        for( DynamicDBView<>::sql_iterator print_it = view.begin();
                             print_it != view.end(); print_it++)
                        {
                            variant_row row = *print_it;
                            string t = row["n"];
                            docid = toType<long>( t ) + 1;
                        }

                        if( multiVersion )
                        {
                            // make a new entry in the urlmap table for
                            // non-existing URLs
                            if( !URLWasAlreadyIndexed )
                            {
                                fh_stringstream sqlss;
                                sqlss << "select count(" << "urlid"
                                      << ") as n from "
                                      << getURLMapTableName();
                                DynamicDBView<> view( DynamicDBView<>::Args()
                                                      .tables( tostr(sqlss) )
                                                      .conn( getConnection() ));
            
                                for( DynamicDBView<>::sql_iterator print_it = view.begin();
                                     print_it != view.end(); print_it++)
                                {
                                    variant_row row = *print_it;
                                    string t = row["n"];
                                    urlid = toType<long>( t ) + 1;
                                }
                            
                                fh_stringstream insertss;
                                insertss << "insert into " << getURLMapTableName()
                                         << " (" << getURLMapURLColumnName()
                                         << " ," << "urlid"
                                         << " ) values ('"
                                         << c->getURL()
                                         << "'," << urlid << ")";

                                LG_EAIDX_D << "SQL urlmap:" << tostr(insertss) << endl;
                                Execute( tostr( insertss ) );
                            }
                            
                            fh_stringstream insertss;
                            insertss << "insert into " << getDocumentMapTableName()
                                     << " (" << "urlid"
                                     << "," << getDocumentMapDocIDColumnName()
                                     << "," << getDocumentMapDocIDTimeColumnName()
                                     << ") values ("
                                     << urlid << "," << docid
                                     << ",'" << toSQLTimeString( Time::getTime() ) << "'"
                                     << ")";
                            LG_EAIDX_D << "SQL docmap:" << tostr(insertss) << endl;
                            Execute( tostr( insertss ) );
                        }
                        else
                        {
                            fh_stringstream insertss;
                            insertss << "insert into " << getDocumentMapTableName()
                                     << " (" << getDocumentMapURLColumnName()
                                     << "," << getDocumentMapDocIDColumnName()
                                     << ") values ('"
                                     << c->getURL() << "'," << docid
                                     << ")";
                            LG_EAIDX_D << "SQL docmap:" << tostr(insertss) << endl;
                            Execute( tostr( insertss ) );
                        }
                        
                        docid = getDocumentID( earl, multiVersion, urlid );
                        LG_EAIDX_D << "obtainDocumentID() new docid:" << docid
                                   << " urlid:" << urlid
                                   << endl;
////                        getConnection().CommitAll();
                    }
                    return docid;
                }


            /**
             * if multiVersion is false then resolve a docid to its URL
             * otherwise resolve a urlid to its URL.
             */
            std::string
            ODBCResolveDocumentID( DOCID_T id, bool multiVersion = false )
                {
                    LG_EAIDX_D << "ODBCResolveDocumentID(enter) id:" << id
                               << " multiVersion:" << multiVersion
                               << endl;
                    
                    std::string columnName = getDocumentMapURLColumnName();
                    std::string tableName  = getDocumentMapTableName();
                    std::string idName     = getDocumentMapDocIDColumnName();
                    
                    fh_stringstream sqlss;
                    if( multiVersion )
                    {
                        fh_URLMap um  = getURLMap();
                        string    url = um->find( id );
                        if( !url.empty() )
                            return url;
                        
                        fh_stringstream ess;
                        ess << "Failed to resolve document ID:" << id
                            << " in database:" << getConnection().GetDSN()
                            << endl;
                        Throw_IndexException( tostr( ess ), 0 );
                        
//                         columnName = getURLMapURLColumnName();
//                         tableName  = getURLMapTableName();
//                         idName     = getURLMapURLIDColumnName();
                    }
                    
                    sqlss << "select " << columnName
                          << " from " << tableName
                          << " where " << idName
                          << " = " << id << "";
                    LG_EAIDX_D << "SQL:" << tostr(sqlss) << endl;
                    
                    DynamicDBView<> view( DynamicDBView<>::Args()
                                          .tables( tostr(sqlss) )
                                          .conn( getConnection() ));

                    for( DynamicDBView<>::sql_iterator print_it = view.begin();
                         print_it != view.end(); print_it++)
                    {
                        variant_row row = *print_it;
                        string url = row[ columnName ];
                        LG_EAIDX_D << "ODBCResolveDocumentID(ok) id:" << id << endl;
                        return url;
                    }

                    LG_EAIDX_W << "ODBCResolveDocumentID(error) id:" << id << endl;
                    
                    fh_stringstream ess;
                    ess << "Failed to resolve document ID:" << id
                        << " in database:" << getConnection().GetDSN()
                        << endl;
                    Throw_IndexException( tostr( ess ), 0 );
                }
            
        };
        
    };
};
#endif
