/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris internal metadata broker
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

    $Id: ferris-internal-metadata-broker.cpp,v 1.8 2010/09/24 21:31:18 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "metadata-clients-common.hh"

#include <Ferris/FerrisEAGeneratorPlugin_private.hh>
#include <Ferris/Image_private.hh>

#include "worker_interface.h"
//#include "worker_interface.cpp"
//#include "broker_adaptor.h"
//#include "broker_adaptor.cpp"

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ferris-internal-metadata-broker";

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
static const char* WORKER_OBJECT = "com.libferris.Metadata.Worker";
static string WORKER_PREFIX = (string)WORKER_OBJECT + ".";

int RETURN_CODE = 0;
bool running = true;

namespace Ferris
{
    class MetadataBrokerDispatchInformation
        :
        public Handlable
    {
        typedef map< string, stringlist_t > m_plugins_t;

        string getEAGeneratorName( const string& earl,
                                   const string& rdn,
                                   bool writable )
            {
                LG_MDSERV_D << "getEAGeneratorName(called) earl:" << earl
                            << " rdn:" << rdn
                            << " writable:" << writable
                            << endl;
                string ret = "";

                try
                {
                    fh_context ctx = Resolve( earl );
                    LG_MDSERV_D << "resolve()ed ctx:" << ctx->getURL() << endl;
                    
                    StaticEAGenFactorys_t fac;
                    getOutOfProcess_EAGeneratorsStaticFactories( fac );

                    LG_MDSERV_D << "Looking at factories - start... fac.sz:" << fac.size() << endl;
                    {
                        StaticEAGenFactorys_t::iterator end = fac.end();
                        for( StaticEAGenFactorys_t::iterator fi = fac.begin(); fi != end; ++fi )
                        {
                            Context::fh_MatchedEAGeneratorFactory f = *fi;
//                             if( f->hasState() )
//                             {
//                                 f = f->clone();
//                             }

                            if( f->hasInterest( ctx ) )
                            {
                                if( StaticGModuleMatchedEAGeneratorFactory* fd =
                                    dynamic_cast<StaticGModuleMatchedEAGeneratorFactory*>(GetImpl( f )))
                                {
                                    string sn = fd->getShortName();
                                    LG_MDSERV_D << "getEAGeneratorName(called) earl:" << earl
                                                << " rdn:" << rdn
                                                << " writable:" << writable
                                                << " sn:" << sn
                                                << endl;
                                    return sn;
                                }
                            }
                        }
                    }
                }
                catch( exception& e )
                {
                    LG_MDSERV_I << "getEAGeneratorName(called) earl:" << earl
                                << " rdn:" << rdn
                                << " writable:" << writable
                                << " e:" << e.what()
                                << endl;
                }
                
                return ret;
            }
        
        string getImageEAGeneratorName( const string& earl,
                                        const string& rdn,
                                        bool writable )
            {
                LG_MDSERV_D << "getImageEAGeneratorName(called) earl:" << earl
                            << " rdn:" << rdn
                            << " writable:" << writable
                            << endl;

                if( isImageMetadataAttribute( rdn ) )
                {
                    LG_MDSERV_D << "getImageEAGeneratorName(not image metadata) earl:" << earl
                                << " rdn:" << rdn
                                << " writable:" << writable
                                << endl;
                    return "";
                }
                
                
                string ret = "";
                string nameExtension = "";
                string::size_type pos = earl.rfind('.');
                if( string::npos != pos )
                {
                    nameExtension = earl.substr( pos+1 );
                }
                nameExtension = tolowerstr()( nameExtension );

                LG_MDSERV_D << "getImageEAGeneratorName() earl:" << earl
                            << " nameExtension:" << nameExtension
                            << " writable:" << writable
                            << endl;
                
                if( !nameExtension.empty() )
                {
                    s_ImageEAGeneratorsExtensionToShortName_t& genmap = getOutOfProcess_ImageEAGeneratorsExtensionToShortName();
                    s_ImageEAGeneratorsExtensionToShortName_t::iterator giter = genmap.find( nameExtension );

                    LG_MDSERV_D << "getImageEAGeneratorName() earl:" << earl
                                << " nameExtension:" << nameExtension
                                << " writable:" << writable
                                << " genmap.sz:" << genmap.size()
                                << " found:" << ( giter != genmap.end() )
                                << endl;
                    
                    if( giter != genmap.end() )
                    {
                        string pshortname = giter->second.first;
                        bool   pwritable  = giter->second.second;

                        if( !writable || (writable && pwritable) )
                            ret = pshortname;
                    }
                }
                
                return ret;
            }
        
        
    public:
        MetadataBrokerDispatchInformation()
            {
            }
        ~MetadataBrokerDispatchInformation()
            {
            }
        
        string getServiceName( string& earl, const string& name, bool writable = false )
            {
                LG_MDSERV_D << "getServiceName() earl:" << earl << " name:" << name << " writable:" << writable << endl;
                
                string ret = "";
                ret = getImageEAGeneratorName( earl, name, writable );
                if( !ret.empty() )
                {
                    LG_MDSERV_D << "getServiceName(img) earl:" << earl
                                << " name:" << name
                                << " writable:" << writable
                                << " ret:" << ret
                                << endl;
                    return WORKER_PREFIX + ret;
                }


                ret = getEAGeneratorName( earl, name, writable );
                if( !ret.empty() )
                {
                    LG_MDSERV_D << "getServiceName(ea) earl:" << earl
                                << " name:" << name
                                << " writable:" << writable
                                << " ret:" << ret
                                << endl;
                    return WORKER_PREFIX + ret;
                }
                

                LG_MDSERV_D << "getServiceName(done) earl:" << earl
                            << " name:" << name
                            << " writable:" << writable
                            << " ret:" << ret
                            << endl;
                return ret;
            }

        typedef map< std::string, worker* > m_workerCache_t;
        m_workerCache_t m_workerCache;
        
        worker* getWorker( string& earl, const string& name, bool writable = false )
        {
            string workerName = getServiceName( earl, name, writable );
            LG_MDSERV_D << "getWorker() earl:" << earl
                        << " rdn:" << name
                        << " workerName:" << workerName
                        << endl;
            m_workerCache_t::iterator iter = m_workerCache.find( workerName );
            if( iter != m_workerCache.end() )
            {
                worker* client = iter->second;
                LG_MDSERV_D << "getWorker() cache for worker:" << workerName
                            << " client:" << client << endl;
                return client;
            }
            
            worker* client = new worker(
                workerName.c_str(), DBUS_SERVER_PATH,
                QDBusConnection::sessionBus(), 0 );
            LG_MDSERV_D << "getWorker() client:" << client << endl;
            m_workerCache.insert( make_pair( workerName, client ));
            return client;
        }
        

    };
    FERRIS_SMARTPTR( MetadataBrokerDispatchInformation, fh_MetadataBrokerDispatchInformation );
};


namespace Ferris
{
    typedef long reqid_t;


    class GetOrPut_QDBusPendingCallWatcher
        : public QDBusPendingCallWatcher
    {
    public:

        GetOrPut_QDBusPendingCallWatcher( reqid_t _reqid,
                                          string _earl,
                                          string _attr,
                                          const QDBusPendingCall & call,
                                          QObject * parent = 0 )
            :
            QDBusPendingCallWatcher( call, parent ),
            reqid( _reqid ),
            earl( _earl ),
            attr( _attr )
        {
        }
        
        reqid_t reqid;
        string earl;
        string attr;
    };
    
    
    class MetadataBroker : public QDBusAbstractAdaptor
    {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "com.libferris.Metadata.Broker");
        ExitOnIdle idler;

        reqid_t m_nextReqID;
        fh_MetadataBrokerDispatchInformation m_mdDispatchInfo;
        
    public:
        // typedef QList< QDBusPendingReply<QByteArray> > m_pendingGetCalls_t;
        // m_pendingGetCalls_t m_pendingGetCalls;

    public slots:

        void asyncGetFireReplySlot(QDBusPendingCallWatcher *bcall)
        {
            LG_MDSERV_D << "MetadataBroker::asyncGetFireReplySlot()" << endl;
            
            QDBusPendingReply<QByteArray> reply = *bcall;
            GetOrPut_QDBusPendingCallWatcher* call
                = dynamic_cast<GetOrPut_QDBusPendingCallWatcher*>(bcall);

            LG_MDSERV_D << "MetadataBroker::asyncGetFireReplySlot() bcall:" << toVoid(bcall) << endl;
            LG_MDSERV_D << "MetadataBroker::asyncGetFireReplySlot()  call:" << toVoid(call) << endl;
            
            reqid_t reqid = call->reqid;
            string earl = call->earl;
            string attr = call->attr;
            LG_MDSERV_D << "MetadataBroker::asyncGetFireReplySlot() reqid:" << reqid
                        << " attr:" << attr
                        << " earl:" << earl
                        << endl;
            
            if( reply.isError() )
            {
                LG_MDSERV_D << "asyncGetFireReply(error)" << endl;
                QDBusMessage m = reply.reply();
                QString ename = m.errorName();
                QString emsg = m.errorMessage();
                LG_MDSERV_D << "asyncGetFireReply(error) name:" << tostr(ename) << endl;
                LG_MDSERV_D << "asyncGetFireReply(error)  msg:" << tostr(emsg) << endl;
                int ecode = 1;
                // emit asyncGetFailed( reqid, earl.c_str(),
                //                      ecode, ename, emsg );

                QDBusMessage sig = QDBusMessage::createSignal(
                    "/com/libferris/Metadata",
                    "com.libferris.Metadata.Broker",
                    "asyncGetFailed" );
                QList<QVariant> arguments;
                arguments << qVariantFromValue((int)reqid)
                          << qVariantFromValue(QString(earl.c_str()))
                          << qVariantFromValue((int)ecode)
                          << qVariantFromValue(ename)
                          << qVariantFromValue(emsg);
                sig.setArguments( arguments );
                bool rc = QDBusConnection::sessionBus().send( sig );
                LG_MDSERV_D << "asyncGetFireReply(failed emit) hand sig rc:" << rc << endl;
            }
            else
            {
                QByteArray ba = reply.argumentAt<0>();

                LG_MDSERV_D << "asyncGetFireReply(ok emit)"
                            << " reqid:" << reqid
                            << " attr:" << attr
                            << " earl:" << earl
                            << " value:" << tostr(ba)
                            << endl;

//                emit asyncGetResult( (int)reqid, earl.c_str(), attr.c_str(), ba );

                QDBusMessage sig = QDBusMessage::createSignal(
                    "/com/libferris/Metadata",
                    "com.libferris.Metadata.Broker",
                    "asyncGetResult" );
                QList<QVariant> arguments;
                arguments << qVariantFromValue((int)reqid)
                          << qVariantFromValue(QString(earl.c_str()))
                          << qVariantFromValue(QString(attr.c_str()))
                          << qVariantFromValue(ba);
                sig.setArguments( arguments );
                bool rc = QDBusConnection::sessionBus().send( sig );
                LG_MDSERV_D << "asyncGetFireReply(ok emit) hand sig rc:" << rc << endl;
            }

            delete bcall;
        }
        

        void asyncPutFireReplySlot(QDBusPendingCallWatcher *bcall)
        {
            LG_MDSERV_D << "MetadataBroker::asyncGetFireReplySlot()" << endl;
            
            QDBusPendingReply<QByteArray> reply = *bcall;
            GetOrPut_QDBusPendingCallWatcher* call
                = dynamic_cast<GetOrPut_QDBusPendingCallWatcher*>(bcall);

            LG_MDSERV_D << "MetadataBroker::asyncGetFireReplySlot() bcall:" << toVoid(bcall) << endl;
            LG_MDSERV_D << "MetadataBroker::asyncGetFireReplySlot()  call:" << toVoid(call) << endl;
            
            reqid_t reqid = call->reqid;
            string earl = call->earl;
            string attr = call->attr;

            if( reply.isError() )
            {
                QDBusMessage m = reply.reply();
                QString ename = m.errorName();
                QString emsg = m.errorMessage();
                LG_MDSERV_D << "asyncGetFireReply(error) name:" << tostr(ename) << endl;
                LG_MDSERV_D << "asyncGetFireReply(error)  msg:" << tostr(emsg) << endl;
                int ecode = 1;

                if( ename == "org.freedesktop.DBus.Error.NoReply" )
                {
                }
//                emit asyncPutFailed( reqid, earl, ecode, ename, edesc );

                QDBusMessage sig = QDBusMessage::createSignal(
                    "/com/libferris/Metadata",
                    "com.libferris.Metadata.Broker",
                    "asyncPutFailed" );
                QList<QVariant> arguments;
                arguments << qVariantFromValue((int)reqid)
                          << qVariantFromValue(QString(earl.c_str()))
                          << qVariantFromValue((int)ecode)
                          << qVariantFromValue(ename)
                          << qVariantFromValue(emsg);
                sig.setArguments( arguments );
                bool rc = QDBusConnection::sessionBus().send( sig );
                LG_MDSERV_D << "asyncPutFireReplySlot(failed emit) hand sig rc:" << rc << endl;
            }
            else
            {
                LG_MDSERV_D << "asyncPutCommitted(ok emit)"
                            << " reqid:" << reqid
                            << " attr:" << attr
                            << " earl:" << earl
                            << endl;


//                emit asyncPutCommitted( reqid, earl, attr );

                QDBusMessage sig = QDBusMessage::createSignal(
                    "/com/libferris/Metadata",
                    "com.libferris.Metadata.Broker",
                    "asyncPutCommitted" );
                QList<QVariant> arguments;
                arguments << qVariantFromValue((int)reqid)
                          << qVariantFromValue(QString(earl.c_str()))
                          << qVariantFromValue(QString(attr.c_str()));
                sig.setArguments( arguments );
                bool rc = QDBusConnection::sessionBus().send( sig );
                LG_MDSERV_D << "asyncPutCommitted(ok emit) hand sig rc:" << rc << endl;
            }

            delete bcall;
        }
        
        
    public:
    
        MetadataBroker( QCoreApplication* parent = 0 );
        ~MetadataBroker();

        bool pingWorker( worker* client, const QDBusMessage &message );
                                                                      
    public slots:
        
        int Random()
        {
            idler.resetIdleTimer();
            LG_MDSERV_D << "random()" << endl;

            static int v = 1;
            int ret = v++;
            return ret;
        }
        
        int asyncGet(const QString &earl, const QString &name, const QDBusMessage &message);
        int asyncPut(const QString &earl, const QString &name, const QByteArray &value, const QDBusMessage &message );

    signals:
        void asyncGetFailed(int reqid, const QString &earl, int eno, const QString &ename, const QString &edesc);
        void asyncGetResult(int reqid, const QString &earl, const QString &name, const QByteArray &value);
        void asyncPutCommitted(int reqid, const QString &earl, const QString &name);
        void asyncPutFailed(int reqid, const QString &earl, int eno, const QString &ename, const QString &edesc);
        
    };
    
    MetadataBroker::MetadataBroker( QCoreApplication* parent )
        :
        QDBusAbstractAdaptor( parent ),
        idler( parent ),
        m_nextReqID( 10 ),
        m_mdDispatchInfo( new MetadataBrokerDispatchInformation() )
    {
        idler.resetIdleTimer();
        LG_MDSERV_D << "MetadataBroker()" << endl;
        setAutoRelaySignals( true );
    }

    MetadataBroker::~MetadataBroker()
    {
        LG_MDSERV_D << "~MetadataBroker()" << endl;
    }

    bool
    MetadataBroker::pingWorker( worker* client, const QDBusMessage &message )
    {
        bool ret = true;
        
        try
        {
            LG_MDSERV_D << "MetadataBroker::pingWorker()" << endl;
            int rc = client->ping();
            LG_MDSERV_D << "MetadataBroker::pingWorker(ok) rc:" << rc << endl;
        }
        catch( exception& e )
        {
            LG_MDSERV_D << "MetadataBroker::pingWorker(failed)" 
                        << " reason:" << e.what()
                        << endl;
            dbus_error( message, "can't ping worker..." );
            return false;
        }
        return ret;
    }
    
    
    int
    MetadataBroker::asyncGet(const QString &qearl, const QString &qname, const QDBusMessage &message )
    {
        idler.resetIdleTimer();
        int timeout = 10000;
        std::string earl = tostr(qearl);
        std::string name = tostr(qname);
        int ret = ++m_nextReqID;
        LG_MDSERV_D << "MetadataBroker::asyncGet(called) earl:" << earl << endl;


        string workerName = m_mdDispatchInfo->getServiceName( earl, name, false );
        worker* client = m_mdDispatchInfo->getWorker( earl, name, false );
        LG_MDSERV_D << "MetadataBroker::asyncGet() earl:" << earl
                    << " rdn:" << name << " workerName:" << workerName
                    << endl;

        if( !pingWorker( client, message ))
        {
            return 0;
        }
        
        LG_MDSERV_D << "MetadataBroker::asyncGet(calling async_send) earl:" << earl
                    << " rdn:" << name << " workerName:" << workerName
                    << endl;
        
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(qearl) << qVariantFromValue(qname);
        QDBusPendingReply<QByteArray> reply = client->asyncCallWithArgumentList("get",argumentList);

//        m_pendingGetCalls.push_back( reply );
        GetOrPut_QDBusPendingCallWatcher* watcher = new GetOrPut_QDBusPendingCallWatcher(
            ret, earl, name, reply, 0 );

        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(asyncGetFireReplySlot(QDBusPendingCallWatcher*)));
        
        LG_MDSERV_D << "MetadataBroker::asyncGet(done) earl:" << earl
                    << " rdn:" << name << " workerName:" << workerName
                    << endl;
        return ret;
    }
    
    int MetadataBroker::asyncPut(const QString &qearl,
                                 const QString &qname,
                                 const QByteArray &qvalue,
                                 const QDBusMessage &message )
    {
        idler.resetIdleTimer();
        std::string earl = tostr(qearl);
        std::string name = tostr(qname);
        std::string value = tostr(qvalue);
        int ret = ++m_nextReqID;
        LG_MDSERV_D << "MetadataBroker::asyncPut(called) earl:" << earl << endl;

        string workerName = m_mdDispatchInfo->getServiceName( earl, name, false );
        worker* client = m_mdDispatchInfo->getWorker( earl, name, false );
        LG_MDSERV_D << "MetadataBroker::asyncPut() earl:" << earl
                    << " rdn:" << name << " workerName:" << workerName
                    << endl;

        if( !pingWorker( client, message ))
        {
            return 0;
        }
        
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(qearl)
                     << qVariantFromValue(qname)
                     << qVariantFromValue(qvalue);
        QDBusPendingReply<QByteArray> reply = client->asyncCallWithArgumentList("put",argumentList);
        GetOrPut_QDBusPendingCallWatcher* watcher = new GetOrPut_QDBusPendingCallWatcher(
            ret, earl, name, reply, 0 );

        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(asyncPutFireReplySlot(QDBusPendingCallWatcher*)));
        
        LG_MDSERV_D << "MetadataBroker::PutGet(done) earl:" << earl
                    << " rdn:" << name << " workerName:" << workerName
                    << endl;
        return ret;
    }
};



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


int main( int argc, const char** argv )
{
    setForceOutOfProcessMetadataOff( true );
    
    unsigned long ShowVersion        = 0;
    const char*   PluginNames_CSTR   = 0;
    const char*   FormatDisplayCSTR  = 0;

    struct poptOption optionsTable[] = {

        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },

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

    if (argc < 1 )
    {
        cerr << "Arguments incorrect!" << endl;
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }
    
    if( ShowVersion )
    {
        cout << "ferris-internal-metadata-worker" << nl
             << "version: $Id: ferris-internal-metadata-broker.cpp,v 1.8 2010/09/24 21:31:18 ben Exp $\n"
             << "Written by Ben Martin, aka monkeyiq" << nl
             << nl
             << "Copyright (C) 2010 Ben Martin" << nl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }

    
    try
    {
        LG_MDSERV_D << "broker starting up..." << endl;
        QCoreApplication* app = new QCoreApplication( argc, (char**)argv );
        app->setApplicationName("com.libferris.broker");

        string DBusServerName = "com.libferris.Metadata.Broker";
        // QDBusConnection conn = QDBusConnection::connectToBus ( QDBusConnection::SessionBus,
        //                                                        DBusServerName.c_str() );
        QDBusConnection conn = QDBusConnection::sessionBus();
        conn.registerService( DBUS_SERVER_NAME );


        MetadataBroker* obj = new MetadataBroker( app );
//        broker* b = new broker(obj);
        conn.registerObject( "/com/libferris/Metadata", obj,
                             QDBusConnection::ExportAllSlots
                             | QDBusConnection::ExportAllSignals
                             | QDBusConnection::ExportAllProperties
                             | QDBusConnection::ExportAllContents );
        cerr << "running..." << endl;
        app->exec();
    }
    catch( NoSuchContextClass& e )
    {
        LG_MDSERV_W << "broker error:" << e.what() << endl;
        cerr << "e:" << e.what() << endl;
        exit(1);
    }
    catch( exception& e )
    {
        LG_MDSERV_W << "broker error:" << e.what() << endl;
        cerr << "cought:" << e.what() << endl;
        exit(1);
    }
        
    LG_MDSERV_D << "broker exiting..." << endl;
    poptFreeContext(optCon);
    cout << flush;
    return RETURN_CODE;
}

#include "ferris-internal-metadata-broker.moc"
