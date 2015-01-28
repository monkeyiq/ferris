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

    $Id: FerrisDOM_private.hh,v 1.7 2010/09/24 21:30:36 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_DOM_PRIV_H_
#define _ALREADY_INCLUDED_FERRIS_DOM_PRIV_H_

#include <Ferris/FerrisDOM.hh>

namespace Ferris
{
    using namespace XERCES_CPP_NAMESPACE;

    /********************/
    /********************/
    /********************/

    class FERRISEXP_DLLLOCAL Ferris_DOM_NamespaceImplier
    {
        mutable XMLCh*     m_localName;
        mutable XMLCh*     m_nsURI;
    public:
        explicit Ferris_DOM_NamespaceImplier();
        ~Ferris_DOM_NamespaceImplier();
        const XMLCh* common_getNamespaceURI( const XMLCh* name ) const;
        const XMLCh* common_getLocalName( const XMLCh* name ) const;
    };

    /********************/
    /********************/
    /********************/
    
    class FERRISEXP_DLLLOCAL Ferris_DOMNodeList
        :
        public DOMNodeList,
        public Handlable
    {
        typedef std::vector< DOMNode* > m_col_t;
        m_col_t m_col;
            
    protected:
        Ferris_DOMNodeList(const Ferris_DOMNodeList &) {};
        Ferris_DOMNodeList & operator = (const Ferris_DOMNodeList &) {return *this;};

    public:
        Ferris_DOMNodeList();
        virtual ~Ferris_DOMNodeList();

        void push_back( DOMNode* n );
        DOMNode* front() const;
        DOMNode* back() const;
            
        // DOM1 //
        virtual DOMNode  *item(XMLSize_t index) const;
        virtual XMLSize_t getLength() const;
    };
        
    class FERRISEXP_DLLLOCAL Ferris_DOMNamedNodeMap
        :
        public DOMNamedNodeMap,
        public Handlable
    {
        typedef std::map< std::string, DOMNode* > m_nodes_t;
        mutable m_nodes_t m_nodes;
        
    protected:
        Ferris_DOMNamedNodeMap(const Ferris_DOMNamedNodeMap &) {};
        Ferris_DOMNamedNodeMap & operator = (const Ferris_DOMNamedNodeMap &) {return *this;};
        
    public:
        Ferris_DOMNamedNodeMap( const DOMNode *ownerNod );
        virtual ~Ferris_DOMNamedNodeMap();
        
        virtual DOMNode   *setNamedItem(DOMNode *arg);
        virtual DOMNode     *item(XMLSize_t index) const;
        virtual DOMNode   *getNamedItem(const XMLCh *xname) const;
        virtual XMLSize_t   getLength() const;
        virtual DOMNode    *removeNamedItem(const XMLCh *xname);
        virtual DOMNode* getNamedItemNS(const XMLCh *namespaceURI,
                                        const XMLCh *localName) const;
        virtual DOMNode   *setNamedItemNS(DOMNode *arg);
        virtual DOMNode *removeNamedItemNS(const XMLCh *namespaceURI,
                                           const XMLCh *localName);
    };

    class Ferris_DOMAttr;
    class Ferris_DOMElement;
    class Ferris_DOMDocument;
    FERRIS_SMARTPTR( Ferris_DOMNodeList, fhx_nodelist );
    FERRIS_SMARTPTR( Ferris_DOMNamedNodeMap, fhx_namednodemap );
    FERRIS_SMARTPTR( Ferris_DOMAttr,     fhx_attr );
    FERRIS_SMARTPTR( Ferris_DOMElement,  fhx_element );

    class FERRISEXP_DLLLOCAL Ferris_DOMElement
        :
        public DOMElement,
        public Handlable,
        public Ferris_DOM_NamespaceImplier
    {
        Ferris_DOMDocument* m_doc;
        DOMNode*            m_parent;

        void changed() const;
        int changes() const;

        fh_context m_context;
        typedef std::map< std::string, XMLCh* > m_xcache_t;
        mutable m_xcache_t m_xcache;
        typedef std::map< std::string, fhx_attr > m_xattrcache_t;
        mutable m_xattrcache_t m_xattrcache;

        mutable fhx_nodelist     m_cache_getChildNodes;
        mutable Ferris_DOMNamedNodeMap* m_fAttributes;
    private:
        Ferris_DOMElement(const Ferris_DOMElement& );
        Ferris_DOMElement & operator = (const Ferris_DOMElement& );

        Ferris_DOMNamedNodeMap* ensure_fAttributes() const;
        bool shouldHideAttribute( const XMLCh* xname ) const;
        
    protected:

        /**
         * We have to keep UTF-16 strings around to look like a standard
         * DOMElement. Reuse them if we can
         */
        XMLCh* cacheString( const std::string& s ) const;
            
    public:
        Ferris_DOMElement( Ferris_DOMDocument* m_doc,
                           DOMNode*  m_parent,
                           fh_context c );
        virtual ~Ferris_DOMElement();

        /**
         * This is only for use within Ferris core library, but this header
         * should only be available to libferris anyway
         */
        fh_context getContext() const
            {
                return m_context;
            }
        

        virtual const XMLCh* getTagName() const;
        virtual const XMLCh* getAttribute(const XMLCh* xname) const;
//        virtual DOMAttr* getAttributeNode( const std::string& n ) const;
        virtual DOMAttr* getAttributeNode(const XMLCh* xname) const;
        virtual DOMNodeList   * getElementsByTagName(const XMLCh *name) const;
        virtual void setAttribute(const XMLCh *name, const XMLCh *value);
        virtual DOMAttr* setAttributeNode(DOMAttr *newAttr);
        virtual DOMAttr* removeAttributeNode(DOMAttr *oldAttr);
        virtual void removeAttribute(const XMLCh *name);

        // DOM2 // 
        virtual const XMLCh* getAttributeNS(const XMLCh *namespaceURI,
                                            const XMLCh *localName) const;
            virtual void setAttributeNS(const XMLCh *namespaceURI,
                                        const XMLCh *qualifiedName,
                                        const XMLCh *value);
        virtual void removeAttributeNS(const XMLCh *namespaceURI,
                                       const XMLCh *localName);
        virtual DOMAttr* getAttributeNodeNS(const XMLCh *namespaceURI,
                                            const XMLCh *localName) const;
        virtual DOMAttr* setAttributeNodeNS(DOMAttr *newAttr);
        virtual DOMNodeList* getElementsByTagNameNS(const XMLCh *namespaceURI,
                                                    const XMLCh *localName) const;
            
        virtual bool hasAttribute(const XMLCh* xname) const;
        virtual bool hasAttributeNS(const XMLCh *namespaceURI,
                                    const XMLCh *localName) const;
            
        // DOM3 //
        virtual void setIdAttribute(const XMLCh* name, bool isId);
        virtual void setIdAttributeNS(const XMLCh* namespaceURI, const XMLCh* localName, bool isId);
        virtual void setIdAttributeNode(const DOMAttr *idAttr, bool isId);
        virtual const DOMTypeInfo* getSchemaTypeInfo() const;


        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        // DOMNode //
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        virtual const XMLCh* getNodeName() const;
        virtual const XMLCh* getNodeValue() const;
        virtual XERCES_CPP_NAMESPACE::DOMNode::NodeType getNodeType() const;
        virtual DOMNode* getParentNode() const;
        virtual Ferris_DOMNodeList* getChildNodes_sub() const;
        virtual DOMNodeList *getChildNodes() const;
        virtual DOMNode* getFirstChild() const;
        virtual DOMNode* getLastChild() const;
        virtual DOMNode* getPreviousSibling() const;
        virtual DOMNode* getNextSibling() const;
        virtual DOMNamedNodeMap *getAttributes() const;
        virtual DOMDocument* getOwnerDocument() const;
        virtual DOMNode* cloneNode(bool deep) const;
        virtual DOMNode* insertBefore(DOMNode *newChild,
                                      DOMNode *refChild);
        virtual DOMNode* replaceChild(DOMNode *newChild,
                                      DOMNode *oldChild);
        virtual DOMNode* removeChild(DOMNode *oldChild);
        virtual DOMNode* appendChild(DOMNode *newChild);
        virtual bool hasChildNodes() const;
        virtual void setNodeValue(const XMLCh  *nodeValue);

        virtual DOMElement* getFirstElementChild() const;
        virtual DOMElement* getLastElementChild() const;
        virtual DOMElement* getPreviousElementSibling() const;
        virtual DOMElement* getNextElementSibling() const;
        virtual XMLSize_t getChildElementCount() const;


        // DOM2 //

        virtual void normalize();
        virtual bool isSupported(const XMLCh *feature,
                                 const XMLCh *version) const;
        virtual const XMLCh* getNamespaceURI() const;
        virtual const XMLCh* getPrefix() const;
        virtual const XMLCh* getLocalName() const;
        virtual void setPrefix(const XMLCh * prefix);
        virtual bool hasAttributes() const;
        
        // DOM3 // 
        virtual bool isSameNode(const DOMNode* other) const;
        virtual bool isEqualNode(const DOMNode* arg) const;
            virtual void* setUserData(const XMLCh* key,
                                      void* data,
                                      DOMUserDataHandler* handler);
        virtual void* getUserData(const XMLCh* key) const;
        virtual const XMLCh* getBaseURI() const;
        
        virtual short compareDocumentPosition(const DOMNode* other) const;
        virtual const XMLCh* getTextContent() const;
        virtual void setTextContent(const XMLCh* textContent);
        virtual const XMLCh* lookupPrefix(const XMLCh* namespaceURI) const;
        virtual bool isDefaultNamespace(const XMLCh* namespaceURI) const;
        virtual const XMLCh* lookupNamespaceURI(const XMLCh* prefix) const;
        virtual void* getFeature( const XMLCh* feature, const XMLCh* version ) const;
        virtual void release();

        
        
    };
    
};
#endif
