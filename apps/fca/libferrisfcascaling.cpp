/******************************************************************************
*******************************************************************************
*******************************************************************************

    create a scale from the URLs in the EA index
    Copyright (C) 2005 Ben Martin

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

    $Id: libferrisfcascaling.cpp,v 1.5 2010/09/24 21:31:11 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "libferrisfcascaling.hh"
#include <Ferris/Trimming.hh>
#include <Ferris/EAIndexerSQLCommon_private.hh>

namespace Ferris
{
    const char* CFG_IDX_USER_K   = "cfg-idx-user";
    const char* CFG_IDX_HOST_K   = "cfg-idx-host";
    const char* CFG_IDX_PORT_K   = "cfg-idx-port";
    const char* CFG_IDX_DBNAME_K = "cfg-idx-dbname";
    const char* CFG_IDX_USER_DEF   = "";
    const char* CFG_IDX_HOST_DEF   = "localhost";
    const char* CFG_IDX_PORT_DEF   = "";
    const char* CFG_IDX_DBNAME_DEF = "";


    string cleanAttributeName( const std::string& const_s )
    {
        static tolowerstr lower;
        string s = const_s;
        s = lower(s);
        s = Util::replace_all( s, '_', "__" );
        s = Util::replace_all( s, ' ', '_' );
        s = Util::replace_all( s, '<', "_lt_" );
        s = Util::replace_all( s, '>', "_gt_" );
        s = EANameToSQLColumnName( s );
        return s;
    }

    EAIndex::fh_idx getEAIndex( const char* findexPath_CSTR )
    {
        EAIndex::fh_idx idx = 0;

        if( findexPath_CSTR )
        {
            idx = EAIndex::Factory::getEAIndex( findexPath_CSTR );
        }
        else
        {
            idx = EAIndex::Factory::getDefaultEAIndex();
        }

        return idx;
    }


    static stringmap_t&
    handle_possible_attribute_length_truncation( stringmap_t& ret, const std::string& treename )
    {
        int uniq = 1;
        const int maxlength = 62 - treename.length();

        for( stringmap_t::iterator ri = ret.begin(); ri!=ret.end();  )
        {
            if( ri->first.length() > maxlength )
            {
                string k = ri->first;
                string v = ri->second;

                stringmap_t::iterator t = ri;
                ++ri;
                ret.erase( t );
                string newk = k.substr( 0, maxlength ) + tostr(uniq++);
                ret.insert( make_pair( newk, v ) );
            }
            else
                ++ri;
        }

        cerr << "handle_possible_attribute_length_truncation()" << endl;
        for( stringmap_t::iterator ri = ret.begin(); ri!=ret.end(); ++ri )
        {
            cerr << "k:" << ri->first << endl;
        }
        
        return ret;
    }
    
    static stringmap_t&
    readAttributesStdIn( stringmap_t& ret, const std::string& treename )
    {
        cerr << "Assuming attribute ffilter pairs are on stdin! FORMAT: attrname 'ffilter'\\n" << endl;
        
        PrefixTrimmer trimmer;
        trimmer.push_back( "'" );
        trimmer.push_back( " " );
        PostfixTrimmer ptrimmer;
        ptrimmer.push_back( "'" );
        ptrimmer.push_back( " " );
        
        string s;
        string ffilterstring;
        while( cin )
        {
            if( cin >> s && getline( cin, ffilterstring ) )
            {
                ffilterstring = trimmer(  ffilterstring );
                ffilterstring = ptrimmer( ffilterstring );
                ret[ s ] = ffilterstring;
            }
        }
        return handle_possible_attribute_length_truncation( ret, treename );
    }
    
    stringmap_t&
    readAttributes( stringmap_t& ret, poptContext& optCon, const std::string& treename )
    {
        string k = "";
        while( const char* s_CSTR = poptGetArg(optCon) )
        {
            string s = s_CSTR;
            if( k.empty() )
            {
                k = s;
                if( k == "-" )
                {
                    return readAttributesStdIn( ret, treename );
                }
            }
            else
            {
                ret.insert( make_pair( k, s ) );
                k = "";
            }
        }

        if( ret.empty() )
            return readAttributesStdIn( ret, treename );
            
        return handle_possible_attribute_length_truncation( ret, treename );
    }
    
    std::string guessLookupTableName( work& trans, std::string attrname )
    {
        stringstream ss;
        ss << "select attrid, attrtype from attrmap where attrname='" << attrname << "'";
        cerr << "guessLookupTableName() SQL:  " << ss.str() << endl;
        
        result res = trans.exec( tostr( ss ) );
        cerr << "guessLookupTableName() res.sz:" << res.size() << endl;
        
        for (result::const_iterator c = res.begin(); c != res.end(); ++c)
        {
            int attrid = 0;
            int attrtype = 0;
            c[0].to(attrid);
            c[1].to(attrtype);
            cerr << " att:" << attrtype
                 << " MetaEAIndexerInterface::ATTRTYPEID_DBL:" << MetaEAIndexerInterface::ATTRTYPEID_DBL
                 << endl;
            
            switch( (MetaEAIndexerInterface::AttrType_t)attrtype )
            {
            case MetaEAIndexerInterface::ATTRTYPEID_INT:
                return "intlookup";
            case MetaEAIndexerInterface::ATTRTYPEID_DBL:
                return "doublelookup";
            case MetaEAIndexerInterface::ATTRTYPEID_TIME:
                return "timelookup";
            case MetaEAIndexerInterface::ATTRTYPEID_STR:
            case MetaEAIndexerInterface::ATTRTYPEID_CIS:
                return "strlookup";
            }
        }
        return "intlookup";
    }
    
    std::string formalTimeValueForFormalAttribute( std::string v )
    {
        if( v.find( '_' ) != string::npos )
            return v;
        
        stringstream ss;
        ss << v;
        time_t tt = 0;
        ss >> tt;

        cerr << "formalTimeValueForFormalAttribute() v:" << v
             << " tt:" << tt
             << " tt.ts:" << Time::toTimeString( tt, "%y %b %e %H:%M" )
             << " ret:" << EANameToSQLColumnName(Time::toTimeString( tt, "%y %b %e %H:%M" ))
             << endl;

        return EANameToSQLColumnName(Time::toTimeString( tt, "%Y %b %e %H:%M" ));
    }
    


};

