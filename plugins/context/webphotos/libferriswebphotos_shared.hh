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

#ifndef _ALREADY_INCLUDED_FERRIS_SHARED_WF_H_
#define _ALREADY_INCLUDED_FERRIS_SHARED_WF_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisQt_private.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/SignalStreams.hh>
#include <FerrisStreams/Streams.hh>

#include "config.h"

#include <QObject>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QCoreApplication>


namespace Ferris
{
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class WebPhotos;
    FERRIS_SMARTPTR( WebPhotos, fh_webPhotos );
    class Contact;
    FERRIS_SMARTPTR( Contact, fh_Contact );
    typedef std::list< fh_Contact > ContactList_t;
    class Comment;
    FERRIS_SMARTPTR( Comment, fh_Comment );
    typedef std::list< fh_Comment > CommentList_t;
    class PhotoMetadata;
    FERRIS_SMARTPTR( PhotoMetadata, fh_PhotoMetadata );
    typedef std::list< fh_PhotoMetadata > photolist_t;
    class Tag;
    FERRIS_SMARTPTR( Tag, fh_Tag );
    typedef std::list< fh_Tag > TagList_t;
    
    
    enum privacy_t
    {
        PRIVACY_UNSPECIFIED = 0,
        PRIVACY_PUBLIC  = 1 << 1,
        PRIVACY_FRIENDS = 1 << 2,
        PRIVACY_FAMILY  = 1 << 3,
        PRIVACY_FF      = (PRIVACY_FAMILY | PRIVACY_FRIENDS),
        PRIVACY_PRIVATE = 1 << 5
    };

    /************************************************************/
    /************************************************************/
    /************************************************************/
    
    class FERRISEXP_API Contact
        :
        public QObject,
        public Handlable
    {
        Q_OBJECT;

        fh_webPhotos m_wf;

        std::string m_id;
        std::string m_username;
        std::string m_realname;
        std::string m_location;

        
    public:
        Contact( fh_webPhotos wf, fh_context md );
        Contact( fh_webPhotos wf, const std::string& aid );
        virtual ~Contact();

        std::string getRDN();
        std::string getID();
        std::string getUserName();
        std::string getRealName();
        std::string getLocation();

        std::string getVCard();
    };

    /************************************************************/
    /************************************************************/
    /************************************************************/

    class FERRISEXP_API Comment
        :
        public QObject,
        public Handlable
    {
        Q_OBJECT;

        fh_webPhotos m_wf;
        std::string  m_id;
        std::string  m_content;
        time_t       m_dateCreated;
        std::string  m_permalink;
        std::string  m_authorName;
        std::string  m_authorID;
        fh_Contact   m_author;
        
    public:
        Comment( fh_webPhotos wf, fh_context md );
        virtual ~Comment();

        std::string getID();
        fh_Contact  getAuthor();
        std::string getAuthorName();
        time_t      getDateCreated();
        std::string getPermalink();
        std::string getContent();
        void        setContent( const std::string& newCommentValue );

        static fh_Comment Create( fh_webPhotos wf, fh_PhotoMetadata photo, const std::string& v );
    };

    /************************************************************/
    /************************************************************/
    /************************************************************/

    class FERRISEXP_API Tag
        :
        public QObject,
        public Handlable
    {
        Q_OBJECT;
        fh_webPhotos m_wf;
        fh_PhotoMetadata m_photo;
        
        std::string  m_id;
        std::string  m_content;
        std::string  m_authorID;
        fh_Contact   m_author;
        bool         m_machineTag;
        
    public:
        Tag( fh_webPhotos wf, fh_PhotoMetadata photo, fh_context md );
        Tag( fh_webPhotos wf, fh_PhotoMetadata photo, const std::string& content );
        virtual ~Tag();
        void updateFromMetadata( fh_context md );
        
        std::string getID();
        std::string getContent();
        std::string getAuthorID();
        fh_Contact  getAuthor();
        bool        isMachineTag();
    };
    

    /************************************************************/
    /************************************************************/
    /************************************************************/
    
    class FERRISEXP_API PhotoMetadata
        :
        public QObject,
        public Handlable
    {
        Q_OBJECT;
        friend class WebPhotos;
        fh_webPhotos m_wf;
        QNetworkResponseWaiter m_waiter;

        long long m_id;
        std::string m_secret;
        std::string m_server;
        std::string m_title;
        bool m_isPrimary;
        time_t m_mtime;
        TagList_t m_taglist;
        bool m_haveInitialTagList;
        QByteArray m_thumbnailRGBAData;
        QByteArray m_RGBAData;
        
        typedef std::map< std::string, std::string > m_sizeearl_t;
        m_sizeearl_t m_sizeearl;

        std::string get_sizeearl_LargestSize();
        void ensure_sizeearl_setup();
        
        friend class Tag;
        void ensureTagsContainFullMetadata();

        QByteArray getImageRGBA( const std::string& sizeDesc );
        void ensureThumbnailRGBAData();
        void ensureRGBAData();
        
    public:
        PhotoMetadata( fh_webPhotos wf, fh_context md );
        virtual ~PhotoMetadata();
        
        long        getID();
        std::string getSecret();
        std::string getServer();
        std::string getTitle();
        bool        isPrimary();
        time_t      getMTime();

        guint32*      getThumbnailRGBAData();
        guint32       getThumbnailRGBADataSize();
        guint32*      getFullRGBAData();
        guint32       getFullRGBADataSize();
        
        
        fh_Tag        findTag( const std::string& t );
        fh_Tag        addTag( const std::string& t );
        void          removeTag( const std::string& t );
        TagList_t     getTags();
        CommentList_t getComments();
        std::string getLargestSizeImageURL();
        long long   getLargestSizeImageSize();

                                             

    public slots:
        void handleMetadataChange();
        void handleFinished();
        
                                             

    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_API PhotoSet
        :
        public Handlable
    {
        friend class WebPhotos;

        fh_webPhotos m_wf;
        long long m_id;
        long m_photocount;
        std::string m_secret;
        std::string m_server;
        std::string m_title;
        std::string m_primary;
        std::string m_desc;
        
        PhotoSet( fh_webPhotos wf, fh_context md );
    public:
        virtual ~PhotoSet();

        long        getID();
        std::string getSecret();
        std::string getServer();
        std::string getTitle();
        std::string getPrimary();
        std::string getDescription();

        photolist_t getPhotos( int page = 1,
                               privacy_t privacy = PRIVACY_UNSPECIFIED );
    };
    FERRIS_SMARTPTR( PhotoSet, fh_PhotoSet );
    typedef std::list< fh_PhotoSet > photosets_t;
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    class FERRISEXP_API WebPhotos
        :
        public Handlable
    {
        friend class WebPhotoUpload;

        std::string m_username;
        fh_rex m_includeEAIsPresentRegex;
        fh_rex m_includeEAandValueRegex;
        ContactList_t m_contactlist;

    protected:

        virtual std::string getRESTURL();

    public:

        virtual std::string getUploadURL();
        virtual std::string getAuthURL();
        virtual std::string getPostUploadURLPrefix();
        virtual std::string getAPIKey();
        virtual std::string getAPISecret();
        
    public:

        WebPhotos( const std::string& username );
        virtual ~WebPhotos();

        virtual std::string getImplementationShortName();
        
        long getDefaultLargestDimension();
        void setDefaultLargestDimension( long v );

        fh_rex getDefaultIncludeEAIsPresentRegex();
        std::string getDefaultIncludeEAIsPresentRegexString();
        void setDefaultIncludeEAIsPresentRegex( const std::string& s );
        fh_rex getDefaultIncludeEAandValueRegex();
        std::string getDefaultIncludeEAandValueRegexString();
        void setDefaultIncludeEAandValueRegex( const std::string& s );

        bool isDefaultImageProtectionPublic();
        void setDefaultImageProtectionPublic( bool v );
        bool isDefaultImageProtectionFriend();
        void setDefaultImageProtectionFriend( bool v );
        bool isDefaultImageProtectionFamily();
        void setDefaultImageProtectionFamily( bool v );
        
        
        bool haveAPIKey();
        std::string getToken();
        bool haveToken();
        std::string getDefaultUsername();
        std::string getUserName();
        std::string getFullName();

        fh_context handleCallIO( stringmap_t args );
        std::string makeSignedString( stringmap_t args );
        fh_context signedCall( std::string methodname,
                               stringmap_t args,
                               bool dontAutoIncludeToken = false );

        /////////////
        // These are the public API
        ////////////
        
        photolist_t getMyRecent( time_t tt = 0, int pagenum = 1, int limit = 500 );
        photolist_t getNotInSet( int page,
                                 privacy_t privacy,
                                 time_t min_upload_date, time_t max_upload_date,
                                 time_t min_taken_date,  time_t max_taken_date );
        photolist_t getNotInSet( int page = 1,
                                 privacy_t privacy = PRIVACY_UNSPECIFIED );

        photosets_t getPhotosets( const std::string uid = "" );
        photolist_t getFavoritePhotos( const std::string uid = "" );

        fh_PhotoMetadata getPhotoByID( const std::string photo_id );

        ContactList_t getContacts();
        
        ///////////
        // These are only needed during / just after an auth
        ///////////
        void setToken( const std::string& token );
        void setUserName( const std::string& s );
        void setFullName( const std::string& s );
        void setDefaultUsername( const std::string& s );
    };

    

    class FERRISEXP_API WebPhotoUpload
        :
        public Handlable
    {
        fh_webPhotos m_wf;
        stringlist_t m_uploadedPhotoIDList;
        fh_ostream m_verboseStream;
        bool m_debugVerbose;
        fh_rex m_includeEAisPresentRegex;
        fh_rex m_includeEAandValueRegex;
        bool m_publicViewable;
        bool m_FriendViewable;
        bool m_FamilyViewable;
        stringlist_t m_DescriptionFromEAList;
        long m_largestDimensionExplicit;
        stringmap_t m_args;
        std::string m_title;
        std::string m_filename;


        void throwIfStatusIsBad( fh_context c, const std::string& res );
        
    public:
    
        WebPhotoUpload( fh_webPhotos wf );
        ~WebPhotoUpload();

        void setVerboseStream( fh_ostream m_verboseStream );
        void setDebugVerbose( bool v );
        void setIncludeEAisPresentRegex( fh_rex r );
        void setIncludeEAandValueRegex( fh_rex r );
        void setPublicViewable( bool v );
        void setFriendViewable( bool v );
        void setFamilyViewable( bool v );
        void setDescriptionFromEA( const std::string& s );
        void appendDescriptionFromEA( const std::string& s );
        void setLargestDimensionExplicit( long v );
    
        std::string getTagString( fh_context c );

        // Use these two to perform a streaming upload if the original image
        // is to be sent.
        fh_StreamToQIODevice m_streamToQIO;
        QNetworkReply* m_reply;
        void inspectSourceForMetadata( fh_context c );
        void inspectSourceForMetadataDescriptionAttribute( stringmap_t& args, fh_context c );
        void streamingUploadComplete();
        fh_iostream getUploadIOStream( int ContentLength, const std::string& title, const std::string& desc );

        // possibly scale and send.
        void upload( fh_context c,
                     const std::string& title = "",
                     const std::string& desc = "" );
        void post( stringmap_t args );

        // scale a file
        fh_context makeScaledImage( fh_context c );

        
        std::string getPostUploadURL();
        stringlist_t& getUploadedPhotoIDList();



        
        // private methods
        static bool adjustWidthAndHeightForDesired( int width, int height, int& w, int& h, long MaxDesiredWidthOrHeight );

    };
    FERRIS_SMARTPTR( WebPhotoUpload, fh_webPhotoUpload );

    
    /****************************************/
    /****************************************/
    /****************************************/
    
    namespace Factory
    {
        FERRISEXP_API fh_webPhotos getDefaultFlickrWebPhotos();
        FERRISEXP_API fh_webPhotos getDefault23hqWebPhotos();
        FERRISEXP_API fh_webPhotos getDefaultZooomrWebPhotos();
        FERRISEXP_API fh_webPhotos getDefaultPixelPipeWebPhotos();

        FERRISEXP_API fh_webPhotos getDefaultWebPhotosForShortName( const std::string& n );
    };
};


#endif
