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

#include "libferrisferrisrest_shared.hh"
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

#include <qjson/parser.h>

#define DEBUG LG_FERRISREST_D
//#define DEBUG cerr

using namespace std;

namespace Ferris
{
    namespace Ferrisrest
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
                ss << "ferrisrest--" << Key << "-username";
                user = getConfigString( DBNAME, tostr(ss), "" );
            }
        
            {
                fh_stringstream ss;
                ss << "ferrisrest--" << Key << "-password";
                pass = getConfigString( DBNAME, tostr(ss), "" );
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
                ss << "ferrisrest--" << Key << "-username";
                setConfigString( DBNAME, tostr(ss), user );
            }

            {
                fh_stringstream ss;
                ss << "ferrisrest--" << Key << "-password";
                setConfigString( DBNAME, tostr(ss), pass );
            }        
        }

        std::string getBaseURI( const std::string& server )
        {
            string Key = server;
            fh_stringstream ss;
            ss << "ferrisrest--" << Key << "-baseurl";
            return getConfigString( DBNAME, tostr(ss), "" );
        }
        
        void setBaseURI( const std::string& server, const std::string& baseurl )
        {
            string Key = server;
            fh_stringstream ss;
            ss << "ferrisrest--" << Key << "-baseurl";
            setConfigString( DBNAME, tostr(ss), baseurl );
        }
        
        
        
        ////////////////////
        
        using namespace XML;
        using namespace std;

        const char* CFG_FERRISREST_USERNAME_K = "ferrisrest-username";
        const char* CFG_FERRISREST_USERNAME_DEF = "";

        const char* CFG_FERRISREST_FULLNAME_K = "ferrisrest-fullname";
        const char* CFG_FERRISREST_FULLNAME_DEF = "";
        
        const char* CFG_FERRISREST_AUTH_TOKEN_K = "ferrisrest-auth-token";
        const char* CFG_FERRISREST_AUTH_TOKEN_DEF = "";

        const char* CFG_FERRISREST_UID_K = "ferrisrest-uid";
        const char* CFG_FERRISREST_UID_DEF = "";
    
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        QNetworkAccessManager*
        Ferrisrest::getQManager()
        {
            return ::Ferris::getQManager();
        }

        Ferrisrest::Ferrisrest( const std::string& server )
            : m_serverURI( server )
        {
        }
    
        Ferrisrest::~Ferrisrest()
        {
        }


        bool
        Ferrisrest::ensureAuthenticated()
        {
            if( isAuthenticated() )
                return true;
            m_authenticated = true;
            return m_authenticated;
        }
        
        std::string
        Ferrisrest::getBaseURI() const
        {
            return "http://" + m_serverURI + ::Ferris::Ferrisrest::getBaseURI( m_serverURI );
        }
        
        
        fh_domdoc
        Ferrisrest::toDOM( const QByteArray& ba )
        {
            fh_stringstream ss;
            ss << tostr(ba);
            ss = trimXMLDeclaration( ss, true );
            fh_domdoc dom = Factory::StringToDOM( tostr(ss) );
            DOMElement* e = dom->getDocumentElement();
            return dom;
        }

        fh_domdoc
        Ferrisrest::toDOM( QNetworkReply* r )
        {
            QByteArray ba = r->readAll();
            DEBUG << "todom, readAll() gave:" << tostr(ba) << endl;
            return toDOM( ba );
        }
        

        bool
        Ferrisrest::isAuthenticated() const
        {
            return m_authenticated;
        }


        QNetworkRequest
        Ferrisrest::sign( std::string earlTail, stringmap_t args )
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
        Ferrisrest::sign( QNetworkRequest request, stringmap_t args )
        {
            DEBUG << "sign(1) url:" << tostr(request.url().toString()) << endl;

            QUrl url = request.url();
            QString x = url.toString();
            DEBUG << "sign.x:" << tostr(x) << endl;
            DEBUG << "sign.args.sz:" << args.size() << endl;
            url.setEncodedUrl( x.toUtf8() );
            url = addQueryItems( url, args );
            request.setUrl( url );
//            request.setRawHeader( "Authorization", header );
            DEBUG << "sign(2) url:" << tostr(request.url().toString()) << endl;
            return request;
        }
        
        QNetworkReply*
        Ferrisrest::post( std::string earlTail, stringmap_t args, QByteArray content )
        {
            QNetworkRequest request = sign( earlTail, args );
            QUrl url = request.url();
            QString x = url.toString();
            DEBUG << "post.x:" << tostr(x) << endl;
            QNetworkReply*    reply = post( request, content );
            return reply;
        }


        QNetworkReply*
        Ferrisrest::post( stringmap_t args, QByteArray content )
        {
            std::string earlTail = ::Ferris::Ferrisrest::getBaseURI( m_serverURI );
            QNetworkRequest request = sign( earlTail, args );
            QUrl url = request.url();
            QString x = url.toString();
            DEBUG << "post.x:" << tostr(x) << endl;
            QNetworkReply*    reply = post( request, content );
            return reply;
        }



        QNetworkReply*
        Ferrisrest::post( QNetworkRequest request, QByteArray content )
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
        ::Ferris::Ferrisrest::fh_ferrisrest getFerrisrest( const std::string& server )
        {
            ::Ferris::Ferrisrest::fh_ferrisrest ret( new ::Ferris::Ferrisrest::Ferrisrest( server ) );
            return ret;
        }
        
    };
    
    
    
    /****************************************/
    /****************************************/
    /****************************************/
    
};
