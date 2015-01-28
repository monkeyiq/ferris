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

    $Id: CGIFerrisTransform.cpp,v 1.4 2010/09/24 21:31:20 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
//QUERY_STRING="hi=there&name=frodo&address=bree"  ./CGIFerrisTransform >|/tmp/new7.html

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>

#include <XalanTransformer/XalanTransformer.hpp>
#include <XercesParserLiaison/XercesDOMSupport.hpp>
#include <XercesParserLiaison/XercesParserLiaison.hpp>
//#include <XercesParserLiaison/XercesDocumentBridge.hpp>
#include <XalanTransformer/XercesDOMWrapperParsedSource.hpp>
#include <XalanDOM/XalanDOMException.hpp>
#include <PlatformSupport/XSLException.hpp>
#include <XercesParserLiaison/FormatterToXercesDOM.hpp>
#include <XSLT/XSLTInputSource.hpp>

#include <Ferris.hh>
#include <ContextPopt.hh>
#include <FerrisDOM.hh>
#include <Trimming.hh>

#include <iostream>
#include <sstream>

#include <cgicc/Cgicc.h>

using namespace std;
using namespace Ferris;
using namespace XALAN_CPP_NAMESPACE;

#define X(str) XStr(str).unicodeForm()

const string PROGRAM_NAME = "XSLTransform";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

/**
 * Convert CGI data into an XML DOM
 */
DOMDocument* CGItoDOM()
{
    DOMImplementation *impl = Factory::getDefaultDOMImpl();
    const XMLCh* namespaceURI = 0;
    DOMDocument* doc = impl->createDocument( namespaceURI,
                                             X("cgidata"),
                                             0 );
    DOMElement* dome = doc->getDocumentElement();
    
    cgicc::Cgicc cgi;

    typedef std::vector<cgicc::FormEntry> e_t;
    const e_t& cgie = cgi.getElements();
    for( e_t::const_iterator iter = cgie.begin(); iter != cgie.end(); ++iter )
    {
        if( getAttribute( dome, iter->getName() ).empty() )
        {
            setAttribute( dome, iter->getName(), iter->getValue() );
        }
        
        DOMElement* childe;
        childe = doc->createElement( X(iter->getName().c_str()) );
        childe->appendChild( doc->createTextNode( X(iter->getValue().c_str()) ));
        dome->appendChild(childe);
    }

    return doc;
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/


class CGIFerrisTransform
{
    string DatabaseContextURL;
    string FerrisXSQLURL;
    string FinalXSLTURL;

    fh_context md;
    fh_context dbc;

    enum Debug_t
    {
        D_NONE              = (1<<0),
        D_GENERAL_VERBOSE   = (1<<1),
        D_CGIDOM            = (1<<2),
        D_TRANSFORMED_FXSQL = (1<<3),
        D_WRAPPED_METADATA  = (1<<4),
        D_QUERYDOM          = (1<<5)
    };
    Debug_t debug;

    void dumpDOM( fh_domdoc dom )
        {
            fh_istream x = tostream( dom );
            std::copy( std::istreambuf_iterator<char>( x ),
                       std::istreambuf_iterator<char>(),
                       std::ostreambuf_iterator<char>(cerr));
        }
    
    void debugCGIDOM( DOMDocument* dom )
        {
            if( debug & D_CGIDOM )
            {
                cerr << "------------------------------------------------------------" << endl;
                cerr << "Have cgiDOM..." << endl;
                dumpDOM( dom );
                cerr << "------------------------------------------------------------" << endl;
            }
        }
    
    void debugTransformed_fxsql( DOMDocument* dom )
        {
            if( debug & D_TRANSFORMED_FXSQL )
            {
                cerr << "------------------------------------------------------------" << endl;
                cerr << "Have transformed FXSQL..." << endl;
                dumpDOM( dom );
                cerr << "------------------------------------------------------------" << endl;
            }
        }
    
    void debugWrappedMetaData( fh_context md )
        {
            if( debug & D_WRAPPED_METADATA )
            {
                cerr << "------------------------------------------------------------" << endl;
                cerr << "------------------------------------------------------------" << endl;
                cerr << " have wrapped dom" << endl;
                fh_context c = md->getSubContext( "queryview" );
                string qstr    = getStrSubCtx( c, "sql", "" );
                string tabname = getStrSubCtx( c, "name", "" );
                    
                cerr << " qstr:" << qstr << " tabname:" << tabname << endl;

                
                typedef ContextCollection::SubContextNames_t X;

                X na = md->getSubContextNames();
                for( X::iterator iter = na.begin(); iter != na.end(); ++iter )
                {
                    cerr << *iter << endl;
                    
                    X na2 = md->getSubContext(*iter)->getSubContextNames();
                    for( X::iterator iter2 = na2.begin(); iter2 != na2.end(); ++iter2 )
                    {
                        cerr << "sub:" << *iter2 << endl;
                    }
                }
                cerr << "------------------------------------------------------------" << endl;
                cerr << "------------------------------------------------------------" << endl;
            }
        }
    
    void debugQueryDOM( fh_domdoc dom )
        {
            if( debug & D_QUERYDOM )
            {
                cerr << "------------------------------------------------------------" << endl;
                cerr << "Have result set DOM..." << endl;
                dumpDOM( dom );
                cerr << "------------------------------------------------------------" << endl;
            }
        }
    

    ostream& DG()
        {
            static fh_stringstream dummy;

            if( debug & D_GENERAL_VERBOSE )
                return cerr;
            return dummy;
        }

public:
    CGIFerrisTransform()
        :
        debug( D_NONE )
        {
            /* make sure xerces and xalan are ready */
            Factory::ensureXMLPlatformInitialized();
        }

    void setDebugGeneral( bool v )
        {
            debug = Debug_t( debug | D_GENERAL_VERBOSE );
        }

    void setDebugCGIDOM( bool v )
        {
            debug = Debug_t( debug | D_CGIDOM );
        }

    void setDebugTransformed_fxsql( bool v )
        {
            debug = Debug_t( debug | D_TRANSFORMED_FXSQL );
        }

    void setDebugWrappedMetaData( bool v )
        {
            debug = Debug_t( debug | D_WRAPPED_METADATA );
        }
    
    void setDebugQueryDOM( bool v )
        {
            debug = Debug_t( debug | D_QUERYDOM );
        }
    
    void setDatabaseContextURL( const string& s )
        {
            DatabaseContextURL = s;
        }
    
    void setFerrisXSQLURL( const string& s )
        {
            FerrisXSQLURL = s;
        }

    void setFinalXSLTURL( const string& s )
        {
            FinalXSLTURL = s;
        }


    void xsltransform( fh_domdoc theDOM,
                       const XSLTInputSource& XSLTURL,
                       const XSLTResultTarget& theOut )
        {
            XercesDOMSupport theDOMSupport;
            XercesParserLiaison theParserLiaison(theDOMSupport);
        
            DG() << "Creating the parsed source..." << endl;
            const XercesDOMWrapperParsedSource parsedSource(
                GetImpl(theDOM), theParserLiaison, theDOMSupport );

            DG() << "Performing the XSLT." << endl;
            XalanTransformer theXalanTransformer;
            int theResult = theXalanTransformer.transform(
                parsedSource,
                XSLTURL,
                theOut );

            DG() << "done. theResult:" << theResult << endl;
            if( theResult )
            {
                cerr << "XalanError: " << theXalanTransformer.getLastError() << endl;
                exit(1);
            }
        }

    DOMDocument* xsltransform( DOMDocument* theDOM, const XSLTInputSource& XSLTURL )
        {
            DOMImplementation*  impl = Factory::getDefaultDOMImpl();
            const XMLCh* namespaceURI = 0;
            DOMDocument* doc = impl->createDocument( namespaceURI,
                                                     X("fakeroot"),
                                                     0 );
            DOMElement* root_element = doc->getDocumentElement();
            
            XercesDOMSupport theDOMSupport;
            XercesParserLiaison theParserLiaison(theDOMSupport);

            FormatterToXercesDOM theFormatter( doc, root_element );
            XSLTResultTarget theDOMResultTarget( theFormatter );

            xsltransform( theDOM, XSLTURL, theDOMResultTarget );

            // adjust so that the fake root element is not in the output
            doc->removeChild( root_element );
            DOMNode* orph = root_element->getFirstChild();
            root_element->removeChild( orph );
            doc->appendChild( orph );
            return doc;
        }

    fh_stringstream get_XSQL_XSLT()
        {
            fh_context c = Resolve( FerrisXSQLURL );
            fh_stringstream ss;

            ss << "<?xml version=\"1.0\"?>\n"
               << "<xsl:stylesheet xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\"\n"
               << " version=\"1.0\">\n"
               << "<xsl:output method=\"xml\" version=\"1.0\" standalone=\"yes\" />\n"
               << "    <xsl:template match=\"/*\"> " << endl;

            fh_istream iss = c->getIStream();
            copy( istreambuf_iterator<char>(iss),
                  istreambuf_iterator<char>(),
                  ostreambuf_iterator<char>(ss));

            ss << "    </xsl:template>\n"
               << "</xsl:stylesheet>" << endl;

            if( debug )
            {
                DG() << "Have FXSQL... for:" << c->getURL() << endl;
                DG() << tostr(ss) << endl;
                DG()  << "------------------------------------------------------------" << endl;
            }
            
            ss.seekp(0);
            ss.seekg(0);
            ss.clear();
            return ss;
        }
    
    void operator()()
        {
            /*
             * convert cgiDOM into a new DOM using FerrisXSQLURL and then bind
             * md to that new dom
             */
            DOMDocument* cgiDOM = CGItoDOM();
            debugCGIDOM( cgiDOM );

            fh_istream xsql_xslt = get_XSQL_XSLT();
            DOMDocument* mdDOM = xsltransform( cgiDOM, &xsql_xslt );
            debugTransformed_fxsql( mdDOM );
            
            md = Factory::mountDOM( mdDOM );
            debugWrappedMetaData( md );

            dbc = Resolve( DatabaseContextURL );
            
            DG() << "Creating the subcontext for the query... dbc:"
                 << dbc->getURL() << endl;
            fh_context queryc = dbc->createSubContext("", md );

            DG() << "Creating the DOM for:" << queryc->getURL() << endl;
            fh_domdoc theDOM =  Factory::makeDOM( queryc );
            debugQueryDOM( theDOM );

            if( !FinalXSLTURL.length() )
            {
                fh_istream x = tostream( theDOM );
                std::copy( std::istreambuf_iterator<char>( x ),
                           std::istreambuf_iterator<char>(),
                           std::ostreambuf_iterator<char>(cout));
                cout << endl;
            }
            else
            {
                xsltransform( theDOM, 
                              FinalXSLTURL.c_str(),
                              cout);
            }
        }
};


int main( int argc, const char** argv )
{
    const char*   DatabaseContextURL = "mysql://localhost/ferristest";
    const char*   FerrisXSQLURL      = "./query.fxsql";
    const char*   FerrisFinalXSLTURL_CSTR = "./query.xsl";
    unsigned long DebugGeneral = 0;
    unsigned long DebugCGIDOM = 0;
    unsigned long DebugFXSQL = 0;
    unsigned long DebugWrappedMetaData = 0;
    unsigned long DebugQueryDOM = 0;
    unsigned long IntArg = 0;

    poptContext optCon;
    try
    {
        struct poptOption optionsTable[] = {

            { "db-context-url", 'd', POPT_ARG_STRING, &DatabaseContextURL, 0,
              "Server and database name to connect to",
              "mysql://localhost/ferristest" },

            { "fxsql", 's', POPT_ARG_STRING, &FerrisXSQLURL, 0,
              "Ferris SQL XML file to process query from",
              "./query.fxsql" },

            { "post-process", 'x', POPT_ARG_STRING, &FerrisFinalXSLTURL_CSTR, 0,
              "XSLT post process to present the query results with prior to display",
              "./query.xsl" },

            { "verbose", 'v', POPT_ARG_INT, &DebugGeneral, 0,
              "more info as to the current step on cerr", 0 },

            { "dump-cgidom", '1', POPT_ARG_INT, &DebugCGIDOM, 0,
              "Show the DOM created from CGI params on cerr", 0 },

            { "dump-fxsql", '2', POPT_ARG_INT, &DebugFXSQL, 0,
              "Show the fxsql after transformations on cerr", 0 },

            { "dump-wrapped-md", '3', POPT_ARG_INT, &DebugWrappedMetaData, 0,
              "Show the metadata for the ferris mount on cerr", 0 },

            { "dump-result-dom", '4', POPT_ARG_INT, &DebugQueryDOM, 0,
              "Show the result set DOM on cerr", 0 },
            
            FERRIS_POPT_OPTIONS
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

        string FerrisFinalXSLTURL = FerrisFinalXSLTURL_CSTR;
        
        if( char* p = getenv( "PATH_TRANSLATED" ) )
        {
            FerrisXSQLURL = p;

            string s;
            PostfixTrimmer trimmer;
            trimmer.push_back( ".fxsql" );
            s = trimmer( FerrisXSQLURL );
            FerrisFinalXSLTURL  = s;
            FerrisFinalXSLTURL += ".xsl";

            try
            {
                fh_context c = Resolve( FerrisFinalXSLTURL );
            }
            catch( exception& e )
            {
                // file does not exist.
                FerrisFinalXSLTURL = "";
            }
            
                
            cout << "Content-Type: text/html\n\n" << endl;
        }
        if( char* p = getenv( "CGI_FERRISTRANSFORM_XSLT" ) )
        {
            FerrisFinalXSLTURL = p;
        }

//         cout << "FerrisXSQLURL:" << FerrisXSQLURL << endl;
//         cout << "DatabaseContextURL:" << DatabaseContextURL << endl;
//         cout << "FerrisFinalXSLTURL:" << FerrisFinalXSLTURL << endl;
//         cout << getenv( "QUERY_STRING" ) << endl;
        
        if( !strlen(FerrisXSQLURL) || !strlen(DatabaseContextURL) )
        {
            cout << "exiting with exxx" << endl;
            poptPrintUsage(optCon, stderr, 0);
            exit(1);
        }

        CGIFerrisTransform app;

        if( DebugGeneral )          app.setDebugGeneral( DebugGeneral );
        if( DebugCGIDOM )           app.setDebugCGIDOM( DebugCGIDOM );
        if( DebugFXSQL )            app.setDebugTransformed_fxsql( DebugFXSQL );
        if( DebugWrappedMetaData )  app.setDebugWrappedMetaData( DebugWrappedMetaData );
        if( DebugQueryDOM )         app.setDebugQueryDOM( DebugQueryDOM );
        

        app.setDatabaseContextURL( DatabaseContextURL );
        app.setFerrisXSQLURL( FerrisXSQLURL );
        app.setFinalXSLTURL( FerrisFinalXSLTURL );

        app();
    }
    catch( XSLException& e )
    {
        cout << "Cought XSL exception:" << e.getMessage() << endl;
    }
    catch( XalanDOMException& e )
    {
        cout << "Cought DOM exception:" << e.getExceptionCode() << endl;
    }
    catch( exception& e )
    {
        cout << "Cought exception:" << e.what() << endl;
    }

    return 0;
}
