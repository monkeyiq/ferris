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

    $Id: UAsyncIO.hh,v 1.2 2010/09/24 21:31:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_UASYNCIO_H_
#define _ALREADY_INCLUDED_FERRIS_UASYNCIO_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <Ferris/Ferris.hh>
#include <Ferris/Runner.hh>
#include <Ferris/AsyncIO.hh>

namespace Ferris
{
    

    template <class StreamClass = fh_stringstream >
    class FERRISEXP_API GTK_StreamCollector
        :
        public StreamCollector< StreamClass >
    {
        typedef GTK_StreamCollector< StreamClass > _Self;
        typedef StreamCollector< StreamClass >     _Base;
        
        GtkProgressBar* w_progress;

    public:

        GTK_StreamCollector( GtkProgressBar* w, StreamClass oss = StreamClass() )
            :
            _Base( oss ),
            w_progress( w )
            {}
        
        virtual fh_istream io_cb( fh_istream iss )
            {
                iss = _Base::io_cb( iss );
            
                if( this->m_totalsz > 0 )
                {
                    double d = this->m_donesz;
                    d /= this->m_totalsz;
                    gtk_progress_set_percentage( GTK_PROGRESS(w_progress), d );
                }

                if( this->w_progress )
                {
                    fh_stringstream ss;
                    ss << this->m_donesz << " / " << this->m_totalsz;
                    gtk_progress_bar_set_text( GTK_PROGRESS_BAR(w_progress), tostr(ss).c_str() );
                }

                return iss;
            }
    };

    typedef GTK_StreamCollector< fh_stringstream > GTK_StringStreamCollector;
    FERRIS_SMARTPTR( GTK_StringStreamCollector, fh_gtk_sstreamcol );

    typedef GTK_StreamCollector< fh_fstream > GTK_FileStreamCollector;
    FERRIS_SMARTPTR( GTK_FileStreamCollector, fh_gtk_fstreamcol );

    namespace Factory
    {
        FERRISEXP_API  fh_sstreamcol MakeGTKStringStreamCol( GtkProgressBar* w );
        FERRISEXP_API  fh_fstreamcol MakeGTKFileStreamCol(
            GtkProgressBar* w,
            const std::string& s,
            std::ios_base::openmode m = std::ios_base::out );
    };
    
    
};
#endif

