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

    $Id: ut_outofproc_notification.cpp,v 1.1 2006/12/10 04:52:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris/Ferris.hh>
#include <Ferris/PluginOutOfProcNotificationEngine.hh>
#include <Ferris/Runner.hh>
#include <Ferris/Medallion.hh>

#include <popt.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>


#include <string>
#include <vector>
#include <map>
#include <list>
#include <iterator>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ut_outofproc_notification";

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

string BaseDir        = "/tmp/ut_outofproc_notification";
string UnitTestExeBaseDir = "";
string OutputFileName = "/tmp/ut_outofproc_notification/my-output";
static bool s_looping = true;



void sig_usr2_cb(int sign)
{
    s_looping = false;
}

void runtest_saveInputMessages()
{
    OutOfProcNotificationEngineDebug en;
    en.setBaseDir( BaseDir );
    fh_ofstream oss = fh_ofstream( OutputFileName, ios::out | ios::trunc );
    cerr << "OutputFileName:" << OutputFileName << endl;
    oss << "header" << endl;
    en.setOutputStream( oss );
    en.connect();

    LG_JOURNAL_D << "--run-simple-client waiting for data to save at:" << OutputFileName
                 << " from server on basedir:" << BaseDir
                 << " getpid:" << tostr(getpid())
                 << endl;
    
    while( s_looping )
    {
        while( g_main_pending() )
            g_main_iteration( false );
        g_usleep(50);
    }
    oss << flush;
}

void runtest_sendSimpleMessages()
{
    OutOfProcNotificationEngineDebug en;
    en.setBaseDir( BaseDir );
    fh_ofstream oss = fh_ofstream( OutputFileName, ios::out | ios::trunc );
    oss << "server header" << endl;
    en.setOutputStream( oss );
    en.connect();

    LG_JOURNAL_D << "--run-simple-serv OutputFileName:" << OutputFileName
                 << " basedir:" << BaseDir
                 << " getpid:" << tostr(getpid())
                 << endl;
    
    stringmap_t m1, m2, m3;

    m1[ "m1key1" ] = "m1value1";
    m1[ "m1key2" ] = "m1value2";
    m1[ "m1key3" ] = "m1value3";

    m2[ "m2key1" ] = "m2value1";
    m2[ "m2key2" ] = "m2value2";
    m2[ "m2key3" ] = "m2value3";

    m3[ "m3key1" ] = "m3value1";
    m3[ "m3key2" ] = "m3value2";
    m3[ "m3key3" ] = "m3value3";
    
    en.sendMessage( m1 );
    usleep( random() % 100 + 10000 );
    en.sendMessage( m2 );
    usleep( random() % 100 + 10000 );
    en.sendMessage( m3 );
}

typedef map< int, fh_runner > clients_t;

void run_readers( clients_t& cl, int count )
{
    for( int i=0; i<count; ++i )
    {
        cl[i] = new Runner();
        cl[i]->setSpawnFlags(
            GSpawnFlags(
                G_SPAWN_SEARCH_PATH |
                G_SPAWN_STDERR_TO_DEV_NULL |
                G_SPAWN_STDOUT_TO_DEV_NULL |
                cl[i]->getSpawnFlags()));
        fh_stringstream cmdss;
        cmdss << UnitTestExeBaseDir << "/" << PROGRAM_NAME
              << " --run-simple-client"
              << " --outputfile /tmp/client-out-" << i
              << " --basedir " << BaseDir;
        cl[i]->setCommandLine( tostr( cmdss ) );
        cerr << "----------------" << endl;
        cerr << "Starting client:" << tostr(cmdss) << endl;
        cerr << "----------------" << endl;
        cl[i]->Run();
    }

    // a little racey
    sleep( 10 );
}

void
run_writers( clients_t& cl, int count )
{
    for( int i=0; i<count; ++i )
    {
        cl[i] = new Runner();
        cl[i]->setSpawnFlags(
            GSpawnFlags(
                G_SPAWN_SEARCH_PATH |
                G_SPAWN_STDERR_TO_DEV_NULL |
                G_SPAWN_STDOUT_TO_DEV_NULL |
                cl[i]->getSpawnFlags()));
        fh_stringstream cmdss;
        cmdss << UnitTestExeBaseDir << "/" << PROGRAM_NAME
              << " --run-simple-serv"
              << " --outputfile /tmp/serv-out"
              << " --basedir " << BaseDir;
        cl[i]->setCommandLine( tostr( cmdss ) );
        cerr << "----------------" << endl;
        cerr << "Starting server:" << tostr(cmdss) << endl;
        cerr << "----------------" << endl;
        cl[i]->Run();
    }
}

void
wait_all( string desc, clients_t& cl )
{
    int count = cl.size();
    
    for( int i=0; i<count; ++i )
    {
        cerr << "----------------" << endl;
        cerr << "waiting on " << desc << " pid:" << cl[i]->getChildProcessID() << endl;
        cerr << "----------------" << endl;
    
        gint e = cl[i]->getExitStatus();
        // e == 0 for success
        if( e > 1 )
        {
            E() << desc << " failed" << endl;
        }
    }
}


void
runtest_simple( int numWriters = 1, int numReaders = 3 )
{
    clients_t readers;
    clients_t writers;

    run_readers( readers, numReaders );
    run_writers( writers, numWriters );

    wait_all( "writing servers", writers );

    cerr << "----------------" << endl;
    cerr << "sleeping for clients to catch up" << endl;
    cerr << "----------------" << endl;
    // a little racey
    sleep( 10 );
    
    for( int i=0; i<numReaders; ++i )
    {
        readers[i]->signalChild( SIGUSR2 );
    }
    wait_all( "reader clients", readers );
    
    // now we need to test the output generated by the clients
    // to make sure that they all got what we gave them
    if( errors )
        return;

    if( numWriters==1 )
    {
        string expectedOutputFileName = "ut_outofproc_notification_simpleclient_output.txt";
        for( int i=0; i<numReaders; ++i )
        {
            fh_ifstream expectedss = fh_ifstream( UnitTestExeBaseDir + "/" + expectedOutputFileName );
            fh_ifstream actualss   = fh_ifstream( "/tmp/client-out-" + tostr(i) );
            
            assertcompare( "actual output of client " + tostr(i) + " was not as expected",
                           StreamToString( expectedss ),
                           StreamToString( actualss ) );
        }
    }
    else
    {
        // because we add usleep()s into places to mix it up a little
        // we can't expect messages written to be in the exact order
        // that servers are started. We can only compare the sorted
        // output to the sorted expected.
        string expectedOutputFileName = "ut_outofproc_notification_simpleclient_output_"
            + tostr(numWriters) + ".txt";

        fh_ifstream expectedss = fh_ifstream( UnitTestExeBaseDir + "/" + expectedOutputFileName );
        string s;
        set< string > expectedset;
        set< string > actualset;
        while( getline( expectedss, s ))
            if( expectedset.find( s ) == expectedset.end() )
                expectedset.insert( s );
        
        for( int i=0; i<numReaders; ++i )
        {
            fh_ifstream actualss   = fh_ifstream( "/tmp/client-out-" + tostr(i) );

            while( getline( actualss, s ))
                if( actualset.find( s ) == actualset.end() )
                    actualset.insert( s );

            if( expectedset != actualset )
            {
                E() << "actual output of client " << tostr(i) << " was not as expected" << endl;
                E() << " expected follows" << endl;
                for( set< string >::iterator si = expectedset.begin(); si != expectedset.end(); ++si )
                    E() << *si << endl;
                for( set< string >::iterator si = actualset.begin(); si != actualset.end(); ++si )
                    E() << *si << endl;
            }
        }
    }
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


void runtest_monitorMedallion_OnUpdate( fh_context c,
                                        fh_ofstream* oss_ptr,
                                        emblems_t* el_ptr )
{
    fh_ofstream& oss = *oss_ptr;
    emblems_t&    el = *el_ptr;

    fh_medallion m = c->getMedallion();
    emblems_t currentel = m->getMostSpecificEmblems( Emblem::LIMITEDVIEW_PRI_LOW );
    emblems_t removed;
    emblems_t added;
    
    el.sort();
    currentel.sort();
    set_difference( el.begin(), el.end(),
                    currentel.begin(), currentel.end(),
                    back_inserter( removed ));
    set_difference( currentel.begin(), currentel.end(),
                    el.begin(), el.end(),
                    back_inserter( added ));

    fh_stringstream ss;
    
    ss << "el emblems:";
    for( emblems_t::iterator ei = el.begin(); ei != el.end(); ++ei )
        ss << (*ei)->getUniqueName() << ",";
    ss << endl;

    ss << "currentel emblems:";
    for( emblems_t::iterator ei = currentel.begin(); ei != currentel.end(); ++ei )
        ss << (*ei)->getUniqueName() << ",";
    ss << endl;
    
    ss << "added emblems:";
    for( emblems_t::iterator ei = added.begin(); ei != added.end(); ++ei )
        ss << (*ei)->getUniqueName() << ",";
    ss << endl;

    ss << "removed emblems:";
    for( emblems_t::iterator ei = removed.begin(); ei != removed.end(); ++ei )
        ss << (*ei)->getUniqueName() << ",";
    ss << endl;

    cerr << "updates are as follows..." << endl;
    cerr << tostr(ss) << endl;
    oss << tostr(ss) << endl;
    
    el.clear();
    copy( currentel.begin(), currentel.end(), back_inserter( el ));
}

void
runtest_monitorMedallion( const std::string& url )
{
    fh_context    c = Resolve( url );
    fh_medallion  m = c->getMedallion();
    fh_ofstream oss = fh_ofstream( OutputFileName, ios::out | ios::trunc );
    emblems_t    el = m->getMostSpecificEmblems( Emblem::LIMITEDVIEW_PRI_LOW );
    
    c->getNamingEvent_MedallionUpdated_Sig().connect(
        boost::bind( &runtest_monitorMedallion_OnUpdate, _1, &oss, &el ));

    oss << "medallion header" << endl;

    while( s_looping )
    {
        while( g_main_pending() )
            g_main_iteration( false );
        g_usleep(50);
    }
}

/**
 * for the medallion of url then add/remove emname.
 *
 * if 'has' is true then make sure this medallion has em
 * is 'has' is false then make sure the med hasn't got the emblem
  */
void
runtest_updateMedallion( const std::string& url, const std::string& emname, bool has )
{
    fh_context   c = Resolve( url );
    fh_medallion m = c->getMedallion();
    fh_etagere  et = Factory::getEtagere();
    fh_emblem   em = et->getEmblemByName( emname );

    m->ensureEmblem( em, has );
}

void
runtest_medallionNotification_update( const std::string& MedallionURL,
                                      const std::string& emblemName,
                                      bool has )
{
    fh_runner r = new Runner();
    r->setSpawnFlags(
        GSpawnFlags(
            G_SPAWN_SEARCH_PATH |
            G_SPAWN_STDERR_TO_DEV_NULL |
            G_SPAWN_STDOUT_TO_DEV_NULL |
            r->getSpawnFlags()));
    fh_stringstream cmdss;
    cmdss << UnitTestExeBaseDir << "/" << PROGRAM_NAME
          << " --run-med-update"
          << " --medallion " << MedallionURL
          << " --emblem " << emblemName;

    if( has )
        cmdss << " --medallion-has";
    
    cmdss << " --outputfile /tmp/serv-out"
          << " --basedir " << BaseDir;
    r->setCommandLine( tostr( cmdss ) );
    cerr << "----------------" << endl;
    cerr << "Starting update server:" << tostr(cmdss) << endl;
    cerr << "----------------" << endl;
    r->Run();

    clients_t tmp;
    tmp[0] = r;
    wait_all( "medallion update client", tmp );
}


void
runtest_medallionNotification()
{
    clients_t readers;
    int numReaders = 2;
    string MedallionURL = "/tmp/medallion-plaything";

    Shell::touch( MedallionURL );
    
    for( int i=0; i<numReaders; ++i )
    {
        readers[i] = new Runner();
        readers[i]->setSpawnFlags(
            GSpawnFlags(
                G_SPAWN_SEARCH_PATH |
                G_SPAWN_STDERR_TO_DEV_NULL |
                G_SPAWN_STDOUT_TO_DEV_NULL |
                readers[i]->getSpawnFlags()));
        fh_stringstream cmdss;
        cmdss << UnitTestExeBaseDir << "/" << PROGRAM_NAME
              << " --run-med-monitor"
              << " --medallion " << MedallionURL
              << " --outputfile /tmp/client-out-" << i
              << " --basedir " << BaseDir;
        readers[i]->setCommandLine( tostr( cmdss ) );
        cerr << "----------------" << endl;
        cerr << "Starting client:" << tostr(cmdss) << endl;
        cerr << "----------------" << endl;
        readers[i]->Run();
    }

    // a little racey
    sleep( 10 );

    string en_tape = "tape";
    string en_bomb = "bomb";

    fh_etagere et = Factory::getEtagere();
    try {
        et->getEmblemByName( en_tape );
    }
    catch( ... ) {
        et->createColdEmblem( en_tape );
    }
    try {
        et->getEmblemByName( en_bomb );
    }
    catch( ... ) {
        et->createColdEmblem( en_bomb );
    }
    

    runtest_medallionNotification_update( MedallionURL, en_tape,  true );
    runtest_medallionNotification_update( MedallionURL, en_bomb, true );
    runtest_medallionNotification_update( MedallionURL, en_tape,  false );
    runtest_medallionNotification_update( MedallionURL, en_bomb, true );
    runtest_medallionNotification_update( MedallionURL, en_bomb, false );
    
    
    wait_all( "medallion monitor clients", readers );
    
    // now we need to test the output generated by the clients
    // to make sure that they all got what we gave them
    if( errors )
        return;

    string expectedOutputFileName = "ut_outofproc_notification_med_output.txt";
    for( int i=0; i<numReaders; ++i )
    {
        fh_ifstream expectedss = fh_ifstream( UnitTestExeBaseDir + "/" + expectedOutputFileName );
        fh_ifstream actualss   = fh_ifstream( "/tmp/client-out-" + tostr(i) );
            
        assertcompare( "actual output of client " + tostr(i) + " was not as expected",
                       StreamToString( expectedss ),
                       StreamToString( actualss ) );
    }
    
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
        unsigned long Simple               = 0;
        unsigned long ManyWriters          = 0;
        unsigned long SimpleClient         = 0;
        unsigned long SimpleServer         = 0;
        const char*   BaseDir_CSTR         = "/tmp/baseDir";
        const char*   OutputFileName_CSTR  = "/tmp/baseDir/out";
        const char*   UnitTestExeBaseDir_CSTR = "/ferris/testsuite";

        unsigned long MedallionTest            = 0;
        unsigned long MedallionMonitor         = 0;
        unsigned long MedallionUpdate          = 0;
        const char*   EmblemName_CSTR          = "tape";
        const char*   MedallionFileName_CSTR   = "";
        unsigned long MedallionHas             = 0;
        
        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "basedir", 0, POPT_ARG_STRING, &BaseDir_CSTR, 0,
                  "where the test files are located ", "/tmp/baseDir" },

                { "unittest-exe-basedir", 0, POPT_ARG_STRING, &UnitTestExeBaseDir_CSTR, 0,
                  "directory that contains ut_outofproc_notification executable", "" },

                { "outputfile", 0, POPT_ARG_STRING, &OutputFileName_CSTR, 0,
                  "where to save messages that are read", "/tmp/baseDir/out" },
                
                { "run-simple", '1', POPT_ARG_NONE, &Simple, 0,
                  "run a simple test with one writer and many readers", "" },

                { "run-many-writers", '2', POPT_ARG_NONE, &ManyWriters, 0,
                  "run a test with many writers and many readers", "" },
                
                { "run-simple-client", 0, POPT_ARG_NONE, &SimpleClient, 0,
                  "used by run-simple to make a listening client", "" },

                { "run-simple-serv", 0, POPT_ARG_NONE, &SimpleServer, 0,
                  "used by run-simple to make a writing server", "" },


                

                { "run-med-test", '3', POPT_ARG_NONE, &MedallionTest, 0,
                  "test cross proc notification of medallion updates", "" },
                
                { "run-med-monitor", 0, POPT_ARG_NONE, &MedallionMonitor, 0,
                  "create a client that monitors and logs changes to a medallion until USR2 sig", "" },

                { "run-med-update", 0, POPT_ARG_NONE, &MedallionUpdate, 0,
                  "update a medallion", "" },

                { "medallion", 0, POPT_ARG_STRING, &MedallionFileName_CSTR, 0,
                  "name of context supplying medallion to operate on (run-med-monitor|run-med-update)", "" },

                { "emblem", 0, POPT_ARG_STRING, &EmblemName_CSTR, 0,
                  "name of emblem to add/remove from medallion", "tape" },

                { "medallion-has", 0, POPT_ARG_NONE, &MedallionHas, 0,
                  "if present then add --emblem to --medallion otherwise remove --emblem", "" },
                
                
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

        BaseDir            = BaseDir_CSTR;
        UnitTestExeBaseDir = UnitTestExeBaseDir_CSTR;
        OutputFileName     = OutputFileName_CSTR;
        Shell::touch( BaseDir, true, true );

        struct sigaction newinth;
        newinth.sa_handler = sig_usr2_cb;
        sigemptyset(&newinth.sa_mask);
        newinth.sa_flags   = SA_RESTART;
        if( -1 == sigaction( SIGUSR2, &newinth, NULL))
        {
            cerr << "ERROR: signal handling is not active. " << endl;
            exit(2);
        }
        
        if( Simple )
            runtest_simple( 1, 3 );
        if( ManyWriters )
            runtest_simple( 3, 6 );
        if( SimpleClient )
            runtest_saveInputMessages();
        if( SimpleServer )
            runtest_sendSimpleMessages();


        if( MedallionMonitor )
            runtest_monitorMedallion( MedallionFileName_CSTR );
        if( MedallionUpdate )
            runtest_updateMedallion( MedallionFileName_CSTR,
                                     EmblemName_CSTR,
                                     MedallionHas );
        if( MedallionTest )
            runtest_medallionNotification();

        
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
