/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of FerrisCreate.

    FerrisCreate is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FerrisCreate is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FerrisCreate.  If not, see <http://www.gnu.org/licenses/>.

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: createcommon.cpp,v 1.4 2011/06/18 21:38:27 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <gmodule.h>

#include <config.h>
#include <Ferris.hh>
#include <Ferris_private.hh>
#include <Ferris/Runner.hh>
#include "ferriscreatecommon.hh"

#include <string>
#include <unistd.h>



extern int xmlLoadExtDtdDefaultValue;

using namespace std;

namespace Ferris
{
const char* getSchemaToGladeXSLTFileNameDefault()
{
    static std::string s = getDotFerrisPath() + "ferriscreate/SchemaToGlade.xsl";
    return s.c_str();
}

const char* SchemaToListXSLTFileNameDefault()
{
    static std::string s = getDotFerrisPath() + "ferriscreate/SchemaToList.xsl";
    return s.c_str();
}

const char* getGladeToDocumentXSLTFileNameDefault()
{
    static std::string s = getDotFerrisPath() + "ferriscreate/GladeToDocuemnt.xsl";
    return s.c_str();
}


    
    FerrisCreate::FerrisCreate()
        :
        SchemaToGladeXSLT(0),
        GladeToDocumentXSLT(0),
        DumpFerrisXSD(false),
        DumpToGladeXML(false),
        DumpFromGladeXML(false),
        DumpToFerrisXML(false)
    {
//         xmlSubstituteEntitiesDefault(1);
//         xmlLoadExtDtdDefaultValue = 1;
    }
    
    FerrisCreate::~FerrisCreate()
    {
        XalanTransformer theXalanTransformer;

        if( SchemaToGladeXSLT )
        {
            theXalanTransformer.destroyStylesheet(SchemaToGladeXSLT);
        }
        
        if( GladeToDocumentXSLT )
        {
            theXalanTransformer.destroyStylesheet( GladeToDocumentXSLT );
        }
    }


    void
    FerrisCreate::setDumpFerrisXSD( bool v )
    {
        DumpFerrisXSD = v;
    }
    
    void
    FerrisCreate::setDumpToGladeXML( bool v )
    {
        DumpToGladeXML = v;
    }
    
    void
    FerrisCreate::setDumpFromGladeXML( bool v )
    {
        DumpFromGladeXML = v;
    }
    
    void
    FerrisCreate::setDumpToFerrisXML( bool v )
    {
        DumpToFerrisXML = v;
    }
    
    

    void
    FerrisCreate::setSchemaToGladeXSLTFileName( const std::string& s )
    {
        fh_context c = Resolve( s );
        SchemaToGladeXSLTFileName = c->getDirPath();
        LG_FERRISCREATE_D << "SchemaToGladeXSLTFileName:" << SchemaToGladeXSLTFileName << endl;


        XalanTransformer theXalanTransformer;
        ifstream inputss( SchemaToGladeXSLTFileName.c_str() );
//        int r =	theXalanTransformer.compileStylesheet( SchemaToGladeXSLTFileName.c_str(),
        int r =	theXalanTransformer.compileStylesheet( &inputss,
                                                       SchemaToGladeXSLT );

        if( r || !SchemaToGladeXSLT )
        {
            cerr << "Failed to compile stylesheet:" << SchemaToGladeXSLTFileName << endl;
            exit(1);
        }
    }
    
    void
    FerrisCreate::setGladeToDocumentXSLTFileName( const std::string& s )
    {
        fh_context c = Resolve( s );
        GladeToDocumentXSLTFileName = c->getDirPath();
        LG_FERRISCREATE_D << "GladeToDocumentXSLT:" << GladeToDocumentXSLT << endl;

        XalanTransformer theXalanTransformer;
        ifstream inputss( GladeToDocumentXSLTFileName.c_str() );
        XSLTInputSource xsltinputss( &inputss );
        int r =	theXalanTransformer.compileStylesheet( xsltinputss, GladeToDocumentXSLT );

        if( r || !GladeToDocumentXSLT )
        {
            cerr << "Failed to compile stylesheet:" << SchemaToGladeXSLTFileName << endl;
            exit(1);
        }
    }
    
    
    void
    FerrisCreate::setSchema( fh_istream ss )
    {
        SchemaString = StreamToString(ss);
        
        if( DumpFerrisXSD )
        {
            ss.clear();
            ss.seekg(0);
            fh_ofstream xss( DumpFerrisXSDFileName );
            std::copy( std::istreambuf_iterator<char>(ss),
                       std::istreambuf_iterator<char>(),
                       std::ostreambuf_iterator<char>(xss));
        }
    }


    fh_istream
    FerrisCreate::getGlade2Document()
    {
        fh_stringstream ret;
//         const char* params[16 + 1];
//         int nbparams = 0;
//        params[nbparams] = 0;

//         xmlDocPtr doc = xmlParseMemory( SchemaString.c_str(), SchemaString.length() );
//         GladeDoc      = xsltApplyStylesheet( SchemaToGladeXSLT, doc, params);

//         MemBufInputSource memiss( SchemaString.c_str(), SchemaString.length(), "", true );
//         DOMParser *parser = new DOMParser;
//         parser->parse(memiss);

//         XercesDOMSupport theDOMSupport;
//         XercesParserLiaison theParserLiaison(theDOMSupport);
//         const XercesDOMWrapperParsedSource parsedSource(
//             theDOM,
//             theParserLiaison,
//             theDOMSupport );

        XalanTransformer theXalanTransformer;
        stringstream inputss;
        inputss << SchemaString;
//        stringstream stylesheetss;
//        stylesheetss << getStrSubCtx( SchemaToGladeXSLTFileName, "" );
//        stylesheetss.seekg(0);
        XSLTInputSource  xsltinputss( &inputss );
//        XSLTInputSource  stylesheet( &stylesheetss );
        XSLTResultTarget xsltoutss( &ret );

        cerr << "About to perform SchemaToGlade XSLT obj:"
             << (void*)SchemaToGladeXSLT
             << " SchemaToGladeXSLTFileName:" << SchemaToGladeXSLTFileName
             << endl;
//        cerr << "SchemaString:" << SchemaString << endl;
        
        
        // {
        //     const char* ofssfn = "/tmp/ofss";
        //     ofstream ofss( ofssfn );
        //     ofss << SchemaString;
        //     ofss.close();
            
        //     // int theResult = theXalanTransformer.transform( ofssfn,
        //     //                                                SchemaToGladeXSLTFileName.c_str(),
        //     //                                                "/tmp/out.xml" );
        // }

        // theXalanTransformer.transform( "/tmp/ofss",
        //                                "/home/ben/.ferriscreate/SchemaToGlade.xsl",
        //                                "/tmp/foo.xml");
        
        int theResult = theXalanTransformer.transform( &inputss,
//                                                       SchemaToGladeXSLT,
                                                       SchemaToGladeXSLTFileName.c_str(),
                                                       &ret );

        if(theResult != 0)
	    {
		    cerr << "getGlade2Document() XalanError: \n" << theXalanTransformer.getLastError() << endl;
            exit(1);
	    }
        

//         xmlOutputBufferPtr xbp = xmlAllocOutputBuffer(0);
//         xsltSaveResultTo( xbp, GladeDoc, SchemaToGladeXSLT );
//         int len = xmlBufferLength( xbp->buffer );
//         const xmlChar* p = xmlBufferContent( xbp->buffer );
//         ret << p;

        if( DumpToGladeXML )
        {
            fh_ofstream oss( DumpToGladeXMLFileName );
            oss << tostr(ret);
        }
        
//         xmlFreeDoc(doc);
        return ret;
    }



    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

#ifdef HAVE_GTK2
    void gcont_fe ( GtkWidget *widget, gpointer udata)
    {
        FerrisCreate* d = (FerrisCreate*)udata;
//        LG_FERRISCREATE_D << "gcont_fe name:" << gtk_widget_get_name (widget) << endl;

        d->writeWidget( widget, true );

        if( GTK_IS_CONTAINER(widget) )
        {
            gtk_container_foreach ( GTK_CONTAINER(widget), gcont_fe, d );
        }

        d->writeWidget( widget, false );
    }

    void FerrisCreate::writeProps( GtkWidget* w )
    {
        fh_stringstream& ss   = createDocumentStream;
        guint gpn             = 0;
        guint i               = 0;
        GObjectClass *oclass  = G_OBJECT_GET_CLASS(w);
        GParamSpec** gp       = g_object_class_list_properties ( oclass, &gpn );
        GParamSpec* p         = 0;



        for( i=0; i < gpn; ++i )
        {
            const char* pname = gp[i]->name;
            GParamSpec* p = g_object_class_find_property ( oclass, pname );
            string pvalue = "unknown";
            GValue gv;
            memset( &gv, 0, sizeof(gv));

            if(!( gp[i]->flags & G_PARAM_READABLE ))
                continue;
            
            g_value_init( &gv, gp[i]->value_type );
            g_object_get_property ( G_OBJECT(w), pname, &gv );
//            LG_FERRISCREATE_D << "Gval:" << G_VALUE_TYPE(&gv) << endl;

            if( G_VALUE_HOLDS_STRING(&gv) && g_value_get_string(&gv) )
            {
                pvalue = g_value_get_string(&gv);
            }
            else if( G_VALUE_HOLDS_INT(&gv) )
            {
                pvalue = tostr(g_value_get_int(&gv));
            }
            else if( G_VALUE_HOLDS_UINT(&gv) )
            {
                pvalue = tostr(g_value_get_uint(&gv));
            }
            else if( G_VALUE_HOLDS_BOOLEAN(&gv) )
            {
                pvalue = tostr(g_value_get_boolean(&gv));
            }

            g_value_unset ( &gv );
//            LG_FERRISCREATE_D << "pval:" << pvalue << endl;
            
            if( pvalue.length() )
            {
                ss << "   <property name=\"" << pname << "\">"
                   << pvalue << "</property>" << endl;
            }
        }
    }
    
    void FerrisCreate::writeWidget( GtkWidget* w, bool beforeChildren )
    {
        fh_stringstream& ss = createDocumentStream;

        string id = gtk_widget_get_name (w);

        if( beforeChildren )
        {
//             if( GTK_IS_ENTRY(w) || GTK_IS_LABEL(w) || GTK_IS_VBOX(w)
//                 || GTK_IS_HBOX(w) || GTK_IS_NOTEBOOK(w) || GTK_IS_WINDOW(w)
//                 )
            {
//                GtkEntry* e = GTK_ENTRY(w);
                
                ss << "<child><widget"
                   << " class=\"" << G_OBJECT_CLASS_NAME( G_OBJECT_GET_CLASS(w) ) << "\" "
                   << " id=\"" << id << "\"> " << endl;
                writeProps( w );
            }
        }
        else
        {
                ss << "</widget></child>" << endl;
        }
    }
    
    fh_istream FerrisCreate::createDocument( GladeXML *xml, GtkWindow* win )
    {
        fh_stringstream ret;
        fh_stringstream ss;

        createDocumentStream.clear();
        createDocumentStream.seekg(0);
        createDocumentStream.seekp(0);
        
        // XXX
        createDocumentStream = ss;

        ss << "<?xml version=\"1.0\" standalone=\"no\"?>" << endl
//           << "<!DOCTYPE glade-interface SYSTEM \"glade-2.0.dtd\">" << endl
           << "<glade-interface>" << endl;
        
        writeWidget( GTK_WIDGET(win), true );
        gtk_container_foreach ( GTK_CONTAINER(win), gcont_fe, this );
        writeWidget( GTK_WIDGET(win), false );

        ss << "</glade-interface>" << endl;

//ss << "xml doc" << endl;
//        return ss;

        /* Translate the glade2 into a proper xml doc */
        const char* params[16 + 1];
        int nbparams = 0;
        params[nbparams] = 0;

        if( DumpFromGladeXML )
        {
            fh_ofstream fss( DumpFromGladeXMLFileName );
            fss << tostr(ss) << endl;
        }

//         xmlDocPtr idoc = xmlParseMemory( tostr(ss).c_str(), tostr(ss).length() );
//         xmlDocPtr odoc = xsltApplyStylesheet( GladeToDocumentXSLT, idoc, params);

/*******************************************************************************/        
        XalanTransformer theXalanTransformer;
        stringstream inputss( tostr(ss) );
        theXalanTransformer.setUseValidation( false );
        int theResult = theXalanTransformer.transform( &inputss,
                                                       GladeToDocumentXSLTFileName.c_str(),
//                                                       GladeToDocumentXSLT,
                                                       &ret );

//         cerr << " GladeToDocumentXSLTFileName:" << GladeToDocumentXSLTFileName
//              << " theResult:" << theResult
//              << endl;
//         cerr << " ret:" << tostr(ret) << endl;
/*******************************************************************************/        

//         XalanTransformer theXalanTransformer;
//         char       *BufferID = NULL;
//         MemBufInputSource *MemBufIS = NULL;

//         DOMParser *Parser = new DOMParser;
// //        Parser->setCreateEntityReferenceNodes(false);
//         Parser->setDoNamespaces(false);
//         Parser->setDoSchema(false);
//         Parser->setDoValidation(false);
// //        Parser->setToCreateXMLDeclTypeNode(true);
// //        Parser->setValidationScheme(false);

//         string str =  tostr(ss);
//         MemBufIS = new MemBufInputSource(
//             (const XMLByte*)str.c_str(),    // Stream pointer
//             str.length() + 1,               // Byte (not character) count
//             BufferID,                       // Buffer ID (becomes System ID)
//             false);                         // Copy (not adopt) caller's buffer
        
        
//         if( !MemBufIS )
//         {
//             // Handle errors here
// 		    cerr << "FerrisCreate::createDocument(!MemBufIS) XalanError: \n" << theXalanTransformer.getLastError() << endl;
//             exit(1);
//         }
        
//         Parser->parse(*MemBufIS);
//         DOM_Document theDOM = Parser->getDocument();

//         XercesDOMSupport theDOMSupport;
//         XercesParserLiaison theParserLiaison(theDOMSupport);
        
//         const XercesDOMWrapperParsedSource parsedSource(
//             theDOM,
//             theParserLiaison,
//             theDOMSupport );

        
//         int theResult = theXalanTransformer.transform( parsedSource,
//                                                        GladeToDocumentXSLTFileName.c_str(),
//                                                        &ret );
/*******************************************************************************/        
        
        if(theResult != 0)
	    {
		    cerr << "FerrisCreate::createDocument() XalanError: \n" << theXalanTransformer.getLastError() << endl;
            exit(1);
	    }
        

//         xmlOutputBufferPtr xbp = xmlAllocOutputBuffer(0);
//         xsltSaveResultTo( xbp, odoc, GladeToDocumentXSLT );
//         int len = xmlBufferLength( xbp->buffer );
//         const xmlChar* p = xmlBufferContent( xbp->buffer );
//         ret << p;
        if( DumpToFerrisXML )
        {
//            cerr << "dumping to DumpToFerrisXMLFileName:" << DumpToFerrisXMLFileName << endl;
            fh_ofstream oss( DumpToFerrisXMLFileName );
            oss << tostr(ret); // << flush;
//            oss.rdbuf()->pubsync();
        }
        
//         FILE* fp      = fopen( tfilename, "w+" );
//         xsltSaveResultToFile(fp, odoc, GladeToDocumentXSLT);
//         fclose(fp);

        
//         xmlFreeDoc(idoc);
//         xmlFreeDoc(odoc);

//         fh_context c = Resolve( tfilename );
//         return c->getIStream();

        return ret;
    }
#endif // gtk2
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    /*
     * Make sure that this user is setup.
     */
    void ensureDotFilesCreated( const std::string& SchemaToGladeXSLTFileName,
                                const std::string& GladeToDocumentXSLTFileName )
    {
        if( starts_with( SchemaToGladeXSLTFileName, "~/.ferriscreate/" )
            || starts_with( GladeToDocumentXSLTFileName, "~/.ferriscreate/" ) )
        {
            try
            {
                fh_context c = Resolve( "~/.ferriscreate/" );
            }
            catch( exception& e )
            {
                cerr << "Error resolving ~/.ferriscreate/ e:" << e.what() << endl
                     << "...attempting to setup user." << endl;
                
                string setupcommand = "ferriscreate-setup-user.sh";
            
                try
                {
                    fh_runner r = new Runner();
                    r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags()
                                                   | G_SPAWN_STDOUT_TO_DEV_NULL
                                                   | G_SPAWN_STDERR_TO_DEV_NULL
                                                   | G_SPAWN_SEARCH_PATH) );
                    r->setCommandLine( setupcommand );
                    r->Run();
                    fh_context c = Resolve( "~/.ferriscreate/" );
                }
                catch( exception& e )
                {
                    cerr << "User is not setup. Running:" << setupcommand << " failed."
                         << " This means that files in /etc/skel are not properly configured."
                         << " please install ferriscreate again." << endl;
                    exit(1);
                }
            }
        }
    }
    

    
};



