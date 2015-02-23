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

    $Id: FSParser.cpp,v 1.3 2010/09/24 21:30:33 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "FSParser_private.hh"
#include <SignalStreams.hh>
#include <General.hh>
#include <SM.hh>

using namespace std;


namespace Ferris
{
    FSParser::FSParser()
        :
        m_e( 0 )
    {
    }
    
    int
    FSParser::toint( string s )
    {
        stringstream ss;
        ss << s;
        int ret;
        ss >> ret;
        return ret;
    }
    
    string
    FSParser::reverse( string s )
    {
        std::reverse( s.begin(), s.end() );
        return s;
    }
    
    
    void
    FSParser::get_name_f( const char* beg, const char* end )
    {
        string v( beg, end );
        m_name = reverse( v );
        while( !m_name.empty() )
        {
            char ch = m_name[ 0 ];
            if( ch == '-' || ch == '_' || ch == ' ' )
            {
                m_name = m_name.substr( 1 );
            }
            else
                return;
        }
    }

    void
    FSParser::get_group_f( const char* beg, const char* end )
    {
        string v( beg, end );
        m_group = reverse( v );
    }
    void
    FSParser::get_checksum_f( const char* beg, const char* end )
    {
        string v( beg, end );
        m_checksum = reverse( v );
        m_checksum = Util::replace_all( m_checksum, "[", "" );
        m_checksum = Util::replace_all( m_checksum, "]", "" );
        m_checksum = tolowerstr()( m_checksum );
    }
    void
    FSParser::get_e_f( const char* beg, const char* end )
    {
        string v( beg, end );
        m_e = toint( reverse( v ) );
    }
    
    bool
    FSParser::parse( const std::string& s_const )
    {
        string s = s_const;
        typedef scanner_list<scanner<>, phrase_scanner_t> scanners;
        typedef rule< scanners > R;
            
        s = reverse( s );
            
        R checksum_p = regex_p("\\][A-F0-9][A-F0-9][A-F0-9][A-F0-9][A-F0-9][A-F0-9][A-F0-9][A-F0-9]\\[")[ F( &_Self::get_checksum_f ) ];
        R group_p    = regex_p("\\][^]]+\\[")[ F( &_Self::get_group_f ) ];
        R cg_p = checksum_p | group_p;
        R extension_p = regex_p("[A-Za-z4]+\\.");
        R junk_p     = regex_p("[-_]+") | regex_p("\\).*\\(");
        R e_p        = regex_p("[0-9]+")[ F( &_Self::get_e_f ) ];
        R name_p     = regex_p("[^]]+")[ F( &_Self::get_name_f ) ];
        R fs_p = extension_p >> *cg_p >> *junk_p
                             >> *(regex_p("[0-9]+[Vv]") >> *junk_p)
                             >> *e_p
                             >> *junk_p >> name_p >> *junk_p >> *cg_p;

        parse_info<> info = ::boost::spirit::parse( s.c_str(), fs_p, space_p );
        if (info.full)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
    
    string
    FSParser::getChecksum()
    {
        return m_checksum;
    }
    string
    FSParser::getGroup()
    {
        return m_group;
    }
    string
    FSParser::getName()
    {
        return m_name;
    }
    int
    FSParser::getE()
    {
        return m_e;
    }

};
