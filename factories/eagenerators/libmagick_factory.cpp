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

    $Id: libmagick_factory.cpp,v 1.4 2010/09/24 21:31:30 ben Exp $

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
        bool        writable  = true;
        string      libname   = AUTOTOOLS_CONFIG_LIBDIR + "/ferris/plugins/eagenerators/libmagick.so";
        const char* shortname = "magick";
        
        bool r = Context::RegisterImageEAGeneratorModule(
            ext, writable, libname, shortname );

        bool w = writable;
        string ln = libname;
        const char* sn = shortname;
        
        bool r2  = Context::RegisterImageEAGeneratorModule( "avs", w, ln, sn );
        bool r3  = Context::RegisterImageEAGeneratorModule( "avs", w, ln, sn );
        bool r4  = Context::RegisterImageEAGeneratorModule( "gif", w, ln, sn );
        bool r5  = Context::RegisterImageEAGeneratorModule( "bmp", w, ln, sn );
        bool r6  = Context::RegisterImageEAGeneratorModule( "cgm", w, ln, sn );
        bool r7  = Context::RegisterImageEAGeneratorModule( "dcm", w, ln, sn );
        bool r8  = Context::RegisterImageEAGeneratorModule( "dcx", w, ln, sn );
        bool r9  = Context::RegisterImageEAGeneratorModule( "dib", w, ln, sn );
        bool r10 = Context::RegisterImageEAGeneratorModule( "dpx", w, ln, sn );
//        bool r11 = Context::RegisterImageEAGeneratorModule( "epdf", w, ln, sn );
        bool r12 = Context::RegisterImageEAGeneratorModule( "epi", w, ln, sn );
        bool r13 = Context::RegisterImageEAGeneratorModule( "eps", w, ln, sn );
        bool r14 = Context::RegisterImageEAGeneratorModule( "eps2", w, ln, sn );
        bool r15 = Context::RegisterImageEAGeneratorModule( "epsf", w, ln, sn );
        bool r16 = Context::RegisterImageEAGeneratorModule( "epsi", w, ln, sn );
        bool r17 = Context::RegisterImageEAGeneratorModule( "ept", w, ln, sn );
        bool r18 = Context::RegisterImageEAGeneratorModule( "fax", w, ln, sn );
        bool r19 = Context::RegisterImageEAGeneratorModule( "fig", w, ln, sn );
        bool r20 = Context::RegisterImageEAGeneratorModule( "fits", w, ln, sn );
        bool r21 = Context::RegisterImageEAGeneratorModule( "fpx", w, ln, sn );
        bool r22 = Context::RegisterImageEAGeneratorModule( "gplt", w, ln, sn );
//        bool r23 = Context::RegisterImageEAGeneratorModule( "html", w, ln, sn );
        bool r24 = Context::RegisterImageEAGeneratorModule( "ico", w, ln, sn );
        bool r25 = Context::RegisterImageEAGeneratorModule( "icc", w, ln, sn );
        bool r26 = Context::RegisterImageEAGeneratorModule( "ilbm", w, ln, sn );
        bool r27 = Context::RegisterImageEAGeneratorModule( "iff", w, ln, sn );
        bool r28 = Context::RegisterImageEAGeneratorModule( "itpc", w, ln, sn );
        bool r29 = Context::RegisterImageEAGeneratorModule( "jbig", w, ln, sn );
        bool r30 = Context::RegisterImageEAGeneratorModule( "man", w, ln, sn );
        bool r31 = Context::RegisterImageEAGeneratorModule( "miff", w, ln, sn );
        bool r32 = Context::RegisterImageEAGeneratorModule( "mng", w, ln, sn );
        bool r33 = Context::RegisterImageEAGeneratorModule( "m2v", w, ln, sn );
        bool r34 = Context::RegisterImageEAGeneratorModule( "mpc", w, ln, sn );
        bool r35 = Context::RegisterImageEAGeneratorModule( "mtv", w, ln, sn );
        bool r36 = Context::RegisterImageEAGeneratorModule( "mvg", w, ln, sn );
        bool r37 = Context::RegisterImageEAGeneratorModule( "netscape", w, ln, sn );
        bool r38 = Context::RegisterImageEAGeneratorModule( "pbm", w, ln, sn );
        bool r39 = Context::RegisterImageEAGeneratorModule( "pcd", w, ln, sn );
        bool r40 = Context::RegisterImageEAGeneratorModule( "pcds", w, ln, sn );
        bool r41 = Context::RegisterImageEAGeneratorModule( "pcx", w, ln, sn );
        bool r42 = Context::RegisterImageEAGeneratorModule( "pdb", w, ln, sn );
//        bool r43 = Context::RegisterImageEAGeneratorModule( "pdf", w, ln, sn );
        bool r44 = Context::RegisterImageEAGeneratorModule( "pgm", w, ln, sn );
        bool r45 = Context::RegisterImageEAGeneratorModule( "pict", w, ln, sn );
        bool r46 = Context::RegisterImageEAGeneratorModule( "pix", w, ln, sn );
        bool r47 = Context::RegisterImageEAGeneratorModule( "pnm", w, ln, sn );
        bool r48 = Context::RegisterImageEAGeneratorModule( "ppm", w, ln, sn );
//         bool r49 = Context::RegisterImageEAGeneratorModule( "ps", w, ln, sn );
//         bool r50 = Context::RegisterImageEAGeneratorModule( "ps2", w, ln, sn );
//         bool r51 = Context::RegisterImageEAGeneratorModule( "ps3", w, ln, sn );
        bool r52 = Context::RegisterImageEAGeneratorModule( "ptif", w, ln, sn );
        bool r53 = Context::RegisterImageEAGeneratorModule( "pwp", w, ln, sn );
        bool r54 = Context::RegisterImageEAGeneratorModule( "p7", w, ln, sn );
        bool r55 = Context::RegisterImageEAGeneratorModule( "rad", w, ln, sn );
        bool r56 = Context::RegisterImageEAGeneratorModule( "rla", w, ln, sn );
        bool r57 = Context::RegisterImageEAGeneratorModule( "rle", w, ln, sn );
        bool r58 = Context::RegisterImageEAGeneratorModule( "sct", w, ln, sn );
        bool r59 = Context::RegisterImageEAGeneratorModule( "sfw", w, ln, sn );
        bool r60 = Context::RegisterImageEAGeneratorModule( "sgi", w, ln, sn );
        bool r61 = Context::RegisterImageEAGeneratorModule( "stegano", w, ln, sn );
        bool r62 = Context::RegisterImageEAGeneratorModule( "sun", w, ln, sn );
//        bool r63 = Context::RegisterImageEAGeneratorModule( "svg", w, ln, sn );
        bool r64 = Context::RegisterImageEAGeneratorModule( "tga", w, ln, sn );
        bool r65 = Context::RegisterImageEAGeneratorModule( "tiff", w, ln, sn );
        bool r66 = Context::RegisterImageEAGeneratorModule( "tile", w, ln, sn );
        bool r67 = Context::RegisterImageEAGeneratorModule( "tim", w, ln, sn );
        bool r68 = Context::RegisterImageEAGeneratorModule( "ttf", w, ln, sn );
        bool r69 = Context::RegisterImageEAGeneratorModule( "uil", w, ln, sn );
        bool r70 = Context::RegisterImageEAGeneratorModule( "vicar", w, ln, sn );
        bool r71 = Context::RegisterImageEAGeneratorModule( "vid", w, ln, sn );
        bool r72 = Context::RegisterImageEAGeneratorModule( "viff", w, ln, sn );
        bool r73 = Context::RegisterImageEAGeneratorModule( "wbmp", w, ln, sn );
        bool r74 = Context::RegisterImageEAGeneratorModule( "wpg", w, ln, sn );
        bool r75 = Context::RegisterImageEAGeneratorModule( "wmf", w, ln, sn );
        bool r76 = Context::RegisterImageEAGeneratorModule( "xc", w, ln, sn );
        bool r77 = Context::RegisterImageEAGeneratorModule( "xbm", w, ln, sn );
//        bool r78 = Context::RegisterImageEAGeneratorModule( "xpm", w, ln, sn );
        bool r79 = Context::RegisterImageEAGeneratorModule( "xwd", w, ln, sn );
    };
};
