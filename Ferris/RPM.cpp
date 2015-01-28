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

    $Id: RPM.cpp,v 1.2 2010/09/24 21:30:56 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <RPM_private.hh>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>

#ifdef HAVE_LIBRPM

#include <iostream>
using namespace std;

namespace Ferris
{
    static long rpmdb_ref_count = 0;
    static rpmdb rpmdb_db = NULL;
    static guint rpmdb_timer = 0;
    const int rpmdb_timer_interval = 5000;

    static gint
    rpmdb_closedb(gpointer data)
    {
        rpmdbClose( rpmdb_db );
        rpmdb_db = 0;
    }
    
    rpmdb get_rpmdb()
    {
        if( !rpmdb_db )
        {
            rpmReadConfigFiles(NULL, NULL);
            if (rpmdbOpen(NULL, &rpmdb_db, O_RDONLY, 0644))
            {
                cerr << "could not open RPM database\n" << endl;
                return 0;
            }
        }

        if( rpmdb_timer )
        {
            g_source_remove( rpmdb_timer );
            rpmdb_timer = 0;
        }
        ++rpmdb_ref_count;
        return rpmdb_db;
    }

    void release_rpmdb( rpmdb d )
    {
        if( d )
        {
            --rpmdb_ref_count;
            if( !rpmdb_ref_count )
            {
                if( !rpmdb_timer )
                    rpmdb_timer  = g_timeout_add( rpmdb_timer_interval,
                                                  GSourceFunc(rpmdb_closedb), 0 );
            }
        }
    }
    
    
};
#endif
