/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2003 Ben Martin

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

    $Id: libferrisxmp_factory.cpp,v 1.3 2010/09/24 21:31:29 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <FerrisEAPlugin.hh>

using namespace std;

namespace Ferris
{
    static fh_matcher getMatcher()
    {
        static fh_matcher m;
        static Factory::EndingList r;
        static bool virgin = true;

        if( virgin )
        {
            virgin = false;

            /*
                cache_mtime >= mtime && strlen( index-offsets )
                || !(cache_mtime >= mtime) && name = pdf
            */            
            

            fh_matcher hasXMP
                = Factory::MakeHasOneOrMoreBytesMatcher(
                    "http://witme.sf.net/libferris-core/xmp-0.1/index-offsets", false );
            fh_matcher isCurrent = Factory::MakeEAValueGreaterEqMTime(
                "http://witme.sf.net/libferris-core/xmp-0.1/index-mtime" );

            r.push_back( make_pair( string("name"), string(".pdf") ));
            r.push_back( make_pair( string("name"), string(".psd") ));

//            m = Factory::MakeAndMatcher( hasXMP, isCurrent );
//             m = Factory::MakeAndMatcher(
//                 Factory::MakeNotMatcher( isCurrent ),
//                 Factory::ComposeEndsWithMatcher( r ) );
            
            
            fh_matcher leftm = Factory::MakeAndMatcher(
                Factory::ComposeEndsWithMatcher( r ),
                Factory::MakeNotMatcher( isCurrent ) );
            fh_matcher rightm = Factory::MakeAndMatcher( hasXMP, isCurrent );
            m = Factory::MakeOrMatcher( leftm, rightm );

        }
    
        return m;
    }
    
    /**
     *
     * New method that allows the factory itself to be statically bound
     * to libferris but the plugin to be dynamically loaded. (1.1.10)+
     *
     */
    namespace 
    {
        static bool r = RegisterEAGeneratorModule(
            getMatcher(),
            AUTOTOOLS_CONFIG_LIBDIR + "/ferris/plugins/eagenerators/libferrisxmp.so",
            "xmp",
            false,
            true,
            AttributeCreator::CREATE_PRI_MED );
    };
};
