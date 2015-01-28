/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris simpleroot

    Copyright (C) 2001 Ben Martin

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

    $Id: simpleroot.cpp,v 1.3 2008/05/25 21:30:06 ben Exp $

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
#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;


const string PROGRAM_NAME = "simpleroot";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


int main( int argc, const char** argv )
{
//    const char* RootContextClass = "Native";

    struct poptOption optionsTable[] = {
//         { "root-context-class", 0, POPT_ARG_STRING, &RootContextClass, 0,
//           "Name of the class that handles reading the root context", "Native" },

        FERRIS_POPT_OPTIONS
        POPT_AUTOHELP
        POPT_TABLEEND
    };
    poptContext optCon;


    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* RootName1 ...");

    if (argc < 2) {
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }
    
    
    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {}


    try
    {
        bool Done = false;
        for( int  First_Time = 1; !Done ; )
        {
            const char* RootNameCSTR = poptGetArg(optCon);

            /*
             * If there are no dirs specified we use "." and are done after this dir.
             */
            if( !RootNameCSTR && First_Time )
            {
                Done         = true;
                RootNameCSTR = ".";
            }
            else if (c < -1)
            {
                /* an error occurred during option processing */
                fprintf(stderr, "%s: %s\n", 
                        poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
                        poptStrerror(c));
                return 1;
            }
            else if( !RootNameCSTR )
            {
                break;
            }
            string RootName = RootNameCSTR;

            fh_context c = Resolve( RootName );
            cerr << "c.path:" << c->getDirPath() << endl;
            
            string earl = c->getURL();
            cerr << "earl:" << earl << endl;

            fh_context c2 = Resolve( earl );

            cerr << "c .path:" << c->getDirPath() << " URL:" << c->getURL() << endl;
            cerr << "c2.path:" << c2->getDirPath() << " URL:" << c2->getURL() << endl;
            exit(0);
        }
    }
    catch( exception& e )
    {
        cerr << "Cought exception:" << e.what() << endl << flush;
    }
    
    poptFreeContext(optCon);
    return 0;
}
