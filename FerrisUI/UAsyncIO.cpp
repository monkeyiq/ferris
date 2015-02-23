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

    $Id: UAsyncIO.cpp,v 1.2 2010/09/24 21:31:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "GtkFerris.hh"

using namespace std;

namespace FerrisUI
{
    namespace Factory
    {
        fh_sstreamcol MakeGTKStringStreamCol( GtkProgressBar* w )
        {
            return new GTK_StringStreamCollector( w );
        }
        
        fh_fstreamcol MakeGTKFileStreamCol( GtkProgressBar* w,
                                            const std::string& s,
                                            std::ios_base::openmode m )
        {
            fh_fstream ss( s, m );
            return new GTK_FileStreamCollector( w, ss );
        }
        
    };
    
};
