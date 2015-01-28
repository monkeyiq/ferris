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

    $Id: libeaindexsoprano.cpp,v 1.3 2009/10/02 21:30:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 *
 * earlnode       EAIDXPFX / urlid                 N
 * earlnode       uuidPredNode()                   uuidnode
 * uuidnode       nextdocid                        DocIDN
 * uuidnode       latestindextime                  timet
 * uuidnode       hasinstance/node                 inode
 * uuidnode       getPredicateURI(hasinstanceid)   YDocID
 * inode          getPredicateURI(eaname)          getPredicateURI(eavalue)
 *
 *
 *
 */

#include "EAIndexerMetaInterface.hh"
#include "EAIndexer.hh"
#include "EAQuery.hh"
#include "General.hh"
#include "FactoriesCreationCommon_private.hh"
#include "Ferris/Ferris_private.hh"
#include "Ferris/Medallion.hh"
#include "Ferris/Iterator.hh"
#include "Ferris/FerrisSemantic.hh"

#include "EAIndexerSQLCommon_private.hh"

// #define DEBUG_ADDING_TO_INDEX 1

using namespace std;

namespace Ferris
{
    namespace EAIndex 
    {
        using namespace ::Ferris::RDFCore;
        using namespace ::Ferris::Semantic;
        
        static string EAIDXPFX = "http://witme.sf.net/libferris.web/rdf/eaindex/";

        static const char* CFG_IDX_DBNAME_K = "cfg-idx-dbname";
        static const char* CFG_IDX_DBNAME_DEF = "ferriseaindex";
        static const char* CFG_IDX_DBOPTIONS_K = "cfg-idx-dboptions";
        static const char* CFG_IDX_DBOPTIONS_DEF = "hash-type='bdb',contexts='yes',index-predicates='yes',";
        static const char* CFG_IDX_STORAGENAME_K = "cfg-idx-storage-name";
        static const char* CFG_IDX_STORAGENAME_DEF = "hashes";


        static string quoteStr( const std::string& s )
        {
            return "'" + s + "'";
        }

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/

        
        class FERRISEXP_DLLLOCAL EAIndexerSoprano
            :
            public MetaEAIndexerInterface
        {
            
            int         m_filesIndexedCount;
            fh_model    m_model;

            int getLimit()
                {
                    return 0;
                }
            
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
            EAIndexerSoprano();
            virtual ~EAIndexerSoprano();

            virtual void sync();
            virtual void prepareForWrites( int f );

            docid_t obtainURLID( fh_context c, fh_node earlnode );

            virtual void addToIndex( fh_context c,
                                     fh_docindexer di );
            void AddSPARQLOp( fh_stringstream& sqlss,
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
                const std::string& termMergeSQLOpCode,
                fh_context q,
                docNumSet_t& output,
                fh_eaquery qobj,
                fh_stringstream& sqlss,
                bool& queryHasTimeRestriction,
                stringset_t& eanamesUsed,
                int nestedDepth,
                bool negating,
                bool& startsWithLogicalOR );
            
            virtual docNumSet_t& BuildQuery(
                fh_context q,
                docNumSet_t& output,
                fh_eaquery qobj,
                fh_stringstream& sqlss,
                bool& queryHasTimeRestriction,
                stringset_t& eanamesUsed,
                int nestedDepth,
                bool negating,
                bool& startsWithLogicalOR );

            string getURLIDNodePredicates( bool& queryHasTimeRestriction );
            virtual docNumSet_t& BuildQuerySPARQL(
                fh_context q,
                docNumSet_t& output,
                fh_eaquery qobj,
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
            
        };

        /************************************************************/
        /************************************************************/
        /************************************************************/

        EAIndexerSoprano::EAIndexerSoprano()
            :
            m_filesIndexedCount( 0 ),
            m_model( 0 )
        {
        }

        

        EAIndexerSoprano::~EAIndexerSoprano()
        {
            if( m_model )
            {
                sync();
            }
        }

        
        void
        EAIndexerSoprano::Setup()
        {
            LG_EAIDX_D << "EAIndexerSoprano::Setup()" << endl;
            if( !m_model )
            {
                string storage_name = this->getConfig( CFG_IDX_STORAGENAME_K, CFG_IDX_STORAGENAME_DEF, true );
                string db_name      = this->getConfig( CFG_IDX_DBNAME_K,      CFG_IDX_DBNAME_DEF, true );
                string db_options   = this->getConfig( CFG_IDX_DBOPTIONS_K,   CFG_IDX_DBOPTIONS_DEF, true );

                LG_EAIDX_D << "Opening model. storage:" << storage_name
                           << " db_name:" << db_name
                           << endl << " db_options:" << db_options << endl;

                fh_context md = Resolve( db_name );
                m_model = Model::FromMetadataContext( md );
                
                // if( storage_name != "hashes" )
                // {
                //     m_model = Model::ObtainDB( storage_name, db_name, db_options );
                // }
                // else
                // {
                //     fh_context rdfdbc = Resolve( this->getURL() );
                //     fh_stringstream dbops;
                //     dbops << db_options << ","
                //           << "db-environment-dir='" << rdfdbc->getDirPath() << "'";

                //     LG_EAIDX_D << "Opening hashes model."
                //                << " db_name:" << db_name
                //                << endl << " db_options:" << tostr(dbops) << endl;
                //     m_model = Model::ObtainDB( storage_name, db_name, tostr(dbops) );
                // }
            }
        }

        
        
        
        void
        EAIndexerSoprano::CreateIndex( fh_context c, fh_context md )
        {
            string dbname      = getStrSubCtx( md, "dbname", CFG_IDX_DBNAME_DEF );
            string dboptions   = getStrSubCtx( md, "dboptions", CFG_IDX_DBOPTIONS_DEF );
            string storagename = getStrSubCtx( md, "storagename", CFG_IDX_STORAGENAME_DEF );

            if( dboptions.find( "hash-type='bdb" ) != string::npos )
            {
                if( dboptions.find( "dir=" ) == string::npos )
                {
                    dboptions += ",dir='" + getPath() + "'";
                }
            }
            
            setConfig( CFG_IDX_DBNAME_K, dbname );
            setConfig( CFG_IDX_DBOPTIONS_K, dboptions );
            setConfig( CFG_IDX_STORAGENAME_K, storagename );

            LG_EAIDX_D << "creaing index...A" << endl;

            Setup();
            {
                fh_node gsubj = Node::CreateURI( EAIDXPFX + "metadata" );
                fh_node gpred = Node::CreateURI( EAIDXPFX + "nexturlid" );
                m_model->set( gsubj, gpred, Node::CreateLiteral( tostr( 1 ) ) );
                LG_EAIDX_D << "Setting up the initial nexturlid" << endl;

                fh_context c = Resolve( getPath() );
                fh_node earlnode = Node::CreateURI( c->getURL() );
                
                docid_t id = obtainURLID( c, earlnode );
                
            }
            m_model->sync();
        }

        
        void
        EAIndexerSoprano::CommonConstruction()
        {
        }
        
        
        void
        EAIndexerSoprano::sync()
        {
            if( m_model )
                m_model->sync();
        }

        void
        EAIndexerSoprano::prepareForWrites( int f )
        {
        }
        

        std::string
        EAIndexerSoprano::asString( const std::string& rawValue,
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
        EAIndexerSoprano::asString( IndexableValue& v, AttrType_t att, bool quote )
        {
            return asString( v.rawValueString(),
                             v.rawEANameString(),
                             v.isCaseSensitive(),
                             att,
                             quote );
        }

        docid_t
        EAIndexerSoprano::obtainURLID( fh_context c, fh_node earlnode )
        {
            fh_node pred = Node::CreateURI( EAIDXPFX + "urlid" );
            fh_node n = m_model->getObject( earlnode, pred );
            if( n )
            {
                long ret = toint( n->toString() );
                return ret;
            }

            fh_node gsubj = Node::CreateURI( EAIDXPFX + "metadata" );
            fh_node gpred = Node::CreateURI( EAIDXPFX + "nexturlid" );
            fh_node nextidn = m_model->getObject( gsubj, gpred );
            LG_EAIDX_D << "obtainURLID() model:" << toVoid(m_model) << " c:" << c->getURL() << endl
                 << " gsubj:" << gsubj->toString() << endl
                 << " gpred:" << gpred->toString() << endl
                 << " nextidn:" << isBound( nextidn ) << endl;
            
            long ret = toint( nextidn->toString() );
            m_model->set( gsubj, gpred, Node::CreateLiteral( tostr( ret+1 ) ) );
            m_model->set( earlnode, pred, Node::CreateLiteral( tostr( ret ) ) );
            return ret;
        }


        bool
        EAIndexerSoprano::getIndexMethodSupportsIsFileNewerThanIndexedVersion()
        {
            return true;
        }
            
        bool
        EAIndexerSoprano::isFileNewerThanIndexedVersion( const fh_context& c )
        {
            string  earl     = c->getURL();
            fh_node earlnode = Node::CreateURI( c->getURL() );
            fh_node uuidnode = ensureUUIDNode( m_model, c );

            fh_node pred = Node::CreateURI( EAIDXPFX + "latestindextime" );
            fh_node n    = m_model->getObject( uuidnode, pred );
            if( !n )
                return true;

            long nv = toint( n->toString() );
            time_t ct = getTimeAttr( c, "ferris-should-reindex-if-newer", 0 );
            if( !ct )
                return true;

            return nv <= ct;
        }
        
        
        
        void
        EAIndexerSoprano::addToIndex( fh_context c, fh_docindexer di )
        {
            LG_EAIDX_D << "EAIndexerSoprano::addToIndex(top)" << endl;

            bool    hadError = false;
            string  earl     = c->getURL();
            fh_node earlnode = Node::CreateURI( c->getURL() );
            fh_node uuidnode = ensureUUIDNode( m_model, c );

            long urlid  = obtainURLID( c, earlnode );
                          
            LG_EAIDX_D << "EAIndexerSoprano::addToIndex(1) earl:" << earl << endl;
            
            int attributesDone = 0;
            int signalWindow   = 0;
            stringlist_t slist;
            getEANamesToIndex( c, slist );
            int totalAttributes = slist.size();

            LG_EAIDX_D << "EAIndexerSoprano::addToIndex(2) earl:" << earl << endl;
            
            Time::Benchmark bm( "earl:" + earl );
            bm.start();

            
            LG_EAIDX_D << "EAIndexerSoprano::addToIndex() earl:" << earl << endl;
            long docid  = -1;
            {
                fh_node nextdocidpred = Node::CreateURI( EAIDXPFX + "nextdocid" );
                fh_node nextdocid = m_model->getObject( uuidnode, nextdocidpred );
                if( nextdocid )
                {
                    docid = toint( nextdocid->toString() );
                }
                else
                {
                    docid = 1;
                }
                m_model->set( uuidnode, nextdocidpred,
                              Node::CreateLiteral( tostr( docid+1 ) ) );
            }
            fh_node inode = Node::CreateURI( getPredicateURI( Util::makeUUID() ) );
            m_model->insert( uuidnode,
                             Node::CreateURI( getPredicateURI( "hasinstance/node" ) ), inode );
            m_model->set( uuidnode,
                          Node::CreateURI( getPredicateURI( "hasinstance/latestnode" ) ), inode );
            m_model->set( inode,
                          Node::CreateURI( EAIDXPFX + "metadata/id" ),
                          Node::CreateLiteral( tostr( docid ) ) );
            m_model->set( inode,
                          Node::CreateURI( EAIDXPFX + "metadata/indexedtime" ),
                          Node::CreateLiteral( tostr( Time::getTime() ) ) );
            m_model->set( uuidnode,
                          Node::CreateURI( EAIDXPFX + "latestindextime" ),
                          Node::CreateLiteral( tostr( Time::getTime() ) ) );

            
            LG_EAIDX_D << "EAIndexerSoprano::addToIndex() docid:" << docid
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

                    LG_EAIDX_D << "EAIndexerSoprano::addToIndex() attributeName:" << attributeName << endl;
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
            // add the triples inode, eaname, eavalue
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


                m_model->set( inode,
                              Node::CreateURI( getPredicateURI( k ) ),
                              Node::CreateLiteral( v ) );
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

            ++m_filesIndexedCount;
        }


        
        void
        EAIndexerSoprano::AddSPARQLOp( fh_stringstream& sqlss,
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
            
            LG_EAIDX_D << "EAIndexerSoprano::AddSPARQLOp() "
                       << " v:" << iv.rawValueString()
                       << " v.asstring:" << asString( iv, att )
                       << endl;

            static int varnamePostfix = 1;
            string varname = (string)"?foo" + tostr(varnamePostfix);
            ++varnamePostfix;
            sqlss << "{ ?inode " 
                  << " fa:" << eaname  << " "
                  << " " << varname
                  << " . ";

            string negatingPrefix = "";
            string negatingPostfix = "";
            if( negating )
            {
                negatingPrefix = "!(";
                negatingPostfix = ")";
            }
            
            if( opcode == "~" )
            {
                sqlss << " FILTER regex( " << negatingPrefix
                      << varname << ", " << asString( iv, att )
                      << negatingPostfix << " )"
                      << "}" << endl;
            }
            else
            {
                sqlss << " FILTER( " << negatingPrefix
                      << asString( varname, varname, iv.isCaseSensitive(), att, true )
                      << " " << opcode << " "
                      << asString( iv, att )
                      << negatingPostfix << " )"
                      << "}" << endl;
            }
            
//             sqlss << caseSenPrefix << "?inode d.\"" << colname << "\"" << caseSenPostfix << " "
//                   << opcode 
//                   << caseSenPrefix << asString( iv, att ) << caseSenPostfix;
        }

        MetaEAIndexerInterface::AttrType_t
        EAIndexerSoprano::SQLColumnTypeToAttrType( const std::string& coltype,
                                                      IndexableValue& iv )
        {
//             AttrType_t att = ATTRTYPEID_CIS;
//             if( starts_with( coltype, "int" ))
//                 att = ATTRTYPEID_INT;
//             if( starts_with( coltype, "double" ) || starts_with( coltype, "float" ) )
//                 att = ATTRTYPEID_DBL;
//             if( starts_with( coltype, "timestamp" ))
//                 att = ATTRTYPEID_TIME;
//             if( iv.isCaseSensitive() )
//                 att = ATTRTYPEID_STR;
            
//             return att;
        }
        
        
        void
        EAIndexerSoprano::AddSPARQLOpHeur( fh_stringstream& sqlss,
                                        const std::string& eaname,
                                        const std::string& opcode,
                                        IndexableValue& iv,
                                        bool negating )
        {
//             // FIXME: Heuristic lookup for columns inlined into docmap should
//             // only cast to the type of that column,

//             stringmap_t::const_iterator eci
//                 = m_ExtraColumnsToInlineInDocmap.find( eaname );

//             if( eci != m_ExtraColumnsToInlineInDocmap.end() )
//             {
//                 string colname = ( eci->first );
//                 string coltype = eci->second;
//                 AttrType_t att = SQLColumnTypeToAttrType( coltype, iv );
//                 AddSPARQLOp( sqlss, eaname, opcode, iv, att );
//                 return;
//             }
            
//             AttrTypeList_t atl = getAllAttrTypes();
            
//             bool v = true;

//             sqlss << "d.docid in (" << nl
//                   << " SELECT distinct(docattrs.docid) as docid FROM docattrs "
//                   << " WHERE  " << nl;

//             for( AttrTypeList_t::const_iterator attrTypeIter = atl.begin();
//                  attrTypeIter!=atl.end(); ++attrTypeIter )
//             {
// //                 if( *attrTypeIter == ATTRTYPEID_STR )
// //                     continue;
                
//                 if( v ) v = false;
//                 else    sqlss << " " << nl
// //                              << " OR d.docid in " << nl
//                               << " OR  " << nl
//                               << " ";
//                 AddSPARQLOp( sqlss, eaname, opcode, iv, *attrTypeIter );
//             }
//             sqlss << ")" << endl;
        }

        string
        EAIndexerSoprano::BuildQuery_getToken( fh_context q )
        {
            string token   = getStrAttr( q, "token", "" );
            string tokenfc = foldcase( token );
            return tokenfc;
        }

        pair< fh_context, fh_context >
        EAIndexerSoprano::BuildQuery_getLeftAndRightContexts( fh_context q )
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
        EAIndexerSoprano::BuildQuery_getEAName( fh_context q )
        {
            pair< fh_context, fh_context > p = BuildQuery_getLeftAndRightContexts( q );
            fh_context c = p.first;
            string ret   = getStrAttr( c, "token", "" );
            return ret;
        }
        
        docNumSet_t&
        EAIndexerSoprano::BuildQuery_LogicalCombine(
            const std::string& termMergeSQLOpCode,
            fh_context q,
            docNumSet_t& output,
            fh_eaquery qobj,
            fh_stringstream& sqlss,
            bool& queryHasTimeRestriction,
            stringset_t& eanamesUsed,
            int nestedDepth,
            bool negating,
            bool& startsWithLogicalOR )
        {
            LG_EAIDX_D << " operator:" << termMergeSQLOpCode << ", child count:" << q->SubContextCount() << endl;
            
            if( nestedDepth > 1 )
                sqlss << " { ";
                
            sqlss << "  {}" << nl;
            typedef list< fh_context > ctxlist_t;
            ctxlist_t ctxlist;
            for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                ctxlist.push_front( *ci );

            LG_EAIDX_D << "BuildQuery_LogicalCombine() op:" << termMergeSQLOpCode
                 << " nestedDepth:" << nestedDepth
                 << " terms:" << ctxlist.size()
                 << " negating:" << negating
                 << endl;
            
            bool v = true;
            // Handle remaining sub terms.
            for( ctxlist_t::iterator ci = ctxlist.begin(); ci != ctxlist.end(); ++ci )
            {
//                if( v ) v = false;
//                else
                sqlss << termMergeSQLOpCode << nl;

                sqlss << "{ # start union block " << termMergeSQLOpCode << nl;
                string tokenfc = BuildQuery_getToken( *ci );
                if( tokenfc == "!" || tokenfc == "&" || tokenfc == "|" )
                {
                    sqlss << " {} " << nl;
                }
                else
                {
                    sqlss << getURLIDNodePredicates( queryHasTimeRestriction ) << nl;
                }
                sqlss << " . " << nl;
                
                
                BuildQuery( *ci, output, qobj, sqlss,
                            queryHasTimeRestriction,
                            eanamesUsed,
                            nestedDepth+1,
                            negating,
                            startsWithLogicalOR );

                sqlss << "} # end union block " << nl;
            }
            sqlss << "  " << nl;
            if( nestedDepth > 1 )
                sqlss << " } ";

            return output;
        }
             

        
        docNumSet_t&
        EAIndexerSoprano::BuildQuery(
            fh_context q,
            docNumSet_t& output,
            fh_eaquery qobj,
            fh_stringstream& sqlss,
            bool& queryHasTimeRestriction,
            stringset_t& eanamesUsed,
            int nestedDepth,
            bool negating,
            bool& startsWithLogicalOR )
        {
            string tokenfc = BuildQuery_getToken( q );
            pair< fh_context, fh_context > lcrc_pair = BuildQuery_getLeftAndRightContexts( q );
            fh_context lc = lcrc_pair.first;
            fh_context rc = lcrc_pair.second;
            
            if( tokenfc == "!" )
            {
//                 string termMergeSQLOpCode = " . ";
//                 return BuildQuery_LogicalCombine( termMergeSQLOpCode,
//                                                   q,
//                                                   output,
//                                                   qobj,
//                                                   sqlss,
//                                                   queryHasTimeRestriction,
//                                                   eanamesUsed,
//                                                   nestedDepth,
//                                                   !negating );

                BuildQuery( lc, output, qobj, sqlss,
                            queryHasTimeRestriction,
                            eanamesUsed,
                            nestedDepth,
                            !negating,
                            startsWithLogicalOR );
                return output;
            }


            if( tokenfc == "&" )
            {
                string termMergeSQLOpCode = " . ";
                return BuildQuery_LogicalCombine( termMergeSQLOpCode,
                                                  q,
                                                  output,
                                                  qobj,
                                                  sqlss,
                                                  queryHasTimeRestriction,
                                                  eanamesUsed,
                                                  nestedDepth,
                                                  negating,
                                                  startsWithLogicalOR );
            }
            else if( tokenfc == "|" )
            {
                string termMergeSQLOpCode = " UNION ";

                if( nestedDepth == 1 )
                    startsWithLogicalOR = true;
                LG_EAIDX_D << "nestedDepth:" << nestedDepth
                           << " startsWithLogicalOR:" << startsWithLogicalOR << endl;
                
                return BuildQuery_LogicalCombine( termMergeSQLOpCode,
                                                  q,
                                                  output,
                                                  qobj,
                                                  sqlss,
                                                  queryHasTimeRestriction,
                                                  eanamesUsed,
                                                  nestedDepth,
                                                  negating,
                                                  startsWithLogicalOR );
                
            }

            string eaname = getStrAttr( lc, "token", "" );
            eanamesUsed.insert( eaname );
            IndexableValue iv = getIndexableValueFromToken( eaname, rc );
            string value = asString( iv );
//            string comparisonOperator = iv.getComparisonOperator();
            AttrType_t attrTypeID = inferAttrTypeID( iv );

//             cerr << "ADDING TO eanamesUsed:" << eaname
//                  << " val:" << value
//                  << endl;
            


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

                        }

                        stringstream sqlss;
                        sqlss << "PREFIX  fa:  <http://witme.sf.net/libferris.web/rdf/ferris-attr/>" << endl
                              << "PREFIX  fai: <http://witme.sf.net/libferris.web/rdf/ferris-attr/hasinstance/>" << endl
                              << "PREFIX  eai: <http://witme.sf.net/libferris.web/rdf/eaindex/>" << endl
                              << "PREFIX   op: <http://www.w3.org/2004/07/xpath-functions/>" << endl
                              << "" << endl
                              << "SELECT ?urlid ?earlnode" << endl
                              << "WHERE {" << endl
                              << "" << endl
                              << "?earlnode eai:urlid ?urlid ." << endl
                              << "FILTER(" << endl;
                        
                        bool v = true;
                        for( stringset_t::const_iterator si = earls.begin(); si != earls.end(); ++si )
                        {
                            LG_EAIDX_D << "Fulltext match:" << *si << endl;
                            if( v ) v = false;
                            else    sqlss << " || ";
                            sqlss << " ?earlnode = <" << *si << ">" << endl;
                        }
                        sqlss << endl << ") }" << endl;
                        
                        LG_EAIDX_D << "select urlid SPARQL:" << tostr(sqlss) << endl;
                        
                        // selecting foreign urls to index
                        BindingsIterator iter = m_model->findBindings( tostr(sqlss) );
                        BindingsIterator e;
                        for( ; iter != e ; ++iter )
                        {
                            fh_node n = iter[ "urlid" ];
                            docid_t d = -1;
                            d = toint( n->toString() );
                            LG_EAIDX_D << " urlid from fulltext match:" << d << endl;
                            docnums.insert( d );
                        }
                        
                        LG_EAIDX_D << "docnums.sz:" << docnums.size() << endl;
                    }


                    // make the current query select these urlids from the index as well.
                    sqlss << " { FILTER( ";
                    bool v = true;
                    for( docNumSet_t::const_iterator di = docnums.begin();
                         di != docnums.end(); ++di )
                    {
                        if( v ) v = false;
                        else    sqlss << " || ";
                        sqlss << " ?urlid = '" << *di << "'  " << endl;                        
                    }
                    sqlss << " ) } ";
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
                AddSPARQLOp( sqlss, eaname, tokenfc, iv, iv.getAttrTypeID(), negating );
            }
            else if( tokenfc == "=~" )
            {
                AddSPARQLOp( sqlss, eaname, tokenfc, iv, iv.getAttrTypeID(), negating );
            }
            else if( tokenfc == ">=" )
            {
                AddSPARQLOp( sqlss, eaname, tokenfc, iv, iv.getAttrTypeID(), negating );
            }
            else if( tokenfc == "<=" )
            {
                AddSPARQLOp( sqlss, eaname, tokenfc, iv, iv.getAttrTypeID(), negating );
            }
            else if( tokenfc == "=?=" )
            {
                AddSPARQLOpHeur( sqlss, eaname, "=", iv, negating );
            }
            else if( tokenfc == ">?=" )
            {
                AddSPARQLOpHeur( sqlss, eaname, ">=", iv, negating );
            }
            else if( tokenfc == "<?=" )
            {
                AddSPARQLOpHeur( sqlss, eaname, "<=", iv, negating );
            }
        }

        string
        EAIndexerSoprano::getURLIDNodePredicates( bool& queryHasTimeRestriction )
        {
            stringstream ss;
            
            ss << "        {" << nl;
            ss << "          ?earlnode fa:uuid        ?uuidnode ." << nl;
            ss << "	      ?earlnode eai:urlid      ?earlid ." << nl;
            if( queryHasTimeRestriction )
                ss << "	      ?uuidnode fai:node       ?inode ." << nl;
            else
                ss << "	      ?uuidnode fai:latestnode ?inode ." << nl;
            ss << "	      ?earlnode eai:urlid ?urlid ." << nl;
            ss << "        }" << nl;

            return tostr(ss);
        }
        
        
        docNumSet_t&
        EAIndexerSoprano::BuildQuerySPARQL(
            fh_context q,
            docNumSet_t& output,
            fh_eaquery qobj,
            std::stringstream& SQLHeaderSS,
            std::stringstream& SQLWherePredicatesSS,
            std::stringstream& SQLTailerSS,
            bool& queryHasTimeRestriction,
            stringset_t& eanamesUsed )
        {
            bool startsWithLogicalOR = false;

            bool shouldPutInEarlHeader = true;
            string tokenfc = BuildQuery_getToken( q );
            if( tokenfc == "&" || tokenfc == "|" )
                shouldPutInEarlHeader = false;

            fh_stringstream fSQLWherePredicatesSS;
            docNumSet_t& ret = BuildQuery( q,
                                           output,
                                           qobj,
                                           fSQLWherePredicatesSS,
                                           queryHasTimeRestriction,
                                           eanamesUsed,
                                           1,
                                           false,
                                           startsWithLogicalOR );
            LG_EAIDX_D << "EAIndexerSoprano::BuildQuerySPARQL() where:" << tostr(fSQLWherePredicatesSS)
                       << endl;
            SQLWherePredicatesSS << tostr(fSQLWherePredicatesSS);

            SQLHeaderSS << "PREFIX  fa:  <http://witme.sf.net/libferris.web/rdf/ferris-attr/>" << nl;
            SQLHeaderSS << "PREFIX  fai: <http://witme.sf.net/libferris.web/rdf/ferris-attr/hasinstance/>" << nl;
            SQLHeaderSS << "PREFIX  eai: <http://witme.sf.net/libferris.web/rdf/eaindex/>" << nl;
            SQLHeaderSS << "PREFIX   op: <http://www.w3.org/2004/07/xpath-functions/>" << nl;
            SQLHeaderSS << "PREFIX  xsd: <http://www.w3.org/2001/XMLSchema#>" << nl;
            SQLHeaderSS << nl;
            SQLHeaderSS << "SELECT ?urlid ?earlnode " << nl;
            SQLHeaderSS << "WHERE {" << nl;
           if( shouldPutInEarlHeader )
               SQLHeaderSS << getURLIDNodePredicates( queryHasTimeRestriction ) << nl;
            
            if( queryHasTimeRestriction )
            {
            }
            else
            {
            }

            SQLTailerSS << "}" << endl;
            return output;
        }
        
        
        docNumSet_t&
        EAIndexerSoprano::ExecuteQuery( fh_context q,
                                        docNumSet_t& output,
                                        fh_eaquery qobj,
                                        int limit )
        {
            stringset_t eanamesUsed;
            bool queryHasTimeRestriction = false;
            std::stringstream HeaderSS;
            std::stringstream whereclauseSS;
            std::stringstream TailerSS;
            
            BuildQuerySPARQL( q, output, qobj,
                              HeaderSS,
                              whereclauseSS,
                              TailerSS,
                              queryHasTimeRestriction,
                              eanamesUsed );

            fh_stringstream qss;
            qss << HeaderSS.str() << endl;
            qss << whereclauseSS.str() << endl;
            qss << TailerSS.str() << endl;
            if( limit )
            {
                qss << endl << " limit " << limit << endl;
            }
            
            const std::string& query = tostr(qss);
            LG_EAIDX_D << "SPARQL:" << nl << query << endl << endl;
            LG_EAIDX_D << "SPARQL:" << nl << query << endl << endl;
            
            BindingsIterator iter = m_model->findBindings( query );

            BindingsIterator e;
            for( ; iter != e ; ++iter )
            {
                fh_node n = iter[ "urlid" ];
                docid_t d = -1;
                d = toint( n->toString() );
                output.insert( d );
            }
            
            return output;
        }
        
        std::string
        EAIndexerSoprano::resolveDocumentID( docid_t id )
        {
            fh_node docidnode = Node::CreateLiteral( tostr( id ) );                
            fh_node pred      = Node::CreateURI( EAIDXPFX + "urlid" );
            fh_node urlnode   = m_model->getSubject( pred, docidnode );
            if( urlnode )
            {
                return urlnode->getURI()->toString();
            }
            
            LG_EAIDX_W << "ResolveDocumentID(error) id:" << id << endl;
            fh_stringstream ess;
            ess << "Failed to resolve document ID:" << id
                << endl;
            Throw_IndexException( tostr( ess ), 0 );
        }

    };
};



extern "C"
{
    FERRISEXP_API Ferris::EAIndex::MetaEAIndexerInterface* Create()
    {
        return new Ferris::EAIndex::EAIndexerSoprano();
    }
};
