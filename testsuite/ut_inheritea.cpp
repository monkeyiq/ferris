/******************************************************************************
*******************************************************************************
*******************************************************************************

    unit test code
    Copyright (C) 2003 Ben Martin

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

    $Id: ut_inheritea.cpp,v 1.1 2006/12/10 04:52:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris.hh>
#include <Ferris/Iterator.hh>
#include <popt.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <map>
#include <list>
#include <iterator>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ut_inheritea";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void runtest( fh_context xml )
{
    cerr << "runtest xml:" << xml->getURL() << endl;
    xml->read();
    fh_context base = xml->getSubContext( "basedir" );
    cerr << "runtest base:" << base->getURL() << endl;

    if( getStrAttr( base, "base", "" ) != "fred" )
    {
        cerr << "ERROR bad base ea on base context:" << base->getURL() << endl;
        return;
    }
    
    fh_context fileA = base->getSubContext( "fileA" );
    if( getStrAttr( fileA, "base", "" ) != "fred" )
    {
        cerr << "ERROR bad base ea on fileA context:" << fileA->getURL() << endl;
        return;
    }
    
    fh_context fileB = base->getSubContext( "fileB" );
    if( getStrAttr( fileB, "base", "" ) != "override" )
    {
        cerr << "ERROR bad base ea on fileB context:" << fileB->getURL() << endl;
        return;
    }
    
    fh_context dir01  = base->getSubContext( "dir01" );
    fh_context file02 = dir01->getSubContext( "file02" );
    
    if( getStrAttr( file02, "base", "" ) != "fred" )
    {
        cerr << "ERROR bad \"base\" ea on file02 context:" << file02->getURL() << endl;
        cerr << "expected:fred:" << endl;
        cerr << "actual:" << getStrAttr( file02, "base", "<can't read>" ) << endl;
        return;
    }
    
    if( getStrAttr( file02, "a", "" ) != "2" )
    {
        cerr << "ERROR bad \"a\" ea on file02 context:" << file02->getURL() << endl;
        return;
    }
    
    cerr << "Success" << endl;
}


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long Verbose              = 0;
        unsigned long Wrap                 = 0;
        const char*   XMLPath_CSTR         = "/tmp/inherit-context-test.xml";

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "inheritea-wrap", 'w', POPT_ARG_NONE, &Wrap, 0,
                  "wrap the xmlpath in an inherit ea context after resolve()", "" },
                
                { "xmlpath", 'x', POPT_ARG_STRING, &XMLPath_CSTR, 0,
                  "Where to start the inherit ea test", "/tmp/inherit-context-test.xml" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]*  ...");

        /* Now do options processing */
        char c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}

        if (argc < 2 )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        fh_context xml = Resolve( XMLPath_CSTR );
        if( Wrap )
        {
            xml = Factory::makeInheritingEAContext( xml );
        }
        runtest( xml );
    }
    catch( exception& e )
    {
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}
