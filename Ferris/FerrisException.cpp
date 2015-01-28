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

    $Id: FerrisException.cpp,v 1.2 2010/09/24 21:30:37 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris_private.hh>
#include <FerrisException.hh>
#include <string>
#include <errno.h>

#ifdef FERRIS_HAVE_ATTR
#include <attr/xattr.h>
#endif

using namespace std;

namespace Ferris
{
    

    FerrisVFSExceptionBase::FerrisVFSExceptionBase(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const char* e,
        Attribute* a
        )
        :
        FerrisExceptionBase( state, log, e )
        {
            setAttribute( a );
        }

    FerrisVFSExceptionBase::~FerrisVFSExceptionBase() throw ()
    {
    }

    const string&
    FerrisVFSExceptionBase::whats() const
    {
        m_cache = _Base::whats();

        try {
            if(    Attr
                   && Attr->isParentBound())
            {
                m_cache += " For path:" + Attr->getDirPath();
            }
        }
        catch( exception& e )
        {
        }
    
        return m_cache;
    }



    
    void ThrowFromErrno( int eno, const std::string& e, Attribute* a )
    {
        string es = errnum_to_string( "", eno );

        fh_stringstream ss;
        ss << e << " reason:" << es;
    
        switch( eno )
        {
        case EACCES:  Throw_AccessDenied( tostr(ss), a );
        case ENOENT:  Throw_NoSuchSubContext( tostr(ss), a );
        case ENOSPC:  Throw_StorageFull(  tostr(ss), a );
        case EDQUOT:  Throw_QuotaStorageFull( tostr(ss), a );
        case ENOTSUP: Throw_NotSupported( tostr(ss), a );
        case EEXIST:  Throw_ObjectExists( tostr(ss), a );
#ifdef FERRIS_HAVE_ATTR
        case ENOATTR: Throw_NoSuchObject( tostr(ss), a );
#endif
        }
        Throw_GenericError( tostr(ss), a );
    }


    
};

