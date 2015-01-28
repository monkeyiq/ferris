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

    $Id: libeaidxcustomferrisdb4tree.hh,v 1.3 2010/09/24 21:31:23 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/HiddenSymbolSupport.hh>
#include "fulltextindexers_custom_plugin/libftxcustomferris.hh"

namespace Ferris
{
    namespace EAIndex 
    {
        using namespace ::Ferris::FullTextIndex;
        using namespace ::STLdb4;

        enum invertedSort_t {
            INV_INT    = 0,
            INV_DOUBLE = 1,
            INV_CIS    = 2,
            INV_STRING = 3,
        };
        enum { INV_LAST = INV_STRING };
        
        typedef termid_t aid_t;
        typedef termid_t vid_t;
        typedef docid_t  urlid_t;
//        typedef std::set< urlid_t > docNumSet_t;
        typedef gint64 urllist_id_t;

        
        class InvertedFileDB4Tree;
        FERRIS_SMARTPTR( InvertedFileDB4Tree, fh_invertedfiledb4tree );

        class URLList;
        FERRIS_SMARTPTR( URLList, fh_urllist );

        typedef std::list< fh_urllist > urllists_t;
        
        struct invertedfile_dbiter_compare;

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        class FERRISEXP_API InvertedFileDB4Tree
            :
            public Handlable
        {
            fh_env        m_dbenv;
            fh_database   m_db;
            fh_db4treeidx m_idx;
            std::string   m_basepath;
            std::string   m_filename;
            

            friend struct invertedfile_dbiter_compare;
            friend int bt_compare_aidvid_cs(DB* rawdb, const DBT* a, const DBT* b );


            /**
             * If a lower_bound() is operating on a string then it should make sure that
             * both strings are of equal length, if it is a numeric type then any trimming
             * of the value will trash the results. Only trim if m_shouldTrimValueCompareString is true
             */
            bool m_shouldTrimValueCompareString;


            invertedSort_t m_sortType;

            

        public:
            InvertedFileDB4Tree( fh_db4treeidx m_idx, fh_env dbenv,
                          invertedSort_t m_sortType,
                          const std::string& basepath,
                          const std::string& filename = "" );
            virtual ~InvertedFileDB4Tree();
            

            void setImplicitTransaction( fh_trans t );
            
            /**
             * ensure that a quad is in the index.
             */
            void put( aid_t aid, XSDBasic_t scid, const std::string& value, urlid_t urlid );
            

            
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

            fh_db4treeidx getIndex();
            
        };

        class FERRISEXP_API EAIndexManagerDB4TreeCreator;
        
        
        class FERRISEXP_API EAIndexManagerDB4Tree
            :
            public MetaEAIndexerInterface,
            public PathManager
        {
            friend class EAIndexManagerDB4TreeCreator;
            EAIndexManagerDB4Tree();

            fh_env      m_dbenv;
            int         m_filesIndexedCount;
            int         m_filesIndexedSinceAnalyseCount;
            fh_trans    m_transaction;
            fh_lexicon  m_cachedAttributeNameMap;
            
            fh_trans ensureTransaction();
            fh_lexicon getCachedAttributeNameMap();
            
        protected:

            virtual void CreateIndex( fh_context c,
                                      fh_context md );
            virtual void CommonConstruction();

            void ensure_docmap();
            void refresh_transaction( bool force = false );
            
        public:
            


            
        private:
            
//             friend fh_idx Factory::getDefaultEAIndex();
//             friend fh_idx Factory::getEAIndex( const std::string& );

/////            EAIndexManagerDB4Tree( const std::string& basepath );

            std::string      m_basepath;
            fh_lexicon       m_attributeNameMap;
//             fh_lexicon       m_schemaValueMap;
//             fh_revlexicon    m_reverseValueMap;
            fh_docmap        m_docmap;

//            fh_invertedfiledb4tree  m_inv;
//            fh_invertedfiledb4tree m_invertedfiles[ INV_LAST + 1 ];
            fh_invertedfiledb4tree* m_invertedfiles;

            void ensureAttrNameValueReverseValueMapsCreated();
            
//             /**
//              * Ensure that there is a db4 file there for setConfig() / getConfig() to use
//              */
//             void ensureConfigFileCreated();

            typedef std::pair<Database::iterator, Database::iterator> dbiterpair_t;
            /**
             * add all the docids found in the given range to docnums
             */
            void addDocs( docNumSet_t& docnums, dbiterpair_t& r );
            
            /**
             * As ExecuteQuery() might want to execute an opcode against many different
             * inverted files, it can call one of the special methods to perform
             * ==, <= or >= opcodes using the given inverted file.
             */
            docNumSet_t& ExecuteEquals( fh_context q,
                                        docNumSet_t& docnums,
                                        fh_invertedfiledb4tree inv,
                                        aid_t aid,
                                        const std::string& value );
            docNumSet_t& ExecuteLtEq( fh_context q,
                                      docNumSet_t& docnums,
                                      fh_invertedfiledb4tree inv,
                                      aid_t aid,
                                      const std::string& value );
            docNumSet_t& ExecuteGtEq( fh_context q,
                                      docNumSet_t& docnums,
                                      fh_invertedfiledb4tree inv,
                                      aid_t aid,
                                      const std::string& value );
            
            docNumSet_t&
            ExecuteQuery( fh_context q, docNumSet_t& docnums, int limit = 0 );
            
        public:

            virtual ~EAIndexManagerDB4Tree();

            virtual std::string getBasePath();
            virtual void sync();

            virtual void addToIndex( fh_context c,
                                     fh_docindexer di );
            virtual docNumSet_t& ExecuteQuery( fh_context q,
                                               docNumSet_t& output,
                                               fh_eaquery qobj,
                                               int limit = 0 );
            virtual std::string resolveDocumentID( docid_t );
            

            fh_lexicon       getAttributeNameMap();
//             fh_lexicon       getSchemaValueMap();
//             fh_revlexicon    getReverseValueMap();
            fh_docmap        getDocumentMap();
            fh_invertedfiledb4tree  getInvertedFile( invertedSort_t v );
            fh_invertedfiledb4tree  getInvertedFile( const std::string& sortType );

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
            
        };
        
        
    };
};
