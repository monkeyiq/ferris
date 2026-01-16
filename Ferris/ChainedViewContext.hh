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

    $Id: ChainedViewContext.hh,v 1.10 2010/09/24 21:30:25 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_CHAINED_VIEW_CONTEXT_H_
#define _ALREADY_INCLUDED_FERRIS_CHAINED_VIEW_CONTEXT_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <string>

#include <sigc++/sigc++.h>
#include <SmartPtr.h>

#include <TypeDecl.hh>
#include <Ferris.hh>
#include <FerrisEvent.hh>

namespace Ferris
{
    fh_context getUnderlyingContext( fh_context c );
    
    /**
     * Decorator pattern for context objects (GoF book).
     *
     * Create a Context that is just a lightweight facade ontop of a base view.
     * All interesting operations are delegated to the base view, so an object
     * of this class appears just like an object of the base view class.
     *
     * This class is most useful to subclass if one is trying to seem like a
     * context but add a small constraint/feature to that class.
     *
     * Note that the ChainedViewContext class maintains its own items_t collection,
     * so that the items in the view can be different to those in the base context.
     *
     * @see FilteredContext for a new example of this usage.
     *
 */
    class FERRISEXP_API ChainedViewContext
        :
        public Context
    {
        typedef ChainedViewContext _Self;
        typedef Context            _Base;

        friend class Private::CacheManagerContextStateInTime;
        friend class ContextIterator;
        friend fh_context getUnderlyingContext( fh_context c );

        /**
         * This uses getForceLocalAttributeNames() and then adds in some EA names
         * which should always be local. These are things like ferris-delegate-url.
         */
        stringset_t& getAugmentedForceLocalAttributeNames();
        
    protected:
        // TESTING ONLY
        friend class Context;
        friend class ManyBaseToOneViewContext;
        friend class SelectionContext; // test for two time addition of item
        
        fh_context Delegate;
        void setDelegate( fh_context c );

        virtual void createStateLessAttributes( bool force = false );
    public:

        ChainedViewContext( const fh_context& ctx,
                            bool setupEventConnections = true );
        ChainedViewContext( const fh_context& ctx,
                            const std::string& rdn,
                            bool setupEventConnections = true );
        
        
        ChainedViewContext( const fh_context& parent,
                            const fh_context& delegate,
                            bool setupEventConnections = true,
                            bool callSetContextWithDelegateRdn = true );
        ChainedViewContext( const fh_context& parent,
                            const fh_context& delegate,
                            const std::string& rdn,
                            bool setupEventConnections = true );
        virtual ~ChainedViewContext();

        virtual bool getIsNativeContext() const;
        
        /**
         * Mainly used in testing. apps shouldn't have to worry about it.
         */
        fh_context getDelegate();
        
        /**
         * Adjust reference counting and record that this is the root of a
         * chainedView tree. Made public so that factory methods can use it.
         */
        void setIsChainedViewContextRoot();
        
        /**
         * Setup event slots on this object so that the base context signals us when it changes.
         * by default we conncet to 'Delegate' or the user can supply the context to monitor
         * so that ManyBaseToOneViewContext subclasses can connect on many contexts
         *
         * Most of the OnWhatever methods are connected in this method.
         */
        void SetupEventConnections( fh_context c = 0 );

        /**
         * Ensure that SetupEventConnections() has been called if it hasn't already
         * for this context.
         */
        void ensureEventConnections();
    
        virtual void OnDeleted( NamingEvent_Deleted* ev, std::string olddn, std::string newdn );
        virtual void OnExists ( NamingEvent_Exists* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn );
        virtual void OnCreated( NamingEvent_Created* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn );
        virtual void OnChanged( NamingEvent_Changed* ev,
                                std::string olddn, std::string newdn );

        void OnStartReading( NamingEvent_Start_Reading_Context* );
        void OnStopReading ( NamingEvent_Stop_Reading_Context* );
    

        ///////////////////////////////////////////////////////////////////////////////
        //
        // Delegating Attribute Methods.
        //
        ///////////////////////////////////////////////////////////////////////////////
    public:
    
        virtual fh_istream getIStream( ferris_ios::openmode m = std::ios::in );
    
        virtual fh_istream getLocalIStream( std::string& new_dn, ferris_ios::openmode m = std::ios::in );
    
        virtual fh_iostream getIOStream( ferris_ios::openmode m = std::ios::in|std::ios::out );

        virtual Parent_t getParent();
        virtual bool isParentBound();
        virtual const std::string& getDirName() const;
        virtual std::string getDirPath();
        virtual std::string getURL();

        virtual stringset_t& getForceLocalAttributeNames();
        
        virtual fh_attribute getAttribute( const std::string& rdn );
        virtual AttributeNames_t& getAttributeNames( AttributeNames_t& ret );
        virtual int  getAttributeCount();
        virtual bool isAttributeBound( const std::string& rdn,
                                       bool createIfNotThere = true
            );
        
        virtual ref_count_t AddRef();
        virtual ref_count_t Release();
        virtual bool all_attributes_have_single_ref_count();

        ///////////////////////////////////////////////////////////////////////////////
        //
        // Delegating Context Methods.
        //
        ///////////////////////////////////////////////////////////////////////////////
    public:

        virtual fh_attribute createAttribute( const std::string& rdn );

        virtual fh_attribute acquireAttribute( const std::string& rdn );

        virtual fh_context createSubContext( const std::string& rdn, fh_context md = 0 );
        
        virtual fh_context createSubContext( const std::string& rdn, fh_mdcontext md );
    
        virtual fh_context getRelativeContext( const std::string& xdn, RootContextFactory* f = 0 );
        
//     virtual SubContextNames_t getSubContextNames();
//     virtual fh_context getSubContext( const std::string& rdn );
//     bool isSubContextBound( const std::string& rdn );
    
        virtual void read( bool force = 0 );
        virtual long guessSize();

        virtual void dumpOutItems();
        
    protected:

        virtual std::string private_getStrAttr( const std::string& rdn,
                                                const std::string& def = "",
                                                bool getAllLines = false ,
                                                bool throwEx = false );
        virtual Context* priv_CreateContext( Context* parent, std::string rdn );
        
        
    private:
        virtual bool supportsReClaim();
        static fh_stringstream SL_getDelegateURLStream( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getDelegatePathStream( Context* gc, const std::string& rdn, EA_Atom* atom );
        
    };
    FERRIS_SMARTPTR( ChainedViewContext, fh_cvc );
};
#endif
