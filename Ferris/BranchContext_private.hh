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

    $Id: BranchContext_private.hh,v 1.5 2010/09/24 21:30:24 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_BRANCH_CONTEXT_PRIVH_
#define _ALREADY_INCLUDED_FERRIS_BRANCH_CONTEXT_PRIVH_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>

namespace Ferris
{
    class FerrisBranchInternalContext;

    typedef Loki::Functor< FerrisBranchInternalContext*,
                           LOKI_TYPELIST_3( Context*, const fh_context&, const std::string& ) >
    BranchInternalContextCreatorFunctor_t;
    
    /**
     * Root context for branch filesystems. eg. gpg-signatures://
     */
    class FERRISEXP_DLLLOCAL FerrisBranchRootContext
        :
        public StateLessEAHolder< FerrisBranchRootContext, FakeInternalContext >
    {
        typedef FerrisBranchRootContext                                           _Self;
        typedef StateLessEAHolder< FerrisBranchRootContext, FakeInternalContext > _Base;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );

        BranchInternalContextCreatorFunctor_t m_BranchInternalContextCreatorFunctor;
        
    protected:

        virtual void priv_read();
        
    public:

        FerrisBranchRootContext( BranchInternalContextCreatorFunctor_t m_BranchInternalContextCreatorFunctor );
        virtual ~FerrisBranchRootContext();

        void createStateLessAttributes( bool force = false );
        BranchInternalContextCreatorFunctor_t& getCreator();

        virtual stringset_t& getForceLocalAttributeNames();
    };

    FERRISEXP_DLLLOCAL bool FerrisBranchRootContext_Register( const std::string& url_scheme,
                                                              BranchInternalContextCreatorFunctor_t m_BranchInternalContextCreatorFunctor );
                                                     

    /**
     * Context to mirror the underlying filesystem and carry the base fh_context pointer
     * down to the file level in the layer that is sitting above the normal filesystem.
     *
     * eg. gpg-signatures://file:///tmp will have a Delegate of /tmp
     *
     */
    class FERRISEXP_DLLLOCAL FerrisBranchInternalContext
        :
        public ChainedViewContext
    {
        typedef FerrisBranchInternalContext _Self;
        typedef ChainedViewContext                 _Base;
        typedef Context                            _DontDelegateBase;

    protected:

        virtual void priv_read_leaf() = 0;


        virtual bool isDir();
        static fh_stringstream SL_getIsFile( Context* c, const std::string& rdn, EA_Atom* atom );
        
        
        virtual void UnPageSubContextsIfNeeded();
//         virtual std::string private_getStrAttr( const std::string& rdn,
//                                                 const std::string& def = "",
//                                                 bool getAllLines = false ,
//                                                 bool throwEx = false );

        fh_context addNewChild( fh_context c );
        
    public:

        void setupState( fh_context m_delegate )
            {
                setDelegate( m_delegate );
            }
        FerrisBranchInternalContext( Context* theParent, const fh_context& theDelegate );
        FerrisBranchInternalContext( Context* theParent, const fh_context& theDelegate,
                                            const std::string& rdn );
        virtual ~FerrisBranchInternalContext();

        virtual const std::string& getDirName() const;
        virtual std::string getDirPath() throw (FerrisParentNotSetError);
        virtual std::string getURL();

        stringset_t& getForceLocalAttributeNames();
//         virtual fh_attribute getAttribute( const std::string& rdn ) throw( NoSuchAttribute );
//         virtual AttributeNames_t& getAttributeNames( AttributeNames_t& ret );
//         virtual int  getAttributeCount();
//         virtual bool isAttributeBound( const std::string& rdn,
//                                        bool createIfNotThere = true
//             ) throw( NoSuchAttribute );
        
        virtual void OnDeleted( NamingEvent_Deleted* ev, std::string olddn, std::string newdn );
        virtual void OnExists ( NamingEvent_Exists* ev,
                                const fh_context&,
                                std::string olddn, std::string newdn );
        virtual void OnCreated( NamingEvent_Created* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn );
        
        virtual void read( bool force = 0 );
        virtual long guessSize() throw();
        

        void createStateLessAttributes( bool force = false );
        _Self* priv_CreateContext( Context* parent, std::string rdn );
        virtual fh_context priv_getSubContext( const std::string& rdn )
            throw( NoSuchSubContext );

        BranchInternalContextCreatorFunctor_t& getCreator();
    };
    
};
#endif

