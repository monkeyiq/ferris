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

    $Id: libferrispostgresqlshared.hh,v 1.2 2006/12/07 06:49:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_SHARED_IDENTICA_H_
#define _ALREADY_INCLUDED_FERRIS_SHARED_IDENTICA_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/SignalStreams.hh>
#include <Ferris/FerrisQt_private.hh>
#include <Ferris/FerrisWebServices_private.hh>

#include <QObject>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QCoreApplication>

namespace QOAuth
{
    class Interface;
};

namespace Ferris
{
    namespace Identica
    {
        
        class Identica;
        FERRIS_SMARTPTR( Identica, fh_identica );

    
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/


        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
    
        class FERRISEXP_API Identica
            :
            public QObject,
            public Handlable
        {
            Q_OBJECT;

            QNetworkAccessManager* getQManager();
            
            std::string m_baseURI;             //< Base to prepend to partial URL fragments
            std::string m_secDBPrefix;         //< Prefix to load/save the m_token and m_tokenSecret to in the secure config
            std::string m_consumerKeyPrefix;   //< Try to load the consumer key/secret from m_consumerKeyPrefix-api-key.txt
            //                                 //  and m_consumerKeyPrefix-api-secret.txt
            std::string m_authToken;           //< Token used only during auth
            std::string m_authTokenSecret;     //< Secret for above
            std::string m_token;               //< Main token used for requests
            std::string m_tokenSecret;         //< Secret for above
            QOAuth::Interface* m_qauth;

            std::string m_uid;
            std::string m_username;
            std::string m_fullname;

            fh_domdoc toDOM( const QByteArray& ba );

        public:

            Identica( const std::string& baseURI,
                      const std::string& secdbprefix = "",
                      const std::string& consumerKeyPrefix = "" );
            virtual ~Identica();

            std::string getUserID();
            std::string getUserName();
            std::string getFullName();

            //////////////////////
            // Web service API
            //////////////////////

            bool isAuthenticated() const;
            bool haveAPIKey() const;
            void setBaseURI( const std::string& earl );

            void setStatus( const std::string& s );
            
            std::string requestToken( const std::string& earlTail,
                                      stringmap_t args = stringmap_t() );
            stringmap_t accessToken( const std::string& earlTail,
                                     const std::string& verifier,
                                     stringmap_t args = stringmap_t() );

            QNetworkRequest sign( std::string earlTail, stringmap_t args );
            QNetworkRequest sign( QNetworkRequest request, stringmap_t args );
            
            QNetworkReply* post( std::string earlTail, stringmap_t args, QByteArray content = QByteArray() );
            QNetworkReply* post( QNetworkRequest request, QByteArray content = QByteArray() );
            

            void setToken( const std::string& s );
            void setTokenSecret( const std::string& s );
            
            
        };

        
        /****************************************/
        /****************************************/
        /****************************************/
    };
    
    /****************************************/
    /****************************************/
    /****************************************/
    
    namespace Factory
    {
        // rbase is the REST base URL
        // kbase is the base of the key and secret files in ~/.ferris
        Identica::fh_identica getIdentica( const std::string& rbase, const std::string& kbase );

        // service is the name of a directory in ~/.ferris/identica
        Identica::fh_identica getIdentica( const std::string& service );
    };
};

#endif
