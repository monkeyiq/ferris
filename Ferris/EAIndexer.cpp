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

    $Id: EAIndexer.cpp,v 1.19 2010/09/24 21:30:30 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Ferris/EAIndexer.hh>
#include <Ferris/EAIndexer_private.hh>
#include <Ferris/FullTextIndexer_private.hh>
#include <Ferris/Configuration_private.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/Trimming.hh>
#include <Ferris/Iterator.hh>
#include <Ferris/EAQuery.hh>
#include <Ferris/Medallion.hh>
#include <Ferris/Medallion_private.hh>

using namespace std;

namespace Ferris
{
    namespace EAIndex 
    {
        const string IDXMGR_EXPLICIT_EANAMES_TO_INDEX_K = "idxmgr-explicit-eanames-to-index-k";

        const string IDXMGR_EANAMES_TO_INDEX_REGEX_K = "idxmgr-eanames-to-index-regex-k";


        const string IDXMGR_EANAMES_IGNORE_K = "idxmgr-eanames-ignore-k";
//         const string IDXMGR_EANAMES_IGNORE_DEFAULT = "as-xml,as-text,content,"
//         /**/  "block-count,dontfollow-block-count,inode,dontfollow-inode,dontfollow-size,"
//         /**/  "fs-available-block-count,fs-file-nodes-free,fs-free-block-count,"
//         /**/  "size-human-readable,atime-ctime,ctime-ctime,mtime-ctime,"
//         /**/  "atime-display,ctime-display,mtime-display,"
//         /**/  "md2,mdc2,attribute-count,block-size,device-type,"
//         /**/  "fs-block-count,fs-block-size,fs-file-name-length-maximum,fs-file-nodes-total,"
//         /**/  "fs-type,protection-ls,force-passive-view,"
//         /**/  "recommended-ea,recommended-ea-union,recommended-ea-union-view,"
//         /**/  "download-if-mtime-since-ctime,download-if-mtime-since-display"
//         /**/  "";
        
        const string IDXMGR_EANAMES_REGEX_IGNORE_K = "idxmgr-eanames-regex-ignore-k";
//         const string IDXMGR_EANAMES_REGEX_IGNORE_DEFAULT = (string)"schema:.*" + '\0' 
//         /**/                                                     + "recursive-.*" + '\0'
//         /**/                                                     + "dontfollow-.*" + '\0'
//         /**/                                                     + "subcontext.*";
        const string IDXMGR_MAX_VALUE_SIZE_K = "idxmgr-max-value-size-k";
//        const string IDXMGR_MAX_VALUE_SIZE_DEFAULT = "200";

        const string IDXMGR_FULLTEXT_INDEX_PATH_K = "idxmgr-fulltext-index-path-k";

        
        
        void SET_EAINDEX_EANAMES_NOT_TO_INDEX_DEFAULT( const std::string& v )
        {
            setConfigString( GET_FDB_GENERAL(), "EAINDEX_EANAMES_NOT_TO_INDEX", v );
        }
        void SET_EAINDEX_EANAMES_REGEX_IGNORE_DEFAULT( const std::string& v )
        {
            setConfigString( GET_FDB_GENERAL(), "EAINDEX_EANAMES_REGEX_IGNORE", v );
        }
        void SET_EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT( const std::string& v )
        {
            setConfigString( GET_FDB_GENERAL(), "EAINDEX_MAX_VALUE_SIZE_TO_INDEX", v );
        }
        void SET_EAINDEX_EANAMES_TO_INDEX_REGEX_DEFAULT( const std::string& v )
        {
            setConfigString( GET_FDB_GENERAL(), "EAINDEX_EANAMES_TO_INDEX_REGEX", v );
        }

        std::string GET_EAINDEX_EANAMES_NOT_TO_INDEX_DEFAULT()
        {
            return getConfigString(
                GET_FDB_GENERAL(), "EAINDEX_EANAMES_NOT_TO_INDEX",
                "name,language-human,content,as-xml,as-json,as-text,as-rdf,"
                "block-count,dontfollow-block-count,inode,dontfollow-inode,dontfollow-size,"
                "dontfollow-block-size,dontfollow-device,dontfollow-device-type,"
                "dontfollow-filesystem-filetype,dontfollow-hard-link-count,"
                "recommended-ea-short,"
                "associated-branches,depth-per-color,xfs-ea-names,exif:ea-names,"
                "fs-available-block-count,fs-file-nodes-free,fs-free-block-count,"
                "size-human-readable,"
                "attribute-count,block-size,device-type,"
                "fs-block-count,fs-block-size,fs-file-name-length-maximum,fs-file-nodes-total,"
                "fs-type,protection-ls,force-passive-view,"
                "recommended-ea,recommended-ea-union,recommended-ea-union-view,"
                "is-dir-try-automounting,"
                "rgba-32bpp,exif:thumbnail-update,exif:thumbnail-rgba-32bpp,md2,sha1,crc32,crc32-is-valid,mdc2,download-if-mtime-since,force-passive-view,path,realpath,emblem:upset,emblem:list,emblem:list-ui" );
        }
        std::string GET_EAINDEX_EANAMES_REGEX_IGNORE_DEFAULT( bool commaSeperated )
        {
            string def = (std::string)"^schema:.*" + '\0'
                /**/ + "^recursive-.*" + '\0'
                /**/ + "^subcontext.*" + '\0'
                /**/ + "^branchfs-.*" + '\0'
                /**/ + "^associated-branches-.*" + '\0'
                /**/ + "^as-.*" + '\0'
                /**/ + "^medallion.*" + '\0'
                /**/ + ".*-ctime$" + '\0'
                /**/ + ".*-display$" + '\0'
                /**/ + ".*-cdrom-count$" + '\0'
                /**/ + ".*-dvd-count$" + '\0'
                /**/ + ".*-human-readable$" + '\0'
                /**/ + ".*-granularity$" + '\0'
                /**/ + "^emblem:id-fuzzy.*$" + '\0'
                /**/ + "^emblem:has-fuzzy.*$" + '\0'
                /**/ + "^emblem:.*-mtime.*$" + '\0'
                /**/ + "^geospatial-.*$" + '\0'
                ;
            
            string ret = getConfigString( GET_FDB_GENERAL(), "EAINDEX_EANAMES_REGEX_IGNORE", def );

//             cerr << "GET_EAINDEX_EANAMES_REGEX_IGNORE_DEFAULT(cs:" << commaSeperated << ") "
//                  << " FDB_GENERAL:" << GET_FDB_GENERAL()
//                  << " ret:" << ret
//                  << endl;
            if( commaSeperated )
                replace( ret.begin(), ret.end(), '\0', ',' );
            
            return ret;
        }
        std::string GET_EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT()
        {
            return getConfigString(
                GET_FDB_GENERAL(), "EAINDEX_MAX_VALUE_SIZE_TO_INDEX",
                "1024" );
        }
        std::string GET_EAINDEX_EANAMES_TO_INDEX_REGEX_DEFAULT()
        {
            return getConfigString( GET_FDB_GENERAL(), "EAINDEX_EANAMES_TO_INDEX_REGEX", ".*" );
        }
        



        

        const string IDXMGR_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_K = "idxmgr-nonresolvable-not-to-remove-regex-k";
        void SET_EAINDEX_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_DEFAULT( const std::string& v )
        {
            setConfigString( GET_FDB_GENERAL(), "EAINDEX_NONRESOLVABLE_NOT_TO_REMOVE_REGEX", v );
        }
        std::string GET_EAINDEX_NONRESOLVABLE_NOT_TO_REMOVE_REGEX_DEFAULT( bool commaSeperated )
        {
            string def = (std::string)"" + '\0';
            string ret = getConfigString( GET_FDB_GENERAL(), "EAINDEX_NONRESOLVABLE_NOT_TO_REMOVE_REGEX", def );

            if( commaSeperated )
                replace( ret.begin(), ret.end(), '\0', ',' );
            
            return ret;
        }
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        DocumentIndexer::DocumentIndexer( fh_idx idx )
            :
            m_idx( idx ),
            m_dontCheckIfAlreadyThere( false ),
            m_filesIndexedCount( 0 ),
            m_haveCalledPrepareForWrites( false ),
            m_AddEvenIfAlreadyCurrent( false ),
            m_haveReportedIndexDoesNotSupportAddEvenIfAlreadyCurrent( false ),
            m_retireNonExistentDocumentsToMuliversion( false ),
            m_NonExistentDocumentsState( 0 )
        {
//            cerr << "m_idx->getEANamesIgnore() s:" << m_idx->getEANamesIgnore() << endl;
            setIgnoreEANames( m_idx->getEANamesIgnore() );
            appendIgnoreEARegexs( m_idx->getEANamesIgnoreRegexes() );
            setMaximunValueSize( m_idx->getMaxValueSize() );
            appendNonResolvableURLsNotToRemoveRegexes( m_idx->getNonResolvableURLsNotToRemoveRegexes() );
            setEANamesToIndexRegex( m_idx->getEANamesToIndexRegex() );
        }

        DocumentIndexer::~DocumentIndexer()
        {
            if( m_idx )
                m_idx->allWritesComplete();
                
            if( m_idx )
                m_idx->sync();
        }
        
        AddToEAIndexProgress_Sig_t&
        DocumentIndexer::getProgressSig()
        {
            return m_progressSig;
        }


        void
        DocumentIndexer::setIgnoreEANames( const std::string& s )
        {
            clearIgnoreEANames();
            appendIgnoreEANames( s );
        }

        void
        DocumentIndexer::appendIgnoreEANames( const std::string& s )
        {
            stringlist_t sl = Util::parseCommaSeperatedList( s );
            copy( sl.begin(), sl.end(), inserter( m_ignoreEANames, m_ignoreEANames.end() ));
        }

        void
        DocumentIndexer::clearIgnoreEANames()
        {
            m_ignoreEANames.clear();
        }
        
        stringset_t&
        DocumentIndexer::getIgnoreEANames()
        {
            return m_ignoreEANames;
        }
        

        void
        DocumentIndexer::appendIgnoreEARegexs( const std::string& s )
        {
            m_ignoreEARegexs.append( s );
        }

        void
        DocumentIndexer::appendIgnoreEARegexs( const stringlist_t& sl )
        {
            m_ignoreEARegexs.append( sl );
        }
        
        void
        DocumentIndexer::clearIgnoreEARegexs()
        {
            m_ignoreEARegexs.clear();
        }

        DocumentIndexer::m_ignoreEARegexs_t&
        DocumentIndexer::getIgnoreEARegexs()
        {
            return m_ignoreEARegexs;
        }



        
        void
        DocumentIndexer::appendNonResolvableURLsNotToRemoveRegexes( const std::string& s )
        {
            m_nonResolvableURLsNotToRemoveRegexes.append( s );
        }

        
        void DocumentIndexer::appendNonResolvableURLsNotToRemoveRegexes( const stringlist_t& sl )
        {
            m_nonResolvableURLsNotToRemoveRegexes.append( sl );
        }
        
        void DocumentIndexer::clearNonResolvableURLsNotToRemoveRegexes()
        {
            m_nonResolvableURLsNotToRemoveRegexes.clear();
        }
        RegexCollection&
        DocumentIndexer::getNonResolvableURLsNotToRemoveRegexes()
        {
            return m_nonResolvableURLsNotToRemoveRegexes;
        }
        
        void
        DocumentIndexer::setEANamesToIndex( const stringlist_t& sl )
        {
            m_EANamesToIndex.clear();
            copy( sl.begin(), sl.end(), inserter( m_EANamesToIndex, m_EANamesToIndex.end() ) );
        }
        
        stringlist_t
        DocumentIndexer::getEANamesToIndex()
        {
            stringlist_t ret;
            copy( m_EANamesToIndex.begin(),
                  m_EANamesToIndex.end(),
                  back_inserter( ret ));
            return ret;
        }
        
        bool
        DocumentIndexer::EANamesToIndexContains( const std::string& s )
        {
            if( !m_EANamesToIndex.empty() )
            {
                if( m_EANamesToIndex.find( s ) == m_EANamesToIndex.end() )
                    return false;
            }
            return true;
        }

        

        void
        DocumentIndexer::setEANamesToIndexRegex( const std::string& s )
        {
            m_EANamesToIndexRegex = 0;
            m_EANamesToIndexRegexString = "";
            if( !s.empty() )
                m_EANamesToIndexRegex = toregexh( s );
            m_EANamesToIndexRegexString = s;
        }
        
        std::string&
        DocumentIndexer::getEANamesToIndexRegex()
        {
            return m_EANamesToIndexRegexString;
        }
        
        bool
        DocumentIndexer::EANamesToIndexRegexContains( const std::string& s )
        {
            if( m_EANamesToIndexRegex )
            {
                if( !regex_match( s, m_EANamesToIndexRegex ))
                {
                    return false;
                }
            }
            return true;
        }
        
        
        
        void
        DocumentIndexer::setMaximunValueSize( std::streamsize sz )
        {
            m_maxValueSize = sz;
        }

        std::streamsize
        DocumentIndexer::getMaximunValueSize()
        {
            return m_maxValueSize;
        }
        
        void
        DocumentIndexer::setDontCheckIfAlreadyThere( bool v )
        {
            m_dontCheckIfAlreadyThere = v;
        }
        
        bool
        DocumentIndexer::getDontCheckIfAlreadyThere()
        {
            return m_dontCheckIfAlreadyThere;
        }

        int
        DocumentIndexer::getFilesIndexedCount()
        {
            return m_filesIndexedCount;
        }

        void
        DocumentIndexer::setAddEvenIfAlreadyCurrent( bool v )
        {
            m_AddEvenIfAlreadyCurrent = v;
        }
        bool
        DocumentIndexer::getAddEvenIfAlreadyCurrent()
        {
            return m_AddEvenIfAlreadyCurrent;
        }
        
        

        void
        DocumentIndexer::setRetireNonExistentDocumentsToMuliversion( bool v )
        {
            m_retireNonExistentDocumentsToMuliversion = v;
        }


        class NonExistentDocumentsState
            :
            public Handlable
        {
            fh_context m_dirContext;

            stringset_t m_indexedDocs;
        public:
            NonExistentDocumentsState( fh_context dc );
            void add( fh_context c );
            void retire( fh_context dc, fh_docindexer di );
        };
        
        NonExistentDocumentsState::NonExistentDocumentsState( fh_context dc )
            :
            m_dirContext( dc )
        {
            LG_EAIDX_D << "NonExistentDocumentsState(ctor) dc:" << dc->getURL() << endl;
        }
        
        void
        NonExistentDocumentsState::add( fh_context c )
        {
            LG_EAIDX_D << "NonExistentDocumentsState::add() c:" << c->getURL() << endl;
            m_indexedDocs.insert( c->getURL() );
        }
        
        void
        NonExistentDocumentsState::retire( fh_context dc, fh_docindexer di )
        {
            LG_EAIDX_D << "NonExistentDocumentsState::retire() dc:" << dc->getURL() << endl;
            // Steps:
            // find all the filenames for directory dc
            // remove the filenames we have seen
            // retire the filenames that were there that we didn't find this time


            // Find existing docids from database
            stringset_t dbDocs;
            {
                stringstream qss;
                qss << "(parent-url==" << dc->getURL() << ")";
                ExecuteQueryToToURLs( qss.str(), di->m_idx, dbDocs );
            }

            // Find which are in the database but not seen now.
            stringset_t delme;
            set_difference( dbDocs.begin(), dbDocs.end(),
                            m_indexedDocs.begin(), m_indexedDocs.end(),
                            inserter( delme, delme.end() ) );

            // Remove old docs.
            if( !delme.empty() )
            {
                if( LG_EAIDX_D_ACTIVE )
                {
                    LG_EAIDX_D << "NonExistentDocumentsState::retire() removing docids.sz:"
                               << delme.size() << endl;
                    for( stringset_t::iterator si = delme.begin(); si!=delme.end(); ++si )
                    {
                        LG_EAIDX_D << "retire doc:" << *si << endl;
                    }
                }

                di->m_idx->retireDocumentsFromIndex( delme );
            }
        }
        
        
        void
        DocumentIndexer::EnteringContext(fh_context ctx)
        {
            if( m_retireNonExistentDocumentsToMuliversion )
            {
//                cerr << "DocumentIndexer::EnteringContext() ctx:" << ctx->getURL();
                m_NonExistentDocumentsState = new NonExistentDocumentsState( ctx );
            }
        }
        
        void
        DocumentIndexer::LeavingContext(fh_context ctx)
        {
            if( m_NonExistentDocumentsState )
            {
//                cerr << "DocumentIndexer::LeavingContext() ctx:" << ctx->getURL();
                m_NonExistentDocumentsState->retire( ctx, this );
                m_NonExistentDocumentsState = 0;
            }
        }
        

        
        void
        DocumentIndexer::addContextToIndex( fh_context c )
        {
            if( !m_haveCalledPrepareForWrites )
            {
                m_haveCalledPrepareForWrites = true;
                int f = MetaEAIndexerInterface::PREPARE_FOR_WRITES_NONE;
                if( !m_AddEvenIfAlreadyCurrent )
                    f |= MetaEAIndexerInterface::PREPARE_FOR_WRITES_ISNEWER_TESTS;

                m_idx->prepareForWrites( f );
            }

            LG_EAIDX_D << "DocumentIndexer::addContextToIndex() c:" << c->getURL() << endl;
            
            try
            {
                LG_EAIDX_D << "DocumentIndexer::addContextToIndex(AA) c:" << c->getURL() << endl;
                
                if( !m_AddEvenIfAlreadyCurrent )
                {
                    bool sup = m_idx->getIndexMethodSupportsIsFileNewerThanIndexedVersion();
                    if( !sup && !m_haveReportedIndexDoesNotSupportAddEvenIfAlreadyCurrent )
                    {
                        m_haveReportedIndexDoesNotSupportAddEvenIfAlreadyCurrent = true;
                        cerr << "WARNING: index doesn't support isNewer() checks" << endl;
                    }
                    
                    if( sup )
                    {
                        bool isNewer = m_idx->isFileNewerThanIndexedVersion( c );
                        if( !isNewer )
                        {
                            LG_EAIDX_D << "Skipping because index is current for c:"
                                       << c->getURL() << endl;

                            // retain previous metadata as is.
                            if( m_retireNonExistentDocumentsToMuliversion )
                            {
                                m_NonExistentDocumentsState->add( c );
                            }
                            
                            return;
                        }
                    }

                    if( fh_emblem em = getShouldSkipIndexingEmblem() )
                    {
                        if( c->hasMedallion() )
                        {
                            fh_medallion med  = c->getMedallion();
                            if( med && med->hasEmblem( em ) )
                            {
                                LG_EAIDX_D << "Context has should-skip-indexing emblem... skipping c:" << c->getURL() << endl;
                                cerr << "Context has should-skip-indexing emblem... skipping c:" << c->getURL() << endl;
                                return;
                            }
                        }
                    }
                }

                LG_EAIDX_D << "DocumentIndexer::addContextToIndex(BB) c:" << c->getURL() << endl;
                
                m_idx->visitingContext( c );

                
                LG_EAIDX_D << "DocumentIndexer::addContextToIndex(calling addToIndex) c:"
                           << c->getURL() << endl;

                if( m_retireNonExistentDocumentsToMuliversion )
                {
                    m_NonExistentDocumentsState->add( c );
                }

                m_idx->addToIndex( c, this );
                ++m_filesIndexedCount;
            }
            catch( exception& e )
            {
                // The file existed, but there was an error, don't wipe the old metadata.
                if( m_retireNonExistentDocumentsToMuliversion )
                {
                    m_NonExistentDocumentsState->add( c );
                }
                
                LG_EAIDX_W << "DocumentIndexer::addContextToIndex(1) e:" << e.what() << endl;
                m_idx->sync();
                LG_EAIDX_W << "DocumentIndexer::addContextToIndex(2) e:" << e.what() << endl;
                throw e;
            }
        }


        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        Ferrisls_feaindexadd_display_base::Ferrisls_feaindexadd_display_base( EAIndex::fh_idx idx )
            :
            m_idx( idx ),
            m_contextCount( 0 ),
            exit_status( 0 ),
            m_TotalFilesDoneCount( 0 ),
            m_TotalFilesIndexedCount( 0 ),
            m_tryToAddToFulltextIndexToo( false ),
            m_sloth( false ),
            m_autoClose( false ),
            m_hadUserInteraction( false ),
            m_fullTextIndexPath(""),
            m_onlyAddToFullTextIndex(false),
            m_ftxidx( 0 ),
            m_userSelectedTotalFilesToIndexPerRun( 0 )
        {
            if( m_idx )
                m_indexer = EAIndex::Factory::makeDocumentIndexer( m_idx );
        }

        void
        Ferrisls_feaindexadd_display_base::perform( const std::string& earl )
        {
            if( m_verbose )
                cerr << "srcurl:" << earl << endl;

            m_ls.setURL( earl );
            m_ls();
            DetachAllSignals();
        }
        
        
        void
        Ferrisls_feaindexadd_display_base::setUserSelectedTotalFilesToIndexPerRun( int v )
        {
            m_userSelectedTotalFilesToIndexPerRun = v;
        }
        
        

        void
        Ferrisls_feaindexadd_display_base::sync()
        {
            m_idx->sync();
        }
        
        void
        Ferrisls_feaindexadd_display_base::printTotals()
        {
            if( !m_showTotals )
                return;
        }
    

        void
        Ferrisls_feaindexadd_display_base::setVerbose( bool v )
        {
            m_verbose = v;
        }

        void
        Ferrisls_feaindexadd_display_base::setAddEvenIfAlreadyCurrent( bool v )
        {
            m_indexer->setAddEvenIfAlreadyCurrent( v );
        }
    

        void
        Ferrisls_feaindexadd_display_base::setShowTotals( bool v )
        {
            m_showTotals = v;
        }
    
        void
        Ferrisls_feaindexadd_display_base::setIndex( EAIndex::fh_idx idx )
        {
            m_idx = idx;
            if( m_idx )
                m_indexer = EAIndex::Factory::makeDocumentIndexer( m_idx );
        }
    
        void Ferrisls_feaindexadd_display_base::setIgnoreEANames( const std::string& s )
        {
            m_indexer->setIgnoreEANames( s );
        }
    
        void Ferrisls_feaindexadd_display_base::appendIgnoreEANames( const std::string& s )
        {
            m_indexer->appendIgnoreEANames( s );
        }
        void Ferrisls_feaindexadd_display_base::appendIgnoreEARegexs( const std::string& s )
        {
            m_indexer->appendIgnoreEARegexs( s );
        }
    
        void Ferrisls_feaindexadd_display_base::setMaximunValueSize( std::streamsize sz )
        {
            m_indexer->setMaximunValueSize( sz );
        }

        void Ferrisls_feaindexadd_display_base::setDontCheckIfAlreadyThere( bool v )
        {
            m_indexer->setDontCheckIfAlreadyThere( v );
        }
    
        void Ferrisls_feaindexadd_display_base::setEANamesToIndex( const stringlist_t& sl )
        {
            m_indexer->clearIgnoreEARegexs();
            m_indexer->clearIgnoreEANames();
            m_indexer->setEANamesToIndex( sl );
        }
        int Ferrisls_feaindexadd_display_base::getFilesIndexedCount()
        {
            return m_indexer->getFilesIndexedCount();
        }
        void Ferrisls_feaindexadd_display_base::setRetireNonExistentDocumentsToMuliversion( bool v )
        {
            m_indexer->setRetireNonExistentDocumentsToMuliversion( v );
        }
        void
        Ferrisls_feaindexadd_display_base::setTryToAddToFulltextIndexToo( bool v )
        {
            m_ftxidx = 0;
            m_tryToAddToFulltextIndexToo = v;
            if( m_tryToAddToFulltextIndexToo )
            {
                if( m_idx )
                {
                    if( m_fullTextIndexPath.empty() )
                    {
                        m_ftxidx = m_idx->getFulltextIndex();
                    }
                    else
                    {
                        m_ftxidx = FullTextIndex::Factory::getFullTextIndex( m_fullTextIndexPath );
                    }
                }
            }
        }
        void
        Ferrisls_feaindexadd_display_base::setSloth( bool v )
        {
            m_sloth = v;
        }

        void
        Ferrisls_feaindexadd_display_base::setAutoClose( bool v )
        {
            m_autoClose = v;
        }

        void
        Ferrisls_feaindexadd_display_base::setFullTextIndexPath( const std::string& v )
        {
            m_fullTextIndexPath = v;
            if( !v.empty() )
                m_tryToAddToFulltextIndexToo = true;
        }
        
        void
        Ferrisls_feaindexadd_display_base::setOnlyAddToFullTextIndex( bool v )
        {
            m_onlyAddToFullTextIndex = v;
            if( v )
                m_tryToAddToFulltextIndexToo = true;
        }
        
        
        
        
        
        FullTextIndex::fh_idx
        Ferrisls_feaindexadd_display_base::getFulltextIndex()
        {
            return m_ftxidx;
        }


        FullTextIndex::fh_docindexer
        Ferrisls_feaindexadd_display_base::getDocumentIndexer()
        {
            FullTextIndex::fh_docindexer ftxindexer = FullTextIndex::Factory::makeDocumentIndexer( getFulltextIndex() );
            ftxindexer->setDontCheckIfAlreadyThere( m_indexer->getDontCheckIfAlreadyThere() );
//             if( Verbose )
//                 ftxindexer->getProgressSig().connect( sigc::ptr_fun( progressf ) );
            
            return ftxindexer;
        }
        

        bool Ferrisls_feaindexadd_display_base::shouldEAtryToAddToEAIndex()
        {
            return !m_onlyAddToFullTextIndex;
        }
        
        bool Ferrisls_feaindexadd_display_base::shouldEAtryToAddToFulltextIndex()
        {
            return m_tryToAddToFulltextIndexToo;
        }
        
        
        
        
        void
        Ferrisls_feaindexadd_display_base::PrintEA( fh_context ctx,
                                                    int i,
                                                    const std::string& attr,
                                                    const std::string& EA )
        {
        }

        void
        Ferrisls_feaindexadd_display_base::workStarting()
        {
        }

        void
        Ferrisls_feaindexadd_display_base::workComplete()
        {
        }

        void
        Ferrisls_feaindexadd_display_base::addToIndexFromFileList( EAIndex::fh_idx& idx, fh_istream& fiss )
        {
            string srcURL;
            LG_EAIDX_D << "addToIndexFromFileList(top)" << endl;
            
            EAIndex::fh_docindexer indexer = EAIndex::Factory::makeDocumentIndexer( idx );
            indexer->setAddEvenIfAlreadyCurrent( m_indexer->getAddEvenIfAlreadyCurrent() );
            while( getline( fiss, srcURL ) )
            {
                try
                {
                    LG_EAIDX_D << "adding from stdin src:" << srcURL << endl;

                    if( srcURL.empty() )
                        continue;

                    fh_context c = Resolve( srcURL );

                    if( idx->getIndexMethodSupportsIsFileNewerThanIndexedVersion() )
                    {
                        if( !idx->isFileNewerThanIndexedVersion( c ) )
                        {
                            if( m_verbose )
                            {
                                cerr << "Skipping:" << srcURL << endl;
                            }
                            continue;
                        }
                    }
            
                    cerr << "ADDING:" << srcURL << endl;
                    indexer->addContextToIndex( c );
                    m_TotalFilesIndexedCount++;

                    if( m_userSelectedTotalFilesToIndexPerRun
                        && m_TotalFilesIndexedCount >= m_userSelectedTotalFilesToIndexPerRun )
                    {
                        cerr << "Have reached the selected max number of files to index for this run..." << endl;
                        return;
                    }
                }
                catch( exception& e )
                {
                    cerr << "----------------------" << endl;
                    cerr << "for: " << srcURL << endl;
                    cerr << "cought error:" << e.what() << endl;
                    exit_status = 1;
                }
                ++m_TotalFilesDoneCount;
            }
            LG_EAIDX_D << "addToIndexFromFileList(bottom)" << endl;
        }

        int
        Ferrisls_feaindexadd_display_base::getExitStatus()
        {
            return exit_status || m_ls.hadErrors();
        }
        
        
        /******************************/
        /******************************/
        /******************************/



        void
        EAIndexAddPopTableCollector::reset()
        {
//         CreateTypeName_CSTR    = 0;
            IndexPath_CSTR         = 0;
            FilelistFile_CSTR      = 0;
            FilelistStdin        = 0;
            IgnoreEANames          = 0;
            IgnoreEANamesAppend    = 0;
            IgnoreEARegexsAppend   = 0;
            EAToIndex              = 0;
            Verbose              = 0;
            ShowVersion = 0;
            EAIndexMaxValueSize = toint( EAIndex::GET_EAINDEX_MAX_VALUE_SIZE_TO_INDEX_DEFAULT() );
            DontCheckIfAlreadyThere = 0;
            ListDirectoryNodeOnly = 0;
            ForceReadRootDirectoryNodes = 0;
            RecursiveList   = 0;
            FilterStringCSTR = 0;
            RetireNonExistentDocumentsToMuliversion = 0;
            tryToAddToFulltextIndexToo = 0;
            ShowTotals = 0;
            AddEvenIfAlreadyCurrent = 0;
            Sloth                  = 0;
            AutoClose              = 0;
            FullTextIndexPath_CSTR = 0;
            OnlyAddToFullTextIndex = 0;
            userSelectedTotalFilesToIndexPerRun = 0;
        }
        

        void
        EAIndexAddPopTableCollector::poptCallback(poptContext con,
                                                  enum poptCallbackReason reason,
                                                  const struct poptOption * opt,
                                                  const char * arg,
                                                  const void * data)
        {
            const string key = opt->longName;
            LG_EAIDX_D << "poptCallback() key:" << key << endl;
        }
        
            
        EAIndexAddPopTableCollector::EAIndexAddPopTableCollector()
        {
            reset();
        }

        void
        EAIndexAddPopTableCollector::ArgProcessingDone( poptContext optCon )
        {
            if( ShowVersion )
            {
                cout << "feaindexadd version: $Id: EAIndexer.cpp,v 1.19 2010/09/24 21:30:30 ben Exp $\n"
//                     << "ferris   version: " << VERSION << nl
                     << "Written by Ben Martin, aka monkeyiq" << nl
                     << nl
                     << "Copyright (C) 2001 Ben Martin" << nl
                     << "This is free software; see the source for copying conditions.  There is NO\n"
                     << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
                     << endl;
                exit(0);
            }

            EAIndex::fh_idx idx;

            if( IndexPath_CSTR )
            {
                idx = EAIndex::Factory::getEAIndex( IndexPath_CSTR );
            }
            else
            {
                idx = EAIndex::Factory::getDefaultEAIndex();
            }

            string FilterString  = FilterStringCSTR ? FilterStringCSTR : "";
            fh_Ferrisls_feaindexadd_display_base d = m_obj;
            Ferrisls& ls = d-> getFerrisls();
            
            d->setIndex( idx );
            ls.setListDirectoryNodeOnly( ListDirectoryNodeOnly );
            ls.setForceReadRootDirectoryNodes( ForceReadRootDirectoryNodes );
            ls.setRecursiveList( RecursiveList );
            ls.setDisplay( d );
        
            if( FilterString.length() )
            {
                ls.setFilterString( FilterString );
            }

            if( IgnoreEANames )
                d->setIgnoreEANames( IgnoreEANames );
            if( IgnoreEANamesAppend )
                d->appendIgnoreEANames( IgnoreEANamesAppend );
            if( IgnoreEARegexsAppend )
                d->appendIgnoreEARegexs( IgnoreEARegexsAppend );
            d->setMaximunValueSize( EAIndexMaxValueSize );
            d->setDontCheckIfAlreadyThere( DontCheckIfAlreadyThere );
            d->setShowTotals( ShowTotals );
            d->setAddEvenIfAlreadyCurrent( AddEvenIfAlreadyCurrent );
            d->setRetireNonExistentDocumentsToMuliversion( RetireNonExistentDocumentsToMuliversion );
            d->setTryToAddToFulltextIndexToo( tryToAddToFulltextIndexToo );
            d->setSloth( Sloth );
            d->setAutoClose( AutoClose );
            d->setUserSelectedTotalFilesToIndexPerRun( userSelectedTotalFilesToIndexPerRun );
            if( FullTextIndexPath_CSTR )
                d->setFullTextIndexPath( FullTextIndexPath_CSTR );
            d->setOnlyAddToFullTextIndex( OnlyAddToFullTextIndex );

            if( EAToIndex )
            {
                stringlist_t sl = Util::parseCommaSeperatedList( EAToIndex );
                d->setEANamesToIndex( sl );
            }
            d->setVerbose( Verbose );

            if( FilelistFile_CSTR )
            {
                string filelistFile = FilelistFile_CSTR;
                fh_ifstream fiss( filelistFile );
                d->addToIndexFromFileList( idx, fiss );
                exit( 0 );
            }
            if( FilelistStdin )
            {
                fh_istream fiss = Ferris::Factory::fcin();
                d->addToIndexFromFileList( idx, fiss );
                exit( 0 );
            }
            
        }


        struct ::poptOption*
        EAIndexAddPopTableCollector::getTable( fh_Ferrisls_feaindexadd_display_base obj )
        {
            m_obj = obj;
            allocTable( 100 );
            int i=0;
            setToCallbackEntry( &table[i] );
            ++i;

            setEntry(
                &table[i++],  "verbose", 'v', POPT_ARG_NONE, &Verbose,
                "show what is happening", ""  );

            setEntry(
                &table[i++],  "skip-already-indexed-check", 'S', POPT_ARG_NONE, &DontCheckIfAlreadyThere,
                "don't check if the context is already indexed, just add it.", ""  );

            
            setEntry(
                &table[i++],  "index-path", 'P', POPT_ARG_STRING, &IndexPath_CSTR,
                "which index to use", ""  );

            setEntry(
                &table[i++],  "filelist-file", 'f', POPT_ARG_STRING, &FilelistFile_CSTR,
                "file containing the URLs of the files to index (eg. made by find . >foo)", ""  );

            setEntry(
                &table[i++],  "filelist-stdin", '1', POPT_ARG_NONE, &FilelistStdin,
                "read filenames from stdin to index", ""  );

            setEntry(
                &table[i++],  "total-files-to-index-per-run", 'N', POPT_ARG_INT, &userSelectedTotalFilesToIndexPerRun,
                "only add this many files to the index and then exit. Note that skipped files do not count towards this total, files must be really (re)indexed to count.", ""  );
            
            setEntry(
                &table[i++],  "ignore-ea", 0, POPT_ARG_STRING, &IgnoreEANames,
                "ignore the following attributes when creating the ea index", ""  );

            setEntry(
                &table[i++],  "ignore-ea-append", 0, POPT_ARG_STRING, &IgnoreEANamesAppend,
                "append the following to the list of ea names to ignore when creating index",
                ""  );

            setEntry(
                &table[i++],  "ignore-ea-regex-append", 0, POPT_ARG_STRING, &IgnoreEARegexsAppend,
                "append the following to the list of regexes which select which attributes are not to be indexed",
                ""  );

            setEntry(
                &table[i++],  "ea-value-max-size", 0, POPT_ARG_INT, &EAIndexMaxValueSize,
                "largest attribute value to index", ""  );

            setEntry(
                &table[i++],  "ea-to-index", 0, POPT_ARG_STRING, &EAToIndex,
                "index only the attributes in the comma seperated list. good for automated testing. Note that this option makes --ignore-ea-regex-append, --ignore-ea-append and --ignore-ea impotent", ""  );
                
            setEntry(
                &table[i++],  "directory", 'd', POPT_ARG_NONE,
                &ListDirectoryNodeOnly,
                "(like ls -d) index directory entries instead of contents", 0  );

            setEntry(
                &table[i++],  "force-read-root-dir-nodes", 0, POPT_ARG_NONE,
                &ForceReadRootDirectoryNodes,
                "Always read the context given on cmd line."
                "Handy in use with -d to force EA generation", 0  );

            setEntry(
                &table[i++],  "recursive", 'R', POPT_ARG_NONE, &RecursiveList,
                "index directorys recursively", 0  );

            setEntry(
                &table[i++],  "ferris-filter", 0, POPT_ARG_STRING, &FilterStringCSTR,
                "Only index contexts which pass given filter",
                "(name=fred*)"  );

            setEntry(
                &table[i++],  "show-totals", 0, POPT_ARG_NONE, &ShowTotals,
                "Display statistics at the end for number of files indexed etc.", 0  );

            setEntry(
                &table[i++],  "force", 'F', POPT_ARG_NONE,
                &AddEvenIfAlreadyCurrent,
                "Add all files again even if they have not changed between indexing", 0  );
                
//                 setEntry(
//                &table[i++],  "create-type", 0, POPT_ARG_STRING, &CreateTypeName_CSTR,
//                   "what form of context to store the chunks in (dir/db4/gdbm/xml/etc)", 0  );

            setEntry(
                &table[i++],  "retire-non-existent-to-multiversion", 'K', POPT_ARG_NONE,
                &RetireNonExistentDocumentsToMuliversion,
                "Move metadata about files which no longer exist into multiversion instance now.", 0  );

            setEntry(
                &table[i++],  "try-to-add-to-fulltext-index-too", '2', POPT_ARG_NONE,
                &tryToAddToFulltextIndexToo,
                "If a file appears to be text, try to add the file to the fulltext index associated with the eaindex too.", 0  );

            setEntry(
                &table[i++],  "fulltext-index-path", 0, POPT_ARG_STRING,
                &FullTextIndexPath_CSTR,
                "Alternative fulltext index path to use.", 0  );

            setEntry(
                &table[i++],  "only-add-to-fulltext-index", 0, POPT_ARG_NONE,
                &OnlyAddToFullTextIndex,
                "only add to fulltext index, not to eaindex.", 0  );
            
            setEntry(
                &table[i++], "sloth", 0, POPT_ARG_NONE, &Sloth, 
                "keep the main window closed until it is needed", "" );

            setEntry(
                &table[i++], "auto-close", '0', POPT_ARG_NONE, &AutoClose, 
                "If there is no user interaction or objects skipped then close client automatically", "" );
            
            clearEntry( &table[i] );
            return table;
        }

        
        fh_EAIndexAddPopTableCollector
        Ferrisls_feaindexadd_display_base::getPoptCollector()
        {
            if( !isBound( Collector ))
            {
                Collector = new EAIndexAddPopTableCollector();
            }
            return Collector;
        }
        
        
        
        namespace Priv
        {
            struct ::poptOption* getEAIndexAddPopTableCollector( fh_Ferrisls_feaindexadd_display_base obj )
            {
                return obj->getPoptCollector()->getTable( obj );
            }
        };
        
    };
};

