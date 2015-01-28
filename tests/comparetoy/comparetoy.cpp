/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris compare toy
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

    $Id: comparetoy.cpp,v 1.1 2006/12/07 06:58:59 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#if 0

#include <string>
#include <iostream>
#include <functional>

#include <FerrisHandle.hh>

#include <sigc++/signal_system.h>
#ifdef SIGC_CXX_NAMESPACES
using namespace SigC;
#endif

struct Attr
{
    string a;
    Attr( string aa ) : a(aa) 
        {
        }
    
    string getAttribute( string s )
        {
            return a;
        }
    
    
};



template <class _Tp = Attr*, class _StringCompare = less<string> >
struct ferris_sortop : public binary_function<_Tp,_Tp,bool> 
{
    string s;
    ferris_sortop( const string& _s ) : s(_s) {}
    bool operator()(const _Tp& __x, const _Tp& __y) const
        {
            return _StringCompare()(__x->getAttribute(s), __y->getAttribute(s));
        }
};

template < class T = Attr*, class binf = ferris_sortop<T> >
struct ferris_matcher : public binder2nd< binf >
{
    explicit ferris_matcher( const string& s, const T& x )
        :
        binder2nd< binf >( binf(s), x ) 
        {
        }
    typename binf::result_type test( const T& t )
        {
            return operator()(t);
        }
};


struct VirtualMatcherBase
{
    virtual bool operator()( Attr* a ) = 0;
};

template <class T>
struct VirtualMatcher : public VirtualMatcherBase
{
    T t;
    
    VirtualMatcher( const T& tt ) : t(tt) 
        {
        }
    
    virtual bool operator()( Attr* a )
        {
            return t.operator()(a);
        }
    
};




int main( int argc, const char** argv )
{
    Attr* fred = new Attr("fred");
    Attr* fots = new Attr("fots");
    string a = "arch";

    cerr << "fred:" << fred->getAttribute(a) << endl;
    cerr << "fots:" << fots->getAttribute(a) << endl;

    ferris_sortop<Attr*> fmp(a);
    
    cerr << "ferris_sortop( fred, fots ):" << fmp( fred, fots ) << endl;
    cerr << "ferris_sortop( fots, fred ):" << fmp( fots, fred ) << endl;



    
    less<string>* fmp2 = new less<string>();
    cerr << "fmp2_less( fred, fots ):"
         << fmp2->operator()( fred->getAttribute(a), fots->getAttribute(a) ) << endl;


    ferris_matcher<Attr*> fmp3(a, fots);
    cerr << "fcmp3( fred ):" << fmp3( fred ) << endl;

    ferris_matcher<Attr*> fmp4(a, fred);
    cerr << "fcmp4( fots ):" << fmp4( fots ) << endl;

    ferris_matcher<> fmp5(a, fred);
    cerr << "fcmp5( fots ):" << fmp5( fots ) << endl;

    ferris_matcher<>* fmp6 = new ferris_matcher<>(a, fred);
    cerr << "fcmp6( fots ):" << (*fmp6)( fots ) << endl;
    
    typedef FerrisHandle< ferris_matcher<> > fh_sorter;
    fh_sorter fmp7 = new ferris_matcher<>(a, fred);
    cerr << "fcmp7( fots ):" << fmp7->test( fots ) << endl;

    /*
     * Full release. We can store the unary matcher off for later execution.
     * Using the heap for allocation.
     */
    typedef FerrisHandle<VirtualMatcherBase> fh_ferris_uf;
    
    fh_ferris_uf fmp8 =  new VirtualMatcher< ferris_matcher<> >(ferris_matcher<>(a, fred));
    cerr << "fcmp8( fred ):" << fmp8.get_rep()->operator()( fred ) << endl;


    
    
    
    return 0;
}
#endif

int main( int argc, char** argv ) 
{return 0;}
