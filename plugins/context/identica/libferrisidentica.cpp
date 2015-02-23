/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2009 Ben Martin

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

    $Id: libferrispostgresql.cpp,v 1.11 2009/04/18 21:30:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <FerrisContextPlugin.hh>
#include <FerrisKDE.hh>
#include <Trimming.hh>
#include <FerrisDOM.hh>
#include <FerrisCreationPlugin.hh>
#include <FerrisBoost.hh>
#include <FerrisWebServices_private.hh>

#include "libferrisidentica_shared.hh"
#include <qjson/parser.h>

using namespace std;

//#define DEBUG cerr
#define DEBUG LG_IDENTICA_D

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    using namespace Identica;

    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    class FERRISEXP_CTXPLUGIN IdenticaVideoUploadFileContext
        :
        public WebServicesFileUploadContext< IdenticaVideoUploadFileContext >
    {
        typedef WebServicesFileUploadContext< IdenticaVideoUploadFileContext > _Base;
        
    public:

        fh_identica getIdentica();
        virtual fh_WebServicesUpload getWebServicesUpload()
        {
            return m_wsUpload;
        }
        
        IdenticaVideoUploadFileContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
            }
        
    };
    
    
    class FERRISEXP_CTXPLUGIN IdenticaVideoUploadDirectoryContext
        :
        public WebServicesUploadDirectoryContext<
        IdenticaVideoUploadDirectoryContext, IdenticaVideoUploadFileContext >
    {
        typedef WebServicesUploadDirectoryContext<
        IdenticaVideoUploadDirectoryContext, IdenticaVideoUploadFileContext > _Base;
        
    public:
        IdenticaVideoUploadDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
    };
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    template < class ParentCtx >
    class FERRISEXP_CTXPLUGIN IdenticaContextBase
        :
        public ParentCtx
    {
      public:
        IdenticaContextBase( Context* parent, const std::string& rdn )
            :
            ParentCtx( parent, rdn )
            {
            }
        fh_identica getIdentica();

      protected:

      private:
        
    };

    // the twoot, dent, note, msg, whatever.
    class FERRISEXP_CTXPLUGIN IdenticaDingContext
        :
        public StateLessEAHolder< IdenticaDingContext, IdenticaContextBase< leafContext > >
    {
        typedef StateLessEAHolder< IdenticaDingContext, IdenticaContextBase< leafContext > > _Base;
        typedef IdenticaDingContext _Self;

        std::string m_username;
        std::string m_text;
        int m_inReplyTo;
        int m_favorited;
        time_t m_ctime;
        
      public:
        IdenticaDingContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }        
        void setup( QVariantMap dm )
        {
            m_inReplyTo = dm["in_reply_to_status_id"].toInt();
            m_username  = tostr(dm["user"].toMap()["name"].toString());
            m_text      = tostr(dm["text"].toString());
            m_favorited = isTrue(tostr(dm["favorited"].toString()));

            struct tm tm = Time::ParseTimeString(tostr(dm["created_at"].toString()));
            m_ctime = Time::toTime( tm );
            

        }
        string getRecommendedEA()
        {
            return "name,content,user,ctime-display";
        }
        static fh_stringstream
        SL_getUser( _Self* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_username;
                return ss;
            }
        static fh_stringstream
        SL_getFavorited( _Self* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_favorited;
                return ss;
            }
        static fh_stringstream
        SL_getCTime( _Self* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_ctime;
                return ss;
            }
        
        void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
#define SLEA tryAddStateLessAttribute         
                    SLEA( "user",      &_Self::SL_getUser,      XSD_BASIC_STRING );
                    SLEA( "favorited", &_Self::SL_getFavorited, XSD_BASIC_BOOL   );
                    SLEA( "ctime",     &_Self::SL_getCTime,     FXD_UNIXEPOCH_T  );
#undef SLEA
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        fh_istream
        priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ret;
                ret << m_text;
                return ret;
            }
        
    };
    
    class FERRISEXP_CTXPLUGIN IdenticaTimelineContext
        :
        public IdenticaContextBase< FakeInternalContext >
    {
        typedef IdenticaContextBase< FakeInternalContext > _Base;
        typedef IdenticaTimelineContext _Self;

        std::string m_restTail;
        bool m_includeReplies;
        
      public:
        IdenticaTimelineContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
        void setup( const std::string& restTail, bool includeReplies )
        {
            m_restTail = restTail;
            m_includeReplies = includeReplies;
        }

        string getRecommendedEA()
        {
            return "name,content,user,ctime-display";
        }
        

        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                      << " have read:" << getHaveReadDir()
                      << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                fh_context c;
                fh_identica site = getIdentica();

                int count = 200;
                if( contains( m_restTail, "retweet" ))
                    count = 100;
                
                stringmap_t args;
                args["count"] = tostr(count);
                args["page"] = "1";
                args["include_entities"] = "1";
                args["exclude_replies"] =  tostr(m_includeReplies);
                args["contributor_details"] = "1";
                QNetworkReply* reply = site->post( m_restTail, args );
                QByteArray ba = reply->readAll();

                DEBUG << "status update code:" << reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() << endl;
                DEBUG << "status update ba:" << tostr(ba) << endl;

                QJson::Parser parser;
                bool ok;
                QVariantList result = parser.parse( ba, &ok ).toList();
                DEBUG << "result.size:" << result.size() << endl;

                std::list< QVariantMap > replies;
                foreach (QVariant ding, result)
                {
                    QVariantMap dm = ding.toMap();
                    int inReplyTo = dm["in_reply_to_status_id"].toInt();
                    if( inReplyTo )
                    {
                        replies.push_back( dm );
                        continue;
                    }
                                
                    DEBUG << tostr(dm["user"].toMap()["name"].toString()) << " ";
                    DEBUG << tostr(dm["text"].toString()) << endl;

                    std::string rdn = tostr(dm["id"].toString());
                    IdenticaDingContext* c = 0;
                    c = priv_ensureSubContext( rdn, c );
                    c->setup( dm );
                }
                
                if( m_includeReplies )
                {
                    DEBUG << "m_includeReplies" << endl;
                    foreach( QVariantMap dm, replies )
                    {
                        int inReplyTo = dm["in_reply_to_status_id"].toInt();
                        DEBUG << "have reply to msg:" << inReplyTo << endl;
                        if( isSubContextBound(tostr(inReplyTo)))
                        {
                            DEBUG << "found parent..." << endl;
                            fh_context parent = getSubContext( tostr(inReplyTo) );
                            std::string rdn = tostr(dm["id"].toString());
                            IdenticaDingContext* c = 0;
                            c = parent->priv_ensureSubContext( rdn, c );
                            c->setup( dm );
                            DEBUG << "added to parent..." << endl;
                        }
                    }
                }
                
            }
    
        
            
      protected:

      private:
        
    };

    /******************************/
    /******************************/
    /******************************/



    class FERRISEXP_CTXPLUGIN IdenticaFriendContext
        :
        public StateLessEAHolder< IdenticaFriendContext, IdenticaContextBase< leafContext > >
    {
        typedef StateLessEAHolder< IdenticaFriendContext, IdenticaContextBase< leafContext > > _Base;
        typedef IdenticaFriendContext _Self;

        std::string m_id;
        std::string m_username;
        std::string m_screenname;
        std::string m_desc;
        std::string m_imageURL;
        std::string m_homepage;
        time_t m_ctime;
        
      public:
        IdenticaFriendContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }        
        void setup( QVariantMap dm )
        {
            m_id          = tostr(dm["id"].toString());
            m_username    = tostr(dm["name"].toString());
            m_screenname  = tostr(dm["screen_name"].toString());
            m_desc        = tostr(dm["description"].toString());
            m_imageURL    = tostr(dm["profile_image_url"].toString());
            m_homepage    = tostr(dm["url"].toString());

            struct tm tm = Time::ParseTimeString(tostr(dm["created_at"].toString()));
            m_ctime = Time::toTime( tm );
            

        }
        string getRecommendedEA()
        {
            return "id,name,username,screenname,homepage";
        }
        static fh_stringstream
        SL_getID( _Self* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_id;
                return ss;
            }
        static fh_stringstream
        SL_getUserName( _Self* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_username;
                return ss;
            }
        static fh_stringstream
        SL_getScreenName( _Self* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_screenname;
                return ss;
            }
        static fh_stringstream
        SL_getHomepage( _Self* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_homepage;
                return ss;
            }
        static fh_stringstream
        SL_getCTime( _Self* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_ctime;
                return ss;
            }

        void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
#define SLEA tryAddStateLessAttribute         
                    SLEA( "id",          &_Self::SL_getID,            XSD_BASIC_STRING );
                    SLEA( "username",    &_Self::SL_getUserName,      XSD_BASIC_STRING );
                    SLEA( "screenname",  &_Self::SL_getScreenName,    XSD_BASIC_STRING );
                    SLEA( "homepage",    &_Self::SL_getHomepage,      XSD_BASIC_STRING );
                    SLEA( "ctime",       &_Self::SL_getCTime,         FXD_UNIXEPOCH_T  );
#undef SLEA
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        
    };
    

    class FERRISEXP_CTXPLUGIN IdenticaFriendsDirectoryContext
        :
        public IdenticaContextBase< FakeInternalContext >
    {
        typedef IdenticaContextBase< FakeInternalContext > _Base;
        typedef IdenticaFriendsDirectoryContext _Self;

        std::string m_restTail;
        bool m_includeReplies;
        
      public:
        IdenticaFriendsDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
        void setup( const std::string& restTail )
        {
            m_restTail = restTail;
        }

        string getRecommendedEA()
        {
            return "name,phone,email";
        }

        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                      << " have read:" << getHaveReadDir()
                      << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                fh_context c;
                fh_identica site = getIdentica();

                int count = 200;
                stringmap_t args;
                args["count"] = tostr(count);
                args["include_entities"] = "1";
                QNetworkReply* reply = site->post( m_restTail, args );
                QByteArray ba = reply->readAll();

                DEBUG << "status update code:" << reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() << endl;
                DEBUG << "status update ba:" << tostr(ba) << endl;

                QJson::Parser parser;
                bool ok;
                QVariantList result = parser.parse( ba, &ok ).toList();
                DEBUG << "result.size:" << result.size() << endl;

                std::stringstream idss;
                std::list<int> idlist;
                foreach (QVariant ding, result)
                {
                    int id = ding.toInt();
                    idss << id << ",";
                    idlist.push_back( id );
                }

                // in the future when statusnet supports it,
                // switch to batch mode using post( "users/lookup.json" );

                foreach( int id, idlist )
                {
                    args.clear();
                    args["user_id"] = tostr(id);
                    args["include_entities"] = "1";
                    reply = site->post( "users/show.json", args );
                    ba = reply->readAll();
                    DEBUG << "status2 update code:" << reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() << endl;
                    DEBUG << "status2 update ba:" << tostr(ba) << endl;
                    
                    QVariantMap dm = parser.parse( ba, &ok ).toMap();
                    DEBUG << tostr(dm["name"].toString()) << " ";

                    
                    std::string rdn = tostr(dm["name"].toString());
                    IdenticaFriendContext* c = 0;
                    c = priv_ensureSubContext( rdn, c );
                    c->setup( dm );
                }
            }
        
        
            
      protected:

      private:
        
    };
    

    /******************************/
    /******************************/
    /******************************/

    class FERRISEXP_CTXPLUGIN IdenticaDirectMessageContext
        :
        public StateLessEAHolder< IdenticaDirectMessageContext, IdenticaContextBase< leafContext > >
    {
        typedef StateLessEAHolder< IdenticaDirectMessageContext, IdenticaContextBase< leafContext > > _Base;
        typedef IdenticaDirectMessageContext  _Self;

        string m_text;
        string m_from;
        string m_to;
        time_t m_mtime;
        
      public:
        IdenticaDirectMessageContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }
        void setup( QVariantMap dm )
        {
            m_text  = tostr(dm["text"].toString());
            m_from  = tostr(dm["sender_screen_name"].toString());
            m_to    = tostr(dm["recipient_screen_name"].toString());
            struct tm tm = Time::ParseTimeString(tostr(dm["created_at"].toString()));
            m_mtime = Time::toTime( tm );
        }
        string getRecommendedEA()
        {
            return "name,from,to,mtime-display,content";
        }
        static fh_stringstream
        SL_getFrom( _Self* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_from;
                return ss;
            }
        static fh_stringstream
        SL_getTo( _Self* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_to;
                return ss;
            }
        static fh_stringstream
        SL_getMTime( _Self* c, const std::string& ean, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_mtime;
                return ss;
            }

        void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
#define SLEA tryAddStateLessAttribute         
                    SLEA( "from",  &_Self::SL_getFrom,      XSD_BASIC_STRING );
                    SLEA( "to",    &_Self::SL_getTo,        XSD_BASIC_STRING );
                    SLEA( "mtime", &_Self::SL_getMTime,     FXD_UNIXEPOCH_T  );
#undef SLEA
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        fh_istream
        priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ret;
                ret << m_text;
                return ret;
            }
        
        
    };
    
    class FERRISEXP_CTXPLUGIN IdenticaDirectMessagesDirectoryContext
        :
        public IdenticaContextBase< FakeInternalContext >
    {
        typedef IdenticaContextBase< FakeInternalContext > _Base;
        typedef IdenticaDirectMessagesDirectoryContext     _Self;

        std::string m_restTail;
        
      public:
        IdenticaDirectMessagesDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
        void setup( const std::string& restTail )
        {
            m_restTail = restTail;
        }

        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                      << " have read:" << getHaveReadDir()
                      << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                fh_identica site = getIdentica();

                stringmap_t args;
                args["count"] = tostr(200);
                args["page"]  = "1";
                args["include_entities"] = "1";
                QNetworkReply* reply = site->post( m_restTail, args );
                QByteArray ba = reply->readAll();

                DEBUG << "status update code:" << reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() << endl;
                DEBUG << "status update ba:" << tostr(ba) << endl;

                // clean it up (some "notice" junk before real datas)
                {
                    string data = tostr(ba);
                    int st = data.find('[');
                    if( st!=string::npos )
                    {
                        data = data.substr( st );
                        ba = data.c_str();
                    }
                }
                
                QJson::Parser parser;
                bool ok;
                QVariantList result = parser.parse( ba, &ok ).toList();
                DEBUG << "result.size:" << result.size() << endl;

                foreach (QVariant ding, result)
                {
                    QVariantMap dm = ding.toMap();

                    std::string rdn = tostr(dm["id"].toString());
                    IdenticaDirectMessageContext* c = 0;
                    c = priv_ensureSubContext( rdn, c );
                    c->setup( dm );
                }
            }
        
    };


    class FERRISEXP_CTXPLUGIN IdenticaDirectMessagesNewContext
        :
        public IdenticaContextBase< FakeInternalContext >
    {
        typedef IdenticaContextBase< FakeInternalContext > _Base;
        typedef IdenticaDirectMessagesNewContext  _Self;

        std::string m_user_id;
        std::string m_screen_name;
        
      public:
        IdenticaDirectMessagesNewContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
        void setup( fh_context contact )
        {
            m_user_id     = getStrAttr( contact, "id", "" );
            m_screen_name = getStrAttr( contact, "screenname", "" );
        }

        ferris_ios::openmode
            getSupportedOpenModes()
            {
                return
                    ios_base::in        |
                    ios_base::out       |
                    std::ios::in        |
                    std::ios::out       |
                    std::ios::trunc     |
                    std::ios::ate       |
                    std::ios::app       |
                    ios_base::binary    ;
            }
        
        fh_iostream
            priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                DEBUG << "priv_getIOStream(top) status" << endl;
                fh_stringstream ret;
                ret->getCloseSig().connect( bind( sigc::mem_fun(*this, &_Self::OnStreamClosed ), m )); 
                DEBUG << "priv_getIOStream(ret) status" << endl;
                return ret;
            }

        void
        OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
                DEBUG << "priv_OnStreamClosed(top)" << endl;
                
                if( !(m & std::ios::out) )
                    return;

                AdjustForOpenMode_Closing( ss, m, tellp );
                string data = StreamToString(ss);
                DEBUG << "OnStreamClosed() data:" << data << endl;

                fh_identica site = getIdentica();
                stringmap_t args;
                args["text"] = URLencode( data );
                args["user_id"] = m_user_id;
                args["screen_name"] = m_screen_name;
                QNetworkReply* reply = site->post( "direct_messages/new.json", args );
                QByteArray ba = reply->readAll();
                
                DEBUG << "status update code:" << reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() << endl;
                DEBUG << "status update ba:" << tostr(ba) << endl;

            }
        
    };
    
    class FERRISEXP_CTXPLUGIN IdenticaDirectMessagesNewDirectoryContext
        :
        public IdenticaContextBase< FakeInternalContext >
    {
        typedef IdenticaContextBase< FakeInternalContext > _Base;
        typedef IdenticaDirectMessagesNewDirectoryContext  _Self;

      public:
        IdenticaDirectMessagesNewDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }

        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                      << " have read:" << getHaveReadDir()
                      << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                fh_identica site = getIdentica();

                fh_context contacts = Resolve( getURL() + "/../../friends");
                for( Context::iterator ci = contacts->begin(); ci != contacts->end(); ++ci )
                {
                    std::string rdn = (*ci)->getDirName();
                    IdenticaDirectMessagesNewContext* c = 0;
                    c = priv_ensureSubContext( rdn, c );
                    c->setup( *ci );
                }
            }
        
    };
    


    
    class FERRISEXP_CTXPLUGIN IdenticaDirectMessagesRootDirectoryContext
        :
        public IdenticaContextBase< FakeInternalContext >
    {
        typedef IdenticaContextBase< FakeInternalContext > _Base;
        typedef IdenticaDirectMessagesRootDirectoryContext _Self;

        std::string m_restTail;
        bool m_includeReplies;
        
      public:
        IdenticaDirectMessagesRootDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }

        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                      << " have read:" << getHaveReadDir()
                      << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                
                fh_identica site = getIdentica();
                    
                {
                    IdenticaDirectMessagesDirectoryContext* c = 0;
                    c = priv_ensureSubContext( "inbox", c );
                    c->setup( "direct_messages.json" );
                }
                {
                    IdenticaDirectMessagesDirectoryContext* c = 0;
                    c = priv_ensureSubContext( "sent", c );
                    c->setup( "direct_messages/sent.json" );
                }
                {
                    IdenticaDirectMessagesNewDirectoryContext* c = 0;
                    c = priv_ensureSubContext( "new", c );
                }
            }
        
    };

    /******************************/
    /******************************/
    /******************************/
    
    class FERRISEXP_CTXPLUGIN IdenticaStatusContext
        :
        public IdenticaContextBase< leafContext >
    {
        typedef IdenticaContextBase< leafContext > _Base;
        typedef IdenticaStatusContext              _Self;
        
      public:
        IdenticaStatusContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }

      protected:

        ferris_ios::openmode
            getSupportedOpenModes()
            {
                return
                    ios_base::in        |
                    ios_base::out       |
                    std::ios::in        |
                    std::ios::out       |
                    std::ios::trunc     |
                    std::ios::ate       |
                    std::ios::app       |
                    ios_base::binary    ;
            }
        
        fh_iostream
            priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                DEBUG << "priv_getIOStream(top) status" << endl;
                fh_stringstream ret;
                ret->getCloseSig().connect( bind( sigc::mem_fun(*this, &_Self::OnStreamClosed ), m )); 
                DEBUG << "priv_getIOStream(ret) status" << endl;
                return ret;
            }

        void
        OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
                DEBUG << "priv_OnStreamClosed(top)" << endl;
                
                if( !(m & std::ios::out) )
                    return;

                AdjustForOpenMode_Closing( ss, m, tellp );
                string data = StreamToString(ss);
                DEBUG << "OnStreamClosed() data:" << data << endl;

                fh_identica site = getIdentica();
                site->setStatus( data );
            }
        
      private:
        
    };
    
    

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    class FERRISEXP_CTXPLUGIN IdenticaServerContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext   _Base;
        typedef IdenticaServerContext _Self;
        bool m_haveTriedToRead;
        fh_identica m_site;
        
    public:

        IdenticaServerContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_haveTriedToRead( false )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
            }

        fh_identica getIdentica()
        {
            return m_site;
        }
        void setup( fh_identica site )
        {
            m_site = site;
        }
        
        
        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                      << " m_haveTriedToRead:" << m_haveTriedToRead
                      << " have read:" << getHaveReadDir()
                      << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                
                if( m_haveTriedToRead )
                {
                    emitExistsEventForEachItemRAII _raii1( this );
                }
                else
                {
                    m_haveTriedToRead = true;
                    fh_context c;
                    fh_identica site = getIdentica();
                    
                    // {
                    //     IdenticaVideoUploadDirectoryContext* c = 0;
                    //     c = priv_ensureSubContext( "upload", c );
                    // }
                    {
                        IdenticaStatusContext* c = 0;
                        c = priv_ensureSubContext( "status", c );
                    }
                    {
                        IdenticaTimelineContext* c = 0;
                        c = priv_ensureSubContext( "timeline", c );
                        c->setup( "statuses/home_timeline.json", true );
                    }
                    {
                        IdenticaTimelineContext* c = 0;
                        c = priv_ensureSubContext( "mentions", c );
                        c->setup( "statuses/mentions.json", true );
                    }
                    // statusnet {"error":"Unimplemented."
                    // {
                    //     IdenticaTimelineContext* c = 0;
                    //     c = priv_ensureSubContext( "retweeted_by_me", c );
                    //     c->setup( "statuses/retweeted_by_me.json", true );
                    // }
                    // {
                    //     IdenticaTimelineContext* c = 0;
                    //     c = priv_ensureSubContext( "retweeted_to_me", c );
                    //     c->setup( "statuses/retweeted_by_me.json", true );
                    // }
                    {
                        IdenticaTimelineContext* c = 0;
                        c = priv_ensureSubContext( "retweets_of_me", c );
                        c->setup( "statuses/retweets_of_me.json", true );
                    }
                    {
                        IdenticaFriendsDirectoryContext* c = 0;
                        c = priv_ensureSubContext( "friends", c );
                        c->setup( "friends/ids.json" );
                    }
                    {
                        IdenticaDirectMessagesRootDirectoryContext* c = 0;
                        c = priv_ensureSubContext( "messages", c );
                    }
                }
            }
    };

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    class FERRISEXP_CTXPLUGIN IdenticaRootContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        bool m_haveTriedToRead;
    public:

        IdenticaRootContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_haveTriedToRead( false )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
            }

        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                      << " m_haveTriedToRead:" << m_haveTriedToRead
                      << " have read:" << getHaveReadDir()
                      << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                
                if( m_haveTriedToRead )
                {
                    emitExistsEventForEachItemRAII _raii1( this );
                }
                else
                {
                    m_haveTriedToRead = true;
                    fh_context c;

                    {
                        fh_context cfg = Resolve("~/.ferris/identica");
                        for( Context::iterator ci = cfg->begin(); ci != cfg->end(); ++ci )
                        {
                            std::string rdn = ci->getDirName();
                            
                            fh_identica site = Factory::getIdentica( rdn );
                            IdenticaServerContext* c = 0;
                            c = priv_ensureSubContext( rdn, c );
                            c->setup( site );
                        }
                    }
                }
            }
    };


    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    fh_identica IdenticaVideoUploadFileContext::getIdentica()
    {
        IdenticaServerContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p->getIdentica();
    }
    
    template < class ParentCtx >
    fh_identica IdenticaContextBase<ParentCtx>::getIdentica()
    {
        IdenticaServerContext* p = 0;
        p = ParentCtx::getFirstParentOfContextClass( p );
        return p->getIdentica();
    }
    


    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                DEBUG << "Brew()" << endl;

                static IdenticaRootContext* c = 0;
                if( !c )
                {
                    c = new IdenticaRootContext(0, "/" );
                    // Bump ref count.
                    static fh_context keeper = c;
                    static fh_context keeper2 = keeper;
                }
                fh_context ret = c;

                return ret;
            }
            catch( FerrisSqlServerNameNotFound& e )
            {
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
            catch( exception& e )
            {
                LG_PG_W << "e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }

};
