/******************************************************************************
*******************************************************************************
*******************************************************************************

    unit test code for many-to-one subclasses such as union:// context type
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

    $Id: ut_manybasetoone_viewcontext.cpp,v 1.1 2006/12/10 04:52:17 ben Exp $

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

const string PROGRAM_NAME = "ut_manybasetooneviewcontext";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long BigFilteredTest      = 0;
        unsigned long DisjointTest         = 0;
        unsigned long ConflictTest         = 0;
        unsigned long Conflict_3_Test      = 0;
        unsigned long UseUnion             = 0;
        unsigned long UseDifference        = 0;
        unsigned long UseIntersection      = 0;
        unsigned long UseSymDiff           = 0;
        unsigned long UseResolve           = 0;
        unsigned long Verbose              = 0;
        unsigned long SepWithComma         = 0;
        unsigned long ColonThenContent     = 0;
        unsigned long Reverse              = 0;
        unsigned long Dummy                = 0;
        unsigned long UseBigSetData        = 0;
        unsigned long UseBigSetData3       = 0;
        unsigned long DigestResult         = 0;
        const char*   BaseDir_CSTR              = "/tmp";

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "dummy", 0, POPT_ARG_NONE, &Dummy, 0,
                  "impotent option", "" },
                
                { "basedir", 0, POPT_ARG_STRING, &BaseDir_CSTR, 0,
                  "where the test files are located ", "/tmp" },

                { "big-filtered-test", 0, POPT_ARG_NONE, &BigFilteredTest, 0,
                  "Create 3 filtered views of the same directory and use them as input"
                  " Environment is assumed to be setup already from many2onefs.tar.gz at --basedir", "" },
                
                { "disjoint-test", 0, POPT_ARG_NONE, &DisjointTest, 0,
                  "run test using two dirs that have no overlapping data."
                  " Environment is assumed to be setup already from many2onefs.tar.gz at --basedir", "" },

                { "conflict-test", 0, POPT_ARG_NONE, &ConflictTest, 0,
                  "run test using two dirs that have overlapping data."
                  " Environment is assumed to be setup already from many2onefs.tar.gz at --basedir", "" },

                { "conflict-3-test", 0, POPT_ARG_NONE, &Conflict_3_Test, 0,
                  "run test using three dirs that have overlapping data."
                  " Environment is assumed to be setup already from many2onefs.tar.gz at --basedir", "" },
                
                { "use-resolve", 'R', POPT_ARG_NONE, &UseResolve, 0,
                  "use Resolve() to create the context from a string", "" },

                { "use-union", 0, POPT_ARG_NONE, &UseUnion, 0,
                  "test union filesystem. Use --disjoint-test et al with this for a real test", "" },

                { "use-difference", 0, POPT_ARG_NONE, &UseDifference, 0,
                  "test difference filesystem.", "" },

                { "use-intersection", 0, POPT_ARG_NONE, &UseIntersection, 0,
                  "test intersection filesystem.", "" },

                { "use-sym-diff", 0, POPT_ARG_NONE, &UseSymDiff, 0,
                  "test symmetric difference filesystem.", "" },
                
                { "sep-with-comma", 'C', POPT_ARG_NONE, &SepWithComma, 0,
                  "Seperate context walk with commas not newlines", "" },

                { "digest", 'D', POPT_ARG_NONE, &DigestResult, 0,
                  "apply md5 digest to -c -C data result and print that instead of result."
                  " handy for use with --use-bigset and test suites", "" },
                
                { "colon-then-content", 'c', POPT_ARG_NONE, &ColonThenContent, 0,
                  "Show the rdn:content instead of just the rdn", "" },

                { "reverse", 'r', POPT_ARG_NONE, &Reverse, 0,
                  "Reverse the order of the base contexts", "" },

                { "use-bigset", 0, POPT_ARG_NONE, &UseBigSetData, 0,
                  "Assume the data from many2onefs-bigset.tar is at basedir", "" },

                { "use-bigset-3", 0, POPT_ARG_NONE, &UseBigSetData3, 0,
                  "Assume the data from many2onefs-bigset.tar is at basedir and use 3 dirs in test", "" },

                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* src1 src2 ...");

        /* Now do options processing */
        char c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}

        if (argc < 2 )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
        string BaseDir = BaseDir_CSTR;
        if( UseBigSetData3 )
        {
            UseBigSetData = 1;
            Conflict_3_Test = 1;
        }
        


        string opcode = "union://";
        fh_context parent = Resolve( BaseDir );
        fh_context agg    = 0;
        string path1 = "";
        string path2 = "";
        string path3 = "";

        if( UseUnion )             opcode = "union://";
        else if( UseDifference )   opcode = "setdifference://";
        else if( UseIntersection ) opcode = "setintersection://";
        else if( UseSymDiff )      opcode = "setsymdifference://";

        if( BigFilteredTest )
        {
            path1 = "filter:(name=~file-2.*)/file://" + BaseDir + "/dirC/";
            path2 = "filter:(name=~file-3.*)/file://" + BaseDir + "/dirC/";
            path3 = "filter:(name=~file-4.*)/file://" + BaseDir + "/dirC/";
        }
        else if( DisjointTest )
        {
            path1 = "file://" + BaseDir + "/disjoint1/";
            path2 = "file://" + BaseDir + "/disjoint2/";
        }
        else if( ConflictTest || Conflict_3_Test )
        {
            path1 = "file://" + BaseDir + "/disjoint1/";
            path2 = "file://" + BaseDir + "/disjoint1-conflict1/";
            if( Conflict_3_Test )
                path3 = "file://" + BaseDir + "/disjoint1-conflict2/";
        }

        if( UseBigSetData )
        {
            path1 = "file://" + BaseDir + "/dirA/";
            path2 = "file://" + BaseDir + "/dirB/";
            if( Conflict_3_Test )
                path3 = "file://" + BaseDir + "/dirC/";
        }
        
            

        if( Reverse )
        {
            if( Conflict_3_Test ) swap( path3, path1 );
            else                  swap( path1, path2 );
        }
        
        
            
        if( UseResolve )
        {
            fh_stringstream ss;
            ss << opcode << path1 << path2 << path3;
            cerr << "url:" << tostr(ss) << endl;
            agg = Resolve( tostr(ss) );
        }
        else
        {
            fh_context c1 = Resolve( path1 );
            fh_context c2 = Resolve( path2 );

            std::list< fh_context > unionContexts;
            unionContexts.push_back( c1 );
            unionContexts.push_back( c2 );
            if( Conflict_3_Test )
                unionContexts.push_back( Resolve( path3 ) );

            if( UseDifference )
                agg = Factory::MakeSetDifferenceContext( parent, unionContexts );
            else if( UseIntersection )
                agg = Factory::MakeSetIntersectionContext( parent, unionContexts );
            else if( UseSymDiff )
                agg = Factory::MakeSetSymmetricDifferenceContext( parent, unionContexts );
            else if( UseUnion || !agg )
                agg = Factory::MakeUnionContext( parent, unionContexts );
        }


        if( agg )
        {
            fh_stringstream digestss;
            fh_ostream outss = Factory::fcout();
            
            cout << "displaying agg:" << agg->getURL() << endl;

            if( DigestResult )
            {
                outss = digestss;
            }

            for( Context::iterator ci = agg->begin(); ci!=agg->end(); ++ci )
            {
                outss << (*ci)->getDirName();
                if( ColonThenContent )
                {
                    outss << ":" << getStrAttr( *ci, "content", "" );
                }
                
                if( SepWithComma ) outss << ",";
                else               outss << endl;
            }
            if( DigestResult )
            {
                cerr << "disgesting:" << tostr(digestss) << endl;
                InitOpenSSL();
                cout << digest( digestss ) << endl;
            }
            outss << endl;
            if( agg->begin() == agg->end() )
                cout << "NOTHING" << endl;
        }
    }
    catch( exception& e )
    {
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}
