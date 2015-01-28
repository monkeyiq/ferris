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

    $Id: Context_private.hh,v 1.9 2010/09/24 21:30:28 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_CONTEXT_PRIVATE_H_
#define _ALREADY_INCLUDED_FERRIS_CONTEXT_PRIVATE_H_

#include <Ferris/HiddenSymbolSupport.hh>

namespace Ferris
{

/*
 * A context view that applies a particular ordering to the items in its
 * filesystem.
 *
 * As this is a private header, some stuff like thisIterator might be
 * given public access. Code should not use thisIterator unless abolutely
 * needed though.
 */
    class FERRISEXP_API SortedContext
        :
        public ChainedViewContext
    {
        typedef ChainedViewContext _Base;
        typedef SortedContext      _Self;
        
        bool LazySorting;
        SubContextNames_t LazySortCache;
        Items_t ItemsSorted;

        SortedContext( const fh_context& ctx, const Items_t& sorteditemscopy );
        void addToSorted( fh_context c, bool created, bool emit );
        
    public:
        Items_t::iterator thisIterator;

        SortedContext( const fh_context& ctx, const std::string& s );
        virtual void createStateLessAttributes( bool force = false );

        virtual void OnDeleted( NamingEvent_Deleted* ev, std::string olddn, std::string newdn );
        virtual void OnExists ( NamingEvent_Exists* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn );
        virtual void OnCreated( NamingEvent_Created* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn );
        
        virtual void read( bool force = 0 );
        virtual long guessSize() throw();

        void sort();
        void setLazySorting( bool v );
        virtual SubContextNames_t& getSubContextNames();

        virtual int  getMinimumReferenceCount();
        
    protected:

        friend ContextIterator::difference_type
        operator-(const ContextIterator& x, const ContextIterator& y);

        virtual Items_t& getSortedItems(); // see Ferris.cpp file for doco

        virtual void emitExistsEventForEachItem();
        virtual void UnPageSubContextsIfNeeded();

        /**
         * We break the link in ItemsSorted during a remove() operation
         */
        virtual void Remove( Context*   ctx, bool emitdeleted = true ); 
        
        virtual Context* priv_CreateContext( Context* parent, std::string rdn );
    };

    /**
     * A context that is like a softlink but can exist in virtual directories
     */
    class FERRISEXP_API VirtualSoftlinkContext
        :
        public ChainedViewContext
    {
        typedef VirtualSoftlinkContext  _Self;
        typedef ChainedViewContext      _Base;
        typedef Context                 _DontDelegateBase;

    protected:

        virtual void createStateLessAttributes( bool force = false );
        virtual Context* priv_CreateContext( Context* parent, std::string rdn );

//         virtual std::string private_getStrAttr( const std::string& rdn,
//                                                 const std::string& def = "",
//                                                 bool getAllLines = false ,
//                                                 bool throwEx = false );
        
    public:

        VirtualSoftlinkContext( const fh_context& parent,
                                const fh_context& target,
                                bool setupEventConnections = true );
        /**
         * A link can have a different local name to its target
         * localname -> target/path/target-filename
         *
         */
        VirtualSoftlinkContext( const fh_context& parent,
                                const fh_context& target,
                                const std::string& localName, // localName -> ctx->getURL()
                                bool setupEventConnections = true );
        
        virtual ~VirtualSoftlinkContext();

        virtual std::string getRecommendedEA();

        /**
         * Either the link target's dirName() or what is set with setLocalName()
         */
        virtual const std::string& getDirName() const;
        virtual std::string getDirPath() throw (FerrisParentNotSetError);
        virtual std::string getURL();
        
        virtual void read( bool force = 0 );
        
        fh_istream getDelegateURLStream( Context* c, const std::string& rdn, EA_Atom* atom );
        fh_istream getFalseStream( Context* c, const std::string& rdn, EA_Atom* atom );
        fh_istream getTrueStream( Context* c, const std::string& rdn, EA_Atom* atom );

        virtual stringset_t& getForceLocalAttributeNames();
//        virtual fh_attribute getAttribute( const std::string& rdn ) throw( NoSuchAttribute );
//        virtual AttributeNames_t& getAttributeNames( AttributeNames_t& ret );
//        virtual int  getAttributeCount();
//         virtual bool isAttributeBound( const std::string& rdn,
//                                        bool createIfNotThere = true
//             ) throw( NoSuchAttribute );
    };
    FERRIS_CTX_SMARTPTR( VirtualSoftlinkContext, fh_VirtualSoftlinkContext );
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /*** MANY TO ONE VIEWS **********************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Context class that accumulates many base contexts and presents then as one
     * directory. For example, a union context would use this to collect data from
     * many directories and present only the union of those directories.
     */
    class FERRISEXP_DLLLOCAL ManyBaseToOneViewContext
        :
        public ChainedViewContext
    {
        typedef ManyBaseToOneViewContext _Self;
        typedef ChainedViewContext       _Base;

        // testing only
        friend class SetIntersectionContext;
        
    protected:

        typedef std::list< fh_context > m_baseContexts_t;
        m_baseContexts_t m_baseContexts;

        bool m_hasSetupBeenCalled;

        /**
         * true if we should cannibalize the underlying views.
         */
        bool m_isCannibal;
        
        /**
         * Keep track of if setup() has been called already
         */
        bool hasSetupBeenCalled();

        /**
         * A method for sublcasses to override. If this function returns true then the
         * context 'c' is inserted into the view by the default OnExists and OnCreated
         * event handlers. The default is true;
         */
        virtual bool shouldInsertContext( const fh_context& c, bool created );

        /**
         * Insert a wrapped version of c into this context's children. Wrap it in the
         * subcontext class type. This method is used in the default event handlers
         * if shouldInsertContext() == true
         *
         * Note that created is set to true if this method was called from OnCreated()
         * so that the correct Inserted/Created event can be fired.
         */
        virtual void cascadedInsert( Context* c, bool created = false ) = 0;

        /**
         * return true if all m_baseContexts are sorted in the same way
         */
        bool allBaseContextsUseSameSorting();

        /**
         * For views that are cannibals we allow them to remove a context from their
         * base views with this method.
         *
         * Subclasses should call updateViewForCannibalizm on themself instead of using
         * this method directly if possible.
         */
        void cannibalRemove( Context* c );

        /**
         * if m_isCannibal is true then this method calls cannibalRemove() on
         * the base contexts for every context that is currently in the view.
         *
         * Most of the time this is the method that subclasses should call
         */
        void updateViewForCannibalizm();

        /**
         * Get the underlying delegate for a given filtered context
         *
         * ie. A filtered context view will make a FilteredContext that sits
         * ontop of the underlying context. We want a pointer to the underlying
         * context so that we can compare two contexts using only their base pointers.
         */
        Context* getUnderlyingContext( fh_context c );

    protected:

        /**
         * diff context uses stateless EA which means it has to use a different
         * constructor to the one we would perfer. The constructor code is in
         * here so that subclasses can call here after they setup stateless EA
         */
        void common_setup( fh_context parent, fh_context ctx, bool isCannibal = false );
        /**
         * If you call this ctor then you need to call common_setup() very soon
         * after to actaully setup the object.
         */
        ManyBaseToOneViewContext( Context* parent, const std::string& rdn );
        
    public:

        /**
         * parent is the parent of ctx, or if we are wrapping a whole tree its the
         * ManyBaseToOneViewContext subclass wrapper for the parent of ctx.
         *
         * @param isCannibal if true and ctx is a ManyBaseToOneViewContext then objects
         *                   that are inserted into this ManyBaseToOneViewContext are
         *                   treated differently, typically be removing them from the
         *                   base ManyBaseToOneViewContext after insertion.
         *                   ie. we Cannibalize the underlying view
         */
        ManyBaseToOneViewContext( fh_context parent, fh_context ctx, bool isCannibal = false );
        virtual ~ManyBaseToOneViewContext();

        /**
         * Add the items in c to those reported as in this dir. Note for files
         * with the same name, the first match is taken as the winner. For example
         * it would be advisable to appendToBaseContexts("~/.whatever") and then
         * appendToBaseContexts( "/mnt/cdrom" ); so that if there are files in the
         * first pushed dir then they are shown in preference to files of the same
         * DirName in the second pushed context.
         */
        void appendToBaseContexts( fh_context c );


        /**
         * When a context is deleted from the base context it also gets removed from the filtering
         * context. Note that the default implementation will only fire the deleted event if
         * the context was in this context's listed children
         */
        void OnDeleted( NamingEvent_Deleted* ev, std::string olddn, std::string newdn );
        
        
        /**
         * Add the newly discovered context to the view. Note that the new context is ignored
         * if shouldInsertContext() returns false.
         */
        void OnExists( NamingEvent_Exists* ev,
                       const fh_context& newc,
                       std::string olddn, std::string newdn );

        /**
         * Add the newly discovered context to the view. Note that the new context is ignored
         * if shouldInsertContext() returns false.
         */
        void OnCreated( NamingEvent_Created* ev,
                        const fh_context& newc,
                        std::string olddn, std::string newdn );

        /**
         * Default implementation relies on setup() to create the view and then just emits
         * synthetic exists events on further read() calls.
         */
        virtual void read( bool force = 0 );



        /**
         * a private method to setup the links for events on this object. Factory
         * methods should worry about and call this, not developers or API users.
         *
         * In subclasses one should override this method and setup the context from
         * the items in m_baseContexts. After adding all relavent contexts call
         * SetupEventConnections() to start monitoring. Note that the default implementation
         * connects to all contexts listed in m_baseContexts.
         *
         * Perform initial setup of items. Note that the caller *MUST* hold a
         * reference to the object for this call to work.
         */
        virtual void setup();
    };
    
    
    
    /**
     * Context subclass that shows the union of one or more base Context
     * objects. If there are conflicts in the subcontext rdn's then the
     * context whos parent was added first using appendToBaseContexts() will be presented
     * and any other contexts sharing that name will be hidden from this view.
     *
     * Note that the ctx argument to the constructor implicitly defines the first
     * backing context, ie. context added first to appendToBaseContexts().
     *
     * @see Factory::MakeUnionContext()
     */
    class FERRISEXP_DLLLOCAL UnionContext
        :
        public ManyBaseToOneViewContext
    {
        typedef UnionContext              _Self;
        typedef ManyBaseToOneViewContext  _Base;
        friend class DiffContext;

        friend fh_context Factory::MakeUnionContext( const fh_context& parent,
                                                     std::list< fh_context > unionContexts );

        
        virtual bool shouldInsertContext( const fh_context& c, bool created );
        virtual void cascadedInsert( Context* c, bool created = false );


        /**
         * If the union is over a bunch of filters that themself are over the same context
         * object UoD then we can perform the initial set union much faster by assuming that
         * if the Context* is the same then the object is the same. This will fail badly if
         * the union is on fh_context objects that are in different directories or when
         * two context objects can have the same rdn.
         */
        bool m_unionIsOverFilteredViewsOnly;

    protected:

        /**
         * Used by DiffContext so it can have stateless EA
         */
        UnionContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
        
    public:

        UnionContext( fh_context parent, fh_context ctx, bool isCannibal = false );

        virtual ~UnionContext();

        /**
         * Disallow and log any attempt to directly create a new context.
         * All other methods should delegate the creation of new subcontexts to
         * the underlying base context and from there the events will inform
         * this context of the creation and we will in turn filter that new
         * context.
         */
        Context*
        priv_CreateContext( Context* parent, std::string rdn )
            {
                LG_CTX_ER << "priv_CreateContext() should never happen" << std::endl;
                return 0;
            }
        virtual void setup();
    };


    /**
     * Context subclass that shows the set_difference() of one or more base Context
     * objects. This is somewhat handy for removing some contexts based on a
     * pre baked pattern.
     *
     * Note that the ctx argument to the constructor implicitly defines the first
     * backing context, ie. context added first to appendToBaseContexts().
     *
     * After the first context added to appendToBaseContexts(), subsequent contexts
     * added with appendToBaseContexts() define contexts whos contents should be used
     * as a set_difference on the first context.
     *
     * @see Factory::MakeSetDifferenceContext()
     */
    class FERRISEXP_DLLLOCAL DifferenceContext
        :
        public ManyBaseToOneViewContext
    {
        typedef DifferenceContext         _Self;
        typedef ManyBaseToOneViewContext  _Base;

        friend fh_context Factory::MakeSetDifferenceContext( const fh_context& parent,
                                                             std::list< fh_context > sdContexts );

        
        virtual bool shouldInsertContext( const fh_context& c, bool created );
        virtual void cascadedInsert( Context* c, bool created = false );
        
    public:

        DifferenceContext( fh_context parent, fh_context ctx, bool isCannibal = false );
        virtual ~DifferenceContext();


        
        /**
         * Disallow and log any attempt to directly create a new context.
         * All other methods should delegate the creation of new subcontexts to
         * the underlying base context and from there the events will inform
         * this context of the creation and we will in turn filter that new
         * context.
         */
        Context*
        priv_CreateContext( Context* parent, std::string rdn )
            {
                LG_CTX_ER << "priv_CreateContext() should never happen" << std::endl;
                return 0;
            }
        virtual void setup();
    };


    /**
     * Context subclass that shows the set_intersection() of one or more base Context
     * objects. 
     *
     * Note that the ctx argument to the constructor implicitly defines the first
     * backing context, ie. context added first to appendToBaseContexts().
     *
     * After the first context added to appendToBaseContexts(), subsequent contexts
     * added with appendToBaseContexts() define contexts whos contents should be used
     * as a set_intersection on the first context.
     *
     * @see Factory::MakeSetIntersectionContext()
     */
    class FERRISEXP_DLLLOCAL SetIntersectionContext
        :
        public ManyBaseToOneViewContext
    {
        typedef SetIntersectionContext    _Self;
        typedef ManyBaseToOneViewContext  _Base;

        friend fh_context Factory::MakeSetIntersectionContext( const fh_context& parent,
                                                               std::list< fh_context > sdContexts,
                                                               bool isCannibal );

        
        virtual bool shouldInsertContext( const fh_context& c, bool created );
        virtual void cascadedInsert( Context* c, bool created = false );
        
    public:

        SetIntersectionContext( fh_context parent, fh_context ctx, bool isCannibal = false );
        virtual ~SetIntersectionContext();


        
        /**
         * Disallow and log any attempt to directly create a new context.
         * All other methods should delegate the creation of new subcontexts to
         * the underlying base context and from there the events will inform
         * this context of the creation and we will in turn filter that new
         * context.
         */
        Context*
        priv_CreateContext( Context* parent, std::string rdn )
            {
                LG_CTX_ER << "priv_CreateContext() should never happen" << std::endl;
            }
        virtual void setup();
    };


    /**
     * Context subclass that shows the set_symmetric_difference() of one or more base Context
     * objects. 
     *
     * Note that the ctx argument to the constructor implicitly defines the first
     * backing context, ie. context added first to appendToBaseContexts().
     *
     * After the first context added to appendToBaseContexts(), subsequent contexts
     * added with appendToBaseContexts() define contexts whos contents should be used
     * as a set_symmetric_difference() on the n-1th context.
     *
     * @see Factory::MakeSetSymmetricDifferenceContext()
     */
    class FERRISEXP_DLLLOCAL SetSymmetricDifferenceContext
        :
        public ManyBaseToOneViewContext
    {
        typedef SetSymmetricDifferenceContext  _Self;
        typedef ManyBaseToOneViewContext       _Base;

        friend fh_context Factory::MakeSetSymmetricDifferenceContext( const fh_context& parent,
                                                                      std::list< fh_context > sdContexts );

        
        virtual bool shouldInsertContext( const fh_context& c, bool created );
        virtual void cascadedInsert( Context* c, bool created = false );
        
    public:

        SetSymmetricDifferenceContext( fh_context parent, fh_context ctx, bool isCannibal = false );
        virtual ~SetSymmetricDifferenceContext();


        
        /**
         * Disallow and log any attempt to directly create a new context.
         * All other methods should delegate the creation of new subcontexts to
         * the underlying base context and from there the events will inform
         * this context of the creation and we will in turn filter that new
         * context.
         */
        Context*
        priv_CreateContext( Context* parent, std::string rdn )
            {
                LG_CTX_ER << "priv_CreateContext() should never happen" << std::endl;
            }
        virtual void setup();
    };


    /**
     * Serves as a simple subclass of ManyBaseToOneViewContext that is used to delegate
     * like a ChainedViewContext but allows other ManyBaseToOneViewContext subclasses
     * to cannibalize the contexts in the view.
     *
     * @see Factory::MakeManyBaseToOneChainedViewContext()
     */
    class FERRISEXP_DLLLOCAL ManyBaseToOneChainedViewContext
        :
        public ManyBaseToOneViewContext
    {
        typedef ManyBaseToOneChainedViewContext _Self;
        typedef ManyBaseToOneViewContext        _Base;

        virtual bool shouldInsertContext( const fh_context& c, bool created );
        virtual void cascadedInsert( Context* c, bool created = false );
        
    public:

        ManyBaseToOneChainedViewContext( fh_context parent, fh_context ctx, bool isCannibal = false );
        virtual ~ManyBaseToOneChainedViewContext();

        /**
         * Disallow and log any attempt to directly create a new context.
         * All other methods should delegate the creation of new subcontexts to
         * the underlying base context and from there the events will inform
         * this context of the creation and we will in turn filter that new
         * context.
         */
        Context*
        priv_CreateContext( Context* parent, std::string rdn )
            {
                LG_CTX_ER << "priv_CreateContext() should never happen" << std::endl;
                return 0;
            }
        virtual void setup();
    };

//     class ManyBaseToOneViewContext_WithStateLessEA_TrySelf
//     {
//     public:
//         AttributeCollection::AttributeNames_t getAttributeNames( Context* c )
//             {
//                 return c->Context::getAttributeNames();
//             }
        
//         fh_attribute getAttribute( Context* c, const std::string& rdn )
//             throw( NoSuchAttribute )
//             {
//                 return c->Context::getAttribute( rdn );
//             }
//     };
    

//     template < class ChildContextClass,
//                class ParentContextClass,
//                class WhatToDoWhenDelegateIsUnboundClass
//                = ManyBaseToOneViewContext_WithStateLessEA_TrySelf >
//     class ManyBaseToOneViewContext_WithStateLessEA
//         :
//         public StateLessEAHolder< ChildContextClass, ParentContextClass >,
//         public ManyBaseToOneViewContext_WithStateLessEA_TrySelf
//     {
//         typedef ManyBaseToOneViewContext_WithStateLessEA _Self;
//         typedef StateLessEAHolder< ChildContextClass, ParentContextClass > _Base;

//     public:

//         ManyBaseToOneViewContext_WithStateLessEA( Context* parent, const std::string& rdn )
//             :
//             _Base( parent, rdn )
//             {}

//         ManyBaseToOneViewContext_WithStateLessEA()
//             :
//             _Base()
//             {}
        
//     protected:
        
//         /********************************************************************************/
//         /********************************************************************************/
//         /********************************************************************************/
//         /*** Handle ability to have local attributes aswell as those of delegate ********/
//         /********************************************************************************/
        
//         virtual std::string private_getStrAttr( const std::string& rdn,
//                                                 const std::string& def = "",
//                                                 bool getAllLines = false ,
//                                                 bool throwEx = false )
//             {
//                 std::string ret = def;
        
//                 try
//                 {
//                     ret = Delegate->private_getStrAttr( rdn, def, getAllLines, true );
//                     return ret;
//                 }
//                 catch( exception& e )
//                 {
//                     if( !isParentBound() )
//                         throw;
//                     return getParent()->private_getStrAttr( rdn, def, getAllLines, throwEx );
//                 }
                
//             }
        
//     public:
        
//         virtual fh_attribute getAttribute( const std::string& rdn ) throw( NoSuchAttribute )
//             {
//             }
        
//         virtual AttributeNames_t getAttributeNames()
//             {
//             }
        
//         virtual int  getAttributeCount()
//             {
//             }
        
//         virtual bool isAttributeBound( const std::string& rdn,
//                                        bool createIfNotThere = true
//             ) throw( NoSuchAttribute )
//             {
//             }
        
//     };
    

    /**
     * Context subclass that shows the diff of two base Context
     * objects. The difference from the first to the second context
     * object added is shown. Interesting attributes such as
     * was-created
     * was-deleted
     * is-same
     * unidiff
     * different-line-count
     * are added to the view and the union of both contexts is presented.
     * Objects that are not in the first context will be shown with was-deleted=1
     * and new objects in the first context wil appear with was-created=1
     *
     * Note that the ctx argument to the constructor implicitly defines the first
     * backing context, ie. context added first to appendToBaseContexts().
     *
     * @see Factory::MakeDiffContext()
     */
    class FERRISEXP_DLLLOCAL DiffContext
        :
        public StateLessEAHolder< DiffContext, UnionContext >
    {
        typedef DiffContext                                    _Self;
        typedef StateLessEAHolder< DiffContext, UnionContext > _Base;

        fh_runner m_runidiff;
        
        
        friend fh_context Factory::MakeDiffContext( const fh_context& parent,
                                                    std::list< fh_context > sdContexts );

        
        virtual bool shouldInsertContext( const fh_context& c, bool created );
        virtual void cascadedInsert( Context* c, bool created = false );

        DiffContext* getParentDiffContext();
        
        /**
         * return true if the context exists in both the new and old
         * dir
         */
        bool       haveTwoContexts( DiffContext* c );

        /**
         * return the context in the first dir 
         */
        fh_context getFirstContext( DiffContext* c );
        /**
         * return the context in the second dir 
         */
        fh_context getSecondContext( DiffContext* c );
        

    protected:

        virtual void createStateLessAttributes( bool force = false );
        virtual void getTypeInfos( std::list< Loki::TypeInfo >& l )
            {
                l.push_back( typeid( _Self ) );
                _Base::getTypeInfos( l );
            }        
    public:

        DiffContext( fh_context parent, fh_context ctx, bool isCannibal = false );
        virtual ~DiffContext();

        virtual std::string getURL();
        
        /**
         * Disallow and log any attempt to directly create a new context.
         * All other methods should delegate the creation of new subcontexts to
         * the underlying base context and from there the events will inform
         * this context of the creation and we will in turn filter that new
         * context.
         */
        Context*
        priv_CreateContext( Context* parent, std::string rdn )
            {
                LG_CTX_ER << "priv_CreateContext() should never happen" << std::endl;
                return 0;
            }
        virtual void setup();

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        static fh_stringstream SL_wasCreated( DiffContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_wasDeleted( DiffContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_isSame( DiffContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_isSameBytes( DiffContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getUniDiff_Native( DiffContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getUniDiff_Remote( DiffContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getUniDiff( DiffContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getDifferentLineCount( DiffContext* c, const std::string& rdn, EA_Atom* atom );

        static fh_stringstream SL_getLinesAddedCount( DiffContext* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getLinesRemovedCount( DiffContext* c, const std::string& rdn, EA_Atom* atom );
        


        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /*** Handle ability to have local attributes aswell as those of delegate ********/
        /********************************************************************************/

        typedef Context _DontDelegateBase;
        
        virtual std::string private_getStrAttr( const std::string& rdn,
                                                const std::string& def = "",
                                                bool getAllLines = false ,
                                                bool throwEx = false );
    public:
        
        virtual fh_attribute getAttribute( const std::string& rdn ) throw( NoSuchAttribute );
        virtual AttributeNames_t& getAttributeNames( AttributeNames_t& ret );
        virtual int  getAttributeCount();
        virtual bool isAttributeBound( const std::string& rdn,
                                       bool createIfNotThere = true
            ) throw( NoSuchAttribute );
        
    };


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

class FERRISEXP_API StaticContentLeafContext
    :
        public StateLessEAHolder< StaticContentLeafContext, leafContext >
{
    typedef StateLessEAHolder< StaticContentLeafContext, leafContext > _Base;
    typedef StaticContentLeafContext _Self;
    
    std::string m_content;

protected:

    virtual fh_istream getIStream( ferris_ios::openmode m = std::ios::in )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               std::exception);
    virtual void createStateLessAttributes( bool force = false );
    
public:

    StaticContentLeafContext( Context* parent,
                              std::string rdn,
                              const std::string& content );
    virtual ~StaticContentLeafContext();
};

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
};
#endif
