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

#include "libferrisfacebook_shared.hh"
#include <Ferris/Configuration_private.hh>
#include "plugins/context/webphotos/libferriswebphotos_shared.hh"

#include <QNetworkReply>
#include <QNetworkAccessManager>

#include <Ferris/FerrisQt_private.hh>
#include <Ferris/FerrisKDE.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/Iterator.hh>
#include <Ferris/FerrisOpenSSL.hh>

#include <sys/time.h>

#define DEBUG LG_FACEBOOK_D

namespace Ferris
{
    namespace Facebook
    {

        using namespace XML;
        using namespace std;
        static const string DBNAME = FDB_SECURE;

        const char* CFG_FACEBOOK_USERNAME_K = "facebook-username";
        const char* CFG_FACEBOOK_USERNAME_DEF = "";

        const char* CFG_FACEBOOK_SESSION_KEY_K = "facebook-session-key";
        const char* CFG_FACEBOOK_SESSION_KEY_DEF = "";

        const char* CFG_FACEBOOK_SESSION_SECRET_K = "facebook-session-secret";
        const char* CFG_FACEBOOK_SESSION_SECRET_DEF = "";

        const char* CFG_FACEBOOK_UID_K = "facebook-uid";
        const char* CFG_FACEBOOK_UID_DEF = "";
    
        const char* REST_URL = "http://api.facebook.com/restserver.php";
    
        typedef std::list< DOMElement* > entries_t;

        std::string getChildElementText( DOMNode* node, const std::string& name )
        {
            DOMElement* e = getChildElement( node, name );
            return e ? getChildText( e ) : "";
        }


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


        Contact::Contact( fh_facebook fb, fh_domdoc dom, DOMElement* e )
            :
            m_fb(fb)
        {
            update( dom, e );
        }

        void
        Contact::update( fh_domdoc dom, DOMElement* e )
        {
            m_uid  = getStrSubCtx( e, "id" );
            if( m_uid.empty() )
                m_uid  = getStrSubCtx( e, "uid" );
            m_url  = getStrSubCtx( e, "url" );
            m_name = getStrSubCtx( e, "name" );
            m_type = getStrSubCtx( e, "type" );
            m_picURL = getStrSubCtx( e, "pic" );
            if( m_picURL.empty() )
                m_picURL = getStrSubCtx( e, "pic_square" );
            
            m_birthday  = getStrSubCtx( e, "birthday" );
            m_about     = getStrSubCtx( e, "about_me" );
            m_firstName = getStrSubCtx( e, "first_name" );
            m_lastName  = getStrSubCtx( e, "last_name" );
            m_movies    = getStrSubCtx( e, "movies" );
            m_music     = getStrSubCtx( e, "music" );
            m_interests = getStrSubCtx( e, "interests" );
            m_blurb     = getStrSubCtx( e, "profile_blurb" );
            m_sex       = getStrSubCtx( e, "sex" );
            m_websites  = getStrSubCtx( e, "websites" );
            if( DOMElement* z = getChildElement( e, "current_location" ) )
            {
                m_city     = getStrSubCtx( z, "city" );
                m_state    = getStrSubCtx( z, "state" );
                m_country  = getStrSubCtx( z, "country" );
            }
        }

        QByteArray base64encode( QByteArray& d )
        {
            stringstream iss;
            iss.write( d.data(), d.size () );
            string output = ::Ferris::base64encode( iss.str() );
            QByteArray ret( output.data(), output.size() );
            return ret;
        }
    
    
    
        std::string
        Contact::getVCard()
        {
// BEGIN:VCARD
// VERSION:3.0
// N:Gump;Forrest
// FN:Forrest Gump
// ORG:Bubba Gump Shrimp Co.
// TITLE:Shrimp Man
// TEL;TYPE=WORK,VOICE:(111) 555-1212
// TEL;TYPE=HOME,VOICE:(404) 555-1212
// ADR;TYPE=WORK:;;100 Waters Edge;Baytown;LA;30314;United States of America
// LABEL;TYPE=WORK:100 Waters Edge\nBaytown, LA 30314\nUnited States of America
// ADR;TYPE=HOME:;;42 Plantation St.;Baytown;LA;30314;United States of America
// LABEL;TYPE=HOME:42 Plantation St.\nBaytown, LA 30314\nUnited States of America
// EMAIL;TYPE=PREF,INTERNET:forrestgump@example.com
// REV:20080424T195243Z
// END:VCARD
//m_picURL
            stringstream addrss;
            addrss << m_city << ";" << m_state << ";" << m_country;

            string revformat = "%Y%m%dT%H%M%S";
            time_t revtime = Time::getTime();

            QByteArray photodata;
            if( !m_picURL.empty() )
            {
                m_fb->fetchURL( m_picURL, photodata );
                photodata = base64encode( photodata );
            }
            
            stringstream ss;
            ss << "BEGIN:VCARD" << endl
               << "VERSION:3.0" << endl
               << "N:" << m_lastName << ";" << m_firstName << endl
               << "FN:" << getName() << endl
               << "ADR;TYPE=WORK:;;" << addrss.str() << endl
               << "LABEL;TYPE=WORK:" << addrss.str() << endl
               << "UID:" << m_uid << endl
               << "REV:" << Time::toTimeString(revtime,revformat) << endl
               << "";
            if( photodata.size() )
            {
                ss << "PHOTO;ENCODING=BASE64;TYPE=JPEG:" << endl;
                //ss.write( photodata.data(), photodata.size() );
                stringstream photodatass;
                photodatass.write( photodata.data(), photodata.size() );
                string s;
                while( getline( photodatass, s ))
                {
                    ss << " " << s << endl;
                }
            }
            ss << "END:VCARD" << endl;
        
            return ss.str();
        }
    
        
        Contact::~Contact()
        {
        }
    
        std::string
        Contact::getUID()
        {
            return m_uid;
        }

        std::string
        Contact::getID()
        {
            return m_uid;
        }
    
        std::string
        Contact::getName()
        {
            return m_name;
        }

        std::string
        Contact::getFirstName()
        {
            return m_firstName;
        }

        std::string
        Contact::getLastName()
        {
            return m_lastName;
        }
    
        std::string
        Contact::getPictureURL()
        {
            return m_picURL;
        }
    
        std::string
        Contact::getURL()
        {
            return m_url;
        }
    
        std::string
        Contact::getType()
        {
            return m_type;
        }

        std::string
        Contact::getLocation()
        {
            stringstream addrss;
            addrss << m_city << ", " << m_state << ", " << m_country;
            return addrss.str();
        }
    
        std::string
        Contact::getRDN()
        {
            return getID() + (string)".vcf";
        }
    
    
    
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        Album::Album( fh_facebook fb, fh_domdoc dom, DOMElement* e )
            :
            m_fb(fb)
        {
            update( dom, e );
        }
        
        void Album::update( fh_domdoc dom, DOMElement* e )
        {
            m_id           = getStrSubCtx( e, "aid" );
            m_cover_id     = getStrSubCtx( e, "cover_aid" );
            m_owner_id     = getStrSubCtx( e, "owner" );
            m_name         = getStrSubCtx( e, "name" );
            m_desc         = getStrSubCtx( e, "description" );
            m_url          = getStrSubCtx( e, "link" );
            m_type         = getStrSubCtx( e, "type" );
            m_size         = toint(getStrSubCtx( e, "size" ));
            m_createdTime  = toType<time_t>(getStrSubCtx( e, "created" ));
            m_modifiedTime = toType<time_t>(getStrSubCtx( e, "modified" ));
            
        }
        
        Album::~Album()
        {
        }
        
        std::string Album::getID()
        {
            return m_id;
        }
        
        std::string Album::getCoverID()
        {
            return m_cover_id;
        }
        
        std::string Album::getOwnerID()
        {
            return m_owner_id;
        }
        
        std::string Album::getName()
        {
            return m_name;
        }
        
        std::string Album::getDesc()
        {
            return m_desc;
        }
        
        std::string Album::getURL()
        {
            return m_url;
        }
        
        std::string Album::getType()
        {
            return m_type;
        }
        
        int    Album::getSize()
        {
            return m_size;
        }
        
        time_t Album::getCreatedTime()
        {
            return m_createdTime;
        }
        
        time_t Album::getModifiedTime()
        {
            return m_modifiedTime;
        }
            
    
    
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/


        Comment::Comment( fh_facebook fb, fh_domdoc dom, DOMElement* e )
            :
            m_fb(fb)
        {
            update( dom, e );
        }
        
        void Comment::update( fh_domdoc dom, DOMElement* e )
        {
            m_id        = getStrSubCtx( e, "id" );
            m_author_id = getStrSubCtx( e, "fromid" );
            m_message   = getStrSubCtx( e, "text" );
            m_mtime     = toType<time_t>(getStrSubCtx( e, "time" ));
        }
        
        Comment::~Comment()
        {
        }
    
        std::string Comment::getID()
        {
            return m_id;
        }
        
        std::string Comment::getAuthorID()
        {
            return m_author_id;
        }
        
        std::string Comment::getMessage()
        {
            return m_message;
        }
        
        time_t      Comment::getMTime()
        {
            return m_mtime;
        }
        
    
    

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
    
        
        Post::Post( fh_facebook fb, fh_domdoc dom, DOMElement* e )
            :
            m_fb(fb)
        {
            update( dom, e );
        }
        
        void Post::update( fh_domdoc dom, DOMElement* e )
        {
            m_id         = getStrSubCtx( e, "post_id" );
            m_viewer_id  = getStrSubCtx( e, "viewer_id" );
            m_source_id  = getStrSubCtx( e, "source_id" );
            m_type       = getStrSubCtx( e, "type" );
            m_app_id     = getStrSubCtx( e, "app_id" );
            m_message    = getStrSubCtx( e, "message" );
            m_permalink  = getStrSubCtx( e, "permalink" );
            m_createTime = toType<time_t>(getStrSubCtx( e, "create_time" ));
            m_updateTime = toType<time_t>(getStrSubCtx( e, "updated_time" ));

            if( m_message.empty() )
            {
                if( DOMElement* z = getChildElement( e, "attachment" ) )
                    m_message = getStrSubCtx( z, "description" );
            }

            entries_t entries = XML::getAllChildrenElements( e, "comment" );
            for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
            {
                fh_Comment p = new Comment( m_fb, dom, *ei );
                m_comments.insert( make_pair( p->getID(), p ));
            }
        }
        
        Post::~Post()
        {
        }
    
        std::string Post::getID()
        {
            return m_id;
        }
    
        std::string Post::getViewerID()
        {
            return m_viewer_id;
        }
    
        std::string Post::getSourceID()
        {
            return m_source_id;
        }
    
        std::string Post::getType()
        {
            return m_type;
        }
    
        std::string Post::getAppID()
        {
            return m_app_id;
        }
    
        std::string Post::getMessage()
        {
            return m_message;
        }
    
        std::string Post::getPermalink()
        {
            return m_permalink;
        }
    
        time_t      Post::getCreateTime()
        {
            return m_createTime;
        }
    
        time_t      Post::getUpdateTime()
        {
            return m_updateTime;
        }
    
        CommentMap_t     Post::getComments()
        {
            return m_comments;
        }

        std::string
        Post::getRDN()
        {
            return getID();
        }
    
    
        std::string
        Post::getAuthorName()
        {
            std::string sid = getSourceID();
            return sid;
        }
    
    
    

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        QNetworkAccessManager*
        Facebook::getQManager()
        {
            return ::Ferris::getQManager();
        }

        Facebook::Facebook()
        {
            m_username   = getEDBString( DBNAME, CFG_FACEBOOK_USERNAME_K,    CFG_FACEBOOK_USERNAME_DEF );
            m_sessionKey = getEDBString( DBNAME, CFG_FACEBOOK_SESSION_KEY_K, CFG_FACEBOOK_SESSION_KEY_DEF );
            m_sessionSecret = getEDBString( DBNAME, CFG_FACEBOOK_SESSION_SECRET_K, CFG_FACEBOOK_SESSION_SECRET_DEF );
            m_uid = getEDBString( DBNAME, CFG_FACEBOOK_UID_K, CFG_FACEBOOK_UID_DEF );
            m_apiKey = getStrSubCtx( "~/.ferris/facebook-api-key.txt", "" );
            m_apiSecret = getStrSubCtx( "~/.ferris/facebook-api-secret.txt", "" );
        }
    
        Facebook::~Facebook()
        {
        }

        typedef map< string, string > stringmap_t;
        stringmap_t to_std_map( const QList<QPair<QString, QString> >& qm )
        {
            map< string, string > ret;
            for( QList<QPair<QString, QString> >::const_iterator qi = qm.begin(); qi!=qm.end(); ++qi )
            {
                ret.insert( make_pair( tostr(qi->first), tostr(qi->second)));
            }
            return ret;
        }
    
        void
        Facebook::handleFinished()
        {
            QNetworkReply* r = dynamic_cast<QNetworkReply*>(sender());
            m_waiter.unblock( r );
            DEBUG << "handleFinished()" << endl;
        }

        QNetworkRequest
        Facebook::createRequest( QUrl u )
        {
            QNetworkRequest req;
            req.setUrl(u);
            return req;
        }
    
        QNetworkRequest
        Facebook::createRequest( const std::string& u )
        {
            if( u.empty() )
                return createRequest( REST_URL );
            return createRequest( QUrl( u.c_str() ) );
        }
    

        QByteArray&
        Facebook::fetchURL( const std::string earl, QByteArray& ret )
        {
            QNetworkRequest req( QUrl( earl.c_str() ));
            QNetworkReply* reply = getQManager()->get( req );
            connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
            m_waiter.block( reply );
            ret = reply->readAll();
            return ret;
        }
    
        QUrl
        Facebook::sign( QUrl u )
        {
            if( !m_apiKey.empty() )
                u.addQueryItem("api_key", m_apiKey.c_str());
            u.addQueryItem("v", "1.0");
            u.addQueryItem("format", "XML");
            if( !m_sessionKey.empty() )
                u.addQueryItem("session_key", m_sessionKey.c_str() );

            struct timeval tv;
            gettimeofday( &tv, 0 );
            double callid = tv.tv_sec + (tv.tv_usec / 1000000 );
            u.addQueryItem("call_id", tostr(callid).c_str() );

            // sign request
            // http://wiki.developers.facebook.com/index.php/How_Facebook_Authenticates_Your_Application
            {
                stringmap_t argmap = to_std_map( u.queryItems() );
                stringstream ss;
                for( stringmap_t::iterator si = argmap.begin(); si!=argmap.end(); ++si )
                {
                    ss << si->first << "=" << si->second;
                }
                string secret = m_apiSecret;
                if( !m_sessionSecret.empty() )
                    secret = m_sessionSecret;
            
                ss << secret;

                DEBUG << "      apiSecret:" << m_apiSecret << endl;
                DEBUG << "m_sessionSecret:" << m_sessionSecret << endl;
                DEBUG << "   using secret:" << secret << endl;
                DEBUG << "Signing req:" << ss.str() << endl;
                string sig = digest( ss.str(), "md5" );
                u.addQueryItem("sig", sig.c_str());
            }
            
            return u;
        }
        
        QNetworkReply*
        Facebook::get( const std::string& meth, QNetworkRequest req )
        {
            QUrl u = req.url();
            u.addQueryItem("method", meth.c_str() );
            u = sign( u );
            req.setUrl( u );
            QNetworkReply* reply = getQManager()->get( req );
            connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
            m_waiter.block( reply );
            DEBUG << "REST error code:" << reply->error() << endl;
            return reply;
        }
    

        std::string
        Facebook::getUserID()
        {
            return m_uid;
        }
    
        std::string
        Facebook::getUserName()
        {
            return m_username;
        }
    
        bool
        Facebook::haveSessionKey()
        {
            return !m_sessionKey.empty();
        }
    
        std::string
        Facebook::getSessionKey()
        {
            return m_sessionKey;
        }
    
        bool
        Facebook::haveAPIKey()
        {
            return !m_apiKey.empty();
        }

        std::string
        Facebook::APIKey()
        {
            return m_apiKey;
        }

        fh_Upload
        Facebook::createUpload( const std::string& methodName )
        {
            return new Upload( this, methodName );
        }
        
        PostMap_t
        Facebook::getRecentStreamPosts()
        {
            if( m_recentStreamPosts.empty() )
            {
                getStream();
            }
            return m_recentStreamPosts;
        }
    
        ContactMap_t
        Facebook::getContacts()
        {
            if( m_contacts.empty() )
            {
            }
            return m_contacts;
        }

        ContactMap_t
        Facebook::getFriends()
        {
            if( m_friends.empty() )
            {
                m_friends = getUserInfo();
//            m_friends = getStandardUserInfo();
            
            }
            return m_friends;
        
        }
    
    
    
        void
        Facebook::mergeContact( ContactMap_t& dst, fh_Contact c )
        {
            dst.insert( make_pair( c->getID(), c ));
        }
    
        void
        Facebook::getStream( time_t start, time_t end, int limit, int viewer_id )
        {
            QUrl u( REST_URL );
            u.addQueryItem("limit", tostr(limit).c_str());
            // if( start )
            //     u.addQueryItem("start_time", tostr(start).c_str());
            // if( end )
            //     u.addQueryItem("end_time", tostr(end).c_str());
            if( viewer_id )
                u.addQueryItem("viewer_id", tostr(viewer_id).c_str());
            else
                u.addQueryItem("viewer_id", m_uid.c_str());
            DEBUG << "passed vid:" << viewer_id << " m_uid:" << m_uid << endl;
        
            QNetworkRequest  req = createRequest( u );
            QNetworkReply* reply = get( "Stream.get", req );
            QByteArray ba = reply->readAll();
            DEBUG << "getStream() result:" << tostr(ba) << endl;

            entries_t entries;
            fh_domdoc dom = Factory::StringToDOM( tostr(ba) );

            entries = XML::getAllChildrenElements( dom->getDocumentElement(), "stream_post" );
            for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
            {
                fh_Post p = new Post( this, dom, *ei );
                m_recentStreamPosts.insert( make_pair( p->getID(), p ) );
            }

            entries = XML::getAllChildrenElements( dom->getDocumentElement(), "profile" );
            for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
            {
                fh_Contact p = new Contact( this, dom, *ei );
                mergeContact( m_contacts, p );
            }
        
            entries = XML::getAllChildrenElements( dom->getDocumentElement(), "album" );
            for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
            {
                fh_Album p = new Album( this, dom, *ei );
                m_albums.insert( make_pair( p->getID(), p ) );
            }
        
        }

        uidList_t
        Facebook::getFriendIDList()
        {
            uidList_t ret;
        
            QUrl u( REST_URL );
            u.addQueryItem("flid","0");
            u.addQueryItem("uid", m_uid.c_str() );
            QNetworkRequest  req = createRequest( u );
            QNetworkReply* reply = get( "Friends.get", req );
            QByteArray ba = reply->readAll();
            DEBUG << "getFriendIDList() m_uid:" << m_uid << " result:" << tostr(ba) << endl;
            entries_t entries;
            fh_domdoc dom = Factory::StringToDOM( tostr(ba) );

            entries = XML::getAllChildrenElements( dom->getDocumentElement(), "Friends_get_response_elt" );
            for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
            {
                long uid = toType<long>(getChildText( *ei ));
                ret.push_back(uid);
            }
            return ret;
        }
    

        ContactMap_t
        Facebook::getUserInfo( uidList_t uidslist, const std::string fields_add )
        {
            string fields = "uid,about_me,birthday,books,current_location,first_name,last_name,name,interests,is_app_user,";
            fields = fields + (string)"movies,locale,music,notes_count,pic,pic_big,pic_square,profile_blurb,sex,tv,website";
            fields = fields + "," + fields_add;

            QUrl u( REST_URL );
            string uids = Util::createSeperatedList( uidslist.begin(), uidslist.end(), "," );
            DEBUG << "getUserInfo() uids:" << uids << endl;
            DEBUG << "getUserInfo() fields:" << fields << endl;
            u.addQueryItem("uids",   uids.c_str());
            u.addQueryItem("fields", fields.c_str());
            QNetworkRequest  req = createRequest( u );
            QNetworkReply* reply = get( "Users.getInfo", req );
            QByteArray ba = reply->readAll();
            DEBUG << "getUserInfo() result:" << tostr(ba) << endl;
            fh_domdoc dom = Factory::StringToDOM( tostr(ba) );

            ContactMap_t ret;
            entries_t entries = XML::getAllChildrenElements( dom->getDocumentElement(), "user" );
            for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
            {
                fh_Contact p = new Contact( this, dom, *ei );
                mergeContact( ret, p );
            }
            return ret;
        }
    
        ContactMap_t
        Facebook::getUserInfo( const std::string fields )
        {
            uidList_t uids = getFriendIDList();
            return getUserInfo( uids, fields );
        }
    
        ContactMap_t
        Facebook::getStandardUserInfo( uidList_t uidslist )
        {
            string fields = "uid,first_name,last_name,name,timezone,birthday,sex,affiliations,locale,profile_url,proxied_email";
        
            QUrl u( REST_URL );
            string uids = Util::createSeperatedList( uidslist.begin(), uidslist.end(), "," );
            DEBUG << "getUserInfo() uids:" << uids << endl;
            DEBUG << "getUserInfo() fields:" << fields << endl;
            u.addQueryItem("uids",   uids.c_str());
            u.addQueryItem("fields", fields.c_str());
            QNetworkRequest  req = createRequest( u );
            QNetworkReply* reply = get( "Users.getInfo", req );
            QByteArray ba = reply->readAll();
            DEBUG << "getStandardUserInfo() result:" << tostr(ba) << endl;

            ContactMap_t ret;
            return ret;
        }
    
        ContactMap_t
        Facebook::getStandardUserInfo()
        {
            uidList_t uids = getFriendIDList();
            return getStandardUserInfo( uids );
        }
    
    
    
        std::string
        Facebook::getStatus()
        {
            std::map< time_t, std::string > col = getStatusMessages();
            if( col.empty() )
                return "";
            return col.rbegin()->second;
        }

        void
        Facebook::setStatus( const std::string& s )
        {
            QUrl u( REST_URL );
            u.addQueryItem("status", s.c_str());
            QNetworkRequest  req = createRequest( u );
            QNetworkReply* reply = get( "Status.set", req );
            QByteArray ba = reply->readAll();
            DEBUG << "Facebook::setStatus() result:" << tostr(ba) << endl;
            
            // permission denied.
            if( reply->error() == 250 )
            {
            }
        }
    
    
        std::map< time_t, std::string >
        Facebook::getStatusMessages()
        {
            std::map< time_t, std::string > ret;

            DEBUG << "Facebook::getStatusMessages(top)" << endl;
            if( !m_statusMessages.empty() )
                return m_statusMessages;
        
            QNetworkRequest  req = createRequest();
            QNetworkReply* reply = get( "Status.get", req );

            QByteArray ba = reply->readAll();
            DEBUG << "Facebook::getStatusMessages() result:" << tostr(ba) << endl;

            fh_domdoc dom = Factory::StringToDOM( (string)ba );
            entries_t entries = XML::getAllChildrenElements( dom->getDocumentElement(), "user_status", false );
            for( entries_t::iterator ei = entries.begin(); ei!=entries.end(); ++ei )
            {
                time_t tt      = toType<time_t>(getChildElementText( *ei, "time" ));
                string message = getChildElementText( *ei, "message" );
                ret.insert( make_pair( tt, message ) );
            }

            m_statusMessages = ret;
            return ret;
        }
    
    

    
        std::string
        Facebook::createAuthToken()
        {
            m_sessionKey = "";
            DEBUG << "Facebook::createAuthToken() session:" << m_sessionKey << endl;
            QNetworkRequest  req = createRequest();
            QNetworkReply* reply = get( "Auth.createToken", req );
            QByteArray ba = reply->readAll();
            DEBUG << "Facebook::createAuthToken() result:" << tostr(ba) << endl;

            fh_domdoc dom = Factory::StringToDOM( (string)ba );
            string ret = getChildText( dom->getDocumentElement() );
            return ret;
        }

    
        std::string
        Facebook::getAuthSession( const std::string& authToken )
        {
            QUrl u( REST_URL );
            u.addQueryItem("auth_token", authToken.c_str());
            QNetworkRequest  req = createRequest( u );
            QNetworkReply* reply = get( "Auth.getSession", req );
            QByteArray ba = reply->readAll();
            DEBUG << "Facebook::getAuthSession() result:" << tostr(ba) << endl;

            fh_domdoc dom = Factory::StringToDOM( (string)ba );
            DOMElement* e = dom->getDocumentElement();

            m_sessionKey    = getChildElementText( e, "session_key" );
            m_sessionSecret = getChildElementText( e, "secret" );
            m_uid           = getChildElementText( e, "uid" );
            if( !m_sessionKey.empty() )
            {
                setEDBString( DBNAME, CFG_FACEBOOK_SESSION_KEY_K, m_sessionKey );
                setEDBString( DBNAME, CFG_FACEBOOK_SESSION_SECRET_K, m_sessionSecret );
                setEDBString( DBNAME, CFG_FACEBOOK_UID_K, m_uid );
            
            }
            return m_sessionKey;
        }

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        Upload::Upload( fh_facebook fb, const std::string& methodName )
            :
            m_fb( fb ),
            m_methodName( methodName )
        {
        }


        void
        Upload::streamingUploadComplete()
        {
            DEBUG << "Upload::streamingUploadComplete() m_streamToQIO:" << GetImpl(m_streamToQIO)
                  << " blocking for reply" << endl;
            QNetworkReply* reply = m_reply;
        
            m_streamToQIO->writingComplete();
            DEBUG << "Upload::streamingUploadComplete() reading data..." << endl;
            QByteArray ba = m_streamToQIO->readResponse();
            cerr << "Upload::streamingUploadComplete() m_streamToQIO:" << GetImpl(m_streamToQIO)
                 << " reply:" << reply->error()  << endl;
        
            DEBUG << "streamingUploadComplete() reply.err:" << reply->error() << endl;
            DEBUG << "streamingUploadComplete() got reply:" << tostr(ba) << endl;

            fh_domdoc dom = Factory::StringToDOM( tostr(ba) );
            DOMElement* e = dom->getDocumentElement();

            m_url = XML::getChildText( firstChild( e, "link" ));
            m_id  = XML::getChildText( firstChild( e, "vid" ));
            
            string error_code = XML::getChildText( firstChild( e, "error_code" ));
            if( !error_code.empty() )
            {
                string error_msg = XML::getChildText( firstChild( e, "error_msg" ));
                fh_stringstream ss;
                ss << "Error uploading image to facebook" << endl
                   << "reason:" << error_msg << endl;
                Throw_WebPhotoException( tostr(ss), 0 );
            }
        }

        
        long
        Upload::getMaxDesiredWidthOrHeight()
        {
            long ret = toint(getEDBString( FDB_GENERAL, "facebook-max-desired-width-or-height", "0" ));
            return ret;
        }

        void
        Upload::setMaxDesiredWidthOrHeight( long v )
        {
            setEDBString( FDB_GENERAL, "facebook-max-desired-width-or-height", tostr(v) );
        }
        
        
        void
        Upload::OnStreamClosed( fh_istream& ss, std::streamsize tellp, const std::string& ContentType )
        {
            const string s = StreamToString(ss);

            cerr << "making a scaled and transformed image for upload...ct:" << ContentType << endl;
            string ext = "jpg";
            if( ContentType == "image/cr2" || ContentType == "image/x-canon-cr2" )
                ext = "CR2";
            string dst_url = Shell::getTmpDirPath() + "/libferris-upload-temp-image." + ext;
            fh_context c = Shell::acquireContext( dst_url, 0, false );
            {
                fh_iostream oss = c->getIOStream( ios::out | ios::trunc );
                oss << s << flush;
            }
            fh_webPhotoUpload wpu = new WebPhotoUpload(0);
            wpu->setLargestDimensionExplicit( getMaxDesiredWidthOrHeight() );
            fh_context scaledc = wpu->makeScaledImage( c );

            
            cerr << "OnStreamClosed() uploading scaled image, original data.len:" << s.size() << endl;
            cerr << "OnStreamClosed() uploading scaled image, scaled   data.len:" << getStrAttr( scaledc, "size", "unknown" ) << endl;
            {
                fh_iostream oss = real_createStreamingUpload( ContentType );
//                oss << getStrAttr( scaledc, "content", "" ) << flush;
                fh_istream iss = scaledc->getIStream();
                copy( istreambuf_iterator<char>(iss),
                      istreambuf_iterator<char>(),
                      ostreambuf_iterator<char>(oss));
                oss << flush;
            }
        }
        
        fh_iostream
        Upload::createStreamingUpload( const std::string& ContentType )
        {
            m_url = "";
            m_id  = "";

            if( m_methodName == "facebook.photos.upload"
                && getMaxDesiredWidthOrHeight() )
            {
                cerr << "Using old, non streaming upload because of scaling..." << endl;
                fh_stringstream ret;
                ret->getCloseSig().connect(
                    sigc::bind( sigc::mem_fun(*this, &_Self::OnStreamClosed ), ContentType )); 
                return ret;
            }
            
            return real_createStreamingUpload( ContentType );
        }
        
        fh_iostream
        Upload::real_createStreamingUpload( const std::string& ContentType )
        {
            const char* REST_URL = "http://api-video.facebook.com/restserver.php";
            if( m_methodName == "facebook.photos.upload" )
                REST_URL = "http://api.facebook.com/restserver.php";
            QUrl u( REST_URL );
 
            if( !m_title.empty() )
                u.addQueryItem("title", m_title.c_str());
            if( !m_desc.empty() )
            {
                u.addQueryItem("description", m_desc.c_str());
                u.addQueryItem("caption",     m_desc.c_str());
                DEBUG << "setting caption to:" << m_desc << endl;
            }
                        
//            u.addQueryItem("method", "facebook.video.upload" );
            u.addQueryItem("method", m_methodName.c_str() );
            u = m_fb->sign( u );
            DEBUG << "upload method:" << m_methodName << endl;

            QNetworkRequest req = m_fb->createRequest( u );
            req.setHeader(QNetworkRequest::ContentLengthHeader, tostr(m_uploadSize).c_str() );
            m_streamToQIO = Factory::createStreamToQIODevice();
        
            stringstream blobss;
            blobss << "Content-Disposition: form-data; filename=\"" << m_uploadFilename << "\" \n"
                   << "Content-Type: " << ContentType << "";
            m_streamToQIO->setContentType( "multipart/form-data" );
            QNetworkReply* reply = m_streamToQIO->post( ::Ferris::getQNonCachingManager(),
                                                        req, blobss.str() );
            m_reply = reply;
            fh_iostream ret = m_streamToQIO->getStream();
            return ret;
        }
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
    };
    
    namespace Factory
    {
        ::Ferris::Facebook::fh_facebook getFacebook()
        {
            Main::processAllPendingEvents();
            KDE::ensureKDEApplication();
            
            static Facebook::fh_facebook cache = 0;
            if( !cache )
                cache = new Facebook::Facebook();
            return cache;
        }
    };
    
    
    
    /****************************************/
    /****************************************/
    /****************************************/
    
};
