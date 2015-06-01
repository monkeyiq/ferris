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

    $Id: semrawcreate.cpp,v 1.1 2006/12/07 07:02:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"

#include <sys/types.h>
#include <sys/sem.h>
#include <sstream>
#include <iostream>

using namespace std;

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
  #ifdef PLATFORM_OSX
  #else
/* according to X/OPEN we have to define it ourselves */
union semun {
    int val;                    /* value for SETVAL */
    struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
    unsigned short int *array;  /* array for GETALL, SETALL */
    struct seminfo *__buf;      /* buffer for IPC_INFO */
};
  #endif
#endif



int
main( int argc, char** argv )
{
    int rc = semget ( IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0x1FF );
    cerr << "semget rc:" << rc << endl;
    if( rc >= 0 )
    {
        union semun s;
        s.val = 1;
        rc = semctl( rc, 0, SETVAL, s );
        cerr << "semctl rc:" << rc << endl;
    }
        
    return 0;
}

