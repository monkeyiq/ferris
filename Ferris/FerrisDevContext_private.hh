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

    $Id: FerrisDevContext_private.hh,v 1.2 2010/09/24 21:30:36 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_DEV_CONTEXT_PRIVH_
#define _ALREADY_INCLUDED_FERRIS_DEV_CONTEXT_PRIVH_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>

namespace Ferris
{
    /**
     * Root context for ferrisdev://
     */
    class FERRISEXP_DLLLOCAL FerrisDevRootContext
        :
        public StateLessEAHolder< FerrisDevRootContext, FakeInternalContext >
    {
      public:
        typedef FerrisDevRootContext                                           _Self;
        typedef StateLessEAHolder< FerrisDevRootContext, FakeInternalContext > _Base;

        
    protected:

        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );
        virtual void priv_read();
        
    public:

        FerrisDevRootContext();
        virtual ~FerrisDevRootContext();

        void createStateLessAttributes( bool force = false );

        
    };

    /**
     *
     * Generates a unique ID either based on a overflowing 64bit integer
     * or a uuid depending on which file is read.
     *
     */
    class FERRISEXP_DLLLOCAL FerrisDevGeneratorContext
        :
        public StateLessEAHolder< FerrisDevGeneratorContext, leafContext >
    {
    public:
        friend class FerrisDevRootContext;
        typedef FerrisDevGeneratorContext                                   _Self;
        typedef StateLessEAHolder< FerrisDevGeneratorContext, leafContext > _Base;
        

        FerrisDevGeneratorContext( Context* parent, const char* rdn );
        virtual ~FerrisDevGeneratorContext();

        void createStateLessAttributes( bool force = false );

    };

    extern std::string exstr_64;
    extern std::string exstr_32;
    
    
};
#endif
