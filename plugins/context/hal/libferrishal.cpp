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

    $Id: WebPhotosContext.cpp,v 1.5 2008/05/24 21:30:07 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include <Ferris/ContextPlugin.hh>

using namespace std;

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisQt_private.hh>

#include "DBusGlue/org_freedesktop_Hal_Device.h"
#include "DBusGlue/org_freedesktop_Hal_Manager.h"
#include "DBusGlue/org_freedesktop_Hal_Device_Volume.h"
#include "DBusGlue/org_freedesktop_Hal_Device.cpp"
#include "DBusGlue/org_freedesktop_Hal_Manager.cpp"
#include "DBusGlue/org_freedesktop_Hal_Device_Volume.cpp"


namespace Ferris
{
    class HalRootContext;


    std::string tostr( QDBusVariant v )
    {
        return tostr( v.variant() );
    }
    

    class FERRISEXP_DLLLOCAL HalDeviceContext
        :
        public StateLessEAHolder< HalDeviceContext, leafContext >
    {
        typedef HalDeviceContext _Self;
        typedef StateLessEAHolder< HalDeviceContext, leafContext > _Base;

        fdoHalDevice* m_hdev;
        
    public:

        HalDeviceContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }
        virtual ~HalDeviceContext()
            {
            }

//        ::DBus::Variant GetProperty( const ::DBus::String& key )
//        std::map< ::DBus::String, ::DBus::Variant > GetAllProperties()

        //////////////////////////////////////////////////////////////////////////////////////////////////
        // FIXME!!!!!!!!!!!!!!!!!!!!!
        static fdoHalDeviceVolume* cast_to_halvolume( fdoHalDevice* hdev )
        {
//            return (fdoHalDeviceVolume*)hdev;

            LG_HAL_D << "      path:" << tostr(hdev->path()) << endl;
            LG_HAL_D << "   service:" << tostr(hdev->service()) << endl;
            LG_HAL_D << " interface:" << tostr(hdev->interface()) << endl;
            LG_HAL_D << "    param1:" << "org.freedesktop.Hal" << endl;
            
            fdoHalDeviceVolume* ret = 0;
            ret = new fdoHalDeviceVolume( "org.freedesktop.Hal",
                                          hdev->path(), hdev->connection() );

            LG_HAL_D << "ret.  path:" << tostr(ret->path()) << endl;
            LG_HAL_D << "   service:" << tostr(ret->service()) << endl;
            LG_HAL_D << " interface:" << tostr(ret->interface()) << endl;

            return ret;
        }
        
#if 0
        static DBus::RefPtr< HalVolume > cast_to_halvolume( fh_haldevice hdev )
            {
                static string hasVolumeInterface = "interface name=\"org.freedesktop.Hal.Device.Volume\"";
                string intros = hdev->Introspect();
                if( string::npos != intros.find( hasVolumeInterface ) )
                {
                    HalVolume* v = new HalVolume( hdev->conn(), hdev->path());
                    return v;
                }
                return 0;
            }
#endif
        
        
        static fh_stringstream
        SL_getEjectStream( HalDeviceContext* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                LG_HAL_D << "SL_getEjectStream(1) rdn:" << ean << endl;

                
                fdoHalDeviceVolume* v = cast_to_halvolume( c->m_hdev );
                LG_HAL_D << "SL_getEjectStream(2) rdn:" << ean << endl;
                if( v )
                {
                    LG_HAL_D << "...     this device is also a volume...:" << c->getURL() << endl;
                    LG_HAL_D << " isValid:" << v->isValid() << endl;
                    
                    string extra_options = "";
                    QStringList olist;
                    olist << extra_options.c_str();
                    try
                    {
                        int rc = v->Eject( olist );
                        LG_HAL_D << "eject... rc:" << rc << endl;
                    }
                    catch( exception& e )
                    {
                        LG_HAL_D << "error:" << e.what() << endl;
                        cerr << e.what() << endl;
                        throw;
                    }
                    
                }
                return ss;
            }
        

        static fh_stringstream
        SL_getMountIOStream( HalDeviceContext* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                return ss;
            }

        static void
        SL_FlushMountData( HalDeviceContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
            {
                string mp;
                getline( ss, mp );
                
                fdoHalDeviceVolume* v = cast_to_halvolume( c->m_hdev );
                if( v )
                {
                    LG_HAL_D << "...     this device is also a volume...:" << c->getURL() << endl;
                    LG_HAL_D << " isValid:" << v->isValid() << endl;
                    LG_HAL_D << "mp:" << mp << endl;
                    
                    string fstype = "";
                    string extra_options = "";
                    QStringList olist;
//                    olist << extra_options.c_str();

                    try
                    {
                        int rc = v->Mount( mp.c_str(), fstype.c_str(), olist );
                        LG_HAL_D << "... rc:" << rc << endl;
                    }
                    catch( exception& e )
                    {
                        LG_HAL_D << "error:" << e.what() << endl;
                        throw;
                    }
                    
                }
            }

        void unmount()
            {
                LG_HAL_D << "unmount()" << endl;
                fdoHalDeviceVolume* v = cast_to_halvolume( m_hdev );
                if( v )
                {
                    LG_HAL_D << "...     this device is also a volume...:" << getURL() << endl;
                    
                    string extra_options = "";
                    QStringList olist;
                    olist << extra_options.c_str();

                    try
                    {
                        int rc = v->Unmount( olist );
                        LG_HAL_D << "... rc:" << rc << endl;
                    }
                    catch( exception& e )
                    {
                        LG_HAL_D << "error:" << e.what() << endl;
                        throw;
                    }
                }
            }
        
        static fh_stringstream
        SL_getUnMountIStream( HalDeviceContext* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ret;
                LG_HAL_D << "SL_getUnMountIStream" << endl;
                c->unmount();
                return ret;
            }
        static fh_stringstream
        SL_getUnMountIOStream( HalDeviceContext* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ret;
                LG_HAL_D << "SL_getUnMountIOStream" << endl;
                return ret;
            }

        static void
        SL_FlushNullData( HalDeviceContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
            {
                LG_HAL_D << "SL_FlushNullData" << endl;
                c->unmount();
            }
        
        
        
        static fh_stringstream
        SL_getDBusPropertyStream( HalDeviceContext* c, const std::string& ean, EA_Atom* atom )
            {
                LG_HAL_D << "SL_getDBusPropertyStream() rdn:" << ean << endl;
                LG_HAL_D << "SL_getDBusPropertyStream(2) rdn:" << ean << endl;
                
                fh_stringstream ss;
                try
                {
//                    c->m_hdev->GetProperty( ean );
                    QDBusVariant v = c->m_hdev->GetProperty( ean.c_str() );
                    LG_HAL_D << "SL_getDBusPropertyStream(3) rdn:" << ean << endl;
                    LG_HAL_D << "SL_getDBusPropertyStream() v:" << tostr(v) << endl;
                    if( ean == "storage.model" && tostr(v).empty() )
                    {
                        return SL_getEAFromHALParentStream( c, ean, atom );
                    }
                    ss << tostr(v);
                }
                catch( exception&e )
                {
                    cerr << "ERROR:" << e.what() << endl;
                }
                
                return ss;
            }

        typedef map< string, int > m_classInitCount_t;
        static m_classInitCount_t& getClassInitCount()
            {
                static m_classInitCount_t ret;
                return ret;
            }
        typedef map< string, string > m_classInitKey_t;
        static m_classInitKey_t& getClassInitKeys()
            {
                static m_classInitKey_t ret;
                ret[ "hal:///devices/usb" ] = "usb_device.bus_number";
                return ret;
            }
            
        static fh_stringstream
        SL_getStorageURLStream( HalDeviceContext* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                string sdev = getStrAttr( c, "block.storage_device", "" );
                if( !sdev.empty() )
                {
                    ss << "hal://devices/storage/" << remove_prefix( sdev, "/org/freedesktop/Hal/devices/" );
                }
                return ss;
            }
        
        static fh_stringstream
        SL_getEAFromHALParentStream( HalDeviceContext* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                string sdev = getStrAttr( c, "ferris.storage_url", "" );
                if( !sdev.empty() )
                {
                    LG_HAL_D << "trying to get ean:" << ean << " from:" << sdev << endl;
                    fh_context child = Resolve( sdev );
                    ss << getStrAttr( child, ean, "" );
                }
                return ss;
            }

        static fh_stringstream
        SL_getMediaBusStream( HalDeviceContext* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                string sdev = getStrAttr( c, "ferris.storage_url", "" );
                if( !sdev.empty() )
                {
                    LG_HAL_D << "trying to get ean:" << ean << " from:" << sdev << endl;
                    fh_context child = Resolve( sdev );
                    ss << getStrAttr( child, "storage.bus", "" );
                }
                return ss;
            }
        
        
        void setup( fdoHalDevice* hdev )
            {
                m_hdev = hdev;

                string className = getParent()->getURL();
                bool force = AttributeCollection::isStateLessEAVirgin( className );
                LG_HAL_D << "HalDevice(ctor) force:" << force << " klass:" << className << " earl:" << getURL() << endl;
                setup_DynamicClassedStateLessEAHolder( className );

#define SLEA tryAddStateLessAttribute
                if( getClassInitCount()[ className ] < 1 )
                {
                    string pk = getClassInitKeys()[ className ];
                    
                    if( !pk.empty() && m_hdev->PropertyExists( pk.c_str() ) )
                    {
                        getClassInitCount()[ className ]=1;
                        force=1;
                        LG_HAL_D << "HalDevice(ctor2) force:" << force << " klass:" << className << " earl:" << getURL() << endl;
                    }
                }
                
    
                if( force )
                {
                    {
                        QVariantMap plist = m_hdev->GetAllProperties();
                        for( QVariantMap::iterator pi = plist.begin(); pi != plist.end(); ++pi )
                        {
                            string rdn = tostr( *pi );
                            LG_HAL_D << "adding SLEA:" << rdn << endl;
                            SLEA( rdn, &_Self::SL_getDBusPropertyStream, XSD_BASIC_STRING );
                        }
                    }
                    SLEA( "eject", &_Self::SL_getEjectStream, XSD_BASIC_STRING );
                    SLEA( "mount",
                          SL_getMountIOStream,
                          SL_getMountIOStream,
                          SL_FlushMountData,
                          XSD_BASIC_STRING );
                    SLEA( "unmount",
                          SL_getUnMountIStream,
                          SL_getUnMountIOStream,
                          SL_FlushNullData,
                          XSD_BASIC_STRING );
                    SLEA( "umount",
                          SL_getUnMountIStream,
                          SL_getUnMountIOStream,
                          SL_FlushNullData,
                          XSD_BASIC_STRING );
                    if( isStatelessAttributeBound( "block.storage_device" ) )
                    {
                        SLEA( "ferris.storage_url", &_Self::SL_getStorageURLStream, XSD_BASIC_STRING );
                    }
                    if( starts_with( className, "hal:///devices/volume" ))
                    {
                        stringlist_t sl;
                        sl.push_back("storage.bus");
                        sl.push_back("storage.model");
                        sl.push_back("storage.vendor");
                        for( stringlist_t::iterator si = sl.begin(); si != sl.end(); ++si )
                        {
                            SLEA( *si, &_Self::SL_getEAFromHALParentStream, XSD_BASIC_STRING );
                        }
                        SLEA( "media.bus", &_Self::SL_getMediaBusStream, XSD_BASIC_STRING );
                    }
                }
                
                createStateLessAttributes( force );
            }
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
#undef SLEA
    };



    
    
    ////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////

    class FERRISEXP_DLLLOCAL HalRootContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        DYNAMICLINKED_ROOTCTX_CLASSCHUNK( HalRootContext );

        fdoHalManager* m_manager;
        fh_context m_devices;
        
        static QDBusConnection& getClassConnection()
            {
                static QDBusConnection ret = QDBusConnection::systemBus();
                return ret;
            }

    public:
        HalRootContext()
            :
            _Base( 0, "/" ),
            m_manager( 0 ),
            m_devices( 0 )
            {
                LG_HAL_D << "HalRootContext() ctor" << endl;

                ensureQApplication();
                m_manager = new fdoHalManager( "org.freedesktop.Hal",
                                               "/org/freedesktop/Hal/Manager",
                                               getClassConnection(),
                                               0 );
                createStateLessAttributes();
            }
        virtual ~HalRootContext()
            {}

        fh_context getDeviceChild( const std::string& udi )
            {
                if( !isBound( m_devices ) )
                {
                    m_devices = priv_ensureSubContext<FakeInternalContext>( "devices" );
                }

//                 cerr << "m_devices:" << m_devices->getURL () << endl;
//                 cerr << "m_devices.raw:" << toVoid( GetImpl( m_devices ) ) << endl;
                
                fh_context devices = m_devices;
                fh_context pctx = devices;
                
                if( starts_with( udi, "/org/freedesktop/Hal/devices/usb" ) )
                {
                    pctx = devices->priv_ensureSubContext<FakeInternalContext>( "usb" );
                }
                else if( starts_with( udi, "/org/freedesktop/Hal/devices/pci" ) )
                {
                    pctx = devices->priv_ensureSubContext<FakeInternalContext>( "pci" );
                }
                else if( starts_with( udi, "/org/freedesktop/Hal/devices/platform" ) )
                {
                    pctx = devices->priv_ensureSubContext<FakeInternalContext>( "platform" );
                }
                else if( starts_with( udi, "/org/freedesktop/Hal/devices/pnp" ) )
                {
                    pctx = devices->priv_ensureSubContext<FakeInternalContext>( "pnp" );
                }
                else if( starts_with( udi, "/org/freedesktop/Hal/devices/volume" ) )
                {
                    pctx = devices->priv_ensureSubContext<FakeInternalContext>( "volume" );
                }
                else if( starts_with( udi, "/org/freedesktop/Hal/devices/storage" ) )
                {
                    pctx = devices->priv_ensureSubContext<FakeInternalContext>( "storage" );
                }
                else if( starts_with( udi, "/org/freedesktop/Hal/devices/acpi" ) )
                {
                    pctx = devices->priv_ensureSubContext<FakeInternalContext>( "acpi" );
                }
                else if( starts_with( udi, "/org/freedesktop/Hal/devices/computer" ) )
                {
                    pctx = devices->priv_ensureSubContext<FakeInternalContext>( "computer" );
                }
                else if( starts_with( udi, "/org/freedesktop/Hal/devices/net" ) )
                {
                    pctx = devices->priv_ensureSubContext<FakeInternalContext>( "net" );
                }
                else if( starts_with( udi, "/org/freedesktop/Hal/devices/ieee1394" ) )
                {
                    pctx = devices->priv_ensureSubContext<FakeInternalContext>( "firewire" );
                }

                return pctx;
            }
        
        void read( bool force )
            {
                emitExistsEventForEachItemRAII _raii1( this );

                if( empty() )
                {
                    LG_HAL_D << "HalRootContext::read(READING!) url:" << getURL() << endl;

                    fh_context devices = priv_ensureSubContext<FakeInternalContext>( "devices" );
                    

                
                    QStringList dlist = m_manager->GetAllDevices();
                    cerr << "dlist.sz:" << dlist.size() << endl;
                    for( QStringList::iterator iter = dlist.begin();
                         iter != dlist.end(); ++iter )
                    {
                        QString udi = *iter;
//                        std::cerr << "found device:" << udi << std::endl;

                        OnDeviceAdded( udi );
                            
//                        m_devices[udi] = new MyHalDevice(conn(), udi);

//                        cerr << "pctx:" << pctx->getDirName() << " earl:" << pctx->getURL() << endl;
                        
                    }
                }
            }

        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }


        void OnDeviceAdded( const QString &qudi )
        {
            std::string udi = tostr( qudi );
//            m_devices[udi] = new MyHalDevice(conn(), udi);
            LG_HAL_D << "DeviceAdded (start):" << udi << endl;
            fh_context pctx = getDeviceChild( udi );
            string rdn = udi.substr(udi.rfind("/")+1);
            HalDeviceContext* c = pctx->priv_ensureSubContext<HalDeviceContext>( rdn );
            fdoHalDevice* hdev = new fdoHalDevice( "org.freedesktop.Hal.Device",
                                                   udi.c_str(),
                                                   QDBusConnection::systemBus() );
            c->setup( hdev );
            pctx->Emit_Created( 0, c, rdn, rdn, 0 );
            LG_HAL_D << "DeviceAdded (end):" << udi
                     << " pctx:" << pctx->getURL()
                     << " c:" << c->getURL()
                     << endl;
        }
    void OnDeviceRemoved( const QString & qudi )
        {
            std::string udi = tostr( qudi );
//            m_devices.erase( udi );
            LG_HAL_D << "DeviceRemoved:" << udi << endl;
            fh_context pctx = getDeviceChild( udi );
            string rdn = udi.substr(udi.rfind("/")+1);
            pctx->Remove( rdn );
        }
    };

    DYNAMICLINKED_ROOTCTX_DROPPER( "hal", HalRootContext );
    
};
