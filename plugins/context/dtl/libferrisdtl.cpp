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

    $Id: libferrisdtl.cpp,v 1.4 2010/09/24 21:31:34 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <DTL.h>

#include <FerrisContextPlugin.hh>
#include <Trimming.hh>
#include <FerrisDOM.hh>
#include <FerrisCreationPlugin.hh>

#include <libcommonsqldbapi.hh>
#include <libferrisdtlshared.hh>

using namespace std;
using namespace dtl;

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    /*
     * /odbc/localhost/mysqldb/table
     *  |         |        |
     *  x        dsn      dbname
     */
    string getDSN( fh_context c )
    {
        typedef list< fh_context > flattree_t;
        flattree_t ftree;

        fh_context pc = c;
        while( pc->isParentBound() )
        {
            ftree.push_front( pc );
            pc = pc->getParent();
        }

        flattree_t::iterator iter = ftree.begin();
        
        if( iter != ftree.end() )
        {
            string dsn = (*iter)->getDirName();
            return dsn;
        }
        else
        {
            fh_stringstream ss;
            ss << "Can not determine the DSN name for context:"
               << c->getDirPath();
            Throw_FerrisSqlServerNameNotFound( tostr(ss), 0 );
        }
    }

    string getDatabase( fh_context c )
    {
        typedef list< fh_context > flattree_t;
        flattree_t ftree;

        fh_context pc = c;
        while( pc->isParentBound() )
        {
            ftree.push_front( pc );
            pc = pc->getParent();
        }

        flattree_t::iterator iter = ftree.begin();
        
        if( iter != ftree.end() )
        {
            ++iter;
            if( iter != ftree.end() )
            {
                return (*iter)->getDirName();
            }
        }
        return "";
    }

    void
    switchToDB( DBConnection& con, const std::string& dbname )
    {
        if( !dbname.empty() )
        {
            fh_stringstream ss;
            ss << "use " << dbname;
//            ss << "\\c " << dbname;
            LG_DTL_D << " switching databases... " << tostr(ss) << endl;
            DBStmt x( tostr(ss), con );
            x.Execute();
        }
    }
    
    DBConnection& getConnection( fh_context c )
    {
        typedef Loki::SmartPtr<
            DBConnection,
            Loki::RefCounted,
            Loki::DisallowConversion,
            Loki::AssertCheck,
            Loki::DefaultSPStorage > fh_connection;
        typedef map< string, fh_connection > connections_t;
        static connections_t connections;
        
        string     DSN      = getDSN( c );
        userpass_t up       = getODBCUserPass( DSN );
        string dbname = getDatabase( c );

        connections_t::iterator iter = connections.find( DSN );
        if( iter != connections.end() )
        {
            DBConnection& con = *GetImpl(iter->second);
            switchToDB( con, dbname );
            return con;
        }
        
        fh_stringstream args;
        args << "UID=" << up.first  << ";"
             << "PWD=" << up.second << ";"
             << "DSN=" << DSN       << ";"
             << endl;
        DBConnection* ret = new DBConnection();
        ret->Connect( tostr( args ));
        switchToDB( *ret, dbname );

        connections[ DSN ] = ret;
        return *ret;
        
//         DBConnection& ret =  DBConnection::GetDefaultConnection();
//         ret.Connect( tostr( args ));

//         if( !dbname.empty() )
//         {
//             fh_stringstream ss;
//             ss << "use " << dbname;
//             LG_DTL_D << " switching databases... " << tostr(ss) << endl;
//             DBStmt x( tostr(ss), ret );
//             x.Execute();
//         }
        
//         return ret;
    }

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    typedef vector< string > PrimaryKeyNames_t;
    class DTLContext;

    class FERRISEXP_CTXPLUGIN DTLTupleContext
        :
        public CommonSQLDBTupleContext< DTLTupleContext, DTLContext >
    {
        typedef DTLTupleContext                            _Self;
        typedef CommonSQLDBTupleContext< DTLTupleContext, DTLContext > _Base;

    public:

        void setIter( variant_row& row )
            {
                vector<string> colNames = row.GetNames();
                for (vector<string>::iterator it = colNames.begin(); it != colNames.end(); ++it )
                {
                    string k = *it;
                    if( !k.length() )
                        continue;

                    LG_DTL_D << "DTLTupleContext() k:" << k << endl;
                    string v = row[ k ];
                    LG_DTL_D << "DTLTupleContext() v:" << v << endl;

                    kvm[k]=v;
                    if( !isAttributeBound( k ) )
                        addAttribute( k,
                                      this, &_Self::getValueStream,
                                      this, &_Self::getValueStream,
                                      this, &_Self::setValueStream,
                                      XSD_BASIC_STRING );

                    string lowerk = tolowerstr()( k );
                    kvm[ lowerk ]=v;
                    if( !isAttributeBound( lowerk ) )
                        addAttribute( lowerk,
                                      this, &_Self::getValueStream,
                                      this, &_Self::getValueStream,
                                      this, &_Self::setValueStream,
                                      XSD_BASIC_STRING );
                }
            }
        
        DTLTupleContext( const fh_context& parent, const string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }
        
        
    protected:

        virtual void executeUpdate( const std::string& sqlString )
            {
//                 Query query = getConnection( this )->query();
//                 (std::ostream&)query << sqlString; 
//                 query.execute();

                DBConnection& con = getConnection( this );
                DynamicDBView<> view( "", sqlString );
                view.GetDataObj();
            }

        virtual std::string getTableName()
            {
                return getParent()->getDirName();
            }
        
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_CTXPLUGIN DTLContext
        :
        public CommonSQLContext< DTLContext >
    {
        typedef DTLContext _Self;
        typedef CommonSQLContext< DTLContext > _Base;


        string setupQuery()
            {
                fh_stringstream ss;
        
                if( m_tableQuerySQL.length() )
                {
                    LG_DTL_D << "m_tableQuerySQL:" << m_tableQuerySQL << endl;
                    ss << m_tableQuerySQL;
                }
                else
                {
                    ss << "select * from " << getDirName();
                }

                LG_DTL_D << "setupQuery() SQL:" << tostr(ss) << endl;
                return tostr( ss );
            }
        
    public:

        DTLContext( const fh_context& parent, const string& rdn, string QuSQL = "" )
            :
            _Base( parent, rdn, QuSQL )
            {
                DeterminePrimaryKey();
                createStateLessAttributes();
            }
        
        virtual ~DTLContext()
            {
            }
        
    protected:

//         void
//         priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
//             {
//                 try
//                 {
//                     PrimaryKeyNames_t& keyNames = getKeyNames();
                    
//                     LG_DTL_D << "priv_FillCreateSubContextSchemaParts(A)" << endl;
                
//                     static Util::SingleShot v;
//                     if( v() )
//                     {
//                         static bool rst = appendExtraGenerateSchemaSimpleTypes(
                        
//                             "<simpleType name=\"SQLStringT\">"  
//                             "    <restriction base=\"string\">"  
//                             "    </restriction>"  
//                             "</simpleType>"  
//                             "<simpleType name=\"SQLDateT\">"  
//                             "    <restriction base=\"string\">"  
//                             "    </restriction>"  
//                             "</simpleType>"  
//                             "<simpleType name=\"SQLTimeStampT\">"  
//                             "    <restriction base=\"string\">"  
//                             "    </restriction>"  
//                             "</simpleType>"  
//                             "<simpleType name=\"SQLDoubleT\">"  
//                             "    <restriction base=\"string\">"  
//                             "    </restriction>"  
//                             "</simpleType>"
//                             );
//                     }
                
//                     LG_DTL_D << "priv_FillCreateSubContextSchemaParts(A1)" << endl;
//                     fh_stringstream ss;

//                     ss << "	<elementType name=\"tuple\">\n";
//                     LG_DTL_D << "priv_FillCreateSubContextSchemaParts(A2)" << endl;
//                     for( PrimaryKeyNames_t::iterator iter = keyNames.begin();
//                          iter != keyNames.end(); ++iter )
//                     {
//                         LG_DTL_D << "priv_FillCreateSubContextSchemaParts(ALoop0)" << endl;
//                         int maxlen = 0;
//                         std::string xmltype = "string";
//                         std::string ktype = KeyNamesType[*iter].KeyType();
//                         std::string defaultVal = "";

//                         LG_DTL_D << "priv_FillCreateSubContextSchemaParts(ALoop1)"
//                              << " ktype:" << ktype
//                              << endl;
                    
//                         if( ktype == "date" )
//                         {
//                             xmltype = "SQLDateT";
//                         }
//                         else if( starts_with( ktype, "timestamp") )
//                         {
//                             xmltype = "SQLTimeStampT";
//                             defaultVal = "NOW()";
//                         }
//                         else if( ktype == "double" )
//                         {
//                             xmltype = "SQLDoubleT";
//                         }
                    
//                         if( std::string::npos != ktype.find("("))
//                         {
//                             fh_stringstream mss;
//                             mss << ktype.substr( ktype.find("(")+1 );
//                             mss >> maxlen;
//                         }

//                         bool isPrimary = false;
//                         if( m_primaryKeyNames.end() != std::find(
//                                 m_primaryKeyNames.begin(), m_primaryKeyNames.end(),
//                                 *iter ))
//                         {
//                             isPrimary = true;
//                         }
//                         bool minOccur = isPrimary;
//                         if( KeyNamesType[*iter].AutoIncrement() )
//                             minOccur = 0;
//                         if( KeyNamesType[*iter].AllowNulls() )
//                             minOccur = 0;
                        
//                         LG_DTL_D << "iter:" << *iter << endl;
//                         LG_DTL_D << "ktype:" << ktype << endl;
//                         LG_DTL_D << "maxlen:" << maxlen << endl;
//                         LG_DTL_D << "isPrimary:" << isPrimary << endl;

//                         ss << "<elementType name=\"" << *iter << "\" ";
//                         ss << " minOccur=\"" << minOccur << "\" ";
//                         ss << " maxOccur=\"" << 1 << "\" ";
//                         if( maxlen )
//                             ss << " maxLength=\"" << maxlen << "\" ";
//                         if( defaultVal.length() )
//                         {
//                             ss << " default=\"" << defaultVal << "\" ";
//                         }
//                         ss   << " >\n"
//                              << "<dataTypeRef name=\"" << xmltype << "\"/>\n"
//                              << "</elementType>\n";
                    
//                     }
//                     ss << "	</elementType>\n";
                    
//                     LG_DTL_D << "priv_FillCreateSubContextSchemaParts(A5)" << endl;
                
//                     m["tuple"] = SubContextCreator(
//                         SubContextCreator::Perform_t( this, &_Self::SubCreateTuple),
//                         tostr(ss));
//                     LG_DTL_D << "priv_FillCreateSubContextSchemaParts(A6)" << endl;
//                 }
//                 catch( std::exception& e )
//                 {
//                 }
//             }
        

        virtual fh_iostream  getRealStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   exception)
            {
                bool virgin = true;
                fh_stringstream ret;

                DBConnection& con = getConnection( this );
                DynamicDBView<> view( setupQuery(), "" );
                variant_row s(view.GetDataObj());

                virgin = true;
                vector<string> colNames = s.GetNames();
                for (vector<string>::iterator it = colNames.begin(); it != colNames.end(); ++it )
                {
                    if(!virgin) ret << ",";
                    virgin = false;
                        
                    string k = *it;
                    if( !k.length() )
                        continue;
                        
                    ret << k;
                }
                ret << endl;
                
                // Print out all rows and columns from our query
                DynamicDBView<>::select_iterator print_it = view.begin();
                for (print_it = view.begin(); print_it != view.end(); print_it++)
                {
                    variant_row row = *print_it;
                    
                    virgin = true;
                    vector<string> colNames = row.GetNames();
                    for (vector<string>::iterator it = colNames.begin(); it != colNames.end(); ++it )
                    {
                        if(!virgin) ret << ",";
                        virgin = false;
                        
                        string k = *it;
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
                DBConnection& con = getConnection( this );
                fh_stringstream sqlss;
                sqlss << "describe " << getDirName();
                DynamicDBView<> view( tostr( sqlss ), "" );
                
                    
                for( DynamicDBView<>::sql_iterator print_it = view.begin();
                     print_it != view.end(); print_it++)
                {
                    variant_row row  = *print_it;
                
                    string k = row["Field"];
                    string t = row["Type"];
                    LG_DTL_D << "DTLContext::getKeyNames() k:" << k
                             << " t:" << t
                             << endl;
                    KeyNames.push_back(k);

                    string ex = row["Extra"];
                    bool autoinc = ( string::npos != ex.find("auto_increment") );

                    string allowNullStr = row["Null"];
                    bool allowNull = allowNullStr == "YES";
                    KeyNamesType[k] = RowMetaData( t, autoinc, allowNull );
                }
        
                LG_DTL_D << "DTLContext::getKeyNames(end)" << endl;
            }

        void priv_read()
            {
                LG_DTL_D << "DTLContext::read(1)" << endl;

                EnsureStartStopReadingIsFiredRAII _raii1( this );
                clearContext();
                LG_DTL_D << "priv_read(top) path:" << getDirPath() << endl;

                DBConnection& con = getConnection( this );
                DynamicDBView<> view( setupQuery(), "" );


                LG_DTL_D << "DTLContext::read(2)" << endl;
                variant_row s(view.GetDataObj());
                LG_DTL_D << "DTLContext::read(3)" << endl;
                vector<string> colNames = s.GetNames();
                for (vector<string>::iterator it = colNames.begin(); it != colNames.end(); ++it )
                {
                        
                    string k = *it;
                    if( !k.length() )
                        continue;
                        
                    string v;
                    if( k.length() )
                    {
                        addAttribute( k, v, XSD_BASIC_STRING );
                        
                        string lowerk = tolowerstr()( k );
                        addAttribute( lowerk, v, XSD_BASIC_STRING );
                    }
                }
                LG_DTL_D << "DTLContext::read(4)" << endl;
                LG_DTL_D << "DTLContext::read(4) m_primaryKeyNames.sz:" << m_primaryKeyNames.size() << endl;

                
                /*
                 * Create a file, and some EA for each row.
                 */
                DynamicDBView<>::sql_iterator print_it = view.begin();
                for (print_it = view.begin(); print_it != view.end(); print_it++)
                {
                    variant_row row = *print_it;

                    string rdn = getPrimaryKeyRdn( m_primaryKeyNames, row );

                    DTLTupleContext* cc = 0;
                    cc = priv_ensureSubContext( rdn, cc );
                    cc->setIter( row );
                    
//                     DTLTupleContext* child = new DTLTupleContext( this, rdn, row );
//                     addNewChild( child );
                }

                LG_DTL_D << "priv_read(done) path:" << getDirPath() << endl;
            }

        std::string getPrimaryKeyRdn( PrimaryKeyNames_t& pkn, variant_row& row )
            {
                string rdn = "";
                bool v = true;
                for( PrimaryKeyNames_t::iterator iter = pkn.begin(); iter != pkn.end(); ++iter )
                {
                    if( !v ) rdn += "-";
                    v = false;
                    string v = row[ *iter ];
                    rdn += v;
                }
                return rdn;
            }
        
        virtual void DeterminePrimaryKey()
            {
                LG_DTL_D << "DeterminePrimaryKey(1) path:" << getDirPath() << endl;
                bool v = true;

                LG_DTL_D << "DeterminePrimaryKey(2) path:" << getDirPath() << endl;
                LG_DTL_D << "DeterminePrimaryKey(2) m_tableQuerySQL:" << m_tableQuerySQL << endl;

                /*
                 * For queries we use the entire table as the primary key
                 */
                if( m_tableQuerySQL.length() )
                {
                    LG_DTL_D << "DeterminePrimaryKey(with sql) path:" << getDirPath() << endl;

                    DBConnection& con = getConnection( this );
                    DynamicDBView<> view( setupQuery(), "" );
                
                    if( view.begin() == view.end() )
                    {
                        m_recommendedEA += "";
                    }
                    else
                    {
                        variant_row row = *view.begin();

                        variant_row s(view.GetDataObj());
                        vector<string> colNames = s.GetNames();
                        for (vector<string>::iterator it = colNames.begin(); it != colNames.end(); ++it )
                        {
                            string k = *it;
                            if( !k.length() )
                                continue;
                        
                            LG_DTL_D << "res.names(i) : " << k << endl;
                            m_primaryKeyNames.push_back( k );
                        }

                        /*
                         * Default to showing the user a 100% projection of the table.
                         */
                        bool v = true;
                        for (vector<string>::iterator it = colNames.begin(); it != colNames.end(); ++it )
                        {
                            string k = *it;
                            if( !k.length() )
                                continue;
                
                            if( !v )  m_recommendedEA += ",";
                            v = false;
                            LG_DTL_D << "adding:" << k << endl;
                            m_recommendedEA += k;
                        }
                        m_recommendedEA += ",name,primary-key";
                    }
                }
                else
                {
                    DBConnection& con = getConnection( this );
                    fh_stringstream sqlss;
                    sqlss << "describe " << getDirName();
                    DynamicDBView<> view( tostr( sqlss ), "" );

                    bool havePrimaryKey = false;
                    for( DynamicDBView<>::sql_iterator print_it = view.begin();
                         print_it != view.end(); ++print_it )
                    {
                        variant_row row  = *print_it;
                        string      ks   = row["Key"];
                        if( ks.length() )
                        {
                            string t = row["Field"];

                            if( ks == "PRI" )
                            {
                                if( !havePrimaryKey )
                                    m_primaryKeyNames.clear();
                                havePrimaryKey = true;
                            }

                            if( havePrimaryKey && ks == "PRI"
                                || !havePrimaryKey )
                            {
                                m_primaryKeyNames.push_back( t );
                            }
                        }
                    }
                    
                    if( m_primaryKeyNames.empty() )
                    {
                        //
                        // Guess at key, make all the keys that don't allow nulls the key, or
                        // if everything allows nulls, then every column is part of the key.
                        //
                        for( DynamicDBView<>::sql_iterator print_it = view.begin();
                             print_it != view.end(); print_it++)
                        {
                            variant_row row  = *print_it;
                            string      ns   = row["Null"];
                            if( ns != "YES" )
                            {
                                string t = row["Field"];
                                m_primaryKeyNames.push_back( t );
                            }
                        }
                    }

                    if( m_primaryKeyNames.empty() )
                    {
                        // primary key is each column
                        for( DynamicDBView<>::sql_iterator print_it = view.begin();
                             print_it != view.end(); print_it++)
                        {
                            variant_row row = *print_it;
                            string      t   = row["Field"];
                            m_primaryKeyNames.push_back( t );
                        }
                    }

                    /*
                     * Default to showing the user a 100% projection of the table.
                     */
                    v = true;
                    for( DynamicDBView<>::sql_iterator print_it = view.begin();
                         print_it != view.end(); ++print_it )
                    {
                        variant_row row = *print_it;

                        if( !v )  m_recommendedEA += ",";
                        v = false;

                        string t = row["Field"];
                        m_recommendedEA += t;
                    }
                    m_recommendedEA += ",name,primary-key";
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

                LG_DTL_D << "DTLContext::DeterminePrimaryKey() path:" << getDirPath()
                             << " pk:" << m_primaryKey
                             << endl;
            }

        virtual void executeInsert( const std::string& tableName,
                                    const std::string& sqlString )
            {
                LG_DTL_D << "INSERT:" << sqlString;
//                 Query query = getConnection( this )->query();
//                 (std::ostream&)query << sqlString;
//                 query.execute();

                DBConnection& con = getConnection( this );
                DynamicDBView<> view( tableName, sqlString );
                view.GetDataObj();
            }
        
        virtual fh_context addContextForResultingRow( const std::string& tableName,
                                                      const std::string& sqlString )
            {
                DBConnection& con = getConnection( this );
                DynamicDBView<> view( tableName, sqlString );

                DynamicDBView<>::select_iterator print_it = view.begin();
                variant_row row = *print_it;
                
                string rdn = getPrimaryKeyRdn( m_primaryKeyNames, row );

                DTLTupleContext* cc = 0;
                cc = priv_ensureSubContext( rdn, cc );
                cc->setIter( row );
                return cc;
                
//                 DTLTupleContext* child = new DTLTupleContext( this, rdn, row );
//                 fh_context ret = child;
//                 addNewChild( child );
//                return ret;
            }
        
        
        
    };
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/


    class FERRISEXP_CTXPLUGIN DTLDBContext
        :
        public CommonSQLDBContext< DTLDBContext >
    {
        typedef DTLDBContext                         _Self;
        typedef CommonSQLDBContext< DTLDBContext >   _Base;
        
    public:

        DTLDBContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
        
    protected:

        virtual void addAllTables() 
            {
                try
                {
                    
                    DBConnection& con = getConnection( this );
                    string sql = "show tables";
//                    string sql = "select tablename from pg_tables where schemaname = 'public';";
                    DynamicDBView<> view( sql, "" );
                
                    DynamicDBView<>::sql_iterator print_it = view.begin();
                    for (print_it = view.begin(); print_it != view.end(); ++print_it )
                    {
                        variant_row row      = *print_it;
                        string      tabname  = row[0];

                        DTLContext* cc = 0;
                        cc = priv_ensureSubContext( tabname, cc );
                        LG_DTL_D << "  table:" << tabname << endl;
                        
//                         DTLContext* tc = new DTLContext( this, tabname );
//                         addNewChild( tc );
                    }
                }
                catch( DBException& e )
                {
                    LG_DTL_W << "addAllTables() e:" << e.what() << endl;
                    throw;
                }
                catch( exception& e )
                {
                    LG_DTL_W << "addAllTables() e:" << e.what() << endl;
                    throw;
                }
            }

    virtual Context* createSubContext( const fh_context& parent,
                                       const string& rdn,
                                       std::string QuSQL = "" )
    {
        return new DTLContext( parent, rdn, QuSQL );
    }
    
        virtual void createSQLTable( const std::string& tableName,
                                     const std::string& sqlString )
            {
//                 Query query = getConnection( this )->query();
//                 (std::ostream&)query << sqlString;
//                 query.execute();

                DBConnection& con = getConnection( this );
                DynamicDBView<> view( tableName, sqlString );
                view.GetDataObj();
                
            }

        void
        priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
         {
                LG_DTL_D << "priv_FillCreateSubContextSchemaParts(B)" << endl;
//                addStandardFileSubContextSchema(m);
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

    class FERRISEXP_CTXPLUGIN DTLServerContext
        :
        public CommonSQLDBServerContext< DTLServerContext >
    {
        typedef CommonSQLDBServerContext< DTLServerContext > _Base;

    public:
        
        DTLServerContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
        
    protected:

        virtual void addAllDatabases()
            {
                LG_DTL_D << "addAllDatabases() path:" << getDirPath() << endl;

                DBConnection& con = getConnection( this );
                string sql = "show databases";
//                string sql = "select datname from pg_database;";
                DynamicDBView<> view( sql, "" );
                
                LG_DTL_D << "addAllDatabases( got rdn ) path:" << getDirPath() << endl;

                DynamicDBView<>::sql_iterator print_it = view.begin();
                for (print_it = view.begin(); print_it != view.end(); print_it++)
                {
                    variant_row row = *print_it;
                    string dbname = row[0];
        
                    LG_DTL_D << "dbname:" << dbname << endl;
                    LG_DTL_D << "dbname:" << dbname << endl;
                    DTLDBContext* cc = 0;
                    cc = priv_ensureSubContext( dbname, cc );
                    
//                     DTLDBContext* dbc = new DTLDBContext( this, dbname );
//                     addNewChild( dbc );
                }
            }
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_CTXPLUGIN DTLRootContext
        :
        public CommonSQLDBRootContext< DTLServerContext >
    {
        typedef CommonSQLDBRootContext< DTLServerContext > _Base;
        
    public:
        
        DTLRootContext( Context* parent,
                            const std::string& rdn,
                            bool bindall = false )
            :
            _Base( parent, rdn, bindall )
            {
            }
        
    protected:

        virtual std::string TryToCheckServerExists( const std::string& rdn )
            {
                string dsn = rdn;
                userpass_t up = getODBCUserPass( rdn );
                
                try
                {
                    fh_stringstream ss;
                    
                    ss << "UID=" << up.first     << ";"
                       << "PWD=" << up.second    << ";"
                       << "DSN=" << dsn          << ";"
                       << endl;
                    LG_DTL_D << "TryToCheckServerExists data:" << tostr(ss) << endl;

                    string DSN = tostr(ss);
                    
                    // Connect to the database
                    DBConnection::GetDefaultConnection().Connect( DSN );
                }
                catch( exception& e )
                {
                    fh_stringstream ss;
                    ss << "Error connecting to"
                       << " dsn:" << dsn
                       << " user:" << up.first.c_str()
                       << " pass:" << up.second.c_str()
                       << " e:" << e.what()
                       << endl;
                    LG_DTL_D << "connect error:" << tostr(ss) << endl;
                    return tostr(ss);
                }
                return _Base::TryToCheckServerExists( rdn );
            }
    };
    
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
                static DTLRootContext* c = 0;
                const string& root = rf->getInfo( RootContextFactory::ROOT );

                if( !c )
                {
                    LG_DTL_D << "DTL making root context " << endl;
                    c = new DTLRootContext(0, "/", false );

                    LG_DTL_D << "DTL adding localhosts to root context " << endl;
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
                LG_DTL_W << "e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }

};
