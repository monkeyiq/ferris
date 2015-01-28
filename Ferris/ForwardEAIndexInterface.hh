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

    $Id: ForwardEAIndexInterface.hh,v 1.2 2010/09/24 21:30:50 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_EAIDX_FORWARDEAINDEXINTERFACE_H_
#define _ALREADY_INCLUDED_FERRIS_EAIDX_FORWARDEAINDEXINTERFACE_H_

#include <Ferris/TypeDecl.hh>
#include <Ferris/EAIndexerMetaInterface.hh>

namespace Ferris
{
    namespace EAIndex 
    {
        /**
         * EAIndex implementations that can offer forward lookups,
         * url,eaname --> eavalue
         * can implement this class to allow EA values to be exposed
         * directly out of the EA Index.
         */
        class FERRISEXP_API ForwardEAIndexInterface
            :
            public Handlable
        {
        public:
            typedef guint32 docid_t;                      
            typedef std::set< docid_t > docNumSet_t;
            typedef docNumSet_t m_docNumSet_t;

        protected:
            stringset_t m_EAToPrecache;
            bool m_precached;
            m_docNumSet_t m_docNumSet;

            ForwardEAIndexInterface();
            
        public:
            virtual void addEAToPrecache( const std::string& rdn );
            virtual void addEAToPrecache( const stringlist_t& sl );
            virtual void addEAToPrecache( const stringset_t& sl );
            virtual void addDocumentID( docid_t id );

            virtual std::string getStrAttr( Context* c,
                                            const std::string& earl,
                                            const std::string& rdn,
                                            const std::string& def,
                                            bool throw_for_errors = true ) = 0;
            
        };
        
    };
};


#endif
