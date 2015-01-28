/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2002 Ben Martin

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

    $Id: libastextman_factory.cpp,v 1.2 2010/09/24 21:31:24 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <Ferris.hh>
#include <FerrisAsTextPlugin.hh>

using namespace std;

namespace Ferris
{
    static bool r1 = RegisterAsTextFromMime( "text/x-troff-man", "libastextman.so" );

    static const char* endings_compressed[] = {
        "name", ".1.gz",
        "name", ".1p.gz",
        "name", ".2.gz",
        "name", ".3.gz",
        "name", ".3p.gz",
        "name", ".4.gz",
        "name", ".5.gz",
        "name", ".6.gz",
        "name", ".7.gz",
        "name", ".8.gz",
        "name", ".9.gz",

        "name", ".1.bz2",
        "name", ".1p.bz2",
        "name", ".2.bz2",
        "name", ".3.bz2",
        "name", ".3p.bz2",
        "name", ".4.bz2",
        "name", ".5.bz2",
        "name", ".6.bz2",
        "name", ".7.bz2",
        "name", ".8.bz2",
        "name", ".9.bz2",
        0 
    };
    static const char* endings[] = {
        "name", ".1",
        "name", ".1p",
        "name", ".2",
        "name", ".3",
        "name", ".3p",
        "name", ".4",
        "name", ".5",
        "name", ".6",
        "name", ".7",
        "name", ".8",
        "name", ".9",
        0 
    };
    
//     static bool r2 = RegisterAsTextFromMatcher(
//         Factory::ComposeEndsWithMatcher( endings ),
//         "libastextman.so" );

    static bool r2 = RegisterAsTextFromMatcher(
        Factory::MakeOrMatcher(
            Factory::ComposeEndsWithMatcher( endings_compressed ),
            Factory::MakeAndMatcher(
                Factory::MakeRegexMatcher( "path", ".*/man/.*" ),
                Factory::ComposeEndsWithMatcher( endings ))),
        "libastextman.so" );
};
