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

    $Id: libeaidxcustomferris.hh,v 1.11 2010/09/24 21:31:23 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "fulltextindexers_custom_plugin/libftxcustomferris.hh"

namespace Ferris
{
    namespace EAIndex 
    {
        using namespace ::Ferris::FullTextIndex;
        using namespace ::STLdb4;

        
        typedef termid_t aid_t;
        typedef termid_t vid_t;
        typedef docid_t  urlid_t;
//        typedef std::set< urlid_t > docNumSet_t;
        typedef gint64 urllist_id_t;

        
        class InvertedFile;
        FERRIS_SMARTPTR( InvertedFile, fh_invertedfile );

        class URLList;
        FERRIS_SMARTPTR( URLList, fh_urllist );

        typedef std::list< fh_urllist > urllists_t;
        
        struct invertedfile_dbiter_compare;

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        class FERRISEXP_API URLList
            :
            public CacheHandlable
        {
            fh_invertedfile m_inv;
            aid_t           m_aid;
            std::string     m_value;
            bool            m_dirty; //< has there been changes since read

            /**
             * list of documents in this chunk and the scid of the aid,vid,url attribute
             */
//            typedef std::map< docid_t, XSDBasic_ > m_hits_t;
            typedef std::map< docid_t, int > m_hits_t;
            m_hits_t m_hits;
            m_hits_t m_revoked_hits;

            friend class InvertedFile;
            friend struct invertedfile_dbiter_compare;
            friend FERRISEXP_DLLLOCAL int bt_compare_aidvid_cs(DB* rawdb, const DBT* a, const DBT* b );

            
            /**
             * get the database we are storing in
             */
            fh_database getDB();

            /**
             * convert the key matter aid,vid into a string
             */
            std::string makeKey();

            /**
             * convert a key made from makeKey() into the aid,vid combo
             */
            static
            std::pair< aid_t, std::string >
            readKey( const std::string& s );

            fh_db4idx getIndex();

            

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
            URLList( fh_invertedfile inv,
                     Database::iterator dbi,
                     aid_t m_aid,
                     const std::string& value,
                     bool createIfNotThere = true );
            URLList( fh_invertedfile inv,
                     aid_t m_aid,
                     const std::string& value,
                     bool createIfNotThere = true );
            virtual ~URLList();
            
            /**
             * Make sure the url with ID urlid is in the list
             */
            void insert( urlid_t urlid, XSDBasic_t scid );

            /**
             * does the list contain the given URL id
             */
            bool contains( urlid_t urlid );

            /**
             * If there are changes then flush them to disk
             */
            void sync();
            
            void save();

            
            /**
             * Must have valid data for makeKey(). only loads the document ID values.
             */
            void load_private( fh_stringstream& ss );
            void load( bool createIfNotThere );
            void load( Database::iterator dbi, bool createIfNotThere );

            /**
             * Dump this urlist to the given stream in the form
             * aid, vid, { urlid1, urlid2... }
             * to the given stream. or in a nice XML format if that is given
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
             * Is this URLList needing to be saved to disk?
             */
            bool isDirty();

            /**
             * get the inverted file that this term is stored in.
             */
            fh_invertedfile getInvertedFile();

            /**
             * get the AID part of the key for this url list
             */
            aid_t getAID();

            /**
             * get the value part of the key for this url list
             */
            const std::string& getValue();
            
        };

        template <class Iter>
        struct SelectedRange
        {
            SelectedRange( Iter begin, Iter end, Iter selected )
                :
                begin( begin ),
                end( end ),
                selected( selected )
                {
                }
                
            Iter begin;
            Iter end;
            Iter selected;
        };

            
        class FERRISEXP_API InvertedFile
            :
            public Handlable
        {
            fh_env       m_dbenv;
            fh_database  m_db;
            fh_db4idx    m_idx;
            std::string  m_basepath;
            std::string  m_filename;
            

            friend struct invertedfile_dbiter_compare;
            friend int bt_compare_aidvid_cs(DB* rawdb, const DBT* a, const DBT* b );

            typedef Loki::Functor< int,
                                   LOKI_TYPELIST_2( const std::string&,
                                                    const std::string&  ) > ValueCompare;
            ValueCompare m_valueCompare;

            /**
             * If a lower_bound() is operating on a string then it should make sure that
             * both strings are of equal length, if it is a numeric type then any trimming
             * of the value will trash the results. Only trim if m_shouldTrimValueCompareString is true
             */
            bool m_shouldTrimValueCompareString;

            
            /**
             * Use the value comparison function with the given name.
             * @param compareFunctionName can be one of: string, cis, int, float, double, binary
             */
            void setValueCompare( const std::string& compareFunctionName );

            typedef Cache< std::pair< aid_t, std::string >,
                              fh_urllist > m_urllist_cache_t;
            m_urllist_cache_t m_urllist_cache;

            typedef std::map< std::pair< aid_t, std::string >, fh_urllist > m_urllist_hard_cache_t;
            m_urllist_hard_cache_t m_urllist_hard_cache;
            
            /**
             * All methods in this class should use this method to create/access a
             * urllist. This allows them to be cached in memory after being loaded
             * from disk and also for there to be only one object per aid/vid.
             */
            fh_urllist getCachedURLList( Database::iterator dbi,
                                         aid_t aid,
                                         const std::string& value,
                                         bool createIfNotThere = true );
            fh_urllist getCachedURLList( aid_t aid,
                                         const std::string& value,
                                         bool createIfNotThere = true );
            
        public:
            InvertedFile( fh_db4idx m_idx, fh_env dbenv,
                          const std::string& basepath,
                          const std::string& filename = "",
                          const std::string& compareFunctionName = "string" );
            virtual ~InvertedFile();
            
            /**
             * if urlid is not in the inverted list for the key {aid,vid}
             * then add it to that list. If {aid,vid} is not in the inverted
             * file then it is added first.
             */
            fh_urllist ensureURLInMapping( aid_t aid, const std::string& v, urlid_t urlid, XSDBasic_t scid );

            /**
             * either load the urllist for aid,vid if it exists or create a new one
             */
            fh_urllist loadOrAdd( aid_t aid, const std::string& value );
            
            /**
             * get the url list for attributes with aid,vid
             */
            fh_urllist find( aid_t aid, const std::string& value );

            /**
             * get the urllists for all values of an attribute
             */
            urllists_t find_partial( aid_t aid );

            typedef Loki::Functor< bool, LOKI_TYPELIST_1( const std::string& ) > keyPredicate_t;
            urllists_t find_partial( aid_t aid, keyPredicate_t f );
            
            /**
             * Much like STL::lower_bound(). Here we operate on the
             * list of values for a given attribute and return a list
             * of the inverted lists of doc ids that match the lower_bound()
             * for the vid_t range on the aid_t.
             *
             * Note that to find the range returned a binary search is done
             * and each vid_t found in the database is reverse mapped and
             * then compared to the value k
             */
            typedef SelectedRange< Database::iterator > iterator_range;
            iterator_range lower_bound( aid_t aid, const std::string& k );
            iterator_range upper_bound( aid_t aid, const std::string& k );
            urllists_t getURLLists( Database::iterator begin, Database::iterator end );
            
            virtual void sync();

            /**
             * attempt to compact the index by dropping out revoked
             * documents etc.
             */
            virtual void compact( fh_ostream oss, bool verbose = false );

            /**
             * get the database we are storing in
             */
            fh_database getDB();

            /**
             * get the number of items that are in the inverted file.
             * ie, the number of aid,vid -> urllist mappings in the db
             */
            urllist_id_t getNumberOfItems();


            /**
             * Dump out the inderted file (aka index) in the form
             * aid, vid, { urlid1, urlid2... }
             * to the given stream. or in a nice XML format if that is given
             */
            void dumpTo( fh_ostream oss, bool asXML = true );

            fh_db4idx getIndex();
            
        };

        class FERRISEXP_API EAIndexManagerDB4Creator;
        
        
        class FERRISEXP_API EAIndexManagerDB4
            :
            public MetaEAIndexerInterface,
            public PathManager
        {
            friend class EAIndexManagerDB4Creator;
            EAIndexManagerDB4();

            fh_env      m_dbenv;
            int         m_filesIndexedCount;
            int         m_filesIndexedSinceAnalyseCount;
            
            
//             EAIndexManagerDB4( fh_context c,
//                             const std::string& attributeMap_class,
//                             const std::string& forwardValueMap_class,
//                             const std::string& reverseValueMap_class,
//                             const std::string& eanames_ignore,
//                             const std::string& eanames_ignore_regex,
//                             std::streamsize max_value_size );
            
//             friend fh_idx createEAIndex( fh_context c,
//                                          const std::string& attributeMap_class,
//                                          const std::string& forwardValueMap_class,
//                                          const std::string& reverseValueMap_class,
//                                          const std::string& eanames_ignore,
//                                          const std::string& eanames_ignore_regex,
//                                          std::streamsize max_value_size );

        protected:

            virtual void CreateIndex( fh_context c,
                                      fh_context md );
            virtual void CommonConstruction();
            
        public:
            enum invertedSort_t {
                INV_INT    = 0,
                INV_DOUBLE = 1,
                INV_CIS    = 2,
                INV_STRING = 3,
            };
            enum { INV_LAST = INV_STRING };
            

            /**
             * get the string name for the enum type
             */
            std::string invertedSortTypeToString( invertedSort_t v );

            /**
             * convert a string into a enum type
             */
            invertedSort_t stringToInvertedSortType( const std::string& s );
            
        private:
            
//             friend fh_idx Factory::getDefaultEAIndex();
//             friend fh_idx Factory::getEAIndex( const std::string& );

/////            EAIndexManagerDB4( const std::string& basepath );

            std::string      m_basepath;
            fh_lexicon       m_attributeNameMap;
//             fh_lexicon       m_schemaValueMap;
//             fh_revlexicon    m_reverseValueMap;
            fh_docmap        m_docmap;

//            fh_invertedfile  m_inv;
//            fh_invertedfile m_invertedfiles[ INV_LAST + 1 ];
            fh_invertedfile* m_invertedfiles;

            void ensureAttrNameValueReverseValueMapsCreated();
            
//             /**
//              * Ensure that there is a db4 file there for setConfig() / getConfig() to use
//              */
//             void ensureConfigFileCreated();

            /**
             * add all the docids found in the given url list to docnums
             */
            void addDocs( docNumSet_t& docnums, fh_urllist ul );
            
            /**
             * As ExecuteQuery() might want to execute an opcode against many different
             * inverted files, it can call one of the special methods to perform
             * ==, <= or >= opcodes using the given inverted file.
             */
            docNumSet_t& ExecuteEquals( fh_context q,
                                        docNumSet_t& docnums,
                                        fh_invertedfile inv,
                                        aid_t aid,
                                        const std::string& value );
            docNumSet_t& ExecuteLtEq( fh_context q,
                                      docNumSet_t& docnums,
                                      fh_invertedfile inv,
                                      aid_t aid,
                                      const std::string& value );
            docNumSet_t& ExecuteGtEq( fh_context q,
                                      docNumSet_t& docnums,
                                      fh_invertedfile inv,
                                      aid_t aid,
                                      const std::string& value );
            
            docNumSet_t&
            ExecuteQuery( fh_context q, docNumSet_t& docnums, int limit = 0 );
            
        public:

            virtual ~EAIndexManagerDB4();

            virtual std::string getBasePath();
            virtual void sync();

            virtual void addToIndex( fh_context c,
                                     fh_docindexer di );
            virtual docNumSet_t& ExecuteQuery( fh_context q,
                                               docNumSet_t& output,
                                               fh_eaquery qobj,
                                               int limit = 0 );
            virtual std::string resolveDocumentID( docid_t );

            virtual bool getIndexMethodSupportsIsFileNewerThanIndexedVersion();
            virtual bool isFileNewerThanIndexedVersion( const fh_context& c );

            virtual void removeDocumentsMatchingRegexFromIndex( const std::string& s,
                                                                time_t mustBeOlderThan = 0 );
            virtual bool supportsRemove()
                {
                    return true;
                }
            

            fh_lexicon       getAttributeNameMap();
//             fh_lexicon       getSchemaValueMap();
//             fh_revlexicon    getReverseValueMap();
            fh_docmap        getDocumentMap();
            fh_invertedfile  getInvertedFile( invertedSort_t v );
            fh_invertedfile  getInvertedFile( const std::string& sortType );

            virtual std::string getConfig( const std::string& k, const std::string& def,
                                           bool throw_for_errors = false )
                {
                    return MetaEAIndexerInterface::getConfig( k, def, throw_for_errors );
                }
            virtual void setConfig( const std::string& k, const std::string& v )
                {
                    MetaEAIndexerInterface::setConfig( k, v );
                }


            /**
             * attempt to compact the index by dropping out revoked
             * documents etc.
             */
            void compact( fh_ostream oss, bool verbose = false );

            
            /**
             * Get the next available aid to use
             */
            aid_t getNextAID();

            /**
             * Get the next available vid to use
             */
            vid_t getNextVID();


            /**
             * set the name of the code to use on the document numbers in the inverted
             * file.
             */
            void setDocumentNumberGapCode( const std::string& codename );

            /**
             * get the name of the code to use on the document numbers in the inverted
             * file.
             */
            std::string getDocumentNumberGapCode();

            /********************************************************************************/
            /********************************************************************************/
            /*** getting some of the config stuff for index dumping/debug *******************/

            std::string getAttributeNameMapClassName();
//             std::string getValueMapClassName();
//             std::string getReverseValueMapClassName();


            /********************************************************************************/
            /********************************************************************************/
            /********************************************************************************/

        public:

            /************************************************************/
            /************************************************************/
            /************************************************************/
            /*** The following are private ******************************/
            /************************************************************/
            /************************************************************/
            /************************************************************/

            typedef std::set< docid_t > m_revokedDocumentIDCache_t;
            m_revokedDocumentIDCache_t m_revokedDocumentIDCache;
            
            /**
             * Use a fast cache to see if the given docid is revoked.
             */
            bool isRevokedDocumentID( docid_t d );

            /**
             * Remove the given docid from the revoked docid cache.
             */
            void removeRevokedDocumentIDCache( docid_t d );

            /**
             * Ensure the given docid is in the revoked docid cache.
             */
            void ensureRevokedDocumentIDCache( docid_t d );

            std::string getRevokedDocumentIDCacheFileName();
            void loadRevokedDocumentIDCache();
            void saveRevokedDocumentIDCache();
            
            /**
             * internal use only
             */
            static MetaEAIndexerInterface* Create();

            virtual stringset_t&
            getValuesForAttribute( stringset_t& ret, const std::string& eaname, AttrType_t att = ATTRTYPEID_CIS);
            
        };
        
        
    };
};
