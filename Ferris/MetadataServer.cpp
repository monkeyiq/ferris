/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

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

    $Id: MetadataServer.cpp,v 1.5 2010/09/24 21:30:55 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Ferris/Ferris.hh>

#include <glib.h>
#include <sigc++/sigc++.h>
#include <sigc++/slot.h>
#include <sigc++/object.h>
#include <sigc++/object_slot.h>

#include "MetadataServer_private.hh"
#include <Ferris/FerrisQt_private.hh>

//#include "DBusGlue/ferris-internal-metadata-broker-client-glue.h"
#include "DBusGlue/broker_interface.h"
#include "DBusGlue/broker_interface.cpp"
#include "DBusGlue/worker_interface.h"
#include "DBusGlue/worker_interface.cpp"


#include <iostream>



using namespace std;

namespace Ferris
{
    static const char* DBUS_SERVER_NAME = "com.libferris.Metadata.Broker";
    static const char* DBUS_SERVER_PATH = "/com/libferris/Metadata";


    namespace
    {
        class MetadataBrokerHandler : public QObject
        {
            Q_OBJECT;
        public:
            int m_getCount;
            int m_putCount;
            bool is_error;
            string errormsg;
            bool running;
            QByteArray value;
        

            MetadataBrokerHandler( QObject* parent = 0 )
                :
                QObject( parent ),
                m_getCount(0)
            {
            }

            void run()
                {
                    LG_MDSERV_D << "run() running:" << running << endl;
//                    dispatcher->enter();
//                    while( running )
//                        g_main_iteration( true );

                    while( running )
                    {
                        QCoreApplication::processEvents( QEventLoop::AllEvents, 20 );
                    }
                    LG_MDSERV_D << "run() completed running:" << running << endl;
                    
                }

        public slots:


            void onAsyncGetFailed(int reqid, const QString &earl, int eno, const QString &ename, const QString &edesc)
            {
                LG_MDSERV_D << "onAsyncGetFailed() "
                            << " earl:" << tostr(earl)
                            << " ename:" << tostr(ename)
                            << " edesc:" << tostr(edesc)
                            << endl;
                // cerr << "onAsyncGetFailed() "
                //      << " earl:" << tostr(earl)
                //      << " ename:" << tostr(ename)
                //      << " edesc:" << tostr(edesc)
                //      << endl;

                this->is_error = 1;
                this->value.clear();
                this->errormsg = tostr(edesc);
                running = false;
            }
        
            void onAsyncGetResult(int reqid, const QString &earl, const QString &name, const QByteArray &value)
            {
                LG_MDSERV_D << "onAsyncGetResult() req:" << reqid
                            << " earl:" << tostr(earl)
                            << " rdn:" << tostr(name)
                            << " value:" << tostr(value)
                            << endl;
                // cerr << "onAsyncGetResult() req:" << reqid
                //      << " earl:" << tostr(earl)
                //      << " rdn:" << tostr(name)
                //      << " value:" << tostr(value)
                //      << endl;
                    
                this->is_error = 0;
                this->value = value;
                this->errormsg = "";
                running = false;
            }
        
            void onAsyncPutCommitted(int reqid, const QString &earl, const QString &name)
            {
                cerr << "onAsyncPutCommitted() req:" << reqid
                     << " earl:" << tostr(earl)
                     << " rdn:" << tostr(name)
                     << endl;
                running = false;
            }
        
            void onAsyncPutFailed(int reqid, const QString &earl, int eno, const QString &ename, const QString &edesc )
            {
                cerr << "onAsyncPutFailed() "
                     << " earl:" << tostr(earl)
                     << " ename:" << tostr(ename)
                     << " edesc:" << tostr(edesc)
                     << endl;
                running = false;
            }
        

        
        };

    };

    

    std::string syncMetadataServerGet( const std::string& earl,
                                       const std::string& rdn )
    {
        LG_MDSERV_D << "syncMetadataServerGet()" << endl;

        ensureQApplication();
        
        LG_MDSERV_D << "done setting a dispatcher..." << endl;
        MetadataBrokerHandler handler( 0 );
        broker* client = new broker(
            DBUS_SERVER_NAME, DBUS_SERVER_PATH,
            QDBusConnection::sessionBus(), &handler );

        QObject::connect(
            client, SIGNAL(asyncGetFailed(int, const QString &, int , const QString &, const QString &)),
            &handler, SLOT(onAsyncGetFailed(int, const QString &, int , const QString &, const QString &)));
        QObject::connect(
            client, SIGNAL(asyncGetResult(int, const QString&, const QString&, const QByteArray& )),
            &handler, SLOT(onAsyncGetResult(int, const QString&, const QString&, const QByteArray& )));
        QObject::connect(
            client, SIGNAL(asyncPutFailed(int, const QString &, int , const QString &, const QString &)),
            &handler, SLOT(onAsyncPutFailed(int, const QString &, int , const QString &, const QString &)));
        QObject::connect(
            client, SIGNAL(asyncPutCommitted(int, const QString&, const QString& )),
            &handler, SLOT(onAsyncPutCommitted(int, const QString&, const QString& )));
        
        LG_MDSERV_D << "Calling asyncGet() earl:" << earl << " rdn:" << rdn << endl;
        handler.running = true;

        int rc = client->asyncGet( earl.c_str(), rdn.c_str() );
        LG_MDSERV_D << "Calling asyncGet() rc:" << rc << endl;

        LG_MDSERV_D << "Calling run()" << endl;
        handler.run();

        LG_MDSERV_D << "Have a reply is_error:" << handler.is_error << endl;
        if( handler.is_error )
        {
            stringstream ss;
            ss << "error:" << handler.errormsg;
            Throw_DBusException( ss.str(), 0 );
        }

        string ret( handler.value.begin(), handler.value.end() );
        return ret;
    }

    void syncMetadataServerPut( const std::string& earl,
                                const std::string& rdn,
                                const std::string& value )
    {
        LG_MDSERV_D << "FIXME!" << endl;
        Throw_DBusException( "not implemented", 0 );
    }

    /************************************************************/
    /************************************************************/
    /************************************************************/

    // static guint glib_idle_timeout_ID = 0;
    // static gint glib_idle_timeout_func(gpointer data)
    // {
    //     bool* running = (bool*)data;
    //     (*running) = false;
    // }

    // void resetup_glib_idle_to_close_callback_function( int interval, bool* running )
    // {
    //     if( glib_idle_timeout_ID  )
    //         g_source_remove( glib_idle_timeout_ID );
    //     glib_idle_timeout_ID = g_timeout_add_seconds(
    //         interval, GSourceFunc(glib_idle_timeout_func), 0 );
    // }
    
    /************************************************************/
    /************************************************************/
    /************************************************************/

    stringset_t& getImageMetadataAttributes()
    {
        static stringset_t* ret = 0;
        if( !ret )
        {
            ret = new stringset_t;
            string alist = "depth-per-color,depth,gamma,has-alpha,aspect-ratio,width,height,rgba-32bpp,megapixels";
            Util::parseCommaSeperatedList( alist, *ret );
        }
        return *ret;
    }

    bool isImageMetadataAttribute( const std::string& rdn )
    {
        return( getImageMetadataAttributes().end() == getImageMetadataAttributes().find( rdn ) );
    }
    
    s_ImageEAGeneratorsExtensionToShortName_t& getOutOfProcess_ImageEAGeneratorsExtensionToShortName()
    {
        static s_ImageEAGeneratorsExtensionToShortName_t* ret = 0;
        if( !ret )
        {
//            string shortNamesToUseString = "xine,imlib2,jpeg,magick,png";
            string shortNamesToUseString = getStrSubCtx( "~/.ferris/use-out-of-process-metadata-plugins",
                                                         "xine,imlib2,jpeg,magick,png" );

            ret = new s_ImageEAGeneratorsExtensionToShortName_t();
            s_ImageEAGeneratorsExtensionToShortName_t& all = getImageEAGeneratorsExtensionToShortName();
            stringset_t shortNamesToUse;
            Util::parseCommaSeperatedList( shortNamesToUseString, shortNamesToUse );

            s_ImageEAGeneratorsExtensionToShortName_t::iterator iter = all.begin();
            s_ImageEAGeneratorsExtensionToShortName_t::iterator    e = all.end();

            for( ; iter != e; ++iter )
            {
                string shortname = iter->second.first;
                if( shortNamesToUse.end() != shortNamesToUse.find( shortname ) )
                {
                    ret->insert( *iter );
                }
            }
            
        }
        return *ret;
    }

    const stringset_t&
    getOutOfProcess_EAGeneratorsStaticFactoriesShortNamesToUse()
    {
        string shortNamesToUseString = "xine,kde3_metadata";
        static stringset_t shortNamesToUse;
        Util::parseCommaSeperatedList( shortNamesToUseString, shortNamesToUse );
        cerr << "shortNamesToUse.sz:" << shortNamesToUse.size() << endl;
        return shortNamesToUse;
    }
    
    
    StaticEAGenFactorys_t&
    getOutOfProcess_EAGeneratorsStaticFactories( StaticEAGenFactorys_t& ret )
    {
        const stringset_t& shortNamesToUse = getOutOfProcess_EAGeneratorsStaticFactoriesShortNamesToUse();

        StaticEAGenFactorys_t& fac = getStaticLinkedEAGenFactorys();
        StaticEAGenFactorys_t::iterator end = fac.end();
        for( StaticEAGenFactorys_t::iterator fi = fac.begin(); fi != end; ++fi )
        {
            fh_StaticMatchedEAGeneratorFactory f = *fi;
            string sn = f->getShortName();
            cerr << "sn:" << sn << endl;
            if( shortNamesToUse.end() != shortNamesToUse.find( sn ) )
            {
                ret.push_back( f );
            }
        }

        cerr << "MetadataBrokerDispatchInformation() selected factories sz:"
             << ret.size() << endl;

        return ret;
    }

    bool isOutOfProcessMetadataAttribute( const std::string& rdn )
    {
        static stringset_t col;
        if( col.empty() )
        {
            string alist = "";

            // Xine EA
            alist += "width,height,bitrate,can-seek,video-channel-count,video-stream-count,video-bitrate,";
            alist += "video-bitrate-fourcc,xine-can-play-video,audio-channel-count,audio-bitrate,";
            alist += "audio-bitrate-fourcc,xine-can-play-audio,has-video,has-audio,title,comment,artist,";
            alist += "genre,album,year,";
            
            Util::parseCommaSeperatedList( alist, col );
        }

        return col.end() != col.find( rdn );
    }
    
    
}

#include "MetadataServer_moc_impl.cpp"
