/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of libferris.

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

    $Id: ut_close_signal.cpp,v 1.3 2008/05/24 21:31:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#ifndef _ALREADY_INCLUDED_FERRIS_SHARED_GSTREAMER_H_
#define _ALREADY_INCLUDED_FERRIS_SHARED_GSTREAMER_H_

#include <gmodule.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappbuffer.h>

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/SignalStreams.hh>
#include <Ferris/General.hh>



namespace Ferris
{
    
    namespace GStreamer
    {
        void start_feed( GstElement * pipeline, guint size, void* userdata );
        void stop_feed (GstElement * pipeline, void* userdata);
        void freefunc (void *priv);
        gboolean bus_message (GstBus * bus, GstMessage * message, void* userdata);
        
        using namespace XML;
        using namespace std;
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        
        template<
            typename _CharT,
            typename _Traits = std::char_traits < _CharT >,
            typename _Alloc  = std::allocator   < _CharT >
            >
        class  gstreamer_readFrom_streambuf
            :
        public ferris_basic_streambuf< _CharT, _Traits, _Alloc,
                                       typename ::Ferris::ferris_basic_streambuf_sixteenk
                                       >,
        public ferris_basic_streambuf_LargeAvailabeData_Mixin< _CharT, _Traits, _Alloc >
        {
            typedef gstreamer_readFrom_streambuf< _CharT > _Self;
            // prohibit copy/assign
            gstreamer_readFrom_streambuf( const gstreamer_readFrom_streambuf& );
            gstreamer_readFrom_streambuf operator=( const gstreamer_readFrom_streambuf& );

            typedef std::char_traits<_CharT>          traits_type;
            typedef typename traits_type::int_type    int_type;
            typedef typename traits_type::char_type   char_type;
            typedef typename traits_type::pos_type    pos_type;
            typedef typename traits_type::off_type    off_type;

            bool m_EOS;
            bool m_feeding;
            bool m_writing;
            GstElement *m_pipe;
            GstElement *m_source;

            static gboolean
            on_source_message (GstBus * bus, GstMessage * message, _Self *p)
                {
                    GstElement *source;

                    LG_GSTREAMER_D << "on_source_message type:" << GST_MESSAGE_TYPE (message) << endl;
                    
                    switch (GST_MESSAGE_TYPE (message))
                    {
                        case GST_MESSAGE_EOS:
                            cerr << "The source got dry" << endl;
                            p->m_EOS = true;
//                          source = gst_bin_get_by_name (GST_BIN (data->sink), "testsource");
//                          gst_app_src_end_of_stream (GST_APP_SRC (source));
                            break;
                        case GST_MESSAGE_ERROR:
                            cerr << "Received error" << endl;
                            {
                                        GError *err = NULL;
                                        gchar *dbg_info = NULL;

                                        gst_message_parse_error (message, &err, &dbg_info);
                                        g_printerr ("ERROR from element %s: %s\n",
                                                    GST_OBJECT_NAME (message->src), err->message);
                                        g_printerr ("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");
                                        g_error_free (err);
                                        g_free (dbg_info);
                            }
                            
//                g_main_loop_quit (data->loop);
                            break;
                        case GST_MESSAGE_STREAM_STATUS:
                            cerr << "stream status" << endl;
                            break;
                        default:
                            break;
                    }
                    return TRUE;
                }
    
        public:

            gstreamer_readFrom_streambuf( const std::string& pipeDesc )
                : m_EOS( false )
                , m_feeding( true )
                , m_writing( false )
                {
                    LG_GSTREAMER_D << "gstreamer_readFrom_streambuf() pipeDesc:" << pipeDesc << endl;
                    
                    m_pipe = gst_parse_launch ( pipeDesc.c_str(), NULL);
            
                    GstBus *bus = gst_element_get_bus (m_pipe);
                    gst_bus_add_watch (bus, (GstBusFunc) on_source_message, this );
//                    gst_bus_add_watch ( bus, (GstBusFunc) bus_message, this );
                    gst_object_unref (bus);

                    m_source = gst_bin_get_by_name (GST_BIN (m_pipe), "sink");
                    if( m_source )
                    {
                        g_object_set (G_OBJECT (m_source), "sync", FALSE, NULL);
//                        gst_element_set_state (m_pipe, GST_STATE_PLAYING);
                    }
                    else
                    {
                        m_source = gst_bin_get_by_name (GST_BIN (m_pipe), "src");
                        g_assert(GST_IS_APP_SRC( m_source ));
//                        g_object_set (G_OBJECT (m_source), "sync",  FALSE, NULL);
                        g_object_set (G_OBJECT (m_source), "block", TRUE,  NULL);
                        g_signal_connect (G_OBJECT(m_source), "need-data",   G_CALLBACK (start_feed), this);
                        g_signal_connect (G_OBJECT(m_source), "enough-data", G_CALLBACK (stop_feed), this);
                    }

                    gst_element_set_state (m_pipe, GST_STATE_PLAYING);

                    
                }

            ~gstreamer_readFrom_streambuf()
            {
                cerr << "~gstreamer_readFrom_streambuf(top)" << endl;
                if( m_writing )
                {
                    this->ensureMode( this->mode_mute );
//                    sync();
//                    int rc = this->buffer_out();
                    
                    //
                    // FIXME: wait for gstreamer to play out the buffer completely...
                    //
                    GstState st, pending;
                    gst_app_src_end_of_stream (GST_APP_SRC (m_source));
                    gst_element_get_state (m_pipe, &st, &pending, GST_CLOCK_TIME_NONE );
                    m_feeding = false;
                    m_EOS = false;
                    while( !m_EOS && st ==  GST_STATE_PLAYING )
                    {
                        Main::processAllPendingEvents();
                        Time::Sleep( 0.001 );
                        gst_element_get_state (m_pipe, &st, &pending, GST_CLOCK_TIME_NONE );
//                        LG_GSTREAMER_D << "~gstreamer_readFrom_streambuf(x) st:" << st << endl;
                    }
                    
                    LG_GSTREAMER_D << "~gstreamer_readFrom_streambuf(2)" << endl;
                    LG_GSTREAMER_D << "~gstreamer_readFrom_streambuf(2) st:" << st << endl;
                    LG_GSTREAMER_D << "~gstreamer_readFrom_streambuf(2) pending:" << pending << endl;
                }
                
                gst_object_unref (m_source);
                gst_element_set_state (m_pipe, GST_STATE_NULL);
                gst_object_unref (m_pipe);
            }

            void setFeeding( bool v )
            {
                m_feeding = v;
            }
            
            
            int_type underflow()
                {
                    if( this->buffer_in() < 0 )
                    {
                        return traits_type::eof();
                    }
                    else
                    {
                        return traits_type::to_int_type(*this->gptr());
                    }
             
                }


            //////////
            ////////// move this block to superclass sometime. 
            //////////
            
//             char_type* m_largeAvailableData;
//             std::streamsize m_largeAvailableDataSize;
//             std::streamsize m_largeAvailableDataCurrentOffset;
//             void copyLargeAvailabeData( char_type* d, std::streamsize dlen )
//             {
// //                cerr << "copyLargeAvailabeData() dlen:" << dlen << endl;
//                 if(m_largeAvailableData)
//                     delete [] m_largeAvailableData;
//                 m_largeAvailableDataSize = dlen;
//                 m_largeAvailableData = new char_type[ dlen+1 ];
//                 m_largeAvailableDataCurrentOffset = 0;
//                 memcpy( m_largeAvailableData, d, dlen );
//             }
            
//             virtual int make_new_data_avail_from_LargeAvailableData( char_type* out_buffer, std::streamsize out_maxsz )
//             {
//                 if( !m_largeAvailableDataSize )
//                 {
// //                    cerr << "make_avail, LargeAvailabeData() no buffer..." << endl;
//                     return -1;
//                 }
                
//                 // cerr << "make_avail, LargeAvailabeData() "
//                 //      << " bufsz:" << m_largeAvailableDataSize
//                 //      << " maxsz:" << out_maxsz
//                 //      << " offset:" << m_largeAvailableDataCurrentOffset
//                 //      << endl;

//                 std::streamsize copySize = out_maxsz;
//                 copySize = std::min( copySize,
//                                      m_largeAvailableDataSize - m_largeAvailableDataCurrentOffset );
//                 if( copySize )
//                 {
//                     memcpy( out_buffer,
//                             m_largeAvailableData + m_largeAvailableDataCurrentOffset,
//                             copySize );
//                     m_largeAvailableDataCurrentOffset += copySize;
//                 }
//                 else
//                 {
//                     delete [] m_largeAvailableData;
//                     m_largeAvailableDataCurrentOffset = 0;
//                     m_largeAvailableDataSize = 0;
//                     return -1;
//                 }
//                 return copySize;
//             }

            /////////////////
            ////////////////
            /////////////////////
            
            virtual int make_new_data_avail( char_type* out_buffer, std::streamsize out_maxsz )
                {
                    int ret = 0;

//                    BackTrace();
                    
//                    cerr << "make_new_data_avail(top) m_EOS:" << m_EOS << endl;
                    
                    if( m_EOS )
                    {
//                        cerr << "make_new_data_avail() end of stream!" << endl;
                        return -1;
                    }

                    
                    ret = this->make_new_data_avail_from_LargeAvailableData( out_buffer, out_maxsz );
//                    cerr << "make_new_data_avail(2) ret:" << ret << endl;
                    if( ret == -1 )
                    {
//                        cerr << "make_new_data_avail(3) getting more data..." << endl;
                        
                        // try to get more.
                        GstBuffer *buffer = 0;
                        g_signal_emit_by_name (G_OBJECT (m_source), "pull-buffer", &buffer);
                        if(!buffer || !GST_BUFFER_SIZE (buffer) )
                        {
                            // no buffer?
                            m_EOS = true;
//                            cerr << "!buffer, so end of stream! m_EOS:" << m_EOS << endl;
                            return 0;
                        }
                        if( buffer )
                        {
                            int size = GST_BUFFER_SIZE (buffer);
                            // cerr << "make_new_data_avail()  in.sz:" << size << endl;
                            // cerr << "make_new_data_avail() out.sz:" << out_maxsz << endl;
                            this->copyLargeAvailabeData( (_CharT*)GST_BUFFER_DATA (buffer), size );
                            gst_buffer_unref (buffer);
                        }
                        else if( m_EOS )
                        {
//                            cerr << "end of stream!" << endl;
                            return -1;
                        }
                        
                        ret = this->make_new_data_avail_from_LargeAvailableData( out_buffer, out_maxsz );
                    }
                    
//                    cerr << "make_new_data_avail(e) ret:" << ret << endl;
                    return ret;
                }

            virtual int write_out_given_data( const char_type* data, std::streamsize sz )
                {
                    m_writing = true;
                    static std::streamsize writeTotal = 0;
//                    if( !writeTotal )
//                        gst_element_set_state (m_pipe, GST_STATE_PLAYING);                    
                    LG_GSTREAMER_D << "write_out_given_data(top) m_feeding:" << m_feeding
                                   << " sz:" << sz
                                   << " wt:" << writeTotal
                                   << " m_source:" << m_source << endl;
                    
                    if( !data || !sz )
                    {
                        LG_GSTREAMER_W << "ERROR, no data or sz" << endl;
                        return -1;
                    }
                    
                    LG_GSTREAMER_D << "write_out_given_data(2a) m_feeding:" << m_feeding << endl;
                    // while( !m_feeding )
                    // {
                    //     Main::processAllPendingEvents();
                    //     Time::Sleep( 0.0001 );
                    // }
                    LG_GSTREAMER_D << "write_out_given_data(2b) m_feeding:" << m_feeding << endl;

                     GstFlowReturn ret;
                    
                    //
                    // This is annoying, we just want to give 'data' to gstreamer
                    // but attempts to point GST_BUFFER_DATA to it seem to fail.
                    //
                    GstBuffer * gbuf = 0;
                    gbuf = gst_buffer_new_and_alloc( sz );
                    memcpy( GST_BUFFER_DATA (gbuf), data, sz );
                    // GST_BUFFER_SIZE (gbuf) = 2;
                    // GST_BUFFER_DATA (gbuf) = (guint8*)zz;
                    LG_GSTREAMER_D << "write_out_given_data(3) m_feeding:" << m_feeding << endl;
                    g_signal_emit_by_name (G_OBJECT (m_source), "push-buffer", gbuf, &ret );
                    LG_GSTREAMER_D << "write_out_given_data(4) m_feeding:" << m_feeding << endl;
                    gst_buffer_unref( gbuf );
                    if (ret != GST_FLOW_OK)
                    {
                        LG_GSTREAMER_W << "ERROR writing gbuf:" << ret << endl;
                        return -1;
                    }

                    writeTotal += sz;
//                    Main::processAllPendingEvents();
                    LG_GSTREAMER_D << "write_out_given_data(e) sz:" << sz << " m_source:" << m_source << endl;
                    return 0;
                }
        };


    template<
        typename _CharT,
        typename _Traits = std::char_traits < _CharT >,
        typename _Alloc  = std::allocator   < _CharT >
        >
    class  gstreamer_readFrom_stream
        :
        public Ferris_iostream<_CharT, _Traits>,
//        public StreamHandlableSigEmitter< gstreamer_readFrom_stream<_CharT, _Traits, _Alloc> >,
        public stringstream_methods<_CharT, _Traits, io_ferris_stream_traits< _CharT, _Traits > >
    {
        typedef gstreamer_readFrom_streambuf<_CharT, _Traits, _Alloc> ss_impl_t;
        typedef Loki::SmartPtr<  ss_impl_t,
                                 FerrisLoki::FerrisExRefCounted,
                                 Loki::DisallowConversion,
                                 FerrisLoki::FerrisExSmartPointerChecker,
                                 FerrisLoki::FerrisExSmartPtrStorage > ss_t;
        ss_t ss;
        typedef Ferris_commonstream<_CharT, _Traits> _CS;
        typedef io_ferris_stream_traits< _CharT, _Traits > _FerrisStreamTraits;

    public:

        typedef _Traits                           traits_type;
        typedef typename traits_type::int_type    int_type;
        typedef typename traits_type::char_type   char_type;
        typedef typename traits_type::pos_type    pos_type;
        typedef typename traits_type::off_type    off_type;

        typedef stringstream_methods<
            char_type, traits_type,
            io_ferris_stream_traits< char_type, traits_type > > delegating_methods;
        typedef gstreamer_readFrom_stream< _CharT, _Traits, _Alloc > _Self;


        explicit gstreamer_readFrom_stream(
            const std::string& metadataURL,
            std::ios_base::openmode m = _FerrisStreamTraits::DefaultOpenMode )
            :
            ss( new ss_impl_t( metadataURL ) )
            {
                this->setsbT( GetImpl(ss) );
                this->init( rdbuf() );
            }

        gstreamer_readFrom_stream( const gstreamer_readFrom_stream& rhs )
            :
            ss( rhs.ss )
            {
                this->setsbT( GetImpl(ss) );
                this->init( rdbuf() );
            }

        virtual ~gstreamer_readFrom_stream()
            {
            }


        gstreamer_readFrom_stream& operator=( const gstreamer_readFrom_stream& rhs )
            {
//                 LG_QIO_D << "gstreamer_readFrom_stream& op = " << endl;

                setsb( &rhs );
                init( _CS::sb );
                this->exceptions( std::ios_base::goodbit );
                this->clear( rhs.rdstate() );
                this->copyfmt( rhs );
                return *this;
            }

    
        _Self* operator->()
            {
                return this;
            }

    
        /////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////
    
    
        ss_impl_t*
        rdbuf() const
            {
                return GetImpl(ss);
            }


        enum
        {
            stream_readable = true,
            stream_writable = true
        };

    };
typedef gstreamer_readFrom_stream<char>   fh_gstreamer_readFrom_stream;
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
    };
    
};

#endif
