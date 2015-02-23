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

    $Id: SyncDelayer.cpp,v 1.3 2010/09/24 21:31:00 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "SyncDelayer.hh"
#include "Enamel.hh"

#include <iostream>

using namespace std;

namespace Ferris
{
    int SyncDelayer::s_count = 0;
    SyncDelayer::m_flist_t SyncDelayer::m_flist;

    SyncDelayer::SyncDelayer()
    {
        LG_XSLTFS_D << "SyncDelayer::SyncDelayer()" << endl;
        ++s_count;
    }
    SyncDelayer::~SyncDelayer()
    {
        --s_count;
        LG_XSLTFS_D << "SyncDelayer::~SyncDelayer() size:" << m_flist.size()
                    << " s_count:" << s_count
                    << endl;
        if( !s_count )
        {
            LG_XSLTFS_D << "SyncDelayer::~SyncDelayer() calling functors size:" << m_flist.size() << endl;
//            cerr << "SyncDelayer::~SyncDelayer() calling functors size:" << m_flist.size() << endl;
            for( m_flist_t::iterator iter = m_flist.begin(); iter != m_flist.end(); ++iter )
            {
                f_functor f = iter->second;
                f( this );
            }
            m_flist.clear();
        }
    }
    bool
    SyncDelayer::exists()
    {
//        cerr << "SyncDelayer::exists() ret:" << (s_count > 0) << endl;
        return s_count > 0;
    }

    void
    SyncDelayer::add( void* key, const f_functor& f )
    {
        SyncDelayer::m_flist[ key ] = f;
    }

    void
    SyncDelayer::ensure( void* key, const f_functor& f )
    {
        if( SyncDelayer::m_flist.find( key ) == SyncDelayer::m_flist.end() )
        {
            add( key, f );
        }
    }
    
    
};
