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

    $Id: Hashing.hh,v 1.5 2010/09/24 21:30:52 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_HASHING_H_
#define _ALREADY_INCLUDED_FERRIS_HASHING_H_

#include <glib.h>
#include "Ferris/FerrisStdHashMap.hh"
#include <functional>

#include <TypeDecl.hh>

namespace Ferris
{
    template < class T >
    struct f_equal_to
    {
        inline bool operator()(const T __x, const T __y) const
            { return __x == __y; }
    };
    template <>
    struct f_equal_to< fh_context >
    {
        inline bool operator()(const fh_context __x, const fh_context __y) const
            { return GetImpl(__x) == GetImpl(__y); }
    };


    /*
     * This may collide somewhat on a 64bit machine.
     */
    template< class T > 
    struct f_hash
    {
        inline size_t operator()( const T s ) const
            {
                return GPOINTER_TO_INT( s );
            }
    };
    template<> 
    struct f_hash<fh_context> 
    {
        inline size_t operator()( const fh_context s ) const
            {
                return (size_t)GetImpl(s);
//                return GPOINTER_TO_INT( GetImpl(s) );
            }
    };



//     template < class T* >
//     struct f_equal_to : public std::binary_function< T*, T*, bool> 
//     {
//         inline bool operator()(const T* __x, const T* __y) const
//             { return __x == __y; }
//     };

//     /*
//      * This may collide somewhat on a 64bit machine.
//      */
//     template< class T* > 
//     struct f_hash : public std::unary_function< T*, size_t >
//     {
//         inline size_t operator()( const T* s ) const
//             {
//                 return GPOINTER_TO_INT( s );
//             }
//     };

    
    

//(const STL::hash<Ferris::Context* const>) (Ferris::Context* const&)'
    
//     /*
//      * This may collide somewhat on a 64bit machine.
//      */
//     template< class T* const > 
//     struct f_hash : public std::unary_function< T* const& , size_t >
//     {
//         inline size_t operator()( T* const& s ) const
//             {
//                 return GPOINTER_TO_INT( s );
//             }
//     };
    
};

namespace std
{
//     template <class T>
//     struct equal_to : public binary_function<T,T,bool> 
//     {
//         bool operator()(const T& __x, const T& __y) const { return __x == __y; }
//     };
    

//     template <>
//     struct equal_to<Context*>
//         :
//         public std::binary_function< Context*, Context*, bool> 
//     {
//         bool operator()(const Context* __x, const Context* __y) const
//             { return __x == __y;
//             }
//     };

//     template <>
//     struct equal_to<class T*> : public std::binary_function<T*,T*,bool> 
//     {
//         bool operator()(const T* const __x, const T* const __y) const
//             { return __x == __y;
//             }
//     };

    
//     template <class _Key> struct hash { };

//     /*
//      * This may collide somewhat on a 64bit machine.
//      */
//     template<>
//     struct hash<Context*>
//     {
//         size_t operator()( const Context* s ) const
//             {
//                 return GPOINTER_TO_INT( s );
//             }
//     };

    
//     /*
//      * This may collide somewhat on a 64bit machine.
//      */
//     template<>
//     struct hash<class T*>
//     {
//         size_t operator()( const T* s ) const
//             {
//                 return GPOINTER_TO_INT( s );
//             }
//     };

//     /*
//      * This may collide somewhat on a 64bit machine.
//      */
//     template<>
//     struct hash<class T* const>
//     {
//         size_t operator()( T* const&  s ) const
//             {
//                 return GPOINTER_TO_INT( s );
//             }
//     };
    
};
#endif
