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

    $Id: EAIndexer_private.hh,v 1.6 2010/09/24 21:30:31 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_EAIDX_PRIVATE_H_
#define _ALREADY_INCLUDED_FERRIS_EAIDX_PRIVATE_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/EAIndexer.hh>

namespace Ferris
{
    namespace EAIndex 
    {
        FERRISEXP_API extern const std::string EAINDEXROOT;
        FERRISEXP_API extern const std::string DB_EAINDEX;

#define EAIDXMGR_INDEX_CLASS_K          "eaindex-manager-main-class-name"
#define EAIDXMGR_INDEX_CLASS_DEFAULT    "db4"

        FERRISEXP_API void SET_EAINDEX_EANAMES_NOT_TO_INDEX_DEFAULT( const std::string& v );
        FERRISEXP_API void SET_EAINDEX_EANAMES_REGEX_IGNORE_DEFAULT( const std::string& v );
        FERRISEXP_API void SET_EAINDEX_EANAMES_REGEX_DEFAULT( const std::string& v );
        FERRISEXP_API void SET_EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT( const std::string& v );
        FERRISEXP_API void SET_EAINDEX_EANAMES_TO_INDEX_REGEX_DEFAULT( const std::string& v );

        FERRISEXP_API extern const std::string IDXMGR_EXPLICIT_EANAMES_TO_INDEX_K;

        FERRISEXP_API extern const std::string IDXMGR_EANAMES_TO_INDEX_REGEX_K;
        
        FERRISEXP_API extern const std::string IDXMGR_EANAMES_IGNORE_K;
//        FERRISEXP_API extern const std::string IDXMGR_EANAMES_IGNORE_DEFAULT;
        FERRISEXP_API extern const std::string IDXMGR_EANAMES_REGEX_IGNORE_K;
//        FERRISEXP_API extern const std::string IDXMGR_EANAMES_REGEX_IGNORE_DEFAULT;
        FERRISEXP_API extern const std::string IDXMGR_MAX_VALUE_SIZE_K;
//        FERRISEXP_API extern const std::string IDXMGR_MAX_VALUE_SIZE_DEFAULT;
        FERRISEXP_API extern const std::string IDXMGR_EANAMES_REGEX_K;

        FERRISEXP_API extern const std::string IDXMGR_FULLTEXT_INDEX_PATH_K;


        FERRISEXP_API extern const std::string IDXMGR_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_K;
        FERRISEXP_API void SET_EAINDEX_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_DEFAULT( const std::string& v );
        FERRISEXP_API std::string GET_EAINDEX_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_DEFAULT( bool commaSeperated = false );

        
#define IDXMGR_EA_DGAP_CODE_K            "idxmgr-dgap-code-k"
#define IDXMGR_EA_DGAP_CODE_DEFAULT      "none"
        
        FERRISEXP_API fh_idx createEAIndex( fh_context c,
                                            const std::string& attributeMap_class,
                                            const std::string& forwardValueMap_class,
                                            const std::string& reverseValueMap_class,
                                            const std::string& eanames_ignore,
                                            const std::string& eanames_ignore_regex,
                                            std::streamsize max_value_size );
    };
};
#endif
