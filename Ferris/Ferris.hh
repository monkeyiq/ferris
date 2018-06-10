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

    $Id: Ferris.hh,v 1.70 2011/07/31 21:30:49 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/**
 * Define the below to use boost::multi_index instead of std::set<> for
 * storing the parent->child relation.
 */
//#define LIBFERRIS__USE_BOOST_MULTI_INDEX_FOR_STORING_SUBCONTEXTS

/*! \file Ferris.hh
    \brief Main abstraction in ferris.
    
    Check out Context in this file and Attribute in Attribute.hh.
*/

#ifndef _ALREADY_INCLUDED_FERRIS_H_
#define _ALREADY_INCLUDED_FERRIS_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <string>
#include <list>
#include <map>
#include "Ferris/FerrisStdHashMap.hh"
#include <set>
//#include <sstream>
//#include <iostream>


#include <Ferris/Debug.hh>

#include <glib.h>
#include <sigc++/sigc++.h>
#include <sigc++/slot.h>
#include <Functor.h>
#include <SmartPtr.h>
#include <AssocVector.h>

#include <Ferris/TypeDecl.hh>
#include <Ferris/Hashing.hh>
#include <Ferris/ContextSetCompare.hh>

#ifdef LIBFERRIS__USE_BOOST_MULTI_INDEX_FOR_STORING_SUBCONTEXTS
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#endif

namespace Ferris
{
    typedef Loki::Functor< bool, LOKI_TYPELIST_1( const fh_context& ) > fh_matcher;
    typedef std::list<fh_matcher> fh_matchers;
    namespace Factory
    {
        typedef std::list< std::pair< std::string, std::string > > EndingList;
    };
};
#include <Ferris/Resolver.hh>
#include <Ferris/Math.hh>
#include <Ferris/Configuration.hh>

#include <Ferris/FerrisVersioning.hh>

#include <Ferris/Shell.hh>
#include <Ferris/General.hh>
#include <Ferris/Mime.hh>
#include <Ferris/Daemon.hh>

#include <Ferris/SchemaSupport.hh>

namespace Ferris
{
    namespace RDF {};
    namespace redlandea {};
    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
//    std::string errnum_to_string( std::string prefix = "", int en = 0);

};

#include <Ferris/FerrisException.hh>
#include <FerrisLoki/Extensions.hh>
#include <Ferris/FerrisHandle.hh>
#include <Ferris/SignalStreams.hh>
#include <Ferris/FerrisEvent.hh>
#include <Ferris/Enamel.hh>


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

namespace Ferris
{
#ifdef GCC_HASCLASSVISIBILITY
#pragma GCC visibility push(default)
#endif
    
    std::string getIOErrorDescription( fh_istream&  ss );
    std::string getIOErrorDescription( fh_ostream&  ss );
    std::string getIOErrorDescription( fh_iostream& ss );
    std::string getIOErrorDescription( fh_istream&  ss, fh_context c );
    std::string getIOErrorDescription( fh_ostream&  ss, fh_context c );
    std::string getIOErrorDescription( fh_iostream& ss, fh_context c );
    std::string getIOErrorDescription( fh_istream&  ss, const std::string& earl );
    std::string getIOErrorDescription( fh_ostream&  ss, const std::string& earl );
    std::string getIOErrorDescription( fh_iostream& ss, const std::string& earl );
    bool haveIOError( fh_istream&   ss );
    bool haveIOError( fh_ostream&   ss );
    bool haveIOError( fh_iostream&  ss );

    FERRISEXP_API fh_context saveFile( const std::string& parentURL,
                                       const std::string& rdn_raw,
                                       const std::string& byteContent,
                                       bool shouldMonsterName = true,
                                       bool overwrite = false );
    
    FERRISEXP_API std::string getStrAttr( AttributeCollection* c,
                                          const std::string& rdn,
                                          const std::string& def,
                                          bool getAllLines = false,
                                          bool throw_for_errors = false );

    FERRISEXP_API std::string getStrAttr( const fh_context& c,
                                          const std::string& rdn,
                                          const std::string& def,
                                          bool getAllLines = false,
                                          bool throw_for_errors = false );

    FERRISEXP_API std::string getStrAttr( std::string earl,
                                          const std::string& rdn,
                                          const std::string& def,
                                          bool getAllLines = false,
                                          bool throw_for_errors = false );


    FERRISEXP_API fh_context setChild( fh_context c,
                                       const std::string& rdn,
                                       const std::string& v );
    
    FERRISEXP_API std::string setStrAttr( fh_context c,
                                          const std::string& rdn,
                                          const std::string& v,
                                          bool create = false,
                                          bool throw_for_errors = true,
                                          bool dontDelegateToOvermountContext = false );
//     FERRISEXP_API std::string setStrAttr( const char* earl,
//                                           const std::string& rdn,
//                                           const std::string& v,
//                                           bool create = false,
//                                           bool throw_for_errors = true,
//                                           bool dontDelegateToOvermountContext = false );

    
    FERRISEXP_API time_t getTimeAttr( fh_context c,
                                      const std::string& rdn,
                                      time_t v,
                                      bool throw_for_errors = false );

    // New candidates for namespace prefixed attributes
//     std::string& getStrAttrNS( const fh_context& c,
//                                const std::string& ns,
//                                const std::string& rdn,
//                                const std::string& def,
//                                std::string& ret,
//                                bool getAllLines = false,
//                                bool throw_for_errors = false );
    
//     const std::string& setStrAttrNS( fh_context c,
//                                      const std::string& ns,
//                                      const std::string& rdn,
//                                      const std::string& v,
//                                      bool create = false,
//                                      bool throw_for_errors = true );
    
    FERRISEXP_API std::string getStrSubCtx( fh_context c,
                                            std::string subname,
                                            std::string def,
                                            bool getAllLines = false,
                                            bool throw_for_errors = false );
    FERRISEXP_API std::string getStrSubCtx( const std::string& earl,
                                            std::string subname,
                                            std::string def,
                                            bool getAllLines = false,
                                            bool throw_for_errors = false );
    FERRISEXP_API std::string getStrSubCtx( const std::string& earl,
                                            std::string def,
                                            bool getAllLines = false,
                                            bool throw_for_errors = false );

#ifdef GCC_HASCLASSVISIBILITY
#pragma GCC visibility pop
#endif
};

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


#include <Ferris/SchemaManips.hh>
#include <Ferris/Versioned.hh>
#include <Ferris/EAGenerators.hh>

namespace Ferris
{
    class ChainedViewContext;
    
    class FERRISEXP_API CacheManager
    {
    public:
        // Do we need stable iterators?
//        typedef std::set<Context*> freelist_t;
        typedef FERRIS_STD_HASH_SET<Context*,
                                    f_hash<Context* const>,
                                    f_equal_to<Context* const> > freelist_t;
    protected:

        freelist_t  m_freelist;
        freelist_t& getFreeList();
        
//         typedef std::list<Context*> clist_t;
//         clist_t clist;
//         clist_t& getCList();

        int numberOfAllowedPermanentContextsInFreeList;
        int maxNumberOfContextsToFreeAtOnce;
        int maxNumberOfContextsInFreeList;
        bool autoCleanUpCall;
        long m_insideResolveCall;
        bool m_insideCleanupCall;
        
    public:

        CacheManager();
        ~CacheManager();
        
        void dumpFreeListTo( fh_ostream ss );
        int  cleanUp( bool force = true );
        bool shouldAutoCleanUp();

        struct InsideResolve : public Handlable
        {
            CacheManager* cm;
            
            InsideResolve( CacheManager* cm )
                :
                cm( cm )
                {
                    cm->m_insideResolveCall++;
                }
            ~InsideResolve()
                {
                    cm->m_insideResolveCall--;
                }
        };
        FERRIS_SMARTPTR( InsideResolve, fh_insideResolve );
        fh_insideResolve getInsideResolve();

        bool insideCleanupCall();

        int cleanUp_only_CreateMetaDataContext( bool force = false );
    };

    FERRISEXP_API fh_display_aggdata getCachedContextAggregateData( fh_context c, int m=0 );
#ifndef LIBFERRIS_INTERNAL_COMPILING_SWIG_WRAPPER
        FERRISEXP_DLLLOCAL void DepthFirstDelete( CacheManager::freelist_t& fl, Context* cc, bool callReclaimContextObject = true );
        typedef std::list< Context* > cptrlist_t;
        FERRISEXP_DLLLOCAL void DepthFirstDelete( CacheManager::freelist_t& fl, Context* cc, cptrlist_t& l, bool callReclaimContextObject = true );
#endif

    FERRISEXP_API CacheManager* getCacheManager();

    namespace Private
    {
        class CacheManagerImpl;
        FERRISEXP_DLLLOCAL CacheManagerImpl* getCacheManagerImpl();
    };


    /**
     * uses dumpEntireContextListMemoryManagementData to dump data to cerr
     * prefixed with 's' to identify this checkpoint.
     *
     * This should only be used by developers of libferris and core tools.
     */
    FERRISEXP_API void DEBUG_dumpcl( std::string s );

    /**
     * Make a snapshot of all the contexts know and their reference counts
     * and place it into the file filenamePrefix_now().fmemd
     */
    FERRISEXP_API void DEBUG_dumpcl_to_file( const std::string& filenamePrefix );


    template< class T >
    class CacheManaged
    {
    protected:
        
//        CacheManager  OurCacheManager;
    
    public:
        
        Private::CacheManagerImpl* getCacheManager()
            {
                return Private::getCacheManagerImpl();
            }
    };
    
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <Ferris/Attribute.hh>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace Ferris
{

    /*
     * Public interface for a collection of Context objects.
     */
    class FERRISEXP_API ContextCollection
    {
    public:

        /**
         * Prevent bad deletes
         */
        virtual ~ContextCollection() {}
        
        /**
         * STL collection type for getSubContextNames().
         */
        typedef std::list< std::string > SubContextNames_t; 

        /**
     * Get an STL collection of all the rdns that are within this context.
     *
     * @see Attribute::getDirName()
     * @see isSubContextBound()
     * @see getSubContext()
     *
     * @return Collection of the all rdn(s) of each subcontext
     */
        virtual SubContextNames_t& getSubContextNames() = 0;

        /**
     * Check to see if there is a subcontext with the given rdn
     *
     * @see Attribute::getDirName()
     * @see getSubContextNames()
     * @see getSubContext()
     *
     * @param rdn The rdn to look for a subcontext with getDirName() == rdn
     * @return true if there is a subcontext with getDirName() == rdn
     */
        virtual bool isSubContextBound( const std::string& rdn ) = 0;

        /**
     * Get a handle to the subcontext with the given rdn.
     *
     * @see Attribute::getDirName()
     * @see isSubContextBound()
     * @see getSubContextNames()
     *
     * @throws NoSuchSubContext If there is no subcontext with getDirName() == rdn
     * @param rdn The rdn of the subcontext to get
     * @return The context with rdn == getDirName()
     */
        virtual fh_context getSubContext( const std::string& rdn ) throw( NoSuchSubContext ) = 0;

        /**
         * Deprecated, use getSubContextCount() instead.
         *
         * @return Number of subcontexts
         */
        virtual int SubContextCount() = 0;

        /**
         * Get a count of the number of subcontexts.
         *
         * @return Number of subcontexts
         */
        virtual int getSubContextCount()
            {
                return SubContextCount();
            }

        bool empty()
            {
                return getSubContextCount() == 0;
            }
    };

    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    /**
     * Public interface for events that a mutable collection of contexts can fire.
     *
     * Note that these events are fired from the supercontext of the context that
     * they are occuring to. For example if a file "/tmp/test" is changed then the
     * "/tmp/" context will fire a changed event indicating that "test" has changed.
     *
     * @see NamingEvent
     * @see NamingEvent_Changed
     * @see NamingEvent_Deleted
     * @see NamingEvent_Start_Execute
     * @see NamingEvent_Stop_Execute
     * @see NamingEvent_Created
     * @see NamingEvent_Moved
     * @see NamingEvent_Exists
     * @see NamingEvent_Start_Reading_Context
     * @see NamingEvent_Stop_Reading_Context
     */
    class FERRISEXP_API MutableCollectionEvents
    {
    public:

        /**
         * Ensure no slicing
         */
        virtual ~MutableCollectionEvents() 
            {
            }
    
        /**
         * Emitted when a medallion has changed either due to calls by this process
         * to update the medallion addEmblem(), removeEmblem() etc or due to the
         * medallion being changed by another process and this proc having been
         * notified of that change. If this signal is fired due to the medallion
         * changing by another proc then it will be fired after the medallion
         * has been updated in the local address space.
         *
         * @see Medallion::addEmblem()
         * @see Medallion::removeEmblem()
         *
         */
        typedef sigc::signal1< void, fh_context > NamingEvent_MedallionUpdated_Sig_t;
    
        /**
         * Context changed signal type. This signal consists of a special changed
         * object, the olddn and newdn. For this siganl type the olddn and newdn
         * are always identical.
         *
         * What defines a change is context specific, usually this signal means
         * that the contents of that context has changed. You can get the contents
         * of the context using getIStream() or getIOStream(). This signal is also
         * emitted when the metadata of a context has changed.
         *
         * @see getIStream()
         * @see getIOStream()
         *
         * @see getNamingEvent_Changed_Sig()
         * @see NamingEvent
         * @see NamingEvent_Changed
         *
         */
        typedef sigc::signal3< void,
                         NamingEvent_Changed*,
                         std::string, std::string> NamingEvent_Changed_Sig_t;
    

        /**
         * Context deleted signal type.
         *
         * This signal consists of a special deleted
         * object, the olddn and newdn. 
         * For this siganl type the olddn and newdn are always identical.
         *
         * This signal is emitted prior to removing the data from the ferris system.
         * Though the ferris Context system knows about the old context when this
         * signal is emitted, it may already be gone from the underlying VFS code,
         * for example, in a filesystem, the file will no longer exist, so getting
         * exact data, an istream, or some EA may not work as it would prior to
         * the context being deleted or this signal being emitted.
         *
         * @see getNamingEvent_Deleted_Sig()
         * @see NamingEvent
         * @see NamingEvent_Deleted
         *
         */
        typedef sigc::signal3< void,
                         NamingEvent_Deleted*,
                         std::string, std::string> NamingEvent_Deleted_Sig_t;

        /**
         * Context has started being executed signal type.
         *
         * This signal consists of a special start_exe
         * object, the olddn and newdn.
         * For this siganl type the olddn and newdn are always identical.
         *
         * Note that this signal is emitted after the context has started to execute.
         *
         * @see getNamingEvent_Start_Execute_Sig()
         * @see NamingEvent
         * @see NamingEvent_Start_Reading_Context
         * @see NamingEvent_Stop_Reading_Context
         */
        typedef sigc::signal3< void,
                         NamingEvent_Start_Execute*,
                         std::string, std::string> NamingEvent_Start_Execute_Sig_t;

        /**
         * Context has stopped being executed signal type.
         *
         * This signal consists of a special stop_exe
         * object, the olddn and newdn.
         * For this siganl type the olddn and newdn are always identical.
         *
         * Note that this signal is emitted after the context has stopped executing.
         *
         * @see getNamingEvent_Stop_Execute_Sig()
         * @see NamingEvent
         * @see NamingEvent_Start_Reading_Context
         * @see NamingEvent_Stop_Reading_Context
         */
        typedef sigc::signal3< void,
                         NamingEvent_Stop_Execute*,
                         std::string, std::string> NamingEvent_Stop_Execute_Sig_t;
    
        /**
         * Context has been created signal type.
         *
         * This signal consists of a special created
         * object, the olddn and newdn.
         * For this siganl type the olddn and newdn are always identical.
         *
         * This signal is different than NamingEvent_Exists_Sig_t because this signal
         * flags the creation of a new context, whereas NamingEvent_Exists_Sig_t signals
         * the discovery of an already existing context (eg, during a read() operation).
         *
         * Note that this signal is emitted after the context has been created.
         *
         * @see getNamingEvent_Created_Sig()
         * @see NamingEvent
         * @see NamingEvent_Created
         */
        typedef sigc::signal4< void,
                               NamingEvent_Created*,
                               const fh_context&,
                               std::string, std::string> NamingEvent_Created_Sig_t;

        /**
         * Context has been discovered to exist already signal type.
         *
         * This signal consists of a special exists
         * object, the olddn and newdn.
         * For this siganl type the olddn and newdn are always identical.
         *
         * This signal is different than NamingEvent_Created_Sig_t because this signal
         * flags the discovery of an already existing context and not the creation of
         * a new context.
         *
         * @see getNamingEvent_Exists_Sig()
         * @see NamingEvent
         * @see NamingEvent_Exists
         */
        typedef sigc::signal4< void,
                               NamingEvent_Exists*,
                               const fh_context&,
                               std::string, std::string> NamingEvent_Exists_Sig_t;

        /**
         * Context has been moved signal type.
         *
         * This signal consists of a special moved
         * object, the olddn and newdn.
         * For this siganl type the olddn and newdn are different.
         *
         * Note that this signal is emitted after the context has been moved.
         *
         * @see getNamingEvent_Moved_Sig()
         * @see NamingEvent
         * @see NamingEvent_Moved
         */
        typedef sigc::signal3< void,
                         NamingEvent_Moved*,
                         std::string, std::string> NamingEvent_Moved_Sig_t;

        /**
         * Context has started to be read() signal type.
         *
         * This signal is emitted when read() is called.
         *
         * Using this signal a client can setup a progress meter using
         * Context::guessSize() and can connect to the
         * NamingEvent_Stop_Reading_Context_Sig_t to clean up the progress dispaly
         * and indicate read completion.
         *
         * @see getNamingEvent_Start_Reading_Context_Sig()
         * @see getNamingEvent_Stop_Reading_Context_Sig()
         * @see Context::read()
         * @see NamingEvent
         * @see NamingEvent_Start_Reading_Context
         * @see NamingEvent_Stop_Reading_Context
         */
        typedef sigc::signal1< void,
                         NamingEvent_Start_Reading_Context*
                         > NamingEvent_Start_Reading_Context_Sig_t;


        /**
         * Context has finished being read() signal type.
         *
         * This signal is emitted when read() is finished discovering contexts.
         *
         * Using this signal a client can do post read() work.
         *
         * @see getNamingEvent_Start_Reading_Context_Sig()
         * @see getNamingEvent_Stop_Reading_Context_Sig()
         * @see Context::read()
         * @see NamingEvent
         * @see NamingEvent_Start_Reading_Context
         * @see NamingEvent_Stop_Reading_Context
         */
        typedef sigc::signal1< void,
                         NamingEvent_Stop_Reading_Context*
                         > NamingEvent_Stop_Reading_Context_Sig_t;

        /**
         * For contexts that represent network data this event is fired
         * when getIStream() or getIOStream() is called and headers are
         * received over the network.
         *
         * The main use of this signal is to obtain the size/mtime etc info
         * for setting GUIs and download progress meters properly and up front.
         *
         * Spawning children using fcat and specific command line options
         * together with Runner::setAsyncStdOutFunctor() one can have many
         * child processes and read header information from them before they
         * have transfered the entire file.
         *
         * see fcat and ferris-fnews for an example of this usage
         *
         * @see Context::getIStream()
         * @see Context::getIOStream()
         * @see getStrAttr()
         * @see Runner::setAsyncStdOutFunctor()
         *
         * @param fh_context Context that has received headers
         * @param stringset_t collection of the names of header that were obtained.
         *        use getStrAttr( c, set[k] ) to get the values of these keys
         */
        typedef sigc::signal2< void,
                               fh_context, 
                               const stringset_t& > ContextEvent_Headers_Received_Sig_t;

        /**
         * get access to the NamingEvent_MedallionUpdated_Sig_t signal
         * @return Signal
         */
        virtual NamingEvent_MedallionUpdated_Sig_t& getNamingEvent_MedallionUpdated_Sig() = 0;
        
        /**
         * Get access to the NamingEvent_Changed_Sig_t signal.
         * @returns Signal
         */
        virtual NamingEvent_Changed_Sig_t&       getNamingEvent_Changed_Sig() = 0;

        /**
         * Get access to the NamingEvent_Deleted_Sig_t signal.
         * @returns Signal
         */
        virtual NamingEvent_Deleted_Sig_t&       getNamingEvent_Deleted_Sig() = 0;

        /**
         * Get access to the NamingEvent_Start_Execute_Sig_t signal.
         * @returns Signal
         */
        virtual NamingEvent_Start_Execute_Sig_t& getNamingEvent_Start_Execute_Sig() = 0;

        /**
         * Get access to the NamingEvent_Stop_Execute_Sig_t signal.
         * @returns Signal
         */
        virtual NamingEvent_Stop_Execute_Sig_t&  getNamingEvent_Stop_Execute_Sig() = 0;

        /**
         * Get access to the NamingEvent_Created_Sig_t signal.
         * @returns Signal
         */
        virtual NamingEvent_Created_Sig_t&       getNamingEvent_Created_Sig() = 0;

        /**
         * Get access to the NamingEvent_Moved_Sig_t signal.
         * @returns Signal
         */
        virtual NamingEvent_Moved_Sig_t&         getNamingEvent_Moved_Sig() = 0;

        /**
         * Get access to the NamingEvent_Exists_Sig_t signal.
         * @returns Signal
         */
        virtual NamingEvent_Exists_Sig_t&        getNamingEvent_Exists_Sig() = 0;

        /**
         * Get access to the NamingEvent_Start_Reading_Sig_t signal.
         * @returns Signal
         */
        virtual NamingEvent_Start_Reading_Context_Sig_t&
        getNamingEvent_Start_Reading_Context_Sig() = 0;

        /**
         * Get access to the NamingEvent_Stop_Reading_Sig_t signal.
         * @returns Signal
         */
        virtual NamingEvent_Stop_Reading_Context_Sig_t&
        getNamingEvent_Stop_Reading_Context_Sig() = 0;

        /**
         * Get access to the ContextEvent_Headers_Received_Sig_t signal.
         * @returns Signal
         */
        virtual ContextEvent_Headers_Received_Sig_t&
        getContextEvent_Headers_Received_Sig() = 0;
        
        
    };
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    /**
     * Body of Context::StreamIsOpeningHandler() made a global function to limit
     * the scope of some of the friend declarations needed. This function must
     * be a friend of Context, Handlable and Ferris_commonstream.
     */
    class FERRISEXP_DLLLOCAL ContextStreamMemoryManager
    {
        typedef Ferris_commonstream<
            fh_istream::char_type,
            fh_istream::traits_type >::sh_t sh_t;
    public:
        static void StreamIsOpeningHandler( Context* selfp, fh_istream& ss );
        static void StreamIsOpeningHandler( Context* selfp, AttributeProxy* a, fh_istream& ss );
    };

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    namespace Private
    {
        template < typename ResultType, class DownCastContext, class Fun >
        class StatelessFunctorDispatcherHelper 
        {
            Fun fun_;
            ResultType Fire( Context* c, const std::string& rdn, EA_Atom* atom )
                {
                    return fun_( static_cast<DownCastContext*>(c), rdn, atom );
                }
        public:
            StatelessFunctorDispatcherHelper(const Fun& fun) : fun_(fun) {}

            ResultType operator()( Context* c, const std::string& rdn, EA_Atom* atom )
                {
                    return Fire( c, rdn, atom );
                }
        };

        template < typename ResultType, class DownCastContext, class Fun >
        class StatelessClosedFunctorDispatcherHelper 
        {
            Fun fun_;
            ResultType Fire( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
                {
                    return fun_( static_cast<DownCastContext*>(c), rdn, atom, ss );
                }
        public:
            StatelessClosedFunctorDispatcherHelper(const Fun& fun) : fun_(fun) {}

            ResultType operator()( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
                {
                    return Fire( c, rdn, atom, ss );
                }
        };

        template < typename ResultType, class DownCastContext, class Fun >
        class StatelessFunctorDispatcherHelper_PassedInStream
        {
            Fun fun_;
            ResultType Fire( Context* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
                {
                    return fun_( static_cast<DownCastContext*>(c), rdn, atom, ss );
                }
        public:
            StatelessFunctorDispatcherHelper_PassedInStream(const Fun& fun) : fun_(fun) {}

            ResultType operator()( Context* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ss )
                {
                    return Fire( c, rdn, atom, ss );
                }
        };
//         template < typename ResultType, class DownCastContext, class Fun >
//         class StatelessFunctorDispatcherHelper_IO_PassedInStream
//         {
//             Fun fun_;
//             fh_stringstream& Fire( Context* c, const std::string& rdn, EA_Atom* atom, fh_stringstream& ret )
//                 {
//                     return fun_( static_cast<DownCastContext*>(c), rdn, atom, ret );
//                 }
//         public:
//             StatelessFunctorDispatcherHelper_IO_PassedInStream(const Fun& fun) : fun_(fun) {}

//             ResultType operator()( Context* c, const std::string& rdn, EA_Atom* atom )
//                 {
//                     fh_stringstream ret;
//                     Fire( c, rdn, atom, ret );
//                     return ret;
//                 }
//         };
    }

    template <class ChildCtx, class ParentCtx>
    struct StateLessEAHolderTraits
    {
        typedef ParentCtx _Base;
        typedef ParentCtx _ParentCtx;
        typedef ChildCtx  _ChildCtx;



        typedef Loki::Functor< fh_istream,
                               LOKI_TYPELIST_3( Context*,
                                                const std::string&,
                                                EA_Atom* ) > GetIStream_Func_t;
        typedef Loki::Functor< fh_stringstream&,
                               LOKI_TYPELIST_4( Context*,
                                                const std::string&,
                                                EA_Atom*,
                                                fh_stringstream& ) > GetIStream_PassedInStream_Func_t;
        typedef Loki::Functor< fh_iostream,
                               LOKI_TYPELIST_3( Context*,
                                                const std::string&,
                                                EA_Atom* ) > GetIOStream_Func_t;
    typedef Loki::Functor< void,
                           LOKI_TYPELIST_4( Context*,
                                            const std::string&,
                                            EA_Atom*,
                                            fh_istream ) > IOStreamClosed_Func_t;
    
typedef Loki::Functor< fh_istream,
                       LOKI_TYPELIST_3( ChildCtx*,
                                        const std::string&,
                                        EA_Atom* ) > StateLessIEA_t;

typedef Loki::Functor< fh_stringstream&,
                       LOKI_TYPELIST_4( ChildCtx*,
                                        const std::string&,
                                        EA_Atom*,
                                        fh_stringstream& ) > StateLessIEA_PassedInStream_t;

typedef Loki::Functor< fh_iostream,
                       LOKI_TYPELIST_3( ChildCtx*,
                                        const std::string&,
                                        EA_Atom* ) > StateLessIOEA_t;

typedef Loki::Functor< fh_stringstream&,
                       LOKI_TYPELIST_4( ChildCtx*,
                                        const std::string&,
                                        EA_Atom*,
                                        fh_stringstream& ) > StateLessIOEA_PassedInStream_t;

typedef Loki::Functor< void,
                       LOKI_TYPELIST_4( ChildCtx*,
                                        const std::string&,
                                        EA_Atom*,
                                        fh_istream ) > StateLessIOClosedEA_t;
};
    
    struct StateLessAttacher;
    template <class ChildCtx, class ParentCtx = Context>
    class StateLessEAHolder
        :
        public ParentCtx
    {
        friend void attachStatelessSchema( Context* c, const std::string& rawname, XSDBasic_t t );
        friend struct StateLessAttacher;
        
    protected:

        typedef StateLessEAHolder< ChildCtx, ParentCtx >       _Self;
        typedef StateLessEAHolderTraits<ChildCtx,ParentCtx>    _Traits;
        typedef typename _Traits::_Base                 _Base;
        typedef typename _Traits::_ParentCtx            _ParentCtx;
        typedef typename _Traits::_ChildCtx             _ChildCtx;
        typedef typename _Traits::StateLessIEA_t        StateLessIEA_t;
        typedef typename _Traits::StateLessIEA_PassedInStream_t StateLessIEA_PassedInStream_t;
        typedef typename _Traits::StateLessIOEA_t       StateLessIOEA_t;
        typedef typename _Traits::StateLessIOEA_PassedInStream_t       StateLessIOEA_PassedInStream_t;
        typedef typename _Traits::StateLessIOClosedEA_t StateLessIOClosedEA_t;
        typedef typename _Traits::GetIStream_Func_t     GetIStream_Func_t;
        typedef typename _Traits::GetIStream_PassedInStream_Func_t     GetIStream_PassedInStream_Func_t;
        typedef typename _Traits::GetIOStream_Func_t    GetIOStream_Func_t;
        typedef typename _Traits::IOStreamClosed_Func_t IOStreamClosed_Func_t;
        
        bool isStateLessEAVirgin()
            {
                static bool v = true;
                bool ret = v;
                v = false;
                return ret;
            }

        /**
         * Attempt to add a stateless attribute with the name rdn and a function
         * fi that is called whenever the ea value is requested. The ea will be
         * read only.
         *
         * Note that one should consider choosing a value other than the defualt
         * Unknown schema from the XSDBasic_t enumeration so that browsing tools
         * can manipulate the value in a more user friendly way. Note that the sct
         * parameter is augmented as being read only because there is no function to
         * get an iostream for this attribute.
         */
        void
        tryAddStateLessAttribute( const std::string& rdn,
                                  const StateLessIEA_t&  fi,
                                  XSDBasic_t sct = XSD_UNKNOWN )
            {
                try
                {
                    EA_Atom* atx = 0;

                    typedef GetIStream_Func_t FunctorType;
                    typedef Private::StatelessFunctorDispatcherHelper
                        < fh_istream, ChildCtx, StateLessIEA_t > Adapter;
                    
                    atx = new EA_Atom_ReadOnly( FunctorType( Adapter( fi ) ) );
                    this->setAttribute( rdn, atx, false, sct, true );
                    attachStatelessSchema( static_cast<Context*>(this), rdn,
                                           XSDBasic_t(sct | FXDC_READONLY) );
                }
                catch( std::exception& e )
                {
                }
            }
        void
        tryAddStateLessAttributePI( const std::string& rdn,
                                  const StateLessIEA_PassedInStream_t&  fi,
                                  XSDBasic_t sct = XSD_UNKNOWN )
            {
                try
                {
                    EA_Atom* atx = 0;

                    typedef GetIStream_PassedInStream_Func_t FunctorType;
                    typedef Private::StatelessFunctorDispatcherHelper_PassedInStream
                        < fh_stringstream&, ChildCtx, StateLessIEA_PassedInStream_t > Adapter;
                    
                    atx = new EA_Atom_ReadOnly_PassedInStream( FunctorType( Adapter( fi ) ) );
                    this->setAttribute( rdn, atx, false, sct, true );
                    attachStatelessSchema( static_cast<Context*>(this), rdn,
                                           XSDBasic_t(sct | FXDC_READONLY) );
                }
                catch( std::exception& e )
                {
                }
            }

        /**
         * Attempt to add a stateless attribute with the name rdn and a function
         * fi that is called whenever the ea value is requested. The fio function
         * will be called whenever a read/write iostream is requested. the fioc
         * function will be called when the last user reference to an iostream obtained
         * from fio() is dropped.
         * The ea will be read/write depending on which API call the user makes.
         *
         * Note that one should consider choosing a value other than the defualt
         * Unknown schema from the XSDBasic_t enumeration so that browsing tools
         * can manipulate the value in a more user friendly way
         */
        void
        tryAddStateLessAttribute( const std::string& rdn,
                                  const StateLessIEA_t&  fi,
                                  const StateLessIOEA_t& fio,
                                  const StateLessIOClosedEA_t& fioc,
                                  XSDBasic_t sct = XSD_UNKNOWN )
            {
                try
                {
                        EA_Atom* atx = 0;

                        
                        typedef Private::StatelessFunctorDispatcherHelper
                            < fh_istream, ChildCtx, StateLessIEA_t > Adapter_i;
                        typedef Private::StatelessFunctorDispatcherHelper
                            < fh_iostream, ChildCtx, StateLessIOEA_t > Adapter_io;
                        typedef Private::StatelessClosedFunctorDispatcherHelper
                            < void, ChildCtx, StateLessIOClosedEA_t > Adapter_closed;

                        atx = new EA_Atom_ReadWrite(
                            GetIStream_Func_t( Adapter_i( fi ) ),
                            GetIOStream_Func_t( Adapter_io( fio ) ),
                            IOStreamClosed_Func_t( Adapter_closed( fioc ) )
                            );
                        this->setAttribute( rdn, atx, false, sct, true );
                        attachStatelessSchema( static_cast<Context*>(this), rdn, sct );
                }
                catch( std::exception& e )
                {
                }
            }
        void
        tryAddStateLessAttributePI( const std::string& rdn,
                                    const StateLessIEA_PassedInStream_t&  fi,
//                                    const StateLessIOEA_t& fio,
                                    const StateLessIOClosedEA_t& fioc,
                                    XSDBasic_t sct = XSD_UNKNOWN )
            {
                try
                {
                        EA_Atom* atx = 0;

                        typedef Private::StatelessFunctorDispatcherHelper_PassedInStream
                            < fh_stringstream&, ChildCtx, StateLessIEA_PassedInStream_t > Adapter_i;
//                         typedef Private::StatelessFunctorDispatcherHelper_IO_PassedInStream
//                             < fh_iostream, ChildCtx, StateLessIEA_PassedInStream_t > Adapter_io;
                        typedef Private::StatelessClosedFunctorDispatcherHelper
                            < void, ChildCtx, StateLessIOClosedEA_t > Adapter_closed;

                        atx = new EA_Atom_ReadWrite_PassedInStream(
                            GetIStream_PassedInStream_Func_t( Adapter_i( fi ) ),
//                            GetIOStream_Func_t( Adapter_io( fi ) ),
                            IOStreamClosed_Func_t( Adapter_closed( fioc ) )
                            );
                        this->setAttribute( rdn, atx, false, sct, true );
                        attachStatelessSchema( static_cast<Context*>(this), rdn, sct );
                }
                catch( std::exception& e )
                {
                }
            }


        /************************************************************/
        /************************************************************/
        /************************************************************/
        

        /**
         * See the warning about OpenModeCached in
         * Attribute.hh/class EA_Atom_ReadWrite_OpenModeCached
         * please don't use this method unless needed, use the above standard
         * tryAddStateLessAttribute() methods
         */
        void
        tryAddStateLessAttribute_OpenModeCached( const std::string& rdn,
                                                 const StateLessIEA_t&  fi,
                                                 const StateLessIOEA_t& fio,
                                                 const StateLessIOClosedEA_t& fioc,
                                                 XSDBasic_t sct = XSD_UNKNOWN )
            {
                try
                {
                        EA_Atom* atx = 0;

                        typedef Private::StatelessFunctorDispatcherHelper
                            < fh_istream, ChildCtx, StateLessIEA_t > Adapter_i;
                        typedef Private::StatelessFunctorDispatcherHelper
                            < fh_iostream, ChildCtx, StateLessIOEA_t > Adapter_io;
                        typedef Private::StatelessClosedFunctorDispatcherHelper
                            < void, ChildCtx, StateLessIOClosedEA_t > Adapter_closed;

                        atx = new EA_Atom_ReadWrite_OpenModeCached(
                            GetIStream_Func_t( Adapter_i( fi ) ),
                            GetIOStream_Func_t( Adapter_io( fio ) ),
                            IOStreamClosed_Func_t( Adapter_closed( fioc ) )
                            );
                        this->setAttribute( rdn, atx, false, sct, true );
                        attachStatelessSchema( static_cast<Context*>(this), rdn, sct );
                }
                catch( std::exception& e )
                {
                }
            }


        /**
         * Because the content EA maps to getIOStream() on the context,
         * it doesn't need a "closed" function so this class/method was added.
         *
         * Note that in the future if the closed functor can be passed a null value
         * and a operator== exists for functors then we might move to passing a null updator
         */
        void
        tryAddStateLessAttribute_Contents( const std::string& rdn,
                                           const StateLessIEA_t&  fi,
                                           const StateLessIOEA_t& fio,
                                           XSDBasic_t sct = XSD_UNKNOWN )
            {
                try
                {
                        EA_Atom* atx = 0;

                        typedef Private::StatelessFunctorDispatcherHelper
                            < fh_istream, ChildCtx, StateLessIEA_t > Adapter_i;
                        typedef Private::StatelessFunctorDispatcherHelper
                            < fh_iostream, ChildCtx, StateLessIOEA_t > Adapter_io;
                        typedef Private::StatelessClosedFunctorDispatcherHelper
                            < void, ChildCtx, StateLessIOClosedEA_t > Adapter_closed;

                        atx = new EA_Atom_ReadWrite_Contents(
                            GetIStream_Func_t( Adapter_i( fi ) ),
                            GetIOStream_Func_t( Adapter_io( fio ) ) );
                        this->setAttribute( rdn, atx, false, sct, true );
                        attachStatelessSchema( static_cast<Context*>(this), rdn, sct );
                }
                catch( std::exception& e )
                {
                }
            }
        
        
        virtual void getTypeInfos( std::list< Loki::TypeInfo >& l )
            {
                l.push_back( typeid( _Self ) );
                _Base::getTypeInfos( l );
            }
        
    public:

        explicit StateLessEAHolder( Context* parent, const std::string& rdn )
            :
            ParentCtx( parent, rdn )
            {}

        explicit StateLessEAHolder( const fh_context& parent,
                           const fh_context& target,
                           const std::string& localName,
                           bool setupEventConnections )
            :
            ParentCtx( parent, target, localName, setupEventConnections )
            {}
        
        explicit StateLessEAHolder()
            :
            ParentCtx()
            {}

        explicit StateLessEAHolder( const fh_context& delegate )
            :
            ParentCtx( delegate )
            {}
    };
    
};

#include <ContextIterator.hh>
#include <Cache.hh>

namespace Ferris
{
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    struct ContextSetComparePData;
    struct FERRISEXP_DLLLOCAL ContextSetCompare
    {
        ContextSetCompare( fh_sorter s );
        ContextSetCompare( const std::string& s = "name" );
        ContextSetCompare( const ContextSetCompare& ci );
        ContextSetCompare& operator=( const ContextSetCompare& ci );
        ~ContextSetCompare();
        
        bool operator()( const fh_context& s1,
                         const fh_context& s2 ) const;

        // special overload used by Context::ctx_lower_bound()
        bool operator()( const fh_context& s1,
                         const std::string& name ) const;

    private:

        void setup( const std::string& s );
        ContextSetComparePData*const d;
        friend bool operator==( const ContextSetCompare& a, const ContextSetCompare& b );
    };
    bool operator==( const ContextSetCompare& a, const ContextSetCompare& b );
    bool operator!=( const ContextSetCompare& a, const ContextSetCompare& b );
    
    namespace Factory
    {
        /**
         * Create a sorting definition string that gives all the objects in the
         * original in reverse order.
         */
        FERRISEXP_API std::string ReverseSortStringOrder( std::string s );

        /**
         * Construct a preprocessed description of a sorting definition.
         * If you are intending to use a sortng definition many times it is
         * recommended you make a fh_sorter with this method and use the same
         * fh_sorter object in many invocations of MakeSortedContext().
         *
         * Even if your app notices no real speed gain today using these two
         * methods will allow speed gain later with no code changes in your app.
         */
        FERRISEXP_API fh_sorter MakeSorter( const std::string& s );

        /**
         * Make a sorted version of ctx using the sort order defined in 's' and return it.
         *
         * The order spec is either a string describing a sort order $x$ or a list of such
         * strings contained in $()$ brackets.
         *
         * The format for $x$ is the name of the attribute to sort on with an optional $:meta:$
         * prefix for this sort. The meta applies only to the following attribute and
         * can add reverse ordering (!), lazy ordering (L), or explicitly specifiy the
         * type to use for the sort; numeric (#), floating point(FLOAT), case insensitive
         * string (CIS) or version sort (V). Version sort gives the semantics of 
         * ``ls(1) -v'', lazy ordering performs a normal sort and then new objects added
         * to the context are appended to the sort order (much like Microsoft File Explorer).
         *
         * @param ctx underlying context to build sorted context from
         * @param s   sorting order definition
         * @return sorted version of ctx using 's' as sorting definition
         */
        FERRISEXP_API fh_context MakeSortedContext( fh_context& ctx, const std::string& s );

        /**
         * See the other version of MakeSortedContext() for details. The only difference is
         * that 'f' defines the preprocessed version of the sorting definition.
         * 
         * Apart from potentially being much faster for making a lot of sorted contexts
         * this method is logically equal to
         *
         * fh_context ctx    = ...;
         * string     s      = ... my sort ordering string ...;
         * fh_context sorted = MakeSortedContext( ctx, MakeSorter( s ) );
         *
         * @see MakeSorter()
         */
        FERRISEXP_API fh_context MakeSortedContext( fh_context& ctx, fh_sorter& f );
    };


    /**
     * adjust the eaname to one that is preferred to sort on. 
     * For example, given an eaname like size-human-readable this returns "size".
     */
    std::string adjustEANameForSorting( const std::string& s );

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
#ifdef GCC_HASCLASSVISIBILITY
#pragma GCC visibility push(default)
#endif
    
    fh_context SL_SubCreate_alwaysThrow( fh_context c, fh_context md );
    fh_context SL_SubCreate_file( fh_context c, fh_context md );
    fh_context SL_SubCreate_dir( fh_context c, fh_context md );
    fh_context SL_SubCreate_ea( fh_context c, fh_context md );
    fh_context SL_SubCreate_text ( fh_context c, fh_context md );
    fh_context SL_edb_SubCreate_dir( fh_context c, fh_context md );
    fh_context SL_edb_SubCreate_file( fh_context c, fh_context md );
    fh_context SL_ipc_SubCreate_file( fh_context c, fh_context md );
    fh_context SL_ipc_sem_SubCreate_file( fh_context c, fh_context md );
    fh_context SL_db4_SubCreate_dir( fh_context c, fh_context md );
    fh_context SL_db4_SubCreate_file( fh_context c, fh_context md );
    fh_context SL_commondb_SubCreate_dir( fh_context c, fh_context md );
    fh_context SL_commondb_SubCreate_file( fh_context c, fh_context md );
    fh_context SL_SubCreate_commondb ( fh_context c, fh_context md );
    fh_context SL_ldap_SubCreate_context( fh_context c, fh_context md );

#ifdef GCC_HASCLASSVISIBILITY
#pragma GCC visibility pop
#endif
    
    /**
     * Main VFS abstraction. This is the equal of a file/dir in a normal VFS.
     * BEGINCTX BEGINCONTEXT
     */
    class FERRISEXP_API Context
        :
//         public StateLessEAHolder< Context, Attribute >,
//         public AttributeCollection,
        public Attribute,
        public StateLessEAHolder< Context, AttributeCollection >,
        public ContextCollection,
        public MutableCollectionEvents,
        public CacheManaged<Context>
    {
        typedef Context _Self;
        
#ifndef LIBFERRIS_INTERNAL_COMPILING_SWIG_WRAPPER
        friend std::string monsterName( const fh_context& c, const std::string& rdn );
#endif
        friend class Private::CacheManagerContextStateInTime;
        friend fh_context Resolve( const std::string dirty_earl, ResolveStyle rs );
        friend class RemembranceTopLevelContext;
        friend class HalRootContext;
        friend class childContext;
        friend class NameMonsteringPolicy_AppendNumber;
        friend class SortedContext;
        friend class CachedContext;
        friend class FilteredContext;
        friend class InheritingEAContext;
        friend class DiffContext;
        friend class ManyBaseToOneViewContext;
        friend class externalContext;
        friend class SqlPlusTupleContext;
        friend class RootContextFactory;
        friend class NativeContext;
        friend class SelectionFactoryContext;
        friend class ChainedViewContext;
        friend class FerrisInternal;
        friend class RPMContext;
        friend class RPMRootContext;
        friend class RPMPackageContext;
        friend class EmblemCommonCreator;
        friend class RedlandRootContext;
        friend class RedlandStatementContext;
        friend class FerrisGPGSignaturesInternalContext;
        friend class FerrisBranchInternalContext;
        friend class DelayedCommitParentContext;
        friend class recordfileContext;
        
        typedef StateLessEAHolder< Context, AttributeCollection > SL;
#define FERRIS_LS FERRISEXP_DLLLOCAL static
#define FERRIS_ES FERRISEXP_DLLLOCAL static
        FERRIS_LS fh_istream SL_getSubContextCountStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_istream SL_getAttributeCountStream( Context* c, const std::string& rdn, EA_Atom* attr );
//        friend fh_istream SL_getAttributeCountStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRISEXP_API static fh_istream SL_getDirNameStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRISEXP_API static fh_iostream SL_getDirNameIOStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS void SL_RenameContext( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss );
        FERRIS_LS fh_istream SL_getParentDirNameStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_istream SL_getParentURLStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_istream SL_getDirNameExtensionStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_istream SL_getRecommendedEAStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_istream SL_getRecommendedEAShortStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_istream SL_getRecommendedEAUnionStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_istream SL_getRecommendedEAUnionViewStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS void recommendedEAUnionViewAdd( std::set<std::string>& theSet, Context* c );
        
        FERRIS_LS fh_istream SL_getEANamesStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_istream SL_getRecursiveSubcontextCountStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_iostream SL_getRecursiveSubcontextCountIOStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS void SL_FlushAggregateData( Context* c, const std::string& rdn, EA_Atom* atom,
                                           fh_istream ss );
        FERRIS_LS fh_iostream SL_getRecursiveSubcontextMaxDepthIOStream( Context* c, const std::string& rdn, EA_Atom* atom );
        
        FERRIS_LS fh_istream SL_getXTimeCTimeStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_stringstream getTimeStrFTimeStream( Context* c, const std::string& rdn, EA_Atom* atom );
//        FERRIS_LS fh_istream SL_getTimeStrFTimeIStream( Context* c, const std::string& rdn, EA_Atom* atom );
    protected:
        FERRIS_LS fh_stringstream SL_getTimeStrFTimeIOStream( Context* c, const std::string& rdn, EA_Atom* atom );
    private:
        FERRIS_LS fh_istream SL_getXTimeDayGranularityStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_istream SL_getXTimeMonthGranularityStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_istream SL_getXTimeYearGranularityStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_istream SL_getSizeFromContentIStream( Context* c, const std::string& rdn, EA_Atom* atom );

        FERRIS_LS void SL_setTimeStrFTimeStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss );
        FERRIS_LS fh_istream SL_getSizeHumanReadableStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_istream SL_getContentIStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_istream SL_getIsRemote( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_iostream SL_getAsJSON( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_iostream SL_getAsXML( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_istream  SL_getAsText( Context* c, const std::string& rdn, EA_Atom* atom );

        FERRIS_LS fh_stringstream SL_getIsActiveView( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_stringstream SL_getForcePassiveView( Context* c, const std::string& rdn, EA_Atom* atom );

        FERRIS_LS fh_stringstream SL_getIsDir( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_stringstream SL_getIsDirTryAutoMounting( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_stringstream SL_getFerrisCTimeStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_stringstream SL_getOnlyFerrisMetadataCTimeStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_stringstream SL_getFerrisShouldReindexIfNewerStream( Context* c, const std::string& rdn, EA_Atom* atom );
        
        FERRIS_LS fh_istream SL_getFollowLinkAsFerrisFSSizeStream( Context* c, const std::string& rdn, EA_Atom* atom );
        
        FERRIS_LS fh_stringstream SL_hasEmblem( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_stringstream SL_hasEmblemFuzzy( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS void SL_hasEmblemStreamClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss );
        FERRIS_LS fh_stringstream SL_getEmblemTime( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_stringstream SL_getEmblemList( int cutoff, Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_stringstream SL_getEmblemListAll( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_stringstream SL_getEmblemListDefault( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_stringstream SL_getEmblemUpset( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_stringstream SL_getHasMedallion( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_stringstream SL_getLanguageHuman( Context* c, const std::string& rdn, EA_Atom* atom );

    protected:
        FERRIS_LS fh_stringstream SL_getNothingStream( Context* c,const std::string& rdn, EA_Atom* atom );
        FERRIS_LS fh_stringstream SL_getStreamWithNumberOneStream( Context* c,const std::string& rdn, EA_Atom* atom );
        FERRIS_LS void SL_setNothingStream( Context* c,const std::string& rdn, EA_Atom* atom, fh_istream iss );

        

#undef FERRIS_ES        
#undef FERRIS_LS        

    protected:
        FERRISEXP_API static fh_istream SL_getHasSubContextsGuessStream( Context* c, const std::string& rdn, EA_Atom* atom );

        template <class Remembrance>
        FERRISEXP_API static fh_iostream SL_getIsUnRemembered( Context* c, const std::string& rdn, EA_Atom* atom );
        template <class Remembrance>
        FERRISEXP_API static void SL_setIsUnRemembered(
            Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss );
        template <class Remembrance>
        FERRISEXP_API static fh_istream SL_getLastRememberedSubContexts( Context* c, const std::string& rdn, EA_Atom* atom );
        template <class Remembrance>
        FERRISEXP_API static fh_istream SL_getRemembranceTime( Context* c, const std::string& rdn, EA_Atom* atom );
        template <class Remembrance>
        FERRISEXP_API static fh_iostream SL_getRemembranceCommand( Context* c, const std::string& rdn, EA_Atom* atom );
        template <class Remembrance>
        FERRISEXP_DLLLOCAL static void
        SL_setRemembranceCommand( Context* c,
                               const std::string& rdn,
                               EA_Atom* atom, fh_istream ss );
        
        
    private:
#ifndef LIBFERRIS_INTERNAL_COMPILING_SWIG_WRAPPER
        friend FERRISEXP_API fh_display_aggdata getCachedContextAggregateData( fh_context c, int m );
#endif
        
        /************************************************************/
        /************************************************************/
        /************************************************************/
        /*** These two are used by Context to augment a list of *****/
        /*** the EA names which are stateless for this object type **/
        /************************************************************/
        /************************************************************/
    public:
        static stringset_t& getContextClassStatelessEANames();
    private:

        
        void
        ContextClass_SLEA( const std::string& rdn,
                           const StateLessIEA_t&  fi,
                           XSDBasic_t sct = XSD_UNKNOWN );
        void
        ContextClass_SLEA( const std::string& rdn,
                           const StateLessIEA_t&  fi,
                           const StateLessIOEA_t& fio,
                           const StateLessIOClosedEA_t& fioc,
                           XSDBasic_t sct = XSD_UNKNOWN );
        
        /******************************************************************************/
        /******************************************************************************/
        /******************************************************************************/
        /******************************************************************************/

         typedef FERRIS_STD_HASH_MAP< Context* const,
                               time_t,
                               f_hash<Context* const>,
                               f_equal_to<Context* const> > s_downloadMTimeSince_t;
//        typedef std::hash_map< Context* const, time_t > s_downloadMTimeSince_t;
        s_downloadMTimeSince_t& s_downloadMTimeSince();

        FERRISEXP_DLLLOCAL static fh_istream
        SL_getFerrisCurrentTimeIStream( Context* c,
                                        const std::string& rdn,
                                        EA_Atom* atom );
        
        FERRISEXP_DLLLOCAL static fh_istream
        SL_getDownloadIfMTimeSinceIStream( Context* c,
                                           const std::string& rdn,
                                           EA_Atom* atom );
        FERRISEXP_DLLLOCAL static fh_iostream
        SL_getDownloadIfMTimeSinceIOStream( Context* c,
                                            const std::string& rdn,
                                            EA_Atom* atom );
        FERRISEXP_DLLLOCAL static void
        SL_downloadIfMTimeSinceClosed( Context* c,
                                       const std::string& rdn,
                                       EA_Atom* atom, fh_istream ss );

    protected:

        bool   testDownloadIfMTimeSince( time_t mtime = 0, bool force = true );
        
        time_t getDownloadIfMTimeSince();
        void   setDownloadIfMTimeSince( time_t x );

    private:

        /**
         * Stateless EA generator that calls getAsRDFXML() to get an RDF/XML file
         * for this context.
         */
        static fh_iostream SL_getAsRDF( Context* c, const std::string& rdn, EA_Atom* atom );
        
    protected:
        /**
         * Place the RDF/XML for this context into the given stream. This method
         * is not ment to be recursive, ie. its purpose is to export the context's
         * EA as an RDF/XML document but not to include the children contexts info.
         */
        virtual fh_stringstream& getAsRDFXML( fh_stringstream& ss );

        /**
         * some clients will need to allow a "refresh" button on contexts that
         * are difficult or expensive to have active hot views for. If the
         * viewer should call read(true) to get the latest view then this
         * will return true. Note that this is is different to the module
         * actually supporting active views as would be the case just using
         * supportsMonitoring(), this tells if for a given context active
         * viewing is actually occuring. For example, a plugin may support
         * active viewing using a high level network API but have a fallback
         * to polling mode if that API is not present. A more interesting example
         * is when the user has explicitly blocked active views on the URL,
         * such as blocking updates on /tmp
         *
         * @see supportsMonitoring()
         * @see getForcePassiveView()
         * @see is-active-view EA
         */
        virtual bool isActiveView();

        /**
         * Some contexts that can possibly be active are forced to be passive.
         * An example of this would be /tmp which is expensive to monitor due
         * to many users adding/changing/removing files constantly.
         *
         * @see isActiveView()
         * @see force-passive-view EA
         */
        bool getForcePassiveView();

    public:
        /**
         * some testing to see if this is context has children.
         * The default impl checks the mimetype and then sees if begin()==end()
         */
        virtual bool isDir();
        
    private:
        /******************************************************************************/
        /******************************************************************************/
    protected:

        /*
         * Overload find() so that subclasses can use the generic version unqualified.
         */
        template<class InputIterator, class EqualityComparable>
        InputIterator find(InputIterator first, InputIterator last,
                           const EqualityComparable& value)
            {
                return std::find( first, last, value );
            }
        
//         struct ctx_idx_getimpl : public std::unary_function< const fh_context&, const Context* >
//         {
//             inline const Context* operator()( const fh_context& s ) const
//                 {
//                     return GetImpl(s);
//                 }
//         };
    public:
        struct ITEMS_T_BY_NAME_UNORDERED_TAG  {};
        struct ITEMS_T_BY_NAME_ORDERED_TAG   {};
        
        virtual const std::string& getDirName() const;
    protected:
        
        /**
         * Note that Items_t must provide
         * 1) fast lookup by key (rdn)
         * 2) stable iterators over insert() and erase() for ContextIterator
         * 3) sorted order for ContextIterator to use lower_bound() when the
         *    item the iterator points at is deleted from under the iterator
         */
#ifdef LIBFERRIS__USE_BOOST_MULTI_INDEX_FOR_STORING_SUBCONTEXTS
        typedef boost::multi_index::multi_index_container<
            fh_context,
            boost::multi_index::indexed_by<
                
                boost::multi_index::ordered_unique<
                    boost::multi_index::tag<ITEMS_T_BY_NAME_ORDERED_TAG>,
                    boost::multi_index::identity<fh_context>, ContextSetCompare >,

                boost::multi_index::hashed_unique<
                    boost::multi_index::tag<ITEMS_T_BY_NAME_UNORDERED_TAG>,
                    boost::multi_index::const_mem_fun<Context,const std::string&,&Context::getDirName> >
                
//            boost::multi_index::hashed_unique< ctx_idx_getimpl >
        > 
        > Items_t;
        typedef Items_t::index<ITEMS_T_BY_NAME_UNORDERED_TAG>::type Items_By_Name_Hashed_t;
        typedef Items_t::index<ITEMS_T_BY_NAME_ORDERED_TAG>::type Items_By_Name_Ordered_t;
#else
        typedef std::set< fh_context, ContextSetCompare > Items_t;
#endif

        /**
         * Works like set::lower_bound() except it has to play tricks
         * because set::lower_bound() expects a key_type to find, not a string
         */
        Items_t::iterator ctx_lower_bound( Items_t& items, const std::string& rdn );
        
        
    private:

        fh_context CoveredContext;
        fh_context ParentContext;

        fh_display_aggdata AggregateData;
        
        std::string DirName;
    protected:
        void setContext( fh_context parent, const std::string& rdn );

        int getReadingDir() const;

        fh_context ThisContext();

#ifndef LIBFERRIS_INTERNAL_COMPILING_SWIG_WRAPPER
        FERRISEXP_API virtual std::string private_getStrAttr( const std::string& rdn,
                                                              const std::string& def = "",
                                                              bool getAllLines = false ,
                                                              bool throwEx = false );
#endif
        
        friend std::string Shared_getFSArgs( Context* ctx );
        /**
         * Get the first context in the transitive parents of this context
         * which is of a given Context subclass. This is different to getBaseContext()
         * because we skip all context parents which are not a subclass or the
         * context class sought.
         *
         * @see getBaseContext()
         */
        template< class DesiredContextClass >
        DesiredContextClass* getFirstParentOfContextClass( DesiredContextClass* dummy,
                                                           Context* parentOverRide = 0 )
            {
                if( this->getOverMountContext() != this )
                {
                    return this->getOverMountContext()->getFirstParentOfContextClass<>( dummy );
                }

                Context* c = this;
                if( parentOverRide )
                    c = parentOverRide;
                
                while( c && c->isParentBound() )
                {
                    Context* p = c->getParent()->getOverMountContext();
                    if(DesiredContextClass* dc = dynamic_cast<DesiredContextClass*>( p ))
                    {
                        return dc;
                    }
                    else
                    {
                        c = p;
                    }
                }
                return 0;
            }

        /**
         *
         * @see getFirstParentOfContextClass()
         */
        template < class ChildContextClass >
        ChildContextClass* getBaseContext()
            {
                if( this->getOverMountContext() != this )
                {
                    ChildContextClass* c = dynamic_cast<ChildContextClass*>(
                        this->getOverMountContext());
                    return c->template getBaseContext<ChildContextClass>();
                }

                
                ChildContextClass* c = dynamic_cast<ChildContextClass*>(this);
                while( c && c->isParentBound() )
                {
                    Context* p = c->getParent()->getOverMountContext();
                    if(ChildContextClass* nextc = dynamic_cast<ChildContextClass*>( p ))
                    {
                        c = nextc;
                    }
                    else
                    {
                        return c;
                    }
                }
                return c;
            }
        
        
    public:

        virtual Ferris::Attribute::Parent_t getParent() throw (FerrisParentNotSetError);
        virtual bool isParentBound();
        
        virtual std::string getRecommendedEA();
        virtual bool getHasSubContextsGuess();
        
        int getHaveReadDir() const;
        fh_attribute toAttribute()
            {
                return getAttribute(".");
            }

        /**
         * For resolution types like xpath:// we are really interested in the resulting
         * children, not the top level context that is returned. For such contexts the
         * parent (eg. selection://) is replaced inline with all of its children.
         *
         * All such unrolling should be done with this method if possible because of
         * future unrolling types that may be added
         */
        template< class ColT, class IterT >
        static ColT& UnrollQueryResultContexts( ColT& col, IterT be )
            {
                ColT result;
                
                for( IterT ci = be; ci != col.end(); ++ci )
                {
                    fh_context c = *ci;

                    while( true )
                    {
                        if( c->CoveredContext )
                        {
                            c = c->CoveredContext;
                            continue;
                        }
                        if( c->isParentBound() )
                        {
                            c = c->getParent();
                            continue;
                        }
                        break;
                    }

                    if( c )
                    {
                        std::string earl = c->getURL();
//                        cerr << "2d top:" << earl << endl;
                        if( starts_with( earl, "selectionfactory:" ))
                        {
                            for( Context::iterator child = (*ci)->begin();
                                 child!=(*ci)->end(); ++child )
                            {
                                result.push_back( *child );
                            }
                        }
                        else
                        {
                            result.push_back( *ci );
                        }
                    }
                }
                col.clear();
                col = result;
                return col;
            }
                
        
        /**
         * Get the schema for the given ea. Note that this should be the
         * same as Resolve(getStrAttr( this, "schema:" + eaname, "" ));
         * Also note that schema attributes may have schemas themself
         * ie. something like schema:schema:size will tell you that
         * the ea schema:size is indeed a schema and return some generic
         * information about schemas.
         *
         * There are two main kinds of schema that can be attached to
         * a context
         *
         * (a) for each EA an individual schema URL can be attached
         *
         * (b) for each context a schema conforming to the RelaxNG
         * template may be attached which describes the structure of
         * the subtree bound at that context.
         *
         * However the schemas are bound behind the scenes this method
         * will return the context which represents the ea (or for "." 
         * the context itself).
         *
         * @param eaname the name of ea or "." for the context itself.
         */
        virtual fh_context getSchema( const std::string& eaname );

        /**
         * Similar to getSchema except a default type is passed in and
         * will be returned if there is no explicit schema bound.
         *
         * @see getSchema()
         */
        fh_context getSchemaOrDefault( const std::string& eaname,
                                       XSDBasic_t sct );
        

    private:

        friend struct ContextIteratorData_OldRdnSet;
        friend struct ContextIteratorData;
        friend class  ContextIterator;
        friend bool operator<(const ContextIterator& x, const ContextIterator& y);
        friend ContextIterator::difference_type
        operator-(const ContextIterator& x, const ContextIterator& y);
        friend struct contextset_less_created_or_existed;
        

        ContextDirOpVersion_t DirOpVersion;
        ContextDirOpVersion_t getDirOpVersion();
    protected:
        virtual Items_t& getSortedItems(); // see .cpp file for doco
    public:
        /**
         * Standard STL begin()/end() for iterating over children
         */
        typedef ContextIterator iterator;
        typedef std::reverse_iterator<ContextIterator> reverse_iterator;
        Context::iterator begin();
        Context::iterator end();
        Context::reverse_iterator rbegin();
        Context::reverse_iterator rend();
        Context::iterator find( const std::string& rdn );
        
    private:
        
        /* Bitfield of flags */
        guint32
/**/         Dirty:1,
/**/         AttributesHaveBeenCreated:1,
/**/         HaveDynamicAttributes:1,
/**/         updateMetaData_First_Time:1,
/**/         ensureUpdateMetaDataCalled_virgin:1,
/**/         ReadingDir:1,
/**/         HaveReadDir:1,
/**/         FiredStartReading:1,
/**/         WeAreInFreeList:1,
/**/         EAGenFactorys_isVirgin:1,
/**/         SubContextNamesCacheIsValid:1,    //< ==1 if the cache of contextnames is valid
/**/         ContextWasCreatedNotDiscovered:1, //< ==1 If context was created after read()
/**/         HasBeenDeleted:1,                 //< ==1 when user has references but data on fs is gone.
/**/         m_overMountAttemptHasAlreadyFailed:1, //< ==1 when tryToOverMount() has already failed for this context
/**/         m_isNativeContext:1, //< cache which is true if we are a native context (file://)
/**/         m_ChainedViewContext_Called_SetupEventConnections:1,
/**/         m_tryToGetImplicitTreeSmushHasFailed_forDirectory:1, //< No implicit tree smushes for context
/**/         m_tryToGetImplicitTreeSmushHasFailed_forURL:1,       //< Attempt to implicitly find uuidnode has failed
/**/         m_overMountAttemptHasAlreadyFailedEAOnly:1, //< Overmounting for EA is less powerful than for content.
/**/         m_forcePassiveViewCache:1, //< A cache used in getForcePassiveView()
/**/         m_forcePassiveViewCacheIsValid:1, //< if !true recalculate m_forcePassiveViewCache
/**/         m_holdingReferenceToParentContext:1; //< true if we have called getParent()->AddRef()
        
        friend struct tryToGetUUIDNode_StringARM;
        friend struct Semantic::tryToGetUUIDNode_StringARM;
        friend class FCA::DatabaseResultSetContext;
        
    protected:

        void setAttributesHaveBeenCreated()
            {
                AttributesHaveBeenCreated = 1;
            }
        
        
        class ReadingDirRAII 
        {
            Context* m_c;
            bool m_cache;
            NOT_COPYABLE( ReadingDirRAII );
        public:
            explicit ReadingDirRAII( Context* c, bool tmpState )
                :
                m_c( c ),
                m_cache( c->ReadingDir )
                {
                    if( m_c )
                        m_c->ReadingDir = tmpState;
                }
            ~ReadingDirRAII()
                {
                    if( m_c )
                        m_c->ReadingDir = m_cache;
                }
        };
        friend class ReadingDirRAII;
        
        void setHasBeenDeleted( bool v )
            {
                LG_CTX_D << "Context::setHasBeenDeleted() v:" << v << std::endl;
                if( HasBeenDeleted && !v )
                {
                    LG_CTX_D << "Context::setHasBeenDeleted() url:" << getURL()
                             << " pb:" << isParentBound()
                             << std::endl;
                    
                    if( isParentBound() )
                    {
                        LG_CTX_D << "Context::setHasBeenDeleted() url:" << getURL()
                                 << " pnsc:" << getParent()->NumberOfSubContexts
                                 << std::endl;
                    
                        getParent()->NumberOfSubContexts++;
                        LG_CTX_D << "Context::setHasBeenDeleted() url:" << getURL()
                                 << " pnsc:" << getParent()->NumberOfSubContexts
                                 << std::endl;
                    }
                }
                HasBeenDeleted = v;
            }

        class FERRISEXP_API EnsureStartStopReadingIsFiredRAII
        {
            NOT_COPYABLE( EnsureStartStopReadingIsFiredRAII );
        protected:
            Context* m_c;
        public:
            explicit EnsureStartStopReadingIsFiredRAII( Context* c );
            ~EnsureStartStopReadingIsFiredRAII();
        };

        class FERRISEXP_API emitExistsEventForEachItemRAII : public EnsureStartStopReadingIsFiredRAII
        {
            NOT_COPYABLE( emitExistsEventForEachItemRAII );
        public:
            explicit emitExistsEventForEachItemRAII( Context* c );
            ~emitExistsEventForEachItemRAII();
        };

        class FERRISEXP_API staticDirContentsRAII : public EnsureStartStopReadingIsFiredRAII
        {
            NOT_COPYABLE( staticDirContentsRAII );
        public:
            explicit staticDirContentsRAII( Context* c );
            ~staticDirContentsRAII();
        };
        
        friend class EnsureStartStopReadingIsFiredRAII;

        void priv_ensureSubContext_helper_ins( fh_context c, bool created );

        template < class SubT >
        SubT* priv_ensureSubContext( const std::string& rdnconst )
            {
                SubT* ret = 0;
                ret = priv_ensureSubContext( rdnconst, ret );
                return ret;
            }

        template < class SubT >
        SubT* priv_ensureSubContext( const std::string& rdnconst,
                                     bool shouldMonsterName,
                                     bool created )
            {
//                std::cerr << "priv_ensureSubContext() rdn:" << rdnconst << " created:" << created << std::endl;
                SubT* ret = 0;
                ret = priv_ensureSubContext( rdnconst, ret, shouldMonsterName, created );
                return ret;
            }
    public:
        
        template < class SubT >
        SubT* priv_ensureSubContext( const std::string& rdnconst,
                                     SubT*,
                                     bool shouldMonsterName = false,
                                     bool created = false )
            {
                std::string rdn = rdnconst;
                
                try
                {
                    LG_CTX_D << "Context::priv_readSubContext(1) url:" << getURL() << std::endl;
                    
                    {
                        Items_t::iterator subc_iter;
                        bool isBound = priv_isSubContextBound( rdn, subc_iter );
                        LG_CTX_D << "Context::priv_readSubContext(2) url:" << getURL() << std::endl;
                        if( isBound )
                        {
                            LG_CTX_D << "Context::priv_readSubContext(2.1) url:" << getURL()
                                     << " iterHasBeenDeleted:" << (*subc_iter)->HasBeenDeleted
                                     << std::endl;
                        }
                        
                        if( shouldMonsterName && isBound && !(*subc_iter)->HasBeenDeleted )
                        {
                            rdn = this->monsterName( rdn );
                            isBound = priv_isSubContextBound( rdn, subc_iter );
                            LG_CTX_D << "Context::priv_readSubContext(3) url:" << getURL() << std::endl;
                        }
                        if( isBound )
                        {
                            fh_context ret = *subc_iter;
                            LG_CTX_D << "Context::priv_readSubContext(4) url:" << getURL()
                                     << " ret->HasBeenDeleted:" << ret->HasBeenDeleted
                                     << " nsc:" << NumberOfSubContexts
                                     << " ret:" << toVoid( ret )
                                     << " retp:" << ret->getParent()
                                     << " this:" << this
                                     << std::endl;
                            ret->setHasBeenDeleted( false );
                            LG_CTX_D << "Context::priv_readSubContext(5) url:" << getURL()
                                     << " HasBeenDeleted:" << HasBeenDeleted
                                     << " nsc:" << NumberOfSubContexts
                                     << " ret:" << toVoid( ret )
                                     << " retp:" << ret->getParent()
                                     << " this:" << this
                                     << std::endl;
                            if( created )
                                Emit_Created( 0, ret, rdn, rdn, 0 );
                            else
                                Emit_Exists( 0, ret, rdn, rdn, 0 );
                            return dynamic_cast<SubT*>(GetImpl(ret));
                        }
                    }

                    LG_CTX_D << "Context::priv_readSubContext(create) url:" << getURL() << std::endl;
                    SubT* c = new SubT( this, rdn );
                    priv_ensureSubContext_helper_ins( c, created );
                    return c;
                }
                catch( NoSuchSubContext& e )
                {
                    std::cerr << "Context::priv_readSubContext() e:" << e.what() << std::endl;
                    throw e;
                }
                catch( FerrisNotSupportedInThisContext& e )
                {
                    std::cerr << "Context::priv_readSubContext() e:" << e.what() << std::endl;
                    throw e;
                }
                catch( std::exception& e )
                {
                    std::cerr << "Context::priv_readSubContext() strange e:" << e.what() << std::endl;
                    BackTrace();
                    throw e;
                }
             }
        
    protected:

        template < class SubT, class Creator >
        SubT* priv_ensureSubContext( const std::string& rdn,
                                     SubT*,
                                     const Creator& creator,
                                     bool created = false )
            {
                try
                {
                    LG_CTX_D << "Context::priv_readSubContext(1) url:" << getURL() << std::endl;
                    
                    {
                        Items_t::iterator subc_iter;
                        if( priv_isSubContextBound( rdn, subc_iter ) )
                        {
                            fh_context c = *subc_iter;
                            c->setHasBeenDeleted( false );
                            if( created )
                                Emit_Created( 0, c, rdn, rdn, 0 );
                            else
                                Emit_Exists( 0, c, rdn, rdn, 0 );
                            SubT* ret = dynamic_cast<SubT*>(GetImpl(c));
                            creator.setupExisting( ret );
                            return ret;
                        }
                    }

                    SubT* c = creator.create( this, rdn );
                    priv_ensureSubContext_helper_ins( c, created );
                    creator.setupNew( c );
                    return c;
                }
                catch( NoSuchSubContext& e )
                {
                    std::cerr << "Context::priv_readSubContext() e:" << e.what() << std::endl;
                    throw e;
                }
                catch( FerrisNotSupportedInThisContext& e )
                {
                    std::cerr << "Context::priv_readSubContext() e:" << e.what() << std::endl;
                    throw e;
                }
                catch( std::exception& e )
                {
                    std::cerr << "Context::priv_readSubContext() strange e:" << e.what() << std::endl;
                    BackTrace();
                    throw e;
                }
            }

        
        
        void setIsNativeContext();
        
    public:

        /**
         * Assuming that !isActiveView() this returns an rdn which is unique
         * attempting to use the given rdn and appending version numbers as needed.
         */
        std::string monsterName( const std::string& rdn );

        /**
         * Mainly only interesting for internal APIs which can provide faster
         * codepaths for file:// native contexts.
         */
        virtual bool getIsNativeContext() const;
        
        
        void setHasDynamicAttributes( bool v );

        /**
         * This is like addAttribute except it does no checking, and can add stateless
         * DONT use this method directly, either use addAttribute() or
         * tryAddStateLessAttribute()
         *
         * returns 1 if the attribute was added
         */
        virtual bool setAttribute( const std::string& rdn,
                                   EA_Atom* atx,
                                   bool addToREA,
                                   XSDBasic_t sct = XSD_UNKNOWN,
                                   bool isStateLess = false )
            throw( AttributeAlreadyInUse );

        /**
         * If for all of our subcontexts each attribute with the same name
         * has the same schema (eg. width, height) then this is true.
         *
         * Some contexts, for example XML files allow the same name attribute
         * to have a different schema for each child. Such contexts will return
         * false here.
         */
        virtual bool getSubContextAttributesWithSameNameHaveSameSchema();
        
    private:

        SubContextNames_t SubContextNamesCache;

        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////

        /**
         * Add stateless attributes for emblem
         * used in OnEmblemCreated and createStateLessAttributes()
         */
        void createStateLessAttributesForEmblem( fh_emblem em );
        void OnEmblemCreated( fh_etagere et, fh_emblem em );

    private:
        friend class EAGenerator_XMP;
    protected:
        /**
         * supplementStateLessAttributes_size() is called from
         * supplementStateLessAttributes() to add size related augmentation
         */
        virtual void supplementStateLessAttributes_size(  std::string an );

        /**
         * supplementStateLessAttributes_timet() is called from
         * supplementStateLessAttributes() to add unix time related augmentation
         */
        virtual void supplementStateLessAttributes_timet( std::string an );
        
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////

    protected:

        // help making the gtktreemodel faster
        friend class GtkTreeModelStrongArm;
        
        /*
         * Test if the given rdn can be inserted into the collection.
         */
        virtual bool canInsert( const std::string& rdn );
    
        virtual void emitExistsEventForEachItem();
    

        bool areReadingDir();

    public:

        /*
         * Debug only, this prints out the subcontexts info.
         * Consider this method private to libferris' use only.
         */
        virtual void dumpOutItems();


//         /**
//          * Return 1 if added, 0 if not added because it was already there.
//          */
//         virtual bool addAttribute( const std::string& rdn,
//                                    EA_Atom* atom,
//                                    bool addToREA = false );
        
//         virtual bool addAttribute( const char* rdn,
//                                    const EA_Atom_ReadOnly::GetIStream_Func_t& f,
//                                    bool addToREA = false,
//                                    Loki::TypeInfo t = typeid( Attribute ) );
        
//         virtual bool addAttribute( const char* rdn,
//                                    const EA_Atom_ReadOnly::GetIStream_Func_t& f_i,
//                                    const EA_Atom_ReadWrite::GetIOStream_Func_t& f_io,
//                                    const EA_Atom_ReadWrite::IOStreamClosed_Func_t& f_closed,
//                                    bool addToREA = false,
//                                    Loki::TypeInfo t = typeid( Attribute ) );
        
//         virtual bool addAttribute( const std::string& rdn,
//                                    const std::string& v,
//                                    bool addToREA = false );

        /******************************************************************************/
        /******************************************************************************/
        /******************************************************************************/

//         virtual EA_Atom* tryAddHeapAttribute( const std::string& rdn,
//                                                         EA_Atom* a,
//                                                         bool addToREA = false );
        
    private:

        friend class Private::CacheManagerImpl;
#ifndef LIBFERRIS_INTERNAL_COMPILING_SWIG_WRAPPER
        friend FERRISEXP_DLLLOCAL void DepthFirstDeletePCCTS_DropInOderList( childContext* cc );
#endif
        friend class UnionContext;
        friend class DifferenceContext;
        friend class SetIntersectionContext;
        friend class SetSymmetricDifferenceContext;
        friend class NormalContextXSLTFSWrapper;
        friend class XSLTFS_DOMWrapper;
        friend class SelectionContext;
        friend class ManyBaseToOneChainedViewContext;
        Items_t Items;
        Items_t& getItems();

        Items_t::iterator getItemIter( const std::string& rdn );
        fh_context getItem( const std::string& rdn );
        void eraseItemByName( Items_t& items, const std::string& rdn );
        void eraseItemByName( Items_t& items, Context* c );
        
        gint32 NumberOfSubContexts;

        friend class CacheManager;
        friend class FCA::LatticeRootConceptVFS_RootContextDropper;
        friend class ContextVFS_RootContextDropper;
        friend class RootContextVFS_RootContextDropper;
        friend class RootContext;
        friend struct RootRootContext;

        sigc::connection AttributeCountRaisedFromOne_Connection;

    protected:
        virtual fh_context priv_getSubContext( const std::string& rdn )
            throw( NoSuchSubContext );
    private:

        /*
         * Handling of overmounts.
         *
         * OverMountContext_Delegate points from the base context (eg native) to the
         * overmounting context (eg db4)
         *
         * CoveredContext points from the overmounting context (eg db4) back to the
         * covered context (eg native).
         */
        fh_context OverMountContext_Delegate;
        void       setOverMountContext( const fh_context& c );
        void       clearOverMountContext();
        void       unOverMount( CacheManager::freelist_t& fl );
        void       unOverMount_delete( CacheManager::freelist_t& fl, Context* c );
        fh_context findOverMounter( bool attemptingOverMountOnlyToFindEA = false );
        bool       hasOverMounter();
    protected:
        void       setOverMountAttemptHasAlreadyFailed( bool v );
        
        friend class MessageContext;
        void       setCoveredContext( const fh_context& c );
        Context*   getCoveredContext(); // this || CoveredContext
    public: // This is only public so that getBaseContext() can work as expected.
        Context*   getOverMountContext(); // this || OverMountContext_Delegate
    private:
        
        /**
         * Has to do some stuff with getOverMountContext()
         */
        friend class PluginOutOfProcNotificationEngine;
        
        /********************************************************************************/
        /*** META DATA ABOUT PLUGIN MODULES *********************************************/
        /********************************************************************************/

        virtual bool supportsMonitoring();
        virtual bool supportsReClaim();
        virtual bool supportsRename();
        virtual bool isRemote();

        /**
         * In some cases such as an XML file which is the result of an XSLT inside
         * xsltfs:// the filesystem itself might want to override the automatic
         * overmounting process.
         */
        virtual bool disableOverMountingForContext();
        
        /**
         * If a subclass overrides priv_getSubContext() to check and make a subcontext
         * without reading the whole dir (see native module) then it should override
         * this method and return true.
         *
         * @return true if supports, false by default
         */
        virtual bool priv_supportsShortCutLoading();
    protected:
        bool supportsShortCutLoading();
    private:
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        virtual fh_context priv_rename( const std::string& rdn,
                                        const std::string& newPath,
                                        bool TryToCopyOverFileSystems = true,
                                        bool OverWriteDstIfExists = false );
        fh_context contextHasBeenRenamed( const std::string& oldrdn, const std::string& rdn );

        /*
         * Removal
         */
        virtual bool supportsRemove();
        virtual void priv_remove( fh_context c );

    public:

//        void removeSelf();
        void remove( const std::string& rdn );
        void remove( fh_context c );
        virtual fh_context rename( const std::string& rdn,
                                   const std::string& newPath,
                                   bool TryToCopyOverFileSystems = true,
                                   bool OverWriteDstIfExists = false );

        fh_context copyTo( const std::string& newPath, bool OverWriteDstIfExists = false );
        
        
        Context( Context* CoveredContext = 0 );
        Context( Context* parent, const std::string& rdn );
        virtual ~Context();

                                                   
        virtual fh_attribute createAttribute( const std::string& rdn )
            throw( FerrisCreateAttributeFailed,
                   FerrisCreateAttributeNotSupported,
                   AttributeAlreadyInUse );

        virtual fh_attribute acquireAttribute( const std::string& rdn )
            throw( FerrisCreateAttributeFailed,
                   FerrisCreateAttributeNotSupported );

        

        fh_istream getCreateSubContextSchema();

        virtual fh_context
        createSubContext( const std::string& rdn,
                          fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );

        virtual fh_context
        createSubContext( const std::string& rdn, fh_mdcontext md )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );
    
    
        virtual fh_context getRelativeContext( const std::string& xdn, RootContextFactory* f = 0 )
            throw( NoSuchSubContext );

    
        virtual ContextCollection::SubContextNames_t& getSubContextNames();
        virtual fh_context getSubContext( const std::string& rdn ) throw( NoSuchSubContext );

        bool priv_isSubContextBound( const std::string& rdn, Items_t::iterator& iter );
        bool priv_isSubContextBound( const std::string& rdn );
        bool isSubContextBound( const std::string& rdn );
    

    
        virtual void read( bool force = 0 );
        long guessSize() throw();


        bool        hasSubContexts();
        virtual int SubContextCount();

        NamingEvent_MedallionUpdated_Sig_t&      getNamingEvent_MedallionUpdated_Sig();
        NamingEvent_Changed_Sig_t&               getNamingEvent_Changed_Sig();
        NamingEvent_Deleted_Sig_t&               getNamingEvent_Deleted_Sig();
        NamingEvent_Start_Execute_Sig_t&         getNamingEvent_Start_Execute_Sig();
        NamingEvent_Stop_Execute_Sig_t&          getNamingEvent_Stop_Execute_Sig();
        NamingEvent_Created_Sig_t&               getNamingEvent_Created_Sig();
        NamingEvent_Moved_Sig_t&                 getNamingEvent_Moved_Sig();
        NamingEvent_Exists_Sig_t&                getNamingEvent_Exists_Sig();
        NamingEvent_Start_Reading_Context_Sig_t& getNamingEvent_Start_Reading_Context_Sig();
        NamingEvent_Stop_Reading_Context_Sig_t&  getNamingEvent_Stop_Reading_Context_Sig();
        ContextEvent_Headers_Received_Sig_t&     getContextEvent_Headers_Received_Sig();


        virtual FerrisLoki::Handlable::ref_count_t AddRef();
        virtual FerrisLoki::Handlable::ref_count_t Release();
        virtual bool        all_attributes_have_single_ref_count();

        virtual bool isAttributeBound( const std::string& rdn,
                                       bool createIfNotThere = true
            ) throw( NoSuchAttribute );
        virtual fh_attribute     getAttribute( const std::string& rdn ) throw( NoSuchAttribute );
        virtual AttributeNames_t& getAttributeNames( AttributeNames_t& ret );
        virtual int              getAttributeCount();
        fh_emblem getAttributeRootEmblem();

        void Emit_MedallionUpdated();
        void Emit_Changed( NamingEvent_Changed* e,
                           const std::string& olddn, const std::string& newdn, sigc::trackable* ExtraData );
        void Emit_Deleted( NamingEvent_Deleted* e,
                           std::string olddn, std::string newdn, sigc::trackable* ExtraData );
        void Emit_Start_Execute( NamingEvent_Start_Execute* e,
                                 std::string olddn, std::string newdn, sigc::trackable* ExtraData );
        void Emit_Stop_Execute( NamingEvent_Stop_Execute* e,
                                std::string olddn, std::string newdn, sigc::trackable* ExtraData );
        void Emit_Created( NamingEvent_Created* e,
                           const fh_context& newc,
                           std::string olddn, std::string newdn, sigc::trackable* ExtraData );
        void Emit_Moved( NamingEvent_Moved* e,
                         std::string olddn, std::string newdn, sigc::trackable* ExtraData );
        void Emit_Exists( NamingEvent_Exists* e,
                          const fh_context& newc,
                          std::string olddn, std::string newdn, sigc::trackable* ExtraData );
        void Emit_Start_Reading_Context( NamingEvent_Start_Reading_Context* e,
                                         sigc::trackable* ExtraData );
        void Emit_Stop_Reading_Context( NamingEvent_Stop_Reading_Context* e,
                                        sigc::trackable* ExtraData );


    protected:

        /**
         * Used by getURL() to get the scheme:// part of the URL.
         * mainly made availaible for root:// to override the scheme
         * because its children are schemes themself.
         */
        virtual std::string getURLScheme();
    public:
//        virtual const std::string& getDirName() const;
        virtual std::string getURL();

    protected:
    
        /*************************************************************
         * These are the methods that children might wish to override.
         *************************************************************/
        virtual Context* priv_CreateContext( Context* parent, std::string rdn ) = 0;
        Context* CreateContext( Context* parent, const std::string& rdn )
            {
                return priv_CreateContext( parent, rdn );
            }

#ifndef LIBFERRIS_INTERNAL_COMPILING_SWIG_WRAPPER
        friend FERRISEXP_API fh_context SL_SubCreate_alwaysThrow( fh_context c, fh_context md );
        class SubContextCreator
        {
        public:
            /**
             * This takes the context to create the subcontext on and the metadata
             * returned from the client.
             *
             * @returns the new context that was created.
             *
             */
            typedef Loki::Functor< fh_context,
                                   LOKI_TYPELIST_2( fh_context, fh_context ) > Perform_t;

            SubContextCreator( Perform_t f = SL_SubCreate_alwaysThrow,
                               std::string schema = "",
                               std::string simpleTypes = "" )
                :
                Perform(f),
                Schema(schema),
                m_simpleTypes( simpleTypes )
                {
                }
            
            std::string getSchema()
                {
                    return Schema;
                }

            std::string getXSDSimpleTypes()
                {
                    return m_simpleTypes;
                }
            
            fh_context perform( fh_context c, fh_context md )
                {
                    return Perform( c, md );
                }

        private:
            Perform_t Perform;
            std::string Schema;
            std::string m_simpleTypes;
        };

        friend FERRISEXP_API fh_context SL_SubCreate_file( fh_context c, fh_context md );
        friend FERRISEXP_API fh_context SL_SubCreate_dir( fh_context c, fh_context md );
        friend FERRISEXP_API fh_context SL_SubCreate_ea( fh_context c, fh_context md );
        friend FERRISEXP_API fh_context SL_SubCreate_text ( fh_context c, fh_context md );

        friend fh_context SL_edb_SubCreate_dir( fh_context c, fh_context md );
        friend fh_context SL_edb_SubCreate_file( fh_context c, fh_context md );
        friend fh_context SL_ipc_SubCreate_file( fh_context c, fh_context md );
        friend fh_context SL_ipc_sem_SubCreate_file( fh_context c, fh_context md );

        friend fh_context SL_db4_SubCreate_dir( fh_context c, fh_context md );
        friend fh_context SL_db4_SubCreate_file( fh_context c, fh_context md );
//         friend fh_context SL_SubCreate_CustomType( std::string libname,
//                                                    fh_context c, fh_context md );
        friend struct SubCreate_CustomType;
        friend class CreationStatelessFunctor;
        
        friend fh_context SL_commondb_SubCreate_dir( fh_context c, fh_context md );
        friend fh_context SL_commondb_SubCreate_file( fh_context c, fh_context md );
        friend fh_context SL_SubCreate_commondb ( fh_context c, fh_context md );
        
        friend fh_context SL_ldap_SubCreate_context( fh_context c, fh_context md );
#endif
        
    public:
        /**
         * PRIVATE INTERNAL USE ONLY. This method is public only to allow templates
         * in libferris itself to use it more easily.
         */ 
        virtual FERRISEXP_API fh_context SubCreate_file( fh_context c, fh_context md );
    protected:

        
        virtual fh_context SubCreate_ea( fh_context c, fh_context md );
#ifndef LIBFERRIS_INTERNAL_COMPILING_SWIG_WRAPPER
        typedef std::map< std::string, SubContextCreator > CreateSubContextSchemaPart_t;
        virtual fh_istream generateSchema( CreateSubContextSchemaPart_t& m );
        virtual void      addStandardFileSubContextSchema( CreateSubContextSchemaPart_t& m );
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );
        void addEAGeneratorCreateSubContextSchema( CreateSubContextSchemaPart_t& m );
    private:
        void              FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m,
                                                           bool followOvermount = true );
        friend void insertAllCreatorModules( Context::CreateSubContextSchemaPart_t& m );
        friend void insertAbstractCreatorModules( Context::CreateSubContextSchemaPart_t& m );
        friend void insertCreatorModules( Context::CreateSubContextSchemaPart_t& m, bool );
#endif
    private:


        void addToCreateHistory( const std::string& fileType );
        
        virtual fh_context
        priv_createSubContext( const std::string& rdn, fh_context md )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );

        gboolean getUpdateMetaData_First_Time();
    protected:
        virtual void updateMetaData();
        virtual void ensureUpdateMetaDataCalled();
        virtual void ensureAttributesAreCreated( const std::string& eaname = "" );
    private:
        virtual void tryToFindAttributeByOverMounting( const std::string& eaname = "" );

//         typedef ::Loki::Functor< void,
//                                  LOKI_TYPELIST_2(const fh_context& ctx,
//                                             const string& eaname)> ContextEAGenFunctor_t;
//         typedef map< string, ContextEAGenFunctor_t > ContextEAGenMap_t;
//         virtual ContextEAGenMap_t& getContextEAGenerators();
        
    protected:
        virtual void priv_createAttributes();
        virtual void createAttributes();

        virtual void createStateLessAttributes( bool force = false );
        virtual void supplementStateLessAttributes( bool force = false );
    private:
        
        void tryToOverMount( bool silentIgnore = false, bool attemptingOverMountOnlyToFindEA = false );
        void readOverMount();
    protected:
        virtual void priv_read();
        virtual fh_context priv_readSubContext(
            const std::string& rdn,
            bool created = false,
            bool checkIfExistsAlready = true
            )
            throw( NoSuchSubContext, FerrisNotSupportedInThisContext );
    private:
        fh_context priv_discoveredSubContext( const std::string& rdn, bool created = false )
            throw( NoSuchSubContext, FerrisNotSupportedInThisContext );
        

        virtual long priv_guessSize() throw();


        /*************************************************************
         * Helpers for context subclasses to add/remove items
         *************************************************************/
    protected:
        virtual fh_context Insert( Context* ctx, bool created = false, bool emit = true )
            throw( SubContextAlreadyInUse );
    
        // Silent failure if not in collection
        virtual void Remove( Context*   ctx, bool emitdeleted = true ); 
        void         Remove( fh_context ctx, bool emitdeleted = true ); 
        void         Remove( const std::string& ctxName, bool emitdeleted = true ); 

    private:
        void ReadDone( NamingEvent_Stop_Reading_Context* src );
    protected:
        virtual void clearContext();

        /*************************************************************
         * Some handy methods for emitting events at the right time.
         *************************************************************/
    protected:
        void ClearStartReadingFlag();
        void EnsureStartReadingIsFired();
        void EnsureStopReadingIsFired();

    private:
        /*************************************************************
         * Reference counting mechanics.
         * Used by garbo collector, so can not create more handles.
         *************************************************************/
//         typedef list< fh_context > ViewContexts_t;
//         /**
//          * The base context keeps a collection of all the filtered/sorted/cached
//          * contexts that are sitting ontop of it. This keeps the VM system more uniform.
//          */
//         ViewContexts_t ViewContexts;
        void TryToAddOurselfToFreeList();
        void RemoveOurselfFromFreeList();
        virtual int  getMinimumReferenceCount();
        virtual void UnPageSubContextsIfNeeded();
        bool isReClaimable();
        bool reclaimContextObject( Context* a );
#ifndef LIBFERRIS_INTERNAL_COMPILING_SWIG_WRAPPER
        friend FERRISEXP_DLLLOCAL void DepthFirstDelete( CacheManager::freelist_t& fl, Context* cc, bool callReclaimContextObject );
        typedef std::list< Context* > cptrlist_t;
        friend FERRISEXP_DLLLOCAL void DepthFirstDelete( CacheManager::freelist_t& fl, Context* cc, cptrlist_t& l, bool callReclaimContextObject );
#endif
        
    public: // FIXME: only for testing
        void dumpRefDebugData( fh_ostream ss );
    protected:
#ifndef LIBFERRIS_INTERNAL_COMPILING_SWIG_WRAPPER
        friend void dumpEntireContextListMemoryManagementData( fh_ostream ss );
        friend FERRISEXP_DLLLOCAL void debug_ferris_check_for_single_ctx_violation(
            Context* parentc, Context* childc );
#endif
        

        // memory management
        friend class ContextStreamMemoryManager;
        virtual void RegisterStreamWithContextMemoryManagement( fh_istream& ss );
//         // For Attribute::RegisterStreamWithContextMemoryManagement to
//         // call StreamIsOpeningHandler
        friend class Attribute;
//         void StreamIsOpeningHandler( fh_istream ss );
        void StreamIsClosingHandler( FerrisLoki::Handlable* );
        
        /*************************************************************
         * Event mechanics.
         *************************************************************/
        NamingEvent_MedallionUpdated_Sig_t NamingEvent_MedallionUpdated_Sig;
        NamingEvent_Changed_Sig_t          NamingEvent_Changed_Sig;
        NamingEvent_Deleted_Sig_t          NamingEvent_Deleted_Sig;
        NamingEvent_Start_Execute_Sig_t    NamingEvent_Start_Execute_Sig;
        NamingEvent_Stop_Execute_Sig_t     NamingEvent_Stop_Execute_Sig;
        NamingEvent_Created_Sig_t          NamingEvent_Created_Sig;
        NamingEvent_Moved_Sig_t            NamingEvent_Moved_Sig;
        NamingEvent_Exists_Sig_t           NamingEvent_Exists_Sig;
        NamingEvent_Start_Reading_Context_Sig_t NamingEvent_Start_Reading_Context_Sig;
        NamingEvent_Stop_Reading_Context_Sig_t  NamingEvent_Stop_Reading_Context_Sig;
        ContextEvent_Headers_Received_Sig_t     ContextEvent_Headers_Received_Sig;

        fh_istream getSubContextCountStream  ( Attribute* attr );
        fh_istream getAttributeCountStream   ( Attribute* attr );
        fh_istream getDirNameStream          ( Attribute* attr );
        fh_istream getDirNameExtensionStream ( Attribute* attr );
        fh_istream getDirPathStream          ( Attribute* attr );
        fh_istream getEANamesStream          ( Attribute* attr );
//        fh_istream getDigestStream           ( std::string digestName, const std::string& rdn, EA_Atom* atom );


        /*************************************************************
         * Stuff that Context adds too.
         *************************************************************/
//         virtual void setAttribute(fh_attribute atx)
//             throw( AttributeAlreadyInUse );

        /*************************************************************
         * Stuff for resolving relative paths
         *************************************************************/
    public:
        typedef std::list<std::string> SplitPath_t;

        Context::SplitPath_t   splitPath( const std::string& dn );
        std::string            unSplitPath( const Context::SplitPath_t& pa );
    private:
        fh_context priv_getRelativeContext(
            SplitPath_t  pa,
            const std::string& xdn,
            const std::string& full_xdn,
            RootContextFactory* f
            )
            throw( NoSuchSubContext );

//         friend fh_context CreateDirWithParents( fh_context c,
//                                                 const std::string& n,
//                                                 int mode );
//         friend fh_context Shell::CreateDir( const std::string& path,
//                                             bool WithParents, int mode );
        
        /*************************************************************
         * Stuff for matched EA generators.
         *************************************************************/
    private:
        friend class MetadataBrokerDispatchInformation;
        
        // factory object to generating EA
        FERRIS_SMARTPTR( MatchedEAGeneratorFactory, fh_MatchedEAGeneratorFactory );

        // Stateless EA factories can be shared between all Context objects
        typedef std::list< fh_MatchedEAGeneratorFactory >   s_StatelessEAGenFactorys_t;
        const s_StatelessEAGenFactorys_t& getStatelessEAGenFactorys();

        // Statefull EA factories keep some data inside the factory itself on a
        // per context basis. As such we have to create a new factory for each context
        typedef std::list< fh_MatchedEAGeneratorFactory > m_StatefullEAGenFactorys_t;
        m_StatefullEAGenFactorys_t m_StatefullEAGenFactorys;
        friend class EAGenFactorys_iterator_class;

        // Because we have to create a new copy for each statefull EA generator
        // we keep a function around that can make factories
        typedef std::list< MatchedEAGeneratorFactory* (*)() > s_StatefullEAGenFactorysFactorys_t;
        struct EAGenData {
            s_StatelessEAGenFactorys_t* SL;
            s_StatefullEAGenFactorysFactorys_t* SF;
        };
        // called only by ensureEAGenFactorysSetup()
        void
        getStaticEAGenFactorys( EAGenData& d, bool& SFEAGenDynamic );

        // This is the method that should be used to make sure that the stateless
        // and statefull ea generators are setup for this context
        void ensureEAGenFactorysSetup();

        // called only once to add stateless factories to s_StatelessEAGenFactorys
        friend void AppendAllStaticEAGeneratorFactories_Stateless(
            Context::s_StatelessEAGenFactorys_t& SL );

        // called to setup each context with new statefull factories
        friend  bool AppendAllStaticEAGeneratorFactories_Statefull(
            Context::m_StatefullEAGenFactorys_t& SF );


        
        virtual bool VetoEA();

#ifndef LIBFERRIS_INTERNAL_COMPILING_SWIG_WRAPPER
        friend std::string getStrAttr( const fh_context& c,
                                       const std::string& rdn,
                                       const std::string& def,
                                       bool getAllLines,
                                       bool throwEx );
        friend std::string getStrAttr( AttributeCollection* c,
                                       const std::string& rdn,
                                       const std::string& def,
                                       bool getAllLines,
                                       bool throwEx );
        friend std::string setStrAttr( fh_context c,
                                       const std::string& rdn,
                                       const std::string& v,
                                       bool create,
                                       bool throw_for_errors,
                                       bool dontDelegateToOvermountContext );
        
#endif
        
    public:        
        /*************************************************************
         * Multiple mimetype engine support
         *************************************************************/
    protected:
        virtual std::string priv_getMimeType( bool fromContent = false );
    public:
        std::string getMimeType( bool fromContent = false );
        virtual std::string getFileType();



        /************************************************************/
        /*** medallions *********************************************/
        /************************************************************/
    public:
        fh_medallion getMedallion();
        bool hasMedallion();

        /************************************************************/
        /*** namespaces *********************************************/
        /************************************************************/
    public:
        virtual stringlist_t  getNamespacePrefixes();
    protected:
        virtual void          readNamespaces();
        virtual std::string   resolveFerrisXMLNamespace( const std::string& s );

        /************************************************************/
        /*** new branches *******************************************/
        /************************************************************/
    public:
        /**
         * Branch filesystems like medallion, signature, parents,
         * children are all direct children of the filesystem returned
         * from this call.
         *
         *
         * / (This is the context which is returned, each branch
         *    filesystem is a child)
         *  medallion/
         *     ...
         *  signatures/
         *     ...
         *  parents/
         *     ...
         *  children/
         *     ...
         */
        virtual fh_context getBranchFileSystem();

    protected:
        virtual std::string priv_getRecommendedEA();


        /************************************************************/
        /*** new image metadata interface *** 1.3.40 ****************/
        /************************************************************/
    public:
        /**
         * Get the filename extension
         */
        std::string getNameExtension();

        
        /**
         * A function that can create an image object from a context
         * object
         */
        typedef Loki::Functor< fh_image, LOKI_TYPELIST_1( const fh_context& ) > f_imageEAGenerator;
    protected:

        /**
         * a collection of image object creators given the filename extension.
         * The key of the map is the filename extension for the image plugin
         * std::pair is the function object itself and if the image will be writable.
         */
        typedef std::map< std::string, std::pair< f_imageEAGenerator, bool > > s_imageEAGenerators_t;
        static s_imageEAGenerators_t& getImageEAGenerators();
        
        
    public:

        
        /**
         * Get the image associated with this context
         */
        fh_image getImage();
        /**
         * Like getImage() but instead of throwing an exception when
         * there is no image it will return 0
         */
        fh_image getImageOrNULL();

        /**
         * Subclasses of Context can override this if they know how to
         * create an image object for embedded data. For example EFL
         * eet files contain images which are embedded in the eet file.
         *
         * Subclasses do not need to worry about caching as the getImage()
         * call which calls priv_getImage() will manage image object caching.
         */
        virtual fh_image priv_getImage();
        
        /**
         * When an image needs to be written out before its removed from cache
         * it can be tainted so that it is not written right away but can be
         * modified further before writing.
         */
        void taintImage( fh_image im );

        /**
         * Used by image plugins to register themselves for various filename extensions.
         */
        static bool RegisterImageEAGeneratorModule(
            const std::string& ext,
            bool writable,
            const std::string& implname,
            const std::string& shortname );
        static bool UnrollRegisteredImageEAGeneratorModule(
            const std::string& ext,
            bool writable,
            const std::string& implname,
            const f_imageEAGenerator& f );
        
        
    protected:

        virtual void imageEAGenerator_priv_createAttributes( bool checkForImageLoader = true );
        bool imageEAGenerator_haveLoader();

        static fh_istream imageEAGenerator_getMegapixelsStream  ( Context*, const std::string&, EA_Atom* attr );
        static fh_istream imageEAGenerator_getWidthStream  ( Context*, const std::string&, EA_Atom* attr );
        static fh_istream imageEAGenerator_getHeightStream        ( Context*, const std::string&, EA_Atom* attr );
        static fh_istream imageEAGenerator_getDepthPerColorStream ( Context*, const std::string&, EA_Atom* attr );
        static fh_istream imageEAGenerator_getDepthStream         ( Context*, const std::string&, EA_Atom* attr );
        static fh_istream imageEAGenerator_getGammaStream         ( Context*, const std::string&, EA_Atom* attr );
        static fh_istream imageEAGenerator_getHasAlphaStream      ( Context*, const std::string&, EA_Atom* attr );
        static fh_istream imageEAGenerator_getAspectRatioStream   ( Context*, const std::string&, EA_Atom* attr );
        static fh_istream imageEAGenerator_getRGBAStream          ( Context*, const std::string&, EA_Atom* attr );

        static fh_iostream imageEAGenerator_getWidthIOStream(  Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_iostream imageEAGenerator_getHeightIOStream( Context* c, const std::string& rdn, EA_Atom* atom );
        static void imageEAGenerator_updateWidthFromStream(  Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss );
        static void imageEAGenerator_updateHeightFromStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss );
    
        static fh_iostream imageEAGenerator_getRGBAIOStream( Context* c, const std::string& rdn, EA_Atom* atom );
        static void imageEAGenerator_updateFromStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss );

        /**
         * getImageWidth() and getImageHeight() allow context subclasses to
         * provide special implementations of width/height finding.
         */
        virtual int getImageWidth();
        virtual int getImageHeight();
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

    public:

        bool isCompressedContext();

        /**
         * If this directory is part of an implicit smush set then return that set.
         * failure is cached per session.
         */
//        redlandea::fh_SmushSet tryToGetImplicitTreeSmush();
        Semantic::fh_SmushSet tryToGetImplicitTreeSmushSet();


    protected:

        void supplementFerrisLinkTargetFromAbsolute();
        fh_istream getFerrisLinkTargetAbsoluteStream( Context*, const std::string&, EA_Atom* attr );
        fh_istream getFerrisIsBrokenLinkStream( Context*, const std::string&, EA_Atom* attr );


    protected:
#define FERRIS_LS FERRISEXP_DLLLOCAL static
#define FERRIS_ES FERRISEXP_DLLLOCAL static
        FERRIS_LS fh_iostream SL_getFerrisPostCopyActionStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS void SL_FerrisPostCopyActionStreamClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss );
        FERRIS_LS fh_iostream SL_getFerrisPreCopyActionStream( Context* c, const std::string& rdn, EA_Atom* atom );
        FERRIS_LS void SL_FerrisPreCopyActionStreamClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss );
#undef FERRIS_LS
#undef FERRIS_ES
        virtual void priv_postCopyAction( fh_context c );
        virtual void priv_preCopyAction( fh_context c );

        
        friend class MetadataWorker;
        /**
         * Used in the DBus metadata servers to select only a single plugin to be used
         * for metadata generation and no others.
         */
        virtual std::string getStrAttr_UsingRestrictedPlugins( const std::string& eaname,
                                                               const std::set< std::string >& plugins,
                                                               bool getAllLines = true );

    public:
        /**
         * This will AddRef() the passed handlable object and it will be Release()ed when
         * this Context object is deleted. This makes the handlable effectively have
         * the same lifetime as the context.
         */
        void addHandlableToBeReleasedWithContext( Handlable* h );
    private:
        typedef std::list< Handlable* > m_handlableList_t;
        m_handlableList_t* m_handlablesToReleaseWithContext;


    public:
        /**
         * PRIVATE INTERNAL USE ONLY. This method is public only to allow templates
         * in libferris itself to use it more easily.
         */ 
        virtual FERRISEXP_API fh_context SubCreate_dir( fh_context c, fh_context md );

    protected:


        virtual void OnReadComplete_setupUserOverlayLinks();

        virtual fh_istream getRecommendedEAUnionView();

        
        FERRISEXP_DLLLOCAL static fh_iostream
            SL_getSubtitlesStream( Context* c, const std::string& rdn, EA_Atom* atom );

        FERRISEXP_DLLLOCAL static fh_iostream
            SL_getDotEAStream( Context* c, const std::string& rdn, EA_Atom* atom );

      public:
        virtual void dumpTree();

    };


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
};


#include <Ferris/SM.hh>
#include <Ferris/MatchedEAGenerators.hh>


namespace Ferris
{
    



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//     std::string setStrAttr( fh_context c,
//                             const std::string& rdn,
//                             const std::string& v,
//                             bool create = false );
    
    
    FERRISEXP_API bool isTrue( const std::string& s );
    FERRISEXP_API bool isFalse( const std::string& s );
    FERRISEXP_API bool isNumber( const std::string& s );

    namespace Factory
    {
        FERRISEXP_API fh_context getCreateHistory();
    };

    namespace Main
    {
        FERRISEXP_API gboolean EventPending();
        FERRISEXP_API void processEvent();
        FERRISEXP_API void processAllPendingEvents();
        FERRISEXP_API void mainLoop();
    };
    

    namespace Util
    {
        /**
         * Class that returns true only once for operator()();
         */
        class FERRISEXP_API SingleShot
        {
            bool virgin;
            
        public:

            SingleShot();
            bool operator()();
            bool value();
            inline operator bool() {
                return operator()();
                }
        };

    };

    /**
     * Get the first mimetype for the context 'c'.
     * This function may cleanup what is in the "mimetype" EA a little
     * before returning it.
     */
    FERRISEXP_API std::string getMimeName( fh_context c );

    /**
     * Get a context iterator for the given context.
     * ret = c->parent->find( c->name )
     */
    FERRISEXP_API Context::iterator toContextIterator( fh_context c );
    

    FERRISEXP_API void addEAToSet( std::set<std::string>& theSet, const std::string commaSepEA );


    /**
     * Execute a SPARQL query against the myrdf datastore.
     */
    FERRISEXP_API fh_context ExecuteQueryAgainstMyRDF( const std::string& sparql );
    
    
    namespace ImplementationDetail
    {
        FERRISEXP_API stringset_t& getStaticLinkedRootContextNames();
        FERRISEXP_API bool appendToStaticLinkedRootContextNames( const std::string& s );

        extern const ::Ferris::Handlable::ref_count_t MAX_REF_COUNT;

    };

    
    FERRISEXP_API void ParseOnly_FERRIS_POPT_OPTIONS( const std::string& PROGRAM_NAME,
                                                      int argc,
                                                      const char** argv );

    FERRISEXP_API bool tryToUseOutOfProcessMetadataServer();

    FERRISEXP_API std::string makeFerrisPluginPath( const std::string& dir, const std::string& libname = "" );
    
};


#include <popt.h>
#define FERRIS_POPT_OPTIONS                               \
{ 0, 0, POPT_ARG_INCLUDE_TABLE,::Ferris::Logging::getPopTable(),   \
  0, "Ferris options:", 0 },

#include <Ferris/BuildDependentMethods.hh>

// fh_mdcontext is used by some clients.
#include <Ferris/Context.hh>
    

#endif // ifndef _ALREADY_INCLUDED_FERRIS_H_
