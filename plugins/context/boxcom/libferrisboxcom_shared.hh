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

#ifndef _ALREADY_INCLUDED_FERRIS_SHARED_BOXCOM_H_
#define _ALREADY_INCLUDED_FERRIS_SHARED_BOXCOM_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisQt_private.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/SignalStreams.hh>
#include <Ferris/FerrisWebServices_private.hh>

#include <QObject>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QCoreApplication>


namespace Ferris
{
    /****************************************/
    /****************************************/
    /****************************************/

    class BoxComFile;
    FERRIS_SMARTPTR( BoxComFile, fh_BoxComFile );
    typedef std::list< fh_BoxComFile > boxfiles_t;
    class BoxComClient;
    FERRIS_SMARTPTR( BoxComClient, fh_BoxComClient );

    // class FERRISEXP_API BoxComPermission
    //     :
    //     public Handlable
    // {
    //   public:

    //     int m_perm;
    //     std::string m_name;
    //     enum 
    //     {
    //         NONE = 0, READ = 1, WRITE = 2
    //     };
        
    //     BoxComPermission( int perm, std::string name )
    //         : m_perm( perm )
    //         , m_name( name )
    //     {
    //     }
    //     std::string getRoleString()
    //     {
    //         if( m_perm == NONE )
    //             return "none";
    //         if( m_perm == READ )
    //             return "read";
    //         if( m_perm == WRITE )
    //             return "write";
    //         return "unknown!";
    //     }
        
    // };
    // FERRIS_SMARTPTR( BoxComPermission, fh_BoxComPermission );
    // typedef std::list< fh_BoxComPermission > BoxComPermissions_t;
    
    
    class FERRISEXP_API BoxComFile
        :
        public QObject,
        public Handlable
    {
        Q_OBJECT;
        typedef BoxComFile _Self;

    //     fh_StreamToQIODevice m_streamToQIO;
    //     QNetworkReply* m_streamToQIO_reply;
    //     void OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m );

    //     QNetworkRequest addAuth( QNetworkRequest req );
    //     static std::string DRIVE_BASE();
        
    //     fh_BoxComClient getDrive() 
    //     {
    //         return m_gd;
    //     }
        
      public:

        fh_BoxComClient m_gd;
    //     string m_id;
    //     string m_etag;
    //     string m_rdn;
    //     string m_mime;
    //     string m_title;
    //     string m_desc;
    //     string m_earl;
    //     string m_earlview;
    //     string m_ext;
    //     string m_md5;
    //     long long m_sz;
    //     time_t m_ctime;
    //     time_t m_mtime;
    //     time_t m_mmtime;

    //     time_t parseTime( const std::string& v );
    //     QNetworkRequest createRequest( const std::string& earl );

        
      public:
        
        BoxComFile( fh_BoxComClient gd = 0, QVariantMap dm = QVariantMap() );

    //     fh_istream getIStream();
    //     fh_iostream getIOStream();
    //     bool isDir() const;

    //     void updateMetadata( const std::string& key, const std::string& value );
    //     void updateMetadata( stringmap_t& update );
    //     QNetworkReply* wait(QNetworkReply* reply );

    //     fh_BoxComFile createFile( const std::string& title );

    //     BoxComPermissions_t readPermissions();
    //     void sharesAdd( stringlist_t& emails );
    //     void sharesAdd( std::string email );

    };
    
    
    class FERRISEXP_API BoxComClient
        :
        public QObject,
        public Handlable
    {
        Q_OBJECT;
        friend class BoxComFile;
        
        string m_clientID;
        string m_secret;
        string m_accessToken;
        string m_refreshToken;
        time_t m_accessToken_expiretime;
        QNetworkResponseWaiter m_waiter;

        static std::string AUTH_BASE();
        static std::string DRIVE_BASE();
        QNetworkRequest createRequest( const std::string& earlTail );

        void readAuthTokens();
        
      protected:

        BoxComClient();
        
      public:
        
        static fh_BoxComClient getBoxComClient();

        bool haveAPIKey() const;
        bool isAuthenticated() const;
        std::string requestToken( stringmap_t args = stringmap_t() );
        void accessToken( const std::string& code, stringmap_t args = stringmap_t() );
        void ensureAccessTokenFresh( int force = false );

        QNetworkReply* wait(QNetworkReply* reply );
        QNetworkReply* post( QNetworkRequest req );
        QNetworkReply* callMeth( QNetworkRequest req, stringmap_t args );
        QNetworkReply* callPost( QNetworkRequest req, stringmap_t args, const std::string& body );

        boxfiles_t filesList( const std::string& q = "", const std::string& pageToken = "" );
        QNetworkRequest addAuth( QNetworkRequest req );

                                                      
      public slots:
        void handleFinished();
        
    };
    

    
};

#endif
