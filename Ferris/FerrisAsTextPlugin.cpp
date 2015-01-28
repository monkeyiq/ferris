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

    $Id: FerrisAsTextPlugin.cpp,v 1.2 2010/09/24 21:30:34 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisAsTextPlugin.hh>
#include <Ferris.hh>

using namespace std;

namespace Ferris
{
    static stringmap_t& getFromMimeMap()
    {
        static stringmap_t o;
        return o;
    }
    static stringmap_t& getFromFerrisTypeMap()
    {
        static stringmap_t o;
        return o;
    }
    typedef list< pair< fh_matcher, string > > MatcherMap_t;
    static MatcherMap_t& getFromMatcherMap()
    {
        static MatcherMap_t o;
        return o;
    }
    
    bool RegisterAsTextFromMime( const std::string& mimetype,
                                 const std::string& libname )
    {
        getFromMimeMap().insert( make_pair( mimetype, libname ) );
    }
    
    bool RegisterAsTextFromFerrisType( const std::string& mimetype,
                                       const std::string& libname )
    {
        getFromFerrisTypeMap().insert( make_pair( mimetype, libname ) );
    }

    bool RegisterAsTextFromMatcher(
        const fh_matcher& ma,
        const std::string& libname )
    {
        getFromMatcherMap().push_back( make_pair( ma, libname ) );
    }
    
    
    std::string getLibraryNameFromMime( const std::string& mimetype )
    {
        stringmap_t::iterator mi = getFromMimeMap().find( mimetype );
        if( mi == getFromMimeMap().end() )
            return "";
        return mi->second;
    }
    
    std::string getLibraryNameFromFerrisType( const std::string& s )
    {
        stringmap_t::iterator mi = getFromFerrisTypeMap().find( s );
        if( mi == getFromFerrisTypeMap().end() )
            return "";
        return mi->second;
    }

    std::string getLibraryNameFromMatcher( fh_context& c )
    {
        for( MatcherMap_t::const_iterator ci = getFromMatcherMap().begin();
             ci != getFromMatcherMap().end(); ++ci )
        {
            fh_matcher m = ci->first;
            if( m( c ) )
            {
                return ci->second;
            }
        }
        return "";
    }
    
    
};

