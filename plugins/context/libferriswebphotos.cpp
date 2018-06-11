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

    $Id: WebPhotosContext.cpp,v 1.5 2008/05/24 21:30:07 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Ferris/Ferris.hh>
#include "libferriswebphotos_shared.hh"
#include <Ferris/Resolver_private.hh>
#include <Ferris/Enamel.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/Context_private.hh>

#define DEBUG LG_WEBPHOTO_D


using namespace std;

namespace Ferris
{

    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };
    

    std::string getTagEAPrefix()
    {
        return "tag:";
    }

    std::string stripTagEAPrefix( const std::string& s )
    {
        if( starts_with( s, "tag:" ) )
            return s.substr( strlen("tag:"));
        return s;
    }
    

    /**
     */
    class FERRISEXP_DLLLOCAL WebPhotosRootContext
        :
        public StateLessEAHolder< WebPhotosRootContext, FakeInternalContext >
    {
        typedef WebPhotosRootContext _Self;
        typedef StateLessEAHolder< WebPhotosRootContext, FakeInternalContext > _Base;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {}
        
    protected:

        virtual void priv_read();
        
    public:

        WebPhotosRootContext()
            :
            _Base( 0, "/" )
            {
                cerr << "WebPhotosRootContext()" << endl;
                DEBUG << "WebPhotosRootContext()" << endl;
                createStateLessAttributes();
            }
    
        
        virtual ~WebPhotosRootContext()
            {}
        
        virtual fh_context
        createSubContext( const std::string& rdn, fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
            {
                fh_stringstream ss;
                ss << "webphotos:// directory can not have new items created in this way" << endl;
                Throw_FerrisCreateSubContextNotSupported( tostr(ss), this );
            }

        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
            
    };
    
    /************************************************************/
    /************************************************************/
    /************************************************************/

    class FERRISEXP_DLLLOCAL WebPhotosObjectTopLevelContext;
    FERRIS_CTX_SMARTPTR( WebPhotosObjectTopLevelContext, fh_WebPhotosObjectTopLevelContext );
    class FERRISEXP_DLLLOCAL WebPhotosObjectTopLevelContext
        :
        public StateLessEAHolder< WebPhotosObjectTopLevelContext, FakeInternalContext >
    {
        typedef StateLessEAHolder< WebPhotosObjectTopLevelContext, FakeInternalContext > _Base;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {}

        fh_webPhotos m_wf;
        
    protected:

        virtual void priv_read();

    public:

        WebPhotosObjectTopLevelContext( Context* parent,
                                        const std::string& rdn,
                                        fh_webPhotos wf )
            :
            _Base( parent, rdn ),
            m_wf( wf )
            {
                createStateLessAttributes();
            }
        
        virtual ~WebPhotosObjectTopLevelContext()
            {}


        virtual fh_context
        createSubContext( const std::string& rdn, fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
            {
                fh_stringstream ss;
                ss << "webphotos:// directory can not have new items created in this way" << endl;
                Throw_FerrisCreateSubContextNotSupported( tostr(ss), this );
            }

        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }

        fh_webPhotos getWebPhotos()
            {
                return m_wf;
            }
    };


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_DLLLOCAL WebPhotosUploadFileContext;
    FERRIS_CTX_SMARTPTR( WebPhotosUploadFileContext, fh_WebPhotosUploadFileContext );
    class FERRISEXP_DLLLOCAL WebPhotosUploadFileContext
        :
        public StateLessEAHolder< WebPhotosUploadFileContext, leafContext >
    {
        typedef StateLessEAHolder< WebPhotosUploadFileContext, leafContext > _Base;
        typedef WebPhotosUploadFileContext _Self;

        string m_id;
        long long m_ContentLength;
        string m_explicitUploadFilename;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
                LG_WEBPHOTO_D << "upload file. setting file creation schema" << endl;

                m["ea"] = SubContextCreator(
                    SL_SubCreate_ea,
                    "	<elementType name=\"ea\">\n"
                    "		<elementType name=\"name\" default=\"new ea\">\n"
                    "			<dataTypeRef name=\"string\"/>\n"
                    "		</elementType>\n"
                    "		<elementType name=\"value\" default=\"\">\n"
                    "			<dataTypeRef name=\"string\"/>\n"
                    "		</elementType>\n"
                    "	</elementType>\n");
            }


        fh_iostream
        getNullEAStream( Context*, const std::string&, EA_Atom* attr )
            {
                fh_stringstream ss;
                return ss;
            }
        void
        NullEAStreamClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
            {
            }
        

        void addNullAttr( const std::string& rdn )
            {
                addAttribute( rdn,
                              this, &_Self::getNullEAStream,
                              this, &_Self::getNullEAStream,
                              this, &_Self::NullEAStreamClosed,
                              XSD_BASIC_STRING );
            }
        

        
        fh_context
        SubCreate_ea( fh_context c, fh_context md )
            {
                LG_WEBPHOTO_D << "SubCreate_ea(1)" << endl;
                string rdn = getStrSubCtx( md, "name", "", true, true );
                addNullAttr( rdn );
                return c;
            }
        
        fh_webPhotoUpload m_webPhotosUpload;
        
    public:

        
        fh_webPhotos getWebPhotos();
        fh_webPhotoUpload getWebPhotoUpload()
        {
            if( !m_webPhotosUpload )
            {
                m_webPhotosUpload = new WebPhotoUpload( getWebPhotos() );
                m_webPhotosUpload->appendDescriptionFromEA( "description" );
                m_webPhotosUpload->appendDescriptionFromEA( "annotation" );
            }
            return m_webPhotosUpload;
        }
        
        WebPhotosUploadFileContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_id(""),
            m_webPhotosUpload( 0 ),
            m_ContentLength( 0 )
            {
                createStateLessAttributes();

                addNullAttr( "user-owner-number" );
                addNullAttr( "group-owner-number" );
                addNullAttr( "ferris-type" );
                addNullAttr( "dontfollow-selinux-context" );
                addNullAttr( "mode" );
                addNullAttr( "mtime" );
                addNullAttr( "atime" );
                addNullAttr( "ctime" );
            }
        
        virtual ~WebPhotosUploadFileContext()
            {}

        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );

                    
                    supplementStateLessAttributes( true );
                }
            }

        fh_stringstream
        real_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   exception)
            {
                fh_stringstream ss;
                LG_WEBPHOTO_D << "returning an empty stream" << endl;
                return ss;
            }

        bool shouldStreamUpload()
        {
            fh_webPhotos wf = getWebPhotos();
            long MaxDesiredWidthOrHeight = wf->getDefaultLargestDimension();
            return !MaxDesiredWidthOrHeight;
        }
        
        virtual void priv_preCopyAction( fh_context c )
            {
                LG_WEBPHOTO_D << "preCopyAction() c:" << c->getURL()
                              << " m_id:" << m_id
                              << endl;

                // Force collection.
                fh_webPhotoUpload wu = getWebPhotoUpload();
                fh_webPhotos wf = getWebPhotos();
                LG_WEBPHOTO_D << "preCopyAction() wu:" << isBound(wu)
                              << " wf:" << isBound(wf)
                              << " m_webPhotosUpload:" << isBound(m_webPhotosUpload)
                              << endl;
                if( m_webPhotosUpload )
                {
                    fh_webPhotoUpload wu = getWebPhotoUpload();
                    wu->inspectSourceForMetadata( c );
                    m_ContentLength = toint( getStrAttr( c, "size", "200" ));
                    LG_WEBPHOTO_D << "m_ContentLength:" << m_ContentLength << endl;
                }
                m_explicitUploadFilename = getStrAttr( c, "upload-filename", "" );
            }
        
        virtual void priv_postCopyAction( fh_context c )
            {
                LG_WEBPHOTO_D << "postCopyAction() c:" << c->getURL()
                              << " m_id:" << m_id
                              << endl;

                if( m_webPhotosUpload )
                {
                
                ////////////////////////////
                ////////////////////////////
                ////////////////////////////
                // FIXME: remove this block when closeSig works properly again.
                //
                // cerr  << "postCopyAction() c:" << c->getURL() << endl;
                // DEBUG << "postCopyAction() c:" << c->getURL() << endl;
                // if( m_webPhotosUpload )
                // {
                //     fh_webPhotos wf = getWebPhotos();
                //     fh_webPhotoUpload wu = getWebPhotoUpload();
                //     wu->streamingUploadComplete();
                
                //     {
                //         LG_WEBPHOTO_D << "OnStreamClosed() upload-list.sz:"
                //                       << wu->getUploadedPhotoIDList().size() << endl;
                //         stringlist_t& sl = wu->getUploadedPhotoIDList();
                //         if( !sl.empty() )
                //         {
                //             stringlist_t::iterator iter = sl.end();
                //             --iter;
                //             m_id = *iter;
                //             LG_WEBPHOTO_D << "OnStreamClosed() setting m_id:" << m_id << endl;
                //         }
                //     }
                // }
                ////////////////////////////
                ////////////////////////////
                ////////////////////////////
                }

                if( m_id.empty() )
                {
                    LG_WEBPHOTO_W << "postCopyAction() c:" << c->getURL()
                                  << " no web photo ID! not tagging remote file."
                                  << endl;
                    return;
                }
                
                fh_webPhotos wf = getWebPhotos();
                // Save off the photo-id on the source image for later use
                try
                {
                    string sn = wf->getImplementationShortName();
                    setStrAttr( c, sn + "-photo-id", m_id, true, true );
                    setStrAttr( c, "webphoto-service", sn, true, true );
                    setStrAttr( c, "webphoto-photo-id", m_id, true, true );
                }
                catch( exception& e )
                {
                    LG_WEBPHOTO_D << "Error linking source with photo id:" << e.what() << endl;
                }
                
                
                fh_webPhotoUpload wu = new WebPhotoUpload( wf );
                string tags = wu->getTagString( c );

                string id = m_id;
                
                stringmap_t args;
                args["method"]   = "flickr.photos.addTags";
                args["tags"]     = tags;
                args["photo_id"] = id;

                LG_WEBPHOTO_D << "postCopyAction() photo_id:" << id << endl;
                
                try
                {
                    wu->post( args );
                }
                catch( exception& e )
                {
                    stringstream ss;
                    ss << "Error setting tags for image:" << c->getURL() << endl
                       << " reason:" << e.what() << endl;
                    cerr << ss.str() << endl;
                    LG_WEBPHOTO_W << ss.str() << endl;
                }
            }
        
        void
        OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
                cerr << "OnStreamClosed()" << endl;
                
                if( !(m & std::ios::out) )
                    return;
                
                AdjustForOpenMode_Closing( ss, m, tellp );
                const string s = StreamToString(ss);

                fh_webPhotos wf = getWebPhotos();
                fh_webPhotoUpload wu = new WebPhotoUpload( wf );

                bool Verbose = true;
                bool DebugVerbose = true;
                if( Verbose )
                    wu->setVerboseStream( Factory::fcout() );
                wu->setDebugVerbose( DebugVerbose );

                string extension = "jpg";
                int dotpos = getDirName().rfind('.');
                if( dotpos != string::npos )
                    extension = getDirName().substr( dotpos );
                string tmp_url = Shell::getTmpDirPath() + (string)"/libferris-webupload-fs-temp-image." + extension;
                LG_WEBPHOTO_D << "tmp_url:" << tmp_url << endl;
                fh_context c = Shell::acquireContext( tmp_url, 600, false );
                {
                    fh_iostream oss = c->getIOStream( ios::out | ios::trunc );
                    oss << s << flush;
                }

                wu->setPublicViewable( wf->isDefaultImageProtectionPublic() );
                wu->setFriendViewable( wf->isDefaultImageProtectionFriend() );
                wu->setFamilyViewable( wf->isDefaultImageProtectionFamily() );
                
                
                LG_WEBPHOTO_D << "OnStreamClosed() c:" << c->getURL() << endl;
                string uploadFilename = getDirName();
                if( !m_explicitUploadFilename.empty() )
                {
                    uploadFilename = m_explicitUploadFilename;
                }
                LG_WEBPHOTO_D << " uploadFilename:" << uploadFilename << endl;

                
                wu->upload( c, uploadFilename );

                
                LG_WEBPHOTO_D << "OnStreamClosed() s.sz:" << s.size() << endl;
                
                {
                    LG_WEBPHOTO_D << "OnStreamClosed() upload-list.sz:"
                                  << wu->getUploadedPhotoIDList().size() << endl;
                    stringlist_t& sl = wu->getUploadedPhotoIDList();
                    if( !sl.empty() )
                    {
                        stringlist_t::iterator iter = sl.end();
                        --iter;
                        m_id = *iter;
                        LG_WEBPHOTO_D << "OnStreamClosed() setting m_id:" << m_id << endl;
                    }
                }
            }
        
        ferris_ios::openmode
        getSupportedOpenModes()
            {
                return ios::in | ios::out | ios::trunc | ios::binary;
            }


        // fh_iostream
        // priv_getIOStream( ferris_ios::openmode m )
        //     throw (FerrisParentNotSetError,
        //            AttributeNotWritable,
        //            CanNotGetStream,
        //            std::exception)
        //     {
        //         fh_stringstream ret = real_getIOStream( m );
        //         ret->getCloseSig().connect( bind( sigc::mem_fun(*this, &_Self::OnStreamClosed ), m )); 
        //         return ret;
        //     }
        fh_istream
        priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ret = real_getIOStream( m );
                return ret;
            }
        
        
        
        void
        OnStreamingWriteClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
                cerr << "OnStreamingWriteClosed()" << endl;

                if( !(m & std::ios::out) )
                    return;

                fh_webPhotos wf = getWebPhotos();
                fh_webPhotoUpload wu = getWebPhotoUpload();
                
                wu->streamingUploadComplete();
                
                {
                    LG_WEBPHOTO_D << "OnStreamClosed() upload-list.sz:"
                                  << wu->getUploadedPhotoIDList().size() << endl;
                    stringlist_t& sl = wu->getUploadedPhotoIDList();
                    if( !sl.empty() )
                    {
                        stringlist_t::iterator iter = sl.end();
                        --iter;
                        m_id = *iter;
                        LG_WEBPHOTO_D << "OnStreamClosed() setting m_id:" << m_id << endl;
                    }
                }
            }
        

        

        fh_iostream
            priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                cerr << "getIOStream A" << endl;
                fh_webPhotos wf = getWebPhotos();

                // If the user wants to only upload a scaled image,
                // we can't stream it so have to fall back
                // to scaling to a /tmp image and sending that.
                long MaxDesiredWidthOrHeight = wf->getDefaultLargestDimension();
                cerr << "MaxDesiredWidthOrHeight:" << MaxDesiredWidthOrHeight << endl;

                if( !shouldStreamUpload() )
                {
                    cerr << "Using old, non streaming upload because of scaling..." << endl;
                    fh_stringstream ret = real_getIOStream( m );
                    ret->getCloseSig().connect(
                        sigc::bind( sigc::mem_fun(*this, &_Self::OnStreamClosed ), m )); 
                    return ret;
                }
                
                fh_webPhotoUpload wu = getWebPhotoUpload();

                
                wu->setPublicViewable( wf->isDefaultImageProtectionPublic() );
                wu->setFriendViewable( wf->isDefaultImageProtectionFriend() );
                wu->setFamilyViewable( wf->isDefaultImageProtectionFamily() );
                
                int ContentLength = 200;
                if( m_ContentLength )
                    ContentLength = m_ContentLength;
                LG_WEBPHOTO_D << "ContentLength:" << ContentLength << endl;
                string title = "title";
                string desc  = "desc";

                fh_iostream ret = wu->getUploadIOStream( ContentLength, title, desc );
                ret->getCloseSig().connect(
                    sigc::bind( sigc::mem_fun(*this, &_Self::OnStreamingWriteClosed ), m )); 
                return ret;
            }
        
        
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_DLLLOCAL WebPhotosUploadDirectoryContext;
    FERRIS_CTX_SMARTPTR( WebPhotosUploadDirectoryContext, fh_WebPhotosUploadDirectoryContext );
    class FERRISEXP_DLLLOCAL WebPhotosUploadDirectoryContext
        :
        public StateLessEAHolder< WebPhotosUploadDirectoryContext, FakeInternalContext >
    {
        typedef StateLessEAHolder< WebPhotosUploadDirectoryContext, FakeInternalContext > _Base;

        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
                LG_WEBPHOTO_D << "upload dir. setting file creation schema" << endl;
                m["file"] = SubContextCreator(SL_SubCreate_file,
                                              "	<elementType name=\"file\">\n"
                                              "		<elementType name=\"name\" default=\"new file\">\n"
                                              "			<dataTypeRef name=\"string\"/>\n"
                                              "		</elementType>\n"
                                              "	</elementType>\n");
            }
        
    protected:

    void priv_read()
    {
        staticDirContentsRAII _raii1( this );

        if( empty() )
        {
            // We have to setup the children the first time..
            fh_context child = 0;
        }
    }
        
    public:

        fh_webPhotos getWebPhotos()
            {
                WebPhotosObjectTopLevelContext* p = 0;
                p = getFirstParentOfContextClass( p );
                return p->getWebPhotos();
            }
        
        WebPhotosUploadDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }
        
        virtual ~WebPhotosUploadDirectoryContext()
            {}

        bool isDir()
            {
                return true;
            }
        
        

        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }

        
        virtual fh_context SubCreate_file( fh_context c, fh_context md )
            {
                string rdn = getStrSubCtx( md, "name", "" );
                LG_WEBPHOTO_D << "create_file for rdn:" << rdn << endl;
                
                fh_context child = 0;
                child = new WebPhotosUploadFileContext( this, rdn );
                Insert( GetImpl(child), false, true );

                
                LG_WEBPHOTO_D << "create_file OK for rdn:" << rdn << endl;
                return child;
            }
        
        
    };


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_DLLLOCAL WebPhotosCommentContext;
    FERRIS_CTX_SMARTPTR( WebPhotosCommentContext, fh_WebPhotosCommentContext );
    class FERRISEXP_DLLLOCAL WebPhotosCommentContext
        :
        public StateLessEAHolder< WebPhotosCommentContext, leafContext >
    {
        typedef WebPhotosCommentContext _Self;
        typedef StateLessEAHolder< WebPhotosCommentContext, leafContext > _Base;

       
    protected:

        fh_Comment m_comment;
 
        ferris_ios::openmode
        getSupportedOpenModes()
            {
                return ios::in | ios::out | ios::trunc | ios::binary;
            }

        fh_stringstream
        real_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   exception)
            {
                fh_stringstream ss;
                ss << m_comment->getContent();
                return ss;
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
        fh_iostream
            priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
        {
            fh_stringstream ret = real_getIOStream( m );
            ret->getCloseSig().connect( sigc::bind( sigc::mem_fun(*this, &_Self::OnStreamClosed ), m )); 
            return ret;
        }
        
        void OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
        {
            if( !(m & std::ios::out) )
                return;

            AdjustForOpenMode_Closing( ss, m, tellp );
            const string s = StreamToString(ss);

            m_comment->setContent(s);
        }
        
        
    public:

        static string getRDN( fh_Comment c )
        {
            return c->getID();
        }
    
        fh_webPhotos getWebPhotos()
            {
                WebPhotosObjectTopLevelContext* p = 0;
                p = getFirstParentOfContextClass( p );
                return p->getWebPhotos();
            }
        
        WebPhotosCommentContext( Context* parent, const std::string& rdn, fh_Comment md )
            :
            _Base( parent, rdn ),
            m_comment( md )
            {
                createStateLessAttributes();
                DEBUG << "WebPhotosCommentContext() id:" << md->getID() << endl;
                
            }
        
        virtual ~WebPhotosCommentContext()
            {}

        bool isDir()
            {
                return false;
            }

        static fh_stringstream SL_getSize( WebPhotosCommentContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_comment->getContent().size();
                return ss;
            }

        static fh_stringstream SL_getID( WebPhotosCommentContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_comment->getID();
                return ss;
            }

        static fh_stringstream SL_getAuthor( WebPhotosCommentContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_comment->getAuthorName();
                return ss;
            }

        static fh_stringstream SL_getMTime( WebPhotosCommentContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_comment->getDateCreated();
                return ss;
            }
        
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );

#define SLEA  tryAddStateLessAttribute         

                    SLEA( "size",     &_Self::SL_getSize,    FXD_FILESIZE );
                    SLEA( "id",       &_Self::SL_getID,      XSD_BASIC_STRING );
                    SLEA( "author",   &_Self::SL_getAuthor,  XSD_BASIC_STRING );
                    SLEA( "mtime",    &_Self::SL_getMTime,   FXD_UNIXEPOCH_T );
                    
#undef SLEA                    
                    supplementStateLessAttributes( true );
                }
            }

        virtual std::string getRecommendedEA()
            {
//                return _Base::getRecommendedEA() + ",name,size,id,author,mtime-display,content,";
                return "name,id,author,mtime-display,size,content,";
            }
    };



    class FERRISEXP_DLLLOCAL WebPhotosCommentsDirectoryContext;
    FERRIS_CTX_SMARTPTR( WebPhotosCommentsDirectoryContext, fh_WebPhotosCommentsDirectoryContext );
    class FERRISEXP_DLLLOCAL WebPhotosCommentsDirectoryContext
        :
        public StateLessEAHolder< WebPhotosCommentsDirectoryContext, FakeInternalContext >
    {
        typedef WebPhotosCommentsDirectoryContext _Self;
        typedef StateLessEAHolder< WebPhotosCommentsDirectoryContext, FakeInternalContext > _Base;

        fh_PhotoMetadata m_photo;
        
    protected:

        
        void priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    fh_webPhotos wf = getWebPhotos();
                    CommentList_t col = m_photo->getComments();
                    DEBUG << "priv_read() col.sz:" << col.size() << endl;
                    for( rs<CommentList_t> pi( col ); pi; ++pi )
                    {
                        fh_Comment com = *pi;
                        
                        string rdn = WebPhotosCommentContext::getRDN( com );
                        DEBUG << "rdn:" << rdn << endl;
                        fh_context child = new WebPhotosCommentContext( this, rdn, com );
                        addNewChild( child );
                    }
                }
            }

        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
                LG_WEBPHOTO_D << "upload dir. setting file creation schema" << endl;
                m["file"] = SubContextCreator(SL_SubCreate_file,
                                              "	<elementType name=\"file\">\n"
                                              "		<elementType name=\"name\" default=\"new file\">\n"
                                              "			<dataTypeRef name=\"string\"/>\n"
                                              "		</elementType>\n"
                                              "	</elementType>\n");
            }

        virtual fh_context SubCreate_file( fh_context c, fh_context md )
            {
                string xdn = getStrSubCtx( md, "name", "" );
                string content = getStrSubCtx( md, "content", "new" );
                DEBUG << "create_file (comment) for xdn:" << xdn << endl;
                DEBUG << "create_file (comment) with content:" << content << ":" << endl;

                fh_webPhotos wf = getWebPhotos();
                fh_Comment com = Comment::Create( wf, m_photo, content );
                DEBUG << "create_file (comment) have comment:" << com->getID() << endl;

                string rdn = WebPhotosCommentContext::getRDN( com );
                Items_t::iterator isSubContextBoundCache;
                if( priv_isSubContextBound( rdn, isSubContextBoundCache ) )
                {
                    return *isSubContextBoundCache;
                }
                DEBUG << "rdn:" << rdn << endl;
                fh_context child = new WebPhotosCommentContext( this, rdn, com );
                addNewChild( child );
                
                LG_WEBPHOTO_D << "create_file OK for rdn:" << rdn << endl;
                return child;
            }
        
    public:

        fh_webPhotos getWebPhotos()
        {
            WebPhotosObjectTopLevelContext* p = 0;
            p = getFirstParentOfContextClass( p );
            return p->getWebPhotos();
        }

        void constructObject( fh_PhotoMetadata photo )
        {
            m_photo = photo;
        }

        WebPhotosCommentsDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
        {
            createStateLessAttributes();
        }
        
        virtual ~WebPhotosCommentsDirectoryContext()
        {}

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

                SLEA( "webphoto-id", &_Self::SL_getNothingStream, XSD_BASIC_INT );
                SLEA( "size", &_Self::SL_getNothingStream, XSD_BASIC_INT );
                SLEA( "id", &_Self::SL_getNothingStream, XSD_BASIC_INT );
                SLEA( "author", &_Self::SL_getNothingStream, XSD_BASIC_INT );
                SLEA( "mtime-display", &_Self::SL_getNothingStream, XSD_BASIC_INT );
                SLEA( "mtime", &_Self::SL_getNothingStream, XSD_BASIC_INT );
                SLEA( "content", &_Self::SL_getNothingStream, XSD_BASIC_INT );

#undef SLEA                    
                    
                supplementStateLessAttributes( true );
            }
        }

        virtual std::string getRecommendedEA()
        {
//            return _Base::getRecommendedEA() + ",name,size,id,author,mtime-display,content,";
            return "name,";
        }
        virtual fh_istream getRecommendedEAUnionView()
        {
            fh_stringstream ss;
            ss << "name,";
            return ss;
        }
    };

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    
    class FERRISEXP_DLLLOCAL WebPhotosFromPhotoMetadataContext;
    FERRIS_CTX_SMARTPTR( WebPhotosFromPhotoMetadataContext, fh_WebPhotosFromPhotoMetadataContext );
    class FERRISEXP_DLLLOCAL WebPhotosFromPhotoMetadataContext
        :
        public StateLessEAHolder< WebPhotosFromPhotoMetadataContext, leafContext >
    {
        typedef WebPhotosFromPhotoMetadataContext _Self;
        typedef StateLessEAHolder< WebPhotosFromPhotoMetadataContext, leafContext > _Base;

       
    protected:

        fh_PhotoMetadata m_md;

        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
                DEBUG << "x" << endl;

                m["ea"] = SubContextCreator(
                    SL_SubCreate_ea,
                    "	<elementType name=\"ea\">\n"
                    "		<elementType name=\"name\" default=\"new ea\">\n"
                    "			<dataTypeRef name=\"string\"/>\n"
                    "		</elementType>\n"
                    "		<elementType name=\"value\" default=\"\">\n"
                    "			<dataTypeRef name=\"string\"/>\n"
                    "		</elementType>\n"
                    "	</elementType>\n");
            }
        fh_context
        SubCreate_ea( fh_context c, fh_context md )
            {
                DEBUG << "SubCreate_ea(1)" << endl;
                string rdn = getStrSubCtx( md, "name", "", true, true );
                DEBUG << "SubCreate_ea() rdn:" << rdn << endl;

                string t = stripTagEAPrefix( rdn );
                fh_Tag tag = m_md->addTag( t );

                string eaname = getTagEAPrefix() + t;
                DEBUG << "adding attribute eaname:" << eaname << endl;
                    
                addAttribute( eaname,
                              this, &_Self::getTagStream,
                              this, &_Self::getTagStream,
                              this, &_Self::OnTagStreamClosed,
                              XSD_BASIC_BOOL );
                return c;
            }
        
        

        ferris_ios::openmode
        getSupportedOpenModes()
            {
                return ios::in | ios::binary;
            }

        
        
        fh_istream
        priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                LG_WEBPHOTO_D << "getting remote image as large as possible..." << endl;

                string earl = m_md->getLargestSizeImageURL();
                                
                LG_WEBPHOTO_D << "original image URL:" << earl << endl;

                fh_context ret = Resolve( earl );
                return ret->getIStream( m );
            }
        
    public:

        static string getRDN( fh_PhotoMetadata pm )
        {
            string ret = pm->getTitle();
//            if( ret.empty() )
                ret = tostr(pm->getID());
            DEBUG << "getRDN() ret:" << ret << endl;
            return ret;
        }
    
        fh_webPhotos getWebPhotos()
            {
                WebPhotosObjectTopLevelContext* p = 0;
                p = getFirstParentOfContextClass( p );
                return p->getWebPhotos();
            }


        fh_iostream getTagStream( Context*, const std::string&, EA_Atom* attr )
        {
            fh_stringstream ss;
            ss << "1";
            return ss;
        }
        void OnTagStreamClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
        {
            const string s = StreamToString(ss);
            string tagname = stripTagEAPrefix( rdn );

            DEBUG << "Setting tag:" << tagname << " to state:" << s << endl;

            if( isFalse(s) )
            {
                m_md->removeTag( tagname );
            }
            else
            {
                m_md->addTag( tagname );
            }
        }
        
        WebPhotosFromPhotoMetadataContext( Context* parent, const std::string& rdn, fh_PhotoMetadata md )
            :
            _Base( parent, rdn ),
            m_md( md )
            {
                createStateLessAttributes();
                DEBUG << "WebPhotosFromPhotoMetadataContext(top) id:" << md->getID() << endl;

                TagList_t col = md->getTags();
                for( TagList_t::iterator ti = col.begin(); ti != col.end(); ++ti )
                {
                    fh_Tag t = *ti;

                    string eaname = getTagEAPrefix() + t->getContent();
                    DEBUG << "adding attribute eaname:" << eaname << endl;
//                    addAttribute( eaname, "1" );
                    
                    addAttribute( eaname,
                                  this, &_Self::getTagStream,
                                  this, &_Self::getTagStream,
                                  this, &_Self::OnTagStreamClosed,
                                  XSD_BASIC_BOOL );
                    
                }
                DEBUG << "WebPhotosFromPhotoMetadataContext(bottom) id:" << md->getID() << endl;
            }
        
        virtual ~WebPhotosFromPhotoMetadataContext()
            {}

        bool isDir()
            {
                return false;
            }

        static fh_stringstream SL_getID( WebPhotosFromPhotoMetadataContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_md->getID();
                LG_WEBPHOTO_D << "SL_getID() ret:" << tostr(ss) << endl;
                return ss;
            }

        static fh_stringstream SL_getSecret( WebPhotosFromPhotoMetadataContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_md->getSecret();
                return ss;
            }

        static fh_stringstream SL_getTitle( WebPhotosFromPhotoMetadataContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_md->getTitle();
                LG_WEBPHOTO_D << "SL_getTitle() ret:" << tostr(ss) << endl;
                return ss;
            }

        static fh_stringstream SL_getServer( WebPhotosFromPhotoMetadataContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_md->getServer();
                return ss;
            }

        static fh_stringstream SL_isPrimary( WebPhotosFromPhotoMetadataContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_md->isPrimary();
                return ss;
            }

        static fh_stringstream SL_getSize( WebPhotosFromPhotoMetadataContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_md->getLargestSizeImageSize();
                return ss;
            }
        static fh_stringstream SL_getMTime( WebPhotosFromPhotoMetadataContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_md->getMTime();
                return ss;
            }
        static fh_stringstream SL_getLargestSizeImageURL( WebPhotosFromPhotoMetadataContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss << c->m_md->getLargestSizeImageURL();
                return ss;
            }
        static fh_stringstream SL_getThumbnailRGBAData( WebPhotosFromPhotoMetadataContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss.write( (const char*)c->m_md->getThumbnailRGBAData(),
                          c->m_md->getThumbnailRGBADataSize() );
                return ss;
            }
        static fh_stringstream SL_getFullRGBAData( WebPhotosFromPhotoMetadataContext* c, const std::string&, EA_Atom*)
            {
                fh_stringstream ss;
                ss.write( (const char*)c->m_md->getFullRGBAData(),
                          c->m_md->getFullRGBADataSize() );
                return ss;
            }

        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );

#define SLEA  tryAddStateLessAttribute         

                    SLEA( "webphoto-id", &_Self::SL_getID, XSD_BASIC_INT );
                    SLEA( "id",          &_Self::SL_getID, XSD_BASIC_INT );
                    SLEA( "secret", &_Self::SL_getSecret,  XSD_BASIC_STRING );
                    SLEA( "title",  &_Self::SL_getTitle,   XSD_BASIC_STRING );
                    SLEA( "server", &_Self::SL_getServer,  XSD_BASIC_STRING );
                    SLEA( "is-primary", &_Self::SL_isPrimary, XSD_BASIC_BOOL );
                    SLEA( "size",   &_Self::SL_getSize,    FXD_FILESIZE );
                    SLEA( "mtime",  &_Self::SL_getMTime,   FXD_UNIXEPOCH_T );

                    SLEA( "link-target",  &_Self::SL_getLargestSizeImageURL, XSD_BASIC_STRING );
                    SLEA( "rgba-32bpp",   &_Self::SL_getFullRGBAData, FXD_BINARY_RGBA32 );
                    SLEA( "exif:thumbnail-rgba-32bpp", &_Self::SL_getThumbnailRGBAData, FXD_BINARY_RGBA32 );
                    
#undef SLEA                    
                    supplementStateLessAttributes( true );
                }
            }

        virtual std::string getRecommendedEA()
            {
//                return _Base::getRecommendedEA() + ",webphoto-id,title,mtime-display";
                return "name,webphoto-id,title,mtime-display,";
            }
        
        
        
        
    };


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_DLLLOCAL WebPhotosRecentDirectoryContext;
    FERRIS_CTX_SMARTPTR( WebPhotosRecentDirectoryContext, fh_WebPhotosRecentDirectoryContext );
    class FERRISEXP_DLLLOCAL WebPhotosRecentDirectoryContext
        :
        public StateLessEAHolder< WebPhotosRecentDirectoryContext, FakeInternalContext >
    {
        typedef WebPhotosRecentDirectoryContext _Self;
        typedef StateLessEAHolder< WebPhotosRecentDirectoryContext, FakeInternalContext > _Base;

    protected:

        void priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    fh_webPhotos wf = getWebPhotos();
                    LG_WEBPHOTO_D << "recent/priv_read()" << endl;
                    photolist_t pl = wf->getMyRecent();
                    for( rs<photolist_t> pi( pl ); pi; ++pi )
                    {
                        string rdn = WebPhotosFromPhotoMetadataContext::getRDN( *pi );
                        fh_context child = new WebPhotosFromPhotoMetadataContext( this, rdn, *pi );
                        addNewChild( child );
                    }
                }
            }
        
    public:

        fh_webPhotos getWebPhotos()
        {
            WebPhotosObjectTopLevelContext* p = 0;
            p = getFirstParentOfContextClass( p );
            return p->getWebPhotos();
        }


        WebPhotosRecentDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
        {
            createStateLessAttributes();
        }
        
        virtual ~WebPhotosRecentDirectoryContext()
        {}

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

                SLEA( "webphoto-id", &_Self::SL_getNothingStream, XSD_BASIC_INT );
                SLEA( "secret", &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "title",  &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "server", &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "is-primary", &_Self::SL_getNothingStream, XSD_BASIC_BOOL );

#undef SLEA                    
                    
                supplementStateLessAttributes( true );
            }
        }

        virtual std::string getRecommendedEA()
        {
            return _Base::getRecommendedEA() + ",webphoto-id,title,";
        }
        

    };

    

    /************************************************************/
    /************************************************************/
    /************************************************************/
    /************************************************************/
    /************************************************************/
    /************************************************************/


    class FERRISEXP_DLLLOCAL WebPhotosByPhotoIDDirectoryContext;
    FERRIS_CTX_SMARTPTR( WebPhotosByPhotoIDDirectoryContext, fh_WebPhotosByPhotoIDDirectoryContext );
    class FERRISEXP_DLLLOCAL WebPhotosByPhotoIDDirectoryContext
        :
        public StateLessEAHolder< WebPhotosByPhotoIDDirectoryContext, FakeInternalContext >
    {
        typedef WebPhotosByPhotoIDDirectoryContext _Self;
        typedef StateLessEAHolder< WebPhotosByPhotoIDDirectoryContext, FakeInternalContext > _Base;

    protected:

        void priv_read()
            {
                staticDirContentsRAII _raii1( this );
            }

        fh_context
            priv_getSubContext( const string& rdn )
            throw( NoSuchSubContext )
        {
            try
            {
                Items_t::iterator isSubContextBoundCache;
                if( priv_isSubContextBound( rdn, isSubContextBoundCache ) )
                {
                    return *isSubContextBoundCache;
                }
                
                fh_webPhotos wf = getWebPhotos();
                fh_PhotoMetadata m = wf->getPhotoByID( rdn );
//                string xdn = WebPhotosFromPhotoMetadataContext::getRDN( m );
                string xdn = rdn;
                DEBUG << "adding new photo rdn:" << xdn << " for given ID:" << rdn << endl;
                fh_context child = new WebPhotosFromPhotoMetadataContext( this, xdn, m );
                addNewChild( child );
                return child;
            }
            catch(...)
            {}
            fh_stringstream ss;
            ss << "NoSuchSubContext:" << rdn;
            Throw_NoSuchSubContext( tostr(ss), this );
        }
        
    public:

        fh_webPhotos getWebPhotos()
        {
            WebPhotosObjectTopLevelContext* p = 0;
            p = getFirstParentOfContextClass( p );
            return p->getWebPhotos();
        }


        WebPhotosByPhotoIDDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
        {
            createStateLessAttributes();
        }
        
        virtual ~WebPhotosByPhotoIDDirectoryContext()
        {}

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

                SLEA( "webphoto-id", &_Self::SL_getNothingStream, XSD_BASIC_INT );
                SLEA( "secret", &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "title",  &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "server", &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "is-primary", &_Self::SL_getNothingStream, XSD_BASIC_BOOL );

#undef SLEA                    
                    
                supplementStateLessAttributes( true );
            }
        }

        virtual std::string getRecommendedEA()
        {
            return _Base::getRecommendedEA() + ",webphoto-id,title,";
        }
        

    };
    

    /************************************************************/
    /************************************************************/
    /************************************************************/
    /************************************************************/
    /************************************************************/
    /************************************************************/



    /************************************************************/
    /************************************************************/
    /************************************************************/
    


    class FERRISEXP_DLLLOCAL WebPhotosetsDirectoryContext;
    FERRIS_CTX_SMARTPTR( WebPhotosetsDirectoryContext, fh_WebPhotosetsDirectoryContext );
    class FERRISEXP_DLLLOCAL WebPhotosetsDirectoryContext
        :
        public StateLessEAHolder< WebPhotosetsDirectoryContext, FakeInternalContext >
    {
        typedef WebPhotosetsDirectoryContext _Self;
        typedef StateLessEAHolder< WebPhotosetsDirectoryContext, FakeInternalContext > _Base;

        string      m_userID;
        fh_PhotoSet m_photoset;
        photolist_t m_photolist;
        
    protected:

        std::string getUserID();
        
        
        void priv_read()
        {
            staticDirContentsRAII _raii1( this );

            if( empty() )
            {
                fh_webPhotos wf = getWebPhotos();
                LG_WEBPHOTO_D << "priv_read()" << endl;

                if( m_photoset || !m_photolist.empty() )
                {
                    photolist_t pl = m_photolist;
                    if( pl.empty() )
                        pl = m_photoset->getPhotos();
                    for( rs<photolist_t> pi( pl ); pi; ++pi )
                    {
//                        string photoid = (*pi)->getID();
                        
                        string rdn = WebPhotosFromPhotoMetadataContext::getRDN( *pi );
                        fh_context child = new WebPhotosFromPhotoMetadataContext( this, rdn, *pi );
                        addNewChild( child );

                        {
                            string commentdir = rdn + "_comments";
                            WebPhotosCommentsDirectoryContext* c = 0;
                            c = priv_ensureSubContext( commentdir, c );
                            c->constructObject( *pi );
                        }
                    }
                }
                else
                {
                    photosets_t col = wf->getPhotosets( getUserID() );
                    for( rs<photosets_t> pi( col ); pi; ++pi )
                    {
                        fh_PhotoSet pset = *pi;
                        string rdn = pset->getTitle();
                        DEBUG << "rdn:" << rdn << endl;
                        WebPhotosetsDirectoryContext* c = 0;
                        c = priv_ensureSubContext( rdn, c );
                        c->constructObject( pset );
                    }
                }
            }
        }
        
    public:

        fh_webPhotos getWebPhotos()
        {
            WebPhotosObjectTopLevelContext* p = 0;
            p = getFirstParentOfContextClass( p );
            return p->getWebPhotos();
        }


        WebPhotosetsDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_photoset( 0 )
            {
                createStateLessAttributes();
            }

        void constructObject( fh_PhotoSet photoset )
        {
            m_photoset = photoset;
        }
        void constructObject( fh_PhotoSet photoset, photolist_t plist )
        {
            m_photoset = photoset;
            m_photolist = plist;
        }
        
        
        virtual ~WebPhotosetsDirectoryContext()
        {}

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

                SLEA( "webphoto-id", &_Self::SL_getNothingStream, XSD_BASIC_INT );
                SLEA( "secret", &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "title",  &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "server", &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "is-primary", &_Self::SL_getNothingStream, XSD_BASIC_BOOL );

#undef SLEA                    
                    
                supplementStateLessAttributes( true );
            }
        }

        virtual std::string getRecommendedEA()
        {
            return _Base::getRecommendedEA() + ",webphoto-id,title,";
        }
        

    };

    /************************************************************/
    /************************************************************/
    /************************************************************/
    /************************************************************/
    /************************************************************/
    /************************************************************/
    /************************************************************/
    /************************************************************/
    /************************************************************/


    class FERRISEXP_DLLLOCAL WebPhotosContactContext;
    FERRIS_CTX_SMARTPTR( WebPhotosContactContext, fh_WebPhotosContactContext );
    class FERRISEXP_DLLLOCAL WebPhotosContactContext
        :
        public StateLessEAHolder< WebPhotosContactContext, FakeInternalContext >
    {
        typedef WebPhotosContactContext _Self;
        typedef StateLessEAHolder< WebPhotosContactContext, FakeInternalContext > _Base;

        
    protected:

        void priv_read()
        {
            staticDirContentsRAII _raii1( this );

            if( empty() )
            {
                fh_webPhotos wf = getWebPhotos();
                string uid = m_contact->getID();
                
                // We have to setup the children the first time..
                fh_context child = 0;

                child = new WebPhotosetsDirectoryContext( this, "photosets" );
                addNewChild( child );

                {
                    WebPhotosetsDirectoryContext* cc = new WebPhotosetsDirectoryContext( this, "favs" );
                    fh_context c = cc;

                    photolist_t col = wf->getFavoritePhotos( uid );
                    cc->constructObject( 0, col );
                
                    addNewChild( c );
                }
                
            }
        }
        
    public:

        
        fh_webPhotos getWebPhotos()
        {
            WebPhotosObjectTopLevelContext* p = 0;
            p = getFirstParentOfContextClass( p );
            return p->getWebPhotos();
        }

        fh_Contact m_contact;

        WebPhotosContactContext( Context* parent, const std::string& rdn, fh_Contact contact )
            :
            _Base( parent, rdn ),
            m_contact( contact )
            {
                createStateLessAttributes();
            }

        virtual ~WebPhotosContactContext()
        {}

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

                // SLEA( "webphoto-id", &_Self::SL_getNothingStream, XSD_BASIC_INT );
                // SLEA( "secret", &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                // SLEA( "title",  &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                // SLEA( "server", &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                // SLEA( "is-primary", &_Self::SL_getNothingStream, XSD_BASIC_BOOL );

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
        

        // virtual std::string getRecommendedEA()
        // {
        //     return _Base::getRecommendedEA() + ",webphoto-id,title,";
        // }
        

    };
    
    /************************************************************/
    /************************************************************/
    /************************************************************/
    
    class FERRISEXP_DLLLOCAL WebPhotosContactsDirectoryContext;
    FERRIS_CTX_SMARTPTR( WebPhotosContactsDirectoryContext, fh_WebPhotosContactsDirectoryContext );
    class FERRISEXP_DLLLOCAL WebPhotosContactsDirectoryContext
        :
        public StateLessEAHolder< WebPhotosContactsDirectoryContext, FakeInternalContext >
    {
        typedef WebPhotosContactsDirectoryContext _Self;
        typedef StateLessEAHolder< WebPhotosContactsDirectoryContext, FakeInternalContext > _Base;

        
    protected:

        void priv_read()
        {
            staticDirContentsRAII _raii1( this );

            if( empty() )
            {
                ContactList_t col = getWebPhotos()->getContacts();
                for( ContactList_t::iterator ci = col.begin(); ci != col.end(); ++ci )
                {
                    fh_Contact c = *ci;
                    string rdn = c->getRDN();
                    fh_context child = new WebPhotosContactContext( this, rdn, c );
                    addNewChild( child );
                }
            }
        }
        
    public:

        fh_webPhotos getWebPhotos()
        {
            WebPhotosObjectTopLevelContext* p = 0;
            p = getFirstParentOfContextClass( p );
            return p->getWebPhotos();
        }


        WebPhotosContactsDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }

        virtual ~WebPhotosContactsDirectoryContext()
        {}

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

                // SLEA( "webphoto-id", &_Self::SL_getNothingStream, XSD_BASIC_INT );
                // SLEA( "secret", &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                // SLEA( "title",  &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                // SLEA( "server", &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                // SLEA( "is-primary", &_Self::SL_getNothingStream, XSD_BASIC_BOOL );

#undef SLEA                    
                    
                supplementStateLessAttributes( true );
            }
        }

        // virtual std::string getRecommendedEA()
        // {
        //     return _Base::getRecommendedEA() + ",webphoto-id,title,";
        // }
        

    };

    /************************************************************/
    /************************************************************/
    /************************************************************/

    void
    WebPhotosObjectTopLevelContext::priv_read()
    {
        DEBUG << "WebPhotosObjectTopLevelContext::priv_read(top)" << endl;
        cerr << "WebPhotosObjectTopLevelContext::priv_read(top)" << endl;
        staticDirContentsRAII _raii1( this );

        if( empty() )
        {
            DEBUG << "WebPhotosObjectTopLevelContext::priv_read(r)" << endl;
            fh_webPhotos wf = getWebPhotos();
            string uid = "";
            
            // We have to setup the children the first time..
            fh_context child = 0;
            
            child = new WebPhotosUploadDirectoryContext( this, "upload" );
            addNewChild( child );

            if( getDirName() == "pixelpipe" )
                return;
            
 
            child = new WebPhotosRecentDirectoryContext( this, "recent" );
            addNewChild( child );

            child = new WebPhotosetsDirectoryContext( this, "photosets" );
            addNewChild( child );

            {
                WebPhotosetsDirectoryContext* cc = new WebPhotosetsDirectoryContext( this, "not-in-any-photosets" );
                fh_context c = cc;

                photolist_t col = m_wf->getNotInSet();
                cc->constructObject( 0, col );
                
                addNewChild( c );
            }


            {
                WebPhotosetsDirectoryContext* cc = new WebPhotosetsDirectoryContext( this, "favs" );
                fh_context c = cc;

                photolist_t col = m_wf->getFavoritePhotos( uid );
                cc->constructObject( 0, col );
                
                addNewChild( c );
            }
            
            {
                fh_context child = new WebPhotosByPhotoIDDirectoryContext( this, "by-id" );
                addNewChild( child );
            }

            {
                fh_context child = new WebPhotosContactsDirectoryContext( this, "contacts" );
                addNewChild( child );
            }
        }
    }


    /************************************************************/
    /************************************************************/
    /************************************************************/
    

    void
    WebPhotosRootContext::priv_read()
    {
        staticDirContentsRAII _raii1( this );

        if( empty() )
        {
            // We have to setup the children the first time..
            fh_context child = 0;
            
            {
                fh_fcontext hc = new FakeInternalContext( this, "flickr" );
                Insert( GetImpl(hc), false );

                fh_webPhotos wf = Factory::getDefaultFlickrWebPhotos();
                child = new WebPhotosObjectTopLevelContext( GetImpl(hc),
                                                            wf->getUserName(),
                                                            wf );
                hc->addNewChild( child );
                hc->addNewChild( new VirtualSoftlinkContext( hc, child, "me", true ) );
            }

            {
                fh_fcontext hc = new FakeInternalContext( this, "23hq" );
                Insert( GetImpl(hc), false );

                fh_webPhotos wf = Factory::getDefault23hqWebPhotos();
                child = new WebPhotosObjectTopLevelContext( GetImpl(hc),
                                                            wf->getUserName(),
                                                            wf );
                hc->addNewChild( child );
                hc->addNewChild( new VirtualSoftlinkContext( hc, child, "me", true ) );
            }


            {
                fh_fcontext hc = new FakeInternalContext( this, "pixelpipe" );
                Insert( GetImpl(hc), false );

                fh_webPhotos wf = Factory::getDefaultPixelPipeWebPhotos();
                child = new WebPhotosObjectTopLevelContext( GetImpl(hc),
                                                            wf->getUserName(),
                                                            wf );
                hc->addNewChild( child );
                hc->addNewChild( new VirtualSoftlinkContext( hc, child, "me", true ) );
            }
            
        }
    }

    /************************************************************/
    /************************************************************/
    /************************************************************/

    fh_webPhotos
    WebPhotosUploadFileContext::getWebPhotos()
    {
        WebPhotosUploadDirectoryContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p->getWebPhotos();
    }
    
    /************************************************************/
    /************************************************************/
    /************************************************************/

    std::string
    WebPhotosetsDirectoryContext::getUserID()
    {
        WebPhotosContactContext* p = 0;
        if( p = getFirstParentOfContextClass( p ) )
            return p->m_contact->getID();
        return "";
    }

    /************************************************************/
    /************************************************************/
    /************************************************************/

    
    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                DEBUG << "Brew(TOP)" << endl;

                static WebPhotosRootContext* c = 0;
                if( !c )
                {
                    c = new WebPhotosRootContext();
                    // Bump ref count.
                    static fh_context keeper = c;
                    static fh_context keeper2 = keeper;
                }
                fh_context ret = c;
                DEBUG << "Brew(END)" << endl;
                return ret;
            }
            catch( exception& e )
            {
                DEBUG << "e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }
    
    
};
