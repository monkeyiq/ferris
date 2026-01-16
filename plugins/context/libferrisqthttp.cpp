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

    $Id: libferriscurl.cpp,v 1.11 2009/09/21 21:30:18 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
//
//
// Note that RFC 0959 is the FTP one.
//
//
#include <config.h>

#include <Ferris_private.hh>
#include <FerrisContextPlugin.hh>
#include <Trimming.hh>
#include <General.hh>

#include <SmartPtr.h>
#include <Singleton.h>
#include <Factory.h>
#include <Functor.h>

#include <string>
#include <map>
#include <vector>

#include <sys/utsname.h>
#include <time.h>

#include <Ferris/FerrisQt_private.hh>
#include <Ferris/FerrisKDE.hh>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#undef emit

#include <signal.h>

using namespace std;

#define DEBUG LG_QIO_D


namespace Ferris
{

    /****************************************/
    /****************************************/
    /****************************************/

    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf );
    };
    
    class qtHTTPContext;
    class qtHTTPServerContext;
    class qtHTTPRootContext;


    class FERRISEXP_CTXPLUGIN qtHTTPContext
        :
        public QObject,
        public StateLessEAHolder< qtHTTPContext, FakeInternalContext >
    {
        Q_OBJECT;
        typedef StateLessEAHolder< qtHTTPContext, FakeInternalContext > _Base;

        fh_StreamToQIODevice m_qio;
        time_t     m_mtime;
        streamsize m_size;
        bool m_haveMetadata;
        QNetworkResponseWaiter m_waiter;
                                       
    public slots:

        void On_QNetworkReply_MetaDataChanged();
        
    public:

        qtHTTPContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_mtime( 0 ),
            m_size( 0 ),
            m_haveMetadata( false )
            {
                createStateLessAttributes();
                addAttribute( EAN_IS_REMOTE, "1", XSD_BASIC_BOOL );
            }
        // void constructObject( const QUrlInfo & i )
        // {
        //     m_info = i;
        // }
        virtual ~qtHTTPContext()
        {
        }
        virtual fh_context priv_getSubContext( const std::string& rdn )
        {
            DEBUG << "priv_getSubContext(A) rdn:" << rdn << endl;
            
            qtHTTPContext* child = 0;
            child = priv_ensureSubContext( rdn, child );
            return child;
        }
            
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m );
        void priv_getIOStream_IOStreamClosed( fh_istream& ss, std::streamsize tellp );
        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m );

        static fh_istream SL_getMTimeRawStream( qtHTTPContext* c, const std::string& rdn, EA_Atom* atom )
        {
            c->ensureMetadata();
            fh_stringstream ss;
            ss << c->m_mtime;
            return ss;
        }
        static fh_istream SL_getSizeStream( qtHTTPContext* c, const std::string& rdn, EA_Atom* atom )
        {
            c->ensureMetadata();
            fh_stringstream ss;
            ss << c->m_size;
            return ss;
        }
        
        void createStateLessAttributes( bool force = false )
        {
            static Util::SingleShot virgin;
            if( virgin() )
            {
                DEBUG << "createStateLessAttributes()" << endl;

                tryAddStateLessAttribute( "mtime", SL_getMTimeRawStream, FXD_UNIXEPOCH_T );
                tryAddStateLessAttribute( "size",  SL_getSizeStream,     FXD_FILESIZE );
                
                _Base::createStateLessAttributes( true );
                supplementStateLessAttributes( true );
            }
        }

        void priv_read();
        virtual qtHTTPServerContext* getServer();
        string getHTTPPath();
        QNetworkRequest createRequest()
        {
            DEBUG << "createRequest() url:" << getHTTPPath() << endl;
            QNetworkRequest request( QUrl(getHTTPPath().c_str()) );
//        request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");
            return request;
        }
        void ensureMetadata();
    };
        
        
    

    class FERRISEXP_CTXPLUGIN qtHTTPServerContext
        :
        public qtHTTPContext
    {
        Q_OBJECT;
        typedef qtHTTPContext _Base;
        
    public slots:

    public:

        qtHTTPServerContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
        {
        }
        
        virtual ~qtHTTPServerContext()
        {
        }
        
        virtual qtHTTPServerContext* getServer()
        {
            return this;
        }
    };
    

    /*
     * Allow network hosts to be discovered at run time.
     */
    class FERRISEXP_CTXPLUGIN qtHTTPRootContext
        :
        public networkRootContext<qtHTTPServerContext>
    {
        typedef qtHTTPRootContext                        _Self;
        typedef networkRootContext<qtHTTPServerContext>  _Base;
    public:
        
        qtHTTPRootContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn, true )
            {
            }
        
        virtual ~qtHTTPRootContext()
            {
            }
    };

    /************************************************************/
    /************************************************************/
    /************************************************************/

    void
    qtHTTPContext::On_QNetworkReply_MetaDataChanged()
    {
        QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());
        stringset_t strset;
        
        QList<QByteArray> ql = reply->rawHeaderList();
        for( QList<QByteArray>::iterator qiter = ql.begin(); qiter != ql.end(); ++qiter )
        {
            QByteArray ba = *qiter;
            fh_stringstream ss;
            ss << ba;
            string k = tostr(ba);
            string v = tostr( reply->rawHeader( k.c_str() ));
            DEBUG << "got header:" << k << endl;

            try
            {
                if( k == "Last-Modified" )
                {
                    PrefixTrimmer trimmer;
                    trimmer.push_back( " " );
                    trimmer.push_back( "\t" );
                    v = trimmer( v );
                    
                    struct tm t = Time::ParseTimeString( v, "" );
                    m_mtime = mktime( &t );
                    m_haveMetadata = true;
                    DEBUG << "Got header for mtime! v:" << v << endl;
                    DEBUG << "Got header for mtime! remote mtime is:" << m_mtime << endl;
                    strset.insert( "mtime" );
                }
                else if( k == "Content-Length" )
                {
                    DEBUG << "SETTING theSize2 to:" << v << endl;
                    m_size = toType<streamsize>( v );
                    m_haveMetadata = true;
                    DEBUG << "Got header for sz! v:" << v << endl;
                    DEBUG << "Got header for sz! remote is:" << m_size << endl;
                    strset.insert( "size" );
                }
                else if( k == "Location" )
                {
                    DEBUG << "Location redirect to:" << v << endl;
                }
                
                addAttribute( k, v );
                addAttribute( tolowerstr()(k), v );
            }
            catch( exception& e )
            {
                LG_CURL_W << "Problem parsing header k:" << k << " v:" << v << endl;
            }
        }

        getContextEvent_Headers_Received_Sig().emit( this, strset );
        m_waiter.unblock( reply );
    }
    
    fh_istream
    qtHTTPContext::priv_getIStream( ferris_ios::openmode m )
    {
        DEBUG << "getting I stream...url:" << getURL() << endl;

        QNetworkAccessManager* qm = getQManager();
        QNetworkRequest request = createRequest();
        DEBUG << "getting I stream2...url:" << getURL() << endl;
        QNetworkReply *reply = qm->get( request );
        connect( (QObject*)reply,
                 SIGNAL( metaDataChanged() ),
                 SLOT( On_QNetworkReply_MetaDataChanged() ) );
        fh_istream ret = Factory::createIStreamFromQIODevice( reply );
        return ret;
    }
    
    fh_iostream
    qtHTTPContext::priv_getIOStream( ferris_ios::openmode m )
    {
        QNetworkAccessManager* qm = getQManager();
        QNetworkRequest request = createRequest();

        std::streamsize totalPostSize = 0;
        m_qio = Factory::createStreamToQIODevice( totalPostSize );
        QNetworkReply* reply = m_qio->post( qm, request, "" );
        fh_iostream ret = m_qio->getStream();
        return ret;
    }
        
    void
    qtHTTPContext::priv_read()
    {
        stringstream ss;
        ss << "Can not read HTTP directories. url:" << getURL() << endl;
        Throw_FerrisNotReadableAsContext( tostr(ss), this );
    }
    
    qtHTTPServerContext*
    qtHTTPContext::getServer()
    {
        qtHTTPServerContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p;
    }
    
    string
    qtHTTPContext::getHTTPPath()
    {
        string ret = getURL();
        if( starts_with( ret, "http:///" ))
        {
            ret = (string)"http://" + ret.substr(strlen("http:///"));
        }
        if( contains( ret, "paste.kde.org" )
            && contains( ret, "/api/raw/" ))
        {
            ret += "/";
        }
        return ret;
    }
    
    void
    qtHTTPContext::ensureMetadata()
    {
        if( m_haveMetadata )
            return;

        DEBUG << "ensureMetadata() performing head" << endl;
        QNetworkAccessManager* qm = getQManager();
        QNetworkRequest request = createRequest();
        QNetworkReply *reply = qm->head(request);
        connect( (QObject*)reply,
                 SIGNAL( metaDataChanged() ),
                 SLOT( On_QNetworkReply_MetaDataChanged() ) );
        DEBUG << "ensureMetadata() waiting for head" << endl;
        m_waiter.block( reply );
        DEBUG << "ensureMetadata() head complete" << endl;
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


    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
        {
            try
            {
                Main::processAllPendingEvents();
                KDE::ensureKDEApplication();
                
                static qtHTTPRootContext* c = 0;

                if( !c )
                {
                    DEBUG << "Making FakeInternalContext(1) " << endl;
                    c = new qtHTTPRootContext(0, "/");
            
                    // Bump ref count.
                    static fh_context keeper = c;
                    static fh_context keeper2 = keeper;
                    DEBUG << "Making FakeInternalContext(3) " << endl;
                }

                DEBUG << "Making FakeInternalContext(4) brewing return" << endl;

                fh_context ret = c;
                DEBUG << "curl.brew() ret:" << ret->getURL() << endl;
                return ret;
            }
            catch( FerrisSqlServerNameNotFound& e )
            {
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
            catch( exception& e )
            {
                DEBUG << "Brew() e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }
};

#include "libferrisqthttp_moc_impl.cpp"
