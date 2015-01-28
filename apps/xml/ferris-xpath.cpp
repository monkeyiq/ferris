/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris xpath
    Copyright (C) 2011 Ben Martin

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

    $Id: ferris-xml-edit.cpp,v 1.3 2010/09/24 21:31:20 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <string>
#include <list>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <Ferris.hh>
#include <FerrisDOM.hh>
#include <FerrisBoost.hh>
#include <Resolver_private.hh>
#include <Ferrisls.hh>
#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;
using namespace Ferris::XML;


const string PROGRAM_NAME = "ferris-xpath";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

string commandLine = "";



int main( int argc, const char** argv )
{
    const char* XX_CSTR= "";
    unsigned long ShowVersion        = 0;

    struct poptOption optionsTable[] = {

//         { "show-columns", 0, POPT_ARG_STRING, &ShowColumnsCSTR, 0,
//           "Same as --show-ea",
//           "size,mime-type,name" },

        { "x", 'U', POPT_ARG_STRING, &XX_CSTR, 0,
          "x,...", 0 },

        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },
        
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

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* ...");

    if (argc < 1) {
//        poptPrintHelp(optCon, stderr, 0);
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }

    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
//         switch (c) {
//         }
    }

    if( ShowVersion )
    {
        cout << PROGRAM_NAME << " version: $Id: ferris-xml-edit.cpp,v 1.3 2010/09/24 21:31:20 ben Exp $\n"
             << "Written by Ben Martin, aka monkeyiq" << nl
             << nl
             << "Copyright (C) 2001-2011 Ben Martin" << nl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }

    try
    {
        const char* RootNameCSTR = poptGetArg(optCon);
        const char* XPathCSTR = poptGetArg(optCon);
                  
        cerr << "ctx:" << RootNameCSTR << endl
             << " xpath:" << XPathCSTR << endl;
        
        fh_context c  = Resolve(RootNameCSTR);
        fh_domdoc doc = Factory::StreamToDOM( c->getIStream() );
        DOMElementList_t nl = evalXPathToElements( doc, XPathCSTR );
        cerr << "nl.size:" << nl.size() << endl;
        for( DOMElementList_t::iterator niter = nl.begin(); niter != nl.end(); ++niter )
        {
            const DOMElement* n = *niter;
            cerr << "name:" << tostr(n->getNodeName()) << endl;
            cerr << "value:" << n->getNodeValue() << endl;
            cerr << "attr2:" << getAttribute( n, "attr2" ) << endl;
            cerr << "------------------------" << endl;
        }
        
        
        
    }
    catch( exception& e )
    {
        cerr << "cought:" << e.what() << endl;
        exit(1);
    }

    return(0);
}
        
