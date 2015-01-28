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

#include "libferrisvimeo_shared.hh"
#include <Ferris/Configuration_private.hh>

#include <QNetworkReply>
#include <QNetworkAccessManager>

#include <Ferris/FerrisQt_private.hh>
#include <Ferris/FerrisOpenSSL.hh>
#include <Ferris/FerrisKDE.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/Iterator.hh>
#include <Ferris/FerrisOpenSSL.hh>

#include <sys/time.h>

#include <QtOAuth>
#include <qjson/parser.h>

#define DEBUG LG_VIMEO_D
//#define DEBUG cerr


namespace Ferris
{
    namespace Vimeo
    {

        using namespace XML;
        using namespace std;
        static const string DBNAME = FDB_SECURE;

        const char* CFG_VIMEO_USERNAME_K = "vimeo-username";
        const char* CFG_VIMEO_USERNAME_DEF = "";

        const char* CFG_VIMEO_FULLNAME_K = "vimeo-fullname";
        const char* CFG_VIMEO_FULLNAME_DEF = "";
        
        const char* CFG_VIMEO_AUTH_TOKEN_K = "vimeo-auth-token";
        const char* CFG_VIMEO_AUTH_TOKEN_DEF = "";

        const char* CFG_VIMEO_UID_K = "vimeo-uid";
        const char* CFG_VIMEO_UID_DEF = "";
    
        const char* REST_URL = "http://vimeo.com/api/rest/v2";
        std::string AUTH_URL = "https://vimeo.com/oauth/";
        
        typedef std::list< DOMElement* > entries_t;

        std::string getChildElementText( DOMNode* node, const std::string& name )
        {
            DOMElement* e = getChildElement( node, name );
            return e ? getChildText( e ) : "";
        }

        static QOAuth::ParamMap topm( stringmap_t sm )
        {
            QOAuth::ParamMap ret;
            for( stringmap_t::iterator iter = sm.begin(); iter != sm.end(); ++iter )
            {
                ret.insert( iter->first.c_str(), iter->second.c_str() );
            }
            return ret;
        }

        static stringmap_t frompm( QOAuth::ParamMap pm )
        {
            stringmap_t ret;
            typedef QOAuth::ParamMap::iterator iter_t;
            for( QOAuth::ParamMap::iterator iter = pm.begin(); iter != pm.end(); ++iter )
            {
                ret.insert( make_pair( tostr(iter.key()), tostr(iter.value())) );
            }
            return ret;
        }
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        QNetworkAccessManager*
        Vimeo::getQManager()
        {
            return ::Ferris::getQManager();
        }

        Vimeo::Vimeo()
            : m_qauth( new QOAuth::Interface )
        {
            m_uid        = getEDBString( DBNAME, CFG_VIMEO_UID_K, CFG_VIMEO_UID_DEF );
            m_username   = getEDBString( DBNAME, CFG_VIMEO_USERNAME_K,    CFG_VIMEO_USERNAME_DEF );
            m_fullname   = getEDBString( DBNAME, CFG_VIMEO_FULLNAME_K,    CFG_VIMEO_FULLNAME_DEF );
            m_authToken  = getEDBString( DBNAME, CFG_VIMEO_AUTH_TOKEN_K,  CFG_VIMEO_AUTH_TOKEN_DEF );
            m_apiKey     = getStrSubCtx( "~/.ferris/vimeo-api-key.txt", "" );
            m_apiSecret  = getStrSubCtx( "~/.ferris/vimeo-api-secret.txt", "" );

            m_token       = getEDBString( DBNAME, "vimeo-token", "" );
            m_tokenSecret = getEDBString( DBNAME, "vimeo-token-secret", "" );
            m_qauth->setConsumerKey(    getStrSubCtx( "~/.ferris/vimeo-api-key.txt", "" ).c_str());
            m_qauth->setConsumerSecret( getStrSubCtx( "~/.ferris/vimeo-api-secret.txt", "" ).c_str());
        }
    
        Vimeo::~Vimeo()
        {
        }

        string
        Vimeo::getToken() const
        {
            return m_token;
        }
        
        void
        Vimeo::handleFinished()
        {
            QNetworkReply* r = dynamic_cast<QNetworkReply*>(sender());
            m_waiter.unblock(r);
            DEBUG << "handleFinished() r:" << r << endl;
        }

        QNetworkRequest
        Vimeo::createRequest( QUrl u )
        {
            QNetworkRequest req;
            req.setUrl(u);
            return req;
        }
    
        QNetworkRequest
        Vimeo::createRequest( const std::string& u )
        {
            if( u.empty() )
                return createRequest( REST_URL );
            return createRequest( QUrl( u.c_str() ) );
        }
    

        QByteArray&
        Vimeo::fetchURL( const std::string earl, QByteArray& ret )
        {
            QNetworkRequest req( QUrl( earl.c_str() ));
            QNetworkReply* reply = getQManager()->get( req );
            connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
            m_waiter.block(reply);
            ret = reply->readAll();
            return ret;
        }

        QNetworkRequest
        Vimeo::sign( QUrl u, stringmap_t args )
        {
            QNetworkRequest request;
            request.setUrl( u );
            request = sign( request, args );
            return request;
        }
    
        
        QNetworkRequest
        Vimeo::sign( QNetworkRequest request, stringmap_t args, QOAuth::HttpMethod httpMethod )
        {
            DEBUG << "top1" << endl;
            DEBUG << "sign(1) url:" << tostr(request.url().toString()) << endl;
            DEBUG << "sign(1) token:" << m_token << " secret:" << m_tokenSecret << endl;


            QOAuth::ParamMap inp = topm( args );
            
            QByteArray header =
                m_qauth->createParametersString( request.url().toString(),
                                                 httpMethod,
                                                 m_token.c_str(), m_tokenSecret.c_str(),
                                                 QOAuth::HMAC_SHA1,
                                                 inp,
                                                 QOAuth::ParseForHeaderArguments );


            QUrl url = request.url();
            QString x = url.toString();
            DEBUG << "sign.x:" << tostr(x) << endl;
            x.append( m_qauth->inlineParameters( inp, QOAuth::ParseForInlineQuery ) );
            url.setEncodedUrl( x.toUtf8() );
            request.setUrl( url );
            
            request.setRawHeader( "Authorization", header );
            
            DEBUG << "sign(2) url:" << tostr(request.url().toString()) << endl;
            DEBUG << "sign(2) auth:" << tostr(header) << endl;
            return request;
        }


        QNetworkRequest
        Vimeo::signPUT( QNetworkRequest request, stringmap_t args )
        {
            QOAuth::HttpMethod httpMethod = QOAuth::POST;
            DEBUG << "top1" << endl;
            DEBUG << "sign(1) url:" << tostr(request.url().toString()) << endl;
            DEBUG << "sign(1) token:" << m_token << " secret:" << m_tokenSecret << endl;


            QOAuth::ParamMap inp = topm( args );
            QByteArray header =
                m_qauth->createParametersString( request.url().toString(),
                                                 httpMethod,
                                                 m_token.c_str(), m_tokenSecret.c_str(),
                                                 QOAuth::HMAC_SHA1,
                                                 inp,
                                                 QOAuth::ParseForRequestContent );

            
            QUrl url = request.url();
            QString x = url.toString();
            DEBUG << "sign.x:" << tostr(x) << endl;
            x.append( m_qauth->inlineParameters( inp, QOAuth::ParseForRequestContent ) );
            x.append( "&" );
            x.append( header );
            url.setEncodedUrl( x.toUtf8() );
            request.setUrl( url );
            
            DEBUG << "signGet(2) url:" << tostr(request.url().toString()) << endl;
            DEBUG << "signGet(2) auth:" << tostr(header) << endl;
            return request;
        }
        

        
        
        QNetworkReply*
        Vimeo::get( const std::string& meth, QNetworkRequest req, stringmap_t args )
        {
            args["method"] = meth;
            req = sign( req, args );
            QNetworkReply* reply = getQManager()->get( req );
            connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
            m_waiter.block(reply);
            DEBUG << "REST error code:" << reply->error() << endl;
            return reply;
        }
        QNetworkReply*
        Vimeo::callMeth( const std::string& meth, QNetworkRequest req, stringmap_t args )
        {
            args["method"] = meth;
            req = sign( req, args );
            req.setRawHeader( "Accept", "*/*" );
            req.setRawHeader( "Accept-Encoding", "none" );
            req.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
            QNetworkReply* reply = post( meth, req, false );
            waitfor( reply );
            return reply;
        }
        QNetworkReply*
        Vimeo::post( const std::string& meth, QNetworkRequest req, bool addAuth )
        {
            stringmap_t args;
            args["method"] = meth;
            if( addAuth )
                req = sign( req, args );
            
            QNetworkReply* reply = getQManager()->post( req, "" );
            connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
            m_waiter.block(reply);
            DEBUG << "REST error code:" << reply->error() << endl;
            return reply;
        }

        QNetworkReply*
        Vimeo::post( QNetworkRequest req, const std::string& body )
        {
            QNetworkReply* reply = getQManager()->post( req, body.c_str() );
            connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
            m_waiter.block(reply);
            DEBUG << "REST error code:" << reply->error() << endl;
            return reply;
        }

        QNetworkReply*
        Vimeo::waitfor( QNetworkReply* reply )
        {
            connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
            m_waiter.block(reply);
            DEBUG << "WAITFOR error code:" << reply->error() << endl;
            return reply;
        }
        
        
        

        std::string
        Vimeo::getUserID()
        {
            return m_uid;
        }
    
        std::string
        Vimeo::getUserName()
        {
            return m_username;
        }

        std::string
        Vimeo::getFullName()
        {
            return m_fullname;
        }
        
        bool
        Vimeo::haveAuthToken()
        {
            return !m_authToken.empty();
        }
    
        std::string
        Vimeo::getAuthToken()
        {
            return m_authToken;
        }

        bool
        Vimeo::isAuthenticated() const
        {
            return !m_token.empty() && !m_tokenSecret.empty();
        }
        
        bool
        Vimeo::haveAPIKey()
        {
            return !m_apiKey.empty();
        }

        std::string
        Vimeo::APIKey()
        {
            return m_apiKey;
        }

        std::string
        Vimeo::APISecret()
        {
            return m_apiSecret;
        }


        fh_domdoc
        Vimeo::toDOM( const QByteArray& ba )
        {
            fh_domdoc dom = Factory::StringToDOM( tostr(ba) );
            DOMElement* e = dom->getDocumentElement();
            return dom;
        }


        std::string
        Vimeo::requestToken( stringmap_t args )
        {
            if( !args.count("oauth_callback"))
                args.insert( make_pair( "oauth_callback", "oob" ) );
            QOAuth::ParamMap inp = topm( args );
            QOAuth::ParamMap map = m_qauth->requestToken( (AUTH_URL + "request_token").c_str(),
                                                          QOAuth::GET, QOAuth::HMAC_SHA1, inp );
            DEBUG << " erorr:" << m_qauth->error() << endl;
            if ( m_qauth->error() != QOAuth::NoError )
            {
                DEBUG << "exiting..." << endl;
            }

            m_authToken       = tostr(map.value( QOAuth::tokenParameterName()));
            m_authTokenSecret = tostr(map.value( QOAuth::tokenSecretParameterName()));
    
            DEBUG << "token param:"  << m_authToken << endl;
            DEBUG << "token secret:" << m_authTokenSecret << endl;
    
            string perm = "delete";
            std::string authURL = AUTH_URL + "authorize?oauth_token=" + m_authToken + "&permission=" + perm;
            return authURL;
        }

        stringmap_t
        Vimeo::accessToken( const std::string& verifier_const,
                            stringmap_t args )
        {
            std::string verifier = verifier_const;
            if( starts_with( verifier, "http" ))
            {
                static const boost::regex rx = toregex( ".*oauth_verifier=" );
                verifier = replaceg( verifier, rx, "" );
                verifier = replaceg( verifier, "&.*", "" );
            }
            
            cerr << "m_authToken:" << m_authToken << endl;
            cerr << "m_authTokenSecret:" << m_authTokenSecret << endl;
            cerr << "verifier:" << verifier << endl;
            args.insert( make_pair( "oauth_verifier", verifier ));
//            args.insert( make_pair( "x_auth_permission", "write" ));
            QOAuth::ParamMap pm;
            QOAuth::ParamMap inp = topm( args );
            pm = m_qauth->accessToken( (AUTH_URL + "access_token").c_str(),
                                       QOAuth::POST,
                                       m_authToken.c_str(), m_authTokenSecret.c_str(),
                                       QOAuth::HMAC_SHA1,
                                       inp );
            if ( m_qauth->error() != QOAuth::NoError )
            {
                DEBUG << "got an error:" << m_qauth->error() << endl;
            }
    
            m_token = tostr( pm.value( QOAuth::tokenParameterName() ));
            m_tokenSecret = tostr( pm.value( QOAuth::tokenSecretParameterName() ));
            DEBUG << " accessToken:" << m_token << endl;
            DEBUG << "accessSecret:" << m_tokenSecret << endl;

            const string DBNAME = FDB_SECURE;
            setEDBString( DBNAME, "vimeo-token",        m_token );
            setEDBString( DBNAME, "vimeo-token-secret", m_tokenSecret );
            return frompm( pm );
        }
        
        
                

        
        
        
        
        std::string
        Vimeo::testLogin()
        {
            QUrl u( REST_URL );
//            u.addQueryItem("frob",  frob.c_str());
            QNetworkRequest req = createRequest( u );
            QNetworkReply* reply = get( "vimeo.test.login", req );
            QByteArray ba = reply->readAll();
            return tostr(ba);
        }

        fh_Upload
        Vimeo::createUpload()
        {
            return new Upload( this );
        }

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        Upload::Upload( fh_vimeo vim )
            :
            m_vim( vim )
        {
        }

        
        void
        Upload::streamingUploadComplete()
        {
            DEBUG << "streamingUploadComplete(top)" << endl;

            QByteArray ba;
            
            m_streamToQIO->writingComplete();
            DEBUG << "Upload::streamingUploadComplete() reading data..." << endl;
            ba = m_streamToQIO->readResponse();
            DEBUG << "Upload::streamingUploadComplete() m_streamToQIO:" << GetImpl(m_streamToQIO)
                  << " reply:" << m_reply->error()  << endl;
            DEBUG << "streamingUploadComplete() got reply:" << tostr(ba) << endl;
            DEBUG << "streamingUploadComplete() -------------------------------------" << endl;
            
            {
                DEBUG << "Calling verifyChunks." << endl;
                QUrl u( REST_URL );
                QNetworkRequest req = m_vim->createRequest( u );
                stringmap_t args;
                args["ticket_id"] = m_uploadTicket;
                args["method"] = "vimeo.videos.upload.verifyChunks";

                QNetworkReply* reply = m_vim->callMeth( "vimeo.videos.upload.verifyChunks", req, args );
                ba = reply->readAll();
                DEBUG << "VERIFY got reply:" << tostr(ba) << endl;
            }
                
            
            // post manifests
            {
                DEBUG << "Calling upload.complete" << endl;
                QUrl u( REST_URL );
                QNetworkRequest req = m_vim->createRequest( u );
                stringmap_t args;
                args["ticket_id"] = m_uploadTicket;
                args["filename"] = m_uploadFilename;

                QNetworkReply* reply = m_vim->callMeth( "vimeo.videos.upload.complete", req, args );
                ba = reply->readAll();
                DEBUG << "COMPLETE got reply:" << tostr(ba) << endl;
                DEBUG << "confirm message got reply.error:" << reply->error() << endl;
                DEBUG << "confirm message got reply:" << tostr(ba) << endl;

                // <ticket id="abcdef124567890" video_id="1234567" />
                fh_domdoc dom = m_vim->toDOM( ba );
                if( DOMElement* e = firstChild( dom->getDocumentElement(), "ticket" ))
                {
                    string id   = getAttribute( e, "id" );
                    string vid  = getAttribute( e, "video_id" );
                    DEBUG << "id:" << id << " vid:" << vid << endl;
                    m_id = vid;
                    m_url = (string)"http://www.vimeo.com/" + vid;
                    if( !m_id.empty() )
                        cout << "uploaded URL: " << m_url << endl;
                }

                if( m_id.empty() )
                {
                    stringstream ss;
                    ss << "Upload failed! no video ID returned from server";
                    cerr << ss.str() << endl;
                    Throw_getIOStreamCloseUpdateFailed( tostr(ss), 0 );
                }
            }
        }
        
        fh_iostream
        Upload::createStreamingUpload( const std::string& ContentType )
        {
            DEBUG << "createStreamingUpload() ct:" << ContentType << endl;

            // get an upload ticket
            string uploadTicket = "";
            string uploadEndpoint = "";
            {
                DEBUG << "getting a ticket... REST_URL:" << REST_URL << endl;
                QUrl u( REST_URL );
                QNetworkRequest req = m_vim->createRequest( u );
                QNetworkReply* reply = m_vim->callMeth( "vimeo.videos.upload.getTicket", req );
                QByteArray ba = reply->readAll();
                DEBUG << "getTicket result:" << tostr(ba) << endl;
                fh_domdoc dom = m_vim->toDOM( ba );
                if( DOMElement* e = firstChild( dom->getDocumentElement(), "ticket" ))
                {
                    uploadTicket   = getAttribute( e, "id" );
                    uploadEndpoint = getAttribute( e, "endpoint" );
                }
                else
                {
                    stringstream ss;
                    ss << "Upload failed! Can not get an upload ticket:";
                    ss << tostr(ba) << endl;
                    Throw_getIOStreamCloseUpdateFailed( tostr(ss), 0 );
                }
            }

            DEBUG << "uploadTicket:"   << uploadTicket   << endl;
            DEBUG << "uploadEndpoint:" << uploadEndpoint << endl;
            DEBUG << "m_uploadSize:"   << m_uploadSize   << endl;

            // post the video, streaming
            QUrl u( uploadEndpoint.c_str() );

            QNetworkRequest req;
            u.addQueryItem("chunk_id", "0" );
            req.setUrl(u);
            
            req.setHeader(QNetworkRequest::ContentLengthHeader, tostr(m_uploadSize).c_str() );
            req.setRawHeader( "Accept", "*/*" );
            req.setRawHeader( "Connection", "" );            
            req.setRawHeader( "Accept-Encoding", "" );
            req.setRawHeader( "Accept-Language", "" );
            req.setRawHeader( "User-Agent", "" );

            DEBUG << "PUT()ing main request..." << endl;
            stringmap_t args;
            req = m_vim->signPUT( req, args );
            m_streamToQIO = Factory::createStreamToQIODevice();
            QNetworkReply* reply = m_streamToQIO->put( ::Ferris::getQNonCachingManager(), req );
            
            m_streamToQIO_reply = reply;
            m_reply = reply;
            m_uploadTicket = uploadTicket;

            DEBUG << "preparing iostream for user..." << endl;
            {
                fh_iostream ret = m_streamToQIO->getStream();
                DEBUG << "createStreamingUpload(end)" << endl;
                return ret;
            }
        }
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
    };
    
    namespace Factory
    {
        ::Ferris::Vimeo::fh_vimeo getVimeo()
        {
            Main::processAllPendingEvents();
            KDE::ensureKDEApplication();
            
            static Vimeo::fh_vimeo cache = 0;
            if( !cache )
                cache = new Vimeo::Vimeo();
            return cache;
        }
    };
    
    
    
    /****************************************/
    /****************************************/
    /****************************************/
    
};
