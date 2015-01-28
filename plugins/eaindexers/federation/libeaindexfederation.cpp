/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: libeaindexfederation.cpp,v 1.1 2006/12/07 06:49:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
using namespace boost;

#include <Ferris/EAIndexerMetaInterface.hh>
#include <Ferris/EAIndexer_private.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/Native.hh>
#include <Ferris/EAQuery.hh>
#include <Ferris/Configuration_private.hh>
#include <STLdb4/stldb4.hh>
#include <Ferris/FerrisBoost.hh>

// guessComparisonOperatorFromData
#include "FilteredContext_private.hh"

#include <Functor.h>
#include <gmodule.h>

using namespace std;

namespace Ferris
{
    
    namespace EAIndex 
    {
        using namespace ::STLdb4;


        static const char* CFG_IDX_FEDERATION_PRIMARY_WRITE_INDEX_K = "cfg-idx-federation-primary-write-index-k";
        static const char* CFG_IDX_FEDERATION_URLS_K = "cfg-idx-federation-urls-k";

        static const char* CFG_IDX_FEDERATION_SUBST_REGEX_FOR_INDEX_PRE_K = "cfg-idx-federation-subst-regex-k-for-";
        static const char* CFG_IDX_FEDERATION_SUBST_FORMAT_FOR_INDEX_PRE_K = "cfg-idx-federation-subst-format-k-for-";
        

        class FERRISEXP_API FederatedEAIndexer
            :
            public MetaEAIndexerInterface
        {
            typedef MetaEAIndexerInterface _Base;
            typedef FederatedEAIndexer _Self;

            /****************************************/
            /****************************************/
            /****************************************/

            struct RegexAndFormat
            {
                fh_rex r;
                string format;
                RegexAndFormat( fh_rex r = 0, string format = "" )
                    : r( r ), format( format )
                    {
                    }
            };
            typedef map< fh_MetaEAIndexerInterface, RegexAndFormat > m_index_url_substs_t;
            m_index_url_substs_t m_index_url_substs;

            std::string applyRegexSubstsForIndex( fh_MetaEAIndexerInterface eidx, const string& earl )
                {
                    fh_rex      r = m_index_url_substs[ eidx ].r;
                    string format = m_index_url_substs[ eidx ].format;

                    LG_EAIDX_D << "applyRegexSubstsForIndex() r:" << r << endl;
                    if( r )
                    {
                        string s = replaceg( earl, r, format );
                        
                        LG_EAIDX_D << "applyRegexSubstsForIndex() given:" << earl
                                   << " returning:" << s << endl;
                        return s;
                    }
                    return earl;
                }
            
            /****************************************/
            /****************************************/
            /****************************************/
            
            fh_MetaEAIndexerInterface m_primaryWriteIndex;
            typedef list< fh_MetaEAIndexerInterface > m_indexes_t;
            m_indexes_t m_indexes;

            struct CompositeID
            {
                fh_MetaEAIndexerInterface m_index;
                docid_t m_docid;
                CompositeID( fh_MetaEAIndexerInterface m_index = 0,
                             docid_t m_docid = 0 )
                    :
                    m_index( m_index ),
                    m_docid( m_docid )
                    {
                    }
                
            };
            typedef map< docid_t, CompositeID > m_federation_t;
            m_federation_t m_federation;
            docid_t m_federation_nextID;

            void setupCompositeIDs( fh_MetaEAIndexerInterface index, docNumSet_t& d )
                {
                    for( docNumSet_t::const_iterator ci = d.begin();
                         ci != d.end(); ++ci )
                    {
                        docid_t docid = *ci;
                        
                        m_federation.insert(
                            make_pair( m_federation_nextID,
                                       CompositeID( index, docid )));
                        ++m_federation_nextID;
                    }
                }
            
            
        protected:

            virtual void Setup()
                {
                    setOpenConfigReadOnly( true );
                }
            virtual void CreateIndexBeforeConfig( fh_context c,
                                                  fh_context md )
                {
                }
            virtual void CreateIndex( fh_context c,
                                      fh_context md )
                {
                    string primaryWriteIndexURL = getStrSubCtx( md, "primary-write-index-url", "" );
                    m_primaryWriteIndex = Factory::getEAIndex( primaryWriteIndexURL );
                    setConfig( CFG_IDX_FEDERATION_PRIMARY_WRITE_INDEX_K, m_primaryWriteIndex->getURL() );

                    stringlist_t sl = Util::parseSeperatedList( getStrSubCtx( md, "read-only-federates", "" ) );
                    setConfig( CFG_IDX_FEDERATION_URLS_K, Util::createSeperatedList( sl ) );
                    
//                     {
//                         stringstream ss;
//                         for( m_indexes_t::iterator iter = m_indexes.begin(); iter!=m_index.end(); ++iter )
//                         {
//                             ss << iter->getURL();
//                         }
//                         setConfig( CFG_IDX_FEDERATION_URLS_K, tostr(ss) );
//                     }
                }
            virtual void CommonConstruction()
                {
                    m_federation_nextID = 1;
                    m_primaryWriteIndex = 0;
                    m_indexes.clear();

                    string earl;

                    earl = getConfig( CFG_IDX_FEDERATION_PRIMARY_WRITE_INDEX_K, "" );
                    m_primaryWriteIndex = Factory::getEAIndex( earl );

                    stringlist_t sl = Util::parseSeperatedList( getConfig( CFG_IDX_FEDERATION_URLS_K, "" ) );
                    for( stringlist_t::iterator si = sl.begin(); si!=sl.end(); ++si )
                    {
                        string earl = *si;
                        
                        fh_idx index = Factory::getEAIndex( earl );
                        m_indexes.push_back( index );

                        {
                            stringstream kss;
                            kss << CFG_IDX_FEDERATION_SUBST_REGEX_FOR_INDEX_PRE_K << earl;
                            string regstr = getConfig( tostr(kss), "" );
                            if( !regstr.empty() )
                            {
                                stringstream kss;
                                kss << CFG_IDX_FEDERATION_SUBST_FORMAT_FOR_INDEX_PRE_K << earl;
                                string formatstr = getConfig( tostr(kss), "" );

                                LG_EAIDX_D << "index:" << earl
                                           << " regex:" << regstr
                                           << " format:" << formatstr << endl;
                                    
                                fh_rex r = toregexh( regstr );
                                m_index_url_substs[ index ] = RegexAndFormat( r, formatstr );
                            }
                        }
                        
                    }
                }

            virtual std::string asString( IndexableValue& v, AttrType_t att )
                {
                    return m_primaryWriteIndex->asString( v, att );
                }
            
            virtual std::string asString( IndexableValue& v )
                {
                    return m_primaryWriteIndex->asString( v );
                }
            
            virtual AttrType_t inferAttrTypeID( const std::string& eaname,
                                                const std::string& value,
                                                const std::string& cop )
                {
                    return m_primaryWriteIndex->inferAttrTypeID( eaname, value, cop );
                }
            
            virtual AttrType_t inferAttrTypeID( const std::string& eaname,
                                                const std::string& value )
                {
                    return m_primaryWriteIndex->inferAttrTypeID( eaname, value );
                }
                
            virtual AttrType_t inferAttrTypeID( IndexableValue& iv )
                {
                    return m_primaryWriteIndex->inferAttrTypeID( iv );
                }

            /****************************************/
            /****************************************/
            /****************************************/

        public:

            FederatedEAIndexer()
                {
                    m_primaryWriteIndex = 0;
                    m_indexes.clear();
                }
                
//             std::string getPath();
//             std::string getURL();

            virtual void sync()
                {
                    m_primaryWriteIndex->sync();
                }

            virtual void prepareForWrites( int f = PREPARE_FOR_WRITES_NONE )
                {
                    m_primaryWriteIndex->prepareForWrites( f );
                }
//            void visitingContext( const fh_context& c );
              virtual void addToIndex( fh_context c,
                                       fh_docindexer di )
                {
                    m_primaryWriteIndex->addToIndex( c, di );
                }
            

            virtual docNumSet_t& ExecuteQuery( fh_context q,
                                               docNumSet_t& output,
                                               fh_eaquery qobj,
                                               int limit = 0 )
                {
                    {
                        docNumSet_t tmp;
                        m_primaryWriteIndex->ExecuteQuery( q, tmp, qobj, limit );
                        setupCompositeIDs( m_primaryWriteIndex, tmp );
                    }
                    
                    for( m_indexes_t::iterator iter = m_indexes.begin(); iter!=m_indexes.end(); ++iter )
                    {
                        docNumSet_t tmp;
                        (*iter)->ExecuteQuery( q, tmp, qobj, limit );
                        setupCompositeIDs( *iter, tmp );
                    }

                    for( m_federation_t::const_iterator fi = m_federation.begin();
                         fi != m_federation.end(); ++fi )
                    {
                        output.insert( fi->first );
                    }
                    
                    return output;
                }
            


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
                                                BuildQuerySQLTermInfo_t& termInfo )
                {
                    return m_primaryWriteIndex->BuildQuerySQL(
                        q, output, qobj,
                        SQLHeader,
                        SQLWherePredicates,
                        SQLTailer,
                        lookupTablesUsed,
                        queryHasTimeRestriction,
                        DocIDColumn,
                        eanamesUsed,
                        termInfo );
                }
                

            virtual void cleanDocumentIDCache()
                {
                    m_primaryWriteIndex->cleanDocumentIDCache();
                }

            virtual std::string resolveDocumentID( docid_t d )
                {
                    CompositeID cid = m_federation[ d ];
                    return applyRegexSubstsForIndex(
                        cid.m_index, cid.m_index->resolveDocumentID( cid.m_docid ) );
                }
            
            virtual guint64 convertStringToInteger( const std::string& v )
                {
                    return m_primaryWriteIndex->convertStringToInteger( v );
                }
            
//            void setFulltextIndex( const std::string& path );

            virtual bool getIndexMethodSupportsIsFileNewerThanIndexedVersion()
                {
                    return m_primaryWriteIndex->getIndexMethodSupportsIsFileNewerThanIndexedVersion();
                }

            virtual bool isFileNewerThanIndexedVersion( const fh_context& c )
                {
                    return m_primaryWriteIndex->isFileNewerThanIndexedVersion( c );
                }
            
        };
        class FederatedEAIndexer;
        FERRIS_SMARTPTR( FederatedEAIndexer, fh_FederatedEAIndexer );



        
        
    };
};

extern "C"
{
    FERRISEXP_API Ferris::EAIndex::MetaEAIndexerInterface* Create()
    {
        return new Ferris::EAIndex::FederatedEAIndexer();
    }
};


