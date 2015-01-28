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

    $Id: rdfassert.cpp,v 1.4 2009/10/02 21:30:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisRDFCore.hh>

using namespace std;
using namespace Ferris;
using namespace Ferris::RDFCore;

const string PROGRAM_NAME = "rdfassert";

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
    const char*   inputfilename_CSTR  = 0;
    unsigned int  ObjectIsLiteral     = 0;
    
    struct poptOption optionsTable[] =
        {
            { "object-is-literal", 'l', POPT_ARG_NONE, &ObjectIsLiteral, 0,
              "given object is literal instead of URI", "" },
            
            FERRIS_POPT_OPTIONS
            POPT_AUTOHELP
            POPT_TABLEEND
        };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* RDF/Repository Subject Predicate Object");

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

    if( argc < 5 )
    {
        poptPrintHelp(optCon, stderr, 0);
        exit(1);
    }
    
    try
    {
        string infilepath = poptGetArg(optCon);
        string subjstr    = poptGetArg(optCon);
        string predstr    = poptGetArg(optCon);
        string objestr    = poptGetArg(optCon);

        fh_model   m = Model::FromMetadataContext( infilepath );

        fh_node subj = Node::CreateURI( subjstr );
        fh_node pred = Node::CreateURI( predstr );
        fh_node obje = ObjectIsLiteral
            ? Node::CreateLiteral( objestr )
            : Node::CreateURI( objestr );
    
        m->insert( subj, pred, obje );
        m->sync();
    }
    catch( exception& e )
    {
        cerr << "Error:" << e.what() << endl;
    }
    
    poptFreeContext(optCon);
    return 0;
}
    
    
