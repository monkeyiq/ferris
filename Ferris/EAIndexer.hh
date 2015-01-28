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

    $Id: EAIndexer.hh,v 1.12 2010/09/24 21:30:30 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_EAIDX_H_
#define _ALREADY_INCLUDED_FERRIS_EAIDX_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <sigc++/sigc++.h>

#include <Ferris/TypeDecl.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FullTextIndexer.hh>
#include <Ferris/EAIndexerMetaInterface.hh>
#include <Ferris/Cache.hh>
#include <Ferris/Ferrisls.hh>
#include <Ferris/FerrisPopt.hh>

#include <STLdb4/stldb4.hh>

namespace Ferris
{
    /**
     * Support for indexing and querying EA
     *
     * Note that some functions from the FullTextIndex namespace are used here
     * for things like folding case etc.
     */
    namespace EAIndex 
    {
        using namespace ::Ferris::FullTextIndex;
        using namespace ::STLdb4;

        FERRISEXP_API std::string GET_EAINDEX_EANAMES_NOT_TO_INDEX_DEFAULT();
        FERRISEXP_API std::string GET_EAINDEX_EANAMES_REGEX_IGNORE_DEFAULT( bool commaSeperated = false );
        FERRISEXP_API std::string GET_EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT();
        FERRISEXP_API std::string GET_EAINDEX_EANAMES_TO_INDEX_REGEX_DEFAULT();

        class DocumentIndexer;
        FERRIS_SMARTPTR( DocumentIndexer, fh_docindexer );

        class NonExistentDocumentsState;
        FERRIS_SMARTPTR( NonExistentDocumentsState, fh_NonExistentDocumentsState );
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        /**
         * class whos sole objective is to add new documents to the ea index.
         */
        class FERRISEXP_API DocumentIndexer
            :
            public Handlable
        {
        public:
            typedef RegexCollection m_ignoreEARegexs_t;
        private:            
            AddToEAIndexProgress_Sig_t m_progressSig;

            friend fh_docindexer Factory::makeDocumentIndexer( fh_idx idx );
            DocumentIndexer( fh_idx idx );

            fh_idx m_idx;

            int m_attributesDone;
            int m_signalWindow;
            
            stringset_t  m_ignoreEANames;
            m_ignoreEARegexs_t m_ignoreEARegexs;
            std::streamsize m_maxValueSize;
            bool m_dontCheckIfAlreadyThere;
            RegexCollection m_nonResolvableURLsNotToRemoveRegexes;
            
            stringset_t m_EANamesToIndex;
            std::string m_EANamesToIndexRegexString;
            fh_rex      m_EANamesToIndexRegex;

            // total number of files added to the index via this object
            int m_filesIndexedCount;
            bool m_haveCalledPrepareForWrites;

            bool m_AddEvenIfAlreadyCurrent;
            bool m_haveReportedIndexDoesNotSupportAddEvenIfAlreadyCurrent;
            bool m_retireNonExistentDocumentsToMuliversion;
            fh_NonExistentDocumentsState m_NonExistentDocumentsState;
            friend class NonExistentDocumentsState;
            
        public:

            /**
             * syncs data to disk
             */
            virtual ~DocumentIndexer();

            /**
             * A signal that is fired at intervals to allow UIs to update
             * and show progress to the user when indexing a document
             */
            AddToEAIndexProgress_Sig_t& getProgressSig();

            /**
             * Add the passed context to the index
             */
            void addContextToIndex( fh_context c );


            /**
             * Called whenever the directory changes.
             * Used to allow the documentindexer to purge files from
             * the index which are no longer present after a directory
             * has been indexed.
             *
             * ie. for indexing a directory /foo/bar
             * call EnteringContext( /foo/bar );
             * for each file in /foo/bar
             *   call addContextToIndex( file )
             * call LeavingContext( /foo/bar );
             *
             */
            void EnteringContext(fh_context ctx);
            void LeavingContext(fh_context ctx);
            /**
             * If this is true then the caller *must* call
             * EnteringContext() and LeavingContext() and index a
             * single directory at a time.
             */
            void setRetireNonExistentDocumentsToMuliversion( bool v );

            

            
            /**
             * Set the names of attributes to ignore to the following comma
             * seperated list
             */
            void setIgnoreEANames( const std::string& s );

            /**
             * Append to the names of attributes to ignore to the following comma
             * seperated list
             */
            void appendIgnoreEANames( const std::string& s );

            /**
             * clear the names of attributes to ignore.
             */
            void clearIgnoreEANames();

            stringset_t& getIgnoreEANames();


            
            
            /**
             * Append to the list of regex strings which if one passes will cause the
             * attribute named to not be indexed
             * example: schema:.*
             */
            void appendIgnoreEARegexs( const std::string& s );
            void appendIgnoreEARegexs( const stringlist_t& sl );
            void clearIgnoreEARegexs();
            m_ignoreEARegexs_t& getIgnoreEARegexs();

            
            /**
             * Regular expressions for URLs which do not resolve at query time
             * but should not be automatically removed from the index.
             */
            void appendNonResolvableURLsNotToRemoveRegexes( const std::string& s );
            void appendNonResolvableURLsNotToRemoveRegexes( const stringlist_t& sl );
            void clearNonResolvableURLsNotToRemoveRegexes();
            RegexCollection& getNonResolvableURLsNotToRemoveRegexes();
            

            /**
             * If this is set then an attribute must have one of the names in the passed stringlist
             * to be indexed. Note that if an attribute appears in one of the
             * appendIgnoreEARegexs() then it will still be ignored. This should be considered another
             * way to limit which attributes are indexed, ie. for an attribute to be indexed it should
             * appear in this list but if it is in setIgnoreEANames(), appendIgnoreEANames()
             * or appendIgnoreEARegexs() then it will still be ignored.
             * You may consider calling clearIgnoreEARegexs(), clearIgnoreEANames() to make
             * this list the definitive list of EA to index.
             *
             * @see clearIgnoreEARegexs()
             * @see clearIgnoreEANames()
             */
            void setEANamesToIndex( const stringlist_t& sl );
            stringlist_t getEANamesToIndex();
            bool EANamesToIndexContains( const std::string& s );

            /**
             * Regex version of setEANamesToIndex()
             */
            void setEANamesToIndexRegex( const std::string& s );
            std::string& getEANamesToIndexRegex();
            bool EANamesToIndexRegexContains( const std::string& s );
            

            /**
             * Max size for an attribute value to be indexed.
             */
            void setMaximunValueSize( std::streamsize sz );
            std::streamsize getMaximunValueSize();

            /**
             * This option can cause problems because its use allows
             * the possibility for many items in the document map to
             * exist numerous times. It is handy for building indexes
             * when one is sure that a document is only going to be
             * added once. Example: making a index for a cdrom before
             * burning it to disk.
             *
             * Leave it as the default which is false, ie. a check is
             * performed before a new docID is allocated. The only
             * problem with this is that it can be slow to check if
             * a URL has a docid without a reverse document map.
             */
            void setDontCheckIfAlreadyThere( bool v );
            bool getDontCheckIfAlreadyThere();

            /**
             * Get the total number of files that were added to the index
             */
            int getFilesIndexedCount();

            /**
             * Should files be added to the index even if they have not
             * changed since they were last indexed.
             *
             * Default: false
             */
            void setAddEvenIfAlreadyCurrent( bool v = false );
            bool getAddEvenIfAlreadyCurrent();
        };

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        class EAIndexAddPopTableCollector;
        FERRIS_SMARTPTR( EAIndexAddPopTableCollector, fh_EAIndexAddPopTableCollector );
        
        class Ferrisls_feaindexadd_display_base;
        FERRIS_SMARTPTR( Ferrisls_feaindexadd_display_base, fh_Ferrisls_feaindexadd_display_base );
        class FERRISEXP_API Ferrisls_feaindexadd_display_base
            :
            public Ferrisls_display
        {
            typedef Ferrisls_display _Base;

            fh_EAIndexAddPopTableCollector Collector;
            
        protected:

            FullTextIndex::fh_idx m_ftxidx;
            
            EAIndex::fh_idx m_idx;
            EAIndex::fh_docindexer m_indexer;
            bool m_verbose;
            bool m_showTotals;
            int m_contextCount;
            int exit_status;
            int m_TotalFilesDoneCount; //< All files seen
            int m_TotalFilesIndexedCount; //< All files actually added to index (not skipped)
            bool m_tryToAddToFulltextIndexToo;
            bool m_sloth;
            bool m_autoClose;
            bool m_hadUserInteraction;
            std::string m_fullTextIndexPath;
            bool m_onlyAddToFullTextIndex;
            int m_userSelectedTotalFilesToIndexPerRun;

            Ferrisls m_ls;

            virtual void workStarting();
            virtual void workComplete();
            
            virtual void PrintEA( fh_context ctx,
                                  int i,
                                  const std::string& attr,
                                  const std::string& EA );

            FullTextIndex::fh_idx getFulltextIndex();
            FullTextIndex::fh_docindexer getDocumentIndexer();

            bool shouldEAtryToAddToEAIndex();
            bool shouldEAtryToAddToFulltextIndex();
            
        public:

            Ferrisls_feaindexadd_display_base( EAIndex::fh_idx idx );

            void perform( const std::string& earl );
            void addToIndexFromFileList( EAIndex::fh_idx& idx, fh_istream& fiss );
            int getExitStatus();
            
            void sync();
            void printTotals();
            void setVerbose( bool v );
            void setAddEvenIfAlreadyCurrent( bool v );
            void setShowTotals( bool v );
            void setIndex( EAIndex::fh_idx idx );
            void setIgnoreEANames( const std::string& s );
            void appendIgnoreEANames( const std::string& s );
            void appendIgnoreEARegexs( const std::string& s );
            void setMaximunValueSize( std::streamsize sz );
            void setDontCheckIfAlreadyThere( bool v );
            void setEANamesToIndex( const stringlist_t& sl );
            int getFilesIndexedCount();
            void setRetireNonExistentDocumentsToMuliversion( bool v );
            void setTryToAddToFulltextIndexToo( bool v );
            void setSloth( bool v );
            void setAutoClose( bool v );
            void setFullTextIndexPath( const std::string& v );
            void setOnlyAddToFullTextIndex( bool v );
            void setUserSelectedTotalFilesToIndexPerRun( int v );
            
            fh_EAIndexAddPopTableCollector getPoptCollector();

            Ferrisls& getFerrisls()
                {
                    return m_ls;
                }
        };


        class FERRISEXP_API EAIndexAddPopTableCollector
            :
            public basic_PopTableCollector,
            public Handlable
        {
            fh_Ferrisls_feaindexadd_display_base m_obj;

//        const char* CreateTypeName_CSTR    ;
            const char* IndexPath_CSTR         ;
            const char* FilelistFile_CSTR      ;
            unsigned long FilelistStdin        ;
            const char* IgnoreEANames          ;
            const char* IgnoreEANamesAppend    ;
            const char* IgnoreEARegexsAppend   ;
            const char* EAToIndex              ;
            unsigned long Verbose              ;
            unsigned long ShowVersion              ;
            unsigned long EAIndexMaxValueSize  ;
            
            unsigned long DontCheckIfAlreadyThere ;
            unsigned long ListDirectoryNodeOnly ;
            unsigned long ForceReadRootDirectoryNodes ;
            unsigned long RecursiveList   ;
            const char* FilterStringCSTR ;
            unsigned long RetireNonExistentDocumentsToMuliversion ;

            unsigned long ShowTotals ;
            unsigned long AddEvenIfAlreadyCurrent;
            unsigned long tryToAddToFulltextIndexToo;
            unsigned long Sloth;
            unsigned long AutoClose              ;
            const char*   FullTextIndexPath_CSTR;
            unsigned long OnlyAddToFullTextIndex;
            unsigned long userSelectedTotalFilesToIndexPerRun;
            
            void reset();

            virtual void poptCallback(poptContext con,
                                      enum poptCallbackReason reason,
                                      const struct poptOption * opt,
                                      const char * arg,
                                      const void * data);
            
        public:
            EAIndexAddPopTableCollector();
            struct ::poptOption* getTable( fh_Ferrisls_feaindexadd_display_base obj );
            void ArgProcessingDone( poptContext optCon );
            
        };
        
        namespace Priv
        {
            FERRISEXP_API struct ::poptOption* getEAIndexAddPopTableCollector( fh_Ferrisls_feaindexadd_display_base obj );
        };
    
#define FERRIS_EAINDEXADD_OPTIONS(obj) { 0, 0, POPT_ARG_INCLUDE_TABLE,     \
                /**/  ::Ferris::EAIndex::Priv::getEAIndexAddPopTableCollector(obj), \
                /**/  0, "common eaindex add options:", 0 },
        
    };
};
#endif

