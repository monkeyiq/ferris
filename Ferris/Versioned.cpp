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

    $Id: Versioned.cpp,v 1.2 2010/09/24 21:31:01 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <config.h>
#include <Versioned.hh>

namespace Ferris
{
    /**
     * Default ctor, version numbering starts at MINIMUM_VERSION
     *
     */
    Versioned::Versioned()
        :
        Version( MINIMUM_VERSION )
    {
    }
    
    /**
     * Get the current version of this versioned
     *
     * @return The current version
     */
    Versioned::Version_t
    Versioned::getVersion() const
    {
        return Version;
    }


    /**
     * Increment the current version by one unit.
     */
    void
    Versioned::bumpVersion()
    {
        Version++;
    }


    /**
     * Set the version to a given number. This method should be
     * used with great caution because classes that expect a
     * Versioned usually expect the version to steadily increase
     * and never backtrack.
     *
     * @param v The version to set to
     */
    void
    Versioned::setVersion(Version_t v)
    {
        Version = v;
    }
};

