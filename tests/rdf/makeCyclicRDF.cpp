/******************************************************************************
*******************************************************************************
*******************************************************************************

    create some RDF/XML which contains a cycle when mounted

    Copyright (C) 2002 Ben Martin

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

    $Id: makeCyclicRDF.cpp,v 1.4 2009/10/02 21:30:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisRDFCore.hh>

using namespace std;
using namespace Ferris;
using namespace Ferris::RDFCore;

const string PROGRAM_NAME = "makeCyclicRDF";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int main( int argc, const char** argv )
{
    const char*    inputfilename_CSTR  = 0;
    const char*   outputfilepath_CSTR  = 0;
    const char*   name_CSTR            = 0;
    unsigned int  asDB                 = 0;
    
    struct poptOption optionsTable[] =
        {
            { "as-db", 'd', POPT_ARG_NONE, &asDB, 0,
              "create berkeley db files instead of RDF/XML", "" },

            { "output", 'o', POPT_ARG_STRING, &outputfilepath_CSTR, 0,
              "place RDF/DB at given path", "" },

            { "name", 'n', POPT_ARG_STRING, &name_CSTR, 0,
              "use name for prefix of RDF/DB", "" },
            
            FERRIS_POPT_OPTIONS
            POPT_AUTOHELP
            POPT_TABLEEND
        };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]*");

    if (argc < 1)
    {
        poptPrintHelp(optCon, stderr, 0);
        exit(1);
    }
        
    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
    }

    if( asDB && ( !outputfilepath_CSTR || !name_CSTR ))
    {
        poptPrintHelp(optCon, stderr, 0);
        exit(1);
    }
    
    try
    {

        fh_model model = Model::MemoryOnlyModel();
        cerr << "asDB:" << asDB << endl;
        
        if( asDB )
        {
            fh_context md = Shell::acquireContext( (string)outputfilepath_CSTR + "/metadata" );
            model = Model::FromMetadataContext( md );
        }

        fh_node basen = Node::CreateURI( "http://witme.sf.net/rdf/base" );
        model->insert( basen,
                       Node::CreateURI( "http://witme.sf.net/rdf/topleveldir" ),
                       Node::CreateURI( "http://witme.sf.net/rdf/childA" ));
        model->insert( basen,
                       Node::CreateURI( "http://witme.sf.net/rdf/baseEA1" ),
                       Node::CreateLiteral( "frodo" ));
        model->insert( basen,
                       Node::CreateURI( "http://witme.sf.net/rdf/baseEA2" ),
                       Node::CreateLiteral( "gandalf" ));
        model->insert( Node::CreateURI( "http://witme.sf.net/rdf/base" ),
                       Node::CreateURI( "http://witme.sf.net/rdf/topleveldir" ),
                       Node::CreateURI( "http://witme.sf.net/rdf/childB" ));

        model->insert( Node::CreateURI( "http://witme.sf.net/rdf/childA" ),
                       Node::CreateURI( "http://witme.sf.net/rdf/subleveldir" ),
                       Node::CreateURI( "http://witme.sf.net/rdf/childAch1" ));
        model->insert( Node::CreateURI( "http://witme.sf.net/rdf/childA" ),
                       Node::CreateURI( "http://witme.sf.net/rdf/subleveldir" ),
                       Node::CreateURI( "http://witme.sf.net/rdf/childAch2" ));

        model->insert( Node::CreateURI( "http://witme.sf.net/rdf/childAch2" ),
                       Node::CreateURI( "http://witme.sf.net/rdf/subsubleveldir" ),
                       Node::CreateURI( "http://witme.sf.net/rdf/base" ));
        model->insert( Node::CreateURI( "http://witme.sf.net/rdf/childAch2" ),
                       Node::CreateURI( "http://witme.sf.net/rdf/subEA1" ),
                       Node::CreateLiteral( "the grey" ));

        model->sync();
        
        if( !asDB )
        {
            model->write( Factory::fcout() );
            Factory::fcout() << flush;
        }
    }
    catch( exception& e )
    {
        cerr << "Error:" << e.what() << endl;
    }
    
    poptFreeContext(optCon);
    return 0;
}
    
    
