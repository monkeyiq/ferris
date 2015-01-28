/******************************************************************************
*******************************************************************************
*******************************************************************************

    ftee
    Copyright (C) 2009 Ben Martin

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

    $Id: fcat.cpp,v 1.10 2009/04/14 21:30:11 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
 * return 5 for not modified
 * 
 *
 *
*/


#include <Ferris.hh>
#include <FerrisOpenSSL.hh>
#include <FerrisDOM.hh>
#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ftee";

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

int main( int argc, char** argv )
{
    int exit_status = 0;
    unsigned long FerrisInternalAsyncMessageSlave      = 0;
    const char*   FerrisInternalAsyncMessageSlaveAttrs = 0;
    const char*   SourceAttrName         = "content";

    try
    {
        unsigned long Verbose              = 0;
        unsigned long Append               = 0;
        unsigned long DigestMD5            = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "append", 'a', POPT_ARG_NONE, &Append, 0,
                  "append to given files", "" },

                { "md5", '5', POPT_ARG_NONE, &DigestMD5, 0,
                  "show md5 on stderr at end", "" },
                
                
                // { "src-attr", 'a', POPT_ARG_STRING,
                //   &SourceAttrName, 0,
                //   "cat an EA rather than the content itself", "" },

                // { "ea", 0, POPT_ARG_STRING,
                //   &SourceAttrName, 0,
                //   "cat an EA rather than the content itself", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* dst");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {
//         switch (c) {
//         }
        }

        if (argc < 2)
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }


        stringlist_t dsts;
        dsts = expandShellGlobs( dsts, optCon );

        if( dsts.size() != 1 )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }


        // // digest
        // {
        //     fh_istream iss  = Factory::fcin();
        //     fh_iostream z = Factory::MakeDigestStream( "md5" );
        //     cerr << "copying..." << endl;
        //     copy( istreambuf_iterator<char>(iss),
        //           istreambuf_iterator<char>(),
        //           ostreambuf_iterator<char>(z));
        //     cerr << "2........" << endl;
        //     z << flush;
        //     cerr << "3........" << endl;
        //     {
        //         string s;
        //         cerr << "4........" << endl;
        //         z >> s;
        //         cerr << "digest:" << s << endl;
        //     }
        //     exit(0);
        // }
        
        
        string earl = *(dsts.begin());
        fh_istream iss  = Factory::fcin();
        fh_ostream oss1 = Factory::fcout();
        fh_context outc = Shell::acquireContext( earl, 0, false );
        ferris_ios::openmode m = std::ios::out;
        if( Append )
        {
            m |= ios::app | ios::ate;
        }
        fh_ostream oss2 = outc->getIOStream( m );

        fh_ostream oss = Factory::MakeTeeStream( oss1, oss2 );

        fh_iostream digestStream;
        if( DigestMD5 )
        {
            digestStream = Factory::MakeDigestStream( "md5" );
            oss = Factory::MakeTeeStream( oss1, digestStream );
        }
        

        copy( istreambuf_iterator<char>(iss),
              istreambuf_iterator<char>(),
              ostreambuf_iterator<char>(oss));
        oss  << flush;
        oss1 << flush;
        oss2 << flush;
        digestStream << flush;

        if( DigestMD5 )
        {
            string s;
            digestStream >> s;
            cerr << "digest:" << s << endl;
        }
        
        
        string emsg = "";
        if( !iss.good() )
        {
            emsg = getIOErrorDescription( iss, "<STDIN>" );
        }
        else if( haveIOError( oss ) )
        {
            emsg = getIOErrorDescription( oss, "<STDOUT>" );
        }

        if( !emsg.empty() )
        {
            cerr << emsg << endl;
            exit_status = 1;
        }
    }
    catch( exception& e )
    {
        string emsg = e.what();
        cerr << "error:" << emsg << endl;
        exit_status = 1;
    }
    return exit_status;
}


