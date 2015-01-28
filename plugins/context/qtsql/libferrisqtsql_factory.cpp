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

    $Id: libferrispostgresql_factory.cpp,v 1.1 2005/07/12 04:04:30 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>

using namespace std;
using Ferris::Factory::MakeAndMatcher;
using Ferris::Factory::MakeEqualMatcher;
using Ferris::Factory::MakeEndsWithMatcher;

namespace Ferris
{
    extern "C"
    {
//         FERRISEXP_EXPORT fh_ommatchers GetOverMountMatchers(RootContextFactory* rf)
//         {
//             fh_ommatchers ret;
//             fh_matcher m = Factory::MakeAlwaysFalseMatcher();
//             ret.push_back( m );
//             return ret;
//         }

        FERRISEXP_EXPORT fh_ommatchers GetOverMountMatchers(RootContextFactory* rf)
        {
            fh_ommatchers ret;
            ret.push_back( Factory::MakeEndsWithMatcher("name","sqlite") );
            ret.push_back( Factory::MakeEqualMatcher   ("ferris-type","sqlite") );

            ret.push_back(
                MakeAndMatcher( Factory::MakeEqualMatcher   ("name-extension","db"),
                                Factory::MakeStartsWithMatcher("mimetype", "application/x-sqlite" )));
            return ret;
        }
        

        FERRISEXP_EXPORT string getName()
        {
            // This is a bit of a hack, "_" sorts before "db" so we will get a chance
            // to mount a foo.db file before stldb4 does
            return "_qtsql";
        }

        FERRISEXP_EXPORT bool isTopLevel()
        {
            return true;
        }
    }
 
};
