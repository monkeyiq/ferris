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

    $Id: libferrissqlplus.cpp,v 1.6 2010/09/24 21:31:48 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <Trimming.hh>
#include <FerrisDOM.hh>
#include <FerrisCreationPlugin.hh>
#include <mysql++.h>
#include <libferrissqlplusshared.hh>
#include <unistd.h>

#include <libcommonsqldbapi.hh>

using namespace std;
using namespace mysqlpp;

namespace Ferris
{
//     static void testdb( const std::string& s, Connection* c = 0 )
//         {
//             try
//             {
                
//             LG_SQLPLUS_D << "testdb( top ) s:" << s << endl;

//             if( !c )
//             {
//                 Connection* conp = new Connection(use_exceptions);
//                 conp->connect("", "localhost", "", "" );
//                 c = conp;
//             }
            

//             if( c )
//             {
//                 {
//                     Query query = c->query();
//                     (std::ostream&)query << "use fca";
//                     query.execute();
//                 }

                
//                 Query query = c->query();
//                 (std::ostream&)query << "show databases";
//                 Result res = query.store();
//                 Result::iterator i;
//                 Row row;
//                 for (i = res.begin(); i != res.end(); i++)
//                 {
//                     row = *i;
//                     string dbname = (string)row[0];
//                     LG_SQLPLUS_D << "db:" << dbname << endl;
//                 }
//             }
//             LG_SQLPLUS_D << "testdb( bottom ) s:" << s << endl;
//             }
//             catch( exception& e )
//             {
//                 LG_SQLPLUS_D << "testdb() cought e:" << e.what() << endl;
//             }
            
//         }
        
    
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    string getnames( Result& res, int i )
    {
        const Field& f = res.fields( i );
        string k = f.name ? f.name : "";
        return k;
    }
    

    struct SqlDetails
    {
        string serv;
        string db;
        string table;

        SqlDetails()
            {
            }
            
        SqlDetails( const string& _serv, const string& _db = "", const string& _t = "" )
            :
            serv( _serv ),
            db( _db ),
            table( _t )
            {
            }

        void reset()
            {
                serv = "";
                db = "";
                table = "";
            }
        
    };



    static void switchToDB( Connection* c, const string& db )
    {
        if( db.length() )
        {
            LG_SQLPLUS_D << "switchToDB() db:" << db << endl;
        
            Query query = c->query();
            (std::ostream&)query << "use " << db;
            query.execute();

            LG_SQLPLUS_D << "switchToDB(2) db:" << db << endl;
        }
    }

    /*
     * /mysql/localhost/mysqldb/table
     *  |         |        |
     *  x       serv      dbname
     */
    const SqlDetails& getSqlDetails( fh_context c )
    {
        static SqlDetails d;
        typedef list< fh_context > flattree_t;
        flattree_t ftree;

        d.reset();

        fh_context pc = c;
        while( pc->isParentBound() )
        {
            ftree.push_front( pc );
            pc = pc->getParent();
        }

        flattree_t::iterator iter = ftree.begin();
        
        if( iter != ftree.end() )
        {
            d.serv = (*iter)->getDirName();

            if( ++iter != ftree.end() )
            {
                d.db = (*iter)->getDirName();
                if( ++iter != ftree.end() )
                {
                    d.table = (*iter)->getDirName();
                }
            }
        }
        else
        {
            fh_stringstream ss;
            ss << "Can not determine the server name for context:"
               << c->getDirPath();
            Throw_FerrisSqlServerNameNotFound( tostr(ss), 0 );
        }
        
        
        LG_SQLPLUS_D << "getSqlDetails() c:" << c->getDirPath()
                     << " serv:" << d.serv
                     << " db:" << d.db
                     << " table:" << d.table
                     << endl;
        
        return d;
    }
    
    
    Connection* getConnection( fh_context c )
    {
        typedef Loki::SmartPtr<
            Connection,
            Loki::RefCounted,
            Loki::DisallowConversion,
            Loki::AssertCheck,
            Loki::DefaultSPStorage > fh_connection;
        typedef map< string, fh_connection > connections_t;
        static connections_t connections;

        LG_SQLPLUS_D << "getConnection() c:" << c->getDirPath() << endl;
        
        const SqlDetails& sdt    = getSqlDetails( c );
        const string& servername = sdt.serv;
        const string& dbname     = sdt.db;
        LG_SQLPLUS_D << "getConnection(2) c:" << c->getDirPath() << endl;
        
        connections_t::iterator iter = connections.find( servername );
        if( iter != connections.end() )
        {
            LG_SQLPLUS_D << "getConnection() connection cached c:" << c->getDirPath() << endl;
            Connection* con = GetImpl(iter->second);
            switchToDB( con, dbname );
            return con;
        }
  
        string username = "";
        string passwd = "";
        Connection* conp = new Connection(use_exceptions);
        connections[servername] = conp;

        userpass_t up = getUserPass( servername );

        LG_SQLPLUS_D << "making mysql connection. serv:" << servername
             << " user:" << up.first << " pass:" << up.second << endl;
        
        conp->connect("", servername.c_str(),
                      up.first.c_str(), up.second.c_str() );

        LG_SQLPLUS_D << "have connected... :" << conp->connected() << endl;
        
        LG_SQLPLUS_D << "getConnection() new connection created c:" << c->getDirPath() << endl;
        
        switchToDB( conp, dbname );

        LG_SQLPLUS_D << "getConnection(ret) new connection created c:" << c->getDirPath() << endl;

        return conp;
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
    class SqlPlusContext;

    class FERRISEXP_CTXPLUGIN SqlPlusTupleContext
        :
        public CommonSQLDBTupleContext< SqlPlusTupleContext, SqlPlusContext >
    {
        typedef SqlPlusTupleContext                            _Self;
        typedef CommonSQLDBTupleContext< SqlPlusTupleContext, SqlPlusContext > _Base;

    public:

        SqlPlusTupleContext( const fh_context& parent,
                             const string& rdn,
                             Result& res,
                             Row& row )
            :
            _Base( parent, rdn )
            {  
                for (unsigned int i = 0; i < row.size(); i++)
                {
                    string k = getnames( res, i );
                    if( !k.length() )
                        continue;

                    LG_SQLPLUS_D << "SqlPlusTupleContext() k:" << k << endl;
                    string v = (string)row[ k.c_str() ];
//                    string v = (string)row[k];

                    kvm[k]=v;
                    addAttribute( k,
                                  this, &_Self::getValueStream,
                                  this, &_Self::getValueStream,
                                  this, &_Self::setValueStream,
                                  XSD_BASIC_STRING );

                    string lowerk = tolowerstr()( k );
                    kvm[ lowerk ]=v;
                    addAttribute( lowerk,
                                  this, &_Self::getValueStream,
                                  this, &_Self::getValueStream,
                                  this, &_Self::setValueStream,
                                  XSD_BASIC_STRING );
                }
                createStateLessAttributes();
            }
        
        
    protected:

        virtual void executeUpdate( const std::string& sqlString )
            {
                const SqlDetails& d = getSqlDetails( this );
                Query query = getConnection( this )->query();
                (std::ostream&)query << sqlString; 
                query.execute();
            }

        virtual std::string getTableName()
            {
                const SqlDetails& d = getSqlDetails( this );
                return d.table;
            }
        
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_CTXPLUGIN SqlPlusContext
        :
        public CommonSQLContext< SqlPlusContext >
    {
        typedef SqlPlusContext _Self;
        typedef CommonSQLContext< SqlPlusContext > _Base;

        void setupQuery( Query& q )
            {
                fh_stringstream ss;
        
                if( m_tableQuerySQL.length() )
                {
                    LG_SQLPLUS_D << "m_tableQuerySQL:" << m_tableQuerySQL << endl;
                    ss << m_tableQuerySQL;
                }
                else
                {
                    ss << "select * from " << getDirName();
                }
                q << tostr(ss);
                LG_SQLPLUS_D << "SqlPlusContext::setupQuery() ss:" << tostr(ss) << endl;
            }
        
    public:

        SqlPlusContext( const fh_context& parent, const string& rdn, string QuSQL = "" )
            :
            _Base( parent, rdn, QuSQL )
            {
                DeterminePrimaryKey();
                createStateLessAttributes();
            }
        
        virtual ~SqlPlusContext()
            {
            }
        
    protected:

        void
        priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
                try
                {
                    PrimaryKeyNames_t& keyNames = getKeyNames();
                    
//                    cerr << "priv_FillCreateSubContextSchemaParts(A)" << endl;
                
                    static Util::SingleShot v;
                    if( v() )
                    {
                        static bool rst = appendExtraGenerateSchemaSimpleTypes(
                        
                            "<simpleType name=\"SQLStringT\">"  
                            "    <restriction base=\"string\">"  
                            "    </restriction>"  
                            "</simpleType>"  
                            "<simpleType name=\"SQLDateT\">"  
                            "    <restriction base=\"string\">"  
                            "    </restriction>"  
                            "</simpleType>"  
                            "<simpleType name=\"SQLTimeStampT\">"  
                            "    <restriction base=\"string\">"  
                            "    </restriction>"  
                            "</simpleType>"  
                            "<simpleType name=\"SQLDoubleT\">"  
                            "    <restriction base=\"string\">"  
                            "    </restriction>"  
                            "</simpleType>"
                            );
                    }
                
//                    cerr << "priv_FillCreateSubContextSchemaParts(A1)" << endl;
                    fh_stringstream ss;

                    ss << "	<elementType name=\"tuple\">\n";
//                    cerr << "priv_FillCreateSubContextSchemaParts(A2)" << endl;
                    for( PrimaryKeyNames_t::iterator iter = keyNames.begin();
                         iter != keyNames.end(); ++iter )
                    {
//                        cerr << "priv_FillCreateSubContextSchemaParts(ALoop0)" << endl;
                        int maxlen = 0;
                        std::string xmltype = "string";
                        std::string ktype = KeyNamesType[*iter].KeyType();
                        std::string defaultVal = "";

//                         cerr << "priv_FillCreateSubContextSchemaParts(ALoop1)"
//                              << " ktype:" << ktype
//                              << endl;
                    
                        if( ktype == "date" )
                        {
                            xmltype = "SQLDateT";
                        }
                        else if( starts_with( ktype, "timestamp") )
                        {
                            xmltype = "SQLTimeStampT";
                            defaultVal = "NOW()";
                        }
                        else if( ktype == "double" )
                        {
                            xmltype = "SQLDoubleT";
                        }
                    
                        if( std::string::npos != ktype.find("("))
                        {
                            fh_stringstream mss;
                            mss << ktype.substr( ktype.find("(")+1 );
                            mss >> maxlen;
                        }

                        bool isPrimary = false;
                        if( m_primaryKeyNames.end() != std::find(
                                m_primaryKeyNames.begin(), m_primaryKeyNames.end(),
                                *iter ))
                        {
                            isPrimary = true;
                        }
                        bool minOccur = isPrimary;
                        if( KeyNamesType[*iter].AutoIncrement() )
                            minOccur = 0;
                        if( KeyNamesType[*iter].AllowNulls() )
                            minOccur = 0;
                        
                        LG_SQLPLUS_D << "iter:" << *iter << endl;
                        LG_SQLPLUS_D << "ktype:" << ktype << endl;
                        LG_SQLPLUS_D << "maxlen:" << maxlen << endl;
                        LG_SQLPLUS_D << "isPrimary:" << isPrimary << endl;

                        ss << "<elementType name=\"" << *iter << "\" ";
                        ss << " minOccur=\"" << minOccur << "\" ";
                        ss << " maxOccur=\"" << 1 << "\" ";
                        if( maxlen )
                            ss << " maxLength=\"" << maxlen << "\" ";
                        if( defaultVal.length() )
                        {
                            ss << " default=\"" << defaultVal << "\" ";
                        }
                        ss   << " >\n"
                             << "<dataTypeRef name=\"" << xmltype << "\"/>\n"
                             << "</elementType>\n";
                    
                    }
                    ss << "	</elementType>\n";
                    
//                    cerr << "priv_FillCreateSubContextSchemaParts(A5)" << endl;
                
                    m["tuple"] = SubContextCreator(
                        SubContextCreator::Perform_t( this, &_Self::SubCreateTuple),
                        tostr(ss));
//                    cerr << "priv_FillCreateSubContextSchemaParts(A6)" << endl;
                }
                catch( std::exception& e )
                {
                }
            }
        

        virtual fh_iostream  getRealStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   exception)
            {
                bool virgin = true;
                fh_stringstream ret;

                Query query = getConnection( this )->query();
                setupQuery( query );
                Result res = query.store();

                virgin = true;
                if( res.size() )
                {
                    Row row = *(res.begin());
            
                    for (unsigned int i = 0; i < row.size(); i++)
                    {
                        if(!virgin) ret << ",";
                        virgin = false;

                        string k = getnames( res, i );
                        if( !k.length() )
                            continue;
            
                        ret << k;
                    }
                    ret << endl;
                }
        
                for (Result::iterator iter = res.begin(); iter != res.end(); iter++)
                {
                    Row row = *iter;

                    virgin = true;
                    for (unsigned int i = 0; i < res.size(); i++)
                    {
                        if(!virgin) ret << ",";
                        virgin = false;

                        ret << row.at(i);
                    }
                    ret << endl;
                }
                return ret;
            }

        virtual void populateKeyNames( PrimaryKeyNames_t& KeyNames )
            {
//                cerr << "SqlPlusContext::getKeyNames(2)" << endl;
                Query query = getConnection( this )->query();


                (std::ostream&)query << "describe " << getDirName();
                Result res = query.store();
        
                for (Result::iterator iter = res.begin(); iter != res.end(); iter++)
                {
                    Row row = *iter;
                    string k = (string)row["Field"];
                    string t = (string)row["Type"];
                    LG_SQLPLUS_D << "SqlPlusContext::getKeyNames() k:" << k
                                 << " t:" << t
                                 << endl;
                    KeyNames.push_back(k);

                    string ex = (string)row["Extra"];
                    bool autoinc = ( string::npos != ex.find("auto_increment") );

                    string allowNullStr = (string)row["Null"];
                    bool allowNull = allowNullStr == "YES";
                    KeyNamesType[k] = RowMetaData( t, autoinc, allowNull );
                }
        
                LG_SQLPLUS_D << "SqlPlusContext::getKeyNames(end)" << endl;
            }

        void priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    clearContext();
                    LG_SQLPLUS_D << "priv_read(top) path:" << getDirPath() << endl;

                    Query query = getConnection( this )->query();
                    setupQuery( query );
                    Result res = query.store();


                    LG_SQLPLUS_D << "priv_read() res.size:" << res.size() << endl;

                    if( res.size() )
                    {
                        Row row = *(res.begin());
            
                        for (unsigned int i = 0; i < row.size(); i++)
                        {
                            LG_SQLPLUS_D << "priv_read() i:" << i << endl;
                            string k = getnames( res, i );
                            LG_SQLPLUS_D << "priv_read() k:" << k << endl;
                            string v;
                            if( k.length() )
                            {
                                addAttribute( k, v, XSD_BASIC_STRING );

                                string lowerk = tolowerstr()( k );
                                addAttribute( lowerk, v, XSD_BASIC_STRING );
                            }
                        }
                    }
        
        
        
                    /*
                     * Create a file, and some EA for each row.
                     */
                    for (Result::iterator iter = res.begin(); iter != res.end(); iter++)
                    {
                        Row row = *iter;

                        string rdn = getPrimaryKeyRdn( m_primaryKeyNames, row );
                        SqlPlusTupleContext* child = new SqlPlusTupleContext( this, rdn, res, row );
                        addNewChild( child );
                    }
                }
                
                LG_SQLPLUS_D << "priv_read(done) path:" << getDirPath() << endl;
            }

        std::string getPrimaryKeyRdn( PrimaryKeyNames_t& pkn, Row row )
            {
                string rdn = "";
                bool v = true;
                for( PrimaryKeyNames_t::iterator iter = pkn.begin(); iter != pkn.end(); ++iter )
                {
                    if( !v ) rdn += "-";
                    v = false;
                    rdn += (string)row[ iter->c_str() ];
                }
                return rdn;
            }
        
        virtual void DeterminePrimaryKey()
            {
                LG_SQLPLUS_D << "DeterminePrimaryKey(1) path:" << getDirPath() << endl;
                bool v = true;
                Query query = getConnection( this )->query();
                Result res;

                LG_SQLPLUS_D << "DeterminePrimaryKey(2) path:" << getDirPath() << endl;
                LG_SQLPLUS_D << "DeterminePrimaryKey(2) m_tableQuerySQL:" << m_tableQuerySQL << endl;

                /*
                 * For queries we use the entire table as the primary key
                 */
                if( m_tableQuerySQL.length() )
                {
                    LG_SQLPLUS_D << "DeterminePrimaryKey(with sql) path:" << getDirPath() << endl;
        
                    setupQuery( query );
                    res = query.store();
                    LG_SQLPLUS_D << " res.size() :" << res.size() << endl;

                    if( res.size() < 1 )
                    {
                        m_recommendedEA += "";
                    }
                    else
                    {
                        Row row = *(res.begin());
            
                        for (unsigned int i = 0; i < row.size(); i++)
                        {
                            string k = getnames( res, i );
                            if( !k.length() )
                                continue;
                
                            LG_SQLPLUS_D << "res.names(i) : " << k << endl;
                            m_primaryKeyNames.push_back( k );
                        }

                        /*
                         * Default to showing the user a 100% projection of the table.
                         */
                        bool v = true;
                        for (unsigned int i = 0; i < row.size(); i++)
                        {
                            string k = getnames( res, i );
                            if( !k.length() )
                                continue;
                
                            if( !v )  m_recommendedEA += ",";
                            v = false;
                            LG_SQLPLUS_D << "adding:" << k << endl;
                            m_recommendedEA += k;
                        }
                        m_recommendedEA += ",name,primary-key";
                    }
                }
                else
                {
                    (std::ostream&)query << "describe " << getDirName();
                    res = query.store();
                    LG_SQLPLUS_D << "DeterminePrimaryKey(2) path:" << getDirPath() << endl;

                    for (Result::iterator iter = res.begin(); iter != res.end(); iter++)
                    {
                        Row row = *iter;
                        string ks = (string)row["Key"];
                        if( ks.length() )
                        {
                            m_primaryKeyNames.push_back( (string)row["Field"] );
                        }
                    }

            

                    if( ! m_primaryKeyNames.size() )
                    {
                        //
                        // Guess at key, make all the keys that don't allow nulls the key, or
                        // if everything allows nulls, then every column is part of the key.
                        //
                        for (Result::iterator iter = res.begin(); iter != res.end(); iter++)
                        {
                            Row row = *iter;
                            string ns = (string)row["Null"];
                            if( ns != "YES" )
                            {
                                m_primaryKeyNames.push_back( (string)row["Field"] );
                            }
                        }
                    }
                    if( ! m_primaryKeyNames.size() )
                    {
                        // primary key is each column
                        for (Result::iterator iter = res.begin(); iter != res.end(); iter++)
                        {
                            Row row = *iter;
                            m_primaryKeyNames.push_back( (string)row["Field"] );
                        }
                    }


                    /*
                     * Default to showing the user a 100% projection of the table.
                     */
                    v = true;
                    for (Result::iterator iter = res.begin(); iter != res.end(); iter++)
                    {
                        Row row = *iter;
                        if( !v )  m_recommendedEA += ",";
                        v = false;
                
                        m_recommendedEA += (string)row["Field"];
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

                LG_SQLPLUS_D << "SqlPlusContext::DeterminePrimaryKey() path:" << getDirPath()
                             << " pk:" << m_primaryKey
                             << endl;
            }

        virtual void executeInsert( const std::string& tableName,
                                    const std::string& sqlString )
            {
                LG_SQLPLUS_D << "INSERT:" << sqlString;
                Query query = getConnection( this )->query();
                (std::ostream&)query << sqlString;
                query.execute();
            }
        
        virtual fh_context addContextForResultingRow( const std::string& tableName,
                                                      const std::string& sqlString )
            {
                Query q = getConnection( this )->query();
                q << sqlString;
                Result res = q.store();
                Row row = *(res.begin());
                string rdn = getPrimaryKeyRdn( m_primaryKeyNames, row );
                SqlPlusTupleContext* child = new SqlPlusTupleContext( this, rdn, res, row );
                fh_context ret = child;
                addNewChild( child );
                return ret;
            }
        
        
        
    };
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/


    class FERRISEXP_CTXPLUGIN SqlPlusDBContext
        :
        public CommonSQLDBContext< SqlPlusDBContext >
    {
        typedef SqlPlusDBContext                         _Self;
        typedef CommonSQLDBContext< SqlPlusDBContext >   _Base;
        
    public:

        SqlPlusDBContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
        
    protected:

        virtual void addAllTables() 
            {
                Query query = getConnection( this )->query();
                (std::ostream&)query << "show tables";
                Result res = query.store();
                Result::iterator i;
                Row row;
                for (i = res.begin(); i != res.end(); i++)
                {
                    row = *i;
                    string tabname = (string)row.at(0);

                    LG_SQLPLUS_D << "  table:" << tabname << endl;

                    SqlPlusContext* cc = 0;
                    cc = priv_ensureSubContext( tabname, cc );
                    
//                     SqlPlusContext* tc = new SqlPlusContext( this, tabname );
//                     addNewChild( tc );

//                fh_istream iss = tc->getIStream();
//                 std::copy( std::istreambuf_iterator<char>(iss),
//                            std::istreambuf_iterator<char>(),
//                            std::ostreambuf_iterator<char>(cerr));
//                 cerr << endl;
                
                }
            }

        virtual Context* createSubContext( const fh_context& parent,
                                           const string& rdn,
                                           std::string QuSQL = "" )
            {
                return new SqlPlusContext( parent, rdn, QuSQL );
            }
        
        virtual void createSQLTable( const std::string& tableName,
                                     const std::string& sqlString )
            {
                Query query = getConnection( this )->query();
                (std::ostream&)query << sqlString;
                query.execute();
            }

        void
        priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
         {
//                 cerr << "priv_FillCreateSubContextSchemaParts(B)" << endl;
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

    class FERRISEXP_CTXPLUGIN SqlPlusServerContext
        :
        public CommonSQLDBServerContext< SqlPlusServerContext >
    {
        typedef CommonSQLDBServerContext< SqlPlusServerContext > _Base;

    public:
        
        SqlPlusServerContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
        
    protected:

        virtual void addAllDatabases()
            {
        
                LG_SQLPLUS_D << "addAllDatabases() path:" << getDirPath() << endl;

                Connection* con = getConnection( this );
        
                Query query = con->query();
                LG_SQLPLUS_D << "addAllDatabases( got q ) path:" << getDirPath() << endl;
                (std::ostream&)query << "show databases";

//                cerr << "addAllDatabases() have connection..." << con->connected() << endl;
        
                Result res = query.store();
//                cerr << "addAllDatabases() q res:" << query.success() << endl;
                if( !query.success() )
                {
                    LG_SQLPLUS_W << "addAllDatabases() q e:" << query.error() << endl;
                }
        
        
        
                LG_SQLPLUS_D << "addAllDatabases( got r ) path:" << getDirPath() << endl;
                Result::iterator i;
                Row row;
                for (i = res.begin(); i != res.end(); i++)
                {
                    row = *i;
                    string dbname = (string)row.at(0);
            
                    LG_SQLPLUS_D << "dbname:" << dbname << endl;
                    LG_SQLPLUS_D << "dbname:" << dbname << endl;
                    SqlPlusDBContext* cc = 0;
                    cc = priv_ensureSubContext( dbname, cc );
                    
//                     SqlPlusDBContext* dbc = new SqlPlusDBContext( this, dbname );
//                     addNewChild( dbc );
                }
            }
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_CTXPLUGIN sqlplusRootContext
        :
        public CommonSQLDBRootContext< SqlPlusServerContext >
    {
        typedef CommonSQLDBRootContext< SqlPlusServerContext > _Base;
        
    public:
        
        sqlplusRootContext( Context* parent,
                            const std::string& rdn,
                            bool bindall = false )
            :
            _Base( parent, rdn, bindall )
            {
            }
        
    protected:

        virtual std::string TryToCheckServerExists( const std::string& rdn )
            {
                string db = "";
                string serv = rdn;

                userpass_t up = getUserPass( rdn );
                
                try
                {
                    Connection con(use_exceptions);


                    Connection conp(use_exceptions);
                    conp.connect( db.c_str(), serv.c_str(),
                                  up.first.c_str(), up.second.c_str() );
                }
                catch( exception& e )
                {
                    fh_stringstream ss;
                    ss << "Error connecting to db:" << db
                       << " serv:" << serv
                       << " user:" << up.first.c_str()
                       << " pass:" << up.second.c_str()
                       << endl;
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
            
                static sqlplusRootContext* c = 0;
                const string& root = rf->getInfo( RootContextFactory::ROOT );

                if( !c )
                {
                    LG_SQLPLUS_D << "Making FakeInternalContext(1) " << endl;
                    c = new sqlplusRootContext(0, "/", false );
                    c->tryAugmentLocalhostNames();
                
                    LG_SQLPLUS_D << "Making FakeInternalContext(1.A) " << endl;
            
                    // Bump ref count.
                    static fh_context keeper = c;
                    LG_SQLPLUS_D << "Making FakeInternalContext(1.B) " << endl;
                    static fh_context keeper2 = keeper;

                    LG_SQLPLUS_D << "Making FakeInternalContext(2) " << endl;
//                 FakeInternalContext* servc = new FakeInternalContext( c, "localhost");
//                 c->addNewChild( servc );
                    LG_SQLPLUS_D << "Making FakeInternalContext(2.1) " << endl;
//                addAllDatabases( servc );
    
                    LG_SQLPLUS_D << "Making FakeInternalContext(3) " << endl;
                }

                LG_SQLPLUS_D << "Making FakeInternalContext(4) brewing return" << endl;

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
                LG_SQLPLUS_I << "e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }

};
