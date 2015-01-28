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

    $Id: Mime.hh,v 1.2 2010/09/24 21:30:55 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_MIME_H_
#define _ALREADY_INCLUDED_FERRIS_MIME_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/FerrisException.hh>

namespace Ferris
{

    class FERRISEXP_EXCEPTION DesktopFileKeyNotFound : public FerrisVFSExceptionBase
    {
    public:

        inline DesktopFileKeyNotFound(
            const FerrisException_CodeState& state,
            fh_ostream log,
            const std::string& e,
            Attribute* a=0)
            :
            FerrisVFSExceptionBase( state, log, e.c_str(), a )
            {
                setExceptionName("DesktopFileKeyNotFound");
            }
    };

#define Throw_DesktopFileKeyNotFound(e,a) \
throw DesktopFileKeyNotFound( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (e), (a))

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    FERRISEXP_API void importDesktopFileTo( fh_context parentc, fh_context desktopc );
    FERRISEXP_API void importDesktopFile( fh_context c );
    
};
#endif
