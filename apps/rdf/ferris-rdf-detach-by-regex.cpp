/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris-rdf-detach-by-regex.cpp
    Copyright (C) 2005 Ben Martin

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

    $Id: ferris-rdf-detach-by-regex.cpp,v 1.4 2010/09/24 21:31:19 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
 * 
*/


#include <Ferris.hh>
#include <FerrisBoost.hh>
#include <FerrisSemantic.hh>
#include <popt.h>

using namespace std;
using namespace Ferris;
using namespace Ferris::RDFCore;

const string PROGRAM_NAME = "ferris-rdf-detach-by-regex";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

unsigned long Verbose              = 0;
unsigned long RemoveDanglingUUIDNodes = 0;
//const char* oldURL_CSTR = 0;

int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "remove-dangling-uuidnodes", 'f', POPT_ARG_NONE, &RemoveDanglingUUIDNodes, 0,
                  "when a uuidnode becomes unused remove it.", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* regex");

        /* Now do options processing */
        {
            int c=-1;
            while ((c = poptGetNextOpt(optCon)) >= 0)
            {}
        }
        

        if (argc < 2)
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        fh_model m = getDefaultFerrisModel();
        
        list<fh_node> danglingUUIDNodes;
        
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            
            if( tmpCSTR )
            {
                string regex = tmpCSTR;

                stringstream qss;
                qss << " \n"
                    << "PREFIX ferris: <http://witme.sf.net/libferris.web/rdf/ferris-attr/> \n"
                    << " \n"
                    << "SELECT ?uuid ?earl \n"
                    << "  WHERE {   \n"
                    << "     ?earl ferris:uuid ?uuid .  \n"
                    << "     FILTER (  \n"
                    << "       regex( str(?earl),  \"" << regex << "\")  \n"
                    << " ) }  \n";

                LG_RDF_D << "sparql query:" << endl << tostr(qss) << endl;
                
                BindingsIterator iter = m->findBindings( tostr(qss) );
                BindingsIterator e;
                fh_node earlnode = 0;
                fh_node uuidnode = 0;
                fh_node pred = Semantic::uuidPredNode();
                
                for( ; iter != e ; ++iter )
                {
                    earlnode = iter[ "earl" ];
                    uuidnode = iter[ "uuid" ];

                    LG_RDF_D << "detaching uuid:" << uuidnode->toString()
                             << " from c:" << earlnode->toString() << endl;

                    int AttachedFiles = 0;
                    {
                        StatementIterator e;
                        fh_statement partial_statement = new Statement();
                        partial_statement->setPredicate( pred );
                        partial_statement->setObject( uuidnode );

                        StatementIterator iter = m->findStatements( partial_statement );
                        AttachedFiles = distance( iter, StatementIterator() );
                    }
                    
                    if( Verbose )
                    {
                        cout << "rc:" << AttachedFiles << " " << earlnode->getURI()->toString() << endl;
                    }
                    
                    m->erase( earlnode, pred, uuidnode );
                    if( AttachedFiles <= 1 )
                    {
                        danglingUUIDNodes.push_back( uuidnode );
                    }
                }
            }
        }

        if( RemoveDanglingUUIDNodes )
        {
            for( rs<list <fh_node> > r(danglingUUIDNodes); r; ++r )
            {
                m->eraseTransitive( *r );
            }
        }
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


