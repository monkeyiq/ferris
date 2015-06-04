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

    $Id: FerrisStdHashMap.hh,v 1.5 2010/09/24 21:30:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_STD_HASH_MAP_H_
#define _ALREADY_INCLUDED_FERRIS_STD_HASH_MAP_H_

#include <config.h>

#ifdef PLATFORM_OSX

    #include <unordered_set>
    #include <unordered_map>
    #define FERRIS_STD_HASH_MAP std::unordered_map
    #define FERRIS_STD_HASH_SET std::unordered_set

#else

#include <string>
#include <ext/hash_map>
#include <ext/hash_set>

#ifndef PLATFORM_OSX
    namespace __gnu_cxx
    {
        template<>
        struct hash<std::string>
        {
            size_t operator()(std::string& __s) const
                { return __stl_hash_string(__s.c_str()); }
            size_t operator()(const std::string& __s) const
                { return __stl_hash_string(__s.c_str()); }
        };
    };
#endif
namespace std
{
    using __gnu_cxx::hash;
};
#define FERRIS_STD_HASH_MAP __gnu_cxx::hash_map
#define FERRIS_STD_HASH_SET __gnu_cxx::hash_set




#endif
#endif
