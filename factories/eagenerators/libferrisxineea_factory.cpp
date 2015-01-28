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

    $Id: libferrisxineea_factory.cpp,v 1.6 2010/11/17 21:30:49 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include "config.h"
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

#ifdef HAVE_MEDIAINFO

                r.push_back( make_pair( string("name"), string(".disabled-xine-plugin") ));
                
#else
                r.push_back( make_pair( string("name"), string(".avi") ));
                r.push_back( make_pair( string("name"), string(".AVI") ));
                r.push_back( make_pair( string("name"), string(".mkv") ));
                r.push_back( make_pair( string("name"), string(".MKV") ));
                r.push_back( make_pair( string("name"), string(".mpg") ));
                r.push_back( make_pair( string("name"), string(".mp4") ));
                r.push_back( make_pair( string("name"), string(".ogm") ));
#endif
                
                m = Factory::ComposeEndsWithMatcher( r );
            }
    
            return m;
        }

        static bool r1 = RegisterEAGeneratorModule(
            getMatcher(),
            AUTOTOOLS_CONFIG_LIBDIR + "/ferris/plugins/eagenerators/libferrisxineea.so",
            "xine_metadata",
            false, // dyn
            true,  // state
            AttributeCreator::CREATE_PRI_MED );


        bool        writable  = false;
        string      libname   = AUTOTOOLS_CONFIG_LIBDIR + "/ferris/plugins/eagenerators/libferrisxineea.so";
        const char* shortname = "xine";

#ifndef HAVE_MEDIAINFO
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
#endif
        
    };
};
