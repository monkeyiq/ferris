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

    $Id: ferris-internal-metadata-broker-direct-client.cpp,v 1.8 2010/09/24 21:31:18 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#define NEW_QBUS_IMPL 1

#include "Ferris/MetadataServer_private.hh"
#include <Ferris/FerrisQt_private.hh>

#include <popt.h>

#include <iostream>
#include <string>
#include <list>

#include <QtDBus>
#include <QDBusArgument>
#include <QCoreApplication>

//#include "broker_interface.h"
#include "broker_interface.cpp"

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

static const char* DBUS_SERVER_NAME = "com.libferris.Metadata.Broker";
static const char* DBUS_SERVER_PATH = "/com/libferris/Metadata";

int RETURN_CODE = 0;
stringlist_t srcs;


namespace Ferris
{
    class MetadataBrokerClientApp : public QCoreApplication
    {
        Q_OBJECT;
        int m_getCount;
        
    public:

        MetadataBrokerClientApp( int & argc, char ** argv )
            :
            QCoreApplication( argc, argv ),
            m_getCount(0)
        {
        }

        void incrGetCount()
        {
            m_getCount++;
            if( m_getCount == srcs.size() )
            {
                cerr << "processing complete..." << endl;
                exit(0);
            }
            
        }
        
    public slots:


        void onAsyncGetFailed(int reqid, const QString &earl, int eno, const QString &ename, const QString &edesc)
        {
            cerr << "onAsyncGetFailed() "
                 << " earl:" << tostr(earl)
                 << " ename:" << tostr(ename)
                 << " edesc:" << tostr(edesc)
                 << endl;
            incrGetCount();
        }
        
        void onAsyncGetResult(int reqid, const QString &earl, const QString &name, const QByteArray &value)
        {
            cerr << "onAsyncGetResult() req:" << reqid
                 << " earl:" << tostr(earl)
                 << " rdn:" << tostr(name)
                 << " value:" << tostr(value)
                 << endl;
            incrGetCount();
        }
        
        

        void onAsyncPutCommitted(int reqid, const QString &earl, const QString &name)
        {
            cerr << "onAsyncPutCommitted() req:" << reqid
                 << " earl:" << tostr(earl)
                 << " rdn:" << tostr(name)
                 << endl;
            incrGetCount();
        }
        
        void onAsyncPutFailed(int reqid, const QString &earl, int eno, const QString &ename, const QString &edesc )
        {
            cerr << "onAsyncPutFailed() "
                 << " earl:" << tostr(earl)
                 << " ename:" << tostr(ename)
                 << " edesc:" << tostr(edesc)
                 << endl;
            incrGetCount();
        }
    };
};



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


int main( int argc, const char** argv )
{
    unsigned long ShowVersion        = 0;
    unsigned long UseSyncAPI         = 0;
    const char*   Metadata_CSTR      = 0;

    struct poptOption optionsTable[] = {

        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },

        { "metadata", 'a', POPT_ARG_STRING, &Metadata_CSTR, 0,
          "metadata to extract", 0 },

        { "sync", 's', POPT_ARG_NONE, &UseSyncAPI, 0,
          "use the sync API to read the value", 0 },
        
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
             << "version: $Id: ferris-internal-metadata-broker-direct-client.cpp,v 1.8 2010/09/24 21:31:18 ben Exp $\n"
             << "Written by Ben Martin, aka monkeyiq" << endl
             << endl
             << "Copyright (C) 2007 Ben Martin" << endl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }


    while( const char* RootNameCSTR = poptGetArg(optCon) )
    {
        string RootName = RootNameCSTR;
        srcs.push_back( RootName );
    }
    
    string rdn = Metadata_CSTR;
    if( UseSyncAPI )
    {
        cout << "using blocking API..." << endl;
    }
    
    try
    {
        MetadataBrokerClientApp* app = new MetadataBrokerClientApp( argc, (char**)argv );
        broker* client = new broker(
            DBUS_SERVER_NAME, DBUS_SERVER_PATH,
            QDBusConnection::sessionBus(), app );

        // {
        //     bool rc = QDBusConnection::sessionBus().connect(
        //         DBUS_SERVER_NAME, DBUS_SERVER_PATH, DBUS_SERVER_NAME, "asyncGetResult",
        //         app, SLOT(onAsyncGetResult(int, const QString&, const QString&, const QByteArray& )));
        //     // const QString & service, const QString & path, const QString & interface,
        //     // const QString & name, QObject * receiver, const char * slot );
        //     cerr << "rc:" << rc << endl;
        // }
        
    
        QObject::connect(
            client, SIGNAL(asyncGetFailed(int, const QString &, int , const QString &, const QString &)),
            app, SLOT(onAsyncGetFailed(int, const QString &, int , const QString &, const QString &)));
        QObject::connect(
            client, SIGNAL(asyncGetResult(int, const QString&, const QString&, const QByteArray& )),
            app, SLOT(onAsyncGetResult(int, const QString&, const QString&, const QByteArray& )));
        QObject::connect(
            client, SIGNAL(asyncPutFailed(int, const QString &, int , const QString &, const QString &)),
            app, SLOT(onAsyncPutFailed(int, const QString &, int , const QString &, const QString &)));
        QObject::connect(
            client, SIGNAL(asyncPutCommitted(int, const QString&, const QString& )),
            app, SLOT(onAsyncPutCommitted(int, const QString&, const QString& )));

        cerr << "connecting to DBUS_SERVER_NAME:" << DBUS_SERVER_NAME << endl;
        cerr << "connecting to DBUS_SERVER_PATH:" << DBUS_SERVER_PATH << endl;
        
        stringlist_t::iterator srcsiter = srcs.begin();
        stringlist_t::iterator srcsend  = srcs.end();
        for( ; srcsiter != srcsend ; ++srcsiter )
        {
            string earl = *srcsiter;

            if( UseSyncAPI )
            {
                cout << Ferris::syncMetadataServerGet( earl, rdn ) << endl;
            }
            else
            {
                cout << "rand():" << client->Random() << endl;
                cout << "rand():" << client->Random() << endl;
                cout << "rand():" << client->Random() << endl;

                cout << "getting earl:" << earl << endl;
                cout << client->asyncGet( earl.c_str(), rdn.c_str() ) << endl;
            }
        }

        if( !UseSyncAPI )
        {
            app->exec();
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

#include "ferris-internal-metadata-broker-direct-client.moc"
