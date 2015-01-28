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

#ifndef _ALREADY_INCLUDED_FERRIS_SHARED_VIMEO_H_
#define _ALREADY_INCLUDED_FERRIS_SHARED_VIMEO_H_

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

#include <QtOAuth>


namespace Ferris
{
    namespace Vimeo
    {
        
        class Vimeo;
        FERRIS_SMARTPTR( Vimeo, fh_vimeo );

        class Upload;
        FERRIS_SMARTPTR( Upload, fh_Upload );
    
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/


        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
    
        class FERRISEXP_API Vimeo
            :
            public QObject,
            public Handlable
        {
            Q_OBJECT;

            QNetworkAccessManager* getQManager();
            friend class Upload;
            
            std::string m_uid;
            std::string m_username;
            std::string m_fullname;
            std::string m_apiKey;
            std::string m_apiSecret;
            std::string m_authToken;           //< Token used only during auth
            std::string m_authTokenSecret;     //< Secret for above
            std::string m_token;               //< Main token used for requests
            std::string m_tokenSecret;         //< Secret for above
            QOAuth::Interface* m_qauth;

            QNetworkResponseWaiter m_waiter;

            QNetworkReply* get( const std::string& meth, QNetworkRequest req, stringmap_t args = stringmap_t() );

           
            QNetworkReply* post( const std::string& meth, QNetworkRequest req, bool addAuth = true );
            QNetworkReply* post( QNetworkRequest req, const std::string& body );
            QNetworkReply* waitfor( QNetworkReply* reply );

            QNetworkRequest sign( QUrl u, stringmap_t args = stringmap_t() );
            QNetworkRequest sign( QNetworkRequest request,
                                  stringmap_t args = stringmap_t(),
                                  QOAuth::HttpMethod httpMethod = QOAuth::POST  );
            QNetworkRequest signPUT( QNetworkRequest request, stringmap_t args = stringmap_t() );

            fh_domdoc toDOM( const QByteArray& ba );

        public:

            Vimeo();
            virtual ~Vimeo();

            std::string getUserID();
            std::string getUserName();
            std::string getFullName();
            bool        haveAuthToken();
            std::string getAuthToken();
            bool isAuthenticated() const;
            bool        haveAPIKey();
            std::string APIKey();
            std::string APISecret();
            std::string getToken() const;
            QByteArray& fetchURL( const std::string earl, QByteArray& ret );
            
            //////////////////////
            // Web service API
            //////////////////////

            std::string requestToken( stringmap_t args = stringmap_t() );
            stringmap_t accessToken( const std::string& verifier,
                                     stringmap_t args = stringmap_t() );
            
            std::string testLogin();

            
            fh_Upload    createUpload();

            QNetworkRequest createRequest( QUrl u );
            QNetworkRequest createRequest( const std::string& u = "" );
            QNetworkReply* callMeth( const std::string& meth, QNetworkRequest req, stringmap_t args = stringmap_t() );
                                       
        public slots:
            void handleFinished();
        
        };

        /****************************************/
        /****************************************/
        /****************************************/

        class FERRISEXP_API Upload
            :
            public WebServicesUpload
        {
            fh_vimeo m_vim;

            fh_StreamToQIODevice m_streamToQIO;
            QNetworkReply* m_streamToQIO_reply;
            std::string m_uploadTicket;
            fh_iostream m_uploadBaseStream;
            fh_iostream m_uploadDigestStream;
            
        public:

            Upload( fh_vimeo vim );
            void streamingUploadComplete();
            fh_iostream createStreamingUpload( const std::string& ContentType );
        };
        
    };
    
    /****************************************/
    /****************************************/
    /****************************************/
    
    namespace Factory
    {
        Vimeo::fh_vimeo getVimeo();
    };
};

#endif
