/******************************************************************************
*******************************************************************************
*******************************************************************************

    fmedallion command line client
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

    $Id: ferriscdexe.cpp,v 1.3 2010/09/24 21:31:12 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
/*
 * return 0 for success
 * return 1 for generic error
 * return 5 for no suitable directories found to cd to
*/

#include <Ferris.hh>

#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ferriscd";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

typedef list< string > cdpath_t;
cdpath_t cdpath;

typedef std::list< fh_context > targets_t;
targets_t targets;

unsigned long RetainLinks          = 0;
unsigned long ResolveLinks         = 0;

//
// This uses any arg above that is not given explicitly as a param to the
// function.
//
// @return true if a directory was added to targets.
//
bool tryAddTarget( targets_t& targets,
                   bool useCDPath,
                   string earl )
{
    try
    {
//        cerr << "tryAddTarget() cdpath:" << useCDPath << " earl:" << earl << endl;

        
        fh_context c = Resolve( earl, RESOLVE_EXACT,
                                0
                                | ResolveLinks ? RESOLVEEX_UNROLL_LINKS : 0 );
        targets.push_back( c );
//        cerr << "tryAddTarget() added url:" << c->getURL() << endl;
        
        return true;
    }
    catch( exception& e )
    {
        if( useCDPath )
        {
            for( cdpath_t::iterator pi = cdpath.begin(); pi != cdpath.end(); ++pi )
            {
                if( tryAddTarget( targets, false, earl ))
                    return true;
            }
        }
    }
    return false;
}


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        const char*   CDPathCSTR           = 0;
        unsigned long Verbose              = 0;
        unsigned long CDableVars           = 0;
        bool useCDPath                     = true;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "report more details than normal", "" },

                { "retain-links", 'L', POPT_ARG_NONE, &RetainLinks, 0,
                  "follow symbolic links like -L in bash's cd", "" },
                
                { "resolve-links", 'P', POPT_ARG_NONE, &ResolveLinks, 0,
                  "when cd'ing into a link move to the physical path of the link", "" },

                { "try-cdable-vars", 0, POPT_ARG_NONE, &CDableVars, 0,
                  "if the given DIR can not be resolved try to dereference a env"
                  "var $DIR and go there", "" },

                { "cdpath", 0, POPT_ARG_STRING, &CDPathCSTR, 0,
                  "what cdpath to use", "" },
                
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

        if( !RetainLinks && !ResolveLinks )
        {
            RetainLinks = true;
        }

        //
        // Parse the CDPATH environment variable into a nice STL collection
        //
        if( CDPathCSTR )
        {
            string cdpathstr = CDPathCSTR;
            cerr << "cdpathstrA:" << cdpathstr << endl;

//             cerr << "PATH:" <<  g_getenv("PATH") << endl;
//             cerr << "CDPATH:" <<  g_getenv("CDPATH") << endl;
//             cerr << "CDPathCSTR:" <<  CDPathCSTR << endl;
            
            while( true )
            {
                int pos = cdpathstr.find("::");
                if( pos == string::npos )
                    break;
                fh_stringstream ss;
                ss << ":" << Shell::quote(Shell::getCWDString()) << ":";
                cdpathstr.replace( pos, 2, tostr(ss) );
            }

            cerr << "cdpathstr:" << cdpathstr << endl;
            Util::parseSeperatedList( cdpathstr,
                                      cdpath,
                                      back_inserter( cdpath ),
                                      ':' );
        }

        typedef vector<string> srcs_t;
        srcs_t srcs;
        
        while( const char* RootNameCSTR = poptGetArg(optCon) )
        {
            string RootName = RootNameCSTR;
            srcs.push_back( RootName );
//            cerr << "adding src:" << RootName << endl;
        }

        // default is to go HOME //
        if( srcs.empty() )
        {
            const gchar* p = g_getenv("HOME");
            cout << "cd " << Shell::quote(p) << endl;
            return 0;
        }
        
        //
        // try to add each directory to the target list
        // 
        for( srcs_t::iterator si = srcs.begin(); si!=srcs.end(); ++si )
        {
            string earl = *si;

            useCDPath = !earl.empty() && earl[0] != '/';

            tryAddTarget( targets, useCDPath, earl );
        }

        //
        // If we have no targets but there was a string given to cd to
        // then they might want an environment variable dereferenced.
        //
        if( CDableVars && targets.empty() )
        {
            for( srcs_t::iterator si = srcs.begin(); si!=srcs.end(); ++si )
            {
                string earl = *si;
                if( const gchar* tmp = g_getenv(earl.c_str()) )
                {
                    tryAddTarget( targets, false, earl );
                }
            }
        }

        if( targets.empty() )
            return 5;

        Context::UnrollQueryResultContexts( targets, targets.begin() );
        
        //
        // make a command to cd to the possibly many target directories.
        //
        fh_context lastc = targets.back();
        targets.pop_back();
        fh_stringstream cmdss;
        
        for( targets_t::iterator ti = targets.begin(); ti!=targets.end(); ++ti )
        {
            fh_context c = *ti;
            cmdss << "pushd " << Shell::quote( c->getDirPath() ) << ";" << endl;
        }
        cmdss << "cd " << Shell::quote( lastc->getDirPath() ) << ";" << endl;
        cout << tostr(cmdss);
    }
    catch( exception& e )
    {
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


