/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris echoserver

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

    $Id: echoserver.cpp,v 1.3 2008/05/25 21:30:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <string>
#include <list>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <Ferris.hh>
#include <ContextPopt.hh>

#include <popt.h>
#include <unistd.h>

#include <pthread.h>

using namespace std;
using namespace Ferris;


const string PROGRAM_NAME = "echoserver";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

class EchoArg : public Handlable
{
public:

    pthread_t threadID;
    fh_iostream ss;
    int Counter;

    int getNewCounter()
        {
            static int StaticCounter = 0;
            return ++StaticCounter;
        }
    
    EchoArg( fh_iostream _ss )
        :
        Counter( getNewCounter() ),
        ss(_ss)
        {
        }
    
};

FERRIS_SMARTPTR( EchoArg, fh_echoarg );



void* EchoThread(void * voiddata)
{
//     if( int rc = pthread_detach( pthread_self() ) )
//     {
// //        err_abort( status, "Detach thread" );
//     }
    
//    pthread_setcancelstate(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
 
    fh_echoarg state = (EchoArg*)voiddata;
    fh_iostream ss = state->ss;
    string s;

    cout << " voiddata:" << voiddata << endl;
    cout << "Reading data from client. Counter:" << state->Counter << endl;
    
    while( ss )
    {
        getline( ss, s );
        ss << s << endl;
        cout << " Thread Counter:" << state->Counter
             << " Read and sent   :" << s << endl;
        s = "undefined";
    }

//    pthread_exit(NULL);
    return NULL;
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
        cerr << "getting the tcp socket context" << endl;
        fh_context c = Resolve( "socket:///tcp" );
        
        cerr << "getting the creation metadata" << endl;
        fh_context md = Resolve( "./echoserver.xml" );

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
        
        fh_context socketc = c->createSubContext("", md );

        while( true )
        {
            vector<fh_echoarg> echos;
            
            fh_echoarg arg = new EchoArg( socketc->getIOStream() );
            echos.push_back( arg );
            
//            int rc = pthread_create( &arg->threadID, 0, EchoThread, GetImpl(arg));
            EchoThread( GetImpl(arg) );
        }
        poptFreeContext(optCon);
    }
    catch( exception& e )
    {
        cerr << "Cought exception:" << e.what() << endl << flush;
    }
    return 0;
}
