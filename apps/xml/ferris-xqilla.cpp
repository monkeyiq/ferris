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

    $Id: ferris-xqilla.cpp,v 1.13 2010/09/24 21:31:20 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/XQilla_private.hh>
#include <xqilla/xqilla-simple.hpp>

#ifdef X
#undef X
#endif
#define X(str) ::Ferris::XStr(str).unicodeForm()

using namespace std;
using namespace Ferris;
using namespace Ferris::qilla;

const string PROGRAM_NAME = "ferris-xqilla";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


string D( const XMLCh* c )
{
    stringstream ret;
    while( *c )
    {
        ret << (char)*c;
        ++c;
    }
    return ret.str();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const char*   BaseURI_CSTR         = 0;
const char*   InputFilesystem_CSTR = "root://";
const char*   ShowColumns_CSTR     = 0;
const char*   ShowColumnsRegex_CSTR= 0;
unsigned long FullTextMode         = 0;
unsigned long XPathMode            = 0;
unsigned long UseNative            = 0;
unsigned long ShowVersion          = 0;
unsigned long prettyPrint          = 0;
unsigned long XUpdate              = 0;
XQilla::Language language = XQilla::XQUERY;
string ShowColumns = "";
string ShowColumnsRegex = "";
stringlist_t ShowColumnsList;

std::string&
readQuery( std::string& q, const std::string& qfile )
{
    if( qfile == "-" )
    {
        fh_istream iss = Factory::fcin();
        q = StreamToString( iss );
    }
    else
    {
        fh_context c = Resolve( qfile );
        fh_istream iss = c->getIStream();
        q = StreamToString( iss );
    }
    return q;
}


DOMLSSerializer* getSerializer( DynamicContext* dynamicContext )
{
    static DOMLSSerializer *m_fSerializer = 0;
    
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
    }

    return m_fSerializer;
}


void
cloneTree( fh_domdoc dom, DOMElement* parent, fh_context ctx )
{
    string rdn = ctx->getDirName();

//     xmlctx->addAttribute( "libferris-element-name",   element_rdn );
//     xmlctx->addAttribute( "libferris-element-number", elementNum );
    string t = getStrAttr( ctx, "libferris-element-name", "" );
    if( !t.empty() )
        rdn = t;

    
    DOMElement* el = XML::createElement( dom, parent,
                                         canonicalElementName(rdn) );
                        
    for( rs<stringlist_t> r( ShowColumnsList ); r; ++r )
    {
        string eaname = *r;
        try
        {
            LG_XQILLA_D << "result:" << ctx->getURL()
                        << " getting eaname:" << eaname
                        << " res:" << getStrAttr( ctx, eaname, "" )
                        << endl;
            setAttribute(
                el, eaname,
                getStrAttr( ctx, eaname, "", true, true ) );
        }
        catch( exception& e )
        {
        }
    }
    Context::iterator  e = ctx->end();
    Context::iterator ci = ctx->begin();
    for( ; ci != e ; ++ci )
    {
        cloneTree( dom, el, *ci );
    }
    
}


int main( int argc, char** argv )
{

    struct poptOption optionsTable[] = {

        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },

        { "pretty-print", 0, POPT_ARG_NONE, &prettyPrint, 0,
          "format the output for easier human reading", 0 },
        
        { "native", 'N', POPT_ARG_NONE, &UseNative, 0,
          "use native XQilla to process -i option. (-i must be an XML file)", 0 },

        { "baseuri", 'b', POPT_ARG_STRING, &BaseURI_CSTR, 0,
          "Set the base URI for the context", "" },

        { "full-text", 'f', POPT_ARG_NONE, &FullTextMode, 0,
          "Parse in XQuery Full-Text mode (default is XQuery mode)", 0 },

        { "xpath", 'p', POPT_ARG_NONE, &XPathMode, 0,
          "Parse in XPath 2 mode (default is XQuery mode)", 0 },
        
        { "input", 'i', POPT_ARG_STRING, &InputFilesystem_CSTR, 0,
          "Load XML document (filesystem) and bind it as the context item", "root://" },

        { "show-ea", 0, POPT_ARG_STRING, &ShowColumns_CSTR, 0,
          "Show these attributes only for the resulting elements", "" },

        { "show-ea-regex", 0, POPT_ARG_STRING, &ShowColumnsRegex_CSTR, 0,
          "also show attributes matching this regex for the resulting elements", "" },

        { "update", 'u', POPT_ARG_NONE, &XUpdate, 0,
          "Parse using W3C Update extensions", 0 },
        
        
        /*
         * Standard Ferris options
         */
        FERRIS_POPT_OPTIONS
        
//         { "show-all-attributes", 0, POPT_ARG_NONE,
//           &ShowAllAttributes, 0,
//           "Show every available bit of metadata about each object in context",
//           0 },
        POPT_AUTOHELP
        POPT_TABLEEND
    };
    poptContext optCon;
    
    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* <XQuery file> or - for query on stdin...");

    if (argc < 1)
    {
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
    }

    if( ShowVersion )
    {
        cout << PROGRAM_NAME << " version: $Id: ferris-xqilla.cpp,v 1.13 2010/09/24 21:31:20 ben Exp $\n"
             << "Written by Ben Martin, aka monkeyiq" << nl
             << nl
             << "Copyright (C) 2001-2008 Ben Martin" << nl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }

    typedef stringlist_t queries_t;
    typedef stringmap_t  variables_t;
    queries_t queries;
    variables_t variables;
    
    while( const char* sCSTR = poptGetArg(optCon) )
    {
        string s = sCSTR;

        if( !starts_with( s, "/" )
            && contains( s, "=" ) )
        {
            string k = s.substr( 0, s.find('=') );
            string v = s.substr( s.find('=')+1 );
            cerr << "k:" << k << endl;
            cerr << "v:" << v << endl;
            variables[ k ] = v;
        }
        else
        {
            queries.push_back( s );
        }
    }

    if( XPathMode )
    {
        if(language == XQilla::XQUERY)
          language = XQilla::XPATH2;
        else if(language == XQilla::XQUERY_FULLTEXT)
          language = XQilla::XPATH2_FULLTEXT;
    }
    if( FullTextMode )
    {
        if(language == XQilla::XQUERY)
            language = XQilla::XQUERY_FULLTEXT;
        else if(language == XQilla::XPATH2)
          language = XQilla::XPATH2_FULLTEXT;
    }

    string BaseURI = "";
    if( BaseURI_CSTR )
    {
        BaseURI = BaseURI_CSTR;
    }
    string InputFilesystem;
    if( InputFilesystem_CSTR )
    {
        InputFilesystem = InputFilesystem_CSTR;
    }
    if( ShowColumns_CSTR )
    {
        ShowColumns = ShowColumns_CSTR;
        Util::parseCommaSeperatedList( ShowColumns, ShowColumnsList );

        if( UseNative )
        {
            cerr << "Native execution and custom attribute selection not implemented..." << endl;
            cerr << "http://sourceforge.net/donate/index.php?group_id=16036" << endl;
        }
    }
    if( ShowColumnsRegex_CSTR )
    {
        ShowColumnsRegex = ShowColumnsRegex_CSTR;
    }
    
    
    
    
    
    try
    {
        XQilla xqilla;

        LG_XQILLA_D << "Starting to process queries..." << endl;
        for( queries_t::iterator qi = queries.begin(); qi != queries.end(); ++qi )
        {
            string qfile = *qi;
            string q;
            LG_XQILLA_D << "Have query file:" << qfile << endl;
            q = readQuery( q, qfile );
            LG_XQILLA_D << "Have query:" << q << endl;

            XQContextImpl* stcontext = 0;
            FerrisXQillaConfiguration conf;

            if( XUpdate )
            {
                LG_XQILLA_D << "using ferris configuration and xupdate..." << endl;
                stcontext = dynamic_cast<XQContextImpl*>(
                    xqilla.createContext(XQilla::XQUERY_UPDATE,  &conf));
            }
            else
            {
                LG_XQILLA_D << "using default configuration..." << endl;
                stcontext = dynamic_cast<XQContextImpl*>(xqilla.createContext());
            }
            
            LG_XQILLA_D << "registering xqilla functions..." << endl;
            LG_XQILLA_D << "create dynamic context..." << endl;
            AutoDelete<DynamicContext> ctxGuard( stcontext->createModuleContext(stcontext->getMemoryManager()));
            DynamicContext* context = ctxGuard.get();
            RegisterFerrisXQillaFunctions( context );

            // Set the external variable values
            for( variables_t::iterator vi = variables.begin(); vi!=variables.end(); ++vi )
            {
                Item::Ptr value = context->getItemFactory()->createString(X(vi->second.c_str()), ctxGuard.get());
                context->setExternalVariable(X(vi->first.c_str()), value);
            }
            
            LG_XQILLA_D << "create query..." << endl;
            AutoDelete<XQQuery> query( xqilla.parse(X(q.c_str()), ctxGuard.adopt(), 0, language ));
    


            if( !BaseURI.empty() )
            {
                context->setBaseURI(X(BaseURI.c_str()));
            }

            LG_XQILLA_D << "Setting up input docs..." << endl;
            
            Sequence seq;
            if( UseNative )
            {
                seq = context->resolveDocument(X( InputFilesystem.c_str() ), 0);
                if(!seq.isEmpty() && seq.first()->isNode())
                {
                    context->setContextItem(seq.first());
                    context->setContextPosition(1);
                    context->setContextSize(1);
                }

//                 ItemFactory* ifactory = context->getItemFactory();
//                 context->setItemFactory(
//                     new FQilla_ItemFactory( ifactory ) );
                
            }
            else
            {
                fh_context m_ctx = Resolve( InputFilesystem );
                LG_XQILLA_D << "have input context:" << m_ctx->getURL() << endl;
                
                ItemFactory* ifactory = context->getItemFactory();
//                context->setItemFactory( new FQilla_ItemFactory( ifactory ) );
                LG_XQILLA_D << "set custom item factory..." << endl;
                
                if( !ShowColumns.empty() || !ShowColumnsRegex.empty() )
                {
                    fh_shouldIncludeAttribute n = new ShouldIncludeAttribute();

                    if( !ShowColumns.empty() )
                        n->setShowColumns( ShowColumns );
                    if( !ShowColumnsRegex.empty() )
                        n->setShowColumnsRegex( ShowColumnsRegex );

                    setShouldIncludeAttribute( context, n );
                }
                LG_XQILLA_D << "have set columns to display..." << endl;
                
                FQilla_Node* qn = new FQilla_Node( context, m_ctx, "", true );
                context->setContextItem( qn );
                context->setContextPosition(1);
                context->setContextSize(1);
            }


            
            LG_XQILLA_D << "about to execute query..." << endl;

            // Execute the query, using the context
            Result result = query->execute(context);

            LG_XQILLA_D << "==================================================" << endl;
            LG_XQILLA_D << "================== HAVE RESULT ===================" << endl;
            LG_XQILLA_D << "==================================================" << endl;
            
        
            // Iterate over the results, printing them
            Item::Ptr item;
            while(item = result->next(context))
            {
                LG_XQILLA_D << "===R==============================================" << endl;

                //
                // This has been moved into Qilla.cpp itself, the output DOM should
                // only contain the wanted attributes.
                //
//                 if( !ShowColumns.empty() )
//                 {
//                     else
//                     {
//                         cerr << "Custom output..." << endl;
//                         const FQilla_Node* fqn = (const FQilla_Node*)item->getInterface(FQilla_Node::fQilla);
//                         fh_context ctx = fqn->getFerrisContext();
//                         fh_domdoc dom = Factory::makeDOM( "libferris" );
//                         DOMElement* parent = dom->getDocumentElement();

                        
//                         cloneTree( dom, parent, ctx );
                        
                        
//                         DOMNode* n = dom->getDocumentElement();
//                         XML::domnode_list_t nl;
//                         XML::getChildren( nl, (DOMElement*)n );
//                         if( nl.size() == 1 )
//                         {
//                             n = nl.front();

//                             fh_stringstream iss = tostream( *n, true );
//                             cout << StreamToString( iss ) << endl;
                            
// //                             DOMWriter* Serializer = getSerializer( context );
// //                             const XMLCh* data = Serializer->writeToString( *n );
// //                             cout << tostr(data) << endl;
//                         }
//                     }
//                 }
//                 else
                {
                    if( prettyPrint )
                    {
                        fh_stringstream result;
                        result << tostr( item->asString(context) ) << endl;
                        fh_domdoc doc = Factory::StreamToDOM( result );
                        bool gFormatPrettyPrint = true;
                        fh_stringstream ss = tostream( doc, gFormatPrettyPrint );
                        cout << StreamToString( ss ) << endl;
                    }
                    else
                    {
                        std::cout << UTF8(item->asString(context)) << std::endl;
                    }
                }
            }
        }
    }

//    try {}
    
    catch( XQException& e )
    {
        cerr << "ERROR tov:" << toVoid( e.getError() ) << endl;
        cerr << "ERROR:" << D(e.getError()) << endl;
        cerr << "ERROR:" << tostr(e.getError()) << endl;
        return 1;
    }
    catch( exception& e )
    {
        cerr << "ERROR:" << e.what() << endl;
        return 1;
    }

    poptFreeContext(optCon);
    return 0;
}


