/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2006 Ben Martin

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

    $Id: libeaindexxapian.cpp,v 1.1 2006/12/07 06:49:43 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 *
 *
 */

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/version.hpp>

#include <xapian.h>
#include <xapian/queryparser.h>

#include "EAIndexerMetaInterface.hh"
#include "EAIndexer.hh"
#include "EAQuery.hh"
#include "General.hh"
#include "FactoriesCreationCommon_private.hh"
#include "Ferris/Ferris_private.hh"
#include "Ferris/Medallion.hh"
#include "Ferris/Iterator.hh"

#include "EAIndexerSQLCommon_private.hh"

#include "../../fulltextindexers/xapian/libferrisxapianeashared.hh"

// #define DEBUG_ADDING_TO_INDEX 1

using namespace Xapian;
using namespace std;

namespace Ferris
{
    namespace EAIndex 
    {
        static const char* CFG_IDX_DBNAME_K = "cfg-idx-dbname";
        static const char* CFG_IDX_DBNAME_DEF = "ferriseaindex";
        static const char* CFG_XAPIANIDX_DBNAME_K          = "cfg-xapianidx-dbname";
        static const char* CFG_XAPIANIDX_DBNAME_DEFAULT    = "fxapianidx.fidx";
        
        static string quoteStr( const std::string& s )
        {
            return "'" + s + "'";
        }

        
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/

        
        class FERRISEXP_DLLLOCAL EAIndexerXapian
            :
            public MetaEAIndexerInterface
        {
            typedef EAIndexerXapian _Self;
            
            WritableDatabase m_database;
            typedef map< int, string > m_docidmap_t;
            m_docidmap_t m_docidmap;
            typedef map< string, int > m_revdocidmap_t;
            m_revdocidmap_t m_revdocidmap;
            int m_docidmap_nextid;
            
            int         m_filesIndexedCount;

            //////////////////////////////////////////////

            string getDBPath();
            
            
            //////////////////////////////////////////////
            //////////////////////////////////////////////
            //////////////////////////////////////////////
            

            
        protected:
            
            virtual void Setup();
            virtual void CreateIndex( fh_context c,
                                      fh_context md );
            virtual void CommonConstruction();

            virtual std::string asString( const std::string& rawValue,
                                          const std::string& rawEAName,
                                          bool isCaseSensitive,
                                          AttrType_t att, bool quote );
            
            virtual std::string asString( IndexableValue& v, AttrType_t att, bool quote );
            virtual std::string asString( IndexableValue& v, AttrType_t att )
                {
                    return asString( v, att, true );
                }
            virtual std::string asString( IndexableValue& v )
                {
                    return asString( v, v.getAttrTypeID() );
                }
            
            

            
        public:
            EAIndexerXapian();
            virtual ~EAIndexerXapian();

            virtual void sync();
            virtual void prepareForWrites( int f );


            virtual void addToIndex( fh_context c,
                                     fh_docindexer di );
            void AddSPARQLOp( fh_stringstream& sqlss,
                              Query& xq,
                              const std::string& eaname,
                              const std::string& opcode,
                              IndexableValue& iv,
                              AttrType_t att,
                              bool negating );
            AttrType_t
            SQLColumnTypeToAttrType( const std::string& coltype,
                                     IndexableValue& iv );
            void AddSPARQLOpHeur( fh_stringstream& sqlss,
                               const std::string& eaname,
                               const std::string& opcode,
                               IndexableValue& iv,
                               bool negating 
                                );
            virtual docNumSet_t& ExecuteQuery( fh_context q,
                                               docNumSet_t& output,
                                               fh_eaquery qobj,
                                               int limit = 0 );

            string BuildQuery_getToken( fh_context q );
            pair< fh_context, fh_context > BuildQuery_getLeftAndRightContexts( fh_context q );
            string  BuildQuery_getEAName( fh_context q );

            docNumSet_t& BuildQuery_LogicalCombine(
                Xapian::Query::op xapianOpCode,
                fh_context q,
                docNumSet_t& output,
                fh_eaquery qobj,
                Query& xq,
                fh_stringstream& sqlss,
                bool& queryHasTimeRestriction,
                stringset_t& eanamesUsed,
                bool negating );
            
            virtual docNumSet_t& BuildQuery(
                fh_context q,
                docNumSet_t& output,
                fh_eaquery qobj,
                Query& xq,
                fh_stringstream& sqlss,
                bool& queryHasTimeRestriction,
                stringset_t& eanamesUsed,
                bool negating );

            string getURLIDNodePredicates( bool& queryHasTimeRestriction );
            virtual docNumSet_t& BuildQueryXapian(
                fh_context q,
                docNumSet_t& output,
                fh_eaquery qobj,
                Query& xq,
                std::stringstream& SQLHeader,
                std::stringstream& SQLWherePredicates,
                std::stringstream& SQLTailer,
                bool& queryHasTimeRestriction,
                stringset_t& eanamesUsed );
            
            
            virtual std::string resolveDocumentID( docid_t );

            
            /************************************************************/
            /************************************************************/
            /************************************************************/

            virtual bool getIndexMethodSupportsIsFileNewerThanIndexedVersion();
            virtual bool isFileNewerThanIndexedVersion( const fh_context& c );
            

            docNumSet_t& addAllDocumentsMatchingMSet( MSet& matches, docNumSet_t& output );
        };

        /************************************************************/
        /************************************************************/
        /************************************************************/

        EAIndexerXapian::EAIndexerXapian()
            :
            m_docidmap_nextid( 1 )
        {
        }

        
        EAIndexerXapian::~EAIndexerXapian()
        {
        }

        string
        EAIndexerXapian::getDBPath()
        {
            string dbname = this->getConfig( CFG_XAPIANIDX_DBNAME_K,
                                             CFG_XAPIANIDX_DBNAME_DEFAULT );
            fh_stringstream ss;
            ss << getPath() << "/" << dbname;
            return tostr(ss);
        }
        
        void
        EAIndexerXapian::Setup()
        {
            LG_EAIDX_D << "EAIndexerXapian::Setup()" << endl;

            string dbpath = getDBPath();
            string db_name      = this->getConfig( CFG_IDX_DBNAME_K,      CFG_IDX_DBNAME_DEF, true );

            try {
                // Open the database
                m_database = WritableDatabase( dbpath, DB_CREATE_OR_OPEN );
            } catch (const Error &error) {
                cerr << "Exception: "  << error.get_msg() << endl;
                throw;
            }
        }

        
        void
        EAIndexerXapian::CreateIndex( fh_context c, fh_context md )
        {
            string dbname   = getStrSubCtx( md, "dbname", "" );
            setConfig( CFG_XAPIANIDX_DBNAME_K,   dbname );

            setConfig( CFG_IDX_DBNAME_K,
                       getStrSubCtx( md, "dbname", CFG_IDX_DBNAME_DEF ) );

            Setup();
        }

        
        void
        EAIndexerXapian::CommonConstruction()
        {
        }
        
        
        void
        EAIndexerXapian::sync()
        {
            m_database.flush();
        }

        void
        EAIndexerXapian::prepareForWrites( int f )
        {
            if( f & PREPARE_FOR_WRITES_ISNEWER_TESTS )
            {
            }
        }
        

        std::string
        EAIndexerXapian::asString( const std::string& rawValue,
                                    const std::string& rawEAName,
                                    bool isCaseSensitive,
                                    AttrType_t att, bool quote )
        {
            switch( att )
            {
            case ATTRTYPEID_INT:
                LG_EAIDX_D << "asString(int) eaname:" << rawEAName
                           << " v.rawValueString:" << rawValue << endl;
                {
                    string ret = rawValue;
                    if( starts_with( rawValue, "?" ) )
                    {
                        ret = (string)"xsd:integer(" + ret + ")";
                    }
                    else
                    {
                        ret = tostr(convertStringToInteger( rawValue ));
                        if( quote )
                            ret = quoteStr( ret );
                        ret = (string)"xsd:integer(" + ret + ")";
                    }
                    return ret;
                }
                
            case ATTRTYPEID_DBL:
            {
                string ret = rawValue;
                if( ret.empty() )
                    ret = "0";
                if( quote )
                    ret = quoteStr( ret );
                ret = (string)"xsd:double(" + ret + ")";
                return ret;
            }
            case ATTRTYPEID_TIME:
            {
                LG_EAIDX_D << "asString(time) eaname:" << rawEAName
                           << " v.rawValueString:" << rawValue << endl;
                stringstream ss;
                // CHANGED: made quoting optional here
                if( quote )
                    ss << "'";
                if( starts_with( rawValue, "?" ) )
                    ss << rawValue;
                else
                    ss << toSQLTimeString( convertStringToInteger( rawValue ));
                if( quote )
                    ss << "'";
                string ret = tostr(ss);
                ret = (string)"xsd:date(" + ret + ")";
                return ret;
            }
            case ATTRTYPEID_CIS:
            case ATTRTYPEID_STR:
            {
                fh_stringstream ss;
                string str;
                
                if( isCaseSensitive )
                    str = rawValue;
                else
                    str = tolowerstr()( rawValue );
                
                if( quote )
                    ss << quoteStr( str );
                else
                    ss << str;
                
                return tostr(ss);
            }
            }
            return rawValue;
        }
        
        
        string
        EAIndexerXapian::asString( IndexableValue& v, AttrType_t att, bool quote )
        {
            return asString( v.rawValueString(),
                             v.rawEANameString(),
                             v.isCaseSensitive(),
                             att,
                             quote );
        }



        bool
        EAIndexerXapian::getIndexMethodSupportsIsFileNewerThanIndexedVersion()
        {
            return true;
        }
            
        bool
        EAIndexerXapian::isFileNewerThanIndexedVersion( const fh_context& c )
        {
            return common_isFileNewerThanIndexedVersion( m_database, c );
        }
        
        
        
        void
        EAIndexerXapian::addToIndex( fh_context c, fh_docindexer di )
        {
            LG_EAIDX_D << "EAIndexerXapian::addToIndex(top)" << endl;

            bool    hadError = false;
            string  earl     = c->getURL();
            LG_EAIDX_D << "EAIndexerXapian::addToIndex(1) earl:" << earl << endl;
            
            int attributesDone = 0;
            int signalWindow   = 0;
            stringlist_t slist;
            getEANamesToIndex( c, slist );
            int totalAttributes = slist.size();

            LG_EAIDX_D << "EAIndexerXapian::addToIndex(2) earl:" << earl << endl;
            
            Time::Benchmark bm( "earl:" + earl );
            bm.start();

            LG_EAIDX_D << "EAIndexerXapian::addToIndex() earl:" << earl << endl;


            string docidtime = tostr(Time::getTime());
            Xapian::Document doc;
            setupNewDocument( doc, c, docidtime );

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

                    LG_EAIDX_D << "EAIndexerXapian::addToIndex() attributeName:" << attributeName << endl;
                    LG_EAIDX_D << "addContextToIndex(a) c:" << c->getURL() << endl
                               << " k:" << k << " v:" << v << endl;

                    IndexableValue iv  = getIndexableValue( c, k, v );
                    ivs.insert( make_pair( attributeName, iv ) );
                }
                catch( exception& e )
                {
                    LG_EAIDX_D << "EXCEPTION e:" << e.what() << endl;
                }
            }

            
            //
            // add the k-v pairs to the doc as terms.
            // 
            for( ivs_t::iterator ivi = ivs.begin(); ivi!=ivs.end(); ++ivi )
            {
                IndexableValue& iv = ivi->second;
                const std::string& k = iv.rawEANameString();
                string             v = iv.rawValueString();

                string term = makeTerm( k, v );
                doc.add_term( term ); 

                LG_EAIDX_D << "added k:" << k << " v:" << v << " term:" << term << endl;
            }

            if( hadError )
            {
                LG_EAIDX_W << "addToIndex() had an error with file:" << c->getURL() << endl;
                cerr << "addToIndex() had an error with file:" << c->getURL() << endl;
            }
            else
            {
                LG_EAIDX_D << "addToIndex() commiting transaction" << endl;
                
            }

            string urlterm = makeURLTerm( c );
            Xapian::docid did = m_database.replace_document( urlterm, doc );
            ++m_filesIndexedCount;
        }


        
        void
        EAIndexerXapian::AddSPARQLOp( fh_stringstream& sqlss,
                                      Query& xq,
                                       const std::string& eaname,
                                       const std::string& opcode_const,
                                       IndexableValue& iv,
                                       AttrType_t att,
                                       bool negating )
        {
            string opcode = opcode_const;
            if( opcode == "==" )
                opcode = "=";
            if( opcode == "=~" )
                opcode = "~";
            
            string SQLDistinctClauseStart = " ";
            string SQLDistinctClauseEnd   = " ";
//             string SQLDistinctClauseStart = " distinct(";
//             string SQLDistinctClauseEnd   = ") ";
            
            string caseSenPrefix = " ";
            string caseSenPostfix = " ";
            LG_EAIDX_D << "AddSPARQLOp() att:" << att << " isCaseSen:" << iv.isCaseSensitive()
                       << " val:" << asString( iv, att )
                       << endl;
//             if( !iv.isCaseSensitive() && opcode != "~" )
//             {
//                 caseSenPrefix = " lower( ";
//                 caseSenPostfix = " ) ";
//             }
            
            LG_EAIDX_D << "EAIndexerXapian::AddSPARQLOp() "
                       << " v:" << iv.rawValueString()
                       << " v.asstring:" << asString( iv, att )
                       << endl;


            string negatingPrefix = "";
            string negatingPostfix = "";
            if( negating )
            {
                negatingPrefix = "( NOT (";
                negatingPostfix = "))";
            }
            
            if( opcode == "~" )
            {
                cerr << "Error, regex term matches do not work for xapian metadata indexes" << endl;
            }
            else
            {
                const std::string& k = iv.rawEANameString();
                string             v = iv.rawValueString();
//                sqlss << "(" << makeTerm( k,  v ) << ")";
                xq = Query( makeTerm( k,  v ) );
                sqlss << makeTerm( k,  v );
            }
        }

        MetaEAIndexerInterface::AttrType_t
        EAIndexerXapian::SQLColumnTypeToAttrType( const std::string& coltype,
                                                      IndexableValue& iv )
        {
        }
        
        
        void
        EAIndexerXapian::AddSPARQLOpHeur( fh_stringstream& sqlss,
                                        const std::string& eaname,
                                        const std::string& opcode,
                                        IndexableValue& iv,
                                        bool negating )
        {
        }

        string
        EAIndexerXapian::BuildQuery_getToken( fh_context q )
        {
            string token   = getStrAttr( q, "token", "" );
            string tokenfc = foldcase( token );
            return tokenfc;
        }

        pair< fh_context, fh_context >
        EAIndexerXapian::BuildQuery_getLeftAndRightContexts( fh_context q )
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
        EAIndexerXapian::BuildQuery_getEAName( fh_context q )
        {
            pair< fh_context, fh_context > p = BuildQuery_getLeftAndRightContexts( q );
            fh_context c = p.first;
            string ret   = getStrAttr( c, "token", "" );
            return ret;
        }
        
        docNumSet_t&
        EAIndexerXapian::BuildQuery_LogicalCombine(
            Xapian::Query::op xapianOpCode,
            fh_context q,
            docNumSet_t& output,
            fh_eaquery qobj,
            Query& xq,
            fh_stringstream& sqlss,
            bool& queryHasTimeRestriction,
            stringset_t& eanamesUsed,
            bool negating )
        {
            LG_EAIDX_D << " operator:" << xapianOpCode << ", child count:" << q->SubContextCount() << endl;
            
                
//            sqlss << "  {}" << nl;
            typedef list< fh_context > ctxlist_t;
            ctxlist_t ctxlist;
            for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                ctxlist.push_front( *ci );

            LG_EAIDX_D << "BuildQuery_LogicalCombine() op:" << xapianOpCode
                 << " terms:" << ctxlist.size()
                 << " negating:" << negating
                 << endl;
            
            bool v = true;
            // Handle remaining sub terms.
            for( ctxlist_t::iterator ci = ctxlist.begin(); ci != ctxlist.end(); ++ci )
            {
//                 if( v ) v = false;
//                 else    sqlss << termMergeSQLOpCode;

                if( !v )
                    sqlss << " AND ";

                
//                sqlss << "{ # start union block " << termMergeSQLOpCode << nl;
//                 string tokenfc = BuildQuery_getToken( *ci );
//                 if( tokenfc == "!" || tokenfc == "&" || tokenfc == "|" )
//                 {
//                     sqlss << " {} " << nl;
//                 }
//                 else
//                 {
//                     sqlss << getURLIDNodePredicates( queryHasTimeRestriction ) << nl;
//                 }
//                 sqlss << " . " << nl;
                
                Query tmp;
                BuildQuery( *ci, output, qobj, tmp, sqlss,
                            queryHasTimeRestriction,
                            eanamesUsed,
                            negating );
                if( v )
                {
                    v = false;
                    xq = tmp;
                }
                else
                {
                    xq = Query( xapianOpCode, xq, tmp );
                }
                
                

//                sqlss << "} # end union block " << nl;
            }
            sqlss << "  " << nl;

            return output;
        }
             

        
        docNumSet_t&
        EAIndexerXapian::BuildQuery(
            fh_context q,
            docNumSet_t& output,
            fh_eaquery qobj,
            Query& xq,
            fh_stringstream& sqlss,
            bool& queryHasTimeRestriction,
            stringset_t& eanamesUsed,
            bool negating )
        {
            string tokenfc = BuildQuery_getToken( q );
            pair< fh_context, fh_context > lcrc_pair = BuildQuery_getLeftAndRightContexts( q );
            fh_context lc = lcrc_pair.first;
            fh_context rc = lcrc_pair.second;
            
            if( tokenfc == "!" )
            {
                Query tmp;
                
                BuildQuery( lc, output, qobj, tmp, sqlss,
                            queryHasTimeRestriction,
                            eanamesUsed,
                            !negating );
                xq = Query( Xapian::Query::OP_AND_NOT, xq, tmp );
                return output;
            }


            if( tokenfc == "&" )
            {
                return BuildQuery_LogicalCombine( Xapian::Query::OP_AND,
                                                  q,
                                                  output,
                                                  qobj,
                                                  xq,
                                                  sqlss,
                                                  queryHasTimeRestriction,
                                                  eanamesUsed,
                                                  negating );
            }
            else if( tokenfc == "|" )
            {
                return BuildQuery_LogicalCombine( Xapian::Query::OP_OR,
                                                  q,
                                                  output,
                                                  qobj,
                                                  xq,
                                                  sqlss,
                                                  queryHasTimeRestriction,
                                                  eanamesUsed,
                                                  negating );
                
            }

            string eaname = getStrAttr( lc, "token", "" );
            eanamesUsed.insert( eaname );
            IndexableValue iv = getIndexableValueFromToken( eaname, rc );
            string value = asString( iv );
//            string comparisonOperator = iv.getComparisonOperator();
            AttrType_t attrTypeID = inferAttrTypeID( iv );

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
                    bool useShortcutDocIDs = false; // fidx->isTSearch2IndexInGivenDatabase( our_dbname );

                    string qstr = iv.rawValueString();
                    docNumSet_t docnums;

                    if( useShortcutDocIDs )
                    {
                        LG_EAIDX_D << "shortcut evaluation for tsearch2 index. "
                                   << "running ftx query:" << qstr << " against ftxpath:" << ftxpath << endl;
                        fidx->ExecuteRawFullTextQuery( qstr, docnums );
                    }
                    else
                    {
                        LG_EAIDX_D << "having to lookup all URLs from fulltext query to integrate cross engine index. "
                                   << "running ftx query:" << qstr << " against ftxpath:" << ftxpath << endl;
                        docNumSet_t tmp;
                        fidx->ExecuteRawFullTextQuery( qstr, tmp );

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

                        }

                        for( stringset_t::const_iterator si = earls.begin(); si != earls.end(); ++si )
                        {
                            string earl = *si;
                            string urlterm = makeURLTerm( earl );
                            if ( m_database.term_exists(urlterm) )
                            {
                                Enquire enquire( m_database );
                                Query query( urlterm );
                                enquire.set_query(query);
                                MSet matches = enquire.get_mset(0, 500);
//                                addAllDocumentsMatchingMSet( matches, output );
                            }
                        }
                    }

                    // make the current query select these urlids from the index as well.
//                  addAllDocumentsMatchingMSet( matches, output );
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
                AddSPARQLOp( sqlss, xq, eaname, tokenfc, iv, iv.getAttrTypeID(), negating );
            }
            else
            {
                stringstream ss;
                ss << "Xapian index currently only supports == metadata matches with possible * at the end of term." << endl;
                Throw_SyntaxError( tostr(ss), 0 );
            }
//             else if( tokenfc == "=~" )
//             {
//                 AddSPARQLOp( sqlss, eaname, tokenfc, iv, iv.getAttrTypeID(), negating );
//             }
//             else if( tokenfc == ">=" )
//             {
//                 AddSPARQLOp( sqlss, eaname, tokenfc, iv, iv.getAttrTypeID(), negating );
//             }
//             else if( tokenfc == "<=" )
//             {
//                 AddSPARQLOp( sqlss, eaname, tokenfc, iv, iv.getAttrTypeID(), negating );
//             }
//             else if( tokenfc == "=?=" )
//             {
//                 AddSPARQLOpHeur( sqlss, eaname, "=", iv, negating );
//             }
//             else if( tokenfc == ">?=" )
//             {
//                 AddSPARQLOpHeur( sqlss, eaname, ">=", iv, negating );
//             }
//             else if( tokenfc == "<?=" )
//             {
//                 AddSPARQLOpHeur( sqlss, eaname, "<=", iv, negating );
//             }
        }

        string
        EAIndexerXapian::getURLIDNodePredicates( bool& queryHasTimeRestriction )
        {
            stringstream ss;
            
//             ss << "        {" << nl;
//             ss << "          ?earlnode fa:uuid        ?uuidnode ." << nl;
//             ss << "	      ?earlnode eai:urlid      ?earlid ." << nl;
//             if( queryHasTimeRestriction )
//                 ss << "	      ?uuidnode fai:node       ?inode ." << nl;
//             else
//                 ss << "	      ?uuidnode fai:latestnode ?inode ." << nl;
//             ss << "	      ?earlnode eai:urlid ?urlid ." << nl;
//             ss << "        }" << nl;

            return tostr(ss);
        }
        
        
        docNumSet_t&
        EAIndexerXapian::BuildQueryXapian(
            fh_context q,
            docNumSet_t& output,
            fh_eaquery qobj,
            Query& xq,
            std::stringstream& SQLHeaderSS,
            std::stringstream& SQLWherePredicatesSS,
            std::stringstream& SQLTailerSS,
            bool& queryHasTimeRestriction,
            stringset_t& eanamesUsed )
        {
            fh_stringstream fSQLWherePredicatesSS;
            docNumSet_t& ret = BuildQuery( q,
                                           output,
                                           qobj,
                                           xq,
                                           fSQLWherePredicatesSS,
                                           queryHasTimeRestriction,
                                           eanamesUsed,
                                           false );
            LG_EAIDX_D << "EAIndexerXapian::BuildQueryXapian() where:" << tostr(fSQLWherePredicatesSS) << endl;
            SQLWherePredicatesSS << tostr(fSQLWherePredicatesSS);
            return output;
        }
        
        
        docNumSet_t&
        EAIndexerXapian::ExecuteQuery( fh_context q,
                                       docNumSet_t& output,
                                       fh_eaquery qobj,
                                       int limit )
        {
            if( limit == 0 )
                limit = 1000000;
            
            stringset_t eanamesUsed;
            bool queryHasTimeRestriction = false;
            std::stringstream HeaderSS;
            std::stringstream whereclauseSS;
            std::stringstream TailerSS;

            Query xq;
            BuildQueryXapian( q, output, qobj, xq,
                              HeaderSS,
                              whereclauseSS,
                              TailerSS,
                              queryHasTimeRestriction,
                              eanamesUsed );

            fh_stringstream qss;
//            qss << HeaderSS.str() << endl;
            qss << whereclauseSS.str();
//            qss << TailerSS.str() << endl;
            
            const std::string& querystr = tostr(qss);
            LG_EAIDX_D << "limit:" << limit << endl;
            LG_EAIDX_D << "XAPIAN QDMK -->:" << querystr << "<---" << endl << endl;
            LG_EAIDX_D << "XAPIAN Q:" << nl << querystr << endl << endl;
            LG_EAIDX_D << "XAPIAN DESC:" << nl << xq.get_description() << endl << endl;

//             if ( m_database.term_exists("sizeEQUALS459") )
//             {
//                 cerr << "Have 459 term" << endl;
//             }
            
            
//            string urlterm = makeURLTerm( "file:///tmp/KK/q.sparql" );
//            string urlterm = "Ufile:///tmp/KK/q.sparql";
//            string urlterm = "sizeEQUALS459";
            string urlterm = "size=459";
//            string urlterm = querystr;
            Enquire enquire( m_database );
            Query query = xq;

            QueryParser qp;
//            Query query( urlterm );
//            Query query( Query::OP_AND, Query(urlterm), Query("name-extension=sparql") );
            
//             Query subq1("");
//             Query subq2( Query::OP_AND, Query(urlterm), subq1 );
//             Query query( Query::OP_AND, subq2, Query("name-extension=sparql") );
//             Query query = qp.parse_query( querystr,
//                                           QueryParser::FLAG_PHRASE | QueryParser::FLAG_BOOLEAN | QueryParser::FLAG_WILDCARD );

//             qp.set_stemmer( Xapian::Stem() );
//             qp.set_stemming_strategy(Xapian::QueryParser::STEM_NONE);
//             qp.set_stopper( 0 );
//             Query query = qp.parse_query( "name-extension=ext AND size=459",
//                                           QueryParser::FLAG_BOOLEAN | QueryParser::FLAG_WILDCARD );
            enquire.set_query( query );
            MSet matches = enquire.get_mset(0, limit );
            addAllDocumentsMatchingMSet( matches, output );
            
            return output;
        }
        
        std::string
        EAIndexerXapian::resolveDocumentID( docid_t id )
        {
            return m_docidmap[ id ];
        }

        docNumSet_t&
        EAIndexerXapian::addAllDocumentsMatchingMSet( MSet& matches, docNumSet_t& output )
        {
            return common_addAllDocumentsMatchingMSet( matches, output,
                                                       m_docidmap,
                                                       m_revdocidmap,
                                                       m_docidmap_nextid,
                                                       addDocIDFunctor( this, &_Self::addDocID ) );
        }
    };
};



extern "C"
{
    FERRISEXP_API Ferris::EAIndex::MetaEAIndexerInterface* Create()
    {
        return new Ferris::EAIndex::EAIndexerXapian();
    }
};
