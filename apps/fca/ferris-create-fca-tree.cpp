/******************************************************************************
*******************************************************************************
*******************************************************************************

    create a new FCA tree command line client
    Copyright (C) 2003 Ben Martin

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

    $Id: ferris-create-fca-tree.cpp,v 1.11 2010/09/24 21:31:10 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include "libferrisfcascaling.hh"

#include <Ferris/EAIndexer_private.hh>
#include <Ferris/Runner.hh>
#include <Ferris/EAIndexerSQLCommon_private.hh>
#include <Ferris/FCA.hh>


using namespace Ferris::FCA;


const string PROGRAM_NAME = "ferris-create-fca-tree";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

class Titanic
{
    int m_BifFieldSize;

    typedef std::list< std::string > PGOutputTableDataColumnsList_t;
    PGOutputTableDataColumnsList_t PGOutputTableDataColumnsList;

    typedef list< string > colNames_t;
    colNames_t colNames;
    colNames_t colNamesBitf;
    bool colNamesBitfSetup;
    colNames_t* SplitColNames;

    int m_transactionCount;
    
public:

    Titanic()
        :
        m_BifFieldSize( 0 ),
        colNamesBitfSetup( false )
        {
            SplitColNames = &colNames;
        }

    typedef FerrisBitMagic::bvector<> itemset_t;
    typedef list< itemset_t > itemsetList_t;

    void weight( itemsetList_t& l )
        {
        }
    
    
    void perform()
        {
            string PGQuery = "select bitf  FROM wn1 d";
            connection* db_connection = 0;

    
    
            {
                stringstream ss;
                string user = "";
                string port = "";
                string PGServer = "localhost";
                string PGDatabase = "findexpriv";
      
                if( !user.empty() )
                    ss << " user=" << user;
                if( !PGServer.empty() )
                    ss << " host=" << PGServer;
                if( !port.empty() )
                    ss << " port=" << port;
                if( !PGDatabase.empty() )
                    ss << " dbname=" << PGDatabase;

                string constring = ss.str();
                db_connection = new connection( constring );
            }

            work trans( *db_connection, "getting the data..." );

            result res = trans.exec( PGQuery );

            if( res.empty() )
            {
                cerr << "SQL Query generates no tuples" << endl;
                exit(1);
            }

            // if we are making an output table then we need
            // to know the input column names for the output table
            {
                string InputBitVaryingColumnName = "bitf";
        
                m_BifFieldSize = 1;
                result::const_iterator c = res.begin();
                if( c != res.end() )
                {
                    result::tuple::const_iterator e = c->end();
                    for( result::tuple::const_iterator cur = c->begin();
                         cur != e; ++cur )
                    {
                        string name = cur->name();

                        if( InputBitVaryingColumnName == name )
                        {
                            string t;
                            cur->to(t);
                            m_BifFieldSize = t.length();
                        }
                  
                        PGOutputTableDataColumnsList.push_back( name );
                        colNames.push_back( name );
                    }
                }
            }


            for( colNames_t::const_iterator cni = colNames.begin();
                 cni != colNames.end(); ++cni )
            {
                fprintf(stderr,"cni:%s\n", cni->c_str() );
            }

            m_transactionCount = 0;
            for( result::const_iterator res_iter = res.begin(); res_iter != res.end(); ++res_iter )
            {

                ++m_transactionCount;
            }

            cerr << "m_transactionCount:" << m_transactionCount << endl;
        }
};


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


static string makeNonNormalizedLiftString( const std::string& rdn )
{
    stringstream ss;
    ss << "    ,d." << rdn << "  as \"" << rdn << "\" " << nl;
    return ss.str();
}

static string DQuoteStr( work& trans, const std::string& s )
{
    stringstream ss;
    ss << "\"" << trans.esc( s ) << "\"";
    return tostr(ss);
}


int main( int argc, char** argv )
{
    int exit_status = 0;

//     Titanic t;
//     t.perform();
//     cerr << "EXITING AFTER TITANIC!" << endl;
//     exit(0);
    

    
    try
    {
        unsigned long Verbose              = 0;
        unsigned long dontCreateCFI        = 0;
        unsigned long dontCreateParentChild = 0;
        unsigned long dontDenormalizeEAIntoBaseTable = 0;
        const char* host_CSTR   = 0;
        const char* dbname_CSTR = 0;
        const char* findexPath_CSTR = 0;
        const char* newTreeName = 0;
        const char* newTreeCFIPostfix = "_cfi";

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "report more details than normal", "" },

//                 { "host", 'h', POPT_ARG_STRING, &host_CSTR, 0,
//                   "host running postgresql server", "localhost" },

//                 { "dbname", 'd', POPT_ARG_STRING, &dbname_CSTR, 0,
//                   "database to connect to", "" },

                { "index-path", 'P', POPT_ARG_STRING, &findexPath_CSTR, 0,
                  "path to existing postgresql EA index", "" },
                
                { "treename", 'n', POPT_ARG_STRING, &newTreeName, 0,
                  "name of the new fca-tree", "" },
                
                { "skip-cfi", 'C', POPT_ARG_NONE, &dontCreateCFI, 0,
                  "don't calculate the CFI at this time.", "" },

                { "skip-parent-child", 'c', POPT_ARG_NONE, &dontCreateParentChild, 0,
                  "don't calculate the parent/child relation for the CFI at this time.", "" },

                { "dont-denormalize-ea-into-basetable", '1', POPT_ARG_NONE,
                  &dontDenormalizeEAIntoBaseTable, 0,
                  "don't denormalize all the EA in all ffilters into basetable.", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* attr1_name 'attr1_ffilter' attr2_name 'attr2_ffilter' ...");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}

        if( !newTreeName )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
        
        EAIndex::fh_idx idx = getEAIndex( findexPath_CSTR );

        string host = "localhost";
        if( host_CSTR )
            host = host_CSTR;
        else
            host = idx->getConfig( CFG_IDX_HOST_K, CFG_IDX_HOST_DEF );

        string dbname = "";
        if( dbname_CSTR )
            dbname = dbname_CSTR;
        else
        {
            try {
                dbname = idx->getConfig( CFG_IDX_DBNAME_K, CFG_IDX_DBNAME_DEF, true );
            }
            catch( exception& e )
            {
                cerr << "Can't find out database name. e:" << e.what() << endl;
                exit(1);
            }
        }
            
        

        // read in the name ffilter pairs
        stringmap_t ffilterStrings;
        readAttributes( ffilterStrings, optCon, newTreeName );
        

        if( ffilterStrings.empty() )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        string newTreeCFIName = newTreeName;
        newTreeCFIName += newTreeCFIPostfix;
        string newTreeAttrNamesTable = newTreeName;
        newTreeAttrNamesTable += "_attrnames";
        
        fh_context dotferris = Resolve( "~/.ferris" );
        fh_context viewc = 0;
        fh_context ffilterc = 0;
        {
            stringstream ss;
            ss << "~/.ferris/fcatree/"
               << host << "/"
               << dbname << "/"
               << newTreeName;
            viewc = Shell::acquireContext( ss.str() );
            ss << "/ffilters";
            ffilterc = Shell::acquireContext( ss.str() );
        }

        // save our command line to a file for full exposure
        {
            stringstream ss;
            for( int i=0; i<argc; ++i )
            {
                if( starts_with( argv[i], "-" ))
                    ss << "\"" << argv[i] << "\" " ;
                else
                    ss << argv[i] << " " ;
            }
            setFile( viewc, "ferris-create-fca-tree-command", ss.str() );
        }

        
        
        
        //
        // set cfi/base table name and findexpath
        //
        setFile( viewc, "findex-path",    findexPath_CSTR ? findexPath_CSTR : EAINDEXROOT );
        setFile( viewc, "tablename-cfi",  newTreeCFIName );
        setFile( viewc, "tablename-base", newTreeName );
        //
        {
            stringstream ss;

            ss << "#!/bin/bash" << endl
               << "rm -f tablename-cfi-is-augmented" << endl
               << "echo 'alter table " << newTreeCFIName << " drop column lattice_parents; ' | psql " << dbname << endl
               << "echo 'alter table " << newTreeCFIName << " drop column lattice_children; ' | psql " << dbname << endl
               << "echo 'alter table " << newTreeCFIName << " drop column lattice_added_attrs; ' | psql " << dbname << endl
               << "echo 'alter table " << newTreeCFIName << " drop column  concept_only_support_abs; ' | psql " << dbname << endl;
            setFile( viewc, "reset-lattice.sh", ss.str(), 700 );
            
        }
        

        //
        // save the ffilter strings under their column names
        //
        stringlist_t formalContextColumns;
        for( stringmap_t::const_iterator si = ffilterStrings.begin();
             si != ffilterStrings.end(); ++si )
        {
            string rdn     = si->first;
            string ffilter = si->second;
            fh_context c = Shell::acquireSubContext( ffilterc, rdn );
            setStrAttr( c, "content", ffilter );

            formalContextColumns.push_back( rdn );
        }

        //
        // binary columns names
        //
        {
            fh_context c = Shell::acquireSubContext( viewc, "formal-context-columns" );
            setStrAttr( c, "content", Util::createSeperatedList( formalContextColumns ) );
        }
        
        
        //
        // work out what the SQL view will look like.
        //
        typedef map< string, int > BitFieldShifts_t;
        BitFieldShifts_t BitFieldShifts;
        stringlist_t sqlfuncs;
        stringstream SQLHeaderSS;
        stringstream SQLTailerINTO_SS;
        stringstream SQLTailerFROM_SS;
        stringstream SQLTailerWHERE_SS;
        stringstream SQLWhereSS;
        
        stringset_t alreadySelectedAttributes;
        alreadySelectedAttributes.insert( "url" );
        alreadySelectedAttributes.insert( "urlid" );
        alreadySelectedAttributes.insert( "docid" );

        int BitFieldSize = ffilterStrings.size() + 2;
        setFile( viewc, "bit-field-size", tostr(BitFieldSize) );
        
//        SQLHeaderSS << "create or replace view " << newTreeName << " as ";
        SQLHeaderSS << " SELECT " << nl
//                    << "     u.url    as url " << nl
//                     << "    ,d.width  as width " << nl
//                     << "    ,d.height as height " << nl
                    << "    d.urlid  as urlid " << nl
                    << "    ,d.docid  as docid " << nl
                    << "    ,B'0'::bit(" << BitFieldSize << ") " << nl
                    << nl;
        SQLTailerINTO_SS << "  INTO TABLE " << newTreeName << nl;
        SQLTailerFROM_SS << " FROM (SELECT d.* " << nl
                         << "       FROM docmap d, " << nl
                         << "          ( select max(docidtime) as ddtime, urlid " << nl
                         << "            from docmap " << nl
                         << "            group by urlid " << nl
                         << "          ) dd " << nl
                         << "       WHERE d.urlid=dd.urlid " << nl
                         << "       AND d.docidtime=dd.ddtime " << nl
                         << "       ) as d JOIN urlmap as u ON( d.urlid = u.urlid ) " << nl;
//         SQLTailerFROM_SS << " FROM (SELECT d.* " << nl
//                          << "       FROM docmap d, " << nl
//                          << "          ( select max(docidtime) as ddtime, urlid " << nl
//                          << "            from docmap " << nl
//                          << "            group by urlid " << nl
//                          << "          ) dd " << nl
//                          << "       WHERE d.urlid=dd.urlid " << nl
//                          << "       AND d.docidtime=dd.ddtime " << nl
//                          << "       ) as d," << nl
//                          << "       urlmap as u " << nl;
//         SQLTailerWHERE_SS << " where d.urlid = u.urlid " << nl;

        LG_FCA_D << "ffilterStrings.size:" << ffilterStrings.size() << endl;
        stringset_t eanamesUsed;
        stringstream conSS;
        conSS << " host=" << host;
        conSS << " dbname=" << dbname;
        connection con( conSS.str() );

        
        stringmap_t lifted_columns;
        stringset_t columns_in_docmap;
        {
            work trans( con, "getting the schema..." );
            result res = trans.exec( "select * from docmap where docid < 0" );

            result::tuple::size_type res_col_sz = res.columns();
            for( int i=0; i<res_col_sz; ++i )
            {
                columns_in_docmap.insert( res.column_name( i ) );
            }
        }

        stringstream ReadCFISQLSS;
        int BitFieldShift = 0;
        MetaEAIndexerInterface::BuildQuerySQLTermInfo_t termInfo;
        for( stringmap_t::const_iterator si = ffilterStrings.begin();
             si != ffilterStrings.end(); ++si )
        {
            string rdn            = si->first;
            string ffilter_string = si->second;
        
            fh_eaquery q = EAIndex::Factory::makeEAQuery( ffilter_string, idx );

            std::stringstream LocalSQLHeaderSS;
            std::stringstream LocalSQLWherePredicatesSS;
            std::stringstream LocalSQLTailerSS;
            stringset_t lookupTablesUsed;
            bool queryHasTimeRestriction;
            std::string DocIDColumn;
            
            q->getQuerySQL( LocalSQLHeaderSS,
                            LocalSQLWherePredicatesSS,
                            LocalSQLTailerSS,
                            lookupTablesUsed,
                            queryHasTimeRestriction,
                            DocIDColumn,
                            eanamesUsed,
                            termInfo );
            LG_FCA_D << "rdn:" << rdn << " SQLWherePredicatesSS:" << LocalSQLWherePredicatesSS.str() << endl;

//             if( SQLHeader.empty() || queryHasTimeRestriction )
//                 SQLHeader = LocalSQLHeaderSS.str();
//             if( SQLTailer.empty() || queryHasTimeRestriction )
//                 SQLTailer = LocalSQLTailerSS.str();

//             SQLWhereSS << "," << LocalSQLWherePredicatesSS.str()
//                        << " as \"" << rdn << "\" " << nl;

            SQLHeaderSS << " 	|  " << nl
                        << "CASE (" << LocalSQLWherePredicatesSS.str() << ")" << nl
                        << " WHEN 't' THEN B'1'::bit(" << BitFieldSize <<  ") >> "
                        << BitFieldShift << " " << nl
                       << " ELSE B'0'::bit(" << BitFieldSize << ") " << nl
                       << "END" << endl;

            {
                BitFieldShifts[rdn] = BitFieldShift;
                {
//                     sqlfuncs.push_front(
//                         (string)"drop function "
//                         + getBitFunctionName( newTreeName, rdn ) + "(bit varying);" );
                    stringstream ss;
                    ss << "CREATE OR REPLACE FUNCTION "
                       << getBitFunctionName( newTreeName, rdn )
                       << "( bit varying ) RETURNS boolean IMMUTABLE STRICT AS '" << nl
                       << "DECLARE" << nl
                       << "    bv ALIAS FOR $1;" << nl
                       << "BEGIN" << nl
                       << "RETURN ( bv::bit(" << BitFieldSize << ") "
                       << "     & (B''1''::bit(" << BitFieldSize << ") >> " << BitFieldShift << ")"
                       << "     = (B''1''::bit(" << BitFieldSize << ") >> " << BitFieldShift << "));"
                       << "END;" << nl
                       << "' LANGUAGE plpgsql;";
                    sqlfuncs.push_front( ss.str() );
                }
                {
//                     sqlfuncs.push_front(
//                         (string)"drop function "
//                         + getBitFunctionName( newTreeName, rdn ) + ";" );
                    stringstream ss;
                    ss << "CREATE OR REPLACE FUNCTION "
                       << getBitFunctionName( newTreeName, rdn )
                       << "() RETURNS bit(" << BitFieldSize << ") IMMUTABLE STRICT AS '" << nl
                       << "DECLARE" << nl
                       << "BEGIN" << nl
                       << "RETURN (B''1''::bit(" << BitFieldSize << ") >> " << BitFieldShift << ");"
                    << "END;" << nl
                    << "' LANGUAGE plpgsql;";
                    sqlfuncs.push_front( ss.str() );
                }
                
                
                ReadCFISQLSS << ", " << getBitFunctionName( newTreeName, rdn )
                             << "(d.bitf) as \"" << rdn << "\" ";
            }
            ++BitFieldShift;
        }
        SQLHeaderSS << "        as bitf " << nl;


        ReadCFISQLSS << " from  " << newTreeCFIName << " d  order by id desc ";
        setFile( viewc, "read-cfi-sql-tail", ReadCFISQLSS );
        
        {
//             sqlfuncs.push_front(
//                 (string)"drop function "
//                 +  "meta_" + getBitFunctionName( newTreeName, "zero_bitf",false ) );
            
            stringstream ss;
            ss << "CREATE OR REPLACE FUNCTION "
               << "meta_" << getBitFunctionName( newTreeName, "zero_bitf",false )
               << "() RETURNS bit(" << BitFieldSize << ") IMMUTABLE STRICT AS '" << nl
               << "DECLARE" << nl
               << "BEGIN" << nl
               << "RETURN (B''0''::bit(" << (BitFieldSize) << "));"
               << "END;" << nl
               << "' LANGUAGE plpgsql;";
            sqlfuncs.push_front( ss.str() );
        }
        {
//             sqlfuncs.push_front(
//                 (string)"drop function "
//                 + "meta_" + getBitFunctionName( newTreeName, "bitf_size",false ) );

            stringstream ss;
            ss << "CREATE OR REPLACE FUNCTION "
               << "meta_" << getBitFunctionName( newTreeName, "bitf_size",false )
               << "() RETURNS int IMMUTABLE STRICT AS '" << nl
               << "DECLARE" << nl
               << "BEGIN" << nl
               << "RETURN " << (BitFieldSize-3) << " ;"
               << "END;" << nl
               << "' LANGUAGE plpgsql;";
            sqlfuncs.push_front( ss.str() );
        }

        
        if( eanamesUsed.find("width") != eanamesUsed.end() )
        {
            eanamesUsed.insert("height");
        }
        if( eanamesUsed.find("height") != eanamesUsed.end() )
        {
            eanamesUsed.insert("width");
        }

        for( stringset_t::const_iterator si = eanamesUsed.begin();
             si != eanamesUsed.end(); ++si )
        {
            string rdn = *si;

            LG_FCA_D << "eanamesUsed ITER:" << rdn << endl;
            if( columns_in_docmap.find( rdn ) != columns_in_docmap.end() )
            {
                LG_FCA_D << "LIFTING:" << rdn << endl;
                lifted_columns[ rdn ] = makeNonNormalizedLiftString( rdn );
            }
            else if( !dontDenormalizeEAIntoBaseTable )
            {
                if( alreadySelectedAttributes.find( rdn ) != alreadySelectedAttributes.end() )
                    continue;
                if( rdn == "name" )
                    continue;
                if( starts_with( rdn, "emblem:" ))
                    continue;
                
                //
                // This is tricky, we are going to denormalize the EA value
                // into the new base table.
                //
                string tmptableName = (string)"tab_" + rdn;
                tmptableName = Util::replace_all( tmptableName, '-', '_' );
                MetaEAIndexerInterface::
                    BuildQuerySQLTermInfo_t::const_iterator ti = termInfo.find( rdn );
                
                if( ti == termInfo.end() )
                    continue;

                string lookupTableName = ti->second.m_lookupTable;
                int attrtype = ti->second.m_attrType;
                stringstream ss;
                ss << ",  \"" << tmptableName << "\".docid as \""
                   << Util::replace_all( rdn, '-', '_' ) << "\" " << nl;
                lifted_columns[ rdn ] = ss.str();

                SQLTailerFROM_SS
                    << "LEFT OUTER JOIN  " << nl
                    << "(   SELECT docattrs.docid as docid, " << nl
                    << "           " << lookupTableName << ".attrvalue as value" << nl
                    << "      FROM docattrs, attrmap , " << lookupTableName << nl
                    << "      WHERE" << nl
                    << "      ((" << nl
//                    << "            docattrs.docid = d.docid and " << nl
//                    << "        " << lookupTableName << ".attrvalue FIXME! " << nl
                    << "        " << lookupTableName << ".vid = docattrs.vid" << nl
                    << "       and  attrmap.attrname='" << rdn << "'" << nl
                    << "       and ( attrmap.attrtype='" << attrtype << "' ";

                if( attrtype == MetaEAIndexerInterface::ATTRTYPEID_CIS )
                    SQLTailerFROM_SS << nl
                                     << "  or attrmap.attrtype='"
                                     << MetaEAIndexerInterface::ATTRTYPEID_STR
                                     << "' ) " << nl;
                else
                    SQLTailerFROM_SS << " ) " << nl;
                
                SQLTailerFROM_SS
                    << "       and  attrmap.attrid=docattrs.attrid" << nl
                    << "      ))" << nl
//                     << "      UNION ALL SELECT null as value"
//                     << "      LIMIT 1 " << nl
                    << "  ) as \"" << tmptableName << "\" " << nl
                    << "       ON (\"" << tmptableName << "\".docid = d.docid)" << nl
                    << nl;

//                 SQLTailerWHERE_SS
//                     << " and " << tmptableName << ".docid = d.docid " << nl;
            }
            
        }
        
        //
        // For those EA which are mentioned in the various ffilters
        // that make up the formal context, we should copy those EA
        // into the new basetable.
        //
        {
            for( stringmap_t::const_iterator ei = lifted_columns.begin();
                 ei != lifted_columns.end(); ++ei )
            {
                string rdn = ei->first;
                string sql = ei->second;
                
                if( alreadySelectedAttributes.find( rdn ) != alreadySelectedAttributes.end() )
                    continue;
                
                SQLHeaderSS << sql << nl;
            }
        }

        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////
        
//        SQLHeaderSS << " ,'t'='t'  as ferris_top_generator " << nl;
//        SQLHeaderSS << " ,'t'<>'t' as ferris_bottom_generator " << nl;

        // create the database view
        {
            stringstream sqlss;
            sqlss << SQLHeaderSS.str() << nl << SQLWhereSS.str() << nl
                  << SQLTailerINTO_SS.str()
                  << SQLTailerFROM_SS.str()
                  << SQLTailerWHERE_SS.str()
                  << ";" << endl;

            LG_FCA_D << "Base table generator SQL:"
                     << sqlss.str()
                     << endl;
            cerr << "Base table generator SQL:"
                 << sqlss.str()
                 << endl;
            
            fh_context c = Shell::acquireSubContext( viewc, "recreate-sql" );
            setStrAttr( c, "content", sqlss.str() );

//             try
//             {
//                 work trans( con, "dropping the view..." );
//                 stringstream dropSS;
//                 dropSS << "drop view " << newTreeName << ";";
//                 trans.exec( dropSS.str() );
//                 trans.commit();
//             }
//             catch( exception& e )
//             {}
            try
            {
                work trans( con, "dropping the table..." );
                stringstream dropSS;
                dropSS << "drop table " << newTreeName << ";";
                trans.exec( dropSS.str() );
                trans.commit();
            }
            catch( exception& e )
            {}
            try
            {
                work trans( con, "dropping the table..." );
                stringstream dropSS;
                dropSS << "drop table " << newTreeAttrNamesTable << ";";
                trans.exec( dropSS.str() );
                trans.commit();
            }
            catch( exception& e )
            {}

            /////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////

            {
                fh_context c = Shell::acquireSubContext( viewc, "bitf-column-names" );
                setStrAttr( c, "content", Util::createSeperatedList( formalContextColumns, '\n' ) );

                stringstream ss;
                ss << "create table " << newTreeAttrNamesTable << " ( "
                   << "   id serial primary key,"
                   << "   name varchar(100) NOT NULL"
                   << "  ) without oids "
                   << " "
                   << endl;
                work trans( con, "setting up attrnames table..." );
                trans.exec( ss.str() );

                for( BitFieldShifts_t::const_iterator ci = BitFieldShifts.begin();
                     ci != BitFieldShifts.end(); ++ci )
                {
                    string name = ci->first;
                    int    id   = ci->second;
                
                    stringstream ss;
                    ss << "insert into " << newTreeAttrNamesTable << " (id,name) values ("
                       << id << ",'" << name  << "'"
                       << ");" << endl;
                    LG_FCA_D << ss.str();
                    trans.exec( ss.str() );
                }
                
                {
                    stringstream ss;
                    ss << ""
                       << "create or replace function " << newTreeName << "_n( int[] ) returns varchar[] AS '"
                       << "DECLARE"
                       << "	MyArgument alias for $1;"
                       << "	sz int := 300; "
                       << "	i  int := 1; "
                       << "	ret varchar[]; "
                       << "	Rec RECORD; "
                       << "BEGIN "
                       << "	ret := ARRAY[100]; "
                       << "	FOR Rec IN select name from wn2_attrnames where id = ANY  ( MyArgument ) LOOP "
                       << "		ret[ i ] = Rec.name; "
                       << "		i := i + 1; "
                       << "	END LOOP; "
                       << "	RETURN ret; "
                       << "END; "
                       << "' LANGUAGE plpgsql; ";
                    LG_FCA_D << "SQL:" << ss.str() << endl;
                    trans.exec( ss.str() );
                }
                {
                    stringstream ss;
                    ss << ""
                       << "create or replace function " << newTreeName << "_n( bit varying ) returns varchar[] AS '"
                       << "DECLARE"
                       << "	MyArgument alias for $1;"
                       << "BEGIN "
                       << " RETURN " << newTreeName << "_n(" << newTreeName << "_intvec(MyArgument));"
                       << "END; "
                       << "' LANGUAGE plpgsql; ";
                    LG_FCA_D << "SQL:" << ss.str() << endl;
                    trans.exec( ss.str() );
                }
                
            
                trans.commit();
            }

            /////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////
            
            for( stringlist_t::const_iterator si = sqlfuncs.begin();
                 si != sqlfuncs.end(); ++si )
            {
                LG_EAIDX_D << "SQL:  " << *si << endl;
            }

            cerr << "making the view..." << endl;
            {
                work trans( con, "making the view..." );
                LG_EAIDX_D << "SQL:  " << sqlss.str() << endl;
                trans.exec( sqlss.str() );
                trans.commit();
            }
            cerr << "making the functions..." << endl;
            work trans( con, "making the view..." );
            LG_FCA_D << "MADE TABLE" << endl;
            for( stringlist_t::const_iterator si = sqlfuncs.begin();
                 si != sqlfuncs.end(); ++si )
            {
                LG_EAIDX_D << "SQL:  " << *si << endl;
                LG_FCA_D << "SQL:" << *si << endl;
                trans.exec( *si );
            }
            cerr << "making the index..." << endl;
            {
                stringstream ss;
                ss << "create index " << newTreeName << "docid on " << newTreeName << "(docid)";
                LG_EAIDX_D << "SQL:  " << ss.str() << endl;
                trans.exec( ss.str() );
            }
            string intvecFunctionName = (string)newTreeName + "_intvec";
            {
                stringstream ss;
                ss << "CREATE OR REPLACE FUNCTION " << intvecFunctionName
                   << " ( bit varying ) RETURNS int[] IMMUTABLE STRICT AS ' "
                   << " DECLARE "
                   << "	bv alias for $1;"
                   << "	sz int := " << BitFieldSize << "-1;"
                   << "	ret int[];"
                   << "	tmp int;"
                   << "BEGIN"
                   << "	tmp := -1;"
                   << "	FOR i IN 0..sz LOOP"
                   << "		IF (bv::bit(" << BitFieldSize << ") "
                   << " & (B''1''::bit(" << BitFieldSize << ") >> i )) "
                   << " = (B''1''::bit(" << BitFieldSize << ") >> i)"
                   << "		THEN"
                   << "			tmp := i;"
                   << "			EXIT;"
                   << "		END IF;"
                   << "	END LOOP;"
                   << "	IF tmp = -1 "
                   << "	THEN"
                   << "		RETURN ret;"
                   << "	END IF;"
                   << "	ret := ARRAY[tmp];"
                   << "	tmp := tmp + 1;"
                   << "	FOR i IN tmp..sz LOOP"
                   << "		IF (bv::bit(" << BitFieldSize << ")"
                   << " & (B''1''::bit(" << BitFieldSize << ") >> i ))"
                   << " = (B''1''::bit(" << BitFieldSize << ") >> i)"
                   << "		THEN"
                   << "			ret = array_append(ret,i); "
                   << "		END IF;"
                   << "	END LOOP;"
                   << "	RETURN ret;"
                   << "END;"
                   << "' LANGUAGE plpgsql;";
                    
                LG_EAIDX_D << "BITF2INTVEC FUNC:" << ss.str() << endl;
                trans.exec( ss.str() );
            }
            {
                stringstream ss;
                ss << "create index " << newTreeName << "bitfsubeq on " << newTreeName
                   << " using gist ( " << intvecFunctionName << "(bitf) gist__int_ops, "
                   << " (#( " << intvecFunctionName << "(bitf))) gist_int4_ops )";
                LG_EAIDX_D << "SQL:  " << ss.str() << endl;
                trans.exec( ss.str() );
            }
            
            trans.commit();

            
//             {
//                 work trans( con, "making indexes..." );
            
//                 {
//                     stringstream idxss;
//                     idxss << "create index urlidx on " << newTreeName
//                           << " ( url ) " << nl;
//                     trans.exec( idxss.str() );
//                 }
//                 {
//                     stringstream idxss;
//                     idxss << "create index urlididx on " << newTreeName
//                           << " ( urlid ) " << nl;
//                     trans.exec( idxss.str() );
//                 }
//             trans.commit();
//             }
            
        }

        cerr << "setting up ~/.ferris/tree files..." << endl;
        
        string CFICmd = "";
        string LatticeFromCFICmd = "";

        stringstream recreate_concept_lattice_SS;
        recreate_concept_lattice_SS << "#!/bin/bash" << endl;
        // workout what the command will be to make the CFI table
        {
            work trans( con, "as string..." );
            stringlist_t formalContextBitFieldFunctions;
            stringlist_t formalContextColumnsQuoted;
            stringlist_t::const_iterator ce = formalContextColumns.end();
            for( stringlist_t::const_iterator ci = formalContextColumns.begin();
                 ci != ce; ++ci )
            {
                formalContextColumnsQuoted.push_back( DQuoteStr( trans, *ci ));
                formalContextBitFieldFunctions.push_front(
                    getBitFunctionName( newTreeName, *ci ) + "(d)" );
            }

            int attributeCount = ffilterStrings.size();
            string commaSepColumnsSQL = Util::createSeperatedList( formalContextColumnsQuoted );
            string commaSepBitFFSQL =
                Util::createSeperatedList( formalContextBitFieldFunctions );
            
//             {
                
//             stringstream cmdSS;
//             cmdSS << "aprioripg -tc -n" << (MaxItemSetSize) << " -s0 -P " << host
//                   << " " << dbname << " "
//                   << " 'select " << commaSepColumnsSQL
// //                  << " 'select " << commaSepBitFFSQL
// //                  << " ,ferris_top_generator "
// //                  << " ,ferris_bottom_generator "
//                   << " FROM " << newTreeName << " d "
// //                  << " GROUP BY " << commaSepColumnsSQL
//                   << " ' "
//                   << " " << newTreeCFIName;
            
//             setFile( viewc, "apriori-commandline", cmdSS );
//             CFICmd = cmdSS.str();
//             recreate_concept_lattice_SS << cmdSS.str() << endl;
//             }
            
            
            {
                // FIXME: -s1 means 1% not one object!
                stringstream cmdSS;
                int MaxItemSetSize = 7;
                double minSupport = 0.05;
                int TargetLatticeSize = 500;
                
//                cmdSS << "#!/bin/bash" << endl
//                      << "minsup=${1:-" << minSupport << "};" << endl;
                cmdSS << "aprioripg -Vbitf -B -tc -n" << (MaxItemSetSize)
                      << " -s" << minSupport
                      << " -T" << TargetLatticeSize
                      << " -D "
                      << " -P " << host
                      << " " << dbname << " ";
                cmdSS << " 'select bitf "; // << commaSepBitFFSQL
//                 bool v = true;
//                 for(BitFieldShifts_t::const_iterator iter = BitFieldShifts.begin();
//                     iter != BitFieldShifts.end(); ++iter )
//                 {
//                     if( v ) v = false;
//                     else    cmdSS << " , ";
//                     string rdn = iter->first;
//                     int shift  = iter->second;

//                     cmdSS << getBitFunctionName( newTreeName, rdn )
//                              << "(d.bitf) as \"" << EANameToSQLColumnName(rdn) << "\" ";

// //                     cmdSS << " ( d.bitf::bit(" << BitFieldSize << ") "
// //                           << "     & (B'1'::bit(" << BitFieldSize << ") >> " << shift << ")"
// //                           << "     = (B'1'::bit(" << BitFieldSize << ") >> " << shift << "))"
// //                           << " as " << EANameToSQLColumnName(rdn) << " " << endl;
//                 }
                cmdSS << " FROM " << newTreeName << " d "
                      << " ' "
                      << " " << newTreeCFIName;
            
                CFICmd = cmdSS.str();
                recreate_concept_lattice_SS << cmdSS.str() << endl;
                setFile( viewc, "apriori-commandline", cmdSS, 700 );
            }
        }
        
//         {
//             stringstream cmdSS;
//             cmdSS << "#!/bin/bash" << endl
//                   << "tmpfile=/tmp/out.txt;" << endl
//                   << "tc=0;" << endl
//                   << endl
//                   << "for supp in \"0.9\" \"0.8\" \"0.7\" \"0.6\" \"0.5\" \"0.4\" \"0.3\" \"0.2\" \"0.1\" \"0.05\"" << endl
//                   << "do" << endl
//                   << "./apriori-commandline $supp >|$tmpfile 2>&1" << endl
//                   << "tc=$(tail -1 $tmpfile | cut -f 1 -d ' ' | cut -b\"2-\")" << endl
//                   << "echo $tc;" << endl
//                   << "if [ $tc -gt \"1000\" ]; " << endl
//                   << " then " << endl
//                   << "   break;" << endl
//                   << " fi " << endl
//                   << "done" << endl   
//                   << endl;
//             setFile( viewc, "apriori-commandline-adaptive", cmdSS, 700 );
//         }
        

        // workout what the command is to set child/parent links in CFI table
        {
            stringstream cmdSS;
            cmdSS << "ferris-lattice-from-cfi --lattice-tree-path=" << viewc->getURL()
                  << "";
//             cmdSS << "ferris-lattice-from-cfi -d " << dbname
//                   << " -Q 'select * from " << newTreeCFIName << " order by id desc' "
//                   << " -o " << newTreeCFIName;

            setFile( viewc, "lattice-from-cfi-commandline", cmdSS, 700 );
            LatticeFromCFICmd = cmdSS.str();
            recreate_concept_lattice_SS << viewc->getDirPath() << "/" << "reset-lattice.sh" << endl;
            recreate_concept_lattice_SS << cmdSS.str() << endl;
        }
        
        {
            setFile( viewc, "recreate-concept-lattice-script",
                     recreate_concept_lattice_SS, 700 );
        }

        // run apriori against the database view to create the CFI
        if( !dontCreateCFI )
        {
            fh_runner r = new Runner();
            r->setCommandLine( CFICmd );
            r->setSpawnFlags( (GSpawnFlags)(r->getSpawnFlags()
                                            | G_SPAWN_STDOUT_TO_DEV_NULL
                                            | G_SPAWN_STDERR_TO_DEV_NULL ));
            r->Run();
            gint es = r->getExitStatus();
//            cerr << "apriori es:" << es << endl;
            if( es != 0 )
            {
                cerr << "Generating the set of concepts failed! error:" << es << endl;
                cerr << "CFICmd:" << CFICmd << endl;
            }
        }

//        sleep(3);
        // generate the parent/child relations
        if( !dontCreateParentChild )
        {
            fh_runner r = new Runner();
            r->setCommandLine( LatticeFromCFICmd );
            r->setSpawnFlags( (GSpawnFlags)(r->getSpawnFlags()
                                            | G_SPAWN_STDOUT_TO_DEV_NULL
                                            | G_SPAWN_STDERR_TO_DEV_NULL ));
            r->Run();
            gint es = r->getExitStatus();
//            cerr << "LatticeFromCFI es:" << es << endl;
            if( es != 0 )
            {
                cerr << "Generating the parent/child relations failed! error:" << es << endl;
                cerr << "LatticeFromCFICmd:" << LatticeFromCFICmd << endl;
            }
        }
    }
    catch( exception& e )
    {
        LG_FCA_ER << PROGRAM_NAME << " cought exception:" << e.what() << endl;
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


