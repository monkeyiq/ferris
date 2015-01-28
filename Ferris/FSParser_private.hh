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

    $Id: FSParser_private.hh,v 1.4 2010/09/24 21:30:33 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_FSPARSER_H_
#define _ALREADY_INCLUDED_FERRIS_FSPARSER_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <FerrisLoki/loki/Functor.h>
#include <string>

#define BOOST_SPIRIT_RULE_SCANNERTYPE_LIMIT 3

#include <boost/spirit.hpp>
#include <boost/spirit/home/classic/utility/regex.hpp>
using namespace boost::spirit;

#include <boost/function.hpp>
#include <boost/lambda/lambda.hpp>
using namespace boost::lambda;

// #ifdef GCC_HASCLASSVISIBILITY
// #pragma GCC visibility push(hidden)
// #endif

// #ifdef GCC_HASCLASSVISIBILITY
// #pragma GCC visibility pop
// #endif

namespace Ferris
{
    // see play/boost/spirit for this code.
    struct FERRISEXP_DLLLOCAL Tramp
    {
        typedef Loki::Functor<
            void, LOKI_TYPELIST_2( const char*, const char* ) >
        SAction_t;
        mutable SAction_t SAction;
        typedef const char* IteratorT;
    
        Tramp( const SAction_t& SAction )
            :
            SAction( SAction )
            {
            }
        template <typename PointerToObj, typename PointerToMemFn>
        Tramp( const PointerToObj& pObj, PointerToMemFn pMemFn )
            :
            SAction( SAction_t( pObj, pMemFn ) )
            {
            }

        void operator()( IteratorT first, IteratorT last) const
            {
                SAction( first, last );
            }
    };
    
    class FERRISEXP_DLLLOCAL FSParser
    {
        typedef FSParser _Self;
    
        int m_e;
        std::string m_name;
        std::string m_group;
        std::string m_checksum;

        typedef Tramp F_t;
        template < typename PointerToMemFn >
        F_t F( PointerToMemFn pMemFun )
        {
            return F_t( this, pMemFun );
        }
        
    public:

        FSParser();
        int toint( std::string s );
        std::string reverse( std::string s );

        void get_name_f( const char* beg, const char* end );
        void get_group_f( const char* beg, const char* end );
        void get_checksum_f( const char* beg, const char* end );
        void get_e_f( const char* beg, const char* end );
        bool parse( const std::string& s_const );
    
        std::string getChecksum();
        std::string getGroup();
        std::string getName();
        int getE();
    };
    
};
#endif
