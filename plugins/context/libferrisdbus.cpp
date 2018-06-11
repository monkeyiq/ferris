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

    $Id: libferrisdbus.cpp,v 1.4 2010/09/24 21:31:34 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <config.h>

#include <FerrisContextPlugin.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisQt_private.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/Trimming.hh>
#include <Ferris/General.hh>
#include <Ferris/Cache.hh>
#include <Ferris/Runner.hh>
#include <Ferris/Medallion.hh>
#include <Ferris/SyncDelayer.hh>
#include <Ferris/MetadataServer_private.hh>

#include "DBusGlue/org_freedesktop_DBus.h"
#include "DBusGlue/org_freedesktop_DBus.cpp"
#include "DBusGlue/org_freedesktop_DBus_Introspectable.h"
#include "DBusGlue/org_freedesktop_DBus_Introspectable.cpp"


using namespace std;

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    

    const char* DBUS_SERVER_NAME = "org.freedesktop.DBus";
    const char* DBUS_SERVER_PATH = "/org/freedesktop/DBus";
    
    string appendPath( const std::string& p1, const std::string& p2 )
    {
        cerr << "appendPath() p1:" << p1 << " p2:" << p2 << endl;
        
        stringstream ss;
        ss << p1;
        if( !ends_with( p1, "/" ) )
            ss << "/";
        ss << p2;

        string ret = ss.str();
        return ret;
    }
    

    struct argData
    {
        bool isInput;
        bool isOutput;
        string type;
        string name;
        argData()
            :
            isInput(false),
            isOutput(false),
            type(""),
            name("")
            {
            }
    };
                
    typedef list< argData > argList_t;
    typedef std::list< DOMElement* > delist_t;

    fh_ostream serialize( fh_ostream oss, QList<QVariant>& rl )
    {
        for( QList<QVariant>::iterator ri = rl.begin(); ri != rl.end(); ++ri )
        {
            oss << tostr( *ri );
        }

        return oss;
    }
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_CTXPLUGIN dbusCalledObjectContext
        :
        public StateLessEAHolder< dbusCalledObjectContext, leafContext >
    {
        typedef dbusCalledObjectContext                 _Self;
        typedef StateLessEAHolder< _Self, leafContext > _Base;

        string m_methodName;
        argList_t m_argSpec;
        stringlist_t m_args;
        
    public:

        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception);
        

        std::string
        priv_getRecommendedEA()
            {
                static string rea = "name,content";
                return rea;
            }
        
        dbusCalledObjectContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
                LG_DBUS_D << "dbusCalledObjectContext() rdn:" << rdn << endl;
            }

        void setMethodName( const std::string& s )
            {
                m_methodName = s;
            }

        void setArgList( argList_t& a )
            {
                m_argSpec = a;
            }
        
        void setArguments( const stringlist_t& args )
            {
                m_args = args;
            }
        
        QDBusConnection& getConnection();
        string getDBusServiceName();
        
    };


    class FERRISEXP_CTXPLUGIN dbusMethodArgumentDetailsContext
        :
        public StateLessEAHolder< dbusMethodArgumentDetailsContext, leafContext >
    {
        typedef dbusMethodArgumentDetailsContext        _Self;
        typedef StateLessEAHolder< _Self, leafContext > _Base;

        argData m_argData;
        
    public:

        static fh_stringstream
        SL_getType( dbusMethodArgumentDetailsContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->getData().type;
                return ss;
            }

        static fh_stringstream
        SL_getDescription( dbusMethodArgumentDetailsContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->getData().name;
                return ss;
            }

        static fh_stringstream
        SL_getIsInput( dbusMethodArgumentDetailsContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->getData().isInput;
                return ss;
            }

        static fh_stringstream
        SL_getIsOutput( dbusMethodArgumentDetailsContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->getData().isOutput;
                return ss;
            }

        std::string
        priv_getRecommendedEA()
            {
                static string rea = "name,type,description,is-input,is-output";
                return rea;
            }
        
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
#define SLEA  tryAddStateLessAttribute

                    SLEA( "type",        SL_getType,        XSD_BASIC_STRING );
                    SLEA( "description", SL_getDescription, XSD_BASIC_STRING );
                    SLEA( "is-input",    SL_getIsInput,  XSD_BASIC_BOOL );
                    SLEA( "is-output",   SL_getIsOutput, XSD_BASIC_BOOL );

#undef SLEA
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        
        dbusMethodArgumentDetailsContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
                LG_DBUS_D << "dbusMethodArgumentDetailsContext() rdn:" << rdn << endl;
            }
        argList_t getArgList();
        argData getData()
            {
                return m_argData;
            }
        void setData( argData& d )
            {
                m_argData = d;
            }
        
    };
    

    class FERRISEXP_CTXPLUGIN dbusMethodDetailsContext
        :
        public StateLessEAHolder< dbusMethodDetailsContext, FakeInternalContext >
    {
        typedef dbusMethodDetailsContext                        _Self;
        typedef StateLessEAHolder< _Self, FakeInternalContext > _Base;

    protected:

        void
        priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    fh_context dc = 0;

                    cerr << "priv_read(method details)" << endl;

                    {
                        FakeInternalContext* parent = 0;
                        parent = priv_ensureSubContext( "arguments", parent );
                        
                        argList_t a = getArgList();
                        int i = 0;
                        for( argList_t::iterator ai = a.begin(); ai != a.end(); ++ai, ++i )
                        {
                            string rdn = tostr(i);
                            dbusMethodArgumentDetailsContext* t = new dbusMethodArgumentDetailsContext( parent, rdn );
                            dc = t;
                            t->setData( *ai );
                            parent->addNewChild( dc );
                        }
                    }

                    
                }
            }
        
    public:

        dbusMethodDetailsContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
                LG_DBUS_D << "dbusMethodDetailsContext() rdn:" << rdn << endl;
            }
        argList_t getArgList();
        
    };
    

    class FERRISEXP_CTXPLUGIN dbusInterfaceContext
        :
        public StateLessEAHolder< dbusInterfaceContext, FakeInternalContext >
    {
        typedef dbusInterfaceContext                             _Self;
        typedef StateLessEAHolder< _Self, FakeInternalContext >  _Base;

        typedef map< string, argList_t > m_methods_t;
        m_methods_t m_methods;
        
    protected:

        string getDPath();

        
        virtual bool priv_supportsShortCutLoading()  { return true; }
        //
        // Short cut loading each dir unless absolutely needed.
        //
        fh_context
        priv_getSubContext( const string& rdn )
            throw( NoSuchSubContext )
            {
//                cerr << "priv_getSubContext() rdn:" << rdn << endl;
                
                try
                {
                    LG_DBUS_D << "priv_getSubContext() p:" << getDirPath()
                                << " rdn:" << rdn
                                << endl;

                    Items_t::iterator isSubContextBoundCache;
                    if( priv_isSubContextBound( rdn, isSubContextBoundCache ) )
                    {
                        LG_DBUS_D << "priv_getSubContext(bound already) p:" << getDirPath()
                                    << " rdn:" << rdn
                                    << endl;
                        return *isSubContextBoundCache;
                    }

                    try
                    {
                        return _Base::priv_getSubContext( rdn );
                    }
                    catch( exception& e )
                    {
                        if( contains( rdn, "[" ) || contains( rdn, "(" ) )
                        {
                            cerr << "have a method call!" << endl;
                            char closingBr = ']';
                            int pos = rdn.find('[');
                            if( pos == string::npos )
                            {
                                pos = rdn.find('(');
                                closingBr = ')';
                            }
                            int epos = rdn.rfind( closingBr );
                            if( epos == string::npos )
                            {
                                stringstream ss;
                                ss << "Unterminated method arguments:" << rdn << endl;
                                Throw_NoSuchSubContext( tostr(ss), this );
                            }

                            string mname = rdn.substr( 0, pos );
                            cerr << "method name:" << mname << endl;

                            m_methods_t::iterator methiter = m_methods.find( mname );
                            if( methiter == m_methods.end() )
                            {
                                stringstream ss;
                                cerr << "m_methods.size:" << m_methods.size() << endl;
                                ss << "Unkown method:" << rdn << endl;
                                Throw_NoSuchSubContext( tostr(ss), this );
                            }
                            
                            argList_t a = methiter->second;
                            cerr << "child rdn:" << rdn << endl;


                            string argString = rdn.substr( pos+1, epos-pos-1 );
                            cerr << "argString:" << argString << endl;
                            stringlist_t args;
                            Util::parseSeperatedList( argString, args, ',' );
                            // FIXME actually use args

                            cerr << "Creating new child for rdn:" << rdn << endl;

                            dbusCalledObjectContext* cc = 0;
                            cc = priv_ensureSubContext( rdn, cc );
                            cc->setMethodName( mname );
                            cc->setArgList( a );
                            cc->setArguments( args );
                            return cc;
                        }
                    }
                }
                catch( exception& e )
                {
                    throw;
                }
                
                fh_stringstream ss;
                ss << "NoSuchSubContext:" << rdn;
                Throw_NoSuchSubContext( tostr(ss), this );
            }
        
        
        void ensureMethodChild( const std::string& ifname,
                                const std::string& mname,
                                DOMElement* meth )
            {
                fh_context dc = 0;

                stringstream rdnss;
                rdnss << mname << "(";
                argList_t argList;

                delist_t delist = XML::getAllChildrenElements( meth, "arg", true );
                delist_t::iterator e = delist.end();
                bool v = true;
                for( delist_t::iterator iter = delist.begin(); iter != e; ++iter )
                {
                    argData a;
                    a.name = ::Ferris::getAttribute( *iter, "name" );
                    a.type = ::Ferris::getAttribute( *iter, "type" );
                    string dir = ::Ferris::getAttribute( *iter, "direction" );
                    if( contains( dir, "in" ) )
                        a.isInput = true;
                    if( contains( dir, "out" ) )
                        a.isOutput = true;
                    argList.push_back( a );
                    
                    
                    if( v ) v = false;
                    else rdnss << ",";
                    rdnss << a.type;
                }
                rdnss << ")";

                m_methods[ mname ] = argList;
                cerr << "rdnss:" << rdnss.str() << endl;

//                 dbusMethodDetailsContext* t = new dbusMethodDetailsContext( this, mname );
//                 dc = t;
//                 Insert( GetImpl(dc) );

                dbusMethodDetailsContext* cc = 0;
                cc = priv_ensureSubContext( mname, cc );
                
                
//     <method name="Get">
//       <arg direction="in" type="s" name="interface_name"/>
//       <arg direction="in" type="s" name="property_name"/>
//       <arg direction="out" type="v" name="value"/>
//     </method>
                
            }
        

        void
        priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    fh_context dc = 0;

                    cerr << "dobject getDBusServiceName:" << getDBusServiceName() << endl;
                    cerr << "dobject dpath:" << getDPath() << endl;
                    fdoDBusIntrospectable* dobj = new fdoDBusIntrospectable(
                        getDBusServiceName().c_str(), getDPath().c_str(), 
                        getConnection(), QCoreApplication::instance() );

                    string xmldesc = tostr(dobj->Introspect());
                    LG_DBUS_D << "dobject xmldesc:" << xmldesc << endl;
                    cerr << "dobject xmldesc:" << xmldesc << endl;
                    
                    try
                    {
                        fh_domdoc dom = Factory::StringToDOM( xmldesc );
                        DOMElement* de = dom->getDocumentElement();

                        delist_t delist;

                        // get all methods
                        {
                            delist_t iflist = XML::getAllChildrenElements( de, "interface", false );
                            delist_t::iterator de = iflist.end();
//                            cerr << "Interface count:" << distance( iflist.begin(), iflist.end() ) << endl;
                            
                            for( delist_t::iterator ifiter = iflist.begin(); ifiter != de; ++ifiter )
                            {
                                DOMElement* iface = *ifiter;
                                string iname = ::Ferris::getAttribute( iface, "name" );
//                                cerr << "Interface:" << iname << endl;

                                if( iname != getDirName() )
                                    continue;
                                
                                delist_t mlist = XML::getAllChildrenElements( iface, "method", false );
                                delist_t::iterator me = mlist.end();
                                for( delist_t::iterator miter = mlist.begin(); miter != me; ++miter )
                                {
                                    string mname = ::Ferris::getAttribute( *miter, "name" );

                                    LG_DBUS_D << "iname:" << iname << " mname:" << mname << endl;
                                    DOMElement* meth = *miter;

                                    ensureMethodChild( iname, mname, meth );
                                }
                            }
                            
                        }
                    }
                    catch( exception& e )
                    {
                        throw;
                    }
                }
            }

        
        
    public:

        dbusInterfaceContext( Context* parent, const std::string& rdn )
             :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
                LG_DBUS_D << "dbusInterfaceContext() rdn:" << rdn << endl;
            }

        QDBusConnection& getConnection();
        string getDBusServiceName();

        argList_t getArgList( const std::string& mname )
            {
                m_methods_t::iterator mi = m_methods.find( mname );
                if( mi != m_methods.end() )
                    return mi->second;
                
                stringstream ss;
                ss << "No method information for method:" << mname << endl;
                Throw_NoSuchObject( ss.str(), this );
            }
        
    };
    

    
    class FERRISEXP_CTXPLUGIN dbusObjectContext
        :
        public StateLessEAHolder< dbusObjectContext, FakeInternalContext >
    {
        typedef dbusObjectContext                               _Self;
        typedef StateLessEAHolder< _Self, FakeInternalContext > _Base;

        string m_dpath;
        string m_dname;
        bool m_haveMethods;

    protected:


                    
// <node>
//   <node name="Properties"/>
// </node>

// <node name="/org/freedesktop/DBus/Examples/Properties">
//   <interface name="org.freedesktop.DBus.Introspectable">
//     <method name="Introspect">
//       <arg direction="out" type="s" name="data"/>
//     </method>
//   </interface>
//   <interface name="org.freedesktop.DBus.Properties">
//     <method name="Get">
//       <arg direction="in" type="s" name="interface_name"/>
//       <arg direction="in" type="s" name="property_name"/>
//       <arg direction="out" type="v" name="value"/>
//     </method>
//     <method name="Set">
//       <arg direction="in" type="s" name="interface_name"/>
//       <arg direction="in" type="s" name="property_name"/>
//       <arg direction="in" type="v" name="value"/>
//     </method>
//   </interface>
//   <interface name="org.freedesktop.DBus.PropsDemo">
//     <property name="Version" type="i" access="read"/>
//     <property name="Message" type="s" access="readwrite"/>
//     <signal name="MessageChanged">
//       <arg type="s" name="message"/>
//     </signal>
//   </interface>
// </node>
        void
        priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    fh_context dc = 0;

                    cerr << "dobject getDBusServiceName:" << getDBusServiceName() << endl;
                    cerr << "dobject dpath:" << m_dpath << endl;
                    fdoDBusIntrospectable* dobj = new fdoDBusIntrospectable(
                        getDBusServiceName().c_str(), m_dpath.c_str(), 
                        getConnection(), QCoreApplication::instance() );

                    string xmldesc = tostr(dobj->Introspect());
                    LG_DBUS_D << "dobject xmldesc:" << xmldesc << endl;
                    cerr << "dobject xmldesc:" << xmldesc << endl;
                    
                    try
                    {
                        fh_domdoc dom = Factory::StringToDOM( xmldesc );
                        DOMElement* de = dom->getDocumentElement();

                        delist_t delist;

                        // get all the children
                        {
                            delist_t delist = XML::getAllChildrenElements( de, "node", false );
                            delist_t::iterator e = delist.end();
                            for( delist_t::iterator iter = delist.begin(); iter != e; ++iter )
                            {
                                string rdn = ::Ferris::getAttribute( *iter, "name" );
                                LG_DBUS_D << "child rdn:" << rdn << endl;

                                string path = appendPath( m_dpath, rdn );
                                LG_DBUS_D << "child path:" << path << endl;
//                                 dbusObjectContext* t = new dbusObjectContext( this, rdn );
//                                 dc = t;
//                                 t->setDPath( path );
//                                 Insert( GetImpl(dc) );

                                dbusObjectContext* cc = 0;
                                cc = priv_ensureSubContext( rdn, cc );
                                cc->setDPath( path );
                                
                            }
                        }

                        // get all methods
                        {
                            delist_t iflist = XML::getAllChildrenElements( de, "interface", false );
                            delist_t::iterator de = iflist.end();
//                            cerr << "Interface count:" << distance( iflist.begin(), iflist.end() ) << endl;
                            
                            for( delist_t::iterator ifiter = iflist.begin(); ifiter != de; ++ifiter )
                            {
                                DOMElement* iface = *ifiter;
                                string iname = ::Ferris::getAttribute( iface, "name" );
                                LG_DBUS_D << "interface child iname:" << iname << endl;

                                string rdn = iname;
                                dbusInterfaceContext* cc = 0;
                                cc = priv_ensureSubContext( rdn, cc );
                            }
                            
                        }
                        
                    }
                    catch( exception& e )
                    {
                        throw;
                    }
                }
            }
        
        
    public:

        dbusObjectContext( Context* parent, const std::string& rdn )
             :
            _Base( parent, rdn ),
            m_dpath( "/" ),
            m_haveMethods( false )
            {
                createStateLessAttributes( true );
                LG_DBUS_D << "dbusObjectContext() rdn:" << rdn << endl;
            }

        void setDPath( const std::string& s )
            {
                m_dpath = s;
            }
        string getDPath()
            {
                return m_dpath;
            }

        
        QDBusConnection& getConnection();

        string getDBusServiceName()
            {
                if( m_dname.empty() )
                {
                    if( dbusObjectContext* pc = dynamic_cast<dbusObjectContext*>( getParent() ) )
                        return pc->getDBusServiceName();
                    return "";
                }
                
                return m_dname;
            }
        void getDBusServiceName( const std::string& s )
            {
                m_dname = s;
            }
    };
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    

    class FERRISEXP_CTXPLUGIN dbusConnectionContext
        :
        public StateLessEAHolder< dbusConnectionContext, FakeInternalContext >
    {
        typedef dbusConnectionContext                           _Self;
        typedef StateLessEAHolder< _Self, FakeInternalContext > _Base;

        QDBusConnection m_conn;
        
    protected:

        void priv_read()
        {
            ensureQApplication();
            staticDirContentsRAII _raii1( this );

            if( empty() )
            {
                fh_context dc = 0;

                fdoDBus* dobj = new fdoDBus( DBUS_SERVER_NAME, DBUS_SERVER_PATH,
                                             m_conn, QCoreApplication::instance() );

                QStringList sl = dobj->ListNames();
//                copy( sl.begin(), sl.end(), ostream_iterator<string>( cerr, "\n" ));
                for( QStringList::iterator si = sl.begin(); si != sl.end(); ++si )
                {
                    string rdn = tostr(*si);
                    cerr << "rdn:" << rdn << endl;
                    if( !starts_with( rdn, ":" ) )
                    {
                        dbusObjectContext* cc = 0;
                        cc = priv_ensureSubContext( rdn, cc );
                        cc->getDBusServiceName( rdn );
                    }
                }
                
            }
        }
        
    public:

        dbusConnectionContext( Context* parent, const std::string& rdn, const QDBusConnection& conn )
            :
            _Base( parent, rdn ),
            m_conn( conn )
            {
                createStateLessAttributes( true );
                LG_DBUS_D << "dbusConnectionContext() rdn:" << rdn << endl;
            }

        QDBusConnection& getConnection()
            {
                return m_conn;
            }
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    
    class FERRISEXP_CTXPLUGIN dbusServerContext
        :
        public StateLessEAHolder< dbusServerContext, FakeInternalContext >
    {
        typedef dbusServerContext                               _Self;
        typedef StateLessEAHolder< _Self, FakeInternalContext > _Base;

    protected:

        void
        priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    fh_context dc = 0;
                    

                    {
                        dc = new dbusConnectionContext( this, "session", QDBusConnection::sessionBus() );
                        Insert( GetImpl(dc) );
                    }

                    {
                        dc = new dbusConnectionContext( this, "system", QDBusConnection::systemBus() );
                        Insert( GetImpl(dc) );
                    }
                }
            }
        
    public:

        dbusServerContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
                LG_DBUS_D << "dbusServerContext() rdn:" << rdn << endl;
            }
        
    };
    
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    class FERRISEXP_CTXPLUGIN dbusRootContext
        :
        public networkRootContext< dbusServerContext >
    {
        typedef dbusRootContext                         _Self;
        typedef networkRootContext< dbusServerContext > _Base;

    protected:

        virtual std::string TryToCheckServerExists( const std::string& rdn )
            {
                LG_DBUS_D << "TryToCheckServerExists() rdn:" << rdn << endl;
                return "";
            }
        
    public:

        dbusRootContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn, true )
            {
                LG_DBUS_D << "dbusRootContext()" << endl;
                createStateLessAttributes( true );
            }
        virtual ~dbusRootContext()
            {
                LG_DBUS_D << "~dbusRootContext()" << endl;
            }
        virtual std::string priv_getRecommendedEA()
            {
                return "name";
            }
        virtual std::string getRecommendedEA()
            {
                return "name";
            }
        

        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
        dbusRootContext* priv_CreateContext( Context* parent, string rdn )
            {
                dbusRootContext* ret = new dbusRootContext();
                ret->setContext( parent, rdn );
                return ret;
            }
    };





    QDBusConnection&
    dbusObjectContext::getConnection()
    {
        dbusConnectionContext* cc = 0;
        cc = getFirstParentOfContextClass<>( cc );
        return cc->getConnection();
    }
    
    QDBusConnection&
    dbusCalledObjectContext::getConnection()
    {
        dbusConnectionContext* cc = 0;
        cc = getFirstParentOfContextClass<>( cc );
        return cc->getConnection();
    }
    string dbusCalledObjectContext::getDBusServiceName()
    {
        dbusObjectContext* cc = 0;
        cc = getFirstParentOfContextClass<>( cc );
        return cc->getDBusServiceName();
    }

    argList_t dbusMethodArgumentDetailsContext::getArgList()
    {
        dbusMethodDetailsContext* cc = 0;
        cc = getFirstParentOfContextClass<>( cc );
        return cc->getArgList();
    }
    
    
    argList_t dbusMethodDetailsContext::getArgList()
    {
        dbusInterfaceContext* cc = 0;
        cc = getFirstParentOfContextClass<>( cc );
        return cc->getArgList( getDirName() );
    }
    
    



    fh_istream
    dbusCalledObjectContext::priv_getIStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               std::exception)
    {
        fh_stringstream ret;
        LG_DBUS_D << "priv_getIStream() url:" << getURL() << endl;

                
//     const char* DBUS_SERVER_NAME = "org.freedesktop.DBus";
//     const char* DBUS_SERVER_PATH = "/org/freedesktop/DBus";
//                string iname = "org.freedesktop.DBus.EchoDemo";
        string dpath = "/org/gnome/Banshee/Player";
// FIXME
        string iname = getParent()->getDirName();
        {
            dbusConnectionContext* cc = 0;
            cc = getFirstParentOfContextClass<>( cc );
            Context* end = dynamic_cast<Context*>(cc);
            Context* iter = getParent();
            stringlist_t sl;
            cerr << "XXX end:" << end->getDirPath() << endl;
            cerr << "XXX iter:" << iter->getDirPath() << endl;
            
            for( iter = iter->getParent();
                 iter->isParentBound() && iter->getParent() != end; iter = iter->getParent() )
            {
                sl.push_front( iter->getDirName() );
            }
            dpath = string("/") + Util::createSeperatedList( sl, '/' );
        }
                
                
        cerr << "methodname:" << m_methodName << endl;
        cerr << " path:" << getDirPath() << endl;
        cerr << "dpath:" << dpath << endl;
        cerr << endl;
                
//                CallMessage call( DBUS_SERVER_NAME, DBUS_SERVER_PATH, DBUS_SERVER_NAME, "ListNames" );
        // CallMessage( const char* dest, const char* path, const char* iface, const char* method );
//                 CallMessage call( "org.freedesktop.DBus.Examples.Echo",
//                                   "/org/freedesktop/DBus/Examples/Echo",
//                                   "org.freedesktop.DBus.EchoDemo",
//                                   m_methodName.c_str() );

        QDBusMessage call = QDBusMessage::createMethodCall( getDBusServiceName().c_str(),
                          dpath.c_str(), iname.c_str(), m_methodName.c_str() );
        QList<QVariant> call_arguments;

        if( !m_args.empty() )
        {
            cerr << "have arguments! sz:" << m_args.size() << endl;
                    
            stringlist_t::iterator argsITER = m_args.begin();
            stringlist_t::iterator argsEND  = m_args.end();
            argList_t::iterator specITER = m_argSpec.begin();
            argList_t::iterator specEND  = m_argSpec.end();
                    
            for( ; argsITER != argsEND; ++argsITER )
            {
                while( !specITER->isInput && specITER != specEND )
                {
                    ++specITER;
                }
                if( specITER == specEND )
                {
                    stringstream ss;
                    ss << "Given arguments do not match required arguments.";
                    Throw_NoSuchObject( ss.str(), 0 );
                }

                string type = specITER->type;
                string ARG = *argsITER;
                cerr << "ARG:" << ARG << endl;
                cerr << "type:" << type << endl;

                
                if( type == "v" )
                {
                    call_arguments << QString(ARG.c_str());
                }
                else if( type == "b" )
                {
                    call_arguments << isTrue(ARG);
                }
                else
                {
                    call_arguments << ARG.c_str();
                }
            }
        }

        call.setArguments( call_arguments );
        QDBusMessage rc = QDBusConnection::sessionBus().call( call, QDBus::BlockWithGui );
        QList<QVariant> rl = rc.arguments ();
        serialize( ret, rl );
                
        return ret;
    }

    QDBusConnection& dbusInterfaceContext::getConnection()
    {
        dbusConnectionContext* cc = 0;
        cc = getFirstParentOfContextClass<>( cc );
        return cc->getConnection();
    }
    string dbusInterfaceContext::getDBusServiceName()
    {
        dbusObjectContext* cc = 0;
        cc = getFirstParentOfContextClass<>( cc );
        return cc->getDBusServiceName();
    }
    
    string dbusInterfaceContext::getDPath()
    {
        dbusObjectContext* cc = 0;
        cc = getFirstParentOfContextClass<>( cc );
        return cc->getDPath();
    }

    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    
    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                static dbusRootContext c;
                fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
                return ret;
            }
            catch( exception& e )
            {
                LG_CTX_ER << "Brew() e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

};
