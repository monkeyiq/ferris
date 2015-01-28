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

    $Id: libeaidxcustomferris.cpp,v 1.13 2010/09/24 21:31:23 ben Exp $

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

#include "libeaidxcustomferris.hh"

#include <numeric>
#ifndef STLPORT
#include <ext/numeric>
#endif


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

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/


        URLList::URLList( fh_invertedfile inv,
                          Database::iterator dbi,
                          aid_t m_aid,
                          const std::string& value,
                          bool createIfNotThere )
            :
            m_inv( inv ),
            m_aid( m_aid ),
            m_value( value ),
            m_dirty( false )
        {
//            cerr << "URLList::URLList() aid:" << m_aid << " value:" << m_value << endl;
            load( dbi, createIfNotThere );
        }

        URLList::URLList( fh_invertedfile inv,
                          aid_t m_aid,
                          const std::string& value,
                          bool createIfNotThere )
            :
            m_inv( inv ),
            m_aid( m_aid ),
            m_value( value ),
            m_dirty( false )
        {
//            cerr << "URLList::URLList() aid:" << m_aid << " value:" << m_value << endl;
            load( createIfNotThere );
        }
        
        URLList::~URLList()
        {
            if( m_dirty )
                save();
        }
        
        
        void
        URLList::insert( urlid_t urlid, XSDBasic_t scid )
        {
            setDirty( true );
            m_hits.insert( make_pair( urlid, scid ) );
        }

        bool
        URLList::contains( urlid_t urlid )
        {
            return m_hits.end() != m_hits.find( urlid );
        }

        void
        URLList::sync()
        {
            if( isDirty() )
                save();
        }
        
        fh_database
        URLList::getDB()
        {
            return m_inv->getDB();
        }

        static string makePartialKey( aid_t aid )
        {
            fh_stringstream ss;
            ss << aid;
//            writenum( ss, aid );
            return tostr(ss);
        }
        
        string
        URLList::makeKey()
        {
//            cerr << "URLList::makeKey(T)" << endl;
            stringstream ss;
            ValueSize_t vs = m_value.size();
            ss << m_aid << "," << vs << "," << m_value;

//             cerr << "URLList::makeKey()" << endl;
//             cerr << "URLList::makeKey() aid:" << m_aid << endl;
//             cerr << "URLList::makeKey() vs:" << vs << endl;
//             cerr << "URLList::makeKey() value.len:" << m_value.length() << endl;
//             cerr << "URLList::makeKey() value:" << m_value << endl;

            
//             writenum( ss, m_aid );
//             writenum( ss, vs );
//             writestring( ss, m_value );
            
//            ss << m_aid << "," << m_value;
            return ss.str();
        }

        static std::pair< aid_t, std::string >
        readKeyFIXME( const std::string& s )
        {
            std::pair< aid_t, string > ret;

            char c;
            fh_stringstream ss( s );
            ValueSize_t vs = 0;
            ss >> ret.first >> c >> vs >> c;
            ret.second = readstring( ss, vs );

//             readnum( ss, ret.first );
//             ValueSize_t vs = 0;
//             readnum( ss, vs );
//             ret.second = readstring( ss, vs );
            
//             char c;
//             fh_stringstream ss( s );
//             ss >> ret.first >> c >> ret.second;
            return ret;
        }

        
        std::pair< aid_t, std::string >
        URLList::readKey( const std::string& s )
        {
            std::pair< aid_t, string > ret;
            ret.first = 0;
            
            char c;
            fh_stringstream ss( s );
            ValueSize_t vs = 0;
            ss >> ret.first >> c >> vs >> c;
            ret.second = readstring( ss, vs );
            
//             readnum( ss, ret.first );
//             ValueSize_t vs = 0;
//             readnum( ss, vs );
//             ret.second = readstring( ss, vs );
            
//             char c;
//             fh_stringstream ss( s );
//             ss >> ret.first >> c >> ret.second;
            return ret;
        }

        fh_db4idx
        URLList::getIndex()
        {
            return m_inv->getIndex();
        }
        
        FERRISEXP_DLLLOCAL enum URLListFormat_t {
            URLLISTFORMAT_DEBUG_URLID      = 100,
            URLLISTFORMAT_DEBUG_URLID_SCID = 101,
            URLLISTFORMAT_INTERPOLATIVE    = 102,
            URLLISTFORMAT_GOLOMB           = 103,
            URLLISTFORMAT_ELIAS            = 104,
            URLLISTFORMAT_DELTA            = 105,
            URLLISTFORMAT_GAMMA            = 106,
            URLLISTFORMAT_BOOST_UNCOMPRESSED = 110
        };


        static URLListFormat_t GapCodeToIOFormat( const std::string& dgap_code )
        {
            if( dgap_code == "Interpolative" ) return URLLISTFORMAT_INTERPOLATIVE;
            if( dgap_code == "Golomb" )        return URLLISTFORMAT_GOLOMB;
            if( dgap_code == "Elias" )         return URLLISTFORMAT_ELIAS;
            if( dgap_code == "Delta" )         return URLLISTFORMAT_DELTA;
            if( dgap_code == "boost::uncompressed" ) return URLLISTFORMAT_BOOST_UNCOMPRESSED;
            if( dgap_code == "Gamma" )         return URLLISTFORMAT_GAMMA;
            return URLLISTFORMAT_DEBUG_URLID;
        }

        void
        URLList::moveRevokedToMRevokedHits()
        {
            fh_docmap dm = getIndex()->getDocumentMap();
            
            for( m_hits_t::iterator mi = m_hits.begin(); mi!=m_hits.end(); )
            {
                docid_t docid = mi->first;

                if( getIndex()->isRevokedDocumentID( docid ) )
                {
                    m_revoked_hits.insert( make_pair( docid, mi->second ) );
                    m_hits_t::iterator tmp = mi;
                    ++mi;
                    m_hits.erase( tmp );
                    continue;
                }
                ++mi;
            }
        }
        
        void
        URLList::attemptToPurgeRevoked()
        {
            fh_docmap dm = getIndex()->getDocumentMap();

            if( !m_revoked_hits.empty() )
            {
                LG_IDX_D << "EAIndex attemptToPurgeRevoked() aid:" << m_aid
                         << " value:" << m_value
                         << " revoked.size:" << m_revoked_hits.size()
                         << endl;

                for( m_hits_t::iterator mi = m_revoked_hits.begin(); mi!=m_revoked_hits.end(); ++mi )
                {
                    docid_t docid = mi->first;
                    if( dm->isRevoked( docid ) )
                    {
                        fh_doc d = dm->lookup( docid );
                        if( d->decrInvertedReferenceCount() )
                        {
                            getIndex()->removeRevokedDocumentIDCache( docid );
                        }
                    }
                }
            }
        }
        
        
        void
        URLList::save()
        {
//            cerr << "URLList::save() key:" << makeKey() << " m_hits.size():" << m_hits.size() << endl;
//             BackTrace();
            
            
            attemptToPurgeRevoked();
            
            skippedchunksize_t numberOfDocumentsInList = m_hits.size();
            skippedchunksize_t lengthOfChunkPayload    = 0;
            
            docid_t firstDocID = 0;
            docid_t  lastDocID = 0;
            if( numberOfDocumentsInList )
            {
                m_hits_t::iterator firstiter = m_hits.begin();
                m_hits_t::iterator lastiter  = m_hits.end();
                --lastiter;
                
                firstDocID = firstiter->first;
                lastDocID  = lastiter->first;
            }

            fh_stringstream compressedss;

            // docnum for 2nd to 2nd last documents 
            if( m_hits.size() > 2 )
            {
                m_hits_t::iterator iter     = m_hits.begin();
                m_hits_t::iterator lastiter = m_hits.end();
                ++iter;
                --lastiter;
            
                string dgap_code = getIndex()->getDocumentNumberGapCode();
                URLListFormat_t format = GapCodeToIOFormat( dgap_code );
                int bw = 0;
                list< docid_t > tmp;

//                cerr << "URLList::save() format:" << format << " m_hits.size():" << m_hits.size() << endl;
                
                switch( format )
                {
                case URLLISTFORMAT_INTERPOLATIVE:
                    bw = encode_interpolative<>( map_domain_iterator(iter),
                                                 map_domain_iterator(lastiter),
                                                 firstDocID,
                                                 lastDocID,
                                                 compressedss );
                    break;

                case URLLISTFORMAT_GOLOMB:
                case URLLISTFORMAT_ELIAS:
                case URLLISTFORMAT_DELTA:
                case URLLISTFORMAT_GAMMA:
                    convertToDGaps( map_domain_iterator(iter),
                                    map_domain_iterator(lastiter),
                                    tmp );
                    bw = encodeDocumentNumbers<>( compressedss, dgap_code, tmp.begin(), tmp.end(),
                                                  lastDocID, tmp.size() );
                    break;

//                 case URLLISTFORMAT_DEBUG_URLID:
//                     for( ; iter != lastiter; ++iter )
//                     {
//                         writenum( compressedss, iter->first );
//                     }
//                     break;
                case URLLISTFORMAT_DEBUG_URLID:
                    for( ; iter != lastiter; ++iter )
                    {
                        compressedss << iter->first << ",";
                    }
                    break;
                case URLLISTFORMAT_DEBUG_URLID_SCID:
                    compressedss << m_hits.size() << ":";
                    for( ; iter != lastiter; ++iter )
                    {
                        compressedss << "(" << iter->first << "," << iter->second << "),";
                    }
                    break;
                case URLLISTFORMAT_BOOST_UNCOMPRESSED: {

//                     cerr << "URLList::save() for URLLISTFORMAT_BOOST_UNCOMPRESSED" << endl;
//                     BackTrace();
                    
                    boost::archive::binary_oarchive archive( compressedss );
                    const m_hits_t& h = m_hits;
                    archive << h;
                    
                } break;
                    
                }
            }
            
            fh_database db = getDB();
            fh_stringstream vss;

            //
            // write the header and payload
            //
            compressedss << flush;
            string compressedData = tostr( compressedss );
            lengthOfChunkPayload  = compressedData.length();

            writenum( vss, firstDocID );
            writenum( vss, lastDocID  );
            writenum( vss, numberOfDocumentsInList );
            writenum( vss, lengthOfChunkPayload );
            writestring( vss, compressedData );

            LG_EAIDX_D << "URLList::save() first:" << firstDocID
                       << " last:" << lastDocID
                       << " count:" << numberOfDocumentsInList
                       << " length-of-payload:" << lengthOfChunkPayload
                       << " payload:" << compressedData
                       << endl;

            db->set( makeKey(), tostr(vss) );
            setDirty( false );
        }

        void
        URLList::load_private( fh_stringstream& ss )
        {
            skippedchunksize_t numberOfDocumentsInList = 0;
            skippedchunksize_t lengthOfChunkPayload    = 0;
            docid_t firstDocID = 0;
            docid_t  lastDocID = 0;

            streamsize startOfBlockTellg = 0;

//            cerr << "URLList::load_private(1)" << endl;
            
            startOfBlockTellg += readnum( ss, firstDocID );
            startOfBlockTellg += readnum( ss, lastDocID  );
            startOfBlockTellg += readnum( ss, numberOfDocumentsInList );
            startOfBlockTellg += readnum( ss, lengthOfChunkPayload );

            if( numberOfDocumentsInList )
            {
                m_hits[ firstDocID ] = XSD_UNKNOWN;
                m_hits[ lastDocID  ] = XSD_UNKNOWN;
            }

//            streamsize startOfBlockTellg = ss.tellg();
            // ss is tellg()==16 bytes at this point.
            fh_istream compressedss = ss;
            streamsize processedPayloadSize = 0;

            string dgap_code = getIndex()->getDocumentNumberGapCode();
            URLListFormat_t format = GapCodeToIOFormat( dgap_code );
            int bread = 0;
            vector< docid_t > tmp;

            //
            // docnum for 2nd to 2nd last documents
            //
            for( int i=0; i < (numberOfDocumentsInList-2); ++i )
            {
                if( format == URLLISTFORMAT_INTERPOLATIVE )
                {
                    bread = decode_interpolative<>( istreambuf_iterator<char>(compressedss),
                                                    lengthOfChunkPayload,
                                                    numberOfDocumentsInList-2,
                                                    firstDocID,
                                                    lastDocID,
                                                    tmp );
                    processedPayloadSize += bread;
                    compressedss.seekg( (-1 * lengthOfChunkPayload) + bread, ios::cur );
                    for( vector< docid_t >::const_iterator ti = tmp.begin(); ti != tmp.end(); ++ti )
                    {
                        m_hits[ *ti ] = XSD_UNKNOWN;
                    }
                    break;
                }
                else if( format == URLLISTFORMAT_GOLOMB
                         || format == URLLISTFORMAT_ELIAS
                         || format == URLLISTFORMAT_DELTA
                         || format == URLLISTFORMAT_GAMMA )
                {
                    processedPayloadSize +=
                        decodeDocumentNumbers( compressedss,
                                               dgap_code,
                                               numberOfDocumentsInList-2,
                                               lengthOfChunkPayload,
                                               tmp,
                                               lastDocID,
                                               numberOfDocumentsInList-2 );
                    convertFromDGaps( tmp.begin(), tmp.end(), m_hits );
                    break;
                }
//                 else if( format == URLLISTFORMAT_DEBUG_URLID )
//                 {
//                     docid_t d = 0;
//                     processedPayloadSize += readnum( compressedss, d );
//                     m_hits[ d ] = XSD_UNKNOWN;
//                 }
                else if( format == URLLISTFORMAT_DEBUG_URLID )
                {
                    string s;
                    while( getline( compressedss, s, ',' ))
                    {
                        docid_t d = toType<docid_t>( s );
                        processedPayloadSize += s.length() + 1;
                        m_hits[ d ] = XSD_UNKNOWN;
                    }
                }
                else if( format == URLLISTFORMAT_BOOST_UNCOMPRESSED )
                {
                    boost::archive::binary_iarchive archive( compressedss );
                    archive >> m_hits;
                }
                else
                {
                    docid_t d = 0;
                    readnum( compressedss, d );
                    processedPayloadSize += sizeof(d);
                    m_hits[ d ] = XSD_UNKNOWN;
                }
//                 case URLLISTFORMAT_DEBUG_URLID_SCID:
//                 {
//                     char ch;
//                     int sz = 0;
                    
//                     vss >> sz;
//                     vss >> ch;

//                     while( getline( vss, tmp, ')' ))
//                     {
//                         if( !tmp.empty() )
//                         {
//                             docid_t docid   = 0;
//                             XSDBasic_t scid = XSD_UNKNOWN;
//                             fh_stringstream twoints;
//                             twoints << tmp.substr( 1 );
//                             vss >> docid;
//                             vss >> ch;
//                             vss >> scid;
//                             m_hits.insert( make_pair( docid, scid ) );
//                         }
//                         vss >> ch;
//                     }
//                 }
            }

//            cerr << "URLList::load_private(4) hits.sz:" << m_hits.size() << " numdocs:" << numberOfDocumentsInList << endl;
            if( m_hits.size() != numberOfDocumentsInList )
            {
                // oh no!
                fh_stringstream ss;
                ss << "URLList::load() "
                   << " Failed to load all coded document numbers." << endl
                   << " dgap_code:" << dgap_code
                   << " processedPayloadSize:" << processedPayloadSize
                   << " could load:" << tmp.size()+2
                   << " should have loaded:" << numberOfDocumentsInList
                   << endl;
                Throw_FullTextIndexException( tostr(ss), 0 );
            }

            streamsize endOfBlockTellg = ss.tellg();
            if( endOfBlockTellg - startOfBlockTellg > lengthOfChunkPayload )
            {
                LG_EAIDX_ER << "SkippedListChunk::load( ERROR! ) "
                            << " read too many bytes! "
                            << " endOfBlockTellg:" << endOfBlockTellg
                            << " startOfBlockTellg:" << startOfBlockTellg
                            << " lengthOfChunkPayload:" << lengthOfChunkPayload
                            << endl;
            }
            else
                LG_EAIDX_D << "SkippedListChunk::load( OK-Done ) "
                           << " endOfBlockTellg:" << endOfBlockTellg
                           << " startOfBlockTellg:" << startOfBlockTellg
                           << " lengthOfChunkPayload:" << lengthOfChunkPayload
                           << endl;

            
            moveRevokedToMRevokedHits();
        }

        void
        URLList::load( bool createIfNotThere )
        {
            fh_stringstream ss;

            try
            {
//                cerr << "URLList::load() makeKey():" << makeKey() << endl;
                string data;
                fh_database db = getDB();
                db->get( makeKey(), data );
                ss << data;
                load_private( ss );
            }
            catch( dbException& e )
            {
                if( createIfNotThere )
                {
                    LG_EAIDX_D << "Warning, overwrite for aid:" << m_aid << " value:" << m_value << endl;
                    save();
                }
            }
        }
        
        void
        URLList::load( Database::iterator dbi, bool createIfNotThere )
        {
            fh_stringstream ss;

            try
            {
                string data;
                fh_database db = getDB();
                dbi.getValue( data );
                ss << data;
                load_private( ss );
            }
            catch( dbException& e )
            {
                if( createIfNotThere )
                {
                    LG_EAIDX_D << "Warning, overwrite for aid:" << m_aid << " value:" << m_value << endl;
                    save();
                }
            }
        }
        

        void
        URLList::dumpTo( fh_ostream oss, bool asXML, bool includeDiskSizes )
        {
            m_hits_t col = m_hits;
            for( m_hits_t::iterator iter = m_revoked_hits.begin(); iter != m_revoked_hits.end(); ++iter )
                col[ iter->first ] = iter->second;
            
            bool v=true;
            for( m_hits_t::iterator iter = col.begin(); iter != col.end(); ++iter )
            {
                if( asXML )
                {
                    oss << "  <d id=\"" << iter->first << "\" " << "/>\n";
                }
                else
                {
                    if( !v )
                        oss << ", ";
                    oss << iter->first;
                    v = false;
                }
            }
        }
        
        
        docNumSet_t&
        URLList::getDocumentNumbers( docNumSet_t& z )
        {
            z.insert( map_domain_iterator( m_hits.begin() ),
                      map_domain_iterator( m_hits.end() ));
            return z;
            
                
//             docNumSet_t::iterator hint = z.end();
//             for( m_hits_t::iterator iter = m_hits.begin(); iter != m_hits.end(); ++iter )
//             {
//                 pair<docNumSet_t::iterator, bool> rc = z.insert( iter->first );
//                 if( rc.second )
//                     hint = rc.first;
//             }
//             return z;
        }

        
        void
        URLList::setDirty( bool v )
        {
            m_dirty = v;
        }

        bool
        URLList::isDirty()
        {
            return m_dirty;
        }
        
        fh_invertedfile
        URLList::getInvertedFile()
        {
            return m_inv;
        }
        

        aid_t
        URLList::getAID()
        {
            return m_aid;
        }
        

        const std::string&
        URLList::getValue()
        {
            return m_value;
        }
        
            
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        InvertedFile::InvertedFile( fh_db4idx m_idx, fh_env dbenv,
                                    const std::string& basepath,
                                    const std::string& filename,
                                    const std::string& compareFunctionName )
            :
            m_dbenv( dbenv ),
            m_db( 0 ),
            m_idx( m_idx ),
            m_basepath( basepath ),
            m_filename( filename )
        {
            if( m_filename.empty() )
                m_filename = "invertedfile.db";

            setValueCompare( compareFunctionName );

            m_urllist_cache.setMaxCollectableSize( 15000 );
            m_urllist_cache.setTimerInterval( 0 );
        }
        
        InvertedFile::~InvertedFile()
        {
        }

        fh_db4idx
        InvertedFile::getIndex()
        {
            return m_idx;
        }

        fh_urllist
        InvertedFile::getCachedURLList( aid_t aid,
                                        const std::string& value,
                                        bool createIfNotThere )
        {
            std::pair< aid_t, string > p = make_pair( aid, value );

            fh_urllist ul = m_urllist_cache.get( p );
            if( ul )
            {
                m_urllist_hard_cache[ p ] = ul;
                return ul;
            }

            // This is the method used in testing.
//            cerr << "getCachedURLList1. getCollectableSize:" << m_urllist_cache.getCollectableSize() << endl;
            ul = new URLList( this, aid, value, createIfNotThere );
            m_urllist_cache.putNoCollect( p, ul );
            m_urllist_hard_cache[ p ] = ul;
            return ul;
        }
        
        
        fh_urllist
        InvertedFile::getCachedURLList( Database::iterator dbi,
                                        aid_t aid,
                                        const std::string& value,
                                        bool createIfNotThere )
        {
            std::pair< aid_t, string > p = make_pair( aid, value );

            fh_urllist ul = m_urllist_cache.get( p );
            if( ul )
            {
                m_urllist_hard_cache[ p ] = ul;
                return ul;
            }

//            cerr << "getCachedURLList2. getCollectableSize:" << m_urllist_cache.getCollectableSize() << endl;
            ul = new URLList( this, dbi, aid, value, createIfNotThere );
            m_urllist_cache.putNoCollect( p, ul );
            m_urllist_hard_cache[ p ] = ul;
            return ul;
            
            
//             m_urllist_cache_t::iterator ci = m_urllist_cache.find( p );
//             if( ci != m_urllist_cache.end() )
//             {
// //                cerr << "Got cached urllist for aid:" << aid << " vid:" << vid << endl;
//                 return ci->second;
//             }
            
//             fh_urllist ul = new URLList( this, aid, vid, createIfNotThere );
//             m_urllist_cache.insert( make_pair( p, ul ) );
//             return ul;
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
        InvertedFile::setValueCompare( const std::string& compareFunctionName )
        {
            m_shouldTrimValueCompareString = false;

            
            if( compareFunctionName == "int" || compareFunctionName =="long" )
            {
                m_valueCompare = ValueCompare( ValueCompare_numeric<long> );
            }
            else if( compareFunctionName == "float" || compareFunctionName == "double" )
            {
                m_valueCompare = ValueCompare( ValueCompare_numeric<double> );
            }
            else if( compareFunctionName == "cis" )
            {
                m_shouldTrimValueCompareString = true;
                m_valueCompare = ValueCompare( ValueCompare_cis );
            }
            else
            {
                m_shouldTrimValueCompareString = true;
                m_valueCompare = ValueCompare( ValueCompare_string );
            }
        }
        
        
        
        fh_urllist
        InvertedFile::ensureURLInMapping( aid_t aid,
                                          const std::string& value,
                                          urlid_t urlid,
                                          XSDBasic_t scid )
        {
            LG_EAIDX_D << "InvertedFile::ensureURLInMapping() aid:" << aid << " v:" << value
                       << " urlid:" << urlid << " scid:" << scid << endl;

            fh_urllist ul = getCachedURLList( aid, value );
            ul->insert( urlid, scid );
            return ul;
        }

        fh_urllist
        InvertedFile::loadOrAdd( aid_t aid, const std::string& value )
        {
            fh_urllist ul = getCachedURLList( aid, value );
            return ul;
        }
        
        fh_urllist
        InvertedFile::find( aid_t aid, const std::string& value )
        {
            fh_urllist ul = getCachedURLList( aid, value, false );
            return ul;
        }

        urllists_t
        InvertedFile::find_partial( aid_t aid )
        {
            urllists_t ret;
            fh_database db = getDB();

//             Database::iterator lb = db->lower_bound_partial( tostr(aid) );
//             LG_EAIDX_D << "InvertedFile::find_partial() aid:" << aid
//                        << " lb==end:" << (lb == db->end())
//                        << endl;
//             if( lb != db->end() )
//             {
//                 cerr << "lb... k:" << lb->first << " v:" << lb->second << endl;
//             }
            
            
            pair<Database::iterator, Database::iterator> p = db->equal_range_partial( makePartialKey(aid) );
            LG_EAIDX_D << "InvertedFile::find_partial() aid:" << aid
                       << " partial key:" << makePartialKey(aid)
                       << " db:" << CleanupURL( m_basepath + "/" + m_filename )
                       << " range.begin==range.end:" << (p.first == p.second)
                       << " range size:" << distance( p.first, p.second )
                       << endl;
            
            for( Database::iterator iter = p.first; iter != p.second; ++iter )
            {
                string k;
                iter.getKey( k );
                const string value = URLList::readKey( k ).second;

                LG_EAIDX_D << "InvertedFile::find_partial() aid:" << aid
                           << " k:" << k << " value:" << value << endl;
                
                fh_urllist ul = getCachedURLList( aid, value, false );
                ret.push_back( ul );
            }
            
            return ret;
        }

        urllists_t
        InvertedFile::find_partial( aid_t aid, keyPredicate_t f )
        {
            urllists_t ret;
            fh_database db = getDB();

//             Database::iterator lb = db->lower_bound_partial( tostr(aid) );
//             LG_EAIDX_D << "InvertedFile::find_partial() aid:" << aid
//                        << " lb==end:" << (lb == db->end())
//                        << endl;
//             if( lb != db->end() )
//             {
//                 cerr << "lb... k:" << lb->first << " v:" << lb->second << endl;
//             }
            
            
            pair<Database::iterator, Database::iterator> p = db->equal_range_partial( makePartialKey(aid) );
            LG_EAIDX_D << "InvertedFile::find_partial() aid:" << aid
                       << " partial key:" << makePartialKey(aid)
                       << " db:" << CleanupURL( m_basepath + "/" + m_filename )
                       << " range.begin==range.end:" << (p.first == p.second)
                       << " range size:" << distance( p.first, p.second )
                       << endl;
            
            for( Database::iterator iter = p.first; iter != p.second; ++iter )
            {
                string k;
                iter.getKey( k );
                const string value = URLList::readKey( k ).second;
//                cerr << "find_partial(pred) k:" << value << endl;
                if( f( value ) )
                {
                    LG_EAIDX_D << "InvertedFile::find_partial() aid:" << aid
                               << " k:" << k << " value:" << value << endl;
                
                    fh_urllist ul = getCachedURLList( aid, value, false );
                    ret.push_back( ul );
                }
            }
            
            return ret;
        }
        
        
        struct FERRISEXP_DLLLOCAL invertedfile_dbiter_compare
        {
            fh_invertedfile  m_inv;
            
            invertedfile_dbiter_compare( fh_invertedfile m_inv )
                :
                m_inv( m_inv )
                {
                }
            
//             inline bool operator()( Database::iterator s1, Database::iterator s2 ) const
//                 {
//                     return less< string >()( s1->first, s2->first );
//                 }
        
//             inline bool operator()( Database::iterator s1, const std::string s2 ) const
//                 {
//                     return less< string >()( s1->first, s2 );
//                 }
//             inline bool operator()( const std::string s1,  Database::iterator s2 ) const
//                 {
//                     return less< string >()( s1, s2->first );
//                 }

//             inline bool comp( const std::string& s1, const std::string& s2 ) const
//                 {
//                     return less< int >()( toint(s1), toint(s2) );
//                 }

            bool comp( const std::string& s1, const std::string& s2 ) const
                {
                    const int minlen = min( s1.length(), s2.length() );
                    LG_EAIDX_D << "invertedfile_dbiter_compare::comp()"
                               << " minlen:" << minlen
                               << " s1full:" << s1
                               << " s2full:" << s2
//                                << " s1:" << s1.substr( 0, minlen )
//                                << " s2:" << s2.substr( 0, minlen )
//                                << " fn:" << m_inv->m_filename
//                                << " res:"
//                                << ( m_inv->m_valueCompare( s1.substr( 0, minlen ), s2.substr( 0, minlen ) ) < 0 )
                               << endl;

//                     return less< string >()( s1.substr( 0, minlen ),
//                                              s2.substr( 0, minlen ) );

                    if( m_inv->m_shouldTrimValueCompareString )
                    {
                        const std::string trim1 = s1.substr( 0, minlen );
                        const std::string trim2 = s2.substr( 0, minlen );

                        LG_EAIDX_D << "invertedfile_dbiter_compare::comp(tr)"
                                   << " ret:" << m_inv->m_valueCompare( trim1, trim2 )
                                   << endl;
                        
                        return m_inv->m_valueCompare( trim1, trim2 ) < 0;
                    }

                    LG_EAIDX_D << "invertedfile_dbiter_compare::comp(e)"
                               << " ret:" << ( m_inv->m_valueCompare( s1, s2 ) < 0 )
                               << endl;
                    return m_inv->m_valueCompare( s1, s2 ) < 0;
                }

            bool operator()( const pair< string, string >& s1,  const std::string& s2 ) const
                {
                    string dbv = URLList::readKey( s1.first ).second;
                    return comp( dbv, s2 );
                }
            bool operator()( const std::string& s1,  const pair< string, string >& s2 ) const
                {
                    string dbv  = URLList::readKey( s2.first ).second;
                    return comp( s1, dbv );
                }
                
        };


//         static void dump( Database::iterator iter, Database::iterator end )
//         {
//             for( ; iter != end; ++iter )
//             {
//                 std::pair< aid_t, std::string > k = readKeyFIXME( iter->first );
//                 LG_EAIDX_D << "dump() k.aid:" << k.first << " k.value:" << k.second << endl;
//             }
//         }
        
        
        InvertedFile::iterator_range
        InvertedFile::lower_bound( aid_t aid, const std::string& k )
        {
            fh_database db = getDB();

            pair<Database::iterator, Database::iterator> p = db->equal_range_partial( makePartialKey(aid) );

            LG_EAIDX_D << "InvertedFile::lower_bound() sz:" << distance( p.first, p.second )
                       << " aid:" << aid
                       << " makePartialKey:" << makePartialKey(aid)
                       << " k:" << k
                       << endl;
//            dump( p.first, p.second );
            
            Database::iterator iter = std::lower_bound(
                p.first, p.second, k,
                invertedfile_dbiter_compare( this ));

            LG_EAIDX_D << "InvertedFile::lower_bound() iter==begin:" << (iter==p.first) << endl;
            LG_EAIDX_D << "InvertedFile::lower_bound() iter==end:"   << (iter==p.second) << endl;
            LG_EAIDX_D << "InvertedFile::lower_bound() size:"   << distance( iter, p.second ) << endl;
//             cerr << "InvertedFile::lower_bound() (first,selected]:" << endl;
//             dump( p.first, iter );
            

            SelectedRange< Database::iterator > ret( p.first, p.second, iter );
            return ret;
        }

        InvertedFile::iterator_range
        InvertedFile::upper_bound( aid_t aid, const std::string& k )
        {
            fh_database db = getDB();

            pair<Database::iterator, Database::iterator> p = db->equal_range_partial( makePartialKey(aid) );

            LG_EAIDX_D << "InvertedFile::upper_bound() k:" << k << " aid:" << aid << endl;
            LG_EAIDX_D << "InvertedFile::upper_bound() first==sec:" << (p.first == p.second) << endl;
//            LG_EAIDX_D << "getReverseValueMap:" << toVoid( GetImpl( m_idx->getReverseValueMap() )) << endl;
//             for( Database::iterator di = p.first; di != p.second; ++di )
//             {
//                 LG_EAIDX_D << "InvertedFile::upper_bound() k:" << di->first << " v:" << di->second << endl;
//             }
            
            Database::iterator iter = std::upper_bound(
                p.first, p.second, k,
                invertedfile_dbiter_compare( this ));

            SelectedRange< Database::iterator > ret( p.first, p.second, iter );
            return ret;
        }
        
        urllists_t
        InvertedFile::getURLLists( Database::iterator begin, Database::iterator end )
        {
            urllists_t ret;
            fh_database db = getDB();

//            cerr << "InvertedFile::getURLLists(s)" << endl;

            for( Database::iterator iter = begin; iter != end; ++iter )
            {
                string k;
                iter.getKey( k );
                pair< aid_t, string > p = URLList::readKey( k );

                LG_EAIDX_D << "InvertedFile::getURLLists() k:" << k << endl;

                fh_urllist ul = getCachedURLList( iter, p.first, p.second, false );
                ret.push_back( ul );
            }
            LG_EAIDX_D << "InvertedFile::getURLLists(e)" << endl;
            LG_EAIDX_D << "InvertedFile::getURLLists(e) ret.size:" << ret.size() << endl;

            return ret;
        }
        

//         urllists_t
//         InvertedFile::upper_bound( aid_t aid, const std::string& k )
//         {
//             urllists_t ret;
//             fh_database db = getDB();

//             pair<Database::iterator, Database::iterator> p = db->equal_range_partial( makePartialKey(aid) );
//             for( Database::iterator iter = p.first; iter != p.second; ++iter )
//             {
//                 string dbk;
//                 iter.getKey( dbk );

//                 if( !less_equal< string >( dbk, k ) )
//                     break;

//                 fh_urllist ul = getCachedURLList( aid, vid, false );
//                 ret.push_back( ul );
//             }
//             return ret;
//         }
        
        
        
        void
        InvertedFile::sync()
        {
            for( m_urllist_cache_t::iterator ci = m_urllist_cache.begin();
                 ci != m_urllist_cache.end(); ++ci )
            {
                ci->second->sync();
            }
            m_urllist_hard_cache.clear();
            getDB()->sync();
        }

        void
        InvertedFile::compact( fh_ostream oss, bool verbose )
        {
            try
            {
                fh_database db = getDB();
                guint32 nkeys = getNumberOfItems();
                if( verbose ) oss << "InvertedFile::compact() nkeys:" << nkeys << endl;

                for( Database::iterator iter = db->begin(); iter != db->end(); ++iter )
                {
                    string k;
                    iter.getKey( k );
                    pair< aid_t, string > p = URLList::readKey( k );

                    fh_urllist ul = getCachedURLList( p.first, p.second, false );
                    ul->setDirty( true );
                    ul->sync();
                }
            }
            catch( dbException& e )
            {
                oss << "<error> Error traversing data:" << e.what() << " </error>" << endl;
            }
            sync();
        }
        
        
        typedef map<DB*, fh_invertedfile > bt_compare_lookup_t;
        bt_compare_lookup_t& get_bt_compare_lookup()
        {
            static bt_compare_lookup_t ret;
            return ret;
        }

        static string dbt_tostr( const DBT* a )
        {
            string ret;
            ret.resize( a->size );
            memcpy( (char*)ret.data(), a->data, a->size );


//             string ret;
//             ret.resize( a->size + 1 );
//             memcpy( (char*)ret.data(), a->data, a->size );
//             char* p = (char*)ret.data() + a->size;
//             *p = '\0';
            return ret;
        }

        
        FERRISEXP_DLLLOCAL int bt_compare_aidvid_cs(DB* rawdb, const DBT* a, const DBT* b )
        {
            int ret = 0;

            std::pair< aid_t, string > ap = URLList::readKey( dbt_tostr( a ) );
            std::pair< aid_t, string > bp = URLList::readKey( dbt_tostr( b ) );

            if( ap.first < bp.first )
            {
                ret = -1;
            }
            else if( ap.first > bp.first )
            {
                ret = 1;
            }
            else if( ap.second.empty() && bp.second.empty() )
                ret = 0;
            else if( ap.second.empty() )
                return -1;
            else if( bp.second.empty() )
                return 1;
            else if( ap.second == bp.second )
                return 0;
            else
            {
                fh_invertedfile inv = get_bt_compare_lookup()[ rawdb ];
                ret = inv->m_valueCompare( ap.second, bp.second );
            }

//             LG_EAIDX_D << "     a:" << dbt_tostr( a )
//                        << " b:" << dbt_tostr( b )
//                        << endl;
//             LG_EAIDX_D << "bt_compare_aidvid_cs() a.aid:" << ap.first
//                        << " b.aid:" << bp.first
//                        << " a.size:" << a->size
//                        << " b.size:" << b->size
//                        << " a.v:" << ap.second
//                        << " b.v:" << bp.second
//                        << " ret:" << ret
//                        << endl;
            return ret;
            

            
//             const char *p1, *p2;
            
// //             {
// //                 string A;
// //                 A.resize( a->size + 1 );
// //                 memcpy( (char*)A.data(), a->data, a->size );
// //                 char* p = (char*)A.data() + a->size;
// //                 *p = '\0';
// //                 LG_EAIDX_D << "bt_compare_aidvid_cs a:" << A << endl;
// //             }


//             std::pair< aid_t, vid_t > ap = URLList::readKey( dbt_tostr( a ) );
//             std::pair< aid_t, vid_t > bp = URLList::readKey( dbt_tostr( b ) );

//             LG_EAIDX_D << "bt_compare_aidvid_cs()" << endl
//                        << "   a:" << dbt_tostr(a) << " a.aid:" << ap.first << " a.vid:" << ap.second << endl
//                        << "   b:" << dbt_tostr(b) << " b.aid:" << bp.first << " b.vid:" << bp.second << endl;
            
//             if( ap.first < bp.first )
//                 return -1;
//             if( ap.first > bp.first )
//                 return 1;
//             if( !ap.second && !bp.second )
//                 return 0;
//             if( !ap.second )
//                 return -1;
//             if( !bp.second )
//                 return 1;
//             if( ap.second == bp.second )
//                 return 0;

            
//             fh_invertedfile inv = get_bt_compare_lookup()[ rawdb ];
//             fh_db4idx       idx = inv->m_idx;
//             fh_revlexicon   ran = idx->getReverseValueMap();
            
//             string av = ran->lookup( ap.second );
//             string bv = ran->lookup( bp.second );


//             return inv->m_valueCompare( av, bv );
// //            return strcmp( av.c_str(), bv.c_str() );
        }
        
        fh_database
        InvertedFile::getDB()
        {
            if( !m_db )
            {
                m_db = new Database( m_dbenv );
                get_bt_compare_lookup()[ m_db->raw() ] = this;
                m_db->set_bt_compare( bt_compare_aidvid_cs );
                m_db->create( DB_BTREE, CleanupURL( m_basepath + "/" + m_filename ) );
                m_db->setAdvanceFunctor( Database::makeAdvanceFunctorNumeric() );
            }
            
            return m_db;
        }
        
        urllist_id_t
        InvertedFile::getNumberOfItems()
        {
            return getDB()->size();
        }

        void
        InvertedFile::dumpTo( fh_ostream oss, bool asXML )
        {
            try
            {
                if( asXML ) oss << "<invertedfile>" << endl;
                fh_database db = getDB();

                for( Database::iterator iter = db->begin(); iter != db->end(); ++iter )
                {
                    string k;
                    iter.getKey( k );
                    pair< aid_t, string > p = URLList::readKey( k );

                    LG_EAIDX_D << "InvertedFile::getURLLists() k:" << k << endl;

                    fh_urllist ul = getCachedURLList( p.first, p.second, false );
                    oss << "<term aid=\"" << p.first << "\" vid=\"" << p.second << "\" >" << endl;
                    ul->dumpTo( oss, asXML );
                    oss << "</term>" << endl;
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

//         namespace
//         {
//             static const std::string MetaIndexClassName = "db4";
//             static bool reged = MetaEAIndexerInterfaceFactory::Instance().
//                 Register( MetaIndexClassName,
//                           &EAIndexManagerDB4::Create );
//             static bool regedx = appendToMetaEAIndexClassNames( MetaIndexClassName );
//         }

        EAIndexManagerDB4::EAIndexManagerDB4()
            :
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
            m_invertedfiles = new fh_invertedfile[ m_invertedfiles_sz ];
        }

        MetaEAIndexerInterface*
        EAIndexManagerDB4::Create()
        {
            return new EAIndexManagerDB4();
        }
        


//         EAIndexManagerDB4::EAIndexManagerDB4( const std::string& basepath )
//             :
//             m_basepath( basepath )
//         {
//             Shell::acquireContext( m_basepath );
//             common_construction();
//         }

        void
        EAIndexManagerDB4::CreateIndex( fh_context c, fh_context md )
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
            {
                string s = getStrSubCtx( md, "attributes-not-to-index-regex-append", "" );
                if( !s.empty() )
                {
                    LG_EAIDX_D << "append-attr-notToIndexRegex:" << s << endl;
                    cerr << "BBB append-attr-notToIndexRegex:" << s << endl;
                    replace( s.begin(), s.end(), ',','\0' );
                    stringstream ss;
                    if( !attributesNotToIndexRegex.empty() )
                        ss << "(" << attributesNotToIndexRegex << "|";
                    ss << s;
                    if( !attributesNotToIndexRegex.empty() )
                        ss << ")";
                    attributesNotToIndexRegex = ss.str();
                    cerr << "BBB append-attr-notToIndexRegex2. new attributesNotToIndexRegex:" << attributesNotToIndexRegex << endl;

                    // PURE DEBUG
                    {
                        string t = attributesNotToIndexRegex;
                        replace( t.begin(), t.end(), '\0', ',' );
                        LG_EAIDX_D << "final-appended-attributesNotToIndexRegex:" << t << endl;
                    }
                }
            }
            
        
            string dgap_code      = getStrSubCtx( md, "document-number-gap-code",
                                                  IDXMGR_EA_DGAP_CODE_DEFAULT );
            string maxValueSize   = getStrSubCtx( md, "max-value-size-to-index",
                                                  GET_EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT());
            
//             Shell::acquireContext( m_basepath );
//             ensureConfigFileCreated();

//            cerr << "attributesNotToIndex:" << attributesNotToIndex << endl;
            
            setConfig( IDXMGR_ATTRIBUTENAMEMAP_LEXICON_CLASS_K, attributeMap_class );
            setConfig( IDXMGR_EANAMES_IGNORE_K,       attributesNotToIndex );
            setConfig( IDXMGR_EANAMES_REGEX_IGNORE_K, attributesNotToIndexRegex );
            setConfig( IDXMGR_MAX_VALUE_SIZE_K,       maxValueSize );
            cerr << "BBB append-attr-notToIndexRegex3 attributesNotToIndexRegex:" << attributesNotToIndexRegex << endl;

            string attributesToIndexRegex
                = getStrSubCtx( md, "attributes-to-index-regex",
                                GET_EAINDEX_EANAMES_TO_INDEX_REGEX_DEFAULT() );
            setConfig( IDXMGR_EANAMES_TO_INDEX_REGEX_K, attributesToIndexRegex );
            
            setDocumentNumberGapCode( dgap_code );
        
//            common_construction();

            ensureAttrNameValueReverseValueMapsCreated();

            // we always have access to the URL string
            // and map queries to use that string instead of strlookup.
            //appendToEANamesIgnore("url");

        }
        
        
//         EAIndexManagerDB4::EAIndexManagerDB4( fh_context c,
//                                         const std::string& attributeMap_class,
//                                         const std::string& forwardValueMap_class,
//                                         const std::string& reverseValueMap_class,
//                                         const std::string& eanames_ignore,
//                                         const std::string& eanames_ignore_regex,
//                                         std::streamsize max_value_size )
//             :
//             m_basepath( c->getDirPath() )
//         {
//             Shell::acquireContext( m_basepath );
//             ensureConfigFileCreated();

//             setConfig( IDXMGR_ATTRIBUTENAMEMAP_LEXICON_CLASS_K, attributeMap_class );
//             setConfig( IDXMGR_SCHEMAVALUEMAP_LEXICON_CLASS_K,   forwardValueMap_class );
//             setConfig( IDXMGR_REVERSE_VALUEMAP_LEXICON_CLASS_K, reverseValueMap_class );
        
//             setConfig( IDXMGR_EANAMES_IGNORE_K,       eanames_ignore );
//             setConfig( IDXMGR_EANAMES_REGEX_IGNORE_K, eanames_ignore_regex );
//             setConfig( IDXMGR_MAX_VALUE_SIZE_K,       tostr(max_value_size) );
        
//             common_construction();

//             m_schemaValueMap->insert(  Lexicon::NULL_VALUE_TERM,       Lexicon::NULL_VALUE_TERMID );
//             m_schemaValueMap->insert(  Lexicon::UNREADABLE_VALUE_TERM, Lexicon::UNREADABLE_VALUE_TERMID );
//             m_schemaValueMap->sync();

//             m_reverseValueMap->insert( Lexicon::NULL_VALUE_TERM,       Lexicon::NULL_VALUE_TERMID );
//             m_reverseValueMap->insert( Lexicon::UNREADABLE_VALUE_TERM, Lexicon::UNREADABLE_VALUE_TERMID );
//             m_reverseValueMap->sync();
            
//         }
        
//         fh_idx createEAIndex( fh_context c,
//                               const std::string& attributeMap_class,
//                               const std::string& forwardValueMap_class,
//                               const std::string& reverseValueMap_class,
//                               const std::string& eanames_ignore,
//                               const std::string& eanames_ignore_regex,
//                               std::streamsize max_value_size )
//         {
//             return new EAIndexManagerDB4( c,
//                                        attributeMap_class,
//                                        forwardValueMap_class,
//                                        reverseValueMap_class,
//                                        eanames_ignore,
//                                        eanames_ignore_regex,
//                                        max_value_size );
//         }
        
                                       
//         void
//         EAIndexManagerDB4::ensureConfigFileCreated()
//         {
//             try
//             {
//                 fh_context cfg = Resolve( m_basepath + DB_EAINDEX );
//             }
//             catch( exception& e )
//             {
//                 fh_context c = Resolve( m_basepath );
//                 string rdn = DB_EAINDEX;
//                 PrefixTrimmer trimmer;
//                 trimmer.push_back( "/" );
//                 rdn = trimmer( rdn );
//                 fh_mdcontext md = new f_mdcontext();
//                 fh_mdcontext child = md->setChild( "db4", "" );
//                 child->setChild( "name", rdn );
//                 fh_context newc   = c->createSubContext( "", md );
//             }
//         }

        void
        EAIndexManagerDB4::ensureAttrNameValueReverseValueMapsCreated()
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
        
        
        void
        EAIndexManagerDB4::CommonConstruction()
        {
            m_basepath = getPath();
            
            ensureAttrNameValueReverseValueMapsCreated();
            
//             // turn off transaction and logging
//             STLdb4::Environment::setDefaultOpenFlags(
//                 STLdb4::Environment::get_openflag_create()
//                 | STLdb4::Environment::get_openflag_init_mpool() );
            
//            ensureConfigFileCreated();

            const u_int32_t  environment_open_flags = Environment::get_openflag_create()
//                 | Environment::get_openflag_init_txn()
//                 | Environment::get_openflag_init_lock()
//                 | Environment::get_openflag_init_log()
//                 | DB_RECOVER
                | Environment::get_openflag_init_mpool();
            fh_env dbenv = new Environment( getBasePath(), environment_open_flags );
            m_dbenv = dbenv;
            m_docmap = new DocumentMap( 0, dbenv, this, true );

//            m_inv    = new InvertedFile( this, getBasePath() );
            m_invertedfiles[ INV_INT ] = new InvertedFile( this, dbenv,
                                                           getBasePath(),
                                                           "invertedfile.int.db",
                                                           invertedSortTypeToString( INV_INT ) );
            m_invertedfiles[ INV_DOUBLE ] = new InvertedFile( this,  dbenv,
                                                              getBasePath(),
                                                              "invertedfile.double.db",
                                                              invertedSortTypeToString( INV_DOUBLE ) );
            m_invertedfiles[ INV_CIS ] = new InvertedFile( this,  dbenv,
                                                           getBasePath(),
                                                           "invertedfile.cis.db",
                                                           invertedSortTypeToString( INV_CIS ) );
            m_invertedfiles[ INV_STRING ] = new InvertedFile( this,  dbenv,
                                                              getBasePath(),
                                                              "invertedfile.string.db",
                                                              invertedSortTypeToString( INV_STRING ) );


            loadRevokedDocumentIDCache();
            
//            ensureConfigFileCreated();
        }

        

        EAIndexManagerDB4::~EAIndexManagerDB4()
        {
            sync();
            delete [] m_invertedfiles;
        }
            
        string
        EAIndexManagerDB4::invertedSortTypeToString( invertedSort_t v )
        {
            switch( v )
            {
            case INV_INT:    return "int";
            case INV_DOUBLE: return "float";
            case INV_CIS:    return "cis";
            case INV_STRING: return "string";
            }
        }

        EAIndexManagerDB4::invertedSort_t
        EAIndexManagerDB4::stringToInvertedSortType( const std::string& s )
        {
            if( s == "cis" )
                return INV_CIS;
            if( s == "int" || s == "long" )
                return INV_INT;
            if( s == "float" || s == "double" )
                return INV_DOUBLE;
            return INV_STRING;
        }
        
        std::string
        EAIndexManagerDB4::getBasePath()
        {
            return m_basepath;
        }
            
        void
        EAIndexManagerDB4::sync()
        {
            LG_EAIDX_D << "EAIndexManagerDB4::sync()" << endl;

            if( m_attributeNameMap )
                m_attributeNameMap->sync();

//             if( m_reverseValueMap )
//                 m_reverseValueMap->sync();

//             if( m_schemaValueMap )
//                 m_schemaValueMap->sync();
            
//            m_inv->sync();
            m_invertedfiles[ INV_INT    ]->sync();
            m_invertedfiles[ INV_DOUBLE ]->sync();
            m_invertedfiles[ INV_CIS    ]->sync();
            m_invertedfiles[ INV_STRING ]->sync();

            saveRevokedDocumentIDCache();
            
            m_docmap->sync();
        }
            


//         std::string
//         EAIndexManagerDB4::getConfig( const std::string& k, const std::string& def,
//                                       bool throw_for_errors )
//         {
//             return get_db4_string( getBasePath() + DB_EAINDEX, k, def, throw_for_errors );
//         }
        
//         void
//         EAIndexManagerDB4::setConfig( const std::string& k, const std::string& v )
//         {
//             set_db4_string( getBasePath() + DB_EAINDEX, k, v );
//         }

        void
        EAIndexManagerDB4::compact( fh_ostream oss, bool verbose )
        {
            for( int i=0; i <= INV_LAST; ++i )
            {
                fh_invertedfile inv = m_invertedfiles[ i ];
                if( inv )
                {
                    inv->compact( oss, verbose );
                }
            }
        }
        
        
        
        aid_t
        EAIndexManagerDB4::getNextAID()
        {
            aid_t ret = toint(getConfig( IDXMGR_NEXT_AID, "0" ));
            ++ret;
            setConfig( IDXMGR_NEXT_AID, tostr(ret) );
            return ret;
        }
        

        vid_t
        EAIndexManagerDB4::getNextVID()
        {
            vid_t ret = toint(getConfig( IDXMGR_NEXT_VID, "2" ));
            ++ret;
            setConfig( IDXMGR_NEXT_VID, tostr(ret) );
            return ret;
        }

        std::string
        EAIndexManagerDB4::getDocumentNumberGapCode()
        {
            return getConfig( IDXMGR_EA_DGAP_CODE_K, IDXMGR_EA_DGAP_CODE_DEFAULT );
        }

        void
        EAIndexManagerDB4::setDocumentNumberGapCode( const std::string& codename )
        {
            setConfig( IDXMGR_EA_DGAP_CODE_K, codename );
        }

        fh_lexicon
        EAIndexManagerDB4::getAttributeNameMap()
        {
            return m_attributeNameMap;
        }

//         fh_revlexicon
//         EAIndexManagerDB4::getReverseValueMap()
//         {
//             return m_reverseValueMap;
//         }
        
//         fh_lexicon
//         EAIndexManagerDB4::getSchemaValueMap()
//         {
//             return m_schemaValueMap;
//         }
        
        fh_docmap
        EAIndexManagerDB4::getDocumentMap()
        {
            return m_docmap;
        }

        fh_invertedfile
        EAIndexManagerDB4::getInvertedFile( invertedSort_t v )
        {
            return m_invertedfiles[ v ];
        }

        fh_invertedfile
        EAIndexManagerDB4::getInvertedFile( const std::string& sortType )
        {
            return getInvertedFile( stringToInvertedSortType( sortType ));
        }
        
        std::string
        EAIndexManagerDB4::getAttributeNameMapClassName()
        {
            return getConfig( IDXMGR_ATTRIBUTENAMEMAP_LEXICON_CLASS_K,
                              IDXMGR_ATTRIBUTENAMEMAP_LEXICON_CLASS_DEFAULT );
        }
        
//         std::string
//         EAIndexManagerDB4::getValueMapClassName()
//         {
//             return getConfig( IDXMGR_SCHEMAVALUEMAP_LEXICON_CLASS_K,
//                               IDXMGR_SCHEMAVALUEMAP_LEXICON_CLASS_DEFAULT );
//         }
        
//         std::string
//         EAIndexManagerDB4::getReverseValueMapClassName()
//         {
//             return getConfig( IDXMGR_REVERSE_VALUEMAP_LEXICON_CLASS_K,
//                               IDXMGR_REVERSE_VALUEMAP_LEXICON_CLASS_DEFAULT );
//         }

        void
        EAIndexManagerDB4::addDocs( docNumSet_t& docnums, fh_urllist ul )
        {
            ul->getDocumentNumbers( docnums );
        }

        
        docNumSet_t& EAIndexManagerDB4::ExecuteEquals( fh_context q,
                                                       docNumSet_t& docnums,
                                                       fh_invertedfile inv,
                                                       aid_t aid,
                                                       const std::string& value )
        {
//             fh_lexicon SchemaValueMap   = getSchemaValueMap();
//             vid_t vid = SchemaValueMap->lookup( value );

            fh_urllist ul = inv->find( aid, value );
            addDocs( docnums, ul );
            
//             {
//                 docNumSet_t ul_docs;
//                 ul->getDocumentNumbers( ul_docs );
                
//                 LG_EAIDX_D << "EAQuery_Heur::ExecuteEquals() value:" << value
//                            << " aid:" << aid
//                            << " found size:" << ul_docs.size()
//                            << endl;
//             }
        }
        
        docNumSet_t& EAIndexManagerDB4::ExecuteLtEq( fh_context q,
                                                     docNumSet_t& docnums,
                                                     fh_invertedfile inv,
                                                     aid_t aid,
                                                     const std::string& value )
        {
            LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(<=) top aid:" << aid
                       << " value:" << value << endl;
            InvertedFile::iterator_range range = inv->upper_bound( aid, value );
//            cerr << "EAQuery_Heur::ExecuteQuery(<=) 1" << endl;
            urllists_t col = inv->getURLLists( range.begin, range.selected );
            
            LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(<=) col.sz:" << col.size() << endl;
            for( urllists_t::iterator li = col.begin(); li != col.end(); ++li )
            {
                fh_urllist ul  = *li;
                addDocs( docnums, ul );
            }
        }
        
        docNumSet_t& EAIndexManagerDB4::ExecuteGtEq( fh_context q,
                                                     docNumSet_t& docnums,
                                                     fh_invertedfile inv,
                                                     aid_t aid,
                                                     const std::string& value )
        {
            LG_EAIDX_D << "EAQuery_Heur::ExecuteGtEq(1)" << endl;
            InvertedFile::iterator_range range = inv->lower_bound( aid, value );
            urllists_t col = inv->getURLLists( range.selected, range.end );

            LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(>=) col.sz:" << col.size() << endl;
            for( urllists_t::iterator li = col.begin(); li != col.end(); ++li )
            {
                fh_urllist ul  = *li;
                addDocs( docnums, ul );
            }
        }
        
        
        void
        EAIndexManagerDB4::addToIndex( fh_context c,
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

//             fh_trans trans = new Transaction( 0, m_dbenv );
//             setImplicitTransaction( trans );
            
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

            // wrapWithCache(
            fh_lexicon    attributeNameMap =  wrapWithCache( getAttributeNameMap() );
//            fh_lexicon    SchemaValueMap   =  wrapWithCache( getSchemaValueMap() );
//            fh_revlexicon ran              =  getReverseValueMap();

            
            
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

//                     //
//                     // Add to {value} -> vid mapping
//                     //  
//                     if( !v.empty() )
//                     {
//                         vid = SchemaValueMap->lookup( v );
//                         if( !vid )
//                         {
//                             vid = getNextVID();
//                             LG_EAIDX_D << "DocumentIndexer::addContextToIndex() adding new gets vid:" << vid
//                                        << " for value:" << v << endl;
//                             SchemaValueMap->insert( v, vid );
//                         }
//                     }
                    
//                     //
//                     // Add reverse value->vid mapping
//                     //
//                     LG_EAIDX_D << "DocumentIndexer::addContextToIndex() c:" << c->getURL() << endl
//                                << " ran.exists:" << ran->exists( vid ) << endl;
//                     if( !ran->exists( vid ) && !v.empty() )
//                     {
//                         ran->insert( v, vid );
//                     }
                

                    LG_EAIDX_D << "DocumentIndexer::addContextToIndex() urlid:" << urlid
                               << " c:" << c->getURL() << endl
                               << endl;

                    //
                    // add urlid to mapping
                    // aid, vid -> { urlid, scid }
                    //
                    typedef EAIndexManagerDB4::invertedSort_t invertedSort_t;
                    fh_context     sc    = c->getSchema( attributeName );
                    invertedSort_t st    =
                        stringToInvertedSortType( getSchemaDefaultSort( sc ) );
                    fh_invertedfile inv  = getInvertedFile( st );


                    fh_urllist ul = inv->loadOrAdd( aid, v );
                    if( !ul->contains( urlid ) )
                    {
//                         if( attributeName == "size" )
//                         {
//                             cerr << "Adding-sz attr c:" << c->getURL()
//                                  << " aid:" << aid << " v:" << v << endl;
//                         }
                        
                        inv->ensureURLInMapping( aid, v, urlid, scid );
                        document->incrInvertedReferenceCount();
                        LG_EAIDX_D << "Adding aid:" << aid
                                   << " v:" << v << " urlid:" << urlid
                                   << " scid:" << scid
                                   << endl;
                    }

                    if( m_filesIndexedSinceAnalyseCount > 500 )
                    {
                        m_filesIndexedSinceAnalyseCount = 0;
                        LG_EAIDX_I << "Syncing the db4 files" << endl;
                        sync();
                    }
                    
                    
                    
//                     typedef std::pair< aid_t, vid_t >       key_t;
//                     typedef std::map < key_t, fh_urllist >  cache_t;
//                     cache_t cache[ EAIndexManagerDB4::INV_LAST + 1 ];

//                     key_t cache_k = make_pair( aid, vid );
//                     cache_t::iterator ci = cache[ st ].find( cache_k );
//                     if( ci != cache[st].end() )
//                     {
//                         fh_urllist ul = ci->second;
//                         if( ul->contains( urlid ) )
//                             continue;
                        
//                         inv->ensureURLInMapping( aid, vid, urlid, scid );
//                         document->incrInvertedReferenceCount();
//                     }
//                     else
//                     {
//                         fh_urllist ul = inv->loadOrAdd( aid, vid );
//                         if( !ul->contains( urlid ) )
//                         {
//                             inv->ensureURLInMapping( aid, vid, urlid, scid );
//                             document->incrInvertedReferenceCount();
//                         }
//                         cache[ st ].insert( make_pair( cache_k, ul ) );
//                     }
                    

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

        struct ExecuteQuery_FindByURLRegex_Functor 
        {
            fh_regex rex;
            docNumSet_t& docnums;
            int limit;
                        
            ExecuteQuery_FindByURLRegex_Functor( fh_regex rex, docNumSet_t& docnums, int limit )
                :
                rex( rex ), docnums( docnums ), limit( limit )
                {}
            void operator()( const std::string& earl, docid_t docid )
                {
                    LG_EAIDX_D << "op() earl:" << earl << endl;

                    if( !rex->operator()( earl ) )
                        return;

                    docnums.insert( docid );
                }
        };

        struct KeyPredicateRegex
        {
            fh_regex m_rex;
            KeyPredicateRegex( fh_regex rex )
                : m_rex( rex )
                {
                }
            bool operator()( const std::string& s )
                {
                    if( !m_rex->operator()( s ) )
                        return false;
                    return true;
                }
        };
        
        docNumSet_t&
        EAIndexManagerDB4::ExecuteQuery( fh_context q, docNumSet_t& docnums, int limit )
        {
            string token   = getStrAttr( q, "token", "" );
            string tokenfc = foldcase( token );
            fh_attribute orderedtla = q->getAttribute("in-order-insert-list");
            fh_istream   orderedtls = orderedtla->getIStream();

            LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery() token:" << tokenfc << endl;
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

                for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
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
                
                    

//                 docNumSet_t ldocs;
//                 docNumSet_t rdocs;
//                 docNumSet_t tmp;
                
//                 ExecuteQuery( lc, ldocs, limit );
//                 ExecuteQuery( rc, rdocs, limit );

//                 set_intersection( ldocs.begin(), ldocs.end(),
//                                   rdocs.begin(), rdocs.end(),
//                                   inserter( tmp, tmp.begin() ) );
//                 docnums.insert( tmp.begin(), tmp.end() );
//                 return docnums;
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
                
//                 docNumSet_t ldocs;
//                 docNumSet_t rdocs;
//                 docNumSet_t tmp;

//                 ExecuteQuery( lc, ldocs, limit );
//                 ExecuteQuery( rc, rdocs, limit );

//                 set_union( ldocs.begin(), ldocs.end(),
//                            rdocs.begin(), rdocs.end(),
//                            inserter( tmp, tmp.begin() ) );
//                 docnums.insert( tmp.begin(), tmp.end() );
//                 return docnums;
            }
            
            
            

            string eaname = getStrAttr( lc, "token", "" );
//            string value  = getStrAttr( rc, "token", "" );
            IndexableValue iv = getIndexableValueFromToken( eaname, rc );
            string value = asString( iv );
            string comparisonOperator = guessComparisonOperatorFromData( value );

            LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery() eaname:" << eaname
                       << " value:" << value << endl;

            fh_lexicon attributeNameMap = getAttributeNameMap();

            aid_t aid = attributeNameMap->lookup( eaname );
            string sortType = guessComparisonOperatorFromData( value );
            LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery() aid:" << aid
                       << " sortType:" << sortType
                       << endl;
            
            fh_invertedfile inv = getInvertedFile( sortType );

            if( tokenfc == "DEBUG==" )
            {
                LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(==) detected DEBUG CODE" << endl;
                
                urllists_t    col = inv->find_partial( aid );
                for( urllists_t::iterator li = col.begin(); li != col.end(); ++li )
                {
                    fh_urllist ul  = *li;
                    addDocs( docnums, ul );
                }
            }
            if( tokenfc == "==" )
            {
                LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(==) detected" << endl;

                //
                // Handle looking these up using the docmap secondary index
                //
                if( eaname == "url" || eaname == "name" )
                {
                    string esc_value = Util::EscapeStringAsRegex(value);
                    fh_regex rex = new Regex( "^" + esc_value + "$" );

                    if( eaname == "name" )
                    {
                        stringstream ss;
                        ss << ".*/" << esc_value << "$";
                        rex = new Regex( ss.str() );
                    }
                    cerr << "Looking by URL!" << endl;
                    ExecuteQuery_FindByURLRegex_Functor f( rex, docnums, limit );
                    m_docmap->for_each( DocumentMap::ForEachDocumentFunctor_t( f ) );
                }
                else
                {
                    ExecuteEquals( q, docnums, inv, aid, value );
//                    vid_t vid = SchemaValueMap->lookup( value );
//                    fh_urllist ul = inv->find( aid, vid );
//                    addDocs( docnums, ul );
                }
            }
            else if( tokenfc == "=?=" )
            {
                ExecuteEquals( q, docnums, 
                               getInvertedFile( EAIndexManagerDB4::INV_INT ),
                               aid, tostr(convertStringToInteger( value )) );
                ExecuteEquals( q, docnums, 
                               getInvertedFile( EAIndexManagerDB4::INV_DOUBLE ),
                               aid, tostr(convertStringToInteger( value )) );
                ExecuteEquals( q, docnums, 
                               getInvertedFile( EAIndexManagerDB4::INV_CIS ),
                               aid, value );
                ExecuteEquals( q, docnums, 
                               getInvertedFile( EAIndexManagerDB4::INV_STRING ),
                               aid, value );
            }
            else if( tokenfc == "=~" )
            {
                LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(=~) detected" << endl;
                LG_EAIDX_D << "iv.isCaseSensitive:" << iv.isCaseSensitive() << endl;
                
                boost::regex::flag_type rflags = boost::regex::optimize;
                if( !iv.isCaseSensitive() )
                    rflags |= boost::regex::icase;
                
                //
                // Handle looking these up using the docmap secondary index
                //
                if( eaname == "url" || eaname == "name" )
                {
                    fh_regex rex = new Regex( value, rflags );
                    
                    if( eaname == "name" )
                    {
                        stringstream ss;
                        ss << ".*/" << value << "$";
                        rex = new Regex( ss.str() );
                    }
                    cerr << "Looking by URL!" << endl;
                    ExecuteQuery_FindByURLRegex_Functor f( rex, docnums, limit );
                    m_docmap->for_each( DocumentMap::ForEachDocumentFunctor_t( f ) );
                }
                else
                {
                    fh_regex reg = new Regex( value, rflags );
                    KeyPredicateRegex pred( reg );
                    urllists_t col = inv->find_partial( aid, InvertedFile::keyPredicate_t(pred) );
                    
                    LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(=~) lists.sz:" << col.size() << endl;
//                    cerr << "EAQuery_Heur::ExecuteQuery(=~) aid:" << aid << " lists.sz:" << col.size() << endl;

                    for( urllists_t::iterator li = col.begin(); li != col.end(); ++li )
                    {
                        fh_urllist ul  = *li;
                        const std::string& dbv = (*li)->getValue();
                        LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(=~) dbv:" << dbv << endl;

                        //
                        // check to make sure that the current value passes the regex
                        //
                        if( !reg->operator()( dbv ) )
                            continue;
                    
                        addDocs( docnums, ul );
                    }
                }
            }
            else if( tokenfc == ">=" )
            {
                LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(>=) detected" << endl;

                ExecuteGtEq( q, docnums, inv, aid, value );
                LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(>=) executed" << endl;
                
//                 InvertedFile::iterator_range range = inv->lower_bound( aid, value );
//                 urllists_t col = inv->getURLLists( range.selected, range.end );

//                 LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(>=) col.sz:" << col.size() << endl;
//                 for( urllists_t::iterator li = col.begin(); li != col.end(); ++li )
//                 {
//                     fh_urllist ul  = *li;
//                     addDocs( docnums, ul );
//                 }
            }
            else if( tokenfc == ">?=" )
            {
                LG_EAIDX_D << " For >?= opcode integer version:"
                           << tostr(convertStringToInteger( value ))
                           << endl;
                
                ExecuteGtEq( q, docnums, 
                             getInvertedFile( EAIndexManagerDB4::INV_INT ),
                             aid, tostr(convertStringToInteger( value )) );
                ExecuteGtEq( q, docnums, 
                             getInvertedFile( EAIndexManagerDB4::INV_DOUBLE ),
                             aid, tostr(convertStringToInteger( value )) );
                ExecuteGtEq( q, docnums, 
                             getInvertedFile( EAIndexManagerDB4::INV_CIS ),
                             aid, value );
                ExecuteGtEq( q, docnums, 
                             getInvertedFile( EAIndexManagerDB4::INV_STRING ),
                             aid, value );
            }
            else if( tokenfc == "<=" )
            {
                LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(<=) detected" << endl;

                ExecuteLtEq( q, docnums, inv, aid, value );
                
//                 InvertedFile::iterator_range range = inv->upper_bound( aid, value );
//                 urllists_t col = inv->getURLLists( range.begin, range.selected );

//                 LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(<=) col.sz:" << col.size() << endl;
//                 for( urllists_t::iterator li = col.begin(); li != col.end(); ++li )
//                 {
//                     fh_urllist ul  = *li;
//                     addDocs( docnums, ul );
//                 }
            }
            else if( tokenfc == "<?=" )
            {
                ExecuteLtEq( q, docnums, 
                             getInvertedFile( EAIndexManagerDB4::INV_INT ),
                             aid, tostr(convertStringToInteger( value )) );
                ExecuteLtEq( q, docnums, 
                             getInvertedFile( EAIndexManagerDB4::INV_DOUBLE ),
                             aid, tostr(convertStringToInteger( value )) );
                ExecuteLtEq( q, docnums, 
                             getInvertedFile( EAIndexManagerDB4::INV_CIS ),
                             aid, value );
                ExecuteLtEq( q, docnums, 
                             getInvertedFile( EAIndexManagerDB4::INV_STRING ),
                             aid, value );
            }
//             // OLD way of doing range stuff... works ok.
//             else if( tokenfc == "OLD <=" )
//             {
//                 LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(<=) detected" << endl;

//                 fh_revlexicon ran = getReverseValueMap();
//                 urllists_t    col = inv->find_partial( aid );
                
//                 LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(<=) lists.sz:" << col.size() << endl;
//                 for( urllists_t::iterator li = col.begin(); li != col.end(); ++li )
//                 {
//                     fh_urllist ul  = *li;
//                     vid_t      vid = ul->getVID();
//                     string     dbv = ran->lookup( vid );

//                     LG_EAIDX_D << "EAQuery_Heur::ExecuteQuery(<=) vid:" << vid
//                          << " value:" << value
//                          << " ran->exists():" << ran->exists( vid )
//                          << " dbv:" << dbv
//                          << endl;

//                     if( !ran->exists( vid ) )
//                     {
//                         cerr << "ERROR, vid:" << vid << " has no reverse value map" << endl;
//                         continue;
//                     }
                    

//                     //
//                     // check to make sure that the current value is still less
//                     // than that desired.
//                     //
//                     if( comparisonOperator == "int" )
//                         if( !less_equal<int>()( toint( dbv ), toint( value )))
//                             break;
                    
//                     addDocs( docnums, ul );
//                 }
//             }

            //
            // Filter out revoked docids.
            //
            {
                LG_EAIDX_D << "m_revokedDocumentIDCache.sz:" << m_revokedDocumentIDCache.size() << endl;
                
                docNumSet_t rawDocs = docnums;
                docnums.clear();

                docNumSet_t::iterator e = rawDocs.end();
                for( docNumSet_t::iterator iter = rawDocs.begin(); iter != e; ++iter )
                {
                    docid_t d = *iter;
                    
                    if( !isRevokedDocumentID( d ) )
                    {
                        docnums.insert( d );
                    }
                }
            }
            
            return docnums;
        }
    
        docNumSet_t&
        EAIndexManagerDB4::ExecuteQuery( fh_context q,
                                         docNumSet_t& output,
                                         fh_eaquery qobj,
                                         int limit )
        {
            return ExecuteQuery( q, output, limit );
        }
        
        std::string
        EAIndexManagerDB4::resolveDocumentID( docid_t docid )
        {
            fh_doc d = getDocumentMap()->lookup( docid );
            return d->getURL();
        }
        

        bool
        EAIndexManagerDB4::isRevokedDocumentID( docid_t d )
        {
            return m_revokedDocumentIDCache.count( d );
        }
        
        void
        EAIndexManagerDB4::removeRevokedDocumentIDCache( docid_t d )
        {
            m_revokedDocumentIDCache.erase( d );
        }
        
        void
        EAIndexManagerDB4::ensureRevokedDocumentIDCache( docid_t d )
        {
            m_revokedDocumentIDCache.insert( d );
        }

        std::string
        EAIndexManagerDB4::getRevokedDocumentIDCacheFileName()
        {
            string ret = getBasePath() + "/" + "revoked-docid-set-cache";
            return ret;
        }
        
        
        void
        EAIndexManagerDB4::loadRevokedDocumentIDCache()
        {
            fh_ifstream iss( getRevokedDocumentIDCacheFileName() );
            string s = StreamToString( iss );
            Util::parseSeperatedList( s,
                                      m_revokedDocumentIDCache,
                                      inserter( m_revokedDocumentIDCache,
                                                m_revokedDocumentIDCache.begin() ) );
        }
        
        void
        EAIndexManagerDB4::saveRevokedDocumentIDCache()
        {
            fh_ofstream oss( getRevokedDocumentIDCacheFileName(),
                             std::ios_base::out | std::ios_base::trunc );
            oss << Util::createSeperatedList( m_revokedDocumentIDCache.begin(),
                                              m_revokedDocumentIDCache.end() );
        }
        
        
        bool
        EAIndexManagerDB4::getIndexMethodSupportsIsFileNewerThanIndexedVersion()
        {
            return true;
        }
        
        bool
        EAIndexManagerDB4::isFileNewerThanIndexedVersion( const fh_context& c )
        {
            bool ret = true;

            time_t tt = Time::getTime();
            time_t ct = getTimeAttr( c, "ferris-should-reindex-if-newer", 0 );
            if( !ct )
                return ret;

            string earl = c->getURL();

            LG_EAIDX_D << "isFileNewerThanIndexedVersion() earl:" << earl << endl;
            LG_EAIDX_D << "isFileNewerThanIndexedVersion() ct:" << ct << endl;
            
            if( fh_doc document = m_docmap->lookup( c ) )
            {
                time_t dt = document->getMTime();
                LG_EAIDX_D << "isFileNewerThanIndexedVersion(test) dt:" << dt << endl;

                LG_EAIDX_D << "isFileNewerThanIndexedVersion() ct:" << Time::toTimeString( ct ) << endl;
                LG_EAIDX_D << "isFileNewerThanIndexedVersion() dt:" << Time::toTimeString( dt ) << endl;
                
                if( ct < dt )
                {
                    LG_EAIDX_D << "isFileNewerThanIndexedVersion(DO NOT INDEX) document->getMTime():"
                               << document->getMTime() << endl;
                    return false;
                }
            }
            
            return ret;
        }
        
        void
        EAIndexManagerDB4::removeDocumentsMatchingRegexFromIndex( const std::string& earl, time_t mustBeOlderThan )
        {
            cerr << "EAIndexManagerDB4::removeDocumentsMatchingRegexFromIndex() earl:" << earl << endl;
            
            fh_docmap  docmap   = getDocumentMap();

            if( fh_doc document = docmap->lookupByURL( earl ) )
            {
                cerr << "EAIndexManagerDB4::removeDocumentsMatchingRegexFromIndex() earl:" << earl
                     << " has docid:" << document->getID() 
                     << endl;
                ensureRevokedDocumentIDCache( document->getID() );
                document->revokeDocument();
            }
        }
        
        
        stringset_t&
        EAIndexManagerDB4::getValuesForAttribute( stringset_t& ret, const std::string& eaname, AttrType_t att )
        {
            fh_database inv = m_invertedfiles[ INV_STRING ]->getDB();
            if( att == ATTRTYPEID_INT )
                inv = m_invertedfiles[ INV_INT ]->getDB();
            if( att == ATTRTYPEID_DBL )
                inv = m_invertedfiles[ INV_DOUBLE ]->getDB();
            if( att == ATTRTYPEID_CIS )
                inv = m_invertedfiles[ INV_CIS ]->getDB();

            std::pair<Database::iterator, Database::iterator> iterpair;
            iterpair = inv->equal_range_partial( eaname + "," );
            Database::iterator iter = iterpair.first;
            Database::iterator e = iterpair.second;
        
            for( ; iter != e ; ++iter )
            {
                std::string s = iter->first;
                s = s.substr( s.find(",")+1);
                s = s.substr( s.find(",")+1);
                s = tolowerstr()( s );
                ret.insert( s );
            }
            
        }
        
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        class FERRISEXP_API EAIndexManagerDB4Creator
        {
        public:
            static Ferris::EAIndex::MetaEAIndexerInterface* Create()
                {
                    return new Ferris::EAIndex::EAIndexManagerDB4();
                }
        };
        
    };
};




extern "C"
{
    FERRISEXP_API Ferris::EAIndex::MetaEAIndexerInterface* Create()
    {
        return Ferris::EAIndex::EAIndexManagerDB4Creator::Create();
    }
};
