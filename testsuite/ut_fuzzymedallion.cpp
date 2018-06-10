/******************************************************************************
*******************************************************************************
*******************************************************************************

    unit test code
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

    $Id: ut_fuzzymedallion.cpp,v 1.1 2006/12/10 04:52:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris/All.hh>
#include <Ferris/Configuration_private.hh>

#include <popt.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <map>
#include <list>
#include <iterator>

#include <STLdb4/stldb4.hh>

using namespace std;
using namespace Ferris;
using namespace STLdb4;

const string PROGRAM_NAME = "ut_fuzzymedallion";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

int errors = 0;

fh_ostream& E()
{
    ++errors;
    static fh_ostream ret = Factory::fcerr();
    ret << "error:";
    return ret;
}

void
assertcompare( const std::string& emsg,
               const std::string& expected,
               const std::string& actual )
{
    if( expected != actual )
        E() << emsg << endl
            << " expected:" << expected << ":" 
            << " actual:" << actual << ":" << endl;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

string BaseDir = "/tmp";
string FileName = "test.db";

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long Verbose              = 0;
        const char*   BaseDir_CSTR         = "/tmp";
        const char*   FileName_CSTR        = "test.db";
        const char*   Personality_CSTR        = "user";
        const char*   EmblemBeliefsMatch_CSTR = 0;
        const char*   EmblemFuzzyValues_CSTR  = 0;
        
        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "basedir", 0, POPT_ARG_STRING, &BaseDir_CSTR, 0,
                  "where the test files are located ", "/tmp" },

                { "filename", 'f', POPT_ARG_STRING, &FileName_CSTR, 0,
                  "what is the name of the db file to play with in --basedir ", "test.db" },

                { "personality", 'p', POPT_ARG_STRING, &Personality_CSTR, 0,
                  "what personality to check the beliefs of", "user" },
                
                { "emblem-beliefs-match", 0, POPT_ARG_STRING, &EmblemBeliefsMatch_CSTR, 0,
                  "list of emblems and assertions for personality -p"
                  " in the form emblemname -100_to_100 emblemname -100_to_100 ...", "" },

                { "emblem-fuzzy-values", 0, POPT_ARG_STRING, &EmblemFuzzyValues_CSTR, 0,
                  "list of emblems and fuzzy resolution of assertions"
                  " in the form emblemname -100_to_100 emblemname -100_to_100 ...", "" },
                
                
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]*  ...");

        /* Now do options processing */
        {
            char c=-1;
            while ((c = poptGetNextOpt(optCon)) >= 0)
            {}
        }
        BaseDir  = BaseDir_CSTR;
        FileName = FileName_CSTR;

        fh_context        c = Resolve( BaseDir + "/" + FileName );
        fh_medallion      m = c->getMedallion();
        fh_personality pers = findPersonality( Personality_CSTR );
        fh_etagere       et = Factory::getEtagere();
        emblems_t        el = et->getAllEmblems();
        
        typedef map< string, double > TestData_t;
        TestData_t ExpectedData;
        TestData_t ActualData;

        if( EmblemBeliefsMatch_CSTR || EmblemFuzzyValues_CSTR )
        {
            fh_stringstream iss;
            string datastr = (EmblemBeliefsMatch_CSTR ? EmblemBeliefsMatch_CSTR : EmblemFuzzyValues_CSTR);
            cerr << "datastr:" << datastr << endl;
            iss << datastr;
            while( iss )
            {
                string s;
                string ename;
                double sureness;

                if( iss >> ename )
                    if( iss >> sureness )
                    {
                        ExpectedData[ ename ] = sureness;
                        cerr << "Ex name:" << ename
                             << " sureness:" << sureness
                             << endl;
                    }
            }
        }
        
        if( EmblemBeliefsMatch_CSTR )
        {
            for( emblems_t::iterator ei = el.begin(); ei!=el.end(); ++ei )
            {
                fh_emblem em = *ei;
                if( m->hasBelief( em, pers ) )
                {
                    fh_medallionBelief b = m->getBelief( em, pers );
                    ActualData[ em->getName() ] = b->getSureness();
                    cerr << "Ac name:" << em->getName()
                         << " sureness:" << b->getSureness()
                         << endl;
                }
            }
            
            if( ActualData != ExpectedData )
            {
                E() << "Different emblem beliefs than expected for personality:" << pers->getName()
                    << " on the file " << c->getURL() << endl;
            }
        }
        else if( EmblemFuzzyValues_CSTR )
        {
            for( emblems_t::iterator ei = el.begin(); ei!=el.end(); ++ei )
            {
                fh_emblem em = *ei;
                double sureness = m->getFuzzyBelief( em );
                if( sureness != 0 )
                {
                    ActualData[ em->getName() ] = sureness;
                    cerr << "Ac name:" << em->getName()
                         << " sureness:" << sureness
                         << endl;
                }
            }
            
            if( ActualData != ExpectedData )
            {
                E() << "Different emblem beliefs than expected for personality:" << pers->getName()
                    << " on the file " << c->getURL() << endl;
            }
        }
        

    }
    catch( exception& e )
    {
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    if( !errors )
        cerr << "Success" << endl;
    else
        cerr << "error: error count != 0" << endl;
    return exit_status;
}
