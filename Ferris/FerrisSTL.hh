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

    $Id: FerrisSTL.hh,v 1.4 2010/09/24 21:30:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <string>
#include <map>

#ifndef _ALREADY_INCLUDED_FERRIS_STL_H_
#define _ALREADY_INCLUDED_FERRIS_STL_H_

namespace Ferris
{

    /**
     * Normal STL has limited sets, using lower_bound() on iterators
     * allows lookup by a compare function but the lower_bound()
     * member function doesn't allow a functor. Thus if one has a
     * collection of classes which are sorted by a key (for example a
     * string) and one wishes to use a string in the member function
     * version of lower_bound() then they can't.
     *
     * This method will use a more efficient but non standard
     * member function version of set::lower_bound() if it exists
     * or will default to using the global lower_bound() function
     * on begin() to end() bidirectional iterators if the std::set
     * doesn't include the extended lower_bound() function.
     *
     * On systems without an extended set::lower_bound member function
     * this call should be equal to
     * lower_bound( col.begin(), col.end(), key, f );
     * though on systems with the extended set member it will perform
     * much faster.
     *
     * @param col The collection to perform a lower_bound( begin, end )
     *            style of operation on
     * @param key The extended key which defines the object to find in col.
     * @param f   A function which compares objects of type col::key and
     *            ExtendedKey much like the functor argument in std::lower_bound()
     * @return an iterator to the object in col which matches key or col.end()
     */
    template < class Collection, class ExtendedKey, class StrictWeakOrdering >
    inline typename Collection::iterator ex_lower_bound( Collection& col,
                                                         const ExtendedKey& key,
                                                         StrictWeakOrdering f )
    {
#ifdef FERRIS_STL_HAS_MEMBER_LOWER_BOUND_WITH_FUNCTOR
        return col.lower_bound( key, f );
#else
        return lower_bound( col.begin(), col.end(), key, f );
#endif
    }


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    
    typedef std::map< std::string, std::string > StringMap_t;

    template < class Col, class E >
    void ensure_back( Col& c, E& e )
    {
        if( c.end() == find( c.begin(), c.end(), e ) )
            c.push_back( e );
    }

    template < class Col >
    void erase( Col& c, const typename  Col::value_type & e )
    {
//        typename Col::iterator ci = c.find( e );
        typename Col::iterator ci = find( c.begin(), c.end(), e );
        if( ci != c.end() )
            c.erase( ci );
    }

    template < class Col >
    bool contains( Col& c, const typename Col::value_type & e )
    {
        typename Col::iterator ci = find( c.begin(), c.end(), e );
        return ci != c.end();
    }

    template < class Col, class OutputIterator >
    OutputIterator copy( Col& c, OutputIterator e )
    {
        return std::copy( c.begin(), c.end(), e );
    }
    
    
/**
 * Copy data from the range first, last to result only copying
 * data that passes pred
 *
 * @return result + (last - first);
 */
    template <class _InputIter, class _OutputIter, class _Predicate>
    _OutputIter
    copy_if( _InputIter first,
             _InputIter last,
             _OutputIter result,
             _Predicate pred)
    {
        for ( ; first != last; ++first)
        {
            if (pred(*first))
            {
                *result = *first;
                ++result;
            }
        }
        return result;
    }

/**
 * Copy data from the range first, last to result stopping when
 * pred fails for the first time. All items upto the item that
 * pred failed on will be in the result.
 *
 * @return result + (last - first);
 */
    template <class _InputIter, class _OutputIter, class _Predicate>
    _OutputIter
    copy_until( _InputIter first,
                _InputIter last,
                _OutputIter result,
                _Predicate pred)
    {
        for ( ; first != last; ++first)
        {
            if (pred(*first))
            {
                *result = *first;
                ++result;
            }
        }
        return result;
    }
        


        
    template <class ItemType, class SetType>
    struct itemInSet
    {
        bool operator()(const ItemType& x, const SetType& se) const
            {
                return se.end() != se.find( x );
            }
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
};
#endif
