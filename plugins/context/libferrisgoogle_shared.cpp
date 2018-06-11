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

#include "libferrisgoogle_shared.hh"
#include <Ferris/Configuration_private.hh>

#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QBuffer>

#include <Ferris/FerrisQt_private.hh>
#include <Ferris/FerrisKDE.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/Iterator.hh>
#include <Ferris/FerrisKDE.hh>

#include <qjson/parser.h>


#define DEBUG LG_GOOGLE_D

namespace Ferris
{
    using namespace XML;
    using namespace std;
    static const string DBNAME = FDB_SECURE;

    string prettyprintxml( const std::string& s )
    {
        try
        {
            fh_domdoc dom = Factory::StringToDOM( s );
            fh_stringstream ss = tostream( dom );
            return ss.str();
        }
        catch( exception& e )
        {
            return s;
        }
    }
    
    FERRISEXP_EXPORT userpass_t getGoogleUserPass(
        const std::string& server )
    {
        string user;
        string pass;

        string Key = ""; // server;
        
        {
            stringstream ss;
            ss << "google" << Key << "-username";
            user = getConfigString( DBNAME, tostr(ss), "" );
        }
        
        {
            stringstream ss;
            ss << "google" << Key << "-password";
            pass = getConfigString( DBNAME, tostr(ss), "" );
        }

        return make_pair( user, pass );
    }
    
    FERRISEXP_EXPORT void setGoogleUserPass(
        const std::string& server,
        const std::string& user, const std::string& pass )
    {
        string Key = ""; // server;

        {
            stringstream ss;
            ss << "google" << Key << "-username";
            setConfigString( DBNAME, tostr(ss), user );
        }

        {
            stringstream ss;
            ss << "google" << Key << "-password";
            setConfigString( DBNAME, tostr(ss), pass );
        }        
    }

    FERRISEXP_EXPORT std::string
    columnNumberToName( int col )
    {
        stringstream ss;
        int aval = 'a';
        --aval;
        ss << (char)(aval + col);
        string v = ss.str();
        return v;
    }
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    
    // GoogleClientResponseWaiter::GoogleClientResponseWaiter()
    // {
    //     m_loop = g_main_loop_new( 0, 0 );
    // }
    
    
    // void
    // GoogleClientResponseWaiter::block()
    // {
    //     LG_GOOGLE_D << "GoogleClientResponseWaiter::block(top)" << endl;
    //     g_main_loop_run( m_loop );
    //     LG_GOOGLE_D << "GoogleClientResponseWaiter::block(done)" << endl;
    // }
    
    // void
    // GoogleClientResponseWaiter::unblock()
    // {
    //     g_main_loop_quit( m_loop );
    // }
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    
    GoogleClient::GoogleClient()
        :
        m_qmanager( 0 )
    {
    }


    QNetworkAccessManager*
    GoogleClient::getQManager()
    {
        return ::Ferris::getQManager();
        
        // return new QNetworkAccessManager(0);
            
        // if( !m_qmanager )
        // {
        //     m_qmanager = new QNetworkAccessManager(0);
        // }
        // return m_qmanager;
    }

    void
    GoogleClient::addAuth( QNetworkRequest& r, const std::string& service )
    {
        if( m_authTokens[ service ].empty() )
            Authenticate_ClientLogin( service );

        DEBUG << "Auth token service:" << service << " token:" << m_authTokens[service] << endl;
        r.setRawHeader("Authorization", m_authTokens[service].c_str() );
    }
    
    
    
    void
    GoogleClient::dumpReplyToConsole(QNetworkReply* reply )
    {
        m_waiter.unblock( reply );
        
        cerr << "GoogleClient::dumpReplyToConsole()" << endl;
        cerr << "error:" << reply->error() << endl;
        cerr << "earl:" << tostr(reply->url().toString()) << endl;
        QByteArray ba = reply->readAll();
        cerr << "ba.sz:" << ba.size() << endl;
        cerr << "ba:" << (string)ba << endl;
        cerr << "-------------" << endl;
    }

    void
    GoogleClient::dumpReplyToConsole()
    {
        QNetworkReply* reply = (QNetworkReply*)sender();
        m_waiter.unblock( reply );
        
        cerr << "GoogleClient::dumpReplyToConsole()" << endl;
        cerr << "error:" << reply->error() << endl;
        cerr << "earl:" << tostr(reply->url().toString()) << endl;
        QByteArray ba = reply->readAll();
        cerr << "ba.sz:" << ba.size() << endl;
        cerr << "ba:" << (string)ba << endl;
        cerr << "-------------" << endl;
    }

    std::string responseToAuthToken( const std::string& data )
    {
        string ret;
        
        stringstream ss;
        ss << data;
        string s;
        while( getline(ss,s) )
        {
//            LG_GOOGLE_D << "s:" << s << endl;
            if( starts_with( s, "Auth=" ))
            {
                ret = "GoogleLogin " + s;
                ret = Util::replace_all( ret, "Auth=", "auth=" );
            }
        }
        return ret;
    }
    
    void
    GoogleClient::replyFinished(QNetworkReply* reply )
    {
        m_waiter.unblock( reply );

        LG_GOOGLE_D << "-------------" << endl;
        LG_GOOGLE_D << "GoogleClient::replyFinished(3)" << endl;

//        m_authToken = responseToAuthToken( (string)reply->readAll() );
        
        
//         stringstream ss;
//         ss << reply->readAll().data();
//         string s;
//         while( getline(ss,s) )
//         {
// //            LG_GOOGLE_D << "s:" << s << endl;
//             if( starts_with( s, "Auth=" ))
//             {
//                 m_authToken = "GoogleLogin " + s;
//             }
//         }

//        listSheets();
    }
    
    void
    GoogleClient::handleFinished()
    {
        QNetworkReply* r = dynamic_cast<QNetworkReply*>(sender());
        m_waiter.unblock(r);
        LG_GOOGLE_D << "GoogleClient::handleFinished()" << endl;
    }

    void
    GoogleClient::handleFinished( QNetworkReply* r )
    {
        m_waiter.unblock(r);
        LG_GOOGLE_D << "GoogleClient::handleFinished(QNR)" << endl;
    }
    
    void
    GoogleClient::localtest()
    {
        QNetworkAccessManager* qm = getQManager();
        
        QNetworkRequest request;
        request.setUrl(QUrl("http://alkid"));
        request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");

        cerr << "getting reply..." << endl;

        connect( qm, SIGNAL(finished(QNetworkReply*)), SLOT(dumpReplyToConsole(QNetworkReply*)));
        QNetworkReply *reply = qm->get(request);
        QUrl postdata("https://www.google.com/accounts/ClientLogin");
        postdata.addQueryItem("accountType", "HOSTED_OR_GOOGLE");

//        connect( reply, SIGNAL( finished() ), SLOT( dumpReplyToConsole() ) );
        m_waiter.block(reply);
    }
    
    
    void
    GoogleClient::Authenticate_ClientLogin( const std::string& service )
    {
        userpass_t up = getGoogleUserPass();
        std::string username = up.first;
        std::string password = up.second;
        
        QNetworkAccessManager* qm = getQManager();
        QNetworkRequest request;

        QUrl postdata("https://www.google.com/accounts/ClientLogin");
        postdata.addQueryItem("accountType", "HOSTED_OR_GOOGLE");
        postdata.addQueryItem("Email",       username.c_str() );
        postdata.addQueryItem("Passwd",      password.c_str() );
        postdata.addQueryItem("service",     service.c_str() );
        postdata.addQueryItem("source",      "libferris" FERRIS_VERSION);
        request.setUrl(postdata);
        connect( qm, SIGNAL(finished(QNetworkReply*)), SLOT(handleFinished(QNetworkReply*)));

        LG_GOOGLE_D << "postdata.toEncoded():" << tostr(QString(postdata.toEncoded())) << endl;
        LG_GOOGLE_D << "getting reply...2" << endl;
        
        QByteArray empty;
        QNetworkReply *reply = qm->post( request, empty );
        m_waiter.block(reply);

        string token = responseToAuthToken( (string)reply->readAll() );
        LG_GOOGLE_D << "service:" << service << " token:" << token << endl;
        m_authTokens[service] = token;
    }


    QNetworkReply*
    GoogleClient::get( QNetworkRequest req )
    {
        QNetworkAccessManager* qm = getQManager();
        QNetworkReply* reply = qm->get( req );
        connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
        m_waiter.block(reply);
        return reply;
    }

    QNetworkReply*
    GoogleClient::put( QNetworkRequest req, QByteArray& ba )
    {
        QNetworkAccessManager* qm = getQManager();
        QNetworkReply* reply = qm->put( req, ba );
        connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
        m_waiter.block(reply);
        return reply;
    }
    
    
    QNetworkReply*
    GoogleClient::put( QNetworkRequest req, const std::string& data )
    {
        QByteArray ba( data.data(), data.size() );
        return put( req, ba );
    }

    QNetworkReply*
    GoogleClient::post( QNetworkRequest req, const std::string& data )
    {
        QNetworkAccessManager* qm = getQManager();
        QNetworkReply* reply = qm->post( req, data.c_str() );
        connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
        m_waiter.block(reply);
        return reply;
    }
    
    QNetworkRequest
    GoogleClient::createRequest( QUrl& u, const std::string& service, const std::string& gversion )
    {
        QNetworkRequest req;
        addAuth( req, service );
        if( !gversion.empty() )
            req.setRawHeader("GData-Version", gversion.c_str() );
        req.setUrl(u);
        return req;
    }
    

    std::string
    GoogleClient::getYoutubeDevKey()
    {
        return getStrSubCtx( "~/.ferris/youtube-dev-key.txt", "" );
    }
    



    FERRISEXP_API std::list< DOMElement* >
    getAllChildrenElementsWithAttribute( DOMNode* node,
                                         const std::string& name,
                                         const std::string& attr,
                                         const std::string& value,
                                         bool recurse = true );
    FERRISEXP_API DOMElement*
    getFirstChildElementsWithAttribute( DOMNode* node,
                                        const std::string& name,
                                        const std::string& attrname,
                                        const std::string& value,
                                        bool recurse = true );


    FERRISEXP_API std::list< DOMElement* >
    getAllChildrenElementsWithAttribute( DOMNode* node,
                                         const std::string& name,
                                         const std::string& attrname,
                                         const std::string& value,
                                         bool recurse )
    {
        typedef std::list< DOMElement* > LIST;
        
        LIST l = getAllChildrenElements( node,
                                                             name,
                                                             recurse );

        LIST ret;
        for( LIST::iterator li = l.begin(); li != l.end(); ++li )
        {
            DOMElement* e = *li;
            std::string s = getAttribute( e, attrname );
//            cerr << "value:" << value << " s:" << s << endl;
            if( s == value )
            {
//                cerr << "match!" << endl;
                ret.push_back( e );
            }
        }
//        cerr << "ret.sz:" << ret.size() << endl;
        return ret;
    }
    
    FERRISEXP_API DOMElement*
    getFirstChildElementsWithAttribute( DOMNode* node,
                                        const std::string& name,
                                        const std::string& attrname,
                                        const std::string& value,
                                        bool recurse )
    {
        DOMElement* ret = 0;
        std::list< DOMElement* > l = getAllChildrenElementsWithAttribute( node, name, attrname, value, recurse );
        
        if( !l.empty() )
            ret = l.front();
        return ret;
    }
    


    std::string getMatchingAttribute( DOMNode* node,
                                      const std::string& name,
                                      const std::string& attrname,
                                      const std::string& value,
                                      const std::string& desiredAttribute )
    {
        DOMElement* e = getFirstChildElementsWithAttribute( node, name, attrname, value );
        if( e )
        {
            return getAttribute( e, desiredAttribute );
        }
        return "";
    }
    
                                      
    typedef std::list< DOMElement* > entries_t;
                                      

    GoogleSpreadSheets_t
    GoogleClient::listSheets()
    {
        LG_GOOGLE_D << "running listSheets...." << endl;

        QUrl u("http://spreadsheets.google.com/feeds/spreadsheets/private/full");
        QNetworkRequest request = createRequest( u );
        QNetworkReply *reply = get( request );

        LG_GOOGLE_D << "after running listSheets...." << endl;
        QByteArray ba = reply->readAll();
        LG_GOOGLE_D << "spreadsheets error:" << reply->error() << endl;
        LG_GOOGLE_D << "spreadsheets reply:" << (string)ba << endl;

        fh_domdoc dom = Factory::StringToDOM( (string)ba );
        entries_t entries = XML::getAllChildrenElements( dom->getDocumentElement(), "entry", false );

        GoogleSpreadSheets_t ret;
        for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
        {
            fh_GoogleSpreadSheet t = new GoogleSpreadSheet( this, dom, *ei );
            ret.push_back(t);
        }
        return ret;
    }

    fh_GoogleDocumentFolder
    GoogleClient::getRootFolder()
    {
        DEBUG << "GoogleClient::getRootFolder...." << endl;
        fh_GoogleDocumentFolder ret = new GoogleDocumentFolder( this, "" );
        return ret;
    }
    
    
    GoogleDocuments_t
    GoogleClient::listDocuments()
    {
        LG_GOOGLE_D << "running listDocuments...." << endl;

        QUrl u("http://docs.google.com/feeds/documents/private/full");
        QNetworkRequest request = createRequest( u, "writely" );
        request.setRawHeader("GData-Version", " 2.0");
        QNetworkReply *reply = get( request );
        
        LG_GOOGLE_D << "after running listDocuments...." << endl;
        QByteArray ba = reply->readAll();
        LG_GOOGLE_D << "docs error:" << reply->error() << endl;
        LG_GOOGLE_D << "docs reply:" << (string)ba << endl;

        fh_domdoc dom = Factory::StringToDOM( (string)ba );
        m_documentsETag = getAttribute( dom->getDocumentElement(), "gd:etag" );
        entries_t entries = XML::getAllChildrenElements( dom->getDocumentElement(), "entry", false );

        if( LG_GOOGLE_D_ACTIVE )
        {
            fh_stringstream ss = tostream( dom );
            LG_GOOGLE_D << "Documents:" << ss.str() << endl;
            LG_GOOGLE_D << "m_documentsETag:" << m_documentsETag << endl;
        }
       
        GoogleDocuments_t ret;
        for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
        {
            fh_GoogleDocument t = new GoogleDocument( this, dom, *ei );
            ret.push_back(t);
        }

        //
        // Have to get all the folder names to get the top level ones :(
        // GET /feeds/documents/private/full/-/folder?showfolders=true
        //
        {
            request.setUrl( QUrl( "http://docs.google.com/feeds/documents/private/full/-/folder?showfolders=true" ));
            QNetworkReply *reply = get( request );
            QByteArray ba = reply->readAll();
            LG_GOOGLE_D << "docs folders error:" << reply->error() << endl;
            LG_GOOGLE_D << "docs folders reply:" << (string)ba << endl;
            fh_domdoc dom = Factory::StringToDOM( (string)ba );
            entries_t entries = XML::getAllChildrenElements( dom->getDocumentElement(), "entry", false );
            for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
            {
                fh_GoogleDocument t = new GoogleDocument( this, dom, *ei );
                ret.push_back(t);
            }
        }
        

        
        // // GET /feeds/documents/private/full/-/folder?showfolders=true
        // {
        //     QUrl u("http://docs.google.com/feeds/documents/private/full/-/folder?showfolders=true");
        //     QNetworkRequest request = createRequest( u, "writely" );
        //     request.setRawHeader("GData-Version", " 2.0");
        //     QNetworkReply *reply = get( request );
        //     LG_GOOGLE_D << "after running listDocuments...." << endl;
        //     QByteArray ba = reply->readAll();
        //     LG_GOOGLE_D << "docs error:" << reply->error() << endl;
        //     fh_domdoc dom = Factory::StringToDOM( (string)ba );
        //     fh_stringstream ss = tostream( dom );
        //     LG_GOOGLE_D << "Folders:" << ss.str() << endl;
        // }
        
        // GET http://docs.google.com/feeds/folders/private/full/folder%3Afolder_id
        {
            LG_GOOGLE_D << "------------------------------------------------------------------------" << endl;
//            QUrl u("http://docs.google.com/feeds/folders/private/full/folder%3A1d90eb20-cad4-4bf3-b4d7-0ff8ddf3448d");
            QUrl u("http://docs.google.com/feeds/folders/private/full/folder%3Abe4824a5-8b8f-4129-9b33-5c5f3c1cdb57?showfolders=true");
            QNetworkRequest request = createRequest( u, "writely" );
            request.setRawHeader("GData-Version", " 2.0");
            QNetworkReply*  reply = get( request );
            QByteArray ba = reply->readAll();
            LG_GOOGLE_D << "x error:" << reply->error() << endl;
            LG_GOOGLE_D << "x reply:" << (string)ba << endl;
            fh_domdoc dom = Factory::StringToDOM( (string)ba );
            fh_stringstream ss = tostream( dom );
            LG_GOOGLE_D << "DATA:" << ss.str() << endl;
        }
        
        return ret;
    }
    
    fh_YoutubeUpload
    GoogleClient::createYoutubeUpload()
    {
        return new YoutubeUpload( this );
    }
    
    
    
    /****************************************/
    /****************************************/
    /****************************************/

    // This is for the root directory...
    GoogleDocumentFolder::GoogleDocumentFolder( fh_GoogleClient gc, const std::string& folderID )
        :
        m_gc( gc ),
        m_haveRead( false ),
        m_folderID( folderID )
    {
    }

    GoogleDocumentFolder::GoogleDocumentFolder( fh_GoogleClient gc, fh_domdoc dom, DOMElement* e )
        :
        m_gc( gc ),
        m_haveRead( false )
    {
        m_title   = getStrSubCtx( e, "title" );
        m_editURL = getMatchingAttribute( e, "link",
                                          "rel", "edit",
                                          "href" );
        
        m_folderID = getStrSubCtx( e, "id" );
        m_folderID = m_folderID.substr( m_folderID.length() - strlen("fac1125b-6dc7-4e79-8ef2-7dc1c8b4c9b8"));
        // m_folderID = Util::replace_all( m_folderID,
        //                                 "http://docs.google.com/feeds/documents/private/full/folder%3A",
        //                                 "" );
    }
    

    // <category label="spreadsheet" scheme="http://schemas.google.com/g/2005#kind" term="http://schemas.google.com/docs/2007#spreadsheet"/>

    void
    GoogleDocumentFolder::addItem( fh_domdoc dom, DOMElement* e )
    {
        string kind = getMatchingAttribute( e, "category",
                                            "scheme", "http://schemas.google.com/g/2005#kind",
                                            "label" );
        DEBUG << "kind:" << kind << endl;

        if( kind == "folder" )
        {
            fh_GoogleDocumentFolder f = new GoogleDocumentFolder( m_gc, dom, e );
            m_folders.push_back(f);
        }
        else
        {
            fh_GoogleDocument d = new GoogleDocument( m_gc, dom, e );
            m_docs.push_back(d);
        }
        
        
    }
    
    void
    GoogleDocumentFolder::read( bool force )
    {
        DEBUG << "read() title:" << m_title << " folderID:" << m_folderID << endl;
        GoogleDocumentFolders_t ret;

        if( m_haveRead )
            return;
        m_haveRead = true;
        
        //
        // Have to get all the folder names to get the top level ones :(
        // GET /feeds/documents/private/full/-/folder?showfolders=true
        //
        if( m_folderID.empty() )
        {
            QUrl u("http://docs.google.com/feeds/documents/private/full?showfolders=true");
            QNetworkRequest request = m_gc->createRequest( u, "writely" );
            request.setRawHeader("GData-Version", " 2.0");
            QNetworkReply *reply = m_gc->get( request );
            QByteArray ba = reply->readAll();
            LG_GOOGLE_D << "docs folders error:" << reply->error() << endl;
            LG_GOOGLE_D << "docs folders reply:" << (string)ba << endl;
            if( reply->error() == 0 )
            {
                fh_domdoc dom = Factory::StringToDOM( (string)ba );
                entries_t entries = XML::getAllChildrenElements( dom->getDocumentElement(), "entry", false );
                for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
                {
                    DOMElement* e = *ei;
                    string p = getMatchingAttribute( e, "link",
                                                     "rel", "http://schemas.google.com/docs/2007#parent",
                                                     "href" );
                    if( p.empty() )
                    {
                        addItem( dom, *ei );
                    }
                }

                if( LG_GOOGLE_D_ACTIVE )
                {
                    fh_stringstream ss = tostream( dom );
                    LG_GOOGLE_D << "TOP FOLDER....:" << ss.str() << endl;
                }
            }
        }
        else
        {
            stringstream uss;
            uss << "http://docs.google.com/feeds/folders/private/full/folder%3A" << m_folderID << "?showfolders=true";
            QUrl u(uss.str().c_str());
            QNetworkRequest request = m_gc->createRequest( u, "writely" );
            request.setRawHeader("GData-Version", " 2.0");
            QNetworkReply*  reply = m_gc->get( request );
            QByteArray ba = reply->readAll();
            LG_GOOGLE_D << "x error:" << reply->error() << endl;
            LG_GOOGLE_D << "x reply:" << (string)ba << endl;
            if( reply->error() == 0 )
            {
                fh_domdoc dom = Factory::StringToDOM( (string)ba );
                fh_stringstream ss = tostream( dom );
                LG_GOOGLE_D << "DATA:" << ss.str() << endl;
                entries_t entries = XML::getAllChildrenElements( dom->getDocumentElement(), "entry", false );
                for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
                {
                    addItem( dom, *ei );
                }
            }
        }
    }

    GoogleDocumentFolders_t
    GoogleDocumentFolder::getSubFolders()
    {
        read();
        return m_folders;
    }
    
    GoogleDocuments_t
    GoogleDocumentFolder::getDocuments()
    {
        read();
        return m_docs;
    }
    
    
    std::string
    GoogleDocumentFolder::getEditURL()
    {
        return m_editURL;
    }
    
    std::string
    GoogleDocumentFolder::getTitle()
    {
        return m_title;
    }

    fh_GoogleDocument
    GoogleDocumentFolder::createDocument( const std::string& data,
                                          const std::string& slug,
                                          const std::string& format )
    {
        fh_GoogleDocument ret = new GoogleDocument( m_gc, data, slug, format );
        return ret;
    }
    
    
    /****************************************/
    /****************************************/
    /****************************************/
    
    GoogleDocument::GoogleDocument( fh_GoogleClient gc, fh_domdoc dom, DOMElement* e )
        :
        m_gc( gc ),
        m_haveRead( false )
    {
        m_title   = getStrSubCtx( e, "title" );
        m_editURL = getMatchingAttribute( e, "link",
                                          "rel", "edit",
                                          "href" );

        m_docID = getStrSubCtx( e, "gd:resourceId" );
        m_docID = Util::replace_all( m_docID, "document:", "" );
    }

    GoogleDocument::GoogleDocument( fh_GoogleClient gc,
                                    const std::string& data,
                                    const std::string& slug,
                                    const std::string& format )
        :
        m_gc(gc),
        m_haveRead( false )
    {
        QNetworkRequest request = prepareCreateOrReplace( data, slug, format, true );
        QNetworkReply* reply = m_gc->post( request, data.c_str() );
        QByteArray ba = reply->readAll();
        LG_GOOGLE_D << "create doc error code:" << reply->error() << endl;
        LG_GOOGLE_D << "create doc reply:" << (string)ba << endl;
    }
    
    
    
    std::string
    GoogleDocument::getEditURL()
    {
        return m_editURL;
    }
    
    std::string
    GoogleDocument::getTitle()
    {
        return m_title;
    }

    
    fh_istream
    GoogleDocument::exportToFormat( const std::string& format )
    {
        DEBUG << "exportToFormat() m_docID:" << m_docID << endl;

        QUrl u( "http://docs.google.com/feeds/download/documents/Export" );
        u.addQueryItem("docID",        m_docID.c_str() );
        u.addQueryItem("exportFormat", format.c_str() );
        LG_GOOGLE_D << "export args:" << tostr(QString(u.toEncoded())) << endl;

        QNetworkRequest request = m_gc->createRequest( u, "writely", "" );
        QNetworkReply *reply = m_gc->get( request );
        QByteArray ba = reply->readAll();
        LG_GOOGLE_D << "export error:" << reply->error() << endl;
        LG_GOOGLE_D << "export reply:" << (string)ba << endl;

        fh_stringstream ret;
        ret.write( ba.data(), ba.size() );
        ret->clear();
        ret->seekg(0, ios::beg);
        ret->seekp(0, ios::beg);
        ret->clear();
        
        return ret;
    }


    
    
    
    string filenameToContentType( const std::string& slug_const,
                                  const std::string& format,
                                  const std::string& data = "" )
    {
        string slug = tolowerstr()( slug_const );

        // 299, update reply:Content-Type application/vnd.oasis.opendocument.spreadsheet is not a valid input type.

        string ret = "text/plain";
        if( format == "xx" || ends_with( slug, "xx" ) )
            ret = "application/vnd.ms-excel";
        else if( format == "xls" || ends_with( slug, "xls" ) )
            ret = "application/vnd.ms-excel";
        else if( format == "doc" || ends_with( slug, "doc" ) )
            ret = "application/msword";
        else if( format == "odt" || ends_with( slug, "odt" ) )
            ret = "application/vnd.oasis.opendocument.text";
        else if( format == "ods" || ends_with( slug, "ods" ) )
            ret = "application/vnd.oasis.opendocument.spreadsheet";
        else if( ends_with( slug, "png" ) )
            ret = "image/png";
        else if( ends_with( slug, "pdf" ) )
            ret = "application/pdf";
        else
        {
            string mt = regex_match_single( data, ".*mimetypeapplication/([^P]+)PK.*" );
            if( !mt.empty() )
                ret = "application/" + mt;
        }
        
        return ret;
    }
    
    QNetworkRequest
    GoogleDocument::prepareCreateOrReplace( const std::string& data,
                                            const std::string& slug,
                                            const std::string& format,
                                            bool create )
    {
        string ctype = filenameToContentType( slug, format, data );

        stringstream uss;
        if( create )
            uss << "http://docs.google.com/feeds/documents/private/full";
        else
        {
            if( contains( m_docID, "spreadshee" ))
                uss << "http://docs.google.com/feeds/media/private/full/" << m_docID;
            else
                uss << "http://docs.google.com/feeds/media/private/full/document:" << m_docID;
        }
        QUrl u( uss.str().c_str() );
        QNetworkRequest request = m_gc->createRequest( u, "writely", "" );
        request.setHeader( QNetworkRequest::ContentTypeHeader, ctype.c_str() );
        request.setHeader( QNetworkRequest::ContentLengthHeader, tostr(data.length()).c_str() );
        request.setRawHeader("Content-Type", ctype.c_str() );
//        request.setRawHeader("Content-Length", tostr(data.length()).c_str() );
        request.setRawHeader("Slug", slug.c_str() );
        if( !create )
            request.setRawHeader("If-Match", "*" );

        DEBUG << "import url:" << tostr(QString(u.toEncoded())) << endl;
        DEBUG << "slug:" << slug << endl;
        DEBUG << "ctype:" << ctype << endl;
        DEBUG << "format:" << format << endl;
        return request;
    }
    

    void
    GoogleDocument::importFromFormat( fh_istream iss, const std::string& format )
    {
        DEBUG << "importFromFormat() m_title:" << m_title << endl;
        string data = StreamToString(iss);
        QNetworkRequest request = prepareCreateOrReplace( data, m_title, format, false );
        DEBUG << "importFromFormat() Calling put..." << endl;
        QNetworkReply* reply = m_gc->put( request, data );
        QByteArray ba = reply->readAll();
        LG_GOOGLE_D << "update error code:" << reply->error() << endl;
        LG_GOOGLE_D << "update reply:" << (string)ba << endl;
    }
    
        
    

    
    /****************************************/
    /****************************************/
    /****************************************/
    
    GoogleSpreadSheet::GoogleSpreadSheet( fh_GoogleClient gc, fh_domdoc dom, DOMElement* e )
        :
        m_gc( gc ),
        m_haveRead( false )
    {
        m_title = getStrSubCtx( e, "title" );

        m_feedURL = getMatchingAttribute( e, "content",
                                          "type", "application/atom+xml;type=feed",
                                          "src" );

        // non versioned data header
        // m_feedURL = getMatchingAttribute( e, "link",
        //                                   "rel", "http://schemas.google.com/spreadsheets/2006#worksheetsfeed",
        //                                   "href" );
        

    }
    
    std::string
    GoogleSpreadSheet::getFeedURL()
    {
        return m_feedURL;
    }
    
    std::string
    GoogleSpreadSheet::getTitle()
    {
        return m_title;
    }

    
    GoogleWorkSheets_t
    GoogleSpreadSheet::listSheets()
    {
        LG_GOOGLE_D << "running list WORK Sheets...." << endl;

        if( m_haveRead )
            return m_sheets;
        
        QUrl u( getFeedURL().c_str() );
        QNetworkRequest request = m_gc->createRequest( u );
        QNetworkReply*  reply   = m_gc->get( request );

        LG_GOOGLE_D << "after running list WORK Sheets...." << endl;
        QByteArray ba = reply->readAll();
        LG_GOOGLE_D << "worksheets error:" << reply->error() << endl;
        LG_GOOGLE_D << "worksheets reply:" << (string)ba << endl;
        fh_domdoc dom = Factory::StringToDOM( (string)ba );
        entries_t entries = XML::getAllChildrenElements( dom->getDocumentElement(), "entry", false );

        for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
        {
            fh_GoogleWorkSheet t = new GoogleWorkSheet( m_gc, dom, *ei );
            m_sheets.push_back(t);
        }
        
        return m_sheets;
    }

    // void
    // GoogleSpreadSheet::addDocsAPIAuth_reply(QNetworkReply* reply )
    // {
    //     m_gc->m_waiter.unblock(reply);
    // }
    
    // void
    // GoogleSpreadSheet::addDocsAPIAuth( QNetworkRequest& r )
    // {
    //     if( m_docsAPIAuth.empty() )
    //     {
       
    //         QNetworkAccessManager* qm = m_gc->getQManager();
    //         QNetworkRequest request;

    //         userpass_t up = getGoogleUserPass();
    //         string username = up.first;
    //         string password = up.second;
            
    //         QUrl postdata("https://www.google.com/accounts/ClientLogin");
    //         postdata.addQueryItem("accountType", "HOSTED_OR_GOOGLE");
    //         postdata.addQueryItem("Email",       username.c_str() );
    //         postdata.addQueryItem("Passwd",      password.c_str() );
    //         postdata.addQueryItem("service",     "writely");
    //         postdata.addQueryItem("source",      "libferris" FERRIS_VERSION);
    //         request.setUrl(postdata);
    //         connect( qm, SIGNAL(finished(QNetworkReply*)), SLOT(addDocsAPIAuth_reply(QNetworkReply*)));

    //         LG_GOOGLE_D << "GoogleSpreadSheet::addDocsAPIAuth():" << tostr(QString(postdata.toEncoded())) << endl;
    //         LG_GOOGLE_D << "getting reply...2" << endl;
            
    //         QByteArray empty;
    //         QNetworkReply *reply = qm->post( request, empty );
    //         LG_GOOGLE_D << "calling block()" << endl;
    //         m_gc->m_waiter.block(reply);
    //         LG_GOOGLE_D << "after block()" << endl;
    //         m_docsAPIAuth = responseToAuthToken( (string)reply->readAll() );
    //         LG_GOOGLE_D << "m_docsAPIAuth:" << m_docsAPIAuth << endl;
    //     }
    //     r.setRawHeader("Authorization", m_docsAPIAuth.c_str() );
    // }
    
    

    string
    GoogleSpreadSheet::getDocIDFromTitle( const std::string& title )
    {
//        cerr << "FIXME GoogleSpreadSheet::getDocIDFromTitle() " << endl;
//        return "tz96EupEQKYKTbu3m0GpqTw";


//        QUrl u( "http://docs.google.com/feeds/documents/private/full/-/spreadsheet" );
//        QNetworkRequest request = m_gc->createRequest( u );
//        request.setRawHeader("GData-Version", " 2.0");


        QUrl u("http://docs.google.com/feeds/documents/private/full");
        
        QNetworkRequest request;
//        m_gc->addAuth( request, "wise" );
        m_gc->addAuth( request, "writely" );
        request.setRawHeader("GData-Version", " 2.0");
        request.setUrl(u);

        
        LG_GOOGLE_D << "about to issue google docs request..." << endl;
        QNetworkReply *reply = m_gc->get( request );
        LG_GOOGLE_D << "done with google docs request..." << endl;
        QByteArray ba = reply->readAll();
        LG_GOOGLE_D << "getDocIDFromTitle error:" << reply->error() << endl;
        LG_GOOGLE_D << "getDocIDFromTitle reply:" << (string)ba << endl;
        string ret;

        fh_domdoc dom = Factory::StringToDOM( (string)ba );
        string etag   = getAttribute( dom->getDocumentElement(), "gd:etag" );
        entries_t entries = XML::getAllChildrenElements( dom->getDocumentElement(), "entry", false );
        for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
        {
            DOMElement* e = *ei;
            string t = getStrSubCtx( e, "title" );
            if( t == title )
            {
                ret = getStrSubCtx( e, "id" );
                break;
            }
        }
        LG_GOOGLE_D << "docid ret:" << ret << endl;
        return ret;
    }


    fh_GoogleDocument
    GoogleSpreadSheet::getDocument()
    {
        // getDocIDFromTitle
        // ( fh_GoogleClient gc, fh_domdoc dom, DOMElement* e );
        stringstream uss;
        uss << "http://docs.google.com/feeds/documents/private/full?"
            << "title-exact=true&title=" << getTitle();
        QUrl u( uss.str().c_str() );
        DEBUG << "GoogleSpreadSheet::getDocument() earl:" << uss.str() << endl;
        QNetworkRequest request = m_gc->createRequest( u, "writely" );
        request.setRawHeader("GData-Version", " 2.0");
        QNetworkReply *reply = m_gc->get( request );

        QByteArray ba = reply->readAll();
        LG_GOOGLE_D << "docs error:" << reply->error() << endl;
        LG_GOOGLE_D << "docs reply:" << (string)ba << endl;

        fh_domdoc dom = Factory::StringToDOM( (string)ba );
        entries_t entries = XML::getAllChildrenElements( dom->getDocumentElement(), "entry", false );

        if( LG_GOOGLE_D_ACTIVE )
        {
            fh_stringstream ss = tostream( dom );
            LG_GOOGLE_D << "Documents:" << ss.str() << endl;
        }
       
        for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
        {
            fh_GoogleDocument t = new GoogleDocument( m_gc, dom, *ei );
            return t;
        }

        return 0;
    }
    
    

    void
    GoogleSpreadSheet::importFromFormat( fh_istream iss, const std::string& format )
    {
        LG_GOOGLE_D << "GoogleSpreadSheet::importFromFormat() format:" << format << endl;
        string docid = getDocIDFromTitle( m_title );
        LG_GOOGLE_D << "getDocIDFromTitle() title:" << m_title << " docid:" << docid << endl;

        fh_GoogleDocument doc = getDocument();
        doc->importFromFormat( iss, format );
    }
    
    
    fh_istream
    GoogleSpreadSheet::exportToFormat( const std::string& format )
    {
        LG_GOOGLE_D << "GoogleSpreadSheet::exportToFormat() format:" << format << endl;

        string docid = getDocIDFromTitle( m_title );
        LG_GOOGLE_D << "getDocIDFromTitle() title:" << m_title << " docid:" << docid << endl;

        QUrl u( "http://spreadsheets.google.com/feeds/download/spreadsheets/Export" );
        u.addQueryItem("key", docid.c_str() );
        u.addQueryItem("exportFormat", format.c_str() );
        LG_GOOGLE_D << "exportToFormat, args:" << tostr(QString(u.toEncoded())) << endl;

        QNetworkRequest request = m_gc->createRequest( u );
        QNetworkReply *reply = m_gc->get( request );
        QByteArray ba = reply->readAll();
        
        LG_GOOGLE_D << "exportToFormat error:" << reply->error() << endl;
        if( !reply->error() )
            LG_GOOGLE_D << "exportToFormat reply.len:" << ba.size() << endl;
        else
            LG_GOOGLE_D << "exportToFormat reply:" << (string)ba << endl;

        {
            QVariant qv = reply->header( QNetworkRequest::ContentTypeHeader );
            DEBUG << "Got ctype:" << tostr(qv.toString()) << endl;
            // application/vnd.oasis.opendocument.spreadsheet; charset=UTF-8
        }
        
        fh_stringstream ret;
        ret.write( ba.data(), ba.size() );
        return ret;
    }
    

    fh_GoogleWorkSheet
    GoogleSpreadSheet::createWorkSheet( const std::string& name )
    {
        LG_GOOGLE_D << "createWorkSheet() top" << endl;
        
        stringstream ss;
        ss << "<entry xmlns=\"http://www.w3.org/2005/Atom\"" << endl
           << "  xmlns:gs=\"http://schemas.google.com/spreadsheets/2006\">" << endl
           << "     <title>" << name << "</title>" << endl
           << "     <gs:rowCount>100</gs:rowCount>" << endl
           << "     <gs:colCount>100</gs:colCount>" << endl
           << "</entry>" << endl;
        
        QUrl u( getFeedURL().c_str() );
        QNetworkRequest request = m_gc->createRequest( u );
        request.setHeader( QNetworkRequest::ContentTypeHeader, "application/atom+xml" );
        QNetworkReply *reply = m_gc->post( request, ss.str() );

        
        QByteArray ba = reply->readAll();
        LG_GOOGLE_D << "createWorkSheet() reply:" << (string)ba << endl;
        fh_domdoc dom = Factory::StringToDOM( (string)ba );
        DOMElement* e = dom->getDocumentElement();
        fh_GoogleWorkSheet t = new GoogleWorkSheet( m_gc, dom, e );
        m_sheets.push_back(t);
        return t;
    }
    
    


    /****************************************/
    /****************************************/
    /****************************************/

    GoogleWorkSheet::GoogleWorkSheet( fh_GoogleClient gc, fh_domdoc dom, DOMElement* e )
        :
        m_gc( gc ),
        m_cellsFetched( false ),
        m_delayCellSync( false )
    {
        m_etag = getAttribute( e, "gd:etag" );
        m_title = getStrSubCtx( e, "title" );
        m_cellFeedURL = getMatchingAttribute( e, "link",
                                              "rel", "http://schemas.google.com/spreadsheets/2006#cellsfeed",
                                              "href" );
        m_editURL = getMatchingAttribute( e, "link",
                                          "rel", "edit",
                                          "href" );

        if( LG_GOOGLE_D_ACTIVE )
        {
            fh_stringstream ss = tostream( *e );
            LG_GOOGLE_D << "WorkSheet:" << ss.str() << endl;
        }
        
    }
    
    std::string
    GoogleWorkSheet::getCellFeedURL()
    {
        return m_cellFeedURL;
    }
    
    std::string
    GoogleWorkSheet::getTitle()
    {
        return m_title;
    }
    

    void
    GoogleWorkSheet::fetchCells()
    {
        
        QUrl u( getCellFeedURL().c_str() );
        QNetworkRequest request = m_gc->createRequest( u );
        // If-None-Match: W/"D08FQn8-eil7ImA9WxZbFEw."
        if( !m_cellFeedETag.empty() )
            request.setRawHeader("If-None-Match", m_cellFeedETag.c_str() );
        QNetworkReply*    reply = m_gc->get( request );

        QByteArray ba = reply->readAll();
//        cerr << "CELL DATA:" << (string)ba << endl;
        fh_domdoc dom = Factory::StringToDOM( (string)ba );
        m_cellFeedETag = getAttribute( dom->getDocumentElement(), "gd:etag" );
        entries_t entries = XML::getAllChildrenElements( dom->getDocumentElement(), "entry", false );
        int cellURLLength = getCellFeedURL().length();

        int header_rownum = 1;
        int max_headerrow_colnum = 0;
        for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
        {
            DOMElement* e = *ei;

            fh_GoogleWorkSheetCell cell = new GoogleWorkSheetCell( m_gc, dom, *ei, this, cellURLLength );
            LG_GOOGLE_D << "have cell at row:" << cell->row() << " col:" << cell->col() << endl;

            m_cells_t::iterator ci = m_cells.find( make_pair( cell->row(), cell->col() ));
            if( ci != m_cells.end() )
            {
                fh_GoogleWorkSheetCell cell = ci->second;
                cell->update( dom, e, cellURLLength );
            }
            else
            {
                m_cells[ make_pair( cell->row(), cell->col() ) ] = cell;
            }

            if( cell->row() == header_rownum )
            {
                max_headerrow_colnum = max( max_headerrow_colnum, cell->col() );
            }
        }

        m_colnames.clear();
        LG_GOOGLE_D << "max_headerrow_colnum:" << max_headerrow_colnum << endl;
        for( int col=1; col <= max_headerrow_colnum; ++col )
        {
            m_cells_t::iterator ci = m_cells.find( make_pair( header_rownum, col ));
            if( ci != m_cells.end() )
            {
                string v = ci->second->value();
                m_colnames[ v ] = col;
            }

            string v = columnNumberToName( col );
            m_colnames[ v ] = col;
            LG_GOOGLE_D << " setting col:" << col << " to name:" << v << endl;
        }
        for( int col=1; col < 26; ++col )
        {
            string v = columnNumberToName( col );
            m_colnames[ v ] = col;
        }
        
        
        
        m_cellsFetched = true;
    }

    std::list< std::string >
    GoogleWorkSheet::getColumnNames()
    {
        ensureCellsFetched();
        
        std::list< std::string > ret;
        copy( map_domain_iterator( m_colnames.begin() ),
              map_domain_iterator( m_colnames.end() ),
              back_inserter( ret ) );
        return ret;
    }
    
    
    void
    GoogleWorkSheet::ensureCellsFetched()
    {
        if( !m_cellsFetched )
            fetchCells();
    }
    
    fh_GoogleWorkSheetCell
    GoogleWorkSheet::getCell( int row, int col )
    {
        ensureCellsFetched();

        m_cells_t::iterator ci = m_cells.find( make_pair( row, col ));
        if( ci == m_cells.end() )
        {
            LG_GOOGLE_D << "Creating new cell at row:" << row << " col:" << col << endl;
            fh_GoogleWorkSheetCell cell = new GoogleWorkSheetCell( m_gc, this, row, col, "" );
            m_cells[ make_pair( cell->row(), cell->col() ) ] = cell;
            return cell;
        }
        return ci->second;
    }

    fh_GoogleWorkSheetCell
    GoogleWorkSheet::getCell( int row, const std::string& colName )
    {
        ensureCellsFetched();

        int c = m_colnames[ colName ];
        LG_GOOGLE_D << "m_colnames.sz:" << m_colnames.size() << " colName:" << colName << " has number:" << c << endl;
        for( m_colnames_t::iterator ci = m_colnames.begin(); ci != m_colnames.end(); ++ci )
        {
            LG_GOOGLE_D << "ci.first:" << ci->first << " sec:" << ci->second << endl;
        }
        return getCell( row, c );
    }
    
    

    int
    GoogleWorkSheet::getLargestRowNumber()
    {
        ensureCellsFetched();

        int ret = 0;
        for( m_cells_t::iterator ci = m_cells.begin(); ci != m_cells.end(); ++ci )
        {
            fh_GoogleWorkSheetCell c = ci->second;
            ret = max( ret, c->row() );
        }
        return ret;
    }
    

    bool
    GoogleWorkSheet::getDelayCellSync()
    {
        return m_delayCellSync;
    }
    
    void
    GoogleWorkSheet::setDelayCellSync( bool v )
    {
        m_delayCellSync = v;
    }
    

    void
    GoogleWorkSheet::sync()
    {
        if( !getDelayCellSync() )
            return;

        string baseURL = getCellFeedURL();
        string fullEditURL = baseURL + "/batch";
        stringstream updatess;
        updatess << "<feed xmlns=\"http://www.w3.org/2005/Atom\"" << endl
                 << "   xmlns:batch=\"http://schemas.google.com/gdata/batch\" " << endl
                 << "   xmlns:gs=\"http://schemas.google.com/spreadsheets/2006\" " << endl
                 << " >" << endl
                 << endl
                 << "<id>" << getCellFeedURL() << "</id> " << endl
                 << endl;
        
        for( m_cells_t::iterator ci = m_cells.begin(); ci != m_cells.end(); ++ci )
        {
            fh_GoogleWorkSheetCell c = ci->second;
            // if( c->isCreated() )
            // {
            //     c->sync();
            // }
            // else
            {
                c->writeUpdateBlock( updatess );
            }
        }

        updatess << "</feed>" << endl;

        QUrl u(fullEditURL.c_str());
        QNetworkRequest request = m_gc->createRequest( u );
        request.setHeader( QNetworkRequest::ContentTypeHeader, "application/atom+xml" );
        if( !m_etag.empty() )
            request.setRawHeader("If-Match", "*" );
        QNetworkReply*    reply = m_gc->post( request, updatess.str() );

        
        cerr << "-------------" << endl;
        cerr << "m_etag:" << m_etag << endl;
        cerr << "m_cellFeedETag:" << m_cellFeedETag << endl;
        cerr << "DST URL:" << fullEditURL << endl;
        cerr << "SENT2" << endl;
        cerr << updatess.str() << endl;
        cerr << "error:" << reply->error() << endl;
        
        QByteArray ba = reply->readAll();
        cerr << "ba.sz:" << ba.size() << endl;
        cerr << "ba:" << prettyprintxml( (string)ba ) << endl;
        cerr << "-------------" << endl;

        fh_domdoc dom = Factory::StringToDOM( (string)ba );
        entries_t entries = XML::getAllChildrenElements( dom->getDocumentElement(), "atom:entry", false );

        int cellURLLength = getCellFeedURL().length();
        GoogleSpreadSheets_t ret;
        for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
        {
            DOMElement* e = *ei;
            cerr << "have entry..." << endl;
            DOMElement* x = XML::getChildElement( e, "gs:cell" );
            if( x )
            {
                cerr << "have entry/x..." << endl;
                int row = toint( getAttribute( x, "row" ) );
                int col = toint( getAttribute( x, "col" ) );
                cerr << "row:" << row << " col:" << col << endl;
                if( fh_GoogleWorkSheetCell cell = m_cells[ make_pair( row, col ) ] )
                    cell->update( dom, e, cellURLLength );
            }
        }

        //
        // now, update any formula cells
        // this shouldn't need to be explicit with ETags and feeds
        //
        fetchCells();
                
        // for( m_cells_t::iterator ci = m_cells.begin(); ci != m_cells.end(); ++ci )
        // {
        //     fh_GoogleWorkSheetCell c = ci->second;
        //     if( c->isFormula() )
        //     {
        //     }
        // }
        
    }
    
    

    /****************************************/
    /****************************************/
    /****************************************/

    GoogleWorkSheetCell::GoogleWorkSheetCell( fh_GoogleClient gc,
                                              fh_domdoc dom,
                                              DOMElement* e,
                                              fh_GoogleWorkSheet ws,
                                              int cellURLLength )
        :
        m_gc( gc ),
        m_ws( ws ),
        m_dirty( false ),
        m_created( false ),
        m_etag("")
    {
        update( dom, e, cellURLLength );
        
        // string editURL = getMatchingAttribute( e, "link",
        //                                        "rel", "edit",
        //                                        "href" );
            
        // DOMElement* x = XML::getChildElement( e, "gs:cell" );

        // m_value = getStrSubCtx( e, "gs:cell" );
        // m_row = toint( getAttribute( x, "row" ) );
        // m_col = toint( getAttribute( x, "col" ) );
        // m_cellURL = editURL.substr( cellURLLength );


        {
            fh_stringstream ss = tostream( *e );
            LG_GOOGLE_D << "CELL:" << ss.str() << endl;
        }
        
    }

    void
    GoogleWorkSheetCell::update( fh_domdoc dom, DOMElement* e, int cellURLLength )
    {
        string editURL = getMatchingAttribute( e, "link",
                                               "rel", "edit",
                                               "href" );
        if( editURL.empty() )
            editURL = getMatchingAttribute( e, "atom:link",
                                            "rel", "edit",
                                            "href" );
        m_etag = getAttribute( e, "gd:etag" );
        
        DOMElement* x = XML::getChildElement( e, "gs:cell" );

        m_value = getStrSubCtx( e, "gs:cell" );
        m_row = toint( getAttribute( x, "row" ) );
        m_col = toint( getAttribute( x, "col" ) );
        m_cellURL   = editURL.substr( cellURLLength );
        m_isFormula = starts_with( getAttribute( x, "inputValue" ), "=" );

        LG_GOOGLE_D << "editURL:" << editURL << endl;
        LG_GOOGLE_D << "etag:" << m_etag << endl;
        LG_GOOGLE_D << "m_value:" << m_value << endl;
        LG_GOOGLE_D << "m_isFormula:" << m_isFormula << endl;
    }
    
    

    GoogleWorkSheetCell::GoogleWorkSheetCell( fh_GoogleClient gc, fh_GoogleWorkSheet ws,
                                              int r, int c, const std::string& v )
        :
        m_gc( gc ),
        m_ws( ws ),
        m_created( true ),
        m_etag("")
    {
        m_row = r;
        m_col = c;
        m_value = v;
        {
            stringstream ss;
            ss << "/R" << r << "C" << c;
            m_cellURL = ss.str();
        }
    }
    
    bool
    GoogleWorkSheetCell::isCreated()
    {
        return m_created;
    }

    bool
    GoogleWorkSheetCell::isFormula()
    {
        return m_isFormula;
    }
    
        
    int
    GoogleWorkSheetCell::row()
    {
        return m_row;
    }

    
        
    int
    GoogleWorkSheetCell::col()
    {
        return m_col;
    }
    

    std::string
    GoogleWorkSheetCell::value()
    {
        LG_GOOGLE_D << "reading cell at row:" << m_row << " col:" << m_col << " result:" << m_value << endl;
        return m_value;
    }

    

    void
    GoogleWorkSheetCell::writeUpdateBlock( stringstream& ss )
    {
        if( !m_dirty )
            return;

        string fullEditURL = editURL();
        if( m_created )
            fullEditURL += "/latest";

        cerr << "m_etag:" << m_etag << endl;
        ss << "" << endl
           << "<entry "
           // << " xmlns:gd=\"http://schemas.google.com/g/2005\" "
           // << " gd:etag=\"" << Util::replace_all( m_etag, "\"", "&quot;" ) << "\" "
           << "  >" << endl
           << "   <batch:id>A" << toVoid(this) << "</batch:id> " << endl
           << "   <batch:operation type=\"update\"/> " << endl
           << "   <id>" << fullEditURL << "</id> " << endl
           << "   <link rel=\"edit\" type=\"application/atom+xml\" " << endl
           << "    href=\"" << fullEditURL << "\"/>" << endl
           << "  <gs:cell row=\"" << row() << "\" col=\"" << col() << "\" inputValue=\"" << m_value << "\"/>" << endl
           << "</entry> " << endl
           << endl;
    }

    void
    GoogleWorkSheetCell::sync()
    {
        string fullEditURL = editURL();

        if( m_created )
            fullEditURL += "/latest";

        cerr << "SENDING VALUE:" << m_value << endl;
        stringstream updatess;
        updatess << "" << endl
                 << "<entry xmlns=\"http://www.w3.org/2005/Atom\"" << endl
                 << "    xmlns:gs=\"http://schemas.google.com/spreadsheets/2006\">" << endl
                 << "  <id>" << "http://spreadsheets.google.com/feeds/cells/key/worksheetId/private/full/cellId" << "</id>"  << endl
                 << "  <link rel=\"edit\" type=\"application/atom+xml\""  << endl
                 << "    href=\"" << fullEditURL << "\"/>"  << endl
                 << "  <gs:cell row=\"" << row() << "\" col=\"" << col() << "\" inputValue=\"" << m_value << "\" />" << endl
                 << "</entry>" << endl
                 <<"" << endl;

        QUrl u(fullEditURL.c_str());
        QNetworkRequest request = m_gc->createRequest( u );
        request.setHeader( QNetworkRequest::ContentTypeHeader, "application/atom+xml" );
        QNetworkReply*    reply = m_gc->put( request, updatess.str() );

        cerr << "-------------" << endl;
        cerr << "SENT to:" << fullEditURL << endl;
        cerr << updatess.str() << endl;
        cerr << "error:" << reply->error() << endl;
        
        QByteArray ba = reply->readAll();
        cerr << "ba.sz:" << ba.size() << endl;
        cerr << "ba:" << prettyprintxml( (string)ba ) << endl;
        cerr << "-------------" << endl;
        
        fh_domdoc dom = Factory::StringToDOM( (string)ba );
        DOMElement* e = dom->getDocumentElement();
        string editURL = getMatchingAttribute( e, "link",
                                               "rel", "edit",
                                               "href" );
        cerr << "have updated editURL:" << editURL << endl;
        int cellURLLength = m_ws->getCellFeedURL().length();
        m_value   = getStrSubCtx( e, "gs:cell" );
        m_cellURL = editURL.substr( cellURLLength );
    }
    
    void
    GoogleWorkSheetCell::value( const std::string& s )
    {
        if( m_ws->getDelayCellSync() )
        {
            m_dirty = true;
            m_value = s;
            return;
        }

        m_value = s;
        sync();
    }
    

    std::string
    GoogleWorkSheetCell::editURL()
    {
        return m_ws->getCellFeedURL() + m_cellURL;
    }
    

    /****************************************/
    /****************************************/
    /****************************************/

    YoutubeUpload::YoutubeUpload( fh_GoogleClient gc )
        :
        m_gc( gc )
    {
    }

    YoutubeUpload::~YoutubeUpload()
    {
//        cerr << "~YoutubeUpload()" << endl;
//        BackTrace();
    }
    
    
    void
    YoutubeUpload::streamingUploadComplete()
    {
        cerr << "YoutubeUpload::streamingUploadComplete() m_streamToQIO:" << GetImpl(m_streamToQIO)
             << " blocking for reply" << endl;
        DEBUG << "YoutubeUpload::streamingUploadComplete() blocking for reply" << endl;
        QNetworkReply* reply = m_reply;
        cerr << "YoutubeUpload::streamingUploadComplete() m_streamToQIO:" << GetImpl(m_streamToQIO) << " wc1" << endl;
        
        m_streamToQIO->writingComplete();
        cerr << "YoutubeUpload::streamingUploadComplete() m_streamToQIO:" << GetImpl(m_streamToQIO) << " wc2" << endl;
        DEBUG << "YoutubeUpload::streamingUploadComplete() reading data..." << endl;
        QByteArray ba = m_streamToQIO->readResponse();
//        QByteArray ba = reply->readAll();
        cerr << "YoutubeUpload::streamingUploadComplete() m_streamToQIO:" << GetImpl(m_streamToQIO)
             << " reply:" << reply->error()  << endl;
        
        DEBUG << "streamingUploadComplete() reply.err:" << reply->error() << endl;
        DEBUG << "streamingUploadComplete() got reply:" << tostr(ba) << endl;

        fh_domdoc dom = Factory::StringToDOM( tostr(ba) );
        DOMElement* e = dom->getDocumentElement();
        m_url = getMatchingAttribute( e, "link",
                                      "rel", "self",
                                      "href" );
        m_id = XML::getChildText( firstChild( e, "yt:videoid" ));

//         <?xml version="1.0" encoding="UTF-8"?>
// <entry xmlns="http://www.w3.org/2005/Atom" xmlns:app="http://www.w3.org/2007/app" xmlns:media="http://search.yahoo.com/mrss/" xmlns:gd="http://schemas.google.com/g/2005" xmlns:yt="http://gdata.youtube.com/schemas/2007" gd:etag="W/&quot;C0YMQXoycCp7ImA9WxNTFEU.&quot;">
//   <id>tag:youtube.com,2008:video:jKY6tYD5s0U</id>
//   <published>2009-08-16T20:53:00.498-07:00</published>
//   <updated>2009-08-16T20:53:00.498-07:00</updated>
//   <app:edited>2009-08-16T20:53:00.498-07:00</app:edited>
//   <app:control>
//     <app:draft>yes</app:draft>
//     <yt:state name="processing"/>
//   </app:control>
//   <category scheme="http://schemas.google.com/g/2005#kind" term="http://gdata.youtube.com/schemas/2007#video"/>
//   <category scheme="http://gdata.youtube.com/schemas/2007/categories.cat" term="People" label="People &amp; Blogs"/>
//   <category scheme="http://gdata.youtube.com/schemas/2007/keywords.cat" term="nothing"/>
//   <title>nothing</title>
//   <link rel="alternate" type="text/html" href="http://www.youtube.com/watch?v=jKY6tYD5s0U"/>
//   <link rel="http://gdata.youtube.com/schemas/2007#video.responses" type="application/atom+xml" href="http://gdata.youtube.com/feeds/api/videos/jKY6tYD5s0U/responses"/>
//   <link rel="http://gdata.youtube.com/schemas/2007#video.ratings" type="application/atom+xml" href="http://gdata.youtube.com/feeds/api/videos/jKY6tYD5s0U/ratings"/>
//   <link rel="http://gdata.youtube.com/schemas/2007#video.complaints" type="application/atom+xml" href="http://gdata.youtube.com/feeds/api/videos/jKY6tYD5s0U/complaints"/>
//   <link rel="http://gdata.youtube.com/schemas/2007#video.related" type="application/atom+xml" href="http://gdata.youtube.com/feeds/api/videos/jKY6tYD5s0U/related"/>
//   <link rel="http://gdata.youtube.com/schemas/2007#video.captionTracks" type="application/atom+xml" href="http://gdata.youtube.com/feeds/api/videos/jKY6tYD5s0U/captions" yt:hasEntries="false"/>
//   <link rel="http://gdata.youtube.com/schemas/2007#insight.views" type="text/html" href="http://insight.youtube.com/video-analytics/csvreports?query=jKY6tYD5s0U&amp;type=v&amp;starttime=1249862400000&amp;endtime=1250467200000&amp;region=world&amp;token=yyT3Wb4oxV4D5VKuz1sLnroReKR8MTI1MDQ4Mjk4MA%3D%3D&amp;hl=en_US"/>
//   <link rel="self" type="application/atom+xml" href="http://gdata.youtube.com/feeds/api/users/monkeyiqtesting/uploads/jKY6tYD5s0U"/>
//   <link rel="edit" type="application/atom+xml" href="http://gdata.youtube.com/feeds/api/users/monkeyiqtesting/uploads/jKY6tYD5s0U"/>
//   <author>
//     <name>monkeyiqtesting</name>
//     <uri>http://gdata.youtube.com/feeds/api/users/monkeyiqtesting</uri>
//   </author>
//   <gd:comments>
//     <gd:feedLink href="http://gdata.youtube.com/feeds/api/videos/jKY6tYD5s0U/comments" countHint="0"/>
//   </gd:comments>
//   <media:group>
//     <media:category label="People &amp; Blogs" scheme="http://gdata.youtube.com/schemas/2007/categories.cat">People</media:category>
//     <media:content url="http://www.youtube.com/v/jKY6tYD5s0U&amp;f=user_uploads&amp;d=DeKCSMvhFol1x0mvu9wlZWD9LlbsOl3qUImVMV6ramM&amp;app=youtube_gdata" type="application/x-shockwave-flash" medium="video" isDefault="true" expression="full" yt:format="5"/>
//     <media:credit role="uploader" scheme="urn:youtube">monkeyiqtesting</media:credit>
//     <media:description type="plain">nothing</media:description>
//     <media:keywords>nothing</media:keywords>
//     <media:player url="http://www.youtube.com/watch?v=jKY6tYD5s0U"/>
//     <media:title type="plain">nothing</media:title>
//     <yt:uploaded>2009-08-16T20:53:00.498-07:00</yt:uploaded>
//     <yt:videoid>jKY6tYD5s0U</yt:videoid>
//   </media:group>
// </entry>
              
    }
    
    fh_iostream
    YoutubeUpload::createStreamingUpload( const std::string& ContentType )
    {
        m_url = "";
        m_id  = "";
        
// NO X-GData-Client: <client_id>
// OK X-GData-Key: key=<developer_key>
// OK Slug: <video_filename>
// OK Authorization: AuthSub token="<authentication_token>"
// OK GData-Version: 2
// OK Content-Length: <content_length>
        
        string devkey = (string)"key=" + m_gc->getYoutubeDevKey();

        QUrl u( "http://uploads.gdata.youtube.com/feeds/api/users/default/uploads");
//        u.addQueryItem("Slug", m_uploadFilename.c_str());
//        u.addQueryItem("X-GData-Key", devkey.c_str());

        QNetworkRequest req = m_gc->createRequest( u, "youtube", "2" );
        req.setRawHeader("Slug", m_uploadFilename.c_str());
        req.setRawHeader("X-GData-Key", devkey.c_str());
        DEBUG << "X-GData-Key START|" <<  devkey << "|END" << endl;
        req.setHeader(QNetworkRequest::ContentLengthHeader, tostr(m_uploadSize).c_str() );
        
        m_streamToQIO = Factory::createStreamToQIODevice();
        {
            string desc = m_desc;
            string title = m_title;
            string keywords = m_keywords;
            if( keywords.empty() )
                keywords = "none";
            
            stringstream ss;
            ss << "Content-Type: application/atom+xml; charset=UTF-8\n";
            ss << "\n";
            ss << "<?xml version=\"1.0\"?>";
            ss << "<entry xmlns=\"http://www.w3.org/2005/Atom\"\n";
            ss << "  xmlns:media=\"http://search.yahoo.com/mrss/\"\n";
            ss << "  xmlns:yt=\"http://gdata.youtube.com/schemas/2007\">\n";
            ss << "  <media:group>\n";
            // if( m_uploadDefaultsToPrivate )
            //     ss << "   <yt:private/>\n";
            ss << "    <media:title type=\"plain\">" << title << "</media:title>\n";
            ss << "    <media:description type=\"plain\">\n";
            ss << "      " << desc << "\n";
            ss << "    </media:description>\n";
            ss << "    <media:category\n";
            ss << "      scheme=\"http://gdata.youtube.com/schemas/2007/categories.cat\">People\n";
            ss << "    </media:category>\n";
            ss << "    <media:keywords>" << keywords << "</media:keywords>\n";
            ss << "  </media:group>\n";
            ss << "</entry>\n";

            string API_XML_request;
            API_XML_request = ss.str();
            m_streamToQIO->addExtraDataChunk( API_XML_request );
        }
        
        stringstream blobss;
        blobss << "Content-Type: " << ContentType << "\n"
               << "Content-Transfer-Encoding: binary;";
        m_streamToQIO->setContentType( "multipart/related" );
        QNetworkReply* reply = m_streamToQIO->post( ::Ferris::getQNonCachingManager(),
                                                    req, blobss.str() );
        m_reply = reply;
        fh_iostream ret = m_streamToQIO->getStream();
        return ret;
    }
    
    
    
    /****************************************/
    /****************************************/
    /****************************************/
    
    namespace Factory
    {
        fh_GoogleClient createGoogleClient()
        {
            Main::processAllPendingEvents();
            KDE::ensureKDEApplication();
            return new GoogleClient();
        }
    };

    /******************************************************/
    /******************************************************/
    /******************************************************/


    GDriveFile::GDriveFile( fh_GDriveClient gd, QVariantMap dm )
        : m_gd( gd )
        , m_id(tostr(dm["id"]))
        , m_etag(tostr(dm["etag"]))
        , m_rdn(tostr(dm["originalFilename"]))
        , m_mime(tostr(dm["mimeType"]))
        , m_title(tostr(dm["title"]))
        , m_desc(tostr(dm["description"]))
        , m_earl(tostr(dm["downloadUrl"]))
        , m_earlview(tostr(dm["webViewLink"]))
        , m_ext(tostr(dm["fileExtension"]))
        , m_md5(tostr(dm["md5checksum"]))
        , m_sz(toint(tostr(dm["fileSize"])))
    {
        m_ctime  = parseTime(tostr(dm["createdDate"]));
        m_mtime  = parseTime(tostr(dm["modifiedDate"]));
        m_mmtime = parseTime(tostr(dm["modifiedByMeDate"]));
    }

    time_t
    GDriveFile::parseTime( const std::string& v_const )
    {
        string v = v_const;
        try
        {
            // 2013-07-26T03:51:28.238Z
            v = replaceg( v, "\\.[0-9]*Z", "Z" );
            return Time::toTime(Time::ParseTimeString( v ));
        }
        catch( ... )
        {          
            return 0;
        }
    }

    QNetworkRequest
    GDriveFile::createRequest( const std::string& earl )
    {
        QNetworkRequest req( QUrl(earl.c_str()) );
        req.setRawHeader("Authorization", string(string("Bearer ") + m_gd->m_accessToken).c_str() );
        return req;
    }
    
    fh_istream
    GDriveFile::getIStream()
    {
        m_gd->ensureAccessTokenFresh();

        QNetworkAccessManager* qm = getQManager();
        QNetworkRequest req = createRequest(m_earl);

        DEBUG << "getIStream()...url:" << tostr(QString(req.url().toEncoded())) << endl;
        QNetworkReply *reply = qm->get( req );
        fh_istream ret = Factory::createIStreamFromQIODevice( reply );
        return ret;
    }

    void
    GDriveFile::OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
    {
        DEBUG << "GDriveFile::OnStreamClosed(top)" << endl;
        
        m_streamToQIO->writingComplete();
        QByteArray ba = m_streamToQIO->readResponse();
        DEBUG << "RESULT:" << tostr(ba) << endl;
    }
    
    
    fh_iostream
    GDriveFile::getIOStream()
    {
        DEBUG << "GDriveFile::getIOStream(top)" << endl;
        // PUT https://www.googleapis.com/upload/drive/v2/files/fileId
        stringstream earlss;
        earlss << "https://www.googleapis.com/upload/drive/v2/files/" << m_id << "?"
               << "uploadType=media&"
               << "fileId=" << m_id;
        QNetworkRequest req = createRequest(tostr(earlss));
        req.setRawHeader( "Accept", "*/*" );
        req.setRawHeader( "Connection", "" );            
        req.setRawHeader( "Accept-Encoding", "" );
        req.setRawHeader( "Accept-Language", "" );
        req.setRawHeader( "User-Agent", "" );

        DEBUG << "PUT()ing main request..." << endl;
        m_streamToQIO = Factory::createStreamToQIODevice();
        QNetworkReply* reply = m_streamToQIO->put( ::Ferris::getQNonCachingManager(), req );
            
        m_streamToQIO_reply = reply;

        DEBUG << "preparing iostream for user..." << endl;
        fh_iostream ret = m_streamToQIO->getStream();
        ferris_ios::openmode m = 0;
        ret->getCloseSig().connect( sigc::bind( sigc::mem_fun(*this, &_Self::OnStreamClosed ), m )); 
        DEBUG << "GDriveFile::getIOStream(end)" << endl;
        return ret;
    }

    bool
    GDriveFile::isDir() const
    {
        return m_mime == "application/vnd.google-apps.folder";
    }

    string
    GDriveFile::DRIVE_BASE()
    {
        return "https://www.googleapis.com/drive/v2/";
    }

    QNetworkRequest
    GDriveFile::addAuth( QNetworkRequest req )
    {
        return m_gd->addAuth( req );
    }
    
    QNetworkReply*
    GDriveFile::wait(QNetworkReply* reply )
    {
        return getDrive()->wait( reply );
    }
    
    void
    GDriveFile::updateMetadata( const std::string& key, const std::string& value )
    {
        stringmap_t update;
        update[ key ] = value;
        updateMetadata( update );
    }

    fh_GDriveFile
    GDriveFile::createFile( const std::string& title )
    {
        QNetworkRequest req = createRequest(DRIVE_BASE() + "files");
        req.setRawHeader("Content-Type", "application/json" );
        stringmap_t update;
        update["fileId"] = "";
        update["title"] = title;
        update["description"] = "new";
        update["data"] = "";
        update["mimeType"] = KDE::guessMimeType( title );
        string body = stringmapToJSON( update );
        DEBUG << "createFile()  url:" << tostr( req.url().toEncoded() ) << endl;
        DEBUG << "createFile() body:" << body << endl;
        QNetworkReply* reply = getDrive()->callPost( req, stringmap_t(), body );
        wait( reply );
        QByteArray ba = reply->readAll();
        DEBUG << "REST error code:" << reply->error() << endl;
        DEBUG << "HTTP response  :" << httpResponse(reply) << endl;

        
        QVariantMap dm = JSONToQVMap( tostr(ba) );
        fh_GDriveFile f = new GDriveFile( getDrive(), dm );
        return f;
    }
    
    void
    GDriveFile::updateMetadata( stringmap_t& update )
    {
        getDrive()->ensureAccessTokenFresh();

        // PATCH https://www.googleapis.com/drive/v2/files/fileId
        
        QUrl u( string(DRIVE_BASE() + "files/" + m_id).c_str() );

        QNetworkRequest req;
        req.setUrl( u );
        DEBUG << "u1.str::" << tostr(req.url().toString()) << endl;
        req.setRawHeader("Content-Type", "application/json" );
        req = addAuth( req );
        DEBUG << "u2.str::" << tostr(req.url().toString()) << endl;
        std::string json = stringmapToJSON( update );
        DEBUG << " json:" << json << endl;
        QBuffer* buf = new QBuffer(new QByteArray(json.c_str()));
        QNetworkReply* reply = getQManager()->sendCustomRequest( req, "PATCH", buf );
        wait( reply );
        QByteArray ba = reply->readAll();
        DEBUG << "REST error code:" << reply->error() << endl;
        DEBUG << "HTTP response  :" << httpResponse(reply) << endl;
        
        DEBUG << "result:" << tostr(ba) << endl;
        stringmap_t sm = JSONToStringMap( tostr(ba) );
        fh_stringstream ess;
        for( stringmap_t::iterator iter = update.begin(); iter != update.end(); ++iter )
        {
            DEBUG << "iter->first:" << iter->first << endl;
            DEBUG << "iter->second:" << iter->second << endl;
            DEBUG << "         got:" << sm[iter->first] << endl;
            
            if( sm[iter->first] != iter->second )
            {
                ess << "attribute " << iter->first << " not correct." << endl;
                ess << "expected:" << iter->second << endl;
                ess << "     got:" << sm[iter->first] << endl;
            }
        }

        string e = tostr(ess);
        DEBUG << "e:" << e << endl;
        if( !e.empty() )
        {
            Throw_WebAPIException( e, 0 );
        }
    }
    
    GDrivePermissions_t
    GDriveFile::readPermissions()
    {
        GDrivePermissions_t ret;
        QNetworkRequest req = createRequest(DRIVE_BASE() + "files/" + m_id + "/permissions");
        req.setRawHeader("Content-Type", "application/json" );
        stringmap_t args;
        args["fileId"] = m_id;
        QNetworkReply* reply = getDrive()->callMeth( req, args );
        QByteArray ba = reply->readAll();
        DEBUG << "readPermissions() REST error code:" << reply->error() << endl;
        DEBUG << "readPermissions() HTTP response  :" << httpResponse(reply) << endl;
        DEBUG << "readPermissions() RESULT:" << tostr(ba) << endl;

        QVariantMap qm = JSONToQVMap( tostr(ba) );
        QVariantList l = qm["items"].toList();
        foreach (QVariant ding, l)
        {
            QVariantMap dm = ding.toMap();

            int perm = GDrivePermission::NONE;
            if( dm["role"] == "reader" )
                perm = GDrivePermission::READ;
            if( dm["role"] == "writer" || dm["role"] == "owner" )
                perm = GDrivePermission::WRITE;
            
            fh_GDrivePermission p = new GDrivePermission( perm, tostr(dm["name"]));
            ret.push_back(p);
        }
        
        
        return ret;
    }

    void
    GDriveFile::sharesAdd( std::string email )
    {
        stringlist_t emails;
        emails.push_back( email );
        return sharesAdd( emails );
    }
    
    void
    GDriveFile::sharesAdd( stringlist_t& emails )
    {
        // POST https://www.googleapis.com/drive/v2/files/fileId/permissions

        for( stringlist_t::iterator si = emails.begin(); si != emails.end(); ++si )
        {
            string email = *si;
            
            QNetworkRequest req = createRequest(DRIVE_BASE() + "files/" + m_id + "/permissions");
            req.setRawHeader("Content-Type", "application/json" );
            stringmap_t args;
            args["fileId"] = m_id;
            args["emailMessage"] = "Life moves pretty fast. If you don't stop and look around once in a while, you could miss it.";

            stringstream bodyss;
            bodyss << "{"                                       << endl
                   << " \"kind\": \"drive#permission\", "       << endl
                   << "   \"value\": \"" << email << "\" , " << endl
                   << "   \"role\": \"writer\","                << endl
                   << "   \"type\": \"user\""                   << endl
                   << " } "                                     << endl;

            DEBUG << "body:" << tostr(bodyss) << endl;
            QNetworkReply* reply = getDrive()->callPost( req, args, tostr(bodyss) );
            QByteArray ba = reply->readAll();
            DEBUG << "sharesAdd() REST error code:" << reply->error() << endl;
            DEBUG << "sharesAdd() HTTP response  :" << httpResponse(reply) << endl;
            DEBUG << "sharesAdd() RESULT:" << tostr(ba) << endl;

            stringmap_t sm = JSONToStringMap( tostr(ba) );
            if( !sm["etag"].empty() && !sm["id"].empty() )
                continue;

            // Failed
            stringstream ess;
            ess << "Failed to create permission for user:" << email << endl
                   << " reply:" << tostr(ba) << endl;
            Throw_WebAPIException( tostr(ess), 0 );
        }
    }
    
    

    /********************/
    
    
    GDriveClient::GDriveClient()
        : m_clientID( "881964254376.apps.googleusercontent.com" )
        , m_secret(   "UH9zxZ8k_Fj3actLPRVPVG8Q" )
    {
        readAuthTokens();        
    }

    void
    GDriveClient::readAuthTokens()
    {
        m_accessToken  = getConfigString( FDB_SECURE, "gdrive-access-token",  "" );
        m_refreshToken = getConfigString( FDB_SECURE, "gdrive-refresh-token", "" );
        m_accessToken_expiretime = toType<time_t>(
            getConfigString( FDB_SECURE,
                             "gdrive-access-token-expires-timet", "0"));
    }
    
    std::string
    GDriveClient::AUTH_BASE()
    {
        return "https://accounts.google.com/o/oauth2/";
    }
    
    
    fh_GDriveClient
    GDriveClient::getGDriveClient()
    {
        Main::processAllPendingEvents();
        KDE::ensureKDEApplication();
        static fh_GDriveClient ret = new GDriveClient();
        return ret;
    }

    bool
    GDriveClient::haveAPIKey() const
    {
        return !m_clientID.empty() && !m_secret.empty();
    }

    bool
    GDriveClient::isAuthenticated() const
    {
        return !m_accessToken.empty() && !m_refreshToken.empty();
    }
    
    
    void
    GDriveClient::handleFinished()
    {
        QNetworkReply* r = dynamic_cast<QNetworkReply*>(sender());
        m_waiter.unblock(r);
        DEBUG << "handleFinished() r:" << r << endl;
    }

    QNetworkReply*
    GDriveClient::wait(QNetworkReply* reply )
    {
        connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
        m_waiter.block(reply);
        return reply;
    }
    
    
    QNetworkReply*
    GDriveClient::post( QNetworkRequest req )
    {
        QUrl u = req.url();
        DEBUG << "u.str::" << tostr(u.toString()) << endl;
        string body = tostr(u.encodedQuery());
        DEBUG << "body:" << body << endl;
        u.setEncodedQuery(QByteArray());
//        u.setUrl( "http://localhost/testing" );
        req.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
        req.setHeader(QNetworkRequest::ContentLengthHeader, tostr(body.size()).c_str() );
        QNetworkReply* reply = getQManager()->post( req, body.c_str() );
        connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
        m_waiter.block(reply);
        DEBUG << "REST error code:" << reply->error() << endl;
        return reply;
    }

    std::string
    GDriveClient::requestToken( stringmap_t args )
    {
        QUrl u( string(AUTH_BASE() + "auth").c_str() );
        u.addQueryItem("response_type", "code" );
        u.addQueryItem("client_id",     m_clientID.c_str() );
        u.addQueryItem("redirect_uri",  "urn:ietf:wg:oauth:2.0:oob" );
        u.addQueryItem("scope",         "https://www.googleapis.com/auth/drive.file "
                       "https://www.googleapis.com/auth/drive "
                       "https://www.googleapis.com/auth/drive.scripts "
                       "https://www.googleapis.com/auth/drive.appdata " );
        u.addQueryItem("state",         "anyone... anyone?" );
        std::string authURL = tostr( u.toEncoded() );
        return authURL;
    }


    

    void
    GDriveClient::accessToken( const std::string& code, stringmap_t args )
    {
        QUrl u( string(AUTH_BASE() + "token").c_str() );

        QNetworkRequest req;
        req.setUrl( u );
        args["code"]          = code;
        args["client_id"]     = m_clientID;
        args["client_secret"] = m_secret;
        args["redirect_uri"]  = "urn:ietf:wg:oauth:2.0:oob";
        args["grant_type"]    = "authorization_code";
        DEBUG << "u1.str::" << tostr(req.url().toString()) << endl;
        req = setArgs( req, args );
        DEBUG << "u2.str::" << tostr(req.url().toString()) << endl;
        QNetworkReply* reply = post( req );
        QByteArray ba = reply->readAll();
        
        cerr << "result:" << tostr(ba) << endl;
        stringmap_t sm = JSONToStringMap( tostr(ba) );
        time_t expiretime = Time::getTime() + toint(sm["expires_in"]);
        setConfigString( FDB_SECURE, "gdrive-access-token",  sm["access_token"]  );
        setConfigString( FDB_SECURE, "gdrive-refresh-token", sm["refresh_token"] );
        setConfigString( FDB_SECURE, "gdrive-access-token-expires-timet", tostr(expiretime) );
        readAuthTokens();
    }

    void
    GDriveClient::ensureAccessTokenFresh( int force )
    {
        if( !isAuthenticated() )
            return;
        if( !force )
        {
            if( Time::getTime() + 30 < m_accessToken_expiretime )
                return;
        }
        
        DEBUG << "ensureAccessTokenFresh() really doing it!" << endl;
        QUrl u( string(AUTH_BASE() + "token").c_str() );
        QNetworkRequest req;
        req.setUrl( u );
        stringmap_t args;
        args["refresh_token"] = m_refreshToken;
        args["client_id"]     = m_clientID;
        args["client_secret"] = m_secret;
        args["grant_type"]    = "refresh_token";
        req = setArgs( req, args );
        QNetworkReply* reply = post( req );
        cerr << "ensureAccessTokenFresh() have reply..." << endl;
        QByteArray ba = reply->readAll();
        
        cerr << "ensureAccessTokenFresh(b) m_accessToken:" << m_accessToken << endl;
        cerr << "ensureAccessTokenFresh() result:" << tostr(ba) << endl;
        stringmap_t sm = JSONToStringMap( tostr(ba) );
        time_t expiretime = Time::getTime() + toint(sm["expires_in"]);
        setConfigString( FDB_SECURE, "gdrive-access-token",  sm["access_token"]  );
        setConfigString( FDB_SECURE, "gdrive-access-token-expires-timet", tostr(expiretime) );
        readAuthTokens();
        cerr << "ensureAccessTokenFresh(e) m_accessToken:" << m_accessToken << endl;
    }

    QNetworkRequest 
    GDriveClient::addAuth( QNetworkRequest req )
    {
        req.setRawHeader("Authorization", string(string("Bearer ") + m_accessToken).c_str() );
        return req;
    }
    

    QNetworkReply*
    GDriveClient::callMeth( QNetworkRequest req, stringmap_t args )
    {
        ensureAccessTokenFresh();

        req = setArgs( req, args );
        req = addAuth( req );
        QUrl u = req.url();
        DEBUG << "callMeth U:" << tostr(QString(u.toEncoded())) << endl;
//        u.setUrl( "http://localhost/testing" );
//        req.setUrl(u);
        
        QNetworkReply* reply = getQManager()->get( req );
        connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
        m_waiter.block(reply);
        DEBUG << "REST error code:" << reply->error() << endl;
        return reply;
    }

    QNetworkReply*
    GDriveClient::callPost( QNetworkRequest req, stringmap_t args, const std::string& body )
    {
        ensureAccessTokenFresh();

        req = setArgs( req, args );
        req = addAuth( req );
        QUrl u = req.url();
        DEBUG << "callPost U:" << tostr(QString(u.toEncoded())) << endl;
        DEBUG << "      body:" << body << endl;
        QBuffer* buf = new QBuffer(new QByteArray(body.c_str()));
        QNetworkReply* reply = getQManager()->post( req, buf );
        connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
        m_waiter.block(reply);
        DEBUG << "REST error code:" << reply->error() << endl;
        return reply;
    }
    
    string
    GDriveClient::DRIVE_BASE()
    {
        return "https://www.googleapis.com/drive/v2/";
    }
    QNetworkRequest
    GDriveClient::createRequest( const std::string& earlTail )
    {
        QUrl u( string(DRIVE_BASE() + earlTail).c_str() );
        QNetworkRequest req;
        req.setUrl( u );
        return req;
    }


    
    
    files_t
    GDriveClient::filesList( const std::string& q_const, const std::string& pageToken )
    {
        string q = q_const;
        
        if( q.empty() )
        {
            q = "hidden = false and trashed = false";
        }
        
        files_t ret;
//        QNetworkRequest req = createRequest("files/root/children");
        QNetworkRequest req = createRequest("files");
        stringmap_t args;
        args["maxResults"] = tostr(1000);
        if( !pageToken.empty() )
            args["pageToken"] = pageToken;
        if( !q.empty() )
            args["q"] = q;

        QNetworkReply* reply = callMeth( req, args );
        QByteArray ba = reply->readAll();
        DEBUG << "filesList() result:" << tostr(ba) << endl;
        stringmap_t sm = JSONToStringMap( tostr(ba) );

        DEBUG << "etag:" << sm["etag"] << endl;

        QVariantMap qm = JSONToQVMap( tostr(ba) );

        QVariantList l = qm["items"].toList();
        foreach (QVariant ding, l)
        {
            QVariantMap dm = ding.toMap();
            string rdn = tostr(dm["originalFilename"]);
            long sz = toint(tostr(dm["fileSize"]));
            QVariantMap labels = dm["labels"].toMap();

            if( labels["hidden"].toInt() || labels["trashed"].toInt() )
                continue;

            bool skipThisOne = false;
            QVariantList parents = dm["parents"].toList();
            foreach (QVariant pv, parents)
            {
                QVariantMap pm = pv.toMap();
                if( !pm["isRoot"].toInt() )
                {
                    skipThisOne = 1;
                    break;
                }
            }
            if( dm["userPermission"].toMap()["role"] != "owner" )
                skipThisOne = true;
            
            if( skipThisOne )
                continue;
            
            DEBUG << "sz:" << sz << "   fn:" << rdn << endl;
            fh_GDriveFile f = new GDriveFile( this, dm );
            ret.push_back(f);
        }
        
        return ret;
    }
    

    /****************************************/
    /****************************************/
    /****************************************/
    
    
    
};
