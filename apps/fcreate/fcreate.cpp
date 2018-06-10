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

    $Id: fcreate.cpp,v 1.4 2011/06/18 21:38:27 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <gmodule.h>

#include "config.h"
#include "ferriscreatecommon.hh"

#include <popt.h>
#include <sys/time.h>

#include <Ferris/Runner.hh>
#include <Ferris.hh>
#include <FerrisDOM.hh>

#include <string>
using namespace std;
using namespace Ferris;
using namespace Ferris::Factory;


const string PROGRAM_NAME = "ferriscreate";

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/


int main( int argc, const char** argv )
{
    const char* SchemaToGladeXSLTFileName    = getSchemaToGladeXSLTFileNameDefault();
    const char* SchemaToListXSLTFileNameCSTR = SchemaToListXSLTFileNameDefault();
    const char* GladeToDocumentXSLTFileName  = getGladeToDocumentXSLTFileNameDefault();
    const char* CreateTypeName              = 0;
    const char* CreateRdn                   = 0;
    const char* TargetPath                  = 0;
    unsigned long cix = 1;
    unsigned long AutoRun            = 1;
    unsigned long Sloth              = 0;
    unsigned long ShowVersion        = 0;
    unsigned long DumpFerrisXSD      = 0;
    unsigned long DumpToGladeXML     = 0;
    unsigned long DumpFromGladeXML   = 0;
    unsigned long DumpToFerrisXML    = 0;
    unsigned long DontReadRootContext= 0;
    unsigned long ListCreateTypes    = 0;

    struct poptOption optionsTable[] = {

        { "auto-run", 'f', POPT_ARG_NONE, &AutoRun, 0,
          "Automatically try to create the object without the user hitting ok [always on]", 0 },

        { "sloth", 0, POPT_ARG_NONE, &Sloth, 0,
          "dummy option to allow same command line as gfcreate", 0 },
        
        { "schema-to-glade-xsl", 0, POPT_ARG_STRING, &SchemaToGladeXSLTFileName, 0,
          "dummy option to allow same command line as gfcreate", 0 },

        { "schema-to-list-xsl", 0, POPT_ARG_STRING, &SchemaToListXSLTFileNameCSTR, 0,
          "xslt file to parse DTD schema to a list of types that can be created", 0 },
        
        { "glade-to-document-xsl", 0, POPT_ARG_STRING, &GladeToDocumentXSLTFileName, 0,
          "dummy option to allow same command line as gfcreate", 0 },

        { "create-type", 0, POPT_ARG_STRING, &CreateTypeName, 0,
          "name of object type to create", 0 },

        { "target-path", 0, POPT_ARG_STRING, &TargetPath, 0,
          "specify the target for the new object seperately from k=v pairs", 0 },
        
        { "rdn", 0, POPT_ARG_STRING, &CreateRdn, 0,
          "rdn of object to create", 0 },
        
        { "list-types", 'l', POPT_ARG_NONE, &ListCreateTypes, 0,
          "list the names of object types that can be created", 0 },
        
        { "dump-ferris-xsd", '1', POPT_ARG_NONE, &DumpFerrisXSD, 0,
          "Dump the incomming ferris xsd file " DumpFerrisXSDFileName, 0 },

        { "dump-to-libglade-xml", '2', POPT_ARG_NONE, &DumpToGladeXML, 0,
          "dummy option to allow same command line as gfcreate", 0 },

        { "dump-from-libglade-xml", '3', POPT_ARG_NONE, &DumpFromGladeXML, 0,
          "dummy option to allow same command line as gfcreate", 0 },

        { "dump-to-ferris-xml", '4', POPT_ARG_NONE, &DumpToFerrisXML, 0,
          "dummy option to allow same command line as gfcreate", 0 },

        { "dont-read-root-context", 'x', POPT_ARG_NONE, &DontReadRootContext, 0,
          "Normally if the initial root context is seen to have children it is read to allow proper"
          " ea creation by the overmounting context. This option disables reading the root context",
          0 },
        
        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },
        FERRIS_POPT_OPTIONS
        POPT_AUTOHELP
        POPT_TABLEEND
    };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* RootName1 ...");

    if (argc < 1) {
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }

    /* Now do options processing */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {}

    if( ShowVersion )
    {
        cout << "fcreate version: $Id: fcreate.cpp,v 1.4 2011/06/18 21:38:27 ben Exp $\n"
             << "release      version: " << VERSION << endl
             << "Written by Ben Martin, aka monkeyiq" << endl
             << endl
             << "Copyright (C) 2001 Ben Martin" << endl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }


    
    /*
     * Make sure that this user is setup.
     */
    ensureDotFilesCreated( SchemaToGladeXSLTFileName, GladeToDocumentXSLTFileName );
    ensureXMLPlatformInitialized();

    try
    {
        stringlist_t earls;
        typedef map< string, string > kvs_t;
        kvs_t kvs;
        
        for( const char* t = ""; t ; )
        {
            t = poptGetArg(optCon);
            if( !t )
            {
                break;
            }

            string s = t;
            int pos  = s.find("=");
            if( pos != string::npos )
            {
                kvs[ s.substr( 0, pos ) ] = s.substr( pos+1 );
            }
            else
            {
                earls.push_back( s );
            }
        }

        if( TargetPath )
            earls.push_back( TargetPath );


        for( stringlist_t::iterator sliter = earls.begin(); sliter!=earls.end(); ++sliter )
        {
            string RootName = *sliter;
            fh_context   c  = Resolve( RootName );

            if( ListCreateTypes )
            {
                fh_istream   schemass = c->getCreateSubContextSchema();

                if( !DontReadRootContext && c->getHasSubContextsGuess() )
                {
                    cerr << "Reading url:" << c->getURL() << endl;
                    c->read();
                }
                
                cerr << "listing types that can be created for context: " << c->getURL() << endl;
                
                if( DumpFerrisXSD )
                {
                    schemass.clear();
                    schemass.seekg(0);
                    fh_ofstream xss( DumpFerrisXSDFileName );
                    std::copy( std::istreambuf_iterator<char>(schemass),
                               std::istreambuf_iterator<char>(),
                               std::ostreambuf_iterator<char>(xss));
                    schemass.clear();
                    schemass.seekg(0);
                }
                
                XalanTransformer theXalanTransformer;
                XSLTInputSource  xsltinputss( &schemass );
                XSLTResultTarget xsltoutss( &cout );


                
                fh_context xslc = Resolve( SchemaToListXSLTFileNameCSTR );
                string SchemaToListXSLTFileName = xslc->getDirPath();
                
                int theResult = theXalanTransformer.transform( xsltinputss,
                                                               SchemaToListXSLTFileName.c_str(),
                                                               xsltoutss );
                if(theResult != 0)
                {
                    cerr << "ERROR listing types that can be created: \n"
                         << theXalanTransformer.getLastError() << endl;
                    exit(1);
                }
                exit(0);
            }

            if( !CreateTypeName )
            {
                poptPrintUsage(optCon, stderr, 0);
                exit(1);
            }
            
            /*
             * Make the object that the user has specified.
             */
            fh_mdcontext md = new f_mdcontext();
            fh_mdcontext child = md->setChild( CreateTypeName, "" );
            
            for( kvs_t::iterator ki = kvs.begin(); ki != kvs.end(); ++ki )
            {
                cerr << "Setting k:" << ki->first << " v:" <<  ki->second << endl;
                child->setChild( ki->first, ki->second );
            }
            if( CreateRdn )
                child->setChild( "name", CreateRdn );
            
            fh_context newc   = c->createSubContext( "", md );
            cerr << "Created new context: " << newc->getURL() << endl;
            return 0;
        }
    }
    catch( exception& e )
    {
        cerr << "cought e:" << e.what() << endl;
        exit(1);
    }
    

    poptFreeContext(optCon);
    return 0;
}
