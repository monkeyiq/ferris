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

    $Id: ContextIterator.cpp,v 1.9 2010/09/24 21:30:27 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <ContextIterator.hh>
#include <Ferris.hh>
#include <Ferris_private.hh>
#include <Context_private.hh>

using namespace std;


namespace Ferris
{
    int ImplicitIteratorUpdateLock::lock = 0;
    
    ImplicitIteratorUpdateLock::ImplicitIteratorUpdateLock()
    {
        ++lock;
    }
    
    ImplicitIteratorUpdateLock::~ImplicitIteratorUpdateLock()
    {
        --lock;
    }
    
    bool
    ImplicitIteratorUpdateLock::isTaken()
    {
        return lock;
    }
    

    template <class T>
    T ForceNeg( T x )
    {
        if( x >= 0 ) return -1 * x;
        return x;
    }

    struct FERRISEXP_DLLLOCAL ContextIteratorData_OldRdnSet : public Handlable
    {
        typedef vector< string > OldRdnSet_t;
        OldRdnSet_t m_OldRdnSet;
        VersionWatcher<ContextDirOpVersion_t> m_Version;

        ContextIteratorData_OldRdnSet()
            {
            }

        ContextIteratorData_OldRdnSet( fh_context& theParent )
            {
                m_OldRdnSet.clear();
                m_OldRdnSet.reserve( theParent->getItems().size() + 2 );

                m_Version = theParent->getDirOpVersion();
                Context::Items_t items = theParent->getSortedItems();
                Context::Items_t::iterator iter = items.begin();
                for( ; iter != items.end(); ++iter )
                {
                    m_OldRdnSet.push_back( (*iter)->getDirName() );
                }
            }
        
        ContextIteratorData_OldRdnSet*
        update( bool force, fh_context& theParent )
            {
                if( force
                    || !m_Version.test( theParent->getDirOpVersion())
                    || m_OldRdnSet.empty()
                    )
                {
                    return new ContextIteratorData_OldRdnSet( theParent );
                }
                return 0;
            }


        string getNextRDN( const std::string& rdn )
            {
                OldRdnSet_t::iterator oldrdniter
                    = find( m_OldRdnSet.begin(), m_OldRdnSet.end(), rdn );
                if( oldrdniter != m_OldRdnSet.end() )
                    ++oldrdniter;
                    
                string wanted_rdn = "";
                if( oldrdniter != m_OldRdnSet.end() )
                {
                    wanted_rdn =  *oldrdniter;
                }
            }
    };
    FERRIS_SMARTPTR( ContextIteratorData_OldRdnSet, fh_ContextIteratorData_OldRdnSet );
    
    struct FERRISEXP_DLLLOCAL ContextIteratorData
    {
        /**
         * This is what we are pointing to currently, note that we are iterating
         * over the contexts in theContext's parent context.
         */
        fh_context theContext;
        fh_context theParent;

        /**
         * A cache of theContext ie. theContext->getParent()->getSortedItems().find( rdn );
         */
        Context::Items_t::iterator theContextIterator;
        
        VersionWatcher<ContextDirOpVersion_t> Version;
        std::string rdn;
        bool revalidateDidntFindOldContext;
        bool isSortedContext;

//         typedef vector< string > ForSortedOnly_OldRdnSet_t;
//         ForSortedOnly_OldRdnSet_t ForSortedOnly_OldRdnSet;
        fh_ContextIteratorData_OldRdnSet m_ForSortedOnly_OldRdnSet;
        void m_ForSortedOnly_OldRdnSet_reset()
            {
                m_ForSortedOnly_OldRdnSet = new ContextIteratorData_OldRdnSet();
            }
        
        /**
         * end() should be stable. We can just cache one of these for our lifetime.
         */
        Context::Items_t::iterator m_endIter;
        
        ContextIteratorData()
            :
            revalidateDidntFindOldContext( false ),
            isSortedContext( false )
            {
                m_ForSortedOnly_OldRdnSet_reset();
            }

        bool isTheContextIteratorSet()
            {
                if( theContext )
                    return theContextIterator != theContext->getParent()->getSortedItems().end();
                return false;
            }

        gpointer getSortedItemsPointer()
            {
                return &theParent->getSortedItems();
            }
        Context::Items_t getSortedItems()
            {
                return theParent->getSortedItems();
            }

        
        
        void dump( const std::string& s )
            {
                cerr << s << endl;
                fh_context c = theParent;
                if( theContext && !c )
                    c = theContext->getParent();
                c->dumpOutItems();
            }
        
        void update_OldRdnSet_cache( bool force = false )
            {
                /* Only do anything if we are sorted and the version has changed */
                if( isSortedContext )
                {
                    if( ContextIteratorData_OldRdnSet* replacement =
                        m_ForSortedOnly_OldRdnSet->update( force, theParent ) )
                    {
                        m_ForSortedOnly_OldRdnSet = replacement;
                    }
                }
            }
        
        
        /**
         * Check if the context has changed and if so reset the iterator to the correct
         * location in the updated context.
         */
        void revalidate( bool force = false )
            {
                /* Not much to revalidate if we are at end() */
                if( !isBound( theContext ))
                {
                    theContextIterator = theParent->getSortedItems().end();
                    m_ForSortedOnly_OldRdnSet_reset();
                    return;
                }

                if( !ImplicitIteratorUpdateLock::isTaken() )
                {
                    if( theContext->supportsMonitoring() )
                    {
                        /* Update the contexts if something has changed */
                        Main::processAllPending_VFSFD_Events();
                    }
                }
                
                
                revalidateDidntFindOldContext = false;
                if( !force && Version( theParent->getDirOpVersion() ) )
                {
                    /* This are still the same */
                    return;
                }

                /* PURE DEBUG */
//                 if( !force )
//                 {
//                     cerr << "validate changed";
//                     if( isBound( theContext ) )
//                         cerr << " for c:" << theContext->getURL();
//                     cerr << " force:" << force
//                          << " rdncache.size():" << ForSortedOnly_OldRdnSet.size()
//                          << endl;
//                 }
                
                
//                cerr << "CI::revalidate(1)" << endl;
                /*
                 * Quick check to see if changes are not to current iterator
                 */
                Context::Items_t& items = theParent->getSortedItems();

                Context::Items_t::iterator ti =
                    theParent->ctx_lower_bound( theParent->getItems(), rdn );
//                 Context::Items_t::iterator ti = ex_lower_bound( theParent->getItems(),
//                                                                 rdn,
//                                                                 items_lookup_compare() );
//                 Context::Items_t::iterator ti =
//                     lower_bound( theParent->getItems().begin(),
//                                  theParent->getItems().end(),
//                                  rdn,
//                                  items_lookup_compare() );

                
                /* Can we shortcut out of here? */
                if( !force
                    && ti != items.end() && GetImpl( *ti ) == GetImpl(theContext))
                {
                    /* We rely on Items_t being stable for both insert() and erase()
                     * http://www.sgi.com/tech/stl/Map.html
                     */
//                    cerr << "base iterator still valid. " << endl;

                    /* Update cache of rdn names for iterators into sorted data */
                    update_OldRdnSet_cache();
//                    cerr << "CI::revalidate(2)" << endl;

                    return;
                }

                
                /*
                 * Sorted contexts have to be revalidated in a completely different
                 * way. This is because new items are inserted into their correct
                 * place in the list when they are discovered.
                 */
                if( isSortedContext )
                {
//                    cerr << "CI::revalidate(3)" << endl;
//                    cerr << "reseting a sorted iterator old rdn:" << rdn << endl;
                    /*
                     * This is a little tricky, SortedContext always wraps its
                     * children as sortedcontext's too. During insert() into
                     * SortedItems a SortedContext sets the child's thisIterator
                     * to the iterator for the child in SortedItems. This way we
                     * can use the fast set<> indexed on name to find the subcontext
                     * and then use thisIterator to get an iterator into the SortedItems
                     * collection.
                     */
                    if( theParent->isSubContextBound( rdn ) )
                    {
                        fh_context tmp = theParent->getItem( rdn );
                        if( SortedContext* sc = dynamic_cast<SortedContext*>(GetImpl(tmp)))
                        {
                            theContextIterator = sc->thisIterator;
                            setContext( *theContextIterator );
//                             cerr << "Reseting iterator using short cut iter:"
//                                  << (*theContextIterator)->getURL()
//                                  << endl;

                            /* Update cache of rdn names for iterators into sorted data */
                            update_OldRdnSet_cache();
                            return;
                        }
                    }

//                    cerr << "CI::revalidate(4)" << endl;

                    
                    /*
                     * The item under the iterator has been removed. This is a very
                     * tricky thing for a sortedContext because the sorting function
                     * can be chained and rely on any EA of all the siblings to
                     * determine its ordering. The only safe way to handle this is
                     * to use a collection of the old rdn and the old rdn name and
                     * keep moving towards end() from where the item used to be until
                     * we find a context that is currently still bound. A rather costly
                     * solution but this only happens when the iterator is over a context
                     * that was deleted from under it.
                     *
                     * NOTE: theContextIterator
                     *       MUST use getSortedItems()
                     *       CANT use iterators into getItems()
                     */
                    string wanted_rdn = m_ForSortedOnly_OldRdnSet->getNextRDN( rdn );
                    
                    typedef Context::Items_t Items_t;
                    Context::Items_t& items = theParent->getSortedItems();
                    theContextIterator = items.end();
                    
//                     cerr << "Reseting sorted iter rdn:" << rdn
//                          << " wanted_rdn:" << wanted_rdn
//                          << " rdncache.size():" << ForSortedOnly_OldRdnSet.size()
//                          << endl;
                    
                    if( wanted_rdn.length() )
                    {
                        /* Get an iterator into SortedItems pointing at wanted_rdn */
                        for( Items_t::iterator iter = items.begin();
                             iter != items.end(); ++iter )
                        {
                            theContextIterator = iter;
                            
                            string iter_rdn = (*iter)->getDirName();
                            if( iter_rdn == wanted_rdn )
                            {
//                                 cerr << "Breaking for iter_rdn:" << iter_rdn
//                                      << " wanted_rdn:" << wanted_rdn
//                                      << endl;
                                break;
                            }
                        }
                    }
                    
                    if( theContextIterator == items.end()
                        || (*theContextIterator)->getDirName() != rdn )
                    {
                        /* We have moved to a different item */
                        revalidateDidntFindOldContext = true;
                    }

                    /* PURE DEBUG */
                    {
//                         string earl = "(end)";
//                         if( theContextIterator != items.end() )
//                         {
//                             earl = (*theContextIterator)->getURL();
//                         }
//                         cerr << "reset sorted iterator the long way "
//                              << " old rdn:" << rdn
//                              << " new c:" << earl
//                              << " didntfindold:" << revalidateDidntFindOldContext
//                              << endl;
                    }

                    if( theContextIterator == items.end() )
                        setContext( 0 );
                    else
                        setContext( *theContextIterator );

                    /* Update cache of rdn names for iterators into sorted data */
                    update_OldRdnSet_cache();
                    
                    return;
                }

                /*
                 * Something has changed in the parent of theContext so we
                 * need to revalidate all the current data and maybe reset it to
                 * new values.
                 */
//                 Context::Items_t& items = theParent->getSortedItems();
//                 theContextIterator = items.lower_bound( rdn );
                /*
                 * Reuse 'ti' value from above.
                 * NOTE: we have culled out SortedContext above, so we know
                 * that getItems() == getSortedItems() at this point.
                 */
                theContextIterator = ti;
                if( theContextIterator == items.end()
                    || (*theContextIterator)->getDirName() != rdn )
                {
                    /* We have moved to a different item */
                    revalidateDidntFindOldContext = true;
                }
                setContext( *theContextIterator );
            }

        void setContext_NoChecking( const fh_context& c )
            {
                rdn = c->getDirName();
                theContext = c;
            }
        void setContext_ExplicitlyToNull()
            {
                rdn = "";
                theContext = 0;
            }
        
        /**
         * Set the context we are pointing to and keep enough info to
         * revalidate the iterator if the dir changes.
         */
        void setContext( const fh_context& c )
            {
                if( isBound(c) )
                {
                    rdn = c->getDirName();
                    theContext = c;
                }
                else
                {
                    rdn = "";
                    theContext = 0;
                }
            }
        
        /**
         * revalidate and move the iterator by n 
         */
        void shiftIterator( int n )
            {
                /* Are we end? */
                if( !isBound( theContext ))
                {
                    if( n >= 0 )
                        return;
                }
                
                revalidate();

                /*
                 * If we didn't refind the old context, then we are already advanced one
                 */
                if( revalidateDidntFindOldContext && n > 0 )
                    --n;

                
//                 /* map<> doesn't like getting --end(); */
//                 if( !isBound( theContext ))
//                 {
//                     typedef Context::SubContextNames_t cn_t;
//                     cn_t cn = theParent->getSubContextNames();
//                     theContextIterator = theParent->getItemIter( cn.back() );
//                     ++n;
//                 }

                
                // We should check if HasBeenDeleted is set 
//                advance( theContextIterator, n );
                bool beginIterIsValid = false;
                Context::Items_t::iterator beginIter; //  = theParent->getSortedItems().begin();
//                    Context::Items_t::iterator endIter   = theParent->getSortedItems().end();

//                    cerr << "Advance n:" << n << endl;
                    
                    for( int i = n; i!=0; )
                    {
                        if( n > 0 )  ++theContextIterator;
                        else
                        {
                            if( !beginIterIsValid )
                            {
                                beginIterIsValid = true;
                                beginIter = theParent->getSortedItems().begin();
                            }
                            
                            if( theContextIterator == beginIter )
                                break;
                            --theContextIterator;
                        }
                        
                        if( theContextIterator == m_endIter )
                        {
                            break;
                        }
                        if( (*theContextIterator)->HasBeenDeleted )
                            continue;

                        if( n > 0 ) --i;
                        else        ++i;

//                        cerr << "Advance.iter i:" << i << " n:" << n << endl;
                    }
                
                    if( theContextIterator == m_endIter )
                    {
                        rdn = "";
                        theContext = 0;
                    }
                    else
                    {
//                        theContext = *theContextIterator;
                        theContext = GetImpl(*theContextIterator);
                        rdn = theContext->getDirName();
                    }
            }
        
    };
    
    inline struct FERRISEXP_DLLLOCAL ContextIteratorData* Data( const ContextIterator* ci )
    {
        return (ContextIteratorData*)ci->p_data;
    }
    #define DATA Data(this)
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    ContextIterator::ContextIterator()
        :
        p_data(0)
    {
        p_data = new ContextIteratorData();
        DATA->theParent  = 0;
        DATA->theContext = 0;
    }
    
    
    ContextIterator::ContextIterator( fh_context parent, const std::string& rdn )
        :
        p_data(0)
    {
        p_data = new ContextIteratorData();
        DATA->theParent = parent;
        DATA->theContext = 0;

        DATA->m_endIter = parent->getSortedItems().end();
        DATA->theContextIterator = DATA->m_endIter;
        if( rdn.length() )
        {
            DATA->theContext   = parent->getSubContext( rdn );
        }
        

        /*
         * This is a little icky, but the method for revalidation is
         * currently different depending on if we are iterating over
         * a sorted context
         */
        Context* raw = GetImpl(parent);
        while( ChainedViewContext* cvc = dynamic_cast<ChainedViewContext*>( raw ))
        {
            if( SortedContext* sc = dynamic_cast<SortedContext*>( raw ) )
            {
                DATA->isSortedContext = true;
                DATA->theContextIterator = sc->thisIterator;
                break;
            }
            raw = GetImpl( cvc->Delegate );
        }

//         cerr << "ContextIterator::ContextIterator() rdn:" << rdn
//              << " parent:"  << parent->getURL()
//              << " parent:"  << toVoid( GetImpl(parent) )
//              << " isbound:" << parent->priv_isSubContextBound( rdn )
//              << " isSorted:" << DATA->isSortedContext
//              << endl;

        /* quickly set theContextIterator so that we dont have to revalidate */
        if( ! DATA->isSortedContext )
        {
            if( rdn.length() )
            {
                DATA->theContextIterator = parent->getItemIter( rdn );
            }
        }
        
        
        /* Force a revalidation for next operation that might want it */
        DATA->Version   = parent->getDirOpVersion();
        setContext( DATA->theContext );

        /* Try to make end() iterators fast to create */
        if( !rdn.length() )
        {
            DATA->theContextIterator = DATA->theParent->getSortedItems().end();
            DATA->m_ForSortedOnly_OldRdnSet_reset();
        }
        else
        {
            DATA->update_OldRdnSet_cache();
            DATA->revalidate( true );
        }
    }

    ContextIterator::ContextIterator( const ContextIterator& ci )
    {
        p_data = new ContextIteratorData();

        ContextIteratorData* d = Data( &ci );
        DATA->theParent        = d->theParent;
        DATA->Version          = d->Version;
        DATA->isSortedContext  = d->isSortedContext;
        setContext( d->theContext );
        DATA->theContextIterator      = d->theContextIterator;
        DATA->m_endIter               = d->m_endIter;
        DATA->m_ForSortedOnly_OldRdnSet = d->m_ForSortedOnly_OldRdnSet;
    }
    
    ContextIterator&
    ContextIterator::operator=( const ContextIterator& ci )
    {
        ContextIteratorData* d = Data( &ci );
        DATA->theParent       = d->theParent;
        DATA->Version         = d->Version;
        DATA->isSortedContext = d->isSortedContext;
        setContext( d->theContext );
        DATA->theContextIterator      = d->theContextIterator;
        DATA->m_endIter               = d->m_endIter;
        DATA->m_ForSortedOnly_OldRdnSet = d->m_ForSortedOnly_OldRdnSet;
        return *this;
    }
    

    ContextIterator::~ContextIterator()
    {
        if( p_data )
            {
                delete DATA;
            }
    }
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    void
    ContextIterator::revalidate()
    {
        DATA->revalidate();
    }

    void
    ContextIterator::setContext( const fh_context& c )
    {
        DATA->setContext( c );
    }

    void
    ContextIterator::shiftIterator( int n )
    {
        DATA->shiftIterator( n );
    }
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    
    ContextIterator::reference ContextIterator::operator*() const
    {
        return DATA->theContext;
    }
    
    ContextIterator::pointer   ContextIterator::operator->()
    {
        return DATA->theContext;
    }
    
    ContextIterator::reference ContextIterator::operator[](difference_type n) const
    {
        ContextIterator tmp(*this);
        tmp.shiftIterator(n);
        ContextIteratorData* d = Data( &tmp );
        return d->theContext;
    }
    

    ContextIterator&
    ContextIterator::operator++()
    {
        shiftIterator(1);
        return *this;
    }
    
    ContextIterator
    ContextIterator::operator++(int)
    {
        ContextIterator tmp(*this);
        shiftIterator(1);
        return tmp;
    }
    
    ContextIterator&
    ContextIterator::operator--()
    {
        shiftIterator(-1);
        return *this;
    }
    
    ContextIterator
    ContextIterator::operator--(int)
    {
        ContextIterator tmp(*this);
        shiftIterator(-1);
        return tmp;
    }
    
    ContextIterator
    ContextIterator::operator+(difference_type n) const
    {
        ContextIterator tmp(*this);
        tmp.shiftIterator(n);
        return tmp;
    }
    
    ContextIterator&
    ContextIterator::operator+=(difference_type n)
    {
        shiftIterator(n);
        return *this;
    }

    ContextIterator
    ContextIterator::operator-(difference_type n) const
    {
        ContextIterator tmp(*this);
        tmp.shiftIterator( ForceNeg(n) );
        return tmp;
    }

//     ContextIterator::difference_type
//     ContextIterator::operator-(const _Self& ci) const
//     {
//         difference_type ret = 0;
//         ContextIteratorData* d = Data( &ci );

//         ret  = distance( DATA->theParent->getSortedItems().begin(), DATA->theContextIterator );
//         ret -= distance(    d->theParent->getSortedItems().begin(),    d->theContextIterator );

//         return ret;
//     }
    

    
    ContextIterator&
    ContextIterator::operator-=(difference_type n)
    {
//        cerr << "operator-= n:" << ForceNeg(n) << endl;
        shiftIterator( ForceNeg(n) );
        return *this;
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    ContextIterator::difference_type
    operator-(const ContextIterator& x, const ContextIterator& y)
    {
        ContextIteratorData* xd = Data( &x );
        ContextIteratorData* yd = Data( &y );

//         cerr << "operator-( ctxiter, ctxiter )" << endl;
//         if( xd->theContext )
//             cerr << "  xd.isTheContextIteratorSet():" << xd->isTheContextIteratorSet()
//                  << "  xd.getSortedItemsPointer():"   << xd->getSortedItemsPointer()
//                  << endl;
//         if( yd->theContext )
//             cerr << "  yd.isTheContextIteratorSet():" << yd->isTheContextIteratorSet()
//                  << "  yd.getSortedItemsPointer():"   << yd->getSortedItemsPointer()
//                  << endl;
//         if( xd->theParent )
//             cerr << "  xd.theParent:" << toVoid( xd->theParent ) << endl;
//         if( yd->theParent )
//             cerr << "  yd.theParent:" << toVoid( yd->theParent ) << endl;
//         if( xd->isTheContextIteratorSet() )
//             cerr << " *xd.citer:" << toVoid( *xd->theContextIterator )
//                  << " xd.citer.url:" << (*xd->theContextIterator)->getURL()
//                  << endl;
//         if( yd->isTheContextIteratorSet() )
//             cerr << " *yd.citer:" << toVoid( *yd->theContextIterator )
//                  << " yd.citer.url:" << (*yd->theContextIterator)->getURL()
//                  << endl;
        
        if( xd->theParent && xd->theParent != yd->theParent )
        {
//             cerr << " different parents, should handle this vlink case seperately" << endl;
//             xd->dump("xd");
//             yd->dump("yd");
//             if( xd->theParent )
//                 cerr << "  xd.theParent:" << toVoid( xd->theParent ) << endl;
//             if( yd->theParent )
//                 cerr << "  yd.theParent:" << toVoid( yd->theParent ) << endl;

            {
                static ContextSetCompare cssName("name");
                SortedContext* xsc = dynamic_cast<SortedContext*>( GetImpl( xd->theParent ));
                SortedContext* ysc = dynamic_cast<SortedContext*>( GetImpl( yd->theParent ));

                /* PURE DEBUG */
                if( xsc ) {
                    LG_CTX_D << " xd.is-sorted-context" << endl;
                }
                if( ysc ) {
                    LG_CTX_D << " yd.is-sorted-context" << endl;
                }

                bool usingSameSortingOrder = true;
                
                if( xsc && ysc )
                {
                    if( xsc->getSortedItems().key_comp() != ysc->getSortedItems().key_comp() )
                    {
                        LG_CTX_D << " xd and yd are sorted but using different ordering" << endl;
                        usingSameSortingOrder = false;
                    }
                }
                else if( xsc )
                {
                    if( xsc->getSortedItems().key_comp() != cssName )
                    {
                        LG_CTX_D << " xd is sorted yd isn't and are sorted but using different ordering" << endl;
                        usingSameSortingOrder = false;
                    }
                }
                else if( ysc )
                {
                    if( ysc->getSortedItems().key_comp() != cssName )
                    {
                        LG_CTX_D << " xd isn't sorted yd is and are sorted but using different ordering" << endl;
                        usingSameSortingOrder = false;
                    }
                }
                
                if( !usingSameSortingOrder )
                {
                    LG_CTX_D << "Using special handling for different parents that have a different"
                         << " sorting order." << endl;

                    Context::Items_t::iterator xiter  = xd->theContextIterator;
                    Context::Items_t::iterator yiter  = yd->theContextIterator;

                    // Get the xdist as an offset for xd->theContext in the ordering
                    // of the children of yd->theParent. This way both xdist and ydist
                    // are distances in the same ordering on the directory contents.
                    int xdist = 0;
                    for( Context::Items_t::iterator ci = yd->getSortedItems().begin() ; ; ++ci )
                    {
                        if( ci != yd->getSortedItems().end() )
                        {
                            // xd is not found in yd->theParent's children.
                            // something very bad has happened!

                            LG_CTX_ER << "operator-(Context::iterator,Context::iterator) "
                                      << " trying to operate on the children of a VirtualSortLink"
                                      << " where the two parents of the child (its natural parent"
                                      << " and the link context itself) are using different sorting"
                                      << " and the xd parameter:" << xd->theContext->getURL() << endl
                                      << " is not to be found in the yd directory:"
                                      << yd->theParent->getURL() << endl;
                            return 0;
                        }
                        
                        if( *ci == *xd->theContextIterator )
                            break;

                        LG_CTX_D << "operator-(++xdist) xd.ci:" << toVoid( GetImpl( *ci ) )
                             << " xd.ci:" << (*ci)->getURL()
                             << endl;
                        ++xdist;
                    }

                    int ydist = 0;
                    for( Context::Items_t::iterator ci = yd->getSortedItems().begin();
                         *ci != *yd->theContextIterator;
                         ++ci )
                    {
                        LG_CTX_D << "operator-(++ydist) yd.ci:" << toVoid( GetImpl( *ci ) )
                             << " yd.ci:" << (*ci)->getURL()
                             << endl;
                        ++ydist;
                    }
                    

                    LG_CTX_D << "   xdist:" << xdist << endl;
                    LG_CTX_D << "   ydist:" << ydist << endl;
                    LG_CTX_D << "     ret:" << (xdist - ydist) << endl;
                    return xdist - ydist;
                }
            }
            
            
            int xdist = 0;
            for( Context::Items_t::iterator ci = xd->getSortedItems().begin();
                 *ci != *xd->theContextIterator;
                 ++ci )
            {
//                 LG_CTX_D << "operator-(++xdist) xd.ci:" << toVoid( GetImpl( *ci ) )
//                      << " xd.ci:" << (*ci)->getURL()
//                      << endl;
                ++xdist;
            }
            
//            int xdist = distance( xd->getSortedItems().begin(), xd->theContextIterator );
//            LG_CTX_D << " different parents, should handle this vlink case seperately2" << endl;

            int ydist = 0;
            for( Context::Items_t::iterator ci = yd->getSortedItems().begin();
                 *ci != *yd->theContextIterator;
                 ++ci )
            {
//                 LG_CTX_D << "operator-(++ydist) yd.ci:" << toVoid( GetImpl( *ci ) )
//                      << " yd.ci:" << (*ci)->getURL()
//                      << endl;
                ++ydist;
            }
            
//            int ydist = distance( yd->getSortedItems().begin(), yd->theContextIterator );
//             LG_CTX_D << " different parents, should handle this vlink case seperately3" << endl;
//             LG_CTX_D << "   xdist:" << xdist << endl;
//             LG_CTX_D << "   ydist:" << ydist << endl;
//             LG_CTX_D << "     ret:" << (xdist - ydist) << endl;
            
            return xdist - ydist;
        }
        
//         if( xd->theContextIterator != xd->theParent->getSortedItems().end() )
//             LG_CTX_D << " xd context:" << (*xd->theContextIterator)->getURL() << endl;

//         if( yd->theContextIterator != yd->theParent->getSortedItems().end() )
//             LG_CTX_D << " yd context:" << (*yd->theContextIterator)->getURL() << endl;
        
        return distance( yd->theContextIterator, xd->theContextIterator );
    }
    
    bool operator==(const ContextIterator& x, const ContextIterator& y)
    {
        // These two say the same thing. The first version will expand
        // to the second when compiler inlining is allowed. The second
        // is here so that debug builds are not slowed down.
//         struct ContextIteratorData* xd = Data( &x );
//         struct ContextIteratorData* yd = Data( &y );
        struct ContextIteratorData* xd = (ContextIteratorData*)x.p_data;
        struct ContextIteratorData* yd = (ContextIteratorData*)y.p_data;


//         LG_CTX_D << "operator==() xd->Context:" << toVoid( xd->theContext )
//              << " yd->Context:" << toVoid( yd->theContext );
//         if( xd->theContext )
//             LG_CTX_D << " xd->theContext->getParent():" << toVoid( xd->theContext->getParent() );
//         if( yd->theContext )
//             LG_CTX_D << " yd->theContext->getParent():" << toVoid( yd->theContext->getParent() );
//         if( xd->theContext && yd->theContext )
//             LG_CTX_D << " xd.parent==yd.parent:"
//                  << (xd->theContext->getParent() == yd->theContext->getParent());
//         LG_CTX_D << " xd.ci == yd.ci:" 
//              << (xd->theContextIterator == yd->theContextIterator)
//              << endl;
//         if( xd->theContext )
//             LG_CTX_D << "  xd.isTheContextIteratorSet():" << xd->isTheContextIteratorSet()
//                  << "  xd.getSortedItemsPointer():"   << xd->getSortedItemsPointer()
//                  << endl;
//         if( yd->theContext )
//             LG_CTX_D << "  yd.isTheContextIteratorSet():" << yd->isTheContextIteratorSet()
//                  << "  yd.getSortedItemsPointer():"   << yd->getSortedItemsPointer()
//                  << endl;
//         if( xd->theParent )
//             LG_CTX_D << "  xd.theParent:" << toVoid( xd->theParent ) << endl;
//         if( yd->theParent )
//             LG_CTX_D << "  yd.theParent:" << toVoid( yd->theParent ) << endl;
//         if( xd->isTheContextIteratorSet() )
//             LG_CTX_D << " *xd.citer:" << toVoid( *xd->theContextIterator )
//                  << " xd.citer.url:" << (*xd->theContextIterator)->getURL()
//                  << endl;
//         if( yd->isTheContextIteratorSet() )
//             LG_CTX_D << " *yd.citer:" << toVoid( *yd->theContextIterator )
//                  << " yd.citer.url:" << (*yd->theContextIterator)->getURL()
//                  << endl;
//         xd->dump("xd");
//         yd->dump("yd");

        
//         if( !isBound( xd->theContext ) && !isBound( yd->theContext ) )
//             return true;
//         if( !isBound( xd->theContext ) || !isBound( yd->theContext ) )
//             return false;
        bool xdBound = isBound( xd->theContext );
        bool ydBound = isBound( yd->theContext );
        if( !xdBound && !ydBound )
            return true;
        if( !xdBound || !ydBound )
            return false;
        

        if( xd->theParent && xd->theParent != yd->theParent )
        {
//             LG_CTX_D << " different parents, should handle this vlink case seperately" << endl;
//             LG_CTX_D << "Different SortedItemsPointers. "
//                  << " *xd->theContextIterator == *yd->theContextIterator:"
//                  << (*xd->theContextIterator == *yd->theContextIterator)
//                  << endl;
            
            return xd->theContext->getParent() == yd->theContext->getParent()
                && *xd->theContextIterator == *yd->theContextIterator;
        }
        
        return xd->theContext->getParent() == yd->theContext->getParent()
            && xd->theContextIterator == yd->theContextIterator;
    }

    bool operator<(const ContextIterator& x, const ContextIterator& y)
    {
        struct ContextIteratorData* xd = Data( &x );
        struct ContextIteratorData* yd = Data( &y );

        int xoff = distance( xd->theContext->getParent()->getSortedItems().begin(),
                             xd->theContextIterator );
        int yoff = distance( yd->theContext->getParent()->getSortedItems().begin(),
                             yd->theContextIterator );
        
        return yoff < xoff;
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    bool operator>(const ContextIterator& x, const ContextIterator& y) {
        return y < x;
    }

    bool operator!=(const ContextIterator& x, const ContextIterator& y) {
        return !(x == y);
    }
    
    bool operator<=(const ContextIterator& x, const ContextIterator& y) {
        return !(y < x);
    }

    bool operator>=(const ContextIterator& x, const ContextIterator& y) {
        return !(x < y);
    }
    
    
};

