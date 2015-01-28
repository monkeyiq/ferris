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

    $Id: MetadataServer_private.hh,v 1.5 2010/09/24 21:30:55 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_METADATA_SERVER_PRIV_H_
#define _ALREADY_INCLUDED_FERRIS_METADATA_SERVER_PRIV_H_

#include <Ferris/Ferris.hh>
#include <Ferris/Image_private.hh>
#include <Ferris/FerrisEAGeneratorPlugin_private.hh>

#include <string>

#include <QtDBus>
#include <QCoreApplication>



namespace Ferris
{
#ifdef GCC_HASCLASSVISIBILITY
#pragma GCC visibility push(default)
#endif
    
    std::string syncMetadataServerGet( const std::string& earl,
                                       const std::string& rdn );
    void syncMetadataServerPut( const std::string& earl,
                                const std::string& rdn,
                                const std::string& value );

//    void resetup_glib_idle_to_close_callback_function( int interval, bool* running );


    stringset_t& getImageMetadataAttributes();
    bool isImageMetadataAttribute( const std::string& rdn );
    s_ImageEAGeneratorsExtensionToShortName_t& getOutOfProcess_ImageEAGeneratorsExtensionToShortName();


    StaticEAGenFactorys_t& getOutOfProcess_EAGeneratorsStaticFactories( StaticEAGenFactorys_t& ret );
    const stringset_t& getOutOfProcess_EAGeneratorsStaticFactoriesShortNamesToUse();

    bool isOutOfProcessMetadataAttribute( const std::string& rdn );

    
#ifdef GCC_HASCLASSVISIBILITY
#pragma GCC visibility pop
#endif
};
#endif
