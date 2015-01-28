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

#include "libferrisboxcom_shared.hh"
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


#define DEBUG LG_BOXCOM_D

namespace Ferris
{
    using namespace XML;
    using namespace std;
    static const string DBNAME = FDB_SECURE;


    /******************************************************/
    /******************************************************/
    /******************************************************/


    BoxComFile::BoxComFile( fh_BoxComClient gd, QVariantMap dm )
        : m_gd( gd )
    //     , m_id(tostr(dm["id"]))
    //     , m_etag(tostr(dm["etag"]))
    //     , m_rdn(tostr(dm["originalFilename"]))
    //     , m_mime(tostr(dm["mimeType"]))
    //     , m_title(tostr(dm["title"]))
    //     , m_desc(tostr(dm["description"]))
    //     , m_earl(tostr(dm["downloadUrl"]))
    //     , m_earlview(tostr(dm["webViewLink"]))
    //     , m_ext(tostr(dm["fileExtension"]))
    //     , m_md5(tostr(dm["md5checksum"]))
    //     , m_sz(toint(tostr(dm["fileSize"])))
    {
    //     m_ctime  = parseTime(tostr(dm["createdDate"]));
    //     m_mtime  = parseTime(tostr(dm["modifiedDate"]));
    //     m_mmtime = parseTime(tostr(dm["modifiedByMeDate"]));
    }

    // time_t
    // BoxComFile::parseTime( const std::string& v_const )
    // {
    //     string v = v_const;
    //     try
    //     {
    //         // 2013-07-26T03:51:28.238Z
    //         v = replaceg( v, "\\.[0-9]*Z", "Z" );
    //         return Time::toTime(Time::ParseTimeString( v ));
    //     }
    //     catch( ... )
    //     {          
    //         return 0;
    //     }
    // }

    // QNetworkRequest
    // BoxComFile::createRequest( const std::string& earl )
    // {
    //     QNetworkRequest req( QUrl(earl.c_str()) );
    //     req.setRawHeader("Authorization", string(string("Bearer ") + m_gd->m_accessToken).c_str() );
    //     return req;
    // }
    
    // fh_istream
    // BoxComFile::getIStream()
    // {
    //     QNetworkAccessManager* qm = getQManager();
    //     QNetworkRequest req = createRequest(m_earl);

    //     DEBUG << "getIStream()...url:" << tostr(QString(req.url().toEncoded())) << endl;

    //     m_gd->ensureAccessTokenFresh();
    //     QNetworkReply *reply = qm->get( req );
    //     fh_istream ret = Factory::createIStreamFromQIODevice( reply );
    //     return ret;
    // }

    // void
    // BoxComFile::OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
    // {
    //     DEBUG << "BoxComFile::OnStreamClosed(top)" << endl;
        
    //     m_streamToQIO->writingComplete();
    //     QByteArray ba = m_streamToQIO->readResponse();
    //     DEBUG << "RESULT:" << tostr(ba) << endl;
    // }
    
    
    // fh_iostream
    // BoxComFile::getIOStream()
    // {
    //     DEBUG << "BoxComFile::getIOStream(top)" << endl;
    //     // PUT https://www.boxcomapis.com/upload/drive/v2/files/fileId
    //     stringstream earlss;
    //     earlss << "https://www.boxcomapis.com/upload/drive/v2/files/" << m_id << "?"
    //            << "uploadType=media&"
    //            << "fileId=" << m_id;
    //     QNetworkRequest req = createRequest(tostr(earlss));
    //     req.setRawHeader( "Accept", "*/*" );
    //     req.setRawHeader( "Connection", "" );            
    //     req.setRawHeader( "Accept-Encoding", "" );
    //     req.setRawHeader( "Accept-Language", "" );
    //     req.setRawHeader( "User-Agent", "" );

    //     DEBUG << "PUT()ing main request..." << endl;
    //     m_streamToQIO = Factory::createStreamToQIODevice();
    //     QNetworkReply* reply = m_streamToQIO->put( ::Ferris::getQNonCachingManager(), req );
            
    //     m_streamToQIO_reply = reply;

    //     DEBUG << "preparing iostream for user..." << endl;
    //     fh_iostream ret = m_streamToQIO->getStream();
    //     ferris_ios::openmode m = 0;
    //     ret->getCloseSig().connect( bind( sigc::mem_fun(*this, &_Self::OnStreamClosed ), m )); 
    //     DEBUG << "BoxComFile::getIOStream(end)" << endl;
    //     return ret;
    // }

    // bool
    // BoxComFile::isDir() const
    // {
    //     return m_mime == "application/vnd.boxcom-apps.folder";
    // }

    // string
    // BoxComFile::DRIVE_BASE()
    // {
    //     return "https://www.boxcomapis.com/drive/v2/";
    // }

    // QNetworkRequest
    // BoxComFile::addAuth( QNetworkRequest req )
    // {
    //     return m_gd->addAuth( req );
    // }
    
    // QNetworkReply*
    // BoxComFile::wait(QNetworkReply* reply )
    // {
    //     return getDrive()->wait( reply );
    // }
    
    // void
    // BoxComFile::updateMetadata( const std::string& key, const std::string& value )
    // {
    //     stringmap_t update;
    //     update[ key ] = value;
    //     updateMetadata( update );
    // }

    // fh_BoxComFile
    // BoxComFile::createFile( const std::string& title )
    // {
    //     QNetworkRequest req = createRequest(DRIVE_BASE() + "files");
    //     req.setRawHeader("Content-Type", "application/json" );
    //     stringmap_t update;
    //     update["fileId"] = "";
    //     update["title"] = title;
    //     update["description"] = "new";
    //     update["data"] = "";
    //     update["mimeType"] = KDE::guessMimeType( title );
    //     string body = stringmapToJSON( update );
    //     DEBUG << "createFile()  url:" << tostr( req.url().toEncoded() ) << endl;
    //     DEBUG << "createFile() body:" << body << endl;
    //     QNetworkReply* reply = getDrive()->callPost( req, stringmap_t(), body );
    //     wait( reply );
    //     QByteArray ba = reply->readAll();
    //     DEBUG << "REST error code:" << reply->error() << endl;
    //     DEBUG << "HTTP response  :" << httpResponse(reply) << endl;

        
    //     QVariantMap dm = JSONToQVMap( tostr(ba) );
    //     fh_BoxComFile f = new BoxComFile( getDrive(), dm );
    //     return f;
    // }
    
    // void
    // BoxComFile::updateMetadata( stringmap_t& update )
    // {
    //     // PATCH https://www.boxcomapis.com/drive/v2/files/fileId
        
    //     QUrl u( string(DRIVE_BASE() + "files/" + m_id).c_str() );

    //     QNetworkRequest req;
    //     req.setUrl( u );
    //     DEBUG << "u1.str::" << tostr(req.url().toString()) << endl;
    //     req.setRawHeader("Content-Type", "application/json" );
    //     req = addAuth( req );
    //     DEBUG << "u2.str::" << tostr(req.url().toString()) << endl;
    //     std::string json = stringmapToJSON( update );
    //     DEBUG << " json:" << json << endl;
    //     QBuffer* buf = new QBuffer(new QByteArray(json.c_str()));
    //     getDrive()->ensureAccessTokenFresh();
    //     QNetworkReply* reply = getQManager()->sendCustomRequest( req, "PATCH", buf );
    //     wait( reply );
    //     QByteArray ba = reply->readAll();
    //     DEBUG << "REST error code:" << reply->error() << endl;
    //     DEBUG << "HTTP response  :" << httpResponse(reply) << endl;
        
    //     DEBUG << "result:" << tostr(ba) << endl;
    //     stringmap_t sm = JSONToStringMap( tostr(ba) );
    //     fh_stringstream ess;
    //     for( stringmap_t::iterator iter = update.begin(); iter != update.end(); ++iter )
    //     {
    //         DEBUG << "iter->first:" << iter->first << endl;
    //         DEBUG << "iter->second:" << iter->second << endl;
    //         DEBUG << "         got:" << sm[iter->first] << endl;
            
    //         if( sm[iter->first] != iter->second )
    //         {
    //             ess << "attribute " << iter->first << " not correct." << endl;
    //             ess << "expected:" << iter->second << endl;
    //             ess << "     got:" << sm[iter->first] << endl;
    //         }
    //     }

    //     string e = tostr(ess);
    //     DEBUG << "e:" << e << endl;
    //     if( !e.empty() )
    //     {
    //         Throw_WebAPIException( e, 0 );
    //     }
    // }
    
    // BoxComPermissions_t
    // BoxComFile::readPermissions()
    // {
    //     BoxComPermissions_t ret;
    //     QNetworkRequest req = createRequest(DRIVE_BASE() + "files/" + m_id + "/permissions");
    //     req.setRawHeader("Content-Type", "application/json" );
    //     stringmap_t args;
    //     args["fileId"] = m_id;
    //     QNetworkReply* reply = getDrive()->callMeth( req, args );
    //     QByteArray ba = reply->readAll();
    //     DEBUG << "readPermissions() REST error code:" << reply->error() << endl;
    //     DEBUG << "readPermissions() HTTP response  :" << httpResponse(reply) << endl;
    //     DEBUG << "readPermissions() RESULT:" << tostr(ba) << endl;

    //     QVariantMap qm = JSONToQVMap( tostr(ba) );
    //     QVariantList l = qm["items"].toList();
    //     foreach (QVariant ding, l)
    //     {
    //         QVariantMap dm = ding.toMap();

    //         int perm = BoxComPermission::NONE;
    //         if( dm["role"] == "reader" )
    //             perm = BoxComPermission::READ;
    //         if( dm["role"] == "writer" || dm["role"] == "owner" )
    //             perm = BoxComPermission::WRITE;
            
    //         fh_BoxComPermission p = new BoxComPermission( perm, tostr(dm["name"]));
    //         ret.push_back(p);
    //     }
        
        
    //     return ret;
    // }

    // void
    // BoxComFile::sharesAdd( std::string email )
    // {
    //     stringlist_t emails;
    //     emails.push_back( email );
    //     return sharesAdd( emails );
    // }
    
    // void
    // BoxComFile::sharesAdd( stringlist_t& emails )
    // {
    //     // POST https://www.boxcomapis.com/drive/v2/files/fileId/permissions

    //     for( stringlist_t::iterator si = emails.begin(); si != emails.end(); ++si )
    //     {
    //         string email = *si;
            
    //         QNetworkRequest req = createRequest(DRIVE_BASE() + "files/" + m_id + "/permissions");
    //         req.setRawHeader("Content-Type", "application/json" );
    //         stringmap_t args;
    //         args["fileId"] = m_id;
    //         args["emailMessage"] = "Life moves pretty fast. If you don't stop and look around once in a while, you could miss it.";

    //         stringstream bodyss;
    //         bodyss << "{"                                       << endl
    //                << " \"kind\": \"drive#permission\", "       << endl
    //                << "   \"value\": \"" << email << "\" , " << endl
    //                << "   \"role\": \"writer\","                << endl
    //                << "   \"type\": \"user\""                   << endl
    //                << " } "                                     << endl;

    //         DEBUG << "body:" << tostr(bodyss) << endl;
    //         QNetworkReply* reply = getDrive()->callPost( req, args, tostr(bodyss) );
    //         QByteArray ba = reply->readAll();
    //         DEBUG << "sharesAdd() REST error code:" << reply->error() << endl;
    //         DEBUG << "sharesAdd() HTTP response  :" << httpResponse(reply) << endl;
    //         DEBUG << "sharesAdd() RESULT:" << tostr(ba) << endl;

    //         stringmap_t sm = JSONToStringMap( tostr(ba) );
    //         if( !sm["etag"].empty() && !sm["id"].empty() )
    //             continue;

    //         // Failed
    //         stringstream ess;
    //         ess << "Failed to create permission for user:" << email << endl
    //                << " reply:" << tostr(ba) << endl;
    //         Throw_WebAPIException( tostr(ess), 0 );
    //     }
    // }
    
    

    /********************/
    
    
    BoxComClient::BoxComClient()
        : m_clientID( "881964254376.apps.boxcomusercontent.com" )
        , m_secret(   "UH9zxZ8k_Fj3actLPRVPVG8Q" )
    {
        readAuthTokens();        
    }

    void
    BoxComClient::readAuthTokens()
    {
        m_accessToken  = getEDBString( FDB_SECURE, "gdrive-access-token",  "" );
        m_refreshToken = getEDBString( FDB_SECURE, "gdrive-refresh-token", "" );
        m_accessToken_expiretime = toType<time_t>(
            getEDBString( FDB_SECURE,
                          "gdrive-access-token-expires-timet", "0"));
    }
    
    std::string
    BoxComClient::AUTH_BASE()
    {
        return "https://accounts.boxcom.com/o/oauth2/";
    }
    
    
    fh_BoxComClient
    BoxComClient::getBoxComClient()
    {
        Main::processAllPendingEvents();
        KDE::ensureKDEApplication();
        static fh_BoxComClient ret = new BoxComClient();
        return ret;
    }

    bool
    BoxComClient::haveAPIKey() const
    {
        return !m_clientID.empty() && !m_secret.empty();
    }

    bool
    BoxComClient::isAuthenticated() const
    {
        return !m_accessToken.empty() && !m_refreshToken.empty();
    }
    
    
    void
    BoxComClient::handleFinished()
    {
        QNetworkReply* r = dynamic_cast<QNetworkReply*>(sender());
        m_waiter.unblock(r);
        DEBUG << "handleFinished() r:" << r << endl;
    }

    QNetworkReply*
    BoxComClient::wait(QNetworkReply* reply )
    {
        connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
        m_waiter.block(reply);
        return reply;
    }
    
    
    QNetworkReply*
    BoxComClient::post( QNetworkRequest req )
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
    BoxComClient::requestToken( stringmap_t args )
    {
        QUrl u( string(AUTH_BASE() + "auth").c_str() );
        u.addQueryItem("response_type", "code" );
        u.addQueryItem("client_id",     m_clientID.c_str() );
        u.addQueryItem("redirect_uri",  "urn:ietf:wg:oauth:2.0:oob" );
        u.addQueryItem("scope",         "https://www.boxcomapis.com/auth/drive.file "
                       "https://www.boxcomapis.com/auth/drive "
                       "https://www.boxcomapis.com/auth/drive.scripts "
                       "https://www.boxcomapis.com/auth/drive.appdata " );
        u.addQueryItem("state",         "anyone... anyone?" );
        std::string authURL = tostr( u.toEncoded() );
        return authURL;
    }


    

    void
    BoxComClient::accessToken( const std::string& code, stringmap_t args )
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
        setEDBString( FDB_SECURE, "gdrive-access-token",  sm["access_token"]  );
        setEDBString( FDB_SECURE, "gdrive-refresh-token", sm["refresh_token"] );
        setEDBString( FDB_SECURE, "gdrive-access-token-expires-timet", tostr(expiretime) );
        readAuthTokens();
    }

    void
    BoxComClient::ensureAccessTokenFresh( int force )
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
        setEDBString( FDB_SECURE, "gdrive-access-token",  sm["access_token"]  );
        setEDBString( FDB_SECURE, "gdrive-access-token-expires-timet", tostr(expiretime) );
        readAuthTokens();
        cerr << "ensureAccessTokenFresh(e) m_accessToken:" << m_accessToken << endl;
    }

    QNetworkRequest 
    BoxComClient::addAuth( QNetworkRequest req )
    {
        req.setRawHeader("Authorization", string(string("Bearer ") + m_accessToken).c_str() );
        return req;
    }
    

    QNetworkReply*
    BoxComClient::callMeth( QNetworkRequest req, stringmap_t args )
    {
        req = setArgs( req, args );
        req = addAuth( req );
        ensureAccessTokenFresh();

        
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
    BoxComClient::callPost( QNetworkRequest req, stringmap_t args, const std::string& body )
    {
        req = setArgs( req, args );
        req = addAuth( req );
        ensureAccessTokenFresh();

        
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
    BoxComClient::DRIVE_BASE()
    {
        return "https://www.boxcomapis.com/drive/v2/";
    }
    QNetworkRequest
    BoxComClient::createRequest( const std::string& earlTail )
    {
        QUrl u( string(DRIVE_BASE() + earlTail).c_str() );
        QNetworkRequest req;
        req.setUrl( u );
        return req;
    }


    
    
    boxfiles_t
    BoxComClient::filesList( const std::string& q_const, const std::string& pageToken )
    {
        string q = q_const;
        
        if( q.empty() )
        {
            q = "hidden = false and trashed = false";
        }
        
        boxfiles_t ret;
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
//            fh_BoxComFile f = new BoxComFile( this, dm );
//            ret.push_back(f);
        }
        
        return ret;
    }
    

    /****************************************/
    /****************************************/
    /****************************************/
    
    
    
};
