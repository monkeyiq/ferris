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

    $Id: ut_cache.cpp,v 1.1 2006/12/10 04:52:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris/Ferris.hh>
#include <Ferris/Cache.hh>

#include <popt.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <map>
#include <list>
#include <iterator>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ut_cache";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

int errors = 0;

fh_ostream& E()
{
    ++errors;
    static fh_ostream ret = Factory::fcerr();
    ret << "error:";
    return ret;
}

void
assertcompare( const std::string& emsg,
               const std::string& expected,
               const std::string& actual )
{
    if( expected != actual )
        E() << emsg << endl
            << " expected:" << expected << ":" 
            << " actual:" << actual << ":" << endl;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

string BaseDir = "/tmp";

class Animal : public CacheHandlable
{
public:
    string name;
    Animal( string name )
        :
        name( name )
        {
        }
};
FERRIS_SMARTPTR( Animal, fh_animal );    

GMainContext* gmc;
GMainLoop* gml;
static gint qf(gpointer data)
{
    g_main_loop_quit( gml );
}


void runtest_cache()
{
    Cache<string,fh_animal> c;
    c.setTimerInterval( 1000 );
    {
        fh_animal dog  = new Animal( "dog" );
        fh_animal cat  = new Animal( "cat" );
        fh_animal fish = new Animal( "fish" );
    
        c.put( "dog",  dog );
        c.put( "cat",  cat );
        c.put( "fish", fish );
    }

    fh_animal ca = c.get("dog");
    if( !ca )
        E() << "couldn't get the dog back from the cache" << endl;
    cerr << "got cache animal:" << ca->name << endl;

    gmc = g_main_context_default();
    gml = g_main_loop_new( gmc, 0 );
    g_timeout_add( 3000, GSourceFunc(qf), 0 );
    g_main_loop_run( gml );
    g_main_destroy( gml );
    
    fh_animal fa = c.get("fish");
    if( fa )
        E() << "The fish object in the cache should have been reclaimed."
            << " it wasn't and so the caching is broken." << endl;
    if( !fa )
        cerr << "(expected) couldn't get the fish from the cache!" << endl;


    fh_animal t = c.get("dog");
    if( !t )
        E() << "We kept a reference to the dog when the timer cleaner was invoked"
            << " the cleaner went and reclaimed the dog when it was in use. BAD" << endl;
    if( t )
        cerr << "the dog was not reclaimed by timer because we had a reference." << endl;
}

void
runtest_ManyTimeReference()
{
    Cache<string,fh_animal> c;
    c.setTimerInterval( 1000 );
    
    {
        fh_animal dog = new Animal( "dog" );
        c.put( "dog",  dog );
    }

    
    for( int i=0; i < 5; ++i )
    {
        fh_animal o = c.get( "dog" );
    }

    int collectableSize = c.getCollectableSize();
    cerr << "Number of items in cache at end:" << collectableSize << endl;
    if( collectableSize != 1 )
        E() << " After repeatedly fetching and dropping the same reference"
            << " it should only be in the cache claimable list the once."
            << " Currently it appears:" << collectableSize << " times"
            << " distance( c.begin(), c.end() ):" << distance( c.begin(), c.end() )
            << endl;
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long Verbose              = 0;
        unsigned long ManyTimeReference    = 0;
        const char*   BaseDir_CSTR         = "/tmp";

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "basedir", 0, POPT_ARG_STRING, &BaseDir_CSTR, 0,
                  "where the test files are located ", "/tmp" },

                { "many-time-reference", 0, POPT_ARG_NONE, &ManyTimeReference, 0,
                  "add/drop a single reference many times", "" },

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
        BaseDir = BaseDir_CSTR;

        if( ManyTimeReference )
        {
            runtest_ManyTimeReference();
        }
        else
        {
            runtest_cache();
        }
    }
    catch( exception& e )
    {
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    if( !errors )
        cerr << "Success" << endl;
    else
        cerr << "error: error count != 0" << endl;
    return exit_status;
}
