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

    $Id: CacheManager.cpp,v 1.8 2010/09/24 21:30:25 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "Ferris.hh"
#include "Ferris/FilteredContext.hh"
#include "Ferris/FilteredContext_private.hh"

#include "config.h"
#define CERR cerr

#include "Ferris_private.hh"
#include "CacheManager_private.hh"
#include "PluginOutOfProcNotificationEngine.hh"
#include "Cache.hh"

using namespace std;

/*
 * Have to free PCCTS mounts top down because of InOrder child lists and such.
 */
//#include <plugins/context/pccts/PcctsChildContext.hh>

// lstat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


namespace Ferris
{
#ifdef DEBUG_CONTEXT_MEMORY_MANAGEMENT

    debug_mm_contexts_t& getMMCtx()
    {
        static debug_mm_contexts_t v;
        return v;
    }
        
    void addContextToMemoryManagementData( Context* c )
    {
        static int num=0;
        getMMCtx()[c] = ++num;
    }
    
    void remContextToMemoryManagementData( Context* c )
    {
        if( !getMMCtx().empty() )
        {
            getMMCtx().erase(c);
        }
    }

#endif

    void DEBUG_dumpcl( string s )
    {
        fh_stringstream ss;
        dumpEntireContextListMemoryManagementData( ss );
        cerr << "-- BEGIN DEBUG_dumpcl s:" << s << "--\n"
             << StreamToString(ss)
             << "-- END DEBUG_dumpMemc s:" << s << "--\n"
             << endl;
    }

    void DEBUG_dumpcl_to_file( const std::string& filenamePrefix )
    {
        time_t now = Time::getTime();
        string filename = filenamePrefix + "_" + tostr(now) + ".fmemd";

        struct stat statbuf;
        if( !lstat( filename.c_str(), &statbuf ) )
        {
            cerr << "DEBUG_dumpcl_to_file() refusing to override existing file:" << filename << endl;
            return;
        }

        fh_context c = Shell::acquireContext( filename, 0, false );
        fh_iostream ioss = c->getIOStream( ios::out | ios::trunc );
        dumpEntireContextListMemoryManagementData( ioss );

        ioss << "-------------------------------------------------------------------------------\n" << endl;
        CacheManager* cc = getCacheManager();
        cc->dumpFreeListTo( ioss );
        ioss << "-------------------------------------------------------------------------------\n" << endl;
    }
        
    
    void dumpEntireContextListMemoryManagementData( fh_ostream ss )
    {
#ifdef DEBUG_CONTEXT_MEMORY_MANAGEMENT
        ss << "dumpEntireContextListMemoryManagementData() CList start" << endl;
        for( debug_mm_contexts_t::iterator iter = getMMCtx().begin();
             iter != getMMCtx().end(); iter++ )
        {
            if( iter->first )
            {
                iter->first->dumpRefDebugData(ss);
            }
        }
        ss << "dumpEntireContextListMemoryManagementData() CList end" << endl;
#endif
    }

    /**
     * Note that this method fails if the client is running any filtered contexts.
     *
     * It is only here for low level testing.
     */
    FERRISEXP_DLLLOCAL void debug_ferris_check_for_single_ctx_violation( Context* parentc,
                                                                         Context* childc )
    {
// #define FERRIS_CHECK_FOR_SINGLE_CTX_VIOLATION 1
            
// #ifdef DEBUG_CONTEXT_MEMORY_MANAGEMENT
// #ifdef FERRIS_CHECK_FOR_SINGLE_CTX_VIOLATION
            
//         cerr << "debug_ferris_check_for_single_ctx_violation() CList start"
//              << " childc:" << childc->getDirPath()
//              << endl;

//         int c = 0;

//         for( debug_mm_contexts_t::iterator iter = getMMCtx().begin();
//              iter != getMMCtx().end(); iter++ )
//         {
// //            if( iter->first->getURL() == childc->getURL() )
            
//             cerr << "testing iter:" << iter->first->getDirPath() << endl;
            
//             if( iter->first->getDirPath() == childc->getDirPath() )
//             {
//                 ++c;
//             }
            
//         }

//         if( c > 1 )
//         {
//             DEBUG_dumpcl( "debug_ferris_check_for_single_ctx_violation( LIST )" );
//             cerr << "debug_ferris_check_for_single_ctx_violation( PROBLEM ) c:" << c << endl;
//             fh_stringstream ss;
//             for( debug_mm_contexts_t::iterator iter = getMMCtx().begin();
//                  iter != getMMCtx().end(); iter++ )
//             {
//                 if( iter->first->getDirPath() == childc->getDirPath() )
//                 {
//                     fh_stringstream ss;
//                     ss << "iter:" << endl;
//                     iter->first->dumpRefDebugData(ss);
//                 }
//             }
//             ss << "child:" << endl;
//             childc->dumpRefDebugData(ss);
//             ss << "parent:" << endl;
//             parentc->dumpRefDebugData(ss);
//             cerr << tostr(ss) << endl;
//             g_on_error_query(0);
//         }
        
//         cerr << "debug_ferris_check_for_single_ctx_violation() CList end" << endl;
// #endif
// #endif
    }

    namespace Private
    {
        static bool haveAnyContextReferenceWatches_Result = false;
        static bool haveAnyContextReferenceWatchesByName_Result = false;
        static bool haveAnyContextParentReferenceWatches_Result = false;
        
        static bool _isFilter( Context* c )
        {
            if( dynamic_cast<FilteredContext*>(c) )
                return true;
            return false;
        }


        static bool _isSorter( Context* c )
        {
            if( dynamic_cast<SortedContext*>(c) )
                return true;
            return false;
        }

        CacheManagerContextStateInTime::CacheManagerContextStateInTime( Context* c )
            :
            ref_count( c->ref_count ),
            NumberOfSubContexts( c->NumberOfSubContexts ),
            ItemsSz( c->getItems().size() ),
            isReClaimable( c->isReClaimable() ),
            WeAreInFreeList( c->WeAreInFreeList ),
            MinimumReferenceCount( c->getMinimumReferenceCount() ),
            ContextThisPtr( c ),
            isFilter( _isFilter(c) ),
            isSorter( _isSorter(c) ),
            HasBeenDeleted( HasBeenDeleted ),
            isInheritingContext( isInheritingContext ),
            cvc( dynamic_cast<ChainedViewContext*>( c ) ),
            CoveredContext( GetImpl( c->CoveredContext ) ),
            OverMountContext_Delegate( GetImpl( c->OverMountContext_Delegate ) ),
            isParentBound( c->isParentBound() ),
            parent( 0 ), parentR( 0 ), Delegate( 0 )
        {
            if( cvc )
            {
                Delegate = GetImpl(cvc->Delegate);
            }
            if( isParentBound )
                parent = c->getParent();

            if( isBound( c->ParentContext )
                && c->isParentBound()
                && GetImpl(c->ParentContext) != c->getParent() )
            {
                parentR = GetImpl(c->ParentContext);
            }
            
            name = c->getDirName();
            path = c->getDirPath();
        }
        
        CacheManagerContextStateInTime::~CacheManagerContextStateInTime()
        {
        }
        
        string CacheManagerContextStateInTime::str() const
        {
            stringstream ss;

            ss << "C"
               << " rc:" << ref_count
               << " numSc:" << NumberOfSubContexts
               << " item.sz:" << ItemsSz
               << " claim:" << isReClaimable
               << " frel:" << WeAreInFreeList
               << " mRC:" << MinimumReferenceCount
               << " this:" << (void*)ContextThisPtr
               << " isF:" << isFilter
               << " isS:" << isSorter;
            if( HasBeenDeleted )
                ss << " DEL";
            if( isInheritingContext )
                ss << " inh";
            if( Delegate )
            {
                ss << " D:" << toVoid(Delegate);
            }
            if( CoveredContext )
            {
                ss << " cc:" << toVoid( CoveredContext );
            }
            if( OverMountContext_Delegate )
            {
                ss << " om:" << toVoid( OverMountContext_Delegate );
            }
            
            if( parent )
                ss << " pnt:" << (void*)parent;
            if( parentR )
                ss << " pnt:" << (void*)parentR;

            ss << " n:" << name;
            ss << " p:" << path;
            
            return ss.str();
        }
        
        void dumpTo( fh_ostream oss, CacheManagerContextStateInTimeList_t& list, const std::string& hdr )
        {
            oss << "+++START DUMP+++" << hdr << endl;
            CacheManagerContextStateInTimeList_t::iterator iter = list.begin();
            CacheManagerContextStateInTimeList_t::iterator    e = list.end();
            
            for( ; iter != e ; ++iter )
            {
                oss << iter->str() << endl;
            }
            oss << "+++END DUMP+++" << hdr << endl;
        }
        
        Private::CacheManagerContextStateInTimeList_t&
        createMMCtxInTimeList( Private::CacheManagerContextStateInTimeList_t& ret )
        {
            for( debug_mm_contexts_t::iterator iter = getMMCtx().begin();
                 iter != getMMCtx().end(); iter++ )
            {
                if( iter->first )
                {
                    ret.push_back( iter->first );
                }
            }
        }

        CacheManagerContextStateInTimeIndexSet_t toIndexSet( CacheManagerContextStateInTimeList_t& l )
        {
            CacheManagerContextStateInTimeIndexSet_t ret;
            
            CacheManagerContextStateInTimeList_t::iterator iter = l.begin();
            CacheManagerContextStateInTimeList_t::iterator    e = l.end();
            for( ; iter!=e ; ++iter )
            {
                ret.insert( (*iter) );
            }
            return ret;
        }


        bool operator<( const CacheManagerContextStateInTime& a, const CacheManagerContextStateInTime& b )
        {
            if( a.ContextThisPtr && b.ContextThisPtr )
                return a.ContextThisPtr < b.ContextThisPtr;
            if( a.cvc && b.cvc )
                return a.cvc < b.cvc;
            return &a < &b;
        }
        
        bool operator==( const CacheManagerContextStateInTime& a, const CacheManagerContextStateInTime& b )
        {
            if( a.ContextThisPtr && b.ContextThisPtr )
                return a.ContextThisPtr == b.ContextThisPtr;
            if( a.cvc && b.cvc )
                return a.cvc == b.cvc;
            return &a == &b;
        }
        struct CacheManagerContextStateInTime_hash : public std::unary_function< CacheManagerContextStateInTime, size_t >
        {
            inline size_t operator()( const CacheManagerContextStateInTime& s ) const
                {
                    if( s.ContextThisPtr )
                        return size_t(s.ContextThisPtr);
                    if( s.cvc )
                        return size_t(s.cvc);
                    return size_t(&s);
                }
        };
        struct CacheManagerContextStateInTime_equal_to : public std::binary_function< CacheManagerContextStateInTime&, CacheManagerContextStateInTime&, bool>
        {
            inline bool operator()( const CacheManagerContextStateInTime& a, const CacheManagerContextStateInTime& b ) const
                {
                    if( a.ContextThisPtr && b.ContextThisPtr )
                        return a.ContextThisPtr == b.ContextThisPtr;
                    if( a.cvc && b.cvc )
                        return a.cvc == b.cvc;
                    return &a == &b;
                }
        };
            
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        
        void
        CacheManagerImpl::addToFreeList( Context* a )
        {
//            cerr << "addToFreeList() a:" << toVoid(a) << endl;

            a->WeAreInFreeList = true;
#ifdef FERRIS_DEBUG_VM
            if( a )
                LG_VM_D << "CacheManagerImpl::addToFreeList() c:" << toVoid(a)
                        << " " << a->getDirPath() << endl;
#endif

            getFreeList().insert(a);

//             if( autoCleanUpCall )
//             {
//                 cleanUp( false );
//             }
        }

        void
        CacheManagerImpl::removeFromFreeList( Context* a, bool quiet )
        {
            a->WeAreInFreeList = false;
            
#ifdef FERRIS_DEBUG_VM
            if( !quiet && a )
                LG_VM_D << "CacheManagerImpl::removeFromFreeList() c:" << toVoid(a)
                        << " " << a->getDirPath() << endl;
#endif

            getFreeList().erase( a );
        }

        void
        CacheManagerImpl::AutoClean()
        {
            if( shouldAutoCleanUp() )
            {
                cleanUp( false );
            }
        }

    Private::CacheManagerContextStateInTimeList_t&
    CacheManagerImpl::createCacheManagerContextStateInTimeList(
        Private::CacheManagerContextStateInTimeList_t& ret )
    {
        LG_VM_D << "CacheManager::createCacheManagerContextStateInTimeList() size:" << getFreeList().size() << endl;

        for( freelist_t::iterator iter = getFreeList().begin();
             iter != getFreeList().end(); iter++ )
        {
            Context* c = (*iter);
            if( c )
            {
                LG_VM_D << "CacheManager::createCacheManagerContextStateInTimeList() c:" << (void*)c << endl;
                if( c )
                {
                    ret.push_back( Private::CacheManagerContextStateInTime( c ) );
                }
            }
        }
    }
        
//        typedef Loki::SingletonHolder< CacheManagerImpl > CacheManagerSingleton;
    
        CacheManagerImpl* getCacheManagerImpl()
        {
            static CacheManagerImpl* ret = 0;
            if( !ret )
            {
                ret = new CacheManagerImpl();
            }
            return ret;
            
//             static CacheManagerImpl ret;
//             return &ret;
//            return &CacheManagerSingleton::Instance();
        }
    };
    

    CacheManager* getCacheManager()
    {
        return Private::getCacheManagerImpl();
    }
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    CacheManager::CacheManager()
        :
//         numberOfAllowedPermanentContextsInFreeList(100),
//         maxNumberOfContextsToFreeAtOnce(100),
//         maxNumberOfContextsInFreeList(500),
//         autoCleanUpCall(true)
        numberOfAllowedPermanentContextsInFreeList(-1),
        maxNumberOfContextsToFreeAtOnce(
            toint( getEDBString( FDB_GENERAL, "vm-auto-cleanup-maxfreeatonce", "100" ))),
        maxNumberOfContextsInFreeList(
            toint( getEDBString( FDB_GENERAL, "vm-auto-cleanup-maxnumberinfreelist", "15" ))),
        autoCleanUpCall( toint( getEDBString( FDB_GENERAL, "vm-auto-cleanup", "0" ))),
        m_insideResolveCall( 0 ),
        m_insideCleanupCall( false )
    {
#ifdef FERRIS_DEBUG_VM
        cerr << "CacheManager() autoCleanUpCall:" << autoCleanUpCall << endl;
#endif
        if( getenv( "LIBFERRIS_VM_AUTO_CLEANUP" ) != 0 )
            autoCleanUpCall = true;
    }

    CacheManager::~CacheManager()
    {
//        cerr << "CacheManager::~CacheManager()" << endl;
    }
    
    
    CacheManager::freelist_t&
    CacheManager::getFreeList()
    {
        return m_freelist;
    }
    


    void
    CacheManager::dumpFreeListTo( fh_ostream ss )
    {
        LG_VM_D << "CacheManager::dumpFreeListTo() size:" << getFreeList().size() << endl;

        ss << "CacheManager::dumpFreeListTo() CList start" << endl;
        for( freelist_t::iterator iter = getFreeList().begin();
             iter != getFreeList().end(); iter++ )
        {
            Context* c = (*iter);
            if( c )
            {
                LG_VM_D << "CacheManager::dumpFreeListTo() c:" << (void*)c << endl;
                if( c )
                    c->dumpRefDebugData( ss );
            }
        }
        ss << "CacheManager::dumpFreeListTo() CList end" << endl;
    }


    

    bool
    CacheManager::shouldAutoCleanUp()
    {
        return autoCleanUpCall;
    }
    
    struct FERRISEXP_DLLLOCAL IgnoreCallsWhileExists
    {
        bool* val;
        bool  WasAlreadySet;
        
        IgnoreCallsWhileExists( bool* v )
            :
            val(v),
            WasAlreadySet( *v )
            {
//                cerr << "Creating IgnoreCallsWhileExists v:" << (*val) << endl;
                *val = true;
            }

        bool operator()()
            {
                return WasAlreadySet;
            }
        
        ~IgnoreCallsWhileExists()
            {
//                cerr << "Destroy IgnoreCallsWhileExists v:" << (*val) << endl;
                if( !WasAlreadySet )
                {
                    *val = false;
                }
            }
    };

    
//    typedef list< Context* > cptrlist_t;
    FERRISEXP_DLLLOCAL void DepthFirstDelete( CacheManager::freelist_t& fl,
                                              Context* cc,
                                              Context::cptrlist_t& l,
                                              bool callReclaimContextObject )
    {
        cerr << "DepthFirstDelete(1) cc:" << cc << endl;
        
        for( Context::Items_t::iterator iter = cc->getItems().begin(); iter != cc->getItems().end(); )
        {
            Context* c = GetImpl(*iter);
            ++iter;
            DepthFirstDelete( fl, c, l );
        }

//        callReclaimContextObject = false;
        cerr << "DepthFirstDelete(2) cc:" << cc << endl;

        if( cc->isParentBound() )
        {
            l.push_back( cc );
            if( callReclaimContextObject )
            {
                cerr << "DepthFirstDelete() cc:" << toVoid(cc) << endl;
                if( !cc->getParent()->reclaimContextObject( cc ) )
                {
                    cerr << "Failed to DepthFirstDelete() context cc:" << toVoid(cc) << endl;
                }
                else
                {
                }
            }
        }
        else
        {
            l.push_back( cc );
            if( callReclaimContextObject )
            {
                fl.erase( cc );
                delete cc;
            }
        }
#ifdef FERRIS_DEBUG_VM
        cerr << "DepthFirstDelete() returning a level" << endl;
#endif
    }
    
    FERRISEXP_DLLLOCAL void DepthFirstDelete( CacheManager::freelist_t& fl,
                                              Context* cc, bool callReclaimContextObject )
    {
        Context::cptrlist_t l;
        cerr << "DepthFirstDelete(wrapper) calling recursive function" << endl;
        DepthFirstDelete( fl, cc, l, callReclaimContextObject );
        cerr << "DepthFirstDelete(wrapper) cleanup of freelist" << endl;
        
        Private::CacheManagerImpl* cm = Private::getCacheManagerImpl();
        for( Context::cptrlist_t::iterator iter = l.begin(); iter != l.end(); ++iter )
        {
//             cerr << "DepthFirstDelete(wrapper) cleanup of freelist:" << toVoid(*iter) << endl;
            cm->removeFromFreeList( *iter, true );
        }
//         cerr << "DepthFirstDelete(wrapper) done" << endl;
        cerr << "DepthFirstDelete(wrapper) ending" << endl;
    }

#ifdef PCCTSCTX
    FERRISEXP_DLLLOCAL void DepthFirstDeletePCCTS_DropInOderList( childContext* cc )
    {
        for( Context::Items_t::iterator ci = cc->getItems().begin();
             ci != cc->getItems().end(); ++ci )
        {
            childContext* c = dynamic_cast<childContext*>(GetImpl(*ci));
            DepthFirstDeletePCCTS_DropInOderList( c );
        }
        cc->InOrderInsertList.clear();
    }
    
    
    /**
     * Delete a PCCTS mount point. We must delete children first, but also first
     * drop InOrderList references.
     */
    FERRISEXP_DLLLOCAL void DepthFirstDeletePCCTS( CacheManager::freelist_t& fl, childContext* cc )
    {
#ifdef FERRIS_DEBUG_VM
        cerr << "DepthFirstDeletePCCTS(1) cc:" << cc->getDirPath() << endl;
#endif
        DepthFirstDeletePCCTS_DropInOderList( cc );

#ifdef FERRIS_DEBUG_VM
        cerr << "DepthFirstDeletePCCTS(2) cc:" << cc->getDirPath() << endl;
        DEBUG_dumpcl("DepthFirstDeletePCCTS()");
        cerr << "DepthFirstDeletePCCTS(3) cc:" << cc->getDirPath() << endl;
#endif

        DepthFirstDelete( fl, cc );

#ifdef FERRIS_DEBUG_VM
        cerr << "DepthFirstDeletePCCTS(end)" << endl;
        DEBUG_dumpcl("DepthFirstDeletePCCTS(end)");
#endif
    }
#endif
    
    CacheManager::fh_insideResolve
    CacheManager::getInsideResolve()
    {
        return new InsideResolve( this );
    }

    bool
    CacheManager::insideCleanupCall()
    {
        return m_insideCleanupCall;
    }
    

    int
    CacheManager::cleanUp_only_CreateMetaDataContext( bool force )
    {
        int ret = 0;

        cerr << "CacheManager::cleanUp_only_CreateMetaDataContext(T)" << endl;
        Util::ValueRestorer< bool > _obj_insideCleanupCall( m_insideCleanupCall, true );
        if( m_insideResolveCall )
        {
            return ret;
        }

        CacheManager::freelist_t ctxlist = getFreeList();
        int ctxlistsize = ctxlist.size();
        cerr << "CacheManager::cleanUp_only_CreateMetaDataContext() ctxlistsize:" << ctxlistsize << endl;

        for( freelist_t::iterator iter = ctxlist.begin(); iter != ctxlist.end(); ++iter )
        {
            Context* c = (*iter);
            if( !c )
                continue;

            if( !c->isParentBound() )
            {
                cerr << "no parent bound.... c:" << (void*)c << endl;
                cerr << "type:" << Loki::TypeInfo( typeid(c) ).name() << endl;
                cerr << "url:" << c->getURL() << endl;

                if( f_mdcontext* cc = dynamic_cast<f_mdcontext*>(c) )
                {
                    cerr << "SHOULD FREE xCMDC:" << (void*)cc << endl;
                }
                
                /**
                 * Freeing a metadata context tree which is out of scope.
                 */
                if( CreateMetaDataContext* cc = dynamic_cast<CreateMetaDataContext*>( c ) )
                {
                    cerr << "SHOULD FREE CMDC:" << (void*)cc << endl;
                    if( !c->getItems().empty() )
                    {
                        cerr << "VM ERROR. Attempt to free a cmdc which still has children."
                             << " cc:" << (void*)cc << endl;
                        continue;
                    }
                        
                    ++ret;
                    Private::CacheManagerImpl* cm = Private::getCacheManagerImpl();
                    cm->removeFromFreeList( cc, true );
                    delete cc;
                    getFreeList().erase(c);
                    continue;
                }
            }
        }
        return ret;
    }
    

    /**
     * Remove any contexts that are not needed and can be recreated.
     */
    int
    CacheManager::cleanUp( bool force )
    {
        Util::ValueRestorer< bool > _obj_insideCleanupCall( m_insideCleanupCall, true );

#ifdef FERRIS_DEBUG_VM
        LG_VM_D << "CacheManager::cleanUp( force=" << force << " )" << endl;
#endif

        if( m_insideResolveCall )
        {
#ifdef FERRIS_DEBUG_VM
            LG_VM_D << "CacheManager::cleanUp( inside resolve() by:" << m_insideResolveCall
                    << " returning )" << endl;
#endif
            return 0;
        }
        
        
        /*
         * Only allow one call to cleanup to process at once, any other calls
         * happening at the same time are expected to be generated by the
         * system while processing this cleanup.
         */
        static bool IgnoreCallsWhileExists_ignoring = false;
        IgnoreCallsWhileExists IgnoreCallsWhileExists_obj( &IgnoreCallsWhileExists_ignoring );
        if( IgnoreCallsWhileExists_obj() )
            return 0;

        int ret = 0;
        freelist_t rejects;

        /*
         * Copy and clear the free list so that in the course of freeing contexts
         * new ones can be added to the free list for next round.
         */
        CacheManager::freelist_t ctxlist = getFreeList();
        int ctxlistsize = ctxlist.size();


#ifdef FERRIS_DEBUG_VM
        LG_VM_D << "CacheManager::cleanUp(starting) existing freelist sz:"
                << ctxlist.size() << endl;
        for( freelist_t::iterator iter = ctxlist.begin(); iter != ctxlist.end(); ++iter )
        {
            LG_VM_D << "CacheManager::cleanUp(starting) item:" << toVoid( *iter )
                    << " rdn:" << (*iter)->getDirName() << endl;
        }
        LG_VM_D << "CacheManager::cleanUp( ctxlistsize.sz:" << ctxlistsize << "  )" << endl;
#endif
        
        if( !force && maxNumberOfContextsInFreeList > 0
            && ctxlistsize <= maxNumberOfContextsInFreeList )
        {
#ifdef FERRIS_DEBUG_VM
            LG_VM_D << "CacheManager::cleanUp(ret) not doing anything this call"
                    << " force:" << force
                    << " maxNumberOfContextsInFreeList:" << maxNumberOfContextsInFreeList
                    << " ctxlistsize:" << ctxlistsize
                    << endl;
#endif
            return 0;
        }
        
        if( numberOfAllowedPermanentContextsInFreeList > 0 )
        {
            if( ctxlistsize <= numberOfAllowedPermanentContextsInFreeList )
            {
#ifdef FERRIS_DEBUG_VM
                LG_VM_D << "CacheManager::cleanUp(ret) not doing anything this call2 "
                        << " force:" << force
                        << " numberOfAllowedPermanentContextsInFreeList:" << numberOfAllowedPermanentContextsInFreeList
                        << " ctxlistsize:" << ctxlistsize
                        << endl;
#endif
                return 0;
            }
        }

#ifdef FERRIS_DEBUG_VM
        cerr << "CacheManager::cleanUp() starting work..." << endl;
#endif
        getFreeList().clear();
        LG_VM_D << "--------------------------------------------------------------------------------\n";
        LG_VM_D << "VM Autocleanup call all.contexts:" << getMMCtx().size() << endl;
        LG_VM_D << "--------------------------------------------------------------------------------\n";
        for( freelist_t::iterator iter = ctxlist.begin(); iter != ctxlist.end(); ++iter )
        {
            Context* c = (*iter);
            LG_VM_D << "  ctxlist.iter:" << toVoid(c) << " rdn:" << c->getDirName() << endl;
        }
        LG_VM_D << "--------------------------------------------------------------------------------\n";
        
        for( freelist_t::iterator iter = ctxlist.begin(); iter != ctxlist.end();  )
        {
            bool itermoved = false;
            LG_CTX_D << "CacheManager::cleanUp(A) " << endl;

            if( maxNumberOfContextsToFreeAtOnce > 0
                && ret >= maxNumberOfContextsToFreeAtOnce )
            {
                copy( iter, ctxlist.end(), inserter( rejects, rejects.end() ) );
                LG_VM_D << "Freed all we should at one time." << endl;
                cerr << "Freed all we should at one time." << endl;
                break;
            }

//            DEBUG_dumpcl("CacheManager::cleanUp(iteration...)");
            
            
//            if( *iter )
            {
                Context* c = (*iter);
                {
                    freelist_t::iterator t = iter;
                    ++iter;
                    itermoved = true;
                    ctxlist.erase( t );
                }
//                cerr << "starting with c:" << (void*)c << endl;
                
                LG_CTX_D << "CacheManager::cleanUp(B) " << endl;
                if( !c )
                    continue;

                c->WeAreInFreeList = false;

#ifdef FERRIS_DEBUG_VM
                cerr << "CacheManager::cleanUp( 8 ) c:" << toVoid(c) << endl;
                cerr << "CacheManager::cleanUp( 8 ) rc:" << c->ref_count << endl;
                cerr << "CacheManager::cleanUp( 8 ) rdn:" << c->getDirName() << endl;
//                DEBUG_dumpcl("CacheManager::cleanUp");
#endif

                cerr << "CacheManager::cleanUp0 c:" << (void*)c << endl;
                if( c )
                {
                    cerr << "CacheManager::cleanUp0 c.rc:" << c->ref_count << endl;
                }
                
                

// This block was uncommented but seems incorrect.
//                 /*
//                  * An overmount for something like XML will have a CoveredContext
//                  * and no OverMountContext.
//                  *
//                  * Call the OverMountContext OMC and the XML base context CC
//                  *
//                  * The CC context will have all of its children claimable and
//                  * the CC will have a single outstanding reference (from the OMC to CC).
//                  */
//                 if( c->isParentBound() )
//                 {
//                     Context* omc = c->getParent();
                    
//                     LG_VM_D << "Perhaps have found the root of an overmount tree."
//                             << " c:" << toVoid(omc)
//                             << " rdn:" << omc->getDirName()
//                             << " cc:" << isBound( omc->CoveredContext )
//                             << " omc:" << isBound( omc->OverMountContext_Delegate )
//                             << " c.rc:" << omc->ref_count
//                             << " c.mRC:" << omc->getMinimumReferenceCount()
//                             << endl;

//                     if( isBound( omc->OverMountContext_Delegate ) && !isBound( omc->CoveredContext ) )
//                     {
//                         Context* cc = GetImpl( omc->OverMountContext_Delegate );
                        
//                         LG_VM_D << "Have found the root of an overmount tree."
//                                 << " c:" << toVoid(cc)
//                                 << " c.rc:" << cc->ref_count
//                                 << " c.mRC:" << cc->getMinimumReferenceCount()
//                                 << endl;
//                         cerr << "Have found the root of an overmount tree."
//                                 << " c:" << toVoid(cc)
//                                 << " c.rc:" << cc->ref_count
//                                 << " c.mRC:" << cc->getMinimumReferenceCount()
//                                 << endl;
//                         if( cc->ref_count = (cc->getMinimumReferenceCount()+1) )
//                         {
//                             LG_VM_D << "Calling depthFirstDelete on whole overmount tree:"
//                                     << toVoid(cc)
//                                     << " cc.items().size:" << cc->getItems().size()
//                                     << " omc.items().size:" << omc->getItems().size()
//                                     << endl;

// //                            DepthFirstDelete( cc, false );
//                             cerr << "About to unOverMount()" << endl;
//                             fh_context base = cc->CoveredContext;
//                             base->unOverMount( ctxlist );
//                             cerr << "Done with to unOverMount()" << endl;
//                             base->NumberOfSubContexts = 0;
//                             DEBUG_dumpcl("CacheManager::cleanUp(overmount)");
//                             LG_VM_D << "Cleaned up1 whole overmount tree:" << toVoid(cc) << endl;
// //                             Private::CacheManagerImpl* cm = Private::getCacheManagerImpl();
// //                             cm->removeFromFreeList( cc, true );
// //                             delete cc;
//                             LG_VM_D << "Cleaned up2 whole overmount tree:" << toVoid(cc) << endl;
// //                            continue;

//                             iter = ctxlist.begin();
//                             continue;
// //                             copy( iter, ctxlist.end(), inserter( rejects, rejects.end() ) );
// //                             copy( rejects.begin(), rejects.end(),
// //                                   inserter(getFreeList(), getFreeList().end() ));
                            
// //                             return 1;
//                         }
//                     }
//                 }
                
                
                
                if( !c->isParentBound() )
                {
                    cerr << "no parent bound.... c:" << (void*)c << endl;
                    /**
                     * Freeing a metadata context tree which is out of scope.
                     */
                    if( CreateMetaDataContext* cc = dynamic_cast<CreateMetaDataContext*>( c ) )
                    {
                        cerr << "SHOULD FREE CMDC:" << (void*)cc << endl;
                        if( !c->getItems().empty() )
                        {
                            cerr << "VM ERROR. Attempt to free a cmdc which still has children."
                                 << " cc:" << (void*)cc << endl;
                            continue;
                        }
                        
                        ++ret;
                        Private::CacheManagerImpl* cm = Private::getCacheManagerImpl();
                        cm->removeFromFreeList( cc, true );
                        delete cc;
                        continue;
                    }
                    
                    cerr << "no parent bound c:" << (void*)c << " have-cc:" << isBound( c->CoveredContext ) << endl;
                    if( !isBound( c->CoveredContext ) )
                    {
#ifdef PCCTSCTX
                        /*
                         * Freeing a whole PCCTS mount
                         */
                        if( childContext* cc = dynamic_cast<childContext*>(c))
                        {
//#ifdef FERRIS_DEBUG_VM
                            cerr << "Freeing PCCTS mount rooted at cc:" << toVoid(cc) << endl;
//#endif
                            DepthFirstDeletePCCTS( ctxlist, cc );
                            iter = ctxlist.begin();
                            continue;
                        }
#endif    
                        LG_VM_D << "Not freeing root node:" << c->getDirPath() << endl;
                        continue;
                    }
                    

                    /*
                     * VM.clean.3: Can cleanup a overmount tree when the root node has rc=2
                     */
                    LG_VM_D << "Cleaning up overmount tree p:" << c->getDirPath() << endl;

                    /*
                     * UnOvermount the context
                     */
                    cerr << " unovermount cc:" << (void*)GetImpl(c->CoveredContext) << endl;
                    fh_context base = c->CoveredContext;
                    freelist_t deletedItems;
                    base->unOverMount( deletedItems );
                    cerr << " called base->unovermount cc:" << (void*)GetImpl(c->CoveredContext) << endl;

                    cerr << "deletedItems.size:" << deletedItems.size() << endl;
                    
                    {
                        // delete the contexts from the ctxlist which were already
                        // deleted by unOverMount()
                        for( freelist_t::iterator iter = deletedItems.begin();
                             iter != deletedItems.end(); ++iter )
                        {
                            ctxlist.erase( *iter );
                        }
                        cerr << "erased deleted items" << endl;
                        
                        // Now, build a lookup from the parent -> context of all the existing
                        // contexts on ctxlist. Then make sure that anything which has a parent
                        // that was deleted is also deleted.
                        // This will happen for example if there is a ./foo.db/a/b
                        // which was remove()d.
                        typedef std::map< Context*, Context* > parent_lookup_t;
                        parent_lookup_t parent_lookup;
                        for( freelist_t::iterator iter = ctxlist.begin();
                             iter != ctxlist.end(); ++iter )
                        {
                            Context* c = (*iter);
                            if( c && c->isParentBound() )
                            {
                                cerr << "adding to pl:" << c->getParent() << " c:" << c << endl;
                                parent_lookup.insert( make_pair( c->getParent(), c ) );
                            }
                        }
                        cerr << "build parent lookup..." << endl;
                        for( freelist_t::iterator iter = deletedItems.begin();
                             iter != deletedItems.end(); ++iter )
                        {
                            cerr << "deletedItem...:" << *iter << endl;
                            parent_lookup_t::iterator pi = parent_lookup.find( *iter );
                            if( parent_lookup.end() != pi )
                            {
                                cerr << "found c:" << pi->second << " with a deleted parent!" << endl;
                                cerr << " 1ctxlist.size:" << ctxlist.size() << endl;
                                Context* cc = pi->second;
                                ctxlist.erase( cc );
                                cerr << " 2ctxlist.size:" << ctxlist.size() << endl;
                                Private::CacheManagerImpl* cm = Private::getCacheManagerImpl();
                                cm->removeFromFreeList( cc, true );
                                cc->ParentContext.ExplicitlyLeakPointer();
                                cc->CoveredContext.ExplicitlyLeakPointer();
                                cc->OverMountContext_Delegate.ExplicitlyLeakPointer();
                                Factory::getPluginOutOfProcNotificationEngine().forgetContext( cc );
                                delete cc;
                            }
                        }
//                         for( freelist_t::iterator iter = deletedItems.begin(); iter != deletedItems.end();  )
//                         {
//                             Context* cc = *iter;
//                             ++iter;
//                             cerr << "finally deleting cc:" << (void*)cc << endl;
//                             delete cc;
//                         }
                    }
                    
                    
                    iter = ctxlist.begin();
                    continue;
                }
                
                string path = c->getDirPath();
                LG_VM_D << "CacheManager::cleanUp c:" << (void*)c
                        << " path:" << path
                        << " supportsReClaim:" << c->supportsReClaim()
                        << endl;
                cerr << "CacheManager::cleanUp1 has-parent:" << c->isParentBound()
                     << " c:" << (void*)c
                     << " path:" << path
                     << " supportsReClaim:" << c->supportsReClaim()
                     << endl;

                // Lets remove the overmount context before the coveredcontext.
                if( isBound( c->OverMountContext_Delegate ) )
                    continue;
                
                if( !c->supportsReClaim() )
                {
                    LG_CTX_D << "CacheManager::cleanUp() removing context from freelist "
                             << "due to lack of reclaim in VFS module for path:"
                             << c->getDirPath()
                             << endl;
#ifdef FERRIS_DEBUG_VM
                    cerr << "CacheManager::cleanUp() removing context from freelist "
                             << "due to lack of reclaim in VFS module for path:"
                             << c->getDirPath()
                             << endl;
#endif
                    continue;
                }
                

                
                try
                {
                    /*
                     * delete from children to parents. If this is able to be cleaned up
                     * but has children which can be cleaned up too, we don't clean the
                     * parent until next time.
                     */
                    if( !c->getItems().empty() )
                    {
                        LG_VM_D << "cleanup() skipping context with children c:" << toVoid(c) << endl;
                        cerr << "cleanup() skipping context with children c:" << toVoid(c) << endl;
                        c->WeAreInFreeList = 1;
                        rejects.insert( c );
                        continue;
                    }
                    
                    
                    /*
                     * Quick check to make sure they are ready to die
                     */
                    if( c->ref_count > c->getMinimumReferenceCount() )
                    {
                        c->WeAreInFreeList = 1;
                        rejects.insert( c );
                        stringstream ss;
                        ss << "VM ERROR: object in the freelist with high reference count: "
                           << toVoid(c);
                        cerr << ss.str() << endl;
                        LG_VM_W << ss.str() << endl;
                        continue;
                    }

                    // Note that we hit the parent directly to avoid bump/drop of the ref for
                    // the parent and subsequent parent tree add/remove from the free list.
                    Context* p = c->getParent(); 
                    
                    cerr << "CacheManager::cleanUp2 c:" << (void*)c
                         << " p:" << (void*)p
                         << " path:" << path
                         << " supportsReClaim:" << c->supportsReClaim()
                         << endl;

                    LG_VM_D << "CacheManager::cleanUp() path: " << path << endl;
                    if( p )
                    {
//                        DEBUG_dumpcl("CacheManager::cleanUp(about to reclaim)");
                        if( !p->reclaimContextObject( c ) )
                        {
                            LG_VM_D << "CacheManager::cleanUp(remove rejected)"
                                    << " path:" << path << " c:" << toVoid(c) << endl;
//#ifdef FERRIS_DEBUG_VM
                            cerr << "CacheManager::cleanUp(remove rejected)"
                                 << " path:" << path << endl;
//#endif
                            c->WeAreInFreeList=1;
                            rejects.insert( c );
                        }
                        else
                        {
                            cerr << "CacheManager::cleanUp(reclaim was ok) freelist sz:"
                                 << getFreeList().size() << endl;
#ifdef FERRIS_DEBUG_VM
                            cerr << "CacheManager::cleanUp(reclaim was ok) freelist sz:"
                                 << getFreeList().size() << endl;
                            for( freelist_t::iterator iter = getFreeList().begin(); iter != getFreeList().end(); ++iter )
                            {
                                cerr << "CacheManager::cleanUp(reclaim was ok) item:" << toVoid( *iter ) << endl;
                            }
                            
                            DEBUG_dumpcl("CacheManager::cleanUp(done with reclaim)");
#endif
                            LG_VM_D << "CacheManager::cleanUp(removed)" 
                                     << " path:" << path << endl;
                        }
                    }
                    ret++;
                }
                catch( exception& e )
                {
                    LG_CTX_ER << "CacheManager::cleanUp() clist exception:" << e.what() << endl;
                    cerr << "CacheManager::cleanUp() clist exception:" << e.what() << endl;
                    LG_CTX_ER << "CacheManager::cleanUp() path:" << path << endl;

                    c->WeAreInFreeList=1;
                    rejects.insert( c );
                    if( !itermoved )
                    {
                        iter++;
                    }
                }
            }
        
        }

        LG_VM_D << "CacheManager::cleanUp(remake CList) cleaned:"
                << ret << " contexts rejects.size:" << rejects.size() << endl;

//         cerr << "CacheManager::cleanUp(end) existing freelist sz:"
//              << getFreeList().size() << endl;
//         for( freelist_t::iterator iter = getFreeList().begin(); iter != getFreeList().end(); ++iter )
//         {
//             cerr << "CacheManager::cleanUp(end) item:" << toVoid( *iter ) << endl;
//         }
        
        
        copy( rejects.begin(), rejects.end(),
              inserter(getFreeList(), getFreeList().end() ));

        DEBUG_dumpcl("CacheManager::cleanUp(exiting)");
        LG_VM_D << "CacheManager::cleanUp() cleaned:" << ret << " contexts" << endl;
        return ret;
    }
    
    namespace Private
    {
        void addContextReferenceWatch( Context* c )
        {
            getContextReferenceWatches().insert( c );
            haveAnyContextReferenceWatches_Result = true;
        }
        
        void removeContextReferenceWatch( Context* c )
        {
            getContextReferenceWatches().erase( c );
            haveAnyContextReferenceWatches_Result = !getContextReferenceWatches().empty();
        }
        ContextReferenceWatches_t& getContextReferenceWatches()
        {
            static ContextReferenceWatches_t ret;
            return ret;
        }

        void addContextReferenceWatchByName( const std::string& n )
        {
            getContextReferenceWatchesByName().insert( n );
            haveAnyContextReferenceWatchesByName_Result = true;
        }
        void removeContextReferenceWatchByName( const std::string& n )
        {
            getContextReferenceWatchesByName().erase( n );
            haveAnyContextReferenceWatchesByName_Result = !getContextReferenceWatchesByName().empty();
        }
        ContextReferenceWatchesByName_t& getContextReferenceWatchesByName()
        {
            static ContextReferenceWatchesByName_t ret;
            return ret;
        }
        
        
        void addContextParentReferenceWatch( Context* c )
        {
            getContextParentReferenceWatches().insert( c );
            haveAnyContextParentReferenceWatches_Result = true;
        }
        void removeContextParentReferenceWatch( Context* c )
        {
            getContextParentReferenceWatches().erase( c );
            haveAnyContextParentReferenceWatches_Result = getContextParentReferenceWatches().empty();
        }
        ContextReferenceWatches_t& getContextParentReferenceWatches()
        {
            static ContextReferenceWatches_t ret;
            return ret;
        }


        bool haveAnyContextReferenceWatches()
        {
            return haveAnyContextReferenceWatches_Result;
        }
        
        bool haveAnyContextReferenceWatchesByName()
        {
            return haveAnyContextReferenceWatchesByName_Result;
        }
        
        bool haveAnyContextParentReferenceWatches()
        {
            return haveAnyContextParentReferenceWatches_Result;
        }

    };
    
    
};


