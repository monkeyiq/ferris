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

    $Id: QTSQLIndexHelper_private.hh,v 1.3 2010/09/24 21:31:57 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_QTSQL_INDEX_HELPER_H_
#define _ALREADY_INCLUDED_FERRIS_QTSQL_INDEX_HELPER_H_

#include <string>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Ferris.hh>
#include <FerrisLoki/loki/Factory.h>
#include "General.hh"

#include "EAIndexerSQLCommon_private.hh"

#include <QtCore>
#include <QtSql>
#include <QVariant>
#include "FerrisQt_private.hh"
#undef emit

namespace Ferris
{
    using namespace std;

    
    namespace Index 
    {
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        /**
         * This class needs to use getConfig() and setConfig() which are part
         * of the MetaInterface for each of the fulltext and eaindex classes.
         * Its easiest for this class to be slotted in as a parent of
         * fulltext/eaindex qtsql handlers and have the metainterface as a parent
         * class of this class so that the config() methods in the metainterface
         * are directly available to this class.
         */
        template < class _Base, class DOCID_T >
        class QTSQLIndexHelper
            :
            public _Base
        {
        protected:
            QSqlDatabase m_db;
            
            
            std::string CFG_QTSQL_QDRIVER_K() 
            {
                return "cfg-qtsql-qdriver";
            }
        
            std::string CFG_QTSQL_DBNAME_K()
            {
                return "cfg-qtsql-dbname";
            }
        
            std::string CFG_QTSQL_HOST_K()
            {
                return "cfg-qtsql-host";
            }
        
            std::string CFG_QTSQL_USER_K()
            {
                return "CFG-QTSQL-USER";
            }
        
            std::string CFG_QTSQL_PASS_K()
            {
                return "CFG-QTSQL-PASS";
            }


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

            
        public:

            QTSQLIndexHelper()
                {
                }
            
            ~QTSQLIndexHelper()
                {
                    m_db.commit();
                }

            static QSqlQuery Execute( QSqlDatabase& db, std::string query )
            {
                QSqlQuery q( db );
                q.exec( query.c_str() );
                return q;
            }
            
            QSqlQuery
            Execute( std::stringstream& ss )
            {
                return Execute( ss.str() );
            }
        
            QSqlQuery
            Execute( std::string query )
            {
                return Execute( m_db, query );
            }
            void
            Execute( stringlist_t sl )
            {
                for( stringlist_t::iterator si = sl.begin(); si!=sl.end(); ++si )
                {
                    Execute( *si );
                }
            }
            void
            Execute( const char** p )
            {
                while( *p )
                {
                    Execute( *p );
                    ++p;
                }
            }
            
            void QTSQLSetup()
            {
                std::string qdriver =this->getConfig( CFG_QTSQL_QDRIVER_K(), "", true );
                std::string dbname = this->getConfig( CFG_QTSQL_DBNAME_K(), "", true );
                std::string host = this->getConfig( CFG_QTSQL_HOST_K(), "", true );
                std::string user = this->getConfig( CFG_QTSQL_USER_K(), "", true );
                std::string pass = this->getConfig( CFG_QTSQL_PASS_K(), "", true );
            
                stringstream connectionNameSS;
                connectionNameSS << qdriver << "-" << host;
                if( !dbname.empty() )
                    connectionNameSS << "-" << dbname;
                string connectionName = tostr(connectionNameSS);
                QSqlDatabase db = QSqlDatabase::database( connectionName.c_str() );
                if( !db.isValid() )
                {
                    db = QSqlDatabase::addDatabase( qdriver.c_str(), connectionName.c_str() );
                    LG_EAIDX_D << " qtDatabaseType:" << qdriver
                               << " host:" << host
                               << " dbname:" << dbname
                               << " user:" << user
                               << endl;
                    db.setHostName(host.c_str());
                    if( !dbname.empty() )
                        db.setDatabaseName(dbname.c_str());
                    db.setUserName(user.c_str());
                    db.setPassword(pass.c_str());
                    PWDScope _obj( this->getPath() );
                    bool ok = db.open();
                    if( !ok )
                    {
                        fh_stringstream ss;
                        ss << "Error to server:" << host
                           << " user:" << user
                           << endl;
                        LG_QTSQL_D << ss.str() << endl;
                    }
                }

                m_db = db;
                }
            
            void QTSQLCreateIndex( fh_context c, fh_context md )
                {
                    string qdriver = getStrSubCtx( md, "qdriver", "QSQLITE" );
                    string dbname  = getStrSubCtx( md, "dbname", "index.db" );
                    string host    = getStrSubCtx( md, "host", "" );
                    string user    = getStrSubCtx( md, "user", "" );
                    string pass    = getStrSubCtx( md, "pass", "" );
                    this->setConfig( CFG_QTSQL_QDRIVER_K(),    qdriver );
                    this->setConfig( CFG_QTSQL_DBNAME_K(),     dbname );
                    this->setConfig( CFG_QTSQL_HOST_K() ,      host );
                    this->setConfig( CFG_QTSQL_USER_K(),       user );
                    this->setConfig( CFG_QTSQL_PASS_K(),       pass );
                    this->Setup();
                }
            
            QSqlDatabase& getConnection()
                {
                    return m_db;
                }

            
            void QTSQLsync()
                {
                    cerr << "QTSQLsync()" << endl;
                    m_db.commit();
                }
            
            string getDBType()
                {
                    return this->getConfig( CFG_QTSQL_QDRIVER_K(), "", true );
                }

            long
            getDocumentID( const std::string& earl,
                           long& urlid )
            {
                string columnName = getDocumentMapDocIDColumnName();
                
                std::stringstream sqlss;
                sqlss << "select * from " << getDocumentMapTableName()
                      << " where " << getDocumentMapURLColumnName()
                      << " = '" << earl << "'";
                QSqlQuery q = Execute( sqlss );
                long docid = -1;
                if (q.next())
                {
                    QSqlRecord rec = q.record();
                    docid = toint(tostr(q.value(rec.indexOf("docid")).toString()));
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
            DOCID_T
            obtainDocumentID( fh_context c )
            {
                long urlid = 0;
                const std::string& earl = c->getURL();

                LG_EAIDX_D << "obtainDocumentID(1) c:" << earl << endl;
                    
                long docid = getDocumentID( earl, urlid );
                LG_EAIDX_D << "obtainDocumentID(2) c:" << earl << " docid:" << docid << endl;
                {
                    bool URLWasAlreadyIndexed = docid >= 0;
                        
                    if( URLWasAlreadyIndexed )
                    {
                        reindexingDocument( c, docid );
                        return docid;
                    }
                        
                    std::stringstream insertss;
                    insertss << "insert into " << getDocumentMapTableName()
                             << " (" << getDocumentMapURLColumnName()
                             << ") values ('"
                             << c->getURL() 
                             << "');";
                    LG_EAIDX_D << "SQL docmap:" << tostr(insertss) << endl;
                    getConnection().transaction();
                    QSqlQuery query = Execute( insertss );
                    LG_EAIDX_D << "LastID:" << query.lastInsertId().toInt() << endl;
                    LG_EAIDX_D << "err.databasetext:" << tostr(query.lastError().databaseText()) << endl;
                    LG_EAIDX_D << "err.drivertext  :" << tostr(query.lastError().driverText())<< endl;
                    LG_EAIDX_D << "err.isvalid     :" << tostr(query.lastError().isValid())<< endl;
                    docid = query.lastInsertId().toInt();
//                    docid = getDocumentID( earl, urlid );
                    LG_EAIDX_D << "obtainDocumentID() new docid:" << docid
                               << " urlid:" << urlid
                               << endl;
////                        getConnection().CommitAll();
//////////                    getConnection().commit();
                }
                return docid;
            }


            /**
             * if multiVersion is false then resolve a docid to its URL
             * otherwise resolve a urlid to its URL.
             */
            std::string resolveDocumentID( DOCID_T id )
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
            QString q( std::string s )
            {
                return s.c_str();
            }


            virtual bool getIndexMethodSupportsIsFileNewerThanIndexedVersion()
            {
                return true;
            }

            void touchdocmapdocidtime( const fh_context& c, long docid )
            {
                time_t ct = getTimeAttr( c, "ferris-should-reindex-if-newer", Time::getTime() );
                stringstream sqlss;
                sqlss << "update docmap "
                      << " set docidtime = " << ct
                      << " where docid = " << docid << endl;
                Execute(sqlss);
            }

            virtual std::string getisFileNewerThanIndexedVersionDBTimeColumnName()
            {
                return "mtime";
            }
            
            virtual bool isFileNewerThanIndexedVersion( const fh_context& c )
            {
                const std::string dbtimecol = getisFileNewerThanIndexedVersionDBTimeColumnName();
                
                LG_EAIDX_D << "isFileNewerThanIndexedVersion() c:" << c->getURL() << endl;
                
                bool ret = true;

                time_t tt = Time::getTime();
                time_t ct = getTimeAttr( c, "ferris-should-reindex-if-newer", 0 );
                if( !ct )
                    return ret;

                QSqlQuery query( m_db );
                std::stringstream querystr;
                querystr << "select " << dbtimecol << " from docmap where url = :earl";
                query.prepare( querystr.str().c_str() );
                query.bindValue( ":earl", q(c->getURL()) );
                query.exec();
                if (query.next())
                {
                    QSqlRecord rec = query.record();
                    string mtimestring = tostr(query.value(rec.indexOf(dbtimecol.c_str())).toString());
                    cerr << "mtimestring:" << mtimestring << endl;
                    time_t idxtime = 0;
                    if( isNumber(mtimestring) )
                        idxtime = toType<time_t>(mtimestring);
                    else
                        idxtime = Time::toTime(Time::ParseTimeString(mtimestring));
                    LG_EAIDX_D << "idxtime:" << idxtime << endl;
                    LG_EAIDX_D << "ct:" << ct << endl;
                    ret = idxtime < ct;
                }
            
                return ret;
            }
                
            
        };
        
    };
};
#endif
