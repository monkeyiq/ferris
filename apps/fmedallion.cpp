/******************************************************************************
*******************************************************************************
*******************************************************************************

    fmedallion command line client
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

    $Id: fmedallion.cpp,v 1.5 2010/09/24 21:31:15 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

/*
 * return 0 for success
 * return 1 for generic error
 * return 2 for unknown personality requested
*/

#include <Ferris.hh>
#include <Medallion.hh>
#include <Personalities.hh>

#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "fmedallion";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void showEmblemList( emblems_t& el )
{
    for( emblems_t::iterator ei = el.begin(); ei!=el.end(); ++ei )
    {
        fh_emblem em = *ei;
        cout << "id:" << em->getID() << " name:" << em->getName() << endl;
    }
}

int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        const char* Z_CSTR                 = 0;
        unsigned long Verbose              = 0;
        unsigned long ListEmblems          = 0;
        unsigned long AddEmblem            = 0;
        unsigned long RetractEmblem        = 0;
        unsigned long RemoveEmblem         = 0;
        unsigned long ListEmblem           = 0;
        unsigned long ShowBelief           = 0;
        unsigned long UpsetEmblem          = 0;
        unsigned long CreateEmblem         = 0;
        double        Sureness             = 0;
        const char* PersonalityName_CSTR   = 0;
        const char* UseETagereAtPath       = 0;
        const char* EmblemName             = 0;
        const char* EmblemID               = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "report more details than normal", "" },

                { "create", 'c', POPT_ARG_NONE, &CreateEmblem, 0,
                  "Create emblem is it doesn't already exist", "" },
                
                { "list-all-emblems", 'A', POPT_ARG_NONE, &ListEmblems, 0,
                  "Present a listing of all emblems sorted by name", "" },
                
                { "emblem-name", 'e', POPT_ARG_STRING, &EmblemName, 0,
                  "name of emblem for operation", "" },

                { "emblem-id", 'i', POPT_ARG_STRING, &EmblemID, 0,
                  "emblem ID for operation", "" },

                { "personality", 'p', POPT_ARG_STRING, &PersonalityName_CSTR, 0,
                  "personality to use (see also sureness)", "" },

                { "sureness", 's', POPT_ARG_DOUBLE, &Sureness, 0,
                  "sureness of assertion for personality (no effect for user personality)", "" },
                
                { "add", 'a', POPT_ARG_NONE, &AddEmblem, 0,
                  "add the emblem to all contexts listed", "" },

                { "retract", 'r', POPT_ARG_NONE, &RetractEmblem, 0,
                  "retract the emblem from all contexts listed (negative association)", "" },

                { "remove", 'R', POPT_ARG_NONE, &RemoveEmblem, 0,
                  "remove the emblem from all contexts listed (neither assertion or "
                  "retraction is recorded for the emblem, medallion, personality combination", "" },
                
                { "list", 'l', POPT_ARG_NONE, &ListEmblem, 0,
                  "show the emblems that each listed context has", "" },

                { "belief", 'b', POPT_ARG_NONE, &ShowBelief, 0,
                  "show the current assertions and retractions for all personalities for a file", "" },
                
                { "upset", 'u', POPT_ARG_NONE, &UpsetEmblem, 0,
                  "show the upset of the emblems that each listed context has", "" },
                
                { "use-etagere", 0, POPT_ARG_STRING, &UseETagereAtPath, 0,
                  "use etagere at specified location instead of default", "" },
                
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
        {}

        if (argc < 2 )
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

        // cerr << "CreateEmblem:" << CreateEmblem << endl;
        // cerr << "AddEmblem:" << AddEmblem << endl;
        // cerr << "EmblemName:" << EmblemName << endl;
        //
        // If they want to create a new emblem when it doesn't
        // already exist, then create it now.
        //
        if( CreateEmblem && AddEmblem && EmblemName )
        {
            try
            {
                et->getEmblemByName( EmblemName );
            }
            catch( EmblemNotFoundException& e )
            {
                fh_emblem root = et->obtainEmblemByName( "libferris-created-emblems" );
                fh_emblem em = root->obtainChild( EmblemName );
                et->sync();
            }
            catch(...)
            {
                throw;
            }
        }
        
        if( AddEmblem || RetractEmblem || RemoveEmblem || ListEmblem || UpsetEmblem || ShowBelief )
        {
            fh_emblem em = 0;
            if( EmblemName )
                em = et->getEmblemByName( EmblemName );
            if( EmblemID )
                em = et->getEmblemByID( toType<emblemID_t>( EmblemID ));
            
            while( const char* tmpCSTR = poptGetArg(optCon) )
            {
                string URL = tmpCSTR;
                try
                {
                    fh_context   c = Resolve( URL );
                    fh_medallion m = c->getMedallion();
                    fh_personality pers = Factory::getCurrentUserPersonality();

                    if( PersonalityName_CSTR )
                    {
//                        cerr << "Looking for personality:" << PersonalityName_CSTR << "..." << endl;
                        pers = obtainPersonality( PersonalityName_CSTR );
                        if( !pers )
                        {
                            cerr << "Personality:" << PersonalityName_CSTR << " not found" << endl;
                            exit( 2 );
                        }
//                        cerr << "Obtained personality:" << PersonalityName_CSTR << "..." << endl;
                    }
                    
                    if( AddEmblem )
                    {
                        if( PersonalityName_CSTR )
                        {
                            m->addEmblem( em, pers, Sureness );
                        }
                        else
                        {
                            m->addEmblem( em );
                        }
                    }
                    if( RetractEmblem )
                    {
                        if( PersonalityName_CSTR )
                        {
                            m->retractEmblem( em, pers, Sureness );
                        }
                        else
                        {
                            m->retractEmblem( em );
                        }
                    }
                    if( RemoveEmblem )
                    {
                        cerr << "RemoveEmblem em:" << em->getName()
                             << " pers:" << pers->getName()
                             << endl;
                        
                        if( PersonalityName_CSTR )
                        {
                            m->removeEmblem( em, pers );
                        }
                        else
                        {
                            m->removeEmblem( em );
                        }
                    }
                    if( ListEmblem )
                    {
                        emblems_t el = m->getMostSpecificEmblems();
                        cerr << "ListEmblem count:" << el.size() << endl;
                        showEmblemList( el );
                    }
                    if( ShowBelief )
                    {
                        emblems_t el = m->getMostSpecificEmblems();
                        for( emblems_t::iterator ei = el.begin(); ei!=el.end(); ++ei )
                        {
                            fh_emblem em = *ei;
                            
                            typedef std::list< fh_personality > plist_t;
                            plist_t plist = m->getListOfPersonalitiesWhoHaveOpinion( em );

                            cerr << "For emblem: " << em->getName() << endl;
                            cerr << "Fuzzy resolution from -100 to 100: "
                                 << m->getFuzzyBelief( em ) << endl;
                            for( plist_t::iterator pi = plist.begin(); pi!=plist.end(); ++pi )
                            {
                                fh_medallionBelief bel = m->getBelief( em, *pi );
                                fh_times        t = bel->getTimes();
                                double   sureness = bel->getSureness();
                            
                                cerr << "Personality: " << (*pi)->getName() << endl;
                                cerr << "   sureness: " << sureness << endl;
                                cerr << "      atime: " << Time::toTimeString(t->getATime()) << endl;
                                cerr << "      mtime: " << Time::toTimeString(t->getMTime()) << endl;
                            }
                            cerr << endl;
                            cerr << endl;
                        }
                    }
                    if( UpsetEmblem )
                    {
                        emblems_t el = m->getAllEmblems();
                        showEmblemList( el );
                    }
                    m->sync();
                }
                catch( exception& e )
                {
                    cerr << "Problem changing emblems for:" << URL << endl
                         << " e:" << e.what()
                         << endl;
                }
            }
        }
        
        if( ListEmblems )
        {
            emblems_t el = et->getAllEmblems();
            showEmblemList( el );
        }
    }
    catch( exception& e )
    {
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


