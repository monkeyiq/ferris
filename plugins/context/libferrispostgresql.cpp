/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2005 Ben Martin

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

    $Id: libferrispostgresql.cpp,v 1.13 2011/01/07 21:30:17 ben Exp $

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
#include <pqxx/result>
#include <pqxx/cursor>
#include <fstream>

using namespace PGSTD;
using namespace pqxx;

#include <FerrisContextPlugin.hh>
#include <Trimming.hh>
#include <FerrisDOM.hh>
#include <FerrisCreationPlugin.hh>
#include <FerrisBoost.hh>

#include <libcommonsqldbapi.hh>
#include <libferrispostgresqlshared.hh>

using namespace std;

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

//     /**
//      * Modeled on StateLessEAHolder but allowing a string to define the type of the stateless
//      * class, so you can have many children of C++ Class FooContext, and for example, five
//      * different directories each with their own set of SLEA.
//      */
//     template <class ChildCtx, class ParentCtx = Context>
//     class FERRISEXP_CTXPLUGIN DynamicClassedStateLessEAHolder
//         :
//         public StateLessEAHolder< ChildCtx, ParentCtx >
//     {
//     protected:
//         typedef DynamicClassedStateLessEAHolder< ChildCtx, ParentCtx > _Self;
//         typedef StateLessEAHolder< ChildCtx, ParentCtx > _Base;
        
//         typedef StateLessEAHolderTraits<ChildCtx,ParentCtx>    _Traits;
//         typedef typename _Traits::_ParentCtx            _ParentCtx;
//         typedef typename _Traits::_ChildCtx             _ChildCtx;
//         typedef typename _Traits::StateLessIEA_t        StateLessIEA_t;
//         typedef typename _Traits::StateLessIOEA_t       StateLessIOEA_t;
//         typedef typename _Traits::StateLessIOClosedEA_t StateLessIOClosedEA_t;
//         typedef typename _Traits::GetIStream_Func_t     GetIStream_Func_t;
//         typedef typename _Traits::GetIOStream_Func_t    GetIOStream_Func_t;
//         typedef typename _Traits::IOStreamClosed_Func_t IOStreamClosed_Func_t;

//         static FERRIS_STD_HASH_SET< string >& getCounterHash()
//             {
//                 static FERRIS_STD_HASH_SET< string > ret;
//                 return ret;
//             }
        
//         bool isStateLessEAVirgin( const std::string& s )
//             {
//                 bool ret = getCounterHash().insert( s ).second;
//                 return ret;
//             }

//     private:

//         typedef FERRIS_STD_HASH_MAP< std::string, EA_Atom* > SLAttributes_t;
//         SLAttributes_t* getStateLessAttrs_cache;
        
//         typedef FERRIS_STD_HASH_MAP< string, AttributeCollection::SLAttributes_t* > DynStateLessAttrsHolder_t;
//         DynStateLessAttrsHolder_t DynStateLessAttrsHolder;

//         static DynStateLessAttrsHolder_t& getDynStateLessAttrsHolder()
//             {
//                 DynStateLessAttrsHolder_t ret;
//                 return ret;
//             }
        
//         void DynamicClassedStateLessEAHolder_setup( const std::string& className )
//             {
//                 SLAttributes_t* newcol = getDynStateLessAttrsHolder()[ className ];
//                 if( !newcol )
//                 {
//                     newcol = new SLAttributes_t();
//                     getDynStateLessAttrsHolder()[ className ] = newcol;
//                 }
//                 getStateLessAttrs_cache = newcol;
//             }
//         virtual SLAttributes_t* getStateLessAttrs()
//             {
//                 cerr << "DynClass::getStateLessAttrs()" << endl;
//                 return getStateLessAttrs_cache;
//             }
        
        
//     protected:
        
//         DynamicClassedStateLessEAHolder( const std::string& className,
//                                          const fh_context& parent,
//                                          const string& rdn )
//             :
//             _Base( parent, rdn )
//             {
//                 DynamicClassedStateLessEAHolder_setup( className );
//             }
        
//     };
    


    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    
    typedef vector< string > PrimaryKeyNames_t;
    class PostgreSQLContext;

    class FERRISEXP_CTXPLUGIN PostgreSQLTupleContext
        :
        public CommonSQLDBTupleContext< PostgreSQLTupleContext, PostgreSQLContext >
        //        public DynamicClassedStateLessEAHolder< PostgreSQLTupleContext, CommonSQLDBTupleContext< PostgreSQLTupleContext, PostgreSQLContext > >
    {
        typedef PostgreSQLTupleContext                            _Self;
        typedef CommonSQLDBTupleContext< PostgreSQLTupleContext, PostgreSQLContext > _Base;
//        typedef DynamicClassedStateLessEAHolder< PostgreSQLTupleContext, CommonSQLDBTupleContext< PostgreSQLTupleContext, PostgreSQLContext > > _Base;

    public:

        void
        priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
        {
            LG_PG_D << "priv_FillCreateSubContextSchemaParts()" << endl;
            _Base::priv_FillCreateSubContextSchemaParts( m );
        }
        
        void setIter( result::const_iterator& row )
            {
                result::tuple::const_iterator e = row->end();
                for( result::tuple::const_iterator cur = row->begin(); cur != e; ++cur )
                {
                    string k = cur->name();
                    if( !k.length() )
                        continue;

                    string v = "";
                    cur->to( v );

                    LG_PG_D << "PostgreSQLTupleContext() k:" << k << endl;
                    LG_PG_D << "PostgreSQLTupleContext() v:" << v << endl;
                    
                    kvm[k]=v;
//                     if( !isAttributeBound( k ) )
//                         addAttribute( k,
//                                       this, &_Self::getValueStream,
//                                       this, &_Self::getValueStream,
//                                       this, &_Self::setValueStream,
//                                       getSchemaType( k ) );

                    string lowerk = tolowerstr()( k );
                    kvm[ lowerk ]=v;
//                     if( !isAttributeBound( lowerk ) )
//                         addAttribute( lowerk,
//                                       this, &_Self::getValueStream,
//                                       this, &_Self::getValueStream,
//                                       this, &_Self::setValueStream,
//                                       XSD_BASIC_STRING );
                }
            }
        
        PostgreSQLTupleContext( const fh_context& parent, const string& rdn,
                                result::const_iterator& m_iter )
            :
            _Base( parent, rdn )
            {
                setIter( m_iter );
                
                string className = parent->getURL();
                bool force = AttributeCollection::isStateLessEAVirgin( className );
                LG_PG_D << "PostgreSQLTupleContext(ctor) force:" << force
                        << " rdn:" << rdn << " className:" << className
                        << endl;
                setup_DynamicClassedStateLessEAHolder( className );
                if( force )
                {
#define SLEA tryAddStateLessAttribute

                    result::tuple::const_iterator e = m_iter->end();
                    for( result::tuple::const_iterator cur = m_iter->begin(); cur != e; ++cur )
                    {
                        string k = cur->name();
                        if( !k.length() )
                            continue;

                        LG_PG_D << "Adding SLEA for k:" << k << endl;
                        SLEA( k,
                              &_Self::SL_getValueStream,
                              &_Self::SL_getValueStream,
                              &_Self::SL_setValueStream,
                              getSchemaType( k ) );

                        string lowerk = tolowerstr()( k );
                        SLEA( lowerk,
                              &_Self::SL_getValueStream,
                              &_Self::SL_getValueStream,
                              &_Self::SL_setValueStream,
                              XSD_BASIC_STRING );
                    }
                
#undef SLEA
                }
                createStateLessAttributes( force );
            }

        virtual void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    LG_PG_D << "Context::createStateLessAttributes() path:" << getDirPath() << endl;
                    _Base::createStateLessAttributes( force );
                }
            }
        
        virtual bool supportsReClaim()
            {
                return true;
            }
        
        connection& getConnection();
        XSDBasic_t getSchemaType( const std::string& eaname );

    protected:

        virtual void executeUpdate( const std::string& sqlString )
            {
//                 Query query = getConnection( this )->query();
//                 (std::ostream&)query << sqlString; 
//                 query.execute();

                connection& con = getConnection();
                work trans( con, "executeUpdate..." );
                trans.exec( sqlString );
                trans.commit();
            }

        virtual std::string getTableName()
            {
                return getParent()->getDirName();
            }
        
    };

    struct PostgreSQLTupleContextCreator
    {
        result::const_iterator& m_iter;
            
        PostgreSQLTupleContextCreator( result::const_iterator& m_iter )
            :
            m_iter( m_iter )
            {}
        
        PostgreSQLTupleContext* create( Context* parent, const std::string& rdn ) const
            {
                PostgreSQLTupleContext* ret = new PostgreSQLTupleContext( parent, rdn, m_iter );
                return ret;
            }
        void setupExisting( PostgreSQLTupleContext* fc ) const
            {
                fc->setIter( m_iter );
            }
        void setupNew( PostgreSQLTupleContext* fc ) const
            {}
    };

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    
    class FERRISEXP_CTXPLUGIN PostgreSQLContext
        :
        public CommonSQLContext< PostgreSQLContext >
    {
        typedef PostgreSQLContext _Self;
        typedef CommonSQLContext< PostgreSQLContext > _Base;


        string setupQuery()
            {
                fh_stringstream ss;
        
                if( m_tableQuerySQL.length() )
                {
                    LG_PG_D << "m_tableQuerySQL:" << m_tableQuerySQL << endl;
                    ss << m_tableQuerySQL;
                }
                else
                {
                    ss << "select * from " << getDirName();
                }

                LG_PG_D << "setupQuery() SQL:" << tostr(ss) << endl;
                return tostr( ss );
            }
        
    public:

        PostgreSQLContext( const fh_context& parent, const string& rdn, string QuSQL = "" )
            :
            _Base( parent, rdn, QuSQL )
            {
                DeterminePrimaryKey();
                createStateLessAttributes();
            }
        
        virtual ~PostgreSQLContext()
            {
            }

    protected:

        virtual bool supportsReClaim()
            {
                return true;
            }
        
        connection& getConnection();
        

        virtual fh_iostream  getRealStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   exception)
            {
                bool virgin = true;
                fh_stringstream ret;

                connection& con = getConnection();
                work trans( con, "getRealStream..." );
                result res = trans.exec( setupQuery() );

//                 virgin = true;
//                 vector<string> colNames = s.GetNames();
//                 for (vector<string>::iterator it = colNames.begin(); it != colNames.end(); ++it )
//                 {
//                     if(!virgin) ret << ",";
//                     virgin = false;
                        
//                     string k = *it;
//                     if( !k.length() )
//                         continue;
                        
//                     ret << k;
//                 }
//                 ret << endl;


                // Print out all rows and columns from our query
                for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
                {
                    virgin = true;
                    result::tuple::const_iterator e = iter->end();
                    for( result::tuple::const_iterator cur = iter->begin();
                         cur != e; ++cur )
                    {
                        if(!virgin) ret << ",";
                        virgin = false;

                        string k = "";
                        cur.to( k );
                        
                        if( !k.length() )
                            continue;
                        ret << k;
                    }
                    ret << endl;
                }
                
                return ret;
            }

        virtual void populateKeyNames( PrimaryKeyNames_t& KeyNames )
            {
                LG_PG_D << "populateKeyNames(top)" << endl;
                
                connection& con = getConnection();
                work trans( con, "populateKeyNames..." );

                string tableName = getDirName();

                //
                // pg functions that include varchar parameters make bad SQL
                //
                if( string::npos != tableName.find("'") )
                    return;
                
                string oid;
                {
                    stringstream sqlss;
                    sqlss << " SELECT c.oid,"
                          << "   n.nspname, "
                          << "   c.relname "
                          << " FROM pg_catalog.pg_class c "
                          << "      LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace "
                          << "where pg_catalog.pg_table_is_visible(c.oid) "
                          << "and c.relname = '" << tableName << "' "
                          << "ORDER BY 2, 3; ";

                    LG_PG_D << "populateKeyNames() IOD SQL:" << sqlss.str() << endl;
                    result res = trans.exec( sqlss.str() );

                    // its a function //
                    if( res.empty() )
                    {
                        return;
                    }
                    
                    res[0][0].to(oid);
                }
                
                stringstream sqlss;
                sqlss << "SELECT a.attname as attname,"
                      << "  pg_catalog.format_type(a.atttypid, a.atttypmod) as format_type, "
                      << "  (SELECT substring(d.adsrc for 128) FROM pg_catalog.pg_attrdef d "
                      << "   WHERE d.adrelid = a.attrelid AND d.adnum = a.attnum AND a.atthasdef) as ex, "
                      << "    a.attnotnull as notnull, a.attnum as attnum "
                      << "FROM pg_catalog.pg_attribute a "
                      << "WHERE a.attrelid = '" << oid << "' AND a.attnum > 0 AND NOT a.attisdropped "
                      << "ORDER BY a.attnum; ";
                result res = trans.exec( sqlss.str() );

                LG_PG_D << "populateKeyNames() table:" << tableName
                        << " res.size:" << res.size()
                        << endl;
                for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
                {
                    string attname;
                    string sqltype;
                    string ex;
                    string notnull;

                    iter["attname"].to(attname);
                    iter["format_type"].to(sqltype);
                    iter["ex"].to(ex);
                    iter["notnull"].to(notnull);

                    bool autoinc = ( string::npos != ex.find("nextval") );
                    bool allowNull = (notnull == "f");
                    KeyNamesType[attname] = RowMetaData( sqltype, autoinc, allowNull );
                    KeyNames.push_back( attname );
                    LG_PG_D << " attrname:" << attname << " type:" << sqltype << endl;
                }
                
                LG_PG_D << "populateKeyNames(end)" << endl;
            }

        virtual bool isDir()
            {
                return true;
            }
        
        void priv_read()
            {
                LG_PG_D << "PostgreSQLContext::read(1) HaveReadDir:" << getHaveReadDir()
                        << " m_primaryKeyNames.size:" << m_primaryKeyNames.size() << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                clearContext();
                LG_PG_D << "priv_read(top) path:" << getDirPath() << endl;

                getKeyNames();
                {
                    connection& con = getConnection();
                    work trans( con, "PostgreSQLContext read..." );
                    LG_PG_D << "read() sql:" << setupQuery() << endl;
                    result res = trans.exec( setupQuery() );

                    for (result::tuple::size_type c = 0; c < res.columns(); ++c)
                    {
                        string k= res.column_name(c);
                        if( !k.length() )
                            continue;

                        string v;
                        if( k.length() )
                        {
//                             cerr << "colname...k:" << k
//                                  << " schema:" << getSchemaType( k ) 
//                                  << endl;
                            
                            if( !isAttributeBound( k ) )
                                addAttribute( k, v, getSchemaType( k ) );
                        
                            string lowerk = tolowerstr()( k );
                            if( !isAttributeBound( lowerk ) )
                                addAttribute( lowerk, v, getSchemaType( k ) );
                        }
                    }
                
                    /*
                     * Create a file, and some EA for each row.
                     */
                    for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
                    {
                        string rdn = getPrimaryKeyRdn( m_primaryKeyNames, iter );
                        LG_PG_D << "rdn:" << rdn << endl;

//                         PostgreSQLTupleContext* cc = 0;
//                         cc = priv_ensureSubContext( rdn, cc );
//                         cc->setIter( iter );

                        PostgreSQLTupleContext* cc = 0;
                        cc = priv_ensureSubContext(
                            rdn, cc, PostgreSQLTupleContextCreator( iter ) );
                        


                            
                    
//                     if( !priv_isSubContextBound( rdn ) )
//                     {
//                         PostgreSQLTupleContext* child = new PostgreSQLTupleContext( this, rdn, iter );
//                         addNewChild( child );
//                     }
                    }
                }
                
                LG_PG_D << "priv_read(done) HaveReadDir:" << getHaveReadDir()
                        << " path:" << getDirPath() << endl;
            }

        std::string getPrimaryKeyRdn( PrimaryKeyNames_t& pkn, result::const_iterator& tuple )
            {
                string rdn = "";
                bool v = true;
                for( PrimaryKeyNames_t::iterator iter = pkn.begin(); iter != pkn.end(); ++iter )
                {
                    if( !v ) rdn += "-";
                    v = false;
                    string v;
                    tuple[ *iter ].to(v);
                    rdn += v;
                }
                return rdn;
            }

        virtual stringlist_t DeterminePrimaryKey_getBaseTableNamesFromViewDef( const std::string& viewDef )
            {
                int p = viewDef.find("FROM ");
                LG_PG_D << "p:" << p << endl;
                list< string > tables;
                if( p != string::npos )
                {
                    stringstream ss;
                    ss << viewDef.substr( p+5 );
                    LG_PG_D << "ss:" << ss.str() << endl;
                    while( ss )
                    {
                        string tname;
                        getline( ss, tname, " ,;" );
                        tables.push_back( tname );
                        LG_PG_D << "found table:" << tname << endl;
                        char ch;
                        while( ss >> noskipws >> ch )
                        {
                            if( ch == ',' )
                            {
                                LG_PG_D << "BREAK:" << ch << endl;
                                ss >> noskipws >> ch;
                                break;
                            }
                            LG_PG_D << "skipping:" << ch << endl;
                        }
                    }
                }

                
//                 LG_PG_D << "RESULT:" << endl;
//                 copy( tables.begin(), tables.end(), ostream_iterator<string>(cerr, "\n") );
                return tables;
            }


        virtual string DeterminePrimaryKey_forBaseTableName( work& trans,
                                                             const std::string& tableName,
                                                             stringset_t& pk,
                                                             stringset_t& uniqcols )
            {
                string oid;

                LG_PG_D << "_forBaseTableName:" << tableName << endl;
                {
                    stringstream sqlss;
                    sqlss << " SELECT c.oid,"
                          << "   n.nspname, "
                          << "   c.relname "
                          << " FROM pg_catalog.pg_class c "
                          << "      LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace "
                          << "where pg_catalog.pg_table_is_visible(c.oid) "
                          << "and c.relname = '" << tableName << "' "
                          << "ORDER BY 2, 3; ";

                    result res = trans.exec( sqlss.str() );
                    res[0][0].to(oid);
                }
                
                stringstream sqlss;
                sqlss << "SELECT c2.relname, i.indisprimary, i.indisunique, i.indisclustered, "
                      << " pg_catalog.pg_get_indexdef(i.indexrelid, 0, true)"
                      << " FROM pg_catalog.pg_class c, pg_catalog.pg_class c2, pg_catalog.pg_index i"
                      << " WHERE c.oid = '" << oid << "' AND c.oid = i.indrelid AND i.indexrelid = c2.oid"
                      << " ORDER BY i.indisprimary DESC, i.indisunique DESC, c2.relname";
                result res = trans.exec( sqlss.str() );

                LG_PG_D << "determine PK rdn:" << getDirName() << endl;

                for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
                {
                    string columns = "";
                    string indisprimary = "f";
                    string indisunique = "f";
                    iter["indisprimary"].to( indisprimary );
                    iter["indisunique"].to( indisunique );
                    iter["pg_get_indexdef"].to( columns );

                    int openbr = columns.find('(');
                    columns = columns.substr( openbr+1, columns.rfind(')') - openbr-1 );

                    if( indisprimary == "t" )
                    {
                        LG_PG_D << "PK indisprimary:" << indisprimary << " columns:" << columns << endl;

                        stringlist_t sl = Util::parseCommaSeperatedList( columns );
                        copy( sl.begin(), sl.end(), inserter( pk, pk.end() ));
                    }
                    if( indisunique == "t" )
                    {
                        stringlist_t sl = Util::parseCommaSeperatedList( columns );
                        copy( sl.begin(), sl.end(), inserter( uniqcols, uniqcols.end() ));
                    }
                }
                return oid;
            }
        
        virtual void DeterminePrimaryKey()
            {
                LG_PG_D << "DeterminePrimaryKey(1) path:" << getDirPath() << endl;
                bool v = true;

                LG_PG_D << "DeterminePrimaryKey(2) path:" << getDirPath() << endl;
                LG_PG_D << "DeterminePrimaryKey(2) m_tableQuerySQL:" << m_tableQuerySQL << endl;

                /*
                 * For queries we use the entire table as the primary key
                 */
                if( m_tableQuerySQL.length() )
                {
                    LG_PG_D << "DeterminePrimaryKey(with sql) path:" << getDirPath() << endl;

                    connection& con = getConnection();
                    work trans( con, "DeterminePrimaryKey..." );
                    result res = trans.exec( setupQuery() );
                    LG_PG_D << "DeterminePrimaryKey(with sql) have result..." << endl;

                    for (result::tuple::size_type c = 0; c < res.columns(); ++c)
                    {
                        string k= res.column_name(c);
                        if( !k.length() )
                            continue;
                        
                        LG_PG_D << "res.names(i) : " << k << endl;
                        m_primaryKeyNames.push_back( k );
                        m_recommendedEA += ",";
                        m_recommendedEA += k;
                    }
                    m_recommendedEA += ",name,primary-key";
                    LG_PG_D << "DeterminePrimaryKey(with sql) done...m_recommendedEA:" << m_recommendedEA << endl;
                }
                else
                {
                    connection& con = getConnection();
                    work trans( con, "DeterminePrimaryKey..." );

                    string tableName = getDirName();

                    stringset_t pk;
                    stringset_t uniqcols;
                    string oid = DeterminePrimaryKey_forBaseTableName(
                        trans, tableName, pk, uniqcols );

                    LG_PG_D << "After initial attempt to find pk..." << endl;
                    LG_PG_D << "pk:" << Util::createSeperatedList( pk ) << endl;
                    LG_PG_D << "uniqcols:" << Util::createSeperatedList( uniqcols ) << endl;
                                    
                    
//                     string oid;
//                     {
//                         stringstream sqlss;
//                         sqlss << " SELECT c.oid,"
//                               << "   n.nspname, "
//                               << "   c.relname "
//                               << " FROM pg_catalog.pg_class c "
//                               << "      LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace "
//                               << "where pg_catalog.pg_table_is_visible(c.oid) "
//                               << "and c.relname = '" << tableName << "' "
//                               << "ORDER BY 2, 3; ";

//                         result res = trans.exec( sqlss.str() );
//                         res[0][0].to(oid);
//                     }
                
//                     stringstream sqlss;
//                     sqlss << "SELECT c2.relname, i.indisprimary, i.indisunique, i.indisclustered, "
//                           << " pg_catalog.pg_get_indexdef(i.indexrelid, 0, true)"
//                           << " FROM pg_catalog.pg_class c, pg_catalog.pg_class c2, pg_catalog.pg_index i"
//                           << " WHERE c.oid = '" << oid << "' AND c.oid = i.indrelid AND i.indexrelid = c2.oid"
//                           << " ORDER BY i.indisprimary DESC, i.indisunique DESC, c2.relname";
//                     result res = trans.exec( sqlss.str() );

//                     LG_PG_D << "determine PK rdn:" << getDirName() << endl;

//                     stringset_t pk;
//                     stringset_t uniqcols;
//                     for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
//                     {
//                         string columns = "";
//                         string indisprimary = "f";
//                         string indisunique = "f";
//                         iter["indisprimary"].to( indisprimary );
//                         iter["indisunique"].to( indisunique );
//                         iter["pg_get_indexdef"].to( columns );

//                         int openbr = columns.find('(');
//                         columns = columns.substr( openbr+1, columns.rfind(')') - openbr-1 );

//                         if( indisprimary == "t" )
//                         {
//                             LG_PG_D << "PK indisprimary:" << indisprimary << " columns:" << columns << endl;

//                             stringlist_t sl = Util::parseCommaSeperatedList( columns );
//                             copy( sl.begin(), sl.end(), inserter( pk, pk.end() ));
//                         }
//                         if( indisunique == "t" )
//                         {
//                             stringlist_t sl = Util::parseCommaSeperatedList( columns );
//                             copy( sl.begin(), sl.end(), inserter( uniqcols, uniqcols.end() ));
//                         }
//                     }

                    //
                    // If is a view then make the pk = union(basetable.pk)
                    //
                    if( pk.empty() )
                    {
                        LG_PG_D << "DeterminePrimaryKey() trying to determine view key" << endl;
                        
                        string viewDef;
                        stringstream sqlss;
                        sqlss << "select definition from pg_catalog.pg_views where "
                              << " viewname='" << tableName << "' ";
                        result res = trans.exec( sqlss.str() );
                        if( !res.empty() )
                        {
                            res[0][0].to(viewDef);
                            if( !viewDef.empty() )
                            {
                                stringset_t tcols;
                                stringlist_t btabs = DeterminePrimaryKey_getBaseTableNamesFromViewDef( viewDef );
                                for( rs< stringlist_t > bi( btabs ); bi; ++bi )
                                {
                                    string tableName = *bi;
                                    DeterminePrimaryKey_forBaseTableName(
                                        trans, tableName, pk, tcols );
                                }

//                                 if( LG_PG_ACTIVE )
//                                 {
//                                     LG_PG_D << "pk.sz:" << pk.size() << endl;
//                                     LG_PG_D << "uniqcols.sz:" << uniqcols.size() << endl;

//                                     LG_PG_D << "pk:" << Util::createSeperatedList( pk ) << endl;
//                                     LG_PG_D << "uniqcols:" << Util::createSeperatedList( uniqcols ) << endl;
                                    
//                                 }
                                
//                                 //
//                                 // Only if all the pk's for the basetables are also
//                                 // columns in the view
//                                 //
//                                 if( !includes( uniqcols.begin(),uniqcols.end(),
//                                                pk.begin(), pk.end() ) )
//                                 {
//                                     LG_PG_D << "DeterminePrimaryKey() uniqcols does not include all of pk..."
//                                             << " clearing pk to be whole table again..." << endl;
//                                     pk.clear();
//                                 }
                            }
                        }
                    }
                    

                    if( pk.empty() )
                    {
                        //
                        // Guess at key, make all the keys that don't allow nulls the key, or
                        // if everything allows nulls, then every column is part of the key.
                        //

                        pk = uniqcols;
                    }

                    if( pk.empty() )
                    {
                        //
                        // every column is part of the primary key
                        //
                        stringstream sqlss;
                        sqlss << "SELECT a.attname as an,"
                              << "  pg_catalog.format_type(a.atttypid, a.atttypmod), "
                              << "  (SELECT substring(d.adsrc for 128) FROM pg_catalog.pg_attrdef d "
                              << "   WHERE d.adrelid = a.attrelid AND d.adnum = a.attnum AND a.atthasdef), "
                              << "    a.attnotnull, a.attnum "
                              << "FROM pg_catalog.pg_attribute a "
                              << "WHERE a.attrelid = '" << oid << "' AND a.attnum > 0 AND NOT a.attisdropped "
                              << "ORDER BY a.attnum; ";
                        result res = trans.exec( sqlss.str() );
                        for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
                        {
                            string an = "";
                            iter["an"].to( an );
                            pk.insert( an );
                        }
                    }

                    copy( pk.begin(), pk.end(), back_inserter(m_primaryKeyNames));

                    /*
                     * Default to showing the user a 100% projection of the table.
                     */
                    {
                        v = true;
                        stringstream sqlss;
                        sqlss << "SELECT a.attname as an,"
                              << "  pg_catalog.format_type(a.atttypid, a.atttypmod), "
                              << "  (SELECT substring(d.adsrc for 128) FROM pg_catalog.pg_attrdef d "
                              << "   WHERE d.adrelid = a.attrelid AND d.adnum = a.attnum AND a.atthasdef), "
                              << "    a.attnotnull, a.attnum "
                              << "FROM pg_catalog.pg_attribute a "
                              << "WHERE a.attrelid = '" << oid << "' AND a.attnum > 0 AND NOT a.attisdropped "
                              << "ORDER BY a.attnum; ";
                        result res = trans.exec( sqlss.str() );
                        for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
                        {
                            string an = "";
                            iter["an"].to( an );
                            if( !v )  m_recommendedEA += ",";
                            v = false;

                            m_recommendedEA += an;
                        }
                        m_recommendedEA += ",name,primary-key";
                    }
                }

                

                //
                // Make the composite key
                //
                m_primaryKey = "";
                v = true;
                for( PrimaryKeyNames_t::iterator iter = m_primaryKeyNames.begin();
                     iter != m_primaryKeyNames.end(); ++iter )
                {
                    if( !v )  m_primaryKey += "-";
                    v = false;
                    m_primaryKey += *iter;
                }

                LG_PG_D << "PostgreSQLContext::DeterminePrimaryKey() path:" << getDirPath()
                             << " pk:" << m_primaryKey
                             << endl;
            }

        virtual void executeInsert( const std::string& tableName,
                                    const std::string& sqlString )
            {
                LG_PG_D << "SQL-INSERT:" << sqlString;
//                 Query query = getConnection( this )->query();
//                 (std::ostream&)query << sqlString;
//                 query.execute();

                try
                {
                    connection& con = getConnection();
                    work trans( con, "executeInsert..." );
                    result res = trans.exec( sqlString );
                    trans.commit();
                    LG_PG_D << "DONE." << endl;
                }
                catch( exception& e )
                {
                    LG_PG_D << "ERROR e:" << e.what() << endl;
                    throw;
                }
            }
        
        virtual fh_context addContextForResultingRow( const std::string& tableName,
                                                      const std::string& sqlString )
            {
                connection& con = getConnection();
                work trans( con, "addContextForResultingRow..." );
                result res = trans.exec( sqlString );

                for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
                {
                    string rdn = getPrimaryKeyRdn( m_primaryKeyNames, iter );

                    
                    if( priv_isSubContextBound( rdn ) )
                    {
                        fh_context ret = getSubContext( rdn );
//                        ret->setHasBeenDeleted( false );
                        return ret;
                    }
                    else
                    {
                        PostgreSQLTupleContext* child = new PostgreSQLTupleContext( this, rdn, iter );
                        fh_context ret = child;
//                        child->setIter( iter );
                        addNewChild( child );

                        return ret;
                    }
                }
                return 0;
            }

        virtual bool supportsRemove()
            {
                return true;
            }

        virtual void priv_remove( fh_context c_ctx )
            {
                PostgreSQLTupleContext* c = dynamic_cast<PostgreSQLTupleContext*>( (GetImpl(c_ctx) ) );
                if( !c )
                {
                    fh_stringstream ss;
                    ss << "Attempt to remove a non postgresql tuple context! url:" << c_ctx->getURL();
                    Throw_CanNotDelete( tostr(ss), GetImpl(c_ctx) );
                }
                if( !m_tableQuerySQL.empty() )
                {
                    LG_PG_D << "m_tableQuerySQL is not empty..." << endl;
                    stringstream ss;
                    ss << " can not delete from a filesystem made from an SQL query." << endl;
                    Throw_CanNotDelete( tostr(ss), GetImpl(c_ctx) );
                }
                string url = c->getURL();
                LG_PG_D << "remove() url:" << url << endl;

                string tableName = getDirName();
                stringstream ss;
                ss << "delete from " << tableName << " where ";
                {
                    bool v = true;
                    for( PrimaryKeyNames_t::iterator iter = m_primaryKeyNames.begin();
                         iter != m_primaryKeyNames.end(); ++iter )
                    {
                        if( v ) v = false;
                        else    ss << " AND ";
                        
                        string k = *iter;
                        string v = getStrAttr( c_ctx, k, "", true, true );
                        ss << " " << k << " = " << v << endl;
                    }
                }
                ss << ";" << endl;

                LG_PG_D << "REMOVAL SQL:" << ss.str() << endl;

                connection& con = getConnection();
                work trans( con, "execute remove..." );
                trans.exec( ss.str() );
                trans.commit();
                
//                 fh_database db = c->getDB();
//                 db->erase( c->dbKey() );

//                 stringlist_t ealist = c->getEAKeys();
//                 for( stringlist_t::iterator iter = ealist.begin(); iter != ealist.end(); ++iter )
//                 {
//                     LG_PG_D << "Removing EA belonging to url:" << url << " k:" << *iter << endl;
//                     db->erase( *iter );
//                 }
//                 db->sync();
            }
        
    };
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/


    class FERRISEXP_CTXPLUGIN PostgreSQLDBContext
        :
        public CommonSQLDBContext< PostgreSQLDBContext >
    {
        typedef PostgreSQLDBContext                         _Self;
        typedef CommonSQLDBContext< PostgreSQLDBContext >   _Base;

        connection* m_connection;
//         typedef stringset_t m_functionNames_t;
//         m_functionNames_t m_functionNames;
        
        virtual bool priv_supportsShortCutLoading()
            { return true; }

        virtual fh_context priv_getSubContext( const std::string& rdn )
            throw( NoSuchSubContext )
            {
                try
                {
                    LG_PG_D << "priv_getSubContext() p:" << this->getDirPath()
                            << " rdn:" << rdn
                            << endl;
                    Context::Items_t::iterator isSubContextBoundCache;
                    if( this->priv_isSubContextBound( rdn, isSubContextBoundCache ) )
                    {
                        LG_PG_D << "priv_getSubContext(bound already) p:" << this->getDirPath()
                                      << " rdn:" << rdn
                                      << endl;
//                return _Base::priv_getSubContext( rdn );
                        return *isSubContextBoundCache;
                    }

                    if( rdn.empty() )
                    {
                        fh_stringstream ss;
                        ss << "NoSuchSubContext no rdn given";
                        Throw_NoSuchSubContext( tostr(ss), this );
                    }
                    else if( rdn[0] == '/' )
                    {
                        fh_stringstream ss;
                        ss << "NoSuchSubContext no files start with unescaped '/' as filename";
                        Throw_NoSuchSubContext( tostr(ss), this );
                    }


                    //
                    // its a pg function!
                    //
                    if( contains( rdn, "(" ) )
                    {
                        connection& con = getConnection();
                        stringstream sqlss;
                        sqlss << "SELECT * from " << rdn << ";";

                        LG_PG_D << "sql:" << sqlss.str() << endl;
//                        work trans( con, "getting list of tables..." );
//                        result res = trans.exec( sqlss.str() );

                        LG_PG_D << "making custom function context...1" << endl;
                        PostgreSQLContext* cc = new PostgreSQLContext( this, rdn, tostr(sqlss) );
                        LG_PG_D << "making custom function context...2" << endl;
                        Insert( cc );
                        LG_PG_D << "making custom function context...3" << endl;
//                        m_functionNames.insert( rdn );
                        LG_PG_D << "returning custom function context..." << endl;
                        return cc;
//                         PostgreSQLContext* cc = 0;
//                         cc = priv_ensureSubContext( rdn, cc );
                        
                    }
                    

//                    string k = dbKey( this, rdn );
//                    read();

                    return _Base::priv_getSubContext( rdn );
                }
                catch( NoSuchSubContext& e )
                {
                    throw e;
                }
                catch( exception& e )
                {
                    string s = e.what();
//            cerr << "NativeContext::priv_getSubContext() e:" << e.what() << endl;
                    Throw_NoSuchSubContext( s, this );
                }
                catch(...)
                {}
                fh_stringstream ss;
                ss << "NoSuchSubContext:" << rdn;
                Throw_NoSuchSubContext( tostr(ss), this );
            }
                    
        
    public:

        PostgreSQLDBContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_connection( 0 )
            {
            }

        virtual ~PostgreSQLDBContext()
            {
                LG_PG_D << "~PostgreSQLDBContext() rdn:" << getDirName() << endl;
                delete m_connection;
            }
        
        virtual bool supportsReClaim()
            {
                return true;
            }

        connection& getConnection();
        
        
    protected:

        virtual void addAllTables() 
            {
                LG_PG_D << "addAllTables(top)" << endl;
                try
                {
                    connection& con = getConnection();

                    stringstream sqlss;
                    sqlss << "SELECT n.nspname as Schema, "
                          << "       c.relname as Name, "
                          << "       CASE c.relkind WHEN 'r' THEN 'table' WHEN 'v' THEN 'view' WHEN 'i' THEN 'index' WHEN 'S' THEN 'seq' WHEN 's' THEN 'special' END as Type, "
                          << "       u.usename as Owner "
                          << "FROM pg_catalog.pg_class c "
                          << "LEFT JOIN pg_catalog.pg_user u ON u.usesysid = c.relowner "
                          << "LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace "
                          << "WHERE c.relkind IN ('r','v') "
                          << "AND n.nspname NOT IN ('pg_catalog', 'pg_toast') "
                          << "AND pg_catalog.pg_table_is_visible(c.oid) "
                          << "ORDER BY 1,2; ";

                    LG_PG_D << "sql:" << sqlss.str() << endl;
//                    cerr << "Getting list of tables transaction........." << endl;
                    work trans( con, "getting list of tables..." );
                    result res = trans.exec( sqlss.str() );

                    stringlist_t sl;

                    for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
                    {
                        int id = 0;
                        string rdn = "";
                        iter["Name"].to( rdn );
                        sl.push_back( rdn );
                    }
                    trans.commit();
                    
                    for( stringlist_t::const_iterator si = sl.begin(); si != sl.end(); ++si )
                    {
                        string rdn = *si;
                        LG_PG_D << "name:" << rdn << endl;

                        PostgreSQLContext* cc = 0;
                        cc = priv_ensureSubContext( rdn, cc );
                        
//                         if( !priv_isSubContextBound( rdn ) )
//                         {
//                             PostgreSQLContext* tc = new PostgreSQLContext( this, rdn );
//                             addNewChild( tc );
//                         }
                    }
                }
                catch( exception& e )
                {
                    LG_PG_W << "addAllTables() e:" << e.what() << endl;
                    throw;
                }

//                 LG_PG_D << "m_functionNames.size:" << m_functionNames.size() << endl;
//                 m_functionNames_t::iterator iter = m_functionNames.begin();
//                 m_functionNames_t::iterator    e = m_functionNames.end();
//                 for( ; iter != e ; ++iter )
//                 {
//                     string rdn = *iter;
                    
//                     PostgreSQLContext* cc = 0;
//                     cc = priv_ensureSubContext( rdn, cc );
//                 }
            }

        virtual Context* createSubContext( const fh_context& parent,
                                           const string& rdn,
                                           std::string QuSQL = "" )
            {
                return new PostgreSQLContext( parent, rdn, QuSQL );
            }
        
        virtual void createSQLTable( const std::string& tableName,
                                     const std::string& sqlString )
            {
                connection& con = getConnection();
                work trans( con, "create table..." );
                trans.exec( sqlString );
                trans.commit();
            }

        void
        priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
         {
                LG_PG_D << "priv_FillCreateSubContextSchemaParts(B)" << endl;
                m["table"] = SubContextCreator(
                    SubContextCreator::Perform_t( this, &_Self::SubCreateTable),
/**/                "	<elementType name=\"table\">\n"
/**/                "		<elementType name=\"name\" default=\"newtable\">\n"
/**/                "			<dataTypeRef name=\"string\"/>\n"
/**/                "		</elementType>\n"
/**/                "		<elementType name=\"sql\" >\n"
/**/                "			<dataTypeRef name=\"string\"/>\n"
/**/                "		</elementType>\n"
/**/                "	</elementType>\n");

                m["queryview"] = SubContextCreator(
                        SubContextCreator::Perform_t( this, &_Self::SubCreateQV),
/**/                "	<elementType name=\"queryview\">\n"
/**/                "		<elementType name=\"name\" default=\"newqtable\">\n"
/**/                "			<dataTypeRef name=\"string\"/>\n"
/**/                "		</elementType>\n"
/**/                "		<elementType name=\"sql\" >\n"
/**/                "			<dataTypeRef name=\"string\"/>\n"
/**/                "		</elementType>\n"
/**/                "	</elementType>\n");

            }

    };

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_CTXPLUGIN PostgreSQLServerContext
        :
        public CommonSQLDBServerContext< PostgreSQLServerContext >
    {
        typedef CommonSQLDBServerContext< PostgreSQLServerContext > _Base;

        connection* m_connection;
        
    public:
        
        PostgreSQLServerContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_connection( 0 )
            {
            }

        virtual ~PostgreSQLServerContext()
            {
                LG_PG_D << "~PostgreSQLServerContext() rdn:" << getDirName() << endl;
                delete m_connection;
            }
        
        virtual bool supportsReClaim()
            {
                return true;
            }

        connection& getConnection()
            {
                if( m_connection )
                    return (*m_connection);
                
                string rdn = getDirName();
                userpass_t up = getPostgreSQLUserPass( rdn );
                
                fh_stringstream ss;
                ss << "host=" << rdn      << " ";
                if( !up.first.empty() )
                    ss << "user=" << up.first << " ";
                if( !up.second.empty() )
                    ss << "password=" << up.second << " ";
                ss << " dbname=template1 ";
                ss << endl;
                LG_PG_D << "TryToCheckServerExists data:" << tostr(ss) << endl;

                try
                {
                    m_connection = new connection( ss.str() );
                }
                catch( exception& e )
                {
                    LG_PG_D << "PostgreSQLServerContext::getConnection(err):" << e.what() << endl;
                    throw;
                }
                
                return (*m_connection);
            }
        
    protected:

//         PostgreSQLDBContext*
//         priv_CreateContext( Context* parent, string rdn )
//             {
//                 PostgreSQLDBContext* ret = new PostgreSQLDBContext( parent, rdn );
//                 return ret;
//             }
        
        virtual void addAllDatabases()
            {
                LG_PG_D << "addAllDatabases() path:" << getDirPath() << endl;

                try
                {
                    
                connection& con = getConnection();
                stringstream sqlss;
                sqlss << "SELECT d.datname as Name, u.usename as Owner, "
                      << "     pg_catalog.pg_encoding_to_char(d.encoding) as Encoding "
                      << "FROM pg_catalog.pg_database d  "
                      << "LEFT JOIN pg_catalog.pg_user u ON d.datdba = u.usesysid order by 1;";
                LG_PG_D << "sql:" << sqlss.str() << endl;
                LG_PG_D << "addAllDatabases(getting data) path:" << getDirPath() << endl;
                work trans( con, "getting list of databases..." );
                result res = trans.exec( sqlss.str() );
                LG_PG_D << "addAllDatabases(have res) path:" << getDirPath() << endl;

                for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
                {
                    int id = 0;
                    string rdn = "";
                    iter["Name"].to( rdn );
//                    LG_PG_D << "iter name:" << rdn << endl;

                    PostgreSQLDBContext* cc = 0;
                    cc = priv_ensureSubContext( rdn, cc );
                    
//                    fh_context cc = priv_readSubContext( rdn, false );
                    
//                     if( !priv_isSubContextBound( rdn ) )
//                     {
//                         PostgreSQLDBContext* dbc = new PostgreSQLDBContext( this, rdn );
//                         addNewChild( dbc );
//                     }
                }

                LG_PG_D << "addAllDatabases(ret) path:" << getDirPath()
                        << " subcontexts:" << res.size()
                        << endl;
                }
                catch( exception& e )
                {
                    LG_PG_D << "addAllDatabases(ret) path:" << getDirPath()
                            << " e:" << e.what()
                            << endl;
                    throw;
                }
            }
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_CTXPLUGIN PostgreSQLRootContext
        :
        public CommonSQLDBRootContext< PostgreSQLServerContext >
    {
        typedef CommonSQLDBRootContext< PostgreSQLServerContext > _Base;
        
    public:
        
        PostgreSQLRootContext( Context* parent,
                               const std::string& rdn,
                               bool bindall = false )
            :
            _Base( parent, rdn, bindall )
            {
            }
        
    protected:

        virtual std::string TryToCheckServerExists( const std::string& rdn )
            {
                userpass_t up = getPostgreSQLUserPass( rdn );
                
                fh_stringstream ss;
                try
                {
                    ss << "host=" << rdn      << " ";
                    if( !up.first.empty() )
                        ss << "user=" << up.first << " ";
                    if( !up.second.empty() )
                        ss << "password=" << up.second << " ";
                    ss << endl;
                    LG_PG_D << "TryToCheckServerExists data:" << tostr(ss) << endl;

                    connection c( ss.str() );
                }
                catch( exception& e )
                {
                    fh_stringstream ss;
                    ss << "Error to server:" << rdn
                       << " user:" << up.first.c_str()
                       << " e:" << e.what()
                       << endl;
                    LG_PG_D << "connect error:" << tostr(ss) << endl;
                    return tostr(ss);
                }
                return _Base::TryToCheckServerExists( rdn );
            }
    };


    connection& PostgreSQLContext::getConnection()
    {
        return getFirstParentOfContextClass<>( (PostgreSQLDBContext*)0)->getConnection();
    }

    connection& PostgreSQLTupleContext::getConnection()
    {
        return getFirstParentOfContextClass<>( (PostgreSQLDBContext*)0)->getConnection();
    }

    XSDBasic_t
    PostgreSQLTupleContext::getSchemaType( const std::string& eaname )
    {
        if( PostgreSQLContext* p = dynamic_cast<PostgreSQLContext*>( getParent() ) )
            return p->getSchemaType( eaname );
        return XSD_BASIC_STRING;
    }
    
    
//     connection& PostgreSQLDBContext::getConnection()
//     {
//         return getFirstParentOfContextClass<>( (PostgreSQLServerContext*)0)->getConnection();
//     }

    connection& PostgreSQLDBContext::getConnection()
    {
        if( m_connection )
            return (*m_connection);
                
        string rdn = getDirName();
        userpass_t up = getPostgreSQLUserPass( rdn );
                
        fh_stringstream ss;
        ss << "host=" << getParent()->getDirName()      << " ";
        if( !up.first.empty() )
            ss << "user=" << up.first << " ";
        if( !up.second.empty() )
            ss << "password=" << up.second << " ";
        ss << " dbname=" << getDirName();
        ss << endl;
        LG_PG_D << "TryToCheckServerExists data:" << tostr(ss) << endl;

        try
        {
            m_connection = new connection( ss.str() );
        }
        catch( exception& e )
        {
            LG_PG_D << "PostgreSQLDBContext::getConnection(err) e:" << e.what() << endl;
            throw;
        }
        return (*m_connection);
    }

    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/


    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
            
                static PostgreSQLRootContext* c = 0;
                const string& root = rf->getInfo( RootContextFactory::ROOT );

                if( !c )
                {
                    LG_PG_D << "PostgreSQL making root context " << endl;
                    c = new PostgreSQLRootContext(0, "/", false );

                    LG_PG_D << "PostgreSQL adding localhosts to root context " << endl;
                    c->tryAugmentLocalhostNames();
                
                    // Bump ref count.
                    static fh_context keeper = c;
                    static fh_context keeper2 = keeper;
                }

                fh_context ret = c;

                if( root != "/" )
                {
                    string path = rf->getInfo( RootContextFactory::PATH );
                    string newpath = root;

                    newpath += "/";
                    newpath += path;

                    rf->AddInfo( RootContextFactory::PATH, newpath );
                }
                return ret;
            }
            catch( FerrisSqlServerNameNotFound& e )
            {
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
            catch( exception& e )
            {
                LG_PG_W << "e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }

};
