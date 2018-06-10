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

    $Id: libferrisxmlshared.cpp,v 1.15 2010/11/12 21:30:10 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <libferrisxmlshared.hh>

namespace Ferris
{
    using namespace std;
    
    /********************/
    /********************/
    /********************/

    const std::string XMLBaseContext::dcURI = "http://purl.org/dc/elements/1.1/";
    const std::string XMLBaseContext::dcPrefix = "dc";

    XMLBaseContext::XMLBaseContext( Context* parent, const std::string& rdn )
        :
        _Base( parent, rdn ),
        m_doc( 0 ),
        m_publicID_attributeCreated( false ),
        m_systemID_attributeCreated( false ),
        m_element( 0 )
    {
    }
    
    XMLBaseContext::~XMLBaseContext()
    {
        if( m_doc )
        {
            LG_DOM_D << "~XMLBaseContext() dom:" << toVoid(GetImpl(m_doc)) << endl;
        }
        
        
    }

    fh_istream
    XMLBaseContext::getFerrisAsXMLDocument( Context* c, const std::string& rdn, EA_Atom* atom )
    {
//        fh_stringstream ret =  tostream( getDocument() );
        fh_stringstream ret =  tostream( *getElement() );
        return ret;
        
//         Factory::ensureXMLPlatformInitialized();

//         DOMImplementation *impl = Factory::getDefaultDOMImpl();
//         fh_domdoc doc = impl->createDocument( 0, X("context"), 0 );
//         DOMElement* root = doc->getDocumentElement();

//         typedef AttributeCollection::AttributeNames_t::iterator I;
//         AttributeCollection::AttributeNames_t an;
//         c->getAttributeNames( an );
            
//         for( I ai = an.begin(); ai != an.end(); ++ai )
//         {
//             string k = *ai;
//             if( k == "content" )
//                 continue;
//             if( k == "as-xml" )
//                 continue;
//             if( k == "as-rdf" )
//                 continue;

//             try
//             {
//                 string v = getStrAttr( c, k, "", true, true );
                    
//                 DOMElement* e = doc->createElement( X("keyval") );
//                 root->appendChild( e );
//                 ::Ferris::setAttribute( e, "key",   k );
// //                    setAttribute( e, "value", v );
//                 DOMText* payload = doc->createTextNode( X(v.c_str()));
//                 e->appendChild( payload );
//             }
//             catch( exception& e )
//             {
//             }
//         }

//         fh_stringstream retss = tostream( doc );
//         return retss;
        
    }
    
    void
    XMLBaseContext::WrapDOMTree( DOMNode* node,
                                 fh_xmlbc parent,
                                 string new_node_name,
                                 RootContextFactory* rf )
        throw( RootContextCreationFailed )
    {
        LG_XML_D << "WrapDOMTree path   : " << getDirPath() << endl;
        LG_XML_D << "WrapDOMTree newname: " << new_node_name << endl;

        const XMLCh* Name  = node->getNodeName();
        const XMLCh* Value = node->getNodeValue();
        unsigned long NameLen  = XMLString::stringLen( Name );
        unsigned long ValueLen = XMLString::stringLen( Value );

        switch (node->getNodeType())
        {
        case DOMNode::TEXT_NODE:
        {
            DOMText* x = (DOMText*)node;
        
            LG_XML_D
                << "DOMNode::TEXT_NODE:  Name :" << tostr( Name  ) << endl
                << "DOMNode::TEXT_NODE:  value:" << tostr( Value ) << endl
                << "DOMNode::TEXT_NODE:  valueLen:" << tostr( Value ).length() << endl;
//         fh_xmlbc c = (XMLBaseContext*)CreateContext( parent, parent->monsterName(tostr(Name)) );
//         parent->Insert( c );
            LG_XML_D << "DOMNode::TEXT_NODE:  parent:" << parent->getDirName() << endl
                     << "DOMNode::TEXT_NODE:  parent:" << parent->getDirPath() << endl;

            if( !x->isIgnorableWhitespace() )
            {
//             fh_xmlbc c = parent->ensureCreated( new_node_name );
//             c->FileContents = tostr( Value );
            
//            parent->FileContents = tostr( Value );
//            cerr << "Just set text node contents to:" <<parent->FileContents<<endl;
            
            }
        
            break;
        }
    

        case DOMNode::PROCESSING_INSTRUCTION_NODE:
            break;

        case DOMNode::DOCUMENT_NODE:
        {
            LG_XML_D << "DOMNode::DOCUMENT_NODE: Name :" << tostr(Name) << endl;
//        LG_XML_D << "DOMNode::DOCUMENT_NODE: Value:" << tostr(Value) << endl;

            DOMNode* child = node->getFirstChild();
            while( child != 0 )
            {
                WrapDOMTree( child, parent, tostr(Name), rf );
                child = child->getNextSibling();
            }
            break;
        }
     

        case DOMNode::ELEMENT_NODE:
        {
            string element_rdn = tostr(Name);
            DOMNamedNodeMap* attributes = node->getAttributes();

            if( !tostr(node->getLocalName()).empty() )
                element_rdn = tostr(node->getLocalName());
            
            LG_XML_D << "DOMNode::ELEMENT_NODE: Name:" << element_rdn << endl
                     << " local:" << tostr(node->getLocalName()) << endl
                     << " nsURI:" << tostr(node->getNamespaceURI()) << endl
                     << " nsPRE:" << tostr(node->getPrefix()) << endl
//                     << "DOMNode::ELEMENT_NODE: Value:" << tostr(Value) << endl
                     << "DOMNode::ELEMENT_NODE: attr size:" << attributes->getLength() << endl;
            
            m_attrs.clear();
            if( !tostr(node->getNamespaceURI()).empty() )
                m_attrs[ "ferris-element-namespace-uri" ] = tostr(node->getNamespaceURI());
            
            int attrCount = attributes->getLength();
            for (int i = 0; i < attrCount; i++)
            {
                DOMNode*  attribute = attributes->item(i);
                const string& n = tostr(attribute->getNodeName());
                const string& v = tostr(attribute->getNodeValue());
                m_attrs[ n ] = v;

                LG_XML_D << "DOMNode forall(attrs) node_name:" << tostr(attribute->getNodeName()) << endl
                         << " local:" << tostr(attribute->getLocalName()) << endl
                         << " nsURI:" << tostr(attribute->getNamespaceURI()) << endl
                         << " nsPRE:" << tostr(attribute->getPrefix()) << endl;
            }

            typedef m_attributesThatCanNameNode_t::iterator I;
            for( I i  = getAttributesThatCanNameNode().begin();
                 i   != getAttributesThatCanNameNode().end();
                 i++ )
            {
                if( m_attrs.find( *i ) != m_attrs.end() )
                {
                    if( !m_attrs[*i].empty() )
                    {
                        LG_XML_D << "Renaming node:" << element_rdn
                                 << " to:" << m_attrs[*i]
                                 << endl;
                        element_rdn = m_attrs[*i];
//                    sleep(4);
                        break;
                    }
                }
            }

            fh_xmlbc xmlctx = parent->ensureCreated( parent->monsterName(element_rdn), (DOMElement*)node );
//            xmlctx->setElement( (DOMElement*)node );

            if( xmlctx->getDirName() != element_rdn )
            {
                string rdn = xmlctx->getDirName();
                string elementNum = rdn.substr( element_rdn.length() + 2 );
                xmlctx->addAttribute( "libferris-element-name",   element_rdn );
                xmlctx->addAttribute( "libferris-element-number", elementNum );
            }
            else
            {
                xmlctx->addAttribute( "libferris-element-name",   element_rdn );
                xmlctx->addAttribute( "libferris-element-number", "0" );
            }
            
            
            for( m_attrs_t::iterator iter = m_attrs.begin();
                 iter != m_attrs.end();
                 iter++ )
            {
                const string& n = iter->first;
                const string& v = iter->second;
                LG_XML_D << "ATTR NAME1:" << n << " VALUE:" << v << endl;
//                xmlctx->setStaticEA( n, v );
                xmlctx->addAttribute( n,
                                      xmlctx, &_Self::getEA,
                                      xmlctx, &_Self::getEA,
                                      xmlctx, &_Self::EAUpdated,
                                      XSD_UNKNOWN );
                LG_XML_D << "ATTR NAME2:" << n << " VALUE:" << v << endl;
            }
            if( m_attrs.find( "url" ) != m_attrs.end() )
            {
                string n = "ferris-xml-url";
                xmlctx->m_attrs[ n ] = m_attrs["url"];
                xmlctx->addAttribute( n,
                                      xmlctx, &_Self::getEA,
                                      xmlctx, &_Self::getEA,
                                      xmlctx, &_Self::EAUpdated,
                                      XSD_BASIC_STRING );

                LG_DOM_D << "setup() c:" << xmlctx->getURL()
                         << " e:" << toVoid(xmlctx->getElement())
                         << " c->v:" << xmlctx->m_attrs["ferris-xml-url"]
                         << endl;
                
            }
            

            xmlctx->addAttribute( "libferris-tree-as-xml",
                                  xmlctx, &_Self::getFerrisAsXMLDocument );
            
            
            for( DOMNode* child = node->getFirstChild();
                 child != 0;
                 child = child->getNextSibling())
            { 
                WrapDOMTree( child, xmlctx, element_rdn, rf );
            }


//             /*
//              * If the element has a text component then add that as the content
//              * if this context.
//              */
//             for( DOMNode* child = node.getFirstChild();
//                  child != 0;
//                  child = child.getNextSibling())
//             {
//                 if( child.getNodeType() == TEXT_NODE )
//                 {
//                     DOMText* x = (DOMText*)child;
//                     if( !x.isIgnorableWhitespace() )
//                     {
                        
//                     }
//                     break;
//                 }
//             }
            
            break;
        }

        case DOMNode::ENTITY_REFERENCE_NODE:
        {
            LG_XML_D << "DOMNode::ENTITY_REFERENCE_NODE: Value:" << tostr(Name) << endl
                     << "DOMNode::ENTITY_REFERENCE_NODE: Value:" << tostr(Value) << endl;
            DOMNode* child;
            break;
        }


        case DOMNode::CDATA_SECTION_NODE:
        {
            LG_XML_D << "DOMNode::CDATA_SECTION_NODE: Value:" << tostr(Value) << endl;
                
//             *gFormatter << XMLFormatter::NoEscapes << gStartCDATA
//                         << nodeValue << gEndCDATA;
            break;
        }


        case DOMNode::COMMENT_NODE:
        {
            LG_XML_D << "DOMNode::COMMENT_NODE: Value:" << tostr(Value) << endl;
             
//             *gFormatter << XMLFormatter::NoEscapes << gStartComment
//                         << nodeValue << gEndComment;
            break;
        }


        case DOMNode::DOCUMENT_TYPE_NODE:
        {
            DOMDocumentType* x = (DOMDocumentType*)node;
            LG_XML_D << "DOMNode::DOCUMENT_TYPE_NODE: Value:" << tostr(Value) << endl;

            if( x->getPublicId() )
                parent->setPublicID( tostr(x->getPublicId()) );
            if( x->getSystemId() )
                parent->setSystemID( tostr(x->getSystemId()) );
            if( x->getInternalSubset() )
                parent->m_internalSubset = tostr( x->getInternalSubset() );

            parent->addAttribute( "internal-subset",
                                  parent, &XMLBaseContext::getInternalSubsetStream,
                                  XSD_BASIC_STRING,
                                  true );
            break;
        }


        case DOMNode::ENTITY_NODE:
        {
            DOMEntity* x = (DOMEntity*)node;
            LG_XML_D << "DOMNode::ENTITY_NODE: Value:" << tostr(Value) << endl;

            if( x->getPublicId() )
                parent->setPublicID( tostr(x->getPublicId()) );
            if( x->getSystemId() )
                parent->setSystemID( tostr(x->getSystemId()) );
            if( x->getNotationName() )
                parent->m_notationName = tostr( x->getNotationName() );

            parent->addAttribute( "notation-name",
                                  parent, &XMLBaseContext::getNotationNameStream,
                                  XSD_BASIC_STRING,
                                  true );
            break;
        }


//     case DOMNode::XML_DECL_NODE:
//     { 
//         DOM_XMLDecl& x = (DOM_XMLDecl&)node;
//         LG_XML_D << "DOMNode::XML_DECL_NODE: Value:" << tostr(Value) << endl;
            
//         parent->Version    = tostr( x.getVersion() );
//         if( tostr(x.getEncoding()).length() )
//         {
//             parent->Encoding   = tostr( x.getEncoding() );
//         }
//         parent->StandAlone = tostr( x.getStandalone() );

//         parent->setStaticEA( "version",     parent->Version );
//         parent->setStaticEA( "encoding",    parent->Encoding );
//         parent->setStaticEA( "stand-alone", parent->StandAlone );
//         break;
//     }

    
        default:
            LG_XML_D << "Unrecognized node type = "
                     << (long)node->getNodeType() << endl; 
        }
    }


    /********************/
    /********************/
    /********************/

//     void
//     handleBadDeclaration( fh_stringstream& ret )
//     {
//         string s;
//         getline( ret, s );
//         LG_XML_D << "first line:" << s << endl;
//         if( starts_with( s, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?><" ))
//         {
//             LG_XML_D << "BAD first line:" << s << endl;
//             fh_stringstream ss;
//             ss << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n";
//             ss << "<";

//             ss << s.substr( strlen("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?><") ) << endl;
//             copy( istreambuf_iterator<char>(ret),
//                   istreambuf_iterator<char>(),
//                   ostreambuf_iterator<char>(ss));
//             ss.clear();
//             ss.seekg(0);
//             ss.clear();
//             LG_XML_D << "BAD first line NEW STREAM sz:" << tostr(ss).size() << endl;
//             LG_XML_D << "BAD first line NEW STREAM doc:" << tostr(ss) << endl;
//             ret = ss;
//         }

//         ret.clear();
//         ret.seekg(0);
//         ret.clear();
//     }
    
    
    fh_stringstream
    XMLBaseContext::real_getIOStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               exception)
    {
        fh_stringstream ss;

//         if( m & ios_base::trunc )
//         {
//             return ss;
//         }
//        cerr << "XMLBaseContext::real_getIOStream() is base:" << ( getBaseContext() == this ) << endl;
        
        if( getBaseContext() == this )
        {
            LG_XML_D << "XMLBaseContext::real_getIOStream() this==base doc:" << toVoid(getDocument()) << endl;
            fh_stringstream ret = tostream( getDocument() );
            LG_XML_D << "XMLBaseContext::real_getIOStream(end) url:" << getURL()
                     << " sz:" << tostr(ret).size() << endl;
            LG_XML_D << "DOCUMENT:" << tostr(ret) << "---END-OF-DOC---" << endl;
//             handleBadDeclaration( ret );
//             LG_XML_D << "XMLBaseContext::real_getIOStream(end) url:" << getURL()
//                      << " sz:" << tostr(ret).size() << endl;
//             LG_XML_D << "DOCUMENT:" << tostr(ret) << "---END-OF-DOC---" << endl;
            return ret;
        }
        else
        {
            LG_XML_D << "XMLBaseContext::real_getIOStream() this!=base" << endl;
            
            DOMElement* element = getElement();

            LG_XML_D << "element:" << element << endl;
            if( element->getNodeName() )
                LG_XML_D << "element name:" << tostr(element->getNodeName()) << endl;
            
            // This doesn't count when path only contains one item
//             if( getDocument()->getDocumentElement() == element )
//             {
//                 cerr << "Serious bug..." << endl;
//                 cerr << "XMLBaseContext::real_getIOStream() is base:" << ( getBaseContext() == this ) << endl;
//                 cerr << "XMLBaseContext::real_getIOStream() url:" << ( getURL() ) << endl;
//                 cerr << "XMLBaseContext::real_getIOStream() root:" << ( getDocument()->getDocumentElement() ) << endl;
//                 cerr << "XMLBaseContext::real_getIOStream() element:" << ( getElement() ) << endl;
//                 cerr << "XMLBaseContext::real_getIOStream() element-name:" << ( ::Ferris::getAttribute( getElement(), "name" ) ) << endl;
//                 cerr << "XMLBaseContext::real_getIOStream() element-url:" << ( ::Ferris::getAttribute( getElement(), "url" ) ) << endl;
//                 {
//                     int i=0;
//                     for( DOMNode* child = getElement()->getFirstChild();
//                          child != 0; child = child->getNextSibling())
//                         ++i;
                    
//                     cerr << "XMLBaseContext::real_getIOStream() children:" << ( i ) << endl;
                    
//                 }
//             }
            
            
            LG_XML_D << "XMLBaseContext::real_getIOStream() url:" << getURL() << "  start walk..." << endl;
            for( DOMNode* child = element->getFirstChild(); child != 0; child = child->getNextSibling())
            {
                LG_XML_D << "XMLBaseContext::real_getIOStream() nodetype:" << child->getNodeType()
                         << " name:" << tostr(child->getNodeName())
                         << endl;
//                 if( tostr(child->getNodeName()) == "text:p" )
//                 {
//                     for( DOMNode* c2 = child->getFirstChild(); c2 != 0; c2 = c2->getNextSibling())
//                     {
//                         LG_XML_D << "XMLBaseContext::real_getIOStream() c2.nodetype:" << c2->getNodeType()
//                                  << " c2.name:" << tostr(c2->getNodeName())
//                                  << endl;
//                         if( c2->getNodeType() == DOMNode::TEXT_NODE )
//                         {
//                             LG_XML_D << "text content:" << tostr(c2->getNodeValue()) << endl;
//                         }
//                     }
//                 }
                
                if( child->getNodeType() == DOMNode::TEXT_NODE )
                {
//                    if( !XMLString::isWSCollapsed( child->getNodeValue() ) )
                    {
                        LG_XML_D << "XMLBaseContext::real_getIOStream() url:" << getURL() << "  text child:"
                                 << tostr(child->getNodeValue()) << ":" << endl;
                        ss << tostr(child->getNodeValue());
                    }
                }
            }
            LG_XML_D << "XMLBaseContext::real_getIOStream() url:" << getURL() << "  end walk..." << endl;
        }
        ss.clear();
        ss.seekp( 0 );
        LG_XML_D << "XMLBaseContext::real_getIOStream(end) url:" << getURL() << " sz:" << tostr(ss).size() << endl;
        return ss;
    
    
//     f_stringstream ret(FileContents);
//     return ss;
    }

    fh_istream
    XMLBaseContext::priv_getIStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               std::exception)
    {
        LG_XSLTFS_D << "XMLBaseContext::priv_getIStream() this:" << toVoid(this)
                    << " url:" << getURL() << endl;
        LG_XML_D << "XMLBaseContext::priv_getIStream() this:" << toVoid(this)
                 << " url:" << getURL() << endl;
        fh_stringstream ret = real_getIOStream( m );
        LG_XML_D << "XMLBaseContext::priv_getIStream() ret.sz:" << tostr(ret).size()
                 << " ret:" << tostr(ret) << "---END-OF-DOC---" << endl;
//         {
//             ofstream o("/tmp/dump111111");
//             o << tostr(ret);
//             o << flush;
//         }
        
        return ret;
    }


    void
    XMLBaseContext::OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
    {
        LG_XML_D << "OnStreamClosed() url:" << getURL() << endl;
        LG_XML_D << "OnStreamClosed() ferris_ios::openmode:" << (long)m << endl;
        LG_XML_D << "OnStreamClosed() ferris_ios::openmode::out:" << (long)std::ios::out << endl;
        LG_XML_D << "OnStreamClosed() ferris_ios::openmode::is-out:" << (long)(m & std::ios::out) << endl;
        if( !(m & std::ios::out) )
            return;
            
        AdjustForOpenMode_Closing( ss, m, tellp );
        const string s = StreamToString(ss);
        LG_XML_D << "OnStreamClosed() len:" << s.length() << " s:" << s << " tellp:" << tellp << endl;

        
//    FileContents = s;

//     cerr << "XMLBaseContext::OnStreamClosed() len:" << s.length()
//          << " looking for txt node" << endl;

//     /* PURE DEBUG */
//     {
//         DOMNodeList* nl =  getElement()->getChildNodes();
//         cerr << "child count:" << nl->getLength();

//         cerr << " name :" << tostr( getElement()->getNodeName() ) << endl;
//         cerr << " value:" << tostr( getElement()->getNodeValue() ) << endl;
//     }

        if( getBaseContext() == this )
        {
            LG_XML_D << "XMLBaseContext::OnStreamClosed() newdoc:" << s << endl;
            
            fh_stringstream tmp;
            tmp << s;
            m_doc = Factory::StreamToDOM( tmp );
            clearContext();
            WrapDOMTree( GetImpl(m_doc), this, "", 0 );
        }
        else
        {
//             {
//                 fh_stringstream ss = tostream( getBaseContext()->m_doc );
//                 LG_XSLTFS_D << "dom before changes.:" << tostr(ss) << endl;
//             }
            
            DOMElement* element = getElement();
            XML::removeAllTextChildren( element );

            LG_XML_D << "OnStreamClosed() removed text children" << endl;
            
            if( !s.empty() )
            {
                DOMText* t = getDocument()->createTextNode( X(s.c_str()) );
                element->appendChild( t );
                LG_XML_D << "OnStreamClosed() added text child for s:" << s << endl;
                syncTree();
                if( shouldPerformFullJournaling() )
                    Factory::getPluginOutOfProcNotificationEngine().signalContextChanged( this, s );
                
                LG_XML_D << "OnStreamClosed() completed changes child for s:" << s << endl;
//                 {
//                     fh_stringstream ss = tostream( getBaseContext()->m_doc );
//                     LG_XSLTFS_D << "dom after changes.:" << tostr(ss) << endl;
//                 }
                
                return;
            }
        
            
//             bool setValue = false;
//             for( DOMNode* child = element->getFirstChild();
//                  child != 0; child = child->getNextSibling())
//             {
//                 if( child->getNodeType() == DOMNode::TEXT_NODE )
//                 {
//                     DOMText* x = (DOMText*)child;

//                     if( setValue )
//                     {
//                         if( !XMLString::isAllWhiteSpace( child->getNodeValue() ) )
//                         {
//                             x->setData( X("") );
//                         }
//                     }
//                     else
//                     {
//                         x->setData( X(s.c_str()) );
//                         setValue = true;
//                     }
//                     cerr << "OnStreamClosed(setting existing node) len:" << s.length() << " s:" << s
//                          << " setValue:" << setValue << endl;
                    
//                 }
//             }
//             cerr << "OnStreamClosed(maybe add?) len:" << s.length() << " s:" << s
//                  << " setValue:" << setValue << endl;

//             if( !setValue )
//             {
//                 DOMText* t = getDocument()->createTextNode( X(s.c_str()) );
//                 element->appendChild( t );
//             }
        }
        
    
        syncTree();
    }


    fh_iostream
    XMLBaseContext::priv_getIOStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               CanNotGetStream,
               std::exception)
    {
        LG_XML_D << "XMLBaseContext::priv_getIOStream()" << endl;
//        BackTrace();
        fh_stringstream ret = real_getIOStream( m );
        ret->getCloseSig().connect( sigc::bind( sigc::mem_fun(*this, &_Self::OnStreamClosed ), m )); 
        return ret;
    }


    /********************/
    /********************/
    /********************/

    void
    XMLBaseContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
        m["dir"] = SubContextCreator(xmlbase_SubCreate_element,
                                     "	<elementType name=\"dir\">\n"
                                     "		<elementType name=\"name\" default=\"new directory\">\n"
                                     "			<dataTypeRef name=\"string\"/>\n"
                                     "		</elementType>\n"
                                     "	</elementType>\n");
        m["element"] = SubContextCreator(xmlbase_SubCreate_element,
                                         "	<elementType name=\"element\">\n"
                                         "		<elementType name=\"name\" default=\"new directory\">\n"
                                         "			<dataTypeRef name=\"string\"/>\n"
                                         "		</elementType>\n"
                                         "		<elementType name=\"content\" default=\"\">\n"
                                         "			<dataTypeRef name=\"string\"/>\n"
                                         "		</elementType>\n"
                                         "	</elementType>\n");
        m["file"] = SubContextCreator(xmlbase_SubCreate_element,
                                      "	<elementType name=\"file\">\n"
                                      "		<elementType name=\"name\" default=\"new directory\">\n"
                                      "			<dataTypeRef name=\"string\"/>\n"
                                      "		</elementType>\n"
                                      "		<elementType name=\"content\" default=\"\">\n"
                                      "			<dataTypeRef name=\"string\"/>\n"
                                      "		</elementType>\n"
                                      "	</elementType>\n");

        m["ea"] = SubContextCreator( xmlbase_SubCreate_attribute,
                                     "	<elementType name=\"ea\">\n"
                                     "		<elementType name=\"name\" default=\"new ea\">\n"
                                     "			<dataTypeRef name=\"string\"/>\n"
                                     "		</elementType>\n"
                                     "		<elementType name=\"value\" default=\"\">\n"
                                     "			<dataTypeRef name=\"string\"/>\n"
                                     "		</elementType>\n"
                                     "		<elementType name=\"namespace\" default=\"\">\n"
                                     "			<dataTypeRef name=\"string\"/>\n"
                                     "		</elementType>\n"
                                     "	</elementType>\n");

        m["attribute"] = SubContextCreator( xmlbase_SubCreate_attribute,
                                            "	<elementType name=\"attribute\">\n"
                                            "		<elementType name=\"name\" default=\"new ea\">\n"
                                            "			<dataTypeRef name=\"string\"/>\n"
                                            "		</elementType>\n"
                                            "		<elementType name=\"value\" default=\"\">\n"
                                            "			<dataTypeRef name=\"string\"/>\n"
                                            "		</elementType>\n"
                                            "		<elementType name=\"namespace\" default=\"\">\n"
                                            "			<dataTypeRef name=\"string\"/>\n"
                                            "		</elementType>\n"
                                            "	</elementType>\n");
    }

    fh_context
    XMLBaseContext::xmlbase_SubCreate_element( fh_context c, fh_context md )
    {
        LG_XML_D << "SL_xml_SubCreate_element() c:" << c->getURL() << endl;

        if( fh_xmlbc xmlc = dynamic_cast<XMLBaseContext*>(GetImpl(c)))
        {
            string rdn     = getStrSubCtx( md, "name", "" );
            string content = getStrSubCtx( md, "content", "" );

            LG_XML_D << "SL_xml_SubCreate_element() c:" << c->getURL()
                     << " rdn:" << rdn
                     << endl;
            
            DOMElement* pe = xmlc->getElement();
            if( !pe )
            {
                if( xmlc->getBaseContext() == GetImpl(c) )
                {
                    fh_stringstream ss;
                    ss << "Can not create more than one root element in an XML document!"
                       << "URL:" << c->getURL()
                       << " child-name:" << rdn
                       << endl;
                    Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
                }
                fh_stringstream ss;
                ss << "Parent element has no XML node!"
                       << "URL:" << c->getURL()
                       << " child-name:" << rdn
                       << endl;
                Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
            }
            DOMElement* ne = xmlc->getDocument()->createElement( X(rdn.c_str()) );
            pe->appendChild( ne );
            if( content.length() )
            {
                DOMText* t = xmlc->getDocument()->createTextNode( X( content.c_str() ) );
                ne->appendChild( t );
            }
//            cerr << "TESTING1!. v:" << ::Ferris::getAttribute( ne, rdn ) << endl;
            
            fh_xmlbc ret = xmlc->ensureCreated( rdn, ne );
//            ret->setElement( ne );
//            ret->FileContents = content;


            
            xmlc->syncTree();

            
            LG_XML_D << "SL_xml_SubCreate_element() fire context created:" << ret->getURL() << endl;
            if( xmlc->shouldPerformFullJournaling() )
                Factory::getPluginOutOfProcNotificationEngine().signalContextCreated( ret, content );
            
            return ret;
        }

        fh_stringstream ss;
        ss << "Attempt to create XML element failed!"
           << " url:" << c->getURL()
           << endl;
        LG_XML_D << tostr(ss);
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
    }

    bool
    XMLBaseContext::shouldPerformFullJournaling()
    {
        return false;
    }
    
    void
    XMLBaseContext::addNewAttribute( const std::string& prefix,
                                     const std::string& rdn, 
                                     XSDBasic_t sct )
    {
        addAttribute( rdn,
                      this, &_Self::getEA,
                      this, &_Self::getEA,
                      this, &_Self::EAUpdated,
                      sct );
    }
    
    fh_context
    XMLBaseContext::xmlbase_SubCreate_attribute(  fh_context c, fh_context md )
    {
        LG_XML_D << "SL_xml_SubCreate_attribute() c:" << c->getURL() << endl;

        if( fh_xmlbc xmlc = dynamic_cast<XMLBaseContext*>(GetImpl(c)))
        {
            string rdn          = getStrSubCtx( md, "name", "" );
            string v            = getStrSubCtx( md, "value", "", true );
            string namespaceURI = getStrSubCtx( md, "namespace", "" );

            LG_XML_D << "SL_xml_SubCreate_attribute() c:" << c->getURL()
                     << " rdn:" << rdn
                     << " v:" << v
                     << endl;
            
            DOMElement* e = xmlc->getElement();
            if( !e )
            {
                LG_ATTR_D << "no element... cc:" << xmlc->getCC()
                          << " this:" << dynamic_cast<Context*>(GetImpl(c))
                          << " cc.url:" << xmlc->getCC()->getURL()
                          << endl;
                
                if( xmlc->getCC() != dynamic_cast<Context*>(GetImpl(c)) )
                {
                    Shell::createEA( xmlc->getCC(), rdn, v, true );
                    return c;
                }

                stringstream ss;
                ss << "Can not create attribute on context c:" << c->getURL() << " EA:" << rdn << endl;
                Throw_FerrisCreateAttributeFailed( ss.str(), 0 );
            }
            
            ::Ferris::setAttributeNS( e, namespaceURI, rdn, v );
//             cerr << "SL_xml_SubCreate_attribute(2) c:" << c->getURL()
//                  << " rdn:" << rdn
//                  << " v:" << v
//                  << endl;

            xmlc->addNewAttribute( namespaceURI,
                                   rdn,
                                   XSD_BASIC_STRING );
//             xmlc->addAttribute( rdn,
//                                 xmlc, &_Self::getEA,
//                                 xmlc, &_Self::getEA,
//                                 xmlc, &_Self::EAUpdated,
//                                 XSD_BASIC_STRING );
//             cerr << "SL_xml_SubCreate_attribute(3) c:" << c->getURL()
//                  << " rdn:" << rdn
//                  << " v:" << v
//                  << endl;

            xmlc->syncTree();

//             cerr << "SL_xml_SubCreate_attribute(4) c:" << c->getURL()
//                  << " rdn:" << rdn
//                  << " v:" << v
//                  << endl;

            if( xmlc->shouldPerformFullJournaling() )
                Factory::getPluginOutOfProcNotificationEngine().signalEACreated( c, rdn );
        }

        return c;
    }
    
    /********************/
    /********************/
    /********************/
    
    

    fh_iostream
    XMLBaseContext::getEA( Context* _c, const std::string& rdn, EA_Atom* atom )
    {
        
        if( _Self* c = dynamic_cast<_Self*>( _c ))
        {
            DOMElement* e = c->getElement();

            LG_DOM_D << "getEA(0) doc:" << getDocument() << endl;
            LG_DOM_D << "getEA(1) c:" << c->getURL()
                     << " rdn:" << rdn
                     << " e:" << toVoid(e)
                     << endl;
            LG_DOM_D << "getEA(2) c:" << c->getURL()
                     << " e:" << toVoid(e)
                     << " e-owner-doc:" << e->getOwnerDocument()
                     << endl;

            if( rdn == "ferris-xml-url" )
            {
                LG_DOM_D << "getEA(3) c:" << c->getURL()
                         << " e:" << toVoid(e)
                         << " e-owner-doc:" << e->getOwnerDocument()
                         << endl;

                if( XMLBaseContext* pc = dynamic_cast<XMLBaseContext*>(c))
                {
                    LG_DOM_D << "getEA(4) c:" << c->getURL()
                             << " e:" << toVoid(e)
                             << " c->v:" << pc->m_attrs["ferris-xml-url"]
                             << endl;
                        fh_stringstream ss;
                        ss << pc->m_attrs["ferris-xml-url"];
                        return ss;
                }
                
                
//                 if( isParentBound() )
//                 {
//                     if( XMLBaseContext* pc = dynamic_cast<XMLBaseContext*>(c->getParent()))
//                     {
//                         LG_DOM_D << "getEA(5) c:" << c->getURL()
//                                  << " e:" << toVoid(e)
//                                  << " pc->v:" << pc->m_attrs["ferris-xml-url"]
//                                  << endl;

//                         fh_stringstream ss;
//                         ss << pc->m_attrs["ferris-xml-url"];
//                         return ss;
//                     }
//                 }
            }

            fh_stringstream ss;
            ss << ::Ferris::getAttribute( e, rdn );
            return ss;
        }
    }


    void
    XMLBaseContext::EAUpdated( Context* _c, const std::string& rdn, EA_Atom* atom, fh_istream iss )
    {
        LG_XML_D << "EAUpdated(1) c:" << _c->getURL() << endl;
        
        if( _Self* c = dynamic_cast<_Self*>( _c ))
        {
            DOMElement* e = c->getElement();

            LG_DOM_D << "EAUpdated(1) c:" << c->getURL()
                     << " e:" << toVoid(e) 
                     << " rdn:" << rdn << endl;
                    
            std::string v = StreamToString( iss );
            ::Ferris::setAttribute( e, rdn, v );

            LG_DOM_D << "EAUpdated(2) c:" << c->getURL()
                     << " e:" << toVoid(e) 
                     << " rdn:" << rdn
                     << " v:" << v
                     << endl;
                    
            c->syncTree();
                    
            LG_DOM_D << "EAUpdated(3) c:" << c->getURL()
                     << " e:" << toVoid(e)
                     << endl;
        }
    }

    
};

