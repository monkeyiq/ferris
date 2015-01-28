/******************************************************************************
*******************************************************************************
*******************************************************************************

    fclipundo
    Copyright (C) 2001 Ben Martin

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

    $Id: fclipundo.cpp,v 1.3 2010/09/24 21:31:14 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris.hh>
#include <ClipAPI.hh>
#include <popt.h>

using namespace std;
using namespace Ferris;
using namespace Ferris::FileClip;
using namespace Ferris::Cursor;

const string PROGRAM_NAME = "fclipundo";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

int main( int argc, char** argv )
{
    try
    {
        const char* strx              = 0;
        unsigned long Verbose         = 0;
        unsigned long DontMoveCursor  = 0;

        struct poptOption optionsTable[] =
            {
                { "dont-move-cursor", 'd', POPT_ARG_NONE, &DontMoveCursor, 0,
                  "leave cursor at current position", "" },

                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

//             { "target-directory", 0, POPT_ARG_STRING, &DstNameCSTR, 0,
//               "Specify destination explicity, all remaining URLs are assumed to be source files",
//               "DIR" },

                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]*");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {
//         switch (c) {
//         }
        }

//         if (argc < 2)
//         {
//             poptPrintHelp(optCon, stderr, 0);
//             exit(1);
//         }

        fh_context clip   = Factory::getFileClipboard();
        fh_context cursor = Factory::getCursor( clip );
        
        Undo( cursor );

        if( !DontMoveCursor )
        {
            cursorPrev( cursor );
        }
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return 0;
}


