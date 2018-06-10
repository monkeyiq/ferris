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

    $Id: ut_medallion.cpp,v 1.1 2006/12/10 04:52:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/Medallion.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/Iterator.hh>
#include <Ferris/FilteredContext.hh>

#include <popt.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <map>
#include <list>
#include <iterator>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ut_medallion";

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

void
assertcompare_regex( const std::string& emsg,
                     const std::string& expected_re,
                     const std::string& actual )
{
    fh_rex r = toregexh( expected_re );
    if( !regex_match( actual, r, boost::match_any ) )
    {
        E() << emsg << endl
            << " expected:" << expected_re << ":" 
            << " actual:" << actual << ":" << endl;
        ofstream expectedss("/tmp/expected");
        expectedss << expected_re << flush;

        ofstream actualss("/tmp/actual");
        actualss << actual << flush;
        
    }
}


void
dump( const std::string& msg, emblems_t l )
{
    cerr << msg << endl;
    cerr << "dump(start)" << endl;
    for( emblems_t::iterator i = l.begin(); i != l.end(); ++i )
        cerr << " emblem id:" << (*i)->getID() << " name:" << (*i)->getName() << endl;
    cerr << "dump(end)" << endl;
    
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
string BaseDir = "/tmp";


void runtest_create_verysmall_Etagere()
{
    string path = BaseDir + "/etagere";
    fh_etagere et1 = Factory::makeEtagere( path );
    Factory::setDefaultEtagere( et1 );

//    fh_etagere et1 = Factory::getEtagere();
    
    stringlist_t sl;
    
    sl.push_back( "fav" );
    sl.push_back( "anime" );
    sl.push_back( "music" );
    sl.push_back( "drasp" );

    for( stringlist_t::iterator si = sl.begin(); si != sl.end(); ++si )
    {
        et1->createColdEmblem( *si );
    }
    et1->sync();
    
//     fh_etagere et2 = Factory::makeEtagere( path );
//     for( stringlist_t::iterator si = sl.begin(); si != sl.end(); ++si )
//     {
//         fh_emblem e1 = et1->getEmblemByName( *si );
//         fh_emblem e2 = et2->getEmblemByName( *si );
//     }
}

#define COMMON_ETAGERE_PREFIX_DATA \
    "  <emblem id=\"1\" name=\"libferris\" \n"                          \
    "[^>]+>\n" \
    "      <parents>\n"                                                 \
    "      </parents>\n"                                                \
    "      <children>\n"                                                \
    "          <emblemref id=\"\\d\" />\n"                                \
    "          <emblemref id=\"\\d\" />\n"                                \
    "          <emblemref id=\"\\d\" />\n"                                \
    "          <emblemref id=\"\\d\" />\n"                                \
    "      </children>\n"                                               \
    "  </emblem>\n"                                                     \
    "  <emblem id=\"2\" name=\"personalities\" \n"                      \
    "[^>]+>\n" \
    "      <parents>\n"                                                 \
    "          <emblemref id=\"\\d\" />\n"                                \
    "      </parents>\n"                                                \
    "      <children>\n"                                                \
    "          <emblemref id=\"\\d\" />\n"                                \
    "          <emblemref id=\"\\d\" />\n"                                \
    "      </children>\n"                                               \
    "  </emblem>\n"                                                     \
    "  <emblem id=\"4\" name=\"user\" \n"                               \
    "[^>]+>\n" \
    "      <parents>\n"                                                 \
    "          <emblemref id=\"2\" />\n"                                \
    "      </parents>\n"                                                \
    "      <children>\n"                                                \
    "      </children>\n"                                               \
    "  </emblem>\n"                                                     \
    "  <emblem id=\"5\" name=\"system\" \n"                             \
    "[^>]+>\n" \
    "      <parents>\n"                                                 \
    "          <emblemref id=\"1\" />\n"                                \
    "      </parents>\n"                                                \
    "      <children>\n"                                                \
    "      </children>\n"                                               \
    "  </emblem>\n"                                                     \
    "  <emblem id=\"6\" name=\"generic-user\" \n"                       \
    "[^>]+>\n" \
    "      <parents>\n"                                                 \
    "          <emblemref id=\"2\" />\n"                                \
    "      </parents>\n"                                                \
    "      <children>\n"                                                \
    "      </children>\n"                                               \
    "  </emblem>\n"                                                     \
    "  <emblem id=\"7\" name=\"ea-ordering\".*\n"                         \
    "[^>]+>\n" \
    "      <parents>\n"                                                   \
    "          <emblemref id=\"1\" />\n"                                  \
    "      </parents>\n"                                                  \
    "      <children>\n"                                                  \
    "      </children>\n"                                                 \
    "  </emblem>\n"                                                       \
    "  <emblem id=\"8\" name=\"libferris-geospatial\".*\n"                \
    "[^>]+>\n" \
    "      <parents>\n"                                                   \
    "          <emblemref id=\"1\" />\n"                                  \
    "      </parents>\n"                                                  \
    "      <children>\n"                                                  \
    "      </children>\n"                                                 \
    "  </emblem>\n"
;


                                                                                 

void
runtest_dumpto_verysmall()
{
    string path = BaseDir + "/etagere";
    fh_etagere et = Factory::makeEtagere( path );
    Factory::setDefaultEtagere( et );

    fh_stringstream ss;
    et->dumpTo( ss );
    cerr << "runtest_dumpto_verysmall() ss:" << tostr(ss) << endl;

    assertcompare_regex
        ( "invalid dumpTo() output in runtest_dumpto_verysmall()",
          "<etagere>\n"

          COMMON_ETAGERE_PREFIX_DATA
                   
          "  <emblem id=\"\\d+\" name=\"fav\" \n"
          "[^>]+>\n"
          "      <parents>\n"
          "      </parents>\n"
          "      <children>\n"
          "      </children>\n"
          "  </emblem>\n"
          "  <emblem id=\"\\d+\" name=\"anime\" \n"
          "[^>]+>\n"
          "      <parents>\n"
          "      </parents>\n"
          "      <children>\n"
          "      </children>\n"
          "  </emblem>\n"
          "  <emblem id=\"\\d+\" name=\"music\" \n"
          "[^>]+>\n"
          "      <parents>\n"
          "      </parents>\n"
          "      <children>\n"
          "      </children>\n"
          "  </emblem>\n"
          "  <emblem id=\"\\d+\" name=\"drasp\" \n"
          "[^>]+>\n"
          "      <parents>\n"
          "      </parents>\n"
          "      <children>\n"
          "      </children>\n"
          "  </emblem>\n"
          "</etagere>\n",
          tostr( ss ) );

}


void
runtest_set_icon_and_desc()
{
    string path = BaseDir + "/etagere";
    fh_etagere et1 = Factory::makeEtagere( path );
    Factory::setDefaultEtagere( et1 );
    string expected;
    
    fh_emblem em = et1->createColdEmblem( "freddy" );

    expected = "icons://xml.png";
    em->setIconName( expected );
    assertcompare( "emblem::setIconName() failed", expected, em->getIconName() );

    expected = "some waffle";
    em->setDescription( expected );
    assertcompare( "emblem::setDescription() failed", expected, em->getDescription() );
}

void
runtest_simple_medallion_set()
{
    string path = BaseDir + "/etagere";
    fh_etagere et1 = Factory::makeEtagere( path );
    Factory::setDefaultEtagere( et1 );
    fh_emblem   em = et1->getEmblemByName( "freddy" );
    fh_context   c = Shell::acquireContext( BaseDir + "/etagere/simple-med" );
    fh_medallion m = c->getMedallion();

    m->addEmblem( em );
}
void
runtest_simple_medallion_get()
{
    string path = BaseDir + "/etagere";
    fh_etagere et1 = Factory::makeEtagere( path );
    Factory::setDefaultEtagere( et1 );
    fh_emblem   em = et1->getEmblemByName( "freddy" );
    fh_context   c = Shell::acquireContext( BaseDir + "/etagere/simple-med" );
    fh_medallion m = c->getMedallion();

    if( !m->hasEmblem( em ) )
    {
        E() << "Failed to read back a medallion that contains the saved emblem" << endl;
    }

    emblems_t el = m->getMostSpecificEmblems();
    if( el.size() != 1 )
    {
        E() << "Medallion contains incorrect emblem count..." << endl
            << " expected: 1  actual:" << el.size()
            << endl;
    }
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void
runtest_linear_medallion_set()
{
    string path = BaseDir + "/etagere";
    fh_etagere et = Factory::makeEtagere( path );
    Factory::setDefaultEtagere( et );
    fh_context   c = Shell::acquireContext( BaseDir + "/etagere/linear-med" );
    fh_medallion m = c->getMedallion();

    m->addEmblem( et->getEmblemByName( "fav" ) );
    m->addEmblem( et->getEmblemByName( "music" ) );
}
void
runtest_linear_medallion_get()
{
    string path = BaseDir + "/etagere";
    fh_etagere et = Factory::makeEtagere( path );
    Factory::setDefaultEtagere( et );
    fh_context   c = Shell::acquireContext( BaseDir + "/etagere/linear-med" );
    fh_medallion m = c->getMedallion();

    if( !m->hasEmblem( et->getEmblemByName( "fav" ) ) )
    {
        E() << "Failed to read back a medallion that contains the saved emblem 'fav'" << endl;
    }
    if( !m->hasEmblem( et->getEmblemByName( "music" ) ) )
    {
        E() << "Failed to read back a medallion that contains the saved emblem 'music'" << endl;
    }

    emblems_t el = m->getMostSpecificEmblems();
    if( el.size() != 2 )
    {
        E() << "Medallion contains incorrect emblem count..." << endl
            << " expected: 2  actual:" << el.size()
            << endl;
    }
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void runtest_create_single_parent_chain()
{
    string path = BaseDir + "/etagere";
    fh_etagere et = Factory::makeEtagere( path );
    Factory::setDefaultEtagere( et );
    
    stringlist_t sl;
    
    sl.push_back( "fav" );
    sl.push_back( "anime" );
    sl.push_back( "music" );
    sl.push_back( "drasp" );

    emblems_t el;
    for( stringlist_t::iterator si = sl.begin(); si != sl.end(); ++si )
    {
        el.push_back( et->createColdEmblem( *si ) );
    }

    fh_emblem last = 0;
    for( emblems_t::iterator ei = el.begin(); ei != el.end(); ++ei )
    {
        if( last )
        {
            cerr << "*** linking last:" << last->getID() << " to:" << (*ei)->getID() << endl;
            link( last, *ei );
        }
        last = *ei;
    }

    et->sync();
}

void
runtest_dumpto_single_parent_chain()
{
    string path = BaseDir + "/etagere";
    fh_etagere et = Factory::makeEtagere( path );
    Factory::setDefaultEtagere( et );

    fh_stringstream ss;
    et->dumpTo( ss );
    cerr << "runtest_dumpto_verysmall() ss:" << tostr(ss) << endl;

    assertcompare_regex( "invalid dumpTo() output in runtest_dumpto_single_parent_chain()",
                   "<etagere>\n"
                   COMMON_ETAGERE_PREFIX_DATA
                   "  <emblem id=\"\\d+\" name=\"fav\" \n"
                   "[^>]+>\n"
                   "      <parents>\n"
                   "      </parents>\n"
                   "      <children>\n"
                   "          <emblemref id=\"\\d+\" />\n"
                   "      </children>\n"
                   "  </emblem>\n"
                   "  <emblem id=\"\\d+\" name=\"anime\" \n"
                   "[^>]+>\n"
                   "      <parents>\n"
                   "          <emblemref id=\"\\d+\" />\n"
                   "      </parents>\n"
                   "      <children>\n"
                   "          <emblemref id=\"\\d+\" />\n"
                   "      </children>\n"
                   "  </emblem>\n"
                   "  <emblem id=\"\\d+\" name=\"music\" \n"
                   "[^>]+>\n"
                   "      <parents>\n"
                   "          <emblemref id=\"\\d+\" />\n"
                   "      </parents>\n"
                   "      <children>\n"
                   "          <emblemref id=\"\\d+\" />\n"
                   "      </children>\n"
                   "  </emblem>\n"
                   "  <emblem id=\"\\d+\" name=\"drasp\" \n"
                   "[^>]+>\n"
                   "      <parents>\n"
                   "          <emblemref id=\"\\d+\" />\n"
                   "      </parents>\n"
                   "      <children>\n"
                   "      </children>\n"
                   "  </emblem>\n"
                   "</etagere>\n",
                   tostr(ss) );

}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void
runtest_single_parent()
{
    string path = BaseDir + "/etagere";
    fh_etagere et = Factory::makeEtagere( path );
    Factory::setDefaultEtagere( et );

    stringlist_t sl;
    
    sl.push_back( "fav" );
    sl.push_back( "anime" );
    sl.push_back( "music" );
    sl.push_back( "drasp" );
    sl.push_back( "amerc" );

    emblems_t el;
    for( stringlist_t::iterator si = sl.begin(); si != sl.end(); ++si )
    {
        el.push_back( et->createColdEmblem( *si ) );
    }

    fh_emblem last = el.back();
    el.pop_back();
    for( emblems_t::iterator ei = el.begin(); ei != el.end(); ++ei )
    {
        link( last, *ei );
    }

    et->sync();
}

void
runtest_dumpto_single_parent()
{
    string path = BaseDir + "/etagere";
    fh_etagere et = Factory::makeEtagere( path );
    Factory::setDefaultEtagere( et );
    
    fh_stringstream ss;
    et->dumpTo( ss );
    cerr << "runtest_dumpto_verysmall() ss:" << tostr(ss) << endl;

    assertcompare( "invalid dumpTo() output in runtest_dumpto_single_parent()",
                   "<etagere>\n"
                   COMMON_ETAGERE_PREFIX_DATA
                   "  <emblem id=\"1\" name=\"fav\" \n"
                   "       iconname=\"icons://unknown.png\" description=\"\" >\n"
                   "      <parents>\n"
                   "          <emblemref id=\"5\" />\n"
                   "      </parents>\n"
                   "      <children>\n"
                   "      </children>\n"
                   "  </emblem>\n"
                   "  <emblem id=\"2\" name=\"anime\" \n"
                   "       iconname=\"icons://unknown.png\" description=\"\" >\n"
                   "      <parents>\n"
                   "          <emblemref id=\"5\" />\n"
                   "      </parents>\n"
                   "      <children>\n"
                   "      </children>\n"
                   "  </emblem>\n"
                   "  <emblem id=\"3\" name=\"music\" \n"
                   "       iconname=\"icons://unknown.png\" description=\"\" >\n"
                   "      <parents>\n"
                   "          <emblemref id=\"5\" />\n"
                   "      </parents>\n"
                   "      <children>\n"
                   "      </children>\n"
                   "  </emblem>\n"
                   "  <emblem id=\"4\" name=\"drasp\" \n"
                   "       iconname=\"icons://unknown.png\" description=\"\" >\n"
                   "      <parents>\n"
                   "          <emblemref id=\"5\" />\n"
                   "      </parents>\n"
                   "      <children>\n"
                   "      </children>\n"
                   "  </emblem>\n"
                   "  <emblem id=\"5\" name=\"amerc\" \n"
                   "       iconname=\"icons://unknown.png\" description=\"\" >\n"
                   "      <parents>\n"
                   "      </parents>\n"
                   "      <children>\n"
                   "          <emblemref id=\"1\" />\n"
                   "          <emblemref id=\"2\" />\n"
                   "          <emblemref id=\"3\" />\n"
                   "          <emblemref id=\"4\" />\n"
                   "      </children>\n"
                   "  </emblem>\n"
                   "</etagere>\n",
                   tostr(ss) );
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void
runtest_large_tree()
{
    string path = BaseDir + "/etagere";
    fh_etagere et = Factory::makeEtagere( path );
    Factory::setDefaultEtagere( et );

    fh_emblem all       = et->createColdEmblem("all");
    fh_emblem animal    = et->createColdEmblem("animal");
    fh_emblem mammal    = et->createColdEmblem("mammal");
    fh_emblem human     = et->createColdEmblem("human");
    fh_emblem man       = et->createColdEmblem("man");
    link( all,     animal );
    link( animal,  mammal );
    link( mammal,  human  );
    link( human,   man    );

    fh_emblem vehicle   = et->createColdEmblem("vehicle");
    fh_emblem motorbike = et->createColdEmblem("motorbike");
    link( all,     vehicle );
    link( vehicle, motorbike );

    fh_emblem it        = et->createColdEmblem("it");
    fh_emblem software  = et->createColdEmblem("software");
    fh_emblem bonobo    = et->createColdEmblem("bonobo");
    fh_emblem gevas     = et->createColdEmblem("gevas");
    fh_emblem vfs       = et->createColdEmblem("vfs");
    fh_emblem ferris    = et->createColdEmblem("ferris");
    fh_emblem gnome_vfs = et->createColdEmblem("gnome-vfs");
    link( all,      it );
    link( it,       software );
    link( software, bonobo );
    link( mammal,   bonobo ); // break the plain tree a little.
    link( software, gevas );
    link( software, vfs );
    link( vfs,      ferris );
    link( vfs,      gnome_vfs );
    

    et->sync();
}

void
runtest_dumpto_large_tree()
{
    string path = BaseDir + "/etagere";
    fh_etagere et = Factory::makeEtagere( path );
    Factory::setDefaultEtagere( et );
    
    fh_stringstream ss;
    et->dumpTo( ss );
    cerr << "runtest_dumpto_large_tree() ss:" << tostr(ss) << endl;

    assertcompare_regex(
        "invalid dumpTo() output in runtest_dumpto_large_tree()",
        "<etagere>\n"
        COMMON_ETAGERE_PREFIX_DATA
        "  <emblem id=\"\\d+\" name=\"all\" \n"
        "[^>]+>\n"
        "      <parents>\n"
        "      </parents>\n"
        "      <children>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "          <emblemref id=\"\\d+\" />\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </children>\n"
        "  </emblem>\n"
        "  <emblem id=\"\\d+\" name=\"animal\" \n"
        "[^>]+>\n"
        "      <parents>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </parents>\n"
        "      <children>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </children>\n"
        "  </emblem>\n"
        "  <emblem id=\"\\d+\" name=\"mammal\" \n"
        "[^>]+>\n"
        "      <parents>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </parents>\n"
        "      <children>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </children>\n"
        "  </emblem>\n"
        "  <emblem id=\"\\d+\" name=\"human\" \n"
        "[^>]+>\n"
        "      <parents>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </parents>\n"
        "      <children>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </children>\n"
        "  </emblem>\n"
        "  <emblem id=\"\\d+\" name=\"man\" \n"
        "[^>]+>\n"
        "      <parents>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </parents>\n"
        "      <children>\n"
        "      </children>\n"
        "  </emblem>\n"
        "  <emblem id=\"\\d+\" name=\"vehicle\" \n"
        "[^>]+>\n"
        "      <parents>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </parents>\n"
        "      <children>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </children>\n"
        "  </emblem>\n"
        "  <emblem id=\"\\d+\" name=\"motorbike\" \n"
        "[^>]+>\n"
        "      <parents>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </parents>\n"
        "      <children>\n"
        "      </children>\n"
        "  </emblem>\n"
        "  <emblem id=\"\\d+\" name=\"it\" \n"
        "[^>]+>\n"
        "      <parents>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </parents>\n"
        "      <children>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </children>\n"
        "  </emblem>\n"
        "  <emblem id=\"\\d+\" name=\"software\" \n"
        "[^>]+>\n"
        "      <parents>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </parents>\n"
        "      <children>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "          <emblemref id=\"\\d+\" />\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </children>\n"
        "  </emblem>\n"
        "  <emblem id=\"\\d+\" name=\"bonobo\" \n"
        "[^>]+>\n"
        "      <parents>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </parents>\n"
        "      <children>\n"
        "      </children>\n"
        "  </emblem>\n"
        "  <emblem id=\"\\d+\" name=\"gevas\" \n"
        "[^>]+>\n"
        "      <parents>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </parents>\n"
        "      <children>\n"
        "      </children>\n"
        "  </emblem>\n"
        "  <emblem id=\"\\d+\" name=\"vfs\" \n"
        "[^>]+>\n"
        "      <parents>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </parents>\n"
        "      <children>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </children>\n"
        "  </emblem>\n"
        "  <emblem id=\"\\d+\" name=\"ferris\" \n"
        "[^>]+>\n"
        "      <parents>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </parents>\n"
        "      <children>\n"
        "      </children>\n"
        "  </emblem>\n"
        "  <emblem id=\"\\d+\" name=\"gnome-vfs\" \n"
        "[^>]+>\n"
        "      <parents>\n"
        "          <emblemref id=\"\\d+\" />\n"
        "      </parents>\n"
        "      <children>\n"
        "      </children>\n"
        "  </emblem>\n"
        "</etagere>\n",
        tostr(ss) );
}
    

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void
runtest_emblems_from_medallion_set()
{
    runtest_large_tree();
    fh_etagere et = Factory::getEtagere();

    fh_emblem man       = et->getEmblemByName("man");
    fh_emblem vehicle   = et->getEmblemByName("vehicle");
    fh_emblem bonobo    = et->getEmblemByName("bonobo");
    fh_emblem vfs       = et->getEmblemByName("vfs");
    
    fh_context   c = Shell::acquireContext( BaseDir + "/etagere/tree-med" );
    fh_medallion m = c->getMedallion();

    m->addEmblem( man );
    m->addEmblem( vehicle );
    m->addEmblem( bonobo );
    m->addEmblem( vfs );
}

void
runtest_emblems_from_medallion_get()
{
    string path = BaseDir + "/etagere";
    fh_etagere et = Factory::makeEtagere( path );
    Factory::setDefaultEtagere( et );

    fh_context   c = Shell::acquireContext( BaseDir + "/etagere/tree-med" );
    fh_medallion m = c->getMedallion();

    emblems_t specl = m->getMostSpecificEmblems();
    emblems_t upset = m->getAllEmblems();

    emblems_t expectedspecific;
    expectedspecific.push_back( et->getEmblemByName("man") );
    expectedspecific.push_back( et->getEmblemByName("vehicle") );
    expectedspecific.push_back( et->getEmblemByName("bonobo") );
    expectedspecific.push_back( et->getEmblemByName("vfs") );

    if( expectedspecific != specl )
    {
        E() << "List of expected emblems not found in the specific emblem list." << endl;
        dump( "expected", expectedspecific );
        dump( "actual",   specl );
    }

    emblems_t expected_upset;
    expected_upset.push_back( et->getEmblemByName("man") );
    expected_upset.push_back( et->getEmblemByName("vehicle") );
    expected_upset.push_back( et->getEmblemByName("bonobo") );
    expected_upset.push_back( et->getEmblemByName("vfs") );

    expected_upset.push_back( et->getEmblemByName("all") );
    expected_upset.push_back( et->getEmblemByName("animal") );
    expected_upset.push_back( et->getEmblemByName("mammal") );
    expected_upset.push_back( et->getEmblemByName("human") );

    expected_upset.push_back( et->getEmblemByName("it") );
    expected_upset.push_back( et->getEmblemByName("software") );

    expected_upset.sort();
    upset.sort();
    if( expected_upset != upset )
    {
        E() << "Upset of expected emblems not found in the specific emblem list." << endl;
        dump( "expected", expected_upset );
        dump( "actual",   upset );
    }
    
    
}

void
runtest_stateless_medallion_ea()
{
    string      path = BaseDir + "/etagere";
    fh_etagere  et   = Factory::getEtagere();
    fh_context  c    = Shell::acquireContext( BaseDir + "/etagere/tree-med" );

    assertcompare( "has-bonobo medallion stateless ea is incorrect!",
                   "1",
                   tostr( isTrue( getStrAttr( c, "emblem:has-bonobo", "0" ))));
    assertcompare( "has-felix medallion stateless ea is incorrect!",
                   "D", getStrAttr( c, "emblem:has-felix", "D" ));
    assertcompare( "has-motorbike medallion stateless ea is incorrect!",
                   "0",
                   tostr( isTrue( getStrAttr( c, "emblem:has-motorbike", "1" ))));
    

    cerr << "--------- about to create emblem -----------" << endl;
    // create new emblem
    fh_emblem em = et->createColdEmblem("fresh");
    et->sync();

    try
    {
        assertcompare( "has-fresh medallion stateless ea is incorrect! (fresh not added to em yet)",
                       "0",
                       getStrAttr( c, "emblem:has-fresh", "1", 1, 1 ));
    }
    catch( exception& e )
    {
        cerr << "EX:" << e.what() << endl;
    }
    fh_medallion m = c->getMedallion();
    m->sync();
    cerr << "em:" << toVoid( GetImpl( em )) << endl;
    m->addEmblem( em );
    m->sync();

    assertcompare( "has-fresh medallion stateless ea is incorrect! (fresh made and added)",
                   "1",
                   tostr( isTrue( getStrAttr( c, "emblem:has-fresh", "0" ))));
    
}



/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long Verbose              = 0;
        unsigned long CreateEtagere        = 0;
        unsigned long set_icon_and_desc    = 0;
        unsigned long simple_medallion_set = 0;
        unsigned long simple_medallion_get = 0;
        unsigned long linear_medallion_set = 0;
        unsigned long linear_medallion_get = 0;
        unsigned long single_parent_chain  = 0;
        unsigned long dumpto_single_parent_chain  = 0;
        unsigned long dumpto_verysmall     = 0;
        unsigned long single_parent        = 0;
        unsigned long dumpto_single_parent = 0;
        unsigned long large_tree           = 0;
        unsigned long dumpto_large_tree    = 0;
        unsigned long emblems_from_medallion_set = 0;
        unsigned long emblems_from_medallion_get = 0;
        unsigned long stateless_medallion_ea     = 0;
        const char*   BaseDir_CSTR         = "/tmp";

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "basedir", 0, POPT_ARG_STRING, &BaseDir_CSTR, 0,
                  "where the test files are located ", "/tmp" },
                
                { "create-verysmall-etagere", 0, POPT_ARG_NONE, &CreateEtagere, 0,
                  "create a etagere with a handful of new emblems and save/load it", "" },

                { "test-emblem-setters", 0, POPT_ARG_NONE, &set_icon_and_desc, 0,
                  "test the setXXX() methods on emblem", "" },

                { "test-simple-medallion-set", 0, POPT_ARG_NONE, &simple_medallion_set, 0,
                  "test giving a file a medallion with a single emblem "
                  "(expects --test-emblem-setters to have been run first)", "" },

                { "test-simple-medallion-get", 0, POPT_ARG_NONE, &simple_medallion_get, 0,
                  "test reading the test-simple-medallion-set result", "" },

                { "test-linear-medallion-set", 0, POPT_ARG_NONE, &linear_medallion_set, 0,
                  "test giving a file a medallion with a many emblems at one level "
                  "(expects --create-verysmall-etagere to have been run first)", "" },

                { "test-linear-medallion-get", 0, POPT_ARG_NONE, &linear_medallion_get, 0,
                  "test reading the test-linear-medallion-set result", "" },

                { "single-parent-chain", 0, POPT_ARG_NONE, &single_parent_chain, 0,
                  "create an etagere with a chain of emblems (emblem #parents=0 or 1)", "" },

                { "test-dumpto-single-parent-chain", 0, POPT_ARG_NONE, &dumpto_single_parent_chain, 0,
                  "tests the output against --single-parent-chain", "" },
                
                { "test-dumpto-verysmall", 0, POPT_ARG_NONE, &dumpto_verysmall, 0,
                  "tests the output against --create-verysmall-etagere", "" },

                { "test-single-parent", 0, POPT_ARG_NONE, &single_parent, 0,
                  "create a etagere with a single root emblem and many children", "" },

                { "test-dumpto-single-parent", 0, POPT_ARG_NONE, &dumpto_single_parent, 0,
                  "test XML output after running --test-single-parent", "" },

                
                { "test-large-tree", 0, POPT_ARG_NONE, &large_tree, 0,
                  "create a complex emblem collection with mixed parents and children", "" },

                { "test-dumpto-large-tree", 0, POPT_ARG_NONE, &dumpto_large_tree, 0,
                  "test XML output after running --test-large-tree", "" },
                
                

                { "emblems-from-medallion-set", 0, POPT_ARG_NONE, &emblems_from_medallion_set, 0,
                  "setup for test getting a tree of emblems from a medallion", "" },
                
                { "emblems-from-medallion-get", 0, POPT_ARG_NONE, &emblems_from_medallion_get, 0,
                  "test --emblems-from-medallion-set", "" },


                { "stateless-medallion-ea-test", 0, POPT_ARG_NONE, &stateless_medallion_ea, 0,
                  "test stateless ea for existing and newly created emblems in medallions."
                  "assumes --emblems-from-medallion-get has been run", "" },
                

                
                
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
        BaseDir = BaseDir_CSTR;

        if( CreateEtagere )
            runtest_create_verysmall_Etagere();

        if( set_icon_and_desc )
            runtest_set_icon_and_desc();

        if( simple_medallion_set )
            runtest_simple_medallion_set();

        if( simple_medallion_get )
            runtest_simple_medallion_get();

        if( linear_medallion_set )
            runtest_linear_medallion_set();

        if( linear_medallion_get )
            runtest_linear_medallion_get();

        if( single_parent_chain )
            runtest_create_single_parent_chain();

        if( dumpto_single_parent_chain )
            runtest_dumpto_single_parent_chain();

        if( dumpto_verysmall )
            runtest_dumpto_verysmall();

        if( single_parent )
            runtest_single_parent();

        if( dumpto_single_parent )
            runtest_dumpto_single_parent();

        if( large_tree )
            runtest_large_tree();
        if( dumpto_large_tree )
            runtest_dumpto_large_tree();
        
        if( emblems_from_medallion_set )
            runtest_emblems_from_medallion_set();
        if( emblems_from_medallion_get )
            runtest_emblems_from_medallion_get();

        if( stateless_medallion_ea )
            runtest_stateless_medallion_ea();
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
