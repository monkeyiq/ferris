/******************************************************************************
*******************************************************************************
*******************************************************************************

    feaindex-federation-add-index command line client
    Copyright (C) 2006 Ben Martin

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

    $Id: DirectIndexConfigAccess.hh,v 1.2 2010/09/24 21:31:15 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/Configuration_private.hh>
#include <Ferris/EAIndexer_private.hh>
using namespace STLdb4;
using namespace std;

namespace Ferris
{
    fh_database getDB( EAIndex::fh_idx eidx )
    {
        string dbfilename = CleanupURL( eidx->getPath() + "/" + EAIndex::DB_EAINDEX );
        fh_database db = new Database( dbfilename );
        return db;
    }

    string getConfig( fh_database db, const std::string& k, const std::string& def )
    {
        string ret = get_db4_string( db, k, def, true );
        return ret;
    }

    void setConfig( fh_database db, const std::string& k, const std::string& v )
    {
        set_db4_string( db, k, v );
        db->sync();
    }
};
