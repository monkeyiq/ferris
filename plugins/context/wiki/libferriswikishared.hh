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

    $Id: libferrispostgresqlshared.hh,v 1.3 2010/09/24 21:31:46 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#ifndef _ALREADY_INCLUDED_FERRIS_SHARED_WIKI_H_
#define _ALREADY_INCLUDED_FERRIS_SHARED_WIKI_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <TypeDecl.hh>
#include <Ferris.hh>

#include <Ferris/FerrisQt_private.hh>
#include <QObject>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QCoreApplication>

namespace Ferris
{
    FERRISEXP_EXPORT userpass_t getWikiUserPass( const std::string& server );
    FERRISEXP_EXPORT void setWikiUserPass( const std::string& server,
                                           const std::string& user, const std::string& pass );

    class WikiPage
    {
        std::string m_name;
        int m_pageID;
        int m_ns;
        
    public:
        WikiPage( const std::string& name, int pageID, int ns );
        std::string getName() const;
        int getPageID() const;
    };

    typedef std::list< WikiPage > WikiPageList_t;
    
    
    class Wiki;
    FERRIS_SMARTPTR( Wiki, fh_Wiki );

    class Wiki
        :
        public QObject,
        public Handlable
    {
        Q_OBJECT;
        std::string m_serverURL;
        std::string m_editToken;
        std::string m_uploadToken;
        std::string m_moveToken;
        QNetworkResponseWaiter m_waiter;

        // Use these two to perform a streaming upload 
        fh_StreamToQIODevice m_streamToQIO;
        QNetworkReply* m_reply;
        
    public:
        Wiki( const std::string& serverURL );
        ~Wiki();

        QUrl getAPIUrl();
        std::string request( stringmap_t args );
        std::string request( stringmap_t args, const QByteArray& postba );
        void login();
        WikiPageList_t getPageList();
        std::string move( const std::string& oldrdn, const std::string& newrdn );
        
        void ensureEditToken( const std::string& titles = "" );
        void ensureUploadToken();
        void ensureMoveToken();
        fh_iostream getEditIOStream( int ContentLength, const std::string& title );
        fh_iostream getUploadIOStream( int ContentLength, const std::string& title );
        void streamingUploadComplete();
                                      
    public slots:
        
        void handleFinished();
    };
    
};

#endif
