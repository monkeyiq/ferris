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

    $Id: libferrismediainfoea_factory.cpp,v 1.4 2010/09/24 21:31:29 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <FerrisEAPlugin.hh>

using namespace std;

namespace Ferris
{
    /**
     */
    namespace 
    {
        static fh_matcher getMatcher()
        {
            static fh_matcher m;
            static Factory::EndingList r;
            static bool virgin = true;

            if( virgin )
            {
                virgin = false;

                r.push_back( make_pair( string("name"), string(".avi") ));
                r.push_back( make_pair( string("name"), string(".AVI") ));
                r.push_back( make_pair( string("name"), string(".mkv") ));
                r.push_back( make_pair( string("name"), string(".MKV") ));
                r.push_back( make_pair( string("name"), string(".mpg") ));
                r.push_back( make_pair( string("name"), string(".mp4") ));
                r.push_back( make_pair( string("name"), string(".ogm") ));
                r.push_back( make_pair( string("name"), string(".m2ts") ));
                r.push_back( make_pair( string("name"), string(".M2TS") ));

                m = Factory::ComposeEndsWithMatcher( r );
            }
    
            return m;
        }

        static bool r1 = RegisterEAGeneratorModule(
            getMatcher(),
            AUTOTOOLS_CONFIG_LIBDIR + "/ferris/plugins/eagenerators/libferrismediainfo.so",
            "mediainfo_metadata",
            false, // dyn
            true,  // state
            AttributeCreator::CREATE_PRI_HIGH );


        bool        writable  = false;
        string      libname   = AUTOTOOLS_CONFIG_LIBDIR + "/ferris/plugins/eagenerators/libferrismediainfo.so";
        const char* shortname = "mediainfo";
        bool ri1 = Context::RegisterImageEAGeneratorModule(
            "avi", writable, libname, shortname );
        bool ri2 = Context::RegisterImageEAGeneratorModule(
            "mkv", writable, libname, shortname );
        bool ri3 = Context::RegisterImageEAGeneratorModule(
            "mpg", writable, libname, shortname );
        bool ri4 = Context::RegisterImageEAGeneratorModule(
            "ogg", writable, libname, shortname );
        bool ri5 = Context::RegisterImageEAGeneratorModule(
            "mp4", writable, libname, shortname );
        bool ri6 = Context::RegisterImageEAGeneratorModule(
            "ogm", writable, libname, shortname );
        
    };
};
