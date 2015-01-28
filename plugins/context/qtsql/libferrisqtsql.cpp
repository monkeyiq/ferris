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

    $Id: libferrisqtsql.cpp,v 1.10 2007/05/24 21:30:08 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <fstream>

#include <FerrisContextPlugin.hh>
#include <FerrisKDE.hh>
#include <Trimming.hh>
#include <FerrisDOM.hh>
#include <FerrisCreationPlugin.hh>
#include <FerrisBoost.hh>

#include <libcommonsqldbapi.hh>
#include <libferrisqtsqlshared.hh>

#include <QtCore>
#include <QtSql>

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
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    static string tostr( const QString& v )
    {
        string ret;
        int sz = v.size();
        ret.resize(sz);
        memcpy( (char*)ret.data(), v.toUtf8().data(), sz );
        return ret;
//        return v.toUtf8().data();
    }

    static string toSQLType( QVariant::Type t )
    {
        // FIXME!
        return "varchar";
    }

    stringmap_t& getSupportedServers()
    {
        static stringmap_t ret;
        
        ret.insert(make_pair("postgresql","QPSQL"));
        ret.insert(make_pair("mysql","QMYSQL"));
        ret.insert(make_pair("sqlite","QSQLITE"));
        return ret;
    }
    
    
    QSqlDatabase basic_getConnection( const std::string& qdriver,
                                      const std::string& host, const std::string& dbname,
                                      const std::string& user, const std::string& pass )
    {
        stringstream connectionNameSS;
        connectionNameSS << qdriver << "-" << host;
        if( !dbname.empty() )
            connectionNameSS << "-" << dbname;
        string connectionName = tostr(connectionNameSS);
        
        QSqlDatabase db = QSqlDatabase::database( connectionName.c_str() );
        if( !db.isValid() )
        {
            db = QSqlDatabase::addDatabase( qdriver.c_str(), connectionName.c_str() );
            LG_QTSQL_D << " qtDatabaseType:" << qdriver
                       << " host:" << host
                       << " dbname:" << dbname
                       << " user:" << user
                       << endl;
            db.setHostName(host.c_str());
            if( !dbname.empty() )
                db.setDatabaseName(dbname.c_str());
            db.setUserName(user.c_str());
            db.setPassword(pass.c_str());
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
        return db;
    }
    QSqlDatabase basic_getConnection( const std::string& qdriver,
                                      const std::string& host, const std::string& dbname,
                                      userpass_t up )
    {
        return basic_getConnection( qdriver, host, dbname, up.first, up.second );
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    typedef vector< string > PrimaryKeyNames_t;
    class QtsqlContext;


    class FERRISEXP_CTXPLUGIN QtsqlTupleContext
        :
        public CommonSQLDBTupleContext< QtsqlTupleContext, QtsqlContext >
    {
        typedef QtsqlTupleContext                            _Self;
        typedef CommonSQLDBTupleContext< QtsqlTupleContext, QtsqlContext > _Base;

    public:

        void setIter( QSqlQuery& query, QSqlRecord& rec )
            {
                for( int i=0; i<rec.count(); ++i )
                {
                    QSqlField f = rec.field( i );
                    string k = tostr(f.name());

                    int colnum = rec.indexOf( k.c_str() );
                    string v = tostr( query.value( colnum ).toString() );

                    LG_QTSQL_D << "QtsqlTupleContext() q.v.sz:" << query.value( colnum ).toString().size() << endl;
                    LG_QTSQL_D << "QtsqlTupleContext() k:" << k << endl;
                    LG_QTSQL_D << "QtsqlTupleContext() v.sz:" << v.size() << endl;
//                    LG_QTSQL_D << "QtsqlTupleContext() v:" << v << endl;
                    
                    kvm[k]=v;

                    string lowerk = tolowerstr()( k );
                    kvm[ lowerk ]=v;
                }
            }
        
        QtsqlTupleContext( const fh_context& parent, const string& rdn,
                           QSqlQuery& query, QSqlRecord& rec )
            :
            _Base( parent, rdn )
            {
                setIter( query, rec );
                
                string className = parent->getURL();
                bool force = AttributeCollection::isStateLessEAVirgin( className );
                LG_QTSQL_D << "QtsqlTupleContext(ctor) force:" << force << " rdn:" << rdn << endl;
                setup_DynamicClassedStateLessEAHolder( className );
                if( force )
                {
#define SLEA tryAddStateLessAttribute

                    for( int i=0; i<rec.count(); ++i )
                    {
                        QSqlField f = rec.field( i );
                        string k = tostr(f.name());
                        if( !k.length() )
                            continue;

                        LG_QTSQL_D << "Adding SLEA for k:" << k << endl;
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
                    LG_QTSQL_D << "Context::createStateLessAttributes() path:" << getDirPath() << endl;
                    _Base::createStateLessAttributes( force );
                }
            }
        
        virtual bool supportsReClaim()
            {
                return true;
            }

        QSqlDatabase getConnection();
        XSDBasic_t getSchemaType( const std::string& eaname );

    protected:

        virtual void executeUpdate( const std::string& sqlString )
            {
//                 Query query = getConnection( this )->query();
//                 (std::ostream&)query << sqlString; 
//                 query.execute();

                QSqlDatabase db = getConnection();
                db.exec( sqlString.c_str() );
            }

        virtual std::string getTableName()
            {
                return getParent()->getDirName();
            }
        
    };

    struct QtsqlTupleContextCreator
    {
        QSqlQuery&  m_query;
        QSqlRecord& m_iter;
            
        QtsqlTupleContextCreator( QSqlQuery& query, QSqlRecord& m_iter )
            :
            m_query( query ),
            m_iter( m_iter )
            {}
        
        QtsqlTupleContext* create( Context* parent, const std::string& rdn ) const
            {
                QtsqlTupleContext* ret = new QtsqlTupleContext( parent, rdn, m_query, m_iter );
                return ret;
            }
        void setupExisting( QtsqlTupleContext* fc ) const
            {
                fc->setIter( m_query, m_iter );
            }
        void setupNew( QtsqlTupleContext* fc ) const
            {}
    };

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    
    class FERRISEXP_CTXPLUGIN QtsqlContext
        :
        public CommonSQLContext< QtsqlContext >
    {
        typedef QtsqlContext _Self;
        typedef CommonSQLContext< QtsqlContext > _Base;


        string setupQuery()
            {
                fh_stringstream ss;
        
                if( m_tableQuerySQL.length() )
                {
                    LG_QTSQL_D << "m_tableQuerySQL:" << m_tableQuerySQL << endl;
                    ss << m_tableQuerySQL;
                }
                else
                {
                    ss << "select * from " << getDirName();
                }

                LG_QTSQL_D << "setupQuery() SQL:" << tostr(ss) << endl;
                return tostr( ss );
            }
        
    public:

        QtsqlContext( const fh_context& parent, const string& rdn, string QuSQL = "" )
            :
            _Base( parent, rdn, QuSQL )
            {
                DeterminePrimaryKey();
                createStateLessAttributes();
            }
        
        virtual ~QtsqlContext()
            {
            }

        QSqlDatabase getConnection();
        
    protected:

        virtual bool supportsReClaim()
            {
                return true;
            }
        
        
        virtual fh_iostream  getRealStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   exception)
            {
                bool virgin = true;
                fh_stringstream ret;

                QSqlDatabase db = getConnection();
                QSqlQuery query = db.exec( setupQuery().c_str() );
                while (query.next())
                {
                    virgin = true;
                    QSqlRecord rec = query.record();
                    for( int i=0; i<rec.count(); ++i )
                    {
                        QSqlField f = rec.field( i );
                        QVariant v = f.value ();
                        string k = tostr(v.toString());

                        if(!virgin) ret << ",";
                        virgin = false;

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
                LG_QTSQL_D << "populateKeyNames(top)" << endl;

                QSqlDatabase db = getConnection();
                string tableName = getDirName();
                QSqlRecord r = db.record( tableName.c_str() );

                for( int i=0; i<r.count(); ++i )
                {
                    QSqlField f = r.field ( i );

                    string sqltype = toSQLType( f.type() );
                    string attname = tostr(f.name());
                    bool autoinc = f.isAutoValue();
                    bool allowNull = QSqlField::Required != f.requiredStatus();
                    
                    KeyNamesType[attname] = RowMetaData( sqltype, autoinc, allowNull );
                    KeyNames.push_back( attname );
                    LG_QTSQL_D << " attrname:" << attname << " type:" << sqltype << endl;
                }
                
                LG_QTSQL_D << "populateKeyNames(end)" << endl;
            }

        virtual bool isDir()
            {
                return true;
            }
        
        void priv_read()
            {
                LG_QTSQL_D << "QtsqlContext::read(1) HaveReadDir:" << getHaveReadDir()
                        << " m_primaryKeyNames.size:" << m_primaryKeyNames.size() << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                clearContext();
                LG_QTSQL_D << "priv_read(top) path:" << getDirPath() << endl;

                getKeyNames();
                {
                    QSqlDatabase db = getConnection();
                    string tableName = getDirName();
                    
                    // FIXME: getting record for an SQL view?
                    QSqlRecord r = db.record( tableName.c_str() );
                    for( int i=0; i<r.count(); ++i )
                    {
                        QSqlField f = r.field ( i );

                        string k = tostr(f.name());
                        if( !k.length() )
                            continue;

                        string v;
                        if( k.length() )
                        {
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
                    QSqlQuery query = db.exec( setupQuery().c_str() );
                    QSqlRecord rec = query.record();
                    while (query.next())
                    {
                        string rdn = getPrimaryKeyRdn( m_primaryKeyNames, query, rec );
                        LG_QTSQL_D << "rdn:" << rdn << endl;
                        LG_QTSQL_D << "m_primaryKeyNames.sz:" << m_primaryKeyNames.size() << endl;

                        
                        QtsqlTupleContext* cc = 0;
                        cc = priv_ensureSubContext(
                            rdn, cc, QtsqlTupleContextCreator( query, rec ) );
                    }
                }
                
                LG_QTSQL_D << "priv_read(done) HaveReadDir:" << getHaveReadDir()
                        << " path:" << getDirPath() << endl;
            }

        std::string getPrimaryKeyRdn( PrimaryKeyNames_t& pkn, QSqlQuery& query, QSqlRecord& rec )
            {
                
                string rdn = "";
                bool v = true;
                for( PrimaryKeyNames_t::iterator iter = pkn.begin(); iter != pkn.end(); ++iter )
                {
                    if( !v ) rdn += "-";
                    v = false;
                    int colnum = rec.indexOf( iter->c_str() );
                    string v = tostr( query.value( colnum ).toString() );
                    rdn += v;
                }
                return rdn;
            }

        virtual stringlist_t DeterminePrimaryKey_getBaseTableNamesFromViewDef( const std::string& viewDef )
            {
                int p = viewDef.find("FROM ");
                LG_QTSQL_D << "p:" << p << endl;
                list< string > tables;
                if( p != string::npos )
                {
                    stringstream ss;
                    ss << viewDef.substr( p+5 );
                    LG_QTSQL_D << "ss:" << ss.str() << endl;
                    while( ss )
                    {
                        string tname;
                        getline( ss, tname, " ,;" );
                        tables.push_back( tname );
                        LG_QTSQL_D << "found table:" << tname << endl;
                        char ch;
                        while( ss >> noskipws >> ch )
                        {
                            if( ch == ',' )
                            {
                                LG_QTSQL_D << "BREAK:" << ch << endl;
                                ss >> noskipws >> ch;
                                break;
                            }
                            LG_QTSQL_D << "skipping:" << ch << endl;
                        }
                    }
                }

                return tables;
            }


        virtual void DeterminePrimaryKey_forBaseTableName( const std::string& tableName,
                                                           stringset_t& pk,
                                                           stringset_t& uniqcols )
            {
                QSqlDatabase db = getConnection();
                {
                    QSqlIndex idx = db.primaryIndex( tableName.c_str() );
                    for( int i=0; i<idx.count(); ++i )
                    {
                        QSqlField f = idx.field ( i );
                        string n = tostr(f.name());
                        pk.insert( n );
                    }
                }
                {
                    QSqlRecord rec = db.record( tableName.c_str() );
                    for( int i=0; i<rec.count(); ++i )
                    {
                        QSqlField f = rec.field ( i );
                        string n = tostr(f.name());
                        if( f.requiredStatus() == QSqlField::Required )
                            uniqcols.insert(n);
                    }
                }
            }
        
        virtual void DeterminePrimaryKey()
            {
                LG_QTSQL_D << "DeterminePrimaryKey(1) path:" << getDirPath() << endl;
                bool v = true;

                LG_QTSQL_D << "DeterminePrimaryKey(2) path:" << getDirPath() << endl;
                LG_QTSQL_D << "DeterminePrimaryKey(2) m_tableQuerySQL:" << m_tableQuerySQL << endl;

                /*
                 * For queries we use the entire table as the primary key
                 */
                if( m_tableQuerySQL.length() )
                {
                    LG_QTSQL_D << "DeterminePrimaryKey(with sql) path:" << getDirPath() << endl;

                    QSqlDatabase db = getConnection();
                    QSqlQuery query = db.exec( setupQuery().c_str() );
                    QSqlRecord rec = query.record();
                    for( int i=0; i<rec.count(); ++i )
                    {
                        QSqlField f = rec.field ( i );
                        string k = tostr(f.name());
                        
                        m_primaryKeyNames.push_back( k );
                        m_recommendedEA += ",";
                        m_recommendedEA += k;
                    }
                    m_recommendedEA += ",name,primary-key";
                    LG_QTSQL_D << "DeterminePrimaryKey(with sql) done...m_recommendedEA:" << m_recommendedEA << endl;
                }
                else
                {
                    string tableName = getDirName();
                    stringset_t pk;
                    stringset_t uniqcols;
                    
                    QSqlDatabase db = getConnection();
                    DeterminePrimaryKey_forBaseTableName( tableName, pk, uniqcols );
                    
                    LG_QTSQL_D << "After initial attempt to find pk..." << endl;
                    LG_QTSQL_D << "pk:" << Util::createSeperatedList( pk ) << endl;
                    LG_QTSQL_D << "uniqcols:" << Util::createSeperatedList( uniqcols ) << endl;
                                    
                    //
                    // If is a view then make the pk = union(basetable.pk)
                    //
                    if( pk.empty() )
                    {
                        LG_QTSQL_D << "FIXME: DeterminePrimaryKey() trying to determine view key" << endl;
                        PrimaryKeyNames_t tmp;
                        populateKeyNames( tmp );
                        copy( tmp.begin(), tmp.end(), inserter( pk, pk.end() ));

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
                        // FIXME:
                        
                    }

                    copy( pk.begin(), pk.end(), back_inserter(m_primaryKeyNames));

                    /*
                     * Default to showing the user a 100% projection of the table.
                     */
                    {
                        v = true;
                        QSqlDatabase db = getConnection();
                        QSqlRecord rec = db.record( tableName.c_str() );
                        for( int i=0; i<rec.count(); ++i )
                        {
                            QSqlField f = rec.field ( i );
                            string an = tostr(f.name());
                            if( !v )  m_recommendedEA += ",";
                            v = false;

                            m_recommendedEA += an;
                        }
                        LG_QTSQL_D << "XXX1 tablename:" << tableName
                                   << " rec.count:" << rec.count()
                                   << " m_recommendedEA:" << m_recommendedEA
                                   << endl;
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

                LG_QTSQL_D << "QtsqlContext::DeterminePrimaryKey() path:" << getDirPath()
                           << " pk:" << m_primaryKey
                           << " m_recommendedEA:" << m_recommendedEA
                           << endl;
            }

        virtual void executeInsert( const std::string& tableName,
                                    const std::string& sqlString )
            {
                LG_QTSQL_D << "SQL-INSERT:" << sqlString;

                try
                {
                    QSqlDatabase db = getConnection();
                    db.exec( sqlString.c_str() );
                    LG_QTSQL_D << "DONE." << endl;
                }
                catch( exception& e )
                {
                    LG_QTSQL_D << "ERROR e:" << e.what() << endl;
                    throw;
                }
            }
        
        virtual fh_context addContextForResultingRow( const std::string& tableName,
                                                      const std::string& sqlString )
            {
                QSqlDatabase db = getConnection();
                QSqlQuery query = db.exec( sqlString.c_str() );
                QSqlRecord rec = query.record();
                while (query.next())
                {
                    string rdn = getPrimaryKeyRdn( m_primaryKeyNames, query, rec );

                    if( priv_isSubContextBound( rdn ) )
                    {
                        fh_context ret = getSubContext( rdn );
                        return ret;
                    }
                    else
                    {
                        QtsqlTupleContext* child = new QtsqlTupleContext( this, rdn, query, rec );
                        fh_context ret = child;
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
                QtsqlTupleContext* c = dynamic_cast<QtsqlTupleContext*>( (GetImpl(c_ctx) ) );
                if( !c )
                {
                    fh_stringstream ss;
                    ss << "Attempt to remove a non qtsql tuple context! url:" << c_ctx->getURL();
                    Throw_CanNotDelete( tostr(ss), GetImpl(c_ctx) );
                }
                if( !m_tableQuerySQL.empty() )
                {
                    LG_QTSQL_D << "m_tableQuerySQL is not empty..." << endl;
                    stringstream ss;
                    ss << " can not delete from a filesystem made from an SQL query." << endl;
                    Throw_CanNotDelete( tostr(ss), GetImpl(c_ctx) );
                }
                string url = c->getURL();
                LG_QTSQL_D << "remove() url:" << url << endl;

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

                LG_QTSQL_D << "REMOVAL SQL:" << ss.str() << endl;

                QSqlDatabase db = getConnection();
                db.exec( tostr(ss).c_str() );
            }
    };

    
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    class FERRISEXP_CTXPLUGIN QtsqlDBContext
        :
        public CommonSQLDBContext< QtsqlDBContext >
    {
        typedef QtsqlDBContext                         _Self;
        typedef CommonSQLDBContext< QtsqlDBContext >   _Base;

        virtual bool priv_supportsShortCutLoading()
            { return true; }

        virtual fh_context priv_getSubContext( const std::string& rdn )
            throw( NoSuchSubContext )
            {
                try
                {
                    LG_QTSQL_D << "priv_getSubContext() p:" << this->getDirPath()
                               << " rdn:" << rdn
                               << endl;
                    Context::Items_t::iterator isSubContextBoundCache;
                    if( this->priv_isSubContextBound( rdn, isSubContextBoundCache ) )
                    {
                        LG_QTSQL_D << "priv_getSubContext(bound already) p:" << this->getDirPath()
                                   << " rdn:" << rdn
                                   << endl;
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

//                     //
//                     // its a database function!
//                     //
//                     if( contains( rdn, "(" ) )
//                     {
//                         stringstream sqlss;
//                         sqlss << "SELECT * from " << rdn << ";";

//                         LG_QTSQL_D << "sql:" << sqlss.str() << endl;
//                         LG_QTSQL_D << "making custom function context...1" << endl;
//                         QtsqlContext* cc = new QtsqlContext( this, rdn, tostr(sqlss) );
//                         Insert( cc );
//                         LG_QTSQL_D << "returning custom function context..." << endl;
//                         return cc;
//                     }
                    
                    return _Base::priv_getSubContext( rdn );
                }
                catch( NoSuchSubContext& e )
                {
                    throw e;
                }
                catch( exception& e )
                {
                    string s = e.what();
                    Throw_NoSuchSubContext( s, this );
                }
                catch(...)
                {}
                fh_stringstream ss;
                ss << "NoSuchSubContext:" << rdn;
                Throw_NoSuchSubContext( tostr(ss), this );
            }
                    
        
    public:

        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
        
        QtsqlDBContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
//                cerr << "QtsqlDBContext() rdn:" << rdn << endl;
            }

        virtual ~QtsqlDBContext()
            {
                LG_QTSQL_D << "~QtsqlDBContext() rdn:" << getDirName() << endl;
            }
        
        virtual bool supportsReClaim()
            {
                return true;
            }

        QSqlDatabase getConnection();
        
    protected:

        string getHostName()
            {
                return getParent()->getDirName();
            }
        string getQDriverName();
        string getDBName()
            {
                return getDirName();
            }
        
        
        virtual void addAllTables() 
            {
                LG_QTSQL_D << "addAllTables(top)" << endl;
                try
                {
                    QSqlDatabase db = getConnection();
                    QStringList sl = db.tables( QSql::TableType(QSql::Tables | QSql::Views) );
                    for (QStringList::iterator iter = sl.begin(); iter != sl.end(); ++iter)
                    {
                        string rdn = iter->toLocal8Bit().constData();
                        LG_QTSQL_D << "table:" << rdn << endl;
                        
                        QtsqlContext* cc = 0;
                        cc = priv_ensureSubContext( rdn, cc );
                    }
                }
                catch( exception& e )
                {
                    LG_PG_W << "addAllTables() e:" << e.what() << endl;
                    throw;
                }
            }

        virtual Context* createSubContext( const fh_context& parent,
                                           const string& rdn,
                                           std::string QuSQL = "" )
            {
                return new QtsqlContext( parent, rdn, QuSQL );
            }
        
        virtual void createSQLTable( const std::string& tableName,
                                     const std::string& sqlString )
            {
                QSqlDatabase db = getConnection();
                db.exec( sqlString.c_str() );
            }

        void
        priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
         {
                LG_QTSQL_D << "priv_FillCreateSubContextSchemaParts()" << endl;
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

    
    class FERRISEXP_CTXPLUGIN QtsqlServerContext
        :
        public CommonSQLDBServerContext< QtsqlServerContext >
    {
        typedef CommonSQLDBServerContext< QtsqlServerContext > _Base;

    public:
        
        QtsqlServerContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
//                cerr << "QtsqlServerContext()" << endl;
            }

        virtual ~QtsqlServerContext()
            {
                LG_QTSQL_D << "~QtsqlServerContext() rdn:" << getDirName() << endl;
            }
        
        virtual bool supportsReClaim()
            {
                return true;
            }


    public:

        string getDBServerName()
            {
                return getParent()->getDirName();
            }
        string getQDriverName()
            {
                stringmap_t& supported_servers = getSupportedServers();
                return supported_servers[ getDBServerName() ];
            }
        
    protected:

        
        virtual void addAllDatabases()
            {
                LG_QTSQL_D << "addAllDatabases() path:" << getDirPath() << endl;
                string rdn = getDirName();
                userpass_t up = getQtSQLUserPass( rdn );

                try
                {
                    QSqlDatabase db = basic_getConnection( getQDriverName(), rdn, "", up );
                    if( !db.open() )
                    {
                        fh_stringstream ss;
                        ss << "Error to server:" << rdn
                           << " user:" << up.first
                           << endl;
                        LG_QTSQL_D << ss.str() << endl;
                        Throw_CanNotReadContext( tostr(ss), this );
                    }

                    string dbserver = getDBServerName();
                    stringstream sqlss;
                    if( dbserver == "postgresql" )
                    {
                        sqlss << "SELECT d.datname as Name, u.usename as Owner, "
                              << "     pg_catalog.pg_encoding_to_char(d.encoding) as Encoding "
                              << "FROM pg_catalog.pg_database d  "
                              << "LEFT JOIN pg_catalog.pg_user u ON d.datdba = u.usesysid order by 1;";
                    }
                    else if( dbserver == "mysql" )
                    {
                        sqlss << "show databases";
                    }
                    else
                    {
                        stringstream ss;
                        ss << "It would seem that QtSQL is unable to list the databases for a server" << endl
                           << " in a generic way. There is no SQL snippit in libferris currently for" << endl
                           << " the database type:" << dbserver << endl;
                        cerr << ss.str() << endl;
                        LG_QTSQL_ER << ss.str() << endl;
                        Throw_CanNotReadContext( tostr(ss), this );
                    }
                    LG_QTSQL_D << "sql:" << sqlss.str() << endl;

                    QSqlQuery query = db.exec( sqlss.str().c_str() );
                    while (query.next())
                    {
                        string name = tostr(query.value(0).toString());
                        LG_QTSQL_D << " name:" << name << endl;
                        QtsqlDBContext* cc = 0;
                        cc = priv_ensureSubContext( name, cc );
                    }
                }
                catch( exception& e )
                {
                    fh_stringstream ss;
                    ss << "Error to server:" << rdn
                       << " user:" << up.first
                       << " e:" << e.what()
                       << endl;
                    LG_QTSQL_D << "connect error:" << tostr(ss) << endl;
                    throw;
                }
            }
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_CTXPLUGIN QtsqlRootContext
        :
        public CommonSQLDBRootContext< QtsqlServerContext >
    {
        typedef CommonSQLDBRootContext< QtsqlServerContext > _Base;

        string m_qtDatabaseType;
        
    public:
        
        QtsqlRootContext( Context* parent,
                          const std::string& rdn,
                          bool bindall = false,
                          string m_qtDatabaseType = "" )
            :
            _Base( parent, rdn, bindall ),
            m_qtDatabaseType( m_qtDatabaseType )
            {
//                cerr << "ctor2, have read:" << getHaveReadDir() << " rdn:" << rdn << " dirname:" << getDirName() << endl;
            }
        
    protected:

        virtual std::string TryToCheckServerExists( const std::string& rdn )
            {
                LG_QTSQL_D << "TryToCheckServerExists(A) m_qtDatabaseType:" << m_qtDatabaseType << " rdn:" << rdn << endl;
                
                ::Ferris::KDE::ensureKDEApplication();

                LG_QTSQL_D << "TryToCheckServerExists() rdn:" << rdn << endl;

                if( isParentBound() && getDirName() == "sqlite" )
                {
                    if( rdn == "localhost" )
                        return "";
                    return "error";
                }
                
                userpass_t up = getQtSQLUserPass( rdn );
                
                fh_stringstream ss;
                try
                {
                    LG_QTSQL_D << "m_qtDatabaseType:" << m_qtDatabaseType << endl;

                    QSqlDatabase db = basic_getConnection( m_qtDatabaseType, rdn, "", up );
                    LG_QTSQL_D << "rdn:" << rdn
                               << " user:" << up.first
                               << " pass:" << up.second
                               << endl;
                    if( ! db.isOpen() )
                    {
                        LG_QTSQL_D << "TryToCheckServerExists failed to connect, rdn:" << rdn << endl;
                        return "error connecting...";
                    }
                    LG_QTSQL_D << "database exists, rdn:" << rdn << endl;
                    return "";
                }
                catch( exception& e )
                {
                    fh_stringstream ss;
                    ss << "Error to server:" << rdn
                       << " user:" << up.first
                       << " e:" << e.what()
                       << endl;
                    LG_QTSQL_D << "connect error:" << tostr(ss) << endl;
                    return tostr(ss);
                }
                return _Base::TryToCheckServerExists( rdn );
            }
    };

    QSqlDatabase QtsqlContext::getConnection()
    {
        return getFirstParentOfContextClass<>( (QtsqlDBContext*)0)->getConnection();
    }

    QSqlDatabase QtsqlTupleContext::getConnection()
    {
        return getFirstParentOfContextClass<>( (QtsqlDBContext*)0)->getConnection();
    }

    XSDBasic_t
    QtsqlTupleContext::getSchemaType( const std::string& eaname )
    {
        if( QtsqlContext* p = dynamic_cast<QtsqlContext*>( getParent() ) )
            return p->getSchemaType( eaname );
        return XSD_BASIC_STRING;
    }
    
    string
    QtsqlDBContext::getQDriverName()
    {
        if( !isParentBound() )
        {
            // SQLITE overmount...
            return getSupportedServers()["sqlite"];
        }
        
        LG_QTSQL_D << "QtsqlDBContext::getQDriverName() ret:" << getFirstParentOfContextClass<>( (QtsqlServerContext*)0)->getQDriverName() << endl;
        
        return getFirstParentOfContextClass<>( (QtsqlServerContext*)0)->getQDriverName();
    }
    
    QSqlDatabase
    QtsqlDBContext::getConnection()
    {
        string qtDatabaseType = getQDriverName();
        string host = getHostName();
        string dbname = getDBName();
        userpass_t up = getQtSQLUserPass( host );
        QSqlDatabase db = basic_getConnection( getQDriverName(), host, dbname, up );
        if( !db.open() )
        {
            fh_stringstream ss;
            ss << "Error to server:" << host
               << " user:" << up.first
               << endl;
            LG_QTSQL_D << ss.str() << endl;
            Throw_CanNotReadContext( tostr(ss), this );
        }
        return db;
    }
    
    
//     connection& QtsqlDBContext::getConnection()
//     {
//         if( m_connection )
//             return (*m_connection);
                
//         string rdn = getDirName();
//         userpass_t up = getQtsqlUserPass( rdn );
                
//         fh_stringstream ss;
//         ss << "host=" << getParent()->getDirName()      << " ";
//         if( !up.first.empty() )
//             ss << "user=" << up.first << " ";
//         if( !up.second.empty() )
//             ss << "password=" << up.second << " ";
//         ss << " dbname=" << getDirName();
//         ss << endl;
//         LG_QTSQL_D << "TryToCheckServerExists data:" << tostr(ss) << endl;

//         try
//         {
//             m_connection = new connection( ss.str() );
//         }
//         catch( exception& e )
//         {
//             LG_QTSQL_D << "QtsqlDBContext::getConnection(err) e:" << e.what() << endl;
//             throw;
//         }
//         return (*m_connection);
//     }


    class FERRISEXP_CTXPLUGIN QtsqlDatabaseServerTypeListContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        bool m_haveTriedToRead;
    public:

        QtsqlDatabaseServerTypeListContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_haveTriedToRead( false )
            {
                LG_QTSQL_D << "ctor, have read:" << getHaveReadDir() << endl;
            }

        void priv_read()
            {
                LG_QTSQL_D << "priv_read() url:" << getURL()
                           << " m_haveTriedToRead:" << m_haveTriedToRead
                           << " have read:" << getHaveReadDir()
                           << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                
                if( m_haveTriedToRead )
                {
                    emitExistsEventForEachItemRAII _raii1( this );
                }
                else
                {
                    m_haveTriedToRead = true;
                    stringmap_t& supported_servers = getSupportedServers();
                    
                    for( stringmap_t::iterator si = supported_servers.begin();
                         si != supported_servers.end(); ++si )
                    {
                        LG_QTSQL_D << "priv_read() url:" << getURL() << " populating... si:" << si->first << endl;
                        QtsqlRootContext* c = new QtsqlRootContext( this, si->first, false, si->second );
                        fh_context subc = c;
                        c->tryAugmentLocalhostNames();
                        Insert( c );
                    }
                }
            
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
                LG_QTSQL_D << "Brew()" << endl;
                
                static QtsqlDatabaseServerTypeListContext* c = 0;
                if( !c )
                {
                    c = new QtsqlDatabaseServerTypeListContext(0, "/" );
                    // Bump ref count.
                    static fh_context keeper = c;
                    static fh_context keeper2 = keeper;
                }
                fh_context ret = c;

                {
                    const string& root = rf->getInfo( RootContextFactory::ROOT );
                    LG_QTSQL_D << "Brew() root:" << root << endl;

                    if( !root.empty() && root != "/" )
                    {
                        stringstream ss;
                        ss << "/sqlite/localhost/" << root;
                        rf->AddInfo( RootContextFactory::ROOT, ss.str() );

                        LG_QTSQL_D << "Brew(2) root:" << rf->getInfo( RootContextFactory::ROOT ) << endl;

                        ret = new QtsqlDBContext( 0, root );
                    }
                }
                
                
                
//                 static QtsqlRootContext* c = 0;
//                 const string& root = rf->getInfo( RootContextFactory::ROOT );

//                 LG_QTSQL_D << "Brew() root:" << root << endl;
                
//                 if( !c )
//                 {
//                     LG_QTSQL_D << "Qtsql making root context " << endl;
//                     c = new QtsqlRootContext(0, "/", false );

//                     LG_QTSQL_D << "Qtsql adding localhosts to root context " << endl;
//                     c->tryAugmentLocalhostNames();
                
//                     // Bump ref count.
//                     static fh_context keeper = c;
//                     static fh_context keeper2 = keeper;
//                 }

//                 fh_context ret = c;

//                 if( root != "/" )
//                 {
//                     string path = rf->getInfo( RootContextFactory::PATH );
//                     string newpath = root;

//                     newpath += "/";
//                     newpath += path;

//                     rf->AddInfo( RootContextFactory::PATH, newpath );
//                 }
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
