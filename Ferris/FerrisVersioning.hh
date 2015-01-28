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

    $Id: FerrisVersioning.hh,v 1.2 2010/09/24 21:30:43 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_VERSIONING_H_
#define _ALREADY_INCLUDED_FERRIS_VERSIONING_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <glib.h>

namespace Ferris
{
    typedef guint32 ContextDirOpVersion_t;

    template < class VersionType >
    class VersionWatcher
    {
        VersionType ourVersion;
    public:
        VersionWatcher( VersionType v = 0 )
            :
            ourVersion( v )
            {
            }

        VersionWatcher( const VersionWatcher<VersionType>& v )
            {
                ourVersion = v.ourVersion;
            }

        VersionWatcher<VersionType>& operator=( const VersionWatcher<VersionType>& v )
            {
                ourVersion = v.ourVersion;
                return *this;
            }
        

        /** 
         * returns true if we have the same version still
         */
        bool test( VersionType theirV )
            {
                return ourVersion == theirV;
            }

        void update( VersionType theirV )
            {
                ourVersion = theirV;
            }

        /** 
         * returns true if we have the same version still
         */
        bool operator()( VersionType theirV )
            {
                bool r = test(theirV);
                ourVersion = theirV;
                return r;
            }
    };
};




#endif

