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

    $Id: FerrisWebServices_private.hh,v 1.4 2010/11/15 21:30:33 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_WEB_SERVICES_PRIV_H_
#define _ALREADY_INCLUDED_FERRIS_WEB_SERVICES_PRIV_H_

#include <HiddenSymbolSupport.hh>
#include <TypeDecl.hh>

#include "Ferris/Context.hh"
#include "Ferris/FerrisQt_private.hh"

namespace Ferris
{
    using std::endl;
    using std::string;
    
    /****************************************/
    /****************************************/
    /****************************************/

    class WebServicesUpload;
    FERRIS_SMARTPTR( WebServicesUpload, fh_WebServicesUpload );
    
    class FERRISEXP_API WebServicesUpload
        :
        public QObject,
        public Handlable
    {
        Q_OBJECT;

    protected:
        int m_uploadSize;
        std::string m_uploadFilename;
        std::string m_title;
        std::string m_desc;
        std::string m_keywords;
        bool m_uploadDefaultsToPrivate;
        fh_StreamToQIODevice m_streamToQIO;
        QNetworkReply* m_reply;

        std::string m_url;
        std::string m_id;

    protected:
        WebServicesUpload();
        
    public:

        virtual ~WebServicesUpload();

        void setFilename( const std::string& s );
        void setLength( int sz );
        void setTitle( const std::string& s );
        void setDescription( const std::string& s );
        void setKeywords( const std::string& s );
        
        std::string getURL();
        std::string getID();
        
        virtual void streamingUploadComplete() = 0;
        virtual fh_iostream createStreamingUpload( const std::string& ContentType ) = 0;
    };

    FERRISEXP_API std::string filenameToContextType( const std::string& s );
    
    /****************************************/
    /****************************************/
    /****************************************/


    template < class ChildContextClass, class ParentContextClass = leafContext >
    class FERRISEXP_API WebServicesFileUploadContext
        :
        public ParentContextClass
    {
        typedef ParentContextClass                _Base;
        typedef WebServicesFileUploadContext< ChildContextClass, ParentContextClass > _Self;

    protected:
        fh_WebServicesUpload m_wsUpload;
        std::string m_filename;
        std::string m_title;
        std::string m_desc;
        std::string m_keywords;
        std::string m_ContentType;
        int         m_ContentLength;

    public:

        virtual fh_WebServicesUpload getWebServicesUpload() = 0;

        WebServicesFileUploadContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_ContentLength( 0 )
            {
                LG_WEBSERVICE_D << "ctor, have read:" << this->getHaveReadDir() << endl;
            }

        ferris_ios::openmode
            getSupportedOpenModes()
            {
                return
                    std::ios_base::in        |
                    std::ios_base::out       |
                    std::ios::trunc     |
                    std::ios::ate       |
                    std::ios::app       |
                    std::ios_base::binary    ;
            }

        void OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
                LG_WEBSERVICE_D << "OnStreamClosed()" << endl;
                if( !(m & std::ios::out) )
                    return;
                LG_WEBSERVICE_D << "OnStreamClosed() waiting..." << endl;
                getWebServicesUpload()->streamingUploadComplete();

                if( m_wsUpload )
                {
                    LG_WEBSERVICE_D << "OnStreamClosed video url:" << m_wsUpload->getURL() << endl;
                    LG_WEBSERVICE_D << "OnStreamClosed video  id:" << m_wsUpload->getID() << endl;
                }
            }


        
        virtual void priv_preCopyAction( fh_context c )
            {
                LG_WEBSERVICE_D << "preCopyAction(top) c:" << c->getURL() << endl;
        
//                fh_YoutubeUpload u = getYoutubeUpload();

                m_filename = c->getDirName();
                string t = getStrAttr( c, "upload-filename", "" );
                if( !t.empty() )
                {
                    m_filename = t;
                }
                m_title = getStrAttr( c, "title", "" );
                LG_WEBSERVICE_D << "preCopyAction() title1:" << m_title << endl;
                if( m_title.empty() )
                {
                    m_title = getStrAttr( c, "description", "" );
                    LG_WEBSERVICE_D << "preCopyAction() title2:" << m_title << endl;
                }
                if( m_title.empty() )
                {
                    m_title = getStrAttr( c, "keywords", "" );
                    LG_WEBSERVICE_D << "preCopyAction() title3:" << m_title << endl;
                }
                if( m_title.empty() )
                {
                    m_title = getStrAttr( c, "annotation", "" );
                    LG_WEBSERVICE_D << "preCopyAction() title4:" << m_title << endl;
                }
                
                m_ContentType = getStrAttr( c, "mimetype" ,"" );
                m_ContentLength = toint( getStrAttr( c, "size", "200" ));
        
                LG_WEBSERVICE_D << "m_ContentLength:" << m_ContentLength << endl;
            }

        virtual void priv_postCopyAction( fh_context c )
            {
                LG_WEBSERVICE_D << "postCopyAction() c:" << c->getURL() << endl;
                // if( m_YoutubeUpload )
                // {
                //     getWebServicesUpload()->streamingUploadComplete();
                // }
            }
        
        
        fh_iostream
            priv_getIOStream( ferris_ios::openmode m )
            {
                fh_WebServicesUpload u = getWebServicesUpload();
                
                string filename = m_filename;
                string title = m_title;
                string desc = m_desc;
                string keywords = m_keywords;

                LG_WEBSERVICE_D << "priv_getIOStream() fn:" << filename
                                << " title:" << title << " desc:" << desc << endl;
                if( filename.empty() )
                    filename = this->getDirName();
                if( title.empty() )
                    title = filename;
                if( desc.empty() )
                    desc = title;

                int ContentLength = 200;
                if( m_ContentLength )
                    ContentLength = m_ContentLength;

                u->setFilename( filename );
                u->setTitle( title );
                u->setDescription( desc );
                u->setKeywords( keywords );
                u->setLength( ContentLength );

                string ContentType = filenameToContextType( filename );
                if( !m_ContentType.empty() )
                    ContentType = m_ContentType;

                LG_WEBSERVICE_D << "  ContentType:" << ContentType << endl;
                LG_WEBSERVICE_D << "ContentLength:" << ContentLength << endl;
                
                fh_iostream ret = u->createStreamingUpload( ContentType );
                ret->getCloseSig().connect( sigc::bind( sigc::mem_fun(*this, &_Self::OnStreamClosed ), m )); 
                return ret;
            }
        
        fh_istream
        priv_getIStream( ferris_ios::openmode m )
            {
                fh_stringstream ret;
                return ret;
            }

        virtual void priv_FillCreateSubContextSchemaParts( Context::CreateSubContextSchemaPart_t& m )
        {
            LG_WEBSERVICE_D << "upload file. setting file creation schema" << endl;

            m["ea"] = Context::SubContextCreator(
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
        
    };
    
    template < class ChildContextClass, class UploadFileContextClass, class ParentContextClass = FakeInternalContext >
    class FERRISEXP_API WebServicesUploadDirectoryContext
        :
        public ParentContextClass
    {
        typedef ParentContextClass                _Base;
        typedef WebServicesUploadDirectoryContext< ChildContextClass, ParentContextClass > _Self;

    public:

        WebServicesUploadDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }

        bool isDir()
            {
                return true;
            }

        void createStateLessAttributes( bool force = false )
            {
                if( force || this->isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    this->supplementStateLessAttributes( true );
                }
            }

        

        virtual fh_context SubCreate_file( fh_context c, fh_context md )
            {
                string rdn         = getStrSubCtx( md, "name", "" );
                string v           = "";
                LG_WEBSERVICE_D << "SubCreate_file() rdn:" << rdn << " v:" << v << endl;

                if( WebServicesUploadDirectoryContext* cc = dynamic_cast<WebServicesUploadDirectoryContext*>(GetImpl(c)))
                {
                    UploadFileContextClass* c = 0;
                    c = this->priv_ensureSubContext( rdn, c );
                    return c;
                }
                std::stringstream ss;
                ss << "Attempt to create a subobject on a context that is not an webservices one!"
                   << " url:" << c->getURL()
                   << endl;
                Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
            }
        
        void priv_FillCreateSubContextSchemaParts( Context::CreateSubContextSchemaPart_t& m )
        {
            LG_WEBSERVICE_D << "priv_FillCreateSubContextSchemaParts()" << endl;
            m["file"] = Context::SubContextCreator( ::Ferris::SL_SubCreate_file,
                                         "	<elementType name=\"file\">\n"
                                         "		<elementType name=\"name\" default=\"new file\">\n"
                                         "			<dataTypeRef name=\"string\"/>\n"
                                         "		</elementType>\n"
                                         "	</elementType>\n");
        }

        void priv_read()
            {
                LG_WEBSERVICE_D << "priv_read() url:" << this->getURL()
                                << " have read:" << this->getHaveReadDir()
                                << endl;
                Context::EnsureStartStopReadingIsFiredRAII _raii1( this );
                Context::emitExistsEventForEachItemRAII    _raii2( this );
            }
    };
    
};
#endif
