/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2009 Ben Martin

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

    $Id: libferrispostgresql.cpp,v 1.11 2009/04/18 21:30:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <Trimming.hh>
#include <FerrisDOM.hh>
#include <FerrisCreationPlugin.hh>
#include <FerrisBoost.hh>

#include "Platinum.h"
#include "PltMediaServer.h"
#include "PltSyncMediaBrowser.h"
#include "PltMediaController.h"
#include "NptMap.h"
#include "NptStack.h"
#include "PltFileMediaServer.h"
#include "PltMediaRenderer.h"
#include "PltVersion.h"

using namespace std;

#define DEBUG LG_UPNP_D
//#define DEBUG cerr

namespace Ferris
{
    
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };


    template <class OSS>
    OSS& operator<<( OSS& oss, NPT_String& s )
    {
        oss << s.GetChars();
        return oss;
    }

    string tostr( const NPT_String& s )
    {
        return s.GetChars();
    }
    
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    class UPNPRootContext;
    class UPnPServerContext;

    class FERRISEXP_CTXPLUGIN UPnPContext
        :
        public StateLessEAHolder< UPnPContext, FakeInternalContext >
    {
        typedef StateLessEAHolder< UPnPContext, FakeInternalContext > _Base;
        typedef UPnPContext _Self;

        std::string m_objectID;
        bool m_isDir;
        virtual bool isDir()
        {
            return m_isDir;
        }
        
        
    public:
        UPnPContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_isDir( 0 )
            {
                createStateLessAttributes();
            }
        void constructObject( PLT_MediaObject* mo )
        {
            m_objectID = tostr(mo->m_ObjectID);
            m_isDir = mo->IsContainer();

            m_ea["object-id"]   = mo->m_ObjectID;
            m_ea["class"]       = mo->m_ObjectClass.type;
            m_ea["delegate-url"]= "";
            m_ea["thumbnail-url"]= mo->m_ExtraInfo.album_art_uri;
            m_ea["track"]       = tostr( mo->m_MiscInfo.original_track_number );
            m_ea["title"]       = mo->m_Title;
            m_ea["description"] = mo->m_Title;
            int cpos = m_ea["title"].find(":");
            if( string::npos != cpos )
            {
                m_ea["title"]       = m_ea["title"].substr( 0, cpos );
                m_ea["description"] = m_ea["description"].substr( cpos+1 );
                m_ea["description"] = replaceg( m_ea["description"], "^[ ]*(.*)", "\\1" );
            }
            m_ea["size"] = "0";
            m_ea["duration"] = "0";
            m_ea["mtime"]= "0";
            
            DEBUG << "title:" << mo->m_Title << endl;
            DEBUG << "ObjectID:" << mo->m_ObjectID << endl;
            DEBUG << "Class:" << mo->m_ObjectClass.type << endl;
            DEBUG << "Creator:" << mo->m_Creator << endl;
            DEBUG << "Date:" << mo->m_Date << endl;
            DEBUG << "Art Uri:" << mo->m_ExtraInfo.album_art_uri << endl;
            DEBUG << "Art Uri Dlna Profile:" << mo->m_ExtraInfo.album_art_uri_dlna_profile << endl;
            for ( long i=0; i < mo->m_Resources.GetItemCount(); ++i)
            {
                DEBUG << "\tResource[" << i << "].uri:" << mo->m_Resources[i].m_Uri << endl;
                DEBUG << "\tResource[" << i << "].profile:" << mo->m_Resources[i].m_ProtocolInfo << endl;
                DEBUG << "\tResource[" << i << "].duration:" << mo->m_Resources[i].m_Duration << endl;
                DEBUG << "\tResource[" << i << "].size:" << mo->m_Resources[i].m_Size << endl;
                m_ea["size"] = tostr(mo->m_Resources[i].m_Size);
                m_ea["duration"] = tostr(mo->m_Resources[i].m_Duration);

                string earl = tostr( mo->m_Resources[i].m_Uri );
                m_ea["delegate-url"] = earl;
                m_ea["mtime"]= "0";
                string t = regex_match_single( earl, "^.*StartTime=([^&]+).*" );
                if( !t.empty() )
                    m_ea["mtime"] = tostr(Time::toTime(Time::ParseTimeString(t)));
            }

            m_ea["duration-human-readable"] = Time::toHMSString( toint( m_ea["duration"] ), false );
        }

        typedef map< string, string > m_ea_t;
        m_ea_t m_ea;
        
        static fh_istream SL_getEAStream( UPnPContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            ss << c->m_ea[ rdn ];
            return ss;
        }

        static fh_istream SL_getThumbnailRGBA( UPnPContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            fh_context thumbc = Resolve( c->m_ea["thumbnail-url"] );
            string thumbdata = getStrAttr( thumbc, "content", "", true, true );
                        
            return ss;
        }
        
        virtual void createStateLessAttributes( bool force = false )
        {
            static Util::SingleShot virgin;
            if( virgin() )
            {
#define SLEA tryAddStateLessAttribute

                SLEA( "size",         SL_getEAStream, FXD_FILESIZE );
                SLEA( "mtime",        SL_getEAStream, FXD_UNIXEPOCH_T );
                SLEA( "delegate-url", SL_getEAStream, FXD_URL );
                SLEA( "thumbnail-url",SL_getEAStream, FXD_URL );
                SLEA( "duration",     SL_getEAStream, XSD_BASIC_INT );
                SLEA( "class",        SL_getEAStream, XSD_BASIC_STRING );
                SLEA( "object-id",    SL_getEAStream, XSD_BASIC_STRING );
                SLEA( "title",        SL_getEAStream, XSD_BASIC_STRING );
                SLEA( "track",        SL_getEAStream, XSD_BASIC_STRING );
                SLEA( "description",  SL_getEAStream, XSD_BASIC_STRING );
                SLEA( "duration-human-readable", SL_getEAStream, XSD_BASIC_STRING );

//                SLEA( "exif:thumbnail-rgba-32bpp", SL_getThumbnailRGBA, FXD_FILESIZE );
#undef SLEA
                _Base::createStateLessAttributes( true );
                supplementStateLessAttributes( true );
            }
        }

        string getRecommendedEA()
        {
            fh_stringstream ret;
            ret << "title,size-human-readable,mtime-display,duration-human-readable,description";
            return ret.str();
        }
    
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
        {
            string earl = m_ea["delegate-url"];
            DEBUG << "reading data from earl:" << earl << endl;
            fh_context c = Resolve( earl );
            return c->getIStream( m );
        }
        
        
        UPNPRootContext* getRoot();
        virtual UPnPServerContext* getServer();
        virtual PLT_DeviceDataReference& getDevice();
        virtual string getDirPath_UPnP();
        virtual void priv_read();
    };
    
    
    
    class FERRISEXP_CTXPLUGIN UPnPServerContext
        :
        public UPnPContext
    {
        typedef UPnPContext       _Base;
        typedef UPnPServerContext _Self;
        
    public:

        PLT_DeviceDataReference m_device;
        UPnPServerContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                DEBUG << "UPnPServerContext() rdn:" << rdn << endl;
                createStateLessAttributes();
            }
        void constructObject( PLT_DeviceDataReference& device )
        {
            m_device = device;
        }
        virtual UPnPServerContext* getServer()
        {
            return this;
        }
        virtual PLT_DeviceDataReference& getDevice()
        {
            return m_device;
        }
        virtual bool isDir()
        {
            return true;
        }
    };


    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    class FERRISEXP_CTXPLUGIN UPNPRootContext
        :
        public FakeInternalContext,
        public PLT_SyncMediaBrowser,
        public PLT_MediaController,
        public PLT_MediaControllerDelegate
    {
        typedef FakeInternalContext _Base;
        typedef UPNPRootContext _Self;
        
    public:

        PLT_UPnP* m_upnp;
        PLT_CtrlPointReference m_controlPoint;

        UPNPRootContext( Context* parent, const std::string& rdn,
                         PLT_CtrlPointReference controlPoint )
            :
            _Base( parent, rdn ),
            m_upnp( new PLT_UPnP() ),
            m_controlPoint( controlPoint ),
            PLT_MediaController( controlPoint, this ),
            PLT_SyncMediaBrowser( controlPoint ),
            m_OnDeviceAdded_queue( 0 ),
            m_glib_thead_OnDeviceAdded_interval( 300 )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;

                m_OnDeviceAdded_queue = g_async_queue_new();
                
//                PLT_MediaController::SetDelegate(this);
                m_upnp->AddCtrlPoint( m_controlPoint );
                m_upnp->Start();

                // force broadcast prods for best "visibility"
                m_controlPoint->Discover(NPT_HttpUrl("255.255.255.255", 1900, "*"), "upnp:rootdevice", 1, 6000);
                m_controlPoint->Discover(NPT_HttpUrl("239.255.255.250", 1900, "*"), "upnp:rootdevice", 1, 6000);

                DEBUG << "started upnp client..." << endl;
            }

        virtual ~UPNPRootContext()
        {
            m_upnp->Stop();
        }

        
        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                           << " have read:" << getHaveReadDir()
                           << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                
                // if( !empty() )
                // {
                //     DEBUG << "priv_read() url:" << getURL() << endl;
                //     emitExistsEventForEachItemRAII _raii1( this );
                // }
                // else
                {
                    DEBUG << "reading..." << endl;
                    
                    const NPT_Lock<PLT_DeviceMap>& dm = GetMediaServersMap();
                    const NPT_List<PLT_DeviceMapEntry*>& entries = dm.GetEntries();
                    NPT_List<PLT_DeviceMapEntry*>::Iterator entry = entries.GetFirstItem();
                    for (; entry; ++entry )
                    {
                        PLT_DeviceDataReference device = (*entry)->GetValue();
                        NPT_String              name   = device->GetFriendlyName();
                        DEBUG << "key:" << (char*)((*entry)->GetKey())
                              << " name:" <<  (char*)name
                              << endl;

                        UPnPServerContext* c = 0;
                        c = priv_ensureSubContext( name.GetChars(), c );
                        c->constructObject( device );
                    }

                    sleep(1);
                }
            }

        guint m_glib_thead_OnDeviceAdded_timer;
        guint m_glib_thead_OnDeviceAdded_interval;
        void glib_thead_reconnectTimer()
            {
                if( m_glib_thead_OnDeviceAdded_timer )
                {
                    g_source_remove( m_glib_thead_OnDeviceAdded_timer );
                    m_glib_thead_OnDeviceAdded_timer = 0;
                }
                if( m_glib_thead_OnDeviceAdded_interval )
                {
                    m_glib_thead_OnDeviceAdded_timer =
                        g_timeout_add( m_glib_thead_OnDeviceAdded_interval,
                                       GSourceFunc(s_glib_thead_OnDeviceAdded), this );
                }
            }

        static void s_glib_thead_OnDeviceAdded( gpointer data )
        {
            _Self* sp = (_Self*)data;
            sp->glib_thead_OnDeviceAdded();
        }

        GAsyncQueue* m_OnDeviceAdded_queue;
        void glib_thead_OnDeviceAdded()
        {
            gpointer d = g_async_queue_try_pop( m_OnDeviceAdded_queue );
            PLT_DeviceDataReference device( (PLT_DeviceData*)d );

            DEBUG << "g_OnDeviceAdded() ... " << endl;
            string name = (char*)device->GetFriendlyName();
            DEBUG << " name:" <<  name << endl;

            UPnPServerContext* c = 0;
            c = priv_ensureSubContext( name.c_str(), c );
            c->constructObject( device );
            
        }
        
        
        NPT_Result OnDeviceAdded( PLT_DeviceDataReference& device )
        {
            DEBUG << "OnDeviceAdded() ... " << endl;
            string name = (char*)device->GetFriendlyName();
            DEBUG << " name:" <<  name << endl;

//            PLT_DeviceData* dp = device.AsPointer();
//            g_async_queue_push( m_OnDeviceAdded_queue, dp );

            // string name = (char*)device->GetFriendlyName();
            // DEBUG << " name:" <<  name << endl;

            // UPnPServerContext* c = 0;
            // c = priv_ensureSubContext( name.c_str(), c );
            // c->constructObject( device );
            
            return PLT_SyncMediaBrowser::OnDeviceAdded( device );
        }


        PLT_MediaObjectListReference
            DoBrowse( PLT_DeviceDataReference& dev, const std::string& path )
        {
            PLT_MediaObjectListReference ret;
            NPT_Result rc = NPT_FAILURE;
            if (!dev.IsNull())
            {
                {
                    string name = (char*)dev->GetFriendlyName();
                    DEBUG << "device_name:" <<  name << endl;
                }
                DEBUG << "calling BrowseSync() path:" << path << endl;
                rc = BrowseSync( dev, (const char*)path.c_str(), ret );
                DEBUG << "rc:" << rc << endl;
            }

            return ret;
        }
        
        
    };


    
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    UPNPRootContext*
    UPnPContext::getRoot()
    {
        UPNPRootContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p;
    }

    UPnPServerContext*
    UPnPContext::getServer()
    {
        UPnPServerContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p;
    }
    
    PLT_DeviceDataReference&
    UPnPContext::getDevice()
    {
        return getServer()->getDevice();
    }
    
    string
    UPnPContext::getDirPath_UPnP()
    {
        if( this == getServer() )
        {
            return "0";
        }
        
        string serverPath = getServer()->getDirPath();
        string p = getDirPath();
//        string ret = p.substr( serverPath.length()+1 );
        string ret = m_objectID;
            
        DEBUG << "getDirPath_UPnP() p:" << getDirPath() << endl
             << " ret:" << ret
             << endl;
        return ret;
    }
    
    
    void
    UPnPContext::priv_read()
    {
        DEBUG << "priv_read() url:" << getURL()
              << " have read:" << getHaveReadDir()
              << endl;
        EnsureStartStopReadingIsFiredRAII _raii1( this );
        clearContext();
        
        string name = (char*)getDevice()->GetFriendlyName();
        DEBUG << "UPnPContext::priv_read() device_name:" <<  name << endl;
        
        string upath = getDirPath_UPnP();
        UPNPRootContext* r = getRoot();
        PLT_MediaObjectListReference result = r->DoBrowse( getDevice(), upath );
        if (result.IsNull())
        {
            DEBUG << "NO Result!" << endl;
        }
        else
        {
            DEBUG << "result count:" << result->GetItemCount() << endl;

            for( NPT_List<PLT_MediaObject*>::Iterator iter = result->GetFirstItem();
                 iter; ++iter )
            {
                PLT_MediaObject* mo = *iter;
                DEBUG << "dir:" << mo->IsContainer()
                     << " title:" << mo->m_Title << endl
                     << "... id:" << mo->m_ObjectID
                     << endl;

                string rdn = tostr( mo->m_Title );
                if( !mo->IsContainer() )
                    rdn = rdn.substr( 0, rdn.find(":"));

               
                // make sure rdn is uniq, but if its bound with
                // same object-id don't change the name
                // rdn = monsterName( rdn );
                {
                    //DEBUG << "--------------------------------------------------------------------" << endl;
                    //DEBUG << "Checking for name clash, rdn:" << rdn << " object-id:" << mo->m_ObjectID << endl;
                    string baserdn = rdn;
                    int version = 1;
                    Items_t::iterator iter;
                    while( priv_isSubContextBound( rdn, iter ) )
                    {
                        if( UPnPContext* c = dynamic_cast<UPnPContext*>( GetImpl(*iter) ) )
                        {
//                            DEBUG << "found with object-id:" << c->m_ea["object-id"] << endl;
                            if( c->m_ea["object-id"] == tostr(mo->m_ObjectID) )
                                break;

                            ostringstream ss;
                            ss << baserdn << "--" << version++;
                            rdn = tostr(ss);
                        }
                        else
                        {
                            rdn = monsterName( rdn );
                            break;
                        }
                    }
                }

                DEBUG << "adding file with rdn:" << rdn << endl;
                UPnPContext* c = 0;
                c = priv_ensureSubContext( rdn.c_str(), c );
                c->constructObject( mo );
            }
        }
        // DEBUG << "+++ at end of priv_read() size:" << SubContextCount() << endl;
        // dumpOutItems();
        
        // emitExistsEventForEachItem();
    }

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                DEBUG << "Brew()" << endl;
                if (!g_thread_supported ()) g_thread_init (NULL);

                static UPNPRootContext* c = 0;
                if( !c )
                {
                    PLT_CtrlPointReference controlPoint( new PLT_CtrlPoint() );
                    c = new UPNPRootContext(0, "/", controlPoint );
                    // Bump ref count.
                    static fh_context keeper = c;
                    static fh_context keeper2 = keeper;
                }
                fh_context ret = c;
                return ret;
            }
            catch( FerrisSqlServerNameNotFound& e )
            {
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
            catch( exception& e )
            {
                LG_PG_W << "e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }

};
