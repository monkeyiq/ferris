/******************************************************************************
*******************************************************************************
*******************************************************************************

    ftouch command line client
    Copyright (C) 2003 Ben Martin

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

    $Id: ftouch.cpp,v 1.7 2010/09/24 21:31:15 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisBoost.hh>
#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;
using namespace Ferris::Time;


const string PROGRAM_NAME = "fcompress";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

bool isRelativePath( const std::string& path )
{
    static fh_rex r = toregexh( "^[-_A-Za-z]+:/.*" );
    if( regex_match( path, r ) )
        return false;
    
    return( !starts_with( path, "/" )
            && !starts_with( path, "file:" )
            && !starts_with( path, "~" )
            && !starts_with( path, "root:" )
            && !starts_with( path, "x-ferris:" ) );
}


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        const char*   Date_CSTR              = 0;
        const char*   OutputTimeFormat_CSTR  = 0;
        const char*   ReferenceDate_CSTR     = 0;
        const char*   StampDate_CSTR         = 0;
        const char*   UpdateNamedStamps_CSTR = 0;
        const char*   NewContextMode_CSTR  = 0;
        unsigned long ChangeOnlyATime      = 0;
        unsigned long ChangeOnlyMTime      = 0;
        unsigned long DontCreate           = 0;
        unsigned long CreateDir            = 0;
        const char*   CreateEA_CSTR        = 0;
        const char*   CreateEAValue_CSTR   = 0;
        const char*   TouchEA_CSTR         = 0;
        const char*   TouchEAValue_CSTR    = 0;
        unsigned long Dummy                = 0;
        unsigned long Verbose              = 0;
        const char*   ExplicitSELinuxContext_CSTR  = 0;
        const char*   ExplicitSELinuxType_CSTR     = 0;
        const char*   ReferenceSELinuxContext_CSTR = 0;
        
        struct poptOption optionsTable[] =
            {
                { 0, 'a', POPT_ARG_NONE, &ChangeOnlyATime, 0,
                  "change only access time", "" },

                { "no-create", 'c', POPT_ARG_NONE, &DontCreate, 0,
                  "do not create any files", "" },

                { "create-dir", 0, POPT_ARG_NONE, &CreateDir, 0,
                  "targets should be directories, not files", "" },

                { "create-ea", 0, POPT_ARG_STRING, &CreateEA_CSTR, 0,
                  "create the specified EA at the touched location", "" },

                { "create-ea-value", 0, POPT_ARG_STRING, &CreateEAValue_CSTR, 0,
                  "optional value to write to the newly created EA at creation time.", "" },

                { "touch-ea", 0, POPT_ARG_STRING, &TouchEA_CSTR, 0,
                  "touch the given EA writing touch-ea-value to it.", "" },

                { "touch-ea-value", 0, POPT_ARG_STRING, &TouchEAValue_CSTR, 0,
                  "value to write to the touched-ea.", "" },
                
                { "chmod", 0, POPT_ARG_STRING, &NewContextMode_CSTR, 0,
                  "mode to create new objects with", "" },

                { "date", 'd', POPT_ARG_STRING, &Date_CSTR, 0,
                  "pass given time string and use instead of current time", "" },

                { "output-time-format", 0, POPT_ARG_STRING, &OutputTimeFormat_CSTR, 0,
                  "when displaying time values, use the strftime(3) format provided", "" },
                
                { 0, 'f', POPT_ARG_NONE, &Dummy, 0,
                  "ignored", "" },

                { 0, 'm', POPT_ARG_NONE, &ChangeOnlyMTime, 0,
                  "change only modification time", "" },

                { "reference", 'r', POPT_ARG_STRING, &ReferenceDate_CSTR, 0,
                  "use mtime and atime from context referenced at given URL instead of current time", "" },

//                 { 0, 't', POPT_ARG_STRING, &StampDate_CSTR, 0,
//                   "use [[CC]YY]MMDDhhmm[.ss] instead of current time", "" },

                { "time", 0, POPT_ARG_STRING, &UpdateNamedStamps_CSTR, 0,
                  "set  time  given  by  WORD: access atime use (same as -a) modify mtime (same as -m)", "" },

                { "set", 0, POPT_ARG_STRING, &ExplicitSELinuxType_CSTR, 0,
                  "set security context of copy to CONTEXT", "" },

                { "selinux-type", 0, POPT_ARG_STRING, &ExplicitSELinuxType_CSTR, 0,
                  "set security context of copy to CONTEXT", "" },
                
                { "context", 'Z', POPT_ARG_STRING, &ExplicitSELinuxContext_CSTR, 0,
                  "set the SELinux context to CONTEXT", "" },

                { "reference-context", 'X', POPT_ARG_STRING, &ReferenceSELinuxContext_CSTR, 0,
                  "set the SELinux context to the same as the given reference file", "" },
                
                
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },
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

        if (argc < 2 )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        ////////////////////////////////////////////////////////////////////////
        //
        // parse some popt options into params that can be used later
        //
        ////////////////////////////////////////////////////////////////////////
        bool    CreateContexts = !DontCreate;
        mode_t  Mode           = 0;
        bool    touchMTime     = true;
        bool    touchATime     = true;
        time_t  newATime       = getTime();
        time_t  newMTime       = newATime;
        string  timeFormat     = "%y %b %e %H:%M:%S";
        string  ExplicitSELinuxContext =
            ExplicitSELinuxContext_CSTR ? ExplicitSELinuxContext_CSTR : "";
            
        if( OutputTimeFormat_CSTR )
        {
            timeFormat = OutputTimeFormat_CSTR;
        }
        
        if( Date_CSTR )
        {
            struct tm tm = ParseTimeString( Date_CSTR );
            newATime = newMTime = mktime( &tm );
        }
        if( ReferenceDate_CSTR )
        {
            fh_context c = Resolve( ReferenceDate_CSTR );
            newATime = toType<time_t>( getStrAttr( c, "atime", "0" ));
            newMTime = toType<time_t>( getStrAttr( c, "mtime", "0" ));
        }
        if( ReferenceSELinuxContext_CSTR )
        {
            fh_context c = Resolve( ReferenceSELinuxContext_CSTR );
            ExplicitSELinuxContext = getStrAttr( c,
                                                 "dontfollow-selinux-context",
                                                 "",
                                                 true, true );
        }
        if( UpdateNamedStamps_CSTR )
        {
            string namedStamps = UpdateNamedStamps_CSTR;
            
            if( contains( namedStamps, "access" )
                || contains( namedStamps, "atime" )
                || contains( namedStamps, "use" ) )
            {
                touchATime = true;
            }
            if( contains( namedStamps, "modify" )
                || contains( namedStamps, "mtime" ) )
            {
                touchMTime = true;
            }
        }
        
        if( ChangeOnlyATime ) touchMTime = false;
        if( ChangeOnlyMTime ) touchATime = false;
        
        if( NewContextMode_CSTR )
        {
            Mode = Factory::MakeInitializationMode( NewContextMode_CSTR );
        }
        

        ////////////////////////////////////////////////////////////////////////
        //
        // apply changes to each URL
        //
        ////////////////////////////////////////////////////////////////////////
        stringlist_t srcs;
        srcs = expandShellGlobs( srcs, optCon );
        stringlist_t::iterator srcsiter = srcs.begin();
        stringlist_t::iterator srcsend  = srcs.end();
        for( ; srcsiter != srcsend ; ++srcsiter )
        {
            try
            {
                string srcURL = *srcsiter;

                if( Verbose )
                {
                    cerr << "touching " << srcURL << endl;
                    if( CreateContexts )
                    {
                        cerr << "  will create if needed, as a ";
                        if( CreateDir ) cerr << "directory";
                        else            cerr << "file";
                        if( Mode )      cerr << " with mode " << Mode;
                        else            cerr << " with default mode";
                        cerr << endl;
                    }
                    if( touchMTime )
                        cerr << "  setting mtime to:" << toTimeString(newMTime,timeFormat) << endl;
                    if( touchATime )
                        cerr << "  setting atime to:" << toTimeString(newATime,timeFormat) << endl;
                }

                if( isRelativePath( srcURL ) )
                {
                    srcURL = Shell::getCWDDirPath() + "/" + srcURL;
                    if( Verbose )
                        cerr << "you have given a relative path, new fully qualified path:" << srcURL << endl;
                }
                

                
                fh_context c = Shell::touch( srcURL,
                                             ExplicitSELinuxContext,
                                             CreateContexts,
                                             CreateDir,
                                             Mode,
                                             touchMTime,
                                             touchATime,
                                             newMTime,
                                             newATime );
                if( ExplicitSELinuxType_CSTR )
                {
                    setStrAttr( c,
                                "dontfollow-selinux-type",
                                ExplicitSELinuxType_CSTR,
                                true, true );
                }

                if( CreateEA_CSTR )
                {
                    string k = CreateEA_CSTR;
                    string v = "";
                    if( CreateEAValue_CSTR )
                        v = CreateEAValue_CSTR;
                    
                    Shell::createEA( c, k, v );
                }

                if( TouchEA_CSTR )
                {
                    string k = TouchEA_CSTR;
                    string v = "";
                    if( TouchEAValue_CSTR )
                        v = TouchEAValue_CSTR;

                    setStrAttr( c, k, v, true, true );
                }

                if( Verbose )
                {
                    cerr << "touched: " << srcURL << endl;
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


