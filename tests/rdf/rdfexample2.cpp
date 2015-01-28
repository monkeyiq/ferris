/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris istream forward sweep test
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

    $Id: rdfexample2.cpp,v 1.3 2009/10/01 21:30:10 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisRDFCore.hh>

using namespace std;
using namespace Ferris;
using namespace Ferris::RDFCore;

const string PROGRAM_NAME = "rdfexample2";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

static const char *rdfxml_content="<?xml version=\"1.0\"?>\
<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\
     xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\
  <rdf:Description rdf:about=\"http://purl.org/net/dajobe/\">\
    <dc:title>Dave Beckett's Home Page</dc:title>\
    <dc:creator>Dave Beckett</dc:creator>\
    <dc:description>The generic home page of Dave Beckett.</dc:description>\
  </rdf:Description> \
</rdf:RDF>\
";

int main( int argc, const char** argv )
{
    const char*   X_CSTR  = 0;
    unsigned int  Y       = 0;
    
    struct poptOption optionsTable[] =
        {
//             { "explicit-size", 'n', POPT_ARG_INT, &Y, 0,
//               "size of input context", "" },
            
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

    try
    {
        fh_model   model   = Model::MemoryOnlyModel();
        fh_uri     uri     = new URI( "http://example.librdf.org/" );
        fh_parser  parser  = new Parser();
        parser->ParseIntoModel( model, rdfxml_content, uri );
        parser = 0;
        uri    = 0;
        
        fh_statement st = new Statement();
        st->setSubject(   Node::CreateURI("http://example.org/subject") );
        st->setPredicate( Node::CreateURI("http://example.org/pred1") );
        st->setObject(    Node::CreateLiteral("object") );
        model->addStatement( st );
        model->write( Factory::fcerr() );
        Factory::fcerr() << flush;
        cerr << "done with write()." << endl;

        if( model->contains( st ) )
        {
            cerr << "Statement found, removing it now." << endl;
            model->erase( st );
            cerr << "After remove, is-found:" << model->contains( st ) << endl;

            cerr << "--- final RDF/XML ---" << endl;
            model->write( Factory::fcerr() );
            Factory::fcerr() << flush;
        }
    }
    catch( exception& e )
    {
        cerr << "Error:" << e.what() << endl;
    }
    
    poptFreeContext(optCon);
    return 0;
}
    
    
