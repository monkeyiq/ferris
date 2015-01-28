/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001-2003 Ben Martin

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

    $Id: libferrisdbxml.cpp,v 1.5 2010/09/24 21:31:34 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "db.h"
#include "db_cxx.h"

#include <Ferris/DublinCore.hh>
#include <Resolver_private.hh>
#include <libferrisxmlshared.hh>


#include "dbxmlwrappers.hh"
#include "dbxml/DbXml.hpp"


#include <list>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

using namespace std;
using namespace DbXml;
// using namespace XERCES_CPP_NAMESPACE;
// using namespace XALAN_CPP_NAMESPACE;

namespace Ferris
{

    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    


    /**
     * This class handles the directory at top level of a .dbxml file.
     * This is very much like the XmlContainer level in the dbxml API.
     *
     * We create a DBXMLDocumentContext object for each document in the container.
     */
    class FERRISEXP_CTXPLUGIN DBXMLRootContext
        :
        public FakeInternalContext
    {
        typedef DBXMLRootContext     _Self;
        typedef FakeInternalContext  _Base;
        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );

        myXmlContainer* m_XmlContainer;
        DbEnv* m_dbenv;
        XmlManager* m_manager;
        
    protected:

        DbEnv* get_db_env();
        
        
        virtual bool isDir()
            {
                return true;
            }
        
        virtual ferris_ios::openmode getSupportedOpenModes()
            {
                return ios::in | ios::out | ios::binary | ios::ate | ios::trunc;
            }
        
        static fh_context my_SubCreate_file( fh_context c, fh_context md )
            {
                if( DBXMLRootContext* bc = dynamic_cast<DBXMLRootContext*>( GetImpl(c) ))
                {
//                    cerr << "my_SubCreate_file() c:" << c->getURL() << endl;
                    return bc->SubCreate_file( c, md );
                }
                Throw_FerrisCreateSubContextNotSupported("", GetImpl(c) );
            }
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
                m["file"] = SubContextCreator( _Self::my_SubCreate_file,
                                               "	<elementType name=\"file\">\n"
                                               "		<elementType name=\"name\" default=\"new directory\">\n"
                                               "			<dataTypeRef name=\"string\"/>\n"
                                               "		</elementType>\n"
                                               "		<elementType name=\"root-element\" default=\"root\">\n"
                                               "			<dataTypeRef name=\"string\"/>\n"
                                               "		</elementType>\n"
                                               "	</elementType>\n");
            }
        
        virtual fh_context SubCreate_file( fh_context c, fh_context md );

        virtual bool supportsRemove()
            {
                return true;
            }
    
        virtual void priv_remove( fh_context c_ctx );
        

        virtual DBXMLRootContext* priv_CreateContext( Context* parent, std::string rdn )
            {
                DBXMLRootContext* ret = new DBXMLRootContext( parent, rdn );
                return ret;
            }
        

        virtual void priv_read();

        
    public:

        DBXMLRootContext( Context* parent, const std::string& rdn ); 
        virtual ~DBXMLRootContext();

        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }

//         std::string
//         getDirPath() throw (FerrisParentNotSetError)
//             {
//                 cerr << "dbxml::getDirPath() isParentBound():" << isParentBound()
//                      << " basepath:" << _Base::getDirPath()
//                      << " dirname:" << getDirName()
//                      << endl;

//                 return getCoveredContext()->getDirPath();
//             }

//         virtual std::string getDirName() const
//             {
//                 std::string ret = getCoveredContext()->getDirName();
//                 return ret;
//             }
        

        
        /********************/
        /********************/
        /********************/

        myXmlContainer* getContainer()
            {
                return m_XmlContainer;
            }
        XmlManager* manager()
            {
                return m_manager;
            }
        
        
    };

    /**
     * This is very much the same level as XmlDocument in the dbxml API.
     * We get the xerces-c DOM from dbxml and mount it.
     */
    class FERRISEXP_CTXPLUGIN DBXMLDocumentContext
        :
        public XMLBaseContext
    {
        typedef DBXMLDocumentContext  _Self;
        typedef XMLBaseContext        _Base;

        XmlDocument  m_xdoc;
        
        
    protected:
        
        virtual void priv_syncTree( bool force = false );
        virtual XMLBaseContext* ensureCreated( const std::string& rdn );
        virtual void priv_read();

        virtual fh_xmlbc ensureCreated( const std::string& rdn, DOMElement* e )
            {
                Throw_FerrisInternalError("not implemented", 0 );
            }
        
        
        virtual DBXMLDocumentContext* priv_CreateContext( Context* parent, std::string rdn )
            {
                DBXMLDocumentContext* ret = new DBXMLDocumentContext( parent, rdn );
                return ret;
            }
        DBXMLDocumentContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_xdoc()
            {
            }


        
        void
        addNewAttribute( const std::string& prefix,
                         const std::string& rdn, 
                         XSDBasic_t sct = XSD_UNKNOWN )
            {
                addAttribute( rdn,
                              this, &_Self::getDublinCoreAttributeStream,
                              this, &_Self::getDublinCoreAttributeStream,
                              this, &_Self::OnDublinCoreAttributeStreamClosed,
                              sct );
            }
        
    public:

        XmlManager* manager()
            {
                return (dynamic_cast<DBXMLRootContext*>( getParent() ))->manager();
            }
        
            
        DBXMLDocumentContext( DBXMLRootContext* parent, const std::string& rdn, XmlDocument& d )
            :
            _Base( parent, rdn ),
            m_xdoc( d )
            {
                const stringlist_t& dca = getUnqualifiedDublinCoreAttributeNames();
                for( stringlist_t::const_iterator si = dca.begin(); si != dca.end(); ++si )
                {
                    addNewAttribute( dcURI, *si, XSD_BASIC_STRING );
                }
                addNewAttribute( dcURI, "mtime", FXD_UNIXEPOCH_T );
                addNewAttribute( dcURI, "atime", FXD_UNIXEPOCH_T );
                addNewAttribute( dcURI, "ctime", FXD_UNIXEPOCH_T );
            }

        fh_stringstream
        getDublinCoreAttributeStream( Context* c,
                                      const std::string& rdn,
                                      EA_Atom* atom )
            {
                fh_stringstream ret;

                if( DBXMLDocumentContext* selfp = dynamic_cast<DBXMLDocumentContext*>(c))
                {
                    string k = rdn;
                    XmlValue metaValue;
                    if( selfp->m_xdoc.getMetaData( dcURI, k, metaValue ) )
                        ret << metaValue.asString();
                }
                
                return ret;
            }

        void
        OnDublinCoreAttributeStreamClosed( Context* c,
                                           const std::string& rdn,
                                           EA_Atom* atom,
                                           fh_istream ss )
            {
                string k = rdn;

                if( DBXMLDocumentContext* selfp = dynamic_cast<DBXMLDocumentContext*>(c))
                {
                    try
                    {
                        XmlValue metaValue( StreamToString(ss) );
//                        selfp->m_xdoc.setMetaData( dcURI, dcPrefix, k, metaValue );
                        selfp->m_xdoc.setMetaData( dcURI, k, metaValue );
                    }
                    catch( XmlException& e )
                    {
                        fh_stringstream ss;
                        ss << "Update of dublin core metadata failed for:" << c->getURL()
                           << " reason:" << e.what() << endl;
                        Throw_getIOStreamCloseUpdateFailed( tostr(ss), c );
                    }
                }
            }
        
        
        virtual ~DBXMLDocumentContext()
            {
            }

        XmlDocument& getXmlDocument()
            {
                return m_xdoc;
            }

        DBXMLRootContext* getDBXMLRootContext()
            {
                Context* pp = getBaseContext()->getParent();
                
                DBXMLRootContext* ret = dynamic_cast<DBXMLRootContext*>( pp->getOverMountContext() );
                if( !ret )
                {
                    LG_XML_ER << "getDBXMLRootContext() failed. should never happen!" << endl;
                    BackTrace();
                }
                return ret;
            }
        
        myXmlContainer* getContainer()
            {
                return getDBXMLRootContext()->getContainer();
            }
        
    };
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    DBXMLRootContext::DBXMLRootContext( Context* parent, const std::string& rdn )
        :
        _Base( parent, rdn ),
        m_XmlContainer( 0 ),
        m_manager( 0 ),
        m_dbenv( 0 )
    {
        LG_CTX_D << "DBXMLRootContext::DBXMLRootContext() rdn:" << rdn << endl;
        createStateLessAttributes();
    }

    DBXMLRootContext::~DBXMLRootContext()
    {
        if( m_XmlContainer )
            delete m_XmlContainer;
    }
    

    fh_context
    DBXMLRootContext::SubCreate_file( fh_context c, fh_context md )
    {
        string rdn    = getStrSubCtx( md, "name", "" );
        string earl   = c->getDirPath() + "/" + rdn;
        string ename  = getStrSubCtx( md, "root-element", "defaultroot" );

        fh_stringstream contentss;
        contentss << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << endl;
        contentss << "<" << ename << "/>" << endl;

//        cerr << "SubCreate_file content:" << tostr(contentss) << endl;

        XmlUpdateContext dbxmlContext = m_manager->createUpdateContext();
        XmlDocument newdoc;
        newdoc.setContent( tostr( contentss ));
        
        //Set the document name
        newdoc.setName( rdn );

        if( !m_XmlContainer )
            priv_read();
        
        m_XmlContainer->getContainer().putDocument( newdoc, dbxmlContext, 0 );

        Context* tmp = new DBXMLDocumentContext( this, newdoc.getName(), newdoc );
        fh_context child = tmp;
        Insert( tmp );
        return child;
    }

    void
    DBXMLRootContext::priv_remove( fh_context c_ctx )
    {
        LG_CTX_D << "dbxml_remove(top) url:" << getURL() << endl;
                
        DBXMLDocumentContext* c = dynamic_cast<DBXMLDocumentContext*>( (GetImpl(c_ctx) ) );
        if( !c )
        {
            fh_stringstream ss;
            ss << "Attempt to remove a non xml context from within a dbXML file!"
               << " url:" << c_ctx->getURL();
            Throw_CanNotDelete( tostr(ss), c );
        }
        string url = c->getURL();
        string rdn = c->getDirName();
                
        LG_XML_D << "dbxml_remove() url:" << url << endl;
        
        try
        {
            Throw_CanNotDelete( "API needs update...", 0 );

            
//             XmlResults results(
//                 m_XmlContainer->getContainer().queryWithXPath(
//                     0, "//*[@dbxml:name='" + rdn + "']" ) );

//             XmlUpdateContext dbxmlContext = m_manager->createUpdateContext();
//             XmlValue value;
//             while(results.next(value))
//             {
//                 XmlDocument d( value.asDocument(0) );
// //                cerr << "calling deleteDocument() for name:" << d.getName() << endl;
//                 m_XmlContainer->getContainer().deleteDocument( d, dbxmlContext );
//             }
        }
        catch (const XmlException& e)
        {
            std::ostringstream ss;
            ss << "Can not delete child:" << url
               << " reason:" << e.what() << endl;
            Throw_CanNotDelete( tostr(ss), 0 );
        }
        m_XmlContainer->sync();
    }


    DbEnv*
    DBXMLRootContext::get_db_env()
    {
        if( m_dbenv )
            return m_dbenv;
        
        u_int32_t env_flags = 0 |
            DB_CREATE     |  // If the environment does not exist, create it.
            DB_INIT_LOCK  |  // Initialize locking
            DB_INIT_LOG   |  // Initialize logging
            DB_INIT_MPOOL |  // Initialize the cache
            DB_INIT_TXN;     // Initialize transactions

        std::string envHome = getParent()->getDirPath();
        m_dbenv = new DbEnv(0);
        m_dbenv->open( envHome.c_str(), env_flags, 0 );
        return m_dbenv;
    }
    
    
    void
    DBXMLRootContext::priv_read()
    {
        LG_CTX_D << "DBXMLRootContext::priv_read() url:" << getURL() << endl;

        
        staticDirContentsRAII _raii1( this );

        if( empty() )
        {
            if( !m_XmlContainer )
            {
                emitExistsEventForEachItem();
                return;
            }

            try
            {
                string filepath = Ferris::CleanupURL( getURL() );

                if( !m_dbenv )
                    get_db_env();
                if( !m_manager )
                    m_manager =  new XmlManager( m_dbenv, DBXML_ADOPT_DBENV );
                
//                static myDbEnv theDBEnv( "/tmp/dbxml-1.1.0/examples/cxx/gettingStarted/dbEnvironment" );
//                m_XmlContainer = new myXmlContainer( "simpleExampleData.dbxml", theDBEnv );
//                cerr << "Trying to read container at:" << filepath << endl;
                m_XmlContainer = new myXmlContainer( m_manager, filepath );
              
                XmlQueryContext context;
//              context.setNamespace( "ferris", "ferris://");

                XmlQueryContext qc = manager()->createQueryContext();
                XmlQueryExpression qe = manager()->prepare( "//*[@dbxml:name]", qc );
                XmlResults results = qe.execute( qc, 0 ); 
//                 XmlResults results(
//                     m_XmlContainer->getContainer().queryWithXPath(
//                         0, "//*[@dbxml:name]", &context ) );

                XmlValue value;
                while(results.next(value))
                {
                    XmlDocument theDoc = value.asDocument();
                    LG_CTX_D << "Found document:" << theDoc.getName() << endl;
                    
                    fh_context child = new DBXMLDocumentContext( this, theDoc.getName(), theDoc );
                    addNewChild( child );
                }
            }
            catch(XmlException& e)
            {
                fh_stringstream ss;
                ss << "Error reading list of documents in dbxml file e:" << e.what() << endl;
                Throw_CanNotReadContext( tostr(ss), this );
            }
        }
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    void
    DBXMLDocumentContext::priv_syncTree( bool force )
    {
        LG_CTX_D << "DBXMLDocumentContext::priv_syncTree() url:" << getURL() << endl;
        
        fh_stringstream ss = tostream( getDocument() );
        m_xdoc.setContent( tostr( ss ) );
        
        {
            DbXml::XmlUpdateContext dbxmlContext = manager()->createUpdateContext();
            getContainer()->updateDocument( m_xdoc, dbxmlContext );
        }
        getContainer()->sync();
    }
    
    XMLBaseContext*
    DBXMLDocumentContext::ensureCreated( const std::string& rdn )
    {
        Util::ValueBumpDrop<ref_count_t> dummy( ref_count );

        if( !isSubContextBound( rdn ) )
        {
            DBXMLDocumentContext* ret = (DBXMLDocumentContext*)CreateContext( this, monsterName( rdn ));
            Insert( ret );
            return ret;
        }
        return (DBXMLDocumentContext*)GetImpl(getSubContext( rdn ));
    }
    
    
    void
    DBXMLDocumentContext::priv_read()
    {
        EnsureStartReadingIsFired();
        if( SubContextCount() == 0 && getBaseContext() == this )
        {
            try
            {
                LG_CTX_D << "DBXMLDocumentContext::priv_read() url:" << getURL() << endl;
                
                DOMDocument* dom = m_xdoc.getContentAsDOM();
                setDocument( dom );
                LG_CTX_D << "DBXMLDocumentContext::priv_read() dom:" << toVoid(dom) << endl;
                WrapDOMTree( dom, this, "", 0 );
            }
            catch(XmlException& e)
            {
                fh_stringstream ss;
                ss << "Error reading document in dbxml file e:" << e.what() << endl;
                Throw_CanNotReadContext( tostr(ss), this );
            }
        }
        else
        {
            emitExistsEventForEachItem();
        }
        EnsureStopReadingIsFired();
        updateMetaData();
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/



    static void StaticInit()
    {
        Factory::ensureXMLPlatformInitialized();
    }

    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            StaticInit();

            static DBXMLRootContext raw_obj(0,"/");
            fh_context ret = raw_obj.CreateContext( 0, rf->getInfo( "Root" ));
            return ret;
        }
    }
 
};
