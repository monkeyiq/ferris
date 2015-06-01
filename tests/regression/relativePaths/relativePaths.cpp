/******************************************************************************
*******************************************************************************
*******************************************************************************

    A test for the resolution of relative paths in libferris.so

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

    $Id: relativePaths.cpp,v 1.3 2008/05/24 21:30:08 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include <string>
#include <list>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <Ferris.hh>
#include <popt.h>

using namespace std;
using namespace Ferris;


const string PROGRAM_NAME = "relativePaths";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}





class relativePaths : public sigc::trackable
{
    string earl;
    fh_context rctx;

public:

    relativePaths()
        :
        rctx(0)
        {
        }


    void setURL( const string& s )
        {
            earl = s;
        }
    
    void resolveTest( fh_context c, string xdn )
        {
            fh_context test  = c->getRelativeContext( xdn );
            cerr << "testRelativePaths()  xdn :" <<  xdn << endl;
            cerr << "testRelativePaths()  c   :" <<     c->getDirPath() << endl;
            cerr << "testRelativePaths()  test:" <<  test->getDirPath() << endl;
        }


    int operator()( const string& xdn )
        {
            fh_context cwd  = Ferris::Shell::getCWD();
            
            cwd->read();
            resolveTest( cwd, xdn );
        }
};


int main( int argc, const char** argv )
{
    relativePaths mainobj;

    const char* earl = "";
    const char* xdn = 0;
    
    struct poptOption optionsTable[] = {
//        { "url", 0, POPT_ARG_STRING, &earl, 0, "", "" },
        POPT_AUTOHELP
        { NULL, 0, 0, NULL, 0 }
   };
    poptContext optCon;


    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* xdn1 ...");

    if (argc < 2) {
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


//     try {
//         mainobj.setURL( earl );
//     }
//     catch( NoSuchContextClass& e )
//     {
//         cerr << "Invalid url given e:" << e.what() << endl;
//         exit(1);
//     }
    

    for( int First_Time = 1; xdn = poptGetArg(optCon); )
    {
        if( !xdn && First_Time )
            usage(optCon, 1, "Specify a root name", ".e.g., /");

        if (c < -1) {
            /* an error occurred during option processing */
            fprintf(stderr, "%s: %s\n", 
                    poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
                    poptStrerror(c));
            return 1;
        }

        mainobj( xdn );
    }
    
    poptFreeContext(optCon);
    return 0;
}
