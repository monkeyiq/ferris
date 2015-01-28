/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris internal metadata worker direct client
    Copyright (C) 2007 Ben Martin

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

    $Id: ferris-internal-metadata-worker-direct-client.cpp,v 1.6 2010/09/24 21:31:18 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#define NEW_QBUS_IMPL 1

#include "Ferris/MetadataServer_private.hh"
#include <Ferris/FerrisQt_private.hh>

#include <popt.h>
#include <signal.h>

#include <iostream>
#include <string>
#include <list>

#include <QtDBus>
#include <QDBusArgument>
#include <QCoreApplication>

#include "worker_interface.h"
#include "worker_interface.cpp"

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ferris-internal-metadata-worker-direct-client";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static const char* DBUS_SERVER_NAME = "com.libferris.Metadata.Worker";
static const char* DBUS_SERVER_PATH = "/com/libferris/Metadata";
int RETURN_CODE = 0;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


int main( int argc, const char** argv )
{
    unsigned long ShowVersion        = 0;
    const char*   Metadata_CSTR      = 0;
    const char*   DBusServerName_CSTR = 0;

    struct poptOption optionsTable[] = {

        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },

        { "metadata", 'a', POPT_ARG_STRING, &Metadata_CSTR, 0,
          "metadata to extract", 0 },

        { "dbus-server-name", 0, POPT_ARG_STRING, &DBusServerName_CSTR, 0,
          "name to use for this dbus server object", 0 },
        
        /*
         * Standard Ferris options
         */
//        FERRIS_POPT_OPTIONS
        POPT_AUTOHELP
        POPT_TABLEEND
    };
    poptContext optCon;


    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* url1 url2 ...");

    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {}

    if (argc < 1 || !Metadata_CSTR )
    {
        cerr << "Arguments incorrect!" << endl;
//        poptPrintHelp(optCon, stderr, 0);
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }
    
    if( ShowVersion )
    {
        cout << "ferris-internal-metadata-worker" << endl
             << "version: $Id: ferris-internal-metadata-worker-direct-client.cpp,v 1.6 2010/09/24 21:31:18 ben Exp $\n"
             << "Written by Ben Martin, aka monkeyiq" << endl
             << endl
             << "Copyright (C) 2007 Ben Martin" << endl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }

    string DBusServerName = DBUS_SERVER_NAME;
    if( DBusServerName_CSTR )
        DBusServerName = string("") + DBUS_SERVER_NAME + "." + DBusServerName_CSTR;
    

    stringlist_t srcs;
    while( const char* RootNameCSTR = poptGetArg(optCon) )
    {
        string RootName = RootNameCSTR;
        srcs.push_back( RootName );
    }
    
    string rdn = Metadata_CSTR;
    
    try
    {
        QCoreApplication* app = new QCoreApplication( argc, (char**)argv );
        worker* client = new worker(
            DBusServerName.c_str(), DBUS_SERVER_PATH,
            QDBusConnection::sessionBus(), app );
    
        stringlist_t::iterator srcsiter = srcs.begin();
        stringlist_t::iterator srcsend  = srcs.end();
        for( ; srcsiter != srcsend ; ++srcsiter )
        {
            string earl = *srcsiter;

            QByteArray ba = client->get( earl.c_str(), rdn.c_str() );
            cout << tostr(ba) << endl;
        }
    }
    catch( exception& e )
    {
        cerr << "cought:" << e.what() << endl;
        exit(1);
    }
        
    poptFreeContext(optCon);
    cout << flush;
    return RETURN_CODE;
}


// ~std::cout
// <<
//     net::sf::witme::Libferris::Metadata::Worker::get(const DBus::String&,
//                                                      const DBus::String&)(((const DBus::String&)((const DBus::String*)(& earl))),
//                                                                           ((const DBus::String&)((const DBus::String*)(& rdn))))~
    
