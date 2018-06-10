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

#include "libferrisidentica_shared.hh"
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

#define DEBUG LG_IDENTICA_D
//#define DEBUG cerr

using namespace std;

namespace Ferris
{
    namespace Identica
    {

        using namespace XML;
        using namespace std;
        static const string DBNAME = FDB_SECURE;

        const char* CFG_IDENTICA_USERNAME_K = "identica-username";
        const char* CFG_IDENTICA_USERNAME_DEF = "";

        const char* CFG_IDENTICA_FULLNAME_K = "identica-fullname";
        const char* CFG_IDENTICA_FULLNAME_DEF = "";
        
        const char* CFG_IDENTICA_AUTH_TOKEN_K = "identica-auth-token";
        const char* CFG_IDENTICA_AUTH_TOKEN_DEF = "";

        const char* CFG_IDENTICA_UID_K = "identica-uid";
        const char* CFG_IDENTICA_UID_DEF = "";
    
    
        typedef std::list< DOMElement* > entries_t;

        std::string getChildElementText( DOMNode* node, const std::string& name )
        {
            DOMElement* e = getChildElement( node, name );
            return e ? getChildText( e ) : "";
        }

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

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

        QNetworkAccessManager*
        Identica::getQManager()
        {
            return ::Ferris::getQManager();
        }

        Identica::Identica( const std::string& baseURI,
                            const std::string& secdbprefix,
                            const std::string& consumerKeyPrefix )
            : m_baseURI( baseURI )
            , m_qauth( new QOAuth::Interface )
            , m_secDBPrefix( secdbprefix )
            , m_consumerKeyPrefix( consumerKeyPrefix )
        {
            m_uid        = getConfigString( DBNAME, CFG_IDENTICA_UID_K,       CFG_IDENTICA_UID_DEF );
            m_username   = getConfigString( DBNAME, CFG_IDENTICA_USERNAME_K,  CFG_IDENTICA_USERNAME_DEF );
            m_fullname   = getConfigString( DBNAME, CFG_IDENTICA_FULLNAME_K,  CFG_IDENTICA_FULLNAME_DEF );

            if( !m_consumerKeyPrefix.empty() )
            {
                DEBUG << "consumer key file:" << (m_consumerKeyPrefix + "-api-key.txt") << endl;
                DEBUG << "consumer key:" << getStrSubCtx( m_consumerKeyPrefix + "-api-key.txt", "" ).c_str() << endl;
        
                m_qauth->setConsumerKey(
                    getStrSubCtx( m_consumerKeyPrefix + "-api-key.txt", "" ).c_str());
                m_qauth->setConsumerSecret(
                    getStrSubCtx( m_consumerKeyPrefix + "-api-secret.txt", "" ).c_str());
        
            }
            if( !m_secDBPrefix.empty() )
            {
                const string DBNAME = FDB_SECURE;
                std::string k;

                m_token = getConfigString( DBNAME, m_secDBPrefix + "-token", "" );
                m_tokenSecret = getConfigString( DBNAME, m_secDBPrefix + "-token-secret", "" );
            }
    
    
            m_qauth->setRequestTimeout( 10000 );
        }
    
        Identica::~Identica()
        {
            delete m_qauth;
        }


        

        std::string
        Identica::getUserID()
        {
            return m_uid;
        }
    
        std::string
        Identica::getUserName()
        {
            return m_username;
        }

        std::string
        Identica::getFullName()
        {
            return m_fullname;
        }
        
        fh_domdoc
        Identica::toDOM( const QByteArray& ba )
        {
            fh_domdoc dom = Factory::StringToDOM( tostr(ba) );
            DOMElement* e = dom->getDocumentElement();
            return dom;
        }


        bool
        Identica::isAuthenticated() const
        {
            return !m_token.empty() && !m_tokenSecret.empty();
        }

        bool
        Identica::haveAPIKey() const
        {
            std::string key = tostr(m_qauth->consumerKey());
            std::string sec = tostr(m_qauth->consumerSecret());
            return !key.empty() && !sec.empty();
        }



        void
        Identica::setBaseURI( const std::string& earl )
        {
            m_baseURI = earl;
        }
        
        void
        Identica::setStatus( const std::string& s_const )
        {
            std::string s = URLencode(s_const);
            stringmap_t args;
            args["status"] = s;
            DEBUG << "status:" << s << endl;
            QNetworkReply* reply = post( "statuses/update.json", args );
            QByteArray ba = reply->readAll();
            DEBUG << "status update code:" << reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() << endl;
            DEBUG << "status update ba:" << tostr(ba) << endl;
        }
        

    
        std::string
        Identica::requestToken( const std::string& earlTail,
                                stringmap_t args )
        {
            if( !args.count("oauth_callback"))
                args.insert( make_pair( "oauth_callback", "oob" ) );
            QOAuth::ParamMap inp = topm( args );
            QOAuth::ParamMap map = m_qauth->requestToken( (m_baseURI + "oauth/request_token").c_str(),
                                                          QOAuth::GET, QOAuth::HMAC_SHA1, inp );
            DEBUG << " erorr:" << m_qauth->error() << endl;
            if ( m_qauth->error() != QOAuth::NoError )
            {
                DEBUG << "exiting..." << endl;
            }

            m_authToken = tostr(map.value( QOAuth::tokenParameterName()));
            m_authTokenSecret = tostr(map.value( QOAuth::tokenSecretParameterName()));
    
            DEBUG << "token param:"  << m_authToken << endl;
            DEBUG << "token secret:" << m_authTokenSecret << endl;
    
            std::string authURL = m_baseURI + "oauth/authorize?oauth_token=" + m_authToken;
            return authURL;
        }

        stringmap_t
        Identica::accessToken( const std::string& earlTail,
                               const std::string& verifier,
                               stringmap_t args )
        {
            args.insert( make_pair( "oauth_verifier", verifier ));
            QOAuth::ParamMap pm;
            QOAuth::ParamMap inp = topm( args );
            pm = m_qauth->accessToken( (m_baseURI + "oauth/access_token").c_str(),
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
            setConfigString( DBNAME, m_secDBPrefix + "-token", m_token );
            setConfigString( DBNAME, m_secDBPrefix + "-token-secret", m_tokenSecret );
            return frompm( pm );
        }

        QNetworkRequest
        Identica::sign( std::string earlTail, stringmap_t args )
        {
            QNetworkRequest request;
            request.setUrl( QUrl( (m_baseURI + earlTail).c_str() ) );
            DEBUG << "sign(et 1) url:" << tostr(request.url().toString()) << endl;
            request = sign( request, args );
            DEBUG << "sign(et 2) url:" << tostr(request.url().toString()) << endl;
            DEBUG << "sign(et 2) auth:" << tostr(request.rawHeader("Authorization")) << endl;
            return request;
        }


        QNetworkRequest
        Identica::sign( QNetworkRequest request, stringmap_t args )
        {
            DEBUG << "sign(1) url:" << tostr(request.url().toString()) << endl;
            DEBUG << "sign(1) token:" << m_token << " secret:" << m_tokenSecret << endl;
            QOAuth::ParamMap inp = topm( args );
            QByteArray header =
                m_qauth->createParametersString( request.url().toString(),
                                                 QOAuth::POST, 
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

        QNetworkReply*
        Identica::post( std::string earlTail, stringmap_t args, QByteArray content )
        {
            QNetworkRequest request = sign( earlTail, args );
            QNetworkReply*    reply = post( request, content );
            return reply;
        }



        QNetworkReply*
        Identica::post( QNetworkRequest request, QByteArray content )
        {
            QNetworkAccessManager* qm = getQNonCachingManager();

            QEventLoop* loop = new QEventLoop(0);
            qm->connect( qm, SIGNAL(finished(QNetworkReply*)), loop, SLOT(quit()) );
            QNetworkReply* reply = qm->post( request, content );
            DEBUG << "calling loop exec to wait for status update..." << endl;
            loop->exec();
            return reply;
        }


        void
        Identica::setToken( const std::string& s )
        {
            m_token = s;
        }

        void
        Identica::setTokenSecret( const std::string& s )
        {
            m_tokenSecret = s;
        }

        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
    };
    
    namespace Factory
    {
        ::Ferris::Identica::fh_identica getIdentica( const std::string& rbase,
                                                     const std::string& kbase )
        {
            Main::processAllPendingEvents();
            KDE::ensureKDEApplication();

            typedef std::map< std::string, Identica::fh_identica > cache_t;
            static cache_t cache;
            cache_t::iterator iter = cache.find( rbase );
            if( iter == cache.end() )
            {
                std::string REST_BASE  = rbase;
                std::string tokenPrefix = kbase;
                std::string consumerKeyPrefix = "~/.ferris/" + kbase;
                Identica::fh_identica site = new Identica::Identica( REST_BASE, tokenPrefix, consumerKeyPrefix );
                cache.insert( make_pair( rbase, site ) );
                iter = cache.find( rbase );
            }
            return iter->second;
        }
        ::Ferris::Identica::fh_identica getIdentica( const std::string& service )
        {
            std::string earl = "~/.ferris/identica/" + service;
            std::string rbase = getStrSubCtx( earl + "/rest_base", "" );
            std::string kbase = getStrSubCtx( earl + "/token_prefix", "" );
            DEBUG << "rbase:" << rbase << endl;
            DEBUG << "kbase:" << kbase << endl;
            
            return getIdentica( rbase, kbase );
        }
        
    };
    
    
    
    /****************************************/
    /****************************************/
    /****************************************/
    
};
