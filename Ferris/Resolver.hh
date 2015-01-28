/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of libferris.

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

    $Id: Resolver.hh,v 1.5 2010/09/24 21:30:57 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <string>

#ifndef _ALREADY_INCLUDED_FERRIS_RESOLVER_H_
#define _ALREADY_INCLUDED_FERRIS_RESOLVER_H_


namespace Ferris
{
#ifdef BUILDING_LIBFERRIS
    #pragma GCC visibility push(default)
#endif
    
    enum ResolveStyle {
        RESOLVE_CLOSEST =  (1<<1), //< Closest match to the context sought
        RESOLVE_PARENT  =  (1<<2), //< Parent of the context sought
        RESOLVE_EXACT   =  (1<<3),  //< Exactly match the required context
        ENABLE_ATTRIBUTE_RESOLUTION = (1<<4) //< Allow path/to/file@attr to select an EA
    };
    
    fh_context Resolve( const std::string earl, ResolveStyle rs = RESOLVE_EXACT );

    enum {
        RESOLVEEX_UNROLL_LINKS = (1<<1)  //< follow links and return non-link result
    };
    
    /**
     * Extended Resolve() method added in ferris 1.1.7 to allow inlining of some other
     * calls that would normally happen after a Resolve().
     *
     * like the other Resolve() method but also processes the RESOLVEEX param in ex.
     * if RESOLVEEX_UNROLL_LINKS is set then soft/hard links
     * are unrolled so that the context returned is not a link.
     *
     * This method should allow other flags to be added without breaking ABI.
     */
    fh_context Resolve( const std::string earl, ResolveStyle rs, int ex );

//     namespace Factory
//     {
//         enum ConfigLocation
//         {
//             CONFIGLOC_EVENTBIND = (1<<1),
//             CONFIGLOC_MIMEBIND  = (1<<2),
//             CONFIGLOC_APPS      = (1<<3),
//             CONFIGLOC_ICONS     = (1<<4),
//             CONFIGLOC_ENUMSIZE  = (1<<5)
//         };
//         fh_context Resolve( ConfigLocation cl, std::string extrapath );
//         fh_context ResolveMime( std::string majort, std::string minort );
//         fh_context ResolveIcon( std::string s );
//     };

    /**
     * Shells such as bash do not natively understand all URLs that libferris
     * does, so this method allows libferris to expand something like
     * pg://localhost/mydb/foo/file*
     * into a list of matching URLs similar to the work that bash does
     * for /tmp/foo* 
     */
    stringlist_t& expandShellGlobs( stringlist_t& ret, const std::string& s );

#ifdef BUILDING_LIBFERRIS
    #pragma GCC visibility pop
#endif
};

#include <popt.h>
namespace Ferris
{
    FERRISEXP_API struct ::poptOption* getExpandShellGlobsPopTable();
    FERRISEXP_API stringlist_t& expandShellGlobs( stringlist_t& ret, poptContext& optCon );
};
#define FERRIS_SHELL_GLOB_POPT_OPTIONS                             \
{ 0, 0, POPT_ARG_INCLUDE_TABLE,::Ferris::getExpandShellGlobsPopTable(),   \
  0, "Ferris shell glob options:", 0 },


#endif
