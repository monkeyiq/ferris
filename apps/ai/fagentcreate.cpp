/******************************************************************************
*******************************************************************************
*******************************************************************************

    fagentcreate command line client
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

    $Id: fagentcreate.cpp,v 1.3 2010/09/24 21:31:08 ben Exp $

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
#include <Ferris/Personalities.hh>

#include <popt.h>
#include <unistd.h>
#include <SignalStreams.hh>
#include <iterator>

using namespace std;
using namespace Ferris;
using namespace Ferris::AI;

const string PROGRAM_NAME = "fagentcreate";

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
    const char* AgentName              = 0;
    const char* AgentImplemenationName = 0;
    const char* UseETagereAtPath       = 0;
    const char* stateDir               = 0;
    const char* EmblemName       = 0;
    const char* EmblemID         = 0;
    const char* PersonalityName  = 0;
    const char* PersonalityID    = 0;
    unsigned long Verbose        = 0;
    unsigned long isBinaryClassifierAgent = 0;
    
    try
    {

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "agent-name", 'a', POPT_ARG_STRING, &AgentName, 0,
                  "name of agent", "" },

                { "agent-impl-name", 'A', POPT_ARG_STRING, &AgentImplemenationName, 0,
                  "name of algorithm / plugin implementation to perform ML", "" },
                
                { "use-etagere", 0, POPT_ARG_STRING, &UseETagereAtPath, 0,
                  "use etagere at specified location instead of default", "" },

                { "statedir", 0, POPT_ARG_STRING, &stateDir, 0,
                  "where agent stores its state", "" },
                
                { "emblem-name", 'e', POPT_ARG_STRING, &EmblemName, 0,
                  "name of emblem for operation", "" },

                { "emblem-id", 'i', POPT_ARG_STRING, &EmblemID, 0,
                  "emblem ID for operation", "" },

                { "pers-name", 'p', POPT_ARG_STRING, &PersonalityName, 0,
                  "name of personality that agent makes assertions as", "" },

                { "pers-id", 'P', POPT_ARG_STRING, &PersonalityID, 0,
                  "emblem ID of personality that agent makes assertions as", "" },
                
                { "binary", 'b', POPT_ARG_NONE, &isBinaryClassifierAgent, 0,
                  "create a binary classifier agent", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "OPTIONS ...");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {
//         switch (c) {
//         }
        }

        if( !AgentName
            || !AgentImplemenationName
            || !stateDir
            || ( !EmblemName && !EmblemID )
            || ( !isBinaryClassifierAgent ) )
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
        
        fh_emblem em = 0;
        if( EmblemName )
            em = et->getEmblemByName( EmblemName );
        if( EmblemID )
            em = et->getEmblemByID( toType<emblemID_t>( EmblemID ));

        fh_personality pers = Factory::getGenericClassifierAgentPersonality( et );
        if( PersonalityName )
            pers = obtainPersonality( PersonalityName );
        if( PersonalityID )
            pers = obtainPersonality( toType<emblemID_t>( PersonalityID ));

        fh_agent d = 0;
        
        if( isBinaryClassifierAgent )
        {
            d = createBinaryAgent( AgentName,
                                   AgentImplemenationName,
                                   stateDir,
                                   em,
                                   pers );
        }
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


