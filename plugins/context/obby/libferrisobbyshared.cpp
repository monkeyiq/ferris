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

    $Id: libferrisobbyshared.cpp,v 1.2 2010/09/24 21:31:41 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#define LIBFERRIS__DONT_INCLUDE_SIGCC_EXTENSIONS

#include <libferrisobbyshared.hh>
#include <Configuration_private.hh>

namespace Ferris
{
    using namespace std;
    static const string DBNAME = FDB_SECURE;
    
    userpass_t getOBBYUserPass( std::string DSN )
    {
        string user;
        string pass;
        
        {
            fh_stringstream ss;
            ss << "obby-" << DSN << "-username";
            user = getConfigString( DBNAME, tostr(ss), "" );
        }
        
        {
            fh_stringstream ss;
            ss << "obby-" << DSN << "-password";
            pass = getConfigString( DBNAME, tostr(ss), "" );
        }

        return make_pair( user, pass );
    }

    void setOBBYUserPass( std::string serv, const std::string& user, const std::string& pass )
    {
        {
            fh_stringstream ss;
            ss << "obby-" << serv << "-username";
            setConfigString( DBNAME, tostr(ss), user );
        }

        {
            fh_stringstream ss;
            ss << "obby-" << serv << "-password";
            setConfigString( DBNAME, tostr(ss), pass );
        }        
    }
    
};
