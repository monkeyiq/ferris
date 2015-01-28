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

    $Id: FerrisCurl_private.hh,v 1.4 2010/09/24 21:30:35 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#ifndef _ALREADY_INCLUDED_FERRIS_FERRISCURL_H_
#define _ALREADY_INCLUDED_FERRIS_FERRISCURL_H_

#include <stddef.h>
#include <string>
#include "Ferris/TypeDecl.hh"

namespace Ferris
{
    size_t ferriscurlWriteCallback(void *ptr, size_t size, size_t nmemb, void *userdata);

    class FerrisCurl
    {
        friend size_t ferriscurlWriteCallback(void *ptr, size_t size, size_t nmemb, void *userdata);

        struct FileToUpload
            :
            public Handlable
        {
            fh_context c;
            std::string filename;
            std::string mimetype;
            std::string formElementName;
            std::string m_data;
            
            FileToUpload( fh_context c, std::string filename, std::string mimetype, std::string formElementName );
            ~FileToUpload();
            const std::string& getData();

            NOT_COPYABLE( FileToUpload );
        };
        FERRIS_SMARTPTR( FileToUpload, fh_FileToUpload );
        
        typedef std::list< fh_FileToUpload > m_filesToUpload_t;
        m_filesToUpload_t m_filesToUpload;
        stringmap_t m_multiPartPostData;
        std::string m_httpAccept;
        std::string m_contentType;
        std::stringstream* m_resultDataSS;
        long m_httpStatus;
        bool m_isPost;
        bool m_debugVerbose;
        
        std::string perform( const std::string& earl,
                             const std::string& postData = "" );
        
    public:
        FerrisCurl();
        ~FerrisCurl();

        /**
         * Turns on CURL debugging
         */
        void setDebugVerbose( bool v );
        
        void setHttpAccept( const std::string& s );
        void setContentType( const std::string& s );

        std::string post( const std::string& earl,
                          const std::string& postData );

        void setMultiPartPostData( stringmap_t sl );
        void appendFileToUpload( fh_context c );
        void appendFileToUpload( fh_context c, std::string filename, std::string mimetype, std::string formElementName );
    };
};
#endif
