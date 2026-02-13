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

    $Id: libftxidxtsearch2.cpp,v 1.8 2010/06/06 21:30:18 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include <pqxx/pqxx>

using namespace pqxx;

#include <Ferris/FullTextIndexerMetaInterface.hh>

#include <Ferris/Ferris.hh>
#include <Ferris/FullTextIndexer_private.hh>
#include <Ferris/EAIndexerSQLCommon_private.hh>

#include <string>
using namespace std;

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


namespace Ferris
{
    namespace FullTextIndex 
    {
        static const char* CFG_IDX_USER_K   = "cfg-idx-user";
        static const char* CFG_IDX_HOST_K   = "cfg-idx-host";
        static const char* CFG_IDX_PORT_K   = "cfg-idx-port";
        static const char* CFG_IDX_DBNAME_K = "cfg-idx-dbname";
        static const char* CFG_IDX_TEMPLATE_DBNAME_K = "cfg-idx-template-dbname";
        static const char* CFG_IDX_USER_DEF   = "";
        static const char* CFG_IDX_HOST_DEF   = "localhost";
        static const char* CFG_IDX_PORT_DEF   = "";
        static const char* CFG_IDX_DBNAME_DEF = "ferrisfulltextindex";
        static const char* CFG_IDX_TEMPLATE_DBNAME_DEF = "ferrisftxtemplate";
        static const char* CFG_IDX_MULTIVERSION_K = "cfg-idx-multiversion";
        static const char* CFG_IDX_MULTIVERSION_DEF = "0";

        static string quoteStr( work& trans, const std::string& s )
        {
            stringstream ss;
            ss << "'" << trans.esc( s.c_str() ) << "'";
            return tostr(ss);
        }
        
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        class FERRISEXP_DLLLOCAL FullTextIndexerTSEARCH2
            :
            public MetaFullTextIndexerInterface
        {
            connection* m_connection;
            string makeConnectionString( bool includeDBName = true );

            bool m_multiVersion;
            
        protected:

            void retire_old_docids_from_docmap();

            
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

            FullTextIndexerTSEARCH2();
            virtual ~FullTextIndexerTSEARCH2();

            virtual void sync();
            virtual void prepareForWrites( int f );
            virtual void allWritesComplete();

            virtual
            docNumSet_t&
            ExecuteTsearch2FullTextQuery( const std::string& queryString,
                                          docNumSet_t& docnums,
                                          int limit );
            
            virtual void removeDocumentsMatchingRegexFromIndex( const std::string& s, time_t mustBeOlderThan = 0  );
            virtual bool supportsRemove()
                {
                    return true;
                }

            docNumSet_t& addAllDocumentsMatchingResult( result res, docNumSet_t& output );


            typedef map< string, time_t > m_isFileNewerThanIndexedVersionCache_t;
            m_isFileNewerThanIndexedVersionCache_t m_isFileNewerThanIndexedVersionCache;
            m_isFileNewerThanIndexedVersionCache_t &getIsFileNewerThanIndexedVersionCache();
            virtual bool getIndexMethodSupportsIsFileNewerThanIndexedVersion();
            virtual bool isFileNewerThanIndexedVersion( const fh_context& c );
            void setupIsFileNewerThanIndexedVersionCache();
            
            virtual void purgeDocumentInstancesOlderThan( time_t t );
      
            
            inline result& ExecuteRes( result& ret,
                                       work& work,
                                       const std::string& sql )
                {
                    LG_IDX_D << "SQL:" << sql << endl;
                    ret = work.exec( sql.c_str() );
                    return ret;
                }

            void Execute( work& work, const std::string& sql )
                {
                    LG_IDX_D << "SQL:" << sql << endl;
                    work.exec( sql.c_str() );
                }
            
            void Execute( work& work, const char** commands )
                {
                    for( const char** cmd = commands; *cmd; ++cmd )
                    {
                        LG_IDX_D << "SQL:" << *cmd << endl;
                        work.exec( *cmd );
                    }
                }

            void Execute( work& work, const stringlist_t& sl )
                {
                    for( stringlist_t::const_iterator si=sl.begin();si!=sl.end();++si)
                    {
                        LG_IDX_D << "SQL:" << *si << endl;
                        work.exec( si->c_str() );
                    }
                }

            docid_t
            obtainURLID( work& trans, fh_context c, const std::string& url )
                {
                    long urlid = -1;

                    LG_IDX_D << "EAIndexerPostgresql::obtainURLID(1) " << endl;
                    result res;
                    res = ExecuteRes( res, trans,
                                      "SELECT urlid FROM urlmap WHERE url=" + quoteStr(trans,url) );
                    LG_IDX_D << "EAIndexerPostgresql::obtainURLID(2) " << endl;
                    if( res.empty() )
                    {
                        Execute( trans, "INSERT INTO urlmap (url) VALUES (" + quoteStr(trans,url) + ")" );
                        LG_IDX_D << "EAIndexerPostgresql::obtainURLID(3) " << endl;
                        res = ExecuteRes( res, trans,
                                          "SELECT urlid FROM urlmap WHERE url=" + quoteStr(trans,url) );
                        LG_IDX_D << "EAIndexerPostgresql::obtainURLID(4) " << endl;
                    }
                    LG_IDX_D << "EAIndexerPostgresql::obtainURLID(5) " << endl;
                    docid_t ret;
                    res[0][0].to(ret);
                    return ret;
                }
            
        };

        
        static const char* CFG_TSEARCH2IDX_DBNAME_K          = "cfg-tsearch2idx-dbname";
        static const char* CFG_TSEARCH2IDX_DBNAME_DEFAULT    = "ftsearch2idx.fidx";
        static const char* CFG_TSEARCH2IDX_STEMLANG_K        = "cfg-tsearch2idx-stemlang";
        static const char* CFG_TSEARCH2IDX_STEMLANG_DEFAULT  = "english";

        /************************************************************/
        /************************************************************/
        /************************************************************/

        FullTextIndexerTSEARCH2::FullTextIndexerTSEARCH2()
            :
            m_connection( 0 ),
            m_multiVersion( false )
        {
        }

        FullTextIndexerTSEARCH2::~FullTextIndexerTSEARCH2()
        {
        }

        void FullTextIndexerTSEARCH2::sync()
        {
            retire_old_docids_from_docmap();
            
            nontransaction trans( *m_connection, "analyse ftxdocmap" );
            fh_stringstream ss;
            ss << "ANALYZE ftxdocmap";
            trans.exec( tostr(ss) );
        }
        
        void
        FullTextIndexerTSEARCH2::prepareForWrites( int f )
        {
            if( f & PREPARE_FOR_WRITES_ISNEWER_TESTS &&
                m_isFileNewerThanIndexedVersionCache.empty() )
            {
                setupIsFileNewerThanIndexedVersionCache();
            }
        }
        
        void
        FullTextIndexerTSEARCH2::allWritesComplete()
        {
            m_isFileNewerThanIndexedVersionCache.clear();
        }
        


        string
        FullTextIndexerTSEARCH2::makeConnectionString( bool includeDBName )
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
        FullTextIndexerTSEARCH2::Setup()
        {
            setOpenConfigReadOnly( true );
            
            string constring = makeConnectionString();
            m_connection = new connection( constring );
            LG_IDX_D << "EAIndexerPostgresql::Setup() con:" << constring << endl;
        }

        void
        FullTextIndexerTSEARCH2::CreateIndexBeforeConfig( fh_context c,
                                                          bool caseSensitive,
                                                          bool dropStopWords,
                                                          StemMode stemMode,
                                                          const std::string& lex_class,
                                                          fh_context md )
        {
        }
        
        
        void
        FullTextIndexerTSEARCH2::CreateIndex( fh_context c,
                                              bool caseSensitive,
                                              bool dropStopWords,
                                              StemMode stemMode,
                                              const std::string& lex_class,
                                              fh_context md )
        {
            string user   = getStrSubCtx( md, "user", CFG_IDX_USER_DEF );
            string host   = getStrSubCtx( md, "host", CFG_IDX_HOST_DEF );
            string port   = getStrSubCtx( md, "port", CFG_IDX_PORT_DEF );
            string dbname = getStrSubCtx( md, "dbname", CFG_IDX_DBNAME_DEF );
            string templatedbname = getStrSubCtx( md, "template-dbname", CFG_IDX_TEMPLATE_DBNAME_DEF );
            string multiVersion = getStrSubCtx( md, "multiversion", CFG_IDX_MULTIVERSION_DEF );

            setConfig( CFG_IDX_USER_K, user );
            setConfig( CFG_IDX_HOST_K, host );
            setConfig( CFG_IDX_PORT_K, port );
            setConfig( CFG_IDX_DBNAME_K, dbname );
            setConfig( CFG_IDX_TEMPLATE_DBNAME_K, templatedbname );
            setConfig( CFG_IDX_MULTIVERSION_K, multiVersion );
            
            LG_IDX_D << "Create database:" << isFalse(getStrSubCtx( md, "db-exists", "0" )) << endl;
            if( isFalse( getStrSubCtx( md, "db-exists", "0" )))
            {
                stringstream conss;
                conss << makeConnectionString( false );
                conss << " dbname=template1";
                LG_IDX_D << "makeConnectionString:" << tostr(conss) << endl;
                connection dbmaker( tostr(conss) );
                LG_IDX_D << "CreateDB(2)" << endl;
                nontransaction trans( dbmaker, "create database" );
                LG_IDX_D << "CreateDB(3)" << endl;
                stringstream sqlss;
                sqlss << "create database " << dbname << " template " << templatedbname << ";";
                LG_IDX_D << "Creating database sql:" << tostr(sqlss) << endl;
                trans.exec( sqlss.str() );
                trans.commit();
            }

            Setup();
            try
            {
                work trans( *m_connection, "setup database" );
                fh_stringstream urlmapcreatess;
                urlmapcreatess << 
                    "CREATE TABLE urlmap ("
                    "   URL varchar(" << SQL_URL_LENGTH_STR << ") NOT NULL,"
                    "   urlid serial,"
                    "   primary key( urlid )"
                    "  ) without oids ";
                Execute( trans, tostr( urlmapcreatess ) );
                Execute( trans, "create index umurlidx    on urlmap ( url )" );
                Execute( trans, "create index umurlidxcis on urlmap ( lower(url) )" );
                Execute( trans, "create index umurlidxid  on urlmap ( urlid )" );
                trans.commit();
            }
            catch( exception& e )
            {
                if( isTrue( getStrSubCtx( md, "db-exists", "0" )))
                {
                    LG_IDX_D << "Assuming urlmap already exists in creation."
                               << " e:" << e.what() << endl;
                }
                else
                    throw;
            }
            
            work CreateDBTrans( *m_connection, "setup database" );

            string docmapcreateString = "CREATE TABLE ftxdocmap ("
                "   urlid int NOT NULL references urlmap,"
                "   docid serial,"
                "   docidtime timestamp default 'now',"
                "   ftx tsvector,"
                "   primary key( urlid, docid )"
                "  ) without oids ";
            stringlist_t extraDocmapIndexes;
            extraDocmapIndexes.push_back("create index ftxdocmapidxtime on ftxdocmap ( docidtime )");
            extraDocmapIndexes.push_back("create index ftxdocmapidxftx  on ftxdocmap using gist(ftx)" );

            {
                Execute( CreateDBTrans, docmapcreateString );
                string docmapmv = Util::replace_all( docmapcreateString,
                                                     "TABLE ftxdocmap",
                                                     "TABLE ftxdocmap_multiversion" );
                Execute( CreateDBTrans, docmapmv );
            }
            {
                stringlist_t::iterator si = extraDocmapIndexes.begin();
                stringlist_t::iterator se = extraDocmapIndexes.end();
                for( ; si != se; ++si )
                {
                    string s = *si;
                    Execute( CreateDBTrans, s );
                    s = Util::replace_all( s, "create index ftxdocmap", "create index mvftxdocmap" );
                    s = Util::replace_all( s, "on ftxdocmap",           "on ftxdocmap_multiversion" );
                    Execute( CreateDBTrans, s );
                }
            }
            
            
            
            {
                stringstream ss;
                ss << "create or replace view ftxcurrentversions as " << nl
                   << "SELECT d.*" << nl
                   << "FROM ftxdocmap d," << nl
                   << "  ( select max(docidtime) as ddtime, urlid" << nl
                   << "    from ftxdocmap" << nl
                   << "    group by urlid" << nl
                   << "   ) dd" << nl
                   << "WHERE d.urlid=dd.urlid" << nl
                   << "AND d.docidtime=dd.ddtime;" << nl;
                Execute( CreateDBTrans, ss.str() );
            }
            
            CreateDBTrans.commit();
        }

        void
        FullTextIndexerTSEARCH2::CommonConstruction()
        {
            m_multiVersion = isTrue(
                this->getConfig( CFG_IDX_MULTIVERSION_K,
                                 CFG_IDX_MULTIVERSION_DEF ));
            
        }

        void
        FullTextIndexerTSEARCH2::retire_old_docids_from_docmap()
        {
            LG_IDX_D << "moving older versions ftxdocmap into _multiversion tables..." << endl;
            work trans( *m_connection, "retire old fulltext instances" );
                
            {
                stringstream ss;
                ss << "insert into ftxdocmap_multiversion "
                   << "    ( select *  from ftxdocmap where not(docid in ( select docid from ftxcurrentversions )));";
                trans.exec( tostr(ss) );
            }
            {
                stringstream ss;
                ss << "delete from ftxdocmap where  "
                   << "    not( docid in ( select docid from ftxcurrentversions ));";
                trans.exec( tostr(ss) );
            }
                
            LG_IDX_D << "commiting..." << endl;
            trans.commit();
        }
        
        void
        FullTextIndexerTSEARCH2::addToIndex( fh_context c, fh_docindexer di )
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

                work w( *m_connection, "adding document:" + c->getURL() );
                docid_t urlid = obtainURLID( w, c, c->getURL() );
                stringstream ftxdocmapss;
                ftxdocmapss << "INSERT into ftxdocmap values (" << urlid << ",DEFAULT,'now',"
                            << " to_tsvector('default',''));";
                Execute( w, tostr(ftxdocmapss) );
                w.commit();
                
                return;
//                iss = c->getIStream();
            }
            
            streamsize bytesDone    = 0;
            streamsize signalWindow = 0;
            streamsize totalBytes   = toType<streamsize>(getStrAttr( c, "size", "0" ));
            work w( *m_connection, "adding document:" + c->getURL() );
            docid_t urlid = obtainURLID( w, c, c->getURL() );

            SignalThrottle throttle = getSignalThrottle();
            if( !m_multiVersion )
            {
                stringstream sqlss;
                sqlss << "delete from ftxdocmap where urlid=" << urlid << ";";
                Execute( w, tostr(sqlss) );
            }

            const long TSVECTOR_MAX_LENGTH = 1024*1024;
            long currentSize = 0;
            string carryToken = "";
            for( bool haveMoreWork = true; haveMoreWork; )
            {
                fh_stringstream ftxdocmapss;
                ftxdocmapss << "INSERT into ftxdocmap values (" << urlid << ",DEFAULT,'now',"
                            << " to_tsvector('default','";
                if( !carryToken.empty() )
                {
                    ftxdocmapss << w.esc( carryToken.c_str() ) << " ";
                    carryToken = "";
                }

                bool isCaseSensitive = this->isCaseSensitive();
                while( !(s = di->getToken( iss )).empty() )
                {
                    if( !isCaseSensitive )
                        s = tolowerstr()( s );

                    string sqls = w.esc( s.c_str() );
                    currentSize += 16 + sqls.length();

                    if( currentSize >= TSVECTOR_MAX_LENGTH )
                    {
                        LG_IDX_D << "too much data in file, currentSize:" << currentSize
                                 << " carry:" << s << endl;
                        carryToken = s;
                        break;
                    }

                    ftxdocmapss << sqls << " ";

                    if( throttle( di->getBytesCompleted() ) )
                    {
                        di->getProgressSig().emit( c, di->getBytesCompleted(), totalBytes );
                    }
                }
                ftxdocmapss << "'));";

                currentSize = 0;
                if( carryToken.empty() )
                {
                    LG_IDX_D << "no carry token, we are done!" << endl;
                    haveMoreWork = false;
                }
                
                LG_IDX_D << "SQL:" << tostr(ftxdocmapss) << endl;
                Execute( w, tostr(ftxdocmapss) );
            }

            w.commit();
            di->getProgressSig().emit( c, totalBytes, totalBytes );
        }

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        std::string
        FullTextIndexerTSEARCH2::resolveDocumentID( docid_t id )
        {
            fh_stringstream sqlss;
                        
            sqlss << "select url "
                  << " from urlmap "
                  << " where urlid "
                  << " = " << id << "";
            LG_IDX_D << "SQL:" << tostr(sqlss) << endl;

            work trans( *m_connection, "resolve docid" );
            result res;
            res = ExecuteRes( res, trans, tostr( sqlss ).c_str() );
            if( !res.empty() )
            {
                return res[0][0].c_str();
            }
            
            LG_IDX_W << "ResolveDocumentID(error) id:" << id << endl;
            fh_stringstream ess;
            ess << "Failed to resolve document ID:" << id
                << endl;
            Throw_IndexException( tostr( ess ), 0 );
        }

        docNumSet_t&
        FullTextIndexerTSEARCH2::addAllDocumentsMatchingResult( result res, docNumSet_t& output )
        {
            for (result::const_iterator c = res.begin(); c != res.end(); ++c)
            {
                docid_t d = 0;
                c[0].to(d);
                output.insert( d );
            }
                   
            return output;
        }
        
        
        docNumSet_t&
        FullTextIndexerTSEARCH2::addAllDocumentsMatchingTerm(
            const std::string& term_const,
            docNumSet_t& output,
            int limit )
        {
            fh_stringstream queryss;
            queryss << "SELECT * from ftxdocmap where "
                    << "  ftx @@ to_tsquery('default', '" << term_const << "')";
            if( limit )
                queryss << endl << " limit " << limit << " ";
            queryss << " ;";
            nontransaction trans( *m_connection, "query" );
            LG_IDX_D << "SQL:" << tostr(queryss) << endl;
            result res = trans.exec( tostr(queryss) );
            addAllDocumentsMatchingResult( res, output );
            
            return output;
        }


        docNumSet_t&
        FullTextIndexerTSEARCH2::ExecuteTsearch2FullTextQuery( const std::string& queryStringConst,
                                                               docNumSet_t& output,
                                                               int limit )
        {
            string queryString = queryStringConst;
            queryString = Util::replace_all( queryString, " ", "&" );

            LG_IDX_D << "ExecuteTsearch2FullTextQuery() query:" << queryString
                     << endl;
            
            addAllDocumentsMatchingTerm( queryString, output, limit );
            return output;
        }


        void
        FullTextIndexerTSEARCH2::removeDocumentsMatchingRegexFromIndex( const std::string& s,
                                                                        time_t mustBeOlderThan )
        {
            docNumSet_t urlids;
            docNumSet_t docids;

            LG_IDX_D << "removeDocumentsMatchingRegexFromIndex() regex:" << s << endl;
            
            {
                work trans( *m_connection, "select docids" );
                {
                    stringstream sqlss;
                    sqlss << "select d.urlid,d.docid from urlmap u, "
                          << "   ( select * from ftxdocmap union select * from ftxdocmap_multiversion ) d\n"
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
            }

            LG_IDX_D << "removeDocumentsMatchingRegexFromIndex()"
                       << " #urlids:" << urlids.size()
                       << " #docids:" << docids.size()
                       << endl;

            {
                // If there is a fulltext index in the same database we might
                // not be able to remove the URL from the urlmap because it is
                // referenced from ftxdocmap;
                work trans( *m_connection, "recheck urlids" );
                stringstream sqlss;
                sqlss << "select urlid from docmap where urlid in ("
                      << Util::createSeperatedList( urlids.begin(), urlids.end(), ',' )
                      << ");";
                LG_IDX_D << "recheck urlids SQL:" << tostr(sqlss) << endl;
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
                    sqlss << "delete from ftxdocmap where docid in ( " << docidsCommaSep << " );";
                    LG_IDX_D << "ftxdocmap remove sql:" << tostr(sqlss) << endl;
                    Execute( trans, tostr(sqlss) );
                }
                if( !urlids.empty() )
                {
                    stringstream sqlss;
                    sqlss << "delete from urlmap where urlid in ( " << urlidsCommaSep << " );";
                    LG_IDX_D << "urlmap remove sql:" << tostr(sqlss) << endl;
                    Execute( trans, tostr(sqlss) );
                }
                trans.commit();
            }
            LG_IDX_D << "removeDocumentsMatchingRegexFromIndex(done.) regex:" << s << endl;
        }
        
        
        
        /**************************************************/
        /**************************************************/

        FullTextIndexerTSEARCH2::m_isFileNewerThanIndexedVersionCache_t&
        FullTextIndexerTSEARCH2::getIsFileNewerThanIndexedVersionCache()
        {
            return m_isFileNewerThanIndexedVersionCache;
        }
        bool
        FullTextIndexerTSEARCH2::getIndexMethodSupportsIsFileNewerThanIndexedVersion()
        {
            return true;
        }
        bool
        FullTextIndexerTSEARCH2::isFileNewerThanIndexedVersion( const fh_context& c )
        {
            bool ret = true;

            time_t ct = getTimeAttr( c, "mtime", 0 );
            if( !ct )
                return ret;
            LG_IDX_D << "isFileNewerThanIndexedVersion() ct:" << ct << endl;

            string earl = c->getURL();

            m_isFileNewerThanIndexedVersionCache_t::const_iterator ci =
                getIsFileNewerThanIndexedVersionCache().find( earl );
            if( ci != getIsFileNewerThanIndexedVersionCache().end() )
            {
                LG_IDX_D << "PG::isNewer() ct:" << ct << " db-version:" << ci->second << endl;
                return ci->second < ct;
            }

            return ret;
        }


        void
        FullTextIndexerTSEARCH2::setupIsFileNewerThanIndexedVersionCache()
        {
            LG_IDX_D << "setupIsFileNewerThanIndexedVersionCache()" << endl;
            work tr( *m_connection, "setup getIsFileNewerThanIndexedVersionCache" );
            stringstream sqlss;
            sqlss << "select u.url, d.docidtime"
                  << " from urlmap u, ftxdocmap d where d.urlid=u.urlid";
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
                    
                LG_IDX_D << "setupIsFileNewerThanIndexedVersionCache(iter) t:" << t << endl;
                m_isFileNewerThanIndexedVersionCache[ url ] = fromSQLTimeString(t);
            }
            LG_IDX_D << "setupIsFileNewerThanIndexedVersionCache(don)" << endl;
        }
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        void
        FullTextIndexerTSEARCH2::purgeDocumentInstancesOlderThan( time_t t )
        {
            retire_old_docids_from_docmap();
            
            stringstream sqlss;
            sqlss << "delete from ftxdocmap_multiversion where"
                  << "     and docidtime < '" << toSQLTimeString(t) << "'"
                  << "" << endl;
            work w( *m_connection, "delete old document instance data" );
            LG_IDX_D << "purgeDocumentInstancesOlderThan SQL:" << tostr(sqlss) << endl;
            Execute( w, tostr(sqlss) );
            w.commit();
        }
        
    };
};

extern "C"
{
    FERRISEXP_API Ferris::FullTextIndex::MetaFullTextIndexerInterface* Create()
    {
        return new Ferris::FullTextIndex::FullTextIndexerTSEARCH2();
    }
};
