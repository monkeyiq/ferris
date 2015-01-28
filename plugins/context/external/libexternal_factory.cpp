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

    $Id: libexternal_factory.cpp,v 1.3 2010/09/24 21:31:37 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <mapping.hh>

using namespace std;
namespace Ferris
{


extern "C"
{
    
    FERRISEXP_EXPORT fh_ommatchers GetOverMountMatchers(RootContextFactory* rf)
    {
        fh_ommatchers ret;

        vector<string> v = emap::getEndsWithStrings();

        for( vector<string>::iterator iter = v.begin();
             iter != v.end(); ++iter )
        {
            ret.push_back( Factory::MakeEndsWithMatcher("name", *iter ) );
        }
        
        return ret;
    }

    FERRISEXP_EXPORT string getName()
    {
        return "external";
    }

    FERRISEXP_EXPORT bool tryToOverMountToFindEA()
    {
        return false;
    }
}

 
};
