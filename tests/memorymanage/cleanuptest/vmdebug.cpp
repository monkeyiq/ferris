/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: vmdebug.cpp,v 1.1 2006/12/07 07:03:31 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <config.h>
#include <Ferris.hh>
#include <Ferris_private.hh>
#include <FilteredContext_private.hh>

#include <list>

using namespace std;
using namespace Ferris;

bool isFilter( Context* c )
{
    if( dynamic_cast<FilteredContext*>(c) )
        return true;
    return false;
}


bool isSorter( Context* c )
{
    if( dynamic_cast<SortedContext*>(c) )
        return true;
    return false;
}



void dumpcc( string s )
{
    CacheManager* cc = getCacheManager();

    fh_stringstream ss;
    cc->dumpFreeListTo( ss );
    cerr << "-------------------------------------------------------------------------------\n"
         << s << endl
         << StreamToString(ss) << endl;
}

void dumpall( string s )
{
    fh_stringstream ss;
    dumpEntireContextListMemoryManagementData( ss );
    cerr << "-------------------------------------------------------------------------------\n"
         << s << endl
         << StreamToString(ss) << endl;
}

void printentries( const fh_context& c )
{
    cerr << "-------------------------------------------------------------------------------\n"
         << "printentries c:" << c->getDirPath()
         << endl;
    
    typedef Context::SubContextNames_t cnt;
    cnt cn = c->getSubContextNames();
    int i=0;
    for( cnt::iterator iter = cn.begin(); iter != cn.end(); ++iter, ++i )
    {
        cerr << "name:" << c->getSubContext(*iter)->getDirName()
             << " path:" << c->getSubContext(*iter)->getDirPath()
             << endl;
    }
    cerr << "count:" << i << endl;
    cerr << "-------------------------------------------------------------------------------"
         << endl;
}


void printContexts( const std::string& s )
{
    cerr << "---printContexts(" << s << ");--- start" << endl;
//#ifdef FERRIS_DEBUG_VM
    for( debug_mm_contexts_t::iterator iter = getMMCtx().begin();
         iter != getMMCtx().end(); iter++ )
    {
        if( iter->first )
        {
            fh_stringstream ss;
            iter->first->dumpRefDebugData( ss );
            cerr
//                 << " isFC:" << isFilter(iter->first) << " "
//                 << " isSC:" << isSorter(iter->first) << " "
                << tostr(ss);
//             cerr << "c:"      << toVoid(iter->first)
//                  << " ID:" << iter->second
//                  << " rdn:"   << iter->first->getDirName()
//                  << " rc:" << iter->first->ref_count
//                  << " isfilterc:" << isFilter(iter->first)
//                  << endl;
        }
    }
//#endif
    cerr << "---printContexts(" << s << ");--- end" << endl;

    cerr << "---printContexts(" << s << ");--- freelist follows..." << endl;
    getCacheManager()->dumpFreeListTo( Factory::fcerr() );
}

static int errors = 0;

class ExistingContextsState
{
    typedef std::list< Context* > l_t;
    l_t l;

    
public:
    ExistingContextsState()
        {
//#ifdef FERRIS_DEBUG_VM
            for( debug_mm_contexts_t::iterator iter = getMMCtx().begin();
                 iter != getMMCtx().end(); iter++ )
            {
                if( iter->first )
                {
                    l.push_back( iter->first );
                }
            }
//#endif
        }

    /*
     * Check that all the items in this state obj are in the passed object too.
     */
    void allPresentIn( ExistingContextsState& c )
        {
            for( l_t::iterator iter = l.begin(); iter != l.end(); ++iter )
            {
                if( c.l.end() == find( c.l.begin(), c.l.end(), *iter ) )
                {
                    cerr << "ERROR context is not present when it should be. "
                         << " sought:" << toVoid( *iter ) << endl;
                    errors++;
                }
            }
        }

    void equalsCurrent()
        {
            ExistingContextsState cur;

            if( size() != cur.size() )
            {
                cerr << "ERROR, not the correct number of context objects in existance." << endl;
                cerr << "equalsCurrent() preserved state.size:" << size() << endl;
                cerr << "equalsCurrent()         current.size:" << cur.size() << endl;
                ++errors;
            }
            
            allPresentIn( cur );
            cur.allPresentIn( *this );
        }

    int size()
        {
            return l.size();
        }
    
};


void fullyReclaim( const std::string& msg )
{
    dumpall( string("before ") + msg );
    dumpcc( string("before ") + msg );

    int oldSize = 0;
    {
        ExistingContextsState st;
        oldSize = st.size();
    }
    
    CacheManager* cc = getCacheManager();
    for( int count = 0; true; ++count )
    {
        cc->cleanUp();
        ExistingContextsState st;
        cerr << "st.size:" << st.size()
             << " oldSize:" << oldSize << endl;
        if( st.size() >= oldSize )
        {
            break;
        }
        oldSize = st.size();

        string prefix = "during iteration:" + tostr(count) + " ";
        dumpall( prefix + msg );
        dumpcc( prefix + msg );
    }
    

    dumpall( string("after ") + msg );
    dumpcc( string("after ") + msg );
}
