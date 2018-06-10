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

    $Id: FerrisCurl.cpp,v 1.5 2010/11/14 21:30:13 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "Ferris/FerrisCurl_private.hh"
#include <Configuration_private.hh>

#include <sstream>

#include <curl/curl.h>
#include <curl/easy.h>

using namespace std;


namespace Ferris
{
    FerrisCurl::FerrisCurl()
        :
        m_resultDataSS( 0 ),
        m_isPost( false ),
        m_debugVerbose( false )
    {
    }
    
    FerrisCurl::~FerrisCurl()
    {
    }

    void
    FerrisCurl::setDebugVerbose( bool v )
    {
        m_debugVerbose = v;
    }
    
    void
    FerrisCurl::setHttpAccept( const std::string& s )
    {
        m_httpAccept = s;
    }

    void
    FerrisCurl::setContentType( const std::string& s )
    {
        m_contentType = s;
    }
    
    void
    FerrisCurl::setMultiPartPostData( stringmap_t sl )
    {
        m_multiPartPostData = sl;
    }

    void
    FerrisCurl::appendFileToUpload( fh_context c, string filename, string mimetype, string formElementName )
    {
        m_filesToUpload.push_back( new FileToUpload( c, filename, mimetype, formElementName ) );
    }
    
    
    void
    FerrisCurl::appendFileToUpload( fh_context c )
    {
        appendFileToUpload( c, "", "", "file" );
    }
    
    
    
    
    std::string
    FerrisCurl::post( const std::string& earl, const std::string& postData )
    {
        m_isPost = 1;
        return perform( earl, postData );
    }

    size_t ferriscurlWriteCallback(void *ptr, size_t size, size_t nmemb, void *userdata) 
    {
        FerrisCurl* thisp = (FerrisCurl*)userdata;
        int len=size*nmemb;
        LG_CURL_D << "ferriscurlWriteCallback() len:" << len << endl;

        thisp->m_resultDataSS->write( (const char*)ptr, len );
        return len;
    }

    FerrisCurl::FileToUpload::FileToUpload(
        fh_context c, std::string filename, std::string mimetype, std::string formElementName )
        :
        c(c),
        filename( filename ),
        mimetype( mimetype ),
        formElementName( formElementName )
    {
        if( mimetype.empty() )
        {
            this->mimetype = getStrAttr( c, "mimetype", "" );
            LG_CURL_D << "no mimetype set for c:" << c->getURL()
                      << " read type as:" << this->mimetype
                      << endl;
        }
        if( filename.empty() )
        {
            this->filename = c->getDirName();
        }

        LG_CURL_D << "FileToUpload() c:" << c->getURL()
                  << " mimetype:" << this->mimetype
                  << " filename:" << this->filename
                  << " formElementName:" << this->formElementName
                  << endl;
    }

    FerrisCurl::FileToUpload::~FileToUpload()
    {
        LG_CURL_D << "~FileToUpload()" << endl;
    }
    
    const string&
    FerrisCurl::FileToUpload::getData()
    {
        LG_WEBPHOTO_D << "FileToUpload::getData() c:" << c->getURL() << endl;
        LG_WEBPHOTO_D << "FileToUpload::getData() sz:" << getStrAttr( c, "content", "", true, true ).length()   << endl;
        m_data = getStrAttr( c, "content", "", true, true );
        return m_data;
    }
    
    
    std::string
    FerrisCurl::perform( const std::string& earl, const std::string& postData )
    {
        if( m_resultDataSS )
            delete m_resultDataSS;
        m_resultDataSS = new std::stringstream();
        m_httpStatus = 0;

        struct curl_slist *slist=NULL;

        CURL* m_curl = 0;
        m_curl=curl_easy_init();
        
        curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION,
                         ferriscurlWriteCallback);
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this );

        char errbuf[CURL_ERROR_SIZE];
        errbuf[0] = 0;
        curl_easy_setopt( m_curl, CURLOPT_ERRORBUFFER, errbuf );

        curl_easy_setopt( m_curl, CURLOPT_URL, earl.c_str() );

//        string proxyname = getConfigString( FDB_GENERAL, "curl-use-proxy-name", "" );
//        curl_easy_setopt( m_curl, CURLOPT_PROXY, proxyname.c_str() );
        
        curl_easy_setopt( m_curl, CURLOPT_CUSTOMREQUEST, 0 );

        if( !m_httpAccept.empty() )
            slist=curl_slist_append(slist, (const char*)m_httpAccept.c_str() );

        if( m_isPost )
        {
            if( !m_multiPartPostData.empty() )
            {
                struct curl_httppost* post = NULL;
                struct curl_httppost* last = NULL;

                LG_CURL_D << "have many part post form data... m_multiPartPostData.size:"
                          << m_multiPartPostData.size() << endl;
                for( stringmap_t::iterator si = m_multiPartPostData.begin();
                     si != m_multiPartPostData.end(); ++si )
                {
                    LG_CURL_D << "arg name:" << si->first << endl;
                    LG_CURL_D << "arg second.len:" << si->second.length() << endl;

//                     if( si->first == "photo" )
//                     {
// //                         curl_formadd(&post, &last,
// // //                                      CURLFORM_PTRNAME,        si->first.c_str(),
// // //                                      CURLFORM_PTRCONTENTS,    si->second.c_str(),
// // //                                      CURLFORM_NAMELENGTH,     si->first.length(),
// // //                                      CURLFORM_CONTENTSLENGTH, si->second.length(),
// //                                      CURLFORM_COPYNAME, "photo",
// //                                      CURLFORM_CONTENTTYPE,    "image/png",
// // //                                     CURLFORM_FILENAME,       "foobar.jpg",
// //                                      CURLFORM_FILE,       "/tmp/testing-image.png",
// //                                      CURLFORM_END);

//                         curl_formadd(&post, &last,
//                                      CURLFORM_PTRNAME,        si->first.c_str(),
//                                      CURLFORM_NAMELENGTH,     si->first.length(),
//                                      CURLFORM_CONTENTTYPE,    "image/png",
//                                      CURLFORM_BUFFER,         "foobar.jpg",
//                                      CURLFORM_BUFFERPTR,      si->second.c_str(),
//                                      CURLFORM_BUFFERLENGTH,   si->second.length(),
//                                      CURLFORM_END);
//                     }
//                     else
                    {
                        curl_formadd(&post, &last,
                                     CURLFORM_PTRNAME,        si->first.c_str(),
                                     CURLFORM_PTRCONTENTS,    si->second.c_str(),
                                     CURLFORM_NAMELENGTH,     si->first.length(),
                                     CURLFORM_CONTENTSLENGTH, si->second.length(),
                                     CURLFORM_END);
                    }
                }

                LG_CURL_D << "m_filesToUpload.sz:" << m_filesToUpload.size() << endl;
                
                for( m_filesToUpload_t::iterator iter = m_filesToUpload.begin();
                     iter != m_filesToUpload.end(); ++iter )
                {
                    fh_FileToUpload ftu = *iter;
                    fh_context c = ftu->c;

                    LG_CURL_D << "m_filesToUpload.form_name:" << ftu->formElementName << endl;
                    LG_CURL_D << "m_filesToUpload.mimetype:" << ftu->mimetype << endl;
                    LG_CURL_D << "m_filesToUpload.filename:" << ftu->filename << endl;
                    LG_CURL_D << "m_filesToUpload.data.sz:" << ftu->getData().length() << endl;
                    LG_CURL_D << "m_filesToUpload.url:" << c->getURL() << endl;
                    
                    curl_formadd(&post, &last,
                                 CURLFORM_PTRNAME,        ftu->formElementName.c_str(),
                                 CURLFORM_NAMELENGTH,     ftu->formElementName.length(),
                                 CURLFORM_CONTENTTYPE,    ftu->mimetype.c_str(),
                                 CURLFORM_BUFFER,         ftu->filename.c_str(),
                                 CURLFORM_BUFFERPTR,      ftu->getData().c_str(),
                                 CURLFORM_BUFFERLENGTH,   ftu->getData().length(),
                                 CURLFORM_END);


                    
//                     // This works OK.
//                     curl_formadd(&post, &last,
//                                  CURLFORM_COPYNAME,       "photo",
//                                  CURLFORM_FILE,           "/tmp/libferris-webupload-fs-temp-image..jpg",
//                                  CURLFORM_END);
                    
                }
                
                curl_easy_setopt(m_curl, CURLOPT_HTTPPOST, post );
            }
            else
            {
                curl_easy_setopt(m_curl, CURLOPT_POST, 1);
                curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS,    postData.c_str() );
                curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, postData.length() );
            }
        }
        else
        {
            curl_easy_setopt(m_curl, CURLOPT_HTTPPOST, 0);
            curl_easy_setopt(m_curl, CURLOPT_POST, 0);
        }
        
        if( !m_contentType.empty() )
        {
            LG_CURL_D << "setting content type...:" << m_contentType << endl;
            slist=curl_slist_append(slist, (const char*)m_contentType.c_str() );
        }

        LG_CURL_D << "m_debugVerbose:" << m_debugVerbose << endl;
        curl_easy_setopt(m_curl, CURLOPT_VERBOSE, m_debugVerbose );
        
        if(curl_easy_perform(m_curl))
        {
            // error
            LG_CURL_I << "Error:" << errbuf << endl;
        }
        else
        {
            curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &m_httpStatus);
        }
        
        return m_resultDataSS->str();
        
    }
    

    
};

