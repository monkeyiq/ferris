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

    $Id: FullTextContext.hh,v 1.2 2010/09/24 21:30:50 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_FULLTEXT_CONTEXT_H_
#define _ALREADY_INCLUDED_FERRIS_FULLTEXT_CONTEXT_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>

namespace Ferris
{
    /**
     * Root context for full text queries this is the context for
     * fulltextquery://
     */
    class FERRISEXP_DLLLOCAL FullTextQueryRootContext
        :
        public StateLessEAHolder< FullTextQueryRootContext, FakeInternalContext >
    {
        typedef StateLessEAHolder< FullTextQueryRootContext, FakeInternalContext > _Base;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );

    protected:

        virtual void priv_read();
        
    public:

        FullTextQueryRootContext();
        virtual ~FullTextQueryRootContext();

        virtual fh_context
        createSubContext( const std::string& rdn, fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );

        void createStateLessAttributes( bool force = false );
    };

    /**
     * Provides assistance methods for the ranked and boolean query
     * context classes. A query can be triggered either through the
     * create interface or by just attempting to get a subcontext
     */
    class FERRISEXP_DLLLOCAL FullTextQueryRunnerContext
        :
        public StateLessEAHolder< FullTextQueryRunnerContext, FakeInternalContext >
    {
        typedef StateLessEAHolder< FullTextQueryRunnerContext, FakeInternalContext > _Base;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );

    protected:

        virtual void priv_read();
        virtual fh_context priv_getSubContext( const std::string& rdn )
            throw( NoSuchSubContext );

    public:

        FullTextQueryRunnerContext( Context* parent, const std::string& rdn );
        virtual ~FullTextQueryRunnerContext();

        virtual fh_context
        createSubContext( const std::string& rdn, fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );

        void createStateLessAttributes( bool force = false );

        /**
         * Subcontext classes should override this method and perform a
         * fulltext query when this is called.
         */
        virtual fh_context performQuery( const std::string& rdn ) = 0;
    };

    
    /**
     * Class to perform ranked queries. 
     * fulltextquery://ranked
     */
    class FERRISEXP_DLLLOCAL FullTextQueryContext_Ranked
        :
        public FullTextQueryRunnerContext
    {
        typedef FullTextQueryRunnerContext _Base;
        
    public:

        FullTextQueryContext_Ranked( Context* parent, const std::string& rdn );
        virtual ~FullTextQueryContext_Ranked();
        void createStateLessAttributes( bool force = false );
        virtual fh_context performQuery( const std::string& rdn );
    };


    /**
     * Class to perform boolean queries. 
     * fulltextquery://boolean
     */
    class FERRISEXP_DLLLOCAL FullTextQueryContext_Boolean
        :
        public FullTextQueryRunnerContext
    {
        typedef FullTextQueryRunnerContext _Base;
        
    public:

        FullTextQueryContext_Boolean( Context* parent, const std::string& rdn );
        virtual ~FullTextQueryContext_Boolean();
        void createStateLessAttributes( bool force = false );
        virtual fh_context performQuery( const std::string& rdn );
    };
    
};
#endif
