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

    $Id: FullTextIndexerSyntheticDocID.cpp,v 1.2 2010/09/24 21:30:52 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "FullTextIndexerSyntheticDocID_private.hh"
#include "Enamel.hh"

using namespace std;

namespace Ferris
{
    namespace FullTextIndex 
    {

        std::string
        FullTextIndexerSyntheticDocID::resolveDocumentID( docid_t id )
        {
            Items_By_DocID_t::iterator diter = Items.get<ITEMS_BY_DOCID>().find( id );
            if( diter == Items.get<ITEMS_BY_DOCID>().end() )
            {
                LG_IDX_W << "resolveDocumentID() id:" << id << " has no URL!" << endl;
                return "";
            }
            return diter->earl;
        }
                
        int
        FullTextIndexerSyntheticDocID::getDocID( const std::string earl )
        {
            Items_By_Earl_t::iterator eiter
                = Items.get<ITEMS_BY_EARL>().find( earl );
            if( eiter != Items.get<ITEMS_BY_EARL>().end() )
            {
                return eiter->docid;
            }
            
            static int newID = 0;
            ++newID;
            Items.insert( ContainerItem( newID, earl ) );
            return newID;
        }
    };
};
