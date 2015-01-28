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

    $Id: libftxidxbeagle.cpp,v 1.2 2008/12/19 21:30:12 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

extern "C" {
#include <beagle/beagle.h>
};

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
        
        class FERRISEXP_DLLLOCAL FullTextIndexerBEAGLE
            :
            public FullTextIndexerSyntheticDocID
        {
            BeagleClient* m_client;


            
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

            FullTextIndexerBEAGLE();
            virtual ~FullTextIndexerBEAGLE();

            void
            hits_added_cb (BeagleQuery *query,
                           BeagleHitsAddedResponse *response,
                           docNumSet_t* output );
            
            virtual
            docNumSet_t&
            ExecuteBeagleFullTextQuery( const std::string& queryString,
                                        docNumSet_t& docnums,
                                        int limit );
            
        };

        struct FullTextIndexerBEAGLEData
        {
            FullTextIndexerBEAGLE* idx;
            docNumSet_t* output;
            bool looping;

            FullTextIndexerBEAGLEData(
                FullTextIndexerBEAGLE* idx, 
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

        FullTextIndexerBEAGLE::FullTextIndexerBEAGLE()
            :
            m_client( 0 )
        {
        }

        FullTextIndexerBEAGLE::~FullTextIndexerBEAGLE()
        {
            if( m_client )
            	g_object_unref( m_client );

        }

        
        
        void
        FullTextIndexerBEAGLE::Setup()
        {
            g_type_init();
            
            m_client = beagle_client_new( 0 );
            LG_IDX_D << "Setup()" << endl;
        }

        void
        FullTextIndexerBEAGLE::CreateIndexBeforeConfig( fh_context c,
                                                          bool caseSensitive,
                                                          bool dropStopWords,
                                                          StemMode stemMode,
                                                          const std::string& lex_class,
                                                          fh_context md )
        {
        }
        
        
        void
        FullTextIndexerBEAGLE::CreateIndex( fh_context c,
                                              bool caseSensitive,
                                              bool dropStopWords,
                                              StemMode stemMode,
                                              const std::string& lex_class,
                                              fh_context md )
        {
            Setup();
        }

        void
        FullTextIndexerBEAGLE::CommonConstruction()
        {
        }


        void
        FullTextIndexerBEAGLE::addToIndex( fh_context c, fh_docindexer di )
        {
            stringstream ss;
            ss << "Sorry, adding to Beagle indexes is still a work in progress." << endl;
            Throw_IndexException( ss.str(), 0 );
            
            LG_BEAGLE_D << "addToIndexDocTermsClass() c:" << c->getURL() << endl;

            string s;
            fh_istream iss;
            try
            {
                fh_attribute a = c->getAttribute( "as-text" );
                iss = a->getIStream();
            }
            catch( exception& e )
            {
                LG_BEAGLE_W << "WARNING, Failed to obtain plaintext for url:" << c->getURL()
                            << " error:" << e.what() << endl;
                return;
            }
            streamsize bytesDone    = 0;
            streamsize signalWindow = 0;
            streamsize totalBytes   = toType<streamsize>(getStrAttr( c, "size", "0" ));

            string earl = c->getURL();
            BeagleIndexable* bi = beagle_indexable_new( earl.c_str() );

            beagle_indexable_set_hit_type( bi, "libferris" );
            beagle_indexable_set_mime_type( bi, "text/plain" );

            BeagleTimestamp* btime = beagle_timestamp_new_from_unix_time( Time::getTime() );
            beagle_indexable_set_timestamp( bi, btime );
//            g_object_unref( btime );

            s = getStrAttr( c, "title", "" );
            if( !s.empty() )
            {
                BeagleProperty* prop = beagle_property_new( BEAGLE_PROPERTY_TYPE_KEYWORD,
                                                            "dc:title", s.c_str() );
                beagle_indexable_add_property( bi, prop );
                g_object_unref( prop );
            }

            string tempFileName = "/tmp/libferris-beagle-indexing-tempfile";
            
            {
                fh_iostream ios = Shell::generteTempFile( tempFileName );
                copy( istreambuf_iterator<char>(iss),
                      istreambuf_iterator<char>(),
                      ostreambuf_iterator<char>(ios));
                ios << flush;
            }
            beagle_indexable_set_content_uri( bi, tempFileName.c_str() );
            beagle_indexable_set_delete_content( bi, true );

            LG_BEAGLE_D << "Adding earl:" << earl
                        << " contents at:" << tempFileName
                        << endl;

            BeagleIndexingServiceRequest* req = beagle_indexing_service_request_new();
            beagle_indexing_service_request_add( req, bi );

            GError *error = NULL;
            BeagleResponse* bres = beagle_client_send_request( m_client, BEAGLE_REQUEST(req), &error );
            g_object_unref( req );
            g_object_unref( bres );

            if (error != NULL)
            {
                LG_BEAGLE_D << "Error adding file:" << error->message << endl;
                g_error_free (error);
            } 

            LG_BEAGLE_D << "Added earl:" << earl << endl;
        }

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        
        
        docNumSet_t&
        FullTextIndexerBEAGLE::addAllDocumentsMatchingTerm(
            const std::string& term_const,
            docNumSet_t& output,
            int limit )
        {
            return ExecuteBeagleFullTextQuery( term_const, output, limit );
        }

        
        static void
        hits_added_scb (BeagleQuery *query, BeagleHitsAddedResponse *response, void *user_data)
        {
            FullTextIndexerBEAGLEData* d = (FullTextIndexerBEAGLEData*) user_data;
            d->idx->hits_added_cb( query, response, d->output );
        }

        void
        FullTextIndexerBEAGLE::hits_added_cb (BeagleQuery *query,
                                              BeagleHitsAddedResponse *response,
                                              docNumSet_t* output )
        {
            GSList *hits, *l;
            gint    i;

            hits = beagle_hits_added_response_get_hits (response);

            for (l = hits, i = 1; l; l = l->next, ++i)
            {
                BeagleHit *hit = (BeagleHit *)l->data;

                string earl = beagle_hit_get_uri (hit);
                docid_t docid = getDocID( earl );
                output->insert( docid );

//                 cerr << "uri:" << beagle_hit_get_uri (hit) << endl;
//                 cerr << "score:" << beagle_hit_get_score (hit) << endl;
            }
        }


        
        static void
        finished_cb (BeagleQuery            *query,
                     BeagleFinishedResponse *response, 
                     void* user_data )
        {
            FullTextIndexerBEAGLEData* d = (FullTextIndexerBEAGLEData*) user_data;
            d->looping = false;
            cerr << "done." << endl;
        }
        

        docNumSet_t&
        FullTextIndexerBEAGLE::ExecuteBeagleFullTextQuery( const std::string& queryStringConst,
                                                           docNumSet_t& output,
                                                           int limit )
        {
            string queryString = queryStringConst;

            LG_IDX_D << "ExecuteBeagleFullTextQuery() query:" << queryString
                     << endl;
            
            BeagleQuery* q = beagle_query_new();
            beagle_query_add_text (q, queryString.c_str() );

            FullTextIndexerBEAGLEData* d = new FullTextIndexerBEAGLEData( this, &output );
            g_signal_connect (q, "hits-added", G_CALLBACK (hits_added_scb), d );
            g_signal_connect (q, "finished",   G_CALLBACK (finished_cb),   d );
            beagle_client_send_request_async ( m_client, BEAGLE_REQUEST (q), NULL);

            
            while( d->looping )
            {
                Main::processAllPendingEvents();
                
                g_usleep(50);
            }
            g_object_unref( q );

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
        return new Ferris::FullTextIndex::FullTextIndexerBEAGLE();
    }
};
