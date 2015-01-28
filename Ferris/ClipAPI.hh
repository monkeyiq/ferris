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

    $Id: ClipAPI.hh,v 1.2 2010/09/24 21:30:26 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_CLIP_API_H_
#define _ALREADY_INCLUDED_FERRIS_CLIP_API_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/CursorAPI.hh>

namespace Ferris
{
    namespace Factory
    {
        FERRISEXP_API fh_context getFileClipboard( std::string s = "" );
    };
    namespace FileClip
    {
        FERRISEXP_API void Clear( fh_context clip );
        FERRISEXP_API void Cut(   fh_context clip, fh_context c );
        FERRISEXP_API void Copy(  fh_context clip, fh_context c );
        FERRISEXP_API void Link(  fh_context clip, fh_context c );
        FERRISEXP_API void Paste( fh_context clip, fh_context dst );

        FERRISEXP_API void Undo( fh_context cursor );
        FERRISEXP_API void Redo( fh_context cursor );

        FERRISEXP_API stringlist_t getMimeHistory( fh_context clip, fh_context c,
                                                   std::string action = "" );

        /*
         * Setting options regarding the paste command that is executed.
         */
        FERRISEXP_API void setUseSloth( fh_context clip,     bool v );
        FERRISEXP_API void setUseAutoClose( fh_context clip, bool v );
    };
};

#endif
