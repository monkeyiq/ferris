/******************************************************************************
*******************************************************************************
*******************************************************************************

    fhead
    Copyright (C) 2005 Ben Martin

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

    $Id: fhead.cpp,v 1.3 2010/09/24 21:31:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

/*
 * return 0 for success
 * return 1 for generic error
 * 
 *
 *
 *
 *
*/


#include <Ferris.hh>
#include <popt.h>
#include <unistd.h>

using namespace __gnu_cxx;

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "fhead";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

unsigned long Verbose              = 0;
unsigned long Quiet                = 0;
long FirstBytes           = 0;
long FirstLines           = 10;
//const char* SourceAttrName         = "content";


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "quiet", 'q', POPT_ARG_NONE, &Quiet, 0,
                  "never print headers giving file names", "" },
                { "silent", 0, POPT_ARG_NONE, &Quiet, 0,
                  "never print headers giving file names", "" },

                { "bytes", 'c', POPT_ARG_INT, &FirstBytes, 0,
                  "print the first N bytes of each file. With leading '-' print all but last N bytes.", "" },

                { "lines", 'n', POPT_ARG_INT, &FirstLines, 0,
                  "print the first N lines of each file. With leading '-' print all but last N lines.", "" },
                
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
        {
//         switch (c) {
//         }
        }

        if (argc < 1)
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
        stringlist_t srcs;
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            string srcURL = tmpCSTR;
            srcs.push_back( srcURL );
        }
        if( srcs.empty() )
        {
            srcs.push_back("-");
        }

        int srcs_sz = srcs.size();
        fh_ostream oss = Factory::MakeFdOStream( STDOUT_FILENO );
        for( stringlist_t::const_iterator si = srcs.begin(); si != srcs.end(); ++si )
        {
            string earl = *si;

            if( srcs_sz > 1 && !Quiet )
            {
                oss << "==> " << earl << " <==" << endl;
            }
            
//            streamsize filesz = -1;
            fh_istream iss;
            if( earl == "-" )
            {
                iss = Factory::MakeFdIStream( STDIN_FILENO );
            }
            else
            {
                fh_context c = Resolve( earl );
                iss = c->getIStream();
//                streamsize filesz = toType<streamsize>(getStrAttr( c, "size", "", true, true ));
            }

            if( FirstBytes != 0 )
            {
                char ch = 0;
                
                if( FirstBytes > 0 )
                {
//                     copy_n( istreambuf_iterator<char>(iss), min( filesz, FirstBytes ),
//                             ostreambuf_iterator<char>(oss));

                    for( int i = 0; i < FirstBytes; ++i )
                    {
                        if( !(iss >> noskipws >> ch) )  break;
                        if( !(oss << ch) )  exit( 1 );
                    }
                    oss << flush;
                }
                else
                {
//                     fh_istream limitedss = Factory::MakeLimitingIStream( iss, 0, filesz + FirstBytes );
//                     copy( istreambuf_iterator<char>(limitedss), istreambuf_iterator<char>(),
//                           ostreambuf_iterator<char>(oss));
//                     oss << flush;

                    typedef list< char > buffer_t;
                    buffer_t buffer;
                    for( int i = 0; i < (-1*FirstBytes); ++i )
                    {
                        if( !(iss >> noskipws >> ch) )
                            break;

                        buffer.push_back( ch );
                    }
                    if( buffer.size() < (-1*FirstBytes) )
                        continue;
                    
                    while( iss >> noskipws >> ch )
                    {
                        buffer.push_back( ch );
                        ch = buffer.front();
                        buffer.pop_front();
                        oss << ch;
                    }
                    oss << flush;
                    
                }
            }
            else
            {
                string s;
                if( !FirstLines )
                {
                    poptPrintHelp(optCon, stderr, 0);
                    exit(1);
                }
                if( FirstLines > 0 )
                {
                    for( int i = 0; i < FirstLines; ++i )
                    {
                        if( !getline( iss, s ) )
                        {
                            oss << flush;
                            continue;
                        }
                        
                        oss << s << nl;
                    }
                    oss << flush;
                }
                else
                {
                    stringlist_t buffer;
                    for( int i = 0; i < (-1*FirstLines); ++i )
                    {
                        getline( iss, s );
                        buffer.push_back( s );
                    }
                    while( getline( iss, s ) )
                    {
                        buffer.push_back( s );
                        s = buffer.front();
                        buffer.pop_front();
                        oss << s << endl;
                    }
                    oss << flush;
                }
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


