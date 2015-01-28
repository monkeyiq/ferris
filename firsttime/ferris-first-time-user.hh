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

    $Id: ferris-first-time-user.hh,v 1.3 2010/09/24 21:31:30 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_FIRST_TIME_USER_H_
#define _ALREADY_INCLUDED_FERRIS_FIRST_TIME_USER_H_

#include "Ferris/FerrisStdHashMap.hh"
#include <string>

namespace Ferris
{
    class ConfigAtom
    {
        std::string name;
        std::string value;
        std::string desc;
        std::string poptOptionName;
        int    pageNumber;
        int    pagePlacement;
        
    public:

        ConfigAtom( std::string name,
                    std::string value,
                    std::string desc,
                    std::string poptOptionName,
                    int    pageNumber,
                    int    pagePlacement )
            :
            name( name ),
            value( value ),
            desc( desc ),
            poptOptionName( poptOptionName ),
            pageNumber( pageNumber ),
            pagePlacement( pagePlacement )
            {
            }

        // eg. DEFAULT_PROXY_ADDRESS
        const char* getName() { return name.c_str();   }

        // eg. localhost
        const char* getValue() { return value.c_str(); }

        // eg. The default proxy address
        const char* getDesc() { return desc.c_str(); }
        
        // eg. --default-proxy-address
        const char* getPoptOptionName() { return poptOptionName.c_str(); }

        // What druid page to display on
        int getPageNumber() { return pageNumber; }

        // where on page to place item
        int getPagePlacement() { return pagePlacement; }
    };

    typedef FERRIS_STD_HASH_MAP< std::string, ConfigAtom > configAtoms_t;
    namespace Factory
    {
        configAtoms_t& getConfigAtoms()
        {
            static configAtoms_t ret;

            if( ret.empty() )
            {
                int page = 0;
                int loc  = 0;
                char* n = 0;

                page = 0;
                loc  = -1;
                n = "--proxy-data-page";
                ret.insert( std::make_pair(
                                n, ConfigAtom( "PROXY_DATA", "", "Proxy settings",
                                               n, page, loc++ )));
                n = "--proxy-address";
                ret.insert( std::make_pair(
                                n, ConfigAtom( "PROXY_ADDRESS", "localhost", "default proxy address",
                                               n, page, loc++ )));
                n = "--proxy-port";
                ret.insert( std::make_pair(
                                n, ConfigAtom( "PROXY_PORT",    "3128", "default proxy port",
                                               n, page, loc++ )));
            }

            return ret;
        }
    };
};

#endif
