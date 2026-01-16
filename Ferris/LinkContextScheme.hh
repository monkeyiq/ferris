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

    $Id: LinkContextScheme.hh,v 1.3 2010/09/24 21:30:53 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_LINK_CONTEXT_SCHEME_H_
#define _ALREADY_INCLUDED_LINK_CONTEXT_SCHEME_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Resolver.hh>
#include <Ferris/Resolver_private.hh>

namespace Ferris
{

    template <const std::string& name, const std::string& earl >
    class LinkContextSchemeVFS_RootContextDropper
        :
        public RootContextDropper
    {
    public:
        
        LinkContextSchemeVFS_RootContextDropper()
            {
                ImplementationDetail::appendToStaticLinkedRootContextNames( name );
                RootContextFactory::Register( name, this );
            }

        fh_context Brew( RootContextFactory* rf )
            {
//                 cerr << "LinkContextSchemeVFS_RootContextDropper<1> name:" << name
//                      << " url:" << earl << endl;
//                 DEBUG_dumpcl( "LinkContextSchemeVFS_RootContextDropper<1>" );

                try
                {
                    static fh_context c = Resolve( earl );
                    return c;
                }
                catch( std::exception& e )
                {
                    fh_stringstream ss;
                    ss << "ERROR: fundamental access scheme can not resolve." << std::endl
                       << "looking for earl:" << earl << std::endl
                       << " nested:" << e.what() << std::endl
                       << "Your libferris is not installed correctly." << std::endl;
                    std::cerr << tostr(ss) << std::endl;
                    Throw_RootContextCreationFailed( tostr(ss), 0 );
                }
            }
    };
 
};


#endif
