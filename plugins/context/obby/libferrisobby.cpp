/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2005 Ben Martin

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

    $Id: libferrisobby.cpp,v 1.3 2010/09/24 21:31:40 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#define LIBFERRIS__DONT_INCLUDE_SIGCC_EXTENSIONS

#include <FerrisContextPlugin.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/Trimming.hh>
#include <Ferris/General.hh>
#include <Ferris/Cache.hh>
#include <Ferris/Runner.hh>

#include <algorithm>
#include <numeric>

#include <config.h>

#include <obby/client_buffer.hpp>
#include <obby/chat.hpp>

using namespace std;

#include "gselectorglib.hh"
#include "libferrisobbyshared.hh"

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    typedef obby::basic_client_buffer< obby::document, GSelectorGlib> buffer_t;
    typedef obby::basic_client_document_info< obby::document, GSelectorGlib> client_docinfo_t;
    typedef obby::basic_document_info< obby::document, GSelectorGlib> docinfo_t;
//    typedef obby::basic_client_buffer< obby::document, Gobby::GSelector> buffer_t;
//    typedef obby::basic_client_buffer< obby::document, net6::selector > buffer_t;
    typedef Loki::SmartPtr<
        buffer_t,
        Loki::RefCounted,
        Loki::DisallowConversion,
        Loki::AssertCheck,
        Loki::DefaultSPStorage > fh_buffer;
    
    
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_CTXPLUGIN obbyDocumentContext
        :
        public StateLessEAHolder< obbyDocumentContext, FakeInternalContext >
    {
        typedef StateLessEAHolder< obbyDocumentContext, FakeInternalContext > _Base;
        typedef obbyDocumentContext _Self;
        
        buffer_t::document_info_type* m_di;

    protected:

        void
        priv_read()
            {
                stringstream ss;
                ss << "FerrisNotReadableAsContext for url:" << getURL();
                Throw_FerrisNotReadableAsContext( tostr(ss), this );
            }
        
        ferris_ios::openmode getSupportedOpenModes()
            {
                return
                    ios_base::in        |
                    ios_base::out       |
                    ios_base::trunc     |
                    ios_base::binary    ;
            }

        fh_stringstream priv_getRealStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                LG_OBBY_D << "priv_getStream() URL:" << getURL() << endl;

                if( !m_di )
                {
                    fh_stringstream ss;
                    ss << "Do not have document information for this file!"
                       << " this is a logic error in libferris. Please report it"
                       << " preferably with a backtrace!" << endl;
                    Throw_CanNotGetStream( tostr(ss), this );
                }
                
                fh_stringstream ss;
                if( !( m & ios_base::trunc ) )
                {
                    const obby::document& doc = m_di->get_content();
                    ss << doc.get_text();
                }
                return ss;
            }

        virtual void OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
                AdjustForOpenMode_Closing( ss, m, tellp );
                const std::string s = StreamToString(ss);

                LG_OBBY_D << "Replacement string:" << s << endl;

                int sz = 0;
                {
                    const obby::document& doc = m_di->get_content();
                    sz = doc.get_text().length();
                }
                if( sz > 0 )
                    m_di->erase( 0, sz );
                if( !s.empty() )
                    m_di->insert( 0, s );
                LG_OBBY_D << "sz:" << sz << endl;

                cerr << "OnStreamClosed() processing" << endl;
                Main::processAllPendingEvents();
            }
        
        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ret = priv_getRealStream( m );
                ret->getCloseSig().connect( bind( SigC::slot( *this, &_Self::OnStreamClosed ), m ));
                return ret;
            }
        
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   exception)
            {
                return priv_getRealStream( m );
            }
        
        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    _Base::createStateLessAttributes( true );
                }
            }
    public:

        obbyDocumentContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_di( 0 )
            {
                createStateLessAttributes( true );
            }

        void constructObject( buffer_t::document_info_type* di )
            {
                m_di = di;
            }
        
    };
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    class FERRISEXP_CTXPLUGIN obbyInstanceContext
        :
        public StateLessEAHolder< obbyInstanceContext, FakeInternalContext >
    {
        typedef obbyInstanceContext _Self;
        typedef StateLessEAHolder< obbyInstanceContext, FakeInternalContext > _Base;

        fh_buffer m_buffer;
        bool m_loggedIn;
        guint m_documentsInitiallySubscribedCounter;
        guint m_documentsInitiallySubscribedTotal;
        bool m_loginHasFailed;
        obby::colour m_color;

        void on_sync_init(unsigned int count)
            {
                LG_OBBY_D << "on_sync_init count:" << count << endl;
            }

        void on_subscribe( const obby::user& u, buffer_t::document_info_type* di )
            {
                LG_OBBY_D << "on_subscribe" << endl;
                LG_OBBY_D << "my user id:" << u.get_id() << endl;
                LG_OBBY_D << "document subscribed too:" << di->get_suffixed_title() << endl;
                cerr << "document subscribed too:" << di->get_suffixed_title() << endl;

                string rdn = di->get_suffixed_title();
                obbyDocumentContext* cc = 0;
                cc = priv_ensureSubContext( rdn, cc );
                cc->constructObject( di );
                
                ++m_documentsInitiallySubscribedCounter;
                if( m_documentsInitiallySubscribedCounter >=
                    m_documentsInitiallySubscribedTotal )
                {
                    m_loggedIn = true;
                }
            }
        
        void on_message( const obby::chat::message& m )
            {
                LG_OBBY_D << "on_message() time:" << m.format_timestamp("%a, %e %b %Y %H:%M:%S")
                     << " text:" << m.get_text()
                     << " repr:" << m.repr() 
                     << endl;
                if( const obby::chat::user_message* um = dynamic_cast<const obby::chat::user_message*>(&m))
                {
                    LG_OBBY_D << "user message... user:" << um->get_user().get_id() << endl;
                }
            }
        
        void on_sync_final()
            {
                LG_OBBY_D << "on_sync_final()" << endl;
    
                {
                    const obby::chat& c = m_buffer->get_chat();
                    c.message_event().connect( sigc::mem_fun( this, &_Self::on_message ) );
                }
                m_buffer->send_message( "tadaima!" );

                m_documentsInitiallySubscribedTotal = 0;
                m_documentsInitiallySubscribedCounter = 0;
                buffer_t::document_iterator iter = m_buffer->document_begin();
                buffer_t::document_iterator e    = m_buffer->document_end();
                typedef buffer_t::document_info_type docInfo_t;
                for( ; iter != e ; ++iter )
                {
                    docInfo_t& di = dynamic_cast< docInfo_t& >( *iter );

                    LG_OBBY_D << "------------------" << endl;
                    LG_OBBY_D << "docid:" << iter->get_id() << endl;
                    LG_OBBY_D << "title:" << iter->get_title() << endl;
                    LG_OBBY_D << "utitle:" << iter->get_suffixed_title() << endl;
                    LG_OBBY_D << "owner:" << iter->get_owner() << endl;
                    LG_OBBY_D << "owner-id:" << iter->get_owner_id() << endl;

                    ++m_documentsInitiallySubscribedTotal;
                    di.subscribe_event().connect( bind( sigc::mem_fun( this, &_Self::on_subscribe ), &di ) );
                    di.subscribe();
                }

            }
        
                
        
        void on_welcome()
            {
                LG_OBBY_D << "on_welcome()" << endl;
                
                m_buffer->sync_init_event().connect( sigc::mem_fun( this, &_Self::on_sync_init ) );
                m_buffer->sync_final_event().connect( sigc::mem_fun( this, &_Self::on_sync_final ) );
                userpass_t up = getOBBYUserPass( getDirName() );
                string m_username = up.first;
                m_buffer->login( m_username, m_color );
            }

        void on_login_failed(obby::login::error error)
            {
                LG_OBBY_W << "login failed:" << obby::login::errstring(error) << endl;
                m_loginHasFailed = true;
            }

        void on_close()
            {
                LG_OBBY_D << "Connection lost" << endl;
                m_loginHasFailed = true;
            }

        void on_obby_document_insert( docinfo_t& di )
            {
                string rdn = di.get_suffixed_title();
                cerr << "on_obby_document_insert di:" << rdn << endl;
                obbyDocumentContext* cc = 0;
                cc = priv_ensureSubContext( rdn, cc );
                cc->constructObject( dynamic_cast<buffer_t::document_info_type*>(&di) );
            }
        
    protected:
        
        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    _Base::createStateLessAttributes( true );
                }
            }

        void
        priv_read()
            {
                LG_OBBY_D << "priv_read() url:" << getURL() << endl;

                EnsureStartStopReadingIsFiredRAII _raii( this );
//                EnsureStartReadingIsFired();

                if( !m_buffer )
                {
                    clearContext();
                    
                    string hostname = getDirName();
                    unsigned int port = 6522;
                    
                    m_buffer = new buffer_t();
                    m_buffer->welcome_event().connect( sigc::mem_fun( this, &_Self::on_welcome) );
                    m_buffer->login_failed_event().connect(  sigc::mem_fun( this, &_Self::on_login_failed) );
                    m_buffer->close_event().connect(  sigc::mem_fun( this, &_Self::on_close) );
                    m_buffer->connect( hostname, port );

                    m_buffer->document_insert_event().connect(
                        sigc::mem_fun( this, &_Self::on_obby_document_insert) );
                    
                    while( !m_loggedIn && !m_loginHasFailed )
                    {
                        g_main_iteration( false );
//                        Main::processAllPendingEvents();
                    }
                }
                
//                 EnsureStopReadingIsFired();
//                 updateMetaData();
                
            }

        fh_context
        SubCreate_file( fh_context c, fh_context md )
            {
                std::string rdn      = getStrSubCtx( md, "name", "" );
                std::string v        = "";

                if( _Self* edbc = dynamic_cast<_Self*>(GetImpl(c)))
                {
                    if( priv_isSubContextBound( rdn ) )
                    {
                        fh_stringstream ss;
                        ss << "Context already exists! rdn:" << rdn
                           << " url:" << c->getURL()
                           << endl;
                        Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
                    }

                    try
                    {
                        // tell obby to create a new document.
                        buffer_t::document_info_type* di = 0;
                        std::string title = rdn;
                        std::string encoding = "UTF-8";
                        std::string content = v;

                        
                        cerr << "About to call document_create()" << endl;
                        m_buffer->document_create( title, encoding, content );

                        cerr << "About to sleep waiting for document_create()" << endl;
                        Main::processAllPendingEvents();
                        cerr << "Done with sleeping waiting for document_create()" << endl;
                        
                        return getSubContext( rdn );
                    }
                    catch( exception& e )
                    {
                        fh_stringstream ss;
                        ss << "Error creating new obby buffer. e:" << e.what()
                           << " rdn:" << rdn
                           << " url:" << c->getURL()
                           << endl;
                        Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
                    }
                }

                fh_stringstream ss;
                ss << "SHOULD NEVER HAPPEN!"
                   << " Attempt to create a new obby file in a context that is not an obby one!"
                   << " url:" << c->getURL()
                   << endl;
                Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
                
            }
        
        void
        priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
                m["file"] = SubContextCreator( SL_SubCreate_file,
                                              "	<elementType name=\"file\">\n"
                                              "		<elementType name=\"name\" default=\"new file\">\n"
                                              "			<dataTypeRef name=\"string\"/>\n"
                                              "		</elementType>\n"
                                               "	</elementType>\n");
            }
        
    public:

        obbyInstanceContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_buffer( 0 ),
            m_loginHasFailed( false ),
            m_loggedIn( false ),
            m_color( 255, 200, 0 )
            {
                createStateLessAttributes( true );
            }
        
        
    };
    
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    class FERRISEXP_CTXPLUGIN obbyRootContext
        :
        public networkRootContext< obbyInstanceContext >
    {
        typedef obbyRootContext                           _Self;
        typedef networkRootContext< obbyInstanceContext > _Base;

    protected:

        virtual std::string TryToCheckServerExists( const std::string& rdn )
            {
                userpass_t up = getOBBYUserPass( rdn );
                if( up.first.empty() )
                    return "No username set for server";
                return "";
            }
        
        void
        priv_read()
            {
                LG_OBBY_D << "obbyRootContext.priv_read() url:" << getURL() << endl;
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    tryAugmentLocalhostNames();
                }
            }

    public:

        obbyRootContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }
        virtual ~obbyRootContext()
            {
            }
        virtual std::string priv_getRecommendedEA()
            {
                return "name";
            }
        virtual std::string getRecommendedEA()
            {
                return "name";
            }
        

        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
        obbyRootContext* priv_CreateContext( Context* parent, string rdn )
            {
                obbyRootContext* ret = new obbyRootContext();
                ret->setContext( parent, rdn );
                return ret;
            }
        
        
    };

    
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                static obbyRootContext c;
                fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
                return ret;
            }
            catch( exception& e )
            {
                LG_OBBY_ER << "Brew() e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

};
