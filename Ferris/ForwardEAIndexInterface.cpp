/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2008 Ben Martin

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

    $Id: ForwardEAIndexInterface.cpp,v 1.2 2010/09/24 21:30:50 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "ForwardEAIndexInterface.hh"

namespace Ferris
{
    namespace EAIndex 
    {
        ForwardEAIndexInterface::ForwardEAIndexInterface()
            :
            m_precached( false )
        {
        }
        
        
        void
        ForwardEAIndexInterface::addEAToPrecache( const std::string& rdn )
        {
            m_precached = false;
            m_EAToPrecache.insert( rdn );
        }
        
        void
        ForwardEAIndexInterface::addEAToPrecache( const stringlist_t& sl )
        {
            m_precached = false;
            for( stringlist_t::const_iterator si = sl.begin(); si!=sl.end(); ++si )
                m_EAToPrecache.insert( *si );
        }

        void
        ForwardEAIndexInterface::addEAToPrecache( const stringset_t& sl )
        {
            m_precached = false;
            for( stringset_t::const_iterator si = sl.begin(); si!=sl.end(); ++si )
                m_EAToPrecache.insert( *si );
        }

        void
        ForwardEAIndexInterface::addDocumentID( docid_t id )
        {
            m_docNumSet.insert( id );
        }
        
        
    };
};
