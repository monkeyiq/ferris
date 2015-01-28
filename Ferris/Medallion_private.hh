/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001-2003 Ben Martin

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

    $Id: Medallion_private.hh,v 1.5 2010/09/24 21:30:54 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_MEDALLION_PRIV_H_
#define _ALREADY_INCLUDED_FERRIS_MEDALLION_PRIV_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Medallion.hh>

namespace Ferris
{
    /**
     * As some relations can be captured using emblems nicely
     * such as agent personalities, xlinks, annotation, we reserve
     * a single top level emblem "libferris" for a system emblem
     * partial ordering.
     *
     * This function will get the top level "libferris" emblem and
     * make it if its not already there.
     *
     * @param et The Etagere to use, the default is to use the default
     *           for this user.
     */
    FERRISEXP_API fh_emblem getFerrisSystemEmblem( fh_etagere et = 0 );

    FERRISEXP_API fh_emblem getDummyTreeModelEmblem();

    /**
     * This gets called quite often just to get the EMBLEM_EANAMES_ORDERING_NAME
     * emblem which is the child of getFerrisSystemEmblem()
     * Thus we cache the result for the default etagere.
     */
    FERRISEXP_API fh_emblem private_getAttributeRootEmblem( fh_etagere et = 0 );

    /**
     * If a file has this emblem, then don't try to index it.
     */
    FERRISEXP_API fh_emblem getShouldSkipIndexingEmblem();
    
};
#endif
