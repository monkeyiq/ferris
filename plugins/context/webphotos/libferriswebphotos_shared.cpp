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

#define HAVE_GFXMAGIC 1

#include "config.h"

#include "libferriswebphotos_shared.hh"
#include <Ferris/FerrisDOM.hh>
#include <Ferris/FerrisOpenSSL.hh>
#include <Ferris/Configuration_private.hh>
#include <Ferris/Runner.hh>

#include <QNetworkReply>
#include <QNetworkAccessManager>

#include <Ferris/FerrisQt_private.hh>
#include <Ferris/FerrisKDE.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/Iterator.hh>
#include <Ferris/Enamel.hh>
#include <Ferris/FerrisWebServices_private.hh>

#include <X11/Xlib.h>
#ifdef HAVE_IMLIB2
#include <Imlib2.h>
#endif

#define DEBUG LG_WEBPHOTO_D


#include "Ferris/FerrisCurl_private.hh"



using namespace std;


namespace Ferris
{

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    Contact::Contact( fh_webPhotos wf, fh_context md )
        :
        m_wf(wf)
    {
        m_id       = getStrAttr( md, "nsid", "" );
        m_username = getStrAttr( md, "username", "" );
        m_realname = getStrAttr( md, "realname", "" );
        m_location = getStrAttr( md, "location", "" );
    }
    
    Contact::Contact( fh_webPhotos wf, const std::string& aid )
        :
        m_wf(wf)
    {
    }
    
    Contact::~Contact()
    {
    }
    
    std::string
    Contact::getRDN()
    {
        return getUserName();
    }
    

    std::string
    Contact::getID()
    {
        return m_id;
    }
    
    std::string
    Contact::getUserName()
    {
        return m_username;
    }
    
    std::string
    Contact::getRealName()
    {
        return m_realname;
    }
    
    std::string
    Contact::getLocation()
    {
        return m_location;
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
//            addrss << m_city << ";" << m_state << ";" << m_country;

            string revformat = "%Y%m%dT%H%M%S";
            time_t revtime = Time::getTime();

            QByteArray photodata;
            // if( !m_picURL.empty() )
            // {
            //     m_fb->fetchURL( m_picURL, photodata );
            //     photodata = base64encode( photodata );
            // }
            
            stringstream ss;
            ss << "BEGIN:VCARD" << endl
               << "VERSION:3.0" << endl
               << "N:" << getUserName() << endl
               << "FN:" << getRealName() << endl
               << "ADR;TYPE=WORK:;;" << addrss.str() << endl
               << "LABEL;TYPE=WORK:" << addrss.str() << endl
               << "UID:" << m_id << endl
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
    
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    Comment::Comment( fh_webPhotos wf, fh_context md )
        :
        m_wf(wf),
        m_author( 0 )
    {
        m_id          = getStrAttr( md, "id", "" );
        m_dateCreated = toType<time_t>(getStrAttr( md, "datecreate", "0" ));
        m_permalink   = getStrAttr( md, "permalink", "" );
        m_authorID    = getStrAttr( md, "author", "" );
        m_authorName  = getStrAttr( md, "authorname", "" );

        m_content = getStrAttr( md, "content", "" );
    }

    fh_Comment
    Comment::Create( fh_webPhotos wf, fh_PhotoMetadata photo, const std::string& v )
    {
        stringmap_t args;
        args["photo_id"] = tostr(photo->getID());
        args["comment_text"] = v;
        fh_context res = wf->signedCall( "flickr.photos.comments.addComment", args );
        DEBUG << "Comment::Create() made signed call..." << endl;
        fh_context c = res;
        if( !c->empty() )
        {
            c = *(c->begin());
        }
        string id = getStrAttr( c, "id", "" );
        DEBUG << "Created id:" << id << endl;
        
        CommentList_t col = photo->getComments();
        for( rs<CommentList_t> pi( col ); pi; ++pi )
        {
            fh_Comment com = *pi;
            if( com->getID() == id )
                return com;
        }
        DEBUG << "Cant find new comment! id:" << id << endl;
        return 0;
    }
    
        
    Comment::~Comment()
    {
    }

    std::string
    Comment::getID()
    {
        return m_id;
    }
    
    
    fh_Contact
    Comment::getAuthor()
    {
        if( !m_author )
        {
            m_author = new Contact( m_wf, m_authorID );
        }
        return m_author;
    }

    std::string
    Comment::getAuthorName()
    {
        return m_authorName;
    }
    
        
    time_t
    Comment::getDateCreated()
    {
        return m_dateCreated;
    }
    
    std::string
    Comment::getPermalink()
    {
        return m_permalink;
    }
    
    std::string
    Comment::getContent()
    {
        return m_content;
    }

    void
    Comment::setContent( const std::string& newCommentValue_uncoded )
    {
        fh_webPhotos wf = m_wf;
        string id = getID();

        string newCommentValue = tostr(QUrl::toPercentEncoding( newCommentValue_uncoded.c_str() ));
        
        stringmap_t args;
        args["comment_id"] = id;
        args["comment_text"] = newCommentValue_uncoded;
        fh_context res = wf->signedCall( "flickr.photos.comments.editComment", args );
        m_content = newCommentValue;
    }
    
    
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

        
    Tag::Tag( fh_webPhotos wf, fh_PhotoMetadata photo, fh_context md )
        :
        m_wf(wf),
        m_photo( photo ),
        m_author(0)
    {
        updateFromMetadata( md );
    }

    Tag::Tag( fh_webPhotos wf, fh_PhotoMetadata photo, const std::string& content )
        :
        m_wf(wf),
        m_photo( photo ),
        m_author(0)
    {
        m_content = content;
    }
    
    
    Tag::~Tag()
    {
    }

    void
    Tag::updateFromMetadata( fh_context md )
    {
        m_id          = getStrAttr( md, "id", "" );
        m_content     = getStrAttr( md, "content", "" );
        m_authorID    = getStrAttr( md, "author", "" );
        m_machineTag = isTrue(getStrAttr( md, "machine_tag", "" ));
    }
    
    
        
    std::string Tag::getID()
    {
        m_photo->ensureTagsContainFullMetadata();
        return m_id;
    }
        
    std::string Tag::getContent()
    {
        return m_content;
    }

    
    std::string Tag::getAuthorID()
    {
        m_photo->ensureTagsContainFullMetadata();
        return m_authorID;
    }
    
    fh_Contact
    Tag::getAuthor()
    {
        m_photo->ensureTagsContainFullMetadata();
        if( !m_author )
        {
            m_author = new Contact( m_wf, m_authorID );
        }
        return m_author;
    }
    
    bool
    Tag::isMachineTag()
    {
        m_photo->ensureTagsContainFullMetadata();
        return m_machineTag;
    }
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    PhotoMetadata::PhotoMetadata( fh_webPhotos wf, fh_context md )
        :
        m_wf( wf ),
        m_haveInitialTagList( false )
    {
        m_id = toType<long long>(getStrAttr( md, "id", "0" ));
        DEBUG << "PhotoMetadata() id:" << m_id << endl;
        m_secret = getStrAttr( md, "secret", "" );
        m_server = getStrAttr( md, "server", "" );
        m_title  = getStrAttr( md, "title", "" );
        m_isPrimary = isTrue(getStrAttr( md, "isprimary", "0" ));
        m_mtime = toType<time_t>(getStrAttr( md, "dateupload", "0" ));
            
    
        string s;

        s = getStrAttr( md, "url_m", "" );
        if( !s.empty() )
            m_sizeearl["Medium"] = s;
        s = getStrAttr( md, "url_o", "" );
        if( !s.empty() )
            m_sizeearl["Original"] = s;
        s = getStrAttr( md, "url_t", "" );
        if( !s.empty() )
            m_sizeearl["Thumbnail"] = s;
        s = getStrAttr( md, "url_s", "" );
        if( !s.empty() )
            m_sizeearl["Small"] = s;

        {
            {
                string nothing = "<nothing>";
                m_haveInitialTagList = ( nothing != getStrAttr( md, "tags", nothing ) );
            }
            
            string t = getStrAttr( md, "tags", "" );
            DEBUG << "PhotoMetadata() have tags:" << t << endl;
            stringlist_t tags;
            Util::parseSeperatedList( t, tags, ' ' );
            for( stringlist_t::iterator ti = tags.begin(); ti != tags.end(); ++ti )
            {
                DEBUG << "PhotoMetadata() adding tag:" << *ti << endl;
                fh_Tag tag = new Tag( wf, this, *ti );
                m_taglist.push_back( tag );
            }
        }
    }
    
    PhotoMetadata::~PhotoMetadata()
    {
    }
    

    string
    PhotoMetadata::get_sizeearl_LargestSize()
    {
        ensure_sizeearl_setup();

        string s = "Original";
        if( m_sizeearl.end() != m_sizeearl.find( s ) )
            return s;
        s = "Large";
        if( m_sizeearl.end() != m_sizeearl.find( s ) )
            return s;            
        s = "Medium";
        if( m_sizeearl.end() != m_sizeearl.find( s ) )
            return s;            
        s = "Small";
        if( m_sizeearl.end() != m_sizeearl.find( s ) )
            return s;            
        s = "Thumbnail";
        if( m_sizeearl.end() != m_sizeearl.find( s ) )
            return s;            
        s = "";
        return s;
    }

    void
    PhotoMetadata::ensure_sizeearl_setup()
    {
        if( !m_sizeearl.empty() )
            return;

        fh_webPhotos wf = m_wf;
        string id = tostr(getID());
                
        stringmap_t args;
        args["api_key"] = wf->getAPIKey();
        args["photo_id"] = id;
        fh_context res = wf->signedCall( "flickr.photos.getSizes", args );
        fh_context c = res->getSubContext( "sizes" );
        string earl = "";
        bool getOriginal = false;
        Context::iterator ci = c->begin();
        Context::iterator ce = c->end();
        string currentsz = "Thumbnail";
        for( ; ci!=ce; ++ci )
        {
            string sz = getStrAttr( *ci, "label", "" );
            string e = getStrAttr( *ci, "source", "" );

            m_sizeearl[ sz ] = e;
        }
    }

    void
    PhotoMetadata::ensureTagsContainFullMetadata()
    {
        DEBUG << "PhotoMetadata::ensureTagsContainFullMetadata(top)" << endl;
        fh_webPhotos wf = m_wf;
        string id = tostr(getID());
        stringmap_t args;
        args["api_key"] = wf->getAPIKey();
        args["photo_id"] = id;
        fh_context res = wf->signedCall( "flickr.photos.getInfo", args );
        fh_context c = res;
        if( !c->empty() )
        {
            c = *(c->begin());
        }
        DEBUG << "PhotoMetadata::ensureTagsContainFullMetadata() c:" << c->getDirName() << endl;
        c = c->getSubContext( "tags" );
        bool wasEmpty = m_taglist.empty();

        Context::iterator ci = c->begin();
        Context::iterator ce = c->end();
        for( ; ci!=ce; ++ci )
        {
            fh_context md = *ci;
            
            if( fh_Tag tag = findTag( getStrAttr( md, "content", "" ) ))
            {
                tag->updateFromMetadata( md );
                continue;
            }

            fh_Tag c = new Tag( wf, this, md );
            m_taglist.push_back(c);
        }
    }

    QByteArray DecodeImageToRGBA32( QByteArray& ba )
    {
        
        string tmpfilepath = "/tmp/ferris-tmp.jpg";
        ofstream oss( tmpfilepath.c_str() );
        oss.write( ba.data(), ba.size() );

#ifdef HAVE_IMLIB2
        Imlib_Load_Error ierr;
        Imlib_Image image = imlib_load_image_with_error_return( tmpfilepath.c_str(), &ierr );
        imlib_context_set_image( image );
        int w = imlib_image_get_width();
        int h = imlib_image_get_height();

        int sz = 4*w*h;
        DATA32* d = imlib_image_get_data_for_reading_only();
        DEBUG << "decoded image data sz:" << sz << endl;
        QByteArray ret( (const char*)d, sz );
        
        imlib_free_image();
#else
        QByteArray ret;
#endif
        return ret;
    }

    QByteArray
    PhotoMetadata::getImageRGBA( const std::string& sizeDesc )
    {
        ensure_sizeearl_setup();
        string earl = m_sizeearl[sizeDesc];
        DEBUG << "thumbnail earl:" << earl << endl;
        if( earl.empty() )
        {
        }

        QNetworkRequest req;
        req.setUrl( QUrl(earl.c_str()) );
        QNetworkAccessManager* qm = getQManager();
        QNetworkReply* reply = qm->get( req );
        connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
        DEBUG << "About to get thumbnail... waiting." << endl;
        m_waiter.block(reply);
        QByteArray ba = reply->readAll();
        DEBUG << "getImageRGBA() got result.sz:" << ba.size() << endl;
        // PURE DEBUG
        {
            ofstream oss("/tmp/out1.jpg");
            oss.write( ba.data(), ba.size() );
        }

        QByteArray ret = DecodeImageToRGBA32( ba );
        DEBUG << "getImageRGBA() got decoded result.sz:" << ret.size() << endl;
        return ret;
    }

    void
    PhotoMetadata::ensureThumbnailRGBAData()
    {
        m_thumbnailRGBAData = getImageRGBA( "Thumbnail" );
    }
    
    void
    PhotoMetadata::ensureRGBAData()
    {
        m_RGBAData = getImageRGBA( get_sizeearl_LargestSize() );
    }
    

    guint32*
    PhotoMetadata::getThumbnailRGBAData()
    {
        ensureThumbnailRGBAData();
        return (guint32*)m_thumbnailRGBAData.data();
    }
    
    guint32
    PhotoMetadata::getThumbnailRGBADataSize()
    {
        ensureThumbnailRGBAData();
        return m_thumbnailRGBAData.size();
    }
    
    guint32*
    PhotoMetadata::getFullRGBAData()
    {
        ensureRGBAData();
        return (guint32*)m_RGBAData.data();
    }
    
    guint32
    PhotoMetadata::getFullRGBADataSize()
    {
        ensureRGBAData();
        return m_RGBAData.size();
    }
    
    
    fh_Tag
    PhotoMetadata::findTag( const std::string& t )
    {
        fh_Tag ret = 0;
        for( TagList_t::iterator ti = m_taglist.begin(); ti != m_taglist.end(); ++ti )
        {
            if( (*ti)->getContent() == t )
            {
                return *ti;
            }
        }
        return ret;
    }
    
    
    fh_Tag
    PhotoMetadata::addTag( const std::string& t )
    {
        DEBUG << "addTag:" << t << endl;
        fh_Tag tag = findTag( t );
        if( tag )
            return tag;

        fh_webPhotos wf = m_wf;
        string id = tostr(getID());
        stringmap_t args;
        args["api_key"] = wf->getAPIKey();
        args["photo_id"] = id;
        args["tags"] = t;
        fh_context res = wf->signedCall( "flickr.photos.addTags", args );
        tag = new Tag( wf, this, t );
        return tag;
    }
    
    
    void
    PhotoMetadata::removeTag( const std::string& t )
    {
        DEBUG << "removeTag:" << t << endl;

        fh_Tag tag = findTag( t );
        if( tag )
        {
            DEBUG << "removeTag, have tag object for:" << t << endl;
            string tagid = tag->getID();
            fh_webPhotos wf = m_wf;
            string id = tostr(getID());
            DEBUG << "removeTag id:" << tagid << " name:" << t << endl;
            
            stringmap_t args;
            args["api_key"] = wf->getAPIKey();
            args["tag_id"] = tagid;
            fh_context res = wf->signedCall( "flickr.photos.removeTag", args );
        }
    }
    
    
    TagList_t
    PhotoMetadata::getTags()
    {
        if( m_haveInitialTagList )
            return m_taglist;

        ensureTagsContainFullMetadata();
        return m_taglist;
    }
    
    CommentList_t
    PhotoMetadata::getComments()
    {
        fh_webPhotos wf = m_wf;
        string id = tostr(getID());
        stringmap_t args;
        args["api_key"] = wf->getAPIKey();
        args["photo_id"] = id;
        fh_context res = wf->signedCall( "flickr.photos.comments.getList", args );
        fh_context c = res->getSubContext( "comments" );

        CommentList_t ret;
        Context::iterator ci = c->begin();
        Context::iterator ce = c->end();
        for( ; ci!=ce; ++ci )
        {
            fh_context md = *ci;
            fh_Comment c = new Comment( wf, md );
            ret.push_back(c);
        }
        return ret;
    }
    
    
    std::string
    PhotoMetadata::getLargestSizeImageURL()
    {
        return m_sizeearl[ get_sizeearl_LargestSize() ];
    }


    void
    PhotoMetadata::handleMetadataChange()
    {
        DEBUG << "PhotoMetadata::handleMetadataChange()" << endl;
        QNetworkReply* r = dynamic_cast<QNetworkReply*>(sender());
        m_waiter.unblock(r);
    }

    void
    PhotoMetadata::handleFinished()
    {
        DEBUG << "PhotoMetadata::handleFinished()" << endl;
        QNetworkReply* r = dynamic_cast<QNetworkReply*>(sender());
        m_waiter.unblock(r);
    }
    
    long long
    PhotoMetadata::getLargestSizeImageSize()
    {
        string earl = getLargestSizeImageURL();
        LG_WEBPHOTO_D << "original image URL:" << earl << endl;

        QNetworkRequest req;
        req.setUrl( QUrl(earl.c_str()) );
        QNetworkAccessManager* qm = getQManager();
        QNetworkReply* reply = qm->get( req );
        connect( reply, SIGNAL( metaDataChanged() ), SLOT( handleMetadataChange() ) );
        connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );

        cerr << "sz:" << reply->header( QNetworkRequest::ContentLengthHeader ).toInt() << endl;
        DEBUG << "About to get size information... waiting." << endl;
        cerr << "About to get size information... waiting." << endl;
        m_waiter.block(reply);
        
        long long sz = reply->header( QNetworkRequest::ContentLengthHeader ).toInt();
        reply->abort();
        delete reply;
        
        return sz;
    }
    
    
    long
    PhotoMetadata::getID()
    {
        return m_id;
    }
    
    std::string PhotoMetadata::getSecret()
    {
        return m_secret;
    }
    
    std::string PhotoMetadata::getServer()
    {
        return m_server;
    }
    
    std::string PhotoMetadata::getTitle()
    {
        return m_title;
    }
    
    bool
    PhotoMetadata::isPrimary()
    {
        return m_isPrimary;
    }

    time_t
    PhotoMetadata::getMTime()
    {
        return m_mtime;
    }
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/



    PhotoSet::PhotoSet( fh_webPhotos wf, fh_context md )
        :
        m_wf( wf )
    {
        m_id      = toType<long long>(getStrAttr( md, "id", "0" ));
        DEBUG << "PhotoSet() photoset-id:" << m_id << endl;
        m_secret  = getStrAttr( md, "secret", "" );
        m_server  = getStrAttr( md, "server", "" );
        m_primary = getStrAttr( md, "primary", "" );
        m_photocount = toint(getStrAttr( md, "photos", "" ));

        m_title  = getStrSubCtx( md, "title", "" );
        m_desc   = getStrSubCtx( md, "description", "" );
    }
        
    PhotoSet::~PhotoSet()
    {
    }
    
    long
    PhotoSet::getID()
    {
        return m_id;
    }
    
    std::string
    PhotoSet::getSecret()
    {
        return m_secret;
    }
    
    std::string
    PhotoSet::getServer()
    {
        return m_server;
    }
    
    std::string
    PhotoSet::getTitle()
    {
        return m_title;
    }
    
    std::string
    PhotoSet::getPrimary()
    {
        return m_primary;
    }
    
    std::string
    PhotoSet::getDescription()
    {
        return m_desc;
    }
    
    photolist_t
    PhotoSet::getPhotos( int page,
                         privacy_t privacy )
    {
        fh_webPhotos wf = m_wf;
        DEBUG << "PhotoSet::getPhotos() photoset-id:" << m_id << endl;
        
        photolist_t ret;
        stringmap_t args;
        args["api_key"] = wf->getAPIKey();
        args["per_page" ] = tostr(500);
        args["page"] = tostr(page);
        args["extras"] = "license,date_upload,date_taken,owner_name,icon_server,original_format,last_update,geo,tags,machine_tags,o_dims,views,media,path_alias,url_sq,url_t,url_s,url_m,url_o";
        args["photoset_id"] = tostr(m_id);
    

        
        fh_context res = wf->signedCall( "flickr.photosets.getPhotos", args );

        fh_context c = res;
        DEBUG << "XXXXXXXXXXXXXXXXXXXXXX1" << endl;
        DEBUG << "name:" << c->getDirName() << endl;
        DEBUG << "XXXXXXXXXXXXXXXXXXXXXX2" << endl;

        if( !c->empty() )
        {
            c = *(c->begin());
        }
        
        Context::iterator ci = c->begin();
        Context::iterator ce = c->end();
        
        LG_WEBPHOTO_D << "getPhotos() sz:" << distance( c->begin(), c->end() ) << endl;
        for( ; ci!=ce; ++ci )
        {
            DEBUG << "name:" << (*ci)->getDirName() << endl;
            LG_WEBPHOTO_D << "Have web photo... id:" << getStrAttr( *ci, "id", "none" ) << endl;
            ret.push_back( new PhotoMetadata( m_wf, *ci ) );
        }

        return ret;
    }
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    WebPhotos::WebPhotos( const std::string& username )
        :
        m_username( username ),
        m_includeEAIsPresentRegex( 0 ),
        m_includeEAandValueRegex( 0 )
    {
        Main::processAllPendingEvents();
        KDE::ensureKDEApplication();
        
//         cerr << "username1:" << m_username << endl;
//         if( m_username.empty() )
//         {
// //            m_username = getUserName();
//             cerr << "username2:" << m_username << endl;
//             if( m_username.empty() )
//                 m_username = WebPhotos::getDefaultUsername();
//         }
//         cerr << "usernameE:" << m_username << endl;
    }
    
    WebPhotos::~WebPhotos()
    {
    }
    
    
    std::string
    WebPhotos::getImplementationShortName()
    {
        cerr << "WebPhotos::getImplementationShortName() this:" << this << endl;
        return "flickr";
    }
    
    std::string
    WebPhotos::getRESTURL()
    {
        std::string url = "http://www.flickr.com/services/rest/";
        return url;
    }

    std::string
    WebPhotos::getUploadURL()
    {
        string earl = "http://api.flickr.com/services/upload/?";
        return earl;
    }

    
    std::string
    WebPhotos::getAuthURL()
    {
        return "http://www.flickr.com/services/auth/";
    }
    

    std::string
    WebPhotos::getPostUploadURLPrefix()
    {
        return "http://www.flickr.com/tools/uploader_edit.gne?ids=";
    }
    

    bool
    WebPhotos::haveAPIKey()
    {
        try
        {
            fh_context apikey = Resolve( "~/.ferris/flickr-api-key.txt" );
            fh_context apisec = Resolve( "~/.ferris/flickr-api-secret.txt" );

            return !getStrAttr( apikey, "content", "" ).empty()
                && !getStrAttr( apisec, "content", "" ).empty();
        }
        catch( ... )
        {
        }
        return false;
    }

    std::string
    WebPhotos::getAPIKey()
    {
        return getStrSubCtx( "~/.ferris/flickr-api-key.txt", "" );
    }

    std::string
    WebPhotos::getAPISecret()
    {
        return getStrSubCtx( "~/.ferris/flickr-api-secret.txt", "" );
    }

    
    std::string
    WebPhotos::getToken()
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_TOKEN_K << "-" << getImplementationShortName() << "-" << getUserName();
        string ret = getEDBString( FDB_SECURE, tostr(ss), CFG_WEBPHOTOS_TOKEN_DEFAULT );

        LG_WEBPHOTO_D << "getting token key:" << ss.str() << endl;
        LG_WEBPHOTO_D << "have token:" << ret << endl;
        return ret;
    }

    
    void
    WebPhotos::setToken( const std::string& token )
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_TOKEN_K << "-" << getImplementationShortName() << "-" << getUserName();
        setEDBString( FDB_SECURE, tostr(ss), token );
        LG_WEBPHOTO_D << "setting token key:" << ss.str() << endl;
    }

    
    bool
    WebPhotos::haveToken()
    {
        return !getToken().empty();
    }

    std::string
    WebPhotos::getDefaultUsername()
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_DEFAULT_USERNAME_KEY << "-" << getImplementationShortName();
        string ret = getEDBString( FDB_SECURE, tostr(ss),
                                   CFG_WEBPHOTOS_DEFAULT_USERNAME_DEFAULT );
        // cerr << "WebPhotos::getDefaultUsername() k:" << ss.str()
        //      << " ret:" << ret << endl;
        // cerr << "rest url:" << getRESTURL() << endl;
        return ret;
    }

    void
    WebPhotos::setDefaultUsername( const std::string& s )
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_DEFAULT_USERNAME_KEY << "-" << getImplementationShortName();
        setEDBString( FDB_SECURE, tostr(ss), s );
    }


    long
    WebPhotos::getDefaultLargestDimension()
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_LARGEST_DIM_K << "-" << getImplementationShortName();
        string ret = getEDBString( FDB_GENERAL, tostr(ss), "0" );
        return toint(ret);
    }
    
    void
    WebPhotos::setDefaultLargestDimension( long v )
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_LARGEST_DIM_K << "-" << getImplementationShortName();
        setEDBString( FDB_GENERAL, tostr(ss), tostr(v) );
    }


    std::string
    WebPhotos::getDefaultIncludeEAIsPresentRegexString()
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_INCLUDE_EA_IS_PRESENT_REGEX_K << "-" << getImplementationShortName();
        string s = getEDBString( FDB_GENERAL, tostr(ss), "" );
        return s;
    }
    
    
    fh_rex
    WebPhotos::getDefaultIncludeEAIsPresentRegex()
    {
        if( m_includeEAIsPresentRegex )
            return m_includeEAIsPresentRegex;

        string s = getDefaultIncludeEAIsPresentRegexString();
        if( s.empty() )
            return 0;

        m_includeEAIsPresentRegex = toregexh( s );
        return m_includeEAIsPresentRegex;
    }
    
    void
    WebPhotos::setDefaultIncludeEAIsPresentRegex( const std::string& s )
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_INCLUDE_EA_IS_PRESENT_REGEX_K << "-" << getImplementationShortName();
        setEDBString( FDB_GENERAL, tostr(ss), s );
        
        if( s.empty() )
            m_includeEAIsPresentRegex = 0;
        else
            m_includeEAIsPresentRegex = toregexh( s );
    }

    std::string
    WebPhotos::getDefaultIncludeEAandValueRegexString()
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_INCLUDE_EA_AND_VALUE_REGEX_K << "-" << getImplementationShortName();
        string s = getEDBString( FDB_GENERAL, tostr(ss), "" );
        return s;
    }
    
    
    fh_rex
    WebPhotos::getDefaultIncludeEAandValueRegex()
    {
        if( m_includeEAandValueRegex )
            return m_includeEAandValueRegex;

        string s = getDefaultIncludeEAandValueRegexString();
        if( s.empty() )
            return 0;

        m_includeEAandValueRegex = toregexh( s );
        return m_includeEAandValueRegex;
    }
    
    void
    WebPhotos::setDefaultIncludeEAandValueRegex( const std::string& s )
    {
//        cerr << "setDefaultIncludeEAandValueRegex() s:" << s << endl;
        stringstream ss;
        ss << CFG_WEBPHOTOS_INCLUDE_EA_AND_VALUE_REGEX_K << "-" << getImplementationShortName();
        setEDBString( FDB_GENERAL, tostr(ss), s );

        if( s.empty() )
            m_includeEAandValueRegex = 0;
        else
            m_includeEAandValueRegex = toregexh( s );
    }
    

    bool
    WebPhotos::isDefaultImageProtectionPublic()
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_DEFAULT_PROTECTION_PUBLIC_K << "-" << getImplementationShortName();
        string ret = getEDBString( FDB_GENERAL, tostr(ss), "0" );
        return isTrue(ret);
    }
    
    void WebPhotos::setDefaultImageProtectionPublic( bool v )
    {
//        cerr << "setDefaultImageProtectionPublic() v:" << v << endl;
        stringstream ss;
        ss << CFG_WEBPHOTOS_DEFAULT_PROTECTION_PUBLIC_K << "-" << getImplementationShortName();
        setEDBString( FDB_GENERAL, tostr(ss), tostr(v) );
    }
    
    bool WebPhotos::isDefaultImageProtectionFriend()
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_DEFAULT_PROTECTION_FRIEND_K << "-" << getImplementationShortName();
        string ret = getEDBString( FDB_GENERAL, tostr(ss), "0" );
        return isTrue(ret);
    }
        
    void WebPhotos::setDefaultImageProtectionFriend( bool v )
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_DEFAULT_PROTECTION_FRIEND_K << "-" << getImplementationShortName();
        setEDBString( FDB_GENERAL, tostr(ss), tostr(v) );
    }
    
        
    bool WebPhotos::isDefaultImageProtectionFamily()
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_DEFAULT_PROTECTION_FAMILY_K << "-" << getImplementationShortName();
        string ret = getEDBString( FDB_GENERAL, tostr(ss), "0" );
        return isTrue(ret);
    }
            
    void WebPhotos::setDefaultImageProtectionFamily( bool v )
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_DEFAULT_PROTECTION_FAMILY_K << "-" << getImplementationShortName();
        setEDBString( FDB_GENERAL, tostr(ss), tostr(v) );
    }
    
            
    
    std::string
    WebPhotos::getUserName()
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_USERNAME_K << "-" << getImplementationShortName();
        string ret = getEDBString( FDB_SECURE, tostr(ss), "" );
        cerr << "WebPhotos::getUserName() k:" << ss.str()
             << " ret:" << ret << endl;
        if( ret.empty() )
        {
            ret = getDefaultUsername();
        }
        
        m_username = ret;
        return ret;
    }
    
    std::string
    WebPhotos::getFullName()
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_FULLNAME_K << "-" << getImplementationShortName();
        string ret = getEDBString( FDB_SECURE, tostr(ss), "" );
        return ret;
    }
    


    void
    WebPhotos::setUserName( const std::string& s )
    {
        m_username = s;
        stringstream ss;
        ss << CFG_WEBPHOTOS_USERNAME_K << "-" << getImplementationShortName();
        setEDBString( FDB_SECURE, tostr(ss), s );
    }
    
    void
    WebPhotos::setFullName( const std::string& s )
    {
        stringstream ss;
        ss << CFG_WEBPHOTOS_FULLNAME_K << "-" << getImplementationShortName();
        setEDBString( FDB_SECURE, tostr(ss), s );
    }
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    fh_context
    WebPhotos::handleCallIO( stringmap_t args )
    {
        std::string url = getRESTURL();
        
        
        fh_context ret = 0;
        stringstream callss;
        callss << url << "?";
        for( stringmap_t::iterator iter = args.begin(); iter != args.end(); ++iter )
        {
            callss << iter->first << "=" << iter->second << "&";
        }

        string callstring = callss.str();
        LG_WEBPHOTO_W << "call string:" << callstring << endl;

        try
        {
            fh_context c = Resolve( callstring );
            string content = getStrAttr( c, "content", "<error/>", true, true );
            LG_WEBPHOTO_W << "raw result:" << content << endl;

            fh_domdoc doc = Factory::StringToDOM( content );
            // {
            //     fh_stringstream ss = tostream( doc );
            //     DEBUG << "to DOM and back to string..." << endl;
            //     DEBUG << StreamToString( ss ) << endl;
            // }
            
            DOMElement* root = doc->getDocumentElement();
            std::string statcode = getAttribute( root, "stat" );
            if( statcode != "" && statcode != "ok" )
            {
                stringstream ss;
                ss << "Error. Data from server:" << content << endl;
                LG_WEBPHOTO_W << tostr(ss);
                Throw_WebPhotoException( tostr(ss), 0 );
            }

            ret = Factory::mountDOM( doc );
            if( !ret->isSubContextBound( "rsp" ) )
            {
                stringstream ss;
                ss << "Error. No RSP element at top level of data." << endl
                   << "Data from server:" << tostr( doc, true ) << endl;
                LG_WEBPHOTO_W << tostr(ss);
                Throw_WebPhotoException( tostr(ss), 0 );
            }
            ret = ret->getSubContext( "rsp" );
            doc = 0;
            
            return ret;
        }
        catch( exception& e )
        {
            LG_WEBPHOTO_W << "e:" << e.what() << endl;
            throw;
        }

        return ret;
    }

    std::string
    WebPhotos::makeSignedString( stringmap_t args )
    {
        stringstream ss;
        ss << getAPISecret();
        for( stringmap_t::iterator iter = args.begin(); iter != args.end(); ++iter )
        {
            ss << iter->first << iter->second;
        }
        LG_WEBPHOTO_W << "signed_string() args:" << ss.str() << endl;
        std::string md5string = digest( ss.str(), "md5" );
        return md5string;
    }

    
    fh_context
    WebPhotos::signedCall( std::string methodname,
                           stringmap_t args,
                           bool dontAutoIncludeToken )
    {
        if( !dontAutoIncludeToken )
        {
            string token = getToken();
            args[ "auth_token" ] = token;
            LG_WEBPHOTO_W << "Adding token:" << token << endl;
        }
        args["method"]  = methodname;
        args["api_key"] = getAPIKey();
        LG_WEBPHOTO_W << "methodname:" << methodname << endl;
        
        string md5string = makeSignedString( args );

        {
            stringmap_t::iterator ai = args.find("comment_text");
            if( ai != args.end() )
            {
                string v = ai->second;
                DEBUG << "comment_text:" << v << endl;
                string vc = tostr(QUrl::toPercentEncoding( v.c_str() ));
                args["comment_text"] = vc;
            }
        }
        
        stringmap_t callargs = args;
        callargs.insert( make_pair( "api_sig", md5string ) );
        fh_context doc = handleCallIO( callargs );
        return doc;
    }

//     fh_context
//     WebPhotos::SignedCall( std::string methodname, stringmap_t args )
//     {
//         return signedCall( getDefaultUsername(), methodname, args );
//     }

    photolist_t
    WebPhotos::getMyRecent( time_t tt, int pagenum, int limit )
    {
        photolist_t ret;

        tt = Time::getTime() - (30*24*3600);
        
        stringmap_t args;
        args["api_key"] = getAPIKey();
        args["min_date"] = tostr(tt);
        args["per_page" ] = tostr(limit);
        args["page"] = tostr(pagenum);
        args["extras"] = "date_upload,date_taken,original_format,geo,tags,machine_tags";

        fh_context res = signedCall( "flickr.photos.recentlyUpdated", args );

        fh_context c = res;
        if( c->isSubContextBound("photos") )
            c = c->getSubContext("photos");

        Context::iterator ci = c->begin();
        Context::iterator ce = c->end();
        
        LG_WEBPHOTO_D << "getMyRecent() sz:" << distance( res->begin(), res->end() ) << endl;
        for( ; ci!=ce; ++ci )
        {
            LG_WEBPHOTO_D << "Have web photo... id:" << getStrAttr( *ci, "id", "none" ) << endl;
            ret.push_back( new PhotoMetadata( this, *ci ) );
        }

        return ret;
    }

    string spaces( int indent )
    {
        stringstream ss;
        for( int i=0; i<indent; ++i )
            ss << " ";
        return ss.str();
    }

    void dumptree( fh_context c, int offset = 0 )
    {
        DEBUG << spaces(offset) << " c:" << c->getDirName() << endl;
        Context::iterator ci = c->begin();
        Context::iterator ce = c->end();
        for( ; ci!=ce; ++ci )
        {
            dumptree( *ci, offset+4 );
        }
    }
    
    photolist_t
    WebPhotos::getNotInSet( int page,
                            privacy_t privacy,
                            time_t min_upload_date, time_t max_upload_date,
                            time_t min_taken_date,  time_t max_taken_date )
    {
        photolist_t ret;
        stringmap_t args;
        args["api_key"] = getAPIKey();
        if( min_upload_date )
            args["min_upload_date"] = tostr(min_upload_date);
        if( max_upload_date )
            args["max_upload_date"] = tostr(max_upload_date);
        args["per_page" ] = tostr(500);
        args["page"] = tostr(page);
        args["extras"] = "license,date_upload,date_taken,owner_name,icon_server,original_format,last_update,geo,tags,machine_tags,o_dims,views,media,path_alias,url_sq,url_t,url_s,url_m,url_o";

        fh_context res = signedCall( "flickr.photos.getNotInSet", args );

        dumptree(res);
        
        fh_context c = res;
        if( c->isSubContextBound("photos") )
            c = c->getSubContext("photos");
        
        
        Context::iterator ci = c->begin();
        Context::iterator ce = c->end();
        
        DEBUG << "getNotInSet() sz:" << distance( c->begin(), c->end() ) << endl;
        DEBUG << "name:" << c->getDirName() << endl;
        for( ; ci!=ce; ++ci )
        {
            LG_WEBPHOTO_D << "Have web photo... id:" << getStrAttr( *ci, "id", "none" ) << endl;
            ret.push_back( new PhotoMetadata( this, *ci ) );
        }

        DEBUG << "WebPhotos::getNotInSet() return sz:" << ret.size() << endl;
        return ret;
    }
    
    photolist_t
    WebPhotos::getNotInSet( int page, privacy_t privacy )
    {
        return getNotInSet( page, privacy, 0, 0, 0, 0 );
    }
    
    photosets_t
    WebPhotos::getPhotosets( const std::string uid )
    {
        stringmap_t args;
        args["api_key"] = getAPIKey();
        if( !uid.empty() )
            args["user_id"] = uid;

        fh_context res = signedCall( "flickr.photosets.getList", args );

        fh_context c = res;
        if( c->isSubContextBound("photosets") )
            c = c->getSubContext("photosets");

        Context::iterator ci = c->begin();
        Context::iterator ce = c->end();

        photosets_t ret;
        LG_WEBPHOTO_D << "sz:" << distance( res->begin(), res->end() ) << endl;
        for( ; ci!=ce; ++ci )
        {
            LG_WEBPHOTO_D << "Have web photoset... id:" << getStrAttr( *ci, "id", "none" ) << endl;
            ret.push_back( new PhotoSet( this, *ci ) );
        }

        return ret;
    }

    photolist_t
    WebPhotos::getFavoritePhotos( const std::string uid )
    {
        stringmap_t args;
        args["api_key"] = getAPIKey();
        if( !uid.empty() )
            args["user_id"] = uid;
        args["extras"] = "license,date_upload,date_taken,owner_name,icon_server,original_format,last_update,geo,tags,machine_tags,o_dims,views,media,path_alias,url_sq,url_t,url_s,url_m,url_o";
        args["per_page"] = "500";
        
        fh_context res = signedCall( "flickr.favorites.getList", args );
        fh_context c = res;
        if( c->isSubContextBound("photos") )
            c = c->getSubContext("photos");

        Context::iterator ci = c->begin();
        Context::iterator ce = c->end();
        photolist_t ret;
        LG_WEBPHOTO_D << "getFavoritePhotos() sz:" << distance( c->begin(), c->end() ) << endl;
        for( ; ci!=ce; ++ci )
        {
            LG_WEBPHOTO_D << "Have web photo... id:" << getStrAttr( *ci, "id", "none" ) << endl;
            ret.push_back( new PhotoMetadata( this, *ci ) );
        }

        return ret;
        
    }
    
    
    fh_PhotoMetadata
    WebPhotos::getPhotoByID( const std::string photo_id )
    {
        stringmap_t args;
        args["api_key"] = getAPIKey();
        args["photo_id"] = photo_id;
        fh_context res = signedCall( "flickr.photos.getInfo", args );
        fh_context c = res;
        if( !c->empty() )
        {
            c = *(c->begin());
        }
        LG_WEBPHOTO_D << "Have web photo... id:" << getStrAttr( c, "id", "none" ) << endl;
        fh_PhotoMetadata ret = new PhotoMetadata( this, c );
        return ret;
    }


    ContactList_t
    WebPhotos::getContacts()
    {
        if( !m_contactlist.empty() )
            return m_contactlist;
        
        stringmap_t args;
        fh_context res = signedCall( "flickr.contacts.getList", args );
        fh_context c = res;
        if( !c->empty() )
        {
            c = *(c->begin());
        }

        Context::iterator ci = c->begin();
        Context::iterator ce = c->end();
        for( ; ci!=ce; ++ci )
        {
            fh_context md = *ci;
            fh_Contact z = new Contact( this, md );
            m_contactlist.push_back(z);
        }
        return m_contactlist;
    }
    
    
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    WebPhotoUpload::WebPhotoUpload( fh_webPhotos wf )
        :
        m_wf( wf ),
        m_debugVerbose( false ),
        m_includeEAisPresentRegex( 0 ),
        m_includeEAandValueRegex( 0 ),
        m_publicViewable( false ),
        m_FriendViewable( false ),
        m_FamilyViewable( false ),
        m_largestDimensionExplicit( -1 )
    {
        m_verboseStream = Factory::fcnull();
    }

    WebPhotoUpload::~WebPhotoUpload()
    {
    }

    void
    WebPhotoUpload::setLargestDimensionExplicit( long v )
    {
        m_largestDimensionExplicit = v;
    }


    void WebPhotoUpload::setPublicViewable( bool v )
    {
        m_publicViewable = v;
    }

    void WebPhotoUpload::setFriendViewable( bool v )
    {
        m_FriendViewable = v;
    }

    void WebPhotoUpload::setFamilyViewable( bool v )
    {
        m_FamilyViewable = v;
    }

    void
    WebPhotoUpload::setDescriptionFromEA( const std::string& s )
    {
        m_DescriptionFromEAList.clear();
        m_DescriptionFromEAList.push_back(s);
    }

    void
    WebPhotoUpload::appendDescriptionFromEA( const std::string& s )
    {
        m_DescriptionFromEAList.push_back(s);
    }

    void
    WebPhotoUpload::setIncludeEAisPresentRegex( fh_rex r )
    {
        m_includeEAisPresentRegex = r;
    }

    void
    WebPhotoUpload::setIncludeEAandValueRegex( fh_rex r )
    {
        m_includeEAandValueRegex = r;
    }


    void
    WebPhotoUpload::setVerboseStream( fh_ostream v )
    {
        m_verboseStream = v;
    }

    void
    WebPhotoUpload::setDebugVerbose( bool v )
    {
        m_debugVerbose = v;
    }



    stringlist_t&
    WebPhotoUpload::getUploadedPhotoIDList()
    {
        return m_uploadedPhotoIDList;
    }


    string
    WebPhotoUpload::getTagString( fh_context c )
    {
        fh_rex includeEAisPresentRegex = m_includeEAisPresentRegex;
        fh_rex includeEAandValueRegex  = m_includeEAandValueRegex;
    
        if( !includeEAisPresentRegex )
            includeEAisPresentRegex = m_wf->getDefaultIncludeEAIsPresentRegex();
        if( !includeEAandValueRegex )
            includeEAandValueRegex = m_wf->getDefaultIncludeEAandValueRegex();

        typedef AttributeCollection::AttributeNames_t an_t;
        an_t an;
        if( includeEAisPresentRegex || includeEAandValueRegex )
            c->getAttributeNames( an );

        stringstream tagSS;
        if( includeEAisPresentRegex )
        {
            for( an_t::iterator ai = an.begin(); ai!=an.end(); ++ai )
            {
                if( regex_match( *ai, includeEAisPresentRegex ) )
                {
                    if( isTrue( getStrAttr( c, *ai, "0" ) ))
                    {
                        tagSS << *ai << " ";
                    }
                }
            }
        }
        if( includeEAandValueRegex )
        {
            for( an_t::iterator ai = an.begin(); ai!=an.end(); ++ai )
            {
//                cerr << "ai:" << *ai << endl;
                if( regex_match( *ai, includeEAandValueRegex ) )
                {
                    tagSS << *ai << "=" << getStrAttr( c, *ai, "" ) << " ";
                }
            }
        }

        LG_WEBPHOTO_D << "tags:" << tagSS.str() << endl;
        return tagSS.str();
    }

    template< class T >
    T copyTo( fh_istream iss, T oss )
    {
        copy( istreambuf_iterator<char>(iss),
              istreambuf_iterator<char>(),
              ostreambuf_iterator<char>(oss));
        oss << flush;
        return oss;
    }

    bool
    WebPhotoUpload::adjustWidthAndHeightForDesired(
        int width,
        int height,
        int& w,
        int& h,
        long MaxDesiredWidthOrHeight )
    {
        double ratio = MaxDesiredWidthOrHeight;
        w = h = 0;

        if( width > 0 && height > 0 && height > width )
        {
            ratio /= height;
        }
        if( w < MaxDesiredWidthOrHeight && h < MaxDesiredWidthOrHeight )
            return false;
        
        if( w > MaxDesiredWidthOrHeight )
        {
            ratio = MaxDesiredWidthOrHeight;
            ratio /= w;
            w = MaxDesiredWidthOrHeight;
            h = (int)( ratio * h );
        }
        if( h > MaxDesiredWidthOrHeight )
        {
            ratio = MaxDesiredWidthOrHeight;
            ratio /= h;
            w = (int)( ratio * w );
            h = MaxDesiredWidthOrHeight;
        }
        return true;
    }
    
    
    fh_context
    WebPhotoUpload::makeScaledImage( fh_context c )
    {
        
        bool m_antialias = 1;
        long MaxDesiredWidthOrHeight = 0;

        if( m_wf )
            MaxDesiredWidthOrHeight = m_wf->getDefaultLargestDimension();
        if( m_largestDimensionExplicit >= 0 )
            MaxDesiredWidthOrHeight = m_largestDimensionExplicit;

        LG_WEBPHOTO_D << "MaxDesiredWidthOrHeight:" << MaxDesiredWidthOrHeight << endl;
        if( m_wf )
            LG_WEBPHOTO_D << "wf->getDefaultLargestDimension():" << m_wf->getDefaultLargestDimension() << endl;
        LG_WEBPHOTO_D << "m_largestDimensionExplicit:" << m_largestDimensionExplicit << endl;
        LG_WEBPHOTO_D << "c:" << c->getURL() << endl;

        if( !MaxDesiredWidthOrHeight )
        {
            LG_WEBPHOTO_D << "No scaling needed. returning c:" << c->getURL() << endl;
            return c;
        }

        bool LIBFERRIS_USE_INTERNAL_SCALE_IMAGE_BY_DIMENSION = ( g_getenv ("LIBFERRIS_USE_INTERNAL_SCALE_IMAGE_BY_DIMENSION" ));
        string ext = getStrAttr( c, "name-extension",  "" );
        if( ext == "CR2" || ext == "cr2" )
        {
            LIBFERRIS_USE_INTERNAL_SCALE_IMAGE_BY_DIMENSION = true;
        }
        cerr << "ext:" << ext << endl;
        cerr << "c:" << c->getURL() << endl;
        
        if( LIBFERRIS_USE_INTERNAL_SCALE_IMAGE_BY_DIMENSION )
        {
            //
            // gm convert -resize 320  - - >|o
            //
            LG_WEBPHOTO_D << "Scaling image with ferris-internal-scale-image-by-dimension.sh script" << endl;
            
            fh_context img = c;
            string earl = c->getURL();
            string dst_url = Shell::getTmpDirPath() + "/libferris-webupload-temp-image.jpg";
            int DesiredWidth = MaxDesiredWidthOrHeight;

            string ext = getStrAttr( img, "name-extension",  "" );
            int width  = toint( getStrAttr( img, "width",  "-1" ));
            int height = toint( getStrAttr( img, "height", "-1" ));
            if( width > 0 && height > 0 && height > width )
            {
                double ratio = MaxDesiredWidthOrHeight;
                ratio /= height;
                DesiredWidth = (int)( ratio * width );
            }
            
            fh_runner r = new Runner();
            r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags()
                                           | G_SPAWN_STDERR_TO_DEV_NULL
                                           | G_SPAWN_DO_NOT_REAP_CHILD
                                           | G_SPAWN_SEARCH_PATH) );
//            r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags() & ~(G_SPAWN_DO_NOT_REAP_CHILD)));

            stringstream cmdliness;
            cmdliness << "ferris-internal-scale-image-by-dimension.sh"
                      << " " << MaxDesiredWidthOrHeight
                      << " " << ext;
            r->setCommandLine( cmdliness.str() );
            r->setConnectStdIn( true );
            LG_WEBPHOTO_D << "command:" << cmdliness.str() << endl;
            r->Run();
            {
                LG_WEBPHOTO_D << "copying image data..img:" << img->getURL() << endl;
                copyTo( img->getIStream(), r->getStdIn() );
            }
            {
                fh_context dst = Shell::acquireContext( dst_url, 0, false );
                copyTo( r->getStdOut(), dst->getIOStream( ios::trunc | ios::out ) );
            }
            
            
            LG_WEBPHOTO_D << "waiting..." << endl;
            gint e = r->getExitStatus();
            LG_WEBPHOTO_D << "done with conversion..." << endl;
            
            fh_context ret = Resolve(  dst_url );
            LG_WEBPHOTO_D << "dst size:" << getStrAttr( ret, "size", "unknown" ) << endl;
            return ret;
        }

        
        
#ifndef HAVE_IMLIB2
        cerr << "Imlib2 is required to perform image resizing!" << endl;
        cerr << "you must rebuild your libferris :(" << endl;
        return c;
#else

        LG_WEBPHOTO_D << "Scaling image at:" << c->getURL() << endl;
        string dst_url = Shell::getTmpDirPath() + "/libferris-webupload-temp-image.jpg";
        string m_imageDataCache;
        fh_context img = c;
        string earl = c->getURL();
        string imlibpath = c->getURL();
        imlib_context_set_anti_alias( m_antialias );
        Imlib_Load_Error ierr;
        if( starts_with( earl, "file:" ) )
            imlibpath = c->getDirPath();
        Imlib_Image image = imlib_load_image_with_error_return( earl.c_str(), &ierr );
        if( !image )
        {
            LG_WEBPHOTO_D << "trying to load image with libferris plugins... URL:" << earl << endl;
            /*
             * Attempt to load the image using the rgba-data plugins of libferris
             */
            int width  = toint( getStrAttr( img, "width",  "-1" ));
            int height = toint( getStrAttr( img, "height", "-1" ));
        
            m_imageDataCache = getStrAttr( img, "rgba-32bpp", "", true, false );
            if( m_imageDataCache.empty() || width == -1 || height == -1 )
            {
                LG_WEBPHOTO_W << "Could not load image at:" << earl << " reason:" << ierr << endl;
                cerr << "Could not load image at:" << earl << " reason:" << ierr << endl;
                cerr << "  data.sz:" << m_imageDataCache.size() << endl;
                cerr << "  width:"  << width  << endl;
                cerr << "  height:" << height << endl;
                return 0;
            }

            image = imlib_create_image_using_data(
                width, height, (unsigned int*)m_imageDataCache.data() );
        }

        imlib_context_set_image( image );

        string exiforient = getStrAttr( img, "exif:orientation", "" );
        LG_WEBPHOTO_D << "exiforient:"  << exiforient << endl;
        LG_WEBPHOTO_D << "exif orientation string:" << exiforient << endl;
        if( !exiforient.empty() )
        {
            int spin = toint(exiforient);
            if( spin > 1 && spin < 10 )
            {
                LG_WEBPHOTO_D << "exif orientation flag:" << spin << endl;
                Imlib_Image ni = imlib_clone_image();
                imlib_free_image();
                imlib_context_set_image( ni );
                switch( spin )
                {
                    case 6:  // rotate 90
                        imlib_image_orientate(1);                        
                        break;
                    case 3: // rotate 180
                        imlib_image_orientate(2);
                        break;
                    case 8: // rotate 270
                        imlib_image_orientate(3);
                        break;
                    case 2:
                        imlib_image_flip_horizontal();
                        break;
                    case 4:
                        imlib_image_flip_vertical();
                        break;
                    case 5:  // rotate 90, transpose
                        imlib_image_orientate(1);
                        imlib_image_flip_horizontal();
                        break;
                    case 7:  // rotate 270, transverse
                        imlib_image_orientate(1);
                        imlib_image_flip_horizontal();
                        break;
                }
            }
        }

        int w, h;
        if( !adjustWidthAndHeightForDesired( imlib_image_get_width(),
                                             imlib_image_get_height(),
                                             w, h, MaxDesiredWidthOrHeight ))
        {
            LG_WEBPHOTO_D << "Scaling image with Imlib2, no scaling needed.... img:" << earl << endl;
            return c;
        }

        Imlib_Image out_image =
            imlib_create_cropped_scaled_image(0, 0,
                                              imlib_image_get_width(),
                                              imlib_image_get_height(),
                                              w, h);
        imlib_free_image();
        imlib_context_set_image(out_image);
    
    
        imlib_image_set_format( "jpg" );
        imlib_save_image( dst_url.c_str() );
        imlib_free_image();

        fh_context ret = Resolve(  dst_url );
        return ret;
#endif
    }

    void
    WebPhotoUpload::throwIfStatusIsBad( fh_context contextMaybeNull, const std::string& res )
    {
        string earl = "<unknown>";
        if( contextMaybeNull )
            earl = contextMaybeNull->getURL();
        
        fh_domdoc doc = Factory::StringToDOM( res );
        DOMElement* root = doc->getDocumentElement();
        std::string statcode = getAttribute( root, "stat" );
        if( statcode != "" && statcode != "ok" )
        {
            cerr << "processing file:" << earl << endl;
            cerr << "Error. Data from server:" << res << endl;

            stringstream ss;
            ss << "Failed to upload image from:" << earl << endl;
            ss << "raw reason:" << res << endl;
            Throw_WebPhotoException( tostr(ss), 0 );
        }
    }
    
    
    void
    WebPhotoUpload::post( stringmap_t args )
    {
        fh_webPhotos wf = m_wf;
        string earl = wf->getRESTURL();

        FerrisCurl cu;
        m_verboseStream << "Setting DebugVerbose to:" << m_debugVerbose << endl;
        cu.setDebugVerbose( m_debugVerbose );

        args["api_key"] = wf->getAPIKey();
        args["auth_token"] = wf->getToken();
        string sig = wf->makeSignedString( args );
        args["api_sig"] = sig;

        LG_WEBPHOTO_D << "api_key:" << wf->getAPIKey() << endl;
        LG_WEBPHOTO_D << "sig:" << sig << endl;

        cu.setHttpAccept("Content-Type: application/xml");

        m_verboseStream << "Sending POST...to url:" << earl << endl;

        cu.setMultiPartPostData( args );
        string res = cu.post( earl, "" );

        m_verboseStream << "Have result:" << res << endl;
        throwIfStatusIsBad( 0, res );
    }

    void
    WebPhotoUpload::streamingUploadComplete()
    {
        LG_WEBPHOTO_D << "WebPhotoUpload::streamingUploadComplete()" << endl;
        
        QNetworkReply* reply = m_reply;
        m_streamToQIO->writingComplete();
        QByteArray ba = m_streamToQIO->readResponse();
        string res = tostr(ba);
        LG_WEBPHOTO_D << "reply->error:" << reply->error() << endl;
        LG_WEBPHOTO_D << "result:" << res << endl;

        m_verboseStream << "Have result:" << res << endl;
        throwIfStatusIsBad( 0, res );
        
        //<rsp stat="ok">
        //  <photoid>438542969</photoid>
        //</rsp>
    
        fh_domdoc doc = Factory::StringToDOM( res );
        DOMElement* root = doc->getDocumentElement();
        if( DOMElement* e = XML::getChildElement( root, "photoid" ) )
        {
            string id = XML::getChildText( e );
            LG_WEBPHOTO_D   << "uploaded id:" << id << endl;
            m_verboseStream << "uploaded id:" << id << endl;
            m_uploadedPhotoIDList.push_back(id);
        }
    }

    void
    WebPhotoUpload::inspectSourceForMetadataDescriptionAttribute( stringmap_t& args, fh_context c )
    {
        LG_WEBPHOTO_D << "inspectSourceForMetadataDescriptionAttribute() c:" << c->getURL() << endl;
        for( stringlist_t::iterator si = m_DescriptionFromEAList.begin();
             si != m_DescriptionFromEAList.end(); ++si )
        {
            string v = getStrAttr( c, *si, "", true );
            if( !v.empty() )
            {
                LG_WEBPHOTO_D << "have description from EA:" << *si << " v:" << v << endl;
                LG_WEBPHOTO_D << " title:" << args["title"] << endl;
                args["description"] = v;
                break;
            }
        }
    }
    
    void
    WebPhotoUpload::inspectSourceForMetadata( fh_context c )
    {
        LG_WEBPHOTO_D << "inspectSourceForMetadata() c:" << c->getURL() << endl;
        m_args.clear();
        stringmap_t& args = m_args;

        args["tags"] = getTagString( c );
        inspectSourceForMetadataDescriptionAttribute( args, c );
        m_title = getStrAttr( c, "title", "" );
        m_filename = c->getDirName();
    }
    
    
    fh_iostream
    WebPhotoUpload::getUploadIOStream( int ContentLength, const std::string& title, const std::string& desc )
    {
        fh_webPhotos wf = m_wf;
        m_verboseStream << "Uploading image from <stream>"
                        << " explicit title:" << title
                        << " explicit desc:" << desc
                        << endl;
        LG_WEBPHOTO_D << "Uploading image from <stream>"
                      << " explicit title:" << title
                      << " explicit desc:" << desc
                      << " tags:" << m_args["tags"]
                      << endl;
        LG_WEBPHOTO_D  << "pub:" << m_publicViewable
                       << " fri:" << m_FriendViewable
                       << " fam:" << m_FamilyViewable
                       << endl;
        m_verboseStream << "Setting DebugVerbose to:" << m_debugVerbose << endl;


        stringmap_t& args = m_args;
        args["api_key"] = wf->getAPIKey();
        args["is_public"] = tostr(m_publicViewable);
        args["is_friend"] = tostr(m_FriendViewable);
        args["is_family"] = tostr(m_FamilyViewable);
        args["auth_token"] = wf->getToken();
        
        LG_WEBPHOTO_D << "before adding tags, args:"
                      << Util::CreateKeyValueString( args ) << endl;
        if( !m_title.empty() )  args["title"] = m_title;
        if( !title.empty() && title != "title" )
        {
            args["title"] = title;
        }
        if( !desc.empty() && desc != "desc" )
        {
            args["description"] = desc;
        }
        if( !m_args["tags"].empty() )
            args["tags"] = m_args["tags"];
        LG_WEBPHOTO_D << "after adding tags, args:"
                      << Util::CreateKeyValueString( args ) << endl;
        string sig = wf->makeSignedString( args );
        args["api_sig"] = sig;
        
        LG_WEBPHOTO_D << "api_key:" << wf->getAPIKey() << endl;
        LG_WEBPHOTO_D << "sig:" << sig << endl;
                
        string earl = wf->getUploadURL();
        m_verboseStream << "Sending upload POST...to url:" << earl << endl;
        LG_WEBPHOTO_D << "Sending upload POST...to url:" << earl << endl;

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        std::string filename = "filename.jpg";
        if( !m_filename.empty() )
            filename = m_filename;
        std::string contentType = filenameToContextType( filename );
        string uploadFilename = filename;

        QNetworkAccessManager* qm = getQNonCachingManager();
        QUrl u( earl.c_str() );
        addQueryItems( u, args );
        QNetworkRequest req(u);

        LG_WEBPHOTO_D << "+++ ContentLength:" << ContentLength << endl;
        if( ContentLength )
            req.setHeader(QNetworkRequest::ContentLengthHeader, tostr(ContentLength).c_str() );

        // req.setRawHeader("Cache-Control", "no-cache" );
        // req.setRawHeader("Pragma",  "no-cache" );

        
        m_streamToQIO = Factory::createStreamToQIODevice();
        stringstream blobss;
        blobss << "Content-Disposition: form-data; name=\"photo\"; filename=\"" << uploadFilename << "\"" << "\n"
               << "Content-Type: " << contentType;
        QNetworkReply* reply = m_streamToQIO->post( qm, req, blobss.str() );
        m_reply = reply;
        fh_iostream oss = m_streamToQIO->getStream();
        return oss;
    }
    
    void
    WebPhotoUpload::upload( fh_context c, const std::string& title, const std::string& desc )
    {
        fh_webPhotos wf = m_wf;
        m_verboseStream << "Uploading image from:" << c->getURL()
                        << " explicit title:" << title
                        << " explicit desc:" << desc
                        << endl;
        LG_WEBPHOTO_D << "Uploading image from:" << c->getURL()
                      << " explicit title:" << title
                      << " explicit desc:" << desc
                      << endl;
        LG_WEBPHOTO_D  << "pub:" << m_publicViewable
                       << " fri:" << m_FriendViewable
                       << " fam:" << m_FamilyViewable
                       << endl;
    
        FerrisCurl cu;
        m_verboseStream << "Setting DebugVerbose to:" << m_debugVerbose << endl;
        cu.setDebugVerbose( m_debugVerbose );
                
        stringmap_t args;
        args["api_key"] = wf->getAPIKey();
        args["is_public"] = tostr(m_publicViewable);
        args["is_friend"] = tostr(m_FriendViewable);
        args["is_family"] = tostr(m_FamilyViewable);
        args["auth_token"] = wf->getToken();
        LG_WEBPHOTO_D << "before adding tags, args:"
                      << Util::CreateKeyValueString( args ) << endl;
        LG_WEBPHOTO_D << "moving on..." << endl;
        if( !title.empty() && title != "title" )
        {
            args["title"] = title;
        }
        if( !desc.empty() && desc != "desc" )
        {
            args["description"] = desc;
        }
        args["tags"] = getTagString( c );
        inspectSourceForMetadataDescriptionAttribute( args, c );
        string sig = wf->makeSignedString( args );
        args["api_sig"] = sig;
        fh_context scaledc = makeScaledImage( c );
        if( !scaledc )
        {
            stringstream ss;
            ss << "Failed to scale image from:" << c->getURL() << endl;
            Throw_WebPhotoException( tostr(ss), 0 );
        }
        LG_WEBPHOTO_D << "Adding to upload real scaledc:" << scaledc->getURL() << endl;
        LG_WEBPHOTO_D << "Adding to upload real scaledc size:" << getStrAttr( scaledc, "size", "unknown" ) << endl;
        LG_WEBPHOTO_D << "Adding to upload real scaledc dirname:" << c->getDirName() << endl;
        cu.appendFileToUpload( scaledc, c->getDirName(), "", "photo" );

        LG_WEBPHOTO_D << "api_key:" << wf->getAPIKey() << endl;
        LG_WEBPHOTO_D << "sig:" << sig << endl;
                
        cu.setHttpAccept("Content-Type: application/xml");
        string earl = wf->getUploadURL();

        m_verboseStream << "Sending upload() POST...to url:" << earl << endl;
        LG_WEBPHOTO_D   << "Sending upload() POST...to url:" << earl << endl;

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
//        cu.setMultiPartPostData( args );
//        string res = cu.post( earl, "" );

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        fh_context uploadFile = scaledc;
        std::string filename = c->getDirName();
        std::string contentType = "image/jpeg";
        int ContentLength = toint( getStrAttr( uploadFile, "size", "1" ));
        LG_WEBPHOTO_D << "ContentLength:" << ContentLength << endl;

        
        QNetworkAccessManager* qm = new QNetworkAccessManager(0);
//        QNetworkAccessManager* qm = getQManager();
        QUrl u( earl.c_str() );
        addQueryItems( u, args );
        QNetworkRequest req(u);
        cerr << "+++ ContentLength:" << ContentLength << endl;
        req.setHeader(QNetworkRequest::ContentLengthHeader, tostr(ContentLength).c_str() );
        fh_StreamToQIODevice streamToQIO = Factory::createStreamToQIODevice();
        stringstream blobss;
        blobss << "Content-Disposition: form-data; name=\"photo\"; filename=\"" << filename << "\"" << "\n"
               << "Content-Type: " << contentType;
        QNetworkReply* reply = streamToQIO->post( qm, req, blobss.str() );
        {
            fh_ostream oss = streamToQIO->getStream();
            fh_istream iss = uploadFile->getIStream();
            std::copy( std::istreambuf_iterator<char>(iss),
                       std::istreambuf_iterator<char>(),
                       std::ostreambuf_iterator<char>(oss));
            oss << flush;
        }
        streamToQIO->writingComplete();
        QByteArray ba = streamToQIO->readResponse();
        cerr << "reply->error:" << reply->error() << endl;
        string res = tostr(ba);
        if( 99 == reply->error() )
            cerr << "reply->result:" << res << endl;
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        
        m_verboseStream << "Have result:" << res << endl;
        throwIfStatusIsBad( c, res );
        
        //<rsp stat="ok">
        //  <photoid>438542969</photoid>
        //</rsp>
    
        fh_domdoc doc = Factory::StringToDOM( res );
        DOMElement* root = doc->getDocumentElement();
        if( DOMElement* e = XML::getChildElement( root, "photoid" ) )
        {
            string id = XML::getChildText( e );
            LG_WEBPHOTO_D   << "uploaded id:" << id << endl;
            m_verboseStream << "uploaded id:" << id << endl;
            m_uploadedPhotoIDList.push_back(id);
        }
    }

    string
    WebPhotoUpload::getPostUploadURL()
    {
        cerr << "m_uploadedPhotoIDList.sz:" << m_uploadedPhotoIDList.size() << endl;
        stringstream ss;
        ss << m_wf->getPostUploadURLPrefix();
        ss << Util::createSeperatedList( m_uploadedPhotoIDList );
        return ss.str();
    }
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    class WebPhotos23hq
        :
        public WebPhotos
    {
    protected:

        virtual std::string getImplementationShortName()
            {
                return "23hq";
            }
        
        virtual std::string getRESTURL()
            {
                std::string url = "http://www.23hq.com/services/rest/";
                return url;
            }
        
    public:

        string getUploadURL()
            {
                string earl = "http://www.23hq.com/services/upload/?";
                return earl;
            }

        string getAuthURL()
            {
                return "http://www.23hq.com/services/auth/";
            }
    
        
        
        virtual std::string getPostUploadURLPrefix()
            {
                return "http://www.23hq.com/tools/uploader_edit.gne?ids=";
            }
        
        virtual std::string getAPIKey()
            {
                return getStrSubCtx( "~/.ferris/23hq-api-key.txt", "" );
            }
        
        virtual std::string getAPISecret()
            {
                return getStrSubCtx( "~/.ferris/23hq-api-secret.txt", "" );
            }
        
        
    public:

        WebPhotos23hq( const std::string& username )
            :
            WebPhotos( username )
            {
            }
        
        virtual ~WebPhotos23hq()
            {}
        
    };



    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    class WebPhotosZooomr
        :
        public WebPhotos
    {
    protected:

        virtual std::string getImplementationShortName()
            {
                return "zooomr";
            }
        
        virtual std::string getRESTURL()
            {
                std::string url = "http://beta.zooomr.com/bluenote/api/rest/";
                return url;
            }
        
    public:

        string getUploadURL()
            {
                string earl = "http://beta.zooomr.com/bluenote/api/upload/?";
                return earl;
            }

        string getAuthURL()
            {
                return "http://beta.zooomr.com/auth/";
            }
    
        
        
        virtual std::string getPostUploadURLPrefix()
            {
                return "http://beta.zooomr.com/tools/uploader_edit.gne?ids=";
            }
        
        virtual std::string getAPIKey()
            {
                return getStrSubCtx( "~/.ferris/zooomr-api-key.txt", "" );
            }
        
        virtual std::string getAPISecret()
            {
                return getStrSubCtx( "~/.ferris/zooomr-api-secret.txt", "" );
            }
        
        
    public:

        WebPhotosZooomr( const std::string& username )
            :
            WebPhotos( username )
            {
            }
        
        virtual ~WebPhotosZooomr()
            {}
        
    };



    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    class WebPhotosPixelPipe
        :
        public WebPhotos
    {
    protected:

        
        virtual std::string getRESTURL()
            {
                string url = "http://api.pixelpipe.com/services/rest/";
                return url;
            }
        
    public:

        virtual std::string getImplementationShortName()
            {
                return "pixelpipe";
            }
        
        string getUploadURL()
            {
                string earl = "http://api.pixelpipe.com/services/upload/";
                return earl;
            }

        string getAuthURL()
            {
                return "http://pixelpipe.com/services/auth/";
            }
    
        
        
        virtual std::string getPostUploadURLPrefix()
            {
                return "http://pixelpipe.com/services/rest/";
            }
        
        virtual std::string getAPIKey()
            {
                return getStrSubCtx( "~/.ferris/pixelpipe-api-key.txt", "" );
            }
        
        virtual std::string getAPISecret()
            {
                return getStrSubCtx( "~/.ferris/pixelpipe-api-secret.txt", "" );
            }
        
        
    public:

        WebPhotosPixelPipe( const std::string& username )
            :
            WebPhotos( username )
            {
            }
        
        virtual ~WebPhotosPixelPipe()
            {}
        
    };

    

    
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    
    
    namespace Factory
    {
        fh_webPhotos getDefaultFlickrWebPhotos()
        {
            static fh_webPhotos ret = 0;
            if( !ret )
            {
                ret = new WebPhotos( "" );
            }
            return ret;
        }

        fh_webPhotos getDefault23hqWebPhotos()
        {
            static fh_webPhotos ret = 0;
            if( !ret )
            {
                ret = new WebPhotos23hq( "" );
            }
            return ret;
        }

        fh_webPhotos getDefaultZooomrWebPhotos()
        {
            static fh_webPhotos ret = 0;
            if( !ret )
            {
                ret = new WebPhotosZooomr( "" );
            }
            return ret;
        }

        fh_webPhotos getDefaultPixelPipeWebPhotos()
        {
            static fh_webPhotos ret = 0;
            if( !ret )
            {
                ret = new WebPhotosPixelPipe( "" );
            }
            return ret;
        }
        
        fh_webPhotos getDefaultWebPhotosForShortName( const std::string& n )
        {
            if( n == "flickr" )
                return getDefaultFlickrWebPhotos();
            if( n == "23hq" )
                return getDefault23hqWebPhotos();
            if( n == "zooomr" )
                return getDefaultZooomrWebPhotos();
            if( n == "pixelpipe" )
                return getDefaultPixelPipeWebPhotos();
            stringstream ss;
            ss << "No default webphoto service by name:" << n << endl;
            Throw_WebPhotoException( tostr(ss), 0 );
        }
        
        
    };
    
    
};
