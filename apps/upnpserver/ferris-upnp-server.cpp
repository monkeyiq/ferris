/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris ferris-upnp-server
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

    $Id: ls.cpp,v 1.12 2008/04/27 21:30:11 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#define DEBUG 0

#include <config.h>

#include <string>
#include <list>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisKDE.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferrisls.hh>

#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;

#include "PltUPnP.h"
#include "PltFileMediaServer.h"
#include "PltDidl.h"
NPT_SET_LOCAL_LOGGER("libferris.upnp")


const string PROGRAM_NAME = "ferris-upnp-server";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

string tostr( NPT_String& s )
{
    return s.GetChars();
}


const char* ConfigURL_CSTR = 0;
unsigned long Verbose = 0;
unsigned long Broadcast = 0;
unsigned long ShowVersion = 0;


class Ferris_FileMediaServer
    :
    public PLT_FileMediaServer
{
    typedef PLT_FileMediaServer _Base;

    fh_context m_base;
    
public:
    Ferris_FileMediaServer( const char*  path, 
                            const char*  friendly_name,
                            bool         show_ip = false,
                            const char*  uuid = NULL,
                            NPT_UInt16   port = 0,
                            bool         port_rebind = false)
        :
        PLT_FileMediaServer( path, friendly_name, show_ip, uuid, port, port_rebind )
        {
            m_base = Resolve( path );
        }

    bool ProcessFile( fh_context c )
        {
            return true;
        }

    virtual PLT_MediaObject* BuildFromFilePath( fh_context                   c,
                                               const PLT_HttpRequestContext& context,
                                               bool                          with_count = true,
                                               bool                          keep_extension_in_title = false)
        {
            PLT_MediaItemResource resource;
            PLT_MediaObject*      object = NULL;

            cerr << "Building didl for url:" << c->getURL() << endl;

            if( isTrue( getStrAttr( c, "is-dir", "0" )))
            {
                object = new PLT_MediaContainer;

                /* Assign a title for this container */
                if( c->getURL() == m_base->getURL() )
                {
                    object->m_Title = "Root";
                }
                else
                {
                    object->m_Title = c->getDirName().c_str();
                    if (!object->m_Title.GetLength())
                    {
                        delete object;
                        return NULL;
                    }
                }

                /* Get the number of children for this container */
                if( with_count )
                {
                    ((PLT_MediaContainer*)object)->m_ChildrenCount = c->getSubContextCount();
                }
                object->m_ObjectClass.type = "object.container.storageFolder";
            }
            else
            {
                // its a file
                object = new PLT_MediaItem();

                object->m_Description.long_description = getStrAttr( c, "description" ,"" ).c_str();
                object->m_Title = c->getDirName().c_str();

                /* Set the protocol Info from the extension */
                cerr << "setting resource.m_Uri to:" << c->getURL() << endl;
                string mimeType = getStrAttr( c, "mimetype", "application/unknown" );
                stringstream protinfoss;
                protinfoss << "http-get:*:" << mimeType << ":"
                           << PLT_MediaObject::GetDlnaExtension( mimeType.c_str(), &context );
                cerr << "protinfoss:" << protinfoss.str() << endl;
                resource.m_ProtocolInfo = protinfoss.str().c_str();
                resource.m_Size = toType<long long>( getStrAttr( c, "size", "0" ));
                resource.m_Duration = toint( getStrAttr( c, "duration", "1" ) );
//                resource.m_Protection = 0;

                NPT_List<NPT_IpAddress> ips;
                PLT_UPnPMessageHelper::GetIPAddresses(ips);
                // if we're passed an interface where we received the request from
                // move the ip to the top
                if (context.GetLocalAddress().GetIpAddress().ToString() != "0.0.0.0") {
                    ips.Remove(context.GetLocalAddress().GetIpAddress());
                    ips.Insert(ips.GetFirstItem(), context.GetLocalAddress().GetIpAddress());
                }
                // iterate through list and build list of resources
                NPT_List<NPT_IpAddress>::Iterator ip = ips.GetFirstItem();

                while (ip)
                {
                    resource.m_Uri = BuildResourceUri(
                        m_FileBaseUri,
                        ip->ToString(),
                        c->getURL().c_str() );
                    
                    if( !object->m_ExtraInfo.album_art_uri.GetLength() )
                    {
                        object->m_ExtraInfo.album_art_uri = 
                            NPT_Uri::PercentEncode(
                                BuildResourceUri(
                                    m_AlbumArtBaseUri, ip->ToString(), c->getURL().c_str()), 
                                NPT_Uri::UnsafeCharsToEncode);
                    }
                    object->m_Resources.Add(resource);
                    ++ip;
                }
            }

            if( c->getURL() == m_base->getURL() )
            {
                // root
                object->m_ParentID = "-1";
                object->m_ObjectID = "0";
            }
            else
            {
                object->m_ParentID = "0";
                object->m_ObjectID = ("0" + c->getDirPath()).c_str();
            }
            return object;
        }
    
    
    virtual NPT_Result OnBrowseDirectChildren( PLT_ActionReference&          action, 
                                               const char*                   object_id, 
                                               const char*                   filter,
                                               NPT_UInt32                    starting_index,
                                               NPT_UInt32                    requested_count,
                                               const NPT_List<NPT_String>&   sort_criteria,
                                               const PLT_HttpRequestContext& context)
        {
            cerr << "OnBrowseDirectChildren()" << endl;

            /* locate the file from the object ID */
            NPT_String dir;
            if (NPT_FAILED(GetFilePath(object_id, dir))) {
                /* error */
                NPT_LOG_WARNING("ObjectID not found.");
                action->SetError(710, "No Such Container.");
                return NPT_FAILURE;
            }

            fh_context c = Resolve( tostr(dir) );
            
            if( isFalse( getStrAttr( c, "is-dir", "0" )))
            {
                /* error */
                NPT_LOG_WARNING("BROWSEDIRECTCHILDREN not allowed on an item.");
                action->SetError(710, "No such container");
                return NPT_FAILURE;
            }


            unsigned long cur_index = 0;
            unsigned long num_returned = 0;
            unsigned long total_matches = 0;
            NPT_String didl = didl_header;

            PLT_MediaObjectReference item;
            for( Context::iterator ci = c->begin(); ci != c->end(); ++ci )
            {
        
                // verify we want to process this file first
                if (!ProcessFile( *ci )) continue;
        
                item = BuildFromFilePath( *ci, context, true);

                if (!item.IsNull()) {
                    if ((cur_index >= starting_index) && 
                        ((num_returned < requested_count) || (requested_count == 0)))
                    {
                        NPT_String tmp;
                        NPT_CHECK_SEVERE(PLT_Didl::ToDidl(*item.AsPointer(), filter, tmp));
                        
                        didl += tmp;
                        num_returned++;
                    }
                    cur_index++;
                    total_matches++;        
                }
            };

            didl += didl_footer;

            NPT_CHECK_SEVERE(action->SetArgumentValue("Result", didl));
            NPT_CHECK_SEVERE(action->SetArgumentValue("NumberReturned",
                                                      NPT_String::FromInteger(num_returned)));
            NPT_CHECK_SEVERE(action->SetArgumentValue("TotalMatches",
                                                      NPT_String::FromInteger(total_matches)));
            NPT_CHECK_SEVERE(action->SetArgumentValue("UpdateId", "1"));
            return NPT_SUCCESS;
            
            // return _Base::OnBrowseDirectChildren( action, object_id, filter,
            //                                       starting_index, requested_count,
            //                                       sort_criteria, context );
        }
    
    NPT_Result GetFilePath( const char* object_id_CSTR, NPT_String& filepath) 
        {
            if (!object_id_CSTR) return NPT_ERROR_INVALID_PARAMETERS;

            string object_id = object_id_CSTR;
            if( object_id == "0" )
            {
                filepath = m_base->getURL().c_str();
            }
            else
            {
                filepath = object_id.substr(1).c_str();
            }
            
            cerr << "GetFilePath() object_id:" << object_id << " ret:" << filepath.GetChars() << endl;
            return NPT_SUCCESS;
        }
    
};

int main( int argc, const char** argv )
{
    Ferrisls ls;

    
    struct poptOption optionsTable[] = {

        { "config-file", 0, POPT_ARG_STRING, &ConfigURL_CSTR, 0,
          "URL of config file to use for UPnP server", "	" },
        
        /*
         * Other handy stuff
         */

        { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
          "show chatter about what is happening", 0 },

        { "broadcast", 'b', POPT_ARG_NONE, &Broadcast, 0,
          "announce yourself via broadcast", 0 },
        
        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },

        /*
         * Standard Ferris options
         */
        FERRIS_POPT_OPTIONS

        /**
         * Expansion of strange-url://foo*
         */
        FERRIS_SHELL_GLOB_POPT_OPTIONS
        POPT_AUTOHELP
        POPT_TABLEEND
   };
    poptContext optCon;


    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* url1 url2 ...");

    if (argc < 1) {
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }

    
    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {}

    if( ShowVersion )
    {
        cout << "ferris-upnp-server version: $Id: ls.cpp,v 1.12 2008/04/27 21:30:11 ben Exp $\n"
             << "ferris   version: " << VERSION << nl
             << "Written by Ben Martin, aka monkeyiq" << nl
             << nl
             << "Copyright (C) 2009 Ben Martin" << nl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }

    try
    {
        KDE::ensureKDEApplication();
        
        stringlist_t srcs;
        srcs = expandShellGlobs( srcs, optCon );
        if( srcs.empty() )
        {
            srcs.push_back(".");
        }
        stringlist_t::iterator srcsiter = srcs.begin();
        stringlist_t::iterator srcsend  = srcs.end();

        string UPnPServerName = "libferris";
        string UPnPServerDesc = "libferris media server";
        string UPnPServerVersion = "1.0";
        string UUID = "e1be5865-647a-413f-b2ef-f1c1c9834d7e";
        string baseURL = *srcsiter;
        int SpecificPort = 0;

        cerr << "Broadcast:" << Broadcast << endl;
        cerr << "baseURL:" << baseURL << endl;
        cerr << "UPnPServerName:" << UPnPServerName << endl;
        
        PLT_UPnP upnp( 1900, !Broadcast );
        PLT_DeviceHostReference device(
            new Ferris_FileMediaServer(
                baseURL.c_str(), 
                UPnPServerName.c_str(),
                false,
                UUID.c_str(),
                (NPT_UInt16)SpecificPort
                )
            );
        device->ToLog(NPT_LOG_LEVEL_ALL);

        //device->m_PresentationURL = NPT_HttpUrl(ip, 80, "/").ToString();
        device->m_ModelDescription = UPnPServerDesc.c_str();
        device->m_ModelURL = "http://www.libferris.com/";
        device->m_ModelNumber = UPnPServerVersion.c_str();
        device->m_ModelName = UPnPServerName.c_str();
        device->m_Manufacturer = "the libferris crew";
        device->m_ManufacturerURL = "http://www.libferris.com/";
        device->SetBroadcast(Broadcast);

        upnp.AddDevice(device);
        NPT_String uuid = device->GetUUID();
        NPT_CHECK_SEVERE(upnp.Start());

        cerr << "main loop..." << endl;
        Main::mainLoop();
        
    }
    catch( NoSuchContextClass& e )
    {
        cerr << "e:" << e.what() << endl;
        exit(1);
    }
    catch( exception& e )
    {
        cerr << "ls.cpp cought:" << e.what() << endl;
        exit(1);
    }

    poptFreeContext(optCon);
    cout << flush;
    return 0;
}
