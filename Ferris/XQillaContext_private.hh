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

    $Id: XQillaContext_private.hh,v 1.2 2010/09/24 21:31:02 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_XPATH_CONTEXT_PRIVH_
#define _ALREADY_INCLUDED_FERRIS_XPATH_CONTEXT_PRIVH_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/framework/StdInInputSource.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>

#include <xqilla/xqilla-dom3.hpp>
#include <xqilla/exceptions/XQException.hpp>

namespace Ferris
{
    
    /**
     * root context for xpath evaluation
     * this is the context for xpath://
     */
    class FERRISEXP_DLLLOCAL XPathRootContext
        :
        public StateLessEAHolder< XPathRootContext, FakeInternalContext >
    {
        typedef XPathRootContext                                           _Self;
        typedef StateLessEAHolder< XPathRootContext, FakeInternalContext > _Base;

        bool m_querying;
        
    protected:

        virtual void priv_read();
        
    public:

        XPathRootContext();
        virtual ~XPathRootContext();

        void createStateLessAttributes( bool force = false );

        virtual fh_context getSubContext( const std::string& rdn ) throw( NoSuchSubContext );
        virtual fh_context performQuery( const std::string& rdn );
    };


    
};
#endif
