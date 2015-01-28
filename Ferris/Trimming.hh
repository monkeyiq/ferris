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

    $Id: Trimming.hh,v 1.2 2010/09/24 21:31:00 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_TRIMMING_H_
#define _ALREADY_INCLUDED_TRIMMING_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <vector>
#include <string>
#include <iostream>


class FERRISEXP_API PrefixTrimmerOps
{
public:

    inline bool match( const std::string& s, const std::string& code )
        {
            std::string::size_type e = s.find( code );
            return e == 0;
        }

    inline std::string trim( const std::string& s, const std::string& code )
        {
            return s.substr( code.length() );
        }
    
};


class FERRISEXP_API PostfixTrimmerOps
{
public:

    inline bool match( const std::string& s, const std::string& code )
        {
            std::string::size_type e = s.rfind( code );
            return e == (s.length() - code.length());
        }

    inline std::string trim( const std::string& s, const std::string& code )
        {
            return s.substr( 0, s.length() - code.length() );
        }
    
};


class FERRISEXP_API TrimmerBase
{
protected:

    typedef std::vector<std::string> codes_t;

private:
    
    codes_t codes;

protected:

    codes_t& getCodes()
        {
            return codes;
        }

public:

    virtual ~TrimmerBase()
        {}
    

    void push_back( const std::string& s )
        {
            codes.push_back(s);
        }

    void clear()
        {
            codes.clear();
        }
    
    virtual std::string operator()( std::string s ) = 0;
};


template < class MatchingT >
class Trimmer : public TrimmerBase
{
    MatchingT mt;

public:

    virtual std::string operator()( std::string s ) 
        {
            bool foundPrefix = true;

            while( foundPrefix )
            {
                foundPrefix = false;
                
                for( codes_t::iterator iter = getCodes().begin();
                     iter != getCodes().end();
                     ++iter )
                {
                    if( iter->length() < s.length() )
                    {
                        if( mt.match( s, *iter ) )
                        {
                            foundPrefix = true;
                            s = mt.trim( s, *iter );
                            break;
                        }
                    }
                }
                
            }

            return s;
        }
};

template < class X, class Y >
class CompositeTrimmer : public TrimmerBase
{
    X x;
    Y y;
    
public:

    virtual std::string operator()( std::string s )
        {
            x.clear();
            y.clear();

            for( codes_t::iterator iter = getCodes().begin();
                 iter != getCodes().end();
                 ++iter )
            {
                x.push_back( *iter );
                y.push_back( *iter );
            }
            
            s = x(s);
            s = y(s);

            return s;
        }
    
    
};


typedef Trimmer<PrefixTrimmerOps > PrefixTrimmer;
typedef Trimmer<PostfixTrimmerOps> PostfixTrimmer;
typedef CompositeTrimmer< PrefixTrimmer, PostfixTrimmer > PrePostTrimmer;


#endif
