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

    $Id: SM.hh,v 1.4 2010/09/24 21:30:58 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/FerrisHandle.hh>
#include <Ferris/Enamel.hh>
#include <functional>

#ifndef _ALREADY_INCLUDED_SM_H_
#define _ALREADY_INCLUDED_SM_H_

namespace Ferris
{
    /**
     * Unary function to convert objects of type X to type Y using existing
     * IOStream inserters and extractors to do the dirty work.
     */
    template <class X, class Y = int >
    struct xtoy
    {
        /**
         * Convert object __x of type X into an object of type Y.
         *
         * @param __x X type object
         * @return Y type object that represents __x
         */
        inline Y operator()(const X& __x) const
            {
                std::istringstream ss(__x);
                Y y;
                ss >> y;
                return y;
            }
    };

    /**
     * Unary function to convert strings to lower case
     */
    struct FERRISEXP_API tolowerstr
    {
        /**
         */
        inline std::string operator()(const std::string& x) const
            {
                std::string ret = x;

                std::transform (ret.begin(), ret.end(), ret.begin(), ::tolower );
                
//                 std::string::iterator e = ret.end();
//                 for( std::string::iterator p = ret.begin(); p != e; ++p )
//                 {
//                     *p = ::tolower( *p );
//                 }

                // This is nastier code but not really much faster.
                // size() and length() both seem to use begin()/end()
                // to work out the string size.
//                 char* p = (char*)ret.data();
//                 int max = ret.size();
//                 for( int i = 0; i < max; ++i, ++p )
//                 {
//                     *p = ::tolower( *p );
//                 }
                
                return ret;
            }
    };

    struct FERRISEXP_API toupperstr
    {
        inline std::string operator()(const std::string& x) const
            {
                std::string ret = x;
                
                for( std::string::iterator p = ret.begin(); p != ret.end(); ++p )
                {
                    *p = ::toupper( *p );
                }
                
                return ret;
            }
    };



    template <class _Operation1, class _Operation2, class _Operation3>
    class tuple_binary_compose
    {
    protected:

        _Operation1 _M_op1;
        _Operation2 _M_op2;
        _Operation3 _M_op3;
    
    public:

        tuple_binary_compose(
            const _Operation1& __x = _Operation1(),
            const _Operation2& __y = _Operation2(),
            const _Operation3& __z = _Operation3())
            :
            _M_op1(__x), _M_op2(__y), _M_op3(__z)
            { }

        typename _Operation1::result_type
        inline operator()(
            const typename _Operation2::argument_type& __x,
            const typename _Operation2::argument_type& __y
            ) const
            {
                return _M_op1(_M_op2(__x), _M_op3(__y));
            }
    };

    template <class _Operation1, class _Operation2, class _Operation3>
    inline tuple_binary_compose<_Operation1, _Operation2, _Operation3>
    tuple_compose2(const _Operation1& __op1,
                   const _Operation2& __op2,
                   const _Operation3& __op3)
    {
        return tuple_binary_compose<_Operation1,_Operation2,_Operation3>
            (__op1, __op2, __op3);
    }
 
};
#endif
