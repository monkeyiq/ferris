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

    $Id: libferrispostgresqlshared.cpp,v 1.2 2006/12/07 06:49:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"

#include "libferrisgstreamer_shared.hh"
#include <Ferris/Configuration_private.hh>


#define DEBUG LG_GSTREAMER_D
//#define DEBUG cerr


namespace Ferris
{

    namespace GStreamer
    {
        void
        start_feed( GstElement * pipeline, guint size, void* userdata )
        {
            DEBUG << "+++ START_FEED size:" << size << endl;
            gstreamer_readFrom_streambuf<char>* sb = (gstreamer_readFrom_streambuf<char>*)userdata;
            sb->setFeeding(true);
        }
        void
        stop_feed( GstElement * pipeline, void* userdata )
        {
            DEBUG << "+++STOP_FEED" << endl;
            gstreamer_readFrom_streambuf<char>* sb = (gstreamer_readFrom_streambuf<char>*)userdata;
            sb->setFeeding(false);
        }
        void
        freefunc (void *priv)
        {
            cerr << "freeing buffer for pointer :" << (void*)priv << endl;
//  free (priv);
        }
gboolean
bus_message (GstBus * bus, GstMessage * message, void* userdata)
{
  GST_DEBUG ("got message %s",
      gst_message_type_get_name (GST_MESSAGE_TYPE (message)));

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR: {
        GError *err = NULL;
        gchar *dbg_info = NULL;

        gst_message_parse_error (message, &err, &dbg_info);
        g_printerr ("ERROR from element %s: %s\n",
            GST_OBJECT_NAME (message->src), err->message);
        g_printerr ("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");
        g_error_free (err);
        g_free (dbg_info);
//        g_main_loop_quit (app->loop);
        break;
    }
    case GST_MESSAGE_EOS:
//      g_main_loop_quit (app->loop);
      break;
    default:
      break;
  }
  return TRUE;
}
    };
    
    /****************************************/
    /****************************************/
    /****************************************/
    
};
