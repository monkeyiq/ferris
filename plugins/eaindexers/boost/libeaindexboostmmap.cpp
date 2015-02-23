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

    $Id: libeaindexboostmmap.cpp,v 1.6 2009/02/16 21:30:18 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>

namespace bmi = boost::multi_index;


#include <Ferris/Ferris.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/EAIndexerMetaInterface.hh>
#include "ForwardEAIndexInterface.hh"
#include <Ferris/EAIndexer_private.hh>
#include <Ferris/Native.hh>
#include <Ferris/EAQuery.hh>
#include <Ferris/Configuration_private.hh>

// guessComparisonOperatorFromData
#include "FilteredContext_private.hh"

#include <string>

#include <Functor.h>
#include <gmodule.h>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/segment_manager.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/dynamic_bitset/config.hpp>

#include <boost/detail/dynamic_bitset.hpp>
#include "mmap_dynamic_bitset.hpp"
//#include <boost/dynamic_bitset.hpp>

//using namespace boost;
//using namespace boost::interprocess;
//using namespace std;

using boost::interprocess::file_mapping;
using boost::interprocess::mapped_region;
using boost::interprocess::read_write;
using boost::interprocess::open_or_create;
using boost::interprocess::open_only;
using boost::interprocess::open_read_only;
using boost::interprocess::managed_mapped_file;
using namespace boost::interprocess;
using namespace boost;
using std::cerr;
using std::endl;


namespace Ferris
{
    namespace EAIndex 
    {


//         static const char* CFG_IDX_BOOSTMMAP_PRIMARY_WRITE_INDEX_K = "cfg-idx-boostmmap-primary-write-index-k";
//         static const char* CFG_IDX_BOOSTMMAP_URLS_K = "cfg-idx-boostmmap-urls-k";

        typedef allocator<char, managed_mapped_file::segment_manager> char_allocator;
        typedef basic_string<char, std::char_traits<char>, char_allocator> shm_string;

        struct shm_string_less : std::binary_function<shm_string, shm_string, bool>
        {
            bool
            operator()(const shm_string& t1, const shm_string& t2) const
                { return t1 < t2; }
            bool
            operator()(const std::string& t1, const shm_string& t2) const
                { return t1 < std::string(t2.c_str()); }
            bool
            operator()(const shm_string& t1, const std::string& t2) const
                { return std::string(t1.c_str()) < t2; }

        };

        int getDigraphKey( char c1, char c2 )
        {
            if( c1 > 'z' || c1 < 'a' )
                return 0;
            if( c2 > 'z' || c2 < 'a' )
                return 0;

//            return (int)1 + (c1 - 'a'+1) * (c2 - 'a'+1);

            int t1 = c1 - 'a' + 1;
            t1 *= 26;
            return (int)1 + t1 + (c2 - 'a');
        }
        
        int getDigraphKey( const std::string& s )
        {
            if( s.length() < 2 )
                return 0;
            
            char c1 = s[0];
            char c2 = s[1];
            return getDigraphKey( c1, c2 );
        }
        typedef allocator<unsigned long, managed_mapped_file::segment_manager> ulong_allocator;
//        typedef boost::dynamic_bitset< unsigned long, ulong_allocator > tbitset_t;
        typedef boost::mmap_dynamic_bitset< unsigned long, ulong_allocator > digraph_bitset_t;
//        typedef boost::dynamic_bitset<> digraph_bitset_t;
        void setDigraph( digraph_bitset_t& ret, const std::string& s )
        {
            const char* p = s.c_str();
            const char* e = p + s.length() - 1;
            for( ; p != e; ++p )
            {
                int k = getDigraphKey( *p, *(p+1) );
                if( k )
                {
                    ret[ k ] = 1;
                }
            }
        }
        struct urlmap
        {
            uint32_t    urlid;
            uint32_t    tt;
            shm_string  earl;
            mutable digraph_bitset_t earl_bits;
            
            urlmap( uint32_t urlid, uint32_t tt, std::string earl, const char_allocator &a,
                    boost::interprocess::managed_mapped_file& m_mmap )
                :
                urlid( urlid ), tt(tt), earl( earl.c_str(), a )
//                , earl_bits( 768, 0, ulong_allocator( m_mmap.get_segment_manager() ))
                , earl_bits( ulong_allocator( m_mmap.get_segment_manager() ))
                {
                    earl_bits.resize( 768 );
                    std::string lower_earl = tolowerstr()( earl );
                    setDigraph( earl_bits, lower_earl );
                }
            void updateEarlBits() const
                {
                    std::string lower_earl = tolowerstr()( earl.c_str() );
                    setDigraph( earl_bits, lower_earl );
                }
        };
        // Tags
        struct urlid{};
        struct tt{};
        struct byearl{};
        // Collection
        typedef bmi::multi_index_container<
            urlmap,
            bmi::indexed_by<
            bmi::ordered_unique
            <bmi::tag<urlid>,  BOOST_MULTI_INDEX_MEMBER(urlmap,uint32_t,urlid)>,
            bmi::ordered_non_unique<
            bmi::tag<tt>,BOOST_MULTI_INDEX_MEMBER(urlmap,uint32_t,tt)>,
            bmi::ordered_unique
            <bmi::tag<byearl>, BOOST_MULTI_INDEX_MEMBER(urlmap,shm_string,earl), shm_string_less > >,
            managed_mapped_file::allocator<urlmap>::type
        > urlmap_collectin;
        typedef urlmap_collectin::index<urlid>::type urlmap_by_urlid;
        typedef urlmap_collectin::index<byearl>::type  urlmap_by_earl;

        ////////////////////////////////////////////////
        
        struct attrid{};
        struct vid{};
        struct value{};
        struct attridvid{};
        struct attridvalue{};
        struct urlattr{};
        
        ////////////////////////////////////////////////
        ////////////////////////////////////////////////
         
        struct attrmap
        {
            uint32_t    attrid;
            shm_string  value;
            attrmap( uint32_t attrid, std::string value, const char_allocator &a)
                : attrid(attrid), value( value.c_str(), a )
                {}
        };
        // Collection
        typedef bmi::multi_index_container<
            attrmap,
            bmi::indexed_by<
            bmi::ordered_unique<
            bmi::tag<attrid>,BOOST_MULTI_INDEX_MEMBER(attrmap,uint32_t,attrid)>,
            bmi::ordered_unique
            <bmi::tag<value>, BOOST_MULTI_INDEX_MEMBER(attrmap,shm_string,value), shm_string_less > >,
            managed_mapped_file::allocator<attrmap>::type
        > attrmap_collectin;
        typedef attrmap_collectin::index<value>::type attrmap_by_value;

        struct valuemap
        {
            uint32_t    vid;
            shm_string  value;
            valuemap( uint32_t vid, std::string value, const char_allocator &a)
                : vid(vid), value( value.c_str(), a )
                {}
        };
        // Collection
        typedef bmi::multi_index_container<
            valuemap,
            bmi::indexed_by<
            bmi::ordered_unique<
            bmi::tag<vid>,BOOST_MULTI_INDEX_MEMBER(valuemap,uint32_t,vid)>,
            bmi::ordered_unique
            <bmi::tag<value>, BOOST_MULTI_INDEX_MEMBER(valuemap,shm_string,value), shm_string_less > >,
            managed_mapped_file::allocator<valuemap>::type
        > valuemap_collectin;
        typedef valuemap_collectin::index<value>::type valuemap_by_value;
        typedef valuemap_collectin::index<vid>::type   valuemap_by_vid;


        //////////////////

        valuemap_collectin* vmptr = 0;

        struct vid_deref_less : std::binary_function<uint32_t, uint32_t, bool>
        {
            bool
            operator()(const uint32_t& vid1, const uint32_t& vid2) const
                {
                    valuemap_by_vid& m = vmptr->get<vid>();
                    valuemap_by_vid::iterator e = m.end();

                    valuemap_by_vid::iterator it1 = m.find( vid1 );
                    valuemap_by_vid::iterator it2 = m.find( vid2 );

                    if( it1 == e )
                        return 1;
                    if( it2 == e )
                        return 0;
                    
                    return it1->value < it2->value;
                }

        };
        

        struct docattrs
        {
            uint32_t    urlid;
            uint32_t    attrid;
            uint32_t    vid;
//            shm_string  value;
            docattrs( uint32_t urlid, uint32_t attrid, uint32_t vid, std::string value, const char_allocator &a)
                : urlid( urlid ), attrid(attrid), vid(vid)
                    // , value( value.c_str(), a )
                {}
        };
        // Collection
        typedef bmi::multi_index_container<
            docattrs,
            bmi::indexed_by<

            bmi::ordered_non_unique
            <bmi::tag<urlid>,  BOOST_MULTI_INDEX_MEMBER(docattrs,uint32_t,urlid)>,

//             bmi::ordered_non_unique<
//             bmi::tag<attrid>,BOOST_MULTI_INDEX_MEMBER(docattrs,uint32_t,attrid)>,

            bmi::ordered_non_unique
            <bmi::tag<value>, BOOST_MULTI_INDEX_MEMBER(docattrs,uint32_t,vid) >,

            
            bmi::ordered_non_unique< bmi::tag<attridvid>,
                                     bmi::composite_key<
            docattrs,
            bmi::member<docattrs,uint32_t,&docattrs::attrid>,
            bmi::member<docattrs,uint32_t,&docattrs::vid>
        >,
                                     bmi::composite_key_compare<
            std::less<uint32_t>,
            vid_deref_less
//            std::less<uint32_t>
        > >

        
            ,
            bmi::ordered_unique< bmi::tag<urlattr>,
                                 bmi::composite_key<
            docattrs,
            bmi::member<docattrs,uint32_t,&docattrs::urlid>,
            bmi::member<docattrs,uint32_t,&docattrs::attrid>
        > >
            
        
//            ,
//             bmi::ordered_non_unique< bmi::tag<attridvalue>,
//                                      bmi::composite_key<
//             docattrs,
//             bmi::member<docattrs,uint32_t,&docattrs::attrid>,
//             bmi::member<docattrs,shm_string,&docattrs::value>
//         >,
//                                      bmi::composite_key_compare<
//             std::less<uint32_t>,   
//             shm_string_less
//         > >


        
        >,
        managed_mapped_file::allocator<docattrs>::type
        > docattrs_collectin;
        typedef docattrs_collectin::index<attridvid>::type   docattrs_by_attridvid;
//        typedef docattrs_collectin::index<attrid>::type      docattrs_by_attrid;
        typedef docattrs_collectin::index<urlattr>::type     docattrs_by_urlattr;
//        typedef docattrs_collectin::index<attridvalue>::type docattrs_by_attridvalue;

        
        ///////////////////////////////////////        
        

    typedef std::list< uint32_t > vids_t;

    struct BoostMMapEAIndexerData
    {
        uint32_t m_nextURLID;
    };



        /********************************************************************/
        /********************************************************************/
        /********************************************************************/

        class BoostMMapEAIndexer;
        FERRIS_SMARTPTR( BoostMMapEAIndexer, fh_BoostMMapEAIndexer );

        class FERRISEXP_API ForwardEAIndexInterfaceBoost
            :
            public ForwardEAIndexInterface
        {
            fh_BoostMMapEAIndexer m_idx;
            
        public:
            ForwardEAIndexInterfaceBoost( fh_BoostMMapEAIndexer idx );
            virtual std::string getStrAttr( Context* c,
                                            const std::string& earl,
                                            const std::string& rdn,
                                            const std::string& def,
                                            bool throw_for_errors = true );
        };
        
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        

        class FERRISEXP_API BoostMMapEAIndexer
            :
            public MetaEAIndexerInterface
        {
            typedef MetaEAIndexerInterface _Base;
            typedef BoostMMapEAIndexer     _Self;

            /****************************************/
            /****************************************/
            /****************************************/

            boost::interprocess::managed_mapped_file m_mmap;
            BoostMMapEAIndexerData* m_data;
            urlmap_collectin* m_urlmap;
            docattrs_collectin* m_docattrs;
            attrmap_collectin* m_attrmap;
            valuemap_collectin* m_valuemap;
            

            
        protected:

            virtual void Setup()
                {
                    setOpenConfigReadOnly( true );
                }
            virtual void CreateIndexBeforeConfig( fh_context c,
                                                  fh_context md )
                {
                }
            virtual void CreateIndex( fh_context c,
                                      fh_context md )
                {
                }
            virtual void CommonConstruction()
                {
                    LG_EAIDX_D << "CommonConstruction()" << endl;
                    
                    std::string filepath = getPath() + "/mapfile.boost";
                    cerr << "filepath:" << filepath << endl;

                    bool readOnly = false;
                    if( readOnly )
                    {
                        boost::interprocess::managed_mapped_file mmap( open_read_only, filepath.c_str(), 0 );
                        m_mmap.swap( mmap );

                        m_data = m_mmap.find<BoostMMapEAIndexerData>("data").first;
                        m_urlmap = m_mmap.find<urlmap_collectin>("urlmap").first;
                        m_docattrs = m_mmap.find<docattrs_collectin>("docattrs").first;
                        m_attrmap = m_mmap.find<attrmap_collectin>("attrmap").first;
                        m_valuemap = m_mmap.find<valuemap_collectin>("valuemap").first;
                        vmptr = m_valuemap;
                    }
                    else
                    {
                        long initialSize = 1024*1024;
                        long fSize = toType<long>(getStrAttr( filepath, "size", "0" ));
                        initialSize = std::max( initialSize, fSize );
//                        cerr << "fSize:" << fSize << endl;
//                        cerr << "initialSize:" << initialSize << endl;
                        
                        boost::interprocess::managed_mapped_file mmap( open_or_create, filepath.c_str(), initialSize );
                        m_mmap.swap( mmap );

                        typedef allocator<BoostMMapEAIndexerData, managed_mapped_file::segment_manager> BoostMMapEAIndexerData_allocator;
                        m_data = m_mmap.find_or_construct<BoostMMapEAIndexerData>("data")();

                        typedef allocator<urlmap_collectin, managed_mapped_file::segment_manager> urlmap_collectin_allocator;
                        m_urlmap = m_mmap.find_or_construct<urlmap_collectin>("urlmap")
                            (urlmap_collectin_allocator(m_mmap.get_segment_manager()));//first ctor parameter

                        typedef allocator<docattrs_collectin, managed_mapped_file::segment_manager> docattrs_collectin_allocator;
                        m_docattrs = m_mmap.find_or_construct<docattrs_collectin>("docattrs")
                            (docattrs_collectin_allocator(m_mmap.get_segment_manager()));//first ctor parameter

                        typedef allocator<attrmap_collectin, managed_mapped_file::segment_manager> attrmap_collectin_allocator;
                        m_attrmap = m_mmap.find_or_construct<attrmap_collectin>("attrmap")
                            (attrmap_collectin_allocator(m_mmap.get_segment_manager()));//first ctor parameter

                        typedef allocator<valuemap_collectin, managed_mapped_file::segment_manager> valuemap_collectin_allocator;
                        m_valuemap = m_mmap.find_or_construct<valuemap_collectin>("valuemap")
                            (valuemap_collectin_allocator(m_mmap.get_segment_manager()));//first ctor parameter

                        vmptr = m_valuemap;
                        
                        typedef std::vector<  AttrType_t > attr_types_t;
                        attr_types_t attr_types;
                        attr_types.push_back( ATTRTYPEID_INT );
                        attr_types.push_back( ATTRTYPEID_DBL );
                        attr_types.push_back( ATTRTYPEID_TIME );
                        attr_types.push_back( ATTRTYPEID_STR );
                        attr_types.push_back( ATTRTYPEID_CIS );
                        attr_types_t::iterator ae = attr_types.end();
                        const char_allocator cAllocator (m_mmap.get_segment_manager());
                        
                        for( attr_types_t::iterator it = attr_types.begin(); it != ae; ++it )
                        {
                            AttrType_t att = *it;
                            std::string v = canonValue( "0", att );
                            uint32_t vid = getValueIDForString( v );
                            if( !vid )
                            {
                                vid = m_valuemap->size() + 1;
                                m_valuemap->insert(valuemap( vid, v, cAllocator ));
                            }
                        }
                        




                        
                    
                        typedef allocator<int, managed_mapped_file::segment_manager> ShmemAllocator;
                        typedef vector<int, ShmemAllocator> MyVector;
                        const ShmemAllocator alloc_inst (m_mmap.get_segment_manager());
                        MyVector *myvector = 
                            m_mmap.find_or_construct<MyVector>("MyVector") //object name
                            (alloc_inst);//first ctor parameter

                        const char_allocator myAllocator (m_mmap.get_segment_manager());
                        shm_string* mapped_string =
                            m_mmap.find_or_construct<shm_string>("test string")
                            ( myAllocator );
                        shm_string x( myAllocator );
                        *mapped_string = "raw data here";
//                    x.swap(  *mapped_string );
                        *mapped_string = "foobar 33333333333333 xxx 1111112222222 yyy";

                        m_mmap.flush();
                    }
                }

            /****************************************/
            /****************************************/
            /****************************************/

        public:

            BoostMMapEAIndexer()
                :
                m_data( 0 ),
                m_urlmap(0),
                m_docattrs(0),
                m_attrmap(0),
                m_valuemap(0)
                {
                    LG_EAIDX_D << "BoostMMapEAIndexer()" << endl;
                }
                

            virtual void sync()
                {
                }

            
            virtual void prepareForWrites( int f = PREPARE_FOR_WRITES_NONE )
                {
                }

            uint32_t getAttrIDForString( const std::string& k )
                {
                    uint32_t aid = 0;
                    attrmap_by_value& amap=m_attrmap->get<value>();
                    attrmap_by_value::iterator it = amap.find( k );
                    if( it != amap.end() )
                    {
                        aid = it->attrid;
//                        cerr << "getAttrIDForString() aid:" << aid << " k:" << k << " full-k:" << it->value << endl;
                    }
                    return aid;
                }

            std::string getValueForVID( uint32_t v )
                {
                    std::string ret;
                    valuemap_by_vid& vmap=m_valuemap->get<vid>();
                    valuemap_by_vid::iterator it = vmap.find( v );
                    if( it != vmap.end() )
                    {
                        ret = it->value.c_str();
                    }
                    return ret;
                }
            uint32_t getValueIDForString( const std::string& v )
                {
                    uint32_t vid = 0;
                    valuemap_by_value& vmap=m_valuemap->get<value>();
                    valuemap_by_value::iterator it = vmap.find( v );
                    if( it != vmap.end() )
                    {
                        vid = it->vid;
                    }
                    return vid;
                }
            uint32_t getValueIDForStringUB( const std::string& v )
                {
                    uint32_t vid = 0;
                    valuemap_by_value& vmap=m_valuemap->get<value>();
                    valuemap_by_value::iterator it = vmap.upper_bound( v );
                    if( it != vmap.end() )
                    {
                        vid = it->vid;
                    }
                    return vid;
                }
            uint32_t getValueIDForStringLB( const std::string& v )
                {
                    uint32_t vid = 0;
                    valuemap_by_value& vmap=m_valuemap->get<value>();
                    valuemap_by_value::iterator it = vmap.lower_bound( v );
                    if( it != vmap.end() )
                    {
                        vid = it->vid;
                    }
                    return vid;
                }

            
            std::string canonValue( std::string v, AttrType_t att )
                {
                    if( att >= ATTRTYPEID_STR )
                        return v;
                    std::stringstream ss;
                    if( att == ATTRTYPEID_DBL )
                    {
                        double l = log10( toType<double>(v) );
                        if( l >= 28 )
                            ss << std::setfill('0') << "huge" << std::setw(24) << std::setprecision(4) << std::fixed << l;
                        else
                            ss << std::setfill('0') << std::setw(28) << std::setprecision(4) << std::fixed << toType<double>(v);
                    }
                    else
                    {
                        ss << std::setfill('0') << std::setw(32) << toint(v);
                    }
                    
                    return ss.str();
                }

            bool expandMMapFile()
                {
                    cerr << "expandMMapFile() growing memory mapped file again..." << endl;
                    
                    bool ret = false;
                    
                    std::string filepath = getPath() + "/mapfile.boost";
                    int growSize = 1024*1024;
                    ret =  boost::interprocess::managed_mapped_file::grow( filepath.c_str(), growSize );
                    if( ret )
                    {
                        CommonConstruction();
                    }
                    return ret;
                }
            
            bool
            getIndexMethodSupportsIsFileNewerThanIndexedVersion()
                {
                    return true;
                }

            bool
            isFileNewerThanIndexedVersion( const fh_context& c )
                {
                    bool ret = true;

                    time_t tt = Time::getTime();
                    time_t ct = getTimeAttr( c, "ferris-should-reindex-if-newer", 0 );
                    if( !ct )
                        return ret;

                    urlmap_by_earl& m = m_urlmap->get<byearl>();
                    urlmap_by_earl::iterator it = m.find( c->getURL() );
                    urlmap_by_earl::iterator  e = m.end();
                    if( it != e )
                    {
                        uint32_t dt = it->tt;
                        
                        LG_EAIDX_D << "isFileNewerThanIndexedVersion(test) dt:" << dt << endl;
                        LG_EAIDX_D << "isFileNewerThanIndexedVersion() ct:" << Time::toTimeString( ct ) << endl;
                        LG_EAIDX_D << "isFileNewerThanIndexedVersion() dt:" << Time::toTimeString( dt ) << endl;

                
                        if( ct < dt )
                        {
                            LG_EAIDX_D << "isFileNewerThanIndexedVersion(DO NOT INDEX) document->getMTime():" << dt << endl;
                            return false;
                        }
                    }
                    
                    return ret;
                }
            
            
            
            virtual void addToIndex( fh_context c,
                                     fh_docindexer di )
            {
                try
                {
                    const char_allocator cAllocator (m_mmap.get_segment_manager());

                    uint32_t urlid = 0;
                    urlmap_by_earl& m = m_urlmap->get<byearl>();
                    urlmap_by_earl::iterator it = m.find( c->getURL() );
                    urlmap_by_earl::iterator  e = m.end();
                    if( it != e )
                    {
                        urlid = it->urlid;
                    }
                    else
                    {
                        urlid = m_urlmap->size()+1;
                        uint32_t tt = time(0);
                        m_urlmap->insert(urlmap( urlid, tt, c->getURL(), cAllocator, m_mmap ));
                    }

                    std::string earl = c->getURL();
                    stringlist_t slist;
                    getEANamesToIndex( c, slist );
                    int totalAttributes = slist.size();
                    
                    typedef std::map< std::string, IndexableValue > ivs_t;
                    ivs_t ivs;

                    for( stringlist_t::iterator si = slist.begin(); si != slist.end(); ++si )
                    {
                        try
                        {
                            std::string attributeName = *si;
                            std::string k = attributeName;
                            std::string v = "";
                    
                            if( !obtainValueIfShouldIndex( c, di, attributeName, v ))
                                continue;

                            LG_EAIDX_D << "EAIndexerPostgresql::addToIndex() attributeName:" << attributeName << endl;
                            LG_EAIDX_I << "addContextToIndex(a) c:" << c->getURL() << endl
                                       << " k:" << k << " v:" << v << endl;

                    
                            IndexableValue iv  = getIndexableValue( c, k, v );
                            ivs.insert( make_pair( attributeName, iv ) );
                        }
                        catch( std::exception& e )
                        {
                            LG_EAIDX_D << "EXCEPTION e:" << e.what() << endl;
                        }
                    }


                    for( ivs_t::iterator ivi = ivs.begin(); ivi!=ivs.end(); ++ivi )
                    {
                        IndexableValue& iv = ivi->second;
                        const std::string& k = iv.rawEANameString();
                        LG_EAIDX_D << "attributeName:" << k << endl;
                        std::string v = iv.rawValueString();

                        XSDBasic_t sct = iv.getSchemaType();
                        AttrType_t att = iv.getAttrTypeID();
                        if( att == ATTRTYPEID_CIS )
                            att = ATTRTYPEID_STR;
                        if( sct == FXD_UNIXEPOCH_T )
                            att = ATTRTYPEID_TIME;

                        uint32_t aid = getAttrIDForString( k );
//                        cerr << "addToIndeX() aid:" << aid << " k:" << k << endl;
                        if( !aid )
                        {
                            aid = m_attrmap->size() + 1;
                            m_attrmap->insert(attrmap( aid, k, cAllocator ));
                        }

                        v = canonValue( v, att );
                        uint32_t vid = getValueIDForString( v );
                        if( !vid )
                        {
                            vid = m_valuemap->size() + 1;
                            m_valuemap->insert(valuemap( vid, v, cAllocator ));
                        }
                        
                        m_docattrs->insert(docattrs( urlid, aid, vid, v, cAllocator ));
                    }

                    incrFilesIndexedCount();
                }
                catch( bad_alloc& e )
                {
                    cerr << "bad alloc e:" << e.what() << endl;

                    if( !expandMMapFile() )
                    {
                        cerr << "Error expanding memory mapped file!" << endl;
                        exit(1);
                    }
                    addToIndex( c, di );
                }
            }
            
            // FIXME: same as method in postgresql module
            std::string
            BuildQuery_getToken( fh_context q )
                {
                    std::string token   = getStrAttr( q, "token", "" );
                    std::string tokenfc = foldcase( token );
                    return tokenfc;
                }

            // FIXME: same as method in postgresql module
            std::pair< fh_context, fh_context >
            BuildQuery_getLeftAndRightContexts( fh_context q )
                {
                    fh_attribute orderedtla = q->getAttribute("in-order-insert-list");
                    fh_istream   orderedtls = orderedtla->getIStream();
            
//             LG_EAIDX_D << "BuildQuery() token:" << tokenfc << endl;
//             LG_EAIDX_D << " q.child count:" << q->SubContextCount() << endl;

                    std::string s;
                    getline( orderedtls, s );
                    LG_EAIDX_D << "BuildQuery() lc:" << s << endl;
                    fh_context lc = q->getSubContext( s );

                    fh_context rc = 0;
                    if( getline( orderedtls, s ) && !s.empty() )
                    {
                        LG_EAIDX_D << "EAQuery_Heur::BuildQuery() rc:" << s << endl;
                        rc = q->getSubContext( s );
                    }
                    
                    return std::make_pair( lc, rc );
                }

//             void get_vids_for_aid( vids_t& vids, uint32_t aid )
//                 {
//                     docattrs_by_attrid& dm_aid = m_docattrs->get<attrid>();
//                     docattrs_by_attrid::iterator it = dm_aid.lower_bound( aid );
//                     docattrs_by_attrid::iterator  e = dm_aid.upper_bound( aid );
//                     for( ; it != e; ++it )
//                     {
//                         vids.push_back( it->vid );
//                     }
//                 }

//             struct compare_operator_lt
//             {
//                 std::string m_value;
//                 compare_operator_lt( std::string value )
//                     :
//                     m_value( value )
//                     {
//                     }
//                 inline bool operator()( shm_string x ) const
//                     {
//                         bool ret = false;
//                         ret = (std::string(x.c_str()) <= m_value);
//                         return ret;
//                     }
//             };

//             struct compare_operator_regex
//             {
//                 fh_rex m_rex;
//                 compare_operator_regex( fh_rex rex )
//                     :
//                     m_rex( rex )
//                     {
//                     }
//                 inline bool operator()( shm_string x ) const
//                     {
//                         return ( regex_search( x.c_str(), m_rex ) );
//                     }
//             };

//             template< class T >
//             docNumSet_t& ExecuteQueryDocAttrsComparison( fh_context q,
//                                                          docNumSet_t& output,
//                                                          fh_eaquery qobj,
//                                                          int limit,
//                                                          uint32_t aid,
//                                                          const T& cop )
//                 {
//                     cerr << "getting vids" << endl;
//                     vids_t vids;
//                     get_vids_for_aid( vids, aid );
//                     cerr << "vids.sz:" << vids.size() << endl;
                    
//                     valuemap_by_vid& vmvid = m_valuemap->get<vid>();
//                     valuemap_by_vid::iterator vmvidend = vmvid.end();
//                     vids_t::iterator viter = vids.begin();
//                     vids_t::iterator vend  = vids.end();
//                     for( ; viter != vend; ++viter )
//                     {
//                         valuemap_by_vid::iterator it = vmvid.find( *viter );
//                         if( it != vmvidend )
//                         {
//                             if( cop( it->value ) )
//                             {
//                                 uint32_t vid = it->vid;
// //                                cerr << " iter->value:" << it->value << " vid:" << vid << endl;

//                                 docattrs_by_attridvid& m = m_docattrs->get<attridvid>();
// //                                             docattrs_by_attridvid::iterator it = m.find( boost::make_tuple( aid, vid ) );
// //                                             if( it != m.end() )
//                                 std::pair<
//                                     docattrs_by_attridvid::iterator,
//                                     docattrs_by_attridvid::iterator> p = m.equal_range( boost::make_tuple( aid, vid ) );
//                                 for( docattrs_by_attridvid::iterator it = p.first; it != p.second; ++it )
//                                 {
//                                     cerr << "have a match!" << endl;
//                                     output.insert( it->urlid );
//                                 }
                                            
//                             }
//                         }
//                     }
//                 }
            

            // FIXME: make this a common method in the base class
            virtual docNumSet_t& ExecuteQueryGenericAnd( fh_context q,
                                                         docNumSet_t& docnums,
                                                         fh_eaquery qobj,
                                                         int limit = 0 )
                {
                    for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                    {
                        docNumSet_t qdocs;
                        docNumSet_t intersect;
                        ExecuteQuery( *ci, qdocs, qobj, limit );
                        LG_EAIDX_D << "subquery has count:" << qdocs.size() << endl;
                        if( docnums.empty() )
                        {
                            swap( qdocs, docnums );
                        }
                        else
                        {
                            set_intersection( docnums.begin(), docnums.end(),
                                              qdocs.begin(), qdocs.end(),
                                              inserter( intersect, intersect.begin() ) );
                            LG_EAIDX_D << "docnums has count:" << docnums.size() << endl;
                            LG_EAIDX_D << "inters has count:" << intersect.size() << endl;
                            swap( intersect, docnums );
                        }
                    
                    }
                    return docnums;
                }


            // FIXME: make this a common method in the base class
            virtual docNumSet_t& ExecuteQueryGenericOr( fh_context q,
                                                        docNumSet_t& docnums,
                                                        fh_eaquery qobj,
                                                        int limit = 0 )
                {
                    docNumSet_t tmp1;
                    docNumSet_t tmp2;

                    docNumSet_t& fromDocs = tmp1;
                    docNumSet_t& toDocs   = tmp2;
                
                    for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                    {
                        docNumSet_t qdocs;
                        ExecuteQuery( *ci, qdocs, qobj, limit );
                        set_union( fromDocs.begin(), fromDocs.end(),
                                   qdocs.begin(), qdocs.end(),
                                   inserter( toDocs, toDocs.begin() ) );
                        swap( fromDocs, toDocs );
                        toDocs.clear();
                    }
                    docnums.insert( fromDocs.begin(), fromDocs.end() );
                    return docnums;
                }


            
            virtual docNumSet_t& ExecuteQuery( fh_context q,
                                               docNumSet_t& output,
                                               fh_eaquery qobj,
                                               int limit = 0 )
                {
                    std::string tokenfc = BuildQuery_getToken( q );
                    std::pair< fh_context, fh_context > lcrc_pair = BuildQuery_getLeftAndRightContexts( q );
                    fh_context lc = lcrc_pair.first;
                    fh_context rc = lcrc_pair.second;

                    // handle combinators
                    if( tokenfc == "&" )
                    {
                        LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(&) detected" << endl;
                        return ExecuteQueryGenericAnd( q, output, qobj, limit );
                    }
                    else if( tokenfc == "|" )
                    {
                        return ExecuteQueryGenericOr( q, output, qobj, limit );
                    }
                    
                    
                    std::string eaname = getStrAttr( lc, "token", "" );
                    IndexableValue iv = getIndexableValueFromToken( eaname, rc );
                    std::string value = asString( iv );
                    AttrType_t attrTypeID = inferAttrTypeID( iv );

                    value = canonValue( value, attrTypeID );

                    cerr << "attrTypeID:" << attrTypeID << endl;
                    cerr << "key:" << eaname << endl;
                    cerr << "value:" << value << endl;
                    
                    if( tokenfc == "<=" )
                    {
                        uint32_t aid = getAttrIDForString( eaname );
                        if( aid )
                        {
//                            compare_operator_lt cop( value );
//                            ExecuteQueryDocAttrsComparison( q, output, qobj, limit, aid, cop );

                            //// version 2
                            
//                             cerr << "searching <=, aid:" << aid << " value:" << value << endl;
//                             docattrs_by_attridvalue& m = m_docattrs->get<attridvalue>();

//                             docattrs_by_attridvalue::iterator it = m.lower_bound( boost::make_tuple( aid, canonValue( "0", attrTypeID ) ) );
//                             docattrs_by_attridvalue::iterator  e = m.upper_bound( boost::make_tuple( aid, value ) );
//                             for( ; it != e; ++it )
//                             {
//                                 cerr << "match:" << it->value << endl;
//                                 output.insert( it->urlid );
//                             }


                            // version 3

                            cerr << "searching <=, aid:" << aid << " value:" << value << endl;
                            docattrs_by_attridvid& m = m_docattrs->get<attridvid>();

                            uint32_t valuevid = getValueIDForStringUB( value );
                            uint32_t zerovid  = getValueIDForString( canonValue( "0", attrTypeID ) );
                            cerr << "value vid:" << valuevid << endl;
                            cerr << "zero vid:" << zerovid << endl;
                            cerr << "value:" << value << endl;


//                             docattrs_by_attridvid::iterator it = m.lower_bound( boost::make_tuple( aid ) );
//                             docattrs_by_attridvid::iterator  e = m.upper_bound( boost::make_tuple( aid ) );
                            
                            docattrs_by_attridvid::iterator it = m.lower_bound( boost::make_tuple( aid, zerovid ) );
                            docattrs_by_attridvid::iterator  e = m.lower_bound( boost::make_tuple( aid, valuevid ) );
                            for( ; it != e; ++it )
                            {
//                                cerr << "match:" << it->value << endl;
                                output.insert( it->urlid );
                            }
                            
                        }
                    }
                    else if( tokenfc == ">=" )
                    {
                        uint32_t aid = getAttrIDForString( eaname );
                        if( aid )
                        {
                            cerr << "searching >=, aid:" << aid << " value:" << value << endl;

                            // version 2
//                             docattrs_by_attridvalue& m = m_docattrs->get<attridvalue>();
//                             docattrs_by_attridvalue::iterator it = m.lower_bound( boost::make_tuple( aid, value ) );
//                             docattrs_by_attridvalue::iterator  e = m.lower_bound( boost::make_tuple( aid+1 ) );
//                             for( ; it != e; ++it )
//                             {
//                                 cerr << "match:" << it->value << endl;
//                                 output.insert( it->urlid );
//                             }

                            // version 3
                            docattrs_by_attridvid& m = m_docattrs->get<attridvid>();
                            uint32_t valuevid = getValueIDForStringLB( value );
                            docattrs_by_attridvid::iterator it = m.lower_bound( boost::make_tuple( aid, valuevid ) );
                            docattrs_by_attridvid::iterator  e = m.lower_bound( boost::make_tuple( aid+1 ) );
                            for( ; it != e; ++it )
                            {
//                                cerr << "match:" << it->value << endl;
                                output.insert( it->urlid );
                            }
                            
                        }
                    }
                    else if( tokenfc == "==" )
                    {
                        if( eaname == "url" )
                        {
                            urlmap_by_earl& m = m_urlmap->get<byearl>();
                            urlmap_by_earl::iterator it = m.find( value );
                            urlmap_by_earl::iterator  e = m.end();
                            if( it != e )
                            {
                                cerr << "found url:" << it->earl << endl;
                                output.insert( it->urlid );
                            }
                        }
                        else
                        {
                            uint32_t aid = getAttrIDForString( eaname );
                            if( aid )
                            {
                                uint32_t vid = getValueIDForString( value );
                                if( vid )
                                {
                                    docattrs_by_attridvid& m = m_docattrs->get<attridvid>();

                                    std::pair<
                                        docattrs_by_attridvid::iterator,
                                        docattrs_by_attridvid::iterator> p = m.equal_range( boost::make_tuple( aid, vid ) );
                                    for( docattrs_by_attridvid::iterator it = p.first; it != p.second; ++it )
                                    {
                                        cerr << "have a match!" << endl;
                                        output.insert( it->urlid );
                                    }
                                }
                            }
                        }
                    }
                    else if( tokenfc == "=~" )
                    {
                        boost::regex::flag_type rflags = boost::regex::optimize;
                        if( !iv.isCaseSensitive() )
                            rflags |= boost::regex::icase;
                        fh_rex rex = toregexh( value, rflags );

                        bool isTrivialRegex = true;
                        {
                            for( const char* p = value.c_str(); *p; ++p )
                            {
                                if( *p < 'a' || *p > 'z' )
                                {
                                    isTrivialRegex = false;
                                    break;
                                }
                            }
                        }
//                        isTrivialRegex = false;
                        cerr << "isTrivialRegex:" << isTrivialRegex << endl;

                        
                        int matches = 0;
                        int regex_calls = 0;
                        if( eaname == "url" )
                        {
//                            cerr << "have url!" << endl;
//                            cerr << "m_urlmap.sz:" << m_urlmap->size() << endl;
                            urlmap_collectin::iterator iter = m_urlmap->begin();
                            urlmap_collectin::iterator    e = m_urlmap->end();
                            for( ; iter != e; ++iter )
                            {
//                                cerr << "earl:" << iter->earl << endl;
                                if( isTrivialRegex )
                                {
                                    bool skip = false;
                                    static int LIBFERRIS_DISABLE_DIGRAPH_FASTSKIP
                                        = g_getenv ("LIBFERRIS_DISABLE_DIGRAPH_FASTSKIP") > 0;

                                    if( !LIBFERRIS_DISABLE_DIGRAPH_FASTSKIP )
                                    {
                                        const char* p = value.c_str();
                                        const char* e = p + value.length() - 1;
                                        for( ; p != e && !skip; ++p )
                                        {
                                            int k = getDigraphKey( *p, *(p+1) );
//                                          cerr << "....k:" << k <<  " p:" << *p << " " << *(p+1)
//                                               << " pv:" << (*p - 'a')
//                                               << " " << (*(p+1) - 'a')
//                                               << endl;

                                            if( k )
                                            {
                                                if( !iter->earl_bits[ k ] )
                                                {
                                                    skip = true;
//                                                cerr << "skipping one! earl:" << iter->earl << endl;
                                                }
                                            }
                                        }
                                    }
                                    
                                    if( skip )
                                        continue;
//                                    cerr << "not skipping:" << iter->earl << endl;

//                                     cerr << "---------------------" << endl;
//                                     // DEBUG
//                                     {
//                                         std::string s = iter->earl.c_str();
//                                         const char* p = s.c_str();
//                                         const char* e = p + s.length() - 1;
//                                         for( ; p != e; ++p )
//                                         {
//                                             int k = getDigraphKey( *p, *(p+1) );
//                                             cerr << "....k:" << k <<  " p:" << *p << " " << *(p+1)
//                                                  << " pv:" << (*p - 'a')
//                                                  << " " << (*(p+1) - 'a')
//                                                  << endl;
//                                         }
//                                     }
//                                     cerr << "---------------------" << endl;
                                    
                                }
                                
                                ++regex_calls;
                                if( regex_search( iter->earl.c_str(), rex ) )
                                {
                                    ++matches;
                                    output.insert( iter->urlid );
                                    
//                                     if( ends_with( iter->earl.c_str(), ".ogg" ))
//                                     {
//                                         ++matches;
//                                         output.insert( iter->urlid );
//                                     }
                                }
                                if( limit && matches >= limit )
                                {
                                    break;
                                }
                            }

                            
                            cerr << "regex_calls:" << regex_calls << endl;
                        }
                        else
                        {
                            uint32_t aid = getAttrIDForString( eaname );
                            if( aid )
                            {
                                cerr << "eaname:" << eaname << " aid:" << aid << endl;
//                                uint32_t vid = getValueIDForString( value );

//                                compare_operator_regex cop( rex );
//                                ExecuteQueryDocAttrsComparison( q, output, qobj, limit, aid, cop );



                                valuemap_by_vid& vmvid = m_valuemap->get<vid>();
                                valuemap_by_vid::iterator vmvidend = vmvid.end();

//                                 docattrs_by_attrid& dm_aid = m_docattrs->get<attrid>();
//                                 docattrs_by_attrid::iterator it = dm_aid.lower_bound( aid );
//                                 docattrs_by_attrid::iterator  e = dm_aid.upper_bound( aid );

                                docattrs_by_attridvid& dm_aid = m_docattrs->get<attridvid>();
                                docattrs_by_attridvid::iterator it = dm_aid.lower_bound( aid );
                                docattrs_by_attridvid::iterator  e = dm_aid.upper_bound( aid );
                                
                                for( ; it != e; ++it )
                                {
                                    uint32_t vid = it->vid;

                                    valuemap_by_vid::iterator viter = vmvid.find( vid );
                                    if( viter != vmvidend )
                                    {
                                        if( regex_search( viter->value.c_str(), rex ) )
                                        {
                                            cerr << "have a match!" << endl;
                                            output.insert( it->urlid );
                                        }
                                    }
                                }







                                

                                
                                
//                                 uint32_t vid = 0;
//                                 valuemap_collectin::iterator iter = m_valuemap->begin();
//                                 valuemap_collectin::iterator    e = m_valuemap->end();
//                                 for( ; iter != e; ++iter )
//                                 {
//                                     if( regex_search( iter->value.c_str(), rex ) )
//                                     {
//                                         vid = iter->vid;
//                                         cerr << "value:" << value << " iter->value:" << iter->value << " vid:" << vid << endl;

//                                         docattrs_by_attridvid& m = m_docattrs->get<attridvid>();
//                                         docattrs_by_attridvid::iterator it = m.find( boost::make_tuple( aid, vid ) );
//                                         if( it != m.end() )
//                                         {
//                                             cerr << "have a match!" << endl;
//                                             output.insert( it->urlid );
//                                         }
//                                     }
//                                 }

                            }

                            if( eaname == "debug" )
                            {
                                
                                // DEBUG
                                {
                                    cerr << "--------docattrs follows ---------" << endl;
                                    docattrs_collectin::iterator it = m_docattrs->begin();
                                    docattrs_collectin::iterator  e = m_docattrs->end();
                                    for( ; it != e; ++it )
                                    {
                                        cerr << "urlid:" << it->urlid << " attrid:" << it->attrid << " vid:" << it->vid << endl;
                                    }
                                }
                                // DEBUG
                                {
                                    cerr << "--------attrmap follows ---------" << endl;
                                    attrmap_collectin::iterator it = m_attrmap->begin();
                                    attrmap_collectin::iterator  e = m_attrmap->end();
                                    for( ; it != e; ++it )
                                    {
                                        cerr << " attrid:" << it->attrid << " value:" << it->value << endl;
                                    }
                                }
                                // DEBUG
                                {
                                    cerr << "--------urlmap follows ---------" << endl;
                                    urlmap_collectin::iterator it = m_urlmap->begin();
                                    urlmap_collectin::iterator  e = m_urlmap->end();
                                    for( ; it != e; ++it )
                                    {
                                        cerr << " urlid:" << it->urlid << " earl:" << it->earl << endl;
                                    }
                                }
                                
                            }
                            
                            
                            
                        }
                    }
                    
                    
                    return output;
                }
            


            virtual void cleanDocumentIDCache()
                {
                }

            virtual std::string resolveDocumentID( docid_t d )
                {
                    std::string ret;

                    urlmap_by_urlid::iterator iter = m_urlmap->get<urlid>().find( d );
                    if( iter != m_urlmap->end() )
                    {
                        ret = iter->earl.c_str();
                    }
                    else
                    {
                        cerr << "FAILED to resolve urlid:" << d << endl;
                    }
                    return ret;
                }

            virtual void compact( fh_ostream oss, bool verbose = false )
                {
                    //
                    // update the regex prefilter
                    //
                    {
                        urlmap_collectin::iterator iter = m_urlmap->begin();
                        urlmap_collectin::iterator    e = m_urlmap->end();
                        for( ; iter != e; ++iter )
                        {
                            iter->updateEarlBits();
                        }
                    }
                    
                    //
                    // Remove any URLs which do not resolve into the multiversion
                    //
                    {
                        urlmap_collectin::iterator iter = m_urlmap->begin();
                        urlmap_collectin::iterator    e = m_urlmap->end();
                        for( ; iter != e; )
                        {
                            if( canResolve( iter->earl.c_str() ) )
                            {
                                ++iter;
                            }
                            else
                            {
                                LG_EAIDX_D << "removing non resolvable url:" << iter->earl.c_str() << endl;
                                iter = m_urlmap->erase( iter );
                            }
                        }
                    }
                    
                    sync();
                }
            
        public:

            virtual stringset_t&
            getValuesForAttribute( stringset_t& ret, const std::string& eaname, AttrType_t att = ATTRTYPEID_CIS )
                {
                    uint32_t aid = getAttrIDForString( eaname );
                    if( aid )
                    {
//                         docattrs_by_attrid& dm_aid = m_docattrs->get<attrid>();
//                         docattrs_by_attrid::iterator it = dm_aid.lower_bound( aid );
//                         docattrs_by_attrid::iterator  e = dm_aid.upper_bound( aid );

                        docattrs_by_attridvid& dm_aid = m_docattrs->get<attridvid>();
                        docattrs_by_attridvid::iterator it = dm_aid.lower_bound( aid );
                        docattrs_by_attridvid::iterator  e = dm_aid.upper_bound( aid );
                        for( ; it != e; ++it )
                        {
                            uint32_t vid = it->vid;
                            std::string v = getValueForVID( vid );
                            if( !v.empty() )
                            {
                                ret.insert( v );
                            }
                        }
                    }
                }

            fh_fwdeaidx
            tryToCreateForwardEAIndexInterface()
                {
                    return new ForwardEAIndexInterfaceBoost( this );
                }
            
            std::string
            FWDgetStrAttr( Context* c,
                        const std::string& earl,
                        const std::string& rdn,
                        const std::string& def,
                        bool throw_for_errors )
                {
                    uint32_t aid = getAttrIDForString( rdn );
                    LG_EAIDX_D << "FWDgetStrAttr() aid:" << aid << " rdn:" << rdn << " earl:" << earl << endl;
                    if( aid )
                    {
                        urlmap_by_earl& m = m_urlmap->get<byearl>();
                        urlmap_by_earl::iterator it = m.find( earl );
                        urlmap_by_earl::iterator  e = m.end();
                        if( it != e )
                        {
                            uint32_t urlid = it->urlid;
                            LG_EAIDX_D << "urlid:" << urlid << endl;
                            
                            docattrs_by_urlattr& m = m_docattrs->get<urlattr>();
                            docattrs_by_urlattr::iterator it = m.find( boost::make_tuple( urlid, aid ) );
                            docattrs_by_urlattr::iterator  e = m.end();
                            if( it != e )
                            {
                                std::string v = getValueForVID( it->vid );
                                LG_EAIDX_D << "v:" << v << endl;
                                return v;
                            }
                        }
                    }
                    
                    if( throw_for_errors )
                    {
                        std::stringstream ss;
                        ss << "No precached attribute from ferris metadata index for url:" << earl
                           << " attribute:" << rdn << endl;
                        Throw_NoSuchAttribute( tostr(ss), 0 );
                    }
                    return def;
                }
        };
//         class BoostMMapEAIndexer;
//         FERRIS_SMARTPTR( BoostMMapEAIndexer, fh_BoostMMapEAIndexer );


        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        ForwardEAIndexInterfaceBoost::ForwardEAIndexInterfaceBoost( fh_BoostMMapEAIndexer idx )
            :
            m_idx( idx )
        {
        }
        
        std::string
        ForwardEAIndexInterfaceBoost::getStrAttr( Context* c,
                                                  const std::string& earl,
                                                  const std::string& rdn,
                                                  const std::string& def,
                                                  bool throw_for_errors )
        {
            return m_idx->FWDgetStrAttr( c, earl, rdn, def, throw_for_errors );
        }
    };
};

extern "C"
{
    FERRISEXP_API Ferris::EAIndex::MetaEAIndexerInterface* Create()
    {
        return new Ferris::EAIndex::BoostMMapEAIndexer();
    }
};


