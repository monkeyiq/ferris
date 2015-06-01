/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
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

    $Id: testMakeDOM.cpp,v 1.2 2008/04/27 21:30:12 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include <Ferris.hh>
#include <FerrisDOM.hh>

using namespace std;
using namespace Ferris;
#define X(str) XStr(str).unicodeForm()


const string PROGRAM_NAME = "testMakeDOM";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}


/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/


int main( int argc, const char** argv )
{
    poptContext optCon;
    try
    {
        struct poptOption optionsTable[] = {
            FERRIS_POPT_OPTIONS
            POPT_AUTOHELP
            POPT_TABLEEND
        };
        
        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* ...");

        /***/
        int ch=-1;
        while ((ch = poptGetNextOpt(optCon)) >= 0)
        {}

        cout << "Option processing done." << endl;

        fh_context   c = Resolve( "./test.xml/company" );
        fh_domdoc    d = Factory::makeDOM( c );

        DOMNodeList* nl = d->getElementsByTagName(X("*"));
        unsigned int elementCount = nl->getLength();
        cout << "The tree just created contains: " << elementCount
             << " elements." << endl;

        poptFreeContext(optCon);
    }
    catch( exception& e )
    {
        cerr << "Cought exception:" << e.what() << endl << flush;
    }
    
    return 0;
}

