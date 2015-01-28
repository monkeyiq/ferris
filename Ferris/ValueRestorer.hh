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

    $Id: ValueRestorer.hh,v 1.3 2010/09/24 21:31:01 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_VALUE_RESTORER_H_
#define _ALREADY_INCLUDED_VALUE_RESTORER_H_

#include <Ferris/HiddenSymbolSupport.hh>

namespace Ferris
{
    namespace Util
    {
        /**
         * Class that stores a value into a location for the existance of an
         * object of this class. This can be handy to set a value within a
         * limited scope only, and to be sure that the old value is restored
         * when that scope is left.
         */
        template <class T>
        struct ValueRestorer
        {
            typedef T& RefType;
    
            RefType var;
            const T originalValue;
    
            explicit ValueRestorer( RefType x, const T& tmpVal )
                :
                var(x),
                originalValue( x )
                {
                    var = tmpVal;
                }
            explicit ValueRestorer( RefType x )
                :
                var(x),
                originalValue( x )
                {
                }
            ~ValueRestorer()
                {
                    var = originalValue;
                }
    
        };

        /**
         * increment the value on construction and decrement on dtor
         */
        template <class T>
        struct ValueBumpDrop
        {
            typedef T& RefType;
            RefType var;
            
            ValueBumpDrop( RefType x )
                :
                var( x )
                {
                    ++var;
                }
            ~ValueBumpDrop()
                {
                    --var;
                }
        };
        
    };
    
};

#endif
