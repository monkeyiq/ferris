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
#include <Ferris/FilteredContext.hh>
#include <Ferris/CacheManager_private.hh>

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
using namespace Ferris::Private;
using namespace STLdb4;

const string PROGRAM_NAME = "ut_memory_manager";

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


    
// #include <execinfo.h>
// #include <cxxabi.h>
// void BackTrace2( int fd = 2 )
//     {
//         const int arraysz = 500;
//         void* array[arraysz];
//         size_t size;

//         size = backtrace( array, arraysz );
//         write( fd, "raw symbol backtrace...\n",
//                strlen("raw symbol backtrace...\n") );
//         backtrace_symbols_fd( array, size, fd );

//         write( fd, "\n\ndemangled symbol backtrace...\n",
//                strlen("\n\ndemangled symbol backtrace...\n") );
//         if( char** symbarray = backtrace_symbols( array, size ) )
//         {
//             size_t outsz = 4096;
//             char* out = (char*)malloc( outsz+1 );
//             for( int i=0; i < size; ++i )
//             {
// //                cerr << "sym:" << symbarray[i] << endl;
//                 string s = symbarray[i];
//                 string mangled = s.substr( 0, s.find(' ') );
//                 mangled = mangled.substr( mangled.find('(')+1 );
//                 mangled = mangled.substr( 0, mangled.rfind(')') );
//                 mangled = mangled.substr( 0, mangled.rfind('+') );
// //                cerr << "mangled:" << mangled << endl;
                
//                 int status = 0;
//                 char* unmangled = __cxxabiv1::__cxa_demangle( mangled.c_str(), out, &outsz, &status );
// //                char* unmangled = __cxxabiv1::__cxa_demangle( mangled.c_str(), 0, 0, &status );
//                 if( !status )
//                 {
//                     out = unmangled;
// //                    cerr << unmangled << endl;
//                     write( fd, out, strlen(out) );
//                     write( fd, "\n", 1 );
//                 }
//                 else
//                 {
// //                    cerr << "status:" << status << endl;
//                     write( fd, mangled.c_str(), mangled.length() );
//                     write( fd, "\n", 1 );
//                 }
//             }
//             free( out );
//             free( symbarray );
//         }
        
//     }


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

const char*   FerrisFilter_CSTR    = 0;
unsigned long CallCleanupNTimes = 1;
unsigned long useIsDirTryOverMounting = 0;
string isDirEA = "is-dir";

string BaseDir = "/tmp";

typedef Private::CacheManagerContextStateInTimeList_t pitlist_t;
typedef Private::CacheManagerContextStateInTimeIndexSet_t pitset_t;


class debug_errorFor_EntriesOnlyInSecond
{
    bool m_ignoreContextsWithNoParent;
    
public:
    debug_errorFor_EntriesOnlyInSecond()
        :
        m_ignoreContextsWithNoParent( true )
        {
        }
    
    void operator()( pitlist_t& alist, pitlist_t& blist );
};



void
debug_errorFor_EntriesOnlyInSecond::operator()( pitlist_t& alist, pitlist_t& blist )
{
    pitset_t a = toIndexSet( alist );
    pitset_t b = toIndexSet( blist );
    pitset_t d;
    
    set_difference( b.begin(), b.end(), a.begin(), a.end(), inserter( d, d.end() ) );
    if( d.size() )
    {
        {
            string homedir = Shell::getHomeDirPath_nochecks();
            string ferrisdir = Shell::getHomeDirPath_nochecks() + "/.ferris";
            pitset_t::iterator diter = d.begin();
            pitset_t::iterator    de = d.end();
            for( ; diter != de; )
            {
                pitset_t::iterator cur = diter;
                cerr << "checking..." << cur->path << "..." << endl;
                ++diter;

                if( cur->path == "/" || cur->path == "/home" || cur->path == homedir )
                {
                    cerr << "1 remove..." << cur->path << "..." << endl;
                    d.erase( cur );
                }
                else if( starts_with( cur->path, ferrisdir ) )
                {
                    cerr << "2 remove..." << cur->path << "..." << endl;
                    d.erase( cur );
                }
                else if( m_ignoreContextsWithNoParent && !cur->parent )
                {
                    cerr << "3. no parent remove..." << cur->path << "..." << endl;
                    d.erase( cur );
                }
            }
        }
        
        if( d.size() )
        {
            E() << "debug_errorFor_EntriesOnlyInSecond() have invalid contexts... diff.sz:" << d.size() << endl;
            pitset_t::iterator diter = d.begin();
            pitset_t::iterator    de = d.end();
        
            for( ; diter != de; ++diter )
            {
                E() << diter->str() << endl;
            }
        }
    }

    cerr << "ok. debug_errorFor_EntriesOnlyInSecond() second list contains <= first list" << endl;
}

fh_context wrap( fh_context c )
{
    if( FerrisFilter_CSTR )
    {
        cerr << "Adding filter wrapper:" << FerrisFilter_CSTR << endl;
        c = Factory::MakeFilteredContext( c, FerrisFilter_CSTR );
    }

    return c;
}

void
runtest_SimpleClean()
{
    cerr << "Simple clean" << endl;

    Private::CacheManagerImpl* cm = Private::getCacheManagerImpl();
    pitlist_t startlist, endlist;

    fh_context c = Resolve( BaseDir );
    createMMCtxInTimeList( startlist );
    wrap( c );
    
    for( int i=0; i < CallCleanupNTimes; ++i )
        cm->cleanUp();
    createMMCtxInTimeList( endlist );

    dumpTo( Factory::fcerr(), startlist, "Start list" );
    dumpTo( Factory::fcerr(),   endlist, "End   list" );
    debug_errorFor_EntriesOnlyInSecond debugop;
    debugop( startlist, endlist );
}

typedef boost::function< void (fh_context ) > f_SingleContextFunctor;

void SimpleReadFunc( fh_context c )
{
    c->read();
}

void IteratorReadFunc( fh_context c )
{
    Context::iterator ci = c->begin();
    Context::iterator ce = c->end();
    for( ; ci != ce; ++ci )
    {
    }
}

void RecFunc( fh_context c )
{
    Context::iterator ci = c->begin();
    Context::iterator ce = c->end();
    for( ; ci != ce; ++ci )
    {
        if( isTrue( getStrAttr( *ci, isDirEA, "0" ) ) )
            RecFunc( *ci );
    }
}



// cp alice13a.txt boysw10.txt dmoro11.txt nobos10.txt snark12.txt warw11.txt /tmp/guten/
void
runtest_SingleContextActionAndClean( f_SingleContextFunctor f )
{
    cerr << "Simple action and clean for basedir:" << BaseDir << endl;
//    BackTrace();
    
    Private::CacheManagerImpl* cm = Private::getCacheManagerImpl();
    pitlist_t startlist;
    pitlist_t midlist;
    pitlist_t endlist;
    
    fh_context c = Resolve( BaseDir );
    cerr << "Simple action and clean for basedir c:" << toVoid(GetImpl(c)) << endl;
//    addContextReferenceWatch( GetImpl(c) );
//    addContextParentReferenceWatch( GetImpl(c) );
//    addContextReferenceWatchByName( "child2" );

    createMMCtxInTimeList( startlist );
    dumpTo( Factory::fcerr(), startlist, "--- Starting list" );
    
    {
        fh_context tc = wrap( c );
        f( tc );
        createMMCtxInTimeList( midlist );
        
        {
            pitlist_t tmp;
            createMMCtxInTimeList( tmp );
            dumpTo( Factory::fcerr(), tmp, "--- Holding wrapped TC" );
        }
    }


        {
            pitlist_t tmp;
            createMMCtxInTimeList( tmp );
            dumpTo( Factory::fcerr(), tmp, "--- Starting cleanups..." );
        }
    
    for( int i=0; i < CallCleanupNTimes; ++i )
    {
        cm->cleanUp();
        
        {
            stringstream ss;
            ss << "--- After cleanup number:" << i << " ... " << endl;
            pitlist_t tmp;
            createMMCtxInTimeList( tmp );
            dumpTo( Factory::fcerr(), tmp, ss.str() );
        }
    }
    createMMCtxInTimeList( endlist );

//     dumpTo( Factory::fcerr(), midlist, "Middle (full) list" );
    dumpTo( Factory::fcerr(), startlist, "Start list" );
    dumpTo( Factory::fcerr(),   endlist, "End   list" );

    cerr << "startlist.sz:" << startlist.size() << endl;
    cerr << "endlist.sz:" << endlist.size() << endl;
    debug_errorFor_EntriesOnlyInSecond debugop;
    debugop( startlist, endlist );
    cerr << "runtest_SingleContextActionAndClean(done)" << endl;
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
        unsigned long SimpleClean          = 0;
        unsigned long SimpleReadAndClean   = 0;
        unsigned long SimpleIteratorSweepAndClean    = 0;
        unsigned long SimpleRecReadAndClean          = 0;
        unsigned long NTimes               = 1;
        const char*   BaseDir_CSTR         = "/tmp";
        unsigned long FerrisFilterClean    = 0;

        
        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "basedir", 0, POPT_ARG_STRING, &BaseDir_CSTR, 0,
                  "where the test files are located ", "/tmp" },

                { "ntimes", 'n', POPT_ARG_INT, &NTimes, 0,
                  "run test ntimes in a row.", "1" },

                { "clean-ntimes", 'c', POPT_ARG_INT, &CallCleanupNTimes, 0,
                  "call cleanup 'c' times after each run.", "1" },
                

                { "ferris-filter", 0, POPT_ARG_STRING, &FerrisFilter_CSTR, 0,
                  "wrap all contexts with this filter... ", "" },

                { "use-is-dir-try-overmounting", '1', POPT_ARG_NONE, &useIsDirTryOverMounting, 0,
                  ".... ", "" },



                
                { "simple-clean", 0, POPT_ARG_NONE, &SimpleClean, 0,
                  "do nothing and cleanup memory", "" },

                { "simple-read-and-clean", 0, POPT_ARG_NONE, &SimpleReadAndClean, 0,
                  "read basedir and cleanup memory", "" },

                { "simple-iterator-sweep-and-clean", 0, POPT_ARG_NONE, &SimpleIteratorSweepAndClean, 0,
                  "read basedir and cleanup memory", "" },

                { "simple-recursive-read-and-clean", 0, POPT_ARG_NONE, &SimpleRecReadAndClean, 0,
                  "read basedir and cleanup memory", "" },


                { "ferris-filter-clean", 0, POPT_ARG_NONE, &FerrisFilterClean, 0,
                  "mount the --ferris-filter string and cleaup", "" },
                

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

        if( useIsDirTryOverMounting )
            isDirEA = "is-dir-try-automounting";

        if( FerrisFilterClean )
        {
            if( !FerrisFilter_CSTR )
            {
                E() << "Must supply --ferris-filter string" << endl;
                exit(1);
            }
            
            Private::CacheManagerImpl* cm = Private::getCacheManagerImpl();
            pitlist_t startlist, endlist;

            fh_context c = Resolve( "/" );
            createMMCtxInTimeList( startlist );
            
            {
                fh_context filter = Factory::MakeFilter( FerrisFilter_CSTR );
                
                {
                    pitlist_t tmplist;
                    createMMCtxInTimeList( tmplist );
                    dumpTo( Factory::fcerr(), tmplist, "holding ffilter1" );
                }
            }

            
            for( int i=0; i < CallCleanupNTimes; ++i )
                cm->cleanUp();
                
            createMMCtxInTimeList( endlist );
            dumpTo( Factory::fcerr(), startlist, "Start list" );
            dumpTo( Factory::fcerr(),   endlist, "End   list" );
            debug_errorFor_EntriesOnlyInSecond debugop;
            debugop( startlist, endlist );
        }
        else
        {
            for( int i = 0; i < NTimes; ++i )
            {
                cerr << "--- Running iteration:" << i << endl;
            
                if( SimpleClean )
                {
                    runtest_SimpleClean();
                }
                else if( SimpleReadAndClean )
                {
                    runtest_SingleContextActionAndClean( SimpleReadFunc );
                }
                else if( SimpleIteratorSweepAndClean )
                {
                    runtest_SingleContextActionAndClean( IteratorReadFunc );
                }
                else if( SimpleRecReadAndClean )
                {
                    runtest_SingleContextActionAndClean( RecFunc );
                }
            }
        }
        
    }
    catch( exception& e )
    {
        E() << "cought error:" << e.what() << endl;
        exit(1);
    }
    if( !errors )
        cerr << "Success" << endl;
    else
        cerr << "error: error count != 0" << endl;
    return exit_status;
}
