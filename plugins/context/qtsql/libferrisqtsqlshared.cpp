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

    $Id: libferrispostgresqlshared.cpp,v 1.2 2006/12/07 06:49:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <libferrisqtsqlshared.hh>
#include <Configuration_private.hh>

namespace Ferris
{
    using namespace std;
    static const string DBNAME = FDB_SECURE;

    FERRISEXP_EXPORT userpass_t getQtSQLUserPass(
        const std::string& server )
    {
        string user;
        string pass;

        string Key = server;
        
        {
            fh_stringstream ss;
            ss << "qtsql-database-" << Key << "-username";
            user = getEDBString( DBNAME, tostr(ss), "" );
        }
        
        {
            fh_stringstream ss;
            ss << "qtsql-database-" << Key << "-password";
            pass = getEDBString( DBNAME, tostr(ss), "" );
        }

        return make_pair( user, pass );
    }
    
    FERRISEXP_EXPORT void setQtSQLUserPass(
        const std::string& server,
        const std::string& user, const std::string& pass )
    {
        string Key = server;

        {
            fh_stringstream ss;
            ss << "qtsql-database-" << Key << "-username";
            setEDBString( DBNAME, tostr(ss), user );
        }

        {
            fh_stringstream ss;
            ss << "qtsql-database-" << Key << "-password";
            setEDBString( DBNAME, tostr(ss), pass );
        }        
    }
    
};
