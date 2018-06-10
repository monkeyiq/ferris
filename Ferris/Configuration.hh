/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of libferris.

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

    $Id: Configuration.hh,v 1.13 2010/09/24 21:30:26 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_CONFIGURATION_H_
#define _ALREADY_INCLUDED_FERRIS_CONFIGURATION_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>

namespace Ferris
{
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    FERRISEXP_API std::string getConfigString( const std::string& dbpath,
                                               const std::string& k,
                                               const std::string& def,
                                               bool throw_for_errors = false );

    FERRISEXP_API void setConfigString( const std::string& dbpath,
                                        const std::string& k,
                                        const std::string& v,
                                        bool throw_for_errors = false );
    
    


    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    FERRISEXP_API extern std::string GET_FDB_GENERAL();
    FERRISEXP_API extern const std::string FDB_GENERAL;
    FERRISEXP_API extern const std::string FDB_FERRISDEV;

    FERRISEXP_API extern const char* COPY_INPUT_IN_MMAP_MODE_BY_DEFAULT;
    FERRISEXP_API extern const char* COPY_INPUT_IN_MMAP_MODE_FOR_OBJECTS_LARGER_THAN;
    FERRISEXP_API extern const char* COPY_OUTPUT_IN_MMAP_MODE_BY_DEFAULT;
    FERRISEXP_API extern const char* COPY_OUTPUT_IN_MMAP_MODE_FOR_OBJECTS_LARGER_THAN;
    FERRISEXP_API extern const char* USE_SENDFILE_IF_POSSIBLE;
    FERRISEXP_API extern const char* SENDFILE_CHUNK_SIZE;

    FERRISEXP_API extern const char* CFG_ATTRIBUTES_TO_AUTO_REA_K;
    FERRISEXP_API extern const char* CFG_ATTRIBUTES_TO_AUTO_REA_DEFAULT;

    FERRISEXP_API extern const char* CFG_ATTRIBUTES_ALWAYS_IN_UI_MODEL_K;
    FERRISEXP_API extern const char* CFG_ATTRIBUTES_ALWAYS_IN_UI_MODEL_DEFAULT;
    
    FERRISEXP_API extern const char* COPY_USES_DIRECTIO_BY_DEFAULT;
    FERRISEXP_API extern const char* COPY_USES_DIRECTIO_FOR_OBJECTS_LARGER_THAN;

    FERRISEXP_API extern const char* SHOW_LOGGING_POPT_OPTIONS_BY_DEFAULT;
    FERRISEXP_API extern const char* LOGGING_TO_FILENAME_KEY;

    FERRISEXP_API extern const char* CFG_ATTRIBUTES_GNU_DIFF_CMD_FILES_K;
    FERRISEXP_API extern const char* CFG_ATTRIBUTES_GNU_DIFF_CMD_FILES_DEFAULT;

    FERRISEXP_API extern const char* CFG_FORCE_PASSIVE_VIEW_K;
    FERRISEXP_API extern const char* CFG_FORCE_PASSIVE_VIEW_DEFAULT;

    FERRISEXP_API extern const char* CFG_XSLTFS_STYLESHEET_PATH_K;
    FERRISEXP_API extern std::string CFG_XSLTFS_STYLESHEET_PATH_DEFAULT;


    FERRISEXP_API extern const char* CFG_GLOB_SKIP_REGEX_LIST_K;
    FERRISEXP_API extern const char* CFG_GLOB_SKIP_REGEX_LIST_DEFAULT;

    FERRISEXP_API extern const char* CFG_GLOB_SKIP_FILE_URLS_K;
    FERRISEXP_API extern const char* CFG_GLOB_SKIP_FILE_URLS_DEFAULT;

    FERRISEXP_API extern const char* CFG_PRECALCULATE_FOR_COPY_K;
    FERRISEXP_API extern const char* CFG_PRECALCULATE_FOR_COPY_DEFAULT;
    
    FERRISEXP_API extern const char* CFG_RDFCACHE_ATTRS_LIST_K;
    FERRISEXP_API extern const char* CFG_RDFCACHE_ATTRS_LIST_DEFAULT;

    FERRISEXP_API extern const char* CFG_RDFCACHE_ATTRS_ENABLED_K;
    FERRISEXP_API extern const char* CFG_RDFCACHE_ATTRS_ENABLED_DEFAULT;

    FERRISEXP_API extern const char* CFG_RDF_GLOBAL_SMUSH_GROUP_LEADER_K;
    FERRISEXP_API extern const char* CFG_RDF_GLOBAL_SMUSH_GROUP_LEADER_DEFAULT;

    FERRISEXP_API extern const char* CFG_LOWEST_EMBLEM_PRI_TO_SHOW_K;
    FERRISEXP_API extern const char* CFG_LOWEST_EMBLEM_PRI_TO_SHOW_DEFAULT;

    FERRISEXP_API extern const char* CFG_THUMBNAILS_MAX_DESIRED_WIDTH_OR_HEIGHT_K;
    FERRISEXP_API extern const char* CFG_THUMBNAILS_MAX_DESIRED_WIDTH_OR_HEIGHT_DEFAULT;
    FERRISEXP_API extern const char* CFG_THUMBNAILS_JPEG_IMG_QUALITY_K;
    FERRISEXP_API extern const char* CFG_THUMBNAILS_JPEG_IMG_QUALITY_DEFAULT;


    FERRISEXP_API extern const std::string CFG_CDROM_SIZE_BYTES_K;
    FERRISEXP_API extern const std::string CFG_CDROM_SIZE_BYTES_DEFAULT;
    FERRISEXP_API extern const std::string CFG_CDROM_SIZE_BYTES_EA_POSTFIX;
    FERRISEXP_API extern const std::string CFG_DVD_SIZE_BYTES_K;
    FERRISEXP_API extern const std::string CFG_DVD_SIZE_BYTES_DEFAULT;
    FERRISEXP_API extern const std::string CFG_DVD_SIZE_BYTES_EA_POSTFIX;

    FERRISEXP_API extern const char* FERRIS_LUCENE_METADATA_DBNAME;


    FERRISEXP_API extern const char* CFG_ALLOWS_KERNEL_EA_REGEX;
    FERRISEXP_API extern const char* CFG_ALLOWS_KERNEL_EA_REGEX_DEFAULT;


    FERRISEXP_API extern const char* CFG_WEBPHOTOS_TOKEN_K;
    FERRISEXP_API extern const char* CFG_WEBPHOTOS_TOKEN_DEFAULT;

    FERRISEXP_API extern const char* CFG_WEBPHOTOS_DEFAULT_USERNAME_KEY;
    FERRISEXP_API extern const char* CFG_WEBPHOTOS_DEFAULT_USERNAME_DEFAULT;

    FERRISEXP_API extern const char* CFG_WEBPHOTOS_USERNAME_K;
    FERRISEXP_API extern const char* CFG_WEBPHOTOS_USERNAME_DEF;
    FERRISEXP_API extern const char* CFG_WEBPHOTOS_FULLNAME_K;
    FERRISEXP_API extern const char* CFG_WEBPHOTOS_FULLNAME_DEF;

    FERRISEXP_API extern const char* CFG_WEBPHOTOS_LARGEST_DIM_K;
    FERRISEXP_API extern const char* CFG_WEBPHOTOS_LARGEST_DIM_DEF;

    FERRISEXP_API extern const char* CFG_WEBPHOTOS_INCLUDE_EA_IS_PRESENT_REGEX_K;
    FERRISEXP_API extern const char* CFG_WEBPHOTOS_INCLUDE_EA_AND_VALUE_REGEX_K;

    FERRISEXP_API extern const char* CFG_WEBPHOTOS_DEFAULT_PROTECTION_PUBLIC_K;
    FERRISEXP_API extern const char* CFG_WEBPHOTOS_DEFAULT_PROTECTION_FRIEND_K;
    FERRISEXP_API extern const char* CFG_WEBPHOTOS_DEFAULT_PROTECTION_FAMILY_K;
    
    


    FERRISEXP_API extern const char* CFG_FSPOT_POSITIVE_OVERLAY_REGEX_K;
    FERRISEXP_API extern const char* CFG_FSPOT_POSITIVE_OVERLAY_REGEX_DEF;

    FERRISEXP_API extern const char* CFG_STRIGI_POSITIVE_OVERLAY_REGEX_K;
    FERRISEXP_API extern const char* CFG_STRIGI_POSITIVE_OVERLAY_REGEX_DEF;
    
    
};
#endif
