/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris-get-emblems-within-range command line client
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

    $Id: ferris-get-emblems-within-range.cpp,v 1.3 2010/09/24 21:31:14 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris.hh>
#include <Medallion.hh>

#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ferris-get-emblems-within-range";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void showEmblemList( emblemset_t& el )
{
    bool v = true;
    for( emblemset_t::iterator ei = el.begin(); ei!=el.end(); ++ei )
    {
        fh_emblem em = *ei;
        if( v ) v = false;
        else    cout << ",";
        
        cout << em->getID();
    }
    cout << endl;
}

int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        const char* Z_CSTR                 = 0;
        unsigned long EAQueryFormat        = 0;
        unsigned long Verbose              = 0;
        unsigned long ShowDownSet          = 0;
        unsigned long InKilometers         = 0;
        double        LatRange             = 0;
        double        LongRange            = 0;
        double        BothRange            = 0;
        const char* PersonalityName_CSTR   = 0;
        const char* UseETagereAtPath       = 0;
        const char* EmblemName             = 0;
        const char* EmblemID               = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "report more details than normal", "" },

                { "eaquery-format", 'q', POPT_ARG_NONE, &EAQueryFormat, 0,
                  "generate output in the format the feaindexquery expects", "" },
                
                { "emblem-name", 'e', POPT_ARG_STRING, &EmblemName, 0,
                  "name of emblem for operation", "" },

                { "emblem-id", 'i', POPT_ARG_STRING, &EmblemID, 0,
                  "emblem ID for operation", "" },

                { "latitude-range", 'A', POPT_ARG_DOUBLE, &LatRange, 0,
                  "range left and right emblem", "" },

                { "longitude-range", 'O', POPT_ARG_DOUBLE, &LongRange, 0,
                  "range above and below emblem", "" },

                { "range", 'R', POPT_ARG_DOUBLE, &BothRange, 0,
                  "range to use for both lat/long (-A and -O in one)", "" },
                
                { "kilometers", 'k', POPT_ARG_NONE, &InKilometers, 0,
                  "latitude-range and latitude-range are in km instead of digital long/lat", "" },
                
                { "downset", 'd', POPT_ARG_NONE, &ShowDownSet, 0,
                  "show the downsets of all emblems within range.", "" },
                
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

        if( BothRange )
        {
            LatRange = BothRange;
            LongRange = BothRange;
        }
        
        
        double kmPerDigitalDigit = 111.10527282045992;
        if( InKilometers )
        {
//            cerr << "InKilometers" << endl;
            LatRange /= kmPerDigitalDigit;
            LongRange /= kmPerDigitalDigit;
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

        double elat = em->getDigitalLatitude();
        double elong = em->getDigitalLongitude();

//        cerr << "LatRange:" << LatRange << " LongRange:" << LongRange << endl;
//        cerr << " elat:" << elat << " elong:" << elong << endl;
        
        emblemset_t result;
        fh_emblem geospatialem = et->getEmblemByName( "libferris-geospatial" );
        emblems_t all = geospatialem->getDownset();
        for( emblems_t::iterator ai = all.begin(); ai != all.end(); ++ai )
        {
            double alat = (*ai)->getDigitalLatitude();
            double along = (*ai)->getDigitalLongitude();

            if( !alat || !along )
                continue;
            
//             cerr << "eid:" << (*ai)->getID()
//                  << " alat:" << alat << " along:" << along
//                  << endl;

            if( fabs( alat - elat ) < LatRange )
            {
                if( fabs( along - elong ) < LongRange )
                {
                    if( ShowDownSet )
                    {
                        emblems_t ds = (*ai)->getDownset();
                        copy( ds.begin(), ds.end(), inserter( result, result.end() ) );
                    }
                    else
                    {
                        result.insert( *ai );
                    }
                }
            }
        }

        if( EAQueryFormat )
        {
            cout << "(";
            if( result.size() > 1 )
                cout << "|";
            bool v = true;
            for( emblemset_t::iterator ei = result.begin(); ei!=result.end(); ++ei )
            {
                fh_emblem em = *ei;
                if( v ) v = false;
                else    cout << "";
        
                cout << "(emblem:id-" << em->getID() << "==1)";
            }
            cout << ")";
        }
        else
        {
            showEmblemList( result );
        }
    }
    catch( exception& e )
    {
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


