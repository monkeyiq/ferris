/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of libferris.

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

    $Id: XQilla_private.hh,v 1.13 2010/09/24 21:31:02 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_XQILLA_PRIVATE_PRIVH_
#define _ALREADY_INCLUDED_FERRIS_XQILLA_PRIVATE_PRIVH_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/FerrisBoost.hh>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/framework/StdInInputSource.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>

#include <xqilla/xqilla-dom3.hpp>
#include <xqilla/exceptions/XQException.hpp>

#include <xqilla/items/Node.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/context/impl/ItemFactoryImpl.hpp>

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/ast/XQFunction.hpp>

#include <xqilla/functions/FunctionLookup.hpp>
#include <xqilla/context/impl/XQContextImpl.hpp>
#include <xqilla/simple-api/XQillaConfiguration.hpp>

namespace Ferris
{
    namespace qilla
    {
        FERRISEXP_API std::string canonicalElementName( std::string rdn );
        FERRISEXP_API std::string canonicalElementName( fh_context c );
        class FQilla_ItemFactory;

        FERRISEXP_API void RegisterFerrisXQillaFunctions( StaticContext* context );


        
        class FERRISEXP_API ShouldIncludeAttribute
            :
            public Handlable
        {
            std::string   m_showColumnsRegexString;
            boost::regex  m_showColumnsRegex;
            stringset_t   m_showColumnsList;

            bool nameContainsIllegalXMLAttributeChar( const std::string& eaname );
            
        public:
            ShouldIncludeAttribute();

            
            void setShowColumns( const std::string& s );
            void setShowColumnsRegex( const std::string& s );

            
            bool shouldInclude( fh_context c, const std::string& eaname );
        };
        FERRIS_SMARTPTR( ShouldIncludeAttribute, fh_shouldIncludeAttribute );


        FERRISEXP_API fh_shouldIncludeAttribute getShouldIncludeAttribute( DynamicContext* context );
        FERRISEXP_API void setShouldIncludeAttribute( DynamicContext* context, fh_shouldIncludeAttribute sia );


        /////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////


        
        class FERRISEXP_API FQilla_Node : public Node
        {
            typedef FQilla_Node _Self;
            typedef Node _Base;
            friend class FQilla_ItemFactory;

            fh_context m_ctx;
            const std::string m_eaname;
            bool m_forceNoParent;
            bool m_isSingleTextContentsNode;
        
            mutable XERCES_CPP_NAMESPACE::DOMLSSerializer *m_fSerializer;
            
            void getAllChildren( fh_context c, ctxlist_t* cl ) const;

            typedef std::list< fh_domdoc > dom_cache_t;
            mutable dom_cache_t dom_cache;
            void cacheDOM( fh_domdoc ) const;

            bool isAttrNode() const;
        
        public:

            /** The "FQilla" item interface */
            static const XMLCh fQilla[];
        
            FQilla_Node( const DynamicContext* dynamicContext,
                         fh_context ctx,
                         const std::string& eaname = "",
                         bool forceNoParent = false,
                         bool m_isSingleTextContentsNode = false );
            virtual ~FQilla_Node();

            virtual void *getInterface(const XMLCh *name) const;
        
            virtual bool isNode() const;
            virtual bool isAtomicValue() const;
            virtual const XMLCh* asString(const DynamicContext* context) const;
            virtual bool hasInstanceOfType(const XMLCh* typeURI, const XMLCh* typeName, const DynamicContext* context) const;
            virtual Sequence dmBaseURI(const DynamicContext* context) const;
            virtual const XMLCh* dmNodeKind() const;
            virtual ATQNameOrDerived::Ptr dmNodeName(const DynamicContext* context) const;
            virtual const XMLCh* dmStringValue(const DynamicContext* context) const;
            virtual Sequence dmTypedValue(DynamicContext* context) const;
            virtual Sequence dmDocumentURI(const DynamicContext* context) const;
            virtual ATQNameOrDerived::Ptr dmTypeName(const DynamicContext* context) const;
            virtual ATBooleanOrDerived::Ptr dmNilled(const DynamicContext* context) const;
            virtual bool lessThan(const Node::Ptr &other, const DynamicContext *context) const;
            virtual bool equals(const Node::Ptr &other) const;
            virtual bool uniqueLessThan(const Node::Ptr &other, const DynamicContext *context) const;
            virtual Node::Ptr dmParent(const DynamicContext* context) const;
            virtual Result dmAttributes(const DynamicContext* context, const LocationInfo *info) const;
            virtual Result dmNamespaceNodes(const DynamicContext* context, const LocationInfo *info) const;
            virtual Result dmChildren(const DynamicContext *context, const LocationInfo *info) const;
            virtual Result getAxisResult(XQStep::Axis axis, const NodeTest *nodeTest, const DynamicContext *context, const LocationInfo *info) const;
            virtual ATBooleanOrDerived::Ptr dmIsId(const DynamicContext* context) const;
            virtual ATBooleanOrDerived::Ptr dmIsIdRefs(const DynamicContext* context) const;
            virtual const XMLCh* getTypeURI() const;
            virtual const XMLCh* getTypeName() const;

            fh_context getFerrisContext() const;

            virtual Node::Ptr root(const DynamicContext* context) const;
            virtual void generateEvents(EventHandler *events, const DynamicContext *context,
                                        bool preserveNS = true, bool preserveType = true) const;
        };

        ///////////////////////////////////////////////////
        ///////////////////////////////////////////////////
        ///////////////////////////////////////////////////
                            
        class FERRISEXP_API FunctionFerrisDoc : public XQFunction
        {
        public:
            static const XMLCh name[];
            static const unsigned int minArgs;
            static const unsigned int maxArgs;

            FunctionFerrisDoc(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr);
  
            virtual ASTNode* staticResolution(StaticContext *context);
            virtual ASTNode *staticTypingImpl(StaticContext *context);

            Sequence createSequence(DynamicContext* context, int flags=0) const;
        };
        

        class FERRISEXP_API FunctionFerrisDocQuiet : public XQFunction
        {
        public:
            static const XMLCh name[];
            static const unsigned int minArgs;
            static const unsigned int maxArgs;

            FunctionFerrisDocQuiet(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr);
  
            virtual ASTNode* staticResolution(StaticContext *context);
            virtual ASTNode *staticTypingImpl(StaticContext *context);

            Sequence createSequence(DynamicContext* context, int flags=0) const;
        };


        struct IndexLookupResult
        {
            // eavalue -> seq
            typedef std::map< std::string, Sequence > m_valueLookup_t;
            m_valueLookup_t m_valueLookup;
            
            IndexLookupResult();
            void insert( DynamicContext* context, const std::string& value, fh_context c );
            Sequence find( DynamicContext* context, const std::string& value );
        };

        
        class FERRISEXP_API FunctionFerrisIndexLookup : public XQFunction
        {
            // key=earl,eanamex
            typedef std::map< std::pair< std::string, std::string>, IndexLookupResult* > m_lookup_t;
            mutable m_lookup_t m_lookup;
            
        public:
            static const XMLCh name[];
            static const unsigned int minArgs;
            static const unsigned int maxArgs;

            FunctionFerrisIndexLookup(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr);
  
            virtual ASTNode* staticResolution(StaticContext *context);
            virtual ASTNode *staticTypingImpl(StaticContext *context);

            Sequence createSequence(DynamicContext* context, int flags=0) const;

            void populateLookup( DynamicContext* context, IndexLookupResult* l, fh_context ctx, const std::string eaname ) const;
            
        };
        
        ///////////////////////////////////////////////////
        ///////////////////////////////////////////////////
        ///////////////////////////////////////////////////

        class FERRISEXP_API FerrisXQillaConfiguration
            :
            public XQillaConfiguration
        {
        public:
            virtual DocumentCache *createDocumentCache(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr);
            
            virtual SequenceBuilder *createSequenceBuilder(const DynamicContext *context);
            
            virtual ItemFactory *createItemFactory(DocumentCache *cache,
                                                   XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr);
            
            virtual UpdateFactory *createUpdateFactory(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr);
            
            virtual URIResolver *createDefaultURIResolver(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr);
        };
        

        ///////////////////////////////////////////////////
        ///////////////////////////////////////////////////
        ///////////////////////////////////////////////////
        
        
//         class FQilla_ItemFactory : public ItemFactory
//         {
//             typedef FQilla_ItemFactory _Self;
//             typedef ItemFactoryImpl _Base;

//             ItemFactory* m_ifactory;
// //            mutable fh_domdoc m_outputDocument;
            
//         public:

//             FQilla_ItemFactory( ItemFactory* ifactory );
//             virtual ~FQilla_ItemFactory();

//             /* @name Node factory methods */

//             virtual Node::Ptr cloneNode(const Node::Ptr node, const DynamicContext *context) const;
//             virtual Node::Ptr createTextNode(const XMLCh *value, const DynamicContext *context) const;
//             virtual Node::Ptr createCommentNode(const XMLCh *value, const DynamicContext *context) const;
//             virtual Node::Ptr createPINode(const XMLCh *name, const XMLCh *value, const DynamicContext *context) const;
//             virtual Node::Ptr createAttributeNode(const XMLCh *uri, const XMLCh *prefix, const XMLCh *name,
//                                                   const XMLCh *value, const DynamicContext *context) const;
//             virtual Node::Ptr createElementNode(const XMLCh *uri, const XMLCh *prefix, const XMLCh *name,
//                                                 const std::vector<Node::Ptr> &attrList, const std::vector<ItemFactory::ElementChild> &childList,
//                                                 const DynamicContext *context) const;
//             virtual Node::Ptr createDocumentNode(const std::vector<Node::Ptr> &childList, const DynamicContext *context) const;
//             virtual const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* createNamespaceNode(const XMLCh* prefix, const XMLCh* uri, const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* parentNode, const DynamicContext *context) const;

//             /* @name Atomic type factory methods */

//             virtual AnyAtomicType::AtomicObjectType getPrimitiveTypeIndex(const XMLCh* typeURI, const XMLCh* typeName, bool &isPrimitive) const;
//             virtual AnyAtomicType::Ptr createDerivedFromAtomicType(AnyAtomicType::AtomicObjectType typeIndex, const XMLCh* value,
//                                                                    const DynamicContext* context);
//             virtual AnyAtomicType::Ptr createDerivedFromAtomicType(AnyAtomicType::AtomicObjectType typeIndex, const XMLCh* typeURI,
//                                                                    const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);
//             virtual AnyAtomicType::Ptr createDerivedFromAtomicType(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);

//             /* @name Number factory methods */

//             virtual ATDoubleOrDerived::Ptr createDouble(const MAPM value, const DynamicContext* context);
//             virtual ATDoubleOrDerived::Ptr createDouble(const XMLCh* value, const DynamicContext* context);
//             virtual ATDoubleOrDerived::Ptr createDoubleOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const MAPM value, const DynamicContext* context);
//             virtual ATDoubleOrDerived::Ptr createDoubleOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);

//             virtual ATFloatOrDerived::Ptr createFloat(const MAPM value, const DynamicContext* context);
//             virtual ATFloatOrDerived::Ptr createFloat(const XMLCh* value, const DynamicContext* context);
//             virtual ATFloatOrDerived::Ptr createFloatOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const MAPM value, const DynamicContext* context);
//             virtual ATFloatOrDerived::Ptr createFloatOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);

//             virtual ATDecimalOrDerived::Ptr createInteger(const int value, const DynamicContext* context);
//             virtual ATDecimalOrDerived::Ptr createInteger(const MAPM value, const DynamicContext* context);
//             virtual ATDecimalOrDerived::Ptr createInteger(const XMLCh* value, const DynamicContext* context);
//             virtual ATDecimalOrDerived::Ptr createNonNegativeInteger(const MAPM value, const DynamicContext* context);

//             virtual ATDecimalOrDerived::Ptr createDecimal(const MAPM value, const DynamicContext* context);
//             virtual ATDecimalOrDerived::Ptr createDecimal(const XMLCh* value, const DynamicContext* context);
//             virtual ATDecimalOrDerived::Ptr createDecimalOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const MAPM value, const DynamicContext* context);
//             virtual ATDecimalOrDerived::Ptr createDecimalOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);

//             /* @name Date / time factory methods */

//             virtual ATDateOrDerived::Ptr createDate(const XMLCh* value, const DynamicContext* context);
//             virtual ATDateOrDerived::Ptr createDateOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);

//             virtual ATDateTimeOrDerived::Ptr createDateTime(const XMLCh* value, const DynamicContext* context);
//             virtual ATDateTimeOrDerived::Ptr createDateTimeOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);

//             virtual ATTimeOrDerived::Ptr createTime(const XMLCh* value, const DynamicContext* context);
//             virtual ATTimeOrDerived::Ptr createTimeOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);

//             virtual ATGDayOrDerived::Ptr createGDayOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);
//             virtual ATGMonthDayOrDerived::Ptr createGMonthDayOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);
//             virtual ATGMonthOrDerived::Ptr createGMonthOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);
//             virtual ATGYearMonthOrDerived::Ptr createGYearMonthOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);
//             virtual ATGYearOrDerived::Ptr createGYearOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);

//             virtual ATDurationOrDerived::Ptr createDayTimeDuration(const XMLCh* value, const DynamicContext* context);
//             virtual ATDurationOrDerived::Ptr createDayTimeDuration(const MAPM &seconds, const DynamicContext* context);
//             virtual ATDurationOrDerived::Ptr createYearMonthDuration(const XMLCh* value, const DynamicContext* context);
//             virtual ATDurationOrDerived::Ptr createYearMonthDuration(const MAPM &months, const DynamicContext* context);
//             virtual ATDurationOrDerived::Ptr createDurationOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);

//             /* @name Other factory methods */

//             virtual ATStringOrDerived::Ptr createString(const XMLCh* value, const DynamicContext* context);
//             virtual ATStringOrDerived::Ptr createStringOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);

//             virtual ATUntypedAtomic::Ptr createUntypedAtomic(const XMLCh* value, const DynamicContext* context);

//             virtual ATBooleanOrDerived::Ptr createBoolean(bool value, const DynamicContext* context);
//             virtual ATBooleanOrDerived::Ptr createBoolean(const XMLCh* value, const DynamicContext* context);
//             virtual ATBooleanOrDerived::Ptr createBooleanOrDerived(const XMLCh* typeURI, const XMLCh* typeName, bool value, const DynamicContext* context);
//             virtual ATBooleanOrDerived::Ptr createBooleanOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);

//             virtual ATAnyURIOrDerived::Ptr createAnyURI(const XMLCh* value, const DynamicContext* context);

//             virtual ATQNameOrDerived::Ptr createQName(const XMLCh* uri, const XMLCh* prefix, const XMLCh* name, const DynamicContext* context);
//             virtual ATQNameOrDerived::Ptr createQNameOrDerived(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* uri, const XMLCh* prefix,
//                                                                const XMLCh* name, const DynamicContext* context);

//             XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *getOutputDocument(const DynamicContext *context) const;
// //            fh_domdoc getDOM( const DynamicContext *context ) const;
//         };

        
//         class Ferris_XQDynamicContextImpl : public XQDynamicContextImpl
//         {
//         public:
//             Ferris_XQDynamicContextImpl(const StaticContext *staticContext,
//                                         XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr);
//             virtual ~Ferris_XQDynamicContextImpl();
//         };

                                  
    };
};
#endif
