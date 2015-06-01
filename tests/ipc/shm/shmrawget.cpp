/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris 
    Copyright (C) 2002 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: shmrawget.cpp,v 1.2 2008/05/25 21:30:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include <sys/types.h>
#include <sys/shm.h>
#include <stdlib.h>

#include <sstream>
#include <iostream>

using namespace std;

int
main( int argc, char** argv )
{
    if( argc < 3 )
    {
        cerr << "Usage: " << argv[0] << " shmid size " << endl;
        exit(1);
    }
    
    int shmid = 0;
    int size = 0;
    {
        stringstream ss;
        ss << argv[1];
        ss >> shmid;
    }
    {
        stringstream ss;
        ss << argv[2];
        ss >> size;
    }

    char* p = (char*)shmat( shmid, 0, 0 );
    cerr << "shmid:" << shmid << " size:" << size << " p:" << (void*)p << endl;

    for( int i=0; i < size; ++i )
    {
        cerr << " i:" << i
             << " p[" << i << "]:" << hex << (int)p[i]
             << " p[" << i << "]:" << p[i]
             << endl;
    }
    
    shmdt( p );

    return 0;
}

