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

    $Id: libftxcustomferris.cpp,v 1.8 2010/09/24 21:31:31 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "FullTextIndexer.hh"
#include "FullTextIndexer_private.hh"
#include "Indexing/IndexPrivate.hh"
#include <Configuration_private.hh>
#include "Trimming.hh"
#include "Iterator.hh"
#include "Ferris/FullTextQuery.hh"

#include <mg/stem.h>
#include <mg/bitio_m.h>
#include <mg/bitio_m_mem.h>
#include <mg/bitio_mem.h>
#include <mg/bitio_gen.h>

#include <errno.h>

#include <iostream>
#include <iterator>
#include <algorithm>
#include <numeric>

#include "libftxcustomferris.hh"

// #undef  LG_IDX_D
// #define LG_IDX_D cerr

using namespace std;
using namespace STLdb4;

namespace Ferris
{
    namespace FullTextIndex 
    {
        const int MAX_TERM_HIT_COUNT = 65000;

        const db_recno_t DOCUMENTMAP_DISKFILE_VERSION_RECNO   = 1;
        const string     DOCUMENTMAP_DISKFILE_VERSION_CURRENT = "3";

//         namespace
//         {
//             static void junk()
//             {
//                 BIO_Mem_Encode_Start( 0, 0, 0 );
//             }
//         };
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        

        
        fh_database
        Document::getDB()
        {
            return m_docmap->getDB();
        }
        

        Document::Document( fh_docmap docmap, fh_context c )
            :
            m_deleted( false ),
            m_dirty( false ),
            m_docmap( docmap ),
            m_url( c->getURL() ),
            m_id( 0 ),
            m_invertedReferenceCount( 0 ),
            m_isRevokedDocument( false ),
            m_weight( 0 ),
            m_documentIndexedWithMTime( 0 )
        {
            freshenMTime();
            save();
        }
        

        Document::Document( fh_docmap docmap, docid_t n )
            :
            m_deleted( false ),
            m_dirty( false ),
            m_docmap( docmap ),
            m_url( "" ),
            m_id( n ),
            m_invertedReferenceCount( 0 ),
            m_isRevokedDocument( false ),
            m_weight( 0 )
        {
            fh_database db = getDB();

            string data = db->get( m_id );
            stringstream ss;
            ss << data;
            if( m_docmap->m_idx && m_docmap->m_idx->supportsRankedQuery() )
            {
                readnum( ss, m_weight );
            }
            readnum( ss, m_documentIndexedWithMTime );
            readnum( ss, m_invertedReferenceCount );
            m_savedBitfieldSize_t bf = 0;
            readnum( ss, bf );
            m_isRevokedDocument = bf & ( 1 << E_ISREVOKED_SHIFT );
            getline( ss, m_url );
            
            
//             db_recno_t recno = m_id;

//             Dbt key ( (void*)&recno, sizeof(recno) );
//             Dbt data( 0, 0 );
//             db->get( 0, &key, &data, 0 );

//             if( data.get_size() )
//             {
//                 stringstream ss;
//                 ss.write( (const char*)data.get_data(), data.get_size() );
//                 if( m_docmap->m_idx && m_docmap->m_idx->supportsRankedQuery() )
//                 {
//                     readnum( ss, m_weight );
//                 }
//                 getline( ss, m_url );
//             }
        }

        void
        Document::save()
        {
            if( m_deleted )
                return;
            
            fh_database db = getDB();

            fh_stringstream ss;
            if( m_docmap->m_idx && m_docmap->m_idx->supportsRankedQuery() )
            {
                writenum( ss, m_weight );
            }
            writenum( ss, m_documentIndexedWithMTime );
            writenum( ss, m_invertedReferenceCount );
            m_savedBitfieldSize_t bf = ( m_isRevokedDocument << E_ISREVOKED_SHIFT ) | 0;
            writenum( ss, bf );
            ss << m_url << flush;
            string datastr = tostr(ss);

//             if( !m_id )
//                 m_id = db->size() + 1;
//             db->set( tostr( m_id ), datastr );
            
            if( !m_id )
            {
                m_id = db->append( datastr );
            }
            else
                db->set( m_id, datastr );
        }
        
        Document::~Document()
        {
            sync();
        }
        
        void
        Document::sync()
        {
            if( m_dirty )
            {
                save();
            }
        }
        
        guint32
        Document::getID()
        {
            return m_id;
        }
        
        std::string
        Document::getURL()
        {
            return m_url;
        }
        
        fh_context
        Document::getContext()
        {
            return Resolve( getURL() );
        }

        void
        Document::setDocumentWeight( documentWeight_t w )
        {
            m_dirty = true;
            m_weight = w;
        }

        documentWeight_t
        Document::getDocumentWeight()
        {
            return m_weight;
        }
        
        void
        Document::freshenMTime()
        {
            try
            {
                m_documentIndexedWithMTime = Time::getTime();
//                 m_documentIndexedWithMTime
//                     = toType<time_t>(
//                         getStrAttr( getContext(), "mtime", "0" ) );
                m_dirty = true;
            }
            catch( exception& e )
            {}
        }

        time_t
        Document::getMTime()
        {
            return m_documentIndexedWithMTime;
        }
        
        
        void
        Document::incrInvertedReferenceCount()
        {
            m_dirty = true;
            ++m_invertedReferenceCount;
        }
        
        bool
        Document::decrInvertedReferenceCount()
        {
            m_dirty = true;
            --m_invertedReferenceCount;

            // if we have no more references then we can finally die
            if( !m_invertedReferenceCount )
            {
                sync();
                m_deleted = true;
                m_docmap->erase( this );
                return true;
            }
            return false;
        }
        
        docidInvertedRef_t
        Document::getInvertedReferenceCount()
        {
            return m_invertedReferenceCount;
        }
        
        void
        Document::revokeDocument()
        {
            LG_IDX_D << "Document::revokeDocument() REVOKING DOCUMENT ID:" << getID() << endl;
            
            m_dirty = true;
            m_isRevokedDocument = true;
            m_docmap->addToRevokedIDCache( getID() );
        }
        
        bool
        Document::isRevoked()
        {
            return m_isRevokedDocument;
        }
        
        
        /********************************************************************************/

        DocumentMap::DocumentMap( fh_nidx idx, fh_env dbenv,
                                  PathManager* path_mgr, bool useSecondaryIndex )
            :
            m_dbenv( dbenv ),
            m_idx( idx ),
            m_path_mgr( path_mgr ),
            m_db( 0 ),
            m_useSecondaryIndex( useSecondaryIndex )
        {
            if( !m_path_mgr && idx )
                m_path_mgr = GetImpl(idx);
        }
        
        DocumentMap::~DocumentMap()
        {
            if( m_db )
                m_db->sync();
            if( m_secdb )
                m_secdb->sync();

            m_db = 0;
            m_secdb = 0;
        }

        void
        DocumentMap::addToRevokedIDCache( docid_t id )
        {
            m_revoked.insert( id );
        }
        
        void
        DocumentMap::erase( fh_doc d )
        {
            docid_t id = d->getID();
            m_revoked.erase( m_revoked.find( id ));

            LG_IDX_D << "DocumentMap::erase() id:" << id << " url:" << d->getURL() << endl;
            
            Database::iterator di = m_db->find( id );
            if( di != m_db->end() )
            {
                LG_IDX_D << "DocumentMap::erase(found) id:" << id << " url:" << d->getURL() << endl;
                m_db->erase( di );
                m_documents.erase( m_documents.find( id ));
                sync();

                try
                {
                    Database::iterator di = m_db->find( id );
                    LG_IDX_D << "DocumentMap::erase() post test lookup after erase() is OK?"
                         << " di==end:" << (di == m_db->end())
                         << endl;
                }
                catch( exception& e )
                {
                    LG_IDX_W << "DocumentMap::erase() post test e:" << e.what() << endl;
                }
            }
        }


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

//         template < int offset >
//         int XoffsetSecIdxFunction( DB* db, const DBT* pkey, const DBT* pdata, DBT* skey)
//         {
//             if( !skey )
//                 return(0);
//             if( pdata->size < offset )
//                 return( DB_DONOTINDEX );
//             memset(skey, 0, sizeof(DBT));
//             skey->data = (char*)pdata->data + offset;
//             skey->size = pdata->size - offset;

//             if( skey->size &&  skey->size < 1000 )
//             {
//                 char b[1024];
//                 memset( b, 0, 1000 );
//                 strncpy( b, (char*)skey->data, skey->size );
//                 LG_IDX_D << "XoffsetSecIdxFunction() b:" << b << endl;
//             }
            
//             return (0);
//         }

//         /**
//          * Create a secondary index function that creates the secondary key from
//          * the primary data starting at an offset and containing all data from there
//          * onwards.
//          */
//         template < int offset >
//         Database::sec_idx_callback XgetOffsetSecIdx()
//         {
//             return XoffsetSecIdxFunction< offset >;
//         }

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

        int
        DocumentMap::getSecondaryDBOffset()
        {
            const int nonRankedOffset = sizeof( Document::m_documentIndexedWithMTime_t )
                + sizeof( Document::m_invertedReferenceCount_t )
                + sizeof( Document::m_savedBitfieldSize_t );

            const int rankedOffset = nonRankedOffset + sizeof( Document::m_weight_t );
            if( m_idx && m_idx->supportsRankedQuery() )
            {
                return rankedOffset;
            }
            return nonRankedOffset;
        }
        
        Database::sec_idx_callback
        DocumentMap::getSecondaryDBCallback()
        {
            const int nonRankedOffset = sizeof( Document::m_documentIndexedWithMTime_t )
                + sizeof( Document::m_invertedReferenceCount_t )
                + sizeof( Document::m_savedBitfieldSize_t );

            const int rankedOffset = nonRankedOffset + sizeof( Document::m_weight_t );
            Database::sec_idx_callback f = getOffsetSecIdx< nonRankedOffset >();

            if( m_idx && m_idx->supportsRankedQuery() )
            {
                f = getOffsetSecIdx< rankedOffset >();
            }
            return f;
        }
        
        fh_database
        DocumentMap::getDB()
        {
            if( !m_db )
            {
//                 m_db = new Database( DB_RECNO,
//                                      CleanupURL( m_path_mgr->getBasePath() + "/document-map.db" ) );

                m_db = new Database( m_dbenv );
                m_db->create( DB_RECNO,
                              CleanupURL( m_path_mgr->getBasePath() + "/document-map.db" ) );
                
                db_recno_t recno = DOCUMENTMAP_DISKFILE_VERSION_RECNO;
                try
                {
                    int diskv    = toint( m_db->get( recno ) );
                    int currentv = toint( DOCUMENTMAP_DISKFILE_VERSION_CURRENT );

//                     LG_IDX_D << "DocumentMap::getDB() diskv:" << diskv
//                          << " currentv:" << currentv
//                          << endl;
                    
                    if( diskv != currentv )
                    {
                        fh_stringstream ss;
                        ss << "Document map is in an old format. recreate index." << endl
                           << " current:" << currentv
                           << " on disk format:" << diskv << endl;
                        cerr << tostr(ss) << endl;
                        Throw_FullTextIndexException( tostr(ss), 0 );
                    }
                }
                catch( FullTextIndexException& )
                {
                    throw;
                }
                catch( exception& )
                {
                    try
                    {
//                         cerr << "DocumentMap::getDB() setting recno:" << recno
//                              << " disk version to:" << DOCUMENTMAP_DISKFILE_VERSION_CURRENT << endl;
                        
                        m_db->set( recno, DOCUMENTMAP_DISKFILE_VERSION_CURRENT );

//                         try {
//                             cerr << "read back:" << toint( m_db->get( recno ) ) << endl;
//                         }
//                         catch( ... ) {
//                         }
                    
//                         m_db->sync();
                    }
                    catch( exception& e )
                    {
                        LG_IDX_ER << "Unable to record on disk format version e:" << e.what() << endl;
                    }
                }

                if( m_useSecondaryIndex )
                {
                    Database::sec_idx_callback f = getSecondaryDBCallback();

                    LG_IDX_D << "associating (with dbenv) the secondary index. at:"
                         << CleanupURL( m_path_mgr->getBasePath() + "/document-map.sec.db" ) << endl;
                    cerr << "associating (with dbenv) the secondary index. at:"
                         << CleanupURL( m_path_mgr->getBasePath() + "/document-map.sec.db" ) << endl;
                    m_secdb = Database::makeSecondaryIndex(
                        m_dbenv,
                        0,
                        m_db,
                        f,
                        DB_BTREE,
                        CleanupURL( m_path_mgr->getBasePath() + "/document-map.sec.db" ) );

                    LG_IDX_D << "Associated secondary index at:" 
                         << CleanupURL( m_path_mgr->getBasePath() + "/document-map.sec.db" )
                         << " m_secdb:" << toVoid(m_secdb) << " m_db:" << toVoid(m_db)
                         << endl;
                }

                // load in the cache of revoked docids again
                string revokedstr = m_path_mgr->getConfig( IDXMGR_DOCMAP_REVOKED_ID_CACHE_K,
                                                           IDXMGR_DOCMAP_REVOKED_ID_CACHE_DEFAULT );
                m_revoked.clear();
                LG_IDX_D << "revokedstr:" << revokedstr << endl;
                Util::parseSeperatedList( revokedstr, m_revoked );
            }
            return m_db;
        }

        /**
         * Get the cache of which document IDs are currently revoked.
         */
        std::set< docid_t >
        DocumentMap::getRevokedDocumentIDs()
        {
            getDB();
            return m_revoked;
        }
        
        

        guint32
        DocumentMap::size()
        {
            return getDB()->size();
        }
        

        fh_doc
        DocumentMap::append( fh_context c )
        {
            LG_IDX_D << "DocumentMap::append(1) c:" << c->getURL() << endl;
            fh_doc ret = new Document( this, c );
            LG_IDX_D << "DocumentMap::append(2) c:" << c->getURL() << endl;
            m_documents.insert( make_pair( ret->getID(), ret ));

//            sync();
            return ret;
        }
        
        fh_doc
        DocumentMap::lookup( docid_t id )
        {
            m_documents_t::iterator di = m_documents.find( id );
            if( di != m_documents.end() )
                return di->second;

            // we better check to make sure that the docid has not been
            // purged from the database.
            {
                fh_database db = getDB();
                Database::iterator di = db->find( id );
                if( di == db->end() )
                    return 0;
            }
            
            fh_doc ret = new Document( this, id );
            m_documents.insert( make_pair( ret->getID(), ret ));
            return ret;
        }

        void
        DocumentMap::for_each( ForEachDocumentFunctor_t f )
        {
            cerr << "m_secdb isbound:" << isBound( m_secdb ) << endl;

            getDB();
            
            Database::iterator di = m_secdb->begin();
            Database::iterator de = m_secdb->end();

            for( ; di != de ; ++di )
            {
                string earl = "";
                string sid = "0";
                di.getPrimaryKey( sid );
                di.getKey( earl );
                docid_t id = toType<docid_t>( tostr(di.getRecNumber()) );
                f( earl, id );
            }
        }


        fh_doc
        DocumentMap::lookupByURL( const std::string& earl )
        {
            fh_database db = getDB();

            if( m_useSecondaryIndex )
            {
                LG_IDX_D << "DocumentMap::lookup() using secondary. [good]" << endl;
                
                //
                // Quick check using the secondary index
                //
                Database::iterator de = m_secdb->end();
                Database::iterator di = m_secdb->find( earl );
                for( ; di != de ; di.moveCursorNextDup() )
                {
                    string sid = "0";
                    di.getPrimaryKey( sid );
                    docid_t id = toType<docid_t>( tostr(di.getRecNumber()) );
                    LG_IDX_D << "DocumentMap::lookup() found using secondary..."
                             << " c:" << earl
                             << " id:" << id
//                          << " first:" << di->first
//                          << " second:" << di->second
                             << endl;
//                    LG_IDX_D << "id:" << id << endl;

                    if( !id || isRevoked( id ) )
                        continue;

                    LG_IDX_D << "id:" << id << endl;
                    return lookup( id );
                }
                return 0;
            }
            else
             {
                LG_IDX_D << "DocumentMap::lookup() no secondary index... using linear search on primary index." << endl;
                //
                // This may take a while, linear search.
                //
                for( Database::iterator di = db->begin(); di != db->end(); ++di )
                {
//                     LG_IDX_D << "di first:" << di->first << endl;
//                     LG_IDX_D << "di second:" << di->second << endl;
//                     LG_IDX_D << " wanted:" << earl << endl;
//                     LG_IDX_D << " offset:" << getSecondaryDBOffset() << endl;
//                     LG_IDX_D << endl;

                    if( getSecondaryDBOffset() > di->second.length() )
                        continue;
                    
                    if( di->second.substr( getSecondaryDBOffset() )  == earl )
                    {
//                        LG_IDX_D << "di->second == earl" << endl;
                    
                        docid_t id = toType<docid_t>( di->first );
//                        LG_IDX_D << "id:" << id << endl;

                        if( !id || isRevoked( id ) )
                            continue;
                        
                        return lookup( id );
                    }
//                    LG_IDX_D << "looping..." << endl;
                
                }
            }
            return 0;
        }
        
        
        fh_doc
        DocumentMap::lookup( fh_context c )
        {
            return lookupByURL( c->getURL() );
            
//             fh_database db = getDB();

//             if( m_useSecondaryIndex )
//             {
//                 LG_IDX_D << "DocumentMap::lookup() using secondary. [good]" << endl;
                
//                 //
//                 // Quick check using the secondary index
//                 //
//                 Database::iterator di = m_secdb->find( c->getURL() );
//                 for( ; m_secdb->end() != di ; di.moveCursorNextDup() )
//                 {
//                     string sid = "0";
//                     di.getPrimaryKey( sid );
//                     docid_t id = toType<docid_t>( tostr(di.getRecNumber()) );
//                     LG_IDX_D << "DocumentMap::lookup() found using secondary..."
//                          << " c:" << c->getURL()
//                          << " id:" << id
// //                          << " first:" << di->first
// //                          << " second:" << di->second
//                          << endl;
// //                    LG_IDX_D << "id:" << id << endl;

//                     if( !id || isRevoked( id ) )
//                         continue;
                        
//                     return lookup( id );
//                 }
//                 return 0;
//             }
//             else
//              {
//                 LG_IDX_D << "DocumentMap::lookup() no secondary index... using linear search on primary index." << endl;
//                 //
//                 // This may take a while, linear search.
//                 //
//                 for( Database::iterator di = db->begin(); di != db->end(); ++di )
//                 {
// //                     LG_IDX_D << "di first:" << di->first << endl;
// //                     LG_IDX_D << "di second:" << di->second << endl;
// //                     LG_IDX_D << " wanted:" << c->getURL() << endl;
// //                     LG_IDX_D << " offset:" << getSecondaryDBOffset() << endl;
// //                     LG_IDX_D << endl;

//                     if( getSecondaryDBOffset() > di->second.length() )
//                         continue;
                    
//                     if( di->second.substr( getSecondaryDBOffset() )  == c->getURL() )
//                     {
// //                        LG_IDX_D << "di->second == c->getURL()" << endl;
                    
//                         docid_t id = toType<docid_t>( di->first );
// //                        LG_IDX_D << "id:" << id << endl;

//                         if( !id || isRevoked( id ) )
//                             continue;
                        
//                         return lookup( id );
//                     }
// //                    LG_IDX_D << "looping..." << endl;
                
//                 }
//             }
//             return 0;
        }
        
        
        void
        DocumentMap::sync()
        {
            for( m_documents_t::iterator di = m_documents.begin();
                 di != m_documents.end(); ++di )
            {
                di->second->sync();
            }

            string revokedstr = Util::createSeperatedList( m_revoked.begin(), m_revoked.end() );
            m_path_mgr->setConfig( IDXMGR_DOCMAP_REVOKED_ID_CACHE_K,   revokedstr );
            
            getDB()->sync();
            if( m_secdb )
                m_secdb->sync();
        }

        void
        DocumentMap::dumpTo( fh_ostream oss, bool asXML )
        {
            if( asXML )
                oss << "<documentmap size=\"" << getDB()->size() << "\" >" << endl;
            
            try
            {
                fh_database db = getDB();
                guint32 nkeys  = db->size();
//                LG_IDX_D << "nkeys:" << nkeys << endl;
                for( guint32 i=1; i <= nkeys; ++i )
                {
                    fh_doc doc = lookup( i );
                    if( !doc )
                        continue;
                    
                    if( asXML )
                        oss << "<doc id=\""  << doc->getID() << "\" "
                            << " url=\""     << doc->getURL() << "\""
                            << " weight=\""  << doc->getDocumentWeight() << "\""
                            << " irc=\""     << doc->getInvertedReferenceCount() << "\""
                            << " revoked=\"" << doc->isRevoked() << "\""
                            << " />" << endl;
                    else
                        oss << " " << doc->getID() << ", " << doc->getURL() << endl;
                }
            }
            catch( exception& e )
            {
                oss << "<error> Error traversing data:" << e.what() << " </error>" << endl;
            }

            if( asXML ) oss << "</documentmap>" << endl;
        }
        
        bool
        DocumentMap::isRevoked( docid_t id )
        {
            getDB();
            return( m_revoked.find( id ) != m_revoked.end() );
//             fh_doc d = lookup( id );
//             if( !d )
//                 return true;
//             return d->isRevoked();
        }
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        SkippedListChunk::SkippedListChunk( Term* t )
            :
            m_term( t )
        {
        }
        
        SkippedListChunk::~SkippedListChunk()
        {
        }

        void
        SkippedListChunk::setDirty( bool v )
        {
            getTerm()->setDirty();
        }
        
        bool
        SkippedListChunk::canFit( fh_doc d, int freq )
        {
            unsigned long maxitems = getIndex()->getInvertedSkiplistMaxSize();
            return m_hits.size() < maxitems;
        }

        bool
        SkippedListChunk::insert( fh_doc d, int freq )
        {
            if( !canFit( d, freq ) )
                return false;

            setDirty( true );
            m_hits.insert( make_pair( d->getID(), freq ));
            return true;
        }

        void
        SkippedListChunk::incrementCountForDocument( fh_doc d )
        {
            if( m_hits[ d->getID() ] == MAX_TERM_HIT_COUNT )
                return;
            
            setDirty( true );
            m_hits[ d->getID() ]++;
        }

        bool
        SkippedListChunk::doesDocumentContain( fh_doc d )
        {
            m_hits_t::iterator iter = m_hits.find( d->getID() );
            if( iter != m_hits.end() )
                return true;
            
            return false;
        }
        
        doctermfreq_t
        SkippedListChunk::getFreqOfTermInDocument( fh_doc d )
        {
            m_hits_t::iterator iter = m_hits.find( d->getID() );
            if( iter != m_hits.end() )
                return iter->second;
            
            return 0;
        }
        
        void
        SkippedListChunk::dumpTo( fh_ostream oss, bool asXML )
        {
            m_hits_t col = m_hits;
            for( m_hits_t::iterator iter = m_revoked_hits.begin(); iter != m_revoked_hits.end(); ++iter )
                col[ iter->first ] = iter->second;
            
            bool v=true;
            for( m_hits_t::iterator iter = col.begin(); iter != col.end(); ++iter )
            {
                if( asXML )
                {
                    oss << "  <d id=\"" << iter->first << "\" freq=\"" << iter->second << "\" "
                        << " w_d_t=\"" << ( 1 + log( iter->second ) ) << "\" "
                        << "/>\n";
                }
                else
                {
                    if( !v )
                        oss << ", ";
                    oss << iter->first << "," << iter->second;
                    v = false;
                }
            }
        }
        

        docNumSet_t&
        SkippedListChunk::getDocumentNumbers( docNumSet_t& z )
        {
            for( m_hits_t::iterator iter = m_hits.begin(); iter != m_hits.end(); ++iter )
            {
                z.insert( iter->first );
            }
            return z;
        }
        
        
        Term*
        SkippedListChunk::getTerm()
        {
            return m_term;
        }

        termid_t
        SkippedListChunk::getID()
        {
            return getTerm()->getID();
        }
        
        
        fh_invertedfile
        SkippedListChunk::getInvertedFile()
        {
            return getTerm()->getInvertedFile();
        }
        
        fh_nidx
        SkippedListChunk::getIndex()
        {
            return getInvertedFile()->getIndex();
        }
        
        fh_database
        SkippedListChunk::getDB()
        {
            return getInvertedFile()->getDB();
        }



/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

        
        fh_skiplist
        SkippedListChunk::load( fh_term term, fh_istream ss )
        {
            fh_skiplist ret = new SkippedListChunk( GetImpl(term) );

            skippedchunksize_t numberOfDocumentsInList = 0;
            skippedchunksize_t lengthOfChunkPayload    = 0;
            docid_t firstDocID = 0;
            docid_t  lastDocID = 0;

            readnum( ss, firstDocID );
            readnum( ss, lastDocID  );
            readnum( ss, numberOfDocumentsInList );
            readnum( ss, lengthOfChunkPayload );

            if( numberOfDocumentsInList )
            {
                ret->m_hits[ firstDocID ] = 0;
                ret->m_hits[ lastDocID  ] = 0;
            }

            streamsize startOfBlockTellg = ss.tellg();

            LG_IDX_D << "SkippedListChunkload( START ) tid:" << term->getID()
                     << " firstDocID:" << firstDocID
                     << " lastDocID:"  << lastDocID
                     << " numberOfDocumentsInList:" << numberOfDocumentsInList
                     << " lengthOfChunkPayload:" << lengthOfChunkPayload
                     << " tellg:" << ss.tellg()
                     << endl;

            // ss is tellg()==16 bytes at this point.
            fh_istream compressedss = ss;
            streamsize processedPayloadSize = 0;

            //
            // docnum for 2nd to 2nd last documents
            //
            for( int i=0; i < (numberOfDocumentsInList-2); ++i )
            {
                string dgap_code = term->getInvertedFile()->getIndex()->getDocumentNumberGapCode();

                if( dgap_code == "Interpolative" )
                {
                    vector< docid_t > tmp;
                    int bread = decode_interpolative<>( istreambuf_iterator<char>(compressedss),
                                                        lengthOfChunkPayload,
                                                        numberOfDocumentsInList-2,
                                                        firstDocID,
                                                        lastDocID,
                                                        tmp );
                    processedPayloadSize += bread;
                    compressedss.seekg( (-1 * lengthOfChunkPayload) + bread, ios::cur );

                    LG_IDX_D << "load() tid:" << term->getID()
                             << " ret:" << toVoid(ret)
                             << " dgap_code:" << dgap_code
                             << " processedPayloadSize:" << processedPayloadSize
                             << " tmp.size:" << tmp.size()
                             << " numberOfDocumentsInList:" << numberOfDocumentsInList
                             << " bread:" << bread
                             << " firstDocID:" << firstDocID
                             << " lastDocID:" << lastDocID
                             << endl;
                    
                    
                    for( vector< docid_t >::const_iterator ti = tmp.begin(); ti != tmp.end(); ++ti )
                    {
                        ret->m_hits[ *ti ] = 0;
                    }

                    if( ret->m_hits.size() != numberOfDocumentsInList )
                    {
                        // oh no!
                        fh_stringstream ss;
                        ss << "SkippedListChunk::load() tid:" << term->getID()
                           << " Failed to load all coded document numbers." << endl
                           << " dgap_code:" << dgap_code
                           << " processedPayloadSize:" << processedPayloadSize
                           << " could load:" << tmp.size()+2
                           << " should have loaded:" << numberOfDocumentsInList
                           << endl;
                        Throw_FullTextIndexException( tostr(ss), 0 );
                    }
                    break;
                }
                else if( dgap_code != "None" )
                {
                    vector< docid_t > tmp;
                    processedPayloadSize +=
                        decodeDocumentNumbers( compressedss,
                                               dgap_code,
                                               numberOfDocumentsInList-2,
                                               lengthOfChunkPayload,
                                               tmp,
                                               lastDocID,
                                               numberOfDocumentsInList-2 );
                    convertFromDGaps( tmp.begin(), tmp.end(), ret->m_hits );

                    LG_IDX_D << "load() tid:" << term->getID()
                             << " ret:" << toVoid(ret)
                             << " dgap_code:" << dgap_code
                             << " processedPayloadSize:" << processedPayloadSize
                             << " tmp.size:" << tmp.size()
                             << endl;

                    if( ret->m_hits.size() != numberOfDocumentsInList )
                    {
                        // oh no!
                        fh_stringstream ss;
                        ss << "SkippedListChunk::load() tid:" << term->getID()
                           << " Failed to load all coded document numbers." << endl
                           << " dgap_code:" << dgap_code
                           << " processedPayloadSize:" << processedPayloadSize
                           << " tmp.size():" << tmp.size()
                           << " could load:" << ret->m_hits.size()
                           << " should have loaded:" << numberOfDocumentsInList
                           << endl;
                        Throw_FullTextIndexException( tostr(ss), 0 );
                    }
                    
//                     LG_IDX_D << "SkippedListChunk::load() tmp is   : ";
//                     copy( tmp.begin(), tmp.end(), ostream_iterator<int>( cerr, " " ));
//                     LG_IDX_D << endl;

//                     LG_IDX_D << "SkippedListChunk::load() m_hits is: ";
//                     copy( map_domain_iterator(ret->m_hits.begin()),
//                           map_domain_iterator(ret->m_hits.end()),
//                           ostream_iterator<int>( cerr, " " ));
//                     LG_IDX_D << endl;
                    break;
                }
                else
                {
                    docid_t d = 0;
                    readnum( compressedss, d );
                    processedPayloadSize += sizeof(d);
                    ret->m_hits[ d ] = 0;
                }
            }

            LG_IDX_D << "SkippedListChunk::load( f(d,t) START ) tid:" << term->getID()
                     << " stream.good:" << compressedss.good()
                     << " tellg:" << compressedss.tellg()
                     << " lengthOfChunkPayload:" << lengthOfChunkPayload
                     << " processedPayloadSize:" << processedPayloadSize
                     << " fdt byte range:" << (lengthOfChunkPayload - processedPayloadSize)
                     << " count-of-fdt to decode:" << ret->m_hits.size()
                     << endl;

            // d(f,t) for all docs in chunk
            string fdt_code = term->getInvertedFile()->getIndex()->getFrequencyOfTermInDocumentCode();
            if( fdt_code != "None" )
            {
                list< doctermfreq_t > out;
                int br = 0;
                
                if( fdt_code == "Delta" )
                    br = decode< BitCoder< DeltaPolicy > >( istreambuf_iterator<char>(compressedss),
                                                            lengthOfChunkPayload - processedPayloadSize,
                                                            ret->m_hits.size(),
                                                            out );
                else
                    br = decode< BitCoder< GammaPolicy > >( istreambuf_iterator<char>(compressedss),
                                                            lengthOfChunkPayload - processedPayloadSize,
                                                            ret->m_hits.size(),
                                                            out );

                if( ret->m_hits.size() != out.size() )
                {
                    fh_stringstream ss;
                    ss << "SkippedListChunk::load() tid:" << term->getID()
                       << " failed to load all document term frequency numbers"
                       << " could load:" << out.size()
                       << " should have loaded:" << ret->m_hits.size()
                       << " numberOfDocumentsInList:" << numberOfDocumentsInList
                       << " lengthOfChunkPayload:" << lengthOfChunkPayload
                       << endl;
                    Throw_FullTextIndexException( tostr(ss), 0 );
                }
                
                list< doctermfreq_t >::iterator dft = out.begin();
                for( m_hits_t::iterator iter = ret->m_hits.begin(); iter != ret->m_hits.end(); ++iter )
                {
                    iter->second = *dft;
                    LG_IDX_D << "chunk::load() tid:" << term->getID()
                             << " d:" << iter->first
                             << " f:" << iter->second << endl;
                    ++dft;
                }
            }
            else
            {
                for( m_hits_t::iterator iter = ret->m_hits.begin();
                     iter != ret->m_hits.end(); ++iter )
                {
                    doctermfreq_t n = 0;
                    readnum( compressedss, n );

                    LG_IDX_D << "SkippedListChunk::load( f(d,t) ) tid:" << term->getID()
                             << " firstDocID:" << firstDocID
                             << " lastDocID:" << lastDocID
                             << " numberOfDocumentsInList:" << numberOfDocumentsInList
                             << " lengthOfChunkPayload:" << lengthOfChunkPayload
                             << " docid:"  << iter->first
                             << " f(d,t):" << n
                             << " compressedss.good:" << compressedss.good()
                             << endl;
                
                    iter->second = n;
                }
            }

            streamsize endOfBlockTellg = ss.tellg();
            if( endOfBlockTellg - startOfBlockTellg > lengthOfChunkPayload )
            {
                LG_IDX_D << "SkippedListChunk::load( ERROR! ) tid:" << term->getID()
                         << " read too many bytes! "
                         << " endOfBlockTellg:" << endOfBlockTellg
                         << " startOfBlockTellg:" << startOfBlockTellg
                         << " lengthOfChunkPayload:" << lengthOfChunkPayload
                         << endl;
            }
            else
                LG_IDX_D << "SkippedListChunk::load( OK-Done ) tid:" << term->getID()
                         << " endOfBlockTellg:" << endOfBlockTellg
                         << " startOfBlockTellg:" << startOfBlockTellg
                         << " lengthOfChunkPayload:" << lengthOfChunkPayload
                         << endl;
                
            ret->moveRevokedToMRevokedHits();
            return ret;
        }


        void
        SkippedListChunk::moveRevokedToMRevokedHits()
        {
            fh_docmap dm = getIndex()->getDocumentMap();
            
            for( m_hits_t::iterator mi = m_hits.begin(); mi!=m_hits.end(); )
            {
                if( dm->isRevoked( mi->first ) )
                {
                    m_revoked_hits.insert( make_pair( mi->first, mi->second ) );
                    m_hits_t::iterator tmp = mi;
                    ++mi;
                    m_hits.erase( tmp );
                    continue;
                }
                ++mi;
            }
        }
        
        void
        SkippedListChunk::attemptToPurgeRevoked()
        {
            fh_docmap dm = getIndex()->getDocumentMap();

//            LG_IDX_D << "SkippedListChunk::attemptToPurgeRevoked() revoked.size:" << m_revoked_hits.size() << endl;
            
            if( !m_revoked_hits.empty() )
            {
//                setDirty( true );

                LG_IDX_D << "attemptToPurgeRevoked() id:" << getTerm()->getID()
                         << " revoked.size:" << m_revoked_hits.size()
                         << endl;

                for( m_hits_t::iterator mi = m_revoked_hits.begin(); mi!=m_revoked_hits.end(); ++mi )
                {
                    if( dm->isRevoked( mi->first ) )
                    {
                        fh_doc d = dm->lookup( mi->first );
                        d->decrInvertedReferenceCount();
                    }
                }
            }
            
                
//             bool changed = false;
//             fh_docmap dm = getIndex()->getDocumentMap();
            
//             for( m_hits_t::iterator mi = m_hits.begin(); mi!=m_hits.end(); )
//             {
//                 if( dm->isRevoked( mi->first ) )
//                 {
//                     fh_doc d = dm->lookup( mi->first );
//                     changed = true;
//                     d->decrInvertedReferenceCount();
//                     m_hits_t::iterator garbo = mi;
//                     ++mi;
//                     m_hits.erase( garbo );
//                     continue;
//                 }
//                 ++mi;
//             }

//             if( changed )
//                 setDirty( true );
        }
        
        
        void
        SkippedListChunk::save( fh_ostream ss, termid_t tid )
        {
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

                if( dgap_code == "Interpolative" )
                {
                    int bw = encode_interpolative<>( map_domain_iterator(iter),
                                                     map_domain_iterator(lastiter),
                                                     firstDocID,
                                                     lastDocID,
                                                     compressedss );
                    LG_IDX_D << "SkippedListChunk::save(interpol dgap)  tid:" << tid
                             << " bw:" << bw
                             << " compressedss.len:" << tostr(compressedss).length()
                             << " count-of-compressed-doc-ids:" << distance( iter, lastiter )
                             << " firstDocID:" << firstDocID
                             << " lastDocID:" << lastDocID
                             << " compressedss.tellp():" << compressedss.tellp()
                             << " compressedss.good():" << compressedss.good()
                             << endl;
                }
                else if( dgap_code != "None" )
                {
//                     // DEBUG
//                     {
//                         cerr << "SkippedListChunk::save(dnum) tid:" << tid
//                              << " for compressed mode:" << dgap_code << endl;
//                         cerr << "iter to lastiter are: ";
//                         copy( map_domain_iterator(iter), map_domain_iterator(lastiter),
//                               ostream_iterator<int>( cerr, " " ));
//                         cerr << endl;
//                     }

                    
                    list< docid_t > tmp;
                    convertToDGaps( map_domain_iterator(iter),
                                    map_domain_iterator(lastiter),
                                    tmp );

                    if( tmp.size() != distance( iter, lastiter ) )
                    {
                        fh_stringstream ss;
                        ss << "SkippedListChunk::save() tid:" << tid
                           << " failed to convert document numbers into dgaps."
                           << " converted list size:" << tmp.size()
                           << " should be:" << distance( iter, lastiter )
                           << endl;
                        Throw_FullTextIndexException( tostr(ss), 0 );
                    }
                    
//                     // DEBUG
//                     {
//                         cerr << "SkippedListChunk::save(dgap) tid:" << tid
//                              << " for compressed mode:" << dgap_code << endl;
//                         cerr << "tmp is: ";
//                         copy( tmp.begin(), tmp.end(),
//                               ostream_iterator<int>( cerr, " " ));
//                         cerr << endl;
//                     }

                    
                    int bwr = encodeDocumentNumbers<>( compressedss, dgap_code, tmp.begin(), tmp.end(),
                                                       lastDocID, tmp.size() );

                    LG_IDX_D << "SkippedListChunk::save()  tid:" << tid
                             << " bwr:" << bwr
                             << " compressedss.tellp():" << compressedss.tellp()
                             << " compressedss.good():" << compressedss.good()
                             << endl;
                }
                else 
                {
                    for( ; iter != lastiter; ++iter )
                    {
                        writenum( compressedss, iter->first );
                    }
                }
            }

            LG_IDX_D << "saved compressed document IDs...  tid:" << tid
                     << " compressedss.tellp():" << compressedss.tellp()
                     << " compressedss.good():" << compressedss.good()
                     << " length:" << tostr( compressedss ).length()
                     << endl;

            // d(f,t) for all docs in chunk
            string fdt_code = getIndex()->getFrequencyOfTermInDocumentCode();
            if( fdt_code != "None" )
            {
                int bw = 0;
                
                if( fdt_code == "Delta" )
                    bw =
                        encode< BitCoder< DeltaPolicy > >(
                            map_range_iterator(m_hits.begin()),
                            map_range_iterator(m_hits.end()),
                            compressedss );
                else
                    bw =
                        encode< BitCoder< GammaPolicy > >(
                            map_range_iterator(m_hits.begin()),
                            map_range_iterator(m_hits.end()),
                            compressedss );

                LG_IDX_D << "saved compressed document IDs and f(d,t)... tid:" << tid
                         << " bw:" << bw
                         << " compressedss.tellp():" << compressedss.tellp()
                         << " compressedss.good():" << compressedss.good()
                         << " len:" << tostr(compressedss).length() 
                         << endl;
            }
            else
            {
                
                for( m_hits_t::iterator iter = m_hits.begin(); iter != m_hits.end(); ++iter )
                {
                    LG_IDX_D << "SkippedListChunk::save(  f(d,t)  ) this:" << toVoid(this)
                             << " docid:"  << iter->first
                             << " f(d,t):" << iter->second
                             << endl;
                    writenum( compressedss, iter->second );
                }
            }
            
            //
            // write the header and payload
            //
            compressedss << flush;
            string compressedData = tostr( compressedss );
            lengthOfChunkPayload  = compressedData.length();

            LG_IDX_D << "saved compressed document IDs and f(d,t)... tid:" << tid
                     << " compressedss.tellp():" << compressedss.tellp()
                     << " firstDocID:" << firstDocID
                     << " lastDocID:" << lastDocID
                     << " lengthOfChunkPayload:" << lengthOfChunkPayload
                     << " numberOfDocumentsInList:" << numberOfDocumentsInList
                     << endl;

            writenum( ss, firstDocID );
            writenum( ss, lastDocID  );
            writenum( ss, numberOfDocumentsInList );
            writenum( ss, lengthOfChunkPayload );
            writestring( ss, compressedData );
        }

        streamsize
        SkippedListChunk::getHeaderSize()
        {
            streamsize offset = sizeof( docid_t )
                + sizeof( docid_t )
                + sizeof( skippedchunksize_t )
                + sizeof( skippedchunksize_t );
            return offset;
        }
        
        bool
        SkippedListChunk::doesChunkContainDocument( fh_istream ss, docid_t d )
        {
            skippedchunksize_t numberOfDocumentsInList = 0;
            skippedchunksize_t lengthOfChunkPayload    = 0;
            docid_t firstDocID = 0;
            docid_t  lastDocID = 0;

            readnum( ss, firstDocID );
            readnum( ss, lastDocID  );
            readnum( ss, numberOfDocumentsInList );
            readnum( ss, lengthOfChunkPayload );

            if( firstDocID <= d && d <= lastDocID )
            {
                streamsize offset = -1 * getHeaderSize();
                ss.seekg( offset, ios::cur );
                return true;
            }

            // skip to start of next chunk
            ss.seekg( lengthOfChunkPayload, ios::cur );
            return false;
        }
        

        int
        SkippedListChunk::getNumberOfBytesToStoreDocumentIDs()
        {
            fh_stringstream junkss;
            int sz = sizeof( docid_t ) * 2;
            
            if( m_hits.size() > 2 )
            {
                m_hits_t::iterator iter     = m_hits.begin();
                m_hits_t::iterator lastiter = m_hits.end();
                ++iter;
                --lastiter;

                string dgap_code = getIndex()->getDocumentNumberGapCode();

                
                if( dgap_code == "Interpolative" )
                {
                    fh_stringstream junkss;
                    int rc = encode_interpolative<>( map_domain_iterator(iter),
                                                     map_domain_iterator(lastiter),
                                                     m_hits.begin()->first,
                                                     lastiter->first,
                                                     junkss );
                    sz += rc;
                }
                if( dgap_code != "None" )
                {
            
                    list< docid_t > tmp;
                    convertToDGaps( map_domain_iterator(iter),
                                    map_domain_iterator(lastiter),
                                    tmp );

                    int rc = encodeDocumentNumbers<>( junkss, dgap_code, tmp.begin(), tmp.end(),
                                                      lastiter->first, tmp.size() );
                    LG_IDX_D << "SkippedListChunk::getNumberOfBytesToStoreDocumentIDs() rc:" << rc << endl;
                    sz += rc;
                }
                else
                    sz += sizeof( docid_t ) * (m_hits.size() - 2);
            }
            return sz;
        }
        
        int
        SkippedListChunk::getNumberOfBytesToStoreFDTs()
        {
            string fdt_code = getIndex()->getFrequencyOfTermInDocumentCode();
            if( fdt_code != "None" )
            {
                fh_stringstream junkss;
                int bw = 0;
                
                if( fdt_code == "Delta" )
                    bw =
                        encode< BitCoder< GammaPolicy > >(
                            map_range_iterator(m_hits.begin()),
                            map_range_iterator(m_hits.end()),
                            junkss );
                else
                    bw =
                        encode< BitCoder< GammaPolicy > >(
                            map_range_iterator(m_hits.begin()),
                            map_range_iterator(m_hits.end()),
                            junkss );

                return bw;
            }
            else
            {
                return sizeof( doctermfreq_t ) * m_hits.size();
            }
        }
        
        
        /********************************************************************************/
        /********************************************************************************/
        
        Term::Term( fh_invertedfile inv, termid_t id, bool created )
            :
            m_termFreq( 0 ),
            m_onDiskChunkCount( 0 ),
            m_id( id ),
            m_dirty( created ),
            m_readAllChunks( created ),
            m_inv( inv )
        {
        }
        
        Term::~Term()
        {
            LG_IDX_D << "~Term() id:" << m_id << " dirty:" << m_dirty << endl;

            sync();
        }

        void
        Term::sync()
        {
            if( m_dirty )
            {
                save();
            }
        }
        
        void
        Term::save( int put_flags )
        {
            fh_database db = m_inv->getDB();
            
            fh_stringstream datass;
            toStream( datass );
            db_recno_t recno = m_id;

            string payload = tostr(datass);
            db->set( recno, payload, put_flags );
            

            
//             Dbt key ( (void*)&recno, sizeof(recno) );
//             string payload = tostr(datass);
//             Dbt data( (void*)payload.data(), payload.size() );
//             int rc = db->put( 0, &key, &data, put_flags );

            m_dirty = false;
            
            LG_IDX_D << "Term::save() id:" << m_id
                     << " payload.size:" << payload.size()
                     << endl;
        }
        
        void
        Term::save()
        {
            save( 0 );
        }
        
        void
        Term::append()
        {
            save( DB_APPEND );
        }
        
        fh_term
        Term::load( fh_invertedfile inv, termid_t id )
        {
            fh_term ret = new Term( inv, id );
            if( !ret->readAllChunks() )
            {
                LG_IDX_W << "term::load() no such term for id:" << id << endl;
                return 0;
            }
            return ret;
        }
        
        void
        Term::setDirty( bool v )
        {
            if( !m_dirty && !readAllChunks() )
            {
                LG_IDX_ER << "ERROR, FAILED TO LOAD ALL CHUNKS FOR term for id:" << m_id << endl;
            }
            m_dirty = true;
        }
        
        void
        Term::setID( termid_t id )
        {
            setDirty();
            m_id = id;
        }
        
        termid_t
        Term::getID()
        {
            return m_id;
        }
        

        fh_skiplist
        Term::createNewChunk()
        {
            fh_skiplist c = new SkippedListChunk( this );
            m_chunks.push_back( c );
            ++m_onDiskChunkCount;
            return c;
        }

        fh_skiplist
        Term::getLastChunkOrNewChunk()
        {
            fh_skiplist c = 0;
            if( !m_chunks.empty() )
            {
                m_chunks_t::iterator ci = m_chunks.end();
                --ci;
                c = *ci;
            }
            else
            {
                c = createNewChunk();
            }
            return c;
        }
        
        void
        Term::insert( fh_doc d, int freq )
        {
            LG_IDX_D << "Term::insert() m_id:" << m_id << " doc:" << d->getID() << endl;

            readAllChunks();
            setDirty();

            fh_skiplist lastchunk = getLastChunkOrNewChunk();

            ++m_termFreq;
            if( !lastchunk->insert( d, freq ) )
            {
                //
                // chunk is full, create new chunk
                //
                LG_IDX_D << "term::insert() chunk is full, making new chunk for termid:" << m_id << endl;
                lastchunk = createNewChunk();
                lastchunk->insert( d, freq );
            }
            
            LG_IDX_D << "Term::insert(end) m_id:" << m_id << " doc:" << d->getID()
                     << " m_chunks.size:" << m_chunks.size()
                     << " m_onDiskChunkCount:" << m_onDiskChunkCount
                     << " dirty:" << m_dirty
                     << endl;
        }

        fh_skiplist
        Term::getSkipListForDocument( fh_doc d )
        {
            if( !m_readAllChunks )
            {
                //
                // test the cached chunks first
                //
                for( m_chunks_t::iterator ci = m_chunks.begin(); ci != m_chunks.end(); ++ci )
                {
                    if( (*ci)->doesDocumentContain( d ))
                        return *ci;
                }
                
                //
                // try to find the chunk that is needed and avoid loading the
                // unrequired chunks for now
                // 
                fh_stringstream ss;
                if( readPayloadFromDatabase( ss ) )
                {
                    for( int i=0; i < m_onDiskChunkCount; ++i )
                    {
                        if( SkippedListChunk::doesChunkContainDocument( ss, d->getID() ) )
                        {
                            //
                            // found the chunk containing the wanted ID.
                            //
                            fh_skiplist c = SkippedListChunk::load( this, ss );
                            m_chunks.push_back( c );
                            return c;
                        }
                    }
                }
            }
            
            readAllChunks();
            
            for( m_chunks_t::iterator ci = m_chunks.begin(); ci != m_chunks.end(); ++ci )
            {
                if( (*ci)->doesDocumentContain( d ))
                    return *ci;
            }
            return 0;
        }
        
        void
        Term::incrementCountForDocument( fh_doc d )
        {
            fh_skiplist sl = getSkipListForDocument( d );
            if( sl )
            {
                sl->incrementCountForDocument( d );
            }
        }
        

        bool
        Term::doesDocumentContain( fh_doc d )
        {
            fh_skiplist sl = getSkipListForDocument( d );
            return isBound(sl);
        }

        doctermfreq_t
        Term::getFreqOfTermInDocument( fh_doc d )
        {
            fh_skiplist sl = getSkipListForDocument( d );
            if( sl )
            {
                return sl->getFreqOfTermInDocument( d );
            }
            return 0;
        }
        
        doctermfreq_t
        Term::getFreqOfTerm()
        {
            return m_termFreq;
        }
        
        bool
        Term::readPayloadFromDatabase( fh_stringstream ss )
        {
            fh_database db = getInvertedFile()->getDB();
            db_recno_t recno = m_id;

            try
            {
                string payload = db->get( recno );
                ss << payload;
            }
            catch( exception& e )
            {
                LG_IDX_D << "Term::readPayloadFromDatabase() failed to load data for"
                         << " m_id:" << m_id << endl;
                return false;
            }
            return true;
            
            

//             Dbt key ( (void*)&recno, sizeof(recno) );
//             Dbt data( 0, 0 );
//             db->get( 0, &key, &data, 0 );
//             if( data.get_size() )
//             {
//                 ss.write( (const char*)data.get_data(), data.get_size() );
//                 return true;
//             }

//             LG_IDX_D << "Term::readPayloadFromDatabase() failed to load data for key:" << m_id << endl;
//             return false;
        }
        

        bool
        Term::readAllChunks()
        {
            if( m_readAllChunks )
                return true;

            m_chunks.clear();
            
            fh_stringstream ss;
            if( readPayloadFromDatabase( ss ) )
            {
                m_termFreq         = 0;
                m_onDiskChunkCount = 0;
                readnum( ss, m_termFreq );
                readnum( ss, m_onDiskChunkCount );

                LG_IDX_D << "Term::readAllChunks() for term:" << m_id
                         << " m_termFreq:" << m_termFreq
                         << " m_onDiskChunkCount:" << m_onDiskChunkCount
                         << " payload.size:" << tostr(ss).length()
                         << endl;
//                m_chunks.reserve( m_onDiskChunkCount );
                for( int i=0; i < m_onDiskChunkCount; ++i )
                {
                    fh_skiplist sl = SkippedListChunk::load( this, ss );
                    m_chunks.push_back( sl );
                }
                
                m_readAllChunks = true;
                return true;
            }
            LG_IDX_D << "Term::readAllChunks( FAILED ) for term:" << m_id
                     << " m_termFreq:" << m_termFreq
                     << " m_onDiskChunkCount:" << m_onDiskChunkCount
                     << " payload.size:" << tostr(ss).length()
                     << endl;
            return false;
        }
        

        
        fh_ostream
        Term::toStream( fh_ostream oss )
        {
            writenum( oss, m_termFreq );
            writenum( oss, m_onDiskChunkCount );
            
            readAllChunks();

            if( !m_readAllChunks )
            {
                LG_IDX_ER << "ERROR, saving inverted list when all chunks are not loaded!" << endl;
            }

            LG_IDX_D << "term::toStream() id:" << m_id << " chunks.size:" << m_chunks.size()
                     << " termFreq:" << m_termFreq
                     << " m_onDiskChunkCount:" << m_onDiskChunkCount
                     << endl;
            
            for( m_chunks_t::iterator ci = m_chunks.begin(); ci != m_chunks.end(); ++ci )
            {
                (*ci)->save( oss, m_id );
            }
            return oss;
        }

        documentWeight_t
        Term::getWeight()
        {
            documentWeight_t N = m_inv->getNumberOfTerms();
            doctermfreq_t  f_t = getFreqOfTerm();
            documentWeight_t w_t = log( N / f_t + 1 );
            LG_IDX_D << " Term::getWeight() id:" << m_id
                     << " N:" << N << " f_t:" << f_t << " w_t:" << w_t << endl;
            return w_t;
        }
        
        
        
        void
        Term::dumpTo( fh_ostream oss, bool asXML, bool includeDiskSizes )
        {
            readAllChunks();

            if( asXML )
            {
                fh_database db = getInvertedFile()->getDB();
                db_recno_t recno = m_id;

                string payload = db->get( recno );
                
//                 Dbt key ( (void*)&recno, sizeof(recno) );
//                 Dbt data( 0, 0 );
//                 db->get( 0, &key, &data, 0 );

                oss << "<term id=\"" << m_id << "\" matches=\"" << m_termFreq << "\" ";
                oss << " w_t=\"" << getWeight() << "\" ";
                oss << " chunkcount=\"" << m_onDiskChunkCount << "\" ";
                oss << " ondisksize=\"" << payload.size() << "\" ";
                if( includeDiskSizes )
                {
                    oss << " coded_docid_size=\"" << getNumberOfBytesToStoreDocumentIDs() << "\" ";
                    oss << " coded_fdt_size=\""   << getNumberOfBytesToStoreFDTs()        << "\" ";
                }
                oss << " >\n<doclist>" << endl;
            }
            
            else
                oss << " " << m_id << " " << m_termFreq << " { ";
            
            bool v=true;
            for( m_chunks_t::iterator ci = m_chunks.begin(); ci != m_chunks.end(); ++ci )
            {
                (*ci)->dumpTo( oss, asXML );
            }

            if( asXML )
                oss << "</doclist></term>" << endl;
            else
                oss << " } " << endl;
        }

        docNumSet_t&
        Term::getDocumentNumbers( docNumSet_t& z )
        {
            readAllChunks();
            for( m_chunks_t::iterator ci = m_chunks.begin(); ci != m_chunks.end(); ++ci )
            {
                (*ci)->getDocumentNumbers( z );
            }
            return z;
        }
        

        fh_invertedfile
        Term::getInvertedFile()
        {
            return m_inv;
        }
        

        int
        Term::getNumberOfBytesToStoreDocumentIDs()
        {
            int sz = 0;
            for( m_chunks_t::iterator ci = m_chunks.begin(); ci != m_chunks.end(); ++ci )
            {
                sz += (*ci)->getNumberOfBytesToStoreDocumentIDs();
            }
            return sz;
        }
        
        int
        Term::getNumberOfBytesToStoreFDTs()
        {
            int sz = 0;
            for( m_chunks_t::iterator ci = m_chunks.begin(); ci != m_chunks.end(); ++ci )
            {
                sz += (*ci)->getNumberOfBytesToStoreFDTs();
            }
            return sz;
        }
        

        
        /********************************************************************************/
        /********************************************************************************/

        fh_database
        InvertedFile::getDB()
        {
            if( !m_db )
            {
                LG_IDX_W << "opening index file at:" << (m_idx->getBasePath() + "/invertedfile.db" ) << endl;

                m_db = new Database( m_dbenv );
                m_db->set_pagesize( 64*1024 );

                m_db->open( CleanupURL( m_idx->getBasePath() + "/invertedfile.db" ),
                            "",
                            DB_RECNO,
                            DB_CREATE,
                            0600 );
                
//                 m_db = new Db(0,0);
//                 m_db->set_pagesize( 64*1024 );
//                 m_db->open( CleanupURL( m_idx->getBasePath() + "/invertedfile.db" ).c_str(),
//                             0,
//                             DB_RECNO,
//                             DB_CREATE,
//                             0600 );
            }
            return m_db;
        }
        
        InvertedFile::InvertedFile( fh_nidx idx, fh_env dbenv )
            :
            m_dbenv( dbenv ),
            m_db( 0 ),
            m_idx( idx )
        {
        }

        InvertedFile::~InvertedFile()
        {
            try
            {
                sync();
            }
            catch( exception& e )
            {
                LG_IDX_ER << "InvertedFile::~InvertedFile() e:" << e.what() << endl;
            }
        }

        fh_term
        InvertedFile::getTerm( termid_t id )
        {
            return getCachedTerm( id );
        }

        termid_t
        InvertedFile::getNumberOfTerms()
        {
            return getDB()->size();
        }
        
        
        fh_term
        InvertedFile::insert()
        {
            try
            {
                fh_database db = getDB();
                guint32 nkeys = db->size();
                LG_IDX_D << "InvertedFile::insert() number of keys:" << nkeys << endl;
                db_recno_t recno = nkeys + 1;
                fh_term ret = new Term( this, recno, true );
                
                ret->setID( recno );

                // We either need to cache the nkeys or save the new term
                // to the db here to ensure that the next term added has the
                // next logical ID
                ret->append();

//                m_term_cache.put( recno, ret );
                
                return ret;
            }
            catch( exception& e )
            {
                LG_IDX_ER << "InvertedFile::insert() e:" << e.what() << endl;
                throw;
            }
        }

        void
        InvertedFile::dumpTo( fh_ostream oss, bool asXML )
        {
            try
            {
                if( asXML ) oss << "<invertedfile>" << endl;
                
                fh_database db = getDB();
                guint32 nkeys = db->size();
                LG_IDX_D << "InvertedFile::dumpTo() nkeys:" << nkeys << endl;
                for( termid_t i=1; i <= nkeys; ++i )
                {
                    fh_term term = getTerm( i );
                    term->dumpTo( oss );
                }

                if( asXML ) oss << "</invertedfile>" << endl;
            }
            catch( exception& e )
            {
                oss << "<error> Error traversing data:" << e.what() << " </error>" << endl;
            }
        }

        void
        InvertedFile::compact( fh_ostream oss, bool verbose )
        {
            try
            {
                fh_database db = getDB();
                guint32 nkeys = db->size();
                if( verbose ) oss << "InvertedFile::compact() nkeys:" << nkeys << endl;
                
                for( termid_t i=1; i <= nkeys; ++i )
                {
                    fh_term term = getTerm( i );
                    term->setDirty( true );
                    term->sync();
                }
            }
            catch( exception& e )
            {
                oss << "<error> Error traversing data:" << e.what() << " </error>" << endl;
            }
        }
        
        
        void
        InvertedFile::sync()
        {
            getDB()->sync();

//             for( m_term_cache_t::iterator ti = m_term_cache.begin();
//                  ti != m_term_cache.end(); ++ti )
//             {
//                 ti->second->sync();
//             }
        }

        fh_term
        InvertedFile::getCachedTerm( termid_t tid )
        {
            fh_term t = Term::load( this, tid );
            return t;
            
//             fh_term t = m_term_cache.get( tid );
//             if( t )
//                 return t;

//             t = Term::load( this, tid );
//             if( t )
//                 m_term_cache.put( tid, t );
            
//             return t;
        }
        
        
        
        
        fh_nidx
        InvertedFile::getIndex()
        {
            return m_idx;
        }
        
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/


//         namespace
//         {
//             static const std::string MetaIndexClassName = "native";
//             static bool reged = MetaFullTextIndexerInterfaceFactory::Instance().
//                 Register( MetaIndexClassName,
//                           &FullTextIndexManagerNative::Create );
//             static bool regedx = appendToMetaFullTextIndexClassNames( MetaIndexClassName );
//         }

        MetaFullTextIndexerInterface*
        FullTextIndexManagerNative::Create()
        {
            return new FullTextIndexManagerNative();
        }
        
        FullTextIndexManagerNative::FullTextIndexManagerNative()
        {
        }

//         FullTextIndexManagerNative::FullTextIndexManagerNative( const std::string& basepath,
//                                                                 bool caseSensitive,
//                                                                 bool dropStopWords,
//                                                                 StemMode stemMode,
//                                                                 const std::string& ctr_lex_class )
//             :
//             m_basepath( basepath ),
//             m_config( 0 ),
//             m_configNotAvailable( false )
//         {
//             LG_IDX_D << "FullTextIndexManagerNative(create) basepath:" << m_basepath << endl;
//             Shell::acquireContext( m_basepath );

//             ensureConfigFileCreated();
//             setConfig( IDXMGR_LEXICON_CLASS_K,       ctr_lex_class );

// //             LG_IDX_D << "FullTextIndexManagerNative() givenlex:" << ctr_lex_class
// //                  << " read back:" << getConfig( IDXMGR_LEXICON_CLASS_K, "Nothing" )
// //                  << endl;
//             setConfig( IDXMGR_CASESEN_CLASS_K,       tostr(caseSensitive));
//             setConfig( IDXMGR_DROPSTOPWORDS_CLASS_K, tostr(dropStopWords));
//             setConfig( IDXMGR_STEMMER_CLASS_K,       tostr(stemMode));

//             common_construction();
//         }

        void
        FullTextIndexManagerNative::CreateIndex( fh_context c,
                                                 bool caseSensitive,
                                                 bool dropStopWords,
                                                 StemMode stemMode,
                                                 const std::string& lex_class,
                                                 fh_context md )
        {
            setConfig( IDXMGR_LEXICON_CLASS_K, lex_class );
        }
        
        

// void
// FullTextIndexManagerNative::ensureConfigFileCreated()
// {
//     try
//     {
//         fh_context cfg = Resolve( m_basepath + DB_FULLTEXT );
//     }
//     catch( exception& e )
//     {
//         fh_context c = Resolve( m_basepath );
//         string rdn = DB_FULLTEXT;
//         PrefixTrimmer trimmer;
//         trimmer.push_back( "/" );
//         rdn = trimmer( rdn );
//         fh_mdcontext md = new f_mdcontext();
//         fh_mdcontext child = md->setChild( "db4", "" );
//         child->setChild( "name", rdn );
//         fh_context newc   = c->createSubContext( "", md );
//     }
// }


        void
        FullTextIndexManagerNative::CommonConstruction()
        {
//            ensureConfigFileCreated();
            string lexicon_class = getConfig( IDXMGR_LEXICON_CLASS_K, IDXMGR_LEXICON_CLASS_DEFAULT );
            string inv_class     = getConfig( IDXMGR_INVERTEDFILE_CLASS_K,
                                              IDXMGR_INVERTEDFILE_CLASS_DEFAULT );

            LG_IDX_D << "FullTextIndexManagerNative::common_construction() lex class:" << lexicon_class << endl;
//            LG_IDX_D << "FullTextIndexManagerNative::common_construction() lex class:" << lexicon_class << endl;

            try
            {
                m_lex = LexiconFactory::Instance().CreateObject( lexicon_class );
            }
            catch( exception& e )
            {
                LG_IDX_ER << "cant make lexicon class:" << lexicon_class << " e:" << e.what() << endl;
                throw;
            }
            m_lex->setIndex( this );

            const u_int32_t  environment_open_flags = Environment::get_openflag_create()
//                | Environment::get_openflag_init_txn()
//                | Environment::get_openflag_init_lock()
//                | Environment::get_openflag_init_log()
                | Environment::get_openflag_init_mpool();
            fh_env dbenv = new Environment( getBasePath(), environment_open_flags );
            m_inv        = new InvertedFile( this, dbenv );
            m_docmap     =
                new DocumentMap( this, dbenv,
                                 this,
                                 toint( getConfig( IDXMGR_DOCMAP_USE_SECIDX_K,
                                                   IDXMGR_DOCMAP_USE_SECIDX_DEFAULT )) );

//            ensureConfigFileCreated();
            
            if( toint(getConfig( IDXMGR_LEXICON_VERSION_K, "0" )) > IDXMGR_LEXICON_VERSION_CURRENT )
            {
                string filename        = "lexicon";
                fh_stringstream ss;
                ss << "FullTextIndexManagerNative, " << filename << " file was created with a newer libferris than"
                   << " the one currently being used to access it. Please upgrade libferris"
                   << " to use this index";
                Throw_FullTextIndexException( tostr(ss), 0 );
            }
            if( toint(getConfig( IDXMGR_INVERTEDFILE_VERSION_K, "0" )) > IDXMGR_INVERTEDFILE_VERSION_CURRENT )
            {
                string filename        = "inverted";
                fh_stringstream ss;
                ss << "FullTextIndexManagerNative, " << filename << " file was created with a newer libferris than"
                   << " the one currently being used to access it. Please upgrade libferris"
                   << " to use this index";
                Throw_FullTextIndexException( tostr(ss), 0 );
            }
            if( toint(getConfig( IDXMGR_DOCUMENTMAP_VERSION_K, "0" )) > IDXMGR_DOCUMENTMAP_VERSION_CURRENT )
            {
//                 LG_IDX_D << " got:" << getConfig( IDXMGR_DOCUMENTMAP_VERSION_K, "0" )
//                      << " current:" << IDXMGR_DOCUMENTMAP_VERSION_CURRENT
//                      << endl;
                string filename        = "document map";
                fh_stringstream ss;
                ss << "FullTextIndexManagerNative, " << filename << " file was created with a newer libferris than"
                   << " the one currently being used to access it. Please upgrade libferris"
                   << " to use this index";
                Throw_FullTextIndexException( tostr(ss), 0 );
            }

            setConfig( IDXMGR_LEXICON_VERSION_K,      tostr(IDXMGR_LEXICON_VERSION_CURRENT) );
            setConfig( IDXMGR_INVERTEDFILE_VERSION_K, tostr(IDXMGR_INVERTEDFILE_VERSION_CURRENT) );
            setConfig( IDXMGR_DOCUMENTMAP_VERSION_K,  tostr(IDXMGR_DOCUMENTMAP_VERSION_CURRENT) );
        }
        


        FullTextIndexManagerNative::~FullTextIndexManagerNative()
        {
        }
        
        void
        FullTextIndexManagerNative::addToIndex( fh_context c,
                                                fh_docindexer di )
        {
            
            string s;
            fh_istream iss;
            try
            {
                fh_attribute a = c->getAttribute( "as-text" );
                iss = a->getIStream();
            }
            catch( exception& e )
            {
                iss = c->getIStream();
            }
            streamsize bytesDone    = 0;
            streamsize signalWindow = 0;
            streamsize totalBytes   = toType<streamsize>(getStrAttr( c, "size", "0" ));
            fh_invertedfile inv = getInvertedFile();
            fh_lexicon lex      = getLexicon();
            fh_docmap  docmap   = getDocumentMap();
            fh_doc document     = 0;
            
            if( !di->getDontCheckIfAlreadyThere() )
            {
                LG_IDX_D << "ABOUT TO TEST FOR Revoke document c:" << c->getURL() << endl;
                // see if we already have the URL in the database, if so revoke its ID.
                if( document = docmap->lookup( c ) )
                {
                    LG_IDX_D << "Revoking document c:" << c->getURL() << " id:" << document->getID() << endl;
                    document->revokeDocument();
                }
                LG_IDX_D << "AFTER TEST FOR Revoke document c:" << c->getURL() << endl;
            }
            document = docmap->append( c );
            
            fh_term term = 0;
            typedef map< termid_t, fh_term > termcache_t;
            termcache_t termcache;

            bool isCaseSensitive = this->isCaseSensitive();
            bool DropStopWords   = getDropStopWords();
            StemMode stemmer     = getStemMode();
            
//            LG_IDX_D << "Starting to parse tokens" << endl;
            
            while( !(s = di->getToken( iss )).empty() )
            {
//                LG_IDX_D << "got token:" << s << endl;

                if( !isCaseSensitive )
                {
                    s = foldcase( s );
//                    LG_IDX_D << "case folded:" << s << endl;
                }

                s = stem( s, stemmer );

                if( DropStopWords )
                {
                    if( getStopWords().count( s ) )
                    {
                        continue;
                    }
                }

                termid_t tid = lex->lookup( s );
                if( !tid )
                {
                    term = inv->insert();
                    tid  = term->getID();
                    lex->insert( s, tid );
                    termcache.insert( make_pair( tid, term ));
                    
                    LG_IDX_D << "adding stemmed term:" << s << " tid:" << tid << endl;
                }
                else
                {
                    termcache_t::iterator iter = termcache.find( tid );
                    if( iter != termcache.end() )
                        term = iter->second;
                    else
                    {
                        term = inv->getTerm( tid );
                        termcache.insert( make_pair( tid, term ));
                    }
                }
                
                if( !term->doesDocumentContain( document ))
                {
                    LG_IDX_D << "adding document number:" << document->getID()
                             << " to term:" << s << " tid:" << tid << endl;
                    term->insert( document, 1 );
                    document->incrInvertedReferenceCount();
                }
                else
                {
                    term->incrementCountForDocument( document );
                }
                

                streamsize bdone = di->getBytesCompleted();
                if( bdone % 16*1024 == 0 )
                {
                    di->getProgressSig().emit( c, bdone, totalBytes );
                }
            }

//            LG_IDX_D << "Done to parsing tokens" << endl;
            
            //
            // Calculate and store W(d), the weight of the document for
            // ranked queries.
            //
            LG_IDX_D << "DocumentIndexer::addContextToIndex() supports ranked:"
                     << supportsRankedQuery() << endl;
            if( supportsRankedQuery() )
            {
                long double sum   = 0;

                for( termcache_t::iterator iter = termcache.begin(); iter!=termcache.end(); ++iter )
                {
                    fh_term term  = iter->second;
                    double f_d_t  = term->getFreqOfTermInDocument( document );
                    double w_d_t  = 1 + log( f_d_t );
                    sum   += pow( w_d_t, 2 );
                }
#ifdef FERRIS_HAVE_SQRTL
                sum = sqrtl( sum );
#else
                LG_IDX_I << "no sqrtl() on this platform, response may be less than optimal (TM)" << endl;
                sum = sqrt( (double)sum );
#endif
                LG_IDX_D << "DocumentIndexer::addContextToIndex() document weight:" << sum << endl;
                document->setDocumentWeight( sum );
            }

            LG_IDX_D << "DocumentIndexer::addContextToIndex(end)" << endl;
        }
        
        docNumSet_t&
        FullTextIndexManagerNative::addAllDocumentsMatchingTerm( const std::string& token,
                                                                 docNumSet_t& output,
                                                                 int limit )
        {

            LG_IDX_D << "looking up token:" << token << endl;
            termid_t tid = m_lex->lookup( token );
            if( !tid )
            {
                LG_IDX_D << "token not found in lexicon:" << token << endl;
                return output;
            }
            
            fh_term term = m_inv->getTerm( tid );

            docNumSet_t tmp;
            term->getDocumentNumbers( tmp );
            output.insert( tmp.begin(), tmp.end() );
            return output;
        }
        
        std::string
        FullTextIndexManagerNative::resolveDocumentID( docid_t docid )
        {
            fh_doc d = getDocumentMap()->lookup( docid );
            fh_context ctx = d->getContext();
            return ctx->getURL();
        }
        
        

//         fh_database
//         FullTextIndexManagerNative::getConfigDB()
//         {
// //             LG_IDX_D << "FullTextIndexManagerNative::getConfigDB() m_config:" << toVoid(m_config)
// //                  << " m_configNotAvailable:" << m_configNotAvailable
// //                  << endl;
            
//             if( m_config || m_configNotAvailable )
//                 return m_config;

//             try
//             {
//                 m_config = new Database( CleanupURL(getBasePath() + DB_FULLTEXT) );
//             }
//             catch( exception& e )
//             {
//                 LG_IDX_D << "FullTextIndexManagerNative::getConfigDB() e:" << e.what()
//                      << " path:" << CleanupURL(getBasePath() + DB_FULLTEXT)
//                      << " 2xpath:" << CleanupURL(CleanupURL(getBasePath() + DB_FULLTEXT))
//                      << endl;
//                 m_configNotAvailable = true;
//             }
//             return m_config;
//         }
        

//         std::string
//         FullTextIndexManagerNative::getConfig( const std::string& k, const std::string& def, bool throw_for_errors )
//         {
//             getConfigDB();
            
//             if( m_configNotAvailable && !throw_for_errors )
//                 return def;
            
//             return get_db4_string( getConfigDB(), k, def, throw_for_errors );
//         }
        
//         void
//         FullTextIndexManagerNative::setConfig( const std::string& k, const std::string& v )
//         {
//             set_db4_string( getConfigDB(), k, v );
//             getConfigDB()->sync();
//         }

        int
        FullTextIndexManagerNative::getLexiconFileVersion()
        {
            return toint(getConfig( IDXMGR_LEXICON_VERSION_K, tostr(IDXMGR_LEXICON_VERSION_CURRENT) ));
        }


        int
        FullTextIndexManagerNative::getInvertedFileVersion()
        {
            return toint(getConfig( IDXMGR_INVERTEDFILE_VERSION_K, tostr(IDXMGR_INVERTEDFILE_VERSION_CURRENT) ));
        }


        int
        FullTextIndexManagerNative::getDocumentMapFileVersion()
        {
            return toint(getConfig( IDXMGR_DOCUMENTMAP_VERSION_K, tostr(IDXMGR_DOCUMENTMAP_VERSION_CURRENT) ));
        }



        unsigned long
        FullTextIndexManagerNative::getInvertedSkiplistMaxSize()
        {
            unsigned long ret = toType<unsigned long>(
                getConfig( IDXMGR_MAXPTRS_PER_SKIPLISTCHUNK_K, IDXMGR_MAXPTRS_PER_SKIPLISTCHUNK_DEFAULT ));
            return ret;
        }

        void
        FullTextIndexManagerNative::setInvertedSkiplistMaxSize( int v )
        {
            setConfig( IDXMGR_MAXPTRS_PER_SKIPLISTCHUNK_K, tostr(v) );
        }

        
        std::string
        FullTextIndexManagerNative::getDocumentNumberGapCode()
        {
            return getConfig( IDXMGR_DGAP_CODE_K, IDXMGR_DGAP_CODE_DEFAULT );
        }
        

        void
        FullTextIndexManagerNative::setDocumentNumberGapCode( const std::string& codename )
        {
            setConfig( IDXMGR_DGAP_CODE_K, codename );
        }


        std::string
        FullTextIndexManagerNative::getFrequencyOfTermInDocumentCode()
        {
            return getConfig( IDXMGR_FDT_CODE_K, IDXMGR_FDT_CODE_DEFAULT );
        }

        std::string
        FullTextIndexManagerNative::getLexiconClassName()
        {
            return getConfig( IDXMGR_LEXICON_CLASS_K, IDXMGR_LEXICON_CLASS_DEFAULT );
        }


        
        void
        FullTextIndexManagerNative::setFrequencyOfTermInDocumentCode( const std::string& codename )
        {
            setConfig( IDXMGR_FDT_CODE_K, codename );
        }
        


        
//         bool
//         FullTextIndexManagerNative::getDropStopWords()
//         {
//             return isTrue( getConfig( IDXMGR_DROPSTOPWORDS_CLASS_K, "0" ));
//         }

//         bool
//         FullTextIndexManagerNative::isCaseSensitive()
//         {
//             return isTrue( getConfig( IDXMGR_CASESEN_CLASS_K, "0" ));
//         }
        

//         StemMode
//         FullTextIndexManagerNative::getStemMode()
//         {
//             return StemMode(
//                 toType<int>(
//                     getConfig( IDXMGR_STEMMER_CLASS_K, tostr(STEM_J_B_LOVINS_68) )));
//         }

        bool
        FullTextIndexManagerNative::supportsRankedQuery()
        {
            return isTrue(getConfig( IDXMGR_SUPPORTS_RANKED_K, "1" ));
        }

        fh_lexicon
        FullTextIndexManagerNative::getLexicon()
        {
            return m_lex;
        }

        fh_invertedfile             
        FullTextIndexManagerNative::getInvertedFile()
        {
            return m_inv;
        }

        fh_docmap
        FullTextIndexManagerNative::getDocumentMap()
        {
            return m_docmap;
        }

        string
        FullTextIndexManagerNative::getBasePath()
        {
            return getPath();
        }

        void
        FullTextIndexManagerNative::sync()
        {
            m_lex->sync();
            m_inv->sync();
            m_docmap->sync();
        }

        void
        FullTextIndexManagerNative::executeRankedQuery( fh_context selection,
                                                        string query_string,
                                                        int    m_accumulatorsMaxSize,
                                                        int    m_resultSetMaxSize )
        {
            fh_lexicon      lex  = getLexicon();
            fh_invertedfile inv  = getInvertedFile();
//            fh_docindexer   docs = Factory::makeDocumentIndexer( idx );


            if( query_string.empty() )
            {
                return;
            }
            
            LG_IDX_D << "ExecuteRankedQuery(top) query:" << query_string << endl;
            
            //
            // This follows the page 202 outline from Managing Gigabytes book
            //
            //
            docNumSet_t docnums;
            typedef map< docid_t, documentWeight_t > Accumulators_t;
            Accumulators_t Accumulators;
            int AccumulatorsSize = 0;
            fh_stringstream query_ss;
            query_ss << query_string;
            string token;
            while( getline( query_ss, token, ' ' ))
            {
                LG_IDX_D << "ExecuteRankedQuery() token:" << token << endl;
                token = stem( token, getStemMode() );
                termid_t tid = lex->lookup( token );
                if( !tid )
                {
                    LG_IDX_D << "token not found in lexicon:" << token << endl;
                    continue;
                }

                fh_term term         = inv->getTerm( tid );
                documentWeight_t   N = inv->getNumberOfTerms();
                doctermfreq_t    f_t = term->getFreqOfTerm();
                documentWeight_t w_t = term->getWeight();

                LG_IDX_D << "ExecuteRankedQuery() token:" << token
                         << " term:" << term->getID()
                         << " N:" << N
                         << " f_t:" << f_t
                         << " w_t:" << w_t
                         << endl
                         << " Accumulators.sz:" << Accumulators.size()
                         << " AccumulatorsSize:" << AccumulatorsSize
                         << " m_accumulatorsMaxSize:" << m_accumulatorsMaxSize
                         << endl;

                //
                // take each document matching into account in the Accumulators 
                //
                term->readAllChunks();
                
                for( Term::m_chunks_t::iterator ci = term->m_chunks.begin();
                     ci != term->m_chunks.end(); ++ci )
                {
                    for( SkippedListChunk::m_hits_t::iterator iter = (*ci)->m_hits.begin();
                         iter != (*ci)->m_hits.end(); ++iter )
                    {
                        docid_t    docid  = iter->first;
                        doctermfreq_t    f_d_t  = iter->second;
                        documentWeight_t accu   = 0;
                    
                        Accumulators_t::iterator ai = Accumulators.find( docid );
                        bool alreadyInCollection = ai != Accumulators.end();

                        if( alreadyInCollection )
                            accu = ai->second;
                        accu += log( 1 + f_d_t ) * w_t;
                        Accumulators[ docid ] = accu;
                    }
                }
            }


            LG_IDX_D << "ExecuteRankedQuery(t) Accumulators before taking w_d into scale" << endl;
            for( Accumulators_t::iterator ai = Accumulators.begin(); ai!=Accumulators.end(); ++ai )
            {
                LG_IDX_D << " docid:" << ai->first << " weight:" << ai->second << endl;
            }
            LG_IDX_D << "ExecuteRankedQuery(e) Accumulators before taking w_d into scale" << endl;
            
                
            //
            // scale accumulated values to each w_d, cache the max collection of values
            // into topRankedDocuments. Note that the lowest value in topRankedDocuments
            // can be found easily and an iterator to it is cached.
            //
            typedef map< documentWeight_t, docid_t > TopRankedDocuments_t;
            TopRankedDocuments_t TopRankedDocuments;
            TopRankedDocuments_t::iterator LowestTopRankedDocument = TopRankedDocuments.end();
            bool FillingTopRankedDocuments = true;
            
            for( Accumulators_t::iterator ai = Accumulators.begin(); ai!=Accumulators.end(); ++ai )
            {
                fh_doc doc = getDocumentMap()->lookup( ai->first );
                documentWeight_t w_d = doc->getDocumentWeight();
                documentWeight_t scaled_weight = ai->second / w_d;

                if( FillingTopRankedDocuments )
                {
                    TopRankedDocuments.insert( make_pair( scaled_weight, ai->first ));
                    if( TopRankedDocuments.size() == m_resultSetMaxSize )
                    {
                        FillingTopRankedDocuments = false;
                        LowestTopRankedDocument = TopRankedDocuments.begin();
                    }
                }
                else
                {
                    if( LowestTopRankedDocument->first < scaled_weight )
                    {
                        LG_IDX_D << "bumping out a document. first:" << LowestTopRankedDocument->first
                                 << " scaled weight:" << scaled_weight
                                 << endl;
                        TopRankedDocuments.erase( LowestTopRankedDocument );
                        TopRankedDocuments.insert( make_pair( scaled_weight, ai->first ));
                        LowestTopRankedDocument = TopRankedDocuments.begin();
                    }
                }
            }

            
            LG_IDX_D << "ExecuteRankedQuery(t) TopRankedDocuments before taking w_d into scale" << endl;
            for( TopRankedDocuments_t::iterator ri = TopRankedDocuments.begin();
                 ri != TopRankedDocuments.end(); ++ri )
            {
                LG_IDX_D << " docid:" << ri->second << " weight:" << ri->first << endl;
            }
            LG_IDX_D << "ExecuteRankedQuery(e) TopRankedDocuments before taking w_d into scale" << endl;

            //
            // Lookup the topRankedDocuments and make filesystem
            //
            selection->addAttribute( "rank",   "0", FXD_FTX_RANK, true );
            selection->addAttribute( "filter",
                                     makeFullTextQueryFilterString( query_string,
                                                                    QUERYMODE_RANKED,
                                                                    this ),
                                     FXD_FFILTER,
                                     true );
            for( TopRankedDocuments_t::iterator ri = TopRankedDocuments.begin();
                 ri != TopRankedDocuments.end(); ++ri )
            {
                documentWeight_t w = ri->first;
                fh_doc d = getDocumentMap()->lookup( ri->second );
                LG_IDX_D << "  matching URL:" << d->getURL() << endl;
                fh_context ctx = d->getContext();
                selection->createSubContext( "", ctx );
                ctx->addAttribute( "rank", toString(w), FXD_FTX_RANK, true );
            }
            
            
        }
        
        
        
    };
};



extern "C"
{
    FERRISEXP_API Ferris::FullTextIndex::MetaFullTextIndexerInterface* Create()
    {
        return new Ferris::FullTextIndex::FullTextIndexManagerNative();
    }
};
