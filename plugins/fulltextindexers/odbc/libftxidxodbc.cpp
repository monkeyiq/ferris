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

    $Id: libftxidxodbc.cpp,v 1.2 2008/12/19 21:30:13 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <typeinfo>
#include <DTL.h>
#include <Ferris/FullTextIndexerMetaInterface.hh>

#include "ODBCIndexHelper_private.hh"

#include <Ferris/Ferris.hh>
#include <Ferris/FullTextIndexer_private.hh>

// fifo
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <string>
using namespace std;
using namespace dtl;

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
        struct getTermIDView
            :
            public Handlable
        {
            struct getTermIDRow
            {
                string term;
                int tid;
    
                getTermIDRow( string term = "", int tid = 0 ) : term(term), tid( tid )
                    {
                    }
            };
            class FERRISEXP_DLLLOCAL BCAgetTermIDRow
            {
            public:
                void operator()(BoundIOs &cols, getTermIDRow &row)
                    {
                        cols["tid"]  >> row.tid;
                        cols["term"] == row.term;
                    }
            };
//             class InsValidate
//                 {
//                 public:

//                     bool operator()(BoundIOs &boundIOs, getTermIDRow& rowbuf)
//                     {
//                         boundIOs.ClearNull(); // clear null on all BoundIO's
//                         boundIOs["tid"].SetNull();
//                         LG_IDX_D << "InsValidate" << endl;

//                         for (BoundIOs::iterator b_it = boundIOs.begin();
//                              b_it != boundIOs.end(); b_it++)
//                         {
//                             LG_IDX_D << " col:" << (*b_it).first << endl;
//                             if( (*b_it).first == "tid" )
//                                 (*b_it).second.SetNull();
//                         }
//                         return true;
//                     }
//                 };
            
            typedef DBView<getTermIDRow> DBV;
            DBV view;
            IndexedDBView<DBV> indexed_view;
            IndexedDBView<DBV>::iterator idxview_it;
            IndexedDBView<DBV>::iterator end_it;
            int sz;
            
            getTermIDView( DBConnection& con, FetchMode fm )
                :
                view( DBV::Args()
                      .tables("lexicon")
                      .bca( BCAgetTermIDRow() )
                      .conn( con )
//                      .InsValidate( InsValidate() )
                    ),
                indexed_view(
                    IndexedDBView<DBV>::Args().view( view )
                    .indexes( "UNIQUE PrimaryIndex; term;" )
                    .bound( BOUND )
                    .key_mode( USE_PK_FIELDS_ONLY )
                    .fetch_mode( fm )
                    .fetch_records( 1000 )),
                idxview_it( indexed_view.end() ),
                end_it( idxview_it )
                {
                    sz = indexed_view.size();
                }
            
            long find( const std::string& term )
                {
                    idxview_it = indexed_view.find( term );
                    if( end_it != idxview_it )
                        return idxview_it->tid;
                    return -1;
                }
            long ensure( const std::string& term )
                {
                    idxview_it = indexed_view.find( term );
                    if( end_it != idxview_it )
                        return idxview_it->tid;

                    // Finally, insert a new item into the container
                    pair<IndexedDBView<DBV>::iterator, bool> p
                        = indexed_view.insert(
//                            getTermIDRow( term, -1 ));
                            getTermIDRow( term, sz++ ));
//                    LG_IDX_D << "inserted new term:" << term << " new tid:" << p.first->tid << endl;
                    return p.second ? p.first->tid : -1;
                }
                
        };
        FERRIS_SMARTPTR( getTermIDView, fh_getTermIDView );

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        class FERRISEXP_DLLLOCAL FullTextIndexerODBC
            :
            public ::Ferris::Index::ODBCIndexHelper< MetaFullTextIndexerInterface,
                                                     docid_t >
        {
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

            FullTextIndexerODBC();
            virtual ~FullTextIndexerODBC();
            

            virtual void reindexingDocument( fh_context c, docid_t docid );
            template <class DocTermsClass>
            void
            addToIndexDocTermsClass( fh_context c,
                                     fh_docindexer di );
            

        };

        
//         static const char* CFG_ODBCIDX_DSN_K          = "cfg-odbcidx-dsn";
//         static const char* CFG_ODBCIDX_USER_K         = "cfg-odbcidx-user";
//         static const char* CFG_ODBCIDX_DBTYPE_K       = "cfg-odbcidx-dbtype";
//         static const char* CFG_ODBCIDX_DBTYPE_DEFAULT = "";
//         static const char* CFG_ODBCIDX_USEDTLBULK_K   = "cfg-odbcidx-use-dtl-bulk";

        /************************************************************/
        /************************************************************/
        /************************************************************/

        FullTextIndexerODBC::FullTextIndexerODBC()
        {
        }

        FullTextIndexerODBC::~FullTextIndexerODBC()
        {
        }
        
        void
        FullTextIndexerODBC::Setup()
        {
            ODBCSetup();
        }

        void
        FullTextIndexerODBC::CreateIndexBeforeConfig( fh_context c,
                                                      bool caseSensitive,
                                                      bool dropStopWords,
                                                      StemMode stemMode,
                                                      const std::string& lex_class,
                                                      fh_context md )
        {
        }
        
        
        void
        FullTextIndexerODBC::CreateIndex( fh_context c,
                                          bool caseSensitive,
                                          bool dropStopWords,
                                          StemMode stemMode,
                                          const std::string& lex_class,
                                          fh_context md )
        {
            ODBCCreateIndex( c, md );
            
            // auto_increment is non SQL92
            static const char* commands[] =
                {
                    "CREATE TABLE docmap ("
                    "  URL varchar(200) NOT NULL default '',"
                    "  docid int NOT NULL default 0,"
                    "  PRIMARY KEY (docid)"
                    ")",
            
                    "CREATE TABLE docterms ("
                    "  docid int NOT NULL default 0,"
                    "  tid int NOT NULL default 0,"
                    "  freq int NOT NULL default 0,"
                    "  PRIMARY KEY (docid,tid)"
                    ")",

                    "CREATE TABLE lexicon ("
                    "  term varchar(100) NOT NULL default '',"
                    "  tid int NOT NULL default 0 unique,"
                    "  PRIMARY KEY (term)"
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
            getConnection().CommitAll();
        }

        void
        FullTextIndexerODBC::CommonConstruction()
        {
        }

        fh_getTermIDView
        FullTextIndexerODBC::getLexicon()
        {
            if( !isBound( m_lexicon ) )
            {
                LG_IDX_D << "addToIndexDocTermsClass(getting lexicon)" << endl;
                m_lexicon = new getTermIDView( getConnection(), getFetchMode() );
                LG_IDX_D << "addToIndexDocTermsClass(got lexicon)" << endl;
            }
            return m_lexicon;
        }
        
        
//         long getDocumentID( DBConnection& con,
//                             const std::string& earl )
//         {
//             fh_stringstream sqlss;
//             sqlss << "select * from docmap where URL = '" << earl << "'";
//             DynamicDBView<> view( tostr(sqlss), "", DYNAMICDBVIEW_CRUFT, con );
            
//             long docid = -1;
//             for( DynamicDBView<>::sql_iterator print_it = view.begin();
//                  print_it != view.end(); print_it++)
//             {
//                 variant_row row = *print_it;
//                 string docidstr = row["docid"];
//                 docid = toType<long>( docidstr );
//                 break;
//             }

//             return docid;
//         }

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/


        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        struct FERRISEXP_DLLLOCAL DocTermsView
            :
            public Handlable
        {
        public:
            struct myRow
            {
                int docid;
                int tid;
                int freq;
    
                myRow( int docid = 0, int tid = 0, int freq = 0 )
                    : docid(docid), tid( tid ), freq(freq)
                    {
                    }
            };
            class FERRISEXP_API myBCA
            {
            public:
                void operator()(BoundIOs &cols, myRow &row)
                    {
                        cols["docid"] == row.docid;
                        cols["tid"] == row.tid;
                        cols["freq"] == row.freq;
                    }
            };
            
            typedef DBView<myRow> DBV;
            DBV view;
            IndexedDBView<DBV> indexed_view;
            IndexedDBView<DBV>::iterator idxview_it;
            IndexedDBView<DBV>::iterator end_it;
            
            DocTermsView( DBConnection& con )
                :
                view( DBV::Args()
                      .tables("docterms")
                      .bca( myBCA() )
                      .conn( con )
                    ),
                indexed_view(
                    view,
                    "UNIQUE PrimaryIndex; docid, tid;", 
                    BOUND, USE_ALL_FIELDS ),
                idxview_it( indexed_view.end() ),
                end_it( idxview_it )
                {
                }
            
            void set( long docid, long tid, long freq )
                {
                    idxview_it = indexed_view.find( docid, tid );
                    if( end_it != idxview_it )
                    {
//                        LG_IDX_D << "giving larger freq to termid:" << tid << endl;
                        myRow replacement = *idxview_it;
                        replacement.freq = freq;
                        indexed_view.replace(idxview_it, replacement);
                    }
                    else
                    {
                        // Finally, insert a new item into the container
                        pair<IndexedDBView<DBV>::iterator, bool> p
                            = indexed_view.insert(
                                myRow( docid, tid, freq ));
                    }
                }
                
        };
        FERRIS_SMARTPTR( DocTermsView, fh_DocTermsView );

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
//         long getTermID( DBConnection& con, const std::string& term )
//         {
//             long termid = -1;

//             cout << "select * from lexicon where term = '" << term << "' limit 1;" << endl;
            
//             // fancy dbview with custom classes
// //             DBView<getTermIDRow, getTermIDParamObj>
// //                 view("lexicon",	BCAgetTermIDRow(),
// //                      "WHERE term = (?) "
// //                      "ORDER BY tid", BPAgetTermIDParamObj(),
// //                      DefaultSelValidate<getTermIDRow>(),
// //                      DefaultInsValidate<getTermIDRow>(),
// //                      DEFAULT_IO_HANDLER<getTermIDRow, getTermIDParamObj>(),
// //                      con );

// //             DBView<getTermIDRow, getTermIDParamObj>::select_iterator read_it = view.begin();
// //             read_it.Params().term = term;

// //             for ( ; read_it != view.end(); read_it++)
// //             {
// // //                LG_IDX_D << read_it->tid << endl;
// //                 termid = read_it->tid;
// //                 break;
// //             }
            
// //             // DynamicDBView
// //             fh_stringstream sqlss;
// //             sqlss << "select * from lexicon where term = '" << term << "' limit 1;";
// //             DynamicDBView<> view( DynamicDBView<>::Args().tables(tostr(sqlss)).conn(con) );
            
// //             for( DynamicDBView<>::sql_iterator print_it = view.begin();
// //                  print_it != view.end(); print_it++)
// //             {
// //                 variant_row row = *print_it;
// //                 string t = row["tid"];
// //                 termid = toType<long>( t );
// //                 LG_IDX_D << "term -> " << t
// //                      << " num:" << termid << endl;
// //             }

// //             // DBView<getTermIDRow>
// //             fh_stringstream sqlss;
// //             sqlss << "select * from lexicon where term = '" << term << "' limit 1;";
// //             DBView<getTermIDRow> view( DBView<getTermIDRow>::Args().tables(tostr(sqlss))
// //                                        .conn(con)
// //                                        .bca( BCAgetTermIDRow() ) );
            
// //             for( DBView<getTermIDRow>::sql_iterator print_it = view.begin();
// //                  print_it != view.end(); print_it++)
// //             {
// //                 getTermIDRow row = *print_it;
// //                 termid = row.tid;
// //                 LG_IDX_D << "term -> " << term
// //                      << " num:" << termid << endl;
// //             }



            
//             static getTermIDView obj( con );
//             termid = obj.find( term );

            
// //             fh_stringstream sqlss;
// //             sqlss << "select * from lexicon where term = '" << term << "' limit 1;";
// //             DynamicDBView<> view( tostr(sqlss), "" , DYNAMICDBVIEW_CRUFT, con );
            
// //             for( DynamicDBView<>::sql_iterator print_it = view.begin();
// //                  print_it != view.end(); print_it++)
// //             {
// //                 variant_row row = *print_it;
// //                 string t = row["tid"];
// //                 termid = toType<long>( t );
// //                 LG_IDX_D << "term -> " << t
// //                      << " num:" << termid << endl;
// //             }


//             LG_IDX_D << "term:" << term
//                  << " num:" << termid << endl;
            
//             return termid;
//         }


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
            DocTermsUpdaterNULL(  DBConnection& )
                {
                }
            void set( long docid, long termid, long termCount )
                {
                }
            void comit( FullTextIndexerODBC* )
                {
                }
        };
        struct FERRISEXP_DLLLOCAL DocTermsUpdaterMySQL
        {
            string batchInsertPath;
            fh_ofstream batchInsert;
            
            DocTermsUpdaterMySQL( DBConnection& )
//                 :
//                 batchInsert( "/tmp/.ferris-batch-sql-tmpfile" )
                {
                    batchInsertPath = "/tmp/.ferris-batch-sql-tmpfile";
                    unlink( batchInsertPath.c_str() );
//                  mkfifo( batchInsertPath.c_str(), S_IRUSR | S_IWUSR | O_NONBLOCK );
//                  int batchInsertFD = open( batchInsertPath.c_str(), O_WRONLY | O_NONBLOCK );
//                  fh_ostream batchInsert = Ferris::Factory::MakeFdOStream( batchInsertFD );
                    batchInsert.open( batchInsertPath );
                }
            void set( long docid, long termid, long termCount )
                {
                    batchInsert << docid << ","
                                << termid << ","
                                << termCount << "\n";
                }
            void comit( FullTextIndexerODBC* ftio )
                {
                    LG_IDX_D << "mysql comit( starting )" << endl;
                    batchInsert << flush;
                    batchInsert.close();
                    
                    fh_stringstream insertss;
                    insertss << "LOAD DATA LOCAL INFILE\n \'" << batchInsertPath << "\'\n"
                             << " into TABLE docterms "
                             << " FIELDS TERMINATED BY ',' ENCLOSED BY '' ESCAPED BY '\\\\' "
                             << endl;
                    ftio->Execute( tostr( insertss ) );
                    ftio->getConnection().CommitAll();
                    LG_IDX_D << "mysql comit( ending )" << endl;
                }
        };

        
//         // COPY TO/FROM only work at the psql level, not ODBC
//         struct DocTermsUpdaterPostgreSQL
//         {
//             fh_stringstream batchInsert;
            
//             DocTermsUpdaterPostgreSQL( DBConnection& )
//                 {
//                     batchInsert << "copy docterms from STDIN WITH delimiter ',';" << endl;
//                 }
//             void set( long docid, long termid, long termCount )
//                 {
//                     batchInsert << docid << ","
//                                 << termid << ","
//                                 << termCount << "\n";
//                 }
//             void comit( FullTextIndexerODBC* ftio )
//                 {
//                     batchInsert << "\\." << flush;
//                     LG_IDX_D << "batchInsert:" << tostr( batchInsert ) << endl;
//                     ftio->Execute( tostr( batchInsert ) );
//                     ftio->getConnection().CommitAll();
//                 }
//         };


        static const int m_rows_sz = 1000;
        struct FERRISEXP_DLLLOCAL DocTermsUpdaterGenericDTLBulk
        {
//             enum
//             {
//                 m_rows_sz = 1000
//             };
            DocTermsView::myRow m_rows[m_rows_sz+1];
            int m_rows_idx;
            DBConnection& m_con;
            
            DocTermsUpdaterGenericDTLBulk( DBConnection& con )
                :
                m_con( con ),
                m_rows_idx( 0 )
                {
                }

            void putrecords( DBConnection& con )
                {
                    DBView<DocTermsView::myRow> view( DBView<DocTermsView::myRow>::Args()
                                                      .tables("docterms")
                                                      .bca( DocTermsView::myBCA() )
                                                      .conn( con )
                        );
                    DBView<DocTermsView::myRow>::insert_iterator ins_it = view;
                    bulk_copy(&m_rows[0], &m_rows[ m_rows_idx ], ins_it);
                    m_rows_idx = 0;
                }
            
            
            void set( long docid, long termid, long termCount )
                {
                    if( m_rows_idx == m_rows_sz )
                        putrecords( m_con );
                    
                    m_rows[ m_rows_idx ].docid = docid;
                    m_rows[ m_rows_idx ].tid   = termid;
                    m_rows[ m_rows_idx ].freq  = termCount;
                    ++m_rows_idx;
                }
            void comit( FullTextIndexerODBC* ftio )
                {
                    putrecords( m_con );
                    ftio->getConnection().CommitAll();
                }
        };

        struct FERRISEXP_DLLLOCAL DocTermsUpdaterGenericSingle
        {
            fh_DocTermsView docterms;
            DocTermsUpdaterGenericSingle( DBConnection& con )
                {
                    docterms = new DocTermsView( con );
                }
            
            void set( long docid, long termid, long termCount )
                {
                    docterms->set( docid, termid, termCount );
                }
            void comit( FullTextIndexerODBC* ftio )
                {
                    ftio->getConnection().CommitAll();
                }
        };

        void
        FullTextIndexerODBC::reindexingDocument( fh_context c, docid_t docid )
        {
            LG_IDX_D << "FullTextIndexerODBC::reindexingDocument() c:" << c->getURL() << endl;
            Execute( "delete from docterms where docid = " + tostr(docid) );
        }
        

        template <class DocTermsClass>
        void
        FullTextIndexerODBC::addToIndexDocTermsClass( fh_context c,
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
//             // check if the document is already in the index, if it is
//             // we have to remove it from the join table and use its
//             // document ID, otherwise we have to create a new entry
//             // in the docmap table and retain the docid for inserting
//             // entries into the join table.
//             long docid = getDocumentID( getConnection(), c->getURL() );
//             {
//                 if( docid >= 0 )
//                 {
//                     Execute( "delete from docterms where docid = " + tostr(docid) );
//                 }
//                 else
//                 {
//                     {
//                         fh_stringstream sqlss;
//                         sqlss << "select count(docid) as n from docmap";
//                         DynamicDBView<> view( tostr(sqlss), "", DYNAMICDBVIEW_CRUFT,
//                                               getConnection() );
            
//                         for( DynamicDBView<>::sql_iterator print_it = view.begin();
//                              print_it != view.end(); print_it++)
//                         {
//                             variant_row row = *print_it;
//                             string t = row["n"];
//                             docid = toType<long>( t ) + 1;
//                         }
//                     }

//                     fh_stringstream insertss;
//                     insertss << "insert into docmap (URL,docid) values ('"
//                              << c->getURL() << "'," << docid << ")";
//                     Execute( tostr( insertss ) );

//                     docid = getDocumentID( getConnection(), c->getURL() );
//                 }
//             }
            LG_IDX_D << "addToIndexDocTermsClass(got docid) c:" << c->getURL() << endl;

            int termCount = 1;
            UniqSortedTerms uniqTerms( iss, di,
                                       isCaseSensitive,
                                       stemmer,
                                       getStopWords(),
                                       DropStopWords );
            LG_IDX_D << "addToIndexDocTermsClass(got uniqsorted) c:" << c->getURL() << endl;
            fh_getTermIDView lexicon = getLexicon();
            
//            while( !(s = di->getToken( iss )).empty() )
            while( uniqTerms.next( s, termCount ) )
            {
//                LG_IDX_D << "got token:" << s << endl;

                // get the lexicon.tid either by lookup or by creating it.
                long termid = lexicon->ensure( s );
//                long termid = getTermID( getConnection(), s );
//                 if( termid < 0 )
//                 {
//                     fh_stringstream insertss;
//                     insertss << "insert into lexicon (term) values ('" << s << "');";
//                     Execute( tostr( insertss ) );
//                     termid = getTermID( getConnection(), s );
// //                    LG_IDX_D << "inserted term...id:" << termid << endl;
//                 }
//                 else
//                 {
// //                    LG_IDX_D << "found term...id:" << termid << endl;
//                 }
                

//                 LG_IDX_D << "doc:" << docid << " term:" << termid << endl;

                // bump the join table count
//                docterms->set( docid, termid, termCount );

                dtc.set( docid, termid, termCount );
//                 cout << docid << ","
//                      << termid << ","
//                      << termCount << "\n";
                
                
//                 {
//                     try
//                     {
//                         fh_stringstream insertss;
//                         insertss << "insert into docterms (docid,tid,freq) values ("
//                                  << docid << ","
//                                  << termid << ","
//                                  << 1 << ");";
//                         Execute( tostr( insertss ) );
//                     }
//                     catch( exception& e )
//                     {
//                         fh_stringstream updatess;
//                         updatess << "update docterms set freq=freq+1 where docid=" << docid
//                                  << " and tid=" << termid
//                                  << endl;
//                         Execute( tostr( updatess ) );
//                     }
//                 }
                
//                 streamsize bdone = di->getBytesCompleted();
//                 if( bdone % 16*1024 == 0 )
//                 {
//                     di->getProgressSig().emit( c, bdone, totalBytes );
//                 }
            }

            dtc.comit( this );
        }
        

        void
        FullTextIndexerODBC::addToIndex( fh_context c,
                                         fh_docindexer di )
        {
            string driver = getDBType();

            LG_IDX_D << "Adding document:" << c->getURL()
                 << " driver:" << driver
                 << endl;

            driver = tolowerstr()( driver );
            
            if( driver == "mysql" )
                addToIndexDocTermsClass<DocTermsUpdaterMySQL>( c, di );
//             if( driver == "postgresql" || driver == "pg" )
//                 addToIndexDocTermsClass<DocTermsUpdaterPostgreSQL>( c, di );
            else if( useDTLBulk() )
                addToIndexDocTermsClass<DocTermsUpdaterGenericDTLBulk>( c, di );
            else
                addToIndexDocTermsClass<DocTermsUpdaterGenericSingle>( c, di );
        }

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        std::string
        FullTextIndexerODBC::resolveDocumentID( docid_t id )
        {
            return ODBCResolveDocumentID( id );
            
//             fh_stringstream sqlss;
//             sqlss << "select URL from docmap where docid = " << id << "";
//             DynamicDBView<> view( tostr(sqlss), "", DYNAMICDBVIEW_CRUFT,
//                                   getConnection() );

//             for( DynamicDBView<>::sql_iterator print_it = view.begin();
//                  print_it != view.end(); print_it++)
//             {
//                 variant_row row = *print_it;
//                 string url = row["URL"];
//                 return url;
//             }

//             fh_stringstream ess;
//             ess << "Failed to resolve document ID:" << id
//                 << " in database:" << getConnection().GetDSN() << endl;
//             Throw_FullTextIndexException( tostr( ess ), 0 );
        }
        
        docNumSet_t&
        FullTextIndexerODBC::addAllDocumentsMatchingTerm(
            const std::string& term,
            docNumSet_t& output,
            int limit )
        {
            fh_stringstream sqlss;
            sqlss << "select d.docid as docid from docterms d, lexicon l where "
                  << " d.tid = l.tid "
                  << " and l.term = '" << term << "' ";
            DynamicDBView<> view( tostr(sqlss), "", DYNAMICDBVIEW_CRUFT,
                                  getConnection() );

            for( DynamicDBView<>::sql_iterator print_it = view.begin();
                 print_it != view.end(); print_it++)
            {
                variant_row row = *print_it;
                string t = row["docid"];
                output.insert( toint( t ) );
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
        return new Ferris::FullTextIndex::FullTextIndexerODBC();
    }
};
