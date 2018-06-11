/******************************************************************************
*******************************************************************************
*******************************************************************************

    fhead
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

    $Id: ferris-file-action.cpp,v 1.4 2010/09/24 21:31:13 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

/*
 * return 0 for success
 * return 1 for generic error
 * 
*/


#include <Ferris.hh>
#include <FerrisFileActions.hh>
#include <popt.h>

using namespace std;
using namespace Ferris;
using namespace Ferris::FileActions;

const string PROGRAM_NAME = "ferris-file-action";

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
unsigned long ViewMode             = 0;
unsigned long EditMode             = 0;
const char*   AppName              = 0;
unsigned long ListAppNames         = 0;


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {

        struct poptOption optionsTable[] =
            {
                { "verbose", 0, POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "view", 'v', POPT_ARG_NONE, &ViewMode, 0,
                  "view the given file URLs", "" },

                { "edit", 'e', POPT_ARG_NONE, &EditMode, 0,
                  "edit the given file URLs", "" },

                { "app", 'a', POPT_ARG_STRING, &AppName, 0,
                  "execute the named operation", "" },

                { "list", 'l', POPT_ARG_NONE, &ListAppNames, 0,
                  "list named operations for given urls", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* src1 src2 ...");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {
//         switch (c) {
//         }
        }

        if (argc < 1)
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
        ctxlist_t selection;
        stringlist_t srcs;
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            string srcURL = tmpCSTR;
            fh_context c = Resolve( srcURL );
            selection.push_back( c );
        }

        KnownFileOperations opcode = OP_VIEW;
        if( EditMode )
            opcode = OP_EDIT;

        if( ListAppNames )
        {
            fh_context c = getOpenWithContext( selection );
            cerr << "Actions are defined at:" << c->getURL() << endl;
            for( Context::iterator ci = c->begin(); ci!=c->end(); ++ci )
            {
                cout << (*ci)->getDirName() << endl;
            }
        }
        else
        {
            int selection_sz = selection.size();
            if( AppName )
            {
                KnownFileOperations opcode = OP_UNKNOWN;
                if( ViewMode )
                    opcode = OP_VIEW;
                if( EditMode )
                    opcode = OP_EDIT;

                ExecuteApplicaionByShortcutName( AppName, selection, opcode );
            }
            else
            {
                long rc = ExecuteOperation( opcode, selection );
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


