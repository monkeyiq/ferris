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
#include <config.h>

#ifndef _ALREADY_INCLUDED_FERRIS_SHARED_FERRISREST_H_
#define _ALREADY_INCLUDED_FERRIS_SHARED_FERRISREST_H_

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


namespace Ferris
{
    namespace Ferrisrest
    {
        FERRISEXP_EXPORT userpass_t getUserPass( const std::string& server );
        FERRISEXP_EXPORT void       setUserPass( const std::string& server,
                                                 const std::string& user, const std::string& pass );
        FERRISEXP_EXPORT std::string getBaseURI( const std::string& server );
        FERRISEXP_EXPORT void        setBaseURI( const std::string& server, const std::string& baseurl );
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        class Ferrisrest;
        FERRIS_SMARTPTR( Ferrisrest, fh_ferrisrest );

        class FERRISEXP_API Ferrisrest
            :
            public QObject,
            public Handlable
        {
            Q_OBJECT;


            std::string m_serverURI;           //< Base to prepend to partial URL fragments
            bool        m_authenticated;
        

        public:

            Ferrisrest( const std::string& server );
            virtual ~Ferrisrest();

            QNetworkAccessManager* getQManager();

            fh_domdoc toDOM( const QByteArray& ba );
            fh_domdoc toDOM( QNetworkReply* r );
            
            bool isAuthenticated() const;
            bool ensureAuthenticated();

            std::string getBaseURI() const;
            QNetworkRequest sign( std::string earlTail, stringmap_t args );
            QNetworkRequest sign( QNetworkRequest request, stringmap_t args );

            
            QNetworkReply* post( stringmap_t args, QByteArray content = QByteArray() );
            QNetworkReply* post( std::string earlTail, stringmap_t args, QByteArray content = QByteArray() );
            QNetworkReply* post( QNetworkRequest request, QByteArray content = QByteArray() );
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
        Ferrisrest::fh_ferrisrest getFerrisrest( const std::string& server );
    };
};

#endif
