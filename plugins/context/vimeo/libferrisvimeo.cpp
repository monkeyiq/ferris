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

#include "libferrisvimeo_shared.hh"

using namespace std;

#define DEBUG LG_FACEBOOK_D

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    using namespace Vimeo;

    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    static fh_domdoc throwIfNotOK( QNetworkReply* reply )
    {
        QByteArray ba = reply->readAll();
//        DEBUG << "reply:" << tostr(ba) << endl;
        
        fh_domdoc doc = Factory::StringToDOM( tostr(ba) );
        DOMElement* root = doc->getDocumentElement();
        std::string statcode = getAttribute( root, "stat" );
        if( statcode != "" && statcode != "ok" )
        {
            stringstream ss;
            ss << "Error. Data from server:" << tostr(ba) << endl;
            LG_WEBPHOTO_W << tostr(ss);
            Throw_WebPhotoException( tostr(ss), 0 );
        }
        return doc;
    }
    
    
    class FERRISEXP_CTXPLUGIN VimeoVideoUploadFileContext
        :
        public WebServicesFileUploadContext< VimeoVideoUploadFileContext >
    {
        typedef WebServicesFileUploadContext< VimeoVideoUploadFileContext > _Base;
        
    public:

        fh_vimeo getVimeo();
        virtual fh_WebServicesUpload getWebServicesUpload()
        {
            if( !m_wsUpload )
            {
                fh_vimeo vim = getVimeo();
                m_wsUpload = vim->createUpload();
            }
            return m_wsUpload;
        }
        
        VimeoVideoUploadFileContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
            }
        
    };
    
    
    class FERRISEXP_CTXPLUGIN VimeoVideoUploadDirectoryContext
        :
        public WebServicesUploadDirectoryContext<
        VimeoVideoUploadDirectoryContext, VimeoVideoUploadFileContext >
    {
        typedef WebServicesUploadDirectoryContext<
        VimeoVideoUploadDirectoryContext, VimeoVideoUploadFileContext > _Base;
        
    public:
        VimeoVideoUploadDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
    };
    

    static fh_context xmlToFilesystem( const std::string& xml )
    {
        cerr << "xmlToFilesystem() xml.sz:" << xml.size() << endl;
        fh_domdoc doc = Factory::StringToDOM( xml );

        cerr << "have doc:" << toVoid(GetImpl(doc)) << endl;
        DOMElement* root = doc->getDocumentElement();
        std::string statcode = getAttribute( root, "stat" );
        if( statcode != "" && statcode != "ok" )
        {
            stringstream ss;
            ss << "Error. Data from server:" << xml << endl;
            DEBUG << tostr(ss);
            Throw_WebServiceException( tostr(ss), 0 );
        }
        
        fh_context ret = Ferris::Factory::mountDOM( doc );
        if( !ret->isSubContextBound( "rsp" ) )
        {
            stringstream ss;
            ss << "Error. No RSP element at top level of data." << endl
               << "Data from server:" << tostr( doc, true ) << endl;
            DEBUG << tostr(ss);
            Throw_WebServiceException( tostr(ss), 0 );
        }
        ret = ret->getSubContext( "rsp" );
        doc = 0;

        DEBUG << "returning:" << ret->getDirPath() << endl;
        return ret;
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    
    class FERRISEXP_CTXPLUGIN VimeoVideoContext
        :
        public StateLessEAHolder< VimeoVideoContext, leafContext >
    {
        typedef VimeoVideoContext _Self;
        typedef StateLessEAHolder< VimeoVideoContext, leafContext > _Base;
      public:
#define READABLE_ATTRIBUTE( atype, aname ) \
        atype m_##aname;                                                \
            static fh_stringstream SL_getStream_##aname( VimeoVideoContext* c, \
                                                        const std::string& rdn, \
                                                        EA_Atom* atom ) \
            {                                                           \
                fh_stringstream ss;                                     \
                ss << c->m_##aname;                                     \
                return ss;                                              \
            }

        READABLE_ATTRIBUTE( string, videoID );
        READABLE_ATTRIBUTE( int, duration );
        READABLE_ATTRIBUTE( int, width );
        READABLE_ATTRIBUTE( int, height );
        READABLE_ATTRIBUTE( int, likes );
        READABLE_ATTRIBUTE( int, plays );
        READABLE_ATTRIBUTE( string, earl );
        READABLE_ATTRIBUTE( string, thumbearl );
        READABLE_ATTRIBUTE( string, desc );
        READABLE_ATTRIBUTE( string, title );
        READABLE_ATTRIBUTE( time_t, utime );
        READABLE_ATTRIBUTE( time_t, mtime );
      private:
        
        static void SL_setStream_title( VimeoVideoContext* c,
                                        const std::string& rdn,
                                        EA_Atom* atom,
                                        fh_istream iss )
            {
                string newTitle = StreamToString(iss);

                fh_vimeo v = Factory::getVimeo();
                QNetworkRequest req = v->createRequest();
                stringmap_t args;
                args["video_id"]      = c->m_videoID;
                args["title"]         = newTitle;
                QNetworkReply* reply = v->callMeth( "vimeo.videos.setTitle", req, args );
                throwIfNotOK( reply );
                c->m_title = newTitle;
            }

        void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
        {
        }
        
      public:

        VimeoVideoContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }

        bool isDir()
        {
            return false;
        }

        
        void createStateLessAttributes( bool force = false )
        {
            if( force || isStateLessEAVirgin() )
            {
                _Base::createStateLessAttributes( true );

#define SLEA  tryAddStateLessAttribute         

                SLEA( "video-id",  &_Self::SL_getStream_videoID, XSD_BASIC_STRING );
                SLEA( "title",
                      &_Self::SL_getStream_title, &_Self::SL_getStream_title,
                      &_Self::SL_setStream_title, XSD_BASIC_STRING );
                SLEA( "desc",      &_Self::SL_getStream_desc,   XSD_BASIC_STRING );
                SLEA( "likes",     &_Self::SL_getStream_likes,   XSD_BASIC_INT );
                SLEA( "plays",     &_Self::SL_getStream_plays,   XSD_BASIC_INT );
                SLEA( "width",     &_Self::SL_getStream_width,   XSD_BASIC_INT );
                SLEA( "height",    &_Self::SL_getStream_height,  XSD_BASIC_INT );
                SLEA( "duration",  &_Self::SL_getStream_duration,XSD_BASIC_INT );
                SLEA( "earl",      &_Self::SL_getStream_earl,    XSD_BASIC_STRING );
                SLEA( "thumbearl", &_Self::SL_getStream_thumbearl,XSD_BASIC_STRING );
                SLEA( "utime",     &_Self::SL_getStream_utime,   FXD_UNIXEPOCH_T );
                SLEA( "mtime",     &_Self::SL_getStream_mtime,   FXD_UNIXEPOCH_T );
                supplementStateLessAttributes_timet( "utime" );

#undef SLEA                    
                    
                supplementStateLessAttributes( true );
            }
        }
        
        virtual std::string getRecommendedEA()
        {
            return _Base::getRecommendedEA() + ",video-id,title,utime,";
        }

        static string getRDN( fh_context md )
        {
            string ret = getStrAttr( md, "id", "" );
//            ret = getStrSubCtx( md, "title", "" );
            DEBUG << "getRDN() ret:" << ret << endl;
            return ret;
        }

        time_t TT( string stddatestr )
        {
            cerr << "TT:" << stddatestr << endl;
            if( stddatestr.empty() )
                return 0;
            try
            {
                struct tm mytm = Time::ParseTimeString( stddatestr );
                return Time::toTime( &mytm );
            }
            catch( ... )
            {
                return 0;
            }
        }
        
        void constructObject( fh_context md )
        {
            m_videoID = getStrAttr( md, "id", "" );
            m_title = getStrSubCtx( md, "title", "" );
            m_desc  = getStrSubCtx( md, "description", "" );
            m_likes  = toint(getStrSubCtx( md, "number_of_likes", "0" ));
            m_plays  = toint(getStrSubCtx( md, "number_of_plays", "0" ));
            m_width  = toint(getStrSubCtx( md, "width", "0" ));
            m_height = toint(getStrSubCtx( md, "height", "0" ));
            m_duration = toint(getStrSubCtx( md, "duration", "0" ));
            m_utime = TT(getStrSubCtx( md, "upload_date", "" ));
            m_mtime = TT(getStrSubCtx( md, "modified_date", "" ));
            
            if( md->isSubContextBound("urls"))
            {
                fh_context t = md->getSubContext("urls");
                if( t->begin() != t->end() )
                    t = *(t->begin());
                m_earl   = getStrAttr( t, "content", "" );
            }
            if( md->isSubContextBound("thumbnails"))
            {
                fh_context t = md->getSubContext("thumbnails");
                Context::iterator ci = t->begin();
                Context::iterator ce = t->end();
                int cw = 0;
                string earl = "";
                for( ; ci!=ce; ++ci )
                {
                    int w = toint(getStrAttr( *ci, "width", "0" ));
                    if( w && w > cw )
                    {
                        earl = getStrAttr( *ci, "content", "" );
                    }
                }
                m_thumbearl = earl;
            }
            
        }
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
        

    class FERRISEXP_CTXPLUGIN VimeoVideosDirectoryContext
        :
        public StateLessEAHolder< VimeoVideosDirectoryContext, FakeInternalContext >
    {
        typedef VimeoVideosDirectoryContext _Self;
        typedef StateLessEAHolder< VimeoVideosDirectoryContext, FakeInternalContext > _Base;

        void priv_read()
        {
            staticDirContentsRAII _raii1( this );

            if( empty() )
            {
                fh_vimeo v = Factory::getVimeo();
                QNetworkRequest req = v->createRequest();
                stringmap_t args;
                args["user_id"]       = v->getToken();
                args["sort"]          = "newest";
                args["per_page"]      = "50";
                args["full_response"] = "1";
                QNetworkReply* reply = v->callMeth( "vimeo.videos.getUploaded", req, args );
                
                QByteArray ba = reply->readAll();
                cerr << "reply:" << tostr(ba) << endl;

                fh_context dl = xmlToFilesystem( tostr(ba) );
                if( dl->isSubContextBound("videos"))
                    dl = dl->getSubContext("videos");
                cerr << "dl:" << dl->getDirPath() << endl;
                
                Context::iterator ci = dl->begin();
                Context::iterator ce = dl->end();
                for( ; ci!=ce; ++ci )
                {
                    string id    = getStrAttr( *ci, "id", "" );
                    string title = getStrSubCtx( *ci, "title", "" );
                    string desc  = getStrSubCtx( *ci, "description", "" );

                    cerr << "title:" << title << endl;
                    
                    string rdn = VimeoVideoContext::getRDN( *ci );
                    VimeoVideoContext* c = 0;
                    c = priv_ensureSubContext( rdn, c );
                    c->constructObject( *ci );
                }
            }
        }

        template <class ChildContextType>
            ChildContextType* downcastContext( fh_context c )
        {
            ChildContextType* ret = 0;
            ret = dynamic_cast<ChildContextType*>(GetImpl(c));
            return ret;
        }
        
        bool supportsRemove()
        {
            return true;
        }
        
        void priv_remove( fh_context c )
        {
            DEBUG << "remove() path:" << c->getDirPath() << endl;

            fh_vimeo v = Factory::getVimeo();
            QNetworkRequest req = v->createRequest();
            stringmap_t args;
            args["video_id"]      = downcastContext<VimeoVideoContext>(c)->m_videoID;
            QNetworkReply* reply = v->callMeth( "vimeo.videos.delete", req, args );
            throwIfNotOK( reply );
        }
        
    public:
        VimeoVideosDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }

        bool isDir()
        {
            return true;
        }
        
        void createStateLessAttributes( bool force = false )
        {
            if( force || isStateLessEAVirgin() )
            {
                _Base::createStateLessAttributes( true );

#define SLEA  tryAddStateLessAttribute         

                SLEA( "video-id", &_Self::SL_getNothingStream, XSD_BASIC_INT );
                SLEA( "title",
                      &_Self::SL_getNothingStream, &_Self::SL_getNothingStream,
                      &_Self::SL_setNothingStream, XSD_BASIC_STRING );
                SLEA( "utime", &_Self::SL_getNothingStream,   FXD_UNIXEPOCH_T );
                supplementStateLessAttributes_timet( "utime" );

#undef SLEA                    
                    
                supplementStateLessAttributes( true );
            }
        }

        virtual std::string getRecommendedEA()
        {
            return _Base::getRecommendedEA() + ",video-id,title,utime,duration,width,height,";
        }
    };
    
    

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    class FERRISEXP_CTXPLUGIN VimeoRootContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        bool m_haveTriedToRead;
    public:

        VimeoRootContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_haveTriedToRead( false )
            {
                LG_FACEBOOK_D << "ctor, have read:" << getHaveReadDir() << endl;
            }

        fh_vimeo getVimeo()
        {
            return Factory::getVimeo();
        }
        
        void priv_read()
            {
                LG_FACEBOOK_D << "priv_read() url:" << getURL()
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

                    fh_vimeo vim = getVimeo();
                    

                    {
                        VimeoVideoUploadDirectoryContext* c = 0;
                        c = priv_ensureSubContext( "upload", c );
                    }

                    {
                        VimeoVideosDirectoryContext* c = 0;
                        c = priv_ensureSubContext( "videos", c );
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

    fh_vimeo VimeoVideoUploadFileContext::getVimeo()
    {
        VimeoRootContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p->getVimeo();
    }
    


    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                LG_FACEBOOK_D << "Brew()" << endl;

                static VimeoRootContext* c = 0;
                if( !c )
                {
                    c = new VimeoRootContext(0, "/" );
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
