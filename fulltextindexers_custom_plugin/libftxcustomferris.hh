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

    $Id: libftxcustomferris.hh,v 1.8 2010/09/24 21:31:31 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_FULLTEXTIDX_CUSTOM_PLUGIN_H_
#define _ALREADY_INCLUDED_FERRIS_FULLTEXTIDX_CUSTOM_PLUGIN_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <sigc++/sigc++.h>

#include <Ferris/TypeDecl.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/Cache.hh>
#include <Ferris/FullTextIndexerMetaInterface.hh>

#include <string>
#include <list>
#include <map>
#include "Ferris/FerrisStdHashMap.hh"
#include <set>

#include <STLdb4/stldb4.hh>

#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility push(default)
#endif
namespace Ferris
{

    namespace EAIndex 
    {
        class EAIndexManagerDB4;
    };
    namespace AI
    {
        class SvmLight_BinaryClassifierAgentImplemenation;
    };

    /**
     * Support for full text indexing and querys
     */
    namespace FullTextIndex 
    {
        using namespace ::STLdb4;
        class Lexitest;

        class Document;
        FERRIS_SMARTPTR( Document, fh_doc );

        class DocumentMap;
        FERRIS_SMARTPTR( DocumentMap, fh_docmap );
        
        class Term;
        FERRIS_SMARTPTR( Term, fh_term );

        class SkippedListChunk;
        FERRIS_SMARTPTR( SkippedListChunk, fh_skiplist );
        
        class InvertedFile;
        FERRIS_SMARTPTR( InvertedFile, fh_invertedfile );

        class FullTextIndexManagerNative;
        FERRIS_SMARTPTR( FullTextIndexManagerNative, fh_nidx );
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

//        typedef guint32 termid_t;
        typedef guint32 docidInvertedRef_t; //< number of active inverted lists this docid is in
        typedef guint16 doctermfreq_t;
        typedef guint16 skippedchunksize_t;
        typedef double documentWeight_t;
        typedef guint16 SkippedListChunkCount_t;
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        /**
         * Mapping from numerical document number to fh_context.
         *
         * WARNING: if the size of the header of payload for an item changes
         * then DocumentMap::getDB() will need changing for secondary index URL offset calculation.
         * Currently this size depends on m_weight, m_documentIndexedWithMTime,
         * m_invertedReferenceCount and includes an 8 bit boolean area.
         *
         * Note that the m_invertedReferenceCount and m_isRevokedDocument are updated
         * by the code that adds files to the index. This is because there are many
         * classes that manage inverted lists but only two ways to add an item to
         * any index, FullTextIndex::DocumentIndexer and EAIndex::DocumentIndexer.
         *
         * Inverted list subclasses are responsible for purgeing revoked documentIDs
         * though. If isRevoked() and the doc appears in an inverted list
         * then the inverted list class should try to remove the docid, maybe during a
         * sync() and when removed from that list it should decrInvertedReferenceCount()
         * the document. Note that the perfered method of testing if a docid has been revoked
         * is using DocumentMap::isRevoked() which may maintain a hash for greater speed.
         */
        class FERRISEXP_API Document
            :
            public Handlable
        {
            friend class DocumentMap;

            typedef time_t              m_documentIndexedWithMTime_t;
            typedef docidInvertedRef_t  m_invertedReferenceCount_t;
            typedef documentWeight_t    m_weight_t;
            typedef guint8 m_savedBitfieldSize_t; //< size that m_isRevokedDocument and others take in all

            bool                m_dirty;
            fh_docmap           m_docmap;
            std::string         m_url;
            docid_t       m_id;
            bool                m_isRevokedDocument;        //< true if this docid has been removed from sys
            m_weight_t          m_weight;
            m_invertedReferenceCount_t   m_invertedReferenceCount;   //< number of inverted lists we are in
            m_documentIndexedWithMTime_t m_documentIndexedWithMTime; //< What mtime was when doc was added
            bool                m_deleted;  //< when we are revoked with invrc==0 this is set

            enum {
                E_ISREVOKED_SHIFT = 1
            };
            
            fh_database getDB();
            
            Document( fh_docmap docmap, fh_context c );
            Document( fh_docmap docmap, docid_t n );

            void save();
            
        public:

            virtual ~Document();
            void sync();
            
            docid_t getID();
            std::string getURL();
            fh_context getContext();

            void setDocumentWeight( documentWeight_t w );
            documentWeight_t getDocumentWeight();

            /**
             * When a document is added to an inverted list
             * incrInvertedReferenceCount() should be called.
             * call decrInvertedReferenceCount() when the document is
             * removed from an inverted list.
             */
            void incrInvertedReferenceCount();
            /**
             * call decrInvertedReferenceCount() when the document is
             * removed from an inverted list.
             * @returns true if this was the last reference to the document.
             */
            bool decrInvertedReferenceCount();
            docidInvertedRef_t getInvertedReferenceCount();

            /**
             * When a docuemnt is to be removed from the index call this
             * for the old document. This method can not be undone, the
             * document will have to be added to the index again with
             * a new docid.
             */
            void revokeDocument();
            bool isRevoked();

            /**
             * Update the mtime when doc was indexed to the current
             * value of mtime for the fh_context we refer to.
             */
            void freshenMTime();
            time_t getMTime();
        };

        class FERRISEXP_API DocumentMap
            :
            public Handlable
        {
            friend class Document;
            friend class FullTextIndexManagerNative;
            friend class ::Ferris::EAIndex::EAIndexManagerDB4;
            friend class ::Ferris::EAIndex::EAIndexManagerDB4Tree;
            DocumentMap( fh_nidx idx, fh_env dbenv, PathManager* path_mgr = 0, bool useSecondaryIndex = true );
            fh_nidx m_idx;
            PathManager* m_path_mgr;

            fh_env      m_dbenv;
            fh_database m_db;

            fh_database m_secdb;      //< if(m_useSecondaryIndex) handle for secondary index 
            bool m_useSecondaryIndex; //< create a secondary index for help determining if a doc is indexed

            typedef FERRIS_STD_HASH_MAP< docid_t, fh_doc > m_documents_t;
            m_documents_t m_documents;


            // A list of all revoked docids for this docmap. Note that this will
            // always be current but m_documents may not be fully loaded.
            typedef std::set< docid_t > m_revoked_t;
            m_revoked_t m_revoked; 
            
            
            /**
             * When a docid has been revoked and its inverted reference count
             * drops to zero it should call here to make its death complete.
             * @see addToRevokedIDCache()
             */
            void erase( fh_doc d );

            /**
             * When a document is revoked with document::revokeDocument() it will
             * call here so that the map itself can quickly test a docid to see
             * if its revoked or not. When a document has no more inverted references
             * then it will call erase() and the docmap can drop its ID from the cache
             * of revoked documents.
             *
             * @see erase()
             */
            void addToRevokedIDCache( docid_t id );

            int getSecondaryDBOffset();
            Database::sec_idx_callback getSecondaryDBCallback();

        protected:
            
            
        public:

            virtual ~DocumentMap();

            fh_database getDB();
            fh_doc append( fh_context c );
            fh_doc lookup( docid_t id );

            typedef Loki::Functor< void, LOKI_TYPELIST_2( const std::string&, docid_t ) >
            ForEachDocumentFunctor_t;
            void for_each( ForEachDocumentFunctor_t f );
            
            /**
             * Perform a quick test to see if this document is revoked or not
             */
            bool isRevoked( docid_t id );

            /**
             * When you add a context to the index (fulltext or ea) we
             * need to see if the document is already in the index and
             * if so we should return the existing document.
             *
             * This call may be slow on files that don't have a reverse
             * lookup stored.
             */
            fh_doc lookupByURL( const std::string& earl );
            fh_doc lookup( fh_context c );

            /**
             * Save all data to disk
             */
            void sync();

            /**
             * Dump the doucment table in the form
             * docid, url
             */
            void dumpTo( fh_ostream oss, bool asXML = true );

            /**
             * Get the number of documents in the map
             */
            guint32 size();

            /********************************************************************************/
            /********************************************************************************/
            /********************************************************************************/
            /**** the following are mainly for query interfaces and stats, not regular use **/
            /********************************************************************************/
            /********************************************************************************/

            /**
             * Get the cache of which document IDs are currently revoked.
             */
            std::set< docid_t > getRevokedDocumentIDs();
            
        };
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/


        /**
         * Each inverted list is split up into SkippedListChunks.
         *
         * Each chunk contains a small header to allow quick skipping of chunks
         * that are not going to be relavent.
         * The header format is:
         * firstdocidinblock    as sizeof(docid_t)            bytes
         *  lastdocidinblock    as sizeof(docid_t)            bytes
         * numberofdocs         as sizeof(skippedchunksize_t) bytes
         * lengthofchunk        as sizeof(skippedchunksize_t) bytes
         *
         * note that lengthofchunk doesn't include the bytes of the header itself.
         * ie. lengthofchunk is the number of bytes from after the header until
         * the next start of chunk.
         *
         * numberofdocs is the number of documents in this chunk including the two
         * document numbers in the header
         *
         * The payload of the saved chunk is in the following format:
         *
         * docnum for 2nd to 2nd last documents 
         * d(f,t) for first  document
         * d(f,t) for second document
         * d(f,t) for ...
         * d(f,t) for last document
         *
         * note that the payload will be stored possibly compressed in the above
         * format.
         *
         * note that for a chunk without any elements, if such a chunk is saved
         * then the header is intact and
         * firstdocidinblock = lastdocidinblock = lengthofchunk = 0;
         */
        class FERRISEXP_API SkippedListChunk
            :
            public Handlable
        {
            friend class RankedFullTextQuery;
            friend class FullTextIndexManagerNative;

            //
            // If we keep a smart pointer here then the term won't die because
            // we hold a reference
            Term*           m_term;
//            fh_term         m_term;

            /**
             * list of documents in this chunk and the freq of m_term in them
             */
            typedef std::map< docid_t, doctermfreq_t > m_hits_t;
            m_hits_t m_hits;
            m_hits_t m_revoked_hits;
            
            Term*           getTerm();
            fh_invertedfile getInvertedFile();
            fh_nidx         getIndex();
            fh_database     getDB();

            /**
             * Get unique ID for this term. This is the same ID that the lexicon
             * maps words to
             */
            termid_t getID();

            /**
             * We taint the fh_term itself when we need to be saved so that all
             * the skipped lists are saved
             */
            void setDirty( bool v );

            /**
             * For seeking, this is the fixed header size of the chunk
             */
            static std::streamsize getHeaderSize();

            /**
             * At the end of a load() operation this method is called
             * which moved revoked documents that are still in the on disk
             * inverted list from m_hits into m_revoked_hits. Note that
             * save() calls attemptToPurgeRevoked() which uses m_revoked_hits
             * as the collection of docids that should have a reference count
             * drop.
             */
            void moveRevokedToMRevokedHits();
            
            /**
             * anytime after a load() is done the revoked documentIDs can
             * be purged from this chunk. By purging we may become dirty
             * because we have lost some docids from the chunk.
             */
            void attemptToPurgeRevoked();
            
        public:

            SkippedListChunk( Term* t = 0 );
            virtual ~SkippedListChunk();

            /**
             * return true if the item can fit into this chunk
             */
            bool canFit( fh_doc d, int freq );

            /**
             * Add document to inverted index
             * returns false if the item can not fit.
             */
            bool insert( fh_doc d, int freq );
            
            /**
             * Increment the number of times this term occurs in
             * the given document
             */
            void incrementCountForDocument( fh_doc d );
            
            /**
             * Check if the given document contains this term
             */
            bool doesDocumentContain( fh_doc d );

            /**
             * Get the frequency of the term in the given document
             * if ranked queries are not enabled for this index then
             * the result is boolean
             */
            doctermfreq_t getFreqOfTermInDocument( fh_doc d );

            /**
             * Dump this term to the given stream in the form
             * termid number-of-doc-matches { docid, freq, docid, freq ... }
             */
            void dumpTo( fh_ostream oss, bool asXML = true );

            /**
             * get a set of all the document numbers that contain this term
             * note that the return value is the same as z for speed.
             */
            docNumSet_t& getDocumentNumbers( docNumSet_t& z );

            /**
             * Loading and saving of chunks. It is assumed that fh_term knows
             * about the header for each chunk and can skip chunks. The stream
             * param is assumed to be at the currect place for read/write to begin.
             */
            static fh_skiplist load( fh_term, fh_istream ss );
            void               save( fh_ostream ss, termid_t tid = 0 );

            /**
             * Check if the chunk currently pointed to by 'ss' contains the
             * desired term. If it does, leave the stream at the start of block
             * and return true, otherwise skip past the current chunk to the
             * start of the next chunk
             */
            static bool doesChunkContainDocument( fh_istream ss, docid_t d );


            /**
             * A slow method mainly for comparison of the on disk formats and the
             * code methods for storing document numbers
             */
            int getNumberOfBytesToStoreDocumentIDs();

            /**
             * A slow method mainly for comparison of the on disk formats and the
             * code methods for storing f(d,t) numbers
             */
            int getNumberOfBytesToStoreFDTs();
            
        };

        /********************************************************************************/
        /********************************************************************************/
        
        
        /**
         * Term in the index.
         *
         * Terms are saved in the following format:
         * TermFreq               as sizeof( doctermfreq_t ) bytes
         * SkippedListChunkCount  as sizeof( SkippedListChunkCount_t ) bytes
         * SkippedListChunk1
         * SkippedListChunk2
         * SkippedListChunkn
         *
         */
        class FERRISEXP_API Term
            :
            public CacheHandlable
        {
            friend class InvertedFile;
            friend class RankedFullTextQuery;
            friend class FullTextIndexManagerNative;

//             typedef std::pair< int, int > DocFreq;
//             typedef std::list< DocFreq > m_hits_t;
//             typedef std::map< docid_t, doctermfreq_t > m_hits_t;
//             m_hits_t m_hits;

            /**
             * All the skiplist chunks that we have read.
             */
            typedef std::list< fh_skiplist > m_chunks_t;
            m_chunks_t m_chunks;

            doctermfreq_t           m_termFreq;         //< Number of document pointers in total for this term
            SkippedListChunkCount_t m_onDiskChunkCount; //< Number of chunks on disk for the skipped list

            termid_t        m_id;    //< ID for this term (same as ID in lexicon for term)
            bool            m_dirty; //< has there been changes since read
            bool            m_readAllChunks; //< set to true once all chunks have been read
            fh_invertedfile m_inv;   //< inverted file we are stored in
            
            Term( fh_invertedfile inv, termid_t id = 0, bool created = false );
            void setID( termid_t id );

            /**
             * pass in DB_APPEND to add a new item
             */
            void save( int put_flags );

            /**
             * make sure that all the chunks in the inverted list are read, handy
             * for if you are about to store the chunks again. Note that this method
             * will clear currently loaded Skippedlists so it should be called before
             * modifications begin. returns true for success.
             */
            bool readAllChunks();

            /**
             * convert the Term into a chunk of data. Note that the m_id is left out
             * because it is assumed to be the key 
             */
            fh_ostream toStream( fh_ostream oss );

            /**
             * Used to create a new chunk, append it and update counters properly and
             * return a smartptr to the chunk
             */
            fh_skiplist createNewChunk();

            /**
             * Gets the last chunk or if there are no chunks creates a new one and
             * returns it.
             */
            fh_skiplist getLastChunkOrNewChunk();

            /**
             * get the skiplist that should contain the given docid. Note that this method
             * might not load all the skiplistchunks if they are not all needed.
             */
            fh_skiplist getSkipListForDocument( fh_doc d );

            /**
             * Loads the payload of data for this term into the given stringstream
             * @returns true on success
             */
            bool readPayloadFromDatabase( fh_stringstream ss );
            
            
        public:

            virtual ~Term();

            void sync();

            /**
             * Add document to inverted index
             * Note that insert() currently assumes that the document ID always gets larger.
             */
            void insert( fh_doc d, int freq );

            /**
             * Increment the number of times this term occurs in
             * the given document
             */
            void incrementCountForDocument( fh_doc d );
            
            /**
             * Check if the given document contains this term
             */
            bool doesDocumentContain( fh_doc d );

            /**
             * Get the frequency of the term in the given document
             * if ranked queries are not enabled for this index then
             * the result is boolean
             */
            doctermfreq_t getFreqOfTermInDocument( fh_doc d );


            /**
             * Get the number of documents that contain this term
             */
            doctermfreq_t getFreqOfTerm();

            /**
             * get the weight of this term.
             * weight is calculated as
             * 1 + log( total_terms_in_lexicon / freq_of_this_term )
             */
            documentWeight_t getWeight();
            
            /**
             * Get unique ID for this term. This is the same ID that the lexicon
             * maps words to
             */
            termid_t getID();

            void save();
            void append();
            static fh_term load( fh_invertedfile inv, termid_t tid );

            /**
             * Dump this term to the given stream in the form
             * termid number-of-doc-matches { docid, freq, docid, freq ... }
             */
            void dumpTo( fh_ostream oss, bool asXML = true, bool includeDiskSizes = true );

            /**
             * get a set of all the document numbers that contain this term
             * note that the return value is the same as z for speed.
             */
            docNumSet_t& getDocumentNumbers( docNumSet_t& z );

            /**
             * Marks the term for disk write. Note that this might leed to loading
             * the entire inverted list so that new data can be written properly.
             */
            void setDirty( bool v = true );

            /**
             * get the inverted file that this term is stored in.
             */
            fh_invertedfile getInvertedFile();


            /**
             * A slow method mainly for comparison of the on disk formats and the
             * code methods for storing document numbers
             */
            int getNumberOfBytesToStoreDocumentIDs();

            /**
             * A slow method mainly for comparison of the on disk formats and the
             * code methods for storing f(d,t) numbers
             */
            int getNumberOfBytesToStoreFDTs();
            
        };
        
        /********************************************************************************/
        /********************************************************************************/

        
        /********************************************************************************/
        /********************************************************************************/
        
        
        /**
         * Inverted index file. Contains mapping
         * termid -> { d, f(t,d ) , ... }
         *
         * 
         */
        class FERRISEXP_API InvertedFile
            :
            public Handlable
        {
            friend class FullTextIndexManagerNative;

            InvertedFile( fh_nidx idx, fh_env dbenv );
            
            fh_env       m_dbenv;
            fh_database  m_db;
            fh_nidx      m_idx;

//             typedef Cache< termid_t, fh_term > m_term_cache_t;
//             m_term_cache_t m_term_cache;

            /**
             * other methods in the class should call here to get a
             * term. This will either load it or return a reference
             * to a cached object.
             */
            fh_term getCachedTerm( termid_t tid );
            
        public:

            virtual ~InvertedFile();

            fh_term getTerm( termid_t );

            fh_term insert();

            /**
             * get the db4 file that this inverted file is stored in
             */
            fh_database getDB();

            /**
             * get the number of terms that are in the inverted file.
             */
            termid_t getNumberOfTerms();

            /**
             * Dump out the inderted file (aka index) in the form
             * termid { docid, freq, docid, freq ... }
             * to the given stream.
             */
            void dumpTo( fh_ostream oss, bool asXML = true );


            /**
             * Save all data to disk
             */
            void sync();

            /**
             * attempt to compact the index by dropping out revoked
             * documents etc.
             */
            void compact( fh_ostream oss, bool verbose = false );
            

            /**
             * get the index that this inverted file belongs to
             */
            fh_nidx getIndex();
        };
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        class FERRISEXP_API FullTextIndexManagerNative
            :
            public MetaFullTextIndexerInterface,
            public PathManager
        {
//             friend fh_idx Factory::getDefaultFullTextIndex();
//             friend fh_idx Factory::getFullTextIndex( const std::string& );

            
//             /**
//              * Used in the creation of a new index
//              */
//             FullTextIndexManagerNative( const std::string& basepath,
//                                         bool caseSensitive,
//                                         bool dropStopWords,
//                                         StemMode stemMode,
//                                         const std::string& lex_class );

//             friend fh_idx createFullTextIndexNative( fh_context c,
//                                                      bool caseSensitive,
//                                                      bool dropStopWords,
//                                                      StemMode stemMode,
//                                                      const std::string& lex_class );

        protected:

            virtual void CreateIndex( fh_context c,
                                      bool caseSensitive,
                                      bool dropStopWords,
                                      StemMode stemMode,
                                      const std::string& lex_class,
                                      fh_context md );
            virtual void CommonConstruction();

//            std::string     m_basepath;
            fh_lexicon      m_lex;
            fh_invertedfile m_inv;
            fh_docmap       m_docmap;
//             fh_database     m_config;
//             bool            m_configNotAvailable;
//            fh_database     getConfigDB();
            
//             /**
//              * Ensure that there is a db4 file there for setConfig() / getConfig() to use
//              */
//             void ensureConfigFileCreated();

        protected:

            virtual void sync();
            
        public:

            FullTextIndexManagerNative();
            virtual ~FullTextIndexManagerNative();

            virtual std::string getConfig( const std::string& k, const std::string& def,
                                           bool throw_for_errors = false )
                {
                    return MetaFullTextIndexerInterface::getConfig( k, def, throw_for_errors );
                }
            virtual void setConfig( const std::string& k, const std::string& v )
                {
                    MetaFullTextIndexerInterface::setConfig( k, v );
                }
            
            virtual void addToIndex( fh_context c, fh_docindexer di );
            virtual docNumSet_t& addAllDocumentsMatchingTerm(
                const std::string& term,
                docNumSet_t& output,
                int limit );
            virtual std::string resolveDocumentID( docid_t );


            fh_lexicon      getLexicon();
            fh_invertedfile getInvertedFile();
            fh_docmap       getDocumentMap();
            virtual std::string getBasePath();

//             std::string getConfig( const std::string& k, const std::string& def,
//                                    bool throw_for_errors = false );
//             void setConfig( const std::string& k, const std::string& v );


//             /**
//              * should stop words be dropped in this index
//              */
//             bool getDropStopWords();

//             /**
//              * should case be perserved. if not then case folding should be
//              * done to lower case.
//              */
//             bool isCaseSensitive();

//             /**
//              * get the stemming algo that should be used to trim new items in
//              * the lexicon
//              */
//             StemMode getStemMode();

            /**
             * Indexes may be created that only support boolean queries.
             */
            virtual bool supportsRankedQuery();

            /**
             * Max number of doc,f(d,t) pointers to a skiplist before a new one is made
             */
            unsigned long getInvertedSkiplistMaxSize();

            /**
             * get the name of the code to use on the document numbers in the inverted
             * file.
             */
            std::string getDocumentNumberGapCode();

            /**
             * get the name of the code to use on the f(d,t) part of the document pointers
             * in the inverted file.
             */
            std::string getFrequencyOfTermInDocumentCode();

            /**
             * name of the lexicon class handling IO for this index's lexicon
             */
            std::string getLexiconClassName();

            /**
             * Each of the lexicon, inverted file and document map are branded with a version
             * to identify what generation they are. If there are incompatible changes in the
             * format then a higher generation is saved for the new file when it is converted.
             * This gets the lexicon's generation.
             */
            int getLexiconFileVersion();

            /**
             * Each of the lexicon, inverted file and document map are branded with a version
             * to identify what generation they are. If there are incompatible changes in the
             * format then a higher generation is saved for the new file when it is converted.
             * This gets the invertedfile's generation.
             */
            int getInvertedFileVersion();

            /**
             * Each of the lexicon, inverted file and document map are branded with a version
             * to identify what generation they are. If there are incompatible changes in the
             * format then a higher generation is saved for the new file when it is converted.
             * This gets the document map's generation.
             */
            int getDocumentMapFileVersion();
            
            /********************************************************************************/
            /********************************************************************************/
            /*** The following are only valid to set before anything is in the index ********/
            /********************************************************************************/
            /********************************************************************************/
            
            /**
             * Set the max length of the inverted skip lists, can only be set
             * before the index is used.
             */
            void setInvertedSkiplistMaxSize( int v );

            /**
             * set the name of the code to use on the document numbers in the inverted
             * file.
             */
            void setDocumentNumberGapCode( const std::string& codename );

            /**
             * set the name of the code to use on the f(d,t) part of the document pointers
             * in the inverted file.
             */
            void setFrequencyOfTermInDocumentCode( const std::string& codename );

        public:

            /**
             * Called when the index is about to be added to.
             */
            virtual void prepareForInsertions()
                {
                    getLexicon()->prepareForInsertions();
                }
            
            /**
             * BRIDGE METHOD. returns true if this is a ferriscustom index.
             */
            virtual bool isCustomFerrisIndex()
                {
                    return true;
                }

            virtual PathManager* tryToCastToPathManager()
                {
                    return this;
                }

            virtual void executeRankedQuery( fh_context selection,
                                             std::string query_string,
                                             int    m_accumulatorsMaxSize,
                                             int    m_resultSetMaxSize );
            
            
            
        public: // internal use only
            static MetaFullTextIndexerInterface* Create();
        };
        
    };
};
#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility pop
#endif
#endif
