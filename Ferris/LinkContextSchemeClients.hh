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

    $Id: LinkContextSchemeClients.hh,v 1.12 2011/09/12 21:31:33 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_LINK_CONTEXT_SCHEME_CLIENTS_H_
#define _ALREADY_INCLUDED_LINK_CONTEXT_SCHEME_CLIENTS_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/LinkContextScheme.hh>

#include <string>

namespace Ferris
{
    namespace Private
    {
        extern const std::string link_ctx_applications_name;
        extern const std::string link_ctx_applications_url;
        extern const std::string link_ctx_apps_name;
        extern const std::string link_ctx_apps_url;

        extern const std::string link_ctx_events_name;
        extern const std::string link_ctx_events_url;

        extern const std::string link_ctx_icons_name;
        extern const std::string link_ctx_icons_url;
        extern const std::string link_ctx_gnomeicons_name;
        extern const std::string link_ctx_gnomeicons_url;

        extern const std::string link_ctx_mime_name;
        extern const std::string link_ctx_mime_url;

        extern const std::string link_ctx_fileclip_name;
        extern const std::string link_ctx_fileclip_url;

        extern const std::string link_ctx_news_name;
        extern const std::string link_ctx_news_url;

        extern const std::string link_ctx_bookmarks_name;
        extern const std::string link_ctx_bookmarks_url;

        extern const std::string link_ctx_eaq_name;
        extern const std::string link_ctx_eaq_url;

        extern const std::string link_ctx_ftxq_name;
        extern const std::string link_ctx_ftxq_url;
        
        extern const std::string link_ctx_rdf_name;
        extern const std::string link_ctx_rdf_url;
        
//         extern const std::string link_ctx_schema_name;
//         extern const std::string link_ctx_schema_url;

        extern const std::string link_ctx_camera_name;
        extern const std::string link_ctx_camera_url;

        extern const std::string link_ctx_pg_name;
        extern const std::string link_ctx_pg_url;

        extern const std::string link_ctx_flickr_name;
        extern const std::string link_ctx_flickr_url;

        extern const std::string link_ctx_23hq_name;
        extern const std::string link_ctx_23hq_url;

        extern const std::string link_ctx_pixelpipe_name;
        extern const std::string link_ctx_pixelpipe_url;

        extern const std::string link_ctx_eaindexes_name;
        extern const std::string link_ctx_eaindexes_url;

        extern const std::string link_ctx_ftxindexes_name;
        extern const std::string link_ctx_ftxindexes_url;

        extern const std::string link_ctx_qtsqlmysql_name;
        extern const std::string link_ctx_qtsqlmysql_url;

        extern const std::string link_ctx_qtsqlpostgresql_name;
        extern const std::string link_ctx_qtsqlpostgresql_url;

        extern const std::string link_ctx_youtube_name;
        extern const std::string link_ctx_youtube_url;

        extern const std::string link_ctx_sane_name;
        extern const std::string link_ctx_sane_url;

        extern const std::string link_ctx_pulseaudio_name;
        extern const std::string link_ctx_pulseaudio_url;

    };
};


#endif
