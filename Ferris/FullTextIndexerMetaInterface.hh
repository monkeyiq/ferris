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

    $Id: FullTextIndexerMetaInterface.hh,v 1.14 2010/09/24 21:30:51 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/***
 * This class/interface was created when the Lucene fulltext indexing was added.
 * Some Main Operations were identified and support added as virtual methods
 * in the meta interface. This header file has limitations on what can be
 * #include<>ed and should be as light as possible.
 *
 * 1) Add a new fh_context to index
 * 2) append all the document numbers matching a term to a list
 * 3) resolve a docid to a fh_context
 */
#ifndef _ALREADY_INCLUDED_FERRIS_FULLTEXTIDX_METAINTERFACE_H_
#define _ALREADY_INCLUDED_FERRIS_FULLTEXTIDX_METAINTERFACE_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <FerrisLoki/loki/Factory.h>
#include <FerrisLoki/loki/Functor.h>
#include <string>

namespace Ferris
{
    namespace FullTextIndex 
    {
        class PathManager;
        typedef guint32 docid_t;                      
        typedef std::set< docid_t > docNumSet_t;      

        class MetaFullTextIndexerInterface;
        FERRIS_SMARTPTR( MetaFullTextIndexerInterface, fh_idx );

        class DocumentIndexer;
        FERRIS_SMARTPTR( DocumentIndexer, fh_docindexer );
        
        /**
         * Progress signal for adding new contexts to the full text indexing
         * engine.
         * @param fh_context is the context being added
         * @param std::streamsize is the current byte offset into the context being added
         * @param std::streamsize is the total size of the context being added
         */
        typedef sigc::signal< void ( fh_context, std::streamsize, std::streamsize ) >
        AddToFullTextIndexProgress_Sig_t;
        AddToFullTextIndexProgress_Sig_t& getNullAddToFullTextIndexProgress_Sig();

        namespace Factory
        {
            /**
             * Get the default lexicon, inverted file and document map for
             * the user.
             */
            FERRISEXP_API fh_idx getDefaultFullTextIndex();
            
            /**
             * Get the lexicon, inverted file and document map rooted
             * at the given path.
             */
            FERRISEXP_API fh_idx getFullTextIndex( const std::string& basepath );

            /**
             * Get an indexing object with defaults all setup how the user wishes
             */
            FERRISEXP_API fh_docindexer makeDocumentIndexer( fh_idx idx = 0 );
        };

        class MetaFullTextIndexerInterfacePriv;
        class FERRISEXP_API MetaFullTextIndexerInterface
            :
            public Handlable
        {
            MetaFullTextIndexerInterfacePriv* P;
            
            friend fh_idx createFullTextIndex( const std::string& index_classname,
                                               fh_context c,
                                               bool caseSensitive,
                                               bool dropStopWords,
                                               StemMode stemMode,
                                               const std::string& lex_class,
                                               fh_context md );
            friend fh_idx Factory::getFullTextIndex( const std::string& basepath );
            
            /**
             * Called after object creation for indexes that are existing.
             * New indexes will instead have CreateIndexBeforeConfig() and
             * CreateIndex() called.
             */
            void private_initialize( const std::string& basepath );
            /**
             * Called for local object init before CommonConstruction() is called.
             */
            void LocalCommonConstruction();

        protected:

            void addDocID( docNumSet_t& output, docid_t v );
            
            /**
             * internal use only.
             * creator must call private_initialize() next
             */
            MetaFullTextIndexerInterface();

            /**
             * Called on object creation for existing indexes for
             * subclasses to perform extra work. For new indexes
             * CreateIndexBeforeConfig() and CreateIndex() will
             * instead be called. After either Setup() or
             * CreateIndex() is called then CommonConstruction() will
             * be called so that common setup operations for the index
             * can be performed.
             *
             * Note that during the call to CreateIndexBeforeConfig()
             * there is no config file created yet, so setConfig()
             * should not be called. Configuration information can be
             * instead set during CreateIndex() which is called after
             * CreateIndexBeforeConfig() has been called and after a
             * config file has been created. There are two methods so
             * that subclasses which are using an index engine that
             * wipes out the entire directory contents on
             * initialization such as Lucene does can create the index
             * and then have the system make the config file *after*
             * the index has been made so that the config file remains
             * in existance.
             *
             * All of Setup(), CreateIndexBeforeConfig(),
             * CreateIndex() and CommonConstruction() do not have to
             * call the parent, they are here only as hooks for
             * subclasses to perform needed work
             */
            virtual void Setup()
                {}
            /**
             * See Setup()
             *
             * @param md Will always be a valid context, its path and url
             *           are meaningless but it could contain EA which the
             *           user has set in a fcreate/gfcreate operation to
             *           pass in extra metadata for the creation process.
             */
            virtual void CreateIndexBeforeConfig( fh_context c,
                                                  bool caseSensitive,
                                                  bool dropStopWords,
                                                  StemMode stemMode,
                                                  const std::string& lex_class,
                                                  fh_context md )
                {}
            /**
             * See Setup()
             *
             * @param md Will always be a valid context, its path and url
             *           are meaningless but it could contain EA which the
             *           user has set in a fcreate/gfcreate operation to
             *           pass in extra metadata for the creation process.
             */
            virtual void CreateIndex( fh_context c,
                                      bool caseSensitive,
                                      bool dropStopWords,
                                      StemMode stemMode,
                                      const std::string& lex_class,
                                      fh_context md ) = 0;
            virtual void CommonConstruction()
                {}
            

            std::string getConfig( const std::string& k, const std::string& def,
                                   bool throw_for_errors = false );
            void setConfig( const std::string& k, const std::string& v );

            class SignalThrottle;
            SignalThrottle getSignalThrottle();
            
            class SignalThrottle
            {
                long m_fireEveryN;
                long m_nextFireAtByte;
                SignalThrottle( long fireEveryN );
                friend SignalThrottle MetaFullTextIndexerInterface::getSignalThrottle();
            public:
                bool operator()( long current );
            };            

            
        public:
            virtual ~MetaFullTextIndexerInterface();

            std::string getPath();
            std::string getURL();

            stringlist_t getNonResolvableURLsNotToRemoveRegexes();
            
            
            /**
             * Remove the files with these URLs from the index if possible.
             * The user might explicitly deny removal of some URLs if they want.
             *
             * Relies on supportsRemove() and removeDocumentsMatchingRegexFromIndex()
             */
            void removeByURL( stringlist_t& sl );
            
            /**
             * When a query system finds URLs it can't resolve it should call
             * here. This will handle the possible removal of those URLs from
             * the index depending on the user's desires. For example, they
             * might not want to remove URLs which are on an NFS server which
             * happens to be down at the moment.
             */
            void queryFoundNonResolvableURLs( stringlist_t& sl );

            /********************************************************************************/
            /********************************************************************************/
            /********************************************************************************/
            
            /**
             * Subclasses supporting isFileNewerThanIndexedVersion() should override this
             * method and return true
             */
            virtual bool getIndexMethodSupportsIsFileNewerThanIndexedVersion();
            /**
             * Return true if the passed context is newer than when it was last added
             * to the index.
             */
            virtual bool isFileNewerThanIndexedVersion( const fh_context& c );

            /********************************************************************************/
            /********************************************************************************/
            /********************************************************************************/
            
            /**
             * Used in the PG EA Index module to shortcut ferris-fulltext EA predicates
             * when the fulltext index chosen is also in the same database.
             */
            bool isTSearch2IndexInGivenDatabase( const std::string& wanted_dbname );

            /**
             * Used by the Clucene ea index module to shortcut embedded fulltext queries
             */
            bool isCLuceneIndex();
            /**
             * A special method to add in the CLucene query object for a fulltext query
             * to the boolean query and return the result.
             */
            virtual void* BuildCLuceneQuery( const std::string& qstr, void* current_query_vp );
            
            
            /**
             * should stop words be dropped in this index
             */
            bool getDropStopWords();

            /**
             * List of static stop words
             */
            stringset_t& getStopWords();

            /**
             * should case be perserved. if not then case folding should be
             * done to lower case.
             */
            bool isCaseSensitive();

            /**
             * get the stemming algo that should be used to trim new items in
             * the lexicon
             */
            StemMode getStemMode();

            /**
             * Indexes may be created that only support boolean queries.
             */
            virtual bool supportsRankedQuery();

            /**
             * Called to sync the various data structures that the index might
             * have like; invertedFile, document map, lexicon etc.
             */
            virtual void sync()
                {}

            /**
             * Called by DocumentIndexer to allow the index implementation to
             * do once off preparations for adding new items to the index.
             * Such preperations can be wound back in sync() after all documents
             * have been added.
             */
            enum prepareForWritesFlags {
                PREPARE_FOR_WRITES_NONE   = 0,
                PREPARE_FOR_WRITES_ISNEWER_TESTS = 1<<1
            };
            virtual void prepareForWrites( int f = PREPARE_FOR_WRITES_NONE )
                {}
            

            /**
             * prepareForWrites() is called before any addToIndex() calls.
             * This is called after all addToIndex() calls to allow subclasses
             * to flush caches setup in prepareForWrites().
             */
            virtual void allWritesComplete();
            
            /**
             * Add the file at the given URL to the index.
             *
             * subclasses should make an effort to emit the progress signal
             * so that UIs can update how far through the indexing is.
             * Use the docIndexer object's getProgressSig() for progress reporting.
             */
            virtual void addToIndex( fh_context c,
                                     fh_docindexer di ) = 0;
            
            /**
             * Add all the document ID's that match a given term
             * to the collection passed in.
             *
             * @return output set for easy method chaining.
             */
            virtual docNumSet_t& addAllDocumentsMatchingTerm(
                const std::string& term,
                docNumSet_t& output,
                int limit ) = 0;

            /**
             * Some indexing code might not store a persistent doc <-> id
             * mapping. In such a case addAllDocumentsMatchingTerm()
             * should keep building an internal one and this method will
             * free that internal build up.
             */
            virtual void cleanDocumentIDCache()
                {}

            /**
             * Resolve a document number to its URL. This call only has
             * to be valid between one or more calls to
             * addAllDocumentsMatchingTerm() and the subsequent call to
             * cleanDocumentIDCache().
             */
            virtual std::string resolveDocumentID( docid_t ) = 0;


            /**
             * For the ability to pass in raw xipian format query
             * strings and have them resolved to a set of docids.
             *
             * Mainly of interest for the xipian module unless other
             * fulltext index modules wish to parse xipian format
             * query strings and resolve them.
             *
             * default is to throw and error of unsupported query format.
             */
            virtual
            docNumSet_t&
            ExecuteXapianFullTextQuery( const std::string& queryString,
                                        docNumSet_t& docnums,
                                        int limit );

            virtual
            docNumSet_t&
            ExecuteWebFullTextQuery( const std::string& queryString,
                                     docNumSet_t& docnums,
                                     int limit );

            

            /**
             * For the ability to pass in raw tsearch2 format query
             * strings and have them resolved to a set of docids.
             *
             * Mainly of interest for the pg tsearch2 module unless other
             * fulltext index modules wish to parse tsearch2 format
             * query strings and resolve them.
             *
             * default is to throw and error of unsupported query format.
             */
            virtual
            docNumSet_t&
            ExecuteTsearch2FullTextQuery( const std::string& queryString,
                                          docNumSet_t& docnums,
                                          int limit );

            virtual
            docNumSet_t&
            ExecuteExternalFullTextQuery( const std::string& queryString,
                                          docNumSet_t& docnums,
                                          int limit );

            virtual
            docNumSet_t&
            ExecuteBeagleFullTextQuery( const std::string& queryString,
                                        docNumSet_t& docnums,
                                        int limit );

            virtual
            docNumSet_t&
            ExecuteLuceneFullTextQuery( const std::string& queryString,
                                        docNumSet_t& docnums,
                                        int limit );
            
            /**
             * Try to execute a queryString using the fulltext index backend.
             * ie, a call to ExecuteXapianFullTextQuery() or
             * ExecuteTsearch2FullTextQuery() is made depending on the backend
             * in use.
             */
            virtual
            docNumSet_t&
            ExecuteRawFullTextQuery( const std::string& queryString,
                                     docNumSet_t& docnums,
                                     int limit );

            /**
             * Called when the index is about to be added to.
             */
            virtual void prepareForInsertions();
            
            /**
             * BRIDGE METHOD. returns true if this is a ferriscustom index.
             */
            virtual bool isCustomFerrisIndex();

            /**
             * if(isCustomFerrisIndex()) cast the class to a path manager.
             */
            virtual PathManager* tryToCastToPathManager();

            /**
             * if(isCustomFerrisIndex()) execute a ranked query
             */
            virtual void executeRankedQuery( fh_context selection,
                                             std::string query_string,
                                             int    m_accumulatorsMaxSize,
                                             int    m_resultSetMaxSize );

            
            /********************************************************************************/
            /********************************************************************************/
            /********************************************************************************/

            /**
             * If the index plugin supports the removal of documents, this method
             * should be overriden to remove any documents with URLs matching the
             * given regex.
             */
            virtual void removeDocumentsMatchingRegexFromIndex( const std::string& s,
                                                                time_t mustBeOlderThan = 0 );

            /**
             * If a class provides the above method then override this and return 1;
             */
            virtual bool supportsRemove();

            /**
             * When multiversioning, remove any instances of document data
             * which are older than the given time. This never effects the
             * current version of document data, only outdated versions.
             */
            virtual void purgeDocumentInstancesOlderThan( time_t t );
            
        protected:

            /**
             * If an index plugin doesn't want to modify anything in the
             * db file it should set this in Setup() to allow the db4 file
             * to live on a read only NFS share if desired.
             */
            void setOpenConfigReadOnly( bool v );
            
            
        };


        // factories for creating the indexes.
        FERRISEXP_API stringlist_t& getMetaFullTextIndexClassNames();
        FERRISEXP_API bool appendToMetaFullTextIndexClassNames( const std::string& s );
        typedef Loki::SingletonHolder<
            Loki::Factory< MetaFullTextIndexerInterface, std::string >,
            Loki::CreateUsingNew, Loki::NoDestroy >
        MetaFullTextIndexerInterfaceFactory;


        //
        // libferris 1.1.40+ function used by the loadable ftx index
        // module factories to load an implementation and create
        // an object of that MetaFullTextIndexerInterface subclass.
        //
        FERRISEXP_API MetaFullTextIndexerInterface* CreateFullTextIndexerFromLibrary(
            const std::string& implnameraw );
        
    };

    FERRISEXP_API bool RegisterFulltextIndexPlugin( const char* libname,
                                                    const std::string& ferristype,
                                                    const std::string& xsd,
                                                    bool requiresNativeKernelDrive,
                                                    const std::string& simpleTypes );
    FERRISEXP_API void RegisterFulltextIndexPluginAlias( const char* libname,
                                                         const std::string& ferristype );
        

};

#endif

