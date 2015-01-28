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

#include "libferriszoneminder_shared.hh"
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

#define DEBUG LG_ZONEMINDER_D
//#define DEBUG cerr

using namespace std;

namespace Ferris
{
    namespace Zoneminder
    {
        static const string DBNAME = FDB_SECURE;

        FERRISEXP_EXPORT userpass_t getUserPass(
            const std::string& server )
        {
            string user;
            string pass;

            string Key = server;
        
            {
                fh_stringstream ss;
                ss << "zoneminder--" << Key << "-username";
                user = getEDBString( DBNAME, tostr(ss), "" );
            }
        
            {
                fh_stringstream ss;
                ss << "zoneminder--" << Key << "-password";
                pass = getEDBString( DBNAME, tostr(ss), "" );
            }

            return make_pair( user, pass );
        }
    
        FERRISEXP_EXPORT void setUserPass(
            const std::string& server,
            const std::string& user, const std::string& pass )
        {
            string Key = server;

            {
                fh_stringstream ss;
                ss << "zoneminder--" << Key << "-username";
                setEDBString( DBNAME, tostr(ss), user );
            }

            {
                fh_stringstream ss;
                ss << "zoneminder--" << Key << "-password";
                setEDBString( DBNAME, tostr(ss), pass );
            }        
        }

        
        ////////////////////
        
        using namespace XML;
        using namespace std;

        const char* CFG_ZONEMINDER_USERNAME_K = "zoneminder-username";
        const char* CFG_ZONEMINDER_USERNAME_DEF = "";

        const char* CFG_ZONEMINDER_FULLNAME_K = "zoneminder-fullname";
        const char* CFG_ZONEMINDER_FULLNAME_DEF = "";
        
        const char* CFG_ZONEMINDER_AUTH_TOKEN_K = "zoneminder-auth-token";
        const char* CFG_ZONEMINDER_AUTH_TOKEN_DEF = "";

        const char* CFG_ZONEMINDER_UID_K = "zoneminder-uid";
        const char* CFG_ZONEMINDER_UID_DEF = "";
    
    
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
        Zoneminder::getQManager()
        {
            return ::Ferris::getQManager();
        }

        Zoneminder::Zoneminder( const std::string& server )
            : m_serverURI( server )
        {
        }
    
        Zoneminder::~Zoneminder()
        {
        }


        bool
        Zoneminder::ensureAuthenticated()
        {
            if( isAuthenticated() )
                return true;

            userpass_t up = ::Ferris::Zoneminder::getUserPass( m_serverURI );
                
            stringstream earlss;
            earlss << "http://" << m_serverURI << "/zm/";
            earlss << "/index.php";
            earlss << "?action=login";
            earlss << "&username=" + up.first;
            earlss << "&password=" + up.second + "&foo=bar";

            QNetworkRequest request;
            if( up.first.empty() )
                return false;
            request.setUrl(QUrl( earlss.str().c_str() ));
            DEBUG << "authenticating with server using url: " << earlss.str() << endl;
            QNetworkReply *reply = post( request, QByteArray() );
            QByteArray ba = reply->readAll();
            DEBUG << "ba:" << tostr(ba) << endl;
            m_authenticated = true;
            return m_authenticated;
        }
        
        std::string
        Zoneminder::getBaseURI() const
        {
            return "http://" + m_serverURI + "/zm/";
        }
        
        
        fh_domdoc
        Zoneminder::toDOM( const QByteArray& ba )
        {
            fh_domdoc dom = Factory::StringToDOM( tostr(ba) );
            DOMElement* e = dom->getDocumentElement();
            return dom;
        }

        fh_domdoc
        Zoneminder::toDOM( QNetworkReply* r )
        {
            QByteArray ba = r->readAll();
            DEBUG << "todom, readAll() gave:" << tostr(ba) << endl;
            return toDOM( ba );
        }
        

        bool
        Zoneminder::isAuthenticated() const
        {
            return m_authenticated;
        }


        QNetworkRequest
        Zoneminder::sign( std::string earlTail, stringmap_t args )
        {
            QNetworkRequest request;
            request.setUrl( QUrl( ("http://" + m_serverURI + earlTail).c_str() ) );
            DEBUG << "sign(et 1) url:" << tostr(request.url().toString()) << endl;
            request = sign( request, args );
            DEBUG << "sign(et 2) url:" << tostr(request.url().toString()) << endl;
            DEBUG << "sign(et 2) auth:" << tostr(request.rawHeader("Authorization")) << endl;
            return request;
        }


        QNetworkRequest
        Zoneminder::sign( QNetworkRequest request, stringmap_t args )
        {
            DEBUG << "sign(1) url:" << tostr(request.url().toString()) << endl;
            QOAuth::ParamMap inp = topm( args );

            QUrl url = request.url();
            QString x = url.toString();
            DEBUG << "sign.x:" << tostr(x) << endl;
            DEBUG << "sign.args.sz:" << args.size() << endl;
            url.setEncodedUrl( x.toUtf8() );
            url = addQueryItems( url, args );
//            x.append( m_qauth->inlineParameters( inp, QOAuth::ParseForInlineQuery ) );
            request.setUrl( url );
//            request.setRawHeader( "Authorization", header );
            DEBUG << "sign(2) url:" << tostr(request.url().toString()) << endl;
            return request;
        }
        
        QNetworkReply*
        Zoneminder::post( std::string earlTail, stringmap_t args, QByteArray content )
        {
            QNetworkRequest request = sign( earlTail, args );
            QUrl url = request.url();
            QString x = url.toString();
            DEBUG << "post.x:" << tostr(x) << endl;
            QNetworkReply*    reply = post( request, content );
            return reply;
        }


        QNetworkReply*
        Zoneminder::post( stringmap_t args, QByteArray content )
        {
            std::string earlTail = "/zm/index.php";
            QNetworkRequest request = sign( earlTail, args );
            QUrl url = request.url();
            QString x = url.toString();
            DEBUG << "post.x:" << tostr(x) << endl;
            QNetworkReply*    reply = post( request, content );
            return reply;
        }



        QNetworkReply*
        Zoneminder::post( QNetworkRequest request, QByteArray content )
        {
            QNetworkReply* reply = getQManager()->post( request, content );
            wait( reply );
            return reply;
        }

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
    };
    
    namespace Factory
    {
        ::Ferris::Zoneminder::fh_zoneminder getZoneminder( const std::string& server )
        {
            ::Ferris::Zoneminder::fh_zoneminder ret( new ::Ferris::Zoneminder::Zoneminder( server ) );
            return ret;
        }
        
    };
    
    
    
    /****************************************/
    /****************************************/
    /****************************************/
    
};
