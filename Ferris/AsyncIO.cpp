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

    $Id: AsyncIO.cpp,v 1.6 2010/09/24 21:30:23 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "Ferris.hh"
#include "FerrisDOM.hh"
#include "AsyncIO.hh"
#include <Ferris/Trimming.hh>

using namespace std;

namespace Ferris
{
    fh_istream null_asyncio_functor( fh_aiohandler, fh_istream s )
    {
        return s;
    }
    
    FERRISEXP_DLLLOCAL gboolean
    AsyncIOHandler_async_cb( GIOChannel *source,
                             GIOCondition condition,
                             gpointer user_data )
    {
        AsyncIOHandler* a = (AsyncIOHandler*)user_data;
        return a->async_cb( source, condition );
    }

    /*
     * group the new data into one block and call the async functor to handle it
     */
    gboolean
    AsyncIOHandler::async_cb( GIOChannel *source, GIOCondition condition )
    {
//        cerr << "AsyncIOHandler::async_cb() pid:" << getpid() << endl;
        
        g_io_channel_set_flags( source, G_IO_FLAG_NONBLOCK, 0 );
        const int buf_sz = 4096;
        char buf[ buf_sz + 2 ];
        gsize bytes_read = 0;
        bool virgin = true;
        GError* e = 0;
        streamsize totalBytesRead = 0;

        buf[buf_sz + 1] = '\0';
        fh_stringstream ss;

        while( virgin || bytes_read )
        {
            virgin = false;
            
            GIOStatus rc = g_io_channel_read_chars( source,
                                                    buf,         buf_sz,
                                                    &bytes_read, &e );
            if( bytes_read )
            {
                totalBytesRead += bytes_read;
                ss.write( buf, bytes_read );
            }
        }

        // cerr << "AsyncIOHandler::async_cb() totalBytesRead:" << totalBytesRead
        //      << " ss:" << tostr(ss)
        //      << endl;
        
        if( totalBytesRead )
        {
//             cerr << "AsyncIOHandler::async_cb() totalBytesRead:" << totalBytesRead
//                  << " ss:" << tostr(ss)
//                  << endl;
            fh_aiohandler a = this;
            theFunctor( a, ss );
        }
        
        
        return 1; // call again
        
    }
    
    AsyncIOHandler::AsyncIOHandler( int fd )
        :
        m_channel( 0 ),
        m_channelID( 0 ),
        theFunctor( null_asyncio_functor )
    {
        if( fd >= 0 )
        {
            attach( fd );
        }
    }
    
    AsyncIOHandler::~AsyncIOHandler()
    {
        release();
    }
    
    void
    AsyncIOHandler::attach( int fd )
    {
        release();

        
        //
        // We need to attach a callback for when data is available from the
        // child.
        //
        GIOCondition cond = GIOCondition(G_IO_IN | G_IO_ERR | G_IO_PRI);

        m_channel = g_io_channel_unix_new( fd );
//        g_io_channel_set_encoding( m_channel, 0, 0 );
        m_channelID = g_io_add_watch( m_channel, cond,
                                      AsyncIOHandler_async_cb, this );
        g_io_channel_set_flags( m_channel, G_IO_FLAG_NONBLOCK, 0 );
//        g_io_channel_set_buffered( m_channel, 0 );
    }
    
    void
    AsyncIOHandler::setFunctor( AsyncIOFunctor_t x )
    {
        theFunctor = x;
    }

    void
    AsyncIOHandler::disconnect()
    {
        if( m_channelID )
        {
//            g_io_channel_set_flags( m_channel, G_IO_FLAG_NONBLOCK, 1 );
            
            g_source_remove( m_channelID );
            m_channelID = 0;
        }
    }
    
    void
    AsyncIOHandler::release()
    {
        disconnect();
        
        if( m_channel )
        {
            g_io_channel_unref( m_channel );
            m_channel = 0;
        }
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

//     template <class StreamClass >
//     void
//     StreamCollector<StreamClass>::attach( fh_runner r )
//     {
//         r->setAsyncStdOutFunctor(
//             Runner::AsyncIOFunctor_t(
//                 this, &_Self::async_io_cb ));
//     }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    XMLMsgStreamCollector::XMLMsgStreamCollector()
        :
        m_msg(""),
        m_overflow("")
    {
    }


    const std::string&
    XMLMsgStreamCollector::getXMLString()
    {
        return m_msg;
    }

    stringmap_t&
    XMLMsgStreamCollector::getStringMap()
    {
        return m_stringmap;
    }

    XMLMsgStreamCollector::MessageArrivedSig_t&
    XMLMsgStreamCollector::getMessageArrivedSig()
    {
        return MessageArrivedSig;
    }

    /**
     * Given a stream that contains possibly more than
     * one message, seek to the end of the first message
     *
     * @return a non empty string if there is another complete XML msg
     *         if there isn't another complete msg then m_overflow contains
     *         the start of the next msg and "" is returned.
     */
    std::string
    XMLMsgStreamCollector::getNextXMLMsg( fh_istream iss )
    {
        fh_stringstream ret;
        
        char lastch = 0;
        char ch     = 0;
        int bracketCount = 0;
        
        for( ; iss >> noskipws >> ch; lastch = ch )
        {
            ret << ch;
            
            if( lastch == '<' )
                if( ch == '/' )
                    --bracketCount;
                else
                    ++bracketCount;
            
            if( ch == '>' && lastch == '?' )
                --bracketCount;
            else if( ch == '>' )
            {
                if( lastch == '/' )
                    --bracketCount;
                
                if( !bracketCount )
                {
                    PrefixTrimmer trimmer;
                    trimmer.push_back( "\n" );
                    trimmer.push_back( " " );
                    trimmer.push_back( "\t" );
                    return trimmer( tostr(ret) );
                }
            }
        }
        m_overflow = tostr(ret);
        return "";
    }
    
    
    fh_istream
    XMLMsgStreamCollector::io_cb( fh_istream iss )
    {

        // cerr << "XMLMsgStreamCollector::io_cb(begin) " 
        //      << " p:" << m_ss.tellp()
        //      << " g:" << m_ss.tellg()
        //      << " m_ss:" << tostr(m_ss)
        //      << endl;

        fh_stringstream msgss;
        msgss << m_overflow << StreamToString( iss ) << flush;

        m_overflow = "";
        msgss.clear();
        msgss.seekg( 0 );

        for( m_msg = getNextXMLMsg( msgss ); !m_msg.empty() ; m_msg = getNextXMLMsg( msgss ) )
        {
            try
            {
//                cerr << "XMLMsgStreamCollector::io_cb(iter) m_msg:" << m_msg << endl;
                m_stringmap.clear();
                fh_stringstream xmlss;
                xmlss << m_msg << flush;
                xmlss.clear();
                xmlss.seekg( 0 );
                XML::readMessage( xmlss, m_stringmap );
//                cerr << "XMLMsgStreamCollector::io_cb(iter have msg)" << endl;

                /*
                 * Emit signal (m_msg and m_stringmap are assumed to be valid at this point)
                 */
                fh_xstreamcol h = this;
                getMessageArrivedSig().emit( h );
            }
            catch( XMLParse& e )
            {
                m_ss.clear();
                m_ss.seekp( ios::end );
                m_ss.seekg( 0 );
//                cerr << "ERROR XMLMsgStreamCollector::async_io_cb(parse e):" << e.what() << endl;
                LG_XML_ER << "XMLMsgStreamCollector::async_io_cb(parse e):" << e.what() << endl;
                return iss;
            }
            catch( exception& e )
            {
//                cerr << "ERROR XMLMsgStreamCollector::async_io_cb(parse std e):" << e.what() << endl;
                LG_XML_ER << "XMLMsgStreamCollector::async_io_cb(parse std e):" << e.what() << endl;
                return iss;
            }
        }
        
        return iss;

        
//         m_ss.clear();
//         m_ss.seekp( ios::end );
// //         copy( istreambuf_iterator<char>(iss),
// //               istreambuf_iterator<char>(),
// //               ostreambuf_iterator<char>(m_ss));
//         {
//             cerr << "---------MANUAL_READ(start)--------------" << endl;
//             char ch = 0;
//             m_ss << flush;
//             while( iss >> noskipws >> ch )
//             {
//                 cerr << ch;
//                 m_ss << ch;
//                 msgss << ch;
//             }
//             cerr << "---------MANUAL_READ(end)--------------" << endl;
//         }
        
//         m_ss << flush;

//         msgss.clear();
//         msgss.seekg( 0 );

//         while( seekToEndOfXMLMsg( msgss ) )
//         {
//             try
//             {
//             }
            
//         }
        
        
//         try
//         {
// //             m_ss.clear();
// //             m_ss.seekg( 0 );
// //             m_stringmap.clear();
// //             cerr << "XMLMsgStreamCollector::io_cb(readmsg) from:" << tostr(m_ss) << endl;
// //             XML::readMessage( m_ss, m_stringmap );

//             msgss.clear();
//             msgss.seekg( 0 );
//             m_stringmap.clear();
//             cerr << "XMLMsgStreamCollector::io_cb(readmsg) from:" << tostr(msgss) << endl;
//             XML::readMessage( msgss, m_stringmap );

//         }
//         catch( XMLParse& e )
//         {
//             m_ss.clear();
//             m_ss.seekp( ios::end );
//             m_ss.seekg( 0 );
//             cerr << "XMLMsgStreamCollector::async_io_cb(parse e):" << e.what() << endl;
//             return iss;
//         }
//         catch( exception& e )
//         {
//             LG_XML_W << "XMLMsgStreamCollector::async_io_cb(parse std e):" << e.what() << endl;
//             cerr << "XMLMsgStreamCollector::async_io_cb(parse std e):" << e.what() << endl;
//             return iss;
//         }
        
// //        cerr << "XMLMsgStreamCollector::async_io_cb(cleanup) " << endl;

//         /*
//          * Grab the remaining data after what the XML parse used and keep it for
//          * the next XML msg.
//          */
//         streamsize tellg = m_ss.tellg();
//         fh_stringstream tmp;
//         copy( istreambuf_iterator<char>(m_ss),
//               istreambuf_iterator<char>(),
//               ostreambuf_iterator<char>(tmp));
//         cerr << "XMLMsgStreamCollector::async_io_cb() tellg:" << tellg
//              << " tmp:" << tostr(tmp)
//              << endl;

//         /*
//          * We need to read the whole XML message as a string to send to the clients
//          */
//         {
// //             m_ss.clear();
// //             m_ss.seekg(0);
// //             m_ss.seekp(0);
// //             fh_istream limitedss = Factory::MakeLimitingIStream( m_ss, 0, tellg );

//             msgss.clear();
//             msgss.seekg(0);
//             msgss.seekp(0);
//             fh_istream limitedss = Factory::MakeLimitingIStream( msgss, 0, tellg );

//             fh_stringstream z;
//             copy( istreambuf_iterator<char>(limitedss),
//                   istreambuf_iterator<char>(),
//                   ostreambuf_iterator<char>(z));
//             m_msg = tostr(z);
// //             cerr << "XMLMsgStreamCollector::async_io_cb() tellg:" << tellg
// //                  << " msg:" << m_msg
// //                  << endl;
//         }

//         m_overflow = tostr(tmp);
        
//         /*
//          * start m_ss over with just the remaining data that was
//          * not processed in the last xml msg
//          */
//         m_ss = tmp;
//         m_ss.clear();
// //        m_ss.str( tostr(tmp) );
//         m_ss.seekg( 0 );
//         m_ss.seekp( 0 + tostr(tmp).length() );
//         m_ss.clear();
        
// //         cerr << "XMLMsgStreamCollector::async_io_cb(end) " 
// //              << " p:" << m_ss.tellp()
// //              << " g:" << m_ss.tellg()
// //              << " tostr:" << tostr(m_ss)
// //              << endl;
        

//         /*
//          * Emit signal (m_msg and m_stringmap are assumed to be valid at this point)
//          */
//         fh_xstreamcol h = this;
//         getMessageArrivedSig().emit( h );
        
//         return iss;
    }


    
    


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    
    namespace Factory
    {
        fh_sstreamcol MakeStringStreamCol()
        {
            return new StringStreamCollector();
        }
        
        fh_fstreamcol MakeFileStreamCol( const std::string& s,
                                         std::ios_base::openmode m )
        {
            fh_fstream ss( s, m );
            return new FileStreamCollector( ss );
        }

        fh_xstreamcol MakeXMLStreamCol()
        {
            return new XMLMsgStreamCollector();
        }
        
        
    };
    
};
