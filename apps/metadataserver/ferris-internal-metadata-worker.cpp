/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris internal metadata worker
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

    $Id: ferris-internal-metadata-worker.cpp,v 1.6 2010/09/24 21:31:18 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "metadata-clients-common.hh"
//#include "worker_adaptor.h"

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ferris-internal-metadata-worker";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int RETURN_CODE = 0;

namespace Ferris
{
    class MetadataWorker : public QDBusAbstractAdaptor
    {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "com.libferris.Metadata.Worker");
        ExitOnIdle idler;

        typedef stringset_t m_pluginNames_t;
        m_pluginNames_t m_pluginNames;
        
    public:
    
        MetadataWorker( const stringset_t& pluginNames, QObject *parent = 0 );
        ~MetadataWorker();

    public slots:
        int ping();
        QByteArray get(const QString &earl, const QString &name, const QDBusMessage &message );
        void set(const QString &earl, const QString &name, const QByteArray &value, const QDBusMessage &message);
    };

    MetadataWorker::MetadataWorker( const stringset_t& pluginNames, QObject *parent )
        :
        QDBusAbstractAdaptor( parent ),
        idler( (QCoreApplication*)parent ),
        m_pluginNames( pluginNames )
    {
        idler.resetIdleTimer();
        LG_MDSERV_D << "MetadataWorker()" << endl;
    }

    MetadataWorker::~MetadataWorker()
    {
        LG_MDSERV_D << "~MetadataWorker()" << endl;
    }

    QByteArray
    MetadataWorker::get(const QString &qearl, const QString &qeaname, const QDBusMessage &message )
    {
        idler.resetIdleTimer();

        cerr << "MetadataWorker::get() " <<  endl;
        LG_MDSERV_I << "MetadataWorker::get() " <<  endl;

        std::string earl = tostr(qearl);
        std::string eaname = tostr(qeaname);
        
        LG_MDSERV_I << "MetadataWorker::get() url:" << earl << " eaname:" << eaname << endl;
        
        stringstream ss;
        try
        {
            fh_context c = Resolve( earl );

            LG_MDSERV_D << "MetadataWorker::get() calling getStrAttr()."
                        << " url:" << earl << " eaname:" << eaname << endl;
            
            std::string v = c->getStrAttr_UsingRestrictedPlugins( eaname, m_pluginNames );
//            ss << v;

            LG_MDSERV_D << "MetadataWorker::get(ok) url:" << earl
                        << " eaname:" << eaname
                        << " ret.sz:" << v.size()
                        << endl;
            if( eaname != "rgba-32bpp" )
            {
                LG_MDSERV_D << "MetadataWorker::get(ok) url:" << earl
                            << " eaname:" << eaname
                            << " ret:" << v
                            << endl;
            }

            QByteArray ba = toba( v );
            return ba;
        }
        catch( exception& e )
        {
            ss << "MetadataExtractionError:" << e.what();
            cerr << ss.str() << endl;
            LG_MDSERV_W << "MetadataWorker::get(fail) url:" << earl
                        << " eaname:" << eaname
                        << " e:" << ss.str()
                        << endl;
            dbus_error( message, ss );
        }
        return QByteArray();
    }


    
    void
    MetadataWorker::set(const QString &earl,
                        const QString &name,
                        const QByteArray &value,
                        const QDBusMessage &message )
    {
        idler.resetIdleTimer();

        LG_MDSERV_ER << "MetadataWorker::set() not implemented!" << endl;
        dbus_error( message, "MetadataWorker::set() not implemented yet!" );
    }
    
    int
    MetadataWorker::ping()
    {
        idler.resetIdleTimer();

        cerr << "MetadataWorker::ping()" << endl;
        LG_MDSERV_D << "MetadataWorker::ping()" << endl;

        int t = time( 0 );
        return t;
    }
};




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

using namespace Ferris;

int main( int argc, const char** argv )
{
    setForceOutOfProcessMetadataOff( true );

    unsigned long ShowVersion        = 0;
    const char*   PluginNames_CSTR   = 0;
    const char*   FormatDisplayCSTR  = 0;
    const char*   DBusServerName_CSTR = 0;
    
    struct poptOption optionsTable[] = {

        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },

        { "plugin-names", 0, POPT_ARG_STRING, &PluginNames_CSTR, 0,
          "comma seperated list of plugins to use for metadata handling", 0 },

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
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* ...");

    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {}

    if (argc < 1 || !PluginNames_CSTR )
    {
        cerr << "Arguments incorrect!" << endl;
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }
    
    if( ShowVersion )
    {
        cout << "ferris-internal-metadata-worker" << nl
             << "version: $Id: ferris-internal-metadata-worker.cpp,v 1.6 2010/09/24 21:31:18 ben Exp $\n"
             << "ferris   version: " << VERSION << nl
             << "Written by Ben Martin, aka monkeyiq" << nl
             << nl
             << "Copyright (C) 2007 Ben Martin" << nl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }

    string DBusServerName = "com.libferris.Metadata.Worker";
    if( DBusServerName_CSTR )
        DBusServerName = DBusServerName_CSTR;
    
        
    stringset_t pluginNames;
    Util::parseCommaSeperatedList( PluginNames_CSTR, pluginNames );

    LG_MDSERV_D << "about to start new worker process" << endl;
    LG_MDSERV_D << "Registering as:" << DBusServerName << endl;
    
    try
    {
        QCoreApplication* app = new QCoreApplication( argc, (char**)argv );
        app->setApplicationName("com.libferris");

        QDBusConnection conn = QDBusConnection::connectToBus ( QDBusConnection::SessionBus,
                                                               DBusServerName.c_str() );
        conn.registerService( DBusServerName.c_str() );
        
        MetadataWorker* w = new MetadataWorker( pluginNames, app );
        conn.registerObject( "/com/libferris/Metadata", w,
                             QDBusConnection::ExportAllSlots
                             | QDBusConnection::ExportAllSignals
                             | QDBusConnection::ExportAllProperties
                             | QDBusConnection::ExportAllContents );
        cerr << "running..." << endl;
        app->exec();
    }
    catch( NoSuchContextClass& e )
    {
        cerr << "e:" << e.what() << endl;
        LG_MDSERV_W << "Worker process exiting with error:" << e.what() << endl;
        exit(1);
    }
    catch( exception& e )
    {
        cerr << "cought:" << e.what() << endl;
        LG_MDSERV_W << "Worker process exiting with error:" << e.what() << endl;
        exit(1);
    }
        
    poptFreeContext(optCon);
    cout << flush;
    LG_MDSERV_D << "Worker process exiting" << endl;
    return RETURN_CODE;
}

#include "ferris-internal-metadata-worker.moc"
