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

    $Id: FerrisKDE.hh,v 1.3 2010/09/24 21:30:39 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_KDE3_H_
#define _ALREADY_INCLUDED_FERRIS_KDE3_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>

namespace Ferris
{
    namespace KDE
    {
        FERRISEXP_API void ensureKDEApplication();
        FERRISEXP_API std::string getMimeType( fh_context c, bool fromContent = false );
        FERRISEXP_API std::string guessMimeType( const std::string& path );
    };
};

#endif
