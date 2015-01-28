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

    $Id: singlecontext.cpp,v 1.1 2006/12/07 07:03:31 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "vmdebug.cpp"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
using namespace boost::multi_index;

    struct FERRISEXP_DLLLOCAL contextset_rdn
    {
        contextset_rdn() 
            {}

        inline bool operator()( const fh_context&  xc, const fh_context& yc ) const
            {
                return xc->getDirName() < yc->getDirName();
            }
    };

    template < class T >
    struct fx_equal_to : public std::binary_function< T, T, bool> 
    {
        inline bool operator()(const T __x, const T __y) const
            { return __x == __y; }
    };
    template< class T > 
    struct f_hash_raw : public std::unary_function< T, size_t >
    {
        inline size_t operator()( const T s ) const
            {
                return (size_t)( s );
            }
    };


    /*
     * This may collide somewhat on a 64bit machine.
     */
    template< class T > 
    struct fx_hash : public std::unary_function< T, size_t >
    {
        inline size_t operator()( const T s ) const
            {
                return (unsigned long)GetImpl(s);
//                return __gnu_cxx::hash<unsigned long>::operator()( (unsigned long)GetImpl(s) );
//                return GPOINTER_TO_INT( GetImpl(s) );
            }
    };

template < class T >
struct fx_dn_equal_to : public std::binary_function< T, T, bool> 
{
    inline bool operator()(const T __x, const T __y) const
        { return __x->getDirName() == __y->getDirName(); }
};
template< class T > 
struct fx_dn_hash : public __gnu_cxx::hash< const char* >
{
    inline size_t operator()( const T s ) const
        {
            return __gnu_cxx::hash< const char* >::operator()( s->getDirName().c_str() );
        }
};


template< class T > 
struct fx_identity : public std::unary_function< T, size_t >
{
    inline size_t operator()( const T& s ) const
        {
            return (size_t)GetImpl(s);
        }
};


int main( int argc, char** argv )
{
    ExistingContextsState* st_rootOnly;
    string path = "/";
    bool readPath = false;
    
    if( argc >= 2 )
        path = argv[1];
    if( argc >= 3 )
        readPath = true;
    
    /* Get the root context and save state, that is what should be available at the end */
    {
        printContexts("entered scope");
        fh_context rootc = Resolve( "/" );
        {
            // Overmounting a db4 file will create a throw away
            // never free'd context object.
            fh_context t = Resolve("apps://");
            t->read();
        }
        fullyReclaim("starting up");
        printContexts("have root");
        st_rootOnly = new ExistingContextsState();
    }

    {
        fh_context c = Resolve("apps://");
        cerr << "1raw apps:// ctx:" << (void*)GetImpl(c) << endl;
        cerr << "1raw before read REF_COUNT:" << c->ref_count
             << " c:" << GetImpl(c)
             << " omc:" << c->getOverMountContext()
             << endl;
        c->read();
        cerr << "1raw after read REF_COUNT:" << c->ref_count << endl;

        for( Context::iterator ci = c->begin(); ci != c->end(); ++ci )
        {
            cerr << " 1app:" << (*ci)->getDirName() << endl;
        }
        cerr << "about to drop apps:// handle" << endl;
    }
    fullyReclaim("apps");
    {
        fh_context c = Resolve("apps://");
        cerr << "2raw apps:// ctx:" << (void*)GetImpl(c) << endl;
        for( Context::iterator ci = c->begin(); ci != c->end(); ++ci )
        {
            cerr << " 2app:" << (*ci)->getDirName() << endl;
        }
    }
    fullyReclaim("apps2");
    
    
//     {
//         printContexts("entered scope");
//         fh_context c = Resolve( path );
//         cerr << "c:" << toVoid(GetImpl(c)) << endl;
//         if( readPath )
//             c->read( true );
        
//         printContexts("exiting scope");
//     }
    
//     printContexts("about to clean");
//     CacheManager* cc = getCacheManager();
//     cc->cleanUp( true );
//     printContexts("about to clean again 1");
//     cc->cleanUp( true );
//     printContexts("about to clean again 2");
//     cc->cleanUp( true );
//     printContexts("about to clean again 3");
//     cc->cleanUp( true );
//     printContexts("WAVE1 cleaned up first wave");


//     {
//         printContexts("entered scope2");
//         fh_context c = Resolve( path );
//         cerr << "c:" << toVoid(GetImpl(c)) << endl;
//         printContexts("exiting scope2");
//     }
//     printContexts("about to clean2");
//     cc->cleanUp( true );
//     printContexts("about to clean2 again 1");
//     cc->cleanUp( true );
//     printContexts("about to clean2 again 2");
//     cc->cleanUp( true );
//     printContexts("about to clean2 again 3");
//     cc->cleanUp( true );

//     if( argc >= 4 )
//     {
//         fullyReclaim("preparing for create");
//         Shell::acquireContext( argv[3] );
//         fullyReclaim("After createfile");
//     }
    
    
    printContexts("end");
    st_rootOnly->equalsCurrent();
    if( !errors )
        cout << "FULL Success" << endl;
    return errors;
}
