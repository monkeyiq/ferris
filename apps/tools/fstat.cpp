/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris fstat
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

const string PROGRAM_NAME = "fstat";

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

int hadErrors = 0;

const char* DefaultPrintfString = ""
   "  File:%N\n"
   "  Size:%s\tBlocks:%b\tIO Block:%B\t%F\n"
   "Device:%t/%T\tInode:%i\tLinks:%h\n"
   "Access: (%a/%A)\tUid: (%u/%U)\tGid: (%g/%G)\n"
   "Access: %x\n"
   "Modify: %y\n"
   "Change: %z\n"
   "";
const char* TersePrintfString = "%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o\n";

unsigned long Verbose = 0;
unsigned long ShowVersion = 0;
unsigned long Dereference = 0;
unsigned long ShowFilesystemInfoInstead = 0;
const char* FormatString_CSTR = 0;
const char* PrintfString_CSTR = 0;
unsigned long Terse = 0;

unsigned long FuzzUserAndGroupToCurrent = 0;
unsigned long FuzzTimesToZero = 0;
unsigned long FuzzTimesToCurrent = 0;
unsigned long FuzzAllToCurrent = 0;

string DefaultProtectionRaw = "0";
string DefaultProtectionLs  = "----------";
string DefaultUserID;
string DefaultUserName;
string DefaultGroupID;
string DefaultGroupName;
string DefaultMTime = "0";
string DefaultATime = "0";
string DefaultCTime = "0";

string LinkPrefix = "dontfollow-";

//
// This is expanded from all format options to be the single source for visit()
//
string FormatString = "";
string expandPrintfEscapes( const std::string& s )
{
   string ret = s;
   ret = Util::replace_all( ret, "\\f", "\f" );
   ret = Util::replace_all( ret, "\\r", "\r" );
   ret = Util::replace_all( ret, "\\n", "\n" );
   ret = Util::replace_all( ret, "\\t", "\t" );
   ret = Util::replace_all( ret, "\\v", "\v" );
   ret = Util::replace_all( ret, "\\a", "\a" );
   ret = Util::replace_all( ret, "\\b", "\b" );
   ret = Util::replace_all( ret, "\\e", "\e" );
   ret = Util::replace_all( ret, "\\e", "\x1B" );
   return ret;
}


void visit( fh_context ctx )
{
   stringstream ss;
   ss << FormatString;
   char c;
   while( ss >> noskipws >> c )
   {
      if( c=='%' )
      {
         if( ss >> noskipws >> c )
         {
            switch(c)
            {
               case 'n':
                  cout << ctx->getDirPath();
                  break;
               case 'N':
                  cout << "`";
                  cout << ctx->getDirPath();
                  cout << "'";
                  break;
               case 's':
                  cout << getStrAttr( ctx, LinkPrefix + "size", "0" );
                  break;
               case 'b':
                  cout << getStrAttr( ctx, LinkPrefix + "block-count", "0" );
                  break;
               case 'B':
                  cout << getStrAttr( ctx, LinkPrefix + "block-size", "0" );
                  break;
               case 'o':
                  cout << getStrAttr( ctx, LinkPrefix + "block-size", "0" );
                  break;
               case 'f':
                  cout << getStrAttr( ctx, LinkPrefix + "mode", "0" );
                  break;
               case 't':
                  cout << getStrAttr( ctx, LinkPrefix + "device-major-hex", "0" );
                  break;
               case 'T':
                  cout << getStrAttr( ctx, LinkPrefix + "device-minor-hex", "0" );
                  break;
               case 'D':
                  cout << getStrAttr( ctx, LinkPrefix + "device-hex", "0" );
                  break;
               case 'i':
                  cout << getStrAttr( ctx, LinkPrefix + "inode", "0" );
                  break;
               case 'h':
                  cout << getStrAttr( ctx, LinkPrefix + "hard-link-count", "0" );
                  break;
               case 'a':
                  cout << getStrAttr( ctx, LinkPrefix + "protection-raw", DefaultProtectionRaw );
                  break;
               case 'A':
                  cout << getStrAttr( ctx, LinkPrefix + "protection-ls", DefaultProtectionLs );
                  break;
               case 'u':
                  cout << getStrAttr( ctx, LinkPrefix + "user-owner-number", DefaultUserID );
                  break;
               case 'U':
                  cout << getStrAttr( ctx, LinkPrefix + "user-owner-name", DefaultUserName );
                  break;
               case 'F':
                  cout << getStrAttr( ctx, LinkPrefix + "filesystem-filetype", "" );
                  break;
               case 'g':
                  cout << getStrAttr( ctx, LinkPrefix + "group-owner-number", DefaultGroupID );
                  break;
               case 'G':
                  cout << getStrAttr( ctx, LinkPrefix + "group-owner-name", DefaultGroupName );
                  break;
               case 'x':
                  cout << getStrAttr( ctx, LinkPrefix + "atime-display", "" );
                  break;
               case 'y':
                  cout << getStrAttr( ctx, LinkPrefix + "mtime-display", "" );
                  break;
               case 'z':
                  cout << getStrAttr( ctx, LinkPrefix + "ctime-display", "" );
                  break;
               case 'X':
                  cout << getStrAttr( ctx, LinkPrefix + "atime", DefaultATime );
                  break;
               case 'Y':
                  cout << getStrAttr( ctx, LinkPrefix + "mtime", DefaultMTime );
                  break;
               case 'Z':
                  cout << getStrAttr( ctx, LinkPrefix + "ctime", DefaultCTime );
                  break;
            }
         }
      }
      else
      {
         cout << c;
      }
   }
   cout << flush;
}


int main( int argc, const char** argv )
{
    struct poptOption optionsTable[] = {

       { "dereference", 'L', POPT_ARG_NONE, &Dereference, 0,
         "dereference links before getting stat information", 0 },

       { "file-system", 'f', POPT_ARG_NONE, &ShowFilesystemInfoInstead, 0,
         "Report information about the file systems where the given files"
         " are located instead of information about the files themselves.", 0 },

       { "format", 'c', POPT_ARG_STRING, &FormatString_CSTR, 0,
         "Use FORMAT rather than the default format", 0 },

       { "printf", 0, POPT_ARG_STRING, &PrintfString_CSTR, 0,
         "Like `--format', but interpret backslash escapes, and do not output a mandatory"
         " trailing newline. Use FORMAT rather than the default format", 0 },

       { "terse", 't', POPT_ARG_NONE, &Terse, 0,
         "Print the information in terse form, suitable for parsing by other programs.", 0 },

       { "fuzz-user-and-group", 0, POPT_ARG_NONE, &FuzzUserAndGroupToCurrent, 0,
         "Fake the file owner and group to current user data if nothing is known", 0 },

       { "fuzz-times", 0, POPT_ARG_NONE, &FuzzTimesToZero, 0,
         "Fake the mtime, atime, and ctime to zero if nothing is known", 0 },

       { "fuzz-times-to-now", 0, POPT_ARG_NONE, &FuzzTimesToCurrent, 0,
         "Fake the mtime, atime, and ctime to current time if nothing is known", 0 },

       { "fuzz", 0, POPT_ARG_NONE, &FuzzAllToCurrent, 0,
         "Fake all data to current environment if nothing is known", 0 },

       
      
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
    if( ShowFilesystemInfoInstead )
    {
       cerr << "FIXME: not implemented!" << endl;
       exit(1);
    }
    

    FormatString = DefaultPrintfString;
    if( Terse )
    {
       FormatString = TersePrintfString;
    }
    if( FormatString_CSTR )
    {
       FormatString = FormatString_CSTR;
       FormatString += "\n";
    }
    else
    {
       if( PrintfString_CSTR )
       {
          FormatString = PrintfString_CSTR;
       }
       FormatString = expandPrintfEscapes( FormatString );
    }

    if( Dereference )
    {
       LinkPrefix = "";
    }

    if( FuzzAllToCurrent )
    {
       FuzzUserAndGroupToCurrent = 1;
       FuzzTimesToZero = 1;

       DefaultProtectionRaw = "600";
       DefaultProtectionLs  = "-rw-------";
    }

    if( FuzzUserAndGroupToCurrent )
    {
       DefaultUserID = tostr( Shell::getUserID() );
       DefaultUserName = Shell::getUserName(Shell::getUserID());
       DefaultGroupID = tostr( Shell::getGroupID() );
       DefaultGroupName = Shell::getGroupName(Shell::getGroupID());
    }
    if( FuzzTimesToCurrent || FuzzTimesToZero )
    {
       time_t tt = 0;
       if( FuzzTimesToCurrent )
       {
          tt = Time::getTime();
       }
       string s = tostr(tt);
       DefaultMTime = s;
       DefaultATime = s;
       DefaultCTime = s;
    }
    
    
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

            fh_context c = Resolve( earl );
            visit( c );
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
        cerr << "ls.cpp cought:" << e.what() << endl;
        exit(1);
    }

    poptFreeContext(optCon);
    cout << flush;
    return hadErrors;
}
