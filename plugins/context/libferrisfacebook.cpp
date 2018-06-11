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
#include "Ferris/FerrisWebServices_private.hh"

#include "libferrisfacebook_shared.hh"

using namespace std;

#define DEBUG LG_FACEBOOK_D

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    using namespace Facebook;

    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    
    class FERRISEXP_CTXPLUGIN FacebookPostContext
        :
        public StateLessEAHolder< FacebookPostContext, leafContext >
    {
        typedef StateLessEAHolder< FacebookPostContext, leafContext > _Base;
        typedef FacebookPostContext _Self;

        fh_Post m_post;
        
    public:

        FacebookPostContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~FacebookPostContext()
        {
        }
        void constructObject( fh_Post post )
        {
            m_post = post;
        }
        

        virtual std::string getRecommendedEA()
        {
            return adjustRecommendedEAForDotFiles(this, "name,author,permalink,mtime");
        }

        static fh_stringstream SL_getID( FacebookPostContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_post->getID();
                return ss;
            }
        static fh_stringstream SL_getAuthor( FacebookPostContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_post->getAuthorName();
                return ss;
            }
        static fh_stringstream SL_getPermalink( FacebookPostContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_post->getPermalink();
                return ss;
            }
        static fh_stringstream SL_getMTime( FacebookPostContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_post->getCreateTime();
                return ss;
            }

        
        
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );

#define SLEA  tryAddStateLessAttribute         

                    SLEA( "id",          &_Self::SL_getID,           XSD_BASIC_STRING );
                    SLEA( "author",      &_Self::SL_getAuthor,       XSD_BASIC_STRING );
                    SLEA( "permalink",   &_Self::SL_getPermalink,    XSD_BASIC_STRING );
                    SLEA( "mtime",       &_Self::SL_getMTime,        FXD_UNIXEPOCH_T );
                    
#undef SLEA                    
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
                ret << m_post->getMessage();
                return ret;
            }
        

        ferris_ios::openmode
            getSupportedOpenModes()
            {
                return
                    ios_base::in        |
                    ios_base::binary    ;
            }

    };

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    

    class FERRISEXP_CTXPLUGIN FacebookRecentStreamDirectoryContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        typedef FacebookRecentStreamDirectoryContext _Self;
        
    public:

        FacebookRecentStreamDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
        fh_facebook getFacebook();
        
        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                      << " have read:" << getHaveReadDir()
                      << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                
                if( !empty() )
                {
                    emitExistsEventForEachItemRAII _raii1( this );
                }
                else
                {
                    fh_facebook  fb = getFacebook();
                    PostMap_t posts = fb->getRecentStreamPosts();
                    DEBUG << "recent posts.sz:" << posts.size() << endl;
                    for( PostMap_t::iterator pi = posts.begin(); pi != posts.end(); ++pi )
                    {
                        fh_Post p = pi->second;
                        string rdn = p->getRDN();
                        DEBUG << "recent post rdn:" << rdn << endl;

                        if( !rdn.empty() )
                        {
                            FacebookPostContext* c = 0;
                            c = priv_ensureSubContext( rdn, c );
                            c->constructObject( p );
                        }
                    }
                    DEBUG << "done adding recent posts.sz:" << posts.size() << endl;
                }
            }
    };

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/


    class FERRISEXP_CTXPLUGIN FacebookPhotoUploadFileContext
        :
        public WebServicesFileUploadContext< FacebookPhotoUploadFileContext >
    {
        typedef WebServicesFileUploadContext< FacebookPhotoUploadFileContext > _Base;
        
    public:

        fh_facebook getFacebook();
        virtual fh_WebServicesUpload getWebServicesUpload()
        {
            if( !m_wsUpload )
            {
                fh_facebook fb = getFacebook();
                m_wsUpload = fb->createUpload( "facebook.photos.upload" );
            }
            return m_wsUpload;
        }
        
        FacebookPhotoUploadFileContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
            }
        
    };


    class FERRISEXP_CTXPLUGIN FacebookPhotoUploadDirectoryContext
        :
        public WebServicesUploadDirectoryContext<
        FacebookPhotoUploadDirectoryContext, FacebookPhotoUploadFileContext >
    {
        typedef WebServicesUploadDirectoryContext<
        FacebookPhotoUploadDirectoryContext, FacebookPhotoUploadFileContext > _Base;
        
    public:
        FacebookPhotoUploadDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
    };
    
    
    class FERRISEXP_CTXPLUGIN FacebookPhotoDirectoryContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        typedef FacebookPhotoDirectoryContext _Self;
        
    public:

        FacebookPhotoDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
        fh_facebook getFacebook();
        
        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                      << " have read:" << getHaveReadDir()
                      << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                
                if( !empty() )
                {
                    emitExistsEventForEachItemRAII _raii1( this );
                }
                else
                {
                    fh_facebook  fb = getFacebook();

                    FacebookPhotoUploadDirectoryContext* c = 0;
                    c = priv_ensureSubContext( "upload", c );
                }
            }
    };

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    

    class FERRISEXP_CTXPLUGIN FacebookVideoUploadFileContext
        :
        public WebServicesFileUploadContext< FacebookVideoUploadFileContext >
    {
        typedef WebServicesFileUploadContext< FacebookVideoUploadFileContext > _Base;
        
    public:

        fh_facebook getFacebook();
        virtual fh_WebServicesUpload getWebServicesUpload()
        {
            if( !m_wsUpload )
            {
                fh_facebook fb = getFacebook();
                m_wsUpload = fb->createUpload( "facebook.video.upload" );
            }
            return m_wsUpload;
        }
        
        FacebookVideoUploadFileContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
            }
        
    };
    
    class FERRISEXP_CTXPLUGIN FacebookVideoUploadDirectoryContext
        :
        public WebServicesUploadDirectoryContext<
        FacebookVideoUploadDirectoryContext, FacebookVideoUploadFileContext >
    {
        typedef WebServicesUploadDirectoryContext<
        FacebookVideoUploadDirectoryContext, FacebookVideoUploadFileContext > _Base;
        
    public:
        FacebookVideoUploadDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
    };
    
    
    class FERRISEXP_CTXPLUGIN FacebookVideoDirectoryContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        typedef FacebookVideoDirectoryContext _Self;
        
    public:

        FacebookVideoDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
        fh_facebook getFacebook();
        
        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                      << " have read:" << getHaveReadDir()
                      << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                
                if( !empty() )
                {
                    emitExistsEventForEachItemRAII _raii1( this );
                }
                else
                {
                    fh_facebook  fb = getFacebook();

                    FacebookVideoUploadDirectoryContext* c = 0;
                    c = priv_ensureSubContext( "upload", c );
                }
            }
    };
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/


    class FERRISEXP_CTXPLUGIN FacebookContactContext
        :
        public StateLessEAHolder< FacebookContactContext, leafContext >
    {
        typedef StateLessEAHolder< FacebookContactContext, leafContext > _Base;
        typedef FacebookContactContext _Self;

        fh_Contact m_contact;
        
    public:

        FacebookContactContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        fh_facebook getFacebook();
        
        
        virtual ~FacebookContactContext()
        {
        }
        void constructObject( fh_Contact contact )
        {
            m_contact = contact;
        }
        

        virtual std::string getRecommendedEA()
        {
            return adjustRecommendedEAForDotFiles(this, "name,first-name,last-name,location,id");
        }

        static fh_stringstream SL_getID( _Self* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_contact->getID();
                return ss;
            }
        static fh_stringstream SL_getName( _Self* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_contact->getName();
                return ss;
            }
        static fh_stringstream SL_getFirstName( _Self* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_contact->getFirstName();
                return ss;
            }
        static fh_stringstream SL_getLastName( _Self* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_contact->getLastName();
                return ss;
            }
        static fh_stringstream SL_getPicutreURL( _Self* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_contact->getPictureURL();
                return ss;
            }
        static fh_stringstream SL_getProfileURL( _Self* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_contact->getURL();
                return ss;
            }
        static fh_stringstream SL_getLocation( _Self* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_contact->getLocation();
                return ss;
            }

        
        
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );

#define SLEA  tryAddStateLessAttribute         

                    SLEA( "id",          &_Self::SL_getID,           XSD_BASIC_STRING );
                    SLEA( "full-name",   &_Self::SL_getName,         XSD_BASIC_STRING );
                    SLEA( "first-name",  &_Self::SL_getFirstName,    XSD_BASIC_STRING );
                    SLEA( "last-name",   &_Self::SL_getLastName,     XSD_BASIC_STRING );
                    SLEA( "picture-url", &_Self::SL_getPicutreURL,   XSD_BASIC_STRING );
                    SLEA( "profile-url", &_Self::SL_getProfileURL,   XSD_BASIC_STRING );
                    SLEA( "location",    &_Self::SL_getLocation,     XSD_BASIC_STRING );
                    
#undef SLEA                    
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
                ret << m_contact->getVCard();
                return ret;
            }
        

        ferris_ios::openmode
            getSupportedOpenModes()
            {
                return
                    ios_base::in        |
                    ios_base::binary    ;
            }

    };
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    class FERRISEXP_CTXPLUGIN FacebookContactsDirectoryContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        typedef FacebookContactsDirectoryContext _Self;
        
    public:

        FacebookContactsDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
        fh_facebook getFacebook();

        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                      << " have read:" << getHaveReadDir()
                      << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                
                if( !empty() )
                {
                    emitExistsEventForEachItemRAII _raii1( this );
                }
                else
                {
                    fh_facebook fb = getFacebook();

                    ContactMap_t friends = fb->getFriends();
                    for( ContactMap_t::iterator ci = friends.begin(); ci != friends.end(); ++ci )
                    {
                        fh_Contact contact = ci->second;
                        string rdn = contact->getRDN();
                        FacebookContactContext* c = 0;
                        c = priv_ensureSubContext( rdn, c );
                        c->constructObject( contact );
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

    class FERRISEXP_CTXPLUGIN FacebookStatusContext
        :
        public leafContext
    {
        typedef leafContext _Base;
        typedef FacebookStatusContext _Self;
        
    public:

        FacebookStatusContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }
        fh_facebook getFacebook();
        virtual ~FacebookStatusContext()
        {
        }

        virtual std::string getRecommendedEA()
            {
                return adjustRecommendedEAForDotFiles(this, "name,content");
            }

        fh_iostream
            priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                DEBUG << "priv_getIOStream(top) status" << endl;
                fh_stringstream ret = real_getIOStream( m );
                ret->getCloseSig().connect( sigc::bind( sigc::mem_fun(*this, &_Self::OnStreamClosed ), m )); 
                DEBUG << "priv_getIOStream(ret) status" << endl;
                return ret;
            }
        fh_istream
        priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ret = real_getIOStream( m );
                return ret;
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

        void
        OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
                DEBUG << "priv_OnStreamClosed(top)" << endl;
                
                if( !(m & std::ios::out) )
                    return;

                AdjustForOpenMode_Closing( ss, m, tellp );
                string data = StreamToString(ss);
                DEBUG << "OnStreamClosed() data:" << data << endl;

                fh_facebook fb = getFacebook();
                fb->setStatus( data );
            }

        fh_stringstream
        real_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   exception)
            {
                fh_facebook fb = getFacebook();
                fh_stringstream ss;
                ss << fb->getStatus();
                ss->clear();
                ss->seekg(0, ios::beg);
                ss->seekp(0, ios::beg);
                ss->clear();
                return ss;
            }
        
    };

    

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    class FERRISEXP_CTXPLUGIN FacebookRootContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        bool m_haveTriedToRead;
    public:

        FacebookRootContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_haveTriedToRead( false )
            {
                LG_FACEBOOK_D << "ctor, have read:" << getHaveReadDir() << endl;
            }

        fh_facebook getFacebook()
        {
            return Factory::getFacebook();
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

                    fh_facebook fb = getFacebook();
                    

                    {
                        FacebookStatusContext* c = 0;
                        c = priv_ensureSubContext( "status", c );
                    }
                    {
                        FacebookContactsDirectoryContext* c = 0;
                        c = priv_ensureSubContext( "contacts", c );
                    }
                    {
                        FacebookRecentStreamDirectoryContext* c = 0;
                        c = priv_ensureSubContext( "recent", c );
                    }
                    {
                        FacebookVideoDirectoryContext* c = 0;
                        c = priv_ensureSubContext( "videos", c );
                    }
                    {
                        FacebookPhotoDirectoryContext* c = 0;
                        c = priv_ensureSubContext( "photos", c );
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

    fh_facebook FacebookRecentStreamDirectoryContext::getFacebook()
    {
        FacebookRootContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p->getFacebook();
    }
    fh_facebook FacebookVideoDirectoryContext::getFacebook()
    {
        FacebookRootContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p->getFacebook();
    }
    fh_facebook FacebookVideoUploadFileContext::getFacebook()
    {
        FacebookRootContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p->getFacebook();
    }
    fh_facebook FacebookPhotoDirectoryContext::getFacebook()
    {
        FacebookRootContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p->getFacebook();
    }
    fh_facebook FacebookPhotoUploadFileContext::getFacebook()
    {
        FacebookRootContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p->getFacebook();
    }
    fh_facebook FacebookContactContext::getFacebook()
    {
        FacebookRootContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p->getFacebook();
    }
    fh_facebook FacebookContactsDirectoryContext::getFacebook()
    {
        FacebookRootContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p->getFacebook();
    }
    fh_facebook FacebookStatusContext::getFacebook()
    {
        FacebookRootContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p->getFacebook();
    }
    


    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                LG_FACEBOOK_D << "Brew()" << endl;

                static FacebookRootContext* c = 0;
                if( !c )
                {
                    c = new FacebookRootContext(0, "/" );
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
    
