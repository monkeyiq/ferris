/******************************************************************************
*******************************************************************************
*******************************************************************************

    unit test code for doing Formal Concept Analysis with filesystems
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

    $Id: ut_fca.cpp,v 1.1 2006/12/10 04:52:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris.hh>
#include <FullTextIndexer.hh>
#include <FullTextIndexer_private.hh>
#include <Ferris/Iterator.hh>
#include <Ferris/FerrisOpenSSL.hh>
#include <popt.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <map>
#include <list>
#include <iterator>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ut_fca";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

string BaseDir = "/tmp";

/**
 * Very simple test to create a filtered view on a directory using a name based
 * formal scale and create just the first and second level concepts.
 */
void testUnions()
{
    typedef list< fh_context > cl_t;
    typedef cl_t::iterator I;
    cl_t cl;
    cl_t cascade;
    fh_context parent = Resolve( BaseDir + "/dirC/" );
    cl.push_back( Resolve( "filter:(name=~file-2.*)/file://"  + BaseDir + "/dirC/" ));
    cl.push_back( Resolve( "filter:(name=~file-31.*)/file://" + BaseDir + "/dirC/" ));
    cl.push_back( Resolve( "filter:(name=~file-32.*)/file://" + BaseDir + "/dirC/" ));
    cl.push_back( Resolve( "filter:(name=~file-4.*)/file://"  + BaseDir + "/dirC/" ));

    I last = cl.begin();
    I ci   = last;
    for( ++ci; ci != cl.end(); ++ci, ++last )
    {
        std::list< fh_context > unionContexts;
        unionContexts.push_back( *last );
        unionContexts.push_back( *ci );
        cascade.push_back( Factory::MakeUnionContext( parent, unionContexts ) );
    }

    for( I iter = cl.begin(); iter != cl.end(); ++iter )
    {
        cerr << "1 url:" << (*iter)->getURL() << " size:" << (*iter)->SubContextCount() << endl;
        for( Context::iterator ci = (*iter)->begin(); ci != (*iter)->end(); ++ci )
            cerr << "1...:" << (*ci)->getURL() << endl;
    }
    
    for( I iter = cascade.begin(); iter != cascade.end(); ++iter )
    {
        cerr << "2 url:" << (*iter)->getURL() << " size:" << (*iter)->SubContextCount() << endl;
        for( Context::iterator ci = (*iter)->begin(); ci != (*iter)->end(); ++ci )
            cerr << "2...:" << (*ci)->getURL() << endl;
    }
    
    
    
}

int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long Verbose              = 0;
        unsigned long TestUnions           = 0;
        unsigned long Dummy                = 0;
        const char*   BaseDir_CSTR         = "/tmp";

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "dummy", 0, POPT_ARG_NONE, &Dummy, 0,
                  "impotent option", "" },
                
                { "basedir", 0, POPT_ARG_STRING, &BaseDir_CSTR, 0,
                  "where the test files are located ", "/tmp" },

                { "test-unions", 0, POPT_ARG_NONE, &TestUnions, 0,
                  "Test cascaded unions of extent sizes"
                  " Assumes the data from many2onefs-bigset.tar is at basedir", "" },
                
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
        BaseDir = BaseDir_CSTR;

        if( TestUnions ) testUnions();
        
    }
    catch( exception& e )
    {
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}
