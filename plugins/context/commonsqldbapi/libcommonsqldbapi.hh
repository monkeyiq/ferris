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

    $Id: libcommonsqldbapi.hh,v 1.6 2010/09/24 21:31:33 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_COMMONSQLDB_H_
#define _ALREADY_INCLUDED_FERRIS_COMMONSQLDB_H_

#include <Ferris.hh>
#include <FerrisDOM.hh>
#include <FerrisContextPlugin.hh>
#include <Trimming.hh>
#include <FerrisCreationPlugin.hh>
#include <Configuration_private.hh>
#include <PluginOutOfProcNotificationEngine.hh>

#include <errno.h>

namespace Ferris
{
    using namespace std;

    /**
     * Subclasses should override
     * TryToCheckServerExists()
     */
    template < class OurDirectChildrenContextClass >
    class CommonSQLDBRootContext
        :
        public networkRootContext< OurDirectChildrenContextClass >
    {
        typedef CommonSQLDBRootContext< OurDirectChildrenContextClass > _Self;
        typedef networkRootContext< OurDirectChildrenContextClass >     _Base;
        
    protected:

        virtual void createStateLessAttributes( bool force = false )
            {
                LG_SQLDB_D << "RootContext::createStateLessAttributes() " << endl;
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    _Base::createStateLessAttributes( true );
                    this->supplementStateLessAttributes( true );
                }
            }

        /* We want to make sure that this object is always around */
        enum {
            REFCOUNT = 3
        };
        virtual FerrisLoki::Handlable::ref_count_t AddRef()
            { return REFCOUNT; }
        virtual FerrisLoki::Handlable::ref_count_t Release()
            { return REFCOUNT; }
        
    public:
        
        CommonSQLDBRootContext( Context* parent,
                                const std::string& rdn,
                                bool bindall = false )
            :
            _Base( parent, rdn, bindall )
            {
                LG_SQLDB_D << "sqlplusRootContext() rdn:" << rdn << endl;
                createStateLessAttributes();
            }
        
        virtual ~CommonSQLDBRootContext()
            {
            }
        
    };


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Subclasses should override
     * addAllDatabases()
     */
    template < class ChildContextType >
    class CommonSQLDBServerContext
        :
        public StateLessEAHolder< ChildContextType, FakeInternalContext >
    {
        typedef CommonSQLDBServerContext                                   _Self;
        typedef StateLessEAHolder< ChildContextType, FakeInternalContext > _Base;

    protected:

        virtual void addAllDatabases() = 0;

        virtual void priv_read()
            {
                LG_SQLDB_D << "ServerContext::priv_read(T) path:" << this->getDirPath() << endl;

                Context::EnsureStartStopReadingIsFiredRAII _raii1( this );
                this->clearContext();
                LG_SQLDB_D << "priv_read(top) path:" << this->getDirPath() << endl;

                addAllDatabases();
        
                LG_SQLDB_D << "priv_read(done) path:" << this->getDirPath() << endl;
            }
    
        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    _Base::createStateLessAttributes( true );
                    this->supplementStateLessAttributes( true );
                }
            }
        
    public:

        CommonSQLDBServerContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                LG_SQLDB_D << "ServerContext rdn:" << rdn
                           << " parent:" << (parent?parent->getURL():"<null>")
                           << endl;
                createStateLessAttributes();
            }
    };


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Subclasses should override
     * addAllTables()
     * createSubContext()
     * createSQLTable()
     */
    template < class ChildContextType >
    class CommonSQLDBContext
        :
        public StateLessEAHolder< ChildContextType, FakeInternalContext >
    {
        typedef CommonSQLDBContext                                         _Self;
        typedef StateLessEAHolder< ChildContextType, FakeInternalContext > _Base;

    public:

        CommonSQLDBContext( Context* parent, const std::string& rdn )
            {
                this->setContext( parent, rdn );
                this->createStateLessAttributes();
            }
        
    protected:

        virtual void addAllTables() = 0;
        virtual Context* createSubContext( const fh_context& parent,
                                           const std::string& rdn,
                                           std::string QuSQL = "" ) = 0;
        virtual void createSQLTable( const std::string& tableName,
                                     const std::string& sqlString ) = 0;
        
        
        virtual void priv_read()
            {
                Context::EnsureStartStopReadingIsFiredRAII _raii1( this );
                this->clearContext();
                LG_SQLDB_D << "priv_read(top) path:" << this->getDirPath() << endl;

                addAllTables();
        
                LG_SQLDB_D << "priv_read(done) path:" << this->getDirPath() << endl;
            }

        fh_context
        SubCreateQV( fh_context c, fh_context md )
            {
                fh_context ret;

                LG_SQLDB_D << "SubCreateQV() " << endl;
                
                std::string qstr    = getStrSubCtx( md, "sql", "", true );
                std::string tabname = getStrSubCtx( md, "name", "" );

                if( !qstr.length() )
                {
                    fh_stringstream ss;
                    ss << "Can not create a query view with a zero length SQL Query."
                       << " URL::" << this->getURL()
                       << endl;
                    Throw_FerrisCreateSubContextFailed( tostr(ss), 0 );
                }
                
                try
                {
                    LG_SQLDB_D << "SubCreateQV() qstr:" << qstr
                                 << " tabname:" << tabname
                                 << endl;


                    ret = this->createSubContext( this, tabname, qstr );
                    this->addNewChild( ret );

                    LG_SQLDB_D << "SubCreateQV(2) qstr:" << qstr << endl;
                }
                catch( std::exception& e )
                {
                    Throw_FerrisCreateSubContextFailed( e.what(), 0 );
                }
                return ret;
            }

        fh_context
        SubCreateTable( fh_context c, fh_context md )
            {
                fh_context ret;
                try
                {
                    std::string qstr    = getStrSubCtx( md, "sql", "" );
                    std::string tabname = getStrSubCtx( md, "name", "" );
                    
                    LG_SQLDB_D << "SubCreateTable() qstr:" << qstr
                                 << " tabname:" << tabname
                                 << endl;

                    createSQLTable( tabname, qstr );

                    ChildContextType* child = new ChildContextType( this, tabname );
                    ret = child;
                    this->addNewChild( child );
                }
                catch( std::exception& e )
                {
                    Throw_FerrisCreateSubContextFailed( e.what(), 0 );
                }
                return ret;
            }


        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    _Base::createStateLessAttributes( true );
                    this->supplementStateLessAttributes( true );
                }
            }
        
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Subclasses should override
     * getRealStream()
     * populateKeyNames()
     */
    template < class ChildContextType >
    class CommonSQLContext
        :
        public StateLessEAHolder< ChildContextType, FakeInternalContext >
    {
        typedef CommonSQLContext _Self;
        typedef StateLessEAHolder< ChildContextType, FakeInternalContext > _Base;

    protected:

        typedef std::vector< std::string > PrimaryKeyNames_t;
        std::string        m_primaryKey;
        std::string        m_recommendedEA;
        std::string        m_tableQuerySQL;

        PrimaryKeyNames_t  m_primaryKeyNames;
        PrimaryKeyNames_t  m_keyNames;

        class RowMetaData 
        {
            bool   allowNulls;
            bool   ai;
            std::string ktype;

        public:
            RowMetaData( std::string _ktype = "", bool _ai = false, bool _allowNulls = true )
                :
                ktype( _ktype ),
                ai( _ai ),
                allowNulls( _allowNulls )
                {
                }

            bool AutoIncrement()
                {
                    return ai;
                }

            std::string KeyType()
                {
                    return ktype;
                }

            bool AllowNulls()
                {
                    return allowNulls;
                }
        };
        typedef std::map< std::string, RowMetaData > KeyNamesType_t;
        KeyNamesType_t KeyNamesType;

        /******************************/
        /******************************/
        /******************************/

        void
        priv_FillCreateSubContextSchemaParts( Context::CreateSubContextSchemaPart_t& m )
            {
                try
                {
                    PrimaryKeyNames_t& keyNames = getKeyNames();
                    
                    LG_SQLDB_D << "priv_FillCreateSubContextSchemaParts(A)" << endl;
                
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
                
                    LG_SQLDB_D << "priv_FillCreateSubContextSchemaParts(A1)" << endl;
                    fh_stringstream ss;

                    ss << "	<elementType name=\"tuple\">\n";
                    LG_SQLDB_D << "priv_FillCreateSubContextSchemaParts(A2) keyNames.size:"
                               << keyNames.size() << endl;
                    for( PrimaryKeyNames_t::iterator iter = keyNames.begin();
                         iter != keyNames.end(); ++iter )
                    {
                        LG_SQLDB_D << "priv_FillCreateSubContextSchemaParts(ALoop0)" << endl;
                        int maxlen = 0;
                        std::string xmltype = "string";
                        std::string ktype = KeyNamesType[*iter].KeyType();
                        std::string defaultVal = "";

                        LG_SQLDB_D << "priv_FillCreateSubContextSchemaParts(ALoop1)"
                             << " ktype:" << ktype
                             << endl;
                    
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
                        if( !KeyNamesType[*iter].AllowNulls() )
                            minOccur = 1;
                        if( KeyNamesType[*iter].AllowNulls() )
                            minOccur = 0;
                        
                        LG_SQLDB_D << "iter:" << *iter << endl;
                        LG_SQLDB_D << "ktype:" << ktype << endl;
                        LG_SQLDB_D << "maxlen:" << maxlen << endl;
                        LG_SQLDB_D << "isPrimary:" << isPrimary << endl;

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
                    
                    LG_SQLDB_D << "priv_FillCreateSubContextSchemaParts(A5)" << endl;
                
                    m["tuple"] = Context::SubContextCreator(
                        Context::SubContextCreator::Perform_t( this, &_Self::SubCreateTuple),
                        tostr(ss));
                    LG_SQLDB_D << "priv_FillCreateSubContextSchemaParts(A6)" << endl;
                }
                catch( std::exception& e )
                {
                }
            }

        /******************************/
        /******************************/
        /******************************/
        
        
//        virtual void DeterminePrimaryKey() = 0;
        
        virtual fh_iostream  getRealStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)        = 0;
        
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                LG_SQLDB_D << "getting I stream..." << endl;
                return getRealStream( m );
            }
            
        ferris_ios::openmode
        getSupportedOpenModes()
            {
                return ios::in | ios::binary; // | ios::out;
            }


        virtual void populateKeyNames( PrimaryKeyNames_t& ) = 0;
        PrimaryKeyNames_t& getKeyNames()
            {
                if( !m_keyNames.empty() )
                    return m_keyNames;

                populateKeyNames( m_keyNames );
                return m_keyNames;
            }
        
        
        void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    Context::tryAddStateLessAttribute( "primary-key",
                                                       _Self::SL_getPrimaryKeyStream,
                                                       FXD_PRIMARY_KEY_REAL );
                    _Base::createStateLessAttributes( true );
                    this->supplementStateLessAttributes( true );
                }
            }

        virtual std::string getRecommendedEA()
            {
                return m_recommendedEA;
            }

        bool ImplicitlyQuoteColumn( std::string s )
            {
                std::string ktype = KeyNamesType[s].KeyType();
                if( starts_with( ktype, "timestamp") )
                {
                    return 0;
                }
                return 1;
            }

        virtual void executeInsert( const std::string& tableName,
                                    const std::string& sqlString ) = 0;
        virtual fh_context addContextForResultingRow( const std::string& tableName,
                                                      const std::string& sqlString ) = 0;
        

        /* FIXME */
        fh_context
        SubCreateTuple( fh_context c, fh_context md )
            {
                fh_context ret;
                try
                {
                    bool v = true;
                    fh_stringstream ss;
                    ss << "INSERT INTO " << this->getDirName() << " " << endl;
                    

                    ss << "(";
                    v = true;
                    for( PrimaryKeyNames_t::iterator iter = getKeyNames().begin();
                         iter != getKeyNames().end(); ++iter )
                    {
                        if(!v)
                            ss << ",";
                        v = false;

                        ss << *iter;
                    }
                    ss << ")" << endl
                       << "VALUES " << endl
                       << " (";
                    
                    v = true;
                    for( PrimaryKeyNames_t::iterator iter = getKeyNames().begin();
                         iter != getKeyNames().end(); ++iter )
                    {
                        if(!v)
                            ss << ",";
                        v = false;

                        std::string s = getStrSubCtx( md, *iter, "" );
                        if( ImplicitlyQuoteColumn( *iter ) )
                        {
                            ss << " '" << s << "' ";
                        }
                        else
                        {
                            ss << s << " ";
                        }
                    }
                    
                    ss << ")" << endl;

                    
                    executeInsert( this->getDirName(), tostr(ss) );
                    
                    
                    {
                        fh_stringstream ss;
                        
//                        Query q = getConnection( this )->query();
                        ss << "select * from " << this->getDirName()
                           << " where ";
                        bool v=true;
                        for( PrimaryKeyNames_t::iterator iter = m_primaryKeyNames.begin();
                             iter != m_primaryKeyNames.end(); ++iter )
                        {
                            if( v ) v = false;
                            else    ss << " and ";
                            
                            ss << " " << *iter << " = '"
                               << getStrSubCtx( md, *iter, "" ) << "' ";
                        }
                        LG_SQLDB_D << "Query:" << tostr(ss) << endl;

                        ret = addContextForResultingRow( this->getDirName(), tostr(ss) );
                        
                    }
                }
                catch( std::exception& e )
                {
                    Throw_FerrisCreateSubContextFailed( e.what(), 0 );
                }
                return ret;
            }


        
        
        
        /******************************/
        /******************************/
        /******************************/

    public:

        CommonSQLContext( const fh_context& parent, const std::string& rdn, std::string QuSQL = "" )
            : 
            m_tableQuerySQL( QuSQL )
            {
                this->setContext( GetImpl(parent), this->monsterName(rdn) );
//                DeterminePrimaryKey();
//                createStateLessAttributes();
            }
        
        virtual ~CommonSQLContext()
            {
            }

        std::string getTableQuerySQL()
            {
                return m_tableQuerySQL;
            }
        
        virtual std::string getPrimaryKey()
            {
                return m_primaryKey;
            }

        PrimaryKeyNames_t& getPrimaryKeyNames()
            {
                return m_primaryKeyNames;
            }
        
        static fh_istream SL_getPrimaryKeyStream( Context* c,
                                                  const std::string& rdn,
                                                  EA_Atom* atom )
            {
                fh_stringstream ss;
                if( CommonSQLContext* cc = dynamic_cast<CommonSQLContext*>(c))
                {
                    ss << cc->getPrimaryKey();
                }
                return ss;
            }

        XSDBasic_t getSchemaType( const std::string& eaname )
            {
                typename KeyNamesType_t::iterator iter = KeyNamesType.find( eaname );
                if( iter != KeyNamesType.end() )
                {
                    string ktype = iter->second.KeyType();
//                    cerr << "KeyNamesType eaname:" << eaname << " kt:" << ktype << endl;

                    if( ktype == "int" || ktype == "integer" )
                        return XSD_BASIC_INT;
                    if( ktype == "bool" || ktype == "boolean" )
                        return XSD_BASIC_BOOL;
                    if( ktype == "date" )
                        return FXD_UNIXEPOCH_STRING;
                }
                return XSD_BASIC_STRING;
            }
        
    };

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Subclasses should override
     * executeUpdate()
     * getTableName()
     */
    template < class ChildContextType, class ParentChildContextType >
    class CommonSQLDBTupleContext
        :
        public StateLessEAHolder< ChildContextType, leafContext >
    {
        typedef CommonSQLDBTupleContext                            _Self;
        typedef StateLessEAHolder< ChildContextType, leafContext > _Base;

        ParentChildContextType* getCommonSQLContext()
            {
                if( this->isParentBound() )
                {
                    fh_context pc = this->getParent();
                    if( ParentChildContextType* p
                        = dynamic_cast< ParentChildContextType* >( GetImpl(pc)))
                    {
                        return p;
                    }
                }
                return 0;
            }
        
        
    protected:

        typedef std::vector< std::string > PrimaryKeyNames_t;
        typedef std::map< std::string, std::string > kvm_t;
        kvm_t kvm;

        virtual void executeUpdate( const std::string& sqlString ) = 0;
        virtual std::string getTableName() = 0;
        
    public:

        CommonSQLDBTupleContext( const fh_context& parent,
                                 const std::string& rdn )
            {
                this->setContext( GetImpl(parent), this->monsterName(rdn) );
            }
        
        virtual ~CommonSQLDBTupleContext()
            {}
        
        fh_iostream getValueStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
//                 cerr << "getValueStream() rdn:" << rdn << " kvm.sz:" << kvm.size() << endl;
//                 for( kvm_t::const_iterator ki = kvm.begin(); ki != kvm.end(); ++ki )
//                 {
//                     cerr << "   getValueStream() ki.first:" << ki->first << " second:" << ki->second << endl;
//                 }
                
                fh_stringstream ss;
                ss << kvm[ rdn ];
                return ss;
            }

        static fh_iostream SL_getValueStream( CommonSQLDBTupleContext* c, const std::string& rdn, EA_Atom* atom )
            {
                LG_SQLDB_D << "SL_getValueStream rdn:" << rdn << endl;
                return c->getValueStream( c, rdn, atom );
            }
        static void SL_setValueStream( CommonSQLDBTupleContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
            {
                return c->setValueStream( c, rdn, atom, ss );
            }
        
                
        void setValueStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
            {
                std::string s;
                getline( ss, s );
                ParentChildContextType* sqlc = getCommonSQLContext();

                LG_SQLDB_D << "SqlPlusTupleContext::setValueStream() newval:" << s << endl;
    
                if( !sqlc )
                {
                    Throw_CanNotGetStream("Internal error, malformed sql context tree", 0 );
                }
            
                /*
                 * Special handling of user based fake tables.
                 */
                if( sqlc->getTableQuerySQL().length() )
                {
                    Throw_CanNotGetStream("Can not update fake tables", 0 );
                }
        

//                const SqlDetails& d = getSqlDetails( this );
//                Query query = getConnection( this )->query();
                fh_stringstream qstr;
//                 qstr << " update " << d.table
//                      << " set " << rdn
//                      << " = '" << s << "' "
//                      << " where ";
                qstr << " update " << getTableName()
                     << " set " << rdn
                     << " = '" << s << "' "
                     << " where ";
                bool v = true;
                for( PrimaryKeyNames_t::iterator iter = getPrimaryKeyNames().begin();
                     iter != getPrimaryKeyNames().end(); ++iter )
                {
                    if( !v )
                        qstr << " and ";
                    
                    qstr << " " << *iter << " = '" << kvm[*iter] << "' ";
                    v = false;
                }
        
                LG_SQLDB_D << "pk:" << getPrimaryKey() << endl;
                LG_SQLDB_D << "update query:" << tostr(qstr) << endl;

                //(std::ostream&)query << tostr(qstr); 
                //query.execute();
                executeUpdate( tostr( qstr ) );
                kvm[ rdn ] = s;
            }

        std::string getPrimaryKey()
            {
                if( getCommonSQLContext() )
                {
                    return getCommonSQLContext()->getPrimaryKey();
                }
                return "";
            }
        
        PrimaryKeyNames_t& getPrimaryKeyNames()
            {
                static PrimaryKeyNames_t ret;
        
                if( getCommonSQLContext() )
                {
                    return getCommonSQLContext()->getPrimaryKeyNames();
                }
                return ret;
            }
            
        PrimaryKeyNames_t& getKeyNames()
            {
                static PrimaryKeyNames_t ret;
        
                if( getCommonSQLContext() )
                {
                    return getCommonSQLContext()->getKeyNames();
                }
                return ret;
            }

        virtual std::string getRecommendedEA()
            {
                return this->getParent()->getRecommendedEA();
            }

        ferris_ios::openmode
        getSupportedOpenModes()
            {
                return ios::in | ios::binary; // | ios::out;
            }

        bool
        getHasSubContextsGuess()
            {
                return false;
            }
        
        fh_istream priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ss;
                ss << "<context ";

                for( kvm_t::iterator iter = kvm.begin();
                     iter != kvm.end(); ++iter )
                {
                    ss << " " << iter->first <<  "=\""
                       << XML::escapeToXMLAttr(iter->second) << "\" ";
                }

                /* this recurses forever, some of the EA rely on getIstream()
                 * eg. md5 */
//         AttributeNames_t an = getAttributeNames();
//         for( AttributeNames_t::iterator iter = an.begin();
//              iter != an.end(); ++iter )
//         {
//             string s = getStrAttr( this, *iter, "" );
//             ss << " " << *iter <<  "=\"" << s << "\" ";
//         }
        
                ss << " /> ";
                return ss;
            }
    
        static fh_istream SL_getTuplePrimaryKeyStream( Context* c,
                                                       const std::string& rdn,
                                                       EA_Atom* atom )
            {
                fh_stringstream ss;
                if( _Self* p = dynamic_cast< _Self* >( c ))
                {
                    ss << p->getPrimaryKey();
                }
                return ss;
            }

        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    Context::tryAddStateLessAttribute( "primary-key",
                                                       _Self::SL_getTuplePrimaryKeyStream,
                                                       FXD_PRIMARY_KEY_VIRTUAL );
                    _Base::createStateLessAttributes( true );
                    this->supplementStateLessAttributes( true );
                }
            }
        
    };
    



};
#endif
