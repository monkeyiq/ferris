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

    $Id: AsyncIO.hh,v 1.5 2010/09/24 21:30:23 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_ASYNCIO_H_
#define _ALREADY_INCLUDED_FERRIS_ASYNCIO_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <Ferris/FerrisException.hh>
#include <Ferris/FerrisHandle.hh>
#include <Ferris/SignalStreams.hh>
#include <Ferris/Runner.hh>
#include <Ferris/Runner_FunctorType.hh>
//#include <Ferris/ChildStreamServer.hh>

#include <glib.h>

#include <sigc++/sigc++.h>
#include <SmartPtr.h>

namespace Ferris
{
    /**
     * Hookup async IO using g io channels and the main GLib loop.
     */
    FERRISEXP_API fh_istream null_asyncio_functor( fh_aiohandler, fh_istream s );

    class FERRISEXP_API AsyncIOHandler
        :
        public Handlable
    {
        GIOChannel* m_channel;
        guint       m_channelID;

        friend FERRISEXP_DLLLOCAL gboolean AsyncIOHandler_async_cb( GIOChannel *source,
                                                                    GIOCondition condition,
                                                                    gpointer user_data );
        gboolean async_cb( GIOChannel *source, GIOCondition condition );
        
            
    public:

        typedef Loki::Functor< fh_istream,
                               LOKI_TYPELIST_2( fh_aiohandler, fh_istream ) > AsyncIOFunctor_t;
        AsyncIOFunctor_t theFunctor;
        
        AsyncIOHandler( int fd = -1 );
        virtual ~AsyncIOHandler();

        /*
         * Attach internal callbacks on the given fd
         */
        void attach( int fd );

        /*
         * set functor that will be called when data is available
         */
        void setFunctor( AsyncIOFunctor_t x = null_asyncio_functor );

        /*
         * disconnect gio channels for async io on child.
         */
        void disconnect();
        /*
         * unref and zero gio channels for async io on child.
         */
        void release();
        
    };


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Use this class to collect async io to a stream, primarily useful for reading
     * all the data of a child and then processing it when the child terminates.
     *
     * see fh_sstreamcol and fh_fstreamcol and their factory methods
     * MakeStringStreamCol() and MakeFileStreamCol( )
     *
     */
    template <class StreamClass = fh_stringstream >
    class StreamCollector
        :
        public Handlable
    {
        typedef StreamCollector<StreamClass> _Self;
        
    protected:

        StreamClass m_ss;
        std::streamsize  m_totalsz;
        std::streamsize  m_donesz;
//        fh_childserv m_ChildStreamServer;
    
    public:

        StreamCollector( StreamClass oss = StreamClass() )
            :
            m_ss( oss ),
            m_totalsz( 0 ),
            m_donesz( 0 )
//            m_ChildStreamServer( 0 )
            {
                reset();
            }

        /*
         * get the stream that has collected all the data from callbacks
         */
        StreamClass& getStream()
            {
                return m_ss;
            }

        void setStream( const StreamClass& ss )
            {
                m_ss = ss;
            }
        

        /*
         * reset internal state to starting a new collection
         */
        void reset()
            {
                m_totalsz = 0;
                m_donesz = 0;
        
                m_ss.clear();
                m_ss.seekg(0);
                m_ss.seekp(0);
            }
        

        /*
         * collect the data that has arrived from child process
         */
        virtual fh_istream io_cb( fh_istream iss )
            {
//                std::cerr << "StreamCollector::async_io_cb(top) donesz:" << this->m_donesz << std::endl;
        
                std::streamsize readsz = 0;
                const int bufsz = 1024;
                char buf[ bufsz + 1 ];
                while( true )
                {
                    iss.read( buf, bufsz );
                    readsz = iss.gcount();
                    if( !readsz )
                        break;
            
                    m_ss.write( buf, readsz );
                    m_donesz += readsz;
                }
        
                return iss;
            }

        virtual fh_istream aioh_io_cb( fh_aiohandler h, fh_istream iss )
            {
                return io_cb( iss );
            }
        
        virtual fh_istream runner_io_cb( fh_runner r, fh_istream iss )
            {
                return io_cb( iss );
            }
        
        

        /*
         * hookup to the data available signal on the given runner.
         * we will collect all the information provided through this
         * runners stdout
         */
        void attach( fh_runner r )
            {
                Private::attachStreamCollector( r, Runner_AsyncIOFunctor_t(
                                                    this, &_Self::runner_io_cb ));
//                 r->setAsyncStdOutFunctor(
//                     Runner_AsyncIOFunctor_t(
//                         this, &_Self::runner_io_cb ));
            }


//         void attachWithComplete( fh_runner r )
//             {
//                 attach( r );
//                 m_ChildStreamServer = new ChildStreamServer();
//                 m_ChildStreamServer->addChild( r );
//                 m_ChildStreamServer->getChildCompleteSig().connect( sigc::mem_fun( *this, &_Self::OnChildComeplete ) );
//             }
        

//         typedef sigc::signal4< void, ChildStreamServer*, fh_runner, int, int > ChildCompleteSig_t;
//         ChildCompleteSig_t ChildCompleteSig;
//         ChildCompleteSig_t& getChildCompleteSig()
//             {
//                 return ChildCompleteSig;
//             }
        
//         void OnChildComeplete( ChildStreamServer* css, fh_runner r, int status, int estatus )
//             {
//                 ChildCompleteSig.emit( css, r, status, estatus );
//             }
        
        void attach( fh_aiohandler h )
            {
                h->setFunctor( AsyncIOHandler::AsyncIOFunctor_t( this, &_Self::aioh_io_cb ));
            }
        
        
    };
    
    typedef StreamCollector< fh_stringstream > StringStreamCollector;
    FERRIS_SMARTPTR( StringStreamCollector, fh_sstreamcol );

    typedef StreamCollector< fh_fstream > FileStreamCollector;
    FERRIS_SMARTPTR( FileStreamCollector, fh_fstreamcol );

    

    /**
     * See Factory::MakeXMLStreamCol() to create one.
     *
     * This manages a buffer and when a valid XML document arrives then a
     * signal is emited and others can query for the data.
     *
     * Primarily for use with writeMessage() and readMessage().
     **/
    class FERRISEXP_API XMLMsgStreamCollector
        :
        public StringStreamCollector
    {
        typedef StringStreamCollector _Base;
        typedef XMLMsgStreamCollector _Self;

        std::string m_msg;
        std::string m_overflow;
        stringmap_t m_stringmap;

        std::string getNextXMLMsg( fh_istream iss );
        
    public:

        XMLMsgStreamCollector();
        virtual fh_istream io_cb( fh_istream iss );

        
        typedef sigc::signal< void ( fh_xstreamcol ) > MessageArrivedSig_t;
        MessageArrivedSig_t& getMessageArrivedSig();
        
        const std::string& getXMLString();
        stringmap_t& getStringMap();


    private:
        MessageArrivedSig_t MessageArrivedSig;
    };

    /****************************************/
    /****************************************/
    /****************************************/

    namespace Factory
    {
        FERRISEXP_API fh_sstreamcol MakeStringStreamCol();
        FERRISEXP_API fh_fstreamcol MakeFileStreamCol( const std::string& s,
                                         std::ios_base::openmode m = std::ios_base::out );
        FERRISEXP_API fh_xstreamcol MakeXMLStreamCol();
    };
};
#endif
