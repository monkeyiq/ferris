/******************************************************************************
*******************************************************************************
*******************************************************************************

    fmodestr2octal
    Copyright (C) 2002 Ben Martin

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

    $Id: fmodestr2octal.cpp,v 1.3 2010/09/24 21:31:15 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
 * 
 *
 *
 *
 *
 **/


#include <Ferris/Ferris.hh>
#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;
using namespace Ferris::Shell;

const string PROGRAM_NAME = "fmodestr2octal";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

const char* ModeCSTR   = 0;
unsigned long Verbose  = 0;

int main( int argc, char** argv )
{
    int exit_status = 0;
    ContextCreated_Sig_t createdHandler;
    
    try
    {
        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "mode", 'm', POPT_ARG_STRING, &ModeCSTR, 0,
                  "Mode string to convert to octal", "" },

                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* basemode1 basemode2 ...");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}

        if (argc < 2 || !ModeCSTR )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            string URL    = tmpCSTR;
            fh_context c  = Resolve( URL );
            fh_chmod cmod = Factory::MakeChmod( ModeCSTR );

            if( Verbose )
            {
                cout << "using base mode from url:" << c->getURL() << endl;
            }
            
            mode_t m = cmod->apply( toType<mode_t>(getStrAttr( c, "mode", "0" )));
//            cerr << "init mode:" <<  cmod->getInitializationMode()
            cerr << "init mode:" <<  cmod->apply( 0 )
                 << " mode:" <<  m
                 << endl;
        }
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


