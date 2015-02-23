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

    $Id: ferris-redirect.cpp,v 1.9 2010/09/24 21:31:12 ben Exp $

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

const string PROGRAM_NAME = "ferris-redirect";

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
unsigned long Truncate             = 0;
unsigned long Append               = 0;
unsigned long DontAttemptMount     = 0;
const char* EAName_CSTR = 0;


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "trunc", 'T', POPT_ARG_NONE, &Truncate, 0,
                  "truncate output file", "" },

                { "append", 'z', POPT_ARG_NONE, &Append, 0,
                  "append to output file", "" },
                
                { "ea", 'a', POPT_ARG_STRING, &EAName_CSTR, 0,
                  "put redirection into this EA", "" },

                { "dont-attempt-mount", 'x', POPT_ARG_NONE, &DontAttemptMount, 0,
                  "truncate output file", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* src1 src2 ...");

        /* Now do options processing */
        {
            int c=-1;
            while ((c = poptGetNextOpt(optCon)) >= 0)
            {}
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
            fh_context c = Shell::acquireContext( srcURL, 0, false );
            selection.push_back( c );
        }

        if( selection.empty() )
        {
            cerr << "please supply a URL where data is to be redirected to." << endl;
            exit(1);
        }
        
        fh_context c = selection.front();
        fh_istream iss = Factory::fcin();

        ferris_ios::openmode om = std::ios::out | ferris_ios::o_mseq;
        if( Truncate )
        {
            om |= std::ios::trunc;
        }
        if( Append )
        {
            om |= std::ios::ate;
        }
        

        fh_ostream oss = Factory::fcerr();
        if( EAName_CSTR )
        {
            string eaname = EAName_CSTR;
            string v = StreamToString( iss );
            bool create = true;
            bool throw_for_errors = true;
            bool dontDelegateToOvermountContext = DontAttemptMount;
            setStrAttr( c, eaname, v, create, throw_for_errors, dontDelegateToOvermountContext );
        }
        else
        {
            oss = c->getIOStream( om );
            copy( istreambuf_iterator<char>( iss ),
                  istreambuf_iterator<char>(),
                  ostreambuf_iterator<char>( oss ) );
        }
        
        if( Verbose )
        {
            cerr << "ferris redirection successfully performed." << endl;
        }
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


