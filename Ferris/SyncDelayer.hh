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

    $Id: SyncDelayer.hh,v 1.4 2010/09/24 21:31:00 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_SYNC_DELAYER_H_
#define _ALREADY_INCLUDED_FERRIS_SYNC_DELAYER_H_

#include <Functor.h>
#include <map>

namespace Ferris
{
    class SyncDelayer
    {
    public:
        explicit SyncDelayer();
        ~SyncDelayer();


        typedef Loki::Functor< void, LOKI_TYPELIST_1( SyncDelayer* ) > f_functor;
        static bool exists();
        static void add( void* key, const f_functor& f );
        static void ensure( void* key, const f_functor& f );
    private:
        static int s_count;
        typedef std::map< void*, f_functor > m_flist_t;
        static m_flist_t m_flist;
    };
};

#endif
