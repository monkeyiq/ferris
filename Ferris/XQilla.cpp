/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2007 Ben Martin

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

    $Id: XQilla.cpp,v 1.20 2010/09/24 21:31:01 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/


#include <Ferris/XQilla_private.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/Hashing.hh>

#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

#include <xqilla/schema/DocumentCache.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/schema/DocumentCacheImpl.hpp>
#include <xqilla/functions/FunctionConstructor.hpp>
#include <xqilla/axis/NodeTest.hpp>

#include <xqilla/functions/FuncFactory.hpp>

#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/exceptions/XMLParseException.hpp>


#include <xqilla/context/impl/XQDynamicContextImpl.hpp>
#include <xqilla/xerces/XercesConfiguration.hpp>
#include <xqilla/context/UpdateFactory.hpp>
#include <xqilla/context/URIResolver.hpp>
#include <xqilla/events/SequenceBuilder.hpp>
#include <xqilla/update/PendingUpdateList.hpp>

using namespace XERCES_CPP_NAMESPACE;
using namespace std;

bool PERFORM_UPDATES = true;



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// These are from xqilla internal header files :/
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


class XercesSequenceBuilder : public SequenceBuilder
{
public:
  XercesSequenceBuilder(const DynamicContext *context);
  virtual ~XercesSequenceBuilder();

  virtual void startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding);
  virtual void endDocumentEvent();
  virtual void startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname);
  virtual void endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                               const XMLCh *typeURI, const XMLCh *typeName);
  virtual void piEvent(const XMLCh *target, const XMLCh *value);
  virtual void textEvent(const XMLCh *value);
  virtual void textEvent(const XMLCh *chars, unsigned int length);
  virtual void commentEvent(const XMLCh *value);
  virtual void attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                              const XMLCh *typeURI, const XMLCh *typeName);
  virtual void namespaceEvent(const XMLCh *prefix, const XMLCh *uri);
  virtual void atomicItemEvent(AnyAtomicType::AtomicObjectType type, const XMLCh *value, const XMLCh *typeURI,
                               const XMLCh *typeName);
  virtual void endEvent();

  virtual Sequence getSequence() const { return seq_; }

  static void setElementTypeInfo(XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *element, const XMLCh *typeURI, const XMLCh *typeName);
  static void setAttributeTypeInfo(XERCES_CPP_NAMESPACE_QUALIFIER DOMAttr *attr, const XMLCh *typeURI, const XMLCh *typeName);

private:
  const DynamicContext *context_;
  XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *document_;
  XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *currentParent_;
  XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *currentNode_;
  Sequence seq_;
};

/// Stores and manages all the information for a Fast XDM document
class XQILLA_API FastXDMDocument : public EventHandler, public ReferenceCounted
{
public:
  typedef RefCountPointer<FastXDMDocument> Ptr;

  enum NodeKind {
    DOCUMENT,
    ELEMENT,
    TEXT,
    COMMENT,
    PROCESSING_INSTRUCTION,
    MARKER
  };

  struct Node;

  struct Attribute {
    void set(unsigned int oi, const XMLCh *p, const XMLCh *u, const XMLCh *l, const XMLCh *v, const XMLCh *tu, const XMLCh *tn)
    {
      owner.index = oi; prefix = p; uri = u; localname = l; value = v; typeURI = tu; typeName = tn;
    }
    void setMarker()
    {
      owner.index = (unsigned int)-1; prefix = 0; uri = 0; localname = 0; value = 0;
    }

    union {
      unsigned int index;
      Node *ptr;
    } owner;

    const XMLCh *prefix;
    const XMLCh *uri;
    const XMLCh *localname;

    const XMLCh *value;

    const XMLCh *typeURI;
    const XMLCh *typeName;
  };

  struct Namespace {
    void set(unsigned int oi, const XMLCh *p, const XMLCh *u)
    {
      owner.index = oi; prefix = p; uri = u;
    }

    union {
      unsigned int index;
      Node *ptr;
    } owner;

    const XMLCh *prefix;
    const XMLCh *uri;
  };

  struct Node {
    void setDocument(const XMLCh *d, const XMLCh *e)
    {
      nodeKind = DOCUMENT; level = 0; nextSibling.index = (unsigned int)-1; data.document.documentURI = d;
      data.document.encoding = e;
    }
    void setElement(unsigned int lv, const XMLCh *p, const XMLCh *u, const XMLCh *l)
    {
      nodeKind = ELEMENT; level = lv; nextSibling.index = (unsigned int)-1;
      data.element.attributes.index = (unsigned int)-1; data.element.namespaces.index = (unsigned int)-1;
      data.element.prefix = p; data.element.uri = u; data.element.localname = l;
    }
    void setElementType(const XMLCh *tu, const XMLCh *tn)
    {
      data.element.typeURI = tu; data.element.typeName = tn;
    }
    void setPI(unsigned int lv, const XMLCh *t, const XMLCh *v)
    {
      nodeKind = PROCESSING_INSTRUCTION; level = lv; nextSibling.index = (unsigned int)-1; data.other.target = t;
      data.other.value = v;
    }
    void setText(unsigned int lv, const XMLCh *v)
    {
      nodeKind = TEXT; level = lv; nextSibling.index = (unsigned int)-1; data.other.target = 0; data.other.value = v;
    }
    void setComment(unsigned int lv, const XMLCh *v)
    {
      nodeKind = COMMENT; level = lv; nextSibling.index = (unsigned int)-1; data.other.target = 0; data.other.value = v;
    }
    void setMarker()
    {
      nodeKind = MARKER; level = 0; nextSibling.index = (unsigned int)-1;
    }

    NodeKind nodeKind;
    unsigned int level;

    union {
      unsigned int index;
      Node *ptr;
    } nextSibling;

    union {
      struct {
        const XMLCh *documentURI;
        const XMLCh *encoding;
      } document;

      struct {
        union {
          unsigned int index;
          Attribute *ptr;
        } attributes;

        union {
          unsigned int index;
          Namespace *ptr;
        } namespaces;

        const XMLCh *prefix;
        const XMLCh *uri;
        const XMLCh *localname;

        const XMLCh *typeURI;
        const XMLCh *typeName;
      } element;

      struct {
        const XMLCh *target;
        const XMLCh *value;
      } other;

    } data;

  };

  FastXDMDocument(XPath2MemoryManager *mm);
  FastXDMDocument(unsigned int numNodes, unsigned int numAttributes, unsigned int numNamespaces, XPath2MemoryManager *mm);
  virtual ~FastXDMDocument();

  virtual void startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding);
  virtual void endDocumentEvent();
  virtual void startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname);
  virtual void endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                               const XMLCh *typeURI, const XMLCh *typeName);
  virtual void piEvent(const XMLCh *target, const XMLCh *value);
  virtual void textEvent(const XMLCh *value);
  virtual void textEvent(const XMLCh *chars, unsigned int length);
  virtual void commentEvent(const XMLCh *value);
  virtual void attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                              const XMLCh *typeURI, const XMLCh *typeName);
  virtual void namespaceEvent(const XMLCh *prefix, const XMLCh *uri);
  virtual void endEvent();

  Node *getNode(unsigned int i);
  const Node *getNode(unsigned int i) const;
  unsigned int getNumNodes() const { return numNodes_; }

  Attribute *getAttribute(unsigned int i);
  const Attribute *getAttribute(unsigned int i) const;
  unsigned int getNumAttributes() const { return numAttributes_; }

  Namespace *getNamespace(unsigned int i);
  const Namespace *getNamespace(unsigned int i) const;
  unsigned int getNumNamespaces() const { return numNamespaces_; }

  static const Node *getParent(const Node *node);

  static void toEvents(const Node *node, EventHandler *events, bool preserveNS = true, bool preserveType = true);
  static void toEvents(const Attribute *attr, EventHandler *events, bool preserveType = true);
  static void toEvents(const Namespace *ns, EventHandler *events);

private:
  void resizeNodes();
  void resizeAttributes();
  void resizeNamespaces();

  XERCES_CPP_NAMESPACE_QUALIFIER ValueStackOf<unsigned int> elementStack_;
  unsigned int prevNode_;

  XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer textBuffer_;
  bool textToCreate_;

  Node *nodes_;
  unsigned int numNodes_;
  unsigned int maxNodes_;

  Attribute *attributes_;
  unsigned int numAttributes_;
  unsigned int maxAttributes_;

  Namespace *namespaces_;
  unsigned int numNamespaces_;
  unsigned int maxNamespaces_;

  XPath2MemoryManager *mm_;
};

inline const FastXDMDocument::Node *FastXDMDocument::getParent(const Node *node)
{
  unsigned int level = node->level;

  if(level == 0) return 0;

  do {
    --node;
  } while(node->level >= level);

  return node;
}

class FastXDMSequenceBuilder : public SequenceBuilder
{
public:
  FastXDMSequenceBuilder(const DynamicContext *context);

  virtual void startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding);
  virtual void endDocumentEvent();
  virtual void startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname);
  virtual void endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                               const XMLCh *typeURI, const XMLCh *typeName);
  virtual void piEvent(const XMLCh *target, const XMLCh *value);
  virtual void textEvent(const XMLCh *value);
  virtual void textEvent(const XMLCh *chars, unsigned int length);
  virtual void commentEvent(const XMLCh *value);
  virtual void attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                              const XMLCh *typeURI, const XMLCh *typeName);
  virtual void namespaceEvent(const XMLCh *prefix, const XMLCh *uri);
  virtual void atomicItemEvent(AnyAtomicType::AtomicObjectType type, const XMLCh *value, const XMLCh *typeURI,
                               const XMLCh *typeName);
  virtual void endEvent();

  virtual Sequence getSequence() const { return seq_; }

private:
  const DynamicContext *context_;
  unsigned int level_;
  FastXDMDocument::Ptr document_;
  Sequence seq_;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Ferris
{
    namespace qilla
    {
        void copyChildren( const DynamicContext* dc,
                           DOMDocument* document, fh_context c, DOMElement* target );
        
        std::string
        canonicalElementName( std::string rdn )
        {
            LG_XQILLA_D << "canonicalElementName(top) rdn:" << rdn << endl;
            
            string nn = rdn;
            if( starts_with( nn, "." ))
                nn = (string)"_" + nn;
            nn = Util::replace_all( nn, " ", "_space_" );
            nn = Util::replace_all( nn, "+", "_plus_" );
            nn = Util::replace_all( nn, "(", "_oper_" );
            nn = Util::replace_all( nn, ")", "_cper_" );
            nn = Util::replace_all( nn, ",", "_comma_" );
            nn = Util::replace_all( nn, "-", "_dash_" );
            nn = Util::replace_all( nn, ":", "_colon_" );

            static const boost::regex r = toregex( "^[0-9]" );
            if( regex_search( nn, r, boost::match_any ) )
            {
                nn = "number_" + nn;
            }
            

            
            LG_XQILLA_D << "canonicalElementName(bottom) rdn:" << nn << endl;
            return nn;
        }
        std::string
        canonicalElementName( fh_context c )
        {
            return canonicalElementName( c->getDirName() );
        }


        /////////////////////////////////////////
        /////////////////////////////////////////
        /////////////////////////////////////////
        /////////////////////////////////////////
        
            
        ShouldIncludeAttribute::ShouldIncludeAttribute()
            :
            m_showColumnsRegexString("")
        {
        }
            
        bool
        ShouldIncludeAttribute::nameContainsIllegalXMLAttributeChar( const std::string& eaname )
        {
            LG_XQILLA_D << "nameContainsIllegalXMLAttributeChar() eaname:" << eaname << endl;
            static const boost::regex r = toregex( "[-A-Za-z0-9_]+" );
            return( !regex_match( eaname, r, boost::match_any ) );
        }
        

            
        void
        ShouldIncludeAttribute::setShowColumns( const std::string& s )
        {
            Util::parseCommaSeperatedList( s, m_showColumnsList );
        }
            
        void
        ShouldIncludeAttribute::setShowColumnsRegex( const std::string& s )
        {
            m_showColumnsRegexString = s;
            m_showColumnsRegex       = toregex( s );
        }
            
        bool
        ShouldIncludeAttribute::shouldInclude( fh_context c, const std::string& eaname )
        {
            
            if( m_showColumnsList.count( eaname ) )
            {
                if( nameContainsIllegalXMLAttributeChar( eaname ) )
                    return false;
                return true;
            }

            if( !m_showColumnsRegexString.empty()
                && regex_search( eaname, m_showColumnsRegex, boost::match_any ))
            {
                if( nameContainsIllegalXMLAttributeChar( eaname ) )
                    return false;
                return true;
            }

            return false;
        }
            

        typedef map< const DynamicContext*, fh_shouldIncludeAttribute > getShouldIncludeAttribute_cache_t;
        getShouldIncludeAttribute_cache_t& getShouldIncludeAttribute_cache()
        {
            static getShouldIncludeAttribute_cache_t ret;
            return ret;
        }
        
        
        fh_shouldIncludeAttribute getShouldIncludeAttribute( const DynamicContext* context )
        {
            getShouldIncludeAttribute_cache_t::iterator iter = getShouldIncludeAttribute_cache().find( context );
            if( iter != getShouldIncludeAttribute_cache().end() )
            {
                return iter->second;
            }
            fh_shouldIncludeAttribute n = new ShouldIncludeAttribute();
            n->setShowColumnsRegex(".*");
            getShouldIncludeAttribute_cache().insert( make_pair( context, n ) );
            return getShouldIncludeAttribute_cache()[ context ];
        }
        
        void setShouldIncludeAttribute( DynamicContext* context, fh_shouldIncludeAttribute sia )
        {
            getShouldIncludeAttribute_cache()[ context ] = sia;
        }
        
        

        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        
        class FQilla_ResultImpl : public ResultImpl
        {
        protected:
            fh_context m_ctx;
            const NodeTest* m_nodeTest;
            string m_NodeNameWildcard;
            string m_NodeNamespaceURIWildcard;
            SequenceType::ItemType* m_itemType;
            boost::regex m_NodeNameWildcardRegex;
            
        
            bool haveItemType()
                {
                    return m_itemType;
                }
        
            Item::Ptr handleNodeTestForItemType( DynamicContext *dynamicContext, fh_context ctx, const std::string& eaname )
                {
                    SequenceType::ItemType *itemType = m_nodeTest->getItemType();
                    if(itemType != 0)
                    {
//                    Node::Ptr result = factory_.createNode(node, dynamicContext);
                        Node::Ptr result = new FQilla_Node( dynamicContext, ctx, eaname );
                        if(itemType->matches(result, dynamicContext)) {
                            return result;
                        }
                    }
                    return 0;
                }

            bool passesElementTests( fh_context ctx )
                {
                    LG_XQILLA_D << "passesElementTests(1) ctx:" << ctx->getURL() << endl;
                    LG_XQILLA_D << "passesElementTests(1.b) have name wildcard:" << (m_nodeTest->getNameWildcard()!=0) << endl;
                    LG_XQILLA_D << "passesElementTests(1.b) have type wildcard:" << (m_nodeTest->getTypeWildcard()!=0) << endl;

                    if( !m_nodeTest->getTypeWildcard()
                       && m_nodeTest->getNodeType() != Node::element_string 
                       && m_nodeTest->getNodeType() != Node::text_string )
                    {
                        LG_XQILLA_D << "passesElementTests(1R) ctx:" << ctx->getURL() << endl;
                        return false;
                    }
                    
                    LG_XQILLA_D << "passesElementTests(2) ctx:" << ctx->getURL() << endl;
                    LG_XQILLA_D << "m_NodeNameWildcard:" << m_NodeNameWildcard << endl;
                    std::string localName;
                    if( !m_NodeNameWildcard.empty() )//|| !m_NodeNameWildcard.empty() )
                        localName = canonicalElementName( ctx );
                    
                    if( !m_NodeNameWildcard.empty() &&
                        !regex_match( localName, m_NodeNameWildcardRegex, boost::match_any ))
                    {
                        LG_XQILLA_D << "passesElementTests(2-bad) ctx:" << ctx->getURL()
                                    << " localName:" << localName
                                    << " m_NodeNameWildcard:" << m_NodeNameWildcard
                                    << endl;
                        return false;
                    }

//                     std::string localName = canonicalElementName( ctx );
//                     if( !m_NodeNameWildcard.empty() && m_NodeNameWildcard != localName )
//                     {
//                         return false;
//                     }
                    
                    LG_XQILLA_D << "passesElementTests(3) ctx:" << ctx->getURL() << endl;
                    if( !m_NodeNamespaceURIWildcard.empty() && m_NodeNamespaceURIWildcard != "FIXME" )
                    {
                        return false;
                    }

                    LG_XQILLA_D << "passesElementTests(ok) ctx:" << ctx->getURL() << endl;
                    return true;
                }
        
        
        public:
            FQilla_ResultImpl( const LocationInfo *o,
                               const NodeTest* nodeTest,
                               fh_context ctx )
                :
                ResultImpl( o ),
                m_nodeTest( nodeTest ),
                m_ctx( ctx ),
                m_itemType( 0 ),
                m_NodeNameWildcard(""),
                m_NodeNamespaceURIWildcard("")
                {
                    if( m_nodeTest )
                    {
                        LG_XQILLA_D << "FQilla_ResultImpl() have nodeTest" << endl;
                        m_itemType = m_nodeTest->getItemType();

                        if(!m_nodeTest->getNameWildcard() && !tostr(m_nodeTest->getNodeName()).empty())
                        {
                            m_NodeNameWildcard = tostr(m_nodeTest->getNodeName());
                            m_NodeNameWildcardRegex = toregex( m_NodeNameWildcard + "(--[0-9]+)?" );
                            LG_XQILLA_D << "FQilla_ResultImpl() m_NodeNameWildcard:" << m_NodeNameWildcard << endl;
                        }
                        
                        if(!m_nodeTest->getNamespaceWildcard())
                            m_NodeNamespaceURIWildcard = tostr(m_nodeTest->getNodeUri());
                    }
                }
            virtual ~FQilla_ResultImpl()
                {
                }
        
//        virtual Item::Ptr next(DynamicContext *context);
            virtual std::string asString(DynamicContext *context, int indent) const
                {
                    LG_XQILLA_W << "asString() called but not implemented!" << endl;
                }
        };


        class FQilla_Attr_ResultImpl : public FQilla_ResultImpl
        {
            AttributeCollection::AttributeNames_t an;
            AttributeCollection::AttributeNames_t::iterator b;
            AttributeCollection::AttributeNames_t::iterator e;
            AttributeCollection::AttributeNames_t::iterator iter;
            string m_singleAttribute;
            
        public:
            FQilla_Attr_ResultImpl( const LocationInfo *o,
                                    const NodeTest* nodeTest,
                                    fh_context ctx )
                :
                FQilla_ResultImpl( o,
                                   (nodeTest && nodeTest->getNodeName()) ? 0 : nodeTest, ctx )
                {
                    bool haveSetup = false;
                    
                    LG_XQILLA_D << "FQilla_Attr_ResultImpl() nodeTest:" << toVoid(nodeTest) << endl;
                    if( nodeTest && nodeTest->getNodeName() )
                    {
                        LG_XQILLA_D << "node-type==attr:" << (nodeTest->getNodeType() == Node::attribute_string) << endl;
                        LG_XQILLA_D << "node.name:" << tostr(nodeTest->getNodeName()) << endl;

                        string eaname = tostr(nodeTest->getNodeName());
                        b = e = an.end();
                        iter = e;
                        haveSetup = true;
                        m_singleAttribute = eaname;
                    }

                    if( !haveSetup )
                    {
                        ctx->getAttributeNames( an );
                        b = an.begin();
                        e = an.end();
                        iter = an.begin();
                    }
                }
            virtual ~FQilla_Attr_ResultImpl()
                {
                }
            virtual Item::Ptr next( DynamicContext* dynamicContext )
                {
                    LG_XQILLA_D << "FQilla_Attr_ResultImpl::next() this:" << toVoid(this) << endl;
                    LG_XQILLA_D << "FQilla_Attr_ResultImpl::next() m_singleAttribute:" << m_singleAttribute << endl;

                    if( !m_singleAttribute.empty() )
                    {
                        LG_XQILLA_D << "m_singleAttribute:" << m_singleAttribute << endl;
                        string eaname = m_singleAttribute;
                        m_singleAttribute = "";
                        FQilla_Node* ret = new FQilla_Node( dynamicContext, m_ctx, eaname );
                        return ret;
                    }
                    
                    while( iter != e )
                    {
                        string eaname = *iter;
                        ++iter;
                        LG_XQILLA_D << "FQilla_Attr_ResultImpl::next() ea:" << eaname << endl;

                        if( haveItemType() )
                        {
                            LG_XQILLA_D << "FQilla_Attr_ResultImpl::next() haveItemType..." << endl;
                            return handleNodeTestForItemType( dynamicContext, m_ctx, eaname );
                        }
                    

                        if(m_nodeTest->getTypeWildcard())
                        {
                            if(m_nodeTest->getHasChildren())
                                continue;
                        }
                        else
                        {
                            if(m_nodeTest->getNodeType() != Node::attribute_string)
                            {
                                continue;
                            }
                        }

                        string localName = eaname;
                        if( !m_NodeNameWildcard.empty() && m_NodeNameWildcard != localName )
                        {
                            LG_XQILLA_D << "FQilla_Attr_ResultImpl::next() ln-bad m_NodeNameWildcard:" << m_NodeNameWildcard << "  ea:" << eaname << endl;
                            LG_XQILLA_D << "an.sz:" << an.size() << endl;
                            
                            if( !contains(m_NodeNameWildcard,"*") )
                            {
                                AttributeCollection::AttributeNames_t::iterator t = iter;
                                for( ; t!=e; ++t )
                                {
//                                    LG_XQILLA_D << "FQilla_Attr_ResultImpl::next() AA t:" << *t << endl;
                                    if( *t == m_NodeNameWildcard )
                                    {
                                        LG_XQILLA_D << "FQilla_Attr_ResultImpl::next() skipping right to it.. t:" << *t
                                                    << " url:" << m_ctx->getURL()
                                                    << endl;
                                        iter = t;
                                        eaname = *iter;
                                        ++iter;
                                        LG_XQILLA_D << "FQilla_Attr_ResultImpl::next() skipping right to it.. eaname:" << eaname
                                                    << " url:" << m_ctx->getURL()
                                                    << endl;
                                        
                                        return new FQilla_Node( dynamicContext, m_ctx, eaname );
//                                    continue;
                                    }
                                }
                                LG_XQILLA_D << "FQilla_Attr_ResultImpl::next() NOT FOUND! url:" << m_ctx->getURL() << endl
                                            <<  " m_NodeNameWildcard:" << m_NodeNameWildcard << "  ea:" << eaname << endl;
                                return 0;
                            }
                            

//                             AttributeCollection::AttributeNames_t::iterator t = find( an.begin(), e, m_NodeNameWildcard );
//                             if( t != e )
//                             {
//                                 LG_XQILLA_D << "FQilla_Attr_ResultImpl::next() skipping right to it... t:" << *t << endl;
//                                 iter = t;
//                                 continue;
//                             }
                            
//                        if( m_ctx->isAttributeBound( m_NodeNameWildcard ) )
                            {
                                // FIXME:
                                // skip ahead to the attribute, need to update iter to be the correct attr as well
                            }
                            continue;
                        }
                        LG_XQILLA_D << "FQilla_Attr_ResultImpl::next() ln-ok m_NodeNameWildcard:" << m_NodeNameWildcard << "  ea:" << eaname << endl;
                        if( !m_NodeNamespaceURIWildcard.empty() && m_NodeNamespaceURIWildcard != "FIXME" )
                            continue;

                        LG_XQILLA_D << "FQilla_Attr_ResultImpl::next() MATCH  ea:" << eaname
                                    << " m_ctx:" << m_ctx->getURL()
                                    << endl;
//                    return new FQilla_Node( m_ctx, "" );
                        return new FQilla_Node( dynamicContext, m_ctx, eaname );
                    }
                    return 0;
                }
        };

        template< class IteratorType >
        class FQilla_Iterator_ResultImpl : public FQilla_ResultImpl
        {
            string eaname;
            IteratorType m_iter;
            IteratorType m_begin;
            IteratorType m_end;
            ctxlist_t* m_ctxlist;
            bool selectingSingleNodeWithWildCard;
        
        public:
            FQilla_Iterator_ResultImpl( const LocationInfo *o,
                                        const NodeTest *nodeTest,
                                        fh_context ctx,
                                        IteratorType begin,
                                        IteratorType end,
                                        ctxlist_t* cl = 0 )
                :
                FQilla_ResultImpl( o, nodeTest, ctx ),
                m_iter( begin ),
                m_begin( begin ),
                m_end( end ),
                m_ctxlist( cl ),
                selectingSingleNodeWithWildCard( false )
                {
                    LG_XQILLA_D << "FQilla_Iterator_ResultImpl()" << endl;
                    
//                 if( !m_NodeNameWildcard.empty() )
//                 {
//                     Context::iterator ci = ctx->find( m_NodeNameWildcard );
//                     if( ci != m_end )
//                     {
//                         m_iter = ci;
//                         selectingSingleNodeWithWildCard = true;
//                     }
//                 }
                }
            virtual ~FQilla_Iterator_ResultImpl()
                {
                    if( m_ctxlist )
                    {
                        m_ctxlist->clear();
                        delete m_ctxlist;
                    }
                }

        
            virtual Item::Ptr next(DynamicContext *dynamicContext)
                {
                    LG_XQILLA_D << "FQilla_Iterator_ResultImpl::next()" << endl;
                    LG_XQILLA_D << "FQilla_Iterator_ResultImpl::next() m_nodeTest:" << m_nodeTest << endl;

                    if(!m_nodeTest->getTypeWildcard())
                    {
                        LG_XQILLA_D << "NT:" << tostr(m_nodeTest->getNodeType()) << endl;
                    }
                    if(!m_nodeTest->getNameWildcard())
                    {
                        LG_XQILLA_D << "NN:" << tostr(m_nodeTest->getNodeName()) << endl;
                    }


                    while( m_iter != m_end )
                    {
                        fh_context ctx = *m_iter;
                        ++m_iter;
                        LG_XQILLA_D << "FQilla_Iterator_ResultImpl::next() ctx:" << ctx->getURL() << endl;
//                        LG_XQILLA_D << "canonicalElementName( ctx ):" << canonicalElementName( ctx ) << endl;

                        if( selectingSingleNodeWithWildCard )
                        {
                            LG_XQILLA_D << "FQilla_Iterator_ResultImpl::next() selecting single via wildcard..." << endl;
                            m_iter = m_end;
                        }
                    

                        if( haveItemType() )
                            return handleNodeTestForItemType( dynamicContext, ctx, "" );

                        LG_XQILLA_D << "FQilla_Iterator_ResultImpl::next() calling passesElementTests" << endl;
                        if( !passesElementTests( ctx ) )
                        {
                            continue;
                        }
                    
                    
//                     if(!m_nodeTest->getTypeWildcard()
//                        && m_nodeTest->getNodeType() != Node::element_string)
//                     {
//                         continue;
//                     }
                    
//                     std::string localName = canonicalElementName( ctx );
//                     if( !m_NodeNameWildcard.empty() && m_NodeNameWildcard != localName )
//                     {
//                         continue;
//                     }

//                     if( !m_NodeNamespaceURIWildcard.empty() && m_NodeNamespaceURIWildcard != "FIXME" )
//                     {
//                         continue;
//                     }

                        LG_XQILLA_D << "FQilla_Iterator_ResultImpl::next() ok ctx:" << ctx->getURL() << endl;
                        return new FQilla_Node( dynamicContext, ctx );
                    }
                    return 0;
                }
        
        };


        class FQilla_SingleNode_ResultImpl : public FQilla_ResultImpl
        {
            const FQilla_Node* m_node;
            const FQilla_Node* m_iter;

        public:
            FQilla_SingleNode_ResultImpl( const LocationInfo *o,
                                          const NodeTest *nodeTest,
                                          fh_context ctx,
                                          const Node* n )
                :
                FQilla_ResultImpl( o, nodeTest, ctx ),
                m_node( (FQilla_Node*)n ),
                m_iter( (FQilla_Node*)n )
                {
                }
        
            virtual ~FQilla_SingleNode_ResultImpl()
                {
                }
        
            virtual Item::Ptr next(DynamicContext* dynamicContext)
                {
                    LG_XQILLA_D << "FQilla_SingleNode_ResultImpl::next()" << endl;
                    if( !m_iter )
                        return 0;
                    
                    LG_XQILLA_D << "FQilla_SingleNode_ResultImpl::next(2)" << endl;
                    const FQilla_Node* ret = m_iter;
                    m_iter = 0;

                    fh_context ctx = ret->getFerrisContext();
                    LG_XQILLA_D << "FQilla_SingleNode_ResultImpl::next(3)" << endl;
                    LG_XQILLA_D << "FQilla_SingleNode_ResultImpl::next(4) ctx:" << ctx->getURL() << endl;
                
                    if( haveItemType() )
                        return handleNodeTestForItemType( dynamicContext, ctx, "" );
                    LG_XQILLA_D << "FQilla_SingleNode_ResultImpl::next(5)" << endl;

                    if( !passesElementTests( ctx ) )
                        return 0;
                    LG_XQILLA_D << "FQilla_SingleNode_ResultImpl::next(6)" << endl;
                
                    return ret;
                }
        };
    

    
    
    
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
    
        struct fr_hash : public std::unary_function< pair< const DynamicContext*, Context* const >, size_t >
        {
            inline size_t operator()( const pair< const DynamicContext*, Context* const >& s ) const
                {
                    return GPOINTER_TO_INT( s.first ) | GPOINTER_TO_INT( s.second );
                }
        };
        struct fr_equal_to
            : public std::binary_function<
            pair< const DynamicContext*, Context* const >,
            pair< const DynamicContext*, Context* const >, bool> 
        {
            inline bool operator()(const pair< const DynamicContext*, Context* const >& __x,
                                   const pair< const DynamicContext*, Context* const >& __y) const
                {
                    return __x.first == __y.first && __x.second == __y.second;
                }
        };
    
        typedef FERRIS_STD_HASH_MAP<
            pair< const DynamicContext*, Context* const >,
            bool,
            fr_hash, fr_equal_to > m_fqilla_fakeroots_t;
        m_fqilla_fakeroots_t& get_fqilla_fakeroots()
        {
            static m_fqilla_fakeroots_t m_fqilla_fakeroots;
            return m_fqilla_fakeroots;
        }
    

        const XMLCh fastxdm_string[] = { 'f', 'a', 's', 't', 'x', 'd', 'm', 0 };
    
        const XMLCh FQilla_Node::fQilla[] =   // Points to "fQilla"
        {
            XERCES_CPP_NAMESPACE_QUALIFIER chLatin_F,
            XERCES_CPP_NAMESPACE_QUALIFIER chLatin_Q,
            XERCES_CPP_NAMESPACE_QUALIFIER chLatin_i,
            XERCES_CPP_NAMESPACE_QUALIFIER chLatin_l,
            XERCES_CPP_NAMESPACE_QUALIFIER chLatin_l,
            XERCES_CPP_NAMESPACE_QUALIFIER chLatin_a,
            XERCES_CPP_NAMESPACE_QUALIFIER chNull
        };
    
        FQilla_Node::FQilla_Node( const DynamicContext* dynamicContext,
                                  fh_context ctx,
                                  const std::string& eaname,
                                  bool forceNoParent,
                                  bool isSingleTextContentsNode )
            :
            m_ctx( ctx ),
            m_eaname( eaname ),
            m_fSerializer(0),
            m_forceNoParent( forceNoParent ),
            m_isSingleTextContentsNode( isSingleTextContentsNode )
        {
            if( forceNoParent )
            {
                get_fqilla_fakeroots().insert(
                    make_pair( make_pair( dynamicContext, GetImpl(m_ctx) ), 1 )
                    );
            }
        }
        
    
        FQilla_Node::~FQilla_Node()
        {
            if(m_fSerializer != 0)
            {
                m_fSerializer->release();
            }
        }

        void *
        FQilla_Node::getInterface(const XMLCh *name) const
        {
            LG_XQILLA_D << "FQilla_Node::getInterface()" << endl;
            
            if(XPath2Utils::equals(name,FQilla_Node::fQilla))
            {
                return (void*)this;
            }
            else if(XPath2Utils::equals( name, XercesConfiguration::gXerces ))
            {
                bool hideXMLAttribute = true;
                bool hideEmblemAttributes = true;
                bool hideSchemaAttributes = true;
            
                fh_domdoc dom = Factory::makeDOM( m_ctx,
                                                  hideXMLAttribute,
                                                  hideEmblemAttributes,
                                                  hideSchemaAttributes );

                cacheDOM( dom );
                return dom->getDocumentElement();
            }
            return 0;
        }

        void
        FQilla_Node::cacheDOM( fh_domdoc d ) const
        {
            dom_cache.push_back( d );
        }
    
    
    
        bool
        FQilla_Node::isNode() const
        {
            LG_XQILLA_D << "FQilla_Node::isNode()" << endl;
//        return m_eaname.empty();
            return true;
        }
    
        bool
        FQilla_Node::isAtomicValue() const
        {
            LG_XQILLA_D << "FQilla_Node::isAtomicValue()" << endl;
            return false;
        }

//#define X(str) XStr(str).unicodeForm()
#define X(str) XStr(str).unicodeForm()

        const XMLCh*
        FQilla_Node::asString(const DynamicContext* dynamicContext) const
        {
            LG_XQILLA_D << "FQilla_Node::asString()" << endl;
            LG_XQILLA_D << "FQilla_Node::asString()  ctx:" << m_ctx->getURL() << endl;
//        dmNodeName( dynamicContext );
            
            {
                
                fh_domdoc dom = Factory::makeDOM( "libferris" );
                DOMElement* parent = dom->getDocumentElement();
                copyChildren( dynamicContext, GetImpl(dom), m_ctx, parent );
                DOMNode* n = dom->getDocumentElement();
                XML::domnode_list_t nl;
                XML::getChildren( nl, (DOMElement*)n );
                if( nl.size() == 1 )
                {
                    n = nl.front();
                
                    fh_stringstream iss = tostream( *n, true );
                    static string s = StreamToString( iss );
//                    cout << s << endl;
                    return X(s.c_str());
                }

                return 0;
            }
            


            

//         stringstream ss;
//         ss << "<context url=\"" << m_ctx->getURL() << "\">" << endl;
//         ss << "</context>";
//         static string s;
//         s = ss.str();
//         return X(s.c_str());


        
            if(!m_fSerializer)
            {
                const static XMLCh ls_string[] =
                    {
                        XERCES_CPP_NAMESPACE_QUALIFIER chLatin_L,
                        XERCES_CPP_NAMESPACE_QUALIFIER chLatin_S,
                        XERCES_CPP_NAMESPACE_QUALIFIER chNull
                    };
            
                DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(ls_string);

                m_fSerializer = ((DOMImplementationLS*)impl)->createLSSerializer(); 
                DOMConfiguration* dc = m_fSerializer->getDomConfig();
                dc->setParameter(XMLUni::fgDOMXMLDeclaration, false);

                // m_fSerializer =
                //     ((DOMImplementationLS*)impl)->createDOMWriter( dynamicContext->getMemoryManager());
                // m_fSerializer->setFeature(XMLUni::fgDOMXMLDeclaration, false);
            }

            fh_domdoc dom = Factory::makeDOM( "root" );
            DOMElement* de = XML::createElement( dom, dom->getDocumentElement(), "context" );
            setAttribute( de, "url", m_ctx->getURL() );
        
        
//         bool hideXMLAttribute = true;
//         bool hideEmblemAttributes = true;
//         bool hideSchemaAttributes = true;
        
//         fh_domdoc dom = Factory::makeDOM( m_ctx,
//                                           hideXMLAttribute,
//                                           hideEmblemAttributes,
//                                           hideSchemaAttributes );

            DOMNode* n = dom->getDocumentElement();
            return m_fSerializer->writeToString( n, 0 );
        }
    
        bool FQilla_Node::hasInstanceOfType(const XMLCh* typeURI, const XMLCh* typeName, const DynamicContext* dynamicContext) const
        {
            LG_XQILLA_D << "hasInstanceOfType() typeURI:" << tostr(typeURI)
                        << " typeName:" << tostr(typeName)
                        << " dc:" << toVoid(dynamicContext)
                        << endl;
            return false;
        }
    
        Sequence FQilla_Node::dmBaseURI(const DynamicContext* dynamicContext) const
        {
            LG_XQILLA_D << "dmBaseURI()"
                        << " dc:" << toVoid(dynamicContext)
                        << endl;
            return Sequence(dynamicContext->getMemoryManager());
        }
    
        const XMLCh* FQilla_Node::dmNodeKind() const
        {
            LG_XQILLA_D << "FQilla_Node::dmNodeKind this:" << toVoid(this) << endl;
            LG_XQILLA_D << "dmNodeKind() m_isSingleTextContentsNode:" << m_isSingleTextContentsNode << endl;
            if( isBound( m_ctx ) )
            {
                LG_XQILLA_D << "dmNodeKind() ctx:" << m_ctx->getURL() << endl;
                LG_XQILLA_D << "dmNodeKind() isAttrNode:" << isAttrNode() << endl;
            }
            
            if( isAttrNode() )
                return attribute_string;
            if( m_isSingleTextContentsNode )
                return text_string;
            
            return element_string;
        }

//     std::string
//     FQilla_Node::canonicalElementName( fh_context c ) const
//     {
//         string nn = m_ctx->getDirName().c_str();
//         if( starts_with( nn, "." ))
//             nn = (string)"_" + nn;
//         nn = Util::replace_all( nn, "+", "_plus_" );
//         return nn;
//     }
    
    
        ATQNameOrDerived::Ptr
        FQilla_Node::dmNodeName(const DynamicContext* dynamicContext) const
        {
            LG_XQILLA_D << "dmNodeName()" << endl;
            LG_XQILLA_D << "dmNodeName() ctx:" << m_ctx->getURL() << endl;

            XMLCh* ns = 0;
            XMLCh* prefix = 0;
            string nn = canonicalElementName( m_ctx );
            XMLCh* localname = XMLString::transcode( nn.c_str() );

            LG_XQILLA_D << "dmNodeName() ret:" << nn << endl;
        
            return dynamicContext->getItemFactory()->createQName(
                ns, prefix, localname, dynamicContext );
        }
    
        const XMLCh* FQilla_Node::dmStringValue(const DynamicContext* dynamicContext) const
        {
            LG_XQILLA_D << "dmStringValue() this:" << toVoid(this) << endl;
            LG_XQILLA_D << "m_isSingleTextContentsNode:" << m_isSingleTextContentsNode << endl;
            
            if( isAttrNode() )
            {
                string v = getStrAttr( m_ctx, m_eaname, "" );
                return dynamicContext->getMemoryManager()->getPooledString( X(v.c_str()) );
            }

            fh_istream iss = m_ctx->getIStream();
            string v = StreamToString( iss );

//             LG_XQILLA_D << "dmStringValue(X)" << endl;
//             return dynamicContext->getMemoryManager()->getPooledString( X( "<foo>bar</foo>" ) );
                
            return dynamicContext->getMemoryManager()->getPooledString( X(v.c_str()) );
        }
    
        Sequence FQilla_Node::dmTypedValue(DynamicContext* dynamicContext) const
        {
            LG_XQILLA_D << "dmTypedValue() this:" << toVoid(this) << endl;
            LG_XQILLA_D << "m_isSingleTextContentsNode:" << m_isSingleTextContentsNode << endl;

            if( isAttrNode() )
            {
                Item::Ptr item = 0;
                item = (const Item::Ptr)dynamicContext->getItemFactory()->createUntypedAtomic(
                    dmStringValue(dynamicContext), dynamicContext);
                return Sequence(item, dynamicContext->getMemoryManager()); 
            }


            Item::Ptr item = 0;
            item = (const Item::Ptr)dynamicContext->getItemFactory()->createUntypedAtomic(
                dmStringValue(dynamicContext), dynamicContext);
            return Sequence(item, dynamicContext->getMemoryManager()); 
        
//        return Sequence(dynamicContext->getMemoryManager());
        }
    
        Sequence FQilla_Node::dmDocumentURI(const DynamicContext* dynamicContext) const
        {
            LG_XQILLA_D << "dmDocumentURI()" << endl;
            return Sequence(dynamicContext->getMemoryManager());
        }
    
        ATQNameOrDerived::Ptr FQilla_Node::dmTypeName(const DynamicContext* dynamicContext) const
        {
            LG_XQILLA_D << "dmTypeName()" << endl;

            const XMLCh* uri = FunctionConstructor::XMLChXPath2DatatypesURI;
//            const XMLCh* uri = XMLUni::fgZeroLenString;
            const XMLCh* name = DocumentCache::g_szUntyped;
        
            if( isAttrNode() )
            {
                uri=FunctionConstructor::XMLChXPath2DatatypesURI;
                name=ATUntypedAtomic::fgDT_UNTYPEDATOMIC;
            }

            return dynamicContext->getItemFactory()->
                createQName(uri, XMLUni::fgZeroLenString, name, dynamicContext);
        }
    
        ATBooleanOrDerived::Ptr FQilla_Node::dmNilled(const DynamicContext* dynamicContext) const
        {
            LG_XQILLA_D << "dmNilled()" << endl;

            if( isAttrNode() )
            {
                return 0;
            }
        
            return dynamicContext->getItemFactory()->createBoolean( false, dynamicContext );
        }
    
        bool FQilla_Node::lessThan(const Node::Ptr &other, const DynamicContext* dynamicContext) const
        {
            LG_XQILLA_D << "lessThan(top)" << endl;
            const FQilla_Node* otherImpl = (const FQilla_Node*)other->getInterface(FQilla_Node::fQilla);
        
            fh_context otherc = otherImpl->m_ctx;

            string earl  = m_ctx->getURL();
            string oearl = otherc->getURL();

            LG_XQILLA_D << "lessThan()  earl:" << earl << endl;
            LG_XQILLA_D << "lessThan() oearl:" << oearl << endl;
            LG_XQILLA_D << "lessThan() ret:" << (earl < oearl) << endl;
        
            return earl < oearl;
        }
    
        bool FQilla_Node::equals(const Node::Ptr &other) const
        {
            LG_XQILLA_D << "equals(top)" << endl;
            const FQilla_Node* otherImpl = (const FQilla_Node*)other->getInterface(FQilla_Node::fQilla);
            fh_context otherc = otherImpl->m_ctx;

            string earl  = m_ctx->getURL();
            string oearl = otherc->getURL();

            LG_XQILLA_D << "equals()  earl:" << earl << endl;
            LG_XQILLA_D << "equals() oearl:" << oearl << endl;
            LG_XQILLA_D << "equals() ret:" << (earl == oearl) << endl;
        
            return earl == oearl;
        }
    
        bool FQilla_Node::uniqueLessThan(const Node::Ptr &other, const DynamicContext* dynamicContext) const
        {
            LG_XQILLA_D << "uniqueLessThan()" << endl;
            return lessThan( other, dynamicContext );
        }
        Node::Ptr FQilla_Node::dmParent(const DynamicContext* dynamicContext) const
        {
            LG_XQILLA_D << "dmParent()" << endl;
            LG_XQILLA_D << "m_ctx:" << m_ctx->getURL() << endl;

            if( m_forceNoParent )
                return 0;

            if( m_ctx->isParentBound() )
            {
                return new FQilla_Node( (DynamicContext*)dynamicContext, m_ctx->getParent(), "" );
            }
            return 0;
        }
        Result FQilla_Node::dmAttributes(const DynamicContext* dynamicContext, const LocationInfo *info) const
        {
            LG_XQILLA_D << "dmAttributes()" << endl;
            if( m_eaname.empty() )
            {
                return new FQilla_Attr_ResultImpl( info, 0, m_ctx );
//            return new AttributeAxis(info, fNode, this, NULL);
            }
            return 0;
        }
        Result FQilla_Node::dmNamespaceNodes(const DynamicContext* dynamicContext, const LocationInfo *info) const
        {
            LG_XQILLA_D << "dmNamespaceNodes()" << endl;
            return 0;
        }
        Result FQilla_Node::dmChildren(const DynamicContext *context, const LocationInfo *info) const
        {
            LG_XQILLA_D << "dmChildren()" << endl;
            if( m_eaname.empty() )
            {
                return new FQilla_Iterator_ResultImpl<Context::iterator>( info, 0, m_ctx, m_ctx->begin(), m_ctx->end() );
//            return new ChildAxis(info, fNode, this, NULL);
            }
            return 0;
        }

        void
        FQilla_Node::getAllChildren( fh_context c, ctxlist_t* cl ) const
        {
            Context::iterator    e = c->end();
            Context::iterator iter = c->begin();
            for( ; iter != e; ++iter )
            {
                cl->push_back( *iter );
                getAllChildren( *iter, cl );
            }
        }
    

    
        Result FQilla_Node::getAxisResult(XQStep::Axis axis,
                                          const NodeTest *nodeTest,
                                          const DynamicContext* dynamicContext,
                                          const LocationInfo *info) const
        {
            LG_XQILLA_D << "getAxisResult() axis:" << axis << endl;
            LG_XQILLA_D << "getAxisResult() ctx:" << m_ctx->getURL() << endl;
            LG_XQILLA_D << " attr type:" << XQStep::ATTRIBUTE << " child type:" << XQStep::CHILD << endl;
            LG_XQILLA_D << "have nodetest:" << toVoid( nodeTest ) << endl;
            
            if( nodeTest )
            {
                if(nodeTest->getNameWildcard())
                    LG_XQILLA_D << " name wildcard:" << tostr(nodeTest->getNameWildcard()) << endl;
                if(nodeTest->getTypeWildcard())
                    LG_XQILLA_D << " type wildcard:" << tostr(nodeTest->getTypeWildcard()) << endl;
                if(nodeTest->getNodeName())
                    LG_XQILLA_D << " nn:" << tostr(nodeTest->getNodeName()) << endl;
                if(nodeTest->getNodePrefix())
                    LG_XQILLA_D << " prefix:" << tostr(nodeTest->getNodePrefix()) << endl;
                if(nodeTest->getNodeType())
                    LG_XQILLA_D << " node type:" << tostr(nodeTest->getNodeType()) << endl;
                SequenceType::ItemType *itemType = nodeTest->getItemType();
                if( itemType )
                {
                    LG_XQILLA_D << "have itemtype..." << endl;
                }
            }
            
                
            switch(axis)
            {
            case XQStep::ANCESTOR_OR_SELF:
            case XQStep::ANCESTOR:
            {
                LG_XQILLA_D << "XQStep::ANCESTOR or XQStep::ANCESTOR_OR_SELF" << endl;
            
                ctxlist_t* cl = new ctxlist_t;
                fh_context c = m_ctx;
                if( axis == XQStep::ANCESTOR_OR_SELF )
                {
                    cl->push_back( c );
                }
                while( c && c->isParentBound() )
                {
                    c = c->getParent();

                    cl->push_back( c );
                    if( get_fqilla_fakeroots().end() !=
                        get_fqilla_fakeroots().find( make_pair( dynamicContext, GetImpl(c) ) ) )
                    {
                        break;
                    }
                }
                return new FQilla_Iterator_ResultImpl<ctxlist_t::iterator>( info, nodeTest, m_ctx, cl->begin(), cl->end(), cl );
            }
            case XQStep::ATTRIBUTE:
            {
                LG_XQILLA_D << "XQStep::ATTRIBUTE m_eaname:" << m_eaname << endl;
                
                if( m_eaname.empty() )
                {
                    return new FQilla_Attr_ResultImpl( info, nodeTest, m_ctx );
                }
                break;
            }
            case XQStep::CHILD:
            {
                LG_XQILLA_D << "XQStep::CHILD option..." << endl;
                if( !isAttrNode() )
                {
                    if(!nodeTest->getNameWildcard())
                    {
                        string n = tostr(nodeTest->getNodeName());
                        LG_XQILLA_D << "specific child selected n:" << n << endl;

                        if( m_ctx->isSubContextBound( n ) )
                        {
                            fh_context cc = m_ctx->getSubContext( n );
                            FQilla_Node* theNode = new FQilla_Node(
                                (DynamicContext*)dynamicContext, cc, "" );
                            return new FQilla_SingleNode_ResultImpl( info, nodeTest, cc, theNode );
                        }
                    }

                    Context::iterator b = m_ctx->begin();
                    Context::iterator e = m_ctx->end();

//                     if( b == e )
//                     {
//                         string eaname = "";
//                         bool forceNoParent = false;
//                         bool isSingleTextContentsNode = true;
//                         FQilla_Node* p = new FQilla_Node( dynamicContext, m_ctx, eaname, forceNoParent, isSingleTextContentsNode );
//                         LG_XQILLA_D << "single TEXT child:" << toVoid(p) << endl;
//                         return new FQilla_SingleNode_ResultImpl( info, nodeTest, m_ctx, p );
//                     }
                    return new FQilla_Iterator_ResultImpl<Context::iterator>( info, nodeTest, m_ctx, b, e );
                }
                break;
            }
            case XQStep::DESCENDANT: 
            case XQStep::DESCENDANT_OR_SELF:
            {
                LG_XQILLA_D << "XQStep::DESCENDANT/XQStep::DESCENDANT_OR_SELF" << endl;
            
                ctxlist_t* cl = new ctxlist_t;
                fh_context c = m_ctx;
                if( axis == XQStep::DESCENDANT_OR_SELF )
                {
                    cl->push_back( c );
                }
                getAllChildren( m_ctx, cl );
                return new FQilla_Iterator_ResultImpl<ctxlist_t::iterator>( info, nodeTest, m_ctx, cl->begin(), cl->end(), cl );
                break;
            }
            case XQStep::FOLLOWING:
            {
                if( m_forceNoParent )
                    return 0;
            
                if( m_ctx->isParentBound() )
                {
                    ctxlist_t* cl = new ctxlist_t;
                
                    fh_context p = m_ctx->getParent();
                    Context::iterator    e = p->end();
                    Context::iterator iter = p->find( m_ctx->getDirName() );
                    ++iter;
                    for( ; iter != e ; ++iter )
                    {
                        cl->push_back( *iter );
                        getAllChildren( *iter, cl );
                    }
                    return new FQilla_Iterator_ResultImpl<ctxlist_t::iterator>( info, nodeTest, p, cl->begin(), cl->end(), cl );
                }
            }
            case XQStep::FOLLOWING_SIBLING:
            {
                if( m_forceNoParent )
                    return 0;

                if( m_ctx->isParentBound() )
                {
                    fh_context p = m_ctx->getParent();
                    Context::iterator    e = p->end();
                    Context::iterator iter = p->find( m_ctx->getDirName() );
                    ++iter;
                    return new FQilla_Iterator_ResultImpl<Context::iterator>( info, nodeTest, p, iter, e );
                }
//            return new FollowingSiblingAxis(info, nodeTest, fNode, this, nodeTest);
            }
            case XQStep::NAMESPACE: {
//            if(fNode->getNodeType() == DOMNode::ELEMENT_NODE) {
//                return new NamespaceAxis(info, fNode, this, nodeTest);
//            }
                break;
            }
            case XQStep::PARENT: {
                Node::Ptr p = dmParent( dynamicContext );
                return new FQilla_SingleNode_ResultImpl( info, nodeTest, m_ctx, p );
            }
            case XQStep::PRECEDING:
            {
                if( m_forceNoParent )
                    return 0;

                if( m_ctx->isParentBound() )
                {
                    ctxlist_t* cl = new ctxlist_t;
                
                    fh_context p = m_ctx->getParent();
                    Context::iterator iter = p->begin();
                    Context::iterator    e = p->find( m_ctx->getDirName() );
                    for( ; iter != e ; ++iter )
                    {
                        cl->push_back( *iter );
                        getAllChildren( *iter, cl );
                    }
                    return new FQilla_Iterator_ResultImpl<ctxlist_t::iterator>( info, nodeTest, p, cl->begin(), cl->end(), cl );
                }
            }
            case XQStep::PRECEDING_SIBLING:
            {
                if( m_forceNoParent )
                    return 0;

                if( m_ctx->isParentBound() )
                {
                    fh_context p = m_ctx->getParent();
                    Context::iterator iter = p->begin();
                    Context::iterator    e = p->find( m_ctx->getDirName() );
                    return new FQilla_Iterator_ResultImpl<Context::iterator>( info, nodeTest, p, iter, e );
                }
            }
            case XQStep::SELF: {
                return new FQilla_SingleNode_ResultImpl( info, nodeTest, m_ctx, this );
            }
            }

            return 0;
        
        }
        ATBooleanOrDerived::Ptr FQilla_Node::dmIsId(const DynamicContext* dynamicContext) const
        {
            LG_XQILLA_D << "dmIsId()" << endl;
            return 0;
        }
        ATBooleanOrDerived::Ptr FQilla_Node::dmIsIdRefs(const DynamicContext* dynamicContext) const
        {
            LG_XQILLA_D << "dmIsIdRefs()" << endl;
            return 0;
        }
        const XMLCh* FQilla_Node::getTypeURI() const
        {
            LG_XQILLA_D << "getTypeURI()" << endl;
            return FunctionConstructor::XMLChXPath2DatatypesURI;
        }
        const XMLCh* FQilla_Node::getTypeName() const
        {
            LG_XQILLA_D << "getTypeName()" << endl;

            if( m_eaname.empty() )
            {
                return DocumentCache::g_szUntyped;
            }
            return ATUntypedAtomic::fgDT_UNTYPEDATOMIC;
        }

        fh_context
        FQilla_Node::getFerrisContext() const
        {
            return m_ctx;
        }
    
        bool
        FQilla_Node::isAttrNode() const
        {
            return !m_eaname.empty();
        }


        Node::Ptr
        FQilla_Node::root(const DynamicContext* context) const
        {
            return new FQilla_Node( context, m_ctx );
        }
        
        void
        FQilla_Node::generateEvents(EventHandler *events, const DynamicContext *context,
                                    bool preserveNS, bool preserveType) const
        {
            cerr << "FIXME: FQilla_Node::generateEvents()" << endl;
//            FastXDMDocument::toEvents( attr_, events, preserveType );
        }
        
        

        /////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////

        ///Macro used to facilitate the creation of functions
        template<class TYPE>
        class FuncFactoryTemplate : public FuncFactory
        {
            XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer uriname;
        public:

            FuncFactoryTemplate( int argCount, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm )
                :
                FuncFactory( argCount, mm )
                {
                    uriname.set(getName());
                    uriname.append(getURI());
                }

            virtual ASTNode *createInstance(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr) const
                {
                    return new (memMgr) TYPE(args, memMgr);
                }

            virtual const XMLCh *getName() const
                {
                    return TYPE::name;
                }

            virtual const XMLCh *getURI() const
                {
                    return TYPE::XMLChFunctionURI;
                }
            virtual const XMLCh *getURINameHash() const
                {
                    return uriname.getRawBuffer();
                }
            

            virtual const XMLCh *getQName() const
                {
                    return getName();
                }

            virtual size_t getMinArgs() const
                {
                    return TYPE::minArgs;
                }

            virtual size_t getMaxArgs() const
                {
                    return TYPE::maxArgs;
                }
        };

        

        const XMLCh FunctionFerrisDoc::name[] = {
            chLatin_f, chLatin_e, chLatin_r, chLatin_r, chLatin_i, chLatin_s,
            chDash,
            chLatin_d, chLatin_o, chLatin_c, 
            chNull 
        };
        const unsigned int FunctionFerrisDoc::minArgs = 1;
        const unsigned int FunctionFerrisDoc::maxArgs = 1;

        /**
         * fn:ferris-doc($uri as xs:string?) as document?
         **/
        FunctionFerrisDoc::FunctionFerrisDoc(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
            : XQFunction(name, minArgs, maxArgs, "string?", args, memMgr)
        {
        }

        ASTNode* FunctionFerrisDoc::staticResolution(StaticContext *context)
        {
            return resolveArguments(context);
        }

        ASTNode *FunctionFerrisDoc::staticTypingImpl(StaticContext *context)
        {
            _src.clear();

            _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                               StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::ONENODE);
            _src.getStaticType() = StaticType(StaticType::DOCUMENT_TYPE, 0, 1);
            _src.availableDocumentsUsed(true);

            return calculateSRCForArguments(context);
        }
        
        

        Sequence FunctionFerrisDoc::createSequence(DynamicContext* context, int flags) const
        {
            LG_XQILLA_D << "FunctionFerrisDoc::createSequence(context,flags) flags:" << flags << endl;
            Sequence uriArg = getParamNumber(1,context)->toSequence(context);
  
            if (uriArg.isEmpty())
            {
                return Sequence(context->getMemoryManager());
            }
  
            const XMLCh* uri = uriArg.first()->asString(context);
//             if(!XPath2Utils::isValidURI(uri, context->getMemoryManager()))
//                 XQThrow(FunctionException,
//                         X("FunctionFerrisDoc::collapseTreeInternal"),
//                         X("Invalid argument to fn:doc function [err:FODC0005]"));

            try
            {
//                return context->resolveDocument(uri, this);
                Sequence result(context->getMemoryManager());

                LG_XQILLA_D << "FunctionFerrisDoc::createSequence(context,flags) uri:" << tostr(uri) << endl;
                fh_context m_ctx = Resolve( tostr(uri) );
                FQilla_Node* qn = new FQilla_Node( context, m_ctx, "", true );
                result.addItem( qn );
                return result;
            } 
            //TODO:  once DocumentCacheImpl can throw different errors, we should be able to throw the correct corresponding error messages.
            catch(XMLParseException &e)
            {
                XQThrow(FunctionException,
                        X("FunctionFerrisDoc::collapseTreeInternal"),
                        e.getError());
            }
            return Sequence(context->getMemoryManager());
        }
        
        ////////////////////////////////////////
        ////////////////////////////////////////
        ////////////////////////////////////////

        const XMLCh FunctionFerrisDocQuiet::name[] = {
            chLatin_f, chLatin_e, chLatin_r, chLatin_r, chLatin_i, chLatin_s,
            chDash,
            chLatin_d, chLatin_o, chLatin_c, 
            chDash,
            chLatin_q, chLatin_u, chLatin_i, chLatin_e, chLatin_t, 
            chNull 
        };
        const unsigned int FunctionFerrisDocQuiet::minArgs = 1;
        const unsigned int FunctionFerrisDocQuiet::maxArgs = 1;

        /**
         * fn:ferris-doc($uri as xs:string?) as document?
         **/
        FunctionFerrisDocQuiet::FunctionFerrisDocQuiet(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
            : XQFunction(name, minArgs, maxArgs, "string?", args, memMgr)
        {
        }

        ASTNode* FunctionFerrisDocQuiet::staticResolution(StaticContext *context)
        {
            return resolveArguments(context);
        }

        ASTNode *FunctionFerrisDocQuiet::staticTypingImpl(StaticContext *context)
        {
            _src.clear();

            _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                               StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::ONENODE);
            _src.getStaticType() = StaticType(StaticType::DOCUMENT_TYPE, 0, 1);
            _src.availableDocumentsUsed(true);

            return calculateSRCForArguments(context);
        }

        Sequence FunctionFerrisDocQuiet::createSequence(DynamicContext* context, int flags) const
        {
            LG_XQILLA_D << "FunctionFerrisDocQuiet::createSequence(top)" << endl;
            Sequence uriArg = getParamNumber(1,context)->toSequence(context);
  
            if (uriArg.isEmpty())
            {
                return Sequence(context->getMemoryManager());
            }
  
            const XMLCh* uri = uriArg.first()->asString(context);
//             if(!XPath2Utils::isValidURI(uri, context->getMemoryManager()))
//                 XQThrow(FunctionException,
//                         X("FunctionFerrisDocQuiet::collapseTreeInternal"),
//                         X("Invalid argument to fn:doc function [err:FODC0005]"));

            try
            {
//                return context->resolveDocument(uri, this);
                Sequence result(context->getMemoryManager());

                try
                {
                    LG_XQILLA_D << "FunctionFerrisDocQuiet::createSequence() uri:" << tostr(uri) << endl;
                    cerr << "FunctionFerrisDocQuiet::createSequence() uri:" << tostr(uri) << endl;
                    fh_context m_ctx = Resolve( tostr(uri) );
                    FQilla_Node* qn = new FQilla_Node( context, m_ctx, "", true );
                    result.addItem( qn );
                }
                catch( ... )
                {
                }
                return result;
            } 
            //TODO:  once DocumentCacheImpl can throw different errors, we should be able to throw the correct corresponding error messages.
            catch(XMLParseException &e)
            {
                XQThrow(FunctionException,
                        X("FunctionFerrisDocQuiet::collapseTreeInternal"),
                        e.getError());
            }
            return Sequence(context->getMemoryManager());
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        ////////////////////////////////////////

        const XMLCh FunctionFerrisIndexLookup::name[] = {
            chLatin_f, chLatin_e, chLatin_r, chLatin_r, chLatin_i, chLatin_s,
            chDash,
            chLatin_i, chLatin_n, chLatin_d, chLatin_e, chLatin_x, 
            chDash,
            chLatin_l, chLatin_o, chLatin_o, chLatin_k, chLatin_u, chLatin_p, 
            chNull 
        };
        const unsigned int FunctionFerrisIndexLookup::minArgs = 3;
        const unsigned int FunctionFerrisIndexLookup::maxArgs = 3;

        /**
         * fn:ferris-doc($uri as xs:string?) as document?
         **/
        FunctionFerrisIndexLookup::FunctionFerrisIndexLookup(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
            : XQFunction(name, minArgs, maxArgs, "string, string, string", args, memMgr)
        {
        }

        ASTNode* FunctionFerrisIndexLookup::staticResolution(StaticContext *context)
        {
            return resolveArguments(context);
        }

        ASTNode *FunctionFerrisIndexLookup::staticTypingImpl(StaticContext *context)
        {
            _src.clear();

            _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                               StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::ONENODE);
            _src.getStaticType() = StaticType(StaticType::DOCUMENT_TYPE, 0, 1);
            _src.availableDocumentsUsed(true);

            return calculateSRCForArguments(context);
        }

        IndexLookupResult::IndexLookupResult()
        {
        }
        
        void
        IndexLookupResult::insert( DynamicContext* context, const std::string& value, fh_context ctx )
        {
            m_valueLookup_t::iterator iter = m_valueLookup.find( value );
            if( iter == m_valueLookup.end() )
            {
                Sequence seq(context->getMemoryManager());
                iter = m_valueLookup.insert( make_pair( value, seq ) ).first;
            }

            Sequence& seq = iter->second;
            FQilla_Node* qn = new FQilla_Node( context, ctx, "", true );
            seq.addItem( qn );
        }

        Sequence
        IndexLookupResult::find( DynamicContext* context, const std::string& value )
        {
            m_valueLookup_t::iterator iter = m_valueLookup.find( value );
            if( iter == m_valueLookup.end() )
            {
                LG_XQILLA_D << "IndexLookupResult::find() NO result for v:" << value << endl;
                Sequence seq(context->getMemoryManager());
                return seq;
            }
            LG_XQILLA_D << "IndexLookupResult::find() returning result for v:" << value << endl;
            LG_XQILLA_D << "IndexLookupResult::find() result.len:" << iter->second.getLength() << endl;
            
            return iter->second;
        }
        
        
        void
        FunctionFerrisIndexLookup::populateLookup( DynamicContext* context, IndexLookupResult* l, fh_context ctx, const std::string eaname ) const
        {
            try
            {
                if( ctx->isAttributeBound( eaname ) )
                {
                    string value = getStrAttr( ctx, eaname, "" );
                    LG_XQILLA_D << "FunctionFerrisIndexLookup::populateLookup() ea:" << eaname
                                << " v:" << value
                                << " adding ctx:" << ctx->getURL()
                                << endl;

                    l->insert( context, value, ctx );
                }
                for( Context::iterator ci = ctx->begin(); ci != ctx->end(); ++ci )
                    populateLookup( context, l, *ci, eaname );
            }
            catch( ... )
            {
            }
        }
        
        
        Sequence
        FunctionFerrisIndexLookup::createSequence( DynamicContext* context, int flags) const
        {
            LG_XQILLA_D << "FunctionFerrisIndexLookup::createSequence(top)" << endl;

            if( getParamNumber(1,context)->next(context)->asString(context) == 0
                || getParamNumber(2,context)->next(context)->asString(context) == 0
                || getParamNumber(3,context)->next(context)->asString(context) == 0
                )
            {
                LG_XQILLA_D << "FunctionFerrisIndexLookup::createSequence(bad args)" << endl;
                return Sequence(context->getMemoryManager());
            }
            
            string earl   = tostr(getParamNumber(1,context)->next(context)->asString(context));
            string eaname = tostr(getParamNumber(2,context)->next(context)->asString(context));
            string value  = tostr(getParamNumber(3,context)->next(context)->asString(context));

            LG_XQILLA_D << "FunctionFerrisIndexLookup::createSequence() eaname:" << eaname << endl;
            LG_XQILLA_D << "FunctionFerrisIndexLookup::createSequence() value:" << value << endl;
            LG_XQILLA_D << "FunctionFerrisIndexLookup::createSequence() earl:" << earl << endl;
            
            try
            {
                fh_context ctx = Resolve( earl );
                Sequence result(context->getMemoryManager());

                m_lookup_t::iterator li = m_lookup.find( make_pair( earl, eaname ) );
                if( li == m_lookup.end() )
                {
                    LG_XQILLA_D << "FunctionFerrisIndexLookup::createSequence() populating index for eaname:" << eaname << endl;
                    IndexLookupResult* l = new IndexLookupResult();
                    populateLookup( context, l, ctx, eaname );
                    li = m_lookup.insert( make_pair( make_pair( earl, eaname ), l ) ).first;
                }

                if( li == m_lookup.end() )
                    return result;

                return li->second->find( context, value );
            } 
            catch(XMLParseException &e)
            {
                XQThrow(FunctionException,
                        X("FunctionFerrisIndexLookup::collapseTreeInternal"),
                        e.getError());
            }
            return Sequence(context->getMemoryManager());
        }
        
        ////////////////////////////////////////
        ////////////////////////////////////////
        ////////////////////////////////////////
        
        void RegisterFerrisXQillaFunctions( StaticContext* context ) 
        {


            context->addCustomFunction( new FuncFactoryTemplate<FunctionFerrisDoc>(1,context->getMemoryManager()) );
            context->addCustomFunction( new FuncFactoryTemplate<FunctionFerrisDocQuiet>(1,context->getMemoryManager()) );
            context->addCustomFunction( new FuncFactoryTemplate<FunctionFerrisIndexLookup>(3,context->getMemoryManager()) );
            
            LG_XQILLA_D << "Added custom ferris xqilla functions..." << endl;
        }
        


        /////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////

        /**
         * Copy all children of 'c' to 'target'
         */
        void copyChildren( const DynamicContext* dc,
                           DOMDocument* document, fh_context c, DOMElement* target )
        {
            string txt = getStrAttr( c, "content", "" );
            if( !txt.empty() )
            {
                XML::setChildText( document, (DOMElement*)target, txt );
            }

            fh_shouldIncludeAttribute sia = getShouldIncludeAttribute( dc );
            
            AttributeCollection::AttributeNames_t an;
            c->getAttributeNames( an );
            for( AttributeCollection::AttributeNames_t::iterator iter = an.begin();
                 iter != an.end(); iter++ )
            {
                std::string eaname = *iter;
                if( sia->shouldInclude( c, eaname ) )
                {
                    string v = getStrAttr( c, eaname, "" );
                    LG_XQILLA_D << "copying attribute:" << eaname << " value:" << v << endl;
                    ensureAttribute( document, target, eaname, v );
                }
            }
            


            Context::iterator    e = c->end();
            Context::iterator iter = c->begin();
            for( ; iter != e ; ++iter )
            {
                string rdn = (*iter)->getDirName();

                string t = getStrAttr( *iter, "libferris-element-name", "" );
                if( !t.empty() )
                    rdn = t;
                
                string crdn = canonicalElementName( rdn );
                DOMNode *newChild = document->createElement(X( crdn.c_str() ));
                target->appendChild(newChild);

                copyChildren( dc, document, *iter, (DOMElement*)newChild );
            }
        }

        /////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////
        

        
//         FQilla_ItemFactory::FQilla_ItemFactory( ItemFactory* ifactory )
// //                                                 const DocumentCache* dc,
// //                                                 MemoryManager* memMgr )
//             :
//                                               //            ItemFactoryImpl( dc, memMgr ),
//         m_ifactory( ifactory )
// //        m_outputDocument( 0 )
//         {
//         }
        
//         FQilla_ItemFactory::~FQilla_ItemFactory()
//         {
//         }



//         Node::Ptr FQilla_ItemFactory::cloneNode(const Node::Ptr node, const DynamicContext *context) const
//         {
//             return m_ifactory->cloneNode( node, context );
//         }
        
//         Node::Ptr FQilla_ItemFactory::createTextNode(const XMLCh *value, const DynamicContext *context) const
//         {
//             return m_ifactory->createTextNode( value, context );
//         }
        
//         Node::Ptr FQilla_ItemFactory::createCommentNode(const XMLCh *value, const DynamicContext *context) const
//         {
//             return m_ifactory->createCommentNode( value, context );
//         }
        
//         Node::Ptr FQilla_ItemFactory::createPINode(const XMLCh *name, const XMLCh *value, const DynamicContext *context) const
//         {
//             return m_ifactory->createPINode( name, value, context );
//         }
        
//         Node::Ptr FQilla_ItemFactory::createAttributeNode(const XMLCh *uri, const XMLCh *prefix, const XMLCh *name,
//                                                           const XMLCh *value, const DynamicContext *context) const
//         {
//             return m_ifactory->createAttributeNode( uri, prefix, name, value, context );
//         }


        
        
//         Node::Ptr FQilla_ItemFactory::createElementNode(const XMLCh *uri, const XMLCh *prefix, const XMLCh *name,
//                                                         const std::vector<Node::Ptr> &attrList,
//                                                         const std::vector<ItemFactory::ElementChild> &childList,
//                                                         const DynamicContext *context) const
//         {
// //            return m_ifactory->createElementNode( uri, prefix, name, attrList, childList, context );

//             StaticContext::ConstructionMode constrMode=context->getConstructionMode();
//             bool nsPreserveMode=context->getPreserveNamespaces();
//             bool nsInheritMode=context->getInheritNamespaces();

//              DOMDocument *document = getOutputDocument(context);
//              DOMElement *element = document->createElementNS(uri, name);
//              if(prefix != 0 && !XPath2Utils::equals(XMLUni::fgZeroLenString, prefix))
//                  element->setPrefix(prefix);

//             for(std::vector<Node::Ptr>::const_iterator a = attrList.begin(); a != attrList.end(); ++a)
//             {
//                 if( const NodeImpl *nodeImpl = (const NodeImpl *)(*a)->getInterface(Item::gXQilla) )
//                 {
//                     LG_XQILLA_D << "createElementNode(attr) xqilla native node" << endl;
                    
//                     const DOMNode* attr=nodeImpl->getDOMNode();
//                     DOMAttr* exists=element->getAttributeNodeNS(attr->getNamespaceURI(), attr->getLocalName());

//                     LG_XQILLA_D << "createElementNode(attr) xqilla native node localName:" << tostr(attr->getLocalName()) << endl;

//                     DOMAttr* imported = (DOMAttr*)document->importNode(const_cast<DOMNode*>(attr),true);
//                     if(constrMode == StaticContext::CONSTRUCTION_MODE_PRESERVE)
//                         XPath2Utils::copyAttributeType(document, imported, (const DOMAttr*)attr);
//                     if(context->getDebugCallback()) context->getDebugCallback()->ReportClonedNode(const_cast<DynamicContext*>(context), attr, imported);

//                     element->setAttributeNodeNS(imported);
//                 }
//                 else
//                 {
//                     LG_XQILLA_D << "createElementNode(attr) ferris-xqilla native node" << endl;
                    
//                     const FQilla_Node* nodeImpl = (const FQilla_Node*)(*a)->getInterface(FQilla_Node::fQilla);
//                     assert( nodeImpl != 0 );

//                     LG_XQILLA_D << "createElementNode(attr) ferris-xqilla native node eaname:" << nodeImpl->m_eaname << endl;
//                     ensureAttribute( document, element, nodeImpl->m_eaname,
//                                      getStrAttr( nodeImpl->m_ctx, nodeImpl->m_eaname, "" ) );
//                 }
                
                
//             }

// //             std::vector<const XMLCh*> inScopePrefixes;
// //             if(!nsInheritMode)
// //             {
// //                 DOMNode* elt=element;
// //                 while(elt!=NULL)
// //                 {
// //                     // check if this node has a namespace, but the prefix is not declared in the attribute map
// //                     const XMLCh* uri=elt->getNamespaceURI();
// //                     if(uri && *uri)
// //                     {
// //                         const XMLCh* prefix=elt->getPrefix();
// //                         if (!XPath2Utils::containsString(inScopePrefixes, prefix))
// //                             inScopePrefixes.push_back(prefix);
// //                     }
// //                     DOMNamedNodeMap* attrMap=elt->getAttributes();
// //                     for(XMLSize_t i=0;i<attrMap->getLength();i++)
// //                     {
// //                         DOMNode* attr=attrMap->item(i);
// //                         if(XPath2Utils::equals(attr->getPrefix(), XMLUni::fgXMLNSString) || XPath2Utils::equals(attr->getNodeName(), XMLUni::fgXMLNSString))
// //                         {
// //                             const XMLCh* prefix=attr->getPrefix()==NULL?XMLUni::fgZeroLenString:attr->getLocalName();
// //                             if (!XPath2Utils::containsString(inScopePrefixes, prefix))
// //                                 inScopePrefixes.push_back(prefix);
// //                         }
// //                     }
// //                     elt=elt->getParentNode();
// //                 }
// //             }

//             for(std::vector<ItemFactory::ElementChild>::const_iterator i = childList.begin(); i != childList.end(); ++i)
//             {
//                 if( const NodeImpl *nodeImpl = (const NodeImpl *)(*i)->getInterface(Item::gXQilla) )
//                 {
//                     LG_XQILLA_D << "createElementNode(element) xqilla native node" << endl;
//                     LG_XQILLA_D << "createElementNode(element) xqilla native node localName:" << tostr(nodeImpl->getDOMNode()->getLocalName()) << endl;
// //                     LG_XQILLA_D << "createElementNode(element) xqilla native node year:"
// //                                 << getAttribute( (DOMElement*)(nodeImpl->getDOMNode()), "year" )
// //                                 << endl;
                    

//                     DOMNode *newChild = NULL;
//                     if(nodeImpl->getDOMNode()->getOwnerDocument() == document)
//                     {
//                         LG_XQILLA_D << "createElementNode(element) xqilla native node (same doc) i->clone:" << i->clone << endl;
                        
//                         if(i->clone) {
//                             newChild = nodeImpl->getDOMNode()->cloneNode(true);
// //                             if(constrMode == StaticContext::CONSTRUCTION_MODE_PRESERVE && nodeImpl->dmNodeKind()==Node::element_string)
// //                                 XPath2Utils::copyElementType(newChild->getOwnerDocument(), (DOMElement*)newChild, (DOMElement*)nodeImpl->getDOMNode());
// //                             if(context->getDebugCallback()) context->getDebugCallback()->ReportClonedNode(const_cast<DynamicContext*>(context), nodeImpl->getDOMNode(), newChild);
//                         }
//                         else
//                         {
//                             LG_XQILLA_D << "createElementNode(element) xqilla native node (diff doc)" << endl;
//                             newChild = const_cast<DOMNode*>(nodeImpl->getDOMNode());
//                         }
//                     }
//                     else {
//                         newChild = document->importNode(const_cast<DOMNode*>(nodeImpl->getDOMNode()),true);
//                         if(constrMode == StaticContext::CONSTRUCTION_MODE_PRESERVE && nodeImpl->dmNodeKind()==Node::element_string)
//                             XPath2Utils::copyElementType(newChild->getOwnerDocument(), (DOMElement*)newChild, (DOMElement*)nodeImpl->getDOMNode());
//                         if(context->getDebugCallback()) context->getDebugCallback()->ReportClonedNode(const_cast<DynamicContext*>(context), nodeImpl->getDOMNode(), newChild);
//                     }

// //                     if(!nsPreserveMode && newChild->getNodeType()==DOMNode::ELEMENT_NODE)
// //                     {
// //                         DOMElement* childElem=(DOMElement*)newChild;
      
// //                         DOMNamedNodeMap* attrMap=childElem->getAttributes();
// //                         for(XMLSize_t i=0;i<attrMap->getLength();)
// //                         {
// //                             bool bPreserved=true;
// //                             DOMNode* attr=attrMap->item(i);

// //                             if(XPath2Utils::equals(attr->getPrefix(), XMLUni::fgXMLNSString) || XPath2Utils::equals(attr->getNodeName(), XMLUni::fgXMLNSString))
// //                             {
// //                                 const XMLCh* prefix=attr->getPrefix()==NULL?XMLUni::fgZeroLenString:attr->getLocalName();
// //                                 // copy this namespace only if needed by the element name...
// //                                 if(!XPath2Utils::equals(childElem->getPrefix(), prefix))
// //                                 {
// //                                     bPreserved=false;
// //                                     //... or by its attributes
// //                                     for(XMLSize_t j=0;j<attrMap->getLength();j++)
// //                                         if(XPath2Utils::equals(attrMap->item(j)->getPrefix(), prefix))
// //                                         {
// //                                             bPreserved=true;
// //                                             break;
// //                                         }
// //                                     if(!bPreserved)
// //                                         attrMap->removeNamedItemNS(attr->getNamespaceURI(), attr->getLocalName());
// //                                 }
// //                             }
// //                             if(bPreserved)
// //                                 i++;
// //                         }
// //                     }

//                     element->appendChild(newChild);
//                 }
//                 else
//                 {
//                     LG_XQILLA_D << "createElementNode(element) ferris-xqilla native node" << endl;
                    
//                     const FQilla_Node* nodeImpl = (const FQilla_Node*)(*i)->getInterface(FQilla_Node::fQilla);
//                     assert( nodeImpl != 0 );

//                     LG_XQILLA_D << "createElementNode(element) ferris-xqilla native node ctx:" << nodeImpl->m_ctx->getURL() << endl;
//                     LG_XQILLA_D << "createElementNode(element) ferris-xqilla native node i->clone:" << i->clone << endl;
                    
//                     string rdn = nodeImpl->m_ctx->getDirName();
//                     LG_XQILLA_D << "createElementNode(element) ...2" << endl;
//                     string crdn = canonicalElementName( rdn );
//                     LG_XQILLA_D << "createElementNode(element) ...3 crdn:" << crdn << endl;
//                     DOMNode *newChild = document->createElement(X( crdn.c_str() ));
//                     LG_XQILLA_D << "createElementNode(element) ...4" << endl;
                    
//                     // FIXME: maybe copy nodeImpl -> attributes and children to newChild?
//                     // we should do this because the above is all deep copies,
//                     // BUT: of all the attributes how do we know which to copy?

//                     copyChildren( context, document, nodeImpl->m_ctx, (DOMElement*)newChild );
//                     LG_XQILLA_D << "createElementNode(element) ...5" << endl;
                    
// //                    ensureAttribute( document, (DOMElement*)newChild, "foo", "bar" );
// //                     {
// //                         string txt = getStrAttr( nodeImpl->m_ctx, "content", "" );
// //                         if( !txt.empty() )
// //                         {
// //                             XML::setChildText( document, (DOMElement*)newChild, txt );
// //                         }
// //                     }
                    
//                     element->appendChild(newChild);
//                     LG_XQILLA_D << "createElementNode(element) ferris-xqilla done..." << endl;
//                 }
//             }  


//             return new NodeImpl(element, context);
//         }
        
//         Node::Ptr
//         FQilla_ItemFactory::createDocumentNode(const std::vector<Node::Ptr> &childList, const DynamicContext *context) const
//         {
//             return m_ifactory->createDocumentNode( childList, context );
//         }
        
//         const DOMNode*
//         FQilla_ItemFactory::createNamespaceNode(const XMLCh* prefix, const XMLCh* uri,
//                             const DOMNode* parentNode, const DynamicContext *context) const
//         {
//             return m_ifactory->createNamespaceNode( prefix, uri, parentNode, context );
//         }
        

//         AnyAtomicType::AtomicObjectType
//         FQilla_ItemFactory::getPrimitiveTypeIndex(const XMLCh* typeURI, const XMLCh* typeName, bool &isPrimitive) const
//         {
//             return m_ifactory->getPrimitiveTypeIndex( typeURI, typeName, isPrimitive );
//         }
        
//         AnyAtomicType::Ptr
//         FQilla_ItemFactory::createDerivedFromAtomicType(AnyAtomicType::AtomicObjectType typeIndex, const XMLCh* value,
//                                                         const DynamicContext* context)
//         {
//             return m_ifactory->createDerivedFromAtomicType( typeIndex, value, context );
//         }
        
//         AnyAtomicType::Ptr
//         FQilla_ItemFactory::createDerivedFromAtomicType(AnyAtomicType::AtomicObjectType typeIndex,
//                                                         const XMLCh* typeURI,
//                                                         const XMLCh* typeName,
//                                                         const XMLCh* value,
//                                                         const DynamicContext* context)
//         {
//             return m_ifactory->createDerivedFromAtomicType( typeIndex, typeURI, typeName, value, context );
//         }
        
//         AnyAtomicType::Ptr
//         FQilla_ItemFactory::createDerivedFromAtomicType(const XMLCh* typeURI,
//                                                         const XMLCh* typeName,
//                                                         const XMLCh* value,
//                                                         const DynamicContext* context)
//         {
//             return m_ifactory->createDerivedFromAtomicType( typeURI, typeName, value, context );
//         }
        

//         ATDoubleOrDerived::Ptr
//         FQilla_ItemFactory::createDouble(const MAPM value, const DynamicContext* context)
//         {
//             return m_ifactory->createDouble( value, context );
//         }
        
//         ATDoubleOrDerived::Ptr
//         FQilla_ItemFactory::createDouble(const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createDouble( value, context );
//         }
            
//         ATDoubleOrDerived::Ptr
//         FQilla_ItemFactory::createDoubleOrDerived(const XMLCh* typeURI,
//                                                   const XMLCh* typeName,
//                                                   const MAPM value,
//                                                   const DynamicContext* context)
//         {
//             return m_ifactory->createDoubleOrDerived( typeURI, typeName, value, context );
//         }
            
//         ATDoubleOrDerived::Ptr
//         FQilla_ItemFactory::createDoubleOrDerived(const XMLCh* typeURI,
//                                                   const XMLCh* typeName,
//                                                   const XMLCh* value,
//                                                   const DynamicContext* context)
//         {
//             return m_ifactory->createDoubleOrDerived( typeURI, typeName, value, context );
//         }
            

//         ATFloatOrDerived::Ptr
//         FQilla_ItemFactory::createFloat(const MAPM value, const DynamicContext* context)
//         {
//             return m_ifactory->createFloat( value, context );
//         }
        
//         ATFloatOrDerived::Ptr
//         FQilla_ItemFactory::createFloat(const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createFloat( value, context );
//         }
            
//         ATFloatOrDerived::Ptr
//         FQilla_ItemFactory::createFloatOrDerived(const XMLCh* typeURI,
//                                                  const XMLCh* typeName,
//                                                  const MAPM value,
//                                                  const DynamicContext* context)
//         {
//             return m_ifactory->createFloatOrDerived( typeURI, typeName, value, context );
//         }
            
//         ATFloatOrDerived::Ptr
//         FQilla_ItemFactory::createFloatOrDerived(const XMLCh* typeURI,
//                                                  const XMLCh* typeName,
//                                                  const XMLCh* value,
//                                                  const DynamicContext* context)
//         {
//             return m_ifactory->createFloatOrDerived( typeURI, typeName, value, context );
//         }

//         ATDecimalOrDerived::Ptr FQilla_ItemFactory::createInteger(const int value, const DynamicContext* context)
//         {
//             return m_ifactory->createInteger( value, context );
//         }
//         ATDecimalOrDerived::Ptr FQilla_ItemFactory::createInteger(const MAPM value, const DynamicContext* context)
//         {
//             return m_ifactory->createInteger( value, context );
//         }
//         ATDecimalOrDerived::Ptr FQilla_ItemFactory::createInteger(const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createInteger( value, context );
//         }
//         ATDecimalOrDerived::Ptr FQilla_ItemFactory::createNonNegativeInteger(const MAPM value, const DynamicContext* context)
//         {
//             return m_ifactory->createNonNegativeInteger( value, context );
//         }
            

//         ATDecimalOrDerived::Ptr FQilla_ItemFactory::createDecimal(const MAPM value, const DynamicContext* context)
//         {
//             return m_ifactory->createDecimal( value, context );
//         }
//         ATDecimalOrDerived::Ptr FQilla_ItemFactory::createDecimal(const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createDecimal( value, context );
//         }
//         ATDecimalOrDerived::Ptr FQilla_ItemFactory::createDecimalOrDerived(const XMLCh* typeURI, const XMLCh* typeName,
//                                                                            const MAPM value, const DynamicContext* context)
//         {
//             return m_ifactory->createDecimalOrDerived( typeURI, typeName, value, context );
//         }
//         ATDecimalOrDerived::Ptr FQilla_ItemFactory::createDecimalOrDerived(const XMLCh* typeURI, const XMLCh* typeName,
//                                                                            const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createDecimalOrDerived( typeURI, typeName, value, context );
//         }
            

        
//         ATDateOrDerived::Ptr FQilla_ItemFactory::createDate(const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createDate( value, context );
//         }
//         ATDateOrDerived::Ptr FQilla_ItemFactory::createDateOrDerived(const XMLCh* typeURI, const XMLCh* typeName,
//                                                                      const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createDateOrDerived( typeURI, typeName, value, context );
//         }

//         ATDateTimeOrDerived::Ptr FQilla_ItemFactory::createDateTime(const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createDateTime( value, context );
//         }
//         ATDateTimeOrDerived::Ptr FQilla_ItemFactory::createDateTimeOrDerived(const XMLCh* typeURI, const XMLCh* typeName,
//                                                                              const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createDateTimeOrDerived( typeURI, typeName, value, context );
//         }


//         ATTimeOrDerived::Ptr FQilla_ItemFactory::createTime(const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createTime( value, context );
//         }
//         ATTimeOrDerived::Ptr FQilla_ItemFactory::createTimeOrDerived(const XMLCh* typeURI, const XMLCh* typeName,
//                                                                      const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createTimeOrDerived( typeURI, typeName, value, context );
//         }
            

//         ATGDayOrDerived::Ptr FQilla_ItemFactory::createGDayOrDerived(const XMLCh* typeURI, const XMLCh* typeName,
//                                                                      const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createGDayOrDerived( typeURI, typeName, value, context );
//         }
//         ATGMonthDayOrDerived::Ptr FQilla_ItemFactory::createGMonthDayOrDerived(const XMLCh* typeURI, const XMLCh* typeName,
//                                                                                const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createGMonthDayOrDerived( typeURI, typeName, value, context );
//         }
//         ATGMonthOrDerived::Ptr FQilla_ItemFactory::createGMonthOrDerived(const XMLCh* typeURI, const XMLCh* typeName,
//                                                                          const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createGMonthOrDerived( typeURI, typeName, value, context );
//         }
//         ATGYearMonthOrDerived::Ptr FQilla_ItemFactory::createGYearMonthOrDerived(const XMLCh* typeURI, const XMLCh* typeName,
//                                                                                  const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createGYearMonthOrDerived( typeURI, typeName, value, context );
//         }
//         ATGYearOrDerived::Ptr FQilla_ItemFactory::createGYearOrDerived(const XMLCh* typeURI, const XMLCh* typeName,
//                                                                        const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createGYearOrDerived( typeURI, typeName, value, context );
//         }

        
//         ATDurationOrDerived::Ptr FQilla_ItemFactory::createDayTimeDuration(const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createDayTimeDuration( value, context );
//         }
            
//         ATDurationOrDerived::Ptr FQilla_ItemFactory::createDayTimeDuration(const MAPM &seconds, const DynamicContext* context)
//         {
//             return m_ifactory->createDayTimeDuration( seconds, context );
//         }
//         ATDurationOrDerived::Ptr FQilla_ItemFactory::createYearMonthDuration(const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createYearMonthDuration( value, context );
//         }
//         ATDurationOrDerived::Ptr FQilla_ItemFactory::createYearMonthDuration(const MAPM &months, const DynamicContext* context)
//         {
//             return m_ifactory->createYearMonthDuration( months, context );
//         }
//         ATDurationOrDerived::Ptr FQilla_ItemFactory::createDurationOrDerived(const XMLCh* typeURI, const XMLCh* typeName,
//                                                                              const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createDurationOrDerived( typeURI, typeName, value, context );
//         }
            

//         ATStringOrDerived::Ptr FQilla_ItemFactory::createString(const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createString( value, context );
//         }
//         ATStringOrDerived::Ptr FQilla_ItemFactory::createStringOrDerived(const XMLCh* typeURI, const XMLCh* typeName,
//                                                                          const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createStringOrDerived( typeURI, typeName, value, context );
//         }
            

//         ATUntypedAtomic::Ptr FQilla_ItemFactory::createUntypedAtomic(const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createUntypedAtomic( value, context );
//         }

//         ATBooleanOrDerived::Ptr FQilla_ItemFactory::createBoolean(bool value, const DynamicContext* context)
//         {
//             return m_ifactory->createBoolean( value, context );
//         }
//         ATBooleanOrDerived::Ptr FQilla_ItemFactory::createBoolean(const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createBoolean( value, context );
//         }
//         ATBooleanOrDerived::Ptr FQilla_ItemFactory::createBooleanOrDerived(const XMLCh* typeURI, const XMLCh* typeName,
//                                                                            bool value, const DynamicContext* context)
//         {
//             return m_ifactory->createBooleanOrDerived( typeURI, typeName, value, context );
//         }
//         ATBooleanOrDerived::Ptr FQilla_ItemFactory::createBooleanOrDerived(const XMLCh* typeURI, const XMLCh* typeName,
//                                                                            const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createBooleanOrDerived( typeURI, typeName, value, context );
//         }

//         ATAnyURIOrDerived::Ptr FQilla_ItemFactory::createAnyURI(const XMLCh* value, const DynamicContext* context)
//         {
//             return m_ifactory->createAnyURI( value, context );
//         }

//         ATQNameOrDerived::Ptr FQilla_ItemFactory::createQName(const XMLCh* uri, const XMLCh* prefix, const XMLCh* name,
//                                                               const DynamicContext* context)
//         {
//             return m_ifactory->createQName( uri, prefix, name, context );
//         }
            
//         ATQNameOrDerived::Ptr FQilla_ItemFactory::createQNameOrDerived(const XMLCh* typeURI, const XMLCh* typeName,
//                                                                        const XMLCh* uri, const XMLCh* prefix,
//                                                                        const XMLCh* name, const DynamicContext* context)
//         {
//             return m_ifactory->createQNameOrDerived( typeURI, typeName, uri, prefix, name, context );
//         }
            

//         DOMDocument*
//         FQilla_ItemFactory::getOutputDocument(const DynamicContext *context) const
//         {
// //            getDOM( context );
// //            return GetImpl( m_outputDocument );
//             return ((ItemFactoryImpl*)m_ifactory)->getOutputDocument( context );
//         }
            
        
// //         fh_domdoc
// //         FQilla_ItemFactory::getDOM( const DynamicContext *context ) const
// //         {
// //             return 0;
            
// // //             if( !m_outputDocument )
// // //             {
// // //                 m_outputDocument = Factory::makeDOM( "" );
// // //             }
            
// // //             return m_outputDocument;
            

// // //              if( m_outputDocument )
// // //                  return m_outputDocument;

// // //              m_outputDocument = getOutputDocument( context );

// // // //             static list< fh_domdoc > explicitLeakList;
// // // //             explicitLeakList.push_back( m_outputDocument );
// // //              return m_outputDocument;
// //         }
        


        
            
            
// //         Ferris_XQDynamicContextImpl::Ferris_XQDynamicContextImpl(
// //             const StaticContext *staticContext,
// //             XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr)
// //             :
// //             XQDynamicContextImpl( staticContext, memMgr )
// //         {
// //         }
        
// //             Ferris_XQDynamicContextImpl::~Ferris_XQDynamicContextImpl()
// //         {
// //         }
        
        
        /////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////


        DocumentCache*
        FerrisXQillaConfiguration::createDocumentCache(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr)
        {
            return new (memMgr) DocumentCacheImpl(memMgr);
        }
        
        SequenceBuilder*
        FerrisXQillaConfiguration::createSequenceBuilder(const DynamicContext *context)
        {
//            return new (context->getMemoryManager()) XercesSequenceBuilder(context);
            return new (context->getMemoryManager()) FastXDMSequenceBuilder(context);
            
        }
        
            
        ItemFactory*
        FerrisXQillaConfiguration::createItemFactory(DocumentCache *cache,
                                                  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr)
        {
            return new (memMgr) ItemFactoryImpl(cache, memMgr);
        }


        class XQILLA_API FerrisXQillaUpdateFactory
            :
            public UpdateFactory
        {
        public:
            virtual void applyPut(const PendingUpdate &update, DynamicContext *context);
            virtual void applyInsertInto(const PendingUpdate &update, DynamicContext *context);
            virtual void applyInsertAttributes(const PendingUpdate &update, DynamicContext *context);
            virtual void applyReplaceValue(const PendingUpdate &update, DynamicContext *context);
            virtual void applyRename(const PendingUpdate &update, DynamicContext *context);
            virtual void applyDelete(const PendingUpdate &update, DynamicContext *context);
            virtual void applyInsertBefore(const PendingUpdate &update, DynamicContext *context);
            virtual void applyInsertAfter(const PendingUpdate &update, DynamicContext *context);
            virtual void applyInsertAsFirst(const PendingUpdate &update, DynamicContext *context);
            virtual void applyInsertAsLast(const PendingUpdate &update, DynamicContext *context);
            virtual void applyReplaceNode(const PendingUpdate &update, DynamicContext *context);
            virtual void applyReplaceAttribute(const PendingUpdate &update, DynamicContext *context);
            virtual void applyReplaceElementContent(const PendingUpdate &update, DynamicContext *context);

            virtual void completeRevalidation(DynamicContext *context);
            virtual void completeDeletions(DynamicContext *context);
            virtual void completeUpdate(DynamicContext *context);
        };

        //////////

        static const XMLCh file_scheme[] = { chLatin_f, chLatin_i, chLatin_l, chLatin_e, 0 };
        static const XMLCh utf8_str[] = { chLatin_u, chLatin_t, chLatin_f, chDash, chDigit_8, 0 };

        void FerrisXQillaUpdateFactory::applyPut(const PendingUpdate &update, DynamicContext *context)
        {
            LG_XQILLA_D << "FerrisXQillaUpdateFactory::applyPut()" << endl;
            assert(false);
        }

        void FerrisXQillaUpdateFactory::applyInsertInto(const PendingUpdate &update, DynamicContext *context)
        {
            LG_XQILLA_D << "applyInsertInto()" << endl;

            const FQilla_Node* nodeImpl = (const FQilla_Node*)update.getTarget()->getInterface(FQilla_Node::fQilla);
            LG_XQILLA_D << "applyInsertInto() nodeImpl:" << nodeImpl->getFerrisContext()->getURL() << endl;


            Result children = update.getValue();
            Item::Ptr item;
            while((item = children->next(context)).notNull())
            {
                const Node* childImpl = (const Node*)item->getInterface(FQilla_Node::fQilla);
                childImpl = (const Node*)item.get();

//                 {
//                     const Node* zz = 0;
//                     zz = (const Node*)item->getInterface(Item::gXQilla);
//                     LG_XQILLA_D << "zz1:" << zz << endl;
                    
//                     LG_XQILLA_D << "c0:" << (const Node*)item->getInterface(FQilla_Node::fQilla) << endl;
//                     LG_XQILLA_D << "c1:" << (const Node*)item->getInterface(fastxdm_string) << endl;
//                     LG_XQILLA_D << "c2:" << (const Node*)item->getInterface(XercesConfiguration::gXerces) << endl;
//                     LG_XQILLA_D << "c3:" << (const Node*)item->getInterface(Item::gXQilla) << endl;

//                     zz = (const Node*)item.get();
//                     LG_XQILLA_D << "zz2:" << zz << endl;
//                     const XMLCh* zzstr = 0;
//                     zzstr = zz->asString( context );
//                     LG_XQILLA_D << "zz2.str:" << tostr(zzstr) << endl;
//                 }

                string childImplString = tostr( childImpl->asString( context ) );
                
                // Deep copy childImpl filesystem into nodeImpl.
                LG_XQILLA_D << "applyInsertInto() copy to:" << nodeImpl->getFerrisContext()->getURL() << endl;
                LG_XQILLA_D << "applyInsertInto() data:" << childImplString << endl;
                
                {
                    ATQNameOrDerived::Ptr qname = childImpl->dmNodeName( context );
                    const XMLCh* childNameXC = qname->getName();
                    if( childNameXC )
                    {
                        string childName = tostr(childNameXC);
                        LG_XQILLA_D << "childName:" << childName << endl;

                        Result res = childImpl->dmChildren( context, 0 );
                        Item::Ptr resitem;
                        if((resitem = res->next(context)).notNull())
                        {
                            const Node* tnode = (const Node*)resitem.get();
                            if( tnode->dmNodeKind() == Node::text_string )
                            {
                                LG_XQILLA_D << "Child is a text node" << endl;
                                string content = tostr( tnode->asString( context ) );
                                LG_XQILLA_D << "CONTENT:" << content << endl;

                                fh_context parentc = nodeImpl->getFerrisContext();
                                LG_XQILLA_D << "Should create new child at:" << parentc->getURL() << endl;
                                LG_XQILLA_D << "Filename:" << childName << endl;
                                LG_XQILLA_D << "Contents:" << content << endl;

                                if( PERFORM_UPDATES )
                                {
                                    string rdn = parentc->monsterName( childName );
                                    LG_XQILLA_D << "Creating new child at:" << parentc->getURL() << endl;
                                    LG_XQILLA_D << "Filename:" << rdn << endl;
                                    LG_XQILLA_D << "Contents:" << content << endl;
                                            
                                    fh_context ctx = Shell::CreateFile( nodeImpl->getFerrisContext(), rdn );
                                    setStrAttr( ctx, "content", content, true, true );
                                    break;
                                }
                            }
                        }
                    }
                }
                
                
                
                LG_XQILLA_D << "FIXME!" << endl;
            }
            
        }

        void FerrisXQillaUpdateFactory::applyInsertAttributes(const PendingUpdate &update, DynamicContext *context)
        {
            LG_XQILLA_D << "FerrisXQillaUpdateFactory::applyInsertAttributes()" << endl;
            assert(false);
        }

        void FerrisXQillaUpdateFactory::applyReplaceValue(const PendingUpdate &update, DynamicContext *context)
        {
            LG_XQILLA_D << "FerrisXQillaUpdateFactory::applyReplaceValue()" << endl;
            assert(false);
        }

        void FerrisXQillaUpdateFactory::applyRename(const PendingUpdate &update, DynamicContext *context)
        {
            LG_XQILLA_D << "FerrisXQillaUpdateFactory::applyRename()" << endl;
            assert(false);
        }

        void FerrisXQillaUpdateFactory::applyDelete(const PendingUpdate &update, DynamicContext *context)
        {
            LG_XQILLA_D << "FerrisXQillaUpdateFactory::applyDelete()" << endl;
            assert(false);
        }

        void FerrisXQillaUpdateFactory::applyInsertBefore(const PendingUpdate &update, DynamicContext *context)
        {
            LG_XQILLA_D << "FerrisXQillaUpdateFactory::applyInsertBefore()" << endl;
            assert(false);
        }

        void FerrisXQillaUpdateFactory::applyInsertAfter(const PendingUpdate &update, DynamicContext *context)
        {
            LG_XQILLA_D << "FerrisXQillaUpdateFactory::applyInsertAfter()" << endl;
            assert(false);
        }

        void FerrisXQillaUpdateFactory::applyInsertAsFirst(const PendingUpdate &update, DynamicContext *context)
        {
            LG_XQILLA_D << "FerrisXQillaUpdateFactory::applyInsertAsFirst()" << endl;
            assert(false);
        }

        void FerrisXQillaUpdateFactory::applyInsertAsLast(const PendingUpdate &update, DynamicContext *context)
        {
            LG_XQILLA_D << "FerrisXQillaUpdateFactory::applyInsertAsLast()" << endl;
            assert(false);
        }

        void FerrisXQillaUpdateFactory::applyReplaceNode(const PendingUpdate &update, DynamicContext *context)
        {
            LG_XQILLA_D << "FerrisXQillaUpdateFactory::applyReplaceNode()" << endl;
            assert(false);
        }

        void FerrisXQillaUpdateFactory::applyReplaceAttribute(const PendingUpdate &update, DynamicContext *context)
        {
            LG_XQILLA_D << "FerrisXQillaUpdateFactory::applyReplaceAttribute()" << endl;
            assert(false);
        }

        void FerrisXQillaUpdateFactory::applyReplaceElementContent(const PendingUpdate &update, DynamicContext *context)
        {
            LG_XQILLA_D << "FerrisXQillaUpdateFactory::applyReplaceElementContent()" << endl;

            const FQilla_Node* nodeImpl = (const FQilla_Node*)update.getTarget()->getInterface(FQilla_Node::fQilla);
//            DOMElement *domnode = (DOMElement*)nodeImpl->getDOMNode();

            LG_XQILLA_D << "FerrisXQillaUpdateFactory::applyReplaceElementContent() nodeImpl:" << nodeImpl->getFerrisContext()->getURL() << endl;
            
//             // 1. For each node $C that is a child of $target, the parent property of $C is set to empty.
//             DOMNode *child = domnode->getFirstChild();
//             while(child != 0) {
//                 forDeletion_.insert(child);
//                 child = child->getNextSibling();
//             }

            const XMLCh *value = update.getValue().first()->asString(context);
            LG_XQILLA_D << "applyReplaceElementContent() have-value:" << (value!=0) << endl;
            if( value )
                LG_XQILLA_D << "applyReplaceElementContent() *value:" << (*value != 0) << endl;

            if(value != 0 && *value != 0)
            {
                // 2. The parent property of $text is set to $target.
                // 3a. children is set to consist exclusively of $text. If $text is an empty sequence, then $target has
                //     no children.
                // 3b. typed-value and string-value are set to the content property of $text. If $text is an empty sequence,
                //     then typed-value is an empty sequence and string-value is an empty string.

                LG_XQILLA_D << "applyReplaceElementContent() nodeImpl:" << nodeImpl->getFerrisContext()->getURL() << endl;
                LG_XQILLA_D << "applyReplaceElementContent() new-value:" << tostr(value) << endl;

                if( PERFORM_UPDATES )
                {
                    fh_context c = nodeImpl->getFerrisContext();
                    setStrAttr( c, "content", tostr(value) );
//                  domnode->appendChild(domnode->getOwnerDocument()->createTextNode(value));
                }
            }

            // 3c. upd:removeType($target) is invoked.
//            removeType(domnode);

//            addToPutList(domnode, &update, context);
        }

        void FerrisXQillaUpdateFactory::completeDeletions(DynamicContext *context)
        {
            LG_XQILLA_D << "FerrisXQillaUpdateFactory::completeDeletions()" << endl;
        }

        void FerrisXQillaUpdateFactory::completeRevalidation(DynamicContext *context)
        {
            LG_XQILLA_D << "FerrisXQillaUpdateFactory::completeRevalidation()" << endl;
        }

        void FerrisXQillaUpdateFactory::completeUpdate(DynamicContext *context)
        {
            LG_XQILLA_D << "FerrisXQillaUpdateFactory::completeUpdate(top)" << endl;
            completeDeletions(context);

            LG_XQILLA_D << "FerrisXQillaUpdateFactory::completeUpdate(body)" << endl;
        }

/////////        
        
        UpdateFactory*
        FerrisXQillaConfiguration::createUpdateFactory(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr)
        {
            return new (memMgr) FerrisXQillaUpdateFactory();
        }

        class FerrisXDMURIResolver
            :
            public URIResolver
        {
        public:
            FerrisXDMURIResolver(MemoryManager *mm) : _documentMap(3, mm) {}
            
            virtual bool putDocument(const RefCountPointer<const Node>&, const XMLCh*, DynamicContext*)
                {
                    cerr << "FIXME: URIResolver::putDocument() " << endl;
                    return false;
                }
            
    
            virtual bool resolveDocument(Sequence &result, const XMLCh* uri, DynamicContext* context, const QueryPathNode *projection)
                {
                    LG_XQILLA_D << "resolveDocument() uri:" << tostr(uri) << endl;
                    
                    fh_context m_ctx = Resolve( tostr(uri) );
                    FQilla_Node* qn = new FQilla_Node( context, m_ctx, "", true );
                    result.addItem( qn );
                    return true;
                    
                    
//                     Node::Ptr doc;

//                     // Resolve the uri against the base uri
//                     const XMLCh *systemId = uri;
//                     XMLURL urlTmp(context->getMemoryManager());
//                     if(urlTmp.setURL(context->getBaseURI(), uri, urlTmp)) {
//                         systemId = context->getMemoryManager()->getPooledString(urlTmp.getURLText());
//                     }

//                     // Check in the cache
//                     try {
//                         doc = _documentMap.get(systemId);
//                     }
//                     catch(NoSuchElementException &ex) {
//                     }

//                     // Check to see if we can locate and parse the document
//                     if(doc.isNull()) {
//                         try {
//                             doc = const_cast<DocumentCache*>(context->getDocumentCache())->loadDocument(uri, context, projection);
//                             _documentMap.put((void*)systemId, doc);
//                         }
//                         catch(const XMLParseException& e) {
//                             XMLBuffer errMsg;
//                             errMsg.set(X("Error parsing resource: "));
//                             errMsg.append(uri);
//                             errMsg.append(X(". Error message: "));
//                             errMsg.append(e.getError());
//                             errMsg.append(X(" [err:FODC0002]"));
//                             XQThrow2(XMLParseException,X("FastXDMContextImpl::resolveDocument"), errMsg.getRawBuffer());
//                         }
//                     }

//                     if(doc.notNull()) {
//                         result.addItem(doc);
//                         return true;
//                     }

//                     XMLBuffer errMsg;
//                     errMsg.set(X("Error retrieving resource: "));
//                     errMsg.append(uri);
//                     errMsg.append(X(" [err:FODC0002]"));
//                     XQThrow2(XMLParseException,X("FastXDMContextImpl::resolveDocument"), errMsg.getRawBuffer());

//                     return false;
                }

            virtual bool resolveCollection(Sequence &result, const XMLCh* uri, DynamicContext* context, const QueryPathNode *projection)
                {
                    return resolveDocument( result, uri, context, projection );
                }

            virtual bool resolveDefaultCollection(Sequence &result, DynamicContext* context, const QueryPathNode *projection)
                {
                    return false;
                }

        private:
            ValueHashTableOf<Node::Ptr> _documentMap;
        };
        
        URIResolver*
        FerrisXQillaConfiguration::createDefaultURIResolver(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr)
        {
            return new (memMgr) FerrisXDMURIResolver(memMgr);
        }
        
        /////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////
        
    };
};
