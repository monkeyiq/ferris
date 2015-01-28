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

    $Id: RPM_private.hh,v 1.2 2010/09/24 21:30:57 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_RPM_PRIVATE_H_
#define _ALREADY_INCLUDED_FERRIS_RPM_PRIVATE_H_

#include <config.h>
#include <Ferris/HiddenSymbolSupport.hh>

#ifdef HAVE_LIBRPM
#include <rpm/rpmdb.h>
#include <rpm/rpmfi.h>
#include <rpm/rpmcli.h>
#include <rpm/rpmerr.h>

namespace Ferris
{
    /**
     * get an open reference to the rpm database. calls to get_rpmdb()
     * are counted internally and must be matched with calls to release_rpmdb();
     * For easy use one should create a rpmdb_releaser object on the stack which
     * will take care of releasing the reference for you.
     */
    FERRISEXP_API rpmdb get_rpmdb();

    /**
     * release a reference to an open rpmdb obtained via get_rpmdb();
     * Please use a rpmdb_releaser object on the stack instead of explicit call to
     * this function.
     */
    FERRISEXP_API void release_rpmdb( rpmdb );

    /**
     * used to release a rpmdb by assignment is initialization
     */
    struct FERRISEXP_API rpmdb_releaser 
    {
        rpmdb d;
        rpmdb_releaser( rpmdb d ) : d(d) 
            {}
        ~rpmdb_releaser()
            {
                release_rpmdb( d );
            }
    };
    
};
#endif
#endif
