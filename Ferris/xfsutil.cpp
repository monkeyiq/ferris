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

    $Id: xfsutil.cpp,v 1.3 2010/09/24 21:31:03 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#ifdef FERRIS_HAVE_XFSPROGS
#include <xfsutil.hh>
#include <libxfs.h>
#endif
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/vfs.h>
#include <Ferris.hh>
#include <Resolver_private.hh>

namespace Ferris
{

    bool isXFS( int fd )
    {
#ifndef FERRIS_HAVE_XFSPROGS
        return false;
#else
        struct statfs	buf;

        fstatfs(fd, &buf);
        if (buf.f_type == XFS_SUPER_MAGIC)
        {
            return true;
        }
        return false;
#endif
    }
    
    bool isXFS( fh_context c )
    {
#ifndef FERRIS_HAVE_XFSPROGS
        return false;
#else
        const std::string s = c->getDirPath();
        return isXFS( s );
#endif
    }
    
    bool isXFS( const std::string& s )
    {
#ifndef FERRIS_HAVE_XFSPROGS
        return false;
#else
        FerrisURL fu = FerrisURL::fromString( s );

        LG_NATIVE_D << " isXFS s:" << s
                    << " scheme:" << fu.getScheme()
                    << endl;

        if( fu.getScheme() != "file" )
        {
            LG_NATIVE_D << "isXFS s:" << s
                        << " not using a file scheme: " << fu.getScheme()
                        << endl;
            return false;
        }

        int fd = open( s.c_str(), O_RDONLY);
        if (fd < 0)
        {
            return false;
        }

        bool ret = isXFS( fd );
        LG_NATIVE_D << " isXFS() s:" << s << " ret:" << ret << endl;
        close(fd);
        return ret;
#endif
    }
    
};
