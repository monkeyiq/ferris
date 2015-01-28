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

    $Id: FCA.hh,v 1.21 2010/09/24 21:30:33 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_FCA_H_
#define _ALREADY_INCLUDED_FERRIS_FCA_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisBitMagic.hh>

#include <boost/dynamic_bitset.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>

namespace Ferris
{
    using boost::multi_index_container;
    using namespace boost::multi_index;


    struct from{};
    struct to{};

    template<typename FromType,typename ToType>
    struct bidirectional_map
    {
        typedef std::pair<FromType,ToType> value_type;

#if defined(BOOST_NO_POINTER_TO_MEMBER_TEMPLATE_PARAMETERS) ||  \
    defined(BOOST_MSVC)&&(BOOST_MSVC<1300) ||                   \
    defined(BOOST_INTEL_CXX_VERSION)&&defined(_MSC_VER)&&       \
    (BOOST_INTEL_CXX_VERSION<=700)

/* see Compiler specifics: Use of member_offset for info on member<> and
 * member_offset<>
 */

        BOOST_STATIC_CONSTANT(unsigned,from_offset=offsetof(value_type,first));
        BOOST_STATIC_CONSTANT(unsigned,to_offset  =offsetof(value_type,second));

        typedef multi_index_container<
            value_type,
            indexed_by<
            ordered_unique<
            tag<from>,member_offset<value_type,FromType,from_offset> >,
            ordered_unique<
            tag<to>,  member_offset<value_type,ToType,to_offset> >
        >
            > type;

#else

        /* A bidirectional map can be simulated as a multi_index_container
         * of pairs of (FromType,ToType) with two unique indices, one
         * for each member of the pair.
         */

        typedef multi_index_container<
            value_type,
            indexed_by<
                ordered_unique<
                tag<from>,member<value_type,FromType,&value_type::first> >,
                ordered_unique<
                tag<to>,  member<value_type,ToType,&value_type::second> >
            >
            > type;

#endif
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    
    typedef std::list< fh_context > filters_t;
    
    namespace FCA 
    {
//#pragma GCC visibility push(default)
        
        FERRISEXP_API std::string getBitFunctionName( const std::string& treeName,
                                        const std::string& attributeName,
                                        bool quote = true );
        FERRISEXP_API void setFile( fh_context parent, const std::string& rdn,
                                    const std::string& data, int mode = 0 );
        FERRISEXP_API void setFile( fh_context parent, const std::string& rdn,
                                    std::stringstream& dataSS, int mode = 0 );
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        typedef boost::dynamic_bitset<> fca_std_bitset_t;
        typedef boost::dynamic_bitset<> extent_bitset_t;

        typedef std::map< long, FerrisBitMagic::bvector<> > VerticalFormalContext_t;
        typedef std::map< FerrisBitMagic::bvector<>, FerrisBitMagic::bvector<> > FormalContext_t;

        typedef std::map< long, extent_bitset_t > VerticalFormalContextBitSet_t;
        typedef std::map< fca_std_bitset_t, extent_bitset_t > FormalContextBitSet_t;

        
        class Concept;
        FERRIS_SMARTPTR( Concept, fh_concept );

        struct FERRISEXP_API RawConceptHandleCompare
        {
            bool operator()( fh_concept s1,
                             fh_concept s2 ) const
                {
                    return GetImpl( s1 ) < GetImpl( s2 );
                }
        };
        
        typedef std::set< fh_concept, RawConceptHandleCompare >  clist_t;
        typedef std::list< fh_context > ctxlist_t;

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        template<typename STREAM>
        inline STREAM &operator<<(STREAM &S, const FerrisBitMagic::bvector<>& bv )
        {
            return S;
            
            if( !bv.count() )
                return S;
            
            unsigned value = bv.get_first();
            do
            {
                S << value;
                value = bv.get_next(value);
                if (value)
                {
                    S << ",";
                }
                else
                {
                    break;
                }
            } while(1);
            S << std::endl;
        }

        FERRISEXP_API bool second_is_superset( const fca_std_bitset_t& a, const fca_std_bitset_t& b );
        FERRISEXP_API bool second_is_superset( const FerrisBitMagic::bvector<>& a, const FerrisBitMagic::bvector<>& b );
        FERRISEXP_API bool second_is_superset( const FerrisBitMagic::bvector<>& a,
                                               const FerrisBitMagic::bvector<
                                               FerrisBitMagic::standard_allocator, 
                                               FerrisBitMagic::miniset<
                                               FerrisBitMagic::block_allocator, FerrisBitMagic::set_total_blocks> >& b );
        FERRISEXP_API bool second_is_superset( const FerrisBitMagic::bvector<
                                               FerrisBitMagic::standard_allocator, 
                                               FerrisBitMagic::miniset<
                                               FerrisBitMagic::block_allocator, FerrisBitMagic::set_total_blocks> >& a,
                                               const FerrisBitMagic::bvector<>& b );
        FERRISEXP_API bool second_is_superset( const FerrisBitMagic::bvector<
                                               FerrisBitMagic::standard_allocator, 
                                               FerrisBitMagic::miniset<
                                               FerrisBitMagic::block_allocator, FerrisBitMagic::set_total_blocks> >& a,
                                               const FerrisBitMagic::bvector<
                                               FerrisBitMagic::standard_allocator, 
                                               FerrisBitMagic::miniset<
                                               FerrisBitMagic::block_allocator, FerrisBitMagic::set_total_blocks> >& b );

        template < class A, class B >
        bool second_is_superset( const A& a,
                                 const B& b )
        {
            typename A::enumerator an     = a.first();
            typename A::enumerator an_end = a.end();

            while (an < an_end)
            {
                if( !b[ *an ] )
                    return false;
                ++an;  // Fastest way to increment enumerator
            }
            return true;
        }

        
        FERRISEXP_API bool anythingSet( const FerrisBitMagic::bvector<>& a );
        FERRISEXP_API bool equiv( const fca_std_bitset_t& a, const fca_std_bitset_t& b );
        FERRISEXP_API bool equiv( const FerrisBitMagic::bvector<>& a, const FerrisBitMagic::bvector<>& b );
        FERRISEXP_API bool equiv( const FerrisBitMagic::bvector<>& a,
                                  const FerrisBitMagic::bvector<
                                  FerrisBitMagic::standard_allocator, 
                                  FerrisBitMagic::miniset<
                                  FerrisBitMagic::block_allocator, FerrisBitMagic::set_total_blocks> >& b );
        template < unsigned int N >
        bool equiv( const FerrisBitMagic::bvector<>& a,
                    const FerrisBitMagic::bvector<
                    FerrisBitMagic::standard_allocator,
                    FerrisBitMagic::bvmini<N> >& b )
        {
            FerrisBitMagic::bvector<>::enumerator an     = a.first();
            FerrisBitMagic::bvector<>::enumerator an_end = a.end();

            while (an < an_end)
            {
                if( !b[ *an ] )
                    return false;
                ++an;
            }
            return true;
        }
        
        FERRISEXP_API bool isZero( const FerrisBitMagic::bvector<>& a );

        template< unsigned int N >
        FerrisBitMagic::bvector<
            FerrisBitMagic::standard_allocator,
            FerrisBitMagic::bvmini< N > >& to_bvmini( const FerrisBitMagic::bvector<>& b,
                                                      FerrisBitMagic::bvector<
                                                      FerrisBitMagic::standard_allocator,
                                                      FerrisBitMagic::bvmini< N > >& ret )
        {
            FerrisBitMagic::bvector<>::enumerator bn     = b.first();
            FerrisBitMagic::bvector<>::enumerator bn_end = b.end();
            
            while (bn < bn_end)
            {
                ret[ *bn ] = 1;
                ++bn;
            }
        }

        
//         template<typename STREAM>
//         inline STREAM &operator<<(STREAM &S, const std::bit_vector& bv )
//         {
//             int i = 0;
//             S << "sz:" << bv.size() << " ";
//             for( std::bit_vector::const_iterator ci = bv.begin(); ci != bv.end(); ++ci )
//             {
//                 S << " " << *ci;
//                 if( ++i > 10 )
//                     break;
//             }
//             S << " ";
//             return S;
//         }

//        std::bit_vector operator&( const std::bit_vector& a, const std::bit_vector& b );
//        std::bit_vector operator^( const std::bit_vector& a, const std::bit_vector& b );
//        std::bit_vector& operator|=( std::bit_vector& a, const std::bit_vector& b );
//        std::bit_vector& operator^=( std::bit_vector& a, const std::bit_vector& b );
//         bool anythingSet( const std::bit_vector& a );
//         bool equiv( const std::bit_vector& a, const std::bit_vector& b );
//         bool isZero( const std::bit_vector& a );
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        class FERRISEXP_API Point 
        {
            float x;
            float y;
            float z;
        public:
            explicit Point( float x = 0, float y = 0, float z = 0 )
                : x(x), y(y), z(z)
                {
                }

            float getX() const
                {
                    return x;
                }
            float getY() const
                {
                    return y;
                }
            float getZ() const
                {
                    return z;
                }
            void setX( float v )
                {
                    x = v;
                }
            void setY( float v )
                {
                    y = v;
                }
            void setZ( float v )
                {
                    z = v;
                }
            inline Point& operator+=( const Point& p )
                {
                    x += p.x;
                    y += p.y;
                    z += p.z;
                    return *this;
                }

            inline bool operator==( const Point& p )
                {
                    return x == p.x && y == p.y && z == p.z;
                }
        };

        template < class _CharT, class _Traits >
        std::basic_ostream< _CharT, _Traits >&
        operator<<( std::basic_ostream< _CharT, _Traits >& ss, const Point& p )
        {
            ss << "Point x:" << p.getX() << " y:" << p.getY() << " z:" << p.getZ();
            return ss;
        }
        


        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        class ConceptLattice;
        FERRIS_SMARTPTR( ConceptLattice, fh_conceptLattice );
        
        
        /**
         * A formal concept from FCA.
         */
        class FERRISEXP_API Concept
            :
            public Handlable
        {
            typedef Concept _Self;
            friend class ConceptLattice;

            bool m_dirty;
            
            // new PG backed attributes
            fh_conceptLattice m_cl;          // concept lattice for this concept.
            int             m_id;            // uniq id
            FerrisBitMagic::bvector<> m_itemSet;       // items for this concet from the ConceptLattice
            int             m_itemSetWeight; // number of bits set in m_itemSet
            std::list< FerrisBitMagic::bvector<> > m_addedAttrs;    // attributes which are new for this concept.

            fca_std_bitset_t    m_itemSet_AsBitSet;  // m_itemSet as a std::bitset<> type
            
            int             m_extentSize;    // number of objects in extent.
            int             m_conceptOnlyMatchSize; // number of objects in just this concept
            
            double m_support_perc;
            int    m_support_abs;
            double m_support_ext;
            
            
            clist_t    m_parents;
            clist_t    m_upset;
            clist_t    m_children;
            clist_t    m_downset;
            fh_context m_extent;

            // old non PG backed attributes (maybe change?)
            filters_t  m_baseFilters;
            filters_t  m_contingentFilters;

            float m_x, m_y, m_z;
            
        public:
            // a cache list of the matching docids from getBaseTableName()
            // which match this concept.
            FerrisBitMagic::bvector<> m_conceptOnlyMatchingDocIDs;

            // a cache list of the matching docids of the extent
            FerrisBitMagic::bvector<> m_MatchingDocIDs;
        private:
            
            /**
             * Used by both getUpSet() and getDownSet() to get the transitive
             * sets
             */
            void     getUpDownSet_recurse( fh_concept p, clist_t& cache, bool isUpSet = true );
            clist_t& getUpDownSet( bool includeSelf = true, bool isUpSet = true );

        protected:

            void setDirty( bool v );
            
        public:

            virtual ~Concept();
            Concept( fh_conceptLattice cl, fh_context extent = 0 );
            void Construct( fh_conceptLattice cl );

            /**
             * Get the extent of this concept
             */
            fh_context getExtent();


            FerrisBitMagic::bvector<>& getExtentVerticalVector( FerrisBitMagic::bvector<>& ret,
                                                                bool clarified = false );
            extent_bitset_t& getExtentVerticalVectorBitSet( extent_bitset_t& ret,
                                                            bool clarified = false );


            /**
             * get the intent of this concept
             *
             * The intent is defined as the concepts in the upset that
             * introduce new filters, ie. all concepts in the upset
             * with getContingentFilters().size() > 0
             */
            clist_t getIntent();

            /**
             * get a list of filters that define this concept
             */
            filters_t& getBaseFilterList();

            /**
             * set the list of filters that define this concept
             * This list should be equal to the union of calling
             * getContingentFilters() on the upset of this concept
             * but also the base filters are ment to be in the rawest
             * form, ie. each column in the formal context is a single
             * filter in the passed list.
             */
            void setBaseFilterList( const filters_t& fl );

            /**
             * If a concept introduces any new filters to the lattice then
             * they should be set with this method. A concept that introduces
             * attributes is one that will appear in the intent of concepts
             * below itself.
             */
            virtual void setContingentFilters( const filters_t& fl );

            /**
             * Get the new filters that are introduced by this concept.
             */
            filters_t& getContingentFilters();

            /**
             * Get a list of the contingent filters for this concept and all
             * of its upset.
             */
            std::list< filters_t > getTransitiveContingentFilters();

            /**
             * If there are contingentFilters for this concept then it introduces
             * a new filter to the lattice, as such it is also defined as containing
             * an attribute. ie. ret is getContingentFilters().size()
             */
            bool containsAttribute();
            
            /**
             * get the single top concept node.
             */
            fh_concept getRootConcept();
             
            /**
             * Set the extent of this concept
             */
            virtual void setExtent( const fh_context& c );

            /**
             * Get the concepts that are direct parents of this concept
             */
            clist_t& getParents();
            
            /**
             * Remove a parent concept
             * Note that no links are effected.
             */
            void removeParent( const fh_concept& c );

            /**
             * Get the concepts that are direct children of this concept
             */
            clist_t& getChildren();
            clist_t& getChildren( clist_t& l );
            
            
            /**
             * Remove a child concept
             * Note that no links are effected.
             */
            void removeChild( const fh_concept& c );

            /**
             * is this concept meet irreducible, ie. getParents().size() == 1
             */
            bool isMeetIrreducible();

            /**
             * Get the size of the longest path from this concept to top.
             */
            int getMaxDepth();
            
            /**
             * Is this concept <= c
             * ie. c.getBaseFilterList() is a subset of this.getBaseFilterList()
             */
            bool less_than_or_equal( fh_concept c );
            
            /**
             * Get the concepts that are transitively above this concept
             *
             * @param if includeSelf is true then the concept itself is included
             *        in the return value
             */
            clist_t& getUpSet( bool includeSelf = true );

            /**
             * Get the concepts that are transitively below this concept
             *
             * @param if includeSelf is true then the concept itself is included
             *        in the return value
             */
            clist_t& getDownSet( bool includeSelf = true );

            clist_t& getXSet( bool isUpSet );

            typedef int intentsz_t;
            intentsz_t getIntentSize();

            /**
             * Associate the concept 'c' with this one as either a child or
             * parent. Note that subclasses will probably define a better method
             * to use for association, for example makeLink()
             */
            virtual void makeUniDirectionalLink( fh_concept c, bool isUpLink );
            void addParent( fh_concept c );
            void addChild( fh_concept c );
            void addChildren( clist_t& cl );
            
            /**
             * Make a bidirectional link
             */
            virtual void makeLink( fh_concept c, bool isUpLink );

            /**
             * disconnect this concept from its parents and children
             */
            virtual void disconnect();
            
            virtual void  setX( float x );
            virtual float getX();
            virtual void  setY( float y );
            virtual float getY();
            virtual void  setZ( float z );
            virtual float getZ();
            void setLocation( float x, float y, float z );
            void setPoint( const Point& p );
            Point getPoint();
            
            
            virtual void setFilterString( const std::string& s );
            virtual std::string getFilterString();
            

            /**
             * The fancy name is a string that allows any characters that
             * are legal in a string to be in it. Subclasses might have a
             * different limited name also that follows the restrictions of
             * the GUI toolkit used for the subclass view
             */
            virtual void        setFancyName( const std::string& s );
            virtual std::string getFancyName();
            
            int  getID();
            void setID( int v );

            /**
             * Call bvector::optimize() on the itemset.
             */
            void optimizeItemSet();
            
            /**
             * Get the itemset of the concept
             */
            const FerrisBitMagic::bvector<>& getItemSet() const;

            /**
             * Get the itemset of the concept as std::bitset<>
             */
            const fca_std_bitset_t& getItemSetBitSet() const;
            
            /**
             * Set the itemset of this concept.
             */
            void setItemSet( const FerrisBitMagic::bvector<>& v );

            /**
             * Get the names of the attributes which have been added to this
             * concept. ie, the attributes which none of our parent concepts
             * have.
             */
            stringlist_t& getAddedFormalConceptAttributes( stringlist_t& sl );

            /**
             * Get the attributes which are introduced considering all the attributes
             * that only one parent has.
             */
            stringlist_t& getAddedFormalConceptAttributesRelativeToParent( fh_concept pivc,
                                                                           stringlist_t& sl );
            
            /**
             * The size of the extent might be known before the extent
             * itself is fetched.
             */
            int getExtentSize();

            /**
             * update the ConceptOnlyMatchSize for this concept
             * from the database
             */
            void updateConceptOnlyMatchSize();

            /**
             * Get the number of items in the database for which this
             * concept is the lowest in the lattice matching that file.
             * ie. no subconcept of this concept will match the files in
             * the counter returned.
             */
            int getConceptOnlyMatchSize();

            
            /**
             * The name of the SQL Table which contains the formal context
             */
            std::string getBaseTableName();

            /**
             * Has this concept changed since it was read?
             */
            bool isDirty();

            /**
             * Get the concept label when viewing the lattice in fish eye mode
             */
            stringlist_t& getFishEyeLabel( stringlist_t& sl );

            fh_conceptLattice getConceptLattice();
            
        private:
            bool getFishEyeLabelRec( stringlist_t& sl );
            
        };


        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        struct ConceptLatticePriv;

        /**
         * These are the columns which are metadata about the concepts. Other
         * columns are assumed to be part of the formal context attributes.
         */
        stringset_t& getOutOfBandColumns( stringset_t& ret );

        class FERRISEXP_API ConceptLattice
            :
            public Handlable
        {
            typedef ConceptLattice _Self;

            bool m_dirty;
            void setDirty( bool v );
            int getNumberOfDirtyConcepts();
            
            
            /**
             * PImpl away the database connection etc.
             */
            struct ConceptLatticePriv* P;
            friend class Concept;

            /**
             * Mapping from concept->getID() to concept
             */
            typedef std::map< int, fh_concept > m_IDToConceptMap_t;
            m_IDToConceptMap_t m_IDToConceptMap;

            /**
             * fixed size for all concepts to use for their m_itemSet
             */
            int m_itemSetSize;

            fh_concept m_topConcept;       // lattice top concept
            fh_concept m_bottomConcept;    // lattice bottom concept

            /**
             * Used to obtain the names of the attributes in an itemset
             */
            typedef std::map< FerrisBitMagic::bvector<>, std::string > m_attrIDToString_t;
            m_attrIDToString_t m_attrIDToString;

            /**
             * findex that we are using for this lattice
             */
            std::string m_findexPath;

            /**
             * table that contains the CFI
             */
            std::string m_CFITableName;

            fh_context m_latticeTree;
            bool m_CFITableIsAugmented;
            std::string m_CFITableSelectTail;
            
            /**
             * table that contains the formal context
             */
            std::string m_baseTableName;
            
        public:
            typedef Loki::Functor< fh_concept, LOKI_TYPELIST_2( const fh_conceptLattice&,
                                                                int /* maxItemSetSizeInBits */ ) > F_ConceptFactory_t;
        private:
            /**
             * Factory function to create new Concept objects.
             */
            F_ConceptFactory_t m_conceptFactory_func;
            
        protected:
            ConceptLattice( fh_context latticeTree, const F_ConceptFactory_t& func );

            void priv_load_invertedlist( fh_context c, int id,
                                         FerrisBitMagic::bvector<>& bv );
            
            /**
             * Load a lattice from a CFI table. Called by the many static load() methods
             */
            void priv_load();

            /**
             * Set the single formal attribute name 'v' to be for the single
             * set bit in 'x'
             */
            void setAttrIDToString( const FerrisBitMagic::bvector<>& x, std::string v );

            /**
             * MUST BE called by the Concept class when it is changing its bit_vector
             */
            void priv_ConceptItemSetChanging( Concept* c,
                                              FerrisBitMagic::bvector<> oldv,
                                              FerrisBitMagic::bvector<> newv );
            /**
             * MUST BE Called by the Concept class when it is changing its ID
             */
            void priv_ConceptIDChanging( Concept* c, int oldv, int newv );

            void save_invertedlist( fh_context c, int id, FerrisBitMagic::bvector<>& bv );
            
        public:

            /**
             * This is for internal use only. Basically it is here to
             * allow access to the PImpl data and the PostgreSQL database
             * conncetion from methods in other classes internal to libferris.
             */
            ConceptLatticePriv* getPrivatePart();
            
            virtual ~ConceptLattice();

            static fh_conceptLattice load( const std::string& latticeTreePath );
            static fh_conceptLattice load( fh_context latticeTree );
            static fh_conceptLattice load(
                const std::string& latticeTreePath,
                const F_ConceptFactory_t& func );


            /**
             * Load a concept lattice from the given lattice tree using the
             * factory method to create new concept objects.
             *
             * The lattice tree path is a path under ~/.ferris/fcatree. The
             * path itself will be to the named view, for example,
             * ~/.ferris/fcatree/localhost/mydb/myview
             */
            static fh_conceptLattice load(
                fh_context latticeTree,
                const F_ConceptFactory_t& func );

            /**
             * Save the concept lattice back to the CFI SQL Table
             */
            void save();

            /**
             * The name of the SQL Table which contains the formal context
             */
            std::string getBaseTableName();

            /**
             * Get the top concept
             */
            fh_concept getTopConcept();

            /**
             * Get the bottom concept
             */
            fh_concept getBottomConcept();

            /**
             * Max number of bits in the itemset (ie, |A| )
             */
            long getMaxItemSetSizeInBits() const;
            
            /**
             * Get the collection of all concepts in the lattice
             */
            std::list< fh_concept >&   getConcepts( std::list< fh_concept >& ret ) const;
            
            /**
             * For a single set bit in a bit_vector get its string name
             *
             * @see AttrIDToStringList()
             */
            std::string AttrIDToString( const FerrisBitMagic::bvector<>& x );

            /**
             * Get the list of string attribute names for each set bit
             * in the bit_vector 'b'.
             */
            stringlist_t AttrIDToStringList( const FerrisBitMagic::bvector<>& x, stringlist_t& sl );

            /**
             * Get the string names of all the formal attributes in the lattice
             */
            stringlist_t& getAllAttributeNames( stringlist_t& ret ) const;

            /**
             * Get all the fh_context objects which might play a role in any concept
             * for the lattice. ie, the set G in ganter & wille.
             */
            fh_context getAllObjects();

            /**
             *
             * Attr -> Extent
             */
            VerticalFormalContext_t& getVerticalFormalContext(
                FerrisBitMagic::bvector<>& Gret,
                VerticalFormalContext_t& ret,
                bool clarify = false );

            VerticalFormalContextBitSet_t& getVerticalFormalContextBitSet(
                extent_bitset_t& Gret,
                VerticalFormalContextBitSet_t& ret,
                bool clarify = false );


            typedef bidirectional_map< fh_concept, FerrisBitMagic::bvector<>* >::type AllExtentVerticalVectors_t;
            AllExtentVerticalVectors_t& getAllExtentVerticalVectors( AllExtentVerticalVectors_t& ret,
                                                                     bool clarified = false );
            
            
            /**
             * get the attributeName -> ffilter mapping
             */
            stringmap_t&
            getAttributeToFFilterMap( stringmap_t& out );
            
            /**
             * Get a bit_vector with the named attribute set
             */
            FerrisBitMagic::bvector<> StringToAttrID( const std::string& s );

            /**
             * Get a list of which attributes the child concept has that the
             * parent concept doesn't.
             */
            stringlist_t getAddedAttributes( fh_concept parent,
                                             fh_concept child );

            /**
             * Reduce formal attributes which are known as direct implications from
             * formal scaling. For example, a linear scale on mtime would allow
             * mtime>100 to be removed if mtime>200 is also in the attribute list.
             */
            stringlist_t& makeReducedAttributes( stringlist_t& sl );

            /**
             * Convert some attribute names like mtime_111 into
             * mtime <= friday
             * type human readable labels.
             */
            stringlist_t& convertAttributeNamesToLabels( stringlist_t& sl );

            /**
             * For a given attribute name, eg, mtime, give a reasonable format string
             * for making labels of that attribute in a lattice.
             */
            std::string getReasonableTimeFormatStringForAttribute( stringmap_t& atof,
                                                                   const std::string& s );
            
            /**
             * Get an itemSet which is getItemSetSize() in size and has every
             * formal context bit (getFormalAttributeCount()) set to true.
             *
             * @see getFormalAttributeCount()
             * @see getItemSetSize()
             */
            FerrisBitMagic::bvector<> getFullItemSet();

            fca_std_bitset_t getFullItemSetBitSet();
            
            /**
             * The size that every Concept::getItemSet() will be. Use this size
             * in the constructor of FerrisBitMagic::bvector<> to make new itemSet vectors
             * which are to be compared with the itemSet vectors of this lattice.
             *
             * @see getFullItemSet()
             */
            int getItemSetSize();

            /**
             * What is the largest numeric concept ID in use
             */
            int        getHighestConceptID() const;

            /**
             * Get the concept which has the given ID
             */
            fh_concept getConcept( int id ) const;

            /**
             * Add a new concept to the lattice. The concept will be assumed to
             * not have any parents or children, you will have to link it to its
             * correct location in the ordering using Concept::makeLink() etc.
             *
             * @see Concept::makeLink()
             */
            void addConcept( fh_concept c );

            /**
             * Remove this concept from the database.
             */
            void removeConcept( fh_concept c );
            
            /**
             * Sometimes a conceptLattice will create new concpets, for example,
             * during the load() calls. You can set a factory which will create
             * concepts so that subclasses of concept can be made instead of
             * a plain concept.
             *
             * @see load()
             */
            void setConceptFactory( const F_ConceptFactory_t& func );

            /**
             * Get the number of attributes which are officially part of the
             * formal context that this concept lattice is derived from.
             *
             * @see getItemSetSize()
             */
            int getFormalAttributeCount();

            /**
             * Check the database and update the contingent size for every
             * concept in the lattice
             */
            void refreshAllConceptsContingentCounter();

            /**
             * Check if we have to create new concepts because:
             * top level concept (t) has no frequent itemsets which do not contain an item (x) not in (t)
             * ie if most of the collection contains attribute (x) we may get an itemset (t) which is large
             * and frequent but does not contain (x). This will lead to (t) being a strange top level concept
             * unless we create some other concepts to move (t) down in the lattice.
             */
            void fixInvalidTopLevelConcepts();

            /**
             * Has this lattice or any of its concepts changed since it was read?
             */
            bool isDirty();


            void exportAsToscanaJConceptualSchema( const std::string& outbase );
            void exportAsBurmeister( const std::string& outbase );


            /**
             * Get an integer configuration param from the latticetree root.
             */
            int getStrSubCtxInt( const std::string& k, int def );
            
        };
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        class LatticeLayout;
        typedef Loki::SmartPtr<
            LatticeLayout,
            FerrisLoki::FerrisExRefCounted,
            Loki::DisallowConversion,
            FerrisLoki::FerrisExSmartPointerChecker,
            FerrisLoki::FerrisExSmartPtrStorage >   fh_latticelayout;


        class LatticeLayout
            :
            public Handlable
        {
        protected:

            /**
             * Get a magnification factor that all x,y,z movements should be multiplied by
             */
            float getMagnificationFactor();
        
            /**
             * Move the root element of the lattice to 0,0,0
             */
            void assignZeroToRoot( fh_conceptLattice l );

            /**
             * Set each concept in begin to end to p
             */
            void assignPointToAll( fh_conceptLattice,
                                   clist_t::iterator begin, clist_t::iterator end, 
                                   const Point& p );
            void assignPointToAll( fh_conceptLattice, const Point& p );
        
            /**
             * PURE DEBUG to dump out the vectors for each contingent.
             */
            void dump( fh_conceptLattice );
        
        public:
            virtual fh_conceptLattice layout( fh_conceptLattice l ) = 0;
        };
        
        class LatticeLayout_Additive
            :
            public LatticeLayout
        {
            void
            assignVectorToMI( fh_conceptLattice l,
                              float ix, float iy, float iz,
                              float dx, float dy, float dz );
        
        public:
            virtual fh_conceptLattice layout( fh_conceptLattice l );
        };

        class LatticeLayout_Layered
            :
            public LatticeLayout
        {
            void shuffleToX( std::list< fh_concept >&, fh_concept c, double x );
        
        public:
            virtual fh_conceptLattice layout( fh_conceptLattice l );
        };

        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
//#pragma GCC visibility pop

    };
};
#endif
