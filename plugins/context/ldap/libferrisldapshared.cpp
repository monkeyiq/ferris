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

    $Id: libferrisldapshared.cpp,v 1.3 2010/09/24 21:31:39 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris.hh>
#include <Ferris_private.hh>
#include <Trimming.hh>
#include <libferrisldapshared.hh>

using namespace std;

#define DBNAME FDB_SECURE


namespace Ferris
{
    namespace LDAPAuth
    {

        LDAPAuthInfo::LDAPAuthInfo()
            :
            username(""), password(""), basedn(""), lookup_basedn( true )
        {}

        LDAPAuthInfo::LDAPAuthInfo( const LDAPAuthInfo& d )
            :
            username( d.username ),
            password( d.password ),
            basedn( d.basedn ),
            lookup_basedn( d.lookup_basedn )
        {
        }
        
        LDAPAuthInfo& LDAPAuthInfo::operator=( const LDAPAuthInfo& d )
        {
            if( this != &d )
            {
                username = d.username;
                password = d.password;
                basedn   = d.basedn;
                lookup_basedn = d.lookup_basedn;
            }
            return *this;
        }
        
        
        

        static string KPREFIX = "ldap-server-";

        static string makeKey( string serv, string post )
        {
            fh_stringstream ss;
            ss << KPREFIX << serv << post;
            return tostr(ss);
        }

        
        LDAPAuthInfo getUserPass( std::string serv )
        {
//            cerr << "getUserPass(top) serv:" << serv << endl;
            LDAPAuthInfo ret;

            ret.username = getEDBString( DBNAME, makeKey( serv, "-username" ), "");
            ret.password = getEDBString( DBNAME, makeKey( serv, "-password" ), "");
            ret.basedn   = getEDBString( DBNAME, makeKey( serv, "-basedn" ), "");
            ret.lookup_basedn =
                isTrue( getEDBString( DBNAME, makeKey( serv, "-lookup-basedn" ), "0" ));
            if( ret.basedn.empty() )
                ret.lookup_basedn = "1";

//             cerr << "getUserPass() serv:" << serv << endl;
//             cerr << " u:" << ret.username << " p:" << ret.password
//                  << " base:" << ret.basedn << " lookup:" << ret.lookup_basedn
//                  << endl;

            return ret;
        }
        
        void setUserPass( std::string serv, const LDAPAuthInfo& d )
        {
            setEDBString( DBNAME, makeKey( serv, "-username" ), d.username );
            setEDBString( DBNAME, makeKey( serv, "-password" ), d.password );
            setEDBString( DBNAME, makeKey( serv, "-basedn" ), d.basedn );
            setEDBString( DBNAME, makeKey( serv, "-lookup-basedn" ), tostr(d.lookup_basedn) );
        }
        
    };
};
