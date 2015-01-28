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

    $Id: Iterator.hh,v 1.2 2010/09/24 21:30:53 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_ITERATOR_H_
#define _ALREADY_INCLUDED_FERRIS_ITERATOR_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <SmartPtr.h>
#include <iostream>
#include <iterator>

namespace std
{

template <class _Collection, typename _Iterator = typename _Collection::iterator >
class circular_iterator : 
    public iterator<typename iterator_traits<_Iterator>::iterator_category,
                    typename iterator_traits<_Iterator>::value_type,
                    typename iterator_traits<_Iterator>::difference_type,
                    typename iterator_traits<_Iterator>::pointer,
                    typename iterator_traits<_Iterator>::reference>
{
protected:
    _Iterator current;
    _Iterator initial;
    _Collection& col;
    
    typedef circular_iterator<_Collection,_Iterator> _Self;
public:
    typedef typename iterator_traits<_Iterator>::iterator_category  iterator_category;
    typedef typename iterator_traits<_Iterator>::value_type         value_type;
    typedef typename iterator_traits<_Iterator>::difference_type    difference_type;
    typedef typename iterator_traits<_Iterator>::pointer            pointer;
    typedef typename iterator_traits<_Iterator>::reference          reference;
    typedef _Iterator   iterator_type;
    typedef _Collection collection_type;
    
public:
    
    circular_iterator() {}
    explicit circular_iterator(collection_type& __c, iterator_type& __x)
        : col(__c), current(__x), initial(__x) {}
    explicit circular_iterator(collection_type& __c)
        : col(__c), current( __c.end() ), initial( __c.end() ) {}

    template
    <
        typename T1,
        template <class> class OP1,
        class CP1,
        template <class> class KP1,
        template <class> class SP1
    >
    explicit circular_iterator( const Loki::SmartPtr<T1, OP1, CP1, KP1, SP1>& __c,
                                iterator_type& __x )
        : col( *GetImpl(__c)), current( __x ), initial( __x ) {}
    template
    <
        typename T1,
        template <class> class OP1,
        class CP1,
        template <class> class KP1,
        template <class> class SP1
    >
    explicit circular_iterator( const Loki::SmartPtr<T1, OP1, CP1, KP1, SP1>& __c )
        : col( *GetImpl(__c)), current( __c->end() ), initial( __c->end() ) {}
    
    circular_iterator(const _Self& __x)
        : col(__x.col), current(__x.current), initial(__x.initial) {}
    _Self& operator = (const _Self& __x)
        { col = __x.col; current = __x.base(); initial = __x.initial; return *this; } 
    
    iterator_type base() const { return current; }

    void __advance( iterator_type& i ) 
        {
            if( i != col.end() )
            {
                ++i;
                if( i == col.end() )
                {
                    i = col.begin();
                }
                
                if( i == initial )
                {
                    i = col.end();
                }
            }
        }
    
    reference operator*() 
        {
            _Iterator __tmp = current;

//             if( __tmp == initial )
//                 __tmp = col.end();
            
//            __advance( __tmp );
            return *__tmp;
        }

    _Self& operator++() {
        __advance( current );
        return *this;
    }
    _Self operator++(int) {
        _Self __tmp = *this;
        __advance( current );
        return __tmp;
    }
//     _Self& operator--() {
//         __advance( current );
//         return *this;
//     }
//     _Self operator--(int) {
//         _Self __tmp = *this;
//         ++current;
//         return __tmp;
//     }

//     _Self operator+(difference_type __n) const {
//         return _Self(current - __n);
//     }
//     _Self& operator+=(difference_type __n) {
//         current -= __n;
//         return *this;
//     }
//     _Self operator-(difference_type __n) const {
//         return _Self(current + __n);
//     }
//     _Self& operator-=(difference_type __n) {
//         current += __n;
//         return *this;
//     }
//    reference operator[](difference_type __n) const { return *(*this + __n); }  
}; 
 
template <class _Collection, class _Iterator>
inline bool   operator==(const circular_iterator<_Collection,_Iterator>& __x, 
                                   const circular_iterator<_Collection,_Iterator>& __y) {

    return __x.base() == __y.base();
}


template <class _Collection, class _Iterator>
inline bool  operator<(const circular_iterator<_Collection,_Iterator>& __x, 
                                 const circular_iterator<_Collection,_Iterator>& __y) {
    return __y.base() < __x.base();
}


template <class _Collection, class _Iterator>
inline bool  operator!=(const circular_iterator<_Collection,_Iterator>& __x, 
                                  const circular_iterator<_Collection,_Iterator>& __y) {
    return !(__x == __y);
}

template <class _Collection, class _Iterator>
inline bool  operator>(const circular_iterator<_Collection,_Iterator>& __x, 
                                 const circular_iterator<_Collection,_Iterator>& __y) {
    return __y < __x;
}

template <class _Collection, class _Iterator>
inline bool  operator<=(const circular_iterator<_Collection,_Iterator>& __x, 
                                  const circular_iterator<_Collection,_Iterator>& __y) {
    return !(__y < __x);
}

template <class _Collection, class _Iterator>
inline bool  operator>=(const circular_iterator<_Collection,_Iterator>& __x, 
                                  const circular_iterator<_Collection,_Iterator>& __y) {
    return !(__x < __y);
}

    
template <class _Collection>
circular_iterator<_Collection, typename _Collection::iterator>
make_circular_iterator( _Collection& c )
{
    return circular_iterator<_Collection, typename _Collection::iterator>(c);
}

template
<
    typename T1,
    template <class> class OP1,
    class CP1,
    template <class> class KP1,
    template <class> class SP1
>
circular_iterator< T1, typename T1::iterator>
make_circular_iterator( const Loki::SmartPtr<T1, OP1, CP1, KP1, SP1>& c )
{
    return circular_iterator< T1, typename T1::iterator>( *GetImpl(c) );
}

template
<
    typename T1,
    template <class> class OP1,
    class CP1,
    template <class> class KP1,
    template <class> class SP1,
    class _Iterator
>
circular_iterator< T1, _Iterator >
make_circular_iterator( const Loki::SmartPtr<T1, OP1, CP1, KP1, SP1>& c,
                        _Iterator& i )
{
    return circular_iterator< T1, _Iterator >( *GetImpl(c), i );
}

template <class _Collection, class _Iterator >
circular_iterator<_Collection, typename _Collection::iterator >
make_circular_iterator( _Collection& c,
                        typename _Collection::iterator& i )
{
    return circular_iterator<_Collection, typename _Collection::iterator>( c, i );
}



// template
// <
//     typename T1,
//     template <class> class OP1,
//     class CP1,
//     template <class> class KP1,
//     template <class> class SP1
// >
// circular_iterator< T1, typename T1::iterator >
// make_circular_iterator( const Loki::SmartPtr<T1, OP1, CP1, KP1, SP1>& c,
//                         typename T1::iterator& i )
// {
//     return circular_iterator< T1, typename T1::iterator>( *GetImpl(c), i );
// }



/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

    template <class T>
    T ForceNegative( T x )
    {
        if( x >= 0 ) return -1 * x;
        return x;
    }
    
/**
 * Iterate over the domain of a map as if it was a vector<> or list<>
 */
template < class DelegateIterClass  >
class map_domain_iterator_class
{
    typedef map_domain_iterator_class< DelegateIterClass > _Self;
    
public:

    typedef typename std::iterator_traits<DelegateIterClass> traits_type;
    typedef typename traits_type::iterator_category      iterator_category;
    typedef typename traits_type::value_type::first_type value_type;
    typedef long                                         difference_type;
    typedef value_type                                   pointer;
    typedef value_type                                   reference;
    
    
    map_domain_iterator_class( DelegateIterClass de )
        :
        m_delegate( de )
        {
        }

    reference operator*() const
        {
            return m_delegate->first;
        }
    
    pointer operator->()
        {
            return m_delegate->first;
        }

    _Self& operator++()
        {
            shiftIterator(1);
            return *this;
        }
    
    _Self  operator++(int)
        {
            _Self tmp(*this);
            shiftIterator(1);
            return tmp;
        }
    
    _Self& operator--()
        {
            shiftIterator(-1);
            return *this;
        }

    _Self  operator--(int)
        {
            _Self tmp(*this);
            shiftIterator(-1);
            return tmp;
        }

    _Self
    operator+(difference_type n) const
        {
            _Self tmp(*this);
            tmp.shiftIterator(n);
            return tmp;
        }
    
    _Self&
    operator+=(difference_type n)
        {
            shiftIterator(n);
            return *this;
        }

    _Self
    operator-(difference_type n) const
        {
            _Self tmp(*this);
            tmp.shiftIterator( ForceNegative(n) );
            return tmp;
        }

    _Self&
    operator-=(difference_type n)
        {
            shiftIterator( ForceNegative(n) );
            return *this;
        }

    
public: // dont use anything from here on
    
    DelegateIterClass m_delegate;

    void shiftIterator( int n )
        {
            advance( m_delegate, n );
        }
};

/**
 * iterate over the range of a map
 */
template < class DelegateIterClass  >
class map_range_iterator_class
{
    typedef map_range_iterator_class< DelegateIterClass > _Self;
    
public:

    typedef typename std::iterator_traits<DelegateIterClass> traits_type;
    typedef typename traits_type::iterator_category       iterator_category;
    typedef typename traits_type::value_type::second_type value_type;
    typedef long                                          difference_type;
    typedef value_type                                    pointer;
    typedef value_type                                    reference;
    
    
    map_range_iterator_class( DelegateIterClass de )
        :
        m_delegate( de )
        {
        }

    reference operator*() const
        {
            return m_delegate->second;
        }
    
    pointer operator->()
        {
            return m_delegate->second;
        }

    _Self& operator++()
        {
            shiftIterator(1);
            return *this;
        }
    
    _Self  operator++(int)
        {
            _Self tmp(*this);
            shiftIterator(1);
            return tmp;
        }
    
    _Self& operator--()
        {
            shiftIterator(-1);
            return *this;
        }

    _Self  operator--(int)
        {
            _Self tmp(*this);
            shiftIterator(-1);
            return tmp;
        }

    _Self
    operator+(difference_type n) const
        {
            _Self tmp(*this);
            tmp.shiftIterator(n);
            return tmp;
        }
    
    _Self&
    operator+=(difference_type n)
        {
            shiftIterator(n);
            return *this;
        }

    _Self
    operator-(difference_type n) const
        {
            _Self tmp(*this);
            tmp.shiftIterator( ForceNegative(n) );
            return tmp;
        }

    _Self&
    operator-=(difference_type n)
        {
            shiftIterator( ForceNegative(n) );
            return *this;
        }
    
public: // dont use anything from here on
    
    DelegateIterClass m_delegate;

    void shiftIterator( int n )
        {
            advance( m_delegate, n );
        }
};


template< class DelegateIterClass >
bool operator==(const map_domain_iterator_class< DelegateIterClass >& x,
                const map_domain_iterator_class< DelegateIterClass >& y)
{
    return x.m_delegate == y.m_delegate;
}

template< class DelegateIterClass >
bool operator<( const map_domain_iterator_class< DelegateIterClass >& x,
                const map_domain_iterator_class< DelegateIterClass >& y)
{
    return x.m_delegate < y.m_delegate;
}

template< class DelegateIterClass >
bool operator>(const map_domain_iterator_class< DelegateIterClass >& x,
               const map_domain_iterator_class< DelegateIterClass >& y) {
    return y < x;
}

template< class DelegateIterClass >
bool operator!=(const map_domain_iterator_class< DelegateIterClass >& x,
                const map_domain_iterator_class< DelegateIterClass >& y) {
    return !(x == y);
}
    
template< class DelegateIterClass >
bool operator<=(const map_domain_iterator_class< DelegateIterClass >& x,
                const map_domain_iterator_class< DelegateIterClass >& y) {
    return !(y < x);
}

template< class DelegateIterClass >
bool operator>=(const map_domain_iterator_class< DelegateIterClass >& x,
                const map_domain_iterator_class< DelegateIterClass >& y) {
    return !(x < y);
}


template< class DelegateIterClass >
bool operator==(const map_range_iterator_class< DelegateIterClass >& x,
                const map_range_iterator_class< DelegateIterClass >& y)
{
    return x.m_delegate == y.m_delegate;
}

template< class DelegateIterClass >
bool operator<( const map_range_iterator_class< DelegateIterClass >& x,
                const map_range_iterator_class< DelegateIterClass >& y)
{
    return x.m_delegate < y.m_delegate;
}

template< class DelegateIterClass >
bool operator>(const map_range_iterator_class< DelegateIterClass >& x,
               const map_range_iterator_class< DelegateIterClass >& y) {
    return y < x;
}

template< class DelegateIterClass >
bool operator!=(const map_range_iterator_class< DelegateIterClass >& x,
                const map_range_iterator_class< DelegateIterClass >& y) {
    return !(x == y);
}
    
template< class DelegateIterClass >
bool operator<=(const map_range_iterator_class< DelegateIterClass >& x,
                const map_range_iterator_class< DelegateIterClass >& y) {
    return !(y < x);
}

template< class DelegateIterClass >
bool operator>=(const map_range_iterator_class< DelegateIterClass >& x,
                const map_range_iterator_class< DelegateIterClass >& y) {
    return !(x < y);
}


template < class DelegateIterClass >
map_domain_iterator_class< DelegateIterClass >
map_domain_iterator( DelegateIterClass iter )
{
    return map_domain_iterator_class< DelegateIterClass >( iter );
}

template < class DelegateIterClass >
map_range_iterator_class< DelegateIterClass >
map_range_iterator( DelegateIterClass iter )
{
    return map_range_iterator_class< DelegateIterClass >( iter );
}


 
};

#endif

