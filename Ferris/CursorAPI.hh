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

    $Id: CursorAPI.hh,v 1.2 2010/09/24 21:30:28 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_CURSOR_API_H_
#define _ALREADY_INCLUDED_FERRIS_CURSOR_API_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Iterator.hh>

namespace Ferris
{
    namespace Config
    {
        /**
         * Values are 0 or 1. If 1 and the cursor exists when moving it
         * then the file is remade.
         */
        FERRISEXP_API extern const std::string cursor_object_always_remake;

        /**
         * Set how many objects are in the circular list before wrapping
         */
        FERRISEXP_API extern const std::string cursor_object_list_size;
        
        /**
         * Use the above strings as keys and pass the values as described
         * by each
         */
        FERRISEXP_API void setCursorOption( fh_context parent,
                                            const std::string& k,
                                            const std::string& v );
    };
    namespace Factory
    {
        /**
         * Get the cursor for this directory.
         *
         * note that one can use circular_iterator from Iterator.hh to move
         * from the cursor right through the collection.
         *
         * eg.
         * fh_context parent = Resolve(...);
         * fh_context cursor = Factory::getCursor( parent );
         * Context::iterator cursor_iter = parent->find( cursor->getDirName() );
         *
         * for( circular_iterator< Context, Context::iterator >
         *       ci  = make_circular_iterator( parent, cursor_iter );
         *       ci != make_circular_iterator( parent ); ++ci )
         * {}
         *
         */
        FERRISEXP_API fh_context getCursor( fh_context parent );
    };
    namespace Cursor
    {
        FERRISEXP_API fh_context cursorNext( fh_context cursor );
        FERRISEXP_API fh_context cursorPrev( fh_context cursor );
        FERRISEXP_API fh_context cursorOffset( fh_context cursor, int offset );
        FERRISEXP_API fh_context cursorSet( fh_context cursor, fh_context newc );
    };
};

#endif
