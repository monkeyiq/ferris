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

    $Id: FullTextIndexer_private.hh,v 1.4 2010/09/24 21:30:52 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_FULLTEXTIDX_PRV_H_
#define _ALREADY_INCLUDED_FERRIS_FULLTEXTIDX_PRV_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/FullTextIndexer.hh>
#include <string>

namespace Ferris
{
    namespace FullTextIndex 
    {
#define IDXMGR_INDEX_CLASS_K          "index-manager-main-class-name"
#define IDXMGR_INDEX_CLASS_DEFAULT    "lucene"
//#define IDXMGR_INDEX_CLASS_DEFAULT    "native"
        
#define IDXMGR_MAXPTRS_PER_SKIPLISTCHUNK_K "idxmgr-maxptrs-per-skiplistchunk-k"
#define IDXMGR_MAXPTRS_PER_SKIPLISTCHUNK_DEFAULT  "512"
#define IDXMGR_DGAP_CODE_K            "idxmgr-dgap-code-k"
#define IDXMGR_DGAP_CODE_DEFAULT      "Interpolative"
#define IDXMGR_FDT_CODE_K             "idxmgr-fdt-code-k"
#define IDXMGR_FDT_CODE_DEFAULT       "Gamma"
#define IDXMGR_LEXICON_CLASS_K        "index-manager-lexicon-class"
#define IDXMGR_LEXICON_CLASS_DEFAULT  "FrontCodedBlocks (3-in-4)"
#define IDXMGR_INVERTEDFILE_CLASS_K   "index-manager-inverted-file-class"
#define IDXMGR_INVERTEDFILE_CLASS_DEFAULT "native"
#define IDXMGR_CASESEN_CLASS_K        "index-manager-case-sensitive"
#define IDXMGR_DROPSTOPWORDS_CLASS_K  "index-manager-drop-stop-words"
#define IDXMGR_STOPWORDSLIST_K        "index-manager-stop-words-list"
#define IDXMGR_STOPWORDSLIST_DEFAULT  "the,a,and"
#define IDXMGR_STEMMER_CLASS_K        "index-manager-stemmer"
#define IDXMGR_SUPPORTS_RANKED_K      "index-manager-supports-ranked-query"

#define IDXMGR_LEXICON_VERSION_K       "index-manager-lexicon-version"
#define IDXMGR_INVERTEDFILE_VERSION_K  "index-manager-invertedfile-version"
#define IDXMGR_DOCUMENTMAP_VERSION_K   "index-manager-documentmap-version"
#define IDXMGR_LEXICON_VERSION_CURRENT         1
#define IDXMGR_INVERTEDFILE_VERSION_CURRENT    1
#define IDXMGR_DOCUMENTMAP_VERSION_CURRENT     1 

#define IDXMGR_DOCMAP_USE_SECIDX_K        "idxmgr-docmap-use-secidx-k"
#define IDXMGR_DOCMAP_USE_SECIDX_DEFAULT  "1"

#define IDXMGR_DOCMAP_REVOKED_ID_CACHE_K        "idxmgr-docmap-revoked_id_cache-k"
#define IDXMGR_DOCMAP_REVOKED_ID_CACHE_DEFAULT  ""
        
        FERRISEXP_API extern const std::string IDXMGR_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_K;
        FERRISEXP_API std::string GET_INDEX_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_DEFAULT( bool commaSeperated = false );
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        /**
         * Create a new index.
         */
        FERRISEXP_API fh_idx createFullTextIndex(
            const std::string& index_classname,
            fh_context c,
            bool caseSensitive,
            bool dropStopWords,
            StemMode stemMode,
            const std::string& lex_class,
            fh_context md = 0 );

        
    };
};
#endif
