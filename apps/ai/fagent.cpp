/******************************************************************************
*******************************************************************************
*******************************************************************************

    fagent command line client
    Copyright (C) 2003 Ben Martin

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

    $Id: fagent.cpp,v 1.3 2010/09/24 21:31:08 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris/Ferris.hh>
#include <Ferris/Agent.hh>
#include <Ferris/Medallion.hh>

#include <popt.h>
#include <unistd.h>
#include <SignalStreams.hh>
#include <iterator>

using namespace std;
using namespace Ferris;
using namespace Ferris::AI;

const string PROGRAM_NAME = "fagent";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int exit_status = 0;
const char* AgentName        = 0;
const char* UseETagereAtPath = 0;
unsigned long TrainingMode   = 0;
unsigned long ListAgents     = 0;
unsigned long EraseAgent     = 0;
unsigned long Verbose        = 0;



int main( int argc, char** argv )
{
    
    try
    {

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "train", 't', POPT_ARG_NONE, &TrainingMode, 0,
                  "train agent with existing emblem attachment", "" },

                { "list-agents", 'l', POPT_ARG_NONE, &ListAgents, 0,
                  "list agent names that are available", "" },

                { "agent-name", 'a', POPT_ARG_STRING, &AgentName, 0,
                  "name of emblem for operation", "" },

                { "use-etagere", 0, POPT_ARG_STRING, &UseETagereAtPath, 0,
                  "use etagere at specified location instead of default", "" },

                { "erase", 0, POPT_ARG_NONE, &EraseAgent, 0,
                  "erase the agent with --agent-name", "" },
                
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

        if( ListAgents )
        {
            stringlist_t& sl = getAgentNames();
            copy( sl.begin(), sl.end(), ostream_iterator<string>( cout, "\n"));
            exit( 0 );
        }

        
        if (argc < 2 )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
        if ( !AgentName )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        fh_etagere et = Factory::getEtagere();
        if( UseETagereAtPath )
        {
            et = Factory::makeEtagere( UseETagereAtPath );
            Factory::setDefaultEtagere( et );
        }
        
        fh_agent d = getAgent( AgentName );
        
        if ( !d )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        if( EraseAgent )
        {
            eraseAgent( d );
            exit(0);
        }
        
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            try
            {
                string srcURL = tmpCSTR;
                fh_context c = Resolve( srcURL );

                if( TrainingMode )
                {
                    d->addTrainingExample( c );
                }
                else
                {
                    d->classify( c );
                }
            }
            catch( exception& e )
            {
                cerr << "error:" << e.what() << endl;
                exit_status = 1;
            }
        }

        if( TrainingMode )
            d->train();
        
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


