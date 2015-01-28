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

    $Id: EAQueryContext.hh,v 1.3 2010/09/24 21:30:32 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_EAQUERY_CONTEXT_H_
#define _ALREADY_INCLUDED_FERRIS_EAQUERY_CONTEXT_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>

namespace Ferris
{
    /**
     * Root context for full text queries this is the context for
     * eaquery://
     */
    class FERRISEXP_DLLLOCAL EAQueryRootContext
        :
        public StateLessEAHolder< EAQueryRootContext, FakeInternalContext >
    {
        typedef StateLessEAHolder< EAQueryRootContext, FakeInternalContext > _Base;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );

    protected:

        virtual void priv_read();
        
    public:

        EAQueryRootContext();
        virtual ~EAQueryRootContext();

        virtual fh_context
        createSubContext( const std::string& rdn, fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );

        void createStateLessAttributes( bool force = false );
    };

    /**
     * Provides assistance methods for accessing the EA index via a
     * filesystem interface.
     *
     * A query can be triggered either through the
     * create interface or by just attempting to get a subcontext
     */
    class FERRISEXP_DLLLOCAL EAQueryRunnerContext
        :
        public StateLessEAHolder< EAQueryRunnerContext, FakeInternalContext >
    {
        typedef StateLessEAHolder< EAQueryRunnerContext, FakeInternalContext > _Base;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );

    protected:

        virtual void priv_read();
        virtual fh_context priv_getSubContext( const std::string& rdn )
            throw( NoSuchSubContext );

    public:

        EAQueryRunnerContext( Context* parent, const std::string& rdn );
        virtual ~EAQueryRunnerContext();

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
     * Class to perform normal ffilter based queries. 
     * eaquery://filter/
     */
    class FERRISEXP_DLLLOCAL FullTextQueryContext_FFilter
        :
        public EAQueryRunnerContext
    {
        typedef EAQueryRunnerContext _Base;

        bool m_useShortedNames;
        int m_limit;
        
    public:

        FullTextQueryContext_FFilter( Context* parent, const std::string& rdn,
                                      bool useShortedNames = false,
                                      int limit = 0 );
        virtual ~FullTextQueryContext_FFilter();
        void createStateLessAttributes( bool force = false );
        virtual fh_context performQuery( const std::string& rdn );
        
    };


    
};
#endif
