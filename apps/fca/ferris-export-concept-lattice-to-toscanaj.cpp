/******************************************************************************
*******************************************************************************
*******************************************************************************

    export part of a libferris concept lattice to a file readable by ToscanaJ
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

    $Id: ferris-export-concept-lattice-to-toscanaj.cpp,v 1.3 2010/09/24 21:31:10 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris/Ferris.hh>
#include <Ferris/FCA.hh>
using namespace std;
using namespace Ferris;
using namespace Ferris::FCA;


const string PROGRAM_NAME = "ferris-export-concept-lattice-to-toscanaj";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

int hadErrors = 0;

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long ShowVersion         = 0;
        unsigned long Verbose             = 0;
        const char* flatticeTreePath_CSTR = 0;
        const char* outputFileBase_CSTR = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "report more details than normal", "" },

                { "lattice-tree-path", 'L', POPT_ARG_STRING, &flatticeTreePath_CSTR, 0,
                  "url for the lattice tree", "" },

                { "output-filebase", 'o', POPT_ARG_STRING, &outputFileBase_CSTR, 0,
                  "path without extension for output", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* [-P ea-index-url] ");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}

        if (argc < 2 || !flatticeTreePath_CSTR || !outputFileBase_CSTR )
        {
            if( !flatticeTreePath_CSTR )
                cerr << "ERROR: you must supply an fca lattice tree path" << endl;
        
//        poptPrintHelp(optCon, stderr, 0);
            poptPrintUsage(optCon, stderr, 0);
            exit(5);
        }
        string flatticeTreePath = flatticeTreePath_CSTR;
        string outputFileBase   = outputFileBase_CSTR;
        
        if( ShowVersion )
        {
            cout << "ferris-export-concept-lattice-to-toscanaj version:"
                 << " $Id: ferris-export-concept-lattice-to-toscanaj.cpp,v 1.3 2010/09/24 21:31:10 ben Exp $\n"
                 << "Written by Ben Martin, aka monkeyiq" << endl
                 << endl
                 << "Copyright (C) 2005 Ben Martin" << endl
                 << "This is free software; see the source for copying conditions.  There is NO\n"
                 << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
                 << endl;
            exit(0);
        }    

        cerr << "getting the data..." << endl;
            
        fh_conceptLattice cl = 0;
        cl = ConceptLattice::load( flatticeTreePath );

        cl->exportAsToscanaJConceptualSchema( outputFileBase );
        poptFreeContext(optCon);
    }
    catch( exception& e )
    {
        LG_FCA_ER << PROGRAM_NAME << " cought exception:" << e.what() << endl;
        cerr << "error:" << e.what() << endl;
        exit(3);
    }
    
    cout << flush;
    return hadErrors;
}
