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

    $Id: FullTextIndexerSyntheticDocID_private.hh,v 1.2 2010/09/24 21:30:52 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <string>

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/FullTextIndexerMetaInterface.hh>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>

namespace Ferris
{
    namespace FullTextIndex 
    {
        class FERRISEXP_API FullTextIndexerSyntheticDocID
            :
            public MetaFullTextIndexerInterface
        {
        protected:
            struct ContainerItem
            {
                int docid;
                std::string earl;
                ContainerItem( int docid = 0, const std::string& earl = "" )
                    :
                    docid( docid ),
                    earl( earl )
                    {
                    }
                inline int getDocID() const
                    {
                        return docid;
                    }
                inline const std::string& getEarl() const
                    {
                        return earl;
                    }
                
            };
            struct ITEMS_BY_DOCID {};
            struct ITEMS_BY_EARL {};
            
            typedef boost::multi_index::multi_index_container<
                ContainerItem,
                boost::multi_index::indexed_by<
                
                boost::multi_index::ordered_unique<
                boost::multi_index::tag<ITEMS_BY_EARL>,
                boost::multi_index::const_mem_fun<ContainerItem,
                                                  const std::string&,&ContainerItem::getEarl> >,
                
                boost::multi_index::hashed_unique<
                boost::multi_index::tag<ITEMS_BY_DOCID>,
                boost::multi_index::const_mem_fun<ContainerItem,
                                                  int,&ContainerItem::getDocID> >
            > 
            > Items_t;
            Items_t Items;
            typedef Items_t::index<ITEMS_BY_DOCID>::type Items_By_DocID_t;
            typedef Items_t::index<ITEMS_BY_EARL>::type  Items_By_Earl_t;

            virtual std::string resolveDocumentID( docid_t );
            int getDocID( const std::string earl );
            
        };
    };
};

