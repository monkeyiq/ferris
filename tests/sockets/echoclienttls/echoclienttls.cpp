/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris echo

    Copyright (C) 2001 Ben Martin

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

    $Id: echoclienttls.cpp,v 1.3 2008/05/25 21:30:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <string>
#include <list>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <unistd.h>

#include <Ferris.hh>
#include <ContextPopt.hh>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "echo";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}



void printstate( const fh_iostream& ss )
{
    if( !ss.good() )
    {
        cerr << " rdstate:" << ss.rdstate() << endl;
        cerr << " good state:" << ss.good() << endl;
        cerr << " eof  state:" << ss.eof() << endl;
        cerr << " fail state:" << ss.fail() << endl;
        cerr << " bad  state:" << ss.bad() << endl;
    }
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void printattr( const fh_context& c, string name )
{
    fh_attribute a = c->getAttribute(name);
    fh_istream ss;
    
    ss = a->getIStream();
    cout << name << ":";
    copy( istreambuf_iterator<char>(ss), istreambuf_iterator<char>(),
          ostreambuf_iterator<char>(cout));
    cout << endl;
}


/**
 * Dump out some certificate info
 */
void
printCertificateInfo( const fh_context& c )
{
    cout << "printCertificateInfo()" << endl;
    
    try 
    {
        printattr( c, "certificate-subject" );
        printattr( c, "certificate-issuer" );
    }
    catch( NoSuchAttribute& e )
    {
        cerr << "No such attribute for server certificate information e:"
             << e.what() << endl;
    }
    catch( CanNotGetStream& e )
    {
        cerr << "Can not get EA stream for server certificate information e:"
             << e.what() << endl;
    }
    catch( exception& e )
    {
        cerr << "Error reading certificate EA e:"
             << e.what() << endl;
    }
}

void
printEA( const fh_context& c )
{
    cout << "printEA()" << endl;

    try 
    {
        printattr( c, "cipher" );
        printattr( c, "cipher-bits" );
        printattr( c, "cipher-list" );
        printattr( c, "cipher-name" );
        printattr( c, "cipher-version" );
    }
    catch( NoSuchAttribute& e )
    {
        cerr << "No such attribute e:"
             << e.what() << endl;
    }
    catch( CanNotGetStream& e )
    {
        cerr << "Can not get EA stream for attribute e:"
             << e.what() << endl;
    }
    catch( exception& e )
    {
        cerr << "Error reading certificate EA e:"
             << e.what() << endl;
    }
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


int main( int argc, const char** argv )
{
    /* Parse --logging options, this should be done early */
    ParseOnly_FERRIS_POPT_OPTIONS( PROGRAM_NAME, argc, argv );

    poptContext optCon;
    try
    {
        fh_context c = Resolve( "socket:///tcp" );
        fh_context md = Resolve( "./echoclienttls.xml" );
        
        struct poptOption optionsTable[] = {
            FERRIS_POPT_OPTIONS
            FERRIS_CONTEXTPOPT_OPTIONS( md )            
            POPT_AUTOHELP
            POPT_TABLEEND
        };

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* ...");
    
        
        /***/
        int ch=-1;
        while ((ch = poptGetNextOpt(optCon)) >= 0)
        {}
        cout << "Option processing done." << endl;

        /*
         * Make connection
         */
        fh_context socketc = c->createSubContext("", md );
        fh_iostream ss     = socketc->getIOStream();
        
        
        /*
         * Dump out some certificate info
         */
        printCertificateInfo( socketc );

        /*
         * Dump out some interesting info about the connection.
         */
        printEA( socketc );
        
            
        /*
         * Do the echoing.
         */
        cout << "Begin typing in data for the echo server." << endl;
        for( string s; cin; )
        {
            getline( cin, s );

            if( ss << s << endl )
            {
                cerr << "Read and sent   :" << s << endl;
                printstate(ss);

                s = "undefined";
                if(getline(ss,s))
                {
                    cout << "Read from server:" << s << endl;
                    printstate(ss);
                }
            }
        }

        cout << "client done." << endl;
        printstate(ss);
    }
    catch( exception& e )
    {
        cerr << "Cought exception:" << e.what() << endl << flush;
    }
    
    poptFreeContext(optCon);
    return 0;
}
