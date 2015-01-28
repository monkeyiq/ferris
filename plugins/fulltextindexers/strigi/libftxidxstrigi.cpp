/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: libftxidxstrigi.cpp,v 1.2 2008/12/19 21:30:13 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "strigi/asyncsocketclient.h"

// #include <strigi/strigi.h>

#include <Ferris/DBus_private.hh>
#include <Ferris/FullTextIndexerMetaInterface.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FullTextIndexer_private.hh>
#include <Ferris/EAIndexerSQLCommon_private.hh>
#include <Ferris/FullTextIndexerSyntheticDocID_private.hh>

#include <string>
using namespace std;

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


namespace Ferris
{
    namespace FullTextIndex 
    {
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        class FERRISEXP_DLLLOCAL FullTextIndexerSTRIGI
            :
            public FullTextIndexerSyntheticDocID
        {
//             DBusConnection* m_dbus;
//             DBusMessage* m_getHits;

            AsyncSocketClient m_strigi;
            
        protected:


            
            
            virtual void Setup();
            virtual void CreateIndexBeforeConfig( fh_context c,
                                                  bool caseSensitive,
                                                  bool dropStopWords,
                                                  StemMode stemMode,
                                                  const std::string& lex_class,
                                                  fh_context md );
            virtual void CreateIndex( fh_context c,
                                      bool caseSensitive,
                                      bool dropStopWords,
                                      StemMode stemMode,
                                      const std::string& lex_class,
                                      fh_context md );
            virtual void CommonConstruction();
            
            virtual void addToIndex( fh_context c,
                                     fh_docindexer di );

            virtual docNumSet_t& addAllDocumentsMatchingTerm(
                const std::string& term,
                docNumSet_t& output,
                int limit );

        public:

            FullTextIndexerSTRIGI();
            virtual ~FullTextIndexerSTRIGI();

            
            virtual
            docNumSet_t&
            ExecuteStrigiFullTextQuery( const std::string& queryString,
                                        docNumSet_t& docnums,
                                        int limit );
            
        };

        struct FullTextIndexerSTRIGIData
        {
            FullTextIndexerSTRIGI* idx;
            docNumSet_t* output;
            bool looping;

            FullTextIndexerSTRIGIData(
                FullTextIndexerSTRIGI* idx, 
                docNumSet_t* output )
                :
                looping( true ),
                idx( idx ),
                output( output )
                {
                }
        };
        
        
        /************************************************************/
        /************************************************************/
        /************************************************************/

        FullTextIndexerSTRIGI::FullTextIndexerSTRIGI()
//            :
//            m_strigi( 0 )
//             m_getHits( 0 ),
//             m_dbus( 0 )
        {
        }

        FullTextIndexerSTRIGI::~FullTextIndexerSTRIGI()
        {
//             if( m_getHits )
//             {
//                 dbus_message_unref( m_getHits );
//             }
        }

        
        
        void
        FullTextIndexerSTRIGI::Setup()
        {
            LG_IDX_D << "Setup(start)" << endl;

            m_strigi.setSocketPath( Shell::getHomeDirPath() + "/.strigi/socket");

//             m_dbus = DBus::getSessionBus();
            
//             const char * destination = 0;
//             m_getHits = dbus_message_new_method_call(
//                 destination,
//                 "/search",
//                 "vandenoever.strigi",
//                 "getHits" );

            LG_IDX_D << "Setup(done)" << endl;
        }

        void
        FullTextIndexerSTRIGI::CreateIndexBeforeConfig( fh_context c,
                                                          bool caseSensitive,
                                                          bool dropStopWords,
                                                          StemMode stemMode,
                                                          const std::string& lex_class,
                                                          fh_context md )
        {
        }
        
        
        void
        FullTextIndexerSTRIGI::CreateIndex( fh_context c,
                                              bool caseSensitive,
                                              bool dropStopWords,
                                              StemMode stemMode,
                                              const std::string& lex_class,
                                              fh_context md )
        {
            Setup();
        }

        void
        FullTextIndexerSTRIGI::CommonConstruction()
        {
        }


        void
        FullTextIndexerSTRIGI::addToIndex( fh_context c, fh_docindexer di )
        {
            stringstream ss;
            ss << "Sorry, adding to Strigi indexes is still a work in progress." << endl;
            Throw_IndexException( ss.str(), 0 );
        }

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        
        
        docNumSet_t&
        FullTextIndexerSTRIGI::addAllDocumentsMatchingTerm(
            const std::string& term_const,
            docNumSet_t& output,
            int limit )
        {
            return ExecuteStrigiFullTextQuery( term_const, output, limit );
        }

        
        docNumSet_t&
        FullTextIndexerSTRIGI::ExecuteStrigiFullTextQuery( const std::string& queryStringConst,
                                                           docNumSet_t& output,
                                                           int limit )
        {
            string queryString = queryStringConst;
            long max = 100;
            long offset = 0;
            LG_IDX_D << "ExecuteStrigiFullTextQuery() query:" << queryString << endl;
            cerr << "ExecuteStrigiFullTextQuery() query:" << queryString << endl;


            bool ok = m_strigi.query( queryString.c_str(), max, offset );
            if( !ok )
            {
                stringstream ss;
                ss << "error sending query to strigi daemon" << endl;
                Throw_IndexException( ss.str(), 0 );
            }

            while (!m_strigi.statusChanged())
            {
                struct timespec sleeptime;
                sleeptime.tv_sec = 0;
                sleeptime.tv_nsec = 10000;
                nanosleep(&sleeptime, 0);
//                cerr << "x" << flush;
            }

            cerr << "hits:" << m_strigi.getHits().hits.size() << endl;
            typedef std::vector<Strigi::IndexedDocument>::const_iterator ITER;
            const std::vector<Strigi::IndexedDocument>& hits = m_strigi.getHits().hits;
            ITER e = hits.end();
            for( ITER iter = hits.begin(); iter != e; ++iter )
            {
                const Strigi::IndexedDocument& id = *iter;
                string earl = id.uri;
                docid_t docid = getDocID( earl );
                output.insert( docid );
            }
            

//             DBusMessage* m = dbus_message_copy( m_getHits );
//             DBusPendingCall* pending = 0;
//             const char* str1 = queryString.c_str();
//             cerr << "adding qs:" << str1 << endl;
//             dbus_message_append_args( m,
//                                       DBUS_TYPE_STRING, &str1,
//                                       DBUS_TYPE_INVALID  );
//             cerr << "adding max:" << max << endl;
//             dbus_message_append_args( m,
//                                       DBUS_TYPE_INT16, &max,
//                                       DBUS_TYPE_INVALID  );
//             cerr << "adding offset:" << offset << endl;
//             dbus_message_append_args( m,
//                                       DBUS_TYPE_INT16, &offset,
//                                       DBUS_TYPE_INVALID  );

//             cerr << "calling send..." << endl;
            
//             int timeout_milliseconds = 0;
//             if( !dbus_connection_send_with_reply( m_dbus, m, &pending, timeout_milliseconds ) )
//             {
//                 LG_DBUS_W << "DBUS: error sending message" << endl;
//                 stringstream ss;
//                 ss << "error sending message to strigi daemon" << endl;
//                 Throw_IndexException( ss.str(), 0 );
//             }
//             if (!pending)
//             { 
//                 LG_DBUS_W << "DBUS: error sending message" << endl;
//                 stringstream ss;
//                 ss << "error sending message to strigi daemon" << endl;
//                 Throw_IndexException( ss.str(), 0 );
//             }
//             dbus_connection_flush(m_dbus);
//             dbus_message_unref(m);

//             bool stat;
//             dbus_uint32_t level;
        
//             // block until we receive a reply
//             dbus_pending_call_block(pending);
            
//             // get the reply message
//             m = dbus_pending_call_steal_reply(pending);
//             if (!m)
//             {
//                 LG_DBUS_W << "DBUS: error sending message" << endl;
//                 stringstream ss;
//                 ss << "error sending message to strigi daemon" << endl;
//                 Throw_IndexException( ss.str(), 0 );
//             }
//             // free the pending message handle
//             dbus_pending_call_unref(pending);

//             DBusMessageIter args;
            
//             if (!dbus_message_iter_init(m, &args))
//             {
//                 LG_DBUS_W << "DBUS: error sending message" << endl;
//                 stringstream ss;
//                 ss << "error sending message to strigi daemon" << endl;
//                 Throw_IndexException( ss.str(), 0 );
//             }

// //             StrigiHit *hit = (StrigiHit *)l->data;
// //             string earl = strigi_hit_get_uri (hit);
// //             docid_t docid = getDocID( earl );
// //             output->insert( docid );
                
//             cerr << "Reply arg1 type:" << dbus_message_iter_get_arg_type(&args) << endl;
//             {
//                 dbus_uint64_t value;
//                 dbus_message_iter_get_basic (&args, &value);
//                 cerr << "Reply arg1 v:" << (char*)value << endl;
                
//             }
//             for( int i=0; dbus_message_iter_next(&args); ++i )
//             {
//                 cerr << "Reply arg-" << i << " type:" << dbus_message_iter_get_arg_type(&args) << endl;
//             }
            
            
//             dbus_message_unref(m);

            return output;
        }
        
        
        /**************************************************/
        /**************************************************/

        
    };
};

extern "C"
{
    FERRISEXP_API Ferris::FullTextIndex::MetaFullTextIndexerInterface* Create()
    {
        return new Ferris::FullTextIndex::FullTextIndexerSTRIGI();
    }
};
