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

    $Id: libftxidxqtsql.cpp,v 1.2 2008/12/19 21:30:13 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <typeinfo>
#include <Ferris/FullTextIndexerMetaInterface.hh>

#include "QtSQLIndexHelper_private.hh"
#include <Ferris/Ferris.hh>
#include <Ferris/FullTextIndexer_private.hh>

// fifo
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <string>
using namespace std;

#define DYNAMICDBVIEW_CRUFT "", DefaultSelValidate<variant_row>(), \
        DefaultInsValidate<variant_row>(), DEFAULT_IO_HANDLER<variant_row, variant_row>()

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


namespace Ferris
{
    /********************************************************************/
    /********************************************************************/
    /********************************************************************/
    
    namespace FullTextIndex 
    {
        static QString q( std::string s )
        {
            return s.c_str();
        }

        struct getTermIDView
            :
            public Handlable
        {
            QSqlDatabase& m_db;
            int m_sz;
            
            getTermIDView( QSqlDatabase& db )
                : m_db( db )
                , m_sz( -1 )
                {
                }
            
            long find( const std::string& term )
                {
                    QSqlQuery query( m_db );
                    query.prepare( "select * from lexicon where term = :term" );
                    query.bindValue( ":term", term.c_str() );
                    query.exec();
                    while (query.next())
                    {
                        QSqlRecord rec = query.record();
                        string tid = tostr(query.value(rec.indexOf("tid")).toString());
                        return toint(tid);
                    }
                    return -1;
                }
            long ensure( const std::string& term )
                {
                    long tid = find( term );
                    if( tid >= 0 )
                        return tid;

                    QSqlQuery query( m_db );
                    query.prepare( "insert into lexicon (term) values (:term)");
                    query.bindValue( ":term", term.c_str() );
                    query.exec();
                    return find( term );
                }
                
        };
        FERRIS_SMARTPTR( getTermIDView, fh_getTermIDView );

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        class FERRISEXP_DLLLOCAL FullTextIndexerQTSQL
            :
            public ::Ferris::Index::QTSQLIndexHelper< MetaFullTextIndexerInterface,
                                                     docid_t >
        {
            typedef ::Ferris::Index::QTSQLIndexHelper< MetaFullTextIndexerInterface,
                                                       docid_t > _Base;
            fh_getTermIDView  m_lexicon;
            
        protected:

            fh_getTermIDView getLexicon();

            
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

            FullTextIndexerQTSQL();
            virtual ~FullTextIndexerQTSQL();
            

            virtual void reindexingDocument( fh_context c, docid_t docid );
            template <class DocTermsClass>
            void
            addToIndexDocTermsClass( fh_context c,
                                     fh_docindexer di );
            

        };

        

        /************************************************************/
        /************************************************************/
        /************************************************************/

        FullTextIndexerQTSQL::FullTextIndexerQTSQL()
        {
        }

        FullTextIndexerQTSQL::~FullTextIndexerQTSQL()
        {
        }
        
        void
        FullTextIndexerQTSQL::Setup()
        {
            QTSQLSetup();
        }

        void
        FullTextIndexerQTSQL::CreateIndexBeforeConfig( fh_context c,
                                                      bool caseSensitive,
                                                      bool dropStopWords,
                                                      StemMode stemMode,
                                                      const std::string& lex_class,
                                                      fh_context md )
        {
        }
        
        
        void
        FullTextIndexerQTSQL::CreateIndex( fh_context c,
                                          bool caseSensitive,
                                          bool dropStopWords,
                                          StemMode stemMode,
                                          const std::string& lex_class,
                                          fh_context md )
        {
            QTSQLCreateIndex( c, md );
            
            // auto_increment is non SQL92
            static const char* commands[] =
                {
                    "CREATE TABLE docmap ("
                    "  URL varchar(1024) NOT NULL default '',"
                    "  docid INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "  mtime timestamp"
                    ")",
            
                    "CREATE TABLE docterms ("
                    "  docid int NOT NULL default 0,"
                    "  tid int NOT NULL default 0,"
                    "  freq int NOT NULL default 0,"
                    "  PRIMARY KEY (docid,tid)"
                    ")",

                    "CREATE TABLE lexicon ("
                    "  tid INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "  term varchar(100) NOT NULL unique"
                    ")",

                    0
                };

            Execute( commands );

            
            /**
             * Create the index for using docterms table effectively.
             */
            static const char* defaultindexing_commands[] = {
                "CREATE INDEX mydocterms_tididx ON docterms (tid)",
                0
            };
            const char** indexing_commands = defaultindexing_commands;

            Execute( indexing_commands );
            m_db.commit();
        }

        void
        FullTextIndexerQTSQL::CommonConstruction()
        {
        }

        fh_getTermIDView
        FullTextIndexerQTSQL::getLexicon()
        {
            if( !isBound( m_lexicon ) )
            {
                LG_IDX_D << "addToIndexDocTermsClass(getting lexicon)" << endl;
                m_lexicon = new getTermIDView( m_db );
                LG_IDX_D << "addToIndexDocTermsClass(got lexicon)" << endl;
            }
            return m_lexicon;
        }
        
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        struct FERRISEXP_DLLLOCAL DocTermsView
            :
            public Handlable
        {
        public:

            QSqlDatabase& m_db;
            
            
            DocTermsView( QSqlDatabase& db )
                : m_db( db )
                {
                }
            
            void set( long docid, long tid, long freq )
                {
                    try 
                    {
                        QSqlQuery query( m_db );
                        query.prepare( "delete from docterms where docid = :docid and tid = :tid" );
                        query.bindValue( ":docid", q(tostr(docid)) );
                        query.bindValue( ":tid",   q(tostr(tid)) );
                        query.exec();
                    }
                    catch( exception& e )
                    {
                    }
                    QSqlQuery query( m_db );
                    query.prepare( "insert into docterms (docid,tid,freq) values (:docid,:tid,:freq)" );
                    query.bindValue( ":docid", q(tostr(docid)) );
                    query.bindValue( ":tid",   q(tostr(tid)) );
                    query.bindValue( ":freq",  q(tostr(freq)) );
                    query.exec();
                }
                
        };
        FERRIS_SMARTPTR( DocTermsView, fh_DocTermsView );

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        


        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        /*** We have policy classes to manage the updating of the join table*/
        /*** this allows for batch updates of the join table for databases  */
        /*** which explicit support has been given to. eg MySQL manual says */
        /*** that using LOAD DATA INFILE can give a 20x speedup over using  */
        /*** a large INSERT INTO clause *************************************/
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        struct FERRISEXP_DLLLOCAL DocTermsUpdaterNULL
        {
            DocTermsUpdaterNULL( QSqlDatabase& )
                {
                }
            void set( long docid, long termid, long termCount )
                {
                }
            void comit( FullTextIndexerQTSQL* )
                {
                }
        };

        struct FERRISEXP_DLLLOCAL DocTermsUpdaterGenericSingle
        {
            fh_DocTermsView docterms;
            DocTermsUpdaterGenericSingle( QSqlDatabase& db )
                {
                    docterms = new DocTermsView( db );
                }
            
            void set( long docid, long termid, long termCount )
                {
                    docterms->set( docid, termid, termCount );
                }
            void comit( FullTextIndexerQTSQL* ftio )
                {
                }
        };

        void
        FullTextIndexerQTSQL::reindexingDocument( fh_context c, docid_t docid )
        {
            LG_IDX_D << "FullTextIndexerQTSQL::reindexingDocument() c:" << c->getURL() << endl;
            Execute( "delete from docterms where docid = " + tostr(docid) );
        }
        

        template <class DocTermsClass>
        void
        FullTextIndexerQTSQL::addToIndexDocTermsClass( fh_context c,
                                                      fh_docindexer di )
        {
            LG_IDX_D << "addToIndexDocTermsClass() c:" << c->getURL() << endl;
            DocTermsClass dtc( getConnection() );
            string s;
            fh_istream iss;
            try
            {
                fh_attribute a = c->getAttribute( "as-text" );
                iss = a->getIStream();
            }
            catch( exception& e )
            {
                iss = c->getIStream();
            }
            streamsize bytesDone    = 0;
            streamsize signalWindow = 0;
            streamsize totalBytes   = toType<streamsize>(getStrAttr( c, "size", "0" ));

            bool isCaseSensitive = this->isCaseSensitive();
            bool DropStopWords   = getDropStopWords();
            StemMode stemmer     = getStemMode();

            long docid = obtainDocumentID( c );
            LG_IDX_D << "addToIndexDocTermsClass(got docid) c:" << c->getURL() << endl;
            {
                stringstream sqlss;
                sqlss << "update docmap "
                      << " set mtime = " << Time::getTime()
                      << " where docid = " << docid << endl;
                Execute(sqlss);
            }
            
            
            int termCount = 1;
            UniqSortedTerms uniqTerms( iss, di,
                                       isCaseSensitive,
                                       stemmer,
                                       getStopWords(),
                                       DropStopWords );
            LG_IDX_D << "addToIndexDocTermsClass(got uniqsorted) c:" << c->getURL() << endl;
            fh_getTermIDView lexicon = getLexicon();
            
            while( uniqTerms.next( s, termCount ) )
            {

                // get the lexicon.tid either by lookup or by creating it.
                long termid = lexicon->ensure( s );

                dtc.set( docid, termid, termCount );
            }

            dtc.comit( this );
        }
        

        void
        FullTextIndexerQTSQL::addToIndex( fh_context c,
                                         fh_docindexer di )
        {
            string driver = getDBType();

            LG_IDX_D << "Adding document:" << c->getURL()
                 << " driver:" << driver
                 << endl;

            driver = tolowerstr()( driver );
            
            addToIndexDocTermsClass< DocTermsUpdaterGenericSingle >( c, di );
        }

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        std::string
        FullTextIndexerQTSQL::resolveDocumentID( docid_t id )
        {
            return _Base::resolveDocumentID( id );
        }
        
        docNumSet_t&
        FullTextIndexerQTSQL::addAllDocumentsMatchingTerm(
            const std::string& term,
            docNumSet_t& output,
            int limit )
        {
            LG_EAIDX_D << "addAllDocumentsMatchingTerm() term:" << term << endl;
            
            stringstream sqlss;
            sqlss << "select d.docid as docid from docterms d, lexicon l where "
                  << " d.tid = l.tid "
                  << " and l.term = '" << term << "' ";
            if( limit > 0 )
                sqlss << "\n limit " << limit;

            LG_EAIDX_D << "SQL:" << sqlss.str() << endl;
            QSqlQuery query = Execute( sqlss );
            while (query.next())
            {
                QSqlRecord rec = query.record();
                long docid = toint(tostr(query.value(rec.indexOf("docid")).toString()));
                output.insert( docid );
            }
            
            return output;
        }
        
        /**************************************************/
        /**************************************************/

        
    };
};

extern "C"
{
    FERRISEXP_API Ferris::FullTextIndex::MetaFullTextIndexerInterface* Create()
    {
        return new Ferris::FullTextIndex::FullTextIndexerQTSQL();
    }
};
