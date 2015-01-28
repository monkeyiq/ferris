/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris fchown
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

const string PROGRAM_NAME = "freadlink";

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

int RET_OK = 0;  
int RET_BAD = 1; 
int RET_ERR = 2;
int hadErrors = RET_OK;


unsigned long Canonicalize = 0;
unsigned long CanonicalizeExisting = 0;
unsigned long CanonicalizeMissing = 0;
unsigned long NoNewline = 0;
unsigned long SilentMode = 0;
unsigned long Verbose = 0;
unsigned long ShowVersion = 0;

unsigned long CanonicalizeAnyOption = 0;

void newline()
{
   if(NoNewline) 
      cout << flush;
   else
      cout << endl;
}


void visit( fh_context ctx )
{
   // cerr << "visit() islink:" << isTrue( getStrAttr( ctx, "is-link", "0" ))
   //      << " ctx:" << ctx->getURL() << endl;
   if( !CanonicalizeAnyOption )
   {
      if( isTrue( getStrAttr( ctx, "is-link", "0" )))
      {
         cout << getStrAttr( ctx, "link-target", "" );
         newline();
      }
   }
   else
   {
      cout << ctx->getURL();
      newline();
   }
}


int main( int argc, const char** argv )
{
    struct poptOption optionsTable[] = {

       { "canonicalize", 'f', POPT_ARG_NONE, &Canonicalize, 0,
         "If any component of the file name"
         " except the last one is missing or unavailable, freadlink produces"
         " no output and exits with a nonzero exit code", 0 },

       { "canonicalize-existing", 'e', POPT_ARG_NONE, &CanonicalizeExisting, 0,
         "If any component is missing or"
         " unavailable, freadlink produces no output and exits with a"
         " nonzero exit code", 0 },

       { "canonicalize-missing", 'm', POPT_ARG_NONE, &CanonicalizeMissing, 0,
         "If any component is missing or unavailable, freadlink treats it as a directory", 0 },
       
       { "no-newline", 'n', POPT_ARG_NONE, &NoNewline, 0,
         "Do not output the trailing newline", 0 },

       { "silent", 's', POPT_ARG_NONE, &SilentMode, 0,
         "Suppress most error messages", 0 },
       { "quiet", 'q', POPT_ARG_NONE, &SilentMode, 0,
         "Suppress most error messages", 0 },

      
       { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
         "Output a diagnostic for every file processed", 0 },

       { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
         "Show version information and quit", 0 },
       
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
    poptSetOtherOptionHelp(optCon, "fchown [OPTION]... {NEW-OWNER | --reference=REF_URL} URL1 URL2 ...");
                                                        
    if (argc < 1) {
//        poptPrintHelp(optCon, stderr, 0);
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

    if( ShowVersion )
    {
       cout << PROGRAM_NAME << " version: $Id: ls.cpp,v 1.12 2008/04/27 21:30:11 ben Exp $\n"
            << "ferris   version: " << VERSION << nl
            << "Written by Ben Martin, aka monkeyiq" << nl
            << nl
            << "Copyright (C) 2001-2010 Ben Martin" << nl
            << "This is free software; see the source for copying conditions.  There is NO\n"
            << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
            << endl;
       exit(0);
    }

    CanonicalizeAnyOption = Canonicalize
       || CanonicalizeExisting
       || CanonicalizeMissing;
    
    try
    {
        stringlist_t srcs;
        srcs = expandShellGlobs( srcs, optCon );
        if( srcs.empty() )
        {
           poptPrintUsage(optCon, stderr, 0);
           exit(1);
        }
        stringlist_t::iterator srcsiter = srcs.begin();
        stringlist_t::iterator srcsend  = srcs.end();
        
        bool Done = false;
        for( int  First_Time = 1; !Done && srcsiter != srcsend ; ++srcsiter )
        {
            string earl = *srcsiter;
            First_Time = 0;

            if( CanonicalizeMissing )
            {
               std::string ret = canonicalizeMissing( earl );
               cout << ret;
               newline();
               continue;
            }
            
            if( Canonicalize )
            {
               fh_context parent = Resolve( earl, RESOLVE_PARENT );
               string rdn = parent->getLastPartOfName( earl );
               string ret = parent->appendToPath( parent->getDirPath(), rdn );
               cerr << "parent c:" << parent->getURL() << endl;
               cerr << "rdn:" << rdn << endl;
               cerr << "ret:" << ret << endl;
               fh_context c = Resolve( ret );
               visit( c );
               continue;
            }
            
            fh_context c = Resolve( earl );
            visit( c );
        }
    }
    catch( NoSuchSubContext& e )
    {
       if( !CanonicalizeAnyOption || CanonicalizeExisting )
       {
          exit(RET_BAD);
       }
    }
    catch( NoSuchContextClass& e )
    {
//        cerr << "Invalid context class given:" << RootContextClass << endl;
        cerr << "e:" << e.what() << endl;
        exit(1);
    }
    catch( exception& e )
    {
        cerr << "cought:" << e.what() << endl;
        exit(1);
    }

    poptFreeContext(optCon);
    cout << flush;
    return hadErrors;
}
