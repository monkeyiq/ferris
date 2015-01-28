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

    $Id: CacheManager_private.hh,v 1.5 2010/09/24 21:30:25 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_CACHE_MGR_PRIV_H_
#define _ALREADY_INCLUDED_FERRIS_CACHE_MGR_PRIV_H_

namespace Ferris
{
    namespace Private
    {
        struct CacheManagerContextStateInTime_hash;
        struct CacheManagerContextStateInTime_equal_to;
        
        class FERRISEXP_API CacheManagerContextStateInTime
        {
        public:
            FerrisLoki::Handlable::ref_count_t ref_count;
            gint32 NumberOfSubContexts;
            gint32 ItemsSz;
            bool isReClaimable;
            bool WeAreInFreeList;
            long MinimumReferenceCount;
            Context* ContextThisPtr;
            bool isFilter;
            bool isSorter;
            bool HasBeenDeleted;
            bool isInheritingContext;
            ChainedViewContext* cvc;
            Context* Delegate;
            Context* CoveredContext;
            Context* OverMountContext_Delegate;
            bool isParentBound;
            Context* parent;
            Context* parentR;
            std::string name;
            std::string path;

            friend struct CacheManagerContextStateInTime_hash;
            friend struct CacheManagerContextStateInTime_equal_to;

            friend bool operator<( const CacheManagerContextStateInTime& k1, const CacheManagerContextStateInTime& k2 );
            friend bool operator==( const CacheManagerContextStateInTime& k1, const CacheManagerContextStateInTime& k2 );
            
            CacheManagerContextStateInTime( Context* c );
            ~CacheManagerContextStateInTime();

            std::string str() const;
        };
        typedef std::list< CacheManagerContextStateInTime > CacheManagerContextStateInTimeList_t;

        typedef std::set<CacheManagerContextStateInTime > CacheManagerContextStateInTimeIndexSet_t;
        
//         typedef FERRIS_STD_HASH_SET<CacheManagerContextStateInTime,
//                                      CacheManagerContextStateInTime_hash,
//                                      CacheManagerContextStateInTime_equal_to > CacheManagerContextStateInTimeIndexSet_t;

        CacheManagerContextStateInTimeIndexSet_t toIndexSet( CacheManagerContextStateInTimeList_t& l );
        
        /********************/
        /********************/
        /********************/
        
        class FERRISEXP_DLLLOCAL CacheManagerImpl
            :
            public CacheManager
        {
        public:
            void addToFreeList( Context* a );
            /**
             * Note that if quiet is true then the object might already have
             * been deleted, so no logging or examination is performed in quiet
             * mode only the collection is reeped of any pointers to the old location
             */
            void removeFromFreeList( Context* a, bool quiet = false );
            void AutoClean();

            Private::CacheManagerContextStateInTimeList_t&
            createCacheManagerContextStateInTimeList( Private::CacheManagerContextStateInTimeList_t& ret );

        };

        CacheManagerImpl* getCacheManagerImpl();

        
        void dumpTo( fh_ostream oss, CacheManagerContextStateInTimeList_t& list, const std::string& hdr );

        Private::CacheManagerContextStateInTimeList_t&
        createMMCtxInTimeList( Private::CacheManagerContextStateInTimeList_t& ret );

        void addContextReferenceWatch( Context* c );
        void removeContextReferenceWatch( Context* c );
        typedef FERRIS_STD_HASH_SET<Context*,
                                    f_hash<Context* const>,
                                    f_equal_to<Context* const> > ContextReferenceWatches_t;
        ContextReferenceWatches_t& getContextReferenceWatches();
        bool haveAnyContextReferenceWatches();

        void addContextReferenceWatchByName( const std::string& n );
        void removeContextReferenceWatchByName( const std::string& n );
        typedef FERRIS_STD_HASH_SET< std::string > ContextReferenceWatchesByName_t;
        ContextReferenceWatchesByName_t& getContextReferenceWatchesByName();
        bool haveAnyContextReferenceWatchesByName();

        void addContextParentReferenceWatch( Context* c );
        void removeContextParentReferenceWatch( Context* c );
        ContextReferenceWatches_t& getContextParentReferenceWatches();
        bool haveAnyContextParentReferenceWatches();
        
    }
    
};
#endif
