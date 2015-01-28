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

    $Id: FerrisBoost.cpp,v 1.6 2010/09/24 21:30:34 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <Ferris/FerrisBoost.hh>
#include <Ferris/Enamel.hh>
#include <sstream>
#include <map>
#include <string>

using namespace std;
using namespace boost;

namespace Ferris
{

    regexlist_t& stringlist_to_regexlist( regexlist_t& ret, const stringlist_t& sl )
    {
        for( stringlist_t::const_iterator si = sl.begin(); si != sl.end(); ++si )
        {
            ret.push_back( boost::regex( *si ) );
        }
        return ret;
    }
    void erase_any_matches( regexlist_t& rel, stringlist_t& sl )
    {
        for( stringlist_t::iterator si = sl.begin(); si!=sl.end(); )
        {
            stringlist_t::iterator current = si;
            ++si;
            bool remove = false;
            string query_string = *current;

            for( regexlist_t::iterator ri = rel.begin(); ri!=rel.end(); ++ri )
            {
                if(boost::regex_search( query_string, *ri, boost::match_any ))
                {
                    remove = true;
                    break;
                }
            }

            if( remove )
                sl.erase( current );
        }
    }

    void erase_any_matches( const boost::regex& r, stringlist_t& sl )
    {
        for( stringlist_t::iterator si = sl.begin(); si!=sl.end(); )
        {
            stringlist_t::iterator current = si;
            ++si;
            cerr << "test:" << *current << endl;
            if(boost::regex_search( *current, r, boost::match_any ))
            {
                cerr << "erase:" << *current << endl;
                sl.erase( current );
            }
        }
    }
    
    
    std::string MakeCompositeRegexString( const stringlist_t& sl )
    {
        stringstream ss;

        if( sl.empty() )
            return "";

        int pos = 0;
        bool optimizeDone = false;
        for( pos = 0; !optimizeDone; ++pos )
        {
            char ch = 0;
            stringlist_t::const_iterator si = sl.begin();
            if( pos >= si->length() )
            {
                optimizeDone = true;
                break;
            }
            ch = (*si)[ pos ];
            for( ; si != sl.end(); ++si )
            {
                if( pos >= si->length() )
                {
                    optimizeDone = true;
                    break;
                }
                if( ch != (*si)[ pos ] )
                {
                    optimizeDone = true;
                    break;
                }
            }

            if( optimizeDone )
                break;

            //
            // All the strings share this si[pos] in common.
            //
            LG_STRF_D << "common char at pos:" << pos << " ch:" << ch << endl;
            ss << ch;
        }
        
        ss << "(";
        for( stringlist_t::const_iterator si = sl.begin(); si != sl.end(); )
        {
            ss << si->substr( pos );
            ++si;
            if( si != sl.end() )
                ss << "|";
        }
        ss << ")";

        if( LG_STRF_D_ACTIVE )
        {
            for( stringlist_t::const_iterator si = sl.begin(); si != sl.end(); ++si )
            {
                LG_STRF_D << "Given:" << *si << endl;
            }
            LG_STRF_D << "Result:" << ss.str() << endl;
        }

        return ss.str();
    }

    boost::regex toregex( const std::string& s, regex::flag_type rflags )
    {
        return boost::regex( s, rflags | regex::optimize );
    }
    boost::regex toregex( const stringlist_t& sl, boost::regex::flag_type rflags )
    {
        std::string s = MakeCompositeRegexString( sl );
        return toregex( s, rflags );
    }
    fh_rex toregexh( const std::string& s, boost::regex::flag_type rflags )
    {
        return new boost::regex( s, rflags | regex::optimize );
    }
    fh_rex toregexh( const stringlist_t& sl, boost::regex::flag_type rflags )
    {
        std::string s = MakeCompositeRegexString( sl );
        return toregexh( s, rflags );
    }

    boost::regex toregexi( const std::string& s, regex::flag_type rflags )
    {
        return toregex( s, rflags | boost::regex::icase );
    }
    boost::regex toregexi( const stringlist_t& sl, boost::regex::flag_type rflags )
    {
        return toregex( sl, rflags | boost::regex::icase );
    }
    fh_rex toregexhi( const std::string& s, boost::regex::flag_type rflags )
    {
        return toregexh( s, rflags | boost::regex::icase );
    }
    fh_rex toregexhi( const stringlist_t& sl, boost::regex::flag_type rflags )
    {
        return toregexh( sl, rflags | boost::regex::icase );
    }

    string regex_match_single( const std::string& s, 
                               const fh_rex& e, 
                               boost::match_flag_type flags )
    {
        boost::smatch matches;
        if(regex_match( s, matches, e, flags ))
        {
//            cerr << "matches.size:" << matches.size() << endl;
            if( matches.size() == 2 )
            {
                string ret = matches[1];
                return ret;
            }
        }
        return "";
    }

    string regex_match_single( const std::string& s, 
                               const std::string& rex,
                               boost::match_flag_type flags )
    {
        fh_rex r = toregexh( rex );
        return regex_match_single( s, r, flags );
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    struct RegexCollectionPriv
    {
        fh_rex m_regex;
        stringlist_t m_regexlist;
        bool m_caseSensitive;
        
        RegexCollectionPriv( bool caseSensitive )
            :
            m_regex( 0 ),
            m_caseSensitive( caseSensitive )
            {
            }
    };

    RegexCollection::RegexCollection( bool caseSensitive )
        :
        m_priv( new RegexCollectionPriv( caseSensitive ) )
    {
    }
    RegexCollection::~RegexCollection()
    {
    }
    
    void
    RegexCollection::append( const std::string& s )
    {
        if( !s.empty() )
        {
            m_priv->m_regex = 0;
            m_priv->m_regexlist.push_back( s );
        }
    }

    void
    RegexCollection::append( const std::list< std::string >& sl )
    {
        m_priv->m_regex = 0;
        for( stringlist_t::const_iterator si = sl.begin(); si != sl.end(); ++si )
            if( !si->empty() )
                m_priv->m_regexlist.push_back( *si );
    }
    
    
    void RegexCollection::clear()
    {
        m_priv->m_regexlist.clear();
        m_priv->m_regex = 0;
    }
    
    fh_rex
    RegexCollection::getRegex() const
    {
        if( GetImpl(m_priv->m_regex) )
            return m_priv->m_regex;
        if( m_priv->m_regexlist.empty() )
            return 0;

        string s = MakeCompositeRegexString( m_priv->m_regexlist );
        boost::regex::flag_type flags = boost::regex::optimize;
        if( m_priv->m_caseSensitive )
            flags |= boost::regex::icase;
        
        m_priv->m_regex = toregexh( s, flags );
        return m_priv->m_regex;
    }

    std::string replaceg( const std::string& s,
                          const std::string& rex,
                          const std::string& fmt,
                          boost::match_flag_type flags )
    {
        typedef map< string, fh_rex > cache_t;
        static cache_t cache;
        cache_t::iterator iter = cache.find( s );
        if( iter == cache.end() )
        {
            fh_rex r = toregexh( rex );
            iter = cache.insert( make_pair(s, r) ).first;
        }
        return replaceg( s, iter->second, fmt, flags );
    }
    
};
