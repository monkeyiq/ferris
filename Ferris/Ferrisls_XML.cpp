/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferrisls client helper code.

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

    $Id: Ferrisls_XML.cpp,v 1.4 2010/09/24 21:30:50 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Ferris.hh>
#include <FerrisDOM.hh>
#include <Ferrisls.hh>
#include <FilteredContext.hh>
#include <Context.hh>
#include <iomanip>
#include <ContextSetCompare_private.hh>
#include "Trimming.hh"
#include "ValueRestorer.hh"
#include <FerrisBoost.hh>
#include <FerrisStdHashMap.hh>
#include <xercesc/util/TransService.hpp>
#include <xercesc/framework/XMLRecognizer.hpp>
#include <xercesc/util/Base64.hpp>

using namespace std;

#define X(str) XStr(str).unicodeForm()

namespace Ferris
{
    using namespace XML;



Ferrisls_xmlraw_display::Ferrisls_xmlraw_display()
{
}
    
    Ferrisls_xmlraw_display::~Ferrisls_xmlraw_display()
    {
    }

    
    void
    Ferrisls_xmlraw_display::PrintEA( fh_context ctx,
                                   int i,
                                   const std::string& attr,
                                   const std::string& EA )
    {
        fh_stringstream bss;
        bss << EA;
        std::string ealine;
        getline( bss, ealine );
        PostfixTrimmer trimmer;
        trimmer.push_back( "\r" );
        cout << " " << attr << "=\"" << trimmer( ealine ) << "\" ";
    }
        
    void
    Ferrisls_xmlraw_display::ShowAttributes( fh_context ctx )
    {
        cout << " <context ";
        _Base::ShowAttributes( ctx );
        cout << " />" << endl;
    }

    void
    Ferrisls_xmlraw_display::workStarting()
    {
        cout << "<ferrisls>" << endl;
    }

    void
    Ferrisls_xmlraw_display::workComplete()
    {
        cout << "</ferrisls>" << endl;
    }

    void
    Ferrisls_xmlraw_display::EnteringContext(fh_context ctx)
    {
        cout << "<ferrisls url=\"" << ctx->getURL() << "\" "
             << " name=\"" << ctx->getDirName() << "\" "
             << " >" << endl;
    }

    void
    Ferrisls_xmlraw_display::LeavingContext(fh_context ctx)
    {
        cout << "</ferrisls>" << endl;
    }

    //////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    

    struct Ferrisls_xml_display_private
    {
        fh_domdoc m_dom;
        DOMElement* m_ShowAttributes_element;
        typedef FERRIS_STD_HASH_MAP< string, DOMElement* > m_elementmap_t;
        m_elementmap_t m_elementmap;
        bool m_trimXMLDecl;
        
        Ferrisls_xml_display_private()
            :
            m_dom( 0 ),
            m_ShowAttributes_element( 0 )
            {
                m_dom = Factory::makeDOM( "ferrisls" );
            }
        
        DOMElement* getParentElement( fh_context ctx )
            {
                DOMElement* parent = 0;
                if( ctx->isParentBound() )
                {
                    fh_context pc = ctx->getParent();
                    parent = m_elementmap[ pc->getURL() ];
                }
                if( !parent )
                {
                    parent = m_dom->getDocumentElement();
                }
                return parent;
            }
    };
    
    
Ferrisls_xml_display::Ferrisls_xml_display()
    :
    P( new Ferrisls_xml_display_private() )
{
}
    
    Ferrisls_xml_display::~Ferrisls_xml_display()
    {
        delete P;
    }

    
    void
    Ferrisls_xml_display::PrintEA( fh_context ctx,
                                   int i,
                                   const std::string& attr,
                                   const std::string& EA )
    {
        fh_stringstream bss;
        bss << EA;
        std::string ealine;
        getline( bss, ealine );
        PostfixTrimmer trimmer;
        trimmer.push_back( "\r" );
//        cout << " " << attr << "=\"" << trimmer( ealine ) << "\" ";
        setAttribute( P->m_ShowAttributes_element, attr, trimmer( ealine ) );
    }
        
    void
    Ferrisls_xml_display::ShowAttributes( fh_context ctx )
    {
//        cerr << "ShowAttributes() ctx:" << ctx->getURL() << endl;
        DOMElement* parent = P->getParentElement( ctx );
        
        DOMElement* el = XML::createElement( P->m_dom, parent, "context" );
        Util::ValueRestorer< DOMElement* > _obj( P->m_ShowAttributes_element, el );
        
//        cout << " <context ";
        _Base::ShowAttributes( ctx );
//        cout << " />" << endl;
    }

    void
    Ferrisls_xml_display::workStarting()
    {
//        cout << "<ferrisls>" << endl;
    }

    void
    Ferrisls_xml_display::workComplete()
    {
//        cout << "</ferrisls>" << endl;

        bool gFormatPrettyPrint = true;
        fh_stringstream ss = tostream( P->m_dom, gFormatPrettyPrint );
        ss = trimXMLDeclaration( ss, m_hideXMLDecl );
        cout << ss.str() << flush;
    }

    void
    Ferrisls_xml_display::EnteringContext(fh_context ctx)
    {
//        cerr << "EnteringContext() ctx:" << ctx->getURL() << endl;
        DOMElement* parent = P->getParentElement( ctx );
        DOMElement* el = XML::createElement( P->m_dom,
                                             parent,
                                             "ferrisls" );
        P->m_elementmap[ ctx->getURL() ] = el;
//        cerr << "EnteringContext(set el) ctx:" << ctx->getURL() << endl;
        setAttribute( el, "url", ctx->getURL() );
        setAttribute( el, "name", ctx->getDirName() );
        
        {
            Util::ValueRestorer< DOMElement* > _obj( P->m_ShowAttributes_element, el );
            _Base::ShowAttributes( ctx );
        }
        
//         cout << "<ferrisls url=\"" << ctx->getURL() << "\" "
//              << " name=\"" << ctx->getDirName() << "\" "
//              << " >" << endl;
    }

    void
    Ferrisls_xml_display::LeavingContext(fh_context ctx)
    {
        P->m_elementmap[ ctx->getURL() ] = 0;
//        cout << "</ferrisls>" << endl;
    }


    /////////////////////////////////////////
    /////////////////////////////////////////
    /////////////////////////////////////////



    void
    Ferrisls_xmle_display::PrintEA( fh_context ctx,
                                   int i,
                                   const std::string& attr,
                                   const std::string& EA )
    {
        fh_stringstream bss;


        DOMElement* el = XML::createElement( P->m_dom, P->m_ShowAttributes_element, attr );

//            DOMText* childVal = P->m_dom->createTextNode( X(ealine.c_str()) );
//            DOMCDATASection* childVal = P->m_dom->createCDATASection( X(ealine.c_str()) );

//             XMLTransService::Codes resCode;
//             XMLTranscoder* myTranscoder =  XMLPlatformUtils::fgTransService->
//                 makeNewTranscoderFor( XMLRecognizer::US_ASCII, resCode, 16*1024, 
//                                      XMLPlatformUtils::fgMemoryManager);
//             unsigned int charsEaten;
// //            XMLCh resultXMLString_Encoded[16*1024+4];
//             XMLCh* resultXMLString_Encoded = new XMLCh[16*1024+4];

//             cerr << "attr:" << attr << " sz:" << EA.size() << " data -->:" << EA << ":<--" << endl;
//             unsigned char charSizes[16*1024];
//             myTranscoder->transcodeFrom( (const XMLByte*)EA.c_str(), EA.size(),
//                                          resultXMLString_Encoded, 16*1024,
//                                          charsEaten, charSizes );

            
//             delete myTranscoder;

//             XMLCh* xch = resultXMLString_Encoded;
            
//            XMLCh* xch = XMLString::transcode( EA.c_str() );

        
        int maxChars = EA.size();
        fh_xmlch xch = new XMLCh[ maxChars + 2 ];
        bool encodedOK = XMLString::transcode( EA.c_str(), xch, EA.size() );
        
//        cerr << "attr:" << attr << " have-xch:" << isBound(xch) << endl;
            if( encodedOK )
            {
                DOMText* childVal = P->m_dom->createTextNode( xch );
                el->appendChild( childVal );
            }
            else
            {
//                cerr << "Failed to get plain XML text attribute data for ea:" << EA << endl;

                xch = Factory::makeXMLBase64encoded( EA );

                DOMText* childVal = P->m_dom->createTextNode( xch );
                el->appendChild( childVal );
                setAttribute( el, "encoding", "base64" );
            }
            
            

    }

    Ferrisls_xmle_display::Ferrisls_xmle_display()
        :
        Ferrisls_xml_display()
    {
    }
    
    Ferrisls_xmle_display::~Ferrisls_xmle_display()
    {
    }


    /////////////////////////////////////////
    /////////////////////////////////////////
    /////////////////////////////////////////


    void
    Ferrisls_xml_xsltfs_debug_display::PrintEA( fh_context ctx,
                                   int i,
                                   const std::string& attr,
                                   const std::string& EA )
    {
        Ferrisls_xml_display::PrintEA( ctx, i, attr, EA );
    }

    Ferrisls_xml_xsltfs_debug_display::Ferrisls_xml_xsltfs_debug_display()
        :
        Ferrisls_xml_display()
    {
    }
    
    Ferrisls_xml_xsltfs_debug_display::~Ferrisls_xml_xsltfs_debug_display()
    {
    }

    void
    Ferrisls_xml_xsltfs_debug_display::ShowAttributes( fh_context ctx )
    {
//        cerr << "ShowAttributes() ctx:" << ctx->getURL() << endl;

        DOMElement* el = (DOMElement*)ensureElementCreated( ctx );
        Util::ValueRestorer< DOMElement* > _obj( P->m_ShowAttributes_element, el );
        _Base::ShowAttributes( ctx );
    }

    void*
    Ferrisls_xml_xsltfs_debug_display::ensureElementCreated( fh_context ctx )
    {
        if( !P->m_elementmap[ ctx->getURL() ] )
        {
            DOMElement* parent = P->getParentElement( ctx );
            DOMElement* el = XML::createElement( P->m_dom,
                                                 parent,
                                                 ctx->getDirName() );
            P->m_elementmap[ ctx->getURL() ] = el;
        }
        return P->m_elementmap[ ctx->getURL() ];
    }
    
    
    void
    Ferrisls_xml_xsltfs_debug_display::EnteringContext(fh_context ctx)
    {
        DOMElement* el = (DOMElement*)ensureElementCreated( ctx );
        setAttribute( el, "url", ctx->getURL() );
        setAttribute( el, "name", ctx->getDirName() );
    }

    void
    Ferrisls_xml_xsltfs_debug_display::LeavingContext(fh_context ctx)
    {
    }
    
};

