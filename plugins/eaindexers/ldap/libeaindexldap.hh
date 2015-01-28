/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2004 Ben Martin

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

    $Id: libeaindexldap.hh,v 1.2 2010/09/24 21:31:57 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_EAIDX_LDAP_H_
#define _ALREADY_INCLUDED_FERRIS_EAIDX_LDAP_H_

#define CFG_LDAPIDX_SERVERNAME_K        "ldapidx-servername"
#define CFG_LDAPIDX_SERVERNAME_DEFAULT  "localhost"
#define CFG_LDAPIDX_PORT_K              "ldapidx-port"
#define CFG_LDAPIDX_PORT_DEFAULT        "389"
#define CFG_LDAPIDX_BASEDN_K            "basedn"
#define CFG_LDAPIDX_BASEDN_DEFAULT      ""
#define CFG_LDAPIDX_USERNAME_K          "ldapidx-user"
#define CFG_LDAPIDX_USERNAME_DEFAULT    ""
#define CFG_LDAPIDX_PASSWORD_K          "ldapidx-password"
#define CFG_LDAPIDX_PASSWORD_DEFAULT    ""
        
#define CFG_LDAPIDX_CURRENTMAXDOCID_K          "ldapidx-currentmaxdocid"
#define CFG_LDAPIDX_CURRENTMAXDOCID_DEFAULT    "1"

#define CFG_LDAPIDX_NEWOID_K          "ldapidx-newoid"
#define CFG_LDAPIDX_NEWOID_DEFAULT    "1"

#endif
