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

    $Id: ut_config.cpp,v 1.1 2006/12/10 04:52:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris/Ferris.hh>
#include <Ferris/Configuration_private.hh>

#include <popt.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <map>
#include <list>
#include <iterator>

#include <STLdb4/stldb4.hh>

using namespace std;
using namespace Ferris;
using namespace STLdb4;

const string PROGRAM_NAME = "ut_config";

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
string FileName = "test.db";

void
runtest_SimpleSetGet()
{
    string key = "TheKey";
    string val = "TheValue";
    string filename = BaseDir + "/" + FileName;
    
    set_db4_string( filename, key, val );
    string actual = get_db4_string( filename, key, "no" );

    assertcompare( "Didn't get to read back the expected data", val, actual );
}


void
runtest_KeyNotFound()
{
    string key = "TheKeyWhichDoesntExist";
    string val = "TheValue";
    string def = "DefaultValue";
    string filename = BaseDir + "/" + FileName;
    string actual;

    actual = get_db4_string( filename, key, def, false );
    assertcompare( "Didn't get to read back the expected data", def, actual );
    cerr << "Read back actual:" << actual << " which should be def:" << def
         << " for k:" << key << endl;
    try
    {
        actual = get_db4_string( filename, key, def, true );
        cerr << "SHOULD HAVE THROWN Read back actual:" << actual << " which should be def:" << def
             << " for k:" << key << endl;
        E() << "Should have thrown an exception because key is not bound" << endl;
    }
    catch( exception& e )
    {
        cerr << "Rightfully got e:" << e.what() << endl;
    }
    
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
        unsigned long SimpleSetGet         = 0;
        unsigned long KeyNotFound          = 0;
        const char*   BaseDir_CSTR         = "/tmp";
        const char*   FileName_CSTR        = "test.db";
        const char*   ReadKey_CSTR         = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "basedir", 0, POPT_ARG_STRING, &BaseDir_CSTR, 0,
                  "where the test files are located ", "/tmp" },

                { "filename", 'f', POPT_ARG_STRING, &FileName_CSTR, 0,
                  "what is the name of the db file to play with in --basedir ", "test.db" },

                { "readkey", 0, POPT_ARG_STRING, &ReadKey_CSTR, 0,
                  "try to read the key in --filename", "" },

                
                { "simple-set-get", 0, POPT_ARG_NONE, &SimpleSetGet, 0,
                  "a set_db4_string followed by a get_db4_string", "" },

                { "key-not-found", 0, POPT_ARG_NONE, &KeyNotFound, 0,
                  "test get_db4_string() in both exception modes for a nonbound key", "" },
                
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
        BaseDir  = BaseDir_CSTR;
        FileName = FileName_CSTR;

        if( SimpleSetGet )
        {
            runtest_SimpleSetGet();
        }
        else if( KeyNotFound )
        {
            runtest_KeyNotFound();
        }
        else if( ReadKey_CSTR )
        {
            string filename = BaseDir + "/" + FileName;
            string k = ReadKey_CSTR;
            string actual;
            
            actual = get_db4_string( filename, k, "", true );

// //             fh_database db = new Database();
// //             db->open( filename.c_str(), "", DB_UNKNOWN, 0, 0 );

//             int rc = 0;

//             DB* m_db = 0;
//             rc = db_create( &m_db, 0, 0 );

//             cerr << "create rc:" << rc << endl;

//             rc = m_db->open( m_db,
//                              0,
//                              filename.c_str(),
//                              0,
//                              DB_UNKNOWN,
//                              0,
//                              0 );

//             cerr << "open rc:" << rc << endl;
            
            
// //             DBT key ( (void*)k.data(), k.length() );
// //             DBT data( 0, 0 );
//             DBT key, data;
//             bzero( &key,  sizeof(DBT));
//             bzero( &data, sizeof(DBT));
//             key.data = (void*)k.data();
//             key.size = k.length();
            
//             rc = m_db->get( m_db, 0, &key, &data, 0 );

//             cerr << " raw:" << toVoid( m_db ) << endl;
//             cerr << "rc:" << rc << endl;
//             cerr << "data.size:" << data.size << endl;
            
// //            db->get( k, actual );
            
            cerr << actual << endl;
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
