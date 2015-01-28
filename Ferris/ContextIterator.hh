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

    $Id: ContextIterator.hh,v 1.6 2010/09/24 21:30:27 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_CONTEXT_ITERATOR_H_
#define _ALREADY_INCLUDED_FERRIS_CONTEXT_ITERATOR_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/FerrisHandle.hh>
#include <Ferris/TypeDecl.hh>

namespace Ferris
{
    
    class FERRISEXP_API ImplicitIteratorUpdateLock
    {
        static int lock;
    public:
        ImplicitIteratorUpdateLock();
        ~ImplicitIteratorUpdateLock();
        static bool isTaken();
    };
    
    
    class FERRISEXP_API ContextIterator : public Handlable
    {
#ifdef BUILDING_LIBFERRIS
    public:
#endif
        void* p_data;
    private:
#ifndef LIBFERRIS_INTERNAL_COMPILING_SWIG_WRAPPER
        friend struct ContextIteratorData* Data( const ContextIterator* ci );
#endif
        void revalidate();
        void setContext( const fh_context& );
        void shiftIterator( int n );
        
    public:

        typedef ContextIterator _Self;
        typedef Handlable       _Base;
        
//        typedef std::random_access_iterator_tag iterator_category;
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef fh_context value_type;
        typedef long       difference_type;
        typedef fh_context pointer;
        typedef fh_context reference;


        /**
         * Context creates these, c is the parent, rdn is the child
         * if rdn is "" then the iterator is assumed to be end()
         */
        explicit ContextIterator( fh_context c, const std::string& rdn );
        // for STL
        ContextIterator();
        virtual ~ContextIterator();
        ContextIterator( const ContextIterator& ci );
        ContextIterator& operator=( const ContextIterator& ci );
        
        reference operator*() const;
        pointer   operator->();
        reference operator[](difference_type n) const;

        _Self& operator++();
        _Self  operator++(int);
        _Self& operator--();
        _Self  operator--(int);

        _Self  operator+(difference_type n) const;
        _Self& operator+=(difference_type n);
        _Self  operator-(difference_type n) const;
        _Self& operator-=(difference_type n);

//        difference_type operator-(const _Self& n) const;
    };
    
    FERRISEXP_API ContextIterator::difference_type
    operator-(const ContextIterator& x, const ContextIterator& y);
    
    template < class DifferenceType >
    inline ContextIterator
    operator+( DifferenceType n, const ContextIterator& x)
    {
        return x.operator+(n);
    }

    FERRISEXP_API bool operator==(const ContextIterator& x, const ContextIterator& y);
    FERRISEXP_API bool operator<(const ContextIterator& x, const ContextIterator& y);
    FERRISEXP_API bool operator!=(const ContextIterator& x, const ContextIterator& y);
    FERRISEXP_API bool operator>(const ContextIterator& x, const ContextIterator& y);
    FERRISEXP_API bool operator<=(const ContextIterator& x, const ContextIterator& y);
    FERRISEXP_API bool operator>=(const ContextIterator& x, const ContextIterator& y);

};
#endif

