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

#ifndef _ALREADY_INCLUDED_FERRIS_SHARED_FACEBOOK_H_
#define _ALREADY_INCLUDED_FERRIS_SHARED_FACEBOOK_H_

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
    namespace Facebook
    {
        
        class Facebook;
        FERRIS_SMARTPTR( Facebook, fh_facebook );

        class Contact;
        FERRIS_SMARTPTR( Contact, fh_Contact );
        typedef std::list< fh_Contact > ContactList_t;
        typedef std::map< std::string, fh_Contact > ContactMap_t;

        class Album;
        FERRIS_SMARTPTR( Album, fh_Album );
        typedef std::list< fh_Album > AlbumList_t;
        typedef std::map< std::string, fh_Album > AlbumMap_t;
    
        class Comment;
        FERRIS_SMARTPTR( Comment, fh_Comment );
        typedef std::list< fh_Comment > CommentList_t;
        typedef std::map< std::string, fh_Comment > CommentMap_t;

        class Post;
        FERRIS_SMARTPTR( Post, fh_Post );
        typedef std::list< fh_Post > PostList_t;
        typedef std::map< std::string, fh_Post > PostMap_t;

        typedef std::list< long > uidList_t;

        class Upload;
        FERRIS_SMARTPTR( Upload, fh_Upload );
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        class FERRISEXP_API Contact
            :
            public QObject,
            public Handlable
        {
            Q_OBJECT;

            fh_facebook m_fb;
            std::string m_uid;
            std::string m_name;
            std::string m_picURL;
            std::string m_url;
            std::string m_type;

            std::string m_birthday;
            std::string m_about;
            std::string m_city;
            std::string m_state;
            std::string m_country;
            std::string m_firstName;
            std::string m_lastName;
            std::string m_interests;
            std::string m_movies;
            std::string m_music;
            std::string m_blurb;
            std::string m_sex;
            std::string m_websites;
        
            friend class Facebook;
            Contact( fh_facebook fb, fh_domdoc dom, DOMElement* e );
            void update( fh_domdoc dom, DOMElement* e );
        public:

            virtual ~Contact();

            std::string getID();
            std::string getUID();
            std::string getName();
            std::string getFirstName();
            std::string getLastName();
            std::string getPictureURL();
            std::string getURL();
            std::string getType();
            std::string getLocation();
            std::string getRDN();

            std::string getVCard();
        };

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        class FERRISEXP_API Album
            :
            public QObject,
            public Handlable
        {
            Q_OBJECT;

            fh_facebook m_fb;
            std::string m_id;
            std::string m_cover_id;
            std::string m_owner_id;
            std::string m_name;
            std::string m_desc;
            std::string m_url;
            std::string m_type;
            int    m_size;
            time_t m_createdTime;
            time_t m_modifiedTime;
        
            friend class Facebook;
            Album( fh_facebook fb, fh_domdoc dom, DOMElement* e );
            void update( fh_domdoc dom, DOMElement* e );
        public:

            virtual ~Album();

            std::string getID();
            std::string getCoverID();
            std::string getOwnerID();
            std::string getName();
            std::string getDesc();
            std::string getURL();
            std::string getType();
            int    getSize();
            time_t getCreatedTime();
            time_t getModifiedTime();
        };
    
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        class FERRISEXP_API Comment
            :
            public QObject,
            public Handlable
        {
            Q_OBJECT;

            fh_facebook m_fb;
            std::string m_id;
            std::string m_author_id;
            std::string m_message;
            time_t      m_mtime;

            friend class Post;
            Comment( fh_facebook fb, fh_domdoc dom, DOMElement* e );
            void update( fh_domdoc dom, DOMElement* e );
        public:

            virtual ~Comment();

            std::string getID();
            std::string getAuthorID();
            std::string getMessage();
            time_t      getMTime();
        };
    

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
    
        class FERRISEXP_API Post
            :
            public QObject,
            public Handlable
        {
            Q_OBJECT;

            fh_facebook m_fb;
            std::string m_id;
            std::string m_viewer_id;
            std::string m_source_id;
            std::string m_type;
            std::string m_app_id;
            std::string m_message;
            std::string m_permalink;
            time_t       m_createTime;
            time_t       m_updateTime;
            CommentMap_t m_comments;
        
            friend class Facebook;
            Post( fh_facebook fb, fh_domdoc dom, DOMElement* e );
            void update( fh_domdoc dom, DOMElement* e );
        public:

            virtual ~Post();

            std::string getID();
            std::string getViewerID();
            std::string getSourceID();
            std::string getType();
            std::string getAppID();
            std::string getMessage();
            std::string getPermalink();
            time_t      getCreateTime();
            time_t      getUpdateTime();
            std::string getRDN();
        
            // extra work needed for these methods
            std::string getAuthorName();
        

            CommentMap_t getComments();

        };

    

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
    
        class FERRISEXP_API Facebook
            :
            public QObject,
            public Handlable
        {
            Q_OBJECT;

            QNetworkAccessManager* getQManager();

            std::string m_uid;
            std::string m_username;
            std::string m_sessionKey;
            std::string m_sessionSecret;
            std::string m_apiKey;
            std::string m_apiSecret;

            ContactMap_t m_friends;
            ContactMap_t m_contacts;
            PostMap_t    m_recentStreamPosts;
            AlbumMap_t   m_albums;
            std::map< time_t, std::string > m_statusMessages;
            
            QNetworkResponseWaiter m_waiter;

            void mergeContact( ContactMap_t& dst, fh_Contact c );
        
            //
            // Web service API
            //
            void getStream( time_t start = 0, time_t end = 0, int limit = 30, int viewer_id = 0 );
            uidList_t getFriendIDList();
            ContactMap_t getUserInfo( uidList_t uids, const std::string fields_add = "" );
            ContactMap_t getUserInfo( const std::string fields_add = "" );
            ContactMap_t getStandardUserInfo( uidList_t uids );
            ContactMap_t getStandardUserInfo();

        public:

            Facebook();
            virtual ~Facebook();

            std::string getUserID();
            std::string getUserName();
            bool        haveSessionKey();
            std::string getSessionKey();
            bool        haveAPIKey();
            std::string APIKey();

            QByteArray& fetchURL( const std::string earl, QByteArray& ret );
        
            //////////////////////
            // Web service API
            //////////////////////

            fh_Upload    createUpload( const std::string& methodName = "facebook.video.upload" );
            
            // these are based on Stream.get web API //
            PostMap_t    getRecentStreamPosts();
            ContactMap_t getContacts();
            ContactMap_t getFriends();
 
        
            std::string getStatus();
            void        setStatus( const std::string& s );
            std::map< time_t, std::string > getStatusMessages();
        
            std::string createAuthToken();  // Auth.createToken
            std::string getAuthSession( const std::string& authToken );   // Auth.getSession

            ////////////////////
            // Internal use only
            //
            QNetworkRequest createRequest( QUrl u );
            QNetworkRequest createRequest( const std::string& u = "" );
            QNetworkReply* get( const std::string& meth, QNetworkRequest req );
            QUrl sign( QUrl u );
                                                                      
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
            typedef Upload _Self;
            
            fh_facebook m_fb;
            const std::string m_methodName;

            void OnStreamClosed( fh_istream& ss, std::streamsize tellp, const std::string& ContentType );
            fh_iostream real_createStreamingUpload( const std::string& ContentType );
            
        public:

            Upload( fh_facebook fb, const std::string& methodName );

            void streamingUploadComplete();
            fh_iostream createStreamingUpload( const std::string& ContentType );

            static long getMaxDesiredWidthOrHeight();
            static void setMaxDesiredWidthOrHeight( long v );
        };
    
        /****************************************/
        /****************************************/
        /****************************************/
    };
    
    namespace Factory
    {
        Facebook::fh_facebook getFacebook();
    };
};

#endif
