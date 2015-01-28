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

    $Id: XSLTransform.cpp,v 1.6 2010/09/24 21:31:20 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>

#include <xercesc/util/PlatformUtils.hpp>
#include <XalanTransformer/XalanTransformer.hpp>
#include <XercesParserLiaison/XercesDOMSupport.hpp>
#include <XercesParserLiaison/XercesParserLiaison.hpp>
#include <XalanTransformer/XercesDOMWrapperParsedSource.hpp>
#include <XalanDOM/XalanDOMException.hpp>
#include <PlatformSupport/XSLException.hpp>

#include <Ferris.hh>
#include <ContextPopt.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/FerrisXalan_private.hh>

#include <iostream>
#include <sstream>

using namespace std;
using namespace Ferris;
#define X(str) XStr(str).unicodeForm()

const string PROGRAM_NAME = "XSLTransform";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

// /*******************************************************************************/
// /*******************************************************************************/
// /*******************************************************************************/
// /*** Support code to print a DOM (from DOMPrint.cpp) ***************************/
// /*** From here down to the next three line comment block is apache code ********/
// /*******************************************************************************/
// /*******************************************************************************/
// /*******************************************************************************/
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
// {
//         chOpenAngle, chQuestion, chLatin_x, chLatin_m, chLatin_l
//     ,   chSpace, chLatin_v, chLatin_e, chLatin_r, chLatin_s, chLatin_i
//     ,   chLatin_o, chLatin_n, chEqual, chDoubleQuote, chNull
// };
// static const XMLCh  gXMLDecl2[] =
// {
//         chDoubleQuote, chSpace, chLatin_e, chLatin_n, chLatin_c
//     ,   chLatin_o, chLatin_d, chLatin_i, chLatin_n, chLatin_g, chEqual
//     ,   chDoubleQuote, chNull
// };
// static const XMLCh  gXMLDecl3[] =
// {
//         chDoubleQuote, chSpace, chLatin_s, chLatin_t, chLatin_a
//     ,   chLatin_n, chLatin_d, chLatin_a, chLatin_l, chLatin_o
//     ,   chLatin_n, chLatin_e, chEqual, chDoubleQuote, chNull
// };
// static const XMLCh  gXMLDecl4[] =
// {
//         chDoubleQuote, chQuestion, chCloseAngle
//     ,   chLF, chNull
// };

// static const XMLCh  gStartCDATA[] =
// {
//         chOpenAngle, chBang, chOpenSquare, chLatin_C, chLatin_D,
//         chLatin_A, chLatin_T, chLatin_A, chOpenSquare, chNull
// };

// static const XMLCh  gEndCDATA[] =
// {
//     chCloseSquare, chCloseSquare, chCloseAngle, chNull
// };
// static const XMLCh  gStartComment[] =
// {
//     chOpenAngle, chBang, chDash, chDash, chNull
// };

// static const XMLCh  gEndComment[] =
// {
//     chDash, chDash, chCloseAngle, chNull
// };

// static const XMLCh  gStartDoctype[] =
// {
//     chOpenAngle, chBang, chLatin_D, chLatin_O, chLatin_C, chLatin_T,
//     chLatin_Y, chLatin_P, chLatin_E, chSpace, chNull
// };
// static const XMLCh  gPublic[] =
// {
//     chLatin_P, chLatin_U, chLatin_B, chLatin_L, chLatin_I,
//     chLatin_C, chSpace, chDoubleQuote, chNull
// };
// static const XMLCh  gSystem[] =
// {
//     chLatin_S, chLatin_Y, chLatin_S, chLatin_T, chLatin_E,
//     chLatin_M, chSpace, chDoubleQuote, chNull
// };
// static const XMLCh  gStartEntity[] =
// {
//     chOpenAngle, chBang, chLatin_E, chLatin_N, chLatin_T, chLatin_I,
//     chLatin_T, chLatin_Y, chSpace, chNull
// };
// static const XMLCh  gNotation[] =
// {
//     chLatin_N, chLatin_D, chLatin_A, chLatin_T, chLatin_A,
//     chSpace, chDoubleQuote, chNull
// };

//     static XMLFormatter*            gFormatter             = 0;
// //    static XMLCh*                   gEncodingName          = 0;
//     static XMLFormatter::UnRepFlags gUnRepFlags            = XMLFormatter::UnRep_CharRef;

// // ---------------------------------------------------------------------------
// //  Local classes
// // ---------------------------------------------------------------------------

// class DOMPrintFormatTarget : public XMLFormatTarget
// {
// public:
//     DOMPrintFormatTarget()  {};
//     ~DOMPrintFormatTarget() {};

//     // -----------------------------------------------------------------------
//     //  Implementations of the format target interface
//     // -----------------------------------------------------------------------

//     void writeChars(const   XMLByte* const  toWrite,
//                     const   unsigned int    count,
//                             XMLFormatter * const formatter)
//     {
//         // Surprisingly, Solaris was the only platform on which
//         // required the char* cast to print out the string correctly.
//         // Without the cast, it was printing the pointer value in hex.
//         // Quite annoying, considering every other platform printed
//         // the string with the explicit cast to char* below.
//         cout.write((char *) toWrite, (int) count);
//     };

// private:
//     // -----------------------------------------------------------------------
//     //  Unimplemented methods.
//     // -----------------------------------------------------------------------
//     DOMPrintFormatTarget(const DOMPrintFormatTarget& other);
//     void operator=(const DOMPrintFormatTarget& rhs);
// };

// ostream& operator<<(ostream& target,
//                     const DOMString& toWrite);
// ostream& operator<<(ostream& target, DOM_Node& toWrite);
// XMLFormatter& operator<< (XMLFormatter& strm, const DOMString& s);

// // ---------------------------------------------------------------------------
// //  ostream << DOM_Node
// //
// //  Stream out a DOM node, and, recursively, all of its children. This
// //  function is the heart of writing a DOM tree out as XML source. Give it
// //  a document node and it will do the whole thing.
// // ---------------------------------------------------------------------------
// ostream& operator<<(ostream& target, DOMNode* toWrite)
// {
//     // Get the name and value out for convenience
//     const XMLCh* nodeName  = toWrite->getNodeName();
//     const XMLCh* nodeValue = toWrite->getNodeValue();
//     unsigned long lent = XMLString::stringLen( nodeValue );

//     switch (toWrite->getNodeType())
//     {
//         case DOMNode::TEXT_NODE:
//         {
//             gFormatter->formatBuf( nodeValue, lent, XMLFormatter::CharEscapes);
//             break;
//         }


//         case DOMNode::PROCESSING_INSTRUCTION_NODE :
//         {
//             *gFormatter << XMLFormatter::NoEscapes << gStartPI  << nodeName;
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
//             while( child != 0)
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
//             if (child != 0)
//             {
//                 // There are children. Close start-tag, and output children.
//                 // No escapes are legal here
//                 *gFormatter << XMLFormatter::NoEscapes << chCloseAngle;

//                 while( child != 0 )
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
//             {
//                 DOMNode* child;
// #if 0
//                 for (child = toWrite->getFirstChild(); child; child = child->getNextSibling())
//                 {
//                     target << child;
//                 }
// #else
//                 //
//                 // Instead of printing the refernece tree
//                 // we'd output the actual text as it appeared in the xml file.
//                 // This would be the case when -e option was chosen
//                 //
//                     *gFormatter << XMLFormatter::NoEscapes << chAmpersand
//                                 << nodeName << chSemiColon;
// #endif
//                 break;
//             }


//         case DOMNode::CDATA_SECTION_NODE:
//             {
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
//             DOMDocumentType* doctype = (DOMDocumentType*)toWrite;;

//             *gFormatter << XMLFormatter::NoEscapes  << gStartDoctype
//                         << nodeName;

            
//             if( const XMLCh* id = doctype->getPublicId() )
//             {
//                 *gFormatter << XMLFormatter::NoEscapes << chSpace << gPublic
//                     << id << chDoubleQuote;
//                 if( const XMLCh* id = doctype->getSystemId() )
//                 {
//                     *gFormatter << XMLFormatter::NoEscapes << chSpace
//                        << chDoubleQuote << id << chDoubleQuote;
//                 }
//             }
//             else
//             {
//                 if( const XMLCh* id = doctype->getSystemId() )
//                 {
//                     *gFormatter << XMLFormatter::NoEscapes << chSpace << gSystem
//                         << id << chDoubleQuote;
//                 }
//             }

//             if( const XMLCh* id = doctype->getInternalSubset() )
//                 *gFormatter << XMLFormatter::NoEscapes << chOpenSquare
//                             << id << chCloseSquare;

//             *gFormatter << XMLFormatter::NoEscapes << chCloseAngle;
//             break;
//         }


//         case DOMNode::ENTITY_NODE:
//         {
//             *gFormatter << XMLFormatter::NoEscapes << gStartEntity
//                         << nodeName;

//             DOMEntity* e = (DOMEntity*)toWrite;
            
//             if( const XMLCh* id = e->getPublicId() )
//                 *gFormatter << XMLFormatter::NoEscapes << gPublic
//                             << id << chDoubleQuote;

//             if( const XMLCh* id = e->getSystemId() )
//                 *gFormatter << XMLFormatter::NoEscapes << gSystem
//                             << id << chDoubleQuote;
            
//             if( const XMLCh* id = e->getNotationName() )
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
//             cerr << "Unrecognized node type = "
//                  << (long)toWrite->getNodeType() << endl;
//     }
//     return target;
// }

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

//
// This one I sort of hashed together from a bunch of stuff. I wouldn't want to
// say its xerces-c src related code ;0
//
void dump( DOMDocument* doc )
{
    fh_stringstream ss = tostream( doc, true );
    cout << tostr(ss) << endl;
    
//     DOMPrintFormatTarget* formatTarget = new DOMPrintFormatTarget();

    
// //     if (gEncodingName == 0)
// //     {
// //         DOMString encNameStr("UTF-8");
// //         DOM_Node aNode = doc->getFirstChild();
// //         if (aNode.getNodeType() == DOMNode::XML_DECL_NODE)
// //         {
// //             DOMString aStr = ((DOM_XMLDecl &)aNode).getEncoding();
// //             if (aStr != "")
// //             {
// //                 encNameStr = aStr;
// //             }
// //         }
// //         unsigned int lent = encNameStr.length();
// //         gEncodingName = new XMLCh[lent + 1];
// //         XMLString::copyNString(gEncodingName, encNameStr.rawBuffer(), lent);
// //         gEncodingName[lent] = 0;
// //     }

    
//     gFormatter = new XMLFormatter( X("UTF-8"), formatTarget,
//                                   XMLFormatter::NoEscapes,
//                                    gUnRepFlags);
//     cout << doc;
//     *gFormatter << chLF; // add linefeed in requested output encoding
//     cout << flush;

//     delete gFormatter;
}





/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

int main( int argc, const char** argv )
{
    const char*   XSLFileName = "./query.xsl";
    const char*   DatabaseContextURL = "mysql://localhost/ferristest";
//    const char*   DatabaseContextURL = "mysql://localhost/ferristest/ferrisxsl";
    const char*   DatabaseQueryMetaDataURL = "./query.xml/query";
    unsigned long IntArg = 0;

    /* Parse --logging options, this should be done early */
    ParseOnly_FERRIS_POPT_OPTIONS( PROGRAM_NAME, argc, argv );

    poptContext optCon;
    try
    {
        /* make sure xerces and xalan are ready */
        Factory::ensureXMLPlatformInitialized();

        cerr << "Getting the context:" << DatabaseQueryMetaDataURL << endl;
        fh_context md = Resolve( DatabaseQueryMetaDataURL );

        struct poptOption optionsTable[] = {
            FERRIS_POPT_OPTIONS
            FERRIS_CONTEXTPOPT_OPTIONS( md )            
            POPT_AUTOHELP
            POPT_TABLEEND
        };
        
        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* ...");

        /***/
        int ch=-1;
        while ((ch = poptGetNextOpt(optCon)) >= 0)
        {}
        cerr << "Option processing done." << endl;

        if( !strlen(XSLFileName) )
        {
            poptPrintUsage(optCon, stderr, 0);
            exit(1);
        }

        cerr << "Getting the context:" << DatabaseContextURL << endl;
        fh_context c  = Resolve( DatabaseContextURL );
        
        cerr << "Creating the subcontext for the query..." << endl;
        fh_context queryc = c->createSubContext("", md );
//        fh_context queryc = c;

        cerr << "Creating the DOM..." << endl;
        fh_domdoc theDOM =  Factory::makeDOM( queryc );

        cerr << "Have DOM..." << endl;

//        dump( theDOM );

        cerr << "Dumped DOM..." << endl;
        
//        theDOM->normalize();

        XercesDOMSupport theDOMSupport;
        XercesParserLiaison theParserLiaison(theDOMSupport);
        
        cerr << "Creating the parsed source..." << endl;
        const XercesDOMWrapperParsedSource parsedSource(
            GetImpl(theDOM),
            theParserLiaison,
            theDOMSupport );

        cerr << "Performing the XSLT." << endl;
        XalanTransformer theXalanTransformer;
        int theResult = theXalanTransformer.transform( parsedSource,
                                                       XSLFileName,
                                                       cout );

        cerr << "done." << endl;
        if(theResult != 0)
	    {
		    cerr << "XalanError: \n" << theXalanTransformer.getLastError() << endl;
	    }
        
        return theResult;
    }
    catch( exception& e )
    {
        cerr << "Cought exception:" << e.what() << endl;
    }
    catch( XSLException& e )
    {
        cerr << "Cought XSL exception:" << e.getMessage() << endl;
    }
    catch( XalanDOMException& e )
    {
        cerr << "Cought DOM exception:" << e.getExceptionCode() << endl;
    }
//     catch( ... )
//     {
//         cerr << "Cought ... e" << endl;
//     }
    
    return 0;
}
