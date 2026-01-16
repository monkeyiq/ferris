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

    $Id: Cache.hh,v 1.6 2010/09/24 21:30:25 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_CACHE_H_
#define _ALREADY_INCLUDED_FERRIS_CACHE_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <Ferris/TypeDecl.hh>
#include <FerrisLoki/Extensions.hh>

#include <sigc++/sigc++.h>
#include <sigc++/slot.h>

#include <glib.h>

#include <string>
#include <map>
#include <list>
#include <iostream>

namespace Ferris
{
    /**
     * Objects that can exist inside a cache should inherit from this class
     * instead of just Handlable. Note that at current a single cacheHandlable
     * can only exist in one cache.
     */
    class FERRISEXP_API CacheHandlable
        :
        public Handlable 
    {
        typedef Handlable _Base;

        int getNumberOfCachesWeAreIn()
            {
                return 1;
            }
        
    public:
        typedef sigc::signal< void ( CacheHandlable* ) > OnlyInCacheSignal_t;
        OnlyInCacheSignal_t OnlyInCacheSignal;
        OnlyInCacheSignal_t& getOnlyInCacheSignal()
            {
                return OnlyInCacheSignal;
            }
    
        CacheHandlable()
            :
            Handlable()
            {
            }
    
        virtual ref_count_t AddRef()
            {
                return _Base::AddRef();
            }
    
        virtual ref_count_t Release()
            {
                ref_count_t ret = _Base::Release();
                if( ret == getNumberOfCachesWeAreIn() )
                {
                    getOnlyInCacheSignal().emit( this );
                }
                return ret;
            }

        /**
         * called when the ref_count drops to one to possibly save
         * the state of the object to disk
         */
        virtual void sync()
            {
            }
    };

    /**
     * Create a cache of objects keyed of some data like a string or pointer.
     */
    template< class Key, class Value >
    class FERRISEXP_API Cache
        :
        public sigc::trackable
    {
        typedef Cache<Key,Value> _Self;
    
        typedef std::map< Key, Value > m_t;
        m_t m;

        typedef std::list< FerrisLoki::Handlable* > m_collectable_t;
        m_collectable_t m_collectable;

        // when to start reaping collectable items
        int m_max_collectable_size;

        // glib timer number
        guint m_timer;

        // how many millis until timer should be called
        int m_timer_interval;
    
        /**
         * When a value drops its last reference
         */
        void OnGenericCloseSignal( CacheHandlable* valueptr )
            {
                m_collectable.push_back( valueptr );
                valueptr->sync();
            }

        /**
         * Every so often we reclaim the cache to avoid buildup
         * of many large objects in the cache that have not
         * been used for a while.
         */
        static gint s_timer_f(gpointer data)
            {
//                cerr << "s_timer_f()" << endl;
                _Self* sp = (_Self*)data;
                return sp->timer_f();
            }
        gint timer_f()
            {
//                 cerr << "timer_f() m.size:" << m.size()
//                      << " collectable.size:" << m_collectable.size()
//                      << endl;
                maybe_collect( m_collectable.size() );
                return 0;
            }

        /**
         * Make the timer happen m_timer_interval in the future
         */
        void reconnectTimer()
            {
                if( m_timer )
                {
                    g_source_remove( m_timer );
                    m_timer = 0;
                }
                if( m_timer_interval )
                    m_timer = g_timeout_add( m_timer_interval,
                                             GSourceFunc(s_timer_f), this );
            }
    
    
        /**
         * if there are too many outstanding m_collectable
         * then collect some of them
         *
         * @param count is the number of object to try to collect or zero
         *        if we just want to trim the cache to user prefs
         */
        void maybe_collect( int count = 0 )
            {
//                cerr << "maybe_collect(top) count:" << count << endl;
            
                if( !count )
                {
                    if( m_collectable.size() >= m_max_collectable_size )
                    {
                        count = m_collectable.size() - m_max_collectable_size;
                    }
                }
//                 cerr << "maybe_collect(2) count:" << count << endl;
//                 cerr << "maybe_collect(2) m_collectable.size:" << m_collectable.size() << endl;

                if( count > 0 && !m_collectable.empty() )
                {
                    count = std::min( (size_t)count, m_collectable.size() );
                    
                    for( int i=0; i<count; i++ )
                    {
                        FerrisLoki::Handlable* h = m_collectable.front();
                        m_collectable.pop_front();

//                         cerr << "cache<> m_collectable.size:" << m_collectable.size()
//                              << " count:" << count
//                              << " h:" << toVoid( h )
//                              << endl;
//                         cerr << " h.rc:" << h->getReferenceCount()
//                              << endl;
                        
                        if( h->getReferenceCount() > 1 )
                        {
                            // it was referenced again through the cache.
                            continue;
                        }

                        typedef typename m_t::iterator iterator;
                        for( iterator mi = m.begin(); mi!=m.end(); )
                        {
                            FerrisLoki::Handlable* mih = GetImpl(mi->second);
                            if( mih == h )
                            {
                                iterator delme = mi;
                                ++mi;
                                m.erase( delme );
                                continue;
                            }
                            ++mi;
                        }
                    }
                }

                if( !m_collectable.empty() )
                    reconnectTimer();
            }

    public:

        Cache()
            :
            m_max_collectable_size( 100 ),
            m_timer( 0 ),
            m_timer_interval( 20000 )
            {
            }

        /**
         * Try to find the item in the cache. Returns 0 if the item was
         * not put() or if it has been reclaimed since then
         */
        Value get( const Key& k )
            {
                typename m_t::iterator mi = m.find( k );
                if( mi != m.end() )
                {
                    // if it was claimable then we should purge it from
                    // the collectable list
                    if( mi->second->getReferenceCount() )
                    {
                        for( m_collectable_t::iterator ci = m_collectable.begin();
                             ci != m_collectable.end(); ++ci )
                        {
                            if( *ci == dynamic_cast<FerrisLoki::Handlable*>(GetImpl(mi->second)))
                            {
                                m_collectable.erase( ci );
                                break;
                            }
                        }
                    }
                    
//                     cerr << "got from cache. m.sz:" << m.size()
//                          << " m_collectable.size:" << m_collectable.size()
//                          << " ref_count:" << mi->second->getReferenceCount()
//                          << endl;
                    return mi->second;
                }
                return 0;
            }
    
        void put( const Key& k, Value& v )
            {
                if( !isBound(v) )
                    return;
                
                maybe_collect();
                reconnectTimer();
                v->getOnlyInCacheSignal().connect( mem_fun( *this, &_Self::OnGenericCloseSignal ) );
                m[ k ] = v;
            }

        void putNoCollect( const Key& k, Value& v )
            {
                if( !isBound(v) )
                    return;
                
                v->getOnlyInCacheSignal().connect( mem_fun( *this, &_Self::OnGenericCloseSignal ) );
                m[ k ] = v;
            }

        /**
         * Set the number of items in the cache that are not referenced outside the
         * cache that can remain in the cache without being purged
         */
        void setMaxCollectableSize( int v )
            {
                m_max_collectable_size = v;
            }

        /**
         * how many millis until timer should be called to try to reclaim cache.
         */
        void setTimerInterval( int v )
            {
                m_timer_interval = v;
                reconnectTimer();
            }

        /********************************************************************************/
        /*** allow iteration over the cache *********************************************/
        /********************************************************************************/

        typedef typename m_t::iterator iterator;
        iterator begin()
            {
                return m.begin();
            }
        iterator end()
            {
                return m.end();
            }

        
        /********************************************************************************/
        /*** mainly for unit testing. non-interesting otherwise *************************/
        /********************************************************************************/

        int getCollectableSize()
            {
                return m_collectable.size();
            }
    };
 
};
#endif
