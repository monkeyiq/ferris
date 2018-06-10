/******************************************************************************
*******************************************************************************
*******************************************************************************

    unit test code. Note assumes ParseTimeString() has been tested as correct

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

    $Id: ut_ParseRelativeTimeString.cpp,v 1.1 2006/12/10 04:52:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris.hh>
#include <Ferris/Iterator.hh>
#include <popt.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <map>
#include <list>
#include <iterator>

using namespace std;
using namespace Ferris;
using namespace Ferris::Time;

const string PROGRAM_NAME = "ut_ParseRelativeTimeString";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

const string ttformat = "%d/%m/%y %H:%M:%S";
int errors = 0;

void five_mintues_ago_test( time_t reftt, const std::string& deltadesc )
{
    time_t tt = ParseRelativeTimeString( deltadesc, reftt );
    cerr << "reference time:" << toTimeString( reftt,       ttformat )
         << " and operation:" << deltadesc
         << " gives:"      << toTimeString( tt, ttformat ) << endl;

    time_t gap = 5*60;
    if( reftt - gap != tt )
    {
        cerr << "ERROR reference time_t + " << deltadesc
             << " does not work correctly gap is:" << (reftt - tt)
             << " should be:" << gap
             << endl;
        ++errors;
    }
}

void x_hour_ago_test( time_t reftt, const std::string& deltadesc, int numberOfHours )
{
    time_t tt = ParseRelativeTimeString( deltadesc, reftt );
    cerr << "reference time:" << toTimeString( reftt,       ttformat )
         << " and operation:" << deltadesc
         << " numberOfHours:" << numberOfHours
         << " gives:"      << toTimeString( tt, ttformat ) << endl;

    time_t gap = numberOfHours * 3600;
    if( reftt - gap != tt )
    {
        cerr << "ERROR reference time_t + " << deltadesc
             << " does not work correctly gap is:" << (reftt - tt)
             << " should be:" << gap
             << endl;
        ++errors;
    }
}

void last_month_test( time_t reftt, const std::string& deltadesc )
{
    time_t tt = ParseRelativeTimeString( deltadesc, reftt );
    cerr << "reference time:" << toTimeString( reftt,       ttformat )
         << " and operation:" << deltadesc
         << " gives:"      << toTimeString( tt, ttformat ) << endl;

    time_t gap = 30*24*3600;
    if( reftt - gap != tt )
    {
        cerr << "ERROR reference time_t + " << deltadesc
             << " does not work correctly gap is:" << (reftt - tt)
             << " should be:" << gap
             << endl;
        ++errors;
    }
}

void begin_last_month_test( time_t reftt, const std::string& deltadesc )
{
    time_t tt = ParseRelativeTimeString( deltadesc, reftt );
    cerr << "reference time:" << toTimeString( reftt,       ttformat )
         << " and operation:" << deltadesc
         << " gives:"      << toTimeString( tt, ttformat ) << endl;

    time_t gap = 30 + 20*60 + 10*3600 + 24*24*3600 + 30*24*3600;
    if( reftt - gap != tt )
    {
        cerr << "ERROR reference time_t + " << deltadesc
             << " does not work correctly gap is:" << (reftt - tt)
             << " should be:" << gap
             << endl;
        ++errors;
    }
}

void begin_last_3months_test( time_t reftt, const std::string& deltadesc )
{
    time_t tt = ParseRelativeTimeString( deltadesc, reftt );
    cerr << "reference time:" << toTimeString( reftt,       ttformat )
         << " and operation:" << deltadesc
         << " gives:"      << toTimeString( tt, ttformat ) << endl;

    time_t gap = 30 + 20*60 + 10*3600 + 24*24*3600 + 30*24*3600
        + 31*24*3600
        + 30*24*3600;
    if( reftt - gap != tt )
    {
        cerr << "ERROR reference time_t + " << deltadesc
             << " does not work correctly gap is:" << (reftt - tt)
             << " should be:" << gap
             << endl;
        ++errors;
    }
}


void runtest()
{
    struct tm reftm = ParseTimeString( "25/12/03 10:20:30" );
    time_t    reftt = mktime( &reftm );
    
    time_t yesterdaytt = ParseRelativeTimeString( "yesterday", reftt );
    cerr << "reference time:" << toTimeString( reftt,       ttformat )
         << " yesterday:"     << toTimeString( yesterdaytt, ttformat ) << endl;

    if( reftt - (30+20*60+10*3600 + 24*3600 ) != yesterdaytt )
    {
        cerr << "ERROR reference time_t + 'yesterday' does not work correctly" << endl;
        ++errors;
    }


    
    time_t tomorrowtt = ParseRelativeTimeString( "tomorrow", reftt );
    cerr << "reference time:" << toTimeString( reftt,       ttformat )
         << " tomorrow:"      << toTimeString( tomorrowtt, ttformat ) << endl;

    if( reftt + (30+39*60+13*3600 ) != tomorrowtt )
    {
        cerr << "ERROR reference time_t + 'tomorrow' does not work correctly"
             << " gap is:" << (reftt - tomorrowtt)
             << " should be:" << (-1*(30+39*60+13*3600 ))
             << endl;
        ++errors;
    }

    five_mintues_ago_test( reftt, "5 minutes ago" );
    five_mintues_ago_test( reftt, "-5minutes" );
    five_mintues_ago_test( reftt, "-5min" );
    
    x_hour_ago_test( reftt, "1 hour ago", 1 );
    x_hour_ago_test( reftt, "-1hr", 1 );
    x_hour_ago_test( reftt, "-1h", 1 );

    x_hour_ago_test( reftt, "13 hour ago", 13 );

    last_month_test( reftt, "last month" );
    begin_last_month_test( reftt, "begin last month" );
    begin_last_3months_test( reftt, "begin last 3 months" );

    if( !errors )
        cerr << "Success" << endl;
}


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long Verbose              = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]*  ...");

        /* Now do options processing */
        char c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}

        runtest();
    }
    catch( exception& e )
    {
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}
