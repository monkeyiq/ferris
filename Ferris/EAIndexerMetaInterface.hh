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

    $Id: EAIndexerMetaInterface.hh,v 1.21 2011/05/03 21:30:21 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/***
 * This class/interface was created when a relational database indexing module
 * for EA data was added. The old native EA indexing became a new subclass
 * of the meta interface which provides a course enough API to allow different
 * backends to handle query resultion in a manner efficient to that backend.
 *
 * Some Main Operations were identified and support added as virtual methods
 * in the meta interface. This header file has limitations on what can be
 * #include<>ed and should be as light as possible.
 *
 * 1) Add a new fh_context to index
 * 2) resolve a query tree and append all the document numbers matching a it
 *    to a list. This allows a direct translation of the
 *    (&(width<100)(size>=4k))
 *    usual query syntax into SQL for the relational backend.
 * 3) resolve a docid to a fh_context
 *
 * Note that this light weight interface was created sometime shortly after
 * the FullText meta interface and the two classes are very similar.
 *
 */
#ifndef _ALREADY_INCLUDED_FERRIS_EAIDX_METAINTERFACE_H_
#define _ALREADY_INCLUDED_FERRIS_EAIDX_METAINTERFACE_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <FerrisLoki/loki/Factory.h>
#include <FerrisLoki/loki/Functor.h>
#include <Ferris/SchemaSupport.hh>
#include <string>

#include <FerrisStreams/All.hh>

namespace Ferris
{
    namespace FullTextIndex 
    {
        class MetaFullTextIndexerInterface;
        FERRIS_SMARTPTR( MetaFullTextIndexerInterface, fh_idx );
    };
    

    namespace EAIndex 
    {
        class FERRISEXP_API FederatedEAIndexer;
        class FERRISEXP_API MetaEAIndexerInterface;
        FERRIS_SMARTPTR( MetaEAIndexerInterface, fh_MetaEAIndexerInterface );
        
        typedef guint32 docid_t;                      
        typedef std::set< docid_t > docNumSet_t;      

        class MetaEAIndexerInterface;
        FERRIS_SMARTPTR( MetaEAIndexerInterface, fh_idx );

        class DocumentIndexer;
        FERRIS_SMARTPTR( DocumentIndexer, fh_docindexer );
        
        /**
         * Progress signal for adding new contexts to the ea indexing engine.
         *
         * @param fh_context is the context being added
         * @param int is the current attribute number being added for this context
         * @param int is the total number of attributes that will be added for this context
         */
        typedef sigc::signal3< void, fh_context, int, int >
        AddToEAIndexProgress_Sig_t;
        FERRISEXP_API AddToEAIndexProgress_Sig_t& getNullAddToEAIndexProgress_Sig();

        fh_idx createEAIndex( const std::string& index_classname,
                              fh_context c,
                              fh_context md );

        namespace Factory
        {
            /**
             * Get the default ea index for the user.
             */
            FERRISEXP_API fh_idx getDefaultEAIndex();

            FERRISEXP_API void setDefaultEAIndexPath( const char* EAIndexPath_CSTR );
            

            /**
             * Get the ea index rooted at the given path.
             */
            FERRISEXP_API fh_idx getEAIndex( const std::string& basepath );

            /**
             * Get an indexing object with defaults all setup how the user wishes
             *
             * @param idx the ea index to use, or getDefaultEAIndex() if none
             *            is passed in
             */
            FERRISEXP_API fh_docindexer makeDocumentIndexer( fh_idx idx = 0 );

            /**
             * Use these to make an EA index federation that lasts a single process.
             */
            typedef std::list< fh_idx > eaidxlist_t;
            FERRISEXP_API fh_idx createTemporaryEAIndexFederation( ctxlist_t   eaindexlist, fh_context md = 0 );
            FERRISEXP_API fh_idx createTemporaryEAIndexFederation( eaidxlist_t eaindexlist, fh_context md = 0 );
        };

        FERRISEXP_API void addToTemporaryEAIndexFederation( fh_idx fed, fh_idx subidx );
        
        class MetaEAIndexerInterfacePriv;
        class FERRISEXP_API MetaEAIndexerInterface
            :
            public Handlable
        {
            friend class FederatedEAIndexer;
            MetaEAIndexerInterfacePriv* P;

            friend fh_idx createEAIndex( const std::string& index_classname,
                                         fh_context c,
                                         fh_context md );
            
            friend fh_idx Factory::getEAIndex( const std::string& basepath );
            
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

            /**
             * Builds the map eaname -> metadata for native stateless EA.
             */
            void ensureStatelessEAMetaDataCachePopulated( fh_context& c );
            void ensureStatelessEAMetaDataCachePopulated( fh_context& c, const stringset_t& sl );

            
        protected:

            void addDocID( docNumSet_t& output, docid_t v );
            
            /**
             * internal use only.
             * creator must call private_initialize() next
             */
            MetaEAIndexerInterface();

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
                                      fh_context md ) = 0;
            virtual void CommonConstruction()
                {}

            /**
             * return true if the attribute 'attributeName' should be indexed for context 'c'
             * @see obtainValueIfShouldIndex()
             */
            bool shouldIndex( fh_context c, fh_docindexer di,
                              const std::string& attributeName );
        public:
            std::string getConfig( const std::string& k, const std::string& def,
                                   bool throw_for_errors = false );
        protected:
            void setConfig( const std::string& k, const std::string& v );

            /********************/
            /********************/
            /********************/

            /**
             * The type of an attribute value. This is mainly used to
             * dispatch lookup to the appropriate place because each
             * of these types will probably be best stored in a different
             * lookup because sorting the types or the width of the types
             * gives most efficiency stored in a different lookup.
             */
        public:
            enum AttrType_t
            {
                ATTRTYPEID_INT  = 1,
                ATTRTYPEID_DBL  = 2,
                ATTRTYPEID_TIME = 3,
                ATTRTYPEID_STR  = 10,
                ATTRTYPEID_CIS  = 11
            };
        protected:
            typedef std::list< AttrType_t > AttrTypeList_t;
            AttrTypeList_t getAllAttrTypes();

            /**
             * Mostly a single container for an EA value to be indexed
             * and the metadata about its value.
             *
             * The class uses callbacks into MetaEAIndexerInterface
             * sometimes to allow virtual methods in MetaEAIndexerInterface
             * to supply different implementations for different storage
             * methods.
             */
            class FERRISEXP_API IndexableValue
            {
            private:
                std::string     eaname;
                std::string     value;
                AttrType_t att;
                XSDBasic_t sct;
                
            public:
                AttrType_t      getAttrTypeID() const;
                XSDBasic_t      getSchemaType() const;
                bool            isCaseSensitive() const;
                const std::string& rawValueString() const;
                const std::string& rawEANameString()  const;
                
                
                IndexableValue( MetaEAIndexerInterface* midx,
                                const std::string& eaname,
                                const std::string& value );
                IndexableValue( MetaEAIndexerInterface* midx,
                                fh_context c,
                                const std::string& eaname,
                                const std::string& value );
            };

            /**
             * Can be used by clients to obtain the value of an EA
             * if it should be indexed. Unlike shouldIndex() this method
             * reads the value and if it is larger than the max size that
             * the user wishes to index returns false also.
             *
             * @param c  Context we are indexing
             * @param di Indexer for operation
             * @param attributeName EA of 'c' we are indexing
             * @param v  Where to put the value of 'attributeName' if return value
             *           is true
             * @param acceptNullValues if true then for attributes which exist but
             *                         can't be read isNULL is set to true and
             *                         'v' is empty and return value is true.
             * @return true if attributeName should be indexed.
             * @see shouldIndex()
             */
            bool obtainValueIfShouldIndex( fh_context c, fh_docindexer di,
                                           const std::string& attributeName,
                                           std::string& v,
                                           bool acceptNullValues,
                                           bool& isNULL );
            bool obtainValueIfShouldIndex( fh_context c, fh_docindexer di,
                                           const std::string& attributeName,
                                           std::string& v );

//             bool getDontStoreZeroIntegerAttributes();
//             bool getDontStoreEmptyStringAttributes();
//             void setDontStoreZeroIntegerAttributes( bool v );
//             void setDontStoreEmptyStringAttributes( bool v );
            

            /**
             * Get a list of which ea names should be indexed.
             * subclasses should use obtainValueIfShouldIndex() to
             * read each EA
             */
            stringlist_t&
            getEANamesToIndex( fh_context& c,
                               stringlist_t& ret );
            
            /**
             * Read the EA "token" for the given context and work out
             * what type the value should have in AttrType_t
             * The possibly quoted and value converted version of the
             * IndexableValue can be obtained with asString( IndexableValue );
             *
             * @param eaname is the name of the attribute which we are reading
             *        the value for
             * @param c is the context that has a 'token' with the value we want
             *        to read.
             */
            IndexableValue getIndexableValueFromToken( const std::string& eaname,
                                                       fh_context tokenc );
            IndexableValue getIndexableValue( fh_context c,
                                              const std::string& eaname,
                                              const std::string& value );

            /**
             * Convert the value into a string format with possible quoting
             * of the value or conversion of time values into correct date
             * formats. This method is virtual to allow for example, the ODBC
             * module to format and quite time_t values into value SQL date
             * syntax.
             */
            virtual std::string asString( IndexableValue& v, AttrType_t att );
            /**
             * Calls the longer asString() method with v.getAttrTypeID() as AttrType_t
             */
            virtual std::string asString( IndexableValue& v );

            /**
             * Given a eaname,value combination try to work out what type the
             * value has. Some modules may override this method and use
             * statistics from the index itself to tell which attrType a
             * attribute should have. For example, the vast majority of
             * mtime attributes will be of type EPOCH, other time attributes
             * will likely follow suit.
             *
             * if cop is not provided it will be guessed from the value given
             */
            virtual AttrType_t inferAttrTypeID( const std::string& eaname,
                                                const std::string& value,
                                                const std::string& cop );
            virtual AttrType_t inferAttrTypeID( const std::string& eaname,
                                                const std::string& value );
            /**
             * Calls inferAttrTypeID() with broken up values from iv.
             */
            virtual AttrType_t inferAttrTypeID( IndexableValue& iv );
            
            /********************/
            /********************/
            /********************/
            
        public:
            virtual ~MetaEAIndexerInterface();
            
            std::string getPath();
            std::string getURL();

            /**
             * Called to sync the various data structures that the index might
             * have or to commit current transactions to disk.
             */
            virtual void sync()
                {}

            /**
             * attempt to compact the index by dropping out revoked
             * documents etc.
             */
            virtual void compact( fh_ostream oss, bool verbose = false )
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
             * Before calling obtainValueIfShouldIndex() for the EAs which an indexer
             * wishes to index it should call this method once for the context
             * so that this class can optimize some calls in
             * the obtainValueIfShouldIndex() calls for this context.
             *
             * Note that by default the docindexer will call this method for
             * EAindex subclasses.
             */
            void visitingContext( const fh_context& c );
            
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
             * Add all the document ID's that match a mounted LDAP search
             * query string.
             *
             * @return output set for easy method chaining.
             * @param q root of query tree
             * @param output where to insert docids which match satisfy
             *        the query 'q'.
             */
            virtual docNumSet_t& ExecuteQuery( fh_context q,
                                               docNumSet_t& output,
                                               fh_eaquery qobj,
                                               int limit = 0 ) = 0;

            /**
             * Used in the creation of base views for FCA.
             * This method is only likely to be implemented in SQL backends
             * such as the postgresql backend. As the FCA code is tied to the
             * postgresql database anyway this method is safe for most FCA code.
             *
             * The method is really only of interest to internal libferris
             * code. Default version just returns output as it was passed in.
             */
            struct QueryTermInfo
            {
                AttrType_t m_attrType;
                std::string m_lookupTable;
                QueryTermInfo( AttrType_t m_attrType, std::string m_lookupTable )
                    :
                    m_attrType( m_attrType ),
                    m_lookupTable( m_lookupTable )
                    {
                    }
            };
            typedef std::map< std::string, QueryTermInfo > BuildQuerySQLTermInfo_t;
            virtual docNumSet_t& BuildQuerySQL( fh_context q,
                                                docNumSet_t& output,
                                                fh_eaquery qobj,
                                                std::stringstream& SQLHeader,
                                                std::stringstream& SQLWherePredicates,
                                                std::stringstream& SQLTailer,
                                                stringset_t& lookupTablesUsed,
                                                bool& queryHasTimeRestriction,
                                                std::string& DocIDColumn,
                                                stringset_t& eanamesUsed,
                                                BuildQuerySQLTermInfo_t& termInfo );
            
            
            
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
             * For executing heuristic queries we need to be able to
             * convert strings to integer, size and date information.
             * This method will make a best effort to perform such
             * conversions.
             */
            virtual guint64 convertStringToInteger( const std::string& v );


            /**
             * Allow ferris-fulltext-search clauses to be resolved within the
             * EA index queries using the fulltext index at path.
             */
            void setFulltextIndex( const std::string& path );
            void setFulltextIndex( FullTextIndex::fh_idx fidx );
            FullTextIndex::fh_idx getFulltextIndex();
            
            /********************************************************************************/
            /********************************************************************************/
            /********************************************************************************/

            std::string  getEANamesIgnore();
            void         appendToEANamesIgnore( const std::string& s );
            stringlist_t getEANamesIgnoreRegexes();
            std::streamsize getMaxValueSize();
            std::string getFulltextIndexPath();
            std::string getEANamesToIndexRegex();
            
            stringlist_t getNonResolvableURLsNotToRemoveRegexes();

            /**
             * Remove the files with these URLs from the index if possible.
             * The user might explicitly deny removal of some URLs if they want.
             *
             * Relies on supportsRemove() and removeDocumentsMatchingRegexFromIndex()
             */
            void removeByURL( stringlist_t& sl );
            void removeByURL( stringset_t& sl );

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
             * If the index plugin supports the removal of documents, this method
             * should be overriden to remove any documents with URLs matching the
             * given regex.
             *
             * @param mustBeOlderThan if not zero (the defualt) then matches are only removed
             *        if their docidtime is < mustBeOlderThan
             */
            virtual void removeDocumentsMatchingRegexFromIndex( const std::string& s,
                                                                time_t mustBeOlderThan = 0 );


            /**
             * Either move the files with the following URLs into the multiversion
             * area or for single instance indexes remove them from the index.
             */
            virtual void retireDocumentsFromIndex( docNumSet_t& docids );
            virtual void retireDocumentsFromIndex( stringset_t& urls );
            
            
            

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

            void setFilesIndexedCount( int v );
            void incrFilesIndexedCount();
            
        public:

            /**
             * If an EAIndex implementation supports resolving
             * (url,eaname -> eavalue)
             * then it should return an implementaiton here. The
             * default is null
             */
            virtual fh_fwdeaidx tryToCreateForwardEAIndexInterface();

            /**
             * Given an attribute like artist or title get all the possible
             * values that attribute has in the index. This might be slow
             * or not implemented depending on how the index is designed.
             */
            virtual stringset_t&
            getValuesForAttribute( stringset_t& ret, const std::string& eaname, AttrType_t att = ATTRTYPEID_CIS );

            /**
             * Setup the cache for docid -> url in cache for the
             * document numbers in docnums This allows bulk lookup of
             * the URLs for a set of document IDs if that might be
             * quicker. This is optional, you only have to implement
             * resolveDocumentID(). But if a set can be done quicker,
             * you should consider implementing this method too. for
             * example, if you have to connect to a remote server then
             * one round trip will likely be much faster than 100
             * sequential ones.
             */
            virtual void precacheDocIDs( docNumSet_t& docnums, std::map< docid_t, std::string >& cache );

            /**
             * Number of files added to index so far this time. This is not the total
             * number of files in the index itself, but the number of files added this
             * indexing run.
             */
            int getFilesIndexedCount();

            /*
             * if this is a mixed case string then true
             */
            bool isCaseSensitive( const std::string s );
        };


        // factories for creating the indexes.
        stringlist_t& getMetaEAIndexClassNames();
        bool appendToMetaEAIndexClassNames( const std::string& s );
        typedef Loki::SingletonHolder<
            Loki::Factory< MetaEAIndexerInterface, std::string >,
            Loki::CreateUsingNew, Loki::NoDestroy >
        MetaEAIndexerInterfaceFactory;


        //
        // libferris 1.1.40+ function used by the loadable eaindex
        // module factories to load an implementation and create
        // an object of that eaindex subclass.
        //
        MetaEAIndexerInterface* CreateEAIndexerFromLibrary(
            const std::string& implnameraw );
        

    };

    FERRISEXP_API bool RegisterEAIndexPlugin( const char* libname,
                                              const std::string& ferristype,
                                              const std::string& xsd,
                                              bool requiresNativeKernelDrive,
                                              const std::string& simpleTypes );
    FERRISEXP_API void RegisterEAIndexPluginAlias( const char* libname,
                                                   const std::string& ferristype );
        
};


#endif

