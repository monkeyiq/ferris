/******************************************************************************
*******************************************************************************
*******************************************************************************

    fmkdir
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

    $Id: fmkdir.cpp,v 1.6 2010/09/24 21:31:15 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
 * return 2 for no -p and no parent existing
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

const string PROGRAM_NAME = "fmkdir";

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
unsigned long Parents  = 0;
unsigned long Mode     = 0;

void verboseDirMade( fh_context c )
{
    if( Verbose )
        cerr << "fmkdir: created directory " << quote( c->getURL() ) << endl;
}



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
                  "set permission mode (as in chmod), not rwxrwxrwx - umask", "" },

                { "parents", 'p', POPT_ARG_NONE, &Parents, 0,
                  "no error if existing, make parent directories as needed", "" },
                
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
        {}

        if (argc < 2)
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        if( ModeCSTR )
        {
            Mode = Factory::MakeInitializationMode( ModeCSTR );
            cerr << "Have mode:" << Mode << endl;
        }

        if( Verbose )
        {
            createdHandler.connect( sigc::ptr_fun( verboseDirMade ));
        }
        
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            string URL = tmpCSTR;
            
            try
            {
                if( Parents )
                {
                    fh_context c = CreateDir( URL, Parents, Mode, createdHandler );
//                    verboseDirMade( c );
                }
                else
                {
                    fh_context p = 0;
                    try
                    {
                        p = Resolve( URL, RESOLVE_PARENT );
                    }
                    catch( exception& e )
                    {
                        cerr << "fmkdir: parent of url:" << quote( URL )
                             << " does not exist and no -p option used." << endl;
                        exit( 2 );
                    }
                    try
                    {
                        fh_context c = CreateDir( URL, false, Mode, createdHandler );
//                        verboseDirMade( c );
                    }
                    catch( exception& e )
                    {
                        cerr << "fmkdir: error:" << e.what() << endl;
                        exit( 3 );
                    }
                }
            }
            catch( exception& e )
            {
                cerr << "error:" << e.what() << endl;
                exit_status = 1;
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


