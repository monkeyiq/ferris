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

    $Id: libferrisxmlshared.hh,v 1.10 2010/09/24 21:31:02 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_XML_SHARED_H_
#define _ALREADY_INCLUDED_FERRIS_XML_SHARED_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <FerrisContextPlugin.hh>
#include <FerrisDOM.hh>
#include <PluginOutOfProcNotificationEngine.hh>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/URLInputSource.hpp>

#define STRING_OR_EMPTY( x )  (x) ? (x) : (XMLCh*)"";
#define X(str) XStr(str).unicodeForm()

namespace Ferris
{
    using namespace std;
    using namespace XERCES_CPP_NAMESPACE;

    class XMLBaseContext;
    FERRIS_CTX_SMARTPTR( XMLBaseContext, fh_xmlbc );
    class XMLContext;

    
    /**
     * Base context for all things that expose a Xerces DOM.
     * Current examples are mounting XML and mounting dbxml.
     *
     * This class handles converting rename/create/remove/read/write
     * from libferris style interaction to xerces-c DOM interaction.
     *
     * Sublcasses only really have to handle:
     * 1) create new context objects with ensureCreated()
     * 2) somehow generate an xerces-c DOM and pass it to WrapDOMTree()
     * 3) somehow save entire DOM to disk when priv_syncTree() is called.
     */
    class FERRISEXP_EXPORT XMLBaseContext
        :
        public RecommendedEACollectingContext<Context>
    {
        typedef XMLBaseContext                           _Self;
        typedef RecommendedEACollectingContext<Context>  _Base;
        friend class XMLContext;
        
        Context* priv_CreateContext( Context* parent, std::string rdn ) = 0;

        /**
         * This is only valid in the base context. access it only via getDocument()
         */
        fh_domdoc m_doc;

        bool m_publicID_attributeCreated;
        bool m_systemID_attributeCreated;
        
        std::string m_publicID;
        std::string m_systemID;
        std::string m_internalSubset;
        std::string m_notationName;
        
        typedef std::map< std::string, std::string, Nocase> m_attrs_t;
        m_attrs_t m_attrs;

        DOMElement* m_element;

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        void setPublicID( const std::string& s )
            {
                m_publicID = s;

                if( !m_publicID_attributeCreated )
                {
                    m_publicID_attributeCreated = true;
                    addAttribute( "public-id",
                                  this,
                                  &_Self::getPublicIDStream,
                                  XSD_BASIC_STRING,
                                  true );
                }
            }
        
        void setSystemID( const std::string& s )
            {
                m_systemID = s;
                
                if( !m_systemID_attributeCreated )
                {
                    m_systemID_attributeCreated = true;
                    addAttribute( "system-id",
                                  this,
                                  &_Self::getSystemIDStream,
                                  XSD_BASIC_STRING,
                                  true );
                }
            }
            
    
        std::string getPublicID( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                return m_publicID;
            }

        fh_istream getPublicIDStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << m_publicID;
                return ss;
            }

        std::string getSystemID( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                return m_systemID;
            }

        fh_istream getSystemIDStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << m_systemID;
                return ss;
            }
        
        std::string getInternalSubset( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                return m_internalSubset;
            }

        fh_istream getInternalSubsetStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << m_internalSubset;
                return ss;
            }

        std::string getNotationName( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                return m_notationName;
            }
        
        fh_istream getNotationNameStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << m_notationName;
                return ss;
            }
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        typedef std::list< std::string > m_attributesThatCanNameNode_t;

        /**
         * Get a list of which attribute an element can have that can give a
         * name for this node in the filesystem
         */
        virtual m_attributesThatCanNameNode_t&
        getAttributesThatCanNameNode()
            {
                static m_attributesThatCanNameNode_t ret;

                if( ret.empty() )
                {
                    ret.push_back( "title" );
                    ret.push_back( "name" );
                    ret.push_back( "rdn" );
                    ret.push_back( "dn" );
                    ret.push_back( "id" );
                }
                
                return ret;
            }

        void OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m );
        virtual fh_stringstream real_getIOStream( ferris_ios::openmode m );
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m );
        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m );

        virtual bool supportsRemove()
            {
                return true;
            }
    
        virtual void priv_remove( fh_context c_ctx )
            {
                std::string olddn = c_ctx->getDirName();
                
                XMLBaseContext* c = dynamic_cast<XMLBaseContext*>( (GetImpl(c_ctx) ) );
                if( !c )
                {
                    fh_stringstream ss;
                    ss << "Attempt to remove a non xml context from XML file! url:" << c_ctx->getURL();
                    Throw_CanNotDelete( tostr(ss), GetImpl(c_ctx) );
                }
                std::string url = c->getURL();
                LG_XML_D << "remove() url:" << url << endl;

                try
                {
                    DOMElement* parent = this->getElement();
                    DOMElement* child  =    c->getElement();
                    parent->removeChild( child );
                }
                catch (const XMLException& e)
                {
                    std::ostringstream ss;
                    ss << "Can not delete child:" << url
                       << " reason:" << tostr(e.getMessage()) << endl;
                    Throw_CanNotDelete( tostr(ss), 0 );
                }

                syncTree();
                
                if( shouldPerformFullJournaling() )
                    Factory::getPluginOutOfProcNotificationEngine().signalContextDeleted( this, olddn );
            }

        void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );
        static fh_context xmlbase_SubCreate_element( fh_context c, fh_context md );
        static fh_context xmlbase_SubCreate_attribute(  fh_context c, fh_context md );

    protected:
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /*** Methods that subclasses must override for the module to work ***************/
        /********************************************************************************/

        /**
         * When something has changed in the DOM this method is called so that the
         * module can write the DOM back to disk. This method is only ever called
         * on the base of the XML tree which should also be the context that owns
         * the DOMDocument.
         */
        virtual void priv_syncTree( bool force = false ) = 0;

        /**
         * When WrapDOMTree() and other methods discover or create
         * an element in the DOM the subclass must create a new instance
         * of itself. The rdn is always the relative name for the new context
         * and can only ever be a direct child of the Context it is called on.
         * See libxml.cpp for an example of it.
         *
         * @param rdn name for new child context.
         */
        virtual fh_xmlbc ensureCreated( const std::string& rdn, DOMElement* e ) = 0;

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /*** Methods that subclasses may override for the module to work ****************/
        /********************************************************************************/

        /**
         * Should this XML tree perform full metadata+data journaling across processes?
         */
        virtual bool shouldPerformFullJournaling();
        
        /**
         * Called to attach the read/write handlers for a new attribute which the user
         * created. This gives subclasses the ability to put the attribute into a custom
         * namespaceURI in the storage.
         */
        virtual void addNewAttribute( const std::string& prefix,
                                      const std::string& rdn, 
                                      XSDBasic_t sct = XSD_UNKNOWN );

        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        static const std::string dcURI;
        static const std::string dcPrefix;
        
        XMLBaseContext( Context* parent, const std::string& rdn );

    public:
        virtual ~XMLBaseContext();
        Context*   getCC()
            {
                return _Base::getCoveredContext();
            }
        
        
    protected:
        /**
         * The main method for this class. Takes a top level DOM and creates wrappers
         * for all of that DOMs elements and attributes for the ferris world.
         */
        void WrapDOMTree( DOMNode* node,
                          fh_xmlbc parent,
                          std::string new_node_name,
                          RootContextFactory* rf );

        /**
         * call priv_syncTree() on the base context
         */
        void syncTree( bool force = false )
            {
                if( getBaseContext() != this )
                {
                    getBaseContext()->syncTree( force );
                    return;
                }
                LG_DOM_D << "syncTree() doc:" << getDocument() << endl;
                priv_syncTree( force );
            }
        
        
        XMLBaseContext* getBaseContext()
            {
                XMLBaseContext* c = dynamic_cast<XMLBaseContext*>( this );

                while( c && c->isParentBound() )
                {
                    Context* p = c->getParent()->getOverMountContext();
                    if(XMLBaseContext* nextc = dynamic_cast<XMLBaseContext*>( p ))
                    {
                        c = nextc;
                    }
                    else
                    {
                        return c;
                    }
                }
                return c;
            }


        std::string xmlFileName()
            {
                XMLBaseContext* c = getBaseContext();
                std::string s = c->getDirPath();
                return s;
            }

    public:
        fh_domdoc getDocument()
            {
//                 cerr << "xmlbase::getDoc() base:" << toVoid( getBaseContext() ) << endl;
//                 cerr << " m_doc:" << getBaseContext()->m_doc << endl;
                if( !getBaseContext()->m_doc )
                    getBaseContext()->read();
//                cerr << " m_doc2:" << getBaseContext()->m_doc << endl;
                
                return getBaseContext()->m_doc;
            }

    protected:
        void setDocument( fh_domdoc d )
            {
                getBaseContext()->m_doc = d;
            }
        
        void setElement( DOMElement* e )
            {
                m_element = e;
            }
        
        DOMElement* getElement()
            {
                return m_element;
            }
        
        

        /**
         * FIXME: for this to be able to be stateless then each child of XMLBaseContext
         * has to inherit us through a StatelessHolder context.
         */
        void setStaticEA( const std::string& k, const std::string& v )
            {
                try
                {
                    LG_XML_D << "adding to :" << getDirPath()
                             << " attr k:" << k << " v:" << v
                             << endl;
                    
                    addAttribute( k, v, XSD_BASIC_STRING, true );
                }
                catch(...)
                {}
            }

        
        fh_iostream getEA( Context* _c, const std::string& rdn, EA_Atom* atom );
        void EAUpdated( Context* _c, const std::string& rdn, EA_Atom* atom, fh_istream iss );
        


        fh_istream getFerrisAsXMLDocument( Context* _c, const std::string& rdn, EA_Atom* atom );
        
        
        virtual void priv_read()
            {
                updateMetaData();
                LG_XML_D << "XMLBaseContext::priv_read() path :" << getDirPath() << endl;
                emitExistsEventForEachItemRAII _raii1( this );
//                 EnsureStartReadingIsFired();
//                 emitExistsEventForEachItem();
//                 EnsureStopReadingIsFired();
            }
        

        virtual bool getSubContextAttributesWithSameNameHaveSameSchema()
            {
                return false;
            }

        virtual ferris_ios::openmode getSupportedOpenModes()
            {
//                cerr << "xbase::getSupportedOpenModes()" << endl;
                return ios::in | ios::out | ios::binary | ios::ate | ios::trunc;
            }
        
    };
    
    

};
#endif
