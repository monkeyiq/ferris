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

    $Id: libferrissqlplusshared.cpp,v 1.2 2010/09/24 21:31:48 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris.hh>
#include <Ferris_private.hh>
#include <Trimming.hh>
#include <sqlplus.hh>
#include <libferrissqlplusshared.hh>

using namespace std;

#define DBNAME FDB_SECURE


namespace Ferris
{
    typedef pair<string,string> userpass_t;
    
    userpass_t getUserPass( string serv )
    {
        string user;
        string pass;
        
        {
            fh_stringstream ss;
            ss << "mysql-server-" << serv << "-username";
            user = getConfigString( DBNAME, tostr(ss), "" );
        }
        
        {
            fh_stringstream ss;
            ss << "mysql-server-" << serv << "-password";
            pass = getConfigString( DBNAME, tostr(ss), "" );
        }

        return make_pair( user, pass );
    }


    void setUserPass( string serv, const std::string& user, const std::string& pass )
    {
        {
            fh_stringstream ss;
            ss << "mysql-server-" << serv << "-username";
            setConfigString( DBNAME, tostr(ss), user );
        }

        {
            fh_stringstream ss;
            ss << "mysql-server-" << serv << "-password";
            setConfigString( DBNAME, tostr(ss), pass );
        }
    }
    
};
