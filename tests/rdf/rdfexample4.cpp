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

    $Id: rdfexample4.cpp,v 1.5 2009/10/02 21:30:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisRDFCore.hh>

using namespace std;
using namespace Ferris;
using namespace Ferris::RDFCore;

const string PROGRAM_NAME = "rdfexample4";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

string yn( int x )
{
    if( x )
        return "yes";
    return "no";
}

int main( int argc, const char** argv )
{
    const char*   CMD_PRINT = 0;
    unsigned int  CMD_CONTAINS = 0;
    unsigned int  CMD_STATEMENTS = 0;
    unsigned int  CMD_SOURCES = 0;
    unsigned int  CMD_ARCS = 0;
    unsigned int  CMD_TARGETS = 0;
    unsigned int  CMD_SOURCE = 0;
    unsigned int  CMD_ARC = 0;
    unsigned int  CMD_TARGET = 0;
    unsigned int  CMD_ADD = 0;
    unsigned int  CMD_REMOVE = 0;
    unsigned int  CMD_ADD_TYPED = 0;
    const char*   CMD_PARSE_MODEL = 0;
    unsigned int  CMD_PARSE_STREAM = 0;
    unsigned int  CMD_ARCS_IN = 0;
    unsigned int  CMD_ARCS_OUT = 0;
    unsigned int  CMD_HAS_ARC_IN = 0;
    unsigned int  CMD_HAS_ARC_OUT = 0;
    unsigned int  CMD_QUERY = 0;
    unsigned int  CMD_SERIALIZE = 0;
    const char*   Subject_CSTR    = 0;
    const char*   Predicate_CSTR  = 0;
    const char*   Object_CSTR     = 0;
    const char*   ParserName_CSTR = 0;
    const char*   BaseURI_CSTR    = 0;
    
    struct poptOption optionsTable[] =
        {
            { "base-uri", 'B', POPT_ARG_STRING, &BaseURI_CSTR, 0,
              "Base URI to use in operations", "" },

            { "parse", 'i', POPT_ARG_STRING, &CMD_PARSE_MODEL, 0,
              "Parse syntax at URI into this model using --parser-name", "" },
            
            { "parser-name", 0, POPT_ARG_STRING, &ParserName_CSTR, 0,
              "Optional name of parser to use", "" },
            
            { "print", 'p', POPT_ARG_STRING, &CMD_PRINT, 0,
              "Prints all the statements", "path/to/rdf" },

            { "contains", 'c', POPT_ARG_INT, &CMD_CONTAINS, 0,
              "Check if statement is in the model", "" },

            { "subject", '1', POPT_ARG_STRING, &Subject_CSTR, 0,
              "subject used by commands (contains,statements)", "" },

            { "predicate", '2', POPT_ARG_STRING, &Predicate_CSTR, 0,
              "predicate used by commands (contains,statements)", "" },

            { "object", '3', POPT_ARG_STRING, &Object_CSTR, 0,
              "object used by commands (contains,statements)", "" },

            { "statements", 'l', POPT_ARG_INT, &CMD_STATEMENTS, 0,
              "find statements matching optional values subject, predicate, object", "" },

            { "sources", 0, POPT_ARG_INT, &CMD_SOURCES, 0,
              "Query for matching nodes", "" },
            
            
            FERRIS_POPT_OPTIONS
            POPT_AUTOHELP
            POPT_TABLEEND
        };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]*");

    if (argc < 2)
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
        bool writeStorage = CMD_ADD | CMD_REMOVE | CMD_ADD_TYPED
            | (CMD_PARSE_MODEL!=0) | CMD_PARSE_STREAM;
        bool createStorage = (CMD_PARSE_MODEL!=0) | CMD_PARSE_STREAM;

        fh_model model = Model::MemoryOnlyModel();
        
        // fh_stringstream storageoptions;
        // storageoptions << "hash-type='bdb',dir='.',write='" << yn( writeStorage )
        //                << "',new='" << yn(createStorage) << "'";
        // fh_storage storage = Storage::CreateStorage( "memory", "frodo", tostr(storageoptions) );
        // fh_model   model   = storage->CreateModel();

        if( CMD_PRINT )
        {
            fh_context c = Resolve( (string)CMD_PRINT + "/metadata" );
            model = Model::FromMetadataContext( c );
            model->write( Factory::fcout() );
            Factory::fcout() << flush;
        }
        if( CMD_PARSE_MODEL || CMD_PARSE_STREAM )
        {
            string     filename   = CMD_PARSE_MODEL;
            fh_context inputc     = Resolve( filename );
            string     parserName = ParserName_CSTR ? ParserName_CSTR : "";
            fh_parser  parser  = new Parser( parserName );
            fh_uri     baseuri = 0;
            cerr << "Parsing URI " << filename << " with " << parserName << " parser" << endl;
            if( BaseURI_CSTR )
                baseuri = new URI( BaseURI_CSTR );

            if( CMD_PARSE_MODEL )
            {
                parser->ParseIntoModel( model, inputc, baseuri );
            }

            model->write( Factory::fcout() );
            Factory::fcout() << flush;
        }
        
//         fh_uri   uri       = w->CreateURI( inputfilename_CSTR );
//         fh_parser  parser  = w->CreateParser();
//         parser->ParseIntoModel( model, uri, uri );

//         fh_statement st = w->CreateStatement(
//             w->CreateNodeURI( "http://purl.org/net/dajobe/" ),
//             w->CreateNodeURI( "http://purl.org/dc/elements/1.1/title" ),
//             w->CreateNodeLiteral( "My Home Page" ));
//         model->addStatement( st );
//         model->write( Factory::fcerr() );
//         Factory::fcerr() << flush;
//         cerr << "done with write()." << endl;

//         fh_statement partial_st = w->CreateStatement();
//         partial_st->setSubject(   w->CreateNodeURI("http://purl.org/net/dajobe/") );
//         partial_st->setPredicate( w->CreateNodeURI("http://purl.org/dc/elements/1.1/title") );
//         StatementIterator si = model->findStatements( partial_st );
//         for( ; si != StatementIterator(); ++si )
//         {
//             fh_statement st = *si;
//             fputs("  Matched statement: ", stdout);
//             librdf_statement_print( st->getRAW(), stdout);
//             fputc('\n', stdout);
//         }
    }
    catch( exception& e )
    {
        cerr << "Error:" << e.what() << endl;
    }
    
    poptFreeContext(optCon);
    return 0;
}
    
    
