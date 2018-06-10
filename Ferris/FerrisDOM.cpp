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

    $Id: FerrisDOM.cpp,v 1.31 2011/11/09 21:31:12 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <config.h>

#include <Ferris.hh>
#include <FerrisDOM.hh>
#include <FerrisDOM_private.hh>
#include <Resolver_private.hh>
#include <Enamel.hh>
#include <Iterator.hh>

#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/framework/XMLRecognizer.hpp>
#include <xercesc/util/Base64.hpp>

#include <sys/types.h>
#include <unistd.h>

#ifdef HAVE_XALAN
#include <XalanTransformer/XalanTransformer.hpp>
#include <XalanDOM/XalanDOMException.hpp>
#include <PlatformSupport/XSLException.hpp>
#include <XalanTransformer/XalanTransformer.hpp>
#include <XercesParserLiaison/XercesDOMSupport.hpp>
#include <XercesParserLiaison/XercesParserLiaison.hpp>
#include <XalanTransformer/XercesDOMWrapperParsedSource.hpp>
#endif

#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/dom/DOMRange.hpp>

#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/ErrorHandler.hpp>

#include "xerces/DOMTreeWalkerImpl.hpp"
#include "xerces/DOMRangeImpl.hpp"
#include "xerces/DOMAttrImpl.hpp"
#include "xerces/DOMTypeInfoImpl.hpp"
#include "xerces/DOMDocumentImpl.hpp"


#include <string>

using namespace std;
using namespace XERCES_CPP_NAMESPACE;

#define STRING_OR_EMPTY( x )  (x) ? (x) : (XMLCh*)"";
#define X(str) XStr(str).unicodeForm()

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
/*** Support code to print a DOM (from DOMPrint.cpp) ***************************/
/*** From here down to the next three line comment block is apache code ********/
/*** though some of it is changed, so it should be considered tainted   ********/
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
// /*
//  * The Apache Software License, Version 1.1
//  *
//  * Copyright (c) 1999-2001 The Apache Software Foundation.  All rights
//  * reserved.
//  *
//  * Redistribution and use in source and binary forms, with or without
//  * modification, are permitted provided that the following conditions
//  * are met:
//  *
//  * 1. Redistributions of source code must retain the above copyright
//  *    notice, this list of conditions and the following disclaimer.
//  *
//  * 2. Redistributions in binary form must reproduce the above copyright
//  *    notice, this list of conditions and the following disclaimer in
//  *    the documentation and/or other materials provided with the
//  *    distribution.
//  *
//  * 3. The end-user documentation included with the redistribution,
//  *    if any, must include the following acknowledgment:
//  *       "This product includes software developed by the
//  *        Apache Software Foundation (http://www.apache.org/)."
//  *    Alternately, this acknowledgment may appear in the software itself,
//  *    if and wherever such third-party acknowledgments normally appear.
//  *
//  * 4. The names "Xerces" and "Apache Software Foundation" must
//  *    not be used to endorse or promote products derived from this
//  *    software without prior written permission. For written
//  *    permission, please contact apache\@apache.org.
//  *
//  * 5. Products derived from this software may not be called "Apache",
//  *    nor may "Apache" appear in their name, without prior written
//  *    permission of the Apache Software Foundation.
//  *
//  * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
//  * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//  * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
//  * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
//  * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//  * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
//  * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
//  * SUCH DAMAGE.
//  * ====================================================================
//  *
//  * This software consists of voluntary contributions made by many
//  * individuals on behalf of the Apache Software Foundation, and was
//  * originally based on software copyright (c) 1999, International
//  * Business Machines, Inc., http://www.ibm.com .  For more information
//  * on the Apache Software Foundation, please see
//  * <http://www.apache.org/>.
//  */

// static const XMLCh  gEndElement[] = { chOpenAngle, chForwardSlash, chNull };
// static const XMLCh  gEndPI[] = { chQuestion, chCloseAngle, chNull};
// static const XMLCh  gStartPI[] = { chOpenAngle, chQuestion, chNull };
// static const XMLCh  gXMLDecl1[] =
//     {
//         chOpenAngle, chQuestion, chLatin_x, chLatin_m, chLatin_l
//         ,   chSpace, chLatin_v, chLatin_e, chLatin_r, chLatin_s, chLatin_i
//         ,   chLatin_o, chLatin_n, chEqual, chDoubleQuote, chNull
//     };
//     static const XMLCh  gXMLDecl2[] =
//     {
//         chDoubleQuote, chSpace, chLatin_e, chLatin_n, chLatin_c
//         ,   chLatin_o, chLatin_d, chLatin_i, chLatin_n, chLatin_g, chEqual
//         ,   chDoubleQuote, chNull
//     };
//     static const XMLCh  gXMLDecl3[] =
//     {
//         chDoubleQuote, chSpace, chLatin_s, chLatin_t, chLatin_a
//         ,   chLatin_n, chLatin_d, chLatin_a, chLatin_l, chLatin_o
//         ,   chLatin_n, chLatin_e, chEqual, chDoubleQuote, chNull
//     };
//     static const XMLCh  gXMLDecl4[] =
//     {
//         chDoubleQuote, chQuestion, chCloseAngle
//         ,   chLF, chNull
//     };

//     static const XMLCh  gStartCDATA[] =
//     {
//         chOpenAngle, chBang, chOpenSquare, chLatin_C, chLatin_D,
//         chLatin_A, chLatin_T, chLatin_A, chOpenSquare, chNull
//     };

//     static const XMLCh  gEndCDATA[] =
//     {
//         chCloseSquare, chCloseSquare, chCloseAngle, chNull
//     };
//     static const XMLCh  gStartComment[] =
//     {
//         chOpenAngle, chBang, chDash, chDash, chNull
//     };

//     static const XMLCh  gEndComment[] =
//     {
//         chDash, chDash, chCloseAngle, chNull
//     };

//     static const XMLCh  gStartDoctype[] =
//     {
//         chOpenAngle, chBang, chLatin_D, chLatin_O, chLatin_C, chLatin_T,
//         chLatin_Y, chLatin_P, chLatin_E, chSpace, chNull
//     };
//     static const XMLCh  gPublic[] =
//     {
//         chLatin_P, chLatin_U, chLatin_B, chLatin_L, chLatin_I,
//         chLatin_C, chSpace, chDoubleQuote, chNull
//     };
//     static const XMLCh  gSystem[] =
//     {
//         chLatin_S, chLatin_Y, chLatin_S, chLatin_T, chLatin_E,
//         chLatin_M, chSpace, chDoubleQuote, chNull
//     };
//     static const XMLCh  gStartEntity[] =
//     {
//         chOpenAngle, chBang, chLatin_E, chLatin_N, chLatin_T, chLatin_I,
//         chLatin_T, chLatin_Y, chSpace, chNull
//     };
//     static const XMLCh  gNotation[] =
//     {
//         chLatin_N, chLatin_D, chLatin_A, chLatin_T, chLatin_A,
//         chSpace, chDoubleQuote, chNull
//     };

//     static XMLFormatter*            gFormatter             = 0;
//     static XMLCh*                   gEncodingName          = 0;
//     static XMLFormatter::UnRepFlags gUnRepFlags            = XMLFormatter::UnRep_CharRef;

// // ---------------------------------------------------------------------------
// //  Local classes
// // ---------------------------------------------------------------------------

//     class DOMPrintFormatTarget : public XMLFormatTarget
//     {
//         ostream& oss;
    
//     public:
//         DOMPrintFormatTarget( ostream& _oss = cout )
//             :
//             oss(_oss)
//             {};
//         ~DOMPrintFormatTarget() {};

//         // -----------------------------------------------------------------------
//         //  Implementations of the format target interface
//         // -----------------------------------------------------------------------

//         void writeChars(const   XMLByte* const  toWrite,
//                         const   unsigned int    count,
//                         XMLFormatter * const formatter)
//             {
//                 // Surprisingly, Solaris was the only platform on which
//                 // required the char* cast to print out the string correctly.
//                 // Without the cast, it was printing the pointer value in hex.
//                 // Quite annoying, considering every other platform printed
//                 // the string with the explicit cast to char* below.
//                 oss.write((char *) toWrite, (int) count);
//             };

//     private:
//         // -----------------------------------------------------------------------
//         //  Unimplemented methods.
//         // -----------------------------------------------------------------------
//         DOMPrintFormatTarget(const DOMPrintFormatTarget& other);
//         void operator=(const DOMPrintFormatTarget& rhs);
//     };

//     ostream& operator<<(ostream& target, const DOMString& toWrite);
//     ostream& operator<<(ostream& target, DOM_Node& toWrite);
//     XMLFormatter& operator<< (XMLFormatter& strm, const DOMString& s);

// // ---------------------------------------------------------------------------
// //  ostream << DOM_Node
// //
// //  Stream out a DOM node, and, recursively, all of its children. This
// //  function is the heart of writing a DOM tree out as XML source. Give it
// //  a document node and it will do the whole thing.
// // ---------------------------------------------------------------------------
//     ostream& operator<<(ostream& target, DOMNode* toWrite)
//     {
//         // Get the name and value out for convenience
//         const XMLCh* nodeName  = toWrite->getNodeName();
//         const XMLCh* nodeValue = toWrite->getNodeValue();
//         unsigned long lent = XMLString::stringLen( nodeValue );

//         switch (toWrite->getNodeType())
//         {
//         case DOMNode::TEXT_NODE:
//         {
//             gFormatter->formatBuf( nodeValue, lent, XMLFormatter::CharEscapes);
//             break;
//         }


//         case DOMNode::PROCESSING_INSTRUCTION_NODE :
//         {
//             *gFormatter << XMLFormatter::NoEscapes << gStartPI << nodeName;
//             if (lent > 0)
//             {
//                 *gFormatter << chSpace << nodeValue;
//             }
//             *gFormatter << XMLFormatter::NoEscapes << gEndPI;
//             break;
//         }

        
//         case DOMNode::DOCUMENT_NODE :
//         {
//             DOMNode* child = toWrite->getFirstChild();
//             while( child )
//             {
//                 target << child;
//                 // add linefeed in requested output encoding
//                 *gFormatter << chLF;
//                 target << flush;
//                 child = child->getNextSibling();
//             }
//             break;
//         }


//         case DOMNode::ELEMENT_NODE :
//         {
//             // The name has to be representable without any escapes
//             *gFormatter  << XMLFormatter::NoEscapes
//                          << chOpenAngle << nodeName;

//             // Output the element start tag.

//             // Output any attributes on this element
//             DOMNamedNodeMap* attributes = toWrite->getAttributes();
//             int attrCount = attributes->getLength();
//             for (int i = 0; i < attrCount; i++)
//             {
//                 DOMNode* attribute = attributes->item(i);

//                 //
//                 //  Again the name has to be completely representable. But the
//                 //  attribute can have refs and requires the attribute style
//                 //  escaping.
//                 //
//                 *gFormatter  << XMLFormatter::NoEscapes
//                              << chSpace << attribute->getNodeName()
//                              << chEqual << chDoubleQuote
//                              << XMLFormatter::AttrEscapes
//                              << attribute->getNodeValue()
//                              << XMLFormatter::NoEscapes
//                              << chDoubleQuote;
//             }

//             //
//             //  Test for the presence of children, which includes both
//             //  text content and nested elements.
//             //
//             DOMNode* child = toWrite->getFirstChild();
//             if ( child )
//             {
//                 // There are children. Close start-tag, and output children.
//                 // No escapes are legal here
//                 *gFormatter << XMLFormatter::NoEscapes << chCloseAngle;

//                 while( child )
//                 {
//                     target << child;
//                     child = child->getNextSibling();
//                 }

//                 //
//                 // Done with children.  Output the end tag.
//                 //
//                 *gFormatter << XMLFormatter::NoEscapes << gEndElement
//                             << nodeName << chCloseAngle;
//             }
//             else
//             {
//                 //
//                 //  There were no children. Output the short form close of
//                 //  the element start tag, making it an empty-element tag.
//                 //
//                 *gFormatter << XMLFormatter::NoEscapes << chForwardSlash << chCloseAngle;
//             }
//             break;
//         }


//         case DOMNode::ENTITY_REFERENCE_NODE:
//         {
//             DOMNode* child;
// #if 0
//             for (child = toWrite->getFirstChild(); child; child = child->getNextSibling())
//             {
//                 target << child;
//             }
// #else
//             //
//             // Instead of printing the refernece tree
//             // we'd output the actual text as it appeared in the xml file.
//             // This would be the case when -e option was chosen
//             //
//             *gFormatter << XMLFormatter::NoEscapes << chAmpersand
//                         << nodeName << chSemiColon;
// #endif
//             break;
//         }


//         case DOMNode::CDATA_SECTION_NODE:
//         {
//             *gFormatter << XMLFormatter::NoEscapes << gStartCDATA
//                         << nodeValue << gEndCDATA;
//             break;
//         }


//         case DOMNode::COMMENT_NODE:
//         {
//             *gFormatter << XMLFormatter::NoEscapes << gStartComment
//                         << nodeValue << gEndComment;
//             break;
//         }


//         case DOMNode::DOCUMENT_TYPE_NODE:
//         {
//             DOMDocumentType* doctype = (DOMDocumentType*)toWrite;

//             *gFormatter << XMLFormatter::NoEscapes  << gStartDoctype
//                         << nodeName;

//             const XMLCh* id = doctype->getPublicId();
//             if( id )
//             {
//                 *gFormatter << XMLFormatter::NoEscapes << chSpace << gPublic
//                             << id << chDoubleQuote;
//                 id = doctype->getSystemId() ? doctype->getSystemId() : (XMLCh*)"";
//                 if ( id )
//                 {
//                     *gFormatter << XMLFormatter::NoEscapes << chSpace
//                                 << chDoubleQuote << id << chDoubleQuote;
//                 }
//             }
//             else
//             {
//                 id = doctype->getSystemId();
//                 if( id )
//                 {
//                     *gFormatter << XMLFormatter::NoEscapes << chSpace << gSystem
//                                 << id << chDoubleQuote;
//                 }
//             }

//             id = doctype->getInternalSubset();
//             if( id )
//                 *gFormatter << XMLFormatter::NoEscapes << chOpenSquare
//                             << id << chCloseSquare;

//             *gFormatter << XMLFormatter::NoEscapes << chCloseAngle;
//             break;
//         }


//         case DOMNode::ENTITY_NODE:
//         {
//             DOMEntity* den = (DOMEntity*)toWrite;
            
//             *gFormatter << XMLFormatter::NoEscapes << gStartEntity
//                         << nodeName;

//             const XMLCh* id = den->getPublicId();
//             if ( id )
//                 *gFormatter << XMLFormatter::NoEscapes << gPublic
//                             << id << chDoubleQuote;
            
//             id = den->getSystemId();
//             if ( id )
//                 *gFormatter << XMLFormatter::NoEscapes << gSystem
//                             << id << chDoubleQuote;

//             id = den->getNotationName();
//             if ( id )
//                 *gFormatter << XMLFormatter::NoEscapes << gNotation
//                             << id << chDoubleQuote;

//             *gFormatter << XMLFormatter::NoEscapes << chCloseAngle << chLF;

//             break;
//         }


// //         case DOMNode::XML_DECL_NODE:
// //         {
// //             DOMString  str;

// //             *gFormatter << gXMLDecl1 << ((DOM_XMLDecl &)toWrite).getVersion();

// //             *gFormatter << gXMLDecl2 << gEncodingName;

// //             str = ((DOM_XMLDecl &)toWrite).getStandalone();
// //             if (str != 0)
// //                 *gFormatter << gXMLDecl3 << str;

// //             *gFormatter << gXMLDecl4;

// //             break;
// //         }


//         default:
//             LG_XML_W << "Unrecognized node type = "
//                      << (long)toWrite->getNodeType() << endl;
//         }
//         return target;
//     }

// // ---------------------------------------------------------------------------
// //  ostream << DOMString
// //
// //  Stream out a DOM string. Doing this requires that we first transcode
// //  to char * form in the default code page for the system
// // ---------------------------------------------------------------------------
// //     ostream& operator<< (ostream& target, const DOMString& s)
// //     {
// //         char *p = s.transcode();
// //         target << p;
// //         delete [] p;
// //         return target;
// //     }


// //     XMLFormatter& operator<< (XMLFormatter& strm, const DOMString& s)
// //     {
// //         unsigned int lent = s.length();

// //         if (lent <= 0)
// //             return strm;

// //         XMLCh*  buf = new XMLCh[lent + 1];
// //         XMLString::copyNString(buf, s.rawBuffer(), lent);
// //         buf[lent] = 0;
// //         strm << buf;
// //         delete [] buf;
// //         return strm;
// //     }


/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
// Here on in is all 100% (C) Ben Martin
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

namespace Ferris
{

    class FERRISEXP_DLLLOCAL StringStreamFormatTarget
        :
        public XMLFormatTarget
    {
        fh_ostream& ss;
        
    public:

        StringStreamFormatTarget( fh_ostream& ss )
            :
            ss( ss )
            {
            }
        virtual void writeChars( const XMLByte *const toWrite,
                                 const XMLSize_t count,
                                 XMLFormatter *const formatter )
            {
                ss.write((char *)toWrite, (int)count);
            }
    };
    

    class FERRISEXP_DLLLOCAL FerrisDOMPrintErrorHandler
        :
        public DOMErrorHandler
    {
        bool throwForErrors;
        
    public:
        
        FerrisDOMPrintErrorHandler( bool throwForErrors = false )
            :
            throwForErrors( throwForErrors )
            {};
        ~FerrisDOMPrintErrorHandler(){};

        /** @name The error handler interface */
        bool handleError(const DOMError& domError);
        void resetErrors(){};

    private :
        /* Unimplemented constructors and operators */
        FerrisDOMPrintErrorHandler(const DOMErrorHandler&);
        void operator=(const DOMErrorHandler&);
    };

    bool
    FerrisDOMPrintErrorHandler::handleError(const DOMError &domError)
    {
        char *msg_CSTR = XMLString::transcode(domError.getMessage());
        string msg = msg_CSTR;
        XMLString::release(&msg_CSTR);
        
        // Display whatever error message passed from the serializer
        if (domError.getSeverity() == DOMError::DOM_SEVERITY_WARNING)
            LG_XML_W << "Warning Message:" << msg << endl;
        else
        {
            if (domError.getSeverity() == DOMError::DOM_SEVERITY_ERROR)
            {
                LG_XML_I << "Error msg:" << msg << endl;
                if( throwForErrors )
                    Throw_XMLParseError( msg, 0 );
            }
            else
            {
                LG_XML_I << "Fatal msg:" << msg << endl;
                Throw_XMLFatalError( msg, 0 );
            }
        }

        // Instructs the serializer to continue serialization if possible.
        return true;
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


//     std::string tostr( const StrX& ds )
//     {
//         string ret = ds.localForm();
//         cerr << "std::string tostr( StrX ds ) lf:" << ds.localForm() << endl;
//         cerr << "std::string tostr( StrX ds ) ret:" << ret << endl;
//         return ret;
//     }

    std::string tostr( const XMLCh* xc )
    {
        if( !xc )
            return "";
        
        char* native_cstr = XMLString::transcode( xc );
        string ret = native_cstr;
        XMLString::release( &native_cstr );
        return ret;
    }
    
    std::string XMLToString( const XMLCh* xc, const std::string def )
    {
        if( !xc )
            return def;
        return tostr( xc );
    }
    

//     std::string tostr( const XMLString& ds )
//     {
//         ostringstream ss;
//         char* tmp = XMLString::transcode( ds );
//         ss << tmp;
//         XMLString::release( &tmp );
//         return tostr(ss);
//     }

//     std::string tostr( const Ferris::DOMString& s )
//     {
//         fh_stringstream ss;
//         ss << (char*)s.c_str();
//         return tostr(ss);
//     }
    
    namespace XML
    {
        static std::list< const DOMNode* > evalXPathGeneric( fh_domdoc doc,
                                                             std::string expression,
                                                             DOMXPathResult::ResultType  	type,
                                                             const DOMXPathNSResolver *  	resolver = 0 )
        {
            std::list< const DOMNode* > ret;

            DOMXPathResult* result = doc->evaluate( X(expression.c_str()),
                                                    doc->getDocumentElement(),
                                                    resolver,
                                                    type,
                                                    0 );

            cerr << "have result...type:" << result->getResultType() << endl;
            cerr << "snap length:" << result->getSnapshotLength() << endl;

            if( result->getResultType() == DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE )
            {
                XMLSize_t sz = result->getSnapshotLength();
                for( XMLSize_t i=0; i<sz; ++i )
                {
                    result->snapshotItem(i);
                    ret.push_back( result->getNodeValue() );
                }
                result->release();
                return ret;
            }
            
            
            if( result->getResultType() == DOMXPathResult::ORDERED_NODE_ITERATOR_TYPE
                || result->getResultType() == DOMXPathResult::FIRST_ORDERED_NODE_TYPE
                )
            {
                while( true )
                {
                    cerr << "have node..." << endl;
                    ret.push_back( result->getNodeValue() );
                    cerr << "have node2..." << endl;
                    if( result->getResultType() == DOMXPathResult::FIRST_ORDERED_NODE_TYPE
                        || !result->iterateNext() )
                        break;
                    cerr << "have node3..." << endl;
                }
            }
            cerr << "returning..." << endl;
            return ret;
        }
        
                                                             
        std::list< const DOMNode* > evalXPath( fh_domdoc doc,
                                               std::string expression )
        {
            std::list< const DOMNode* > ret;
            ret = evalXPathGeneric( doc, expression, DOMXPathResult::ORDERED_NODE_ITERATOR_TYPE );
            return ret;
        }

        std::list< const DOMElement* > evalXPathToElements( fh_domdoc doc,
                                                            std::string expression )
        {
            std::list< const DOMNode* > tmp =
                evalXPathGeneric( doc, expression, DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE );

            std::list< const DOMElement* > ret;
            for( std::list< const DOMNode* >::iterator iter = tmp.begin(); iter != tmp.end(); ++iter )
            {
                ret.push_back( (const DOMElement*)*iter );
            }
            
            return ret;
        }
        
        
        static std::string performManyReplaces( const std::string& s,
                                                const std::string& oldstr,
                                                const std::string& newstr )
        {
            string::size_type StartOfSearch = 0;
            string ret = s;
            string::size_type loc = string::npos;
            
            while( true )
            {
                loc = ret.find( oldstr, StartOfSearch );
                if( loc == string::npos )
                    break;

                StartOfSearch = max( StartOfSearch, loc+1 );
                if( newstr == "&amp;" && ret.substr( loc, 5 ) == newstr )
                    continue;
                
                ret.replace( loc, oldstr.length(), newstr );
            }
            return ret;
        }
        
        std::string escapeToXMLAttr( const std::string& s )
        {
            Factory::ensureXMLPlatformInitialized();
            
            string ret = s;

            ret = performManyReplaces( ret, "&", "&amp;" );
            ret = performManyReplaces( ret, "<", "&lt;" );
            ret = performManyReplaces( ret, ">", "&gt;" );
            ret = performManyReplaces( ret, "\"", "&quot;" );
            
            return ret;

//             DOMImplementation *impl = Factory::getDefaultDOMImpl();
//             DOMDocument* doc = impl->createDocument( 0, X("msg"), 0 );
//             DOMElement* root = doc->getDocumentElement();
//             setAttribute( root, "a", s );
//             string ret = getAttribute( root, "a" );
//             delete doc;
//             return ret;
        }
    };

    fh_stringstream& trimXMLDeclaration( fh_stringstream& ret, bool trim )
    {
//        cerr << "trimXMLDeclaration:" << trim << endl;
        if( !trim )
            return ret;

        string s;
        getline( ret, s );
        if( starts_with( s, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" ))
        {
//            cerr << "trimming!" << endl;
            
            fh_stringstream ss;
            copy( istreambuf_iterator<char>(ret),
                  istreambuf_iterator<char>(),
                  ostreambuf_iterator<char>(ss));
            ret.str( ss.str() );
        }
        
        ret.clear();
        ret.seekg(0);
        ret.clear();
        return ret;
    }
    
    
    fh_stringstream handleBadDeclaration( fh_stringstream ret )
    {
        string s;
        getline( ret, s );
        LG_XML_D << "first line:" << s << endl;
        if( starts_with( s, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?><" ))
        {
            LG_XML_D << "BAD first line:" << s << endl;
            fh_stringstream ss;
            ss << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n";
            ss << "<";

            ss << s.substr( strlen("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?><") ) << endl;
            copy( istreambuf_iterator<char>(ret),
                  istreambuf_iterator<char>(),
                  ostreambuf_iterator<char>(ss));
            ss.clear();
            ss.seekg(0);
            ss.clear();
            LG_XML_D << "BAD first line NEW STREAM sz:" << tostr(ss).size() << endl;
            LG_XML_D << "BAD first line NEW STREAM doc:" << tostr(ss) << endl;
            return ss;
        }

        ret.clear();
        ret.seekg(0);
        ret.clear();
        return ret;
    }
    
    fh_stringstream tostream( DOMNode& n, bool gFormatPrettyPrint )
    {
        const XMLCh*    gOutputEncoding  = 0;
        const XMLCh*    gMyEOLSequence   = 0;

        DOMImplementation*  impl = Factory::getDefaultDOMImpl();
        DOMLSSerializer* theSerializer = ((DOMImplementationLS*)impl)->createLSSerializer(); 
        DOMConfiguration*     dc = theSerializer->getDomConfig();

        theSerializer->setNewLine(gMyEOLSequence);
//        theSerializer->setEncoding(gOutputEncoding);

        DOMErrorHandler *myErrorHandler = new FerrisDOMPrintErrorHandler();
        dc->setParameter(XMLUni::fgDOMErrorHandler, myErrorHandler );
        dc->setParameter(XMLUni::fgDOMWRTDiscardDefaultContent, true ); 

        bool gSplitCdataSections    = true;
        bool gDiscardDefaultContent = true;
        bool gWriteBOM              = false;


        if (dc->canSetParameter(XMLUni::fgDOMWRTSplitCdataSections, gSplitCdataSections))
            dc->setParameter(XMLUni::fgDOMWRTSplitCdataSections, gSplitCdataSections);
        
        if (dc->canSetParameter(XMLUni::fgDOMWRTDiscardDefaultContent, gDiscardDefaultContent))
            dc->setParameter(XMLUni::fgDOMWRTDiscardDefaultContent, gDiscardDefaultContent);

        if (dc->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, gFormatPrettyPrint))
            dc->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, gFormatPrettyPrint);

        if (dc->canSetParameter(XMLUni::fgDOMWRTBOM, gWriteBOM))
            dc->setParameter(XMLUni::fgDOMWRTBOM, gWriteBOM);

        fh_stringstream ss;
        StringStreamFormatTarget* myFormTarget = new StringStreamFormatTarget( ss );

        DOMLSOutput* theOutput = ((DOMImplementationLS*)impl)->createLSOutput();
        theOutput->setByteStream(myFormTarget);
        theSerializer->write( &n, theOutput );
        
        delete theSerializer;        
        delete myFormTarget;
        delete myErrorHandler;
        return handleBadDeclaration(ss);


        
#if 0
        
        const XMLCh*    gOutputEncoding  = 0;
        const XMLCh*    gMyEOLSequence   = 0;

        // get a serializer, an instance of DOMWriter
        DOMImplementation *impl          = Factory::getDefaultDOMImpl();
        DOMWriter         *theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();

        // set user specified end of line sequence and output encoding
        theSerializer->setNewLine(gMyEOLSequence);
        theSerializer->setEncoding(gOutputEncoding);
        
        // plug in user's own error handler
        DOMErrorHandler *myErrorHandler = new FerrisDOMPrintErrorHandler();
        theSerializer->setErrorHandler(myErrorHandler);

        bool gSplitCdataSections    = true;
        bool gDiscardDefaultContent = true;
        bool gWriteBOM              = false;


        if (theSerializer->canSetFeature(XMLUni::fgDOMWRTSplitCdataSections, gSplitCdataSections))
            theSerializer->setFeature(XMLUni::fgDOMWRTSplitCdataSections, gSplitCdataSections);
        
        if (theSerializer->canSetFeature(XMLUni::fgDOMWRTDiscardDefaultContent, gDiscardDefaultContent))
            theSerializer->setFeature(XMLUni::fgDOMWRTDiscardDefaultContent, gDiscardDefaultContent);

        if (theSerializer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, gFormatPrettyPrint))
            theSerializer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, gFormatPrettyPrint);

        if (theSerializer->canSetFeature(XMLUni::fgDOMWRTBOM, gWriteBOM))
            theSerializer->setFeature(XMLUni::fgDOMWRTBOM, gWriteBOM);

        fh_stringstream ss;
        StringStreamFormatTarget* myFormTarget = new StringStreamFormatTarget( ss );
//        MemBufFormatTarget* myFormTarget = new MemBufFormatTarget( 64000 );


        
        theSerializer->writeNode( myFormTarget, n );




        
        
        
//         const XMLByte* membufdata  = myFormTarget->getRawBuffer();
//         unsigned int membufdatalen = myFormTarget->getLen();

//         fh_stringstream ss;
//         for( int i=0; i<membufdatalen; ++i )
//         {
//             ss << membufdata[i];
//         }
     
        delete theSerializer;        
        delete myFormTarget;
        delete myErrorHandler;

        return handleBadDeclaration(ss);
#endif
    }

    std::string tostr( fh_domdoc doc, bool gFormatPrettyPrint )
    {
        fh_stringstream ss = tostream( doc, gFormatPrettyPrint );
        return tostr(ss);
    }
    
    
    fh_stringstream tostream( fh_domdoc doc, bool gFormatPrettyPrint )
    {
        return tostream( *GetImpl(doc), gFormatPrettyPrint );
        
//         const XMLCh*    gOutputEncoding  = 0;
//         const XMLCh*    gMyEOLSequence   = 0;

//         // get a serializer, an instance of DOMWriter
//         DOMImplementation *impl          = Factory::getDefaultDOMImpl();
//         DOMWriter         *theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();

//         // set user specified end of line sequence and output encoding
//         theSerializer->setNewLine(gMyEOLSequence);
//         theSerializer->setEncoding(gOutputEncoding);
        
//         // plug in user's own error handler
//         DOMErrorHandler *myErrorHandler = new FerrisDOMPrintErrorHandler();
//         theSerializer->setErrorHandler(myErrorHandler);

//         bool gSplitCdataSections    = true;
//         bool gDiscardDefaultContent = true;
//         bool gWriteBOM              = false;


//         if (theSerializer->canSetFeature(XMLUni::fgDOMWRTSplitCdataSections, gSplitCdataSections))
//             theSerializer->setFeature(XMLUni::fgDOMWRTSplitCdataSections, gSplitCdataSections);
        
//         if (theSerializer->canSetFeature(XMLUni::fgDOMWRTDiscardDefaultContent, gDiscardDefaultContent))
//             theSerializer->setFeature(XMLUni::fgDOMWRTDiscardDefaultContent, gDiscardDefaultContent);

//         if (theSerializer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, gFormatPrettyPrint))
//             theSerializer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, gFormatPrettyPrint);

//         if (theSerializer->canSetFeature(XMLUni::fgDOMWRTBOM, gWriteBOM))
//             theSerializer->setFeature(XMLUni::fgDOMWRTBOM, gWriteBOM);

//         fh_stringstream ss;
//         StringStreamFormatTarget* myFormTarget = new StringStreamFormatTarget( ss );
// //        MemBufFormatTarget* myFormTarget = new MemBufFormatTarget( 64000 );


        
//         theSerializer->writeNode(myFormTarget, *doc);




        
        
        
// //         const XMLByte* membufdata  = myFormTarget->getRawBuffer();
// //         unsigned int membufdatalen = myFormTarget->getLen();

// //         fh_stringstream ss;
// //         for( int i=0; i<membufdatalen; ++i )
// //         {
// //             ss << membufdata[i];
// //         }
     
//         delete theSerializer;        
//         delete myFormTarget;
//         delete myErrorHandler;

//         return ss;

        /********************************************************************************/
        /********************************************************************************/

//         fh_stringstream ss;
//         DOMPrintFormatTarget* formatTarget = new DOMPrintFormatTarget( ss );
    
// //         if (gEncodingName == 0)
// //         {
// //             const XMLCh* encNameStr = X("UTF-8");
// //             DOMNode* aNode = doc->getFirstChild();
// // //             if (aNode.getNodeType() == DOMNode::XML_DECL_NODE)
// // //             {
// // //                 DOMString aStr = ((DOM_XMLDecl &)aNode).getEncoding();
// // //                 if (aStr != "")
// // //                 {
// // //                     encNameStr = aStr;
// // //                 }
// // //             }
// //             unsigned int lent = XMLString::stringLen( encNameStr );
// //             gEncodingName = new XMLCh[lent + 1];
// //             XMLString::copyNString( gEncodingName, encNameStr, lent );
// //             gEncodingName[lent] = 0;
// //         }

    
//         gFormatter = new XMLFormatter(X("UTF-8"), formatTarget,
//                                       XMLFormatter::NoEscapes, gUnRepFlags);
//         ss << doc;
//         *gFormatter << chLF; // add linefeed in requested output encoding
//         ss << flush;

//         delete gFormatter;

//         cerr << "tostream() ss:" << tostr(ss) << endl;
        
        
//         return ss;
        
    }

    namespace Private
    {
        
        class FERRISEXP_DLLLOCAL XMLPlatformUtilsKeeper 
        {
            XMLPlatformUtilsKeeper( const XMLPlatformUtilsKeeper &);
            XMLPlatformUtilsKeeper& operator=( const XMLPlatformUtilsKeeper &);
            
        public:
    
            XMLPlatformUtilsKeeper()
                {
                    // Initialize the XML4C2 system
                    try
                    {
//                         cerr << "XMLPlatformUtilsKeeper()" << endl;
//                         BackTrace();
                        
                        XMLPlatformUtils::Initialize();
#ifdef HAVE_XALAN
                        XALAN_CPP_NAMESPACE::XalanTransformer::initialize();
#endif
                    }
                    catch(const XMLException& toCatch)
                    {
                        fh_stringstream ss;
                        ss << "Error during Xerces-c Initialization.\n"
                           << "  Exception message:"
                           << toCatch.getMessage() << endl;
                        Throw_RootContextCreationFailed( tostr(ss), 0 );
                    }
                }

            ~XMLPlatformUtilsKeeper()
                {
// #ifdef HAVE_XALAN
//                     XalanTransformer::terminate();
// #endif
//                     XMLPlatformUtils::Terminate();
                }
        };

        typedef Loki::SingletonHolder<
            XMLPlatformUtilsKeeper,
            Loki::CreateUsingNew,
            Loki::NoDestroy
            > XMLPlatformUtilsSingleton;

        /*******************************************************************************/
        /*******************************************************************************/
        /*******************************************************************************/

        void addAllAttributes( DOMDocument* doc, DOMElement* e, fh_context root, fh_context c )
        {
            typedef AttributeCollection::AttributeNames_t ant;
            ant an;
            c->getAttributeNames( an );
            
            for( ant::iterator iter = an.begin(); iter != an.end(); ++iter )
            {
                e->setAttribute( X(iter->c_str()),
                                 X(getStrAttr( c, *iter, "unknown" ).c_str()) );
            }
        }
        

        DOMElement* addDOMNode( DOMDocument* doc, DOMElement* e, fh_context root, fh_context c )
        {
            LG_XML_D << "addDOMNode(1) c:" << c->getURL() << endl;

            string rdn = getStrAttr( c, "name", "undefined" );
            LG_XML_D << "addDOMNode(1a) c:" << c->getURL() << endl;
            LG_XML_D << "addDOMNode(1a) rdn:" << rdn << endl;

            DOMElement* childe;
            
            try
            {
                if( rdn.length() && isdigit(rdn[0]) )
                {
                    rdn = "x" + rdn;
                    LG_XML_D << "addDOMNode(1b) rdn:" << rdn << endl;
                }

                rdn = "context";
                childe = doc->createElement( X(rdn.c_str()) );
            }
            catch( DOMException& e )
            {
                LG_XML_D << "dom e" << endl;
                LG_XML_D << "Cought DOM exception:" << e.code << endl;
            }
            
                
            LG_XML_D << "addDOMNode(1b) c:" << c->getURL() << endl;
            e->appendChild(childe);

            LG_XML_D << "addDOMNode(2) c:" << c->getURL() << endl;
            addAllAttributes( doc, childe, root, c );
            
            LG_XML_D << "addDOMNode(3) c:" << c->getURL() << endl;

            fh_istream ss = c->getIStream();
            DOMText* childVal = doc->createTextNode( X(StreamToString(ss).c_str()) );
            childe->appendChild( childVal );

            return childe;
        }


        void addAllContexts( DOMDocument* doc, DOMElement* e, fh_context root, fh_context c )
        {
            typedef ContextCollection::SubContextNames_t cnt;
            cnt cn = c->getSubContextNames();

            LG_XML_D << "addAllContexts() c:" << c->getURL() << endl;
 
            for( cnt::iterator iter = cn.begin(); iter != cn.end(); ++iter )
            {
                fh_context child = c->getSubContext( *iter );
                LG_XML_D << "addAllContexts() child:" << child->getURL() << endl;

                DOMElement* childe = addDOMNode( doc, e, root, child );
                addAllContexts( doc, childe, root, child );
            }
        }
        
        
    };
    
    namespace Factory
    {
        /**
         * If one only uses xml4c from ferris and not directly then this
         * method need not be used. This is provided so that a program can
         * use ferris to negotiate XML and also the xml4c API direclty.
         * If you wish direct xml4c API use call here first to make sure
         * what xml4c is initialized, this method is impotent after the first
         * call. Not that this also initilizes xalan if it was around when
         * ferris was being built.
         */
        void ensureXMLPlatformInitialized()
        {
            ::Ferris::Private::XMLPlatformUtilsKeeper& u = 
                  ::Ferris::Private::XMLPlatformUtilsSingleton::Instance();
        }
    };
    

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

    
        static void throw_xml( int c )
        {
            throw DOMException( c, 0 );
        }

static short myCompareDocumentPosition(const DOMNode* x1, const DOMNode* other) 
{
    const DOMNode* thisNode = x1;

    // If the nodes are the same...
    if (thisNode == other)
        return 0;

    // If either node is of type ENTITY or NOTATION, compare as disconnected
    XERCES_CPP_NAMESPACE::DOMNode::NodeType thisType = thisNode->getNodeType();
    XERCES_CPP_NAMESPACE::DOMNode::NodeType otherType = other->getNodeType();

    // If either node is of type ENTITY or NOTATION, compare as disconnected
    if (thisType == DOMNode::ENTITY_NODE ||
            thisType == DOMNode::NOTATION_NODE ||
            otherType == DOMNode::ENTITY_NODE ||
            otherType == DOMNode::NOTATION_NODE )
    {
        return DOMNode::DOCUMENT_POSITION_DISCONNECTED;
    }

    //if this is a custom node, we don't really know what to do, just return
    //user should provide its own compareTreePosition logic, and shouldn't reach here
    if(thisType > 12) {
        return 0;
    }

    //if it is a custom node we must ask it for the order
    if(otherType > 12) {
//        cerr << "Custom node compare!" << endl;
        return 0;
    }

    // Find the ancestor of each node, and the distance each node is from
    // its ancestor.
    // During this traversal, look for ancestor/descendent relationships
    // between the 2 nodes in question.
    // We do this now, so that we get this info correct for attribute nodes
    // and their children.

    const DOMNode *node;
    const DOMNode *thisAncestor = x1;
    const DOMNode *otherAncestor = other;
    int thisDepth=0;
    int otherDepth=0;
    for (node = x1; node != 0; node = node->getParentNode()) {
        thisDepth +=1;
        if (node == other)
        {
            // The other node is an ancestor of this one.
            return DOMNode::DOCUMENT_POSITION_PRECEDING;
        }
        thisAncestor = node;
    }

    for (node=other; node != 0; node = node->getParentNode()) {
        otherDepth +=1;
        if (node == x1)
        {
            // The other node is a descendent of the reference node.
            return DOMNode::DOCUMENT_POSITION_FOLLOWING;
        }
        otherAncestor = node;
    }


    const DOMNode *otherNode = other;

    XERCES_CPP_NAMESPACE::DOMNode::NodeType thisAncestorType = thisAncestor->getNodeType();
    XERCES_CPP_NAMESPACE::DOMNode::NodeType otherAncestorType = otherAncestor->getNodeType();

    // if the ancestor is an attribute, get owning element.
    // we are now interested in the owner to determine position.

     if (thisAncestorType == DOMNode::ATTRIBUTE_NODE)  {
//         cerr << "DOMNode::ATTRIBUTE_NODE" << endl;
         thisNode = ((DOMAttrImpl *)thisAncestor)->getOwnerElement();
     }
     if (otherAncestorType == DOMNode::ATTRIBUTE_NODE) {
         otherNode = ((DOMAttrImpl *)otherAncestor)->getOwnerElement();
     }

    // Before proceeding, we should check if both ancestor nodes turned
    // out to be attributes for the same element
    if (thisAncestorType == DOMNode::ATTRIBUTE_NODE &&
            otherAncestorType == DOMNode::ATTRIBUTE_NODE &&
            thisNode==otherNode)
    {
        return 0; // EQUIVALENT;
    }
    
    // Now, find the ancestor of the owning element, if the original
    // ancestor was an attribute

    if (thisAncestorType == DOMNode::ATTRIBUTE_NODE)
    {
        thisDepth=0;
        for (node=thisNode; node != 0; node = node->getParentNode())
        {
            thisDepth +=1;
            if (node == otherNode)
            {
                // The other node is an ancestor of the owning element
                return DOMNode::DOCUMENT_POSITION_PRECEDING;
            }
            thisAncestor = node;
        }
        for (node=otherNode; node != 0; node = node->getParentNode())
        {
            if (node == thisNode)
            {
                // The other node is an ancestor of the owning element
                return DOMNode::DOCUMENT_POSITION_FOLLOWING;
            }
        }
    }

    // Now, find the ancestor of the owning element, if the original
    // ancestor was an attribute
    if (otherAncestorType == DOMNode::ATTRIBUTE_NODE)
    {
        otherDepth=0;
        for (node=otherNode; node != 0; node = node->getParentNode())
        {
            otherDepth +=1;
            if (node == thisNode)
            {
                // The other node is a descendent of the reference
                // node's element
                return DOMNode::DOCUMENT_POSITION_FOLLOWING;
            }
            otherAncestor = node;
        }
        for (node=thisNode; node != 0; node = node->getParentNode())
        {
            if (node == otherNode)
            {
                // The other node is an ancestor of the owning element
                return DOMNode::DOCUMENT_POSITION_PRECEDING;
            }
        }
    }

    // thisAncestor and otherAncestor must be the same at this point,
    // otherwise, we are not in the same tree or document fragment
    if (thisAncestor != otherAncestor)
    {
//        cerr << "DOMNode::TREE_POSITION_DISCONNECTED 2" << endl;
        return DOMNode::DOCUMENT_POSITION_DISCONNECTED;
    }
    

    // Determine which node is of the greatest depth.
    if (thisDepth > otherDepth)
    {
        for (int i= 0 ; i < thisDepth - otherDepth; i++)
            thisNode = thisNode->getParentNode();
    }
    else
    {
        for (int i = 0; i < otherDepth - thisDepth; i++)
            otherNode = otherNode->getParentNode();
    }

    // We now have nodes at the same depth in the tree.  Find a common
    // ancestor.
    DOMNode *thisNodeP, *otherNodeP;
    for (thisNodeP = thisNode->getParentNode(),
                 otherNodeP = otherNode->getParentNode();
             thisNodeP != otherNodeP;)
    {
        thisNode = thisNodeP;
        otherNode = otherNodeP;
        thisNodeP = thisNodeP->getParentNode();
        otherNodeP = otherNodeP->getParentNode();
    }

    // See whether thisNode or otherNode is the leftmost
    for (DOMNode *current = thisNodeP->getFirstChild();
             current != 0;
             current = current->getNextSibling())
    {
        if (current == otherNode)
        {
            return DOMNode::DOCUMENT_POSITION_PRECEDING;
        }
        else if (current == thisNode)
        {
            return DOMNode::DOCUMENT_POSITION_FOLLOWING;
        }
    }
    // REVISIT:  shouldn't get here.   Should probably throw an
    // exception
    return 0;
}

    
        Ferris_DOMNodeList::Ferris_DOMNodeList()
        {}
        Ferris_DOMNodeList::~Ferris_DOMNodeList()
        {}
        
        void
        Ferris_DOMNodeList::push_back( DOMNode* n )
        {
            m_col.push_back( n );
        }

        DOMNode*
        Ferris_DOMNodeList::front() const
        {
            if( m_col.empty() )
                return 0;
            return m_col.front();
        }
        DOMNode*
        Ferris_DOMNodeList::back() const
        {
            if( m_col.empty() )
                return 0;
            return m_col.back();
        }
            
        DOMNode*
        Ferris_DOMNodeList::item(XMLSize_t index) const
        {
            if( index >= m_col.size() )
                return 0;
            return m_col[ index ];
        }
        XMLSize_t
        Ferris_DOMNodeList::getLength() const
        {
            return m_col.size();
        }

        /********************/
        /********************/
        /********************/
        
        static DOMNodeList* getNullNodeList()
        {
            static Ferris_DOMNodeList* ret = new Ferris_DOMNodeList();
            return ret;
        }

        /********************/
        /********************/
        /********************/

        Ferris_DOMNamedNodeMap::Ferris_DOMNamedNodeMap( const DOMNode *ownerNod )
        {
            LG_XML_D << "Ferris_DOMNamedNodeMap() this:" << (void*)this <<  endl;
        }
        Ferris_DOMNamedNodeMap::~Ferris_DOMNamedNodeMap()
        {
            LG_XML_D << "~Ferris_DOMNamedNodeMap() this:" << (void*)this <<  endl;
        }

        DOMNode*
        Ferris_DOMNamedNodeMap::setNamedItem(DOMNode *arg)
        {
            const XMLCh* kx = arg->getNodeName();
            string k        = tostr(kx);
            m_nodes[ k ]    = arg;
        }
            
        DOMNode*
        Ferris_DOMNamedNodeMap::item(XMLSize_t index) const
        {
            if( index > m_nodes.size() )
                return 0;
                    
            m_nodes_t::iterator ni = m_nodes.begin();
            advance( ni, index );
            return ni->second;
        }
            
        DOMNode*
        Ferris_DOMNamedNodeMap::getNamedItem(const XMLCh *xname) const
        {
            string k = tostr(xname);
            m_nodes_t::iterator ni = m_nodes.find( k );
            if( ni != m_nodes.end() )
                return ni->second;
            return 0;
        }
            
        XMLSize_t
        Ferris_DOMNamedNodeMap::getLength() const
        {
            return m_nodes.size();
        }
            
        DOMNode*
        Ferris_DOMNamedNodeMap::removeNamedItem(const XMLCh *xname)
        {
            string k = tostr(xname);
            m_nodes.erase( m_nodes.find( k ));
        }
            
        DOMNode*
        Ferris_DOMNamedNodeMap::getNamedItemNS(const XMLCh *namespaceURI,
                                               const XMLCh *localName) const
        {
            LG_DOM_D << "Ferris_DOMNamedNodeMap::getNamedItemNS() ns:" << tostr(namespaceURI)
                     << " local:" << tostr(localName) << endl;
            return getNamedItem( localName );
        }
            
        DOMNode*
        Ferris_DOMNamedNodeMap::setNamedItemNS(DOMNode *arg)
        {
            LG_DOM_D << "Ferris_DOMNamedNodeMap::setNamedItemNS()" << endl;
            return setNamedItem( arg );
        }
            
        DOMNode*
        Ferris_DOMNamedNodeMap::removeNamedItemNS(const XMLCh *namespaceURI,
                                                           const XMLCh *localName)
        {
            LG_DOM_D << "Ferris_DOMNamedNodeMap::removeNamedItemNS()" << endl;
            return removeNamedItem( localName );
        }

        /********************/
        /********************/
        /********************/
        /********************/
        /********************/
        /********************/
        /********************/
        /********************/
        /********************/
        

        Ferris_DOM_NamespaceImplier::Ferris_DOM_NamespaceImplier()
            :
            m_localName( 0 ),
            m_nsURI( 0 )
            {
            }


        Ferris_DOM_NamespaceImplier::~Ferris_DOM_NamespaceImplier()
            {
                if( m_localName > (XMLCh*)1 )
                    XMLString::release( &m_localName );
                if( m_nsURI > (XMLCh*)1 )
                    XMLString::release( &m_nsURI );
            }

        const XMLCh*
        Ferris_DOM_NamespaceImplier::common_getNamespaceURI( const XMLCh* name ) const
            {
//                return 0;
                
                if( m_nsURI == (XMLCh*)1 )
                    return 0;
                if( m_nsURI )
                    return m_nsURI;
                    
                string n = tostr( name );
                LG_DOM_D << __PRETTY_FUNCTION__ << " n:" << n << endl;

                if( starts_with( n, "http://" ) )
                {
                    int p = n.rfind("/");
                    string ret = n.substr( 0, p+1 );
                    LG_DOM_D << __PRETTY_FUNCTION__ << " ret:" << ret << endl;
                    m_nsURI = XMLString::transcode( ret.c_str() );
                    return m_nsURI;
                }
                m_nsURI = (XMLCh*)1;
                return 0;
            }

    const XMLCh*
    Ferris_DOM_NamespaceImplier::common_getLocalName( const XMLCh* name ) const 
                {
//                   return 0;

                    if( m_localName == (XMLCh*)1 )
                        return 0;
                    if( m_localName )
                        return m_localName;
                    
                    string n = tostr( name );
                    LG_DOM_D << __PRETTY_FUNCTION__ << " n:" << n << endl;

                    if( starts_with( n, "http://" ) )
                    {
                        int p = n.rfind("/");
                        string ret = n.substr( p+1 );
                        LG_DOM_D << __PRETTY_FUNCTION__ << " ret:" << ret << endl;
                        m_localName = XMLString::transcode( ret.c_str() );
                        return m_localName;
                    }

                    return name;
//                    return 0;
                }

    

    
        /********************/
        /********************/
        /********************/
        /********************/
        /********************/
        /********************/
        /********************/
        /********************/
        /********************/

    
//         class Ferris_DOMAttr;
//         class Ferris_DOMElement;
//         class Ferris_DOMDocument;
//         FERRIS_SMARTPTR( Ferris_DOMNodeList, fhx_nodelist );
//         FERRIS_SMARTPTR( Ferris_DOMNamedNodeMap, fhx_namednodemap );
//         FERRIS_SMARTPTR( Ferris_DOMAttr,     fhx_attr );
//         FERRIS_SMARTPTR( Ferris_DOMElement,  fhx_element );
//         FERRIS_SMARTPTR( Ferris_DOMDocument, fhx_doc );

    
    class FERRISEXP_DLLLOCAL Ferris_DOMAttr
            :
            public DOMAttr,
            public Handlable,
            public Ferris_DOM_NamespaceImplier
        {
            DOMElement*        m_element; //< element we are attached to
            fh_context         m_context;
            string             m_eaname;
            XMLCh*             m_xname;
            mutable XMLCh*     m_xvalue;
            
        protected:
            Ferris_DOMAttr(const Ferris_DOMAttr &) {};
            Ferris_DOMAttr & operator = (const Ferris_DOMAttr &) {return *this;};

        public:
            Ferris_DOMAttr( DOMElement* xelement, fh_context c, const std::string& eaname )
                :
                m_element( xelement ),
                m_context( c ),
                m_eaname( eaname ),
                m_xvalue( 0 )
                {
                    m_xname = XMLString::transcode( m_eaname.c_str() );
                }

            virtual ~Ferris_DOMAttr()
                {
                    if( m_xname )
                        XMLString::release( &m_xname );
                    if( m_xvalue )
                        XMLString::release( &m_xvalue );
                };

            virtual const XMLCh* getName() const
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  eaname:" << m_eaname << endl;
                    return m_xname;
                }
            
            
            virtual bool getSpecified() const
                {
                    return true;
                }
                    
            virtual const XMLCh* getValue() const
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  eaname:" << m_eaname
                             << " m_context:" << m_context->getURL()
                             << endl;
                    if( m_xvalue )
                        XMLString::release( &m_xvalue );
                    
                    string v = getStrAttr( m_context, m_eaname, "" );
                    m_xvalue = XMLString::transcode( v.c_str() );
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  v:" << v << endl;
                    return m_xvalue;
                }
            
            virtual void setValue(const XMLCh *xv)
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  eaname:" << m_eaname << endl;
                    string v = tostr(xv);
                    setStrAttr( m_context, m_eaname, v );
//                    changed();
                }
            
            // DOM2 //
            virtual DOMElement* getOwnerElement() const
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  eaname:" << m_eaname << endl;
                    return m_element;
                }
            
            // DOM3 //
            virtual bool isId() const
                {
                    return false;
                }
            

            virtual const DOMTypeInfo * getSchemaTypeInfo() const 
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  eaname:" << m_eaname << endl;
                    return &DOMTypeInfoImpl::g_DtdNotValidatedAttribute;
                }

            /********************************************************************************/
            /********************************************************************************/
            /********************************************************************************/
            // DOMNode //
            /********************************************************************************/
            /********************************************************************************/
            /********************************************************************************/

            virtual const XMLCh* getNodeName() const
                {
                    return getName();
                }
            
            virtual const XMLCh* getNodeValue() const
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  eaname:" << m_eaname << endl;
                    return getValue();
                }
            
            virtual XERCES_CPP_NAMESPACE::DOMNode::NodeType getNodeType() const
                {
//                    cerr << "Ferris_Attr getNodeType()" << endl;
                    return ATTRIBUTE_NODE;
                }
            
            virtual DOMNode* getParentNode() const
                {
                    return getOwnerElement();
                }
            virtual DOMNodeList    *getChildNodes() const
                {
                    return getNullNodeList();
                }
            virtual DOMNode* getFirstChild() const
                {
                    return 0;
                }
            virtual DOMNode* getLastChild() const
                {
                    return 0;
                }
            virtual DOMNode* getPreviousSibling() const
                {
                    return 0;
                }
            virtual DOMNode* getNextSibling() const 
                {
                    return 0;
                }
            virtual DOMNamedNodeMap *getAttributes() const
                {
                    return 0;
                }
            virtual DOMDocument* getOwnerDocument() const
                {
                    return m_element->getOwnerDocument();
                }
            
            virtual DOMNode* cloneNode(bool deep) const
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented!" << endl;
                    return 0;
                }
            
            virtual DOMNode* insertBefore(DOMNode *newChild,
                                          DOMNode *refChild)
                {
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            virtual DOMNode* replaceChild(DOMNode *newChild,
                                          DOMNode *oldChild)
                {
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            virtual DOMNode* removeChild(DOMNode *oldChild) 
                {
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            virtual DOMNode* appendChild(DOMNode *newChild)
                {
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            virtual bool hasChildNodes() const
                {
                    return false;
                }
            
            virtual void setNodeValue(const XMLCh  *nodeValue)
                {
                    setValue( nodeValue );
                }

            // DOM2 //

            virtual void normalize()
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented!" << endl;
                }
            virtual bool isSupported(const XMLCh *feature,
                                     const XMLCh *version) const
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented!" << endl;
                    return false;
                }
            string getNSURI( const XMLCh* input ) const
                {
                    return "";
                }
            virtual const XMLCh* getNamespaceURI() const
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << " n:" << tostr(getName()) << endl;
                    return common_getNamespaceURI( getName() );
                }
            
//                     if( m_nsURI == 1 )
//                         return 0;
//                     if( m_nsURI )
//                         return m_nsURI;
                    
// //                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented! n:" << tostr(getName()) << endl;

//                     string n = tostr( getName() );
//                     LG_DOM_D << __PRETTY_FUNCTION__ << " n:" << n << endl;

//                     if( starts_with( n, "http://" ) )
//                     {
//                         int p = n.rfind("/");
//                         string ret = n.substr( 0, p+1 );
//                         LG_DOM_D << __PRETTY_FUNCTION__ << " ret:" << ret << endl;
//                         m_nsURI = XMLString::transcode( ret.c_str() );
//                         return m_nsURI;
//                     }
//                     m_nsURI = 1;
//                     return 0;
//                 }
            virtual const XMLCh* getPrefix() const
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented!" << endl;
                    return 0;
                }
            virtual const XMLCh* getLocalName() const 
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << " name:" << tostr(getName()) << endl;
                    return common_getLocalName( getName() );
// //                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented!" << endl;

//                     string n = tostr( getName() );
//                     LG_DOM_D << __PRETTY_FUNCTION__ << " n:" << n << endl;

//                     if( starts_with( n, "http://" ) )
//                     {
//                         int p = n.rfind("/");
//                         string ret = n.substr( p+1 );
//                         LG_DOM_D << __PRETTY_FUNCTION__ << " ret:" << ret << endl;
//                         XMLCh* r = XMLString::transcode( ret.c_str() );
//                         return r;
//                     }
                    
//                     return 0;
                }
            virtual void setPrefix(const XMLCh * prefix)
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented!" << endl;
                }
            
            virtual bool hasAttributes() const
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented!" << endl;
                    return 0;
                }
            
            // DOM3 // 
            virtual bool isSameNode(const DOMNode* other) const
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented!" << endl;
                    return false;
                }
            virtual bool isEqualNode(const DOMNode* arg) const
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented!" << endl;
                    return false;
                }
            virtual void* setUserData(const XMLCh* key,
                                      void* data,
                                      DOMUserDataHandler* handler)
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented!" << endl;
                    return 0;
                }
            virtual void* getUserData(const XMLCh* key) const
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented!" << endl;
                    return 0;
                }
            virtual const XMLCh* getBaseURI() const
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented!" << endl;
                    return 0;
                }
            
            virtual short compareDocumentPosition(const DOMNode* other) const
                {
                    return myCompareDocumentPosition( this, other );
                }
            virtual const XMLCh* getTextContent() const
                {
                    string v = getStrAttr( m_context, m_eaname, "", true );
                    LG_DOM_D << __PRETTY_FUNCTION__ << " c:" << m_context->getURL()
                             << " v:" << v
                             << endl;
                    //return ((Ferris_DOMElement*)getOwnerElement())->cacheString( v ));
                    return XMLString::transcode( v.c_str() );
                }
            virtual void setTextContent(const XMLCh* textContent)
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            
            virtual const XMLCh* lookupPrefix( const XMLCh* namespaceURI ) const
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented!" << endl;
                    return 0;
                }
                    
            virtual bool isDefaultNamespace(const XMLCh* namespaceURI) const
                {
                    return true;
                }
            
            virtual const XMLCh* lookupNamespaceURI(const XMLCh* prefix) const
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented!" << endl;
                    return 0;
                }
            
            virtual void* getFeature(const XMLCh* feature, const XMLCh* version) const
                {
                    LG_DOM_D << __PRETTY_FUNCTION__ << "  not implemented!" << endl;
                    return 0;
                }
            

            // non-standard //
            virtual void release()
                {
                }
        };

        /********************/
        /********************/
        /********************/

    
        XMLCh*
        Ferris_DOMElement::cacheString( const std::string& s ) const
        {
            LG_DOM_D << "element::cacheString() s:" << s << endl;
            
            m_xcache_t::iterator xi = m_xcache.find( s );
            if( xi != m_xcache.end() )
            {
                return xi->second;
            }

            XMLCh* xs = XMLString::transcode( s.c_str() );
            m_xcache[ s ] = xs;
            return xs;
        }
            
            
        Ferris_DOMElement::Ferris_DOMElement( Ferris_DOMDocument* m_doc,
                                              DOMNode*  m_parent,
                                              fh_context c )
            :
            m_doc( m_doc ),
            m_parent( m_parent ),
            m_context( c ),
            m_cache_getChildNodes( 0 ),
            m_fAttributes( 0 )
        {
            LG_DOM_D << "element::element() c:" << c->getURL() << endl;
        }
        Ferris_DOMElement::~Ferris_DOMElement()
        {
            for( m_xcache_t::iterator xi = m_xcache.begin(); xi != m_xcache.end(); ++xi )
            {
                XMLString::release( &xi->second );
            }
        }

        const XMLCh*
        Ferris_DOMElement::getTagName() const
        {
            LG_DOM_D << "element::getTagName() c:" << m_context->getURL() << endl;
            return cacheString( m_context->getDirName() );
        }

        DOMAttr*
        Ferris_DOMElement::getAttributeNode(const XMLCh* xname) const
        {
            if( LG_DOM_ACTIVE )
            {
                string n = tostr( xname );
                LG_DOM_D << "element::getAttributeNode() c:" << m_context->getURL()
                         << " n:" << n
                         << endl;
            }

//            return getAttributeNode( tostr( xname ) );

            if( shouldHideAttribute( xname ) )
                return 0;
            return  (DOMAttr *)ensure_fAttributes()->getNamedItem( xname );
        }
            
        DOMNodeList*
        Ferris_DOMElement::getElementsByTagName(const XMLCh *name) const
        {
//            cerr << "element::getElementsByTagName() c:" << m_context->getURL() << endl;
            DOMDocumentImpl *docImpl = (DOMDocumentImpl *)getOwnerDocument();
            return docImpl->getDeepNodeList( this, name );
//             LG_DOM_D << "element::getElementsByTagName() c:" << m_context->getURL() << endl;
//             return 0;
        }

    void
        Ferris_DOMElement::setAttribute(const XMLCh *name, const XMLCh *value)
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::setAttribute() c:" << m_context->getURL() << endl;
            throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
        }
        DOMAttr*
        Ferris_DOMElement::setAttributeNode(DOMAttr *newAttr) 
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::setAttributeNode() c:" << m_context->getURL() << endl;
            throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
        }
        DOMAttr*
        Ferris_DOMElement::removeAttributeNode(DOMAttr *oldAttr) 
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::removeAttributeNode() c:" << m_context->getURL() << endl;
            throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
        }
        void
        Ferris_DOMElement::removeAttribute(const XMLCh *name)
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::removeAttribute() c:" << m_context->getURL() << endl;
            throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
        }

        // DOM2 // 
        const XMLCh*
        Ferris_DOMElement::getAttributeNS(const XMLCh *namespaceURI,
                                          const XMLCh *localName) const
        {
//            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::getAttributeNS() c:" << m_context->getURL() << endl;
//            return 0;
            
            DOMAttr * attr=
                (DOMAttr *)(ensure_fAttributes()->getNamedItemNS(namespaceURI, localName));
            return (attr==0) ? XMLUni::fgZeroLenString : attr->getValue();
        }
            
        void
        Ferris_DOMElement::setAttributeNS(const XMLCh *namespaceURI,
                                          const XMLCh *qualifiedName,
                                          const XMLCh *value)
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::setAttributeNS() c:" << m_context->getURL() << endl;
            throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
        }
        
        void
        Ferris_DOMElement::removeAttributeNS(const XMLCh *namespaceURI,
                                             const XMLCh *localName)
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::removeAttributeNS() c:" << m_context->getURL() << endl;
            throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
        }
        DOMAttr*
        Ferris_DOMElement::getAttributeNodeNS(const XMLCh *namespaceURI,
                                              const XMLCh *localName) const
        {
//            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::getAttributeNodeNS() c:" << m_context->getURL() << endl;
//            return 0;
            return (DOMAttr *)ensure_fAttributes()->getNamedItemNS(namespaceURI, localName);
            
        }
        
        DOMAttr*
        Ferris_DOMElement::setAttributeNodeNS(DOMAttr *newAttr)
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::setAttributeNodeNS() c:" << m_context->getURL() << endl;
            throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
        }
        DOMNodeList*
        Ferris_DOMElement::getElementsByTagNameNS(const XMLCh *namespaceURI,
                                                  const XMLCh *localName) const
        {
            DOMDocumentImpl *docImpl = (DOMDocumentImpl *)getOwnerDocument();;
            return docImpl->getDeepNodeList( this, namespaceURI, localName );
//             LG_DOM_D << "element::getElementsByTagNameNS() c:" << m_context->getURL() << endl;
//             return 0;
        }
        
        // DOM3 //
        void
        Ferris_DOMElement::setIdAttribute(const XMLCh* name, bool isId )
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::setIdAttribute() c:" << m_context->getURL() << endl;
            throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
        }
        void
        Ferris_DOMElement::setIdAttributeNS(const XMLCh* namespaceURI, const XMLCh* localName, bool isId) 
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::setIdAttributeNS() c:" << m_context->getURL() << endl;
            throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
        }
        void
        Ferris_DOMElement::setIdAttributeNode(const DOMAttr *idAttr, bool isId)
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::setIdAttributeNode() c:" << m_context->getURL() << endl;
            throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
        }
        const DOMTypeInfo*
        Ferris_DOMElement::getSchemaTypeInfo() const
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::getTypeInfo() c:" << m_context->getURL() << endl;
            return 0;
        }
        

        /***********/
        /***********/
        // DOMNode //
        /***********/
        /***********/

        const XMLCh*
        Ferris_DOMElement::getNodeName() const
        {
            LG_DOM_D << "element::getNodeName() c:" << m_context->getURL()
                     << " ret:" << tostr(getTagName())
                     << endl;
//                     cerr << "getNodeName(element) m_context:" << m_context->getURL()
//                          << " type:" << getNodeType()
//                          << endl;
            return getTagName();
        }
        
        const XMLCh*
        Ferris_DOMElement::getNodeValue() const
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::getNodeValue() c:" << m_context->getURL() << endl;
            return 0;
        }
        
        XERCES_CPP_NAMESPACE::DOMNode::NodeType
        Ferris_DOMElement::getNodeType() const
        {
            LG_DOM_D << "element::getNodeType() c:" << m_context->getURL() << endl;
//                    cerr << "Ferris_Element getNodeType()" << endl;
            return ELEMENT_NODE;
        }
        
        DOMNode*
        Ferris_DOMElement::getParentNode() const
        {
            LG_DOM_D << "element::getParentNode() c:" << m_context->getURL() << endl;
            return m_parent;
        }
        Ferris_DOMNodeList*
        Ferris_DOMElement::getChildNodes_sub() const
        {
            LG_DOM_D << "element::getChildNodes_sub() c:" << m_context->getURL()
                     << " m_cache_getChildNodes:" << toVoid( m_cache_getChildNodes )
                     << endl;

            if( m_cache_getChildNodes &&
                m_cache_getChildNodes->getLength() != m_context->getSubContextCount() )
            {
                m_cache_getChildNodes = 0;
            }
            
            if( !m_cache_getChildNodes )
            {
                m_cache_getChildNodes = new Ferris_DOMNodeList();
                try
                {
                    LG_DOM_D << "element::getChildNodes_sub(start) c:" << m_context->getURL()
                             << " count:" << m_context->getSubContextCount()
                             << endl;
                    for( Context::iterator ci = m_context->begin(); ci != m_context->end(); ++ci )
                    {
                        LG_DOM_D << "element::getChildNodes_sub() add:" << (*ci)->getURL() << endl;
                        Ferris_DOMElement* e = new Ferris_DOMElement( m_doc,
                                                                      (Ferris_DOMElement*)this,
                                                                      *ci );
                        m_cache_getChildNodes->push_back( e );
                        changed();
                    }
                }
                catch( exception& e )
                {
//                             cerr << "getChildNodes_sub() m_context:" << m_context->getURL()
//                                  << " e:" << e.what()
//                                  << endl;
                }
            }
            return GetImpl( m_cache_getChildNodes );
        }
        DOMNodeList*
        Ferris_DOMElement::getChildNodes() const
        {
            LG_DOM_D << "element::getChildNodes() c:" << m_context->getURL() << endl;
            return getChildNodes_sub();
        }
        DOMNode*
        Ferris_DOMElement::getFirstChild() const
        {
            LG_DOM_D << "element::getFirstChild() c:" << m_context->getURL() << endl;
            return getChildNodes_sub()->front();
        }
        DOMNode*
        Ferris_DOMElement::getLastChild() const
        {
            LG_DOM_D << "element::getLastChild() c:" << m_context->getURL() << endl;
            return getChildNodes_sub()->back();
        }
        DOMNode*
        Ferris_DOMElement::getPreviousSibling() const
        {
            LG_DOM_D << "element::getPreviousSibling() c:" << m_context->getURL() << endl;
            if( m_parent )
            {
                DOMNodeList* nl = m_parent->getChildNodes();
                for( int i=0; i < nl->getLength(); ++i )
                {
                    DOMNode* n = nl->item( i );
                    if( n == this && i )
                    {
                        return nl->item( i-1 );
                    }
                }
            }
            return 0;
        }
        DOMNode*
        Ferris_DOMElement::getNextSibling() const 
        {
            LG_DOM_D << "element::getNextSibling() c:" << m_context->getURL() << endl;
            if( m_parent )
            {
                DOMNodeList* nl = m_parent->getChildNodes();
                for( int i=0; i < nl->getLength(); ++i )
                {
                    DOMNode* n = nl->item( i );
                    if( n == this )
                    {
//                                 cerr << "getNextSibling() found ourself at i:" << i << endl;
//                                 cerr << "getNextSibling() ret:" << toVoid(nl->item( i+1 )) << endl;
                        return nl->item( i+1 );
                    }
                }
            }
            return 0;
        }
        DOMDocument*
        Ferris_DOMElement::getOwnerDocument() const
        {
            LG_DOM_D << "element::getOwnerDocument() c:" << m_context->getURL() << endl;
            return (DOMDocument*)m_doc;
        }
        
        DOMNode*
        Ferris_DOMElement::cloneNode(bool deep) const
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::cloneNode() c:" << m_context->getURL() << endl;
            return 0;
        }
        
        DOMNode*
        Ferris_DOMElement::insertBefore(DOMNode *newChild,
                                        DOMNode *refChild)
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::insertBefore() c:" << m_context->getURL() << endl;
            throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
        }
        DOMNode*
        Ferris_DOMElement::replaceChild(DOMNode *newChild,
                                        DOMNode *oldChild)
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::replaceChild() c:" << m_context->getURL() << endl;
            throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
        }
        DOMNode*
        Ferris_DOMElement::removeChild(DOMNode *oldChild) 
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::removeChild() c:" << m_context->getURL() << endl;
            throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
        }
        DOMNode*
        Ferris_DOMElement::appendChild(DOMNode *newChild)
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::appendChild() c:" << m_context->getURL() << endl;
            throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
        }
        bool
        Ferris_DOMElement::hasChildNodes() const
        {
            LG_DOM_D << "element::hasChildNodes() c:" << m_context->getURL()
                     << " ret:" << (m_context->begin() != m_context->end())
                     << endl;
            return m_context->begin() != m_context->end();
        }
        
        void
        Ferris_DOMElement::setNodeValue(const XMLCh  *nodeValue)
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::setNodeValue() c:" << m_context->getURL() << endl;
        }

    DOMElement* Ferris_DOMElement::getFirstElementChild() const
    {
        LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
        return 0;
    }
     
    DOMElement* Ferris_DOMElement::getLastElementChild() const
    {
        LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
        return 0;
    }
    DOMElement* Ferris_DOMElement::getPreviousElementSibling() const
    {
        LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
        return 0;
    }
    DOMElement* Ferris_DOMElement::getNextElementSibling() const
    {
        LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
        return 0;
    }

    XMLSize_t Ferris_DOMElement::getChildElementCount() const
    {
        LG_DOM_D << __PRETTY_FUNCTION__ << " " << endl;
        return m_context->getSubContextCount();
    }
    
    
    
        // DOM2 //
        
        void
        Ferris_DOMElement::normalize()
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::normalize() c:" << m_context->getURL() << endl;
        }
        bool
        Ferris_DOMElement::isSupported(const XMLCh *feature,
                                       const XMLCh *version) const
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::isSupported() c:" << m_context->getURL() << endl;
            return false;
        }
        const XMLCh*
        Ferris_DOMElement::getNamespaceURI() const
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::getNamespaceURI() c:" << m_context->getURL() << endl;
            return 0;
        }
        const XMLCh*
        Ferris_DOMElement::getPrefix() const
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::getPrefix() c:" << m_context->getURL() << endl;
            return 0;
        }
        const XMLCh*
        Ferris_DOMElement::getLocalName() const 
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " implemented as directory name..." << endl;
            return getTagName();
            
//             LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
//             LG_DOM_D << "element::getLocalName() c:" << m_context->getURL() << endl;
//             return 0;
        }
        void
        Ferris_DOMElement::setPrefix(const XMLCh * prefix)
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::setPrefix() c:" << m_context->getURL() << endl;
        }
        
        bool
        Ferris_DOMElement::hasAttributes() const
        {
            LG_DOM_D << "element::hasAttributes() c:" << m_context->getURL() << endl;
            return true;
        }
        
        // DOM3 // 
        bool
        Ferris_DOMElement::isSameNode(const DOMNode* other) const
        {
            if( const Ferris_DOMElement* other_e = dynamic_cast<const Ferris_DOMElement*>( other ))
            {
                return m_context->getURL() == other_e->m_context->getURL();
            }
            
            LG_DOM_D << __PRETTY_FUNCTION__ << " c:" << m_context->getURL() << endl;
            return false;
        }
        bool
        Ferris_DOMElement::isEqualNode(const DOMNode* arg) const
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::isEqualNode() c:" << m_context->getURL() << endl;
            return false;
        }
        void*
        Ferris_DOMElement::setUserData(const XMLCh* key,
                                       void* data,
                                       DOMUserDataHandler* handler)
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::setUserData() c:" << m_context->getURL() << endl;
            return 0;
        }
        void*
        Ferris_DOMElement::getUserData(const XMLCh* key) const
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::getUserData() c:" << m_context->getURL() << endl;
            return 0;
        }
        const XMLCh*
        Ferris_DOMElement::getBaseURI() const
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::getBaseURI() c:" << m_context->getURL() << endl;
            return 0;
        }
        
        short
        Ferris_DOMElement::compareDocumentPosition(const DOMNode* other) const
        {
            LG_DOM_D << "element::compareDocumentPosition() c:" << m_context->getURL() << endl;
            return myCompareDocumentPosition( this, other );
        }
        const XMLCh*
        Ferris_DOMElement::getTextContent() const
        {
            string v = getStrAttr( m_context, "content", "", true );
            LG_DOM_D << "element::getTextContent() c:" << m_context->getURL()
                     << " v:" << v
                     << endl;
            return cacheString( v );
        }
        void
        Ferris_DOMElement::setTextContent(const XMLCh* textContent)
        {
            string v = tostr( textContent );
            LG_DOM_D << "element::setTextContent() c:" << m_context->getURL() 
                     << " v:" << v
                     << endl;
            setStrAttr( m_context, "content", v );
            changed();
        }
            
        const XMLCh*
        Ferris_DOMElement::lookupPrefix( const XMLCh* namespaceURI ) const
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::lookupPrefix() c:" << m_context->getURL() << endl;
            return 0;
        }
        
        bool
        Ferris_DOMElement::isDefaultNamespace(const XMLCh* namespaceURI) const
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::isDefaultNamespace() c:" << m_context->getURL() << endl;
            return true;
        }
        
        const XMLCh*
        Ferris_DOMElement::lookupNamespaceURI(const XMLCh* prefix) const
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::lookupNamespaceURI() c:" << m_context->getURL() << endl;
            return 0;
        }

        void*
        Ferris_DOMElement::getFeature(const XMLCh* feature, const XMLCh* version) const
        {
            LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
            LG_DOM_D << "element::getFeature() c:" << m_context->getURL() << endl;
            return 0;
        }
        

    // non-standard //
    void
    Ferris_DOMElement::release()
    {
        LG_DOM_D << "element::release() c:" << m_context->getURL() << endl;
    }


    

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_DLLLOCAL Ferris_DOMImplementation : public DOMImplementation
        {
        protected :
            Ferris_DOMImplementation(const Ferris_DOMImplementation &) {};   // no copy construtor.
            Ferris_DOMImplementation & operator = (const Ferris_DOMImplementation &) {return *this;};  // No Assignment


        public:
            Ferris_DOMImplementation() {};
            virtual ~Ferris_DOMImplementation() {};

            virtual bool  hasFeature(const XMLCh *feature,  const XMLCh *version) const
                {
                    return false;
                }

            // DOM2 //
            virtual  DOMDocumentType *createDocumentType(const XMLCh *qualifiedName,
                                                         const XMLCh *publicId,
                                                         const XMLCh *systemId)
                {
                    throw_xml( DOMException::NOT_SUPPORTED_ERR );
                }

            DOMDocument* createDocument(const XMLCh *namespaceURI,
                                        const XMLCh *qualifiedName,
                                        DOMDocumentType *doctype,
                                        MemoryManager* const manager)
                {
                    throw_xml( DOMException::NOT_SUPPORTED_ERR );
                }
            
            virtual void* getFeature(const XMLCh* feature, const XMLCh* version) const 
                {
                    return 0;
                }

            // non standard //
            DOMDocument* createDocument(MemoryManager* const manager)
                {
                    throw_xml( DOMException::NOT_SUPPORTED_ERR );
                }

            static DOMImplementation *getImplementation()
                {
                    static Ferris_DOMImplementation ret;
                    return &ret;
                }
            
            static bool loadDOMExceptionMsg(
                const   DOMException::ExceptionCode  msgToLoad
                ,       XMLCh* const                 toFill
                , const unsigned int                 maxChars
                )
                {
                    return false;
                }
            
            static bool loadDOMExceptionMsg(
                const   DOMRangeException::RangeExceptionCode  msgToLoad
                ,       XMLCh* const                           toFill
                , const unsigned int                           maxChars
                )
                {
                    return false;
                }

            // DOMBuilder* createDOMBuilder( const short mode,
            //                               const XMLCh* const,
            //                               MemoryManager* const  manager,
            //                               XMLGrammarPool* const gramPool)
            //     {
            //         throw_xml( DOMException::NOT_SUPPORTED_ERR );
            //     }

            // DOMWriter* createDOMWriter(MemoryManager* const manager)
            //     {
            //         throw_xml( DOMException::NOT_SUPPORTED_ERR );
            //     }
            
            // virtual DOMInputSource* createDOMInputSource()
            //     {
            //         throw_xml( DOMException::NOT_SUPPORTED_ERR );
            //     }

            virtual DOMLSParser* createLSParser(const DOMImplementationLSMode mode,
                                                const XMLCh* const     schemaType,
                                                MemoryManager* const   manager = XMLPlatformUtils::fgMemoryManager,
                                                XMLGrammarPool*  const gramPool = 0)
            {
                throw_xml( DOMException::NOT_SUPPORTED_ERR );
            }
            virtual DOMLSSerializer* createLSSerializer(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager)
            {
                throw_xml( DOMException::NOT_SUPPORTED_ERR );
            }
            virtual DOMLSInput* createLSInput(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager)
            {
                throw_xml( DOMException::NOT_SUPPORTED_ERR );
            }
            virtual DOMLSOutput* createLSOutput(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager)
            {
                throw_xml( DOMException::NOT_SUPPORTED_ERR );
            }
   
            
            
        };


        /********************************************************************************/

        // DomImpl!!
    class FERRISEXP_DLLLOCAL Ferris_DOMDocument
            :
//            public DOMDocument,
            public DOMDocumentImpl,
            public Handlable
        {
            fh_context   m_root;

            mutable Ferris_DOMElement*  m_rootelement;
            mutable fhx_nodelist        m_cache_getChildNodes;
            bool                        m_hideXMLAttribute;
            bool                        m_hideEmblemAttributes;
            bool                        m_hideSchemaAttributes;
            
            RefVectorOf<DOMRangeImpl>* m_fRanges;
            
        protected:
//            Ferris_DOMDocument(const Ferris_DOMDocument &) {};
//            Ferris_DOMDocument & operator = (const Ferris_DOMDocument &) {return *this;};

        public:
            Ferris_DOMDocument( fh_context c )
                :
                DOMDocumentImpl( (const XMLCh*)0, (const XMLCh*)0,
                                 0, // doctype
                                 0, // domImpl
                                 0  // mem manager
                    ),
                m_root( c ), m_rootelement( 0 ),
                m_cache_getChildNodes( 0 ),
                m_fRanges(0),
                m_hideXMLAttribute( true ),
                m_hideEmblemAttributes( true ),
                m_hideSchemaAttributes( true )
                {
                    LG_DOM_D << "Ferris_DOMDocument()" << endl;
                };
            virtual ~Ferris_DOMDocument()
                {
                    LG_DOM_D << "~Ferris_DOMDocument() this:" << toVoid(this) << endl;
                };

            
            bool hideAsXMLAttribute()
                {
                    return m_hideXMLAttribute;
                }
            void hideAsXMLAttribute( bool v )
                {
                    m_hideXMLAttribute = v;
                }

            bool hideEmblemAttributes()
                {
                    return m_hideEmblemAttributes;
                }
            void hideEmblemAttributes( bool v )
                {
                    m_hideEmblemAttributes = v;
                }
            
            
            bool hideSchemaAttributes()
                {
                    return m_hideSchemaAttributes;
                }
            void hideSchemaAttributes( bool v )
                {
                    m_hideSchemaAttributes = v;
                }
            
            
            virtual DOMElement* createElement(const XMLCh *tagName)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            virtual DOMDocumentFragment* createDocumentFragment()
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            virtual DOMText* createTextNode(const XMLCh *data)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            virtual DOMComment* createComment(const XMLCh *data)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            virtual DOMCDATASection* createCDATASection(const XMLCh *data)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            virtual DOMProcessingInstruction* createProcessingInstruction(
                const XMLCh *target, const XMLCh *data)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }

            virtual DOMAttr* createAttribute(const XMLCh *name)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }

            virtual DOMEntityReference* createEntityReference(const XMLCh *name)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            
            virtual DOMDocumentType* getDoctype() const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            
            virtual DOMImplementation* getImplementation() const
                {
                    return new Ferris_DOMImplementation();
                }
            
            virtual DOMElement* getDocumentElement() const
                {
                    if( !m_rootelement )
                    {
                        m_rootelement = new Ferris_DOMElement( (Ferris_DOMDocument*)this,
                                                               (DOMNode*)this,
                                                               m_root );
                    }
//                     cerr << "getDocumentElement() m_root:" << m_root->getURL()
//                          << " m_rootelement:" << toVoid(m_rootelement) << endl;
                    return m_rootelement;
                }
            
            virtual DOMNodeList* getElementsByTagName(const XMLCh *tagname) const
                {
                    LG_XML_D << "getElementsByTagName() tag:" << tostr(tagname) << endl;
                    
                    ((Ferris_DOMDocument*)this)->changed();
                    return ((Ferris_DOMDocument*)this)->getDeepNodeList( this, tagname );
                }

            
            virtual DOMNode* importNode(DOMNode *importedNode, bool deep)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NOT_SUPPORTED_ERR );
                }
            

            virtual DOMElement* createElementNS(const XMLCh *namespaceURI,
                                                const XMLCh *qualifiedName)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NOT_SUPPORTED_ERR );
                }
            virtual DOMAttr* createAttributeNS(const XMLCh *namespaceURI,
                                               const XMLCh *qualifiedName)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NOT_SUPPORTED_ERR );
                }
            virtual DOMNodeList* getElementsByTagNameNS(const XMLCh *namespaceURI,
                                                        const XMLCh *localName) const
                {
                    LG_XML_D << "getElementsByTagNameNS() ns:" << tostr(namespaceURI)
                             << " local:" << tostr(localName)
                             << endl;

                    ((Ferris_DOMDocument*)this)->changed();
                    return ((Ferris_DOMDocument*)this)->getDeepNodeList(
                        this, namespaceURI, localName );
//                    throw_xml( DOMException::NOT_SUPPORTED_ERR );
                }
            
            virtual DOMElement* getElementById(const XMLCh *elementId) const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }

            // DOM3 //
            virtual const XMLCh* getActualEncoding() const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            virtual void setActualEncoding(const XMLCh* actualEncoding)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                }
            virtual const XMLCh* getEncoding() const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            virtual void setEncoding(const XMLCh* encoding)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                }
            virtual bool getStandalone() const
                {
                    return true;
                }
            virtual void setStandalone(bool standalone)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                }
            virtual const XMLCh* getVersion() const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            virtual void setVersion(const XMLCh* version)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                }
            virtual const XMLCh* getDocumentURI() const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            virtual void setDocumentURI(const XMLCh* documentURI)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                }
            virtual bool getStrictErrorChecking() const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return false;
                }
            virtual void setStrictErrorChecking(bool strictErrorChecking)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                }
            virtual DOMErrorHandler* getErrorHandler() const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            virtual void setErrorHandler(DOMErrorHandler* const handler)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                }
            virtual DOMNode* renameNode(DOMNode* n, const XMLCh* namespaceURI, const XMLCh* name)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NOT_SUPPORTED_ERR );
                }
            virtual DOMNode* adoptNode(DOMNode* source)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NOT_SUPPORTED_ERR );
                }
            virtual void normalizeDocument()
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                }
            virtual bool canSetNormalizationFeature(const XMLCh* const name, bool state) const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return false;
                }
            virtual void setNormalizationFeature(const XMLCh* const name, bool state)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                }
            virtual bool getNormalizationFeature(const XMLCh* const name) const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return false;
                }
            

            // -----------------------------------------------------------------------
            // Non-standard extensions
            // -----------------------------------------------------------------------
            virtual DOMEntity *createEntity(const XMLCh *name) 
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }

            virtual DOMDocumentType *createDocumentType(const XMLCh *name)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            virtual DOMNotation *createNotation(const XMLCh *name) 
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            virtual DOMElement         *createElementNS(const XMLCh *namespaceURI,
                                                        const XMLCh *qualifiedName,
                                                        const XMLSSize_t lineNum,
                                                        const XMLSSize_t columnNum)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }

//             ProxyMemoryManager _memMgr;
//             const DOMXPathNSResolver* createNSResolver( DOMNode* nodeResolver ) {
//                 return _memMgr.createNSResolver(nodeResolver);
//             }
            
            /********************************************************************************/
            /********************************************************************************/
            /********************************************************************************/
            // DOMNode //
            /********************************************************************************/
            /********************************************************************************/
            /********************************************************************************/

            virtual const XMLCh* getNodeName() const
                {
                    const int cachelen = 100;
                    static XMLCh cache[cachelen+1];
                    XMLString::transcode( "document", cache, cachelen );
                    return cache;
                }
            
            virtual const XMLCh* getNodeValue() const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            
            virtual XERCES_CPP_NAMESPACE::DOMNode::NodeType getNodeType() const
                {
//                    cerr << "Ferris_Doc getNodeType()" << endl;
                    return DOCUMENT_NODE;
                }
            
            virtual DOMNode* getParentNode() const
                {
                    return 0;
                }
            virtual DOMNodeList *getChildNodes() const
                {
                    LG_XML_D << "getChildNodes()" << endl;
                    if( !m_cache_getChildNodes )
                    {
                        m_cache_getChildNodes = new Ferris_DOMNodeList();
                        m_cache_getChildNodes->push_back( getDocumentElement() );
                    }
                    return GetImpl( m_cache_getChildNodes );
                }
            virtual DOMNode* getFirstChild() const
                {
                    LG_XML_D << "getFirstChild()" << endl;
//                    cerr << "getFirstChild(begin) this:" << toVoid(this) << endl;
                    DOMNode* ret = getDocumentElement();
//                     cerr << "getFirstChild() ret:" << toVoid(ret) << endl;
//                     cerr << "getFirstChild(end)" << endl;
                    return ret;
                }
            virtual DOMNode* getLastChild() const
                {
                    LG_XML_D << "getLastChild()" << endl;
                    return getDocumentElement();
                }
            virtual DOMNode* getPreviousSibling() const
                {
                    return 0;
                }
            virtual DOMNode* getNextSibling() const 
                {
                    return 0;
                }
            virtual DOMNamedNodeMap *getAttributes() const
                {
                    return 0;
                }
            virtual DOMDocument* getOwnerDocument() const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            
            virtual DOMNode* cloneNode(bool deep) const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            
            virtual DOMNode* insertBefore(DOMNode *newChild,
                                          DOMNode *refChild)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            virtual DOMNode* replaceChild(DOMNode *newChild,
                                          DOMNode *oldChild)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            virtual DOMNode* removeChild(DOMNode *oldChild) 
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            virtual DOMNode* appendChild(DOMNode *newChild)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            virtual bool hasChildNodes() const
                {
                    return true;
                }
            
            virtual void setNodeValue(const XMLCh  *nodeValue)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                }

            // DOM2 //

            virtual void normalize()
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                }
            virtual bool isSupported(const XMLCh *feature,
                                     const XMLCh *version) const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return false;
                }
            virtual const XMLCh* getNamespaceURI() const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            virtual const XMLCh* getPrefix() const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            virtual const XMLCh* getLocalName() const 
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            virtual void setPrefix(const XMLCh * prefix)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                }
            
            virtual bool hasAttributes() const
                {
                    return true;
                }
            
            // DOM3 // 
            virtual bool isSameNode(const DOMNode* other) const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return false;
                }
            virtual bool isEqualNode(const DOMNode* arg) const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return false;
                }
            virtual void* setUserData(const XMLCh* key,
                                      void* data,
                                      DOMUserDataHandler* handler)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            virtual void* getUserData(const XMLCh* key) const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            virtual const XMLCh* getBaseURI() const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            
            virtual short compareDocumentPosition(const DOMNode* other) const
                {
                    return myCompareDocumentPosition( this, other );
                }
            virtual const XMLCh* getTextContent() const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            virtual void setTextContent(const XMLCh* textContent)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    throw_xml( DOMException::NO_MODIFICATION_ALLOWED_ERR );
                }
            
            virtual const XMLCh* lookupPrefix( const XMLCh* namespaceURI ) const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
                    
            virtual bool isDefaultNamespace(const XMLCh* namespaceURI) const
                {
                    return true;
                }
            
            virtual const XMLCh* lookupNamespaceURI(const XMLCh* prefix) const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            
            void* getFeature(const XMLCh* feature, const XMLCh* version) const
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                }
            

            // non-standard //
            virtual void release()
                {
                }

            // DOMDocumentTraversal //
            virtual DOMNodeIterator* createNodeIterator(DOMNode*         root,
                                                        unsigned long    whatToShow,
                                                        DOMNodeFilter*   filter,
                                                        bool             entityReferenceExpansion)
                {
                    LG_DOM_W << __PRETTY_FUNCTION__ << " not implemented!" << endl;
                    return 0;
                    
//                     if (!root) {
//                         throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0);
//                         return 0;
//                     }

//                     DOMNodeIteratorImpl* nodeIterator = new (this) DOMNodeIteratorImpl(this, root, whatToShow, filter, entityReferenceExpansion);

//                     if (fNodeIterators == 0L) {
//                         fNodeIterators = new (this) NodeIterators(1, false);
//                     }
//                     fNodeIterators->addElement(nodeIterator);

//                     return nodeIterator;
                }
            
            virtual DOMTreeWalker* createTreeWalker(DOMNode*          root,
                                                    unsigned long     whatToShow,
                                                    DOMNodeFilter*    filter,
                                                    bool              entityReferenceExpansion)
                {
                    LG_XML_D << "createTreeWalker()" << endl;
                    if (!root) {
                        throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0);
                        return 0;
                    }
                    
                    return new (this) DOMTreeWalkerImpl(root, whatToShow, filter, entityReferenceExpansion);
                }


            // DOMDocumentRange //
            virtual DOMRange* createRange()
                {
                    LG_XML_D << "createRange()" << endl;

                    MemoryManager* const fMemoryManager = XMLPlatformUtils::fgMemoryManager;
                    
                    DOMRangeImpl* range = new (this) DOMRangeImpl(this, fMemoryManager);

                    if (m_fRanges == 0L) {
                        //m_fRanges = new (this) Ranges(1, false);
                        m_fRanges = new (fMemoryManager) Ranges(1, false, fMemoryManager); // XMemory
                    }
                    m_fRanges->addElement(range);
                    return range;
                    
//                     DOMRangeImpl* range = new (this) DOMRangeImpl(this);

//                     if (m_fRanges == 0L) {
//                         MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager;
//                         m_fRanges = new (this) RefVectorOf<DOMRangeImpl>(1, false, manager);
//                     }
//                     m_fRanges->addElement(range);
//                     return range;
                }
        };

        void
        Ferris_DOMElement::changed() const
            {
                m_doc->changed();
            }

        int
        Ferris_DOMElement::changes() const
            {
                return m_doc->changes();
            };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    bool
    Ferris_DOMElement::shouldHideAttribute( const XMLCh* xname ) const
    {
        string n = tostr( xname );
        
        if( m_doc->hideAsXMLAttribute() && ( n == "as-xml" || n == "as-rdf" ) )
            return true;
        
        return false;
    }
    
        const XMLCh*
        Ferris_DOMElement::getAttribute(const XMLCh* xname) const
        {
            LG_DOM_D << "element::getAttribute() c:" << m_context->getURL() << endl;

            if( shouldHideAttribute( xname ) )
                return XMLUni::fgZeroLenString;
            
            DOMNode * attr = ensure_fAttributes()->getNamedItem( xname );
            if (attr)
                return attr->getNodeValue();
            
            return XMLUni::fgZeroLenString;
            
            
//             string n = tostr( xname );
//             if( m_doc->hideAsXMLAttribute() && ( n == "as-xml" || n == "as-rdf" ) )
//                 return 0;
                    
//             string v = getStrAttr( m_context, n, "" );
//             return cacheString( v );
        }
//         DOMAttr*
//         Ferris_DOMElement::getAttributeNode( const std::string& n ) const
//         {
//             LG_DOM_D << "element::getAttributeNode() c:" << m_context->getURL() << endl;
            
//             if( m_doc->hideAsXMLAttribute() && ( n == "as-xml" || n == "as-rdf" ) )
//                 return 0;
//             m_xattrcache_t::iterator ci = m_xattrcache.find( n );
//             if( ci != m_xattrcache.end() )
//             {
//                 return GetImpl(ci->second);
//             }
//             fhx_attr a = new Ferris_DOMAttr( (Ferris_DOMElement*)this, m_context, n );
//             m_xattrcache[ n ] = a;
//             return GetImpl(a);
//         }
        bool
        Ferris_DOMElement::hasAttribute(const XMLCh* xname) const
        {
            LG_DOM_D << "element::hasAttribute() c:" << m_context->getURL() << endl;

            string n = tostr( xname );
            if( m_doc->hideAsXMLAttribute() && ( n == "as-xml" || n == "as-rdf" ) )
                return false;
            return m_context->isAttributeBound( n );
        }
    bool
    Ferris_DOMElement::hasAttributeNS(const XMLCh *namespaceURI,
                                          const XMLCh *localName) const
        {
            LG_DOM_D << "element::hasAttributeNS() c:" << m_context->getURL() << endl;
            return hasAttribute( localName );
        }

    Ferris_DOMNamedNodeMap*
    Ferris_DOMElement::ensure_fAttributes() const
    {
        if( !m_fAttributes )
        {
            m_fAttributes = new (getOwnerDocument()) Ferris_DOMNamedNodeMap( this );

            typedef AttributeCollection::AttributeNames_t ant;
            ant an;
            m_context->getAttributeNames( an );

            LG_DOM_D << "element::fAttributes(setup1) an.sz:" << an.size() << endl;
            
            for( ant::iterator iter = an.begin(); iter != an.end(); ++iter )
            {
                const string eaname = *iter;
                if( m_doc->hideAsXMLAttribute() && ( eaname == "as-xml" || eaname == "as-rdf" ) )
                    continue;
                if( m_doc->hideSchemaAttributes() && starts_with( eaname, "schema:" ) )
                    continue;
                if( m_doc->hideEmblemAttributes() && starts_with( eaname, "emblem:" ) )
                    continue;

                LG_DOM_D << "element::fAttributes(setup2) eaname:" << eaname << endl;

//                DOMAttr* n = getAttributeNode( eaname );
                DOMAttr* n = new Ferris_DOMAttr( (Ferris_DOMElement*)this, m_context, eaname );
                m_fAttributes->setNamedItem( n );
            }
            LG_DOM_D << "element::fAttributes(setup-end) c:" << m_context->getURL()
                     << " sz:" << m_fAttributes->getLength() << endl;
        }
        return m_fAttributes;
    }
    
        DOMNamedNodeMap*
        Ferris_DOMElement::getAttributes() const
        {
            LG_DOM_D << "element::getAttributes() c:" << m_context->getURL() << endl;

            Ferris_DOMNamedNodeMap* ret = ensure_fAttributes();
            return ret;
            
//             m_cache_getAttributes = 0;
//             m_cache_getAttributes = new (getOwnerDocument()) Ferris_DOMNamedNodeMap( this );
//             // FIXME:!
//             m_cache_getAttributes->AddRef();
            
//             typedef AttributeCollection::AttributeNames_t ant;
//             ant an;
//             m_context->getAttributeNames( an );

//             LG_DOM_D << "element::getAttributes() an.sz:" << an.size() << endl;
            
//             for( ant::iterator iter = an.begin(); iter != an.end(); ++iter )
//             {
//                 const string eaname = *iter;
//                 if( m_doc->hideAsXMLAttribute() && ( eaname == "as-xml" || eaname == "as-rdf" ) )
//                     continue;
//                 if( m_doc->hideSchemaAttributes() && starts_with( eaname, "schema:" ) )
//                     continue;
//                 if( m_doc->hideEmblemAttributes() && starts_with( eaname, "emblem:" ) )
//                     continue;

//                 LG_DOM_D << "element::getAttributes() eaname:" << eaname << endl;
                
//                 DOMAttr* n = getAttributeNode( eaname );
//                 m_cache_getAttributes->setNamedItem( n );
//             }
//             LG_DOM_D << "element::getAttributes() c:" << m_context->getURL()
//                      << " sz:" << m_cache_getAttributes->getLength() << endl;
            
//             return GetImpl( m_cache_getAttributes );
        }

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    namespace Factory
    {
        fh_domdoc makeDOM( fh_context c,
                           bool hideXMLAttribute,
                           bool hideEmblemAttributes,
                           bool hideSchemaAttributes )
        {
            LG_XML_D << "makeDOM(1)" << endl;
            ensureXMLPlatformInitialized();

            Ferris_DOMDocument* doc = new Ferris_DOMDocument( c );
            fh_domdoc ret = doc;
            doc->hideAsXMLAttribute( hideXMLAttribute );
            doc->hideEmblemAttributes( hideEmblemAttributes );
            doc->hideSchemaAttributes( hideSchemaAttributes );
            
            return ret;
        }
        
        
        fh_context
        mountDOM( fh_domdoc dom, const std::string& forcedURLPrefix )
        {
            RootContextFactory fac;

//             fh_stringstream ss = tostream( dom );
//             fac.setContextClass( "xml" );
//             fac.AddInfo( RootContextFactory::ROOT, "/" );
//             fac.AddInfo( "StaticString", StreamToString(ss) );
//             fac.AddInfo( "forcedURLPrefix", forcedURLPrefix );
//             fh_context c = fac.resolveContext( RESOLVE_EXACT );

//            fh_stringstream ss = tostream( dom );

            fac.setContextClass( "xml" );
            fac.AddInfo( RootContextFactory::ROOT, "/" );
            {
                stringstream ss;
                ss << (void*)&dom;
//                ss << (void*)GetImpl(dom);
//                cerr << "setting dom.ptr:" << (void*)GetImpl(dom) << " dom.str:" << ss.str() << endl;
                fac.AddInfo( "StaticDOM", ss.str() );
            }
            
            fac.AddInfo( "forcedURLPrefix", forcedURLPrefix );
            fh_context c = fac.resolveContext( RESOLVE_EXACT );

            return c;
        }
        

        fh_context
        mountDOM( fh_domdoc dom )
        {
            return mountDOM( dom, "" );
        }

        class FERRISEXP_DLLLOCAL FerrisErrorHandler
            :
            public ErrorHandler
        {
        public:
            FerrisErrorHandler()
                {}
            virtual ~FerrisErrorHandler()
                {}

            virtual void warning(const SAXParseException& e)
                {
                    LG_XML_W << "\nWarning at (file " << StrX(e.getSystemId())
                             << ", line " << e.getLineNumber()
                             << ", char " << e.getColumnNumber()
                             << "): " << StrX(e.getMessage()) << endl;
                }
            
            virtual void error(const SAXParseException& e)
                {
                    LG_XML_I << "\nError at (file " << StrX(e.getSystemId())
                             << ", line " << e.getLineNumber()
                             << ", char " << e.getColumnNumber()
                             << "): " << StrX(e.getMessage()) << endl;
                }
            

            virtual void fatalError(const SAXParseException& e)
                {
                    LG_XML_ER << "\nFatal Error at (file " << StrX(e.getSystemId())
                              << ", line " << e.getLineNumber()
                              << ", char " << e.getColumnNumber()
                              << "): " << StrX(e.getMessage()) << endl;
                }
            
            virtual void resetErrors()
                {
                }
            
            private :
            FerrisErrorHandler(const FerrisErrorHandler&);
            void operator=(const FerrisErrorHandler&);
        };

        std::string& StreamToString( fh_stringstream& ss, std::string& ret )
        {
            ret = ss.str();
            return ret;
        }

        /**
         * turn an input stream into an XML dom document
         *
         * @throws XMLParse if there were errors parsing the XML at iss
         */
        fh_domdoc
        StringToDOM( const std::string& s )
        {
            ensureXMLPlatformInitialized();

            
            XMLFormatter::UnRepFlags gUnRepFlags = XMLFormatter::UnRep_CharRef;

//             auto_ptr< FerrisErrorHandler > ehan( new FerrisErrorHandler );
//             auto_ptr< XercesDOMParser > parser(  new XercesDOMParser );
            //
            // making these is relatively expensive. just make one and leak it.
            //
            static FerrisErrorHandler* ehan = 0;
            if( !ehan )
                ehan = new FerrisErrorHandler();
            static XercesDOMParser* parser = 0;
            if( !parser )
            {
                parser = new XercesDOMParser();
                parser->setLoadExternalDTD( false );
                parser->setValidationScheme( XercesDOMParser::Val_Never );
//                parser->setDoNamespaces( 0 );
                parser->setDoNamespaces( 1 );
                parser->setDoSchema( 0 );
                parser->setErrorHandler( ehan );

//                parser->setDoValidation( false );
//                parser->setErrorHandler( ehan.get() );
            }
            
            LG_XML_D << "doc:" << s << endl;
            
            int xl = s.length();
            const XMLByte* xb = (const XMLByte *)(s.data());
            MemBufInputSource xmlsrc( xb, xl, "rdn" );
            parser->parse( xmlsrc );

            LG_XML_D << "error count:" << parser->getErrorCount() << endl;
            if( parser->getErrorCount() )
            {
                fh_stringstream ss;
                ss << "Errors encountered creating DOM from stream. count:"
                   <<  parser->getErrorCount()
                   << " input:" << s
                   << endl;
                int errorCount   = parser->getErrorCount();
                fh_domdoc    doc = parser->adoptDocument();
                
                Throw_XMLParse( tostr(ss), errorCount, doc );
            }
            return parser->adoptDocument();
        }
        
        fh_domdoc StreamToDOM( fh_stringstream& iss )
        {
            return StringToDOM( tostr( iss ) );
        }
            
        
        fh_domdoc StreamToDOM( fh_istream iss )
        {
            return StringToDOM( StreamToString( iss ) );
        }


        DOMImplementation* getDefaultDOMImpl()
        {
            ensureXMLPlatformInitialized();
            
            XMLCh tempStr[100];
            XMLString::transcode("LS", tempStr, 99);
            DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation( tempStr );
            return impl;
        }

        fh_domdoc makeDOM( const std::string& rootname )
        {
            DOMImplementation *impl = Factory::getDefaultDOMImpl();
            fh_domdoc doc = 0;
            
            if( !rootname.empty() )
                doc = impl->createDocument( 0, X(rootname.c_str()), 0 );
            else
                doc = impl->createDocument();
            
            return doc;
        }
        
        
    };


//     static DOMString xstr( const std::string& s )
//     {
//         return DOMString( s.c_str() );
//     }


    void setAttribute( DOMElement* e, const std::string& k, const std::string& v )
    {
        e->setAttribute( X(k.c_str()), X(v.c_str()) );
    }

    void ensureAttribute( DOMDocument* dom,
                          DOMElement* e, const std::string& k, const std::string& v )
    {
        if( !e->hasAttribute( X( k.c_str() ) ) )
            dom->createAttribute( X(k.c_str()) );
        
        setAttribute( e, k, v );
    }
    
        

    string getAttribute( const DOMElement* e, XMLCh* kx )
    {
        const XMLCh* vx = e->getAttribute( kx );
        string ret = tostr(vx);
        return ret;
    }
    
    string getAttribute( DOMElement* e, const std::string& k )
    {
        XMLCh* kx = XMLString::transcode( k.c_str() );
        const XMLCh* vx = e->getAttribute( kx );
        XMLString::release( &kx );
        string ret = tostr(vx);
        return ret;
    }
    
    string getAttribute( const DOMElement* e, const std::string& k )
    {
        XMLCh* kx = XMLString::transcode( k.c_str() );
        const XMLCh* vx = e->getAttribute( kx );
        XMLString::release( &kx );
        string ret = tostr(vx);
        return ret;
        
//         XMLCh kx[4000];
//         XMLString::transcode( k.c_str(), kx, 200 );
//         const XMLCh* vx = e->getAttribute( kx );
//         char ctmp[4000];
//         XMLString::transcode( vx, ctmp, 99 );
//         return ctmp;
    }

    void setAttributeNS( DOMElement* e,
                         const std::string& ns,
                         const std::string& k,
                         const std::string& v )
    {
        if( !e )
        {
            stringstream ss;
            ss << "Can not create attribute k:" << k << endl;
            Throw_FerrisCreateAttributeFailed( ss.str(), 0 );
        }
        
        if( ns.empty() )
            e->setAttribute( X(k.c_str()), X(v.c_str()) );
        else
            e->setAttributeNS( X(ns.c_str()), X(k.c_str()), X(v.c_str()) );
    }
    
    std::string getAttributeNS( const DOMElement* e,
                                const std::string& ns,
                                const std::string& k )
    {
        if( ns.empty() )
            return getAttribute( e, k );
        
        XMLCh* nsx = XMLString::transcode( ns.c_str() );
        XMLCh* kx  = XMLString::transcode( k.c_str() );
        const XMLCh* vx = e->getAttributeNS( nsx, kx );
        XMLString::release( &nsx );
        XMLString::release( &kx );
        string ret = tostr(vx);
        return ret;
    }


    FERRISEXP_API std::string getStrSubCtx( const DOMNode* c,
                                            std::string subname,
                                            std::string def,
                                            bool getAllLines,
                                            bool throw_for_errors )
    {
        DOMElement* x = XML::getChildElement( c, subname.c_str() );
        if( !x )
            return def;
        return XML::getChildText( x );
    }
    

        void
        EAUpdateErrorHandler_cerr::handleError( fh_context c,
                                                const std::string& eaname,
                                                const std::string& what )
        {
           cerr << "EAUpdateErrorHandler c:" << c->getURL() << endl;
           cerr << "   eaname:" << eaname << " what:" << what << endl;
        }
    
    namespace XML
    {
        /**
         * Write a user message to the stream oss. Some out-of-band data is
         * attached as a packet header and the program must use readMessage()
         * to get data identical to 'm' back again.
         */
        void writeMessage( fh_ostream oss, const stringmap_t& m )
        {
//            LG_JOURNAL_D << "writeMsg(begin)" << endl;
            Factory::ensureXMLPlatformInitialized();

            DOMImplementation *impl = Factory::getDefaultDOMImpl();
            fh_domdoc    doc = impl->createDocument( 0, X("msg"), 0 );
            DOMElement* root = doc->getDocumentElement();

            string pid = tostr( getpid() );
            setAttribute( root, "pid", tostr( getpid() ) );
            setAttribute( root, "message-version", "1" );
//            LG_JOURNAL_D << "writeMsg() pid:" << pid << endl;

            for( stringmap_t::const_iterator mi = m.begin(); mi != m.end(); ++mi )
            {
                const string& k = mi->first;
                const string& v = mi->second;
            
                DOMElement* e = doc->createElement( X("keyval") );
                root->appendChild( e );
                setAttribute( e, "key",   k );
                setAttribute( e, "value", v );
            }
        
            fh_stringstream ss = tostream( doc );
//            LG_JOURNAL_D << "writeMsg() ss:" << tostr(ss) << endl;

            std::copy( std::istreambuf_iterator<char>(ss),
                       std::istreambuf_iterator<char>(),
                       std::ostreambuf_iterator<char>(oss));
            oss << flush;
        }
    
    
        /**
         * Recreate the map data that was written using writeMessage()
         *
         * takes the map to alter as 'ret' to avoid copying a stringmap on return
         */
        static stringmap_t& readMessage( const std::string& s, stringmap_t& ret )
        {
            Factory::ensureXMLPlatformInitialized();
            static XMLCh* k_pid = XMLString::transcode( "pid" );
            static XMLCh* k_key   = XMLString::transcode( "key" );
            static XMLCh* k_value = XMLString::transcode( "value" );

            fh_domdoc    doc  = Factory::StringToDOM( s );
            DOMElement*  root = doc->getDocumentElement();
//            string       pid  = getAttribute( root, "pid" );
            string       pid  = getAttribute( root, k_pid );

            DOMNodeList* nl = root->getChildNodes();
            for( int i=0; i < nl->getLength(); ++i )
            {
                DOMNode* n = nl->item( i );
                if( n->getNodeType() == DOMNode::ELEMENT_NODE )
                {
                    DOMElement* child = (DOMElement*)n;

//                     string k = getAttribute( child, "key" );
//                     string v = getAttribute( child, "value" );
//                     ret[ k ] = v;
                    ret[ getAttribute( child, k_key ) ] = getAttribute( child, k_value );
                }
            }
            
            return ret;
        }
        
        stringmap_t& readMessage( fh_stringstream& iss, stringmap_t& ret )
        {
            return readMessage( tostr( iss ), ret );
        }
        
        stringmap_t& readMessage( fh_istream iss, stringmap_t& ret )
        {
            return readMessage( StreamToString( iss ), ret );
        }

        /**
         * Write a list of string data to the stream such that
         * the list can be read back using readList()
         */
        void writeList( fh_ostream oss, const stringlist_t& l )
        {
            stringmap_t m;
            for( stringlist_t::const_iterator li = l.begin(); li != l.end(); ++li )
                m[ *li ] = "";
            writeMessage( oss, m );
        }
        
        stringlist_t& readList( fh_istream iss, stringlist_t& ret )
        {
            stringmap_t m;
            readMessage( iss, m );
            copy( map_domain_iterator( m.begin() ),
                  map_domain_iterator( m.end() ),
                  back_inserter( ret ) );
            return ret;
        }
        

        
        /**
         * Create XML describing the contents and EA for this context.
         * optionally include the whole subtree in the output
         *
         * Note that recurse is currently ignored
         */
        std::string contextToXML( fh_context c,
                                  bool recurse,
                                  bool includeContent )
        {
            Factory::ensureXMLPlatformInitialized();

            DOMImplementation *impl = Factory::getDefaultDOMImpl();
            fh_domdoc doc = impl->createDocument( 0, X("context"), 0 );
            
            DOMElement* root = doc->getDocumentElement();
//            doc->appendChild( root );
            setAttribute( root, "taken-from-url", c->getURL() );

            typedef AttributeCollection::AttributeNames_t::iterator I;
            AttributeCollection::AttributeNames_t an;
            c->getAttributeNames( an );
            
            for( I ai = an.begin(); ai != an.end(); ++ai )
            {
                string k = *ai;
                if( !includeContent && k == "content" )
                    continue;
                if( k == "as-xml" )
                    continue;
                if( k == "as-rdf" )
                    continue;
                if( k == "as-json" )
                    continue;

                try
                {
                    string v = getStrAttr( c, k, "", true, true );
                    
                    DOMElement* e = doc->createElement( X("keyval") );
                    root->appendChild( e );
                    setAttribute( e, "key",   k );
//                    setAttribute( e, "value", v );
                    DOMText* payload = doc->createTextNode( X(v.c_str()));
                    e->appendChild( payload );
                }
                catch( exception& e )
                {
                }
            }

//             string content = getStrAttr( c, "content", "", true, true );
//             DOMText* t = doc->createTextNode( X(content.c_str()) );
//             root->appendChild( t );

            fh_stringstream retss = tostream( doc );
            return tostr( retss );
        }

        std::string stringmapToXML( const stringmap_t& sm )
        {
            DOMImplementation *impl = Factory::getDefaultDOMImpl();
            fh_domdoc doc = impl->createDocument( 0, X("result"), 0 );
            DOMElement* root = doc->getDocumentElement();

            for( stringmap_t::const_iterator si = sm.begin(); si != sm.end(); ++si )
            {
                string k = si->first;
                string v = si->second;
                DOMElement* e = doc->createElement( X("keyval") );
                root->appendChild( e );
                setAttribute( e, "key",   k );
                DOMText* payload = doc->createTextNode( X(v.c_str()));
                e->appendChild( payload );
            }
            
            fh_stringstream retss = tostream( doc );
            return tostr( retss );
        }
        

        static std::string contextToXMLProcessContext( fh_context c,
                                                       stringlist_t eatoinclude,
                                                       bool includeContent,
                                                       stringmap_t extraAttrs = stringmap_t() )
        {
            DOMImplementation *impl = Factory::getDefaultDOMImpl();
            fh_domdoc doc = impl->createDocument( 0, X("context"), 0 );
            
            DOMElement* root = doc->getDocumentElement();
//            doc->appendChild( root );
            setAttribute( root, "taken-from-url", c->getURL() );

            for( stringmap_t::iterator si = extraAttrs.begin();
                 si != extraAttrs.end(); ++si )
            {
                setAttribute( root, si->first, si->second );
            }
            
            stringlist_t::iterator ai = eatoinclude.begin();
            for( ; ai != eatoinclude.end(); ++ai )
            {
                string k = *ai;
                if( !includeContent && k == "content" )
                    continue;
                if( k == "as-xml" )
                    continue;
                if( k == "as-rdf" )
                    continue;
                if( k == "as-json" )
                    continue;

                try
                {
                    string v = getStrAttr( c, k, "", true, true );
                    
                    DOMElement* e = doc->createElement( X("keyval") );
                    root->appendChild( e );
                    setAttribute( e, "key",   k );
//                    setAttribute( e, "value", v );
                    DOMText* payload = doc->createTextNode( X(v.c_str()));
                    e->appendChild( payload );
                }
                catch( exception& e )
                {
                }
            }

//             string content = getStrAttr( c, "content", "", true, true );
//             DOMText* t = doc->createTextNode( X(content.c_str()) );
//             root->appendChild( t );

            fh_stringstream retss = tostream( doc );
            return tostr( retss );
        }
        
        /**
         * Create XML describing the contents and EA for this context.
         * optionally include the whole subtree in the output
         *
         * Note that recurse is currently ignored
         */
        std::string contextToXML( fh_context c,
                                  stringlist_t eatoinclude,
                                  int  recurse,
                                  bool includeContent )
        {
            stringstream retss;
            Factory::ensureXMLPlatformInitialized();

            stringmap_t headerAttrs;
            headerAttrs["path"] = c->getURL();
            
            if( recurse == 1 )
            {
                fh_stringstream tmpss;
                tmpss << contextToXMLProcessContext( c, eatoinclude, includeContent, headerAttrs );
                retss << Util::replace_all( tostr(tmpss), "</context>", "" );

                retss << "<path>" << c->getURL() << "</path>" << endl;
                for( Context::iterator ci = c->begin(); ci != c->end(); ++ci )
                {
                    fh_stringstream tmpss;
                    tmpss << contextToXMLProcessContext( *ci, eatoinclude, includeContent );
                    tmpss = trimXMLDeclaration( tmpss, true );
                    retss << tostr( tmpss );
                }
                retss << "</context>";
            }
            else
            {
                retss << contextToXMLProcessContext( c, eatoinclude, includeContent, headerAttrs );
            }
            return retss.str();
        }



        /**
         * Get a child element node of basenode with has a attribute
         * childattr=childvalue
         * return 0 for failure to find such a node.
         */
        static DOMNode* getChildNode( DOMNode* basenode,
                                      const std::string& childattr,
                                      const std::string& childvalue )
        {
            DOMNodeList* nl = basenode->getChildNodes();
            for( int i=0; i < nl->getLength(); ++i )
            {
                DOMNode* n = nl->item( i );
                if( n->getNodeType() == DOMNode::ELEMENT_NODE )
                {
                    DOMElement* child = (DOMElement*)n;
                    string k = getAttribute( child, childattr );
                    if( k == childvalue )
                        return child;
                }
            }
            return 0;
        }

        domnode_list_t  getChildren( DOMElement* element )
        {
            domnode_list_t nl;
            getChildren( nl, element );
            return nl;
        }
        
        
        domnode_list_t& getChildren( domnode_list_t& nl, DOMElement* element )
        {
            for( DOMNode* child = element->getFirstChild();
                 child != 0; child = child->getNextSibling())
            {
                nl.push_back( child );
            }
            return nl;
        }
    
        bool allWhitespaceNodes( DOMElement* element )
        {
            bool ret = true;
        
            for( DOMNode* child = element->getFirstChild(); child != 0; child = child->getNextSibling())
            {
                if( child->getNodeType() == DOMNode::TEXT_NODE )
                {
                    if( !XMLString::isWSCollapsed( child->getNodeValue() ) )
                    {
                        return false;
                    }
                }
            }
            return ret;
        }
        
        string getChildText( DOMElement* n )
        {
            if( !n )
                return "";
            
            DOMNodeList* nl = n->getChildNodes();
            for( int i=0; i < nl->getLength(); ++i )
            {
                DOMNode* n = nl->item( i );
                if( n->getNodeType() == DOMNode::TEXT_NODE )
                {
                    DOMText* x = (DOMText*)n;
                    return tostr( x->getData() );
                }
            }
            return "";
        }

        const std::string& setChildText( fh_domdoc doc, DOMElement* n, const std::string& s )
        {
            removeAllTextChildren( n );
            DOMText* t = doc->createTextNode( X(s.c_str()) );
            n->appendChild( t );
            return s;
        }
        
        const std::string& setChildText( DOMDocument* doc, DOMElement* n, const std::string& s )
        {
            removeAllTextChildren( n );
            DOMText* t = doc->createTextNode( X(s.c_str()) );
            n->appendChild( t );
            return s;
        }
        

        void removeAllTextChildren( DOMElement* element )
        {
            domnode_list_t nl;
            getChildren( nl, element );

            for( domnode_list_t::const_iterator ni = nl.begin(); ni!=nl.end(); ++ni )
            {
                DOMNode* child = *ni;
                LG_XML_D << "found child:" << tostr(child->getNodeName()) << endl;
                if( child->getNodeType() == DOMNode::TEXT_NODE )
                {
                    LG_XML_D << "removing child:" << tostr(child->getNodeName()) << endl;
                    element->removeChild( child );
                }
            }
        }
        
        
        static void applyXMLChange( fh_context c,
                                    DOMElement* root,
                                    fh_EAUpdateErrorHandler eh,
                                    string key,
                                    stringlist_t& eavaluelist )
        {
            for( stringlist_t::iterator si = eavaluelist.begin();
                 si != eavaluelist.end(); ++si )
            {
                if( DOMNode* n = getChildNode( root, key, *si ) )
                {
                    DOMElement* child = (DOMElement*)n;
//                    string v = getAttribute( child, "value" );
                    string v = getChildText( child );
                    
                    try
                    {
//                        cerr << "applyXMLChange() k:" << *si << " v:" << v << endl;
                        setStrAttr( c, *si, v, true, true );
                    }
                    catch( exception& e )
                    {
                        eh->handleError( c, *si, e.what() );
                    }
                }
            }
        }
        
        
        /**
         * given XML data from contextToXML() update the contents and EA
         * for this context to be what is given in s
         *
         * Note that a recursive change set can be given but recursion ignored.
         *
         * @param eh A object that is called to perform error handling.
         *           under some conditions a client may wish to ignore errors
         *           updating some EA, for example ctime updates will probably
         *           fail for filesystems. The default is to ignore all errors
         *           silently
         */
        void updateFromXML( fh_context c, const std::string& xml_data,
                            bool recurse,
                            fh_EAUpdateErrorHandler eh )
        {
            if( !eh )
                eh = new EAUpdateErrorHandler_Null;
            
            Factory::ensureXMLPlatformInitialized();

            fh_stringstream iss;
            iss << xml_data;
            fh_domdoc    doc  = Factory::StreamToDOM( iss );
            DOMElement*  root = doc->getDocumentElement();

            stringlist_t firstAttributes;
            firstAttributes.push_back( "mode" );
            firstAttributes.push_back( "group-owner-number" );
            firstAttributes.push_back( "user-owner-number" );
            firstAttributes.push_back( "content" );
            applyXMLChange( c, root, eh, "key", firstAttributes );

            
            stringlist_t allAttributes;
            DOMNodeList* nl = root->getChildNodes();
            for( int i=0; i < nl->getLength(); ++i )
            {
                DOMNode* n = nl->item( i );
                if( n->getNodeType() == DOMNode::ELEMENT_NODE )
                {
                    DOMElement* child = (DOMElement*)n;
                    string k = getAttribute( child, "key" );
                    allAttributes.push_back( k );
                }
            }
            applyXMLChange( c, root, eh, "key", allAttributes );
            
            
            stringlist_t postAttributes;
            postAttributes.push_back( "content" );
            postAttributes.push_back( "mtime" );
            postAttributes.push_back( "atime" );
            applyXMLChange( c, root, eh, "key", postAttributes );
        }
        
        DOMElement* createElement( fh_domdoc doc,
                                   DOMElement* parent,
                                   const std::string& childname )
        {
            DOMElement* ret = doc->createElement( X(childname.c_str()) );
            parent->appendChild( ret );
            return ret;
        }
        

        DOMElement* getChildElement( const DOMNode* node, const std::string& name )
        {
            DOMNodeList* nl = node->getChildNodes();
            for( int i=0; i < nl->getLength(); ++i )
            {
                DOMNode* n = nl->item( i );
                if( n->getNodeType() == DOMNode::ELEMENT_NODE )
                {
                    DOMElement* child = (DOMElement*)n;
                    if( tostr(child->getNodeName()) == name )
                    {
                        return child;
                    }
                }
            }
            return 0;
        }

        std::string getChildElementText( DOMNode* node, const std::string& name )
        {
            DOMElement* e = getChildElement( node, name );
            return e ? getChildText( e ) : "";
        }
        

        std::list< DOMElement* > getAllChildrenElements( DOMNode* node,
                                                         const std::string& name,
                                                         bool recurse )
        {
            std::list< DOMElement* > ret;
            getAllChildrenElements( node, name, ret, recurse );
            return ret;
        }

        std::list< DOMElement* >& getAllChildrenElements( DOMNode* node,
                                                          const std::string& name,
                                                          std::list< DOMElement* >& ret,
                                                          bool recurse )
        {
            DOMNodeList* nl = node->getChildNodes();
            
            for( int i=0; i < nl->getLength(); ++i )
            {
                DOMNode* n = nl->item( i );
                
                if( n->getNodeType() == DOMNode::ELEMENT_NODE )
                {
                    DOMElement* child = (DOMElement*)n;
                    if( tostr(child->getNodeName()) == name )
                    {
                        ret.push_back( child );
                    }

                    if( recurse )
                    {
                        getAllChildrenElements( child, name, ret, recurse );
                    }
                    
                }
            }
            return ret;
        }

        DOMElement*
        firstChild( DOMNode* node, const std::string& name )
        {
            std::list< DOMElement* > l = XML::getAllChildrenElements( node, name, true );
            if( l.empty() )
                return 0;
            return *(l.begin());
        }
        

    };

        namespace Private
        {
            class FERRISEXP_API FerrisIStreamBinMemInputStream : public BinInputStream
            {
                fh_istream m_iss;
                
            public:
                FerrisIStreamBinMemInputStream( const fh_istream& iss )
                    :
                    m_iss( iss )
                    {
                    }

                virtual XMLFilePos curPos() const
                    {
                        return 0;
                    }
                
                XMLSize_t readBytes( XMLByte* const  toFill, XMLSize_t maxToRead)
                    {
                        m_iss.read( (char*)toFill, maxToRead );
                        unsigned int ret = m_iss.gcount();
//                        cerr << "readBytes() max:" << maxToRead << " got:" << ret << endl;
                        return ret;
                    }
                virtual const XMLCh* getContentType() const
                {
                    return 0;
                }
                
                
            };
            
            class FERRISEXP_API FerrisIStreamInputSource : public InputSource
            {
                fh_istream m_iss;
                
            public:
                FerrisIStreamInputSource(
                    const fh_istream& iss,
                    const char* const     bufId,
                    MemoryManager* const  manager)
                    :
                    InputSource(bufId, manager),
                    m_iss( iss )
                    {
                    }

                BinInputStream* makeStream() const
                    {
//                        return new (getMemoryManager()) FerrisIStreamBinMemInputStream( m_iss );
                        return new FerrisIStreamBinMemInputStream( m_iss );
//                        return 0;
                    }
            };
        };
        
        namespace Factory
        {
            InputSource* makeInputSource( const fh_istream& iss,
                                          const char* const bufId,
                                          MemoryManager* const  manager )
            {
                Private::FerrisIStreamInputSource* ret = new Private::FerrisIStreamInputSource(
                    iss, bufId, manager );
                return ret;
            }
            InputSource* makeInputSource( const fh_context& c )
            {
                string earl = c->getURL();
                fh_istream iss = c->getIStream();
                return makeInputSource( iss, earl.c_str(), XMLPlatformUtils::fgMemoryManager );
            }
        };

        namespace Factory
        {
            XMLCh* makeXMLBase64encoded( const std::string& s )
            {
                XMLSize_t bytes_length = 0;
                XMLByte* bytes = Base64::encode( (const XMLByte* const)s.data(),
                                                 (XMLSize_t)s.size(),
                                                 &bytes_length );
                ArrayJanitor<XMLByte>  bytes_janitor( bytes );

                XMLTransService::Codes resCode;
                XMLTranscoder* myTranscoder =  XMLPlatformUtils::fgTransService->
                    makeNewTranscoderFor( XMLRecognizer::UTF_8, resCode, 16*1024, 
                                          XMLPlatformUtils::fgMemoryManager);
                XMLSize_t charsEaten = 0;

//                 cerr << "s:" << s << ":" << endl;
//                 cerr << "s.len:" << s.size() << " bytes_length:" << bytes_length << endl;
                XMLCh* resultXMLString_Encoded = new XMLCh[ 16*1024 + 4*bytes_length +4 ];
                unsigned char charSizes[16*1024];
                myTranscoder->transcodeFrom( bytes, bytes_length,
                                             resultXMLString_Encoded, 16*1024,
                                             charsEaten, charSizes );

                return resultXMLString_Encoded;
                
            }
        };

};
