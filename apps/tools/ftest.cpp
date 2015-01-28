/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris ftest
    Copyright (C) 2010 Ben Martin

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

    $Id: ls.cpp,v 1.12 2008/04/27 21:30:11 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <config.h>

#include <string>
#include <list>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <Ferris/Ferris.hh>
#include <Ferrisls.hh>
#include <Ferris/Ferrisls_AggregateData.hh>
#include <Ferris/FerrisBoost.hh>

#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;

#define DEBUG 0

const string PROGRAM_NAME = "ftest";

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
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int RET_OK = 0;  // (test returns "true"  from info page)
int RET_BAD = 1; // (test returns "false" from info page)
int RET_ERR = 2;
int EXIT_CODE = RET_BAD;

unsigned long ShowVersion = 0;
unsigned long Debug = 0;

int isAttributeTrue( string earl, string eaname )
{
   fh_context c = Resolve( earl );
   if( isTrue( getStrAttr( c, eaname, "0" )))
      return RET_OK;
   return RET_BAD;
}

int main( int argc, const char** argv )
{
    struct poptOption optionsTable[] = {

//         { 0, 'x', POPT_ARG_NONE, &HorizList, 0,
//           "list entries by lines instead of by columns", 0 },

//         { "ea-index-path", 0, POPT_ARG_STRING, &EAIndexPath_CSTR, 0,
//           "which EA Index to use", "" },

        /*
         * Other handy stuff
         */

        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },

        { "debug", 0, POPT_ARG_NONE, &Debug, 0,
          "show debugging output", 0 },

        /*
         * Standard Ferris options
         */
        FERRIS_POPT_OPTIONS

        /**
         * Expansion of strange-url://foo*
         */
        FERRIS_SHELL_GLOB_POPT_OPTIONS
        POPT_AUTOHELP
        POPT_TABLEEND
   };
    poptContext optCon;


    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* ...");

    try
    {
       
       if( argc == 2 )
       {
          const char* arg = argv[1];
          if( arg && strlen(arg))
             return RET_OK;
          return RET_BAD;
       }
       if( argc == 3 )
       {
          if( !strcmp(argv[1],"["))
          {
             if( !strcmp(argv[2],"--help"))
             {
                cerr << "This command clones the coreutils 'test' command," << endl
                     << "but works natively on libferris filesystems" << endl;
                return RET_OK;
             }
             if( !strcmp(argv[2],"--version"))
             {
                cout << "ferrisls version: $Id: ls.cpp,v 1.12 2008/04/27 21:30:11 ben Exp $\n"
                     << "ferris   version: " << VERSION << nl
                     << "Written by Ben Martin, aka monkeyiq" << nl
                     << nl
                     << "Copyright (C) 2001-2010 Ben Martin" << nl
                     << "This is free software; see the source for copying conditions.  There is NO\n"
                     << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
                     << endl;
                return RET_OK;
             }
          }
       }


       if( argc == 3 && argv[1] && argv[2] )
       {
          string switchopt = argv[1];
          string param = argv[2];

          if( switchopt == "-d" )
             return isAttributeTrue( param, "is-dir" );
          if( switchopt == "-f" )
             return isAttributeTrue( param, "is-file" );
          if( switchopt == "-h" || switchopt == "-L"  )
             return isAttributeTrue( param, "dontfollow-is-link" );
          if( switchopt == "-g" )
             return isAttributeTrue( param, "is-setgid" );
          if( switchopt == "-k" )
             return isAttributeTrue( param, "is-sticky" );
          if( switchopt == "-r" )
             return isAttributeTrue( param, "readable" );
          if( switchopt == "-u" )
             return isAttributeTrue( param, "is-setuid" );
          if( switchopt == "-w" )
             return isAttributeTrue( param, "writable" );
          if( switchopt == "-x" )
             return isAttributeTrue( param, "runable" );
          if( switchopt == "-e" )
          {
             try 
             {
                fh_context c = Resolve( param );
                return RET_OK;
             }
             catch( ... )
             {
                return RET_BAD;
             }
          }
          if( switchopt == "-s" )
          {
             try 
             {
                fh_context c = Resolve( param );
                if( toType<guint64>(getStrAttr( c, "size", "0",
                                                true, true )) > 0 )
                   return RET_OK;
             }
             catch( ... )
             {
                return RET_BAD;
             }
          }

          if( switchopt == "-z" )
          {
             if( param.size() == 0 )
                return RET_OK;
             return RET_BAD;
          }
          if( switchopt == "-n" )
          {
             if( param.size() != 0 )
                return RET_OK;
             return RET_BAD;
          }
      
      
          cerr << "This option is not implemented" << endl;
          return RET_ERR;
       }

       if( argc == 4 && argv[1] && argv[2] && argv[3] )
       {
          string param1    = argv[1];
          string switchopt = argv[2];
          string param2    = argv[3];

          if( switchopt == "-nt" )
          {
             fh_context c1 = Resolve(param1);
             if( !Shell::contextExists( param2 ))
                return RET_OK;
             fh_context c2 = Resolve(param2);

             if( toType<guint64>(getStrAttr( c1, "mtime", "0", true, true ))
                 > toType<guint64>(getStrAttr( c2, "mtime", "0", true, true )))
             {
                return RET_OK;
             }
             return RET_BAD;
          }
          if( switchopt == "-ot" )
          {
             fh_context c2 = Resolve(param2);
             if( !Shell::contextExists( param1 ))
                return RET_OK;
             fh_context c1 = Resolve(param1);

             if( toType<guint64>(getStrAttr( c1, "mtime", "0", true, true ))
                 < toType<guint64>(getStrAttr( c2, "mtime", "0", true, true )))
             {
                return RET_OK;
             }
             return RET_BAD;
          }
          if( switchopt == "-ef" )
          {
             fh_context c1 = Resolve(param1);
             fh_context c2 = Resolve(param2);
             if( getStrAttr( c1, "inode", "0" ) == getStrAttr( c2, "inode", "1" )
                 && getStrAttr( c1, "device", "0" ) == getStrAttr( c2, "device", "1" ) )
             {
                return RET_OK;
             }
             return RET_BAD;
          }

          // string tests //
          if( switchopt == "=" )
          {
             if( param1 == param2 )
                return RET_OK;
             return RET_BAD;
          }
          if( switchopt == "!=" )
          {
             if( param1 != param2 )
                return RET_OK;
             return RET_BAD;
          }
      
          // numeric tests //
          if( switchopt == "-eq" )
          {
             if( toint(param1) == toint(param2) )
                return RET_OK;
             return RET_BAD;
          }
          if( switchopt == "-ne" )
          {
             if( toint(param1) != toint(param2) )
                return RET_OK;
             return RET_BAD;
          }
          if( switchopt == "-lt" )
          {
             if( toint(param1) < toint(param2) )
                return RET_OK;
             return RET_BAD;
          }
          if( switchopt == "-gt" )
          {
             if( toint(param1) > toint(param2) )
                return RET_OK;
             return RET_BAD;
          }
          if( switchopt == "-le" )
          {
             if( toint(param1) <= toint(param2) )
                return RET_OK;
             return RET_BAD;
          }
          if( switchopt == "-ge" )
          {
             if( toint(param1) >= toint(param2) )
                return RET_OK;
             return RET_BAD;
          }
      
       }
    }
    catch( exception& e )
    {
       return RET_BAD;
    }
    

   
   cerr << "This option is not implemented" << endl;
   return RET_ERR;
}
