/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris
    Copyright (C) 2001 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: FerrisXalanTransform.cpp,v 1.4 2010/09/18 21:30:20 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <xercesc/util/PlatformUtils.hpp>
#include <XalanTransformer/XalanTransformer.hpp>

#include <iostream>
#include <sstream>

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisDOM.hh>
#include "xslt_base.hh"

#include <XercesParserLiaison/XercesDOMSupport.hpp>
#include <XercesParserLiaison/XercesParserLiaison.hpp>
#include <XalanTransformer/XercesDOMWrapperParsedSource.hpp>

#include <xalanc/XercesParserLiaison/FormatterToXercesDOM.hpp>
#include <xalanc/XercesParserLiaison/XercesDOMFormatterWalker.hpp>

using namespace std;
using namespace Ferris;
using namespace FerrisXSLT;

const string PROGRAM_NAME = "FerrisXalanTransform";

int main( int argc, char** argv )
{
    const int MIN_ARG_COUNT = 2;
    int theResult = 0;
    poptContext optCon;

    const char* xslt_file  = "";
    const char* xml_file   = "";
    unsigned long use_ferris_dom = 0;
    unsigned long iopt = 0;

    try
    {
        struct poptOption optionsTable[] = {
            { "xslt", 's', POPT_ARG_STRING, &xslt_file, 0,
              "XSLT file to use", "" },
            { "xml", 'm', POPT_ARG_STRING, &xml_file, 0,
              "XML file to use", "" },

            { "use-ferris-dom", 'D', POPT_ARG_NONE, &use_ferris_dom, 0,
              "Create a virtual DOM for input URL using libferris", "" },
            
            FERRIS_POPT_OPTIONS
            POPT_AUTOHELP
            POPT_TABLEEND
        };

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* xsltparam=value xsltparam2=...");

        if (argc < MIN_ARG_COUNT ) {
            poptPrintUsage(optCon, stderr, 0);
            exit(1);
        }

        Factory::ensureXMLPlatformInitialized();
        XalanTransformer theXalanTransformer;

    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
//         switch (c) {
//         }
    }
        
        const char* Attr;
        for( ; Attr = poptGetArg(optCon); )
        {
            string k;
            string v;

            istringstream ss( Attr );
            getline( ss, k, '=' );
            getline( ss, v );

            cerr << "Setting arg k:" << k << " to v:" << v << endl;
            
            theXalanTransformer.setStylesheetParam(
                XalanDOMString( k.c_str() ),
                XalanDOMString( v.c_str() ));
        }
        
        // Do the transform.
        cerr << "transform XML:" << xml_file << " with xsl:" << xslt_file << endl;

        if( use_ferris_dom )
        {
            fh_context xmlContext = Resolve( xml_file );
            cerr << "have xmlContext:" << xmlContext->getURL() << endl;
            fh_domdoc theDOM = Factory::makeDOM( xmlContext );
            cerr << "have the DOM" << endl;

            XercesParserLiaison theLiaison;
            XercesDOMSupport theDOMSupport(theLiaison);
            XercesParserLiaison theParserLiaison(theDOMSupport);
            cerr << "about to make ParsedSource..." << endl;

            const XercesDOMWrapperParsedSource parsedSource(
                GetImpl(theDOM), theParserLiaison, theDOMSupport );
            cerr << "have parsed source..." << endl;

//            theResult = theXalanTransformer.transform( parsedSource, xslt_file, cout );
//             DOMDocument* outputDOM = DOMImplementation::getImplementation()->createDocument();
//             FormatterToXercesDOM	theFormatter(outputDOM, 0);
            
            fh_domdoc outputDOM = Factory::makeDOM("");
            FormatterToXercesDOM theFormatter( GetImpl(outputDOM), 0);
            theResult = theXalanTransformer.transform( parsedSource, xslt_file, theFormatter );

            fh_stringstream ss = tostream( outputDOM );
            cout << tostr(ss) << endl;
        }
        else
        {
            theResult = theXalanTransformer.transform( xml_file, xslt_file, cout);
        }
        
        if(theResult != 0)
	    {
		    cerr << "XalanError: \n" << theXalanTransformer.getLastError();
	    }
    }
    catch( exception& e )
    {
        cerr << PROGRAM_NAME << ": cought e:" << e.what() << endl;
        exit(1);
    }
        
    poptFreeContext(optCon);
    return theResult;
    
}
