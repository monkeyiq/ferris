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

    $Id: Numeric.hh,v 1.2 2010/09/24 21:30:55 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_NUMERIC_H_
#define _ALREADY_INCLUDED_FERRIS_NUMERIC_H_

namespace Ferris
{
    namespace Numeric
    {
        /**
         * Make sure that mi <= v <= mx
         */
        template < class ValueT, class MinT, class MaxT >
        ValueT clamp( ValueT v, MinT mi, MaxT mx )
        {
            if( v < mi )
                return mi;
            if( v > mx )
                return mx;
            return v;
        }
        
    };
};
#endif

