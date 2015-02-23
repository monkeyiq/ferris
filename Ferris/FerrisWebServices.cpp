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

    $Id: FerrisWebServices.cpp,v 1.3 2011/01/20 21:30:15 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "FerrisWebServices_private.hh"


namespace Ferris
{

    /****************************************/
    /****************************************/
    /****************************************/

    WebServicesUpload::WebServicesUpload()
        :
        m_uploadSize( 0 ),
        m_uploadFilename( "filename.avi" ),
        m_streamToQIO( 0 ),
        m_reply( 0 ),
        m_uploadDefaultsToPrivate( false )
    {
    }
    

    WebServicesUpload::~WebServicesUpload()
    {
    }


    void
    WebServicesUpload::setFilename( const std::string& s )
    {
        m_uploadFilename = s;
    }
    
    void
    WebServicesUpload::setLength( int sz )
    {
        m_uploadSize = sz;
    }

    void
    WebServicesUpload::setTitle( const std::string& s )
    {
        m_title = s;
    }
    
    void
    WebServicesUpload::setDescription( const std::string& s )
    {
        m_desc = s;
    }
    
    void
    WebServicesUpload::setKeywords( const std::string& s )
    {
        m_keywords = s;
    }
    
    std::string
    WebServicesUpload::getURL()
    {
        return m_url;
    }
    
    std::string
    WebServicesUpload::getID()
    {
        return m_id;
    }
    

    /****************************************/
    /****************************************/
    /****************************************/

    std::string filenameToContextType( const std::string& s )
    {
        string ret = "video/mp4";
        
        int p = s.rfind(".");
        if( p != string::npos )
        {
            string ext = s.substr( p+1 );
            ret = (string)"video/" + ext;
        }

        if( ends_with( s, ".cr2" ) || ends_with( s, ".CR2" ) )
            ret = "image/cr2";
        if( ends_with( s, ".flv" ))
            ret = "video/x-flv";
        if( ends_with( s, ".jpg" ))
            ret = "image/jpeg";
        LG_WEBSERVICE_D << "filenameToContextType() ret:" << ret <<  " s:" << s << endl;
        std::cerr << "filenameToContextType() ret:" << ret <<  " s:" << s << endl;
        return ret;
    }
    
    
    /****************************************/
    /****************************************/
    /****************************************/


    
    


    /****************************************/
    /****************************************/
    /****************************************/
    
};


#include "FerrisWebServices_private_moc.cpp"
