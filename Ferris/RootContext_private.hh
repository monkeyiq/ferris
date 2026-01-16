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

    $Id: RootContext_private.hh,v 1.4 2010/09/24 21:30:58 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_ROOT_CONTEXT_PRIV_H_
#define _ALREADY_INCLUDED_FERRIS_ROOT_CONTEXT_PRIV_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/Context.hh>

namespace Ferris
{
    

    class FERRISEXP_DLLLOCAL RootContext : public ChainedViewContext
    {
        typedef ChainedViewContext _Base;
        typedef Context _NonChainedBase;  //< lowest superclass which is not under ChainedViewContext
        
        friend class RootContextFactory;

        Context* priv_CreateContext( Context* parent, std::string rdn );
    
    protected:

        std::set< std::string >& getPreferLocalAttributeNames();
        bool isAttributeLocal( const std::string& s );
        virtual std::string private_getStrAttr( const std::string& rdn,
                                                const std::string& def = "",
                                                bool getAllLines = false ,
                                                bool throwEx = false );
        
    public:
        RootContext( Context* parent,
                     Context* delegate,
                     const std::string& rdn = "" );

        
        virtual void read( bool force = 0 );
        virtual const std::string& getDirName() const;
        virtual std::string getDirPath();
        virtual std::string getURL();
        virtual std::string getURLScheme();

        virtual fh_attribute getAttribute( const std::string& rdn );
        virtual AttributeNames_t& getAttributeNames( AttributeNames_t& ret );
        virtual int  getAttributeCount();
        virtual bool isAttributeBound( const std::string& rdn,
                                       bool createIfNotThere = true
            );
        
//        virtual void OnExists ( NamingEvent_Exists* ev,  std::string olddn, std::string newdn );
    };

};
#endif
