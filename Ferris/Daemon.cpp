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

    $Id: Daemon.cpp,v 1.2 2010/09/24 21:30:29 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "Resolver_private.hh"

using namespace std;

namespace Ferris
{
    /**
     * Detach from the controlling shell
     */
    bool SwitchToDaemonMode()
    {
        pid_t pid;

        if(( pid = fork() ) < 0 )
            return -1;
        else if( pid != 0 )
        {
            /* close the parent proc */
            exit( 0 );
        }

        /* child */
        setsid();
        chdir("/");
        umask(0);
        return 0;
    }

    static int lock_reg( int fd, int cmd, int type, off_t offset, int whence, off_t len )
    {
        struct flock l;

        l.l_type   = type;
        l.l_start  = offset;
        l.l_whence = whence;
        l.l_len    = len;

        return fcntl( fd, cmd, &l );
    }

#define write_lock( fd, offset, whence, len ) \
    lock_reg( fd, F_SETLK, F_WRLCK, offset, whence, len )
    
    
    /**
     * Try to obtain a lock on the given path, if the file is already locked
     * exit.
     */
    void ExitIfAlreadyRunning( const std::string& _lockPath )
    {
        string lockPath = CleanupURL( _lockPath );
        if( starts_with( lockPath, "file:" ) )
            lockPath = lockPath.substr( 5 );
            
        int fd = open( lockPath.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
        if( fd < 0 )
        {
            string es = errnum_to_string( "", errno );
            cerr << "Can't open lock file:" << lockPath
                 << " reason:" << es << endl
                 << " exiting." << endl;
            exit(1);
        }

        if( write_lock( fd, 0, SEEK_SET, 0 ) < 0 )
        {
            if( errno == EACCES || errno == EAGAIN )
                exit(0);
            else
            {
                string es = errnum_to_string( "Write lock error", errno );
                cerr << es << endl;
                exit(1);
            }
        }

        int val;
        if( (val = fcntl( fd, F_GETFD, 0 )) < 0 )
        {
            string es = errnum_to_string( "getting fd status", errno );
            cerr << es << endl;
            exit(1);
        }
        val |= FD_CLOEXEC;
        if( fcntl( fd, F_SETFD, val ) < 0 )
        {
            string es = errnum_to_string( "setting fd status", errno );
            cerr << es << endl;
            exit(1);
        }
        
    }
    
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
};
