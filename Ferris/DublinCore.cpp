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

    $Id: DublinCore.cpp,v 1.2 2010/09/24 21:30:29 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/DublinCore.hh>

namespace Ferris
{
    const stringlist_t& getUnqualifiedDublinCoreAttributeNames()
    {
        static stringlist_t ret;
        if( ret.empty() )
        {
            ret.push_back("title");
            ret.push_back("publisher");
            ret.push_back("creator");
            ret.push_back("description");
            ret.push_back("language");
            ret.push_back("subject");
            ret.push_back("contributor");
            ret.push_back("date");
            ret.push_back("type");
            ret.push_back("format");
            ret.push_back("identifier");
            ret.push_back("source");
            ret.push_back("relation");
            ret.push_back("coverage");
            ret.push_back("rights");
        }
        return ret;
    }
};


