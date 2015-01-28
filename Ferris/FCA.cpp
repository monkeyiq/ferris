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

    $Id: FCA.cpp,v 1.26 2010/09/24 21:30:32 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <config.h>

#include <Ferris/FCA.hh>
#include <Ferris/General.hh>
#include <Ferris/Ferris_private.hh>
#include <Ferris/Iterator.hh>
#include <Ferris/EAIndexerMetaInterface.hh>
#include <Ferris/EAIndexerSQLCommon_private.hh>
#include <Ferris/EAQuery.hh>

// required for RootContextDropper
#include <Ferris/Resolver_private.hh>

#ifdef HAVE_LIBPQXX
#include <pqxx/connection>
#include <pqxx/tablewriter>
#include <pqxx/transaction>
#include <pqxx/nontransaction>
#include <pqxx/tablereader>
#include <pqxx/tablewriter>
#include <pqxx/result>
#include <pqxx/cursor>
#include <fstream>

#include <boost/regex.hpp>

using namespace PGSTD;
using namespace pqxx;

extern "C" {
//#include <pgsql/server/catalog/pg_type.h>
#define BOOLOID			16
#define INT8OID			20
#define INT2OID			21
#define INT2VECTOROID	22
#define INT4OID			23
#define VARCHAROID		1043
#define DATEOID			1082
#define TIMEOID			1083
#define TIMESTAMPOID	1114
};

#endif


namespace Loki
{
    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP
    >
    inline bool operator<(const SmartPtr<T, OP, CP, KP, SP>& lhs, const ::Ferris::Context* rhs)
    {
        return GetImpl(lhs) < rhs;
    }
};



namespace Ferris
{
    using namespace std;
    using namespace FerrisBitMagic;
    
    template <class Col, class Obj>
    void erase( Col& col, Obj& o )
    {
        typedef typename Col::iterator ITER;
        ITER ci = col.find( o );
        if( ci != col.end() )
            col.erase( ci );
        
//        col.erase( col.find( o ));
//        col.erase( o );
    }

    string tostr( double v )
    {
        stringstream ss;
        ss << v;
        return ss.str();
    }
    string tostr( int v )
    {
        stringstream ss;
        ss << v;
        return ss.str();
    }
    
    
    namespace FCA 
    {
        static const int   EXTENT_VIEWER_DEFAULT_MAX_FILES = 20;
        static const int   EXTENT_VIEWER_LARGE_MAX_FILES   = 1000;
        static const char* EXTENT_SUBDIR_NAME              = "-all";
        static const int   EXTENT_SUBDIR_LIMIT             = EXTENT_VIEWER_LARGE_MAX_FILES;
        static const char* EXTENT_SAMPLE_SUBDIR_NAME       = "-all-s";
        static const int   EXTENT_SAMPLE_SUBDIR_LIMIT      = EXTENT_VIEWER_DEFAULT_MAX_FILES;
        static const char* CONTINGENT_SUBDIR_NAME          = "-self";
        static const int   CONTINGENT_SUBDIR_LIMIT         = EXTENT_VIEWER_LARGE_MAX_FILES;
        static const char* CONTINGENT_SAMPLE_SUBDIR_NAME   = "-self-s";
        static const int   CONTINGENT_SAMPLE_SUBDIR_LIMIT  = EXTENT_VIEWER_DEFAULT_MAX_FILES;

        string getBitFunctionName( const std::string& treeName,
                                   const std::string& attributeName,
                                   bool quote )
        {
            stringstream ss;
            if( quote ) ss << " \"";
            ss << EANameToSQLColumnName(treeName) << "_"
               << EANameToSQLColumnName( attributeName ) ;
            if( quote ) ss << "\" ";
            return ss.str();
        }

        void setFile( fh_context parent, const std::string& rdn,
                      const std::string& data, int mode )
        {
            fh_context c = Shell::acquireSubContext( parent, rdn, false, mode );
            setStrAttr( c, "content", data + '\n' );
        }

        void setFile( fh_context parent, const std::string& rdn,
                      stringstream& dataSS, int mode )
        {
            setFile( parent, rdn, dataSS.str(), mode );
        }
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        bool anythingSet( const FerrisBitMagic::bvector<>& a )
        {
            return a.count() > 0;
        }

        bool equiv( const fca_std_bitset_t& a, const fca_std_bitset_t& b )
        {
            return a == b;
        }
        
        
        bool equiv( const FerrisBitMagic::bvector<>& a, const FerrisBitMagic::bvector<>& b )
        {
            return a == b;
        }
        bool equiv( const FerrisBitMagic::bvector<>& a,
                    const FerrisBitMagic::bvector<
                    FerrisBitMagic::standard_allocator, 
                    FerrisBitMagic::miniset<
                    FerrisBitMagic::block_allocator, FerrisBitMagic::set_total_blocks> >& b )
        {
            FerrisBitMagic::bvector<>::enumerator an     = a.first();
            FerrisBitMagic::bvector<>::enumerator an_end = a.end();

            while (an < an_end)
            {
                if( !b[ *an ] )
                    return false;
                ++an;  // Fastest way to increment enumerator
            }
            return true;
        }
    
        

        bool equiv_by_iter( const FerrisBitMagic::bvector<>& a, const FerrisBitMagic::bvector<>& b )
        {
            FerrisBitMagic::bvector<>::enumerator an     = a.first();
            FerrisBitMagic::bvector<>::enumerator an_end = a.end();

            while (an < an_end)
            {
                if( !b[ *an ] )
                    return false;
                ++an;  // Fastest way to increment enumerator
            }
            return true;
        }
        
        bool isZero( const FerrisBitMagic::bvector<>& a )
        {
            return a.count() == 0;
        }

        
        bool second_is_superset( const fca_std_bitset_t& a, const fca_std_bitset_t& b )
        {
            return a.is_subset_of( b );
//            return (a & b) == a;
        }
        

        bool second_is_superset( const FerrisBitMagic::bvector<>& a,
                                 const FerrisBitMagic::bvector<>& b )
        {
//             FerrisBitMagic::bvector<> t = a & b;
//             return t == b;
            
            FerrisBitMagic::bvector<>::enumerator an     = a.first();
            FerrisBitMagic::bvector<>::enumerator an_end = a.end();

            while (an < an_end)
            {
                if( !b[ *an ] )
                    return false;
                ++an;  // Fastest way to increment enumerator
            }
            return true;
        }
        

        bool second_is_superset( const FerrisBitMagic::bvector<>& a,
                                 const FerrisBitMagic::bvector<
                                 FerrisBitMagic::standard_allocator, 
                                 FerrisBitMagic::miniset<
                                 FerrisBitMagic::block_allocator, FerrisBitMagic::set_total_blocks> >& b )
        {
//             FerrisBitMagic::bvector<> t = a & b;
//             return t == b;
            
            FerrisBitMagic::bvector<>::enumerator an     = a.first();
            FerrisBitMagic::bvector<>::enumerator an_end = a.end();

            while (an < an_end)
            {
                if( !b[ *an ] )
                    return false;
                ++an;  // Fastest way to increment enumerator
            }
            return true;
        }

        bool second_is_superset( const FerrisBitMagic::bvector<
                                 FerrisBitMagic::standard_allocator, 
                                 FerrisBitMagic::miniset<
                                 FerrisBitMagic::block_allocator, FerrisBitMagic::set_total_blocks> >& a,
                                 const FerrisBitMagic::bvector<>& b )
        {
            typedef FerrisBitMagic::bvector<
                                 FerrisBitMagic::standard_allocator, 
                                 FerrisBitMagic::miniset<
            FerrisBitMagic::block_allocator, FerrisBitMagic::set_total_blocks> >::enumerator enumerator_t;
            enumerator_t an     = a.first();
            enumerator_t an_end = a.end();
            
            while (an < an_end)
            {
                if( !b[ *an ] )
                    return false;
                ++an;  // Fastest way to increment enumerator
            }
            return true;
        }
        bool second_is_superset( const FerrisBitMagic::bvector<
                                 FerrisBitMagic::standard_allocator, 
                                 FerrisBitMagic::miniset<
                                 FerrisBitMagic::block_allocator, FerrisBitMagic::set_total_blocks> >& a,
                                 const FerrisBitMagic::bvector<
                                 FerrisBitMagic::standard_allocator, 
                                 FerrisBitMagic::miniset<
                                 FerrisBitMagic::block_allocator, FerrisBitMagic::set_total_blocks> >& b )
        {
            typedef FerrisBitMagic::bvector<
                                 FerrisBitMagic::standard_allocator, 
                                 FerrisBitMagic::miniset<
            FerrisBitMagic::block_allocator, FerrisBitMagic::set_total_blocks> >::enumerator enumerator_t;
            enumerator_t an     = a.first();
            enumerator_t an_end = a.end();
            
            while (an < an_end)
            {
                if( !b[ *an ] )
                    return false;
                ++an;  // Fastest way to increment enumerator
            }
            return true;
        }
        
        
        

//         static bit_vector& resize( bit_vector& bv, int sz )
//         {
//             bit_vector j( sz );
//             bv.swap( j );
//             return bv;
//         }
        
//         bit_vector operator&( const bit_vector& a, const bit_vector& b )
//         {
//             bit_vector ret( a.capacity() );
//             int sz = max( a.size(), b.size() );
//             for( int i = 0; i < sz; ++i )
//                 ret[ i ] = a[i] & b[i];
//             return ret;
//         }

//         bit_vector operator^( const bit_vector& a, const bit_vector& b )
//         {
//             bit_vector ret( a.capacity() );
//             int sz = max( a.size(), b.size() );
//             for( int i = 0; i < sz; ++i )
//                 ret[ i ] = a[i] ^ b[i];
//             return ret;
//         }


//         bit_vector& operator|=( bit_vector& a, const bit_vector& b )
//         {
//             bit_vector& ret = a;
//             int sz = max( a.size(), b.size() );
//             for( int i = 0; i < sz; ++i )
//                 ret[ i ] |= b[i];
//             return ret;
//         }


//         bit_vector& operator^=( bit_vector& a, const bit_vector& b )
//         {
//             bit_vector& ret = a;
//             int sz = max( a.size(), b.size() );
//             for( int i = 0; i < sz; ++i )
//             {
//                 bool t = a[i];
//                 ret[ i ] = (t ^ b[i]);
//             }
//             return ret;
//         }

//         bool anythingSet( const bit_vector& a )
//         {
//             bool ret = false;
//             int sz = a.size();
//             for( int i = 0; i < sz; ++i )
//             {
//                 if( a[i] )
//                     return true;
//             }
    
//             return ret;
//         }

//         bool equiv( const bit_vector& a, const bit_vector& b )
//         {
//             bool ret = true;
//             int sz = max( a.size(), b.size() );
//             for( int i = 0; i < sz; ++i )
//             {
//                 if( a[i] != b[i] )
//                     return false;
//             }
//             return ret;
//         }

//         bool isZero( const bit_vector& a )
//         {
//             bool ret = true;
//             int sz = a.size();
//             for( int i = 0; i < sz; ++i )
//             {
//                 if( a[i] == true )
//                     return false;
//             }
//             return ret;
//         }

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        struct ConceptLatticePriv
        {
#ifdef HAVE_LIBPQXX
            friend class Concept;
            
            /**
             * Connection to database
             */
            pqxx::connection* m_connection;
            long m_maxItemSetSizeInBits;

            ConceptLatticePriv()
                {
                    m_connection = 0;
                    m_maxItemSetSizeInBits = 0;
                }
            
            ~ConceptLatticePriv()
                {
                    if( m_connection )
                        delete m_connection;
                }
#endif
        };
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

//         template<bool T> struct itemset_gap_len_table_min
//         {
//             static const gap_word_t _len[FerrisBitMagic::gap_levels];
//         };
//         template<bool T>
//         const gap_word_t itemset_gap_len_table_min<T>::_len[FerrisBitMagic::gap_levels] =
//         { 16, 32, 64, 128 };

        Concept::~Concept()
        {
        }
        
        Concept::Concept( fh_conceptLattice cl, fh_context extent )
            :
            m_cl( cl ),
            m_extent( extent ),
            m_itemSetWeight( 0 ),
            m_extentSize( 0 ),
            m_conceptOnlyMatchSize( 0 ),
            m_id( -1 ),
            m_support_perc( 0 ),
            m_support_abs( 0 ),
            m_support_ext( 0 ),
            m_itemSet( //FerrisBitMagic::BM_GAP,
                       FerrisBitMagic::BM_BIT,
                       FerrisBitMagic::gap_len_table_min<true>::_len,
//                       itemset_gap_len_table_min<true>::_len,
                       cl->getMaxItemSetSizeInBits() ),
            m_itemSet_AsBitSet( cl->getMaxItemSetSizeInBits() ),
            m_dirty( false ),
            m_x(0.0), m_y(0.0), m_z(0.0)
        {
//            cerr << "maxItemSetSizeInBits:" << cl->getMaxItemSetSizeInBits() << endl;
            Construct( cl );
        }
        void
        Concept::Construct( fh_conceptLattice cl )
        {
//            m_cl = 0;
            
            if( !m_cl && cl )
            {
                m_cl = cl;
//                resize( m_itemSet, cl->m_itemSetSize );
            }
        }
        
        
        
        clist_t&
        Concept::getParents()
        {
            return m_parents;
        }

        int Concept::getID()
        {
            return m_id;
        }
        void Concept::setID( int v )
        {
            m_dirty = true;
            m_cl->priv_ConceptIDChanging( this, m_id, v );
            m_id = v;
        }
        
        

        void
        Concept::optimizeItemSet()
        {
            m_itemSet.optimize();
        }
        
        
        const FerrisBitMagic::bvector<>&
        Concept::getItemSet() const
        {
            return m_itemSet;
        }

        const fca_std_bitset_t&
        Concept::getItemSetBitSet() const
        {
            return m_itemSet_AsBitSet;
        }
        
        

        void
        Concept::setItemSet( const FerrisBitMagic::bvector<>& v )
        {
            m_dirty = true;
            m_cl->priv_ConceptItemSetChanging( this, m_itemSet, v );
            m_itemSet = v;

            for( int i=0; i<m_cl->getMaxItemSetSizeInBits(); ++i )
            {
                m_itemSet_AsBitSet[ i ] = m_itemSet[ i ];
            }
        }
        
        int
        Concept::getExtentSize()
        {
            return m_extentSize;
        }
        
        stringlist_t&
        Concept::getAddedFormalConceptAttributes( stringlist_t& sl )
        {
            FerrisBitMagic::bvector<> t;

            for( clist_t::const_iterator ci = getParents().begin();
                 ci != getParents().end(); ++ci )
            {
                t |= (*ci)->getItemSet();
            }
            t ^= getItemSet();
            m_cl->AttrIDToStringList( t, sl );
            return sl;
        }

        stringlist_t&
        Concept::getAddedFormalConceptAttributesRelativeToParent( fh_concept pivc,
                                                                  stringlist_t& sl )
        {
            FerrisBitMagic::bvector<> t = pivc->getItemSet();
            t ^= getItemSet();
            m_cl->AttrIDToStringList( t, sl );
            return sl;
        }
        
        
        
        
        void
        Concept::removeParent( const fh_concept& c )
        {
            m_dirty = true;
            erase( m_parents, c );
            erase( m_upset,   c );
        }
        
        void
        Concept::removeChild( const fh_concept& c )
        {
            m_dirty = true;
            erase( m_children, c );
            erase( m_downset,  c );
        }
        
        bool
        Concept::isMeetIrreducible()
        {
            return getParents().size() == 1;
        }

        int
        Concept::getMaxDepth()
        {
            int d = 0;
        
            const clist_t& pl = getParents();
            for( clist_t::const_iterator pi = pl.begin(); pi!=pl.end(); ++pi )
            {
                d = MAX( d, (*pi)->getMaxDepth()+1 );
            }
            return d;
        }
        
        

        bool
        Concept::less_than_or_equal( fh_concept c )
        {
            filters_t tmp;
            filters_t thisbasef = getBaseFilterList();
            filters_t    cbasef = c->getBaseFilterList();

            thisbasef.sort();
            cbasef.sort();
            
            set_intersection( thisbasef.begin(), thisbasef.end(),
                              cbasef.begin(),    cbasef.end(),
                              back_inserter( tmp ) );

            return tmp == cbasef;
        }
        

        
        clist_t&
        Concept::getChildren()
        {
            return m_children;
        }
        clist_t&
        Concept::getChildren( clist_t& l )
        {
            l = _Self::getChildren();
            return l;
        }
        

        void
        Concept::getUpDownSet_recurse( fh_concept p, clist_t& cache, bool isUpSet )
        {
            clist_t& col = isUpSet ? p->m_parents : p->m_children;

            LG_FCA_D << "getUpDownSet_recurse() p:" << p->getFancyName()
                     << " col.sz:" << col.size()
                     << endl;
            
            if( col.empty() )
                return;
                    
            for( clist_t::iterator ci = col.begin(); ci != col.end(); ++ci )
            {
                cache.insert( *ci );
                getUpDownSet_recurse( *ci, cache, isUpSet );
            }
        }
        
        
        clist_t&
        Concept::getUpDownSet( bool includeSelf, bool isUpSet )
        {
            clist_t& cache = isUpSet ? m_upset : m_downset;
            cache.clear();

            if( cache.empty() )
            {
                fh_concept p = this;
                getUpDownSet_recurse( p, cache, isUpSet );
            }
            if( includeSelf )
                cache.insert( this );
            return cache;
        }
        
        clist_t&
        Concept::getUpSet( bool includeSelf )
        {
            return getUpDownSet( includeSelf, true );
        }

        clist_t&
        Concept::getDownSet( bool includeSelf )
        {
            return getUpDownSet( includeSelf, false );
        }

        clist_t&
        Concept::getXSet( bool isUpSet )
        {
            return isUpSet ? getUpSet() : getDownSet();
        }
        
        fh_concept
        Concept::getRootConcept()
        {
            if( !m_parents.empty() )
                return (*m_parents.begin())->getRootConcept();
            return this;
        }
        

        fh_context
        Concept::getExtent()
        {
            return m_extent;
        }


        FerrisBitMagic::bvector<>&
        Concept::getExtentVerticalVector( FerrisBitMagic::bvector<>& ret, bool clarified )
        {
            fh_conceptLattice cl = getConceptLattice();
            
            stringlist_t itemNames;
            cl->AttrIDToStringList( getItemSet(), itemNames );
            const FerrisBitMagic::bvector<>& k = getItemSet();

            string btab = getBaseTableName();
            string vname = btab + "clarified";
            stringstream sqlss;
            string tableName = btab;
            if( clarified )
                tableName = vname;

            if( clarified )
            {
                sqlss << " select urlid,g from " << tableName << " d "
                      << " where " << btab << "_intvec(bitf) @ '{";
            }
            else
            {
                sqlss << " select urlid from " << tableName << " d "
                      << " where " << btab << "_intvec(bitf) @ '{";
            }
            FerrisBitMagic::bvector<>::enumerator an     = k.first();
            FerrisBitMagic::bvector<>::enumerator an_end = k.end();
            int count = 0;
            for ( ; an < an_end ; ++count)
            {
                if( count ) sqlss << ',';
                sqlss << *an;
                ++an;
            }
            sqlss << "}' ";

            
//             sqlss << "select urlid from " <<  btab << " d "
//                   << " where 't' ";
//             for( stringlist_t::const_iterator si = itemNames.begin();
//                  si != itemNames.end(); ++si )
//             {
//                 sqlss << " and " << getBitFunctionName(btab,*si) << "(d.bitf) ";
//             }
// //             {
// //                 stringset_t intent;
// //                 copy( itemNames.begin(), itemNames.end(), inserter( intent, intent.end() ));
// //                 stringlist_t sl;
// //                 cl->getAllAttributeNames( sl );
// //                 for( stringlist_t::const_iterator si = sl.begin(); si!=sl.end(); ++si )
// //                 {
// //                     if( intent.find( *si ) == intent.end() )
// //                     {
// //                         sqlss << " and not " << getBitFunctionName(btab,*si) << "(d.bitf) ";
// //                     }
// //                 }
// //             }
            sqlss << " ;";


            LG_FCA_D << "SQL:" << sqlss.str() << endl;

            ConceptLatticePriv* P = m_cl->getPrivatePart();
            connection& con = *(P->m_connection);
            work trans( con, "getting the concept only extent with all urlids..." );
            result res = trans.exec( sqlss.str() );

            for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
            {
                long urlid = 0;
                iter["urlid"].to( urlid );
                if( clarified )
                    iter["g"].to(urlid);
                ret[ urlid ] = 1;
            }
            
            return ret;
        }
        
        

        extent_bitset_t&
        Concept::getExtentVerticalVectorBitSet( extent_bitset_t& ret, bool clarified )
        {
            fh_conceptLattice cl = getConceptLattice();
            
            stringlist_t itemNames;
            cl->AttrIDToStringList( getItemSet(), itemNames );
            const FerrisBitMagic::bvector<>& k = getItemSet();

            string btab = getBaseTableName();
            string vname = btab + "clarified";
            stringstream sqlss;
            string tableName = btab;
            if( clarified )
                tableName = vname;

            if( clarified )
            {
                sqlss << " select urlid,g from " << tableName << " d "
                      << " where " << btab << "_intvec(bitf) @ '{";
            }
            else
            {
                sqlss << " select urlid from " << tableName << " d "
                      << " where " << btab << "_intvec(bitf) @ '{";
            }
            FerrisBitMagic::bvector<>::enumerator an     = k.first();
            FerrisBitMagic::bvector<>::enumerator an_end = k.end();
            int count = 0;
            for ( ; an < an_end ; ++count)
            {
                if( count ) sqlss << ',';
                sqlss << *an;
                ++an;
            }
            sqlss << "}' ";
            sqlss << " ;";

            LG_FCA_D << "SQL:" << sqlss.str() << endl;

            ConceptLatticePriv* P = m_cl->getPrivatePart();
            connection& con = *(P->m_connection);
            work trans( con, "getting the concept only extent with all urlids..." );
            result res = trans.exec( sqlss.str() );

            for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
            {
                long urlid = 0;
                iter["urlid"].to( urlid );
                if( clarified )
                    iter["g"].to(urlid);
                ret[ urlid ] = 1;
            }
            
            return ret;
        }
        
        
        clist_t
        Concept::getIntent()
        {
            clist_t& u = getUpSet();
            clist_t ret;

            for( clist_t::iterator ci = u.begin(); ci != u.end(); ++ci )
            {
                if( (*ci)->getContingentFilters().size() )
                {
                    ret.insert( *ci );
                }
            }
            if( getContingentFilters().size() )
                ret.insert( this );
                
            
            
            
//             fh_concept rootc = getRootConcept();

//             for( clist_t::iterator ci = u.begin(); ci != u.end(); ++ci )
//             {
//                 if( (*ci)->m_parents.size() == 1 && (*(*ci)->m_parents.begin()) == rootc )
//                 {
//                     ret.insert( *ci );
//                 }
//             }
            
            return ret;
        }

        filters_t&
        Concept::getBaseFilterList()
        {
            return m_baseFilters;
        }
        
        void
        Concept::setBaseFilterList( const filters_t& fl )
        {
            m_dirty = true;
            m_baseFilters.clear();
            copy( fl.begin(), fl.end(), back_inserter( m_baseFilters ));
        }
        
        void
        Concept::setContingentFilters( const filters_t& fl )
        {
            m_dirty = true;
            m_contingentFilters = fl;
        }

        bool
        Concept::containsAttribute()
        {
            return getContingentFilters().size();
        }

        std::list< filters_t >
        Concept::getTransitiveContingentFilters()
        {
            std::list< filters_t > ret;
            clist_t& upset = getUpSet();

            for( clist_t::iterator iter = upset.begin(); iter != upset.end(); ++iter )
            {
                LG_FCA_D << "getTransitiveContingentFilters() upset:"
                         << (*iter)->getFancyName() << endl;
                
                filters_t& fl = (*iter)->getContingentFilters();
                if( !fl.empty() )
                {
                    ret.push_back( fl );
                }
            }
            
            return ret;
        }
        

        filters_t&
        Concept::getContingentFilters()
        {
            return m_contingentFilters;
        }
        
        
        Concept::intentsz_t
        Concept::getIntentSize()
        {
            return getIntent().size();
        }
        


        void
        Concept::setExtent( const fh_context& c )
        {
            m_dirty = true;
            m_extent = c;
        }
        
        
        void
        Concept::makeUniDirectionalLink( fh_concept c, bool isUpLink )
        {
//             cerr << "Concept::makeUniDirectionalLink from:" << getFancyName()
//                  << " to:" << c->getFancyName() << endl;
            m_dirty = true;
            c->m_dirty = true;
            
            if( isUpLink )
            {
                m_parents.insert( c );
                m_upset.insert( c );
            }
            else
            {
                m_children.insert( c );
                m_downset.insert( c );
            }
        }

        void
        Concept::addParent( fh_concept c )
        {
            m_dirty = true;
            makeUniDirectionalLink( c, true );
        }
        void
        Concept::addChild( fh_concept c )
        {
            m_dirty = true;
            makeUniDirectionalLink( c, false );
        }
        
        void
        Concept::addChildren( clist_t& cl )
        {
            m_dirty = true;
            for( clist_t::const_iterator ci = cl.begin(); ci != cl.end(); ++ci )
                addChild( *ci );
        }
        
        

        void
        Concept::makeLink( fh_concept c, bool isUpLink )
        {
            m_dirty = true;
            c->m_dirty = true;
            makeUniDirectionalLink( c, isUpLink );
            c->makeUniDirectionalLink( this, !isUpLink );
        }

        void
        Concept::disconnect()
        {
            m_dirty = true;
//            cerr << "Concept::disconnect() id:" << getID() << endl;
            
            for( clist_t::const_iterator ci = m_children.begin();
                 ci != m_children.end(); ++ci )
            {
                fh_concept c = *ci;
                c->removeParent( this );
            }
            for( clist_t::const_iterator ci = m_parents.begin();
                 ci != m_parents.end(); ++ci )
            {
                fh_concept c = *ci;
                c->removeChild( this );
            }
        }

        void  Concept::setX( float x )  { m_dirty = true; m_x = x; }
        float Concept::getX() { return m_x; }
        void  Concept::setY( float y ) { m_dirty = true; m_y = y; }
        float Concept::getY() { return m_y; }
        void  Concept::setZ( float z ) { m_dirty = true; m_z = z; }
        float Concept::getZ() { return m_z; }
//         void  Concept::setX( float x )  { m_dirty = true; }
//         float Concept::getX() { return 0; }
//         void  Concept::setY( float y ) { m_dirty = true; }
//         float Concept::getY() { return 0; }
//         void  Concept::setZ( float z ) { m_dirty = true; }
//         float Concept::getZ() { return 0; }

        void
        Concept::setLocation( float x, float y, float z )
        {
            m_dirty = true;
            setX( x );
            setY( y );
            setZ( z );
        }
        void
        Concept::setPoint( const Point& p )
        {
            m_dirty = true;
            setLocation( p.getX(), p.getY(), p.getZ() );
        }
        Point
        Concept::getPoint()
        {
            Point ret( getX(), getY(), getZ() );
            return ret;
        }

        void
        Concept::setFilterString( const std::string& s )
        {
            m_dirty = true;
        }
        
        std::string
        Concept::getFilterString()
        {
            return "";
        }
        
        void
        Concept::setFancyName( const std::string& s )
        {
            m_dirty = true;
        }
        
        std::string
        Concept::getFancyName()
        {
            return "";
        }

        
        void
        Concept::updateConceptOnlyMatchSize()
        {
#ifdef HAVE_LIBPQXX
            m_dirty = true;

            fh_conceptLattice cl = m_cl;
            
            stringlist_t itemNames;
            cl->AttrIDToStringList( getItemSet(), itemNames );

            string btab = getBaseTableName();
            stringstream sqlss;
//             sqlss << "select count(*) from docmap, " <<  btab << " d "
//                   << " where docmap.docid = d.docid ";
            sqlss << "select count(*) from " <<  btab << " d "
                  << " where 't' ";
            for( stringlist_t::const_iterator si = itemNames.begin();
                 si != itemNames.end(); ++si )
            {
                sqlss << " and " << getBitFunctionName(btab,*si) << "(d.bitf) ";
            }
            {
                stringset_t intent;
                copy( itemNames.begin(), itemNames.end(), inserter( intent, intent.end() ));
                stringlist_t sl;
                cl->getAllAttributeNames( sl );
                for( stringlist_t::const_iterator si = sl.begin(); si!=sl.end(); ++si )
                {
                    if( intent.find( *si ) == intent.end() )
                    {
                        sqlss << " and not " << getBitFunctionName(btab,*si) << "(d.bitf) ";
                    }
                }
            }
            sqlss << " ;";

//             sqlss << "select * from docmap, " <<  getBaseTableName() << " "
//                   << " where docmap.docid = " << getBaseTableName() << ".docid ";
//             for( stringlist_t::const_iterator si = itemNames.begin();
//                  si != itemNames.end(); ++si )
//             {
//                 sqlss << " and " << getBaseTableName() << ".\"" << *si << "\"='t' ";
//             }
//             {
//                 stringset_t intent;
//                 copy( itemNames.begin(), itemNames.end(), inserter( intent, intent.end() ));
//                 stringlist_t sl;
//                 cl->getAllAttributeNames( sl );
//                 for( stringlist_t::const_iterator si = sl.begin(); si!=sl.end(); ++si )
//                 {
//                     if( intent.find( *si ) == intent.end() )
//                     {
//                         sqlss << " and " << getBaseTableName() << ".\"" << *si << "\"<>'t' ";
//                     }
//                 }
//             }
//             sqlss << " ;";

            
            LG_FCA_D << "SQL:" << sqlss.str() << endl;

            ConceptLatticePriv* P = m_cl->getPrivatePart();
            connection& con = *(P->m_connection);
            work trans( con, "getting the concept only extent..." );
            result res = trans.exec( sqlss.str() );

            LG_FCA_D << "files in updateConceptOnlyMatchSize():" << res.size() << endl;
//            int fileCount = res.size();
            int fileCount = 0;
            res[0][0].to(fileCount);
            m_conceptOnlyMatchSize = fileCount;
#endif
        }
        
        void
        ConceptLattice::refreshAllConceptsContingentCounter()
        {
#ifdef HAVE_LIBPQXX
            m_dirty = true;

//            cerr << "ConceptLattice::refreshAllConceptsContingentCounter(begin)" << endl;

            Time::Benchmark contingentBM("Updating contingent counters");
            
            ConceptLatticePriv* P = getPrivatePart();
            connection& con = *(P->m_connection);
            work trans( con, "refreshAllConceptsContingentCounter..." );

            for( m_IDToConceptMap_t::const_iterator ci = m_IDToConceptMap.begin();
                 ci != m_IDToConceptMap.end(); ++ci )
            {
                ci->second->m_conceptOnlyMatchSize = 0;
                ci->second->m_conceptOnlyMatchingDocIDs.clear();
//                ci->second->m_MatchingDocIDs.clear();
            }

            // Using subq index
            {
                string btab = getBaseTableName();
                Time::Benchmark b("Using subq index");
                
                for( m_IDToConceptMap_t::const_iterator ci = m_IDToConceptMap.begin();
                     ci != m_IDToConceptMap.end(); ++ci )
                {
                    fh_concept c = ci->second;
                    const FerrisBitMagic::bvector<>& k = c->getItemSet();
                    
                    stringstream ss;
                    ss << " select count(*) from " << btab
                       << " where " << btab << "_intvec(bitf) @ '{";
                    FerrisBitMagic::bvector<>::enumerator an     = k.first();
                    FerrisBitMagic::bvector<>::enumerator an_end = k.end();
                    int count = 0;
                    for ( ; an < an_end ; ++count)
                    {
                        if( count ) ss << ',';
                        ss << *an;
                        ++an;
                    }
                    ss << "}' "
                       << " and #" << btab << "_intvec(bitf)=" << count << "; ";

                    LG_FCA_D << "subq SQL:" << ss.str() << endl;
                    result res = trans.exec( ss.str() );
                    int fileCount = 0;
                    res[0][0].to(fileCount);
                    c->m_conceptOnlyMatchSize = fileCount;
                }
            }


//             // Using toscana style
//             {
//                 string btab = getBaseTableName();
//                 Time::Benchmark b("Using subq index");
                
//                 for( m_IDToConceptMap_t::const_iterator ci = m_IDToConceptMap.begin();
//                      ci != m_IDToConceptMap.end(); ++ci )
//                 {
//                     fh_concept c = ci->second;
//                     const FerrisBitMagic::bvector<>& k = c->getItemSet();
                    
//                     stringstream ss;
//                     ss << " select count(*) from " << btab
//                        << " where ";
                    
//                     FerrisBitMagic::bvector<>::enumerator an     = k.first();
//                     FerrisBitMagic::bvector<>::enumerator an_end = k.end();
//                     int count = 0;
//                     for( int i=0; i < (m_itemSetSize-2); ++i )
//                     {
//                         FerrisBitMagic::bvector<> t;
//                         t[i] = true;
//                         std::string attrName = AttrIDToString( t );

//                         if( i ) ss << " and ";
//                         if( !k[i] )
//                             ss << " not( ";

//                         ss << " " << getBitFunctionName( btab, attrName, 1 ) << "(bitf) ";

//                         if( !k[i] )
//                             ss << "  ) ";

//                     }
//                     ss << " ";

//                     LG_FCA_D << "subq SQL:" << ss.str() << endl;
// //                    cerr << "subq SQL:" << ss.str() << endl;
//                     result res = trans.exec( ss.str() );
//                     int fileCount = 0;
//                     res[0][0].to(fileCount);
//                     c->m_conceptOnlyMatchSize = fileCount;
//                 }
//             }
            

                
//             // Using a single linear database scan
//             {
//                 Time::Benchmark b("Using single linear database scan");
//             string btab = getBaseTableName();
//             cerr << "reading table:" << btab << endl;
//             tablereader tr( trans, btab );
//             vector<string> row;
//             int row_count = 1;
//             while( tr >> row )
//             {
//                 if( !(row_count % 5000) )
//                 {
//                     cerr << row_count << " done." << endl;
//                 }
//                 ++row_count;
                
// //                cerr << "row.size:" << row.size() << endl;
//                 string url,urlid,docid;
//                 string bitfstr;
//                 int i=0;

// //                url     = row[i]; ++i;
//                 urlid   = row[i]; ++i;
//                 docid   = row[i]; ++i;
//                 bitfstr = row[i]; ++i;

//                 int docidNum = toint( docid );
//                 FerrisBitMagic::bvector<> bitf;
// //                resize( bitf, m_itemSetSize );

// //                cerr << "bitfstr:" << bitfstr << endl;
//                 {
//                     string::const_iterator si = bitfstr.begin();
//                     for( int i=0; si != bitfstr.end(); ++si, ++i )
//                     {
//                         if( *si == '1' )
//                             bitf[ i ] = 1;
//                     }
//                 }
                
//                 for( m_IDToConceptMap_t::const_iterator ci = m_IDToConceptMap.begin();
//                      ci != m_IDToConceptMap.end(); ++ci )
//                 {
//                     fh_concept c = ci->second;
//                     const FerrisBitMagic::bvector<>& k = c->getItemSet();

// //                    if( equiv( bitf & k, bitf ) )
// //                    if( second_is_superset( bitf, k ) )
// //                    if( equiv_by_iter( k, bitf ) )
//                     if( equiv( bitf, k ) )
//                     {
//                         c->m_conceptOnlyMatchSize++;
//                         c->m_conceptOnlyMatchingDocIDs[ docidNum ] = 1;
//                     }
// //                     if( second_is_superset( k, bitf ) )
// //                     {
// //                         c->m_MatchingDocIDs[ docidNum ] = 1;
// //                     }
//                 }
                
//                 row.clear();
//             }
//             tr.complete();
//             }
            
//            cerr << "ConceptLattice::refreshAllConceptsContingentCounter(end)" << endl;
#endif
        }
        
        
        void
        ConceptLattice::fixInvalidTopLevelConcepts()
        {
//            cerr << "fixInvalidTopLevelConcepts()" << endl;
            
#ifdef HAVE_LIBPQXX
            m_dirty = true;

            fh_concept root = getTopConcept();
            if( !isBound( root ) )
                return;
            
            int TopLevelInvalidesCount = 0;
            string btab = getBaseTableName();
            string ctab = m_CFITableName;
            string bitvec_generic = getBitFunctionName( btab, "intvec", false );
            string bv = bitvec_generic + "(bitf)";
            string bvg = bitvec_generic + "(gtab.bitf)";
            string aggfunc = ctab + "_agg_bit_and";
            
            connection& con = *(P->m_connection);

            try {
                work trans( con, "creating rd-tree index.." );
                stringstream ss;
                ss << "drop index " << ctab << "_bitfidx ; " << endl;
                trans.exec( ss.str() );
                trans.commit();
                }
            catch( exception& e )
            {}
            try {
                work trans( con, "dropping old agg function.." );
                stringstream ss;
                ss << "drop AGGREGATE " << aggfunc << "( bit );";
                trans.exec( ss.str() );
                trans.commit();
            }
            catch( exception& e )
            {}

            
            work trans( con, "fixing lattice structure..." );
            {
                stringstream ss;
                ss << "CREATE INDEX " << ctab << "_bitfidx on " << ctab
                   << " using gist ( " << btab << "_intvec(bitf) gist__int_ops, "
                   << "(#(" << btab << "_intvec(bitf))) gist_int4_ops ); ";
                trans.exec( ss.str() );
                stringstream ss2;
                ss2 << "analyse " << ctab;
                trans.exec( ss2.str() );
            }
            
            
            stringstream ss;
            ss << "SELECT id," << bv << " as bv FROM " << ctab << " gtab "
               << " WHERE lattice_parents = '{" << root->getID() << "}' "
               << " AND EXISTS ( SELECT id from " << ctab << " where id != " << root->getID()
               << " and " << bv << " && " << bvg
               << "	                      and #" << bv << " < #" << bvg << " "
               << "     LIMIT 1 ); " << endl;

//             cerr << "root->getID():" << root->getID() << endl;
//             cerr << "Invalides checking SQL:" << ss.str() << endl;
            result res = trans.exec( ss.str() );

            typedef list< int > invalidesList_t;
            invalidesList_t invalidesList;
            for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
            {
                int id = 0;
                iter["id"].to( id );

                invalidesList.push_back( id );
//                cerr << "Invalid ID:" << id << endl;
                ++TopLevelInvalidesCount;
            }
            
//            cerr << "TopLevelInvalidesCount:" << TopLevelInvalidesCount << endl;

//            cerr << "Deleting invalides nodes sql follows." << endl;
            for( invalidesList_t::const_iterator ci = invalidesList.begin();
                 ci != invalidesList.end(); ++ci )
            {
                int        id = *ci;
                
//                 stringstream ss;
//                 ss << "delete from " << ctab << " where id = " << id << " ; " << endl;
//                 cerr << ss.str();
//                 trans.exec( ss.str() );

                m_IDToConceptMap_t::iterator iter = m_IDToConceptMap.find( id );
                if( m_IDToConceptMap.end() != iter )
                {
                    fh_concept c  = iter->second;
                    c->disconnect();
                    m_IDToConceptMap.erase( iter );
                }
            }

            //
            // Now if the root only has the one child element after cleanup then
            // we can move the roots child into the root and have a proper lattice.
            //
            {
                int count = 0;
                fh_concept cachec = 0;
                
                for( clist_t::const_iterator ci = root->m_children.begin();
                     ci != root->m_children.end(); ++ci )
                {
                    int id = (*ci)->getID();
                    if( invalidesList.end() == find( invalidesList.begin(),
                                                     invalidesList.end(), id ) )
                    {
                        cachec = (*ci);
                        ++count;
                    }
                }
                if( count == 1 )
                {
                    root->setItemSet( root->getItemSet() | cachec->getItemSet() );
                    root->m_children.clear();
                    copy( cachec->m_children.begin(),
                          cachec->m_children.end(),
                          inserter( root->m_children, root->m_children.end() ) );
                    for( clist_t::const_iterator ci = root->m_children.begin();
                         ci != root->m_children.end(); ++ci )
                    {
                        fh_concept child = *ci;
                        child->m_parents.clear();
                        child->m_parents.insert( root );
                    }
//                    cerr << "Removing cachec from database. id:" << cachec->getID() << endl;
                    m_IDToConceptMap.erase( cachec->getID() );
                }
            }
            
            

//             cerr << "Setting up agg function" << endl;
//             {
//                 stringstream ss;
//                 ss << "CREATE AGGREGATE " << aggfunc << "( "
//                    << "    sfunc = bitand,"
//                    << "    basetype = bit(" << m_itemSetSize << "),"
//                    << "    stype = bit(" << m_itemSetSize << ")"
//                    << " );";
//                 trans.exec( ss.str() );
//             }

//             cerr << "moving common items into lattice top node" << endl;
//             {
//                 stringstream ss;
//                 ss << "SELECT " << aggfunc << "( bitf ) as bv from " << ctab
//                    << " where lattice_parents =  '{" << root->getID() << "}' ";
//                 result res = trans.exec( ss.str() );
//                 FerrisBitMagic::bvector<> bv;
//                 if( res.begin() != res.end() )
//                 {
//                     int i=0;
//                     string s;
//                     res[0][0].to(s);
//                     int slen = s.length();
//                     const char* p = s.c_str();
//                     const char* end = p+slen;
//                     for( i=0; p<end; ++p,++i )
//                     {
//                         if( *p == '1' )
//                         {
//                             bv[i] = true;
//                         }
//                     }
//                     cerr << "common bits.count:" << bv.count() << endl;

//                     FerrisBitMagic::bvector<> newbv = root->getItemSet();
//                     newbv |= bv;
//                     root->setItemSet( newbv );
//                 }

//                 for( clist_t::iterator ci = root->getChildren().begin();
//                      ci != root->getChildren().end(); ++ci )
//                 {
//                     fh_concept n = *ci;
//                     n->setItemSet( n->getItemSet() ^ bv );
//                 }
//             }
            
            trans.commit();
            if( TopLevelInvalidesCount > 0 )
                save();
            
//     if( topNodesCount > 1 )
//     {
//         for( clist_t::iterator ni = topNodeChildren.begin(); ni != topNodeChildren.end(); ++ni )
//         {
//             fh_concept n = *ni;
//             const FerrisBitMagic::bvector<>& bv = n->getItemSet();
            
//             if( bv.count() < 2 )
//                 continue;

//             stringsteam ss;
//             ss << " select count(*) from wn1_cfi where wn1_intvec(bitf) && '{10,11,23,38,102,168,237}' "
//                << " and #wn1_intvec(bitf) < #('{10,11,23,38,102,168,237}')::int[]; ";
            
            
// //             for( clist_t::iterator ci = topNodeChildren.begin(); ci != topNodeChildren.end(); ++ci )
// //             {
// //                 fh_concept c = *ci;
// //                 if( c == n )
// //                     continue;

// //                 // The concept 'n' should be reachable from 'c'
// //                 if( second_is_superset( c->getItemSet(), n->getItemSet() ) )
// //                 {
// //                     ++TopLevelInvalidesCount;
// //                 }
// //             }
//         }
//     }
            
            
#endif
        }

        int
        Concept::getConceptOnlyMatchSize()
        {
            return m_conceptOnlyMatchSize;
        }

        std::string
        Concept::getBaseTableName()
        {
            return m_cl->getBaseTableName();
        }

        bool
        Concept::isDirty()
        {
            return m_dirty;
        }
        void
        Concept::setDirty( bool v )
        {
            m_dirty = v;
        }

        bool
        Concept::getFishEyeLabelRec( stringlist_t& sl )
        {
            stringlist_t tmp;
            getAddedFormalConceptAttributes( tmp );
            if( tmp.empty() )
            {
                clist_t intent = getParents();
                for( clist_t::iterator iter = intent.begin(); iter!=intent.end(); ++iter )
                {
                    (*iter)->getFishEyeLabelRec( tmp );
//                     if( !tmp.empty() )
//                         break;
                }
            }
            copy( tmp.begin(), tmp.end(), back_inserter(sl));
            return true;
        }
        
        stringlist_t&
        Concept::getFishEyeLabel( stringlist_t& sl )
        {
            clist_t intent = getParents();
            
            getAddedFormalConceptAttributes( sl );
            if( sl.empty() )
            {
                for( clist_t::iterator iter = intent.begin(); iter!=intent.end(); ++iter )
                {
                    (*iter)->getFishEyeLabelRec( sl );
                }
            }

            m_cl->makeReducedAttributes( sl );
            m_cl->convertAttributeNamesToLabels( sl );
            return sl;
        }
        
        
        fh_conceptLattice
        Concept::getConceptLattice()
        {
            return m_cl;
        }
        
        
        
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        stringset_t& getOutOfBandColumns( stringset_t& ret )
        {
            ret.insert("id");
            ret.insert("support_perc");
            ret.insert("support_abs");
            ret.insert("support_ext");
            ret.insert("lattice_parents");
            ret.insert("lattice_children");
            ret.insert("lattice_added_attrs");
            ret.insert("ferris_top_generator");
            ret.insert("ferris_bottom_generator");
            ret.insert("concept_only_support_abs");
            return ret;
        }

        static stringlist_t PGArrayToStringList( const string& s, stringlist_t& ret )
        {
            string t = s;
            if( !t.empty() )
                t = t.substr( 1, t.length()-2 );
            
            Ferris::Util::parseCommaSeperatedList( t, ret );
            return ret;
        }

#ifdef HAVE_LIBPQXX
        static stringlist_t PGArrayToStringList( const result::field& s, stringlist_t& ret )
        {
            string str;
            s.to(str);
            return PGArrayToStringList( str, ret);
        }
#endif
        
        static fh_concept ConceptFactoryDefault_func( const fh_conceptLattice& cl,
                                                      int maxItemSetSizeInBits )
        {
            fh_concept ret = new Concept( cl, 0 );
            return ret;
        }
        
        // const std::string& findexPath,
        // const std::string& CFITableName
        ConceptLattice::ConceptLattice( fh_context latticeTree,
                                        const F_ConceptFactory_t& func )
            :
            P( new ConceptLatticePriv() ),
            m_latticeTree( latticeTree ),
            m_topConcept(0),
            m_bottomConcept(0),
            m_findexPath( "" ),
            m_CFITableName( "" ),
            m_baseTableName( "" ),
            m_dirty( false )
        {
            setConceptFactory( func );
            m_findexPath   = getStrSubCtx( latticeTree, "findex-path", "" );
            m_CFITableName = getStrSubCtx( latticeTree, "tablename-cfi", "" );
            m_baseTableName = getStrSubCtx( latticeTree, "tablename-base", "" );
            m_CFITableIsAugmented = isTrue(
                getStrSubCtx( latticeTree, "tablename-cfi-is-augmented", "0" ));
            m_CFITableSelectTail = getStrSubCtx( latticeTree, "read-cfi-sql-tail", "" );
            m_itemSetSize = toint(getStrSubCtx( latticeTree, "bit-field-size", "10" ));
            
            LG_FCA_D << "ConceptLattice()"
                     << " latticeTree:" << latticeTree->getURL() << endl
                     << "m_findexPath:" << m_findexPath         << endl
                     << "m_CFITableName:" << m_CFITableName     << endl;

#ifndef HAVE_LIBPQXX
            {
                fh_stringstream ss;
                ss << "Using ConceptLattices requires libpqxx. please rebuild your libferris";
                Throw_GenericError( tostr(ss), 0 );
            }
#else
            
            EAIndex::fh_idx idx = EAIndex::Factory::getEAIndex( m_findexPath );

            stringstream conSS;
            conSS << " host="   << idx->getConfig("cfg-idx-host","",true);
            conSS << " dbname=" << idx->getConfig("cfg-idx-dbname","",true);
            P->m_connection = new connection( conSS.str() );
#endif
        }

        ConceptLatticePriv*
        ConceptLattice::getPrivatePart()
        {
            return P;
        }
        
        ConceptLattice::~ConceptLattice()
        {
            if( P )
                delete P;
        }

        std::string
        ConceptLattice::getBaseTableName()
        {
            return m_baseTableName;
        }
        
        
        fh_concept
        ConceptLattice::getTopConcept()
        {
            return m_topConcept;
        }
        
        fh_concept
        ConceptLattice::getBottomConcept()
        {
            return m_bottomConcept;
        }

        long
        ConceptLattice::getMaxItemSetSizeInBits() const
        {
            return P->m_maxItemSetSizeInBits;
        }
        
        

        list< fh_concept >&
        ConceptLattice::getConcepts( list< fh_concept >& ret ) const
        {
            for( m_IDToConceptMap_t::const_iterator ci = m_IDToConceptMap.begin();
                 ci != m_IDToConceptMap.end(); ++ci )
            {
                ret.push_back( ci->second );
            }

            return ret;
        }
        
        

        stringlist_t&
        ConceptLattice::getAllAttributeNames( stringlist_t& ret ) const
        {
            for( m_attrIDToString_t::const_iterator ci = m_attrIDToString.begin();
                 ci != m_attrIDToString.end(); ++ci )
            {
                ret.push_back( ci->second );
            }

            return ret;
        }

        fh_context
        ConceptLattice::getAllObjects()
        {
            ConceptLatticePriv* P = getPrivatePart();
            connection& con = *(P->m_connection);
            work trans( con, "getAllObjects()..." );

            string btab = getBaseTableName();
            stringstream ss;
            ss << " select url from " << btab << " btab, urlmap where urlmap.urlid = btab.urlid ";

            fh_context selfactory = Resolve( "selectionfactory://" );
            fh_context selection  = selfactory->createSubContext( "" );
            
            result res = trans.exec( ss.str() );
            for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
            {
                string earl;
                iter["url"].to( earl );
                LG_FCA_D << "getAllObjects() earl:" << earl << endl;
                fh_context c = Resolve( earl );
                selection->createSubContext( "", c );
            }

            return selection;
        }
        

        VerticalFormalContext_t&
        ConceptLattice::getVerticalFormalContext(
            FerrisBitMagic::bvector<>& Gret,
            VerticalFormalContext_t& ret,
            bool clarify )
        {
            ConceptLatticePriv* P = getPrivatePart();
            connection& con = *(P->m_connection);
            string btab = getBaseTableName();
            string vname = btab + "clarified";

            if( clarify )
            {
                bool alread_clarified = false;
                try
                {
                    work trans( con, "getFormalContext()..." );
                    stringstream ss;
                    ss << "select * from " << vname << " limit 1";
                    result res = trans.exec( ss.str() );
                    if( !res.empty() )
                    {
                        alread_clarified = true;
                    }
                }
                catch( ... )
                {}

//                cerr << "alread_clarified:" << alread_clarified << endl;
                if( !alread_clarified )
                {
                    try
                    {
                        LG_FCA_D << "trying to make clarified table..." << endl;
                        work trans( con, "getFormalContext()..." );

                        LG_FCA_D << "maybe...Creating vname:" << vname << endl;
                        {
                            stringstream ss;
                            ss << "CREATE SEQUENCE " << vname << "_seq START 1;" << endl;
                            LG_FCA_D << "sql:" << ss.str() << endl;
                            result res = trans.exec( ss.str() );
                        }
                        {
                            stringstream ss;
                            ss << "select docid,urlid,bitf,nextval('" << vname << "_seq') as g " << endl
                               << " into " << vname << endl
                               << " from " << btab << " where urlid in " << endl
                               << " (select max(urlid) from " << btab << " group by bitf);" << endl;
                            LG_FCA_D << "sql:" << ss.str() << endl;
                            result res = trans.exec( ss.str() );
                        }
                        {
                            stringstream ss;
                            ss << "create index " << vname << "rd on "
                               << vname << " using gist( "
                               << btab << "_intvec(bitf::bit varying) );" << endl;
                            LG_FCA_D << "sql:" << ss.str() << endl;
                            result res = trans.exec( ss.str() );
                        }
//                     {
//                         stringstream ss;
//                         ss << "create view " << vname << " "
//                            << " as ( select docid,mtime,urlid,bitf from " << btab << endl
//                            << "      where urlid in " << endl
//                            << "        ( select max(urlid) from " << btab << " group by bitf) );" << endl;
//                         result res = trans.exec( ss.str() );
//                     }
//                     {
//                         stringstream ss;
//                         ss << "CREATE SEQUENCE " << vname << "gmap_seq START 1;" << endl;
//                         result res = trans.exec( ss.str() );
//                     }
//                     {
//                         stringstream ss;
//                         ss << "create table " << vname << "gmap ( " << endl
//                            << " urlid int primary key, " << endl
//                            << " g int DEFAULT nextval('" << vname << "gmap_seq') );" << endl;
//                         result res = trans.exec( ss.str() );
//                     }
//                     {
//                         stringstream ss;
//                         ss << "insert into " << vname << "gmap "
//                            << " ( select urlid from " << vname << " );" << endl;
//                         result res = trans.exec( ss.str() );
//                     }
                        trans.commit();
                    }
                    catch( ... )
                    {}
                }
            }
            work trans( con, "getFormalContext()..." );

            FerrisBitMagic::bvector<> M = getFullItemSet();
            
            stringstream sqlss;
            if( clarify )
            {
//                 sqlss << "select bitf,max(urlid) as urlid "
//                       << " from " <<  btab << " d group by bitf ";
                sqlss << "select  bitf,d.urlid,g "
                      << " from " <<  vname << " d " << endl
                      << " order by d.urlid ";
            }
            else
            {
                sqlss << "select  bitf,urlid "
                      << " from " <<  btab << " d order by urlid ";
            }
            LG_FCA_D << "SQL:" << sqlss.str() << endl;
            result res = trans.exec( sqlss.str() );
            result::const_iterator iter = res.begin();
            for( int g = 1; iter != res.end(); ++iter, ++g)
            {
                long urlid;
                string bitf;
                iter["urlid"].to( urlid );
                iter["bitf"].to( bitf );

                if( clarify )
                    iter["g"].to(g);
                
                Gret[ urlid ] = 1;
                stringstream ss;
                ss << bitf;
                char ch;
                for( long i=0; ss >> ch; ++i )
                {
                    if( ch=='1' || ch=='t' )
                    {
                        ret[i][g] = 1;
                    }
                }
            }
            
            return ret;
        }


        VerticalFormalContextBitSet_t&
        ConceptLattice::getVerticalFormalContextBitSet(
            extent_bitset_t& Gret,
            VerticalFormalContextBitSet_t& ret,
            bool clarify )
        {
            ConceptLatticePriv* P = getPrivatePart();
            connection& con = *(P->m_connection);
            string btab = getBaseTableName();
            string vname = btab + "clarified";

            if( clarify )
            {
                try
                {
                    LG_FCA_D << "trying to make clarified table..." << endl;
                    work trans( con, "getFormalContext()..." );

                    LG_FCA_D << "maybe...Creating vname:" << vname << endl;
                    {
                        stringstream ss;
                        ss << "CREATE SEQUENCE " << vname << "_seq START 1;" << endl;
                        LG_FCA_D << "sql:" << ss.str() << endl;
                        result res = trans.exec( ss.str() );
                    }
                    {
                        stringstream ss;
                        ss << "select docid,urlid,bitf,nextval('" << vname << "_seq') as g " << endl
                           << " into " << vname << endl
                           << " from " << btab << " where urlid in " << endl
                           << " (select max(urlid) from " << btab << " group by bitf);" << endl;
                        LG_FCA_D << "sql:" << ss.str() << endl;
                        result res = trans.exec( ss.str() );
                    }
                    {
                        stringstream ss;
                        ss << "create index " << vname << "rd on "
                           << vname << " using gist( "
                           << btab << "_intvec(bitf::bit varying) );" << endl;
                        LG_FCA_D << "sql:" << ss.str() << endl;
                        result res = trans.exec( ss.str() );
                    }
                    trans.commit();
                }
                catch( ... )
                {}
            }
            work trans( con, "getFormalContext()..." );

            FerrisBitMagic::bvector<> M = getFullItemSet();
            
            stringstream sqlss;
            if( clarify )
            {
//                 sqlss << "select bitf,max(urlid) as urlid "
//                       << " from " <<  btab << " d group by bitf ";
                sqlss << "select  bitf,d.urlid,g "
                      << " from " <<  vname << " d " << endl
                      << " order by d.urlid ";
            }
            else
            {
                sqlss << "select  bitf,urlid "
                      << " from " <<  btab << " d order by urlid ";
            }
            LG_FCA_D << "SQL:" << sqlss.str() << endl;
            result res = trans.exec( sqlss.str() );
            result::const_iterator iter = res.begin();
            for( int g = 1; iter != res.end(); ++iter, ++g)
            {
                long urlid;
                string bitf;
                iter["urlid"].to( urlid );
                iter["bitf"].to( bitf );

                if( clarify )
                    iter["g"].to(g);
                
                Gret[ urlid ] = 1;
                stringstream ss;
                ss << bitf;
                char ch;
                for( long i=0; ss >> ch; ++i )
                {
                    if( ch=='1' || ch=='t' )
                    {
                        ret[i][g] = 1;
                    }
                }
            }
            
            return ret;
        }
        

        static fca_std_bitset_t& bitfstring_to_intent( fca_std_bitset_t& ret, const std::string& bitf )
        {
//            cerr << "bitfstring_to_intent() bitf:" << bitf << endl;
            stringstream ss;
            ss << bitf;
            char ch;
            for( long i=0; ss >> ch; ++i )
            {
                if( ch=='1' || ch=='t' )
                {
                    ret[i] = 1;
                }
            }
            return ret;
        }
        
        
//        typedef bidirectional_map< fh_concept, FerrisBitMagic::bvector<>* > AllExtentVerticalVectors_t;
        ConceptLattice::AllExtentVerticalVectors_t&
        ConceptLattice::getAllExtentVerticalVectors( AllExtentVerticalVectors_t& ret,
                                                     bool clarified )
        {
            string btab = getBaseTableName();
            string vname = btab + "clarified";
            stringstream sqlss;
            string tableName = btab;
            if( clarified )
                tableName = vname;

            typedef map< fca_std_bitset_t, fh_concept > IntentToConcept_t;
            IntentToConcept_t IntentToConcept;
            for( m_IDToConceptMap_t::const_iterator ci = m_IDToConceptMap.begin();
                 ci != m_IDToConceptMap.end(); ++ci )
            {
                fh_concept c = ci->second;
                const fca_std_bitset_t& bv = c->getItemSetBitSet();
                IntentToConcept[ bv ] = c;

                AllExtentVerticalVectors_t::iterator it=get<from>(ret).find( c );
                if( it == ret.end() )
                {
                    FerrisBitMagic::bvector<>* bv = new FerrisBitMagic::bvector<>( FerrisBitMagic::BM_BIT,
                                                                                   FerrisBitMagic::gap_len_table_min<true>::_len,
                                                                                   getMaxItemSetSizeInBits() );
                    ret.insert( AllExtentVerticalVectors_t::value_type( c, bv ));
                }
            }

            
            sqlss << "select urlid,bitf from " << tableName << ";" << endl;
            LG_FCA_D << "SQL:" << sqlss.str() << endl;

            ConceptLatticePriv* P = getPrivatePart();
            connection& con = *(P->m_connection);
            work trans( con, "getting all concept only extents with all urlids..." );
            result res = trans.exec( sqlss.str() );

            for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
            {
                long urlid = 0;
                iter["urlid"].to( urlid );
                if( clarified )
                    iter["g"].to(urlid);

                string bitf;
                iter["bitf"].to( bitf );

                fca_std_bitset_t intent( getMaxItemSetSizeInBits() );
                bitfstring_to_intent( intent, bitf );

                for( IntentToConcept_t::const_iterator ii = IntentToConcept.begin();
                     ii != IntentToConcept.end(); ++ii )
                {
                    fh_concept concept = ii->second;
                    if( second_is_superset( ii->first, intent ) )
                    {
                        AllExtentVerticalVectors_t::iterator it=get<from>(ret).find( concept );
                        FerrisBitMagic::bvector<>* bv = it->second;
                        (*bv)[ urlid ] = 1;
                    }
                }
            }

            return ret;
        }
        
        
        
        stringlist_t
        ConceptLattice::AttrIDToStringList( const FerrisBitMagic::bvector<>& x, stringlist_t& ret )
        {
            for( int i=0; i<m_itemSetSize; ++i )
            {
                if( !x[ i ] )
                    continue;
                
                FerrisBitMagic::bvector<> t;
                t[ i ] = x[ i ];

                m_attrIDToString_t::iterator iter = m_attrIDToString.find( t );
                if( iter != m_attrIDToString.end() )
                {
                    ret.push_back( iter->second );
                }
            }
            return ret;
        }
        

        
        string
        ConceptLattice::AttrIDToString( const FerrisBitMagic::bvector<>& x )
        {
            if( m_attrIDToString.find( x ) != m_attrIDToString.end() )
                return m_attrIDToString[ x ];

            return "N/A";
        }

        FerrisBitMagic::bvector<>
        ConceptLattice::StringToAttrID( const std::string& s )
        {
            FerrisBitMagic::bvector<> ret;
//            resize( ret, m_itemSetSize );

            for( m_attrIDToString_t::const_iterator ci = m_attrIDToString.begin();
                 ci != m_attrIDToString.end(); ++ci )
            {
                if( ci->second == s )
                    return ci->first;
            }
            
            return ret;
        }
        
        

        void
        ConceptLattice::setAttrIDToString( const FerrisBitMagic::bvector<>& x, string v )
        {
            m_attrIDToString[ x ] = v;
        }

        stringlist_t
        ConceptLattice::getAddedAttributes( fh_concept parent,
                                            fh_concept child )
        {
//             const bit_vector bv_parent = parent->getItemSet();
//             const bit_vector bv_child  = child->getItemSet();
//             bit_vector bv_newAttrs = bv_child;
//             bv_newAttrs ^= bv_parent;

//             cerr << "getAddedAttributes() " << endl;
//             cerr << " parent:" << bv_parent << endl;
//             cerr << " child :" << bv_child  << endl;
//             cerr << " new   :" << bv_newAttrs << endl;

            FerrisBitMagic::bvector<> bv_newAttrs = child->getItemSet() ^ parent->getItemSet();
            
            stringlist_t sl;
            AttrIDToStringList( bv_newAttrs, sl );
            return sl;
        }


        stringmap_t&
        makeAttributeToFFilterMap( fh_context latticeTree,
                                   std::map< FerrisBitMagic::bvector<>, std::string >& attrIDToString,
                                   stringmap_t& out )
        {
            fh_context ffc = latticeTree->getSubContext( "ffilters" );

//             for( stringlist_t::iterator si=sl.begin(); si!=sl.end(); ++si )
//             {
//                 string t = getStrSubCtx( ffc, *si, "", true, true );
//                 out[ *si ] = t;
//             }

            typedef std::map< FerrisBitMagic::bvector<>, std::string > COL;
            COL::iterator end = attrIDToString.end();
            for( COL::iterator iter = attrIDToString.begin(); iter != end; ++iter )
            {
                string a = iter->second;
                string t = getStrSubCtx( ffc, a, "", true, true );
                out[ a ] = t;
            }
            
            return out;
        }

        stringmap_t&
        ConceptLattice::getAttributeToFFilterMap( stringmap_t& out )
        {
            return makeAttributeToFFilterMap( m_latticeTree, m_attrIDToString, out );
        }
        
        
        
        stringlist_t&
        ConceptLattice::makeReducedAttributes( stringlist_t& sl )
        {
            sl.sort();
            stringlist_t::iterator e = unique( sl.begin(), sl.end() );
            sl.erase( e, sl.end() );

            stringset_t slset;
            stringmap_t atof;
            typedef multimap< string, string > aprefixtof_t;
            aprefixtof_t aprefixtof;
            stringmap_t aprefixtof_opcode;
            stringmap_t ftoa;

            makeAttributeToFFilterMap( m_latticeTree, m_attrIDToString, atof );
            boost::regex subqregex("^\\(([^<>=]+)((?:>=|<=))([0-9.]+)\\)$");
            copy( sl.begin(), sl.end(), inserter( slset, slset.end() ));
//            cerr << "START---SZ:" << slset.size() << endl;
//            cerr << "START---V:" << Util::createCommaSeperatedList(slset) << endl;

            for( stringlist_t::iterator si = sl.begin(); si!=sl.end(); ++si )
            {
                string a = *si;
                string f = atof[a];
                ftoa[ f ] = a;

                if( count( f.begin(), f.end(), '(' ) == 1 )
                {
                    boost::smatch matches;
                    if(boost::regex_match( f, matches, subqregex ))
                    {
//                        cerr << "m.sz:" << matches.size() << endl;
                        if( matches.size() == 4 )
                        {
                            string opcode = matches[2];
//                            cerr << "opcode:" << opcode << endl;

                            string apre = a.substr( 0, a.find( '_' ) );
                            aprefixtof.insert( make_pair( apre, f ));
//                            cerr << "a:" << a << " apre:" << apre << endl;

                            if( !aprefixtof_opcode.count( apre ) )
                            {
                                aprefixtof_opcode[apre] = opcode;
                            }
                            else
                            {
                                if( aprefixtof_opcode[apre] != opcode )
                                {
                                    aprefixtof_opcode[apre] = "Invalid";
                                    aprefixtof.erase( apre );
                                }
                            }
                        }
                    }
                }
            }

            if( !aprefixtof.empty() )
            {
                typedef aprefixtof_t::iterator iterator;
                iterator nextk = aprefixtof.begin();
                iterator end = aprefixtof.end();
                
                while( nextk != end )
                {
                    string k = nextk->first;
                    pair< iterator, iterator > er = aprefixtof.equal_range( k );
                    iterator first  = er.first;
                    iterator second = er.second;
                    nextk  = second;
                    if( nextk != end )
                        ++nextk;

                    if( distance( first, second ) == 1 )
                        continue;

                    string opcode = aprefixtof_opcode[first->first];

                    typedef map< long, string > ffilterValueToFilter_t;
                    ffilterValueToFilter_t ffilterValueToFilter;

//                    cerr << "ER first  k:" << first->first << " v:" << first->second << endl;
                    for( iterator iter = first; iter!=second; ++iter )
                    {
                        string f = iter->second;
//                        cerr << "ER iter  k:" << iter->first << " v:" << iter->second << endl;
                        boost::smatch matches;
                        if(boost::regex_match( f, matches, subqregex ))
                        {
//                            cerr << "m.sz:" << matches.size() << endl;
                            if( matches.size() == 4 )
                            {
                                long v = toint(matches[3]);
//                                cerr << "v:" << v << endl;
                                ffilterValueToFilter[ v ] = f;
                            }
                        }
                    }

                    typedef ffilterValueToFilter_t::iterator FI;
                    FI ff = ffilterValueToFilter.begin();
                    FI fl = ffilterValueToFilter.end();
                    --fl;

//                     cerr << "First value:" << ff->first << endl;
//                     cerr << "Last value:" << fl->first << endl;
//                     cerr << "opcode:" << opcode << endl;

                    for( iterator iter = first; iter!=second; ++iter )
                    {
                        string a = iter->first;
                        string f = iter->second;
                        string k = ftoa[f];
                        slset.erase( k );
//                         cerr << "ERASE a:" << a << endl;
//                         cerr << "ERASE f:" << f << endl;
//                         cerr << "ERASE k:" << k << endl;
                    }
                    
                    if( opcode == "<=" )
                    {
                        string f = ff->second;
                        slset.insert( ftoa[ f ] );
//                        cerr << "ADD a:" << ftoa[ f ] << endl;
                    }
                    else if( opcode == ">=" )
                    {
                        string f = fl->second;
                        slset.insert( ftoa[ f ] );
                    }
                }
            }

//             cerr << "END---SZ:" << slset.size() << endl;
//             cerr << "END---V:" << Util::createCommaSeperatedList(slset) << endl;
            stringlist_t ret;
            copy( slset.begin(), slset.end(), back_inserter(ret));

            sl = ret;
            return sl;
        }

        string
        ConceptLattice::getReasonableTimeFormatStringForAttribute( stringmap_t& atof,
                                                                   const std::string& wanted_eaname )
        {
            time_t mintt = 0;
            time_t maxtt = 0;
            static boost::regex subqregex("^\\(([^<>=]+)((?:>=|<=))([0-9.]+)\\)$");

            LG_FCA_D << "getReasonableTimeFormatStringForAttribute() EA:" << wanted_eaname
                     << " atof.sz:" << atof.size()
                     << endl;
            
            for( stringmap_t::iterator iter = atof.begin(); iter!=atof.end(); ++iter )
            {
                string f = iter->second;

                LG_FCA_D << "getReasonableTimeFormatStringForAttribute() EA:" << wanted_eaname
                         << " f:" << f
                         << endl;
                
                boost::smatch matches;
                if(boost::regex_match( f, matches, subqregex ))
                {
                    if( matches.size() == 4 )
                    {
                        string eaname = matches[1];
                        string opcode = matches[2];
                        string v = matches[3];
                        if( eaname == wanted_eaname )
                        {
                            time_t tt = toint( v );
                            
                            LG_FCA_D << "getReasonableTimeFormatStringForAttribute() EA:" << wanted_eaname
                                     << " min:" << mintt
                                     << " max:" << maxtt
                                     << " tt:" << tt
                                     << endl;
                            
                            if( !mintt )
                                mintt = tt;
                            else if( mintt > tt )
                                mintt = tt;

                            if( !maxtt )
                                maxtt = tt;
                            else if( maxtt < tt )
                                maxtt = tt;
                        }
                    }
                }
            }

            LG_FCA_D << "getReasonableTimeFormatStringForAttribute(...) EA:" << wanted_eaname
                     << " atof.sz:" << atof.size()
                     << endl;
            
            time_t tdiff = maxtt - mintt;
            LG_FCA_D << "getReasonableTimeFormatStringForAttribute() EA:" << wanted_eaname
                     << " min:" << mintt
                     << " max:" << maxtt
                     << " diff:" << tdiff
                     << endl;

            if( tdiff > (3600L * 24 * 31 * 9 ))
            {
                return "%d%b%g";
            }
            if( tdiff > (3600L * 24 * 31 ))
            {
                return "%d%b";
            }
            if( tdiff > (3600L * 24 ))
            {
                return "%R_%d%b";
            }

            return "";
        }
    
        stringlist_t&
        ConceptLattice::convertAttributeNamesToLabels( stringlist_t& sl )
        {
            stringset_t slset;
            stringmap_t atof;
            makeAttributeToFFilterMap( m_latticeTree, m_attrIDToString, atof );
            copy( sl.begin(), sl.end(), inserter( slset, slset.end() ));
            static boost::regex subqregex("^\\(([^<>=]+)((?:>=|<=))([0-9.]+)\\)$");

            for( stringlist_t::iterator si = sl.begin(); si!=sl.end(); ++si )
            {
                string a = *si;
                string f = atof[a];

//                cerr << "convertAttributeNamesToLabels() a:" << a << " f:" << f << endl;
                boost::smatch matches;
                if(boost::regex_match( f, matches, subqregex ))
                {
//                    cerr << "b...m.sz:" << matches.size() << endl;
                    if( matches.size() == 4 )
                    {
                        string eaname = matches[1];
                        string opcode = matches[2];
                        string v = matches[3];

//                         cerr << "b...eaname:" << eaname << endl;
//                         cerr << "b...opcode:" << opcode << endl;

                        if( ends_with( eaname, "time" ) )
                        {
                            static boost::regex numrex("^[0-9]+$");
                            if( !boost::regex_match( f, numrex ))
                                continue;
                            
                            string formatstr = getReasonableTimeFormatStringForAttribute( atof, eaname );
                            time_t tt = toint( v );
                            std::string tstr = Time::toTimeString( tt, formatstr );
//                            cerr << "b...tstr:" << tstr << endl;

                            slset.erase( a );
                            stringstream ss;
                            ss << eaname << "" << opcode << "" << tstr;
                            slset.insert( ss.str() );
                        }
                        else
                        {
                            slset.erase( a );
                            stringstream ss;
                            ss << eaname << "" << opcode << "" << v;
                            slset.insert( ss.str() );
                        }
                    }
                }
            }
            
            
            sl.clear();
            copy( slset.begin(), slset.end(), back_inserter(sl));
            return sl;
        }
        

        FerrisBitMagic::bvector<>
        ConceptLattice::getFullItemSet()
        {
            FerrisBitMagic::bvector<> bv;
            int sz = m_attrIDToString.size();
            
            for( int i=0; i<sz; ++i )
                bv[i] = true;

//            cerr << "ConceptLattice::getFullItemSet(1)" << endl;
//            cerr << bv << endl;
//            cerr << "ConceptLattice::getFullItemSet(2)" << endl;
            
            return bv;
        }

        fca_std_bitset_t
        ConceptLattice::getFullItemSetBitSet()
        {
            fca_std_bitset_t bv;
            int sz = m_attrIDToString.size();
            
            for( int i=0; i<sz; ++i )
                bv[i] = true;

            return bv;
        }
        
        

        int
        ConceptLattice::getItemSetSize()
        {
            return m_itemSetSize;
        }
        
        

        int
        ConceptLattice::getFormalAttributeCount()
        {
            return m_attrIDToString.size();
        }
        
        void
        ConceptLattice::priv_load_invertedlist( fh_context c, int id,
                                                FerrisBitMagic::bvector<>& bv )
        {
//             fh_context subc = Shell::acquireSubContext( c, tostr(id), false );
//             string datastr = getStrAttr( subc, "content", "", true );
//             if( !datastr.empty() )
//             {
//                 cerr << "id:" << id << " datastr.len:" << datastr.length() << endl;
// //                FerrisBitMagic::bvector<>  bv;
//                 FerrisBitMagic::deserialize(bv, (unsigned char*)datastr.data());
//                 cerr << "     bv.count:" << bv.count() << endl;
// //                n->m_conceptOnlyMatchingDocIDs = bv;
//             }
        }
        
        
        void
        ConceptLattice::priv_load()
        {
#ifdef HAVE_LIBPQXX

            
            string CFISelectSQL;
            {
                stringstream ss;
                ss << " select id,support_perc,support_abs,support_ext";
                if( m_CFITableIsAugmented )
                {
                    ss << ",lattice_parents,lattice_children"
                       << ",lattice_added_attrs,concept_only_support_abs";
                }
                ss << ",bitf from  " << m_CFITableName << " d  order by id desc";
//                ss << m_CFITableSelectTail;
                CFISelectSQL = ss.str();
                
//                 stringstream sqlss;
//                 sqlss << "select * from " << m_CFITableName << " order by id desc ";
//                 CFISelectSQL = sqlss.str();
            }
            
            LG_FCA_D << "ConceptLattice::priv_load(SQL):" << CFISelectSQL << endl;
            work trans( *P->m_connection, "getting the data..." );
//            cerr << "getting the data...SQL:" << CFISelectSQL << endl;
            Time::Benchmark bm("query");
            bm.start();
            result res = trans.exec( CFISelectSQL );

//             icursorstream cur( trans, CFISelectSQL, "load_lattice", cursor_base::all() );
// //            icursorstream cur( trans, CFISelectSQL, "load_lattice", 500 );
            
//             cerr << "getting the data...B" << endl;
//             result res;
//             if (!(cur >> res))
//             {
//                 cerr << "ERROR loading data. nothing to load..." << endl;
//                 return;
//             }
//            cerr << "getting the data...C" << endl;
            bm.stop();
            bm.print();

            LG_FCA_D << "ConceptLattice::priv_load(top)" << endl;
            
            bool parentsColumnExists = false;
            bool childrenColumnExists = false;
            bool addedAttrColumnExists = false;
            bool conceptOnlyMatchSizeExists = false;
                
            stringset_t outOfBandColumns;
            getOutOfBandColumns( outOfBandColumns );
            
//             {
// //                 string t;
// //                 (*res.begin())["bitf"].to(t);
// //                 m_itemSetSize = t.length();
//                 LG_FCA_D << "ConceptLattice::priv_load(res.begin.sz):" << res.begin()->size() << endl;

//                 m_itemSetSize = res.begin()->size();
//                 m_itemSetSize -= 4;
//                 if( m_CFITableIsAugmented )
//                     m_itemSetSize -= 4;
//             }
            
            LG_FCA_D << "priv_load() m_itemSetSize:" << m_itemSetSize << endl;
            
            // 
            // make a cache of which string names each bit vector
            // position has
            //
            {
                int colNum = 0;
                result::const_iterator firsttuple     = res.begin();
                result::tuple::const_iterator e       = firsttuple->end();
                for( result::tuple::const_iterator cur = firsttuple->begin();
                     cur != e; ++cur )
                {
                    string name = cur->name();

//                     if( outOfBandColumns.find( name ) == outOfBandColumns.end() )
//                     {
//                         FerrisBitMagic::bvector<> bv;
//                         bv[ colNum ] = 1;
//                         setAttrIDToString( bv, name );
                        
//                         ++colNum;
//                     }
                    if( name == "lattice_parents" )  parentsColumnExists = true;
                    if( name == "lattice_children" ) childrenColumnExists = true;
                    if( name == "lattice_added_attrs" ) addedAttrColumnExists = true;
                    if( name == "concept_only_support_abs" ) conceptOnlyMatchSizeExists = true;
                }
            }

            int maxItemSetSizeInBits = 64;
            {
                stringstream ss;
                ss << getStrSubCtx( m_latticeTree, "bitf-column-names", "", true, true );
                string name;
                int colNum = 0;
                while( getline( ss, name ))
                {
                    FerrisBitMagic::bvector<> bv;
                    bv[ colNum ] = 1;
                    setAttrIDToString( bv, name );
                    ++colNum;
                }
                maxItemSetSizeInBits = colNum;
            }
//            cerr << " maxItemSetSizeInBits:" << maxItemSetSizeInBits << endl;
            P->m_maxItemSetSizeInBits = maxItemSetSizeInBits;
                
            // First we read the concepts and make them,
            // later we conncet parent/child associations.
//            cerr << "START Reading tuple results..." << endl;
            Time::Benchmark bm_read("Reading tuple results...");
            bm_read.start();
//             while( true )
//             {
//                 {
//                     static int i =0;
//                     ++i;
//                     cerr << "Reading tuple " << i << endl;
//                 }
                
                for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
                {
                    int id = 0;
                    string addedAttrs = "";

                    // FIXME: ego will want to make a subclass of Concept here
//                fh_concept ivc  = new Concept( this );
                    fh_concept ivc = m_conceptFactory_func( this, maxItemSetSizeInBits );
                
                    int colNum = 0;
                    result::tuple::const_iterator e = iter->end();
                    for( result::tuple::const_iterator cur = iter->begin();
                         cur != e; ++cur )
                    {
                        string name = cur->name();

//                    cerr << "id:" << id << " name:" << name << endl;
                        if( name == "id" )
                        {
                            cur->to( id );
                            if( (id % 1000) == 0 )
                                cerr << "id...:" << id << endl;
                        }
                        else if( name == "support_perc" )
                        {
                            cur->to( ivc->m_support_perc );
                        }
                        else if( name == "support_abs" )
                        {
                            cur->to( ivc->m_support_abs );
                        }
                        else if( name == "support_ext" )
                        {
                            cur->to( ivc->m_support_ext );
                        }
                        else if( name == "concept_only_support_abs" )
                        {
                            cur->to( ivc->m_conceptOnlyMatchSize );
                        }
                        else if( name == "lattice_added_attrs" )
                        {
                            string s;
                            cur->to( addedAttrs );
                            if( !addedAttrs.empty() )
                                addedAttrs = addedAttrs.substr( 1, addedAttrs.length() - 2 );
                        }
                        else if( outOfBandColumns.find( name ) != outOfBandColumns.end() )
                        {
                        }
                        else if( name == "bitf" )
                        {
                            string s;
                            cur->to(s);
//                            cerr << "id:" << id << " bitf:" << s << endl;
                            int slen = s.length();
                            const char* p = s.c_str();
                            const char* end = p+slen;
                            for( ; p<end; ++p )
                            {
                                if( *p == '1' )
                                {
                                    ivc->m_itemSet[ colNum ] = true;
                                    ivc->m_itemSet_AsBitSet[ colNum ] = true;
                                    ivc->m_itemSetWeight++;
                                }
                                ++colNum;
                            }
//                            ivc->m_itemSet.optimize();
                        }
                        else
                        {
                            bool v = 0;
                            cur->to( v );

                            LG_FCA_D << "priv_load() id:" << id << " name:" << name
                                     << " colNum:" << colNum << " v:" << v << endl;
                        
                            if( v )
                            {
                                ivc->m_itemSet[ colNum ] = true;
                                ivc->m_itemSetWeight++;
                            }
                        
                            ++colNum;
                        }
                    }

//                 {
//                     stringstream namess;
//                     namess << id;
//                     if( addedAttrs != "{}" )
//                         namess << " - " << addedAttrs;
//                     ivc->name = namess.str().c_str();
//                 }

//                 cerr << "priv_load() id:" << id
//                      << " m_itemSetWeight:" << ivc->m_itemSetWeight
//                      << " itemSet:" << ivc->m_itemSet << endl;

//                cerr << "ConceptLattice::priv_load(made concept) id:" << id << endl;
                
                    ivc->m_id = id;
                    ivc->m_extentSize = ivc->m_support_abs;


//                     stringlist_t addedAttrsSL;
//                     Util::parseCommaSeperatedList( addedAttrs, addedAttrsSL );
//                     for( stringlist_t::const_iterator si = addedAttrsSL.begin();
//                          si != addedAttrsSL.end(); ++si )
//                     {
//                         string attr = *si;
//                         FerrisBitMagic::bvector<> v = StringToAttrID( attr );
//                         ivc->m_addedAttrs.push_back( v );
//                     }
                
                    m_IDToConceptMap.insert( make_pair( id, ivc ) );
                }
//                 res.clear();
//                 if (!(cur >> res))
//                     break;
//            }
            
            bm_read.stop();
            bm_read.print();

            //
            // Now that we have made all the concepts we can freely link them
            // to each other.
            //
            LG_FCA_D << "parentsColumnExists:" << parentsColumnExists
                     << "  childrenColumnExists:" << childrenColumnExists
                     << endl;
            if( parentsColumnExists && childrenColumnExists )
            {
                for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
                {
                    int id = -1;
                    result::field idf       = (*iter)[ "id"  ];
                    result::field parentsf  = (*iter)[ "lattice_parents"  ];
                    result::field childrenf = (*iter)[ "lattice_children" ];
                    idf.to( id );
        
                    LG_FCA_D << " id:" << id << " parents:" << parentsf.c_str() << endl;

                    stringlist_t parents;
                    stringlist_t children;
        
                    parents  = PGArrayToStringList( parentsf,  parents  );
                    children = PGArrayToStringList( childrenf, children );

                    fh_concept ivc = m_IDToConceptMap[ id ];

                    for( stringlist_t::const_iterator si = parents.begin();
                         si != parents.end(); ++si )
                    {
//                         cerr << "Link ivc:" << ivc->getID() << endl;
//                         cerr << "TO:" << *si << endl;
                        ivc->makeLink( m_IDToConceptMap[ toint(*si) ], true );
                    }

                    if( parents.empty() )
                        m_topConcept = ivc;
                    if( children.empty() )
                        m_bottomConcept = ivc;
                }
            }
        

            {
                //
                // Make sure that the table has a standard extended form
                //
                if( !parentsColumnExists )
                {
                    stringstream ss;
                    ss << "alter table " << m_CFITableName
                       << " add column lattice_parents int[];" << endl;
                    trans.exec( ss.str() );
                }
                if( !childrenColumnExists )
                {
                    stringstream ss;
                    ss << "alter table " << m_CFITableName
                       << " add column lattice_children int[];" << endl;
                    trans.exec( ss.str() );
                }
                if( !addedAttrColumnExists )
                {
                    stringstream ss;
                    ss << "alter table " << m_CFITableName
                       << " add column lattice_added_attrs varchar[];" << endl;
                    trans.exec( ss.str() );
                }
                if( !conceptOnlyMatchSizeExists )
                {
                    stringstream ss;
                    ss << "alter table " << m_CFITableName
                       << " add column concept_only_support_abs int;" << endl;
                    trans.exec( ss.str() );
                }
            }

            trans.commit();
            m_CFITableIsAugmented = 1;
            setFile( m_latticeTree, "tablename-cfi-is-augmented", "1" );


//             //
//             // Load the inverted bit magic vectors
//             //
//             fh_context selfc = Shell::acquireSubContext( m_latticeTree,
//                                                          "inverted-lists-self",
//                                                          true );
//             fh_context extentc = Shell::acquireSubContext( m_latticeTree,
//                                                            "inverted-lists-extent",
//                                                            true );
//             for( m_IDToConceptMap_t::const_iterator ni = m_IDToConceptMap.begin();
//                  ni != m_IDToConceptMap.end(); ++ni )
//             {
//                 int        id = ni->first;
//                 fh_concept n  = ni->second;

// //                 priv_load_invertedlist( selfc,   id, n->m_conceptOnlyMatchingDocIDs );
// //                 priv_load_invertedlist( extentc, id, n->m_MatchingDocIDs );
//             }
            
            m_dirty = false;
#endif
        }

        fh_conceptLattice
        ConceptLattice::load( const std::string& latticeTreePath )
        {
            return ConceptLattice::load( Resolve( latticeTreePath ));
        }
        fh_conceptLattice
        ConceptLattice::load( fh_context latticeTree )
        {
            return ConceptLattice::load( latticeTree,
                                         &ConceptFactoryDefault_func );
        }

        fh_conceptLattice ConceptLattice::load(
            const std::string& latticeTreePath,
            const F_ConceptFactory_t& func )
        {
            return ConceptLattice::load( Resolve( latticeTreePath ), func );
        }
        
        fh_conceptLattice ConceptLattice::load(
            fh_context latticeTree,
            const F_ConceptFactory_t& func )
        {
            fh_conceptLattice cl = new ConceptLattice( latticeTree, func );
            cl->priv_load();
            
            LG_FCA_D << "ConceptLattice::load() cl:" << toVoid( cl ) << endl;
            return cl;
        }
        
        
        
        
//         fh_conceptLattice
//         ConceptLattice::load( pqxx::result& res )
//         {
//             ConceptLattice* cl = new ConceptLattice();
//             cl->priv_load( res );
//             return cl;
//         }

        void
        ConceptLattice::save_invertedlist( fh_context c, int id, FerrisBitMagic::bvector<>& bv )
        {
//             if( id == 805 )
//             {
// //                cerr << "save_invertedlist() id:" << id << endl;
//                 FerrisBitMagic::bvector<>::enumerator an     = bv.first();
//                 FerrisBitMagic::bvector<>::enumerator an_end = bv.end();

//                 while (an < an_end)
//                 {
//                     cerr << " " << *an;
//                     ++an;  // Fastest way to increment enumerator
//                 }
//                 cerr << endl << endl;
//             }
            
            // It is reccomended to optimize vector before serialization.
            bv.optimize();

            FerrisBitMagic::bvector<>::statistics st;
            bv.calc_stat(&st);

                    
            // Serialization to memory.
            string buf( st.max_serialize_mem,'\0');
            unsigned len = FerrisBitMagic::serialize( bv, (unsigned char*)buf.data() );

//             cerr << " save_inv. id:" << id << " len:" << len
//                  << " max:" << st.max_serialize_mem << endl;
            fh_context subc = Shell::acquireSubContext( c, tostr(id), false );
            setStrAttr( subc, "content", buf.substr( 0, len ) );
        }
        
        
        void
        ConceptLattice::save()
        {
#ifdef HAVE_LIBPQXX
//            cerr << "ConceptLattice::save(top)" << endl;
            work trans( *P->m_connection, "updating the lattice table..." );

            typedef set< int > existingIDs_t;
            existingIDs_t existingIDs;

            int totalConcepts = m_IDToConceptMap.size();
            int totalDirtyConcepts = getNumberOfDirtyConcepts();
            
//             cerr << "ConceptLattice::save() total concepts:" << totalConcepts << endl;
//             cerr << "ConceptLattice::save() dirty concepts:" << totalDirtyConcepts << endl;

            if( totalConcepts && ((1.0 * totalDirtyConcepts) / totalConcepts) > 0.3  )
            {
                Time::Benchmark benchmark("save concept lattice with bulk IO");
                
                // probably cheaper to clear the table and bulk load the new versions
                {
                    stringstream ss;
                    ss << "delete from " << m_CFITableName << " ; " << endl;
                    trans.exec( ss.str() );
                }

                tablewriter w( trans, m_CFITableName );

                for( m_IDToConceptMap_t::const_iterator ni = m_IDToConceptMap.begin();
                     ni != m_IDToConceptMap.end(); ++ni )
                {
                    fh_concept n  = ni->second;
                    list<string> tuple;

                    int itemSetNamesCount = m_attrIDToString.size();
                    stringstream bitfss;
                    {
                        stringstream& sqlss = bitfss;
                        const FerrisBitMagic::bvector<>& itemSet = n->getItemSet();
                        for( int i=0; i < itemSetNamesCount; ++i )
                        {
                            if( itemSet[ i ] )
                            {
                                sqlss << '1';
                            }
                            else
                            {
                                sqlss << '0';
                            }
                        }
                        sqlss << "00";
                    }
                    stringstream lattice_children_ss;
                    {
                        stringstream& sqlss = lattice_children_ss;
                        sqlss << "{";
                        bool v = true;
                        for( clist_t::iterator ci = n->m_children.begin();
                             ci != n->m_children.end(); ++ci )
                        {
                            if( v ) v = false;
                            else    sqlss << ",";
                            sqlss << (*ci)->getID();
                        }
                        sqlss << "}";
                    }
                    stringstream lattice_parents_ss;
                    {
                        stringstream& sqlss = lattice_parents_ss;
                        sqlss << "{";
                        bool v = true;
                        for( clist_t::iterator ci = n->m_parents.begin();
                             ci != n->m_parents.end(); ++ci )
                        {
                            if( v ) v = false;
                            else    sqlss << ",";
                            sqlss << (*ci)->getID();
                        }
                        sqlss << "}";
                    }
                    
                    stringstream lattice_added_attrs_ss;
                    {
                        stringstream& ss = lattice_added_attrs_ss;
                        if( n->m_parents.empty() )
                        {
                            ss << "{}";
                        }
                        else
                        {
                            
                            ss << "{";
                            FerrisBitMagic::bvector<> tItemSet;
                            for( clist_t::iterator ci = n->m_parents.begin();
                                 ci != n->m_parents.end(); ++ci )
                            {
                                tItemSet |= (*ci)->getItemSet();
                            }
                            tItemSet ^= n->getItemSet();

                            if( anythingSet( tItemSet ) )
                            {
                                FerrisBitMagic::bvector<>::enumerator an     = tItemSet.first();
                                FerrisBitMagic::bvector<>::enumerator an_end = tItemSet.end();
                                bool v = true;
                                while (an < an_end)
                                {
                                    FerrisBitMagic::bvector<> t;
                                    t[ *an ] = 1;

                                    if( v ) v = false;
                                    else    ss << ",";
                                    string ac = AttrIDToString( t );
                                    LG_FCA_D << " loop.added-attributes:" << t
                                             << " loop.added-attributes:" << ac << endl;
                                    ss << ac;
                                    
                                    ++an;  // Fastest way to increment enumerator
                                }
                                
//                                 bool v = true;
//                                 for( int i=0; i < m_itemSetSize; ++i )
//                                 {
//                                     FerrisBitMagic::bvector<> t;
//                                     t[i] = true;
                            
//                                     LG_FCA_D << " tItemSet:" << tItemSet
//                                              << " t:" << t
//                                              << " tItemSet & t:" << (tItemSet & t)
//                                              << endl;
//                                     if( anythingSet( tItemSet & t ) )
//                                     {
//                                         if( v ) v = false;
//                                         else    ss << ",";
//                                         LG_FCA_D << " loop.added-attributes:" << t
//                                                  << " loop.added-attributes:"
//                                                  << AttrIDToString( t ) << endl;
//                                         string ac = AttrIDToString( t );
//                                         ss << ac;
//                                     }
//                                 }
                            }
                            ss << "}";
                        }
                    }
                    
                    tuple.push_back( tostr(n->getID()) );
                    tuple.push_back( tostr(n->m_support_perc) );
                    tuple.push_back( tostr(n->m_support_abs) );
                    tuple.push_back( tostr(n->m_support_ext) );
                    
                    tuple.push_back( bitfss.str() );
                    tuple.push_back( lattice_parents_ss.str() );
                    tuple.push_back( lattice_children_ss.str() );
                    tuple.push_back( lattice_added_attrs_ss.str() );
                    tuple.push_back( tostr(n->getConceptOnlyMatchSize()) );
                    
                    
                    stringstream sqlss;
                    sqlss << "insert into " << m_CFITableName
                          << " (id,support_perc, support_abs, support_ext "
                          << " ,concept_only_support_abs "
                          << " ,lattice_children, lattice_parents, lattice_added_attrs "
                          << ",bitf ) values ( "
                          << n->getID() << ","
                          << n->m_support_perc << ","
                          << n->m_support_abs  << ","
                          << n->m_support_ext << ","
                          << n->getConceptOnlyMatchSize();
                    
                    sqlss << ",'{}'";
                    w << tuple;
                }
                
                w.complete();
            }
            else
            {
                Time::Benchmark benchmark("save concept lattice");
                
            
                //
                // find which nodes are already in the database
                //
                {
                    stringstream ss;
                    ss << "select id from " << m_CFITableName;
                    result res = trans.exec( ss.str() );

                    for( result::const_iterator iter = res.begin();
                         iter != res.end(); ++iter )
                    {
                        int id;
                        iter["id"].to(id);
                        existingIDs.insert(id);
                    }
                }
//                cerr << "ConceptLattice::save(2)" << endl;
                
                //
                // SQL update existing nodes, SQL insert nodes that are new
                //
                for( m_IDToConceptMap_t::const_iterator ni = m_IDToConceptMap.begin();
                     ni != m_IDToConceptMap.end(); ++ni )
                {
                    int        id = ni->first;
                    fh_concept n  = ni->second;

                    if( n->m_parents.empty() )
                        m_topConcept = n;
                    if( n->m_children.empty() )
                        m_bottomConcept = n;
                
                    if( existingIDs.find( id ) != existingIDs.end() )
                    {
                        stringstream ss;

                        ss << "update " << m_CFITableName
                           << " set concept_only_support_abs = " << n->getConceptOnlyMatchSize() << endl;
                        // update parents
                        {
                            ss << ", lattice_parents = '{";
                            bool v = true;
                            for( clist_t::iterator ci = n->getParents().begin();
                                 ci != n->getParents().end(); ++ci )
                            {
                                if( v ) v = false;
                                else    ss << ",";
                                ss << (*ci)->getID();
                            }
                            ss << "}' " << endl;
                        }
                        // update children
                        {
                            ss << " , lattice_children = '{";
                            bool v = true;
                            for( clist_t::iterator ci = n->getChildren().begin();
                                 ci != n->getChildren().end(); ++ci )
                            {
                                if( v ) v = false;
                                else    ss << ",";
                                ss << (*ci)->getID();
                            }
                            ss << "}' " << endl;
                        }
                        // update bitf
                        {
                            ss << ", bitf = B'";
                            for( int i=0; i < m_itemSetSize; ++i )
                            {
                                ss << n->m_itemSet[ i ];
                            }
                            ss << "'::bit(" << m_itemSetSize << ") " << endl;
                        }
                        // update added attributes
                        if( n->getParents().empty() )
                        {
                            ss << " , lattice_added_attrs = '{}' " << endl;
                        }
                        else
                        {
                            LG_FCA_D << "------------------------" << endl;
                            LG_FCA_D << " n.id:" << n->getID()
                                     << " n.parents.size():" << n->getParents().size() << endl;
                            FerrisBitMagic::bvector<> tItemSet;
                            for( clist_t::iterator pi = n->getParents().begin();
                                 pi != n->getParents().end(); ++pi )
                            {
                                fh_concept p = *pi;
                                LG_FCA_D //<< " p:" << p->m_itemNames
                                    << " p.id:" << p->getID()
                                    << " pitemset:" << p->getItemSet()
                                    << endl;
                                tItemSet |= p->getItemSet();
                            }
                            tItemSet ^= n->getItemSet();
                    
                            ss << " , lattice_added_attrs = '{";

                            if( anythingSet( tItemSet ) )
                            {
                                FerrisBitMagic::bvector<>::enumerator an     = tItemSet.first();
                                FerrisBitMagic::bvector<>::enumerator an_end = tItemSet.end();
                                bool v = true;
                                while (an < an_end)
                                {
                                    FerrisBitMagic::bvector<> t;
                                    t[ *an ] = 1;

                                    if( v ) v = false;
                                    else    ss << ",";
                                    string ac = AttrIDToString( t );
                                    LG_FCA_D << " loop.added-attributes:" << t
                                             << " loop.added-attributes:" << ac << endl;
                                    ss << ac;
                                    
                                    ++an;  // Fastest way to increment enumerator
                                }
                                
//                                 bool v = true;
//                                 for( int i=0; i < m_itemSetSize; ++i )
//                                 {
//                                     FerrisBitMagic::bvector<> t;
//                                     t[i] = true;
                            
//                                     LG_FCA_D << " tItemSet:" << tItemSet
//                                              << " t:" << t
//                                              << " tItemSet & t:" << (tItemSet & t)
//                                              << endl;
//                                     if( anythingSet( tItemSet & t ) )
//                                     {
//                                         if( v ) v = false;
//                                         else    ss << ",";
//                                         LG_FCA_D << " loop.added-attributes:" << t
//                                                  << " loop.added-attributes:"
//                                                  << AttrIDToString( t ) << endl;
//                                         string ac = AttrIDToString( t );
//                                         ss << ac;
//                                     }
//                                 }
                            }
                    
                            ss << "}' " << endl;
                        }
                        ss << " where id = " << n->getID();
                        LG_FCA_D << "UPDATE:" << ss.str() << endl;
                        trans.exec( ss.str() );
                    }
                    else
                    {
                        // concept is new
                        int itemSetNamesCount = m_attrIDToString.size();
//                        stringstream itemSetNamesSS;
//                         int itemSetNamesCount = 0;
//                         for( m_attrIDToString_t::const_iterator ci = m_attrIDToString.begin();
//                              ci != m_attrIDToString.end(); ++ci )
//                         {
// //                            itemSetNamesSS << ",\"" << ci->second << "\"";
//                             ++itemSetNamesCount;
//                         }
                    
                        stringstream sqlss;
                        sqlss << "insert into " << m_CFITableName
                              << " (id,support_perc, support_abs, support_ext "
                              << " ,concept_only_support_abs "
                              << " ,lattice_children, lattice_parents, lattice_added_attrs "
                              << ",bitf ) values ( "
//                          << itemSetNamesSS.str() << " ) values ( "
                              << n->getID() << ","
                              << n->m_support_perc << ","
                              << n->m_support_abs  << ","
                              << n->m_support_ext << ","
                              << n->getConceptOnlyMatchSize();
                        {
                            sqlss << ",'{";
                            bool v = true;
                            for( clist_t::iterator ci = n->m_children.begin();
                                 ci != n->m_children.end(); ++ci )
                            {
                                if( v ) v = false;
                                else    sqlss << ",";
                                sqlss << (*ci)->getID();
                            }
                            sqlss << "}'";
                        }
                        {
                            sqlss << ",'{";
                            bool v = true;
                            for( clist_t::iterator ci = n->m_parents.begin();
                                 ci != n->m_parents.end(); ++ci )
                            {
                                if( v ) v = false;
                                else    sqlss << ",";
                                sqlss << (*ci)->getID();
                            }
                            sqlss << "}'";
                        }
                    
                        sqlss << ",'{}'";

                        string btab = getBaseTableName();
                        sqlss << ",(";
                        sqlss << "meta_" << getBitFunctionName( btab, "zero_bitf",false ) << "() ";
                    
                        stringstream bitfss;
                        const FerrisBitMagic::bvector<>& itemSet = n->getItemSet();
                        for( int i=0; i < itemSetNamesCount; ++i )
                        {
                            FerrisBitMagic::bvector<> t;
                            if( itemSet[ i ] )
                            {
                                t[ i ] = itemSet[ i ];
                                std::string attrName = AttrIDToString( t );
                                sqlss << " |  " << getBitFunctionName(btab,attrName) << "() ";
                            }
                        }
                        // FIXME: we need to know how large the bitf is in the base table.
                        sqlss << " )";
                        sqlss << "::bit( " << m_itemSetSize << ") ";
                        sqlss << " );" << endl;
                        LG_FCA_D << " m_itemSetSize:" << m_itemSetSize << endl;
                        LG_FCA_D << "INSERT.SQL:" << sqlss.str() << endl;
//                        cerr << "INSERT.SQL:" << sqlss.str() << endl;
                        trans.exec( sqlss.str() );
            
                    
                    }
                }


                //
                // If some nodes were erased then remove them from the database aswell
                //
                {
                    typedef set< int > intset_t;
                    intset_t inUseIDs;
                    intset_t result;

                    copy( map_domain_iterator( m_IDToConceptMap.begin() ),
                          map_domain_iterator( m_IDToConceptMap.end() ),
                          inserter( inUseIDs, inUseIDs.end() ));
                
                    set_difference( existingIDs.begin(), existingIDs.end(),
                                    inUseIDs.begin(), inUseIDs.end(),
                                    inserter( result, result.end() ) );

//                     cerr << "Begin deleting nodes....result.sz:" << result.size()
//                          << " existingIDs.size:" << existingIDs.size()
//                          << " inUseIDs.size:" << inUseIDs.size()
//                          << endl;

                    int i = 0;
                    for( intset_t::const_iterator ci = result.begin(); ci != result.end(); ++ci )
                    {
                        stringstream ss;
                        int id = *ci;
                        ss << "delete from " << m_CFITableName << " where id = " << id << " ; " << endl;
//                        cerr << "DELETE " << i << " SQL:" << ss.str();
                        trans.exec( ss.str() );
                        ++i;
                    }
//                    cerr << "FINISHED DELETING NODES...." << endl;
                }
            
            
//                cerr << "ConceptLattice::save(3)" << endl;
            }
        
            trans.commit();
            if( m_topConcept )
                setFile( m_latticeTree, "lattice-top-id", tostr(m_topConcept->getID()) );
            if( m_bottomConcept )
                setFile( m_latticeTree, "lattice-bottom-id", tostr(m_bottomConcept->getID()) );
            
            m_dirty = false;

//            cerr << "save() done" << endl;
            
//             // Save the inverted bitmagic vectors
//             {
//                 fh_context selfc = Shell::acquireSubContext( m_latticeTree,
//                                                              "inverted-lists-self",
//                                                              true );
//                 fh_context extentc = Shell::acquireSubContext( m_latticeTree,
//                                                                "inverted-lists-extent",
//                                                                true );

//                 for( m_IDToConceptMap_t::const_iterator ni = m_IDToConceptMap.begin();
//                      ni != m_IDToConceptMap.end(); ++ni )
//                 {
//                     int        id = ni->first;
//                     fh_concept n  = ni->second;

//                     cerr << "concept:" << id
//                          << " m_conceptOnlyMatchingDocIDs:" << n->m_conceptOnlyMatchingDocIDs.count()
//                          << " m_MatchingDocIDs.count:" << n->m_MatchingDocIDs.count()
//                          << endl;
//                     save_invertedlist( selfc,   id, n->m_conceptOnlyMatchingDocIDs );
//                     save_invertedlist( extentc, id, n->m_MatchingDocIDs );
//                 }
//             }
#endif
        }

        void
        ConceptLattice::setConceptFactory( const F_ConceptFactory_t& func )
        {
            m_conceptFactory_func = func;
        }
        
        
        int
        ConceptLattice::getHighestConceptID() const
        {
            int ret = 0;
            for( m_IDToConceptMap_t::const_iterator ci = m_IDToConceptMap.begin();
                 ci != m_IDToConceptMap.end(); ++ci )
            {
                ret = max( ci->first, ret );
            }
            return ret;
        }
        
        fh_concept
        ConceptLattice::getConcept( int id ) const
        {
            m_IDToConceptMap_t::const_iterator ci = m_IDToConceptMap.find( id );
            if( ci != m_IDToConceptMap.end() )
                return ci->second;

            stringstream ss;
            ss << "getConcept() id:" << id << " no such concept" << endl;
            Throw_NoSuchObject( ss.str(), 0 );
        }

        void
        ConceptLattice::addConcept( fh_concept c )
        {
            m_dirty = true;
            m_IDToConceptMap.insert( make_pair( c->getID(), c ) );
        }

        void
        ConceptLattice::removeConcept( fh_concept c )
        {
            m_dirty = true;
            c->disconnect();
            m_IDToConceptMap.erase( c->getID() );
        }
    
        
        void
        ConceptLattice::priv_ConceptItemSetChanging( Concept* c,
                                                     FerrisBitMagic::bvector<> oldv,
                                                     FerrisBitMagic::bvector<> newv )
        {
            m_dirty = true;
            // FIXME: changing the itemset could change the top/bottom nodes
//             fh_concept m_topConcept;
//             fh_concept m_bottomConcept;
        }
        
        void
        ConceptLattice::priv_ConceptIDChanging( Concept* c, int oldv, int newv )
        {
            m_dirty = true;
            m_IDToConceptMap.erase( oldv );
            m_IDToConceptMap[ newv ] = c;
        }
        
        
        void
        ConceptLattice::setDirty( bool v )
        {
            m_dirty = v;
        }
        
        bool
        ConceptLattice::isDirty()
        {
            if( m_dirty )
                return m_dirty;

            for( m_IDToConceptMap_t::const_iterator ni = m_IDToConceptMap.begin();
                 ni != m_IDToConceptMap.end(); ++ni )
            {
                fh_concept n  = ni->second;
                m_dirty |= n->isDirty();
                if( m_dirty )
                    return m_dirty;
            }
            return m_dirty;
        }
        int
        ConceptLattice::getNumberOfDirtyConcepts()
        {
            int ret = 0;
            
            for( m_IDToConceptMap_t::const_iterator ni = m_IDToConceptMap.begin();
                 ni != m_IDToConceptMap.end(); ++ni )
            {
                fh_concept n  = ni->second;
                if( n->isDirty() )
                    ++ret;
            }
            
            return ret;
        }
        
        void
        ConceptLattice::exportAsBurmeister( const std::string& outbase )
        {
            string btab = getBaseTableName();
            string ctab = m_CFITableName;
            ConceptLatticePriv* P = getPrivatePart();
            connection& con = *(P->m_connection);

            const std::string outFileName = outbase + ".cxt";
            ofstream oss( outFileName.c_str() );

            fh_latticelayout lay = new LatticeLayout_Layered();
            lay->layout( this );
//            cerr << "Lattice is layed out..." << endl;

            work trans( con, "getting the data..." );

            stringlist_t attrnames;
            getAllAttributeNames( attrnames );

            long urlid_max = 0;
            typedef map< long, stringset_t > urlattrs_t;
            urlattrs_t urlattrs;
            
            {
                stringstream sqlss;
                sqlss << "select * from urlmap um, " << btab << " g where um.urlid = g.urlid;"
                      << endl;
                result res = trans.exec( sqlss.str() );
                
                oss << "B" << endl << endl;
                oss << res.size() << endl;
                oss << attrnames.size() << endl;
                oss << endl;
                
                for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
                {
                    string earl;
                    long urlid = 0;
                    iter["urlid"].to( urlid );
                    iter["url"].to( earl );
//                    oss << urlid << endl;
                    oss << earl << endl;
                    urlattrs[ urlid ];
                    urlid_max = max( urlid_max, urlid );
                }

                stringlist_t::iterator iter = attrnames.begin();
                stringlist_t::iterator    e = attrnames.end();
                for( ; iter != e ; ++iter )
                {
                    oss << *iter << endl;
                }
            }
            

            
            {
                {
                    stringlist_t::iterator iter = attrnames.begin();
                    stringlist_t::iterator    e = attrnames.end();
                    for( ; iter != e ; ++iter )
                    {
                        string attr = *iter;

                        stringstream sqlss;
                        sqlss << "select urlid from " << btab << " d where ";
                        sqlss << " " << getBitFunctionName(btab,attr) << "(d.bitf) ;";
                        sqlss << endl;
                        cerr << sqlss.str() << endl;

                    
                        result res = trans.exec( sqlss.str() );
                        for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
                        {
                            long urlid = 0;
                            iter["urlid"].to( urlid );
                            urlattrs[ urlid ].insert( attr );
                        }
                    }
                }

                // typedef map< long, stringset_t > urlattrs_t;
                urlattrs_t::iterator urliter = urlattrs.begin();
                urlattrs_t::iterator    urle = urlattrs.end();
                for( ; urliter != urle ; ++urliter )
                {
                    long urlid = urliter->first;
                    stringset_t& aset = urliter->second;
                    
                    stringlist_t::iterator iter = attrnames.begin();
                    stringlist_t::iterator    e = attrnames.end();
                    for( ; iter != e ; ++iter )
                    {
                        string attr = *iter;
                        if( aset.find( attr ) != aset.end() )
                            oss << "X";
                        else
                            oss << ".";
                    }
                    oss << endl;
                }
                
                
                
                
//                 EAIndex::fh_idx idx = EAIndex::Factory::getEAIndex( m_findexPath );
                
//                 stringmap_t::iterator iter = attrmap.begin();
//                 stringmap_t::iterator    e = attrmap.end();
//                 for( ; iter != e ; ++iter )
//                 {
//                     string ffilter_string = iter->second;

//                     EAIndex::fh_eaquery q = EAIndex::Factory::makeEAQuery( ffilter_string, idx );

//                     std::stringstream LocalSQLHeaderSS;
//                     std::stringstream LocalSQLWherePredicatesSS;
//                     std::stringstream LocalSQLTailerSS;
//                     stringset_t lookupTablesUsed;
//                     bool queryHasTimeRestriction;
//                     std::string DocIDColumn;
//                     stringset_t eanamesUsed;
//                     EAIndex::MetaEAIndexerInterface::BuildQuerySQLTermInfo_t termInfo;
        
//                     q->getQuerySQL( LocalSQLHeaderSS,
//                                     LocalSQLWherePredicatesSS,
//                                     LocalSQLTailerSS,
//                                     lookupTablesUsed,
//                                     queryHasTimeRestriction,
//                                     DocIDColumn,
//                                     eanamesUsed,
//                                     termInfo );

//                     stringstream sqlss;
//                     sqlss << "select * from " << btab << " where ";
//                     sqlss << LocalSQLWherePredicatesSS.str();
//                     sqlss << endl;
//                     cerr << sqlss.str() << endl;

                    
//                     result res = trans.exec( sqlss.str() );
//                     for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
//                     {
//                         long urlid = 0;
//                         iter["urlid"].to( urlid );
//                         oss << urlid << endl;
//                     }

                    
//                     break;
//                 }
            }
            

//             for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
//             {
//                 long urlid = 0;
//                 stringlist_t addedAttrsList;

//                 string addedAttrs;
//                 int support_abs = 0;
//                 int nodeX = 0;
//                 int nodeY = 0;
                    
//                 iter["urlid"].to( urlid );
//                 iter["support_abs"].to( support_abs );
//                 iter["lattice_added_attrs"].to( addedAttrs );
//             }
                
        }
        

        void
        ConceptLattice::exportAsToscanaJConceptualSchema( const std::string& outbase )
        {
            const std::string outXMLFileName = outbase + ".csx";
            const std::string outSQLFileName = outbase + ".sql";
            string btab = getBaseTableName();
            string ctab = m_CFITableName;
            double CoordScaleFactor = 25.0;
            
            ConceptLatticePriv* P = getPrivatePart();
            connection& con = *(P->m_connection);
            work trans( con, "getting the data..." );
            stringstream sqlss;
            sqlss << "select * from " << ctab << endl;
            result res = trans.exec( sqlss.str() );
            
            {
                
                ofstream oss( outSQLFileName.c_str() );
                oss << "create table main " << endl
                    << " ( " << endl
                    << " id int primary key" << endl;
                int max = m_attrIDToString.size();
                for( int i = 0; i < max; ++i )
                {
                    FerrisBitMagic::bvector<> t;
                    t[ i ] = 1;
                    string ac = AttrIDToString( t );
                    oss << ", " << ac << " int" << endl;
                }
                oss << " ); " << endl;
            }

            fh_latticelayout lay = new LatticeLayout_Layered();
            lay->layout( this );
//            cerr << "Lattice is layed out..." << endl;
            
            {
                ofstream oss( outXMLFileName.c_str() );
                oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
                oss << "<conceptualSchema version=\"TJ1.0\">" << endl;
                oss << "<databaseConnection> " << endl
                    << "        <embed url=\"" << outSQLFileName << "\" /> " << endl
                    << "        <table name=\"main\" /> " << endl
                    << "        <key name=\"id\" /> " << endl
                    << "    </databaseConnection> " << endl
                    << "    <diagram title=\"" << btab << "\">" << endl;

                stringstream sqlss;
                sqlss << "select * from " << ctab;
//                cerr << "SQL:" << sqlss.str() << endl;
                result res = trans.exec( sqlss.str() );
                
                //
                // nodes
                //
                for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
                {
                    int id = 0;
                    stringlist_t addedAttrsList;

                    string addedAttrs;
                    int support_abs = 0;
                    int nodeX = 0;
                    int nodeY = 0;
                    
                    iter["id"].to( id );
                    iter["support_abs"].to( support_abs );
                    iter["lattice_added_attrs"].to( addedAttrs );
                    PGArrayToStringList( addedAttrs, addedAttrsList );
                    if( !addedAttrs.empty() )
                        addedAttrs = addedAttrs.substr( 1, addedAttrs.length() - 2 );

                    fh_concept concept = getConcept( id );
                    nodeX = (int)(concept->getX() * CoordScaleFactor);
                    nodeY = (int)(concept->getY() * CoordScaleFactor);
//                     cerr << "Concept.id:" << concept->getID()
//                          << " x:" << concept->getX()
//                          << " y:" << concept->getY()
//                          << endl;
                    
                    
                    oss << "        <node id=\"" << id << "\">" << endl
                        << "            <position x=\"" << nodeX << "\" y=\"" << nodeY << "\" />" << endl
                        << "            <concept>" << endl
                        << "                <objectContingent>" << endl
                        << "                    <object contextPosition=\"0\">"
                        <<                        support_abs << "</object>" << endl
                        << "                </objectContingent>" << endl
                        << "                <attributeContingent>" << endl;

                    for( stringlist_t::const_iterator si = addedAttrsList.begin();
                         si != addedAttrsList.end(); ++si )
                    {
                        string s = *si;
                        FerrisBitMagic::bvector<> bv = StringToAttrID( s );
                        FerrisBitMagic::bvector<>::enumerator an = bv.first();
                        long v = *an;
                        
                        oss << "                    "
                            << "<attribute contextPosition=\"" << v << "\">"
                            << s << "</attribute>" << endl;
                    }

                    oss << "                </attributeContingent>" << endl
                        << "            </concept>" << endl
                        << "            <ndimVector>" << endl
                        << "                <coordinate>0.0</coordinate>" << endl
                        << "                <coordinate>0.0</coordinate>" << endl
                        << "            </ndimVector>" << endl
                        << "        </node>" << endl;
                }


                //
                // edges
                //
                for( result::const_iterator iter = res.begin(); iter != res.end(); ++iter )
                {
                    int id = 0;
                    stringlist_t childrenList;
                    string s;
                    
                    iter["id"].to( id );
                    iter["lattice_children"].to( s );
                    PGArrayToStringList( s, childrenList );

//                    cerr << "id:" << id << " to children:" << Util::createSeperatedList(childrenList) << endl;
                    
                    for( stringlist_t::const_iterator si = childrenList.begin();
                         si != childrenList.end(); ++si )
                    {
                        string child = *si;
                        oss << "       <edge from=\"" << id << "\" to=\"" << child << "\" />" << endl;
                    }
                }
                

                
                oss << "        <projectionBase>" << endl
                    << "            <vector x=\"0.0\" y=\"0.0\" />" << endl
                    << "            <vector x=\"0.0\" y=\"0.0\" />" << endl
                    << "        </projectionBase>" << endl
                    << "    </diagram>" << endl
                    << "</conceptualSchema>" << endl;
                


            }
            
        }
        
        
        int
        ConceptLattice::getStrSubCtxInt( const std::string& k, int def )
        {
            return
                toint(
                    getStrSubCtx(
                        m_latticeTree, k, tostr(def) ) );
        }
        
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
#ifdef HAVE_LIBPQXX
        class LatticeRootConcept;

        /**
         * Individual concept in a concept lattice.
         */
        class FERRISEXP_DLLLOCAL ConceptContext
            :
            public StateLessEAHolding_Recommending_ParentPointingTree_Context< ConceptContext >
        {
            typedef StateLessEAHolding_Recommending_ParentPointingTree_Context< ConceptContext > _Base;
            typedef ConceptContext _Self;
            friend class Ferris::RootContextFactory;
            friend class LatticeRootConcept;

            fh_concept m_concept;
            bool m_extentViewer; // are we an 'extent' sub-directory
            bool m_contingentViewer; // are we an 'cont' sub-directory
            std::string m_ViewOfExtentRecommendedEA;
            int m_extentSizeLimit; // max number of files to show if( isExtentViewer() )

        public:
            typedef std::map< string, string > m_ViewOfExtentSchemas_t;
            m_ViewOfExtentSchemas_t m_ViewOfExtentSchemas;
        private:
            
            void setConcept( fh_concept& v );
            void isExtentViewer( bool v );
            void isContingentViewer( bool v );
            bool getIsViewOfExtent()
                {
                    return m_extentViewer | m_contingentViewer;
                }
            void setExtentSizeLimit( int v )
                {
                    m_extentSizeLimit = v;
                }

            virtual fh_context priv_getSubContext( const std::string& rdn )
                throw( NoSuchSubContext );

            int getStrSubCtxInt( const std::string& k, int def );
            void priv_createExtentViewerSubDir( const char* name, const int limit );
            void createExtentViewerSubDir();
            
        protected:
            virtual void priv_read_files();
            virtual void priv_read();

            virtual long priv_guessSize() throw();
            virtual bool getHasSubContextsGuess()
                {
                    if( getIsViewOfExtent() )
                        return false;
                    return _Base::getHasSubContextsGuess();
                }

            
        public:

            ConceptContext( Context* parent = 0, const std::string& rdn = "" );
            virtual ~ConceptContext();

            virtual fh_iostream getEAStream( Context* c, const std::string& rdn, EA_Atom* atom );
            virtual void setEAStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream );

            LatticeRootConcept* getLatticeRootConcept();
            fh_conceptLattice getLattice();
            fh_concept getConcept();

            std::string getRecommendedEA();
            virtual void createStateLessAttributes( bool force = false );
            static fh_stringstream
            SL_getExtentSize( ConceptContext* c, const std::string& rdn, EA_Atom* atom );
            static fh_stringstream
            SL_getID( ConceptContext* c, const std::string& rdn, EA_Atom* atom );
            static fh_stringstream
            SL_getExtentViewerRecommendedEA( ConceptContext* c, const std::string& rdn, EA_Atom* atom );
            static fh_stringstream
            SL_getFormalAttributes( ConceptContext* c, const std::string& rdn, EA_Atom* atom );
            static fh_stringstream
            SL_getAddedAttributes( ConceptContext* c, const std::string& rdn, EA_Atom* atom );
            static fh_stringstream
            SL_getAddedAttributesRelativeToParent( ConceptContext* c, const std::string& rdn, EA_Atom* atom );
            static fh_stringstream
            SL_getAddedAttributesRelativeToParentLabel( ConceptContext* c, const std::string& rdn, EA_Atom* atom );
            static fh_stringstream
            SL_getFishEyeLabel( ConceptContext* c, const std::string& rdn, EA_Atom* atom );
            
            string getBaseTableName();
        };

        class LatticeRootConceptVFS_RootContextDropper;
        
        /**
         * Contains the database info etc to get the CFI from.
         */
        class FERRISEXP_DLLLOCAL LatticeRootConcept
            : public ParentPointingTreeContext< ConceptContext >
        {
            typedef ParentPointingTreeContext< ConceptContext > _Base;
            typedef ConceptContext _Self;
            friend class Ferris::RootContextFactory;
            friend class LatticeRootConceptVFS_RootContextDropper;
            friend class ConceptContext;

            fh_context m_metadata;
            string m_user;
            string m_port;
            string m_host;
            string m_dbname;
            string m_CFITableName;
            string m_findexPath;
            connection* db_connection;
            
            fh_conceptLattice m_conceptLattice;
            
            void ensureDatabaseConnection();
            void readConcepts();
            void connectConcepts();
            
        protected:
            virtual void priv_read();

            
        public:

            LatticeRootConcept( Context* parent = 0,
                                const std::string& rdn = "",
                                fh_context md = 0 );
            virtual ~LatticeRootConcept();

            virtual fh_iostream getEAStream( Context* c, const std::string& rdn, EA_Atom* atom );
            virtual void setEAStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream );

            fh_conceptLattice getLattice();

            string getBaseTableName();
        };
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        class DatabaseResultSetContext
            :
            public StateLessEAHolder< DatabaseResultSetContext, FakeInternalContext >
        {
            typedef StateLessEAHolder< DatabaseResultSetContext, FakeInternalContext > _Base;
            typedef DatabaseResultSetContext _Self;

            typedef stringmap_t m_md_t;
//            typedef std::hash_map< string, string > m_md_t;
//            typedef Loki::AssocVector< string, string > m_md_t;
//            typedef FAssocVector< string, string > m_md_t;
            m_md_t m_md;

        protected:

            virtual std::string private_getStrAttr( const std::string& rdn,
                                                    const std::string& def = "",
                                                    bool getAllLines = false ,
                                                    bool throwEx = false );
            virtual void createStateLessAttributes( bool force = false );
            virtual void priv_createAttributes()
                {
                    imageEAGenerator_priv_createAttributes();
                }
            
        public:
            DatabaseResultSetContext( Context* parent = 0,
                                      const std::string& rdn = "" );
            virtual ~DatabaseResultSetContext();

            std::string getRecommendedEA()
                {
//                     cerr << "PGContext::getRecommendedEA()"
//                          << getStrAttr( getParent(), "extent-viewer-recommended-ea", "name" )
//                          << endl;
                    
                    return getStrAttr( getParent(), "extent-viewer-recommended-ea", "name" );
                }
//             stringstream ss;
//                     for( m_md_t::const_iterator mdi = m_md.begin();
//                          mdi != m_md.end(); ++mdi )
//                     {
//                         ss << mdi->first << ",";
//                     }
//                     return ss.str();
//                 }

            
            void constructObject( ConceptContext* cc, result::const_iterator& iter );
            virtual std::string getURL()
                {
                    return getDirName();
                }
            virtual int getImageWidth()
                {
                    m_md_t::const_iterator mi = m_md.find("width");
//                    cerr << "PGContext::getImageWidth() bound:" << ( mi != m_md.end() ) << endl;
                    if( mi != m_md.end() )
                        return toint(mi->second);
                    return _Base::getImageWidth();
                }
            virtual int getImageHeight()
                {
                    m_md_t::const_iterator mi = m_md.find("height");
                    if( mi != m_md.end() )
                        return toint(mi->second);
                    return _Base::getImageHeight();
                }


            static fh_stringstream
            SL_getSizeStream( DatabaseResultSetContext* c,
                              const std::string& rdn,
                              EA_Atom* atom );
            static fh_stringstream
            SL_getFerrisType( DatabaseResultSetContext* c,
                              const std::string& rdn,
                              EA_Atom* atom );
            static fh_stringstream
            SL_getDelegateName( DatabaseResultSetContext* c,
                                const std::string& rdn,
                                EA_Atom* atom );
            
            virtual void ensureAttributesAreCreated( const std::string& eaname = "" )
                {
                }

        string toCachedAttrName( string s )
            {
                return "cached-" + s;
            }
        string fromCachedAttrName( string s )
            {
                return s.substr( strlen("cached-") );
            }

        fh_istream GetIStream( Context* c, const std::string& _rdn, EA_Atom* atom )
            {
                string rdn = fromCachedAttrName( _rdn );
                LG_CTX_D << "GetIStream for rdn:" << rdn
                         << " this:" << getURL()
                         << endl;

                m_md_t::const_iterator mdi = m_md.find( rdn );
                if( mdi != m_md.end() )
                {
                    fh_stringstream ret;
                    ret << mdi->second;
                    return ret;
                }
                
                fh_stringstream ss;
                return ss;
            }
            
        fh_iostream GetIOStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                return ss;
            }

            
        void IOStreamClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream iss )
            {
            }

            
        fh_attribute
        getAttribute( const std::string& _rdn ) throw( NoSuchAttribute )
            {
//                cerr << "getAttribute() rdn:" << _rdn << endl;
                
                std::string rdn = _rdn;
                
                if( !rdn.length() || rdn == "." )
                {
                    rdn = "content";
                }
                if( rdn == "content" || rdn == "size-human-readable" )
                {
                    return _Base::getAttribute( rdn );
                }

                string rawrdn = rdn;
                rdn = toCachedAttrName( rdn );
                LG_CTX_D << "CacheContext::getAttribute rdn:" << rdn << endl;

                /* Try for cache hit */
                if( EA_Atom* atom = getAttributeIfExists( rdn ) )
                {
                    LG_CTX_D << "CacheContext::getAttribute cached for rdn:" << rdn << endl;
                    return new AttributeProxy( this, atom, rdn );
                }

                LG_CTX_D << "CacheContext::getAttribute create for rdn:" << rdn << endl;
                /* Create new caching EA, attach it, and return attribute */
                addAttribute( rdn,
                              this, &_Self::GetIStream,
                              this, &_Self::GetIOStream,
                              this, &_Self::IOStreamClosed,
                              FXD_BINARY, // getUnderlyingEAType( rawrdn ),
                              0 );
                if( EA_Atom* atom = getAttributeIfExists( rdn ) )
                {
                    return new AttributeProxy( this, atom, rdn );
                }
                
                std::stringstream ss;
                ss << "NoSuchAttribute() for attr:" << rdn << endl;
                Throw_NoSuchAttribute( tostr(ss), this );
            }
            
            // having a version of this method seems to create fairly large slowdown.
//             virtual AttributeCollection::AttributeNames_t getAttributeNames()
//                 {
//                     stringset_t all;
//                     AttributeCollection::AttributeNames_t an = _Base::getAttributeNames();

// //                    all.reserve( m_md.size() + an.size() + 1 );
// //                     copy( map_domain_iterator(m_md.begin()),
// //                           map_domain_iterator(m_md.end()),
// //                           inserter( all, all.end() ));

//                     typedef stringmap_t m_md_t;
//                     m_md_t::const_iterator mde = m_md.end();
//                     for( m_md_t::const_iterator mdi = m_md.begin(); mdi!=mde; ++mdi )
//                         all.insert( mdi->first );
                    
//                     for( AttributeCollection::AttributeNames_t::const_iterator ai = an.begin();
//                          ai != an.end(); ++ai )
//                     {
//                         all.insert( *ai );
//                     }
//                     AttributeNames_t ret;
//                     copy( all.begin(), all.end(), back_inserter( ret ));
                
//                     return ret;
//                 }

//             int
//             getAttributeCount()
//                 {
//                     return Delegate->getAttributeCount();
//                 }
            bool
            isAttributeBound( const std::string& rdn,
                              bool createIfNotThere
                ) throw( NoSuchAttribute )
                {
//                    cerr << "isAttributeBound() rdn:" << rdn << endl;
                    
                    m_md_t::const_iterator mdi = m_md.find( rdn );
                    if( mdi != m_md.end() )
                    {
//                        cerr << "   isAttributeBound(1) rdn:" << rdn << endl;
                        return true;
                    }
                    
//                     cerr << "   isAttributeBound(2):"
//                          << _Base::isAttributeBound( rdn, createIfNotThere )
//                          << endl;
                    return _Base::isAttributeBound( rdn, createIfNotThere );
                }
    
            
        };
        
        DatabaseResultSetContext::DatabaseResultSetContext(
            Context* parent,
            const std::string& rdn )
            :
            _Base( parent, rdn )
        {
            m_overMountAttemptHasAlreadyFailed = true;
            createStateLessAttributes();
        }
        
        DatabaseResultSetContext::~DatabaseResultSetContext()
        {
        }

        fh_stringstream
        DatabaseResultSetContext::SL_getSizeStream( DatabaseResultSetContext* c,
                                                    const std::string& rdn,
                                                    EA_Atom* atom )
        {
            fh_stringstream ss;
            m_md_t::const_iterator mdi = c->m_md.find( rdn );
            if( mdi != c->m_md.end() )
            {
                ss << mdi->second;
            }
            else
            {
                ss << 0;
            }
            
            return ss;
        }

        fh_stringstream
        DatabaseResultSetContext::SL_getFerrisType( DatabaseResultSetContext* c,
                                                    const std::string& rdn,
                                                    EA_Atom* atom )
        {
            fh_stringstream ss;
            ss << "special-file";
            return ss;
        }

        fh_stringstream
        DatabaseResultSetContext::SL_getDelegateName( DatabaseResultSetContext* c,
                                                    const std::string& rdn,
                                                    EA_Atom* atom )
        {
            fh_stringstream ss;
            string t = c->getDirName();
            int slashpos = t.rfind("/");
            if( slashpos != string::npos )
                ss << t.substr( slashpos+1 );
            else
                ss << t;
            return ss;
        }

        void
        DatabaseResultSetContext::createStateLessAttributes( bool force )
        {
            static Util::SingleShot virgin;
            if( virgin() )
            {
#define SLEA  tryAddStateLessAttribute         
                SLEA( "size",        _Self::SL_getSizeStream, FXD_FILESIZE );
                SLEA( "ferris-type", _Self::SL_getFerrisType, XSD_BASIC_STRING );
                SLEA( "delegate-name", _Self::SL_getDelegateName, XSD_BASIC_STRING );


                SLEA( "fisheye-label",     _Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "added-attributes",  _Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "formal-attributes", _Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "id",                _Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "added-attributes-relative-to-parent", _Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "added-attributes-relative-to-parent-label", _Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "name-label",        _Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "extent-size",       _Self::SL_getStreamWithNumberOneStream, XSD_BASIC_INT );

                _Base::createStateLessAttributes( true );
                supplementStateLessAttributes( true );
            }
        }
        

        std::string
        DatabaseResultSetContext::private_getStrAttr( const std::string& rdn,
                                                      const std::string& def,
                                                      bool getAllLines,
                                                      bool throwEx )
        {

//             LG_FCA_D << "DatabaseResultSetContext::private_getStrAttr() rdn:" << rdn
//                      << " found:" << (mdi != m_md.end())
//                      << endl;

            if( rdn == "ferris-type" )
            {
                return "special-file";
            }

            // FIXME: we need a typemap here using on the basetable's values.
            if( starts_with( rdn, "schema:" ) )
            {
                if( ConceptContext* cc = dynamic_cast<ConceptContext*>(getParent()))
                {
                    string eaname = rdn.substr( strlen("schema:") );
                    ConceptContext::m_ViewOfExtentSchemas_t::const_iterator sci =
                        cc->m_ViewOfExtentSchemas.find( eaname );
                    if( sci != cc->m_ViewOfExtentSchemas.end() )
                    {
                        return sci->second;
                    }
                }
                return "schema://xsd/attributes/string";
            }
            
            
            if( rdn == "ea-names" )
            {
                stringset_t all;
                stringstream ss;

//                 copy( map_domain_iterator(m_md.begin()),
//                       map_domain_iterator(m_md.end()),
//                       inserter( all, all.end() ));
                m_md_t::const_iterator mdi_end = m_md.end();
                for( m_md_t::const_iterator mdi = m_md.begin();
                     mdi != mdi_end; ++mdi )
                {
                    all.insert( mdi->first );
                }
                
                
                AttributeCollection::AttributeNames_t an;
                getAttributeNames( an );
                for( AttributeCollection::AttributeNames_t::const_iterator ai = an.begin();
                     ai != an.end(); ++ai )
                {
                    all.insert( *ai );
                }
                
//                 Util::parseCommaSeperatedList(
//                     _Base::private_getStrAttr( rdn, def, getAllLines, throwEx ),
//                     all );

                return Util::createSeperatedList( all );
            }
            
            
            static stringset_t skipAttrs;
            if( skipAttrs.empty() )
            {
                skipAttrs.insert( "recommended-ea" );
            }
            if( skipAttrs.find( rdn ) != skipAttrs.end()
                || starts_with( rdn, "schema:" ) )
                return _Base::private_getStrAttr( rdn, def, getAllLines, throwEx );
            
            
            m_md_t::const_iterator mdi = m_md.find( rdn );
            if( mdi != m_md.end() )
            {
//                cerr << "found rdn:" << rdn << " value:" << mdi->second << endl;
                
                return mdi->second;
            }

//            cerr << "DatabaseResultSetContext attribute not bound:" << rdn << endl;
            return _Base::private_getStrAttr( rdn, def, getAllLines, throwEx );
            
            
//             if( throwEx )
//             {
//                 stringstream ss;
//                 ss << "DatabaseResultSetContext attribute not bound:" << rdn << endl;
//                 cerr << ss.str() << endl;
//                 cerr << "priv_isBound:" << AttributeCollection::isAttributeBound( rdn, false ) << endl;
//                 Throw_NoSuchAttribute( ss.str(), this );
//             }
//             return def;
        }
        
        void
        DatabaseResultSetContext::constructObject( ConceptContext* cc, result::const_iterator& iter )
        {
            static int VIRG = true;

            ConceptContext::m_ViewOfExtentSchemas_t& m = cc->m_ViewOfExtentSchemas;

            // The mdi 'hint' only works if the SQL query selects in order.
//            m_md_t::iterator mdi = m_md.end();
            result::tuple::const_iterator e = iter->end();
            for( result::tuple::const_iterator cur = iter->begin();
                 cur != e; ++cur )
            {
                const char* name = cur->name();

//                mdi = m_md.insert( mdi, make_pair( name, cur->c_str() ) );
                m_md.insert( make_pair( name, cur->c_str() ) );
//                m_md.insert( make_pair( name, v ));
//                m_md[ name ] = v;
                if( !strcmp( name, "url") )
                {
                    string v;
                    cur->to( v );
                    m_md[ "realpath" ] = v;
                    m_md[ "name" ] = v;
                }

//                 if( VIRG )
//                 {
//                     const oid ftype = cur->type();
//                     cerr << "name:" << name
//                          << " ftype:" << ftype << " ftype.str:" << to_string( ftype )
//                          << endl;
//                 }
                
                
//                 if( VIRG )
//                     cerr << "k:" << name << " v:" << v << endl;
            }

//             if( VIRG )
//             {
//                 VIRG = false;
//                 string rea = getRecommendedEA();
//                 cerr << "rea:" << rea << endl;

//                 cerr << " width by lookup:" << getImageWidth() << endl;
//                 cerr << " ea-names:" << getStrAttr( this, "ea-names", "N/A" ) << endl;
//             }

            VIRG = false;
            
        }
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        ConceptContext::ConceptContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_concept( 0 ),
            m_extentViewer( false ),
            m_contingentViewer( false ),
            m_ViewOfExtentRecommendedEA( "name" ),
            m_extentSizeLimit( EXTENT_VIEWER_DEFAULT_MAX_FILES )
        {
            this->createStateLessAttributes();
        }

        ConceptContext::~ConceptContext()
        {
        }

        void
        ConceptContext::setConcept( fh_concept& v )
        {
            LG_FCA_D << "ConceptContext::setConcept() v:" << v->getID() << endl;
            m_concept = v;
        }

        void
        ConceptContext::isExtentViewer( bool v )
        {
            m_extentViewer = v;
        }

        void
        ConceptContext::isContingentViewer( bool v )
        {
            m_contingentViewer = v;
        }
        
        fh_iostream
        ConceptContext::getEAStream(
            Context* c, const std::string& rdn, EA_Atom* atom )
        {
                fh_stringstream ss;
                ss << "Attempt to get a stream from a"
                   << " getEAStream() call on "
                   << " a ConceptContext. Please report this bug."
                   << " to create the ea:" << rdn
                   << " output."
                   << " path:" << this->getDirPath()
                   << std::endl;
                std::cerr << tostr(ss) << std::endl;
                Throw_CanNotGetStream( tostr(ss), c );
            
        }
            
        void
        ConceptContext::setEAStream(
            Context* c, const std::string& rdn, EA_Atom* atom, fh_istream )
        {
        }

        LatticeRootConcept*
        ConceptContext::getLatticeRootConcept()
        {
            ConceptContext* bc = getBaseContext();
            LatticeRootConcept* bp = dynamic_cast<LatticeRootConcept*>( bc->getParent() );
            return bp;
        }
        
        
        fh_conceptLattice
        ConceptContext::getLattice()
        {
            return getLatticeRootConcept()->getLattice();
        }

        
        fh_concept
        ConceptContext::getConcept()
        {
            return m_concept;
        }

        static string createRDN( fh_conceptLattice cl, fh_concept parent, fh_concept child )
        {
            string ret = tostr( child->getID() );
            
            stringlist_t addedAttrs;
            addedAttrs = cl->getAddedAttributes( parent, child );
            string t = Util::createSeperatedList( addedAttrs );

//            cerr << "Added attrs for id:" << child->getID() << " t->:" <<  t << ":<-" << endl;
            
            if( !t.empty() )
                return t;

            return ret;
        }

        string
        ConceptContext::getBaseTableName()
        {
            return getLatticeRootConcept()->getBaseTableName();
        }
        
        void
        ConceptContext::priv_read_files()
        {
            if( !getIsViewOfExtent() )
                return;

            //
            // setup m_ViewOfExtentRecommendedEA
            //
            {
                stringstream ss;
                ss << "name,size,mtime,";
                Context* c = this;
                while( !dynamic_cast<LatticeRootConcept*>( c ) )
                {
                    string rdn = c->getDirName();
                    if( rdn != EXTENT_SUBDIR_NAME &&
                        rdn != CONTINGENT_SUBDIR_NAME &&
                        rdn != CONTINGENT_SAMPLE_SUBDIR_NAME &&
                        rdn != EXTENT_SAMPLE_SUBDIR_NAME )
                    {
                        ss << rdn << ",";
                    }
                    c = c->getParent();
                }
                m_ViewOfExtentRecommendedEA = ss.str();
//                cerr << "m_ViewOfExtentRecommendedEA:" << m_ViewOfExtentRecommendedEA << endl;
            }

            
            fh_conceptLattice cl = getLattice();
            stringlist_t itemNames;
            cl->AttrIDToStringList( m_concept->getItemSet(), itemNames );
            
            stringstream sqlss;
//            sqlss << "select * from " <<  getBaseTableName() << " "
//                  << " where ";
//            bool v=true;

//             sqlss << "select * from docmap, " <<  getBaseTableName() << " d "
//                   << " where docmap.docid = d.docid and d.docid in (";
//             {
//                 FerrisBitMagic::bvector<>& bv = m_extentViewer
//                     ? m_concept->m_MatchingDocIDs : m_concept->m_conceptOnlyMatchingDocIDs;

//                 FerrisBitMagic::bvector<>::enumerator an     = bv.first();
//                 FerrisBitMagic::bvector<>::enumerator an_end = bv.end();
//                 for( bool virgin = true; an < an_end ; virgin=false )
//                 {
//                     if( !virgin )
//                         sqlss << ',';
//                     sqlss << *an;
//                     ++an;  // Fastest way to increment enumerator
//                 }
//             }
//             sqlss << ");";
            
            
            sqlss << "select * from docmap,urlmap um, " <<  getBaseTableName() << " d "
                  << " where docmap.docid = d.docid "
                  << " and       um.urlid = d.urlid ";
            string btab = getBaseTableName();
            bool v=false;
            for( stringlist_t::const_iterator si = itemNames.begin();
                 si != itemNames.end(); ++si )
            {
                if( v ) v = false;
                else    sqlss << " and ";
                sqlss << " " << getBitFunctionName(btab,*si) << "(d.bitf) ";
//                sqlss << " " << getBaseTableName() << ".\"" << *si << "\"='t' ";
            }
            if( !m_extentViewer )
            {
                stringset_t intent;
                copy( itemNames.begin(), itemNames.end(), inserter( intent, intent.end() ));
                stringlist_t sl;
                cl->getAllAttributeNames( sl );
                for( stringlist_t::const_iterator si = sl.begin(); si!=sl.end(); ++si )
                {
                    if( intent.find( *si ) == intent.end() )
                    {
                        sqlss << " and not " << getBitFunctionName(btab,*si) << "(d.bitf) ";
//                         sqlss << " and ";
//                         sqlss << " " << getBaseTableName() << ".\"" << *si << "\"<>'t' ";
                    }
                }
            }
            if( m_extentSizeLimit )
            {
                sqlss << " limit " << m_extentSizeLimit << endl;
            }
            sqlss << " ;";

            LG_FCA_D << "SQL:" << sqlss.str() << endl;

            connection& con = *(getLatticeRootConcept()->db_connection);
            work trans( con, "getting the extent..." );
            result res = trans.exec( sqlss.str() );


            //
            // setup m_ViewOfExtentSchemas
            //
            {
                m_ViewOfExtentSchemas.clear();
                m_ViewOfExtentSchemas[ "name" ]  = "schema://xsd/attributes/string";
                m_ViewOfExtentSchemas[ "size" ]  = "schema://xsd/attributes/decimal/integer";
                m_ViewOfExtentSchemas[ "mtime" ] = "schema://xsd/attributes/decimal/integer/long/fs/time";
                m_ViewOfExtentSchemas[ "atime" ] = "schema://xsd/attributes/decimal/integer/long/fs/time";
                m_ViewOfExtentSchemas[ "ctime" ] = "schema://xsd/attributes/decimal/integer/long/fs/time";
                if( !res.empty() )
                {
                    result::const_iterator ri = res.begin();
                    result::tuple::const_iterator e = ri->end();
                    for( result::tuple::const_iterator cur = ri->begin(); cur != e; ++cur )
                    {
                        string       name = cur->name();
                        const oid coltype = cur->type();
//                         string v;
//                         cur->to( v );

                        // server/catalog/pg_type.h
                        switch( coltype )
                        {
                        case BOOLOID:
                            m_ViewOfExtentSchemas[ name ] = "schema://xsd/attributes/boolean";
                            break;
                        case INT8OID:
                        case INT2OID:
                        case INT4OID:
                            m_ViewOfExtentSchemas[ name ] = "schema://xsd/attributes/decimal/integer";
                            break;
                        case TIMESTAMPOID:
                            m_ViewOfExtentSchemas[ name ]
                                = "schema://xsd/attributes/decimal/integer/long/fs/time";
                            break;
                        case VARCHAROID:
                            m_ViewOfExtentSchemas[ name ] = "schema://xsd/attributes/string";
                            break;
                        default:
                            m_ViewOfExtentSchemas[ name ] = "schema://xsd/attributes/string";
                        }
                    }
                }
            }
            
            LG_FCA_D << "files in extent:" << res.size() << endl;
            int fileCount = 0;
            for( result::const_iterator iter = res.begin();
                 iter != res.end(); ++iter )
            {
                string url = "";
                iter["url"].to(url);
                LG_FCA_D << "url  " << url << endl;
                
                string rdn = url;

                DatabaseResultSetContext* child = 0;
                child = priv_ensureSubContext( rdn, child );
                child->constructObject( this, iter );
                
                ++fileCount;
                if( fileCount >= m_extentSizeLimit )
                {
                    break;
                }
            }
        }
        
        fh_context
        ConceptContext::priv_getSubContext( const string& rdn )
            throw( NoSuchSubContext )
        {
            try
            {
                fh_conceptLattice cl = getLattice();

                if( getIsViewOfExtent() )
                {
                    Items_t::iterator subc_iter;
                    if( priv_isSubContextBound( rdn, subc_iter ) )
                    {
                        return *subc_iter;
                    }
//                     if( priv_isSubContextBound( rdn ) )
//                         return _Base::priv_getSubContext( rdn );
                }
                
                fh_concept parent = getConcept();
                clist_t ch = parent->getChildren();

                for( clist_t::const_iterator ci = ch.begin(); ci != ch.end(); ++ci )
                {
                    fh_concept c = *ci;
                    string child_rdn = createRDN( cl, parent, c );
                    if( child_rdn == rdn )
                    {
                        if( priv_isSubContextBound( rdn ) )
                            return _Base::priv_getSubContext( rdn );
                        
                        ConceptContext* child = new ConceptContext( this, rdn );
                        child->setConcept( c );
                        Insert( child );
                        return child;
                    }
                }

                createExtentViewerSubDir();
                
                return _Base::priv_getSubContext( rdn );
            }
            catch( NoSuchSubContext& e )
            {
                throw e;
            }
            catch( exception& e )
            {
                string s = e.what();
                Throw_NoSuchSubContext( s, this );
            }
            catch(...)
            {}
            fh_stringstream ss;
            ss << "NoSuchSubContext:" << rdn;
            Throw_NoSuchSubContext( tostr(ss), this );
        }

        void
        ConceptContext::priv_createExtentViewerSubDir( const char* name, const int limit )
        {
            ConceptContext* child = 0;
            child = priv_ensureSubContext( name, child );
            child->setConcept( m_concept );
            child->isExtentViewer( true );
            child->setExtentSizeLimit( limit );
        }

        int
        ConceptContext::getStrSubCtxInt( const std::string& k, int def )
        {
            return getLattice()->getStrSubCtxInt( k, def );
        }
        
        
        void
        ConceptContext::createExtentViewerSubDir()
        {
            bool isViewOfExtent = getIsViewOfExtent();
            if( isViewOfExtent )
                return;

//             priv_createExtentViewerSubDir( EXTENT_SUBDIR_NAME, EXTENT_SUBDIR_LIMIT );
//             priv_createExtentViewerSubDir( EXTENT_SAMPLE_SUBDIR_NAME, EXTENT_SAMPLE_SUBDIR_LIMIT );
//             priv_createExtentViewerSubDir( CONTINGENT_SUBDIR_NAME, CONTINGENT_SUBDIR_LIMIT );
//             priv_createExtentViewerSubDir( CONTINGENT_SAMPLE_SUBDIR_NAME, CONTINGENT_SAMPLE_SUBDIR_LIMIT );

            priv_createExtentViewerSubDir(
                EXTENT_SUBDIR_NAME,
                getStrSubCtxInt( "extent-subdir-limit", EXTENT_SUBDIR_LIMIT));
            priv_createExtentViewerSubDir(
                EXTENT_SAMPLE_SUBDIR_NAME,
                getStrSubCtxInt( "extent-sample-subdir-limit", EXTENT_SAMPLE_SUBDIR_LIMIT ));
            priv_createExtentViewerSubDir(
                CONTINGENT_SUBDIR_NAME,
                getStrSubCtxInt( "contingent-subdir-limit", CONTINGENT_SUBDIR_LIMIT ));
            priv_createExtentViewerSubDir(
                CONTINGENT_SAMPLE_SUBDIR_NAME,
                getStrSubCtxInt( "contingent-sample-subdir-limit", CONTINGENT_SAMPLE_SUBDIR_LIMIT ));

        }
        
        
        void
        ConceptContext::priv_read()
        {
            EnsureStartStopReadingIsFiredRAII _raii1( this );
            
            LG_FCA_D << "priv_read() url:" << getURL()
                     << " #sc:" << getSubContextCount() << endl;

            clearContext();
            fh_conceptLattice cl = getLattice();
            bool isViewOfExtent = getIsViewOfExtent();

            if( !isViewOfExtent )
            {
                fh_concept parent = getConcept();
                clist_t ch = parent->getChildren();

                LG_FCA_D << "priv_read() parent:" << parent->getID()
                         << " childdirs:" << ch.size()
                         << endl;
                
                for( clist_t::const_iterator ci = ch.begin(); ci != ch.end(); ++ci )
                {
                    fh_concept c = *ci;
                    string rdn = createRDN( cl, parent, c );
                        
//                     if( priv_isSubContextBound( rdn ) )
//                         continue;

                    ConceptContext* child = 0;
                    child = priv_ensureSubContext( rdn, child );
                    child->setConcept( c );
                }
            }
                
            createExtentViewerSubDir();
            priv_read_files();
        }

        long
        ConceptContext::priv_guessSize() throw()
        {
            return getConcept()->getExtentSize();
        }
        
        
        std::string
        ConceptContext::getRecommendedEA()
        {
//             if( getIsViewOfExtent() )
//                 return _Base::getRecommendedEA();
            return _Base::getRecommendedEA() + ",extent-size";
        }
        
        fh_stringstream
        ConceptContext::SL_getExtentSize( ConceptContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;

            fh_concept ivc = c->getConcept();
            
            if( !c->m_contingentViewer )
            {
                ss << ivc->getExtentSize();
            }
            else
            {
                ss << ivc->getConceptOnlyMatchSize();
            }
            
            return ss;
        }

        fh_stringstream
        ConceptContext::SL_getID( ConceptContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            fh_concept ivc = c->getConcept();
            ss << ivc->getID();
            return ss;
        }
        
        fh_stringstream
        ConceptContext::SL_getExtentViewerRecommendedEA(
            ConceptContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            ss << c->m_ViewOfExtentRecommendedEA;
            return ss;
        }

        fh_stringstream
        ConceptContext::SL_getFormalAttributes( ConceptContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            fh_concept ivc = c->getConcept();

            clist_t intent = ivc->getIntent();
//             cerr << "ConceptContext::SL_getFormalAttributes() url:" << c->getURL() << endl;
//             cerr << "ConceptContext::SL_getFormalAttributes() intent.sz:" << intent.size() << endl;
//             cerr << "ConceptContext::SL_getFormalAttributes() upset.sz:" << ivc->getUpSet().size() << endl;

            intent = ivc->getUpSet();
            
            stringlist_t sl;
            for( clist_t::iterator iter = intent.begin(); iter!=intent.end(); ++iter )
            {
                (*iter)->getAddedFormalConceptAttributes( sl );
            }
            ss << Util::createCommaSeperatedList( sl );
            return ss;
        }

        fh_stringstream
        ConceptContext::SL_getAddedAttributes( ConceptContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            fh_concept ivc = c->getConcept();

            stringlist_t sl;
            ivc->getAddedFormalConceptAttributes( sl );
            ss << Util::createCommaSeperatedList( sl );
            return ss;
        }

        fh_stringstream
        ConceptContext::SL_getAddedAttributesRelativeToParent( ConceptContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;

            if( !c->isParentBound() )
                return ss;

            fh_concept ivc = c->getConcept();
            ConceptContext* p = dynamic_cast<ConceptContext*>(c->getParent());
            fh_concept pivc = p->getConcept();
            fh_conceptLattice cl = ivc->getConceptLattice();
            
            stringlist_t sl;
            ivc->getAddedFormalConceptAttributesRelativeToParent( pivc, sl );
            cl->makeReducedAttributes( sl );
            ss << Util::createCommaSeperatedList( sl );
            return ss;
        }

        fh_stringstream
        ConceptContext::SL_getAddedAttributesRelativeToParentLabel( ConceptContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;

            if( !c->isParentBound() )
                return ss;

            fh_concept ivc = c->getConcept();
            ConceptContext* p = dynamic_cast<ConceptContext*>(c->getParent());
            if( !p )
                return ss;
            fh_concept pivc = p->getConcept();
            fh_conceptLattice cl = ivc->getConceptLattice();
            
            stringlist_t sl;
            ivc->getAddedFormalConceptAttributesRelativeToParent( pivc, sl );
            cl->makeReducedAttributes( sl );
            cl->convertAttributeNamesToLabels( sl );
            ss << Util::createSeperatedList( sl, ", " );
            return ss;
        }
        
        
        fh_stringstream
        ConceptContext::SL_getFishEyeLabel( ConceptContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            fh_concept ivc = c->getConcept();
            stringlist_t sl;

            ivc->getFishEyeLabel( sl );
            ss << Util::createSeperatedList( sl, ", " );
            return ss;
        }
        
        void
        ConceptContext::createStateLessAttributes( bool force )
        {
            if( force || isStateLessEAVirgin() )
            {
                tryAddStateLessAttribute( "extent-size",
                                          SL_getExtentSize,
                                          XSD_BASIC_INT );
                tryAddStateLessAttribute( "id", SL_getID, XSD_BASIC_INT );
                tryAddStateLessAttribute( "extent-viewer-recommended-ea",
                                          SL_getExtentViewerRecommendedEA,
                                          FXD_STRINGLIST );

                tryAddStateLessAttribute( "formal-attributes",
                                          SL_getFormalAttributes,
                                          FXD_STRINGLIST );
                tryAddStateLessAttribute( "added-attributes",
                                          SL_getAddedAttributes,
                                          FXD_STRINGLIST );

                tryAddStateLessAttribute( "fisheye-label",
                                          SL_getFishEyeLabel,
                                          FXD_STRINGLIST );

                tryAddStateLessAttribute( "added-attributes-relative-to-parent",
                                          SL_getAddedAttributesRelativeToParent,
                                          FXD_STRINGLIST );
                tryAddStateLessAttribute( "added-attributes-relative-to-parent-label",
                                          SL_getAddedAttributesRelativeToParentLabel,
                                          FXD_STRINGLIST );
                tryAddStateLessAttribute( "name-label",
                                          SL_getAddedAttributesRelativeToParentLabel,
                                          FXD_STRINGLIST );
                
                
                _Base::createStateLessAttributes( true );
            }
        }
        
        
        /************************************************************/
        /************************************************************/
        /************************************************************/

        
        LatticeRootConcept::LatticeRootConcept(
            Context* parent,
            const std::string& rdn,
            fh_context md )
            :
            _Base( parent, rdn ),
            m_metadata( 0 ),
            m_conceptLattice( 0 ),
            m_user(""),
            m_port(""),
            m_host("localhost"),
            m_dbname(""),
            db_connection(0)
        {
            this->createStateLessAttributes();

            if( parent )
                m_dbname = parent->getDirName();
            if( md )
            {
                m_metadata = md;
                m_CFITableName  = getStrSubCtx( md, "tablename-cfi",  "", true, true );
                m_findexPath    = getStrSubCtx( md, "findex-path",    "", true, true );
            }
            
            
        }

        LatticeRootConcept::~LatticeRootConcept()
        {
        }
            
        fh_iostream
        LatticeRootConcept::getEAStream(
            Context* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            ss << "Attempt to get a stream from a"
               << " getEAStream() call on "
               << " a LatticeRootConcept. Please report this bug."
               << " to create the ea:" << rdn
               << " output."
               << " path:" << this->getDirPath()
               << std::endl;
            std::cerr << tostr(ss) << std::endl;
            Throw_CanNotGetStream( tostr(ss), c );
        }
            
        void
        LatticeRootConcept::setEAStream(
            Context* c, const std::string& rdn, EA_Atom* atom, fh_istream )
        {
        }

        void
        LatticeRootConcept::ensureDatabaseConnection()
        {
            if( db_connection )
                return;
            
            stringstream ss;
            string user = "";
            string port = "";
      
            if( !m_user.empty() )
                ss << " user=" << m_user;
            if( !m_host.empty() )
                ss << " host=" << m_host;
            if( !m_port.empty() )
                ss << " port=" << m_port;
            ss << " dbname=" << m_dbname;

            string constring = ss.str();

            db_connection = new connection( constring );
  
            if( !db_connection )
            {
                fh_stringstream ss;
                ss << "Faield to connect to backend database for dbname:" << m_dbname
                   << " constring:" << constring
//                   << " error:" << e.what()
                   << " path:" << this->getDirPath()
                   << std::endl;
                std::cerr << tostr(ss) << std::endl;
                Throw_RootContextCreationFailed( tostr(ss), 0 );
            }
        }

        void
        LatticeRootConcept::priv_read()
        {
            EnsureStartStopReadingIsFiredRAII _raii1( this );
            clearContext();

            LG_FCA_D << "priv_read() #sc:" << getSubContextCount() << endl;

            ensureDatabaseConnection();

            LG_FCA_D << "getting the data..." << endl;
            
            m_conceptLattice = ConceptLattice::load( m_metadata->getURL() );
            fh_concept top = m_conceptLattice->getTopConcept();
            fh_concept bot = m_conceptLattice->getBottomConcept();
            
            LG_FCA_D << "have-top:" << isBound(top) << endl;
            LG_FCA_D << "have-bot:" << isBound(bot) << endl;
            LG_FCA_D << "top:" << top->getID() << " bot:" << bot->getID() << endl;
            
            clist_t ch = top->getChildren();
            for( clist_t::const_iterator ci = ch.begin(); ci != ch.end(); ++ci )
            {
                fh_concept c = *ci;
                string rdn = createRDN( m_conceptLattice, top, c );
                ConceptContext * child = 0;
                child = priv_ensureSubContext( rdn, child );
                child->setConcept( c );
            }
        }

        
        fh_conceptLattice
        LatticeRootConcept::getLattice()
        {
            return m_conceptLattice;
        }
        
        string
        LatticeRootConcept::getBaseTableName()
        {
            return m_conceptLattice->getBaseTableName();
        }

        

        /************************************************************/
        /************************************************************/
        /************************************************************/

        static fh_istream
        GetExtentSizeFromSubContextCount( Context* c, 
                                          const std::string& rdn,
                                          EA_Atom* atom )
        {
            fh_stringstream ss;
            ss << getStrAttr( c, "subcontext-count", "1" );
            return ss;
        }
        
        
        class FERRISEXP_DLLLOCAL LatticeRootConceptVFS_RootContextDropper
            : public RootContextDropper
        {
        public:
            LatticeRootConceptVFS_RootContextDropper()
                {
                    ImplementationDetail::appendToStaticLinkedRootContextNames("fca");
                    RootContextFactory::Register("fca", this);
                }
            virtual fh_context Brew(RootContextFactory* rf)
                throw( RootContextCreationFailed )
                {
                    try
                    {
                        static LatticeRootConcept raw_obj;
            
                        Context* ctx = raw_obj.CreateContext( 0, "/");

                        ctx->addAttribute( "extent-size",
                                           &GetExtentSizeFromSubContextCount,
                                           XSD_BASIC_INT, true );
                    
                        fh_context lc = Shell::acquireContext( "~/.ferris/fcatree/localhost" );
                        FakeInternalContext* our_lc = new FakeInternalContext( ctx, "localhost" );
                        our_lc->addAttribute( "extent-size",
                                              &GetExtentSizeFromSubContextCount,
                                              XSD_BASIC_INT, true );
                        ctx->Insert( our_lc );

                        for( Context::iterator dbi = lc->begin(); dbi != lc->end(); ++dbi )
                        {
                            fh_context dbchild = *dbi;
                            if( isFalse( getStrAttr( dbchild, "is-dir", "0" ) ) )
                                continue;
                            
                            FakeInternalContext* our_dbchild = new FakeInternalContext(
                                our_lc, dbchild->getDirName() );
                            our_lc->Insert( our_dbchild );

                            for( Context::iterator viewiter = dbchild->begin();
                                 viewiter != dbchild->end(); ++viewiter )
                            {
                                fh_context viewchild = *viewiter;

                                if( !viewchild->isSubContextBound("tablename-cfi")
                                    || !viewchild->isSubContextBound("findex-path")
                                    )
                                {
                                    continue;
                                }
                            
                                Context* our_viewchild = new LatticeRootConcept(
                                    our_dbchild, viewchild->getDirName(), viewchild );
                                our_dbchild->Insert( our_viewchild );
                            }
                        }
                    
                    
                    
                        return ctx;
                    }
                    catch( exception& e )
                    {
                        cerr << e.what() << endl;
                        LG_CTX_W << e.what() << endl;
                        throw e;
                    }
                }
            
        };

        static LatticeRootConceptVFS_RootContextDropper ___LatticeRootConceptVFS_static_init;

                
#endif        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        template < class Col, class Obj >
        bool contains( Col& col, Obj& o )
        {
            return col.end() != find( col.begin(), col.end(), o );
        }


        bool filter_compare( filters_t la, filters_t lb )
        {
            int lasz = la.size();
            int lbsz = lb.size();

            if( lasz != lbsz ) return false;

            la.sort();
            lb.sort();
        
            list< fh_context >::const_iterator ai = la.begin();
            list< fh_context >::const_iterator bi = lb.begin();
        
            for( ; bi != lb.end() && ai != la.end(); ++ai, ++bi )
            {
                if( GetImpl( *ai ) == GetImpl( *bi ) )
                    continue;
                return false;
            }
            return true;
        }
    
        struct SmallerIntentFirst
            : public binary_function<const list< fh_context >&, const list< fh_context >&, bool>
        {
            bool operator()(const list< fh_context >& x, const list< fh_context >& y)
                { return x.size() < y.size(); }
        };

        static filters_t filter_intersection( filters_t::iterator b1, filters_t::iterator e1,
                                              filters_t::iterator b2, filters_t::iterator e2 )
        {
            filters_t ret;

            typedef list< Context* > cplist_t;
            cplist_t col1, col2, t;

            for( filters_t::iterator i = b1; i != e1; ++i )
                col1.push_back( GetImpl( *i ) );
                            
            for( filters_t::iterator i = b2; i != e2; ++i )
                col2.push_back( GetImpl( *i ) );

            set_intersection( col1.begin(), col1.end(),
                              col2.begin(), col2.end(),
                              back_inserter( t ));

            for( cplist_t::iterator i = t.begin(); i != t.end(); ++i )
                ret.push_back( *i );
        
            return ret;
        }

        float
        LatticeLayout::getMagnificationFactor()
        {
            return 4.0;
        }
    

        void
        LatticeLayout::assignPointToAll( fh_conceptLattice l, clist_t::iterator begin, clist_t::iterator end, const Point& p )
        {
            for( clist_t::iterator iter = begin; iter != end; ++iter )
            {
                (*iter)->setPoint( p );
            }
        }

        void
        LatticeLayout::assignPointToAll( fh_conceptLattice l, const Point& p )
        {
            typedef std::list< fh_concept > L;
            L concepts;
            l->getConcepts( concepts );

            for( L::const_iterator ci = concepts.begin(); ci!=concepts.end(); ++ci )
            {
                fh_concept c = *ci;
                c->setPoint( p );
            }
        }
    
    
    
        void
        LatticeLayout::assignZeroToRoot( fh_conceptLattice l )
        {
            fh_concept root = l->getTopConcept();
            root->setLocation( 0, 0, 0 );
        }



        void
        LatticeLayout::dump( fh_conceptLattice l )
        {
            typedef std::list< fh_concept > L;
            L concepts;
            l->getConcepts( concepts );

            ////////////////
            // PURE DEBUG //
            cerr << "LatticeLayout::assignVectorToAttributeVector(dump start) " << endl;
            for( L::const_iterator ci = concepts.begin(); ci!=concepts.end(); ++ci )
            {
                fh_concept c = *ci;

                Point      p  = c->getPoint();
                cerr << " p:" << p
                     << "   id:" << c->getID() 
                     << "   fname:" << c->getFancyName() 
                     << endl;
            }
        
            cerr << "LatticeLayout::assignVectorToAttributeVector(dump end) " << endl;
        }
    
    
    
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

    
        void
        LatticeLayout_Additive::assignVectorToMI( fh_conceptLattice l,
                                                  float ix, float iy, float iz,
                                                  float dx, float dy, float dz )
        {
            float x = ix;
            float y = iy;
            float z = iz;
//        int currentDepth = 1;

            typedef map< int, int > levelCount_t;
            levelCount_t levelCount;

            typedef map< int, list< Point > > levelPoints_t;
            levelPoints_t levelPoints;
        
            typedef std::list< fh_concept > L;
            L concepts;
            l->getConcepts( concepts );
//            cerr << "LatticeLayout::assignVectorToAttributeVector concepts.sz:" << concepts.size() << endl;

            for( L::iterator ci = concepts.begin(); ci != concepts.end(); ++ci )
            {
                fh_concept c = *ci;
                if( c->isMeetIrreducible() )
                {
                    levelCount[ c->getMaxDepth() ]++;
                }
            }

            int reverse = 1;
            float magnificationFactor = getMagnificationFactor();
            for( levelCount_t::iterator li = levelCount.begin(); li != levelCount.end(); ++li )
            {
                int level = li->first;
            
//            float x = x = -1 * magnificationFactor * (li->second/2);
                for( int i=0; i<li->second; ++i )
                {
                    levelPoints[ level ].push_back( Point( x, y, z ) );
                    if( x < 0 )
                    {
                        x *= -1;
//                    dx *= 1.2;
                    }
                
                    x += dx;
                    x *= reverse;
                    reverse *= -1;
//                 if( x > -magnificationFactor && x < magnificationFactor )
//                     x = magnificationFactor;
                }
                li->second = 0;
                y += dy;
                z += dz;
            }
        
        
            for( L::iterator ci = concepts.begin(); ci != concepts.end(); ++ci )
            {
                fh_concept c = *ci;
                if( c->isMeetIrreducible() )
                {
//                     cerr << "-------------------------------------" << endl;
//                     cerr << "assignVectorToAttributeVector(MIR) id:" << c->getID() << endl;

                    int depth = c->getMaxDepth();
                    list< Point >::iterator piter = levelPoints[ depth ].begin();
                    advance( piter, levelCount[ depth ] );
                    levelCount[ depth ] = levelCount[ depth ]+1;
                    Point p = *piter;

                    const clist_t& upset = c->getUpSet( false );
                    for( clist_t::const_iterator ui = upset.begin(); ui!=upset.end(); ++ui )
                    {
                        fh_concept uc = *ui;
                        if( uc->isMeetIrreducible() )
                        {
//                            cerr << "assignVectorToAttributeVector(parent MIR) uc id:" << uc->getID() << endl;
                            p+=uc->getPoint();
                        }
                    }
//                     cerr << "assignVectorToAttributeVector(MIR setp) id:" << c->getID()
//                          << " point:" << p
//                          << endl;
                    c->setPoint( p );
//                 x += dx;
//                 y += dy;
//                 z += dz;

//                 int newDepth = c->getMaxDepth();
//                 cerr << "assignVectorToAttributeVector(MIR) id:" << c->getID()
//                      << " currentDepth:" << currentDepth
//                      << " newDepth:" << newDepth
//                      << endl;
//                 if( newDepth > currentDepth )
//                 {
//                     currentDepth = newDepth;
//                     x = 0;
//                 }
                }
            }
        
            return;


        
        
//         typedef Lattice::m_conceptsByIntent_t m_conceptsByIntent_t;
//         typedef Lattice::m_conceptsBySize_t   m_conceptsBySize_t;
//         fh_ivconcept rootivc = l->getRoot();
//         m_conceptsByIntent_t& m_conceptsByIntent = l->m_conceptsByIntent;
//         typedef Lattice::m_baseFilters_t m_baseFilters_t;
//         m_baseFilters_t& m_baseFilters = l->m_baseFilters;

//         float x = ix;
//         float y = iy;
//         float z = iz;

// //         for( m_baseFilters_t::iterator fi = m_baseFilters.begin(); fi != m_baseFilters.end(); ++fi )
// //         {
// //             fh_context c = *fi;
// //             m_attributeVector.insert( make_pair( c, Point( x, y, z ) ) );
// //             x += dx;
// //             y += dy;
// //             z += dz;
// //         }
// //         //
// //         // Assign a vector to each contingent.
// //         //
// //         for( m_conceptsByIntent_t::iterator cbi = m_conceptsByIntent.begin();
// //              cbi != m_conceptsByIntent.end(); ++cbi )
// //         {
// //             fh_ivconcept c  = cbi->second;
// //             filters_t    fl = c->getContingentFilters();

// //             if( !fl.empty() )
// //             {
// //                 cerr << "LatticeLayout::assignVectorToAttributeVector(Assign) "
// //                      << " c:" << c->getFancyName()
// //                      << " fl:" << filterToString( fl )
// //                      << endl;
// //                 m_attributeVector.insert( make_pair( fl, Point( x, y, z ) ) );
// //                 x += dx;
// //                 y += dy;
// //                 z += dz;
// //             }
// //         }


// //         //
// //         // Assign a vector to each contingent.
// //         //
// //         Lattice::m_conceptsInOrder_t& cio = l->getConceptsInOrder();
// //         for( Lattice::m_conceptsInOrder_t::iterator ci = cio.begin(); ci != cio.end(); ++ci )
// //         {
// //             fh_concept c = *ci;
// //             filters_t    fl = c->getContingentFilters();
// //             if( !fl.empty() )
// //             {
// //                 cerr << "LatticeLayout::assignVectorToAttributeVector(Assign) "
// //                      << " c:" << c->getFancyName()
// //                      << " fl:" << filterToString( fl )
// //                      << endl;
// //                 m_attributeVector.insert( make_pair( fl, Point( x, y, z ) ) );
// //                 x += dx;
// //                 y += dy;
// //                 z += dz;
// //             }
// //         }


//         //
//         // Assign a vector to each contingent.
//         //
//         x = 0;
//         bool reverse = false;
//         Lattice::m_conceptsInOrder_t& cio = l->getConceptsInOrder();
//         for( Lattice::m_conceptsInOrder_t::reverse_iterator ci = cio.rbegin(); ci != cio.rend(); ++ci )
//         {
//             fh_concept c  = *ci;
//             filters_t  fl = c->getContingentFilters();
//             if( !fl.empty() )
//             {
//                 cerr << "LatticeLayout::assignVectorToAttributeVector(Assign) "
//                      << " c:" << c->getFancyName()
//                      << " fl:" << filterToString( fl )
//                      << endl;
//                 m_attributeVector.insert( make_pair( fl, Point( x, y, z ) ) );
//                 if( reverse ) x = -x - dx;
//                 else          x = 0 - x + dx;
// //                 x += dx;
//                 y += dy;
//                 z += dz;
//                 reverse = !reverse;
//             }
//         }        

        
// //         //
// //         // Assign a vector to each contingent.
// //         //
// //         m_attributeVector.insert( make_pair( rootivc->getContingentFilters(), Point( x, y, z ) ) );
// //         x += dx;
// //         y += dy;
// //         z += dz;
// //         clist_t children = rootivc->Concept::getChildren();
// //         clist_t left;
// //         clist_t right;
// //         clist_t::iterator middle = children.begin();
// //         advance( middle, children.size() / 2 );
// //         copy( children.begin(), middle,         back_inserter( left  ));
// //         copy( middle,           children.end(), back_inserter( right ));
        
        }
    
        fh_conceptLattice
        LatticeLayout_Additive::layout( fh_conceptLattice l )
        {
//            cerr << "LatticeLayout_Additive::layout()" << endl;
        
            /**
             * Position attribute concepts
             */
            float magnificationFactor = getMagnificationFactor();
            float x = -1;//magnificationFactor;
            float y = -magnificationFactor;
            float z = 0.0;

            assignPointToAll( l, Point( 0, 0, 0 ) );

//        x = -1 * magnificationFactor;
            assignVectorToMI( l, 
                              x, y, z,
                              1.5, -1, 0 );

//         // move root to middle
//         {
//             filters_t rootContFitlers = rootivc->getContingentFilters();
//             m_attributeVector_t::iterator avi = m_attributeVector.find( rootContFitlers );
//             if( avi != m_attributeVector.end() )
//             {
//                 m_attributeVector_t::iterator middle = m_attributeVector.begin();
//                 advance( middle, m_attributeVector.size() / 2 );
//                 swap( middle->second, avi->second );
//             }
//         }
//            dump( l );
        

            typedef std::list< fh_concept > L;
            L concepts;
            l->getConcepts( concepts );
//            cerr << "LatticeLayout::assignVectorToAttributeVector(MR) concepts.sz:" << concepts.size() << endl;
        
            for( L::iterator ci = concepts.begin(); ci != concepts.end(); ++ci )
            {
                fh_concept c = *ci;
                Point additive = c->getPoint();

//             if( c->isMeetIrreducible() )
//                 continue;

//                 cerr << "------------------------------------------------------------------" << endl;
//                 cerr << "LatticeLayout::assignVectorToAttributeVector(MR) concept:" << c->getID() << endl;
            
                const clist_t& upset = c->getUpSet( false );
//            const clist_t& upset = c->getParents();
                for( clist_t::const_iterator ui = upset.begin(); ui!=upset.end(); ++ui )
                {
                    fh_concept uc = *ui;
//                     cerr << "LatticeLayout::assignVectorToAttributeVector(MR) uc:" << uc->getID()
//                          << " point:" << uc->getPoint() << endl;
                    additive+=uc->getPoint();
                }

            
                int depth = c->getMaxDepth();
                if( depth )
                {
                    double ratio = additive.getY();
                    additive.setY( -depth * magnificationFactor );
//                additive.setY( additive.getY() * 1.0/depth );
                    ratio /= additive.getY();

//                if( c->isMeetIrreducible() )
                    additive.setX( additive.getX() / ratio );
                
                }
            
                c->setPoint( additive );
            
            }
//            dump( l );

            return l;
        }
    
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        struct OrdinateMaker
        {
            double v;
            double dv;
            int reverse;
        
            OrdinateMaker()
                {
                    v = -1;
                    dv = 1;
                    reverse = -1;
                }
            double operator()()
                {
                    if( v < 0 )
                    {
                        v *= -1;
                    }
                
                    v += dv;
                    v *= reverse;
                    reverse *= -1;
                    return v;
                }
        };
    
        typedef std::list< fh_concept > L;

        void
        LatticeLayout_Layered::shuffleToX( L& col, fh_concept c, double x )
        {
            float mf = 2.0;
            for( L::iterator li = col.begin(); li!=col.end(); ++li )
            {
                fh_concept lc = *li;
                double lcx = lc->getX();

//             |<--------->|
//                 x  lcx
            
                if( (x-mf/2) < lcx && lcx < (x+mf/2) )
                {
                    shuffleToX( col, lc, x + mf );
                    break;
                }
            }
            c->setX( x );
        }
    
        fh_conceptLattice
        LatticeLayout_Layered::layout( fh_conceptLattice l )
        {
//            cerr << "LatticeLayout_Layered::layout()" << endl;

            float magnificationFactor = getMagnificationFactor();
        
            L concepts;
            l->getConcepts( concepts );

            typedef map< int, L > Layer_t;
            Layer_t Layer;
        
            for( L::iterator ci = concepts.begin(); ci != concepts.end(); ++ci )
            {
                fh_concept c = *ci;
                int depth = c->getMaxDepth();
                c->setPoint( Point( 0, -depth * magnificationFactor, 0.0 ));
                Layer[depth].push_back( c );
            }

            OrdinateMaker om;
            for( Layer_t::iterator li = Layer.begin(); li!=Layer.end(); ++li )
            {
                L& col = li->second;
                for( L::iterator iter = col.begin(); iter != col.end(); ++iter )
                {
                    fh_concept c = *iter;
                    if( c->isMeetIrreducible() )
                    {
                        double x = om();
                    
//                        cerr << "Setting MI concept:" << c->getID() << " to x:" << x << endl;
                        c->setX( x );
                    }
                }
            }

            for( Layer_t::iterator li = Layer.begin(); li!=Layer.end(); ++li )
            {
                L& col = li->second;
                for( L::iterator iter = col.begin(); iter != col.end(); ++iter )
                {
                    fh_concept c = *iter;
                    if( !c->isMeetIrreducible() )
                    {
                        double x = c->getX();
                    
                        const clist_t& upset = c->getUpSet( false );
                        for( clist_t::const_iterator ui = upset.begin(); ui!=upset.end(); ++ui )
                        {
                            fh_concept uc = *ui;
//                             cerr << "LatticeLayout::assignVectorToAttributeVector(MR) uc:" << uc->getID()
//                                  << " point:" << uc->getPoint() << endl;
                            x += uc->getX();
                        }
//                        cerr << "Setting MR concept:" << c->getID() << " to x:" << x << endl;
                        c->setX( x );
                    }
                }
            }

            double min = -30;
            double max =  30;
            for( Layer_t::iterator li = Layer.begin(); li!=Layer.end(); ++li )
            {
                L& col = li->second;
                typedef map< double, double > dmap_t;
                dmap_t dmap;
            
                for( L::iterator iter = col.begin(); iter != col.end(); ++iter )
                {
                    fh_concept c = *iter;

//                     cerr << "-----------------------------------------" << endl;
//                     cerr << "Looking at parents for concept:" << c->getID() <<  endl;
                
                    double x = 0;
                    const clist_t& upset = c->getParents();
                    if( !upset.empty() )
                    {
                        for( clist_t::const_iterator ui = upset.begin(); ui!=upset.end(); ++ui )
                        {
                            fh_concept uc = *ui;
//                            cerr << "Parent concept:" << uc->getID() << " x:" << uc->getX() << endl;
                            x += uc->getX();
                        }
                        x /= upset.size();
                    }
                

//                    cerr << "Moving concept:" << c->getID() << " depth:" << c->getMaxDepth() << " to x:" << x << endl;
                    if( !c->isMeetIrreducible() || x )
                    {
                        shuffleToX( col, c, x );
                    }
                    else
                    {
//                        cerr << "Special moving MI concept:" << c->getID() << " to x:" << x << endl;
                        shuffleToX( col, c, c->getX() );
                    }
                }
            }
        
//            dump( l );
            return l;
        }
    

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
    };
};
