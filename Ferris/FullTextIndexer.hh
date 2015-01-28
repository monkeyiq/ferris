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

    $Id: FullTextIndexer.hh,v 1.8 2010/09/24 21:30:51 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/**
 * Feb 2004: miq decided that creating a switch at the level of
 * FullTextIndexManager would make implementing optional support for
 * Lucene indexing easiest. This means that the old
 * FullTextIndexManager becomes a subclass of a new meta interface
 * which is stored in FullTextIndexerMetaInterface.hh. The
 * meta interface has to include no exception code because the current
 * gcc3.3 doesn't support both C++ and Java exceptions in the one
 * compilation unit.
 *
 *
 *
 */
#ifndef _ALREADY_INCLUDED_FERRIS_FULLTEXTIDX_H_
#define _ALREADY_INCLUDED_FERRIS_FULLTEXTIDX_H_

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
        typedef guint32 termid_t;
        


        /**
         * Stem the given word and return stemmed version
         */
        FERRISEXP_API std::string stem( const std::string& s, StemMode sm = STEM_J_B_LOVINS_68 );


        enum wordcase {
            CASE_UPPER = 1,
            CASE_LOWER,
            CASE_CAPPED
        };
    
        /**
         * Fold case of word to given UPPER/lower/Capped
         */
        FERRISEXP_API std::string foldcase( const std::string& s, wordcase c = CASE_LOWER );
        
        


        class Lexicon;
        FERRIS_SMARTPTR( Lexicon, fh_lexicon );

        class ReverseLexicon;
        FERRIS_SMARTPTR( ReverseLexicon, fh_revlexicon );
        

        class PathManager;
//         FERRIS_SMARTPTR( PathManager, fh_path_mgr );

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        

        /**
         * Contains the mapping from
         * Document term -> ID
         *
         * A collection of methods are defined to handle the persistence of this collection
         * and additions to it. Subclasses are created to handle the storage in different
         * formats
         */
        class FERRISEXP_API Lexicon
            :
            public Handlable
        {
            friend class FullTextIndexManagerNative;
            friend class ::Ferris::EAIndex::EAIndexManagerDB4;
            friend class ::Ferris::EAIndex::EAIndexManagerDB4Tree;
            friend class Lexicon_Cache;
            friend class Lexitest;
            friend class ::Ferris::AI::SvmLight_BinaryClassifierAgentImplemenation;
            
        protected:

            /**
             * Because we are made by factory now, this is like the constructor
             * for subclasses
             */
            virtual void setIndex( fh_idx idx );
            
            /**
             * Because we are made by factory now, this is like the constructor
             * for subclasses
             */
            virtual void setPathManager( PathManager* path_mgr )
                {
                    m_path_mgr = path_mgr;
                }
            /**
             * Because we are made by factory now, this is like the constructor
             * for subclasses
             */
            virtual void setFileName( const std::string& fn )
                {
                    m_filename = fn;
                }
            virtual const std::string getFileName()
                {
                    return m_filename;
                }
            
            fh_idx m_idx;
            PathManager* m_path_mgr;
            std::string m_filename;
            
            Lexicon( fh_idx idx = 0, PathManager* path_mgr = 0 );

            /**
             * Used in dumpTo() and other debugging stuff. Gives the
             * ability to (possibly slowly) walk the lexicon.
             */
            virtual std::string getFirstTerm()
                {
                    return "";
                }
            virtual std::string getNextTerm( const std::string& s )
                {
                    return "";
                }

            bool m_usingNullValue;
            bool m_usingUnreadableValue;
            
        public:

            virtual ~Lexicon();

            /**
             * A zero length string is allowed to exist if it has NULL_VALUE_TERMID
             * and UNREADABLE_VALUE_TERMID can be added on index creation and then
             * used to index attributes which can not be read when indexing occurs.
             */
            enum {
                NULL_VALUE_TERMID = 1,
                UNREADABLE_VALUE_TERMID = 2
            };
            static std::string NULL_VALUE_TERM;
            static std::string UNREADABLE_VALUE_TERM;
            
            void setUsingNullValue( bool v );
            void setUsingUnreadableValue( bool v );

            
            /**
             * add a new term with the given termid.
             */
            virtual void insert( const std::string& term, termid_t termid ) = 0;

            /**
             * get the ID for the given term as set before with addTerm()
             * or 0 if no such term exists.
             */
            virtual termid_t lookup( const std::string& term ) = 0;

            /**
             * Dump out the lexicon in the form
             * term string, termid\n
             * to the given stream.
             *
             * default uses getFirstTerm() / getNextTerm()
             */
            virtual void dumpTo( fh_ostream oss, bool asXML = true, const std::string& name = "" );

            /**
             * Save all data to disk. Depending on what type of storage is
             * being done, a sync may recreate the lexicon file by consolidating
             * an overflow area into the new lexicon.
             */
            virtual void sync()
                {}

            /**
             * For adding documents, some lexicon classes might perform much
             * better if they are primed for adding new terms. Such is the
             * case for prefixcoded lexicons where the entire lexicon can
             * be read into RAM because it must be done for sync() anyway.
             */
            virtual void prepareForInsertions()
                {}

        };

        /**
         * Create a Lexicon object which caches previous results from another lexicon
         * for quicker subsequent lookups
         */
        FERRISEXP_API fh_lexicon wrapWithCache( fh_lexicon l );

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        /**
         * Contains a mapping from
         * ID -> string
         *
         * Subclasses are created to handle the storage in different formats
         */
        class FERRISEXP_API ReverseLexicon
            :
            public Handlable
        {
            friend class FullTextIndexManagerNative;
            friend class ::Ferris::EAIndex::EAIndexManagerDB4;
            friend class ReverseLexicon_Cache;
            friend class Lexitest;
            
        protected:

            bool m_usingNullValue;
            bool m_usingUnreadableValue;

            /**
             * Because we are made by factory now, this is like the constructor
             * for subclasses
             */
            virtual void setPathManager( PathManager* path_mgr )
                {
                    m_path_mgr = path_mgr;
                }
            /**
             * Because we are made by factory now, this is like the constructor
             * for subclasses
             */
            virtual void setFileName( const std::string& fn )
                {
                    m_filename = fn;
                }
            virtual const std::string getFileName()
                {
                    return m_filename;
                }
            
            PathManager* m_path_mgr;
            std::string m_filename;
            
            ReverseLexicon();

            virtual termid_t getFirstTerm() = 0;
            virtual termid_t getNextTerm( termid_t t ) = 0;

            /**
             * Because we are made by factory now, this is like the constructor
             * for subclasses
             */
            virtual void setIndex( fh_idx idx = 0 )
                {
                }
            
        public:

            virtual ~ReverseLexicon();

            void setUsingNullValue( bool v );
            void setUsingUnreadableValue( bool v );

            /**
             * set future calls to lookup( id ) to return s
             */
            virtual void insert( const std::string& s, termid_t id ) = 0;

            /**
             * get the string associated with the given ID or "" if no such association
             */
            virtual std::string lookup( termid_t id ) = 0;

            /**
             * check if there is a string associated with the given ID
             */
            virtual bool exists( termid_t id ) = 0;

            /**
             * Dump out the lexicon in the form
             * term string, termid\n
             * to the given stream.
             *
             * default uses getFirstTerm() / getNextTerm()
             */
            virtual void dumpTo( fh_ostream oss, bool asXML = true );

            /**
             * Save all data to disk. Depending on what type of storage is
             * being done, a sync may recreate the lexicon file by consolidating
             * an overflow area into the new lexicon.
             */
            virtual void sync()
                {}

            /**
             * For adding documents, some lexicon classes might perform much
             * better if they are primed for adding new terms. Such is the
             * case for prefixcoded lexicons where the entire lexicon can
             * be read into RAM because it must be done for sync() anyway.
             */
            virtual void prepareForInsertions()
                {}

        };

        /**
         * Create a ReverseLexicon object which caches previous results from another revlexicon
         * for quicker subsequent lookups
         */
        FERRISEXP_API fh_revlexicon wrapWithCache( fh_revlexicon l );

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        class FERRISEXP_API PathManager
        {
        public:
            virtual std::string getBasePath() = 0;
            virtual std::string getConfig( const std::string& k, const std::string& def,
                                           bool throw_for_errors = false )
                {
                    return def;
                }
            virtual void setConfig( const std::string& k, const std::string& v )
                {}
        };
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        /**
         * class whos sole objective is to add new documents to the full text index.
         */
        class FERRISEXP_API DocumentIndexer
            :
            public Handlable
        {
            AddToFullTextIndexProgress_Sig_t m_progressSig;

            friend fh_docindexer Factory::makeDocumentIndexer( fh_idx idx );
            DocumentIndexer( fh_idx idx );

            fh_idx m_idx;

            std::streamsize m_bytesDone;
            bool m_dontCheckIfAlreadyThere;

            // total number of files added to the index via this object
            int m_filesIndexedCount;
            bool m_haveCalledPrepareForWrites;
            bool m_AddEvenIfAlreadyCurrent;
            bool m_haveReportedIndexDoesNotSupportAddEvenIfAlreadyCurrent;

            enum {
                LARGEST_TOKEN = 16*4096
            };
            std::string getTokenBuffer;

        public:

            /**
             * syncs data to disk
             */
            virtual ~DocumentIndexer();

            /**
             * A signal that is fired at intervals to allow UIs to update
             * and show progress to the user when indexing a document
             */
            AddToFullTextIndexProgress_Sig_t& getProgressSig();
            
            /**
             * Add the passed context to the index
             */
            void addContextToIndex( fh_context c );

            /**
             * Sync the index to disk.
             * For postgresql and relational indexes, if only a small amount
             * of files were added then an analyze might not be performed.
             */
            void sync();

            /**
             * Called by addContextToIndex() to get the next token and
             * to update byte counts.
             */
            std::string getToken( fh_istream& iss );

            /**
             * Get the number of bytes consumed by previous calls to getToken()
             */
            std::streamsize getBytesCompleted();
            
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

            int getFilesIndexedCount();
            

            
        };

        /************************************************************/
        /************************************************************/
        /************************************************************/

        /**
         * Class that takes tokens from a fh_docindexer, reads them
         * all and builds a sorted <token,count> collection which can
         * then be enumerated by clients.
         */
        class FERRISEXP_API UniqSortedTerms
        {
            typedef std::map< std::string, long > m_uniqTerms_t;
            m_uniqTerms_t m_uniqTerms;
            m_uniqTerms_t::const_iterator  m_end;
            m_uniqTerms_t::const_iterator  m_cur;

        public:

            /**
             * Bind this uniqSorter to the input stream and document
             * indexer. 
             */
            UniqSortedTerms( fh_istream& iss,
                             fh_docindexer di,
                             bool isCaseSensitive,
                             StemMode stemmer,
                             const stringset_t& stopwords,
                             bool DropStopWords );

            /**
             * As the data returned from next() is all stored in RAM
             * you can reset and iterate again.
             */
            void reset();
            

            /**
             * Get the next uniq token found in sorted order and the
             * number of times it occured.
             *
             * puts output into s and termCount and returns true while
             * there was a token and false if no next token existed on call.
             */
            bool next( std::string& s, int& termCount );
        };

    };
};
#endif

