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

    $Id: Math.hh,v 1.2 2010/09/24 21:30:54 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/HiddenSymbolSupport.hh>
#include <math.h>

namespace Ferris
{
    namespace Math
    {

        /**
         * Perform a log base x.
         *
         * @param v value to get log of
         * @param b base of the log
         * @return logb(v)
         */
        template < class V, class BaseT >
        V log( V v, BaseT b = 2 )
        {
            return static_cast<V>(::log( static_cast<double>(v) ) / ::log( static_cast<double>(b) ));
        }

    };
};
