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

#ifndef _ALREADY_INCLUDED_FERRIS_SHARED_GOOGLE_H_
#define _ALREADY_INCLUDED_FERRIS_SHARED_GOOGLE_H_

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
    FERRISEXP_EXPORT userpass_t getGoogleUserPass( const std::string& server = "" );
    FERRISEXP_EXPORT void setGoogleUserPass( const std::string& server,
                                                 const std::string& user, const std::string& pass );

    FERRISEXP_EXPORT std::string columnNumberToName( int col );
    
    
    class GoogleClient;
    FERRIS_SMARTPTR( GoogleClient, fh_GoogleClient );
    
    class GoogleSpreadSheet;
    FERRIS_SMARTPTR( GoogleSpreadSheet, fh_GoogleSpreadSheet );
    typedef std::list< fh_GoogleSpreadSheet > GoogleSpreadSheets_t;

    class GoogleDocument;
    FERRIS_SMARTPTR( GoogleDocument, fh_GoogleDocument );
    typedef std::list< fh_GoogleDocument > GoogleDocuments_t;

    class GoogleDocumentFolder;
    FERRIS_SMARTPTR( GoogleDocumentFolder, fh_GoogleDocumentFolder );
    typedef std::list< fh_GoogleDocumentFolder > GoogleDocumentFolders_t;
    
    class GoogleWorkSheet;
    FERRIS_SMARTPTR( GoogleWorkSheet, fh_GoogleWorkSheet );
    typedef std::list< fh_GoogleWorkSheet > GoogleWorkSheets_t;
    
    class GoogleWorkSheetCell;
    FERRIS_SMARTPTR( GoogleWorkSheetCell, fh_GoogleWorkSheetCell );

    class YoutubeUpload;
    FERRIS_SMARTPTR( YoutubeUpload, fh_YoutubeUpload );
    
//     class FERRISEXP_API GoogleClientResponseWaiter
//     {
//         GMainLoop* m_loop;

// //        QCoreApplication m_app;
//     public:
//         GoogleClientResponseWaiter();
//         void block();
//         void unblock();
//     };
    
    /****************************************/
    /****************************************/
    /****************************************/

    class FERRISEXP_API GoogleClient
        :
        public QObject,
        public Handlable
    {
        Q_OBJECT;

        QNetworkResponseWaiter m_waiter;
        // service -> token, eg, wise -> Auth=xxxx
        typedef std::map< std::string, std::string > m_authTokens_t;
        m_authTokens_t m_authTokens;
        QNetworkAccessManager* m_qmanager;
        QNetworkAccessManager* getQManager();
        friend class GoogleSpreadSheet;
        std::string m_documentsETag;

        void Authenticate_ClientLogin( const std::string& service = "wise" );

        
    public:

        GoogleSpreadSheets_t listSheets();
        fh_GoogleDocumentFolder getRootFolder();
        GoogleDocuments_t    listDocuments();


        fh_YoutubeUpload createYoutubeUpload();

        
        // private:
        
        GoogleClient();
        void addAuth( QNetworkRequest& r, const std::string& service = "wise" );
        
        void localtest();
        QNetworkRequest createRequest( QUrl& u, const std::string& service = "wise", const std::string& gversion = " 3.0" );
        QNetworkReply* get( QNetworkRequest req );
        QNetworkReply* put( QNetworkRequest req, QByteArray& ba );
        QNetworkReply* put( QNetworkRequest req, const std::string& data );
        QNetworkReply* post( QNetworkRequest req, const std::string& data );

        std::string getYoutubeDevKey();
        
    public slots:
        void dumpReplyToConsole(QNetworkReply* r );
        void replyFinished(QNetworkReply* r );
        void dumpReplyToConsole();
        void handleFinished();
        void handleFinished( QNetworkReply* );
        
    };

    /****************************************/
    /****************************************/
    /****************************************/

    class FERRISEXP_API GoogleDocumentFolder
        :
        public QObject,
        public Handlable
    {
        Q_OBJECT;

        fh_GoogleClient m_gc;
        std::string m_folderID;
        std::string m_title;
        std::string m_editURL;
        bool m_haveRead;
        GoogleDocumentFolders_t m_folders;
        GoogleDocuments_t       m_docs;

        void addItem( fh_domdoc dom, DOMElement* e );
        void read( bool force = false );
        
    public:
        GoogleDocumentFolder( fh_GoogleClient gc, const std::string& folderID );
        GoogleDocumentFolder( fh_GoogleClient gc, fh_domdoc dom, DOMElement* e );

        std::string getEditURL();
        std::string getTitle();

        fh_GoogleDocument createDocument( const std::string& data, const std::string& slug, const std::string& format );
        GoogleDocumentFolders_t getSubFolders();
        GoogleDocuments_t       getDocuments();

    public slots:
    };

    /****************************************/
    /****************************************/
    /****************************************/
    
    class FERRISEXP_API GoogleDocument
        :
        public QObject,
        public Handlable
    {
        Q_OBJECT;

        fh_GoogleClient m_gc;
        std::string m_title;
        std::string m_editURL;
        std::string m_docID;
        bool m_haveRead;

        friend class GoogleDocumentFolder;
        QNetworkRequest prepareCreateOrReplace( const std::string& data,
                                                const std::string& slug,
                                                const std::string& format,
                                                bool create );
        GoogleDocument( fh_GoogleClient gc,
                        const std::string& data,
                        const std::string& slug,
                        const std::string& format );
        

    public:
        GoogleDocument( fh_GoogleClient gc, fh_domdoc dom, DOMElement* e );

        std::string getEditURL();
        std::string getTitle();

        fh_istream exportToFormat(   const std::string& format );
        void       importFromFormat( fh_istream iss, const std::string& format = "" );
                              
    public slots:
    };
    
    /****************************************/
    /****************************************/
    /****************************************/

    class FERRISEXP_API GoogleSpreadSheet
        :
        public QObject,
        public Handlable
    {
        Q_OBJECT;
        
        fh_GoogleClient m_gc;
        std::string m_title;
        std::string m_feedURL;
        bool m_haveRead;
        GoogleWorkSheets_t m_sheets;
//        std::string m_docsAPIAuth;
        
        std::string getDocIDFromTitle( const std::string& title );
//        void addDocsAPIAuth( QNetworkRequest& r );
        
    public:
        GoogleSpreadSheet( fh_GoogleClient gc, fh_domdoc dom, DOMElement* e );
        
        std::string getFeedURL();
        std::string getTitle();
        GoogleWorkSheets_t listSheets();
        fh_GoogleWorkSheet createWorkSheet( const std::string& name );
        fh_istream exportToFormat( const std::string& format );
        void       importFromFormat( fh_istream iss, const std::string& format = "" );

        fh_GoogleDocument getDocument();
                                                                                     
    public slots:
//        void addDocsAPIAuth_reply(QNetworkReply* r );
    };

    /****************************************/
    /****************************************/
    /****************************************/
    
    class FERRISEXP_API GoogleWorkSheet
        :
        public QObject,
        public Handlable
    {
        fh_GoogleClient m_gc;
        std::string m_title;
        std::string m_cellFeedURL;
        std::string m_cellFeedETag;
        std::string m_editURL;
        bool m_cellsFetched;
        bool m_delayCellSync;
        std::string m_etag;

        typedef std::map< std::string, int > m_colnames_t;
        m_colnames_t m_colnames;
        
        typedef std::map< std::pair< int, int >, fh_GoogleWorkSheetCell > m_cells_t;
        m_cells_t m_cells;

        
        
    public:
        GoogleWorkSheet( fh_GoogleClient gc, fh_domdoc dom, DOMElement* e );
        
        std::string getCellFeedURL();
        std::string getTitle();

        fh_GoogleWorkSheetCell getCell( int row, int col );
        fh_GoogleWorkSheetCell getCell( int row, const std::string& colName );

        bool getDelayCellSync();
        void setDelayCellSync( bool v );
        void sync();

        int getLargestRowNumber();
        std::list< std::string > getColumnNames();

        void ensureCellsFetched();
        void fetchCells();
        
    };


    class FERRISEXP_API GoogleWorkSheetCell
        :
        public QObject,
        public Handlable
    {
        fh_GoogleClient m_gc;
        fh_GoogleWorkSheet m_ws;
        int m_row;
        int m_col;
        std::string m_cellURL;
        std::string m_value;
        bool m_dirty;
        bool m_created;
        bool m_isFormula;
        std::string m_etag;
        
    public:
        GoogleWorkSheetCell( fh_GoogleClient gc, fh_domdoc dom, DOMElement* e,
                             fh_GoogleWorkSheet ws, int cellURLLength );
        GoogleWorkSheetCell( fh_GoogleClient gc, fh_GoogleWorkSheet ws,
                             int r, int c, const std::string& v );
        void update( fh_domdoc dom, DOMElement* e, int cellURLLength );
        
        int row();
        int col();

        std::string value();
        void        value( const std::string& s );

        std::string editURL();

        void writeUpdateBlock( std::stringstream& ss );
        bool isCreated();
        bool isFormula();
        void sync();
    };
    
    
    /****************************************/
    /****************************************/
    /****************************************/

    class FERRISEXP_API YoutubeUpload
        :
        public WebServicesUpload
    {
        fh_GoogleClient m_gc;
        
    public:

        YoutubeUpload( fh_GoogleClient gc );
        virtual ~YoutubeUpload();

        void streamingUploadComplete();
        fh_iostream createStreamingUpload( const std::string& ContentType );
    };
    
    /****************************************/
    /****************************************/
    /****************************************/


    namespace Factory
    {
        fh_GoogleClient createGoogleClient();
    };

    /****************************************/
    /****************************************/
    /****************************************/

    class GDriveFile;
    FERRIS_SMARTPTR( GDriveFile, fh_GDriveFile );
    typedef std::list< fh_GDriveFile > files_t;
    class GDriveClient;
    FERRIS_SMARTPTR( GDriveClient, fh_GDriveClient );

    class FERRISEXP_API GDrivePermission
        :
        public Handlable
    {
      public:

        int m_perm;
        std::string m_name;
        enum 
        {
            NONE = 0, READ = 1, WRITE = 2
        };
        
        GDrivePermission( int perm, std::string name )
            : m_perm( perm )
            , m_name( name )
        {
        }
        std::string getRoleString()
        {
            if( m_perm == NONE )
                return "none";
            if( m_perm == READ )
                return "read";
            if( m_perm == WRITE )
                return "write";
            return "unknown!";
        }
        
    };
    FERRIS_SMARTPTR( GDrivePermission, fh_GDrivePermission );
    typedef std::list< fh_GDrivePermission > GDrivePermissions_t;
    
    
    class FERRISEXP_API GDriveFile
        :
        public QObject,
        public Handlable
    {
        Q_OBJECT;
        typedef GDriveFile _Self;

        fh_StreamToQIODevice m_streamToQIO;
        QNetworkReply* m_streamToQIO_reply;
        void OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m );

        QNetworkRequest addAuth( QNetworkRequest req );
        static std::string DRIVE_BASE();
        
        fh_GDriveClient getDrive() 
        {
            return m_gd;
        }
        
      public:

        fh_GDriveClient m_gd;
        string m_id;
        string m_etag;
        string m_rdn;
        string m_mime;
        string m_title;
        string m_desc;
        string m_earl;
        string m_earlview;
        string m_ext;
        string m_md5;
        long long m_sz;
        time_t m_ctime;
        time_t m_mtime;
        time_t m_mmtime;

        time_t parseTime( const std::string& v );
        QNetworkRequest createRequest( const std::string& earl );

        
      public:
        
        GDriveFile( fh_GDriveClient gd = 0, QVariantMap dm = QVariantMap() );

        fh_istream getIStream();
        fh_iostream getIOStream();
        bool isDir() const;

        void updateMetadata( const std::string& key, const std::string& value );
        void updateMetadata( stringmap_t& update );
        QNetworkReply* wait(QNetworkReply* reply );

        fh_GDriveFile createFile( const std::string& title );

        GDrivePermissions_t readPermissions();
        void sharesAdd( stringlist_t& emails );
        void sharesAdd( std::string email );

    };
    
    
    class FERRISEXP_API GDriveClient
        :
        public QObject,
        public Handlable
    {
        Q_OBJECT;
        friend class GDriveFile;
        
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

        GDriveClient();
        
      public:
        
        static fh_GDriveClient getGDriveClient();

        bool haveAPIKey() const;
        bool isAuthenticated() const;
        std::string requestToken( stringmap_t args = stringmap_t() );
        void accessToken( const std::string& code, stringmap_t args = stringmap_t() );
        void ensureAccessTokenFresh( int force = false );

        QNetworkReply* wait(QNetworkReply* reply );
        QNetworkReply* post( QNetworkRequest req );
        QNetworkReply* callMeth( QNetworkRequest req, stringmap_t args );
        QNetworkReply* callPost( QNetworkRequest req, stringmap_t args, const std::string& body );

        files_t filesList( const std::string& q = "", const std::string& pageToken = "" );
        QNetworkRequest addAuth( QNetworkRequest req );

                                                      
      public slots:
        void handleFinished();
        
    };
    

    
};

#endif
