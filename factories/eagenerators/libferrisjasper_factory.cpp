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

    $Id: libferrisjasper_factory.cpp,v 1.3 2010/09/24 21:31:28 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>

using namespace std;
namespace Ferris
{
    /**
     *
     * New method that allows the factory itself to be statically bound
     * to libferris but the plugin to be dynamically loaded. (1.1.10)+
     *
     * New interface that uses a unified image cache (1.1.40)+
     */
    namespace 
    {
        const char* ext       = "jp2";
        bool        writable  = false;
        string      libname   = AUTOTOOLS_CONFIG_LIBDIR + "/ferris/plugins/eagenerators/libferrisjasper.so";
        const char* shortname = "jasper";
        
        bool r = Context::RegisterImageEAGeneratorModule(
            ext, writable, libname, shortname );

        bool r2 = Context::RegisterImageEAGeneratorModule(
            "mif", writable, libname, shortname );
        bool r3 = Context::RegisterImageEAGeneratorModule(
            "jpc", writable, libname, shortname );
        bool r4 = Context::RegisterImageEAGeneratorModule(
            "pgx", writable, libname, shortname );
        bool r5 = Context::RegisterImageEAGeneratorModule(
            "ras", writable, libname, shortname );
        bool r6 = Context::RegisterImageEAGeneratorModule(
            "j2k", writable, libname, shortname );
    };
};
