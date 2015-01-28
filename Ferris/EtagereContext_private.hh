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

    $Id: EtagereContext_private.hh,v 1.3 2010/09/24 21:30:32 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_ETAGERE_CONTEXT_PRIVH_
#define _ALREADY_INCLUDED_FERRIS_ETAGERE_CONTEXT_PRIVH_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/Medallion.hh>

namespace Ferris
{
    class FERRISEXP_DLLLOCAL EmblemCommonCreator
    {
    protected:
        std::pair< fh_context, fh_emblem > SubCreate_emblem( fh_context c, fh_context md, bool isRoot );
    };
    
    /**
     * Root context for emblem partial order,
     * this is the context for etagere://
     */
    class FERRISEXP_DLLLOCAL EtagereRootContext
        :
        public StateLessEAHolder< EtagereRootContext, FakeInternalContext >,
        public EmblemCommonCreator
    {
        typedef EtagereRootContext                                           _Self;
        typedef StateLessEAHolder< EtagereRootContext, FakeInternalContext > _Base;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );

        bool m_showFilesAsLeaves;
        
    protected:

        virtual void priv_read();
        virtual fh_context SubCreate_file( fh_context c, fh_context md );
        virtual bool supportsRemove();
        virtual void priv_remove( fh_context c );
        virtual std::string priv_getRecommendedEA();
        
    public:

        EtagereRootContext( bool m_showFilesAsLeaves = false );
        virtual ~EtagereRootContext();

        void createStateLessAttributes( bool force = false );

        void OnChildAdded( fh_etagere, fh_emblem );
        void OnChildRemoved( fh_etagere, fh_emblem );
        
    };

    /**
     */
    class FERRISEXP_DLLLOCAL EmblemContext
        :
        public StateLessEAHolder< EmblemContext, FakeInternalContext >,
        public EmblemCommonCreator
    {
        friend class EtagereRootContext;
        friend class EmblemCommonCreator;
        
        typedef EmblemContext                                           _Self;
        typedef StateLessEAHolder< EmblemContext, FakeInternalContext > _Base;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );

    protected:

        fh_emblem m_em;
        bool m_showFilesAsLeaves;

        virtual void priv_read();
        virtual fh_context SubCreate_file( fh_context c, fh_context md );
        virtual bool supportsRemove();
        virtual void priv_remove( fh_context c );
        virtual std::string priv_getRecommendedEA();

        virtual bool supportsRename();
        virtual fh_context priv_rename( const std::string& rdn,
                                        const std::string& newPath,
                                        bool TryToCopyOverFileSystems = true,
                                        bool OverWriteDstIfExists = false );
        
    public:

        EmblemContext( Context* parent, fh_emblem em, bool m_showFilesAsLeaves = false );
        virtual ~EmblemContext();

        void createStateLessAttributes( bool force = false );

        void OnChildAdded( fh_emblem em, fh_emblem child );
        void OnChildRemoved( fh_emblem em, fh_emblem child );

        static fh_istream SL_getFSID( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_istream SL_getDescription( EmblemContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getDigitalLatitude( EmblemContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getDigitalLongitude( EmblemContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getZoomRange( EmblemContext* c, const std::string& rdn, EA_Atom* atom );


        static void SL_updateDigitalLatitude( EmblemContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss );
        static void SL_updateDigitalLongitude( EmblemContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss );
        static void SL_updateZoomRange( EmblemContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss );

        void sync();
        
    };

    
};
#endif
