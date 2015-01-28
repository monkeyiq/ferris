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

    $Id: libeaidxcustomferrisdb4tree.cpp,v 1.3 2010/09/24 21:31:23 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <climits>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/split_member.hpp>

#include <Ferris/EAIndexer.hh>
#include <Ferris/EAIndexer_private.hh>
#include <Ferris/FullTextIndexer_private.hh>
#include <Indexing/IndexPrivate.hh>
#include <Ferris/Configuration_private.hh>
#include <Ferris/Trimming.hh>
#include <Ferris/Iterator.hh>
#include <Indexing/IndexPrivate.hh>
#include <Ferris/EAQuery.hh>
#include <Ferris/FilteredContext_private.hh>

#include "libeaidxcustomferrisdb4tree.hh"

#include <numeric>
#ifndef STLPORT
#include <ext/numeric>
#endif

// Warning, codepaths for binary serialization are not tested
#define USE_ASCII_READABLE_SERIALIZATION
#define     ASCII_READABLE_SERIALIZATION_INTEGER_PREPAD_SIZE 32

using namespace std;

namespace Ferris
{
    namespace EAIndex 
    {
        typedef guint16 ValueSize_t;


        
        
        const string IDXMGR_NEXT_AID = "idxmgr-next-aid";
        const string IDXMGR_NEXT_VID = "idxmgr-next-vid";
        const string LEXICON_RAW = "Uncompressed (db4 hash)";
        const string LEXICON_3IN4 = "FrontCodedBlocks (3-in-4)";
        const string IDXMGR_ATTRIBUTENAMEMAP_LEXICON_CLASS_K = "index-manager-attributenamemap-lexicon-class";
        const string IDXMGR_ATTRIBUTENAMEMAP_LEXICON_CLASS_DEFAULT = LEXICON_RAW;
        const string IDXMGR_REVERSE_VALUEMAP_LEXICON_CLASS_K = "index-manager-reverse-value-lexicon-class";
        const string IDXMGR_REVERSE_VALUEMAP_LEXICON_CLASS_DEFAULT = LEXICON_RAW;
        const string IDXMGR_SCHEMAVALUEMAP_LEXICON_CLASS_K = "index-manager-schemavaluemap-lexicon-class";
        const string IDXMGR_SCHEMAVALUEMAP_LEXICON_CLASS_DEFAULT = LEXICON_RAW;


        /**
         * get the string name for the enum type
         */
        static string invertedSortTypeToString( invertedSort_t v )
        {
            switch( v )
            {
            case INV_INT:    return "int";
            case INV_DOUBLE: return "float";
            case INV_CIS:    return "cis";
            case INV_STRING: return "string";
            }
        }

        /**
         * convert a string into a enum type
         */
        static invertedSort_t stringToInvertedSortType( const std::string& s )
        {
            if( s == "cis" )
                return INV_CIS;
            if( s == "int" || s == "long" )
                return INV_INT;
            if( s == "float" || s == "double" )
                return INV_DOUBLE;
            return INV_STRING;
        }

        static string dbt_tostr( const DBT* a )
        {
            string ret;
            ret.resize( a->size );
            memcpy( (char*)ret.data(), a->data, a->size );
            return ret;
        }
        
        struct QuadKey
        {
            aid_t aid;
            invertedSort_t st;
            string value;
            urlid_t urlid;
            
            QuadKey( aid_t aid, invertedSort_t st, const std::string& value, urlid_t urlid = 0 )
                :
                aid( aid ), st( st ), value( value ), urlid( urlid )
                {
                }

            string key()
                {
                    stringstream ss;
                    
#ifdef USE_ASCII_READABLE_SERIALIZATION

                    char comma = '-';
                    ss << aid << comma;
                    char tchar = '0' + st;
                    ss << tchar << comma;
                    if( st == INV_CIS || st == INV_STRING )
                    {
                        int len = value.length();
                        ss << len << comma;
                        ss << value << comma;
                    }
                    else
                    {
                        ss << setfill('0') << setw(ASCII_READABLE_SERIALIZATION_INTEGER_PREPAD_SIZE) << value << comma;
//                        ss << value << comma;
                    }
                    if( urlid )
                        ss << urlid;
                    
#else
                    writenum( ss, aid  );
                    unsigned char stchar = st;
                    writenum( ss, stchar );

                    if( st == INV_CIS || st == INV_STRING )
                    {
                        int len = value.length();
                        writenum( ss, len );
                        ss << value;
                    }
                    else
                    {
                        double v = toType<double>( value );
                        LG_EAIDX_D << "quad::key() is numeric type v:" << v << endl;
                        writenum( ss, v );
                    }

                    if( urlid )
                    {
                        LG_EAIDX_D << "quad::key()  urlid:" << urlid << endl;
                        writenum( ss, urlid );
                    }
                    

#endif
                    return ss.str();
                }

            static QuadKey read_all_that_is_available( const std::string& kstring )
                {
                    aid_t aid = 0;
                    invertedSort_t st = INV_CIS;
                    std::string value = "";
                    urlid_t urlid = 0;
                    char comma;
                    char tchar;
                    
                    stringstream ss;
                    ss << kstring;
                    ss >> aid;
                    ss >> comma;
                    ss >> tchar;
                    st = (invertedSort_t)(tchar - '0');
                    ss >> comma;
                    
                    if( st == INV_CIS || st == INV_STRING )
                    {
                        int len = 0;
                        ss >> len;
                        ss >> comma;

                        char* tbuf = (char*)malloc( len+1 );
                        ss.read( tbuf, len );
                        tbuf[len] = '\0';
                        value = tbuf;
                        free( tbuf );
                        ss >> comma;
                    }
                    else
                    {
                        double v = 0;
                        ss >> v >> comma;
                        
                        {
                            stringstream z;
                            z << v;
                            z >> value;
                        }
                    }
                    ss >> urlid;

                    LG_EAIDX_D << "read_all_that_is_available() string:" << kstring << endl;
                    LG_EAIDX_D << "read_all_that_is_available() aid:" << aid
                               << " st:" << st
                               << " value:" <<  value
                               << " urlid:" << urlid
                               << endl;
                    
                    QuadKey ret( aid, st, value, urlid );
                    return ret;
                }
            
            static urlid_t read_urlid( const std::string& kstring )
                {
#ifdef USE_ASCII_READABLE_SERIALIZATION
                    {
                        QuadKey qk = read_all_that_is_available( kstring );
                        return qk.urlid;
                    }
#else
                    int urlid_offset = 1 + sizeof(aid_t);

                    LG_EAIDX_D << "read_urlid(top) kstring:" << kstring << endl;
                    
                    urlid_t ret = 0;
                    if( kstring.length() > 5 )
                    {
                        invertedSort_t st = INV_CIS;
                        memcpy( &st, kstring.data()+sizeof(aid_t), 1 );
                        LG_EAIDX_D << "read_urlid(top) st:" << st << endl;
                        LG_EAIDX_D << "read_urlid(top) sz:" << sizeof(aid_t) << endl;
                        if( st == INV_CIS || st == INV_STRING )
                        {
                            int slen = 0;
                            memcpy( &slen, kstring.data()+5, sizeof(int) );
                            urlid_offset += sizeof(int) + slen;
                        }
                        else
                        {
                            urlid_offset += sizeof(double);
                        }

                        LG_EAIDX_D << "urlid_offset:" << urlid_offset << endl;
                        LG_EAIDX_D << "substr:" << kstring.substr( urlid_offset ) << endl;
                        stringstream ss;
                        ss << kstring.substr( urlid_offset );
                        readnum( ss, ret );
                        LG_EAIDX_D << "ret:" << ret << endl;
                    }
                    return ret;
#endif
                }
            
            static QuadKey read( const std::string& kstring )
                {
#ifdef USE_ASCII_READABLE_SERIALIZATION
                    {
                        QuadKey qk = read_all_that_is_available( kstring );
                        return qk;
                    }
#else
                    aid_t aid = 0;
                    unsigned char stchar = 0;
                    invertedSort_t st = INV_CIS;
                    string value = "";
                    urlid_t urlid = 0;

                    stringstream ss;
                    ss << kstring;
                    
                    readnum( ss, aid  );
                    readnum( ss, stchar );
                    st = (invertedSort_t)stchar;

                    if( st == INV_CIS || st == INV_STRING )
                    {
                        int len = value.length();
                        readnum( ss, len );
                        ss << value;
                    }
                    else
                    {
                        double v = 0;
                        readnum( ss, v );
                        stringstream z;
                        z << v;
                        z >> value;
                    }

                    readnum( ss, urlid );
                    
                    QuadKey ret( aid, st, value, urlid );
                    return ret;
#endif
                }

            static aid_t get_aid( const DBT* k )
                {
#ifdef USE_ASCII_READABLE_SERIALIZATION
                    {
                        string kstring = dbt_tostr( k );
                        QuadKey qk = read_all_that_is_available( kstring );
                        return qk.aid;
                    }
#else
                    aid_t ret = 0;
                    if( k->size < 4 )
                        return ret;
                    
                    memcpy( &ret, k->data, sizeof(aid_t));
                    return ret;
#endif
                }

            static invertedSort_t get_sort( const DBT* k )
                {
#ifdef USE_ASCII_READABLE_SERIALIZATION
                    {
                        string kstring = dbt_tostr( k );
                        QuadKey qk = read_all_that_is_available( kstring );
                        return qk.st;
                    }
#else
                    invertedSort_t ret = INV_CIS;
                    if( k->size < 5 )
                        return ret;
                    
                    memcpy( &ret, ((const char*)k->data)+sizeof(aid_t), 1);
                    return ret;
#endif
                }

            static string get_value( const DBT* k, invertedSort_t st )
                {
#ifdef USE_ASCII_READABLE_SERIALIZATION
                    {
                        string kstring = dbt_tostr( k );
                        QuadKey qk = read_all_that_is_available( kstring );
                        return qk.value;
                    }
#else
                    string ret;

                    if( k->size < 6 )
                        return ret;

//                     ret.resize( k->size-5 );
//                     memcpy( (char*)ret.data(), ((const char *)k->data)+5, k->size-5 );

                    if( st == INV_CIS || st == INV_STRING )
                    {
                        int len = 0;
                        memcpy( &len, ((const char *)k->data)+5, sizeof(int) );
                        ret.resize( len );
                        memcpy( (char*)ret.data(), ((const char *)k->data)+( 5 + sizeof(int) ), len );
                    }
                    else
                    {
                        double v = 0;
                        memcpy( &v, ((const char *)k->data)+5, sizeof(double) );
                        stringstream ss;
                        ss << v;
                        ss >> ret;
                    }

                    return ret;
#endif
                }

            static int value_compare( const DBT* ak, invertedSort_t ast, const DBT* bk, invertedSort_t bst )
                {
                    LG_EAIDX_D << "value_compare() " << endl;

//                     {
//                         string a = dbt_tostr(ak);
//                         string b = dbt_tostr(bk);
//                         if( a == b )
//                             return 0;
//                         return a<b;
//                     }

#ifdef USE_ASCII_READABLE_SERIALIZATION

                    QuadKey a = read_all_that_is_available( dbt_tostr(ak) );
                    QuadKey b = read_all_that_is_available( dbt_tostr(bk) );
                    invertedSort_t st = ast;
                    if( st == INV_CIS || st == INV_STRING )
                    {
                        if( a.value < b.value )
                            return -1;
                        if( a.value > b.value )
                            return 1;
                    }
                    else
                    {
                        double av = toType<double>(a.value);
                        double bv = toType<double>(b.value);
                        if( av < bv )
                            return -1;
                        if( av > bv )
                            return 1;
                    }

                    if( a.urlid == b.urlid )
                        return 0;
                    if( a.urlid < b.urlid )
                        return -1;
                    return 1;
#else                    
                    
                    int ret = 0;

                    if( ak->size < 6 && bk->size < 6 )
                    {
                        LG_EAIDX_D << "value_compare(1)" << endl;
                        return ret;
                    }
                    if( ak->size < 6 )
                    {
                        LG_EAIDX_D << "value_compare(2)" << endl;
                        return -1;
                    }
                    if( bk->size < 6 )
                    {
                        LG_EAIDX_D << "value_compare(3)" << endl;
                        return -1;
                    }

                    if( ast != bst )
                    {
                        LG_EAIDX_D << "value_compare(4)" << endl;
                        return ast < bst;
                    }
                    

                    invertedSort_t st = ast;
                    if( st == INV_CIS || st == INV_STRING )
                    {
                        string astr, bstr;
                        LG_EAIDX_D << "value_compare(STR)" << endl;
                        
                        {
                            int len = 0;
                            memcpy( &len, ((const char *)ak->data)+5, sizeof(int) );
                            astr.resize( len );
                            memcpy( (char*)astr.data(), ((const char *)ak->data)+( 5 + sizeof(int) ), len );
                        }
                        {
                            int len = 0;
                            memcpy( &len, ((const char *)bk->data)+5, sizeof(int) );
                            astr.resize( len );
                            memcpy( (char*)astr.data(), ((const char *)bk->data)+( 5 + sizeof(int) ), len );
                        }
                        
                        if( astr.empty() && bstr.empty() )
                            return 0;
                        if( astr.empty() )
                            return -1;
                        if( bstr.empty() )
                            return 1;

                        if( astr == bstr )
                            return 0;
                        if( astr < bstr )
                            return -1;
                        return 1;
                    }
                    else
                    {
                        double av = 0;
                        double bv = 0;
                        
                        memcpy( &av, ((const char *)ak->data)+5, sizeof(double) );
                        memcpy( &bv, ((const char *)bk->data)+5, sizeof(double) );

                        LG_EAIDX_D << "value_compare() ak:" << dbt_tostr(ak) << endl;
                        LG_EAIDX_D << "value_compare() bk:" << dbt_tostr(bk) << endl;
                        LG_EAIDX_D << "value_compare() av:" << av << " bv:" << bv << endl;
                        
                        if( av == bv )
                        {
                            LG_EAIDX_D << "value_compare(EQUAL)" << endl;
                            urlid_t ai = 0, bi = 0;
                            memcpy( &ai, ((const char *)ak->data)+5+ sizeof(double), sizeof(urlid_t) ); 
                            memcpy( &bi, ((const char *)bk->data)+5+ sizeof(double), sizeof(urlid_t) );
                            LG_EAIDX_D << "value_compare(EQUAL) ai:" << ai << " bi:" << bi  << endl;

                            if( ai == bi )
                                return 0;
                            if( ai < bi )
                                return -1;
                            return 1;
                        }
                        if( av < bv )
                            return -1;
                        return 1;
                    }

                    return 0;
#endif
                }
        };
        

        typedef map<DB*, fh_invertedfiledb4tree > bt_compare_lookup_t;
        bt_compare_lookup_t& get_bt_compare_lookup()
        {
            static bt_compare_lookup_t ret;
            return ret;
        }

        size_t bt_prefix_fcn(DB *rawdb, const DBT *a, const DBT *b)
        {
            cerr << "bt_prefix_fcn()" << endl;
            LG_EAIDX_D << "bt_prefix_fcn()" << endl;
            size_t cnt, len;
            u_int8_t *p1, *p2;

            cnt = 1; len = a->size > b->size ? b->size : a->size;

            for (p1 = (u_int8_t*)a->data, p2 = (u_int8_t*)b->data; len--; ++p1, ++p2, ++cnt)
                if (*p1 != *p2)
                    return (cnt);
            /*
             * They match up to the smaller of the two sizes.
             * Collate the longer after the shorter.
             */
            if (a->size < b->size)
                return (a->size + 1);
            if (b->size < a->size)
                return (b->size + 1);
            return (b->size);
        }
        
        FERRISEXP_DLLLOCAL int bt_compare_aidvid_cs(DB* rawdb, const DBT* a, const DBT* b )
        {
            cerr << "bt_compare_aidvid_cs()" << endl;
            LG_EAIDX_D << "bt_compare_aidvid_cs()" << endl;

#ifdef USE_ASCII_READABLE_SERIALIZATION
            {
                {
                    string qa = dbt_tostr(a);
                    string qb = dbt_tostr(b);
                    if( qa < qb )
                        return -1;
                    if( qa > qb )
                        return 1;
                    return 0;
                    
                }
                
                QuadKey qa = QuadKey::read_all_that_is_available( dbt_tostr(a) );
                QuadKey qb = QuadKey::read_all_that_is_available( dbt_tostr(b) );

                if( qa.aid < qb.aid )
                    return -1;
                else if( qa.aid > qb.aid )
                    return 1;
                else
                {
                    {
                        double av = toType<double>(qa.value);
                        double bv = toType<double>(qb.value);
                        if( av < bv )
                            return -1;
                        if( av > bv )
                            return 1;
                    }

                    if( qa.urlid < qb.urlid )
                        return -1;
                    if( qa.urlid > qb.urlid )
                        return 1;

                }
                return 0;
            }
#else
                    
//                     {
//                         string astr = dbt_tostr(a);
//                         string bstr = dbt_tostr(b);
//                         if( astr == bstr )
//                             return 0;
//                         return astr<bstr;
//                     }
                    
            
            aid_t a_aid = QuadKey::get_aid( a );
            aid_t b_aid = QuadKey::get_aid( b );

            if( a_aid < b_aid )
            {
                return -1;
            }
            if( a_aid > b_aid )
            {
                return 1;
            }

            invertedSort_t a_sort = QuadKey::get_sort( a );
            invertedSort_t b_sort = QuadKey::get_sort( b );
//             if( a_sort < b_sort )
//             {
//                 return -1;
//             }
//             if( a_sort > b_sort )
//             {
//                 return 1;
//             }

            int ret = QuadKey::value_compare( a, a_sort, b, b_sort );
            return ret;
#endif
        }
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        InvertedFileDB4Tree::InvertedFileDB4Tree( fh_db4treeidx m_idx, fh_env dbenv,
                                    invertedSort_t m_sortType,
                                    const std::string& basepath,
                                    const std::string& filename )
            :
            m_sortType( m_sortType ),
            m_dbenv( dbenv ),
            m_db( 0 ),
            m_idx( m_idx ),
            m_basepath( basepath ),
            m_filename( filename )
        {
            if( m_filename.empty() )
                m_filename = "invertedfile.db";
        }
        
        InvertedFileDB4Tree::~InvertedFileDB4Tree()
        {
        }

        fh_db4treeidx
        InvertedFileDB4Tree::getIndex()
        {
            return m_idx;
        }

        
        static int ValueCompare_string( const std::string& s1, const std::string& s2 )
        {
            return strcmp( s1.c_str(), s2.c_str() );
        }
        static int ValueCompare_cis( const std::string& s1, const std::string& s2 )
        {
            return cmp_nocase( s1, s2 );
        }
        template <class T>
        static int ValueCompare_numeric( const std::string& s1, const std::string& s2 )
        {
            T vs1 = toType<T>( s1 );
            T vs2 = toType<T>( s2 );

//             LG_EAIDX_D << "ValueCompare_numeric() vs1:" << vs1 << " vs2:" << vs2
//                  << " ret:" << (vs1 < vs2 ? -1 : (vs1 > vs2 ? 1 : 0 ))
//                  << endl;
            
            if( vs1 < vs2 ) return -1;
            if( vs1 > vs2 ) return  1;
            return 0;
        }
        
        
        
        

        

        

        void
        InvertedFileDB4Tree::setImplicitTransaction( fh_trans t )
        {
            getDB()->setImplicitTransaction( t );
        }
        
        
        void
        InvertedFileDB4Tree::put( aid_t aid, XSDBasic_t scid, const std::string& value, urlid_t urlid )
        {
            LG_EAIDX_D << "InvertedFile::put() aid:" << aid << " scid:" << scid
                       << " value:" << value << " urlid:" << urlid << endl;
            fh_database db = getDB();
            QuadKey q( aid, m_sortType, value, urlid );
            LG_EAIDX_D << "InvertedFile::put() q.key():" << q.key() << endl;
            db->set( q.key(), "" );
        }
        


        
        

        
        
        
        void
        InvertedFileDB4Tree::sync()
        {
            getDB()->sync();
        }

        void
        InvertedFileDB4Tree::compact( fh_ostream oss, bool verbose )
        {
            sync();
        }
        
        

        
        fh_database
        InvertedFileDB4Tree::getDB()
        {
            if( !m_db )
            {
                m_db = new Database( m_dbenv );
                get_bt_compare_lookup()[ m_db->raw() ] = this;
//                m_db->set_bt_compare( bt_compare_aidvid_cs );
//                m_db->set_bt_prefix( bt_prefix_fcn );
                
                m_db->create( DB_BTREE, CleanupURL( m_basepath + "/" + m_filename ), "", DB_AUTO_COMMIT );
                m_db->setAdvanceFunctor( Database::makeAdvanceFunctorNumeric() );
            }
            
            return m_db;
        }
        
        urllist_id_t
        InvertedFileDB4Tree::getNumberOfItems()
        {
            return getDB()->size();
        }

        void
        InvertedFileDB4Tree::dumpTo( fh_ostream oss, bool asXML )
        {
            try
            {
                if( asXML ) oss << "<invertedfile st=\"" << invertedSortTypeToString( m_sortType ) << "\" "
                                << " sortTyperaw=\"" << m_sortType << "\" >" << endl;
                fh_database db = getDB();

                for( Database::iterator iter = db->begin(); iter != db->end(); ++iter )
                {
                    string k;
                    iter.getKey( k );
                    LG_EAIDX_D << "InvertedFile::dump() k:" << k << endl;
                    QuadKey qk = QuadKey::read( k );

                    oss << "<entry aid=\"" << qk.aid << "\""
                        << "  st=\"" << qk.st << "\" "
                        << "  value=\"" << qk.value << "\" "
                        << "  urlid=\"" << qk.urlid << "\" "
                        << " /> " << endl;
                    
                }

                if( asXML ) oss << "</invertedfile>" << endl;
            }
            catch( exception& e )
            {
                oss << "<error> Error traversing data:" << e.what() << " </error></invertedfile>" << endl;
            }
        }
        
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/


        EAIndexManagerDB4Tree::EAIndexManagerDB4Tree()
            :
            m_transaction( 0 ),
            m_cachedAttributeNameMap( 0 ),
            m_dbenv( 0 ),
            m_invertedfiles( 0 ),
            m_attributeNameMap(0),
            m_filesIndexedCount( 0 ),
            m_filesIndexedSinceAnalyseCount( 0 ),
//             m_schemaValueMap(0),
//             m_reverseValueMap(0),
            m_docmap(0)
        {
            int m_invertedfiles_sz = INV_LAST;
            ++m_invertedfiles_sz;
            m_invertedfiles = new fh_invertedfiledb4tree[ m_invertedfiles_sz ];
        }

        MetaEAIndexerInterface*
        EAIndexManagerDB4Tree::Create()
        {
            return new EAIndexManagerDB4Tree();
        }
        



        void
        EAIndexManagerDB4Tree::CreateIndex( fh_context c, fh_context md )
        {
            m_basepath = c->getDirPath();
            
            string attributeMap_class
                = getStrSubCtx( md, "attribute-name-mapping",
                                IDXMGR_ATTRIBUTENAMEMAP_LEXICON_CLASS_DEFAULT );

            string attributesNotToIndex
                = getStrSubCtx( md, "attributes-not-to-index",
                                GET_EAINDEX_EANAMES_NOT_TO_INDEX_DEFAULT() );
            string attributesNotToIndexRegex
                = getStrSubCtx( md, "attributes-not-to-index-regex",
                                GET_EAINDEX_EANAMES_REGEX_IGNORE_DEFAULT() );

            // FIXME: eanames_ignore_regex should be null seperated, but from the gfcreate
            // it will be comma seperated at current.
            replace( attributesNotToIndexRegex.begin(), attributesNotToIndexRegex.end(),
                     ',','\0' );
        
            string dgap_code      = getStrSubCtx( md, "document-number-gap-code",
                                                  IDXMGR_EA_DGAP_CODE_DEFAULT );
            string maxValueSize   = getStrSubCtx( md, "max-value-size-to-index",
                                                  GET_EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT());
            
//             Shell::acquireContext( m_basepath );
//             ensureConfigFileCreated();

            setConfig( IDXMGR_ATTRIBUTENAMEMAP_LEXICON_CLASS_K, attributeMap_class );
            setConfig( IDXMGR_EANAMES_IGNORE_K,       attributesNotToIndex );
            setConfig( IDXMGR_EANAMES_REGEX_IGNORE_K, attributesNotToIndexRegex );
            setConfig( IDXMGR_MAX_VALUE_SIZE_K,       maxValueSize );

            string attributesToIndexRegex
                = getStrSubCtx( md, "attributes-to-index-regex",
                                GET_EAINDEX_EANAMES_TO_INDEX_REGEX_DEFAULT() );
            setConfig( IDXMGR_EANAMES_TO_INDEX_REGEX_K, attributesToIndexRegex );
            
            setDocumentNumberGapCode( dgap_code );
        
//            common_construction();

            ensureAttrNameValueReverseValueMapsCreated();

            
            ensure_docmap();
        }
        

        void
        EAIndexManagerDB4Tree::ensureAttrNameValueReverseValueMapsCreated()
        {
            string attributeNameLexiconClass = getConfig(
                IDXMGR_ATTRIBUTENAMEMAP_LEXICON_CLASS_K,
                IDXMGR_ATTRIBUTENAMEMAP_LEXICON_CLASS_DEFAULT );

            if( !m_attributeNameMap )
            {
                m_attributeNameMap = LexiconFactory::Instance().
                    CreateObject( attributeNameLexiconClass );
                    
                m_attributeNameMap->setPathManager( this );
                m_attributeNameMap->setFileName( "attributemap.lexicon.db" );
                m_attributeNameMap->setIndex( 0 );
            }
        }
        
        void EAIndexManagerDB4Tree::ensure_docmap()
        {
            if( !m_docmap )
            {
                const u_int32_t  environment_open_flags = Environment::get_openflag_create()
                    | Environment::get_openflag_init_txn()
                    | Environment::get_openflag_init_lock()
                    | Environment::get_openflag_init_log()
                    | Environment::get_openflag_init_mpool();
                fh_env dbenv = new Environment( getBasePath(), environment_open_flags );
                m_dbenv = dbenv;
                m_docmap = new DocumentMap( 0, dbenv, this, true );
            }
        }
        
        
        void
        EAIndexManagerDB4Tree::CommonConstruction()
        {
            m_basepath = getPath();
            
            ensureAttrNameValueReverseValueMapsCreated();
            
//             // turn off transaction and logging
//             STLdb4::Environment::setDefaultOpenFlags(
//                 STLdb4::Environment::get_openflag_create()
//                 | STLdb4::Environment::get_openflag_init_mpool() );
            
//            ensureConfigFileCreated();

            ensure_docmap();
            
            m_invertedfiles[ INV_INT ] = new InvertedFileDB4Tree( this, m_dbenv, INV_INT,
                                                           getBasePath(),
                                                           "invertedfile.int.db" );
            m_invertedfiles[ INV_DOUBLE ] = new InvertedFileDB4Tree( this,  m_dbenv, INV_DOUBLE,
                                                              getBasePath(),
                                                              "invertedfile.double.db" );
            m_invertedfiles[ INV_CIS ] = new InvertedFileDB4Tree( this,  m_dbenv, INV_CIS,
                                                           getBasePath(),
                                                           "invertedfile.cis.db" );
            m_invertedfiles[ INV_STRING ] = new InvertedFileDB4Tree( this,  m_dbenv, INV_STRING,
                                                              getBasePath(),
                                                              "invertedfile.string.db" );

        }

        

        EAIndexManagerDB4Tree::~EAIndexManagerDB4Tree()
        {
            sync();
            delete [] m_invertedfiles;
        }
            
        
        std::string
        EAIndexManagerDB4Tree::getBasePath()
        {
            return m_basepath;
        }
            
        void
        EAIndexManagerDB4Tree::sync()
        {
            LG_EAIDX_D << "EAIndexManagerDB4Tree::sync()" << endl;

            if( m_transaction )
            {
                LG_EAIDX_D << "EAIndexManagerDB4Tree::sync() committing transaction..." << endl;
                m_transaction->commit();
                m_transaction = 0;
            }

            LG_EAIDX_D << "EAIndexManagerDB4Tree::sync(2)" << endl;
            
            if( m_attributeNameMap )
                m_attributeNameMap->sync();
            
//            m_inv->sync();
            m_invertedfiles[ INV_INT    ]->sync();
            m_invertedfiles[ INV_DOUBLE ]->sync();
            m_invertedfiles[ INV_CIS    ]->sync();
            m_invertedfiles[ INV_STRING ]->sync();

            m_docmap->sync();

            m_invertedfiles[ INV_INT    ]->dumpTo( ::Ferris::Factory::fcerr(), 1 );
            
        }
            
        fh_lexicon
        EAIndexManagerDB4Tree::getCachedAttributeNameMap()
        {
            if( !m_cachedAttributeNameMap )
                m_cachedAttributeNameMap = wrapWithCache( getAttributeNameMap() );

            return m_cachedAttributeNameMap;
        }
        

        fh_trans
        EAIndexManagerDB4Tree::ensureTransaction()
        {
            if( m_transaction )
                return m_transaction;
            
            m_transaction = new Transaction( 0, m_dbenv );

            m_invertedfiles[ INV_INT ]->setImplicitTransaction( m_transaction );
            m_invertedfiles[ INV_DOUBLE ]->setImplicitTransaction( m_transaction );
            m_invertedfiles[ INV_CIS ]->setImplicitTransaction( m_transaction );
            m_invertedfiles[ INV_STRING ]->setImplicitTransaction( m_transaction );
            
            return m_transaction;
        }
        
        void
        EAIndexManagerDB4Tree::refresh_transaction( bool force )
        {
            if( force || m_filesIndexedSinceAnalyseCount > 500 )
            {
                if( m_transaction )
                {
                    LG_EAIDX_D << "EAIndexManagerDB4Tree::refresh_transaction() committing" << endl;
                    m_transaction->commit();
                    m_transaction = 0;
                    LG_EAIDX_D << "EAIndexManagerDB4Tree::refresh_transaction() commit done!" << endl;
                }
                m_filesIndexedSinceAnalyseCount = 0;
                LG_EAIDX_I << "Syncing the db4 files" << endl;
//                sync();
            }
            
            ensureTransaction();
        }
        


        void
        EAIndexManagerDB4Tree::compact( fh_ostream oss, bool verbose )
        {
            for( int i=0; i <= INV_LAST; ++i )
            {
                fh_invertedfiledb4tree inv = m_invertedfiles[ i ];
                if( inv )
                {
                    inv->compact( oss, verbose );
                }
            }
        }
        
        
        
        aid_t
        EAIndexManagerDB4Tree::getNextAID()
        {
            aid_t ret = toint(getConfig( IDXMGR_NEXT_AID, "0" ));
            ++ret;
            setConfig( IDXMGR_NEXT_AID, tostr(ret) );
            return ret;
        }
        

        vid_t
        EAIndexManagerDB4Tree::getNextVID()
        {
            vid_t ret = toint(getConfig( IDXMGR_NEXT_VID, "2" ));
            ++ret;
            setConfig( IDXMGR_NEXT_VID, tostr(ret) );
            return ret;
        }

        std::string
        EAIndexManagerDB4Tree::getDocumentNumberGapCode()
        {
            return getConfig( IDXMGR_EA_DGAP_CODE_K, IDXMGR_EA_DGAP_CODE_DEFAULT );
        }

        void
        EAIndexManagerDB4Tree::setDocumentNumberGapCode( const std::string& codename )
        {
            setConfig( IDXMGR_EA_DGAP_CODE_K, codename );
        }

        fh_lexicon
        EAIndexManagerDB4Tree::getAttributeNameMap()
        {
            return m_attributeNameMap;
        }

        
        fh_docmap
        EAIndexManagerDB4Tree::getDocumentMap()
        {
            return m_docmap;
        }

        fh_invertedfiledb4tree
        EAIndexManagerDB4Tree::getInvertedFile( invertedSort_t v )
        {
            return m_invertedfiles[ v ];
        }

        fh_invertedfiledb4tree
        EAIndexManagerDB4Tree::getInvertedFile( const std::string& sortType )
        {
            return getInvertedFile( stringToInvertedSortType( sortType ));
        }
        
        std::string
        EAIndexManagerDB4Tree::getAttributeNameMapClassName()
        {
            return getConfig( IDXMGR_ATTRIBUTENAMEMAP_LEXICON_CLASS_K,
                              IDXMGR_ATTRIBUTENAMEMAP_LEXICON_CLASS_DEFAULT );
        }
        
//         std::string
//         EAIndexManagerDB4Tree::getValueMapClassName()
//         {
//             return getConfig( IDXMGR_SCHEMAVALUEMAP_LEXICON_CLASS_K,
//                               IDXMGR_SCHEMAVALUEMAP_LEXICON_CLASS_DEFAULT );
//         }
        
//         std::string
//         EAIndexManagerDB4Tree::getReverseValueMapClassName()
//         {
//             return getConfig( IDXMGR_REVERSE_VALUEMAP_LEXICON_CLASS_K,
//                               IDXMGR_REVERSE_VALUEMAP_LEXICON_CLASS_DEFAULT );
//         }

        void
        EAIndexManagerDB4Tree::addDocs( docNumSet_t& docnums,
                                        dbiterpair_t& r )
        {
            Database::iterator iter = r.first;
            Database::iterator    e = r.second;
            LG_EAIDX_D << "EAIndexManagerDB4Tree::addDocs() range size:" << distance( r.first, r.second ) << endl;
            for( ; iter != e ; ++iter )
            {
                LG_EAIDX_D << "EAIndexManagerDB4Tree::addDocs() k:" << iter->first << endl;
                urlid_t id = QuadKey::read_urlid( iter->first );
                LG_EAIDX_D << "EAIndexManagerDB4Tree::addDocs() id:" << id << endl;
                docnums.insert( id );
            }
        }

        string lexi_incr( const string& v_const )
        {
            string v = v_const;

            LG_EAIDX_D << "lexi_incr(top) v:" << v << endl;
            
            if( v.empty() )
                return v;

            int l = v.length();
            --l;

            while( l && (unsigned char)(v[l]) == 255 )
            {
                --l;
            }

            if( l == 0 && (unsigned char)(v[l]) == 255 )
            {
                unsigned char tmp = 0x1;
                stringstream ss;
                ss << tmp;
                ss << v;
                v = ss.str();
            }
            else
            {
                unsigned char tmp = v[l];
                ++tmp;
                v[l] = (unsigned char)(tmp);
            }
            
            LG_EAIDX_D << "lexi_incr(bottom) v:" << v << endl;
            return v;
        }
        
        
        docNumSet_t&
        EAIndexManagerDB4Tree::ExecuteEquals( fh_context q,
                                              docNumSet_t& docnums,
                                              fh_invertedfiledb4tree inv,
                                              aid_t aid,
                                              const std::string& value )
        {
            invertedSort_t st = stringToInvertedSortType( guessComparisonOperatorFromData( value ) );

//            QuadKey quad( aid, st, value, 6 );
            QuadKey quad( aid, st, value );
            string qk = quad.key();

            LG_EAIDX_D << "EAIndexManagerDB4Tree::ExecuteEquals() qk:" << qk << endl;

            
//            dbiterpair_t r = inv->getDB()->equal_range_partial( qk );

            dbiterpair_t r;
//            r.first = inv->getDB()->lower_bound( qk );
            r.first = inv->getDB()->find_partial( qk );
            if( r.first == inv->getDB()->end() )
            {
                LG_EAIDX_D << "EAIndexManagerDB4Tree::ExecuteEquals(end) qk:" << qk << endl;
                return docnums;
            }
            r.second = r.first;
            ++(r.second);
            {
                QuadKey quad( aid, st, lexi_incr(value) );
                LG_EAIDX_D << "EAIndexManagerDB4Tree::ExecuteEquals(a)" << endl;
                r.second = inv->getDB()->find_partial( quad.key() );
                LG_EAIDX_D << "EAIndexManagerDB4Tree::ExecuteEquals(b)" << endl;
//                swap( r.first, r.second );
            }
            
            LG_EAIDX_D << "EAIndexManagerDB4Tree::ExecuteEquals() range size:" << distance( r.first, r.second ) << endl;
            addDocs( docnums, r );
            
//             fh_urllist ul = inv->find( aid, value );
//             addDocs( docnums, ul );
            
        }
        
        docNumSet_t&
        EAIndexManagerDB4Tree::ExecuteLtEq( fh_context q,
                                            docNumSet_t& docnums,
                                            fh_invertedfiledb4tree inv,
                                            aid_t aid,
                                            const std::string& value )
        {
            LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(<=) top aid:" << aid
                       << " value:" << value << endl;

            invertedSort_t st = stringToInvertedSortType( guessComparisonOperatorFromData( value ) );
            Database::iterator begin = inv->getDB()->find_partial( QuadKey( aid-1, st, "" ).key() );
            Database::iterator   end = inv->getDB()->find_partial( QuadKey( aid,   st, lexi_incr(value) ).key() );
            if( begin == inv->getDB()->end() )
            {
                LG_EAIDX_D << "no begin" << endl;
            }
            if( end == inv->getDB()->end() )
            {
                LG_EAIDX_D << "no end" << endl;
            }
            
            dbiterpair_t r = make_pair( begin, end );
            LG_EAIDX_D << "EAIndexManagerDB4Tree::ExecuteQuery(<=) range size:" << distance( begin, end ) << endl;
            addDocs( docnums, r );
            
            
//             InvertedFile::iterator_range range = inv->upper_bound( aid, value );
// //            cerr << "EAQuery_Heur::ExecuteQuery(<=) 1" << endl;
//             urllists_t col = inv->getURLLists( range.begin, range.selected );
            
//             LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(<=) col.sz:" << col.size() << endl;
//             for( urllists_t::iterator li = col.begin(); li != col.end(); ++li )
//             {
//                 fh_urllist ul  = *li;
//                 addDocs( docnums, ul );
//             }
        }
        
        docNumSet_t& EAIndexManagerDB4Tree::ExecuteGtEq( fh_context q,
                                                     docNumSet_t& docnums,
                                                     fh_invertedfiledb4tree inv,
                                                     aid_t aid,
                                                     const std::string& value )
        {
            LG_EAIDX_D << "EAQuery_Heur::ExecuteGtEq(1)" << endl;

            invertedSort_t st = stringToInvertedSortType( guessComparisonOperatorFromData( value ) );
            Database::iterator begin = inv->getDB()->find_partial( QuadKey( aid,   st, value ).key() );
            Database::iterator   end = inv->getDB()->find_partial( QuadKey( aid+1, st, "" ).key() );
            if( begin == inv->getDB()->end() )
                LG_EAIDX_D << "no begin" << endl;
            if( end == inv->getDB()->end() )
                LG_EAIDX_D << "no end" << endl;
            
            dbiterpair_t r = make_pair( begin, end );
            LG_EAIDX_D << "EAIndexManagerDB4Tree::ExecuteQuery(>=) range size:" << distance( begin, end ) << endl;
            addDocs( docnums, r );
        }
        
        
        void
        EAIndexManagerDB4Tree::addToIndex( fh_context c,
                                           fh_docindexer di )
        {
            LG_EAIDX_D << "DocumentIndexer::addContextToIndex(begin) c:" << c->getURL() << endl;

            int attributesDone = 0;
            int signalWindow   = 0;
            stringlist_t slist;
            getEANamesToIndex( c, slist );
            int totalAttributes = slist.size();

            string earl = c->getURL();
            LG_EAIDX_D << "DocumentIndexer::addContextToIndex(1) c:" << earl << endl;
            Time::Benchmark bm( "earl:" + earl );
            bm.start();

            refresh_transaction( true );
            
//             if( !m_transaction || m_filesIndexedSinceAnalyseCount > 500 )
//             {
//                 m_filesIndexedSinceAnalyseCount = 0;
//                 refresh_transaction();
//             }
            
            
            
            //
            // get an identifier for this context
            //
            fh_docmap  docmap   = getDocumentMap();
            fh_doc document     = 0;
            LG_EAIDX_D << "DocumentIndexer::addContextToIndex(2) c:" << c->getURL() << endl;
            if( !di->getDontCheckIfAlreadyThere() )
            {
                LG_EAIDX_D << "DocumentIndexer::addContextToIndex(check for old version) c:" << c->getURL() << endl;
                // see if we already have the URL in the database, if so revoke its ID.
                if( document = docmap->lookup( c ) )
                {
                    ensureRevokedDocumentIDCache( document->getID() );
                    document->revokeDocument();
                }
            }
            document = docmap->append( c );

            LG_EAIDX_D << "DocumentIndexer::addContextToIndex(3) c:" << c->getURL() << endl;
            urlid_t urlid = document->getID();

            fh_lexicon attributeNameMap = getCachedAttributeNameMap();

            for( stringlist_t::iterator si = slist.begin(); si != slist.end(); ++si )
            {
                try
                {
                    bool valueIsNull = false;
                    string attributeName = *si;
                    string k = attributeName;
                    string v = "";
                    if( !obtainValueIfShouldIndex( c, di,
                                                   attributeName, v,
                                                   true, valueIsNull ))
                        continue;

                    LG_EAIDX_D << "addToIndex() attributeName:" << attributeName << endl;
                    LG_EAIDX_D << "addContextToIndex(a) c:" << c->getURL() << endl
                               << " k:" << k << " v:" << v << endl;

                    vid_t vid = Lexicon::NULL_VALUE_TERMID;
                    IndexableValue iv   = getIndexableValue( c, k, v );
                    XSDBasic_t     scid = iv.getSchemaType();
                    if( valueIsNull )
                        vid = Lexicon::UNREADABLE_VALUE_TERMID;

                    //
                    // Add to attrname -> aid mapping
                    //
                    aid_t aid = attributeNameMap->lookup( attributeName );
                    if( !aid )
                    {
                        LG_EAIDX_D << "DocumentIndexer::addContextToIndex() adding new attribute:"
                                   << attributeName << endl;
                        aid = getNextAID();
                        LG_EAIDX_D << "DocumentIndexer::addContextToIndex() gets aid:" << aid << endl;
                        attributeNameMap->insert( attributeName, aid );
                    }

                    LG_EAIDX_D << "DocumentIndexer::addContextToIndex() urlid:" << urlid
                               << " c:" << c->getURL() << endl
                               << endl;

                    //
                    // add urlid to mapping
                    // aid, vid -> { urlid, scid }
                    //
                    fh_context     sc    = c->getSchema( attributeName );
                    invertedSort_t st    =
                        stringToInvertedSortType( getSchemaDefaultSort( sc ) );
                    fh_invertedfiledb4tree inv  = getInvertedFile( st );

                    inv->put( aid, scid, v, urlid );
                }
                catch( exception& e )
                {
                    LG_EAIDX_D << "EXCEPTION e:" << e.what() << endl;
                }
                
                if( signalWindow > 5 )
                {
                    signalWindow = 0;
                    di->getProgressSig().emit( c, attributesDone, totalAttributes );
                }
                ++attributesDone;
                ++signalWindow;
            }

            LG_EAIDX_D << "DocumentIndexer::addContextToIndex(end) c:" << c->getURL() << endl;
            ++m_filesIndexedCount;
            ++m_filesIndexedSinceAnalyseCount;
            
        }

        pair< fh_context, fh_context >
        BuildQuery_getLeftAndRightContexts( fh_context q )
        {
            fh_attribute orderedtla = q->getAttribute("in-order-insert-list");
            fh_istream   orderedtls = orderedtla->getIStream();
            
//             LG_EAIDX_D << "BuildQuery() token:" << tokenfc << endl;
//             LG_EAIDX_D << " q.child count:" << q->SubContextCount() << endl;

            string s;
            getline( orderedtls, s );
            LG_EAIDX_D << "BuildQuery() lc:" << s << endl;
            fh_context lc = q->getSubContext( s );

            fh_context rc = 0;
            if( getline( orderedtls, s ) && !s.empty() )
            {
                LG_EAIDX_D << "EAQuery_Heur::BuildQuery() rc:" << s << endl;
                rc = q->getSubContext( s );
            }
            
            return make_pair( lc, rc );
        }

        
        string 
        BuildQuery_getEAName( fh_context q )
        {
            pair< fh_context, fh_context > p = BuildQuery_getLeftAndRightContexts( q );
            fh_context c = p.first;
            string ret   = getStrAttr( c, "token", "" );
            return ret;
        }
        
        string
        BuildQuery_getToken( fh_context q )
        {
            string token   = getStrAttr( q, "token", "" );
            string tokenfc = foldcase( token );
            return tokenfc;
        }
        
        docNumSet_t&
        EAIndexManagerDB4Tree::ExecuteQuery( fh_context q, docNumSet_t& docnums, int limit )
        {
            string token   = getStrAttr( q, "token", "" );
            string tokenfc = foldcase( token );
            fh_attribute orderedtla = q->getAttribute("in-order-insert-list");
            fh_istream   orderedtls = orderedtla->getIStream();

            LG_EAIDX_D << "EAIndexManagerDB4Tree::ExecuteQuery() token:" << tokenfc << endl;
            LG_EAIDX_D << " q.child count:" << q->SubContextCount() << endl;
            
            string s;
            getline( orderedtls, s );
            LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery() lc:" << s << endl;
            fh_context lc = q->getSubContext( s );

            if( tokenfc == "!" )
            {
                docNumSet_t       ldocs;
                vector< urlid_t > rdocs;
                docNumSet_t       tmp;
                
                ExecuteQuery( lc, ldocs, limit );

//                 docNumSet_t::iterator max = max_element( ldocs.begin(), ldocs.end() );
//                 rdocs.resize( *max );
                
                rdocs.resize( getDocumentMap()->size() );
                iota( rdocs.begin(), rdocs.end(), 1 );

                LG_EAIDX_D << "ldocs.size:" << ldocs.size()
                           << " max:" << getDocumentMap()->size()
                           << endl;
                

                set_difference( rdocs.begin(), rdocs.end(),
                                ldocs.begin(), ldocs.end(),
                                inserter( tmp, tmp.begin() ) );
                docnums.insert( tmp.begin(), tmp.end() );
                docnums.erase( docnums.find( 1 ) );
                return docnums;
            }
            
            
            getline( orderedtls, s );
            LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery() rc:" << s << endl;
            fh_context rc = q->getSubContext( s );

            if( tokenfc == "&" )
            {
                LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(&) detected" << endl;

                typedef map< string, ctxlist_t > SameAttrOps_t;
                SameAttrOps_t SameAttrOps;
                stringset_t TokensToCombine;
                TokensToCombine.insert( "<=" );
                TokensToCombine.insert( ">=" );

                typedef list< fh_context > ctxlist_t;
                ctxlist_t ctxlist;
                for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                    ctxlist.push_front( *ci );

                //
                // Count up all the terms which are using the same EA and operations which can
                // be merged into one subquery.
                //
                for( ctxlist_t::iterator ci = ctxlist.begin(); ci != ctxlist.end(); ++ci )
                {
                    string token = BuildQuery_getToken( *ci );

                    //
                    // Standard eaname >= 1 && eaname <= 5 handling
                    //
                    if( TokensToCombine.find( token ) != TokensToCombine.end() )
                    {
                        string attr  = BuildQuery_getEAName( *ci );
                        SameAttrOps[ attr ].push_back( *ci );
                    }
                }

                //
                // Merge all terms using the same EA which we can into single terms
                //
                for( SameAttrOps_t::iterator sai = SameAttrOps.begin(); sai != SameAttrOps.end(); ++sai )
                {
                    if( sai->second.size() == 2 )
                    {
                        string eaname = sai->first;
                        ctxlist_t& cl = sai->second;

                        LG_EAIDX_D << "Checking if range compaction is possible..." << endl;

                        fh_context ltclause = cl.front();
                        cl.pop_front();
                        fh_context gtclause = cl.front();
                        cl.push_front( ltclause );

                        if( BuildQuery_getToken( ltclause ) == ">=" )
                        {
                            swap( ltclause, gtclause );
                        }
                        if( BuildQuery_getToken( ltclause ) != "<=" || BuildQuery_getToken( gtclause ) != ">=" )
                        {
                            continue;
                        }

                        LG_EAIDX_D << "Trying to compact a <= and >= into a single range lookup..." << endl;
                        
                        aid_t aid = getAttributeNameMap()->lookup( eaname );
                        pair< fh_context, fh_context > ltpr = BuildQuery_getLeftAndRightContexts( ltclause );
                        IndexableValue iv = getIndexableValueFromToken( eaname, ltpr.second );
                        string value = iv.rawValueString();
                        LG_EAIDX_D << "Trying to compact...value:" << value << endl;
                        string sortType = guessComparisonOperatorFromData( value );
                        invertedSort_t st = stringToInvertedSortType( guessComparisonOperatorFromData( value ) );
                        fh_invertedfiledb4tree inv = getInvertedFile( sortType );

                        fh_database db = inv->getDB();
                        Database::iterator begin = db->end();
                        Database::iterator   end = db->end();
                        
                        // handle ExecuteLtEq() / ltclause
                        {
                            pair< fh_context, fh_context > pr = BuildQuery_getLeftAndRightContexts( ltclause );
                            IndexableValue iv = getIndexableValueFromToken( eaname, pr.second );
                            string value = iv.rawValueString();
                            LG_EAIDX_D << "Compaction ltclause. value:" << value << endl;

                            end = db->find_partial( QuadKey( aid, st, lexi_incr(value) ).key() );
                        }
                        // handle ExecuteGtEq() / gtclause
                        {
                            pair< fh_context, fh_context > pr = BuildQuery_getLeftAndRightContexts( gtclause );
                            IndexableValue iv = getIndexableValueFromToken( eaname, pr.second );
                            string value = iv.rawValueString();
                            LG_EAIDX_D << "Compaction gtclause. value:" << value << endl;

                            begin = db->find_partial( QuadKey( aid, st, value ).key() );
                        }
                        
                        if( begin == db->end() )
                        {
                            LG_EAIDX_D << "no begin" << endl;
                        }
                        if( end == db->end() )
                        {
                            LG_EAIDX_D << "no end" << endl;
                        }
                        
                        dbiterpair_t r = make_pair( begin, end );
                        LG_EAIDX_D << "EAIndexManagerDB4Tree::ExecuteQuery(join-combine) range size:" << distance( begin, end ) << endl;
                        addDocs( docnums, r );

                        LG_EAIDX_D << "ctxlist.size 1:" << ctxlist.size() << endl;
                        for( ctxlist_t::iterator ci = cl.begin(); ci != cl.end(); ++ci )
                        {
                            ctxlist.erase( find( ctxlist.begin(), ctxlist.end(), *ci ) );
                        }
                        LG_EAIDX_D << "ctxlist.size 2:" << ctxlist.size() << endl;
                    }
                }

                // Handle remaining sub terms.
                for( ctxlist_t::iterator ci = ctxlist.begin(); ci != ctxlist.end(); ++ci )
                {
                    docNumSet_t qdocs;
                    docNumSet_t intersect;
                    ExecuteQuery( *ci, qdocs, limit );
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
            else if( tokenfc == "|" )
            {
                docNumSet_t tmp1;
                docNumSet_t tmp2;

                docNumSet_t& fromDocs = tmp1;
                docNumSet_t& toDocs   = tmp2;
                
                for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                {
                    docNumSet_t qdocs;
                    ExecuteQuery( *ci, qdocs, limit );
                    set_union( fromDocs.begin(), fromDocs.end(),
                               qdocs.begin(), qdocs.end(),
                               inserter( toDocs, toDocs.begin() ) );
                    swap( fromDocs, toDocs );
                    toDocs.clear();
                }
                docnums.insert( fromDocs.begin(), fromDocs.end() );
                return docnums;
                
            }
            
            
            

            string eaname = getStrAttr( lc, "token", "" );
            string value  = getStrAttr( rc, "token", "" );
            string comparisonOperator = guessComparisonOperatorFromData( value );

            LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery() eaname:" << eaname
                       << " value:" << value << endl;

            fh_lexicon attributeNameMap = getAttributeNameMap();

            aid_t aid = attributeNameMap->lookup( eaname );
            string sortType = guessComparisonOperatorFromData( value );
            LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery() aid:" << aid
                       << " sortType:" << sortType
                       << endl;
            
            fh_invertedfiledb4tree inv = getInvertedFile( sortType );

            if( tokenfc == "==" )
            {
                LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(==) detected" << endl;

                ExecuteEquals( q, docnums, inv, aid, value );
            }
            else if( tokenfc == "=?=" )
            {
                ExecuteEquals( q, docnums, 
                               getInvertedFile( INV_INT ),
                               aid, tostr(convertStringToInteger( value )) );
                ExecuteEquals( q, docnums, 
                               getInvertedFile( INV_DOUBLE ),
                               aid, tostr(convertStringToInteger( value )) );
                ExecuteEquals( q, docnums, 
                               getInvertedFile( INV_CIS ),
                               aid, value );
                ExecuteEquals( q, docnums, 
                               getInvertedFile( INV_STRING ),
                               aid, value );
            }
            else if( tokenfc == "=~" )
            {
                LG_EAIDX_W << "ExecuteQuery db4tree plugin(=~) not fully tested!" << endl;
                cerr << "ExecuteQuery db4tree plugin(=~) not fully tested!" << endl;

                fh_regex reg = new Regex( value );
                invertedSort_t st = stringToInvertedSortType( guessComparisonOperatorFromData( value ) );

                fh_database db = inv->getDB();
                Database::iterator  iter = db->end();
                Database::iterator   end = db->end();

                iter  = db->find_partial( QuadKey( aid-1, st, "" ).key() );
                end   = db->find_partial( QuadKey( aid+1, st, "" ).key() );

                for( ; iter != end; ++iter )
                {
                    string k;
                    iter.getKey( k );
                    QuadKey qk = QuadKey::read( k );

                    string dbv = qk.value;
                    if( !reg->operator()( dbv ) )
                        continue;

                    docnums.insert( qk.urlid );
                }
            }
            else if( tokenfc == ">=" )
            {
                LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(>=) detected" << endl;

                ExecuteGtEq( q, docnums, inv, aid, value );
                LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(>=) executed" << endl;
                
            }
            else if( tokenfc == ">?=" )
            {
                LG_EAIDX_D << " For >?= opcode integer version:"
                           << tostr(convertStringToInteger( value ))
                           << endl;
                
                ExecuteGtEq( q, docnums, 
                             getInvertedFile( INV_INT ),
                             aid, tostr(convertStringToInteger( value )) );
                ExecuteGtEq( q, docnums, 
                             getInvertedFile( INV_DOUBLE ),
                             aid, tostr(convertStringToInteger( value )) );
                ExecuteGtEq( q, docnums, 
                             getInvertedFile( INV_CIS ),
                             aid, value );
                ExecuteGtEq( q, docnums, 
                             getInvertedFile( INV_STRING ),
                             aid, value );
            }
            else if( tokenfc == "<=" )
            {
                LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(<=) detected" << endl;

                ExecuteLtEq( q, docnums, inv, aid, value );
                
            }
            else if( tokenfc == "<?=" )
            {
                ExecuteLtEq( q, docnums, 
                             getInvertedFile( INV_INT ),
                             aid, tostr(convertStringToInteger( value )) );
                ExecuteLtEq( q, docnums, 
                             getInvertedFile( INV_DOUBLE ),
                             aid, tostr(convertStringToInteger( value )) );
                ExecuteLtEq( q, docnums, 
                             getInvertedFile( INV_CIS ),
                             aid, value );
                ExecuteLtEq( q, docnums, 
                             getInvertedFile( INV_STRING ),
                             aid, value );
            }
            
            return docnums;
        }
    
        docNumSet_t&
        EAIndexManagerDB4Tree::ExecuteQuery( fh_context q,
                                         docNumSet_t& output,
                                         fh_eaquery qobj,
                                         int limit )
        {
            return ExecuteQuery( q, output, limit );
        }
        
        std::string
        EAIndexManagerDB4Tree::resolveDocumentID( docid_t docid )
        {
            fh_doc d = getDocumentMap()->lookup( docid );
            fh_context ctx = d->getContext();
            return ctx->getURL();
        }
        

        bool
        EAIndexManagerDB4Tree::isRevokedDocumentID( docid_t d )
        {
            return m_revokedDocumentIDCache.count( d );
        }
        
        void
        EAIndexManagerDB4Tree::removeRevokedDocumentIDCache( docid_t d )
        {
            m_revokedDocumentIDCache.erase( d );
        }
        
        void
        EAIndexManagerDB4Tree::ensureRevokedDocumentIDCache( docid_t d )
        {
            m_revokedDocumentIDCache.insert( d );
        }

        std::string
        EAIndexManagerDB4Tree::getRevokedDocumentIDCacheFileName()
        {
            string ret = getBasePath() + "/" + "revoked-docid-set-cache";
            return ret;
        }
        
        
        void
        EAIndexManagerDB4Tree::loadRevokedDocumentIDCache()
        {
            fh_ifstream iss( getRevokedDocumentIDCacheFileName() );
            string s = StreamToString( iss );
            Util::parseSeperatedList( s,
                                      m_revokedDocumentIDCache,
                                      inserter( m_revokedDocumentIDCache,
                                                m_revokedDocumentIDCache.begin() ) );
        }
        
        void
        EAIndexManagerDB4Tree::saveRevokedDocumentIDCache()
        {
            fh_ofstream oss( getRevokedDocumentIDCacheFileName(),
                             std::ios_base::out | std::ios_base::trunc );
            oss << Util::createSeperatedList( m_revokedDocumentIDCache.begin(),
                                              m_revokedDocumentIDCache.end() );
        }
        
        
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        class FERRISEXP_API EAIndexManagerDB4TreeCreator
        {
        public:
            static Ferris::EAIndex::MetaEAIndexerInterface* Create()
                {
                    return new Ferris::EAIndex::EAIndexManagerDB4Tree();
                }
        };
        
    };
};




extern "C"
{
    FERRISEXP_API Ferris::EAIndex::MetaEAIndexerInterface* Create()
    {
        return Ferris::EAIndex::EAIndexManagerDB4TreeCreator::Create();
    }
};
