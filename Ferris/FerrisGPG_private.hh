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

    $Id: FerrisGPG_private.hh,v 1.3 2010/09/24 21:30:38 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_GPG_PRIV_H_
#define _ALREADY_INCLUDED_FERRIS_GPG_PRIV_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <config.h>

#ifdef FERRIS_HAVE_GPGME
extern "C" {
#include <gpgme.h>
};
#endif

namespace Ferris
{
#ifdef FERRIS_HAVE_GPGME
    
    struct FERRISEXP_API GPGMEContextHolder
    {
        gpgme_ctx_t m_gctx;

        // takes ownership of gctx
        GPGMEContextHolder( gpgme_ctx_t gctx );
        GPGMEContextHolder();
        ~GPGMEContextHolder();
        gpgme_ctx_t operator*();
        operator gpgme_ctx_t()
            {
                return m_gctx;
            }
    };


    struct FERRISEXP_API GPGMEDataHolder
    {
    private:
        GPGMEDataHolder( const GPGMEDataHolder& );
        GPGMEDataHolder& operator=( const GPGMEDataHolder& d );

    public:
        gpgme_data_t m_gpt;

        GPGMEDataHolder();

        // takes ownership of 'd'
        GPGMEDataHolder( gpgme_data_t d );
        ~GPGMEDataHolder();

        void TakeOwnership( gpgme_data_t newdata );
        operator gpgme_data_t()
            {
                return m_gpt;
            }
    };

    gpgme_data_t getGPGData( fh_istream* iss );
    
#endif
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

#ifdef FERRIS_HAVE_GPGME
    FERRISEXP_API gpgme_ctx_t getGPGMEContextSingleton();
    FERRISEXP_API bool hasValidSignature( gpgme_ctx_t gctx, fh_context c );
    FERRISEXP_API bool hasValidSignature( fh_context c );
#endif
    
    FERRISEXP_API fh_istream getSignedDocumentStream( fh_context c, fh_istream iss );
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
};
#endif
