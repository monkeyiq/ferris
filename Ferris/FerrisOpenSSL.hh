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

    $Id: FerrisOpenSSL.hh,v 1.4 2010/09/24 21:30:40 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_OPENSSL_H_
#define _ALREADY_INCLUDED_FERRIS_OPENSSL_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>


FERRISEXP_API void InitOpenSSL();
namespace Ferris
{
    FERRISEXP_API std::string digest( fh_istream,  std::string digestName = "md5" );
    FERRISEXP_API std::string digest( std::string, std::string digestName = "md5" );

    FERRISEXP_API std::string base64encode( const std::string& v );

    namespace Factory
    {
        fh_iostream MakeDigestStream( std::string digestName = "md5" );
    };
};



#endif

