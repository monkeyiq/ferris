/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2002 Ben Martin

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

    $Id: FactoriesCreationCommon_private.hh,v 1.6 2010/09/24 21:30:33 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <string>

namespace Ferris
{
    
//     #define  EAINDEX_EANAMES_REGEX_IGNORE_DEFAULT \
//         /**/ (std::string)"^schema:.*" + ','      \
//         /**/ + "^recursive-.*" + ','              \
//         /**/ + "^subcontext.*" + ','              \
//         /**/ + "^branchfs-.*" + ','                         \
//         /**/ + "^associated-branches-.*" + ','              \
//         /**/ + "^as-.*" + ','                               \
//         /**/ + ".*-ctime$" + ','                            \
//         /**/ + ".*-display$" + ','                          \
//         /**/ + ".*-cdrom-count$" + ','                      \
//         /**/ + ".*-dvd-count$" + ','                        \
//         /**/ + ".*-human-readable$" + ','                   \
//         /**/ + ".*-granularity$" + ','                      \
//              /**/ 
//     #define  EAINDEX_EANAMES_NOT_TO_INDEX_DEFAULT \
//     (std::string)"content,as-xml,as-text,as-rdf,rgba-32bpp,exif:thumbnail-update,exif:thumbnail-rgba-32bpp,md5,md2,sha1,mdc2,download-if-mtime-since,force-passive-view,path,realpath,"
    
//     #define  EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT "16384"
    #define  EAINDEX_ODBC3_BULKLOAD "1"

#define CFG_ODBCIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_DEFAULT "atime=timestamp:mtime=timestamp:ctime=timestamp:ferris-should-reindex-if-newer=timestamp:ferris-current-time=timestamp:size=bigint:width=int:height=int:md5=char(40):mimetype=varchar(50):mimetype-from-content=varchar(50):group-owner-number=int:user-owner-number=int:dontfollow-user-owner-number=int:dontfollow-group-owner-number=int"
#define CFG_QTSQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_DEFAULT \
        CFG_ODBCIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_DEFAULT
#define CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_DEFAULT \
        CFG_ODBCIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_DEFAULT
#define CFG_POSTGRESQLIDX_DOCMAP_COLUMNS_TO_FULLTEXT_INDEX_DEFAULT "" 
#define CFG_POSTGRESQLIDX_COLUMNS_TO_INLINE_IN_BITF_CREATE_K "columns-to-bitf"
#define CFG_POSTGRESQLIDX_COLUMNS_TO_INLINE_IN_BITF_K "cfg_postgresqlidx_columns_to_inline_in_bitf_default_k"
#define CFG_POSTGRESQLIDX_COLUMNS_TO_INLINE_IN_BITF_NSV_K "cfg_postgresqlidx_columns_to_inline_in_bitf_nsv_k"
#define CFG_POSTGRESQLIDX_COLUMNS_TO_INLINE_IN_BITF_DEFAULT \
	"exif:has-thumbnail," \
	"writable," \
	"runable," \
	"deletable," \
	"readable," \
	"user-readable," \
	"user-writable," \
	"user-executable," \
	"group-writable," \
	"group-readable," \
	"group-executable," \
	"other-writable," \
	"other-readable," \
	"other-executable," \
	"is-special," \
	"is-source-object," \
	"is-remote," \
	"is-link," \
	"is-image-object," \
	"is-file," \
	"is-dir," \
	"is-audio-object," \
	"is-animation-object," \
	"is-active-view," \
	"has-valid-signature," \
	"has-subcontexts-guess," \
	"has-holes," \
	"has-alpha," \
	"exists-subdir," \
	"dontfollow-is-special," \
	"dontfollow-is-link," \
	"dontfollow-is-file," \
	"dontfollow-is-dir," \
	"dontfollow-has-holes," \
	"rpm-verify-size," \
	"rpm-verify-owner," \
	"rpm-verify-mtime," \
	"rpm-verify-mode," \
	"rpm-verify-md5," \
	"rpm-verify-group," \
	"rpm-verify-device," \
	"rpm-is-readme," \
	"rpm-is-pubkey," \
	"rpm-is-license," \
	"rpm-is-ghost," \
	"rpm-is-doc," \
	"rpm-is-config,is-native," \
    "emblem:has-medallion," \
    "is-setgid,is-setuid,is-sticky,is-unedited,is-unseen,"



};


