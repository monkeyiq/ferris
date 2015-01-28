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

    $Id: ferris-internal-file-to-fifo-command.cpp,v 1.7 2011/11/10 21:30:22 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
 * 
*/


#include <Ferris.hh>
#include <FerrisFileActions.hh>
#include <popt.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ferris-redirect-to-fifo";

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
//unsigned long Truncate             = 0;
unsigned long Writable             = 0;
const char* EAName_CSTR = 0;


fh_ostream getOStream( fh_context c )
{
    ferris_ios::openmode om = std::ios::out | std::ios::trunc | ferris_ios::o_mseq;
    
    if( EAName_CSTR )
    {
        string eaname = EAName_CSTR;
        setStrAttr( c, eaname, "x", true );
        fh_attribute a = c->getAttribute( eaname );
        return a->getIOStream( om );
    }
    else
    {
        return c->getIOStream( om );
    }
    return Factory::fcerr();
}


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

//                 { "trunc", 'T', POPT_ARG_NONE, &Truncate, 0,
//                   "truncate output file", "" },

                { "writable", 'w', POPT_ARG_NONE, &Writable, 0,
                  "after reading file allow client to send updated contents.", "" },
                
                { "ea", 0, POPT_ARG_STRING, &EAName_CSTR, 0,
                  "put redirection into this EA", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* src1 src2 ...");

        /* Now do options processing */
        {
            int c=-1;
            while ((c = poptGetNextOpt(optCon)) >= 0)
            {}
        }
        if (argc < 1)
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        
        char templatestr[500] = "/tmp/libferris-fifo-redirect-tmp-XXXXXX";
        {
            int fd = mkstemp( templatestr );
            if( fd == -1 )
            {
                cerr << errnum_to_string( "Failed to create a temporary file:", errno );
                return 0;
            }
            close(fd);
        }
        

        mode_t mode = S_IRUSR | S_IWUSR;
        unlink( templatestr );
        int rc = mkfifo( templatestr, mode );
        if( rc == -1 )
        {
            cerr << errnum_to_string( "Failed to create a fifo:", errno );
            return 0;
        }
        
        cout << templatestr << flush;
        SwitchToDaemonMode();
        


        ctxlist_t selection;
        stringlist_t srcs;
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            string srcURL = tmpCSTR;
            fh_context c = Shell::acquireContext( srcURL, 0, false );
            selection.push_back( c );
        }

        if( selection.empty() )
        {
            cerr << "please supply a URL to acquire data from." << endl;
            exit(1);
        }
        
        fh_context c = selection.front();

        ferris_ios::openmode om = std::ios::in | ferris_ios::o_mseq;
//         if( Truncate )
//         {
//             om |= std::ios::ate;
//         }

        fh_istream iss = Factory::fcin();
        if( EAName_CSTR )
        {
            string eaname = EAName_CSTR;
            cerr << "Information using getstrattr():" << getStrAttr( c, eaname, "<nothing>" ) << endl;
            try
            {
                fh_attribute a = c->getAttribute( eaname );
                iss = a->getIStream();
            }
            catch( NoSuchAttribute& e )
            {
                cerr << "no attribute" << endl;
                fh_stringstream ss;
                iss = ss;
            }
            catch( exception& e )
            {
                cerr << "unexpected error:" << e.what() << endl;
            }
            
        }
        else
        {
//            cerr << "Reading from c:" << c->getURL() << endl;
            iss = c->getIStream( om );
        }


        
        {
            int fd = open( templatestr, O_WRONLY );
            bool closeFD = true;
            fh_ostream oss = Factory::MakeFdOStream( fd, closeFD );
            copy( istreambuf_iterator<char>( iss ),
                  istreambuf_iterator<char>(),
                  ostreambuf_iterator<char>( oss ) );
            oss << flush;
            close(fd);
        }
        
        if( Writable )
        {
            int fd = open( templatestr, O_RDONLY );
            bool closeFD = true;
            fh_istream iss = Factory::MakeFdIStream( fd, closeFD );
            fh_ostream oss = getOStream( c );
            copy( istreambuf_iterator<char>( iss ),
                  istreambuf_iterator<char>(),
                  ostreambuf_iterator<char>( oss ) );
            oss << flush;
            
        }
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


