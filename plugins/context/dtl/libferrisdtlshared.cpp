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

    $Id: libferrisdtlshared.cpp,v 1.2 2010/09/24 21:31:34 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <libferrisdtlshared.hh>
#include <Configuration_private.hh>

namespace Ferris
{
    using namespace std;
    static const string DBNAME = FDB_SECURE;
    
    userpass_t getODBCUserPass( std::string DSN )
    {
        string user;
        string pass;
        
        {
            fh_stringstream ss;
            ss << "odbc-dsn-" << DSN << "-username";
            user = getConfigString( DBNAME, tostr(ss), "" );
        }
        
        {
            fh_stringstream ss;
            ss << "odbc-dsn-" << DSN << "-password";
            pass = getConfigString( DBNAME, tostr(ss), "" );
        }

        return make_pair( user, pass );
    }

    void setODBCUserPass( std::string serv, const std::string& user, const std::string& pass )
    {
        {
            fh_stringstream ss;
            ss << "odbc-dsn-" << serv << "-username";
            setConfigString( DBNAME, tostr(ss), user );
        }

        {
            fh_stringstream ss;
            ss << "odbc-dsn-" << serv << "-password";
            setConfigString( DBNAME, tostr(ss), pass );
        }        
    }
    
};
