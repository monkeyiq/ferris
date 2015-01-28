/******************************************************************************
*******************************************************************************
*******************************************************************************

    test reading directories which are VirtualSoftLinks

    Copyright (C) 2002 Ben Martin

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

    $Id: timeparse.cpp,v 1.3 2009/04/14 21:30:12 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/Ferris.hh>
#include <Ferris/General.hh>

using namespace std;
using namespace Ferris;
using namespace Ferris::Time;

const string PROGRAM_NAME = "timeparse";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int main( int argc, const char** argv )
{
    int exit_status = 0;
    const char*   Format_CSTR         = 0;
    unsigned int  Y                   = 0;
    unsigned int  Absolute            = 0;
    
    struct poptOption optionsTable[] =
        {
//             { "explicit-size", 'n', POPT_ARG_INT, &Y, 0,
//               "size of input context", "" },

            { "absolute", 'a', POPT_ARG_NONE, &Absolute, 0,
              "parsing an absolute time instead of relative", "" },
            
            { "format", 'f', POPT_ARG_STRING, &Format_CSTR, 0,
              "format in strftime() style", "" },
            
            FERRIS_POPT_OPTIONS
            POPT_AUTOHELP
            POPT_TABLEEND
        };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]*");

    if (argc < 2)
    {
        poptPrintHelp(optCon, stderr, 0);
        exit(1);
    }
    
    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
    }


    while( const char* tmpCSTR = poptGetArg(optCon) )
    {
        try
        {
            string arg = tmpCSTR;

            if( Absolute )
            {
                cerr << "parsing absolute time" << endl;

                string format = "";
                if( Format_CSTR )
                    format = Format_CSTR;

                bool autoFresh = false;
                struct tm tm = Time::ParseTimeString( arg, format, autoFresh );
                time_t tt = mktime( &tm );
                string ts = toTimeString( tt, "%y %b %e %H:%M:%S" );
                cerr << "Input:" << arg << endl
                     << "tt:" << tt << " "
                     << "format:" << format << endl
                     << "Output:" << ts
                     << endl;
            }
            else
            {
                time_t tt = ParseRelativeTimeString( arg );
                string ts = toTimeString( tt, "%y %b %e %H:%M:%S" );
                
                cerr << "Input:" << arg
                     << " tt:" << tt << endl
                     << "Output:" << ts
                     << endl;
            }
        }
        catch( exception& e )
        {
            cerr << "error:" << e.what() << endl;
            exit_status = 1;
        }
    }
    
    poptFreeContext(optCon);
    return exit_status;
}
    
    
