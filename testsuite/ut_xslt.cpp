/******************************************************************************
*******************************************************************************
*******************************************************************************

    unit test code
    Copyright (C) 2003 Ben Martin

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

    $Id: ut_xslt.cpp,v 1.1 2006/12/10 04:52:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/Iterator.hh>
#include <Ferris/FilteredContext.hh>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <XalanTransformer/XalanTransformer.hpp>
#include <XalanTransformer/XercesDOMWrapperParsedSource.hpp>
#include <XalanDOM/XalanDOMException.hpp>
#include <PlatformSupport/XSLException.hpp>
#include <XercesParserLiaison/FormatterToXercesDOM.hpp>
#include <XercesParserLiaison/XercesDOMSupport.hpp>
#include <XercesParserLiaison/XercesParserLiaison.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>

#include <popt.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <map>
#include <list>
#include <iterator>

using namespace std;
using namespace Ferris;
using namespace XERCES_CPP_NAMESPACE;
using namespace XALAN_CPP_NAMESPACE;
#define X(str) XStr(str).unicodeForm()

const string PROGRAM_NAME = "ut_xslt";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

int errors = 0;

fh_ostream& E()
{
    ++errors;
    static fh_ostream ret = Factory::fcerr();
    return ret;
}

void
assertcompare( const std::string& emsg,
               const std::string& expected,
               const std::string& actual )
{
    if( expected != actual )
        E() << emsg << endl
            << " expected:" << expected << ":" 
            << " actual:" << actual << ":" << endl;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
string BaseDir = "/tmp";


string testdoc = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>"
"<testdoc>"
"<childA>hi there</childA>"
"<childB/>"
"<childC><nested1/></childC>"
"</testdoc>";

string testdoc_pritty = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n"
"<testdoc>\n"
"\n"
"  <childA>hi there</childA>\n"
"\n"
"  <childB/>\n"
"\n"
"  <childC>\n"
"    <nested1/>\n"
"  </childC>\n"
"\n"
"</testdoc>\n";


string simplexslt = \
"<?xml version=\"1.0\"?>"
"<xsl:stylesheet xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\" version=\"1.0\">"
"  <xsl:output method=\"xml\" indent=\"yes\"/>"
""
"  <xsl:template match=\"/*\">"
"     <main>out we go!</main>"
"  </xsl:template>"
"</xsl:stylesheet>";


string simplexslt2 = \
"<?xml version=\"1.0\"?>"
"<xsl:stylesheet xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\" version=\"1.0\">"
"  <xsl:output method=\"xml\" indent=\"yes\"/>"
""
"  <xsl:template match=\"/*\">"
"     <main2>out we go!"
""
"     <xsl:for-each select=\".\">"
"       <xsl:sort select=\".\"/>"
"       <xsl:apply-templates/>"
"     </xsl:for-each>"
""
"     </main2>"
"  </xsl:template>"
""
"  <xsl:template match=\"childA\">"
"       <TRChildA>***<xsl:value-of select=\"text()\"/>***</TRChildA>"
"  </xsl:template>"
""
"</xsl:stylesheet>";


void runtest_simple( const std::string& expected, const std::string& xslt )
{
    DOMImplementation*  impl = Factory::getDefaultDOMImpl();

    fh_stringstream testdocss;
    testdocss << testdoc;
    fh_domdoc    indoc  = Factory::StreamToDOM( testdocss );
    DOMElement*  inroot = indoc->getDocumentElement();

    fh_domdoc    outdoc(impl->createDocument( 0, X("fakeroot"), 0 ));
    DOMElement*  outroot = outdoc->getDocumentElement();
    
    XercesParserLiaison theParserLiaison;
    XercesDOMSupport theDOMSupport(theParserLiaison);
    FormatterToXercesDOM theFormatter( GetImpl( outdoc ), outroot );
    XSLTResultTarget theDOMResultTarget( theFormatter );

    const XercesDOMWrapperParsedSource parsedSource(
        GetImpl(indoc), theParserLiaison, theDOMSupport );
    XalanTransformer theXalanTransformer;
    fh_stringstream xsltss( xslt );
    int theResult = theXalanTransformer.transform(
        parsedSource, &xsltss, theDOMResultTarget );
    if( theResult )
    {
        cerr << "XalanError: " << theXalanTransformer.getLastError() << endl;
    }
    
    // adjust so that the fake root element is not in the output
    outdoc->removeChild( outroot );
    DOMNode* orph = outroot->getFirstChild();
    cerr << "orph:" << toVoid( orph ) << endl;
    outroot->removeChild( orph );
    outdoc->appendChild( orph );

    fh_stringstream out = tostream( outdoc, true );
//    cout << "Output:-->" << tostr(out) << "<--" << endl;

    assertcompare( "simple transform failed", expected, tostr(out) );
}

string shallow1 = \
"<?xml version=\"1.0\"?>"
"<xsl:stylesheet xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\" version=\"1.0\">"
"  <xsl:output method=\"xml\" indent=\"yes\"/>"
""
"  <xsl:template match=\"/*\">"
"     <main2>out we go!"
""
"     <xsl:for-each select=\".\">"
"       <xsl:sort select=\".\"/>"
"       <xsl:apply-templates/>"
"     </xsl:for-each>"
""
"     </main2>"
"  </xsl:template>"
""
"  <xsl:template match=\"dj1fileB\">"
"       <dj1fileB>"
"           content=<xsl:value-of select=\"@content\"/>"
"           mtime=<xsl:value-of select=\"@mtime-display\"/>"
"       </dj1fileB>"
"  </xsl:template>"
""
"</xsl:stylesheet>";

void runtest_shallow( fh_context c, const std::string& expected, const std::string& xslt )
{
    DOMImplementation*  impl = Factory::getDefaultDOMImpl();

    fh_domdoc indoc  = Factory::makeDOM( c );

//     // PURE DEBUG //
//     {
//         DOMNode* r  = indoc->getDocumentElement();
//         DOMNode* fc = r->getFirstChild();

//         cerr << "r:" << toVoid(r) << " fc:" << toVoid(fc) << endl;
//         cerr << "r.type:" << r->getNodeType() << " fc.type:" << fc->getNodeType() << endl;
//         cerr << "indoc.type:" << indoc->getNodeType() << endl;
        
//     }
    
//     // PURE DEBUG //
//     {
//         DOMDocument* doc        = indoc;
//         bool gFormatPrettyPrint = true;

        
//         const XMLCh*    gOutputEncoding  = 0;
//         const XMLCh*    gMyEOLSequence   = 0;

//         // get a serializer, an instance of DOMWriter
//         DOMImplementation *impl          = Factory::getDefaultDOMImpl();
//         DOMWriter         *theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();

//         // set user specified end of line sequence and output encoding
//         theSerializer->setNewLine(gMyEOLSequence);
//         theSerializer->setEncoding(gOutputEncoding);
        
// //         // plug in user's own error handler
// //         DOMErrorHandler *myErrorHandler = new FerrisDOMPrintErrorHandler();
// //         theSerializer->setErrorHandler(myErrorHandler);

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
// //        StringStreamFormatTarget* myFormTarget = new StringStreamFormatTarget( ss );
//         MemBufFormatTarget* myFormTarget = new MemBufFormatTarget( 64000 );



//     // PURE DEBUG //
//     {
//         DOMNode* r  = indoc->getDocumentElement();
//         DOMNode* fc = r->getFirstChild();

//         cerr << "r:" << toVoid(r) << " fc:" << toVoid(fc) << endl;
//         cerr << "r.type:" << r->getNodeType() << " fc.type:" << fc->getNodeType() << endl;
//         cerr << "indoc.type:" << indoc->getNodeType() << endl;
        
//     }
        
//         cerr << "BEGIN INLINE FUNCTION CALL===========================================" << endl;

        
//         theSerializer->writeNode(myFormTarget, *doc);


//         cerr << "AFTER INLINE FUNCTION===========================================" << endl;
//     }
    
    
    // PURE DEBUG //
    {
        fh_stringstream s = tostream( indoc );
        cout << "indoc:-->" << tostr(s) << "<--" << endl;
    }
        
    DOMElement*  inroot = indoc->getDocumentElement();

    DOMDocument* outdoc  = impl->createDocument( 0, X("fakeroot"), 0 );
    DOMElement*  outroot = outdoc->getDocumentElement();

    XercesParserLiaison theParserLiaison;
    XercesDOMSupport theDOMSupport(theParserLiaison);
    FormatterToXercesDOM theFormatter( outdoc, outroot );
    XSLTResultTarget theDOMResultTarget( theFormatter );

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    const XercesDOMWrapperParsedSource parsedSource(
        GetImpl(indoc), theParserLiaison, theDOMSupport );
    XalanTransformer theXalanTransformer;
    fh_stringstream xsltss( xslt );
    int theResult = theXalanTransformer.transform(
        parsedSource, &xsltss, theDOMResultTarget );
    if( theResult )
    {
        cerr << "XalanError: " << theXalanTransformer.getLastError() << endl;
    }
    
    // adjust so that the fake root element is not in the output
    outdoc->removeChild( outroot );
    DOMNode* orph = outroot->getFirstChild();
    cerr << "orph:" << toVoid( orph ) << endl;
    outroot->removeChild( orph );
    outdoc->appendChild( orph );

    fh_stringstream out = tostream( outdoc, true );
    cout << "Output:-->" << tostr(out) << "<--" << endl;

    assertcompare( "simple transform failed", expected, tostr(out) );
}


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long Verbose              = 0;
        unsigned long Simple               = 0;
        unsigned long Simple2              = 0;
        unsigned long Shallow1             = 0;
        const char*   BaseDir_CSTR         = "/tmp";

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "basedir", 0, POPT_ARG_STRING, &BaseDir_CSTR, 0,
                  "where the test files are located ", "/tmp" },
                
                { "test-simple", 0, POPT_ARG_NONE, &Simple, 0,
                  "test a simple xml to xml transform", "" },

                { "test-simple2", 0, POPT_ARG_NONE, &Simple2, 0,
                  "test a very mild xml to xml transform", "" },

                { "test-shallow1", 0, POPT_ARG_NONE, &Shallow1, 0,
                  "very mild testing of shallow DOM wrapper in XSLT transform", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]*  ...");

        /* Now do options processing */
        char c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}
        BaseDir = BaseDir_CSTR;

        if( Simple )
            runtest_simple( "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n"
                            "<main>out we go!</main>\n", 
                            simplexslt );
        if( Simple2 )
            runtest_simple( "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n"
                            "<main2>out we go!     \n"
                            "\n"
                            "  <TRChildA>***hi there***</TRChildA>\n"
                            "\n"
                            "</main2>\n",
                            simplexslt2 );

        if( Shallow1 )
            runtest_shallow( Resolve( BaseDir + "/disjoint1" ),
                             "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n"
                             "<main2>out we go!     \n"
                             "\n"
                             "  <dj1fileB>           content=disjoint1           mtime=03 Mar  3 22:46</dj1fileB>\n"
                             "\n"
                             "</main2>\n",
                             shallow1 );
        
        
    }
//     catch( exception& e )
//     {
//         cerr << "cought error:" << e.what() << endl;
//         exit(1);
//     }
    catch( DOMException& e )
    {
        cerr << "cought error: domcode:" << e.code << " domerr: " << e.msg << endl;
        exit(1);
    }
    if( !errors )
        cerr << "Success" << endl;
    return exit_status;
}
