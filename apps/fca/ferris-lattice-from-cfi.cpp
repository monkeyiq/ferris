/******************************************************************************
*******************************************************************************
*******************************************************************************

    LatticeFromCFI
    Copyright (C) 2005 Ben Martin

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

    $Id: ferris-lattice-from-cfi.cpp,v 1.33 2010/09/24 21:31:11 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
//#define HAVE_LIBGISTMIQ


//#define USE_BVMINI 1
#define USE_BITSET

//#define BM_DISBALE_BIT_IN_PTR
#define BM64OPT



#include <popt.h>

#include <Ferris/Ferris.hh>
#include <Ferris/FCA.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/FilteredContext.hh>
using namespace Ferris;
using namespace Ferris::FCA;
using Ferris::Time::Benchmark;
using namespace FerrisBitMagic;

#include <pqxx/connection>
#include <pqxx/tablewriter>
#include <pqxx/transaction>
#include <pqxx/nontransaction>
#include <pqxx/tablereader>
#include <pqxx/tablewriter>
#include <pqxx/result>
#include <fstream>
using namespace PGSTD;
using namespace pqxx;

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
using namespace std;

#ifdef HAVE_LIBGISTMIQ
#include "EasyRDTreeClient.hh"
#include "gist.h"
#include "gist_support.h"
#endif

//#include <Judy.h>

// stringstream junk;
// #undef LG_FCA_D
// #define LG_FCA_D if( 0 )  junk 
// #undef LG_FCA_ACTIVE
// #define LG_FCA_ACTIVE 0


const string PROGRAM_NAME = "ferris-lattice-from-cfi";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

int hadErrors = 0;

enum {
    E_PGCONNECT = -6,
    E_PGNOINPUT = -7,
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static void error (int code, ...)
{
    switch( -code )
    {
    case E_PGCONNECT: cerr << "error connecting to database" << endl; break;
    }
    exit( code );
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

connection* db_connection = 0;
bool parentsColumnExists = false;
bool childrenColumnExists = false;
bool addedAttrColumnExists = false;

int maxID=0;                    // highest ID in use at initial load time
string ItemSetNamesSQL="";      // SQL column names in order of ItemSet
int    ItemSetNamesSQLCount=0;  // Number of columns in ItemSetNamesSQL
fh_conceptLattice cl = 0;       // concept lattice 
fh_concept LatticeTop    = 0;   // top element in lattice
fh_concept LatticeBottom = 0;   // bottom element in lattice
typedef std::list< fh_concept > nodes_t;
nodes_t nodes;  // all nodes except LatticeBottom

string flatticeTreePath = "";

bool LatticeBottomIsAlreadyInDatabase = true;
bool LatticeTopIsAlreadyInDatabase = true;

static void setBit( guint8* Index, int value )
{
    guint8 rem    = value % 8;
    guint8 b      = 1 << rem;
    int    offset = (value-rem)/8;
//    cerr << "setBit() value:" << value << " b:" << (int)b << " offset:" << offset << " rem:" << (int)rem << endl;
    Index[offset] |= b;
}
static void clearBit( guint8* Index, int value )
{
    guint8 rem    = value % 8;
    guint8 b      = 1 << rem;
    b = ~b;
    int    offset = (value-rem)/8;
    Index[offset] &= b;
}
static void dumpBits( guint8* Index, int Index_sz )
{
//     cerr << "dumpBits() ";
//     int e = Index_sz/8+1;
//     for( int i=0; i < e; ++i )
//     {
//         cerr << ((Index[i] & 1) > 0)
//              << ((Index[i] & 2) > 0)
//              << ((Index[i] & 4) > 0)
//              << ((Index[i] & 8) > 0)            
//              << ((Index[i] & 16) > 0)
//              << ((Index[i] & 32) > 0)
//              << ((Index[i] & 64) > 0)
//              << ((Index[i] & 128) > 0)
//              << ' ';
//     }
//     cerr << endl;
}
void dumpBits( const FerrisBitMagic::bvector<>& a )
{
    if( LG_FCA_ACTIVE )
    {
        FerrisBitMagic::bvector<>::enumerator an     = a.first();
        FerrisBitMagic::bvector<>::enumerator an_end = a.end();

        stringstream ss;
        while (an < an_end)
        {
            ss << *an << ' ';
            ++an;  // Fastest way to increment enumerator
        }
        
        LG_FCA_D << "dumpBits(bv) count:" << a.count() << " " << ss.str() << endl;
    }
}
void dumpBits( const FerrisBitMagic::bvector<>& a, fh_ostream oss )
{
    FerrisBitMagic::bvector<>::enumerator an     = a.first();
    FerrisBitMagic::bvector<>::enumerator an_end = a.end();

    stringstream ss;
    while (an < an_end)
    {
        ss << *an << ' ';
        ++an;  // Fastest way to increment enumerator
    }
        
    oss << "dumpBits(bv) count:" << a.count() << " " << ss.str() << endl;
}
void dumpBits( guint64 v, fh_ostream oss )
{
    int count = 0;
    stringstream ss;
    for( long i = 0; i < 64; ++i )
    {
        if( (v >> i) & 0x1 == 0x1 )
        {
            ++count;
            ss << " " << i;
        }
    }
    oss << "dumpBits(u64) count:" << count << ss.str() << endl;
}

void dump( fh_ostream oss, clist_t& cl, const std::string& desc )
{
    oss << "======= start ======:" << desc << endl;
    for( rs<clist_t> ri(cl); ri; ++ri )
    {
        oss << "id:" << (*ri)->getID() << " bv:";
        dumpBits( (*ri)->getItemSet(), oss );
//        oss << endl;
    }
    oss << "======= end ======:" << desc << endl;
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#define BM_DISBALE_BIT_IN_PTR

#ifdef USE_BITSET

struct IntentsData : public Handlable
{
    fh_concept node;
    typedef fca_std_bitset_t intent_t;
    intent_t intent;
    
    IntentsData( fh_concept node, const intent_t& _intent )
        :
        node( node ),
        intent( _intent )
        {
        }
};

#else

#ifndef USE_BVMINI


struct IntentsData : public Handlable
{
    fh_concept node;
    typedef FerrisBitMagic::bvector<> intent_t;
    intent_t intent;
    
    IntentsData( fh_concept node, const FerrisBitMagic::bvector<>& _intent )
        :
        node( node ), intent( _intent )
               //        node( node )
//        node( node ), intent( FerrisBitMagic::BM_BIT, FerrisBitMagic::gap_len_table<true>::_len, 64 )
        {
//            intent |= _intent;
//             intent.set_new_blocks_strat( FerrisBitMagic::BM_GAP );
//             FerrisBitMagic::bvector<>::enumerator an     = _intent.first();
//             FerrisBitMagic::bvector<>::enumerator an_end = _intent.end();

//             while (an < an_end)
//              {
//                  intent[*an]=1;
//                  ++an;  // Fastest way to increment enumerator
//              }
        }
};

#else

#warning "Using bvmini!"
struct IntentsData : public Handlable
{
    fh_concept node;
    typedef FerrisBitMagic::bvector<FerrisBitMagic::standard_allocator,
                                    FerrisBitMagic::bvmini<64>
    > intent_t;
    intent_t intent;
    

    IntentsData( fh_concept node, const FerrisBitMagic::bvector<>& _intent )
        :
        node( node )
        {
            to_bvmini( _intent, intent );
        }
    
};
#endif
#endif

FERRIS_SMARTPTR( IntentsData, fh_IntentsData );
typedef multimap< int, fh_IntentsData > Intents_t;
//typedef multimap< int, IntentsData* > Intents_t;




Intents_t& Maxima( Intents_t& input, Intents_t& ret )
{
    for( Intents_t::reverse_iterator ii = input.rbegin(); ii!=input.rend(); ++ii )
    {
        const IntentsData::intent_t& iIntent = ii->second->intent;
        bool ismin = true;
        for( Intents_t::iterator ri = ret.begin(); ri!=ret.end(); ++ri )
        {
            ismin &= (!second_is_superset(iIntent, ri->second->intent));
            if( !ismin )
                break;
        }

        if( ismin )
        {
            ret.insert( *ii );
        }
    }

    return ret;
}

//fh_concept FindConcept( fh_concept startc, const FerrisBitMagic::bvector<>& intent )
template < class Arg2 >
fh_concept FindConcept( fh_concept startc, const Arg2& intent )
{
#ifdef USE_BITSET

    fh_concept c = startc;
    LG_FCA_D << "FindConcept(bitset) startc:" << c->getID() << endl;
    while( !equiv( c->getItemSetBitSet(), intent ) )
    {
        bool c_changed = false;
        LG_FCA_D << "FindConcept() c:" << c->getID() << endl;
        const clist_t& pl = c->getParents();
        for( clist_t::const_iterator pi = pl.begin(); pi!=pl.end(); ++pi )
        {
            LG_FCA_D << "FindConcept() c:" << c->getID()
                     << " p:" << (*pi)->getID()
                     << endl;
            LG_FCA_D << "FindConcept() c.intent:" << intent
                     << " p.intent:" << (*pi)->getItemSetBitSet()
                     << endl;
            if( !second_is_superset( intent, (*pi)->getItemSetBitSet() ) )
                continue;
            c = *pi;
            c_changed = true;
            break;
        }
        if( !c_changed )
        {
            LG_FCA_D << "FindConcept(ERROR) !c_changed" << endl;
            return 0;
        }
    }
    return c;

#else
    fh_concept c = startc;
    LG_FCA_D << "FindConcept(not bitset) startc:" << c->getID() << endl;
    while( !equiv( c->getItemSet(), intent ) )
    {
        const clist_t& pl = c->getParents();
        for( clist_t::const_iterator pi = pl.begin(); pi!=pl.end(); ++pi )
        {
            if( !second_is_superset( intent, (*pi)->getItemSet() ) )
                continue;
            c = *pi;
            break;
        }
    }
    return c;
#endif
    
    
// //    cerr << "FIXME: FindConcept() needs optimization" << endl;
    
//     nodes_t::iterator begin = nodes.begin();
//     nodes_t::iterator   end = nodes.end();
//     for( nodes_t::iterator ni = begin; ni != end; ++ni )
//     {
//         fh_concept n = *ni;
//         if( n->getItemSet() == intent )
//             return n;
//     }
//     if( LatticeTop->getItemSet() == intent )
//         return LatticeTop;
    
//     return 0;
}


void dumpNodes( fh_ostream oss )
{
    oss << "setupParentAndChildrenLinks(dump begin)" << endl;
    nodes_t::iterator begin = nodes.begin();
    nodes_t::iterator   end = nodes.end();
    for( nodes_t::iterator ni = begin; ni != end; ++ni )
    {
        fh_concept n = *ni;
        oss << "n->id:" << n->getID() << " ";
        dumpBits( n->getItemSet(), oss );            
        oss << endl;
    }
    oss << "setupParentAndChildrenLinks(dump end)" << endl;
}

string
tostr( const FerrisBitMagic::bvector<>& bv )
{
    stringstream ss;
//    ss << "sz:" << bv.count() << " " << bv;

    ss << "sz:" << bv.count() << " bv:";

    if( !bv.count() )
        return ss.str();
    
    unsigned value = bv.get_first();
    do
    {
        ss << value;
        value = bv.get_next(value);
        if (value)
        {
            ss << ",";
        }
        else
        {
            break;
        }
    } while(1);
//    ss << std::endl;
    return ss.str();
}

string
tostr( const extent_bitset_t& bv )
{
    return "tostr(bitset) FIXME";
}

// string
// tostr( const fca_std_bitset_t& bv )
// {
//     return "tostr(bitset) FIXME";
// }

    
unsigned long CLARIFY = 0;

// pp38 Concept Data Analysis CoveringEdges()
void setupParentAndChildrenLinks_CoveringEdges( bool useGIST = false, bool populateGIST = true )
{
    Time::Benchmark setupParentAndChildrenLinks_benchmark("setupParentAndChildrenLinks()");

    int TotalEdges = 0;
    LatticeTop = 0;

    // Create a new lattice top, we shall remove it later if there is already
    // a top node in the lattice.
    int NextID = nodes.size();
    LatticeTop = new Concept( cl );
    LatticeTop->setID( NextID );
    cl->addConcept( LatticeTop );
    LG_FCA_D << "CLARIFY:" << CLARIFY << endl;
    LG_FCA_D << "Lattice top ID:" << LatticeTop->getID() << endl;
//    fh_context G = cl->getAllObjects();
//    LG_FCA_D << "G.sz:" << G->getSubContextCount() << " G.url:" << G->getURL() << endl;

    Intents_t Intents;
    typedef std::list< fh_concept > OuterList_t;
    OuterList_t OuterList;

    copy( nodes.begin(), nodes.end(), back_inserter( OuterList ) );
    OuterList.push_front( LatticeTop );

    FerrisBitMagic::bvector<> M = cl->getFullItemSet();
    stringlist_t allAttributeNames;
    cl->getAllAttributeNames( allAttributeNames );

    // Precalculate m' for every individual item in M
    LG_FCA_D << "setting up m' cache..." << endl;
    FerrisBitMagic::bvector<> G;
    VerticalFormalContext_t vfc;
    {
        Benchmark bm( "BENCH: setting up m' cache...");
        cl->getVerticalFormalContext( G, vfc, CLARIFY );
    }
    
    // Precalculate X for all {nodes} such that X AND m' is quick.
    LG_FCA_D << "setting up X for all nodes..." << endl;
    typedef map< fh_concept, FerrisBitMagic::bvector<> > XCache_t;
    typedef map< FerrisBitMagic::bvector<>, fh_concept > RevXCache_t;
    XCache_t XCache;
#ifdef HAVE_LIBGISTMIQ
    typedef bvector<> key_type;
    typedef EasyRDTreeClientBitMagic tree_type;
    tree_type RevXCacheGIST("tmp-fca-gist");
    tree_type::iterator RevXCacheGIST_end = RevXCacheGIST.end();
#endif    
    fh_concept NullXConcept = 0;
    RevXCache_t RevXCache;
    {
        cerr << "nodes.sz:" << nodes.size() << endl;
        
        Benchmark bm( "BENCH: setting up X for all nodes..."); 
        nodes_t::iterator nend  = nodes.end();
        nodes_t::iterator niter = nodes.begin();
        for( ; niter != nend; ++niter )
        {
            fh_concept c = *niter;

            FerrisBitMagic::bvector<> X;
            c->getExtentVerticalVector( X, CLARIFY );
            long Xcount = X.count();
            
            if( !Xcount )
            {
                NullXConcept = c;
                LG_FCA_D << "ERROR! Processing concept:" << c->getID() << endl;
                LG_FCA_D << "Concept has no getExtentVerticalVector" << endl;
            }
            LG_FCA_D << "Setting up XCache for concept:" << c->getID()
                     << " X.sz:" << X.count()
                     << " X:" << tostr(X)
                     << endl;
            
            XCache[ c ] = X;

#ifdef HAVE_LIBGISTMIQ
            if( useGIST )
            {
                if( populateGIST )
                {
                    if( Xcount )
                    {
                        cerr << "GIST Adding X:" << tostr(X) << endl;
                        RevXCacheGIST.insert( X, c->getID() );
                    }
                }
            }
#endif
            cerr << "RevXCache[X] Adding X:" << tostr(X) << endl;
            RevXCache[ X ] = c;
        }
    }
    
    
//     LG_FCA_D << "setting up atom..." << endl;
//     stringmap_t atof;
//     typedef map< long, fh_matcher > atom_t;
//     atom_t atom;
//     {
//         cl->getAttributeToFFilterMap( atof );
//         for( stringmap_t::iterator ai = atof.begin(); ai != atof.end(); ++ai )
//         {
//             string a    = ai->first;
//             string fstr = ai->second;
//             fh_context z = Factory::MakeFilter( fstr );
//             fh_matcher m = Factory::MakeMatcherFromContext( z );

//             const FerrisBitMagic::bvector<>& t = cl->StringToAttrID( a );
//             long attrid = (*(t.first()));
//             atom[ attrid ] = m;
//         }
//     }
    
//      // Precalculate m' for every individual item in M
//      LG_FCA_D << "setting up m' cache..." << endl;
//      typedef map< long, FerrisBitMagic::bvector<> > emDashCache_t;
//      emDashCache_t emDashCache;
//     {
//         Context::iterator Gend = G->end();

// //         for( stringlist_t miter = allAttributeNames.begin(); miter != allAttributeNames.end(); ++miter )
// //         {
// //             string m = *miter;
        
//         FerrisBitMagic::bvector<>::enumerator an     = M.first();
//         FerrisBitMagic::bvector<>::enumerator an_end = M.end();
//         for ( ; an < an_end; ++an )
//         {
//             long m = *an;
//             LG_FCA_D << "setting up m' cache for m:" << m << endl;
// //            string m_str = AttrIDToString( m );

//             // FIXME: THIS NEXT LOOP IS SLOW!!!
//             FerrisBitMagic::bvector<> bv;
//             int i = 0;
//             for( Context::iterator Giter = G->begin(); Giter != Gend; ++Giter, ++i )
//             {
//                 fh_context g = *Giter;

//                 LG_FCA_D << "testing g:" << g->getURL() << endl;
//                 if( atom[ m ]( g ) )
//                 {
//                     bv[i] = 1;
//                 }
//             }
//             emDashCache[ m ] = bv;
//         }
//     }

//     // Precalculate X for all {nodes} such that X AND m' is quick.
//     LG_FCA_D << "setting up X for all nodes..." << endl;
//     typedef map< fh_concept, FerrisBitMagic::bvector<> > XCache_t;
//     typedef map< FerrisBitMagic::bvector<>, fh_concept > RevXCache_t;
//     XCache_t XCache;
//     RevXCache_t RevXCache;
//     {
//         nodes_t::iterator nend  = nodes.end();
//         nodes_t::iterator niter = nodes.begin();
//         for( ; niter != nend; ++niter )
//         {
//             fh_concept n = *niter;
//             fh_context nextent = n->getExtent();
//             typedef set< string > nurls_t;
//             nurls_t nurls;
//             {
//                 Context::iterator ce = nextent->end();
//                 for( Context::iterator ci = nextent->begin(); ci!=ce; ++ci )
//                 {
//                     nurls.insert( (*ci)->getURL() );
//                 }
//             }
            
//             FerrisBitMagic::bvector<> bv;
            
//             Context::iterator Gend = G->end();
//             Context::iterator Giter = G->begin();
//             for( int i=0; Giter != Gend; ++Giter, ++i )
//             {
//                 fh_context g = *Giter;
//                 if( nurls.count( g->getURL() ) )
//                 {
//                     bv[i] = 1;
//                 }
//             }
//             XCache[ n ] = bv;
//             RevXCache[ bv ] = n;
//         }
//     }

    LG_FCA_D << "G.sz:" << G.count() << endl;
    LG_FCA_D << "nodes.sz:" << nodes.size() << endl;
    LG_FCA_D << "XCache.sz:" << XCache.size() << endl;
    LG_FCA_D << "RevXCache.sz:" << RevXCache.size() << endl;
    
    //
    //
    //
    cerr << "BENCH: Starting main work" << endl;
    {
        Benchmark bm( "BENCH: covering edges Finding parent/child links...");
        OuterList_t::iterator OuterListBegin = OuterList.begin();
        OuterList_t::iterator OuterListEnd   = OuterList.end();
        for( OuterList_t::iterator OuterListIter = OuterListBegin;
             OuterListIter != OuterListEnd; ++OuterListIter )
        {
            fh_concept n = *OuterListIter;
            const FerrisBitMagic::bvector<>& nIntent = n->getItemSet();
            const FerrisBitMagic::bvector<>& X = XCache[ n ];
            if( LG_FCA_ACTIVE )
            {
                LG_FCA_D << "---------------------------------------------------" << endl << endl;
                LG_FCA_D << "Processing concept:" << n->getID()
                         << " nIntent:" << tostr(nIntent)
                         << " X.sz:" << X.count()
                         << endl;
            }
            //          const FerrisBitMagic::bvector<> Y;
//          n->getExtentVerticalVector( Y );

            // set count of any concept in C to 0
            typedef map< fh_concept, long > Counters_t;
            Counters_t Counters;

            // Subtraction = M\Y
            FerrisBitMagic::bvector<> Subtraction = M;
            Subtraction -= nIntent;

            LG_FCA_D << "Subtraction:" << tostr(Subtraction)
                     << endl;
            LG_FCA_D << "-------------" << endl;

            // For m in (M\Y)
            FerrisBitMagic::bvector<>::enumerator an     = Subtraction.first();
            FerrisBitMagic::bvector<>::enumerator an_end = Subtraction.end();
            for ( ; an < an_end; ++an )
            {
                long m = *an;

//              inters = X AND m';
                FerrisBitMagic::bvector<> inters = X;
                inters &= vfc[ m ];
                LG_FCA_D << "m:" << m << " X.sz:" << X.count() << " vfc[ m ].size:" << vfc[ m ].count()
                         << " inters.size:" << inters.count() << endl;
                if( vfc[ m ].count() < 10 )
                    LG_FCA_D << "vfc[ m ]... " << tostr( vfc[ m ] ) << endl;
                
//              fh_concept tc = find( C with X==inters );
//              typedef map< FerrisBitMagic::bvector<>, fh_concept > RevXCache_t;

                LG_FCA_D << "A inters:" << tostr(inters) << endl;
#ifdef HAVE_LIBGISTMIQ
                LG_FCA_D << "B inters:" << libgist::tostr_binary(inters) << endl;
#endif
                
                fh_concept tc = 0;
                if( !useGIST )
                {
                    RevXCache_t::iterator tciter = RevXCache.find( inters );
                    if( tciter == RevXCache.end() )
                        continue;
                    tc = tciter->second;
                }
#ifdef HAVE_LIBGISTMIQ
                if( useGIST )
                {
                    if( !inters.count() )
                    {
                        LG_FCA_D << "GIST !inters.count" << endl;

                        if( NullXConcept )
                        {
                            tc = NullXConcept;
                        }
                        else
                        {
                            RevXCache_t::iterator tciter = RevXCache.find( inters );
                            if( tciter == RevXCache.end() )
                                continue;
                            tc = tciter->second;
                        }
                    }
                    else
                    {
                        tree_type::iterator iter = RevXCacheGIST.find( inters );
                        if( iter == RevXCacheGIST_end )
                        {
                            LG_FCA_D << "GIST !iter!" << endl;
                            continue;
                        }
                        
                        long cid = iter->second;
                        LG_FCA_D << "GIST concept-id:" << cid << endl;
                        tc = cl->getConcept( cid );
                    }
                }
#endif
                
                if( !tc )
                {
                    LG_FCA_D << "WARNING !tc" << endl;
                    continue;
                }
                LG_FCA_D << "tc:" << tc->getID() << endl;
                
                const FerrisBitMagic::bvector<>& tcIntent = tc->getItemSet();
                long newCount = (++Counters[ tc ]);
                LG_FCA_D << "incrementing count(x1,y1) concept:" << tc->getID()
                         << " newCount:" << newCount
                         << " tcIntent.count():" << tcIntent.count()
                         << " nIntent.count():"  << nIntent.count()
                         << " tcIntent:" << tostr(tcIntent)
                         << endl;
                
                if( tcIntent.count() - nIntent.count() == newCount )
                {
//                    LG_FCA_D << "TESTING2!.................." << endl;
//                    if( n != tc )
                    {
                        n->makeLink( tc, false );
                        ++TotalEdges;
                    }
                }
            }
        }
    }
    cerr << "BENCH: Ended main work" << endl;
    
    
//     OuterList_t::iterator OuterListBegin = OuterList.begin();
//     OuterList_t::iterator OuterListEnd   = OuterList.end();
//     for( OuterList_t::iterator OuterListIter = OuterListBegin;
//          OuterListIter != OuterListEnd; ++OuterListIter )
//     {
//         fh_concept OuterListConcept = *OuterListIter;
//         FerrisBitMagic::bvector<> OuterListIntent = OuterListConcept->getItemSet();

//         nodes_t::iterator InnerListBegin = nodes.begin();
//         nodes_t::iterator InnerListEnd   = nodes.end();
//         for( nodes_t::iterator InnerListIter = InnerListBegin;
//              InnerListIter != InnerListEnd; ++InnerListIter )
//         {
//             fh_concept InnerListConcept = *InnerListIter;
//             FerrisBitMagic::bvector<> InnerListIntent = InnerListConcept->getItemSet();

//             if( second_is_superset( OuterListIntent, InnerListIntent ) )
//             {
//                 OuterListConcept->makeLink( InnerListConcept, false );
//             }
//         }
//     }




    
    // If the synthetic lattice top node was made in error remove it and promote
    // the correct child to become the new lattice top
    {
        fh_concept newRoot = 0;
        
        const clist_t& children = LatticeTop->getChildren();
        for( clist_t::const_iterator ci = children.begin(); ci!=children.end(); ++ci )
        {
            fh_concept child = *ci;
            
            if( child->getChildren().empty() )
            {
                newRoot = *ci;
                break;
            }
        }
        if( newRoot )
        {
            LG_FCA_D << "newRoot ID:" << newRoot->getID() << endl;
            LG_FCA_D << "oldRoot ID:" << LatticeTop->getID() << endl;
            for( clist_t::const_iterator ci = children.begin(); ci!=children.end(); ++ci )
            {
                if( (*ci)->getID() != newRoot->getID() )
                    newRoot->makeLink( *ci, false );
            }
            cl->removeConcept( LatticeTop );
        }
    }
    cerr << "TotalConcepts:" << nodes.size() << endl;
    cerr << "TotalEdges:" << TotalEdges << endl;
}


struct myhash
{
    size_t
    operator()(const extent_bitset_t& s) const
        {
            unsigned long __h = 0;
            for( size_t an = s.find_first(); an!=extent_bitset_t::npos; an = s.find_next( an ) )
                __h = 5 * __h + an;
            return size_t(__h);
        }
};


// pp38 Concept Data Analysis CoveringEdges()
void setupParentAndChildrenLinks_CoveringEdgesNativeBitSet()
{
    Time::Benchmark setupParentAndChildrenLinks_benchmark("setupParentAndChildrenLinks()");

    int TotalEdges = 0;
    LatticeTop = 0;

    // Create a new lattice top, we shall remove it later if there is already
    // a top node in the lattice.
    int NextID = nodes.size();
    LatticeTop = new Concept( cl );
    LatticeTop->setID( NextID );
    cl->addConcept( LatticeTop );
    LG_FCA_D << "CLARIFY:" << CLARIFY << endl;
    LG_FCA_D << "Lattice top ID:" << LatticeTop->getID() << endl;

    Intents_t Intents;
    typedef std::list< fh_concept > OuterList_t;
    OuterList_t OuterList;

    copy( nodes.begin(), nodes.end(), back_inserter( OuterList ) );
    OuterList.push_front( LatticeTop );

    
    fca_std_bitset_t M = cl->getFullItemSetBitSet();
    stringlist_t allAttributeNames;
    cl->getAllAttributeNames( allAttributeNames );

    // Precalculate m' for every individual item in M
    LG_FCA_D << "setting up m' cache..." << endl;
    extent_bitset_t G;
    VerticalFormalContextBitSet_t vfc;
    {
        Benchmark bm( "BENCH: setting up m' cache...");
        cl->getVerticalFormalContextBitSet( G, vfc, CLARIFY );
    }
    
    // Precalculate X for all {nodes} such that X AND m' is quick.
    LG_FCA_D << "setting up X for all nodes..." << endl;
    typedef map< fh_concept, extent_bitset_t > XCache_t;
//    typedef map< extent_bitset_t, fh_concept > RevXCache_t;
    typedef FERRIS_STD_HASH_MAP< extent_bitset_t, fh_concept, myhash > RevXCache_t;
    XCache_t XCache;
    RevXCache_t RevXCache;
    {
        Benchmark bm( "BENCH: setting up X for all nodes...");
        nodes_t::iterator nend  = nodes.end();
        nodes_t::iterator niter = nodes.begin();
        for( ; niter != nend; ++niter )
        {
            fh_concept c = *niter;

            extent_bitset_t X;
            c->getExtentVerticalVectorBitSet( X, CLARIFY );

            if( X.count() == 0 )
            {
                LG_FCA_D << "ERROR! Processing concept:" << c->getID() << endl;
            }
            LG_FCA_D << "Setting up XCache for concept:" << c->getID()
                     << " X.sz:" << X.count()
                     << " X:" << tostr(X)
                     << endl;
            
            XCache[ c ] = X;
            RevXCache[ X ] = c;
        }
    }
    

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

    LG_FCA_D << "G.sz:" << G.count() << endl;
    LG_FCA_D << "nodes.sz:" << nodes.size() << endl;
    LG_FCA_D << "XCache.sz:" << XCache.size() << endl;
    LG_FCA_D << "RevXCache.sz:" << RevXCache.size() << endl;
    
    //
    //
    //
    cerr << "BENCH: Starting main work" << endl;
    {
        Benchmark bm( "BENCH: covering edges Finding parent/child links...");
        OuterList_t::iterator OuterListBegin = OuterList.begin();
        OuterList_t::iterator OuterListEnd   = OuterList.end();
        for( OuterList_t::iterator OuterListIter = OuterListBegin;
             OuterListIter != OuterListEnd; ++OuterListIter )
        {
            fh_concept n = *OuterListIter;
            const fca_std_bitset_t& nIntent = n->getItemSetBitSet();
            const extent_bitset_t& X = XCache[ n ];
            if( LG_FCA_ACTIVE )
            {
                LG_FCA_D << "---------------------------------------------------" << endl << endl;
                LG_FCA_D << "Processing concept:" << n->getID()
                         << " nIntent:" << tostr(nIntent)
                         << " X.sz:" << X.count()
                         << endl;
            }

            // set count of any concept in C to 0
            typedef map< fh_concept, long > Counters_t;
            Counters_t Counters;

            // Subtraction = M\Y
//            fca_std_bitset_t Subtraction = M;
//            Subtraction -= nIntent;
            fca_std_bitset_t Subtraction = M ^ (M & nIntent);

            LG_FCA_D << "Subtraction:" << tostr(Subtraction)
                     << endl;
            LG_FCA_D << "-------------" << endl;

            // For m in (M\Y)

            for( size_t an = Subtraction.find_first();
                 an!=fca_std_bitset_t::npos;
                 an = Subtraction.find_next( an ) )
            {
                long m = an;

//              inters = X AND m';
                extent_bitset_t inters = X;
                inters &= vfc[ m ];
                LG_FCA_D << "m:" << m << " X.sz:" << X.count() << " vfc[ m ].size:" << vfc[ m ].count()
                         << " inters.size:" << inters.count() << endl;
                if( vfc[ m ].count() < 10 )
                    LG_FCA_D << "vfc[ m ]... " << tostr( vfc[ m ] ) << endl;
                
//              fh_concept tc = find( C with X==inters );
//              typedef map< FerrisBitMagic::bvector<>, fh_concept > RevXCache_t;
                RevXCache_t::iterator tciter = RevXCache.find( inters );
                if( tciter == RevXCache.end() )
                    continue;

                fh_concept tc = tciter->second;
                const fca_std_bitset_t& tcIntent = tc->getItemSetBitSet();
                long newCount = (++Counters[ tc ]);
                LG_FCA_D << "incrementing count(x1,y1) concept:" << tc->getID()
                         << " newCount:" << newCount
                         << " tcIntent.count():" << tcIntent.count()
                         << " nIntent.count():"  << nIntent.count()
                         << " tcIntent:" << tostr(tcIntent)
                         << endl;
                
                if( tcIntent.count() - nIntent.count() == newCount )
                {
//                    LG_FCA_D << "TESTING2!.................." << endl;
//                    if( n != tc )
                    {
                        n->makeLink( tc, false );
                        ++TotalEdges;
                    }
                }
            }
        }
    }
    cerr << "BENCH: Ended main work" << endl;
    
    
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////


    
    // If the synthetic lattice top node was made in error remove it and promote
    // the correct child to become the new lattice top
    {
        fh_concept newRoot = 0;
        
        const clist_t& children = LatticeTop->getChildren();
        for( clist_t::const_iterator ci = children.begin(); ci!=children.end(); ++ci )
        {
            fh_concept child = *ci;
            
            if( child->getChildren().empty() )
            {
                newRoot = *ci;
                break;
            }
        }
        if( newRoot )
        {
            LG_FCA_D << "newRoot ID:" << newRoot->getID() << endl;
            LG_FCA_D << "oldRoot ID:" << LatticeTop->getID() << endl;
            for( clist_t::const_iterator ci = children.begin(); ci!=children.end(); ++ci )
            {
                if( (*ci)->getID() != newRoot->getID() )
                    newRoot->makeLink( *ci, false );
            }
            cl->removeConcept( LatticeTop );
        }
    }
    cerr << "TotalConcepts:" << nodes.size() << endl;
    cerr << "TotalEdges:" << TotalEdges << endl;
}



void first_and_eq_second( FerrisBitMagic::bvector<>& a,
                          const FerrisBitMagic::bvector<>& b )
{
    a &= b;
}



#ifdef USE_BVMINI
void first_and_eq_second(
    FerrisBitMagic::bvector<
    FerrisBitMagic::standard_allocator,
    FerrisBitMagic::bvmini<64> >& a,
    const FerrisBitMagic::bvector<>& blarge )
{
    FerrisBitMagic::bvector<
    FerrisBitMagic::standard_allocator,
        FerrisBitMagic::bvmini<64> > b64;
    to_bvmini( blarge, b64 );
    a &= b64;
}
#endif


// An extension method
void first_and_eq_second( FerrisBitMagic::bvector<
                          FerrisBitMagic::standard_allocator, 
                          FerrisBitMagic::miniset<
                          FerrisBitMagic::block_allocator, FerrisBitMagic::set_total_blocks> >& a,
                          const FerrisBitMagic::bvector<>& b )
{
    typedef FerrisBitMagic::bvector<>::enumerator enumerator_t;
//     typedef FerrisBitMagic::bvector<
//         FerrisBitMagic::standard_allocator, 
//         FerrisBitMagic::miniset<
//     FerrisBitMagic::block_allocator, FerrisBitMagic::set_total_blocks> >::enumerator enumerator_t;
    enumerator_t bn     = b.first();
    enumerator_t bn_end = b.end();

    while (bn < bn_end)
    {
        a[*bn] = 1;
        ++bn;
    }
}


string getTEXLabel( const FerrisBitMagic::bvector<>& bv )
{
    stringlist_t sl;
    cl->AttrIDToStringList( bv, sl );
    stringstream ss;
    ss << "\\{";
    for( stringlist_t::iterator si = sl.begin(); si != sl.end(); ++si )
        ss << *si;
    //ss << Util::createSeperatedList( sl, '' );
    ss << "\\}";
    return ss.str();
}



FerrisBitMagic::bvector<> to_bvector( const fca_std_bitset_t& bv )
{
    int bv_sz = bv.size();
    FerrisBitMagic::bvector<> ret( FerrisBitMagic::BM_BIT,
                                   FerrisBitMagic::gap_len_table_min<true>::_len,
                                   bv_sz );
    for ( fca_std_bitset_t::size_type i = 0; i < bv_sz; ++i)
        ret[i] = bv[i];
    return ret;
}

string getTEXLabel( const fca_std_bitset_t& bv )
{
    return getTEXLabel( to_bvector( bv ) );
}


void setupParentAndChildrenLinks( bool useGIST = false, bool populateGIST = true )
{
    Time::Benchmark setupParentAndChildrenLinks_benchmark("setupParentAndChildrenLinks()");
    
//    cerr << "setupParentAndChildrenLinks(top)" << endl;
    
    int TotalEdges = 0;
    LatticeTop = 0;
//     for( nodes_t::iterator ni = nodes.begin(); ni != nodes.end(); ++ni )
//     {
//         fh_concept n = *ni;
//         if( isZero( n->getItemSet() ) )
//         {
//             LatticeTop = n;
//             nodes.erase( ni );
//             break;
//         }
//     }

    int nodes_sz = nodes.size();
    
    if( LatticeTop )
    {
    }
    else
    {
        // Create a new lattice top, we shall remove it later if there is already
        // a top node in the lattice.
        int NextID = nodes_sz;
        LatticeTop = new Concept( cl );
        LatticeTop->setID( NextID );
        cl->addConcept( LatticeTop );
    }
    LG_FCA_D << "Lattice top ID:" << LatticeTop->getID() << endl;
    if( LatticeBottom )
        LG_FCA_D << "LatticeBottom ID:" << LatticeBottom->getID() << endl;
    LG_FCA_D << "TRACE. Lattice top ID:" << LatticeTop->getID() << endl;
    if( LatticeBottom )
        LG_FCA_D << "TRACE. LatticeBottom ID:" << LatticeBottom->getID() << endl;

    
    bool Collect_sumOfAllIntentIntersectionSizes = true;
    typedef map< int, int > sumOfAllIntentIntersectionSizes_t;
    sumOfAllIntentIntersectionSizes_t sumOfAllIntentIntersectionSizes;
    int sumOfAllBorderSizes = 0;
    Intents_t Intents;
    clist_t Border;
    Border.insert( LatticeTop );

#ifdef HAVE_LIBGISTMIQ
    typedef bvector<> key_type;
    typedef EasyRDTreeClientNative tree_type;
    tree_type BorderGIST("tmp-fca-gist-border");
    tree_type::iterator BorderGIST_end = BorderGIST.end();
    BorderGIST.insert( LatticeTop->getItemSetBitSet(), LatticeTop->getID() );
    cerr << "TOP.bv:" << libgist::tostr( LatticeTop->getItemSetBitSet() ) << endl;
//    fh_concept BorderGIST_PossibleExtraConcept = LatticeTop;
#endif
    
    FerrisBitMagic::bvector<> tmpbv( //FerrisBitMagic::BM_GAP,
                                     FerrisBitMagic::BM_BIT,
                                     FerrisBitMagic::gap_len_table<false>::_len,
                                     64 );

// //    Intents.reserve( nodes_sz );
//     typedef vector< IntentsData* > IDataCache_t;
//     IDataCache_t IDataCache;
//     int IDataCache_sz = nodes_sz + 10;
//     IDataCache.reserve( IDataCache_sz );
//     FerrisBitMagic::bvector<> IDataCacheTemplate;
//     for( int i=0; i < IDataCache_sz; ++i )
//     {
//         IDataCache[i] = new IntentsData( 0, IDataCacheTemplate );
//     }
//     IDataCache_t::iterator IDataCacheEnd   = IDataCache.end();
//     IDataCache_t::iterator IDataCacheBegin = IDataCache.begin();
//     IDataCache_t::iterator IDataCacheIter  = IDataCacheBegin;

    
    Benchmark bm( "BENCH: border Finding parent/child links...");
    nodes_t::iterator begin = nodes.begin();
    nodes_t::iterator   end = nodes.end();
    for( nodes_t::iterator ni = begin; ni != end; ++ni )
    {
        fh_concept n = *ni;
//        n->optimizeItemSet();
#ifdef USE_BITSET
        const fca_std_bitset_t& nIntent = n->getItemSetBitSet();
#else
        const FerrisBitMagic::bvector<>& nIntent = n->getItemSet();
#endif

        
        if( LG_FCA_ACTIVE )
        {
            LG_FCA_D << "------------------------------------------------------------" << endl;
            LG_FCA_D << "------------------------------------------------------------" << endl;
            LG_FCA_D << "--- Starting on n:" << n->getID() << " ---------------------" << endl;
            dumpBits( n->getItemSet() );
            {
                fh_stringstream ss;
                dump( ss, Border, "border set" );
                LG_FCA_D << "" << ss.str() << endl;
            }
            LG_FCA_D << "------------------------------------------------------------" << endl;
            LG_FCA_D << "------------------------------------------------------------" << endl;
        }
        

        
        Intents.clear();
//        IDataCacheIter = IDataCacheBegin;

#ifdef HAVE_LIBGISTMIQ

        if( useGIST )
        {
//             if( LG_FCA_ACTIVE )
//             {
//                 LG_FCA_D << "   nIntent:"; dumpBits( nIntent );
//             }
            
            
            tree_type::iterator iter = BorderGIST.subsets( nIntent );
            for( ; iter != BorderGIST_end; ++iter )
            {
                LG_FCA_D << "---------------" << endl;
                LG_FCA_D << "iter.data:" << iter->second << endl;
                LG_FCA_D << "iter.bv:"   << libgist::tostr_binary(iter->first) << endl;

                long cid = iter->second;
                LG_FCA_D << "GIST concept-id:" << cid << endl;
                fh_concept borderNode = cl->getConcept( cid );

                fh_IntentsData idata = new IntentsData( borderNode, borderNode->getItemSetBitSet() );
                idata->intent &= nIntent;
                int count = idata->intent.count();
                Intents.insert( make_pair( count, idata ) );

                if( Collect_sumOfAllIntentIntersectionSizes )
                {
                    sumOfAllIntentIntersectionSizes[count]++;
                }

                LG_FCA_D << "   borderID    :" << borderNode->getID() << endl;
                LG_FCA_D << "   borderIntent:"; dumpBits( borderNode->getItemSet() );
                
            }
        }
        
#endif

        if( LG_FCA_ACTIVE )
        {
            LG_FCA_D << "TRACE. ===========================================" << endl;
            LG_FCA_D << "TRACE. ===========================================" << endl;
            LG_FCA_D << "TRACE. ===========================================" << endl;
            LG_FCA_D << "TRACE. WORKING ON CONCEPT:" << n->getID() << endl;
            LG_FCA_D << "TRACE. WORKING ON CONCEPT:" << n->getItemSetBitSet() << endl;
            {
                LG_FCA_D << "TRACE.TEX \\item   Current Concept = $\\{"
                         << getTEXLabel(n->getItemSet())
                         << "\\}$ \\\\" << endl;
            }

            stringstream borderTEX;
            LG_FCA_D << "TRACE. ------border dump -------------- " << endl;
            LG_FCA_D << "TRACE. Border.sz:" << Border.size() << endl;
            for( clist_t::const_iterator bi = Border.begin(); bi!=Border.end(); ++bi )
            {
                fh_concept borderNode = *bi;
                const fca_std_bitset_t& bnodebits = borderNode->getItemSetBitSet();
                LG_FCA_D << "TRACE. borderID    :" << borderNode->getID() << endl;
                LG_FCA_D << "TRACE. borderIntent:" << bnodebits << endl;
                if( !borderTEX.str().empty() )
                    borderTEX << ", ";
                borderTEX << getTEXLabel(borderNode->getItemSet());
            }
            LG_FCA_D << "TRACE. ------border dump -------------- " << endl;


            {
                LG_FCA_D << "TRACE.TEX	Border          = $\\{"
                         << borderTEX.str()
                         << "\\}$ \\\\" << endl;
            }
            
            
            if( LatticeBottom && n->getID() == LatticeBottom->getID() )
            {
                LG_FCA_D << "TRACE. BOTTOM NODE!---------- " << endl;
                LG_FCA_D << "TRACE. borderID    :" << n->getID() << endl;
                LG_FCA_D << "TRACE. borderIntent:" << n->getItemSetBitSet() << endl;
                fh_stringstream ss;
                dumpBits( n->getItemSet(), ss );
                LG_FCA_D << "TRACE. borderIntent:" << ss.str() << endl;
                
            }
            
        }
        
        if( !useGIST )
        {
        
            for( clist_t::const_iterator bi = Border.begin(); bi!=Border.end(); ++bi )
            {
                fh_concept borderNode = *bi;
//            fh_IntentsData idata = new IntentsData( borderNode, borderNode->getItemSet() & nIntent );

#ifdef USE_BITSET
            
                fh_IntentsData idata = new IntentsData( borderNode, borderNode->getItemSetBitSet() );
                LG_FCA_D << "TRACE.bi bn.bits:" << borderNode->getItemSetBitSet() << endl;
                LG_FCA_D << "TRACE.bi d.intent1:" << idata->intent << endl;
                
                idata->intent &= nIntent;
                int count = idata->intent.count();
                Intents.insert( make_pair( count, idata ) );

                LG_FCA_D << "TRACE.bi d.intent2:" << idata->intent << endl;
#else

                fh_IntentsData idata = new IntentsData( borderNode, borderNode->getItemSet() );
//            idata->intent &= nIntent;
                first_and_eq_second( idata->intent, nIntent );
//            idata->intent.optimize();
                int count = idata->intent.count();
                Intents.insert( make_pair( count, idata ) );
            
#endif       

//             IntentsData* idata = *IDataCacheIter;
//             ++IDataCacheIter;
//             idata->node = borderNode;
//             idata->intent = borderNode->getItemSet() & nIntent;
// //            first_and_eq_second( idata->intent, nIntent );
//             int count = idata->intent.count();
//             Intents.insert( make_pair( count, idata ) );
            
            
//             FerrisBitMagic::bvector<> tmpbv( borderNode->getItemSet() );
//             tmpbv &= nIntent;
//             int count = tmpbv.count();
//             if( !count )
//                 continue;
//             fh_IntentsData idata = new IntentsData( borderNode, tmpbv );
//             Intents.insert( make_pair( count, idata ) );
            
            

                if( Collect_sumOfAllIntentIntersectionSizes )
                {
                    sumOfAllIntentIntersectionSizes[count]++;
                }

            
                LG_FCA_D << "   borderID    :" << borderNode->getID() << endl;
                LG_FCA_D << "   borderIntent:"; dumpBits( borderNode->getItemSet() );
//            LG_FCA_D << "generatedIntent:"; dumpBits( idata->intent );
            }
        }

        if( LG_FCA_ACTIVE )
        {
            stringstream borderTEX;
            LG_FCA_D << "TRACE. ------intents dump -------------- " << endl;
            LG_FCA_D << "TRACE. Intents.sz:" << Intents.size() << endl;
            for( Intents_t::reverse_iterator ii = Intents.rbegin(); ii!=Intents.rend(); ++ii )
            {
                const IntentsData::intent_t& iIntent = ii->second->intent;
                LG_FCA_D << "TRACE. nodeID:" << ii->second->node->getID() << endl;
                LG_FCA_D << "TRACE. node-intent:" << ii->second->node->getItemSetBitSet() << endl;
                LG_FCA_D << "TRACE.  sec-intent:" << ii->second->intent << endl;

                if( !borderTEX.str().empty() )
                    borderTEX << ", ";
//                borderTEX << getTEXLabel(ii->second->node->getItemSet());
                borderTEX << getTEXLabel(ii->second->intent);
            }
            LG_FCA_D << "TRACE. ------intents dump -------------- " << endl;

            {
                LG_FCA_D << "TRACE.TEX	Intents         = $\\{"
                         << borderTEX.str()
                         << "\\}$ \\\\" << endl;
            }
        }

        
        // Filter Intents to only have Maxima
        Intents_t CoverIntents;
        Maxima( Intents, CoverIntents );


        if( LG_FCA_ACTIVE )
        {
            stringstream borderTEX;
            LG_FCA_D << "TRACE. ------CoverIntents dump -------------- " << endl;
            LG_FCA_D << "TRACE. CoverIntents.sz:" << CoverIntents.size() << endl;
            for( Intents_t::reverse_iterator ii = CoverIntents.rbegin(); ii!=CoverIntents.rend(); ++ii )
            {
                const IntentsData::intent_t& iIntent = ii->second->intent;
                LG_FCA_D << "TRACE. nodeID:" << ii->second->node->getID() << endl;
                LG_FCA_D << "TRACE. node-intent:" << ii->second->node->getItemSetBitSet() << endl;
                LG_FCA_D << "TRACE.  sec-intent:" << ii->second->intent << endl;

                if( !borderTEX.str().empty() )
                    borderTEX << ", ";
//                borderTEX << getTEXLabel(ii->second->node->getItemSet());
                borderTEX << getTEXLabel(ii->second->intent);
            }
            LG_FCA_D << "TRACE. ------intents dump -------------- " << endl;

            {
                LG_FCA_D << "TRACE.TEX	Maxima(Intents) = $\\{"
                         << borderTEX.str()
                         << "\\}$ \\\\" << endl;
            }
        }


        
        sumOfAllBorderSizes += Border.size();
        LG_FCA_D << "Border.size:" << Border.size() << endl;
        LG_FCA_D << "Intents.size:" << Intents.size() << endl;
        LG_FCA_D << "CoverIntents.size:" << CoverIntents.size() << endl;
        
        for( Intents_t::const_iterator ii = CoverIntents.begin(); ii!=CoverIntents.end(); ++ii )
        {
//            LG_FCA_D << "Calling findconcept(1)" << endl;

            fh_concept parent = FindConcept<IntentsData::intent_t>( ii->second->node, ii->second->intent );

//            LG_FCA_D << "Called  findconcept(2)" << endl;
            if( !parent )
            {
                LG_FCA_D << "ERROR concept not found for intent" << endl;
//                dumpBits( ii->second->intent );
                LG_FCA_D << endl << endl;
                continue;
            }
            LG_FCA_D << "parent:" << parent->getID() << endl;
            LG_FCA_D << "child:" << n->getID() << endl;
            parent->makeLink( n, false );
            TotalEdges++;


            if( LG_FCA_ACTIVE )
            {
                LG_FCA_D << "TRACE. parent:" << parent->getID() << " child:" << n->getID() << endl;
                LG_FCA_D << "TRACE. parent:" << parent->getItemSetBitSet()
                         << " child:" << n->getItemSetBitSet() << endl;

                {
                    LG_FCA_D << "TRACE.TEX	Add edge $"
                             << getTEXLabel(parent->getItemSet())
                             << " \\rightarrow "
                             << getTEXLabel(n->getItemSet())
                             << "$ \\\\" << endl;
                }
                
            }
            
            if( useGIST )
            {
#ifdef HAVE_LIBGISTMIQ
                BorderGIST.erase( parent->getItemSetBitSet() );
#endif
            }
            else
            {
                Border.erase( parent );
            }
            
            if( LG_FCA_ACTIVE )
            {
                fh_stringstream ss;
                dumpBits( parent->getItemSet(), ss );
                LG_FCA_D << "Removing from border set:" << ss.str();
            }
        }
#ifdef HAVE_LIBGISTMIQ
        if( useGIST )
        {
            BorderGIST.insert( n->getItemSetBitSet(), n->getID() );
        }
#endif        
        if( !useGIST )
        {
            Border.insert( n );
        }
    }
    bm.print();
    

    // If the synthetic lattice top node was made in error remove it and promote
    // the correct child to become the new lattice top
    {
        fh_concept newRoot = 0;
        
        const clist_t& children = LatticeTop->getChildren();
        for( clist_t::const_iterator ci = children.begin(); ci!=children.end(); ++ci )
        {
            fh_concept child = *ci;
            
            if( child->getChildren().empty() )
            {
                newRoot = *ci;
                break;
            }
        }
        if( newRoot )
        {
            LG_FCA_D << "newRoot ID:" << newRoot->getID() << endl;
            LG_FCA_D << "oldRoot ID:" << LatticeTop->getID() << endl;
            for( clist_t::const_iterator ci = children.begin(); ci!=children.end(); ++ci )
            {
                if( (*ci)->getID() != newRoot->getID() )
                    newRoot->makeLink( *ci, false );
            }
            cl->removeConcept( LatticeTop );
        }
    }
    
    
    
    

    if( Collect_sumOfAllIntentIntersectionSizes )
    {
        int nodes_sz = nodes.size();
        if( !nodes_sz ) nodes_sz = 1;
        
        for( sumOfAllIntentIntersectionSizes_t::const_iterator ci = sumOfAllIntentIntersectionSizes.begin();
             ci != sumOfAllIntentIntersectionSizes.end(); ++ci )
        {
            int sz    = ci->first;
            int count = ci->second;

            cerr << "Intent.sz:" << sz
                 << " intersection count:" << count
                 << " avg per concept:" << (count*1.0/nodes_sz)
                 << endl;
            
        }
    }

    
    if( !nodes.empty() )
        cerr << "Average border size:" << (1.0*sumOfAllBorderSizes/nodes.size()) << endl;
    cerr << "TotalConcepts:" << nodes.size() << endl;
    cerr << "TotalEdges:" << TotalEdges << endl;
}



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


struct IntentsDataNative;
FERRIS_SMARTPTR( IntentsDataNative, fh_IntentsDataNative );
struct IntentsDataNative : public Handlable
{
    fh_concept node;
    typedef guint64 intent_t;
    intent_t intent;

    static intent_t toNative( const FerrisBitMagic::bvector<>& _intent )
        {
            intent_t ret = 0;
            FerrisBitMagic::bvector<>::enumerator an     = _intent.first();
            FerrisBitMagic::bvector<>::enumerator an_end = _intent.end();

            while (an < an_end)
             {
                 ret |= ( 1ULL << *an );
                 ++an;
             }
            return ret;
        }
    IntentsDataNative( fh_concept node, const FerrisBitMagic::bvector<>& _intent )
        :
        node( node ), intent( 0 )
        {
            intent = toNative( _intent );
        }

    void operator&= ( const FerrisBitMagic::bvector<>& _intent )
        {
            intent &= toNative( _intent );
        }

    void operator&= ( intent_t v )
        {
            intent &= v;
        }
    
    int getIntentCount()
        {
            long ret = 0;
            for( long i = 0; i < 64; ++i )
            {
                if( (intent >> i) & 0x1 == 0x1 )
                    ++ret;
            }
            return ret;
                
//            return (int)log2( intent );
        }

    bool arg_is_superset( fh_IntentsDataNative& other )
        {
            intent_t v = other->intent;
            intent_t t = intent & v;
            return t == intent;
        }
    bool arg_is_superset( const FerrisBitMagic::bvector<>& other )
        {
            intent_t v = toNative( other );
            intent_t t = intent & v;
            return t == intent;
        }
    bool equiv( const FerrisBitMagic::bvector<>& other )
        {
            intent_t v = toNative( other );
            return v == intent;
        }
    
            
};
typedef multimap< int, fh_IntentsDataNative > IntentsNative_t;


IntentsNative_t& Maxima( IntentsNative_t& input, IntentsNative_t& ret )
{
    for( IntentsNative_t::reverse_iterator ii = input.rbegin(); ii!=input.rend(); ++ii )
    {
//        const IntentsDataNative::intent_t& iIntent = ii->second->intent;
        bool ismin = true;
        for( IntentsNative_t::iterator ri = ret.begin(); ri!=ret.end(); ++ri )
        {
//            ismin &= (!second_is_superset(iIntent, ri->second->intent));
            ismin &= !ii->second->arg_is_superset( ri->second );
            if( !ismin )
                break;
        }

        if( ismin )
        {
            ret.insert( *ii );
        }
    }

    return ret;
}


fh_concept FindConceptNative( fh_concept startc, fh_IntentsDataNative intent )
{
//    cerr << "FindConceptNative(enter)" << endl;
    fh_concept c = startc;
    while( !intent->equiv( c->getItemSet() ) )
    {
        bool changedC = false;
        const clist_t& pl = c->getParents();
//        cerr << "FindConceptNative() c:" << c->getID() << " pl.sz:" << pl.size() << endl;
        for( clist_t::const_iterator pi = pl.begin(); pi!=pl.end(); ++pi )
        {
//            if( !second_is_superset( intent, (*pi)->getItemSet() ) )
            if( !intent->arg_is_superset( (*pi)->getItemSet() ) )
            {
//                cerr << "FindConceptNative(SS) pi:" << (*pi)->getID() << endl;
                continue;
            }
            c = *pi;
            changedC = true;
            break;
        }
        if( !changedC )
        {
//            cerr << "FindConceptNative(exit)" << endl;
            return 0;
        }
    }
//    cerr << "FindConceptNative(exit)" << endl;
    return c;
}


void setupParentAndChildrenLinks_NativeBits()
{
    Time::Benchmark setupParentAndChildrenLinks_benchmark("setupParentAndChildrenLinks()");
    
    
    int TotalEdges = 0;
    LatticeTop = 0;

    if( LatticeTop )
    {
    }
    else
    {
        // Create a new lattice top, we shall remove it later if there is already
        // a top node in the lattice.
        int NextID = nodes.size();
        LatticeTop = new Concept( cl );
        LatticeTop->setID( NextID );
        cl->addConcept( LatticeTop );
    }
    LG_FCA_D << "Lattice top ID:" << LatticeTop->getID() << endl;
    
    bool Collect_sumOfAllIntentIntersectionSizes = true;
    typedef map< int, int > sumOfAllIntentIntersectionSizes_t;
    sumOfAllIntentIntersectionSizes_t sumOfAllIntentIntersectionSizes;
    int sumOfAllBorderSizes = 0;
    IntentsNative_t Intents;
    clist_t Border;
    Border.insert( LatticeTop );

    Benchmark bm( "BENCH: native border Finding parent/child links...");
    nodes_t::iterator begin = nodes.begin();
    nodes_t::iterator   end = nodes.end();
    for( nodes_t::iterator ni = begin; ni != end; ++ni )
    {
        fh_concept n = *ni;
        const FerrisBitMagic::bvector<>& nIntent = n->getItemSet();
        IntentsDataNative::intent_t nIntent64 = IntentsDataNative::toNative( nIntent );
        
        if( LG_FCA_ACTIVE )
        {
            LG_FCA_D << "------------------------------------------------------------" << endl;
            LG_FCA_D << "------------------------------------------------------------" << endl;
            LG_FCA_D << "--- Starting on n:" << n->getID() << " ---------------------" << endl;
            dumpBits( n->getItemSet() );
            {
                fh_stringstream ss;
                dumpBits( IntentsDataNative::toNative( n->getItemSet()), ss );
                LG_FCA_D << ss.str() << endl;
            }
            IntentsDataNative idata( n, n->getItemSet() );
            IntentsDataNative::intent_t testA = 0x000000000000000FULL;
            bvector<> tmp;
            tmp[0] = 1;
            tmp[1] = 1;
            tmp[2] = 1;
            LG_FCA_D << "intent-count:" << idata.getIntentCount() << endl;
            idata &= tmp;
            {
                fh_stringstream ss;
                dumpBits( idata.intent, ss );
                LG_FCA_D << "n.itemset&[0,1,2] = :" << ss.str() << endl;
            }

            {
                fh_stringstream ss;
                dump( ss, Border, "border set" );
                LG_FCA_D << "" << ss.str() << endl;
            }
            
            LG_FCA_D << "------------------------------------------------------------" << endl;
            LG_FCA_D << "------------------------------------------------------------" << endl;
        }

        
        Intents.clear();
        for( clist_t::const_iterator bi = Border.begin(); bi!=Border.end(); ++bi )
        {
            fh_concept borderNode = *bi;
//            fh_IntentsDataNative idata = new IntentsDataNative( borderNode, borderNode->getItemSet() & nIntent );
            fh_IntentsDataNative idata = new IntentsDataNative( borderNode, borderNode->getItemSet() );


            
//            idata->intent &= nIntent;
            (*idata) &= nIntent64;

            
            
            int count = idata->getIntentCount();
            Intents.insert( make_pair( count, idata ) );

            if( Collect_sumOfAllIntentIntersectionSizes )
            {
                sumOfAllIntentIntersectionSizes[count]++;
            }
            
            LG_FCA_D << "   borderID    :" << borderNode->getID() << endl;
            LG_FCA_D << "   borderIntent:"; dumpBits( borderNode->getItemSet() );
//            LG_FCA_D << "generatedIntent:"; dumpBits( idata->intent );
        }

        // Filter Intents to only have Maxima
        IntentsNative_t CoverIntents;
        Maxima( Intents, CoverIntents );

        sumOfAllBorderSizes += Border.size();
        LG_FCA_D << "Border.size:" << Border.size() << endl;
        LG_FCA_D << "Intents.size:" << Intents.size() << endl;
        LG_FCA_D << "CoverIntents.size:" << CoverIntents.size() << endl;
        
        for( IntentsNative_t::const_iterator ii = CoverIntents.begin(); ii!=CoverIntents.end(); ++ii )
        {
            fh_concept parent = FindConceptNative( ii->second->node, ii->second );
            if( !parent )
            {
                LG_FCA_D << "ERROR concept not found for intent" << endl;
//                dumpBits( ii->second->intent );
                LG_FCA_D << endl << endl;
                continue;
            }
            LG_FCA_D << "parent:" << parent->getID() << endl;
            LG_FCA_D << "child:" << n->getID() << endl;
            parent->makeLink( n, false );
            TotalEdges++;
            Border.erase( parent );

            if( LG_FCA_ACTIVE )
            {
                fh_stringstream ss;
                dumpBits( parent->getItemSet(), ss );
                LG_FCA_D << "Removing from border set:" << ss.str();
            }
        }
        Border.insert( n );
    }
    bm.print();
    

    // If the synthetic lattice top node was made in error remove it and promote
    // the correct child to become the new lattice top
    {
        fh_concept newRoot = 0;
        
        const clist_t& children = LatticeTop->getChildren();
        for( clist_t::const_iterator ci = children.begin(); ci!=children.end(); ++ci )
        {
            fh_concept child = *ci;
            
            if( child->getChildren().empty() )
            {
                newRoot = *ci;
                break;
            }
        }
        if( newRoot )
        {
            LG_FCA_D << "newRoot ID:" << newRoot->getID() << endl;
            LG_FCA_D << "oldRoot ID:" << LatticeTop->getID() << endl;
            for( clist_t::const_iterator ci = children.begin(); ci!=children.end(); ++ci )
            {
                if( (*ci)->getID() != newRoot->getID() )
                    newRoot->makeLink( *ci, false );
            }
            cl->removeConcept( LatticeTop );
        }
    }
    
    
    
    

    if( Collect_sumOfAllIntentIntersectionSizes )
    {
        int nodes_sz = nodes.size();
        if( !nodes_sz ) nodes_sz = 1;
        
        for( sumOfAllIntentIntersectionSizes_t::const_iterator ci = sumOfAllIntentIntersectionSizes.begin();
             ci != sumOfAllIntentIntersectionSizes.end(); ++ci )
        {
            int sz    = ci->first;
            int count = ci->second;

            cerr << "Intent.sz:" << sz
                 << " intersection count:" << count
                 << " avg per concept:" << (count*1.0/nodes_sz)
                 << endl;
            
        }
    }

    
    if( !nodes.empty() )
        cerr << "Average border size:" << (1.0*sumOfAllBorderSizes/nodes.size()) << endl;
    cerr << "TotalConcepts:" << nodes.size() << endl;
    cerr << "TotalEdges:" << TotalEdges << endl;
}




void dumpParentChildrenLinks()
{
    for( nodes_t::iterator ni = nodes.begin();
         ni != nodes.end(); ++ni )
    {
        fh_concept c = *ni;
        cerr << "c:" << c->getID() << " has-children" << endl;
        clist_t& children = c->getChildren();
        
        for( clist_t::iterator ci = children.begin(); ci != children.end(); ++ci )
        {
            cerr << "       child:" << (*ci)->getID() << endl;
        }
        cerr << endl;
    }
}

//
// precalculate the number of objects which match only this concept.
// ie. they are not in any of the subconcepts.
//
void
setupConceptOnlySupport()
{
    Time::Benchmark refreshAllConceptsContingentCounter_benchmark("refreshAllConceptsContingentCounter()");
    cl->refreshAllConceptsContingentCounter();
    
//     for( nodes_t::iterator ni = nodes.begin(); ni != nodes.end(); ++ni )
//     {
//         fh_concept c = *ni;
//         c->updateConceptOnlyMatchSize();
//     }
}



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

int main( int argc, const char** argv )
{
    unsigned long ShowVersion        = 0;
    const char* flatticeTreePath_CSTR = 0;
    unsigned long UseCoveringEdgesAlgo = 0;
    unsigned long UseCoveringEdgesAlgoNative = 0;
    unsigned long UseNativeBitsBorder = 0;
    unsigned long DumpLinksToCerr   = 0;
    unsigned long DumpNodesToCerr_PriorToFindingLinks = 0;
    unsigned long DontSave = 0;
    unsigned long useGIST = 0;
    unsigned long populateGIST = 0;

#ifdef HAVE_LIBGISTMIQ
    gist_init();
#endif

    struct poptOption optionsTable[] = {

        { "lattice-tree-path", 'L', POPT_ARG_STRING, &flatticeTreePath_CSTR, 0,
          "url for the lattice tree", "" },

        { "gist", 'g', POPT_ARG_NONE, &useGIST, 0,
          "use rd-tree based gist to speed up set selects", "" },

        { "populate-gist", 'G', POPT_ARG_NONE, &populateGIST, 0,
          "assume that gist needs to be populated before use (no existing tree on disk)", "" },
        
        { "covering-edges-algo", 0, POPT_ARG_NONE, &UseCoveringEdgesAlgo, 0,
          "use covering-edges method from concept data analysis (debug/benchmark)", 0 },

        { "covering-edges-algo-native", 0, POPT_ARG_NONE, &UseCoveringEdgesAlgoNative, 0,
          "use covering-edges method (with boost::dynamic_bitset<>) from concept data analysis (debug/benchmark)", 0 },

        
        { "native-bits-border", 0, POPT_ARG_NONE, &UseNativeBitsBorder, 0,
          "use native long long for bitmask in border algo. Only useful with formal attribute count < 64 (debug/benchmark)", 0 },
        
        { "results-to-cerr", 0, POPT_ARG_NONE, &DumpLinksToCerr, 0,
          "show parent/child links found to cerr for debugging.", 0 },

        { "dump-nodes-prior", 0, POPT_ARG_NONE, &DumpNodesToCerr_PriorToFindingLinks, 0,
          "show all nodes in traversal order before finding parent/child relations..", 0 },

        { "dont-save", 0, POPT_ARG_NONE, &DontSave, 0,
          "dont save any modifications to the database..", 0 },

        { "clarify", 0, POPT_ARG_NONE, &CLARIFY, 0,
          "For covering-edges, clarify the context first..", 0 },
        
        /*
         * Other handy stuff
         */
        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },
        
        POPT_AUTOHELP
        POPT_TABLEEND
    };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "-L path-to-fcatree-subdir [OPTIONS]* ...");

    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
//         switch (c) {
//         }
    }


    if (argc < 2 || !flatticeTreePath_CSTR )
    {
        if( !flatticeTreePath_CSTR )
            cerr << "ERROR: you must supply an fca lattice tree path" << endl;
        
//        poptPrintHelp(optCon, stderr, 0);
        poptPrintUsage(optCon, stderr, 0);
        exit(5);
    }
    flatticeTreePath = flatticeTreePath_CSTR;
    
    if( ShowVersion )
    {
        cout << "LatticeFromCFI version: $Id: ferris-lattice-from-cfi.cpp,v 1.33 2010/09/24 21:31:11 ben Exp $\n"
             << "Written by Ben Martin, aka monkeyiq" << endl
             << endl
             << "Copyright (C) 2005 Ben Martin" << endl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }    

    try
    {
        cerr << "getting the data..." << endl;
            
        cl            = ConceptLattice::load( flatticeTreePath );
        maxID         = cl->getHighestConceptID();
        cerr << "maxid:" << maxID << endl;
        fh_concept  c = cl->getConcept( maxID );

        //
        // Make sure there is a bottom element.
        //
        cerr << "got maxid concept:" << c->getID() << endl;
        FerrisBitMagic::bvector<> bv_full = cl->getFullItemSet();

        cerr << "bv_full:" << bv_full << endl;
        cerr << "maxc_bv:" << c->getItemSet() << endl;
            
        if( equiv( bv_full & c->getItemSet(), bv_full ) )
        {
            LatticeBottom = c;
            cerr << "LatticeBottom = " << c->getID() << endl;
        }
        else
        {
            cerr << "Making new lattice bottom..." << endl;
            LatticeBottomIsAlreadyInDatabase = false;

            LatticeBottom = new Concept( cl );
            LatticeBottom->setID( maxID + 1000 );
            LatticeBottom->setItemSet( bv_full );
            cl->addConcept( LatticeBottom );
            cerr << "new LatticeBottom = " << c->getID() << endl;
        }

            
        cerr << "getting nodes..." << endl;
        cl->getConcepts( nodes );
        cerr << "got nodes..." << endl;

        if( DumpNodesToCerr_PriorToFindingLinks )
            dumpNodes( Factory::fcout() );

        cerr << "Working out the parent/child relations..." << endl;
        if( UseCoveringEdgesAlgo )
        {
            setupParentAndChildrenLinks_CoveringEdges( useGIST, populateGIST );
        }
        else if( UseCoveringEdgesAlgoNative )
        {
            setupParentAndChildrenLinks_CoveringEdgesNativeBitSet();
        }
        else if( UseNativeBitsBorder )
        {
            setupParentAndChildrenLinks_NativeBits();
        }
        else
        {
            setupParentAndChildrenLinks( useGIST );
        }

        if( DumpLinksToCerr )
            dumpParentChildrenLinks();

//        cerr << "FIXME! not saving!" << endl;
        if( !DontSave )
        {
            cerr << "Finding the contingent size for each concept..." << endl;
            setupConceptOnlySupport();

            cl->save();
            cl->fixInvalidTopLevelConcepts();
        }
        
        cerr << "all done." << endl;
    }
    catch( exception& e )
    {
        LG_FCA_ER << PROGRAM_NAME << " cought exception:" << e.what() << endl;
        cerr << "error:" << e.what() << endl;
        exit(3);
    }
    
    
    poptFreeContext(optCon);
    cout << flush;
    return hadErrors;
}
