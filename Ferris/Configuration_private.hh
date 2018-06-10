/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of libferris.

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

    $Id: Configuration_private.hh,v 1.4 2010/09/24 21:30:26 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_CONFIGURATION_PRIVATE_H_
#define _ALREADY_INCLUDED_FERRIS_CONFIGURATION_PRIVATE_H_

#include <Ferris.hh>

#include <STLdb4/stldb4.hh>
// #ifdef FERRIS_HAVE_DB4
// #include <db_cxx.h>
// #endif


namespace Ferris
{
    /* Private use only */

    FERRISEXP_API std::string& GET_FDB_LOGGING();
    FERRISEXP_API extern const std::string FDB_LOGGING;
    FERRISEXP_API extern const std::string FDB_SECURE;
    FERRISEXP_API extern const std::string FDB_CACHE;
    FERRISEXP_API extern const std::string CREATEHISTORY_RELATIVE;


    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    FERRISEXP_API std::string get_db4_string( STLdb4::fh_database db,
                                              const std::string& k,
                                              const std::string& def = "",
                                              bool throw_for_errors = true );
    FERRISEXP_API std::string get_db4_string( const std::string& filename,
                                              const std::string& k,
                                              const std::string& def = "",
                                              bool throw_for_errors = true );
    FERRISEXP_API std::string get_db4_string( const std::string& filename,
                                              const std::string& k,
                                              const std::string& def,
                                              bool throw_for_errors,
                                              bool read_only );
    

    FERRISEXP_API void set_db4_string( STLdb4::fh_database db,
                                       const std::string& k,
                                       const std::string& v,
                                       bool throw_for_errors = true );
    FERRISEXP_API void set_db4_string( const std::string& filename,
                                       const std::string& k,
                                       const std::string& v,
                                       bool throw_for_errors = true );

    /**
     * Make sure that the db4 file 'dbName' exists in the directory
     * parentPath. The dir parentPath is created if it doesn't already
     * exist and the db4 file is created if it doesn't already exist.
     *
     * @param parentPath parent dir of new config file
     * @param dbName_maybeWithPrefixSlashes The rdn of the config file,
     *              any leading '/' chars are first stripped.
     * @return The opened db file for config settings with set_db4_string() etc.
     */
    FERRISEXP_API
    STLdb4::fh_database
    ensureFerrisConfigFileExists( const std::string& parentPath,
                                  const std::string& dbName_maybeWithPrefixSlashes );
    
};
#endif
